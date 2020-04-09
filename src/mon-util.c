/**
 * \file mon-util.c
 * \brief Monster manipulation utilities.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "cmd-core.h"
#include "effects.h"
#include "game-world.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-list.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-summon.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "trap.h"
#include "z-set.h"

/**
 * ------------------------------------------------------------------------
 * Lore utilities
 * ------------------------------------------------------------------------ */
static const struct monster_flag monster_flag_table[] =
{
	#define RF(a, b, c) { RF_##a, b, c },
	#include "list-mon-race-flags.h"
	#undef RF
	{RF_MAX, 0, NULL}
};

/**
 * Return a description for the given monster race flag.
 *
 * Returns an empty string for an out-of-range flag.
 *
 * \param flag is one of the RF_ flags.
 */
const char *describe_race_flag(int flag)
{
	const struct monster_flag *rf = &monster_flag_table[flag];

	if (flag <= RF_NONE || flag >= RF_MAX)
		return "";

	return rf->desc;
}

/**
 * Create a mask of monster flags of a specific type.
 *
 * \param f is the flag array we're filling
 * \param ... is the list of flags we're looking for
 *
 * N.B. RFT_MAX must be the last item in the ... list
 */
void create_mon_flag_mask(bitflag *f, ...)
{
	const struct monster_flag *rf;
	int i;
	va_list args;

	rf_wipe(f);

	va_start(args, f);

	/* Process each type in the va_args */
    for (i = va_arg(args, int); i != RFT_MAX; i = va_arg(args, int)) {
		for (rf = monster_flag_table; rf->index < RF_MAX; rf++)
			if (rf->type == i)
				rf_on(f, rf->index);
	}

	va_end(args);

	return;
}


/**
 * ------------------------------------------------------------------------
 * Lookup utilities
 * ------------------------------------------------------------------------ */
/**
 * Returns the monster with the given name. If no monster has the exact name
 * given, returns the first monster with the given name as a (case-insensitive)
 * substring.
 */
struct monster_race *lookup_monster(const char *name)
{
	int i;
	struct monster_race *closest = NULL;

	/* Look for it */
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];
		if (!race->name)
			continue;

		/* Test for equality */
		if (my_stricmp(name, race->name) == 0)
			return race;

		/* Test for close matches */
		if (!closest && my_stristr(race->name, name))
			closest = race;
	}

	/* Return our best match */
	return closest;
}

/**
 * Return the monster base matching the given name.
 */
struct monster_base *lookup_monster_base(const char *name)
{
	struct monster_base *base;

	/* Look for it */
	for (base = rb_info; base; base = base->next) {
		if (streq(name, base->name))
			return base;
	}

	return NULL;
}

/**
 * Return whether the given base matches any of the names given.
 *
 * Accepts a variable-length list of name strings. The list must end with NULL.
 *
 * This function is currently unused, except in a test... -NRM-
 */
bool match_monster_bases(const struct monster_base *base, ...)
{
	bool ok = false;
	va_list vp;
	char *name;

	va_start(vp, base);
	while (!ok && ((name = va_arg(vp, char *)) != NULL))
		ok = base == lookup_monster_base(name);
	va_end(vp);

	return ok;
}

/**
 * Returns the monster currently commanded, or NULL
 */
struct monster *get_commanded_monster(void)
{
	int i;

	/* Look for it */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Test for control */
		if (mon->m_timed[MON_TMD_COMMAND]) return mon;
	}

	return NULL;
}

/**
 * ------------------------------------------------------------------------
 * Monster updates
 * ------------------------------------------------------------------------ */
/**
 * Analyse the path from player to infravision-seen monster and forget any
 * grids which would have blocked line of sight
 */
static void path_analyse(struct chunk *c, struct loc grid)
{
	int path_n, i;
	struct loc path_g[256];

	if (c != cave) {
		return;
	}

	/* Plot the path. */
	path_n = project_path(path_g, z_info->max_range, player->grid, grid,
						  PROJECT_NONE);

	/* Project along the path */
	for (i = 0; i < path_n - 1; ++i) {
		/* Forget grids which would block los */
		if (square_iswall(player->cave, path_g[i])) {
			sqinfo_off(square(c, path_g[i]).info, SQUARE_SEEN);
			square_forget(c, path_g[i]);
			square_light_spot(c, path_g[i]);
		}
	}
}

/**
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player (if requested),
 * and then checking for visibility (natural, infravision, see-invis,
 * telepathy), updating the monster visibility flag, redrawing (or
 * erasing) the monster when its visibility changes, and taking note
 * of any interesting monster flags (cold-blooded, invisible, etc).
 *
 * Note the new "mflag" field which encodes several monster state flags,
 * including "view" for when the monster is currently in line of sight,
 * and "mark" for when the monster is currently visible via detection.
 *
 * The only monster fields that are changed here are "cdis" (the
 * distance from the player), "ml" (visible to the player), and
 * "mflag" (to maintain the "MFLAG_VIEW" flag).
 *
 * Note the special "update_monsters()" function which can be used to
 * call this function once for every monster.
 *
 * Note the "full" flag which requests that the "cdis" field be updated;
 * this is only needed when the monster (or the player) has moved.
 *
 * Every time a monster moves, we must call this function for that
 * monster, and update the distance, and the visibility.  Every time
 * the player moves, we must call this function for every monster, and
 * update the distance, and the visibility.  Whenever the player "state"
 * changes in certain ways ("blindness", "infravision", "telepathy",
 * and "see invisible"), we must call this function for every monster,
 * and update the visibility.
 *
 * Routines that change the "illumination" of a grid must also call this
 * function for any monster in that grid, since the "visibility" of some
 * monsters may be based on the illumination of their grid.
 *
 * Note that this function is called once per monster every time the
 * player moves.  When the player is running, this function is one
 * of the primary bottlenecks, along with "update_view()" and the
 * "process_monsters()" code, so efficiency is important.
 *
 * Note the optimized "inline" version of the "distance()" function.
 *
 * A monster is "visible" to the player if (1) it has been detected
 * by the player, (2) it is close to the player and the player has
 * telepathy, or (3) it is close to the player, and in line of sight
 * of the player, and it is "illuminated" by some combination of
 * infravision, torch light, or permanent light (invisible monsters
 * are only affected by "light" if the player can see invisible).
 *
 * Monsters which are not on the current panel may be "visible" to
 * the player, and their descriptions will include an "offscreen"
 * reference.  Currently, offscreen monsters cannot be targeted
 * or viewed directly, but old targets will remain set.  XXX XXX
 *
 * The player can choose to be disturbed by several things, including
 * "OPT(player, disturb_near)" (monster which is "easily" viewable moves in some
 * way).  Note that "moves" includes "appears" and "disappears".
 */
