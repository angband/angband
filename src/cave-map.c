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
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "trap.h"

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
 *  - g->muliple_objects is TRUE if there is more than one object in the
 *    grid that the player knows and cares about (to facilitate any special
 *    floor stack symbol that might be used).
 *  - g->in_view is TRUE if the player can currently see the grid - this can
 *    be used to indicate field-of-view, such as through the OPT(view_bright_light)
 *    option.
 *  - g->lighting is set to indicate the lighting level for the grid:
 *    LIGHTING_DARK for unlit grids, LIGHTING_LIT for inherently light
 *    grids (lit rooms, etc), LIGHTING_TORCH for grids lit by the player's
 *    light source, and LIGHTING_LOS for grids in the player's line of sight.
 *    Note that lighting is always LIGHTING_LIT for known "interesting" grids
 *    like walls.
 *  - g->is_player is TRUE if the player is on the given grid.
 *  - g->hallucinate is TRUE if the player is hallucinating something "strange"
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
	g->multiple_objects = FALSE;
	g->lighting = LIGHTING_DARK;
	g->unseen_object = FALSE;
	g->unseen_money = FALSE;

	/* Use real feature (remove later) */
	g->f_idx = cave->squares[y][x].feat;
	if (f_info[g->f_idx].mimic)
		g->f_idx = f_info[g->f_idx].mimic;

	g->in_view = (square_isseen(cave, y, x)) ? TRUE : FALSE;
	g->is_player = (cave->squares[y][x].mon < 0) ? TRUE : FALSE;
	g->m_idx = (g->is_player) ? 0 : cave->squares[y][x].mon;
	g->hallucinate = player->timed[TMD_IMAGE] ? TRUE : FALSE;
	g->trapborder = (square_isdedge(cave, y, x)) ? TRUE : FALSE;

	if (g->in_view) {
		g->lighting = LIGHTING_LOS;

		if (!square_isglow(cave, y, x) && OPT(view_yellow_light))
			g->lighting = LIGHTING_TORCH;

		/* Remember seen feature */
		cave_k->squares[y][x].feat = cave->squares[y][x].feat;
	} else if (!square_ismark(cave, y, x)) {
		g->f_idx = FEAT_NONE;
		//cave_k->squares[y][x].feat = FEAT_NONE;
	} else if (square_isglow(cave, y, x)) {
		g->lighting = LIGHTING_LIT;
	}

	/* Use known feature */
