/*
 * File: cmd4.c
 * Purpose: Various kinds of browsing functions.
 *
 * Copyright (c) 1997-2007 Robert A. Koeneke, James E. Wilson, Ben Harrison,
 * Eytan Zweig, Andrew Doull, Pete Mack.
 * Copyright (c) 2004 DarkGod (HTML dump code)
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
#include "externs.h"
#include "files.h"
#include "history.h"
#include "macro.h"
#include "object/tvalsval.h"
#include "option.h"
#include "prefs.h"
#include "squelch.h"
#include "ui.h"
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
		if (ke.key == 'c' ||
			(ke.mousey == 2 && ke.mousex < 26))
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
		else if (ke.key == 'h' || ke.key == ARROW_LEFT ||
				ke.key == ' ' || ke.type == EVT_MOUSE)
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


		/* Scroll forwards or backwards using mouse clicks */
		if (ke.type == EVT_MOUSE)
		{
			/* Go older if legal */
			if (ke.mousey <= hgt / 2)
			{
				if (i + 20 < n)
					i += 20;
			}

			/* Go newer (if able) */
			else
			{
				i = (i >= 20) ? (i - 20) : 0;
			}
		}

		/* Exit on Escape */
		else if (ke.key == ESCAPE)
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
	bool *options = menu_priv(m);

	c_prt(attr, format("%-45s: %s  (%s)", option_desc(oid),
	                   options[oid] ? "yes" : "no ", option_name(oid)),
	                   row, col);
}

/*
 * Handle keypresses for an option entry.
 */
static bool handle_option(menu_type *m, const ui_event_data *event, int oid)
{
	bool next = FALSE;

	if (event->type == EVT_SELECT)
	{
		option_set(option_name(oid), !op_ptr->opt[oid]);
	}
	else if (event->type == EVT_KBRD)
	{
		if (event->key == 'y' || event->key == 'Y')
		{
			option_set(option_name(oid), TRUE);
			next = TRUE;
		}
		else if (event->key == 'n' || event->key == 'N')
		{
			option_set(option_name(oid), FALSE);
			next = TRUE;
		}
		else if (event->key == '?')
			show_file(format("option.txt#%s", option_name(oid)), NULL, 0, 0);
		else
			return FALSE;
	}
	else
		return FALSE;

	/* XXX should be moved to ui-menu somehow */
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
	handle_option,		/* handle */
	NULL
};

static menu_type *option_toggle_menu;


/*
 * Interact with some options
 */
static void do_cmd_options_aux(const char *name, int page)
{
	int i;

	if (!option_toggle_menu)
	{
		option_toggle_menu = menu_new(MN_SKIN_SCROLL, &options_toggle_iter);

		option_toggle_menu->prompt = "Set option (y/n/t), '?' for information";
		option_toggle_menu->cmd_keys = "?YyNnTt";
		option_toggle_menu->selections = "abcdefghijklmopqrsuvwxz";
		option_toggle_menu->flags = MN_DBL_TAP;
	}

	option_toggle_menu->title = name;

	/* XXX assert(page < OPT_PAGE_MAX); */

	/* Find the number of valid entries */
	for (i = 0; i < OPT_PAGE_PER; i++)
	{
		if (option_page[page][i] == OPT_NONE)
			break;
	}

	/* Set the data to the player's options */
	menu_setpriv(option_toggle_menu, OPT_MAX, &op_ptr->opt);
	menu_set_filter(option_toggle_menu, option_page[page], i);
	menu_layout(option_toggle_menu, &SCREEN_REGION);

	/* Run the menu */
	screen_save();
	clear_from(0);

	menu_select(option_toggle_menu, 0);

	screen_load();
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(const char *name, int row)
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
	screen_save();
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
		if (ke.type == EVT_MOUSE)
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
		}

		/* Toggle */
		else if ((ke.key == '5') || (ke.key == 't') ||
				(ke.key == '\n') || (ke.key == '\r') ||
				(ke.type == EVT_MOUSE))
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

	screen_load();
}



