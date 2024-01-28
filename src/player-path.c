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
#include "generate.h"
#include "init.h"
#include "mon-predicate.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-path.h"
#include "player-timed.h"
#include "player-util.h"
#include "trap.h"

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

/**
 * Scale factor for distances in the path_distance array; note that PF_SCL *
 * MAX_PF_LENGTH has to fit in an int
 */
#define PF_SCL 16

/**
 * Bounds of the search rectangle
 */
static struct loc top_left, bottom_right;

/**
 * Array of distances, inflated by PF_SCL, from the player
 */
static int path_distance[MAX_PF_RADIUS][MAX_PF_RADIUS];

/**
 * Pathfinding results
 */
static int path_step_dir[MAX_PF_LENGTH];
static int path_step_idx;

/**
 * Remembered destination for those results.
 */
static struct loc path_dest;

/**
 * Determine whether a grid is OK for the pathfinder to check
 */
static bool is_valid_pf(struct loc grid, bool prefer_known, bool allow_traps)
{
	/*
	 * If prefer_known is true, do not want to consider unvisited squares;
	 * otherwise, they are acceptable.
	 */
	if (!square_isknown(cave, grid)) return !prefer_known;

	/*
	 * For all remaining tests, use the player's memory of what's there
	 * to avoid leaking information if the terrain has changed.
	 */
	/* No damaging terrain */
	if (square_isdamaging(player->cave, grid)) return false;

	/* No trapped squares unless trapsafe or allowing traps */
	if (square_isvisibletrap(player->cave, grid)
			&& !(allow_traps || player_is_trapsafe(player))) {
		return false;
	}

	/* All remaining passable terrain is okay. */
	if (square_ispassable(player->cave, grid)) {
		return true;
	}

	/*
	 * Some impassable terrain can be traversed fairly easily by modifying
	 * the terrain so allow those kinds.
	 */
	if (square_iscloseddoor(player->cave, grid)
			|| square_isrubble(player->cave, grid)) {
		return true;
	}

	/* Reject all other impassable terrain. */
	return false;
}

/**
 * Get pathfinding region
 */
static void get_pathfind_region(void)
{
	top_left.x = MAX(player->grid.x - MAX_PF_RADIUS / 2, 0);
	top_left.y = MAX(player->grid.y - MAX_PF_RADIUS / 2, 0);

	bottom_right.x = MIN(player->grid.x + MAX_PF_RADIUS / 2 - 1, cave->width);
	bottom_right.y = MIN(player->grid.y + MAX_PF_RADIUS / 2 - 1, cave->height);
}

/**
 * Get the path distance info for a grid
 */
static int path_dist(struct loc grid)
{
	return path_distance[grid.y - top_left.y][grid.x - top_left.x];
}

/**
 * Set the path distance info for a grid
 */
static void set_path_dist(struct loc grid, int dist)
{
	assert(grid.y >= top_left.y);
	assert(grid.x >= top_left.x);
	assert(grid.y < bottom_right.y);
	assert(grid.x < bottom_right.x);
	path_distance[grid.y - top_left.y][grid.x - top_left.x] = dist;
}

/**
 * Initialize terrain information
 */
static void path_dist_info_init(bool prefer_known, bool allow_traps)
{
	struct loc grid;

	/* Set distance of all grids in the region to -1 */
	for (grid.y = 0; grid.y < MAX_PF_RADIUS; grid.y++)
		for (grid.x = 0; grid.x < MAX_PF_RADIUS; grid.x++)
			path_distance[grid.y][grid.x] = -1;

	/* Set distance of valid grids to MAX_PF_LENGTH * PF_SCL */
	for (grid.y = top_left.y; grid.y < bottom_right.y; grid.y++)
		for (grid.x = top_left.x; grid.x < bottom_right.x; grid.x++)
			if (is_valid_pf(grid, prefer_known, allow_traps))
				set_path_dist(grid, MAX_PF_LENGTH * PF_SCL);

	/* Set distance of the player's grid to 0 */
	set_path_dist(player->grid, 0);
}

