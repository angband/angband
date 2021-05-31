/**
 * \file ui-birth.c
 * \brief Text-based user interface for character creation
 *
 * Copyright (c) 1987 - 2015 Angband contributors
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
#include "cmds.h"
#include "cmd-core.h"
#include "game-event.h"
#include "game-input.h"
#include "obj-tval.h"
#include "player.h"
#include "player-spell.h"
#include "ui-birth.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-menu.h"
#include "ui-options.h"
#include "ui-player.h"
#include "ui-prefs.h"
#include "ui-target.h"

/**
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

/**
 * A local-to-this-file global to hold the most important bit of state
 * between calls to the game proper.  Probably not strictly necessary,
 * but reduces complexity a bit. */
enum birth_stage
{
	BIRTH_BACK = -1,
	BIRTH_RESET = 0,
	BIRTH_QUICKSTART,
	BIRTH_RACE_CHOICE,
	BIRTH_CLASS_CHOICE,
	BIRTH_ROLLER_CHOICE,
	BIRTH_POINTBASED,
	BIRTH_ROLLER,
	BIRTH_NAME_CHOICE,
	BIRTH_HISTORY_CHOICE,
	BIRTH_FINAL_CONFIRM,
	BIRTH_COMPLETE
};


enum birth_questions
{
	BQ_METHOD = 0,
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
static bool quickstart_allowed = false;
bool arg_force_name;

/**
 * ------------------------------------------------------------------------
 * Quickstart? screen.
 * ------------------------------------------------------------------------ */
static enum birth_stage textui_birth_quickstart(void)
//phantom name change changes
{
	const char *prompt = "['Y': use as is; 'N': redo; 'C': change name/history; '=': set birth options]";

	enum birth_stage next = BIRTH_QUICKSTART;

	/* Prompt for it */
	prt("New character based on previous one:", 0, 0);
	prt(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);

	do {
		/* Get a key */
		struct keypress ke = inkey();
		
		if (ke.code == 'N' || ke.code == 'n') {
			cmdq_push(CMD_BIRTH_RESET);
			next = BIRTH_RACE_CHOICE;
		} else if (ke.code == KTRL('X')) {
			quit(NULL);
		} else if ( !arg_force_name && (ke.code == 'C' || ke.code == 'c')) {
			next = BIRTH_NAME_CHOICE;
		} else if (ke.code == '=') {
			do_cmd_options_birth();
		} else if (ke.code == 'Y' || ke.code == 'y') {
			cmdq_push(CMD_ACCEPT_CHARACTER);
			next = BIRTH_COMPLETE;
		}
	} while (next == BIRTH_QUICKSTART);

	/* Clear prompt */
	clear_from(23);

	return next;
}

/**
 * ------------------------------------------------------------------------
 * The various "menu" bits of the birth process - namely choice of race,
 * class, and roller type.
 * ------------------------------------------------------------------------ */

/**
 * The various menus
 */
static struct menu race_menu, class_menu, roller_menu;

/**
 * Locations of the menus, etc. on the screen
 */
#define HEADER_ROW       1
#define QUESTION_ROW     7
#define TABLE_ROW        9

#define QUESTION_COL     2
#define RACE_COL         2
#define RACE_AUX_COL    19
#define CLASS_COL       19
#define CLASS_AUX_COL   36
#define ROLLER_COL      36
#define HIST_INSTRUCT_ROW 18

#define MENU_ROWS TABLE_ROW + 14

/**
 * upper left column and row, width, and lower column
 */
static region race_region = {RACE_COL, TABLE_ROW, 17, MENU_ROWS};
static region class_region = {CLASS_COL, TABLE_ROW, 17, MENU_ROWS};
static region roller_region = {ROLLER_COL, TABLE_ROW, 34, MENU_ROWS};

/**
 * We use different menu "browse functions" to display the help text
 * sometimes supplied with the menu items - currently just the list
 * of bonuses, etc, corresponding to each race and class.
 */
typedef void (*browse_f) (int oid, void *db, const region *l);

/**
 * We have one of these structures for each menu we display - it holds
 * the useful information for the menu - text of the menu items, "help"
 * text, current (or default) selection, and whether random selection
 * is allowed.
 */
struct birthmenu_data 
{
	const char **items;
	const char *hint;
	bool allow_random;
};

/**
 * A custom "display" function for our menus that simply displays the
 * text from our stored data in a different colour if it's currently
 * selected.
 */
static void birthmenu_display(struct menu *menu, int oid, bool cursor,
			      int row, int col, int width)
{
	struct birthmenu_data *data = menu->menu_data;

	byte attr = curs_attrs[CURS_KNOWN][0 != cursor];
	c_put_str(attr, data->items[oid], row, col);
}

/**
 * Our custom menu iterator, only really needed to allow us to override
 * the default handling of "commands" in the standard iterators (hence
 * only defining the display and handler parts).
 */
static const menu_iter birth_iter = { NULL, NULL, birthmenu_display, NULL, NULL };

static void skill_help(const int r_skills[], const int c_skills[], int mhp, int exp, int infra)
{
	s16b skills[SKILL_MAX];
	unsigned i;

	for (i = 0; i < SKILL_MAX ; ++i)
		skills[i] = (r_skills ? r_skills[i] : 0 ) + (c_skills ? c_skills[i] : 0);

	text_out_e("Hit/Shoot/Throw: %+d/%+d/%+d\n", skills[SKILL_TO_HIT_MELEE],
			   skills[SKILL_TO_HIT_BOW], skills[SKILL_TO_HIT_THROW]);
	text_out_e("Hit die: %2d   XP mod: %d%%\n", mhp, exp);
	text_out_e("Disarm: %+3d/%+3d   Devices: %+3d\n", skills[SKILL_DISARM_PHYS],
			   skills[SKILL_DISARM_MAGIC], skills[SKILL_DEVICE]);
	text_out_e("Save:   %+3d   Stealth: %+3d\n", skills[SKILL_SAVE],
			   skills[SKILL_STEALTH]);
	if (infra >= 0)
		text_out_e("Infravision:  %d ft\n", infra * 10);
	text_out_e("Digging:      %+d\n", skills[SKILL_DIGGING]);
	text_out_e("Search:       %+d", skills[SKILL_SEARCH]);
	if (infra < 0)
		text_out_e("\n");
}

static void race_help(int i, void *db, const region *l)
{
	int j;
	struct player_race *r = player_id2race(i);
	int len = (STAT_MAX + 1) / 2;

	struct player_ability *ability;
	int n_flags = 0;
	int flag_space = 3;

	if (!r) return;

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = RACE_AUX_COL;
	Term_gotoxy(RACE_AUX_COL, TABLE_ROW);

	for (j = 0; j < len; j++) {  
		const char *name = stat_names_reduced[j];
		int adj = r->r_adj[j];

		text_out_e("%s%+3d", name, adj);

		if (j * 2 + 1 < STAT_MAX) {
			name = stat_names_reduced[j + len];
			adj = r->r_adj[j + len];
			text_out_e("  %s%+3d", name, adj);
		}

		text_out("\n");
	}
	
	text_out_e("\n");
	skill_help(r->r_skills, NULL, r->r_mhp, r->r_exp, r->infra);
	text_out_e("\n");

	for (ability = player_abilities; ability; ability = ability->next) {
		if (n_flags >= flag_space) break;
		if (streq(ability->type, "object") &&
			!of_has(r->flags, ability->index)) {
			continue;
		} else if (streq(ability->type, "player") &&
				   !pf_has(r->pflags, ability->index)) {
			continue;
		} else if (streq(ability->type, "element") &&
				   (r->el_info[ability->index].res_level != ability->value)) {
			continue;
		}
		text_out_e("\n%s", ability->name);
		n_flags++;
	}

	while (n_flags < flag_space) {
		text_out_e("\n");
		n_flags++;
	}

	/* Reset text_out() indentation */
	text_out_indent = 0;
}

static void class_help(int i, void *db, const region *l)
{
	int j;
	struct player_class *c = player_id2class(i);
	const struct player_race *r = player->race;
	int len = (STAT_MAX + 1) / 2;

	struct player_ability *ability;
	int n_flags = 0;
	int flag_space = 5;

	if (!c) return;

	/* Output to the screen */
	text_out_hook = text_out_to_screen;
	
	/* Indent output */
	text_out_indent = CLASS_AUX_COL;
	Term_gotoxy(CLASS_AUX_COL, TABLE_ROW);

	for (j = 0; j < len; j++) {  
		const char *name = stat_names_reduced[j];
		int adj = c->c_adj[j] + r->r_adj[j];

		text_out_e("%s%+3d", name, adj);

		if (j*2 + 1 < STAT_MAX) {
			name = stat_names_reduced[j + len];
			adj = c->c_adj[j + len] + r->r_adj[j + len];
			text_out_e("  %s%+3d", name, adj);
		}

		text_out("\n");
	}

	text_out_e("\n");
	
	skill_help(r->r_skills, c->c_skills, r->r_mhp + c->c_mhp,
			   r->r_exp + c->c_exp, -1);

	if (c->magic.total_spells) {
		int count;
		struct magic_realm *realm = class_magic_realms(c, &count), *realm_next;
		char buf[120];

		my_strcpy(buf, realm->name, sizeof(buf));
		realm_next = realm->next;
		mem_free(realm);
		realm = realm_next;
		if (count > 1) {
			while (realm) {
				count--;
				if (count) {
					my_strcat(buf, ", ", sizeof(buf));
				} else {
					my_strcat(buf, " and ", sizeof(buf));
				}
				my_strcat(buf, realm->name, sizeof(buf));
				realm_next = realm->next;
				mem_free(realm);
				realm = realm_next;
			}
		}
		text_out_e("\nLearns %s magic", buf);
	}

	for (ability = player_abilities; ability; ability = ability->next) {
		if (n_flags >= flag_space) break;
		if (streq(ability->type, "object") &&
			!of_has(c->flags, ability->index)) {
			continue;
		} else if (streq(ability->type, "player") &&
				   !pf_has(c->pflags, ability->index)) {
			continue;
		} else if (streq(ability->type, "element")) {
			continue;
		}

		text_out_e("\n%s", ability->name);
		n_flags++;
	}

	while (n_flags < flag_space) {
		text_out_e("\n");
		n_flags++;
	}

	/* Reset text_out() indentation */
	text_out_indent = 0;
}

/**
 * Set up one of our menus ready to display choices for a birth question.
 * This is slightly involved.
 */
static void init_birth_menu(struct menu *menu, int n_choices,
							int initial_choice, const region *reg,
							bool allow_random, browse_f aux)
{
	struct birthmenu_data *menu_data;

	/* Initialise a basic menu */
	menu_init(menu, MN_SKIN_SCROLL, &birth_iter);

	/* A couple of behavioural flags - we want selections letters in
	   lower case and a double tap to act as a selection. */
	menu->selections = lower_case;
	menu->flags = MN_DBL_TAP;

	/* Copy across the game's suggested initial selection, etc. */
	menu->cursor = initial_choice;

	/* Allocate sufficient space for our own bits of menu information. */
	menu_data = mem_alloc(sizeof *menu_data);

	/* Allocate space for an array of menu item texts and help texts
	   (where applicable) */
	menu_data->items = mem_alloc(n_choices * sizeof *menu_data->items);
	menu_data->allow_random = allow_random;

	/* Set private data */
	menu_setpriv(menu, n_choices, menu_data);

	/* Set up the "browse" hook to display help text (where applicable). */
	menu->browse_hook = aux;

	/* Lay out the menu appropriately */
	menu_layout(menu, reg);
}



static void setup_menus(void)
{
	int i, n;
	struct player_class *c;
	struct player_race *r;

	const char *roller_choices[MAX_BIRTH_ROLLERS] = { 
		"Point-based", 
		"Standard roller" 
	};

	struct birthmenu_data *mdata;

	/* Count the races */
	n = 0;
	for (r = races; r; r = r->next) n++;

	/* Race menu. */
	init_birth_menu(&race_menu, n, player->race ? player->race->ridx : 0,
	                &race_region, true, race_help);
	mdata = race_menu.menu_data;

	for (i = 0, r = races; r; r = r->next, i++)
		mdata->items[r->ridx] = r->name;
	mdata->hint = "Race affects stats and skills, and may confer resistances and abilities.";

	/* Count the classes */
	n = 0;
	for (c = classes; c; c = c->next) n++;

	/* Class menu similar to race. */
	init_birth_menu(&class_menu, n, player->class ? player->class->cidx : 0,
	                &class_region, true, class_help);
	mdata = class_menu.menu_data;

	for (i = 0, c = classes; c; c = c->next, i++)
		mdata->items[c->cidx] = c->name;
	mdata->hint = "Class affects stats, skills, and other character traits.";
		
	/* Roller menu straightforward */
	init_birth_menu(&roller_menu, MAX_BIRTH_ROLLERS, 0, &roller_region, false,
					NULL);
	mdata = roller_menu.menu_data;
	for (i = 0; i < MAX_BIRTH_ROLLERS; i++)
		mdata->items[i] = roller_choices[i];
	mdata->hint = "Choose how to generate your intrinsic stats. Point-based is recommended.";
}

/**
 * Cleans up our stored menu info when we've finished with it.
 */
static void free_birth_menu(struct menu *menu)
{
	struct birthmenu_data *data = menu->menu_data;

	if (data) {
		mem_free(data->items);
		mem_free(data);
	}
}

static void free_birth_menus(void)
{
	/* We don't need these any more. */
	free_birth_menu(&race_menu);
	free_birth_menu(&class_menu);
	free_birth_menu(&roller_menu);
}

/**
 * Clear the previous question
 */
static void clear_question(void)
{
	int i;

	for (i = QUESTION_ROW; i < TABLE_ROW; i++)
		/* Clear line, position cursor */
		Term_erase(0, i, 255);
}


#define BIRTH_MENU_HELPTEXT \
	"{light blue}Please select your character traits from the menus below:{/}\n\n" \
	"Use the {light green}movement keys{/} to scroll the menu, " \
	"{light green}Enter{/} to select the current menu item, '{light green}*{/}' " \
	"for a random menu item, '{light green}ESC{/}' to step back through the " \
	"birth process, '{light green}={/}' for the birth options, '{light green}?{/}' " \
	"for help, or '{light green}Ctrl-X{/}' to quit."

/**
 * Show the birth instructions on an otherwise blank screen
 */	
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

/**
 * Allow the user to select from the current menu, and return the 
 * corresponding command to the game.  Some actions are handled entirely
 * by the UI (displaying help text, for instance).
 */
static enum birth_stage menu_question(enum birth_stage current,
									  struct menu *current_menu,
									  cmd_code choice_command)
{
	struct birthmenu_data *menu_data = menu_priv(current_menu);
	ui_event cx;

	enum birth_stage next = BIRTH_RESET;
	
	/* Print the question currently being asked. */
	clear_question();
	Term_putstr(QUESTION_COL, QUESTION_ROW, -1, COLOUR_YELLOW, menu_data->hint);

	current_menu->cmd_keys = "?=*\x18";	 /* ?, =, *, <ctl-X> */

	while (next == BIRTH_RESET) {
		/* Display the menu, wait for a selection of some sort to be made. */
		cx = menu_select(current_menu, EVT_KBRD, false);

		/* As all the menus are displayed in "hierarchical" style, we allow
		   use of "back" (left arrow key or equivalent) to step back in 
		   the proces as well as "escape". */
		if (cx.type == EVT_ESCAPE) {
			next = BIRTH_BACK;
		} else if (cx.type == EVT_SELECT) {
			if (current == BIRTH_ROLLER_CHOICE) {
				if (current_menu->cursor) {
					/* Do a first roll of the stats */
					cmdq_push(CMD_ROLL_STATS);
					next = current + 2;
				} else {
					/* 
					 * Make sure we've got a point-based char to play with. 
					 * We call point_based_start here to make sure we get
					 * an update on the points totals before trying to
					 * display the screen.  The call to CMD_RESET_STATS
					 * forces a rebuying of the stats to give us up-to-date
					 * totals.  This is, it should go without saying, a hack.
					 */
					point_based_start();
					cmdq_push(CMD_RESET_STATS);
					cmd_set_arg_choice(cmdq_peek(), "choice", true);
					next = current + 1;
				}
			} else {
				cmdq_push(choice_command);
				cmd_set_arg_choice(cmdq_peek(), "choice", current_menu->cursor);
				next = current + 1;
			}
		} else if (cx.type == EVT_KBRD) {
			/* '*' chooses an option at random from those the game's provided */
			if (cx.key.code == '*' && menu_data->allow_random) {
				current_menu->cursor = randint0(current_menu->count);
				cmdq_push(choice_command);
				cmd_set_arg_choice(cmdq_peek(), "choice", current_menu->cursor);

				menu_refresh(current_menu, false);
				next = current + 1;
			} else if (cx.key.code == '=') {
				do_cmd_options_birth();
				next = current;
			} else if (cx.key.code == KTRL('X')) {
				quit(NULL);
			} else if (cx.key.code == '?') {
				do_cmd_help();
			}
		}
	}
	
	return next;
}

/**
 * ------------------------------------------------------------------------
 * The rolling bit of the roller.
 * ------------------------------------------------------------------------ */
static enum birth_stage roller_command(bool first_call)
{
	char prompt[80] = "";
	size_t promptlen = 0;

