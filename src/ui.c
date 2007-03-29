/* File ui.c */

/*
 * Copyright (c) 2007 Pete Mack and others
 * This code released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 *
 */

#include "angband.h"


const char default_choice[] =
	"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char lower_case[] = "abcdefghijklmnopqrstuvwxyz";

int jumpscroll = 0;
int menu_width = 23;

void region_erase(const region *loc)
{
	int i = 0;
	int w = loc->width;
	int h = loc->page_rows;
	if(loc->width <= 0 || loc->page_rows <= 0) {
		Term_get_size(&w, &h);
		if(loc->width <= 0) w -= loc->width;
		if(loc->page_rows <= 0) h -= loc->page_rows;
	}
	for(i = 0; i < h; i++) {
		Term_erase(loc->col, loc->row+i, w);
	}
}

static void display_menu_row(menu_type *menu, const int object_list[], int pos,
								int top, bool cursor, int row, int col, int width);

/* Display an event, with posssble preference overrides */

static void display_event_aux(event_action *event, int menuID, byte color,
							int row, int col, int wid)
{
	/* TODO: add preference support */
	/* TODO: wizard mode should show more data */
	Term_erase(col, row, wid);
	if(event->name)
		Term_putstr(col, row, wid - 1, color, event->name);
}

static bool inside(const region *loc, const key_event *key)
{
	if((loc->col > key->mousex) || (loc->col + loc->width <= key->mousex))
		return FALSE;
	if((loc->row > key->mousey) || (loc->row + loc->page_rows <= key->mousey))
		return FALSE;
	return TRUE;
}

/*======================= MN_EVT HELPER FUNCTIONS ====================== */

static void display_event(menu_type *menu, int oid, bool cursor, 
							int row, int col, int width)
{
	event_action *evts = (event_action*)menu->menu_data;
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];
	display_event_aux(&evts[oid], menu->menuID, color, row, col, width);
}

/* act on selection only */
/* Return: true if handled. */
static bool handle_menu_item_event(char cmd, const void *db, int oid)
{
	event_action *evt = &((event_action*)db)[oid];
	if(cmd == '\xff' && evt->action) {
		evt->action(evt->data, evt->name);
		return TRUE;
	}
	else if(cmd == '\xff')
		return TRUE;
	return FALSE;
}

static bool valid_menu_event(menu_type *menu, int oid)
{
	event_action *evts = (event_action *) menu->menu_data;
	return (NULL != evts[oid].name);
}

/* Virtual function table for action_events */
static const menu_class class_menu_event = {
	MN_EVT,
	0,
	valid_menu_event,
	display_event,
	handle_menu_item_event
};

/*======================= MN_ACT HELPER FUNCTIONS ====================== */

static char tag_menu_item(menu_type *menu, int oid)
{
	menu_item *items = (menu_item *) menu->menu_data;
	return items[oid].sel;
}

static void display_menu_item(menu_type *menu, int oid, bool cursor,
								int row, int col, int width)
{
	menu_item *items = (menu_item *) menu->menu_data;

	byte color =
		curs_attrs[!(items[oid].flags & (MN_GRAYED|MN_DISABLED))][0 != cursor];
	display_event_aux(&items[oid].evt, menu->menuID, color, row, col, width);
}

/* act on selection only */
static bool handle_menu_item(char cmd, const void *db, int oid)
{
	if(cmd == '\xff')
	{
		menu_item *item  = &((menu_item*)db)[oid];
		if(item->flags & MN_DISABLED) return TRUE; 
		if(item->evt.action)
			item->evt.action(item->evt.data, item->evt.name);
		if(item->flags & MN_SELECTABLE) {
			item->flags ^= MN_SELECTED;
		}
		return TRUE;
	}
	return FALSE;
}

static bool valid_menu_item(menu_type *menu, int oid)
{
	menu_item *items = (menu_item *) menu->menu_data;
	return (NULL != items[oid].evt.name);
}

/* Virtual function table for menu items */
static menu_class class_menu_item = {
	MN_ACT,
	tag_menu_item,
	valid_menu_item,
	display_menu_item,
	handle_menu_item
	
};


/* ================== SKINS ============== */


