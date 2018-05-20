/**
 * \file player-path.c
 * \brief Pathfinding and running code.
 *
 * Copyright (c) 1988 Christopher J Stuart (running code)
 * Copyright (c) 2004-2007 Christophe Cavalaria, Leon Marrick (pathfinding)
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
#include "cmds.h"
#include "init.h"
#include "mon-predicate.h"
#include "obj-ignore.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-path.h"
#include "player-timed.h"
#include "player-util.h"

/**
 * ------------------------------------------------------------------------
 * Pathfinding code
 * ------------------------------------------------------------------------ */

/**
 * Maximum size around the player to consider in the pathfinder
 */
#define MAX_PF_RADIUS 100

/**
 * Maximum distance to consider in the pathfinder
 */
#define MAX_PF_LENGTH 500


static int terrain[MAX_PF_RADIUS][MAX_PF_RADIUS];
static char pf_result[MAX_PF_LENGTH];
static int pf_result_index;

static int ox, oy, ex, ey;
static int dir_search[8] = {2,4,6,8,1,3,7,9};


static bool is_valid_pf(int y, int x)
{
	/* Unvisited means allowed */
	if (!square_isknown(cave, y, x)) return (true);

	/* No damaging terrain */
	if (!square_isdamaging(cave, y, x)) return (true);

	/* Require open space */
	return (square_ispassable(cave, y, x));
}

static void fill_terrain_info(void)
{
	int i, j;

	ox = MAX(player->px - MAX_PF_RADIUS / 2, 0);
	oy = MAX(player->py - MAX_PF_RADIUS / 2, 0);

	ex = MIN(player->px + MAX_PF_RADIUS / 2 - 1, cave->width);
	ey = MIN(player->py + MAX_PF_RADIUS / 2 - 1, cave->height);

	for (i = 0; i < MAX_PF_RADIUS; i++)
		for (j = 0; j < MAX_PF_RADIUS; j++)
			terrain[i][j] = -1;

	for (j = oy; j < ey; j++)
		for (i = ox; i < ex; i++)
			if (is_valid_pf(j, i))
				terrain[j - oy][i - ox] = MAX_PF_LENGTH;

	terrain[player->py - oy][player->px - ox] = 1;
}

#define MARK_DISTANCE(c,d) if ((c <= MAX_PF_LENGTH) && (c > d)) \
							{ c = d; try_again = (true); }

bool findpath(int y, int x)
{
	int i, j, k;
	int dir = 10;
	bool try_again;
	int cur_distance;

	fill_terrain_info();

	terrain[player->py - oy][player->px - ox] = 1;

	if ((x >= ox) && (x < ex) && (y >= oy) && (y < ey)) {
		if ((cave->squares[y][x].mon > 0) &&
			monster_is_visible(square_monster(cave, y, x))) {
			terrain[y - oy][x - ox] = MAX_PF_LENGTH;
		}
	} else {
		bell("Target out of range.");
		return (false);
	}

	/* 
	 * And now starts the very naive and very 
	 * inefficient pathfinding algorithm
	 */
	do {
		try_again = false;

		for (j = oy + 1; j < ey - 1; j++) {
			for (i = ox + 1; i < ex - 1; i++) {
				cur_distance = terrain[j - oy][i - ox] + 1;

				if ((cur_distance > 0) && (cur_distance < MAX_PF_LENGTH)) {
					for (dir = 1; dir < 10; dir++) {
						int next_y, next_x;
						if (dir == 5)
							dir++;
						next_y = j - oy + ddy[dir];
						next_x = i - ox + ddx[dir];

						MARK_DISTANCE(terrain[next_y][next_x], cur_distance);
					}
				}
			}
		}

		if (terrain[y - oy][x - ox] < MAX_PF_LENGTH)
			try_again = false;

	}
	while (try_again) ;

	/* Failure */
	if (terrain[y - oy][x - ox] == MAX_PF_LENGTH) {
		bell("Target space unreachable.");
		return (false);
	}

	/* Success */
	i = x;
	j = y;

	pf_result_index = 0;

	while ((i != player->px) || (j != player->py)) {
		cur_distance = terrain[j - oy][i - ox] - 1;
		for (k = 0; k < 8; k++) {
			dir = dir_search[k];
			if (terrain[j - oy + ddy[dir]][i - ox + ddx[dir]] == cur_distance)
				break;
		}

		/* Should never happen */
		assert ((dir != 10) && (dir != 5));

		pf_result[pf_result_index++] = '0' + (char)(10 - dir);
		i += ddx[dir];
		j += ddy[dir];
	}

	pf_result_index--;

	return (true);
}

