/**
 * \file mon-blows.c
 * \brief Monster melee module.
 *
 * Copyright (c) 1997 Ben Harrison, David Reeve Sward, Keldon Jones.
 *               2013 Ben Semmler
 *               2016 Nick McConnell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "effects.h"
#include "init.h"
#include "monster.h"
#include "mon-attack.h"
#include "mon-blows.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"

/**
 * ------------------------------------------------------------------------
 * Monster blow methods
 * ------------------------------------------------------------------------ */
/**
 * Return a randomly chosen string to append to an INSULT message.
 */
static const char *monster_blow_random_insult(void)
{
	#define MAX_DESC_INSULT 8
	static const char *desc_insult[MAX_DESC_INSULT] =
	{
		"insults you!",
		"insults your mother!",
		"gives you the finger!",
		"humiliates you!",
		"defiles you!",
		"dances around you!",
		"makes obscene gestures!",
		"moons you!!!"
	};

	return desc_insult[randint0(MAX_DESC_INSULT)];
	#undef MAX_DESC_INSULT
}

/**
 * Return a randomly chosen string to append to a MOAN message.
 */
static const char *monster_blow_random_moan(void)
{
	#define MAX_DESC_MOAN 8
	static const char *desc_moan[MAX_DESC_MOAN] = {
		"wants his mushrooms back",
		"tells you to get off his land",
		"looks for his dogs",
		"says 'Did you kill my Fang?'",
		"asks 'Do you want to buy any mushrooms?'",
		"seems sad about something",
		"asks if you have seen his dogs",
		"mumbles something about mushrooms"
	};

	return desc_moan[randint0(MAX_DESC_MOAN)];
	#undef MAX_DESC_MOAN
}

/**
 * Return an action string to be appended on the attack message.
 *
 * \param method is the blow method.
 */
const char *monster_blow_method_action(struct blow_method *method)
{
	const char *action = NULL;

	if (method->act_msg) {
		action = method->act_msg;
	} else if (streq(method->name, "INSULT")) {
		action = monster_blow_random_insult();
	} else if (streq(method->name, "MOAN")) {
		action = monster_blow_random_moan();
	}

	return action;
}

/**
 * ------------------------------------------------------------------------
 * Monster blow effect helper functions
 * ------------------------------------------------------------------------ */
int blow_index(const char *name)
{
	int i;

	for (i = 1; i < z_info->blow_effects_max; i++) {
		struct blow_effect *effect = &blow_effects[i];
		if (my_stricmp(name, effect->name) == 0)
			return i;
	}
	return 0;
}

static int monster_elemental_damage(melee_effect_handler_context_t *context,
									int type, enum mon_messages *hurt_msg,
									enum mon_messages *die_msg)
{
	struct monster_lore *lore = get_lore(context->t_mon->race);
	int hurt_flag = RF_NONE;
	int imm_flag = RF_NONE;
	int damage = 0;

	/* Deal with elemental types */
	switch (type) {
		case PROJ_ACID: {
			imm_flag = RF_IM_ACID;
			break;
		}
		case PROJ_ELEC: {
			imm_flag = RF_IM_ELEC;
			break;
		}
		case PROJ_FIRE: {
			imm_flag = RF_IM_FIRE;
			hurt_flag = RF_HURT_FIRE;
			*hurt_msg = MON_MSG_CATCH_FIRE;
			*die_msg = MON_MSG_DISINTEGRATES;
			break;
		}
		case PROJ_COLD: {
			imm_flag = RF_IM_COLD;
			hurt_flag = RF_HURT_FIRE;
			*hurt_msg = MON_MSG_BADLY_FROZEN;
			*die_msg = MON_MSG_FREEZE_SHATTER;
			break;
		}
		case PROJ_POIS: {
			imm_flag = RF_IM_POIS;
			break;
		}
		default: return 0;
	}

	rf_on(lore->flags, imm_flag);
	rf_on(lore->flags, hurt_flag);

	if (rf_has(context->t_mon->race->flags, imm_flag)) {
		*hurt_msg = MON_MSG_RESIST_A_LOT;
		damage = context->damage / 9;
	} else if (rf_has(context->t_mon->race->flags, hurt_flag)) {
		damage = context->damage * 2;
	}

	return damage;
}

