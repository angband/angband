/**
 * \file ui-output.c
 * \brief Putting text on the screen, screen saving and loading, panel handling
 *
 * Copyright (c) 2007 Pete Mack and others.
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
#include "player-calcs.h"
#include "ui-input.h"
#include "ui-output.h"
#include "z-textblock.h"

int16_t screen_save_depth;

/**
 * ------------------------------------------------------------------------
 * Regions
 * ------------------------------------------------------------------------ */

/**
 * These functions are used for manipulating regions on the screen, used 
 * mostly (but not exclusively) by the menu functions.
 */

region region_calculate(region loc)
{
	int w, h;
	Term_get_size(&w, &h);

	if (loc.col < 0)
		loc.col += w;
	if (loc.row < 0)
		loc.row += h;
	if (loc.width <= 0)
		loc.width += w - loc.col;
	if (loc.page_rows <= 0)
		loc.page_rows += h - loc.row;

	return loc;
}

void region_erase_bordered(const region *loc)
{
	region calc = region_calculate(*loc);
	int i = 0;

	calc.col = MAX(calc.col - 1, 0);
	calc.row = MAX(calc.row - 1, 0);
	calc.width += 2;
	calc.page_rows += 2;

	for (i = 0; i < calc.page_rows; i++)
		Term_erase(calc.col, calc.row + i, calc.width);
}

void region_erase(const region *loc)
{
	region calc = region_calculate(*loc);
	int i = 0;

	for (i = 0; i < calc.page_rows; i++)
		Term_erase(calc.col, calc.row + i, calc.width);
}

bool region_inside(const region *loc, const ui_event *key)
{
	if ((loc->col > key->mouse.x) || (loc->col + loc->width <= key->mouse.x))
		return false;

	if ((loc->row > key->mouse.y) ||
		(loc->row + loc->page_rows <= key->mouse.y))
		return false;

	return true;
}


/**
 * ------------------------------------------------------------------------
 * Text display
 * ------------------------------------------------------------------------ */

/**
 * These functions are designed to display large blocks of text on the screen
 * all at once.  They are the ui-term specific layer on top of the z-textblock.c
 * functions.
 */

/**
 * Utility function
 */
static void display_area(const wchar_t *text, const uint8_t *attrs,
		size_t *line_starts, size_t *line_lengths,
		size_t n_lines,
		region area, size_t line_from)
{
	size_t i, j;

	n_lines = MIN(n_lines, (size_t) area.page_rows);

	for (i = 0; i < n_lines; i++) {
		Term_erase(area.col, area.row + i, area.width);
		for (j = 0; j < line_lengths[line_from + i]; j++) {
			Term_putch(area.col + j, area.row + i,
					attrs[line_starts[line_from + i] + j],
					text[line_starts[line_from + i] + j]);
		}
	}
}

/**
 * Plonk a textblock on the screen in a certain bounding box.
 */
void textui_textblock_place(textblock *tb, region orig_area, const char *header)
{
	/* xxx on resize this should be recalculated */
	region area = region_calculate(orig_area);

	size_t *line_starts = NULL, *line_lengths = NULL;
	size_t n_lines;

	n_lines = textblock_calculate_lines(tb,
			&line_starts, &line_lengths, area.width);

	if (header != NULL) {
		area.page_rows--;
		c_prt(COLOUR_L_BLUE, header, area.row, area.col);
		area.row++;
	}

	if (n_lines > (size_t) area.page_rows)
		n_lines = area.page_rows;

	display_area(textblock_text(tb), textblock_attrs(tb), line_starts,
	             line_lengths, n_lines, area, 0);

	mem_free(line_starts);
	mem_free(line_lengths);
}

/**
 * Show a textblock interactively
 */
struct keypress textui_textblock_show(textblock *tb, region orig_area, const char *header)
{
	/* xxx on resize this should be recalculated */
	region area = region_calculate(orig_area);

	size_t *line_starts = NULL, *line_lengths = NULL;
	size_t n_lines;

	struct keypress ch = KEYPRESS_NULL;

	n_lines = textblock_calculate_lines(tb,
			&line_starts, &line_lengths, area.width);

	screen_save();

	/* make room for the footer */
	area.page_rows -= 2;

