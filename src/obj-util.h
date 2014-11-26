/**
 * \file obj-util.h
 * \brief Object list maintenance and other object utilities
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef OBJECT_UTIL_H
#define OBJECT_UTIL_H

/* Maximum number of scroll titles generated */
#define MAX_TITLES	 50

/* An item's pval (for charges, amount of gold, etc) is limited to s16b */
#define MAX_PVAL  32767

void flavor_init(void);
void object_flags(const object_type *o_ptr, bitflag flags[OF_SIZE]);
void object_flags_known(const object_type *o_ptr, bitflag flags[OF_SIZE]);
bool object_test(item_tester tester, const struct object *o);
bool item_test(item_tester tester, int item);
bool is_unknown(const object_type *o_ptr);
void acquirement(int y1, int x1, int level, int num, bool great);
unsigned check_for_inscrip(const object_type *o_ptr, const char *inscrip);
struct object_kind *lookup_kind(int tval, int sval);
struct object_kind *objkind_byid(int kidx);
int lookup_artifact_name(const char *name);
int lookup_sval(int tval, const char *name);
void object_short_name(char *buf, size_t max, const char *name);
int compare_items(const object_type *o1, const object_type *o2);
void display_object_recall(struct object *obj);
void display_object_kind_recall(struct object_kind *kind);
void display_object_recall_interactive(object_type *o_ptr);
bool obj_has_charges(const object_type *o_ptr);
bool obj_can_zap(const object_type *o_ptr);
bool obj_is_activatable(const object_type *o_ptr);
bool obj_can_activate(const object_type *o_ptr);
bool obj_can_refill(const object_type *o_ptr);
bool obj_can_browse(const object_type *o_ptr);
bool obj_can_cast_from(const object_type *o_ptr);
bool obj_can_study(const object_type *o_ptr);
bool obj_can_takeoff(const object_type *o_ptr);
bool obj_can_wear(const object_type *o_ptr);
bool obj_can_fire(const object_type *o_ptr);
bool obj_has_inscrip(const object_type *o_ptr);
bool obj_is_useable(const object_type *o_ptr);
u16b object_effect(const object_type *o_ptr);
bool obj_needs_aim(object_type *o_ptr);
bool obj_can_fail(const struct object *o);

int get_use_device_chance(const object_type *o_ptr);
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
void reduce_charges(object_type *o_ptr, int amt);
int number_charging(const object_type *o_ptr);
bool recharge_timeout(object_type *o_ptr);


#endif /* OBJECT_UTIL_H */
