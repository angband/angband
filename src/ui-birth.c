/*
 * File: ui-birth.c
 * Purpose: Text-based user interface for character creation
 *
 * Copyright (c) 1987 - 2007 Angband contributors
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
#include "ui-menu.h"
#include "ui-birth.h"
#include "game-event.h"
#include "game-cmd.h"

/* Two local-to-this-file globals to hold a bit of state between messages
   and command requests from the game proper. Probably not strictly necessary,
   but they reduce code complexity a bit. */
static enum birth_stage current_stage = BIRTH_METHOD_CHOICE;
static int autoroller_maxes[A_MAX];

/* ------------------------------------------------------------------------
 * Quickstart? screen.
 * ------------------------------------------------------------------------ */
static game_command quickstart_question(void)
{
	char ch;
	ui_event_data ke;
	game_command cmd = { CMD_NULL, 0, {0}};

	/* Prompt */
	while (cmd.command == CMD_NULL)
	{
		put_str("Quick-start character based on previous one (y/n)? ", 2, 2);

		/* Buttons */
		button_kill_all();
		button_add("[Exit]", KTRL('X'));
		button_add("[ESC]", ESCAPE);
		button_add("[y]", 'y');
		button_add("[n]", 'n');
		button_add("[Help]", '?');
		
		ke = inkey_ex();
		ch = ke.key;
		
		if (ch == KTRL('X'))
		{
			cmd.command = CMD_QUIT;
		}
		else if (strchr("Nn\r\n", ch))
		{
			cmd.command = CMD_BIRTH_CHOICE;
			cmd.params.choice = 0; /* FIXME */
		}
		else if (strchr("Yy", ch))
		{
			cmd.command = CMD_BIRTH_CHOICE;
			cmd.params.choice = 1; /* FIXME */
		}
		else if (ch == '?')
			(void)show_file("birth.hlp", NULL, 0, 0);
		else
			bell("Illegal answer!");
	}
	
	return cmd;
}


/* ------------------------------------------------------------------------
 * The various "menu" bits of the birth process - namely choice of sex,
 * race, class, and roller type.
 * ------------------------------------------------------------------------ */
static menu_type *current_menu = NULL;

/* Locations of the menus, etc. on the screen */
#define HEADER_ROW       1
#define QUESTION_ROW     7
#define TABLE_ROW       10

#define QUESTION_COL     2
#define SEX_COL          2
#define RACE_COL        14
#define RACE_AUX_COL    29
#define CLASS_COL       29
#define CLASS_AUX_COL   50

static region gender_region = {SEX_COL, TABLE_ROW, 15, -2};
static region race_region = {RACE_COL, TABLE_ROW, 15, -2};
static region class_region = {CLASS_COL, TABLE_ROW, 19, -2};
static region roller_region = {44, TABLE_ROW, 21, -2};

/* The various menus */
menu_type sex_menu, race_menu, class_menu, roller_menu;

/* We have one of these structures for each menu we display - it holds
   the useful information for the menu - text of the menu items, "help"
   text, current (or default) selection, and whether random selection
   is allowed. */
struct birthmenu_data 
{
	int selection;
	const char **items;
	const char **help;
	const char *hint;
	bool allow_random;
};

/*
 * Clear the previous question
 */
static void clear_question(void)
{
	int i;

	for (i = QUESTION_ROW; i < TABLE_ROW; i++)
	{
		/* Clear line, position cursor */
		Term_erase(0, i, 255);
	}
}


#define BIRTH_MENU_HELPTEXT \
	"{lightblue}Please select your character from the menu below:{/}\n\n" \
	"Use the {lightgreen}movement keys{/} to scroll the menu, " \
	"{lightgreen}Enter{/} to select the current menu item, '{lightgreen}*{/}' " \
	"for a random menu item, '{lightgreen}ESC{/}' to step back through the " \
	"birth process, '{lightgreen}={/}' for the birth options, '{lightgreen}?{/} " \
	"for help, or '{lightgreen}Ctrl-X{/}' to quit."

/* Show the birth instructions on an otherwise blank screen */	
static void print_menu_instructions(void)
{
	/* Clear screen */
	Term_clear();
	
	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = QUESTION_COL;
	Term_gotoxy(QUESTION_COL, HEADER_ROW);
	
	/* Display some helpful information */
	text_out_e(BIRTH_MENU_HELPTEXT);
	
	/* Reset text_out() indentation */
	text_out_indent = 0;
}