	struct keypress ch;

	enum birth_stage next = BIRTH_ROLLER;

	/* Used to keep track of whether we've rolled a character before or not. */
	static bool prev_roll = false;

	/* Display the player - a bit cheaty, but never mind. */
	display_player(0);

	if (first_call)
		prev_roll = false;

	/* Prepare a prompt (must squeeze everything in) */
	strnfcat(prompt, sizeof (prompt), &promptlen, "['r' to reroll");
	if (prev_roll) 
		strnfcat(prompt, sizeof(prompt), &promptlen, ", 'p' for previous roll");
	strnfcat(prompt, sizeof (prompt), &promptlen, " or 'Enter' to accept]");

	/* Prompt for it */
	prt(prompt, Term->hgt - 1, Term->wid / 2 - promptlen / 2);
	
	/* Prompt and get a command */
	ch = inkey();

	/* Analyse the command */
	if (ch.code == ESCAPE) {
		/* Back out */
		next = BIRTH_BACK;
	} else if (ch.code == KC_ENTER) {
		/* 'Enter' accepts the roll */
		next = BIRTH_NAME_CHOICE;
	} else if ((ch.code == ' ') || (ch.code == 'r')) {
		/* Reroll this character */
		cmdq_push(CMD_ROLL_STATS);
		prev_roll = true;
	} else if (prev_roll && (ch.code == 'p')) {
		/* Previous character */
		cmdq_push(CMD_PREV_STATS);
	} else if (ch.code == KTRL('X')) {
		/* Quit */
		quit(NULL);
	} else if (ch.code == '?') {
		/* Help XXX */
		do_cmd_help();
	} else {
		/* Nothing handled directly here */
		bell("Illegal roller command!");
	}

