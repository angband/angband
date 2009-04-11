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

void region_erase(const region *loc)
{
	int i = 0;
	int w = loc->width;
	int h = loc->page_rows;

	if (loc->width <= 0 || loc->page_rows <= 0)
	{
		Term_get_size(&w, &h);
		if (loc->width <= 0) w -= loc->width;
		if (loc->page_rows <= 0) h -= loc->page_rows;
	}

	for (i = 0; i < h; i++)
		Term_erase(loc->col, loc->row + i, w);
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

