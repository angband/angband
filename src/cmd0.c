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
#include "macro.h"
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


static void do_cmd_menu(void);


/*** Handling bits ***/

/*
 * Holds a generic command - if cmd is set to other than CMD_NULL 
 * it simply pushes that command to the game, otherwise the hook 
 * function will be called.
 */
struct generic_command
{
	const char *desc;
	unsigned char key;
	cmd_code cmd;
	void (*hook)(void);
	bool (*prereq)(void);
};

/* Item selector type (everything required for get_item()) */
struct item_selector
{
	const char *prompt;
	const char *noop;

	bool (*filter)(const object_type *o_ptr);
	int mode;
};

/* Item "action" type */
struct item_command
{
	struct generic_command base;
	struct item_selector selector;

	void (*action)(object_type *, int);
	const char *id;
	bool needs_aim;
};


/* All possible item actions */
static struct item_command item_actions[] =
{
	/* Not setting IS_HARMLESS for this one because it could cause a true
	 * dangerous command to not be prompted, later.
	 */
	{ { "Uninscribe an object", '}', CMD_UNINSCRIBE, NULL, NULL },
	  { "Un-inscribe which item? ", "You have nothing to un-inscribe.",
	    obj_has_inscrip, (USE_EQUIP | USE_INVEN | USE_FLOOR) },
	  NULL, "uninscribe", FALSE },

	{ { "Inscribe an object", '{', CMD_NULL, NULL, NULL },
	  { "Inscribe which item? ", "You have nothing to inscribe.",
	    NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS) },
	  textui_obj_inscribe, "inscribe", FALSE },

	{ { "Examine an item", 'I', CMD_NULL, NULL, NULL },
	  { "Examine which item? ", "You have nothing to examine.",
	    NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS) },
	  textui_obj_examine, "examine", FALSE },

	/*** Takeoff/drop/wear ***/
	{ { "Take/unwield off an item", 't', CMD_TAKEOFF, NULL, NULL },
	  { "Take off which item? ", "You are not wearing anything you can take off.",
	    obj_can_takeoff, USE_EQUIP },
	  NULL, "takeoff", FALSE },

	{ { "Wear/wield an item", 'w', CMD_WIELD, NULL, NULL },
	  { "Wear/Wield which item? ", "You have nothing you can wear or wield.",
	    obj_can_wear, (USE_INVEN | USE_FLOOR) },
	  textui_obj_wield, "wield", FALSE },

	{ { "Drop an item", 'd', CMD_NULL, NULL, NULL },
	  { "Drop which item? ", "You have nothing to drop.",
	    NULL, (USE_EQUIP | USE_INVEN) },
	  textui_obj_drop, "drop", FALSE },

	/*** Attacking ***/
	{ { "Fire your missile weapon", 'f', CMD_FIRE, NULL, player_can_fire },
	  { "Fire which item? ", "You have nothing to fire.",
	    obj_can_fire, (USE_INVEN | USE_EQUIP | USE_FLOOR | QUIVER_TAGS) },
	  NULL, "fire", TRUE },

	/*** Spellbooks ***/
	{ { "Browse a book", 'b', CMD_NULL, NULL, NULL },
	  { "Browse which book? ", "You have no books that you can read.",
	    obj_can_browse, (USE_INVEN | USE_FLOOR | IS_HARMLESS) },
	  textui_spell_browse, "browse", FALSE },

	{ { "Gain new spells", 'G', CMD_NULL, NULL, player_can_study },
	  { "Study which book? ", "You have no books that you can read.",
	    obj_can_study, (USE_INVEN | USE_FLOOR) },
	  textui_obj_study, "study", FALSE },

	{ { "Cast a spell", 'm', CMD_NULL, NULL, player_can_cast },
	  { "Use which book? ", "You have no books that you can read.",
	    obj_can_cast_from, (USE_INVEN | USE_FLOOR) } ,
	  textui_obj_cast, "cast", FALSE },

	/*** Item usage ***/
	{ { "Use a staff", 'u', CMD_USE_STAFF, NULL, NULL },
	  { "Use which staff? ", "You have no staff to use.",
	    obj_is_staff, (USE_INVEN | USE_FLOOR | SHOW_FAIL) },
	  NULL, "use", TRUE,},

	{ { "Aim a wand", 'a', CMD_USE_WAND, NULL, NULL },
	  { "Aim which wand? ", "You have no wand to aim.",
	    obj_is_wand, (USE_INVEN | USE_FLOOR | SHOW_FAIL) },
	  NULL, "aim", TRUE,},

	{ { "Zap a rod", 'z', CMD_USE_ROD, NULL, NULL },
	  { "Zap which rod? ", "You have no charged rods to zap.",
	    obj_is_rod, (USE_INVEN | USE_FLOOR | SHOW_FAIL) },
	  NULL, "zap", TRUE },

	{ { "Activate an object", 'A', CMD_ACTIVATE, NULL, NULL },
	  { "Activate which item? ", "You have nothing to activate.",
	    obj_is_activatable, (USE_EQUIP | SHOW_FAIL) },
	  NULL, "activate", TRUE },

	{ { "Eat some food", 'E', CMD_EAT, NULL, NULL },
	  { "Eat which item? ", "You have nothing to eat.",
	    obj_is_food, (USE_INVEN | USE_FLOOR) },
	  NULL, "eat", FALSE },

	{ { "Quaff a potion", 'q', CMD_QUAFF, NULL, NULL },
	  { "Quaff which potion? ", "You have no potions to quaff.",
	    obj_is_potion, (USE_INVEN | USE_FLOOR) },
	  NULL, "quaff", TRUE },

	{ { "Read a scroll", 'r', CMD_READ_SCROLL, NULL, player_can_read },
	  { "Read which scroll? ", "You have no scrolls to read.",
	    obj_is_scroll, (USE_INVEN | USE_FLOOR) },
	  NULL, "read", TRUE },

	{ { "Fuel your light source", 'F', CMD_REFILL, NULL, NULL },
	  { "Refuel with what fuel source? ", "You have nothing to refuel with.",
	    obj_can_refill, (USE_INVEN | USE_FLOOR) },
	  NULL, "refill", FALSE },
};




