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
 * monsters, objects and/or traps, and optionally delete those things from
 * the source chunk
 * \param y0
 * \param x0 coordinates of the top left corner of the chunk being written
 * \param height
 * \param width dimensions of the chunk being written
 * \param monsters whether monsters get written
 * \param objects whether objects get written
 * \param traps whether traps get written
 * \param delete_old whether monsters/objects/traps get deleted from the source
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
			int this_o_idx, next_o_idx, held;

			/* Terrain */
			new->feat[y][x] = cave->feat[y0 + y][x0 + x];
			sqinfo_copy(new->info[y][x], cave->info[y0 + y][x0 + x]);

			/* Dungeon objects */
			if (objects){
				if (square_object(cave, y0 + y, x0 + x)) {
					new->o_idx[y][x] = cave_object_count(new) + 1;
					for (this_o_idx = cave->o_idx[y0 + y][x0 + x]; this_o_idx;
						 this_o_idx = next_o_idx) {
						object_type *source_obj = cave_object(cave, this_o_idx);
						object_type *dest_obj = cave_object(new, ++new->obj_cnt);
						
						/* Copy over */
						object_copy(dest_obj, source_obj);
						
						/* Adjust stuff */
						dest_obj->iy = y;
						dest_obj->ix = x;
						next_o_idx = source_obj->next_o_idx;
						if (next_o_idx)
							dest_obj->next_o_idx = new->obj_cnt + 1;
						delete_object_idx(this_o_idx);
					}
				}
			}

			/* Monsters and held objects */
			if (monsters){
				held = 0;
				if (cave->m_idx[y0 + y][x0 + x] > 0) {
					monster_type *source_mon = square_monster(cave, y0 + y, x0 + x);
					monster_type *dest_mon = NULL;

					/* Valid monster */
					if (!source_mon->race)
						continue;

					/* Copy over */
					new->m_idx[y][x] = ++new->mon_cnt;
					dest_mon = cave_monster(new, new->mon_cnt);
					memcpy(dest_mon, source_mon, sizeof(*source_mon));

					/* Adjust position */
					dest_mon->fy = y;
					dest_mon->fx = x;

					/* Held objects */
					if (objects && source_mon->hold_o_idx) {
						for (this_o_idx = source_mon->hold_o_idx; this_o_idx;
							 this_o_idx = next_o_idx) {
							object_type *source_obj = cave_object(cave, this_o_idx);
							object_type *dest_obj = cave_object(new, ++new->obj_cnt);
							
							/* Copy over */
							object_copy(dest_obj, source_obj);

							/* Adjust stuff */
							dest_obj->iy = y;
							dest_obj->ix = x;
							next_o_idx = source_obj->next_o_idx;
							if (next_o_idx)
								dest_obj->next_o_idx = cave_object_count(new) + 1;
							dest_obj->held_m_idx = cave_monster_count(new);
							if (!held)
								held = cave_object_count(new);
							delete_object_idx(this_o_idx);
						}
					}
					dest_mon->hold_o_idx = held;
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
				trap->fy -= y0;
				trap->fx -= x0;
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
 * \param rotate how much to rotate, in multiples of 90 degrees clockwise
 * \param reflect whether to reflect horizontally
 */
void symmetry_transform(int *y, int *x, int y0, int x0, int height, int width,
						int rotate, bool reflect)
{
	int i;

	/* Rotate (in multiples of 90 degrees clockwise) */
    for (i = 0; i < rotate % 4; i++)
    {
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
 * Write a chunk, transformed, to a given offset in another chunk
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
			int this_o_idx, next_o_idx, held;
			bool first_obj = TRUE;

			/* Work out where we're going */
			int dest_y = y;
			int dest_x = x;
			symmetry_transform(&dest_y, &dest_x, y0, x0, h, w, rotate, reflect);

			/* Terrain */
			dest->feat[dest_y][dest_x] = source->feat[y][x];
			sqinfo_copy(dest->info[dest_y][dest_x], source->info[y][x]);

			/* Dungeon objects */
			held = 0;
			if (source->o_idx[y][x]) {
				for (this_o_idx = source->o_idx[y][x]; this_o_idx;
					 this_o_idx = next_o_idx) {
					object_type *source_obj = cave_object(source, this_o_idx);
					object_type *dest_obj = NULL;
					int o_idx = 0;

					/* Is this the first object on this square? */
					if (first_obj) {
						/* Make an object */
						o_idx = o_pop(dest);

						/* Hope this never happens */
						if (!o_idx)
							break;

						/* Mark this square as holding this object */
						dest->o_idx[dest_y][dest_x] = o_idx;

						first_obj = FALSE;
					}

					/* Copy over */
					dest_obj = cave_object(dest, o_idx);
					object_copy(dest_obj, source_obj);

					/* Adjust position */
					dest_obj->iy = dest_y;
					dest_obj->ix = dest_x;

					/* Tell the monster on this square what it's holding */
					if (source_obj->held_m_idx) {
						if (!held)
							held = o_idx;
					}

					/* Look at the next object, if there is one */
					next_o_idx = source_obj->next_o_idx;

					/* Make a slot for it if there is, and point to it */
					if (next_o_idx) {
						o_idx = o_pop(dest);
						if (!o_idx)
							break;
						dest_obj->next_o_idx = o_idx;
					}
				}
			}

			/* Monsters */
			if (source->m_idx[y][x] > 0) {
				monster_type *source_mon = square_monster(source, y, x);
				monster_type *dest_mon = NULL;
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
				dest->m_idx[dest_y][dest_x] = idx;
				memcpy(dest_mon, source_mon, sizeof(*source_mon));

				/* Adjust stuff */
				dest_mon->midx = idx;
				dest_mon->fy = dest_y;
				dest_mon->fx = dest_x;

				/* Objects take some work */
				if (source_mon->hold_o_idx) {
					int o_idx = 0;
					first_obj = TRUE;
					for (this_o_idx = source_mon->hold_o_idx; this_o_idx;
						 this_o_idx = next_o_idx) {
						object_type *held_obj = cave_object(source, this_o_idx);
						object_type *dest_obj = NULL;

						/* Is this the first object on this square? */
						if (first_obj) {
							/* Make an object */
							o_idx = o_pop(dest);

							/* Hope this never happens */
							if (!o_idx)
								break;

							/* Mark this monster as holding this object */
							dest_mon->hold_o_idx = o_idx;

							first_obj = FALSE;
						}

						/* Copy over */
						dest_obj = cave_object(dest, o_idx);
						object_copy(dest_obj, held_obj);

						/* No position, held by this monster */
						dest_obj->iy = 0;
						dest_obj->ix = 0;
						dest_obj->held_m_idx = dest_mon->midx;

						/* Look at the next object, if there is one */
						next_o_idx = held_obj->next_o_idx;

						/* Make a slot for it if there is, and point to it */
						if (next_o_idx) {
							o_idx = o_pop(dest);
							if (!o_idx)
								break;
							dest_obj->next_o_idx = o_idx;
						}
					}
				}
			}

			/* Traps */
			if (source->squares[y][x].trap) {
				struct trap *trap = source->squares[y][x].trap;
				struct trap *new_trap = mem_zalloc(sizeof(*new_trap));
				dest->squares[y][x].trap = new_trap;

				/* Traverse the trap list */
				while (trap) {
					struct trap *last_new_trap = new_trap;

					/* Copy over */
					memcpy(new_trap, trap, sizeof(*trap));

					/* Adjust location */
					new_trap->fy = dest_y;
					new_trap->fx = dest_x;

					/* Step, make a new trap if needed, point at it */
					trap = trap->next;
					if (trap) {
						new_trap = mem_zalloc(sizeof(*new_trap));
						last_new_trap->next = new_trap;
					}
				}
			}

			/* Player */
			if (source->m_idx[y][x] == -1) 
				dest->m_idx[dest_y][dest_x] = -1;
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
	int this_o_idx, next_o_idx;

	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			for (this_o_idx = c->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx) {
				assert(c->objects[this_o_idx].tval != 0);
				next_o_idx = c->objects[this_o_idx].next_o_idx;
			}
			if (c->m_idx[y][x] > 0) {
				monster_type *mon = square_monster(c, y, x);
				if (mon->hold_o_idx) {
					for (this_o_idx = mon->hold_o_idx; this_o_idx; this_o_idx = next_o_idx) {
						assert(c->objects[this_o_idx].tval != 0);
						next_o_idx = c->objects[this_o_idx].next_o_idx;
					}
					
				}
			}
		}
	}
}

