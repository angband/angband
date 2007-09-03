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
#define USE_CAIRO_UTILS

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
 #define DISABLE_GTK_TESTING
 
static game_command cmd = { CMD_NULL, 0 }; 
/*
 * Set foreground color
 */
GdkRectangle cairo_rect_to_gdk(cairo_rectangle_t *r)
{
	GdkRectangle s;
	
	s.x = r->x;
	s.y = r->y;
	s.width = r->width;
	s.height = r->height;
	
	return(s);
}

cairo_rectangle_t gdk_rect_to_cairo(GdkRectangle *r)
{
	cairo_rectangle_t s;
	
	s.x = r->x;
	s.y = r->y;
	s.width = r->width;
	s.height = r->height;
	
	return(s);
}

void invalidate_rect(term_data *td, cairo_rectangle_t r)
{
	GdkRectangle s;
	
	s = cairo_rect_to_gdk(&r);
	gdk_window_invalidate_rect(td->drawing_area->window, &s, TRUE);
}

/* 
 * Get the position of the window and save it. Gtk being what it is, this is a major hack.
 * It'd be nice to replace this with something cleaner...
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
			if (w != 0) td->size.w = w;
			if (h != 0) td->size.h = h;
		}
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Window %s: Location(%d/%d), Size(%d/%d)", td->name, td->location.x, td->location.y, td->size.w, td->size.h);
		#endif
	}
}

static void set_window_defaults(term_data *td)
{
	GdkGeometry geo;
	
	geo.width_inc = td->font_size.w;
	geo.height_inc = td->font_size.h;
	
	geo.max_width = td->font_size.w * 255;
	geo.max_height = td->font_size.h * 255;
	
	if (td == &data[0])
	{
		geo.min_width = td->font_size.w * td->cols;
		geo.min_height = td->font_size.h * td->rows;
	}
	else
	{
		geo.min_width = td->font_size.w;
		geo.min_height = td->font_size.h;
	}
	
	gtk_window_set_geometry_hints(GTK_WINDOW(td->window), td->drawing_area, &geo, 
	GDK_HINT_POS | GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE); 
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

gboolean configure_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	
	term_data *td = user_data;
	
	get_size_pos(td);
	return(FALSE);
}

static void get_term_sizes()
{
	int i;
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling Get Term Sizes.");
	#endif
	
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		if (td->visible)
		{
			get_size_pos(td);
		}
	}
}

/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, 0, 0, 255 * td->font_size.w, 255 * td->font_size.h);
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
	init_cairo_rect(&r, (x * td->font_size.w), (y * td->font_size.h), (td->font_size.w * n), (td->font_size.h));
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
	PangoLayout *layout;
	
	g_assert(cairo_status(td->cr) == CAIRO_STATUS_SUCCESS);
	
	/* Set dimensions */
	init_cairo_rect(&r, x * td->font_size.w, y * td->font_size.h,  td->font_size.w * n, td->font_size.h);
	
	/* Clear the line */
	Term_wipe_gtk(x, y, n);
	
	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout(td->cr); 
	g_assert(layout != NULL);
	
	set_foreground_color(td->cr, a);
	pango_layout_set_text(layout, s, n);
	pango_layout_set_font_description(layout, td->font);
	
	/* Draw the text to the pixmap */
	cairo_move_to(td->cr, r.x, r.y);
	
	pango_cairo_show_layout(td->cr, layout);
	g_object_unref(G_OBJECT(layout));
	invalidate_rect(td, r);
			
	/* Success */
	return (0);
}

static errr Term_pict_gtk(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	/* Set dimensions */
	init_cairo_rect(&r, x * td->font_size.w, y * td->font_size.h,  td->font_size.w * n, td->font_size.h);
	
	/* Clear the line */
	Term_wipe_gtk(x, y, n);
	
	/* I can't count on at all the td fields names being the same for everything that uses cairo-utils,
	    hence a lot of variables being passed. */
	draw_tiles(td->cr, x, y, n, ap, cp, tap, tcp, td->font_size.h, td->font_size.w, td->tile.h, td->tile.w);

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
	
	init_cairo_rect(&r, 0, 0, 255 * td->font_size.w, 255 * td->font_size.h);
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
	init_cairo_rect(&r, (x * td->font_size.w) +1, (y * td->font_size.h) + 1,  (td->font_size.w) - 2, (td->font_size.h) - 2);
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
	
	/* Activate the term */
	Term_activate(&data[0].t);

	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();
	Term_activate(old);
}

