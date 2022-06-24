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
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-slays.h"
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

typedef enum {
	BLOW_TAG_NONE,
	BLOW_TAG_TARGET,
	BLOW_TAG_OF_TARGET,
	BLOW_TAG_HAS
} blow_tag_t;

static blow_tag_t blow_tag_lookup(const char *tag)
{
	if (strncmp(tag, "target", 6) == 0)
		return BLOW_TAG_TARGET;
	else if (strncmp(tag, "oftarget", 8) == 0)
		return BLOW_TAG_OF_TARGET;
	else if (strncmp(tag, "has", 3) == 0)
		return BLOW_TAG_HAS;
	else
		return BLOW_TAG_NONE;
}

/**
 * Print a monster blow message.
 *
 * We fill in the monster name and/or pronoun where necessary in
 * the message to replace instances of {name} or {pronoun}.
 */
char *monster_blow_method_action(struct blow_method *method, int midx)
{
	const char punct[] = ".!?;:,'";
	char buf[1024] = "\0";
	const char *next;
	const char *s;
	const char *tag;
	const char *in_cursor;
	size_t end = 0;
	struct monster *t_mon = NULL;

	int choice = randint0(method->num_messages);
	struct blow_message *msg = method->messages;

	/* Get the target monster, if any */
	if (midx > 0) {
		t_mon = cave_monster(cave, midx);
	}

	/* Pick a message */
	while (choice--) {
		msg = msg->next;
	}
	in_cursor = msg->act_msg;

	/* Add info to the message */
	next = strchr(in_cursor, '{');
	while (next) {
		/* Copy the text leading up to this { */
		strnfcat(buf, 1024, &end, "%.*s", (int) (next - in_cursor),
			in_cursor);

		s = next + 1;
		while (*s && isalpha((unsigned char) *s)) s++;

		/* Valid tag */
		if (*s == '}') {
			/* Start the tag after the { */
			tag = next + 1;
			in_cursor = s + 1;

			switch (blow_tag_lookup(tag)) {
				case BLOW_TAG_TARGET: {
					char m_name[80];
					if (midx > 0) {
						int mdesc_mode = MDESC_TARG;

						if (!strchr(punct, *in_cursor)) {
							mdesc_mode |= MDESC_COMMA;
						}
						monster_desc(m_name,
							sizeof(m_name), t_mon,
							mdesc_mode);
						strnfcat(buf, sizeof(buf),
							&end, "%s", m_name);
					} else {
						strnfcat(buf, sizeof(buf), &end, "you");
					}
					break;
				}
				case BLOW_TAG_OF_TARGET: {
					char m_name[80];
					if (midx > 0) {
						monster_desc(m_name,
							sizeof(m_name), t_mon,
							MDESC_TARG | MDESC_POSS);
						strnfcat(buf, sizeof(buf), &end, m_name);
					} else {
						strnfcat(buf, sizeof(buf), &end, "your");
					}
					break;
				}
				case BLOW_TAG_HAS: {
					if (midx > 0) {
						strnfcat(buf, sizeof(buf), &end, "has");
					} else {
						strnfcat(buf, sizeof(buf), &end, "have");
					}
					break;
				}

				default: {
					break;
				}
			}
		} else {
			/* An invalid tag, skip it */
			in_cursor = next + 1;
		}

		next = strchr(in_cursor, '{');
	}
	strnfcat(buf, 1024, &end, "%s", in_cursor);
	return string_make(buf);
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

/**
 * Monster steals an item from the player
 */
static void steal_player_item(melee_effect_handler_context_t *context)
{
	int tries;

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
        object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, context->p);

		/* Is it one of a stack being stolen? */
		if (obj->number > 1)
			split = true;

		/* Try to steal */
		if (react_to_slay(obj, context->mon)) {
			/* React to objects that hurt the monster */
			char m_name[80];

			/* Get the monster names (or "it") */
			monster_desc(m_name, sizeof(m_name), context->mon, MDESC_STANDARD);

			/* Fail to steal */
			msg("%s tries to steal %s %s, but fails.", m_name,
				(split ? "one of your" : "your"), o_name);
		} else {
			/* Message */
			msg("%s %s (%c) was stolen!",
				(split ? "One of your" : "Your"), o_name,
				gear_to_label(context->p, obj));

			/* Steal and carry */
			stolen = gear_object_for_use(context->p, obj, 1,
				false, &none_left);
			(void)monster_carry(cave, context->mon, stolen);
		}

        /* Obvious */
        context->obvious = true;

        /* Blink away */
        context->blinked = true;

        /* Done */
        break;
    }
}

