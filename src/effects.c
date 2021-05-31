/**
 * \file effects.c
 * \brief Handler and auxiliary functions for every effect in the game
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

#include "angband.h"
#include "cave.h"
#include "effects.h"
#include "game-input.h"
#include "generate.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-summon.h"
#include "mon-util.h"
#include "mon-timed.h"
#include "obj-chest.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "source.h"
#include "target.h"
#include "trap.h"


/**
 * ------------------------------------------------------------------------
 * Structures and helper functions for effects
 * ------------------------------------------------------------------------ */
typedef struct effect_handler_context_s {
	const effect_index effect;
	const struct source origin;
	const struct object *obj;
	const bool aware;
	const int dir;
	const int beam;
	const int boost;
	const random_value value;
	const int subtype, radius, other, y, x;
	const char *msg;
	bool ident;
	struct command *cmd;
} effect_handler_context_t;

typedef bool (*effect_handler_f)(effect_handler_context_t *);

/**
 * Structure for effects
 */
struct effect_kind {
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	const char *info;    /* Effect info (for spell tips) */
	effect_handler_f handler;    /* Function to perform the effect */
	const char *desc;    /* Effect description */
	const char *menu_name;       /* Format string for short name */
};


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


static int effect_calculate_value(effect_handler_context_t *context, bool use_boost)
{
	int final = 0;

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

static void get_target(struct source origin, int dir, struct loc *grid,
					   int *flags)
{
	switch (origin.what) {
		case SRC_MONSTER: {
			struct monster *monster = cave_monster(cave, origin.which.monster);
			int conf_level, accuracy = 100;

			if (!monster) break;

			conf_level = monster_effect_level(monster, MON_TMD_CONF);
			while (conf_level) {
				accuracy *= (100 - CONF_RANDOM_CHANCE);
				accuracy /= 100;
				conf_level--;
			}

			*flags |= (PROJECT_PLAY);

			if (randint1(100) > accuracy) {
				dir = randint1(9);
				*grid = loc_sum(monster->grid, ddgrid[dir]);
			} else if (monster->target.midx > 0) {
				struct monster *mon = cave_monster(cave, monster->target.midx);
				*grid = mon->grid;
			} else {
				if (monster_is_decoyed(monster)) {
					*grid = cave_find_decoy(cave);
				} else {
					*grid = player->grid;
				}
			}

			break;
		}

		case SRC_PLAYER:
			if (dir == DIR_TARGET && target_okay()) {
				target_get(grid);
			} else {
				/* Use the adjacent grid in the given direction as target */
				*grid = loc_sum(player->grid, ddgrid[dir]);
			}

			break;

		default:
			*flags |= PROJECT_PLAY;
			*grid = player->grid;
			break;
	}
}

/**
 * Check for monster targeting another monster
 */
static struct monster *monster_target_monster(effect_handler_context_t *context)
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
 * Apply the project() function in a direction, or at a target
 */
static bool project_aimed(struct source origin,
						  int typ, int dir, int dam, int flg,
						  const struct object *obj)
{
	struct loc grid = loc(-1, -1);

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	get_target(origin, dir, &grid, &flg);

	/* Aim at the target, do NOT explode */
	return (project(origin, 0, grid, dam, typ, flg, 0, 0, obj));
}

/**
 * Apply the project() function to grids around the target
 */
static bool project_touch(int dam, int rad, int typ, bool aware,
						  const struct object *obj)
{
	struct loc pgrid = player->grid;

	int flg = PROJECT_GRID | PROJECT_KILL | PROJECT_HIDE | PROJECT_ITEM | PROJECT_THRU;
	if (aware) flg |= PROJECT_AWARE;
	return (project(source_player(), rad, pgrid, dam, typ, flg, 0, 0, obj));
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
 * Removes an individual curse from an object.
 */
static void remove_object_curse(struct object *obj, int index, bool message)
{
	struct curse_data *c = &obj->curses[index];
	char *name = curses[index].name;
	char *removed = format("The %s curse is removed!", name);
	int i;

	c->power = 0;
	c->timeout = 0;
	if (message) {
		msg(removed);
	}

	/* Check to see if that was the last one */
	for (i = 1; i < z_info->curse_max; i++) {
		if (obj->curses[i].power) {
			return;
		}
	}

	mem_free(obj->curses);
	obj->curses = NULL;
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
			object_desc(o_name, sizeof(o_name), obj, ODESC_FULL);
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
				destroyed = gear_object_for_use(obj, 1, false, &none_left);
				if (destroyed->artifact) {
					/* Artifacts are marked as lost */
					history_lose_artifact(player, destroyed->artifact);
				}
				object_delete(&destroyed->known);
				object_delete(&destroyed);
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
 * Bit flags for the enchant() function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOBOTH  0x03
#define ENCH_TOAC	0x04

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
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(const struct object *obj)
{
	return tval_is_weapon(obj);
}


/**
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(const struct object *obj)
{
	return tval_is_armor(obj);
}

/**
 * Tries to increase an items bonus score, if possible.
 *
 * \returns true if the bonus was increased
 */
static bool enchant_score(s16b *score, bool is_artifact)
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
static bool enchant2(struct object *obj, s16b *score)
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
	item_tester filter = num_ac ?
		item_tester_hook_armour : item_tester_hook_weapon;

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
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

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

		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
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
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(const struct object *obj)
{
	/* Recharge staves and wands */
	if (tval_can_have_charges(obj)) return true;

	return false;
}

/**
 * Hook to specify a staff
 */
static bool item_tester_hook_staff(const struct object *obj)
{
	return obj->tval == TV_STAFF;
}

/**
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(const struct object *obj)
{
	return tval_is_ammo(obj);
}

/**
 * Hook to specify bolts
 */
static bool item_tester_hook_bolt(const struct object *obj)
{
	return obj->tval == TV_BOLT;
}

/**
 * ------------------------------------------------------------------------
 * Effect handlers
 * ------------------------------------------------------------------------ */
/**
 * Dummy effect, to tell the effect code to pick one of the next
 * context->value.base effects at random.
 */
static bool effect_handler_RANDOM(effect_handler_context_t *context)
{
	return true;
}

/**
 * Deal damage from the current monster or trap to the player
 */
static bool effect_handler_DAMAGE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, false);
	char killer[80];

	/* Always ID */
	context->ident = true;

	switch (context->origin.what) {
		case SRC_MONSTER: {
			struct monster *mon = cave_monster(cave,
											   context->origin.which.monster);
			struct monster *t_mon = monster_target_monster(context);
			struct loc decoy = cave_find_decoy(cave);

			/* Damage another monster */
			if (t_mon) {
				bool fear = false;

				mon_take_nonplayer_hit(dam, t_mon, MON_MSG_NONE, MON_MSG_DIE);
				if (fear && monster_is_visible(t_mon)) {
					add_monster_message(t_mon, MON_MSG_FLEE_IN_TERROR, true);
				}
				return true;
			}

			/* Destroy a decoy */
			if (decoy.y && decoy.x) {
				square_destroy_decoy(cave, decoy);
				return true;
			}

			monster_desc(killer, sizeof(killer), mon, MDESC_DIED_FROM);
			break;
		}

		case SRC_TRAP: {
			struct trap *trap = context->origin.which.trap;
			const char *article = is_a_vowel(trap->kind->desc[0]) ? "an " : "a ";
			strnfmt(killer, sizeof(killer), "%s%s", article, trap->kind->desc);
			break;
		}

		case SRC_OBJECT: {
			/* Must be a cursed weapon */
			struct object *obj = context->origin.which.object;
			object_desc(killer, sizeof(killer), obj, ODESC_PREFIX | ODESC_BASE);
			break;
		}

		case SRC_CHEST_TRAP: {
			struct chest_trap *trap = context->origin.which.chest_trap;
			strnfmt(killer, sizeof(killer), "%s", trap->msg_death);
			break;
		}

		case SRC_PLAYER: {
			if (context->msg) {
				my_strcpy(killer, context->msg, sizeof(killer));
			} else {
				my_strcpy(killer, "yourself", sizeof(killer));
			}
			break;
		}

		case SRC_NONE: {
			my_strcpy(killer, "a bug", sizeof(killer));
			break;
		}
	}

	/* Hit the player */
	take_hit(player, dam, killer);

	return true;
}


/**
 * Heal the player by a given percentage of their wounds, or a minimum
 * amount, whichever is larger.
 *
 * context->value.base should be the minimum, and
 * context->value.m_bonus the percentage
 */
static bool effect_handler_HEAL_HP(effect_handler_context_t *context)
{
	int num;

	/* Paranoia */
	if ((context->value.m_bonus <= 0) && (context->value.base <= 0))
		return (true);

	/* Always ID */
	context->ident = true;

	/* No healing needed */
	if (player->chp >= player->mhp) return (true);

	/* Figure percentage healing level */
	num = ((player->mhp - player->chp) * context->value.m_bonus) / 100;

	/* Enforce minimum */
	if (num < context->value.base) num = context->value.base;

	/* Gain hitpoints */
	player->chp += num;

	/* Enforce maximum */
	if (player->chp >= player->mhp) {
		player->chp = player->mhp;
		player->chp_frac = 0;
	}

	/* Redraw */
	player->upkeep->redraw |= (PR_HP);

	/* Print a nice message */
	if (num < 5)
		msg("You feel a little better.");
	else if (num < 15)
		msg("You feel better.");
	else if (num < 35)
		msg("You feel much better.");
	else
		msg("You feel very good.");

	return (true);
}


/**
 * Monster self-healing.
 */
static bool effect_handler_MON_HEAL_HP(effect_handler_context_t *context)
{
	assert(context->origin.what == SRC_MONSTER);

	int midx = context->origin.which.monster;
	struct monster *mon = midx > 0 ? cave_monster(cave, midx) : NULL;

	int amount = effect_calculate_value(context, false);
	char m_name[80], m_poss[80];
	bool seen;

	if (!mon) return true;

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), mon, MDESC_PRO_VIS | MDESC_POSS);

	seen = (!player->timed[TMD_BLIND] && monster_is_visible(mon));

	/* Heal some */
	mon->hp += amount;

	/* Fully healed */
	if (mon->hp >= mon->maxhp) {
		mon->hp = mon->maxhp;

		if (seen)
			msg("%s looks REALLY healthy!", m_name);
		else
			msg("%s sounds REALLY healthy!", m_name);
	} else if (seen) { /* Partially healed */
		msg("%s looks healthier.", m_name);
	} else {
		msg("%s sounds healthier.", m_name);
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Cancel fear */
	if (mon->m_timed[MON_TMD_FEAR]) {
		mon_clear_timed(mon, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE);
		msg("%s recovers %s courage.", m_name, m_poss);
	}

	/* ID */
	context->ident = true;

	return true;
}

