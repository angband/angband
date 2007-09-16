/*
 * File: main-gtk.c
 * Purpose: GTK port for Angband
 *
 * Copyright (c) 2000-2007 Robert Ruehlmann, Shanoah Alkire
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

#ifdef USE_GTK
#include "main-gtk.h"
/* 
 *Add a bunch of debugger message, to trace where problems are. 
 */
/*#define GTK_DEBUG*/

/* 
 *Write all debugger messages to the command line, as well as the debugger window. 
*/
/*#define VERBOSE_DEBUG*/

/*
 * This will disable testing features.
 */
 /*#define DISABLE_GTK_TESTING*/

static int max_win_width(term_data *td)
{
	return (255 * td->font.w);
}

static int max_win_height(term_data *td)
{
	return(255 * td->font.h);
}

static int drawing_win_width(term_data *td)
{
	return (td->cols * td->font.w);
}

static int drawing_win_height(term_data *td)
{
	return(td->rows * td->font.h);
}

static int row_in_pixels(term_data *td, int x)
{
	return(x * td->font.w);
}

static int col_in_pixels(term_data *td, int y)
{
	return(y * td->font.h);
}

/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int * const x, int * const y, const int ox, const int oy)
{
	term_data *td = (term_data*)(Term->data);

	(*x) = (ox / td->font.w);
	(*y) = (oy / td->font.h);
}

static GdkRectangle cairo_rect_to_gdk(cairo_rectangle_t *r)
{
	GdkRectangle s;
	
	s.x = r->x;
	s.y = r->y;
	s.width = r->width;
	s.height = r->height;
	
	return(s);
}

static cairo_rectangle_t gdk_rect_to_cairo(GdkRectangle *r)
{
	cairo_rectangle_t s;
	
	s.x = r->x;
	s.y = r->y;
	s.width = r->width;
	s.height = r->height;
	
	return(s);
}

static void invalidate_rect(term_data *td, cairo_rectangle_t r)
{
	GdkRectangle s;
	
	s = cairo_rect_to_gdk(&r);
	gdk_window_invalidate_rect(td->drawing_area->window, &s, TRUE);
}

static void term_data_resize(term_data *td)
{
	term *old = Term;
	/* Activate the term */
	Term_activate(&td->t);

	Term_resize(td->cols, td->rows);
	
	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();
	Term_key_push(KTRL('R'));
	Term_activate(old);
}
/* 
 * Get the position of the window and save it. Gtk being what it is, this is a major hack.
 * It'd be nice to replace this with something cleaner...
 *
 * Given that I've majorly revised the window code since this was added, some of it 
 * may no longer be neccessary...
 */
static void get_size_pos(term_data *td)
{
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling Get_size_pos");
	#endif
	
	int x = 0, y = 0, w = 0, h = 0, ww = 0, hh = 0;
	GdkRectangle r;
	
	if (GTK_WIDGET_VISIBLE(td->window) && GTK_WIDGET_VISIBLE(td->drawing_area))
	{
		
		gdk_window_get_frame_extents(td->window->window, &r);
		x = r.x;
		y = r.y;
		
		gtk_window_get_size(GTK_WINDOW(td->window), &w, &h);
		
		/* The first few times this is called, both getting the drawing area size,
		     and the main window size go all screwy, but not at the same time,
		     so pick the more reasonable of the two.*/
		gtk_widget_get_size_request(GTK_WIDGET(td->drawing_area), &ww, &hh);
		if (((abs(h - td->size.h) < 20) && (abs(w - td->size.w) < 20)) || ((abs(td->size.h - h) < 20) && (abs(td->size.w - w) < 20)))
		{
			hh = h;
			ww = w;
		}
		
		if ((hh == td->size.h) && (ww == td->size.w))
		{
			h = hh;
			w = ww;
		}
		if ((h <= 100) && (hh > 100)) h = hh;
		if ((h > 100) && (hh <= 100)) hh = h;
		
		/* Take the titlebar into account, but only if this looks like the titlebar height, and it isn't already right. */
		
		if ((abs(y - td->location.y) < 20) || (abs(td->location.y - y) < 20))
		{
			y = td->location.y;
		}
		else if (y != td->location.y)
		{
			if ((r.height - h) < 50) 
			{
				y = y + (r.height - h);
			}
		}
		
		if ((abs(x - td->location.x) < 20) || (abs(td->location.x - x) < 20))
		{
			x = td->location.x;
		}
		/* Don't randomly reposition at the top of the screen, either */
		if (x != 0) td->location.x = x;
		if (y != 0) td->location.y = y;
			
		/* Hurrah for preventing weird glitches with workarounds.
		    I honestly have no idea why the windows occassionally
		    ignore my size requests, and show up as 100x100 or 200x190. */
		if (((w != 200) && (h != 190)) || ((w <= 100) && (h <= 100)))
		{
			/*int cols = td->cols;
			int rows = td->rows;*/
			
			if (w != 0) 
			{
				td->size.w = w;
				td->cols = td->size.w / td->font.w;
			}
			if (h != 0) 
			{
				td->size.h = h;
				td->rows= td->size.h / td->font.h;
			}
			/*if ((cols != td->cols) || (rows != td->rows))
				term_data_resize(td);*/
		}
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Window %s: Location(%d/%d), Size(%d/%d)", td->name, td->location.x, td->location.y, td->size.w, td->size.h);
		#endif
	}
}

static void get_xtra_pos(xtra_win_data *xd)
{
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling Get_xtra_pos");
	#endif
	
	int x = 0, y = 0, w = 0, h = 0;
	GdkRectangle r;
	
	if (GTK_WIDGET_VISIBLE(xd->win))
	{
		
		gdk_window_get_frame_extents(xd->win->window, &r);
		x = r.x;
		y = r.y;
		
		gtk_window_get_size(GTK_WINDOW(xd->win), &w, &h);
		
		if (x != 0)  xd->location.x = r.x;
		if (y != 0)  xd->location.y = r.y;
		if (w != 0) xd->size.w = w;
		if (h != 0)  xd->size.h = h;
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "XTRA-Window %s: Location(%d/%d), Size(%d/%d)", xd->name, xd->location.x, xd->location.y, xd->size.w, xd->size.h);
		#endif
		
		xd->visible = TRUE;
	}
	else
		xd->visible = FALSE;
}

static void set_window_defaults(term_data *td)
{
	GdkGeometry geo;
	
	geo.width_inc = td->font.w;
	geo.height_inc = td->font.h;
	
	/*geo.max_width = max_win_width();
	geo.max_height = max_win_height();*/
	
	/*geo.min_width = td->font.w;
	geo.min_height = td->font.h;*/
	
	gtk_window_set_geometry_hints(GTK_WINDOW(td->window), td->drawing_area, &geo, 
	GDK_HINT_RESIZE_INC | GDK_HINT_MAX_SIZE); /* GDK_HINT_POS | | GDK_HINT_MIN_SIZE*/
}

static void set_window_size(term_data *td)
{
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling Set window size.");
	gtk_log_fmt(TERM_WHITE, "Size for %s is going to be resized to (%d/%d)", td->name, td->size.w, td->size.h);
	#endif
	
	gtk_window_set_position(GTK_WINDOW(td->window),GTK_WIN_POS_NONE);
	
	if MAIN_WINDOW(td)
		gtk_widget_set_size_request(GTK_WIDGET(td->drawing_area), td->size.w, td->size.h);
	else
		gtk_window_resize(GTK_WINDOW(td->window), td->size.w, td->size.h);
	
	gtk_window_move(GTK_WINDOW(td->window), td->location.x, td->location.y);
}

static void set_xtra_window_size(xtra_win_data *xd)
{
	gtk_window_set_position(GTK_WINDOW(xd->win),GTK_WIN_POS_NONE);
	if ((xd->size.w > 0) && (xd->size.h > 0))
		gtk_window_resize(GTK_WINDOW(xd->win), xd->size.w, xd->size.h);
	gtk_window_move(GTK_WINDOW(xd->win), xd->location.x, xd->location.y);
}

gboolean show_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	const char *win_name;
	int t;
	
	win_name = (char*)gtk_widget_get_name(GTK_WIDGET(widget));	
	sscanf(win_name, "term_window_%d", &t);
	
	term_data *td = &data[t];
	set_window_size(td);
	return(FALSE);
}