void update_mon(struct monster *mon, struct chunk *c, bool full)
{
	struct monster_lore *lore;

	int d;

	/* If still generating the level, measure distances from the middle */
	struct loc pgrid = character_dungeon ? player->grid :
		loc(c->width / 2, c->height / 2);

	/* Seen at all */
	bool flag = false;

	/* Seen by vision */
	bool easy = false;

	/* ESP permitted */
	bool telepathy_ok = player_of_has(player, OF_TELEPATHY);

	assert(mon != NULL);

	/* Return if this is not the current level */
	if (c != cave) {
		return;
	}

	lore = get_lore(mon->race);
	
	/* Compute distance, or just use the current one */
	if (full) {
		/* Distance components */
		int dy = ABS(pgrid.y - mon->grid.y);
		int dx = ABS(pgrid.x - mon->grid.x);

		/* Approximate distance */
		d = (dy > dx) ? (dy + (dx >>  1)) : (dx + (dy >> 1));

		/* Restrict distance */
		if (d > 255) d = 255;

		/* Save the distance */
		mon->cdis = d;
	} else {
		/* Extract the distance */
		d = mon->cdis;
	}

	/* Detected */
	if (mflag_has(mon->mflag, MFLAG_MARK)) flag = true;

	/* Check if telepathy works here */
	if (square_isno_esp(c, mon->grid) || square_isno_esp(c, pgrid)) {
		telepathy_ok = false;
	}

	/* Nearby */
	if (d <= z_info->max_sight) {
		/* Basic telepathy */
		if (telepathy_ok && monster_is_esp_detectable(mon)) {
			/* Detectable */
			flag = true;

			/* Check for LOS so that MFLAG_VIEW is set later */
			if (square_isview(c, mon->grid)) easy = true;
		}

		/* Normal line of sight and player is not blind */
		if (square_isview(c, mon->grid) && !player->timed[TMD_BLIND]) {
			/* Use "infravision" */
			if (d <= player->state.see_infra) {
				/* Learn about warm/cold blood */
				rf_on(lore->flags, RF_COLD_BLOOD);

				/* Handle "warm blooded" monsters */
				if (!rf_has(mon->race->flags, RF_COLD_BLOOD)) {
					/* Easy to see */
					easy = flag = true;
				}
			}

			/* Use illumination */
			if (square_isseen(c, mon->grid)) {
				/* Learn about invisibility */
				rf_on(lore->flags, RF_INVISIBLE);

				/* Handle invisibility */
				if (monster_is_invisible(mon)) {
					/* See invisible */
					if (player_of_has(player, OF_SEE_INVIS)) {
						/* Easy to see */
						easy = flag = true;
					}
				} else {
					/* Easy to see */
					easy = flag = true;
				}
			}

			/* Learn about intervening squares */
			path_analyse(c, mon->grid);
		}
	}

	/* If a mimic looks like an ignored item, it's not seen */
	if (monster_is_mimicking(mon)) {
		struct object *obj = mon->mimicked_obj;
		if (ignore_item_ok(obj))
			easy = flag = false;
	}

	/* Is the monster is now visible? */
	if (flag) {
		/* Learn about the monster's mind */
		if (telepathy_ok) {
			flags_set(lore->flags, RF_SIZE, RF_EMPTY_MIND, RF_WEIRD_MIND,
					  RF_SMART, RF_STUPID, FLAG_END);
		}

		/* It was previously unseen */
		if (!monster_is_visible(mon)) {
			/* Mark as visible */
			mflag_on(mon->mflag, MFLAG_VISIBLE);

			/* Draw the monster */
			square_light_spot(c, mon->grid);

			/* Update health bar as needed */
			if (player->upkeep->health_who == mon)
				player->upkeep->redraw |= (PR_HEALTH);

			/* Hack -- Count "fresh" sightings */
			if (lore->sights < SHRT_MAX)
				lore->sights++;

			/* Window stuff */
			player->upkeep->redraw |= PR_MONLIST;
		}
	} else if (monster_is_visible(mon)) {
		/* Not visible but was previously seen - treat mimics differently */
		if (!mon->mimicked_obj || ignore_item_ok(mon->mimicked_obj)) {
			/* Mark as not visible */
			mflag_off(mon->mflag, MFLAG_VISIBLE);

			/* Erase the monster */
			square_light_spot(c, mon->grid);

			/* Update health bar as needed */
			if (player->upkeep->health_who == mon)
				player->upkeep->redraw |= (PR_HEALTH);

			/* Window stuff */
			player->upkeep->redraw |= PR_MONLIST;
		}
	}


	/* Is the monster is now easily visible? */
	if (easy) {
		/* Change */
		if (!monster_is_in_view(mon)) {
			/* Mark as easily visible */
			mflag_on(mon->mflag, MFLAG_VIEW);

			/* Disturb on appearance */
			if (OPT(player, disturb_near))
				disturb(player, 1);

			/* Re-draw monster window */
			player->upkeep->redraw |= PR_MONLIST;
		}
	} else {
		/* Change */
		if (monster_is_in_view(mon)) {
			/* Mark as not easily visible */
			mflag_off(mon->mflag, MFLAG_VIEW);

			/* Disturb on disappearance */
			if (OPT(player, disturb_near) && !monster_is_mimicking(mon))
				disturb(player, 1);

			/* Re-draw monster list window */
			player->upkeep->redraw |= PR_MONLIST;
		}
	}
}

