/*
 * File: gen-cave.c
 * Purpose: Dungeon generation.
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
 */

#include "angband.h"
#include "cave.h"
#include "dungeon.h"
#include "files.h"
#include "game-event.h"
#include "generate.h"
#include "init.h"
#include "math.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "parser.h"
#include "store.h"
#include "tables.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

static bool square_is_granite_with_flag(struct cave *c, int y, int x, int flag)
{
	if (c->feat[y][x] != FEAT_GRANITE) return FALSE;
	if (!sqinfo_has(c->info[y][x], flag)) return FALSE;

	return TRUE;
}

/**
 * Places a streamer of rock through dungeon.
 *
 * Note that their are actually six different terrain features used to
 * represent streamers. Three each of magma and quartz, one for basic vein, one
 * with hidden gold, and one with known gold. The hidden gold types are
 * currently unused.
 */
static void build_streamer(struct cave *c, int feat, int chance)
{
    int i, tx, ty;
    int y, x, dir;

    /* Hack -- Choose starting point */
    y = rand_spread(c->height / 2, 10);
    x = rand_spread(c->width / 2, 15);

    /* Choose a random direction */
    dir = ddd[randint0(8)];

    /* Place streamer into dungeon */
    while (TRUE) {
		/* One grid per density */
		for (i = 0; i < dun->profile->str.den; i++) {
			int d = dun->profile->str.rng;

			/* Pick a nearby grid */
			find_nearby_grid(c, &ty, y, d, &tx, x, d);

			/* Only convert walls */
			if (square_isrock(c, ty, tx)) {
				/* Turn the rock into the vein type */
				square_set_feat(c, ty, tx, feat);

				/* Sometimes add known treasure */
				if (one_in_(chance)) upgrade_mineral(c, ty, tx);
			}
		}

		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Stop at dungeon edge */
		if (!square_in_bounds(c, y, x)) break;
    }
}


/**
 * Constructs a tunnel between two points
 *
 * This function must be called BEFORE any streamers are created, since we use
 * the special "granite wall" sub-types to keep track of legal places for
 * corridors to pierce rooms.
 *
 * We queue the tunnel grids to prevent door creation along a corridor which
 * intersects itself.
 *
 * We queue the wall piercing grids to prevent a corridor from leaving
 * a room and then coming back in through the same entrance.
 *
 * We pierce grids which are outer walls of rooms, and when we do so, we change
 * all adjacent outer walls of rooms into solid walls so that no two corridors
 * may use adjacent grids for exits.
 *
 * The solid wall check prevents corridors from chopping the corners of rooms
 * off, as well as silly door placement, and excessively wide room entrances.
 */
static void build_tunnel(struct cave *c, int row1, int col1, int row2, int col2)
{
    int i, y, x;
    int tmp_row, tmp_col;
    int row_dir, col_dir;
    int start_row, start_col;
    int main_loop_count = 0;

    /* Used to prevent excessive door creation along overlapping corridors. */
    bool door_flag = FALSE;
	
    /* Reset the arrays */
    dun->tunn_n = 0;
    dun->wall_n = 0;
	
    /* Save the starting location */
    start_row = row1;
    start_col = col1;

    /* Start out in the correct direction */
    correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

    /* Keep going until done (or bored) */
    while ((row1 != row2) || (col1 != col2)) {
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) break;

		/* Allow bends in the tunnel */
		if (randint0(100) < dun->profile->tun.chg) {
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (randint0(100) < dun->profile->tun.rnd)
				rand_dir(&row_dir, &col_dir);
		}

		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;

		while (!square_in_bounds(c, tmp_row, tmp_col)) {
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (randint0(100) < dun->profile->tun.rnd)
				rand_dir(&row_dir, &col_dir);

			/* Get the next location */
			tmp_row = row1 + row_dir;
			tmp_col = col1 + col_dir;
		}


		/* Avoid the edge of the dungeon */
		if (square_isperm(c, tmp_row, tmp_col)) continue;

		/* Avoid "solid" granite walls */
		if (square_is_granite_with_flag(c, tmp_row, tmp_col, 
										SQUARE_WALL_SOLID)) 
			continue;

		/* Pierce "outer" walls of rooms */
		if (square_is_granite_with_flag(c, tmp_row, tmp_col, 
										SQUARE_WALL_OUTER)) {
			/* Get the "next" location */
			y = tmp_row + row_dir;
			x = tmp_col + col_dir;

			/* Hack -- Avoid solid permanent walls */
			if (square_isperm(c, y, x)) continue;

			/* Hack -- Avoid outer/solid granite walls */
			if (square_is_granite_with_flag(c, y, x, SQUARE_WALL_OUTER)) 
				continue;
			if (square_is_granite_with_flag(c, y, x, SQUARE_WALL_SOLID)) 
				continue;

			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the wall location */
			if (dun->wall_n < WALL_MAX) {
				dun->wall[dun->wall_n].y = row1;
				dun->wall[dun->wall_n].x = col1;
				dun->wall_n++;
			}

			/* Forbid re-entry near this piercing */
			for (y = row1 - 1; y <= row1 + 1; y++)
				for (x = col1 - 1; x <= col1 + 1; x++)
					if (square_is_granite_with_flag(c, y, x, SQUARE_WALL_OUTER))
						set_marked_granite(c, y, x, SQUARE_WALL_SOLID);

		} else if (sqinfo_has(c->info[tmp_row][tmp_col], SQUARE_ROOM)) {
			/* Travel quickly through rooms */
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

		} else if (tf_has(f_info[c->feat[tmp_row][tmp_col]].flags, TF_GRANITE)||
				   tf_has(f_info[c->feat[tmp_row][tmp_col]].flags, TF_PERMANENT)){
			/* Tunnel through all other walls */
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX) {
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			door_flag = FALSE;

		} else {
			/* Handle corridor intersections or overlaps */
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Collect legal door locations */
			if (!door_flag) {
				/* Save the door location */
				if (dun->door_n < DOOR_MAX) {
					dun->door[dun->door_n].y = row1;
					dun->door[dun->door_n].x = col1;
					dun->door_n++;
				}

				/* No door in next grid */
				door_flag = TRUE;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if (randint0(100) >= dun->profile->tun.con) {
				/* Distance between row1 and start_row */
				tmp_row = row1 - start_row;
				if (tmp_row < 0) tmp_row = (-tmp_row);

				/* Distance between col1 and start_col */
				tmp_col = col1 - start_col;
				if (tmp_col < 0) tmp_col = (-tmp_col);

				/* Terminate the tunnel */
				if ((tmp_row > 10) || (tmp_col > 10)) break;
			}
		}
    }


    /* Turn the tunnel into corridor */
    for (i = 0; i < dun->tunn_n; i++) {
		/* Get the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Clear previous contents, add a floor */
		square_set_feat(c, y, x, FEAT_FLOOR);
    }


    /* Apply the piercings that we found */
    for (i = 0; i < dun->wall_n; i++) {
		/* Get the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Convert to floor grid */
		square_set_feat(c, y, x, FEAT_FLOOR);

		/* Place a random door */
		if (randint0(100) < dun->profile->tun.pen)
			place_random_door(c, y, x);
    }
}

