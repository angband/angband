/*
 * File: gen-util.c
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
#include "math.h"
#include "files.h"
#include "game-event.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "obj-make.h"
#include "obj-tvalsval.h"
#include "obj-util.h"
#include "parser.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/**
 * Shuffle an array using Knuth's shuffle.
 */
void shuffle(int *arr, int n)
{
    int i, j, k;
    for (i = 0; i < n; i++) {
		j = randint0(n - i) + i;
		k = arr[j];
		arr[j] = arr[i];
		arr[i] = k;
    }
}


/**
 * Locate a square in y1 <= y < y2, x1 <= x < x2 which satisfies the given
 * predicate.
 */
static bool _find_in_range(struct cave *c, int *y, int y1, int y2, int *x,
						   int x1, int x2, int *squares, square_predicate pred)
{
    int yd = y2 - y1;
    int xd = x2 - x1;
    int i, n = yd * xd;
    bool found = FALSE;

    /* Test each square in (random) order for openness */
    for (i = 0; i < n && !found; i++) {
		int j = randint0(n - i) + i;
		int k = squares[j];
		squares[j] = squares[i];
		squares[i] = k;

		*y = (k / xd) + y1;
		*x = (k % xd) + x1;
		if (pred(c, *y, *x)) found = TRUE;
    }

    /* Return whether we found an empty square or not. */
    return found;
}


/**
 * Locate a square in the dungeon which satisfies the given predicate.
 */
bool cave_find(struct cave *c, int *y, int *x, square_predicate pred)
{
    int h = c->height;
    int w = c->width;
    return _find_in_range(c, y, 0, h, x, 0, w, cave_squares, pred);
}


/**
 * Locate a square in y1 <= y < y2, x1 <= x < x2 which satisfies the given
 * predicate.
 */
static bool cave_find_in_range(struct cave *c, int *y, int y1, int y2,
							   int *x, int x1, int x2, square_predicate pred)
{
    int yd = y2 - y1;
    int xd = x2 - x1;
    int n = yd * xd;
    int i, found;

    /* Allocate the squares, and randomize their order */
    int *squares = C_ZNEW(n, int);
    for (i = 0; i < n; i++) squares[i] = i;

    /* Do the actual search */
    found = _find_in_range(c, y, y1, y2, x, x1, x2, squares, pred);

    /* Deallocate memory */
    FREE(squares);

    /* Return whether or not we found an empty square */
    return found;
}


/**
 * Locate an empty square for 0 <= y < ymax, 0 <= x < xmax.
 */
bool find_empty(struct cave *c, int *y, int *x)
{
    return cave_find(c, y, x, square_isempty);
}


/**
 * Locate an empty square for y1 <= y < y2, x1 <= x < x2.
 */
bool find_empty_range(struct cave *c, int *y, int y1, int y2, int *x, int x1, int x2)
{
    return cave_find_in_range(c, y, y1, y2, x, x1, x2, square_isempty);
}


/**
 * Locate a grid nearby (y0, x0) within +/- yd, xd.
 */
bool find_nearby_grid(struct cave *c, int *y, int y0, int yd, int *x, int x0, int xd)
{
    int y1 = y0 - yd;
    int x1 = x0 - xd;
    int y2 = y0 + yd + 1;
    int x2 = x0 + xd + 1;
    return cave_find_in_range(c, y, y1, y2, x, x1, x2, square_in_bounds);
}


/**
 * Given two points, pick a valid cardinal direction from one to the other.
 */
void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
    /* Extract vertical and horizontal directions */
    *rdir = CMP(y2, y1);
    *cdir = CMP(x2, x1);

    /* If we only have one direction to go, then we're done */
    if (!*rdir || !*cdir) return;

    /* If we need to go diagonally, then choose a random direction */
    if (randint0(100) < 50)
		*rdir = 0;
    else
		*cdir = 0;
}


/**
 * Pick a random cardinal direction.
 */
void rand_dir(int *rdir, int *cdir)
{
    /* Pick a random direction and extract the dy/dx components */
    int i = randint0(4);
    *rdir = ddy_ddd[i];
    *cdir = ddx_ddd[i];
}


/**
 * Determine whether the given coordinate is a valid starting location.
 */
static bool square_isstart(struct cave *c, int y, int x)
{
    if (!square_isempty(c, y, x)) return FALSE;
    if (square_isvault(c, y, x)) return FALSE;
    return TRUE;
}


/**
 * Place the player at a random starting location.
 */