/*** Interact with macros and keymaps ***/

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
	ui_event_data e;

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
	e = inkey_ex();

	/* Read the pattern */
	while (e.key != 0 && e.type != EVT_MOUSE)
	{
		/* Save the key */
		buf[n++] = e.key;
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
		e = inkey_ex();
	}

	/* Convert the trigger */
	ascii_to_text(tmp, sizeof(tmp), buf);
}


/*
 * Ask for, and display, a keymap trigger.
 *
 * Returns the trigger input.
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static char keymap_get_trigger(void)
{
	char tmp[80];
	char buf[2];

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

	/* Return trigger */
	return buf[0];
}


/*
 * Macro menu action functions
 */

static void macro_pref_load(const char *title, int row)
{
	do_cmd_pref_file_hack(16);
}

static void macro_pref_append(const char *title, int row)
{
	(void)dump_pref_file(macro_dump, "Dump macros", 15);
}

static void macro_query(const char *title, int row)
{
	int k;
	char buf[1024];
	
	prt("Command: Query a macro", 16, 0);
	prt("Trigger: ", 18, 0);
	
	/* Get a macro trigger */
	do_cmd_macro_aux(buf);
	
	/* Get the action */
	k = macro_find_exact(buf);
	
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
		ascii_to_text(buf, sizeof(buf), macro_buffer);
	
		/* Display the current action */
		prt(buf, 22, 0);
	
		/* Prompt */
		prt("", 0, 0);
		msg_print("Found a macro.");
	}
}

static void macro_create(const char *title, int row)
{
	char pat[1024];
	char tmp[1024];

	prt("Command: Create a macro", 16, 0);
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
}

static void macro_remove(const char *title, int row)
{
	char pat[1024];

	prt("Command: Remove a macro", 16, 0);
	prt("Trigger: ", 18, 0);
	
	/* Get a macro trigger */
	do_cmd_macro_aux(pat);
	
	/* Link the macro */
	macro_add(pat, pat);
	
	/* Prompt */
	prt("", 0, 0);
	msg_print("Removed a macro.");
}

static void keymap_pref_append(const char *title, int row)
{
	(void)dump_pref_file(keymap_dump, "Dump keymaps", 13);
}

static void keymap_query(const char *title, int row)
{
	char tmp[1024];
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	char c;
	const char *act;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);
	
	/* Get a keymap trigger & mapping */
	c = keymap_get_trigger();
	act = keymap_act[mode][(byte) c];
	
	/* Nothing found */
	if (!act)
	{
		/* Prompt */
		prt("No keymap with that trigger.  Press any key to continue.", 16, 0);
		inkey();
	}
	
	/* Found one */
	else
	{
		/* Obtain the action */
		my_strcpy(macro_buffer, act, sizeof(macro_buffer));
	
		/* Analyze the current action */
		ascii_to_text(tmp, sizeof(tmp), macro_buffer);
	
		/* Display the current action */
		prt("Found: ", 15, 0);
		Term_addstr(-1, TERM_WHITE, tmp);

		prt("Press any key to continue.", 17, 0);
		inkey();
	}
}

static void keymap_create(const char *title, int row)
{
	char c;
	char tmp[1024];
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);

	c = keymap_get_trigger();

	prt("Action: ", 15, 0);

	/* Get an encoded action, with a default response */
	ascii_to_text(tmp, sizeof(tmp), macro_buffer);
	if (askfor_aux(tmp, sizeof tmp, NULL))
	{
		/* Convert to ascii */
		text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);
	
		/* Make new keymap */
		string_free(keymap_act[mode][(byte) c]);
		keymap_act[mode][(byte) c] = string_make(macro_buffer);

		/* Prompt */
		prt("Keymap added.  Press any key to continue.", 17, 0);
		inkey();
	}
}

static void keymap_remove(const char *title, int row)
{
	char c;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);

	c = keymap_get_trigger();

	if (keymap_act[mode][(byte) c])
	{
		/* Free old keymap */
		string_free(keymap_act[mode][(byte) c]);
		keymap_act[mode][(byte) c] = NULL;

		prt("Removed.", 16, 0);
	}
	else
	{
		prt("No keymap to remove!", 16, 0);
	}

	/* Prompt */
	prt("Press any key to continue.", 17, 0);
	inkey();
}

