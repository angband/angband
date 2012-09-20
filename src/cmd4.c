/* File: cmd4.c */

/* Purpose: Interface commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
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


	/* Hack -- react to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Verify the keymap */
	keymap_init();


	/* Combine and Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);


	/* Update torch */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Forget lite/view */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

	/* Update lite/view */
	p_ptr->update |= (PU_VIEW | PU_LITE);

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw everything */
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_MONSTER | PW_OBJECT);

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
	char	c;

	int		mode = 0;

	char	tmp[160];


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Forever */
	while (1)
	{
		/* Display the player */
		display_player(mode);

		/* Prompt */
		Term_putstr(2, 23, -1, TERM_WHITE,
			"['c' to change name, 'f' to file, 'h' to change mode, or ESC]");

		/* Query */
		c = inkey();

		/* Exit */
		if (c == ESCAPE) break;

		/* Change name */
		if (c == 'c')
		{
			get_name();
		}

		/* File dump */
		else if (c == 'f')
		{
			sprintf(tmp, "%s.txt", player_base);
			if (get_string("File name: ", tmp, 80))
			{
				if (tmp[0] && (tmp[0] != ' '))
				{
					file_character(tmp, FALSE);
				}
			}
		}

		/* Toggle mode */
		else if (c == 'h')
		{
			mode++;
		}

		/* Oops */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
	/* Recall one message XXX XXX XXX */
	prt(format("> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user	-BEN-
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
	int i, j, k, n, q;

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


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Dump up to 20 lines of messages */
		for (j = 0; (j < 20) && (i + j < n); j++)
		{
			cptr msg = message_str(i+j);

			/* Apply horizontal scroll */
			msg = (strlen(msg) >= q) ? (msg + q) : "";

			/* Dump the messages, bottom to top */
			Term_putstr(0, 21-j, -1, TERM_WHITE, msg);

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
		k = inkey();

		/* Exit on Escape */
		if (k == ESCAPE) break;

		/* Hack -- Save the old index */
		j = i;

		/* Horizontal scroll */
		if (k == '4')
		{
			/* Scroll left */
			q = (q >= 40) ? (q - 40) : 0;

			/* Success */
			continue;
		}
		
		/* Horizontal scroll */
		if (k == '6')
		{
			/* Scroll right */
			q = q + 40;

			/* Success */
			continue;
		}
		
		/* Hack -- handle show */
		if (k == '=')
		{
			/* Prompt */
			prt("Show: ", 23, 0);

			/* Get a "shower" string, or continue */
			if (!askfor_aux(shower, 80)) continue;

			/* Okay */
			continue;
		}

		/* Hack -- handle find */
		if (k == '/')
		{
			int z;

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

		/* Recall 1 older message */
		if ((k == '8') || (k == '\n') || (k == '\r'))
		{
			/* Go newer if legal */
			if (i + 1 < n) i += 1;
		}

		/* Recall 10 older messages */
		if (k == '+')
		{
			/* Go older if legal */
			if (i + 10 < n) i += 10;
		}

		/* Recall 20 older messages */
		if ((k == 'p') || (k == KTRL('P')) || (k == ' '))
		{
			/* Go older if legal */
			if (i + 20 < n) i += 20;
		}

		/* Recall 20 newer messages */
		if ((k == 'n') || (k == KTRL('N')))
		{
			/* Go newer (if able) */
			i = (i >= 20) ? (i - 20) : 0;
		}

		/* Recall 10 newer messages */
		if (k == '-')
		{
			/* Go newer (if able) */
			i = (i >= 10) ? (i - 10) : 0;
		}

		/* Recall 1 newer messages */
		if (k == '2')
		{
			/* Go newer (if able) */
			i = (i >= 1) ? (i - 1) : 0;
		}

		/* Hack -- Error of some kind */
		if (i == j) bell();
	}

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}



/*
 * Number of cheating options
 */
#define CHEAT_MAX 6

/*
 * Cheating options
 */
static option_type cheat_info[CHEAT_MAX] =
{
	{ &cheat_peek,		FALSE,	255,	0x01, 0x00,
	"cheat_peek",		"Peek into object creation" },

	{ &cheat_hear,		FALSE,	255,	0x02, 0x00,
	"cheat_hear",		"Peek into monster creation" },

	{ &cheat_room,		FALSE,	255,	0x04, 0x00,
	"cheat_room",		"Peek into dungeon creation" },

	{ &cheat_xtra,		FALSE,	255,	0x08, 0x00,
	"cheat_xtra",		"Peek into something else" },

	{ &cheat_know,		FALSE,	255,	0x10, 0x00,
	"cheat_know",		"Know complete monster info" },

	{ &cheat_live,		FALSE,	255,	0x20, 0x00,
	"cheat_live",		"Allow player to avoid death" }
};


/*
 * Interact with some options for cheating
 */
static void do_cmd_options_cheat(cptr info)
{
	char	ch;

	int		i, k = 0, n = CHEAT_MAX;

	char	buf[80];


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
			        cheat_info[i].o_desc,
			        (*cheat_info[i].o_var ? "yes" : "no "),
			        cheat_info[i].o_text);
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

			case 'y':
			case 'Y':
			case '6':
			{
				noscore |= (cheat_info[k].o_set * 256 + cheat_info[k].o_bit);
				(*cheat_info[k].o_var) = TRUE;
				k = (k + 1) % n;
				break;
			}

			case 'n':
			case 'N':
			case '4':
			{
				(*cheat_info[k].o_var) = FALSE;
				k = (k + 1) % n;
				break;
			}

			default:
			{
				bell();
				break;
			}
		}
	}
}


