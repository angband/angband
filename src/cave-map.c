/**
 * \file cave-map.c
 * \brief Lighting and map management functions
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
#include "init.h"
#include "monster.h"
#include "mon-predicate.h"
#include "mon-util.h"
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "trap.h"

/**
 * This function takes a grid location and extracts information the
 * player is allowed to know about it, filling in the grid_data structure
 * passed in 'g'.
 *
 * The information filled in is as follows:
 *  - g->f_idx is filled in with the terrain's feature type, or FEAT_NONE
 *    if the player doesn't know anything about the grid.  The function
 *    makes use of the "mimic" field in terrain in order to allow one
 *    feature to look like another (hiding secret doors, invisible traps,
 *    etc).  This will return the terrain type the player "Knows" about,
 *    not necessarily the real terrain.
 *  - g->m_idx is set to the monster index, or 0 if there is none (or the
 *    player doesn't know it).
 *  - g->first_kind is set to the object_kind of the first object in a grid
 *    that the player knows about, or NULL for no objects.
 *  - g->muliple_objects is true if there is more than one object in the
 *    grid that the player knows and cares about (to facilitate any special
 *    floor stack symbol that might be used).
 *  - g->in_view is true if the player can currently see the grid - this can
 *    be used to indicate field-of-view, such as through the 
 *    OPT(player, view_bright_light) option.
 *  - g->lighting is set to indicate the lighting level for the grid:
 *    LIGHTING_DARK for unlit grids, LIGHTING_LIT for inherently light
 *    grids (lit rooms, etc), LIGHTING_TORCH for grids lit by the player's
 *    light source, and LIGHTING_LOS for grids in the player's line of sight.
 *    Note that lighting is always LIGHTING_LIT for known "interesting" grids
 *    like walls.
 *  - g->is_player is true if the player is on the given grid.
 *  - g->hallucinate is true if the player is hallucinating something "strange"
 *    for this grid - this should pick a random monster to show if the m_idx
 *    is non-zero, and a random object if first_kind is non-zero.
 * 
 * NOTES:
 * This is called pretty frequently, whenever a grid on the map display
 * needs updating, so don't overcomplicate it.
 *
 * Terrain is remembered separately from objects and monsters, so can be
 * shown even when the player can't "see" it.  This leads to things like
 * doors out of the player's view still change from closed to open and so on.
 *
 * TODO:
 * Hallucination is currently disabled (it was a display-level hack before,
 * and we need it to be a knowledge-level hack).  The idea is that objects
 * may turn into different objects, monsters into different monsters, and
 * terrain may be objects, monsters, or stay the same.
 */
