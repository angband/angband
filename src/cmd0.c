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
#include "monster/monster.h"
#include "textui.h"
#include "ui-menu.h"
#include "wizard.h"

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
	unsigned char key;
	cmd_code cmd;
	void (*hook)(void);
	bool (*prereq)(void);
};

static struct cmd_info cmd_item[] =
{
	{ "Inscribe an object", '{', CMD_INSCRIBE, NULL, NULL },
	{ "Uninscribe an object", '}', CMD_UNINSCRIBE, NULL, NULL },
	{ "Wear/wield an item", 'w', CMD_WIELD, NULL, NULL },
	{ "Take off/unwield an item", 't', CMD_TAKEOFF, NULL, NULL },
	{ "Examine an item", 'I', CMD_NULL, textui_obj_examine },
	{ "Drop an item", 'd', CMD_DROP, NULL, NULL },
	{ "Fire your missile weapon", 'f', CMD_FIRE, NULL, player_can_fire },
	{ "Use a staff", 'u', CMD_USE_STAFF, NULL, NULL },
	{ "Aim a wand", 'a', CMD_USE_WAND, NULL, NULL },
	{ "Zap a rod", 'z', CMD_USE_ROD, NULL, NULL },
	{ "Activate an object", 'A', CMD_ACTIVATE, NULL, NULL },
	{ "Eat some food", 'E', CMD_EAT, NULL, NULL },
	{ "Quaff a potion", 'q', CMD_QUAFF, NULL, NULL },
	{ "Read a scroll", 'r', CMD_READ_SCROLL, NULL, player_can_read },
	{ "Fuel your light source", 'F', CMD_REFILL, NULL, NULL },
};

/* General actions */
static struct cmd_info cmd_action[] =
{
	{ "Search for traps/doors",     's', CMD_SEARCH, NULL },
	{ "Disarm a trap or chest",     'D', CMD_DISARM, NULL },
	{ "Rest for a while",           'R', CMD_NULL, textui_cmd_rest },
	{ "Look around",                'l', CMD_NULL, do_cmd_look },
	{ "Target monster or location", '*', CMD_NULL, do_cmd_target },
	{ "Target closest monster",     '\'', CMD_NULL, do_cmd_target_closest },
	{ "Dig a tunnel",               'T', CMD_TUNNEL, NULL },
	{ "Go up staircase",            '<', CMD_GO_UP, NULL },
	{ "Go down staircase",          '>', CMD_GO_DOWN, NULL },
	{ "Toggle search mode",         'S', CMD_TOGGLE_SEARCH, NULL },
	{ "Open a door or a chest",     'o', CMD_OPEN, NULL },
	{ "Close a door",               'c', CMD_CLOSE, NULL },
	{ "Jam a door shut",            'j', CMD_JAM, NULL },
	{ "Bash a door open",           'B', CMD_BASH, NULL },
	{ "Fire at nearest target",   'h', CMD_NULL, textui_cmd_fire_at_nearest },
	{ "Throw an item",            'v', CMD_THROW, textui_cmd_throw },
	{ "Walk into a trap",         'W', CMD_JUMP, NULL },
};

/* Item management commands */
static struct cmd_info cmd_item_manage[] =
{
	{ "Display equipment listing", 'e', CMD_NULL, do_cmd_equip },
	{ "Display inventory listing", 'i', CMD_NULL, do_cmd_inven },
	{ "Pick up objects",           'g', CMD_PICKUP, NULL },
	{ "Destroy an item",           'k', CMD_DESTROY, textui_cmd_destroy },	
};

/* Information access commands */
static struct cmd_info cmd_info[] =
{
	{ "Browse a book", 'b', CMD_BROWSE_SPELL, textui_spell_browse, NULL },
	{ "Gain new spells", 'G', CMD_STUDY_BOOK, textui_obj_study, player_can_study },
	{ "Cast a spell", 'm', CMD_CAST, textui_obj_cast, player_can_cast },
	{ "Cast a spell", 'p', CMD_CAST, textui_obj_cast, player_can_cast },
	{ "Full dungeon map",             'M', CMD_NULL, do_cmd_view_map },
	{ "Toggle ignoring of items",     'K', CMD_NULL, textui_cmd_toggle_ignore },
	{ "Display visible item list",    ']', CMD_NULL, do_cmd_itemlist },
	{ "Display visible monster list", '[', CMD_NULL, do_cmd_monlist },
	{ "Locate player on map",         'L', CMD_NULL, do_cmd_locate },
	{ "Help",                         '?', CMD_NULL, do_cmd_help },
	{ "Identify symbol",              '/', CMD_NULL, do_cmd_query_symbol },
	{ "Character description",        'C', CMD_NULL, do_cmd_change_name },
	{ "Check knowledge",              '~', CMD_NULL, textui_browse_knowledge },
	{ "Repeat level feeling",   KTRL('F'), CMD_NULL, do_cmd_feeling },
	{ "Show previous message",  KTRL('O'), CMD_NULL, do_cmd_message_one },
	{ "Show previous messages", KTRL('P'), CMD_NULL, do_cmd_messages }
};

/* Utility/assorted commands */
static struct cmd_info cmd_util[] =
{
	{ "Interact with options",        '=', CMD_NULL, do_cmd_xxx_options },