	return next;
}

/**
 * ------------------------------------------------------------------------
 * Point-based stat allocation.
 * ------------------------------------------------------------------------ */

/* The locations of the "costs" area on the birth screen. */
#define COSTS_ROW 2
#define COSTS_COL (42 + 32)
#define TOTAL_COL (42 + 19)

/**
 * This is called whenever a stat changes.  We take the easy road, and just
 * redisplay them all using the standard function.
 */
static void point_based_stats(game_event_type type, game_event_data *data,
							  void *user)
{
	display_player_stat_info();
}

/**
 * This is called whenever any of the other miscellaneous stat-dependent things
 * changed.  We are hooked into changes in the amount of gold in this case,
 * but redisplay everything because it's easier.
 */
static void point_based_misc(game_event_type type, game_event_data *data,
							 void *user)
{
	display_player_xtra_info();
}


/**
 * This is called whenever the points totals are changed (in birth.c), so
 * that we can update our display of how many points have been spent and
 * are available.
 */
static void point_based_points(game_event_type type, game_event_data *data,
							   void *user)
{
	int i;
	int sum = 0;
	int *stats = data->birthstats.stats;

	/* Display the costs header */
	put_str("Cost", COSTS_ROW - 1, COSTS_COL);
	
	/* Display the costs */
	for (i = 0; i < STAT_MAX; i++) {
		/* Display cost */
		put_str(format("%4d", stats[i]), COSTS_ROW + i, COSTS_COL);
		sum += stats[i];
	}
	
	put_str(format("Total Cost: %2d/%2d", sum,
				   data->birthstats.remaining + sum), COSTS_ROW + STAT_MAX,
			TOTAL_COL);
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
	struct keypress ch;
	enum birth_stage next = BIRTH_POINTBASED;

	/* Place cursor just after cost of current stat */
	Term_gotoxy(COSTS_COL + 4, COSTS_ROW + stat);

	/* Get key */
	ch = inkey();
	
	if (ch.code == KTRL('X')) {
		quit(NULL);
	} else if (ch.code == ESCAPE) {
		/* Go back a step, or back to the start of this step */
		next = BIRTH_BACK;
	} else if (ch.code == 'r' || ch.code == 'R') {
		cmdq_push(CMD_RESET_STATS);
		cmd_set_arg_choice(cmdq_peek(), "choice", false);
	} else if (ch.code == KC_ENTER) {
		/* Done */
		next = BIRTH_NAME_CHOICE;
	} else {
		int dir = target_dir(ch);

		/* Prev stat, looping round to the bottom when going off the top */
		if (dir == 8)
			stat = (stat + STAT_MAX - 1) % STAT_MAX;
		
		/* Next stat, looping round to the top when going off the bottom */
		if (dir == 2)
			stat = (stat + 1) % STAT_MAX;
		
		/* Decrease stat (if possible) */
		if (dir == 4) {
			cmdq_push(CMD_SELL_STAT);
			cmd_set_arg_choice(cmdq_peek(), "choice", stat);
		}
		
		/* Increase stat (if possible) */
		if (dir == 6) {
			cmdq_push(CMD_BUY_STAT);
			cmd_set_arg_choice(cmdq_peek(), "choice", stat);
		}
	}

	return next;
}
	
/**
 * ------------------------------------------------------------------------
 * Asking for the player's chosen name.
 * ------------------------------------------------------------------------ */
//phantom changes for server
static enum birth_stage get_name_command(void)
{
	enum birth_stage next;
	char name[PLAYER_NAME_LEN];