/**
 * Monster healing of kin.
 */
static bool effect_handler_MON_HEAL_KIN(effect_handler_context_t *context)
{
	assert(context->origin.what == SRC_MONSTER);

	int midx = context->origin.which.monster;
	struct monster *mon = midx > 0 ? cave_monster(cave, midx) : NULL;
	if (!mon) return true;

	int amount = effect_calculate_value(context, false);
	char m_name[80], m_poss[80];
	bool seen;

	/* Find a nearby monster */
	mon = choose_nearby_injured_kin(cave, mon);
	if (!mon) return true;

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), mon, MDESC_PRO_VIS | MDESC_POSS);

	seen = (!player->timed[TMD_BLIND] && monster_is_visible(mon));

	/* Heal some */
	mon->hp = MIN(mon->hp + amount, mon->maxhp);

	if (seen) {
		if (mon->hp == mon->maxhp) {
			msg("%s looks REALLY healthy!", m_name);
		} else if (seen) { /* Partially healed */
			msg("%s looks healthier.", m_name);
		}
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Cancel fear */
	if (mon->m_timed[MON_TMD_FEAR]) {
		mon_clear_timed(mon, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE);
		msg("%s recovers %s courage.", m_name, m_poss);
	}

	/* ID */
	context->ident = true;

	return true;
}

/**
 * Feed the player, or set their satiety level.
 */
static bool effect_handler_NOURISH(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	amount *= z_info->food_value;
	if (context->subtype == 0) {
		/* Increase food level by amount */
		player_inc_timed(player, TMD_FOOD, MAX(amount, 0), false, false);
	} else if (context->subtype == 1) {
		/* Decrease food level by amount */
		player_dec_timed(player, TMD_FOOD, MAX(amount, 0), false);
	} else if (context->subtype == 2) {
		/* Set food level to amount, vomiting if necessary */
		bool message = player->timed[TMD_FOOD] > amount;
		if (message) {
			msg("You vomit!");
		}
		player_set_timed(player, TMD_FOOD, MAX(amount, 0), false);
	} else if (context->subtype == 3) {
		/* Increase food level to amount if needed */
		if (player->timed[TMD_FOOD] < amount) {
			player_set_timed(player, TMD_FOOD, MAX(amount + 1, 0), false);
		}
	} else {
		return false;
	}
	context->ident = true;
	return true;
}

static bool effect_handler_CRUNCH(effect_handler_context_t *context)
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
static bool effect_handler_CURE(effect_handler_context_t *context)
{
	int type = context->subtype;
	(void) player_clear_timed(player, type, true);
	context->ident = true;
	return true;
}

/**
 * Set a (positive or negative) player status condition.
 */
static bool effect_handler_TIMED_SET(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	player_set_timed(player, context->subtype, MAX(amount, 0), true);
	context->ident = true;
	return true;

}

/**
 * Extend a (positive or negative) player status condition.
 * If context->other is set, increase by that amount if the player already
 * has the status
 */
static bool effect_handler_TIMED_INC(effect_handler_context_t *context)
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
		player_inc_timed(player, context->subtype, MAX(amount, 0), true, true);
	} else {
		player_inc_timed(player, context->subtype, context->other, true, true);
	}
	return true;
}

/**
 * Extend a (positive or negative) player status condition unresistably.
 * If context->other is set, increase by that amount if the player already
 * has the status
 */
static bool effect_handler_TIMED_INC_NO_RES(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);

	if (!player->timed[context->subtype] || !context->other)
		player_inc_timed(player, context->subtype, MAX(amount, 0), true, false);
	else
		player_inc_timed(player, context->subtype, context->other, true, false);
	context->ident = true;
	return true;
}

/**
 * Extend a (positive or negative) monster status condition.
 */
static bool effect_handler_MON_TIMED_INC(effect_handler_context_t *context)
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
static bool effect_handler_TIMED_DEC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	if (context->other)
		amount = player->timed[context->subtype] / context->other;
	(void) player_dec_timed(player, context->subtype, MAX(amount, 0), true);
	context->ident = true;
	return true;
}

/**
 * Create a glyph.
 */
static bool effect_handler_GLYPH(effect_handler_context_t *context)
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
static bool effect_handler_WEB(effect_handler_context_t *context)
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
static bool effect_handler_RESTORE_STAT(effect_handler_context_t *context)
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
static bool effect_handler_DRAIN_STAT(effect_handler_context_t *context)
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
static bool effect_handler_LOSE_RANDOM_STAT(effect_handler_context_t *context)
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
static bool effect_handler_GAIN_STAT(effect_handler_context_t *context)
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
static bool effect_handler_RESTORE_EXP(effect_handler_context_t *context)
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
static bool effect_handler_GAIN_EXP(effect_handler_context_t *context)
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
static bool effect_handler_DRAIN_LIGHT(effect_handler_context_t *context)
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
static bool effect_handler_DRAIN_MANA(effect_handler_context_t *context)
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

