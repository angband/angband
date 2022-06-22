/**
 * \file effect-handler-general.c
 * \brief Handler functions for general effects
 *
 * Copyright (c) 2007 Andi Sidwell
 * Copyright (c) 2016 Ben Semmler, Nick McConnell
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

#include "cave.h"
#include "effect-handler.h"
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-predicate.h"
#include "mon-summon.h"
#include "mon-util.h"
#include "obj-chest.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "source.h"
#include "target.h"
#include "trap.h"


/**
 * Set value for a chain of effects
 */
static int set_value = 0;

int effect_calculate_value(effect_handler_context_t *context, bool use_boost)
{
	int final = 0;

	if (set_value) {
		return set_value;
	}

	if (context->value.base > 0 ||
		(context->value.dice > 0 && context->value.sides > 0)) {
		final = context->value.base +
			damroll(context->value.dice, context->value.sides);
	}

	/* Device boost */
	if (use_boost) {
		final *= (100 + context->boost);
		final /= 100;
	}

	return final;
}

/**
 * Stat adjectives
 */
static const char *desc_stat(int stat, bool positive)
{
	struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_STAT, stat);
	if (positive) {
		return prop->adjective;
	}
	return prop->neg_adj;
}

/**
 * Check for monster targeting another monster
 */
struct monster *monster_target_monster(effect_handler_context_t *context)
{
	if (context->origin.what == SRC_MONSTER) {
		struct monster *mon = cave_monster(cave, context->origin.which.monster);
		if (!mon) return NULL;
		if (mon->target.midx > 0) {
			struct monster *t_mon = cave_monster(cave, mon->target.midx);
			assert(t_mon);
			return t_mon;
		}
	}
	return NULL;
}

/**
 * Check that a grid is sufficient for use as teleport destination.
 *
 * \param c is the chunk to examine.
 * \param grid is the grid to test.
 * \param is_player_moving is true if a player is being teleported; it is
 * false if a monster is being teleported.
 * \return true if the specified grid is sufficient for use as a telepoort
 * destination; otherwise, return false
 *
 * In 4.2.4, the sufficient requirements were a floor grid with no players
 * or monsters, no player traps, no webs, and no objects.  Post 4.2.4,
 * the requirements are:
 *     1) passable but not damaging nor automatically triggers a transition
 *         to a different level or environment (i.e. a shop)
 *     2) does not already have a player or monster
 *     3) does not have webs
 *     3) if a player is moving, it does not have player traps
 *     4) if a monster is moving, it does not have a glyph of warding
 * There's some discussion here,
 * http://angband.oook.cz/forum/showthread.php?t=11066
 */
static bool has_teleport_destination_prereqs(struct chunk *c, struct loc grid,
		bool is_player_moving)
{
	if (is_player_moving) {
		if (!square_ispassable(c, grid)) {
			return false;
		}
		if (square_isplayertrap(c, grid)) {
			return false;
		}
	} else {
		if (!square_is_monster_walkable(c, grid)) {
			return false;
		}
		if (square_iswarded(c, grid)) {
			return false;
		}
	}
	if (square(c, grid)->mon
			|| square_isdamaging(c, grid)
			|| square_iswebbed(c, grid)
			|| square_isshop(c, grid)) {
		return false;
	}
	return true;
}

/**
 * Selects items that have at least one removable curse.
 */
static bool item_tester_uncursable(const struct object *obj)
{
	struct curse_data *c = obj->known->curses;
	if (c) {
		size_t i;
		for (i = 1; i < z_info->curse_max; i++) {
			if (c[i].power < 100) {
				return true;
			}
		}
	}
    return false;
}

/**
 * Attempts to remove a curse from an object.
 */
static bool uncurse_object(struct object *obj, int strength, char *dice_string)
{
	int index = 0;

	if (get_curse(&index, obj, dice_string)) {
		struct curse_data curse = obj->curses[index];
		char o_name[80];

		if (curse.power >= 100) {
			/* Curse is permanent */
			return false;
		} else if (strength >= curse.power) {
			/* Successfully removed this curse */
			remove_object_curse(obj->known, index, false);
			remove_object_curse(obj, index, true);
		} else if (!of_has(obj->flags, OF_FRAGILE)) {
			/* Failure to remove, object is now fragile */
			object_desc(o_name, sizeof(o_name), obj, ODESC_FULL,
				player);
			msgt(MSG_CURSED, "The spell fails; your %s is now fragile.", o_name);
			of_on(obj->flags, OF_FRAGILE);
			player_learn_flag(player, OF_FRAGILE);
		} else if (one_in_(4)) {
			/* Failure - unlucky fragile object is destroyed */
			struct object *destroyed;
			bool none_left = false;
			msg("There is a bang and a flash!");
			take_hit(player, damroll(5, 5), "Failed uncursing");
			if (object_is_carried(player, obj)) {
				destroyed = gear_object_for_use(player, obj,
					1, false, &none_left);
				if (destroyed->artifact) {
					/* Artifacts are marked as lost */
					history_lose_artifact(player, destroyed->artifact);
				}
				object_delete(player->cave, NULL, &destroyed->known);
				object_delete(cave, player->cave, &destroyed);
			} else {
				square_delete_object(cave, obj->grid, obj, true, true);
			}
		} else {
			/* Non-destructive failure */
			msg("The removal fails.");
		}
	} else {
		return false;
	}
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->update |= (PU_BONUS);
	player->upkeep->redraw |= (PR_EQUIP | PR_INVEN);
	return true;
}

/**
 * Selects items that have at least one unknown rune.
 */
static bool item_tester_unknown(const struct object *obj)
{
    return object_runes_known(obj) ? false : true;
}

/**
 * Used by the enchant() function (chance of failure)
 */
static const int enchant_table[16] =
{
	0, 10,  20, 40, 80,
	160, 280, 400, 550, 700,
	800, 900, 950, 970, 990,
	1000
};

/**
 * Tries to increase an items bonus score, if possible.
 *
 * \returns true if the bonus was increased
 */
static bool enchant_score(int16_t *score, bool is_artifact)
{
	int chance;

	/* Artifacts resist enchantment half the time */
	if (is_artifact && randint0(100) < 50) return false;

	/* Figure out the chance to enchant */
	if (*score < 0) chance = 0;
	else if (*score > 15) chance = 1000;
	else chance = enchant_table[*score];

	/* If we roll less-than-or-equal to chance, it fails */
	if (randint1(1000) <= chance) return false;

	/* Increment the score */
	++*score;

	return true;
}

/**
 * Helper function for enchant() which tries increasing an item's bonuses
 *
 * \returns true if a bonus was increased
 */
static bool enchant2(struct object *obj, int16_t *score)
{
	bool result = false;
	bool is_artifact = obj->artifact ? true : false;
	if (enchant_score(score, is_artifact)) result = true;
	return result;
}

/**
 * Enchant an item
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting, and a
 * flag of what to try enchanting.  Artifacts resist enchantment some of the
 * time. Also, any enchantment attempt (even unsuccessful) kicks off a parallel
 * attempt to uncurse a cursed item.
 *
 * Note that an item can technically be enchanted all the way to +15 if you
 * wait a very, very, long time.  Going from +9 to +10 only works about 5% of
 * the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and the larger
 * the pile, the lower the chance of success.
 *
 * \returns true if the item was changed in some way
 */
static bool enchant(struct object *obj, int n, int eflag)
{
	int i, prob;
	bool res = false;

	/* Large piles resist enchantment */
	prob = obj->number * 100;

	/* Missiles are easy to enchant */
	if (tval_is_ammo(obj)) prob = prob / 20;

	/* Try "n" times */
	for (i = 0; i < n; i++)
	{
		/* Roll for pile resistance */
		if (prob > 100 && randint0(prob) >= 100) continue;

		/* Try the three kinds of enchantment we can do */
		if ((eflag & ENCH_TOHIT) && enchant2(obj, &obj->to_h)) res = true;
		if ((eflag & ENCH_TODAM) && enchant2(obj, &obj->to_d)) res = true;
		if ((eflag & ENCH_TOAC)  && enchant2(obj, &obj->to_a)) res = true;
	}

	/* Update knowledge */
	assert(obj->known);
	obj->known->to_h = obj->to_h;
	obj->known->to_d = obj->to_d;
	obj->known->to_a = obj->to_a;

	/* Failure */
	if (!res) return (false);

	/* Recalculate bonuses, gear */
	player->upkeep->update |= (PU_BONUS | PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );

	/* Success */
	return (true);
}

/**
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns true if attempted, false if cancelled
 *
 * Enchanting with the TOBOTH flag will try to enchant
 * both to_hit and to_dam with the same flag.  This
 * may not be the most desirable behavior (ACB).
 */
static bool enchant_spell(int num_hit, int num_dam, int num_ac, struct command *cmd)
{
	bool okay = false;

	struct object *obj;

	char o_name[80];

	const char *q, *s;
	int itemmode = (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR);
	item_tester filter = num_ac ? tval_is_armor : tval_is_weapon;

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (cmd) {
		if (cmd_get_item(cmd, "tgtitem", &obj, q, s, filter,
				itemmode)) {
			return false;
		}
	} else if (!get_item(&obj, q, s, 0, filter, itemmode))
		return false;

	/* Description */
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE, player);

	/* Describe */
	msg("%s %s glow%s brightly!",
		(object_is_carried(player, obj) ? "Your" : "The"), o_name,
			   ((obj->number > 1) ? "" : "s"));

	/* Enchant */
	if (num_dam && enchant(obj, num_hit, ENCH_TOBOTH)) okay = true;
	else if (enchant(obj, num_hit, ENCH_TOHIT)) okay = true;
	else if (enchant(obj, num_dam, ENCH_TODAM)) okay = true;
	if (enchant(obj, num_ac, ENCH_TOAC)) okay = true;

	/* Failure */
	if (!okay) {
		event_signal(EVENT_INPUT_FLUSH);

		/* Message */
		msg("The enchantment failed.");
	}

	/* Something happened */
	return (true);
}

/**
 * Brand weapons (or ammo)
 *
 * Turns the (non-magical) object into an ego-item of 'brand_type'.
 */