/* Allow the user to select from the current menu, and return the 
   corresponding command to the game.  Some actions are handled entirely
   by the UI (displaying help text, for instance). */
static game_command menu_question(void)
{
	/* Note: the const here is just to quell a compiler warning. */
	struct birthmenu_data *menu_data = current_menu->menu_data;
	int cursor = menu_data->selection;
	game_command cmd = { CMD_NULL, 0, {0}};
	ui_event_data cx;
	
	/* Print the question currently being asked. */
	clear_question();
	Term_putstr(QUESTION_COL, QUESTION_ROW, -1, TERM_YELLOW, menu_data->hint);

	current_menu->cmd_keys = "?=*\r\n\x18";	 /* ?, ,= *, \n, <ctl-X> */

	while (cmd.command == CMD_NULL)
	{
		/* Display the menu, wait for a selection of some sort to be made. */
		cx = menu_select(current_menu, &cursor, EVT_CMD);

		/* As all the menus are displayed in "hierarchical" style, we allow
		   use of "back" (left arrow key or equivalent) to step back in 
		   the proces as well as "escape". */
		if (cx.type == EVT_BACK || cx.type == EVT_ESCAPE)
		{
			cmd.command = CMD_BIRTH_BACK;
		}
		/* '\xff' is a mouse selection, '\r' a keyboard one. */
		else if (cx.key == '\xff' || cx.key == '\r') 
		{
			cmd.command = CMD_BIRTH_CHOICE;
			cmd.params.choice = cursor;
		}
		/* '*' chooses an option at random from those the game's provided. */
		else if (cx.key == '*' && menu_data->allow_random) 
		{
			cmd.command = CMD_BIRTH_CHOICE;
			current_menu->cursor = randint0(current_menu->count);
			cmd.params.choice = current_menu->cursor;
			menu_refresh(current_menu);
		}
		else if (cx.key == '=') 
		{
			cmd.command = CMD_OPTIONS;
		}
		else if (cx.key == KTRL('X')) 
		{
			cmd.command = CMD_QUIT;
		}
	}
	
	return cmd;
}

/* A custom "display" function for our menus that simply displays the
   text from our stored data in a different colour if it's currently
   selected. */
static void birthmenu_display(menu_type *menu, int oid, bool cursor,
			      int row, int col, int width)
{
	struct birthmenu_data *data = menu->menu_data;

	byte attr = curs_attrs[CURS_KNOWN][0 != cursor];
	c_put_str(attr, data->items[oid], row, col);
}

/* We defer the choice of actual actions until outside of the menu API 
   in menu_question(), so this can be a reasonably simple function
   for when a menu "command" is activated. */
static bool birthmenu_handler(char cmd, void *db, int oid)
{
	return TRUE;
}

/* Our custom menu iterator, only really needed to allow us to override
   the default handling of "commands" in the standard iterators (hence
   only defining the display and handler parts). */
static const menu_iter birth_iter = { NULL, NULL, birthmenu_display, birthmenu_handler };

/* Cleans up our stored menu info when we've finished with it. */
static void free_birth_menu(menu_type *menu)
{
	struct birthmenu_data *data = menu->menu_data;

	if (data)
	{
		mem_free(data->items);
		mem_free(data->help);
		mem_free(data);
	}
}

/* We use different menu "browse functions" to display the help text
   sometimes supplied with the menu items - currently just the list
   of bonuses, etc, corresponding to each race and class. */
typedef void (*browse_f) (int oid, void *, const region *loc);

static void race_help(int i, void *data, const region *loc)
{
	struct birthmenu_data *menu_data = data;

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = RACE_AUX_COL;
	Term_gotoxy(RACE_AUX_COL, TABLE_ROW);
	
	text_out_e("%s", menu_data->help[i]);
	
	/* Reset text_out() indentation */
	text_out_indent = 0;
}

static void class_help(int i, void *data, const region *loc)
{
	struct birthmenu_data *menu_data = data;

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = CLASS_AUX_COL;
	Term_gotoxy(CLASS_AUX_COL, TABLE_ROW);
	
	text_out_e("%s", menu_data->help[i]);
	
	/* Reset text_out() indentation */
	text_out_indent = 0;
}

/* Set up one of our menus ready to display choices for a birth question.
   This is slightly involved. */
