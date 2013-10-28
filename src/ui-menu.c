/*
 * File: ui-menu.c
 * Purpose: Generic menu interaction functions
 *
 * Copyright (c) 2007 Pete Mack
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
#include "ui-event.h"
#include "ui-menu.h"

/* Cursor colours */
const byte curs_attrs[2][2] =
{
	{ TERM_SLATE, TERM_BLUE },      /* Greyed row */
	{ TERM_WHITE, TERM_L_BLUE }     /* Valid row */
};

/* Some useful constants */
const char lower_case[] = "abcdefghijklmnopqrstuvwxyz";
const char upper_case[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char all_letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* forward declarations */
static void display_menu_row(menu_type *menu, int pos, int top,
			     bool cursor, int row, int col, int width);
static bool menu_calc_size(menu_type *menu);
static bool is_valid_row(menu_type *menu, int cursor);


/* Display an event, with possible preference overrides */
static void display_action_aux(menu_action *act, byte color, int row, int col, int wid)
{
	/* TODO: add preference support */
	/* TODO: wizard mode should show more data */
	Term_erase(col, row, wid);

	if (act->name)
		Term_putstr(col, row, wid, color, act->name);
}

/* ------------------------------------------------------------------------
 * MN_ACTIONS HELPER FUNCTIONS
 *
 * MN_ACTIONS is the type of menu iterator that displays a simple list of
 * menu_actions.
 * ------------------------------------------------------------------------ */

static char menu_action_tag(menu_type *m, int oid)
{
	menu_action *acts = menu_priv(m);
	return acts[oid].tag;
}

static int menu_action_valid(menu_type *m, int oid)
{
	menu_action *acts = menu_priv(m);

	if (acts[oid].flags & MN_ACT_HIDDEN)
		return 2;

	return acts[oid].name ? TRUE : FALSE;
}

static void menu_action_display(menu_type *m, int oid, bool cursor, int row, int col, int width)
{
	menu_action *acts = menu_priv(m);
	byte color = curs_attrs[!(acts[oid].flags & (MN_ACT_GRAYED))][0 != cursor];

	display_action_aux(&acts[oid], color, row, col, width);
}

static bool menu_action_handle(menu_type *m, const ui_event *event, int oid)
{
	menu_action *acts = menu_priv(m);

	if (event->type == EVT_SELECT)
	{
		if (!(acts->flags & MN_ACT_GRAYED) && acts[oid].action)
		{
			acts[oid].action(acts[oid].name, m->cursor);
			return TRUE;
		}
	}

	return FALSE;
}


/* Virtual function table for action_events */
static const menu_iter menu_iter_actions =
{
	menu_action_tag,
	menu_action_valid,
	menu_action_display,
	menu_action_handle,
	NULL
};


/* ------------------------------------------------------------------------
 * MN_STRINGS HELPER FUNCTIONS
 *
 * MN_STRINGS is the type of menu iterator that displays a simple list of 
 * strings - no action is associated, as selection will just return the index.
 * ------------------------------------------------------------------------ */
static void display_string(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	const char **items = menu_priv(m);
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];
	Term_putstr(col, row, width, color, items[oid]);
}

/* Virtual function table for displaying arrays of strings */
const menu_iter menu_iter_strings =
{ 
	NULL,              /* get_tag() */
	NULL,              /* valid_row() */
	display_string,    /* display_row() */
	NULL, 	           /* row_handler() */
	NULL
};




/* ================== SKINS ============== */


/* Scrolling menu */
/* Find the position of a cursor given a screen address */
static int scrolling_get_cursor(int row, int col, int n, int top, region *loc)
{
	int cursor = row - loc->row + top;
	if (cursor >= n) cursor = n - 1;

	return cursor;
}


