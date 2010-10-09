/*
 * File: cmd4.c
 * Purpose: Various kinds of browsing functions.
 *
 * Copyright (c) 1997-2007 Robert A. Koeneke, James E. Wilson, Ben Harrison,
 * Eytan Zweig, Andrew Doull, Pete Mack.
 * HTML dump code (c) 2004 DarkGod 
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
#include "object/tvalsval.h"
#include "cmds.h"
#include "option.h"
#include "ui.h"
#include "externs.h"
#include "ui-menu.h"


static void dump_pref_file(void (*dump)(ang_file *), const char *title, int row)
{
	char ftmp[80];
	char buf[1024];

	/* Prompt */
	prt(format("%s to a pref file", title), row, 0);
	
	/* Prompt */
	prt("File: ", row + 2, 0);
	
	/* Default filename */
	strnfmt(ftmp, sizeof ftmp, "%s.prf", op_ptr->base_name);
	
	/* Get a filename */
	if (!askfor_aux(ftmp, sizeof ftmp, NULL)) return;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, ftmp);
	
	if (!prefs_save(buf, dump, title))
	{
		prt("", 0, 0);
		msg_print("Failed");
		return;
	}

	/* Message */
	prt("", 0, 0);
	msg_print(format("Dumped %s", strstr(title, " ")+1));
}

static void do_cmd_pref_file_hack(long row);



/* Flag value for missing array entry */
#define MISSING -17

#define APP_MACRO	101
#define ASK_MACRO	103
#define DEL_MACRO	104
#define NEW_MACRO	105
#define APP_KEYMAP	106
#define ASK_KEYMAP	107
#define DEL_KEYMAP	108
#define NEW_KEYMAP	109
#define ENTER_ACT	110
#define LOAD_PREF	111
#define DUMP_MON	112
#define DUMP_OBJ	113
#define DUMP_FEAT	114
#define DUMP_FLAV	115
#define DUMP_COL	120
#define MOD_COL		121
#define RESET_VIS	122


#define INFO_SCREENS 2 /* Number of screens in character info mode */



/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This command is also used to "instantiate" the results of the user
 * selecting various things, such as graphics mode, so it must call
 * the "TERM_XTRA_REACT" hook before redrawing the windows.
 *
 */