static void Term_show(term_data *td)
{
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling Term_show for %s.", td->name);
	#endif
	td->visible = TRUE;
	
	if (!GTK_WIDGET_VISIBLE(GTK_WIDGET(td->window)))
	{
		gtk_widget_show(td->window);
	}
}

static void Term_hide(term_data *td)
{	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Calling Term_hide for %s.", td->name);
	#endif
	td->visible = FALSE;
	
	if (GTK_WIDGET_VISIBLE(GTK_WIDGET(td->window)))
		{
		get_size_pos(td);
		gtk_widget_hide(td->window);
		}
}

static errr CheckEvent(bool wait)
{
	if (wait)
	{
		gtk_main_iteration();
	}
	else
	{
		while (gtk_events_pending())
			gtk_main_iteration();
	}

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
	{
		return(TRUE);
	}
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
	
	td->visible = FALSE;
	update_term_menu();
	
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
	PangoRectangle r;
	PangoLayout *temp;
	
	cairo_t *cr;
	cairo_surface_t *surface;
	td->font_name = font_name;
	
	/* Create a temp pango/cairo context to play in */
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 200);
	cr = cairo_create(surface);
	
	temp = pango_cairo_create_layout(cr);
	td->font = pango_font_description_from_string(td->font_name);
	
	g_assert(surface != NULL);
	g_assert(cr != NULL);
	g_assert(td->font != NULL);
	
	/* Draw an @, and measure it */
	pango_layout_set_font_description(temp, td->font);
	td->font_pt = (pango_font_description_get_size(td->font) / (double)(PANGO_SCALE)) * (96.0 / 72.0);
	pango_layout_set_text(temp, "@", 1);
	pango_cairo_show_layout(cr, temp);
	pango_layout_get_pixel_extents(temp, NULL, &r);
	
	/* Set the char width and height */
	td->font_size.w = r.width;
	td->font_size.h = r.height;
	td->size.w = r.width * 80;
	td->size.h = r.height * 24;
	
	/* Blow up our temp variables */
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	g_object_unref(temp);
	
	if (td->window != NULL) 
	{
	/* Set defaults, and resize the window accordingly */
	set_window_defaults(td);
	set_window_size(td);
		
	Term_flush();
	term_data_redraw(td);
	}
}

void set_font(GtkWidget *widget, GtkWidget *font_dialog)
{
	term_data *td = g_object_get_data(G_OBJECT(font_dialog), "term_data");
	g_assert(td != NULL);

	load_font_by_name(td, gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(font_dialog)));
	g_assert(td->font != NULL);
	
	gtk_widget_hide(font_dialog);
}


