/**
 * \file obj-tval.h
 * \brief Wrapper functions for tvals.
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

#include "object.h"

/**
 * The values for the "tval" field of various objects.
 *
 * This value is the primary means by which items are sorted in the
 * player inventory, followed by "sval" and "cost".
 */
enum
{
	#define TV(a, b) TV_##a,
	#include "list-tvals.h"
	#undef TV

	TV_MAX
};

/**
 * Special "sval" value -- unknown "sval"
 */
#define SV_UNKNOWN			0

bool tval_can_have_charges(const struct object *obj);
bool tval_can_have_failure(const struct object *obj);
bool tval_can_have_flavor_k(const struct object_kind *kind);
bool tval_can_have_nourishment(const struct object *obj);
bool tval_can_have_timeout(const struct object *obj);
int tval_find_idx(const char *name);
const char *tval_find_name(int tval);
bool tval_is_ammo(const struct object *obj);
bool tval_is_armor(const struct object *obj);
bool tval_is_body_armor(const struct object *obj);
bool tval_is_book_k(const struct object_kind *kind);
bool tval_is_chest(const struct object *obj);
bool tval_is_food(const struct object *obj);
bool tval_is_food_k(const struct object_kind *kind);
bool tval_is_mushroom(const struct object *obj);
bool tval_is_mushroom_k(const struct object_kind *kind);
bool tval_is_fuel(const struct object *obj);
bool tval_is_head_armor(const struct object *obj);
bool tval_is_jewelry(const struct object *obj);
bool tval_is_launcher(const struct object *obj);
bool tval_is_light(const struct object *obj);
bool tval_is_light_k(const struct object_kind *kind);
bool tval_is_melee_weapon(const struct object *obj);
bool tval_is_money(const struct object *obj);
bool tval_is_money_k(const struct object_kind *kind);
bool tval_is_pointy(const struct object *obj);
bool tval_is_digger(const struct object *obj);
bool tval_is_potion(const struct object *obj);
bool tval_is_ring(const struct object *obj);
bool tval_is_rod(const struct object *obj);
bool tval_is_scroll(const struct object *obj);
bool tval_is_staff(const struct object *obj);
bool tval_is_useable(const struct object *obj);
bool tval_is_wand(const struct object *obj);
bool tval_is_weapon(const struct object *obj);
bool tval_has_variable_power(const struct object *obj);
bool tval_is_wearable(const struct object *obj);
bool tval_is_edible(const struct object *obj);
bool tval_is_zapper(const struct object *obj);
int tval_sval_count(const char *name);
int tval_sval_list(const char *name, int *list, int max_size);

#endif /* OBJECT_TVAL_H */