static bool effect_handler_RESTORE_MANA(effect_handler_context_t *context)
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
static bool effect_handler_REMOVE_CURSE(effect_handler_context_t *context)
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
static bool effect_handler_RECALL(effect_handler_context_t *context)
{
	int target_depth;
	context->ident = true;

	/* No recall */
	if (OPT(player, birth_no_recall) && !player->total_winner) {
		msg("Nothing happens.");
		return true;
	}

	/* No recall from quest levels with force_descend */
	if (OPT(player, birth_force_descend) && (is_quest(player->depth))) {
		msg("Nothing happens.");
		return true;
	}

	/* No recall from single combat */
	if (player->upkeep->arena_level) {
		msg("Nothing happens.");
		return true;
	}

	/* Warn the player if they're descending to an unrecallable level */
	target_depth = dungeon_get_next_level(player->max_depth, 1);
	if (OPT(player, birth_force_descend) && !(player->depth) &&
			(is_quest(target_depth))) {
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

static bool effect_handler_DEEP_DESCENT(effect_handler_context_t *context)
{
	int i;

	/* Calculate target depth */
	int target_increment = (4 / z_info->stair_skip) + 1;
	int target_depth = dungeon_get_next_level(player->max_depth,
											  target_increment);
	for (i = 5; i > 0; i--) {
		if (is_quest(target_depth)) break;
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

static bool effect_handler_ALTER_REALITY(effect_handler_context_t *context)
{
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
static bool effect_handler_MAP_AREA(effect_handler_context_t *context)
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
static bool effect_handler_READ_MINDS(effect_handler_context_t *context)
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
		return true;
	}

	return false;
}

/**
 * Detect traps around the player.  The height to detect above and below the
 * player is context->y, the width either side of the player context->x.
 */
static bool effect_handler_DETECT_TRAPS(effect_handler_context_t *context)
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
				if (!is_trapped_chest(obj)) continue;

				/* Identify once */
				if (!obj->known || obj->known->pval != obj->pval) {
					/* Hack - know the pile */
					square_know_pile(cave, grid);

					/* Know the trap */
					obj->known->pval = obj->pval;

					/* Notice it */
					disturb(player);

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
static bool effect_handler_DETECT_DOORS(effect_handler_context_t *context)
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

			/* Detect secret doors */
			if (square_issecretdoor(cave, grid)) {
				/* Put an actual door */
				place_closed_door(cave, grid);

				/* Memorize */
				square_memorize(cave, grid);
				square_light_spot(cave, grid);

				/* Obvious */
				doors = true;
			}

			/* Forget unknown doors in the mapping area */
			if (square_isdoor(player->cave, grid) &&
				square_isnotknown(cave, grid)) {
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
static bool effect_handler_DETECT_STAIRS(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_GOLD(effect_handler_context_t *context)
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
			object_delete(&obj);
			original->known = NULL;
			delist_object(c, original);
			object_delete(&original);
		}
		obj = next;
	}
}

/**
 * Sense objects around the player.  The height to sense above and below the
 * player is context->y, the width either side of the player context->x
 */
static bool effect_handler_SENSE_OBJECTS(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_OBJECTS(effect_handler_context_t *context)
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
			if (!ignore_item_ok(obj)) {
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
static bool effect_handler_DETECT_LIVING_MONSTERS(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_VISIBLE_MONSTERS(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_INVISIBLE_MONSTERS(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_FEARFUL_MONSTERS(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_EVIL(effect_handler_context_t *context)
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
static bool effect_handler_DETECT_SOUL(effect_handler_context_t *context)
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
static bool effect_handler_IDENTIFY(effect_handler_context_t *context)
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
static bool effect_handler_CREATE_STAIRS(effect_handler_context_t *context)
{
	context->ident = true;

	/* Only allow stairs to be created on empty floor */
	if (!square_isfloor(cave, player->grid)) {
		msg("There is no empty floor here.");
		return false;
	}

	/* Fails for persistent levels (for now) */
	if (OPT(player, birth_levels_persist)) {
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
static bool effect_handler_DISENCHANT(effect_handler_context_t *context)
{
	int i, count = 0;
	struct object *obj;
	char o_name[80];

	/* Count slots */
	for (i = 0; i < player->body.count; i++) {
		/* Ignore rings, amulets and lights */
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

		/* Count disenchantable slots */
		count++;
	}

	/* Pick one at random */
	for (i = player->body.count - 1; i >= 0; i--) {
		/* Ignore rings, amulets and lights */
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

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
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

	/* Artifacts have a 60% chance to resist */
	if (obj->artifact && (randint0(100) < 60)) {
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!", o_name, I2A(i),
			((obj->number != 1) ? "" : "s"));

		return true;
	}

	/* Apply disenchantment, depending on which kind of equipment */
	if (slot_type_is(i, EQUIP_WEAPON) || slot_type_is(i, EQUIP_BOW)) {
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
	msg("Your %s (%c) %s disenchanted!", o_name, I2A(i),
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
static bool effect_handler_ENCHANT(effect_handler_context_t *context)
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
 * Returns N which is the 1 in N chance for recharging to fail.
 */
int recharge_failure_chance(const struct object *obj, int strength) {
	/* Ease of recharge ranges from 9 down to 4 (wands) or 3 (staffs) */
	int ease_of_recharge = (100 - obj->kind->level) / 10;
	int raw_chance = strength + ease_of_recharge
		- 2 * (obj->pval / obj->number);
	return raw_chance > 1 ? raw_chance : 1;
}

/**
 * Recharge a wand or staff from the pack or on the floor.  Recharge strength
 * is context->value.base.
 *
 * It is harder to recharge high level, and highly charged wands.
 */
static bool effect_handler_RECHARGE(effect_handler_context_t *context)
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
				item_tester_hook_recharge, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, item_tester_hook_recharge,
				  itemmode)) {
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
		if (object_is_carried(player, obj))
			destroyed = gear_object_for_use(obj, 1, true, &none_left);
		else
			destroyed = floor_object_for_use(obj, 1, true, &none_left);
		if (destroyed->known)
			object_delete(&destroyed->known);
		object_delete(&destroyed);
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

/**
 * Apply a "project()" directly to all viewable monsters.  If context->other is
 * set, the effect damage boost is applied.  This is a hack - NRM
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
static bool effect_handler_PROJECT_LOS(effect_handler_context_t *context)
{
	int i;
	int dam = effect_calculate_value(context, context->other ? true : false);
	int typ = context->subtype;
	struct loc origin = origin_get_loc(context->origin);
	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	/* Affect all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Require line of sight */
		if (!los(cave, origin, mon->grid)) continue;

		/* Jump directly to the monster */
		(void)project(source_player(), 0, mon->grid, dam, typ, flg, 0, 0,
					  context->obj);
		context->ident = true;
	}

	/* Result */
	return true;
}

/**
 * Just like PROJECT_LOS except the player's awareness of an object using
 * this effect is relevant.
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
static bool effect_handler_PROJECT_LOS_AWARE(effect_handler_context_t *context)
{
	int i;
	int dam = effect_calculate_value(context, context->other ? true : false);
	int typ = context->subtype;

	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	if (context->aware) flg |= PROJECT_AWARE;

	/* Affect all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);
		struct loc grid;

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Location */
		grid = mon->grid;

		/* Require line of sight */
		if (!square_isview(cave, grid)) continue;

		/* Jump directly to the target monster */
		(void)project(source_player(), 0, grid, dam, typ, flg, 0, 0, context->obj);
		context->ident = true;
	}

	/* Result */
	return true;
}

static bool effect_handler_ACQUIRE(effect_handler_context_t *context)
{
	int num = effect_calculate_value(context, false);
	acquirement(player->grid, player->depth, num, true);
	context->ident = true;
	return true;
}

/**
 * Wake up all monsters in line of sight
 */
static bool effect_handler_WAKE(effect_handler_context_t *context)
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
static bool effect_handler_SUMMON(effect_handler_context_t *context)
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
static bool effect_handler_BANISH(effect_handler_context_t *context)
{
	int i;
	unsigned dam = 0;

	char typ;

	context->ident = true;

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
static bool effect_handler_MASS_BANISH(effect_handler_context_t *context)
{
	int i;
	int radius = context->radius ? context->radius : z_info->max_sight;
	unsigned dam = 0;

	context->ident = true;

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
static bool effect_handler_PROBE(effect_handler_context_t *context)
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
					MDESC_IND_HID | MDESC_CAPITAL);

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
static bool effect_handler_TELEPORT(effect_handler_context_t *context)
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

			/* Require "naked" floor space */
			if (!square_isempty(cave, grid)) continue;

			/* No monster teleport onto glyph of warding */
			if (!is_player && square_iswarded(cave, grid)) continue;

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
		msg("Failed to find teleport destination!");
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

	/* Move player */
	monster_swap(start, spots->grid);

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
static bool effect_handler_TELEPORT_TO(effect_handler_context_t *context)
{
	struct monster *mon = NULL;
	struct loc start, aim, land;
	int dis = 0, ctr = 0, dir = DIR_TARGET;
	struct monster *t_mon = monster_target_monster(context);

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
			get_aim_dir(&dir);
		} while (dir == DIR_TARGET && !target_okay());

		if (dir == DIR_TARGET)
			target_get(&aim);
		else
			aim = loc_offset(start, ddx[dir], ddy[dir]);

		/* Randomise the landing a bit if it's a vault */
		if (square_isvault(cave, aim)) dis = 10;
	}

	/* Find a usable location */
	while (1) {
		/* Pick a nearby legal location */
		while (1) {
			land = rand_loc(aim, dis, dis);
			if (square_in_bounds_fully(cave, land)) break;
		}

		/* Accept "naked" floor grids */
		if (square_isempty(cave, land)) break;

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

	/* Clear any projection marker to prevent double processing */
	sqinfo_off(square(cave, land)->info, SQUARE_PROJECT);

	/* Lots of updates after monster_swap */
	handle_stuff(player);

	return true;
}

/**
 * Teleport the player one level up or down (random when legal)
 */
static bool effect_handler_TELEPORT_LEVEL(effect_handler_context_t *context)
{
	bool up = true;
	bool down = true;
	int target_depth = dungeon_get_next_level(player->max_depth, 1);
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
	if (!up && is_quest(target_depth))
		down = false;

	/* Can't leave quest levels or go down deeper than the dungeon */
	if (is_quest(player->depth) || (player->depth >= z_info->max_depth - 1))
		down = false;

	/* Determine up/down if not already done */
	if (up && down) {
		if (randint0(100) < 50)
			up = false;
		else
			down = false;
	}

	/* Now actually do the level change */
	if (up) {
		msgt(MSG_TPLEVEL, "You rise up through the ceiling.");
		target_depth = dungeon_get_next_level(player->depth, -1);
		dungeon_change_level(player, target_depth);
	} else if (down) {
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		if (OPT(player, birth_force_descend)) {
			target_depth = dungeon_get_next_level(player->max_depth, 1);
			dungeon_change_level(player, target_depth);
		} else {
			target_depth = dungeon_get_next_level(player->depth, 1);
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
static bool effect_handler_RUBBLE(effect_handler_context_t *context)
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

static bool effect_handler_GRANITE(effect_handler_context_t *context)
{
	struct trap *trap = context->origin.which.trap;
	square_set_feat(cave, trap->grid, FEAT_GRANITE);

	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return true;
}

/**
 * The destruction effect
 *
 * This effect "deletes" monsters (instead of killing them).
 *
 * This is always an effect centred on the player; it is similar to the
 * earthquake effect.
 */
static bool effect_handler_DESTRUCTION(effect_handler_context_t *context)
{
	int k, r = context->radius;
	int elem = context->subtype;
	int py = player->grid.y;
	int px = player->grid.x;
	struct loc grid;

	context->ident = true;

	/* No effect in town or arena */
	if ((!player->depth) || (player->upkeep->arena_level)) {
		msg("The ground shakes for a moment.");
		return true;
	}

	/* Big area of affect */
	for (grid.y = (py - r); grid.y <= (py + r); grid.y++) {
		for (grid.x = (px - r); grid.x <= (px + r); grid.x++) {
			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Extract the distance */
			k = distance(loc(px, py), grid);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Lose room and vault */
			sqinfo_off(square(cave, grid)->info, SQUARE_ROOM);
			sqinfo_off(square(cave, grid)->info, SQUARE_VAULT);

			/* Forget completely */
			if (!square_isbright(cave, grid)) {
				sqinfo_off(square(cave, grid)->info, SQUARE_GLOW);
			}
			sqinfo_off(square(cave, grid)->info, SQUARE_SEEN);
			square_forget(cave, grid);
			square_light_spot(cave, grid);

			/* Deal with player later */
			if (loc_eq(grid, player->grid)) continue;

			/* Delete the monster (if any) */
			delete_monster(grid);

			/* Don't remove stairs */
			if (square_isstairs(cave, grid)) continue;

			/* Destroy any grid that isn't a permament wall */
			if (!square_isperm(cave, grid)) {
				/* Deal with artifacts */
				struct object *obj = square_object(cave, grid);
				while (obj) {
					if (obj->artifact) {
						if (OPT(player, birth_lose_arts) ||
							obj_is_known_artifact(obj)) {
							history_lose_artifact(player, obj->artifact);
							obj->artifact->created = true;
						} else {
							obj->artifact->created = false;
						}
					}
					obj = obj->next;
				}

				/* Delete objects */
				square_excise_pile(player->cave, grid);
				square_excise_pile(cave, grid);
				square_destroy(cave, grid);
			}
		}
	}

	/* Player is affected */
	if (elem == ELEM_LIGHT) {
		msg("There is a searing blast of light!");
		equip_learn_element(player, ELEM_LIGHT);
		if (!player_resists(player, ELEM_LIGHT)) {
			(void)player_inc_timed(player, TMD_BLIND, 10 + randint1(10), true,
								   true);
		}
	} else if (elem == ELEM_DARK) {
		msg("Darkness seems to crush you!");
		equip_learn_element(player, ELEM_DARK);
		if (!player_resists(player, ELEM_DARK)) {
			(void)player_inc_timed(player, TMD_BLIND, 10 + randint1(10), true,
								   true);
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw monster list */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return true;
}

/**
 * Induce an earthquake of the radius context->radius centred on the instigator.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and jump into a safe grid if possible,
 * otherwise, he will tunnel through the rubble instantaneously.
 *
 * Monsters will take damage, and jump into a safe grid if possible,
 * otherwise they will be buried in the rubble, disappearing from
 * the level in the same way that they do when banished.
 *
 * Note that players and monsters (except eaters of walls and passers
 * through walls) will never occupy the same grid as a wall (or door).
 */
static bool effect_handler_EARTHQUAKE(effect_handler_context_t *context)
{
	int r = context->radius;
	bool targeted = context->subtype ? true : false;

	struct loc pgrid = player->grid;
	int i, y, x;
	struct loc offset, safe_grid = loc(0, 0);
	int safe_grids = 0;
	int damage = 0;
	bool hurt = false;
	bool map[32][32];

	struct loc centre = origin_get_loc(context->origin);

	context->ident = true;

	if ((player->depth) && ((!player->upkeep->arena_level)
							|| (context->origin.what == SRC_MONSTER))) {
		msg("The ground shakes! The ceiling caves in!");
	} else {
		/* No effect in town or arena */
		msg("The ground shakes for a moment.");
		return true;
	}

	/* Sometimes ask for a target */
	if (targeted) {
		int dir = DIR_TARGET;
		get_aim_dir(&dir);
		if ((dir == DIR_TARGET) && target_okay()) {
			target_get(&centre);
		}
	}

	/* Paranoia -- Enforce maximum range */
	if (r > 15) r = 15;

	/* Initialize a map of the maximal blast area */
	for (y = 0; y < 32; y++)
		for (x = 0; x < 32; x++)
			map[y][x] = false;

	/* Check around the epicenter */
	for (offset.y = -r; offset.y <= r; offset.y++) {
		for (offset.x = -r; offset.x <= r; offset.x++) {
			/* Extract the location */
			struct loc grid = loc_sum(centre, offset);

			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Skip distant grids */
			if (distance(centre, grid) > r) continue;

			/* Lose room and vault */
			sqinfo_off(square(cave, grid)->info, SQUARE_ROOM);
			sqinfo_off(square(cave, grid)->info, SQUARE_VAULT);

			/* Forget completely */
			if (!square_isbright(cave, grid)) {
				sqinfo_off(square(cave, grid)->info, SQUARE_GLOW);
			}
			sqinfo_off(square(cave, grid)->info, SQUARE_SEEN);
			square_forget(cave, grid);
			square_light_spot(cave, grid);

			/* Skip the epicenter */
			if (loc_is_zero(offset)) continue;

			/* Skip most grids */
			if (randint0(100) < 85) continue;

			/* Damage this grid */
			map[16 + grid.y - centre.y][16 + grid.x - centre.x] = true;

			/* Take note of player damage */
			if (loc_eq(grid, pgrid)) hurt = true;
		}
	}

	/* First, affect the player (if necessary) */
	if (hurt) {
		/* Check around the player */
		for (i = 0; i < 8; i++) {
			/* Get the location */
			struct loc grid = loc_sum(pgrid, ddgrid_ddd[i]);

			/* Skip non-empty grids - allow pushing into traps and webs */
			if (!square_isopen(cave, grid)) continue;

			/* Important -- Skip grids marked for damage */
			if (map[16 + grid.y - centre.y][16 + grid.x - centre.x]) continue;

			/* Count "safe" grids, apply the randomizer */
			if ((++safe_grids > 1) && (randint0(safe_grids) != 0)) continue;

			/* Save the safe location */
			safe_grid = grid;
		}

		/* Random message */
		switch (randint1(3))
		{
			case 1:
			{
				msg("The cave ceiling collapses on you!");
				break;
			}
			case 2:
			{
				msg("The cave floor twists in an unnatural way!");
				break;
			}
			default:
			{
				msg("The cave quakes!");
				msg("You are pummeled with debris!");
				break;
			}
		}

		/* Hurt the player a lot */
		if (!safe_grids) {
			/* Message and damage */
			msg("You are severely crushed!");
			damage = 300;
		} else {
			/* Destroy the grid, and push the player to (relative) safety */
			switch (randint1(3)) {
				case 1: {
					msg("You nimbly dodge the blast!");
					damage = 0;
					break;
				}
				case 2: {
					msg("You are bashed by rubble!");
					damage = damroll(10, 4);
					(void)player_inc_timed(player, TMD_STUN, randint1(50),
										   true, true);
					break;
				}
				case 3: {
					msg("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)player_inc_timed(player, TMD_STUN, randint1(50),
										   true, true);
					break;
				}
			}

			/* Move player */
			monster_swap(pgrid, safe_grid);
		}

		/* Take some damage */
		if (damage) take_hit(player, damage, "an earthquake");
	}


	/* Examine the quaked region */
	for (offset.y = -r; offset.y <= r; offset.y++) {
		for (offset.x = -r; offset.x <= r; offset.x++) {
			/* Extract the location */
			struct loc grid = loc_sum(centre, offset);

			/* Skip unaffected grids */
			if (!map[16 + grid.y - centre.y][16 + grid.x - centre.x]) continue;

			/* Process monsters */
			if (square(cave, grid)->mon > 0) {
				struct monster *mon = square_monster(cave, grid);

				/* Most monsters cannot co-exist with rock */
				if (!flags_test(mon->race->flags, RF_SIZE, RF_KILL_WALL,
								RF_PASS_WALL, FLAG_END)) {
					char m_name[80];

					/* Assume not safe */
					safe_grids = 0;

					/* Monster can move to escape the wall */
					if (!rf_has(mon->race->flags, RF_NEVER_MOVE)) {
						/* Look for safety */
						for (i = 0; i < 8; i++) {
							/* Get the grid */
							struct loc safe = loc_sum(grid, ddgrid_ddd[i]);

							/* Skip non-empty grids */
							if (!square_isempty(cave, safe)) continue;

							/* Hack -- no safety on glyph of warding */
							if (square_iswarded(cave, safe)) continue;

							/* Important -- Skip quake grids */
							if (map[16 + safe.y - centre.y]
								[16 + safe.x - centre.x]) continue;

							/* Count safe grids, apply the randomizer */
							if ((++safe_grids > 1) &&
								(randint0(safe_grids) != 0))
								continue;

							/* Save the safe grid */
							safe_grid = safe;
						}
					}

					/* Describe the monster */
					monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

					/* Scream in pain */
					msg("%s wails out in pain!", m_name);

					/* Take damage from the quake */
					damage = (safe_grids ? damroll(4, 8) : (mon->hp + 1));

					/* Monster is certainly awake, not thinking about player */
					monster_wake(mon, false, 0);

					/* If the quake finished the monster off, show message */
					if (mon->hp < damage && mon->hp >= 0)
						msg("%s is embedded in the rock!", m_name);

					/* Apply damage directly */
					mon->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (mon->hp < 0) {
						/* Delete the monster */
						delete_monster(grid);

						/* No longer safe */
						safe_grids = 0;
					}

					/* Escape from the rock */
					if (safe_grids)
						/* Move the monster */
						monster_swap(grid, safe_grid);
				}
			}
		}
	}

	/* Player may have moved */
	pgrid = player->grid;

	/* Important -- no wall on player */
	map[16 + pgrid.y - centre.y][16 + pgrid.x - centre.x] = false;


	/* Examine the quaked region and damage marked grids if possible */
	for (offset.y = -r; offset.y <= r; offset.y++) {
		for (offset.x = -r; offset.x <= r; offset.x++) {
			/* Extract the location */
			struct loc grid = loc_sum(centre, offset);

			/* Ignore invalid grids */
			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Note unaffected grids for light changes, etc. */
			if (!map[16 + grid.y - centre.y][16 + grid.x - centre.x])
				square_light_spot(cave, grid);

			/* Destroy location and all objects (if valid) */
			else if (square_changeable(cave, grid)) {
				square_excise_pile(cave, grid);
				square_earthquake(cave, grid);
			}
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update the health bar */
	player->upkeep->redraw |= (PR_HEALTH);

	/* Window stuff */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return true;
}

static bool effect_handler_LIGHT_LEVEL(effect_handler_context_t *context)
{
	bool full = context->value.base ? true : false;
	if (full)
		msg("An image of your surroundings forms in your mind...");
	wiz_light(cave, player, full);
	context->ident = true;
	return true;
}

static bool effect_handler_DARKEN_LEVEL(effect_handler_context_t *context)
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
static bool effect_handler_LIGHT_AREA(effect_handler_context_t *context)
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
static bool effect_handler_DARKEN_AREA(effect_handler_context_t *context)
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
		(void)player_inc_timed(player, TMD_BLIND, 3 + randint1(5), true, true);
	}

	/* Assume seen */
	context->ident = !decoy_unseen;
	return (true);
}

/**
 * Project from the player's grid at the player, with full intensity out to
 * its radius
 * Affect the player (even when player-cast), grids, objects, and monsters
 */
static bool effect_handler_SPOT(effect_handler_context_t *context)
{
	struct loc pgrid = player->grid;
	int dam = effect_calculate_value(context, false);
	int rad = context->radius ? context->radius : 0;

	int flg = PROJECT_STOP | PROJECT_PLAY | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_SELF;

	/* Handle increasing radius with player level */
	if (context->other && context->origin.what == SRC_PLAYER) {
		rad += player->lev / context->other;
	}

	/* Aim at the target, explode */
	if (project(context->origin, rad, pgrid, dam, context->subtype, flg, 0,
				rad, NULL))
		context->ident = true;

	return true;
}

/**
 * Project from the player's grid, act as a ball, with full intensity out as
 * far as the given diameter
 * Affect grids, objects, and monsters
 */
static bool effect_handler_SPHERE(effect_handler_context_t *context)
{
	struct loc pgrid = player->grid;
	int dam = effect_calculate_value(context, false);
	int rad = context->radius ? context->radius : 0;
	int diameter_of_source = context->other ? context->other : 0;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Aim at the target, explode */
	if (project(context->origin, rad, pgrid, dam, context->subtype, flg, 0,
				diameter_of_source, NULL))
		context->ident = true;

	return true;
}

/**
 * Cast a ball spell
 * Stop if we hit a monster or the player, act as a ball
 * Allow target mode to pass over monsters
 * Affect grids, objects, and monsters
 */
static bool effect_handler_BALL(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int rad = context->radius ? context->radius : 2;
	struct loc target = loc(-1, -1);

	int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Player or monster? */
	switch (context->origin.what) {
		case SRC_MONSTER: {
			struct monster *mon = cave_monster(cave, context->origin.which.monster);
			int conf_level, accuracy = 100;
			struct monster *t_mon = monster_target_monster(context);

			assert(mon);

			conf_level = monster_effect_level(mon, MON_TMD_CONF);
			while (conf_level) {
				accuracy *= (100 - CONF_RANDOM_CHANCE);
				accuracy /= 100;
				conf_level--;
			}

			/* Powerful monster */
			if (monster_is_powerful(mon)) {
				rad++;
			}

			flg |= PROJECT_PLAY;
			flg &= ~(PROJECT_STOP | PROJECT_THRU);

			if (randint1(100) > accuracy) {
				/* Confused direction */
				int dir = randint1(9);
				target = loc_sum(mon->grid, ddgrid[dir]);
			} else if (t_mon) {
				/* Target monster */
				target = t_mon->grid;
			} else {
				/* Target player */
				if (monster_is_decoyed(mon)) {
					target = cave_find_decoy(cave);
				} else {
					target = player->grid;
				}
			}

			break;
		}

		case SRC_TRAP: {
			struct trap *trap = context->origin.which.trap;
			flg |= PROJECT_PLAY;
			target = trap->grid;
			break;
		}

		case SRC_PLAYER:
			/* Ask for a target if no direction given */
			if (context->dir == DIR_TARGET && target_okay()) {
				flg &= ~(PROJECT_STOP | PROJECT_THRU);
				target_get(&target);
			} else {
				target = loc_sum(player->grid, ddgrid[context->dir]);
			}

			if (context->other) rad += player->lev / context->other;
			break;

		default:
			break;
	}

	/* Aim at the target, explode */
	if (project(context->origin, rad, target, dam, context->subtype, flg, 0, 0, context->obj))
		context->ident = true;

	return true;
}


/**
 * Breathe an element, in a cone from the breather
 * Affect grids, objects, and monsters
 * context->subtype is element, context->other degrees of arc
 * If context->radius is set it is radius of breath, but it usually isn't
 */
static bool effect_handler_BREATH(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, false);
	int type = context->subtype;

	struct loc target = loc(-1, -1);

	/* Diameter of source starts at 4, so full strength up to 3 grids from
	 * the breather. */
	int diameter_of_source = 4;

	/* Minimum breath width is 20 degrees */
	int degrees_of_arc = MAX(context->other, 20);

	int flg = PROJECT_ARC | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Distance breathed generally has no fixed limit. */
	int rad = context->radius ? context->radius : z_info->max_range;

	/* Player or monster? */
	if (context->origin.what == SRC_MONSTER) {
		struct monster *mon = cave_monster(cave, context->origin.which.monster);
		struct monster *t_mon = monster_target_monster(context);
		int conf_level, accuracy = 100;

		flg |= PROJECT_PLAY;

		conf_level = monster_effect_level(mon, MON_TMD_CONF);
		while (conf_level) {
			accuracy *= (100 - CONF_RANDOM_CHANCE);
			accuracy /= 100;
			conf_level--;
		}

		if (randint1(100) > accuracy) {
			/* Confused direction. */
			int dir = randint1(9);

			target = loc_sum(mon->grid, ddgrid[dir]);
		} else if (t_mon) {
			/* Target monster. */
			target = t_mon->grid;
		} else {
			/* Target player. */
			if (monster_is_decoyed(mon)) {
				target = cave_find_decoy(cave);
			} else {
				target = player->grid;
			}
		}

		dam = breath_dam(type, mon->hp);

		/* Powerful monster */
		if (monster_is_powerful(mon)) {
			/* Breath is now full strength at 5 grids */
			diameter_of_source *= 3;
			diameter_of_source /= 2;
		}
	} else if (context->origin.what == SRC_PLAYER) {
		msgt(projections[type].msgt, "You breathe %s.", projections[type].desc);

		/* Ask for a target if no direction given */
		if (context->dir == DIR_TARGET && target_okay()) {
			target_get(&target);
		} else {
			target = loc_sum(player->grid, ddgrid[context->dir]);
		}
	}

	/* Adjust the diameter of the energy source */
	if (degrees_of_arc < 60) {
		/* Narrower cone means energy drops off less quickly. We now have:
		 * - 30 degree regular breath  | full strength at 7 grids
		 * - 30 degree powerful breath | full strength at 11 grids
		 * - 20 degree regular breath  | full strength at 11 grids
		 * - 20 degree powerful breath | full strength at 17 grids
		 * where grids are measured from the breather. */
		diameter_of_source = diameter_of_source * 60 / degrees_of_arc;

		/* Max */
		if (diameter_of_source > 25)
			diameter_of_source = 25;
	}

	/* Breathe at the target */
	if (project(context->origin, rad, target, dam, type, flg, degrees_of_arc,
				diameter_of_source, context->obj))
		context->ident = true;

	return true;
}


/**
 * Cast an arc-shaped spell.  This is nothing more than a sphere spell
 * centered on the caster with a value for degrees_of_arc (how many degrees
 * wide the the arc is) that is not 360, essentially the same as a breath.
 * The direction given will be the center of the arc, which travels outwards
 * from the caster to a distance given by rad. -LM-
 *
 * Because all arcs start out as being one grid wide, arc spells with a
 * value for degrees_of_arc less than (roughly) 60 do not dissipate as
 * quickly.
 *
 * Affect grids, objects, and monsters
 * context->subtype is element, context->radius radius,
 * context->other degrees of arc (minimum 20)
 */
static bool effect_handler_ARC(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int type = context->subtype;
	int rad = context->radius;

	struct loc target = loc(-1, -1);

	/* Diameter of source starts at 4, so full strength up to 3 grids from
	 * the caster. */
	int diameter_of_source = 4;

	/* Short beams now have their own effect, so we set a minimum arc width */
	int degrees_of_arc = MAX(context->other, 20);

	int flg = PROJECT_ARC | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Radius of zero means no fixed limit. */
	if (rad == 0) {
		rad = z_info->max_range;
	}

	/* Player or monster? */
	if (context->origin.what == SRC_MONSTER) {
		flg |= PROJECT_PLAY;
		target =  player->grid;
	} else if (context->origin.what == SRC_PLAYER) {
		/* Ask for a target if no direction given */
		if (context->dir == DIR_TARGET && target_okay()) {
			target_get(&target);
		} else {
			target = loc_sum(player->grid, ddgrid[context->dir]);
		}
	}

	/* Diameter of the energy source. */
	if (degrees_of_arc < 60) {
			diameter_of_source = diameter_of_source * 60 / degrees_of_arc;
	}

	/* Max */
	if (diameter_of_source > 25) {
		diameter_of_source = 25;
	}

	/* Aim at the target */
	if (project(context->origin, rad, target, dam, type, flg, degrees_of_arc,
				diameter_of_source, context->obj)) {
		context->ident = true;
	}

	return true;
}

/**
 * Cast an defined length beam spell.
 *
 * Affect grids, objects, and monsters
 * context->subtype is element, context->radius radius
 * context->other allows an added radius of 1 every time the player level
 * increases by a multiple of context->other, and will only take effect for
 * player spells
 */
static bool effect_handler_SHORT_BEAM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, false);
	int type = context->subtype;
	bool addons = (context->origin.what == SRC_PLAYER) && (context->other > 0);
	int rad = context->radius + (addons ? player->lev / context->other : 0);

	struct loc target = loc(-1, -1);

	/* Diameter of source is the same as the radius, so the effect is
	 * essentially full strength for its entire length. */
	int diameter_of_source = rad;

	int flg = PROJECT_ARC | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Player or monster? */
	if (context->origin.what == SRC_MONSTER) {
		flg |= PROJECT_PLAY;
		target = player->grid;
	} else if (context->origin.what == SRC_PLAYER) {
		/* Ask for a target if no direction given */
		if (context->dir == DIR_TARGET && target_okay()) {
			target_get(&target);
		} else {
			target = loc_sum(player->grid, ddgrid[context->dir]);
		}
	}

	/* Check bounds */
	if (diameter_of_source > 25) {
		diameter_of_source = 25;
	}

	/* Aim at the target */
	if (project(context->origin, rad, target, dam, type, flg, 0,
				diameter_of_source, context->obj)) {
		context->ident = true;
	}

	return true;
}

/**
 * Crack a whip, or spit at the player; actually just a finite length beam
 * Affect grids, objects, and monsters
 * context->radius is length of beam
 */
static bool effect_handler_LASH(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, false);
	int rad = context->radius;

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_ARC;
	int type;

	struct loc target = loc(-1, -1);

	/* Diameter of source is the same as the radius, so the effect is
	 * essentially full strength for its entire length. */
	int diameter_of_source = rad;

	/* Monsters only */
	if (context->origin.what == SRC_MONSTER) {
		struct monster *mon = cave_monster(cave, context->origin.which.monster);
		struct monster *t_mon = monster_target_monster(context);
		int i;

		flg |= PROJECT_PLAY;

		/* Target player or monster? */
		if (t_mon) {
			target = t_mon->grid;
		} else {
			if (monster_is_decoyed(mon)) {
				target = cave_find_decoy(cave);
			} else {
				target = player->grid;
			}
		}

		/* Paranoia */
		if (rad > z_info->max_range) rad = z_info->max_range;

		/* Get the type (default is PROJ_MISSILE) */
		type = mon->race->blow[0].effect->lash_type;

		/* Scan through all blows for damage */
		for (i = 0; i < z_info->mon_blows_max; i++) {
			/* Extract the attack infomation */
			random_value dice = mon->race->blow[i].dice;

			/* Full damage of first blow, plus half damage of others */
			dam += randcalc(dice, mon->race->level, RANDOMISE) / (i ? 2 : 1);
			if (!mon->race->blow[i].next) break;
		}

		/* No damaging blows */
		if (!dam) return false;
	} else {
		return false;
	}

	/* Check bounds */
	if (diameter_of_source > 25) {
		diameter_of_source = 25;
	}

	/* Lash the target */
	if (project(context->origin, rad, target, dam, type, flg, 0,
				diameter_of_source, context->obj)) {
		context->ident = true;
	}

	return true;
}

/**
 * Cast multiple non-jumping ball spells at the same target.
 *
 * Targets absolute coordinates instead of a specific monster, so that
 * the death of the monster doesn't change the target's location.
 */
static bool effect_handler_SWARM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int num = context->value.m_bonus;

	struct loc target = loc_sum(player->grid, ddgrid[context->dir]);

	int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Ask for a target if no direction given (early detonation) */
	if ((context->dir == DIR_TARGET) && target_okay()) {
		target_get(&target);
	}

	while (num--) {
		/* Aim at the target.  Hurt items on floor. */
		if (project(source_player(), context->radius, target, dam,
					context->subtype, flg, 0, 0, context->obj))
			context->ident = true;
	}

	return true;
}

/**
 * Strike the target with a ball from above
 */
static bool effect_handler_STRIKE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	struct loc target = player->grid;
	int flg = PROJECT_JUMP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Ask for a target; if no direction given, the player is struck  */
	if ((context->dir == DIR_TARGET) && target_okay()) {
		target_get(&target);
	}

	/* Enforce line of sight */
	if (!projectable(cave, player->grid, target, PROJECT_NONE) ||
		!square_isknown(cave, target)) {
		return false;
	}

	/* Aim at the target.  Hurt items on floor. */
	if (project(source_player(), context->radius, target, dam, context->subtype,
				flg, 0, 0, context->obj)) {
		context->ident = true;
	}

	return true;
}

/**
 * Cast a line spell in every direction
 * Stop if we hit a monster, act as a ball
 * Affect grids, objects, and monsters
 */
static bool effect_handler_STAR(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int i;
	struct loc target;

	int flg = PROJECT_THRU | PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;

	/* Describe */
	if (!player->timed[TMD_BLIND])
		msg("Light shoots in all directions!");

	for (i = 0; i < 8; i++) {
		/* Use the current direction */
		target = loc_sum(player->grid, ddgrid_ddd[i]);

		/* Aim at the target */
		if (project(source_player(), 0, target, dam, context->subtype, flg, 0,
					0, context->obj))
			context->ident = true;
	}

	return true;
}


/**
 * Cast a ball spell in every direction
 * Stop if we hit a monster, act as a ball
 * Affect grids, objects, and monsters
 */
static bool effect_handler_STAR_BALL(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int i;
	struct loc target;

	int flg = PROJECT_STOP | PROJECT_THRU | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	for (i = 0; i < 8; i++) {
		/* Use the current direction */
		target = loc_sum(player->grid, ddgrid_ddd[i]);

		/* Aim at the target, explode */
		if (project(source_player(), context->radius, target, dam,
					context->subtype, flg, 0, 0, context->obj))
			context->ident = true;
	}
	return true;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 */
static bool effect_handler_BOLT(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int flg = PROJECT_STOP | PROJECT_KILL;
	(void) project_aimed(context->origin, context->subtype, context->dir, dam,
						 flg, context->obj);
	if (!player->timed[TMD_BLIND])
		context->ident = true;
	return true;
}

/**
 * Cast a beam spell
 * Pass through monsters, as a beam
 * Affect monsters (not grids or objects)
 */
static bool effect_handler_BEAM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int flg = PROJECT_BEAM | PROJECT_KILL;
	(void) project_aimed(context->origin, context->subtype, context->dir, dam,
						 flg, context->obj);
	if (!player->timed[TMD_BLIND])
		context->ident = true;
	return true;
}

/**
 * Cast a bolt spell, or rarely, a beam spell
 * context->other is used as any adjustment to the regular beam chance
 */
static bool effect_handler_BOLT_OR_BEAM(effect_handler_context_t *context)
{
	int beam = context->beam + context->other;

	if (randint0(100) < beam)
		return effect_handler_BEAM(context);
	else
		return effect_handler_BOLT(context);
}

/**
 * Cast a line spell
 * Pass through monsters, as a beam
 * Affect monsters and grids (not objects)
 */
static bool effect_handler_LINE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	if (project_aimed(context->origin, context->subtype, context->dir, dam, flg, context->obj))
		context->ident = true;
	return true;
}

/**
 * Cast an alter spell
 * Affect objects and grids (not monsters)
 */
static bool effect_handler_ALTER(effect_handler_context_t *context)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	if (project_aimed(context->origin, context->subtype, context->dir, 0, flg, context->obj))
		context->ident = true;
	return true;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * Like BOLT, but only identifies on noticing an effect
 */
static bool effect_handler_BOLT_STATUS(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (project_aimed(context->origin, context->subtype, context->dir, dam, flg, context->obj))
		context->ident = true;
	return true;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * The same as BOLT_STATUS, but done as a separate function to aid descriptions
 */
static bool effect_handler_BOLT_STATUS_DAM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (project_aimed(context->origin, context->subtype, context->dir, dam, flg, context->obj))
		context->ident = true;
	return true;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * Notice stuff based on awareness of the effect
 */
static bool effect_handler_BOLT_AWARE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (context->aware) flg |= PROJECT_AWARE;
	if (project_aimed(context->origin, context->subtype, context->dir, dam, flg, context->obj))
		context->ident = true;
	return true;
}

/**
 * Affect adjacent grids (radius 1 ball attack)
 */
static bool effect_handler_TOUCH(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int rad = context->radius ? context->radius : 1;

	if (context->origin.what == SRC_MONSTER) {
		struct monster *mon = cave_monster(cave, context->origin.which.monster);
		struct monster *t_mon = monster_target_monster(context);
		struct loc decoy = cave_find_decoy(cave);

		/* Target decoy */
		if (decoy.y && decoy.x) {
			int flg = PROJECT_GRID | PROJECT_KILL | PROJECT_HIDE | PROJECT_ITEM | PROJECT_THRU;
			return (project(source_trap(square_trap(cave, decoy)),
					rad, decoy, dam, context->subtype,flg, 0, 0, context->obj));
		}

		/* Monster cast at monster */
		if (t_mon) {
			int flg = PROJECT_GRID | PROJECT_KILL | PROJECT_HIDE | PROJECT_ITEM | PROJECT_THRU;
			return (project(source_monster(mon->target.midx), rad,
							t_mon->grid, dam, context->subtype,
							flg, 0, 0, context->obj));
		}
	}

	if (project_touch(dam, rad, context->subtype, false, context->obj))
		context->ident = true;
	return true;
}

/**
 * Affect adjacent grids (radius 1 ball attack)
 * Notice stuff based on awareness of the effect
 */
static bool effect_handler_TOUCH_AWARE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, true);
	int rad = context->radius ? context->radius : 1;
	if (project_touch(dam, rad, context->subtype, context->aware, context->obj))
		context->ident = true;
	return true;
}

/**
 * Curse the player's armor
 */
static bool effect_handler_CURSE_ARMOR(effect_handler_context_t *context)
{
	struct object *obj;

	char o_name[80];

	/* Curse the body armor */
	obj = equipped_item_by_slot_name(player, "body");

	/* Nothing to curse */
	if (!obj) return (true);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL);

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
static bool effect_handler_CURSE_WEAPON(effect_handler_context_t *context)
{
	struct object *obj;

	char o_name[80];

	/* Curse the weapon */
	obj = equipped_item_by_slot_name(player, "weapon");

	/* Nothing to curse */
	if (!obj) return (true);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL);

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
static bool effect_handler_BRAND_WEAPON(effect_handler_context_t *context)
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
static bool effect_handler_BRAND_AMMO(effect_handler_context_t *context)
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
				item_tester_hook_ammo, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, item_tester_hook_ammo, itemmode))
		return used;

	/* Brand the ammo */
	brand_object(obj, brand);

	/* Done */
	return (true);
}

/**
 * Enchant some (non-magical) bolts
 */
static bool effect_handler_BRAND_BOLTS(effect_handler_context_t *context)
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
				item_tester_hook_bolt, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, item_tester_hook_bolt, itemmode))
		return used;

	/* Brand the bolts */
	brand_object(obj, "Flame");

	/* Done */
	return (true);
}