/**
 * Updates all the (non-dead) monsters via update_mon().
 */
void update_monsters(bool full)
{
	int i;

	/* Update each (live) monster */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Update the monster if alive */
		if (mon->race)
			update_mon(mon, cave, full);
	}
}


/**
 * ------------------------------------------------------------------------
 * Monster (and player) actual movement
 * ------------------------------------------------------------------------ */
/**
 * Called when the player has just left grid1 for grid2.
 */
static void player_leaving(struct loc grid1, struct loc grid2)
{
	struct loc decoy = cave_find_decoy(cave);

	/* Decoys get destroyed if player is too far away */
	if (!loc_is_zero(decoy) &&
		distance(decoy, grid2) > z_info->max_sight) {
		square_destroy_decoy(cave, decoy);
	}

	/* Delayed traps trigger when the player leaves. */
	hit_trap(grid1, 1);
}

/**
 * Swap the players/monsters (if any) at two locations.
 */
void monster_swap(struct loc grid1, struct loc grid2)
{
	int m1, m2;
	struct monster *mon;
	struct loc pgrid = player->grid;

	/* Monsters */
	m1 = cave->squares[grid1.y][grid1.x].mon;
	m2 = cave->squares[grid2.y][grid2.x].mon;

	/* Update grids */
	square_set_mon(cave, grid1, m2);
	square_set_mon(cave, grid2, m1);

	/* Monster 1 */
	if (m1 > 0) {
		/* Monster */
		mon = cave_monster(cave, m1);
		mon->grid = grid2;

		/* Update monster */
		update_mon(mon, cave, true);

		/* Affect light? */
		if (mon->race->light != 0)
			player->upkeep->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);
	} else if (m1 < 0) {
		/* Player */
		player->grid = grid2;
		player_leaving(pgrid, player->grid);

		/* Update the trap detection status */
		player->upkeep->redraw |= (PR_DTRAP);

		/* Updates */
		player->upkeep->update |= (PU_PANEL | PU_UPDATE_VIEW | PU_DISTANCE);

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);

		/* Don't allow command repeat if moved away from item used. */
		if (cmdq_does_previous_use_floor_item()) {
			cmd_disable_repeat();
		}
	}

	/* Monster 2 */
	if (m2 > 0) {
		/* Monster */
		mon = cave_monster(cave, m2);
		mon->grid = grid1;

		/* Update monster */
		update_mon(mon, cave, true);

		/* Affect light? */
		if (mon->race->light != 0)
			player->upkeep->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);
	} else if (m2 < 0) {
		/* Player */
		player->grid = grid1;
		player_leaving(pgrid, player->grid);

		/* Update the trap detection status */
		player->upkeep->redraw |= (PR_DTRAP);

		/* Updates */
		player->upkeep->update |= (PU_PANEL | PU_UPDATE_VIEW | PU_DISTANCE);

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);

		/* Don't allow command repeat if moved away from item used. */
		if (cmdq_does_previous_use_floor_item()) {
			cmd_disable_repeat();
		}
	}

	/* Redraw */
	square_light_spot(cave, grid1);
	square_light_spot(cave, grid2);
}

/**
 * ------------------------------------------------------------------------
 * Awareness and learning
 * ------------------------------------------------------------------------ */
/**
 * Monster wakes up and possibly becomes aware of the player
 */
void monster_wake(struct monster *mon, bool notify, int aware_chance)
{
	int flag = notify ? MON_TMD_FLG_NOTIFY : MON_TMD_FLG_NOMESSAGE;
	mon_clear_timed(mon, MON_TMD_SLEEP, flag);
	if (randint0(100) < aware_chance) {
		mflag_on(mon->mflag, MFLAG_AWARE);
	}
}

/**
 * Monster can see a grid
 */
bool monster_can_see(struct chunk *c, struct monster *mon, struct loc grid)
{
	return los(c, mon->grid, grid);
}

/**
 * Make player fully aware of the given mimic.
 *
 * When a player becomes aware of a mimic, we update the monster memory
 * and delete the "fake item" that the monster was mimicking.
 */
