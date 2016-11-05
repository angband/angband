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
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "trap.h"
#include "z-queue.h"

/**
 * This function takes a grid location (x, y) and extracts information the
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
void map_info(unsigned y, unsigned x, struct grid_data *g)
{
	struct object *obj;

	assert(x < (unsigned) cave->width);
	assert(y < (unsigned) cave->height);

	/* Default "clear" values, others will be set later where appropriate. */
	g->first_kind = NULL;
	g->trap = NULL;
	g->multiple_objects = false;
	g->lighting = LIGHTING_DARK;
	g->unseen_object = false;
	g->unseen_money = false;

	/* Use real feature (remove later) */
	g->f_idx = cave->squares[y][x].feat;
	if (f_info[g->f_idx].mimic)
		g->f_idx = f_info[g->f_idx].mimic;

	g->in_view = (square_isseen(cave, y, x)) ? true : false;
	g->is_player = (cave->squares[y][x].mon < 0) ? true : false;
	g->m_idx = (g->is_player) ? 0 : cave->squares[y][x].mon;
	g->hallucinate = player->timed[TMD_IMAGE] ? true : false;

	if (g->in_view) {
		g->lighting = LIGHTING_LOS;

		if (!square_isglow(cave, y, x) && OPT(player, view_yellow_light))
			g->lighting = LIGHTING_TORCH;

		/* Remember seen feature */
		square_memorize(cave, y, x);
	} else if (!square_isknown(cave, y, x)) {
		g->f_idx = FEAT_NONE;
	} else if (square_isglow(cave, y, x)) {
		g->lighting = LIGHTING_LIT;
	}

	/* Use known feature */
	g->f_idx = player->cave->squares[y][x].feat;
	if (f_info[g->f_idx].mimic)
		g->f_idx = f_info[g->f_idx].mimic;

    /* There is a trap in this square */
    if (square_istrap(cave, y, x) && square_isknown(cave, y, x)) {
		struct trap *trap = cave->squares[y][x].trap;

		/* Scan the square trap list */
		while (trap) {
			if (trf_has(trap->flags, TRF_TRAP) ||
				trf_has(trap->flags, TRF_RUNE)) {
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
	for (obj = square_object(player->cave, y, x); obj; obj = obj->next) {
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
		if (!mflag_has(mon->mflag, MFLAG_VISIBLE)) g->m_idx = 0;
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

	assert((int) g->f_idx <= FEAT_PASS_RUBBLE);
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
void square_note_spot(struct chunk *c, int y, int x)
{
	/* Require "seen" flag and the current level */
	if (c != cave) return;
	if (!square_isseen(c, y, x)) return;

	/* Make the player know precisely what is on this grid */
	square_know_pile(c, y, x);

	if (square_isknown(c, y, x))
		return;

	/* Memorize this grid */
	square_memorize(c, y, x);
}



/**
 * Tell the UI that a given map location has been updated
 *
 * This function should only be called on "legal" grids.
 */
void square_light_spot(struct chunk *c, int y, int x)
{
	if (c == cave) {
		player->upkeep->redraw |= PR_ITEMLIST;
		event_signal_point(EVENT_MAP, x, y);
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
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Perma-Light */
		sqinfo_on(cave->squares[y][x].info, SQUARE_GLOW);
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff(player);

	/* Process the grids */
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Redraw the grid */
		square_light_spot(cave, y, x);

		/* Process affected monsters */
		if (cave->squares[y][x].mon > 0)
		{
			int chance = 25;

			struct monster *mon = square_monster(cave, y, x);

			/* Stupid monsters rarely wake up */
			if (rf_has(mon->race->flags, RF_STUPID)) chance = 10;

			/* Smart monsters always wake up */
			if (rf_has(mon->race->flags, RF_SMART)) chance = 100;

			/* Sometimes monsters wake up */
			if (mon->m_timed[MON_TMD_SLEEP] && (randint0(100) < chance))
			{
				/* Wake up! */
				mon_clear_timed(mon, MON_TMD_SLEEP,
					MON_TMD_FLG_NOTIFY, false);

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
	for (i = 0; i < ps->n; i++)
	{
		int y = ps->pts[i].y;
		int x = ps->pts[i].x;

		/* Darken the grid */
		sqinfo_off(cave->squares[y][x].info, SQUARE_GLOW);

		/* Hack -- Forget "boring" grids */
		if (square_isfloor(cave, y, x))
			square_forget(cave, y, x);
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Update stuff */
	update_stuff(player);

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

	if (!square_in_bounds(cave, y, x))
		return;

	if (!square_isroom(cave, y, x))
		return;

	/* Add it to the "seen" set */
	add_to_point_set(seen, y, x);
}

/**
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



/**
 * Light up the dungeon using "claravoyance"
 *
 * This function "illuminates" every grid in the dungeon, memorizes all
 * "objects" (or notes the existence of an object "if" full is true),
 * and memorizes all grids as with magic mapping.
 */
void wiz_light(struct chunk *c, bool full)
{
	int i, y, x;

	/* Scan all grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			/* Process all non-walls */
			if (!square_seemslikewall(c, y, x)) {
				if (!square_in_bounds_fully(c, y, x)) continue;

				/* Scan all neighbors */
				for (i = 0; i < 9; i++) {
					int yy = y + ddy_ddd[i];
					int xx = x + ddx_ddd[i];

					/* Perma-light the grid */
					sqinfo_on(c->squares[yy][xx].info, SQUARE_GLOW);

					/* Memorize normal features */
					if (!square_isfloor(c, yy, xx) || 
						square_isvisibletrap(c, yy, xx)) {
						square_memorize(c, yy, xx);
						square_mark(cave, yy, xx);
					}
				}
			}

			/* Memorize objects */
			if (full) {
				square_know_pile(c, y, x);
			} else {
				square_sense_pile(c, y, x);
			}

			/* Forget unprocessed, unknown grids in the mapping area */
			if (!square_ismark(c, y, x) && square_isnotknown(c, y, x))
				square_forget(c, y, x);
		}
	}

	/* Unmark grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			if (!square_in_bounds(c, y, x)) continue;
			square_unmark(c, y, x);
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw whole map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
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
			
			/* Skip grids with no surrounding floors or stairs */
			for (d = 0; d < 9; d++) {
				/* Extract adjacent (legal) location */
				int yy = y + ddy_ddd[d];
				int xx = x + ddx_ddd[d];

				/* Paranoia */
				if (!square_in_bounds_fully(c, yy, xx)) continue;

				/* Test */
				if (square_isfloor(c, yy, xx) || square_isstairs(c, yy, xx))
					light = true;
			}

			if (!light) continue;

			/* Only interesting grids at night */
			if (daytime || !square_isfloor(c, y, x)) {
				sqinfo_on(c->squares[y][x].info, SQUARE_GLOW);
				square_memorize(c, y, x);
			} else {
				sqinfo_off(c->squares[y][x].info, SQUARE_GLOW);
				square_forget(c, y, x);
			}
		}
	}
			
			
	/* Light shop doorways */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			if (!square_isshop(c, y, x))
				continue;
			for (i = 0; i < 8; i++) {
				int yy = y + ddy_ddd[i];
				int xx = x + ddx_ddd[i];
				sqinfo_on(c->squares[yy][xx].info, SQUARE_GLOW);
				square_memorize(c, yy, xx);
			}
		}
	}


	/* Fully update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}

/**
 * Scent values start at 0 on a newly generated level.  Every time the player
 * moves or the dungeon is altered new scent values are calculated, with
 * all grids within z_info->max_flow_depth getting marked with the
 * current_scent value.
 *
 * Once current_scent value reaches MAX_SCENT, all scent values below
 * MAX_SCENT / 2 are set back to 0, all other scent values are reduced by
 * MAX_SCENT / 2, and current_scent is reduced back to MAX_SCENT / 2.
 *
 * This means that as long as the player does not teleport,
 * then any monster up to 128 + z_info->max_flow_depth will be
 * able to track down the player, and in general, will be
 * able to track down either the player or a position recently
 * occupied by the player.
 */
#define MAX_SCENT 256
static int current_scent = 0;



/**
 * Forget the noise and scent information ready for a complete update
 */
void cave_forget_flow(struct chunk *c)
{
	int x, y;

	/* Nothing to forget */
	if (!current_scent) return;

	/* Check the entire dungeon */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			/* Forget the old data */
			c->squares[y][x].noise = 0;
			c->squares[y][x].scent = 0;
		}
	}

	/* Start over */
	current_scent = 0;
}


/**
 * Fill in the noise field of every grid that the player can reach with
 * the number of steps needed to reach that grid.  This also yields
 * the distance of the player from every grid.
 *
 * Mark the scent of the grids that can reach the player with the
 * incremented value of current_scent.
 *
 * This function currently uses a pair of grid queues for the coordinates of
 * the grids which get scent and noise information.  Making it a single queue
 * of locations would be neater.
 */
void cave_update_flow(struct chunk *c)
{
	int y, x, d;
    struct queue *queue_y = q_new(c->height * c->width);
    struct queue *queue_x = q_new(c->height * c->width);

	/* Reset scent values if current_scent has reached the maximum */
	if (current_scent++ == MAX_SCENT) {
		/* Reduce current_scent */
		current_scent = MAX_SCENT / 2;

		/* Reduce all scent values */
		for (y = 0; y < c->height; y++)	{
			for (x = 0; x < c->width; x++) {
				if (c->squares[y][x].scent >= MAX_SCENT / 2) {
					c->squares[y][x].scent -= MAX_SCENT / 2;
				} else {
					c->squares[y][x].scent = 0;
				}
			}
		}
	}

	/* The player grid is marked with current scent and 0 noise */
	c->squares[player->py][player->px].scent = current_scent;
	c->squares[player->py][player->px].noise = 0;

	/* Start the grid queue */
	q_push_int(queue_y, player->py);
	q_push_int(queue_x, player->px);

	/* Now process the queue */
	while (q_len(queue_y) > 0) {
		/* Get the next grid */
		int ty = q_pop_int(queue_y);
		int tx = q_pop_int(queue_x);

		/* Incremented noise for the adjacent grids */
		int noise = c->squares[ty][tx].noise + 1;

		/* Skip this grid if we've reached maximum flow depth */
		if (noise == z_info->max_flow_depth) continue;

		/* Add the adjacent grids */
		for (d = 0; d < 8; d++) {
			int y = ty + ddy_ddd[d];
			int x = tx + ddx_ddd[d];

			/* Ignore invalid grids */
			if (!square_in_bounds(c, y, x)) continue;

			/* Ignore grids we've already processed */
			if (c->squares[y][x].scent == current_scent) continue;

			/* Ignore grids that don't allow flow information */
			if (square_isnoflow(c, y, x)) continue;

			/* Save the current scent and noise */
			c->squares[y][x].scent = current_scent;
			c->squares[y][x].noise = noise;

			/* Add that grid to the queue */
			q_push_int(queue_y, y);
			q_push_int(queue_x, x);
		}
	}

	q_free(queue_y);
	q_free(queue_x);
}

/**
 * Make map features known, except wall/lava surrounded by wall/lava
 */
void cave_known(struct player *p)
{
	int y, x;
	for (y = 0; y < cave->height; y++) {
		for (x = 0; x < cave->width; x++) {
			int d;
			int xx, yy;
			int count = 0;

			/* Check around the grid */
			for (d = 0; d < 8; d++) {
				/* Extract adjacent location */
				yy = y + ddy_ddd[d];
				xx = x + ddx_ddd[d];

				/* Don't count projectable or lava squares */
				if (!square_isprojectable(cave, yy, xx) ||
					square_isbright(cave, yy, xx))
					++count;
			}

			/* Internal walls not known */
			if (count < 8)
				p->cave->squares[y][x].feat = cave->squares[y][x].feat;
		}
	}
}