/* Scrolling menu */
static int scrolling_get_cursor(int row, int col, int n,
									int top, region *loc)
{
	int cursor = row - loc->row + top;
	if(cursor < top) cursor = top-1;
	else if(cursor > loc->page_rows) cursor = loc->page_rows;
	if(cursor < 0) cursor = 0;
	else if(cursor >= n) cursor = n-1;

	return cursor;
}


void display_scrolling (menu_type *menu, const int object_list[], int n,
						int cursor, int *top, region *loc)
{
	int col = loc->col;
	int row = loc->row;
	int rows_per_page = loc->page_rows;
	int i;

	if(cursor < *top)
		*top = cursor - jumpscroll;
	if(cursor >= *top + rows_per_page)
		 *top = cursor - rows_per_page-1 + jumpscroll;
	if(*top > n - rows_per_page) *top = n - rows_per_page;
	if(*top < 0) *top = 0;

	for(i = 0; i < rows_per_page && i < n; i++)
	{
		bool is_curs = (cursor == i - *top);
		display_menu_row(menu, object_list, i, *top, is_curs, row+i, col, loc->width);
	}
	if(cursor >= 0) {
		Term_gotoxy(col, row + cursor-*top);
	}
}

/* Virtual function table for scrollable menu skin */
static const menu_skin scroll_skin = {
	MN_SCROLL,
	scrolling_get_cursor,
	display_scrolling
};
	

/* multi-column menu */
static int columns_get_cursor(int row, int col, int n, int top, region *loc)
{
	int rows_per_page = loc->page_rows;
	int colw = loc->width / (n + rows_per_page-1)/rows_per_page;
	int cursor = row + rows_per_page * (col - loc->col)/colw;

	if(cursor < 0) cursor = 0; /* assert: This should never happen */
	if(cursor >= n) cursor = n-1;

	return cursor;
}

void display_columns (menu_type *menu, const int object_list[], int n,
						int cursor, int *top, region *loc)
{
	int c, r;
	int w, h;
	int col = loc->col;
	int row = loc->row;
	int rows_per_page = loc->page_rows;
	int cols = (n + rows_per_page  - 1) / rows_per_page;
	int colw = menu_width;
	Term_get_size(&w, &h);
	if(colw * cols > w - col) 
		colw = (w-col) /cols;

	for(c = 0; c < cols; c++) {
		for(r = 0; r < rows_per_page; r++) {
			int pos = c*rows_per_page + r;
			bool is_cursor = (pos == cursor);
			display_menu_row(menu, object_list, pos, 0, is_cursor,
								row+r, col+c*colw, colw);
		}
	}
}

/* Virtual function table for multi-column menu skin */
static const menu_skin column_skin = {
	MN_COLUMNS,
	columns_get_cursor,
	display_columns
};

/* ================== IMPLICIT MENU FOR KEY SELECTION ================== */

static void display_nothing (menu_type *menu, const int object_list[], int n,
		                    int cursor, int *top, region *loc)
{
}

static int no_cursor(int row, int col, int n, int top, region *loc)
{
	return -1;
}

static const menu_skin key_select_skin = {
	MN_KEY_ONLY,
	no_cursor,
	display_nothing,
};



/* ================== GENERIC HELPER FUNCTIONS ============== */

static char dummy_get_tag(menu_type *menu, int cursor)
{
	return menu->selections[cursor];
}

static bool is_valid_row(menu_type *menu, const int object_list[], int cursor)
{
	int oid = cursor;
	if(cursor < 0 || cursor > menu->count) return FALSE;
	if(!menu->valid_row) return TRUE;
	if(object_list) {
		oid = object_list[cursor]; 
	}
	return menu->valid_row(menu, oid);
}

static int get_cursor_key(menu_type *menu, const int object_index[], int n,
							int top, char key)
{
	int i;
	if(menu->flags & MN_REL_TAGS) {
		for(i = 0; i < n; i++) {
			char c = menu->get_tag(menu, i);
			if(c && c == key)
				return i + top;
		}	
	}
	for(i = 0; i < n; i++) {
		int oid = object_index ? object_index[i] : i;
		char c = menu->get_tag(menu, oid);
		if(c && c == key)
			return i;
	}
	return -1;
}

