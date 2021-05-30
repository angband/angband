/**
 * \file gen-cave.c
 * \brief Generation of dungeon levels
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Erik Osheim, Nick McConnell
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
 *
 * In this file, we use the SQUARE_WALL flags to the info field in
 * cave->squares.  Those are usually only applied and tested on granite, but
 * some (SQUARE_WALL_INNER) is applied and tested on permanent walls.
 * SQUARE_WALL_SOLID indicates the wall should not be tunnelled;
 * SQUARE_WALL_INNER marks an inward-facing wall of a room; SQUARE_WALL_OUTER
 * marks an outer wall of a room.
 *
 * We use SQUARE_WALL_SOLID to prevent multiple corridors from piercing a wall
 * in two adjacent locations, which would be messy, and SQUARE_WALL_OUTER
 * to indicate which walls surround rooms, and may thus be pierced by corridors
 * entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the edge of the
 * dungeon in a direction toward that edge will cause "silly" wall piercings,
 * but will have no permanently incorrect effects, as long as the tunnel can
 * eventually exit from another side. And note that the wall may not come back
 * into the room by the hole it left through, so it must bend to the left or
 * right and then optionally re-enter the room (at least 2 grids away). This is
 * not a problem since every room that is large enough to block the passage of
 * tunnels is also large enough to allow the tunnel to pierce the room itself
 * several times.
 *
 * Note that no two corridors may enter a room through adjacent grids, they
 * must either share an entryway or else use entryways at least two grids
 * apart. This prevents large (or "silly") doorways.
 *
 * Traditionally, to create rooms in the dungeon, it was divided up into
 * "blocks" of 11x11 grids each, and all rooms were required to occupy a
 * rectangular group of blocks.  As long as each room type reserved a
 * sufficient number of blocks, the room building routines would not need to
 * check bounds. Note that in classic generation most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that a lot of the original motivation for the block system was the
 * fact that there was only one size of map available, 22x66 grids, and the
 * dungeon level was divided up into nine of these in three rows of three.
 * Now that the map can be resized and enlarged, and dungeon levels themselves
 * can be different sizes, much of this original motivation has gone.  Blocks
 * can still be used, but different cave profiles can set their own block
 * sizes.  The classic generation method still uses the traditional blocks; the
 * main motivation for using blocks now is for the aesthetic effect of placing
 * rooms on a grid.
 */

#include "angband.h"
#include "cave.h"
#include "datafile.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "math.h"
#include "mon-group.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "player-util.h"
#include "store.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/**
 * Check whether a square has one of the tunnelling helper flags
 * \param c is the current chunk
 * \param y are the co-ordinates
 * \param x are the co-ordinates
 * \param flag is the relevant flag
 */
static bool square_is_granite_with_flag(struct chunk *c, struct loc grid,
										int flag)
{
	if (square(c, grid)->feat != FEAT_GRANITE) return false;
	if (!sqinfo_has(square(c, grid)->info, flag)) return false;

	return true;
}

/**
 * Places a streamer of rock through dungeon.
 *
 * \param c is the current chunk
 * \param feat is the base feature (FEAT_MAGMA or FEAT_QUARTZ)
 * \param chance is the number of regular features per one gold
 *
 * Note that their are actually six different terrain features used to
 * represent streamers. Three each of magma and quartz, one for basic vein, one
 * with hidden gold, and one with known gold. The hidden gold types are
 * currently unused.
 */
static void build_streamer(struct chunk *c, int feat, int chance)
{
	/* Hack -- Choose starting point */
	struct loc grid = rand_loc(loc(c->width / 2, c->height / 2), 15, 10);

	/* Choose a random direction */
	int dir = ddd[randint0(8)];

	/* Place streamer into dungeon */
	while (true) {
		int i;
		struct loc change;

		/* One grid per density */
		for (i = 0; i < dun->profile->str.den; i++) {
			int d = dun->profile->str.rng;

			/* Pick a nearby grid */
			find_nearby_grid(c, &change, grid, d, d);

			/* Only convert walls */
			if (square_isrock(c, change)) {
				/* Turn the rock into the vein type */
				square_set_feat(c, change, feat);

				/* Sometimes add known treasure */
				if (one_in_(chance)) square_upgrade_mineral(c, change);
			}
		}

		/* Advance the streamer */
		grid = loc_sum(grid, ddgrid[dir]);

		/* Stop at dungeon edge */
		if (!square_in_bounds(c, grid)) break;
	}
}


/**
 * Reset entrance data for rooms in global dun.
 * \param c Is the chunk holding the rooms.
 */
static void reset_entrance_data(const struct chunk *c)
{
	int i;

	for (i = 0; i < z_info->level_room_max; ++i) {
		dun->ent_n[i] = 0;
	}
	if (dun->ent2room) {
		for (i = 0; dun->ent2room[i]; ++i) {
			mem_free(dun->ent2room[i]);
		}
		mem_free(dun->ent2room);
	}
	/* Add a trailing NULL so the deallocation knows when to stop. */
	dun->ent2room = mem_alloc((c->height + 1) * sizeof(*dun->ent2room));
	for (i = 0; i < c->height; ++i) {
		int j;

		dun->ent2room[i] =
			mem_alloc(c->width * sizeof(*dun->ent2room[i]));
		for (j = 0; j < c->width; ++j) {
			dun->ent2room[i][j] = -1;
		}
	}
	dun->ent2room[c->height] = NULL;
}


/**
 * Randomly choose a room entrance and return its coordinates.
 * \param c Is the chunk to use.
 * \param ridx Is the 0-based index for the room.
 * \param tgt If not NULL, the choice of entrance will either be *tgt if *tgt
 * is an entrance for the room, ridx, or can be biased to be closer to *tgt
 * when *tgt is not an entrance for the room, ridx.
 * \param bias Sets the amount of bias if tgt is not NULL and *tgt is not an
 * entrance for the room, ridx.  A larger value increases the amount of bias.
 * A value of zero will give no bias.  Must be non-negative.
 * \param exc Is an array of grids whose adjacent neighbors (but not the grid
 * itself) should be excluded from selection.  May be NULL if nexc is not
 * positive.
 * \param nexc Is the number of grids to use from exc.
 * \return The returned value is an entrance for the room or (0, 0) if
 * no entrance is available.  An entrance, x, satisfies these requirements:
 * 1) x is the same as dun->ent[ridx][k] for some k between 0 and
 * dun->ent_n[ridx - 1].
 * 2) square_is_marked_granite(c, x, SQUARE_WALL_OUTER) is true.
 * 3) For all m between zero and nexc - 1, ABS(x.x - exc[m].x) > 1 or
 * ABS(x.y - exc[m].y) > 1 or (x.x == exc[m].x and x.y == exc[m].y).
 */
static struct loc choose_random_entrance(struct chunk *c, int ridx,
	const struct loc *tgt, int bias, const struct loc *exc, int nexc)
{
	assert(ridx >= 0 && ridx < dun->cent_n);
	if (dun->ent_n[ridx] > 0) {
		int nchoice = 0;
		int *accum = mem_alloc((dun->ent_n[ridx] + 1) *
			sizeof(*accum));
		int i;

		accum[0] = 0;
		for (i = 0; i < dun->ent_n[ridx]; ++i) {
			bool included = square_is_granite_with_flag(c,
				dun->ent[ridx][i], SQUARE_WALL_OUTER);

			if (included) {
				int j = 0;

				while (1) {
					struct loc diff;

					if (j >= nexc) {
						break;
					}
					diff = loc_diff(dun->ent[ridx][i],
						exc[j]);
					if (ABS(diff.x) <= 1 &&
							ABS(diff.y) <= 1 &&
							(diff.x != 0 ||
							diff.y != 0)) {
						included = false;
						break;
					}
					++j;
				}
			}
			if (included) {
				if (tgt) {
					int d, biased;

					assert(bias >= 0);
					d = distance(dun->ent[ridx][i], *tgt);
					if (d == 0) {
						/*
						 * There's an exact match.  Use
						 * it.
						 */
						mem_free(accum);
						return dun->ent[ridx][i];
					}

					biased = MAX(1, bias - d);
					/*
					 * Squaring here is just a guess without
					 * any specific reason to back it.
					 */
					accum[i + 1] = accum[i] +
						biased * biased;
				} else {
					accum[i + 1] = accum[i] + 1;
				}
				++nchoice;
			} else {
				accum[i + 1] = accum[i];
			}
		}
		if (nchoice > 0) {
			int chosen = randint0(accum[dun->ent_n[ridx]]);
			int low = 0, high = dun->ent_n[ridx];

			/* Locate the selection by binary search. */
			while (1) {
				int mid;

				if (low == high - 1) {
					assert(accum[low] <= chosen &&
						accum[high] > chosen);
					mem_free(accum);
					return dun->ent[ridx][low];
				}
				mid = (low + high) / 2;
				if (accum[mid] <= chosen) {
					low = mid;
				} else {
					high = mid;
				}
			}
		}
		mem_free(accum);
	}

	/* There's no satisfactory marked entrances. */
	return loc(0, 0);
}


/**
 * Help build_tunnel():  pierce an outer wall and prevent nearby piercings.
 * \param c Is the chunk to use.
 * \param grid Is the location to pierce.
 */
static void pierce_outer_wall(struct chunk *c, struct loc grid)
{
	struct loc adj;

	/* Save the wall location */
	if (dun->wall_n < z_info->wall_pierce_max) {
		dun->wall[dun->wall_n] = grid;
		dun->wall_n++;
	}

	/* Forbid re-entry near this piercing */
	for (adj.y = grid.y - 1; adj.y <= grid.y + 1; adj.y++) {
		for (adj.x = grid.x - 1; adj.x <= grid.x + 1; adj.x++) {
			if (adj.x != 0 && adj.y != 0 &&
					square_in_bounds(c, adj) &&
					square_is_granite_with_flag(c, adj,
					SQUARE_WALL_OUTER)) {
				set_marked_granite(c, adj, SQUARE_WALL_SOLID);
			}
		}
	}
}


/**
 * Help build_tunnel():  handle bookkeeping, mainly if there's a diagonal step,
 * for the first step after piercing a wall.
 * \param c Is the chunk to use.
 * \param grid At entry, *grid is the location at which the wall was pierced.
 * At exit, *grid is the starting point for the next iteration of tunnel
 * building.
 * \param dir At entry, *dir is the chosen direction for the first step after
 * the wall piercing.  At exit, *dir is the direction for the next iteration of
 * tunnel building.
 * \param door_flag At entry, *door_flag is the current setting for whether a
 * door can be added.  At exit, *door_flag is the setting for whether a door
 * can be added in the next iteration of tunnel building.
 * \param bend_invl At entry, *bend_intvl is the current setting for the number
 * of tunnel iterations to wait before applying a bend.  At exit, *bend_intvl
 * is what that intverval should be for the next iteration of tunnel building.
 */