/**
 * Count the number of corridor grids adjacent to the given grid.
 *
 * This routine currently only counts actual "empty floor" grids which are not
 * in rooms.
 *
 * TODO: count stairs, open doors, closed doors?
 */
static int next_to_corr(struct cave *c, int y1, int x1)
{
    int i, k = 0;
    assert(square_in_bounds(c, y1, x1));

    /* Scan adjacent grids */
    for (i = 0; i < 4; i++) {
		/* Extract the location */
		int y = y1 + ddy_ddd[i];
		int x = x1 + ddx_ddd[i];

		/* Count only floors which aren't part of rooms */
		if (square_isfloor(c, y, x) && !square_isroom(c, y, x)) k++;
    }

    /* Return the number of corridors */
    return k;
}

/**
 * Returns whether a doorway can be built in a space.
 *
 * To have a doorway, a space must be adjacent to at least two corridors and be
 * between two walls.
 */
static bool possible_doorway(struct cave *c, int y, int x)
{
    assert(square_in_bounds(c, y, x));
    if (next_to_corr(c, y, x) < 2)
		return FALSE;
    else if (square_isstrongwall(c, y - 1, x) && square_isstrongwall(c, y + 1, x))
		return TRUE;
    else if (square_isstrongwall(c, y, x - 1) && square_isstrongwall(c, y, x + 1))
		return TRUE;
    else
		return FALSE;
}


/**
 * Places door at y, x position if at least 2 walls found
 */
static void try_door(struct cave *c, int y, int x)
{
    assert(square_in_bounds(c, y, x));

    if (square_isstrongwall(c, y, x)) return;
    if (square_isroom(c, y, x)) return;

    if (randint0(100) < dun->profile->tun.jct && possible_doorway(c, y, x))
		place_random_door(c, y, x);
}


/**
 * 
 */
static void set_cave_dimensions(struct cave *c, int h, int w)
{
    int i, n = h * w;
    c->height = h;
    c->width = w;
    if (cave_squares != NULL) FREE(cave_squares);
    cave_squares = C_ZNEW(n, int);
    for (i = 0; i < n; i++) cave_squares[i] = i;
}


/**
 * Generate a new dungeon level.
 */
