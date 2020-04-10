/**
 * \file ui-command.c
 * \brief Deal with UI only command processing.
 *
 * Copyright (c) 1997-2014 Angband developers
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
#include "cmd-core.h"
#include "cmds.h"
#include "game-input.h"
#include "game-world.h"
#include "init.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "target.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-event.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-map.h"
#include "ui-menu.h"
#include "ui-options.h"
#include "ui-output.h"
#include "ui-player.h"
#include "ui-prefs.h"
#include "ui-target.h"
#include "wizard.h"



/**
 * Redraw the screen
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
	event_signal(EVENT_INPUT_FLUSH);

	if (character_dungeon)
		verify_panel();

	/* Hack -- React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);

	if (character_dungeon) {
		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Update torch, gear */
		player->upkeep->update |= (PU_TORCH | PU_INVEN);

		/* Update stuff */
		player->upkeep->update |= (PU_BONUS | PU_HP | PU_SPELLS);

		/* Fully update the visuals */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Redraw everything */
		player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN |
								   PR_EQUIP | PR_MESSAGE | PR_MONSTER |
								   PR_OBJECT | PR_MONLIST | PR_ITEMLIST);
	}

	/* Clear screen */
	Term_clear();

	if (character_dungeon) {
		/* Hack -- update */
		handle_stuff(player);

		/* Place the cursor on the player */
		if ((0 != character_dungeon) && OPT(player, show_target) &&
			target_sighted()) {
			struct loc target;
			target_get(&target);
			move_cursor_relative(target.y, target.x);
		} else {
			move_cursor_relative(player->grid.y, player->grid.x);
		}
	}

	/* Redraw every window */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		if (!angband_term[j]) continue;

		Term_activate(angband_term[j]);
		Term_redraw();
		Term_fresh();
		Term_activate(old);
	}
}



/**
 * Display the options and redraw afterward.
 */
void do_cmd_xxx_options(void)
{
	do_cmd_options();
	do_cmd_redraw();
}


/**
 * Invoked when the command isn't recognised.
 */
void do_cmd_unknown(void)
{
	prt("Type '?' for help.", 0, 0);
}


/**
 * Print the version and copyright notice.
 */
void do_cmd_version(void)
{
	char header_buf[120];

	textblock *tb = textblock_new();
	region local_area = { 0, 0, 0, 0 };

	my_strcpy(header_buf,
			  format("You are playing %s.  Type '?' for more info.", buildver),
			  sizeof(header_buf));
	textblock_append(tb, "\n");
	textblock_append(tb, copyright);
	textui_textblock_show(tb, local_area, header_buf);
	textblock_free(tb);
}

/**
 * Verify use of "debug" mode
 */
void textui_cmd_debug(void)
{
	/* Ask first time */
	if (!(player->noscore & NOSCORE_DEBUG)) {
		/* Mention effects */
		msg("You are about to use the dangerous, unsupported, debug commands!");
		msg("Your machine may crash, and your savefile may become corrupted!");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Verify request */
		if (!get_check("Are you sure you want to use the debug commands? "))
			return;

		/* Mark savefile */
		player->noscore |= NOSCORE_DEBUG;
	}

	/* Okay */
	get_debug_command();
}

/**
 * Verify the suicide command
 */
void textui_cmd_suicide(void)
{
	/* Flush input */
	event_signal(EVENT_INPUT_FLUSH);

	/* Verify */
	if (player->total_winner) {
		if (!get_check("Do you want to retire? "))
			return;
	} else {
		struct keypress ch;

		if (!get_check("Do you really want to kill this character? "))
			return;

		/* Special Verification for suicide */
		prt("Please verify KILLING THIS CHARACTER by typing the '@' sign: ", 0, 0);
		event_signal(EVENT_INPUT_FLUSH);
		ch = inkey();
		prt("", 0, 0);
		if (ch.code != '@') return;
	}

	cmdq_push(CMD_SUICIDE);
}

/**
 * Get input for the rest command
 */
void textui_cmd_rest(void)
{
	const char *p = "Rest (0-9999, '!' for HP or SP, '*' for HP and SP, '&' as needed): ";

	char out_val[5] = "& ";

	/* Ask for duration */
	if (!get_string(p, out_val, sizeof(out_val))) return;

	/* Rest... */
	if (out_val[0] == '&') {
		/* ...until done */
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", REST_COMPLETE);
	} else if (out_val[0] == '*') {
		/* ...a lot */
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", REST_ALL_POINTS);
	} else if (out_val[0] == '!') {
		/* ...until HP or SP filled */
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", REST_SOME_POINTS);
	} else {
		/* ...some */
		int turns = atoi(out_val);
		if (turns <= 0) return;
		if (turns > 9999) turns = 9999;

		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", turns);
	}
}


/**
 * Quit the game.
 */
void textui_quit(void)
{
	player->upkeep->playing = false;
}


/**
 * ------------------------------------------------------------------------
 * Screenshot loading/saving code
 * ------------------------------------------------------------------------ */

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


