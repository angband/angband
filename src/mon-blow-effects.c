/*
 * File: mon-blow-effects.c
 * Purpose: Monster melee effects module.
 *
 * Copyright (c) 1997 Ben Harrison, David Reeve Sward, Keldon Jones.
 *               2013 Ben Semmler
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
#include "monster.h"
#include "mon-blow-effects.h"
#include "mon-blow-methods.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-make.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"

/**
 * Do damage as the result of a melee attack that has an elemental aspect.
 *
 * \param context is information for the current attack.
 * \param type is the GF_ constant for the element.
 * \param pure_element should be TRUE if no side effects (mostly a hack for poison).
 */
static void melee_effect_elemental(melee_effect_handler_context_t *context, int type, bool pure_element)
{
	int physical_dam, elemental_dam;

	if (pure_element) {
		/* Obvious */
		context->obvious = TRUE;
	}

	switch (type) {
		case GF_ACID: msg("You are covered in acid!");
			break;
		case GF_ELEC: msg("You are struck by electricity!");
			break;
		case GF_FIRE: msg("You are enveloped in flames!");
			break;
		case GF_COLD: msg("You are covered with frost!");
			break;
	}

	/* Give the player a small bonus to ac for elemental attacks */
	physical_dam = adjust_dam_armor(context->damage, context->ac + 50);

	/* Some attacks do no physical damage */
	if (!monster_blow_method_physical(context->method))
		physical_dam = 0;

	elemental_dam = adjust_dam(type, context->damage, RANDOMISE, 0);

	/* Take the larger of physical or elemental damage */
	context->damage = (physical_dam > elemental_dam) ? physical_dam : elemental_dam;

	if (context->damage > 0) take_hit(context->p, context->damage, context->ddesc);
	if (elemental_dam > 0) inven_damage(context->p, type, MIN(elemental_dam * 5, 300));

	if (pure_element) {
		/* Learn about the player */
		update_smart_learn(context->m_ptr, context->p, 0, type);
	}
}

/**
 * Do damage as the result of a melee attack that has a status effect.
 *
 * \param context is the information for the current attack.
 * \param type is the TMD_ constant for the effect.
 * \param amount is the amount that the timer should be increased by.
 * \param of_flag is the OF_ flag that is passed on to monster learning for this effect.
 * \param attempt_save indicates if a saving throw should be attempted for this effect.
 * \param save_msg is the message that is displayed if the saving throw is successful.
 */
static void melee_effect_timed(melee_effect_handler_context_t *context, int type, int amount, int of_flag, bool attempt_save, const char *save_msg)
{
	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Perform a saving throw if desired. */
	if (attempt_save && randint0(100) < context->p->state.skills[SKILL_SAVE]) {
		if (save_msg != NULL)
			msg(save_msg);

		context->obvious = TRUE;
	}
	else {
		/* Increase timer for type. */
		if (player_inc_timed(context->p, type, amount, TRUE, TRUE))
			context->obvious = TRUE;
	}

	/* Learn about the player */
	update_smart_learn(context->m_ptr, context->p, of_flag, -1);
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
	take_hit(context->p, context->damage, context->ddesc);

	/* Damage (stat) */
	effect_simple(EF_DRAIN_STAT, "0", stat, 0, 0, &context->obvious);
}

/**
 * Do damage as the result of an experience draining melee attack.
 *
 * \param context is the information for the current attack.
 * \param chance is the player's chance of resisting drain if they have OF_HOLD_LIFE.
 * \param drain_amount is the base amount of experience to drain.
 */
static void melee_effect_experience(melee_effect_handler_context_t *context, int chance, int drain_amount)
{
	/* Obvious */
	context->obvious = TRUE;

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);
	update_smart_learn(context->m_ptr, context->p, OF_HOLD_LIFE, -1);

	if (player_of_has(context->p, OF_HOLD_LIFE) && (randint0(100) < chance)) {
		msg("You keep hold of your life force!");
	}
	else {
		s32b d = drain_amount + (context->p->exp/100) * MON_DRAIN_LIFE;
		if (player_of_has(context->p, OF_HOLD_LIFE)) {
			msg("You feel your life slipping away!");
			player_exp_lose(context->p, d / 10, FALSE);
		}
		else {
			msg("You feel your life draining away!");
			player_exp_lose(context->p, d, FALSE);
		}
	}
}

