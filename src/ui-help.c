/**
 * \file ui-help.c
 * \brief In-game help
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "init.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-output.h"
#include "ui-term.h"

/**
 * Make a string lower case.
 */
static void string_lower(char *buf)
{
	char *s;

	/* Lowercase the string */
	for (s = buf; *s != 0; s++) *s = tolower((unsigned char)*s);
}


/**
 * Recursive file perusal.
 *
 * Return false on "?", otherwise true.
 *
 * This function could be made much more efficient with the use of "seek"
 * functionality, especially when moving backwards through a file, or
 * forwards through a file by less than a page at a time.  XXX XXX XXX
 */
bool show_file(const char *name, const char *what, int line, int mode)
{
	int i, k, n;

	struct keypress ch = KEYPRESS_NULL;

	/* Number of "real" lines passed by */
	int next = 0;

	/* Number of "real" lines in the file */
	int size;

	/* Backup value for "line" */
	int back = 0;

	/* This screen has sub-screens */
	bool menu = false;

	/* Case sensitive search */
	bool case_sensitive = false;

	/* Current help file */
	ang_file *fff = NULL;

	/* Find this string (if any) */
	char *find = NULL;

	/* Jump to this tag */
	const char *tag = NULL;

	/* Hold a string to find */
	char finder[80] = "";

	/* Hold a string to show */
	char shower[80] = "";

	/* Filename */
	char filename[1024];

	/* Describe this thing */
	char caption[128] = "";

	/* Path buffer */
	char path[1024];

	/* General buffer */
	char buf[1024];

	/* Lower case version of the buffer, for searching */
	char lc_buf[1024];

	/* Sub-menu information */
	char hook[26][32];

	int wid, hgt;
	
	/* true if we are inside a RST block that should be skipped */
	bool skip_lines = false;



	/* Wipe the hooks */
	for (i = 0; i < 26; i++) hook[i][0] = '\0';

	/* Get size */
	Term_get_size(&wid, &hgt);

	/* Copy the filename */
	my_strcpy(filename, name, sizeof(filename));

	n = strlen(filename);

	/* Extract the tag from the filename */
	for (i = 0; i < n; i++) {
		if (filename[i] == '#') {
			filename[i] = '\0';
			tag = filename + i + 1;
			break;
		}
	}

	/* Redirect the name */
	name = filename;

	/* Currently unused facility to show and describe arbitrary files */
	if (what) {
		my_strcpy(caption, what, sizeof(caption));

		my_strcpy(path, name, sizeof(path));
		fff = file_open(path, MODE_READ, FTYPE_TEXT);
	}

	/* Look in "help" */
	if (!fff) {
		strnfmt(caption, sizeof(caption), "Help file '%s'", name);

		path_build(path, sizeof(path), ANGBAND_DIR_HELP, name);
		fff = file_open(path, MODE_READ, FTYPE_TEXT);
	}

	/* Look in "info" */
	if (!fff) {
		strnfmt(caption, sizeof(caption), "User info file '%s'", name);

		path_build(path, sizeof(path), ANGBAND_DIR_INFO, name);
		fff = file_open(path, MODE_READ, FTYPE_TEXT);
	}

	/* Oops */
	if (!fff) {
		/* Message */
		msg("Cannot open '%s'.", name);
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Oops */
		return (true);
	}


	/* Pre-Parse the file */
	while (true) {
		/* Read a line or stop */
		if (!file_getl(fff, buf, sizeof(buf))) break;

		/* Skip lines if we are inside a RST directive */
		if (skip_lines){
			if (contains_only_spaces(buf))
				skip_lines = false;
			continue;
		}

		/* Parse a very small subset of RST */
		/* TODO: should be more flexible */
		if (prefix(buf, ".. ")) {
			/* parse ".. menu:: [x] filename.txt" (with exact spacing)*/
			if (prefix(buf+strlen(".. "), "menu:: [") && 
                           buf[strlen(".. menu:: [x")]==']') {
				/* This is a menu file */
				menu = true;

				/* Extract the menu item */
				k = A2I(buf[strlen(".. menu:: [")]);

				/* Store the menu item (if valid) */
				if ((k >= 0) && (k < 26))
					my_strcpy(hook[k], buf + strlen(".. menu:: [x] "),
							  sizeof(hook[0]));
			} else if (buf[strlen(".. ")] == '_') {
				/* parse ".. _some_hyperlink_target:" */
				if (tag) {
					/* Remove the closing '>' of the tag */
					buf[strlen(buf) - 1] = '\0';

					/* Compare with the requested tag */
					if (streq(buf + strlen(".. _"), tag)) {
						/* Remember the tagged line */
						line = next;
					}
				}
			}

			/* Skip this and enter skip mode*/
			skip_lines = true;
			continue;
		}

		/* Count the "real" lines */
		next++;
	}

	/* Save the number of "real" lines */
	size = next;


	/* Display the file */
	while (true) {
		/* Clear screen */
		Term_clear();


		/* Restrict the visible range */
		if (line > (size - (hgt - 4))) line = size - (hgt - 4);
		if (line < 0) line = 0;

		skip_lines = false;

		/* Re-open the file if needed */
		if (next > line) {
			/* Close it */
			file_close(fff);

			/* Hack -- Re-Open the file */
			fff = file_open(path, MODE_READ, FTYPE_TEXT);
			if (!fff) return (true);

			/* File has been restarted */
			next = 0;
		}


		/* Goto the selected line */
		while (next < line) {
			/* Get a line */
			if (!file_getl(fff, buf, sizeof(buf))) break;

			/* Skip lines if we are inside a RST directive*/
			if (skip_lines) {
				if (contains_only_spaces(buf))
					skip_lines=false;
				continue;
			}

			/* Skip RST directives */
			if (prefix(buf, ".. ")) {
				skip_lines=true;
				continue;
			}

			/* Count the lines */
			next++;
		}


		/* Dump the next lines of the file */
		for (i = 0; i < hgt - 4; ) {
			/* Hack -- track the "first" line */
			if (!i) line = next;

			/* Get a line of the file or stop */
			if (!file_getl(fff, buf, sizeof(buf))) break;

			/* Skip lines if we are inside a RST directive */
			if (skip_lines) {
				if (contains_only_spaces(buf))
					skip_lines = false;
				continue;
			}

			/* Skip RST directives */
			if (prefix(buf, ".. ")) {
				skip_lines=true;
				continue;
			}

			/* Count the "real" lines */
			next++;

			/* Make a copy of the current line for searching */
			my_strcpy(lc_buf, buf, sizeof(lc_buf));

			/* Make the line lower case */
			if (!case_sensitive) string_lower(lc_buf);

			/* Hack -- keep searching */
			if (find && !i && !strstr(lc_buf, find)) continue;

			/* Hack -- stop searching */
			find = NULL;

			/* Dump the line */
			Term_putstr(0, i+2, -1, COLOUR_WHITE, buf);

			/* Highlight "shower" */
			if (strlen(shower)) {
				const char *str = lc_buf;

				/* Display matches */
				while ((str = strstr(str, shower)) != NULL) {
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-lc_buf, i+2, len, COLOUR_YELLOW,
								&buf[str-lc_buf]);

					/* Advance */
					str += len;
				}
			}

			/* Count the printed lines */
			i++;
		}

		/* Hack -- failed search */
		if (find) {
			bell();
			line = back;
			find = NULL;
			continue;
		}


		/* Show a general "title" */
		prt(format("[%s, %s, Line %d-%d/%d]", buildid,
		           caption, line, line + hgt - 4, size), 0, 0);


		/* Prompt */
		if (menu) {
			/* Menu screen */
			prt("[Press a Letter, or ESC to exit.]", hgt - 1, 0);
		} else if (size <= hgt - 4) {
			/* Small files */
			prt("[Press ESC to exit.]", hgt - 1, 0);
		} else {
			/* Large files */
			prt("[Press Space to advance, or ESC to exit.]", hgt - 1, 0);
		}

		/* Get a keypress */
		ch = inkey();

		/* Exit the help */
		if (ch.code == '?') break;

		/* Toggle case sensitive on/off */
		if (ch.code == '!')
			case_sensitive = !case_sensitive;

		/* Try showing */
		if (ch.code == '&') {
			/* Get "shower" */
			prt("Show: ", hgt - 1, 0);
			(void)askfor_aux(shower, sizeof(shower), NULL);

			/* Make the "shower" lowercase */
			if (!case_sensitive) string_lower(shower);
		}

		/* Try finding */
		if (ch.code == '/') {
			/* Get "finder" */
			prt("Find: ", hgt - 1, 0);
			if (askfor_aux(finder, sizeof(finder), NULL)) {
				/* Find it */
				find = finder;
				back = line;
				line = line + 1;

				/* Make the "finder" lowercase */
				if (!case_sensitive) string_lower(finder);

				/* Show it */
				my_strcpy(shower, finder, sizeof(shower));
			}
		}

		/* Go to a specific line */
		if (ch.code == '#') {
			char tmp[80] = "0";

			prt("Goto Line: ", hgt - 1, 0);
			if (askfor_aux(tmp, sizeof(tmp), NULL))
				line = atoi(tmp);
		}

		/* Go to a specific file */
		if (ch.code == '%') {
			char ftmp[80];

			if (OPT(player, rogue_like_commands)) {
				my_strcpy(ftmp, "r_index.txt", sizeof(ftmp));
			} else {
				my_strcpy(ftmp, "index.txt", sizeof(ftmp));
			}

			prt("Goto File: ", hgt - 1, 0);
			if (askfor_aux(ftmp, sizeof(ftmp), NULL)) {
				if (!show_file(ftmp, NULL, 0, mode))
					ch.code = ESCAPE;
			}
		}

		switch (ch.code) {
			/* up a line */
			case ARROW_UP:
			case 'k':
			case '8': line--; break;

			/* up a page */
			case KC_PGUP:
			case '9':
			case '-': line -= (hgt - 4); break;

			/* home */
			case KC_HOME:
			case '7': line = 0; break;

			/* down a line */
			case ARROW_DOWN:
			case '2':
			case 'j':
			case KC_ENTER: line++; break;

			/* down a page */
			case KC_PGDOWN:
			case '3':
			case ' ': line += hgt - 4; break;

			/* end */
			case KC_END:
			case '1': line = size; break;
		}

		/* Recurse on letters */
		if (menu && isalpha((unsigned char)ch.code)) {
			/* Extract the requested menu item */
			k = A2I(ch.code);

			/* Verify the menu item */
			if ((k >= 0) && (k <= 25) && hook[k][0]) {
				/* Recurse on that file */
				if (!show_file(hook[k], NULL, 0, mode)) ch.code = ESCAPE;
			}
		}

		/* Exit on escape */
		if (ch.code == ESCAPE) break;
	}

	/* Close the file */
	file_close(fff);

	/* Done */
	return (ch.code != '?');
}


/**
 * Peruse the On-Line-Help
 */
void do_cmd_help(void)
{
	/* Save screen */
	screen_save();

	/* Peruse the main help file */
	(void)show_file((OPT(player, rogue_like_commands)) ?
		"r_index.txt" : "index.txt", NULL, 0, 0);

	/* Load screen */
	screen_load();
}