static void macro_enter(const char *title, int row)
{
	char tmp[1024];

	prt(title, 16, 0);
	prt("Action: ", 17, 0);

	/* Get an action, with a default response */
	ascii_to_text(tmp, sizeof(tmp), macro_buffer);
	if (askfor_aux(tmp, sizeof tmp, NULL))
	{
		/* Save to global macro buffer */
		text_to_ascii(macro_buffer, sizeof(macro_buffer), tmp);
	}
}

static void macro_browse_hook(int oid, void *db, const region *loc)
{
	char tmp[1024];

	message_flush();

	clear_from(13);

	/* Show current action */
	prt("Current action (if any) shown below:", 13, 0);
	ascii_to_text(tmp, sizeof(tmp), macro_buffer);
	prt(tmp, 14, 0);
}

static menu_type *macro_menu;
static menu_action macro_actions[] =
{
	{ 0, "Load a user pref file",    macro_pref_load },
	{ 0, "Append macros to a file",  macro_pref_append },
	{ 0, "Query a macro",            macro_query },
	{ 0, "Create a macro",           macro_create },
	{ 0, "Remove a macro",           macro_remove },
	{ 0, "Append keymaps to a file", keymap_pref_append },
	{ 0, "Query a keymap",           keymap_query },
	{ 0, "Create a keymap",          keymap_create },
	{ 0, "Remove a keymap",          keymap_remove },
	{ 0, "Enter a new action",       macro_enter },
};

static void do_cmd_macros(const char *title, int row)
{
	region loc = {0, 0, 0, 12};

	screen_save();
	clear_from(0);

	if (!macro_menu)
	{
		macro_menu = menu_new_action(macro_actions,
				N_ELEMENTS(macro_actions));
	
		macro_menu->title = title;
		macro_menu->selections = lower_case;
		macro_menu->browse_hook = macro_browse_hook;
	}

	menu_layout(macro_menu, &loc);
	menu_select(macro_menu, 0);

	screen_load();
}

#endif /* ALLOW_MACROS */



/*** Interact with visuals ***/

static void visuals_pref_load(const char *title, int row)
{
	do_cmd_pref_file_hack(15);
}

#ifdef ALLOW_VISUALS

static void visuals_dump_monsters(const char *title, int row)
{
	dump_pref_file(dump_monsters, title, 15);
}

static void visuals_dump_objects(const char *title, int row)
{
	dump_pref_file(dump_objects, title, 15);
}

static void visuals_dump_features(const char *title, int row)
{
	dump_pref_file(dump_features, title, 15);
}

static void visuals_dump_flavors(const char *title, int row)
{
	dump_pref_file(dump_flavors, title, 15);
}

#endif /* ALLOW_VISUALS */

static void visuals_reset(const char *title, int row)
{
	/* Reset */
	reset_visuals(TRUE);

	/* Message */
	prt("", 0, 0);
	msg_print("Visual attr/char tables reset.");
	message_flush();
}


static menu_type *visual_menu;
static menu_action visual_menu_items [] =
{
	{ 0, "Load a user pref file",   visuals_pref_load },
#ifdef ALLOW_VISUALS
	{ 0, "Dump monster attr/chars", visuals_dump_monsters },
	{ 0, "Dump object attr/chars",  visuals_dump_objects },
	{ 0, "Dump feature attr/chars", visuals_dump_features },
	{ 0, "Dump flavor attr/chars",  visuals_dump_flavors },
#endif /* ALLOW_VISUALS */
	{ 0, "Reset visuals",           visuals_reset },
};


static void visuals_browse_hook(int oid, void *db, const region *loc)
{
	message_flush();
	clear_from(0);
}


/*
 * Interact with "visuals"
 */
static void do_cmd_visuals(const char *title, int row)
{
	screen_save();
	clear_from(0);

	if (!visual_menu)
	{
		visual_menu = menu_new_action(visual_menu_items,
				N_ELEMENTS(visual_menu_items));

		visual_menu->title = title;
		visual_menu->selections = lower_case;
		visual_menu->browse_hook = visuals_browse_hook;
	}

	menu_layout(visual_menu, &SCREEN_REGION);
	menu_select(visual_menu, 0);

	screen_load();
}


