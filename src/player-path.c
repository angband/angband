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
#include "z-queue.h"

/**
 * ------------------------------------------------------------------------
 * Pathfinding code
 * ------------------------------------------------------------------------ */

/**
 * This is an opaque type for communicating the distances, in expected
 * movement turns, from a player's grid to other grids in the cave.
 */
struct pfdistances {
	/* This is height * width entries to store the distances. */
	int *buffer;
	/* This is height pointers to the start of each row in buffer. */
	int **rows;
	/* This is the grid from which the distances are computed. */
	struct loc start;
	/*
	 * These are the dimensions of the arrays, copied from the player's
	 * view of the cave.
	 */
	int height, width;
};

/**
 * This is a patched version (non-overlapping squares of patch_size by
 * patch_size; patch size is a power of 2) of pfdistances for use in
 * find_path().
 */
struct pfdistances_patched {
	int ***patches;
	int patch_size, patch_shift, patch_mask;
	int npatchy, npatchx;
	int height, width;
};

/**
 * Scale factor for distances in an array of path distances; used to allow for
 * fractional turns; must be positive
 */
#define PF_SCL 16

/**
 * Determine whether a grid is OK for the pathfinder to check
 */
static bool is_valid_pf(struct player *p, struct loc grid, bool only_known,
		bool forbid_traps)
{
	/*
	 * If only_known is true, do not want to consider unremembered squares;
	 * otherwise, they are acceptable.
	 */
	if (!square_isknown(p->cave, grid)) return !only_known;

	/*
	 * For all remaining tests, use the player's memory of what's there
	 * to avoid leaking information if the terrain has changed.
	 */
	/* No damaging terrain */
	if (square_isdamaging(p->cave, grid)) return false;

	/* No trapped squares if forbidding traps unless trapsafe */
	if (forbid_traps && square_isvisibletrap(p->cave, grid)
			&& !player_is_trapsafe(p)) {
		return false;
	}

	/* All remaining passable terrain is okay. */
	if (square_ispassable(p->cave, grid)) {
		return true;
	}

	/*
	 * Some impassable terrain can be traversed fairly easily by modifying
	 * the terrain so allow those kinds.
	 */
	if (square_iscloseddoor(p->cave, grid)
			|| square_isrubble(p->cave, grid)) {
		return true;
	}

	/* Reject all other impassable terrain. */
	return false;
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
			if (adjusted < INT_MAX) {
				penalty = (int)adjusted;
				/* Round to the nearest integer. */
				if (remainder >= (factor.d + 1) / 2) {
					++penalty;
				}
			} else {
				penalty = INT_MAX;
			}
		} else {
			penalty = INT_MAX;
		}
	}
	return penalty;
}

/**
 * Help prepare_pfdistances() and find_path():  compute the penalty to
 * distance for going through a known closed door that is unlocked.
 */
static int compute_unlocked_penalty(struct player *p)
{
	/* An unlocked door takes one turn to open. */
	return convert_turn_penalty(PF_SCL, p);
}

/**
 * Help prepare_pfdistances() and find_path():  compute the penalty to
 * distance for going through a known closed door that is locked.
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
		penalty = INT_MAX;
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
		if (scl_turns < INT_MAX) {
			penalty = (int)scl_turns;
			if (remainder >= (avg_turns.d + 1) / 2) {
				++penalty;
			}
		} else {
			penalty = INT_MAX;
		}
	}

	return convert_turn_penalty(penalty, p);
}

/**
 * Help prepare_pfdistances() and find_path():  compute the penalty to distance
 * for going through known impassable rubble.
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
		penalty = INT_MAX;
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
		if (scl_turns < INT_MAX) {
			penalty = (int)scl_turns;
			if (remainder >= (avg_turns.d + 1) / 2) {
				++penalty;
			}
		} else {
			penalty = INT_MAX;
		}
	}

	return convert_turn_penalty(penalty, p);
}

/**
 * Compute the distances, in movement turns, from a given location to all
 * locations in the cave.
 *
 * \param p is the player of interest.
 * \param start is the starting point for the distance calculations.
 * \param only_known will, if true, cause unknown grids to be treated as
 * unreachable.
 * \param forbid_traps will, if true, cause grids with known visible traps
 * to be treated as unreachable.
 * \return a pointer to the opaque distance array type.  If not NULL, that
 * pointer should be passed to release_pfdistances() when it is no longer
 * needed.  The returned result will be NULL if p->cave is NULL or start
 * is not a valid location in p->cave.
 *
 * The computed distances use the player's memory of the cave.  When
 * only_known is false, grids that the player does not remember and are
 * not on the boundary of the cave are treated as if they were easily passable.
 */