void become_aware(struct monster *mon)
{
	struct monster_lore *lore = get_lore(mon->race);

	if (mflag_has(mon->mflag, MFLAG_CAMOUFLAGE)) {
		mflag_off(mon->mflag, MFLAG_CAMOUFLAGE);

		/* Learn about mimicry */
		if (rf_has(mon->race->flags, RF_UNAWARE))
			rf_on(lore->flags, RF_UNAWARE);

		/* Delete any false items */
		if (mon->mimicked_obj) {
			struct object *obj = mon->mimicked_obj;
			char o_name[80];
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

			/* Print a message */
			if (square_isseen(cave, obj->grid))
				msg("The %s was really a monster!", o_name);

			/* Clear the mimicry */
			obj->mimicking_m_idx = 0;
			mon->mimicked_obj = NULL;

			/*
			 * Give a copy of the object to the monster if
			 * appropriate.
			 */
			if (rf_has(mon->race->flags, RF_MIMIC_INV)) {
				struct object* given = object_new();

				object_copy(given, obj);
				given->oidx = 0;
				if (obj->known) {
					given->known = object_new();
					object_copy(given->known, obj->known);
					given->known->oidx = 0;
					given->known->grid = loc(0, 0);
				}
				if (! monster_carry(cave, mon, given)) {
					if (given->known) {
						object_delete(&given->known);
					}
					object_delete(&given);
				}
			}

			/*
			 * Delete the mimicked object; noting and lighting
			 * done below outside of the if block.
			 */
			square_delete_object(cave, obj->grid, obj, false, false);
		}

		/* Update monster and item lists */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);
	}

	square_note_spot(cave, mon->grid);
	square_light_spot(cave, mon->grid);
}

/**
 * The given monster learns about an "observed" resistance or other player
 * state property, or lack of it.
 *
 * Note that this function is robust to being called with `element` as an
 * arbitrary PROJ_ type
 */
void update_smart_learn(struct monster *mon, struct player *p, int flag,
						int pflag, int element)
{
	bool element_ok = ((element >= 0) && (element < ELEM_MAX));

	/* Sanity check */
	if (!flag && !element_ok) return;

	/* Anything a monster might learn, the player should learn */
	if (flag) equip_learn_flag(p, flag);
	if (element_ok) equip_learn_element(p, element);

	/* Not allowed to learn */
	if (!OPT(p, birth_ai_learn)) return;

	/* Too stupid to learn anything */
	if (monster_is_stupid(mon)) return;

	/* Not intelligent, only learn sometimes */
	if (!monster_is_smart(mon) && one_in_(2)) return;

	/* Analyze the knowledge; fail very rarely */
	if (one_in_(100))
		return;

	/* Learn the flag */
	if (flag) {
		if (player_of_has(p, flag)) {
			of_on(mon->known_pstate.flags, flag);
		} else {
			of_off(mon->known_pstate.flags, flag);
		}
	}

	/* Learn the pflag */
	if (pflag) {
		if (pf_has(p->state.pflags, pflag)) {
			of_on(mon->known_pstate.pflags, pflag);
		} else {
			of_off(mon->known_pstate.pflags, pflag);
		}
	}

	/* Learn the element */
	if (element_ok)
		mon->known_pstate.el_info[element].res_level
			= p->state.el_info[element].res_level;
}

/**
 * ------------------------------------------------------------------------
 * Monster healing
 * ------------------------------------------------------------------------ */
#define MAX_KIN_RADIUS			5
#define MAX_KIN_DISTANCE		5

/**
 * Given a dungeon chunk, a monster, and a location, see if there is
 * an injured monster with the same base kind in LOS and less than
 * MAX_KIN_DISTANCE away.
 */
static struct monster *get_injured_kin(struct chunk *c,
									   const struct monster *mon,
									   struct loc grid)
{
	/* Ignore the monster itself */
	if (loc_eq(grid, mon->grid))
		return NULL;

	/* Check kin */
	struct monster *kin = square_monster(c, grid);
	if (!kin)
		return NULL;

	if (kin->race->base != mon->race->base)
		return NULL;

	/* Check line of sight */
	if (los(c, mon->grid, grid) == false)
		return NULL;

	/* Check injury */
	if (kin->hp == kin->maxhp)
		return NULL;

	/* Check distance */
	if (distance(mon->grid, grid) > MAX_KIN_DISTANCE)
		return NULL;

	return kin;
}

/**
 * Find out if there are any injured monsters nearby.
 *
 * See get_injured_kin() above for more details on what monsters qualify.
 */
bool find_any_nearby_injured_kin(struct chunk *c, const struct monster *mon)
{
	struct loc grid;
	for (grid.y = mon->grid.y - MAX_KIN_RADIUS;
		 grid.y <= mon->grid.y + MAX_KIN_RADIUS; grid.y++) {
		for (grid.x = mon->grid.x - MAX_KIN_RADIUS;
			 grid.x <= mon->grid.x + MAX_KIN_RADIUS; grid.x++) {
			if (get_injured_kin(c, mon, grid) != NULL) {
				return true;
			}
		}
	}

	return false;
}

/**
 * Choose one injured monster of the same base in LOS of the provided monster.
 *
 * Scan MAX_KIN_RADIUS grids around the monster to find potential grids,
 * make a list of kin, and choose a random one.
 */
struct monster *choose_nearby_injured_kin(struct chunk *c,
										  const struct monster *mon)
{
	struct set *set = set_new();
	struct loc grid;

	for (grid.y = mon->grid.y - MAX_KIN_RADIUS;
		 grid.y <= mon->grid.y + MAX_KIN_RADIUS; grid.y++) {
		for (grid.x = mon->grid.x - MAX_KIN_RADIUS;
			 grid.x <= mon->grid.x + MAX_KIN_RADIUS; grid.x++) {
			struct monster *kin = get_injured_kin(c, mon, grid);
			if (kin != NULL) {
				set_add(set, kin);
			}
		}
	}

	struct monster *found = set_choose(set);
	set_free(set);

	return found;
}


/**
 * ------------------------------------------------------------------------
 * Monster damage and death utilities
 * ------------------------------------------------------------------------ */