	{ "Save and don't quit",  KTRL('S'), CMD_SAVE, NULL },
	{ "Save and quit",        KTRL('X'), CMD_QUIT, NULL },
	{ "Quit (commit suicide)",      'Q', CMD_NULL, textui_cmd_suicide },
	{ "Redraw the screen",    KTRL('R'), CMD_NULL, do_cmd_redraw },

	{ "Load \"screen dump\"",       '(', CMD_NULL, do_cmd_load_screen },
	{ "Save \"screen dump\"",       ')', CMD_NULL, do_cmd_save_screen }
};

/* Commands that shouldn't be shown to the user */ 
static struct cmd_info cmd_hidden[] =
{
	{ "Take notes",               ':', CMD_NULL, do_cmd_note },
	{ "Version info",             'V', CMD_NULL, do_cmd_version },
	{ "Load a single pref line",  '"', CMD_NULL, do_cmd_pref },
	{ "Enter a store",            '_', CMD_ENTER_STORE, NULL },
	{ "Toggle windows",     KTRL('E'), CMD_NULL, toggle_inven_equip }, /* XXX */
	{ "Alter a grid",             '+', CMD_ALTER, NULL },
	{ "Walk",                     ';', CMD_WALK, NULL },
	{ "Start running",            '.', CMD_RUN, NULL },
	{ "Stand still",              ',', CMD_HOLD, NULL },
	{ "Center map",              KTRL('L'), CMD_NULL, do_cmd_center_map },

	{ "Toggle wizard mode",  KTRL('W'), CMD_NULL, do_cmd_wizard },
	{ "Repeat previous command",  KTRL('V'), CMD_REPEAT, NULL },
	{ "Do autopickup",           KTRL('G'), CMD_AUTOPICKUP, NULL },

#ifdef ALLOW_DEBUG
	{ "Debug mode commands", KTRL('A'), CMD_NULL, do_cmd_try_debug },
#endif
#ifdef ALLOW_BORG
	{ "Borg commands",       KTRL('Z'), CMD_NULL, do_cmd_try_borg }
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

	(void)width;

	/* Write the description */
	Term_putstr(col, row, -1, attr, commands[oid].desc);

	/* Include keypress */
	Term_addch(attr, ' ');
	Term_addch(attr, '(');

	/* KTRL()ing a control character does not alter it at all */
	if (KTRL(commands[oid].key) == commands[oid].key)
	{
		Term_addch(attr, '^');
		Term_addch(attr, UN_KTRL(commands[oid].key));
	}
	else
	{
		Term_addch(attr, commands[oid].key);
	}

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
	struct cmd_info *selection = selection_p;

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
		*selection = list->list[menu.cursor];

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
static char textui_action_menu_choose(void)
{
	region area = { 21, 5, 37, 6 };

	struct cmd_info chosen_command = { 0 };

	if (!command_menu)
		command_menu = menu_new(MN_SKIN_SCROLL, &command_menu_iter);

	menu_setpriv(command_menu, N_ELEMENTS(cmds_all) - 1, &chosen_command);
	menu_layout(command_menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(19, 4, 58, 11);

	menu_select(command_menu, 0, TRUE);

	screen_load();

	return chosen_command.key;
}


/*** Exported functions ***/

/* List indexed by char */
static struct cmd_info *converted_list[UCHAR_MAX+1];


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
		for (i = 0; i < cmds_all[j].len; i++)
			converted_list[commands[i].key] = &commands[i];
	}
}

unsigned char cmd_lookup_key(cmd_code lookup_cmd)
{
	unsigned int i;

	for (i = 0; i < N_ELEMENTS(converted_list); i++) {
		struct cmd_info *cmd = converted_list[i];

		if (cmd && cmd->cmd == lookup_cmd)
			return cmd->key;
	}

	return 0;
}

cmd_code cmd_lookup(unsigned char key)
{
	if (!converted_list[key])
		return CMD_NULL;

	return converted_list[key]->cmd;
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
		else if (ke.code == 0x7F || ke.code == KTRL('H'))
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
			if (ke.code != '\n' && ke.code != '\r')
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
	if ((p_ptr->py == y) && (p_ptr->px == x))
		textui_cmd_rest();

	else /* if (e.mousebutton == 1) */
	{
		if (p_ptr->timed[TMD_CONFUSED])
		{
			cmd_insert(CMD_WALK);
		}
		else
		{
			cmd_insert(CMD_PATHFIND);
			cmd_set_arg_point(cmd_get_top(), 0, y, x);
		}
	}

#if 0
	else if (e.mousebutton == 2)
	{
		target_set_location(y, x);
		msg_print("Target set.");
	}
#endif
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

	/* XXXmacro this needs rewriting */
	unsigned char c = (unsigned char)kp.code;

	if (c == '\n' || c == '\r')
		c = textui_action_menu_choose();

	if (c == '\0' || c == ESCAPE || c == ' ' || c == '\a')
		return TRUE;

	cmd = converted_list[c];
	if (!cmd) return FALSE;

	if (key_confirm_command(c) &&
			(!cmd->prereq || cmd->prereq())) {
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

	if (!done)
		do_cmd_unknown();
}