/**
 * Turn a staff into arrows
 */
static bool effect_handler_CREATE_ARROWS(effect_handler_context_t *context)
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
				item_tester_hook_staff, itemmode)) {
			return false;
		}
	} else if (!get_item(&obj, q, s, 0, item_tester_hook_staff,
				  itemmode)) {
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
		staff = gear_object_for_use(obj, 1, true, &none_left);
	} else {
		staff = floor_object_for_use(obj, 1, true, &none_left);
	}

	if (staff->known) {
		object_delete(&staff->known);
	}
	object_delete(&staff);

	/* Make some arrows */
	arrows = make_object(cave, player->lev, good, great, false, NULL, TV_ARROW);
	drop_near(cave, &arrows, 0, player->grid, true, true);

	return true;
}

/**
 * Draw energy from a magical device
 */
static bool effect_handler_TAP_DEVICE(effect_handler_context_t *context)
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
				item_tester_hook_recharge, itemmode)) {
			return used;
		}
	} else if (!get_item(&obj, q, s, 0, item_tester_hook_recharge,
				  itemmode)) {
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
 * Draw energy from a nearby undead
 */
static bool effect_handler_TAP_UNLIFE(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	struct loc target;
	struct monster *mon = NULL;
	char m_name[80];
	int drain = 0;
	bool fear = false;
	bool dead = false;

	context->ident = true;

	/* Closest living monster */
	if (!target_set_closest(TARGET_KILL, monster_is_undead)) {
		return false;
	}
	target_get(&target);
	mon = target_get_monster();

	/* Hurt the monster */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);
	msg("You draw power from the %s.", m_name);
	drain = MIN(mon->hp, amount) / 4;
	dead = mon_take_hit(mon, amount, &fear, " is destroyed!");

	/* Gain mana */
	effect_simple(EF_RESTORE_MANA, context->origin, format("%d", drain), 0, 0,
				  0, 0, 0, NULL);

	/* Handle fear for surviving monsters */
	if (!dead && monster_is_visible(mon)) {
		message_pain(mon, amount);
		if (fear) {
			add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
		}
	}

	return true;
}