gboolean configure_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	
	term_data *td = user_data;
	
	get_size_pos(td);
	return(FALSE);
}

gboolean configure_xtra_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	xtra_win_data *xd = user_data;
	
	get_xtra_pos(xd);
	return(FALSE);
}

/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, 0, 0, max_win_width(td), max_win_height(td));
	cairo_clear(td->cr, r, TERM_DARK);
	invalidate_rect(td, r);

	/* Success */
	return (0);
}

/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	/* Set dimensions */
	init_cairo_rect(&r, row_in_pixels(td, x), col_in_pixels(td, y), (td->font.w * n), (td->font.h));
	cairo_clear(td->cr, r, TERM_DARK);
	invalidate_rect(td, r);

	/* Success */
	return (0);
}

/*
 * Draw some textual characters.
 */
static errr Term_text_gtk(int x, int y, int n, byte a, cptr s)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	/* Set dimensions */
	init_cairo_rect(&r, row_in_pixels(td, x), col_in_pixels(td, y),  td->font.w * n, td->font.h);
	
	/* Clear the line */
	Term_wipe_gtk(x, y, n);
	
	/* Draw the text */
	draw_text(td->cr, &td->font, x, y, n, a, s);
	
	invalidate_rect(td, r);
			
	/* Success */
	return (0);
}

static errr Term_pict_gtk(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	/* Set dimensions */
	init_cairo_rect(&r, row_in_pixels(td, x), col_in_pixels(td, y),  td->font.w * n, td->font.h);
	
	/* Clear the line */
	Term_wipe_gtk(x, y, n);
	
	/* I can't count on at all the td fields names being the same for everything that uses cairo-utils,
	    hence a lot of variables being passed. */
	draw_tiles(td->cr, x, y, n, ap, cp, tap, tcp, td->font, td->tile);

	invalidate_rect(td, r);
			
	/* Success */
	return (0);
}

static errr Term_flush_gtk(void)
{
	gdk_flush();
	/* XXX */
	return (0);
}
static errr Term_fresh_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, 0, 0, max_win_width(td), max_win_height(td));
	invalidate_rect(td, r);
	gdk_window_process_updates(td->window->window, 1);
	/* XXX */
	return (0);
}
/*
 * Handle a "special request"
 */
static errr Term_xtra_gtk(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE: 
			return (0);

		/* Flush the output */
		case TERM_XTRA_FRESH: 
			return (Term_fresh_gtk());

		/* Process random events */
		case TERM_XTRA_BORED: 
			return (CheckEvent(0));

		/* Process Events */
		case TERM_XTRA_EVENT: 
			return (CheckEvent(v));

		/* Flush the events */
		case TERM_XTRA_FLUSH: 
			return (Term_flush_gtk());

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: 
			return (0);

		/* Clear the screen */
		case TERM_XTRA_CLEAR: 
			return (Term_clear_gtk());

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
			if (v > 0) usleep(1000 * v);
			return (0);

		/* React to changes */
		case TERM_XTRA_REACT: 
			return (0);
	}

	/* Unknown */
	return (1);
}


static errr Term_curs_gtk(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;

	/* Set dimensions */
	init_cairo_rect(&r, row_in_pixels(td, x)+1, col_in_pixels(td, y) + 1,  (td->font.w) - 2, (td->font.h) - 2);
	cairo_cursor(td->cr, r, TERM_SLATE);
	invalidate_rect(td, r);
	
	/* Success */
	return (0);
}

/*
 * Hack -- redraw a term_data
 *
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td)
{
	term *old = Term;
	
	/* Activate the term passed to it, not term 0! */
	Term_activate(&td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();
	Term_activate(old);
}

static errr CheckEvent(bool wait)
{
	if (wait)
		gtk_main_iteration();
	else
		while (gtk_events_pending())
			gtk_main_iteration();

	return (0);
}

static bool save_game_gtk(void)
{
	if (game_in_progress && character_generated)
	{
		if (!inkey_flag)
		{
			gtk_log_fmt(TERM_WHITE, "You may not do that right now.");
			return(FALSE);
		}

		/* Hack -- Forget messages */
		msg_flag = FALSE;
		
		/* Save the game */
		#ifdef ZANGBAND
		do_cmd_save_game(FALSE);
		#else
		do_cmd_save_game();
		#endif 
	}
	
	return(TRUE);
}

static void hook_quit(cptr str)
{
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling hook_quit");
	#endif
	save_prefs();
	do_cmd_save_game();
	release_memory();
	gtk_exit(0);
}

gboolean quit_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling quit_event_handler");
	#endif
	if (save_game_gtk())
	{
		save_prefs();
		quit(NULL);
		gtk_exit(0);
		return(FALSE);
	}
	else
		return(TRUE);
}

gboolean destroy_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	quit(NULL);
	gtk_exit(0);
	return(FALSE);
}

gboolean hide_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	term_data *td = user_data;
	GtkWidget *menu_item;
	char item_name[20];
	
	if ((td != NULL) && (td->window == widget))
	{
	strnfmt(item_name, 16+1,  "term_menu_item_%i", td->number);
	menu_item = glade_xml_get_widget(gtk_xml, item_name);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), FALSE);
	}
	
	return(TRUE);
}

gboolean xtra_hide_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	xtra_win_data *xd = user_data;
	
	xd->visible = FALSE;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(xd->menu), FALSE);
	
	return(TRUE);
}

gboolean new_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (game_in_progress)
	{
		gtk_log_fmt(TERM_WHITE, "You can't start a new game while you're still playing!");
		return(TRUE);
	}
	else
	{
		/* We'll return NEWGAME to the game. */ 
		cmd.command = CMD_NEWGAME;
		return(FALSE);
	}
}

/*** Callbacks: font selector */

static void load_font_by_name(term_data *td, cptr font_name)
{	
	my_strcpy(td->font.name, font_name, sizeof(td->font.name));
	
	get_font_size(&td->font);
	td->size.w =drawing_win_width(td);
	td->size.h = drawing_win_height(td);
	
	if (td->window != NULL) 
	{
	/* Set defaults, and resize the window accordingly */
	set_window_defaults(td);
	set_window_size(td);
		
	create_term_cairo(td);
	}
}

void set_term_font(GtkFontButton *widget, gpointer user_data)   
{
	const char *font_name, *s;
	int t;
	
	s = (char*)gtk_widget_get_name(GTK_WIDGET(widget));
	sscanf(s, "term_font_%d", &t);
	
	term_data *td=&data[t];
	
	font_name = gtk_font_button_get_font_name(widget);
	load_font_by_name(td, font_name);
}

void set_xtra_font(GtkFontButton *widget, gpointer user_data)   
{
	const char *temp, *s;
	int t;
	
	s = (char*)gtk_widget_get_name(GTK_WIDGET(widget));
	sscanf(s, "xtra_font_%d", &t);
	
	xtra_win_data *xd=&xdata[t];
	
	temp = gtk_font_button_get_font_name(widget);
	my_strcpy(xd->font.name, temp, sizeof(xd->font.name));
	gtk_widget_modify_font(GTK_WIDGET(xd->text_view), pango_font_description_from_string(xd->font.name));
	
}

/*** Callbacks: savefile opening ***/

/* Filter function for the savefile list */
static gboolean file_open_filter(const GtkFileFilterInfo *filter_info, gpointer data)
{
	const char *name = filter_info->display_name;

	(void)data;

	/* Count out known non-savefiles */
	if (strcmp(name, "Makefile") == 0 ||
	    strcmp(name, "delete.me") == 0)
	{
		return FALSE;
	}

	/* Let it pass */
	return TRUE;
}

/*
 * Open/Save Dialog box
 */