static void handle_post_wall_step(struct chunk *c, struct loc *grid,
	struct loc *dir, bool *door_flag, int *bend_intvl)
{
	if (dir->x != 0 && dir->y != 0) {
		/*
		 * Take a diagonal step upon leaving the wall.  Proceed to that.
		 */
		*grid = loc_sum(*grid, *dir);
		assert(!square_is_granite_with_flag(c, *grid, SQUARE_WALL_OUTER) &&
			!square_is_granite_with_flag(c, *grid, SQUARE_WALL_SOLID) &&
			!square_is_granite_with_flag(c, *grid, SQUARE_WALL_INNER) &&
			!square_isperm(c, *grid));

		if (!square_isroom(c, *grid) && square_isgranite(c, *grid)) {
			/* Save the tunnel location */
			if (dun->tunn_n < z_info->tunn_grid_max) {
				dun->tunn[dun->tunn_n] = *grid;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			*door_flag = false;
		}

		/*
		 * Having pierced the wall and taken a step, can forget about
		 * what was set to suppress bends in the past.
		 */
		*bend_intvl = 0;

		/*
		 * Now choose a cardinal direction, one that is +/-45 degrees
		 * from what was used for the diagonal step, for the next step
		 * since the tunnel iterations want a cardinal direction.
		 */
		if (randint0(32768) < 16384) {
			dir->x = 0;
		} else {
			dir->y = 0;
		}
	} else {
		/*
		 * Take a cardinal step upon leaving the wall.  Most of the
		 * passed in state is fine, but temporarily suppress bends so
		 * the step will be handled as is by the next iteration of
		 * tunnel building.
		 */
		*bend_intvl = 1;
	}
}


/**
 * Help build_tunnel():  choose a direction that is approximately normal to a
 * room's wall.
 * \param c Is the chunk to use.
 * \param grid Is a location on the wall.
 * \param inner If true, return a direction that points to the interior of the
 * room.  Otherwise, return a direction pointing to the exterior.
 * \return The returned value is the chosen direction.  It may be loc(0, 0)
 * if no feasible direction could be found.
 */
static struct loc find_normal_to_wall(struct chunk *c, struct loc grid,
	bool inner)
{
	int n = 0, ncardinal = 0, i;
	struct loc choices[8];

	assert(square_is_granite_with_flag(c, grid, SQUARE_WALL_OUTER) ||
		square_is_granite_with_flag(c, grid, SQUARE_WALL_SOLID));
	/* Relies on the cardinal directions being first in ddgrid_ddd. */
	for (i = 0; i < 8; ++i) {
		struct loc chk = loc_sum(grid, ddgrid_ddd[i]);

		if (square_in_bounds(c, chk) &&
			!square_isperm(c, chk) &&
			(square_isroom(c, chk) == inner) &&
			!square_is_granite_with_flag(c, chk, SQUARE_WALL_OUTER) &&
			!square_is_granite_with_flag(c, chk, SQUARE_WALL_SOLID) &&
			!square_is_granite_with_flag(c, chk, SQUARE_WALL_INNER)) {
			choices[n] = ddgrid_ddd[i];
			++n;
			if (i < 4) {
				++ncardinal;
			}
		}
	}
	/* Prefer a cardinal direction if available. */
	if (n > 1 && ncardinal > 0) {
		n = ncardinal;
	}
	return (n == 0) ? loc(0, 0) : choices[randint0(n)];
}


/**
 * Help build_tunnel():  test if a wall-piercing location can have a door.
 * Don't want a door that's only adjacent to terrain that is either
 * 1) not passable and not rubble
 * 2) a door (treat a shop like a door)
 * on either the side facing outside the room or the side facing the room.
 * \param c Is the chunk to use.
 * \param grid Is the location of the wall piercing.
 */
static bool allows_wall_piercing_door(struct chunk *c, struct loc grid)
{
	struct loc chk;
	int n_outside_good = 0;
	int n_inside_good = 0;

	for (chk.y = grid.y - 1; chk.y <= grid.y + 1; ++chk.y) {
		for (chk.x = grid.x - 1; chk.x <= grid.x + 1; ++chk.x) {
			if ((chk.y == 0 && chk.x == 0) ||
					!square_in_bounds(c, chk)) continue;
			if ((square_ispassable(c, chk) ||
					square_isrubble(c, chk)) &&
					!square_isdoor(c, chk) &&
					!square_isshop(c, chk)) {
				if (square_isroom(c, chk)) {
					++n_inside_good;
				} else {
					++n_outside_good;
				}
			}
		}
	}
	return n_outside_good > 0 && n_inside_good > 0;
}


/**
 * Constructs a tunnel between two points
 *
 * \param c is the current chunk
 * \param grid1 is the location of the first point
 * \param grid2 is the location of the second point
 *
 * This function must be called BEFORE any streamers are created, since we use
 * granite with the special SQUARE_WALL flags to keep track of legal places for
 * corridors to pierce rooms.
 *
 * Locations to excavate are queued and applied afterward.  The wall piercings
 * are also queued but the outer wall grids adjacent to the piercing are marked
 * right away to prevent adjacent piercings.  That makes testing where to
 * pierce easier (look at grid flags rather than search through the queued
 * piercings).
 *
 * The solid wall check prevents silly door placement and excessively wide
 * room entrances.
 */
static void build_tunnel(struct chunk *c, struct loc grid1, struct loc grid2)
{
	int i;
	int main_loop_count = 0;
	struct loc start = grid1, tmp_grid, offset;
	/* Used to prevent random bends for a while. */
	int bend_intvl = 0;
	/*
	 * Used to prevent excessive door creation along overlapping corridors.
	 */
	bool door_flag = false;

	/* Reset the arrays */
	dun->tunn_n = 0;
	dun->wall_n = 0;

	/* Start out in the correct direction */
	correct_dir(&offset, grid1, grid2);

	/* Keep going until done (or bored) */
	while (!loc_eq(grid1, grid2)) {
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) break;

		/* Allow bends in the tunnel */
		if (bend_intvl == 0) {
			if (randint0(100) < dun->profile->tun.chg) {
				/* Get the correct direction */
				correct_dir(&offset, grid1, grid2);

				/* Random direction */
				if (randint0(100) < dun->profile->tun.rnd)
					rand_dir(&offset);
			}
		} else {
			assert(bend_intvl > 0);
			--bend_intvl;
		}

		/* Get the next location */
		tmp_grid = loc_sum(grid1, offset);

		while (!square_in_bounds(c, tmp_grid)) {
			/* Get the correct direction */
			correct_dir(&offset, grid1, grid2);

			/* Random direction */
			if (randint0(100) < dun->profile->tun.rnd)
				rand_dir(&offset);

			/* Get the next location */
			tmp_grid = loc_sum(grid1, offset);
		}

		/* Avoid obstacles */
		if ((square_isperm(c, tmp_grid) && !sqinfo_has(square(c,
				tmp_grid)->info, SQUARE_WALL_INNER)) ||
				square_is_granite_with_flag(c, tmp_grid,
				SQUARE_WALL_SOLID)) {
			continue;
		}

		/* Pierce "outer" walls of rooms */
		if (square_is_granite_with_flag(c, tmp_grid, SQUARE_WALL_OUTER)) {
			int iroom;
			struct loc nxtdir = loc_diff(grid2, tmp_grid);

			/* If it's the goal, accept and pierce the wall. */
			if (nxtdir.x == 0 && nxtdir.y == 0) {
				grid1 = tmp_grid;
				pierce_outer_wall(c, grid1);
				continue;
			}
			/*
			 * If it's adjacent to the goal and that is also an
			 * outer wall, then can't pierce without making the
			 * goal unreachable.
			 */
			if (ABS(nxtdir.x) <= 1 && ABS(nxtdir.y) <= 1 &&
					square_is_granite_with_flag(c, grid2,
					SQUARE_WALL_OUTER)) {
				continue;
			}
			/* See if it is a marked entrance. */
			iroom = dun->ent2room[tmp_grid.y][tmp_grid.x];
			if (iroom != -1) {
				/* It is. */
				assert(iroom >= 0 && iroom < dun->cent_n);
				if (square_isroom(c, grid1)) {
					/*
					 * The tunnel is coming from inside the
					 * room.  See if there's somewhere on
					 * the outside to go.
					 */
					nxtdir = find_normal_to_wall(c,
						tmp_grid, false);
					if (nxtdir.x == 0 && nxtdir.y == 0) {
						/* There isn't. */
						continue;
					}
					/*
					 * There is.  Accept the grid and pierce
					 * the wall.
					 */
					grid1 = tmp_grid;
					pierce_outer_wall(c, grid1);
				} else {
					/*
					 * The tunnel is coming from outside the
					 * room.  Choose an entrance (perhaps
					 * the same as the one just entered) to
					 * use as the exit.  Crudely adjust how
					 * biased the entrance selection is
					 * based on how often random steps are
					 * taken while tunneling.  The rationale
					 * for a maximum bias of 80 is similar
					 * to that in
					 * do_traditional_tunneling().
					 */
					int bias = 80 - ((80 *
						MIN(MAX(0, dun->profile->tun.chg), 100) *
						MIN(MAX(0, dun->profile->tun.rnd), 100)) /
						10000);
					int ntry = 0, mtry = 20;
					struct loc exc[2] = { tmp_grid, grid2 };
					struct loc chk = loc(0, 0);

					while (1) {
						if (ntry >= mtry) {
							/*
							 * Didn't find a usable
							 * exit.
							 */
							break;
						}
						chk = choose_random_entrance(
							c, iroom, &grid2, bias,
							exc, 2);
						if (chk.x == 0 && chk.y == 0) {
							/* No exits at all. */
							ntry = mtry;
							break;
						}
						nxtdir = find_normal_to_wall(
							c, chk, false);
						if (nxtdir.x != 0 ||
								nxtdir.y != 0) {
							/*
							 * Found a usable exit.
							 */
							break;
						}
						++ntry;
						/* Also make it less biased. */
						bias = (bias * 8) / 10;
					}
					if (ntry >= mtry) {
						/* No usable exit was found. */
						continue;
					}
					/*
					 * Pierce the wall at the original
					 * entrance.
					 */
					pierce_outer_wall(c, tmp_grid);
					/*
					 * And at the exit which is also the
					 * continuation point for the rest of
					 * the tunnel.
					 */
					pierce_outer_wall(c, chk);
					grid1 = chk;
				}
				offset = nxtdir;
				handle_post_wall_step(c, &grid1, &offset,
					&door_flag, &bend_intvl);
				continue;
			}

			/* Is there a feasible location after the wall? */
			nxtdir = find_normal_to_wall(c, tmp_grid,
				!square_isroom(c, grid1));

			if (nxtdir.x == 0 && nxtdir.y == 0) {
				/* There's no feasible location. */
				continue;
			}

			/* Accept the location and pierce the wall. */
			grid1 = tmp_grid;
			pierce_outer_wall(c, grid1);
			offset = nxtdir;
			handle_post_wall_step(c, &grid1, &offset, &door_flag,
				&bend_intvl);
		} else if (square_isroom(c, tmp_grid)) {
			/* Travel quickly through rooms */

			/* Accept the location */
			grid1 = tmp_grid;
		} else if (square_isgranite(c, tmp_grid)) {
			/* Tunnel through all other walls */

			/* Accept this location */
			grid1 = tmp_grid;

			/* Save the tunnel location */
			if (dun->tunn_n < z_info->tunn_grid_max) {
				dun->tunn[dun->tunn_n] = grid1;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			door_flag = false;
		} else {
			/* Handle corridor intersections or overlaps */

			assert(square_in_bounds_fully(c, tmp_grid));

			/* Accept the location */
			grid1 = tmp_grid;

			/* Collect legal door locations */
			if (!door_flag) {
				/* Save the door location */
				if (dun->door_n < z_info->level_door_max) {
					dun->door[dun->door_n] = grid1;
					dun->door_n++;
				}

				/* No door in next grid */
				door_flag = true;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if (randint0(100) >= dun->profile->tun.con) {
				/* Offset between grid1 and start */
				tmp_grid = loc_diff(grid1, start);

				/* Terminate the tunnel if too far vertically or horizontally */
				if ((ABS(tmp_grid.x) > 10) || (ABS(tmp_grid.y) > 10)) break;
			}
		}
	}

	/* Turn the tunnel into corridor */
	for (i = 0; i < dun->tunn_n; i++) {
		/* Clear previous contents, add a floor */
		square_set_feat(c, dun->tunn[i], FEAT_FLOOR);
	}

	/* Apply the piercings that we found */
	for (i = 0; i < dun->wall_n; i++) {
		/* Convert to floor grid */
		square_set_feat(c, dun->wall[i], FEAT_FLOOR);

		/* Place a random door */
		if (randint0(100) < dun->profile->tun.pen &&
				allows_wall_piercing_door(c, dun->wall[i]))
			place_random_door(c, dun->wall[i]);
	}
}

/**
 * Count the number of corridor grids adjacent to the given grid.
 *
 * This routine currently only counts actual "empty floor" grids which are not
 * in rooms.
 * \param c is the current chunk
 * \param y1 are the co-ordinates
 * \param x1 are the co-ordinates
 *
 * TODO: count stairs, open doors, closed doors?
 */
static int next_to_corr(struct chunk *c, struct loc grid)
{
	int i, k = 0;
	assert(square_in_bounds(c, grid));

	/* Scan adjacent grids */
	for (i = 0; i < 4; i++) {
		/* Extract the location */
		struct loc grid1 = loc_sum(grid, ddgrid_ddd[i]);

		/* Count only floors which aren't part of rooms */
		if (square_isfloor(c, grid1) && !square_isroom(c, grid1)) k++;
	}

	/* Return the number of corridors */
	return k;
}

/**
 * Returns whether a doorway can be built in a space.
 * \param c is the current chunk
 * \param y are the co-ordinates
 * \param x are the co-ordinates
 *
 * To have a doorway, a space must be adjacent to at least two corridors and be
 * between two walls.
 */
static bool possible_doorway(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds(c, grid));
	if (next_to_corr(c, grid) < 2)
		return false;
	else if (square_isstrongwall(c, next_grid(grid, DIR_N)) &&
			 square_isstrongwall(c, next_grid(grid, DIR_S)))
		return true;
	else if (square_isstrongwall(c, next_grid(grid, DIR_W)) &&
			 square_isstrongwall(c, next_grid(grid, DIR_E)))
		return true;
	else
		return false;
}


/**
 * Places door or trap at y, x position if at least 2 walls found
 * \param c is the current chunk
 * \param y are the co-ordinates
 * \param x are the co-ordinates
 */
static void try_door(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds(c, grid));

	if (square_isstrongwall(c, grid)) return;
	if (square_isroom(c, grid)) return;
	if (square_isplayertrap(c, grid)) return;
	if (square_isdoor(c, grid)) return;

	if (randint0(100) < dun->profile->tun.jct && possible_doorway(c, grid))
		place_random_door(c, grid);
	else if (randint0(500) < dun->profile->tun.jct && possible_doorway(c, grid))
		place_trap(c, grid, -1, c->depth);
}


/**
 * Connect the rooms with tunnels in the traditional fashion.
 * \param c Is the chunk to use.
 */
static void do_traditional_tunneling(struct chunk *c)
{
	int *scrambled = mem_alloc(dun->cent_n * sizeof(*scrambled));
	int i;
	struct loc grid;

	/*
	 * Scramble the order in which the rooms will be connected.  Use
	 * indirect indexing so dun->ent2room can be left as it is.
	 */
	for (i = 0; i < dun->cent_n; ++i) {
		scrambled[i] = i;
	}
	for (i = 0; i < dun->cent_n; ++i) {
		int pick1 = randint0(dun->cent_n);
		int pick2 = randint0(dun->cent_n);
		int tmp = scrambled[pick1];

		scrambled[pick1] = scrambled[pick2];
		scrambled[pick2] = tmp;
	}

	/* Start with no tunnel doors. */
	dun->door_n = 0;

	/*
	 * Link the rooms in the scrambled order with the first connecting to
	 * the last.  The bias argument for choose_random_entrance() was
	 * somewhat arbitrarily chosen:  i.e. if the room is more than a
	 * typical screen width away, don't particularly care which entrance is
	 * selected.
	 */
	grid = choose_random_entrance(c, scrambled[dun->cent_n - 1], NULL, 80,
		NULL, 0);
	if (grid.x == 0 && grid.y == 0) {
		/* Use the room's center. */
		grid = dun->cent[scrambled[dun->cent_n - 1]];
	}
	for (i = 0; i < dun->cent_n; ++i) {
		struct loc next_grid = choose_random_entrance(c, scrambled[i],
			&grid, 80, NULL, 0);

		if (next_grid.x == 0 && next_grid.y == 0) {
			next_grid = dun->cent[scrambled[i]];
		}
		build_tunnel(c, next_grid, grid);

		/* Remember the "previous" room. */
		grid = next_grid;
	}

	mem_free(scrambled);

	/* Place intersection doors. */
	for (i = 0; i < dun->door_n; ++i) {
		/* Try placing doors. */
		try_door(c, next_grid(dun->door[i], DIR_W));
		try_door(c, next_grid(dun->door[i], DIR_E));
		try_door(c, next_grid(dun->door[i], DIR_N));
		try_door(c, next_grid(dun->door[i], DIR_S));
	}
}


