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

#include "cmd-core.h"

/**
 * Modes for item lists in "show_inven()"  "show_equip()" and "show_floor()"
 */
typedef enum {
	OLIST_NONE   = 0x00,   /* No options */
	OLIST_WINDOW = 0x01,   /* Display list in a sub-term (left-align) */
	OLIST_QUIVER = 0x02,   /* Display quiver lines */
	OLIST_GOLD   = 0x04,   /* Include gold in the list */
	OLIST_WEIGHT = 0x08,   /* Show item weight */
	OLIST_PRICE  = 0x10,   /* Show item price */
	OLIST_FAIL   = 0x20,    /* Show device failure */
	OLIST_SEMPTY = 0x40
} olist_detail_t;


/*
 * Bit flags for get_item() function
 */
#define USE_EQUIP     0x0001	/* Allow equip items */
#define USE_INVEN     0x0002	/* Allow inven items */
#define USE_FLOOR     0x0004	/* Allow floor items */
#define IS_HARMLESS   0x0008	/* Ignore generic warning inscriptions */
#define SHOW_PRICES   0x0010	/* Show item prices in item lists */
#define SHOW_FAIL     0x0020 	/* Show device failure in item lists */
#define SHOW_QUIVER   0x0040	/* Show quiver summary when in inventory */
#define SHOW_EMPTY    0x0080	/* Show empty slots in equipment display */
#define QUIVER_TAGS   0x0100	/* 0-9 are quiver slots when selecting */


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