/*	g->f_idx = cave_k->squares[y][x].feat;
	if (f_info[g->f_idx].mimic)
		g->f_idx = f_info[g->f_idx].mimic;*/

    /* There is a trap in this square */
    if (square_istrap(cave, y, x) && square_ismark(cave, y, x)) {
		struct trap *trap = cave->squares[y][x].trap;

		/* Scan the square trap list */
		while (trap) {
			if (trf_has(trap->flags, TRF_TRAP) ||
				trf_has(trap->flags, TRF_RUNE)) {
				/* Accept the trap */
				g->trap = trap;
				break;
			}
			trap = trap->next;
		}
    }

	/* Objects */
	for (obj = square_object(cave, y, x); obj; obj = obj->next) {
		if (obj->marked == MARK_AWARE) {

			/* Distinguish between unseen money and objects */
			if (tval_is_money(obj)) {
				g->unseen_money = TRUE;
			} else {
				g->unseen_object = TRUE;
			}

		} else if (obj->marked == MARK_SEEN && !ignore_item_ok(obj)) {
			if (!g->first_kind) {
				g->first_kind = obj->kind;
			} else {
				g->multiple_objects = TRUE;
				break;
			}
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
			g->hallucinate = FALSE;
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
	struct object *obj;

	/* Require "seen" flag */
	if (!square_isseen(c, y, x))
		return;

	for (obj = square_object(c, y, x); obj; obj = obj->next)
		obj->marked = MARK_SEEN;

	if (square_ismark(c, y, x))
		return;

	/* Memorize this grid */
	sqinfo_on(c->squares[y][x].info, SQUARE_MARK);
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
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

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
					MON_TMD_FLG_NOTIFY, FALSE);

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
			sqinfo_off(cave->squares[y][x].info, SQUARE_MARK);
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

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
 * "objects" (or notes the existence of an object "if" full is TRUE),
 * and memorizes all grids as with magic mapping.
 */
void wiz_light(struct chunk *c, bool full)
{
	int i, y, x;

	/* Scan all grids */
	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			struct object *obj;

			/* Process all non-walls */
			if (!square_seemslikewall(c, y, x)) {
				/* Scan all neighbors */
				for (i = 0; i < 9; i++) {
					int yy = y + ddy_ddd[i];
					int xx = x + ddx_ddd[i];

					/* Perma-light the grid */
					sqinfo_on(c->squares[yy][xx].info, SQUARE_GLOW);

					/* Memorize normal features */
					if (!square_isfloor(c, yy, xx) || 
						square_isvisibletrap(c, yy, xx)) {
						sqinfo_on(c->squares[yy][xx].info, SQUARE_MARK);
						cave_k->squares[yy][xx].feat = c->squares[yy][xx].feat;
					}
				}
			}

			/* Memorize objects */
			for (obj = square_object(cave, y, x); obj; obj = obj->next) {
				/* Skip dead objects */
				assert(obj->kind);

				/* Memorize it */
				if (obj->marked < MARK_SEEN)
					obj->marked = full ? MARK_SEEN : MARK_AWARE;
			}
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw whole map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}


/**
 * Forget the dungeon map (ala "Thinking of Maud...").
 */
void wiz_dark(void)
{
	int y, x;


	/* Forget every grid */
	for (y = 0; y < cave->height; y++) {
		for (x = 0; x < cave->width; x++) {
			struct object *obj;

			/* Process the grid */
			sqinfo_off(cave->squares[y][x].info, SQUARE_MARK);
			sqinfo_off(cave->squares[y][x].info, SQUARE_DTRAP);
			sqinfo_off(cave->squares[y][x].info, SQUARE_DEDGE);

			/* Forget all objects */
			for (obj = square_object(cave, y, x); obj; obj = obj->next) {
				/* Skip dead objects */
				assert(obj->kind);

				/* Forget the object */
				obj->marked = MARK_UNAWARE;
			}
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}



/**
 * Light or Darken the town
 */
void cave_illuminate(struct chunk *c, bool daytime)
{
	int y, x, i;

	/* Apply light or darkness */
	for (y = 0; y < c->height; y++)
		for (x = 0; x < c->width; x++) {
			int d;
			bool light = FALSE;
			struct feature *feat = &f_info[c->squares[y][x].feat];
			
			/* Skip grids with no surrounding floors or stairs */
			for (d = 0; d < 9; d++) {
				/* Extract adjacent (legal) location */
				int yy = y + ddy_ddd[d];
				int xx = x + ddx_ddd[d];

				/* Paranoia */
				if (!square_in_bounds_fully(c, yy, xx)) continue;

				/* Test */
				if (square_isfloor(c, yy, xx) || square_isstairs(c, yy, xx))
					light = TRUE;
			}

			if (!light) continue;

			/* Only interesting grids at night */
			if (daytime || !tf_has(feat->flags, TF_FLOOR)) {
				sqinfo_on(c->squares[y][x].info, SQUARE_GLOW);
				sqinfo_on(c->squares[y][x].info, SQUARE_MARK);
			} else {
				sqinfo_off(c->squares[y][x].info, SQUARE_GLOW);
				sqinfo_off(c->squares[y][x].info, SQUARE_MARK);
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
				sqinfo_on(c->squares[yy][xx].info, SQUARE_MARK);
			}
		}
	}


	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw map, monster list */
	player->upkeep->redraw |= (PR_MAP | PR_MONLIST | PR_ITEMLIST);
}

/**
 * Size of the circular queue used by "update_flow()"
 */
#define FLOW_MAX 2048

/*
 * Hack -- provide some "speed" for the "flow" code
 * This entry is the "current index" for the "when" field
 * Note that a "when" value of "zero" means "not used".
 *
 * Note that the "cost" indexes from 1 to 127 are for
 * "old" data, and from 128 to 255 are for "new" data.
 *
 * This means that as long as the player does not "teleport",
 * then any monster up to 128 + z_info->max_flow_depth will be
 * able to track down the player, and in general, will be
 * able to track down either the player or a position recently
 * occupied by the player.
 */
static int flow_save = 0;



/**
 * Forget the "flow" information ready for a complete update
 */
void cave_forget_flow(struct chunk *c)
{
	int x, y;

	/* Nothing to forget */
	if (!flow_save) return;

	/* Check the entire dungeon */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			/* Forget the old data */
			c->squares[y][x].cost = 0;
			c->squares[y][x].when = 0;
		}
	}

	/* Start over */
	flow_save = 0;
}


