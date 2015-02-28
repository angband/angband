/**
 * \file cave.c
 * \brief chunk allocation and utility functions
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
#include "cmds.h"
#include "cmd-core.h"
#include "game-event.h"
#include "game-world.h"
#include "init.h"
#include "monster.h"
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "trap.h"

struct feature *f_info;
struct chunk *cave = NULL;
struct chunk *cave_k = NULL;

/**
 * Global array for looping through the "keypad directions".
 */
const s16b ddd[9] =
{ 2, 8, 6, 4, 3, 1, 9, 7, 5 };

/**
 * Global arrays for converting "keypad direction" into "offsets".
 */
const s16b ddx[10] =
{ 0, -1, 0, 1, -1, 0, 1, -1, 0, 1 };

const s16b ddy[10] =
{ 0, 1, 1, 1, 0, 0, 0, -1, -1, -1 };

/**
 * Global arrays for optimizing "ddx[ddd[i]]" and "ddy[ddd[i]]".
 *
 * This means that each entry in this array corresponds to the direction
 * with the same array index in ddd[].
 */
const s16b ddx_ddd[9] =
{ 0, 0, 1, -1, 1, -1, 1, -1, 0 };

const s16b ddy_ddd[9] =
{ 1, -1, 0, 0, 1, 1, -1, -1, 0 };

/**
 * Find a terrain feature index by name
 */
static int lookup_feat(const char *name)
{
	int i;

	/* Look for it */
	for (i = 0; i < z_info->f_max; i++) {
		struct feature *feat = &f_info[i];
		if (!feat->name)
			continue;

		/* Test for equality */
		if (streq(name, feat->name))
			return i;
	}

	/* Fail horribly */
	quit_fmt("Failed to find terrain feature %s", name);
	return -1;
}

/**
 * Set terrain constants to the indices from terrain.txt
 */
void set_terrain(void)
{
	FEAT_NONE = lookup_feat("unknown grid");
	FEAT_FLOOR = lookup_feat("open floor");
	FEAT_CLOSED = lookup_feat("closed door");
	FEAT_OPEN = lookup_feat("open door");
	FEAT_BROKEN = lookup_feat("broken door");
	FEAT_LESS = lookup_feat("up staircase");
	FEAT_MORE = lookup_feat("down staircase");
	FEAT_SECRET = lookup_feat("secret door");
	FEAT_RUBBLE = lookup_feat("pile of rubble");
	FEAT_PASS_RUBBLE = lookup_feat("pile of passable rubble");
	FEAT_MAGMA = lookup_feat("magma vein");
	FEAT_QUARTZ = lookup_feat("quartz vein");
	FEAT_MAGMA_K = lookup_feat("magma vein with treasure");
	FEAT_QUARTZ_K = lookup_feat("quartz vein with treasure");
	FEAT_GRANITE = lookup_feat("granite wall");
	FEAT_PERM = lookup_feat("permanent wall");
	FEAT_LAVA = lookup_feat("lava flow");
	FEAT_DTRAP_FLOOR = lookup_feat("dtrap edge - floor");
	FEAT_DTRAP_WALL = lookup_feat("dtrap edge - wall");
}

/**
 * Allocate a new chunk of the world
 */
struct chunk *cave_new(int height, int width) {
	int y, x;

	struct chunk *c = mem_zalloc(sizeof *c);
	c->height = height;
	c->width = width;
	c->feat_count = mem_zalloc((z_info->f_max + 1) * sizeof(int));

	c->squares = mem_zalloc(c->height * sizeof(struct square*));
	for (y = 0; y < c->height; y++) {
		c->squares[y] = mem_zalloc(c->width * sizeof(struct square));
		for (x = 0; x < c->width; x++)
			c->squares[y][x].info = mem_zalloc(SQUARE_SIZE * sizeof(bitflag));
	}

	c->monsters = mem_zalloc(z_info->level_monster_max *sizeof(struct monster));
	c->mon_max = 1;
	c->mon_current = -1;

	c->created_at = turn;
	return c;
}

/**
 * Free a chunk
 */
void cave_free(struct chunk *c) {
	int y, x;

	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			mem_free(c->squares[y][x].info);
			if (c->squares[y][x].trap)
				square_free_trap(c, y, x);
			if (c->squares[y][x].obj)
				object_pile_free(c->squares[y][x].obj);
		}
		mem_free(c->squares[y]);
	}
	mem_free(c->squares);

	mem_free(c->feat_count);
	mem_free(c->monsters);
	if (c->name)
		string_free(c->name);
	mem_free(c);
}


/**
 * Standard "find me a location" function
 *
 * Obtains a legal location within the given distance of the initial
 * location, and with "los()" from the source to destination location.
 *
 * This function is often called from inside a loop which searches for
 * locations while increasing the "d" distance.
 *
 * need_los determines whether line of sight is needed
 */
void scatter(struct chunk *c, int *yp, int *xp, int y, int x, int d, bool need_los)
{
	int nx, ny;


	/* Pick a location */
	while (TRUE)
	{
		/* Pick a new location */
		ny = rand_spread(y, d);
		nx = rand_spread(x, d);

		/* Ignore annoying locations */
		if (!square_in_bounds_fully(c, ny, nx)) continue;

		/* Ignore "excessively distant" locations */
		if ((d > 1) && (distance(y, x, ny, nx) > d)) continue;
		
		/* Don't need los */
		if (!need_los) break;

		/* Require "line of sight" if set */
		if (need_los && (los(c, y, x, ny, nx))) break;
	}

	/* Save the location */
	(*yp) = ny;
	(*xp) = nx;
}


/**
 * Get a monster on the current level by its index.
 */
struct monster *cave_monster(struct chunk *c, int idx) {
	if (idx <= 0) return NULL;
	return &c->monsters[idx];
}

/**
 * The maximum number of monsters allowed in the level.
 */
int cave_monster_max(struct chunk *c) {
	return c->mon_max;
}

/**
 * The current number of monsters present on the level.
 */
int cave_monster_count(struct chunk *c) {
	return c->mon_cnt;
}

/**
 * Return the number of doors/traps around (or under) the character.
 */
int count_feats(int *y, int *x, bool (*test)(struct chunk *c, int y, int x), bool under)
{
	int d;
	int xx, yy;
	int count = 0; /* Count how many matches */

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++)
	{
		/* if not searching under player continue */
		if ((d == 8) && !under) continue;

		/* Extract adjacent (legal) location */
		yy = player->py + ddy_ddd[d];
		xx = player->px + ddx_ddd[d];

		/* Paranoia */
		if (!square_in_bounds_fully(cave, yy, xx)) continue;

		/* Must have knowledge */
		if (!square_ismark(cave, yy, xx)) continue;

		/* Not looking for this feature */
		if (!((*test)(cave, yy, xx))) continue;

		/* Count it */
		++count;

		/* Remember the location of the last door found */
		*y = yy;
		*x = xx;
	}

	/* All done */
	return count;
}