static void init_birth_menu(menu_type *menu, game_event_data *data, const region *reg, bool allow_random, browse_f aux)
{
	struct birthmenu_data *menu_data;
	int i;

	/* Get rid of the previous incarnation of this menu. */
	free_birth_menu(menu);
	
	/* A couple of behavioural flags - we want selections letters in
	   lower case and a double tap to act as a selection. */
	menu->selections = lower_case;
	menu->flags = MN_DBL_TAP;

	/* Set the number of choices in the menu to the same as the game
	   has told us we've got to offer. */
	menu->count = data->birthstage.n_choices;

	/* Allocate sufficient space for our own bits of menu information. */
	menu_data = mem_alloc(sizeof *menu_data);

	/* Copy across the game's suggested initial selection, etc. */
	menu_data->selection = data->birthstage.initial_choice;
	menu_data->allow_random = allow_random;

	/* Allocate space for an array of menu item texts and help texts
	   (where applicable) */
	menu_data->items = mem_alloc(menu->count * sizeof *menu_data->items);

	if (data->birthstage.helptexts)
		menu_data->help = mem_alloc(menu->count * sizeof *menu_data->items);
	else
		menu_data->help = NULL;

	/* Make sure we have the appropriate menu text and help text in arrays.
	   The item text, helptext, etc. are guaranteed to no persistent 
	   throughout the birth process (though not beyond), so we can 
	   just point to it, having no wish to display it after that. */
	for (i = 0; i < menu->count; i++)
	{	
		menu_data->items[i] = data->birthstage.choices[i];

		if (menu_data->help)
			menu_data->help[i] = data->birthstage.helptexts[i];
	}

	/* Help text for the menu as a whole (also guaranteed persistent. */
	menu_data->hint = data->birthstage.hint;

	/* Poke our menu data in to the assigned slot in the menu structure. */
	menu->menu_data = menu_data;

	/* Set up the "browse" hook to display help text (where applicable). */
	menu->browse_hook = aux;

	/* Get ui-menu to initialise whatever it wants to to give us a scrollable
	   menu. */
	menu_init(menu, MN_SKIN_SCROLL, &birth_iter, reg);
}

/* ------------------------------------------------------------------------
 * Autoroller-based stat allocation. 
 * ------------------------------------------------------------------------ */

static bool minstat_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	if (keypress == KTRL('x'))
		quit(NULL);

	return askfor_aux_keypress(buf, buflen, curs, len, keypress, firsttime);
}

#define AUTOROLLER_HELPTEXT \
  "The auto-roller will automatically ignore characters which do not " \
  "meet the minimum values for any stats specified below.\n" \
  "Note that stats are not independent, so it is not possible to get " \
  "perfect (or even high) values for all your stats."

static void autoroller_start(int stat_maxes[A_MAX])
{
	int i;
	char inp[80];
	char buf[80];

	/* Clear */
	Term_clear();

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = 5;
	text_out_wrap = 75;

	Term_gotoxy(5, 10);	
	text_out_e("%s", AUTOROLLER_HELPTEXT);	

	/* Reset text_out() indentation */
	text_out_indent = 0;
	text_out_wrap = 0;

	/* Prompt for the minimum stats */
	put_str("Enter minimum value for: ", 15, 2);
	
	/* Output the maximum stats */
	for (i = 0; i < A_MAX; i++)
	{
		int m = stat_maxes[i];
		autoroller_maxes[i] = stat_maxes[i];

		/* Extract a textual format */
		/* cnv_stat(m, inp, sizeof(buf); */
		
		/* Above 18 */
		if (m > 18)
		{
			strnfmt(inp, sizeof(inp), "(Max of 18/%02d):", (m - 18));
		}
		
		/* From 3 to 18 */
		else
		{
			strnfmt(inp, sizeof(inp), "(Max of %2d):", m);
		}
		
		/* Prepare a prompt */
		strnfmt(buf, sizeof(buf), "%-5s%-20s", stat_names[i], inp);
		
		/* Dump the prompt */
		put_str(buf, 16 + i, 5);
	}
}

