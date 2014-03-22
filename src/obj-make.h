/*
 * File: obj-make.h
 * Purpose: Object generation functions.
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

void free_obj_alloc(void);
bool init_obj_alloc(void);
object_kind *get_obj_num(int level, bool good, int tval);
void object_prep(object_type *o_ptr, struct object_kind *kind, int lev, aspect rand_aspect);
s16b apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great, bool extra_roll);
bool make_object(struct chunk *c, object_type *j_ptr, int lev, bool good, bool great, bool extra_roll, s32b *value, int tval);
void make_gold(object_type *j_ptr, int lev, int coin_type);
void copy_artifact_data(object_type *o_ptr, const artifact_type *a_ptr);
void ego_apply_magic(object_type *o_ptr, int level);
void ego_min_pvals(object_type *o_ptr);

#endif /* OBJECT_MAKE_H */
