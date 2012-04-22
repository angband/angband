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
#include "buildid.h"
#include "cave.h"
#include "cmds.h"
#include "externs.h"
#include "files.h"
#include "history.h"
#include "object/tvalsval.h"
#include "option.h"
#include "prefs.h"
#include "squelch.h"
#include "ui.h"
#include "ui-menu.h"
#include "button.h"



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
	handle_stuff(p_ptr);

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
	ui_event ke;
	int mode = 0;

	const char *p;

	bool more = TRUE;
	bool button_state;

	if (OPT(mouse_movement)) {
		/**
		 * show some buttons on the last line regardless of whether
		 * mouse buttons are usually shown
		 */
		button_state = OPT(mouse_buttons);
		if (!button_state) {
			option_set("mouse_buttons", TRUE);
		}
	} else {
		/* make sure we do not change the state of the mouse_buttons option below */
		button_state = TRUE;
	}

	/* Prompt */
	p = "['c' to change name, 'f' to file, 'h' to change mode, or ESC]";

	/* Save screen */
	screen_save();

	/* backup the previous buttons and clear them */
	button_backup_all();
	button_kill_all();

	/* add some 1d buttons for if we are using them */
	button_add("[c]", 'c');
	button_add("[f]", 'f');
	button_add("[->]", 'h');
	button_add("[<-]", 'l');

	button_add("[Ret]",'\n');
	button_add("[ESC]",ESCAPE);

	/* Forever */
	while (more)
	{
		/* Display the player */
		display_player(mode);

		/* Prompt */
		Term_putstr(2, 23, -1, TERM_WHITE, p);
		/* display the mouse buttons */
		if (OPT(mouse_buttons)) {
			if (Term->hgt == 24) {
				button_print(23, 2+62);
			} else {
				button_print(Term->hgt - 1, COL_MAP);
			}
		}

		/* Query */
		ke = inkey_ex();

		if ((ke.type == EVT_KBRD)||(ke.type == EVT_BUTTON)) {
			switch (ke.key.code) {
				case ESCAPE: more = FALSE; break;
				case 'c': {
					char namebuf[32] = "";

					if (get_name(namebuf, sizeof namebuf))
					{
						/* Set player name */
						my_strcpy(op_ptr->full_name, namebuf,
								  sizeof(op_ptr->full_name));

						/* Don't change savefile name. */
						process_player_name(FALSE);
					}
					break;
				}

				case 'f': {
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
					break;
				}
				
				case 'h':
				case ARROW_LEFT:
				case ' ':
					mode = (mode + 1) % INFO_SCREENS;
					break;

				case 'l':
				case ARROW_RIGHT:
					mode = (mode - 1) % INFO_SCREENS;
					break;
			}
		} else if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				/* Flip through the screens */			
				mode = (mode + 1) % INFO_SCREENS;
			} else
			if (ke.mouse.button == 2) {
				/* exit the screen */
				more = FALSE;
			} else
			{
				/* Flip backwards through the screens */			
				mode = (mode - 1) % INFO_SCREENS;
			}
		}

		/* Flush messages */
		message_flush();
	}
	/* Remove the 1d buttons that we added earlier */
	button_restore();
	if (!button_state) {
		option_set("mouse_buttons", FALSE);
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
	ui_event ke;

	bool more = TRUE;

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
	while (more)
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
		if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				if (ke.mouse.y <= hgt / 2) {
					/* Go older if legal */
					if (i + 20 < n)
						i += 20;
				} else {
					/* Go newer */
					i = (i >= 20) ? (i - 20) : 0;
				}
			} else
			if (ke.mouse.button == 2) {
				more = FALSE;
			}
		} else if (ke.type == EVT_KBRD) {
			switch (ke.key.code) {
				case ESCAPE: {
					more = FALSE;
					break;
				}

				case '=': {
					/* Get the string to find */
					prt("Find: ", hgt - 1, 0);
					if (!askfor_aux(shower, sizeof shower, NULL)) continue;
		
					/* Set to find */
					ke.key.code = '-';
					break;
				}

				case ARROW_LEFT:
				case '4':
					q = (q >= wid / 2) ? (q - wid / 2) : 0;
					break;

				case ARROW_RIGHT:
				case '6':
					q = q + wid / 2;
					break;

				case ARROW_UP:
				case '8':
					if (i + 1 < n) i += 1;
					break;

				case ARROW_DOWN:
				case '2':
				case KC_ENTER:
					i = (i >= 1) ? (i - 1) : 0;
					break;

				case KC_PGUP:
				case 'p':
				case ' ':
					if (i + 20 < n) i += 20;
					break;

				case KC_PGDOWN:
				case 'n':
					i = (i >= 20) ? (i - 20) : 0;
					break;
			}
		}

		/* Find the next item */
		if (ke.key.code == '-' && shower[0])
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

/**
 * Record the player's thoughts as a note.
 *
 * This both displays the note back to the player and adds it to the game log.
 * Two fancy note types are supported: notes beginning with "/say" will be
 * written as 'Frodo says: "____"', and notes beginning with "/me" will
 * be written as 'Frodo ____'.
 */