bool classic_gen(struct cave *c, struct player *p) {
    int i, j, k, y, x, y1, x1;
    int by, bx = 0, tby, tbx, key, rarity, built;
    int num_rooms, size_percent;
    int dun_unusual = dun->profile->dun_unusual;

    bool **blocks_tried;

    /* Possibly generate fewer rooms in a smaller area via a scaling factor.
     * Since we scale row_rooms and col_rooms by the same amount, DUN_ROOMS
     * gives the same "room density" no matter what size the level turns out
     * to be. TODO: vary room density slightly? */

    /* XXX: Until vault generation is improved, scaling variance is removed */
    i = randint1(10) + c->depth / 24;
    if (is_quest(c->depth)) size_percent = 100;
    else if (i < 2) size_percent = 75;
    else if (i < 3) size_percent = 80;
    else if (i < 4) size_percent = 85;
    else if (i < 5) size_percent = 90;
    else if (i < 6) size_percent = 95;
    else size_percent = 100;

    /* scale the various generation variables */
    num_rooms = (dun->profile->dun_rooms * size_percent) / 100;
    set_cave_dimensions(c, DUNGEON_HGT, DUNGEON_WID);
    ROOM_LOG("height=%d  width=%d  nrooms=%d", c->height, c->width, num_rooms);

    /* Fill whole level with perma-rock */
    fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, 
				   FEAT_PERM, SQUARE_NONE);

    /* Fill cave area with basic granite */
    fill_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
				   FEAT_GRANITE, SQUARE_NONE);

    /* Actual maximum number of rooms on this level */
    dun->row_rooms = c->height / BLOCK_HGT;
    dun->col_rooms = c->width / BLOCK_WID;

    /* Initialize the room table */
	dun->room_map = mem_zalloc(dun->row_rooms * sizeof(bool*));
	for (i = 0; i < dun->row_rooms; i++)
		dun->room_map[i] = mem_zalloc(dun->col_rooms * sizeof(bool));

    /* Initialize the block table */
    blocks_tried = mem_zalloc(dun->row_rooms * sizeof(bool*));

	for (i = 0; i < dun->row_rooms; i++)
		blocks_tried[i] = mem_zalloc(dun->col_rooms * sizeof(bool));

    /* No rooms yet, pits or otherwise. */
    dun->pit_num = 0;
    dun->cent_n = 0;

    /* Build some rooms */
    built = 0;
    while(built < num_rooms) {

		/* Count the room blocks we haven't tried yet. */
		j = 0;
		tby = 0;
		tbx = 0;
		for(by = 0; by < dun->row_rooms; by++) {
			for(bx = 0; bx < dun->col_rooms; bx++) {
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
		blocks_tried[by][bx] = TRUE;

		/* Roll for random key (to be compared against a profile's cutoff) */
		key = randint0(100);

		/* We generate a rarity number to figure out how exotic to make the
		 * room. This number has a depth/DUN_UNUSUAL chance of being > 0,
		 * a depth^2/DUN_UNUSUAL^2 chance of being > 1, up to MAX_RARITY. */
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
			
			if (room_build(c, by, bx, profile, FALSE)) {
				built++;
				break;
			}
		}
    }

	for (i = 0; i < dun->row_rooms; i++){
		mem_free(blocks_tried[i]);
		mem_free(dun->room_map[i]);
	}
	mem_free(blocks_tried);
	mem_free(dun->room_map);

    /* Generate permanent walls around the edge of the generated area */
    draw_rectangle(c, 0, 0, c->height - 1, c->width - 1, 
				   FEAT_PERM, SQUARE_NONE);

    /* Hack -- Scramble the room order */
    for (i = 0; i < dun->cent_n; i++) {
		int pick1 = randint0(dun->cent_n);
		int pick2 = randint0(dun->cent_n);
		y1 = dun->cent[pick1].y;
		x1 = dun->cent[pick1].x;
		dun->cent[pick1].y = dun->cent[pick2].y;
		dun->cent[pick1].x = dun->cent[pick2].x;
		dun->cent[pick2].y = y1;
		dun->cent[pick2].x = x1;
    }

    /* Start with no tunnel doors */
    dun->door_n = 0;

    /* Hack -- connect the first room to the last room */
    y = dun->cent[dun->cent_n-1].y;
    x = dun->cent[dun->cent_n-1].x;

    /* Connect all the rooms together */
    for (i = 0; i < dun->cent_n; i++) {
		/* Connect the room to the previous room */
		build_tunnel(c, dun->cent[i].y, dun->cent[i].x, y, x);

		/* Remember the "previous" room */
		y = dun->cent[i].y;
		x = dun->cent[i].x;
    }

    /* Place intersection doors */
    for (i = 0; i < dun->door_n; i++) {
		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try placing doors */
		try_door(c, y, x - 1);
		try_door(c, y, x + 1);
		try_door(c, y - 1, x);
		try_door(c, y + 1, x);
    }

    ensure_connectedness(c);

    /* Add some magma streamers */
    for (i = 0; i < dun->profile->str.mag; i++)
		build_streamer(c, FEAT_MAGMA, dun->profile->str.mc);

    /* Add some quartz streamers */
    for (i = 0; i < dun->profile->str.qua; i++)
		build_streamer(c, FEAT_QUARTZ, dun->profile->str.qc);

    /* Place 3 or 4 down stairs near some walls */
    alloc_stairs(c, FEAT_MORE, rand_range(3, 4), 3);

    /* Place 1 or 2 up stairs near some walls */
    alloc_stairs(c, FEAT_LESS, rand_range(1, 2), 3);

    /* General amount of rubble, traps and monsters */
    k = MAX(MIN(c->depth / 3, 10), 2);

    /* Put some rubble in corridors */
    alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

    /* Place some traps in the dungeon */
    alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth, 0);

    /* Determine the character location */
    new_player_spot(c, p);

    /* Pick a base number of monsters */
    i = MIN_M_ALLOC_LEVEL + randint1(8) + k;

    /* Put some monsters in the dungeon */
    for (; i > 0; i--)
		pick_and_place_distant_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

    /* Put some objects in rooms */
    alloc_objects(c, SET_ROOM, TYP_OBJECT, Rand_normal(AMT_ROOM, 3),
				  c->depth, ORIGIN_FLOOR);

    /* Put some objects/gold in the dungeon */
    alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(AMT_ITEM, 3),
				  c->depth, ORIGIN_FLOOR);
    alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(AMT_GOLD, 3),
				  c->depth, ORIGIN_FLOOR);

    return TRUE;
}


