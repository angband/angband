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
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(void)
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < player->max_gear; i++)
	{
		object_type *o_ptr = &player->gear[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Aware and Known */
		if (object_is_known(o_ptr)) continue;

		/* Identify it */
		do_ident_item(o_ptr);
	}
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
 * Set word of recall as appropriate
 */
bool set_recall(void)
{
	/* No recall */
	if (OPT(birth_no_recall) && !player->total_winner)
	{
		msg("Nothing happens.");
		return FALSE;
	}
    
	/* No recall from quest levels with force_descend */
	if (OPT(birth_force_descend) && (is_quest(player->depth))) {
		msg("Nothing happens.");
		return TRUE;
	}

    /* Warn the player if they're descending to an unrecallable level */
	if (OPT(birth_force_descend) && !(player->depth) &&
			(is_quest(player->max_depth + 1))) {
		if (!get_check("Are you sure you want to descend? ")) {
			msg("You prevent the recall from taking place.");
			return FALSE;
		}
	}

	/* Activate recall */
	if (!player->word_recall)
	{
		/* Reset recall depth */
		if ((player->depth > 0) && (player->depth != player->max_depth))
		{
			/* ToDo: Add a new player_type field "recall_depth" */
			if (get_check("Reset recall depth? "))
				player->max_depth = player->depth;
		}

		player->word_recall = randint0(20) + 15;
		msg("The air about you becomes charged...");
	}

	/* Deactivate recall */
	else
	{
		if (!get_check("Word of Recall is already active.  Do you want to cancel it? "))
			return FALSE;

		player->word_recall = 0;
		msg("A tension leaves the air around you...");
	}

	/* Redraw status line */
	player->upkeep->redraw = PR_STATUS;
	handle_stuff(player->upkeep);

	return TRUE;
}


/*
 * Apply disenchantment to the player's stuff
 *
 * This function is also called from the "melee" code.
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything.
 */
bool apply_disenchant(int mode)
{
	int i, count = 0;

	object_type *o_ptr;

	char o_name[80];


	/* Unused parameter */
	(void)mode;

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

	/* Get the item */
	o_ptr = equipped_item_by_slot(player, i);

	/* No item, nothing happens */
	if (!o_ptr->kind) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);


	/* Artifacts have 60% chance to resist */
	if (o_ptr->artifact && (randint0(100) < 60))
	{
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!",
		           o_name, equip_to_label(i),
		           ((o_ptr->number != 1) ? "" : "s"));

		/* Notice */
		return (TRUE);
	}

	/* Apply disenchantment, depending on which kind of equipment */
	if (slot_type_is(i, EQUIP_WEAPON) || slot_type_is(i, EQUIP_BOW))
	{
		/* Disenchant to-hit */
		if (o_ptr->to_h > 0) o_ptr->to_h--;
		if ((o_ptr->to_h > 5) && (randint0(100) < 20)) o_ptr->to_h--;

		/* Disenchant to-dam */
		if (o_ptr->to_d > 0) o_ptr->to_d--;
		if ((o_ptr->to_d > 5) && (randint0(100) < 20)) o_ptr->to_d--;
	}
	else
	{
		/* Disenchant to-ac */
		if (o_ptr->to_a > 0) o_ptr->to_a--;
		if ((o_ptr->to_a > 5) && (randint0(100) < 20)) o_ptr->to_a--;
	}

	/* Message */
	msg("Your %s (%c) %s disenchanted!",
	           o_name, equip_to_label(i),
	           ((o_ptr->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_EQUIP);

	/* Notice */
	return (TRUE);
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
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int y1, int x1, int r, bool full)
{
	int y, x, k;

	bool flag = FALSE;


	/* Unused parameter */
	(void)full;

	/* No effect in town */
	if (!player->depth)
	{
		msg("The ground shakes for a moment.");
		return;
	}

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, y, x)) continue;
			
			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Lose room and vault */
			sqinfo_off(cave->info[y][x], SQUARE_ROOM);
			sqinfo_off(cave->info[y][x], SQUARE_VAULT);

			/* Lose light */
			sqinfo_off(cave->info[y][x], SQUARE_GLOW);
			
			square_light_spot(cave, y, x);

			/* Hack -- Notice player affect */
			if (cave->m_idx[y][x] < 0)
			{
				/* Hurt the player later */
				flag = TRUE;

				/* Do not hurt this grid */
				continue;
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
			delete_monster(y, x);
			
			/* Don't remove stairs */
			if (square_isstairs(cave, y, x)) continue;	
			
			/* Lose knowledge (keeping knowledge of stairs) */
			sqinfo_off(cave->info[y][x], SQUARE_MARK);

			/* Destroy any grid that isn't a permament wall */
			if (!square_isperm(cave, y, x))
			{
				/* Delete objects */
				delete_object(y, x);
				square_destroy(cave, y, x);
			}
		}
	}


	/* Hack -- Affect player */
	if (flag)
	{
		/* Message */
		msg("There is a searing blast of light!");

		/* Blind the player */
		wieldeds_notice_element(player, ELEM_LIGHT);
		if (!player_resists(player, ELEM_LIGHT)) {
			/* Become blind */
			(void)player_inc_timed(player, TMD_BLIND, 10 + randint1(10),
								   TRUE, TRUE);
		}
	}


	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Redraw monster list */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when banished.
 *
 * Note that players and monsters (except eaters of walls and passers
 * through walls) will never occupy the same grid as a wall (or door).
 */