	if (header != NULL) {
		area.page_rows--;
		c_prt(COLOUR_L_BLUE, header, area.row, area.col);
		area.row++;
	}

	if (n_lines > (size_t) area.page_rows) {
		int start_line = 0;

		c_prt(COLOUR_WHITE, "", area.row + area.page_rows, area.col);
		c_prt(COLOUR_L_BLUE, "(Up/down or ESCAPE to exit.)",
				area.row + area.page_rows + 1, area.col);

		/* Pager mode */
		while (1) {			

			display_area(textblock_text(tb), textblock_attrs(tb), line_starts,
					line_lengths, n_lines, area, start_line);

			ch = inkey();
			if (ch.code == ARROW_UP)
				start_line--;
			else if (ch.code == ESCAPE || ch.code == 'q' || ch.code == 'x')
				break;
			else if (ch.code == ']' || ch.code == '[')
				/* Special case to deal with monster and object lists -
				 * see bug #2120 */
				break;
			else if (ch.code == ARROW_DOWN)
				start_line++;
			else if (ch.code == ' ')
				start_line += area.page_rows;

			if (start_line < 0)
				start_line = 0;
			if (start_line + (size_t) area.page_rows > n_lines)
				start_line = n_lines - area.page_rows;
		}
	} else {
		display_area(textblock_text(tb), textblock_attrs(tb), line_starts,
				line_lengths, n_lines, area, 0);

		c_prt(COLOUR_WHITE, "", area.row + n_lines, area.col);
		c_prt(COLOUR_L_BLUE, "(Press any key to continue.)",
				area.row + n_lines + 1, area.col);
		ch = inkey();
	}

	mem_free(line_starts);
	mem_free(line_lengths);

	screen_load();

	return (ch);
}


/**
 * ------------------------------------------------------------------------
 * text_out hook for screen display
 * ------------------------------------------------------------------------ */

/**
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line.  Also, allow "newline" to force
 * a "wrap" to the next line.  Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "text_out()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void text_out_to_screen(uint8_t a, const char *str)
{
	int x, y;

	int wid, h;

	int wrap;

	const wchar_t *s;
	wchar_t buf[1024];

	/* Obtain the size */
	(void)Term_get_size(&wid, &h);

	/* Obtain the cursor */
	(void)Term_locate(&x, &y);

	/* Copy to a rewriteable string */
	text_mbstowcs(buf, str, 1024);
	
	/* Use special wrapping boundary? */
	if ((text_out_wrap > 0) && (text_out_wrap < wid))
		wrap = text_out_wrap;
	else
		wrap = wid;

	/* Process the string */
	for (s = buf; *s; s++) {
		wchar_t ch;

		/* Force wrap */
		if (*s == L'\n') {
			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			x += text_out_pad;
			Term_gotoxy(x, y);

			continue;
		}

		/* Clean up the char */
		ch = (text_iswprint(*s) ? *s : L' ');

		/* Wrap words as needed */
		if ((x >= wrap - 1) && (ch != L' ')) {
			int i, n = 0;

			int av[256];
			wchar_t cv[256];

			/* Wrap word */
			if (x < wrap) {
				/* Scan existing text */
				for (i = wrap - 2; i >= 0; i--) {
					/* Grab existing attr/char */
					Term_what(i, y, &av[i], &cv[i]);

					/* Break on space */
					if (cv[i] == L' ') break;

					/* Track current word */
					n = i;
				}
			}

			/* Special case */
			if (n == 0) n = wrap;

			/* Clear line */
			Term_erase(n, y, 255);

			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			x += text_out_pad;
			Term_gotoxy(x, y);

			/* Wrap the word (if any) */
			for (i = n; i < wrap - 1; i++) {
				/* Dump */
				Term_addch(av[i], cv[i]);

				/* Advance (no wrap) */
				if (++x > wrap) x = wrap;
			}
		}

		/* Dump */
		Term_addch(a, ch);

		/* Advance */
		if (++x > wrap) x = wrap;
	}
}


/**
 * ------------------------------------------------------------------------
 * Simple text display
 * ------------------------------------------------------------------------ */

/**
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(uint8_t attr, const char *str, int row, int col) {
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, str);
}


/**
 * As above, but in white
 */
void put_str(const char *str, int row, int col) {
	c_put_str(COLOUR_WHITE, str, row, col);
}

