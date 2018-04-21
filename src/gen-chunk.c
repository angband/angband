/**
 * \file gen-chunk.c 
 * \brief Handling of chunks of cave
 *
 * Copyright (c) 2014 Nick McConnell
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
 * This file maintains a list of saved chunks of world which can be reloaded
 * at any time.  The intitial example of this is the town, which is saved 
 * immediately after generation and restored when the player returns there.
 *
 * The copying routines are also useful for generating a level in pieces and
 * then copying those pieces into the actual level chunk.
 */

#include "angband.h"
#include "cave.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "obj-util.h"
#include "trap.h"

#define CHUNK_LIST_INCR 10
struct chunk **chunk_list;     /**< list of pointers to saved chunks */
u16b chunk_list_max = 0;      /**< current max actual chunk index */

/**
 * Write a chunk to memory and return a pointer to it.  Optionally write
 * monsters, objects and/or traps, and in those cases delete those things from
 * the source chunk
 * \param y0
 * \param x0 coordinates of the top left corner of the chunk being written
 * \param height
 * \param width dimensions of the chunk being written
 * \param monsters whether monsters get written
 * \param objects whether objects get written
 * \param traps whether traps get written
 * \return the memory location of the chunk
 */
struct chunk *chunk_write(int y0, int x0, int height, int width, bool monsters,
						 bool objects, bool traps)
{
	int x, y;

	struct chunk *new = cave_new(height, width);

	/* Write the location stuff */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/* Terrain */
			new->squares[y][x].feat = cave->squares[y0 + y][x0 + x].feat;
			sqinfo_copy(new->squares[y][x].info,
						cave->squares[y0 + y][x0 + x].info);

			/* Dungeon objects */
			if (objects) {
				struct object *obj = square_object(cave, y0 + y, x0 + x);
				if (obj) {
					new->squares[y][x].obj = obj;
					while (obj) {
						/* Adjust stuff */
						obj->iy = y;
						obj->ix = x;
					}
				}
			}

			/* Monsters and held objects */
			if (monsters) {
				if (cave->squares[y0 + y][x0 + x].mon > 0) {
					struct monster *source_mon = square_monster(cave, y0 + y,
															  x0 + x);
					struct monster *dest_mon = NULL;

					/* Valid monster */
					if (!source_mon->race)
						continue;

					/* Copy over */
					new->squares[y][x].mon = ++new->mon_cnt;
					dest_mon = cave_monster(new, new->mon_cnt);
					memcpy(dest_mon, source_mon, sizeof(*source_mon));

					/* Adjust position */
					dest_mon->fy = y;
					dest_mon->fx = x;

					/* Held objects */
					if (objects && source_mon->held_obj)
						dest_mon->held_obj = source_mon->held_obj;

					delete_monster(y0 + y, x0 + x);
				}
			}

			/* Traps */
			if (traps) {
				/* Copy over */
				struct trap *trap = cave->squares[y][x].trap;
				new->squares[y][x].trap = trap;
				cave->squares[y][x].trap = NULL;

				/* Adjust position */
				trap->fy = y;
				trap->fx = x;
			}
		}
	}

	return new;
}

/**
 * Add an entry to the chunk list - any problems with the length of this will
 * be more in the memory used by the chunks themselves rather than the list
 * \param c the chunk being added to the list
 */
void chunk_list_add(struct chunk *c)
{
	int newsize = (chunk_list_max + CHUNK_LIST_INCR) *	sizeof(struct chunk *);

	/* Lengthen the list if necessary */
	if (chunk_list_max == 0)
		chunk_list = mem_zalloc(newsize);
	else if ((chunk_list_max % CHUNK_LIST_INCR) == 0)
		chunk_list = (struct chunk **) mem_realloc(chunk_list, newsize);

	/* Add the new one */
	chunk_list[chunk_list_max++] = c;
}

/**
 * Remove an entry from the chunk list, return whether it was found
 * \param name the name of the chunk being removed from the list
 * \return whether it was found; success means it was successfully removed
 */