void change_font_event_handler(GtkWidget *widget, gpointer user_data)
{
	term_data *td = (term_data*)(Term->data);
	GtkWidget *font_dialog;

	font_dialog = glade_xml_get_widget(td->xml, "font-window");
	
	g_object_set_data(G_OBJECT(font_dialog), "term_data", user_data);
	
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(font_dialog), 
		pango_font_description_to_string(td->font));
	
	g_signal_connect(G_OBJECT(GTK_FONT_SELECTION_DIALOG(font_dialog)->ok_button), "clicked", 
		G_CALLBACK(set_font), (gpointer)font_dialog);

	gtk_widget_show(GTK_WIDGET(font_dialog));
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
	/* XXX Should disable the menu entry, but hooks need to be added in angband for that */
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
	}
	/* For convenience */
	selector = GTK_FILE_CHOOSER(selector_wid);
	gtk_file_chooser_set_do_overwrite_confirmation (selector, TRUE);
	
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
	
	if (accepted)
	{
		/* Set the command now so that we skip the "Open File" prompt. */ 
		cmd.command = CMD_LOADFILE; 
	}
	
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
		/*sprintf(msg, "%cS_%X%c", 31, event->keyval, 13);*/
		strnfmt(msg, -1, "%cS_%X%c", 31, event->keyval, 13);
		/*strnfmt(msg, sizeof(event->keyval)+2+1, "%cS_%X%c", 31, event->keyval, 13);*/
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

	/* Normal keys with no modifiers */
	if (event->length && !mo && !mx)
	{
		/* Enqueue the normal key(s) */
		for (i = 0; i < event->length; i++) Term_keypress(event->string[i]);

		/* All done */
		return (TRUE);
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
	int x, y, w, h;

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Saving Prefs.");
	#endif
	
	/* Open the settings file */
	fff = file_open(settings, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fff) return;

	/* Header */
	file_putf(fff, "# %s GTK settings\n\n", VERSION_NAME);
		
	/* Graphics setting */
	file_putf(fff,"TILE_SET=%d\n", arg_graphics);

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		if (!td->t.mapped_flag) continue;

		/* Header */
		file_putf(fff, "# Term %d\n", i);
		
		get_size_pos(td);
		x = td->location.x;
		y = td->location.y;
		w = td->size.w;
		h = td->size.h;
		
		/* Window specific location (y) */
		file_putf(fff, "VISIBLE_%d=%d\n", i, td->visible);

		/* Window specific location (x) */
		file_putf(fff, "AT_X_%d=%d\n", i, x);

		/* Window specific location (y) */
		file_putf(fff, "AT_Y_%d=%d\n", i, y);

		/* Window specific location (w) */
		file_putf(fff, "WIDTH_%d=%d\n", i, w);

		/* Window specific location (h) */
		file_putf(fff, "HEIGHT_%d=%d\n", i, h);
		
		/* Window specific cols */
		file_putf(fff, "COLS_%d=%d\n", i, td->cols);

		/* Window specific rows */
		file_putf(fff, "ROWS_%d=%d\n", i, td->rows);
		
		/* Window specific font name */
		file_putf(fff, "FONT_%d=%s\n", i, pango_font_description_to_string(td->font));

		/* Window specific tile width */
		file_putf(fff, "TILE_WIDTH_%d=%d\n", i, td->tile.w);

		/* Window specific tile height */
		file_putf(fff, "TILE_HEIGHT_%d=%d\n", i, td->tile.h);
		
		/* Footer */
		file_putf(fff, "\n");
	}

	/* Close */
	file_close(fff);;
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

