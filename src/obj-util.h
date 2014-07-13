/**
   \file obj-util.h
   \brief Object list maintenance and other object utilities
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

#include "cave.h"
#include "player.h"

/* Maximum number of scroll titles generated */
#define MAX_TITLES	 50

/* An item's pval (for charges, amount of gold, etc) is limited to s16b */
#define MAX_PVAL  32767

/**
 * Maximum number of objects allowed in a single dungeon grid.
 *
 * The main screen originally had a minimum size of 24 rows, so it could always
 * display 23 objects + 1 header line.
 */
#define MAX_FLOOR_STACK			23

/**
 * Modes for stacking by object_similar()
 */
typedef enum
{
	OSTACK_NONE    = 0x00, /* No options (this does NOT mean no stacking) */
	OSTACK_STORE   = 0x01, /* Store stacking */
	OSTACK_PACK    = 0x02, /* Inventory and home */
	OSTACK_LIST    = 0x04, /* Object list */
	OSTACK_MONSTER = 0x08, /* Monster carrying objects */
	OSTACK_FLOOR   = 0x10, /* Floor stacking */
	OSTACK_QUIVER  = 0x20  /* Quiver */
} object_stack_t;


void flavor_init(void);
void object_flags(const object_type *o_ptr, bitflag flags[OF_SIZE]);
void object_flags_known(const object_type *o_ptr, bitflag flags[OF_SIZE]);
bool object_test(item_tester tester, const struct object *o);
bool item_test(item_tester tester, int item);
bool is_unknown(const object_type *o_ptr);
int scan_floor(int *items, int max_size, int y, int x, int mode,
			   item_tester tester);
void excise_object_idx(int o_idx);
void delete_object_idx(int o_idx);
void delete_object(int y, int x);
void compact_objects(int size);
void object_free(struct object *obj);
void wipe_o_list(struct chunk *c);
s16b o_pop(struct chunk *c);
object_type *get_first_object(int y, int x);
object_type *get_next_object(const object_type *o_ptr);
bool object_stackable(const object_type *o_ptr, const object_type *j_ptr,
					  object_stack_t mode);
bool object_similar(const object_type *o_ptr, const object_type *j_ptr,
					object_stack_t mode);
void object_absorb_partial(object_type *o_ptr, object_type *j_ptr);
void object_absorb(object_type *o_ptr, object_type *j_ptr);
void object_wipe(object_type *o_ptr);
void object_copy(object_type *o_ptr, const object_type *j_ptr);
void object_copy_amt(object_type *dst, object_type *src, int amt);
void object_split(struct object *dest, struct object *src, int amt);
s16b floor_carry(struct chunk *c, int y, int x, object_type *j_ptr);
void drop_near(struct chunk *c, object_type *j_ptr, int chance, int y, int x,
			   bool verbose);
void push_object(int y, int x);
void acquirement(int y1, int x1, int level, int num, bool great);
void floor_item_charges(int item);
void floor_item_describe(int item);
void floor_item_increase(int item, int num);
void floor_item_optimize(int item);
unsigned check_for_inscrip(const object_type *o_ptr, const char *inscrip);
struct object_kind *lookup_kind(int tval, int sval);
struct object_kind *objkind_byid(int kidx);
int lookup_artifact_name(const char *name);
int lookup_sval(int tval, const char *name);
int compare_items(const object_type *o1, const object_type *o2);
void display_object_idx_recall(s16b o_idx);
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
object_type *object_from_item_idx(int item);
bool obj_needs_aim(object_type *o_ptr);
bool obj_can_fail(const struct object *o);

int scan_items(int *item_list, size_t item_list_max, int mode,
			   item_tester tester);
bool item_is_available(int item, bool (*tester)(const object_type *), int mode);
int get_use_device_chance(const object_type *o_ptr);
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
void reduce_charges(object_type *o_ptr, int amt);
int number_charging(const object_type *o_ptr);
bool recharge_timeout(object_type *o_ptr);


#endif /* OBJECT_UTIL_H */