/* ------------------ LABYRINTH ---------------- */

/**
 * Used to convert (x, y) into an array index (i) in labyrinth_gen().
 */
static int lab_toi(int y, int x, int w) {
    return y * w + x;
}

/**
 * Used to convert an array index (i) into (x, y) in labyrinth_gen().
 */
static void lab_toyx(int i, int w, int *y, int *x) {
    *y = i / w;
    *x = i % w;
}

/**
 * Given an adjoining wall (a wall which separates two labyrinth cells)
 * set a and b to point to the cell indices which are separated. Used by
 * labyrinth_gen().
 */
static void lab_get_adjoin(int i, int w, int *a, int *b) {
    int y, x;
    lab_toyx(i, w, &y, &x);
    if (x % 2 == 0) {
		*a = lab_toi(y - 1, x, w);
		*b = lab_toi(y + 1, x, w);
    } else {
		*a = lab_toi(y, x - 1, w);
		*b = lab_toi(y, x + 1, w);
    }
}

/**
 * Return whether (x, y) is in a tunnel.
 *
 * For our purposes a tunnel is a horizontal or vertical path, not an
 * intersection. Thus, we want the squares on either side to walls in one
 * case (e.g. up/down) and open in the other case (e.g. left/right). We don't
 * want a square that represents an intersection point.
 *
 * The high-level idea is that these are squares which can't be avoided (by
 * walking diagonally around them).
 */
static bool lab_is_tunnel(struct cave *c, int y, int x) {
    bool west = square_isopen(c, y, x - 1);
    bool east = square_isopen(c, y, x + 1);
    bool north = square_isopen(c, y - 1, x);
    bool south = square_isopen(c, y + 1, x);

    return north == south && west == east && north != west;
}


/**
 * Build a labyrinth level.
 *
 * Note that if the function returns FALSE, a level wasn't generated.
 * Labyrinths use the dungeon level's number to determine whether to generate
 * themselves (which means certain level numbers are more likely to generate
 * labyrinths than others).
 */
