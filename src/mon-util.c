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
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-list.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"


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
		if (streq(name, race->name))
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
 * Nonliving monsters are immune to life drain
 */
bool monster_is_nonliving(struct monster_race *race)
{
	return flags_test(race->flags, RF_SIZE, RF_DEMON, RF_UNDEAD, RF_NONLIVING,
					  FLAG_END);
}

/**
 * Nonliving and stupid monsters are destroyed rather than dying
 */
bool monster_is_unusual(struct monster_race *race)
{
	return (monster_is_nonliving(race) || rf_has(race->flags, RF_STUPID));
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
 * "OPT(disturb_near)" (monster which is "easily" viewable moves in some
 * way).  Note that "moves" includes "appears" and "disappears".
 */
void update_mon(struct monster *mon, struct chunk *c, bool full)
{
	struct monster_lore *lore;

	int d;

	/* Current location */
	int fy, fx;

	/* Seen at all */
	bool flag = false;

	/* Seen by vision */
	bool easy = false;

	/* ESP permitted */
	bool telepathy_ok = true;

	assert(mon != NULL);

	lore = get_lore(mon->race);
	
	fy = mon->fy;
	fx = mon->fx;

	/* Compute distance */
	if (full) {
		int py = player->py;
		int px = player->px;

		/* Distance components */
		int dy = (py > fy) ? (py - fy) : (fy - py);
		int dx = (px > fx) ? (px - fx) : (fx - px);

		/* Approximate distance */
		d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

		/* Restrict distance */
		if (d > 255) d = 255;

		/* Save the distance */
		mon->cdis = d;
	}

	/* Extract distance */
	else {
		/* Extract the distance */
		d = mon->cdis;
	}

	/* Detected */
	if (mflag_has(mon->mflag, MFLAG_MARK)) flag = true;

	/* Check if telepathy works */
	if (square_isno_esp(c, fy, fx) ||
		square_isno_esp(c, player->py, player->px))
		telepathy_ok = false;

	/* Nearby */
	if (d <= z_info->max_sight) {
		/* Basic telepathy */
		if (player_of_has(player, OF_TELEPATHY) && telepathy_ok) {
			/* Empty mind, no telepathy */
			if (rf_has(mon->race->flags, RF_EMPTY_MIND))
			{
				/* Nothing! */
			}

			/* Weird mind, occasional telepathy */
			else if (rf_has(mon->race->flags, RF_WEIRD_MIND)) {
				/* One in ten individuals are detectable */
				if ((mon->midx % 10) == 5) {
					/* Detectable */
					flag = true;

					/* Check for LOS so that MFLAG_VIEW is set later */
					if (square_isview(c, fy, fx)) easy = true;
				}
			}

			/* Normal mind, allow telepathy */
			else {
				/* Detectable */
				flag = true;

				/* Check for LOS to that MFLAG_VIEW is set later */
				if (square_isview(c, fy, fx)) easy = true;
			}
		}

		/* Normal line of sight and player is not blind */
		if (square_isview(c, fy, fx) && !player->timed[TMD_BLIND]) {
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

			/* See if the monster is emitting light */
			/*if (rf_has(mon->race->flags, RF_HAS_LIGHT)) easy = flag = true;*/

			/* Use "illumination" */
			if (square_isseen(c, fy, fx)) {
				/* Learn it emits light */
				rf_on(lore->flags, RF_HAS_LIGHT);

				/* Learn about invisibility */
				rf_on(lore->flags, RF_INVISIBLE);

				/* Handle "invisible" monsters */
				if (rf_has(mon->race->flags, RF_INVISIBLE)) {
					/* See invisible */
					if (player_of_has(player, OF_SEE_INVIS))
					{
						/* Easy to see */
						easy = flag = true;
					}
				}

				/* Handle "normal" monsters */
				else {
					/* Easy to see */
					easy = flag = true;
				}
			}
		}
	}

	/* If a mimic looks like an ignored item, it's not seen */
	if (is_mimicking(mon)) {
		struct object *obj = mon->mimicked_obj;
		if (ignore_item_ok(obj))
			easy = flag = false;
	}

	/* The monster is now visible */
	if (flag) {
		/* Learn about the monster's mind */
		if (player_of_has(player, OF_TELEPATHY))
			flags_set(lore->flags, RF_SIZE, RF_EMPTY_MIND, RF_WEIRD_MIND,
					RF_SMART, RF_STUPID, FLAG_END);

		/* It was previously unseen */
		if (!mflag_has(mon->mflag, MFLAG_VISIBLE)) {
			/* Mark as visible */
			mflag_on(mon->mflag, MFLAG_VISIBLE);

			/* Draw the monster */
			square_light_spot(c, fy, fx);

			/* Update health bar as needed */
			if (player->upkeep->health_who == mon)
				player->upkeep->redraw |= (PR_HEALTH);

			/* Hack -- Count "fresh" sightings */
			if (lore->sights < SHRT_MAX)
				lore->sights++;

			/* Window stuff */
			player->upkeep->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not visible */
	else {
		/* It was previously seen */
		if (mflag_has(mon->mflag, MFLAG_VISIBLE)) {
			/* Treat mimics differently */
			if (!mon->mimicked_obj || ignore_item_ok(mon->mimicked_obj)) {
				/* Mark as not visible */
				mflag_off(mon->mflag, MFLAG_VISIBLE);

				/* Erase the monster */
				square_light_spot(c, fy, fx);

				/* Update health bar as needed */
				if (player->upkeep->health_who == mon)
					player->upkeep->redraw |= (PR_HEALTH);

				/* Window stuff */
				player->upkeep->redraw |= PR_MONLIST;
			}
		}
	}


	/* The monster is now easily visible */
	if (easy) {
		/* Change */
		if (!mflag_has(mon->mflag, MFLAG_VIEW)) {
			/* Mark as easily visible */
			mflag_on(mon->mflag, MFLAG_VIEW);

			/* Disturb on appearance */
			if (OPT(disturb_near)) disturb(player, 1);

			/* Re-draw monster window */
			player->upkeep->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not easily visible */
	else {
		/* Change */
		if (mflag_has(mon->mflag, MFLAG_VIEW)) {
			/* Mark as not easily visible */
			mflag_off(mon->mflag, MFLAG_VIEW);

			/* Disturb on disappearance */
			if (OPT(disturb_near) && !is_mimicking(mon)) disturb(player, 1);

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
	obj->iy = obj->ix = 0;

	/* Link the object to the monster */
	obj->held_m_idx = mon->midx;

	/* Add the object to the monster's inventory */
	list_object(c, obj);
	pile_insert(&mon->held_obj, obj);

	/* Result */
	return true;
}

/**
 * Swap the players/monsters (if any) at two locations.
 */
void monster_swap(int y1, int x1, int y2, int x2)
{
	int m1, m2;

	struct monster *mon;

	/* Monsters */
	m1 = cave->squares[y1][x1].mon;
	m2 = cave->squares[y2][x2].mon;

	/* Update grids */
	cave->squares[y1][x1].mon = m2;
	cave->squares[y2][x2].mon = m1;

	/* Monster 1 */
	if (m1 > 0) {
		mon = cave_monster(cave, m1);

		/* Move monster */
		mon->fy = y2;
		mon->fx = x2;

		/* Update monster */
		update_mon(mon, cave, true);

		/* Radiate light? */
		if (rf_has(mon->race->flags, RF_HAS_LIGHT))
			player->upkeep->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);
	}

	/* Player 1 */
	else if (m1 < 0) {
		/* Move player */
		player->py = y2;
		player->px = x2;

		/* Update the trap detection status */
		player->upkeep->redraw |= (PR_DTRAP);

		/* Update the panel */
		player->upkeep->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		player->upkeep->update |= (PU_UPDATE_FLOW);

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);
	}

	/* Monster 2 */
	if (m2 > 0) {
		mon = cave_monster(cave, m2);

		/* Move monster */
		mon->fy = y1;
		mon->fx = x1;

		/* Update monster */
		update_mon(mon, cave, true);

		/* Radiate light? */
		if (rf_has(mon->race->flags, RF_HAS_LIGHT))
			player->upkeep->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);
	}

	/* Player 2 */
	else if (m2 < 0) {
		/* Move player */
		player->py = y1;
		player->px = x1;

		/* Update the trap detection status */
		player->upkeep->redraw |= (PR_DTRAP);

		/* Update the panel */
		player->upkeep->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		player->upkeep->update |= (PU_UPDATE_FLOW);

		/* Redraw monster list */
		player->upkeep->redraw |= (PR_MONLIST);
	}

	/* Redraw */
	square_light_spot(cave, y1, x1);
	square_light_spot(cave, y2, x2);
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

	if (mflag_has(mon->mflag, MFLAG_UNAWARE)) {
		mflag_off(mon->mflag, MFLAG_UNAWARE);

		/* Learn about mimicry */
		if (rf_has(mon->race->flags, RF_UNAWARE))
			rf_on(lore->flags, RF_UNAWARE);

		/* Delete any false items */
		if (mon->mimicked_obj) {
			struct object *obj = mon->mimicked_obj;
			char o_name[80];
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

			/* Print a message */
			if (square_isseen(cave, obj->iy, obj->ix))
				msg("The %s was really a monster!", o_name);

			/* Clear the mimicry */
			obj->mimicking_m_idx = 0;
			mon->mimicked_obj = NULL;

			square_excise_object(cave, obj->iy, obj->ix, obj);

			/* Give the object to the monster if appropriate */
			if (rf_has(mon->race->flags, RF_MIMIC_INV))
				monster_carry(cave, mon, obj);
			else {
				/* Otherwise delete the mimicked object */
				delist_object(cave, obj);
				object_delete(&obj);
			}
		}

		/* Update monster and item lists */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);
	}

	square_note_spot(cave, mon->fy, mon->fx);
	square_light_spot(cave, mon->fy, mon->fx);
}

/**
 * Returns true if the given monster is currently mimicking an item.
 */
bool is_mimicking(struct monster *mon)
{
	return (mflag_has(mon->mflag, MFLAG_UNAWARE) && mon->mimicked_obj);
}


/**
 * The given monster learns about an "observed" resistance or other player
 * state property, or lack of it.
 *
 * Note that this function is robust to being called with `element` as an
 * arbitrary GF_ type
 */
void update_smart_learn(struct monster *m, struct player *p, int flag,
						int pflag, int element)
{
	bool element_ok = ((element >= 0) && (element < ELEM_MAX));

	/* Sanity check */
	if (!flag && !element_ok) return;

	/* Anything a monster might learn, the player should learn */
	if (flag) equip_learn_flag(p, flag);
	if (element_ok) equip_learn_element(p, element);

	/* Not allowed to learn */
	if (!OPT(birth_ai_learn)) return;

	/* Too stupid to learn anything */
	if (rf_has(m->race->flags, RF_STUPID)) return;

	/* Not intelligent, only learn sometimes */
	if (!rf_has(m->race->flags, RF_SMART) && one_in_(2)) return;

	/* Analyze the knowledge; fail very rarely */
	if (one_in_(100))
		return;

	/* Learn the flag */
	if (flag) {
		if (player_of_has(p, flag))
			of_on(m->known_pstate.flags, flag);
		else
			of_off(m->known_pstate.flags, flag);
	}

	/* Learn the pflag */
	if (pflag) {
		if (pf_has(player->state.pflags, pflag))
			of_on(m->known_pstate.pflags, pflag);
		else
			of_off(m->known_pstate.pflags, pflag);
	}

	/* Learn the element */
	if (element_ok)
		m->known_pstate.el_info[element].res_level
			= player->state.el_info[element].res_level;
}