/**
 * Generate a new dungeon level.
 * \param p is the player 
 * \return a pointer to the generated chunk
 */
struct chunk *classic_gen(struct player *p, int min_height, int min_width) {
	int i, j, k;
	int by, bx = 0, tby, tbx, key, rarity, built;
	int num_rooms, size_percent;
	int dun_unusual = dun->profile->dun_unusual;

	bool **blocks_tried;
	struct chunk *c;

	/* No persistent levels of this type for now */
	if (OPT(p, birth_levels_persist)) return NULL;

	/* This code currently does nothing - see comments below */
	i = randint1(10) + p->depth / 24;
	if (is_quest(p->depth)) size_percent = 100;
	else if (i < 2) size_percent = 75;
	else if (i < 3) size_percent = 80;
	else if (i < 4) size_percent = 85;
	else if (i < 5) size_percent = 90;
	else if (i < 6) size_percent = 95;
	else size_percent = 100;

	/* scale the various generation variables */
	num_rooms = (dun->profile->dun_rooms * size_percent) / 100;
	dun->block_hgt = dun->profile->block_size;
	dun->block_wid = dun->profile->block_size;
	c = cave_new(z_info->dungeon_hgt, z_info->dungeon_wid);
	c->depth = p->depth;
	ROOM_LOG("height=%d  width=%d  nrooms=%d", c->height, c->width, num_rooms);

	/* Fill cave area with basic granite */
	fill_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_GRANITE, SQUARE_NONE);

	/* Actual maximum number of rooms on this level */
	dun->row_blocks = c->height / dun->block_hgt;
	dun->col_blocks = c->width / dun->block_wid;

	/* Initialize the room table */
	dun->room_map = mem_zalloc(dun->row_blocks * sizeof(bool*));
	for (i = 0; i < dun->row_blocks; i++)
		dun->room_map[i] = mem_zalloc(dun->col_blocks * sizeof(bool));

	/* Initialize the block table */
	blocks_tried = mem_zalloc(dun->row_blocks * sizeof(bool*));

	for (i = 0; i < dun->row_blocks; i++)
		blocks_tried[i] = mem_zalloc(dun->col_blocks * sizeof(bool));

	/* No rooms yet, pits or otherwise. */
	dun->pit_num = 0;
	dun->cent_n = 0;
	reset_entrance_data(c);

	/* Build some rooms.  Note that the theoretical maximum number of rooms
	 * in this profile is currently 36, so built never reaches num_rooms,
	 * and room generation is always terminated by having tried all blocks */
	built = 0;
	while(built < num_rooms) {

		/* Count the room blocks we haven't tried yet. */
		j = 0;
		tby = 0;
		tbx = 0;
		for(by = 0; by < dun->row_blocks; by++) {
			for(bx = 0; bx < dun->col_blocks; bx++) {
				if (blocks_tried[by][bx]) continue;
				j++;
				if (one_in_(j)) {
					tby = by;
					tbx = bx;
				}
			} 
		}
		bx = tbx;
		by = tby;

		/* If we've tried all blocks we're done. */
		if (j == 0) break;

		if (blocks_tried[by][bx]) quit_fmt("generation: inconsistent blocks");

		/* Mark that we are trying this block. */
		blocks_tried[by][bx] = true;

		/* Roll for random key (to be compared against a profile's cutoff) */
		key = randint0(100);

		/* We generate a rarity number to figure out how exotic to make
		 * the room. This number has a (50+depth/2)/DUN_UNUSUAL chance
		 * of being > 0, a (50+depth/2)^2/DUN_UNUSUAL^2 chance of
		 * being > 1, up to MAX_RARITY. */
		i = 0;
		rarity = 0;
		while (i == rarity && i < dun->profile->max_rarity) {
			if (randint0(dun_unusual) < 50 + c->depth / 2) rarity++;
			i++;
		}

		/* Once we have a key and a rarity, we iterate through out list of
		 * room profiles looking for a match (whose cutoff > key and whose
		 * rarity > this rarity). We try building the room, and if it works
		 * then we are done with this iteration. We keep going until we find
		 * a room that we can build successfully or we exhaust the profiles. */
		for (i = 0; i < dun->profile->n_room_profiles; i++) {
			struct room_profile profile = dun->profile->room_profiles[i];
			if (profile.rarity > rarity) continue;
			if (profile.cutoff <= key) continue;
			
			if (room_build(c, by, bx, profile, false)) {
				built++;
				break;
			}
		}
	}

	for (i = 0; i < dun->row_blocks; i++){
		mem_free(blocks_tried[i]);
		mem_free(dun->room_map[i]);
	}
	mem_free(blocks_tried);
	mem_free(dun->room_map);

	/* Generate permanent walls around the edge of the generated area */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_PERM, SQUARE_NONE, true);

	/* Connect all the rooms together */
	do_traditional_tunneling(c);
	ensure_connectedness(c, true);

	/* Add some magma streamers */
	for (i = 0; i < dun->profile->str.mag; i++)
		build_streamer(c, FEAT_MAGMA, dun->profile->str.mc);

	/* Add some quartz streamers */
	for (i = 0; i < dun->profile->str.qua; i++)
		build_streamer(c, FEAT_QUARTZ, dun->profile->str.qc);

	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(3, 4));

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2));

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon, reduce frequency by factor of 5 */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k)/5, c->depth, 0);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = z_info->level_monster_min + randint1(8) + k;

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		pick_and_place_distant_monster(c, p, 0, true, c->depth);

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT,
		Rand_normal(z_info->room_item_av, 3), c->depth, ORIGIN_FLOOR);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT,
		Rand_normal(z_info->both_item_av, 3), c->depth, ORIGIN_FLOOR);
	alloc_objects(c, SET_BOTH, TYP_GOLD,
		Rand_normal(z_info->both_gold_av, 3), c->depth, ORIGIN_FLOOR);

	return c;
}


/* ------------------ LABYRINTH ---------------- */

/**
 * Given an adjoining wall (a wall which separates two labyrinth cells)
 * set a and b to point to the cell indices which are separated. Used by
 * labyrinth_gen().
 * \param i is the wall index
 * \param w is the width of the labyrinth
 * \param a are the two cell indices
 * \param b are the two cell indices
 */
static void lab_get_adjoin(int i, int w, int *a, int *b) {
	struct loc grid;
	i_to_grid(i, w, &grid);
	if (grid.x % 2 == 0) {
		*a = grid_to_i(next_grid(grid, DIR_N), w);
		*b = grid_to_i(next_grid(grid, DIR_S), w);
	} else {
		*a = grid_to_i(next_grid(grid, DIR_W), w);
		*b = grid_to_i(next_grid(grid, DIR_E), w);
	}
}

/**
 * Return whether a grid is in a tunnel.
 *
 * \param c is the current chunk
 * \param grid is the location
 *
 * For our purposes a tunnel is a horizontal or vertical path, not an
 * intersection. Thus, we want the squares on either side to walls in one
 * case (e.g. up/down) and open in the other case (e.g. left/right). We don't
 * want a square that represents an intersection point.  Treat doors the same
 * as open floors in the tests since doors may replace a floor but not a wall.
 *
 * The high-level idea is that these are squares which can't be avoided (by
 * walking diagonally around them).
 */
static bool lab_is_tunnel(struct chunk *c, struct loc grid) {
	bool west = square_ispassable(c, next_grid(grid, DIR_W)) ||
		square_iscloseddoor(c, next_grid(grid, DIR_W));
	bool east = square_ispassable(c, next_grid(grid, DIR_E)) ||
		square_iscloseddoor(c, next_grid(grid, DIR_E));
	bool north = square_ispassable(c, next_grid(grid, DIR_N)) ||
		square_iscloseddoor(c, next_grid(grid, DIR_N));
	bool south = square_ispassable(c, next_grid(grid, DIR_S)) ||
		square_iscloseddoor(c, next_grid(grid, DIR_S));

	return north == south && west == east && north != west;
}


/**
 * Build a labyrinth chunk of a given height and width
 *
 * \param depth is the native depth 
 * \param h are the dimensions of the chunk
 * \param w are the dimensions of the chunk
 * \param lit is whether the labyrinth is lit
 * \param soft is true if we use regular walls, false if permanent walls
 * \return a pointer to the generated chunk
 */
static struct chunk *labyrinth_chunk(int depth, int h, int w, bool lit, bool soft)
{
	int i, j, k;
	struct loc grid;

	/* This is the number of squares in the labyrinth */
	int n = h * w;

	/* NOTE: 'sets' and 'walls' are too large... we only need to use about
	 * 1/4 as much memory. However, in that case, the addressing math
	 * becomes a lot more complicated, so let's just stick with this
	 * because it's easier to read. */

	/* 'sets' tracks connectedness; if sets[i] == sets[j] then cells i and j
	 * are connected to each other in the maze. */
	int *sets;

	/* 'walls' is a list of wall coordinates which we will randomize */
	int *walls;

	/* The labyrinth chunk */
	struct chunk *c = cave_new(h + 2, w + 2);
	c->depth = depth;
	/* allocate our arrays */
	sets = mem_zalloc(n * sizeof(int));
	walls = mem_zalloc(n * sizeof(int));

	/* Bound with perma-rock */
	draw_rectangle(c, 0, 0, h + 1, w + 1, FEAT_PERM, SQUARE_NONE, true);

	/* Fill the labyrinth area with rock */
	if (soft)
		fill_rectangle(c, 1, 1, h, w, FEAT_GRANITE, SQUARE_WALL_SOLID);
	else
		fill_rectangle(c, 1, 1, h, w, FEAT_PERM, SQUARE_NONE);

	/* Initialize each wall. */
	for (i = 0; i < n; i++) {
		walls[i] = i;
		sets[i] = -1;
	}

	/* Cut out a grid of 1x1 rooms which we will call "cells" */
	for (grid.y = 0; grid.y < h; grid.y += 2) {
		for (grid.x = 0; grid.x < w; grid.x += 2) {
			int k_local = grid_to_i(grid, w);
			struct loc diag = next_grid(grid, DIR_SE);
			sets[k_local] = k_local;
			square_set_feat(c, diag, FEAT_FLOOR);
			if (lit) sqinfo_on(square(c, diag)->info, SQUARE_GLOW);
		}
	}

	/* Shuffle the walls, using Knuth's shuffle. */
	shuffle(walls, n);

	/* For each adjoining wall, look at the cells it divides. If they aren't
	 * in the same set, remove the wall and join their sets.
	 *
	 * This is a randomized version of Kruskal's algorithm. */
	for (i = 0; i < n; i++) {
		int a, b;

		j = walls[i];

		/* If this cell isn't an adjoining wall, skip it */
		i_to_grid(j, w, &grid);
		if ((grid.x < 1 && grid.y < 1) || (grid.x > w - 2 && grid.y > h - 2))
			continue;
		if (grid.x % 2 == grid.y % 2) continue;

		/* Figure out which cells are separated by this wall */
		lab_get_adjoin(j, w, &a, &b);

		/* If the cells aren't connected, kill the wall and join the sets */
		if (sets[a] != sets[b]) {
			int sa = sets[a];
			int sb = sets[b];
			square_set_feat(c, next_grid(grid, DIR_SE), FEAT_FLOOR);
			if (lit) {
				sqinfo_on(square(c, next_grid(grid, DIR_SE))->info, SQUARE_GLOW);
			}
			for (k = 0; k < n; k++) {
				if (sets[k] == sb) sets[k] = sa;
			}
		}
	}

	/* Generate a door for every 100 squares in the labyrinth */
	for (i = n / 100; i > 0; i--) {
		/* Try 10 times to find a useful place for a door, then place it */
		for (j = 0; j < 10; j++) {
			find_empty(c, &grid);
			if (lab_is_tunnel(c, grid)) break;

		}
		place_closed_door(c, grid);
	}

	/* Unlit labyrinths will have some good items */
	if (!lit)
		alloc_objects(c, SET_BOTH, TYP_GOOD, Rand_normal(3, 2),
			c->depth, ORIGIN_LABYRINTH);

	/* Hard (non-diggable) labyrinths will have some great items */
	if (!soft)
		alloc_objects(c, SET_BOTH, TYP_GREAT, Rand_normal(2, 1),
			c->depth, ORIGIN_LABYRINTH);

	/* Deallocate our lists */
	mem_free(sets);
	mem_free(walls);

	return c;
}

/**
 * Build a labyrinth level.
 * \param p is the player
 * Note that if the function returns false, a level wasn't generated.
 * Labyrinths use the dungeon level's number to determine whether to generate
 * themselves (which means certain level numbers are more likely to generate
 * labyrinths than others).
 */
struct chunk *labyrinth_gen(struct player *p, int min_height, int min_width) {
	int i, k;
	struct chunk *c;
	struct loc grid;
	/* Size of the actual labyrinth part must be odd. */
	/* NOTE: these are not the actual dungeon size, but rather the size of
	 * the area we're generating a labyrinth in (which doesn't count the
	 * enclosing outer walls. */
	int h = 15 + randint0(p->depth / 10) * 2;
	int w = 51 + randint0(p->depth / 10) * 2;

	/* Most labyrinths are lit */
	bool lit = randint0(p->depth) < 25 || randint0(2) < 1;

	/* Many labyrinths are known */
	bool known = lit && randint0(p->depth) < 25;