/**
 * Handles the "death" of a monster.
 *
 * Disperses treasures carried by the monster centered at the monster location.
 * Note that objects dropped may disappear in crowded rooms.
 *
 * Checks for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * If `stats` is true, then we skip updating the monster memory. This is
 * used by stats-generation code, for efficiency.
 */
void monster_death(struct monster *mon, bool stats)
{
	int dump_item = 0;
	int dump_gold = 0;
	struct object *obj = mon->held_obj;

	bool visible = monster_is_visible(mon) || monster_is_unique(mon);

	/* Delete any mimicked objects */
	if (mon->mimicked_obj)
		object_delete(&mon->mimicked_obj);

	/* Drop objects being carried */
	while (obj) {
		struct object *next = obj->next;

		/* Object no longer held */
		obj->held_m_idx = 0;
		pile_excise(&mon->held_obj, obj);

		/* Count it and drop it - refactor once origin is a bitflag */
		if (!stats) {
			if (tval_is_money(obj) && (obj->origin != ORIGIN_STOLEN))
				dump_gold++;
			else if (!tval_is_money(obj) && ((obj->origin == ORIGIN_DROP)
					|| (obj->origin == ORIGIN_DROP_PIT)
					|| (obj->origin == ORIGIN_DROP_VAULT)
					|| (obj->origin == ORIGIN_DROP_SUMMON)
					|| (obj->origin == ORIGIN_DROP_SPECIAL)
					|| (obj->origin == ORIGIN_DROP_BREED)
					|| (obj->origin == ORIGIN_DROP_POLY)
					|| (obj->origin == ORIGIN_DROP_WIZARD)))
				dump_item++;
		}

		/* Change origin if monster is invisible, unless we're in stats mode */
		if (!visible && !stats)
			obj->origin = ORIGIN_DROP_UNKNOWN;

		drop_near(cave, &obj, 0, mon->grid, true);
		obj = next;
	}

	/* Forget objects */
	mon->held_obj = NULL;

	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
		lore_treasure(mon, dump_item, dump_gold);

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;

	/* Affect light? */
	if (mon->race->light != 0)
		player->upkeep->update |= PU_UPDATE_VIEW;

	/* Check if we finished a quest */
	quest_check(mon);
}

/**
 * Handle the consequences of the killing of a monster by the player
 */
static void player_kill_monster(struct monster *mon, const char *note)
{
	s32b div, new_exp, new_exp_frac;
	struct monster_lore *lore = get_lore(mon->race);
	char m_name[80];
	char buf[80];

	/* Assume normal death sound */
	int soundfx = MSG_KILL;

	/* Extract monster name */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_DEFAULT);

	/* Shapechanged monsters revert on death */
	if (mon->original_race) {
		msg("A change comes over %s", m_name);
		monster_revert_shape(mon);
		lore = get_lore(mon->race);
		monster_desc(m_name, sizeof(m_name), mon, MDESC_DEFAULT);
	}

	/* Play a special sound if the monster was unique */
	if (rf_has(mon->race->flags, RF_UNIQUE)) {
		if (mon->race->base == lookup_monster_base("Morgoth"))
			soundfx = MSG_KILL_KING;
		else
			soundfx = MSG_KILL_UNIQUE;
	}

	/* Death message */
	if (note) {
		if (strlen(note) <= 1) {
			/* Death by Spell attack - messages handled by project_m() */
		} else {
			char *str = format("%s%s", m_name, note);
			my_strcap(str);

			/* Make sure to flush any monster messages first */
			notice_stuff(player);

			/* Death by Missile attack */
			msgt(soundfx, "%s", str);
		}
	} else {
		/* Make sure to flush any monster messages first */
		notice_stuff(player);

		if (!monster_is_visible(mon))
			/* Death by physical attack -- invisible monster */
			msgt(soundfx, "You have killed %s.", m_name);
		else if (monster_is_destroyed(mon))
			/* Death by Physical attack -- non-living monster */
			msgt(soundfx, "You have destroyed %s.", m_name);
		else
			/* Death by Physical attack -- living monster */
			msgt(soundfx, "You have slain %s.", m_name);
	}

	/* Player level */
	div = player->lev;

	/* Give some experience for the kill */
	new_exp = ((long)mon->race->mexp * mon->race->level) / div;

	/* Handle fractional experience */
	new_exp_frac = ((((long)mon->race->mexp * mon->race->level) % div)
					* 0x10000L / div) + player->exp_frac;

	/* Keep track of experience */
	if (new_exp_frac >= 0x10000L) {
		new_exp++;
		player->exp_frac = (u16b)(new_exp_frac - 0x10000L);
	} else {
		player->exp_frac = (u16b)new_exp_frac;
	}

	/* When the player kills a Unique, it stays dead */
	if (rf_has(mon->race->flags, RF_UNIQUE)) {
		char unique_name[80];
		mon->race->max_num = 0;

		/*
		 * This gets the correct name if we slay an invisible
		 * unique and don't have See Invisible.
		 */
		monster_desc(unique_name, sizeof(unique_name), mon,
					 MDESC_DIED_FROM);

		/* Log the slaying of a unique */
		strnfmt(buf, sizeof(buf), "Killed %s", unique_name);
		history_add(player, buf, HIST_SLAY_UNIQUE);
	}

	/* Gain experience */
	player_exp_gain(player, new_exp);

	/* Generate treasure */
	monster_death(mon, false);

	/* Bloodlust bonus */
	if (player->timed[TMD_BLOODLUST]) {
		player_inc_timed(player, TMD_BLOODLUST, 10, false, false);
		player_over_exert(player, PY_EXERT_CONF, 5, 3);
		player_over_exert(player, PY_EXERT_HALLU, 5, 10);
	}

	/* Recall even invisible uniques or winners */
	if (monster_is_visible(mon) || monster_is_unique(mon)) {
		/* Count kills this life */
		if (lore->pkills < SHRT_MAX) lore->pkills++;

		/* Count kills in all lives */
		if (lore->tkills < SHRT_MAX) lore->tkills++;

		/* Update lore and tracking */
		lore_update(mon->race, lore);
		monster_race_track(player->upkeep, mon->race);
	}

	/* Delete the monster */
	delete_monster_idx(mon->midx);
}