void do_cmd_redraw(void)
{
	int j;

	term *old = Term;


	/* Low level flush */
	Term_flush();

	/* Reset "inkey()" */
	flush();
	
	if (character_dungeon)
		verify_panel();


	/* Hack -- React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Combine and Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);


	/* Update torch */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw everything */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN | PR_EQUIP |
	                  PR_MESSAGE | PR_MONSTER | PR_OBJECT |
					  PR_MONLIST | PR_ITEMLIST);

	/* Clear screen */
	Term_clear();

	/* Hack -- update */
	handle_stuff();

	/* Place the cursor on the player */
	if (0 != character_dungeon)
		move_cursor_relative(p_ptr->px, p_ptr->py);


	/* Redraw every window */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* Dead window */
		if (!angband_term[j]) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Redraw */
		Term_redraw();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
	ui_event_data ke;

	int mode = 0;

	cptr p;

	/* Prompt */
	p = "['c' to change name, 'f' to file, 'h' to change mode, or ESC]";

	/* Save screen */
	screen_save();

	/* Forever */
	while (1)
	{
		/* Display the player */
		display_player(mode);

		/* Prompt */
		Term_putstr(2, 23, -1, TERM_WHITE, p);

		/* Query */
		ke = inkey_ex();

		/* Exit */
		if (ke.key == ESCAPE) break;

		/* Change name */
		if ((ke.key == 'c') || ((ke.key == '\xff') && (ke.mousey == 2) && (ke.mousex < 26)))
		{
			char namebuf[32] = "";

			if (get_name(namebuf, sizeof namebuf))
			{
				/* Set player name */
				my_strcpy(op_ptr->full_name, namebuf,
						  sizeof(op_ptr->full_name));

				/* Don't change savefile name. */
				process_player_name(FALSE);
			}
		}

		/* File dump */
		else if (ke.key == 'f')
		{
			char buf[1024];
			char fname[80];

			strnfmt(fname, sizeof fname, "%s.txt", op_ptr->base_name);

			if (get_file(fname, buf, sizeof buf))
			{
				if (file_character(buf, FALSE) != 0)
					msg_print("Character dump failed!");
				else
					msg_print("Character dump successful.");
			}
		}

		/* Toggle mode */
		else if ((ke.key == 'h') || (ke.key == '\xff') ||
		         (ke.key == ARROW_LEFT) || (ke.key == ' '))
		{
			mode = (mode + 1) % INFO_SCREENS;
		}

		/* Toggle mode */
		else if ((ke.key == 'l') || ke.key == ARROW_RIGHT)
		{
			mode = (mode - 1) % INFO_SCREENS;
		}


		/* Oops */
		else
		{
			bell(NULL);
		}

		/* Flush messages */
		message_flush();
	}

	/* Load screen */
	screen_load();
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
	/* Recall one message XXX XXX XXX */
	c_prt(message_color(0), format( "> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 *
 * Note that messages may be longer than 80 characters, but they are
 * displayed using "infinite" length, with a special sub-command to
 * "slide" the virtual display to the left or right.
 *
 * Attempt to only highlight the matching portions of the string.
 */
void do_cmd_messages(void)
{
	ui_event_data ke;

	int i, j, n, q;
	int wid, hgt;

	char shower[80] = "";



	/* Total messages */
	n = messages_num();

	/* Start on first message */
	i = 0;

	/* Start at leftmost edge */
	q = 0;

	/* Get size */
	Term_get_size(&wid, &hgt);

	/* Save screen */
	screen_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Dump messages */
		for (j = 0; (j < hgt - 4) && (i + j < n); j++)
		{
			const char *msg;
			const char *str = message_str(i + j);
			byte attr = message_color(i + j);
			u16b count = message_count(i + j);

			if (count == 1)
				msg = str;
			else
				msg = format("%s <%dx>", str, count);

			/* Apply horizontal scroll */
			msg = ((int)strlen(msg) >= q) ? (msg + q) : "";

			/* Dump the messages, bottom to top */
			Term_putstr(0, hgt - 3 - j, -1, attr, msg);

			/* Highlight "shower" */
			if (shower[0])
			{
				str = msg;

				/* Display matches */
				while ((str = my_stristr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-msg, hgt - 3 - j, len, TERM_YELLOW, str);

					/* Advance */
					str += len;
				}
			}
		}

		/* Display header */
		prt(format("Message recall (%d-%d of %d), offset %d", i, i + j - 1, n, q), 0, 0);

		/* Display prompt (not very informative) */
		if (shower[0])
		    prt("[Movement keys to navigate, '-' for next, '=' to find]", hgt - 1, 0);
		else
			prt("[Movement keys to navigate, '=' to find, or ESCAPE to exit]", hgt - 1, 0);
			

		/* Get a command */
		ke = inkey_ex();


		/* Exit on Escape */
		if (ke.key == ESCAPE)
		{
			break;
		}

		/* Find text */
		else if (ke.key == '=')
		{
			/* Get the string to find */
			prt("Find: ", hgt - 1, 0);
			if (!askfor_aux(shower, sizeof shower, NULL)) continue;

			/* Set to find */
			ke.key = '-';
		}

		/* Horizontal scroll */
		else if (ke.key == '4' || ke.key == ARROW_LEFT)
		{
			/* Scroll left */
			q = (q >= wid / 2) ? (q - wid / 2) : 0;

			/* Success */
			continue;
		}

		/* Horizontal scroll */
		else if (ke.key == '6'|| ke.key == ARROW_RIGHT)
		{
			/* Scroll right */
			q = q + wid / 2;

			/* Success */
			continue;
		}

		/* Recall 1 older message */
		else if (ke.key == '8' || ke.key == ARROW_UP)
		{
			/* Go older if legal */
			if (i + 1 < n) i += 1;
		}

		/* Recall 1 newer messages */
		else if (ke.key == '2' || ke.key == ARROW_DOWN || ke.key == '\r' || ke.key == '\n')
		{
			/* Go newer (if able) */
			i = (i >= 1) ? (i - 1) : 0;
		}

		/* Recall 20 older messages */
		else if ((ke.key == 'p') || (ke.key == KTRL('P')) || (ke.key == ' '))
		{
			/* Go older if legal */
			if (i + 20 < n) i += 20;
		}

		/* Recall 20 newer messages */
		else if ((ke.key == 'n') || (ke.key == KTRL('N')))
		{
			/* Go newer (if able) */
			i = (i >= 20) ? (i - 20) : 0;
		}

		/* Scroll forwards or backwards using mouse clicks */
		else if (ke.key == '\xff')
		{
			if (ke.index)
			{
				if (ke.mousey <= hgt / 2)
				{
					/* Go older if legal */
					if (i + 20 < n) i += 20;
				}
				else
				{
					/* Go newer (if able) */
					i = (i >= 20) ? (i - 20) : 0;
				}
			}
		}

		/* Error time */
		else
		{
			bell(NULL);
		}


		/* Find the next item */
		if (ke.key == '-' && shower[0])
		{
			s16b z;

			/* Scan messages */
			for (z = i + 1; z < n; z++)
			{
				/* Search for it */
				if (my_stristr(message_str(z), shower))
				{
					/* New location */
					i = z;

					/* Done */
					break;
				}
			}
		}
	}

	/* Load screen */
	screen_load();
}




/*** Options display and setting ***/

/*
 * Displays an option entry.
 */
static void display_option(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	c_prt(attr, format("%-45s: %s  (%s)", option_desc(oid),
	                   op_ptr->opt[oid] ? "yes" : "no ", option_name(oid)),
	                   row, col);
}

/*
 * Handle keypresses for an option entry.
 */
static bool handle_option(menu_type *m, const ui_event_data *event, int oid)
{
	bool next = FALSE;

	if (event->type == EVT_SELECT)
		op_ptr->opt[oid] = !op_ptr->opt[oid];
	else if (event->type == EVT_KBRD)
	{
		if (event->key == 'y' || event->key == 'Y')
		{
			op_ptr->opt[oid] = TRUE;
			next = TRUE;
		}
		else if (event->key == 'n' || event->key == 'N')
		{
			op_ptr->opt[oid] = FALSE;
			next = TRUE;
		}
		else if (event->key == '?')
			show_file(format("option.txt#%s", option_name(oid)), NULL, 0, 0);
		else
			return FALSE;
	}
	else
		return FALSE;

	if (next)
	{
		m->cursor++;
		m->cursor = (m->cursor + m->filter_count) % m->filter_count;
	}

	return TRUE;
}

static const menu_iter options_toggle_iter =
{
	NULL,
	NULL,
	display_option,		/* label */
	handle_option		/* handle */
};

static menu_type option_toggle_menu;


/*
 * Interact with some options
 */
static void do_cmd_options_aux(void *vpage, cptr info)
{
	int page = (int)vpage;
	int opt[OPT_PAGE_PER];
	int i, n = 0;

	menu_type *menu = &option_toggle_menu;

	/* Filter the options for this page */
	for (i = 0; i < OPT_PAGE_PER; i++)
	{
		if (option_page[page][i] != OPT_NONE)
			opt[n++] = option_page[page][i];
	}

	menu->title = info;

	menu_setpriv(menu, OPT_PAGE_PER, vpage);
	menu_set_filter(menu, opt, n);
	menu_layout(menu, &SCREEN_REGION);

	/* Run the menu */
	screen_save();
	clear_from(0);

	menu_select(menu, 0);

	/* Hack -- Notice use of any "cheat" options */
	/* XXX this should be moved to option_set() */
	for (i = OPT_CHEAT; i < OPT_ADULT; i++)
	{
		if (op_ptr->opt[i])
		{
			/* Set score option */
			op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = TRUE;
		}
	}

	screen_load();
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
	int i, j, d;

	int y = 0;
	int x = 0;

	ui_event_data ke;

	u32b new_flags[ANGBAND_TERM_MAX];


	/* Set new flags to the old values */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		new_flags[j] = op_ptr->window_flag[j];
	}


	/* Clear screen */
	clear_from(0);

	/* Interact */
	while (1)
	{
		/* Prompt */
		prt("Window flags (<dir> to move, 't'/Enter to toggle, or ESC)", 0, 0);

		/* Display the windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++)
		{
			byte a = TERM_WHITE;

			cptr s = angband_term_name[j];

			/* Use color */
			if (j == x) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < PW_MAX_FLAGS; i++)
		{
			byte a = TERM_WHITE;

			cptr str = window_flag_desc[i];

			/* Use color */
			if (i == y) a = TERM_L_BLUE;

			/* Unused option */
			if (!str) str = "(Unused option)";

			/* Flag name */
			Term_putstr(0, i + 5, -1, a, str);

			/* Display the windows */
			for (j = 0; j < ANGBAND_TERM_MAX; j++)
			{
				char c = '.';

				a = TERM_WHITE;

				/* Use color */
				if ((i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (new_flags[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ke = inkey_ex();

		/* Allow escape */
		if ((ke.key == ESCAPE) || (ke.key == 'q')) break;

		/* Mouse interaction */
		if (ke.key == '\xff')
		{
			int choicey = ke.mousey - 5;
			int choicex = (ke.mousex - 35)/5;

			if ((choicey >= 0) && (choicey < PW_MAX_FLAGS)
				&& (choicex > 0) && (choicex < ANGBAND_TERM_MAX)
				&& !(ke.mousex % 5))
			{
				y = choicey;
				x = (ke.mousex - 35)/5;
			}

			/* Toggle using mousebutton later */
			if (!ke.index) continue;
		}

		/* Toggle */
		if ((ke.key == '5') || (ke.key == 't') || (ke.key == '\n') || (ke.key == '\r') || ((ke.key == '\xff') && (ke.index)))
		{
			/* Hack -- ignore the main window */
			if (x == 0)
			{
				bell("Cannot set main window flags!");
			}

			/* Toggle flag (off) */
			else if (new_flags[x] & (1L << y))
			{
				new_flags[x] &= ~(1L << y);
			}

			/* Toggle flag (on) */
			else
			{
				new_flags[x] |= (1L << y);
			}

			/* Continue */
			continue;
		}

		/* Extract direction */
		d = target_dir(ke.key);

		/* Move */
		if (d != 0)
		{
			x = (x + ddx[d] + 8) % ANGBAND_TERM_MAX;
			y = (y + ddy[d] + 16) % PW_MAX_FLAGS;
		}

		/* Oops */
		else
		{
			bell("Illegal command for window options!");
		}
	}

	/* Notice changes */
	subwindows_set_flags(new_flags, ANGBAND_TERM_MAX);
}


#ifdef ALLOW_MACROS

/*
 * Hack -- ask for a "trigger" (see below)
 *
 * Note the complex use of the "inkey()" function from "util.c".
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux(char *buf)
{
	char ch;

	int n = 0;
	int curs_x, curs_y;

	char tmp[1024] = "";

	/* Get cursor position */
	Term_locate(&curs_x, &curs_y);

	/* Flush */
	flush();


	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	ch = inkey();


	/* Read the pattern */
	while (ch != 0 && ch != '\xff')
	{
		/* Save the key */
		buf[n++] = ch;
		buf[n] = 0;

		/* Get representation of the sequence so far */
		ascii_to_text(tmp, sizeof(tmp), buf);

		/* Echo it after the prompt */
		Term_erase(curs_x, curs_y, 80);
		Term_gotoxy(curs_x, curs_y);
		Term_addstr(-1, TERM_WHITE, tmp);
		
		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = SCAN_INSTANT;

		/* Attempt to read a key */
		ch = inkey();
	}

	/* Convert the trigger */
	ascii_to_text(tmp, sizeof(tmp), buf);
}


/*
 * Hack -- ask for a keymap "trigger" (see below)
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux_keymap(char *buf)
{
	char tmp[1024];

	/* Flush */
	flush();

	/* Get a key */
	buf[0] = inkey();
	buf[1] = '\0';

	/* Convert to ascii */
	ascii_to_text(tmp, sizeof(tmp), buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);

	/* Flush */
	flush();
}

#endif


/*
 * Interact with "macros"
 *
 * Could use some helpful instructions on this page.  XXX XXX XXX
 * CLEANUP
 */
static menu_action macro_actions[] =
{
	{ LOAD_PREF,  "Load a user pref file",    0, 0 },
#ifdef ALLOW_MACROS
	{ APP_MACRO,  "Append macros to a file",  0, 0 },
	{ ASK_MACRO,  "Query a macro",            0, 0 },
	{ NEW_MACRO,  "Create a macro",           0, 0 },
	{ DEL_MACRO,  "Remove a macro",           0, 0 },
	{ APP_KEYMAP, "Append keymaps to a file", 0, 0 },
	{ ASK_KEYMAP, "Query a keymap",           0, 0 },
	{ NEW_KEYMAP, "Create a keymap",          0, 0 },
	{ DEL_KEYMAP, "Remove a keymap",          0, 0 },
	{ ENTER_ACT,  "Enter a new action",       0, 0 },
#endif /* ALLOW_MACROS */
};

static menu_type macro_menu;


void do_cmd_macros(void)
{
	char tmp[1024];

	char pat[1024];

	int mode;

	region loc = {0, 0, 0, 12};

	if (OPT(rogue_like_commands))
		mode = KEYMAP_MODE_ROGUE;
	else
		mode = KEYMAP_MODE_ORIG;


	screen_save();

	menu_layout(&macro_menu, &loc);

	/* Process requests until done */
	while (1)
	{
		ui_event_data c;
		int evt;

		/* Clear screen */
		clear_from(0);

		/* Describe current action */
		prt("Current action (if any) shown below:", 13, 0);

		/* Analyze the current action */
		ascii_to_text(tmp, sizeof(tmp), macro_buffer);

		/* Display the current action */
		prt(tmp, 14, 0);

		c = menu_select(&macro_menu, 0);

		if (c.type == EVT_ESCAPE)
			break;

		if (c.type == EVT_KBRD && (c.key == ARROW_LEFT || c.key == ARROW_RIGHT))
			continue;

		evt = macro_actions[macro_menu.cursor].id;

		switch(evt)
		{
		case LOAD_PREF:
		{
			do_cmd_pref_file_hack(16);
			break;
		}

#ifdef ALLOW_MACROS
		case APP_MACRO:
		{
			/* Dump the macros */
			(void)dump_pref_file(macro_dump, "Dump Macros", 15);

			break;
		}

		case ASK_MACRO:
		{
			int k;

			/* Prompt */
			prt("Command: Query a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(pat);

			/* Get the action */
			k = macro_find_exact(pat);

			/* Nothing found */
			if (k < 0)
			{
				/* Prompt */
				prt("", 0, 0);
				msg_print("Found no macro.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				my_strcpy(macro_buffer, macro__act[k], sizeof(macro_buffer));

				/* Analyze the current action */
				ascii_to_text(tmp, sizeof(tmp), macro_buffer);

				/* Display the current action */
				prt(tmp, 22, 0);

				/* Prompt */
				prt("", 0, 0);
				msg_print("Found a macro.");
			}
			break;
		}

		case NEW_MACRO:
		{
			/* Prompt */
			prt("Command: Create a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(pat);

			/* Clear */
			clear_from(20);

			/* Prompt */
			prt("Action: ", 20, 0);

			/* Convert to text */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp, NULL))
			{
				/* Convert to ascii */
				text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);

				/* Link the macro */
				macro_add(pat, macro_buffer);

				/* Prompt */
				prt("", 0, 0);
				msg_print("Added a macro.");
			}
			break;
		}

		case DEL_MACRO:
		{
			/* Prompt */
			prt("Command: Remove a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(pat);

			/* Link the macro */
			macro_add(pat, pat);

			/* Prompt */
			prt("", 0, 0);
			msg_print("Removed a macro.");
			break;
		}
		case APP_KEYMAP:
		{
			/* Dump the keymaps */
			(void)dump_pref_file(keymap_dump, "Dump Keymaps", 15);
			break;
		}
		case ASK_KEYMAP:
		{
			cptr act;

			/* Prompt */
			prt("Command: Query a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(pat);

			/* Look up the keymap */
			act = keymap_act[mode][(byte)(pat[0])];

			/* Nothing found */
			if (!act)
			{
				/* Prompt */
				prt("", 0, 0);
				msg_print("Found no keymap.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				my_strcpy(macro_buffer, act, sizeof(macro_buffer));

				/* Analyze the current action */
				ascii_to_text(tmp, sizeof(tmp), macro_buffer);

				/* Display the current action */
				prt(tmp, 22, 0);

				/* Prompt */
				prt("", 0, 0);
				msg_print("Found a keymap.");
			}
			break;
		}
		case NEW_KEYMAP:
		{
			/* Prompt */
			prt("Command: Create a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(pat);

			/* Clear */
			clear_from(20);

			/* Prompt */
			prt("Action: ", 20, 0);

			/* Convert to text */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp, NULL))
			{
				/* Convert to ascii */
				text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);

				/* Free old keymap */
				string_free(keymap_act[mode][(byte)(pat[0])]);

				/* Make new keymap */
				keymap_act[mode][(byte)(pat[0])] = string_make(macro_buffer);

				/* Prompt */
				prt("", 0, 0);
				msg_print("Added a keymap.");
			}
			break;
		}
		case DEL_KEYMAP:
		{
			/* Prompt */
			prt("Command: Remove a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(pat);

			/* Free old keymap */
			string_free(keymap_act[mode][(byte)(pat[0])]);

			/* Make new keymap */
			keymap_act[mode][(byte)(pat[0])] = NULL;

			/* Prompt */
			prt("", 0, 0);
			msg_print("Removed a keymap.");
			break;
		}
		case ENTER_ACT: /* Enter a new action */
		{
			/* Prompt */
			prt("Command: Enter a new action", 16, 0);

			/* Go to the correct location */
			Term_gotoxy(0, 22);

			/* Analyze the current action */
			ascii_to_text(tmp, sizeof(tmp), macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, sizeof tmp, NULL))
			{
				/* Extract an action */
				text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);
			}
			break;
		}
#endif /* ALLOW_MACROS */
		}

		/* Flush messages */
		message_flush();
	}

	/* Load screen */
	screen_load();
}

menu_action visual_menu_items [] =
{
	{ LOAD_PREF, "Load a user pref file", 0, 0},
	{ DUMP_MON,  "Dump monster attr/chars", 0, 0},
	{ DUMP_OBJ,  "Dump object attr/chars", 0, 0 },
	{ DUMP_FEAT, "Dump feature attr/chars", 0, 0 },
	{ DUMP_FLAV, "Dump flavor attr/chars", 0, 0 },
	{ RESET_VIS, "Reset visuals", 0, 0 },
};

static menu_type visual_menu;


/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
	/* Save screen */
	screen_save();

	menu_layout(&visual_menu, &SCREEN_REGION);

	/* Interact until done */
	while (1)
	{
		ui_event_data key;
		int evt = -1;

		clear_from(0);

		key = menu_select(&visual_menu, 0);

		if (key.type == EVT_ESCAPE)
			break;

		if (key.type == EVT_KBRD && (key.key == ARROW_LEFT || key.key == ARROW_RIGHT))
			continue;

		evt = visual_menu_items[visual_menu.cursor].id;

		if (evt == LOAD_PREF)
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(15);
		}

#ifdef ALLOW_VISUALS

		else if (evt == DUMP_MON)
		{
			dump_pref_file(dump_monsters, "Dump Monster attr/chars", 15);
		}

		else if (evt == DUMP_OBJ)
		{
			dump_pref_file(dump_objects, "Dump Object attr/chars", 15);
		}

		else if (evt == DUMP_FEAT)
		{
			dump_pref_file(dump_features, "Dump Feature attr/chars", 15);
		}

		/* Dump flavor attr/chars */
		else if (evt == DUMP_FLAV) 
		{
			dump_pref_file(dump_flavors, "Dump Flavor attr/chars", 15);
		}

#endif /* ALLOW_VISUALS */

		/* Reset visuals */
		else if (evt == RESET_VIS)
		{
			/* Reset */
			reset_visuals(TRUE);

			/* Message */
			prt("", 0, 0);
			msg_print("Visual attr/char tables reset.");
		}

		message_flush();
	}

	/* Load screen */
	screen_load();
}


static menu_action color_events [] =
{
	{LOAD_PREF, "Load a user pref file", 0, 0},
#ifdef ALLOW_COLORS
	{DUMP_COL, "Dump colors", 0, 0},
	{MOD_COL, "Modify colors", 0, 0}
#endif
};

static menu_type color_menu;


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
	int i;
	int cx;

	screen_save();

	menu_layout(&color_menu, &SCREEN_REGION);

	/* Interact until done */
	while (1)
	{
		ui_event_data key;
		int evt;
		clear_from(0);
		key = menu_select(&color_menu, 0);

		/* Done */
		if (key.type == EVT_ESCAPE) break;
		if (key.key == ARROW_RIGHT || key.key == ARROW_LEFT) continue;

		evt = color_events[color_menu.cursor].id;

		/* Load a user pref file */
		if (evt == LOAD_PREF)
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(8);

			/* Could skip the following if loading cancelled XXX XXX XXX */

			/* Mega-Hack -- React to color changes */
			Term_xtra(TERM_XTRA_REACT, 0);

			/* Mega-Hack -- Redraw physical windows */
			Term_redraw();
		}

#ifdef ALLOW_COLORS

		/* Dump colors */
		else if (evt == DUMP_COL)
		{
			dump_pref_file(dump_colors, "Dump Colors", 15);
		}

		/* Edit colors */
		else if (evt == MOD_COL)
		{
			static byte a = 0;

			/* Prompt */
			prt("Command: Modify colors", 8, 0);

			/* Hack -- query until done */
			while (1)
			{
				cptr name;
				char index;

				/* Clear */
				clear_from(10);

				/* Exhibit the normal colors */
				for (i = 0; i < BASIC_COLORS; i++)
				{
					/* Exhibit this color */
					Term_putstr(i*3, 20, -1, a, "##");

					/* Exhibit character letter */
					Term_putstr(i*3, 21, -1, (byte)i,
								format(" %c", color_table[i].index_char));

					/* Exhibit all colors */
					Term_putstr(i*3, 22, -1, (byte)i, format("%2d", i));
				}

				/* Describe the color */
				name = ((a < BASIC_COLORS) ? color_table[a].name : "undefined");
				index = ((a < BASIC_COLORS) ? color_table[a].index_char : '?');

				/* Describe the color */
				Term_putstr(5, 10, -1, TERM_WHITE,
							format("Color = %d, Name = %s, Index = %c", a, name, index));

				/* Label the Current values */
				Term_putstr(5, 12, -1, TERM_WHITE,
					    format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
						   angband_color_table[a][0],
						   angband_color_table[a][1],
						   angband_color_table[a][2],
						   angband_color_table[a][3]));

				/* Prompt */
				Term_putstr(0, 14, -1, TERM_WHITE,
					    "Command (n/N/k/K/r/R/g/G/b/B): ");

				/* Get a command */
				cx = inkey();

				/* All done */
				if (cx == ESCAPE) break;

				/* Analyze */
				if (cx == 'n') a = (byte)(a + 1);
				if (cx == 'N') a = (byte)(a - 1);
				if (cx == 'k') angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
				if (cx == 'K') angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
				if (cx == 'r') angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
				if (cx == 'R') angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
				if (cx == 'g') angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
				if (cx == 'G') angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
				if (cx == 'b') angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
				if (cx == 'B') angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

				/* Hack -- react to changes */
				Term_xtra(TERM_XTRA_REACT, 0);

				/* Hack -- redraw */
				Term_redraw();
			}
		}