void new_player_spot(struct cave *c, struct player *p)
{
    int y, x;

    /* Try to find a good place to put the player */
    cave_find_in_range(c, &y, 0, c->height, &x, 0, c->width, square_isstart);

    /* Create stairs the player came down if allowed and necessary */
    if (OPT(birth_no_stairs)) {
    } else if (p->create_down_stair) {
		square_set_feat(c, y, x, FEAT_MORE);
		p->create_down_stair = FALSE;
    } else if (p->create_up_stair) {
		square_set_feat(c, y, x, FEAT_LESS);
		p->create_up_stair = FALSE;
    }

    player_place(c, p, y, x);
}


/**
 * Return how many cardinal directions around (x, y) contain walls.
 */
static int next_to_walls(struct cave *c, int y, int x)
{
    int k = 0;
    assert(square_in_bounds(c, y, x));

    if (square_iswall(c, y + 1, x)) k++;
    if (square_iswall(c, y - 1, x)) k++;
    if (square_iswall(c, y, x + 1)) k++;
    if (square_iswall(c, y, x - 1)) k++;

    return k;
}


/**
 * Place rubble at (x, y).
 */
static void place_rubble(struct cave *c, int y, int x)
{
    square_set_feat(c, y, x, FEAT_RUBBLE);
}


/**
 * Place stairs (of the requested type 'feat' if allowed) at (x, y).
 *
 * All stairs from town go down. All stairs on an unfinished quest level go up.
 */
static void place_stairs(struct cave *c, int y, int x, int feat)
{
    if (!c->depth)
		square_set_feat(c, y, x, FEAT_MORE);
    else if (is_quest(c->depth) || c->depth >= MAX_DEPTH - 1)
		square_set_feat(c, y, x, FEAT_LESS);
    else
		square_set_feat(c, y, x, feat);
}


/**
 * Place random stairs at (x, y).
 */
void place_random_stairs(struct cave *c, int y, int x)
{
    int feat = randint0(100) < 50 ? FEAT_LESS : FEAT_MORE;
    if (square_canputitem(c, y, x)) place_stairs(c, y, x, feat);
}


/**
 * Place a random object at (x, y).
 */
void place_object(struct cave *c, int y, int x, int level, bool good, bool great, byte origin, int tval)
{
    s32b rating = 0;
    object_type otype;

    assert(square_in_bounds(c, y, x));

    if (!square_canputitem(c, y, x)) return;

    object_wipe(&otype);
    if (!make_object(c, &otype, level, good, great, FALSE, &rating, tval)) return;

    otype.origin = origin;
    otype.origin_depth = c->depth;

    /* Give it to the floor */
    /* XXX Should this be done in floor_carry? */
    if (!floor_carry(c, y, x, &otype)) {
		if (otype.artifact)
			otype.artifact->created = FALSE;
		return;
    } else {
		if (otype.artifact)
			c->good_item = TRUE;
		if (rating > 250000)
			rating = 250000; /* avoid overflows */
		c->obj_rating += (rating / 10) * (rating / 10);
    }
}


/**
 * Place a random amount of gold at (x, y).
 */
void place_gold(struct cave *c, int y, int x, int level, byte origin)
{
    object_type *i_ptr;
    object_type object_type_body;

    assert(square_in_bounds(c, y, x));

    if (!square_canputitem(c, y, x)) return;

    i_ptr = &object_type_body;
    object_wipe(i_ptr);
    make_gold(i_ptr, level, SV_GOLD_ANY);

    i_ptr->origin = origin;
    i_ptr->origin_depth = level;

    floor_carry(c, y, x, i_ptr);
}


/**
 * Place a secret door at (x, y).
 */
void place_secret_door(struct cave *c, int y, int x)
{
    square_set_feat(c, y, x, FEAT_SECRET);
}


/**
 * Place a closed door at (x, y).
 */
void place_closed_door(struct cave *c, int y, int x)
{
    int tmp = randint0(400);

    if (tmp < 300)
		square_set_feat(c, y, x, FEAT_DOOR_HEAD + 0x00);
    else if (tmp < 399)
		square_set_feat(c, y, x, FEAT_DOOR_HEAD + randint1(7));
    else
		square_set_feat(c, y, x, FEAT_DOOR_HEAD + 0x08 + randint0(8));
}


/**
 * Place a random door at (x, y).
 *
 * The door generated could be closed, open, broken, or secret.
 */
void place_random_door(struct cave *c, int y, int x)
{
    int tmp = randint0(100);

    if (tmp < 30)
		square_set_feat(c, y, x, FEAT_OPEN);
    else if (tmp < 40)
		square_set_feat(c, y, x, FEAT_BROKEN);
    else if (tmp < 60)
		square_set_feat(c, y, x, FEAT_SECRET);
    else
		place_closed_door(c, y, x);
}

/**
 * Place some staircases near walls.
 */