/**
 * See how a monster reacts to damage
 */
static bool monster_scared_by_damage(struct monster *mon, int dam)
{
	int current_fear = mon->m_timed[MON_TMD_FEAR];

	/* Pain can reduce or cancel existing fear, or cause fear */
	if (current_fear) {
		int tmp = randint1(dam);

		/* Cure a little or all fear */
		if (tmp < current_fear) {
			/* Reduce fear */
			mon_dec_timed(mon, MON_TMD_FEAR, tmp, MON_TMD_FLG_NOMESSAGE);
		} else {
			/* Cure fear */
			mon_clear_timed(mon, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE);
			return false;
		}
	} else if (monster_can_be_scared(mon)) {
		/* Percentage of fully healthy */
		int percentage = (100L * mon->hp) / mon->maxhp;

		/* Run (sometimes) if at 10% or less of max hit points... */
		bool low_hp = randint1(10) >= percentage;

		/* ...or (usually) when hit for half its current hit points */
		bool big_hit = (dam >= mon->hp) && (randint0(100) < 80);

		if (low_hp || big_hit) {
			int time = randint1(10);
			if ((dam >= mon->hp) && (percentage > 7)) {
				time += 20;
			} else {
				time += (11 - percentage) * 5;
			}

			/* Note fear */
			mon_inc_timed(mon, MON_TMD_FEAR, time,
						  MON_TMD_FLG_NOMESSAGE | MON_TMD_FLG_NOFAIL);
			return true;
		}
	}
	return false;
}

/**
 * Deal damage to a monster from another monster (or at least not the player).
 *
 * This is a helper for melee handlers. It is very similar to mon_take_hit(),
 * but eliminates the player-oriented stuff of that function.
 *
 * \param context is the project_m context.
 * \param hurt_msg is the message if the monster is hurt (if any).
 * \return true if the monster died, false if it is still alive.
 */
bool mon_take_nonplayer_hit(int dam, struct monster *t_mon,
							enum mon_messages hurt_msg,
							enum mon_messages die_msg)
{
	assert(t_mon);

