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
#include "object/tvalsval.h"
#include "object/object.h"

int context_menu_command();
int context_menu_object(const object_type *o_ptr, const int slot);

int context_menu_player_2(int mx, int my)
{
	menu_type *m;
	region r;
	int selected;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	m->selections = lower_case;
	menu_dynamic_add(m, "Knowledge", 1);
	menu_dynamic_add(m, "Show Map", 2);
	menu_dynamic_add(m, "Show Messages", 3);
	menu_dynamic_add(m, "Toggle Searching", 4);
	menu_dynamic_add(m, "Toggle Squelched", 5);
	menu_dynamic_add(m, "Squelch an item", 6);
	menu_dynamic_add(m, "Options", 8);
	menu_dynamic_add(m, "Commands", 7);

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

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

	screen_load();

	if (selected == 1) {
		/* show knowledge screen */
		Term_keypress('~',0);
	} else
	if (selected == 2) {
		/* Toggle show map */
		Term_keypress('M',0);
	} else
	if (selected == 3) {
		/* Toggle show messages */
		Term_keypress(KTRL('p'),0);/*XXX should be ('p', KC_MOD_CONTROL);*/
	} else
	if (selected == 4) {
		/* Toggle search mode */
		Term_keypress('S',0);
	} else
	if (selected == 5) {
		/* toggle showing squelched objects */
		Term_keypress('K',0);
	} else
	if (selected == 6) {
		/* destroy/squelch an item */
		Term_keypress('k',0);
	} else
	if (selected == 8) {
		/* show options screen */
		Term_keypress('=',0);
	} else
	if (selected == 7) {
		/* show the commands */
		context_menu_command();
	}

	return 1;
}

int context_menu_player(int mx, int my)
{
	menu_type *m;
	region r;
	int selected;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	m->selections = lower_case;
	menu_dynamic_add(m, "Use", 1);
	/* if player can cast, add casting option */
	if (player_can_cast()) {
		menu_dynamic_add(m, "Cast", 2);
	}
	/* if player is on stairs add option to use them */
	if (cave->feat[p_ptr->py][p_ptr->px] == FEAT_LESS) {
		menu_dynamic_add(m, "Go Up", 11);
	} else
	if (cave->feat[p_ptr->py][p_ptr->px] == FEAT_MORE) {
		menu_dynamic_add(m, "Go Down", 12);
	}
	menu_dynamic_add(m, "Search", 3);
	menu_dynamic_add(m, "Look", 6);
	menu_dynamic_add(m, "Rest", 4);
	menu_dynamic_add(m, "Inventory", 5);
	/* if object under player add pickup option */
	if (cave->o_idx[p_ptr->py][p_ptr->px]) {
		object_type *o_ptr;
  		menu_dynamic_add(m, "Floor", 13);
		o_ptr = object_from_item_idx(-1);
		if (!o_ptr->next_o_idx) {
  			menu_dynamic_add(m, "Pickup", 14);
		}
	}
	menu_dynamic_add(m, "Character", 7);
	menu_dynamic_add(m, "Keymaps", 10);
	menu_dynamic_add(m, "Other", 9);

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

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

	screen_load();

	switch(selected) {
	case 1:
		{
			/* use an item */
			Term_keypress('U',0);
		} break;
	case 2:
		{
			/* Cast a spell */
			Term_keypress('m',0);
		} break;
	case 3:
		{
			/* search */
			Term_keypress('s',0);
		} break;
	case 4:
		{
			/* rest */
			Term_keypress('R',0);
		} break;
	case 5:
		{
			/* show inventory screen */
			Term_keypress('i',0);
		} break;
	case 6:
		{
			/* look mode */
			if (target_set_interactive(TARGET_LOOK, p_ptr->px, p_ptr->py)) {
				msg("Target Selected.");
			}
		} break;
	case 7:
		{
			/* show character screen */
			Term_keypress('C',0);
		} break;
	case 9:
		{
			/* show another layer of menu options screen */
			context_menu_player_2(mx,my);
		} break;
	case 10:
		{
			/* show the commands */
			context_menu_command();
		} break;
	case 11:
		{
			/* go up stairs */
			cmd_insert(CMD_GO_UP);
		} break;
	case 12:
		{
			/* go down stairs */
			cmd_insert(CMD_GO_DOWN);
		} break;
	case 13:
		{
			/* there is an item on the floor, show the inventory screen starting
			 * from the floor */
			//Term_keypress('i',0);
			int diff = weight_remaining();

			/* Hack -- Start in "inventory" mode */
			p_ptr->command_wrk = (USE_FLOOR);

			/* Save screen */
			screen_save();

			/* Hack -- show empty slots */
			item_tester_full = TRUE;

			/* Prompt for a command */
			prt(format("(Inventory) Burden %d.%d lb (%d.%d lb %s). Item for command: ",
				p_ptr->total_weight / 10, p_ptr->total_weight % 10,
				abs(diff) / 10, abs(diff) % 10,
				(diff < 0 ? "overweight" : "remaining")),
				0, 0);


			/* Get an item to use a context command on */
			if (get_item(&diff, NULL, NULL, CMD_NULL, USE_EQUIP|USE_INVEN|USE_FLOOR|IS_HARMLESS)) {
				object_type *o_ptr;

				/* Track the object kind */
				track_object(diff);

				o_ptr = object_from_item_idx(diff);

				context_menu_object(o_ptr, diff);
			}

			/* Hack -- hide empty slots */
			item_tester_full = FALSE;

			/* Load screen */
			screen_load();


		} break;
	case 14:
		{
			/* pick the item up */
			cmd_insert(CMD_PICKUP);
			cmd_set_arg_item(cmd_get_top(), 0, -1);
		} break;

	}

	return 1;
}

