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
 * Set one grid location equal to another
 */
void loc_set_eq(struct loc *dest, struct loc source)
{
	(*dest).x = source.x;
	(*dest).y = source.y;
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
void add_to_point_set(struct point_set *ps, int y, int x)
{
	ps->pts[ps->n].x = x;
	ps->pts[ps->n].y = y;
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

int point_set_contains(struct point_set *ps, int y, int x)
{
	int i;
	for (i = 0; i < ps->n; i++)
		if (ps->pts[i].x == x && ps->pts[i].y == y)
			return 1;
	return 0;
}