/*** Interact with colours ***/

#ifdef ALLOW_COLORS

static void colors_pref_load(const char *title, int row)
{
	/* Ask for and load a user pref file */
	do_cmd_pref_file_hack(8);
	
	/* XXX should probably be a cleaner way to tell UI about
	 * colour changes - how about doing this in the pref file
	 * loading code too? */
	Term_xtra(TERM_XTRA_REACT, 0);
	Term_redraw();
}

static void colors_pref_dump(const char *title, int row)
{
	dump_pref_file(dump_colors, title, 15);
}

static void colors_modify(const char *title, int row)
{
	int i;
	int cx;

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

static void colors_browse_hook(int oid, void *db, const region *loc)
{
	message_flush();
	clear_from(0);
}


static menu_type *color_menu;
static menu_action color_events [] =
{
	{ 0, "Load a user pref file", colors_pref_load },
	{ 0, "Dump colors",           colors_pref_dump },
	{ 0, "Modify colors",         colors_modify }
};

/*
 * Interact with "colors"
 */
void do_cmd_colors(const char *title, int row)
{
	screen_save();
	clear_from(0);

	if (!color_menu)
	{
		color_menu = menu_new_action(color_events,
			N_ELEMENTS(color_events));

		color_menu->title = title;
		color_menu->selections = lower_case;
		color_menu->browse_hook = colors_browse_hook;
	}

	menu_layout(color_menu, &SCREEN_REGION);
	menu_select(color_menu, 0);

	screen_load();
}

#endif


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
static void do_cmd_delay(const char *name, int row)
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
static void do_cmd_hp_warn(const char *name, int row)
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
static void do_cmd_lazymove_delay(const char *name, int row)
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
static void do_dump_options(const char *title, int row)
{
	dump_pref_file(option_dump, "Dump options", 20);
}

/*
 * Load a pref file.
 */
static void options_load_pref_file(const char *n, int row)
{
	do_cmd_pref_file_hack(20);
}



/*** Main menu definitions and display ***/

static menu_type *option_menu;
static menu_action option_actions [] = 
{
	{ 'a', "Interface options", do_cmd_options_aux },
	{ 'b', "Display options", do_cmd_options_aux },
	{ 'e', "Warning and disturbance options", do_cmd_options_aux },
	{ 'f', "Birth (difficulty) options", do_cmd_options_aux },
	{ 'g', "Cheat options", do_cmd_options_aux },
	{0, 0, 0}, /* Load and append */
	{ 'w', "Subwindow display settings", do_cmd_options_win },
	{ 's', "Item squelch settings", do_cmd_options_item },
	{ 'd', "Set base delay factor", do_cmd_delay },
	{ 'h', "Set hitpoint warning", do_cmd_hp_warn },
	{ 'i', "Set movement delay", do_cmd_lazymove_delay },
	{ 'l', "Load a user pref file", options_load_pref_file },
	{ 'o', "Save options", do_dump_options }, 
	{0, 0, 0}, /* Interact with */	

#ifdef ALLOW_MACROS
	{ 'm', "Interact with macros (advanced)", do_cmd_macros },
#endif /* ALLOW_MACROS */

	{ 'v', "Interact with visuals (advanced)", do_cmd_visuals },

#ifdef ALLOW_COLORS
	{ 'c', "Interact with colours (advanced)", do_cmd_colors },
#endif /* ALLOW_COLORS */
};


/*
 * Display the options main menu.
 */
void do_cmd_options(void)
{
	if (!option_menu)
	{
		/* Main option menu */
		option_menu = menu_new_action(option_actions,
				N_ELEMENTS(option_actions));

		option_menu->title = "Options Menu";
		option_menu->flags = MN_CASELESS_TAGS;
	}

	screen_save();
	clear_from(0);

	menu_layout(option_menu, &SCREEN_REGION);
	menu_select(option_menu, 0);

	screen_load();
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