bool labyrinth_gen(struct cave *c, struct player *p) {
    int i, j, k, y, x;

    /* Size of the actual labyrinth part must be odd. */
    /* NOTE: these are not the actual dungeon size, but rather the size of the
     * area we're genearting a labyrinth in (which doesn't count theh enclosing
     * outer walls. */
    int h = 15 + randint0(c->depth / 10) * 2;
    int w = 51 + randint0(c->depth / 10) * 2;

    /* This is the number of squares in the labyrinth */
    int n = h * w;

    /* NOTE: 'sets' and 'walls' are too large... we only need to use about
     * 1/4 as much memory. However, in that case, the addressing math becomes
     * a lot more complicated, so let's just stick with this because it's
     * easier to read. */

    /* 'sets' tracks connectedness; if sets[i] == sets[j] then cells i and j
     * are connected to each other in the maze. */
    int *sets;

    /* 'walls' is a list of wall coordinates which we will randomize */
    int *walls;

    /* Most labyrinths are lit */
    bool lit = randint0(c->depth) < 25 || randint0(2) < 1;

    /* Many labyrinths are known */
    bool known = lit && randint0(c->depth) < 25;

    /* Most labyrinths have soft (diggable) walls */
    bool soft = randint0(c->depth) < 35 || randint0(3) < 2;

    /* There's a base 2 in 100 to accept the labyrinth */
    int chance = 2;

    /* If we're too shallow then don't do it */
    if (c->depth < 13) return FALSE;

    /* Don't try this on quest levels, kids... */
    if (is_quest(c->depth)) return FALSE;

    /* Certain numbers increase the chance of having a labyrinth */
    if (c->depth % 3 == 0) chance += 1;
    if (c->depth % 5 == 0) chance += 1;
    if (c->depth % 7 == 0) chance += 1;
    if (c->depth % 11 == 0) chance += 1;
    if (c->depth % 13 == 0) chance += 1;

    /* Only generate the level if we pass a check */
    /* NOTE: This test gets performed after we pass the test to use the
     * labyrinth cave profile. */
    if (randint0(100) >= chance) return FALSE;

    /* allocate our arrays */
    sets = C_ZNEW(n, int);
    walls = C_ZNEW(n, int);

    /* This is the dungeon size, which does include the enclosing walls */
    set_cave_dimensions(c, h + 2, w + 2);

    /* Fill whole level with perma-rock */
    fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, 
				   FEAT_PERM, SQUARE_NONE);

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
    for (y = 0; y < h; y += 2) {
		for (x = 0; x < w; x += 2) {
			int k = lab_toi(y, x, w);
			sets[k] = k;
			square_set_feat(c, y + 1, x + 1, FEAT_FLOOR);
			if (lit) sqinfo_on(c->info[y + 1][x + 1], SQUARE_GLOW);
		}
    }

    /* Shuffle the walls, using Knuth's shuffle. */
    shuffle(walls, n);

    /* For each adjoining wall, look at the cells it divides. If they aren't
     * in the same set, remove the wall and join their sets.
     *
     * This is a randomized version of Kruskal's algorithm. */
    for (i = 0; i < n; i++) {
		int a, b, x, y;

		j = walls[i];

		/* If this cell isn't an adjoining wall, skip it */
		lab_toyx(j, w, &y, &x);
		if ((x < 1 && y < 1) || (x > w - 2 && y > h - 2)) continue;
		if (x % 2 == y % 2) continue;

		/* Figure out which cells are separated by this wall */
		lab_get_adjoin(j, w, &a, &b);

		/* If the cells aren't connected, kill the wall and join the sets */
		if (sets[a] != sets[b]) {
			int sa = sets[a];
			int sb = sets[b];
			square_set_feat(c, y + 1, x + 1, FEAT_FLOOR);
			if (lit) sqinfo_on(c->info[y + 1][x + 1], SQUARE_GLOW);

			for (k = 0; k < n; k++) {
				if (sets[k] == sb) sets[k] = sa;
			}
		}
    }

    /* Determine the character location */
    new_player_spot(c, p);

    /* Generate a single set of stairs up if necessary. */
    if (!cave_find(c, &y, &x, square_isupstairs))
		alloc_stairs(c, FEAT_LESS, 1, 3);

    /* Generate a single set of stairs down if necessary. */
    if (!cave_find(c, &y, &x, square_isdownstairs))
		alloc_stairs(c, FEAT_MORE, 1, 3);

    /* Generate a door for every 100 squares in the labyrinth */
    for (i = n / 100; i > 0; i--) {
		/* Try 10 times to find a useful place for a door, then place it */
		for (j = 0; j < 10; j++) {
			find_empty(c, &y, &x);
			if (lab_is_tunnel(c, y, x)) break;

		}

		place_closed_door(c, y, x);
    }

    /* General some rubble, traps and monsters */
    k = MAX(MIN(c->depth / 3, 10), 2);

    /* Scale number of monsters items by labyrinth size */
    k = (3 * k * (h * w)) / (DUNGEON_HGT * DUNGEON_WID);

    /* Put some rubble in corridors */
    alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);

    /* Place some traps in the dungeon */
    alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth, 0);

    /* Put some monsters in the dungeon */
    for (i = MIN_M_ALLOC_LEVEL + randint1(8) + k; i > 0; i--)
		pick_and_place_distant_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

    /* Put some objects/gold in the dungeon */
    alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(k * 6, 2), c->depth,
				  ORIGIN_LABYRINTH);
    alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(k * 3, 2), c->depth,
				  ORIGIN_LABYRINTH);
    alloc_objects(c, SET_BOTH, TYP_GOOD, randint1(2), c->depth,
				  ORIGIN_LABYRINTH);

    /* Unlit labyrinths will have some good items */
    if (!lit)
		alloc_objects(c, SET_BOTH, TYP_GOOD, Rand_normal(3, 2), c->depth,
					  ORIGIN_LABYRINTH);

    /* Hard (non-diggable) labyrinths will have some great items */
    if (!soft)
		alloc_objects(c, SET_BOTH, TYP_GREAT, Rand_normal(2, 1), c->depth,
					  ORIGIN_LABYRINTH);

    /* If we want the players to see the maze layout, do that now */
    if (known) wiz_light(FALSE);

    /* Deallocate our lists */
    FREE(sets);
    FREE(walls);

    return TRUE;
}


/* ---------------- CAVERNS ---------------------- */

/**
 * Initialize the dungeon array, with a random percentage of squares open.
 */
static void init_cavern(struct cave *c, struct player *p, int density) {
    int h = c->height;
    int w = c->width;
    int size = h * w;
	
    int count = (size * density) / 100;

    /* Fill the edges with perma-rock, and rest with rock */
    fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, 
				   FEAT_PERM, SQUARE_NONE);
    fill_rectangle(c, 1, 1, h - 2, w - 2, FEAT_GRANITE, SQUARE_WALL_SOLID);
	
    while (count > 0) {
		int y = randint1(h - 2);
		int x = randint1(w - 2);
		if (square_isrock(c, y, x)) {
			square_set_feat(c, y, x, FEAT_FLOOR);
			count--;
		}
    }
}

/**
 * Return the number of walls (0-8) adjacent to this square.
 */
static int count_adj_walls(struct cave *c, int y, int x) {
    int yd, xd;
    int count = 0;

    for (yd = -1; yd <= 1; yd++) {
		for (xd = -1; xd <= 1; xd++) {
			if (yd == 0 && xd == 0) continue;
			if (square_isfloor(c, y + yd, x + xd)) continue;
			count++;
		}
    }

    return count;
}

/**
 * Run a single pass of the cellular automata rules (4,5) on the dungeon.
 */
