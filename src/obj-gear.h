/**
  \file: obj-gear.h
  \brief management of inventory, equipment and quiver
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2014 Nick McConnell
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

#ifndef OBJECT_GEAR_H
#define OBJECT_GEAR_H

#include "player.h"

/*
 * Player equipment slot types
 */
enum
{
	#define EQUIP(a, b, c, d, e, f) EQUIP_##a,
	#include "list-equip-slots.h"
	#undef EQUIP
	EQUIP_MAX
};

char index_to_label(int i);
s16b label_to_inven(int c);
s16b label_to_equip(int c);
bool wearable_p(const object_type *o_ptr);
int get_use_device_chance(const object_type *o_ptr);
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
void reduce_charges(object_type *o_ptr, int amt);
int number_charging(const object_type *o_ptr);
bool recharge_timeout(object_type *o_ptr);
int slot_by_name(struct player *p, const char *name);
bool slot_type_is(int slot, int type);
int slot_index(struct player *p, int slot);
struct object *equipped_item_by_slot(struct player *p, int slot);
struct object *equipped_item_by_slot_name(struct player *p, const char *name);
int equipped_item_slot(struct player *p, int item);
bool item_is_equipped(struct player *p, int item);
int object_gear_index(struct player *p, const struct object *obj);
const char *equip_mention(struct player *p, int slot);
const char *equip_describe(struct player *p, int slot);
s16b wield_slot(const object_type *o_ptr);
int minus_ac(struct player *p);
void inven_item_charges(int item);
void inven_item_describe(int item);
void inven_item_increase(int item, int num);
void inven_item_optimize(int item);
bool inven_carry_okay(const object_type *o_ptr);
bool inven_stack_okay(const object_type *o_ptr);
int gear_find_slot(struct player *p);
s16b inven_carry(struct player *p, struct object *o);
void inven_takeoff(int item);
void inven_drop(int item, int amt);
void combine_pack(void);
bool pack_is_full(void);
bool pack_is_overfull(void);
void pack_overflow(void);


#endif /* OBJECT_GEAR_H */