/*
 * Hack -- fill in the "cost" field of every grid that the player can
 * "reach" with the number of steps needed to reach that grid.  This
 * also yields the "distance" of the player from every grid.
 *
 * In addition, mark the "when" of the grids that can reach the player
 * with the incremented value of "flow_save".
 *
 * Hack -- use the local "flow_y" and "flow_x" arrays as a "circular
 * queue" of cave grids.
 *
 * We do not need a priority queue because the cost from grid to grid
 * is always "one" (even along diagonals) and we process them in order.
 */
void cave_update_flow(struct chunk *c)
{
	int py = player->py;
	int px = player->px;

	int ty, tx;

	int y, x;

	int n, d;

	int flow_n;

	int flow_tail = 0;
	int flow_head = 0;

	byte flow_y[FLOW_MAX];
	byte flow_x[FLOW_MAX];


	/*** Cycle the flow ***/

	/* Cycle the flow */
	if (flow_save++ == 255)
	{
		/* Cycle the flow */
		for (y = 0; y < c->height; y++)
		{
			for (x = 0; x < c->width; x++)
			{
				int w = c->squares[y][x].when;
				c->squares[y][x].when = (w >= 128) ? (w - 128) : 0;
			}
		}

		/* Restart */
		flow_save = 128;
	}

	/* Local variable */
	flow_n = flow_save;


	/*** Player Grid ***/

	/* Save the time-stamp */
	c->squares[py][px].when = flow_n;

	/* Save the flow cost */
	c->squares[py][px].cost = 0;

	/* Enqueue that entry */
	flow_y[flow_head] = py;
	flow_x[flow_head] = px;

	/* Advance the queue */
	++flow_tail;


	/*** Process Queue ***/

	/* Now process the queue */
	while (flow_head != flow_tail)
	{
		/* Extract the next entry */
		ty = flow_y[flow_head];
		tx = flow_x[flow_head];

		/* Forget that entry (with wrap) */
		if (++flow_head == FLOW_MAX) flow_head = 0;

		/* Child cost */
		n = c->squares[ty][tx].cost + 1;

		/* Hack -- Limit flow depth */
		if (n == z_info->max_flow_depth) continue;

		/* Add the "children" */
		for (d = 0; d < 8; d++)
		{
			int old_head = flow_tail;

			/* Child location */
			y = ty + ddy_ddd[d];
			x = tx + ddx_ddd[d];
			if (!square_in_bounds(c, y, x)) continue;

			/* Ignore "pre-stamped" entries */
			if (c->squares[y][x].when == flow_n) continue;

			/* Ignore "walls" and "rubble" */
			if (tf_has(f_info[c->squares[y][x].feat].flags, TF_NO_FLOW))
				continue;

			/* Save the time-stamp */
			c->squares[y][x].when = flow_n;

			/* Save the flow cost */
			c->squares[y][x].cost = n;

			/* Enqueue that entry */
			flow_y[flow_tail] = y;
			flow_x[flow_tail] = x;

			/* Advance the queue */
			if (++flow_tail == FLOW_MAX) flow_tail = 0;

			/* Hack -- Overflow by forgetting new entry */
			if (flow_tail == flow_head) flow_tail = old_head;
		}
	}
}

/* Make map features known */
void cave_known (void)
{
	int y,x;
	for (y = 0; y < cave->height; y++)
		for (x = 0; x < cave->width; x++)
			cave_k->squares[y][x].feat = cave->squares[y][x].feat;
}