static bool save_dialog_box(bool save)
{
	GtkWidget *selector_wid;
	GtkFileChooser *selector;
	bool accepted;
	
	char buf[1024];
	const char *filename;

	/* Forget it if the game is in progress */
	/* XXX Should disable the menu entry XXX */
	if (game_in_progress && !save)
	{
		gtk_log_fmt(TERM_WHITE, "You can't open a new game while you're still playing!");
		return(FALSE);
	}
			
	if ((!game_in_progress || !character_generated) && save)
	{
		gtk_log_fmt(TERM_WHITE, "You can't save a new game unless you're still playing!");
		return(FALSE);
	}

	if (!inkey_flag && save)
	{
		gtk_log_fmt(TERM_WHITE, "You may not do that right now.");
		return(FALSE);
	}
	
	/* Create a new file selector dialogue box, with no parent */
	if (!save)
	{
	selector_wid = gtk_file_chooser_dialog_new("Select a savefile", NULL,
	                                       GTK_FILE_CHOOSER_ACTION_OPEN,
	                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	                                       NULL);
	}
	else
	{
	selector_wid = gtk_file_chooser_dialog_new("Save your game", NULL,
	                                       GTK_FILE_CHOOSER_ACTION_SAVE,
	                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
	                                       NULL);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (selector_wid), op_ptr->full_name);
	}
	/* For convenience */
	selector = GTK_FILE_CHOOSER(selector_wid);
	gtk_file_chooser_set_do_overwrite_confirmation (selector, TRUE);;
	
	/* Get the current directory (so we can find lib/save/) */
	filename = gtk_file_chooser_get_current_folder(selector);
	path_build(buf, sizeof buf, filename, ANGBAND_DIR_SAVE);
	gtk_file_chooser_set_current_folder(selector, buf);

	/* Restrict the showing of pointless files */
	GtkFileFilter *filter;
	filter = gtk_file_filter_new();
	gtk_file_filter_add_custom(filter, GTK_FILE_FILTER_DISPLAY_NAME, file_open_filter, NULL, NULL);
	gtk_file_chooser_set_filter(selector, filter);
	
	accepted = (gtk_dialog_run(GTK_DIALOG(selector_wid)) == GTK_RESPONSE_ACCEPT);
	
	/* Run the dialogue */
	if (accepted)
	{
		/* Get the filename, copy it into the savefile name */
		filename = gtk_file_chooser_get_filename(selector);
		my_strcpy(savefile, filename, sizeof(savefile));
	}
		
	/* Destroy it now that we're done with it */
	gtk_widget_destroy(selector_wid);
	
	/* Done */
	return accepted;
}

void open_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	bool accepted;
	
	accepted = save_dialog_box(FALSE);
	
	/* Set the command now so that we skip the "Open File" prompt. */ 
	if (accepted) cmd.command = CMD_LOADFILE; 
	
	/* Done */
	return;
}
gboolean save_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	bool accepted;
	
	accepted = save_dialog_box(TRUE);
	
	if (accepted)
	{
		Term_flush();
		save_game_gtk();
	}
	
	/* Done */
	return(FALSE);
}
gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	save_game_gtk();
	return (FALSE);
}

gboolean keypress_event_handler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	int i, mc, ms, mo, mx;

	char msg[128];

	/* Extract four "modifier flags" */
	mc = (event->state & GDK_CONTROL_MASK) ? TRUE : FALSE;
	ms = (event->state & GDK_SHIFT_MASK) ? TRUE : FALSE;
	mo = (event->state & GDK_MOD1_MASK) ? TRUE : FALSE;
	mx = (event->state & GDK_MOD3_MASK) ? TRUE : FALSE;

	/*
	 * Hack XXX
	 * Parse shifted numeric (keypad) keys specially.
	 */
	if ((event->state == GDK_SHIFT_MASK) &&
	    (event->keyval >= GDK_KP_0) && (event->keyval <= GDK_KP_9))
	{
		/* Build the macro trigger string */
		strnfmt(msg, -1, "%cS_%X%c", 31, event->keyval, 13);
		
		/* Enqueue the "macro trigger" string */
		for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

		/* Hack -- auto-define macros as needed */
		if (event->length && (macro_find_exact(msg) < 0))
		{
			/* Create a macro */
			macro_add(msg, event->string);
		}

		return (TRUE);
	}

	/* Normal keys with no modifiers (except control) */
	if (event->length && !mo && !mx)
	{
		/* Enqueue the normal key(s) */
		for (i = 0; i < event->length; i++) Term_keypress(event->string[i]);

		if (!mc)
			return (TRUE); /* Not a control key, so the keypress is handled */
		else
			return(FALSE); /* Pass the keypress along, so the menus get it */
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch (event->keyval)
	{
		case GDK_Escape:
		{
			Term_keypress(ESCAPE);
			return (TRUE);
		}

		case GDK_Return:
		{
			Term_keypress('\r');
			return (TRUE);
		}

		case GDK_Tab:
		{
			Term_keypress('\t');
			return (TRUE);
		}

		case GDK_Delete:
		case GDK_BackSpace:
		{
			Term_keypress('\010');
			return (TRUE);
		}
		case GDK_Shift_L:
		case GDK_Shift_R:
		case GDK_Control_L:
		case GDK_Control_R:
		case GDK_Caps_Lock:
		case GDK_Shift_Lock:
		case GDK_Meta_L:
		case GDK_Meta_R:
		case GDK_Alt_L:
		case GDK_Alt_R:
		case GDK_Super_L:
		case GDK_Super_R:
		case GDK_Hyper_L:
		case GDK_Hyper_R:
		{
			/* Hack - do nothing to control characters */
			return (TRUE);
		}
	}

	/* Build the macro trigger string */
	strnfmt(msg, sizeof(msg), "%c%s%s%s%s_%X%c", 31,
	        mc ? "N" : "", ms ? "S" : "",
	        mo ? "O" : "", mx ? "M" : "",
	        event->keyval, 13);

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

	/* Hack -- auto-define macros as needed */
	if (event->length && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, event->string);
	}

	return (TRUE);
}

/* Prefs - copied straight from main-x11.c and hacked. */

static void save_prefs(void)
{
	ang_file *fff;
	int i;

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Saving Prefs.");
	#endif
	
	/* Open the settings file */
	fff = file_open(settings, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fff) return;

	/* Header */
	file_putf(fff, "# %s GTK preferences\n\n", VERSION_NAME);
	
	file_putf(fff, "[General Settings]\n");
	/* Graphics setting */
	file_putf(fff,"Tile set=%d\n", arg_graphics);

	/* New section */
	file_putf(fff, "\n");
	
	/* Save window prefs */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		if (!td->t.mapped_flag) continue;

		/* Header */
		file_putf(fff, "[Term window %d]\n", i);
		
		/* Term window specific preferences */
		file_putf(fff, "font=%s\n", td->font.name);
		file_putf(fff, "visible=%d\n", GTK_WIDGET_VISIBLE(td->window));
		file_putf(fff, "x=%d\n",  td->location.x);
		file_putf(fff, "y=%d\n", td->location.y);
		file_putf(fff, "width=%d\n", td->size.w);
		file_putf(fff, "height=%d\n", td->size.h);
		file_putf(fff, "cols=%d\n", td->cols);
		file_putf(fff, "rows=%d\n", td->rows);
		file_putf(fff, "tile width=%d\n", td->tile.w);
		file_putf(fff, "tile height=%d\n", td->tile.h);
		
		/* Footer */
		file_putf(fff, "\n");
	}
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
	
		/* Header */
		file_putf(fff, "[Extra window %d]", i);
		file_putf(fff, "# %s\n",xd->name);
		file_putf(fff, "font=%s\n", xd->font.name);
		file_putf(fff, "visible=%d\n", GTK_WIDGET_VISIBLE(xd->win));
		file_putf(fff, "x=%d\n", xd->location.x);
		file_putf(fff, "y=%d\n", xd->location.y);
		file_putf(fff, "width=%d\n", xd->size.w);
		file_putf(fff, "height=%d\n\n", xd->size.h);
	}

	/* Close */
	file_close(fff);
}

