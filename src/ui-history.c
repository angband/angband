/**
 * \file ui-history.c
 * \brief Character auto-history display UI
 *
 * Copyright (c) 2007 J.D. White
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
#include "player-history.h"
#include "ui-history.h"
#include "ui-input.h"
#include "ui-output.h"

/**
 * Print the header for the history display
 */
static void print_history_header(void)
{
	c_put_str(COLOUR_WHITE, "[Player history]", 0, 0);
	c_put_str(COLOUR_L_BLUE, "      Turn   Depth  Note", 1, 0);
}


/**
 * Handles all of the display functionality for the history list.
 */
void history_display(void)
{
	struct history_info *history_list_local = NULL;
	size_t max_item = history_get_list(player, &history_list_local);
	int row, wid, hgt, page_size;
	char buf[120];
	static size_t first_item = 0;
	size_t i;
	bool active = true;

	Term_get_size(&wid, &hgt);

	/* Five lines provide space for the header and footer */
	page_size = hgt - 5;

	screen_save();

	while (active)
	{
		struct keypress ch;

		Term_clear();

		/* Print everything to screen */
		print_history_header();

		row = 0;
		for (i = first_item; row <= page_size && i < max_item; i++)
		{
			strnfmt(buf, sizeof(buf), "%10d%7d\'  %s",
				history_list_local[i].turn,
				history_list_local[i].dlev * 50,
				history_list_local[i].event);

			if (hist_has(history_list_local[i].type, HIST_ARTIFACT_LOST))
				my_strcat(buf, " (LOST)", sizeof(buf));

			/* Size of header = 3 lines */
			prt(buf, row + 2, 0);
			row++;
		}
		prt("[Arrow keys scroll, p/PgUp for previous page, n/PgDn for next page, ESC to exit.]", hgt - 1, 0);

		ch = inkey();

		switch (ch.code) {
			case 'n':
			case ' ':
			case KC_PGDOWN: {
				size_t scroll_to = first_item + page_size;
				first_item = (scroll_to < max_item ? scroll_to : max_item);
				break;
			}

			case 'p':
			case KC_PGUP: {
				int scroll_to = first_item - page_size;
				first_item = (scroll_to >= 0 ? scroll_to : 0);
				break;
			}

			case 'j':
			case ARROW_DOWN: {
				size_t scroll_to = first_item + 1;
				first_item = (scroll_to < max_item ? scroll_to : max_item);
				break;
			}

			case 'k':
			case ARROW_UP: {
				int scroll_to = first_item - 1;
				first_item = (scroll_to >= 0 ? scroll_to : 0);
				break;
			}

			case ESCAPE:
				active = false;
				break;
		}
	}

	screen_load();

	return;
}


/**
 * Dump character history to a file, which we assume is already open.
 */
void dump_history(ang_file *file)
{
	struct history_info *history_list_local = NULL;
	size_t max_item = history_get_list(player, &history_list_local);
	size_t i;
	char buf[120];

	file_putf(file, "[Player history]\n");
	file_putf(file, "      Turn   Depth  Note\n");

	for (i = 0; i < max_item; i++) {
		strnfmt(buf, sizeof(buf), "%10d%7d\'  %s",
				history_list_local[i].turn,
				history_list_local[i].dlev * 50,
				history_list_local[i].event);

		if (hist_has(history_list_local[i].type, HIST_ARTIFACT_LOST))
			my_strcat(buf, " (LOST)", sizeof(buf));

		file_putf(file, "%s", buf);
		file_put(file, "\n");
	}

	return;
}