/**
 * Deal damage to a monster from another monster.
 *
 * This is a helper for melee handlers. It is very similar to mon_take_hit(),
 * but eliminates the player-oriented stuff of that function.
 *
 * \param context is the project_m context.
 * \param hurt_msg is the message if the monster is hurt (if any).
 * \return true if the monster died, false if it is still alive.
 */
static bool melee_monster_attack(melee_effect_handler_context_t *context,
								 enum mon_messages hurt_msg,
								 enum mon_messages die_msg)
{
	bool mon_died = false;
	int dam = context->damage;
	struct monster *t_mon = context->t_mon;

	/* "Unique" monsters can only be "killed" by the player */
	if (rf_has(t_mon->race->flags, RF_UNIQUE)) {
		/* Reduce monster hp to zero, but don't kill it. */
		if (dam > t_mon->hp) dam = t_mon->hp;
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == t_mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Wake the monster up */
	mon_clear_timed(t_mon, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, false);

	/* Hurt the monster */
	t_mon->hp -= dam;

	/* Dead or damaged monster */
	if (t_mon->hp < 0) {
		/* Death message */
		add_monster_message(t_mon, die_msg, false);

		/* Generate treasure, etc */
		monster_death(t_mon, false);

		/* Delete the monster */
		delete_monster_idx(t_mon->midx);
		mon_died = true;
	} else if (!monster_is_mimicking(t_mon)) {
		/* Give detailed messages if visible */
		if (hurt_msg != MON_MSG_NONE) {
			add_monster_message(t_mon, hurt_msg, false);
		} else if (dam > 0) {
			message_pain(t_mon, dam);
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!t_mon->m_timed[MON_TMD_FEAR] &&
		!rf_has(t_mon->race->flags, RF_NO_FEAR) && dam > 0) {
		int percentage;

		/* Percentage of fully healthy */
		percentage = (100L * t_mon->hp) / t_mon->maxhp;

		/* Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points */
		if ((randint1(10) >= percentage) ||
			((dam >= t_mon->hp) && (randint0(100) < 80))) {
			int timer = randint1(10) + (((dam >= t_mon->hp) && (percentage > 7))
										? 20 : ((11 - percentage) * 5));
			mon_inc_timed(t_mon, MON_TMD_FEAR, timer,
						  MON_TMD_FLG_NOMESSAGE | MON_TMD_FLG_NOFAIL, false);
		}
	}

	return mon_died;
}

/**
 * Do damage as the result of a melee attack that has an elemental aspect.
 *
 * \param context is information for the current attack.
 * \param type is the PROJ_ constant for the element.
 * \param pure_element should be true if no side effects (mostly a hack
 * for poison).
 */
static void melee_effect_elemental(melee_effect_handler_context_t *context,
								   int type, bool pure_element)
{
	int physical_dam, elemental_dam;
	enum mon_messages hurt_msg = MON_MSG_NONE;
	enum mon_messages die_msg = MON_MSG_NONE;

	if (pure_element)
		/* Obvious */
		context->obvious = true;

	if (context->p) {
		switch (type) {
			case PROJ_ACID: msg("You are covered in acid!");
				break;
			case PROJ_ELEC: msg("You are struck by electricity!");
				break;
			case PROJ_FIRE: msg("You are enveloped in flames!");
				break;
			case PROJ_COLD: msg("You are covered with frost!");
				break;
		}
	}

	/* Give a small bonus to ac for elemental attacks */
	physical_dam = adjust_dam_armor(context->damage, context->ac + 50);

	/* Some attacks do no physical damage */
	if (!context->method->phys)
		physical_dam = 0;

	if (context->p) {
		elemental_dam = adjust_dam(context->p, type, context->damage,
								   RANDOMISE, 0, true);
	} else {
		assert(context->t_mon);
		elemental_dam = monster_elemental_damage(context, type, &hurt_msg,
												 &die_msg);
	}

	/* Take the larger of physical or elemental damage */
	context->damage = (physical_dam > elemental_dam) ?
		physical_dam : elemental_dam;

	if (context->p && elemental_dam > 0)
		inven_damage(context->p, type, MIN(elemental_dam * 5, 300));
	if (context->damage > 0) {
		if (context->p) {
			take_hit(context->p, context->damage, context->ddesc);
		} else {
			assert(context->t_mon);
			(void) melee_monster_attack(context, hurt_msg, die_msg);
		}
	}

	/* Learn about the player */
	if (pure_element && context->p) {
		update_smart_learn(context->mon, context->p, 0, 0, type);
	}
}

/**
 * Do damage as the result of a melee attack that has a status effect.
 *
 * \param context is the information for the current attack.
 * \param type is the TMD_ constant for the effect.
 * \param amount is the amount that the timer should be increased by.
 * \param of_flag is the OF_ flag that is passed on to monster learning for
 * this effect.
 * \param attempt_save indicates if a saving throw should be attempted for
 * this effect.
 * \param save_msg is the message that is displayed if the saving throw is
 * successful.
 */
static void melee_effect_timed(melee_effect_handler_context_t *context,
							   int type, int amount, int of_flag, bool save,
							   const char *save_msg)
{
	/* Take damage */
	if (context->p) {
		take_hit(context->p, context->damage, context->ddesc);
		if (context->p->is_dead) return;
	} else {
		assert(context->t_mon);
		if (melee_monster_attack(context, MON_MSG_NONE, MON_MSG_NONE)) return;
	}

	/* Handle status */
	if (context->t_mon) {
		/* Translate to monster timed effect */
		int mon_tmd_effect = -1;

		/* Will do until monster and player timed effects are fused */
		switch (type) {
			case TMD_CONFUSED: {
				mon_tmd_effect = MON_TMD_CONF;
				break;
			}
			case TMD_PARALYZED: {
				mon_tmd_effect = MON_TMD_HOLD;
				break;
			}
			case TMD_BLIND: {
				mon_tmd_effect = MON_TMD_STUN;
				break;
			}
			case TMD_AFRAID: {
				mon_tmd_effect = MON_TMD_FEAR;
				break;
			}
			default: {
				break;
			}
		}
		if (mon_tmd_effect >= 0) {
			mon_inc_timed(context->t_mon, mon_tmd_effect, amount, 0, false);
			context->obvious = true;
		}
	} else if (save && randint0(100) < context->p->state.skills[SKILL_SAVE]) {
		/* Attempt a saving throw if desired. */
		if (save_msg != NULL) {
			msg("%s", save_msg);
		}
		context->obvious = true;
	} else {
		/* Increase timer for type. */
		if (player_inc_timed(context->p, type, amount, true, true)) {
			context->obvious = true;
		}

		/* Learn about the player */
		update_smart_learn(context->mon, context->p, of_flag, 0, -1);
	}
}

/**
 * Do damage as the result of a melee attack that drains a stat.
 *
 * \param context is the information for the current attack.
 * \param stat is the STAT_ constant for the desired stat.
 */
static void melee_effect_stat(melee_effect_handler_context_t *context, int stat)
{
	/* Take damage */
	if (context->p) {
		take_hit(context->p, context->damage, context->ddesc);
		if (context->p->is_dead) return;
	} else {
		assert(context->t_mon);
		(void) melee_monster_attack(context, MON_MSG_NONE, MON_MSG_NONE);
		return;
	}

	/* Damage (stat) */
	effect_simple(EF_DRAIN_STAT,
			source_monster(context->mon->midx),
			"0",
			stat,
			0,
			0,
			0,
			0,
			&context->obvious);
}

/**
 * Do damage as the result of an experience draining melee attack.
 *
 * \param context is the information for the current attack.
 * \param chance is the player's chance of resisting drain if they have
 * OF_HOLD_LIFE.
 * \param drain_amount is the base amount of experience to drain.
 */
static void melee_effect_experience(melee_effect_handler_context_t *context,
									int chance, int drain_amount)
{
	/* Take damage */
	if (context->p) {
		take_hit(context->p, context->damage, context->ddesc);
		context->obvious = true;
		update_smart_learn(context->mon, context->p, OF_HOLD_LIFE, 0, -1);
		if (context->p->is_dead) return;
	} else {
		assert(context->t_mon);
		(void) melee_monster_attack(context, MON_MSG_NONE, MON_MSG_NONE);
		return;
	}

	if (player_of_has(context->p, OF_HOLD_LIFE) && (randint0(100) < chance)) {
		msg("You keep hold of your life force!");
	} else {
		s32b d = drain_amount +
			(context->p->exp/100) * z_info->life_drain_percent;
		if (player_of_has(context->p, OF_HOLD_LIFE)) {
			msg("You feel your life slipping away!");
			player_exp_lose(context->p, d / 10, false);
		} else {
			msg("You feel your life draining away!");
			player_exp_lose(context->p, d, false);
		}
	}
}

/**
 * ------------------------------------------------------------------------
 * Monster blow effect handlers
 * ------------------------------------------------------------------------ */
/**
 * Melee effect handler: Hit the player, but don't do any damage.
 */
static void melee_effect_handler_NONE(melee_effect_handler_context_t *context)
{
	/* Hack -- Assume obvious */
	context->obvious = true;

	/* Hack -- No damage */
	context->damage = 0;
}

/**
 * Melee effect handler: Hurt the player with no side effects.
 */
static void melee_effect_handler_HURT(melee_effect_handler_context_t *context)
{
	/* Obvious */
	context->obvious = true;

	/* Hack -- Player armor reduces total damage */
	context->damage = adjust_dam_armor(context->damage, context->ac);

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);
}

/**
 * Melee effect handler: Poison the player.
 *
 * We can't use melee_effect_timed(), because this is both and elemental attack
 * and a status attack. Note the false value for pure_element for
 * melee_effect_elemental().
 */
static void melee_effect_handler_POISON(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, PROJ_POIS, false);