static game_command autoroller_command(void)
{
	int i, v;
	char inp[80];

	game_command cmd = { CMD_NULL, 0, {0} };

	/* Input the minimum stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Get a minimum stat */
		while (TRUE)
		{
			char *s;
			
			/* Move the cursor */
			put_str("", 16 + i, 30);
			
			/* Default */
			inp[0] = '\0';
			
			/* Get a response (or escape) */
			if (!askfor_aux(inp, 9, minstat_keypress)) 
			{
				if (i == 0) 
				{
					/* Back a step */
					cmd.command = CMD_BIRTH_BACK;
					return cmd;
				}
				else 
				{
					/* Repeat this step */
					return cmd;
				}
			}
			
			/* Hack -- add a fake slash */
			my_strcat(inp, "/", sizeof(inp));
			
			/* Hack -- look for the "slash" */
			s = strchr(inp, '/');
			
			/* Hack -- Nuke the slash */
			*s++ = '\0';
			
			/* Hack -- Extract an input */
			v = atoi(inp) + atoi(s);
			
			/* Break on valid input */
			if (v <= autoroller_maxes[i]) break;
		}
		
		/* Save the minimum stat */
		cmd.params.stat_limits[i] = (v > 0) ? v : 0;
	}

	cmd.command = CMD_AUTOROLL;
	return cmd;
}

/* ------------------------------------------------------------------------
 * The rolling bit of the autoroller/simple roller.
 * ------------------------------------------------------------------------ */
#define ROLLERCOL 42
static bool prev_roll = FALSE;

static void roller_newchar(game_event_type type, game_event_data *data, void *user)
{
   	/* Display the player - a cheat really, given the context. */
	display_player(0);

	/* Non-zero if we've got a previous character to swap with. */
	prev_roll = data->birthstats.remaining;

	Term_fresh();
}

/* 
   Handles the event we get when the autoroller is looking for a suitable
   character but hasn't found one yet.
*/
static void roller_autoroll(game_event_type type, game_event_data *data, void *user)
{
	int col = ROLLERCOL;
	int i;
	char buf[80];

	/* Label */
	put_str(" Limit", 2, col+5);
	
	/* Label */
	put_str("  Freq", 2, col+13);
	
	/* Label */
	put_str("  Roll", 2, col+24);

	/* Put the minimal stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Label stats */
		put_str(stat_names[i], 3+i, col);
		
		/* Put the stat */
		cnv_stat(data->birthautoroll.limits[i], buf, sizeof(buf));
		c_put_str(TERM_L_BLUE, buf, 3+i, col+5);
	}

	/* Label count */
	put_str("Round:", 9, col+13);
	
	/* You can't currently interrupt the autoroller */
/*	put_str("(Hit ESC to stop)", 12, col+13); */

	/* Put the stats (and percents) */
	for (i = 0; i < A_MAX; i++)
	{
		/* Put the stat */
		cnv_stat(data->birthautoroll.current[i], buf, sizeof(buf));
		c_put_str(TERM_L_GREEN, buf, 3+i, col+24);
		
		/* Put the percent */
		if (data->birthautoroll.matches[i])
		{
			int p = 1000L * data->birthautoroll.matches[i] / data->birthautoroll.round;
			byte attr = (p < 100) ? TERM_YELLOW : TERM_L_GREEN;
			strnfmt(buf, sizeof(buf), "%3d.%d%%", p/10, p%10);
			c_put_str(attr, buf, 3+i, col+13);
		}
		
		/* Never happened */
		else
		{
			c_put_str(TERM_RED, "(NONE)", 3+i, col+13);
		}
	}
	
	/* Dump round */
	put_str(format("%10ld", data->birthautoroll.round), 9, col+20);
	
	/* Make sure they see everything */
	Term_fresh();
}

static void roller_start(int stat_maxes[A_MAX])
{
	prev_roll = FALSE;
	Term_clear();

	event_add_handler(EVENT_BIRTHAUTOROLLER, roller_autoroll, NULL);	
	event_add_handler(EVENT_BIRTHSTATS, roller_newchar, NULL);	
}

