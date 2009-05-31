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
#include "cmds.h"

/*
 * Overview
 * ========
 * This file implements the user interface side of the birth process
 * for the classic terminal-based UI of Angband.
 *
 * It models birth as a series of steps which must be carried out in 
 * a specified order, with the option of stepping backwards to revisit
 * past choices.
 *
 * It starts when we receive the EVENT_ENTER_BIRTH event from the game,
 * and ends when we receive the EVENT_LEAVE_BIRTH event.  In between,
 * we will repeatedly be asked to supply a game command, which change
 * the state of the character being rolled.  Once the player is happy
 * with their character, we send the CMD_ACCEPT_CHARACTER command.
 */


/* A local-to-this-file global to hold the most important bit of state
   between calls to the game proper.  Probably not strictly necessary,
   but reduces complexity a bit. */
enum birth_stage
{
	BIRTH_BACK = -1,
	BIRTH_RESET = 0,
	BIRTH_QUICKSTART,
	BIRTH_SEX_CHOICE,
	BIRTH_RACE_CHOICE,
	BIRTH_CLASS_CHOICE,
	BIRTH_ROLLER_CHOICE,
	BIRTH_POINTBASED,
	BIRTH_ROLLER,
	BIRTH_NAME_CHOICE,
	BIRTH_FINAL_CONFIRM,
	BIRTH_COMPLETE
};


enum birth_questions
{
	BQ_METHOD = 0,
	BQ_SEX,
	BQ_RACE,
	BQ_CLASS,
	BQ_ROLLER,
	MAX_BIRTH_QUESTIONS
};

enum birth_rollers
{
	BR_POINTBASED = 0,
	BR_NORMAL,
	MAX_BIRTH_ROLLERS
};


static void point_based_start(void);
static bool quickstart_allowed = FALSE;

/* ------------------------------------------------------------------------
 * Quickstart? screen.
 * ------------------------------------------------------------------------ */
static enum birth_stage get_quickstart_command(void)
{
	const char *prompt = "['Y' to use this character, 'N' to start afresh, 'C' to change name]";
	ui_event_data ke;

	enum birth_stage next = BIRTH_QUICKSTART;

	/* Prompt for it */
	prt("New character based on previous one:", 0, 0);
	prt(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);
	
	/* Buttons */
	button_kill_all();
	button_add("[Y]", 'y');
	button_add("[N]", 'n');
	button_add("[C]", 'c');
	redraw_stuff();
	
	do
	{
		/* Get a key */
		ke = inkey_ex();
		
		if (ke.key == 'N' || ke.key == 'n')
		{
			cmd_insert(CMD_BIRTH_RESET, TRUE);
			next = BIRTH_SEX_CHOICE;
		}
		else if (ke.key == KTRL('X'))
		{
			cmd_insert(CMD_QUIT);
			next = BIRTH_COMPLETE;
		}
		else if (ke.key == 'C' || ke.key == 'c')
		{
			next = BIRTH_NAME_CHOICE;
		}
		else if (ke.key == 'Y' || ke.key == 'y')
		{
			cmd_insert(CMD_ACCEPT_CHARACTER);
			next = BIRTH_COMPLETE;
		}
	} while (next == BIRTH_QUICKSTART);
	
	/* Buttons */
	button_kill_all();
	redraw_stuff();

	/* Clear prompt */
	clear_from(23);

	return next;
}

/* ------------------------------------------------------------------------
 * The various "menu" bits of the birth process - namely choice of sex,
 * race, class, and roller type.
 * ------------------------------------------------------------------------ */

/* The various menus */
static menu_type sex_menu, race_menu, class_menu, roller_menu;

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

/* We use different menu "browse functions" to display the help text
   sometimes supplied with the menu items - currently just the list
   of bonuses, etc, corresponding to each race and class. */
typedef void (*browse_f) (int oid, void *db, const region *l);

/* We have one of these structures for each menu we display - it holds
   the useful information for the menu - text of the menu items, "help"
   text, current (or default) selection, and whether random selection
   is allowed. */
struct birthmenu_data 
{
	const char **items;
	const char *hint;
	bool allow_random;
};

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

static void race_help(int i, void *db, const region *l)
{
	int j;

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = RACE_AUX_COL;
	Term_gotoxy(RACE_AUX_COL, TABLE_ROW);

	for (j = 0; j < A_MAX; j++) 
	{  
		text_out_e("%s%+d\n", stat_names_reduced[j], p_info[i].r_adj[j]);
	}
	
	text_out_e("Hit die: %d\n", p_info[i].r_mhp);
	text_out_e("Experience: %d%%\n", p_info[i].r_exp);
	text_out_e("Infravision: %d ft", p_info[i].infra * 10);
	
	/* Reset text_out() indentation */
	text_out_indent = 0;
}

