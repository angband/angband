/*
 * File: obj-util.h
 * Purpose: Object list maintenance and other object utilities
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


struct object_kind *objkind_get(int tval, int sval);
struct object_kind *objkind_byid(int kidx);
void flavor_init(void);
void object_flags(const object_type *o_ptr, bitflag flags[OF_SIZE]);
void object_flags_known(const object_type *o_ptr, bitflag flags[OF_SIZE]);
bool item_tester_okay(const object_type *o_ptr);
int scan_floor(int *items, int max_size, int y, int x, int mode, item_tester tester);
void excise_object_idx(int o_idx);
void delete_object_idx(int o_idx);
void delete_object(int y, int x);
void compact_objects(int size);
void wipe_o_list(struct chunk *c);
s16b o_pop(struct chunk *c);
object_type *get_first_object(int y, int x);
object_type *get_next_object(const object_type *o_ptr);
bool is_blessed(const object_type *o_ptr);
s32b object_value(const object_type *o_ptr, int qty, int verbose);
s32b object_value_real(const object_type *o_ptr, int qty, int verbose,
					   bool known);
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
object_kind *lookup_kind(int tval, int sval);
int lookup_name(int tval, const char *name);
int lookup_artifact_name(const char *name);
int lookup_sval(int tval, const char *name);
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
bool obj_is_used_aimed(const object_type *o_ptr);
bool obj_is_used_unaimed(const object_type *o_ptr);
u16b object_effect(const object_type *o_ptr);
object_type *object_from_item_idx(int item);
int gear_index_matching_object(const object_type *o_ptr);
bool obj_needs_aim(object_type *o_ptr);
bool obj_can_fail(const struct object *o);
bool object_test(item_tester tester, const struct object *o);
bool item_test(item_tester tester, int item);

int scan_items(int *item_list, size_t item_list_max, int mode, item_tester tester);
bool item_is_available(int item, bool (*tester)(const object_type *), int mode);
extern void display_itemlist(void);
extern void display_object_idx_recall(s16b o_idx);
extern void display_object_kind_recall(struct object_kind *kind);
void display_object_recall_interactive(object_type *o_ptr);
bool is_unknown(const object_type *o_ptr);
int compare_items(const object_type *o1, const object_type *o2);


#endif /* OBJECT_UTIL_H */
