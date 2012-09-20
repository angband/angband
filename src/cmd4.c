/* File: cmd4.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"



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
 */
void do_cmd_redraw(void)
{
	int j;

	term *old = Term;


	/* Low level flush */
	Term_flush();

	/* Reset "inkey()" */
	flush();


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
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Window stuff */
	p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_MONSTER | PW_OBJECT);

	/* Clear screen */
	Term_clear();

	/* Hack -- update */
	handle_stuff();


	/* Redraw every window */
	for (j = 0; j < 8; j++)
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
	char ch;

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
		ch = inkey();

		/* Exit */
		if (ch == ESCAPE) break;

		/* Change name */
		if (ch == 'c')
		{
			get_name();
		}

		/* File dump */
		else if (ch == 'f')
		{
			char ftmp[80];

			sprintf(ftmp, "%s.txt", op_ptr->base_name);

			if (get_string("File name: ", ftmp, 80))
			{
				if (ftmp[0] && (ftmp[0] != ' '))
				{
					if (file_character(ftmp, FALSE))
					{
						msg_print("Character dump failed!");
					}
					else
					{
						msg_print("Character dump successful.");
					}
				}
			}
		}

		/* Toggle mode */
		else if (ch == 'h')
		{
			mode = !mode;
		}

		/* Oops */
		else
		{
			bell("Illegal command for change name!");
		}

		/* Flush messages */
		msg_print(NULL);
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
 * Attempt to only hilite the matching portions of the string.
 */
void do_cmd_messages(void)
{
	char ch;

	int i, j, n;
	uint q;

	char shower[80];
	char finder[80];


	/* Wipe finder */
	strcpy(finder, "");

	/* Wipe shower */
	strcpy(shower, "");


	/* Total messages */
	n = message_num();

	/* Start on first message */
	i = 0;

	/* Start at leftmost edge */
	q = 0;


	/* Save screen */
	screen_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Dump up to 20 lines of messages */
		for (j = 0; (j < 20) && (i + j < n); j++)
		{
			cptr msg = message_str((s16b)(i+j));
			byte attr = message_color((s16b)(i+j));

			/* Apply horizontal scroll */
			msg = (strlen(msg) >= q) ? (msg + q) : "";

			/* Dump the messages, bottom to top */
			Term_putstr(0, 21-j, -1, attr, msg);

			/* Hilite "shower" */
			if (shower[0])
			{
				cptr str = msg;

				/* Display matches */
				while ((str = strstr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-msg, 21-j, len, TERM_YELLOW, shower);

					/* Advance */
					str += len;
				}
			}
		}

		/* Display header XXX XXX XXX */
		prt(format("Message Recall (%d-%d of %d), Offset %d",
		           i, i+j-1, n, q), 0, 0);

		/* Display prompt (not very informative) */
		prt("[Press 'p' for older, 'n' for newer, ..., or ESCAPE]", 23, 0);

		/* Get a command */
		ch = inkey();

		/* Exit on Escape */
		if (ch == ESCAPE) break;

		/* Hack -- Save the old index */
		j = i;

		/* Horizontal scroll */
		if (ch == '4')
		{
			/* Scroll left */
			q = (q >= 40) ? (q - 40) : 0;

			/* Success */
			continue;
		}

		/* Horizontal scroll */
		if (ch == '6')
		{
			/* Scroll right */
			q = q + 40;

			/* Success */
			continue;
		}

		/* Hack -- handle show */
		if (ch == '=')
		{
			/* Prompt */
			prt("Show: ", 23, 0);

			/* Get a "shower" string, or continue */
			if (!askfor_aux(shower, 80)) continue;

			/* Okay */
			continue;
		}

		/* Hack -- handle find */
		if (ch == '/')
		{
			s16b z;

			/* Prompt */
			prt("Find: ", 23, 0);

			/* Get a "finder" string, or continue */
			if (!askfor_aux(finder, 80)) continue;

			/* Show it */
			strcpy(shower, finder);

			/* Scan messages */
			for (z = i + 1; z < n; z++)
			{
				cptr msg = message_str(z);

				/* Search for it */
				if (strstr(msg, finder))
				{
					/* New location */
					i = z;

					/* Done */
					break;
				}
			}
		}

		/* Recall 20 older messages */
		if ((ch == 'p') || (ch == KTRL('P')) || (ch == ' '))
		{
			/* Go older if legal */
			if (i + 20 < n) i += 20;
		}

		/* Recall 10 older messages */
		if (ch == '+')
		{
			/* Go older if legal */
			if (i + 10 < n) i += 10;
		}

		/* Recall 1 older message */
		if ((ch == '8') || (ch == '\n') || (ch == '\r'))
		{
			/* Go newer if legal */
			if (i + 1 < n) i += 1;
		}

		/* Recall 20 newer messages */
		if ((ch == 'n') || (ch == KTRL('N')))
		{
			/* Go newer (if able) */
			i = (i >= 20) ? (i - 20) : 0;
		}

		/* Recall 10 newer messages */
		if (ch == '-')
		{
			/* Go newer (if able) */
			i = (i >= 10) ? (i - 10) : 0;
		}

		/* Recall 1 newer messages */
		if (ch == '2')
		{
			/* Go newer (if able) */
			i = (i >= 1) ? (i - 1) : 0;
		}

		/* Hack -- Error of some kind */
		if (i == j) bell(NULL);
	}

	/* Load screen */
	screen_load();
}