struct pfdistances *prepare_pfdistances(struct player *p, struct loc start,
		bool only_known, bool forbid_traps)
{
	struct pfdistances *result;
	struct loc grid;
	struct queue *pending;
	int unlocked_penalty, locked_penalty, rubble_penalty;

	if (!p->cave || !square_in_bounds_fully(p->cave, start)) {
		return NULL;
	}

	result = mem_alloc(sizeof(*result));
	result->buffer = mem_alloc(p->cave->height * p->cave->width
		* sizeof(*result->buffer));
	result->rows = mem_alloc(p->cave->height * sizeof(*result->rows));
	result->start = start;
	result->height = p->cave->height;
	result->width = p->cave->width;

	/* Set up the row pointers. */
	for (grid.y = 0; grid.y < result->height; ++grid.y) {
		result->rows[grid.y] = result->buffer + grid.y * result->width;
	}

	/*
	 * Mark the outer edge as unreachable (negative distance).  Keeps
	 * things in bounds without extra checks later.  Inner grids may
	 * be unreachable; otherwise they start with the assumption of
	 * the maximum possible distance.
	 */
	grid.y = 0;
	for (grid.x = 0; grid.x < result->width; ++grid.x) {
		result->rows[0][grid.x] = -1;
	}
	for (grid.y = 1; grid.y < result->height - 1; ++grid.y) {
		result->rows[grid.y][0] = -1;
		for (grid.x = 1; grid.x < result->width - 1; ++grid.x) {
			result->rows[grid.y][grid.x] = (is_valid_pf(p, grid,
				only_known, forbid_traps)) ?  INT_MAX : -1;
		}
		result->rows[grid.y][result->width - 1] = -1;
	}
	for (grid.x = 0; grid.x < result->width; ++grid.x) {
		result->rows[result->height - 1][grid.x] = -1;
	}

	/* The distance to the starting point is zero. */
	result->rows[result->start.y][result->start.x] = 0;

	/* Precompute quantities to penalize traversing some terrain. */
	unlocked_penalty = compute_unlocked_penalty(p);
	locked_penalty = compute_locked_penalty(p);
	rubble_penalty = compute_rubble_penalty(p);

	/*
	 * Set up a queue with the feasible points that remain to be
	 * considered.  The length of the perimeter of the cave is a guess
	 * at how many feasible points may be present at once.  Will try to
	 * resize if that turns out to be inadequate.
	 */
	pending = q_new(2 * (result->width + result->height - 2));
	/* The starting point is the point to consider. */
	q_push_int(pending, grid_to_i(result->start, result->width));

	/*
	 * For a feasible point, check the eight neighors to see if they
	 * are feasible.
	 */
	do {
		int cur_distance, i;

		i_to_grid(q_pop_int(pending), result->width, &grid);
		cur_distance = result->rows[grid.y][grid.x];
		/*
		 * Move one grid, i.e. PF_SCL, to get to the next grid.  If
		 * that exceeds the maximum distance possible, have no
		 * feasible points from the one under consideration.
		 */
		if (cur_distance >= INT_MAX - PF_SCL) {
			continue;
		}
		cur_distance += PF_SCL;

		/* Try the neighbors. */
		for (i = 0; i < 8; ++i) {
			struct loc next = loc_sum(grid, ddgrid_ddd[i]);

			/*
			 * Skip points that are unreachable or which have
			 * already been reached by a path which is at least
			 * as short as the path under consideration.
			 */
			if (result->rows[next.y][next.x] <= cur_distance) {
				continue;
			}
			/*
			 * Add next as a feasible point; penalize some terrain
			 * if it is known and hard to traverse.
			 */
			if (!square_isknown(p->cave, next)
					|| square_ispassable(p->cave, next)) {
				result->rows[next.y][next.x] = cur_distance;
			} else {
				int penalty, penalized_distance;

				if (square_iscloseddoor(p->cave, next)) {
					penalty = (square_islockeddoor(p->cave,
						next)) ? locked_penalty :
						unlocked_penalty;
				} else if (square_isrubble(p->cave, next)) {
					penalty = rubble_penalty;
				} else {
					/*
					 * Should not happen, treat it as
					 * completely impassable.
					 */
					continue;
				}

				if (cur_distance >= INT_MAX - penalty) {
					/*
					 * Will exceed the maximum allowed
					 * distance so next is not feasible.
					 */
					continue;
				}
				penalized_distance = cur_distance + penalty;
				if (result->rows[next.y][next.x]
						<= penalized_distance) {
					/*
					 * Already have a path there that is
					 * shorter or the same length.  Do not
					 * need to consider this one.
					 */
					continue;
				}
				result->rows[next.y][next.x] =
					penalized_distance;
			}

			assert(q_len(pending) <= q_size(pending)
				&& q_size(pending) > 0);
			if (q_len(pending) == q_size(pending)) {
				if (q_size(pending) > SIZE_MAX / 2
						|| q_resize(pending,
						2 * q_size(pending))) {
					/*
					 * Can not hold the new pending
					 * feasible grid so skip it.
					 */
					continue;
				}
			}
			q_push_int(pending, grid_to_i(next, result->width));
		}
	} while (q_len(pending) > 0);

	q_free(pending);

	return result;
}

/**
 * Return the expected number of movement turns to reach the given grid.
 *
 * \param a is a distance array previously computed for a player.
 * \param grid is the desired destination grid.
 * \return -1 if the destination grid is outside the bounds of a or is
 * unreachable.  Otherwise, return the expected number of movement turns to
 * return the destination grid.
 */
