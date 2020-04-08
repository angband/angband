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
 * Chest check types
 */
enum chest_query {
	CHEST_ANY,
	CHEST_OPENABLE,
	CHEST_TRAPPED
};

struct chest_trap *chest_traps;
struct chest *chests;
extern struct file_parser chest_trap_parser;

char *chest_trap_name(const struct object *obj);
bool is_trapped_chest(const struct object *obj);
bool is_locked_chest(const struct object *obj);
int pick_chest_traps(struct object *obj);
void unlock_chest(struct object *obj);
struct object *chest_check(struct loc grid, enum chest_query check_type);
int count_chests(struct loc *grid, enum chest_query check_type);
bool do_cmd_open_chest(struct loc grid, struct object *obj);
bool do_cmd_disarm_chest(struct object *obj);

#endif /* OBJECT_CHEST_H */
