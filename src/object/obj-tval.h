/*
 * File: obj-tval.h
 * Purpose: Wrapper functions for tvals.
 *
 * Copyright (c) 2014 Ben Semmler
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

#ifndef OBJECT_TVAL_H
#define OBJECT_TVAL_H

#include "tvalsval.h"
#include "object.h"

bool tval_can_have_charges(const struct object *o_ptr);
bool tval_can_have_failure(const struct object *o_ptr);
bool tval_can_have_nourishment(const struct object *o_ptr);
bool tval_is_ammo(const struct object *o_ptr);
bool tval_is_armor(const struct object *o_ptr);
bool tval_is_body_armor(const struct object *o_ptr);
bool tval_is_chest(const struct object *o_ptr);
bool tval_is_food(const struct object *o_ptr);
bool tval_is_jewelry(const struct object *o_ptr);
bool tval_is_light(const struct object *o_ptr);
bool tval_is_money(const struct object *o_ptr);
bool tval_is_pointy(const struct object *o_ptr);
bool tval_is_potion(const struct object *o_ptr);
bool tval_is_ring(const struct object *o_ptr);
bool tval_is_rod(const struct object *o_ptr);
bool tval_is_scroll(const struct object *o_ptr);
bool tval_is_staff(const struct object *o_ptr);
bool tval_is_useable(const struct object *o_ptr);
bool tval_is_wand(const struct object *o_ptr);
bool tval_is_weapon(const struct object *o_ptr);

#endif /* OBJECT_TVAL_H */