/**
 * Melee effect handler: Hit the player, but don't do any damage.
 */
static void melee_effect_handler_NONE(melee_effect_handler_context_t *context)
{
	/* Hack -- Assume obvious */
	context->obvious = TRUE;

	/* Hack -- No damage */
	context->damage = 0;
}

/**
 * Melee effect handler: Hurt the player with no side effects.
 */
static void melee_effect_handler_HURT(melee_effect_handler_context_t *context)
{
	/* Obvious */
	context->obvious = TRUE;

	/* Hack -- Player armor reduces total damage */
	context->damage = adjust_dam_armor(context->damage, context->ac);

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);
}

/**
 * Melee effect handler: Poison the player.
 *
 * We can't use melee_effect_timed(), because this is both and elemental attack
 * and a status attack. Note the FALSE value for pure_element for
 * melee_effect_elemental().
 */
static void melee_effect_handler_POISON(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, GF_POIS, FALSE);

	/* Take "poison" effect */
	if (player_inc_timed(context->p, TMD_POISONED, 5 + randint1(context->rlev),
						 TRUE, TRUE))
		context->obvious = TRUE;

	/* Learn about the player */
	update_smart_learn(context->m_ptr, context->p, 0, ELEM_POIS);
}

/**
 * Melee effect handler: Disenchant the player.
 */
static void melee_effect_handler_DISENCHANT(melee_effect_handler_context_t *context)
{
	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Allow complete resist */
	if (!player_resists(context->p, ELEM_DISEN))
	{
		/* Apply disenchantment */
		effect_simple(EF_DISENCHANT, "0", 0, 0, 0, &context->obvious);
	}

	/* Learn about the player */
	update_smart_learn(context->m_ptr, context->p, 0, ELEM_DISEN);
}

/**
 * Melee effect handler: Drain charges from the player's inventory.
 */