void do_cmd_note(void) {
	/* Allocate/Initialize strings to get and format user input. */
	char tmp[70];
	char note[90];
	my_strcpy(tmp, "", sizeof(tmp));
	my_strcpy(note, "", sizeof(note));

	/* Read a line of input from the user */
	if (!get_string("Note: ", tmp, sizeof(tmp))) return;

	/* Ignore empty notes */
	if (!tmp[0] || (tmp[0] == ' ')) return;

	/* Format the note correctly, supporting some cute /me commands */
	if (strncmp(tmp, "/say ", 5) == 0)
		strnfmt(note, sizeof(note), "-- %s says: \"%s\"", op_ptr->full_name, &tmp[5]);
	else if (strncmp(tmp, "/me", 3) == 0)
		strnfmt(note, sizeof(note), "-- %s%s", op_ptr->full_name, &tmp[3]);
	else
		strnfmt(note, sizeof(note), "-- Note: %s", tmp);

	/* Display the note (omitting the "-- " prefix) */
	msg(&note[3]);

	/* Add a history entry */
	history_add(note, HISTORY_USER_INPUT, 0);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
	/* Silly message */
	msg("You are playing %s.  Type '?' for more info.", buildver);
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
 * Array of feeling strings for object feelings.
 * Keep strings at 36 or less characters to keep the
 * combined feeling on one row.
 */
static const char *obj_feeling_text[] =
{
	"Looks like any other level.",
	"you sense an item of wondrous power!",
	"there are superb treasures here.",
	"there are excellent treasures here.",
	"there are very good treasures here.",
	"there are good treasures here.",
	"there may be something worthwhile here.",
	"there may not be much interesting here.",
	"there aren't many treasures here.",
	"there are only scraps of junk here.",
	"there are naught but cobwebs here."
};

/*
 * Array of feeling strings for monster feelings.
 * Keep strings at 36 or less characters to keep the
 * combined feeling on one row.
 */
static const char *mon_feeling_text[] =
{
	/* first string is just a place holder to 
	 * maintain symmetry with obj_feeling.
	 */
	"You are still uncertain about this place",
	"Omens of death haunt this place",
	"This place seems murderous",
	"This place seems terribly dangerous",
	"You feel anxious about this place",
	"You feel nervous about this place",
	"This place does not seem too risky",
	"This place seems reasonably safe",
	"This seems a tame, sheltered place",
	"This seems a quiet, peaceful place"
};

/*
 * Display the feeling.  Players always get a monster feeling.
 * Object feelings are delayed until the player has explored some
 * of the level.
 */

void display_feeling(bool obj_only)
{
	u16b obj_feeling = cave->feeling / 10;
	u16b mon_feeling = cave->feeling - (10 * obj_feeling);
	const char *join;

	/* Don't show feelings for cold-hearted characters */
	if (OPT(birth_no_feelings)) return;

	/* No useful feeling in town */
	if (!p_ptr->depth) {
		msg("Looks like a typical town.");
		return;
	}
	
	/* Display only the object feeling when it's first discovered. */
	if (obj_only){
		msg("You feel that %s", obj_feeling_text[obj_feeling]);
		return;
	}
	
	/* Players automatically get a monster feeling. */
	if (cave->feeling_squares < FEELING1){
		msg("%s.", mon_feeling_text[mon_feeling]);
		return;
	}
	
	/* Verify the feelings */
	if (obj_feeling >= N_ELEMENTS(obj_feeling_text))
		obj_feeling = N_ELEMENTS(obj_feeling_text) - 1;

	if (mon_feeling >= N_ELEMENTS(mon_feeling_text))
		mon_feeling = N_ELEMENTS(mon_feeling_text) - 1;

	/* Decide the conjunction */
	if ((mon_feeling <= 5 && obj_feeling > 6) ||
			(mon_feeling > 5 && obj_feeling <= 6))
		join = ", yet";
	else
		join = ", and";

	/* Display the feeling */
	msg("%s%s %s", mon_feeling_text[mon_feeling], join,
		obj_feeling_text[obj_feeling]);
}


void do_cmd_feeling(void)
{
	display_feeling(FALSE);
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
	wchar_t c = L' ';

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
			Term_mbstowcs(&c, &buf[x], 1);
			/* Put the attr/char */
			Term_draw(x, y, TERM_WHITE, c);
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
	wchar_t c = L' ';

	ang_file *fff;

	char buf[1024];
	char *p;

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");
	fff = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	if (!fff) return;


	/* Save screen */
	screen_save();


	/* Dump the screen */
	for (y = 0; y < 24; y++)
	{
		p = buf;
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			p += wctomb(p, c);
		}

		/* Terminate */
		*p = '\0';

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
	process_pref_file(file_name, TRUE, FALSE);
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
	char ch;
	ch = get_char("Dump as (T)ext, (H)TML, or (F)orum text? ", "thf", 3, ' ');

	switch (ch)
	{
		case 't': do_cmd_save_screen_text(); break;
		case 'h': do_cmd_save_screen_html(0); break;
		case 'f': do_cmd_save_screen_html(1); break;
	}
}