static void brand_object(struct object *obj, const char *name)
{
	int i;
	struct ego_item *ego;
	bool ok = false;

	/* You can never modify artifacts, ego items or worthless items */
	if (obj && obj->kind->cost && !obj->artifact && !obj->ego) {
		char o_name[80];
		char brand[20];

		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE, player);
		strnfmt(brand, sizeof(brand), "of %s", name);

		/* Describe */
		msg("The %s %s surrounded with an aura of %s.", o_name,
			(obj->number > 1) ? "are" : "is", name);

		/* Get the right ego type for the object */
		for (i = 0; i < z_info->e_max; i++) {
			ego = &e_info[i];

			/* Match the name */
			if (!ego->name) continue;
			if (streq(ego->name, brand)) {
				struct poss_item *poss;
				for (poss = ego->poss_items; poss; poss = poss->next)
					if (poss->kidx == obj->kind->kidx)
						ok = true;
			}
			if (ok) break;
		}

		assert(ok);

		/* Make it an ego item */
		obj->ego = &e_info[i];
		ego_apply_magic(obj, 0);
		player_know_object(player, obj);

		/* Update the gear */
		player->upkeep->update |= (PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

		/* Enchant */
		enchant(obj, randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);
	} else {
		event_signal(EVENT_INPUT_FLUSH);
		msg("The branding failed.");
	}
}

/**
 * ------------------------------------------------------------------------
 * Effect handlers
 * ------------------------------------------------------------------------ */
/**
 * Dummy effect, to tell the effect code to pick one of the next
 * context->value.base effects at random.
 */
bool effect_handler_RANDOM(effect_handler_context_t *context)
{
	return true;
}

/**
 * Feed the player, or set their satiety level.
 */
bool effect_handler_NOURISH(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	amount *= z_info->food_value;
	if (context->subtype == 0) {
		/* Increase food level by amount */
		player_inc_timed(player, TMD_FOOD, MAX(amount, 0), false,
			context->origin.what != SRC_PLAYER || !context->aware,
			false);
	} else if (context->subtype == 1) {
		/* Decrease food level by amount */
		player_dec_timed(player, TMD_FOOD, MAX(amount, 0), false,
			context->origin.what != SRC_PLAYER || !context->aware);
	} else if (context->subtype == 2) {
		/* Set food level to amount, vomiting if necessary */
		bool message = player->timed[TMD_FOOD] > amount;
		if (message) {
			msg("You vomit!");
		}
		player_set_timed(player, TMD_FOOD, MAX(amount, 0), false,
			context->origin.what != SRC_PLAYER || !context->aware);
	} else if (context->subtype == 3) {
		/* Increase food level to amount if needed */
		if (player->timed[TMD_FOOD] < amount) {
			player_set_timed(player, TMD_FOOD, MAX(amount + 1, 0),
				false, context->origin.what != SRC_PLAYER
				|| !context->aware);
		}
	} else {
		return false;
	}
	context->ident = true;
	return true;
}

bool effect_handler_CRUNCH(effect_handler_context_t *context)
{
	if (one_in_(2))
		msg("It's crunchy.");
	else
		msg("It nearly breaks your tooth!");
	context->ident = true;
	return true;
}

/**
 * Cure a player status condition.
 */
bool effect_handler_CURE(effect_handler_context_t *context)
{
	int type = context->subtype;
	(void) player_clear_timed(player, type, true,
		context->origin.what != SRC_PLAYER || !context->aware);
	context->ident = true;
	return true;
}

/**
 * Set a (positive or negative) player status condition.
 */
bool effect_handler_TIMED_SET(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	player_set_timed(player, context->subtype, MAX(amount, 0), true,
		context->origin.what != SRC_PLAYER || !context->aware);
	context->ident = true;
	return true;

}

/**
 * Extend a (positive or negative) player status condition.
 * If context->other is set, increase by that amount if the player already
 * has the status
 */
bool effect_handler_TIMED_INC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	struct monster *t_mon = monster_target_monster(context);
	struct loc decoy = cave_find_decoy(cave);

	context->ident = true;

	/* Destroy decoy if it's a monster attack */
	if (cave->mon_current > 0 && decoy.y && decoy.x) {
		square_destroy_decoy(cave, decoy);
		return true;
	}

	/* Check for monster targeting another monster */
	if (t_mon) {
		int mon_tmd_effect = -1;

		/* Will do until monster and player timed effects are fused */
		switch (context->subtype) {
			case TMD_CONFUSED: {
				mon_tmd_effect = MON_TMD_CONF;
				break;
			}
			case TMD_SLOW: {
				mon_tmd_effect = MON_TMD_SLOW;
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
			case TMD_AMNESIA: {
				mon_tmd_effect = MON_TMD_SLEEP;
				break;
			}
			default: {
				break;
			}
		}
		if (mon_tmd_effect >= 0) {
			mon_inc_timed(t_mon, mon_tmd_effect, MAX(amount, 0), 0);
		}
		return true;
	}

	if (!player->timed[context->subtype] || !context->other) {
		player_inc_timed(player, context->subtype, MAX(amount, 0), true,
			context->origin.what != SRC_PLAYER || !context->aware,
			true);
	} else {
		player_inc_timed(player, context->subtype, context->other, true,
			context->origin.what != SRC_PLAYER || !context->aware,
			true);
	}
	return true;
}

/**
 * Extend a (positive or negative) player status condition unresistably.
 * If context->other is set, increase by that amount if the player already
 * has the status
 */
bool effect_handler_TIMED_INC_NO_RES(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);

	if (!player->timed[context->subtype] || !context->other)
		player_inc_timed(player, context->subtype, MAX(amount, 0),
			true,
			context->origin.what != SRC_PLAYER || !context->aware,
			false);
	else
		player_inc_timed(player, context->subtype, context->other, true,
			context->origin.what != SRC_PLAYER || !context->aware,
			false);
	context->ident = true;
	return true;
}

/**
 * Extend a (positive or negative) monster status condition.
 */
bool effect_handler_MON_TIMED_INC(effect_handler_context_t *context)
{
	assert(context->origin.what == SRC_MONSTER);

	int amount = effect_calculate_value(context, false);
	struct monster *mon = cave_monster(cave, context->origin.which.monster);

	if (mon) {
		mon_inc_timed(mon, context->subtype, MAX(amount, 0), 0);
		context->ident = true;
	}

	return true;
}

/**
 * Reduce a (positive or negative) player status condition.
 * If context->other is set, decrease by the current value / context->other
 */
bool effect_handler_TIMED_DEC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	if (context->other)
		amount = player->timed[context->subtype] / context->other;
	(void) player_dec_timed(player, context->subtype, MAX(amount, 0), true,
		context->origin.what != SRC_PLAYER || !context->aware);
	context->ident = true;
	return true;
}

/**
 * Create a glyph.
 */
bool effect_handler_GLYPH(effect_handler_context_t *context)
{
	struct loc decoy = cave_find_decoy(cave);

	/* Always notice */
	context->ident = true;

	/* Only one decoy at a time */
	if (!loc_is_zero(decoy) && (context->subtype == GLYPH_DECOY)) {
		msg("You can only deploy one decoy at a time.");
		return false;
	}

	/* See if the effect works */
	if (!square_istrappable(cave, player->grid)) {
		msg("There is no clear floor on which to cast the spell.");
		return false;
	}

	/* Push objects off the grid */
	if (square_object(cave, player->grid))
		push_object(player->grid);

	/* Create a glyph */
	square_add_glyph(cave, player->grid, context->subtype);

	return true;
}

/**
 * Create a web.
 */
bool effect_handler_WEB(effect_handler_context_t *context)
{
	int rad = 1;
	struct monster *mon = NULL;
	struct loc grid;

	/* Get the monster creating */
	if (cave->mon_current > 0) {
		mon = cave_monster(cave, cave->mon_current);
	} else {
		/* Player can't currently create webs */
		return false;
	}

	/* Always notice */
	context->ident = true;

	/* Increase the radius for higher spell power */
	if (mon->race->spell_power > 40) rad++;
	if (mon->race->spell_power > 80) rad++;

	/* Check within the radius for clear floor */
	for (grid.y = mon->grid.y - rad; grid.y <= mon->grid.y + rad; grid.y++) {
		for (grid.x = mon->grid.x - rad; grid.x <= mon->grid.x + rad; grid.x++){
			if (distance(grid, mon->grid) > rad ||
				!square_in_bounds_fully(cave, grid)) continue;

			/* Require a floor grid with no existing traps or glyphs */
			if (!square_iswebbable(cave, grid)) continue;

			/* Create a web */
			square_add_web(cave, grid);
		}
	}

	return true;
}

/**
 * Restore a stat; the stat index is context->subtype
 */
bool effect_handler_RESTORE_STAT(effect_handler_context_t *context)
{
	int stat = context->subtype;

	/* ID */
	context->ident = true;

	/* Check bounds */
	if (stat < 0 || stat >= STAT_MAX) return false;

	/* Not needed */
	if (player->stat_cur[stat] == player->stat_max[stat])
		return true;

	/* Restore */
	player->stat_cur[stat] = player->stat_max[stat];

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	/* Message */
	msg("You feel less %s.", desc_stat(stat, false));

	return (true);
}

/**
 * Drain a stat temporarily.  The stat index is context->subtype.
 */
bool effect_handler_DRAIN_STAT(effect_handler_context_t *context)
{
	int stat = context->subtype;
	int flag = sustain_flag(stat);

	/* Bounds check */
	if (flag < 0) return false;

	/* ID */
	context->ident = true;

	/* Sustain */
	if (player_of_has(player, flag)) {
		/* Notice effect */
		equip_learn_flag(player, flag);

		/* Message */
		msg("You feel very %s for a moment, but the feeling passes.",
				   desc_stat(stat, false));

		return (true);
	}

	/* Attempt to reduce the stat */
	if (player_stat_dec(player, stat, false)){
		int dam = effect_calculate_value(context, false);

		/* Notice effect */
		equip_learn_flag(player, flag);

		/* Message */
		msgt(MSG_DRAIN_STAT, "You feel very %s.", desc_stat(stat, false));
		if (dam)
			take_hit(player, dam, "stat drain");
	}

	return (true);
}

/**
 * Lose a stat point permanently, in a stat other than the one specified
 * in context->subtype.
 */
bool effect_handler_LOSE_RANDOM_STAT(effect_handler_context_t *context)
{
	int safe_stat = context->subtype;
	int loss_stat = randint1(STAT_MAX - 1);

	/* Avoid the safe stat */
	loss_stat = (loss_stat + safe_stat) % STAT_MAX;

	/* Attempt to reduce the stat */
	if (player_stat_dec(player, loss_stat, true)) {
		msgt(MSG_DRAIN_STAT, "You feel very %s.", desc_stat(loss_stat, false));
	}

	/* ID */
	context->ident = true;

	return (true);
}


/**
 * Gain a stat point.  The stat index is context->subtype.
 */
bool effect_handler_GAIN_STAT(effect_handler_context_t *context)
{
	int stat = context->subtype;

	/* Attempt to increase */
	if (player_stat_inc(player, stat)) {
		msg("You feel very %s!", desc_stat(stat, true));
	}

	/* Notice */
	context->ident = true;

	return (true);
}

