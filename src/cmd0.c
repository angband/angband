/*
 * File: cmd0.c
 * Purpose: Deal with command processing.
 *
 * Copyright (c) 2010 Andi Sidwell
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
#include "files.h"
#include "game-cmd.h"
#include "keymap.h"
#include "textui.h"
#include "ui-event.h"
#include "ui-menu.h"
#include "wizard.h"
#include "target.h"

/*
 * This file contains (several) big lists of commands, so that they can be
 * easily maniuplated for e.g. help displays, or if a port wants to provide a
 * native menu containing a command list.
 *
 * Consider a two-paned layout for the command menus. XXX
 *
 * This file still needs some clearing up. XXX
 */

/*** Handling bits ***/

/*
 * Holds a generic command - if cmd is set to other than CMD_NULL 
 * it simply pushes that command to the game, otherwise the hook 
 * function will be called.
 */
struct cmd_info
{
	const char *desc;
	keycode_t key[2];
	cmd_code cmd;
	void (*hook)(void);
	bool (*prereq)(void);
};

static struct cmd_info cmd_item[] =
{
	{ "Inscribe an object", { '{' }, CMD_INSCRIBE },
	{ "Uninscribe an object", { '}' }, CMD_UNINSCRIBE },
	{ "Wear/wield an item", { 'w' }, CMD_WIELD },
	{ "Take off/unwield an item", { 't', 'T'}, CMD_TAKEOFF },
	{ "Examine an item", { 'I' }, CMD_NULL, textui_obj_examine },
	{ "Drop an item", { 'd' }, CMD_DROP },
	{ "Fire your missile weapon", { 'f', 't' }, CMD_FIRE, NULL, player_can_fire_msg },
	{ "Use a staff", { 'u', 'Z' }, CMD_USE_STAFF },
	{ "Aim a wand", {'a', 'z'}, CMD_USE_WAND },
	{ "Zap a rod", {'z', 'a'}, CMD_USE_ROD },
	{ "Activate an object", {'A' }, CMD_ACTIVATE },
	{ "Eat some food", { 'E' }, CMD_EAT },
	{ "Quaff a potion", { 'q' }, CMD_QUAFF },
	{ "Read a scroll", { 'r' }, CMD_READ_SCROLL, NULL, player_can_read_msg },
	{ "Fuel your light source", { 'F' }, CMD_REFILL, NULL, player_can_refuel_msg },
	{ "Use an item", { 'U' }, CMD_USE_ANY }
};

/* General actions */
static struct cmd_info cmd_action[] =
{
	{ "Search for traps/doors", { 's' }, CMD_SEARCH },
	{ "Disarm a trap or chest", { 'D' }, CMD_DISARM },
	{ "Rest for a while", { 'R' }, CMD_NULL, textui_cmd_rest },
	{ "Look around", { 'l', 'x' }, CMD_NULL, do_cmd_look },
	{ "Target monster or location", { '*' }, CMD_NULL, do_cmd_target },
	{ "Target closest monster", { '\'' }, CMD_NULL, do_cmd_target_closest },
	{ "Dig a tunnel", { 'T', KTRL('T') }, CMD_TUNNEL },
	{ "Go up staircase", {'<' }, CMD_GO_UP },
	{ "Go down staircase", { '>' }, CMD_GO_DOWN },
	{ "Toggle search mode", { 'S', '#' }, CMD_TOGGLE_SEARCH },
	{ "Open a door or a chest", { 'o' }, CMD_OPEN },
	{ "Close a door", { 'c' }, CMD_CLOSE },
	{ "Jam a door shut", { 'j', 'S' }, CMD_JAM },
	{ "Bash a door open", { 'B', 'f' }, CMD_BASH },
	{ "Fire at nearest target", { 'h', KC_TAB }, CMD_NULL, textui_cmd_fire_at_nearest },
	{ "Throw an item", { 'v' }, CMD_THROW, textui_cmd_throw },
	{ "Walk into a trap", { 'W', '-' }, CMD_JUMP, NULL },
};

