/*
 * File: spells.c
 * Purpose: Various assorted spell effects
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "dungeon.h"
#include "generate.h"
#include "history.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-util.h"
#include "obj-chest.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-make.h"
#include "obj-tval.h"
#include "obj-ui.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "player-util.h"
#include "spells.h"
#include "tables.h"
#include "trap.h"



/*
 * Array of stat "descriptions"
 */
static const char *desc_stat_neg[] =
{
	"weak",
	"stupid",
	"naive",
	"clumsy",
	"sickly",
	"ugly"
};

/*
 * Lose a "point"
 */
bool do_dec_stat(int stat, bool perma)
{
	bool sust = FALSE;

	/* Get the "sustain" */
	switch (stat)
	{
		case STAT_STR:
			if (player_of_has(player, OF_SUST_STR)) sust = TRUE;
			wieldeds_notice_flag(player, OF_SUST_STR);
			break;
		case STAT_INT:
			if (player_of_has(player, OF_SUST_INT)) sust = TRUE;
			wieldeds_notice_flag(player, OF_SUST_INT);
			break;
		case STAT_WIS:
			if (player_of_has(player, OF_SUST_WIS)) sust = TRUE;
			wieldeds_notice_flag(player, OF_SUST_WIS);
			break;
		case STAT_DEX:
			if (player_of_has(player, OF_SUST_DEX)) sust = TRUE;
			wieldeds_notice_flag(player, OF_SUST_DEX);
			break;
		case STAT_CON:
			if (player_of_has(player, OF_SUST_CON)) sust = TRUE;
			wieldeds_notice_flag(player, OF_SUST_CON);
			break;
	}

	/* Sustain */
	if (sust && !perma)
	{
		/* Message */
		msg("You feel very %s for a moment, but the feeling passes.",
		           desc_stat_neg[stat]);


		/* Notice effect */
		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (player_stat_dec(player, stat, perma))
	{
		/* Message */
		msgt(MSG_DRAIN_STAT, "You feel very %s.", desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Hack -- Removes curse from an object.
 */
static void uncurse_object(object_type *o_ptr)
{
	bitflag f[OF_SIZE];

	create_mask(f, FALSE, OFT_CURSE, OFT_MAX);

	of_diff(o_ptr->flags, f);
}


/*
 * Removes curses from items in inventory.
 *
 * \param heavy removes heavy curses if true
 *
 * \returns number of items uncursed
 */
static int remove_curse_aux(bool heavy)
{
	int i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = 0; i < player->body.count; i++)
	{
		object_type *o_ptr = equipped_item_by_slot(player, i);

		if (!o_ptr->kind) continue;
		if (!cursed_p(o_ptr->flags)) continue;

		/* Heavily cursed items need a special spell */
		if (of_has(o_ptr->flags, OF_HEAVY_CURSE) && !heavy) continue;

		/* Perma-cursed items can never be removed */
		if (of_has(o_ptr->flags, OF_PERMA_CURSE)) continue;

		/* Uncurse, and update things */
		uncurse_object(o_ptr);

		player->upkeep->update |= (PU_BONUS);
		player->upkeep->redraw |= (PR_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(void)
{
	return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
	return (remove_curse_aux(TRUE));
}



/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(const object_type *o_ptr)
{
	return tval_is_weapon(o_ptr);
}


/*
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(const object_type *o_ptr)
{
	return tval_is_armor(o_ptr);
}

/*
 * Hopefully this is OK now
 */
static bool item_tester_unknown(const object_type *o_ptr)
{
	return object_is_known(o_ptr) ? FALSE : TRUE;
}

/*
 * Used by the "enchant" function (chance of failure)
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
static bool enchant_score(s16b *score, bool is_artifact)
{
	int chance;

	/* Artifacts resist enchantment half the time */
	if (is_artifact && randint0(100) < 50) return FALSE;

	/* Figure out the chance to enchant */
	if (*score < 0) chance = 0;
	else if (*score > 15) chance = 1000;
	else chance = enchant_table[*score];

	/* If we roll less-than-or-equal to chance, it fails */
	if (randint1(1000) <= chance) return FALSE;

	/* Increment the score */
	++*score;

	return TRUE;
}

/**
 * Tries to uncurse a cursed item, if possible
 *
 * \returns true if a curse was broken
 */
static bool enchant_curse(object_type *o_ptr, bool is_artifact)
{
	/* If the item isn't cursed (or is perma-cursed) this doesn't work */
	if (!cursed_p(o_ptr->flags) || of_has(o_ptr->flags, OF_PERMA_CURSE)) 
		return FALSE;

	/* Artifacts resist enchanting curses away half the time */
	if (is_artifact && randint0(100) < 50) return FALSE;

	/* Normal items are uncursed 25% of the tiem */
	if (randint0(100) >= 25) return FALSE;

	/* Uncurse the item */
	msg("The curse is broken!");
	uncurse_object(o_ptr);
	return TRUE;
}

/**
 * Helper function for enchant() which tries to do the two things that
 * enchanting an item does, namely increasing its bonuses and breaking curses
 *
 * \returns true if a bonus was increased or a curse was broken
 */
static bool enchant2(object_type *o_ptr, s16b *score)
{
	bool result = FALSE;
	bool is_artifact = o_ptr->artifact ? TRUE : FALSE;
	if (enchant_score(score, is_artifact)) result = TRUE;
	if (enchant_curse(o_ptr, is_artifact)) result = TRUE;
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
bool enchant(object_type *o_ptr, int n, int eflag)
{
	int i, prob;
	bool res = FALSE;

	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if (tval_is_ammo(o_ptr)) prob = prob / 20;

	/* Try "n" times */
	for (i = 0; i < n; i++)
	{
		/* Roll for pile resistance */
		if (prob > 100 && randint0(prob) >= 100) continue;

		/* Try the three kinds of enchantment we can do */
		if ((eflag & ENCH_TOHIT) && enchant2(o_ptr, &o_ptr->to_h)) res = TRUE;
		if ((eflag & ENCH_TODAM) && enchant2(o_ptr, &o_ptr->to_d)) res = TRUE;
		if ((eflag & ENCH_TOAC)  && enchant2(o_ptr, &o_ptr->to_a)) res = TRUE;
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses, gear */
	player->upkeep->update |= (PU_BONUS | PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );

	/* Success */
	return (TRUE);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
	int item;
	bool okay = FALSE;

	object_type *o_ptr;

	char o_name[80];

	const char *q, *s;

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (!get_item(&item, q, s, 0, 
		num_ac ? item_tester_hook_armour : item_tester_hook_weapon,
		(USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR))) return (FALSE);

	o_ptr = object_from_item_idx(item);


	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Describe */
	msg("%s %s glow%s brightly!",
	           ((item >= 0) ? "Your" : "The"), o_name,
	           ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		flush();

		/* Message */
		msg("The enchantment failed.");
	}

	/* Something happened */
	return (TRUE);
}


/**
 * Return TRUE if there are any objects available to identify (whether on
 * floor or in gear)
 */
bool spell_identify_unknown_available(void)
{
	int floor_list[MAX_FLOOR_STACK];
	int floor_num;
	int i;
	bool unidentified_gear = FALSE;

	floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), player->py,
						   player->px, 0x0B, item_tester_unknown);

	for (i = 0; i < player->max_gear; i++) {
		if (item_test(item_tester_unknown, i)) {
			unidentified_gear = TRUE;
			break;
		}
	}

	return unidentified_gear || floor_num > 0;
}




/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(struct monster *m_ptr, int dis)
{
	int ny = 0, nx = 0, oy, ox, d, i, min;

	bool look = TRUE;


	/* Paranoia */
	if (!m_ptr->race) return;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!square_in_bounds_fully(cave, ny, nx)) continue;

			/* Require "empty" floor space */
			if (!square_isempty(cave, ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (square_iswarded(cave, ny, nx)) continue;

			/* No teleporting into vaults and such */
			/* if (cave->info[ny][nx] & square_isvault(cave, ny, nx)) continue; */

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TPOTHER);

	/* Swap the monsters */
	monster_swap(oy, ox, ny, nx);
}

/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
void teleport_player(int dis)
{
	int py = player->py;
	int px = player->px;

	int d, i, min, y, x;

	bool look = TRUE;

	/* Check for a no teleport grid */
	if (square_is_no_teleport(cave, py, px) && (dis > 10)) {
		msg("Teleportation forbidden!");
		return;
	}

	/* Initialize */
	y = py;
	x = px;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(py, dis);
				x = rand_spread(px, dis);
				d = distance(py, px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Require "naked" floor space */
			if (!square_isempty(cave, y, x)) continue;

			/* No teleporting into vaults and such */
			if (square_isvault(cave, y, x)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff(player->upkeep);
}

/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int py = player->py;
	int px = player->px;

	int y, x;

	int dis = 0, ctr = 0;

	/* Initialize */
	y = py;
	x = px;

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (square_in_bounds_fully(cave, y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (square_isempty(cave, y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff(player->upkeep);
}

/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{
	bool up = TRUE, down = TRUE;

	/* No going up with force_descend or in the town */
	if (OPT(birth_force_descend) || !player->depth)
		up = FALSE;

	/* No forcing player down to quest levels if they can't leave */
	if (!up && is_quest(player->max_depth + 1))
		down = FALSE;

	/* Can't leave quest levels or go down deeper than the dungeon */
	if (is_quest(player->depth) || (player->depth >= MAX_DEPTH-1))
		down = FALSE;

	/* Determine up/down if not already done */
	if (up && down) {
		if (randint0(100) < 50)
			up = FALSE;
		else
			down = FALSE;
	}

	/* Now actually do the level change */
	if (up) {
		msgt(MSG_TPLEVEL, "You rise up through the ceiling.");
		dungeon_change_level(player->depth - 1);
	} else if (down) {
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		if (OPT(birth_force_descend))
			dungeon_change_level(player->max_depth + 1);
		else
			dungeon_change_level(player->depth + 1);
	} else {
		msg("Nothing happens.");
	}
}



/*
 * Brand weapons (or ammo)
 *
 * Turns the (non-magical) object into an ego-item of 'brand_type'.
 */
void brand_object(object_type *o_ptr, const char *name)
{
	int i;
	ego_item_type *e_ptr;
	bool ok = FALSE;

	/* you can never modify artifacts / ego-items */
	/* you can never modify cursed / worthless items */
	if (o_ptr->kind && !cursed_p(o_ptr->flags) && o_ptr->kind->cost &&
	    !o_ptr->artifact && !o_ptr->ego)
	{
		char o_name[80];
		char brand[20];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		strnfmt(brand, sizeof(brand), "of %s", name);

		/* Describe */
		msg("The %s %s surrounded with an aura of %s.", o_name,
			(o_ptr->number > 1) ? "are" : "is", name);

		/* Get the right ego type for the object */
		for (i = 0; i < z_info->e_max; i++) {
			e_ptr = &e_info[i];

			/* Match the name */
			if (!e_ptr->name) continue;
			if (streq(e_ptr->name, brand)) {
				struct ego_poss_item *poss;
				for (poss = e_ptr->poss_items; poss; poss = poss->next)
					if (poss->kidx == o_ptr->kind->kidx)
						ok = TRUE;
			}
			if (ok) break;
		}

		/* Make it an ego item */
		o_ptr->ego = &e_info[i];
		ego_apply_magic(o_ptr, 0);
		object_notice_ego(o_ptr);

		/* Update the gear */
		player->upkeep->update |= (PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

		/* Enchant */
		enchant(o_ptr, randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);
	}
	else
	{
		flush();
		msg("The branding failed.");
	}
}


/*
 * Identify an item.
 */
void do_ident_item(object_type *o_ptr)
{
	char o_name[80];

	u32b msg_type = 0;
	int i, index, slot;
	bool bad = TRUE;

    /* Identify and apply autoinscriptions. */
	object_flavor_aware(o_ptr);
	object_notice_everything(o_ptr);
	apply_autoinscription(o_ptr);

	/* Set ignore flag */
	player->upkeep->notice |= PN_IGNORE;

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Determine the message type. */
	/* CC: we need to think more carefully about how we define "bad" with
	 * multiple modifiers - currently using "all nonzero modifiers < 0" */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (o_ptr->modifiers[i] > 0)
			bad = FALSE;

	if (bad)
		msg_type = MSG_IDENT_BAD;
	else if (o_ptr->artifact)
		msg_type = MSG_IDENT_ART;
	else if (o_ptr->ego)
		msg_type = MSG_IDENT_EGO;
	else
		msg_type = MSG_GENERIC;

	/* Log artifacts to the history list. */
	if (o_ptr->artifact)
		history_add_artifact(o_ptr->artifact, TRUE, TRUE);

	/* Describe */
	index = object_gear_index(player, o_ptr);
	slot = equipped_item_slot(player->body, index);
	if (item_is_equipped(player, index)) {
		/* Format and capitalise */
		char *msg = format("%s: %s (%c).", equip_describe(player, slot),
						   o_name, equip_to_label(slot));
		my_strcap(msg);

		msgt(msg_type, msg);
	} else if (index != NO_OBJECT)
		msgt(msg_type, "In your pack: %s (%c).", o_name, gear_to_label(index));
	else
		msgt(msg_type, "On the ground: %s.", o_name);
}