/**
 * Restores any drained experience
 */
bool effect_handler_RESTORE_EXP(effect_handler_context_t *context)
{
	/* Restore experience */
	if (player->exp < player->max_exp) {
		/* Message */
		if (context->origin.what != SRC_NONE)
			msg("You feel your life energies returning.");
		player_exp_gain(player, player->max_exp - player->exp);

		/* Recalculate max. hitpoints */
		update_stuff(player);
	}

	/* Did something */
	context->ident = true;

	return (true);
}

/* Note the divisor of 2, a slight hack to simplify food description */
bool effect_handler_GAIN_EXP(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	if (player->exp < PY_MAX_EXP) {
		msg("You feel more experienced.");
		player_exp_gain(player, amount / 2);
	}
	context->ident = true;

	return true;
}

/**
 * Drain some light from the player's light source, if possible
 */
bool effect_handler_DRAIN_LIGHT(effect_handler_context_t *context)
{
	int drain = effect_calculate_value(context, false);

	int light_slot = slot_by_name(player, "light");
	struct object *obj = slot_object(player, light_slot);

	if (obj && !of_has(obj->flags, OF_NO_FUEL) && (obj->timeout > 0)) {
		/* Reduce fuel */
		obj->timeout -= drain;
		if (obj->timeout < 1) obj->timeout = 1;

		/* Notice */
		if (!player->timed[TMD_BLIND]) {
			msg("Your light dims.");
			context->ident = true;
		}

		/* Redraw stuff */
		player->upkeep->redraw |= (PR_EQUIP);
	}

	return true;
}

/**
 * Drain mana from the player, healing the caster.
 */
bool effect_handler_DRAIN_MANA(effect_handler_context_t *context)
{
	int drain = effect_calculate_value(context, false);
	bool monster = context->origin.what != SRC_TRAP;
	char m_name[80];
	struct monster *mon = NULL;
	struct monster *t_mon = monster_target_monster(context);
	struct loc decoy = cave_find_decoy(cave);

	context->ident = true;

	if (monster) {
		assert(context->origin.what == SRC_MONSTER);

		mon = cave_monster(cave, context->origin.which.monster);

		/* Get the monster name (or "it") */
		monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);
	}

	/* Target is another monster - disenchant it */
	if (t_mon) {
		mon_inc_timed(t_mon, MON_TMD_DISEN, MAX(drain, 0), 0);
		return true;
	}

	/* Target was a decoy - destroy it */
	if (decoy.y && decoy.x) {
		square_destroy_decoy(cave, decoy);
		return true;
	}

	/* The player has no mana */
	if (!player->csp) {
		msg("The draining fails.");
		if (monster) {
			update_smart_learn(mon, player, 0, PF_NO_MANA, -1);
		}
		return true;
	}

	/* Drain the given amount if the player has that much, or all of it */
	if (drain >= player->csp) {
		drain = player->csp;
		player->csp = 0;
		player->csp_frac = 0;
	} else {
		player->csp -= drain;
	}

	/* Heal the monster */
	if (monster) {
		if (mon->hp < mon->maxhp) {
			mon->hp += (6 * drain);
			if (mon->hp > mon->maxhp)
				mon->hp = mon->maxhp;

			/* Redraw (later) if needed */
			if (player->upkeep->health_who == mon)
				player->upkeep->redraw |= (PR_HEALTH);

			/* Special message */
			if (monster_is_visible(mon))
				msg("%s appears healthier.", m_name);
		}
	}

	/* Redraw mana */
	player->upkeep->redraw |= PR_MANA;

	return true;
}

bool effect_handler_RESTORE_MANA(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	if (!amount) amount = player->msp;
	if (player->csp < player->msp) {
		player->csp += amount;
		if (player->csp > player->msp) {
			player->csp = player->msp;
			player->csp_frac = 0;
			msg("You feel your head clear.");
		} else
			msg("You feel your head clear somewhat.");
		player->upkeep->redraw |= (PR_MANA);
	}
	context->ident = true;

	return true;
}

/**
 * Attempt to uncurse an object
 */
bool effect_handler_REMOVE_CURSE(effect_handler_context_t *context)
{
	const char *prompt = "Uncurse which item? ";
	const char *rejmsg = "You have no curses to remove.";
	int itemmode = (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR);
	int strength = effect_calculate_value(context, false);
	struct object *obj = NULL;
	char dice_string[20];

	context->ident = true;

	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, prompt,
				rejmsg, item_tester_uncursable, itemmode)) {
			return false;
		}
	} else if (!get_item(&obj, prompt, rejmsg, 0, item_tester_uncursable,
			itemmode))
		return false;

	/* Get the possible dice strings */
	if ((context->value.dice == 1) && context->value.base) {
		strnfmt(dice_string, sizeof(dice_string), "%d+d%d",
				context->value.base, context->value.sides);
	} else if (context->value.dice && context->value.base) {
		strnfmt(dice_string, sizeof(dice_string), "%d+%dd%d",
				context->value.base, context->value.dice, context->value.sides);
	} else if (context->value.dice == 1) {
		strnfmt(dice_string, sizeof(dice_string), "d%d", context->value.sides);
	} else if (context->value.dice) {
		strnfmt(dice_string, sizeof(dice_string), "%dd%d",
				context->value.dice, context->value.sides);
	} else {
		strnfmt(dice_string, sizeof(dice_string), "%d", context->value.base);
	}

	return uncurse_object(obj, strength, dice_string);
}

/**
 * Set word of recall as appropriate
 */
bool effect_handler_RECALL(effect_handler_context_t *context)
{
	int target_depth;
	context->ident = true;

	/* No recall */
	if (OPT(player, birth_no_recall) && !player->total_winner) {
		msg("Nothing happens.");
		return true;
	}

	/* No recall from quest levels with force_descend */
	if (OPT(player, birth_force_descend)
			&& is_quest(player, player->depth)) {
		msg("Nothing happens.");
		return true;
	}

	/* No recall from single combat */
	if (player->upkeep->arena_level) {
		msg("Nothing happens.");
		return true;
	}

	/* Warn the player if they're descending to an unrecallable level */
	target_depth = dungeon_get_next_level(player, player->max_depth, 1);
	if (OPT(player, birth_force_descend) && !(player->depth)
			&& is_quest(player, target_depth)) {
		if (!get_check("Are you sure you want to descend? ")) {
			return false;
		}
	}

	/* Activate recall */
	if (!player->word_recall) {
		/* Reset recall depth */
		if (player->depth > 0) {
			if (player->depth != player->max_depth) {
				if (get_check("Set recall depth to current depth? ")) {
					player->recall_depth = player->max_depth = player->depth;
				}
			} else {
				player->recall_depth = player->max_depth;
			}
		} else {
			if (OPT(player, birth_levels_persist)) {
				/* Persistent levels players get to choose */
				if (!player_get_recall_depth(player)) return false;
			}
		}

		player->word_recall = randint0(20) + 15;
		msg("The air about you becomes charged...");
	} else {
		/* Deactivate recall */
		if (!get_check("Word of Recall is already active.  Do you want to cancel it? "))
			return false;

		player->word_recall = 0;
		msg("A tension leaves the air around you...");
	}

	/* Redraw status line */
	player->upkeep->redraw |= PR_STATUS;
	handle_stuff(player);

	return true;
}

bool effect_handler_DEEP_DESCENT(effect_handler_context_t *context)
{
	int i;

	/* Calculate target depth */
	int target_increment = (4 / z_info->stair_skip) + 1;
	int target_depth = dungeon_get_next_level(player, player->max_depth,
		target_increment);
	for (i = 5; i > 0; i--) {
		if (is_quest(player, target_depth)) break;
		if (target_depth >= z_info->max_depth - 1) break;

		target_depth++;
	}

	if (target_depth > player->depth) {
		msgt(MSG_TPLEVEL, "The air around you starts to swirl...");
		player->deep_descent = 3 + randint1(4);

		/* Redraw status line */
		player->upkeep->redraw |= PR_STATUS;
		handle_stuff(player);
	} else {
		msgt(MSG_TPLEVEL, "You sense a malevolent presence blocking passage to the levels below.");
	}
	context->ident = true;
	return true;
}

bool effect_handler_ALTER_REALITY(effect_handler_context_t *context)
{
	/* Don't allow in single combat arenas. */
	if (player->upkeep->arena_level) return true;
	msg("The world changes!");
	dungeon_change_level(player, player->depth);
	context->ident = true;
	return true;
}

/**
 * Map an area around a point, usually the player.
 * The height to map above and below the player is context->y,
 * the width either side of the player context->x.
 * For player level dependent areas, we use the hack of applying value dice
 * and sides as the height and width.
 */
bool effect_handler_MAP_AREA(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int dist_y = context->y ? context->y : context->value.dice;
	int dist_x = context->x ? context->x : context->value.sides;
	struct loc centre = origin_get_loc(context->origin);

	/* Pick an area to map */
	y1 = centre.y - dist_y;
	y2 = centre.y + dist_y;
	x1 = centre.x - dist_x;
	x2 = centre.x + dist_x;

	/* Drag the co-ordinates into the dungeon */
	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			struct loc grid = loc(x, y);

			/* Some squares can't be mapped */
			if (square_isno_map(cave, grid)) continue;

			/* All non-walls are "checked" */
			if (!square_seemslikewall(cave, grid)) {
				if (!square_in_bounds_fully(cave, grid)) continue;

				/* Memorize normal features */
				if (!square_isfloor(cave, grid))
					square_memorize(cave, grid);

				/* Memorize known walls */
				for (i = 0; i < 8; i++) {
					int yy = y + ddy_ddd[i];
					int xx = x + ddx_ddd[i];

					/* Memorize walls (etc) */
					if (square_seemslikewall(cave, loc(xx, yy)))
						square_memorize(cave, loc(xx, yy));
				}
			}

			/* Forget unprocessed, unknown grids in the mapping area */
			if (square_isnotknown(cave, grid))
				square_forget(cave, grid);
		}
	}

	/* Unmark grids */
	for (y = y1 - 1; y < y2 + 1; y++) {
		for (x = x1 - 1; x < x2 + 1; x++) {
			struct loc grid = loc(x, y);
			if (!square_in_bounds(cave, grid)) continue;
			square_unmark(cave, grid);
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw whole map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);

	/* Notice */
	context->ident = true;

	return true;
}

/**
 * Map an area around the recently detected monsters.
 * The height to map above and below each monster is context->y,
 * the width either side of each monster context->x.
 * For player level dependent areas, we use the hack of applying value dice
 * and sides as the height and width.
 */
