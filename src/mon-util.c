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
#include "obj-util.h"
#include "player-timed.h"
#include "player-util.h"


/**
 * Returns the monster with the given name. If no monster has the exact name
 * given, returns the first monster with the given name as a (case-insensitive)
 * substring.
 */
monster_race *lookup_monster(const char *name)
{
	int i;
	monster_race *closest = NULL;
	
	/* Look for it */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		if (!r_ptr->name)
			continue;

		/* Test for equality */
		if (streq(name, r_ptr->name))
			return r_ptr;

		/* Test for close matches */
		if (!closest && my_stristr(r_ptr->name, name))
			closest = r_ptr;
	} 

	/* Return our best match */
	return closest;
}

/**
 * Return the monster base matching the given name.
 */
monster_base *lookup_monster_base(const char *name)
{
	monster_base *base;

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
bool match_monster_bases(const monster_base *base, ...)
{
	bool ok = FALSE;
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
void update_mon(struct monster *m_ptr, struct chunk *c, bool full)
{
	monster_lore *l_ptr;

	int d;

	/* Current location */
	int fy, fx;

	/* Seen at all */
	bool flag = FALSE;

	/* Seen by vision */
	bool easy = FALSE;

	/* ESP permitted */
	bool telepathy_ok = TRUE;

	assert(m_ptr != NULL);

	l_ptr = get_lore(m_ptr->race);
	
	fy = m_ptr->fy;
	fx = m_ptr->fx;

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
		m_ptr->cdis = d;
	}

	/* Extract distance */
	else {
		/* Extract the distance */
		d = m_ptr->cdis;
	}

	/* Detected */
	if (m_ptr->mflag & (MFLAG_MARK)) flag = TRUE;

	/* Check if telepathy works */
	if (square_isno_esp(c, fy, fx) ||
		square_isno_esp(c, player->py, player->px))
		telepathy_ok = FALSE;

	/* Nearby */
	if (d <= MAX_SIGHT) {
		/* Basic telepathy */
		if (player_of_has(player, OF_TELEPATHY) && telepathy_ok) {
			/* Empty mind, no telepathy */
			if (rf_has(m_ptr->race->flags, RF_EMPTY_MIND))
			{
				/* Nothing! */
			}

			/* Weird mind, occasional telepathy */
			else if (rf_has(m_ptr->race->flags, RF_WEIRD_MIND)) {
				/* One in ten individuals are detectable */
				if ((m_ptr->midx % 10) == 5) {
					/* Detectable */
					flag = TRUE;

					/* Check for LOS so that MFLAG_VIEW is set later */
					if (player_has_los_bold(fy, fx)) easy = TRUE;
				}
			}

			/* Normal mind, allow telepathy */
			else {
				/* Detectable */
				flag = TRUE;

				/* Check for LOS to that MFLAG_VIEW is set later */
				if (player_has_los_bold(fy, fx)) easy = TRUE;
			}
		}

		/* Normal line of sight and player is not blind */
		if (square_isview(c, fy, fx) && !player->timed[TMD_BLIND]) {
			/* Use "infravision" */
			if (d <= player->state.see_infra) {
				/* Learn about warm/cold blood */
				rf_on(l_ptr->flags, RF_COLD_BLOOD);

				/* Handle "warm blooded" monsters */
				if (!rf_has(m_ptr->race->flags, RF_COLD_BLOOD)) {
					/* Easy to see */
					easy = flag = TRUE;
				}
			}

			/* See if the monster is emitting light */
			/*if (rf_has(m_ptr->race->flags, RF_HAS_LIGHT)) easy = flag = TRUE;*/

			/* Use "illumination" */
			if (square_isseen(c, fy, fx)) {
				/* Learn it emits light */
				rf_on(l_ptr->flags, RF_HAS_LIGHT);

				/* Learn about invisibility */
				rf_on(l_ptr->flags, RF_INVISIBLE);

				/* Handle "invisible" monsters */
				if (rf_has(m_ptr->race->flags, RF_INVISIBLE)) {
					/* See invisible */
					if (player_of_has(player, OF_SEE_INVIS))
					{
						/* Easy to see */
						easy = flag = TRUE;
					}
				}

				/* Handle "normal" monsters */
				else {
					/* Easy to see */
					easy = flag = TRUE;
				}
			}
		}
	}

	/* If a mimic looks like an ignored item, it's not seen */
	if (is_mimicking(m_ptr)) {
		object_type *o_ptr = cave_object(c, m_ptr->mimicked_o_idx);
		if (ignore_item_ok(o_ptr))
			easy = flag = FALSE;
	}
	
	/* The monster is now visible */
	if (flag) {
		/* Learn about the monster's mind */
		if (player_of_has(player, OF_TELEPATHY))
			flags_set(l_ptr->flags, RF_SIZE, RF_EMPTY_MIND, RF_WEIRD_MIND,
					RF_SMART, RF_STUPID, FLAG_END);

		/* It was previously unseen */
		if (!m_ptr->ml) {
			/* Mark as visible */
			m_ptr->ml = TRUE;

			/* Draw the monster */
			square_light_spot(c, fy, fx);

			/* Update health bar as needed */
			if (player->upkeep->health_who == m_ptr)
				player->upkeep->redraw |= (PR_HEALTH);

			/* Hack -- Count "fresh" sightings */
			if (l_ptr->sights < MAX_SHORT)
				l_ptr->sights++;

			/* Window stuff */
			player->upkeep->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not visible */
	else {
		/* It was previously seen */
		if (m_ptr->ml) {
			/* Treat mimics differently */
			if (!m_ptr->mimicked_o_idx || 
				ignore_item_ok(cave_object(c, m_ptr->mimicked_o_idx)))
			{
				/* Mark as not visible */
				m_ptr->ml = FALSE;

				/* Erase the monster */
				square_light_spot(c, fy, fx);

				/* Update health bar as needed */
				if (player->upkeep->health_who == m_ptr)
					player->upkeep->redraw |= (PR_HEALTH);

				/* Window stuff */
				player->upkeep->redraw |= PR_MONLIST;
			}
		}
	}


	/* The monster is now easily visible */
	if (easy) {
		/* Change */
		if (!(m_ptr->mflag & (MFLAG_VIEW))) {
			/* Mark as easily visible */
			m_ptr->mflag |= (MFLAG_VIEW);

			/* Disturb on appearance */
			if (OPT(disturb_near)) disturb(player, 1);

			/* Re-draw monster window */
			player->upkeep->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not easily visible */
	else {
		/* Change */
		if (m_ptr->mflag & (MFLAG_VIEW)) {
			/* Mark as not easily visible */
			m_ptr->mflag &= ~(MFLAG_VIEW);

			/* Disturb on disappearance */
			if (OPT(disturb_near) && !is_mimicking(m_ptr)) disturb(player, 1);

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
		monster_type *m_ptr = cave_monster(cave, i);

		/* Update the monster if alive */
		if (m_ptr->race)
			update_mon(m_ptr, cave, full);
	}
}


/**
 * Add the given object to the given monster's inventory.
 *
 * Returns the o_idx of the new object, or 0 if the object is
 * not successfully added.
 */
s16b monster_carry(struct chunk *c, struct monster *m_ptr, object_type *j_ptr)
{
	s16b o_idx;

	s16b this_o_idx, next_o_idx = 0;

	/* Scan objects already being held for combination */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = cave_object(c, this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr, OSTACK_MONSTER)) {
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Result */
			return (this_o_idx);
		}
	}


	/* Make an object */
	o_idx = o_pop(c);

	/* Success */
	if (o_idx) {
		object_type *o_ptr;

		/* Get new object */
		o_ptr = cave_object(c, o_idx);

		/* Copy object */
		object_copy(o_ptr, j_ptr);

		/* Forget mark */
		o_ptr->marked = FALSE;

		/* Forget location */
		o_ptr->iy = o_ptr->ix = 0;

		/* Link the object to the monster */
		o_ptr->held_m_idx = m_ptr->midx;

		/* Link the object to the pile */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Link the monster to the object */
		m_ptr->hold_o_idx = o_idx;
	}

	/* Result */
	return (o_idx);
}

/**
 * Swap the players/monsters (if any) at two locations.
 */
void monster_swap(int y1, int x1, int y2, int x2)
{
	int m1, m2;

	monster_type *m_ptr;

	/* Monsters */
	m1 = cave->m_idx[y1][x1];
	m2 = cave->m_idx[y2][x2];

	/* Update grids */
	cave->m_idx[y1][x1] = m2;
	cave->m_idx[y2][x2] = m1;

	/* Monster 1 */
	if (m1 > 0) {
		m_ptr = cave_monster(cave, m1);

		/* Move monster */
		m_ptr->fy = y2;
		m_ptr->fx = x2;

		/* Update monster */
		update_mon(m_ptr, cave, TRUE);

		/* Radiate light? */
		if (rf_has(m_ptr->race->flags, RF_HAS_LIGHT))
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
		m_ptr = cave_monster(cave, m2);

		/* Move monster */
		m_ptr->fy = y1;
		m_ptr->fx = x1;

		/* Update monster */
		update_mon(m_ptr, cave, TRUE);

		/* Radiate light? */
		if (rf_has(m_ptr->race->flags, RF_HAS_LIGHT))
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
void become_aware(struct monster *m_ptr)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	if (m_ptr->unaware) {
		m_ptr->unaware = FALSE;

		/* Learn about mimicry */
		if (rf_has(m_ptr->race->flags, RF_UNAWARE))
			rf_on(l_ptr->flags, RF_UNAWARE);

		/* Delete any false items */
		if (m_ptr->mimicked_o_idx > 0) {
			object_type *o_ptr = cave_object(cave, m_ptr->mimicked_o_idx);
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

			/* Print a message */
			msg("The %s was really a monster!", o_name);

			/* Clear the mimicry */
			o_ptr->mimicking_m_idx = 0;

			/* Give the object to the monster if appropriate */
			if (rf_has(m_ptr->race->flags, RF_MIMIC_INV)) {
				object_type *i_ptr;
				object_type object_type_body;
				
				/* Get local object */
				i_ptr = &object_type_body;

				/* Obtain local object */
				object_copy(i_ptr, o_ptr);

				/* Carry the object */
				monster_carry(cave, m_ptr, i_ptr);
			}
				
			/* Delete the mimicked object */
			delete_object_idx(m_ptr->mimicked_o_idx);
			m_ptr->mimicked_o_idx = 0;
		}
		
		/* Update monster and item lists */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);
	}
}

/**
 * Returns TRUE if the given monster is currently mimicking an item.
 */
bool is_mimicking(struct monster *m_ptr)
{
	return (m_ptr->unaware && m_ptr->mimicked_o_idx);
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

	/* anything a monster might learn, the player should learn */
	if (flag) wieldeds_notice_flag(p, flag);
	if (element_ok) wieldeds_notice_element(p, element);

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