static void screenshot_term_query(int wid, int hgt, int x, int y, int *a, wchar_t *c)
{
	if (y < ROW_MAP || y == hgt - 1 || x < COL_MAP) {
		/* Record everything outside the map. */
		(void) Term_what(x, y, a, c);
	} else {
		/*
		 * In the map, skip over the padding for scaled up tiles.  As
		 * necessary, pad trailing columns and rows with blanks.
		 */
		int srcx = (x - COL_MAP) * tile_width + COL_MAP;
		int srcy = (y - ROW_MAP) * tile_height + ROW_MAP;

		if (srcx < wid && srcy < hgt - 1) {
			(void) Term_what(srcx, srcy, a, c);
		} else {
			*a = Term->attr_blank;
			*c = Term->char_blank;
		}
	}
}


/**
 * Take an html screenshot
 */
void html_screenshot(const char *path, int mode)
{
	int y, x;
	int wid, hgt;

	int a = COLOUR_WHITE;
	int oa = COLOUR_WHITE;
	int fg_colour = COLOUR_WHITE;
	int bg_colour = COLOUR_DARK;
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
	if (!fp) {
		plog_fmt("Cannot write the '%s' file!", path);
		return;
	}

	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	if (mode == 0) {
		file_putf(fp, "<!DOCTYPE html><html><head>\n");
		file_putf(fp, "  <meta='generator' content='%s'>\n", buildid);
		file_putf(fp, "  <title>%s</title>\n", path);
		file_putf(fp, "</head>\n\n");
		file_putf(fp, "<body style='color: #fff; background: #000;'>\n");
		file_putf(fp, "<pre>\n");
	} else {
		file_putf(fp, "[CODE][TT][BC=black][COLOR=white]\n");
	}

	/* Dump the screen */
	for (y = 0; y < hgt; y++) {
		for (x = 0; x < wid; x++) {
			/* Get the attr/char */
			screenshot_term_query(wid, hgt, x, y, &a, &c);

			/* Set the foreground and background */
			fg_colour = a % MAX_COLORS;
			switch (a / MAX_COLORS)
			{
				case BG_BLACK:
					bg_colour = COLOUR_DARK;
					break;
				case BG_SAME:
					bg_colour = fg_colour;
					break;
				case BG_DARK:
					bg_colour = COLOUR_SHADE;
					break;
				default:
				assert((a >= BG_BLACK) && (a < BG_MAX * MAX_COLORS));
			}

			/* Color change */
			if (oa != a) {
				if (oa == COLOUR_WHITE) {
					/* From the default white to another color */
					file_putf(fp, new_color_fmt,
							  angband_color_table[fg_colour][1],
							  angband_color_table[fg_colour][2],
							  angband_color_table[fg_colour][3],
							  angband_color_table[bg_colour][1],
							  angband_color_table[bg_colour][2],
							  angband_color_table[bg_colour][3]);
				} else if (fg_colour == COLOUR_WHITE &&
						   bg_colour == COLOUR_DARK) {
					/* From another color to the default white */
					file_putf(fp, close_color_fmt);
				} else {
					/* Change colors */
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
			else {
				char mbseq[MB_LEN_MAX+1] = {0};
				wctomb(mbseq, c);
				file_putf(fp, "%s", mbseq);
			}
		}

		/* End the row */
		file_putf(fp, "\n");
	}

	/* Close the last font-color tag if necessary */
	if (oa != COLOUR_WHITE) file_putf(fp, close_color_fmt);

	if (mode == 0) {
		file_putf(fp, "</pre>\n");
		file_putf(fp, "</body>\n");
		file_putf(fp, "</html>\n");
	} else {
		file_putf(fp, "[/COLOR][/BC][/TT][/CODE]\n");
	}

	/* Close it */
	file_close(fp);
}



/**
 * Hack -- save a screen dump to a file in html format
 */
static void do_cmd_save_screen_html(int mode)
{
	size_t i;

	ang_file *fff;
	char file_name[1024];
	char tmp_val[256];

	typedef void (*dump_func)(ang_file *);
	dump_func dump_visuals [] = { dump_monsters, dump_features, dump_objects,
								  dump_flavors, dump_colors };

	/* Ask for a file */
	if (!get_file(mode == 0 ? "dump.html" : "dump.txt",
				  tmp_val, sizeof(tmp_val))) return;

	/* Save current preferences */
	path_build(file_name, sizeof(file_name), ANGBAND_DIR_USER, "dump.prf");
	fff = file_open(file_name, MODE_WRITE,
					(mode == 0 ? FTYPE_HTML : FTYPE_TEXT));

	/* Check for failure */
	if (!fff) {
		msg("Screen dump failed.");
		event_signal(EVENT_MESSAGE_FLUSH);
		return;
	}

	/* Dump all the visuals */
	for (i = 0; i < N_ELEMENTS(dump_visuals); i++)
		dump_visuals[i](fff);

	file_close(fff);

	/* Dump the screen with raw character attributes */
	reset_visuals(false);
	do_cmd_redraw();
	html_screenshot(tmp_val, mode);

	/* Recover current graphics settings */
	reset_visuals(true);
	process_pref_file(file_name, true, false);
	file_delete(file_name);
	do_cmd_redraw();

	msg("%s screen dump saved.", mode ? "Forum text" : "HTML");
	event_signal(EVENT_MESSAGE_FLUSH);
}


/**
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
	char ch;
	ch = get_char("Dump as (H)TML or (F)orum text? ", "hf", 2, ' ');

	switch (ch)
	{
		case 'h': do_cmd_save_screen_html(0); break;
		case 'f': do_cmd_save_screen_html(1); break;
	}
}