int pfdistances_to_turncount(const struct pfdistances *a, struct loc grid)
{
	if (!a || grid.y < 0 || grid.y >= a->height || grid.x < 0 ||
			grid.x >= a->width || a->rows[grid.y][grid.x] < 0
			|| a->rows[grid.y][grid.x] == INT_MAX) {
		return -1;
	}
	return a->rows[grid.y][grid.x] / PF_SCL
		+ (((a->rows[grid.y][grid.x] % PF_SCL) >= (PF_SCL + 1) / 2)
		? 1 : 0);
}

/**
 * Compute a path given a previously computed distances array and a destination.
 *
 * \param a is a distance array previously computed for a player.
 * \param grid is the desired desination grid.
 * \param step_dirs will, if not NULL, be dereferenced and set to the allocated
 * memory for the steps of the path in reverse order.  Each step is an index
 * to the ddgrid array, so *step_dirs[i] will be the direction to move for
 * the ith (in reverse order) step.  The allocated memory should be released
 * with mem_free() when it is no longer needed.
 * \return the number of steps in the path.  That will be -1 if the destination
 * is unreachable.  That will be zero if the destination is the same grid as
 * player.  When the number of steps is -1 or zero and step_dirs is not NULL,
 * *step_dirs will be set to NULL.
 */
int pfdistances_to_path(const struct pfdistances *a, struct loc grid,
		int16_t **step_dirs)
{
	int16_t *steps;
	int allocated, length;

	if (!a || grid.y < 0 || grid.y >= a->height || grid.x < 0 ||
			grid.x >= a->width || a->rows[grid.y][grid.x] < 0) {
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return -1;
	}
	if (loc_eq(grid, a->start)) {
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return 0;
	}

	/* Work backwards from the destination to the starting point. */
	allocated = 2 + ABS(grid.y - a->start.y) + ABS(grid.x - a->start.x);
	length = 0;
	steps = mem_alloc(allocated * sizeof(*steps));
	while (!loc_eq(grid, a->start)) {
		int k, best_k = -1, best_distance = a->rows[grid.y][grid.x];

		/* Find the next step. */
		for (k = 0; k < 8; ++k) {
			struct loc next = loc_sum(grid, ddgrid_ddd[k]);
			int try_distance;

			/*
			 * The impassable boundary imposed on the distances
			 * array should mean the trial step is always in bounds.
			 */
			assert(next.y >= 0 && next.y < a->height
				&& next.x >= 0 && next.x < a->width);
			try_distance = a->rows[next.y][next.x];
			if (try_distance >= 0 && best_distance > try_distance) {
				best_distance = try_distance;
				best_k = k;
			}
		}

		/*
		 * Bail out if stepping backwards did not improve the distance.
		 */
		if (best_k < 0) {
			mem_free(steps);
			if (step_dirs) {
				*step_dirs = NULL;
			}
			return -1;
		}

		assert(length <= allocated && allocated > 0);
		if (length == allocated) {
			if (allocated > INT_MAX / 2
					|| (size_t)allocated
					> SIZE_MAX / (2 * sizeof(*steps))) {
				mem_free(steps);
				if (step_dirs) {
					*step_dirs = NULL;
				}
				return -1;
			}
			allocated *= 2;
			steps = mem_realloc(steps, allocated * sizeof(*steps));
		}

		/* Record the opposite of the backward direction. */
		steps[length] = 10 - ddd[best_k];
		++length;

		grid = loc_sum(grid, ddgrid_ddd[best_k]);
	}

	if (step_dirs) {
		*step_dirs = steps;
	} else {
		mem_free(steps);
	}
	return length;
}

/**
 * Release a path distance array computed by pfdistances_prepare().
 */
void release_pfdistances(struct pfdistances *a)
{
	if (a) {
		mem_free(a->buffer);
		mem_free(a->rows);
		mem_free(a);
	}
}

static void initialize_patched_distances(struct pfdistances_patched *distances,
		int height, int width)
{
	int i;

	assert(height > 0 && width > 0);
	distances->patch_shift = 4;
	distances->patch_size = 1 << distances->patch_shift;
	distances->patch_mask = distances->patch_size - 1;
	distances->npatchy = height >> distances->patch_shift;
	if (height & distances->patch_mask) {
		++distances->npatchy;
	}
	distances->npatchx = width >> distances->patch_shift;
	if (width & distances->patch_mask) {
		++distances->npatchx;
	}
	distances->height = height;
	distances->width = width;
	distances->patches = mem_alloc(distances->npatchy
		* sizeof(*distances->patches));
	for (i = 0; i < distances->npatchy; ++i) {
		distances->patches[i] = mem_zalloc(distances->npatchx
			* sizeof(**distances->patches));
	}
}

static void release_patched_distances(struct pfdistances_patched *distances)
{
	int i;

	assert(distances->patches && distances->npatchy > 0
		&& distances->npatchx > 0);
	for (i = 0; i < distances->npatchy; ++i) {
		int j;

		assert(distances->patches[i]);
		for (j = 0; j < distances->npatchx; ++j) {
			mem_free(distances->patches[i][j]);
		}
		mem_free(distances->patches[i]);
	}
	mem_free(distances->patches);
}