static void class_help(int i, void *db, const region *l)
{
	int j;

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = CLASS_AUX_COL;
	Term_gotoxy(CLASS_AUX_COL, TABLE_ROW);

	for (j = 0; j < A_MAX; j++) 
	{  
		text_out_e("%s%+d\n", stat_names_reduced[j], c_info[i].c_adj[j]); 
	}

	text_out_e("Hit die: %d\n", c_info[i].c_mhp);   
	text_out_e("Experience: %d%%", c_info[i].c_exp);
	
	/* Reset text_out() indentation */
	text_out_indent = 0;
}

/* Set up one of our menus ready to display choices for a birth question.
   This is slightly involved. */
static void init_birth_menu(menu_type *menu, int n_choices, int initial_choice, const region *reg, bool allow_random, browse_f aux)
{
	struct birthmenu_data *menu_data;

	/* A couple of behavioural flags - we want selections letters in
	   lower case and a double tap to act as a selection. */
	menu->selections = lower_case;
	menu->flags = MN_DBL_TAP;

	/* Set the number of choices in the menu to the same as the game
	   has told us we've got to offer. */
	menu->count = n_choices;

	/* Allocate sufficient space for our own bits of menu information. */
	menu_data = mem_alloc(sizeof *menu_data);

	/* Copy across the game's suggested initial selection, etc. */
	menu->cursor = initial_choice;
	menu_data->allow_random = allow_random;

	/* Allocate space for an array of menu item texts and help texts
	   (where applicable) */
	menu_data->items = mem_alloc(menu->count * sizeof *menu_data->items);

	/* Poke our menu data in to the assigned slot in the menu structure. */
	menu->menu_data = menu_data;

	/* Set up the "browse" hook to display help text (where applicable). */
	menu->browse_hook = aux;

	/* Get ui-menu to initialise whatever it wants to to give us a scrollable
	   menu. */
	menu_init(menu, MN_SKIN_SCROLL, &birth_iter, reg);
}



static void setup_menus()
{
	int i;

	const char *roller_choices[MAX_BIRTH_ROLLERS] = { 
		"Point-based", 
		"Standard roller" 
	};

	struct birthmenu_data *mdata;

	/* Sex menu fairly straightforward */
	init_birth_menu(&sex_menu, MAX_SEXES, p_ptr->psex, &gender_region, TRUE, NULL);
	mdata = sex_menu.menu_data;
	for (i = 0; i < MAX_SEXES; i++)
	{	
		mdata->items[i] = sex_info[i].title;
	}
	mdata->hint = "Your 'sex' does not have any significant gameplay effects.";

	/* Race menu more complicated. */
	init_birth_menu(&race_menu, z_info->p_max, p_ptr->prace, &race_region, TRUE, race_help);
	mdata = race_menu.menu_data;

	for (i = 0; i < z_info->p_max; i++)
	{	
		mdata->items[i] = p_name + p_info[i].name;
	}
	mdata->hint = "Your 'race' determines various intrinsic factors and bonuses.";

	/* Class menu similar to race. */
	init_birth_menu(&class_menu, z_info->c_max, p_ptr->pclass, &class_region, TRUE, class_help);
	mdata = class_menu.menu_data;

	for (i = 0; i < z_info->c_max; i++)
	{	
		mdata->items[i] = c_name + c_info[i].name;
	}
	mdata->hint = "Your 'class' determines various intrinsic abilities and bonuses";
		
	/* Roller menu straightforward again */
	init_birth_menu(&roller_menu, MAX_BIRTH_ROLLERS, 0, &roller_region, FALSE, NULL);
	mdata = roller_menu.menu_data;
	for (i = 0; i < MAX_BIRTH_ROLLERS; i++)
	{	
		mdata->items[i] = roller_choices[i];
	}
	mdata->hint = "Your choice of character generation.  Point-based is recommended.";
}

/* Cleans up our stored menu info when we've finished with it. */
static void free_birth_menu(menu_type *menu)
{
	struct birthmenu_data *data = menu->menu_data;

	if (data)
	{
		mem_free(data->items);
		mem_free(data);
	}
}

static void free_birth_menus()
{
	/* We don't need these any more. */
	free_birth_menu(&sex_menu);
	free_birth_menu(&race_menu);
	free_birth_menu(&class_menu);
	free_birth_menu(&roller_menu);
}

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
static enum birth_stage menu_question(enum birth_stage current, menu_type *current_menu, cmd_code choice_command)
{
	struct birthmenu_data *menu_data = current_menu->menu_data;
	int cursor = current_menu->cursor;
	ui_event_data cx;