void map_info(struct loc grid, struct grid_data *g)
{
	struct object *obj;

	assert(grid.x < cave->width);
	assert(grid.y < cave->height);

	/* Default "clear" values, others will be set later where appropriate. */
	g->first_kind = NULL;
	g->trap = NULL;
	g->multiple_objects = false;
	g->lighting = LIGHTING_LIT;
	g->unseen_object = false;
	g->unseen_money = false;

	/* Use real feature (remove later) */
	g->f_idx = square(cave, grid).feat;
	if (f_info[g->f_idx].mimic)
		g->f_idx = lookup_feat(f_info[g->f_idx].mimic);

	g->in_view = (square_isseen(cave, grid)) ? true : false;
	g->is_player = (square(cave, grid).mon < 0) ? true : false;
	g->m_idx = (g->is_player) ? 0 : square(cave, grid).mon;
	g->hallucinate = player->timed[TMD_IMAGE] ? true : false;

	if (g->in_view) {
		g->lighting = LIGHTING_LOS;

		/* Darkness or torchlight */
		if (!square_isglow(cave, grid)) {
			if (player_has(player, PF_UNLIGHT) && !square_islit(cave, grid)) {
				g->lighting = LIGHTING_DARK;
			} else if (OPT(player, view_yellow_light)) {
				g->lighting = LIGHTING_TORCH;
			}
		} else if (square_iswall(cave, grid)) {
			/* Lit walls only show as lit if we are looking from the room
			 * that's lighting them */
			if (!square_islitwall(cave, grid)) {
				if (square_islit(cave, grid)) {
					if (OPT(player, view_yellow_light)) {
						g->lighting = LIGHTING_TORCH;
					} else if (player_has(player, PF_UNLIGHT)) {
						g->lighting = LIGHTING_DARK;
					} else {
						g->lighting = LIGHTING_LOS;
					}
				} else {
					g->lighting = LIGHTING_LIT;
				}
			}
		}

		/* Remember seen feature */
		square_memorize(cave, grid);
	} else if (!square_isknown(cave, grid)) {
		g->f_idx = FEAT_NONE;
	} else if (square_isglow(cave, grid)) {
		g->lighting = LIGHTING_LIT;
	}

	/* Use known feature */
	g->f_idx = square(player->cave, grid).feat;
	if (f_info[g->f_idx].mimic)
		g->f_idx = lookup_feat(f_info[g->f_idx].mimic);

	/* There is a known trap in this square */
	if (square_trap(player->cave, grid) && square_isknown(cave, grid)) {
		struct trap *trap = square(player->cave, grid).trap;

		/* Scan the square trap list */
		while (trap) {
			if (trf_has(trap->flags, TRF_TRAP) ||
				trf_has(trap->flags, TRF_GLYPH) ||
				trf_has(trap->flags, TRF_WEB)) {
				/* Accept the trap - only if not disabled, maybe we need
				 * a special graphic for this */
				if (!trap->timeout) {
					g->trap = trap;
					break;
				}
			}
			trap = trap->next;
		}
    }

	/* Objects */
	for (obj = square_object(player->cave, grid); obj; obj = obj->next) {
		if (obj->kind == unknown_gold_kind) {
			g->unseen_money = true;
		} else if (obj->kind == unknown_item_kind) {
			g->unseen_object = true;
		} else if (ignore_known_item_ok(obj)) {
			/* Item stays hidden */
		} else if (!g->first_kind) {
			g->first_kind = obj->kind;
		} else {
			g->multiple_objects = true;
			break;
		}
	}

	/* Monsters */
	if (g->m_idx > 0) {
		/* If the monster isn't "visible", make sure we don't list it.*/
		struct monster *mon = cave_monster(cave, g->m_idx);
		if (!monster_is_visible(mon)) g->m_idx = 0;
	}

	/* Rare random hallucination on non-outer walls */
	if (g->hallucinate && g->m_idx == 0 && g->first_kind == 0) {
		if (one_in_(128) && (int) g->f_idx != FEAT_PERM)
			g->m_idx = 1;
		else if (one_in_(128) && (int) g->f_idx != FEAT_PERM)
			/* if hallucinating, we just need first_kind to not be NULL */
			g->first_kind = k_info;
		else
			g->hallucinate = false;
	}

	assert((int) g->f_idx < z_info->f_max);
	if (!g->hallucinate)
		assert((int)g->m_idx < cave->mon_max);
	/* All other g fields are 'flags', mostly booleans. */
}


/**
 * Memorize interesting viewable object/features in the given grid
 *
 * This function should only be called on "legal" grids.
 *
 * This function will memorize the object and/or feature in the given grid,
 * if they are (1) see-able and (2) interesting.  Note that all objects are
 * interesting, all terrain features except floors (and invisible traps) are
 * interesting, and floors (and invisible traps) are interesting sometimes
 * (depending on various options involving the illumination of floor grids).
 *
 * The automatic memorization of all objects and non-floor terrain features
 * as soon as they are displayed allows incredible amounts of optimization
 * in various places, especially "map_info()" and this function itself.
 *
 * Note that the memorization of objects is completely separate from the
 * memorization of terrain features, preventing annoying floor memorization
 * when a detected object is picked up from a dark floor, and object
 * memorization when an object is dropped into a floor grid which is
 * memorized but out-of-sight.
 *
 * This function should be called every time the "memorization" of a grid
 * (or the object in a grid) is called into question, such as when an object
 * is created in a grid, when a terrain feature "changes" from "floor" to
 * "non-floor", and when any grid becomes "see-able" for any reason.
 *
 * This function is called primarily from the "update_view()" function, for
 * each grid which becomes newly "see-able".
 */
void square_note_spot(struct chunk *c, struct loc grid)
{
	/* Require "seen" flag and the current level */
	if (c != cave) return;
	if (!square_isseen(c, grid) && !square_isplayer(c, grid)) return;

	/* Make the player know precisely what is on this grid */
	square_know_pile(c, grid);

	/* Notice traps, memorize those we can see */
	if (square_issecrettrap(c, grid)) {
		square_reveal_trap(c, grid, false, true);
	}
	square_memorize_traps(c, grid);

	if (square_isknown(c, grid))
		return;

	/* Memorize this grid */
	square_memorize(c, grid);
}