static void clear_patched_distances(struct pfdistances_patched *distances)
{
	int i;

	assert(distances->patches && distances->npatchy > 0
		&& distances->npatchx > 0);
	for (i = 0; i < distances->npatchy; ++i) {
		int j;

		assert(distances->patches[i]);
		for (j = 0; j < distances->npatchx; ++j) {
			mem_free(distances->patches[i][j]);
			distances->patches[i][j] = NULL;
		}
	}
}

static void initialize_patch(struct pfdistances_patched *distances,
		struct loc grid, struct player *p, bool only_known,
		bool forbid_traps)
{
	int *block;
	int patchy, patchx, i;
	struct loc corner, cursor;

	assert(grid.y >= 0 && grid.y < distances->height
		&& grid.x >= 0 && grid.x < distances->width);
	patchy = grid.y >> distances->patch_shift;
	assert(patchy >= 0 && patchy < distances->npatchy);
	patchx = grid.x >> distances->patch_shift;
	assert(patchx >= 0 && patchx < distances->npatchx);
	assert(!distances->patches[patchy][patchx]);
	block = mem_alloc(distances->patch_size * distances->patch_size
		* sizeof(*block));
	distances->patches[patchy][patchx] = block;

	corner.y = patchy << distances->patch_shift;
	corner.x = patchx << distances->patch_shift;
	for (cursor.y = corner.y, i = 0; cursor.y < corner.y
			+ distances->patch_size; ++cursor.y) {
		for (cursor.x = corner.x; cursor.x < corner.x
				+ distances->patch_size; ++cursor.x, ++i) {
			block[i] = (square_in_bounds_fully(p->cave, cursor)
				&& is_valid_pf(p, cursor, only_known,
				forbid_traps)) ? INT_MAX : -1;
		}
	}
}

static bool has_patched_distance(const struct pfdistances_patched *distances,
		struct loc grid)
{
	int patchy, patchx;

	assert(grid.y >= 0 && grid.y < distances->height
		&& grid.x >= 0 && grid.x < distances->width);
	patchy = grid.y >> distances->patch_shift;
	assert(patchy >= 0 && patchy < distances->npatchy);
	patchx = grid.x >> distances->patch_shift;
	assert(patchx >= 0 && patchx < distances->npatchx);
	return distances->patches[patchy][patchx];
}

static int get_patched_distance(const struct pfdistances_patched *distances,
		struct loc grid)
{
	int patchy, patchx, patchi;

	assert(grid.y >= 0 && grid.y < distances->height
		&& grid.x >= 0 && grid.x < distances->width);
	patchy = grid.y >> distances->patch_shift;
	assert(patchy >= 0 && patchy < distances->npatchy);
	patchx = grid.x >> distances->patch_shift;
	assert(patchx >= 0 && patchx < distances->npatchx);
	assert(distances->patches[patchy][patchx]);
	patchi = ((grid.y & distances->patch_mask) << distances->patch_shift)
		+ (grid.x & distances->patch_mask);
	assert(patchi >= 0 && patchi < distances->patch_size
		* distances->patch_size);
	return distances->patches[patchy][patchx][patchi];
}

static void set_patched_distance(struct pfdistances_patched *distances,
		struct loc grid, int distance)
{
	int patchy, patchx, patchi;

	assert(grid.y >= 0 && grid.y < distances->height
		&& grid.x >= 0 && grid.x < distances->width);
	patchy = grid.y >> distances->patch_shift;
	assert(patchy >= 0 && patchy < distances->npatchy);
	patchx = grid.x >> distances->patch_shift;
	assert(patchx >= 0 && patchx < distances->npatchx);
	assert(distances->patches[patchy][patchx]);
	patchi = ((grid.y & distances->patch_mask) << distances->patch_shift)
		+ (grid.x & distances->patch_mask);
	assert(patchi >= 0 && patchi < distances->patch_size
		* distances->patch_size);
	distances->patches[patchy][patchx][patchi] = distance;
}

static int patched_distances_to_path(const struct pfdistances_patched
		*distances, struct loc start, struct loc dest,
		int16_t **step_dirs)
{
	int allocated, length, last_distance;
	int16_t *steps;

	/* Work backwards from the destination to the starting point. */
	allocated = 2 + ABS(dest.y - start.y) + ABS(dest.x - start.x);
	length = 0;
	steps = mem_alloc(allocated * sizeof(*steps));
	last_distance = INT_MAX;
	while (!loc_eq(dest, start)) {
		struct loc best_grid = loc(-1, -1);
		int k, best_k = -1;

		/* Find the next step. */
		for (k = 0; k < 8; ++k) {
			struct loc next = loc_sum(dest, ddgrid_ddd[k]);
			int try_distance;

			if (!has_patched_distance(distances, next)) {
				continue;
			}
			try_distance = get_patched_distance(distances, next);
			if (try_distance >= 0 && last_distance > try_distance) {
				last_distance = try_distance;
				best_k = k;
				best_grid = next;
			}
		}

		assert(best_k >= 0);
		assert(best_grid.y >= 0 && best_grid.y < distances->height
			&& best_grid.x >= 0 && best_grid.x < distances->width);
		dest = best_grid;
		assert(length <= allocated && allocated > 0);
		if (length == allocated) {
			if (allocated > INT_MAX / 2
					|| (size_t)allocated
					> SIZE_MAX / (2 * sizeof(*steps))) {
				mem_free(steps);
				if (step_dirs) {
					*step_dirs = NULL;
				}
				return -1;
			}
			allocated *= 2;
			steps = mem_realloc(steps, allocated * sizeof(*steps));
		}

		/* Record the opposite of the backward direction. */
		steps[length] = 10 - ddd[best_k];
		++length;
	}

	if (step_dirs) {
		*step_dirs = steps;
	} else {
		mem_free(steps);
	}
	return length;
}

