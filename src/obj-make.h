/**
 * \file obj-make.h
 * \brief Object generation functions.
 *
 * Copyright (c) 1987-2007 Angband contributors
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

#ifndef OBJECT_MAKE_H
#define OBJECT_MAKE_H

#include "cave.h"

/**
 * Define a value for minima which will be ignored (a replacement for 0,
 * because 0 and some small negatives are valid values).
 */
#define NO_MINIMUM 	255

/**
 * (Roughly) the largest possible gold drop at max depth - this is actually
 * derivable from make_gold(), but this is near enough
 */
#define MAX_GOLD_DROP     (3 * MAX_DEPTH + 30)

void ego_apply_magic(struct object *o_ptr, int level);
void copy_artifact_data(struct object *o_ptr, const struct artifact *a_ptr);
void object_prep(struct object *o_ptr, struct object_kind *kind, int lev,
				 aspect rand_aspect);
int apply_magic(struct object *o_ptr, int lev, bool okay, bool good,
				bool great, bool extra_roll);
bool kind_is_good(const object_kind *kind);
struct object_kind *get_obj_num(int level, bool good, int tval);
struct object *make_object(struct chunk *c, int lev, bool good, bool great,
						   bool extra_roll, s32b *value, int tval);
void acquirement(int y1, int x1, int level, int num, bool great);
struct object_kind *money_kind(const char *name, int value);
struct object *make_gold(int lev, char *coin_type);

#endif /* OBJECT_MAKE_H */