static void mutate_cavern(struct cave *c) {
    int y, x;
    int h = c->height;
    int w = c->width;

    int *temp = C_ZNEW(h * w, int);

    for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			int count = count_adj_walls(c, y, x);
			if (count > 5)
				temp[y * w + x] = FEAT_GRANITE;
			else if (count < 4)
				temp[y * w + x] = FEAT_FLOOR;
			else
				temp[y * w + x] = cave->feat[y][x];
		}
    }

    for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			square_set_feat(c, y, x, temp[y * w + x]);
		}
    }

    FREE(temp);
}

/**
 * Fill an int[] with a single value.
 */
static void array_filler(int data[], int value, int size) {
    int i;
    for (i = 0; i < size; i++) data[i] = value;
}

/**
 * Determine if we need to worry about coloring a point, or can ignore it.
 */
static int ignore_point(struct cave *c, int colors[], int y, int x) {
    int h = c->height;
    int w = c->width;
    int n = lab_toi(y, x, w);

    if (y < 0 || x < 0 || y >= h || x >= w) return TRUE;
    if (colors[n]) return TRUE;
    //if (square_isvault(c, y, x)) return TRUE;
    if (square_isvault(c, y, x)) return FALSE;
    if (square_ispassable(c, y, x)) return FALSE;
    if (square_isdoor(c, y, x)) return FALSE;
    return TRUE;
}

static int xds[] = {0, 0, 1, -1, -1, -1, 1, 1};
static int yds[] = {1, -1, 0, 0, -1, 1, -1, 1};

#if 0 /* XXX d_m - is this meant to be in use? */
static void glow_point(struct cave *c, int y, int x) {
    int i, j;
    for (i = -1; i <= -1; i++)
		for (j = -1; j <= -1; j++)
			sqinfo_on(c->info[y + i][x + j], SQUARE_GLOW);
}
#endif

/**
 * Color a particular point, and all adjacent points.
 */
static void build_color_point(struct cave *c, int colors[], int counts[], int y, int x, int color, bool diagonal) {
    int h = c->height;
    int w = c->width;
    int size = h * w;
    struct queue *queue = q_new(size);

    int dslimit = diagonal ? 8 : 4;

    int *added = C_ZNEW(size, int);
    array_filler(added, 0, size);

    q_push_int(queue, lab_toi(y, x, w));

    counts[color] = 0;

    while (q_len(queue) > 0) {
		int i, y2, x2;
		int n2 = q_pop_int(queue);

		lab_toyx(n2, w, &y2, &x2);

		if (ignore_point(cave, colors, y2, x2)) continue;

		colors[n2] = color;
		counts[color]++;

		/*if (lit) glow_point(c, y2, x2);*/

		for (i = 0; i < dslimit; i++) {
			int y3 = y2 + yds[i];
			int x3 = x2 + xds[i];
			int n3 = lab_toi(y3, x3, w);
			if (ignore_point(cave, colors, y3, x3)) continue;
			if (added[n3]) continue;

			q_push_int(queue, n3);
			added[n3] = 1;
		}
    }

    FREE(added);
    q_free(queue);
}

/**
 * Create a color for each "NESW contiguous" region of the dungeon.
 */
static void build_colors(struct cave *c, int colors[], int counts[], bool diagonal) {
    int y, x;
    int h = c->height;
    int w = c->width;
    int color = 1;

    for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (ignore_point(cave, colors, y, x)) continue;
			build_color_point(cave, colors, counts, y, x, color, diagonal);
			color++;
		}
    }
}

/**
 * Find and delete all small (<9 square) open regions.
 */
static void clear_small_regions(struct cave *c, int colors[], int counts[]) {
    int i, y, x;
    int h = c->height;
    int w = c->width;
    int size = h * w;

    int *deleted = C_ZNEW(size, int);
    array_filler(deleted, 0, size);

    for (i = 0; i < size; i++) {
		if (counts[i] < 9) {
			deleted[i] = 1;
			counts[i] = 0;
		}
    }

    for (y = 1; y < c->height - 1; y++) {
		for (x = 1; x < c->width - 1; x++) {
			i = lab_toi(y, x, w);

			if (!deleted[colors[i]]) continue;

			colors[i] = 0;
			set_marked_granite(c, y, x, SQUARE_WALL_SOLID);
		}
    }
    FREE(deleted);
}

/**
 * Return the number of colors which have active cells.
 */
static int count_colors(int counts[], int size) {
    int i;
    int num = 0;
    for (i = 0; i < size; i++) if (counts[i] > 0) num++;
    return num;
}

/**
 * Return the first color which has one or more active cells.
 */
static int first_color(int counts[], int size) {
    int i;
    for (i = 0; i < size; i++) if (counts[i] > 0) return i;
    return -1;
}

/**
 * Find all cells of 'fromcolor' and repaint them to 'tocolor'.
 */
static void fix_colors(int colors[], int counts[], int from, int to, int size) {
    int i;
    for (i = 0; i < size; i++) if (colors[i] == from) colors[i] = to;
    counts[to] += counts[from];
    counts[from] = 0;
}