static void load_prefs(term_data *td, int i)
{
	cptr font = "";

	int x = 0, y = 0, w = 0, h = 0;
	int line, tile_set = 0;
	int visible = 0;

	int cols = 80;
	int rows = 24;

	cptr str;

	int val;

	ang_file *fff;

	char buf[1024];
	char cmd[40];
	char font_name[256];

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Loading Prefs.");
	#endif
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

			/* Window specific location (x) */
			strnfmt(cmd, 6+1, "AT_X_%d", i);
			if (prefix(buf, cmd))
			{
				x = get_value(buf);
				continue;
			}
			
			/* Window specific location (y) */
			strnfmt(cmd, 6+1, "AT_Y_%d", i);

			if (prefix(buf, cmd))
			{
				y = get_value(buf);
				continue;
			}

			/* Window specific location (x) */
			strnfmt(cmd, 7+1, "WIDTH_%d", i);

			if (prefix(buf, cmd))
			{
				w = get_value(buf);
				continue;
			}
			
			/* Window specific location (y) */
			strnfmt(cmd, 8+1, "HEIGHT_%d", i);

			if (prefix(buf, cmd))
			{
				h = get_value(buf);
				continue;
			}
			
			/* Window is visible? (y) */
			strnfmt(cmd, 9+1, "VISIBLE_%d", i);

			if (prefix(buf, cmd))
			{
				visible = get_value(buf);
				continue;
			}
		
			/* Window specific cols */
			strnfmt(cmd, 6+1, "COLS_%d", i);

			if (prefix(buf, cmd))
			{
				val = get_value(buf);
				if (val > 0) cols = val;
				continue;
			}
				

			/* Window specific rows */
			strnfmt(cmd, 6+1, "ROWS_%d", i);

			if (prefix(buf, cmd))
			{
				val = get_value(buf);
				if (val > 0) rows = val;
				continue;
			}

			/* Window specific font name */
			strnfmt(cmd, 6+sizeof(font_name)+1, "FONT_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				if (str != NULL)
				{
					my_strcpy(font_name, str + 1, sizeof(font_name));
					font = font_name;
				}
				continue;
			}

			/* Window specific tile width */
			strnfmt(cmd, 12+1, "TILE_WIDTH_%d", i);

			if (prefix(buf, cmd))
			{
				val = get_value(buf);
				if (val > 0) td->tile.w= val;
				continue;
			}

			/* Window specific tile height */
			strnfmt(cmd, 13+1, "TILE_HEIGHT_%d", i);

			if (prefix(buf, cmd))
			{
				val = get_value(buf);
				if (val > 0) td->tile.h = val;
				continue;
			}

			/* Window specific tile height */
			strnfmt(cmd, -1, "TILE_SET");

			if (prefix(buf, cmd))
			{
				val = get_value(buf);
				tile_set = val;
				continue;
			}
		}

		/* Close */
		file_close(fff);
	}

	/*
	 * Env-vars overwrite the settings in the settings file
	 */

	x = check_env_i("ANGBAND_X11_AT_X_%d", i, x);
	y = check_env_i("ANGBAND_X11_AT_Y_%d", i, y);
	cols = check_env_i("ANGBAND_X11_COLS_%d", i,  cols);
	rows = check_env_i("ANGBAND_X11_ROWS_%d", i, rows);
		
	/* Window specific font name */
	strnfmt(buf, 18+1, "ANGBAND_X11_FONT_%d", i);
	str = getenv(buf);
	if (str) font = str;
	if (font == "") font = "Monospace 12";
	load_font_by_name(td, font);
	
	if (cols <= 0) cols = 80;
	if (rows <= 0) rows = 24;
	if (i == 0) visible = 1;
	if (w <=0) w = td->font_size.w * cols;
	if (h <=0) h = td->font_size.h * rows;
	if ((x <= 0) && (y <= 0)) x = y = 100;
	
	td->location.x = x;
	td->location.y = y;
	
	td->size.w = w;
	td->size.h = h;
	
	td->cols = cols;
	td->rows = rows;
	td->visible = visible;
	
	if (td->tile.w == 0) td->tile.w = td->font_size.w;
	if (td->tile.h == 0) td->tile.h = td->font_size.h;
	
	arg_graphics = use_graphics = tile_set;
}

/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int * const x, int * const y, const int ox, const int oy)
{
	term_data *td = (term_data*)(Term->data);

	(*x) = (ox / td->font_size.w);
	(*y) = (oy / td->font_size.h) - 1;
}

gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	int z = 0;
	
	/* Where is the mouse */
	int x = event->x;
	int y = event->y;
	z = event->button;

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

gboolean on_window_toggle(GtkWidget *widget, GdkEvent *event, gpointer user_data)
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
	update_windows();
	
	return(td->visible);
}