	/* "Unique" monsters can only be "killed" by the player */
	if (rf_has(t_mon->race->flags, RF_UNIQUE)) {
		/* Reduce monster hp to zero, but don't kill it. */
		if (dam > t_mon->hp) dam = t_mon->hp;
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == t_mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Wake the monster up, doesn't become aware of the player */
	monster_wake(t_mon, false, 0);

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
		return true;
	} else if (!monster_is_mimicking(t_mon)) {
		/* Give detailed messages if visible */
		if (hurt_msg != MON_MSG_NONE) {
			add_monster_message(t_mon, hurt_msg, false);
		} else if (dam > 0) {
			message_pain(t_mon, dam);
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!t_mon->m_timed[MON_TMD_FEAR] && dam > 0) {
		(void) monster_scared_by_damage(t_mon, dam);
	}

	return false;
}

/**
 * Decreases a monster's hit points by `dam` and handle monster death.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * We announce monster death (using an optional "death message" (`note`)
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Returns true if the monster has been killed (and deleted).
 *
 * TODO: Consider decreasing monster experience over time, say, by using
 * "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))" instead
 * of simply "(m_exp * m_lev) / (p_lev)", to make the first monster
 * worth more than subsequent monsters.  This would also need to
 * induce changes in the monster recall code.  XXX XXX XXX
 **/
bool mon_take_hit(struct monster *mon, int dam, bool *fear, const char *note)
{
	/* Redraw (later) if needed */
	if (player->upkeep->health_who == mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Wake it up, make it aware of the player */
	monster_wake(mon, false, 100);
	mon_clear_timed(mon, MON_TMD_HOLD, MON_TMD_FLG_NOTIFY);

	/* Become aware of its presence */
	if (monster_is_camouflaged(mon))
		become_aware(mon);

	/* No damage, we're done */
	if (dam == 0) return false;

	/* Covering tracks is no longer possible */
	player->timed[TMD_COVERTRACKS] = 0;

	/* Hurt it */
	mon->hp -= dam;
	if (mon->hp < 0) {
		/* Deal with arena monsters */
		if (player->upkeep->arena_level) {
			player->upkeep->generate_level = true;
			(*fear) = false;
			return true;
		}

		/* It is dead now */
		player_kill_monster(mon, note);

		/* Not afraid */
		(*fear) = false;

		/* Monster is dead */
		return true;
	} else {
		/* Did it get frightened? */
		(*fear) = monster_scared_by_damage(mon, dam);

		/* Not dead yet */
		return false;
	}
}

void kill_arena_monster(struct monster *mon)
{
	struct monster *old_mon = cave_monster(cave, mon->midx);
	assert(old_mon);
	update_mon(old_mon, cave, true);
	old_mon->hp = -1;
	player_kill_monster(old_mon, " is defeated!");
}

/**
 * Terrain damages monster
 */
void monster_take_terrain_damage(struct monster *mon)
{
	/* Damage the monster */
	if (square_isfiery(cave, mon->grid)) {
		bool fear = false;

		if (!rf_has(mon->race->flags, RF_IM_FIRE)) {
			mon_take_nonplayer_hit(100 + randint1(100), mon, MON_MSG_CATCH_FIRE,
								   MON_MSG_DISINTEGRATES);
		}

		if (fear && monster_is_visible(mon)) {
			add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
		}
	}	
}

/**
 * Terrain is currently damaging monster
 */
bool monster_taking_terrain_damage(struct monster *mon)
{
	if (square_isdamaging(cave, mon->grid) &&
		!rf_has(mon->race->flags, square_feat(cave, mon->grid)->resist_flag)) {
		return true;
	}

	return false;
}


/**
 * ------------------------------------------------------------------------
 * Monster inventory utilities
 * ------------------------------------------------------------------------ */
/**
 * Add the given object to the given monster's inventory.
 *
 * Currently always returns true - it is left as a bool rather than
 * void in case a limit on monster inventory size is proposed in future.
 */
bool monster_carry(struct chunk *c, struct monster *mon, struct object *obj)
{
	struct object *held_obj;

	/* Scan objects already being held for combination */
	for (held_obj = mon->held_obj; held_obj; held_obj = held_obj->next) {
		/* Check for combination */
		if (object_similar(held_obj, obj, OSTACK_MONSTER)) {
			/* Combine the items */
			object_absorb(held_obj, obj);

			/* Result */
			return true;
		}
	}

	/* Forget location */
	obj->grid = loc(0, 0);

	/* Link the object to the monster */
	obj->held_m_idx = mon->midx;

	/* Add the object to the monster's inventory */
	list_object(c, obj);
	if (obj->known) {
		obj->known->oidx = obj->oidx;
		player->cave->objects[obj->oidx] = obj->known;
	}
	pile_insert(&mon->held_obj, obj);

	/* Result */
	return true;
}

/**
 * Get a random object from a monster's inventory
 */
struct object *get_random_monster_object(struct monster *mon)
{
	struct object *obj = mon->held_obj;
	int count = 0;

	if (!obj) return NULL;

	/* Count the objects */
	while (obj) {
		count++;
		obj = obj->next;
	}

	/* Now pick one... */
	obj = mon->held_obj;
	count -= randint1(count);
	while (count) {
		obj = obj->next;
		count--;
	}

	return obj;
}

/**
 * Player or monster midx steals an item from a monster
 *
 * \param mon Monster stolen from
 * \param midx Index of the thief
 */
void steal_monster_item(struct monster *mon, int midx)
{
	struct object *obj = get_random_monster_object(mon);
	struct monster_lore *lore = get_lore(mon->race);
	struct monster *thief = NULL;
	char m_name[80];

	/* Get the target monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);

	if (midx < 0) {
		/* Base monster protection and player stealing skill */
		bool unique = rf_has(mon->race->flags, RF_UNIQUE);
		int guard = (mon->race->level * (unique ? 4 : 3)) / 4 +
			mon->mspeed - player->state.speed;
		int steal_skill = player->state.skills[SKILL_STEALTH] +
			adj_dex_th[player->state.stat_ind[STAT_DEX]];
		int monster_reaction;

		/* No object */
		if (!obj) {
			msg("You can find nothing to steal from %s.", m_name);
			if (one_in_(3)) {
				/* Monster notices */
				monster_wake(mon, false, 100);
			}
			return;
		}

		/* Penalize some status conditions */
		if (player->timed[TMD_BLIND] || player->timed[TMD_CONFUSED] ||
			player->timed[TMD_IMAGE]) {
			steal_skill /= 4;
		}
		if (mon->m_timed[MON_TMD_SLEEP]) {
			guard /= 2;
		}

		/* Monster base reaction, plus allowance for item weight */
		monster_reaction = guard / 2 + randint1(MAX(guard, 1));
		monster_reaction += obj->weight / 20;

		/* Try and steal */
		if (monster_reaction < steal_skill) {
			int wake = 35 - player->state.skills[SKILL_STEALTH];

			/* Success! */
			obj->held_m_idx = 0;
			pile_excise(&mon->held_obj, obj);
			if (tval_is_money(obj)) {
				msg("You steal %d gold pieces worth of treasure.", obj->pval);
				player->au += obj->pval;
				delist_object(cave, obj);
				object_delete(&obj);
			} else {
				object_grab(player, obj);
				delist_object(player->cave, obj->known);
				delist_object(cave, obj);
				/* Drop immediately if ignored to prevent pack overflow */
				if (ignore_item_ok(obj)) {
					char o_name[80];
					object_desc(o_name, sizeof(o_name), obj,
								ODESC_PREFIX | ODESC_FULL);
					drop_near(cave, &obj, 0, player->grid, true);
					msg("You drop %s.", o_name);
				} else {
					inven_carry(player, obj, true, true);
				}
			}

			/* Track thefts */
			lore->thefts++;

			/* Monster wakes a little */
			mon_dec_timed(mon, MON_TMD_SLEEP, wake, MON_TMD_FLG_NOTIFY);
		} else if (monster_reaction / 2 < steal_skill) {
			/* Decent attempt, at least */
			char o_name[80];

			object_see(player, obj);
			if (tval_is_money(obj)) {
				(void)strnfmt(o_name, sizeof(o_name), "treasure");
			} else {
				object_desc(o_name, sizeof(o_name), obj,
							ODESC_PREFIX | ODESC_FULL);
			}
			msg("You fail to steal %s from %s.", o_name, m_name);
			/* Monster wakes, may notice */
			monster_wake(mon, true, 50);
		} else {
			/* Bungled it */
			monster_wake(mon, true, 100);
			monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);
			msg("%s cries out in anger!", m_name);
			effect_simple(EF_WAKE, source_monster(mon->midx), "", 0, 0, 0, 0, 0,
						  NULL);
		}

		/* Player hit and run */
		if (player->timed[TMD_ATT_RUN]) {
			const char *near = "20";
			msg("You vanish into the shadows!");
			effect_simple(EF_TELEPORT, source_player(), near, 0, 0, 0, 0, 0,
						  NULL);
			(void) player_clear_timed(player, TMD_ATT_RUN, false);
		}
	} else {
		/* Get the thief details */
		char t_name[80];
		thief = cave_monster(cave, midx);
		assert(thief);
		monster_desc(t_name, sizeof(t_name), thief, MDESC_STANDARD);

		/* Try to steal */
		if (!obj || react_to_slay(obj, thief)) {
			/* Fail to steal */
			msg("%s tries to steal something from %s, but fails.", t_name,
				m_name);
		} else {
			msg("%s steals something from %s!", t_name, m_name);

			/* Steal and carry */
			obj->held_m_idx = 0;
			pile_excise(&mon->held_obj, obj);
			(void)monster_carry(cave, thief, obj);
		}
	}
}


