/* File ui.c */

/*
 * Copyright (c) 2007 Pete Mack and others
 * This code released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */

/*
Description:
Implementation of Extremely Basic Event Model.
  Limits:
    all events are of the concrete type key_event (see z-util.h), 
    which are supposed to model simple UI actions:
	- < escape >
	- keystroke
	- mousepress
	- select menu element
	- move menu cursor
	- back to parent (hierarchical menu escape)

There are 3 basic event-related classes:
The key_event.
Concrete event, with at most 32 distinct types.

The event_listener observer for key events

The event_target   The registrar for event_listeners.
For convenience, the event target is also an event_listener.


*/

#include "angband.h"

/* Some useful constants */

const char default_choice[] =
	"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char lower_case[] = "abcdefghijklmnopqrstuvwxyz";

int jumpscroll = 0;
int menu_width = 23;

/* forward declarations */
static void display_menu_row(menu_type *menu, int pos, int top,
							 bool cursor, int row, int col, int width);

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

bool region_inside(const region *loc, const key_event *key)
{
	if ((loc->col > key->mousex) || (loc->col + loc->width <= key->mousex))
		return FALSE;

	if ((loc->row > key->mousey) || (loc->row + loc->page_rows <= key->mousey))
		return FALSE;

	return TRUE;
}

/* ======================= EVENTS ======================== */

/* List of event listeners--Helper class for event_target and the event loop */
struct listener_list
{
	event_listener *listener;
	struct listener_list *next;
};


void stop_event_loop()
{
	key_event stop = { EVT_STOP };

	/* Stop right away! */
	Term_event_push(&stop);
}

/*
 * Primitive event loop.
 *  - target = the event target
 *  - forever - if false, stop at first unhandled event. Otherwise, stop only
 *    for STOP events
 *  - start - optional initial event that allows you to prime the loop without
 *    pushing the event queue.
 *  Returns:
 *    EVT_STOP - the loop was halted.
 *    EVT_AGAIN - start was not handled, and forever is false
 *    The first unhandled event - forever is false.
 */
key_event run_event_loop(event_target * target, bool forever, const key_event *start)
{
	key_event ke;
	bool handled = TRUE;

	while (forever || handled)
	{
		listener_list *list = target->observers;
		handled = FALSE;

		if (start) ke = *start;
		else ke = inkey_ex();

		if (ke.type == EVT_STOP)
			break;

		if (ke.type & target->self.events.evt_flags)
			handled = target->self.handler(target->self.object, &ke);

		if (!target->is_modal)
		{
			while (list && !handled)
			{
				if (ke.type & list->listener->events.evt_flags)
					handled = list->listener->handler(list->listener->object, &ke);

				list = list->next;
			}
		}

		if (handled) start = NULL;
	}

	if (start)
	{
		ke.type = EVT_AGAIN;
		ke.key = '\xff';
	}

	return ke;
}

void add_listener(event_target * target, event_listener * observer)
{
	listener_list *link;

	MAKE(link, listener_list);
	link->listener = observer;
	link->next = target->observers;
	target->observers = link;
}

void remove_listener(event_target * target, event_listener * observer)
{
	listener_list *cur = target->observers;
	listener_list **prev = &target->observers;

	while (cur)
	{
		if (cur->listener == observer)
		{
			*prev = cur->next;
			FREE(cur);
			break;
		}
	}

	bell("remove_listener: no such observer");
}



/* ======================= MN_EVT HELPER FUNCTIONS ====================== */

/* Display an event, with possible preference overrides */
static void display_event_aux(event_action *event, int menu_id, byte color, int row, int col, int wid)
{
	/* TODO: add preference support */
	/* TODO: wizard mode should show more data */
	Term_erase(col, row, wid);

	if (event->name)
		Term_putstr(col, row, wid, color, event->name);
}

static void display_event(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	event_action *evts = (event_action *)menu->menu_data;
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];

	display_event_aux(&evts[oid], menu->target.self.object_id, color, row, col,
					  width);
}