void toggle_window(GtkWidget *wind)
{	
	bool window_is_visible;
	
	window_is_visible = (GTK_WIDGET_VISIBLE(GTK_WIDGET(wind)));
	
	if (window_is_visible)
		gtk_widget_hide(wind);
	else
		gtk_widget_show(wind);
}
gboolean toggle_message_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	term_data *main_term= &data[0];
	GtkWidget *wind;
	
	wind = glade_xml_get_widget(main_term->xml, "message_window");
	toggle_window(wind);
	
	return(TRUE);
}
gboolean toggle_inv_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	term_data *main_term= &data[0];
	GtkWidget *wind;
	
	wind = glade_xml_get_widget(main_term->xml, "inv_window");
	toggle_window(wind);
	
	return(TRUE);
}
gboolean toggle_equip_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	term_data *main_term= &data[0];
	GtkWidget *wind;
	
	wind = glade_xml_get_widget(main_term->xml, "equip_window");
	toggle_window(wind);
	
	return(TRUE);
}
gboolean toggle_monst_list_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	term_data *main_term= &data[0];
	GtkWidget *wind;
	
	wind = glade_xml_get_widget(main_term->xml, "monst_list_window");
	toggle_window(wind);
	
	return(TRUE);
}
gboolean toggle_debug_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	term_data *main_term= &data[0];
	GtkWidget *wind;
	
	wind = glade_xml_get_widget(main_term->xml, "debug_window");
	toggle_window(wind);
	
	return(TRUE);
}
static void update_term_menu(void)
{
	int i;
	GtkWidget *menu_item, *view_menu;
	
	term_data *main_term= &data[0];
	
	view_menu = glade_xml_get_widget(main_term->xml, "view_menu_menu");
	
	for ( i = 1; i < num_term; i++)
	{
		char item_name[20];
		term_data *td = &data[i];
		
		strnfmt(item_name, 16+1,  "term_menu_item_%i", i);
		
		menu_item = glade_xml_get_widget(main_term->xml, item_name);
		
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item)) != td->visible)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), td->visible);
	}	
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
			td->tile.w = td->font_size.w;
			td->tile.h = td-> font_size.h;
			
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

void setup_graphics_menu()
{
	GtkWidget *gr[4];
	term_data *main_term= &data[0];
	int i = 0;
	
	gr[GRAPHICS_NONE] = glade_xml_get_widget(main_term->xml, "none_gr_item");
	gr[GRAPHICS_ORIGINAL] = glade_xml_get_widget(main_term->xml, "old_gr_item");
	gr[GRAPHICS_ADAM_BOLT] = glade_xml_get_widget(main_term->xml, "adam_bolt_gr_item");
	gr[GRAPHICS_DAVID_GERVAIS] = glade_xml_get_widget(main_term->xml, "david_gervais_gr_item");
	
	for(i=0;i < 4; i++)
	{
		if ( i != arg_graphics)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gr[i])))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gr[i]), FALSE);
		}
	}
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gr[arg_graphics])))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gr[arg_graphics]), TRUE);
}

gboolean graphics_menu(int graphics_type)
{
	init_graf(graphics_type);
	setup_graphics_menu();
	return(TRUE);
}

/* Hooks for graphics menu items */
gboolean on_no_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
	{return(graphics_menu(GRAPHICS_NONE));}

gboolean on_old_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
	{return(graphics_menu(GRAPHICS_ORIGINAL));}

gboolean on_adam_bolt_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
	{return(graphics_menu(GRAPHICS_ADAM_BOLT));}

gboolean on_david_gervais_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
	{return(graphics_menu(GRAPHICS_DAVID_GERVAIS));}


static void update_windows(void)
{
	int i;
		
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Updating Windows!");
	#endif
	
	get_term_sizes();
	
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		if (td->visible)
		{
			set_window_size(td);
			Term_show(td);
		}
		else
		{
			Term_hide(td);
		}
	}
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
	term_data *main_term= &data[0];

	/* Store the name of the term */
	xd->name = xtra_names[i];
	xd->win_name = xtra_win_names[i];
	xd->text_view_name = xtra_text_names[i];
	xd->item_name = xtra_menu_names[i];
	
	xd->win = glade_xml_get_widget(main_term->xml, xd->win_name);
	xd->text_view = glade_xml_get_widget(main_term->xml, xd->text_view_name);
	xd->menu = glade_xml_get_widget(main_term->xml, xd->item_name);
	g_signal_connect(xd->win, "delete_event", GTK_SIGNAL_FUNC(xtra_hide_event_handler), xd);
	 #ifdef DISABLE_GTK_TESTING
	gtk_widget_set_sensitive(xd->menu, FALSE);
	#endif
	white_on_black_textview(xd);
}