/*
 * Event handler wrapper function
 * Filters unhandled keys & conditions 
 */
static bool handle_menu_key(char cmd, menu_type *menu, int oid)
{
	int flags = menu->flags;
	if(flags & MN_NO_ACT) return FALSE;

	if(isspace(cmd) && (flags & MN_PAGE)) return FALSE;

	if(cmd == ESCAPE) return FALSE;
	if(!cmd == '\xff' && (!menu->cmd_keys || !strchr(menu->cmd_keys, cmd)))
			return FALSE;

	if(menu->handler)
		return menu->handler(cmd, menu->menu_data, oid);
	return FALSE;
}


static void display_menu_row(menu_type *menu, const int object_list[], int pos,
								int top, bool cursor, int row, int col, int width)
{
	int flags = menu->flags;
	char sel = 0;
	int oid = pos;
	if(object_list) oid = object_list[oid];
	if(!(flags & MN_NO_TAGS) && menu->get_tag) {
		if(flags & MN_REL_TAGS) {
			sel = menu->get_tag(menu, pos - top);
		}
		else 
			sel = menu->get_tag(menu, MN_PVT_TAGS ? oid : pos);
	}
	if(sel) {
		/* TODO: CHECK FOR VALID */
		byte color = curs_attrs[CURS_KNOWN][0 != (cursor)];
		c_prt(color, format("%c) ", sel), row, col);
		col += 3;
		width -= 3;
	}
	menu->display_label(menu, oid, cursor, row, col, width);
}


key_event get_menu_choice(menu_type *menu, const int object_list[], int n,
							int *cursor, int top, region *loc)
{
	key_event ke;

retry:
 	ke = inkey_ex();
	switch(ke.type)
	{
		/* Use event handler here, when it becomes available. */
		case EVT_MOUSE:
		{
			int m_curs;
			if(!inside(loc, &ke)) {
				/* TODO: check tracking regions here */
			}
	
				/* Not handled -- used for heirarchical menus */
			if(ke.mousex < loc->col) {
				ke.type = EVT_BACK;
				break;
			}

			m_curs = menu->skin->get_cursor(ke.mousey,ke.mousex, n, top, loc);
			if(!is_valid_row(menu, object_list, m_curs))
				goto retry;

			ke.index = m_curs;
			*cursor = m_curs;
			if(*cursor == m_curs || !(menu->flags & MN_DBL_TAP))
			{
				/* menu selection event*/
				ke.type = EVT_SELECT;
				break;
			}
			/* menu movement event */
			ke.type = EVT_MOVE;
			break;
		}

		/* These could be done with a keyboard handler--maybe overkill */
		case EVT_KBRD:
		{
			int dir;
			if(menu->cmd_keys && strchr(menu->cmd_keys, ke.key))
				break;
			if(!(menu->flags & MN_NO_TAGS)) {
				int c = get_cursor_key(menu, object_list, n, top, ke.key);
				if(c > 0 && !is_valid_row(menu, object_list, c))
					goto retry;
				else if(c >= 0) {
					*cursor = c;
					ke.type = EVT_SELECT;
					ke.index = c;
					ke.key = '\xff';
					break;
				}
			}
			if(menu->flags & MN_NO_CURSOR)
				break;
	
			/* cursor movement */
			dir = target_dir(ke.key);
			if(dir == 2 ) {
				int ind;
				ke.type = EVT_MOVE;
				ke.key = '\xff';
				for(ind = *cursor + 1;
					ind < n && !is_valid_row(menu, object_list, ind);
					ind++);
				if(ind == n) ke.index = n;
				else *cursor = ke.index = ind;
			}
			else if(dir == 8) {
				int ind;
				ke.type = EVT_MOVE;
				ke.key = '\xff';
				for(ind = *cursor - 1;
					ind >=0 && !is_valid_row(menu, object_list, ind);
					ind--);
				if(ind < 0) ke.index = -1;
				else *cursor = ke.index = ind;
			}
			else if(dir == 4) {
				ke.type = EVT_BACK;
				ke.key = '\xff';
				ke.index = *cursor;
			}
			else if(dir == 6) {		/* Selection event */
				ke.type = EVT_SELECT;
				ke.key = '\xff';
				ke.index = *cursor;
			}
			break;
		}
		default:
			/* And you may ask yourself--well ... how did I get here? */
			/* And you may ask yourself--Am I right? ... am I wrong? */
			/* And you may tell yourself--My god! ... what have I done? */
			break;
	}

	return ke;
}