bool effect_handler_READ_MINDS(effect_handler_context_t *context)
{
	int i;
	int dist_y = context->y ? context->y : context->value.dice;
	int dist_x = context->x ? context->x : context->value.sides;
	bool found = false;

	/* Scan monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Detect all appropriate monsters */
		if (mflag_has(mon->mflag, MFLAG_MARK)) {
			/* Map around it */
			effect_simple(EF_MAP_AREA, source_monster(i), "0", 0, 0, 0,
						  dist_y, dist_x, NULL);
			found = true;
		}
	}

	if (found) {
		msg("Images form in your mind!");
		context->ident = true;
	}

	return true;
}

/**
 * Detect traps around the player.  The height to detect above and below the
 * player is context->y, the width either side of the player context->x.
 */
bool effect_handler_DETECT_TRAPS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;

	bool detect = false;

	struct object *obj;

	/* Pick an area to detect */
	y1 = player->grid.y - context->y;
	y2 = player->grid.y + context->y;
	x1 = player->grid.x - context->x;
	x2 = player->grid.x + context->x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;


	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			struct loc grid = loc(x, y);

			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Detect traps */
			if (square_isplayertrap(cave, grid))
				/* Reveal trap */
				if (square_reveal_trap(cave, grid, true, false))
					detect = true;

			/* Scan all objects in the grid to look for traps on chests */
			for (obj = square_object(cave, grid); obj; obj = obj->next) {
				/* Skip anything not a trapped chest */
				if (!is_trapped_chest(obj)
						|| ignore_item_ok(player, obj)) {
					continue;
				}

				/* Identify once */
				if (!obj->known || obj->known->pval != obj->pval) {
					/* Hack - know the pile */
					square_know_pile(cave, grid);

					/* Know the trap */
					obj->known->pval = obj->pval;

					/* We found something to detect */
					detect = true;
				}
			}
			/* Mark as trap-detected */
			sqinfo_on(square(cave, loc(x, y))->info, SQUARE_DTRAP);
		}
	}

	/* Describe */
	if (detect)
		msg("You sense the presence of traps!");

	/* Trap detection always makes you aware, even if no traps are present */
	else
		msg("You sense no traps.");

	/* Notice */
	context->ident = true;

	return true;
}

/**
 * Detect doors around the player.  The height to detect above and below the
 * player is context->y, the width either side of the player context->x.
 */
bool effect_handler_DETECT_DOORS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;

	bool doors = false;

	/* Pick an area to detect */
	y1 = player->grid.y - context->y;
	y2 = player->grid.y + context->y;
	x1 = player->grid.x - context->x;
	x2 = player->grid.x + context->x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			struct loc grid = loc(x, y);

			if (!square_in_bounds_fully(cave, grid)) continue;

			if (square_issecretdoor(cave, grid)) {
				/* Detect secret doors */
				/* Put an actual door */
				place_closed_door(cave, grid);

				/* Memorize */
				square_memorize(cave, grid);
				square_light_spot(cave, grid);

				/* Obvious */
				doors = true;
			} else if (square_isdoor(cave, grid)) {
				/* Detect other types of doors. */
				if (square_isnotknown(cave, grid)) {
					square_memorize(cave, grid);
					square_light_spot(cave, grid);
					doors = true;
				}
			} else if (square_isdoor(player->cave, grid)
					&& square_isnotknown(cave, grid)) {
				/* Forget unknown doors in the mapping area */
				square_forget(cave, grid);
			}
		}
	}

	/* Describe */
	if (doors)
		msg("You sense the presence of doors!");
	else if (context->aware)
		msg("You sense no doors.");

	context->ident = true;

	return true;
}

/**
 * Detect stairs around the player.  The height to detect above and below the
 * player is context->y, the width either side of the player context->x.
 */
bool effect_handler_DETECT_STAIRS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;

	bool stairs = false;

	/* Pick an area to detect */
	y1 = player->grid.y - context->y;
	y2 = player->grid.y + context->y;
	x1 = player->grid.x - context->x;
	x2 = player->grid.x + context->x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			struct loc grid = loc(x, y);

			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Detect stairs */
			if (square_isstairs(cave, grid)) {
				/* Memorize */
				square_memorize(cave, grid);
				square_light_spot(cave, grid);

				/* Obvious */
				stairs = true;
			}
		}
	}

	/* Describe */
	if (stairs)
		msg("You sense the presence of stairs!");
	else if (context->aware)
		msg("You sense no stairs.");

	context->ident = true;
	return true;
}


/**
 * Detect buried gold around the player.  The height to detect above and below
 * the player is context->y, the width either side of the player context->x.
 */
bool effect_handler_DETECT_GOLD(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;

	bool gold_buried = false;

	/* Pick an area to detect */
	y1 = player->grid.y - context->y;
	y2 = player->grid.y + context->y;
	x1 = player->grid.x - context->x;
	x2 = player->grid.x + context->x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			struct loc grid = loc(x, y);

			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Magma/Quartz + Known Gold */
			if (square_hasgoldvein(cave, grid)) {
				/* Memorize */
				square_memorize(cave, grid);
				square_light_spot(cave, grid);

				/* Detect */
				gold_buried = true;
			} else if (square_hasgoldvein(player->cave, grid)) {
				/* Something removed previously seen or
				 * detected buried gold.  Notice the change. */
				square_forget(cave, grid);
			}
		}
	}

	/* Message unless we're silently detecting */
	if (context->origin.what != SRC_NONE) {
		if (gold_buried) {
			msg("You sense the presence of buried treasure!");
		} else if (context->aware) {
			msg("You sense no buried treasure.");
		}
	}

	context->ident = true;
	return true;
}

/**
 * This is a helper for effect_handler_SENSE_OBJECTS and
 * effect_handler_DETECT_OBJECTS to remove remembered objects at locations
 * sensed or detected as empty.
 */
static void forget_remembered_objects(struct chunk *c, struct chunk *knownc, struct loc grid)
{
	struct object *obj = square_object(knownc, grid);

	while (obj) {
		struct object *next = obj->next;
		struct object *original = c->objects[obj->oidx];

		assert(original);
		square_excise_object(knownc, grid, obj);
		obj->grid = loc(0, 0);

		/* Delete objects which no longer exist anywhere */
		if (obj->notice & OBJ_NOTICE_IMAGINED) {
			delist_object(knownc, obj);
			object_delete(player->cave, NULL, &obj);
			original->known = NULL;
			delist_object(c, original);
			object_delete(cave, player->cave, &original);
		}
		obj = next;
	}
}

/**
 * Sense objects around the player.  The height to sense above and below the
 * player is context->y, the width either side of the player context->x
 */
bool effect_handler_SENSE_OBJECTS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;

	bool objects = false;

	/* Pick an area to sense */
	y1 = player->grid.y - context->y;
	y2 = player->grid.y + context->y;
	x1 = player->grid.x - context->x;
	x2 = player->grid.x + context->x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the area for objects */
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			struct loc grid = loc(x, y);
			struct object *obj = square_object(cave, grid);

			if (!obj) {
				/* If empty, remove any remembered objects. */
				forget_remembered_objects(cave, player->cave, grid);
				continue;
			}

			/* Notice an object is detected */
			objects = true;

			/* Mark the pile as aware */
			square_sense_pile(cave, grid);
		}
	}

	if (objects)
		msg("You sense the presence of objects!");
	else if (context->aware)
		msg("You sense no objects.");

	/* Redraw whole map, monster list */
	player->upkeep->redraw |= PR_ITEMLIST;

	context->ident = true;
	return true;
}

/**
 * Detect objects around the player.  The height to detect above and below the
 * player is context->y, the width either side of the player context->x
 */
bool effect_handler_DETECT_OBJECTS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;

	bool objects = false;

	/* Pick an area to detect */
	y1 = player->grid.y - context->y;
	y2 = player->grid.y + context->y;
	x1 = player->grid.x - context->x;
	x2 = player->grid.x + context->x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the area for objects */
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			struct loc grid = loc(x, y);
			struct object *obj = square_object(cave, grid);

			if (!obj) {
				/* If empty, remove any remembered objects. */
				forget_remembered_objects(cave, player->cave, grid);
				continue;
			}

			/* Notice an object is detected */
			if (!ignore_item_ok(player, obj)) {
				objects = true;
			}

			/* Mark the pile as seen */
			square_know_pile(cave, grid);
		}
	}

	if (objects)
		msg("You detect the presence of objects!");
	else if (context->aware)
		msg("You detect no objects.");

	/* Redraw whole map, monster list */
	player->upkeep->redraw |= PR_ITEMLIST;

	context->ident = true;
	return true;
}

/**
 * Detect monsters which satisfy the given predicate around the player.
 * The height to detect above and below the player is y_dist,
 * the width either side of the player x_dist.
 */
static bool detect_monsters(int y_dist, int x_dist, monster_predicate pred)
{
	int i, x, y;
	int x1, x2, y1, y2;

	bool monsters = false;

	/* Set the detection area */
	y1 = player->grid.y - y_dist;
	y2 = player->grid.y + y_dist;
	x1 = player->grid.x - x_dist;
	x2 = player->grid.x + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Location */
		y = mon->grid.y;
		x = mon->grid.x;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect all appropriate, obvious monsters */
		if (pred(mon) && !monster_is_camouflaged(mon)) {
			/* Detect the monster */
			mflag_on(mon->mflag, MFLAG_MARK);
			mflag_on(mon->mflag, MFLAG_SHOW);

			/* Note invisible monsters */
			if (monster_is_invisible(mon)) {
				struct monster_lore *lore = get_lore(mon->race);
				rf_on(lore->flags, RF_INVISIBLE);
			}

			/* Update monster recall window */
			if (player->upkeep->monster_race == mon->race)
				/* Redraw stuff */
				player->upkeep->redraw |= (PR_MONSTER);

			/* Update the monster */
			update_mon(mon, cave, false);

			/* Detect */
			monsters = true;
		}
	}

	return monsters;
}