/**
 * Compute the path to the nearest (to the given starting point) known grid
 * that satisfies the given predicate and is not the same as the starting
 * point.
 *
 * \param p is the player of interest.
 * \param start is the starting point for the search.
 * \param pred is the predicate that the destination must satisfy.
 * \param dest_grid will, if not NULL, be dereferenced and set to the
 * destination for the path.
 * \param step_dirs will, if not NULL, be dereferenced and set to the allocated
 * memory for the steps of the path in reverse order.  Each step is an index
 * to the ddgrid array, so *step_dirs[i] will be the direction to move for
 * the ith (in reverse order) step.  The allocated memory should be released
 * with mem_free() when it is no longer needed.
 * \return the number of steps in the path.  That will be -1 if no suitable
 * destination was found.  When the number of steps is -1, *dest_grid will be
 * set to loc(-1, -1) if dest_grid is not NULL and *step_dirs will be set to
 * NULL if step_dirs is not NULL.
 */
int path_nearest_known(struct player *p, struct loc start,
		bool (*pred)(struct chunk*, struct loc),
		struct loc *dest_grid, int16_t **step_dirs)
{
	bool only_known = true, forbid_traps = true;

	while (1) {
		struct pfdistances *distances = NULL;
		struct loc min_grid = loc(-1, -1);
		int min_turns = INT_MAX;
		struct loc grid;

		for (grid.y = 0; grid.y < p->cave->height; ++grid.y) {
			for (grid.x = 0; grid.x < p->cave->width; ++grid.x) {

				if (loc_eq(grid, start)) {
					continue;
				}
				if (square_isknown(p->cave, grid)
						&& (*pred)(p->cave, grid)) {
					int turns;

					if (!distances) {
						distances = prepare_pfdistances(
							p, start, only_known,
							forbid_traps);
					}
					turns = pfdistances_to_turncount(
						distances, grid);
					if (turns > 0 && min_turns > turns) {
						min_turns = turns;
						min_grid = grid;
					}
				}
			}
		}

		if (min_turns < INT_MAX) {
			int path_length;

			assert(distances
				&& square_in_bounds(p->cave, min_grid));
			if (dest_grid) {
				*dest_grid = min_grid;
			}
			path_length = pfdistances_to_path(distances,
				min_grid, step_dirs);
			release_pfdistances(distances);
			assert(path_length > 0);
			return path_length;
		}

		release_pfdistances(distances);
		/*
		 * No destination was found.  Try looser constraints on the
		 * grids that can be in the path.
		 */
		if (forbid_traps && !player_is_trapsafe(p)) {
			forbid_traps = false;
			continue;
		}
		if (only_known) {
			only_known = false;
			forbid_traps = true;
			continue;
		}
		/* Nothing more to try for passable grids.  Give up. */
		if (dest_grid) {
			*dest_grid = loc(-1, -1);
		}
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return -1;
	}
}

/**
 * Compute the path to either the nearest (to the starting point) known
 * passable grid that is not the staring point and has an unknown neighbor or,
 * if there is not such a grid, to the nearest (to the starting point) known
 * closed door or known rubble that has an unknown neighbor and a known
 * neighbor that is passable.
 *
 * \param p is the player of interest.
 * \param start is the starting point for the search.
 * \param dest_grid will, if not NULL, be dereferenced and set to the
 * destination for the path.
 * \param step_dirs will, if not NULL, be dereferenced and set to the allocated
 * memory for the steps of the path in reverse order.  Each step is an index
 * to the ddgrid array, so *step_dirs[i] will be the direction to move for
 * the ith (in reverse order) step.  The allocated memory should be released
 * with mem_free() when it is no longer needed.
 * \return the number of steps in the path.  That will be -1 if no suitable
 * destination was found.  When the number of steps is -1, *dest_grid will be
 * set to loc(-1, -1) if dest_grid is not NULL and *step_dirs will be set to
 * NULL if step_dirs is not NULL.
 */
int path_nearest_unknown(struct player *p, struct loc start,
		struct loc *dest_grid, int16_t **step_dirs)
{
	bool only_known = true, forbid_traps = true, passable = true;