/* Display current view of a skin */
static void display_scrolling(menu_type *menu, int cursor, int *top, region *loc)
{
	int col = loc->col;
	int row = loc->row;
	int rows_per_page = loc->page_rows;
	int n = menu->filter_list ? menu->filter_count : menu->count;
	int i;

	/* Keep a certain distance from the top when possible */
	if ((cursor <= *top) && (*top > 0))
		*top = cursor - 1;

	/* Keep a certain distance from the bottom when possible */
	if (cursor >= *top + (rows_per_page - 1))
		*top = cursor - (rows_per_page - 1) + 1;

	/* Limit the top to legal places */
	*top = MIN(*top, n - rows_per_page);
	*top = MAX(*top, 0);

	for (i = 0; i < rows_per_page; i++)
	{
		/* Blank all lines */
		Term_erase(col, row + i, loc->width);
		if (i < n)
		{
			/* Redraw the line if it's within the number of menu items */
			bool is_curs = (i == cursor - *top);
			display_menu_row(menu, i + *top, *top, is_curs, row + i, col,
							loc->width);
		}
	}

	if (menu->cursor >= 0)
		Term_gotoxy(col, row + cursor - *top);
}

static char scroll_get_tag(menu_type *menu, int pos)
{
	if (menu->selections)
		return menu->selections[pos - menu->top];

	return 0;
}

static ui_event scroll_process_direction(menu_type *m, int dir)
{
	ui_event out = EVENT_EMPTY;

	/* Reject diagonals */
	if (ddx[dir] && ddy[dir])
		;

	/* Forward/back */
	else if (ddx[dir])
		out.type = ddx[dir] < 0 ? EVT_ESCAPE : EVT_SELECT;

	/* Move up or down to the next valid & visible row */
	else if (ddy[dir])
	{
		m->cursor += ddy[dir];
		out.type = EVT_MOVE;
	}

	return out;
}

/* Virtual function table for scrollable menu skin */
static const menu_skin menu_skin_scroll =
{
	scrolling_get_cursor,
	display_scrolling,
	scroll_get_tag,
	scroll_process_direction
};


/*** Multi-column menus ***/

/* Find the position of a cursor given a screen address */
static int columns_get_cursor(int row, int col, int n, int top, region *loc)
{
	int rows_per_page = loc->page_rows;
	int colw = loc->width / (n + rows_per_page - 1) / rows_per_page;
	int cursor = row + rows_per_page * (col - loc->col) / colw;

	if (cursor < 0) cursor = 0;	/* assert: This should never happen */
	if (cursor >= n) cursor = n - 1;

	return cursor;
}

static void display_columns(menu_type *menu, int cursor, int *top, region *loc)
{
	int c, r;
	int w, h;
	int n = menu->filter_list ? menu->filter_count : menu->count;
	int col = loc->col;
	int row = loc->row;
	int rows_per_page = loc->page_rows;
	int cols = (n + rows_per_page - 1) / rows_per_page;
	int colw = 23;

	Term_get_size(&w, &h);

	if ((colw * cols) > (w - col))
		colw = (w - col) / cols;

	for (c = 0; c < cols; c++)
	{
		for (r = 0; r < rows_per_page; r++)
		{
			int pos = c * rows_per_page + r;
			bool is_cursor = (pos == cursor);

			if (pos < n)
				display_menu_row(menu, pos, 0, is_cursor,
						row + r, col + c * colw, colw);
		}
	}

	if (menu->cursor >= 0)
		Term_gotoxy(col + (cursor / rows_per_page) * colw,
				row + (cursor % rows_per_page) - *top);
}

static char column_get_tag(menu_type *menu, int pos)
{
	if (menu->selections)
		return menu->selections[pos];

	return 0;
}

static ui_event column_process_direction(menu_type *m, int dir)
{
	ui_event out = EVENT_EMPTY;

	int n = m->filter_list ? m->filter_count : m->count;

	region *loc = &m->active;
	int rows_per_page = loc->page_rows;
	int cols = (n + rows_per_page - 1) / rows_per_page;

	if (ddx[dir])
		m->cursor += ddx[dir] * rows_per_page;
	if (ddy[dir])
		m->cursor += ddy[dir];

	/* Adjust to the correct locations (roughly) */
	if (m->cursor > n)
		m->cursor = m->cursor % rows_per_page;
	else if (m->cursor < 0)
		m->cursor = (rows_per_page * cols) + m->cursor;

	out.type = EVT_MOVE;
	return out;
}