/**
 * Detect living monsters around the player.  The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_DETECT_LIVING_MONSTERS(effect_handler_context_t *context)
{
	bool monsters = detect_monsters(context->y, context->x, monster_is_living);

	if (monsters)
		msg("You sense life!");
	else if (context->aware)
		msg("You sense no life.");

	context->ident = true;
	return true;
}


/**
 * Detect visible monsters around the player; note that this means monsters
 * which are in principle visible, not monsters the player can currently see.
 *
 * The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_DETECT_VISIBLE_MONSTERS(effect_handler_context_t *context)
{
	bool monsters = detect_monsters(context->y, context->x,
									monster_is_not_invisible);

	if (monsters)
		msg("You sense the presence of monsters!");
	else if (context->aware)
		msg("You sense no monsters.");

	context->ident = true;
	return true;
}


/**
 * Detect invisible monsters around the player.  The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_DETECT_INVISIBLE_MONSTERS(effect_handler_context_t *context)
{
	bool monsters = detect_monsters(context->y, context->x,
									monster_is_invisible);

	if (monsters)
		msg("You sense the presence of invisible creatures!");
	else if (context->aware)
		msg("You sense no invisible creatures.");

	context->ident = true;
	return true;
}

/**
 * Detect monsters susceptible to fear around the player.  The height to detect
 * above and below the player is context->value.dice, the width either side of
 * the player context->value.sides.
 */
bool effect_handler_DETECT_FEARFUL_MONSTERS(effect_handler_context_t *context)
{
	bool monsters = detect_monsters(context->y, context->x, monster_is_fearful);

	if (monsters)
		msg("These monsters could provide good sport.");
	else if (context->aware)
		msg("You smell no fear in the air.");

	context->ident = true;
	return true;
}

/**
 * Detect evil monsters around the player.  The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_DETECT_EVIL(effect_handler_context_t *context)
{
	bool monsters = detect_monsters(context->y, context->x, monster_is_evil);

	if (monsters)
		msg("You sense the presence of evil creatures!");
	else if (context->aware)
		msg("You sense no evil creatures.");

	context->ident = true;
	return true;
}

/**
 * Detect monsters possessing a spirit around the player.
 * The height to detect above and below the player is context->value.dice,
 * the width either side of the player context->value.sides.
 */
bool effect_handler_DETECT_SOUL(effect_handler_context_t *context)
{
	bool monsters = detect_monsters(context->y, context->x, monster_has_spirit);

	if (monsters)
		msg("You sense the presence of spirits!");
	else if (context->aware)
		msg("You sense no spirits.");

	context->ident = true;
	return true;
}

/**
 * Identify an unknown rune of an item.
 */
bool effect_handler_IDENTIFY(effect_handler_context_t *context)
{
	struct object *obj;
	const char *q, *s;
	int itemmode = (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR);
	bool used = false;

	context->ident = true;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";
	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, q, s,
				item_tester_unknown, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, item_tester_unknown, itemmode))
		return used;

	/* Identify the object */
	object_learn_unknown_rune(player, obj);

	return true;
}


/**
 * Create stairs at the player location
 */
bool effect_handler_CREATE_STAIRS(effect_handler_context_t *context)
{
	context->ident = true;

	/* Only allow stairs to be created on empty floor */
	if (!square_isfloor(cave, player->grid)) {
		msg("There is no empty floor here.");
		return false;
	}

	/* Fails for persistent levels (for now) and arenas */
	if (OPT(player, birth_levels_persist) || player->upkeep->arena_level) {
		msg("Nothing happens!");
		return false;
	}

	/* Push objects off the grid */
	if (square_object(cave, player->grid))
		push_object(player->grid);

	square_add_stairs(cave, player->grid, player->depth);

	return true;
}

/**
 * Apply disenchantment to the player's stuff.
 */
bool effect_handler_DISENCHANT(effect_handler_context_t *context)
{
	int i, count = 0;
	struct object *obj;
	char o_name[80];

	/* Count slots */
	for (i = 0; i < player->body.count; i++) {
		/* Ignore rings, amulets and lights */
		if (slot_type_is(player, i, EQUIP_RING)) continue;
		if (slot_type_is(player, i, EQUIP_AMULET)) continue;
		if (slot_type_is(player, i, EQUIP_LIGHT)) continue;

		/* Count disenchantable slots */
		count++;
	}

	/* Pick one at random */
	for (i = player->body.count - 1; i >= 0; i--) {
		/* Ignore rings, amulets and lights */
		if (slot_type_is(player, i, EQUIP_RING)) continue;
		if (slot_type_is(player, i, EQUIP_AMULET)) continue;
		if (slot_type_is(player, i, EQUIP_LIGHT)) continue;

		if (one_in_(count--)) break;
	}

	/* Notice */
	context->ident = true;

	/* Get the item */
	obj = slot_object(player, i);

	/* No item, nothing happens */
	if (!obj) return true;

	/* Nothing to disenchant */
	if ((obj->to_h <= 0) && (obj->to_d <= 0) && (obj->to_a <= 0))
		return true;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE, player);

	/* Artifacts have a 60% chance to resist */
	if (obj->artifact && (randint0(100) < 60)) {
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!", o_name,
			gear_to_label(player, obj),
			((obj->number != 1) ? "" : "s"));

		return true;
	}

	/* Apply disenchantment, depending on which kind of equipment */
	if (slot_type_is(player, i, EQUIP_WEAPON)
			|| slot_type_is(player, i, EQUIP_BOW)) {
		/* Disenchant to-hit */
		if (obj->to_h > 0) obj->to_h--;
		if ((obj->to_h > 5) && (randint0(100) < 20)) obj->to_h--;
		obj->known->to_h = obj->to_h;

		/* Disenchant to-dam */
		if (obj->to_d > 0) obj->to_d--;
		if ((obj->to_d > 5) && (randint0(100) < 20)) obj->to_d--;
		obj->known->to_d = obj->to_d;
	} else {
		/* Disenchant to-ac */
		if (obj->to_a > 0) obj->to_a--;
		if ((obj->to_a > 5) && (randint0(100) < 20)) obj->to_a--;
		obj->known->to_a = obj->to_a;
	}

	/* Message */
	msg("Your %s (%c) %s disenchanted!", o_name,
		gear_to_label(player, obj),
		((obj->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_EQUIP);

	return true;
}

/**
 * Enchant an item (in the inventory or on the floor)
 * Note that armour, to hit or to dam is controlled by context->subtype
 *
 * Work on incorporating enchant_spell() has been postponed...NRM
 */
bool effect_handler_ENCHANT(effect_handler_context_t *context)
{
	int value = randcalc(context->value, player->depth, RANDOMISE);
	bool used = false;
	context->ident = true;

	if ((context->subtype & ENCH_TOBOTH) == ENCH_TOBOTH) {
		if (enchant_spell(value, value, 0, context->cmd))
			used = true;
	}
	else if (context->subtype & ENCH_TOHIT) {
		if (enchant_spell(value, 0, 0, context->cmd))
			used = true;
	}
	else if (context->subtype & ENCH_TODAM) {
		if (enchant_spell(0, value, 0, context->cmd))
			used = true;
	}
	if (context->subtype & ENCH_TOAC) {
		if (enchant_spell(0, 0, value, context->cmd))
			used = true;
	}

	return used;
}

/**
 * Recharge a wand or staff from the pack or on the floor.  Recharge strength
 * is context->value.base.
 *
 * It is harder to recharge high level, and highly charged wands.
 */
bool effect_handler_RECHARGE(effect_handler_context_t *context)
{
	int i, t;
	int strength = context->value.base;
	int itemmode = (USE_INVEN | USE_FLOOR | SHOW_RECHARGE);
	struct object *obj;
	bool used = false;
	const char *q, *s;

	/* Immediately obvious */
	context->ident = true;

	/* Used to show recharge failure rates */
	player->upkeep->recharge_pow = strength;

	/* Get an item */
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";
	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, q, s,
				tval_can_have_charges, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, tval_can_have_charges, itemmode)) {
		return (used);
	}

	i = recharge_failure_chance(obj, strength);
	/* Back-fire */
	if ((i <= 1) || one_in_(i)) {
		struct object *destroyed;
		bool none_left = false;

		msg("The recharge backfires!");
		msg("There is a bright flash of light.");

		/* Reduce and describe inventory */
		if (object_is_carried(player, obj)) {
			destroyed = gear_object_for_use(player, obj, 1, true,
				&none_left);
		} else {
			destroyed = floor_object_for_use(player, obj, 1, true,
				&none_left);
		}
		if (destroyed->known)
			object_delete(player->cave, NULL, &destroyed->known);
		object_delete(cave, player->cave, &destroyed);
	} else {
		/* Extract a "power" */
		int ease_of_recharge = (100 - obj->kind->level) / 10;
		t = (strength / (10 - ease_of_recharge)) + 1;

		/* Recharge based on the power */
		if (t > 0) obj->pval += 2 + randint1(t);
	}

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN);

	/* Something was done */
	return true;
}

bool effect_handler_ACQUIRE(effect_handler_context_t *context)
{
	int num = effect_calculate_value(context, false);
	acquirement(player->grid, player->depth, num, true);
	context->ident = true;
	return true;
}

/**
 * Wake up all monsters in line of sight
 */
bool effect_handler_WAKE(effect_handler_context_t *context)
{
	int i;
	bool woken = false;

	struct loc origin = origin_get_loc(context->origin);

	/* Wake everyone nearby */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);
		if (mon->race) {
			int radius = z_info->max_sight * 2;
			int dist = distance(origin, mon->grid);

			/* Skip monsters too far away */
			if ((dist < radius) && mon->m_timed[MON_TMD_SLEEP]) {
				/* Monster wakes, closer means likelier to become aware */
				monster_wake(mon, false, 100 - 2 * dist);
				woken = true;
			}
		}
	}

	/* Messages */
	if (woken) {
		msg("You hear a sudden stirring in the distance!");
	}

	context->ident = true;

	return true;
}

/**
 * Summon context->value monsters of context->subtype type.
 */
bool effect_handler_SUMMON(effect_handler_context_t *context)
{
	int summon_max = effect_calculate_value(context, false);
	int summon_type = context->subtype;
	int level_boost = context->other;
	int message_type = summon_message_type(summon_type);
	int fallback_type = summon_fallback_type(summon_type);
	int count = 0, val = 0, attempts = 0;

	sound(message_type);

	/* No summoning in arena levels */
	if (player->upkeep->arena_level) return true;

	/* Monster summon */
	if (context->origin.what == SRC_MONSTER) {
		struct monster *mon = cave_monster(cave, context->origin.which.monster);
		int rlev;

		assert(mon);

		/* Set the kin_base if necessary */
		if (summon_type == summon_name_to_idx("KIN")) {
			kin_base = mon->race->base;
		}

		/* Continue summoning until we reach the current dungeon level */
		rlev = mon->race->level;
		while ((val < player->depth * rlev) && (attempts < summon_max)) {
			int temp;

			/* Get a monster */
			temp = summon_specific(mon->grid, rlev + level_boost, summon_type,
								   false, false);

			val += temp * temp;

			/* Increase the attempt in case no monsters were available. */
			attempts++;

			/* Increase count of summoned monsters */
			if (val > 0)
				count++;
		}

		/* If the summon failed and there's a fallback type, use that */
		if ((count == 0) && (fallback_type >= 0)) {
			attempts = 0;
			while ((val < player->depth * rlev) && (attempts < summon_max)) {
				int temp;

				/* Get a monster */
				temp = summon_specific(mon->grid, rlev + level_boost,
									   fallback_type, false, false);

				val += temp * temp;

				/* Increase the attempt in case no monsters were available. */
				attempts++;

				/* Increase count of summoned monsters */
				if (val > 0)
					count++;
			}
		}

		/* Summoner failed */
		if (!count)
			msg("But nothing comes.");
	} else {
		/* If not a monster summon, it's simple */
		while (summon_max) {
			count += summon_specific(player->grid, player->depth + level_boost,
									 summon_type, true, one_in_(4));
			summon_max--;
		}
	}

	/* Identify */
	context->ident = true;

	/* Message for the blind */
	if (count && player->timed[TMD_BLIND])
		msgt(message_type, "You hear %s appear nearby.",
			 (count > 1 ? "many things" : "something"));

	return true;
}

