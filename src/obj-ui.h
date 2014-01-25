/*
 * File: obj-ui.h
 * Purpose: Mainly object descriptions and generic UI functions
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

#ifndef OBJECT_UI_H
#define OBJECT_UI_H

#include "game-cmd.h"

byte object_kind_attr(const struct object_kind *kind);
wchar_t object_kind_char(const struct object_kind *kind);
byte object_attr(const struct object *o_ptr);
wchar_t object_char(const struct object *o_ptr);
void show_inven(int mode, item_tester tester);
void show_equip(int mode, item_tester tester);
void show_floor(const int *floor_list, int floor_num, int mode, item_tester tester);
bool verify_item(const char *prompt, int item);
bool get_item(int *cp, const char *pmt, const char *str, cmd_code cmd, item_tester tester, int mode);
bool get_item_allow(int item, unsigned char ch, cmd_code cmd, bool is_harmless);

#endif /* OBJECT_UI_H */