int context_menu_cave(struct cave *cave, int y, int x, int adjacent, int mx, int my)
{
	menu_type *m;
	region r;
	int selected;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	m->selections = lower_case;
	menu_dynamic_add(m, "Look At", 1);
	menu_dynamic_add(m, "Use Item On", 2);
	if (player_can_cast()) {
		menu_dynamic_add(m, "Cast On", 3);
	}
	if (adjacent) {
		menu_dynamic_add(m, "Attack", 4);
		if (cave_istrap(cave, y, x)) {
			menu_dynamic_add(m, "Disarm", 5);
			menu_dynamic_add(m, "Jump Onto", 6);
		}
		if (cave_isopendoor(cave, y, x)) {
			menu_dynamic_add(m, "Close", 7);
		} else
		if (cave_iscloseddoor(cave, y, x)) {
			menu_dynamic_add(m, "Open", 8);
			menu_dynamic_add(m, "Bash", 9);
			menu_dynamic_add(m, "Jam", 10);
		} else
		if (cave_isdiggable(cave, y, x)) {
			menu_dynamic_add(m, "Tunnel", 11);
		}
		menu_dynamic_add(m, "Search", 12);
		menu_dynamic_add(m, "Walk Towards", 14);
	} else {
		menu_dynamic_add(m, "Pathfind To", 13);
		menu_dynamic_add(m, "Walk Towards", 14);
		menu_dynamic_add(m, "Run Towards", 15);
	}
	if (player_can_fire()) {
		menu_dynamic_add(m, "Fire On", 16);
	}
	menu_dynamic_add(m, "Throw To", 17);

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

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

	screen_load();

	if (selected == 1) {
		/* look at the spot */
		if (target_set_interactive(TARGET_LOOK, x, y)) {
			msg("Target Selected.");
		}
	} else
	if (selected == 2) {
		/* use an item on the spot */
		cmd_insert(CMD_USE_AIMED);
		cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
	} else
	if (selected == 3) {
		/* cast a spell on the spot */
		if (textui_obj_cast_ret() >= 0) {
			cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
		}
	} else
	if (selected == 4) {
		/* attack a spot adjacent to the player */
		cmd_insert(CMD_ALTER);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 5) {
		/* disarm an adjacent trap or chest */
		cmd_insert(CMD_DISARM);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 6) {
		/* walk onto an adjacent spot even if there is a trap there */
		cmd_insert(CMD_JUMP);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 7) {
		/* close a door */
		cmd_insert(CMD_CLOSE);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 8) {
		/* open a door or chest */
		cmd_insert(CMD_OPEN);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 9) {
		/* bash a door */
		cmd_insert(CMD_BASH);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 10) {
		/* jam a door */
		cmd_insert(CMD_JAM);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 11) {
		/* Tunnel in a direction */
		cmd_insert(CMD_TUNNEL);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 12) {
		/* Search */
		cmd_insert(CMD_SEARCH);
	} else
	if (selected == 13) {
		/* pathfind to the spot */
		cmd_insert(CMD_PATHFIND);
		cmd_set_arg_point(cmd_get_top(), 0, x, y);
	} else
	if (selected == 14) {
		/* walk towards the spot */
		cmd_insert(CMD_WALK);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 15) {
		/* run towards the spot */
		cmd_insert(CMD_RUN);
		cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
	} else
	if (selected == 16) {
		/* Fire ammo towards the spot */
		cmd_insert(CMD_FIRE);
		cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
	} else
	if (selected == 17) {
		/* throw an item towards the spot */
		cmd_insert(CMD_THROW);
 		cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
	}

	return 1;
}

