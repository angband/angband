/*
 * File: cmd0.c
 * Purpose: Deal with command processing.
 *
 * Copyright (c) 2007 Andrew Sidwell, Ben Harrison
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
#include "game-cmd.h"
#include "monster/monster.h"
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

/*** Big list of commands ***/

/* Useful typedef */
typedef void do_cmd_type(void);


/* Forward declare these, because they're really defined later */
static do_cmd_type do_cmd_wizard, do_cmd_try_debug,
            do_cmd_mouseclick, do_cmd_port,
			do_cmd_xxx_options, do_cmd_menu, do_cmd_monlist, do_cmd_itemlist;

#ifdef ALLOW_BORG
static do_cmd_type do_cmd_try_borg;
#endif

/*
 * Holds a generic command - if cmd is set to other than CMD_NULL 
 * it simply pushes that command to the game, otherwise the hook 
 * function will be called.
 */
typedef struct
{
	const char *desc;
	unsigned char key;
	cmd_code cmd;
	do_cmd_type *hook;
} command_type;

/* Magic use */
static command_type cmd_magic[] =
{
	{ "Gain new spells or prayers", 'G', CMD_NULL, textui_cmd_study },
	{ "Browse a book",              'b', CMD_NULL, do_cmd_browse },
	{ "Cast a spell",               'm', CMD_NULL, textui_cmd_cast },
	{ "Pray a prayer",              'p', CMD_NULL, textui_cmd_pray }
};

/* General actions */
static command_type cmd_action[] =
{
	{ "Search for traps/doors",     's', CMD_SEARCH, NULL },
	{ "Disarm a trap or chest",     'D', CMD_NULL, textui_cmd_disarm },
	{ "Rest for a while",           'R', CMD_NULL, textui_cmd_rest },
	{ "Look around",                'l', CMD_NULL, do_cmd_look },
	{ "Target monster or location", '*', CMD_NULL, do_cmd_target },
	{ "Target closest monster",     '\'', CMD_NULL, do_cmd_target_closest },
	{ "Dig a tunnel",               'T', CMD_NULL, textui_cmd_tunnel },
	{ "Go up staircase",            '<', CMD_GO_UP, NULL },
	{ "Go down staircase",          '>', CMD_GO_DOWN, NULL },
	{ "Toggle search mode",         'S', CMD_TOGGLE_SEARCH, NULL },
	{ "Open a door or a chest",     'o', CMD_NULL, textui_cmd_open },
	{ "Close a door",               'c', CMD_NULL, textui_cmd_close },
	{ "Jam a door shut",            'j', CMD_NULL, textui_cmd_spike },
	{ "Bash a door open",           'B', CMD_NULL, textui_cmd_bash }
};

/* Item use commands */
static command_type cmd_item_use[] =
{
	{ "Read a scroll",            'r', CMD_NULL, textui_cmd_read_scroll },
	{ "Quaff a potion",           'q', CMD_NULL, textui_cmd_quaff_potion },
	{ "Use a staff",              'u', CMD_NULL, textui_cmd_use_staff },
	{ "Aim a wand",               'a', CMD_NULL, textui_cmd_aim_wand },
	{ "Zap a rod",                'z', CMD_NULL, textui_cmd_zap_rod },
	{ "Activate an object",       'A', CMD_NULL, textui_cmd_activate },
	{ "Eat some food",            'E', CMD_NULL, textui_cmd_eat_food },
	{ "Fuel your light source",   'F', CMD_NULL, textui_cmd_refill },
	{ "Fire your missile weapon", 'f', CMD_NULL, textui_cmd_fire },
	{ "Fire at nearest target",   'h', CMD_NULL, textui_cmd_fire_at_nearest },
	{ "Throw an item",            'v', CMD_NULL, textui_cmd_throw }
};

