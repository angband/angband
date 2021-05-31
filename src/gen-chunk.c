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
 * at any time.  The initial example of this is the town, which is saved
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
#include "mon-group.h"
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
			new->squares[y][x].feat = square(c, loc(x, y))->feat;
			sqinfo_copy(square(new, loc(x, y))->info, square(c, loc(x, y))->info);
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
bool chunk_list_remove(const char *name)
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
struct chunk *chunk_find_name(const char *name)
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
 * \param grid the grid being transformed
 * \param y0 how much the grid is being translated vertically
 * \param x0 how much the grid is being translated horizontally
 * \param height height of the chunk
 * \param width width of the chunk
 * \param rotate how much to rotate, in multiples of 90 degrees clockwise
 * \param reflect whether to reflect horizontally
 */
void symmetry_transform(struct loc *grid, int y0, int x0, int height, int width,
						int rotate, bool reflect)
{
	/* Track what the dimensions are after rotations. */
	int rheight = height, rwidth = width;
	int i;

	/* Rotate (in multiples of 90 degrees clockwise) */
	for (i = 0; i < rotate % 4; i++) {
		int temp = grid->x;
		grid->x = rheight - 1 - (grid->y);
		grid->y = temp;
		temp = rwidth;
		rwidth = rheight;
		rheight = temp;
	}

	/* Reflect (horizontally in the rotated system) */
	if (reflect)
		grid->x = rwidth - 1 - grid->x;

	/* Translate */
	grid->y += y0;
	grid->x += x0;
}

/**
 * Select a random symmetry transformation subject to certain constraints.
 * \param height Is the height of the piece to transform.
 * \param width Is the width of the piece to transform.
 * \param flags Is a bitwise-or of one or more of SYMTR_FLAG_NONE,
 * SYMTR_FLAG_NO_ROT (disallow 90 and 270 degree rotation and 180 degree
 * rotation if not accompanied by a horizontal reflection - equivalent to a
 * vertical reflection), SYMTR_FLAG_NO_REF (forbid horizontal reflection), and
 * SYMTR_FLAG_FORCE_REF (force horizontal reflection).  If flags
 * includes both SYMTR_FLAG_NO_REF and SYMTR_FLAG_FORCE_REF, the former takes
 * precedence.
 * \param transpose_weight Is the probability weight to use for transformations
 * that include a tranposition (90 degree rotation, 270 degree rotation,
 * 90 degree rotation + horizontal reflection, 270 degree rotation + horizontal
 * reflection).  Coerced to be in the range of [0, SYMTR_MAX_WEIGHT] where 0
 * means forbidding such transformations.
 * \param rotate *rotate is set to the number of 90 degree clockwise rotations
 * to perform for the random transform.
 * \param reflect *reflect is set to whether the random transform includes a
 * horizontal reflection.
 * \param theight If theight is not NULL, *theight is set to the height of the
 * piece after applying the transform.
 * \param twidth If twidth is not NULL, *twidth is set to the width of the
 * piece after applying the transform.
 */
