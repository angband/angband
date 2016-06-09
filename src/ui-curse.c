/**
 * \file ui-curse.c
 * \brief Curse selection menu
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2016 Nick McConnell
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
#include "obj-knowledge.h"
#include "object.h"
#include "ui-menu.h"
#include "ui-output.h"

static struct curse *selection;

/**
 * Display an entry on the item menu
 */
void get_curse_display(struct menu *menu, int oid, bool cursor, int row,
					  int col, int width)
{
	struct curse **choice = menu_priv(menu);
	int attr = cursor ? COLOUR_L_BLUE : COLOUR_WHITE;
	char buf[80];
	int power = choice[oid]->power;
	strnfmt(buf, sizeof(buf), "%s (power %d)", choice[oid]->name, power);
	c_put_str(attr, buf, row + oid, col);
}

/**
 * Deal with events on the get_item menu
 */
bool get_curse_action(struct menu *menu, const ui_event *event, int oid)
{
	struct curse **choice = menu_priv(menu);
	if (event->type == EVT_SELECT) {
		selection = choice[oid];
	}

	return false;
}

/**
 * Show spell long description when browsing
 */
static void curse_menu_browser(int oid, void *data, const region *loc)
{
	struct curse **choice = data;

	/* Redirect output to the screen */
	text_out_hook = text_out_to_screen;
	text_out_wrap = 0;
	text_out_indent = loc->col - 1;
	text_out_pad = 1;

	Term_gotoxy(loc->col, loc->row + loc->page_rows);
	text_out("\n%s\n", choice[oid]->desc);

	/* XXX */
	text_out_pad = 0;
	text_out_indent = 0;
}

/**
 * Display list of curses to choose from
 */
struct curse *curse_menu(struct object *obj)
{
	menu_iter menu_f = { 0, 0, get_curse_display, get_curse_action, 0 };
	struct menu *m = menu_new(MN_SKIN_SCROLL, &menu_f);
	int row;
	unsigned int length = 0;
	int count = 0;
	struct curse *curse = obj->curses;
	struct curse **available;
	static region area = { 20, 1, -1, -2 };

	/* Count and then list the curses */
	while (curse) {
		count++;
		curse = curse->next;
	}
	if (!count) {
		return NULL;
	}
	available = mem_zalloc(count * sizeof(struct curse *));
	count = 0;
	for (curse = obj->curses; curse; curse = curse->next) {
		available[count++] = curse;
		length = MAX(length, strlen(curse->name));
	}

	/* Set up the menu */
	menu_setpriv(m, count, available);
	m->header = "Remove which curse?";
	m->selections = lower_case;
	m->flags = (MN_PVT_TAGS);
	m->browse_hook = curse_menu_browser;

	/* Set up the item list variables */
	selection = NULL;

	/* Set up the menu region */
	area.page_rows = m->count + 1;
	area.row = 1;
	area.col = (Term->wid - 1 - length) / 2;
	if (area.col <= 3)
		area.col = 0;
	area.width = MAX(length + 1, strlen(m->header));

	for (row = area.row; row < area.row + area.page_rows; row++)
		prt("", row, MAX(0, area.col - 1));

	menu_layout(m, &area);

	/* Choose */
	menu_select(m, 0, true);

	/* Clean up */
	mem_free(m);

	/* Result */
	return selection;
}

bool textui_get_curse(struct curse **choice, struct object *obj)
{
	struct curse *curse = curse_menu(obj);
	if (curse != NULL) {
		*choice = curse;
		return true;
	}
	return false;
}