/* Item management commands */
static command_type cmd_item_manage[] =
{
	{ "Display equipment listing", 'e', CMD_NULL, do_cmd_equip },
	{ "Display inventory listing", 'i', CMD_NULL, do_cmd_inven },
	{ "Pick up objects",           'g', CMD_PICKUP, NULL },
	{ "Wear/wield an item",        'w', CMD_NULL, textui_cmd_wield },
	{ "Take/unwield off an item",  't', CMD_NULL, textui_cmd_takeoff },
	{ "Drop an item",              'd', CMD_NULL, textui_cmd_drop },
	{ "Destroy an item",           'k', CMD_NULL, textui_cmd_destroy },
	{ "Examine an item",           'I', CMD_NULL, do_cmd_observe },
	{ "Inscribe an object",        '{', CMD_NULL, textui_cmd_inscribe },
	{ "Uninscribe an object",      '}', CMD_NULL, textui_cmd_uninscribe }
};

/* Information access commands */
static command_type cmd_info[] =
{
	{ "Full dungeon map",             'M', CMD_NULL, do_cmd_view_map },
	{ "Display visible item list",    ']', CMD_NULL, do_cmd_itemlist },
	{ "Display visible monster list", '[', CMD_NULL, do_cmd_monlist },
	{ "Locate player on map",         'L', CMD_NULL, do_cmd_locate },
	{ "Help",                         '?', CMD_NULL, do_cmd_help },
	{ "Identify symbol",              '/', CMD_NULL, do_cmd_query_symbol },
	{ "Character description",        'C', CMD_NULL, do_cmd_change_name },
	{ "Check knowledge",              '~', CMD_NULL, do_cmd_knowledge },
	{ "Repeat level feeling",   KTRL('F'), CMD_NULL, do_cmd_feeling },
	{ "Show previous message",  KTRL('O'), CMD_NULL, do_cmd_message_one },
	{ "Show previous messages", KTRL('P'), CMD_NULL, do_cmd_messages }
};

/* Utility/assorted commands */
static command_type cmd_util[] =
{
	{ "Interact with options",        '=', CMD_NULL, do_cmd_xxx_options },
	{ "Port-specific preferences",    '!', CMD_NULL, do_cmd_port },

	{ "Save and don't quit",  KTRL('S'), CMD_SAVE, NULL },
	{ "Save and quit",        KTRL('X'), CMD_QUIT, NULL },
	{ "Quit (commit suicide)",      'Q', CMD_NULL, textui_cmd_suicide },
	{ "Redraw the screen",    KTRL('R'), CMD_NULL, do_cmd_redraw },

	{ "Load \"screen dump\"",       '(', CMD_NULL, do_cmd_load_screen },
	{ "Save \"screen dump\"",       ')', CMD_NULL, do_cmd_save_screen }
};

/* Commands that shouldn't be shown to the user */ 
static command_type cmd_hidden[] =
{
	{ "Take notes",               ':', CMD_NULL, do_cmd_note },
	{ "Version info",             'V', CMD_NULL, do_cmd_version },
	{ "Load a single pref line",  '"', CMD_NULL, do_cmd_pref },
	{ "Enter a store",            '_', CMD_ENTER_STORE, NULL },
	{ "Toggle windows",     KTRL('E'), CMD_NULL, toggle_inven_equip }, /* XXX */
	{ "Alter a grid",             '+', CMD_NULL, textui_cmd_alter },
	{ "Walk",                     ';', CMD_NULL, textui_cmd_walk },
	{ "Jump into a trap",         '-', CMD_NULL, textui_cmd_jump },
	{ "Start running",            '.', CMD_NULL, textui_cmd_run },
	{ "Stand still",              ',', CMD_HOLD, NULL },
	{ "Check knowledge",          '|', CMD_NULL, do_cmd_knowledge },
	{ "Display menu of actions", '\n', CMD_NULL, do_cmd_menu },
	{ "Display menu of actions", '\r', CMD_NULL, do_cmd_menu },
	{ "Center map",              KTRL('L'), CMD_NULL, do_cmd_center_map },

	{ "Toggle wizard mode",  KTRL('W'), CMD_NULL, do_cmd_wizard },
	{ "Repeat previous command",  KTRL('V'), CMD_REPEAT, NULL },

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
	command_type *list;
	size_t len;
} command_list;