static game_command roller_command(void)
{
	game_command cmd = { CMD_NULL, 0, {0} };
	ui_event_data ke;
	char ch;

	/* bool prev_roll is a static global that's reset when we enter the
	   roller */

	/* Add buttons */
	button_add("[ESC]", ESCAPE);
	button_add("[Enter]", '\r');
	button_add("[r]", 'r');
	if (prev_roll) button_add("[p]", 'p');
	clear_from(Term->hgt - 2);
	redraw_stuff();

	/* Prepare a prompt (must squeeze everything in) */
	Term_gotoxy(2, 23);
	Term_addch(TERM_WHITE, '[');
	Term_addstr(-1, TERM_WHITE, "'r' to reroll");
	if (prev_roll) Term_addstr(-1, TERM_WHITE, ", 'p' for prev");
	Term_addstr(-1, TERM_WHITE, ", or 'Enter' to accept");
	Term_addch(TERM_WHITE, ']');
	
	/* Prompt and get a command */
	ke = inkey_ex();
	ch = ke.key;

	if (ch == ESCAPE) 
	{
		button_kill('r');
		button_kill('p');

		cmd.command = CMD_BIRTH_BACK;
	}

	/* 'Enter' accepts the roll */
	if ((ch == '\r') || (ch == '\n')) 
	{
		cmd.command = CMD_ACCEPT_STATS;
	}

	/* Reroll this character */
	if ((ch == ' ') || (ch == 'r'))
	{
		cmd.command = CMD_ROLL;
	}

	/* Previous character */
	if (prev_roll && (ch == 'p'))
	{
		cmd.command = CMD_PREV_STATS;
	}

	if (ch == KTRL('X')) 
	{
		cmd.command = CMD_QUIT;
	}
	
	/* Nothing handled directly here */
	if (cmd.command == CMD_NULL)
	{
		/* Help XXX */
		if (ch == '?')
			do_cmd_help();
		else
			bell("Illegal auto-roller command!");
	}

	/* Kill buttons */
	button_kill(ESCAPE);
	button_kill('\r');
	button_kill('r');
	button_kill('p');
	redraw_stuff();

	return cmd;
}

static void roller_stop(void)
{
	event_remove_handler(EVENT_BIRTHAUTOROLLER, roller_autoroll, NULL);	
	event_remove_handler(EVENT_BIRTHSTATS, roller_newchar, NULL);	
}


/* ------------------------------------------------------------------------
 * Point-based stat allocation.
 * ------------------------------------------------------------------------ */

/* The locations of the "costs" area on the birth screen. */
#define COSTS_ROW 2
#define COSTS_COL (42 + 32)

/* This is called whenever a stat changes.  We take the easy road, and just
   redisplay them all using the standard function. */
static void point_based_stats(game_event_type type, game_event_data *data, void *user)
{
	display_player_stat_info();
}

/* This is called whenever any of the other miscellaneous stat-dependent things
   changed.  We are hooked into changes in the amount of gold in this case,
   but redisplay everything because it's easier. */
static void point_based_misc(game_event_type type, game_event_data *data, void *user)
{
	display_player_xtra_info();
}

/* This is called whenever the points totals are changed (in birth.c), so
   that we can update our display of how many points have been spent and
   are available. */
static void point_based_points(game_event_type type, game_event_data *data, void *user)
{
	int i;
	int sum = 0;

	/* Display the costs header */
	put_str("Cost", COSTS_ROW - 1, COSTS_COL);
	
	/* Display the costs */
	for (i = 0; i < A_MAX; i++)
	{
		/* Display cost */
		put_str(format("%4d", data->birthstats.stats[i]),
			COSTS_ROW + i, COSTS_COL);

		sum += data->birthstats.stats[i];
	}
	
	prt(format("Total Cost %2d/%2d.  Use 2/8 to move, 4/6 to modify, 'Enter' to accept.", sum, data->birthstats.remaining + sum), 0, 0);
}


static void point_based_start(void)
{
	/* Clear */
	Term_clear();

	/* Display the player */
	display_player_xtra_info();
	display_player_stat_info();

	/* Register handlers for various events - cheat a bit because we redraw
	   the lot at once rather than each bit at a time. */
	event_add_handler(EVENT_STATS, point_based_stats, NULL);	
	event_add_handler(EVENT_GOLD, point_based_misc, NULL);	
	event_add_handler(EVENT_BIRTHSTATS, point_based_points, NULL);	
}

static void point_based_stop(void)
{
	event_remove_handler(EVENT_STATS, point_based_stats, NULL);	
	event_remove_handler(EVENT_GOLD, point_based_misc, NULL);	
	event_remove_handler(EVENT_BIRTHSTATS, point_based_points, NULL);
}

