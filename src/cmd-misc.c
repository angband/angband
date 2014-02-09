/*
 * File: cmd-misc.c
 * Purpose: Deal with miscellaneous commands.
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
#include "game-cmd.h"
#include "monster/mon-util.h"
#include "wizard.h"
#include "target.h"
#include "prefs.h"
#include "files.h"
#include "buildid.h"
#include "history.h"


/*
 * Toggle wizard mode
 */
void do_cmd_wizard(void)
{
	/* Verify first time */
	if (!(p_ptr->noscore & NOSCORE_WIZARD))
	{
		/* Mention effects */
		msg("You are about to enter 'wizard' mode for the very first time!");
		msg("This is a form of cheating, and your game will not be scored!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_WIZARD;
	}

	/* Toggle mode */
	if (p_ptr->wizard)
	{
		p_ptr->wizard = FALSE;
		msg("Wizard mode off.");
	}
	else
	{
		p_ptr->wizard = TRUE;
		msg("Wizard mode on.");
	}

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw "title" */
	p_ptr->redraw |= (PR_TITLE);
}




#ifdef ALLOW_DEBUG

/*
 * Verify use of "debug" mode
 */
void do_cmd_try_debug(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_DEBUG))
	{
		/* Mention effects */
		msg("You are about to use the dangerous, unsupported, debug commands!");
		msg("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the debug commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_DEBUG;
	}

	/* Okay */
	do_cmd_debug();
}

#endif /* ALLOW_DEBUG */



#ifdef ALLOW_BORG

/*
 * Verify use of "borg" mode
 */
void do_cmd_try_borg(void)
{
	/* Ask first time */
	if (!(p_ptr->noscore & NOSCORE_BORG))
	{
		/* Mention effects */
		msg("You are about to use the dangerous, unsupported, borg commands!");
		msg("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the borg commands? "))
			return;

		/* Mark savefile */
		p_ptr->noscore |= NOSCORE_BORG;
	}

	/* Okay */
	do_cmd_borg();
}

#endif /* ALLOW_BORG */


/*
 * Quit the game.
 */
void do_cmd_quit(cmd_code code, cmd_arg args[])
{
	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;
}


/*
 * Display the options and redraw afterward.
 */
void do_cmd_xxx_options(void)
{
	do_cmd_options();
	do_cmd_redraw();
}


/*
 * Invoked when the command isn't recognised.
 */
void do_cmd_unknown(void)
{
	prt("Type '?' for help.", 0, 0);
}


/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(cmd_code code, cmd_arg args[])
{
	/* Commit suicide */
	p_ptr->is_dead = TRUE;

	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;

	/* Cause of death */
	my_strcpy(p_ptr->died_from, "Quitting", sizeof(p_ptr->died_from));
}


void textui_cmd_suicide(void)
{
	/* Flush input */
	flush();

	/* Verify Retirement */
	if (p_ptr->total_winner)
	{
		/* Verify */
		if (!get_check("Do you want to retire? ")) return;
	}

	/* Verify Suicide */
	else
	{
		struct keypress ch;

		/* Verify */
		if (!get_check("Do you really want to commit suicide? ")) return;

		/* Special Verification for suicide */
		prt("Please verify SUICIDE by typing the '@' sign: ", 0, 0);
		flush();
		ch = inkey();
		prt("", 0, 0);
		if (ch.code != '@') return;
	}

	cmd_insert(CMD_SUICIDE);
}

void do_cmd_save_game(cmd_code code, cmd_arg args[])
{
	save_game();
}


/*
 * Target command
 */
void do_cmd_target(void)
{
	if (target_set_interactive(TARGET_KILL, -1, -1))
		msg("Target Selected.");
	else
		msg("Target Aborted.");
}


