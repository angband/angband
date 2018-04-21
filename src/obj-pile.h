/**
 * \file obj-pile.h
 * \brief Deal with piles of objects
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

struct object *object_new(void);
void object_delete(struct object **obj_address);
void object_pile_free(struct object *obj);

void pile_insert(struct object **pile, struct object *obj);
void pile_insert_end(struct object **pile, struct object *obj);
void pile_excise(struct object **pile, struct object *obj);
struct object *pile_last_item(struct object *const pile);
bool pile_contains(const struct object *top, const struct object *obj);

bool object_stackable(const struct object *obj1, const struct object *obj2,
					  object_stack_t mode);
bool object_similar(const struct object *obj1, const struct object *obj2,
					object_stack_t mode);
void object_absorb_partial(struct object *obj1, struct object *obj2);
void object_absorb(struct object *obj1, struct object *obj2);
void object_wipe(struct object *obj);
void object_copy(struct object *obj1, const struct object *obj2);
void object_copy_amt(struct object *dest, struct object *src, int amt);
struct object *object_split(struct object *src, int amt);
struct object *floor_object_for_use(struct object *obj, int num, bool message,
									bool *none_left);
bool floor_carry(struct chunk *c, int y, int x, struct object *drop, bool last);
void drop_near(struct chunk *c, struct object *dropped, int chance, int y,
			   int x, bool verbose);
void push_object(int y, int x);
void floor_item_charges(struct object *obj);
int scan_floor(struct object **items, int max_size, int y, int x, int mode,
			   item_tester tester);
int scan_items(struct object **item_list, size_t item_list_max, int mode,
			   item_tester tester);
bool item_is_available(struct object *obj, bool (*tester)(const struct object *), int mode);