static game_command point_based_command(void)
{
	game_command cmd = { CMD_NULL, 0, {0} };
	static int stat = 0;
	char ch;

	while (cmd.command == CMD_NULL)
	{
		/* Place cursor just after cost of current stat */
		Term_gotoxy(COSTS_COL + 4, COSTS_ROW + stat);

		/* Get key */
		ch = inkey();

		if (ch == KTRL('X')) 
		{
			cmd.command = CMD_QUIT;
		}

		/* Go back a step, or back to the start of this step */
		else if (ch == ESCAPE) 
		{
			cmd.command = CMD_BIRTH_BACK;
		}

		/* Done */
		else if ((ch == '\r') || (ch == '\n')) 
		{
			cmd.command = CMD_ACCEPT_STATS;
		}
		else
		{
			ch = target_dir(ch);
			
			/* Prev stat, looping round to the bottom when going off the top */
			if (ch == 8)
				stat = (stat + A_MAX - 1) % A_MAX;
			
			/* Next stat, looping round to the top when going off the bottom */
			if (ch == 2)
				stat = (stat + 1) % A_MAX;
			
			/* Decrease stat (if possible) */
			if (ch == 4)
			{
				cmd.command = CMD_SELL_STAT;
				cmd.params.choice = stat;
			}
			
			/* Increase stat (if possible) */
			if (ch == 6)
			{
				cmd.command = CMD_BUY_STAT;
				cmd.params.choice = stat;
			}
		}
	}

	return cmd;
}
	
/* ------------------------------------------------------------------------
 * Asking for the player's chosen name.
 * ------------------------------------------------------------------------ */
static game_command get_name_command(void)
{
	game_command cmd;
	char name[32];

	if (get_name(name, sizeof(name)))
	{
		cmd.command = CMD_NAME_CHOICE;
		cmd.params.string = string_make(name);
	}
	else
	{
		cmd.command = CMD_BIRTH_BACK;
	}

	return cmd;
}

/* ------------------------------------------------------------------------
 * Final confirmation of character.
 * ------------------------------------------------------------------------ */
static game_command get_confirm_command(void)
{
	const char *prompt = "['ESC' to step back, 'S' to start over, or any other key to continue]";
	ui_event_data ke;

	game_command cmd;

	/* Prompt for it */
	prt(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);
	
	/* Buttons */
	button_kill_all();
	button_add("[Continue]", 'q');
	button_add("[ESC]", ESCAPE);
	button_add("[S]", 'S');
	redraw_stuff();
	
	/* Get a key */
	ke = inkey_ex();
	
	/* Start over */
	if (ke.key == 'S')
	{
		cmd.command = CMD_BIRTH_RESTART;
	}
	else if (ke.key == KTRL('X'))
	{
		cmd.command = CMD_QUIT;
	}
	else if (ke.key == ESCAPE)
	{
		cmd.command = CMD_BIRTH_BACK;
	}
	else
	{
		cmd.command = CMD_ACCEPT_CHARACTER;
	}
	
	/* Buttons */
	button_kill_all();
	redraw_stuff();

	/* Clear prompt */
	clear_from(23);

	return cmd;
}



/* ------------------------------------------------------------------------
 * Things that relate to the world outside this file: receiving game events
 * and being asked for game commands.
 * ------------------------------------------------------------------------ */

/*
 * This is called on EVENT_BIRTHSTAGE, and we use it to do any initialising
 * of data structures, or setting up of bits of the screen that need to be
 * done when we first enter each stage.
 */
