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

#include "object.h"

/*
 * The values for the "tval" field of various objects.
 *
 * This value is the primary means by which items are sorted in the
 * player inventory, followed by "sval" and "cost".
 *
 * Note that a "BOW" with tval = 19 and sval S = 10*N+P takes a missile
 * weapon with tval = 16+N, and does (xP) damage when so combined.  This
 * fact is not actually used in the source, but it kind of interesting.
 *
 * Note that as of 2.7.8, the "item flags" apply to all items, though
 * only armor and weapons and a few other items use any of these flags.
 */

#define TV_NULL		     0
#define TV_CHEST         7	/* Chests ('~') */
#define TV_SHOT	   	    16	/* Ammo for slings */
#define TV_ARROW        17	/* Ammo for bows */
#define TV_BOLT         18	/* Ammo for x-bows */
#define TV_BOW          19	/* Slings/Bows/Xbows */
#define TV_DIGGING      20	/* Shovels/Picks */
#define TV_HAFTED       21	/* Priest Weapons */
#define TV_POLEARM      22	/* Axes and Pikes */
#define TV_SWORD        23	/* Edged Weapons */
#define TV_BOOTS        30	/* Boots */
#define TV_GLOVES       31	/* Gloves */
#define TV_HELM         32	/* Helms */
#define TV_CROWN        33	/* Crowns */
#define TV_SHIELD       34	/* Shields */
#define TV_CLOAK        35	/* Cloaks */
#define TV_SOFT_ARMOR   36	/* Soft Armor */
#define TV_HARD_ARMOR   37	/* Hard Armor */
#define TV_DRAG_ARMOR	38	/* Dragon Scale Mail */
#define TV_LIGHT        39	/* Lights (including Specials) */
#define TV_AMULET       40	/* Amulets (including Specials) */
#define TV_RING         45	/* Rings (including Specials) */
#define TV_STAFF        55
#define TV_WAND         65
#define TV_ROD          66
#define TV_SCROLL       70
#define TV_POTION       75
#define TV_FLASK        77
#define TV_FOOD         80
#define TV_MUSHROOM     81
#define TV_MAGIC_BOOK   90
#define TV_PRAYER_BOOK  91
#define TV_GOLD         100	/* Gold can only be picked up by players */
#define TV_MAX			101

/*
 * Special "sval" value -- unknown "sval"
 */
#define SV_UNKNOWN			0

bool tval_can_have_charges(const struct object *o_ptr);
bool tval_can_have_failure(const struct object *o_ptr);
bool tval_can_have_flavor_k(const struct object_kind *kind);
bool tval_can_have_nourishment(const struct object *o_ptr);
bool tval_can_have_timeout(const struct object *o_ptr);
int tval_find_idx(const char *name);
const char *tval_find_name(int tval);
bool tval_is_ammo(const struct object *o_ptr);
bool tval_is_armor(const struct object *o_ptr);
bool tval_is_body_armor(const struct object *o_ptr);
bool tval_is_book_k(const struct object_kind *kind);
bool tval_is_chest(const struct object *o_ptr);
bool tval_is_food(const struct object *o_ptr);
bool tval_is_food_k(const struct object_kind *k_ptr);
bool tval_is_mushroom(const struct object *o_ptr);
bool tval_is_mushroom_k(const struct object_kind *k_ptr);
bool tval_is_fuel(const struct object *o_ptr);
bool tval_is_head_armor(const struct object *o_ptr);
bool tval_is_jewelry(const struct object *o_ptr);
bool tval_is_launcher(const struct object *o_ptr);
bool tval_is_light(const struct object *o_ptr);
bool tval_is_light_k(const struct object_kind *k_ptr);
bool tval_is_melee_weapon(const struct object *o_ptr);
bool tval_is_money(const struct object *o_ptr);
bool tval_is_money_k(const struct object_kind *kind);
bool tval_is_pointy(const struct object *o_ptr);
bool tval_is_potion(const struct object *o_ptr);
bool tval_is_ring(const struct object *o_ptr);
bool tval_is_rod(const struct object *o_ptr);
bool tval_is_scroll(const struct object *o_ptr);
bool tval_is_staff(const struct object *o_ptr);
bool tval_is_useable(const struct object *o_ptr);
bool tval_is_wand(const struct object *o_ptr);
bool tval_is_weapon(const struct object *o_ptr);
bool tval_is_wearable(const struct object *o_ptr);
bool tval_is_edible(const struct object *o_ptr);
bool tval_is_zapper(const struct object *o_ptr);
int tval_sval_count(const char *name);
int tval_sval_list(const char *name, int *list, int max_size);

#endif /* OBJECT_TVAL_H */