/* pick the context menu options appropiate for the item */
int context_menu_object(const object_type *o_ptr, const int slot)
{
	menu_type *m;
	region r;
	int selected;

	m = menu_dynamic_new();
	if (!m || !o_ptr) {
		return 0;
	}

	m->selections = lower_case;
	menu_dynamic_add(m, "Inspect", 1);

	if (obj_can_browse(o_ptr)) {
		if (obj_can_cast_from(o_ptr) && player_can_cast()) {
			menu_dynamic_add(m, "Cast", 8);
		}
		if (obj_can_study(o_ptr) && player_can_study()) {
			menu_dynamic_add(m, "Study", 10);
		}
		if (player_can_read()) {
			menu_dynamic_add(m, "Browse", 9);
		}
	} else
	if (obj_is_useable(o_ptr)) {
		if (obj_is_wand(o_ptr)) {
			if (obj_has_charges(o_ptr)) {
				menu_dynamic_add(m, "Aim", 8);
			} else {
				menu_dynamic_add(m, "Aim (grey)", 8);
			}
		} else
		if (obj_is_rod(o_ptr)) {
			if (obj_can_zap(o_ptr)) {
				menu_dynamic_add(m, "Zap", 8);
			} else {
				menu_dynamic_add(m, "Zap (grey)", 8);
			}
		} else
		if (obj_is_staff(o_ptr)) {
			if (obj_has_charges(o_ptr)) {
				menu_dynamic_add(m, "Use", 8);
			} else {
				menu_dynamic_add(m, "Use (grey)", 8);
			}
		} else
		if (obj_is_scroll(o_ptr)) {
			if (player_can_read()) {
				menu_dynamic_add(m, "Read", 8);
			} else {
				menu_dynamic_add(m, "Read (grey)", 8);
			}
		} else
		if (obj_is_potion(o_ptr)) {
			menu_dynamic_add(m, "Quaff", 8);
		} else
		if (obj_is_food(o_ptr)) {
			menu_dynamic_add(m, "Eat", 8);
		} else
		if (obj_is_activatable(o_ptr)) {
			menu_dynamic_add(m, "Activate", 8);
		} else
		if (obj_can_fire(o_ptr)) {
			menu_dynamic_add(m, "Fire", 8);
		} else
		{
			menu_dynamic_add(m, "Use", 8);
		}
	}
	if (obj_can_refill(o_ptr)) {
		menu_dynamic_add(m, "Refill", 11);
	}
	if ((slot > INVEN_WIELD) && obj_can_takeoff(o_ptr)) {
		menu_dynamic_add(m, "Take off", 3);
	} else
	if ((slot <= INVEN_WIELD) && obj_can_wear(o_ptr)) {
		//if (obj_is_armor(o_ptr)) {
		//	menu_dynamic_add(m, "Wear", 2);
		//} else {
		// 	menu_dynamic_add(m, "Wield", 2);
		//}
		menu_dynamic_add(m, "Equip", 2);
	}
	if (slot >= 0) {
		menu_dynamic_add(m, "Drop", 6);
	} else
	{
		menu_dynamic_add(m, "Pickup", 7);
	}
	if (obj_has_inscrip(o_ptr)) {
		menu_dynamic_add(m, "Uninscribe", 5);
	} else
	{
		menu_dynamic_add(m, "Inscribe", 4);
	}
	menu_dynamic_add(m, "Throw", 12);

	/* work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	r.col = Term->wid - r.width - 1;
	r.row = 1;
	r.page_rows = m->count;

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

	screen_load();
	if (selected == 1) {
		/* copied from textui_obj_examine */
		char header[120];

		textblock *tb;
		region area = { 0, 0, 0, 0 };

		/* Display info */
		tb = object_info(o_ptr, OINFO_NONE);
		object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

		textui_textblock_show(tb, area, format("%s", header));
		textblock_free(tb);
	} else
	if (selected == 2) {
		/* wield the item */
 		cmd_insert(CMD_WIELD);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 3) {
		/* take the item off */
 		cmd_insert(CMD_TAKEOFF);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 4) {
		/* inscribe the item */
 		cmd_insert(CMD_INSCRIBE);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 5) {
		/* uninscribe the item */
 		cmd_insert(CMD_UNINSCRIBE);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 6) {
		/* drop the item */
 		cmd_insert(CMD_DROP);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 7) {
		/* pick the item up */
 		cmd_insert(CMD_PICKUP);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 8) {
		/* use the item */
		if (obj_can_browse(o_ptr)) {
			/* copied from textui_obj_cast */
			int spell;
			const char *verb = ((p_ptr->class->spell_book == TV_MAGIC_BOOK) ? "cast" : "recite");
			/* Ask for a spell */
			spell = get_spell(o_ptr, verb, spell_okay_to_cast);
			if (spell >= 0) {
				cmd_insert(CMD_CAST);
				cmd_set_arg_choice(cmd_get_top(), 0, spell);
			}
		} else {
			cmd_insert(CMD_USE_ANY);
			cmd_set_arg_item(cmd_get_top(), 0, slot);
		}
	} else
	if (selected == 9) {
		/* browse a spellbook */
		/* copied from textui_spell_browse */
		textui_book_browse(o_ptr);
	} else
	if (selected == 10) {
		/* study a spell book */
		/* copied from textui_obj_study */
		if (player_has(PF_CHOOSE_SPELLS)) {
			int spell = get_spell(o_ptr,
				  "study", spell_okay_to_study);
			if (spell >= 0) {
				cmd_insert(CMD_STUDY_SPELL);
				cmd_set_arg_choice(cmd_get_top(), 0, spell);
			}
		} else {
			cmd_insert(CMD_STUDY_BOOK);
			cmd_set_arg_item(cmd_get_top(), 0, slot);
		}
	} else
	if (selected == 11) {
		/* use the item to refill a light source */
		cmd_insert(CMD_REFILL);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	} else
	if (selected == 12) {
		/* throw the item */
		cmd_insert(CMD_THROW);
		cmd_set_arg_item(cmd_get_top(), 0, slot);
	}
	return 1;
}

