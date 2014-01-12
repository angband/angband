/*
 * File: gen-room.c
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
#include "monster/mon-make.h"
#include "monster/mon-spell.h"
#include "parser.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/**
 * Chooses a room template of a particular kind at random.
 *
 */
struct room_template *random_room_template(int typ)
{
	struct room_template *t = room_templates;
	struct room_template *r = NULL;
	int n = 1;
	do {
		if (t->typ == typ) {
			if (one_in_(n)) r = t;
			n++;
		}
		t = t->next;
	} while(t);
	return r;
}

/**
 * Chooses a vault of a particular kind at random.
 * 
 * Each vault has equal probability of being chosen. One weird thing is that
 * currently the v->typ indices are one off from the room type indices, which
 * means that build_greater_vault will call this function with "typ=8".
 */
struct vault *random_vault(int typ)
{
	struct vault *v = vaults;
	struct vault *r = NULL;
	int n = 1;
	do {
		if (v->typ == typ) {
			if (one_in_(n)) r = v;
			n++;
		}
		v = v->next;
	} while(v);
	return r;
}


/**
 * Mark squares as being in a room, and optionally light them.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive.
 */
static void generate_room(struct cave *c, int y1, int x1, int y2, int x2, int light)
{
	int y, x;
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++) {
			sqinfo_on(c->info[y][x], SQUARE_ROOM);
			if (light)
				sqinfo_on(c->info[y][x], SQUARE_GLOW);
		}
}


/**
 * Fill a rectangle with a feature.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive.
 */
void fill_rectangle(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y, x;
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			square_set_feat(c, y, x, feat);
}


/**
 * Fill the edges of a rectangle with a feature.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive.
 */
void draw_rectangle(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++) {
		square_set_feat(c, y, x1, feat);
		square_set_feat(c, y, x2, feat);
	}

	for (x = x1; x <= x2; x++) {
		square_set_feat(c, y1, x, feat);
		square_set_feat(c, y2, x, feat);
	}
}


/**
 * Fill a horizontal range with the given feature/info.
 */
static void fill_xrange(struct cave *c, int y, int x1, int x2, int feat, bool light)
{
	int x;
	for (x = x1; x <= x2; x++) {
		square_set_feat(c, y, x, feat);
		sqinfo_on(c->info[y][x], SQUARE_ROOM);
		if (light)
			sqinfo_on(c->info[y][x], SQUARE_GLOW);
	}
}


/**
 * Fill a vertical range with the given feature/info.
 */
static void fill_yrange(struct cave *c, int x, int y1, int y2, int feat, bool light)
{
	int y;
	for (y = y1; y <= y2; y++) {
		square_set_feat(c, y, x, feat);
		sqinfo_on(c->info[y][x], SQUARE_ROOM);
		if (light)
			sqinfo_on(c->info[y][x], SQUARE_GLOW);
	}
}


/**
 * Fill a circle with the given feature/info.
 */
static void fill_circle(struct cave *c, int y0, int x0, int radius, int border, int feat, bool light)
{
	int i, last = 0;
	int r2 = radius * radius;
	for(i = 0; i <= radius; i++) {
		double j = sqrt(r2 - (i * i));
		int k = (int)(j + 0.5);

		int b = border;
		if (border && last > k) b++;
		
		fill_xrange(c, y0 - i, x0 - k - b, x0 + k + b, feat, light);
		fill_xrange(c, y0 + i, x0 - k - b, x0 + k + b, feat, light);
		fill_yrange(c, x0 - i, y0 - k - b, y0 + k + b, feat, light);
		fill_yrange(c, x0 + i, y0 - k - b, y0 + k + b, feat, light);
		last = k;
	}
}


/**
 * Fill the lines of a cross/plus with a feature.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive. When combined with
 * draw_rectangle() this will generate a large rectangular room which is split
 * into four sub-rooms.
 */
static void generate_plus(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	/* Find the center */
	int y0 = (y1 + y2) / 2;
	int x0 = (x1 + x2) / 2;

	assert(c);

	for (y = y1; y <= y2; y++) square_set_feat(c, y, x0, feat);
	for (x = x1; x <= x2; x++) square_set_feat(c, y0, x, feat);
}


/**
 * Generate helper -- open all sides of a rectangle with a feature
 */
static void generate_open(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open all sides */
	square_set_feat(c, y1, x0, feat);
	square_set_feat(c, y0, x1, feat);
	square_set_feat(c, y2, x0, feat);
	square_set_feat(c, y0, x2, feat);
}


/**
 * Generate helper -- open one side of a rectangle with a feature
 */
static void generate_hole(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	/* Find the center */
	int y0 = (y1 + y2) / 2;
	int x0 = (x1 + x2) / 2;

	assert(c);

	/* Open random side */
	switch (randint0(4)) {
	case 0: square_set_feat(c, y1, x0, feat); break;
	case 1: square_set_feat(c, y0, x1, feat); break;
	case 2: square_set_feat(c, y2, x0, feat); break;
	case 3: square_set_feat(c, y0, x2, feat); break;
	}
}