/**
 * Tell the UI that a given map location has been updated
 *
 * This function should only be called on "legal" grids.
 */
void square_light_spot(struct chunk *c, struct loc grid)
{
	if ((c == cave) && player->cave) {
		player->upkeep->redraw |= PR_ITEMLIST;
		event_signal_point(EVENT_MAP, grid.x, grid.y);
	}
}


/**
 * This routine will Perma-Light all grids in the set passed in.
 *
 * This routine is used (only) by "light_room()"
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
	for (i = 0; i < ps->n; i++)	{
		/* Perma-Light */
		sqinfo_on(square(cave, ps->pts[i]).info, SQUARE_GLOW);
	}

	/* Process the grids */
	for (i = 0; i < ps->n; i++)	{
		/* Redraw the grid */
		square_light_spot(cave, ps->pts[i]);

		/* Process affected monsters */
		if (square(cave, ps->pts[i]).mon > 0) {
			int chance = 25;

			struct monster *mon = square_monster(cave, ps->pts[i]);

			/* Stupid monsters rarely wake up */
			if (monster_is_stupid(mon)) chance = 10;

			/* Smart monsters always wake up */
			if (monster_is_smart(mon)) chance = 100;

			/* Sometimes monsters wake up, and become aware if they do */
			if (mon->m_timed[MON_TMD_SLEEP] && (randint0(100) < chance)) {
				monster_wake(mon, true, 100);
			}
		}
	}
}



/**
 * This routine will "darken" all grids in the set passed in.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "light_room()"
 */
static void cave_unlight(struct point_set *ps)
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < ps->n; i++)	{
		struct loc grid = ps->pts[i];

		/* Darken the grid... */
		if (!square_isbright(cave, ps->pts[i])) {
			sqinfo_off(square(cave, ps->pts[i]).info, SQUARE_GLOW);
		}

		/* ...but dark-loving characters remember them */
		if (player_has(player, PF_UNLIGHT)) {
			square_memorize(cave, grid);
		}

		/* Hack -- Forget "boring" grids */
		if (square_isfloor(cave, grid))
			square_forget(cave, grid);
	}

	/* Process the grids */
	for (i = 0; i < ps->n; i++)	{
		/* Redraw the grid */
		square_light_spot(cave, ps->pts[i]);
	}
}

/*
 * Aux function -- see below
 */
static void cave_room_aux(struct point_set *seen, struct loc grid)
{
	if (point_set_contains(seen, grid))
		return;

	if (!square_in_bounds(cave, grid))
		return;

	if (!square_isroom(cave, grid))
		return;

	/* Add it to the "seen" set */
	add_to_point_set(seen, grid);
}

/**
 * Illuminate or darken any room containing the given location.
 */
void light_room(struct loc grid, bool light)
{
	int i, d;
	struct point_set *ps;

	ps = point_set_new(200);

	/* Add the initial grid */
	cave_room_aux(ps, grid);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < ps->n; i++) {
		/* Walls get lit, but stop light */
		if (!square_isprojectable(cave, ps->pts[i])) continue;

		/* Spread to the adjacent grids */
		for (d = 0; d < 8; d++) {
			cave_room_aux(ps, loc_sum(ps->pts[i], ddgrid_ddd[d]));
		}
	}

	/* Now, lighten or darken them all at once */
	if (light) {
		cave_light(ps);
	} else {
		cave_unlight(ps);
	}
	point_set_dispose(ps);

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff(player);
}



/**
 * Light up the dungeon using "claravoyance"
 *
 * This function "illuminates" every grid in the dungeon, memorizes all
 * "objects" (or notes the existence of an object "if" full is true),
 * and memorizes all grids as with magic mapping.
 */
void wiz_light(struct chunk *c, struct player *p, bool full)
{
	int i, y, x;

	/* Scan all grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			struct loc grid = loc(x, y);

			/* Process all non-walls */
			if (!square_seemslikewall(c, grid)) {
				if (!square_in_bounds_fully(c, grid)) continue;

				/* Scan all neighbors */
				for (i = 0; i < 9; i++) {
					struct loc a_grid = loc_sum(grid, ddgrid_ddd[i]);

					/* Perma-light the grid */
					sqinfo_on(square(c, a_grid).info, SQUARE_GLOW);

					/* Memorize normal features */
					if (!square_isfloor(c, a_grid) || 
						square_isvisibletrap(c, a_grid)) {
						square_memorize(c, a_grid);
						square_mark(c, a_grid);
					}
				}
			}

			/* Memorize objects */
			if (full) {
				square_know_pile(c, grid);
			} else {
				square_sense_pile(c, grid);
			}

			/* Forget unprocessed, unknown grids in the mapping area */
			if (!square_ismark(c, grid) && square_isnotknown(c, grid))
				square_forget(c, grid);
		}
	}

	/* Unmark grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			struct loc grid = loc(x, y);
			if (!square_in_bounds(c, grid)) continue;
			square_unmark(c, grid);
		}
	}

	/* Fully update the visuals */
	p->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw whole map, monster list */
	p->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}