#endif /* ALLOW_COLORS */
		message_flush();

		/* Clear screen */
		clear_from(0);
	}


	/* Load screen */
	screen_load();
}



/*** Non-complex menu actions ***/

static bool askfor_aux_numbers(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	switch (keypress)
	{
		case ESCAPE:
		case '\n':
		case '\r':
		case ARROW_LEFT:
		case ARROW_RIGHT:
		case 0x7F:
		case '\010':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return askfor_aux_keypress(buf, buflen, curs, len, keypress, firsttime);
	}

	return FALSE;
}


/*
 * Set base delay factor
 */
static void do_cmd_delay(void)
{
	bool res;
	char tmp[4] = "";
	int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	strnfmt(tmp, sizeof(tmp), "%i", op_ptr->delay_factor);

	/* Prompt */
	prt("Command: Base Delay Factor", 20, 0);

	prt(format("Current base delay factor: %d (%d msec)",
			   op_ptr->delay_factor, msec), 22, 0);
	prt("New base delay factor (0-255): ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res)
	{
		op_ptr->delay_factor = (u16b) strtoul(tmp, NULL, 0);
	}
}


/*
 * Set hitpoint warning level
 */
static void do_cmd_hp_warn(void)
{
	bool res;
	char tmp[4] = "";
	u16b warn;

	strnfmt(tmp, sizeof(tmp), "%i", op_ptr->hitpoint_warn);

	/* Prompt */
	prt("Command: Hitpoint Warning", 20, 0);

	prt(format("Current hitpoint warning: %d (%d%%)",
			   op_ptr->hitpoint_warn, op_ptr->hitpoint_warn * 10), 22, 0);
	prt("New hitpoint warning (0-9): ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res)
	{
		warn = (u16b) strtoul(tmp, NULL, 0);
		
		/* Reset nonsensical warnings */
		if (warn > 9)
			warn = 0;

		op_ptr->hitpoint_warn = warn;
	}
}


/*
 * Set "lazy-movement" delay
 */
static void do_cmd_lazymove_delay(void)
{
	bool res;
	char tmp[4] = "";

	strnfmt(tmp, sizeof(tmp), "%i", lazymove_delay);

	/* Prompt */
	prt("Command: Movement Delay Factor", 20, 0);

	prt(format("Current movement delay: %d (%d msec)",
			   lazymove_delay, lazymove_delay * 10), 22, 0);
	prt("New movement delay: ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res)
	{
		lazymove_delay = (u16b) strtoul(tmp, NULL, 0);
	}
}



/*
 * Ask for a "user pref file" and process it.
 *
 * This function should only be used by standard interaction commands,
 * in which a standard "Command:" prompt is present on the given row.
 *
 * Allow absolute file names?  XXX XXX XXX
 */
static void do_cmd_pref_file_hack(long row)
{
	char ftmp[80];

	/* Prompt */
	prt("Command: Load a user pref file", row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	strnfmt(ftmp, sizeof ftmp, "%s.prf", op_ptr->base_name);

	/* Ask for a file (or cancel) */
	if (!askfor_aux(ftmp, sizeof ftmp, NULL)) return;

	/* Process the given filename */
	if (process_pref_file(ftmp))
	{
		/* Mention failure */
		prt("", 0, 0);
		msg_format("Failed to load '%s'!", ftmp);
	}
	else
	{
		/* Mention success */
		prt("", 0, 0);
		msg_format("Loaded '%s'.", ftmp);
	}
}


/*
 * Write options to a file.
 */
static void do_dump_options(void *unused, const char *title)
{
	(void)unused;
	dump_pref_file(option_dump, "Dump options", 20);
}



/*** Main menu definitions and display ***/

static menu_type option_menu;
static menu_action option_actions [] = 
{
	{'a', "Interface options", do_cmd_options_aux, (void*)0}, 
	{'b', "Display options", do_cmd_options_aux, (void*)1},
	{'e', "Warning and disturbance options", do_cmd_options_aux, (void*)2}, 
	{'f', "Birth (difficulty) options", do_cmd_options_aux, (void*)3}, 
	{'g', "Cheat options", do_cmd_options_aux, (void*)4}, 
	{0, 0, 0, 0}, /* Load and append */
	{'w', "Subwindow display settings", (action_f) do_cmd_options_win, 0}, 
	{'s', "Item squelch settings", (action_f) do_cmd_options_item, 0}, 
	{'d', "Set base delay factor", (action_f) do_cmd_delay, 0}, 
	{'h', "Set hitpoint warning", (action_f) do_cmd_hp_warn, 0}, 
	{'i', "Set movement delay", (action_f) do_cmd_lazymove_delay, 0}, 
	{'l', "Load a user pref file", (action_f) do_cmd_pref_file_hack, (void*)20},
	{'o', "Save options", do_dump_options, 0}, 
	{0, 0, 0, 0}, /* Interact with */	
	{'m', "Interact with macros (advanced)", (action_f) do_cmd_macros, 0},
	{'v', "Interact with visuals (advanced)", (action_f) do_cmd_visuals, 0},
	{'c', "Interact with colours (advanced)", (action_f) do_cmd_colors, 0},
};

/*
 * Display the options main menu.
 */
void do_cmd_options(void)
{
	screen_save();
	clear_from(0);
	menu_layout(&option_menu, &SCREEN_REGION);
	menu_select(&option_menu, 0);
	screen_load();
}




/*
 * Initialise all menus used here.
 */
void init_cmd4_c(void)
{
	/* Initialize the menus */
	menu_type *menu;

	/* options screen selection menu */
	menu = &option_menu;
	menu_init(menu, MN_SKIN_SCROLL, find_menu_iter(MN_ITER_ACTIONS));
	menu_setpriv(menu, N_ELEMENTS(option_actions), option_actions);

	menu->title = "Options Menu";
	menu->flags = MN_CASELESS_TAGS;


	/* Initialize the options toggle menu */
	menu = &option_toggle_menu;
	menu_init(menu, MN_SKIN_SCROLL, &options_toggle_iter);

	menu->prompt = "Set option (y/n/t), '?' for information";
	menu->cmd_keys = "?YyNnTt";
	menu->selections = "abcdefghijklmopqrsuvwxz";
	menu->flags = MN_DBL_TAP;


	/* macro menu */
	menu = &macro_menu;
	menu_init(menu, MN_SKIN_SCROLL, find_menu_iter(MN_ITER_ACTIONS));
	menu_setpriv(menu, N_ELEMENTS(macro_actions), macro_actions);

	menu->title = "Interact with macros";
	menu->selections = lower_case;


	/* visuals menu */
	menu = &visual_menu;
	menu_init(menu, MN_SKIN_SCROLL, find_menu_iter(MN_ITER_ACTIONS));
	menu_setpriv(menu, N_ELEMENTS(visual_menu_items), visual_menu_items);

	menu->title = "Interact with visuals";
	menu->selections = lower_case;


	/* colors menu */
	menu = &color_menu;
	menu_init(menu, MN_SKIN_SCROLL, find_menu_iter(MN_ITER_ACTIONS));
	menu_setpriv(menu, N_ELEMENTS(color_events), color_events);

	menu->title = "Interact with colors";
	menu->selections = lower_case;
}








/*** Non-knowledge/option stuff ***/

/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
	char tmp[80];

	/* Default */
	my_strcpy(tmp, "", sizeof(tmp));

	/* Input */
	if (!get_string("Note: ", tmp, 80)) return;

	/* Ignore empty notes */
	if (!tmp[0] || (tmp[0] == ' ')) return;

	/* Add the note to the message recall */
	msg_format("Note: %s", tmp);

	/* Add a history entry */
	history_add(tmp, HISTORY_USER_INPUT, 0);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
	/* Silly message */
	msg_format("You are playing %s %s.  Type '?' for more info.",
		       VERSION_NAME, VERSION_STRING);
}


/*
 * Ask for a "user pref line" and process it
 */
void do_cmd_pref(void)
{
	char tmp[80];

	/* Default */
	my_strcpy(tmp, "", sizeof(tmp));

	/* Ask for a "user pref command" */
	if (!get_string("Pref: ", tmp, 80)) return;

	/* Process that pref command */
	(void)process_pref_file_command(tmp);
}



/*
 * Array of feeling strings
 */
static const char *feeling_text[] =
{
	"Looks like any other level.",
	"You feel there is something special here...",
	"You have a superb feeling about this level.",
	"You have an excellent feeling...",
	"You have a very good feeling...",
	"You have a good feeling...",
	"You feel a little lucky.",
	"You are unsure about this place.",
	"This place seems reasonably safe.",
	"This seems a quiet, peaceful place.",
	"This place looks uninteresting.",
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
	/* Don't show feelings for cold-hearted characters */
	if (OPT(birth_feelings)) return;

	/* Verify the feeling */
	if (feeling >= N_ELEMENTS(feeling_text))
		feeling = N_ELEMENTS(feeling_text) - 1;

	/* No useful feeling in town */
	if (!p_ptr->depth)
	{
		msg_print("Looks like a typical town.");
		return;
	}

	/* Display the feeling */
	msg_print(feeling_text[feeling]);
}



/*** Screenshot loading/saving code ***/

/*
 * Encode the screen colors
 */
static const char hack[BASIC_COLORS+1] = "dwsorgbuDWvyRGBU";


/*
 * Hack -- load a screen dump from a file
 *
 * ToDo: Add support for loading/saving screen-dumps with graphics
 * and pseudo-graphics.  Allow the player to specify the filename
 * of the dump.
 */
void do_cmd_load_screen(void)
{
	int i, y, x;

	byte a = 0;
	char c = ' ';

	bool okay = TRUE;

	ang_file *fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");
	fp = file_open(buf, MODE_READ, -1);
	if (!fp) return;


	/* Save screen */
	screen_save();


	/* Clear the screen */
	Term_clear();


	/* Load the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (!file_getl(fp, buf, sizeof(buf))) okay = FALSE;


		/* Show each row */
		for (x = 0; x < 79; x++)
		{
			/* Put the attr/char */
			Term_draw(x, y, TERM_WHITE, buf[x]);
		}
	}

	/* Get the blank line */
	if (!file_getl(fp, buf, sizeof(buf))) okay = FALSE;


	/* Dump the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (!file_getl(fp, buf, sizeof(buf))) okay = FALSE;

		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Look up the attr */
			for (i = 0; i < BASIC_COLORS; i++)
			{
				/* Use attr matches */
				if (hack[i] == buf[x]) a = i;
			}

			/* Put the attr/char */
			Term_draw(x, y, a, c);
		}
	}


	/* Close it */
	file_close(fp);


	/* Message */
	msg_print("Screen dump loaded.");
	message_flush();


	/* Load screen */
	screen_load();
}