	/* Player is dead */
	if (!context->p || context->p->is_dead)
		return;

	/* Take "poison" effect */
	if (player_inc_timed(context->p, TMD_POISONED, 5 + randint1(context->rlev),
						 true, true))
		context->obvious = true;

	/* Learn about the player */
	update_smart_learn(context->mon, context->p, 0, 0, ELEM_POIS);
}

/**
 * Melee effect handler: Disenchant the player.
 */
static void melee_effect_handler_DISENCHANT(melee_effect_handler_context_t *context)
{
	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

	/* Apply disenchantment if no resist */
	if (!player_resists(context->p, ELEM_DISEN))
		effect_simple(EF_DISENCHANT, source_monster(context->mon->midx), "0", 0, 0, 0, 0, 0, &context->obvious);

	/* Learn about the player */
	update_smart_learn(context->mon, context->p, 0, 0, ELEM_DISEN);
}

/**
 * Melee effect handler: Drain charges from the player's inventory.
 */
static void melee_effect_handler_DRAIN_CHARGES(melee_effect_handler_context_t *context)
{
	struct object *obj;
	struct monster *monster = context->mon;
	struct player *current_player = context->p;
	int tries;
	int unpower = 0, newcharge;

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (current_player->is_dead)
		return;

	/* Find an item */
	for (tries = 0; tries < 10; tries++) {
		/* Pick an item */
		obj = context->p->upkeep->inven[randint0(z_info->pack_size)];

		/* Skip non-objects */
		if (obj == NULL) continue;

		/* Drain charged wands/staves */
		if (tval_can_have_charges(obj)) {
			/* Charged? */
			if (obj->pval) {
				/* Get number of charge to drain */
				unpower = (context->rlev / (obj->kind->level + 2)) + 1;

				/* Get new charge value, don't allow negative */
				newcharge = MAX((obj->pval - unpower),0);

				/* Remove the charges */
				obj->pval = newcharge;
			}
		}

		if (unpower) {
			int heal = context->rlev * unpower;

			msg("Energy drains from your pack!");

			context->obvious = true;

			/* Don't heal more than max hp */
			heal = MIN(heal, monster->maxhp - monster->hp);

			/* Heal */
			monster->hp += heal;

			/* Redraw (later) if needed */
			if (current_player->upkeep->health_who == monster)
				current_player->upkeep->redraw |= (PR_HEALTH);

			/* Combine the pack */
			current_player->upkeep->notice |= (PN_COMBINE);

			/* Redraw stuff */
			current_player->upkeep->redraw |= (PR_INVEN);

			/* Affect only a single inventory slot */
			break;
		}
	}
}