/* act on selection only */
/* Return: true if handled. */
static bool handle_menu_item_event(char cmd, void *db, int oid)
{
	event_action *evt = &((event_action *)db)[oid];

	if (cmd == '\xff' && evt->action)
	{
		evt->action(evt->data, evt->name);
		return TRUE;
	}
	else if (cmd == '\xff')
	{
		return TRUE;
	}

	return FALSE;
}

static int valid_menu_event(menu_type *menu, int oid)
{
	event_action *evts = (event_action *)menu->menu_data;
	return (NULL != evts[oid].name);
}

/* Virtual function table for action_events */
static const menu_iter menu_iter_event =
{
	MN_EVT,
	0,
	valid_menu_event,
	display_event,
	handle_menu_item_event
};


/*======================= MN_ACT HELPER FUNCTIONS ====================== */

static char tag_menu_item(menu_type *menu, int oid)
{
	menu_item *items = (menu_item *)menu->menu_data;
	return items[oid].sel;
}

static void display_menu_item(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	menu_item *items = (menu_item *)menu->menu_data;
	byte color = curs_attrs[!(items[oid].flags & (MN_GRAYED))][0 != cursor];

	display_event_aux(&items[oid].evt, menu->target.self.object_id, color, row,
					  col, width);
}

/* act on selection only */
static bool handle_menu_item(char cmd, void *db, int oid)
{
	if (cmd == '\xff')
	{
		menu_item *item = &((menu_item *)db)[oid];

		if (item->flags & MN_DISABLED)
			return TRUE;
		if (item->evt.action)
			item->evt.action(item->evt.data, item->evt.name);
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

	return (NULL != items[oid].evt.name);
}

/* Virtual function table for menu items */
static const menu_iter menu_iter_item =
{
	MN_ACT,
	tag_menu_item,
	valid_menu_item,
	display_menu_item,
	handle_menu_item
};

/* Simple strings */

static void display_string(menu_type *menu, int oid, bool cursor,
               int row, int col, int width)
{
	const char **items = (const char **)menu->menu_data;
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];
	Term_putstr(col, row, width, color, items[oid]);
}

/* Virtual function table for displaying arrays of strings */
static const menu_iter menu_iter_string =
{ MN_STRING, 0, 0, display_string, 0 };




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
static void
display_scrolling(menu_type *menu, int cursor, int *top, region *loc)
{
	int col = loc->col;
	int row = loc->row;
	int rows_per_page = loc->page_rows;
	int n = menu->filter_count;
	int i;

	if ((cursor <= *top) && (*top > 0))
		*top = cursor - jumpscroll - 1;
	if (cursor >= *top + (rows_per_page - 1))
		*top = cursor - (rows_per_page - 1) + 1 + jumpscroll;
	if (*top > n - rows_per_page)
		*top = n - rows_per_page;
	if (*top < 0)
		*top = 0;

	for (i = 0; i < rows_per_page && i < n; i++)
	{
		bool is_curs = (i == cursor - *top);
		display_menu_row(menu, i + *top, *top, is_curs, row + i, col,
						 loc->width);
	}

	if (cursor >= 0)
		Term_gotoxy(col, row + cursor - *top);
}

static char scroll_get_tag(menu_type *menu, int pos)
{
	if(menu->selections)
		return menu->selections[pos - menu->top];
	return 0;
}