static void melee_effect_handler_DRAIN_CHARGES(melee_effect_handler_context_t *context)
{
	object_type *o_ptr;
	struct monster *monster = context->m_ptr;
	struct player *player = context->p;
	int item, tries;
	int unpower = 0, newcharge;

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Find an item */
	for (tries = 0; tries < 10; tries++)
	{
		/* Pick an item */
		item = context->p->upkeep->inven[randint0(INVEN_PACK)];

		/* Skip non-objects */
		if (item == NO_OBJECT) continue;

		/* Obtain the item */
		o_ptr = &context->p->gear[item];

		/* Drain charged wands/staves */
		if (tval_can_have_charges(o_ptr))
		{
			/* Charged? */
			if (o_ptr->pval)
			{
				/* Get number of charge to drain */
				unpower = (context->rlev / (o_ptr->kind->level + 2)) + 1;

				/* Get new charge value, don't allow negative */
				newcharge = MAX((o_ptr->pval - unpower),0);

				/* Remove the charges */
				o_ptr->pval = newcharge;
			}
		}

		if (unpower)
		{
			int heal = context->rlev * unpower;

			msg("Energy drains from your pack!");

			context->obvious = TRUE;

			/* Don't heal more than max hp */
			heal = MIN(heal, monster->maxhp - monster->hp);

			/* Heal */
			monster->hp += heal;

			/* Redraw (later) if needed */
			if (player->upkeep->health_who == monster)
				player->upkeep->redraw |= (PR_HEALTH);

			/* Combine the pack */
			player->upkeep->notice |= (PN_COMBINE);

			/* Redraw stuff */
			player->upkeep->redraw |= (PR_INVEN);

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
	struct player *player = context->p;

    /* Take damage */
    take_hit(context->p, context->damage, context->ddesc);

    /* Obvious */
    context->obvious = TRUE;

    /* Saving throw (unless paralyzed) based on dex and level */
    if (!player->timed[TMD_PARALYZED] &&
        (randint0(100) < (adj_dex_safe[player->state.stat_ind[STAT_DEX]]
						  + player->lev)))
    {
        /* Saving throw message */
        msg("You quickly protect your money pouch!");

        /* Occasional blink anyway */
        if (randint0(3)) context->blinked = TRUE;
    }

    /* Eat gold */
    else {
        s32b gold = (player->au / 10) + randint1(25);
        if (gold < 2) gold = 2;
        if (gold > 5000) gold = (player->au / 20) + randint1(3000);
        if (gold > player->au) gold = player->au;
        player->au -= gold;
        if (gold <= 0) {
            msg("Nothing was stolen.");
            return;
        }
        /* Let the player know they were robbed */
        msg("Your purse feels lighter.");
        if (player->au)
            msg("%ld coins were stolen!", (long)gold);
        else
            msg("All of your coins were stolen!");

        /* While we have gold, put it in objects */
        while (gold > 0) {
            int amt;

            /* Create a new temporary object */
            object_type o;
            object_wipe(&o);
            object_prep(&o, money_kind("gold", gold), 0, MINIMISE);

            /* Amount of gold to put in this object */
            amt = gold > MAX_PVAL ? MAX_PVAL : gold;
            o.pval = amt;
            gold -= amt;

            /* Set origin to stolen, so it is not confused with
             * dropped treasure in monster_death */
            o.origin = ORIGIN_STOLEN;

            /* Give the gold to the monster */
            monster_carry(cave, context->m_ptr, &o);
        }

        /* Redraw gold */
        player->upkeep->redraw |= (PR_GOLD);

        /* Blink away */
        context->blinked = TRUE;
    }
}

/**
 * Melee effect handler: Take something from the player's inventory.
 */
static void melee_effect_handler_EAT_ITEM(melee_effect_handler_context_t *context)
{
	int item, tries;

    /* Take damage */
    take_hit(context->p, context->damage, context->ddesc);

    /* Saving throw (unless paralyzed) based on dex and level */
    if (!context->p->timed[TMD_PARALYZED] &&
        (randint0(100) < (adj_dex_safe[context->p->state.stat_ind[STAT_DEX]] +
                          context->p->lev)))
    {
        /* Saving throw message */
        msg("You grab hold of your backpack!");

        /* Occasional "blink" anyway */
        context->blinked = TRUE;

        /* Obvious */
        context->obvious = TRUE;

        /* Done */
        return;
    }

    /* Find an item */
    for (tries = 0; tries < 10; tries++)
    {
		object_type *o_ptr, *i_ptr;
		char o_name[80];
        object_type object_type_body;
		int index = randint0(INVEN_PACK);

        /* Pick an item */
        item = context->p->upkeep->inven[index];

		/* Skip non-objects */
		if (item == NO_OBJECT) continue;

        /* Obtain the item */
        o_ptr = &context->p->gear[item];

        /* Skip artifacts */
        if (o_ptr->artifact) continue;

        /* Get a description */
        object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

        /* Message */
        msg("%s %s (%c) was stolen!",
            ((o_ptr->number > 1) ? "One of your" : "Your"),
            o_name, inven_to_label(index));

        /* Get local object */
        i_ptr = &object_type_body;

        /* Obtain local object */
        object_copy(i_ptr, o_ptr);

        /* Modify number */
        i_ptr->number = 1;

        /* Hack -- If a rod, staff, or wand, allocate total
         * maximum timeouts or charges between those
         * stolen and those missed. -LM-
         */
        distribute_charges(o_ptr, i_ptr, 1);

        /* Carry the object */
        (void)monster_carry(cave, context->m_ptr, i_ptr);

        /* Steal the items */
        inven_item_increase(item, -1);
        inven_item_optimize(item);

        /* Obvious */
        context->obvious = TRUE;

        /* Blink away */
        context->blinked = TRUE;

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
	int item, tries;
	object_type *o_ptr;
	char o_name[80];

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	for (tries = 0; tries < 10; tries++) {
		/* Pick an item from the pack */
		int index = randint0(INVEN_PACK);
		item = context->p->upkeep->inven[index];

		/* Skip non-objects */
		if (item == NO_OBJECT) continue;

		/* Get the item */
		o_ptr = &context->p->gear[item];

		/* Skip non-food objects */
		if (!tval_is_food(o_ptr)) continue;

		if (o_ptr->number == 1) {
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
			msg("Your %s (%c) was eaten!", o_name, inven_to_label(index));
		} else {
			object_desc(o_name, sizeof(o_name), o_ptr,
						ODESC_PREFIX | ODESC_BASE);
			msg("One of your %s (%c) was eaten!", o_name,
				inven_to_label(index));
		}

		/* Steal the items */
		inven_item_increase(item, -1);
		inven_item_optimize(item);

		/* Obvious */
		context->obvious = TRUE;

		/* Done */
		break;
	}
}

/**
 * Melee effect handler: Absorb the player's light.
 */
static void melee_effect_handler_EAT_LIGHT(melee_effect_handler_context_t *context)
{
	object_type *o_ptr;
	int i;

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Get the light - needs streamlining NRM */
	for (i = 0; i < context->p->body.count; i++)
		if (streq("light", context->p->body.slots[i].name)) break;
	o_ptr = &context->p->gear[context->p->body.slots[i].index];

	/* Drain fuel where applicable */
	if (!of_has(o_ptr->flags, OF_NO_FUEL) && (o_ptr->timeout > 0)) {
		/* Reduce fuel */
		o_ptr->timeout -= (250 + randint1(250));
		if (o_ptr->timeout < 1) o_ptr->timeout = 1;

		/* Notice */
		if (!context->p->timed[TMD_BLIND]) {
			msg("Your light dims.");
			context->obvious = TRUE;
		}

		/* Redraw stuff */
		context->p->upkeep->redraw |= (PR_EQUIP);
	}
}

/**
 * Melee effect handler: Attack the player with acid.
 */
static void melee_effect_handler_ACID(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, GF_ACID, TRUE);
}

/**
 * Melee effect handler: Attack the player with electricity.
 */
static void melee_effect_handler_ELEC(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, GF_ELEC, TRUE);
}

/**
 * Melee effect handler: Attack the player with fire.
 */
static void melee_effect_handler_FIRE(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, GF_FIRE, TRUE);
}