/**
 * Melee effect handler: Take the player's gold.
 */
static void melee_effect_handler_EAT_GOLD(melee_effect_handler_context_t *context)
{
	struct player *current_player = context->p;

    /* Take damage */
    take_hit(current_player, context->damage, context->ddesc);

	/* Player is dead */
	if (current_player->is_dead)
		return;

    /* Obvious */
    context->obvious = true;

    /* Attempt saving throw (unless paralyzed) based on dex and level */
    if (!current_player->timed[TMD_PARALYZED] &&
        (randint0(100) < (adj_dex_safe[current_player->state.stat_ind[STAT_DEX]]
						  + current_player->lev))) {
        /* Saving throw message */
        msg("You quickly protect your money pouch!");

        /* Occasional blink anyway */
        if (randint0(3)) context->blinked = true;
    } else {
        s32b gold = (current_player->au / 10) + randint1(25);
        if (gold < 2) gold = 2;
        if (gold > 5000) gold = (current_player->au / 20) + randint1(3000);
        if (gold > current_player->au) gold = current_player->au;
        current_player->au -= gold;
        if (gold <= 0) {
            msg("Nothing was stolen.");
            return;
        }

        /* Let the player know they were robbed */
        msg("Your purse feels lighter.");
        if (current_player->au)
            msg("%d coins were stolen!", gold);
        else
            msg("All of your coins were stolen!");

        /* While we have gold, put it in objects */
        while (gold > 0) {
            int amt;

            /* Create a new temporary object */
            struct object *obj = object_new();
            object_prep(obj, money_kind("gold", gold), 0, MINIMISE);

            /* Amount of gold to put in this object */
            amt = gold > MAX_PVAL ? MAX_PVAL : gold;
            obj->pval = amt;
            gold -= amt;

            /* Set origin to stolen, so it is not confused with
             * dropped treasure in monster_death */
            obj->origin = ORIGIN_STOLEN;
			obj->origin_depth = current_player->depth;

            /* Give the gold to the monster */
            monster_carry(cave, context->mon, obj);
        }

        /* Redraw gold */
        current_player->upkeep->redraw |= (PR_GOLD);

        /* Blink away */
        context->blinked = true;
    }
}