	/* Use frontend-provided savefile name if requested */
	if (arg_name[0]) {
		my_strcpy(player->full_name, arg_name, sizeof(player->full_name));
	}

	if (arg_force_name) {
		next = BIRTH_HISTORY_CHOICE;
	} else if (get_character_name(name, sizeof(name))) {
		cmdq_push(CMD_NAME_CHOICE);
		cmd_set_arg_string(cmdq_peek(), "name", name);
		next = BIRTH_HISTORY_CHOICE;
	} else {
		next = BIRTH_BACK;
	}

	
	return next;
}

static void get_screen_loc(size_t cursor, int *x, int *y, size_t n_lines,
	size_t *line_starts, size_t *line_lengths)
{
	size_t lengths_so_far = 0;
	size_t i;

	if (!line_starts || !line_lengths) return;

	for (i = 0; i < n_lines; i++) {
		if (cursor >= line_starts[i]) {
			if (cursor <= (line_starts[i] + line_lengths[i])) {
				*y = i;
				*x = cursor - lengths_so_far;
				break;
			}
		}
		/* +1 for the space */
		lengths_so_far += line_lengths[i] + 1;
	}
}

static int edit_text(char *buffer, int buflen) {
	int len = strlen(buffer);
	bool done = false;
	int cursor = 0;

	while (!done) {
		int x = 0, y = 0;
		struct keypress ke;

		region area = { 1, HIST_INSTRUCT_ROW + 1, 71, 5 };
		textblock *tb = textblock_new();

		size_t *line_starts = NULL, *line_lengths = NULL;
		size_t n_lines;
		/*
		 * This is the total number of UTF-8 characters; can be less
		 * less than len, the number of 8-bit units in the buffer,
		 * if a single character is encoded with more than one 8-bit
		 * unit.
		 */
		int ulen;

		/* Display on screen */
		clear_from(HIST_INSTRUCT_ROW);
		textblock_append(tb, "%s", buffer);
		textui_textblock_place(tb, area, NULL);

		n_lines = textblock_calculate_lines(tb,
				&line_starts, &line_lengths, area.width);
		ulen = (n_lines > 0) ? line_starts[n_lines - 1] +
			line_lengths[n_lines - 1]: 0;

		/* Set cursor to current editing position */
		get_screen_loc(cursor, &x, &y, n_lines, line_starts, line_lengths);
		Term_gotoxy(1 + x, 19 + y);

		ke = inkey();
		switch (ke.code) {
			case ESCAPE:
				return -1;

			case KC_ENTER:
				done = true;
				break;

			case ARROW_LEFT:
				if (cursor > 0) cursor--;
				break;

			case ARROW_RIGHT:
				if (cursor < ulen) cursor++;
				break;

			case ARROW_DOWN: {
				int add = line_lengths[y] + 1;
				if (cursor + add < ulen) cursor += add;
				break;
			}

			case ARROW_UP:
				if (y > 0) {
					int up = line_lengths[y - 1] + 1;
					if (cursor - up >= 0) cursor -= up;
				}
				break;

			case KC_END:
				cursor = MAX(0, ulen);
				break;

			case KC_HOME:
				cursor = 0;
				break;

			case KC_BACKSPACE:
			case KC_DELETE: {
				char *ocurs, *oshift;

				/* Refuse to backspace into oblivion */
				if ((ke.code == KC_BACKSPACE && cursor == 0) ||
						(ke.code == KC_DELETE && cursor >= ulen))
					break;

				/*
				 * Move the string from k to nul along to the
				 * left by 1.  First, have to get offset
				 * corresponding to the cursor position.
				 */
				ocurs = utf8_fskip(buffer, cursor, NULL);
				assert(ocurs);
				if (ke.code == KC_BACKSPACE) {
					/* Get offset of the previous character. */
					oshift = utf8_rskip(ocurs, 1, buffer);
					assert(oshift);
					memmove(oshift, ocurs,
						len - (ocurs - buffer));
					/* Decrement */
					--cursor;
					len -= ocurs - oshift;
				} else {
					/* Get offset of the next character. */
					oshift = utf8_fskip(ocurs, 1, NULL);
					assert(oshift);
					memmove(ocurs, oshift,
						len - (oshift - buffer));
					/* Decrement. */
					len -= oshift - ocurs;
				}

				/* Terminate */
				buffer[len] = '\0';

				break;
			}
			
			default: {
				bool atnull = (cursor == ulen);
				char encoded[5];
				int n_enc;
				char *ocurs;

				if (!keycode_isprint(ke.code))
					break;

				n_enc = utf32_to_utf8(encoded,
					N_ELEMENTS(encoded), &ke.code, 1, NULL);

				/*
				 * Make sure we have something to add and have
				 * enough space.
				 */
				if (n_enc == 0 || n_enc + len >= buflen) {
					break;
				}

				/* Insert the encoded character. */
				if (atnull) {
					ocurs = buffer + len;
				} else {
					ocurs = utf8_fskip(buffer, cursor, NULL);
					assert(ocurs);
					/*
					 * Move the rest of the buffer along
					 * to make room.
					 */
					memmove(ocurs + n_enc, ocurs,
						len - (ocurs - buffer));
				}
				memcpy(ocurs, encoded, n_enc);

				/* Update cursor position and length. */
				++cursor;
				len += n_enc;

				/* Terminate */
				buffer[len] = '\0';

				break;
			}
		}

		mem_free(line_starts);
		mem_free(line_lengths);
		textblock_free(tb);
	}

	return 0;
}

/**
 * ------------------------------------------------------------------------
 * Allowing the player to choose their history.
 * ------------------------------------------------------------------------ */
static enum birth_stage get_history_command(void)
{
	enum birth_stage next = 0;
	struct keypress ke;
	char old_history[240];