/**
 * Compeletly darken the level, and know all objects
 *
 * This function darkens every grid in the dungeon, memorizes all
 * "objects" (or notes the existence of an object "if" full is true),
 * and memorizes all grids as with magic mapping.
 */
void wiz_dark(struct chunk *c, struct player *p, bool full)
{
	int i, y, x;

	/* Scan all grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			struct loc grid = loc(x, y);

			/* Process all non-walls */
			if (!square_seemslikewall(c, grid)) {
				if (!square_in_bounds_fully(c, grid)) continue;

				/* Scan all neighbors */
				for (i = 0; i < 9; i++) {
					struct loc a_grid = loc_sum(grid, ddgrid_ddd[i]);

					/* Perma-darken the grid */
					sqinfo_off(square(cave, a_grid).info, SQUARE_GLOW);

					/* Memorize normal features */
					if (!square_isfloor(c, a_grid) || 
						square_isvisibletrap(c, a_grid)) {
						square_memorize(c, a_grid);
						square_mark(c, a_grid);
					}
				}
			}

			/* Memorize objects */
			if (full) {
				square_know_pile(c, grid);
			} else {
				square_sense_pile(c, grid);
			}

			/* Forget unprocessed, unknown grids in the mapping area */
			if (!square_ismark(c, grid) && square_isnotknown(c, grid))
				square_forget(c, grid);
		}
	}

	/* Unmark grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			struct loc grid = loc(x, y);
			if (!square_in_bounds(c, grid)) continue;
			square_unmark(c, grid);
		}
	}

	/* Fully update the visuals */
	p->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw whole map, monster list */
	p->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}


/**
 * Light or Darken the town
 */
void cave_illuminate(struct chunk *c, bool daytime)
{
	int y, x, i;

	/* Apply light or darkness */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			int d;
			bool light = false;
			struct loc grid = loc(x, y);
			
			/* Skip grids with no surrounding floors or stairs */
			for (d = 0; d < 9; d++) {
				/* Extract adjacent (legal) location */
				struct loc a_grid = loc_sum(grid, ddgrid_ddd[d]);

				/* Paranoia */
				if (!square_in_bounds_fully(c, a_grid)) continue;

				/* Test */
				if (square_isfloor(c, a_grid) || square_isstairs(c, a_grid))
					light = true;
			}

			/* Only interesting grids at night */
			if (daytime || !square_isfloor(c, grid)) {
				sqinfo_on(square(c, grid).info, SQUARE_GLOW);
				if(light) square_memorize(c, grid);
			} else if (!square_isbright(c, grid)) {
				sqinfo_off(square(c, grid).info, SQUARE_GLOW);
				square_forget(c, grid);
			}
		}
	}
			
			
	/* Light shop doorways */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			struct loc grid = loc(x, y);
			if (!square_isshop(c, grid))
				continue;
			for (i = 0; i < 8; i++) {
				struct loc a_grid = loc_sum(grid, ddgrid_ddd[i]);
				sqinfo_on(square(c, a_grid).info, SQUARE_GLOW);
				square_memorize(c, a_grid);
			}
		}
	}


	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}

/**
 * Make map features known, except wall/lava surrounded by wall/lava
 */
void cave_known(struct player *p)
{
	int y, x;
	for (y = 0; y < cave->height; y++) {
		for (x = 0; x < cave->width; x++) {
			struct loc grid = loc(x, y);
			int d;
			int count = 0;

			/* Check around the grid */
			for (d = 0; d < 8; d++) {
				/* Extract adjacent location */
				struct loc adj_grid = loc_sum(grid, ddgrid_ddd[d]);

				/* Don't count projectable or lava squares */
				if (!square_isprojectable(cave, adj_grid) ||
					square_isbright(cave, adj_grid))
					++count;
			}

			/* Internal walls not known */
			if (count < 8) {
				p->cave->squares[y][x].feat = square(cave, grid).feat;
			}
		}
	}
}
