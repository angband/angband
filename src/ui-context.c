/**
 * \file ui-context.c
 * \brief Show player and terrain context menus.
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
#include "cmd-core.h"
#include "cmds.h"
#include "game-input.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-util.h"
#include "obj-chest.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "target.h"
#include "ui-context.h"
#include "ui-game.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-knowledge.h"
#include "ui-menu.h"
#include "ui-mon-lore.h"
#include "ui-object.h"
#include "ui-player.h"
#include "ui-spell.h"
#include "ui-store.h"
#include "ui-target.h"
#include "wizard.h"

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
	MENU_VALUE_TOGGLE_IGNORED,
	MENU_VALUE_OPTIONS,
	MENU_VALUE_HELP,
};


static int context_menu_player_2(int mx, int my)
{
	struct menu *m;
	int selected;
	char *labels;
	bool allowed = true;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Knowledge", '~', MENU_VALUE_KNOWLEDGE, labels);
	menu_dynamic_add_label(m, "Show Map", 'M', MENU_VALUE_MAP, labels);
	menu_dynamic_add_label(m, "^Show Messages", 'P', MENU_VALUE_MESSAGES,
						   labels);
	menu_dynamic_add_label(m, "Show Monster List", '[', MENU_VALUE_MONSTERS,
						   labels);
	menu_dynamic_add_label(m, "Show Object List", ']', MENU_VALUE_OBJECTS,
						   labels);

	/* Ignore toggle has different keys, but we don't have a way to look them
	 * up (see ui-game.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'K' : 'O';
	menu_dynamic_add_label(m, "Toggle Ignored", cmdkey,
						   MENU_VALUE_TOGGLE_IGNORED, labels);

	ADD_LABEL("Ignore an item", CMD_IGNORE, MN_ROW_VALID);

	menu_dynamic_add_label(m, "Options", '=', MENU_VALUE_OPTIONS, labels);
	menu_dynamic_add_label(m, "Commands", '?', MENU_VALUE_HELP, labels);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

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
		case MENU_VALUE_TOGGLE_IGNORED:
		case MENU_VALUE_HELP:
		case MENU_VALUE_MONSTERS:
		case MENU_VALUE_OBJECTS:
		case MENU_VALUE_OPTIONS:
			allowed = true;
			break;

		case CMD_IGNORE:
			cmdkey = cmd_lookup_key(selected, mode);
			allowed = key_confirm_command(cmdkey);
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = false;
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

		case CMD_IGNORE:
			cmdkey = cmd_lookup_key(selected, mode);
			Term_keypress(cmdkey, 0);
			break;

		case MENU_VALUE_TOGGLE_IGNORED:
			/* Ignore toggle has different keys, but we don't have a way to
			 * look them up (see ui-game.c). */
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
	int diff = weight_remaining(player);
	struct object *obj;

	/* There is an item on the floor, select from there */
	player->upkeep->command_wrk = (USE_FLOOR);

	/* Save screen */
	screen_save();

	/* Prompt for a command */
	prt(format("(Inventory) Burden %d.%d lb (%d.%d lb %s). Item for command: ",
			   player->upkeep->total_weight / 10,
			   player->upkeep->total_weight % 10,
			   abs(diff) / 10, abs(diff) % 10,
			   (diff < 0 ? "overweight" : "remaining")), 0, 0);


	/* Get an item to use a context command on */
	if (get_item(&obj, NULL, NULL, CMD_NULL, NULL, USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR | SHOW_EMPTY | IS_HARMLESS)) {
		/* Track the object kind */
		track_object(player->upkeep, obj);

		context_menu_object(obj);
	}

	/* Load screen */
	screen_load();
}