void alloc_stairs(struct cave *c, int feat, int num, int walls)
{
    int y, x, i, j, done;

    /* Place "num" stairs */
    for (i = 0; i < num; i++) {
		/* Place some stairs */
		for (done = FALSE; !done; ) {
			/* Try several times, then decrease "walls" */
			for (j = 0; !done && j <= 1000; j++) {
				find_empty(c, &y, &x);

				if (next_to_walls(c, y, x) < walls) continue;

				place_stairs(c, y, x, feat);
				done = TRUE;
			}

			/* Require fewer walls */
			if (walls) walls--;
		}
    }
}


/**
 * Allocates 'num' random objects in the dungeon.
 *
 * See alloc_object() for more information.
 */
void alloc_objects(struct cave *c, int set, int typ, int num, int depth, byte origin)
{
    int k, l = 0;
    for (k = 0; k < num; k++) {
		bool ok = alloc_object(c, set, typ, depth, origin);
		if (!ok) l++;
    }
}


/**
 * Allocates a single random object in the dungeon.
 *
 * 'set' controls where the object is placed (corridor, room, either).
 * 'typ' conrols the kind of object (rubble, trap, gold, item).
 */
bool alloc_object(struct cave *c, int set, int typ, int depth, byte origin)
{
    int x = 0, y = 0;
    int tries = 0;
    bool room;

    /* Pick a "legal" spot */
    while (tries < 2000) {
		tries++;

		find_empty(c, &y, &x);

		/* See if our spot is in a room or not */
		room = (sqinfo_has(c->info[y][x], SQUARE_ROOM)) ? TRUE : FALSE;

		/* If we are ok with a corridor and we're in one, we're done */
		if (set & SET_CORR && !room) break;

		/* If we are ok with a room and we're in one, we're done */
		if (set & SET_ROOM && room) break;
    }

    if (tries == 2000) return FALSE;

    /* Place something */
    switch (typ) {
    case TYP_RUBBLE: place_rubble(c, y, x); break;
    case TYP_TRAP: place_trap(c, y, x, -1, depth); break;
    case TYP_GOLD: place_gold(c, y, x, depth, origin); break;
    case TYP_OBJECT: place_object(c, y, x, depth, FALSE, FALSE, origin, 0); break;
    case TYP_GOOD: place_object(c, y, x, depth, TRUE, FALSE, origin, 0); break;
    case TYP_GREAT: place_object(c, y, x, depth, TRUE, TRUE, origin, 0); break;
    }
    return TRUE;
}

/**
 * Create up to 'num' objects near the given coordinates in a vault.
 */
void vault_objects(struct cave *c, int y, int x, int depth, int num)
{
    int i, j, k;

    /* Attempt to place 'num' objects */
    for (; num > 0; --num) {
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i) {
			/* Pick a random location */
			find_nearby_grid(c, &j, y, 2, &k, x, 3);

			/* Require "clean" floor space */
			if (!square_canputitem(c, j, k)) continue;

			/* Place an item or gold */
			if (randint0(100) < 75)
				place_object(c, j, k, depth, FALSE, FALSE, ORIGIN_SPECIAL, 0);
			else
				place_gold(c, j, k, depth, ORIGIN_VAULT);

			/* Placement accomplished */
			break;
		}
    }
}

/**
 * Place a trap near (x, y), with a given displacement.
 */
static void vault_trap_aux(struct cave *c, int y, int x, int yd, int xd)
{
    int tries, y1, x1;

    /* Find a nearby empty grid and place a trap */
    for (tries = 0; tries <= 5; tries++) {
		find_nearby_grid(c, &y1, y, yd, &x1, x, xd);
		if (!square_isempty(c, y1, x1)) continue;

		place_trap(c, y1, x1, -1, c->depth);
		break;
    }
}


/**
 * Place 'num' traps near (x, y), with a given displacement.
 */
void vault_traps(struct cave *c, int y, int x, int yd, int xd, int num)
{
    int i;
    for (i = 0; i < num; i++)
		vault_trap_aux(c, y, x, yd, xd);
}


/**
 * Place 'num' sleeping monsters near (x, y).
 */
void vault_monsters(struct cave *c, int y1, int x1, int depth, int num)
{
    int k, i, y, x;

    /* Try to summon "num" monsters "near" the given location */
    for (k = 0; k < num; k++) {
		/* Try nine locations */
		for (i = 0; i < 9; i++) {
			int d = 1;

			/* Pick a nearby location */
			scatter(&y, &x, y1, x1, d, TRUE);

			/* Require "empty" floor grids */
			if (!square_isempty(cave, y, x)) continue;

			/* Place the monster (allow groups) */
			pick_and_place_monster(c, y, x, depth, TRUE, TRUE, ORIGIN_DROP_SPECIAL);

			break;
		}
    }
}