/**
 * Melee effect handler: Attack the player with cold.
 */
static void melee_effect_handler_COLD(melee_effect_handler_context_t *context)
{
	melee_effect_elemental(context, GF_COLD, TRUE);
}

/**
 * Melee effect handler: Blind the player.
 */
static void melee_effect_handler_BLIND(melee_effect_handler_context_t *context)
{
	melee_effect_timed(context, TMD_BLIND, 10 + randint1(context->rlev), OF_PROT_BLIND, FALSE, NULL);
}

/**
 * Melee effect handler: Confuse the player.
 */
static void melee_effect_handler_CONFUSE(melee_effect_handler_context_t *context)
{
	melee_effect_timed(context, TMD_CONFUSED, 3 + randint1(context->rlev), OF_PROT_CONF, FALSE, NULL);
}

/**
 * Melee effect handler: Terrify the player.
 */
static void melee_effect_handler_TERRIFY(melee_effect_handler_context_t *context)
{
	melee_effect_timed(context, TMD_AFRAID, 3 + randint1(context->rlev), OF_PROT_FEAR, TRUE, "You stand your ground!");
}

/**
 * Melee effect handler: Paralyze the player.
 */
static void melee_effect_handler_PARALYZE(melee_effect_handler_context_t *context)
{
	/* Hack -- Prevent perma-paralysis via damage */
	if (context->p->timed[TMD_PARALYZED] && (context->damage < 1)) context->damage = 1;

	melee_effect_timed(context, TMD_PARALYZED, 3 + randint1(context->rlev), OF_FREE_ACT, TRUE, "You resist the effects!");
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

	/* Damage (stats) */
	effect_simple(EF_DRAIN_STAT, "0", STAT_STR, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, "0", STAT_DEX, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, "0", STAT_CON, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, "0", STAT_INT, 0, 0, &context->obvious);
	effect_simple(EF_DRAIN_STAT, "0", STAT_WIS, 0, 0, &context->obvious);
}

/**
 * Melee effect handler: Cause an earthquake around the player.
 */