	/* Save the original history */
	my_strcpy(old_history, player->history, sizeof(old_history));

	/* Ask for some history */
	prt("Accept character history? [y/n]", 0, 0);
	ke = inkey();

	/* Quit, go back, change history, or accept */
	if (ke.code == KTRL('X')) {
		quit(NULL);
	} else if (ke.code == ESCAPE) {
		next = BIRTH_BACK;
	} else if (ke.code == 'N' || ke.code == 'n') {
		char history[240];
		my_strcpy(history, player->history, sizeof(history));

		switch (edit_text(history, sizeof(history))) {
			case -1:
				next = BIRTH_BACK;
				break;
			case 0:
				cmdq_push(CMD_HISTORY_CHOICE);
				cmd_set_arg_string(cmdq_peek(), "history", history);
				next = BIRTH_HISTORY_CHOICE;
		}
	} else {
		next = BIRTH_FINAL_CONFIRM;
	}

	return next;
}

/**
 * ------------------------------------------------------------------------
 * Final confirmation of character.
 * ------------------------------------------------------------------------ */
static enum birth_stage get_confirm_command(void)
{
	const char *prompt = "['ESC' to step back, 'S' to start over, or any other key to continue]";
	struct keypress ke;

	enum birth_stage next = BIRTH_RESET;