/**
 * Perform a player shapechange
 */
static bool effect_handler_SHAPECHANGE(effect_handler_context_t *context)
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
 * Curse a monster for direct damage
 */
static bool effect_handler_CURSE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, false);
	struct monster *mon = target_get_monster();
	bool fear = false;
	bool dead = false;

	context->ident = true;

	/* Need to choose a monster, not just point */
	if (!mon) {
		msg("No monster selected!");
		return false;
	}

	/* Hit it */
	dead = mon_take_hit(mon, dam, &fear, " dies!");

	/* Handle fear for surviving monsters */
	if (!dead && monster_is_visible(mon)) {
		message_pain(mon, dam);
		if (fear) {
			add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
		}
	}

	return true;
}

/**
 * Take control of a monster
 */
static bool effect_handler_COMMAND(effect_handler_context_t *context)
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
	player_set_timed(player, TMD_COMMAND, MAX(amount, 0), false);

	/* Monster is commanded */
	mon_inc_timed(mon, MON_TMD_COMMAND, MAX(amount, 0), 0);

	return true;
}

/**
 * Jump next to a living monster and draw hitpoints and nourishment from it
 */
static bool effect_handler_JUMP_AND_BITE(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, false);
	struct loc victim, grid;
	int d, first_d = randint0(8);
	struct monster *mon = NULL;
	char m_name[80];
	int drain = 0;
	bool fear = false;
	bool dead = false;

	context->ident = true;

	/* Closest living monster */
	if (!target_set_closest(TARGET_KILL, monster_is_living)) {
		return false;
	}
	target_get(&victim);
	mon = target_get_monster();
	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);

	/* Look next to the monster */
	for (d = first_d; d < first_d + 8; d++) {
		grid = loc_sum(victim, ddgrid_ddd[d % 8]);
		if (square_isplayertrap(cave, grid)) continue;
		if (square_iswebbed(cave, grid)) continue;
		if (square_isopen(cave, grid)) break;
	}

	/* Needed to be adjacent */
	if (d == first_d + 8) {
		msg("Not enough room next to %s!", m_name);
		return false;
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(player->grid, grid);

	/* Now bite it */
	drain = MIN(mon->hp, amount);
	if (drain == 0) return true;
	if (OPT(player, show_damage)) {
		msg("You bite %s. (%d)", m_name, drain);
	} else {
		msg("You bite %s.", m_name);
	}
	dead = mon_take_hit(mon, amount, &fear, " is drained dry!");

	/* Heal and nourish */
	effect_simple(EF_HEAL_HP, context->origin, format("%d", drain), 0, 0, 0,
				  0, 0, NULL);
	player_inc_timed(player, TMD_FOOD, MAX(drain, 0), false, false);

	/* Handle fear for surviving monsters */
	if (!dead && monster_is_visible(mon)) {
		message_pain(mon, amount);
		if (fear) {
			add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
		}
	}

	return true;
}