/**
 * Compute the direction (in the angband 123456789 sense) from a point to a
 * point. We decide to use diagonals if dx and dy are within a factor of two of
 * each other; otherwise we choose a cardinal direction.
 */
int pathfind_direction_to(struct loc from, struct loc to)
{
	int adx = ABS(to.x - from.x);
	int ady = ABS(to.y - from.y);
	int dx = to.x - from.x;
	int dy = to.y - from.y;

	if (dx == 0 && dy == 0)
		return DIR_NONE;

	if (dx >= 0 && dy >= 0) {
		if (adx < ady * 2 && ady < adx * 2)
			return DIR_SE;
		else if (adx > ady)
			return DIR_E;
		else
			return DIR_S;
	} else if (dx > 0 && dy < 0) {
		if (adx < ady * 2 && ady < adx * 2)
			return DIR_NE;
		else if (adx > ady)
			return DIR_E;
		else
			return DIR_N;
	} else if (dx < 0 && dy > 0) {
		if (adx < ady * 2 && ady < adx * 2)
			return DIR_SW;
		else if (adx > ady)
			return DIR_W;
		else
			return DIR_S;
	} else if (dx <= 0 && dy <= 0) {
		if (adx < ady * 2 && ady < adx * 2)
			return DIR_NW;
		else if (adx > ady)
			return DIR_W;
		else
			return DIR_N;
	}

	assert(0);
	return DIR_UNKNOWN;
}

/**
 * ------------------------------------------------------------------------
 * Running code
 * ------------------------------------------------------------------------ */

/**
 * Basically, once you start running, you keep moving until something
 * interesting happens.  In an enclosed space, you run straight, but
 * you follow corners as needed (i.e. hallways).  In an open space,
 * you run straight, but you stop before entering an enclosed space
 * (i.e. a room with a doorway).  In a semi-open space (with walls on
 * one side only), you run straight, but you stop before entering an
 * enclosed space or an open space (i.e. running along side a wall).
 *
 * All discussions below refer to what the player can see, that is,
 * an unknown wall is just like a normal floor.  This means that we
 * must be careful when dealing with "illegal" grids.
 *
 * No assumptions are made about the layout of the dungeon, so this
 * algorithm works in hallways, rooms, town, destroyed areas, etc.
 *
 * In the diagrams below, the player has just arrived in the grid
 * marked as '@', and he has just come from a grid marked as 'o',
 * and he is about to enter the grid marked as 'x'.
 *
 * Running while confused is not allowed, and so running into a wall
 * is only possible when the wall is not seen by the player.  This
 * will take a turn and stop the running.
 *
 * Several conditions are tracked by the running variables.
 *
 *   run_open_area (in the open on at least one side)
 *   run_break_left (wall on the left, stop if it opens)
 *   run_break_right (wall on the right, stop if it opens)
 *
 * When running begins, these conditions are initialized by examining
 * the grids adjacent to the requested destination grid (marked 'x'),
 * two on each side (marked 'L' and 'R').  If either one of the two
 * grids on a given side is a wall, then that side is considered to
 * be "closed".  Both sides enclosed yields a hallway.
 *
 *    LL                     @L
 *    @x      (normal)       RxL   (diagonal)
 *    RR      (east)          R    (south-east)
 *
 * In the diagram below, in which the player is running east along a
 * hallway, he will stop as indicated before attempting to enter the
 * intersection (marked 'x').  Starting a new run in any direction
 * will begin a new hallway run.
 *
 * #.#
 * ##.##
 * o@x..
 * ##.##
 * #.#
 *
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * In the diagram below, the player is running east down a hallway,
 * and will stop in the grid (marked '1') before the intersection.
 * Continuing the run to the south-east would result in a long run
 * stopping at the end of the hallway (marked '2').
 *
 * ##################
 * o@x       1
 * ########### ######
 * #2          #
 * #############
 *
 * After each step, the surroundings are examined to determine if
 * the running should stop, and to determine if the running should
 * change direction.  We examine the new current player location
 * (at which the runner has just arrived) and the direction from
 * which the runner is considered to have come.
 *
 * Moving one grid in some direction places you adjacent to three
 * or five new grids (for straight and diagonal moves respectively)
 * to which you were not previously adjacent (marked as '!').
 *
 *   ...!              ...
 *   .o@!  (normal)    .o.!  (diagonal)
 *   ...!  (east)      ..@!  (south east)
 *                      !!!
 *
 * If any of the newly adjacent grids are "interesting" (monsters,
 * objects, some terrain features) then running stops.
 *
 * If any of the newly adjacent grids seem to be open, and you are
 * looking for a break on that side, then running stops.
 *
 * If any of the newly adjacent grids do not seem to be open, and
 * you are in an open area, and the non-open side was previously
 * entirely open, then running stops.
 *
 * If you are in a hallway, then the algorithm must determine if
 * the running should continue, turn, or stop.  If only one of the
 * newly adjacent grids appears to be open, then running continues
 * in that direction, turning if necessary.  If there are more than
 * two possible choices, then running stops.  If there are exactly
 * two possible choices, separated by a grid which does not seem
 * to be open, then running stops.  Otherwise, as shown below, the
 * player has probably reached a "corner".
 *
 *    ###             o##
 *    o@x  (normal)   #@!   (diagonal)
 *    ##!  (east)     ##x   (south east)
 *
 * In this situation, there will be two newly adjacent open grids,
 * one touching the player on a diagonal, and one directly adjacent.
 * We must consider the two "option" grids further out (marked '?').
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid.
 *
 *    ###s
 *    o@x?   (may be incorrect diagram!)
 *    ##!?
 *
 * If both "option" grids are closed, then there is no reason to enter
 * the corner, and so we can cut the corner, by moving into the other
 * grid (diagonally).  If we choose not to cut the corner, then we may
 * go straight, but we pretend that we got there by moving diagonally.
 * Below, we avoid the obvious grid (marked 'x') and cut the corner
 * instead (marked 'n').
 *
 *    ###:               o##
 *    o@x#   (normal)    #@n    (maybe?)
 *    ##n#   (east)      ##x#
 *                       ####
 *
 * If one of the "option" grids is open, then we may have a choice, so
 * we check to see whether it is a potential corner or an intersection
 * (or room entrance).  If the grid two spaces straight ahead, and the
 * space marked with 's' are both open, then it is a potential corner
 * and we enter it if requested.  Otherwise, we stop, because it is
 * not a corner, and is instead an intersection or a room entrance.
 *
 *    ###
 *    o@x
 *    ##!#
 *
 * I do not think this documentation is correct.
 */



