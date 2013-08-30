/*
 * File: ui.c
 * Purpose: Generic ui functions
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
#include "z-textblock.h"


/*** Regions ***/

/*
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
		return FALSE;

	if ((loc->row > key->mouse.y) || (loc->row + loc->page_rows <= key->mouse.y))
		return FALSE;

	return TRUE;
}


/*** Text display ***/

/*
 * These functions are designed to display large blocks of text on the screen
 * all at once.  They are the z-term specific layer on top of the z-textblock.c
 * functions.
 */

/* Utility function */
static void display_area(const wchar_t *text, const byte *attrs,
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

/*
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
		c_prt(TERM_L_BLUE, header, area.row, area.col);
		area.row++;
	}

	if (n_lines > (size_t) area.page_rows)
		n_lines = area.page_rows;

	display_area(textblock_text(tb), textblock_attrs(tb), line_starts,
	             line_lengths, n_lines, area, 0);

	mem_free(line_starts);
	mem_free(line_lengths);
}

/*
 * Show a textblock interactively
 */
void textui_textblock_show(textblock *tb, region orig_area, const char *header)
{
	/* xxx on resize this should be recalculated */
	region area = region_calculate(orig_area);

	size_t *line_starts = NULL, *line_lengths = NULL;
	size_t n_lines;

	n_lines = textblock_calculate_lines(tb,
			&line_starts, &line_lengths, area.width);

	screen_save();

	/* make room for the footer */
	area.page_rows -= 2;

	if (header != NULL) {
		area.page_rows--;
		c_prt(TERM_L_BLUE, header, area.row, area.col);
		area.row++;
	}

	if (n_lines > (size_t) area.page_rows) {
		int start_line = 0;

		c_prt(TERM_WHITE, "", area.row + area.page_rows, area.col);
		c_prt(TERM_L_BLUE, "(Up/down or ESCAPE to exit.)",
				area.row + area.page_rows + 1, area.col);

		/* Pager mode */
		while (1) {
			struct keypress ch;

			display_area(textblock_text(tb), textblock_attrs(tb), line_starts,
					line_lengths, n_lines, area, start_line);

			ch = inkey();
			if (ch.code == ARROW_UP)
				start_line--;
			else if (ch.code== ESCAPE || ch.code == 'q')
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

		c_prt(TERM_WHITE, "", area.row + n_lines, area.col);
		c_prt(TERM_L_BLUE, "(Press any key to continue.)",
				area.row + n_lines + 1, area.col);
		inkey();
	}

	mem_free(line_starts);
	mem_free(line_lengths);

	screen_load();

	return;
}


/** Simple text display **/

/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(byte attr, const char *str, int row, int col) {
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, str);
}


/* As above, but in white */
void put_str(const char *str, int row, int col) {
	c_put_str(TERM_WHITE, str, row, col);
}

/*
 * Display a string on the screen using an attribute, and clear to the end of the line.
 */
void c_prt(byte attr, const char *str, int row, int col) {
	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}

/* As above, but in white */
void prt(const char *str, int row, int col) {
	c_prt(TERM_WHITE, str, row, col);
}



/** Screen loading/saving **/

/*
 * Screen loading and saving can be done to an arbitrary depth but it's
 * important that every call to screen_save() is balanced by a call to
 * screen_load() later on.  'character_icky' is used by the game to keep
 * track of whether it should try to update the map and sidebar or not,
 * so if you miss out a screen_load you will not get proper game updates.
 *
 * Term_save() / Term_load() do all the heavy lifting here.
 */

/* Depth of the screen_save() stack */
s16b character_icky;

/*
 * Save the screen, and increase the "icky" depth.
 */
void screen_save(void)
{
	message_flush();
	Term_save();
	character_icky++;
}

/*
 * Load the screen, and decrease the "icky" depth.
 */
void screen_load(void)
{
	message_flush();
	Term_load();
	character_icky--;

	/* Mega hack - redraw big graphics - sorry NRM */
	if (character_icky == 0 && (tile_width > 1 || tile_height > 1))
		Term_redraw();
}



/*** Miscellaneous things ***/

/*
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

	Term_putch(origin_x, origin_y, TERM_WHITE, '+');
	Term_putch(end_x, origin_y, TERM_WHITE, '+');
	Term_putch(origin_x, end_y, TERM_WHITE, '+');
	Term_putch(end_x, end_y, TERM_WHITE, '+');

	for (n = 1; n < (end_x - origin_x); n++)
	{
		Term_putch(origin_x + n, origin_y, TERM_WHITE, '-');
		Term_putch(origin_x + n, end_y, TERM_WHITE, '-');
	}

	for (n = 1; n < (end_y - origin_y); n++)
	{
		Term_putch(origin_x, origin_y + n, TERM_WHITE, '|');
		Term_putch(end_x, origin_y + n, TERM_WHITE, '|');
	}
}

