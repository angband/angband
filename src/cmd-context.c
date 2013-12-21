/*
 * File: cmd-context.c
 * Purpose: Show player and terrain context menus.
 *
 * Copyright (c) 2011 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "cmds.h"
#include "files.h"
#include "game-cmd.h"
#include "keymap.h"
#include "textui.h"
#include "ui-menu.h"
#include "wizard.h"
#include "target.h"
#include "squelch.h"
#include "object/tvalsval.h"
#include "object/object.h"
#include "monster/mon-lore.h"
#include "monster/mon-util.h"

#define ADD_LABEL(text, cmd, valid) { \
	cmdkey = cmd_lookup_key_unktrl((cmd), mode); \
	menu_dynamic_add_label_valid(m, (text), cmdkey, (cmd), labels, (valid)); \
}

/**
 * Additional constants for menu item values. The values must not collide
 * with the cmd_code enum, since those are the main values for these menu items.
 */
enum context_menu_value_e {
    MENU_VALUE_INSPECT = CMD_REPEAT + 1000,
    MENU_VALUE_DROP_ALL,
	MENU_VALUE_LOOK,
	MENU_VALUE_RECALL,
	MENU_VALUE_REST,
	MENU_VALUE_INVENTORY,
	MENU_VALUE_CENTER_MAP,
	MENU_VALUE_FLOOR,
	MENU_VALUE_CHARACTER,
	MENU_VALUE_OTHER,
	MENU_VALUE_KNOWLEDGE,
	MENU_VALUE_MAP,
	MENU_VALUE_MESSAGES,
	MENU_VALUE_OBJECTS,
	MENU_VALUE_MONSTERS,
	MENU_VALUE_TOGGLE_SQUELCHED,
	MENU_VALUE_OPTIONS,
	MENU_VALUE_HELP,
};


