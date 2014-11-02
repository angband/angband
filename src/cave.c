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
#include "dungeon.h"
#include "cmd-core.h"
#include "game-event.h"
#include "init.h"
#include "monster.h"
#include "obj-ignore.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "tables.h"
#include "trap.h"

struct feature *f_info;
struct chunk *cave = NULL;
struct chunk *cave_k = NULL;

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
	FEAT_MAGMA = lookup_feat("magma vein");
	FEAT_QUARTZ = lookup_feat("quartz vein");
	FEAT_MAGMA_K = lookup_feat("magma vein with treasure");
	FEAT_QUARTZ_K = lookup_feat("quartz vein with treasure");
	FEAT_GRANITE = lookup_feat("granite wall");
	FEAT_PERM = lookup_feat("permanent wall");
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
	c->info = mem_zalloc(c->height * sizeof(bitflag**));
	c->feat = mem_zalloc(c->height * sizeof(byte*));
	c->m_idx = mem_zalloc(c->height * sizeof(s16b*));
	c->o_idx = mem_zalloc(c->height * sizeof(s16b*));
	for (y = 0; y < c->height; y++){
		c->info[y] = mem_zalloc(c->width * sizeof(bitflag*));
		for (x = 0; x < c->width; x++)
			c->info[y][x] = mem_zalloc(SQUARE_SIZE * sizeof(bitflag));
		c->feat[y] = mem_zalloc(c->width * sizeof(byte));
		c->m_idx[y] = mem_zalloc(c->width * sizeof(s16b));
		c->o_idx[y] = mem_zalloc(c->width * sizeof(s16b));
	}

	c->squares = mem_zalloc(c->height * sizeof(struct square*));
	for (y = 0; y < c->height; y++) {
		c->squares[y] = mem_zalloc(c->width * sizeof(struct square));
		for (x = 0; x < c->width; x++)
			c->squares[y][x].info = mem_zalloc(SQUARE_SIZE * sizeof(bitflag));
	}

	c->monsters = mem_zalloc(z_info->level_monster_max *sizeof(struct monster));
	c->mon_max = 1;
	c->mon_current = -1;

	c->objects = mem_zalloc(z_info->level_object_max * sizeof(struct object));
	c->obj_max = 1;

	c->created_at = turn;
	return c;
}

/**
 * Free a chunk
 */
void cave_free(struct chunk *c) {
	int y, x;
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++)
			mem_free(c->squares[y][x].info);
		mem_free(c->squares[y]);
	}
	mem_free(c->squares);

	for (y = 0; y < c->height; y++){
		for (x = 0; x < c->width; x++)
			mem_free(c->info[y][x]);
		mem_free(c->info[y]);
		mem_free(c->feat[y]);
		mem_free(c->m_idx[y]);
		mem_free(c->o_idx[y]);
	}
	mem_free(c->feat_count);
	mem_free(c->info);
	mem_free(c->feat);
	mem_free(c->m_idx);
	mem_free(c->o_idx);
	mem_free(c->monsters);
	mem_free(c->objects);
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
 * Get an object on the current level by its index.
 */
struct object *cave_object(struct chunk *c, int idx) {
	assert(idx > 0);
	assert(idx <= z_info->level_object_max);
	return &c->objects[idx];
}

/**
 * The maximum number of objects allowed in the level.
 */
int cave_object_max(struct chunk *c) {
	return c->obj_max;
}

/**
 * The current number of objects present on the level.
 */
int cave_object_count(struct chunk *c) {
	return c->obj_cnt;
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