	/* Most labyrinths have soft (diggable) walls */
	bool soft = randint0(p->depth) < 35 || randint0(3) < 2;

	/* No persistent levels of this type for now */
	if (OPT(p, birth_levels_persist)) return NULL;

	/* Enforce minimum dimensions */
	h = MAX(h, min_height);
	w = MAX(w, min_width);

	/* Generate the actual labyrinth */
	c = labyrinth_chunk(p->depth, h, w, lit, soft);
	if (!c) return NULL;
	c->depth = p->depth;

	/* Determine the character location */
	new_player_spot(c, p);

	/* Generate a single set of stairs up if necessary. */
	if (!cave_find(c, &grid, square_isupstairs))
		alloc_stairs(c, FEAT_LESS, 1);

	/* Generate a single set of stairs down if necessary. */
	if (!cave_find(c, &grid, square_isdownstairs))
		alloc_stairs(c, FEAT_MORE, 1);

	/* General some rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Scale number of monsters items by labyrinth size */
	k = (3 * k * (h * w)) / (z_info->dungeon_hgt * z_info->dungeon_wid);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k), c->depth, 0);

	/* Put some monsters in the dungeon */
	for (i = z_info->level_monster_min + randint1(8) + k; i > 0; i--)
		pick_and_place_distant_monster(c, p, 0, true, c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(k * 6, 2), c->depth,
		ORIGIN_LABYRINTH);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(k * 3, 2), c->depth,
		ORIGIN_LABYRINTH);
	alloc_objects(c, SET_BOTH, TYP_GOOD, randint1(2), c->depth,
		ORIGIN_LABYRINTH);

	/* Notify if we want the player to see the maze layout */
	if (known) {
		player->upkeep->light_level = true;
	}

	return c;
}


/* ---------------- CAVERNS ---------------------- */

/**
 * Initialize the dungeon array, with a random percentage of squares open.
 * \param c is the current chunk
 * \param density is the percentage of floors we are aiming for
 */
static void init_cavern(struct chunk *c, int density) {
	int h = c->height;
	int w = c->width;
	int size = h * w;
	
	int count = (size * density) / 100;

	/* Fill the entire chunk with rock */
	fill_rectangle(c, 0, 0, h - 1, w - 1, FEAT_GRANITE, SQUARE_WALL_SOLID);
	
	while (count > 0) {
		struct loc grid = loc(randint1(w - 2), randint1(h - 2));
		if (square_isrock(c, grid)) {
			square_set_feat(c, grid, FEAT_FLOOR);
			count--;
		}
	}
}

/**
 * Return the number of walls (0-8) adjacent to this square.
 * \param c is the current chunk
 * \param y are the co-ordinates
 * \param x are the co-ordinates
 */
static int count_adj_walls(struct chunk *c, struct loc grid) {
	int d;
	int count = 0;

	for (d = 0; d < 8; d++) {
		if (square_isfloor(c, loc_sum(grid, ddgrid_ddd[d]))) continue;
		count++;
	}

	return count;
}

/**
 * Run a single pass of the cellular automata rules (4,5) on the dungeon.
 * \param c is the chunk being mutated
 */
static void mutate_cavern(struct chunk *c) {
	struct loc grid;
	int h = c->height;
	int w = c->width;

	int *temp = mem_zalloc(h * w * sizeof(int));

	for (grid.y = 1; grid.y < h - 1; grid.y++) {
		for (grid.x = 1; grid.x < w - 1; grid.x++) {
			int count = count_adj_walls(c, grid);
			if (count > 5)
				temp[grid_to_i(grid, w)] = FEAT_GRANITE;
			else if (count < 4)
				temp[grid_to_i(grid, w)] = FEAT_FLOOR;
			else
				temp[grid_to_i(grid, w)] = square(c, grid)->feat;
		}
	}

	for (grid.y = 1; grid.y < h - 1; grid.y++) {
		for (grid.x = 1; grid.x < w - 1; grid.x++) {
			if (temp[grid_to_i(grid, w)] == FEAT_GRANITE)
				set_marked_granite(c, grid, SQUARE_WALL_SOLID);
			else
				square_set_feat(c, grid, temp[grid_to_i(grid, w)]);
		}
	}

	mem_free(temp);
}

/**
 * Fill an int[] with a single value.
 * \param data is the array
 * \param value is what it's being filled with
 * \param size is the array length
 */
static void array_filler(int data[], int value, int size) {
	int i;
	for (i = 0; i < size; i++) data[i] = value;
}

/**
 * Determine if we need to worry about coloring a point, or can ignore it.
 * \param c is the current chunk
 * \param colors is the array of current point colors
 * \param y are the co-ordinates
 * \param x are the co-ordinates
 */
static int ignore_point(struct chunk *c, int colors[], struct loc grid) {
	int n = grid_to_i(grid, c->width);

	if (!square_in_bounds(c, grid)) return true;
	if (colors[n]) return true;
	if (square_ispassable(c, grid)) return false;
	if (square_isdoor(c, grid)) return false;
	return true;
}

/**
 * Color a particular point, and all adjacent points.
 * \param c is the current chunk
 * \param colors is the array of current point colors
 * \param counts is the array of current color counts
 * \param grid is the location
 * \param color is the color we are coloring
 * \param diagonal controls whether we can progress diagonally
 */
static void build_color_point(struct chunk *c, int colors[], int counts[],
		struct loc grid, int color, bool diagonal) {
	int h = c->height;
	int w = c->width;
	int size = h * w;
	struct queue *queue = q_new(size);

	int *added = mem_zalloc(size * sizeof(int));

	array_filler(added, 0, size);

	q_push_int(queue, grid_to_i(grid, w));

	counts[color] = 0;

	while (q_len(queue) > 0) {
		int i;
		struct loc grid1;
		int n1 = q_pop_int(queue);

		i_to_grid(n1, w, &grid1);

		if (ignore_point(c, colors, grid1)) continue;

		colors[n1] = color;
		counts[color]++;

		for (i = 0; i < (diagonal ? 8 : 4); i++) {
			struct loc grid2 = loc_sum(grid1, ddgrid_ddd[i]);
			int n2 = grid_to_i(grid2, w);
			if (ignore_point(c, colors, grid2)) continue;
			if (added[n2]) continue;

			q_push_int(queue, n2);
			added[n2] = 1;
		}
	}

	mem_free(added);
	q_free(queue);
}

/**
 * Create a color for each "NESW contiguous" region of the dungeon.
 * \param c is the current chunk
 * \param colors is the array of current point colors
 * \param counts is the array of current color counts
 * \param diagonal controls whether we can progress diagonally
 */
static void build_colors(struct chunk *c, int colors[], int counts[], bool diagonal) {
	int y, x;
	int h = c->height;
	int w = c->width;
	int color = 1;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (ignore_point(c, colors, loc(x, y))) continue;
			build_color_point(c, colors, counts, loc(x, y), color, diagonal);
			color++;
		}
	}
}

/**
 * Find and delete all small (<9 square) open regions.
 * \param c is the current chunk
 * \param colors is the array of current point colors
 * \param counts is the array of current color counts
 */
static void clear_small_regions(struct chunk *c, int colors[], int counts[]) {
	int i, y, x;
	int h = c->height;
	int w = c->width;
	int size = h * w;

	int *deleted = mem_zalloc(size * sizeof(int));
	array_filler(deleted, 0, size);

	for (i = 0; i < size; i++) {
		if (counts[i] < 9) {
			deleted[i] = 1;
			counts[i] = 0;
		}
	}

	for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			struct loc grid = loc(x, y);
			i = grid_to_i(grid, w);

			if (!deleted[colors[i]]) continue;

			colors[i] = 0;
			set_marked_granite(c, grid, SQUARE_WALL_SOLID);
		}
	}
	mem_free(deleted);
}

/**
 * Return the number of colors which have active cells.
 * \param counts is the array of current color counts
 * \param size is the total area
 */
static int count_colors(int counts[], int size) {
	int i;
	int num = 0;
	for (i = 0; i < size; i++) if (counts[i] > 0) num++;
	return num;
}

/**
 * Return the first color which has one or more active cells.
 * \param counts is the array of current color counts
 * \param size is the total area
 */
static int first_color(int counts[], int size) {
	int i;
	for (i = 0; i < size; i++) if (counts[i] > 0) return i;
	return -1;
}

/**
 * Find all cells of 'fromcolor' and repaint them to 'tocolor'.
 * \param colors is the array of current point colors
 * \param counts is the array of current color counts
 * \param from is the color to change
 * \param to is the color to change to
 * \param size is the total area
 */
static void fix_colors(int colors[], int counts[], int from, int to, int size) {
	int i;
	for (i = 0; i < size; i++) if (colors[i] == from) colors[i] = to;
	counts[to] += counts[from];
	counts[from] = 0;
}

/**
 * Create a tunnel connecting a region to one of its nearest neighbors.
 * Set new_color = -1 for any neighbour, the required color for a specific one
 * \param c is the current chunk
 * \param colors is the array of current point colors
 * \param counts is the array of current color counts
 * \param color is the color of the region we want to connect
 * \param new_color is the color of the region we want to connect to (if used)
 * \param allow_vault_disconnect If true, vaults can be included in path
 * planning which can leave regions disconnected.
 */
static void join_region(struct chunk *c, int colors[], int counts[], int color,
	int new_color, bool allow_vault_disconnect)
{
	int i;
	int h = c->height;
	int w = c->width;
	int size = h * w;

	/* Allocate a processing queue */
	struct queue *queue = q_new(size);

	/* Allocate an array to keep track of handled squares, and which square
	 * we reached them from.
	 */
	int *previous = mem_zalloc(size * sizeof(int));
	array_filler(previous, -1, size);

	/* Push all squares of the given color onto the queue */
	for (i = 0; i < size; i++) {
		if (colors[i] == color) {
			q_push_int(queue, i);
			previous[i] = i;
		}
	}

	/* Process all squares into the queue */
	while (q_len(queue) > 0) {
		/* Get the current square and its color */
		int n1 = q_pop_int(queue);
		int color2 = colors[n1];

		/* If we're not looking for a specific color, any new one will do */
		if ((new_color == -1) && color2 && (color2 != color))
			new_color = color2;

		/* See if we've reached a square with a new color */
		if (color2 == new_color) {
			/* Step backward through the path, turning stone to tunnel */
			while (colors[n1] != color) {
				struct loc grid;
				i_to_grid(n1, w, &grid);
				if (colors[n1] > 0) {
					--counts[colors[n1]];
				}
				++counts[color];
				colors[n1] = color;
				/* Don't break permanent walls or vaults.  Also
				 * don't override terrain that already allows
				 * passage. */
				if (!square_isperm(c, grid) &&
						!square_isvault(c, grid) &&
						!(square_ispassable(c, grid) ||
						square_isdoor(c, grid))) {
					square_set_feat(c, grid, FEAT_FLOOR);
				}
				n1 = previous[n1];
			}

			/* Update the color mapping to combine the two colors */
			fix_colors(colors, counts, color2, color, size);

			/* We're done now */
			break;
		}

		/* If we haven't reached a new color, add all the unprocessed adjacent
		 * squares to our queue.
		 */
		for (i = 0; i < 4; i++) {
			int n2;
			struct loc grid;
			i_to_grid(n1, w, &grid);

			/* Move to the adjacent square */
			grid = loc_sum(grid, ddgrid_ddd[i]);

			/* Make sure we stay inside the boundaries */
			if (!square_in_bounds(c, grid)) continue;

			/* If the cell hasn't already been processed and we're
			 * willing to include it, add it to the queue */
			n2 = grid_to_i(grid, w);
			if (previous[n2] >= 0) continue;
			if (square_isperm(c, grid)) continue;
			if (square_isvault(c, grid) &&
				!allow_vault_disconnect) continue;
			q_push_int(queue, n2);
			previous[n2] = n1;
		}
	}

	/* Free the memory we've allocated */
	q_free(queue);
	mem_free(previous);
}


/**
 * Start connecting regions, stopping when the cave is entirely connected.
 * \param c is the current chunk
 * \param colors is the array of current point colors
 * \param counts is the array of current color counts
 * \param allow_vault_disconnect If true, allows vaults to be included in
 * path planning which can leave regions disconnected.
 */
static void join_regions(struct chunk *c, int colors[], int counts[],
		bool allow_vault_disconnect) {
	int h = c->height;
	int w = c->width;
	int size = h * w;
	int num = count_colors(counts, size);

	/* While we have multiple colors (i.e. disconnected regions), join one
	 * of the regions to another one.
	 */
	while (num > 1) {
		int color = first_color(counts, size);
		join_region(c, colors, counts, color, -1,
			allow_vault_disconnect);
		num--;
	}
}


/**
 * Make sure that all the regions of the dungeon are connected.
 * \param c is the current chunk
 *
 * This function colors each connected region of the dungeon, then uses that
 * information to join them into one conected region.
 */
void ensure_connectedness(struct chunk *c, bool allow_vault_disconnect) {
	int size = c->height * c->width;
	int *colors = mem_zalloc(size * sizeof(int));
	int *counts = mem_zalloc(size * sizeof(int));

	build_colors(c, colors, counts, true);
	join_regions(c, colors, counts, allow_vault_disconnect);

	mem_free(colors);
	mem_free(counts);
}


#define MAX_CAVERN_TRIES 10
/**
 * The cavern generator's main function.
 * \param depth the chunk's native depth
 * \param h the chunk's dimensions
 * \param w the chunk's dimensions
 * \return a pointer to the generated chunk
 */
static struct chunk *cavern_chunk(int depth, int h, int w)
{
	int i;
	int size = h * w;
	int limit = size / 13;
	int density = rand_range(25, 40);
	int times = rand_range(3, 6);

	int *colors = mem_zalloc(size * sizeof(int));
	int *counts = mem_zalloc(size * sizeof(int));

	int tries;

	struct chunk *c = cave_new(h, w);
	c->depth = depth;