static void birth_stage_changed(game_event_type type, game_event_data *data, void *user)
{
	/* Before we update current_stage, we'll release handlers, etc. that
	   relate to the previous "current" stage. */
	switch (current_stage)
	{
		case BIRTH_POINTBASED:
		{
			point_based_stop();
			break;
		}

		case BIRTH_ROLLER:
		{
			roller_stop();
			break;
		}
		default:
		{
			/* Nothing to see here. */
		}
	}

	/* Do any initialisation or display changes we need to make on
	   entering this stage. */
	switch (data->birthstage.stage)
	{
		case BIRTH_METHOD_CHOICE:
		{
			Term_clear();
			break;
		}

		case BIRTH_SEX_CHOICE:
		{
			if (current_stage > BIRTH_ROLLER_CHOICE)
				Term_clear();

			/* We only print the generic instructions with the first
			   menu we show on screen.  Likewise, we only need to erase
			   the next menu on screen if we're not proceeding in the
			   usual way. */
			if (current_stage < BIRTH_SEX_CHOICE || current_stage > BIRTH_ROLLER_CHOICE)
				print_menu_instructions();
			else
				region_erase(&race_region);

			init_birth_menu(&sex_menu, data, &gender_region, TRUE, NULL);
			current_menu = &sex_menu;

			break;
		}

		case BIRTH_RACE_CHOICE:
		{
			if (current_stage > BIRTH_RACE_CHOICE)
				region_erase(&class_region);

			init_birth_menu(&race_menu, data, &race_region, TRUE, race_help);
			current_menu = &race_menu;

			break;
		}

		case BIRTH_CLASS_CHOICE:
		{
			if (current_stage > BIRTH_CLASS_CHOICE)
				region_erase(&roller_region);

			init_birth_menu(&class_menu, data, &class_region, TRUE, class_help);
			current_menu = &class_menu;

			break;
		}

		case BIRTH_ROLLER_CHOICE:
		{
			/* When stepping back to here from later in the process,
			   we simulate having whizzed through the first few questions
			   by simply redrawing all the previous menus in their last
			   known state. */
			if (current_stage > BIRTH_ROLLER_CHOICE)
			{
				print_menu_instructions();
				menu_refresh(&sex_menu);
				menu_refresh(&race_menu);
				menu_refresh(&class_menu);
			}

			init_birth_menu(&roller_menu, data, &roller_region, FALSE, NULL);
			current_menu = &roller_menu;

			break;
		}

		case BIRTH_POINTBASED:
		{
			point_based_start();
			break;
		}

		case BIRTH_AUTOROLLER:
		{
			autoroller_start(data->birthstage.xtra);
			break;
		}

		case BIRTH_ROLLER:
		{
			roller_start(data->birthstage.xtra);
			break;
		}

		case BIRTH_NAME_CHOICE:
		case BIRTH_FINAL_CONFIRM:
		{
			display_player(0);
			break;
		}
		case BIRTH_COMPLETE:
		{
			/* We are done. */
		}
	}

	/* Finally update what we consider the "current" stage - this mostly 
	   affects how we handle requests for game commands. */
	current_stage = data->birthstage.stage;
}


/*
 * This is hooked into "get_game_command" when we enter the birthscreen,
 * and is called whenever the game wants a command returned during the 
 * birth process.  
 *
 * This is relatively complex because of the many sub-stages in birth,
 * and so we farm out the actions to other functions to handle
 * each substage.
 */
static game_command ui_get_birth_command(void)
{
	switch (current_stage)
	{
		case BIRTH_METHOD_CHOICE:
			return quickstart_question();

		case BIRTH_SEX_CHOICE:
		case BIRTH_CLASS_CHOICE:
		case BIRTH_RACE_CHOICE:
		case BIRTH_ROLLER_CHOICE:
			return menu_question();

		case BIRTH_POINTBASED:
			return point_based_command();

		case BIRTH_AUTOROLLER:
			return autoroller_command();

		case BIRTH_ROLLER:
			return roller_command();

		case BIRTH_NAME_CHOICE:
			return get_name_command();

		case BIRTH_FINAL_CONFIRM:
			return get_confirm_command();

		default:
		{
			game_command null_cmd = { CMD_QUIT, 0, {0} };
			return null_cmd;
		}
	}
}

/*
 * Called when we enter the birth mode - so we set up handlers, command hooks,
 * etc, here.
 */
static void ui_enter_birthscreen(game_event_type type, game_event_data *data, void *user)
{
	event_add_handler(EVENT_BIRTHSTAGE, birth_stage_changed, NULL);	
	get_game_command = ui_get_birth_command;
}

static void ui_leave_birthscreen(game_event_type type, game_event_data *data, void *user)
{
	/* We don't need these any more. */
	free_birth_menu(&sex_menu);
	free_birth_menu(&race_menu);
	free_birth_menu(&class_menu);
	free_birth_menu(&roller_menu);

	event_remove_handler(EVENT_BIRTHSTAGE, birth_stage_changed, NULL);	
}


void ui_init_birthstate_handlers(void)
{
	event_add_handler(EVENT_ENTER_BIRTH, ui_enter_birthscreen, NULL);
	event_add_handler(EVENT_LEAVE_BIRTH, ui_leave_birthscreen, NULL);
}