/**
 * Delete all non-unique monsters of a given "type" from the level
 * -------
 * Warning - this function assumes that the entered monster symbol is an ASCII
 *		   character, which may not be true in the future - NRM
 * -------
 */
bool effect_handler_BANISH(effect_handler_context_t *context)
{
	int i;
	unsigned dam = 0;

	char typ;

	context->ident = true;

	/* Don't allow in an arena. */
	if (player->upkeep->arena_level) {
		msg("Nothing happens.");
		return true;
	}

	if (!get_com("Choose a monster race (by symbol) to banish: ", &typ))
		return false;

	/* Delete the monsters of that "type" */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Hack -- Skip Unique Monsters */
		if (monster_is_unique(mon)) continue;

		/* Skip "wrong" monsters (see warning above) */
		if ((char) mon->race->d_char != typ) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take some damage */
		dam += randint1(4);
	}

	/* Hurt the player */
	take_hit(player, dam, "the strain of casting Banishment");

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;

	/* Success */
	return true;
}

/**
 * Delete all nearby (non-unique) monsters.  The radius of effect is
 * context->radius if passed, otherwise the player view radius.
 */
bool effect_handler_MASS_BANISH(effect_handler_context_t *context)
{
	int i;
	int radius = context->radius ? context->radius : z_info->max_sight;
	unsigned dam = 0;

	context->ident = true;

	/* Don't allow in an arena. */
	if (player->upkeep->arena_level) {
		msg("Nothing happens.");
		return true;
	}

	/* Delete the (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Hack -- Skip unique monsters */
		if (monster_is_unique(mon)) continue;

		/* Skip distant monsters */
		if (mon->cdis > radius) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take some damage */
		dam += randint1(3);
	}

	/* Hurt the player */
	take_hit(player, dam, "the strain of casting Mass Banishment");

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;

	return true;
}

/**
 * Probe nearby monsters
 */
bool effect_handler_PROBE(effect_handler_context_t *context)
{
	int i;

	bool probe = false;

	/* Probe all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Require line of sight */
		if (!square_isview(cave, mon->grid)) continue;

		/* Probe visible monsters */
		if (monster_is_visible(mon)) {
			char m_name[80];

			/* Start the message */
			if (!probe) msg("Probing...");

			/* Get "the monster" or "something" */
			monster_desc(m_name, sizeof(m_name), mon,
				MDESC_IND_HID | MDESC_CAPITAL | MDESC_COMMA);

			/* Describe the monster */
			msg("%s has %d hit point%s.", m_name, mon->hp, (mon->hp == 1) ? "" : "s");

			/* Learn all of the non-spell, non-treasure flags */
			lore_do_probe(mon);

			/* Probe worked */
			probe = true;
		}
	}

	/* Done */
	if (probe) {
		msg("That's all.");
		context->ident = true;
	}

	return true;
}

/**
 * Teleport player or monster up to context->value.base grids away.
 *
 * If no spaces are readily available, the distance may increase.
 * Try very hard to move the player/monster at least a quarter that distance.
 * Setting context->subtype allows monsters to teleport the player away.
 * Setting context->y and context->x treats them as y and x coordinates
 * and teleports the monster from that grid.
 */
bool effect_handler_TELEPORT(effect_handler_context_t *context)
{
	struct loc start = loc(context->x, context->y);
	int dis = context->value.base;
	int perc = context->value.m_bonus;
	int pick;
	struct loc grid;

	struct jumps {
		struct loc grid;
		struct jumps *next;
	} *spots = NULL;
	int num_spots = 0;
	int current_score = 2 * MAX(z_info->dungeon_wid, z_info->dungeon_hgt);
	bool only_vault_grids_possible = true;

	bool is_player = (context->origin.what != SRC_MONSTER || context->subtype);
	struct monster *t_mon = monster_target_monster(context);

	context->ident = true;

	/* No teleporting in arena levels */
	if (player->upkeep->arena_level) return true;

	/* Establish the coordinates to teleport from, if we don't know already */
	if (!loc_is_zero(start)) {
		/* We're good */
	} else if (t_mon) {
		/* Monster targeting another monster */
		start = t_mon->grid;
	} else if (is_player) {
		/* Decoys get destroyed */
		struct loc decoy = cave_find_decoy(cave);
		if (!loc_is_zero(decoy) && context->subtype) {
			square_destroy_decoy(cave, decoy);
			return true;
		}

		start = player->grid;

		/* Check for a no teleport grid */
		if (square_isno_teleport(cave, start) &&
			((dis > 10) || (dis == 0))) {
			msg("Teleportation forbidden!");
			return true;
		}

		/* Check for a no teleport curse */
		if (player_of_has(player, OF_NO_TELEPORT)) {
			equip_learn_flag(player, OF_NO_TELEPORT);
			msg("Teleportation forbidden!");
			return true;
		}
	} else {
		assert(context->origin.what == SRC_MONSTER);
		struct monster *mon = cave_monster(cave, context->origin.which.monster);
		start = mon->grid;
	}

	/* Percentage of the largest cardinal distance to an edge */
	if (perc) {
		int vertical = MAX(start.y, cave->height - start.y);
		int horizontal = MAX(start.x, cave->width - start.x);
		dis = (MAX(vertical, horizontal) * perc) / 100;
	}

	/* Randomise the distance a little */
	if (one_in_(2)) {
		dis -= randint0(dis / 4);
	} else {
		dis += randint0(dis / 4);
	}

	/* Make a list of the best grids, scoring by how good an approximation
	 * the distance from the start is to the distance we want */
	for (grid.y = 1; grid.y < cave->height - 1; grid.y++) {
		for (grid.x = 1; grid.x < cave->width - 1; grid.x++) {
			int d = distance(grid, start);
			int score = ABS(d - dis);
			struct jumps *new;

			/* Must move */
			if (d == 0) continue;

			if (!has_teleport_destination_prereqs(cave, grid,
					is_player)) continue;

			/* No teleporting into vaults and such, unless there's no choice */
			if (square_isvault(cave, grid)) {
				if (!only_vault_grids_possible) {
					continue;
				}
			} else {
				/* Just starting to consider non-vault grids, so reset score */
				if (only_vault_grids_possible) {
					current_score = 2 * MAX(z_info->dungeon_wid,
											z_info->dungeon_hgt);
				}
				only_vault_grids_possible = false;
			}

			/* Do we have better spots already? */
			if (score > current_score) continue;

			/* Make a new spot */
			new = mem_zalloc(sizeof(struct jumps));
			new->grid = grid;

			/* If improving start a new list, otherwise extend the old one */
			if (score < current_score) {
				current_score = score;
				while (spots) {
					struct jumps *next = spots->next;
					mem_free(spots);
					spots = next;
				}
				spots = new;
				num_spots = 1;
			} else {
				new->next = spots;
				spots = new;
				num_spots++;
			}
		}
	}

	/* Report failure (very unlikely) */
	if (!num_spots) {
		if (is_player) {
			msg("Failed to find teleport destination!");
		} else {
			/*
			 * With either teleport self or teleport other, it'll
			 * be the caster that is puzzled.
			 */
			struct monster *mon = cave_monster(cave,
				context->origin.which.monster);

			if (square_isseen(cave, mon->grid)) {
				add_monster_message(mon, MON_MSG_BRIEF_PUZZLE,
					true);
			}
		}
		return true;
	}

	/* Pick a spot */
	pick = randint0(num_spots);
	while (pick) {
		struct jumps *next = spots->next;
		mem_free(spots);
		spots = next;
		pick--;
	}

	/* Sound */
	sound(is_player ? MSG_TELEPORT : MSG_TPOTHER);

	/* Move player or monster */
	monster_swap(start, spots->grid);
	if (is_player) {
		player_handle_post_move(player, true,
			context->origin.what == SRC_MONSTER);
	}

	/* Clear any projection marker to prevent double processing */
	sqinfo_off(square(cave, spots->grid)->info, SQUARE_PROJECT);

	/* Clear monster target if it's no longer visible */
	if (!target_able(target_get_monster())) {
		target_set_monster(NULL);
	}

	/* Lots of updates after monster_swap */
	handle_stuff(player);

	while (spots) {
		struct jumps *next = spots->next;
		mem_free(spots);
		spots = next;
	}

	return true;
}