/**
 * Melee effect handler: Take something from the player's inventory.
 */
static void melee_effect_handler_EAT_ITEM(melee_effect_handler_context_t *context)
{
	int tries;

    /* Take damage */
    take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

    /* Saving throw (unless paralyzed) based on dex and level */
    if (!context->p->timed[TMD_PARALYZED] &&
        (randint0(100) < (adj_dex_safe[context->p->state.stat_ind[STAT_DEX]] +
                          context->p->lev))) {
        /* Saving throw message */
        msg("You grab hold of your backpack!");

        /* Occasional "blink" anyway */
        context->blinked = true;

        /* Obvious */
        context->obvious = true;

        /* Done */
        return;
    }

    /* Find an item */
    for (tries = 0; tries < 10; tries++) {
		struct object *obj, *stolen;
		char o_name[80];
		bool split = false;
		bool none_left = false;

        /* Pick an item */
		int index = randint0(z_info->pack_size);

        /* Obtain the item */
        obj = context->p->upkeep->inven[index];

		/* Skip non-objects */
		if (obj == NULL) continue;

        /* Skip artifacts */
        if (obj->artifact) continue;

        /* Get a description */
        object_desc(o_name, sizeof(o_name), obj, ODESC_FULL);

		/* Is it one of a stack being stolen? */
		if (obj->number > 1)
			split = true;

        /* Message */
        msg("%s %s (%c) was stolen!", (split ? "One of your" : "Your"),
			o_name, I2A(index));

        /* Steal and carry */
		stolen = gear_object_for_use(obj, 1, false, &none_left);
        (void)monster_carry(cave, context->mon, stolen);

        /* Obvious */
        context->obvious = true;

        /* Blink away */
        context->blinked = true;

        /* Done */
        break;
    }
}

/**
 * Melee effect handler: Eat the player's food.
 */
static void melee_effect_handler_EAT_FOOD(melee_effect_handler_context_t *context)
{
	/* Steal some food */
	int tries;

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

	for (tries = 0; tries < 10; tries++) {
		/* Pick an item from the pack */
		int index = randint0(z_info->pack_size);
		struct object *obj, *eaten;
		char o_name[80];
		bool none_left = false;

		/* Get the item */
		obj = context->p->upkeep->inven[index];

		/* Skip non-objects */
		if (obj == NULL) continue;

		/* Skip non-food objects */
		if (!tval_is_edible(obj)) continue;

		if (obj->number == 1) {
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
			msg("Your %s (%c) was eaten!", o_name, I2A(index));
		} else {
			object_desc(o_name, sizeof(o_name), obj,
						ODESC_PREFIX | ODESC_BASE);
			msg("One of your %s (%c) was eaten!", o_name,
				I2A(index));
		}

		/* Steal and eat */
		eaten = gear_object_for_use(obj, 1, false, &none_left);
		if (eaten->known)
			object_delete(&eaten->known);
		object_delete(&eaten);

		/* Obvious */
		context->obvious = true;

		/* Done */
		break;
	}
}

/**
 * Melee effect handler: Absorb the player's light.
 */