static void init_gtk_windows(void)
{
	char buf[1024], logo[1024];
	GtkWidget *menu_item;
	int i = 0;
	bool err;
	term_data *main_term= &data[0];
	
	
	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, "angband.glade");
	
	main_term->xml = glade_xml_new(buf, NULL, NULL);
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Got Glade at %s.", buf);
	#endif
	main_term->window = glade_xml_get_widget(main_term->xml, "main-window");
	main_term->drawing_area = glade_xml_get_widget(main_term->xml, "drawingarea1");
	menu_item = glade_xml_get_widget(main_term->xml, "font_menu_item");
	
	g_signal_connect(main_term->drawing_area, "expose_event", G_CALLBACK(expose_event_handler), main_term);
	g_signal_connect(main_term->window, "configure_event", G_CALLBACK(configure_event_handler), main_term);
	g_signal_connect(menu_item, "activate", G_CALLBACK(change_font_event_handler), main_term);
	
	/* connect signal handlers that aren't passed data */
	glade_xml_signal_autoconnect(main_term->xml);
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "About to set up the graphics menu.");
	#endif
	
	setup_graphics_menu();
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Set up.");
	#endif
	
	/* Load Extra Windows */
	for (i = 0; i <= 4; i++)
	{
		xtra_data_init(&xdata[i],i);
	}

	/* Load the windows from xml */
	for (i = 1; i < num_term; i++)
	{
		
		term_data *td = &data[i];
	
		/* Set up the Glade file */
		td->xml = glade_xml_new(buf, "term-window", NULL);
		
		td->window = glade_xml_get_widget(td->xml, "term-window");
		td->drawing_area = glade_xml_get_widget(td->xml, "drawingarea2");
		
		g_signal_connect(td->window, "delete_event", GTK_SIGNAL_FUNC(hide_event_handler), td);
		g_signal_connect(td->window, "configure_event", G_CALLBACK(configure_event_handler), td);
		g_signal_connect(td->drawing_area, "expose_event", G_CALLBACK(expose_event_handler), td);
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Loaded term window %s", td->name);
		#endif
	}
	update_term_menu();
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		gtk_widget_realize(td->window);
		g_assert(td->window != NULL);
	
		gtk_widget_realize(td->drawing_area); 
		g_assert(td->drawing_area != NULL);
	
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Creating cairo surface for %s.", td->name);
		#endif
		
		td->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 255 * td->font_size.w, 255 * td->font_size.h);
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Creating cairo context.");
		#endif
		
		td->cr = cairo_create(td->surface);
		g_assert(td->surface != NULL);
		g_assert(td->cr != NULL);
		
		/* Set attributes */
		gtk_window_set_title(GTK_WINDOW(td->window), td->name);
		
		
		path_build(logo, sizeof(logo), ANGBAND_DIR_XTRA, "graf/mr_att.png");
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Getting %s.", logo);
		#endif
		
		err = gtk_window_set_default_icon_from_file(logo, NULL);
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Setting it as the icon.");
		#endif
		
		/* Activate the window screen */
		Term_activate(&data[i].t);
		
		Term_clear_gtk(); 
		
		#ifdef GTK_DEBUG
		gtk_log_fmt(TERM_WHITE, "Clearing the window and activating it.");
		#endif
	}
	
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
	
	load_prefs(td, i);
	
	/* Success */
	return (0);
}

 static game_command get_init_cmd()
{
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Get_init_cmd");
	#endif
	
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
	gtk_log_fmt(TERM_WHITE, "Monster targetting.");
}

/*
 * Set up color tags for all the angband colors.
 */
static void init_color_tags(GtkTextBuffer *buf)
{
	int i;
	char colorname[10] = "color-00";
	GtkTextTagTable *tags;
	
	tags = gtk_text_buffer_get_tag_table(buf);
	
	for (i = 0; i <= 15; i++)
	{
		strnfmt(colorname, 8 + 1,  "color-%d", i);
		if ( gtk_text_tag_table_lookup(tags, colorname) == NULL)
			gtk_text_buffer_create_tag(buf, colorname, "foreground", color_string_table[i],"font", "monospace 10", NULL);
	}
}
#endif