	enum birth_stage next = BIRTH_RESET;
	
	/* Print the question currently being asked. */
	clear_question();
	Term_putstr(QUESTION_COL, QUESTION_ROW, -1, TERM_YELLOW, menu_data->hint);

	current_menu->cmd_keys = "?=*\r\n\x18";	 /* ?, ,= *, \n, <ctl-X> */

	while (next == BIRTH_RESET)
	{
		/* Display the menu, wait for a selection of some sort to be made. */
		cx = menu_select(current_menu, &cursor, EVT_CMD);

		/* As all the menus are displayed in "hierarchical" style, we allow
		   use of "back" (left arrow key or equivalent) to step back in 
		   the proces as well as "escape". */
		if (cx.type == EVT_BACK || cx.type == EVT_ESCAPE)
		{
			next = BIRTH_BACK;
		}
		/* '\xff' is a mouse selection, '\r' a keyboard one. */
		else if (cx.key == '\xff' || cx.key == '\r') 
		{
			if (current == BIRTH_ROLLER_CHOICE)
			{
				if (cursor)
				{
					/* Do a first roll of the stats */
					cmd_insert(CMD_ROLL_STATS);
					next = current + 2;
				}
				else
				{
					/* 
					 * Make sure we've got a point-based char to play with. 
					 * We call point_based_start here to make sure we get
					 * an update on the points totals before trying to
					 * display the screen.
					 */
					point_based_start();
					cmd_insert(CMD_RESET_STATS, TRUE);
					next = current + 1;
				}
			}
			else
			{
				cmd_insert(choice_command, cursor);
				next = current + 1;
			}
		}
		/* '*' chooses an option at random from those the game's provided. */
		else if (cx.key == '*' && menu_data->allow_random) 
		{
			current_menu->cursor = randint0(current_menu->count);
			cmd_insert(choice_command, current_menu->cursor);

			menu_refresh(current_menu);
			next = current + 1;
		}
		else if (cx.key == '=') 
		{
			do_cmd_options();
			next = current;
		}
		else if (cx.key == KTRL('X')) 
		{
			cmd_insert(CMD_QUIT);
			next = BIRTH_COMPLETE;
		}
		else if (cx.key == '?')
		{
			do_cmd_help();
		}
	}
	
	return next;
}

/* ------------------------------------------------------------------------
 * The rolling bit of the roller.
 * ------------------------------------------------------------------------ */
#define ROLLERCOL 42

static enum birth_stage roller_command(bool first_call)
{
	char prompt[80] = "";
	size_t promptlen = 0;

	ui_event_data ke;
	char ch;

	enum birth_stage next = BIRTH_ROLLER;

	/* Used to keep track of whether we've rolled a character before or not. */
	static bool prev_roll = FALSE;

   	/* Display the player - a bit cheaty, but never mind. */
	display_player(0);

	if (first_call)
		prev_roll = FALSE;

	/* Add buttons */
	button_add("[ESC]", ESCAPE);
	button_add("[Enter]", '\r');
	button_add("[r]", 'r');
	if (prev_roll) button_add("[p]", 'p');
	clear_from(Term->hgt - 2);
	redraw_stuff();

	/* Prepare a prompt (must squeeze everything in) */
	strnfcat(prompt, sizeof (prompt), &promptlen, "['r' to reroll");
	if (prev_roll) 
		strnfcat(prompt, sizeof(prompt), &promptlen, ", 'p' for prev");
	strnfcat(prompt, sizeof (prompt), &promptlen, " or 'Enter' to accept]");

	/* Prompt for it */
	prt(prompt, Term->hgt - 1, Term->wid / 2 - promptlen / 2);
	
	/* Prompt and get a command */
	ke = inkey_ex();
	ch = ke.key;

	if (ch == ESCAPE) 
	{
		button_kill('r');
		button_kill('p');

		next = BIRTH_BACK;
	}

	/* 'Enter' accepts the roll */
	if ((ch == '\r') || (ch == '\n')) 
	{
		next = BIRTH_NAME_CHOICE;
	}

	/* Reroll this character */
	else if ((ch == ' ') || (ch == 'r'))
	{
		cmd_insert(CMD_ROLL_STATS);
		prev_roll = TRUE;
	}

	/* Previous character */
	else if (prev_roll && (ch == 'p'))
	{
		cmd_insert(CMD_PREV_STATS);
	}