/**
 * Teleport player or target monster to a grid near the given location
 * Setting context->y and context->x treats them as y and x coordinates
 * Setting context->subtype allows monsters to teleport toward the player.
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
bool effect_handler_TELEPORT_TO(effect_handler_context_t *context)
{
	struct monster *mon = NULL;
	struct loc start, aim, land;
	int dis = 0, ctr = 0, dir = DIR_TARGET;
	struct monster *t_mon = monster_target_monster(context);
	bool dim_door = false;
	bool player_moves = false;

	context->ident = true;

	/* No teleporting in arena levels */
	if (player->upkeep->arena_level) return true;

	if (context->origin.what == SRC_MONSTER) {
		mon = cave_monster(cave, context->origin.which.monster);
		assert(mon);
	}

	/* Where are we coming from? */
	if (t_mon) {
		/* Monster being teleported */
		start = t_mon->grid;
	} else if (context->subtype) {
		/* Monster teleporting to the player */
		start = mon->grid;
	} else {
		/* Targeted decoys get destroyed */
		if (mon && monster_is_decoyed(mon)) {
			square_destroy_decoy(cave, cave_find_decoy(cave));
			return true;
		}

		/* Player being teleported */
		player_moves = true;
		start = player->grid;

		/* Check for a no teleport grid */
		if (square_isno_teleport(cave, start)) {
			msg("Teleportation forbidden!");
			return true;
		}

		/* Check for a no teleport curse */
		if (player_of_has(player, OF_NO_TELEPORT)) {
			equip_learn_flag(player, OF_NO_TELEPORT);
			msg("Teleportation forbidden!");
			return true;
		}
	}

	/* Where are we going? */
	if (context->y && context->x) {
		/* Effect was given co-ordinates */
		aim = loc(context->x, context->y);
	} else if (mon) {
		/* Spell cast by monster */
		if (context->subtype) {
			/* Monster teleporting to player */
			aim = player->grid;
			dis = 2;
		} else {
			/* Player being teleported to monster */
			aim = mon->grid;
		}
	} else {
		/* Player choice */
		do {
			if (!get_aim_dir(&dir)) return false;
		} while (dir == DIR_TARGET && !target_okay());

		if (dir == DIR_TARGET)
			target_get(&aim);
		else
			aim = loc_offset(start, ddx[dir], ddy[dir]);

		/* Randomise the landing a bit if it's a vault */
		if (square_isvault(cave, aim)) dis = 10;
		dim_door = true;
	}

	/* Find a usable location */
	while (1) {
		/* Pick a nearby legal location */
		while (1) {
			land = rand_loc(aim, dis, dis);
			if (square_in_bounds_fully(cave, land)) break;
		}

		if (has_teleport_destination_prereqs(cave, land,
				player_moves)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1)) {
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player or monster */
	monster_swap(start, land);
	if (player_moves) {
		player_handle_post_move(player, true,
			context->origin.what == SRC_MONSTER);
	}

	/* Cancel target if necessary */
	if (dim_door) {
		target_set_location(0, 0);
	}

	/* Clear any projection marker to prevent double processing */
	sqinfo_off(square(cave, land)->info, SQUARE_PROJECT);

	/* Lots of updates after monster_swap */
	handle_stuff(player);

	return true;
}

/**
 * Teleport the player one level up or down (random when legal)
 */
bool effect_handler_TELEPORT_LEVEL(effect_handler_context_t *context)
{
	bool up = true;
	bool down = true;
	int target_depth = dungeon_get_next_level(player, player->max_depth, 1);
	struct monster *t_mon = monster_target_monster(context);
	struct loc decoy = cave_find_decoy(cave);

	context->ident = true;

	/* No teleporting in arena levels */
	if (player->upkeep->arena_level) return true;

	/* Check for monster targeting another monster */
	if (t_mon) {
		/* Monster is just gone */
		add_monster_message(t_mon, MON_MSG_DISAPPEAR, false);
		delete_monster_idx(t_mon->midx);
		return true;
	}

	/* Targeted decoys get destroyed */
	if (decoy.y && decoy.x) {
		square_destroy_decoy(cave, decoy);
		return true;
	}

	/* Check for a no teleport grid */
	if (square_isno_teleport(cave, player->grid)) {
		msg("Teleportation forbidden!");
		return true;
	}

	/* Check for a no teleport curse */
	if (player_of_has(player, OF_NO_TELEPORT)) {
		equip_learn_flag(player, OF_NO_TELEPORT);
		msg("Teleportation forbidden!");
		return true;
	}

	/* Resist hostile teleport */
	if (context->origin.what == SRC_MONSTER &&
			player_resists(player, ELEM_NEXUS)) {
		msg("You resist the effect!");
		return true;
	}

	/* No going up with force_descend or in the town */
	if (OPT(player, birth_force_descend) || !player->depth)
		up = false;

	/* No forcing player down to quest levels if they can't leave */
	if (!up && is_quest(player, target_depth))
		down = false;

	/* Can't leave quest levels or go down deeper than the dungeon */
	if (is_quest(player, player->depth)
			|| (player->depth >= z_info->max_depth - 1))
		down = false;

	/* Determine up/down if not already done */
	if (up && down) {
		if (randint0(100) < 50)
			up = false;
		else
			down = false;
	}

	/*
	 * Now actually do the level change; flush the command queue to
	 * prevent the character from losing an action when first entering
	 * the new level (for instance, player moves putting an autopickup
	 * command in the queue and is then hit by a teleport level spell)
	 */
	if (up) {
		msgt(MSG_TPLEVEL, "You rise up through the ceiling.");
		cmdq_flush();
		target_depth = dungeon_get_next_level(player,
			player->depth, -1);
		dungeon_change_level(player, target_depth);
	} else if (down) {
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		cmdq_flush();
		if (OPT(player, birth_force_descend)) {
			target_depth = dungeon_get_next_level(player,
				player->max_depth, 1);
			dungeon_change_level(player, target_depth);
		} else {
			target_depth = dungeon_get_next_level(player,
				player->depth, 1);
			dungeon_change_level(player, target_depth);
		}
	} else {
		msg("Nothing happens.");
	}

	return true;
}

/**
 * The rubble effect
 *
 * This causes rubble to fall into empty squares.
 */
bool effect_handler_RUBBLE(effect_handler_context_t *context)
{
	/*
	 * First we work out how many grids we want to fill with rubble.  Then we
	 * check that we can actually do this, by counting the number of grids
	 * available, limiting the number of rubble grids to this number if
	 * necessary.
	 */
	int rubble_grids = randint1(3);
	int open_grids = count_feats(NULL, square_isempty, false);

	if (rubble_grids > open_grids) {
		rubble_grids = open_grids;
	}

	/* Avoid infinite loops */
	int iterations = 0;

	while (rubble_grids > 0 && iterations < 10) {
		/* Look around the player */
		for (int d = 0; d < 8; d++) {
			/* Extract adjacent (legal) location */
			struct loc grid = loc_sum(player->grid, ddgrid_ddd[d]);
			if (!square_in_bounds_fully(cave, grid)) continue;
			if (!square_isempty(cave, grid)) continue;

			if (one_in_(3)) {
				if (one_in_(2))
					square_set_feat(cave, grid, FEAT_PASS_RUBBLE);
				else
					square_set_feat(cave, grid, FEAT_RUBBLE);
				if (cave->depth == 0)
					expose_to_sun(cave, grid, is_daytime());
				rubble_grids--;
			}
		}

		iterations++;
	}

	context->ident = true;

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw monster list */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return true;
}

bool effect_handler_GRANITE(effect_handler_context_t *context)
{
	struct trap *trap = context->origin.which.trap;
	square_set_feat(cave, trap->grid, FEAT_GRANITE);
	if (cave->depth == 0) expose_to_sun(cave, trap->grid, is_daytime());

	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return true;
}

bool effect_handler_LIGHT_LEVEL(effect_handler_context_t *context)
{
	bool full = context->value.base ? true : false;
	if (full)
		msg("An image of your surroundings forms in your mind...");
	wiz_light(cave, player, full);
	context->ident = true;
	return true;
}

bool effect_handler_DARKEN_LEVEL(effect_handler_context_t *context)
{
	bool full = context->value.base ? true : false;
	if (full)
		msg("A great blackness rolls through the dungeon...");
	wiz_dark(cave, player, full);
	context->ident = true;
	return true;
}

/**
 * Call light around the player
 */
bool effect_handler_LIGHT_AREA(effect_handler_context_t *context)
{
	/* Message */
	if (!player->timed[TMD_BLIND])
		msg("You are surrounded by a white light.");

	/* Light up the room */
	light_room(player->grid, true);

	/* Assume seen */
	context->ident = true;
	return (true);
}


/**
 * Call darkness around the player or target monster
 */
bool effect_handler_DARKEN_AREA(effect_handler_context_t *context)
{
	struct loc target = player->grid;
	bool message = player->timed[TMD_BLIND] ? false : true;
	struct monster *mon = NULL;
	struct monster *t_mon = monster_target_monster(context);
	struct loc decoy = cave_find_decoy(cave);
	bool decoy_unseen = false;

	if (context->origin.what == SRC_MONSTER) {
		mon = cave_monster(cave, context->origin.which.monster);
	}

	/* Check for monster targeting another monster */
	if (t_mon) {
		char m_name[80];
		target = t_mon->grid;
		monster_desc(m_name, sizeof(m_name), t_mon, MDESC_TARG);
		if (message) {
			msg("Darkness surrounds %s.", m_name);
			message = false;
		}
	}

	/* Check for decoy */
	if (mon && monster_is_decoyed(mon)) {
		target = decoy;
		if (!los(cave, player->grid, decoy) ||
			player->timed[TMD_BLIND]) {
			decoy_unseen = true;
		}
		if (message && !decoy_unseen) {
			msg("Darkness surrounds the decoy.");
			message = false;
		}
	}

	if (message) {
		msg("Darkness surrounds you.");
	}

	/* Darken the room */
	light_room(target, false);

	/* Hack - blind the player directly if player-cast */
	if (context->origin.what == SRC_PLAYER &&
		!player_resists(player, ELEM_DARK)) {
		(void)player_inc_timed(player, TMD_BLIND, 3 + randint1(5),
			true, !context->aware, true);
	}

	/* Assume seen */
	context->ident = !decoy_unseen;
	return (true);
}

/**
 * Curse the player's armor
 */
bool effect_handler_CURSE_ARMOR(effect_handler_context_t *context)
{
	struct object *obj;

	char o_name[80];

	/* Curse the body armor */
	obj = equipped_item_by_slot_name(player, "body");

	/* Nothing to curse */
	if (!obj) return (true);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, player);

	/* Attempt a saving throw for artifacts */
	if (obj->artifact && (randint0(100) < 50)) {
		msg("A %s tries to %s, but your %s resists the effects!",
				   "terrible black aura", "surround your armor", o_name);
	} else {
		int num = randint1(3);
		int max_tries = 20;
		msg("A terrible black aura blasts your %s!", o_name);

		/* Take down bonus a wee bit */
		obj->to_a -= randint1(3);

		/* Try to find enough appropriate curses */
		while (num && max_tries) {
			int pick = randint1(z_info->curse_max - 1);
			int power = 10 * m_bonus(9, player->depth);
			if (!curses[pick].poss[obj->tval]) {
				max_tries--;
				continue;
			}
			append_object_curse(obj, pick, power);
			num--;
		}

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana */
		player->upkeep->update |= (PU_MANA);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}

	context->ident = true;

	return (true);
}


/**
 * Curse the player's weapon
 */