	/* Prompt for it */
	prt(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);

	/* Get a key */
	ke = inkey();
	
	/* Start over */
	if (ke.code == 'S' || ke.code == 's') {
		next = BIRTH_RESET;
	} else if (ke.code == KTRL('X')) {
		quit(NULL);
	} else if (ke.code == ESCAPE) {
		next = BIRTH_BACK;
	} else {
		cmdq_push(CMD_ACCEPT_CHARACTER);
		next = BIRTH_COMPLETE;
	}

	/* Clear prompt */
	clear_from(23);

	return next;
}



/**
 * ------------------------------------------------------------------------
 * Things that relate to the world outside this file: receiving game events
 * and being asked for game commands.
 * ------------------------------------------------------------------------ */

/**
 * This is called when we receive a request for a command in the birth 
 * process.

 * The birth process continues until we send a final character confirmation
 * command (or quit), so this is effectively called in a loop by the main
 * game.
 *
 * We're imposing a step-based system onto the main game here, so we need
 * to keep track of where we're up to, where each step moves on to, etc.
 */
int textui_do_birth(void)
{
	enum birth_stage current_stage = BIRTH_RESET;
	enum birth_stage prev = BIRTH_BACK;
	enum birth_stage roller = BIRTH_RESET;
	enum birth_stage next = current_stage;

	bool done = false;

	cmdq_push(CMD_BIRTH_INIT);
	cmdq_execute(CTX_BIRTH);

	while (!done) {

		switch (current_stage)
		{
			case BIRTH_RESET:
			{
				cmdq_push(CMD_BIRTH_RESET);

				roller = BIRTH_RESET;
				
				if (quickstart_allowed)
					next = BIRTH_QUICKSTART;
				else
					next = BIRTH_RACE_CHOICE;

				break;
			}

			case BIRTH_QUICKSTART:
			{
				display_player(0);
				next = textui_birth_quickstart();
				if (next == BIRTH_COMPLETE)
					done = true;
				break;
			}

			case BIRTH_CLASS_CHOICE:
			case BIRTH_RACE_CHOICE:
			case BIRTH_ROLLER_CHOICE:
			{
				struct menu *menu = &race_menu;
				cmd_code command = CMD_CHOOSE_RACE;

				Term_clear();
				print_menu_instructions();

				if (current_stage > BIRTH_RACE_CHOICE) {
					menu_refresh(&race_menu, false);
					menu = &class_menu;
					command = CMD_CHOOSE_CLASS;
				}

				if (current_stage > BIRTH_CLASS_CHOICE) {
					menu_refresh(&class_menu, false);
					menu = &roller_menu;
				}

				next = menu_question(current_stage, menu, command);

				if (next == BIRTH_BACK)
					next = current_stage - 1;

				/* Make sure the character gets reset before quickstarting */
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

			case BIRTH_HISTORY_CHOICE:
			{
				if (prev < BIRTH_HISTORY_CHOICE)
					display_player(0);

				next = get_history_command();
				if (next == BIRTH_BACK)
					next = BIRTH_NAME_CHOICE;

				break;
			}

			case BIRTH_FINAL_CONFIRM:
			{
				if (prev < BIRTH_FINAL_CONFIRM)
					display_player(0);

				next = get_confirm_command();
				if (next == BIRTH_BACK)
					next = BIRTH_HISTORY_CHOICE;

				if (next == BIRTH_COMPLETE)
					done = true;

				break;
			}

			default:
			{
				/* Remove dodgy compiler warning, */
			}
		}

		prev = current_stage;
		current_stage = next;

		/* Execute whatever commands have been sent */
		cmdq_execute(CTX_BIRTH);
	}

	return 0;
}

/**
 * Called when we enter the birth mode - so we set up handlers, command hooks,
 * etc, here.
 */
static void ui_enter_birthscreen(game_event_type type, game_event_data *data,
								 void *user)
{
	/* Set the ugly static global that tells us if quickstart's available. */
	quickstart_allowed = data->flag;

	setup_menus();
}

static void ui_leave_birthscreen(game_event_type type, game_event_data *data,
								 void *user)
{
	/* Set the savefile name if it's not already set */
	if (!savefile[0])
		savefile_set_name(player->full_name, true, true);

	free_birth_menus();
}


void ui_init_birthstate_handlers(void)
{
	event_add_handler(EVENT_ENTER_BIRTH, ui_enter_birthscreen, NULL);
	event_add_handler(EVENT_LEAVE_BIRTH, ui_leave_birthscreen, NULL);
}