static int context_menu_player_2(int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	char *labels;
	bool allowed = TRUE;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Knowledge", '~', MENU_VALUE_KNOWLEDGE, labels);
	menu_dynamic_add_label(m, "Show Map", 'M', MENU_VALUE_MAP, labels);
	menu_dynamic_add_label(m, "^Show Messages", 'P', MENU_VALUE_MESSAGES, labels);
	menu_dynamic_add_label(m, "Show Monster List", '[', MENU_VALUE_MONSTERS, labels);
	menu_dynamic_add_label(m, "Show Object List", ']', MENU_VALUE_OBJECTS, labels);

	ADD_LABEL("Toggle Searching", CMD_TOGGLE_SEARCH, MN_ROW_VALID);

	/* Squelch toggle has different keys, but we don't have a way to look them up (see cmd-process.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'K' : 'O';
	menu_dynamic_add_label(m, "Toggle Squelched", cmdkey, MENU_VALUE_TOGGLE_SQUELCHED, labels);

	ADD_LABEL("Squelch an item", CMD_DESTROY, MN_ROW_VALID);

	menu_dynamic_add_label(m, "Options", '=', MENU_VALUE_OPTIONS, labels);
	menu_dynamic_add_label(m, "Commands", '?', MENU_VALUE_HELP, labels);

	/* work out display region */
	r.width = (int)menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	/* Check the command to see if it is allowed. */
	switch (selected) {
		case -1:
			/* User cancelled the menu. */
			return 3;

		case MENU_VALUE_KNOWLEDGE:
		case MENU_VALUE_MAP:
		case MENU_VALUE_MESSAGES:
		case MENU_VALUE_TOGGLE_SQUELCHED:
		case MENU_VALUE_HELP:
		case MENU_VALUE_MONSTERS:
		case MENU_VALUE_OBJECTS:
		case MENU_VALUE_OPTIONS:
		case CMD_TOGGLE_SEARCH:
			allowed = TRUE;
			break;

		case CMD_DESTROY:
			cmdkey = cmd_lookup_key(selected, mode);
			allowed = key_confirm_command(cmdkey);
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = FALSE;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch (selected) {
		case MENU_VALUE_KNOWLEDGE:
			Term_keypress('~', 0);
			break;

		case MENU_VALUE_MAP:
			Term_keypress('M', 0);
			break;

		case MENU_VALUE_MESSAGES:
			Term_keypress(KTRL('p'), 0);
			break;

		case CMD_DESTROY:
		case CMD_TOGGLE_SEARCH:
			cmdkey = cmd_lookup_key(selected, mode);
			Term_keypress(cmdkey, 0);
			break;

		case MENU_VALUE_TOGGLE_SQUELCHED:
			/* Squelch toggle has different keys, but we don't have a way to look them up (see cmd-process.c). */
			cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'K' : 'O';
			Term_keypress(cmdkey, 0);
			break;

		case MENU_VALUE_HELP:
			context_menu_command(mx, my);
			break;

		case MENU_VALUE_MONSTERS:
			Term_keypress('[', 0);
			break;

		case MENU_VALUE_OBJECTS:
			Term_keypress(']', 0);
			break;

		case MENU_VALUE_OPTIONS:
			Term_keypress('=', 0);
			break;

		default:
			break;
	}

	return 1;
}

static void context_menu_player_display_floor(void)
{
	/* there is an item on the floor, show the inventory screen starting
	 * from the floor */
	//Term_keypress('i',0);
	int diff = weight_remaining();

	/* Hack -- Start in "inventory" mode */
	p_ptr->command_wrk = (USE_FLOOR);

	/* Save screen */
	screen_save();

	/* Prompt for a command */
	prt(format("(Inventory) Burden %d.%d lb (%d.%d lb %s). Item for command: ",
			   p_ptr->total_weight / 10, p_ptr->total_weight % 10,
			   abs(diff) / 10, abs(diff) % 10,
			   (diff < 0 ? "overweight" : "remaining")),
		0, 0);


	/* Get an item to use a context command on */
	if (get_item(&diff, NULL, NULL, CMD_NULL, USE_EQUIP|USE_INVEN|USE_FLOOR|SHOW_EMPTY|IS_HARMLESS)) {
		object_type *o_ptr;

		/* Track the object kind */
		track_object(diff);

		o_ptr = object_from_item_idx(diff);

		context_menu_object(o_ptr, diff);
	}

	/* Load screen */
	screen_load();
}

int context_menu_player(int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	char *labels;
	bool allowed = TRUE;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	ADD_LABEL("Use", CMD_USE_ANY, MN_ROW_VALID);

	/* if player can cast, add casting option */
	if (player_can_cast(p_ptr, FALSE)) {
		ADD_LABEL("Cast", CMD_CAST, MN_ROW_VALID);
	}

	/* if player is on stairs add option to use them */
	if (cave_isupstairs(cave, p_ptr->py, p_ptr->px)) {
		ADD_LABEL("Go Up", CMD_GO_UP, MN_ROW_VALID);
	}
	else if (cave_isdownstairs(cave, p_ptr->py, p_ptr->px)) {
		ADD_LABEL("Go Down", CMD_GO_DOWN, MN_ROW_VALID);
	}

	ADD_LABEL("Search", CMD_SEARCH, MN_ROW_VALID);

	/* Looking has different keys, but we don't have a way to look them up (see cmd-process.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'l' : 'x';
	menu_dynamic_add_label(m, "Look", cmdkey, MENU_VALUE_LOOK, labels);

	/* 'R' is used for resting in both keymaps. */
	menu_dynamic_add_label(m, "Rest", 'R', MENU_VALUE_REST, labels);

	/* 'i' is used for inventory in both keymaps. */
	menu_dynamic_add_label(m, "Inventory", 'i', MENU_VALUE_INVENTORY, labels);

	/* if object under player add pickup option */
	if (cave->o_idx[p_ptr->py][p_ptr->px]) {
		object_type *o_ptr = object_byid(cave->o_idx[p_ptr->py][p_ptr->px]);
		if (!squelch_item_ok(o_ptr)) {
			menu_row_validity_t valid;

			/* 'f' isn't in rogue keymap, so we can use it here. */
  			menu_dynamic_add_label(m, "Floor", 'f', MENU_VALUE_FLOOR, labels);
			valid = (inven_carry_okay(o_ptr)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Pick up", CMD_PICKUP, valid);
		}
	}

	/* 'C' is used for the character sheet in both keymaps. */
	menu_dynamic_add_label(m, "Character", 'C', MENU_VALUE_CHARACTER, labels);

	/* XXX Don't show the keymap line until the keymap list is implemented, to
	 * avoid confusion as to what should be there */
	/*menu_dynamic_add(m, "Keymaps", 10);*/

	if (!OPT(center_player)) {
		menu_dynamic_add_label(m, "^Center Map", 'L', MENU_VALUE_CENTER_MAP, labels);
	}

	menu_dynamic_add_label(m, "Other", ' ', MENU_VALUE_OTHER, labels);

	/* work out display region */
	r.width = (int)menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	cmdkey = cmd_lookup_key(selected, mode);

	/* Check the command to see if it is allowed. */
	switch(selected) {
		case -1:
			/* User cancelled the menu. */
			return 3;

		case CMD_USE_ANY:
		case CMD_CAST:
		case CMD_SEARCH:
		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_PICKUP:
			/* Only check for ^ inscriptions, since we don't have an object selected (if we need one). */
			allowed = key_confirm_command(cmdkey);
			break;

		case MENU_VALUE_REST:
			allowed = key_confirm_command('R');
			break;

		case MENU_VALUE_INVENTORY:
		case MENU_VALUE_LOOK:
		case MENU_VALUE_CHARACTER:
		case MENU_VALUE_OTHER:
		case MENU_VALUE_FLOOR:
		case MENU_VALUE_CENTER_MAP:
			allowed = TRUE;
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = FALSE;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch(selected) {
		case CMD_USE_ANY:
		case CMD_CAST:
			cmdkey = cmd_lookup_key(selected, mode);
			Term_keypress(cmdkey, 0);
			break;

		case CMD_SEARCH:
		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_PICKUP:
			cmd_insert(selected);
			break;

		case MENU_VALUE_REST:
			Term_keypress('R', 0);
			break;

		case MENU_VALUE_INVENTORY:
			Term_keypress('i', 0);
			break;

		case MENU_VALUE_LOOK:
			if (target_set_interactive(TARGET_LOOK, p_ptr->px, p_ptr->py)) {
				msg("Target Selected.");
			}
			break;

		case MENU_VALUE_CHARACTER:
			Term_keypress('C', 0);
			break;

		case MENU_VALUE_OTHER:
			context_menu_player_2(mx, my);
			break;

		case MENU_VALUE_FLOOR:
			context_menu_player_display_floor();
			break;

		case MENU_VALUE_CENTER_MAP:
			do_cmd_center_map();
			break;

		default:
			break;
	}

	return 1;
}

int context_menu_cave(struct cave *c, int y, int x, int adjacent, int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	char *labels;
	bool allowed = TRUE;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	/* Looking has different keys, but we don't have a way to look them up (see cmd-process.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'l' : 'x';
	menu_dynamic_add_label(m, "Look At", cmdkey, MENU_VALUE_LOOK, labels);

	if (c->m_idx[y][x]) {
		/* '/' is used for recall in both keymaps. */
		menu_dynamic_add_label(m, "Recall Info", '/', MENU_VALUE_RECALL, labels);
	}

	ADD_LABEL("Use Item On", CMD_USE_ANY, MN_ROW_VALID);

	if (player_can_cast(p_ptr, FALSE)) {
		ADD_LABEL("Cast On", CMD_CAST, MN_ROW_VALID);
	}

	if (adjacent) {
		ADD_LABEL((c->m_idx[y][x]) ? "Attack" : "Alter", CMD_ALTER, MN_ROW_VALID);

		if (c->o_idx[y][x]) {
			s16b o_idx = chest_check(y,x, CHEST_ANY);
			if (o_idx) {
				object_type *o_ptr = object_byid(o_idx);
				if (!squelch_item_ok(o_ptr)) {
					if (object_is_known(o_ptr)) {
						if (is_locked_chest(o_ptr)) {
							ADD_LABEL("Disarm Chest", CMD_DISARM, MN_ROW_VALID);
							ADD_LABEL("Open Chest", CMD_OPEN, MN_ROW_VALID);
						}
						else {
							ADD_LABEL("Open Disarmed Chest", CMD_OPEN, MN_ROW_VALID);
						}
					}
					else {
						ADD_LABEL("Open Chest", CMD_OPEN, MN_ROW_VALID);
					}
				}
			}
		}

		if (cave_istrap(c, y, x)) {
			ADD_LABEL("Disarm", CMD_DISARM, MN_ROW_VALID);
			ADD_LABEL("Jump Onto", CMD_JUMP, MN_ROW_VALID);
		}

		if (cave_isopendoor(c, y, x)) {
			ADD_LABEL("Close", CMD_CLOSE, MN_ROW_VALID);
		}
		else if (cave_iscloseddoor(c, y, x)) {
			ADD_LABEL("Open", CMD_OPEN, MN_ROW_VALID);
			ADD_LABEL("Lock", CMD_DISARM, MN_ROW_VALID);
		}
		else if (cave_isdiggable(c, y, x)) {
			ADD_LABEL("Tunnel", CMD_TUNNEL, MN_ROW_VALID);
		}

		ADD_LABEL("Search", CMD_SEARCH, MN_ROW_VALID);
		ADD_LABEL("Walk Towards", CMD_WALK, MN_ROW_VALID);
	}
	else {
		/* ',' is used for squelch in rogue keymap, so we'll just swap letters. */
		cmdkey = (mode == KEYMAP_MODE_ORIG) ? ',' : '.';
		menu_dynamic_add_label(m, "Pathfind To", cmdkey, CMD_PATHFIND, labels);

		ADD_LABEL("Walk Towards", CMD_WALK, MN_ROW_VALID);
		ADD_LABEL("Run Towards", CMD_RUN, MN_ROW_VALID);
	}

	if (player_can_fire(p_ptr, FALSE)) {
		ADD_LABEL("Fire On", CMD_FIRE, MN_ROW_VALID);
	}

	ADD_LABEL("Throw To", CMD_THROW, MN_ROW_VALID);

	/* work out display region */
	r.width = (int)menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	region_erase_bordered(&r);
	if (p_ptr->timed[TMD_IMAGE]) {
		prt("(Enter to select command, ESC to cancel) You see something strange:", 0, 0);
	} else
	if (c->m_idx[y][x]) {
		char m_name[80];
		monster_type *m_ptr = cave_monster_at(c, y, x);

		/* Get the monster name ("a kobold") */
		monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_IND_VIS);

		prt(format("(Enter to select command, ESC to cancel) You see %s:", m_name), 0, 0);
	} else
	if (c->o_idx[y][x] && !squelch_item_ok(object_byid(c->o_idx[y][x]))) {
		char o_name[80];

		/* Get the single object in the list */
		object_type *o_ptr = object_byid(c->o_idx[y][x]);

		/* Obtain an object description */
		object_desc(o_name, sizeof (o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		prt(format("(Enter to select command, ESC to cancel) You see %s:", o_name), 0, 0);
	} else
	{
		/* Feature (apply mimic) */
		const char *name = cave_apparent_name(c, p_ptr, y, x);

		/* Hack -- special introduction for store doors */
		if (cave_isshop(cave, y, x)) {
			prt(format("(Enter to select command, ESC to cancel) You see the entrance to the %s:", name), 0, 0);
		} else {
			prt(format("(Enter to select command, ESC to cancel) You see %s %s:",
					(is_a_vowel(name[0])) ? "an" : "a", name), 0, 0);
		}
	}

	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	cmdkey = cmd_lookup_key(selected, mode);

	/* Check the command to see if it is allowed. */
	switch (selected) {
		case -1:
			/* User cancelled the menu. */
			return 3;

		case MENU_VALUE_LOOK:
		case MENU_VALUE_RECALL:
		case CMD_PATHFIND:
			allowed = TRUE;
			break;

		case CMD_SEARCH:
		case CMD_ALTER:
		case CMD_DISARM:
		case CMD_JUMP:
		case CMD_CLOSE:
		case CMD_OPEN:
		case CMD_TUNNEL:
		case CMD_WALK:
		case CMD_RUN:
		case CMD_CAST:
		case CMD_FIRE:
		case CMD_THROW:
		case CMD_USE_ANY:
			/* Only check for ^ inscriptions, since we don't have an object selected (if we need one). */
			allowed = key_confirm_command(cmdkey);
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = FALSE;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch (selected) {
		case MENU_VALUE_LOOK:
			/* look at the spot */
			if (target_set_interactive(TARGET_LOOK, x, y)) {
				msg("Target Selected.");
			}
			break;

		case MENU_VALUE_RECALL: {
			/* recall monster Info */
			monster_type *m_ptr = cave_monster_at(c, y, x);
			if (m_ptr) {
				monster_lore *lore = get_lore(m_ptr->race);
				lore_show_interactive(m_ptr->race, lore);
			}
		}
			break;

		case CMD_SEARCH:
			cmd_insert(selected);
			break;

		case CMD_PATHFIND:
			cmd_insert(selected);
			cmd_set_arg_point(cmd_get_top(), 0, x, y);
			break;

		case CMD_ALTER:
		case CMD_DISARM:
		case CMD_JUMP:
		case CMD_CLOSE:
		case CMD_OPEN:
		case CMD_TUNNEL:
		case CMD_WALK:
		case CMD_RUN:
			cmd_insert(selected);
			cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
			break;

		case CMD_CAST:
			if (textui_obj_cast_ret() >= 0) {
				cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
			}
			break;

		case CMD_FIRE:
		case CMD_THROW:
		case CMD_USE_ANY:
			cmd_insert(selected);
			cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
			break;

		default:
			break;
	}

	return 1;
}

/* pick the context menu options appropiate for the item */
int context_menu_object(const object_type *o_ptr, const int slot)
{
	menu_type *m;
	region r;
	int selected;
	char *labels;
	char header[120];

	textblock *tb;
	region area = { 0, 0, 0, 0 };

	bool allowed = TRUE;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;

	m = menu_dynamic_new();
	if (!m || !o_ptr) {
		return 0;
	}
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_BASE);

	labels = string_make(lower_case);
	m->selections = labels;

	/* 'I' is used for inspect in both keymaps. */
	menu_dynamic_add_label(m, "Inspect", 'I', MENU_VALUE_INSPECT, labels);

	if (obj_can_browse(o_ptr)) {
		if (obj_can_cast_from(o_ptr) && player_can_cast(p_ptr, FALSE)) {
			ADD_LABEL("Cast", CMD_CAST, MN_ROW_VALID);
		}

		if (obj_can_study(o_ptr) && player_can_study(p_ptr, FALSE)) {
			cmd_code study_cmd = player_has(PF_CHOOSE_SPELLS) ? CMD_STUDY_SPELL : CMD_STUDY_BOOK;
			/* Hack - Use the STUDY_BOOK command key so that we get the correct command key. */
			cmdkey = cmd_lookup_key_unktrl(CMD_STUDY_BOOK, mode);
			menu_dynamic_add_label(m, "Study", cmdkey, study_cmd, labels);
		}

		if (player_can_read(p_ptr, FALSE)) {
			ADD_LABEL("Browse", CMD_BROWSE_SPELL, MN_ROW_VALID);
		}
	}
	else if (obj_is_useable(o_ptr)) {
		if (obj_is_wand(o_ptr)) {
			menu_row_validity_t valid = (obj_has_charges(o_ptr)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Aim", CMD_USE_WAND, valid);
		}
		else if (obj_is_rod(o_ptr)) {
			menu_row_validity_t valid = (obj_can_zap(o_ptr)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Zap", CMD_USE_ROD, valid);
		}
		else if (obj_is_staff(o_ptr)) {
			menu_row_validity_t valid = (obj_has_charges(o_ptr)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Use", CMD_USE_STAFF, valid);
		}
		else if (obj_is_scroll(o_ptr)) {
			menu_row_validity_t valid = (player_can_read(p_ptr, FALSE)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Read", CMD_READ_SCROLL, valid);
		}
		else if (obj_is_potion(o_ptr)) {
			ADD_LABEL("Quaff", CMD_QUAFF, MN_ROW_VALID);
		}
		else if (obj_is_food(o_ptr)) {
			ADD_LABEL("Eat", CMD_EAT, MN_ROW_VALID);
		}
		else if (obj_is_activatable(o_ptr)) {
			menu_row_validity_t valid = (slot >= INVEN_WIELD && obj_can_activate(o_ptr)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Activate", CMD_ACTIVATE, valid);
		}
		else if (obj_can_fire(o_ptr)) {
			ADD_LABEL("Fire", CMD_FIRE, MN_ROW_VALID);
		}
		else {
			ADD_LABEL("Use", CMD_USE_ANY, MN_ROW_VALID);
		}
	}

	if (obj_can_refill(o_ptr)) {
		ADD_LABEL("Refill", CMD_REFILL, MN_ROW_VALID);
	}

	if (slot >= INVEN_WIELD && obj_can_takeoff(o_ptr)) {
		ADD_LABEL("Take off", CMD_TAKEOFF, MN_ROW_VALID);
	}
	else if (slot < INVEN_WIELD && obj_can_wear(o_ptr)) {
		//if (obj_is_armor(o_ptr)) {
		//	menu_dynamic_add(m, "Wear", 2);
		//} else {
		// 	menu_dynamic_add(m, "Wield", 2);
		//}
		ADD_LABEL("Equip", CMD_WIELD, MN_ROW_VALID);
	}

	if (slot >= 0) {
		if (!store_in_store || cave_shopnum(cave, p_ptr->py, p_ptr->px) == STORE_HOME) {
			ADD_LABEL("Drop", CMD_DROP, MN_ROW_VALID);

			if (o_ptr->number > 1) {
				/* 'D' is used for squelch in rogue keymap, so we'll just swap letters. */
				cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'D' : 'k';
				menu_dynamic_add_label(m, "Drop All", cmdkey, MENU_VALUE_DROP_ALL, labels);
			}
		}
	}
	else {
		menu_row_validity_t valid = (inven_carry_okay(o_ptr)) ? MN_ROW_VALID : MN_ROW_INVALID;
		ADD_LABEL("Pick up", CMD_PICKUP, valid);
	}

	ADD_LABEL("Throw", CMD_THROW, MN_ROW_VALID);
	ADD_LABEL("Inscribe", CMD_INSCRIBE, MN_ROW_VALID);

	if (obj_has_inscrip(o_ptr)) {
		ADD_LABEL("Uninscribe", CMD_UNINSCRIBE, MN_ROW_VALID);
	}

	ADD_LABEL( (object_is_squelched(o_ptr) ? "Unignore" : "Ignore"), CMD_DESTROY, MN_ROW_VALID);

	/* work out display region */
	r.width = (int)menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	r.col = Term->wid - r.width - 1;
	r.row = 1;
	r.page_rows = m->count;

	area.width = -(r.width + 2);

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	/* Display info */
	tb = object_info(o_ptr, OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

	textui_textblock_place(tb, area, format("%s", header));
	textblock_free(tb);

	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt(format("(Enter to select, ESC) Command for %s:", header), 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	cmdkey = cmd_lookup_key(selected, mode);

	switch (selected) {
		case -1:
			/* User cancelled the menu. */
			return 3;

		case MENU_VALUE_INSPECT:
			/* copied from textui_obj_examine */
			/* Display info */
			tb = object_info(o_ptr, OINFO_NONE);
			object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

			textui_textblock_show(tb, area, format("%s", header));
			textblock_free(tb);
			return 2;

		case MENU_VALUE_DROP_ALL:
			/* Drop entire stack with confirmation. */
			if (get_check(format("Drop %s? ", header))) {
				cmd_insert(store_in_store ? CMD_STASH : CMD_DROP);
				cmd_set_arg_item(cmd_get_top(), 0, slot);
				cmd_set_arg_number(cmd_get_top(), 1, o_ptr->number);
			}
			return 1;

		case CMD_STUDY_SPELL:
			/* Hack - Use the STUDY_BOOK command key so that get_item_allow() works properly. */
			cmdkey = cmd_lookup_key(CMD_STUDY_BOOK, mode);
			/* Fall through. */
		case CMD_BROWSE_SPELL:
		case CMD_STUDY_BOOK:
		case CMD_CAST:
		case CMD_DESTROY:
		case CMD_WIELD:
		case CMD_TAKEOFF:
		case CMD_INSCRIBE:
		case CMD_UNINSCRIBE:
		case CMD_PICKUP:
		case CMD_DROP:
		case CMD_REFILL:
		case CMD_THROW:
		case CMD_USE_WAND:
		case CMD_USE_ROD:
		case CMD_USE_STAFF:
		case CMD_READ_SCROLL:
		case CMD_QUAFF:
		case CMD_EAT:
		case CMD_ACTIVATE:
		case CMD_FIRE:
		case CMD_USE_ANY:
			/* Check for inscriptions that trigger confirmation. */
			allowed = key_confirm_command(cmdkey) && get_item_allow(slot, cmdkey, selected, FALSE);
			break;
		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = FALSE;
			break;
	}

	if (!allowed)
		return 1;

	if (selected == CMD_DESTROY) {
		/* squelch or unsquelch the item */
		textui_cmd_destroy_menu(slot);
	}
	else if (selected == CMD_BROWSE_SPELL) {
		/* browse a spellbook */
		/* copied from textui_spell_browse */
		textui_book_browse(o_ptr);
		return 2;
	}
	else if (selected == CMD_STUDY_SPELL) {
		/* study a spell book */
		/* copied from textui_obj_study */
		int spell = get_spell(o_ptr, "study", spell_okay_to_study);

		if (spell >= 0) {
			cmd_insert(CMD_STUDY_SPELL);
			cmd_set_arg_choice(cmd_get_top(), 0, spell);
		}
	}
	else if (selected == CMD_CAST) {
		if (obj_can_browse(o_ptr)) {
			/* copied from textui_obj_cast */
			const char *verb = ((p_ptr->class->spell_book == TV_MAGIC_BOOK) ? "cast" : "recite");
			int spell = get_spell(o_ptr, verb, spell_okay_to_cast);

			if (spell >= 0) {
				cmd_insert(CMD_CAST);
				cmd_set_arg_choice(cmd_get_top(), 0, spell);
			}
		}
	}
	else {
		cmd_insert(selected);
		cmd_set_arg_item(cmd_get_top(), 0, slot);

		/* If we're in a store, we need to change the "drop" command to "stash". */
		if (selected == CMD_DROP && store_in_store) {
			game_command *gc = cmd_get_top();
			gc->command = CMD_STASH;
		}
	}

	return 1;
}

/* pick the context menu options appropiate for a store */
int context_menu_store(struct store *store, const int oid, int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	char *labels;
	object_type *o_ptr;

	m = menu_dynamic_new();
	if (!m || !store) {
		return 0;
	}

	/* Get the actual object */
	o_ptr = &store->stock[oid];

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Inspect Inventory", 'I', 1, labels);
	if (store->sidx == STORE_HOME) {
		/*menu_dynamic_add(m, "Stash One", 2);*/
		menu_dynamic_add_label(m, "Stash", 'd', 3, labels);
		menu_dynamic_add_label(m, "Examine", 'x', 4, labels);
		menu_dynamic_add_label(m, "Take", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Take One", 'o', 5, labels);
		}
	} else {
		/*menu_dynamic_add(m, "Sell One", 2);*/
		menu_dynamic_add_label(m, "Sell", 'd', 3, labels);
		menu_dynamic_add_label(m, "Examine", 'x', 4, labels);
		menu_dynamic_add_label(m, "Buy", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Buy One", 'o', 5, labels);
		}
	}
	menu_dynamic_add_label(m, "Exit", '`', 7, labels);


	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();
	if (selected == 1) {
		Term_keypress('I', 0);
	} else
	if (selected == 2) {
		Term_keypress('s', 0);
		/* oid is store item we do not know item we want to sell here */
		/*if (store->sidx == STORE_HOME) {
			cmd_insert(CMD_STASH);
		} else {
			cmd_insert(CMD_SELL);
		}
		cmd_set_arg_item(cmd_get_top(), 0, oid);
		cmd_set_arg_number(cmd_get_top(), 1, 1);*/
	} else
	if (selected == 3) {
		Term_keypress('s', 0);
	} else
	if (selected == 4) {
		Term_keypress('x', 0);
	} else
	if (selected == 5) {
		if (store->sidx == STORE_HOME) {
			cmd_insert(CMD_RETRIEVE);
		} else {
			cmd_insert(CMD_BUY);
		}
		cmd_set_arg_choice(cmd_get_top(), 0, oid);
		cmd_set_arg_number(cmd_get_top(), 1, 1);
	} else
	if (selected == 6) {
		Term_keypress('p', 0);
	} else
	if (selected == 7) {
		Term_keypress(ESCAPE, 0);
	}
	return 1;
}

/* pick the context menu options appropiate for an item available in a store */
int context_menu_store_item(struct store *store, const int oid, int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	char *labels;
	object_type *o_ptr;
	char header[120];

	/* Get the actual object */
	o_ptr = &store->stock[oid];


	m = menu_dynamic_new();
	if (!m || !store) {
		return 0;
	}
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_BASE);

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Examine", 'x', 4, labels);
	if (store->sidx == STORE_HOME) {
		menu_dynamic_add_label(m, "Take", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Take One", 'o', 5, labels);
		}
	} else {
		menu_dynamic_add_label(m, "Buy", 'p', 6, labels);
		if (o_ptr->number > 1) {
			menu_dynamic_add_label(m, "Buy One", 'o', 5, labels);
		}
	}

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	if (mx > Term->wid - r.width - 1) {
		r.col = Term->wid - r.width - 1;
	} else {
		r.col = mx + 1;
	}
	r.page_rows = m->count;
	if (my > Term->hgt - r.page_rows - 1) {
		if (my - r.page_rows - 1 <= 0) {
			/* menu has too many items, so put in upper right corner */
			r.row = 1;
			r.col = Term->wid - r.width - 1;
		} else {
			r.row = Term->hgt - r.page_rows - 1;
		}
	} else {
		r.row = my + 1;
	}

	/* Hack -- no flush needed */
	msg_flag = FALSE;
	screen_save();

	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt(format("(Enter to select, ESC) Command for %s:", header), 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();
	if (selected == 4) {
		Term_keypress('x', 0);
	} else
	if (selected == 5) {
		if (store->sidx == STORE_HOME) {
			cmd_insert(CMD_RETRIEVE);
		} else {
			cmd_insert(CMD_BUY);
		}
		cmd_set_arg_choice(cmd_get_top(), 0, oid);
		cmd_set_arg_number(cmd_get_top(), 1, 1);
	} else
	if (selected == 6) {
		Term_keypress('p', 0);
	}

	return 1;
}