void earthquake(int cy, int cx, int r)
{
	int py = player->py;
	int px = player->px;

	int i, y, x, yy, xx, dy, dx;

	int damage = 0;

	int sn = 0, sy = 0, sx = 0;

	bool hurt = FALSE;

	bool map[32][32];

	/* No effect in town */
	if (!player->depth)
	{
		msg("The ground shakes for a moment.");
		return;
	}

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = FALSE;
		}
	}

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Lose room and vault */
			sqinfo_off(cave->info[yy][xx], SQUARE_ROOM);
			sqinfo_off(cave->info[yy][xx], SQUARE_VAULT);

			/* Lose light and knowledge */
			sqinfo_off(cave->info[yy][xx], SQUARE_GLOW);
			sqinfo_off(cave->info[yy][xx], SQUARE_MARK);
			
			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (randint0(100) < 85) continue;

			/* Damage this grid */
			map[16+yy-cy][16+xx-cx] = TRUE;

			/* Hack -- Take note of player damage */
			if ((yy == py) && (xx == px)) hurt = TRUE;
		}
	}

	/* First, affect the player (if necessary) */
	if (hurt)
	{
		/* Check around the player */
		for (i = 0; i < 8; i++)
		{
			/* Get the location */
			y = py + ddy_ddd[i];
			x = px + ddx_ddd[i];

			/* Skip non-empty grids */
			if (!square_isempty(cave, y, x)) continue;

			/* Important -- Skip "quake" grids */
			if (map[16+y-cy][16+x-cx]) continue;

			/* Count "safe" grids, apply the randomizer */
			if ((++sn > 1) && (randint0(sn) != 0)) continue;

			/* Save the safe location */
			sy = y; sx = x;
		}

		/* Random message */
		switch (randint1(3))
		{
			case 1:
			{
				msg("The cave ceiling collapses!");
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
		if (!sn)
		{
			/* Message and damage */
			msg("You are severely crushed!");
			damage = 300;
		}

		/* Destroy the grid, and push the player to safety */
		else
		{
			/* Calculate results */
			switch (randint1(3))
			{
				case 1:
				{
					msg("You nimbly dodge the blast!");
					damage = 0;
					break;
				}
				case 2:
				{
					msg("You are bashed by rubble!");
					damage = damroll(10, 4);
					(void)player_inc_timed(player, TMD_STUN, randint1(50), TRUE, TRUE);
					break;
				}
				case 3:
				{
					msg("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)player_inc_timed(player, TMD_STUN, randint1(50), TRUE, TRUE);
					break;
				}
			}

			/* Move player */
			monster_swap(py, px, sy, sx);
		}

		/* Take some damage */
		if (damage) take_hit(player, damage, "an earthquake");
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Process monsters */
			if (cave->m_idx[yy][xx] > 0)
			{
				monster_type *m_ptr = square_monster(cave, yy, xx);
				
				/* Most monsters cannot co-exist with rock */
				if (!flags_test(m_ptr->race->flags, RF_SIZE, RF_KILL_WALL, RF_PASS_WALL, FLAG_END))
				{
					char m_name[80];

					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!rf_has(m_ptr->race->flags, RF_NEVER_MOVE))
					{
						/* Look for safety */
						for (i = 0; i < 8; i++)
						{
							/* Get the grid */
							y = yy + ddy_ddd[i];
							x = xx + ddx_ddd[i];

							/* Skip non-empty grids */
							if (!square_isempty(cave, y, x)) continue;

							/* Hack -- no safety on glyph of warding */
							if (square_iswarded(cave, y, x))
								continue;

							/* Important -- Skip "quake" grids */
							if (map[16+y-cy][16+x-cx]) continue;

							/* Count "safe" grids, apply the randomizer */
							if ((++sn > 1) && (randint0(sn) != 0)) continue;

							/* Save the safe grid */
							sy = y;
							sx = x;
						}
					}

					/* Describe the monster */
					monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_STANDARD);

					/* Scream in pain */
					msg("%s wails out in pain!", m_name);

					/* Take damage from the quake */
					damage = (sn ? damroll(4, 8) : (m_ptr->hp + 1));

					/* Monster is certainly awake */
					mon_clear_timed(m_ptr, MON_TMD_SLEEP,
							MON_TMD_FLG_NOMESSAGE, FALSE);

					/* If the quake finished the monster off, show message */
					if (m_ptr->hp < damage && m_ptr->hp >= 0)
						msg("%s is embedded in the rock!", m_name);

					/* Apply damage directly */
					m_ptr->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (m_ptr->hp < 0)
					{
						/* Delete the monster */
						delete_monster(yy, xx);

						/* No longer safe */
						sn = 0;
					}

					/* Hack -- Escape from the rock */
					if (sn)
					{
						/* Move the monster */
						monster_swap(yy, xx, sy, sx);
					}
				}
			}
		}
	}


	/* XXX XXX XXX */

	/* New location */
	py = player->py;
	px = player->px;

	/* Important -- no wall on player */
	map[16+py-cy][16+px-cx] = FALSE;


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* ignore invalid grids */
			if (!square_in_bounds_fully(cave, yy, xx)) continue;

			/* Note unaffected grids for light changes, etc. */
			if (!map[16+yy-cy][16+xx-cx])
			{
				square_light_spot(cave, yy, xx);
			}

			/* Destroy location (if valid) */
			else if (square_valid_bold(yy, xx))
			{
				delete_object(yy, xx);
				square_earthquake(cave, yy, xx);
			}
		}
	}


	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Update the health bar */
	player->upkeep->redraw |= (PR_HEALTH);

	/* Window stuff */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);
}

