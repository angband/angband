/*
 * File: obj-chest.h
 * Purpose: Encapsulation of chest-related functions
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

byte chest_trap_type(const object_type *o_ptr);
bool is_trapped_chest(const object_type *o_ptr);
bool is_locked_chest(const object_type *o_ptr);
void unlock_chest(object_type *o_ptr);
s16b chest_check(int y, int x, enum chest_query check_type);
int count_chests(int *y, int *x, enum chest_query check_type);
bool do_cmd_open_chest(int y, int x, s16b o_idx);
bool do_cmd_disarm_chest(int y, int x, s16b o_idx);

#endif /* OBJECT_CHEST_H */