/**
 * Get the elemental damage taken by a monster from another monster's melee
 */
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
			hurt_flag = RF_HURT_COLD;
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
	if (hurt_flag) {
		rf_on(lore->flags, hurt_flag);
	}

	if (rf_has(context->t_mon->race->flags, imm_flag)) {
		*hurt_msg = MON_MSG_RESIST_A_LOT;
		*die_msg = MON_MSG_DIE;
		damage = context->damage / 9;
	} else if (rf_has(context->t_mon->race->flags, hurt_flag)) {
		damage = context->damage * 2;
	} else {
		*hurt_msg = MON_MSG_NONE;
		*die_msg = MON_MSG_DIE;
	}

	return damage;
}

/**
 * Deal the actual melee damage from a monster to a target player or monster
 *
 * This function is used in handlers where there is no further processing of
 * a monster after damage, so we always return true for monster targets
 */
static bool monster_damage_target(melee_effect_handler_context_t *context,
								  bool no_further_monster_effect)
{
	/* Take damage */
	if (context->p) {
		take_hit(context->p, context->damage, context->ddesc);
		if (context->p->is_dead) return true;
	} else {
		bool dead = false;
		dead = mon_take_nonplayer_hit(context->damage, context->t_mon,
									  MON_MSG_NONE,	MON_MSG_DIE);
		return (dead || no_further_monster_effect);
	}
	return false;
}

/**
 * ------------------------------------------------------------------------
 * Monster blow multi-effect handlers
 * These are each called by several individual effect handlers
 * ------------------------------------------------------------------------ */
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
	enum mon_messages die_msg = MON_MSG_DIE;

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
			(void) mon_take_nonplayer_hit(context->damage, context->t_mon,
										  hurt_msg, die_msg);
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
	if (monster_damage_target(context, false)) return;

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
			mon_inc_timed(context->t_mon, mon_tmd_effect, amount, 0);
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
		if (player_inc_timed(context->p, type, amount, true, true,
				true)) {
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
	if (monster_damage_target(context, true)) return;

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
		(void) mon_take_nonplayer_hit(context->damage, context->t_mon,
									  MON_MSG_NONE, MON_MSG_DIE);
		return;
	}

	if (player_of_has(context->p, OF_HOLD_LIFE) && (randint0(100) < chance)) {
		msg("You keep hold of your life force!");
	} else {
		int32_t d = drain_amount +
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
	context->obvious = true;
	context->damage = 0;
}

/**
 * Melee effect handler: Hurt the player with no side effects.
 */
static void melee_effect_handler_HURT(melee_effect_handler_context_t *context)
{
	/* Obvious */
	context->obvious = true;

	/* Armor reduces total damage */
	context->damage = adjust_dam_armor(context->damage, context->ac);

	/* Take damage */
	(void) monster_damage_target(context, true);
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

	/* Player is dead or not attacked */
	if (!context->p || context->p->is_dead)
		return;

	/* Take "poison" effect */
	if (player_inc_timed(context->p, TMD_POISONED,
			5 + randint1(context->rlev), true, true, true)) {
		context->obvious = true;
	}

	/* Learn about the player */
	update_smart_learn(context->mon, context->p, 0, 0, ELEM_POIS);
}

/**
 * Melee effect handler: Disenchant the player.
 */