/**
 * Move up to 4 spaces then do melee blows.
 * Could vary the length of the move without much work.
 */
static bool effect_handler_MOVE_ATTACK(effect_handler_context_t *context)
{
	int blows = effect_calculate_value(context, false);
	int moves = 4;
	int d, i;
	struct loc target = player->grid;
	struct loc next_grid, grid_diff;
	bool fear;
	struct monster *mon;

	/* Ask for a target */
	if (context->dir == DIR_TARGET) {
		target_get(&target);
	} else {
		target = loc_sum(player->grid, ddgrid[context->dir]);
	}

	mon = square_monster(cave, target);
	if (mon == NULL || !monster_is_obvious(mon)) {
		msg("This spell must target a monster.");
		return false;
	}

	while (distance(player->grid, target) > 1 && moves > 0) {
		int choice[] = { 0, 1, -1 };
		bool attack = false;
		grid_diff = loc_diff(target, player->grid);

		/* Choice of direction simplified by prioritizing diagonals */
		if (grid_diff.x == 0) {
			d = (grid_diff.y < 0) ? 0 : 4; /* up : down */
		} else if (grid_diff.y == 0) {
			d = (grid_diff.x < 0) ? 6 : 2; /* left : right */
		} else if (grid_diff.x < 0) {
			d = (grid_diff.y < 0) ? 7 : 5; /* up-left : down-left */
		} else {/* grid_diff.x > 0 */
			d = (grid_diff.y < 0) ? 1 : 3; /* up-right : down-right */
		}

		/* We'll give up to 3 choices: d, d + 1, d - 1 */
		for (i = 0; i < 3; i++) {
			int d_test = (d + choice[i] + 8) % 8;
			next_grid = loc_sum(player->grid, clockwise_grid[d_test]);
			if (square_ispassable(cave, next_grid)) {
				d = d_test;
				if (square_monster(cave, next_grid)) attack = true;
				break;
			} else if (i == 2) {
				msg("The way is barred.");
				return moves != 4;
			}
		}

		move_player(clockwise_ddd[d], false);
		moves--;
		if (attack) return false;
	}

	/* Reduce blows based on distance traveled, round to nearest blow */
	blows = (blows * moves + 2) / 4;

	/* Should return some energy if monster dies early */
	while (blows-- > 0) {
		if (py_attack_real(player, target, &fear)) break;
	}

	return true;
}