static int check_env_i(char* name, int i, int dfault)
{
	cptr str;
	int val;
	char buf[1024];
	
	strnfmt(buf, -1, name, i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	
	if (val <= 0) val = dfault;
		
	return val;
}

static int get_value(cptr buf)
{
	cptr str;
	int i;
	
	str = strstr(buf, "=");
	i = (str != NULL) ? atoi(str + 1) : -1;
	
	return i;
}

static void load_term_prefs()
{
	int t = 0, x = 0, line = 0, val = 0, section = 0;
	cptr str;
	char buf[1024];
	ang_file *fff;
	term_data *td = &data[0];
	xtra_win_data *xd = &xdata[0];
	
	/* Build the filename and open the file */
	path_build(settings, sizeof(settings), ANGBAND_DIR_USER, "gtk-settings.prf");
	fff = file_open(settings, MODE_READ, -1);
	
	/* File exists */
	if ((fff) && (!ignore_prefs))
	{
		/* Process the file */
		while (file_getl(fff, buf, sizeof(buf)))
		{
			/* Count lines */
			line++;

			/* Skip "empty" lines, "blank" lines, and comments */
			if (!buf[0]) continue;
			if (isspace((unsigned char)buf[0])) continue;
			if (buf[0] == '#') continue;
			
			if (buf[0] == '[') 
			{
				if (prefix(buf, "[General Settings]"))
				{
					section = GENERAL_PREFS;
				}
				else if (prefix(buf,"[Term window"))
				{
					section = TERM_PREFS;
					sscanf(buf, "[Term window %d]", &t);
					td = &data[t];
				}
				else if (prefix(buf,"[Extra window"))
				{
					section = XTRA_WIN_PREFS;
					sscanf(buf, "[Extra window %d]", &x);
					xd = &xdata[x];
				}
				continue;
			}
			
			if (section == GENERAL_PREFS)
			{
				if (prefix(buf, "Tile set="))
				{
					val = get_value(buf);
					sscanf(buf, "Tile set=%d", &val);
					arg_graphics = use_graphics = val;
					continue;
				}
			}
			
			if (section == TERM_PREFS)	
			{
				if (prefix(buf, "visible="))
				{
					sscanf(buf, "visible=%d", &val);
					td->visible = val;
				}
				else if (prefix(buf, "font="))
				{
					str = my_stristr(buf, "=");
					if (str != NULL)
					{
						if (str[0] == '=')
							str++;
						my_strcpy(td->font.name, str, strlen(str) + 1);
					}
				}
				else if (prefix(buf, "x="))                 sscanf(buf, "x=%d", &td->location.x);
				else if (prefix(buf, "y="))                 sscanf(buf, "y=%d",&td->location.y);
				else if (prefix(buf, "width="))         sscanf(buf, "width=%d", &td->size.w);
				else if (prefix(buf, "height="))        sscanf(buf, "height=%d", &td->size.h);
				else if (prefix(buf, "cols="))             sscanf(buf, "cols=%d", &td->cols);
				else if (prefix(buf, "rows="))           sscanf(buf, "rows=%d", &td->rows);
				else if (prefix(buf, "tile_width="))  sscanf(buf, "tile_width=%d", &td->tile.w);
				else if (prefix(buf, "tile_height=")) sscanf(buf, "tile_height=%d", &td->tile.h);
				continue;
			}
			
			if (section == XTRA_WIN_PREFS)
			{
				if (prefix(buf, "visible="))
				{
					sscanf(buf, "visible=%d", &val);
					xd->visible = val;
				}
				else if (prefix(buf, "font="))
				{
					str = my_stristr(buf, "=");
					if (str != NULL)
					{
						if (str[0] == '=')
							str++;
						my_strcpy(xd->font.name, str, strlen(str) + 1);
					}
				}
				else if (prefix(buf, "x="))                 sscanf(buf, "x=%d", &xd->location.x);
				else if (prefix(buf, "y="))                 sscanf(buf, "y=%d",&xd->location.y);
				else if (prefix(buf, "width="))         sscanf(buf, "width=%d", &xd->size.w);
				else if (prefix(buf, "height="))        sscanf(buf, "height=%d", &xd->size.h);
			}
		}
	}
	
	/* Close the file */
	file_close(fff);
}

static void load_prefs()
{
	int i;
	ang_file *fff;
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Loading Prefs.");
	#endif
	load_term_prefs(fff);
	
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		/* deal with env vars */
		td->location.x = check_env_i("ANGBAND_X11_AT_X_%d", i, td->location.x);
		td->location.y = check_env_i("ANGBAND_X11_AT_Y_%d", i,td->location.y);
		td->cols = check_env_i("ANGBAND_X11_COLS_%d", i,  td->cols);
		td->rows = check_env_i("ANGBAND_X11_ROWS_%d", i, td->rows);
	
		/* Set defaults if not initialized, or if funny values are there */
		if (td->cols == 0) td->cols = 80;
		if (td->rows == 0) td->rows = 24;
			
		if (td->size.w <=0) td->size.w = drawing_win_width(td);
		if (td->size.h <=0) td->size.h = drawing_win_height(td);
		if (td->tile.w <= 0) td->tile.w = td->font.w;
		if (td->tile.h <= 0) td->tile.h = td->font.h;
		
		/* The main window should always be visible */
		if (i == 0) td->visible = 1;
			
		if ((td->location.x <= 0) && (td->location.y <= 0)) td->location.x = td->location.y = 100;
		if ((td->font.name == "") || (strlen(td->font.name)<2)) 
			my_strcpy(td->font.name, "Monospace 12", sizeof(td->font.name));
		
		load_font_by_name(td, td->font.name);
	}

	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		if ((xd->font.name == "") || (strlen(xd->font.name)<2)) 
			my_strcpy(xd->font.name, "Monospace 10", sizeof(xd->font.name));
		
		if ((xd->size.w <=0 ) || (xd->size.h <= 0))
		{
			if (i == 5)
			{
				xd->size.h = 430;
				xd->size.w = 130;
			}
			else
			{
				xd->size.h = 130;
				xd->size.w = 430;
			}
		}
	}
}

gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	int z = 0;
	term_data *td = (term_data*)(Term->data);
	
	/* Where is the mouse */
	int x = event->x;
	int y = event->y;
	z = event->button;
	
	/* 
	 * Translate to the drawing area. I'm assuming for the moment that mouse clicks only
	 * are used in the main window. I may have to change that at some point.
	 */
	gtk_widget_translate_coordinates(widget, td->drawing_area, x, y, &x, &y);
	
	/* The co-ordinates are only used in Angband format. */
	pixel_to_square(&x, &y, x, y);
	
	Term_mousepress(x, y, z);
	return(FALSE);
}

gboolean expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;
	cairo_rectangle_t s;
	
	term_data *td = user_data;
	
	g_assert(td->drawing_area->window != 0);
	
	
	if (td->window)
	{
		g_assert(widget->window != 0);
		
		cr = gdk_cairo_create(td->drawing_area->window);
		s = gdk_rect_to_cairo(&event->area);
		cairo_draw_from_surface(cr, td->surface,  s);
		cairo_destroy(cr);
	}

	return (TRUE);
}

gboolean expose_xtra_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;
	cairo_rectangle_t s;
	
	xtra_win_data *xd = user_data;
	
	g_assert(xd->drawing_area->window != 0);
	
	
	if (xd->win)
	{
		g_assert(widget->window != 0);
		
		cr = gdk_cairo_create(xd->drawing_area->window);
		s = gdk_rect_to_cairo(&event->area);
		cairo_draw_from_surface(cr, xd->surface,  s);
		cairo_destroy(cr);
	}

	return (TRUE);
}
gboolean hide_options(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

static void create_term_cairo(term_data *td)
{	
		measurements size;
		
		if (td->surface != NULL)
		{	
			cairo_surface_destroy(td->surface);
			cairo_destroy(td->cr);
		}
	
		size.w = max_win_width(td);
		size.h = max_win_height(td);
		
		td->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.w, size.h);
		td->cr = cairo_create(td->surface);
		
		term_data_redraw(td);
}

gboolean toggle_term_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	int t;
	bool window_is_visible;
	char* s;
	
	s = (char*)gtk_widget_get_name(widget);
	sscanf(s, "term_menu_item_%d", &t);
	term_data *td = &data[t];
	
	window_is_visible = (GTK_WIDGET_VISIBLE(GTK_WIDGET(td->window)));
	
	/* I'm assuming this is only called by a menu item. I may want to put a check in for that. */
	td->visible = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
			
	if (td->visible)
	{
		create_term_cairo(td);
		gtk_widget_show(td->window);
	}
	else
		gtk_widget_hide(td->window);
	
	return(td->visible);
}