void text_view_print(GtkTextBuffer *buf, char *str, byte color)
{
	char colorname[10] = "color-00";
	GtkTextIter start, end;
	
	/* Get the color tag ready */
	strnfmt(colorname, 8 + 1,  "color-%d", color);
	
	strnfmt(str, strlen(str) +2, "%s\n", str);
	
	/* Print it at the end of the window */
	gtk_text_buffer_get_bounds(buf, &start, &end);
	gtk_text_buffer_insert_with_tags_by_name(buf, &end, str, strlen(str), colorname,  NULL);
}

void gtk_log_fmt(byte c, cptr fmt, ...)
{
	#ifndef DISABLE_GTK_TESTING
	term_data *td = (term_data*)(Term->data);
	xtra_win_data *xd = &xdata[4];
	GtkTextBuffer *buf = NULL;
	#endif
	char *res, str[80];
	va_list vp;
	int n;
	
	#ifndef DISABLE_GTK_TESTING
	if (td->xml != NULL)
	{
		buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(xd->text_view));
		init_color_tags(buf);
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
	plog(str);
	#endif
	
	#ifndef DISABLE_GTK_TESTING
	if (td->xml != NULL)
		text_view_print(buf, str, c);
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
	GtkTextBuffer *buf;
	char str[80];
	
	if (xd != NULL)
	{
	buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), buf);
	
	init_color_tags(buf);
	
	num = messages_num();
	
	for (i = 0; i < num; i++)
	{
		c = message_color(i);

		strnfmt(str, strlen(message_str(i)) + 1, "%s", message_str(i));
		text_view_print(buf, str, c);
	}
}
}

/* 
 * Return the last inventory slot.
 */
int last_inv_slot(void)
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

void inv_slot(char *str, int i, bool equip)
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
		
		if (item_tester_okay(o_ptr))
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
	GtkTextBuffer *buf;
	
	if (xd != NULL)
	{
	buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), buf);
	
	init_color_tags(buf);
	
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
			text_view_print(buf, str, attr);
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
	GtkTextBuffer *buf;
	
	if (xd != NULL)
	{
	buf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), buf);
	
	init_color_tags(buf);
	
	/* Display the pack */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		char str[80];
	
		/* Examine the item */
		o_ptr = &inventory[i];
		
		/* Is this item "acceptable"? */
		attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];
		
		inv_slot(str, i, TRUE);
		text_view_print(buf, str, attr);
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
	GtkTextBuffer *tbuf;
	
	if (xd != NULL)
	{
	tbuf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW (xd->text_view), tbuf);
	
	init_color_tags(tbuf);

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
		text_view_print(tbuf, str, TERM_SLATE);

		/* Free up memory */
		FREE(race_count);

		/* Done */
		return;
	}
	
	strnfmt(str, 25 + 1, "You can see %d monster%s:", total_count, (total_count > 1 ? "s" : ""));
	text_view_print(tbuf, str, 1);
	
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
		
		text_view_print(tbuf, buf, attr);
		line++;
	}

	/* Free the race counters */
	FREE(race_count);
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
static void handle_sidebar(game_event_type type, game_event_data *data, void *user)
{
	gtk_log_fmt(TERM_WHITE, "Showing the sidebar.");
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
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;
	}

	/* Init the windows */
	init_gtk_windows(); 
	
	/* Initialise hooks and handlers */
	init_handlers();
	
	Term_activate(&data[0].t);
	
	/* Set the system suffix */
	ANGBAND_SYS = "gtk";

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Catching signals.");
	#endif
	
	/* Catch nasty signals */
	signals_init();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	
	Term_fresh();
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Updating Windows.");
	#endif
	
	update_windows();
	game_in_progress = FALSE;

	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Initializing Angband.");
	#endif
	
	/* Set up the display handlers and things. */
	init_display();
	/* Let's play */
	play_game();
	
	
	#ifdef GTK_DEBUG
	gtk_log_fmt(TERM_WHITE, "Going into the main loop.");
	#endif
	
	/* Processing loop */
	gtk_main();

	/* Stop now */
	exit(0);

	/* Success */
	return (0);
}

#endif  /*USE_GTK */