/*
 * Ask for a "user pref line" and process it
 */
void do_cmd_pref(void)
{
	char tmp[80];

	/* Default */
	strcpy(tmp, "");

	/* Ask for a "user pref command" */
	if (!get_string("Pref: ", tmp, 80)) return;

	/* Process that pref command */
	(void)process_pref_file_aux(tmp);
}


/*
 * Ask for a "user pref file" and process it.
 *
 * This function should only be used by standard interaction commands,
 * in which a standard "Command:" prompt is present on the given row.
 *
 * Allow absolute file names?  XXX XXX XXX
 */
static void do_cmd_pref_file_hack(int row)
{
	char ftmp[80];

	/* Prompt */
	prt("Command: Load a user pref file", row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	sprintf(ftmp, "%s.prf", op_ptr->base_name);

	/* Ask for a file (or cancel) */
	if (!askfor_aux(ftmp, 80)) return;

	/* Process the given filename */
	if (process_pref_file(ftmp))
	{
		/* Mention failure */
		msg_format("Failed to load '%s'!", ftmp);
	}
	else
	{
		/* Mention success */
		msg_format("Loaded '%s'.", ftmp);
	}
}



/*
 * Interact with some options
 */
static void do_cmd_options_aux(int page, cptr info)
{
	char ch;

	int i, k = 0, n = 0;

	int opt[OPT_PAGE_PER];

	char buf[80];


	/* Scan the options */
	for (i = 0; i < OPT_PAGE_PER; i++)
	{
		/* Collect options on this "page" */
		if (option_page[page][i] != 255)
		{
			opt[n++] = option_page[page][i];
		}
	}


	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE)
	{
		/* Prompt XXX XXX XXX */
		sprintf(buf, "%s (RET to advance, y/n to set, ESC to accept) ", info);
		prt(buf, 0, 0);

		/* Display the options */
		for (i = 0; i < n; i++)
		{
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Display the option text */
			sprintf(buf, "%-48s: %s  (%s)",
			        option_desc[opt[i]],
			        op_ptr->opt[opt[i]] ? "yes" : "no ",
			        option_text[opt[i]]);
			c_prt(a, buf, i + 2, 0);
		}

		/* Hilite current option */
		move_cursor(k + 2, 50);

		/* Get a key */
		ch = inkey();

		/* Analyze */
		switch (ch)
		{
			case ESCAPE:
			{
				/* Hack -- Notice use of any "cheat" options */
				for (i = OPT_CHEAT; i < OPT_ADULT; i++)
				{
					if (op_ptr->opt[i])
					{
						/* Set score option */
						op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = TRUE;
					}
				}

				return;
			}

			case '-':
			case '8':
			{
				k = (n + k - 1) % n;
				break;
			}

			case ' ':
			case '\n':
			case '\r':
			case '2':
			{
				k = (k + 1) % n;
				break;
			}

			case 't':
			case '5':
			{
				op_ptr->opt[opt[k]] = !op_ptr->opt[opt[k]];
				break;
			}

			case 'y':
			case '6':
			{
				op_ptr->opt[opt[k]] = TRUE;
				k = (k + 1) % n;
				break;
			}

			case 'n':
			case '4':
			{
				op_ptr->opt[opt[k]] = FALSE;
				k = (k + 1) % n;
				break;
			}

			default:
			{
				bell("Illegal command for normal options!");
				break;
			}
		}
	}
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
	int i, j, d;

	int y = 0;
	int x = 0;

	char ch;

	u32b old_flag[8];


	/* Memorize old flags */
	for (j = 0; j < 8; j++)
	{
		old_flag[j] = op_ptr->window_flag[j];
	}


	/* Clear screen */
	Term_clear();

	/* Interact */
	while (1)
	{
		/* Prompt */
		prt("Window flags (<dir> to move, 't' to toggle, or ESC)", 0, 0);

		/* Display the windows */
		for (j = 0; j < 8; j++)
		{
			byte a = TERM_WHITE;

			cptr s = angband_term_name[j];

			/* Use color */
			if (j == x) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < 16; i++)
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
			for (j = 0; j < 8; j++)
			{
				byte a = TERM_WHITE;

				char c = '.';

				/* Use color */
				if ((i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (op_ptr->window_flag[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ch = inkey();

		/* Allow escape */
		if ((ch == ESCAPE) || (ch == 'q')) break;

		/* Toggle */
		if ((ch == '5') || (ch == 't'))
		{
			/* Hack -- ignore the main window */
			if (x == 0)
			{
				bell("Cannot set main window flags!");
			}

			/* Toggle flag (off) */
			else if (op_ptr->window_flag[x] & (1L << y))
			{
				op_ptr->window_flag[x] &= ~(1L << y);
			}

			/* Toggle flag (on) */
			else
			{
				op_ptr->window_flag[x] |= (1L << y);
			}

			/* Continue */
			continue;
		}

		/* Extract direction */
		d = target_dir(ch);

		/* Move */
		if (d != 0)
		{
			x = (x + ddx[d] + 8) % 8;
			y = (y + ddy[d] + 16) % 16;
		}

		/* Oops */
		else
		{
			bell("Illegal command for window options!");
		}
	}

	/* Notice changes */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* Dead window */
		if (!angband_term[j]) continue;

		/* Ignore non-changes */
		if (op_ptr->window_flag[j] == old_flag[j]) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Erase */
		Term_clear();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Write all current options to the given preference file in the
 * lib/user directory. Modified from KAmband 1.8.
 */
static errr option_dump(cptr fname)
{
	int i, j;

	FILE *fff;

	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return (-1);


	/* Skip some lines */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic option dump\n\n");

	/* Dump options (skip cheat, adult, score) */
	for (i = 0; i < OPT_CHEAT; i++)
	{
		/* Require a real option */
		if (!option_text[i]) continue;

		/* Comment */
		fprintf(fff, "# Option '%s'\n", option_desc[i]);

		/* Dump the option */
		if (op_ptr->opt[i])
		{
			fprintf(fff, "Y:%s\n", option_text[i]);
		}
		else
		{
			fprintf(fff, "X:%s\n", option_text[i]);
		}

		/* Skip a line */
		fprintf(fff, "\n");
	}

	/* Dump window flags */
	for (i = 1; i < 8; i++)
	{
		/* Require a real window */
		if (!angband_term[i]) continue;

		/* Check each flag */
		for (j = 0; j < 32; j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Comment */
			fprintf(fff, "# Window '%s', Flag '%s'\n",
			        angband_term_name[i], window_flag_desc[j]);

			/* Dump the flag */
			if (op_ptr->window_flag[i] & (1L << j))
			{
				fprintf(fff, "W:%d:%d:1\n", i, j);
			}
			else
			{
				fprintf(fff, "W:%d:%d:0\n", i, j);
			}

			/* Skip a line */
			fprintf(fff, "\n");
		}
	}

	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}


/*
 * Set or unset various options.
 *
 * After using this command, a complete redraw should be performed,
 * in case any visual options have been changed.
 */
void do_cmd_options(void)
{
	char ch;


	/* Save screen */
	screen_save();


	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Why are we here */
		prt("Angband options", 2, 0);

		/* Give some choices */
		prt("(1) User Interface Options", 4, 5);
		prt("(2) Disturbance Options", 5, 5);
		prt("(3) Game-Play Options", 6, 5);
		prt("(4) Efficiency Options", 7, 5);
		prt("(5) Birth Options", 8, 5);
		prt("(6) Cheat Options", 9, 5);

		/* Window flags */
		prt("(W) Window flags", 11, 5);

		/* Load and Append */
		prt("(L) Load a user pref file", 13, 5);
		prt("(A) Append options to a file", 14, 5);

		/* Special choices */
		prt("(D) Base Delay Factor", 16, 5);
		prt("(H) Hitpoint Warning", 17, 5);

		/* Prompt */
		prt("Command: ", 19, 0);

		/* Get command */
		ch = inkey();

		/* Exit */
		if (ch == ESCAPE) break;

		/* General Options */
		else if (ch == '1')
		{
			do_cmd_options_aux(0, "User Interface Options");
		}

		/* Disturbance Options */
		else if (ch == '2')
		{
			do_cmd_options_aux(1, "Disturbance Options");
		}

		/* Inventory Options */
		else if (ch == '3')
		{
			do_cmd_options_aux(2, "Game-Play Options");
		}

		/* Efficiency Options */
		else if (ch == '4')
		{
			do_cmd_options_aux(3, "Efficiency Options");
		}

		/* Birth Options */
		else if (ch == '5')
		{
			do_cmd_options_aux(4, "Birth Options");
		}

		/* Cheating Options */
		else if (ch == '6')
		{
			do_cmd_options_aux(5, "Cheat Options");
		}

		/* Window flags */
		else if ((ch == 'W') || (ch == 'w'))
		{
			do_cmd_options_win();
		}

		/* Load a user pref file */
		else if ((ch == 'L') || (ch == 'l'))
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(19);
		}

		/* Append options to a file */
		else if ((ch == 'A') || (ch == 'a'))
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Append options to a file", 19, 0);

			/* Prompt */
			prt("File: ", 21, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Ask for a file */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Drop priv's */
			safe_setuid_drop();

			/* Dump the options */
			if (option_dump(ftmp))
			{
				/* Failure */
				msg_print("Failed!");
			}
			else
			{
				/* Success */
				msg_print("Done.");
			}

			/* Grab priv's */
			safe_setuid_grab();
		}

		/* Hack -- Base Delay Factor */
		else if ((ch == 'D') || (ch == 'd'))
		{
			/* Prompt */
			prt("Command: Base Delay Factor", 19, 0);

			/* Get a new value */
			while (1)
			{
				char cx;
				int msec = op_ptr->delay_factor * op_ptr->delay_factor;
				prt(format("Current base delay factor: %d (%d msec)",
				           op_ptr->delay_factor, msec), 22, 0);
				prt("New base delay factor (0-9 or ESC to accept): ", 21, 0);

				cx = inkey();
				if (cx == ESCAPE) break;
				if (isdigit(cx)) op_ptr->delay_factor = D2I(cx);
				else bell("Illegal delay factor!");
			}
		}

		/* Hack -- hitpoint warning factor */
		else if ((ch == 'H') || (ch == 'h'))
		{
			/* Prompt */
			prt("Command: Hitpoint Warning", 19, 0);

			/* Get a new value */
			while (1)
			{
				char cx;
				prt(format("Current hitpoint warning: %2d%%",
				           op_ptr->hitpoint_warn * 10), 22, 0);
				prt("New hitpoint warning (0-9 or ESC to accept): ", 21, 0);

				cx = inkey();
				if (cx == ESCAPE) break;
				if (isdigit(cx)) op_ptr->hitpoint_warn = D2I(cx);
				else bell("Illegal hitpoint warning!");
			}
		}

		/* Unknown option */
		else
		{
			/* Oops */
			bell("Illegal command for options!");
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Load screen */
	screen_load();
}



#ifdef ALLOW_MACROS

/*
 * Hack -- append all current macros to the given file
 */
static errr macro_dump(cptr fname)
{
	int i;

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return (-1);


	/* Skip some lines */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic macro dump\n\n");

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		fprintf(fff, "# Macro '%d'\n\n", i);

		/* Extract the macro action */
		ascii_to_text(buf, macro__act[i]);

		/* Dump the macro action */
		fprintf(fff, "A:%s\n", buf);

		/* Extract the macro pattern */
		ascii_to_text(buf, macro__pat[i]);

		/* Dump the macro pattern */
		fprintf(fff, "P:%s\n", buf);

		/* End the macro */
		fprintf(fff, "\n\n");
	}

	/* Start dumping */
	fprintf(fff, "\n\n\n\n");


	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}


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

	char tmp[1024];


	/* Flush */
	flush();

	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	ch = inkey();

	/* Read the pattern */
	while (ch != '\0')
	{
		/* Save the key */
		buf[n++] = ch;

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		ch = inkey();
	}

	/* Terminate */
	buf[n] = '\0';

	/* Flush */
	flush();


	/* Convert the trigger */
	ascii_to_text(tmp, buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);
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
	ascii_to_text(tmp, buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);


	/* Flush */
	flush();
}


/*
 * Hack -- Append all keymaps to the given file.
 *
 * Hack -- We only append the keymaps for the "active" mode.
 */
static errr keymap_dump(cptr fname)
{
	int i;

	FILE *fff;

	char buf[1024];

	int mode;


	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return (-1);


	/* Skip some lines */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic keymap dump\n\n");

	/* Dump them */
	for (i = 0; i < 256; i++)
	{
		char key[2] = "?";

		cptr act;

		/* Loop up the keymap */
		act = keymap_act[mode][i];

		/* Skip empty keymaps */
		if (!act) continue;

		/* Encode the action */
		ascii_to_text(buf, act);

		/* Dump the keymap action */
		fprintf(fff, "A:%s\n", buf);

		/* Convert the key into a string */
		key[0] = i;

		/* Encode the key */
		ascii_to_text(buf, key);

		/* Dump the keymap pattern */
		fprintf(fff, "C:%d:%s\n", mode, buf);

		/* Skip a line */
		fprintf(fff, "\n");
	}

	/* Skip some lines */
	fprintf(fff, "\n\n\n");


	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}


#endif


/*
 * Interact with "macros"
 *
 * Could use some helpful instructions on this page.  XXX XXX XXX
 */
void do_cmd_macros(void)
{
	char ch;

	char tmp[1024];

	char pat[1024];

	int mode;


	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Save screen */
	screen_save();


	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Describe */
		prt("Interact with Macros", 2, 0);


		/* Describe that action */
		prt("Current action (if any) shown below:", 20, 0);

		/* Analyze the current action */
		ascii_to_text(tmp, macro_buffer);

		/* Display the current action */
		prt(tmp, 22, 0);


		/* Selections */
		prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_MACROS
		prt("(2) Append macros to a file", 5, 5);
		prt("(3) Query a macro", 6, 5);
		prt("(4) Create a macro", 7, 5);
		prt("(5) Remove a macro", 8, 5);
		prt("(6) Append keymaps to a file", 9, 5);
		prt("(7) Query a keymap", 10, 5);
		prt("(8) Create a keymap", 11, 5);
		prt("(9) Remove a keymap", 12, 5);
		prt("(0) Enter a new action", 13, 5);
#endif /* ALLOW_MACROS */

		/* Prompt */
		prt("Command: ", 16, 0);

		/* Get a command */
		ch = inkey();

		/* Leave */
		if (ch == ESCAPE) break;

		/* Load a user pref file */
		if (ch == '1')
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(16);
		}

#ifdef ALLOW_MACROS

		/* Save macros */
		else if (ch == '2')
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Append macros to a file", 16, 0);

			/* Prompt */
			prt("File: ", 18, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Ask for a file */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Drop priv's */
			safe_setuid_drop();

			/* Dump the macros */
			(void)macro_dump(ftmp);

			/* Grab priv's */
			safe_setuid_grab();

			/* Prompt */
			msg_print("Appended macros.");
		}

		/* Query a macro */
		else if (ch == '3')
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
				msg_print("Found no macro.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				strcpy(macro_buffer, macro__act[k]);

				/* Analyze the current action */
				ascii_to_text(tmp, macro_buffer);

				/* Display the current action */
				prt(tmp, 22, 0);

				/* Prompt */
				msg_print("Found a macro.");
			}
		}

		/* Create a macro */
		else if (ch == '4')
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
			ascii_to_text(tmp, macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, 80))
			{
				/* Convert to ascii */
				text_to_ascii(macro_buffer, tmp);

				/* Link the macro */
				macro_add(pat, macro_buffer);

				/* Prompt */
				msg_print("Added a macro.");
			}
		}

		/* Remove a macro */
		else if (ch == '5')
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
			msg_print("Removed a macro.");
		}

		/* Save keymaps */
		else if (ch == '6')
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Append keymaps to a file", 16, 0);

			/* Prompt */
			prt("File: ", 18, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Ask for a file */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Drop priv's */
			safe_setuid_drop();

			/* Dump the macros */
			(void)keymap_dump(ftmp);

			/* Grab priv's */
			safe_setuid_grab();

			/* Prompt */
			msg_print("Appended keymaps.");
		}

		/* Query a keymap */
		else if (ch == '7')
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
				msg_print("Found no keymap.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				strcpy(macro_buffer, act);

				/* Analyze the current action */
				ascii_to_text(tmp, macro_buffer);

				/* Display the current action */
				prt(tmp, 22, 0);

				/* Prompt */
				msg_print("Found a keymap.");
			}
		}

		/* Create a keymap */
		else if (ch == '8')
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
			ascii_to_text(tmp, macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, 80))
			{
				/* Convert to ascii */
				text_to_ascii(macro_buffer, tmp);

				/* Free old keymap */
				string_free(keymap_act[mode][(byte)(pat[0])]);

				/* Make new keymap */
				keymap_act[mode][(byte)(pat[0])] = string_make(macro_buffer);

				/* Prompt */
				msg_print("Added a keymap.");
			}
		}

		/* Remove a keymap */
		else if (ch == '9')
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
			msg_print("Removed a keymap.");
		}

		/* Enter a new action */
		else if (ch == '0')
		{
			/* Prompt */
			prt("Command: Enter a new action", 16, 0);

			/* Go to the correct location */
			Term_gotoxy(0, 22);

			/* Analyze the current action */
			ascii_to_text(tmp, macro_buffer);

			/* Get an encoded action */
			if (askfor_aux(tmp, 80))
			{
				/* Extract an action */
				text_to_ascii(macro_buffer, tmp);
			}
		}