gboolean toggle_xtra_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{	
	int i;
	
	const char *item_name = gtk_widget_get_name(widget);
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		if (streq(item_name, xtra_menu_names[i]))
		{
			xtra_win_data *xd = &xdata[i];
			
			xd->visible = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
			
			if (xd->visible)
				gtk_widget_show(xd->win);
			else
				gtk_widget_hide(xd->win);
		}
	}
	
	return(TRUE);
}

static void init_graf(int g)
{
	char buf[1024];
	term_data *td= &data[0];
	
	arg_graphics = use_graphics = g;
	
	switch(arg_graphics)
	{
		case GRAPHICS_NONE:
		{
			ANGBAND_GRAF = "none";
			
			use_transparency = FALSE;
			td->tile.w = td->font.w;
			td->tile.h = td-> font.h;
			
			#ifdef GTK_DEBUG
			gtk_log_fmt(TERM_WHITE, "No Graphics.");
			#endif
			break;
		}
		case GRAPHICS_ORIGINAL:
		{
			ANGBAND_GRAF = "old";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, "graf/8x8.png");
			use_transparency = FALSE;
			td->tile.w = td->tile.h = 8;
			#ifdef GTK_DEBUG
			gtk_log_fmt(TERM_WHITE, "Old Graphics.");
			#endif
			break;
		}
		case GRAPHICS_ADAM_BOLT:
		{
			ANGBAND_GRAF = "new";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, "graf/16x16.png");
			use_transparency = TRUE;
			td->tile.w = td->tile.h =16;
			#ifdef GTK_DEBUG
			gtk_log_fmt(TERM_WHITE, "Adam Bolt Graphics.");
			#endif
			break;
		}
		case GRAPHICS_DAVID_GERVAIS:
		{
			ANGBAND_GRAF = "david";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, "graf/32x32.png");
			use_transparency = FALSE;
			td->tile.w = td->tile.h =32;
			#ifdef GTK_DEBUG
			gtk_log_fmt(TERM_WHITE, "David Gervais Graphics.");
			#endif
			break;
		}
	};
	graphical_tiles = cairo_image_surface_create_from_png(buf);
	tile_pattern = cairo_pattern_create_for_surface(graphical_tiles);
	
	g_assert(graphical_tiles != NULL);
	g_assert(tile_pattern != NULL);
	
	/* Hack -- Force redraw */
	if (game_in_progress)
	{
		reset_visuals(TRUE);
		Term_key_push(KTRL('R'));
	}
} 

static void setup_graphics_menu()
{
	int i = 0;
	
	for(i=0;i < 4; i++)
	{
		char s[12];
		GtkWidget *menu;
		bool checked = (i == arg_graphics);
		
		strnfmt(s, 12, "graphics_%d", i);
		menu = glade_xml_get_widget(gtk_xml, s);
		if (checked)
			if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu), TRUE); 
	}
}

/* Hooks for graphics menu items */
gboolean on_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	int g;
	char* s;
	
	s = (char*)gtk_widget_get_name(widget);
	sscanf(s, "graphics_%d", &g);
	
	/* Free up old graphics */
	if (graphical_tiles != NULL)
		cairo_surface_destroy(graphical_tiles);
	if (tile_pattern != NULL)
		cairo_pattern_destroy(tile_pattern);
	
	init_graf(g);
	return(FALSE);
}

/*
 * Make text views "Angbandy" 
 */
static void white_on_black_textview(xtra_win_data *xd)
{
	GdkColor black =  { 0, 0x0000, 0x0000, 0x0000 };
	GdkColor white = { 0, 0xffff, 0xffff, 0xffff };
	
	gtk_widget_modify_base(xd->text_view, GTK_STATE_NORMAL, &black);
	gtk_widget_modify_text(xd->text_view, GTK_STATE_NORMAL, &white);
}

static void xtra_data_init(xtra_win_data *xd, int i)
{
	xd->x = i;
	xd->name = xtra_names[i];
	xd->win_name = xtra_win_names[i];
	xd->text_view_name = xtra_text_names[i];
	xd->item_name = xtra_menu_names[i];
	xd->drawing_area_name = xtra_drawing_names[i];
}

static void init_xtra_windows(void)
{
	int i = 0;
	term_data *main_term= &data[0];
	char temp[20];
	GtkWidget *temp_widget;
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		strnfmt(temp, 14, "xtra_font_%d", i);
		temp_widget = glade_xml_get_widget(gtk_xml, temp);
		gtk_font_button_set_font_name(GTK_FONT_BUTTON (temp_widget), xd->font.name);

		xd->win = glade_xml_get_widget(gtk_xml, xd->win_name);
		g_signal_connect(xd->win, "delete_event", GTK_SIGNAL_FUNC(xtra_hide_event_handler), xd);
		g_signal_connect(xd->win, "configure_event", G_CALLBACK(configure_xtra_event_handler), xd);
		gtk_window_set_title(GTK_WINDOW(xd->win), xd->name);
		
		xd->menu = glade_xml_get_widget(gtk_xml, xd->item_name);
		g_signal_connect(xd->menu, "activate", G_CALLBACK(toggle_xtra_window), (gpointer) xd->name);
		
		#ifdef DISABLE_GTK_TESTING
		gtk_widget_set_sensitive(xd->menu, FALSE);
		#endif
		
		xd->text_view = glade_xml_get_widget(gtk_xml, xd->text_view_name);
		xd->drawing_area = glade_xml_get_widget(gtk_xml, xd->drawing_area_name);
		if (xd->text_view != NULL)
		{
			white_on_black_textview(xd);
			gtk_widget_modify_font(GTK_WIDGET(xd->text_view), pango_font_description_from_string(xd->font.name));
		}
		if (xd->drawing_area != NULL)
		{
			g_signal_connect(xd->drawing_area, "expose_event", G_CALLBACK(expose_xtra_handler), xd);
		
			xd->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 255 * main_term->font.w, 255 * main_term->font.h);
			xd->cr = cairo_create(xd->surface);
		}
		
	}
}

static void init_gtk_windows(void)
{
	char buf[1024], logo[1024], temp[20];
	GtkWidget *options, *temp_widget;
	int i = 0;
	bool err;
	
	/* Build the paths */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, "angband.glade");
	gtk_xml = glade_xml_new(buf, NULL, NULL);
	
	if (gtk_xml == NULL)
	{
		gtk_log_fmt(TERM_RED, "%s is Missing. Unrecoverable error. Aborting!", buf);
		quit(NULL);
		gtk_exit(0);
	}
			
	path_build(logo, sizeof(logo), ANGBAND_DIR_XTRA, "graf/mr_att.png");
	err = gtk_window_set_default_icon_from_file(logo, NULL);
	
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		if (!MAIN_WINDOW(td))
		{
			GladeXML *xml;
			
			/* Set up the Glade file */
			xml = glade_xml_new(buf, "term-window", NULL);
		
			td->window = glade_xml_get_widget(xml, "term-window");
			td->drawing_area = glade_xml_get_widget(xml, "drawingarea2");
			
			g_object_unref (xml);
		}
		else
		{
			td->window = glade_xml_get_widget(gtk_xml, "main-window");
			td->drawing_area = glade_xml_get_widget(gtk_xml, "drawingarea1");
			options = glade_xml_get_widget(gtk_xml, "options_window");
			
			g_signal_connect(options, "delete_event", GTK_SIGNAL_FUNC(hide_options), NULL);
		}
		
		g_signal_connect(td->drawing_area, "expose_event", G_CALLBACK(expose_event_handler), td);
		g_signal_connect(td->window, "configure_event", G_CALLBACK(configure_event_handler), td);
		g_signal_connect(td->window, "hide", G_CALLBACK(hide_event_handler), td);
		g_signal_connect(td->window, "show", G_CALLBACK(show_event_handler), td);
		g_signal_connect(td->window, "delete_event", GTK_SIGNAL_FUNC(hide_event_handler), td);
		
		/* Set title and name */
		strnfmt(temp, 14, "term_window_%d", i);
		gtk_widget_set_name(td->window, temp);
		gtk_window_set_title(GTK_WINDOW(td->window), td->name);
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Loaded term window %s", td->name);
		#endif
	}

	/* connect signal handlers that aren't passed data */
	glade_xml_signal_autoconnect(gtk_xml);
	
	setup_graphics_menu();
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		GdkColor black =  { 0, 0x0000, 0x0000, 0x0000 };
		
		gtk_widget_realize(td->window);
		gtk_widget_realize(td->drawing_area); 
		
		gtk_widget_modify_bg(td->window, GTK_STATE_NORMAL, &black);
		gtk_widget_modify_bg(td->drawing_area, GTK_STATE_NORMAL, &black);
		
		strnfmt(temp, 14, "term_font_%d", i);
		temp_widget = glade_xml_get_widget(gtk_xml, temp);
		gtk_widget_realize(temp_widget);
		
		err = gtk_font_button_set_font_name(GTK_FONT_BUTTON (temp_widget), td->font.name);
	}
}
static void show_windows()
{
	int i;
	GtkWidget *menu_item;
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		char item_name[20];
		
		/* Activate the window screen */
		Term_activate(&data[i].t);
		
		set_window_size(td);
		
		strnfmt(item_name, 16+1,  "term_menu_item_%i", i);
		menu_item = glade_xml_get_widget(gtk_xml, item_name);
		
	if (td->visible)
			create_term_cairo(td);
			
		if (i == 0)
		{
			gtk_widget_show(td->window);
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item)) != td->visible)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), td->visible);
		
		Term_clear_gtk(); 
	}
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
			set_xtra_window_size(xd);
		
			if (xd->visible) 
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(xd->menu), TRUE);
	}
}


