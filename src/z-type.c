/**
 * \file z-type.c
 * \brief Support various data types.
 *
 * Copyright (c) 2007 Angband Developers
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

#include "z-rand.h"
#include "z-type.h"
#include "z-virt.h"

struct loc loc(int x, int y) {
	struct loc p;
	p.x = x;
	p.y = y;
	return p;
}

/**
 * Determine if two grid locations are equal
 */
bool loc_eq(struct loc grid1, struct loc grid2)
{
	return (grid1.x == grid2.x) && (grid1.y == grid2.y);
}

/**
 * Determine if a grid location is the (0, 0) location
 */
bool loc_is_zero(struct loc grid)
{
	return loc_eq(grid, loc(0, 0));
}

/**
 * Sum two grid locations
 */
struct loc loc_sum(struct loc grid1, struct loc grid2)
{
	return loc(grid1.x + grid2.x, grid1.y + grid2.y);
}

/**
 * Take the difference of two grid locations
 */
struct loc loc_diff(struct loc grid1, struct loc grid2)
{
	return loc(grid1.x - grid2.x, grid1.y - grid2.y);
}

/**
 * Get a random location with the given x and y centres and spread 
 */
struct loc rand_loc(struct loc grid, int x_spread, int y_spread)
{
	return loc(rand_spread(grid.x, x_spread), rand_spread(grid.y, y_spread));
}

struct loc loc_offset(struct loc grid, int dx, int dy)
{
	return loc(grid.x + dx, grid.y + dy);
}

/**
 * Utility functions to work with point_sets
 */
struct point_set *point_set_new(int initial_size)
{
	struct point_set *ps = mem_alloc(sizeof(struct point_set));
	ps->n = 0;
	ps->allocated = initial_size;
	ps->pts = mem_zalloc(sizeof(*(ps->pts)) * ps->allocated);
	return ps;
}

void point_set_dispose(struct point_set *ps)
{
	mem_free(ps->pts);
	mem_free(ps);
}

/**
 * Add the point to the given point set, making more space if there is
 * no more space left.
 */
void add_to_point_set(struct point_set *ps, struct loc grid)
{
	ps->pts[ps->n] = grid;
	ps->n++;
	if (ps->n >= ps->allocated) {
		ps->allocated *= 2;
		ps->pts = mem_realloc(ps->pts, sizeof(*(ps->pts)) * ps->allocated);
	}
}

int point_set_size(struct point_set *ps)
{
	return ps->n;
}

int point_set_contains(struct point_set *ps, struct loc grid)
{
	int i;
	for (i = 0; i < ps->n; i++)
		if (loc_eq(ps->pts[i], grid))
			return 1;
	return 0;
}
