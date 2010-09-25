/*
 * File: ui-menu.c
 * Purpose: Generic menu interaction functions
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

/* forward declarations */
static void display_menu_row(menu_type *menu, int pos, int top,
			     bool cursor, int row, int col, int width);

/* ------------------------------------------------------------------------
 * MN_ACTIONS HELPER FUNCTIONS
 *
 * MN_ACTIONS is the type of menu iterator that displays a simple list of
 * menu_actions.
 * ------------------------------------------------------------------------ */

/* Display an event, with possible preference overrides */
static void display_action_aux(menu_action *act, byte color, int row, int col, int wid)
{
	/* TODO: add preference support */
	/* TODO: wizard mode should show more data */
	Term_erase(col, row, wid);

	if (act->name)
		Term_putstr(col, row, wid, color, act->name);
}

static void display_action(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	menu_action *acts = (menu_action *) menu->menu_data;
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];

	display_action_aux(&acts[oid], color, row, col, width);
}

/* act on selection only */
/* Return: true if handled. */
static bool handle_menu_item_action(char cmd, void *db, int oid)
{
	menu_action *act = &((menu_action *)db)[oid];

	if (cmd == '\xff' && act->action)
	{
		act->action(act->data, act->name);
		return TRUE;
	}
	else if (cmd == '\xff')
	{
		return TRUE;
	}

	return FALSE;
}

static int valid_menu_action(menu_type *menu, int oid)
{
	menu_action *acts = (menu_action *)menu->menu_data;
	return (NULL != acts[oid].name);
}

/* Virtual function table for action_events */
const menu_iter menu_iter_actions =
{
	NULL,                     /* get_tag() */
	valid_menu_action,        /* valid_row() */
	display_action,           /* display_row() */
	handle_menu_item_action   /* row_handler() */
};


/* ------------------------------------------------------------------------
 * MN_ITEMS HELPER FUNCTIONS
 *
 * MN_ITEMS is the type of menu iterator that displays a simple list of 
 * menu_items (i.e. menu_actions with optional per-item flags and 
 * "selection" keys.
 * ------------------------------------------------------------------------ */

static char tag_menu_item(menu_type *menu, int oid)
{
	menu_item *items = (menu_item *)menu->menu_data;
	return items[oid].sel;
}

static void display_menu_item(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	menu_item *items = (menu_item *)menu->menu_data;
	byte color = curs_attrs[!(items[oid].flags & (MN_GRAYED))][0 != cursor];

	display_action_aux(&items[oid].act, color, row, col, width);
}

/* act on selection only */
static bool handle_menu_item(char cmd, void *db, int oid)
{
	if (cmd == '\xff')
	{
		menu_item *item = &((menu_item *)db)[oid];

		if (item->flags & MN_DISABLED)
			return TRUE;

		if (item->act.action)
			item->act.action(item->act.data, item->act.name);

		if (item->flags & MN_SELECTABLE)
			item->flags ^= MN_SELECTED;

		return TRUE;
	}

	return FALSE;
}

static int valid_menu_item(menu_type *menu, int oid)
{
	menu_item *items = (menu_item *)menu->menu_data;

	if (items[oid].flags & MN_HIDDEN)
		return 2;

	return (NULL != items[oid].act.name);
}

/* Virtual function table for menu items */
const menu_iter menu_iter_items =
{
	tag_menu_item,       /* get_tag() */
	valid_menu_item,     /* valid_row() */
	display_menu_item,   /* display_row() */
	handle_menu_item     /* row_handler() */
};

/* ------------------------------------------------------------------------
 * MN_STRINGS HELPER FUNCTIONS
 *
 * MN_STRINGS is the type of menu iterator that displays a simple list of 
 * strings - no action is associated, as selection will just return the index.
 * ------------------------------------------------------------------------ */
static void display_string(menu_type *menu, int oid, bool cursor,
               int row, int col, int width)
{
	const char **items = (const char **)menu->menu_data;
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];
	Term_putstr(col, row, width, color, items[oid]);
}

/* Virtual function table for displaying arrays of strings */
const menu_iter menu_iter_strings =
{ 
	NULL,              /* get_tag() */
	NULL,              /* valid_row() */
	display_string,    /* display_row() */
	NULL 	           /* row_handler() */
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
	int n = menu->filter_count;
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


	for (i = 0; i < rows_per_page && i < n; i++)
	{
		bool is_curs = (i == cursor - *top);
		display_menu_row(menu, i + *top, *top, is_curs, row + i, col,
						 loc->width);
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

/* Virtual function table for scrollable menu skin */
const menu_skin menu_skin_scroll =
{
	scrolling_get_cursor,
	display_scrolling,
	scroll_get_tag
};


/* Multi-column menu */
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
	int n = menu->filter_count;
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
			display_menu_row(menu, pos, 0, is_cursor, row + r, col + c * colw,
							 colw);
		}
	}
}