/**
 * Build a circular room (interior radius 4-7).
 */
bool build_circular(struct cave *c, int y0, int x0)
{
	/* Pick a room size */
	int radius = 2 + randint1(2) + randint1(3);

	/* Occasional light */
	bool light = c->depth <= randint1(25) ? TRUE : FALSE;

	/* Generate outer walls and inner floors */
	fill_circle(c, y0, x0, radius + 1, 1, FEAT_WALL_OUTER, light);
	fill_circle(c, y0, x0, radius, 0, FEAT_FLOOR, light);

	/* Especially large circular rooms will have a middle chamber */
	if (radius - 4 > 0 && randint0(4) < radius - 4) {
		/* choose a random direction */
		int cd, rd;
		rand_dir(&rd, &cd);

		/* draw a room with a secret door on a random side */
		draw_rectangle(c, y0 - 2, x0 - 2, y0 + 2, x0 + 2, FEAT_WALL_INNER);
		square_set_feat(c, y0 + cd * 2, x0 + rd * 2, FEAT_SECRET);

		/* Place a treasure in the vault */
		vault_objects(c, y0, x0, c->depth, randint0(2));

		/* create some monsterss */
		vault_monsters(c, y0, x0, c->depth + 1, randint0(3));
	}

	return TRUE;
}


/**
 * Builds a normal rectangular room.
 */
bool build_simple(struct cave *c, int y0, int x0)
{
	int y, x;
	int light = FALSE;

	/* Pick a room size */
	int y1 = y0 - randint1(4);
	int x1 = x0 - randint1(11);
	int y2 = y0 + randint1(3);
	int x2 = x0 + randint1(11);

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls and inner floors */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	if (one_in_(20)) {
		/* Sometimes make a pillar room */
		for (y = y1; y <= y2; y += 2)
			for (x = x1; x <= x2; x += 2)
				square_set_feat(c, y, x, FEAT_WALL_INNER);

	} else if (one_in_(50)) {
		/* Sometimes make a ragged-edge room */
		for (y = y1 + 2; y <= y2 - 2; y += 2) {
			square_set_feat(c, y, x1, FEAT_WALL_INNER);
			square_set_feat(c, y, x2, FEAT_WALL_INNER);
		}

		for (x = x1 + 2; x <= x2 - 2; x += 2) {
			square_set_feat(c, y1, x, FEAT_WALL_INNER);
			square_set_feat(c, y2, x, FEAT_WALL_INNER);
		}
	}
	return TRUE;
}


/**
 * Builds an overlapping rectangular room.
 */