	ROOM_LOG("cavern h=%d w=%d size=%d density=%d times=%d", h, w, size,
			 density, times);

	/* Start trying to build caverns */
	for (tries = 0; tries < MAX_CAVERN_TRIES; tries++) {
		/* Build a random cavern and mutate it a number of times */
		init_cavern(c, density);
		for (i = 0; i < times; i++) mutate_cavern(c);

		/* If there are enough open squares then we're done */
		if (c->feat_count[FEAT_FLOOR] >= limit) {
			ROOM_LOG("cavern ok (%d vs %d)", c->feat_count[FEAT_FLOOR], limit);
			break;
		}
		ROOM_LOG("cavern failed--try again (%d vs %d)",
				 c->feat_count[FEAT_FLOOR], limit);
	}

	/* If we couldn't make a big enough cavern then fail */
	if (tries == MAX_CAVERN_TRIES) {
		mem_free(colors);
		mem_free(counts);
		cave_free(c);
		return NULL;
	}

	build_colors(c, colors, counts, false);
	clear_small_regions(c, colors, counts);
	join_regions(c, colors, counts, true);

	mem_free(colors);
	mem_free(counts);

	return c;
}


/**
 * Make a cavern level.
 * \param p is the player
 */
struct chunk *cavern_gen(struct player *p, int min_height, int min_width) {
	int i, k;

	int h = rand_range(z_info->dungeon_hgt / 2, (z_info->dungeon_hgt * 3) / 4);
	int w = rand_range(z_info->dungeon_wid / 2, (z_info->dungeon_wid * 3) / 4);

	struct chunk *c;

	/* No persistent levels of this type for now */
	if (OPT(p, birth_levels_persist)) return NULL;

	if (p->depth < 15) {
		/* If we're too shallow then don't do it */
		return false;

	} else {
		/* Enforce minimum dimensions */
		h = MAX(h, min_height);
		w = MAX(w, min_width);

		/* Try to build the cavern, fail gracefully */
		c = cavern_chunk(p->depth, h, w);
		if (!c) return NULL;
	}
	c->depth = p->depth;

	/* Surround the level with perma-rock */
	draw_rectangle(c, 0, 0, h - 1, w - 1, FEAT_PERM, SQUARE_NONE, true);

	/* Place 2-3 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(1, 3));

	/* Place 1-2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2));

	/* General some rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Scale number of monsters items by cavern size */
	k = MAX((4 * k * (h * w)) / (z_info->dungeon_hgt * z_info->dungeon_wid), 6);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon, */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k), c->depth, 0);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Put some monsters in the dungeon */
	for (i = randint1(8) + k; i > 0; i--)
		pick_and_place_distant_monster(c, p, 0, true, c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(k, 2), c->depth + 5,
		ORIGIN_CAVERN);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(k / 2, 2), c->depth,
		ORIGIN_CAVERN);
	alloc_objects(c, SET_BOTH, TYP_GOOD, randint0(k / 4), c->depth,
		ORIGIN_CAVERN);

	return c;
}

/* ------------------ TOWN ---------------- */

/**
 * Get the bounds of a town lot.
 * @param xroads - the location of the town crossroads
 * @param lot - the lot location, indexed from the nw corner
 * @param lot_wid - lot width for the town
 * @param lot_hgt - lot height for the town
 * @param west - a pointer to put the minimum x coord of the lot
 * @param north - a pointer to put the minimum y coord of the lot
 * @param east - a pointer to put the maximum x coord of the lot
 * @param south - a pointer to put the maximum y coord of the lot
 */
static void get_lot_bounds(struct loc xroads, struct loc lot,
		int lot_wid, int lot_hgt,
		int *west, int *north, int *east, int *south) {

	// 0 is the road. no lots.
	if (lot.x == 0 || lot.y == 0) {
		*east = 0;
		*west = 0;
		*north = 0;
		*south = 0;
		return;
	}

	if (lot.x < 0) {
		*west = MAX(2, xroads.x - 1 + (lot.x) * lot_wid);
		*east = MIN(z_info->town_wid - 3, xroads.x - 2 + (lot.x + 1) * lot_wid);
	} else {
		*west = MAX(2, xroads.x + 2 + (lot.x - 1) * lot_wid);
		*east = MIN(z_info->town_wid - 3, xroads.x + 1 + (lot.x) * lot_wid);
	}

	if (lot.y < 0) {
		*north = MAX(2, xroads.y + (lot.y) * lot_hgt);
		*south = MIN(z_info->town_hgt - 3, xroads.y - 1 + (lot.y + 1) * lot_hgt);
	} else {
		*north = MAX(2, xroads.y + 2 + (lot.y - 1) * lot_hgt);
		*south = MIN(z_info->town_hgt - 3, xroads.y + 1 + (lot.y) * lot_hgt);
	}
}

static bool lot_is_clear(struct chunk *c, struct loc xroads, struct loc lot,
		int lot_wid, int lot_hgt) {
	struct loc nw_corner, se_corner, probe;

	get_lot_bounds(xroads, lot, lot_wid, lot_hgt,
		&nw_corner.x, &nw_corner.y, &se_corner.x, &se_corner.y);

	if (se_corner.x - nw_corner.x < lot_wid - 1 || se_corner.y - nw_corner.y < lot_hgt - 1) {
		return false;
	}

	for (probe.x = nw_corner.x; probe.x <= se_corner.x; probe.x++) {
		for (probe.y = nw_corner.y; probe.y <= se_corner.y; probe.y++) {
			if (!square_isfloor(c, probe)) {
				return false;
			}
		}
	}

	return true;
}

static bool lot_has_shop(struct chunk *c, struct loc xroads, struct loc lot,
		int lot_wid, int lot_hgt) {
	struct loc nw_corner, se_corner, probe;

	get_lot_bounds(xroads, lot, lot_wid, lot_hgt, &nw_corner.x, &nw_corner.y,
			&se_corner.x, &se_corner.y);

	for (probe.x = nw_corner.x; probe.x <= se_corner.x; probe.x++) {
		for (probe.y = nw_corner.y; probe.y <= se_corner.y; probe.y++) {
			if (feat_is_shop(square(c, probe)->feat)) {
				return true;
			}
		}
	}

	return false;
}

/**
 * Builds a store at a given pseudo-location
 * \param c is the current chunk
 * \param n is which shop it is
 * \param xroads is the location of the town crossroads
 * \param lot the location of this store in the town layout
 */
static void build_store(struct chunk *c, int n, struct loc xroads,
						struct loc lot, int lot_wid, int lot_hgt)
{
	int feat;
	struct loc door;

	int lot_w, lot_n, lot_e, lot_s;

	int build_w, build_n, build_e, build_s;

	get_lot_bounds(xroads, lot, lot_wid, lot_hgt, &lot_w, &lot_n, &lot_e,
		&lot_s);

	if (lot.x < -1 || lot.x > 1) {
		/* on the east west street */
		if (lot.y == -1) {
			/* north side of street */
			door.y = MAX(lot_n + 1, lot_s - randint0(2));
			build_s = door.y;
			build_n = door.y - 2;
		} else {
			/* south side */
			door.y = MIN(lot_s - 1, lot_n + randint0(2));
			build_n = door.y;
			build_s = door.y + 2;
		}

		door.x = rand_range(lot_w + 1, lot_e - 2);
		build_w = rand_range(MAX(lot_w, door.x - 2), door.x);
		if (!square_isfloor(c, loc(build_w - 1, door.y))) {
			build_w++;
			door.x = MAX(door.x, build_w);
		}
		build_e = rand_range(build_w + 2, MIN(door.x + 2, lot_e));
		if (build_e - build_w > 1
			&& !square_isfloor(c, loc(build_e + 1, door.y))) {
			build_e--;
			door.x = MIN(door.x, build_e);
		}

	} else if (lot.y < -1 || lot.y > 1) {
		/* on the north - south street */
		if (lot.x == -1) {
			/* west side of street */
			door.x = MAX(lot_w + 1, lot_e - randint0(2) - randint0(2));
			build_e = door.x;
			build_w = door.x - 2;
		} else {
			/* east side */
			door.x = MIN(lot_e - 1, lot_w + randint0(2) + randint0(2));
			build_w = door.x;
			build_e = door.x + 2;
		}

		door.y = rand_range(lot_n, lot_s - 1);
		build_n = rand_range(MAX(lot_n, door.y - 2), door.y);
		if (!square_isfloor(c, loc(door.x, build_n - 1))) {
			build_n++;
			door.y = MAX(door.y, build_n);
		}

		build_s = rand_range(MAX(build_n + 1, door.y), MIN(lot_s, door.y + 2));
		if (build_s - build_n > 1 &&
			!square_isfloor(c, loc(door.x, build_s + 1))) {
			build_s--;
			door.y = MIN(door.y, build_s);
		}

	} else {
		/* corner store */
		if (lot.x < 0) {
			/* west side */
			door.x = lot_e - 1 - randint0(2);
			build_e = MIN(lot_e, door.x + randint0(2));
			build_w = rand_range(MAX(lot_w, door.x - 2), build_e - 2);
		} else {
			/* east side */
			door.x = lot_w + 1 + randint0(2);
			build_w = MAX(lot_w, door.x - randint0(2));
			build_e = rand_range(build_w + 2, MIN(lot_e, door.x + 2));
		}

		if (lot.y < 0) {
			/* north side */
			door.y = lot_s - randint0(2);
			if (build_e == door.x || build_w == door.x) {
				build_s = door.y + randint0(2);
			} else {
				/* Avoid encapsulating door */
				build_s = door.y;
			}
			build_n = MAX(lot_n, door.y - 2);
			if (build_s - build_n > 1 &&
				!square_isfloor(c, loc(door.x, build_n - 1))) {
				build_n++;
				door.y = MAX(build_n, door.y);
			}
		} else {
			/* south side */
			door.y = lot_n + randint0(2);
			if (build_e == door.x || build_w == door.x) {
				build_n = door.y - randint0(2);
			} else {
				/* Avoid encapsulating door */
				build_n = door.y;
			}
			build_s = MIN(lot_s, door.y + 2);
			if (build_s - build_n > 1 &&
				!square_isfloor(c, loc(door.x, build_s + 1))) {
				build_s--;
				door.y = MIN(build_s, door.y);
			}
		}

		// Avoid placing buildings without space between them
		if (lot.x < 0 && build_e - build_w > 1 &&
				!square_isfloor(c, loc(build_w - 1, door.y))) {
			build_w++;
			door.x = MAX(door.x, build_w);
		} else if (lot.x > 0 && build_e - build_w > 1 &&
				!square_isfloor(c, loc(build_e + 1, door.y))) {
			build_e--;
			door.x = MIN(door.x, build_e);
		}
	}
	build_w = MAX(build_w, lot_w);
	build_e = MIN(build_e, lot_e);
	build_n = MAX(build_n, lot_n);
	build_s = MIN(build_s, lot_s);

	/* Build an invulnerable rectangular building */
	fill_rectangle(c, build_n, build_w, build_s, build_e, FEAT_PERM, SQUARE_NONE);

	/* Clear previous contents, add a store door */
	for (feat = 0; feat < z_info->f_max; feat++)
		if (feat_is_shop(feat) && (f_info[feat].shopnum == n + 1))
			square_set_feat(c, door, feat);
}

static void build_ruin(struct chunk *c, struct loc xroads, struct loc lot, int lot_wid, int lot_hgt) {
	int lot_west, lot_north, lot_east, lot_south;
	int wid, hgt;

	get_lot_bounds(xroads, lot, lot_wid, lot_hgt, &lot_west, &lot_north,
		&lot_east, &lot_south);

	if (lot_east - lot_west < 1 || lot_south - lot_north < 1) return;

	/* make a building */

	wid = rand_range(1, lot_wid - 2);
	hgt = rand_range(1, lot_hgt - 2);

	int offset_x = rand_range(1, lot_wid - 1 - wid);
	int offset_y = rand_range(1, lot_hgt - 1 - hgt);

	int west = lot_west + offset_x;
	int north = lot_north + offset_y;
	int south = lot_south - (lot_hgt - (hgt + offset_y));
	int east = lot_east - (lot_wid - (wid + offset_x));

	fill_rectangle(c, north, west, south, east, FEAT_GRANITE, SQUARE_NONE);

	int x, y;
	/* and then destroy it and spew rubble everywhere */
	for (x = lot_west; x <= lot_east; x++) {
		for (y = lot_north; y <= lot_south; y++) {
			if (x >= west && x <= east && y >= north && y <= south) {
				if (!randint0(4)) {
					square_set_feat(c, loc(x,y), FEAT_RUBBLE);
				}
			} else if (!randint0(3) &&
					square_isfloor(c, loc(x,y)) &&
					/* Avoid placing rubble next to a store */
					(x > lot_west || x == 2 ||
					!square_isperm(c, loc(x-1, y))) &&
					(x < lot_east || x == z_info->town_wid-2 ||
					!square_isperm(c, loc(x+1, y))) &&
					(y > lot_north || y == 2 ||
					!square_isperm(c, loc(x, y-1))) &&
					(y < lot_south || y == z_info-> town_hgt-2 ||
					!square_isperm(c, loc(x, y+1)))) {
				square_set_feat(c, loc(x,y), FEAT_PASS_RUBBLE);
			}
		}
	}
}

/**
 * Generate the town for the first time, and place the player
 * \param c is the current chunk
 * \param p is the player
 */