/**
 * Enter single combat with an enemy
 */
static bool effect_handler_SINGLE_COMBAT(effect_handler_context_t *context)
{
	struct monster *mon = target_get_monster();
	context->ident = true;

	/* Already in an arena */
	if (player->upkeep->arena_level) {
		msg("You are already in single combat!");
		return false;
	}

	/* Need to choose a monster, not just point */
	if (mon) {
		int old_idx = mon->midx;

		/* Monsters with high spell power can resist */
		if (randint0(mon->race->spell_power) > player->lev) {
			char m_name[80];
			monster_desc(m_name, sizeof(m_name), mon, MDESC_CAPITAL);
			msg("%s resists!", m_name);
			return true;
		}

		/* Swap the targeted monster with the first in the monster list */
		if (old_idx == 1) {
			/* Do nothing */
			;
		} else if (cave_monster(cave, 1)->race) {
			monster_index_move(old_idx, cave_monster_max(cave));
			monster_index_move(1, old_idx);
			monster_index_move(cave_monster_max(cave), 1);
		} else {
			monster_index_move(old_idx, 1);
		}
		target_set_monster(cave_monster(cave, 1));
	} else {
		msg("No monster selected!");
		return false;
	}

	/* Head to the arena */
	player->upkeep->arena_level = true;
	dungeon_change_level(player, player->depth);
	return true;
}


static bool effect_handler_MELEE_BLOWS(effect_handler_context_t *context)
{
	int blows = effect_calculate_value(context, false);
	int dam = context->radius;
	bool fear;
	int taim;
	struct loc target = loc(-1, -1);
	struct loc grid = player->grid;
	struct monster *mon = NULL;

	/* players only for now */
	if (context->origin.what != SRC_PLAYER)
		return false;

	/* Ask for a target if no direction given */
	if (context->dir == DIR_TARGET && target_okay()) {
		target_get(&target);
	} else {
		target = loc_sum(player->grid, ddgrid[context->dir]);
	}

	/* Check target validity */
	taim = distance(grid, target);
	mon = square_monster(cave, target);
	if (taim > 1) {
		msgt(MSG_GENERIC, "Target too far away (%d).", taim);
		return false;
	} else if (!mon) {
		msg("You must attack a monster.");
		return false;
	}

	while ((blows-- > 0) && mon) {
		/* Test for damaging the monster */
		int hp = mon->hp;
		if (py_attack_real(player, target, &fear)) return true;
		/*mon = square_monster(cave, target); */
		if (mon && (mon->hp == hp)) continue;

		/* Apply side-effects */
		if (project(context->origin, 0, target, dam, context->subtype,
					PROJECT_KILL, 0, 0, context->obj)) {
			context->ident = true;
		}
	}
	return true;
}

static bool effect_handler_SWEEP(effect_handler_context_t *context)
{
	int blows = effect_calculate_value(context, false);
	bool fear;
	int i;
	struct loc target;

	/* Players only for now */
	if (context->origin.what != SRC_PLAYER)	return false;

	/* Doing these like >1 blows means spinning around multiple times. */
	while (blows-- > 0) {
		for (i = 0; i < 8; i++) {
			target = loc_sum(player->grid, clockwise_grid[i]);
			if (square_monster(cave, target) != NULL)
				py_attack_real(player, target, &fear);
		}
	}

	/* Should return some energy if all enemies killed and blows remain? */
	return true;
}



/**
 * One Ring activation
 */
static bool effect_handler_BIZARRE(effect_handler_context_t *context)
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
 * The "wonder" effect.
 *
 * This spell should become more useful (more
 * controlled) as the player gains experience levels.
 * Thus, add 1/5 of the player's level to the die roll.
 * This eliminates the worst effects later on, while
 * keeping the results quite random.  It also allows
 * some potent effects only at high level
 */
static bool effect_handler_WONDER(effect_handler_context_t *context)
{
	int plev = player->lev;
	int die = effect_calculate_value(context, false);
	int subtype = 0, radius = 0, other = 0, y = 0, x = 0;
	int beam = context->beam;
	effect_handler_f handler = NULL;
	random_value value = { 0, 0, 0, 0 };

	context->ident = true;

	if (die > 100)
		msg("You feel a surge of power!");

	if (die < 8) {
		subtype = PROJ_MON_CLONE;
		handler = effect_handler_BOLT;
	} else if (die < 14) {
		subtype = PROJ_MON_SPEED;
		value.base = 100;
		handler = effect_handler_BOLT;
	} else if (die < 26) {
		subtype = PROJ_MON_HEAL;
		value.dice = 4;
		value.sides = 6;
		handler = effect_handler_BOLT;
	} else if (die < 31) {
		subtype = PROJ_MON_POLY;
		value.base = plev;
		handler = effect_handler_BOLT;
	} else if (die < 36) {
		beam -= 10;
		subtype = PROJ_MISSILE;
		value.dice = 3 + ((plev - 1) / 5);
		value.sides = 4;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 41) {
		subtype = PROJ_MON_CONF;
		value.base = plev;
		handler = effect_handler_BOLT;
	} else if (die < 46) {
		subtype = PROJ_POIS;
		value.base = 20 + plev / 2;
		radius = 3;
		handler = effect_handler_BALL;
	} else if (die < 51) {
		subtype = PROJ_LIGHT_WEAK;
		value.dice = 6;
		value.sides = 8;
		handler = effect_handler_LINE;
	} else if (die < 56) {
		subtype = PROJ_ELEC;
		value.dice = 3 + ((plev - 5) / 6);
		value.sides = 6;
		handler = effect_handler_BEAM;
	} else if (die < 61) {
		beam -= 10;
		subtype = PROJ_COLD;
		value.dice = 5 + ((plev - 5) / 4);
		value.sides = 8;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 66) {
		subtype = PROJ_ACID;
		value.dice = 6 + ((plev - 5) / 4);
		value.sides = 8;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 71) {
		subtype = PROJ_FIRE;
		value.dice = 8 + ((plev - 5) / 4);
		value.sides = 8;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 76) {
		subtype = PROJ_MON_DRAIN;
		value.base = 75;
		handler = effect_handler_BOLT;
	} else if (die < 81) {
		subtype = PROJ_ELEC;
		value.base = 30 + plev / 2;
		radius = 2;
		handler = effect_handler_BALL;
	} else if (die < 86) {
		subtype = PROJ_ACID;
		value.base = 40 + plev;
		radius = 2;
		handler = effect_handler_BALL;
	} else if (die < 91) {
		subtype = PROJ_ICE;
		value.base = 70 + plev;
		radius = 3;
		handler = effect_handler_BALL;
	} else if (die < 96) {
		subtype = PROJ_FIRE;
		value.base = 80 + plev;
		radius = 3;
		handler = effect_handler_BALL;
	} else if (die < 101) {
		subtype = PROJ_MON_DRAIN;
		value.base = 100 + plev;
		handler = effect_handler_BOLT;
	} else if (die < 104) {
		radius = 12;
		handler = effect_handler_EARTHQUAKE;
	} else if (die < 106) {
		radius = 15;
		handler = effect_handler_DESTRUCTION;
	} else if (die < 108) {
		handler = effect_handler_BANISH;
	} else if (die < 110) {
		subtype = PROJ_DISP_ALL;
		value.base = 120;
		handler = effect_handler_PROJECT_LOS;
	}

	if (handler != NULL) {
		effect_handler_context_t new_context = {
			context->effect,
			context->origin,
			context->obj,
			context->aware,
			context->dir,
			beam,
			context->boost,
			value,
			subtype, radius, other, y, x,
			NULL,
			context->ident,
			context->cmd
		};

		return handler(&new_context);
	} else {
		/* RARE */
		effect_simple(EF_PROJECT_LOS, context->origin, "150", PROJ_DISP_ALL, 0, 0, 0, 0, NULL);
		effect_simple(EF_PROJECT_LOS, context->origin, "20", PROJ_MON_SLOW, 0, 0, 0, 0, NULL);
		effect_simple(EF_PROJECT_LOS, context->origin, "40", PROJ_SLEEP_ALL, 0, 0, 0, 0, NULL);
		effect_simple(EF_HEAL_HP, context->origin, "300", 0, 0, 0, 0, 0, NULL);

		return true;
	}
}