/* General actions */
static struct generic_command cmd_action[] =
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
	{ "Bash a door open",           'B', CMD_NULL, textui_cmd_bash },
	{ "Fire at nearest target",   'h', CMD_NULL, textui_cmd_fire_at_nearest },
	{ "Throw an item",            'v', CMD_NULL, textui_cmd_throw }
};

/* Item management commands */
static struct generic_command cmd_item_manage[] =
{
	{ "Display equipment listing", 'e', CMD_NULL, do_cmd_equip },
	{ "Display inventory listing", 'i', CMD_NULL, do_cmd_inven },
	{ "Pick up objects",           'g', CMD_PICKUP, NULL },
	{ "Destroy an item",           'k', CMD_NULL, textui_cmd_destroy },
	
};

/* Information access commands */
static struct generic_command cmd_info[] =
{
	{ "Full dungeon map",             'M', CMD_NULL, do_cmd_view_map },
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
static struct generic_command cmd_util[] =
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
static struct generic_command cmd_hidden[] =
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


static struct generic_command cmd_item_use[N_ELEMENTS(item_actions)];



/*
 * A categorised list of all the command lists.
 */
typedef struct
{
	const char *name;
	struct generic_command *list;
	size_t len;
} command_list;

static command_list cmds_all[] =
{
	{ "Use item",        cmd_item_use,    N_ELEMENTS(item_actions) },
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
	const struct generic_command *commands = menu_priv(menu);

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
	struct generic_command *selection = selection_p;

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

	return FALSE;
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
static void do_cmd_menu(void)
{
	region area = { 21, 5, 37, 6 };

	struct generic_command chosen_command = { 0 };

	if (!command_menu)
		command_menu = menu_new(MN_SKIN_SCROLL, &command_menu_iter);

	menu_setpriv(command_menu, N_ELEMENTS(cmds_all) - 1, &chosen_command);
	menu_layout(command_menu, &area);

	/* Set up the screen */
	screen_save();
	window_make(19, 4, 58, 11);

	menu_select(command_menu, 0);

	screen_load();

	/* If a command was chosen, execute it */
	if (chosen_command.key)
		Term_keypress(chosen_command.key);
}


/*** Exported functions ***/

/* List indexed by char */
struct command
{
	struct generic_command *command;

	bool is_object;
	struct item_command *item;	
};

static struct command converted_list[UCHAR_MAX+1];


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
		struct generic_command *commands = cmds_all[j].list;

		/* Fill everything in */
		for (i = 0; i < cmds_all[j].len; i++)
			converted_list[commands[i].key].command = &commands[i];
	}

	/* Fill in item actions */
	for (j = 0; j < N_ELEMENTS(item_actions); j++)
	{
		struct item_command *act = &item_actions[j];
		unsigned char key = act->base.key;

		converted_list[key].command = &act->base;
		converted_list[key].is_object = TRUE;
		converted_list[key].item = act;

		/* Also update the action menus */
		memcpy(&cmd_item_use[j], &act->base, sizeof(cmd_item_use[0]));
	}
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
		ui_event_data ke;