/* Item management commands */
static struct cmd_info cmd_item_manage[] =
{
	{ "Display equipment listing", { 'e' }, CMD_NULL, do_cmd_equip },
	{ "Display inventory listing", { 'i' }, CMD_NULL, do_cmd_inven },
	{ "Pick up objects", { 'g' }, CMD_PICKUP, NULL },
	{ "Destroy an item", { 'k', KTRL('D') }, CMD_DESTROY, textui_cmd_destroy },	
};

/* Information access commands */
static struct cmd_info cmd_info[] =
{
	{ "Browse a book", { 'b', 'P' }, CMD_BROWSE_SPELL, textui_spell_browse },
	{ "Gain new spells", { 'G' }, CMD_STUDY_BOOK, textui_obj_study, player_can_study_msg },
	{ "Cast a spell", { 'm' }, CMD_CAST, textui_obj_cast, player_can_cast_msg },
	{ "Cast a spell", { 'p' }, CMD_CAST, textui_obj_cast, player_can_cast_msg },
	{ "Full dungeon map", { 'M' }, CMD_NULL, do_cmd_view_map },
	{ "Toggle ignoring of items", { 'K', 'O' }, CMD_NULL, textui_cmd_toggle_ignore },
	{ "Display visible item list", { ']' }, CMD_NULL, do_cmd_itemlist },
	{ "Display visible monster list", { '[' }, CMD_NULL, do_cmd_monlist },
	{ "Locate player on map", { 'L', 'W' }, CMD_NULL, do_cmd_locate },
	{ "Help", { '?' }, CMD_NULL, do_cmd_help },
	{ "Identify symbol", { '/' }, CMD_NULL, do_cmd_query_symbol },
	{ "Character description", { 'C' }, CMD_NULL, do_cmd_change_name },
	{ "Check knowledge", { '~' }, CMD_NULL, textui_browse_knowledge },
	{ "Repeat level feeling", { KTRL('F') }, CMD_NULL, do_cmd_feeling },
	{ "Show previous message", { KTRL('O') }, CMD_NULL, do_cmd_message_one },
	{ "Show previous messages", { KTRL('P') }, CMD_NULL, do_cmd_messages }
};

/* Utility/assorted commands */
static struct cmd_info cmd_util[] =
{
	{ "Interact with options", { '=' }, CMD_NULL, do_cmd_xxx_options },

	{ "Save and don't quit", { KTRL('S') }, CMD_SAVE },
	{ "Save and quit", { KTRL('X') }, CMD_QUIT },
	{ "Quit (commit suicide)", { 'Q' }, CMD_NULL, textui_cmd_suicide },
	{ "Redraw the screen", { KTRL('R') }, CMD_NULL, do_cmd_redraw },

	{ "Load \"screen dump\"", { '(' }, CMD_NULL, do_cmd_load_screen },
	{ "Save \"screen dump\"", { ')' }, CMD_NULL, do_cmd_save_screen }
};

/* Commands that shouldn't be shown to the user */ 
static struct cmd_info cmd_hidden[] =
{
	{ "Take notes", { ':' }, CMD_NULL, do_cmd_note },
	{ "Version info", { 'V' }, CMD_NULL, do_cmd_version },
	{ "Load a single pref line", { '"' }, CMD_NULL, do_cmd_pref },
	{ "Enter a store", { '_' }, CMD_ENTER_STORE, NULL },
	{ "Toggle windows", { KTRL('E') }, CMD_NULL, toggle_inven_equip }, /* XXX */
	{ "Alter a grid", { '+' }, CMD_ALTER, NULL },
	{ "Walk", { ';' }, CMD_WALK, NULL },
	{ "Start running", { '.', ',' }, CMD_RUN, NULL },
	{ "Stand still", { ',', '.' }, CMD_HOLD, NULL },
	{ "Center map", { KTRL('L'), '@' }, CMD_NULL, do_cmd_center_map },

	{ "Toggle wizard mode", { KTRL('W') }, CMD_NULL, do_cmd_wizard },
	{ "Repeat previous command", { 'n', KTRL('V') }, CMD_REPEAT, NULL },
	{ "Do autopickup", { KTRL('G') }, CMD_AUTOPICKUP, NULL },

#ifdef ALLOW_DEBUG
	{ "Debug mode commands", { KTRL('A') }, CMD_NULL, do_cmd_try_debug },
#endif
#ifdef ALLOW_BORG
	{ "Borg commands", { KTRL('Z') }, CMD_NULL, do_cmd_try_borg }
#endif
};