static void melee_effect_handler_EAT_LIGHT(melee_effect_handler_context_t *context)
{
	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

	/* Drain the light source */
	effect_simple(EF_DRAIN_LIGHT,
			source_monster(context->mon->midx),
			"250+1d250",
			0,
			0,
			0,
			0,
			0,
			&context->obvious);
}

/**
 * Melee effect handler: Attack the player with acid.
 */
static void melee_effect_handler_ACID(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, PROJ_ACID, true);
}

/**
 * Melee effect handler: Attack the player with electricity.
 */
static void melee_effect_handler_ELEC(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, PROJ_ELEC, true);
}

/**
 * Melee effect handler: Attack the player with fire.
 */
static void melee_effect_handler_FIRE(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, PROJ_FIRE, true);
}

/**
 * Melee effect handler: Attack the player with cold.
 */
static void melee_effect_handler_COLD(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, PROJ_COLD, true);
}

/**
 * Melee effect handler: Blind the player.
 */
static void melee_effect_handler_BLIND(melee_effect_handler_context_t *context)
{
	melee_effect_timed(context, TMD_BLIND, 10 + randint1(context->rlev),
					   OF_PROT_BLIND, false, NULL);
}

/**
 * Melee effect handler: Confuse the player.
 */
static void melee_effect_handler_CONFUSE(melee_effect_handler_context_t *context)
{
	melee_effect_timed(context, TMD_CONFUSED, 3 + randint1(context->rlev),
					   OF_PROT_CONF, false, NULL);
}

/**
 * Melee effect handler: Terrify the player.
 */
static void melee_effect_handler_TERRIFY(melee_effect_handler_context_t *context)
{
	melee_effect_timed(context, TMD_AFRAID, 3 + randint1(context->rlev),
					   OF_PROT_FEAR, true, "You stand your ground!");
}

/**
 * Melee effect handler: Paralyze the player.
 */
static void melee_effect_handler_PARALYZE(melee_effect_handler_context_t *context)
{
	/* Hack -- Prevent perma-paralysis via damage */
	if (context->p->timed[TMD_PARALYZED] && (context->damage < 1))
		context->damage = 1;

	melee_effect_timed(context, TMD_PARALYZED, 3 + randint1(context->rlev),
					   OF_FREE_ACT, true, "You resist the effects!");
}

/**
 * Melee effect handler: Drain the player's strength.
 */
static void melee_effect_handler_LOSE_STR(melee_effect_handler_context_t *context)
{
	melee_effect_stat(context, STAT_STR);
}

/**
 * Melee effect handler: Drain the player's intelligence.
 */
static void melee_effect_handler_LOSE_INT(melee_effect_handler_context_t *context)
{
	melee_effect_stat(context, STAT_INT);
}

/**
 * Melee effect handler: Drain the player's wisdom.
 */
static void melee_effect_handler_LOSE_WIS(melee_effect_handler_context_t *context)
{
	melee_effect_stat(context, STAT_WIS);
}

/**
 * Melee effect handler: Drain the player's dexterity.
 */
static void melee_effect_handler_LOSE_DEX(melee_effect_handler_context_t *context)
{
	melee_effect_stat(context, STAT_DEX);
}

/**
 * Melee effect handler: Drain the player's constitution.
 */
static void melee_effect_handler_LOSE_CON(melee_effect_handler_context_t *context)
{
	melee_effect_stat(context, STAT_CON);
}

/**
 * Melee effect handler: Drain all of the player's stats.
 */
static void melee_effect_handler_LOSE_ALL(melee_effect_handler_context_t *context)
{
	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

	/* Damage (stats) */
	effect_simple(EF_DRAIN_STAT, source_monster(context->mon->midx), "0", STAT_STR, 0, 0, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, source_monster(context->mon->midx), "0", STAT_DEX, 0, 0, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, source_monster(context->mon->midx), "0", STAT_CON, 0, 0, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, source_monster(context->mon->midx), "0", STAT_INT, 0, 0, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, source_monster(context->mon->midx), "0", STAT_WIS, 0, 0, 0, 0, &context->obvious);
}

/**
 * Melee effect handler: Cause an earthquake around the player.
 */