#endif /* ALLOW_MACROS */

		/* Oops */
		else
		{
			/* Oops */
			bell("Illegal command for macros!");
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Load screen */
	screen_load();
}



/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
	int ch;
	int cx;

	int i;

	FILE *fff;

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Save screen */
	screen_save();


	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Interact with Visuals", 2, 0);

		/* Give some choices */
		prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_VISUALS
		prt("(2) Dump monster attr/chars", 5, 5);
		prt("(3) Dump object attr/chars", 6, 5);
		prt("(4) Dump feature attr/chars", 7, 5);
		prt("(5) (unused)", 8, 5);
		prt("(6) Change monster attr/chars", 9, 5);
		prt("(7) Change object attr/chars", 10, 5);
		prt("(8) Change feature attr/chars", 11, 5);
		prt("(9) (unused)", 12, 5);
#endif
		prt("(0) Reset visuals", 13, 5);

		/* Prompt */
		prt("Command: ", 15, 0);

		/* Prompt */
		ch = inkey();

		/* Done */
		if (ch == ESCAPE) break;

		/* Load a user pref file */
		if (ch == '1')
		{
			/* Ask for and load a user pref file */
			do_cmd_pref_file_hack(15);
		}

#ifdef ALLOW_VISUALS

		/* Dump monster attr/chars */
		else if (ch == '2')
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Dump monster attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Get a filename */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, ftmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;


			/* Skip some lines */
			fprintf(fff, "\n\n");

			/* Start dumping */
			fprintf(fff, "# Monster attr/char definitions\n\n");

			/* Dump monsters */
			for (i = 0; i < z_info->r_max; i++)
			{
				monster_race *r_ptr = &r_info[i];

				/* Skip non-entries */
				if (!r_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (r_name + r_ptr->name));

				/* Dump the monster attr/char info */
				fprintf(fff, "R:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(r_ptr->x_attr), (byte)(r_ptr->x_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped monster attr/chars.");
		}

		/* Dump object attr/chars */
		else if (ch == '3')
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Dump object attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Get a filename */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, ftmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;


			/* Skip some lines */
			fprintf(fff, "\n\n");

			/* Start dumping */
			fprintf(fff, "# Object attr/char definitions\n\n");

			/* Dump objects */
			for (i = 0; i < z_info->k_max; i++)
			{
				object_kind *k_ptr = &k_info[i];

				/* Skip non-entries */
				if (!k_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (k_name + k_ptr->name));

				/* Dump the object attr/char info */
				fprintf(fff, "K:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(k_ptr->x_attr), (byte)(k_ptr->x_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped object attr/chars.");
		}

		/* Dump feature attr/chars */
		else if (ch == '4')
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Dump feature attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Get a filename */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, ftmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;


			/* Skip some lines */
			fprintf(fff, "\n\n");

			/* Start dumping */
			fprintf(fff, "# Feature attr/char definitions\n\n");

			/* Dump features */
			for (i = 0; i < z_info->f_max; i++)
			{
				feature_type *f_ptr = &f_info[i];

				/* Skip non-entries */
				if (!f_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (f_name + f_ptr->name));

				/* Dump the feature attr/char info */
				fprintf(fff, "F:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(f_ptr->x_attr), (byte)(f_ptr->x_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped feature attr/chars.");
		}

		/* Modify monster attr/chars */
		else if (ch == '6')
		{
			static int r = 0;

			/* Prompt */
			prt("Command: Change monster attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				monster_race *r_ptr = &r_info[r];

				byte da = (byte)(r_ptr->d_attr);
				char dc = (byte)(r_ptr->d_char);
				byte ca = (byte)(r_ptr->x_attr);
				char cc = (byte)(r_ptr->x_char);

				/* Label the object */
				Term_putstr(5, 17, -1, TERM_WHITE,
				            format("Monster = %d, Name = %-40.40s",
				                   r, (r_name + r_ptr->name)));

				/* Label the Default values */
				Term_putstr(10, 19, -1, TERM_WHITE,
				            format("Default attr/char = %3u / %3u", da, dc));
				Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 19, da, dc);

				/* Label the Current values */
				Term_putstr(10, 20, -1, TERM_WHITE,
				            format("Current attr/char = %3u / %3u", ca, cc));
				Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 20, ca, cc);

				/* Prompt */
				Term_putstr(0, 22, -1, TERM_WHITE,
				            "Command (n/N/a/A/c/C): ");

				/* Get a command */
				cx = inkey();

				/* All done */
				if (cx == ESCAPE) break;

				/* Analyze */
				if (cx == 'n') r = (r + z_info->r_max + 1) % z_info->r_max;
				if (cx == 'N') r = (r + z_info->r_max - 1) % z_info->r_max;
				if (cx == 'a') r_ptr->x_attr = (byte)(ca + 1);
				if (cx == 'A') r_ptr->x_attr = (byte)(ca - 1);
				if (cx == 'c') r_ptr->x_char = (byte)(cc + 1);
				if (cx == 'C') r_ptr->x_char = (byte)(cc - 1);
			}
		}

		/* Modify object attr/chars */
		else if (ch == '7')
		{
			static int k = 0;

			/* Prompt */
			prt("Command: Change object attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				object_kind *k_ptr = &k_info[k];

				byte da = (byte)(k_ptr->d_attr);
				char dc = (byte)(k_ptr->d_char);
				byte ca = (byte)(k_ptr->x_attr);
				char cc = (byte)(k_ptr->x_char);

				/* Label the object */
				Term_putstr(5, 17, -1, TERM_WHITE,
				            format("Object = %d, Name = %-40.40s",
				                   k, (k_name + k_ptr->name)));

				/* Label the Default values */
				Term_putstr(10, 19, -1, TERM_WHITE,
				            format("Default attr/char = %3d / %3d", da, dc));
				Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 19, da, dc);

				/* Label the Current values */
				Term_putstr(10, 20, -1, TERM_WHITE,
				            format("Current attr/char = %3d / %3d", ca, cc));
				Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 20, ca, cc);

				/* Prompt */
				Term_putstr(0, 22, -1, TERM_WHITE,
				            "Command (n/N/a/A/c/C): ");

				/* Get a command */
				cx = inkey();

				/* All done */
				if (cx == ESCAPE) break;

				/* Analyze */
				if (cx == 'n') k = (k + z_info->k_max + 1) % z_info->k_max;
				if (cx == 'N') k = (k + z_info->k_max - 1) % z_info->k_max;
				if (cx == 'a') k_info[k].x_attr = (byte)(ca + 1);
				if (cx == 'A') k_info[k].x_attr = (byte)(ca - 1);
				if (cx == 'c') k_info[k].x_char = (byte)(cc + 1);
				if (cx == 'C') k_info[k].x_char = (byte)(cc - 1);
			}
		}

		/* Modify feature attr/chars */
		else if (ch == '8')
		{
			static int f = 0;

			/* Prompt */
			prt("Command: Change feature attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				feature_type *f_ptr = &f_info[f];

				byte da = (byte)(f_ptr->d_attr);
				char dc = (byte)(f_ptr->d_char);
				byte ca = (byte)(f_ptr->x_attr);
				char cc = (byte)(f_ptr->x_char);

				/* Label the object */
				Term_putstr(5, 17, -1, TERM_WHITE,
				            format("Terrain = %d, Name = %-40.40s",
				                   f, (f_name + f_ptr->name)));

				/* Label the Default values */
				Term_putstr(10, 19, -1, TERM_WHITE,
				            format("Default attr/char = %3d / %3d", da, dc));
				Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 19, da, dc);

				/* Label the Current values */
				Term_putstr(10, 20, -1, TERM_WHITE,
				            format("Current attr/char = %3d / %3d", ca, cc));
				Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 20, ca, cc);

				/* Prompt */
				Term_putstr(0, 22, -1, TERM_WHITE,
				            "Command (n/N/a/A/c/C): ");

				/* Get a command */
				cx = inkey();

				/* All done */
				if (cx == ESCAPE) break;

				/* Analyze */
				if (cx == 'n') f = (f + z_info->f_max + 1) % z_info->f_max;
				if (cx == 'N') f = (f + z_info->f_max - 1) % z_info->f_max;
				if (cx == 'a') f_info[f].x_attr = (byte)(ca + 1);
				if (cx == 'A') f_info[f].x_attr = (byte)(ca - 1);
				if (cx == 'c') f_info[f].x_char = (byte)(cc + 1);
				if (cx == 'C') f_info[f].x_char = (byte)(cc - 1);
			}
		}