static int run_cur_dir;		/* Direction we are running */
static int run_old_dir;		/* Direction we came from */
static bool run_open_area;		/* Looking for an open area */
static bool run_break_right;	/* Looking for a break (right) */
static bool run_break_left;	/* Looking for a break (left) */

/**
 * Hack -- allow quick "cycling" through the legal directions
 */
static const byte cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };


/**
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static const byte chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };


/**
 * Hack -- Check for a "known wall" (see below)
 */
static int see_wall(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are not known walls XXX XXX XXX */
	if (!square_in_bounds(cave, y, x)) return (false);

	/* Non-wall grids are not known walls */
	if (!square_seemslikewall(cave, y, x))
		return false;

	/* Unknown walls are not known walls */
	if (!square_isknown(cave, y, x)) return (false);

	/* Default */
	return (true);
}


/**
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. (?)
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(int dir)
{
	int py = player->py;
	int px = player->px;

	int i, row, col;

	bool deepleft, deepright;
	bool shortleft, shortright;

	/* Mark that we're starting a run */
	player->upkeep->running_firststep = true;

	/* Save the direction */
	run_cur_dir = dir;

	/* Assume running straight */
	run_old_dir = dir;

	/* Assume looking for open area */
	run_open_area = true;

	/* Assume not looking for breaks */
	run_break_right = false;
	run_break_left = false;

	/* Assume no nearby walls */
	deepleft = deepright = false;
	shortright = shortleft = false;

	/* Find the destination grid */
	row = py + ddy[dir];
	col = px + ddx[dir];

	/* Extract cycle index */
	i = chome[dir];

	/* Check for nearby or distant wall */
	if (see_wall(cycle[i+1], py, px)) {
		run_break_left = true;
		shortleft = true;
	} else if (see_wall(cycle[i+1], row, col)) {
		run_break_left = true;
		deepleft = true;
	}

	/* Check for nearby or distant wall */
	if (see_wall(cycle[i-1], py, px)) {
		run_break_right = true;
		shortright = true;
	} else if (see_wall(cycle[i-1], row, col)) {
		run_break_right = true;
		deepright = true;
	}

	/* Looking for a break */
	if (run_break_left && run_break_right) {
		/* Not looking for open area */
		run_open_area = false;

		/* Angled or blunt corridor entry */
		if (dir & 0x01) {
			if (deepleft && !deepright)
				run_old_dir = cycle[i - 1];
			else if (deepright && !deepleft)
				run_old_dir = cycle[i + 1];
		} else if (see_wall(cycle[i], row, col)) {
			if (shortleft && !shortright)
				run_old_dir = cycle[i - 2];
			else if (shortright && !shortleft)
				run_old_dir = cycle[i + 2];
		}
	}
}