static command_list cmds_all[] =
{
	{ "Use magic/Pray",  cmd_magic,       N_ELEMENTS(cmd_magic) },
	{ "Action commands", cmd_action,      N_ELEMENTS(cmd_action) },
	{ "Use item",        cmd_item_use,    N_ELEMENTS(cmd_item_use) },
	{ "Manage items",    cmd_item_manage, N_ELEMENTS(cmd_item_manage) },
	{ "Information",     cmd_info,        N_ELEMENTS(cmd_info) },
	{ "Utility",         cmd_util,        N_ELEMENTS(cmd_util) },
	{ "Hidden",          cmd_hidden,      N_ELEMENTS(cmd_hidden) }
};





/*
 * Toggle wizard mode
 */
static void do_cmd_wizard(void)
{
	/* Verify first time */
	if (!(p_ptr->noscore & NOSCORE_WIZARD))
	{
		/* Mention effects */
		msg_print("You are about to enter 'wizard' mode for the very first time!");
		msg_print("This is a form of cheating, and your game will not be scored!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_WIZARD;
	}

	/* Toggle mode */
	if (p_ptr->wizard)
	{
		p_ptr->wizard = FALSE;
		msg_print("Wizard mode off.");
	}
	else
	{
		p_ptr->wizard = TRUE;
		msg_print("Wizard mode on.");
	}

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw "title" */
	p_ptr->redraw |= (PR_TITLE);
}




#ifdef ALLOW_DEBUG

/*
 * Verify use of "debug" mode
 */
static void do_cmd_try_debug(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_DEBUG))
	{
		/* Mention effects */
		msg_print("You are about to use the dangerous, unsupported, debug commands!");
		msg_print("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the debug commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_DEBUG;
	}

	/* Okay */
	do_cmd_debug();
}

#endif /* ALLOW_DEBUG */



#ifdef ALLOW_BORG

/*
 * Verify use of "borg" mode
 */
static void do_cmd_try_borg(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_BORG))
	{
		/* Mention effects */
		msg_print("You are about to use the dangerous, unsupported, borg commands!");
		msg_print("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the borg commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_BORG;
	}

	/* Okay */
	do_cmd_borg();
}

#endif /* ALLOW_BORG */


/*
 * Quit the game.
 */
void do_cmd_quit(cmd_code code, cmd_arg args[])
{
	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;
}


/*
 * Handle a mouseclick.
 */
static void do_cmd_mouseclick(void)
{
	int x, y;

	if (!OPT(mouse_movement)) return;

	y = KEY_GRID_Y(p_ptr->command_cmd_ex);
	x = KEY_GRID_X(p_ptr->command_cmd_ex);

	/* Check for a valid location */
	if (!in_bounds_fully(y, x)) return;

	/* XXX We could try various things here like going up/down stairs */
	if ((p_ptr->py == y) && (p_ptr->px == x) /* && (p_ptr->command_cmd_ex.mousebutton) */)
	{
		textui_cmd_rest();
	}
	else /* if (p_ptr->command_cmd_ex.mousebutton == 1) */
	{
		if (p_ptr->timed[TMD_CONFUSED])
		{
			cmd_insert(CMD_WALK, DIR_UNKNOWN);
		}
		else
		{
			cmd_insert(CMD_PATHFIND, y, x);
		}
	}
	/*
	else if (p_ptr->command_cmd_ex.mousebutton == 2)
	{
		target_set_location(y, x);
		msg_print("Target set.");
	}
	*/
}


/*
 * Port-specific options
 *
 * Should be moved to the options screen. XXX
 */
static void do_cmd_port(void)
{
	(void)Term_user(0);
}


/*
 * Display the options and redraw afterward.
 */
static void do_cmd_xxx_options(void)
{
	do_cmd_options();
	do_cmd_redraw();
}


/*
 * Display the main-screen monster list.
 */
static void do_cmd_monlist(void)
{
	/* Save the screen and display the list */
	screen_save();
	display_monlist();

	/* Wait */
	anykey();

	/* Return */
	screen_load();
}


/*
 * Display the main-screen item list.
 */
static void do_cmd_itemlist(void)
{
	/* Save the screen and display the list */
	screen_save();
	display_itemlist();

	/* Wait */
	anykey();

	/* Return */
	screen_load();
}


/*
 * Invoked when the command isn't recognised.
 */