/**
 * Help compute_unlocked_penalty(), compute_locked_penalty(), and
 * compute_rubble_penalty():  convert a penalty in standard turns to one in
 * movement turns.
 */
static int convert_turn_penalty(int penalty, struct player *p)
{
	/*
	 * If a turn moving takes a different amount of energy than a turn
	 * doing something other than moving, adjust the penalty.
	 */
	int moving_energy = energy_per_move(p);

	if (moving_energy != z_info->move_energy) {
		if (moving_energy > 0) {
			struct my_rational factor = my_rational_construct(
				z_info->move_energy, moving_energy);
			unsigned int adjusted, remainder;

			adjusted = my_rational_to_uint(&factor, penalty,
				&remainder);
			if (adjusted < MAX_PF_LENGTH * PF_SCL) {
				penalty = (int)adjusted;
				/* Round to the nearest integer. */
				if (remainder >= (factor.d + 1) / 2) {
					++penalty;
				}
			} else {
				penalty = MAX_PF_LENGTH * PF_SCL;
			}
		} else {
			penalty = MAX_PF_LENGTH * PF_SCL;
		}
	}
	return penalty;
}

/**
 * Help set_up_path_distances():  compute the penalty to distance for going
 * through a known closed door that is unlocked.
 */
static int compute_unlocked_penalty(struct player *p)
{
	/* An unlocked door takes one turn to open. */
	return convert_turn_penalty(PF_SCL, p);
}

/**
 * Help set_up_path_distances():  compute the penalty to distance for going
 * through a known closed door that is locked.
 *
 * Treats the power of the lock as not readily known to the player and assumes
 * the maximum power currently used, 7.  See the calls to square_set_door_lock()
 * in gen-util.c's place_closed_door() and cmd-cave.c's do_cmd_lock_door().
 * Approximates the lighting state when unlocking the door with the current
 * lighting state.
 */
static int compute_locked_penalty(struct player *p)
{
	int chance = calc_unlocking_chance(p, 7, p->state.cur_light < 1
		&& !player_has(p, PF_UNLIGHT));
	int penalty;

	if (chance <= 0) {
		/* It can not be unlocked. */
		penalty = MAX_PF_LENGTH * PF_SCL;
	} else if (chance >= 100) {
		/* It takes one turn to unlock and open. */
		penalty = PF_SCL;
	} else {
		/*
		 * On average, it takes 100 / chance turns to unlock and open.
		 * Round the result to the nearest integer after scaling by
		 * PF_SCL.
		 */
		struct my_rational avg_turns =
			my_rational_construct(100, chance);
		unsigned int scl_turns, remainder;

		scl_turns = my_rational_to_uint(&avg_turns, PF_SCL, &remainder);
		if (scl_turns < MAX_PF_LENGTH * PF_SCL) {
			penalty = (int)scl_turns;
			if (remainder >= (avg_turns.d + 1) / 2) {
				++penalty;
			}
		} else {
			penalty = MAX_PF_LENGTH * PF_SCL;
		}
	}

	return convert_turn_penalty(penalty, p);
}

/**
 * Help set_up_path_distances():  compute the penalty to distance for going
 * through known impassable rubble.
 *
 * Duplicates logic from do_cmd_tunnel_aux().
 */