/*
 * A categorised list of all the command lists.
 */
typedef struct
{
	const char *name;
	struct cmd_info *list;
	size_t len;
} command_list;

static command_list cmds_all[] =
{
	{ "Items",           cmd_item,        N_ELEMENTS(cmd_item) },
	{ "Action commands", cmd_action,      N_ELEMENTS(cmd_action) },
	{ "Manage items",    cmd_item_manage, N_ELEMENTS(cmd_item_manage) },
	{ "Information",     cmd_info,        N_ELEMENTS(cmd_info) },
	{ "Utility",         cmd_util,        N_ELEMENTS(cmd_util) },
	{ "Hidden",          cmd_hidden,      N_ELEMENTS(cmd_hidden) }
};




/*** Menu functions ***/

/* Display an entry on a command menu */
static void cmd_sub_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);
	const struct cmd_info *commands = menu_priv(menu);

	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	struct keypress kp = { EVT_KBRD, commands[oid].key[mode] };
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

/*
 * Display a list of commands.
 */
static bool cmd_menu(command_list *list, void *selection_p)
{
	menu_type menu;
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
	evt = menu_select(&menu, 0, TRUE);

	/* Load de screen */
	screen_load();

	if (evt.type == EVT_SELECT)
		*selection = &list->list[menu.cursor];

	return FALSE;
}



static bool cmd_list_action(menu_type *m, const ui_event *event, int oid)
{
	if (event->type == EVT_SELECT)
		return cmd_menu(&cmds_all[oid], menu_priv(m));
	else
		return FALSE;
}

static void cmd_list_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);
	Term_putstr(col, row, -1, attr, cmds_all[oid].name);
}

static menu_type *command_menu;
static menu_iter command_menu_iter =
{
	NULL,
	NULL,
	cmd_list_entry,
	cmd_list_action,
	NULL
};

/*
 * Display a list of command types, allowing the user to select one.
 */
static struct cmd_info *textui_action_menu_choose(void)
{
	region area = { 21, 5, 37, 6 };

	struct cmd_info *chosen_command = NULL;

	if (!command_menu)
		command_menu = menu_new(MN_SKIN_SCROLL, &command_menu_iter);