/*
 * Save a simple text screendump.
 */
static void do_cmd_save_screen_text(void)
{
	int y, x;

	byte a = 0;
	char c = ' ';

	ang_file *fff;

	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");
	fff = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	if (!fff) return;


	/* Save screen */
	screen_save();


	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		file_putf(fff, "%s\n", buf);
	}

	/* Skip a line */
	file_putf(fff, "\n");


	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = hack[a & 0x0F];
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		file_putf(fff, "%s\n", buf);
	}

	/* Skip a line */
	file_putf(fff, "\n");


	/* Close it */
	file_close(fff);


	/* Message */
	msg_print("Screen dump saved.");
	message_flush();


	/* Load screen */
	screen_load();
}


/*
 * Hack -- save a screen dump to a file in html format
 */
static void do_cmd_save_screen_html(int mode)
{
	size_t i;

	ang_file *fff;
	char file_name[1024];
	char tmp_val[256];

	typedef void (*dump_func)(ang_file *);
	dump_func dump_visuals [] = 
		{ dump_monsters, dump_features, dump_objects, dump_flavors, dump_colors };


	if (mode == 0)
		my_strcpy(tmp_val, "dump.html", sizeof(tmp_val));
	else
		my_strcpy(tmp_val, "dump.txt", sizeof(tmp_val));

	/* Ask for a file */
	if (!get_string("File: ", tmp_val, sizeof(tmp_val))) return;

	/* Save current preferences */
	path_build(file_name, 1024, ANGBAND_DIR_USER, "dump.prf");
	fff = file_open(file_name, MODE_WRITE, (mode == 0 ? FTYPE_HTML : FTYPE_TEXT));

	/* Check for failure */
	if (!fff)
	{
		msg_print("Screen dump failed.");
		message_flush();
		return;
	}

	/* Dump all the visuals */
	for (i = 0; i < N_ELEMENTS(dump_visuals); i++)
		dump_visuals[i](fff);

	file_close(fff);

	/* Dump the screen with raw character attributes */
	reset_visuals(FALSE);
	do_cmd_redraw();
	html_screenshot(tmp_val, mode);

	/* Recover current graphics settings */
	reset_visuals(TRUE);
	process_pref_file(file_name);
	file_delete(file_name);
	do_cmd_redraw();

	msg_print("HTML screen dump saved.");
	message_flush();
}


/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
	msg_print("Dump type [(t)ext; (h)tml; (f)orum embedded html]:");

	while (TRUE)
	{
		char c = inkey();

		switch (c)
		{
			case ESCAPE:
				return;

			case 't':
				do_cmd_save_screen_text();
				return;

			case 'h':
				do_cmd_save_screen_html(0);
				return;

			case 'f':
				do_cmd_save_screen_html(1);
				return;
		}
	}
}