void get_random_symmetry_transform(int height, int width, int flags,
	int transpose_weight, int *rotate, bool *reflect,
	int *theight, int *twidth)
{
	/*
	 * Without any constraints there are 8 possibilities (4 rotations times
	 * 2 options for whether or not there is a horizontal reflection).
	 * Use an array of 9 elements (extra element for a leading zero) to
	 * store the cumulative probability weights.  The first four are for
	 * rotations without reflection.  The remainder are for the rotations
	 * with reflection.
	 */
	int weights[9], draw, ilow, ihigh;

	transpose_weight = MIN(SYMTR_MAX_WEIGHT, MAX(0, transpose_weight));
	weights[0] = 0;
	if ((flags & SYMTR_FLAG_NO_REF) || !(flags & SYMTR_FLAG_FORCE_REF)) {
		weights[1] = weights[0] + SYMTR_MAX_WEIGHT;
	} else {
		weights[1] = weights[0];
	}
	if (flags & SYMTR_FLAG_NO_ROT) {
		weights[2] = weights[1];
		weights[3] = weights[2];
		weights[4] = weights[3];
	} else if ((flags & SYMTR_FLAG_NO_REF) ||
			!(flags & SYMTR_FLAG_FORCE_REF)) {
		weights[2] = weights[1] + transpose_weight;
		weights[3] = weights[2] + SYMTR_MAX_WEIGHT;
		weights[4] = weights[3] + transpose_weight;
	} else {
		/* Reflection is forced so these all have zero weight. */
		weights[2] = weights[1];
		weights[3] = weights[2];
		weights[4] = weights[3];
	}
	if (flags & SYMTR_FLAG_NO_REF) {
		/* Reflection is forbidden so these all have zero weight. */
		weights[5] = weights[4];
		weights[6] = weights[5];
		weights[7] = weights[6];
		weights[8] = weights[7];
	} else {
		weights[5] = weights[4] + SYMTR_MAX_WEIGHT;
		if (flags & SYMTR_FLAG_NO_ROT) {
			weights[6] = weights[5];
			/*
			 * 180 degree rotation with a horizontal reflection is
			 * equivalent to a vertical reflection so don't exclude
			 * in when forbidding rotations.
			 */
			weights[7] = weights[6] + SYMTR_MAX_WEIGHT;
			weights[8] = weights[7];
		} else {
			weights[6] = weights[5] + transpose_weight;
			weights[7] = weights[6] + SYMTR_MAX_WEIGHT;
			weights[8] = weights[7] + transpose_weight;
		}
	}
	assert(weights[8] > 0);

	draw = randint0(weights[8]);

	/* Find by a binary search. */
	ilow = 0;
	ihigh = 8;
	while (1) {
		int imid;

		if (ilow == ihigh - 1) {
			break;
		}
		imid = (ilow + ihigh) / 2;
		if (weights[imid] <= draw) {
			ilow = imid;
		} else {
			ihigh = imid;
		}
	}

	*rotate = ilow % 4;
	*reflect = (ilow >= 4);
	if (theight) {
		*theight = (*rotate == 0 || *rotate == 2) ?  height : width;
	}
	if (twidth) {
		*twidth = (*rotate == 0 || *rotate == 2) ?  width : height;
	}
}

/**
 * Select a weight for transforms that involve transpositions so that
 * such transforms are forbidden if width >= 2 * height and the probability of
 * such a transform increases as height / width up to a maximum of
 * SYMTR_MAX_WEIGHT when the height is greater than or equal to the width.
 * That's so transformed pieces will usually fit well into the aspect ratio
 * of generated levels.
 * \param height Is the height of the piece being transformed.
 * \param width Is the width of the piece being transformed.
 */
int calc_default_transpose_weight(int height, int width)
{
	return (SYMTR_MAX_WEIGHT / 64) *
		MAX(0, MIN(64, (128 * height) / width - 64));
}