/**
 * Dummy effect, to tell the effect code to pick one of the next
 * context->value.base effects at the player's selection or, if the effect
 * wasn't initiated by the player, at random.
 */
static bool effect_handler_SELECT(effect_handler_context_t *context)
{
	return true;
}


/**
 * ------------------------------------------------------------------------
 * Properties of effects
 * ------------------------------------------------------------------------ */
/**
 * Useful things about effects.
 */
static const struct effect_kind effects[] =
{
	{ EF_NONE, false, NULL, NULL, NULL, NULL },
	#define F(x) effect_handler_##x
	#define EFFECT(x, a, b, c, d, e, f)	{ EF_##x, a, b, F(x), e, f },
	#include "list-effects.h"
	#undef EFFECT
	#undef F
	{ EF_MAX, false, NULL, NULL, NULL, NULL }
};


static const char *effect_names[] = {
	NULL,
	#define EFFECT(x, a, b, c, d, e, f)	#x,
	#include "list-effects.h"
	#undef EFFECT
};

/*
 * Utility functions
 */

/**
 * Free all the effects in a structure
 *
 * \param source the effects being freed
 */
void free_effect(struct effect *source)
{
	struct effect *e = source, *e_next;
	while (e) {
		e_next = e->next;
		dice_free(e->dice);
		if (e->msg) {
			string_free(e->msg);
		}
		mem_free(e);
		e = e_next;
	}
}

bool effect_valid(const struct effect *effect)
{
	if (!effect) return false;
	return effect->index > EF_NONE && effect->index < EF_MAX;
}

bool effect_aim(const struct effect *effect)
{
	const struct effect *e = effect;

	if (!effect_valid(effect))
		return false;

	while (e) {
		if (effects[e->index].aim) return true;
		e = e->next;
	}

	return false;
}

const char *effect_info(const struct effect *effect)
{
	if (!effect_valid(effect))
		return NULL;

	return effects[effect->index].info;
}

const char *effect_desc(const struct effect *effect)
{
	if (!effect_valid(effect))
		return NULL;

	return effects[effect->index].desc;
}

effect_index effect_lookup(const char *name)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(effect_names); i++) {
		const char *effect_name = effect_names[i];

		/* Test for equality */
		if (effect_name != NULL && streq(name, effect_name))
			return i;
	}

	return EF_MAX;
}

/**
 * Translate a string to an effect parameter subtype index
 */
int effect_subtype(int index, const char *type)
{
	int val = -1;

	/* If not a numerical value, assign according to effect index */
	if (sscanf(type, "%d", &val) != 1) {
		switch (index) {
				/* Projection name */
			case EF_PROJECT_LOS:
			case EF_PROJECT_LOS_AWARE:
			case EF_DESTRUCTION:
			case EF_SPOT:
			case EF_SPHERE:
			case EF_BALL:
			case EF_BREATH:
			case EF_ARC:
			case EF_SHORT_BEAM:
			case EF_LASH:
			case EF_SWARM:
			case EF_STRIKE:
			case EF_STAR:
			case EF_STAR_BALL:
			case EF_BOLT:
			case EF_BEAM:
			case EF_BOLT_OR_BEAM:
			case EF_LINE:
			case EF_ALTER:
			case EF_BOLT_STATUS:
			case EF_BOLT_STATUS_DAM:
			case EF_BOLT_AWARE:
			case EF_MELEE_BLOWS:
			case EF_TOUCH:
			case EF_TOUCH_AWARE: {
				val = proj_name_to_idx(type);
				break;
			}

				/* Timed effect name */
			case EF_CURE:
			case EF_TIMED_SET:
			case EF_TIMED_INC:
			case EF_TIMED_INC_NO_RES:
			case EF_TIMED_DEC: {
				val = timed_name_to_idx(type);
				break;
			}

				/* Nourishment types */
			case EF_NOURISH: {
				if (streq(type, "INC_BY"))
					val = 0;
				else if (streq(type, "DEC_BY"))
					val = 1;
				else if (streq(type, "SET_TO"))
					val = 2;
				else if (streq(type, "INC_TO"))
					val = 3;
				break;
			}

				/* Monster timed effect name */
			case EF_MON_TIMED_INC: {
				val = mon_timed_name_to_idx(type);
				break;
			}

				/* Summon name */
			case EF_SUMMON: {
				val = summon_name_to_idx(type);
				break;
			}

				/* Stat name */
			case EF_RESTORE_STAT:
			case EF_DRAIN_STAT:
			case EF_LOSE_RANDOM_STAT:
			case EF_GAIN_STAT: {
				val = stat_name_to_idx(type);
				break;
			}

				/* Enchant type name - not worth a separate function */
			case EF_ENCHANT: {
				if (streq(type, "TOBOTH"))
					val = ENCH_TOBOTH;
				else if (streq(type, "TOHIT"))
					val = ENCH_TOHIT;
				else if (streq(type, "TODAM"))
					val = ENCH_TODAM;
				else if (streq(type, "TOAC"))
					val = ENCH_TOAC;
				break;
			}

				/* Player shape name */
			case EF_SHAPECHANGE: {
				val = shape_name_to_idx(type);
				break;
			}

				/* Targeted earthquake */
			case EF_EARTHQUAKE: {
				if (streq(type, "TARGETED"))
					val = 1;
				else if (streq(type, "NONE"))
					val = 0;
				break;
			}

				/* Inscribe a glyph */
			case EF_GLYPH: {
				if (streq(type, "WARDING"))
					val = GLYPH_WARDING;
				else if (streq(type, "DECOY"))
					val = GLYPH_DECOY;
				break;
			}

				/* Allow teleport away */
			case EF_TELEPORT: {
				if (streq(type, "AWAY"))
					val = 1;
				break;
			}

				/* Allow monster teleport toward */
			case EF_TELEPORT_TO: {
				if (streq(type, "SELF"))
					val = 1;
				break;
			}

				/* Some effects only want a radius, so this is a dummy */
			default: {
				if (streq(type, "NONE"))
					val = 0;
			}
		}
	}

	return val;
}

/**
 * ------------------------------------------------------------------------
 * Execution of effects
 * ------------------------------------------------------------------------ */
/**
 * Execute an effect chain.
 *
 * \param effect is the effect chain
 * \param origin is the origin of the effect (player, monster etc.)
 * \param obj    is the object making the effect happen (or NULL)
 * \param ident  will be updated if the effect is identifiable
 *               (NB: no effect ever sets *ident to false)
 * \param aware  indicates whether the player is aware of the effect already
 * \param dir    is the direction the effect will go in
 * \param beam   is the base chance out of 100 that a BOLT_OR_BEAM effect will beam
 * \param boost  is the extent to which skill surpasses difficulty, used as % boost. It
 *               ranges from 0 to 138.
 * \param cmd    If the effect is invoked as part of a command, this is the
 *               the command structure - used primarily so repeating the
 *               command can use the same information without prompting the
 *               player again.  Use NULL for this if not invoked as part of
 *               a command.
 */
bool effect_do(struct effect *effect,
		struct source origin,
		struct object *obj,
		bool *ident,
		bool aware,
		int dir,
		int beam,
		int boost,
		struct command *cmd)
{
	bool completed = false;
	effect_handler_f handler;
	random_value value = { 0, 0, 0, 0 };

	do {
		int choice_count = 0, leftover = 1;

		if (!effect_valid(effect)) {
			msg("Bad effect passed to effect_do(). Please report this bug.");
			return false;
		}

		if (effect->dice != NULL)
			choice_count = dice_roll(effect->dice, &value);

		/* Deal with special random and select effects */
		if (effect->index == EF_RANDOM || effect->index == EF_SELECT) {
			int choice;

			/*
			 * Treat select effects like random ones if they
			 * aren't from a player or if there's really no choice
			 * to be made.
			 */
			if (effect->index == EF_RANDOM ||
					origin.what != SRC_PLAYER ||
					choice_count < 2) {
				choice = randint0(choice_count);
			} else {
				assert(effect->index == EF_SELECT &&
					origin.what == SRC_PLAYER);
				if (cmd) {
					if (cmd_get_effect_from_list(cmd,
							"list_index",
							&choice, NULL,
							effect->next,
							choice_count,
							true) != CMD_OK) {
						return false;
					}
				} else {
					choice = get_effect_from_list(NULL,
						effect->next, choice_count,
						true);
					if (choice == -1) return false;
				}

				/*
				 * If the player chose to use a random effect,
				 * roll for it.
				 */
				if (choice == -2) {
					choice = randint0(choice_count);
				}
				assert(choice >= 0 && choice < choice_count);
			}

			leftover = choice_count - choice;

			/* Skip to the chosen effect */
			effect = effect->next;
			while (choice--)
				effect = effect->next;

			/* Roll the damage, if needed */
			if (effect->dice != NULL)
				(void) dice_roll(effect->dice, &value);
		}

		/* Handle the effect */
		handler = effects[effect->index].handler;
		if (handler != NULL) {
			effect_handler_context_t context = {
				effect->index,
				origin,
				obj,
				aware,
				dir,
				beam,
				boost,
				value,
				effect->subtype,
				effect->radius,
				effect->other,
				effect->y,
				effect->x,
				effect->msg,
				*ident,
				cmd
			};

			completed = handler(&context) || completed;
			*ident = context.ident;
		}

		/* Get the next effect, if there is one */
		while (leftover--)
			effect = effect->next;
	} while (effect);

	return completed;
}

/**
 * Perform a single effect with a simple dice string and parameters
 * Calling with ident a valid pointer will (depending on effect) give success
 * information; ident = NULL will ignore this
 */
void effect_simple(int index,
				   struct source origin,
				   const char *dice_string,
				   int subtype,
				   int radius,
				   int other,
				   int y,
				   int x,
				   bool *ident)
{
	struct effect effect;
	int dir = DIR_TARGET;
	bool dummy_ident = false;

	/* Set all the values */
	memset(&effect, 0, sizeof(effect));
	effect.index = index;
	effect.dice = dice_new();
	dice_parse_string(effect.dice, dice_string);
	effect.subtype = subtype;
	effect.radius = radius;
	effect.other = other;
	effect.y = y;
	effect.x = x;

	/* Direction if needed */
	if (effect_aim(&effect))
		get_aim_dir(&dir);

	/* Do the effect */
	if (!ident) {
		ident = &dummy_ident;
	}

	effect_do(&effect, origin, NULL, ident, true, dir, 0, 0, NULL);
	dice_free(effect.dice);
}