	menu_setpriv(command_menu, N_ELEMENTS(cmds_all) - 1, &chosen_command);
	menu_layout(command_menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(19, 4, 58, 11);

	menu_select(command_menu, 0, TRUE);

	screen_load();

	return chosen_command;
}


/*** Exported functions ***/

/* List indexed by char */
/* XXX 2 shoud be KEYMAP_MAX */
static struct cmd_info *converted_list[2][UCHAR_MAX+1];


/*
 * Initialise the command list.
 */
void cmd_init(void)
{
	size_t i, j;

	memset(converted_list, 0, sizeof(converted_list));

	/* Go through all generic commands */
	for (j = 0; j < N_ELEMENTS(cmds_all); j++)
	{
		struct cmd_info *commands = cmds_all[j].list;

		/* Fill everything in */
		for (i = 0; i < cmds_all[j].len; i++) {
			/* If a roguelike key isn't set, use default */
			if (!commands[i].key[1])
				commands[i].key[1] = commands[i].key[0];

			converted_list[0][commands[i].key[0]] = &commands[i];
			converted_list[1][commands[i].key[1]] = &commands[i];
		}
	}
}

unsigned char cmd_lookup_key(cmd_code lookup_cmd, int mode)
{
	unsigned int i;

	assert(mode == KEYMAP_MODE_ROGUE || mode == KEYMAP_MODE_ORIG);

	for (i = 0; i < N_ELEMENTS(converted_list[mode]); i++) {
		struct cmd_info *cmd = converted_list[mode][i];

		if (cmd && cmd->cmd == lookup_cmd)
			return cmd->key[mode];
	}

	return 0;
}

cmd_code cmd_lookup(unsigned char key, int mode)
{
	assert(mode == KEYMAP_MODE_ROGUE || mode == KEYMAP_MODE_ORIG);

	if (!converted_list[mode][key])
		return CMD_NULL;

	return converted_list[mode][key]->cmd;
}


/*** Input processing ***/


/**
 * Get a command count, with the '0' key.
 */
static int textui_get_count(void)
{
	int count = 0;

	while (1)
	{
		struct keypress ke;

		prt(format("Count: %d", count), 0, 0);

		ke = inkey();
		if (ke.code == ESCAPE)
			return -1;

		/* Simple editing (delete or backspace) */
		else if (ke.code == KC_DELETE || ke.code == KC_BACKSPACE)
			count = count / 10;

		/* Actual numeric data */
		else if (isdigit((unsigned char) ke.code))
		{
			count = count * 10 + D2I(ke.code);

			if (count >= 9999)
			{
				bell("Invalid repeat count!");
				count = 9999;
			}
		}

		/* Anything non-numeric passes straight to command input */
		else
		{
			/* XXX nasty hardcoding of action menu key */
			if (ke.code != KC_ENTER)
				Term_keypress(ke.code, ke.mods);

			break;
		}
	}

	return count;
}



/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static struct keypress request_command_buffer[256];


/*
 * Request a command from the user.
 *
 * Note that "caret" ("^") is treated specially, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that "backslash" is treated specially, and is used to bypass any
 * keymap entry for the following character.  This is useful for macros.
 */
static ui_event textui_get_command(void)
{
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	struct keypress tmp[2] = { { 0 }, { 0 } };

	ui_event ke = EVENT_EMPTY;

	const struct keypress *act = NULL;



	/* Get command */
	while (1)
	{
		/* Hack -- no flush needed */
		msg_flag = FALSE;

		/* Activate "command mode" */
		inkey_flag = TRUE;

		/* Get a command */
		ke = inkey_ex();

		if (ke.type == EVT_KBRD) {
			bool keymap_ok = TRUE;
			switch (ke.key.code) {
				case '0': {
					int count = textui_get_count();

					if (count == -1 || !get_com_ex("Command: ", &ke))
						continue;
					else
						p_ptr->command_arg = count;
					break;
				}

				case '\\': {
					/* Allow keymaps to be bypassed */
					(void)get_com_ex("Command: ", &ke);
					keymap_ok = FALSE;
					break;
				}

				case '^': {
					/* Allow "control chars" to be entered */
					if (get_com("Control: ", &ke.key))
						ke.key.code = KTRL(ke.key.code);
					break;
				}
			}

			/* Find any relevant keymap */
			if (keymap_ok)
				act = keymap_find(mode, ke.key);
		}

		/* Erase the message line */
		prt("", 0, 0);

		if (ke.type == EVT_BUTTON)
		{
			/* Buttons are always specified in standard keyset */
			act = tmp;
			tmp[0] = ke.key;
		}

		/* Apply keymap if not inside a keymap already */
		if (ke.key.code && act && !inkey_next)
		{
			size_t n = 0;
			while (act[n].type)
				n++;

			/* Make room for the terminator */
			n += 1;

			/* Install the keymap */
			memcpy(request_command_buffer, act, n * sizeof(struct keypress));

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			continue;
		}

		/* Done */
		break;
	}

	return ke;
}

int show_command_list(struct cmd_info cmd_list[], int size, int mx, int my)
{
	menu_type *m;
	region r;
	int selected;
	int i;
	char cmd_name[80];
	char key[3];

	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}
	m->selections = lower_case;
	key[2] = '\0';