/*
 * Interact with some options
 */
static void do_cmd_options_aux(int page, cptr info)
{
	char	ch;

	int		i, k = 0, n = 0;

	int		opt[24];

	char	buf[80];


	/* Lookup the options */
	for (i = 0; i < 24; i++) opt[i] = 0;

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		/* Notice options on this "page" */
		if (option_info[i].o_page == page) opt[n++] = i;
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
			        option_info[opt[i]].o_desc,
			        (*option_info[opt[i]].o_var ? "yes" : "no "),
			        option_info[opt[i]].o_text);
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

			case 'y':
			case 'Y':
			case '6':
			{
				(*option_info[opt[k]].o_var) = TRUE;
				k = (k + 1) % n;
				break;
			}

			case 'n':
			case 'N':
			case '4':
			{
				(*option_info[opt[k]].o_var) = FALSE;
				k = (k + 1) % n;
				break;
			}

			default:
			{
				bell();
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

	bool go = TRUE;

	u32b old_flag[8];


	/* Memorize old flags */
	for (j = 0; j < 8; j++)
	{
		/* Acquire current flags */
		old_flag[j] = window_flag[j];
	}


	/* Clear screen */
	Term_clear();

	/* Interact */
	while (go)
	{
		/* Prompt XXX XXX XXX */
		prt("Window flags (<dir>, t, y, n, ESC) ", 0, 0);

		/* Display the windows */
		for (j = 0; j < 8; j++)
		{
			byte a = TERM_WHITE;

			cptr s = angband_term_name[j];

			/* Use color */
			if (use_color && (j == x)) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < 16; i++)
		{
			byte a = TERM_WHITE;

			cptr str = window_flag_desc[i];

			/* Use color */
			if (use_color && (i == y)) a = TERM_L_BLUE;

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
				if (use_color && (i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (window_flag[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ch = inkey();

		/* Analyze */
		switch (ch)
		{
			case ESCAPE:
			{
				go = FALSE;
				break;
			}

			case 'T':
			case 't':
			{
				/* Clear windows */
				for (j = 0; j < 8; j++)
				{
					window_flag[j] &= ~(1L << y);
				}

				/* Clear flags */
				for (i = 0; i < 16; i++)
				{
					window_flag[x] &= ~(1L << i);
				}

				/* Fall through */
			}

			case 'y':
			case 'Y':
			{
				/* Ignore screen */
				if (x == 0) break;

				/* Set flag */
				window_flag[x] |= (1L << y);
				break;
			}

			case 'n':
			case 'N':
			{
				/* Clear flag */
				window_flag[x] &= ~(1L << y);
				break;
			}

			default:
			{
				d = keymap_dirs[ch & 0x7F];

				x = (x + ddx[d] + 8) % 8;
				y = (y + ddy[d] + 16) % 16;

				if (!d) bell();
			}
		}
	}

	/* Notice changes */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* Dead window */
		if (!angband_term[j]) continue;

		/* Ignore non-changes */
		if (window_flag[j] == old_flag[j]) continue;

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
 * Set or unset various options.
 *
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 */
void do_cmd_options(void)
{
	int k;


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


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

		/* Testing */
		prt("(T) Testing options", 9, 5);

		/* Cheating */
		prt("(C) Cheating Options", 10, 5);

		/* Window flags */
		prt("(W) Window flags", 12, 5);

		/* Special choices */
		prt("(D) Base Delay Factor", 14, 5);
		prt("(H) Hitpoint Warning", 15, 5);

		/* Prompt */
		prt("Command: ", 18, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE) break;

		/* Analyze */
		switch (k)
		{
			/* General Options */
			case '1':
			{
				/* Process the general options */
				do_cmd_options_aux(1, "User Interface Options");
				break;
			}

			/* Disturbance Options */
			case '2':
			{
				/* Spawn */
				do_cmd_options_aux(2, "Disturbance Options");
				break;
			}

			/* Inventory Options */
			case '3':
			{
				/* Spawn */
				do_cmd_options_aux(3, "Game-Play Options");
				break;
			}

			/* Efficiency Options */
			case '4':
			{
				/* Spawn */
				do_cmd_options_aux(4, "Efficiency Options");
				break;
			}

			/* Testing Options */
			case 'T':
			{
				/* Spawn */
				do_cmd_options_aux(255, "Testing Options");
				break;
			}

			/* Cheating Options */
			case 'C':
			{
				/* Spawn */
				do_cmd_options_cheat("Cheaters never win");
				break;
			}

			/* Window flags */
			case 'W':
			case 'w':
			{
				/* Spawn */
				do_cmd_options_win();
				break;
			}

			/* Hack -- Delay Speed */
			case 'D':
			case 'd':
			{
				/* Prompt */
				prt("Command: Base Delay Factor", 18, 0);

				/* Get a new value */
				while (1)
				{
					int msec = delay_factor * delay_factor * delay_factor;
					prt(format("Current base delay factor: %d (%d msec)",
					           delay_factor, msec), 22, 0);
					prt("Delay Factor (0-9 or ESC to accept): ", 20, 0);
					k = inkey();
					if (k == ESCAPE) break;
					if (isdigit(k)) delay_factor = D2I(k);
					else bell();
				}

				break;
			}

			/* Hack -- hitpoint warning factor */
			case 'H':
			case 'h':
			{
				/* Prompt */
				prt("Command: Hitpoint Warning", 18, 0);

				/* Get a new value */
				while (1)
				{
					prt(format("Current hitpoint warning: %d0%%",
					           hitpoint_warn), 22, 0);
					prt("Hitpoint Warning (0-9 or ESC to accept): ", 20, 0);
					k = inkey();
					if (k == ESCAPE) break;
					if (isdigit(k)) hitpoint_warn = D2I(k);
					else bell();
				}

				break;
			}

			/* Unknown option */
			default:
			{
				/* Oops */
				bell();
				break;
			}
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;


	/* Verify the keymap */
	keymap_init();
}



/*
 * Ask for a "user pref line" and process it
 *
 * XXX XXX XXX Allow absolute file names?
 */
void do_cmd_pref(void)
{
	char buf[80];

	/* Default */
	strcpy(buf, "");

	/* Ask for a "user pref command" */
	if (!get_string("Pref: ", buf, 80)) return;

	/* Process that pref command */
	(void)process_pref_file_aux(buf);
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


	/* Skip space */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic macro dump\n\n");

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		fprintf(fff, "# Macro '%d'\n\n", i);

		/* Extract the action */
		ascii_to_text(buf, macro__act[i]);

		/* Dump the macro */
		fprintf(fff, "A:%s\n", buf);

		/* Extract the action */
		ascii_to_text(buf, macro__pat[i]);

		/* Dump command macros */
		if (macro__cmd[i]) fprintf(fff, "C:%s\n", buf);

		/* Dump normal macros */
		else fprintf(fff, "P:%s\n", buf);

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
 * Note that both "flush()" calls are extremely important.
 */
static void do_cmd_macro_aux(char *buf)
{
	int i, n = 0;

	char tmp[1024];


	/* Flush */
	flush();

	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	i = inkey();

	/* Read the pattern */
	while (i)
	{
		/* Save the key */
		buf[n++] = i;

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		i = inkey();
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

#endif


/*
 * Interact with "macros"
 *
 * Note that the macro "action" must be defined before the trigger.
 *
 * XXX XXX XXX Need messages for success, plus "helpful" info
 */
void do_cmd_macros(void)
{
	int i;

	char tmp[160];

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Describe */
		Term_putstr(0, 2, -1, TERM_WHITE, "Interact with Macros");


		/* Describe that action */
		Term_putstr(0, 18, -1, TERM_WHITE, "Current action (if any) shown below:");

		/* Analyze the current action */
		ascii_to_text(buf, macro__buf);

		/* Display the current action */
		Term_putstr(0, 20, -1, TERM_WHITE, buf);


		/* Selections */
		Term_putstr(5,  4, -1, TERM_WHITE, "(1) Load a user pref file");
#ifdef ALLOW_MACROS
		Term_putstr(5,  5, -1, TERM_WHITE, "(2) Dump macros");
		Term_putstr(5,  6, -1, TERM_WHITE, "(3) Enter a new action");
		Term_putstr(5,  7, -1, TERM_WHITE, "(4) Query a macro action");
		Term_putstr(5,  8, -1, TERM_WHITE, "(5) Create a command macro");
		Term_putstr(5,  9, -1, TERM_WHITE, "(6) Create a normal macro");
		Term_putstr(5, 10, -1, TERM_WHITE, "(7) Create an identity macro");
		Term_putstr(5, 11, -1, TERM_WHITE, "(8) Create an empty macro");
		Term_putstr(5, 12, -1, TERM_WHITE, "(9) Define a special keymap");
#endif /* ALLOW_MACROS */

		/* Prompt */
		Term_putstr(0, 14, -1, TERM_WHITE, "Command: ");

		/* Get a command */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Load a 'macro' file */
		else if (i == '1')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Load a user pref file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 16, -1, TERM_WHITE, "File: ");

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);
		}

#ifdef ALLOW_MACROS

		/* Save a 'macro' file */
		else if (i == '2')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Save a macro file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 16, -1, TERM_WHITE, "File: ");

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70)) continue;

			/* Drop priv's */
			safe_setuid_drop();

			/* Dump the macros */
			(void)macro_dump(tmp);

			/* Grab priv's */
			safe_setuid_grab();
		}

		/* Enter a new action */
		else if (i == '3')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Enter a new action");

			/* Go to the correct location */
			Term_gotoxy(0, 20);

			/* Hack -- limit the value */
			tmp[80] = '\0';

			/* Get an encoded action */
			if (!askfor_aux(buf, 80)) continue;

			/* Extract an action */
			text_to_ascii(macro__buf, buf);
		}

		/* Query a macro action */
		else if (i == '4')
		{

#if 0
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Query a macro action");

			/* Prompt */
			Term_putstr(0, 16, -1, TERM_WHITE, "Enter a macro trigger: ");

			/* Get a macro trigger */
			do_cmd_macro_aux(buf);
#endif

			/* XXX XXX XXX */
			msg_print("Command not ready.");
		}

		/* Create a command macro */
		else if (i == '5')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create a command macro");

			/* Prompt */
			Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			do_cmd_macro_aux(buf);

			/* Link the macro */
			macro_add(buf, macro__buf, TRUE);

			/* Message */
			msg_print("Created a new command macro.");
		}

		/* Create a normal macro */
		else if (i == '6')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create a normal macro");

			/* Prompt */
			Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			do_cmd_macro_aux(buf);

			/* Link the macro */
			macro_add(buf, macro__buf, FALSE);

			/* Message */
			msg_print("Created a new normal macro.");
		}

		/* Create an identity macro */
		else if (i == '7')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create an identity macro");

			/* Prompt */
			Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			do_cmd_macro_aux(buf);

			/* Link the macro */
			macro_add(buf, buf, FALSE);

			/* Message */
			msg_print("Created a new identity macro.");
		}

		/* Create an empty macro */
		else if (i == '8')
		{
			/* Prompt */
			Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create an empty macro");

			/* Prompt */
			Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			do_cmd_macro_aux(buf);

			/* Link the macro */
			macro_add(buf, "", FALSE);

			/* Message */
			msg_print("Created a new empty macro.");
		}

		/* Define a keymap */
		else if (i == '9')
		{
			char i1, i2, i3;

			/* Flush the input */
			flush();

			/* Get the trigger */
			if (get_com("Type the trigger keypress: ", &i1) &&
			    get_com("Type the resulting keypress: ", &i2) &&
			    get_com("Type a direction (or zero): ", &i3))
			{
				/* Acquire the array index */
				int k = (i1 & 0x7F);
				int r = (i2 & 0x7F);
				int d = (i3 & 0x7F);

				/* Analyze the result key */
				keymap_cmds[k] = r;

				/* Hack -- Analyze the "direction" (always allow numbers) */
				keymap_dirs[k] = (isdigit(d) ? (d - '0') : keymap_dirs[d]);

				/* Success */
				msg_format("Keypress 0x%02x mapped to 0x%02x (direction %d)",
				           k, keymap_cmds[k], keymap_dirs[k]);
			}
		}