static void xtra_data_destroy(xtra_win_data *xd)
{
	if GTK_IS_WIDGET(xd->win)
		gtk_widget_destroy(xd->win);
	if GTK_IS_WIDGET(xd->text_view)
		gtk_widget_destroy(xd->text_view);
	if GTK_IS_WIDGET(xd->menu)
		gtk_widget_destroy(xd->menu);
	if GTK_IS_WIDGET(xd->drawing_area)
		gtk_widget_destroy(xd->drawing_area);
	
	cairo_surface_destroy(xd->surface);
	cairo_destroy(xd->cr);
}

static void term_data_destroy(term_data *td)
{	
	if GTK_IS_WIDGET(td->window)
		gtk_widget_destroy(td->window);
	if GTK_IS_WIDGET(td->drawing_area)
		gtk_widget_destroy(td->drawing_area);
	
	cairo_surface_destroy(td->surface);
	cairo_destroy(td->cr);
}

static void release_memory()
{
	int i;
	
	cairo_pattern_destroy(tile_pattern);
	cairo_surface_destroy(graphical_tiles);
	
	/* Load Extra Windows */
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		xtra_data_destroy(xd);
	}
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		term_data_destroy(td);
	}
	
	g_object_unref (gtk_xml);
}

static errr term_data_init(term_data *td, int i)
{
	term *t = &td->t;

	td->cols = 80;
	td->rows = 24;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, 1024);

	/* Store the name of the term */
	td->name = angband_term_name[i];
	td->number = i;
	strnfmt(td->font.name, sizeof(td->font.name), "Monospace 12");
	
	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "dark space" */
	t->attr_blank = TERM_DARK;
	t->char_blank = ' ';
	t->higher_pict = TRUE;
	
	t->xtra_hook = Term_xtra_gtk;
	t->text_hook = Term_text_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->curs_hook = Term_curs_gtk;
	t->pict_hook = Term_pict_gtk;
	
	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);
	
	/* Success */
	return (0);
}

static game_command get_init_cmd()
{
	Term_fresh();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	CheckEvent(FALSE);
	
	if ((cmd.command == CMD_NEWGAME) || (cmd.command == CMD_LOADFILE))
		game_in_progress = TRUE;
	
	return cmd;
}

#ifndef DISABLE_GTK_TESTING
static void handle_map(game_event_type type, game_event_data *data, void *user)
{
	/*gtk_log_fmt(TERM_WHITE, "The map changed.");*/
}

static void handle_moved(game_event_type type, game_event_data *data, void *user)
{
	/*gtk_log_fmt(TERM_WHITE, "The player moved.");*/
}
static void handle_mons_target(game_event_type type, game_event_data *data, void *user)
{
	/*gtk_log_fmt(TERM_WHITE, "Monster targetting.");*/
}

/*
 * Set up color tags for all the angband colors.
 */
static void init_color_tags(xtra_win_data *xd)
{
	int i;
	char *c = NULL, colorname[10] = "color-00",str[10] = "#FFFFFF";
	GtkTextTagTable *tags;
	c = str;
	tags = gtk_text_buffer_get_tag_table(xd->buf);
	
	for (i = 0; i <= 15; i++)
	{
		strnfmt(colorname, 8 + 1,  "color-%d", i);
		strnfmt(c, 14, "#%02x%02x%02x", angband_color_table[i][1], angband_color_table[i][2], angband_color_table[i][3]);
		
		if (gtk_text_tag_table_lookup(tags, colorname) != NULL)
			gtk_text_tag_table_remove(tags, gtk_text_tag_table_lookup(tags, colorname));
		
		if (gtk_text_tag_table_lookup(tags, colorname) == NULL)
			gtk_text_buffer_create_tag(xd->buf, colorname, "foreground", c, NULL);
	}
}
#endif

static void text_view_put(xtra_win_data *xd, const char *str, byte color)
{
	char colorname[10] = "color-00";
	GtkTextIter start, end;
	
	/* Get the color tag ready, and tack a line feed on the line we're printing */
	strnfmt(colorname, 8 + 1,  "color-%d", color);
	
	/* Print it at the end of the window */
	gtk_text_buffer_get_bounds(xd->buf, &start, &end);
	gtk_text_buffer_insert_with_tags_by_name(xd->buf, &end, str, strlen(str), colorname,  NULL);
}

static void text_view_print(xtra_win_data *xd, const char *str, byte color)
{	
	strnfmt((char *)str, strlen(str) + 2, "%s\n", str);
	text_view_put(xd, str, color);
}

static void aligned_text_print(xtra_win_data *xd, const char *str, byte color, const char *str2, byte color2, int length)
{
	int str_length, str2_length, padding_length;
	char adj_str[length + 1], padding[length+1];
	
	str_length = strlen(str);
	str2_length = strlen(str2);
	padding_length = length - str_length - str2_length + 1;
	if (padding_length > 0)
	{
		strnfmt(padding, padding_length + 1, "%*c", padding_length, ' ');
		strnfmt(adj_str, str_length + padding_length, "%s%s", str, padding);
		text_view_put(xd, adj_str, color);
		text_view_print(xd, str2, color2);
	}
	else
	{
		text_view_put(xd, str, color);
		text_view_print(xd, str2, color2);
	}
}

static void text_status_bar_print(xtra_win_data *xd, float percent, byte color)
{
	char str[13];
	int num = (percent * 10);
	
	if (num == 0)
	{
		strnfmt(str, 13, "[----------] ");
		text_view_put(xd, str, TERM_WHITE);
	}
	else
	{
		strnfmt(str, 2, "[");
		text_view_put(xd, str, TERM_WHITE);
		
		strnfmt(str, num + 1, "%s", "**********");
		text_view_put(xd, str, color);
		num = 10 - num;
		
		if (num == 0)
			strnfmt(str, 2, "]");
		else
		{
			strnfmt(str, num + 1, "%s", "----------");
			strnfmt(str, num + 2, "%s]", str);
		}
		
		text_view_print(xd, str, TERM_WHITE);
	}
}

void gtk_log_fmt(byte c, cptr fmt, ...)
{
	#ifndef DISABLE_GTK_TESTING
	xtra_win_data *xd = &xdata[4];
	#endif
	
	char *res, str[80];
	va_list vp;
	int n;
	
	#ifndef DISABLE_GTK_TESTING
	if (GTK_IS_TEXT_VIEW(xd->text_view))
	{
		xd->buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(xd->text_view));
		if (!GTK_IS_TEXT_BUFFER(xd->buf))
		{
			xd->buf = gtk_text_buffer_new(NULL);
			gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), xd->buf);
		}
		init_color_tags(xd);
	}
	#endif
	
	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args */
	res = vformat(fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	n = my_strcpy(str, res, strlen(res) + 1);
	
	#ifdef VERBOSE_DEBUG
	if (GTK_IS_TEXT_VIEW(xd->text_view))
		plog(str);
	#endif
	
	#ifndef DISABLE_GTK_TESTING
	if (GTK_IS_TEXT_VIEW(xd->text_view))
		text_view_print(xd, str, c);
	else
	#endif
		plog(str);
}