static char column_get_tag(menu_type *menu, int pos)
{
	if (menu->selections)
		return menu->selections[pos];

	return 0;
}

/* Virtual function table for multi-column menu skin */
static const menu_skin menu_skin_column =
{
	columns_get_cursor,
	display_columns,
	column_get_tag
};


/* ================== GENERIC HELPER FUNCTIONS ============== */

static bool is_valid_row(menu_type *menu, int cursor)
{
	int oid = cursor;

	if (cursor < 0 || cursor >= menu->filter_count)
		return FALSE;

	if (menu->filter_list)
		oid = menu->filter_list[cursor];

	if (!menu->row_funcs->valid_row)
		return TRUE;

	return menu->row_funcs->valid_row(menu, oid);
}

/* 
 * Return a new position in the menu based on the key
 * pressed and the flags and various handler functions.
 */
static int get_cursor_key(menu_type *menu, int top, char key)
{
	int i;
	int n = menu->filter_count;

	if (menu->flags & MN_CASELESS_TAGS)
		key = toupper((unsigned char) key);

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

			if (c && c == key)
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

			if (c == key)
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

			if (c && c == key)
				return i;
		}
	}

	return -1;
}

/*
 * Event handler wrapper function
 * Filters unhandled keys & conditions 
 */
static bool handle_menu_key(char cmd, menu_type *menu, int cursor)
{
	int oid = cursor;
	int flags = menu->flags;

	if (menu->filter_list) oid = menu->filter_list[cursor];
	if (flags & MN_NO_ACT) return FALSE;

	if (cmd == ESCAPE) return FALSE;
	if (!(cmd == '\xff') &&
			(!menu->cmd_keys || !strchr(menu->cmd_keys, cmd)))
		return FALSE;

	if (menu->row_funcs->row_handler &&
		menu->row_funcs->row_handler(cmd, (void *)menu->menu_data, oid))
	{
		ui_event_data ke;
		ke.type = EVT_SELECT;
		ke.key = cmd;
		ke.index = cursor;
		Term_event_push(&ke);
		return TRUE;
	}

	return FALSE;
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

void menu_refresh(menu_type *menu)
{
	region *loc = &menu->boundary;
	int oid = menu->cursor;

	if (menu->filter_list && menu->cursor >= 0)
		oid = menu->filter_list[oid];

	region_erase(&menu->boundary);

	if (menu->title)
		Term_putstr(loc->col, loc->row, loc->width, TERM_WHITE, menu->title);

	if (menu->prompt)
		Term_putstr(loc->col, loc->row + loc->page_rows - 1, loc->width,
					TERM_WHITE, menu->prompt);

	if (menu->browse_hook && oid >= 0)
		menu->browse_hook(oid, (void*) menu->menu_data, loc);

	menu->skin->display_list(menu, menu->cursor, &menu->top, &menu->active);
}

/* The menu event loop, called as a handler from the event loop */
bool menu_handle_event(void *object, const ui_event_data *in)
{
	menu_type *menu = object;
	int n = menu->filter_count;
	int *cursor = &menu->cursor;
	ui_event_data out;

	out.key = '\xff';

	switch (in->type)
	{
		case EVT_MOUSE:
		{
			int m_curs;

			if (!region_inside(&menu->active, in))
			{
				if (!region_inside(&menu->boundary, in))
				{
					return FALSE;
				}

				/* used for heirarchical menus */
				if (in->mousex < menu->active.col)
				{
					out.type = EVT_BACK;
					break;
				}

				return FALSE;
			}

			m_curs = menu->skin->get_cursor(in->mousey, in->mousex,
							menu->filter_count, menu->top,
							&menu->active);

			/* Ignore this event - retry */
			if (!is_valid_row(menu, m_curs))
				return TRUE;

			out.index = m_curs;

			if (*cursor == m_curs || !(menu->flags & MN_DBL_TAP))
			{
				if (*cursor != m_curs)
				{
					*cursor = m_curs;
					menu_refresh(menu);
				}
				out.type = EVT_SELECT;
			}
			else
			{
				out.type = EVT_MOVE;
			}

			*cursor = m_curs;

			break;
		}

		case EVT_KBRD:
		{
			int c, dir;

			/* could install handle_menu_key as a handler */
			if ((menu->cmd_keys && strchr(menu->cmd_keys, in->key))
				|| in->key == ESCAPE)
			{
				if (menu->flags & MN_NO_ACT)
					return FALSE;
				else
					return handle_menu_key(in->key, menu, *cursor);
			}

			c = get_cursor_key(menu, menu->top, in->key);

			/* keypress shortcuts are allowed, but the choice */
			/* was invalid - try again! */
			if (c > 0 && !is_valid_row(menu, c))
			{
				return FALSE;
			}
			/* Valid selection */
			else if (c >= 0)
			{
				out.index = c;

				if (*cursor == c || !(menu->flags & MN_DBL_TAP))
				{
					if (*cursor != c)
					{
						*cursor = c;
						menu_refresh(menu);
					}
					out.type = EVT_SELECT;
				}
				else
				{
					out.type = EVT_MOVE;
				}

				*cursor = c;

				break;
			}

			/* Not handled */
			if (menu->flags & MN_NO_CURSOR)
				return FALSE;

			if (in->key == ' ')
			{
				int rows = menu->active.page_rows;
				int total = menu->filter_count;

				/* Ignore it if there's a page or less to show */
				if (rows >= total) return FALSE;

				/* Go to start of next page */
				*cursor += menu->active.page_rows;
				if (*cursor >= total - 1) *cursor = 0;
				menu->top = *cursor;

				/* Set the out event type */
				out.type = EVT_MOVE;
				out.index = *cursor;

				break;
			}

			/* Cursor movement */
			dir = target_dir(in->key);

			/* Handle Enter */
			if (in->key == '\n' || in->key == '\r')
			{
				out.type = EVT_SELECT;
				out.index = *cursor;
			}

			/* Reject diagonals */
			else if (ddx[dir] && ddy[dir])
			{
				return FALSE;
			}

			/* Forward/back */
			else if (ddx[dir])
			{
				out.type = ddx[dir] < 0 ? EVT_BACK : EVT_SELECT;
				out.index = *cursor;
			}

			/* Move up or down to the next valid & visible row */
			else if (ddy[dir])
			{
				int dy = ddy[dir];
				int ind = *cursor + dy;

				/* Duck out here for 0-entry lists */
				if (n == 0) return FALSE;

				/* Find the next valid row */
				while (!is_valid_row(menu, ind))
				{
					/* Loop around */
					if (ind > n - 1)  ind = 0;
					else if (ind < 0) ind = n - 1;
					else              ind += dy;
				}

				/* Set the cursor */
				*cursor = ind;

				/* Set the "out" event information */
				out.type = EVT_MOVE;
				out.index = ind;
			}
			else
			{
				return FALSE;
			}

			break;
		}

		case EVT_REFRESH:
		{
			menu_refresh(menu);
			return FALSE;
		}

		default:
		{
			return FALSE;
		}
	}

	if (out.type == EVT_SELECT && handle_menu_key('\xff', menu, *cursor))
		return TRUE;

	if (out.type == EVT_MOVE)
		menu_refresh(menu);

	Term_event_push(&out);

	return TRUE;
}



/* 
 * Modal selection from a menu.
 * Arguments:
 *  - menu - the menu
 *  - cursor - the row in which the cursor should start.
 *  - no_handle - Don't handle these events. ( bitwise or of ui_event_type)
 *     0 - return values below are limited to the set below.
 * Additional examples:
 *     EVT_MOVE - return values also include menu movement.
 *     EVT_CMD  - return values also include command IDs to process.
 * Returns: an event, possibly requiring further handling.
 * Return values:
 *  EVT_SELECT - success. ui_event_data::index is set to the cursor position.
 *      *cursor is also set to the cursor position.
 *  EVT_OK  - success. A command event was handled.
 *     *cursor is also set to the associated row.
 *  EVT_BACK - no selection; go to previous menu in hierarchy
 *  EVT_ESCAPE - Abandon modal interaction
 *  EVT_KBRD - An unhandled keyboard event
 */
ui_event_data menu_select(menu_type *menu, int *cursor, int no_handle)
{
	ui_event_data ke = EVENT_EMPTY;

	menu->cursor = *cursor;

	/* Menu shall not handle these */
	no_handle |= (EVT_SELECT | EVT_BACK | EVT_ESCAPE);
	no_handle &= ~(EVT_REFRESH);

	if (!menu->filter_list)
		menu->filter_count = menu->count;

	menu_refresh(menu);

	/* Check for command flag */
	if (p_ptr->command_new)
	{
		Term_key_push(p_ptr->command_new);
		p_ptr->command_new = 0;
	}

	/* Stop on first unhandled event */
	while (!(ke.type & no_handle))
	{
		ke = run_event_loop(menu_handle_event, menu, (EVT_KBRD | EVT_MOUSE | EVT_REFRESH), NULL);

		switch (ke.type)
		{
			/* menu always returns these */
			case EVT_SELECT:
			{
				if (*cursor != ke.index)
				{
					*cursor = ke.index;
					/* One last time */
					menu->refresh(menu);
					break;
				}

				/* return sometimes-interesting things here */
			}

			case EVT_MOVE:
			{
				/* EVT_MOVE uses -1, n to allow modular cursor */
				if (ke.index < menu->filter_count && ke.index >= 0)
					*cursor = menu->cursor;
			}

			case EVT_KBRD:
			{
				/* Just in case */
				if (ke.key == ESCAPE)
					ke.type = EVT_ESCAPE;

				break;
			}

			default:
			{
				break;
			}
		}
	}

	return ke;
}


/* ================== MENU ACCESSORS ================ */

/**
 * Return the menu iter struct for a given iter ID.
 */
const menu_iter *find_menu_iter(menu_iter_id id)
{
	switch (id)
	{
		case MN_ITER_ACTIONS:
			return &menu_iter_actions;

		case MN_ITER_ITEMS:
			return &menu_iter_items;

		case MN_ITER_STRINGS:
			return &menu_iter_strings;
	}

	return NULL;
}

/*
 * Return the skin behaviour struct for a given skin ID.
 */
static const menu_skin *find_menu_skin(skin_id id)
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


/*
 * Set the filter to a new value.
 */
void menu_set_filter(menu_type *menu, const int filter_list[], int n)
{
	menu->filter_list = filter_list;
	menu->filter_count = n;
}

/* Remove the filter */
void menu_release_filter(menu_type *menu)
{
	menu->filter_list = NULL;
	menu->filter_count = menu->count;
}

/* ======================== MENU INITIALIZATION ==================== */

/* This is extremely primitive, barely sufficient to the job done */
bool menu_layout(menu_type *menu, const region *loc)
{
	region active;

	if (!loc) return TRUE;
	active = *loc;

	if (active.width <= 0 || active.page_rows <= 0)
	{
		int w, h;
		Term_get_size(&w, &h);
		if (active.width <= 0)
			active.width = w + active.width - active.col;
		if (active.page_rows <= 0)
			active.page_rows = h + loc->page_rows - active.row;
	}

	menu->boundary = active;

	if (menu->title)
	{
		active.row += 2;
		active.page_rows -= 2;
		/* TODO: handle small screens */
		active.col += 4;
	}
	if (menu->prompt)
	{
		if (active.page_rows > 1)
			active.page_rows--;
		else
		{
			int offset = strlen(menu->prompt) + 2;
			active.col += offset;
			active.width -= offset;
		}
	}
	/* TODO: */
	/* if(menu->cmd_keys) active.page_rows--; */

	menu->active = active;
	return (active.width > 0 && active.page_rows > 0);
}


/*
 * Correctly initialise the menu block at 'menu' so that it's ready to use.
 * Use the display skin given in 'skin' and the iterator in 'iter', and set
 * up to use the region of the window given in 'loc'
 *
 * Returns FALSE if something goes wrong, and TRUE otherwise (i.e. always).
 */
bool menu_init(menu_type *menu, skin_id skin_id, const menu_iter *iter, const region *loc)
{
	const menu_skin *skin = find_menu_skin(skin_id);
	assert(skin && "menu skin not found!");
	assert(iter && "menu iter not found!");
	assert(loc && "no screen location specified!");

	/* Menu-specific initialisation */
	menu->refresh = menu_refresh;
	menu->boundary = SCREEN_REGION;
	menu->row_funcs = iter;
	menu->skin = skin;

	/* We rely on filter_count containing the number of items we're
	   selecting from. */
	if (menu->count && !menu->filter_list)
		menu->filter_count = menu->count;

	/* Do an initial layout calculation so we're ready to display. */
	menu_layout(menu, loc);

	/* TODO:  Check for collisions in selections & command keys here */
	return TRUE;
}