key_event menu_select(menu_type *menu, const int object_list[], int n,
				int *cursor, region loc)
{
	key_event ke;
	int top = 0;
	region active;

	if(!menu->skin)
		menu_init(menu);

	if (n < 0) n = menu->count;
	if(loc.page_rows <= 0 || loc.width <=0)
	{
		int w, h;
		Term_get_size(&w, &h);
		/* HACK: This should be part of the menu definition */
		if(loc.page_rows <= 0)
			loc.page_rows = h - loc.row + loc.page_rows;
		if(loc.width <= 0) 
			loc.width = w - loc.col + loc.width;
	}

	if(loc.width == 1) loc.width = menu_width;

	active = loc;

	if(menu->title) {
		active.row += 2;
		active.page_rows -= 2;
		/* TODO: handle small screens */
		active.col += 4;
	}
	if(menu->prompt) {
		active.page_rows--;
	}

	for(;;)
	{
		int oid = *cursor;
		int i;
		if(object_list) oid = object_list[oid];
		/* Clear owned region */
		for(i = loc.row; i < loc.page_rows; i++)
			Term_erase(loc.col, loc.row+i, loc.width);

		if(menu->title) prt(menu->title, loc.row, loc.col);
		if(menu->prompt) prt(menu->prompt, loc.row+loc.page_rows-1, loc.col);
		
		menu->skin->display_list(menu, object_list, n, *cursor, &top, &active);
		/* if(menu->flags & MN_DISPLAY_ONLY)
			return;
		*/
		if(menu->browse_hook) {
			menu->browse_hook(oid, &loc);
		}

		ke = get_menu_choice(menu, object_list, menu->count, cursor, top, &active);
		if(*cursor != oid && ke.type == EVT_SELECT || menu->flags == MN_ONCE)
		{
			/* refresh prior to return */
			menu->skin->display_list(menu, object_list, n, *cursor, &top, &active);
		}
		if(ke.key == ESCAPE) return ke;
		if(menu->flags & MN_ONCE) return ke;
		switch(ke.type)
		{
			case EVT_BACK:
				return ke;
	
			case EVT_MOVE:
				continue;
			case EVT_SELECT:
				if(oid != *cursor) /* refresh */
					menu->skin->display_list(menu, object_list, n, *cursor, &top, &active);
		}
		if(menu->flags & MN_NO_ACT)
			return ke;

		if(handle_menu_key(ke.key, menu, *cursor))
			return ke;
	}
}


/* ================== CREATION & INITIALIZATION ============== */

/* This may be overkill */
menu_skin const *menu_skin_reg[MN_MAX_SKIN] =
{
	&scroll_skin,
	&column_skin,
	0
};

menu_class const *menu_class_reg[15] = 
{
	&class_menu_event,
	&class_menu_item,
	0
};

void menu_set_class(menu_type *menu, const menu_class *def)
{
	menu->flags |= def->flag;
	menu->get_tag = def->get_tag;
	menu->valid_row = def->valid_row;
	menu->handler = def->handler;
	menu->display_label = def->display_row;
}

bool menu_init(menu_type *menu)
{
	int i;
	int skinID = menu->flags & MN_MAX_SKIN;
	int classID = menu->flags & MN_CLASS;
	if(skinID != MN_USER) {
		menu->skin = &scroll_skin;
	}
	for(i = 0; menu_skin_reg[i]; i++)
	{
		if(menu_skin_reg[i]->flag == skinID) 
			menu->skin = menu_skin_reg[i];
	}
	for(i = 0; menu_class_reg[i]; i++)
	{
		if(menu_class_reg[i]->flag == classID) {
			menu_set_class(menu, menu_class_reg[i]);
		}
	}
	if((menu->selections && !(menu->flags & MN_PVT_TAGS))
			|| (menu->flags & MN_REL_TAGS))
	{
		menu->get_tag = dummy_get_tag;
	}
	/* TODO:  Check for collisions in selections & command keys here */
	return TRUE;
}

