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

void ego_apply_magic(struct object *obj, int level);
void copy_artifact_data(struct object *obj, const struct artifact *art);
bool make_fake_artifact(struct object *obj, const struct artifact *artifact);
void object_prep(struct object *obj, struct object_kind *kind, int lev,
				 aspect rand_aspect);
int apply_magic(struct object *obj, int lev, bool okay, bool good,
				bool great, bool extra_roll);
bool kind_is_good(const struct object_kind *kind);
struct object_kind *get_obj_num(int level, bool good, int tval);
struct object *make_object(struct chunk *c, int lev, bool good, bool great,
						   bool extra_roll, s32b *value, int tval);
void acquirement(struct loc grid, int level, int num, bool great);
struct object_kind *money_kind(const char *name, int value);
struct object *make_gold(int lev, const char *coin_type);

#endif /* OBJECT_MAKE_H */
