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

/* =================== GEOMETRY ================= */

region region_calculate(region loc)
{
	int w, h;
	Term_get_size(&w, &h);

	if (loc.col < 0)
		loc.col += w;
	if (loc.row < 0)
		loc.row += w;
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

bool region_inside(const region *loc, const ui_event_data *key)
{
	if ((loc->col > key->mousex) || (loc->col + loc->width <= key->mousex))
		return FALSE;

	if ((loc->row > key->mousey) || (loc->row + loc->page_rows <= key->mousey))
		return FALSE;

	return TRUE;
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