static int compute_rubble_penalty(struct player *p)
{
	int weapon_slot = slot_by_name(p, "weapon");
	struct object *current_weapon = slot_object(p, weapon_slot);
	struct object *best_digger = player_best_digger(p, false);
	struct player_state local_state, *used_state;
	int digging_chances[DIGGING_MAX], num_digger = 1;
	bool swapped_digger;
	int penalty;

	if (best_digger != current_weapon && (!current_weapon
			|| obj_can_takeoff(current_weapon))) {
		swapped_digger = true;
		if (best_digger) {
			num_digger = best_digger->number;
			best_digger->number = 1;
		}
		p->body.slots[weapon_slot].obj = best_digger;
		memcpy(&local_state, &p->state, sizeof(local_state));
		calc_bonuses(p, &local_state, false, true);
		used_state = &local_state;
	} else {
		swapped_digger = false;
		used_state = &p->state;
	}
	calc_digging_chances(used_state, digging_chances);
	if (swapped_digger) {
		if (best_digger) {
			best_digger->number = num_digger;
		}
		p->body.slots[weapon_slot].obj = current_weapon;
		calc_bonuses(p, &local_state, false, true);
	}
	if (digging_chances[DIGGING_RUBBLE] <= 0) {
		/* Can not dig through rubble at all. */
		penalty = MAX_PF_LENGTH * PF_SCL;
	} else if (digging_chances[DIGGING_RUBBLE] >= 1600) {
		/* Takes one turn to dig through rubble. */
		penalty = PF_SCL;
	} else {
		/*
		 * On average, it takes 1600 / digging_chances[DIGGING_RUBBLE]
		 * turns to dig through rubble.  Round the result to the nearest
		 * integer after scaling by PF_SCL.
		 */
		struct my_rational avg_turns = my_rational_construct(1600,
			digging_chances[DIGGING_RUBBLE]);
		unsigned int scl_turns, remainder;

		scl_turns = my_rational_to_uint(&avg_turns, PF_SCL,
			&remainder);
		if (scl_turns < MAX_PF_LENGTH * PF_SCL) {
			penalty = (int)scl_turns;
			if (remainder >= (avg_turns.d + 1) / 2) {
				++penalty;
			}
		} else {
			penalty = MAX_PF_LENGTH * PF_SCL;
		}
	}

	return convert_turn_penalty(penalty, p);
}

/**
 * Try to find a path from the player's grid
 * \param grid the target grid
 */