static void melee_effect_handler_SHATTER(melee_effect_handler_context_t *context)
{
	/* Obvious */
	context->obvious = TRUE;

	/* Hack -- Reduce damage based on the player armor class */
	context->damage = adjust_dam_armor(context->damage, context->ac);

	/* Take damage */
	take_hit(context->p, context->damage, context->ddesc);

	/* Radius 8 earthquake centered at the monster */
	if (context->damage > 23) {
		int px_old = context->p->px;
		int py_old = context->p->py;

		effect_simple(EF_EARTHQUAKE, "0", 0, 8, 0, NULL);

		/* Stop the blows if the player is pushed away */
		if ((px_old != context->p->px) ||
			(py_old != context->p->py))
			context->do_break = TRUE;
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

	/* Increase "image" */
	if (player_inc_timed(context->p, TMD_IMAGE, 3 + randint1(context->rlev / 2), TRUE, TRUE))
		context->obvious = TRUE;

	/* Learn about the player */
	update_smart_learn(context->m_ptr, context->p, 0, ELEM_CHAOS);
}

/**
 * Dummy melee effect handler.
 */
static void melee_effect_handler_MAX(melee_effect_handler_context_t *context)
{
	/* Hack -- Do nothing */
}

/**
 * Return a handler for the given effect.
 *
 * Handlers are named after RBE_ constants.
 *
 * \param effect is the RBE_ constant for the effect.
 * \returns a function pointer to handle the effect, or NULL if not found.
 */
melee_effect_handler_f melee_handler_for_blow_effect(monster_blow_effect_t effect)
{
	static const melee_effect_handler_f blow_handlers[] = {
		#define RBE(x, p, e, d) melee_effect_handler_##x,
		#include "list-blow-effects.h"
		#undef RBE
	};

	if (effect >= RBE_MAX)
		return NULL;

	return blow_handlers[effect];
}

/**
 * Return a power modifier for the given effect.
 *
 * Values are in list-blow-effects.h.
 *
 * \param effect is the RBE_ constant for the effect.
 */
int monster_blow_effect_power(monster_blow_effect_t effect)
{
	static const int effect_powers[] = {
		#define RBE(x, p, e, d) p,
		#include "list-blow-effects.h"
		#undef RBE
	};

	if (effect >= RBE_MAX)
		return 0;

	return effect_powers[effect];
}

/**
 * Return a description for the given monster blow effect flags.
 *
 * Returns an sensible placeholder string for an out-of-range flag. Descriptions are in list-blow-effects.h.
 *
 * \param effect is one of the RBE_ flags.
 */
const char *monster_blow_effect_description(monster_blow_effect_t effect)
{
	static const char *r_blow_effect_description[] = {
		#define RBE(x, p, e, d) d,
		#include "list-blow-effects.h"
		#undef RBE
	};

	/* Some blows have no effects, so we do want to return whatever is in the table for RBE_NONE */
	if (effect >= RBE_MAX)
		return "do weird things";

	return r_blow_effect_description[effect];
}

/**
 * Return a power factor for the given effect to evaluate its power.
 *
 * Values are in list-blow-effects.h.
 *
 * \param effect is the RBE_ constant for the effect.
 */
int monster_blow_effect_eval(monster_blow_effect_t effect)
{
	static const int effect_evals[] = {
		#define RBE(x, p, e, d) e,
		#include "list-blow-effects.h"
		#undef RBE
	};

	if (effect >= RBE_MAX)
		return 0;

	return effect_evals[effect];
}

/**
 * Return the RBE_ constant matching the given string.
 *
 * Values are stringified RBE_ constants.
 *
 * \param string contains a value to search for.
 */
monster_blow_effect_t monster_blow_effect_for_string(const char *string)
{
	int i;
	static const char *r_info_blow_effect[] = {
		#define RBE(x, p, e, d) #x,
		#include "list-blow-effects.h"
		#undef RBE
	};

	for (i = 0; r_info_blow_effect[i]; i++)
		if (streq(string, r_info_blow_effect[i]))
			break;

	return i;
}

/**
 * Return whether the given effect is valid.
 *
 * \param effect is the RBE_ constant for the effect.
 */
bool monster_blow_effect_is_valid(monster_blow_effect_t effect)
{
	return effect < RBE_MAX;
}