/* pick the context menu options appropiate for a store */
int context_menu_store(struct store *store, const int oid, int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	object_type *o_ptr;

	m = menu_dynamic_new();
	if (!m || !store) {
		return 0;
	}

	/* Get the actual object */
	o_ptr = &store->stock[oid];

	m->selections = lower_case;
	menu_dynamic_add(m, "Inspect Inventory", 1);
	if (store->sidx == STORE_HOME) {
		/*menu_dynamic_add(m, "Stash One", 2);*/
		menu_dynamic_add(m, "Stash", 3);
		menu_dynamic_add(m, "Examine", 4);
		menu_dynamic_add(m, "Take", 6);
		if (o_ptr->number > 1) {
			menu_dynamic_add(m, "Take One", 5);
		}
	} else {
		/*menu_dynamic_add(m, "Sell One", 2);*/
		menu_dynamic_add(m, "Sell", 3);
		menu_dynamic_add(m, "Examine", 4);
		menu_dynamic_add(m, "Buy", 6);
		if (o_ptr->number > 1) {
			menu_dynamic_add(m, "Buy One", 5);
		}
	}
	menu_dynamic_add(m, "Exit", 7);


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

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

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
	object_type *o_ptr;

	/* Get the actual object */
	o_ptr = &store->stock[oid];


	m = menu_dynamic_new();
	if (!m || !store) {
		return 0;
	}

	m->selections = lower_case;
	menu_dynamic_add(m, "Examine", 4);
	if (store->sidx == STORE_HOME) {
		menu_dynamic_add(m, "Take", 6);
		if (o_ptr->number > 1) {
			menu_dynamic_add(m, "Take One", 5);
		}
	} else {
		menu_dynamic_add(m, "Buy", 6);
		if (o_ptr->number > 1) {
			menu_dynamic_add(m, "Buy One", 5);
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

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);
	menu_dynamic_free(m);

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

