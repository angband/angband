/**
 * \file obj-chest.h
 * \brief Encapsulation of chest-related functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2012 Peter Denison
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

#ifndef OBJECT_CHEST_H
#define OBJECT_CHEST_H

/**
 * Chest trap flags (see "obj-chest.c")
 */
#define CHEST_LOSE_STR	0x01
#define CHEST_LOSE_CON	0x02
#define CHEST_POISON	0x04
#define CHEST_PARALYZE	0x08
#define CHEST_EXPLODE	0x10
#define CHEST_SUMMON	0x20


/**
 * Chest check types
 */
enum chest_query {
	CHEST_ANY,
	CHEST_OPENABLE,
	CHEST_TRAPPED
};

byte chest_trap_type(const struct object *obj);
bool is_trapped_chest(const struct object *obj);
bool is_locked_chest(const struct object *obj);
void unlock_chest(struct object *obj);
struct object *chest_check(int y, int x, enum chest_query check_type);
int count_chests(int *y, int *x, enum chest_query check_type);
bool do_cmd_open_chest(int y, int x, struct object *obj);
bool do_cmd_disarm_chest(int y, int x, struct object *obj);

#endif /* OBJECT_CHEST_H */