/**
 * Display a string on the screen using an attribute, and clear to the
 * end of the line.
 */
void c_prt(uint8_t attr, const char *str, int row, int col) {
	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}

/**
 * As above, but in white
 */
void prt(const char *str, int row, int col) {
	c_prt(COLOUR_WHITE, str, row, col);
}



/**
 * ------------------------------------------------------------------------
 * Screen loading/saving
 * ------------------------------------------------------------------------ */

/**
 * Screen loading and saving can be done to an arbitrary depth but it's
 * important that every call to screen_save() is balanced by a call to
 * screen_load() later on.  'screen_save_depth' is used by the game to keep
 * track of whether it should try to update the map and sidebar or not,
 * so if you miss out a screen_load you will not get proper game updates.
 *
 * Term_save() / Term_load() do all the heavy lifting here.
 */

/**
 * Depth of the screen_save() stack
 */
int16_t screen_save_depth;

/**
 * Save the screen, and increase the "icky" depth.
 */
void screen_save(void)
{
	player->upkeep->redraw |= PR_MAP;
	redraw_stuff(player);
	event_signal(EVENT_MESSAGE_FLUSH);
	Term_save();
	screen_save_depth++;
}

/**
 * Load the screen, and decrease the "icky" depth.
 */
void screen_load(void)
{
	event_signal(EVENT_MESSAGE_FLUSH);
	Term_load();
	screen_save_depth--;
}

bool textui_map_is_visible(void)
{
	return (screen_save_depth == 0);
}

/**
 * ------------------------------------------------------------------------
 * Miscellaneous things
 * ------------------------------------------------------------------------ */

/**
 * A Hengband-like 'window' function, that draws a surround box in ASCII art.
 */
void window_make(int origin_x, int origin_y, int end_x, int end_y)
{
	int n;
	region to_clear;

	to_clear.col = origin_x;
	to_clear.row = origin_y;
	to_clear.width = end_x - origin_x;
	to_clear.page_rows = end_y - origin_y;

	region_erase(&to_clear);

	Term_putch(origin_x, origin_y, COLOUR_WHITE, L'+');
	Term_putch(end_x, origin_y, COLOUR_WHITE, L'+');
	Term_putch(origin_x, end_y, COLOUR_WHITE, L'+');
	Term_putch(end_x, end_y, COLOUR_WHITE, L'+');

	for (n = 1; n < (end_x - origin_x); n++) {
		Term_putch(origin_x + n, origin_y, COLOUR_WHITE, L'-');
		Term_putch(origin_x + n, end_y, COLOUR_WHITE, L'-');
	}

	for (n = 1; n < (end_y - origin_y); n++) {
		Term_putch(origin_x, origin_y + n, COLOUR_WHITE, L'|');
		Term_putch(end_x, origin_y + n, COLOUR_WHITE, L'|');
	}
}

bool panel_should_modify(term *t, int wy, int wx)
{
	int dungeon_hgt = cave->height;
	int dungeon_wid = cave->width;
	int screen_hgt = (t == angband_term[0]) ?
		SCREEN_HGT : t->hgt / tile_height;
	int screen_wid = (t == angband_term[0]) ?
		SCREEN_WID : t->wid / tile_width;

	/* Verify wy, adjust if needed */
	if (wy > dungeon_hgt - screen_hgt) wy = dungeon_hgt - screen_hgt;
	if (wy < 0) wy = 0;

	/* Verify wx, adjust if needed */
	if (wx > dungeon_wid - screen_wid) wx = dungeon_wid - screen_wid;
	if (wx < 0) wx = 0;

	/* Needs changes? */
	return ((t->offset_y != wy) || (t->offset_x != wx));
}

/**
 * Modify the current panel to the given coordinates, adjusting only to
 * ensure the coordinates are legal, and return true if anything done.
 *
 * The town should never be scrolled around.
 *
 * Note that monsters are no longer affected in any way by panel changes.
 *
 * As a total hack, whenever the current panel changes, we assume that
 * the "overhead view" window should be updated.
 */