bool chunk_list_remove(char *name)
{
	int i, j;
	int newsize = 0;

	for (i = 0; i < chunk_list_max; i++) {
		/* Find the match */
		if (!strcmp(name, chunk_list[i]->name)) {
			/* Copy all the succeeding ones back one */
			for (j = i + 1; j < chunk_list_max; j++)
				chunk_list[j - 1] = chunk_list[j];

			/* Destroy the last one, and shorten the list */
			if ((chunk_list_max % CHUNK_LIST_INCR) == 0)
				newsize = (chunk_list_max - CHUNK_LIST_INCR) *	
					sizeof(struct chunk *);
			chunk_list_max--;
			chunk_list[chunk_list_max] = NULL;
			if (newsize)
				chunk_list = (struct chunk **) mem_realloc(chunk_list, newsize);

			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Find a chunk by name
 * \param name the name of the chunk being sought
 * \return the pointer to the chunk
 */
struct chunk *chunk_find_name(char *name)
{
	int i;

	for (i = 0; i < chunk_list_max; i++)
		if (!strcmp(name, chunk_list[i]->name))
			return chunk_list[i];

	return NULL;
}

/**
 * Find a chunk by pointer
 * \param c the actual pointer to the sought chunk
 * \return if it was found
 */
bool chunk_find(struct chunk *c)
{
	int i;

	for (i = 0; i < chunk_list_max; i++)
		if (c == chunk_list[i]) return TRUE;

	return FALSE;
}

/**
 * Transform y, x coordinates by rotation, reflection and translation
 * Stolen from PosChengband
 * \param y
 * \param x the coordinates being transformed
 * \param y0
 * \param x0 how much the coordinates are being translated
 * \param height height of the chunk
 * \param width width of the chunk
 * \param rotate how much to rotate, in multiples of 90 degrees clockwise
 * \param reflect whether to reflect horizontally
 */
void symmetry_transform(int *y, int *x, int y0, int x0, int height, int width,
						int rotate, bool reflect)
{
	int i;

	/* Rotate (in multiples of 90 degrees clockwise) */
    for (i = 0; i < rotate % 4; i++) {
        int temp = *x;
        *x = height - 1 - (*y);
        *y = temp;
    }

	/* Reflect (horizontally) */
	if (reflect)
		*x = width - 1 - *x;

	/* Translate */
	*y += y0;
	*x += x0;
}

/**
 * Write a chunk, transformed, to a given offset in another chunk.  Note that
 * objects are copied from the old chunk and not retained there
 * \param dest the chunk where the copy is going
 * \param source the chunk being copied
 * \param y0
 * \param x0 
 * \param rotate 
 * \param reflect transformation parameters  - see symmetry_transform()
 * \return success - fails if the copy would not fit in the destination chunk
 */
bool chunk_copy(struct chunk *dest, struct chunk *source, int y0, int x0,
				int rotate, bool reflect)
{
	int i;
	int y, x;
	int h = source->height, w = source->width;

	/* Check bounds */
	if (rotate % 1) {
		if ((w + y0 > dest->height) || (h + x0 > dest->width))
			return FALSE;
	} else {
		if ((h + y0 > dest->height) || (w + x0 > dest->width))
			return FALSE;
	}

	/* Write the location stuff */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			/* Work out where we're going */
			int dest_y = y;
			int dest_x = x;
			symmetry_transform(&dest_y, &dest_x, y0, x0, h, w, rotate, reflect);

			/* Terrain */
			dest->squares[dest_y][dest_x].feat = source->squares[y][x].feat;
			sqinfo_copy(dest->squares[dest_y][dest_x].info,
						source->squares[y][x].info);

			/* Dungeon objects */
			if (square_object(source, y, x)) {
				struct object *obj;
				dest->squares[dest_y][dest_x].obj = square_object(source, y, x);

				for (obj = square_object(source, y, x); obj; obj = obj->next) {
					/* Adjust position */
					obj->iy = dest_y;
					obj->ix = dest_x;
				}
			}

			/* Monsters */
			if (source->squares[y][x].mon > 0) {
				struct monster *source_mon = square_monster(source, y, x);
				struct monster *dest_mon = NULL;
				int idx;

				/* Valid monster */
				if (!source_mon->race)
					continue;

				/* Make a monster */
				idx = mon_pop(dest);

				/* Hope this never happens */
				if (!idx)
					break;

				/* Copy over */
				dest_mon = cave_monster(dest, idx);
				dest->squares[dest_y][dest_x].mon = idx;
				memcpy(dest_mon, source_mon, sizeof(*source_mon));

				/* Adjust stuff */
				dest_mon->midx = idx;
				dest_mon->fy = dest_y;
				dest_mon->fx = dest_x;

				/* Held objects */
				if (source_mon->held_obj)
					dest_mon->held_obj = source_mon->held_obj;
			}

			/* Traps */
			if (source->squares[y][x].trap) {
				struct trap *trap = source->squares[y][x].trap;
				dest->squares[y][x].trap = trap;

				/* Traverse the trap list */
				while (trap) {
					/* Adjust location */
					trap->fy = dest_y;
					trap->fx = dest_x;
					trap = trap->next;
				}
			}

			/* Player */
			if (source->squares[y][x].mon == -1) 
				dest->squares[dest_y][dest_x].mon = -1;
		}
	}

	/* Miscellany */
	for (i = 0; i < z_info->f_max + 1; i++)
		dest->feat_count[i] += source->feat_count[i];

	dest->obj_rating += source->obj_rating;
	dest->mon_rating += source->mon_rating;

	if (source->good_item)
		dest->good_item = TRUE;

	return TRUE;
}

/**
 * Validate that the chunk contains no NULL objects. 
 * Only checks for nonzero tval.
 * \param c is the chunk to validate.
 */

void chunk_validate_objects(struct chunk *c) {
	int x, y;
	struct object *obj;

	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			for (obj = square_object(c, y, x); obj; obj = obj->next)
				assert(obj->tval != 0);
			if (c->squares[y][x].mon > 0) {
				struct monster *mon = square_monster(c, y, x);
				if (mon->held_obj)
					for (obj = mon->held_obj; obj; obj = obj->next)
						assert(obj->tval != 0);
			}
		}
	}
}