		prt(format("Count: %d", count), 0, 0);

		ke = inkey_ex();
		if (ke.type != EVT_KBRD)
			continue;

		if (ke.key == ESCAPE)
			return -1;

		/* Simple editing (delete or backspace) */
		else if (ke.key == 0x7F || ke.key == KTRL('H'))
			count = count / 10;

		/* Actual numeric data */
		else if (isdigit((unsigned char) ke.key))
		{
			count = count * 10 + D2I(ke.key);

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
			if (ke.key != '\n' && ke.key != '\r')
				Term_event_push(&ke);

			break;
		}
	}

	return count;
}



/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static char request_command_buffer[256];


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
static ui_event_data textui_get_command(void)
{
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	char tmp[2] = { '\0', '\0' };

	ui_event_data ke = EVENT_EMPTY;

	cptr act = NULL;



	/* Get command */
	while (1)
	{
		/* Hack -- no flush needed */
		msg_flag = FALSE;

		/* Activate "command mode" */
		inkey_flag = TRUE;

		/* Get a command */
		ke = inkey_ex();


		/* Command Count */
		if (ke.key == '0')
		{
			int count = textui_get_count();

			if (count == -1 || !get_com_ex("Command: ", &ke))
				continue;
			else
				p_ptr->command_arg = count;
		}

		/* Allow "keymaps" to be bypassed */
		else if (ke.key == '\\')
		{
			/* Get a real command */
			(void)get_com("Command: ", &ke.key);

			/* Hack -- bypass keymaps */
			if (!inkey_next) inkey_next = "";
		}

		/* Allow "control chars" to be entered */
		else if (ke.key == '^')
		{
			/* Get a new command and controlify it */
			if (get_com("Control: ", &ke.key))
				ke.key = KTRL(ke.key);
		}

		/* Special case for the arrow keys */
		else if (isarrow(ke.key))
		{
			switch (ke.key)
			{
				case ARROW_DOWN:    ke.key = '2'; break;
				case ARROW_LEFT:    ke.key = '4'; break;
				case ARROW_RIGHT:   ke.key = '6'; break;
				case ARROW_UP:      ke.key = '8'; break;
			}
		}

		/* Erase the message line */
		prt("", 0, 0);


		if (ke.type == EVT_BUTTON)
		{
			/* Buttons are always specified in standard keyset */
			act = tmp;
			tmp[0] = ke.key;
		}
		else if (ke.type == EVT_KBRD)
		{
			/* Look up applicable keymap */
			act = keymap_act[mode][(byte)(ke.key)];
		}

		/* Apply keymap if not inside a keymap already */
		if (ke.key && act && !inkey_next)
		{
			/* Install the keymap */
			my_strcpy(request_command_buffer, act,
			          sizeof(request_command_buffer));

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
static void textui_process_click(ui_event_data e)
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
			cmd_insert(CMD_WALK, DIR_UNKNOWN);
		else
			cmd_insert(CMD_PATHFIND, y, x);
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
		if (!o_ptr->k_idx) continue;

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
static bool textui_process_key(unsigned char c)
{
	struct command *cmd = &converted_list[c];
	struct generic_command *command = cmd->command;

	if (c == ESCAPE || c == ' ' || c == '\a')
		return TRUE;

	if (!command)
		return FALSE;

	if (key_confirm_command(c) &&
			(!command->prereq || command->prereq()))
	{
		if (cmd->is_object)
		{
			struct item_command *act = cmd->item;
			int item;
			object_type *o_ptr;
		
			/* Get item */
			item_tester_hook = act->selector.filter;
			if (!get_item(&item, act->selector.prompt,
					act->selector.noop, c, act->selector.mode))
				return TRUE;

			/* Get the item */
			o_ptr = object_from_item_idx(item);
		
			/* Execute the item command */
			if (act->action != NULL)
				act->action(o_ptr, item);
			else if (act->needs_aim && obj_needs_aim(o_ptr))
				cmd_insert(command->cmd, item, DIR_UNKNOWN);
			else
				cmd_insert(command->cmd, item);
		}
		else
		{
			if (command->cmd != CMD_NULL)
				cmd_insert_repeated(command->cmd, p_ptr->command_arg);

			else if (command->hook)
				command->hook();
		}
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
	ui_event_data e = textui_get_command();

	/* Reset argument */
	p_ptr->command_arg = 0;

	if (e.type == EVT_RESIZE)
		do_cmd_redraw();

	else if (e.type == EVT_MOUSE)
		textui_process_click(e);

	else if (e.type == EVT_KBRD)
		done = textui_process_key(e.key);

	if (!done)
		do_cmd_unknown();
}