/* Virtual function table for multi-column menu skin */
static const menu_skin menu_skin_column =
{
	columns_get_cursor,
	display_columns,
	column_get_tag,
	column_process_direction
};


/* ================== GENERIC HELPER FUNCTIONS ============== */

static bool is_valid_row(menu_type *menu, int cursor)
{
	int oid;
	int count = menu->filter_list ? menu->filter_count : menu->count;

	if (cursor < 0 || cursor >= count)
		return FALSE;

	oid = menu->filter_list ? menu->filter_list[cursor] : cursor;

	if (menu->row_funcs->valid_row)
		return menu->row_funcs->valid_row(menu, oid);

	return TRUE;
}

/* 
 * Return a new position in the menu based on the key
 * pressed and the flags and various handler functions.
 */
static int get_cursor_key(menu_type *menu, int top, struct keypress key)
{
	int i;
	int n = menu->filter_list ? menu->filter_count : menu->count;

	if (menu->flags & MN_CASELESS_TAGS)
		key.code = toupper((unsigned char) key.code);

	if (menu->flags & MN_NO_TAGS)
	{
		return -1;
	}
	else if (menu->flags & MN_REL_TAGS)
	{
		for (i = 0; i < n; i++)
		{
			char c = menu->skin->get_tag(menu, i);

			if ((menu->flags & MN_CASELESS_TAGS) && c)
				c = toupper((unsigned char) c);

			if (c && c == (char)key.code)
				return i + menu->top;
		}
	}
	else if (!(menu->flags & MN_PVT_TAGS) && menu->selections)
	{
		for (i = 0; menu->selections[i]; i++)
		{
			char c = menu->selections[i];

			if (menu->flags & MN_CASELESS_TAGS)
				c = toupper((unsigned char) c);

			if (c == (char)key.code)
				return i;
		}
	}
	else if (menu->row_funcs->get_tag)
	{
		for (i = 0; i < n; i++)
		{
			int oid = menu->filter_list ? menu->filter_list[i] : i;
			char c = menu->row_funcs->get_tag(menu, oid);

			if ((menu->flags & MN_CASELESS_TAGS) && c)
				c = toupper((unsigned char) c);

			if (c && c == (char)key.code)
				return i;
		}
	}

	return -1;
}

/* Modal display of menu */
static void display_menu_row(menu_type *menu, int pos, int top,
                             bool cursor, int row, int col, int width)
{
	int flags = menu->flags;
	char sel = 0;
	int oid = pos;

	if (menu->filter_list)
		oid = menu->filter_list[oid];

	if (menu->row_funcs->valid_row && menu->row_funcs->valid_row(menu, oid) == 2)
		return;

	if (!(flags & MN_NO_TAGS))
	{
		if (flags & MN_REL_TAGS)
			sel = menu->skin->get_tag(menu, pos);
		else if (menu->selections && !(flags & MN_PVT_TAGS))
			sel = menu->selections[pos];
		else if (menu->row_funcs->get_tag)
			sel = menu->row_funcs->get_tag(menu, oid);
	}

	if (sel)
	{
		/* TODO: CHECK FOR VALID */
		byte color = curs_attrs[CURS_KNOWN][0 != (cursor)];
		Term_putstr(col, row, 3, color, format("%c) ", sel));
		col += 3;
		width -= 3;
	}

	menu->row_funcs->display_row(menu, oid, cursor, row, col, width);
}