#ifndef DISABLE_GTK_TESTING
/*
 * Update our own personal message window.
 */
static void handle_message(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[0];
	
	u16b num;
	int i,c;
	char str[80];
	
	if (xd != NULL)
	{
	xd->buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), xd->buf);
	
	init_color_tags(xd);
	
	num = messages_num();
	
	for (i = 0; i < num; i++)
	{
		c = message_color(i);

		strnfmt(str, strlen(message_str(i)) + 1, "%s", message_str(i));
		text_view_print(xd, str, c);
	}
}
}

/* 
 * Return the last inventory slot.
 */
static int last_inv_slot(void)
{
	register int i, z = 0;
	object_type *o_ptr;
	
	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}
	
	return z;
}

static void inv_slot(char *str, int i, bool equip)
{
	object_type *o_ptr;
	register int n;
	char o_name[80], label[5];
	int name_size = 80;
	
	/* Examine the item */
	o_ptr = &inventory[i];

	/* Is this item "acceptable"? */
	if (item_tester_okay(o_ptr) || equip )
	{
		/* Obtain an item description */
		object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);
			
		/* Obtain the length of the description */
		n = strlen(o_name);
		
		strnfmt(label, 3 + 1, "%c) ",index_to_label(i));
		
		if (equip && show_labels)
			name_size = name_size - 19;
		
		/* Display the weight if needed */
		if (o_ptr->weight) 
		{
			
			name_size = name_size - 4 - 9 - 9 - 5;
			int wgt = o_ptr->weight * o_ptr->number;
			
			if (equip && show_labels)
				strnfmt(str, 80,  "%s%-*s %3d.%1d lb <-- %s", label, name_size, o_name, wgt / 10, wgt % 10, mention_use(i));
			else
				strnfmt(str, 80, "%s%-*s %3d.%1d lb", label, name_size, o_name, wgt / 10, wgt % 10);
			
		}
		else
		{
			name_size = name_size - 4 - 9 - 5;
			if (equip && show_labels)
				strnfmt(str, 80, "%s%-*s <-- %s", label, name_size, o_name, mention_use(i));
			else
				strnfmt(str, 80, "%s%-*s", label, name_size, o_name);
		}
	}
}

static void handle_inv(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[1];
	
	register int i, z;
	byte attr;
	object_type *o_ptr;
	
	if (xd != NULL)
	{
	xd->buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), xd->buf);
	
	init_color_tags(xd);
	
	z = last_inv_slot();
	
	/* Display the pack */
	for (i = 0; i < z; i++)
	{
		char str[80];
	
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Get inventory color */
			attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];
			
			inv_slot(str, i, FALSE);
			text_view_print(xd, str, attr);
		}
	}
}
}

static void handle_equip(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[2];
	
	register int i;
	byte attr;
	object_type *o_ptr;
	
	if (xd != NULL)
	{
	xd->buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), xd->buf);
	
	init_color_tags(xd);
	
	/* Display the pack */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		char str[80];
	
		/* Examine the item */
		o_ptr = &inventory[i];
		
		/* Is this item "acceptable"? */
		attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];
		
		inv_slot(str, i, TRUE);
		text_view_print(xd, str, attr);
	}
	}
}
static void handle_mons_list(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[3];
	int i;
	int line = 1, x = 0;
	int cur_x;
	unsigned total_count = 0, disp_count = 0;

	byte attr;

	char *m_name;
	char buf[80];
	char str[80];

	monster_type *m_ptr;
	monster_race *r_ptr;

	u16b *race_count;
	
	if (xd != NULL)
	{
	xd->buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), xd->buf);
	
	init_color_tags(xd);

	/* Allocate the array */
	race_count = C_ZNEW(z_info->r_max, u16b);

	/* Scan the monster list */
	for (i = 1; i < mon_max; i++)
	{
		m_ptr = &mon_list[i];

		/* Only visible monsters */
		if (!m_ptr->ml) continue;

		/* Bump the count for this race, and the total count */
		race_count[m_ptr->r_idx]++;
		total_count++;
	}

	/* Note no visible monsters */
	if (!total_count)
	{
		strnfmt(str, 20 + 1, "You see no monsters.");
		text_view_print(xd, str, TERM_SLATE);

		/* Free up memory */
		FREE(race_count);

		/* Done */
		return;
	}
	
	strnfmt(str, 25 + 1, "You can see %d monster%s:", total_count, (total_count > 1 ? "s" : ""));
	text_view_print(xd, str, 1);
	
	/* Go over */
	for (i = 1; i < z_info->r_max; i++)
	{
		/* No monsters of this race are visible */
		if (!race_count[i]) continue;

		/* Reset position */
		cur_x = x;

		/* Note that these have been displayed */
		disp_count += race_count[i];

		/* Get monster race and name */
		r_ptr = &r_info[i];
		m_name = r_name + r_ptr->name;

		/* Display uniques in a special colour */
		if (r_ptr->flags1 & RF1_UNIQUE)
			attr = TERM_VIOLET;
		else
			attr = TERM_WHITE;

		/* Build the monster name */
		if (race_count[i] == 1)
			my_strcpy(buf, m_name, sizeof(buf));
		else
			strnfmt(buf, sizeof(buf), "%s (x%d) ", m_name, race_count[i]);
		
		text_view_print(xd, buf, attr);
		line++;
	}

	/* Free the race counters */
	FREE(race_count);
	}
}

static void blank_row(xtra_win_data *xd)
{
	char str[80];
	
	strnfmt(str, sizeof(str), " "); 
	text_view_print(xd, str, TERM_WHITE);
}

static byte monst_color(float percent)
{
	int pct = percent * 100;
	byte attr = TERM_WHITE;
	monster_type *m_ptr = &mon_list[p_ptr->health_who];
	
	/* Badly wounded */
	if (pct >= 10) attr = TERM_L_RED;

	/* Wounded */
	if (pct >= 25) attr = TERM_ORANGE;

	/* Somewhat Wounded */
	if (pct >= 60) attr = TERM_YELLOW;

	/* Healthy */
	if (pct >= 100) attr = TERM_L_GREEN;

	/* Afraid */
	if (m_ptr->monfear) attr = TERM_VIOLET;

	/* Confused */
	if (m_ptr->confused) attr = TERM_UMBER;

	/* Stunned */
	if (m_ptr->stunned) attr = TERM_L_BLUE;

	/* Asleep */
	if (m_ptr->csleep) attr = TERM_BLUE;
		
	return attr;
}

/*
 * Equippy chars
 */
static void print_equippy(xtra_win_data *xd)
{
	int i;

	byte a;
	char c[6];

	object_type *o_ptr;

	/* No equippy chars if graphics are on, or we're in bigtile mode */
	if ((use_bigtile) || (arg_graphics != GRAPHICS_NONE))
	{
		blank_row(xd);
		return;
	}

	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Object */
		o_ptr = &inventory[i];

		a = object_attr(o_ptr);
		strnfmt(c, sizeof(c), "%c",object_char(o_ptr)); 

		/* Clear the part of the screen */
		if (!o_ptr->k_idx)
		{
			c [0]= ' ';
			a = TERM_WHITE;
		}

		/* Dump */
		text_view_put(xd, c, a);
	}
	strnfmt(c, sizeof(c), " "); 
	text_view_print(xd,c, a);
}