/**
 * Create a tunnel connecting a region to one of its nearest neighbors.
 */
static void join_region(struct cave *c, int colors[], int counts[], int color) {
    int i;
    int h = c->height;
    int w = c->width;
    int size = h * w;

    /* Allocate a processing queue */
    struct queue *queue = q_new(size);

    /* Allocate an array to keep track of handled squares, and which square
     * we reached them from.
     */
    int *previous = C_ZNEW(size, int);
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
		int n = q_pop_int(queue);
		int color2 = colors[n];

		/* See if we've reached a square with a new color */
		if (color2 && color2 != color) {
			/* Step backward through the path, turning stone to tunnel */
			while (colors[n] != color) {
				int x, y;
				lab_toyx(n, w, &y, &x);
				colors[n] = color;
				if (!square_isperm(c, y, x) && !square_isvault(c, y, x)) {
					square_set_feat(c, y, x, FEAT_FLOOR);
				}
				n = previous[n];
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
			int y, x, n2;
			lab_toyx(n, w, &y, &x);

			/* Move to the adjacent square */
			y += yds[i];
			x += xds[i];

			/* make sure we stay inside the boundaries */
			if (y < 0 || y >= h) continue;
			if (x < 0 || x >= w) continue;

			/* If the cell hasn't already been procssed, add it to the queue */
			n2 = lab_toi(y, x, w);
			if (previous[n2] >= 0) continue;
			q_push_int(queue, n2);
			previous[n2] = n;
		}
    }

    /* Free the memory we've allocated */
    q_free(queue);
    FREE(previous);
}


/**
 * Start connecting regions, stopping when the cave is entirely connected.
 */
static void join_regions(struct cave *c, int colors[], int counts[]) {
    int h = c->height;
    int w = c->width;
    int size = h * w;
    int num = count_colors(counts, size);

    /* While we have multiple colors (i.e. disconnected regions), join one of
     * the regions to another one.
     */
    while (num > 1) {
		int color = first_color(counts, size);
		join_region(c, colors, counts, color);
		num--;
    }
}


/**
 * Count the number of open cells in the dungeon.
 */
static int open_count(struct cave *c) {
    int x, y;
    int h = c->height;
    int w = c->width;
    int num = 0;
    for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			if (square_ispassable(c, y, x)) num++;
    return num;
}


/**
 * Make sure that all the regions of the dungeon are connected.
 *
 * This function colors each connected region of the dungeon, then uses that
 * information to join them into one conected region.
 */
void ensure_connectedness(struct cave *c) {
    int size = c->height * c->width;
    int *colors = C_ZNEW(size, int);
    int *counts = C_ZNEW(size, int);

    build_colors(c, colors, counts, TRUE);
    join_regions(c, colors, counts);

    FREE(colors);
    FREE(counts);
}


#define MAX_CAVERN_TRIES 10
/**
 * The generator's main function.
 */
bool cavern_gen(struct cave *c, struct player *p) {
    int i, k, openc;

    int h = rand_range(DUNGEON_HGT / 2, (DUNGEON_HGT * 3) / 4);
    int w = rand_range(DUNGEON_WID / 2, (DUNGEON_WID * 3) / 4);
    int size = h * w;
    int limit = size / 13;

    int density = rand_range(25, 40);
    int times = rand_range(3, 6);

    int *colors = C_ZNEW(size, int);
    int *counts = C_ZNEW(size, int);

    int tries = 0;

    bool ok = TRUE;

    set_cave_dimensions(c, h, w);
    ROOM_LOG("cavern h=%d w=%d size=%d density=%d times=%d", h, w, size, density, times);

    if (c->depth < 15) {
		/* If we're too shallow then don't do it */
		ok = FALSE;

    } else {
		/* Start trying to build caverns */
		array_filler(colors, 0, size);
		array_filler(counts, 0, size);
	
		for (tries = 0; tries < MAX_CAVERN_TRIES; tries++) {
			/* Build a random cavern and mutate it a number of times */
			init_cavern(c, p, density);
			for (i = 0; i < times; i++) mutate_cavern(c);
	
			/* If there are enough open squares then we're done */
			openc = open_count(c);
			if (openc >= limit) {
				ROOM_LOG("cavern ok (%d vs %d)", openc, limit);
				break;
			}
			ROOM_LOG("cavern failed--try again (%d vs %d)", openc, limit);
		}

		/* If we couldn't make a big enough cavern then fail */
		if (tries == MAX_CAVERN_TRIES) ok = FALSE;
    }

    if (ok) {
		build_colors(c, colors, counts, FALSE);
		clear_small_regions(c, colors, counts);
		join_regions(c, colors, counts);
	
		/* Place 2-3 down stairs near some walls */
		alloc_stairs(c, FEAT_MORE, rand_range(1, 3), 3);
	
		/* Place 1-2 up stairs near some walls */
		alloc_stairs(c, FEAT_LESS, rand_range(1, 2), 3);
	
		/* General some rubble, traps and monsters */
		k = MAX(MIN(c->depth / 3, 10), 2);
	
		/* Scale number of monsters items by cavern size */
		k = MAX((4 * k * (h *  w)) / (DUNGEON_HGT * DUNGEON_WID), 6);
	
		/* Put some rubble in corridors */
		alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);
	
		/* Place some traps in the dungeon */
		alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth, 0);
	
		/* Determine the character location */
		new_player_spot(c, p);
	
		/* Put some monsters in the dungeon */
		for (i = randint1(8) + k; i > 0; i--)
			pick_and_place_distant_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);
	
		/* Put some objects/gold in the dungeon */
		alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(k, 2), c->depth + 5,
					  ORIGIN_CAVERN);
		alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(k / 2, 2), c->depth,
					  ORIGIN_CAVERN);
		alloc_objects(c, SET_BOTH, TYP_GOOD, randint0(k / 4), c->depth,
					  ORIGIN_CAVERN);
    }

    FREE(colors);
    FREE(counts);

    return ok;
}