#endif /* ALLOW_MACROS */

		/* Oops */
		else
		{
			/* Oops */
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Load the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}



/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
	int i;

	FILE *fff;

	char tmp[160];

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


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
		i = inkey();

		/* Done */
		if (i == ESCAPE) break;

		/* Load a 'pref' file */
		else if (i == '1')
		{
			/* Prompt */
			prt("Command: Load a user pref file", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Query */
			if (!askfor_aux(tmp, 70)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);
		}

#ifdef ALLOW_VISUALS

		/* Dump monster attr/chars */
		else if (i == '2')
		{
			/* Prompt */
			prt("Command: Dump monster attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Monster attr/char definitions\n\n");

			/* Dump monsters */
			for (i = 0; i < MAX_R_IDX; i++)
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
		else if (i == '3')
		{
			/* Prompt */
			prt("Command: Dump object attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Object attr/char definitions\n\n");

			/* Dump objects */
			for (i = 0; i < MAX_K_IDX; i++)
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
		else if (i == '4')
		{
			/* Prompt */
			prt("Command: Dump feature attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Feature attr/char definitions\n\n");

			/* Dump features */
			for (i = 0; i < MAX_F_IDX; i++)
			{
				feature_type *f_ptr = &f_info[i];

				/* Skip non-entries */
				if (!f_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (f_name + f_ptr->name));

				/* Dump the feature attr/char info */
				fprintf(fff, "F:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(f_ptr->z_attr), (byte)(f_ptr->z_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped feature attr/chars.");
		}

		/* Modify monster attr/chars */
		else if (i == '6')
		{
			static int r = 0;

			/* Prompt */
			prt("Command: Change monster attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				monster_race *r_ptr = &r_info[r];

				int da = (byte)(r_ptr->d_attr);
				int dc = (byte)(r_ptr->d_char);
				int ca = (byte)(r_ptr->x_attr);
				int cc = (byte)(r_ptr->x_char);

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
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') r = (r + MAX_R_IDX + 1) % MAX_R_IDX;
				if (i == 'N') r = (r + MAX_R_IDX - 1) % MAX_R_IDX;
				if (i == 'a') r_ptr->x_attr = (byte)(ca + 1);
				if (i == 'A') r_ptr->x_attr = (byte)(ca - 1);
				if (i == 'c') r_ptr->x_char = (byte)(cc + 1);
				if (i == 'C') r_ptr->x_char = (byte)(cc - 1);
			}
		}

		/* Modify object attr/chars */
		else if (i == '7')
		{
			static int k = 0;

			/* Prompt */
			prt("Command: Change object attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				object_kind *k_ptr = &k_info[k];

				int da = (byte)k_ptr->k_attr;
				int dc = (byte)k_ptr->k_char;
				int ca = (byte)k_ptr->x_attr;
				int cc = (byte)k_ptr->x_char;

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
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') k = (k + MAX_K_IDX + 1) % MAX_K_IDX;
				if (i == 'N') k = (k + MAX_K_IDX - 1) % MAX_K_IDX;
				if (i == 'a') k_info[k].x_attr = (byte)(ca + 1);
				if (i == 'A') k_info[k].x_attr = (byte)(ca - 1);
				if (i == 'c') k_info[k].x_char = (byte)(cc + 1);
				if (i == 'C') k_info[k].x_char = (byte)(cc - 1);
			}
		}

		/* Modify feature attr/chars */
		else if (i == '8')
		{
			static int f = 0;

			/* Prompt */
			prt("Command: Change feature attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				feature_type *f_ptr = &f_info[f];

				int da = (byte)f_ptr->f_attr;
				int dc = (byte)f_ptr->f_char;
				int ca = (byte)f_ptr->z_attr;
				int cc = (byte)f_ptr->z_char;

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
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') f = (f + MAX_F_IDX + 1) % MAX_F_IDX;
				if (i == 'N') f = (f + MAX_F_IDX - 1) % MAX_F_IDX;
				if (i == 'a') f_info[f].z_attr = (byte)(ca + 1);
				if (i == 'A') f_info[f].z_attr = (byte)(ca - 1);
				if (i == 'c') f_info[f].z_char = (byte)(cc + 1);
				if (i == 'C') f_info[f].z_char = (byte)(cc - 1);
			}
		}

#endif

		/* Reset visuals */
		else if (i == '0')
		{
			/* Reset */
			reset_visuals();

			/* Message */
			msg_print("Visual attr/char tables reset.");
		}

		/* Unknown option */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
	int i;

	FILE *fff;

	char tmp[160];

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


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
		i = inkey();

		/* Done */
		if (i == ESCAPE) break;

		/* Load a 'pref' file */
		if (i == '1')
		{
			/* Prompt */
			prt("Command: Load a user pref file", 8, 0);

			/* Prompt */
			prt("File: ", 10, 0);

			/* Default file */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Query */
			if (!askfor_aux(tmp, 70)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);

			/* Mega-Hack -- react to changes */
			Term_xtra(TERM_XTRA_REACT, 0);

			/* Mega-Hack -- redraw */
			Term_redraw();
		}

#ifdef ALLOW_COLORS

		/* Dump colors */
		else if (i == '2')
		{
			/* Prompt */
			prt("Command: Dump colors", 8, 0);

			/* Prompt */
			prt("File: ", 10, 0);

			/* Default filename */
			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Drop priv's */
			safe_setuid_drop();

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Grab priv's */
			safe_setuid_grab();

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
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
		else if (i == '3')
		{
			static int a = 0;

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
					Term_putstr(i*4, 22, -1, i, format("%3d", i));
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
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') a = (byte)(a + 1);
				if (i == 'N') a = (byte)(a - 1);
				if (i == 'k') angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
				if (i == 'K') angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
				if (i == 'r') angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
				if (i == 'R') angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
				if (i == 'g') angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
				if (i == 'G') angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
				if (i == 'b') angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
				if (i == 'B') angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

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
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
	char buf[80];

	/* Default */
	strcpy(buf, "");

	/* Input */
	if (!get_string("Note: ", buf, 60)) return;

	/* Ignore empty notes */
	if (!buf[0] || (buf[0] == ' ')) return;

	/* Add the note to the message recall */
	msg_format("Note: %s", buf);
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
	if (feeling < 0) feeling = 0;
	if (feeling > 10) feeling = 10;

	/* No useful feeling in town */
	if (!dun_level)
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
 */
void do_cmd_load_screen(void)
{
	int i, y, x;

	byte a = 0;
	char c = ' ';

	bool okay = TRUE;

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

	/* Append to the file */
	fff = my_fopen(buf, "r");

	/* Oops */
	if (!fff) return;


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Clear the screen */
	Term_clear();


	/* Load the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (my_fgets(fff, buf, 1024)) okay = FALSE;

		/* Show each row */
		for (x = 0; x < 79; x++)
		{
			/* Put the attr/char */
			Term_draw(x, y, TERM_WHITE, buf[x]);
		}
	}

	/* Get the blank line */
	if (my_fgets(fff, buf, 1024)) okay = FALSE;


	/* Dump the screen */
	for (y = 0; okay && (y < 24); y++)
	{
		/* Get a line of data */
		if (my_fgets(fff, buf, 1024)) okay = FALSE;

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

			/* Hack -- fake monochrome */
			if (!use_color) a = TERM_WHITE;

			/* Put the attr/char */
			Term_draw(x, y, a, c);
		}

		/* End the row */
		fprintf(fff, "\n");
	}


	/* Get the blank line */
	if (my_fgets(fff, buf, 1024)) okay = FALSE;


	/* Close it */
	my_fclose(fff);


	/* Message */
	msg_print("Screen dump loaded.");
	msg_print(NULL);


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
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


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


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


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}




/*
 * Check the status of "artifacts"
 */
void do_cmd_check_artifacts(void)
{
	int i, k, z, x, y;

	FILE *fff;

	char file_name[1024];

	char base_name[80];

	bool okay[MAX_A_IDX];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Scan the artifacts */
	for (k = 0; k < MAX_A_IDX; k++)
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
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			s16b this_o_idx, next_o_idx = 0;

			/* Scan all objects in the grid */
			for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
			{
				object_type *o_ptr;
			
				/* Acquire object */
				o_ptr = &o_list[this_o_idx];

				/* Acquire next object */
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
	for (k = 0; k < MAX_A_IDX; k++)
	{
		artifact_type *a_ptr = &a_info[k];

		/* List "dead" ones */
		if (!okay[k]) continue;

		/* Paranoia */
		strcpy(base_name, "Unknown Artifact");

		/* Obtain the base object type */
		z = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Real object */
		if (z)
		{
			object_type forge;
			object_type *q_ptr;

			/* Get local object */
			q_ptr = &forge;

			/* Create fake object */
			object_prep(q_ptr, z);

			/* Make it an artifact */
			q_ptr->name1 = k;

			/* Describe the artifact */
			object_desc_store(base_name, q_ptr, FALSE, 0);
		}

		/* Hack -- Build the artifact name */
		fprintf(fff, "     The %s\n", base_name);
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Artifacts Seen");

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Check the status of "uniques"
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 */
void do_cmd_check_uniques(void)
{
	int k;

	FILE *fff;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Scan the monster races */
	for (k = 1; k < MAX_R_IDX-1; k++)
	{
		monster_race *r_ptr = &r_info[k];

		/* Only print Uniques */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			bool dead = (r_ptr->max_num == 0);

			/* Only display "known" uniques */
			if (dead || cheat_know || r_ptr->r_sights)
			{
				/* Print a message */
				fprintf(fff, "     %s is %s\n",
				        (r_name + r_ptr->name),
				        (dead ? "dead" : "alive"));
			}
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Known Uniques");

	/* Remove the file */
	fd_kill(file_name);
}