bool build_overlap(struct cave *c, int y0, int x0)
{
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int light = FALSE;

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Determine extents of room (a) */
	y1a = y0 - randint1(4);
	x1a = x0 - randint1(11);
	y2a = y0 + randint1(3);
	x2a = x0 + randint1(10);

	/* Determine extents of room (b) */
	y1b = y0 - randint1(3);
	x1b = x0 - randint1(10);
	y2b = y0 + randint1(4);
	x2b = x0 + randint1(11);

	/* Generate new room (a) */
	generate_room(c, y1a-1, x1a-1, y2a+1, x2a+1, light);

	/* Generate new room (b) */
	generate_room(c, y1b-1, x1b-1, y2b+1, x2b+1, light);

	/* Generate outer walls (a) */
	draw_rectangle(c, y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	draw_rectangle(c, y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	fill_rectangle(c, y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	fill_rectangle(c, y1b, x1b, y2b, x2b, FEAT_FLOOR);

	return TRUE;
}


/**
 * Builds a cross-shaped room.
 *
 * Room "a" runs north/south, and Room "b" runs east/east 
 * So a "central pillar" would run from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that the code
 * below will work for 5x5 (and perhaps even for unsymetric values like 4x3 or
 * 5x3 or 3x4 or 3x5).
 */
bool build_crossed(struct cave *c, int y0, int x0)
{
	int y, x;

	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int dy, dx, wy, wx;

	int light = FALSE;

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Pick inner dimension */
	wy = 1;
	wx = 1;

	/* Pick outer dimension */
	dy = rand_range(3, 4);
	dx = rand_range(3, 11);

	/* Determine extents of room (a) */
	y1a = y0 - dy;
	x1a = x0 - wx;
	y2a = y0 + dy;
	x2a = x0 + wx;

	/* Determine extents of room (b) */
	y1b = y0 - wy;
	x1b = x0 - dx;
	y2b = y0 + wy;
	x2b = x0 + dx;

	/* Generate new room (a) */
	generate_room(c, y1a-1, x1a-1, y2a+1, x2a+1, light);

	/* Generate new room (b) */
	generate_room(c, y1b-1, x1b-1, y2b+1, x2b+1, light);

	/* Generate outer walls (a) */
	draw_rectangle(c, y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	draw_rectangle(c, y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	fill_rectangle(c, y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	fill_rectangle(c, y1b, x1b, y2b, x2b, FEAT_FLOOR);

	/* Special features */
	switch (randint1(4)) {
		/* Nothing */
	case 1: break;

		/* Large solid middle pillar */
	case 2: {
		fill_rectangle(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);
		break;
	}

		/* Inner treasure vault */
	case 3: {
		/* Generate a small inner vault */
		draw_rectangle(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

		/* Open the inner vault with a secret door */
		generate_hole(c, y1b, x1a, y2b, x2a, FEAT_SECRET);

		/* Place a treasure in the vault */
		place_object(c, y0, x0, c->depth, FALSE, FALSE, ORIGIN_SPECIAL, 0);

		/* Let's guard the treasure well */
		vault_monsters(c, y0, x0, c->depth + 2, randint0(2) + 3);

		/* Traps naturally */
		vault_traps(c, y0, x0, 4, 4, randint0(3) + 2);

		break;
	}

		/* Something else */
	case 4: {
		if (one_in_(3)) {
			/* Occasionally pinch the center shut */

			/* Pinch the east/west sides */
			for (y = y1b; y <= y2b; y++) {
				if (y == y0) continue;
				square_set_feat(c, y, x1a - 1, FEAT_WALL_INNER);
				square_set_feat(c, y, x2a + 1, FEAT_WALL_INNER);
			}

			/* Pinch the north/south sides */
			for (x = x1a; x <= x2a; x++) {
				if (x == x0) continue;
				square_set_feat(c, y1b - 1, x, FEAT_WALL_INNER);
				square_set_feat(c, y2b + 1, x, FEAT_WALL_INNER);
			}

			/* Open sides with secret doors */
			if (one_in_(3))
				generate_open(c, y1b-1, x1a-1, y2b+1, x2a+1, FEAT_SECRET);

		} else if (one_in_(3)) {
			/* Occasionally put a "plus" in the center */
			generate_plus(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

		} else if (one_in_(3)) {
			/* Occasionally put a "pillar" in the center */
			square_set_feat(c, y0, x0, FEAT_WALL_INNER);
		}

		break;
	}
	}

	return TRUE;
}


/**
 * Build a large room with an inner room.
 *
 * Possible sub-types:
 *	1 - An inner room
 *	2 - An inner room with a small inner room
 *	3 - An inner room with a pillar or pillars
 *	4 - An inner room with a checkerboard
 *	5 - An inner room with four compartments
 */
bool build_large(struct cave *c, int y0, int x0)
{
	int y, x, y1, x1, y2, x2;

	int light = FALSE;

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;

	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Inner room variations */
	switch (randint1(5)) {
		/* An inner room */
	case 1: {
		/* Open the inner room with a secret door and place a monster */
		generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);
		vault_monsters(c, y0, x0, c->depth + 2, 1);
		break;
	}


		/* An inner room with a small inner room */
	case 2: {
		/* Open the inner room with a secret door */
		generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

		/* Place another inner room */
		draw_rectangle(c, y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

		/* Open the inner room with a locked door */
		generate_hole(c, y0-1, x0-1, y0+1, x0+1, FEAT_DOOR_HEAD + randint1(7));

		/* Monsters to guard the treasure */
		vault_monsters(c, y0, x0, c->depth + 2, randint1(3) + 2);

		/* Object (80%) or Stairs (20%) */
		if (randint0(100) < 80)
			place_object(c, y0, x0, c->depth, FALSE, FALSE, ORIGIN_SPECIAL, 0);
		else
			place_random_stairs(c, y0, x0);

		/* Traps to protect the treasure */
		vault_traps(c, y0, x0, 4, 10, 2 + randint1(3));

		break;
	}


		/* An inner room with an inner pillar or pillars */
	case 3: {
		/* Open the inner room with a secret door */
		generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

		/* Inner pillar */
		fill_rectangle(c, y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

		/* Occasionally, two more Large Inner Pillars */
		if (one_in_(2)) {
			if (one_in_(2)) {
				fill_rectangle(c, y0-1, x0-7, y0+1, x0-5, FEAT_WALL_INNER);
				fill_rectangle(c, y0-1, x0+5, y0+1, x0+7, FEAT_WALL_INNER);
			} else {
				fill_rectangle(c, y0-1, x0-6, y0+1, x0-4, FEAT_WALL_INNER);
				fill_rectangle(c, y0-1, x0+4, y0+1, x0+6, FEAT_WALL_INNER);
			}
		}

		/* Occasionally, some Inner rooms */
		if (one_in_(3)) {
			/* Inner rectangle */
			draw_rectangle(c, y0-1, x0-5, y0+1, x0+5, FEAT_WALL_INNER);

			/* Secret doors (random top/bottom) */
			place_secret_door(c, y0 - 3 + (randint1(2) * 2), x0 - 3);
			place_secret_door(c, y0 - 3 + (randint1(2) * 2), x0 + 3);

			/* Monsters */
			vault_monsters(c, y0, x0 - 2, c->depth + 2, randint1(2));
			vault_monsters(c, y0, x0 + 2, c->depth + 2, randint1(2));

			/* Objects */
			if (one_in_(3))
				place_object(c, y0, x0 - 2, c->depth, FALSE, FALSE,
							 ORIGIN_SPECIAL, 0);
			if (one_in_(3))
				place_object(c, y0, x0 + 2, c->depth, FALSE, FALSE,
							 ORIGIN_SPECIAL, 0);
		}

		break;
	}


		/* An inner room with a checkerboard */
	case 4: {
		/* Open the inner room with a secret door */
		generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

		/* Checkerboard */
		for (y = y1; y <= y2; y++)
			for (x = x1; x <= x2; x++)
				if ((x + y) & 0x01)
					square_set_feat(c, y, x, FEAT_WALL_INNER);

		/* Monsters just love mazes. */
		vault_monsters(c, y0, x0 - 5, c->depth + 2, randint1(3));
		vault_monsters(c, y0, x0 + 5, c->depth + 2, randint1(3));

		/* Traps make them entertaining. */
		vault_traps(c, y0, x0 - 3, 2, 8, randint1(3));
		vault_traps(c, y0, x0 + 3, 2, 8, randint1(3));

		/* Mazes should have some treasure too. */
		vault_objects(c, y0, x0, c->depth, 3);

		break;
	}


		/* Four small rooms. */
	case 5: {
		/* Inner "cross" */
		generate_plus(c, y1, x1, y2, x2, FEAT_WALL_INNER);

		/* Doors into the rooms */
		if (randint0(100) < 50) {
			int i = randint1(10);
			place_secret_door(c, y1 - 1, x0 - i);
			place_secret_door(c, y1 - 1, x0 + i);
			place_secret_door(c, y2 + 1, x0 - i);
			place_secret_door(c, y2 + 1, x0 + i);
		} else {
			int i = randint1(3);
			place_secret_door(c, y0 + i, x1 - 1);
			place_secret_door(c, y0 - i, x1 - 1);
			place_secret_door(c, y0 + i, x2 + 1);
			place_secret_door(c, y0 - i, x2 + 1);
		}

		/* Treasure, centered at the center of the cross */
		vault_objects(c, y0, x0, c->depth, 2 + randint1(2));

		/* Gotta have some monsters */
		vault_monsters(c, y0 + 1, x0 - 4, c->depth + 2, randint1(4));
		vault_monsters(c, y0 + 1, x0 + 4, c->depth + 2, randint1(4));
		vault_monsters(c, y0 - 1, x0 - 4, c->depth + 2, randint1(4));
		vault_monsters(c, y0 - 1, x0 + 4, c->depth + 2, randint1(4)); 

		break;
	}
	}

	return TRUE;
}


/* Hook for which type of pit we are building */
/* TODO(elly): why is this file-static instead of an argument? */
static pit_profile *pit_type = NULL;


/**
 * Hook for picking monsters appropriate to a nest/pit.
 *
 * Requires pit_type to be set.
 */
static bool mon_pit_hook(monster_race *r_ptr)
{
	bool match_base = TRUE;
	bool match_color = TRUE;

	assert(r_ptr);
	assert(pit_type);

	if (rf_has(r_ptr->flags, RF_UNIQUE))
		return FALSE;
	else if (!rf_is_subset(r_ptr->flags, pit_type->flags))
		return FALSE;
	else if (rf_is_inter(r_ptr->flags, pit_type->forbidden_flags))
		return FALSE;
	else if (!rsf_is_subset(r_ptr->spell_flags, pit_type->spell_flags))
		return FALSE;
	else if (rsf_is_inter(r_ptr->spell_flags, pit_type->forbidden_spell_flags))
		return FALSE;
	else if (pit_type->forbidden_monsters) {
		struct pit_forbidden_monster *monster;
		for (monster = pit_type->forbidden_monsters; monster; monster = monster->next) {
			if (r_ptr == monster->race)
				return FALSE;
		}
	}

	if (pit_type->n_bases > 0) {
		int i;
		match_base = FALSE;

		for (i = 0; i < pit_type->n_bases; i++) {
			if (r_ptr->base == pit_type->base[i])
				match_base = TRUE;
		}
	}
	
	if (pit_type->colors) {
		struct pit_color_profile *colors;
		match_color = FALSE;

		for (colors = pit_type->colors; colors; colors = colors->next) {
			if (r_ptr->d_attr == colors->color)
				match_color = TRUE;
		}
	}

	return (match_base && match_color);
}

/**
 * Pick a type of monster pit, based on the level.
 *
 * We scan through all pits, and for each one generate a random depth
 * using a normal distribution, with the mean given in pit.txt, and a
 * standard deviation of 10. Then we pick the pit that gave us a depth that
 * is closest to the player's actual depth.
 *
 * Sets pit_type, which is required for mon_pit_hook.
 * Returns the index of the chosen pit.
 */
static int set_pit_type(int depth, int type)
{
	int i;
	int pit_idx = 0;
	
	/* Hack -- set initial distance large */
	int pit_dist = 999;
	
	for (i = 0; i < z_info->pit_max; i++) {
		int offset, dist;
		pit_profile *pit = &pit_info[i];
		
		/* Skip empty pits or pits of the wrong room type */
		if (!pit->name || pit->room_type != type) continue;
		
		offset = Rand_normal(pit->ave, 10);
		dist = ABS(offset - depth);
		
		if (dist < pit_dist && one_in_(pit->rarity)) {
			/* This pit is the closest so far */
			pit_idx = i;
			pit_dist = dist;
		}
	}

	pit_type = &pit_info[pit_idx];
        
	return pit_idx;
}


/**
 * Build a monster nest
 *
 * A monster nest consists of a rectangular moat around a room containing
 * monsters of a given type.
 *
 * The monsters are chosen from a set of 64 randomly selected monster races,
 * to allow the nest creation to fail instead of having "holes".
 *
 * Note the use of the "get_mon_num_prep()" function to prepare the
 * "monster allocation table" in such a way as to optimize the selection
 * of "appropriate" non-unique monsters for the nest.
 *
 * The available monster nests are specified in edit/pit.txt.
 *
 * Note that get_mon_num() function can fail, in which case the nest will be
 * empty, and will not affect the level rating.
 *
 * Monster nests will never contain unique monsters.
 */
bool build_nest(struct cave *c, int y0, int x0)
{
	int y, x, y1, x1, y2, x2;
	int i;
	int alloc_obj;
	monster_race *what[64];
	bool empty = FALSE;
	int light = FALSE;
	int pit_idx;
	int size_vary = randint0(4);

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 5 - size_vary;
	x2 = x0 + 5 + size_vary;

	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

	/* Decide on the pit type */
	pit_idx = set_pit_type(c->depth, 2);

	/* Chance of objects on the floor */
	alloc_obj = pit_info[pit_idx].obj_rarity;
	
	/* Prepare allocation table */
	get_mon_num_prep(mon_pit_hook);

	/* Pick some monster types */
	for (i = 0; i < 64; i++) {
		/* Get a (hard) monster type */
		what[i] = get_mon_num(c->depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}

	/* Prepare allocation table */
	get_mon_num_prep(NULL);

	/* Oops */
	if (empty) return FALSE;

	/* Describe */
	ROOM_LOG("Monster nest (%s)", pit_info[pit_idx].name);

	/* Increase the level rating */
	c->mon_rating += (size_vary + pit_info[pit_idx].ave / 20);

	/* Place some monsters */
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			/* Figure out what monster is being used, and place that monster */
			monster_race *race = what[randint0(64)];
			place_new_monster(c, y, x, race, FALSE, FALSE, ORIGIN_DROP_PIT);

			/* Occasionally place an item, making it good 1/3 of the time */
			if (randint0(100) < alloc_obj) 
				place_object(c, y, x, c->depth + 10, one_in_(3), FALSE,
							 ORIGIN_PIT, 0);
		}
	}

	return TRUE;
}

/**
 * Build a monster pit
 *
 * Monster pits are laid-out similarly to monster nests.
 *
 * The available monster pits are specified in edit/pit.txt.
 *
 * The inside room in a monster pit appears as shown below, where the
 * actual monsters in each location depend on the type of the pit
 *
 *   #############
 *   #11000000011#
 *   #01234543210#
 *   #01236763210#
 *   #01234543210#
 *   #11000000011#
 *   #############
 *
 * Note that the monsters in the pit are chosen by using get_mon_num() to
 * request 16 "appropriate" monsters, sorting them by level, and using the
 * "even" entries in this sorted list for the contents of the pit.
 *
 * Note the use of get_mon_num_prep() to prepare the monster allocation
 * table in such a way as to optimize the selection of appropriate non-unique
 * monsters for the pit.
 *
 * The get_mon_num() function can fail, in which case the pit will be empty,
 * and will not effect the level rating.
 *
 * Like monster nests, monster pits will never contain unique monsters.
 */
bool build_pit(struct cave *c, int y0, int x0)
{
	monster_race *what[16];
	int i, j, y, x, y1, x1, y2, x2;
	bool empty = FALSE;
	int light = FALSE;
	int pit_idx;
	int alloc_obj;

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 7;
	x2 = x0 + 7;

	/* Generate new room, outer walls and inner floor */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls, and open with a secret door */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);
	generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

	/* Decide on the pit type */
	pit_idx = set_pit_type(c->depth, 1);

	/* Chance of objects on the floor */
	alloc_obj = pit_info[pit_idx].obj_rarity;
	
	/* Prepare allocation table */
	get_mon_num_prep(mon_pit_hook);

	/* Pick some monster types */
	for (i = 0; i < 16; i++) {
		/* Get a (hard) monster type */
		what[i] = get_mon_num(c->depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}

	/* Prepare allocation table */
	get_mon_num_prep(NULL);

	/* Oops */
	if (empty)
		return FALSE;

	ROOM_LOG("Monster pit (%s)", pit_info[pit_idx].name);

	/* Sort the entries XXX XXX XXX */
	for (i = 0; i < 16 - 1; i++) {
		/* Sort the entries */
		for (j = 0; j < 16 - 1; j++) {
			int i1 = j;
			int i2 = j + 1;

			int p1 = what[i1]->level;
			int p2 = what[i2]->level;

			/* Bubble */
			if (p1 > p2) {
				monster_race *tmp = what[i1];
				what[i1] = what[i2];
				what[i2] = tmp;
			}
		}
	}

	/* Select every other entry */
	for (i = 0; i < 8; i++)
		what[i] = what[i * 2];

	/* Increase the level rating */
	c->mon_rating += (3 + pit_info[pit_idx].ave / 20);

	/* Top and bottom rows (middle) */
	for (x = x0 - 3; x <= x0 + 3; x++) {
		place_new_monster(c, y0 - 2, x, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y0 + 2, x, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);
	}
    
	/* Corners */
	for (x = x0 - 5; x <= x0 - 4; x++) {
		place_new_monster(c, y0 - 2, x, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y0 + 2, x, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
	}
    
	for (x = x0 + 4; x <= x0 + 5; x++) {
		place_new_monster(c, y0 - 2, x, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y0 + 2, x, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
	}
    
	/* Corners */

	/* Middle columns */
	for (y = y0 - 1; y <= y0 + 1; y++) {
		place_new_monster(c, y, x0 - 5, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y, x0 + 5, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_new_monster(c, y, x0 - 4, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y, x0 + 4, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_new_monster(c, y, x0 - 3, what[2], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y, x0 + 3, what[2], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_new_monster(c, y, x0 - 2, what[3], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y, x0 + 2, what[3], FALSE, FALSE, ORIGIN_DROP_PIT);
	}
    
	/* Corners around the middle monster */
	place_new_monster(c, y0 - 1, x0 - 1, what[4], FALSE, FALSE, ORIGIN_DROP_PIT);
	place_new_monster(c, y0 - 1, x0 + 1, what[4], FALSE, FALSE, ORIGIN_DROP_PIT);
	place_new_monster(c, y0 + 1, x0 - 1, what[4], FALSE, FALSE, ORIGIN_DROP_PIT);
	place_new_monster(c, y0 + 1, x0 + 1, what[4], FALSE, FALSE, ORIGIN_DROP_PIT);

	/* Above/Below the center monster */
	for (x = x0 - 1; x <= x0 + 1; x++) {
		place_new_monster(c, y0 + 1, x, what[5], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_new_monster(c, y0 - 1, x, what[5], FALSE, FALSE, ORIGIN_DROP_PIT);
	}

	/* Next to the center monster */
	place_new_monster(c, y0, x0 + 1, what[6], FALSE, FALSE, ORIGIN_DROP_PIT);
	place_new_monster(c, y0, x0 - 1, what[6], FALSE, FALSE, ORIGIN_DROP_PIT);

	/* Center monster */
	place_new_monster(c, y0, x0, what[7], FALSE, FALSE, ORIGIN_DROP_PIT);

	/* Place some objects */
	for (y = y0 - 2; y <= y0 + 2; y++) {
		for (x = x0 - 9; x <= x0 + 9; x++) {
			/* Occasionally place an item, making it good 1/3 of the time */
			if (randint0(100) < alloc_obj) 
				place_object(c, y, x, c->depth + 10, one_in_(3), FALSE,
							 ORIGIN_PIT, 0);
		}
	}

	return TRUE;
}

/**
 * Build a room template from its string representation.
 */


static void build_room_template(struct cave *c, int y0, int x0, int ymax, int xmax, int doors, const char *data, int tval)
{
	int dx, dy, x, y, rnddoors, doorpos;
	const char *t;
	bool rndwalls, light;
	

	assert(c);

	/* Occasional light */
	light = c->depth <= randint1(25) ? TRUE : FALSE;

	/* Set the random door position here so it generates doors in all squares
	 * marked with the same number */
	rnddoors = randint1(doors);

	/* Decide whether optional walls will be generated this time */
	rndwalls = one_in_(2) ? TRUE : FALSE;

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax && *t; dy++) {
		for (dx = 0; dx < xmax && *t; dx++, t++) {
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Skip non-grids */
			if (*t == ' ') continue;

			/* Lay down a floor */
			square_set_feat(c, y, x, FEAT_FLOOR);

			/* Debugging assertion */
			assert(square_isempty(c, y, x));

			/* Analyze the grid */
			switch (*t) {
			case '%': square_set_feat(c, y, x, FEAT_WALL_OUTER); break;
			case '#': square_set_feat(c, y, x, FEAT_WALL_SOLID); break;
			case '+': place_secret_door(c, y, x); break;
			case 'x': {

				/* If optional walls are generated, put a wall in this square */

				if (rndwalls)
					square_set_feat(c, y, x, FEAT_WALL_SOLID);
				break;
			}
			case '(': {

				/* If optional walls are generated, put a door in this square */

				if (rndwalls)
					place_secret_door(c, y, x);
				break;
			}
			case ')': {
				/* If optional walls are not generated, put a door in this square */
				if (!rndwalls)
					place_secret_door(c, y, x);
				else
					square_set_feat(c, y, x, FEAT_WALL_SOLID);

				break;
			}
			case '8': {

				/* Put something nice in this square
				 * Object (80%) or Stairs (20%) */
				if (randint0(100) < 80)
					place_object(c, y, x, c->depth, FALSE, FALSE, ORIGIN_SPECIAL, 0);
				else
					place_random_stairs(c, y, x);

				/* Some monsters to guard it */
				vault_monsters(c, y, x, c->depth + 2, randint0(2) + 3);

				/* And some traps too */
				vault_traps(c, y, x, 4, 4, randint0(3) + 2);

				break;
			}
			case '9': {

				/* Create some interesting stuff nearby */

				/* A few monsters */
				vault_monsters(c, y - 3, x - 3, c->depth + randint0(2), randint1(2));
				vault_monsters(c, y + 3, x + 3, c->depth + randint0(2), randint1(2));

				/* And maybe a bit of treasure */

				if (one_in_(2))
					vault_objects(c, y - 2, x + 2, c->depth, 1 + randint0(2));

				if (one_in_(2))
					vault_objects(c, y + 2, x - 2, c->depth, 1 + randint0(2));

				break;

			}
			case '[': {
				
				/* Place an object of the template's specified tval */
				place_object(c, y, x, c->depth, FALSE, FALSE, ORIGIN_SPECIAL, tval);
				break;
			}
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6': {

				/* Check if this is chosen random door position */

				doorpos = atoi(t);

				if (doorpos == rnddoors)
					place_secret_door(c, y, x);
				else
					square_set_feat(c, y, x, FEAT_WALL_SOLID);

				break;
			}
			}

			/* Part of a room */
			sqinfo_on(c->info[y][x], SQUARE_ROOM);
			if (light)
				sqinfo_on(c->info[y][x], SQUARE_GLOW);
		}
	}
}


/**
 * Helper function for building room templates.
 */
static bool build_room_template_type(struct cave*c, int y0, int x0, int typ, const char *label)
{
	room_template_type *t_ptr = random_room_template(typ);
	
	if (t_ptr == NULL) {
		/*quit_fmt("got NULL from random_room_template(%d)", typ);*/
		return FALSE;
	}

	ROOM_LOG("Room template (%s)", t_ptr->name);

	/* Build the room */
	build_room_template(c, y0, x0, t_ptr->hgt, t_ptr->wid, t_ptr->dor, t_ptr->text, t_ptr->tval);

	return TRUE;
}


bool build_template(struct cave *c, int y0, int x0)
{
	/* All room templates currently have type 1 */
	return build_room_template_type(c, y0, x0, 1, "Special room");
}




/**
 * Build a vault from its string representation.
 */
static void build_vault(struct cave *c, int y0, int x0, int ymax, int xmax, const char *data)
{
	int dx, dy, x, y;
	const char *t;
	bool icky;

	assert(c);

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax && *t; dy++) {
		for (dx = 0; dx < xmax && *t; dx++, t++) {
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Skip non-grids */
			if (*t == ' ') continue;

			/* Lay down a floor */
			square_set_feat(c, y, x, FEAT_FLOOR);

			/* Debugging assertion */
			assert(square_isempty(c, y, x));

			/* By default vault squares are marked icky */
			icky = TRUE;

			/* Analyze the grid */
			switch (*t) {
			case '%': {
				/* In this case, the square isn't really part of the
				 * vault, but rather is part of the "door step" to the
				 * vault. We don't mark it icky so that the tunneling
				 * code knows its allowed to remove this wall. */
				square_set_feat(c, y, x, FEAT_WALL_OUTER);
				icky = FALSE;
				break;
			}
			case '#': square_set_feat(c, y, x, FEAT_WALL_INNER); break;
			case '@': square_set_feat(c, y, x, FEAT_PERM_INNER); break;
			case '+': place_secret_door(c, y, x); break;
			case '^': place_trap(c, y, x, -1, c->depth); break;
			case '&': {
				/* Treasure or a trap */
				if (randint0(100) < 75)
					place_object(c, y, x, c->depth, FALSE, FALSE, ORIGIN_VAULT, 0);
				else
					place_trap(c, y, x, -1, c->depth);
				break;
			}
			}

			/* Part of a vault */
			sqinfo_on(c->info[y][x], SQUARE_ROOM);
			if (icky) sqinfo_on(c->info[y][x], SQUARE_VAULT);
		}
	}


	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax && *t; dy++) {
		for (dx = 0; dx < xmax && *t; dx++, t++) {
			/* Extract the grid */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the symbol */
			switch (*t) {
			case '2': pick_and_place_monster(c, y, x, c->depth + 5, TRUE, TRUE,
											 ORIGIN_DROP_VAULT); break;
			case '6': pick_and_place_monster(c, y, x, c->depth + 11, TRUE, TRUE,
											 ORIGIN_DROP_VAULT); break;

			case '9': {
				/* Meaner monster, plus treasure */
				pick_and_place_monster(c, y, x, c->depth + 9, TRUE, TRUE,
									   ORIGIN_DROP_VAULT);
				place_object(c, y, x, c->depth + 7, TRUE, FALSE,
							 ORIGIN_VAULT, 0);
				break;
			}

			case '8': {
				/* Nasty monster and treasure */
				pick_and_place_monster(c, y, x, c->depth + 40, TRUE, TRUE,
									   ORIGIN_DROP_VAULT);
				place_object(c, y, x, c->depth + 20, TRUE, TRUE,
							 ORIGIN_VAULT, 0);
				break;
			}

			case '4': {
				/* Monster and/or object */
				if (randint0(100) < 50)
					pick_and_place_monster(c, y, x, c->depth + 3, TRUE, TRUE,
										   ORIGIN_DROP_VAULT);
				if (randint0(100) < 50)
					place_object(c, y, x, c->depth + 7, FALSE, FALSE,
								 ORIGIN_VAULT, 0);
				break;
			}
			}
		}
	}
}


/**
 * Helper function for building vaults.
 */
static bool build_vault_type(struct cave*c, int y0, int x0, int typ, const char *label)
{
	struct vault *v_ptr = random_vault(typ);
	if (v_ptr == NULL) {
		/*quit_fmt("got NULL from random_vault(%d)", typ);*/
		return FALSE;
	}

	ROOM_LOG("%s (%s)", label, v_ptr->name);

	/* Boost the rating */
	c->mon_rating += v_ptr->rat;

	/* Build the vault */
	build_vault(c, y0, x0, v_ptr->hgt, v_ptr->wid, v_ptr->text);

	return TRUE;
}


/**
 * Build a lesser vault.
 */
bool build_lesser_vault(struct cave *c, int y0, int x0)
{
	return build_vault_type(c, y0, x0, 6, "Lesser vault");
}


/**
 * Build a (medium) vault.
 */
bool build_medium_vault(struct cave *c, int y0, int x0)
{
	return build_vault_type(c, y0, x0, 7, "Medium vault");
}


/**
 * Build a greater vaults.
 *
 * Since Greater Vaults are so large (4x6 blocks, in a 6x18 dungeon) there is
 * a 63% chance that a randomly chosen quadrant to start a GV on won't work.
 * To balance this, we give Greater Vaults an artificially high probability
 * of being attempted, and then in this function use a depth check to cancel
 * vault creation except at deep depths.
 *
 * The following code should make a greater vault with frequencies:
 * dlvl  freq
 * 100+  18.0%
 * 90-99 16.0 - 18.0%
 * 80-89 10.0 - 11.0%
 * 70-79  5.7 -  6.5%
 * 60-69  3.3 -  3.8%
 * 50-59  1.8 -  2.1%
 * 0-49   0.0 -  1.0%
 */
bool build_greater_vault(struct cave *c, int y0, int x0)
{
	int i;
	int numerator   = 2;
	int denominator = 3;
	
	/* Only try to build a GV as the first room. */
	if (dun->cent_n > 0) return FALSE;

	/* Level 90+ has a 2/3 chance, level 80-89 has 4/9, ... */
	for(i = 90; i > c->depth; i -= 10) {
		numerator *= 2;
		denominator *= 3;
	}

	/* Attempt to pass the depth check and build a GV */
	if (randint0(denominator) >= numerator) return FALSE;

	return build_vault_type(c, y0, x0, 8, "Greater vault");
}


/**
 * Attempt to build a room of the given type at the given block
 *
 * Note that we restrict the number of pits/nests to reduce
 * the chance of overflowing the monster list during level creation.
 */
bool room_build(struct cave *c, int by0, int bx0, struct room_profile profile)
{
	/* Extract blocks */
	int by1 = by0;
	int bx1 = bx0;
	int by2 = by0 + profile.height;
	int bx2 = bx0 + profile.width;

	int allocated;
	int y, x;
	int by, bx;

	/* Enforce the room profile's minimum depth */
	if (c->depth < profile.level) return FALSE;

	/* Only allow at most two pit/nests room per level */
	if ((dun->pit_num >= MAX_PIT) && (profile.pit)){
		return FALSE;
	}

	/* Never run off the screen */
	if (by1 < 0 || by2 >= dun->row_rooms) return FALSE;
	if (bx1 < 0 || bx2 >= dun->col_rooms) return FALSE;

	/* Verify open space */
	for (by = by1; by <= by2; by++) {
		for (bx = bx1; bx <= bx2; bx++) {
			if (1) {
				/* previous rooms prevent new ones */
				if (dun->room_map[by][bx]) return FALSE;
			} else {
				return FALSE; /* XYZ */
			}
		}
	}

	/* Get the location of the room */
	y = ((by1 + by2 + 1) * BLOCK_HGT) / 2;
	x = ((bx1 + bx2 + 1) * BLOCK_WID) / 2;

	/* Try to build a room */
	if (!profile.builder(c, y, x)) return FALSE;

	/* Save the room location */
	if (dun->cent_n < CENT_MAX) {
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;
	}

	/* Reserve some blocks */
	allocated = 0;
	for (by = by1; by < by2; by++) {
		for (bx = bx1; bx < bx2; bx++) {
			dun->room_map[by][bx] = TRUE;
			allocated++;
		}
	}

	/* Count pit/nests rooms */
	if (profile.pit) dun->pit_num++;

	/* Success */
	return TRUE;
}