static void melee_effect_handler_SHATTER(melee_effect_handler_context_t *context)
{
	/* Obvious */
	context->obvious = true;

	/* Hack -- Reduce damage based on the player armor class */
	context->damage = adjust_dam_armor(context->damage, context->ac);

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

	/* Radius 8 earthquake centered at the monster */
	if (context->damage > 23) {
		effect_simple(EF_EARTHQUAKE, source_monster(context->mon->midx), "0",
					  0, 8, 0, 0, 0, NULL);
	}
}

/**
 * Melee effect handler: Drain the player's experience.
 */
static void melee_effect_handler_EXP_10(melee_effect_handler_context_t *context)
{
	melee_effect_experience(context, 95, damroll(10, 6));
}

/**
 * Melee effect handler: Drain the player's experience.
 */
static void melee_effect_handler_EXP_20(melee_effect_handler_context_t *context)
{
	melee_effect_experience(context, 90, damroll(20, 6));
}

/**
 * Melee effect handler: Drain the player's experience.
 */
static void melee_effect_handler_EXP_40(melee_effect_handler_context_t *context)
{
	melee_effect_experience(context, 75, damroll(40, 6));
}

/**
 * Melee effect handler: Drain the player's experience.
 */
static void melee_effect_handler_EXP_80(melee_effect_handler_context_t *context)
{
	melee_effect_experience(context, 50, damroll(80, 6));
}

/**
 * Melee effect handler: Make the player hallucinate.
 *
 * Note that we don't use melee_effect_timed(), due to the different monster
 * learning function.
 */
static void melee_effect_handler_HALLU(melee_effect_handler_context_t *context)
{
	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Player is dead */
	if (context->p->is_dead)
		return;

	/* Increase "image" */
	if (player_inc_timed(context->p, TMD_IMAGE, 3 + randint1(context->rlev / 2),
						 true, true))
		context->obvious = true;

	/* Learn about the player */
	update_smart_learn(context->mon, context->p, 0, 0, ELEM_CHAOS);
}

/**
 * ------------------------------------------------------------------------
 * Monster blow melee handler selection
 * ------------------------------------------------------------------------ */
melee_effect_handler_f melee_handler_for_blow_effect(const char *name)
{
	static const struct effect_handler_s {
		const char *name;
		melee_effect_handler_f function;
	} effect_handlers[] = {
		{ "NONE", melee_effect_handler_NONE },
		{ "HURT", melee_effect_handler_HURT },
		{ "POISON", melee_effect_handler_POISON },
		{ "DISENCHANT", melee_effect_handler_DISENCHANT },
		{ "DRAIN_CHARGES", melee_effect_handler_DRAIN_CHARGES },
		{ "EAT_GOLD", melee_effect_handler_EAT_GOLD },
		{ "EAT_ITEM", melee_effect_handler_EAT_ITEM },
		{ "EAT_FOOD", melee_effect_handler_EAT_FOOD },
		{ "EAT_LIGHT", melee_effect_handler_EAT_LIGHT },
		{ "ACID", melee_effect_handler_ACID },
		{ "ELEC", melee_effect_handler_ELEC },
		{ "FIRE", melee_effect_handler_FIRE },
		{ "COLD", melee_effect_handler_COLD },
		{ "BLIND", melee_effect_handler_BLIND },
		{ "CONFUSE", melee_effect_handler_CONFUSE },
		{ "TERRIFY", melee_effect_handler_TERRIFY },
		{ "PARALYZE", melee_effect_handler_PARALYZE },
		{ "LOSE_STR", melee_effect_handler_LOSE_STR },
		{ "LOSE_INT", melee_effect_handler_LOSE_INT },
		{ "LOSE_WIS", melee_effect_handler_LOSE_WIS },
		{ "LOSE_DEX", melee_effect_handler_LOSE_DEX },
		{ "LOSE_CON", melee_effect_handler_LOSE_CON },
		{ "LOSE_ALL", melee_effect_handler_LOSE_ALL },
		{ "SHATTER", melee_effect_handler_SHATTER },
		{ "EXP_10", melee_effect_handler_EXP_10 },
		{ "EXP_20", melee_effect_handler_EXP_20 },
		{ "EXP_40", melee_effect_handler_EXP_40 },
		{ "EXP_80", melee_effect_handler_EXP_80 },
		{ "HALLU", melee_effect_handler_HALLU },
		{ NULL, NULL },
	};
	const struct effect_handler_s *current = effect_handlers;

	while (current->name != NULL && current->function != NULL) {
		if (my_stricmp(name, current->name) == 0)
			return current->function;

		current++;
	}

	return NULL;
}