void menu_refresh(menu_type *menu, bool reset_screen)
{
	int oid = menu->cursor;
	region *loc = &menu->active;

	if (reset_screen) {
		screen_load();
		screen_save();
	}

	if (menu->filter_list && menu->cursor >= 0)
		oid = menu->filter_list[oid];

	if (menu->title)
		Term_putstr(menu->boundary.col, menu->boundary.row,
				loc->width, TERM_WHITE, menu->title);

	if (menu->header)
		Term_putstr(loc->col, loc->row - 1, loc->width,
				TERM_WHITE, menu->header);

	if (menu->prompt)
		Term_putstr(menu->boundary.col, loc->row + loc->page_rows,
				loc->width, TERM_WHITE, menu->prompt);

	if (menu->browse_hook && oid >= 0)
		menu->browse_hook(oid, menu->menu_data, loc);

	menu->skin->display_list(menu, menu->cursor, &menu->top, loc);
}


/*** MENU RUNNING AND INPUT HANDLING CODE ***/

/*
 * Handle mouse input in a menu.
 * 
 * Mouse output is either moving, selecting, escaping, or nothing.  Returns
 * TRUE if something changes as a result of the click.
 */
bool menu_handle_mouse(menu_type *menu, const ui_event *in,
		ui_event *out)
{
	int new_cursor;

	if (!region_inside(&menu->active, in))
	{
		/* A click to the left of the active region is 'back' */
		if (!region_inside(&menu->active, in) &&
				in->mouse.x < menu->active.col)
			out->type = EVT_ESCAPE;
	}
	else
	{
		int count = menu->filter_list ? menu->filter_count : menu->count;

		new_cursor = menu->skin->get_cursor(in->mouse.y, in->mouse.x,
				count, menu->top, &menu->active);
	
		if (is_valid_row(menu, new_cursor))
		{
			if (new_cursor == menu->cursor || !(menu->flags & MN_DBL_TAP))
				out->type = EVT_SELECT;
			else
				out->type = EVT_MOVE;

			menu->cursor = new_cursor;
		}
	}

	return out->type != EVT_NONE;
}


/**
 * Handle any menu command keys / SELECT events.
 *
 * Returns TRUE if the key was handled at all (including if it's not handled
 * and just ignored).
 */
static bool menu_handle_action(menu_type *m, const ui_event *in)
{
	if (m->row_funcs->row_handler)
	{
		int oid = m->cursor;
		if (m->filter_list)
			oid = m->filter_list[m->cursor];

		return m->row_funcs->row_handler(m, in, oid);
	}

	return FALSE;
}


/**
 * Handle navigation keypresses.
 *
 * Returns TRUE if they key was intelligible as navigation, regardless of
 * whether any action was taken.
 */
bool menu_handle_keypress(menu_type *menu, const ui_event *in,
		ui_event *out)
{
	bool eat = FALSE;
	int count = menu->filter_list ? menu->filter_count : menu->count;

	/* Get the new cursor position from the menu item tags */
	int new_cursor = get_cursor_key(menu, menu->top, in->key);
	if (new_cursor >= 0 && is_valid_row(menu, new_cursor))
	{
		if (!(menu->flags & MN_DBL_TAP) || new_cursor == menu->cursor)
			out->type = EVT_SELECT;
		else
			out->type = EVT_MOVE;

		menu->cursor = new_cursor;
	}

	/* Escape stops us here */
	else if (in->key.code == ESCAPE)
		out->type = EVT_ESCAPE;

	/* Menus with no rows can't be navigated or used, so eat all keypresses */
	else if (count <= 0)
		eat = TRUE;

	/* Try existing, known keys */
	else if (in->key.code == ' ')
	{
		int rows = menu->active.page_rows;
		int total = count;

		if (rows < total)
		{
			/* Go to start of next page */
			menu->cursor += menu->active.page_rows;
			if (menu->cursor >= total - 1) menu->cursor = 0;
			menu->top = menu->cursor;
	
			out->type = EVT_MOVE;
		}
		else
		{
			eat = TRUE;
		}
	}

	else if (in->key.code == '\n' || in->key.code == '\r')
		out->type = EVT_SELECT;

	/* Try directional movement */
	else
	{
		int dir = target_dir(in->key);

		if (dir)
		{
			*out = menu->skin->process_dir(menu, dir);

			if (out->type == EVT_MOVE)
			{
				while (!is_valid_row(menu, menu->cursor))
				{
					/* Loop around */
					if (menu->cursor > count - 1)
						menu->cursor = 0;
					else if (menu->cursor < 0)
						menu->cursor = count - 1;
					else
						menu->cursor += ddy[dir];
				}
			
				assert(menu->cursor >= 0);
				assert(menu->cursor < count);
			}
		}
	}

	return eat;
}