#endif

		/* Reset visuals */
		else if (ch == '0')
		{
			/* Reset */
			reset_visuals(TRUE);

			/* Message */
			msg_print("Visual attr/char tables reset.");
		}

		/* Unknown option */
		else
		{
			bell("Illegal command for visuals!");
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Load screen */
	screen_load();
}


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
	int ch;
	int cx;

	int i;

	FILE *fff;

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Save screen */
	screen_save();


	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Interact with Colors", 2, 0);

		/* Give some choices */
		prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_COLORS
		prt("(2) Dump colors", 5, 5);
		prt("(3) Modify colors", 6, 5);
#endif

		/* Prompt */
		prt("Command: ", 8, 0);

		/* Prompt */
		ch = inkey();

		/* Done */
		if (ch == ESCAPE) break;

		/* Load a user pref file */
		if (ch == '1')
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
		else if (ch == '2')
		{
			char ftmp[80];

			/* Prompt */
			prt("Command: Dump colors", 8, 0);

			/* Prompt */
			prt("File: ", 10, 0);

			/* Default filename */
			sprintf(ftmp, "%s.prf", op_ptr->base_name);

			/* Get a filename */
			if (!askfor_aux(ftmp, 80)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, ftmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;


			/* Skip some lines */
			fprintf(fff, "\n\n");

			/* Start dumping */
			fprintf(fff, "# Color redefinitions\n\n");

			/* Dump colors */
			for (i = 0; i < 256; i++)
			{
				int kv = angband_color_table[i][0];
				int rv = angband_color_table[i][1];
				int gv = angband_color_table[i][2];
				int bv = angband_color_table[i][3];

				cptr name = "unknown";

				/* Skip non-entries */
				if (!kv && !rv && !gv && !bv) continue;

				/* Extract the color name */
				if (i < 16) name = color_names[i];

				/* Dump a comment */
				fprintf(fff, "# Color '%s'\n", name);

				/* Dump the monster attr/char info */
				fprintf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
				        i, kv, rv, gv, bv);
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped color redefinitions.");
		}

		/* Edit colors */
		else if (ch == '3')
		{
			static byte a = 0;

			/* Prompt */
			prt("Command: Modify colors", 8, 0);

			/* Hack -- query until done */
			while (1)
			{
				cptr name;

				/* Clear */
				clear_from(10);

				/* Exhibit the normal colors */
				for (i = 0; i < 16; i++)
				{
					/* Exhibit this color */
					Term_putstr(i*4, 20, -1, a, "###");

					/* Exhibit all colors */
					Term_putstr(i*4, 22, -1, (byte)i, format("%3d", i));
				}

				/* Describe the color */
				name = ((a < 16) ? color_names[a] : "undefined");

				/* Describe the color */
				Term_putstr(5, 10, -1, TERM_WHITE,
				            format("Color = %d, Name = %s", a, name));

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

#endif

		/* Unknown option */
		else
		{
			bell("Illegal command for colors!");
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Load screen */
	screen_load();
}


/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
	char tmp[80];

	/* Default */
	strcpy(tmp, "");

	/* Input */
	if (!get_string("Note: ", tmp, 80)) return;

	/* Ignore empty notes */
	if (!tmp[0] || (tmp[0] == ' ')) return;

	/* Add the note to the message recall */
	msg_format("Note: %s", tmp);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
	/* Silly message */
	msg_format("You are playing Angband %d.%d.%d.  Type '?' for more info.",
	           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}



/*
 * Array of feeling strings
 */
static cptr do_cmd_feeling_text[11] =
{
	"Looks like any other level.",
	"You feel there is something special about this level.",
	"You have a superb feeling about this level.",
	"You have an excellent feeling...",
	"You have a very good feeling...",
	"You have a good feeling...",
	"You feel strangely lucky...",
	"You feel your luck is turning...",
	"You like the look of this place...",
	"This level can't be all bad...",
	"What a boring place..."
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
	/* Verify the feeling */
	if (feeling > 10) feeling = 10;

	/* No useful feeling in town */
	if (!p_ptr->depth)
	{
		msg_print("Looks like a typical town.");
		return;
	}

	/* Display the feeling */
	msg_print(do_cmd_feeling_text[feeling]);
}





/*
 * Encode the screen colors
 */
static char hack[17] = "dwsorgbuDWvyRGBU";


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

	FILE *fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Oops */
	if (!fp) return;


	/* Save screen */
	screen_save();


	/* Clear the screen */
	Term_clear();


	/* Load the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (my_fgets(fp, buf, 1024)) okay = FALSE;


		/* Show each row */
		for (x = 0; x < 79; x++)
		{
			/* Put the attr/char */
			Term_draw(x, y, TERM_WHITE, buf[x]);
		}
	}

	/* Get the blank line */
	if (my_fgets(fp, buf, 1024)) okay = FALSE;


	/* Dump the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (my_fgets(fp, buf, 1024)) okay = FALSE;

		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Look up the attr */
			for (i = 0; i < 16; i++)
			{
				/* Use attr matches */
				if (hack[i] == buf[x]) a = i;
			}

			/* Put the attr/char */
			Term_draw(x, y, a, c);
		}
	}


	/* Close it */
	my_fclose(fp);


	/* Message */
	msg_print("Screen dump loaded.");
	msg_print(NULL);


	/* Load screen */
	screen_load();
}