	for (i=0; i < size; ++i) {
		if (KTRL(cmd_list[i].key[mode]) == cmd_list[i].key[mode]) {
			key[0] = '^';
			key[1] = UN_KTRL(cmd_list[i].key[mode]);
		} else {
			key[0] = cmd_list[i].key[mode];
			key[1] = '\0';
		}
		strnfmt(cmd_name, 80, "%s (%s)",  cmd_list[i].desc, key[mode]);
		menu_dynamic_add(m, cmd_name, i+1);
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

	if ((selected > 0) && (selected < size+1)) {
		/* execute the command */
		Term_keypress(cmd_list[selected-1].key[mode], 0);
	}

	return 1;
}

int context_menu_command(int mx, int my)
{
	menu_type *m;
	region r;
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

	switch (selected) {
		case 1:
			show_command_list(cmd_item, N_ELEMENTS(cmd_item), mx, my);
			break;
		case 2:
			show_command_list(cmd_action, N_ELEMENTS(cmd_action), mx, my);
			break;
		case 3:
			show_command_list(cmd_item_manage, N_ELEMENTS(cmd_item_manage), mx, my);
			break;
		case 4:
			show_command_list(cmd_info, N_ELEMENTS(cmd_info), mx, my);
			break;
		case 5:
			show_command_list(cmd_util, N_ELEMENTS(cmd_util), mx, my);
			break;
		case 6:
			show_command_list(cmd_hidden, N_ELEMENTS(cmd_hidden), mx, my);
			break;
	}

	return 1;
}

int context_menu_player(int mx, int my);
int context_menu_cave(struct cave *cave, int y, int x, int adjacent,int mx, int my);

/**
 * Handle a textui mouseclick.
 */
static void textui_process_click(ui_event e)
{
	int x, y;

	if (!OPT(mouse_movement)) return;

	y = KEY_GRID_Y(e);
	x = KEY_GRID_X(e);

	/* Check for a valid location */
	if (!in_bounds_fully(y, x)) return;

	/* XXX show context menu here */
	if ((p_ptr->py == y) && (p_ptr->px == x)) {
		if (e.mouse.mods & KC_MOD_SHIFT) {
			/* shift-click - cast magic */
			if (e.mouse.button == 1) {
				textui_obj_cast();
			} else
			if (e.mouse.button == 2) {
				Term_keypress('i',0);
			}
		} else
		if (e.mouse.mods & KC_MOD_CONTROL) {
			/* ctrl-click - use feature / use inventory item */
			/* switch with default */
			if (e.mouse.button == 1) {
				if (cave->feat[p_ptr->py][p_ptr->px] == FEAT_LESS) {
					cmd_insert(CMD_GO_UP);
				} else
				if (cave->feat[p_ptr->py][p_ptr->px] == FEAT_MORE) {
					cmd_insert(CMD_GO_DOWN);
				}
			} else
			if (e.mouse.button == 2) {
				cmd_insert(CMD_USE_UNAIMED);
			}
		} else
		if (e.mouse.mods & KC_MOD_ALT) {
			/* alt-click - Search  or show char screen */
			/* XXX call a platform specific hook */
			if (e.mouse.button == 1) {
 				cmd_insert(CMD_SEARCH);
			} else
			if (e.mouse.button == 2) {
				Term_keypress('C',0);
			}
		} else
		{
			if (e.mouse.button == 1) {
				if (cave->o_idx[y][x]) {
					cmd_insert(CMD_PICKUP);
				} else {
					cmd_insert(CMD_HOLD);
				}
			} else
			if (e.mouse.button == 2) {
				// show a context menu
				context_menu_player(e.mouse.x, e.mouse.y);
			}
		}
	}

	else if (e.mouse.button == 1)
	{
		if (p_ptr->timed[TMD_CONFUSED])
		{
			cmd_insert(CMD_WALK);
		}
		else
		{
			if (e.mouse.mods & KC_MOD_SHIFT) {
				/* shift-click - run */
				cmd_insert(CMD_RUN);
				cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
				/*if ((y-p_ptr->py >= -1) && (y-p_ptr->py <= 1)
					&& (x-p_ptr->px >= -1) && (x-p_ptr->px <= 1)) {
					cmd_insert(CMD_JUMP);
					cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
				} else {
				  cmd_insert(CMD_RUN);
				  cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
				}*/
			} else
			if (e.mouse.mods & KC_MOD_CONTROL) {
				/* control-click - alter */
				cmd_insert(CMD_ALTER);
				cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
			} else
			if (e.mouse.mods & KC_MOD_ALT) {
				/* alt-click - look */
				if (target_set_interactive(TARGET_LOOK, x, y)) {
					msg("Target Selected.");
				}
				//cmd_insert(CMD_LOOK);
				//cmd_set_arg_point(cmd_get_top(), 0, y, x);
			} else
			{
				/* pathfind does not work well on trap detection borders,
				 * so if the click is next to the player, force a walk step */
				if ((y-p_ptr->py >= -1) && (y-p_ptr->py <= 1)
					&& (x-p_ptr->px >= -1) && (x-p_ptr->px <= 1)) {
					cmd_insert(CMD_WALK);
					cmd_set_arg_direction(cmd_get_top(), 0, coords_to_dir(y,x));
				} else {
					cmd_insert(CMD_PATHFIND);
					cmd_set_arg_point(cmd_get_top(), 0, y, x);
				}
			}
		}
	}

	else if (e.mouse.button == 2)
	{
		int m_idx = cave->m_idx[y][x];
		if (m_idx && target_able(m_idx)) {
			monster_type *m_ptr = cave_monster(cave, m_idx);
			/* Set up target information */
			monster_race_track(m_ptr->r_idx);
			health_track(p_ptr, m_ptr);
			target_set_monster(m_idx);
		} else {
			target_set_location(y,x);
		}
		if (e.mouse.mods & KC_MOD_SHIFT) {
			/* shift-click - cast spell at target */
			if (textui_obj_cast_ret() >= 0) {
				cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
			}
		} else
		if (e.mouse.mods & KC_MOD_CONTROL) {
			/* control-click - fire at target */
			cmd_insert(CMD_USE_AIMED);
			cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
		} else
		if (e.mouse.mods & KC_MOD_ALT) {
			/* alt-click - throw at target */
			cmd_insert(CMD_THROW);
			cmd_set_arg_target(cmd_get_top(), 1, DIR_TARGET);
		} else
		{
			//msg("Target set.");
			/* see if the click was adjacent to the player */
			if ((y-p_ptr->py >= -1) && (y-p_ptr->py <= 1)
				&& (x-p_ptr->px >= -1) && (x-p_ptr->px <= 1)) {
				context_menu_cave(cave,y,x,1,e.mouse.x, e.mouse.y);
			} else {
				context_menu_cave(cave,y,x,0,e.mouse.x, e.mouse.y);
			}
		}
	}
}


/**
 * Check no currently worn items are stopping the action 'c'
 */
static bool key_confirm_command(unsigned char c)
{
	int i;

	/* Hack -- Scan equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		char verify_inscrip[] = "^*";
		unsigned n;

		object_type *o_ptr = &p_ptr->inventory[i];
		if (!o_ptr->kind) continue;

		/* Set up string to look for, e.g. "^d" */
		verify_inscrip[1] = c;

		/* Verify command */
		n = check_for_inscrip(o_ptr, "^*") +
				check_for_inscrip(o_ptr, verify_inscrip);
		while (n--)
		{
			if (!get_check("Are you sure? "))
				return FALSE;
		}
	}