/**
 * Update the current "run" path
 *
 * Return true if the running should be stopped
 */
static bool run_test(void)
{
	int py = player->py;
	int px = player->px;

	int prev_dir;
	int new_dir;

	int row, col;
	int i, max, inv;
	int option, option2;


	/* No options yet */
	option = 0;
	option2 = 0;

	/* Where we came from */
	prev_dir = run_old_dir;


	/* Range of newly adjacent grids */
	max = (prev_dir & 0x01) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++) {
		struct object *obj;

		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];

		/* New location */
		row = py + ddy[new_dir];
		col = px + ddx[new_dir];


		/* Visible monsters abort running */
		if (cave->squares[row][col].mon > 0) {
			struct monster *mon = square_monster(cave, row, col);
			if (monster_is_visible(mon)) {
				return (true);
			}
		}

		/* Visible objects abort running */
		for (obj = square_object(cave, row, col); obj; obj = obj->next)
			/* Visible object */
			if (obj->known && !ignore_item_ok(obj)) return (true);

		/* Assume unknown */
		inv = true;

		/* Check memorized grids */
		if (square_isknown(cave, row, col)) {
			bool notice = square_isinteresting(cave, row, col);

			/* Interesting feature */
			if (notice) return (true);

			/* The grid is "visible" */
			inv = false;
		}

		/* Analyze unknown grids and floors */
		if (inv || square_ispassable(cave, row, col)) {
			/* Looking for open area */
			if (run_open_area) {
				/* Nothing */
			} else if (!option) {
				/* The first new direction. */
				option = new_dir;
			} else if (option2) {
				/* Three new directions. Stop running. */
				return (true);
			} else if (option != cycle[chome[prev_dir] + i - 1]) {
				/* Two non-adjacent new directions.  Stop running. */
				return (true);
			} else if (new_dir & 0x01) {
				/* Two new (adjacent) directions (case 1) */
				option2 = new_dir;
			} else {
				/* Two new (adjacent) directions (case 2) */
				option2 = option;
				option = new_dir;
			}
		} else { /* Obstacle, while looking for open area */
			if (run_open_area) {
				if (i < 0) {
					/* Break to the right */
					run_break_right = true;
				} else if (i > 0) {
					/* Break to the left */
					run_break_left = true;
				}
			}
		}
	}


	/* Look at every soon to be newly adjacent square. */
	for (i = -max; i <= max; i++) {		
		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];
		
		/* New location */
		row = py + ddy[prev_dir] + ddy[new_dir];
		col = px + ddx[prev_dir] + ddx[new_dir];
		
		/* HACK: Ugh. Sometimes we come up with illegal bounds. This will
		 * treat the symptom but not the disease. */
		if (row >= cave->height || col >= cave->width) continue;
		if (row < 0 || col < 0) continue;

		/* Obvious monsters abort running */
		if (cave->squares[row][col].mon > 0) {
			struct monster *mon = square_monster(cave, row, col);
			if (monster_is_obvious(mon))
				return (true);
		}
	}

	/* Looking for open area */
	if (run_open_area) {
		/* Hack -- look again */
		for (i = -max; i < 0; i++) {
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Unknown grid or non-wall */
			/* Was: square_ispassable(cave, row, col) */
			if (!square_isknown(cave, row, col) ||
			    (square_ispassable(cave, row, col))) {
				/* Looking to break right */
				if (run_break_right) {
					return (true);
				}
			} else { /* Obstacle */
				/* Looking to break left */
				if (run_break_left) {
					return (true);
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--) {
			new_dir = cycle[chome[prev_dir] + i];

			row = py + ddy[new_dir];
			col = px + ddx[new_dir];

			/* Unknown grid or non-wall */
			/* Was: square_ispassable(cave, row, col) */
			if (!square_isknown(cave, row, col) ||
			    (square_ispassable(cave, row, col))) {
				/* Looking to break left */
				if (run_break_left) {
					return (true);
				}
			} else { /* Obstacle */
				/* Looking to break right */
				if (run_break_right) {
					return (true);
				}
			}
		}
	} else { /* Not looking for open area */
		/* No options */
		if (!option) {
			return (true);
		} else if (!option2) { /* One option */
			/* Primary option */
			run_cur_dir = option;

			/* No other options */
			run_old_dir = option;
		} else { /* Two options, examining corners */
			/* Primary option */
			run_cur_dir = option;

			/* Hack -- allow curving */
			run_old_dir = option2;
		}
	}


	/* About to hit a known wall, stop */
	if (see_wall(run_cur_dir, py, px))
		return (true);


	/* Failure */
	return (false);
}



/**
 * Take one step along the current "run" path
 *
 * Called with a real direction to begin a new run, and with zero
 * to continue a run in progress.
 */
void run_step(int dir)
{
	/* Trapsafe player will treat the trap as if it isn't there */
	bool disarm = player_is_trapsafe(player) ? false : true;

	/* Start or continue run */
	if (dir) {
		/* Initialize */
		run_init(dir);

		/* Hack -- Set the run counter if no count given */
		if (player->upkeep->running == 0)
			player->upkeep->running = 9999;

		/* Calculate torch radius */
		player->upkeep->update |= (PU_TORCH);
	} else {
		/* Continue running */
		if (!player->upkeep->running_withpathfind) {
			/* Update regular running */
			if (run_test()) {
				/* Disturb */
				disturb(player, 0);
				return;
			}
		} else if (pf_result_index < 0) {
			/* Pathfinding, and the path is finished */
			disturb(player, 0);
			player->upkeep->running_withpathfind = false;
			return;
		} else {
			int y = player->py + ddy[pf_result[pf_result_index] - '0'];
			int x = player->px + ddx[pf_result[pf_result_index] - '0'];

			if (pf_result_index == 0) {
				/* Known wall */
				if (square_isknown(cave, y, x) &&
					!square_ispassable(cave, y, x)) {
					disturb(player, 0);
					player->upkeep->running_withpathfind = false;
					return;
				}
			} else if (pf_result_index > 0) {
				struct object *obj;

				/* If the player has computed a path that is going to end up
				 * in a wall, we notice this and convert to a normal run. This
				 * allows us to click on unknown areas to explore the map.
				 *
				 * We have to look ahead two, otherwise we don't know which is
				 * the last direction moved and don't initialise the run
				 * properly. */
				y = player->py + ddy[pf_result[pf_result_index] - '0'];
				x = player->px + ddx[pf_result[pf_result_index] - '0'];

				/* Known wall */
				if (square_isknown(cave, y, x) &&
					!square_ispassable(cave, y, x)) {
					disturb(player, 0);
					player->upkeep->running_withpathfind = false;
					return;
				}

				/* Visible monsters abort running */
				if (cave->squares[y][x].mon > 0) {
					struct monster *mon = square_monster(cave, y, x);

					/* Visible monster */
					if (monster_is_visible(mon)) {
						disturb(player, 0);
						player->upkeep->running_withpathfind = false;
						return;
					}
				}

				/* Visible objects abort running */
				for (obj = square_object(cave, y, x); obj; obj = obj->next)
					/* Visible object */
					if (obj->known && !ignore_item_ok(obj)) {
					disturb(player, 0);
					player->upkeep->running_withpathfind = false;
					return;
				}

				/* Get step after */
				y = y + ddy[pf_result[pf_result_index - 1] - '0'];
				x = x + ddx[pf_result[pf_result_index - 1] - '0'];

				/* Known wall, so run the direction we were going */
				if (square_isknown(cave, y, x) &&
					!square_ispassable(cave, y, x)) {
					player->upkeep->running_withpathfind = false;
					run_init(pf_result[pf_result_index] - '0');
				}
			}

			/* Now actually run the step if we're still going */
			run_cur_dir = pf_result[pf_result_index--] - '0';
		}
	}

	/* Take time */
	player->upkeep->energy_use = z_info->move_energy / player->state.num_moves;

	/* Move the player; running straight into a trap == trying to disarm */
	move_player(run_cur_dir, dir && disarm ? true : false);

	/* Decrease counter if it hasn't been cancelled */
	/* occurs after movement so that using p->u->running as flag works */
	if (player->upkeep->running) {
		player->upkeep->running--;
	} else if (!player->upkeep->running_withpathfind)
		return;

	/* Prepare the next step */
	if (player->upkeep->running) {
		cmdq_push(CMD_RUN);
		cmd_set_arg_direction(cmdq_peek(), "direction", 0);
	}
}