static void do_cmd_unknown(void)
{
	prt("Type '?' for help.", 0, 0);
}




/* List indexed by char */
struct {
	do_cmd_type *hook;
	cmd_code cmd;
} converted_list[UCHAR_MAX+1];


/*** Menu functions ***/

/* Display an entry on a command menu */
static void cmd_sub_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);
	const command_type *commands = menu_priv(menu);

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

	ui_event_data evt;
	command_type *selection = selection_p;

	/* Set up th emenu */
	menu_init(&menu, MN_SKIN_SCROLL, &commands_menu);
	menu_setpriv(&menu, list->len, list->list);
	menu_layout(&menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(21, 3, 62, 17);

	/* Select an entry */
	evt = menu_select(&menu, 0);

	/* Load de screen */
	screen_load();

	if (evt.type == EVT_SELECT)
		*selection = list->list[menu.cursor];
	else if (evt.type == EVT_ESCAPE)
		return FALSE;

	return TRUE;
}



static bool cmd_list_action(menu_type *m, const ui_event_data *event, int oid)
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

/*
 * Display a list of command types, allowing the user to select one.
 */
static void do_cmd_menu(void)
{
	menu_type menu;
	menu_iter commands_menu = { NULL, NULL, cmd_list_entry, cmd_list_action, NULL };
	region area = { 21, 5, 37, 6 };

	command_type chosen_command = { 0 };

	/* Set up the menu */
	menu_init(&menu, MN_SKIN_SCROLL, &commands_menu);
	menu_setpriv(&menu, N_ELEMENTS(cmds_all) - 1, &chosen_command);
	menu_layout(&menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(19, 4, 58, 11);

	/* Select an entry */
	menu_select(&menu, 0);

	/* Load de screen */
	screen_load();

	/* If a command was chosen, do it */
	if (chosen_command.cmd != CMD_NULL)
		cmd_insert(chosen_command.cmd);
	else if (chosen_command.hook)
		chosen_command.hook();
}


/*** Exported functions ***/

/*
 * Initialise the command list.
 */
void cmd_init(void)
{
	size_t i, j;

	/* Go through all the lists of commands */
	for (j = 0; j < N_ELEMENTS(cmds_all); j++)
	{
		command_type *commands = cmds_all[j].list;

		/* Fill everything in */
		for (i = 0; i < cmds_all[j].len; i++)
		{
			unsigned char key = commands[i].key;

			/* Note: at present converted_list is UCHAR_MAX + 1 
			   large, so 'key' is always a valid index. */
			converted_list[key].hook = commands[i].hook;
			converted_list[key].cmd = commands[i].cmd;
		}
	}

	/* Fill in the rest */
	for (i = 0; i < N_ELEMENTS(converted_list); i++)
	{
		switch (i)
		{
			/* Ignore */
			case ESCAPE:
			case ' ':
			case '\a':
			{
				break;
			}

			default:
			{
				if (!converted_list[i].hook && !converted_list[i].cmd)
				{
					converted_list[i].hook = do_cmd_unknown;
					converted_list[i].cmd = CMD_NULL;
				}
			}
		}		
	}
}


/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void textui_process_command(bool no_request)
{
	if (!no_request)
		request_command();

	/* Handle repeating the last command */
	/* repeat_check();*/

	/* Handle resize events XXX */
	if (p_ptr->command_cmd_ex.type == EVT_RESIZE)
	{
		do_cmd_redraw();
	}
	else if (p_ptr->command_cmd_ex.type == EVT_MOUSE)
	{
		do_cmd_mouseclick();
	}
	else
	{
		/* Within these boundaries, the cast to unsigned char will have the desired effect */
		assert(p_ptr->command_cmd >= CHAR_MIN && p_ptr->command_cmd <= CHAR_MAX);
		/* Execute the command */
		if (converted_list[(unsigned char) p_ptr->command_cmd].cmd != CMD_NULL)
			cmd_insert_repeated(converted_list[(unsigned char) p_ptr->command_cmd].cmd, p_ptr->command_arg);

		else if (converted_list[(unsigned char) p_ptr->command_cmd].hook)
			converted_list[(unsigned char) p_ptr->command_cmd].hook();
	}
}
