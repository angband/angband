/**
 * \file ui-object.c
 * \brief Various object-related UI functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
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

#include "angband.h"
#include "cave.h"
#include "cmds.h"
#include "cmd-core.h"
#include "effects.h"
#include "game-input.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "ui-command.h"
#include "ui-menu.h"
#include "ui-options.h"

enum {
	IGNORE_THIS_ITEM,
	UNIGNORE_THIS_ITEM,
	IGNORE_THIS_FLAVOR,
	UNIGNORE_THIS_FLAVOR,
	IGNORE_THIS_EGO,
	UNIGNORE_THIS_EGO,
	IGNORE_THIS_QUALITY
};

void textui_cmd_ignore_menu(struct object *obj)
{
	char out_val[160];

	struct menu *m;
	region r;
	int selected;

	if (!obj)
		return;

	m = menu_dynamic_new();
	m->selections = lower_case;

	/* Basic ignore option */
	if (!obj->ignore) {
		menu_dynamic_add(m, "This item only", IGNORE_THIS_ITEM);
	} else {
		menu_dynamic_add(m, "Unignore this item", UNIGNORE_THIS_ITEM);
	}

	/* Flavour-aware ignore */
	if (ignore_tval(obj->tval) &&
			(!obj->artifact || !object_flavor_is_aware(obj))) {
		bool ignored = kind_is_ignored_aware(obj->kind) ||
				kind_is_ignored_unaware(obj->kind);

		char tmp[70];
		object_desc(tmp, sizeof(tmp), obj, ODESC_BASE | ODESC_PLURAL);
		if (!ignored) {
			strnfmt(out_val, sizeof out_val, "All %s", tmp);
			menu_dynamic_add(m, out_val, IGNORE_THIS_FLAVOR);
		} else {
			strnfmt(out_val, sizeof out_val, "Unignore all %s", tmp);
			menu_dynamic_add(m, out_val, UNIGNORE_THIS_FLAVOR);
		}
	}

	/* Ego ignoring */
	if (object_ego_is_visible(obj)) {
		ego_desc choice;
		struct ego_item *ego = obj->ego;
		char tmp[80] = "";

		choice.e_idx = ego->eidx;
		choice.itype = ignore_type_of(obj);
		choice.short_name = "";
		(void) ego_item_name(tmp, sizeof(tmp), &choice);
		if (!ego_is_ignored(choice.e_idx, choice.itype)) {
			strnfmt(out_val, sizeof out_val, "All %s", tmp + 4);
			menu_dynamic_add(m, out_val, IGNORE_THIS_EGO);
		} else {
			strnfmt(out_val, sizeof out_val, "Unignore all %s", tmp + 4);
			menu_dynamic_add(m, out_val, UNIGNORE_THIS_EGO);
		}
	}

	/* Quality ignoring */
	if (object_was_sensed(obj) || object_was_worn(obj) ||
			object_is_known_not_artifact(obj)) {
		byte value = ignore_level_of(obj);
		int type = ignore_type_of(obj);

		if (tval_is_jewelry(obj) &&
					ignore_level_of(obj) != IGNORE_BAD)
			value = IGNORE_MAX;

		if (value != IGNORE_MAX && type != ITYPE_MAX) {
			strnfmt(out_val, sizeof out_val, "All %s %s",
					quality_values[value].name, ignore_name_for_type(type));

			menu_dynamic_add(m, out_val, IGNORE_THIS_QUALITY);
		}
	}

	/* Work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	r.col = 80 - r.width;
	r.row = 1;
	r.page_rows = m->count;

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Ignore:", 0, 0);
	selected = menu_dynamic_select(m);

	screen_load();

	if (selected == IGNORE_THIS_ITEM) {
		obj->ignore = TRUE;
	} else if (selected == UNIGNORE_THIS_ITEM) {
		obj->ignore = FALSE;
	} else if (selected == IGNORE_THIS_FLAVOR) {
		object_ignore_flavor_of(obj);
	} else if (selected == UNIGNORE_THIS_FLAVOR) {
		kind_ignore_clear(obj->kind);
	} else if (selected == IGNORE_THIS_EGO) {
		ego_ignore(obj);
	} else if (selected == UNIGNORE_THIS_EGO) {
		ego_ignore_clear(obj);
	} else if (selected == IGNORE_THIS_QUALITY) {
		byte value = ignore_level_of(obj);
		int type = ignore_type_of(obj);

		ignore_level[type] = value;
	}

	player->upkeep->notice |= PN_IGNORE;

	menu_dynamic_free(m);
}

void textui_cmd_ignore(void)
{
	struct object *obj;

	/* Get an item */
	const char *q = "Ignore which item? ";
	const char *s = "You have nothing to ignore.";
	if (!get_item(&obj, q, s, CMD_IGNORE, NULL,
				  USE_INVEN | USE_QUIVER | USE_EQUIP | USE_FLOOR))
		return;

	textui_cmd_ignore_menu(obj);
}

void textui_cmd_toggle_ignore(void)
{
	player->unignoring = !player->unignoring;
	player->upkeep->notice |= PN_IGNORE;
	do_cmd_redraw();
}

/**
 * Examine an object
 */
void textui_obj_examine(void)
{
	char header[120];

	textblock *tb;
	region area = { 0, 0, 0, 0 };

	struct object *obj;

	/* Select item */
	if (!get_item(&obj, "Examine which item?", "You have nothing to examine.",
			CMD_NULL, NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR | IS_HARMLESS)))
		return;

	/* Track object for object recall */
	track_object(player->upkeep, obj);

	/* Display info */
	tb = object_info(obj, OINFO_NONE);
	object_desc(header, sizeof(header), obj,
			ODESC_PREFIX | ODESC_FULL | ODESC_CAPITAL);

	textui_textblock_show(tb, area, header);
	textblock_free(tb);
}