	while (1) {
		struct pfdistances *distances = NULL;
		struct loc min_grid = loc(-1, -1);
		int min_turns = INT_MAX;
		struct loc grid;

		for (grid.y = 0; grid.y < p->cave->height; ++grid.y) {
			for (grid.x = 0; grid.x < p->cave->width; ++grid.x) {
				struct loc test_grid;
				int turns;

				if (loc_eq(grid, start)
						|| !square_isknown(p->cave,
						grid)) {
					continue;
				}
				if (passable) {
					if (!square_ispassable(p->cave, grid)
							|| count_neighbors(NULL,
							p->cave, grid,
							square_isknown, false)
							== 8) {
						continue;
					}
					test_grid = grid;
				} else {
					if ((!square_iscloseddoor(p->cave, grid)
							&& !square_isrubble(
							p->cave, grid))
							|| count_neighbors(NULL,
							p->cave, grid,
							square_isknown,
							false) == 8) {
						continue;
					}
					if (count_neighbors(&test_grid,
							p->cave, grid,
							square_isknownpassable,
							false) == 0 ||
							loc_eq(test_grid,
							start)) {
						continue;
					}
				}

				if (!distances) {
					distances = prepare_pfdistances(
						p, start, only_known,
						forbid_traps);
				}
				turns = pfdistances_to_turncount(distances,
					test_grid);
				if (turns > 0 && min_turns > turns) {
					min_turns = turns;
					min_grid = test_grid;
				}
			}
		}

		if (min_turns < INT_MAX) {
			int path_length;

			assert(distances
				&& square_in_bounds(p->cave, min_grid));
			if (dest_grid) {
				*dest_grid = min_grid;
			}
			path_length = pfdistances_to_path(distances,
				min_grid, step_dirs);
			release_pfdistances(distances);
			assert(path_length > 0);
			return path_length;
		}

		release_pfdistances(distances);
		/*
		 * No destination was found.  Try looser constraints on the
		 * grids that can be in the path.
		 */
		if (forbid_traps && !player_is_trapsafe(p)) {
			forbid_traps = false;
			continue;
		}
		if (only_known) {
			only_known = false;
			forbid_traps = true;
			continue;
		}
		/*
		 * Looser constraints did not help.  Try looking for known
		 * closed doors or known rubble with unknown neighbors.
		 */
		if (passable) {
			passable = false;
			only_known = true;
			forbid_traps = true;
			continue;
		}
		/* Nothing more to try.  Give up. */
		if (dest_grid) {
			*dest_grid = loc(-1, -1);
		}
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return -1;
	}
}

/**
 * Compute the path from one location to another using the given player's
 * knowledge of the cave.
 *
 * \param p is the player of interest.
 * \param start is the starting point for the path.
 * \param dest is the destination for the path.
 * \param step_dirs will, if not NULL, be dereferenced and set to the allocated
 * memory for the steps of the path in reverse order.  Each step is an index
 * to the ddgrid array, so *step_dirs[i] will be the direction to move for
 * the ith (in reverse order) step.  The allocated memory should be released
 * with mem_free() when it is no longer needed.
 * \return the number of steps in the path.  That will be -1 if the destination
 * is unreachable.  That will be zero if the destination is the same grid as
 * player.  When the number of steps is -1 or zero and step_dirs is not NULL,
 * *step_dirs will be set to NULL.
 *
 * find_path() should perform better than prepare_pfdistances(),
 * pfdistances_to_path(), and release_pfdistances().  When there are paths
 * of the same distances (in expected turncounts) between start and dest, the
 * path returned by find_path() may be different than that returned by
 * pfdistances_to_path().
 */