/* ------------------ TOWN ---------------- */

/**
 * Builds a store at a given pseudo-location
 *
 * Currently, there is a main street horizontally through the middle of town,
 * and all the shops face it (e.g. the shops on the north side face south).
 */
static void build_store(struct cave *c, int n, int yy, int xx) {
    /* Find the "center" of the store */
    int y0 = yy * 9 + 6;
    int x0 = xx * 14 + 12;

    /* Determine the store boundaries */
    int y1 = y0 - randint1((yy == 0) ? 3 : 2);
    int y2 = y0 + randint1((yy == 1) ? 3 : 2);
    int x1 = x0 - randint1(5);
    int x2 = x0 + randint1(5);

    /* Determine door location, based on which side of the street we're on */
    int dy = (yy == 0) ? y2 : y1;
    int dx = rand_range(x1, x2);

    /* Build an invulnerable rectangular building */
    fill_rectangle(c, y1, x1, y2, x2, FEAT_PERM, SQUARE_NONE);

    /* Clear previous contents, add a store door */
    square_set_feat(c, dy, dx, FEAT_SHOP_HEAD + n);
}


/**
 * Generate the "consistent" town features, and place the player
 *
 * HACK: We seed the simple RNG, so we always get the same town layout,
 * including the size and shape of the buildings, the locations of the
 * doorways, and the location of the stairs. This means that if any of the
 * functions used to build the town change the way they use the RNG, the
 * town layout will be generated differently.
 *
 * XXX: Remove this gross hack when this piece of code is fully reentrant -
 * i.e., when all we need to do is swing a pointer to change caves, we just
 * need to generate the town once (we will also need to save/load the town).
 */
static void town_gen_hack(struct cave *c, struct player *p) {
    int y, x, n, k;
    int rooms[MAX_STORES];

    int n_rows = 2;
    int n_cols = (MAX_STORES + 1) / n_rows;

    /* Switch to the "simple" RNG and use our original town seed */
    Rand_quick = TRUE;
    Rand_value = seed_town;

    /* Prepare an Array of "remaining stores", and count them */
    for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

    /* Place rows of stores */
    for (y = 0; y < n_rows; y++) {
		for (x = 0; x < n_cols; x++) {
			if (n < 1) break;

			/* Pick a remaining store */
			k = randint0(n);

			/* Build that store at the proper location */
			build_store(c, rooms[k], y, x);

			/* Shift the stores down, remove one store */
			rooms[k] = rooms[--n];
		}
    }

    /* Place the stairs */
    find_empty_range(c, &y, 3, TOWN_HGT - 3, &x, 3, TOWN_WID - 3);

    /* Clear previous contents, add down stairs */
    square_set_feat(c, y, x, FEAT_MORE);

    /* Place the player */
    player_place(c, p, y, x);

    /* go back to using the "complex" RNG */
    Rand_quick = FALSE;
}


/**
 * Town logic flow for generation of new town.
 *
 * We start with a fully wiped cave of normal floors. This function does NOT do
 * anything about the owners of the stores, nor the contents thereof. It only
 * handles the physical layout.
 */
bool town_gen(struct cave *c, struct player *p) {
    int i;
    bool daytime = turn % (10 * TOWN_DAWN) < (10 * TOWN_DUSK);
    int residents = daytime ? MIN_M_ALLOC_TD : MIN_M_ALLOC_TN;

    assert(c);

    set_cave_dimensions(c, TOWN_HGT, TOWN_WID);

    /* NOTE: We can't use c->height and c->width here because then there'll be
     * a bunch of empty space in the level that monsters might spawn in (or
     * teleport might take you to, or whatever).
     *
     * TODO: fix this to use c->height and c->width when all the 'choose
     * random location' things honor them.
     */

    /* Start with solid walls, and then create some floor in the middle */
    fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, 
				   FEAT_PERM, SQUARE_NONE);
    fill_rectangle(c, 1, 1, c->height -2, c->width - 2, 
				   FEAT_FLOOR, SQUARE_NONE);

    /* Build stuff */
    town_gen_hack(c, p);

    /* Apply illumination */
    cave_illuminate(c, daytime);

    /* Make some residents */
    for (i = 0; i < residents; i++)
		pick_and_place_distant_monster(c, loc(p->px, p->py), 3, TRUE, c->depth);

    return TRUE;
}