/* 
 * Run a menu.
 *
 * If popup is true, the screen is saved before the menu is drawn, and
 * restored afterwards. Each time a popup menu is redrawn, it resets the
 * screen before redrawing.
 */
ui_event menu_select(menu_type *menu, int notify, bool popup)
{
	ui_event in = EVENT_EMPTY;
	bool no_act = (menu->flags & MN_NO_ACTION) ? TRUE : FALSE;

	assert(menu->active.width != 0 && menu->active.page_rows != 0);

	notify |= (EVT_SELECT | EVT_ESCAPE);
	if (popup)
		screen_save();

	/* Stop on first unhandled event */
	while (!(in.type & notify))
	{
		ui_event out = EVENT_EMPTY;

		menu_refresh(menu, popup);
		in = inkey_ex();

		/* Handle mouse & keyboard commands */
		if (in.type == EVT_MOUSE) {
			menu_handle_mouse(menu, &in, &out);
		} else if (in.type == EVT_KBRD) {
			if (!no_act && menu->cmd_keys &&
					strchr(menu->cmd_keys, (char)in.key.code) &&
					menu_handle_action(menu, &in))
				continue;

			menu_handle_keypress(menu, &in, &out);
		} else if (in.type == EVT_RESIZE) {
			menu_calc_size(menu);
			if (menu->row_funcs->resize)
				menu->row_funcs->resize(menu);
		}

		/* XXX should redraw menu here if cursor has moved */

		/* If we've selected an item, then send that event out */
		if (out.type == EVT_SELECT && !no_act && menu_handle_action(menu, &out))
			continue;

		/* Notify about the outgoing type */
		if (notify & out.type) {
			if (popup)
				screen_load();
			return out;
		}
	}

	if (popup)
		screen_load();
	return in;
}


/* ================== MENU ACCESSORS ================ */

/**
 * Return the menu iter struct for a given iter ID.
 */
const menu_iter *menu_find_iter(menu_iter_id id)
{
	switch (id)
	{
		case MN_ITER_ACTIONS:
			return &menu_iter_actions;

		case MN_ITER_STRINGS:
			return &menu_iter_strings;
	}

	return NULL;
}

/*
 * Return the skin behaviour struct for a given skin ID.
 */
static const menu_skin *menu_find_skin(skin_id id)
{
	switch (id)
	{
		case MN_SKIN_SCROLL:
			return &menu_skin_scroll;

		case MN_SKIN_COLUMNS:
			return &menu_skin_column;
	}

	return NULL;
}


void menu_set_filter(menu_type *menu, const int filter_list[], int n)
{
	menu->filter_list = filter_list;
	menu->filter_count = n;

	menu_ensure_cursor_valid(menu);
}

void menu_release_filter(menu_type *menu)
{
	menu->filter_list = NULL;
	menu->filter_count = 0;

	menu_ensure_cursor_valid(menu);

}

void menu_ensure_cursor_valid(menu_type *m)
{
	int row;
	int count = m->filter_list ? m->filter_count : m->count;

	for (row = m->cursor; row < count; row++)
	{
		if (is_valid_row(m, row))
		{
			m->cursor = row;
			return;
		}
	}

	/* If we've run off the end, without finding a valid row, put cursor
	 * on the last row */
	m->cursor = count - 1;
}

/* ======================== MENU INITIALIZATION ==================== */