static void town_gen_layout(struct chunk *c, struct player *p)
{
	int n, x, y;
	struct loc grid, pgrid, xroads;
	int num_lava = 3 + randint0(3);
	int ruins_percent = 80;

	int max_attempts = 100;

	int num_attempts = 0;
	bool success = false;

	int max_store_y = 0;
	int min_store_x = z_info->town_wid;
	int max_store_x = 0;

	/* divide the town into lots */
	u16b lot_hgt = 4, lot_wid = 6;

	/* Create walls */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, FEAT_PERM,
		SQUARE_NONE, true);

	while (!success) {
		/* Initialize to ROCK for build_streamer precondition */
		for (grid.y = 1; grid.y < c->height - 1; grid.y++)
			for (grid.x = 1; grid.x < c->width - 1; grid.x++) {
				square_set_feat(c, grid, FEAT_GRANITE);
			}

		/* Make some lava streamers */
		for (n = 0; n < 3 + num_lava; n++)
			build_streamer(c, FEAT_LAVA, 0);

		/* Make a town-sized starburst room. */
		(void) generate_starburst_room(c, 0, 0, c->height - 1,
			c->width - 1, false, FEAT_FLOOR, false);

		/* Turn off room illumination flag */
		for (grid.y = 1; grid.y < c->height - 1; grid.y++) {
			for (grid.x = 1; grid.x < c->width - 1; grid.x++) {
				if (square_isfloor(c, grid))
					sqinfo_off(square(c, grid)->info, SQUARE_ROOM);
			}
		}

		/* Stairs along north wall */
		pgrid.x = rand_spread(z_info->town_wid / 2,
			z_info->town_wid / 6);
		pgrid.y = 1;
		while (!square_isfloor(c, pgrid) && (pgrid.y < z_info->town_hgt / 4)) {
			pgrid.y++;
		}
		if (pgrid.y >= z_info->town_hgt / 4) continue;


		/* no lava next to stairs */
		for (x = pgrid.x - 1; x <= pgrid.x + 1; x++) {
			for (y = pgrid.y - 1; y <= pgrid.y + 1; y++) {
				if (square_isfiery(c, loc(x, y))) {
					square_set_feat(c, loc(x, y), FEAT_GRANITE);
				}
			}
		}

		xroads.x = pgrid.x;
		xroads.y = z_info->town_hgt / 2
				- randint0(z_info->town_hgt / 4)
				+ randint0(z_info->town_hgt / 8);


		int lot_min_x = -1 * xroads.x / lot_wid;
		int lot_max_x = (z_info->town_wid - xroads.x) / lot_wid;
		int lot_min_y = -1 * xroads.y / lot_hgt;
		int lot_max_y = (z_info->town_hgt - xroads.y) / lot_hgt;

		/* place stores along the streets */
		num_attempts = 0;
		for (n = 0; n < MAX_STORES; n++) {
			struct loc store_lot;
			bool found_spot = false;
			while (!found_spot && num_attempts < max_attempts) {
				num_attempts++;
				if (randint0(2)) {
					/* east-west street */
					store_lot.x = rand_range(lot_min_x, lot_max_x);
					store_lot.y = randint0(2) ? 1 : -1;
				} else {
					/* north-south street */
					store_lot.x = randint0(2) ? 1 : -1;
					store_lot.y = rand_range(lot_min_y, lot_max_y);
				}
				if (store_lot.y == 0 || store_lot.x == 0) continue;
				found_spot = lot_is_clear(c, xroads, store_lot,
					lot_wid, lot_hgt);
			}
			if (num_attempts >= max_attempts) break;

			max_store_y = MAX(max_store_y, xroads.y + lot_hgt * store_lot.y);
			min_store_x = MIN(min_store_x, xroads.x + lot_wid * store_lot.x);
			max_store_x = MAX(max_store_x, xroads.x + lot_wid * store_lot.x);

			build_store(c, n, xroads, store_lot, lot_wid, lot_hgt);
		}
		if (num_attempts >= max_attempts) continue;

		/* place ruins */
		for (x = lot_min_x; x <= lot_max_x; x++) {
			if (x == 0) continue; /* 0 is the street */
			for (y = lot_min_y; y <= lot_max_y; y++) {
				if (y == 0) continue;
				if (randint0(100) > ruins_percent) continue;
				if (one_in_(2) &&
					!lot_has_shop(c, xroads, loc(x, y), lot_wid, lot_hgt)) {
					build_ruin(c, xroads, loc(x, y), lot_wid, lot_hgt);
				}
			}
		}
		success = true;
	}

	/* clear the street */
	square_set_feat(c, loc(pgrid.x, pgrid.y + 1), FEAT_FLOOR);
	fill_rectangle(c, pgrid.y + 2, pgrid.x - 1,
		max_store_y, pgrid.x + 1, FEAT_FLOOR, SQUARE_NONE);

	fill_rectangle(c, xroads.y, min_store_x,
		xroads.y + 1, max_store_x, FEAT_FLOOR, SQUARE_NONE);

	/* Clear previous contents, add down stairs */
	square_set_feat(c, pgrid, FEAT_MORE);

	/* Place the player */
	player_place(c, p, pgrid);
}


/**
 * Town logic flow for generation of new town.
 * \param p is the player
 * \return a pointer to the generated chunk
 * We start with a fully wiped cave of normal floors. This function does NOT do
 * anything about the owners of the stores, nor the contents thereof. It only
 * handles the physical layout.
 */
struct chunk *town_gen(struct player *p, int min_height, int min_width)
{
	int i;
	struct loc grid;
	int residents = is_daytime() ? z_info->town_monsters_day :
		z_info->town_monsters_night;
	struct chunk *c_new, *c_old = chunk_find_name("Town");

	/* Make a new chunk */
	c_new = cave_new(z_info->town_hgt, z_info->town_wid);

	/* First time */
	if (!c_old) {
		c_new->depth = p->depth;

		/* Build stuff */
		town_gen_layout(c_new, p);
	} else {
		/* Copy from the chunk list, remove the old one */
		if (!chunk_copy(c_new, c_old, 0, 0, 0, 0))
			quit_fmt("chunk_copy() level bounds failed!");
		chunk_list_remove("Town");
		cave_free(c_old);

		/* Find the stairs (lame) */
		for (grid.y = 0; grid.y < c_new->height; grid.y++) {
			bool found = false;
			for (grid.x = 0; grid.x < c_new->width; grid.x++) {
				if (square_feat(c_new, grid)->fidx == FEAT_MORE) {
					found = true;
					break;
				}
			}
			if (found) break;
		}

		/* Place the player */
		player_place(c_new, p, grid);
	}

	/* Apply illumination */
	cave_illuminate(c_new, is_daytime());

	/* Make some residents */
	for (i = 0; i < residents; i++)
		pick_and_place_distant_monster(c_new, p, 3, true, c_new->depth);

	return c_new;
}


/* ------------------ MODIFIED ---------------- */
/**
 * The main modified generation algorithm
 * \param depth is the chunk's native depth
 * \param height are the chunk's dimensions
 * \param width are the chunk's dimensions
 * \return a pointer to the generated chunk
 */
static struct chunk *modified_chunk(int depth, int height, int width)
{
	int i;
	int by = 0, bx = 0, key, rarity;
	int num_floors;
	int num_rooms = dun->profile->n_room_profiles;
	int dun_unusual = dun->profile->dun_unusual;
	struct connector *join = dun->join;

	/* Make the cave */
	struct chunk *c = cave_new(height, width);
	c->depth = depth;

	/* Set the intended number of floor grids based on cave floor area */
	num_floors = c->height * c->width / 7;
	ROOM_LOG("height=%d  width=%d  nfloors=%d", c->height, c->width,num_floors);

	/* Fill cave area with basic granite */
	fill_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_GRANITE, SQUARE_NONE);

	/* Generate permanent walls around the generated area (temporarily!) */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_PERM, SQUARE_NONE, true);

	/* Actual maximum number of blocks on this level */
	dun->row_blocks = c->height / dun->block_hgt;
	dun->col_blocks = c->width / dun->block_wid;

	/* Initialize the room table */
	dun->room_map = mem_zalloc(dun->row_blocks * sizeof(bool*));
	for (i = 0; i < dun->row_blocks; i++)
		dun->room_map[i] = mem_zalloc(dun->col_blocks * sizeof(bool));

	/* No rooms yet, pits or otherwise. */
	dun->pit_num = 0;
	dun->cent_n = 0;
	reset_entrance_data(c);

	/* Build the special staircase rooms */
	if (OPT(player, birth_levels_persist)) {
		struct room_profile profile;
		for (i = 0; i < num_rooms; i++) {
			profile = dun->profile->room_profiles[i];
			if (streq(profile.name, "staircase room")) {
				break;
			}
		}
		while (join) {
			if (!room_build(c, dun->join->grid.y, dun->join->grid.x, profile,
							true)) {
				dump_level_simple(NULL, "Modified Generation:"
					"  Failed to Build Staircase Room", c);
				quit("Failed to place stairs");
			}
			join = join->next;
		}
	}

	/* Build rooms until we have enough floor grids and at least two rooms */
	while ((c->feat_count[FEAT_FLOOR] < num_floors) || (dun->cent_n < 2)) {

		/* Roll for random key (to be compared against a profile's cutoff) */
		key = randint0(100);

		/* We generate a rarity number to figure out how exotic to make
		 * the room. This number has a (50+depth/2)/DUN_UNUSUAL chance
		 * of being > 0, a (50+depth/2)^2/DUN_UNUSUAL^2 chance of
		 * being > 1, up to MAX_RARITY. */
		i = 0;
		rarity = 0;
		while (i == rarity && i < dun->profile->max_rarity) {
			if (randint0(dun_unusual) < 50 + c->depth / 2) rarity++;
			i++;
		}

		/* Once we have a key and a rarity, we iterate through out list of
		 * room profiles looking for a match (whose cutoff > key and whose
		 * rarity > this rarity). We try building the room, and if it works
		 * then we are done with this iteration. We keep going until we find
		 * a room that we can build successfully or we exhaust the profiles. */
		for (i = 0; i < num_rooms; i++) {
			struct room_profile profile = dun->profile->room_profiles[i];
			if (profile.rarity > rarity) continue;
			if (profile.cutoff <= key) continue;
			if (room_build(c, by, bx, profile, true)) break;
		}
	}

	for (i = 0; i < dun->row_blocks; i++)
		mem_free(dun->room_map[i]);
	mem_free(dun->room_map);

	/* Connect all the rooms together */
	do_traditional_tunneling(c);
	ensure_connectedness(c, true);

	/* Turn the outer permanent walls back to granite */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_GRANITE, SQUARE_NONE, true);

	return c;
}

/**
 * Generate a new dungeon level.
 * \param p is the player
 * \return a pointer to the generated chunk
 *
 * This is sample code to illustrate some of the new dungeon generation
 * methods; I think it actually produces quite nice levels.  New stuff:
 *
 * - different sized levels
 * - independence from block size: the block size can be set to any number
 *   from 1 (no blocks) to about 15; beyond that it struggles to generate
 *   enough floor space
 * - the find_space function, called from the room builder functions, allows
 *   the room to find space for itself rather than the generation algorithm
 *   allocating it; this helps because the room knows better what size it is
 * - a count is now kept of grids of the various terrains, allowing dungeon
 *   generation to terminate when enough floor is generated
 * - there are three new room types - huge rooms, rooms of chambers
 *   and interesting rooms - as well as many new vaults
 * - there is the ability to place specific monsters and objects in vaults and
 *   interesting rooms, as well as to make general monster restrictions in
 *   areas or the whole dungeon
 */
struct chunk *modified_gen(struct player *p, int min_height, int min_width) {
	int i, k;
	int size_percent, y_size, x_size;
	struct chunk *c;

	/* Scale the level */
	i = randint1(10) + p->depth / 24;
	if (is_quest(p->depth)) size_percent = 100;
	else if (i < 2) size_percent = 75;
	else if (i < 3) size_percent = 80;
	else if (i < 4) size_percent = 85;
	else if (i < 5) size_percent = 90;
	else if (i < 6) size_percent = 95;
	else size_percent = 100;
	y_size = z_info->dungeon_hgt * (size_percent - 5 + randint0(10)) / 100;
	x_size = z_info->dungeon_wid * (size_percent - 5 + randint0(10)) / 100;

	/* Enforce minimum dimensions */
	y_size = MAX(y_size, min_height);
	x_size = MAX(x_size, min_width);

	/* Set the block height and width */
	dun->block_hgt = dun->profile->block_size;
	dun->block_wid = dun->profile->block_size;

	c = modified_chunk(p->depth, MIN(z_info->dungeon_hgt, y_size),
		MIN(z_info->dungeon_wid, x_size));
	c->depth = p->depth;

	/* Generate permanent walls around the edge of the generated area */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1,
		FEAT_PERM, SQUARE_NONE, true);

	/* Add some magma streamers */
	for (i = 0; i < dun->profile->str.mag; i++)
		build_streamer(c, FEAT_MAGMA, dun->profile->str.mc);

	/* Add some quartz streamers */
	for (i = 0; i < dun->profile->str.qua; i++)
		build_streamer(c, FEAT_QUARTZ, dun->profile->str.qc);

	/* Place 3 or 4 down stairs near some walls */
	if (!OPT(p, birth_levels_persist) || !chunk_find_adjacent(p, false)) {
		alloc_stairs(c, FEAT_MORE, rand_range(3, 4));
	}

	/* Place 1 or 2 up stairs near some walls */
	if (!OPT(p, birth_levels_persist) || !chunk_find_adjacent(p, true)) {
		alloc_stairs(c, FEAT_LESS, rand_range(1, 2));
	}

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon, reduce frequency by factor of 5 */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k)/5, c->depth, 0);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = z_info->level_monster_min + randint1(8) + k;

	/* Remove all monster restrictions. */
	mon_restrict(NULL, c->depth, true);

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		pick_and_place_distant_monster(c, p, 0, true, c->depth);

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT,
		Rand_normal(z_info->room_item_av, 3), c->depth, ORIGIN_FLOOR);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT,
		Rand_normal(z_info->both_item_av, 3), c->depth, ORIGIN_FLOOR);
	alloc_objects(c, SET_BOTH, TYP_GOLD,
		Rand_normal(z_info->both_gold_av, 3), c->depth, ORIGIN_FLOOR);

	return c;
}


