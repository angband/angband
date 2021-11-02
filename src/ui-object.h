/**
 * \file ui-obj.h
 * \brief lists of objects and object pictures
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 * Copyright (c) 2015 Nick McConnell
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
 * Modes for item lists in show_inven(), show_equip(), show_quiver() and
 * show_floor()
 */
typedef enum {
	OLIST_NONE   = 0x00,	/* No options */
	OLIST_WINDOW = 0x01,	/* Display list in a sub-term (left-align) */
	OLIST_QUIVER = 0x02,	/* Display quiver lines */
	OLIST_GOLD   = 0x04,	/* Include gold in the list */
	OLIST_WEIGHT = 0x08,	/* Show item weight */
	OLIST_PRICE  = 0x10,	/* Show item price */
	OLIST_FAIL   = 0x20,	/* Show device failure */
	OLIST_SEMPTY = 0x40,
	OLIST_DEATH  = 0x80,
	OLIST_RECHARGE = 0x100	/* Show failure for device recharging */
} olist_detail_t;


uint8_t object_kind_attr(const struct object_kind *kind);
wchar_t object_kind_char(const struct object_kind *kind);
uint8_t object_attr(const struct object *obj);
wchar_t object_char(const struct object *obj);
void show_inven(int mode, item_tester tester);
void show_equip(int mode, item_tester tester);
void show_quiver(int mode, item_tester tester);
void show_floor(struct object **floor_list, int floor_num, int mode,
				item_tester tester);
bool textui_get_item(struct object **choice, const char *pmt, const char *str,
					 cmd_code cmd, item_tester tester, int mode);
bool get_item_allow(const struct object *obj, unsigned char ch, cmd_code cmd,
					bool is_harmless);

void display_object_recall(struct object *obj);
void display_object_kind_recall(struct object_kind *kind);
void display_object_recall_interactive(struct object *obj);
void textui_obj_examine(void);
void textui_cmd_ignore_menu(struct object *obj);
void textui_cmd_ignore(void);
void textui_cmd_toggle_ignore(void);

#endif /* OBJECT_UI_H */