static bool set_up_path_distances(struct loc grid)
{
	struct point_set *reached;
	/*
	 * If the destination is known, first try finding a path only through
	 * known squares.  If that fails, then try a path that can pass both
	 * through known and unknown squares.
	 */
	bool prefer_known = square_isknown(cave, grid);
	/*
	 * Prefer finding paths with no known visible traps.  If no such path
	 * is found, then try with paths that can go through known visible
	 * traps.
	 */
	bool allow_traps = false;
	bool found;
	/*
	 * Are there any known visible traps in the pathfinding area?
	 * Negative means the answer has not been calculated yet, zero means
	 * no, and positive means yes.
	 */
	int has_traps = -1;
	int unlocked_penalty, locked_penalty, rubble_penalty;

	/* Initialize the pathfinding region */
	get_pathfind_region();
	if (grid.x < top_left.x || grid.x >= bottom_right.x
			|| grid.y < top_left.y || grid.y >= bottom_right.y) {
		bell();
		return false;
	}
	path_dist_info_init(prefer_known, allow_traps);

	/* Precompute quantities used to penalize traversing some terrain. */
	unlocked_penalty = compute_unlocked_penalty(player);
	locked_penalty = compute_locked_penalty(player);
	rubble_penalty = compute_rubble_penalty(player);

	reached = point_set_new(MAX_PF_RADIUS * MAX_PF_RADIUS);

	while (1) {
		int i;

		/* Add the player's grid to the set of marked grids */
		add_to_point_set(reached, player->grid);

		/* Add the neighbours of any marked grid in the area */
		for (i = 0; i < reached->n; i++) {
			int k, cur_distance = path_dist(reached->pts[i]);

			/*
			 * Move one grid, i.e. PF_SCL, to get to the next grid.
			 */
			if (cur_distance >= (MAX_PF_LENGTH - 1) * PF_SCL) {
				/*
				 * This path will exceed the maximum so skip
				 * further processing of it.
				 */
				continue;
			}
			cur_distance += PF_SCL;

			for (k = 0; k < 8; k++) {
				struct loc next =
					loc_sum(reached->pts[i], ddgrid_ddd[k]);

				/*
				 * Enforce area bounds and skip paths that are
				 * not shorter than ones already found to next.
				 */
				if ((next.y < top_left.y)
						|| (next.y >= bottom_right.y)
						|| (next.x < top_left.x)
						|| (next.x >= bottom_right.x)
						|| (path_dist(next)
						<= cur_distance)) {
					continue;
				}

				/*
				 * Add the grid; penalize some terrain if it is
				 * known and hard to traverse.
				 */
				if (!square_isknown(cave, next)
						|| square_ispassable(
						player->cave, next)) {
					set_path_dist(next, cur_distance);
				} else {
					int penalty, penalized_distance;

					if (square_iscloseddoor(player->cave,
							next)) {
						penalty = (square_islockeddoor(
							player->cave, next))
							? locked_penalty :
							unlocked_penalty;
					} else if (square_isrubble(player->cave,
							next)) {
						penalty = rubble_penalty;
					} else {
						/*
						 * Should not happen; treat it
						 * as completely impassable.
						 */
						continue;
					}

					if (cur_distance >= MAX_PF_LENGTH
							* PF_SCL - penalty) {
						/*
						 * Will exceed the maximum
						 * allowed distance so skip
						 * further processing.
						 */
						continue;
					}
					penalized_distance =
						cur_distance + penalty;
					if (path_dist(next) <= penalized_distance) {
						/*
						 * Already have a shorter path
						 * there.  Do not need to
						 * consider the longer one.
						 */
						continue;
					}
					set_path_dist(next, penalized_distance);
				}
				add_to_point_set(reached, next);
			}
		}

		/* Failed to find a path. */
		if (path_dist(grid) < 0
				|| path_dist(grid) >= MAX_PF_LENGTH * PF_SCL) {
			if ((square(cave, grid)->mon > 0) &&
					monster_is_visible(
					square_monster(cave, grid))) {
				/*
				 * If the target grid has a known monster,
				 * allow if there is a path to any of the
				 * neighbors.  That means clicking on a
				 * monster (triggering pathfinding) to
				 * attack it will work when the monster is in
				 * terrain that the pathfinding would not
				 * normally traverse.
				 */
				int k;

				for (k = 0; k < 8; ++k) {
					struct loc check_grid =
						loc_sum(grid, ddgrid_ddd[k]);
					int check_dist;

					if (check_grid.y < top_left.y
							|| check_grid.y >=
							bottom_right.y
							|| check_grid.x <
							top_left.x
							|| check_grid.x >=
							bottom_right.x) {
						continue;
					}
					check_dist = path_dist(check_grid);
					if (check_dist >= 0
							&& check_dist <
							MAX_PF_LENGTH * PF_SCL) {
						break;
					}
				}
				if (k < 8) {
					found = true;
					break;
				}
			}

			/*
			 * If forbid known visible traps from the path,
			 * try again allowing such traps if it could make a
			 * difference.
			 */
			if (!allow_traps && !player_is_trapsafe(player)) {
				if (has_traps < 0) {
					struct loc trap_grid;

					has_traps = (cave_find_in_range(
						player->cave, &trap_grid,
						top_left, loc(bottom_right.x -1,
						bottom_right.y - 1),
						square_isvisibletrap)) ? 1 : 0;
				}
				if (has_traps > 0) {
					allow_traps = true;
					path_dist_info_init(prefer_known, true);
					reached->n = 0;
					continue;
				}
			}

			/*
			 * If only considered known squares for the path,
			 * retry with paths that allow known and unknown
			 * squares.
			 */
			if (prefer_known) {
				prefer_known = false;
				allow_traps = false;
				path_dist_info_init(false, false);
				reached->n = 0;
				continue;
			}

			/* Otherwise, report the failure. */
			bell();
			found = false;
			break;
		}

		/* Report the successful path. */
		found = true;
		break;
	}

	point_set_dispose(reached);

	return found;
}

/**
 * Fill the array of path step directions
 * \param grid the target grid
 */