int context_menu_player(int mx, int my)
{
	struct menu *m;
	int selected;
	char *labels;
	bool allowed = true;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;
	struct object *obj;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	ADD_LABEL("Use", CMD_USE, MN_ROW_VALID);

	/* if player can cast, add casting option */
	if (player_can_cast(player, false)) {
		ADD_LABEL("Cast", CMD_CAST, MN_ROW_VALID);
	}

	/* if player is on stairs add option to use them */
	if (square_isupstairs(cave, player->grid)) {
		ADD_LABEL("Go Up", CMD_GO_UP, MN_ROW_VALID);
	}
	else if (square_isdownstairs(cave, player->grid)) {
		ADD_LABEL("Go Down", CMD_GO_DOWN, MN_ROW_VALID);
	}

	/* Looking has different keys, but we don't have a way to look them up
	 * (see ui-game.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'l' : 'x';
	menu_dynamic_add_label(m, "Look", cmdkey, MENU_VALUE_LOOK, labels);

	/* 'R' is used for resting in both keymaps. */
	menu_dynamic_add_label(m, "Rest", 'R', MENU_VALUE_REST, labels);

	/* 'i' is used for inventory in both keymaps. */
	menu_dynamic_add_label(m, "Inventory", 'i', MENU_VALUE_INVENTORY, labels);

	/* if object under player add pickup option */
	obj = square_object(cave, player->grid);
	if (obj && !ignore_item_ok(obj)) {
			menu_row_validity_t valid;

			/* 'f' isn't in rogue keymap, so we can use it here. */
  			menu_dynamic_add_label(m, "Floor", 'f', MENU_VALUE_FLOOR, labels);
			valid = (inven_carry_okay(obj)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Pick up", CMD_PICKUP, valid);
	}

	/* 'C' is used for the character sheet in both keymaps. */
	menu_dynamic_add_label(m, "Character", 'C', MENU_VALUE_CHARACTER, labels);

	if (!OPT(player, center_player)) {
		menu_dynamic_add_label(m, "^Center Map", 'L', MENU_VALUE_CENTER_MAP,
							   labels);
	}

	menu_dynamic_add_label(m, "Other", ' ', MENU_VALUE_OTHER, labels);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

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

		case CMD_USE:
		case CMD_CAST:
		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_PICKUP:
			/* Only check for ^ inscriptions, since we don't have an object
			 * selected (if we need one). */
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
			allowed = true;
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = false;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch(selected) {
		case CMD_USE:
		case CMD_CAST:
			cmdkey = cmd_lookup_key(selected, mode);
			Term_keypress(cmdkey, 0);
			break;

		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_PICKUP:
			cmdq_push(selected);
			break;

		case MENU_VALUE_REST:
			Term_keypress('R', 0);
			break;

		case MENU_VALUE_INVENTORY:
			Term_keypress('i', 0);
			break;

		case MENU_VALUE_LOOK:
			if (target_set_interactive(TARGET_LOOK, player->grid.x, player->grid.y))
				msg("Target Selected.");
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

int context_menu_cave(struct chunk *c, int y, int x, int adjacent, int mx,
					  int my)
{
	struct menu *m;
	int selected;
	char *labels;
	bool allowed = true;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;
	struct loc grid = loc(x, y);
	struct object *square_obj = square_object(c, grid);

	m = menu_dynamic_new();
	if (!m)
		return 0;

	labels = string_make(lower_case);
	m->selections = labels;

	/* Looking has different keys, but we don't have a way to look them up
	 * (see ui-game.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'l' : 'x';
	menu_dynamic_add_label(m, "Look At", cmdkey, MENU_VALUE_LOOK, labels);

	if (square(c, grid).mon)
		/* '/' is used for recall in both keymaps. */
		menu_dynamic_add_label(m, "Recall Info", '/', MENU_VALUE_RECALL,
							   labels);

	ADD_LABEL("Use Item On", CMD_USE, MN_ROW_VALID);

	if (player_can_cast(player, false))
		ADD_LABEL("Cast On", CMD_CAST, MN_ROW_VALID);

	if (adjacent) {
		struct object *obj = chest_check(grid, CHEST_ANY);
		ADD_LABEL((square(c, grid).mon) ? "Attack" : "Alter", CMD_ALTER,
				  MN_ROW_VALID);

		if (obj && !ignore_item_ok(obj)) {
			if (obj->known->pval) {
				if (is_locked_chest(obj)) {
					ADD_LABEL("Disarm Chest", CMD_DISARM, MN_ROW_VALID);
					ADD_LABEL("Open Chest", CMD_OPEN, MN_ROW_VALID);
				} else {
					ADD_LABEL("Open Disarmed Chest", CMD_OPEN, MN_ROW_VALID);
				}
			} else {
				ADD_LABEL("Open Chest", CMD_OPEN, MN_ROW_VALID);
			}
		}

		if ((square(cave, grid).mon > 0) && player_has(player, PF_STEAL)) {
			ADD_LABEL("Steal", CMD_STEAL, MN_ROW_VALID);
		}

		if (square_isdisarmabletrap(c, grid)) {
			ADD_LABEL("Disarm", CMD_DISARM, MN_ROW_VALID);
			ADD_LABEL("Jump Onto", CMD_JUMP, MN_ROW_VALID);
		}

		if (square_isopendoor(c, grid)) {
			ADD_LABEL("Close", CMD_CLOSE, MN_ROW_VALID);
		}
		else if (square_iscloseddoor(c, grid)) {
			ADD_LABEL("Open", CMD_OPEN, MN_ROW_VALID);
			ADD_LABEL("Lock", CMD_DISARM, MN_ROW_VALID);
		}
		else if (square_isdiggable(c, grid)) {
			ADD_LABEL("Tunnel", CMD_TUNNEL, MN_ROW_VALID);
		}

		ADD_LABEL("Walk Towards", CMD_WALK, MN_ROW_VALID);
	} else {
		/* ',' is used for ignore in rogue keymap, so we'll just swap letters */
		cmdkey = (mode == KEYMAP_MODE_ORIG) ? ',' : '.';
		menu_dynamic_add_label(m, "Pathfind To", cmdkey, CMD_PATHFIND, labels);

		ADD_LABEL("Walk Towards", CMD_WALK, MN_ROW_VALID);
		ADD_LABEL("Run Towards", CMD_RUN, MN_ROW_VALID);
	}

	if (player_can_fire(player, false)) {
		ADD_LABEL("Fire On", CMD_FIRE, MN_ROW_VALID);
	}

	ADD_LABEL("Throw To", CMD_THROW, MN_ROW_VALID);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

	if (player->timed[TMD_IMAGE]) {
		prt("(Enter to select command, ESC to cancel) You see something strange:", 0, 0);
	} else if (square(c, grid).mon) {
		char m_name[80];
		struct monster *mon = square_monster(c, grid);

		/* Get the monster name ("a kobold") */
		monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_VIS);

		prt(format("(Enter to select command, ESC to cancel) You see %s:",
				   m_name), 0, 0);
	} else if (square_obj && !ignore_item_ok(square_obj)) {
		char o_name[80];

		/* Obtain an object description */
		object_desc(o_name, sizeof (o_name), square_obj,
					ODESC_PREFIX | ODESC_FULL);

		prt(format("(Enter to select command, ESC to cancel) You see %s:",
				   o_name), 0, 0);
	} else {
		/* Feature (apply mimic) */
		const char *name = square_apparent_name(c, player, grid);

		/* Hack -- special introduction for store doors */
		if (square_isshop(cave, grid)) {
			prt(format("(Enter to select command, ESC to cancel) You see the entrance to the %s:", name), 0, 0);
		} else {
			prt(format("(Enter to select command, ESC to cancel) You see %s %s:", (is_a_vowel(name[0])) ? "an" : "a", name), 0, 0);
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
			allowed = true;
			break;

		case CMD_ALTER:
		case CMD_STEAL:
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
		case CMD_USE:
			/* Only check for ^ inscriptions, since we don't have an object
			 * selected (if we need one). */
			allowed = key_confirm_command(cmdkey);
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = false;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch (selected) {
		case MENU_VALUE_LOOK:
			/* Look at the spot */
			if (target_set_interactive(TARGET_LOOK, x, y)) {
				msg("Target Selected.");
			}
			break;

		case MENU_VALUE_RECALL: {
			/* Recall monster Info */
			struct monster *mon = square_monster(c, grid);
			if (mon) {
				struct monster_lore *lore = get_lore(mon->race);
				lore_show_interactive(mon->race, lore);
			}
		}
			break;

		case CMD_PATHFIND:
			cmdq_push(selected);
			cmd_set_arg_point(cmdq_peek(), "point", loc(x, y));
			break;

		case CMD_ALTER:
		case CMD_STEAL:
		case CMD_DISARM:
		case CMD_JUMP:
		case CMD_CLOSE:
		case CMD_OPEN:
		case CMD_TUNNEL:
		case CMD_WALK:
		case CMD_RUN:
			cmdq_push(selected);
			cmd_set_arg_direction(cmdq_peek(), "direction",
								  motion_dir(player->grid, loc(x, y)));
			break;

		case CMD_CAST:
		case CMD_FIRE:
		case CMD_THROW:
		case CMD_USE:
			cmdq_push(selected);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
			break;

		default:
			break;
	}

	return 1;
}

/**
 * Pick the context menu options appropiate for the item
 */
int context_menu_object(struct object *obj)
{
	struct menu *m;
	region r;
	int selected;
	char *labels;
	char header[120];

	textblock *tb;
	region area = { 0, 0, 0, 0 };

	bool allowed = true;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;

	m = menu_dynamic_new();
	if (!m || !obj)
		return 0;

	object_desc(header, sizeof(header), obj, ODESC_PREFIX | ODESC_BASE);

	labels = string_make(lower_case);
	m->selections = labels;

	/* 'I' is used for inspect in both keymaps. */
	menu_dynamic_add_label(m, "Inspect", 'I', MENU_VALUE_INSPECT, labels);

	if (obj_can_browse(obj)) {
		if (obj_can_cast_from(obj) && player_can_cast(player, false))
			ADD_LABEL("Cast", CMD_CAST, MN_ROW_VALID);

		if (obj_can_study(obj) && player_can_study(player, false))
			ADD_LABEL("Study", CMD_STUDY, MN_ROW_VALID);

		if (player_can_read(player, false))
			ADD_LABEL("Browse", CMD_BROWSE_SPELL, MN_ROW_VALID);
	} else if (obj_is_useable(obj)) {
		if (tval_is_wand(obj)) {
			menu_row_validity_t valid = (obj_has_charges(obj)) ?
				MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Aim", CMD_USE_WAND, valid);
		} else if (tval_is_rod(obj)) {
			menu_row_validity_t valid = (obj_can_zap(obj)) ?
				MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Zap", CMD_USE_ROD, valid);
		} else if (tval_is_staff(obj)) {
			menu_row_validity_t valid = (obj_has_charges(obj)) ?
				MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Use", CMD_USE_STAFF, valid);
		} else if (tval_is_scroll(obj)) {
			menu_row_validity_t valid = (player_can_read(player, false)) ?
				MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Read", CMD_READ_SCROLL, valid);
		} else if (tval_is_potion(obj)) {
			ADD_LABEL("Quaff", CMD_QUAFF, MN_ROW_VALID);
		} else if (tval_is_edible(obj)) {
			ADD_LABEL("Eat", CMD_EAT, MN_ROW_VALID);
		} else if (obj_is_activatable(obj)) {
			menu_row_validity_t valid = (object_is_equipped(player->body, obj)
										 && obj_can_activate(obj)) ?
				MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Activate", CMD_ACTIVATE, valid);
		} else if (obj_can_fire(obj)) {
			ADD_LABEL("Fire", CMD_FIRE, MN_ROW_VALID);
		} else {
			ADD_LABEL("Use", CMD_USE, MN_ROW_VALID);
		}
	}

	if (obj_can_refill(obj))
		ADD_LABEL("Refill", CMD_REFILL, MN_ROW_VALID);

	if (object_is_equipped(player->body, obj) && obj_can_takeoff(obj)) {
		ADD_LABEL("Take off", CMD_TAKEOFF, MN_ROW_VALID);
	} else if (!object_is_equipped(player->body, obj) && obj_can_wear(obj)) {
		ADD_LABEL("Equip", CMD_WIELD, MN_ROW_VALID);
	}

	if (object_is_carried(player, obj)) {
		if (!square_isshop(cave, player->grid)) {
			ADD_LABEL("Drop", CMD_DROP, MN_ROW_VALID);

			if (obj->number > 1) {
				/* 'D' is used for ignore in rogue keymap, so swap letters. */
				cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'D' : 'k';
				menu_dynamic_add_label(m, "Drop All", cmdkey,
									   MENU_VALUE_DROP_ALL, labels);
			}
		} else if (square_shopnum(cave, player->grid) == STORE_HOME) {
			ADD_LABEL("Drop", CMD_DROP, MN_ROW_VALID);

			if (obj->number > 1) {
				/* 'D' is used for ignore in rogue keymap, so swap letters. */
				cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'D' : 'k';
				menu_dynamic_add_label(m, "Drop All", cmdkey,
									   MENU_VALUE_DROP_ALL, labels);
			}
		} else if (store_will_buy_tester(obj)) {
			ADD_LABEL("Sell", CMD_DROP, MN_ROW_VALID);
		}
	} else {
		menu_row_validity_t valid = (inven_carry_okay(obj)) ?
			MN_ROW_VALID : MN_ROW_INVALID;
		ADD_LABEL("Pick up", CMD_PICKUP, valid);
	}

	ADD_LABEL("Throw", CMD_THROW, MN_ROW_VALID);
	ADD_LABEL("Inscribe", CMD_INSCRIBE, MN_ROW_VALID);

	if (obj_has_inscrip(obj))
		ADD_LABEL("Uninscribe", CMD_UNINSCRIBE, MN_ROW_VALID);

	ADD_LABEL( (object_is_ignored(obj) ? "Unignore" : "Ignore"), CMD_IGNORE,
			   MN_ROW_VALID);

	/* work out display region */
	r.width = (int)menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag,
														   * 2 for pad */
	r.col = Term->wid - r.width - 1;
	r.row = 1;
	r.page_rows = m->count;

	area.width = -(r.width + 2);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	/* Display info */
	tb = object_info(obj, OINFO_NONE);
	object_desc(header, sizeof(header), obj, ODESC_PREFIX | ODESC_FULL);

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
			tb = object_info(obj, OINFO_NONE);
			object_desc(header, sizeof(header), obj, ODESC_PREFIX | ODESC_FULL);

			textui_textblock_show(tb, area, format("%s", header));
			textblock_free(tb);
			return 2;

		case MENU_VALUE_DROP_ALL:
			/* Drop entire stack without confirmation. */
			if (square_isshop(cave, player->grid))
				cmdq_push(CMD_STASH);
			else
				cmdq_push(CMD_DROP);
			cmd_set_arg_item(cmdq_peek(), "item", obj);
			cmd_set_arg_number(cmdq_peek(), "quantity", obj->number);
			return 1;

		case CMD_BROWSE_SPELL:
		case CMD_STUDY:
		case CMD_CAST:
		case CMD_IGNORE:
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
		case CMD_USE:
			/* Check for inscriptions that trigger confirmation. */
			allowed = key_confirm_command(cmdkey) &&
				get_item_allow(obj, cmdkey, selected, false);
			break;
		default:
			/* Invalid command; prevent anything from happening. */
			bell("Invalid context menu command.");
			allowed = false;
			break;
	}

	if (!allowed)
		return 1;

	if (selected == CMD_IGNORE) {
		/* ignore or unignore the item */
		textui_cmd_ignore_menu(obj);
	} else if (selected == CMD_BROWSE_SPELL) {
		/* browse a spellbook */
		/* copied from textui_spell_browse */
		textui_book_browse(obj);
		return 2;
	} else if (selected == CMD_STUDY) {
		cmdq_push(CMD_STUDY);
		cmd_set_arg_item(cmdq_peek(), "item", obj);
	} else if (selected == CMD_CAST) {
		if (obj_can_cast_from(obj)) {
			cmdq_push(CMD_CAST);
			cmd_set_arg_item(cmdq_peek(), "book", obj);
		}
	} else {
		cmdq_push(selected);
		cmd_set_arg_item(cmdq_peek(), "item", obj);

		/* If we're in a store, change the "drop" command to "stash". */
		if (selected == CMD_DROP &&
			square_isshop(cave, player->grid)) {
			struct command *gc = cmdq_peek();
			if (square_shopnum(cave, player->grid) == STORE_HOME)
				gc->code = CMD_STASH;
			else
				gc->code = CMD_SELL;
		}
	}

	return 1;
}



static int show_command_list(struct cmd_info cmd_list[], int size, int mx,
                             int my)
{
	struct menu *m;
	int selected;
	int i;
	char cmd_name[80];
	char key[3];

	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}
	m->selections = lower_case;
	key[2] = '\0';

	for (i = 0; i < size; ++i) {
		if (KTRL(cmd_list[i].key[mode]) == cmd_list[i].key[mode]) {
			key[0] = '^';
			key[1] = UN_KTRL(cmd_list[i].key[mode]);
		} else {
			key[0] = cmd_list[i].key[mode];
			key[1] = '\0';
		}
		strnfmt(cmd_name, 80, "%s (%s)",  cmd_list[i].desc, key);
		menu_dynamic_add(m, cmd_name, i+1);
	}

	menu_dynamic_calc_location(m, mx, my);

	screen_save();
	region_erase_bordered(&m->boundary);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

	screen_load();

	if ((selected > 0) && (selected < size+1)) {
		/* execute the command */
		Term_keypress(cmd_list[selected-1].key[mode], 0);
	}

	return 1;
}

int context_menu_command(int mx, int my)
{
	struct menu *m;
	int selected;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	m->selections = lower_case;
	menu_dynamic_add(m, "Item", 1);
	menu_dynamic_add(m, "Action", 2);
	menu_dynamic_add(m, "Item Management", 3);
	menu_dynamic_add(m, "Info", 4);
	menu_dynamic_add(m, "Util", 5);
	menu_dynamic_add(m, "Misc", 6);

	menu_dynamic_calc_location(m, mx, my);

	screen_save();
	region_erase_bordered(&m->boundary);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

	screen_load();

	/* XXX-AS this is gross, as is the way there's two ways to display the
	 * entire command list.  Fix me */
	if (selected > 0) {
		selected--;
		show_command_list(cmds_all[selected].list, cmds_all[selected].len,
						  mx, my);
	} else {
		return 0;
	}

	return 1;
}

/**
 * Handle a textui mouseclick.
 */
void textui_process_click(ui_event e)
{
	int x, y;

	if (!OPT(player, mouse_movement)) return;

	y = KEY_GRID_Y(e);
	x = KEY_GRID_X(e);

	/* Check for a valid location */
	if (!square_in_bounds_fully(cave, loc(x, y))) return;

	/* XXX show context menu here */
	if (loc_eq(player->grid, loc(x, y))) {
		if (e.mouse.mods & KC_MOD_SHIFT) {
			/* shift-click - cast magic */
			if (e.mouse.button == 1) {
				cmdq_push(CMD_CAST);
			} else if (e.mouse.button == 2) {
				Term_keypress('i',0);
			}
		} else if (e.mouse.mods & KC_MOD_CONTROL) {
			/* ctrl-click - use feature / use inventory item */
			/* switch with default */
			if (e.mouse.button == 1) {
				if (square_isupstairs(cave, player->grid))
					cmdq_push(CMD_GO_UP);
				else if (square_isdownstairs(cave, player->grid))
					cmdq_push(CMD_GO_DOWN);
			} else if (e.mouse.button == 2) {
				cmdq_push(CMD_USE);
			}
		} else if (e.mouse.mods & KC_MOD_ALT) {
			/* alt-click - show char screen */
			/* XXX call a platform specific hook */
			if (e.mouse.button == 1) {
				Term_keypress('C',0);
			}
		} else {
			if (e.mouse.button == 1) {
				if (square_object(cave, loc(x, y))) {
					cmdq_push(CMD_PICKUP);
				} else {
					cmdq_push(CMD_HOLD);
				}
			} else if (e.mouse.button == 2) {
				/* Show a context menu */
				context_menu_player(e.mouse.x, e.mouse.y);
			}
		}
	} else if (e.mouse.button == 1) {
		if (player->timed[TMD_CONFUSED]) {
			cmdq_push(CMD_WALK);
		} else {
			if (e.mouse.mods & KC_MOD_SHIFT) {
				/* shift-click - run */
				cmdq_push(CMD_RUN);
				cmd_set_arg_direction(cmdq_peek(), "direction",
									  motion_dir(player->grid, loc(x, y)));
			} else if (e.mouse.mods & KC_MOD_CONTROL) {
				/* control-click - alter */
				cmdq_push(CMD_ALTER);
				cmd_set_arg_direction(cmdq_peek(), "direction",
									  motion_dir(player->grid, loc(x, y)));
			} else if (e.mouse.mods & KC_MOD_ALT) {
				/* alt-click - look */
				if (target_set_interactive(TARGET_LOOK, x, y)) {
					msg("Target Selected.");
				}
			} else {
				/* Pathfind does not work well on trap detection borders,
				 * so if the click is next to the player, force a walk step */
				if ((y - player->grid.y >= -1) && (y - player->grid.y <= 1)	&&
					(x - player->grid.x >= -1) && (x - player->grid.x <= 1)) {
					cmdq_push(CMD_WALK);
					cmd_set_arg_direction(cmdq_peek(), "direction",
										  motion_dir(player->grid, loc(x, y)));
				} else {
					cmdq_push(CMD_PATHFIND);
					cmd_set_arg_point(cmdq_peek(), "point", loc(x, y));
				}
			}
		}
	} else if (e.mouse.button == 2) {
		struct monster *m = square_monster(cave, loc(x, y));
		if (m && target_able(m)) {
			/* Set up target information */
			monster_race_track(player->upkeep, m->race);
			health_track(player->upkeep, m);
			target_set_monster(m);
		} else {
			target_set_location(y, x);
		}

		if (e.mouse.mods & KC_MOD_SHIFT) {
			/* shift-click - cast spell at target */
			cmdq_push(CMD_CAST);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
		} else if (e.mouse.mods & KC_MOD_CONTROL) {
			/* control-click - fire at target */
			cmdq_push(CMD_USE);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
		} else if (e.mouse.mods & KC_MOD_ALT) {
			/* alt-click - throw at target */
			cmdq_push(CMD_THROW);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
		} else {
			/* see if the click was adjacent to the player */
			if ((y - player->grid.y >= -1) && (y - player->grid.y <= 1)	&&
				(x - player->grid.x >= -1) && (x - player->grid.x <= 1)) {
				context_menu_cave(cave,y,x,1,e.mouse.x, e.mouse.y);
			} else {
				context_menu_cave(cave,y,x,0,e.mouse.x, e.mouse.y);
			}
		}
	}
}




/**
 * ------------------------------------------------------------------------
 * Menu functions
 * ------------------------------------------------------------------------ */

/**
 * Display an entry on a command menu
 */
static void cmd_sub_entry(struct menu *menu, int oid, bool cursor, int row,
						  int col, int width)
{
	byte attr = (cursor ? COLOUR_L_BLUE : COLOUR_WHITE);
	const struct cmd_info *commands = menu_priv(menu);

	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	struct keypress kp = { EVT_KBRD, commands[oid].key[mode], 0 };
	char buf[16];

	/* Write the description */
	Term_putstr(col, row, -1, attr, commands[oid].desc);

	/* Include keypress */
	Term_addch(attr, ' ');
	Term_addch(attr, '(');

	/* Get readable version */
	keypress_to_readable(buf, sizeof buf, kp);
	Term_addstr(-1, attr, buf);

	Term_addch(attr, ')');
}

/**
 * Display a list of commands.
 */
static bool cmd_menu(struct command_list *list, void *selection_p)
{
	struct menu menu;
	menu_iter commands_menu = { NULL, NULL, cmd_sub_entry, NULL, NULL };
	region area = { 23, 4, 37, 13 };

	ui_event evt;
	struct cmd_info **selection = selection_p;

	/* Set up the menu */
	menu_init(&menu, MN_SKIN_SCROLL, &commands_menu);
	menu_setpriv(&menu, list->len, list->list);
	menu_layout(&menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(21, 3, 62, 17);

	/* Select an entry */
	evt = menu_select(&menu, 0, true);

	/* Load de screen */
	screen_load();

	if (evt.type == EVT_SELECT)
		*selection = &list->list[menu.cursor];

	return false;
}



static bool cmd_list_action(struct menu *m, const ui_event *event, int oid)
{
	if (event->type == EVT_SELECT)
		return cmd_menu(&cmds_all[oid], menu_priv(m));
	else
		return false;
}

static void cmd_list_entry(struct menu *menu, int oid, bool cursor, int row,
						   int col, int width)
{
	byte attr = (cursor ? COLOUR_L_BLUE : COLOUR_WHITE);
	Term_putstr(col, row, -1, attr, cmds_all[oid].name);
}

static struct menu *command_menu;
static menu_iter command_menu_iter =
{
	NULL,
	NULL,
	cmd_list_entry,
	cmd_list_action,
	NULL
};

/**
 * Display a list of command types, allowing the user to select one.
 */
struct cmd_info *textui_action_menu_choose(void)
{
	region area = { 21, 5, 37, 6 };
	int len = 0;

	struct cmd_info *chosen_command = NULL;

	if (!command_menu)
		command_menu = menu_new(MN_SKIN_SCROLL, &command_menu_iter);

	while (cmds_all[len].len) {
		if (cmds_all[len].len)
			len++;
	};

	menu_setpriv(command_menu, len, &chosen_command);
	menu_layout(command_menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(19, 4, 58, 11);

	menu_select(command_menu, 0, true);

	screen_load();

	return chosen_command;
}