/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
	int y, x;

	byte a = 0;
	char c = ' ';

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Hack -- drop permissions */
	safe_setuid_drop();

	/* Append to the file */
	fff = my_fopen(buf, "w");

	/* Hack -- grab permissions */
	safe_setuid_grab();

	/* Oops */
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
		fprintf(fff, "%s\n", buf);
	}

	/* Skip a line */
	fprintf(fff, "\n");


	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = hack[a&0x0F];
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		fprintf(fff, "%s\n", buf);
	}

	/* Skip a line */
	fprintf(fff, "\n");


	/* Close it */
	my_fclose(fff);


	/* Message */
	msg_print("Screen dump saved.");
	msg_print(NULL);


	/* Load screen */
	screen_load();
}




/*
 * Display known artifacts
 */
static void do_cmd_knowledge_artifacts(void)
{
	int i, k, z, x, y;

	FILE *fff;

	char file_name[1024];

	char o_name[80];

	bool *okay;


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Allocate the "okay" array */
	C_MAKE(okay, z_info->a_max, bool);

	/* Scan the artifacts */
	for (k = 0; k < z_info->a_max; k++)
	{
		artifact_type *a_ptr = &a_info[k];

		/* Default */
		okay[k] = FALSE;

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Skip "uncreated" artifacts */
		if (!a_ptr->cur_num) continue;

		/* Assume okay */
		okay[k] = TRUE;
	}

	/* Check the dungeon */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			s16b this_o_idx, next_o_idx = 0;

			/* Scan all objects in the grid */
			for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
			{
				object_type *o_ptr;

				/* Get the object */
				o_ptr = &o_list[this_o_idx];

				/* Get the next object */
				next_o_idx = o_ptr->next_o_idx;

				/* Ignore non-artifacts */
				if (!artifact_p(o_ptr)) continue;

				/* Ignore known items */
				if (object_known_p(o_ptr)) continue;

				/* Note the artifact */
				okay[o_ptr->name1] = FALSE;
			}
		}
	}

	/* Check the inventory and equipment */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];

		/* Ignore non-objects */
		if (!o_ptr->k_idx) continue;

		/* Ignore non-artifacts */
		if (!artifact_p(o_ptr)) continue;

		/* Ignore known items */
		if (object_known_p(o_ptr)) continue;

		/* Note the artifact */
		okay[o_ptr->name1] = FALSE;
	}

	/* Scan the artifacts */
	for (k = 0; k < z_info->a_max; k++)
	{
		artifact_type *a_ptr = &a_info[k];

		/* List "dead" ones */
		if (!okay[k]) continue;

		/* Paranoia */
		strcpy(o_name, "Unknown Artifact");

		/* Obtain the base object type */
		z = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Real object */
		if (z)
		{
			object_type *i_ptr;
			object_type object_type_body;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Create fake object */
			object_prep(i_ptr, z);

			/* Make it an artifact */
			i_ptr->name1 = k;

			/* Describe the artifact */
			object_desc_store(o_name, i_ptr, FALSE, 0);
		}

		/* Hack -- Build the artifact name */
		fprintf(fff, "     The %s\n", o_name);
	}

	/* Free the "okay" array */
	C_KILL(okay, z_info->a_max, bool);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Known artifacts", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Display known uniques
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 */
static void do_cmd_knowledge_uniques(void)
{
	int i, n;
	FILE *fff;
	char file_name[1024];
	u16b why = 2;
	u16b *who;


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Allocate the "who" array */
	C_MAKE(who, z_info->r_max, u16b);

	/* Collect matching monsters */
	for (i = 1, n = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Require known monsters */
		if (!cheat_know && !l_ptr->r_sights) continue;

		/* Require unique monsters */
		if (!(r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Collect "appropriate" monsters */
		who[n++] = i;
	}

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array by dungeon depth of monsters */
	ang_sort(who, &why, n);


	/* Print the monsters */
	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = &r_info[who[i]];
		bool dead = (r_ptr->max_num == 0);

		/* Print a message */
		fprintf(fff, "     %s is %s\n",
			    (r_name + r_ptr->name),
			    (dead ? "dead" : "alive"));
	}

	/* Free the "who" array */
	C_KILL(who, z_info->r_max, u16b);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Known Uniques", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Display known objects
 */
static void do_cmd_knowledge_objects(void)
{
	int k;

	FILE *fff;

	char o_name[80];

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Scan the object kinds */
	for (k = 1; k < z_info->k_max; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Hack -- skip artifacts */
		if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;

		/* List known flavored objects */
		if (k_ptr->flavor && k_ptr->aware)
		{
			object_type *i_ptr;
			object_type object_type_body;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Create fake object */
			object_prep(i_ptr, k);

			/* Describe the object */
			object_desc_store(o_name, i_ptr, FALSE, 0);

			/* Print a message */
			fprintf(fff, "     %s\n", o_name);
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Known Objects", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Interact with "knowledge"
 */
void do_cmd_knowledge(void)
{
	char ch;


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Save screen */
	screen_save();


	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Display current knowledge", 2, 0);

		/* Give some choices */
		prt("(1) Display known artifacts", 4, 5);
		prt("(2) Display known uniques", 5, 5);
		prt("(3) Display known objects", 6, 5);

		/* Prompt */
		prt("Command: ", 8, 0);

		/* Prompt */
		ch = inkey();

		/* Done */
		if (ch == ESCAPE) break;

		/* Artifacts */
		if (ch == '1')
		{
			/* Spawn */
			do_cmd_knowledge_artifacts();
		}

		/* Uniques */
		else if (ch == '2')
		{
			/* Spawn */
			do_cmd_knowledge_uniques();
		}

		/* Objects */
		else if (ch == '3')
		{
			/* Spawn */
			do_cmd_knowledge_objects();
		}

		/* Unknown option */
		else
		{
			bell("Illegal command for knowledge!");
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Load screen */
	screen_load();
}