bool find_path(struct loc grid)
{
	struct loc new = grid;

	/* Attempt to find a path if necessary */
	if (loc_eq(new, player->grid)) return false;
	if (!set_up_path_distances(grid)) return false;

	/* Now travel along the path, backwards */
	path_step_idx = 0;
	while (!loc_eq(new, player->grid)) {
		int k, best_k = -1, best_distance = path_dist(new);

		/* Find the next step */
		for (k = 0; k < 8; k++) {
			struct loc next = loc_sum(new, ddgrid_ddd[k]);
			int try_distance;

			if (next.y < top_left.y || next.y >= bottom_right.y
					|| next.x < top_left.x
					|| next.x >= bottom_right.x) {
				continue;
			}
			try_distance = path_dist(next);
			if (try_distance >= 0 && best_distance > try_distance) {
				best_distance = try_distance;
				best_k = k;
			}
		}

		/*
		 * Bail out if stepping backward did not improve the distance.
		 */
		if (best_k < 0) return false;

		/* Check we haven't exceeded path length */
		if (path_step_idx >= MAX_PF_LENGTH) return false;

		/* Record the opposite of the backward direction */
		path_step_dir[path_step_idx++] = 10 - ddd[best_k];
		new = loc_sum(new, ddgrid_ddd[best_k]);
	}

	/* Reduce to the actual number of steps */
	path_step_idx--;
	assert(path_step_idx >= 0);

	/* Remember the endpoint. */
	path_dest = grid;

	return true;
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
		if (adx < ady * 2 && ady < adx * 2)	return DIR_SE;
		else if (adx > ady) return DIR_E;
		else return DIR_S;
	} else if (dx > 0 && dy < 0) {
		if (adx < ady * 2 && ady < adx * 2)	return DIR_NE;
		else if (adx > ady)	return DIR_E;
		else return DIR_N;
	} else if (dx < 0 && dy > 0) {
		if (adx < ady * 2 && ady < adx * 2)	return DIR_SW;
		else if (adx > ady)	return DIR_W;
		else return DIR_S;
	} else if (dx <= 0 && dy <= 0) {
		if (adx < ady * 2 && ady < adx * 2)	return DIR_NW;
		else if (adx > ady)	return DIR_W;
		else return DIR_N;
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
static const uint8_t cycle[] =
{ 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };


/**
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static const uint8_t chome[] =
{ 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };


/**
 * Hack -- Check for a "known wall" (see below)
 */
static bool see_wall(int dir, struct loc grid)
{
	/* Get the new location */
	grid = loc_sum(grid, ddgrid[dir]);

	/* Illegal grids are not known walls XXX XXX XXX */
	if (!square_in_bounds(cave, grid)) return false;

	/* Webs are enough like walls */
	if (square_iswebbed(cave, grid)) return true;

	/* Non-wall grids are not known walls */
	if (!square_seemslikewall(cave, grid)) return false;

	/* Unknown walls are not known walls */
	if (!square_isknown(cave, grid)) return false;

	/* Default */
	return true;
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
	int i;
	struct loc grid;

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
	grid = loc_sum(player->grid, ddgrid[dir]);

	/* Extract cycle index */
	i = chome[dir];

	/* Check for nearby or distant wall */
	if (see_wall(cycle[i + 1], player->grid)) {
		/* Wall diagonally left of player's current grid */
		run_break_left = true;
		shortleft = true;
	} else if (see_wall(cycle[i + 1], grid)) {
		/* Wall diagonally left of the grid the player is stepping to */
		run_break_left = true;
		deepleft = true;
	}

	/* Check for nearby or distant wall */
	if (see_wall(cycle[i - 1], player->grid)) {
		/* Wall diagonally right of player's current grid */
		run_break_right = true;
		shortright = true;
	} else if (see_wall(cycle[i - 1], grid)) {
		/* Wall diagonally right of the grid the player is stepping to */
		run_break_right = true;
		deepright = true;
	}

	/* Looking for a break */
	if (run_break_left && run_break_right) {
		/* Not looking for open area */
		run_open_area = false;

		/* Check angled or blunt corridor entry for diagonal directions */
		if (dir & 0x01) {
			if (deepleft && !deepright)
				run_old_dir = cycle[i - 1];
			else if (deepright && !deepleft)
				run_old_dir = cycle[i + 1];
		} else if (see_wall(cycle[i], player->grid)) {
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
static bool run_test(const struct player *p)
{
	int prev_dir;
	int new_dir;

	struct loc grid;
	int i, max, inv;
	int option, option2;

	/* No options yet */
	option = 0;
	option2 = 0;

	/* Where we came from */
	prev_dir = run_old_dir;

	/* Range of newly adjacent grids - 5 for diagonals, 3 for cardinals */
	max = (prev_dir & 0x01) + 1;

	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++) {
		struct object *obj;

		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];

		/* New location */
		grid = loc_sum(p->grid, ddgrid[new_dir]);

		/* Visible monsters abort running */
		if (square(cave, grid)->mon > 0) {
			struct monster *mon = square_monster(cave, grid);
			if (monster_is_visible(mon)) {
				return true;
			}
		}

		/* Visible traps abort running (unless trapsafe) */
		if (square_isvisibletrap(cave, grid) &&
			!player_is_trapsafe(p)) {
			return true;
		}

		/* Visible objects abort running */
		for (obj = square_object(cave, grid); obj; obj = obj->next)
			/* Visible object */
			if (obj->known && !ignore_item_ok(p, obj)) return true;

		/* Assume unknown */
		inv = true;

		/* Check memorized grids */
		if (square_isknown(cave, grid)) {
			bool notice = square_isinteresting(p->cave, grid);

			/* Interesting feature */
			if (notice) return true;

			/* The grid is "visible" */
			inv = false;
		}

		/* Analyze unknown grids and floors */
		if (inv || square_ispassable(cave, grid)) {
			/* Looking for open area */
			if (run_open_area) {
				/* Nothing */
			} else if (!option) {
				/* The first new direction. */
				option = new_dir;
			} else if (option2) {
				/* Three new directions. Stop running. */
				return true;
			} else if (option != cycle[chome[prev_dir] + i - 1]) {
				/* Two non-adjacent new directions.  Stop running. */
				return true;
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
		grid = loc_sum(p->grid, loc_sum(ddgrid[prev_dir], ddgrid[new_dir]));
		
		/* HACK: Ugh. Sometimes we come up with illegal bounds. This will
		 * treat the symptom but not the disease. */
		if (!square_in_bounds(cave, grid)) continue;

		/* Obvious monsters abort running */
		if (square(cave, grid)->mon > 0) {
			struct monster *mon = square_monster(cave, grid);
			if (monster_is_obvious(mon))
				return true;
		}
	}

	/* Looking for open area */
	if (run_open_area) {
		/* Hack -- look again */
		for (i = -max; i < 0; i++) {
			new_dir = cycle[chome[prev_dir] + i];
			grid = loc_sum(p->grid, ddgrid[new_dir]);

			/* Unknown grid or non-wall */
			if (!square_isknown(cave, grid) || square_ispassable(cave, grid)) {
				/* Looking to break right */
				if (run_break_right) {
					return true;
				}
			} else { /* Obstacle */
				/* Looking to break left */
				if (run_break_left) {
					return true;
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--) {
			new_dir = cycle[chome[prev_dir] + i];
			grid = loc_sum(p->grid, ddgrid[new_dir]);

			/* Unknown grid or non-wall */
			if (!square_isknown(cave, grid) || square_ispassable(cave, grid)) {
				/* Looking to break left */
				if (run_break_left) {
					return true;
				}
			} else { /* Obstacle */
				/* Looking to break right */
				if (run_break_right) {
					return true;
				}
			}
		}
	} else { /* Not looking for open area */
		/* No options */
		if (!option) {
			return true;
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
		if (see_wall(run_cur_dir, p->grid))
		return true;

	/* Failure */
	return false;
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
			if (run_test(player)) {
				/* Disturb */
				disturb(player);
				return;
			}
		} else if (path_step_idx < 0) {
			/* Pathfinding, and the path is finished */
			disturb(player);
			player->upkeep->running_withpathfind = false;
			return;
		} else {
			struct loc grid = loc_sum(player->grid,
				ddgrid[path_step_dir[path_step_idx]]);

			/*
			 * Automatically deal with some impassable
			 * terrain if that grid and its immediate neighors are
			 * known.  Since disturb() flushes queued commands,
			 * first stop running before pushing the commands to
			 * deal with the terrain and restart pathfinding.
			 */
			if (square_iscloseddoor(player->cave, grid)) {
				if (count_neighbors(NULL, cave, grid,
						square_isnotknown, true) == 0) {
					disturb(player);
					player->upkeep->running_withpathfind = false;
					cmdq_push(CMD_OPEN);
					cmd_set_arg_direction(cmdq_peek(),
						"direction",
						path_step_dir[path_step_idx]);
					cmdq_push(CMD_PATHFIND);
					cmd_set_arg_point(cmdq_peek(),
						"point",
						path_dest);
					return;
				}
			} else if (square_isrubble(player->cave, grid)
					&& !square_ispassable(player->cave,
					grid)) {
				if (count_neighbors(NULL, cave, grid,
						square_isnotknown, true) == 0) {
					disturb(player);
					player->upkeep->running_withpathfind = false;
					cmdq_push(CMD_TUNNEL);
					cmd_set_arg_direction(cmdq_peek(),
						"direction",
						path_step_dir[path_step_idx]);
					cmdq_push(CMD_PATHFIND);
					cmd_set_arg_point(cmdq_peek(),
						"point",
						path_dest);
					return;
				}
			}

			if (path_step_idx == 0) {
				/*
				 * Known impassable terrain that is not
				 * automatically handled.
				 */
				if (square_isknown(cave, grid) &&
						!square_ispassable(
						player->cave, grid)) {
					disturb(player);
					player->upkeep->running_withpathfind = false;
					return;
				}
			} else if (path_step_idx > 0) {
				struct object *obj;

				/* If the player has computed a path that is going to end up
				 * in a wall, we notice this and convert to a normal run. This
				 * allows us to click on unknown areas to explore the map.
				 *
				 * We have to look ahead two, otherwise we don't know which is
				 * the last direction moved and don't initialise the run
				 * properly. */

				/*
				 * Known impassable terrain that is not
				 * automatically handled
				 */
				if (square_isknown(cave, grid)
						&& !square_ispassable(
						player->cave, grid)) {
					disturb(player);
					player->upkeep->running_withpathfind = false;
					return;
				}

				/* Visible monsters abort running */
				if (square(cave, grid)->mon > 0) {
					struct monster *mon = square_monster(cave, grid);

					/* Visible monster */
					if (monster_is_visible(mon)) {
						disturb(player);
						player->upkeep->running_withpathfind = false;
						return;
					}
				}

				/* Visible objects abort running */
				for (obj = square_object(player->cave, grid);
						obj; obj = obj->next) {
					/* Visible object */
					if (obj->known && !ignore_item_ok(player, obj)) {
						disturb(player);
						player->upkeep->running_withpathfind = false;
						return;
					}
				}

				/* Get step after */
				grid = loc_sum(grid, ddgrid[path_step_dir[path_step_idx - 1]]);

				/*
				 * Known impassible terrain that can not be
				 * automatically handled, so run the direction
				 * we were going
				 */
				if (square_isknown(cave, grid)
						&& !square_ispassable(player->cave, grid)
						&& ((!square_iscloseddoor(player->cave, grid)
						&& !square_isrubble(player->cave, grid))
						|| count_neighbors(NULL, cave, grid, square_isnotknown, true) > 0)) {
					player->upkeep->running_withpathfind = false;
					run_init(path_step_dir[path_step_idx]);
				}
			}

			/* Now actually run the step if we're still going */
			run_cur_dir = path_step_dir[path_step_idx--];
		}
	}

	/* Take time */
	player->upkeep->energy_use = energy_per_move(player);

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