bool modify_panel(term *t, int wy, int wx)
{
	int dungeon_hgt = cave->height;
	int dungeon_wid = cave->width;
	int screen_hgt = (t == angband_term[0]) ?
		SCREEN_HGT : t->hgt / tile_height;
	int screen_wid = (t == angband_term[0]) ?
		SCREEN_WID : t->wid / tile_width;

	/* Verify wy, adjust if needed */
	if (wy > dungeon_hgt - screen_hgt) wy = dungeon_hgt - screen_hgt;
	if (wy < 0) wy = 0;

	/* Verify wx, adjust if needed */
	if (wx > dungeon_wid - screen_wid) wx = dungeon_wid - screen_wid;
	if (wx < 0) wx = 0;

	/* React to changes */
	if (panel_should_modify(t, wy, wx)) {
		/* Save wy, wx */
		t->offset_y = wy;
		t->offset_x = wx;

		/* Redraw map */
		player->upkeep->redraw |= (PR_MAP);

		/* Changed */
		return (true);
	}

	/* No change */
	return (false);
}

static void verify_panel_int(bool centered)
{
	int wy, wx;
	int screen_hgt, screen_wid;

	int panel_wid, panel_hgt;

	int py = player->grid.y;
	int px = player->grid.x;

	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(window_flag[j] & (PW_OVERHEAD))) continue;

		wy = t->offset_y;
		wx = t->offset_x;

		screen_hgt = (j == 0) ? SCREEN_HGT : t->hgt / tile_height;
		screen_wid = (j == 0) ? SCREEN_WID : t->wid / tile_width;

		panel_wid = screen_wid / 2;
		panel_hgt = screen_hgt / 2;


		/* Scroll screen vertically when off-center */
		if (centered && !player->upkeep->running && (py != wy + panel_hgt))
			wy = py - panel_hgt;

		/* Scroll screen vertically when 3 grids from top/bottom edge */
		else if ((py < wy + 3) || (py >= wy + screen_hgt - 3))
			wy = py - panel_hgt;


		/* Scroll screen horizontally when off-center */
		if (centered && !player->upkeep->running && (px != wx + panel_wid))
			wx = px - panel_wid;

		/* Scroll screen horizontally when 3 grids from left/right edge */
		else if ((px < wx + 3) || (px >= wx + screen_wid - 3))
			wx = px - panel_wid;


		/* Scroll if needed */
		modify_panel(t, wy, wx);
	}
}

/**
 * Change the current panel to the panel lying in the given direction.
 *
 * Return true if the panel was changed.
 */
bool change_panel(int dir)
{
	bool changed = false;
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		int screen_hgt, screen_wid;
		int wx, wy;

		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(window_flag[j] & PW_OVERHEAD)) continue;

		screen_hgt = (j == 0) ? SCREEN_HGT : t->hgt / tile_height;
		screen_wid = (j == 0) ? SCREEN_WID : t->wid / tile_width;

		/* Shift by half a panel */
		wy = t->offset_y + ddy[dir] * screen_hgt / 2;
		wx = t->offset_x + ddx[dir] * screen_wid / 2;

		/* Use "modify_panel" */
		if (modify_panel(t, wy, wx)) changed = true;
	}

	return (changed);
}


/**
 * Verify the current panel (relative to the player location).
 *
 * By default, when the player gets "too close" to the edge of the current
 * panel, the map scrolls one panel in that direction so that the player
 * is no longer so close to the edge.
 *
 * The "OPT(player, center_player)" option allows the current panel to always be
 * centered around the player, which is very expensive, and also has some
 * interesting gameplay ramifications.
 */
void verify_panel(void)
{
	verify_panel_int(OPT(player, center_player));
}

void center_panel(void)
{
	verify_panel_int(true);
}

void textui_get_panel(int *min_y, int *min_x, int *max_y, int *max_x)
{
	term *t = term_screen;

	if (!t) return;

	*min_y = t->offset_y;
	*min_x = t->offset_x;
	*max_y = t->offset_y + SCREEN_HGT;
	*max_x = t->offset_x + SCREEN_WID;
}

bool textui_panel_contains(unsigned int y, unsigned int x)
{
	unsigned int hgt;
	unsigned int wid;
	if (!Term)
		return true;
	if (Term == term_screen) {
		hgt = SCREEN_HGT;
		wid = SCREEN_WID;
	} else {
		hgt = Term->hgt / tile_height;
		wid = Term->wid / tile_width;
	}
	return (y - Term->offset_y) < hgt && (x - Term->offset_x) < wid;
}