void do_cmd_target_closest(void)
{
	target_set_closest(TARGET_KILL);
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

	int a = 0;
	wchar_t c = L' ';

	bool okay = TRUE;

	ang_file *fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");
	fp = file_open(buf, MODE_READ, FTYPE_TEXT);
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

	int a = 0;
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


static void write_html_escape_char(ang_file *fp, wchar_t c)
{
	switch (c)
	{
		case L'<':
			file_putf(fp, "&lt;");
			break;
		case L'>':
			file_putf(fp, "&gt;");
			break;
		case L'&':
			file_putf(fp, "&amp;");
			break;
		default:
			{
				char *mbseq = (char*) mem_alloc(sizeof(char)*(MB_CUR_MAX+1));
				byte len;
				len = wctomb(mbseq, c);
				if (len > MB_CUR_MAX) 
				    len = MB_CUR_MAX;
				mbseq[len] = '\0';
				file_putf(fp, "%s", mbseq);
				mem_free(mbseq);
				break;
			}
	}
}


/* Take an html screenshot */
void html_screenshot(const char *path, int mode)
{
	int y, x;
	int wid, hgt;

	int a = TERM_WHITE;
	int oa = TERM_WHITE;
	int fg_colour = TERM_WHITE;
	int bg_colour = TERM_DARK;
	wchar_t c = L' ';

	const char *new_color_fmt = (mode == 0) ?
					"<font color=\"#%02X%02X%02X\" style=\"background-color: #%02X%02X%02X\">"
				 	: "[COLOR=\"#%02X%02X%02X\"]";
	const char *change_color_fmt = (mode == 0) ?
					"</font><font color=\"#%02X%02X%02X\" style=\"background-color: #%02X%02X%02X\">"
					: "[/COLOR][COLOR=\"#%02X%02X%02X\"]";
	const char *close_color_fmt = mode ==  0 ? "</font>" : "[/COLOR]";

	ang_file *fp;

	fp = file_open(path, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fp)
	{
		plog_fmt("Cannot write the '%s' file!", path);
		return;
	}

	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	if (mode == 0)
	{
		file_putf(fp, "<!DOCTYPE html><html><head>\n");
		file_putf(fp, "  <meta='generator' content='%s'>\n", buildid);
		file_putf(fp, "  <title>%s</title>\n", path);
		file_putf(fp, "</head>\n\n");
		file_putf(fp, "<body style='color: #fff; background: #000;'>\n");
		file_putf(fp, "<pre>\n");
	}
	else 
	{
		file_putf(fp, "[CODE][TT][BC=black][COLOR=white]\n");
	}

	/* Dump the screen */
	for (y = 0; y < hgt; y++)
	{
		for (x = 0; x < wid; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Set the foreground and background */
			fg_colour = a % MAX_COLORS;
			switch (a / MAX_COLORS)
			{
				case BG_BLACK:
					bg_colour = TERM_DARK;
					break;
				case BG_SAME:
					bg_colour = fg_colour;
					break;
				case BG_DARK:
					bg_colour = TERM_SHADE;
					break;
				default:
				assert((a >= BG_BLACK) && (a < BG_MAX * MAX_COLORS));
			}

			/* Color change */
			if (oa != a)
			{
				/* From the default white to another color */
				if (oa == TERM_WHITE)
				{
					file_putf(fp, new_color_fmt,
							angband_color_table[fg_colour][1],
							angband_color_table[fg_colour][2],
							angband_color_table[fg_colour][3],
							angband_color_table[bg_colour][1],
							angband_color_table[bg_colour][2],
							angband_color_table[bg_colour][3]);
				}

				/* From another color to the default white */
				else if (fg_colour == TERM_WHITE &&
						bg_colour == TERM_DARK)
				{
					file_putf(fp, close_color_fmt);
				}

				/* Change colors */
				else
				{
					file_putf(fp, change_color_fmt,
							angband_color_table[fg_colour][1],
							angband_color_table[fg_colour][2],
							angband_color_table[fg_colour][3],
							angband_color_table[bg_colour][1],
							angband_color_table[bg_colour][2],
							angband_color_table[bg_colour][3]);
				}

				/* Remember the last color */
				oa = a;
			}

			/* Write the character and escape special HTML characters */
			if (mode == 0) write_html_escape_char(fp, c);
			else
			{
				char mbseq[MB_LEN_MAX+1] = {0};
				wctomb(mbseq, c);
				file_putf(fp, "%s", mbseq);
			}
		}

		/* End the row */
		file_putf(fp, "\n");
	}

	/* Close the last font-color tag if necessary */
	if (oa != TERM_WHITE) file_putf(fp, close_color_fmt);

	if (mode == 0)
	{
		file_putf(fp, "</pre>\n");
		file_putf(fp, "</body>\n");
		file_putf(fp, "</html>\n");
	}
	else 
	{
		file_putf(fp, "[/COLOR][/BC][/TT][/CODE]\n");
	}

	/* Close it */
	file_close(fp);
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

	/* Ask for a file */
	if (!get_file(mode == 0 ? "dump.html" : "dump.txt",
			tmp_val, sizeof(tmp_val))) return;

	/* Save current preferences */
	path_build(file_name, sizeof(file_name), ANGBAND_DIR_USER, "dump.prf");
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