/*
 * This routine will Perma-Light all grids in the set passed in.
 *
 * This routine is used (only) by "light_room(..., LIGHT)"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_light(struct point_set *ps)
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Perma-Light */
		sqinfo_on(cave->info[y][x], SQUARE_GLOW);
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff(player->upkeep);

	/* Process the grids */
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Redraw the grid */
		square_light_spot(cave, y, x);

		/* Process affected monsters */
		if (cave->m_idx[y][x] > 0)
		{
			int chance = 25;

			monster_type *m_ptr = square_monster(cave, y, x);

			/* Stupid monsters rarely wake up */
			if (rf_has(m_ptr->race->flags, RF_STUPID)) chance = 10;

			/* Smart monsters always wake up */
			if (rf_has(m_ptr->race->flags, RF_SMART)) chance = 100;

			/* Sometimes monsters wake up */
			if (m_ptr->m_timed[MON_TMD_SLEEP] && (randint0(100) < chance))
			{
				/* Wake up! */
				mon_clear_timed(m_ptr, MON_TMD_SLEEP,
					MON_TMD_FLG_NOTIFY, FALSE);

			}
		}
	}
}



/*
 * This routine will "darken" all grids in the set passed in.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "light_room(..., UNLIGHT)"
 */
static void cave_unlight(struct point_set *ps)
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Darken the grid */
		sqinfo_off(cave->info[y][x], SQUARE_GLOW);

		/* Hack -- Forget "boring" grids */
		if (!square_isinteresting(cave, y, x))
			sqinfo_off(cave->info[y][x], SQUARE_MARK);
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff(player->upkeep);

	/* Process the grids */
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Redraw the grid */
		square_light_spot(cave, y, x);
	}
}

/*
 * Aux function -- see below
 */
static void cave_room_aux(struct point_set *seen, int y, int x)
{
	if (point_set_contains(seen, y, x))
		return;

	if (!square_isroom(cave, y, x))
		return;

	/* Add it to the "seen" set */
	add_to_point_set(seen, y, x);
}

#define LIGHT TRUE
#define UNLIGHT FALSE
/*
 * Illuminate or darken any room containing the given location.
 */
void light_room(int y1, int x1, bool light)
{
	int i, x, y;
	struct point_set *ps;

	ps = point_set_new(200);
	/* Add the initial grid */
	cave_room_aux(ps, y1, x1);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < ps->n; i++)
	{
		x = ps->pts[i].x, y = ps->pts[i].y;

		/* Walls get lit, but stop light */
		if (!square_isprojectable(cave, y, x)) continue;

		/* Spread adjacent */
		cave_room_aux(ps, y + 1, x);
		cave_room_aux(ps, y - 1, x);
		cave_room_aux(ps, y, x + 1);
		cave_room_aux(ps, y, x - 1);

		/* Spread diagonal */
		cave_room_aux(ps, y + 1, x + 1);
		cave_room_aux(ps, y - 1, x - 1);
		cave_room_aux(ps, y - 1, x + 1);
		cave_room_aux(ps, y + 1, x - 1);
	}

	/* Now, lighten or darken them all at once */
	if (light) {
		cave_light(ps);
	} else {
		cave_unlight(ps);
	}
	point_set_dispose(ps);
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