/* Virtual function table for scrollable menu skin */
static const menu_skin scroll_skin =
{
	MN_SCROLL,
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

void display_columns(menu_type *menu, int cursor, int *top, region *loc)
{
	int c, r;
	int w, h;
	int n = menu->filter_count;
	int col = loc->col;
	int row = loc->row;
	int rows_per_page = loc->page_rows;
	int cols = (n + rows_per_page - 1) / rows_per_page;
	int colw = menu_width;

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
	if(menu->selections)
		return menu->selections[pos];
	return 0;
}

/* Virtual function table for multi-column menu skin */
static const menu_skin column_skin =
{
	MN_COLUMNS,
	columns_get_cursor,
	display_columns,
	column_get_tag
};

/* ================== IMPLICIT MENU FOR KEY SELECTION ================== */

static void display_nothing(menu_type *menu, int cursor, int *top, region *loc)
{
}

static int no_cursor(int row, int col, int n, int top, region *loc)
{
	return -1;
}

static const menu_skin key_select_skin =
{
	MN_KEY_ONLY,
	no_cursor,
	display_nothing,
};


/* ================== GENERIC HELPER FUNCTIONS ============== */

static bool is_valid_row(menu_type *menu, int cursor)
{
	int oid = cursor;

	if (cursor < 0 || cursor >= menu->filter_count)
		return FALSE;

	if (menu->object_list)
		oid = menu->object_list[cursor];

	if (!menu->row_funcs->valid_row)
		return TRUE;

	return menu->row_funcs->valid_row(menu, oid);
}

static int get_cursor_key(menu_type *menu, int top, char key)
{
	int i;
	int n = menu->filter_count;

	if (menu->flags & MN_NO_TAGS)
	{
		return -1;
	}
	else if (menu->flags & MN_REL_TAGS)
	{
		for (i = 0; i < n; i++)
		{
			char c = menu->skin->get_tag(menu, i);
			if (c && c == key)
				return i + menu->top;
		}
	}
	else if (!(menu->flags & MN_PVT_TAGS) && menu->selections)
	{
		for (i = 0; menu->selections[i]; i++)
		{
			if (menu->selections[i] == key)
				return i;
		}
	}
	else if (menu->row_funcs->get_tag)
	{
		for (i = 0; i < n; i++)
		{
			int oid = menu->object_list ? menu->object_list[i] : i;
			char c = menu->row_funcs->get_tag(menu, oid);
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

	if (menu->object_list) oid = menu->object_list[cursor];
	if (flags & MN_NO_ACT) return FALSE;

	if (isspace(cmd) && (flags & MN_PAGE)) return FALSE;

	if (cmd == ESCAPE) return FALSE;
	if (!cmd == '\xff' && (!menu->cmd_keys || !strchr(menu->cmd_keys, cmd)))
		return FALSE;

	if (menu->row_funcs->row_handler &&
		menu->row_funcs->row_handler(cmd, (void *)menu->menu_data, oid))
	{
		key_event ke;
		ke.type = EVT_SELECT;
		ke.key = cmd;
		ke.index = cursor;
		Term_event_push(&ke);
		return TRUE;
	}

	return FALSE;
}

/* Modal display of menu */
static void
display_menu_row(menu_type *menu, int pos, int top,
                 bool cursor, int row, int col, int width)
{
	int flags = menu->flags;
	char sel = 0;
	int oid = pos;

	if (menu->object_list)
		oid = menu->object_list[oid];

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
	if (menu->object_list && menu->cursor >= 0)
		oid = menu->object_list[oid];

	region_erase(&menu->boundary);

	if (menu->title)
		Term_putstr(loc->col, loc->row, loc->width, TERM_WHITE, menu->title);
	if (menu->prompt)
		Term_putstr(loc->col, loc->row + loc->page_rows - 1, loc->width,
					TERM_WHITE, menu->prompt);

	menu->skin->display_list(menu, menu->cursor, &menu->top, &menu->active);

	if (menu->browse_hook && oid >= 0)
		menu->browse_hook(oid, (void*) menu->menu_data, loc);
}

/* The menu event loop */
static bool menu_handle_event(menu_type *menu, const key_event *in)
{
	int n = menu->filter_count;
	int *cursor = &menu->cursor;
	key_event out;

	out.key = '\xff';

	if (menu->target.observers)
	{
		/* TODO: need a panel dispatcher here, not a generic target */
		event_target t = { { 0, 0, 0, 0 }, FALSE, menu->target.observers };
		out = run_event_loop(&t, FALSE, in);

		if (out.type != EVT_AGAIN)
		{
			if (out.type == EVT_SELECT)
			{
				/* HACK: can't return selection event from submenu (no ID) */
				out.type = EVT_REFRESH;
				Term_event_push(&out);
			}

			return TRUE;
		}
	}

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
				out.type = EVT_SELECT;
			else
				out.type = EVT_MOVE;

			*cursor = m_curs;

			break;
		}

		case EVT_KBRD:
		{
			int dir;

			/* could install handle_menu_key as a handler */
			if ((menu->cmd_keys && strchr(menu->cmd_keys, in->key))
				|| in->key == ESCAPE)
			{
				if (menu->flags & MN_NO_ACT)
					return FALSE;
				else
					return handle_menu_key(in->key, menu, *cursor);
			}

			if (!(menu->flags & MN_NO_TAGS))
			{
				int c = get_cursor_key(menu, menu->top, in->key);

				/* retry! */
				if (c > 0 && !is_valid_row(menu, c))
				{
					return FALSE;
				}
				else if (c >= 0)
				{
					menu->cursor = c;
					out.type = EVT_SELECT;
					out.index = c;
					break;
				}
			}

			/* Not handled */
			if (menu->flags & MN_NO_CURSOR)
				return FALSE;

			if (isspace(in->key) && (menu->flags & MN_PAGE))
			{
				/* Go to start of next page */
				*cursor += menu->active.page_rows - (*cursor % menu->active.page_rows);
				if(*cursor >= menu->filter_count) 
					*cursor = 0;
				out.type = EVT_MOVE;
				out.index = *cursor;
			}

			/* cursor movement */
			dir = target_dir(in->key);

			/* Reject diagonals */
			if (ddx[dir] && ddy[dir])
			{
				return FALSE;
			}
			else if (ddx[dir])
			{
				out.type = ddx[dir] < 0 ? EVT_BACK : EVT_SELECT;
				out.index = *cursor;
			}
			/* Move up or down to the next valid & visible row */
			else if (ddy[dir])
			{
				int ind;
				int dy = ddy[dir];

				out.type = EVT_MOVE;
				for (ind = *cursor + dy; ind < n && ind >= 0
					 && (TRUE != is_valid_row(menu, ind)); ind += dy) ;
				out.index = ind;
				if (ind < n && ind >= 0) *cursor = ind;
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


/* VTAB for menus */
static const panel_type menu_target =
{
	{
	 {0,								/* listener.object_id */
	  (handler_f) menu_handle_event,	/* listener.handler */
	  (release_f) menu_destroy,			/* listener.release */
	  0,								/* listener.object */
	  {EVT_KBRD | EVT_MOUSE | EVT_REFRESH}	/* listener.events */
	 },
	 TRUE,								/* target.is_modal */
	},
	menu_refresh,						/* refresh() */
	{0, 0, 0, 0}						/* boundary */
};

/* 
 * Modal selection from a menu.
 * Arguments:
 *  - menu - the menu
 *  - cursor - the row in which the cursor should start.
 *  - no_handle - Don't handle these events. ( bitwise or of event_class)
 *     0 - return values below are limited to the set below.
 * Additional examples:
 *     EVT_MOVE - return values also include menu movement.
 *     EVT_CMD  - return values also include command IDs to process.
 * Returns: an event, possibly requiring further handling.
 * Return values:
 *  EVT_SELECT - success. key_event::index is set to the cursor position.
 *      *cursor is also set to the cursor position.
 *  EVT_OK  - success. A command event was handled.
 *     *cursor is also set to the associated row.
 *  EVT_BACK - no selection; go to previous menu in hierarchy
 *  EVT_ESCAPE - Abandon modal interaction
 *  EVT_KBRD - An unhandled keyboard event
 */
key_event menu_select(menu_type *menu, int *cursor, int no_handle)
{
	key_event ke;

	menu->cursor = *cursor;

	/* Menu shall not handle these */
	no_handle |= (EVT_SELECT | EVT_BACK | EVT_ESCAPE | EVT_STOP);
	no_handle &= ~(EVT_REFRESH);

	if (!menu->object_list)
		menu->filter_count = menu->count;

	ke.type = EVT_REFRESH;
	(void)run_event_loop(&menu->target, FALSE, &ke);

	/* Stop on first unhandled event. */
	while (!(ke.type & no_handle))
	{
		ke = run_event_loop(&menu->target, FALSE, 0);

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

/*
 * The menu skin registry.  In the unlikely event you need to register
 * more skins, make the array bigger.
 */
static menu_skin const *menu_skin_reg[20] =
{
	&scroll_skin,
	&column_skin,
	&key_select_skin,
	0
};

/* 
 * The menu row-iterator registry.
 * Note that there's no need to register "anonymous" row iterators, that is,
 * iterators always accessed by address, and not available in pref files.
 */
static menu_iter const *menu_iter_reg[20] =
{
	&menu_iter_event,
	&menu_iter_item,
	&menu_iter_string,
	0
};


const menu_iter *find_menu_iter(menu_iter_id id)
{
	int i;
	for (i = 0; i < N_ELEMENTS(menu_iter_reg) && menu_iter_reg[i]; i++)
	{
		if (menu_iter_reg[i]->id == id)
			return menu_iter_reg[i];
	}
	return NULL;
}

const menu_skin *find_menu_skin(skin_id id)
{
	int i;
	for (i = 0; i < N_ELEMENTS(menu_skin_reg) && menu_skin_reg[i]; i++)
	{
		if (menu_skin_reg[i]->id == id)
			return menu_skin_reg[i];
	}
	return NULL;
}

void add_menu_skin(const menu_skin *skin, skin_id id)
{
	int i;

	assert(skin->id == id);
	for (i = 0; i < N_ELEMENTS(menu_skin_reg) && menu_skin_reg[i]; i++)
		assert(skin->id != menu_skin_reg[id]->id);

	if (i == N_ELEMENTS(menu_skin_reg))
		quit("too many registered skins!");

	menu_skin_reg[i] = skin;
}

void add_menu_iter(const menu_iter * iter, menu_iter_id id)
{
	int i;

	assert(iter->id == id);
	for (i = 0; i < N_ELEMENTS(menu_iter_reg) && menu_iter_reg[i]; i++)
		assert(iter->id != menu_iter_reg[id]->id);

	if (i == N_ELEMENTS(menu_iter_reg))
		quit("too many registered iters!");

	menu_iter_reg[i] = iter;
}

/*
 * Set the filter to a new value.
 * IMPORTANT: The filter is assumed to be owned by the menu.
 * To remove a filter that is not owned by the menu, use:
 * menu_set_filter(m, NULL, m->count);
 */
void menu_set_filter(menu_type *menu, const int object_list[], int n)
{
	menu->object_list = object_list;
	menu->filter_count = n;
}

/* Delete the filter */
/* HACK: returns old filter for possible destruction */
/* as: FREE(menu_remove_filter(m)); */
void menu_release_filter(menu_type *menu)
{
	if (menu->object_list)
		FREE((void *)menu->object_list);
	menu->object_list = 0;
	menu->filter_count = menu->count;
}

void menu_set_id(menu_type *menu, int id)
{
	menu->target.self.object_id = id;
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


bool menu_init2(menu_type *menu, const menu_skin *skin,
           const menu_iter * iter, const region *loc)
{
	/* VTAB */
	*((panel_type *) menu) = menu_target;
	menu->target.self.object = menu;
	menu->target.is_modal = TRUE;
	menu->row_funcs = iter;
	menu->skin = skin;
	menu->target.self.events.evt_flags = (EVT_MOUSE | EVT_REFRESH | EVT_KBRD);

	if (menu->count && !menu->object_list) menu->filter_count = menu->count;

	if (!loc) loc = &SCREEN_REGION;


	menu_layout(menu, loc);

	/* TODO:  Check for collisions in selections & command keys here */
	return TRUE;
}

bool menu_init(menu_type *menu, skin_id skin_id,
          menu_iter_id iter_id, const region *loc)
{
	const menu_skin *skin = find_menu_skin(skin_id);
	const menu_iter *iter = find_menu_iter(iter_id);
	if (!iter || !skin)
	{
		msg_print(format
				  ("could not find menu VTAB (%d, %d)!", skin_id, iter_id));
		return FALSE;
	}
	return menu_init2(menu, skin, iter, loc);
}

void menu_destroy(menu_type *menu)
{
	if (menu->object_list) FREE((void *)menu->object_list);
}
