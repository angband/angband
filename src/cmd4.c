/*
 * File: cmd4.c
 * Purpose: Various kinds of browsing functions.
 *
 * Copyright (c) 1997-2007 Robert A. Koeneke, James E. Wilson, Ben Harrison,
 * Eytan Zweig, Andrew Doull, Pete Mack.
 * Copyright (c) 2004 DarkGod (HTML dump code)
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
		if (!angband_term[j]) continue;

		Term_activate(angband_term[j]);
		Term_redraw();
		Term_fresh();
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
					msg("Character dump failed!");
				else
					msg("Character dump successful.");
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
	msg("Note: %s", tmp);

	/* Add a history entry */
	history_add(tmp, HISTORY_USER_INPUT, 0);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
	/* Silly message */
	msg("You are playing %s %s.  Type '?' for more info.",
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
	if (OPT(birth_no_feelings)) return;

	/* Verify the feeling */
	if (cave->feeling >= N_ELEMENTS(feeling_text))
		cave->feeling = N_ELEMENTS(feeling_text) - 1;

	/* No useful feeling in town */
	if (!p_ptr->depth)
	{
		msg("Looks like a typical town.");
		return;
	}

	/* Display the feeling */
	msg("%s", feeling_text[cave->feeling]);
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
	msg("Screen dump loaded.");
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
	msg("Screen dump saved.");
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
		msg("Screen dump failed.");
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
	process_pref_file(file_name, TRUE);
	file_delete(file_name);
	do_cmd_redraw();

	msg("HTML screen dump saved.");
	message_flush();
}


/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
	msg("Dump type [(t)ext; (h)tml; (f)orum embedded html]:");

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