int find_path(struct player *p, struct loc start, struct loc dest,
		int16_t **step_dirs)
{
	/*
	 * Store the grid at the head of the path in the queue and have
	 * separate storage for a distance array.  Initialize that distance
	 * array in patches of patch_size by patch_size to limit overhead from
	 * parts of the cave that are not traversed when moving to the
	 * destination.
	 */
	struct pfdistances_patched distances;
	struct priority_queue *pending;
	struct loc next;
	int dist_next;
	int unlocked_penalty, locked_penalty, rubble_penalty;
	bool only_known, forbid_traps, hit_trap;

	if (!p->cave || !square_in_bounds(p->cave, start)
			|| !square_in_bounds(p->cave, dest)) {
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return -1;
	}

	if (loc_eq(start, dest)) {
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return 0;
	}

	/*
	 * If both the starting point and destination are remembered grids,
	 * first try paths that only traverse remembered grids.  If there are
	 * no such paths, then try ones that can include grids that are not
	 * remembered.
	 */
	only_known = square_isknown(p->cave, start)
		&& square_isknown(p->cave, dest);
	/*
	 * First try paths that do not go through grids with known visible
	 * traps.  If there are no such paths, then try paths that include
	 * known visible traps.
	 */
	if (is_valid_pf(p, dest, only_known, true)) {
		forbid_traps = true;
	} else if (is_valid_pf(p, dest, only_known, false)) {
		forbid_traps = false;
	} else {
		/*
		 * The destination is not reachable because it contains
		 * known terrain that pathfinding can not traverse.
		 */
		if (step_dirs) {
			*step_dirs = NULL;
		}
		return -1;
	}
	/*
	 * Remember if the pathfinding would change because of a known visible
	 * trap.
	 */
	hit_trap = false;

	/* Precompute quantities to penalize traversing some terrain. */
	unlocked_penalty = compute_unlocked_penalty(p);
	locked_penalty = compute_locked_penalty(p);
	rubble_penalty = compute_rubble_penalty(p);

	initialize_patched_distances(&distances, p->cave->height,
		p->cave->width);

	/* Set up the priority queue of feasible paths to consider. */
	pending = qp_new(4 * (2 + MAX(ABS(start.y - dest.y),
		ABS(start.x - dest.x))));

	initialize_patch(&distances, start, p, only_known, forbid_traps);
	set_patched_distance(&distances, start, 0);
	next = start;
	dist_next = 0;
	while (1) {
		int add_grid = -1, add_priority = -1;
		/* This is the base distance to any neighbor. */
		int dist_this = dist_next + PF_SCL, i;

		/* Try the neighbors. */
		for (i = 0; i < 8; ++i) {
			struct loc this_grid = loc_sum(next, ddgrid_ddd[i]);
			int dist_stored, dist_remaining, penalty;

			if (loc_eq(this_grid, dest)) {
				/* Reached the destination. */
				int length = patched_distances_to_path(
					&distances, start, dest,
					step_dirs);

				release_patched_distances(&distances);
				qp_free(pending, NULL);
				return length;
			}

			if (!has_patched_distance(&distances, this_grid)) {
				initialize_patch(&distances, this_grid,
					p, only_known, forbid_traps);
			}
			dist_stored = get_patched_distance(&distances,
				this_grid);
			if (dist_stored <= dist_this) {
				/*
				 * Since it is unreachable or already has been
				 * visited by a path that is not longer than
				 * this one, do not need to consider it.  If
				 * not allowing traps and this grid is a known
				 * visible trap, remember that there is at
				 * least one trap that affects the pathfinding.
				 */
				if (forbid_traps && square_isknown(p->cave,
						this_grid)
						&& square_isvisibletrap(
						p->cave, this_grid)) {
					hit_trap = true;
				}
				continue;
			}

			/*
			 * Use A* pathfinding:  add an estimate (in this case
			 * the Chebyshev distance) to get from this_grid to
			 * the destination.
			 */
			dist_remaining = MAX(ABS(dest.x - this_grid.x),
				ABS(dest.y - this_grid.y));
			if (dist_remaining > INT_MAX / PF_SCL
					|| dist_this >= INT_MAX
					- PF_SCL * dist_remaining) {
				/*
				 * Can not reach the destination from this_grid
				 * in a reasonable number of turns, so skip
				 * it.
				 */
				continue;
			}
			dist_remaining *= PF_SCL;

			if (square_isknown(p->cave, this_grid)
					&& !square_ispassable(p->cave,
					this_grid)) {
				/*
				 * Penalize the distance for some known but
				 * impassable terrain.
				 */
				if (square_iscloseddoor(p->cave, this_grid)) {
					penalty = (square_islockeddoor(p->cave,
						this_grid)) ? locked_penalty :
						unlocked_penalty;
				} else if (square_isrubble(p->cave,
						this_grid)) {
					penalty = rubble_penalty;
				} else {
					/*
					 * Should not happen, treat it as
					 * completely impassable.
					 */
					penalty = INT_MAX;
				}

				if (dist_this >= dist_stored - penalty
						|| dist_this >= INT_MAX -
						penalty - dist_remaining) {
					/*
					 * The penalty makes this path
					 * no shorter than what has already
					 * reached this grid or puts the
					 * destination out of reach.  Skip it.
					 */
					continue;
				}
			} else {
				penalty = 0;
			}

			/* Push what is pending onto the queue. */
			if (add_grid > 0) {
				if (qp_len(pending) == qp_size(pending)) {
					assert(qp_size(pending) > 0);
					if (qp_size(pending) > SIZE_MAX / 2 ||
							qp_resize(pending, 2
							* qp_size(pending),
							NULL)) {
						/*
						 * Could not resize so give
						 * up.
						 */
						release_patched_distances(
							&distances);
						qp_free(pending, NULL);
						if (step_dirs) {
							*step_dirs = NULL;
						}
						return -1;
					}
				}
				qp_push_int(pending, add_priority, add_grid);
			}
			add_grid = grid_to_i(this_grid, p->cave->width);
			add_priority = dist_this + penalty + dist_remaining;
			set_patched_distance(&distances, this_grid,
				dist_this + penalty);
		}

		if (add_grid >= 0) {
			i_to_grid(qp_pushpop_int(pending, add_priority,
				add_grid), p->cave->width, &next);
		} else {
			if (qp_len(pending) == 0) {
				/*
				 * Exhausted possible paths without reaching
				 * the destination.
				 */
				if (forbid_traps && !player_is_trapsafe(p)
						&& hit_trap) {
					/*
					 * Retry but allow grids that contain
					 * known visible traps.
					 */
					forbid_traps = false;
					clear_patched_distances(&distances);
					initialize_patch(&distances, start,
						p, only_known, forbid_traps);
					set_patched_distance(&distances,
						start, 0);
					next = start;
					dist_next = 0;
					continue;
				}
				if (only_known) {
					/*
					 * Retry but allow grids that are not
					 * in the player's memory.
					 */
					only_known = false;
					if (is_valid_pf(p, dest, false, true)) {
						forbid_traps = true;
					} else {
						forbid_traps = false;
					}
					hit_trap = false;
					clear_patched_distances(&distances);
					initialize_patch(&distances, start,
						p, only_known, forbid_traps);
					set_patched_distance(&distances,
						start, 0);
					next = start;
					dist_next = 0;
					continue;
				}
				/* Nothing to retry so give up. */
				release_patched_distances(&distances);
				qp_free(pending, NULL);
				if (step_dirs) {
					*step_dirs = NULL;
				}
				return -1;
			}
			i_to_grid(qp_pop_int(pending), p->cave->width, &next);
		}
		/* The relevant patch should already have been initialized. */
		assert(has_patched_distance(&distances, next));
		dist_next = get_patched_distance(&distances, next);
	}
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
		if (!player->upkeep->steps) {
			/* Update regular running */
			if (run_test(player)) {
				/* Disturb */
				disturb(player);
				return;
			}
		} else if (player->upkeep->step_count <= 0) {
			/* Pathfinding, and the path is finished */
			disturb(player);
			return;
		} else {
			int next_step_ind = player->upkeep->step_count - 1;
			int next_step_dir =
				player->upkeep->steps[next_step_ind];
			struct loc grid;
			struct object *obj;

			assert(player->upkeep->steps);
			grid = loc_sum(player->grid, ddgrid[next_step_dir]);

			/*
			 * Automatically deal with some impassable
			 * terrain if that grid and its immediate neighors are
			 * known.  Since disturb() flushes queued commands,
			 * first stop running before pushing the commands to
			 * deal with the terrain and restart pathfinding.
			 */
			if (square_iscloseddoor(player->cave, grid)) {
				if (count_neighbors(NULL, cave, grid,
						square_isknown, true) == 9) {
					struct loc dest =
						player->upkeep->path_dest;

					disturb(player);
					cmdq_push(CMD_OPEN);
					cmdq_peek()->background_command = 1;
					cmd_set_arg_direction(cmdq_peek(),
						"direction", next_step_dir);
					cmdq_push(CMD_PATHFIND);
					cmd_set_arg_point(cmdq_peek(),
						"point", dest);
					return;
				}
			} else if (square_isrubble(player->cave, grid)
					&& !square_ispassable(player->cave,
					grid)) {
				if (count_neighbors(NULL, cave, grid,
						square_isknown, true) == 9) {
					struct loc dest =
						player->upkeep->path_dest;

					disturb(player);
					cmdq_push(CMD_TUNNEL);
					cmdq_peek()->background_command = 1;
					cmd_set_arg_direction(cmdq_peek(),
						"direction", next_step_dir);
					cmdq_push(CMD_PATHFIND);
					cmd_set_arg_point(cmdq_peek(),
						"point", dest);
					return;
				}
			}

			/*
			 * Known impassable terrain that is not automatically
			 * handled.
			 */
			if (square_isknown(cave, grid) && !square_ispassable(
					player->cave, grid)) {
				disturb(player);
				return;
			}

			/* Visible monsters abort running */
			if (square(cave, grid)->mon > 0) {
				struct monster *mon =
					square_monster(cave, grid);

				/* Visible monster */
				if (monster_is_visible(mon)) {
					disturb(player);
					return;
				}
			}

			/* Visible objects abort running */
			for (obj = square_object(player->cave, grid); obj;
					obj = obj->next) {
				/* Visible object */
				if (obj->known && !ignore_item_ok(player,
						obj)) {
					disturb(player);
					return;
				}
			}

			/*
			 * If the player has computed a path that is going to
			 * end up in a wall, we notice this and convert to a
			 * normal run. This allows us to click on unknown areas
			 * to explore the map.
			 *
			 * We have to look ahead two, otherwise we don't know
			 * which is the last direction moved and don't
			 * initialise the run properly.
			 */
			if (next_step_ind > 0) {
				/* Get step after */
				grid = loc_sum(grid, ddgrid[
					player->upkeep->steps[
					next_step_ind - 1]]);

				/*
				 * Known impassable terrain that can not be
				 * automatically handled, so run the direction
				 * we were going
				 */
				if (square_isknown(cave, grid)
						&& !square_ispassable(
						player->cave, grid)
						&& ((!square_iscloseddoor(
						player->cave, grid)
						&& !square_isrubble(
						player->cave, grid))
						|| count_neighbors(NULL, cave,
						grid, square_isknown, true)
						!= 9)) {
					mem_free(player->upkeep->steps);
					player->upkeep->steps = NULL;
					run_init(next_step_dir);
				}
			}

			/* Now actually run the step if we're still going */
			if (player->upkeep->steps) {
				run_cur_dir = next_step_dir;
				player->upkeep->step_count = next_step_ind;
			}
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
	} else if (!player->upkeep->steps)
		return;

	/* Prepare the next step */
	if (player->upkeep->running) {
		cmdq_push(CMD_RUN);
		if (player->upkeep->steps) {
			/*
			 * Running is a side effect of pathfinding.  Do allow
			 * it to trigger bloodlust so pathfinding can not
			 * be exploited as a way to move without the bloodlust
			 * checks.
			 */
			cmdq_peek()->background_command = 1;
		}
		cmd_set_arg_direction(cmdq_peek(), "direction", 0);
	} else if (player->upkeep->steps) {
		mem_free(player->upkeep->steps);
		player->upkeep->steps = NULL;
	}
}