static bool menu_calc_size(menu_type *menu)
{
	/* Calculate term-relative positions */
	menu->active = region_calculate(menu->boundary);

	if (menu->title)
	{
		menu->active.row += 2;
		menu->active.page_rows -= 2;
		menu->active.col += 4;
	}

	if (menu->header)
	{
		menu->active.row++;
		menu->active.page_rows--;
	}

	if (menu->prompt)
	{
		if (menu->active.page_rows > 1) {
			menu->active.page_rows--;
		} else {
			int offset = strlen(menu->prompt) + 2;
			menu->active.col += offset;
			menu->active.width -= offset;
		}
	}

	return (menu->active.width > 0 && menu->active.page_rows > 0);
}

bool menu_layout(menu_type *m, const region *loc)
{
	m->boundary = *loc;
	return menu_calc_size(m);
}

void menu_setpriv(menu_type *menu, int count, void *data)
{
	menu->count = count;
	menu->menu_data = data;

	menu_ensure_cursor_valid(menu);
}

void *menu_priv(menu_type *menu)
{
	return menu->menu_data;
}

void menu_init(menu_type *menu, skin_id skin_id, const menu_iter *iter)
{
	const menu_skin *skin = menu_find_skin(skin_id);
	assert(skin && "menu skin not found!");
	assert(iter && "menu iter not found!");

	/* Wipe the struct */
	memset(menu, 0, sizeof *menu);

	/* Menu-specific initialisation */
	menu->row_funcs = iter;
	menu->skin = skin;
	menu->cursor = 0;
}

menu_type *menu_new(skin_id skin_id, const menu_iter *iter)
{
	menu_type *m = mem_alloc(sizeof *m);
	menu_init(m, skin_id, iter);
	return m;
}

menu_type *menu_new_action(menu_action *acts, size_t n)
{
	menu_type *m = menu_new(MN_SKIN_SCROLL, menu_find_iter(MN_ITER_ACTIONS));
	menu_setpriv(m, n, acts);
	return m;
}


/*** Dynamic menu handling ***/

struct menu_entry {
	char *text;
	int value;

	struct menu_entry *next;
};

static void dynamic_display(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	struct menu_entry *entry;
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];

	for (entry = menu_priv(m); oid; oid--) {
		entry = entry->next;
		assert(entry);
	}

	Term_putstr(col, row, width, color, entry->text);
}

static const menu_iter dynamic_iter = {
	NULL,	/* tag */
	NULL,	/* valid */
	dynamic_display,
	NULL,	/* handler */
	NULL	/* resize */
};

menu_type *menu_dynamic_new(void)
{
	menu_type *m = menu_new(MN_SKIN_SCROLL, &dynamic_iter);
	menu_setpriv(m, 0, NULL);
	return m;
}

void menu_dynamic_add(menu_type *m, const char *text, int value)
{
	struct menu_entry *head = menu_priv(m);
	struct menu_entry *new = mem_zalloc(sizeof *new);

	assert(m->row_funcs == &dynamic_iter);

	new->text = string_make(text);
	new->value = value;

	if (head) {
		struct menu_entry *tail = head;
		while (1) {
			if (tail->next)
				tail = tail->next;
			else
				break;
		}

		tail->next = new;
		menu_setpriv(m, m->count + 1, head);
	} else {
		menu_setpriv(m, m->count + 1, new);
	}
}

size_t menu_dynamic_longest_entry(menu_type *m)
{
	size_t biggest = 0;
	size_t current;

	struct menu_entry *entry;

	for (entry = menu_priv(m); entry; entry = entry->next) {
		current = strlen(entry->text);
		if (current > biggest)
			biggest = current;
	}

	return biggest;
}

int menu_dynamic_select(menu_type *m)
{
	ui_event e = menu_select(m, 0, TRUE);
	struct menu_entry *entry;
	int cursor = m->cursor;

	if (e.type == EVT_ESCAPE)
		return -1;

	for (entry = menu_priv(m); cursor; cursor--) {
		entry = entry->next;
		assert(entry);
	}	

	return entry->value;
}

void menu_dynamic_free(menu_type *m)
{
	struct menu_entry *entry = menu_priv(m);
	if (entry) {
		struct menu_entry *next = entry->next;
		string_free(entry->text);
		mem_free(entry);
		entry = next;
	}
	mem_free(m);
}