	/* Quit */
	else if (ch == KTRL('X')) 
	{
		cmd_insert(CMD_QUIT);
		next = BIRTH_COMPLETE;
	}

	/* Help XXX */
	else if (ch == '?')
	{
		do_cmd_help();
	}

	/* Nothing handled directly here */
	else
	{
		bell("Illegal roller command!");
	}

	/* Kill buttons */
	button_kill(ESCAPE);
	button_kill('\r');
	button_kill('r');
	button_kill('p');
	redraw_stuff();

	return next;
}

/* ------------------------------------------------------------------------
 * Point-based stat allocation.
 * ------------------------------------------------------------------------ */

/* The locations of the "costs" area on the birth screen. */
#define COSTS_ROW 2
#define COSTS_COL (42 + 32)
#define TOTAL_COL (42 + 19)

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
	int *stats = data->birthstats.stats;

	/* Display the costs header */
	put_str("Cost", COSTS_ROW - 1, COSTS_COL);
	
	/* Display the costs */
	for (i = 0; i < A_MAX; i++)
	{
		/* Display cost */
		put_str(format("%4d", stats[i]), COSTS_ROW + i, COSTS_COL);
		sum += stats[i];
	}
	
	put_str(format("Total Cost: %2d/%2d", sum, data->birthstats.remaining + sum), COSTS_ROW + A_MAX, TOTAL_COL);
}

static void point_based_start(void)
{
	const char *prompt = "[up/down to move, left/right to modify, 'r' to reset, 'Enter' to accept]";

	/* Clear */
	Term_clear();

	/* Display the player */
	display_player_xtra_info();
	display_player_stat_info();

	prt(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);

	/* Register handlers for various events - cheat a bit because we redraw
	   the lot at once rather than each bit at a time. */
	event_add_handler(EVENT_BIRTHPOINTS, point_based_points, NULL);	
	event_add_handler(EVENT_STATS, point_based_stats, NULL);	
	event_add_handler(EVENT_GOLD, point_based_misc, NULL);	
}

static void point_based_stop(void)
{
	event_remove_handler(EVENT_BIRTHPOINTS, point_based_points, NULL);	
	event_remove_handler(EVENT_STATS, point_based_stats, NULL);	
	event_remove_handler(EVENT_GOLD, point_based_misc, NULL);	
}

static enum birth_stage point_based_command(void)
{
	static int stat = 0;
	char ch;
	enum birth_stage next = BIRTH_POINTBASED;

/*	point_based_display();*/

	/* Place cursor just after cost of current stat */
	Term_gotoxy(COSTS_COL + 4, COSTS_ROW + stat);

	/* Get key */
	ch = inkey();
	
	if (ch == KTRL('X')) 
	{
		cmd_insert(CMD_QUIT);
		next = BIRTH_COMPLETE;
	}
	
	/* Go back a step, or back to the start of this step */
	else if (ch == ESCAPE) 
	{
		next = BIRTH_BACK;
	}

	else if (ch == 'r' || ch == 'R') 
	{
		cmd_insert(CMD_RESET_STATS, FALSE);
	}
	
	/* Done */
	else if ((ch == '\r') || (ch == '\n')) 
	{
		next = BIRTH_NAME_CHOICE;
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
			cmd_insert(CMD_SELL_STAT, stat);
		}
		
		/* Increase stat (if possible) */
		if (ch == 6)
		{
			cmd_insert(CMD_BUY_STAT, stat);
		}
	}

	return next;
}
	
/* ------------------------------------------------------------------------
 * Asking for the player's chosen name.
 * ------------------------------------------------------------------------ */
static enum birth_stage get_name_command(void)
{
	enum birth_stage next;
	char name[32];

	if (get_name(name, sizeof(name)))
	{	
		cmd_insert(CMD_NAME_CHOICE, name);
		next = BIRTH_FINAL_CONFIRM;
	}
	else
	{
		next = BIRTH_BACK;
	}

	return next;
}

/* ------------------------------------------------------------------------
 * Final confirmation of character.
 * ------------------------------------------------------------------------ */
static enum birth_stage get_confirm_command(void)
{
	const char *prompt = "['ESC' to step back, 'S' to start over, or any other key to continue]";
	ui_event_data ke;

	enum birth_stage next;

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
	if (ke.key == 'S' || ke.key == 's')
	{
		next = BIRTH_RESET;
	}
	else if (ke.key == KTRL('X'))
	{
		cmd_insert(CMD_QUIT);
		next = BIRTH_COMPLETE;
	}
	else if (ke.key == ESCAPE)
	{
		next = BIRTH_BACK;
	}
	else
	{
		cmd_insert(CMD_ACCEPT_CHARACTER);
		next = BIRTH_COMPLETE;
	}
	