bool effect_handler_CURSE_WEAPON(effect_handler_context_t *context)
{
	struct object *obj;

	char o_name[80];

	/* Curse the weapon */
	obj = equipped_item_by_slot_name(player, "weapon");

	/* Nothing to curse */
	if (!obj) return (true);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, player);

	/* Attempt a saving throw */
	if (obj->artifact && (randint0(100) < 50)) {
		msg("A %s tries to %s, but your %s resists the effects!",
				   "terrible black aura", "surround your weapon", o_name);
	} else {
		int num = randint1(3);
		int max_tries = 20;
		msg("A terrible black aura blasts your %s!", o_name);

		/* Hurt it a bit */
		obj->to_h = 0 - randint1(3);
		obj->to_d = 0 - randint1(3);

		/* Curse it */
		while (num) {
			int pick = randint1(z_info->curse_max - 1);
			int power = 10 * m_bonus(9, player->depth);
			if (!curses[pick].poss[obj->tval]) {
				max_tries--;
				continue;
			}
			append_object_curse(obj, pick, power);
			num--;
		}

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana */
		player->upkeep->update |= (PU_MANA);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}

	context->ident = true;

	/* Notice */
	return (true);
}


/**
 * Brand the current weapon
 */
bool effect_handler_BRAND_WEAPON(effect_handler_context_t *context)
{
	struct object *obj = equipped_item_by_slot_name(player, "weapon");

	/* Select the brand */
	const char *brand = one_in_(2) ? "Flame" : "Frost";

	/* Brand the weapon */
	brand_object(obj, brand);

	context->ident = true;
	return true;
}


/**
 * Brand some (non-magical) ammo
 */
bool effect_handler_BRAND_AMMO(effect_handler_context_t *context)
{
	struct object *obj;
	const char *q, *s;
	int itemmode = (USE_INVEN | USE_QUIVER | USE_FLOOR);
	bool used = false;

	/* Select the brand */
	const char *brand = one_in_(3) ? "Flame" : (one_in_(2) ? "Frost" : "Venom");

	context->ident = true;

	/* Get an item */
	q = "Brand which kind of ammunition? ";
	s = "You have nothing to brand.";
	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, q, s,
				tval_is_ammo, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, tval_is_ammo, itemmode))
		return used;

	/* Brand the ammo */
	brand_object(obj, brand);

	/* Done */
	return (true);
}

/**
 * Enchant some (non-magical) bolts
 */
bool effect_handler_BRAND_BOLTS(effect_handler_context_t *context)
{
	struct object *obj;
	const char *q, *s;
	int itemmode = (USE_INVEN | USE_QUIVER | USE_FLOOR);
	bool used = false;

	context->ident = true;

	/* Get an item */
	q = "Brand which bolts? ";
	s = "You have no bolts to brand.";
	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, q, s,
				tval_is_bolt, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, tval_is_bolt, itemmode))
		return used;

	/* Brand the bolts */
	brand_object(obj, "Flame");

	/* Done */
	return (true);
}


/**
 * Turn a staff into arrows
 */
bool effect_handler_CREATE_ARROWS(effect_handler_context_t *context)
{
	int lev;
	struct object *obj, *staff, *arrows;
	const char *q, *s;
	int itemmode = (USE_INVEN | USE_FLOOR);
	bool good = false, great = false;
	bool none_left = false;

	/* Get an item */
	q = "Make arrows from which staff? ";
	s = "You have no staff to use.";
	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, q, s,
				tval_is_staff, itemmode)) {
			return false;
		}
	} else if (!get_item(&obj, q, s, 0, tval_is_staff, itemmode)) {
		return false;
	}

	/* Extract the object "level" */
	lev = obj->kind->level;

	/* Roll for good */
	if (randint1(lev) > 25) {
		good = true;
		/* Roll for great */
		if (randint1(lev) > 50) {
			great = true;
		}
	}

	/* Destroy the staff */
	if (object_is_carried(player, obj)) {
		staff = gear_object_for_use(player, obj, 1, true, &none_left);
	} else {
		staff = floor_object_for_use(player, obj, 1, true, &none_left);
	}

	if (staff->known) {
		object_delete(player->cave, NULL, &staff->known);
	}
	object_delete(cave, player->cave, &staff);

	/* Make some arrows */
	arrows = make_object(cave, player->lev, good, great, false, NULL, TV_ARROW);
	drop_near(cave, &arrows, 0, player->grid, true, true);

	return true;
}

/**
 * Draw energy from a magical device
 */
bool effect_handler_TAP_DEVICE(effect_handler_context_t *context)
{
	int lev;
	int energy = 0;
	struct object *obj;
	bool used = false;
	int itemmode = (USE_INVEN | USE_FLOOR);
	const char *q, *s;
	const char *item = "";

	/* Get an item */
	q = "Drain charges from which item? ";
	s = "You have nothing to drain charges from.";
	if (context->cmd) {
		if (cmd_get_item(context->cmd, "tgtitem", &obj, q, s,
				tval_can_have_charges, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, tval_can_have_charges, itemmode)) {
		return (used);
	}

	/* Extract the object "level" */
	lev = obj->kind->level;

	/* Extract the object's energy and get its generic name. */
	if (tval_is_staff(obj)) {
		energy = (5 + lev) * 3 * obj->pval / 2;
		item = "staff";
	} else if (tval_is_wand(obj)) {
		energy = (5 + lev) * 3 * obj->pval / 2;
		item = "wand";
	}

	/* Turn energy into mana. */
	if (energy < 36) {
		/* Require a resonable amount of energy */
		msg("That %s had no useable energy", item);
	} else {
		/* If mana below maximum, increase mana and drain object. */
		if (player->csp < player->msp) {
			/* Drain the object. */
			obj->pval = 0;


			/* Combine / Reorder the pack (later) */
			player->upkeep->notice |= (PN_COMBINE);

			/* Redraw stuff */
			player->upkeep->redraw |= (PR_INVEN);

			/* Increase mana. */
			player->csp += energy / 6;
			player->csp_frac = 0;
			if (player->csp > player->msp) {
				(player->csp = player->msp);
			}

			msg("You feel your head clear.");
			used = true;
			player_inc_timed(player, TMD_STUN, randint1(2), true,
				context->origin.what != SRC_PLAYER
				|| !context->aware, true);

			player->upkeep->redraw |= (PR_MANA);
		} else {
			char *cap = string_make(item);
			my_strcap(cap);
			msg("Your mana was already at its maximum.  %s not drained.", cap);
			string_free(cap);
		}
	}

	return (used);
}

/**
 * Perform a player shapechange
 */
bool effect_handler_SHAPECHANGE(effect_handler_context_t *context)
{
	struct player_shape *shape = player_shape_by_idx(context->subtype);
	bool ident = false;

	assert(shape);

	/* Change shape */
	player->shape = lookup_player_shape(shape->name);
	msg("You assume the shape of a %s!", shape->name);
	msg("Your gear merges into your body.");

	/* Do effect */
	if (shape->effect) {
		(void) effect_do(shape->effect, source_player(), NULL, &ident, true,
						 0, 0, 0, NULL);
	}

	/* Update */
	shape_learn_on_assume(player, shape->name);
	player->upkeep->update |= (PU_BONUS);
	player->upkeep->redraw |= (PR_TITLE | PR_MISC);
	handle_stuff(player);

	return true;
}

/**
 * Take control of a monster
 */
bool effect_handler_COMMAND(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	struct monster *mon = target_get_monster();

	context->ident = true;

	/* Need to choose a monster, not just point */
	if (!mon) {
		msg("No monster selected!");
		return false;
	}

	/* Wake up, become aware */
	monster_wake(mon, false, 100);

	/* Explicit saving throw */
	if (randint1(player->lev) < randint1(mon->race->level)) {
		char m_name[80];
		monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);
		msg("%s resists your command!", m_name);
		return false;
	}

	/* Player is commanding */
	player_set_timed(player, TMD_COMMAND, MAX(amount, 0), false, false);

	/* Monster is commanded */
	mon_inc_timed(mon, MON_TMD_COMMAND, MAX(amount, 0), 0);

	return true;
}

/**
 * One Ring activation
 */
bool effect_handler_BIZARRE(effect_handler_context_t *context)
{
	context->ident = true;

	/* Pick a random effect */
	switch (randint1(10))
	{
		case 1:
		case 2:
		{
			/* Message */
			msg("You are surrounded by a malignant aura.");

			/* Decrease all stats (permanently) */
			player_stat_dec(player, STAT_STR, true);
			player_stat_dec(player, STAT_INT, true);
			player_stat_dec(player, STAT_WIS, true);
			player_stat_dec(player, STAT_DEX, true);
			player_stat_dec(player, STAT_CON, true);

			/* Lose some experience (permanently) */
			player_exp_lose(player, player->exp / 4, true);

			return true;
		}

		case 3:
		{
			/* Message */
			msg("You are surrounded by a powerful aura.");

			/* Dispel monsters */
			effect_simple(EF_PROJECT_LOS, context->origin, "1000", PROJ_DISP_ALL, 0, 0, 0, 0, NULL);

			return true;
		}

		case 4:
		case 5:
		case 6:
		{
			/* Mana Ball */
			int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
			struct loc target = loc_sum(player->grid, ddgrid[context->dir]);

			/* Ask for a target if no direction given */
			if ((context->dir == DIR_TARGET) && target_okay()) {
				flg &= ~(PROJECT_STOP | PROJECT_THRU);

				target_get(&target);
			}

			/* Aim at the target, explode */
			return (project(source_player(), 3, target, 300, PROJ_MANA, flg, 0,
							0, context->obj));
		}

		case 7:
		case 8:
		case 9:
		case 10:
		{
			/* Mana Bolt */
			int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_THRU;
			struct loc target = loc_sum(player->grid, ddgrid[context->dir]);

			/* Use an actual target */
			if ((context->dir == DIR_TARGET) && target_okay())
				target_get(&target);

			/* Aim at the target, do NOT explode */
			return project(source_player(), 0, target, 250, PROJ_MANA, flg, 0,
						   0, context->obj);
		}
	}

	return false;
}

/**
 * Dummy effect, to tell the effect code to pick one of the next
 * context->value.base effects at the player's selection or, if the effect
 * wasn't initiated by the player, at random.
 */
bool effect_handler_SELECT(effect_handler_context_t *context)
{
	return true;
}

/**
 * Dummy effect, to tell the effect code to set a value for a string of
 * following effects to use, rather than setting their own value.
 * The value will not use the device boost, which should not be a problem
 * as it is unlikely to be used for damage (the main use case is to
 * synchronise the end of timed effects).
 */
bool effect_handler_SET_VALUE(effect_handler_context_t *context)
{
	set_value = effect_calculate_value(context, false);
	return true;
}

/**
 * Dummy effect, to tell the effect code to clear a value set by the
 * SET_VALUE effect.
 */
bool effect_handler_CLEAR_VALUE(effect_handler_context_t *context)
{
	set_value = 0;
	return true;
}