/**
 * Write a chunk, transformed, to a given offset in another chunk.
 *
 * This function assumes that it is being called at level generation, when
 * there has been no interaction between the player and the level, monsters
 * have not been activated, all monsters are in only one group, and objects
 * are in their original positions.
 *
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
	struct loc grid;
	int h = source->height, w = source->width;
	int mon_skip = dest->mon_max - 1;

	/* Check bounds */
	if (rotate % 1) {
		if ((w + y0 > dest->height) || (h + x0 > dest->width))
			return false;
	} else {
		if ((h + y0 > dest->height) || (w + x0 > dest->width))
			return false;
	}

	/* Write the location stuff (terrain, objects, traps) */
	for (grid.y = 0; grid.y < h; grid.y++) {
		for (grid.x = 0; grid.x < w; grid.x++) {
			/* Work out where we're going */
			struct loc dest_grid = grid;
			symmetry_transform(&dest_grid, y0, x0, h, w, rotate, reflect);

			/* Terrain */
			dest->squares[dest_grid.y][dest_grid.x].feat =
				square(source, grid)->feat;
			sqinfo_copy(square(dest, dest_grid)->info,
						square(source, grid)->info);

			/* Dungeon objects */
			if (square_object(source, grid)) {
				struct object *obj;
				dest->squares[dest_grid.y][dest_grid.x].obj =
					square_object(source, grid);

				for (obj = square_object(source, grid); obj; obj = obj->next) {
					/* Adjust position */
					obj->grid = dest_grid;
				}
				source->squares[grid.y][grid.x].obj = NULL;
			}

			/* Traps */
			if (square(source, grid)->trap) {
				struct trap *trap = square(source, grid)->trap;
				dest->squares[dest_grid.y][dest_grid.x].trap = trap;

				/* Traverse the trap list */
				while (trap) {
					/* Adjust location */
					trap->grid = dest_grid;
					trap = trap->next;
				}
				source->squares[grid.y][grid.x].trap = NULL;
			}

			/* Player */
			if (square(source, grid)->mon == -1) {
				dest->squares[dest_grid.y][dest_grid.x].mon = -1;
				player->grid = dest_grid;
			}
		}
	}

	/* Monsters */
	dest->mon_max += source->mon_max;
	dest->mon_cnt += source->mon_cnt;
	dest->num_repro += source->num_repro;
	for (i = 1; i < source->mon_max; i++) {
		struct monster *source_mon = &source->monsters[i];
		struct monster *dest_mon = &dest->monsters[mon_skip + i];

		/* Valid monster */
		if (!source_mon->race) continue;

		/* Copy */
		memcpy(dest_mon, source_mon, sizeof(struct monster));

		/* Adjust monster index */
		dest_mon->midx += mon_skip;

		/* Move grid */
		symmetry_transform(&dest_mon->grid, y0, x0, h, w, rotate, reflect);
		dest->squares[dest_mon->grid.y][dest_mon->grid.x].mon = dest_mon->midx;

		/* Held or mimicked objects */
		if (source_mon->held_obj) {
			struct object *obj;
			dest_mon->held_obj = source_mon->held_obj;
			for (obj = source_mon->held_obj; obj; obj = obj->next) {
				obj->held_m_idx = dest_mon->midx;
			}
		}
		if (source_mon->mimicked_obj) {
			dest_mon->mimicked_obj = source_mon->mimicked_obj;
		}
	}

	/* Find max monster group id */
	for (i = 1; i < z_info->level_monster_max; i++) {
		if (dest->monster_groups[i]) max_group_id = i;
	}

	/* Copy monster groups */
	for (i = 1; i < z_info->level_monster_max - max_group_id; i++) {
		struct monster_group *group = source->monster_groups[i];
		struct mon_group_list_entry *entry;

		/* Copy monster group list */
		dest->monster_groups[i + max_group_id] = source->monster_groups[i];

		/* Adjust monster group indices */
		if (!group) continue;
		entry = group->member_list;
		group->index += max_group_id;
		group->leader += mon_skip;
		while (entry) {
			int idx = entry->midx;
			struct monster *mon = &dest->monsters[mon_skip + idx];
			entry->midx = mon->midx;
			assert(entry->midx == mon_skip + idx);
			mon->group_info[0].index += max_group_id;
			entry = entry->next;
		}
	}
	monster_groups_verify(dest);

	/* Copy object list */
	dest->objects = mem_realloc(dest->objects,
								(dest->obj_max + source->obj_max + 2)
								* sizeof(struct object*));
	for (i = 0; i <= source->obj_max; i++) {
		dest->objects[dest->obj_max + i] = source->objects[i];
		if (dest->objects[dest->obj_max + i] != NULL)
			dest->objects[dest->obj_max + i]->oidx = dest->obj_max + i;
		source->objects[i] = NULL;
	}
	dest->obj_max += source->obj_max + 1;
	source->obj_max = 1;
	object_lists_check_integrity(dest, NULL);

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
			if (square(c, grid)->mon > 0) {
				struct monster *mon = square_monster(c, grid);
				if (mon->held_obj)
					for (obj = mon->held_obj; obj; obj = obj->next)
						assert(obj->tval != 0);
			}
		}
	}
}