/**
 * ------------------------------------------------------------------------
 * Monster shapechange utilities
 * ------------------------------------------------------------------------ */
/**
 * The shape base for shapechanges
 */
struct monster_base *shape_base;

/**
 * Predicate function for get_mon_num_prep
 * Check to see if the monster race has the same base as the desired shape
 */
static bool monster_base_shape_okay(struct monster_race *race)
{
	assert(race);

	/* Check if it matches */
	if (race->base != shape_base) return false;

	return true;
}

/**
 * Monster shapechange
 */
bool monster_change_shape(struct monster *mon)
{
	struct monster_shape *shape = mon->race->shapes;
	struct monster_race *race = NULL;

	/* Use the monster's preferred shapes if any */
	if (shape) {
		/* Pick one */
		int choice = randint0(mon->race->num_shapes);
		while (choice--) {
			shape = shape->next;
		}

		/* Race or base? */
		if (shape->race) {
			/* Simple */
			race = shape->race;
		} else {
			/* Set the shape base */
			shape_base = shape->base;

			/* Choose a race of the given base */
			get_mon_num_prep(monster_base_shape_okay);

			/* Pick a random race */
			race = get_mon_num(player->depth + 5);

			/* Reset allocation table */
			get_mon_num_prep(NULL);
		}
	} else {
		/* Choose something the monster can summon */
		bitflag summon_spells[RSF_SIZE];
		int i, poss = 0, which, index, summon_type;
		const struct monster_spell *spell;

		/* Extract the summon spells */
		create_mon_spell_mask(summon_spells, RST_SUMMON, RST_NONE);
		rsf_inter(summon_spells, mon->race->spell_flags);

		/* Count possibilities */
		for (i = rsf_next(summon_spells, FLAG_START); i != FLAG_END;
			 i = rsf_next(summon_spells, i + 1)) {
			poss++;
		}

		/* Pick one */
		which = randint0(poss);
		index = rsf_next(summon_spells, FLAG_START);
		for (i = 0; i < which; i++) {
			index = rsf_next(summon_spells, index);
		}
		spell = monster_spell_by_index(index);

		/* Set the summon type, and the kin_base if necessary */
		summon_type = spell->effect->subtype;
		if (summon_type == summon_name_to_idx("KIN")) {
			kin_base = mon->race->base;
		}

		/* Choose a race */
		race = select_shape(mon, summon_type);
	}

	/* Print a message immediately, update visuals */
	if (monster_is_obvious(mon)) {
		char m_name[80];
		monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_HID);
		msgt(MSG_GENERIC, "%s %s", m_name, "shimmers and changes!");
		if (player->upkeep->health_who == mon)
			player->upkeep->redraw |= (PR_HEALTH);

		player->upkeep->redraw |= (PR_MONLIST);
		square_light_spot(cave, mon->grid);
	}

	/* Set the race */
	if (race) {
		mon->original_race = mon->race;
		mon->race = race;
		mon->mspeed += mon->race->speed - mon->original_race->speed;
	}

	/* Emergency teleport if needed */
	if (!monster_passes_walls(mon) && square_iswall(cave, mon->grid)) {
		effect_simple(EF_TELEPORT, source_monster(mon->midx), "1", 0, 0, 0,
					  mon->grid.y, mon->grid.x, NULL);
	}

	return mon->original_race != NULL;
}

/**
 * Monster reverse shapechange
 */
bool monster_revert_shape(struct monster *mon)
{
	if (mon->original_race) {
		if (monster_is_obvious(mon)) {
			char m_name[80];
			monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_HID);
			msgt(MSG_GENERIC, "%s %s", m_name, "shimmers and changes!");
			if (player->upkeep->health_who == mon)
				player->upkeep->redraw |= (PR_HEALTH);

			player->upkeep->redraw |= (PR_MONLIST);
			square_light_spot(cave, mon->grid);
		}
		mon->mspeed += mon->original_race->speed - mon->race->speed;
		mon->race = mon->original_race;
		mon->original_race = NULL;

		/* Emergency teleport if needed */
		if (!monster_passes_walls(mon) && square_iswall(cave, mon->grid)) {
			effect_simple(EF_TELEPORT, source_monster(mon->midx), "1", 0, 0, 0,
						  mon->grid.y, mon->grid.x, NULL);
		}

		return true;
	}

	return false;
}