static void handle_sidebar(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[5];
	char str[80], str2[80];
	byte color;
	long xp = (long)p_ptr->exp;
	monster_type *m_ptr = &mon_list[p_ptr->health_who];
	float mhp;
	float m_cur_hp, m_max_hp;
	
	int sidebar_length = 12;

	/* Calculate XP for next level */
	if (p_ptr->lev != 50)
		xp = (long)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L) - p_ptr->exp;
	
	if (xd != NULL)
	{
	xd->buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), xd->buf);
	init_color_tags(xd);
	
	/* Char Name */
	strnfmt(str, sizeof(str), "%s", op_ptr->full_name); 
	text_view_print(xd, str, TERM_L_BLUE);
	/* Char Race */
	strnfmt(str, sizeof(str), "%s", p_name + rp_ptr->name);
	text_view_print(xd, str, TERM_L_BLUE);
	/* Char Title*/
	strnfmt(str, sizeof(str), "%s", c_text + cp_ptr->title[(p_ptr->lev - 1) / 5], TERM_L_BLUE); 
	text_view_print(xd, str, TERM_L_BLUE);
	/* Char Class */
	strnfmt(str, sizeof(str), "%s", c_name + cp_ptr->name); 
	text_view_print(xd, str, TERM_L_BLUE);
	
	/* Char Level */
	strnfmt(str, sizeof(str), "Level:"); 
	strnfmt(str2, sizeof(str2), "%i", p_ptr->lev);
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	/* Char xp */
	if (p_ptr->lev != 50) strnfmt(str, sizeof(str), "Next: ");
	else 			 strnfmt(str, sizeof(str), "XP: "); 
	strnfmt(str2, sizeof(str2), "%ld", xp); 
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	/* Char Gold */
	strnfmt(str, sizeof(str), "Gold:"); 
	strnfmt(str2, sizeof(str2), "%ld", p_ptr->au); 
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	/* Equippy chars */
	print_equippy(xd);
	
	/* Char Stats */
	strnfmt(str, sizeof(str), "STR:"); 
	cnv_stat(p_ptr->stat_use[A_STR], str2, sizeof(str2));
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	strnfmt(str, sizeof(str), "INT:"); 
	cnv_stat(p_ptr->stat_use[A_INT], str2, sizeof(str2));
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	strnfmt(str, sizeof(str), "WIS:"); 
	cnv_stat(p_ptr->stat_use[A_WIS], str2, sizeof(str2));
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	strnfmt(str, sizeof(str), "DEX:"); 
	cnv_stat(p_ptr->stat_use[A_DEX], str2, sizeof(str2));
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	strnfmt(str, sizeof(str), "CON:"); 
	cnv_stat(p_ptr->stat_use[A_CON], str2, sizeof(str2));
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	strnfmt(str, sizeof(str), "CHR:"); 
	cnv_stat(p_ptr->stat_use[A_CHR], str2, sizeof(str2));
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	blank_row(xd);
	
	/* Char AC */
	strnfmt(str, sizeof(str), "AC:"); 
	strnfmt(str2, sizeof(str2), "%i", p_ptr->dis_ac + p_ptr->dis_to_a); 
	aligned_text_print(xd, str, TERM_WHITE, str2, TERM_L_GREEN, sidebar_length);
	
	/* Char HP */
	strnfmt(str, sizeof(str), "HP:"); 
	strnfmt(str2, sizeof(str2), "%4d/%4d", p_ptr->chp, p_ptr->mhp); 
	if (p_ptr->chp >= p_ptr->mhp)
		color = TERM_L_GREEN;
	else if (p_ptr->chp > (p_ptr->mhp * op_ptr->hitpoint_warn) / 10)
		color = TERM_YELLOW;
	else
		color = TERM_RED;
	aligned_text_print(xd, str, TERM_WHITE, str2, color, sidebar_length);
	
	/* Char MP */
	strnfmt(str, sizeof(str), "SP:"); 
	strnfmt(str2, sizeof(str2), "%4d/%4d", p_ptr->csp, p_ptr->msp); 
	if (p_ptr->csp >= p_ptr->msp)
		color = TERM_L_GREEN;
	else if (p_ptr->csp > (p_ptr->msp * op_ptr->hitpoint_warn) / 10)
		color = TERM_YELLOW;
	else
		color = TERM_RED;
	aligned_text_print(xd, str, TERM_WHITE, str2, color, sidebar_length);
	
	blank_row(xd);
	
	m_cur_hp = m_ptr->hp;
	m_max_hp = m_ptr->maxhp;
	if ((m_max_hp > 0.0) && (m_cur_hp > 0.0))
		mhp = m_cur_hp / m_max_hp;
	else
		mhp = 0;
		
	text_status_bar_print(xd, mhp , monst_color(mhp));
	
	blank_row(xd);
	
	/* Print the level */
	if (!p_ptr->depth)
		strnfmt(str, sizeof(str), "Town"); 
	else if (depth_in_feet)
		strnfmt(str, sizeof(str), "%d ft", p_ptr->depth * 50);
	else 
		strnfmt(str, sizeof(str), "Lev %d", p_ptr->depth);
	strnfmt(str, sizeof(str), "%-*s", sidebar_length, str);
	text_view_print(xd, str, TERM_WHITE);
	}
}

static void handle_init_status(game_event_type type, game_event_data *data, void *user)
{
	/*gtk_log_fmt(TERM_WHITE, "Init status.");*/
}
static void handle_birth(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Birth!");
}
static void handle_game(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Into the game.");
}
static void handle_store(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Going shopping.");
}
static void handle_death(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Death - the great equalizer.");
}
static void handle_end(game_event_type type, game_event_data *data, void *user)
{
	/*gtk_log_fmt(TERM_WHITE, "Enough of these events.");*/
}
static void handle_splash(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Showing the splashscreen!!!");
}
static void handle_statusline(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Showing the statusline.");
}
#endif

void init_handlers()
{
	
	/* Activate hooks */
	quit_aux = hook_quit;

	/* Set command hook */
	get_game_command = get_init_cmd;
	
	#ifndef DISABLE_GTK_TESTING
	/* I plan to put everything on the sidebar together, so... */
	event_add_handler_set(my_player_events, N_ELEMENTS(my_player_events), handle_sidebar, NULL);

	/* Same goes for the sidebar */
	event_add_handler_set(my_statusline_events, N_ELEMENTS(my_statusline_events), handle_statusline, NULL);
	
	event_add_handler(EVENT_MAP, handle_map, NULL);
	event_add_handler(EVENT_PLAYERMOVED, handle_moved, NULL);
	event_add_handler(EVENT_INVENTORY, handle_inv, NULL);
	event_add_handler(EVENT_EQUIPMENT, handle_equip, NULL);
	event_add_handler(EVENT_MONSTERLIST, handle_mons_list, NULL);
	event_add_handler(EVENT_MONSTERTARGET, handle_mons_target, NULL);
	event_add_handler(EVENT_MESSAGE, handle_message, NULL);
	event_add_handler(EVENT_INITSTATUS, handle_init_status, NULL);
	event_add_handler(EVENT_ENTER_INIT, handle_splash, NULL);
	event_add_handler(EVENT_ENTER_BIRTH, handle_birth, NULL);
	event_add_handler(EVENT_ENTER_GAME, handle_game, NULL);
	event_add_handler(EVENT_ENTER_STORE, handle_store, NULL);
	event_add_handler(EVENT_ENTER_DEATH, handle_death, NULL);
	event_add_handler(EVENT_END, handle_end, NULL);
	#endif
}

errr init_gtk(int argc, char **argv)
{
	int i;

	/* Initialize the environment */
	gtk_init(&argc, &argv);
	
	/* Parse args */
	for (i = 1; i < argc; i++)
	{	
		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}
		
		if (prefix(argv[i], "-i"))
		{
			gtk_log_fmt(TERM_VIOLET, "Ignoring preferences.");
			ignore_prefs = TRUE;
			continue;
		}
		
		gtk_log_fmt(TERM_VIOLET, "Ignoring option: %s", argv[i]);
	}
	
	/* Load Extra Windows */
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		xtra_data_init(xd, i);
	}
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;
	}
	
	
	/* Load Preferences */
	load_prefs();
	
	/* Init the windows */
	init_gtk_windows(); 
	
	/* Init "Extra" windows */
	init_xtra_windows();
	
	/* Show them all */
	show_windows();
	
	/* Initialise hooks and handlers */
	init_handlers();
	
	Term_activate(&data[0].t);
	
	/* Set the system suffix */
	ANGBAND_SYS = "gtk";

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Catching signals.");
	#endif
	
	/* Catch nasty signals, unless we want to see them */
	#ifndef GTK_DEBUG
	signals_init();
	#endif

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	
	Term_fresh();
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Updating Windows.");
	#endif
	
	game_in_progress = FALSE;

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Initializing Angband.");
	#endif
	
	/* Set up the display handlers and things. */
	init_display();
	
	/* Let's play */
	play_game();

	/* Stop now */
	exit(0);

	/* Success */
	return (0);
}

#endif  /*USE_GTK */