/* ------------------ MORIA ---------------- */
/**
 * The main moria generation algorithm
 * \param depth is the chunk's native depth
 * \param height are the chunk's dimensions
 * \param width are the chunk's dimensions
 * \return a pointer to the generated chunk
 */
static struct chunk *moria_chunk(int depth, int height, int width)
{
	int i;
	int by = 0, bx = 0, key, rarity;
	int num_floors;
	int num_rooms = dun->profile->n_room_profiles;
	int dun_unusual = dun->profile->dun_unusual;

	/* Make the cave */
	struct chunk *c = cave_new(height, width);
	c->depth = depth;

	/* Set the intended number of floor grids based on cave floor area */
	num_floors = c->height * c->width / 7;
	ROOM_LOG("height=%d  width=%d  nfloors=%d", c->height, c->width,num_floors);

	/* Fill cave area with basic granite */
	fill_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_GRANITE, SQUARE_NONE);

	/* Generate permanent walls around the generated area (temporarily!) */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_PERM, SQUARE_NONE, true);

	/* Actual maximum number of blocks on this level */
	dun->row_blocks = c->height / dun->block_hgt;
	dun->col_blocks = c->width / dun->block_wid;

	/* Initialize the room table */
	dun->room_map = mem_zalloc(dun->row_blocks * sizeof(bool*));
	for (i = 0; i < dun->row_blocks; i++)
		dun->room_map[i] = mem_zalloc(dun->col_blocks * sizeof(bool));

	/* No rooms yet, pits or otherwise. */
	dun->pit_num = 0;
	dun->cent_n = 0;
	reset_entrance_data(c);

	/* Build rooms until we have enough floor grids and at least two rooms
	 * (the latter is to make it easier to satisfy the constraints for
	 * player placement) */
	while (c->feat_count[FEAT_FLOOR] < num_floors || dun->cent_n < 2) {

		/* Roll for random key (to be compared against a profile's cutoff) */
		key = randint0(100);

		/* We generate a rarity number to figure out how exotic to make
		 * the room. This number has a (50+depth/2)/DUN_UNUSUAL chance
		 * of being > 0, a (50+depth/2)^2/DUN_UNUSUAL^2 chance of
		 * being > 1, up to MAX_RARITY. */
		i = 0;
		rarity = 0;
		while (i == rarity && i < dun->profile->max_rarity) {
			if (randint0(dun_unusual) < 50 + c->depth / 2) rarity++;
			i++;
		}

		/* Once we have a key and a rarity, we iterate through out list of
		 * room profiles looking for a match (whose cutoff > key and whose
		 * rarity > this rarity). We try building the room, and if it works
		 * then we are done with this iteration. We keep going until we find
		 * a room that we can build successfully or we exhaust the profiles. */
		for (i = 0; i < num_rooms; i++) {
			struct room_profile profile = dun->profile->room_profiles[i];
			if (profile.rarity > rarity) continue;
			if (profile.cutoff <= key) continue;
			if (room_build(c, by, bx, profile, true)) break;
		}
	}

	for (i = 0; i < dun->row_blocks; i++)
		mem_free(dun->room_map[i]);
	mem_free(dun->room_map);

	/* Connect all the rooms together */
	do_traditional_tunneling(c);
	ensure_connectedness(c, true);

	/* Turn the outer permanent walls back to granite */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_GRANITE, SQUARE_NONE, true);

	return c;
}

/**
 * Generate a new dungeon level.
 * \param p is the player
 * \return a pointer to the generated chunk
 *
 * This produces Oangband-style moria levels.
 *
 * Most rooms on these levels are large, ragged-edged and roughly oval-shaped.
 *
 * Monsters are mostly "Moria dwellers" - orcs, ogres, trolls and giants.
 *
 * Apart from the room and monster changes, generation is similar to modified
 * levels.  A good way of selecting these instead of modified (similar to 
 * labyrinth levels are selected) would be
 *	if ((c->depth >= 10) && (c->depth < 40) && one_in_(40))
 */
struct chunk *moria_gen(struct player *p, int min_height, int min_width) {
	int i, k;
	int size_percent, y_size, x_size;
	struct chunk *c;

	/* Scale the level */
	i = randint1(10) + p->depth / 24;
	if (is_quest(p->depth)) size_percent = 100;
	else if (i < 2) size_percent = 75;
	else if (i < 3) size_percent = 80;
	else if (i < 4) size_percent = 85;
	else if (i < 5) size_percent = 90;
	else if (i < 6) size_percent = 95;
	else size_percent = 100;
	y_size = z_info->dungeon_hgt * (size_percent - 5 + randint0(10)) / 100;
	x_size = z_info->dungeon_wid * (size_percent - 5 + randint0(10)) / 100;

	/* Enforce minimum dimensions */
	y_size = MAX(y_size, min_height);
	x_size = MAX(x_size, min_width);

	/* Set the block height and width */
	dun->block_hgt = dun->profile->block_size;
	dun->block_wid = dun->profile->block_size;

	c = moria_chunk(p->depth, MIN(z_info->dungeon_hgt, y_size),
		MIN(z_info->dungeon_wid, x_size));
	c->depth = p->depth;

	/* Generate permanent walls around the edge of the generated area */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1,
		FEAT_PERM, SQUARE_NONE, true);

	/* Add some magma streamers */
	for (i = 0; i < dun->profile->str.mag; i++)
		build_streamer(c, FEAT_MAGMA, dun->profile->str.mc);

	/* Add some quartz streamers */
	for (i = 0; i < dun->profile->str.qua; i++)
		build_streamer(c, FEAT_QUARTZ, dun->profile->str.qc);

	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(3, 4));

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2));

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon, reduce frequency by factor of 5 */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k)/5, c->depth, 0);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = z_info->level_monster_min + randint1(8) + k;

	/* Moria levels have a high proportion of cave dwellers. */
	mon_restrict("Moria dwellers", c->depth, true);

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		pick_and_place_distant_monster(c, p, 0, true, c->depth);

	/* Remove our restrictions. */
	(void) mon_restrict(NULL, c->depth, false);

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT,
		Rand_normal(z_info->room_item_av, 3), c->depth, ORIGIN_FLOOR);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT,
		Rand_normal(z_info->both_item_av, 3), c->depth, ORIGIN_FLOOR);
	alloc_objects(c, SET_BOTH, TYP_GOLD,
		Rand_normal(z_info->both_gold_av, 3), c->depth, ORIGIN_FLOOR);

	return c;
}


/* ------------------ HARD CENTRE ---------------- */
/**
 * Make a chunk consisting only of a greater vault
 * \param p is the player
 * \return a pointer to the generated chunk
 */
static struct chunk *vault_chunk(struct player *p)
{
	struct vault *v;
	struct chunk *c;

	if (one_in_(2)) v = random_vault(p->depth, "Greater vault (new)");
	else v = random_vault(p->depth, "Greater vault");

	/* Make the chunk */
	c = cave_new(v->hgt, v->wid);
	c->depth = p->depth;

	/* Fill with granite; the vault will override for the grids it sets. */
	fill_rectangle(c, 0, 0, v->hgt - 1, v->wid - 1, FEAT_GRANITE,
		SQUARE_NONE);

	/* Build the vault in it */
	dun->cent_n = 0;
	reset_entrance_data(c);
	build_vault(c, loc(v->wid / 2, v->hgt / 2), v);

	return c;
}

/**
 * Make sure that all the caverns surrounding the centre are connected.
 * \param c is the entire current chunk (containing the caverns)
 * \param floor is an array of sample floor grids, one from each cavern in the
 * order left, upper, lower, right
 */
static void connect_caverns(struct chunk *c, struct loc floor[])
{
	int i;
	int size = c->height * c->width;
	int *colors = mem_zalloc(size * sizeof(int));
	int *counts = mem_zalloc(size * sizeof(int));
	int color_of_floor[4];

	/* Color the regions, find which cavern is which color */
	build_colors(c, colors, counts, true);
	for (i = 0; i < 4; i++) {
		int spot = grid_to_i(floor[i], c->width);
		color_of_floor[i] = colors[spot];
	}

	/* Join left and upper, right and lower */
	join_region(c, colors, counts, color_of_floor[0], color_of_floor[1],
		false);
	join_region(c, colors, counts, color_of_floor[2], color_of_floor[3],
		false);

	/* Join the two big caverns */
	for (i = 1; i < 3; i++) {
		int spot = grid_to_i(floor[i], c->width);
		color_of_floor[i] = colors[spot];
	}
	join_region(c, colors, counts, color_of_floor[1], color_of_floor[2],
		false);

	mem_free(colors);
	mem_free(counts);
}
/**
 * Generate a hard centre level - a greater vault surrounded by caverns
 * \param p is the player
 * \return a pointer to the generated chunk
*/
struct chunk *hard_centre_gen(struct player *p, int min_height, int min_width)
{
	/* Make a vault for the centre */
	struct chunk *centre = vault_chunk(p);
	int rotate = 0;

	/* Dimensions for the surrounding caverns */
	int centre_cavern_ypos;
	int centre_cavern_hgt, centre_cavern_wid;
	int upper_cavern_hgt, lower_cavern_hgt;
	struct chunk *upper_cavern;
	struct chunk *lower_cavern;
	int lower_cavern_ypos;
	int left_cavern_wid, right_cavern_wid;
	struct chunk *left_cavern;
	struct chunk *right_cavern;
	struct chunk *c;
	int i, k, cavern_area;
	struct loc grid;
	struct loc floor[4];

	/* No persistent levels of this type for now */
	if (OPT(p, birth_levels_persist)) {
		wipe_mon_list(centre, p);
		cave_free(centre);
		return NULL;
	}

	/*
	 * Carve out entrances to the vault.  Only use one if there aren't
	 * explicitly marked entrances since those vaults typically have empty
	 * space about them and the extra entrances aren't useful.
	 */
	k = 1 + ((dun->ent_n[0] > 0) ? randint1(3) : 0);
	dun->wall_n = 0;
	for (i = 0; i < k; ++i) {
		if (dun->ent_n[0] == 0) {
			/*
			 * There's no explicitly marked entrances.  Look for a
			 * square marked SQUARE_WALL_OUTER.
			 */
			if (!cave_find(centre, &grid, square_iswall_outer)) {
				if (i == 0) {
					wipe_mon_list(centre, p);
					cave_free(centre);
					return NULL;
				}
				break;
			}
		} else {
			grid = choose_random_entrance(centre, 0, NULL, 0,
				dun->wall, i);
			if (loc_eq(grid, loc(0, 0))) {
				if (i == 0) {
					wipe_mon_list(centre, p);
					cave_free(centre);
					return NULL;
				}
				break;
			}
		}
		/*
		 * Store position in dun->wall and mark neighbors as invalid
		 * entrances.
		 */
		pierce_outer_wall(centre, grid);
		/* Convert it to a floor. */
		square_set_feat(centre, grid, FEAT_FLOOR);
	}

	/* Measure the vault, rotate to make it wider than it is high */
	if (centre->height > centre->width) {
		rotate = 1;
		centre_cavern_ypos = (z_info->dungeon_hgt - centre->width) / 2;
		centre_cavern_hgt = centre->width;
		centre_cavern_wid = centre->height;
	} else {
		centre_cavern_ypos = (z_info->dungeon_hgt - centre->height) / 2;
		centre_cavern_hgt = centre->height;
		centre_cavern_wid = centre->width;
	}
	upper_cavern_hgt = centre_cavern_ypos;
	lower_cavern_hgt = z_info->dungeon_hgt - upper_cavern_hgt -
		centre_cavern_hgt;
	lower_cavern_ypos = centre_cavern_ypos + centre_cavern_hgt;

	/* Make the caverns */
	upper_cavern = cavern_chunk(p->depth, upper_cavern_hgt, centre_cavern_wid);
	lower_cavern = cavern_chunk(p->depth, lower_cavern_hgt, centre_cavern_wid);
	left_cavern_wid = (z_info->dungeon_wid - centre_cavern_wid) / 2;
	right_cavern_wid = z_info->dungeon_wid - left_cavern_wid -
		centre_cavern_wid;
	left_cavern = cavern_chunk(p->depth, z_info->dungeon_hgt, left_cavern_wid);
	right_cavern = cavern_chunk(p->depth, z_info->dungeon_hgt, right_cavern_wid);

	/* Return on failure */
	if (!upper_cavern || !lower_cavern || !left_cavern || !right_cavern) {
		if (right_cavern) cave_free(right_cavern);
		if (left_cavern) cave_free(left_cavern);
		if (lower_cavern) cave_free(lower_cavern);
		if (upper_cavern) cave_free(upper_cavern);
		wipe_mon_list(centre, p);
		cave_free(centre);
		return NULL;
	}

	/* Make a cave to copy them into, and find a floor square in each cavern */
	c = cave_new(z_info->dungeon_hgt, z_info->dungeon_wid);
	c->depth = p->depth;

	/* Left */
	chunk_copy(c, left_cavern, 0, 0, 0, false);
	find_empty_range(c, &grid, loc(0, 0),
					 loc(left_cavern_wid - 1, z_info->dungeon_hgt - 1));
	floor[0] = grid;

	/* Upper */
	chunk_copy(c, upper_cavern, 0, left_cavern_wid, 0, false);
	find_empty_range(c, &grid, loc(left_cavern_wid, 0),
					 loc(left_cavern_wid + centre_cavern_wid - 1,
						 upper_cavern_hgt - 1));
	floor[1] = grid;

	/* Centre */
	chunk_copy(c, centre, centre_cavern_ypos, left_cavern_wid, rotate, false);

	/* Lower */
	chunk_copy(c, lower_cavern, lower_cavern_ypos, left_cavern_wid, 0, false);
	find_empty_range(c, &grid, loc(left_cavern_wid, lower_cavern_ypos),
					 loc(left_cavern_wid + centre_cavern_wid - 1,
						 z_info->dungeon_hgt - 1));
	floor[3] = grid;