	/* Buttons */
	button_kill_all();
	redraw_stuff();

	/* Clear prompt */
	clear_from(23);

	return next;
}



/* ------------------------------------------------------------------------
 * Things that relate to the world outside this file: receiving game events
 * and being asked for game commands.
 * ------------------------------------------------------------------------ */

/*
 * This is called when we receive a request for a command in the birth 
 * process.

 * The birth process continues until we send a final character confirmation
 * command (or quit), so this is effectively called in a loop by the main
 * game.
 *
 * We're imposing a step-based system onto the main game here, so we need
 * to keep track of where we're up to, where each step moves on to, etc.
 */
errr get_birth_command(bool wait)
{
	static enum birth_stage current_stage = BIRTH_RESET;
	static enum birth_stage prev;
	static enum birth_stage roller = BIRTH_RESET;
	enum birth_stage next = current_stage;

	switch (current_stage)
	{
		case BIRTH_RESET:
		{
			cmd_insert(CMD_BIRTH_RESET, TRUE);
			roller = BIRTH_RESET;
			
			if (quickstart_allowed)
				next = BIRTH_QUICKSTART;
			else
				next = BIRTH_SEX_CHOICE;

			break;
		}

		case BIRTH_QUICKSTART:
		{
			display_player(0);
			next = get_quickstart_command();
			break;
		}

		case BIRTH_SEX_CHOICE:
		case BIRTH_CLASS_CHOICE:
		case BIRTH_RACE_CHOICE:
		case BIRTH_ROLLER_CHOICE:
		{
			menu_type *menu = &sex_menu;
			cmd_code command = CMD_CHOOSE_SEX;

			Term_clear();
			print_menu_instructions();

			if (current_stage > BIRTH_SEX_CHOICE)
			{
				menu_refresh(&sex_menu);
				menu = &race_menu;
				command = CMD_CHOOSE_RACE;
			}
			
			if (current_stage > BIRTH_RACE_CHOICE)
			{
				menu_refresh(&race_menu);
				menu = &class_menu;
				command = CMD_CHOOSE_CLASS;
			}

			if (current_stage > BIRTH_CLASS_CHOICE)
			{
				menu_refresh(&class_menu);
				menu = &roller_menu;
				command = CMD_NULL;
			}
			
			next = menu_question(current_stage, menu, command);

			if (next == BIRTH_BACK)
				next = current_stage - 1;

			/* Make sure that the character gets reset before quickstarting */
			if (next == BIRTH_QUICKSTART) 
				next = BIRTH_RESET;

			break;
		}

		case BIRTH_POINTBASED:
		{
			roller = BIRTH_POINTBASED;
	
			if (prev > BIRTH_POINTBASED)
				point_based_start();

			next = point_based_command();

			if (next == BIRTH_BACK)
				next = BIRTH_ROLLER_CHOICE;

			if (next != BIRTH_POINTBASED)
				point_based_stop();

			break;
		}

		case BIRTH_ROLLER:
		{
			roller = BIRTH_ROLLER;
			next = roller_command(prev < BIRTH_ROLLER);
			if (next == BIRTH_BACK)
				next = BIRTH_ROLLER_CHOICE;

			break;
		}

		case BIRTH_NAME_CHOICE:
		{
			if (prev < BIRTH_NAME_CHOICE)
				display_player(0);

			next = get_name_command();
			if (next == BIRTH_BACK)
				next = roller;

			break;
		}

		case BIRTH_FINAL_CONFIRM:
		{
			if (prev < BIRTH_FINAL_CONFIRM)
				display_player(0);

			next = get_confirm_command();
			if (next == BIRTH_BACK)
				next = BIRTH_NAME_CHOICE;

			break;
		}

		default:
		{
			/* Remove dodgy compiler warning, */
		}
	}

	prev = current_stage;
	current_stage = next;

	return 0;
}

/*
 * Called when we enter the birth mode - so we set up handlers, command hooks,
 * etc, here.
 */
static void ui_enter_birthscreen(game_event_type type, game_event_data *data, void *user)
{
	/* Set the ugly static global that tells us if quickstart's available. */
	quickstart_allowed = data->flag;

	setup_menus();
}

static void ui_leave_birthscreen(game_event_type type, game_event_data *data, void *user)
{
	free_birth_menus();
}


void ui_init_birthstate_handlers(void)
{
	event_add_handler(EVENT_ENTER_BIRTH, ui_enter_birthscreen, NULL);
	event_add_handler(EVENT_LEAVE_BIRTH, ui_leave_birthscreen, NULL);
}

