/**
   \file obj-make.h
   \brief Object generation functions.
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
 * Don't worry about probabilities for anything past dlev100
 */
#define MAX_O_DEPTH		100

/**
 * The chance of inflating the requested object level (1/x).
 * Lower values yield better objects more often.
 */
#define GREAT_OBJ   20

/**
 * There is a 1/20 (5%) chance that ego-items with an inflated base-level are
 * generated when an object is turned into an ego-item (see make_ego_item()).
 * As above, lower values yield better ego-items more often.
 */
#define GREAT_EGO   20

/**
 * Define a value for minima which will be ignored.
 */
#define NO_MINIMUM 	255

/**
 * The largest possible average gold drop at max depth with biggest spread
 */
#define MAX_GOLD_DROP     (3 * MAX_DEPTH + 30)

/**
 * Refueling constants
 */
#define FUEL_TORCH    5000  /* Maximum amount of fuel in a torch */
#define FUEL_LAMP     15000  /* Maximum amount of fuel in a lantern */
#define DEFAULT_TORCH FUEL_TORCH  /* Default amount of fuel in a torch */
#define DEFAULT_LAMP  (FUEL_LAMP / 2)  /* Default amount of fuel in a lantern */

void free_obj_alloc(void);
bool init_obj_alloc(void);
struct object_kind *get_obj_num(int level, bool good, int tval);
bool kind_is_good(const object_kind *kind);
void object_prep(object_type *o_ptr, struct object_kind *kind, int lev, aspect rand_aspect);
int apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great, bool extra_roll);
struct object *make_object(struct chunk *c, int lev, bool good, bool great,
						   bool extra_roll, s32b *value, int tval);
struct object_kind *money_kind(const char *name, int value);
struct object *make_gold(int lev, char *coin_type);
void copy_artifact_data(object_type *o_ptr, const artifact_type *a_ptr);
void ego_apply_magic(object_type *o_ptr, int level);

#endif /* OBJECT_MAKE_H */