	/* Right */
	chunk_copy(c, right_cavern, 0, left_cavern_wid + centre_cavern_wid, 0,
		false);
	find_empty_range(c, &grid, loc(left_cavern_wid + centre_cavern_wid, 0),
		loc(z_info->dungeon_wid - 1, z_info->dungeon_hgt - 1));
	floor[2] = grid;

	/* Encase in perma-rock */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1,
		FEAT_PERM, SQUARE_NONE, true);

	/* Connect up all the caverns */
	connect_caverns(c, floor);

	/* Connect to the centre entrances. */
	ensure_connectedness(c, false);

	/* Free all the chunks */
	cave_free(left_cavern);
	cave_free(upper_cavern);
	cave_free(centre);
	cave_free(lower_cavern);
	cave_free(right_cavern);

	cavern_area = (left_cavern_wid + right_cavern_wid) * z_info->dungeon_hgt +
		centre_cavern_wid * (upper_cavern_hgt + lower_cavern_hgt);

	/* Place 2-3 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(1, 3));

	/* Place 1-2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2));

	/* Generate some rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Scale number by total cavern size - caverns are fairly sparse */
	k = (k * cavern_area) / (z_info->dungeon_hgt * z_info->dungeon_wid);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k), c->depth, 0);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Put some monsters in the dungeon */
	for (i = randint1(8) + k; i > 0; i--)
		pick_and_place_distant_monster(c, p, 0, true, c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(k, 2), c->depth + 5,
		ORIGIN_CAVERN);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(k / 2, 2), c->depth,
		ORIGIN_CAVERN);
	alloc_objects(c, SET_BOTH, TYP_GOOD, randint0(k / 4), c->depth,
		ORIGIN_CAVERN);

	return c;
}


/* ------------------ LAIR ---------------- */

/**
 * Generate a lair level - a regular cave generated with the modified
 * algorithm, connected to a cavern with themed monsters
 * \param p is the player
 * \return a pointer to the generated chunk
 */
struct chunk *lair_gen(struct player *p, int min_height, int min_width) {
	int i, k;
	int size_percent, y_size, x_size;
	struct chunk *c;
	struct chunk *normal;
	struct chunk *lair;

	/* No persistent levels of this type for now */
	if (OPT(p, birth_levels_persist)) return NULL;

	/* Scale the level */
	i = randint1(10) + p->depth / 24;
	if (is_quest(p->depth)) size_percent = 100;
	else if (i < 2) size_percent = 75;
	else if (i < 3) size_percent = 80;
	else if (i < 4) size_percent = 85;
	else if (i < 5) size_percent = 90;
	else if (i < 6) size_percent = 95;
	else size_percent = 100;
	y_size = z_info->dungeon_hgt * (size_percent - 5 + randint0(10)) / 100;
	x_size = z_info->dungeon_wid * (size_percent - 5 + randint0(10)) / 100;

	/* Enforce minimum dimensions */
	y_size = MAX(y_size, min_height);
	x_size = MAX(x_size, min_width);

	/* Set the block height and width */
	dun->block_hgt = dun->profile->block_size;
	dun->block_wid = dun->profile->block_size;

	normal = modified_chunk(p->depth, y_size, x_size / 2);
	if (!normal) return NULL;
	normal->depth = p->depth;

	lair = cavern_chunk(p->depth, y_size, x_size / 2);
	if (!lair) {
		wipe_mon_list(normal, p);
		cave_free(normal);
		return NULL;
	}
	lair->depth = p->depth;

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(p->depth / 3, 10), 2) / 2;

	/* Put the character in the normal half */
	new_player_spot(normal, p);

	/* Pick a smallish number of monsters for the normal half */
	i = randint1(4) + k;

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		pick_and_place_distant_monster(normal, p, 0, true, normal->depth);

	/* Add some magma streamers */
	for (i = 0; i < dun->profile->str.mag; i++)
		build_streamer(normal, FEAT_MAGMA, dun->profile->str.mc);

	/* Add some quartz streamers */
	for (i = 0; i < dun->profile->str.qua; i++)
		build_streamer(normal, FEAT_QUARTZ, dun->profile->str.qc);

	/* Pick a larger number of monsters for the lair */
	i = (z_info->level_monster_min + randint1(20) + k);

	/* Find appropriate monsters */
	while (true) {
		/* Choose a pit profile */
		set_pit_type(lair->depth, 0);

		/* Set monster generation restrictions */
		if (mon_restrict(dun->pit_type->name, lair->depth, true))
			break;
	}

	ROOM_LOG("Monster lair - %s", dun->pit_type->name);

	/* Place lair monsters */
	spread_monsters(lair, dun->pit_type->name, lair->depth, i, lair->height / 2,
					lair->width / 2, lair->height / 2, lair->width / 2, 
					ORIGIN_CAVERN);

	/* Remove our restrictions. */
	(void) mon_restrict(NULL, lair->depth, false);

	/* Make the level */
	c = cave_new(y_size, x_size);
	c->depth = p->depth;
	if (one_in_(2)) {
		chunk_copy(c, lair, 0, 0, 0, false);
		chunk_copy(c, normal, 0, x_size / 2, 0, false);
	} else {
		chunk_copy(c, normal, 0, 0, 0, false);
		chunk_copy(c, lair, 0, x_size / 2, 0, false);
	}

	/* Free the chunks */
	cave_free(normal);
	cave_free(lair);

	/* Generate permanent walls around the edge of the generated area */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1,
		FEAT_PERM, SQUARE_NONE, true);

	/* Connect */
	ensure_connectedness(c, true);

	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(3, 4));

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2));

	/* Put some rubble in corridors */
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon, reduce frequency by factor of 5 */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k)/5, c->depth, 0);

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT,
		Rand_normal(z_info->room_item_av, 3), c->depth, ORIGIN_FLOOR);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT,
		Rand_normal(z_info->both_item_av, 3), c->depth, ORIGIN_FLOOR);
	alloc_objects(c, SET_BOTH, TYP_GOLD,
		Rand_normal(z_info->both_gold_av, 3), c->depth, ORIGIN_FLOOR);

	return c;
}


/* ------------------ GAUNTLET ---------------- */

/**
 * Generate a gauntlet level - two separate caverns with an unmappable labyrinth
 * between them, and no teleport and only upstairs from the side where the
 * player starts.
 *
 * \param p is the player
 * \return a pointer to the generated chunk
 */
struct chunk *gauntlet_gen(struct player *p, int min_height, int min_width) {
	int i, k;
	struct chunk *c;
	struct chunk *left;
	struct chunk *gauntlet;
	struct chunk *right;
	int gauntlet_hgt = 2 * randint1(5) + 3;
	int gauntlet_wid = 2 * randint1(10) + 19;
	int y_size = z_info->dungeon_hgt - randint0(25 - gauntlet_hgt);
	int x_size = (z_info->dungeon_wid - gauntlet_wid) / 2 -
		randint0(45 - gauntlet_wid);
	int line1, line2;

	/* No persistent levels of this type for now */
	if (OPT(p, birth_levels_persist)) return NULL;

	gauntlet = labyrinth_chunk(p->depth, gauntlet_hgt, gauntlet_wid, false,
		false);
	if (!gauntlet) return NULL;
	gauntlet->depth = p->depth;

	left = cavern_chunk(p->depth, y_size, x_size);
	if (!left) {
		cave_free(gauntlet);
		return NULL;
	}
	left->depth = p->depth;

	right = cavern_chunk(p->depth, y_size, x_size);
	if (!right) {
		cave_free(gauntlet);
		cave_free(left);
		return NULL;
	}
	right->depth = p->depth;

	/* Record lines between chunks */
	line1 = left->width;
	line2 = line1 + gauntlet->width;

	/* Set the movement and mapping restrictions */
	generate_mark(left, 0, 0, left->height - 1, left->width - 1,
		SQUARE_NO_TELEPORT);
	generate_mark(gauntlet, 0, 0, gauntlet->height - 1, gauntlet->width - 1,
		SQUARE_NO_MAP);
	generate_mark(gauntlet, 0, 0, gauntlet->height - 1, gauntlet->width - 1,
		SQUARE_NO_TELEPORT);

	/* Place down stairs in the right cavern */
	alloc_stairs(right, FEAT_MORE, rand_range(2, 3));

	/* Place up stairs in the left cavern */
	alloc_stairs(left, FEAT_LESS, rand_range(1, 3));

	/*
	 * Open the ends of the gauntlet.  Make sure the opening is
	 * horizontally adjacent to a non-permanent wall for interoperability
	 * with ensure_connectedness().
	 */
	i = 0;
	while (1) {
		struct loc grid = loc(0, randint1(gauntlet->height - 2));

		if (i >= 20) {
			cave_free(gauntlet);
			cave_free(left);
			cave_free(right);
			return NULL;
		}
		if (!square_isperm(gauntlet, loc_sum(grid, loc(1, 0)))) {
			square_set_feat(gauntlet, grid, FEAT_GRANITE);
			break;
		}
		++i;
	}
	i = 0;
	while (1) {
		struct loc grid = loc(gauntlet->width - 1,
			randint1(gauntlet->height - 2));

		if (i >= 20) {
			cave_free(gauntlet);
			cave_free(left);
			cave_free(right);
			return NULL;
		}
		if (!square_isperm(gauntlet, loc_sum(grid, loc(-1, 0)))) {
			square_set_feat(gauntlet, grid, FEAT_GRANITE);
			break;
		}
		++i;
	}

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(p->depth / 3, 10), 2) / 2;

	/* Put the character in the arrival cavern */
	new_player_spot(p->upkeep->create_down_stair ? right : left, p);

	/* Pick some monsters for the left cavern */
	i = z_info->level_monster_min + randint1(4) + k;

	/* Place the monsters */
	for (; i > 0; i--)
		pick_and_place_distant_monster(left, p, 0, true, left->depth);

	/* Pick some of monsters for the right cavern */
	i = z_info->level_monster_min + randint1(4) + k;

	/* Place the monsters */
	for (; i > 0; i--)
		pick_and_place_distant_monster(right, p, 0, true, right->depth);

	/* Pick a larger number of monsters for the gauntlet */
	i = (z_info->level_monster_min + randint1(6) + k);

	/* Find appropriate monsters */
	while (true) {
		/* Choose a pit profile */
		set_pit_type(gauntlet->depth, 0);

		/* Set monster generation restrictions */
		if (mon_restrict(dun->pit_type->name, gauntlet->depth, true))
			break;
	}

	ROOM_LOG("Gauntlet - %s", dun->pit_type->name);

	/* Place labyrinth monsters */
	spread_monsters(gauntlet, dun->pit_type->name, gauntlet->depth, i,
					gauntlet->height / 2, gauntlet->width / 2,
					gauntlet->height / 2, gauntlet->width / 2,
					ORIGIN_LABYRINTH);

	/* Remove our restrictions. */
	(void) mon_restrict(NULL, gauntlet->depth, false);

	/* Make the level */
	c = cave_new(y_size, left->width + gauntlet->width + right->width);
	c->depth = p->depth;

	/* Fill cave area with basic granite */
	fill_rectangle(c, 0, 0, c->height - 1, c->width - 1,
		FEAT_GRANITE, SQUARE_NONE);

	/* Fill the area between the caverns with permanent rock */
	fill_rectangle(c, 0, line1, c->height - 1, line2 - 1, FEAT_PERM,
		SQUARE_NONE);

	/* Copy in the pieces */
	chunk_copy(c, left, 0, 0, 0, false);
	chunk_copy(c, gauntlet, (y_size - gauntlet->height) / 2, line1, 0, false);
	chunk_copy(c, right, 0, line2, 0, false);

	/* Free the chunks */
	cave_free(left);
	cave_free(gauntlet);
	cave_free(right);

	/* Generate permanent walls around the edge of the generated area */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
		FEAT_PERM, SQUARE_NONE, true);

	/* Connect */
	ensure_connectedness(c, true);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_CORR, TYP_TRAP, randint1(k), c->depth, 0);

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT,
		Rand_normal(z_info->room_item_av, 3), c->depth, ORIGIN_FLOOR);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT,
		Rand_normal(z_info->both_item_av, 3), c->depth, ORIGIN_FLOOR);
	alloc_objects(c, SET_BOTH, TYP_GOLD,
		Rand_normal(z_info->both_gold_av, 3), c->depth, ORIGIN_FLOOR);

	return c;
}

/* ------------------ ARENA ---------------- */

/**
 * Generate an arena level - an open single combat arena.
 *
 * \param p is the player
 * \return a pointer to the generated chunk
 */
struct chunk *arena_gen(struct player *p, int min_height, int min_width) {
	struct chunk *c;
	struct monster *mon = player->upkeep->health_who;

	c = cave_new(min_height, min_width);
	c->depth = p->depth;
	c->name = string_make("arena");

	/* Fill cave area with floors */
	fill_rectangle(c, 0, 0, c->height - 1, c->width - 1, FEAT_FLOOR,
		SQUARE_NONE);

	/* Bound with perma-rock */
	draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, FEAT_PERM,
		SQUARE_NONE, true);

	/* Place the player */
	player_place(c, p, loc(1, c->height - 2));

	/* Place the monster */
	memcpy(&c->monsters[mon->midx], mon, sizeof(*mon));
	mon = &c->monsters[mon->midx];
	mon->grid = loc(c->width - 2, 1);
	square_set_mon(c, mon->grid, mon->midx);
	c->mon_max = mon->midx + 1;
	c->mon_cnt = 1;
	update_mon(mon, c, true);
	player->upkeep->health_who = mon;

	/* Ignore its held objects */
	mon->held_obj = NULL;

	/* Give it a group */
	monster_group_start(c, mon, 0);

	return c;
}
