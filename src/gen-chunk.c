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
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "obj-util.h"
#include "trap.h"

#define CHUNK_LIST_INCR 10
struct chunk **chunk_list;     /**< list of pointers to saved chunks */
u16b chunk_list_max = 0;      /**< current max actual chunk index */

/**
 * Write the terrain info of a chunk to memory and return a pointer to it
 *
 * \param c chunk being written
 * \return the memory location of the chunk
 */
struct chunk *chunk_write(struct chunk *c)
{
	int x, y;

	struct chunk *new = cave_new(c->height, c->width);

	/* Write the location stuff */
	for (y = 0; y < new->height; y++) {
		for (x = 0; x < new->width; x++) {
			/* Terrain */
			new->squares[y][x].feat = square(c, loc(x, y)).feat;
			sqinfo_copy(square(new, loc(x, y)).info, square(c, loc(x, y)).info);
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
	if ((chunk_list_max % CHUNK_LIST_INCR) == 0)
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
	int i;

	/* Find the match */
	for (i = 0; i < chunk_list_max; i++) {
		if (!strcmp(name, chunk_list[i]->name)) {
			/* Copy all the succeeding chunks back one */
			int j;
			for (j = i + 1; j < chunk_list_max; j++) {
				chunk_list[j - 1] = chunk_list[j];
			}

			/* Shorten the list and return */
			chunk_list_max--;
			chunk_list[chunk_list_max] = NULL;
			return true;
		}
	}

	return false;
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
		if (c == chunk_list[i]) return true;

	return false;
}

/**
 * Find the saved chunk above or below the current player depth
 */
struct chunk *chunk_find_adjacent(struct player *p, bool above)
{
	int depth = above ? p->depth - 1 : p->depth + 1;
	struct level *lev = level_by_depth(depth);

	if (lev) {
		return chunk_find_name(lev->name);
	}

	return NULL;
}

/**
 * Transform y, x coordinates by rotation, reflection and translation
 * Stolen from PosChengband
 * \param y the coordinates being transformed
 * \param x the coordinates being transformed
 * \param y0 how much the coordinates are being translated
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
 * \param y0 transformation parameters  - see symmetry_transform()
 * \param x0 transformation parameters  - see symmetry_transform()
 * \param rotate transformation parameters  - see symmetry_transform()
 * \param reflect transformation parameters  - see symmetry_transform()
 * \return success - fails if the copy would not fit in the destination chunk
 */
bool chunk_copy(struct chunk *dest, struct chunk *source, int y0, int x0,
				int rotate, bool reflect)
{
	int i, max_group_id = 0;
	int y, x;
	int h = source->height, w = source->width;

	/* Check bounds */
	if (rotate % 1) {
		if ((w + y0 > dest->height) || (h + x0 > dest->width))
			return false;
	} else {
		if ((h + y0 > dest->height) || (w + x0 > dest->width))
			return false;
	}

	/* Write the location stuff */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			/* Work out where we're going */
			int dest_y = y;
			int dest_x = x;
			symmetry_transform(&dest_y, &dest_x, y0, x0, h, w, rotate, reflect);

			/* Terrain */
			dest->squares[dest_y][dest_x].feat = square(source, loc(x, y)).feat;
			sqinfo_copy(square(dest, loc(dest_x, dest_y)).info,
						square(source, loc(x, y)).info);

			/* Dungeon objects */
			if (square_object(source, loc(x, y))) {
				struct object *obj;
				dest->squares[dest_y][dest_x].obj = square_object(source, loc(x, y));

				for (obj = square_object(source, loc(x, y)); obj; obj = obj->next) {
					/* Adjust position */
					obj->grid = loc(dest_x, dest_y);
				}
				source->squares[y][x].obj = NULL;
			}

			/* Monsters */
			if (square(source, loc(x, y)).mon > 0) {
				struct monster *source_mon = square_monster(source, loc(x, y));
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
				dest_mon->grid = loc(dest_x, dest_y);

				/* Held objects */
				if (source_mon->held_obj)
					dest_mon->held_obj = source_mon->held_obj;
			}

			/* Traps */
			if (square(source, loc(x, y)).trap) {
				struct trap *trap = square(source, loc(x, y)).trap;
				dest->squares[dest_y][dest_x].trap = trap;

				/* Traverse the trap list */
				while (trap) {
					/* Adjust location */
					trap->grid = loc(dest_x, dest_y);
					trap = trap->next;
				}
				source->squares[y][x].trap = NULL;
			}

			/* Player */
			if (square(source, loc(x, y)).mon == -1) 
				dest->squares[dest_y][dest_x].mon = -1;
		}
	}

	/* Copy object list */
	dest->objects = mem_realloc(dest->objects,
								(dest->obj_max + source->obj_max + 2)
								* sizeof(struct object*));
	for (i = 0; i <= source->obj_max; i++) {
		dest->objects[dest->obj_max + i] = source->objects[i];
		if (dest->objects[dest->obj_max + i] != NULL)
			dest->objects[dest->obj_max + i]->oidx = dest->obj_max + i;
	}
	dest->obj_max += source->obj_max + 1;

	/* Copy monster group list */
	for (i = 0; i < z_info->level_monster_max; i++) {
		if (dest->monster_groups[i]) max_group_id = i;
	}
	for (i = 0; i < z_info->level_monster_max - max_group_id; i++) {
		dest->monster_groups[i + max_group_id] = source->monster_groups[i];
	}

	/* Miscellany */
	for (i = 0; i < z_info->f_max + 1; i++)
		dest->feat_count[i] += source->feat_count[i];

	dest->obj_rating += source->obj_rating;
	dest->mon_rating += source->mon_rating;

	if (source->good_item)
		dest->good_item = true;

	return true;
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
			struct loc grid = loc(x, y);
			for (obj = square_object(c, grid); obj; obj = obj->next)
				assert(obj->tval != 0);
			if (square(c, grid).mon > 0) {
				struct monster *mon = square_monster(c, grid);
				if (mon->held_obj)
					for (obj = mon->held_obj; obj; obj = obj->next)
						assert(obj->tval != 0);
			}
		}
	}
}