static void melee_effect_handler_DISENCHANT(melee_effect_handler_context_t *context)
{
	/* Take damage */
	if (monster_damage_target(context, true)) return;

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
	if (monster_damage_target(context, true)) return;

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
	if (monster_damage_target(context, true)) return;

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
        int32_t gold = (current_player->au / 10) + randint1(25);
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
            obj->origin_depth = convert_depth_to_origin(current_player->depth);

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
    /* Take damage */
	if (monster_damage_target(context, false)) return;

	/* Steal from player or monster */
	if (context->p) {
		int chance = adj_dex_safe[context->p->state.stat_ind[STAT_DEX]] +
			context->p->lev;

		/* Saving throw (unless paralyzed) based on dex and level */
		if (!context->p->timed[TMD_PARALYZED] && (randint0(100) < chance)) {
			/* Saving throw message */
			msg("You grab hold of your backpack!");

			/* Occasional "blink" anyway */
			context->blinked = true;

			/* Obvious */
			context->obvious = true;

			/* Done */
			return;
		}

		/* Try to steal an item */
		steal_player_item(context);
	} else {
		assert(context->t_mon);
		steal_monster_item(context->t_mon, context->mon->midx);
		context->obvious = true;
	}
}

/**
 * Melee effect handler: Eat the player's food.
 */
static void melee_effect_handler_EAT_FOOD(melee_effect_handler_context_t *context)
{
	int tries;

	/* Take damage */
	if (monster_damage_target(context, true)) return;

	/* Steal some food */
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
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE,
				context->p);
			msg("Your %s (%c) was eaten!", o_name,
				gear_to_label(context->p, obj));
		} else {
			object_desc(o_name, sizeof(o_name), obj,
				ODESC_PREFIX | ODESC_BASE, context->p);
			msg("One of your %s (%c) was eaten!", o_name,
				gear_to_label(context->p, obj));
		}

		/* Steal and eat */
		eaten = gear_object_for_use(context->p, obj, 1, false,
			&none_left);
		if (eaten->known)
			object_delete(player->cave, NULL, &eaten->known);
		object_delete(cave, player->cave, &eaten);

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
	if (monster_damage_target(context, true)) return;

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
	if (context->p && context->p->timed[TMD_PARALYZED] && (context->damage < 1))
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
	if (monster_damage_target(context, true)) return;

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
	if (monster_damage_target(context, false)) return;

	/* Earthquake centered at the monster, radius damage-determined */
	if (context->damage > 23) {
		int radius = context->damage / 12;
		effect_simple(EF_EARTHQUAKE, source_monster(context->mon->midx), "0",
					  0, radius, 0, 0, 0, NULL);
	}

	/* Chance of knockback */
	if ((context->damage > 100)) {
		int value = context->damage - 100;
		if (randint1(value) > 40) {
			int dist = 1 + value / 40;
			if (context->p) {
				thrust_away(context->mon->grid, context->p->grid, dist);
			} else {
				thrust_away(context->mon->grid, context->t_mon->grid, dist);
			}
		}
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
	if (monster_damage_target(context, true)) return;

	/* Increase "image" */
	if (player_inc_timed(context->p, TMD_IMAGE,
			3 + randint1(context->rlev / 2), true, true, true))
		context->obvious = true;

	/* Learn about the player */
	update_smart_learn(context->mon, context->p, 0, 0, ELEM_CHAOS);
}

/**
 * Melee effect handler: Give the player Black Breath.
 *
 * Note that we don't use melee_effect_timed(), as this is unresistable.
 */
static void melee_effect_handler_BLACK_BREATH(melee_effect_handler_context_t *context)
{
	/* Take damage */
	if (monster_damage_target(context, true)) return;

	/* Increase Black Breath counter a *small* amount, maybe */
	if (one_in_(5) && player_inc_timed(context->p, TMD_BLACKBREATH,
			context->damage / 10, true, true, false)) {
		context->obvious = true;
	}
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
		{ "BLACK_BREATH", melee_effect_handler_BLACK_BREATH },
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