	return TRUE;
}


/**
 * Process a textui keypress.
 */
static bool textui_process_key(struct keypress kp)
{
	struct cmd_info *cmd;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	/* XXXmacro this needs rewriting */
	keycode_t c = kp.code;

	if (c == '\0' || c == ESCAPE || c == ' ' || c == '\a')
		return TRUE;

	if (c == KC_ENTER) {
		cmd = textui_action_menu_choose();
	} else {
		if (c > UCHAR_MAX) return FALSE;
		cmd = converted_list[mode][c];
	}

	if (!cmd) return FALSE;

	if (key_confirm_command(c) && (!cmd->prereq || cmd->prereq())) {
		if (cmd->hook)
			cmd->hook();
		else if (cmd->cmd)
			cmd_insert_repeated(cmd->cmd, p_ptr->command_arg);
	}

	return TRUE;
}


/**
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void textui_process_command(bool no_request)
{
	bool done = TRUE;
	ui_event e;

	/* Reset argument before getting command */
	p_ptr->command_arg = 0;
	e = textui_get_command();

	if (e.type == EVT_RESIZE)
		do_cmd_redraw();

	else if (e.type == EVT_MOUSE)
		textui_process_click(e);

	else if (e.type == EVT_KBRD)
		done = textui_process_key(e.key);

	else if (e.type == EVT_BUTTON)
		done = textui_process_key(e.key);

	if (!done)
		do_cmd_unknown();
}
