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
#include "buildid.h"
#include "player/player.h"

#ifdef USE_GTK
#include "main-gtk.h"
#include "textui.h"
#include "files.h"
#include "init.h"

/* this is used to draw the various terrain characters */
static unsigned int graphics_table[32] = {
	000, '*', '#', '?', '?', '?', '?', '.',
	'+', '?', '?', '+', '+', '+', '+', '+',
	'~', '-', '-', '-', '_', '+', '+', '+',
	'+', '|', '?', '?', '?', '?', '?', '.',
};

iconv_t conv;

/* 
 *Add a bunch of debugger message, to trace where problems are. 
 */
/*#define GTK_DEBUG*/

/* 
 *Write all debugger messages to the command line, as well as the debugger window. 
*/
#define VERBOSE_DEBUG

#ifdef GTK_DEBUG
static void surface_status(const char *function_name, term_data *td)
{
	if (td->surface == NULL) 
		glog("No Surface for %s in '%s'", function_name, td->name);
	else
		if (cairo_surface_status(td->surface) != CAIRO_STATUS_SUCCESS)
			glog("%s: Surface status for '%s': %s",function_name,td->name, cairo_status_to_string(cairo_surface_status(td->surface)));
}
#endif

/*#define USE_GTK_BUILDER*/

#ifdef USE_GTK_BUILDER
#define GTK_XML "angband.xml"

static GtkWidget *get_widget(GtkBuilder *xml, const char *name)
{
	return GTK_WIDGET(gtk_builder_get_object(xml, name));
}

static GtkBuilder *get_gtk_xml(const char *buf, const char *secondary)
{
	GtkBuilder *xml;
	int e = 0;
	
	xml = gtk_builder_new();
	e = gtk_builder_add_from_file (xml, buf, NULL);
	
	return xml;
}

#else
#define GTK_XML "angband.glade"

static GtkWidget *get_widget(GladeXML *xml, const char *name)
{
	return GTK_WIDGET(glade_xml_get_widget(xml, name));
}

static GladeXML *get_gtk_xml(const char *buf, const char *secondary)
{
	return glade_xml_new(buf, secondary, NULL);
}
#endif

static void recreate_surface(term_data *td)
{
	if ((td->drawing_area != NULL) && (cairo_surface_get_reference_count(td->surface) > 0))
	{
		cairo_surface_destroy(td->surface);
	}
	
	td->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, td->size.w, td->size.h);
	set_term_matrix(td);	
}

static void set_term_menu(int number, bool value)
{
	term_data *td = &data[number];
	
	if (td->menu_item != NULL) 
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(td->menu_item), value);

}

static bool get_term_menu(int number)
{
	bool value = FALSE;
	term_data *td = &data[number];
	
	if (td->menu_item != NULL) 
		value = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(td->menu_item));
	
	return value;

}

static int xtra_window_from_name(const char *s)
{
	xtra_win_data *xd;
	int i = 0,t = -1;
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xd = &xdata[i];
		if (GTK_IS_WIDGET(xd->win) && (xd->win != NULL) && (s == gtk_widget_get_name(xd->win)) )
			t = i;
	}
	return(t);
}

static int xtra_window_from_drawing_area(const char *s)
{
	xtra_win_data *xd;
	int i = 0,t = -1;
	for (i = 0; i < MAX_XTRA_WIN_DATA; i ++)
	{
		xd = &xdata[i];
		if ((xd->drawing_area != NULL) && (s == gtk_widget_get_name(xd->drawing_area))) t = i;
	}
	return(t);
}

static int xtra_window_from_widget(GtkWidget *widget)
{
	return xtra_window_from_name(gtk_widget_get_name(widget));
}

static int td_from_name(const char *s, const char *pattern)
{
	int t = -1;
	sscanf(s, pattern, &t);
	return(t);
}
static int term_window_from_name(const char *s)
{
	return td_from_name(s, "term_window_%d");
}
static int td_from_widget(GtkWidget *widget, const char *pattern)
{
	return td_from_name(gtk_widget_get_name(widget), pattern);
}

static int term_window_from_widget(GtkWidget *widget)
{
	return term_window_from_name(gtk_widget_get_name(widget));
}

static int max_win_width(term_data *td)
{
	return (255 * td->font.w);
}

static int max_win_height(term_data *td)
{
	return(255 * td->font.h);
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

void set_term_matrix(term_data *td)
{
	/* Get a matrix set up to scale the graphics. */
	if ((td->surface != NULL) && (td->visible)) 
		matrix = cairo_font_scaling(td->surface, td->tile.w, td->tile.h, td->actual.w, td->actual.h);
}

gboolean on_big_tiles(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	term_data *td = &data[0];
	
	if ((widget != NULL) && (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))))
		tile_width = 1;
	else
		tile_width = 2;
	
	load_font_by_name(td, td->font.name);
	term_data_redraw(td);
	return(TRUE);
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

static void invalidate_drawing_area(GtkWidget *widget, cairo_rectangle_t r)
{
	GdkRectangle s;
	
	s = cairo_rect_to_gdk(&r);
	if (widget->window != NULL)
	{
		gdk_window_invalidate_rect(widget->window, &s, TRUE);
	}
}

void set_row_and_cols(term_data *td)
{
	int cols = td->cols;
	int rows = td->rows;
	
	td->cols = td->size.w / td->font.w;
	td->rows= td->size.h / td->font.h;
	
	if (MAIN_WINDOW(td))
	{
		if (td->cols < 80) td->cols = cols;
		if (td->rows < 24) td->rows = rows;
	}
	
	if ((cols != td->cols) || (rows != td->rows))
	{ 
		recreate_surface(td);
		term_data_redraw(td);
	}
}
static void redraw_term(term_data *td)
{		
	set_window_size(td);
		
	recreate_surface(td);
	term_data_redraw(td);
	force_redraw();
}

gboolean set_term_row(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	int t = td_from_widget(widget, "row_%d");
	
	if (t != -1)
	{
		term_data *td = &data[t];
		td->rows = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
		redraw_term(td);
	}
	return(FALSE);
}

gboolean set_term_col(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	int t = td_from_widget(widget, "col_%d");
	
	if (t != -1)
	{
		term_data *td = &data[t];
		td->cols = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
		redraw_term(td);
	}
	return(FALSE);
}

/* 
 * Get the position of the window and save it.
 */
gboolean configure_event_handler(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	int t = -1;
	term_data *td;
	
	t = term_window_from_widget(widget);
	
	if (t != -1)
	{
		td = &data[t];
		
		if (td->initialized)
		{
			int x = 0, y = 0, w = 0, h = 0;
			GdkRectangle r;
			
			gdk_window_get_frame_extents(td->win->window, &r);
			x = r.x;
			y = r.y;
			
			gtk_window_get_size(GTK_WINDOW(td->win), &w, &h);
			
			td->location.x = x;
			td->location.y = y;
			
			if (w != 0) td->size.w = w;
			if (h != 0)  td->size.h = h;
		}
	}
	return(FALSE);
}

gboolean xtra_configure_event_handler(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
	xtra_win_data *xd;
	int t;
	
	t = xtra_window_from_widget(widget);
	if (t != -1)
	{
		int x = 0, y = 0, w = 0, h = 0;
		GdkRectangle r;
		
		xd = &xdata[t];
	
		if (GTK_WIDGET_VISIBLE(xd->win))
		{
			gdk_window_get_frame_extents(xd->win->window, &r);
			x = r.x;
			y = r.y;
			
			gtk_window_get_size(GTK_WINDOW(xd->win), &w, &h);
		
			xd->location.x = x;
			xd->location.y = y;
			if (w != 0) xd->size.w = w;
			if (h != 0)  xd->size.h = h;
		
			xd->visible = TRUE;
		}
		else
			xd->visible = FALSE;
	}
	return(FALSE);
}

static void set_window_defaults(term_data *td)
{
	GdkGeometry geo;
	
	geo.width_inc = td->font.w;
	geo.height_inc = td->font.h;
	
	geo.max_width =(td->font.w * td->cols);
	geo.max_height = (td->font.h * td->rows);
	
	if (MAIN_WINDOW(td))
	{
		geo.min_width = geo.max_width;
		geo.min_height = geo.max_height;
	}
	else
	{
		geo.min_width = td->font.w;
		geo.min_height = td->font.h;
	}
	
	gtk_window_set_geometry_hints(GTK_WINDOW(td->win), td->drawing_area, &geo, 
	GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
}

static void set_window_size(term_data *td)
{
	set_window_defaults(td);
	gtk_window_set_position(GTK_WINDOW(td->win),GTK_WIN_POS_NONE);
	
	if MAIN_WINDOW(td)
	{
		gtk_widget_set_size_request(GTK_WIDGET(td->drawing_area), td->size.w, td->size.h);
	}
	
	gtk_window_resize(GTK_WINDOW(td->win), td->size.w, td->size.h);
	
	gtk_window_move(GTK_WINDOW(td->win), td->location.x, td->location.y);
	
	gtk_window_get_size(GTK_WINDOW(td->win), &td->size.w, &td->size.h);
}

static void set_xtra_window_size(xtra_win_data *xd)
{
	gtk_window_set_position(GTK_WINDOW(xd->win),GTK_WIN_POS_NONE);
	if ((xd->size.w > 0) && (xd->size.h > 0))
		gtk_window_resize(GTK_WINDOW(xd->win), xd->size.w, xd->size.h);
	gtk_window_move(GTK_WINDOW(xd->win), xd->location.x, xd->location.y);
}

int button_add_gui(const char *label, unsigned char keypress)
{
	int i=0;
	bool assigned = FALSE;
	GtkWidget *widget;
	int length = strlen(label);
	
	/*static GtkWidget *toolbar;
	static GtkWidget *toolbar_items[12];
	static int toolbar_size;
	static unsigned char toolbar_keypress[12];*/
	

	/* Check we haven't already got a button for this keypress */
	for (i=0; i < toolbar_size; i++)
	{
		if (toolbar_keypress[i] == keypress)
			assigned = TRUE;
	}
	/* Make the button */
	if (!assigned)
	{
		widget = gtk_button_new_with_label(label);
		toolbar_size++;
		toolbar_keypress[toolbar_size]=keypress;
		toolbar_items[toolbar_size] = widget;
		/*gtk_container_add(toolbar, widget);*/
	}

	/* Return the size of the button */
	return (length);
}

int button_kill_gui(unsigned char keypress)
{
	/*int length;*/

	/* Find the button */

	/* No such button */
	if (0)
	{
		return 0;
	}

	/* Find the length */

	/* Move each button up one */

	/* Wipe the data */

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);
	redraw_stuff(p_ptr);

	/* Return the size of the button */
	return 0 /*(length)*/;
}
/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, 0, 0, max_win_width(td), max_win_height(td));
	cairo_clear(td->surface, r, TERM_DARK);
	invalidate_drawing_area(td->drawing_area, r);

	return (0);
}

/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, (td->actual.w * x), (td->actual.h * y), (td->actual.w * n), (td->actual.h));
	cairo_clear(td->surface, r, TERM_DARK);
	invalidate_drawing_area(td->drawing_area, r);

	return (0);
}

static byte Term_xchar_gtk(byte c)
{
	/* Can't translate Latin-1 to UTF-8 here since we have to return a byte. */
	return c;
}

char *process_control_chars(int n, const char *s)
{
	char *s2 = (char *)malloc(sizeof(char) * n);
	int i;
	for (i = 0; i < n; i++) {
		unsigned char c = s[i];
		if (c < 32) {
			s2[i] = graphics_table[c];
		} else if (c == 127) {
			s2[i] = '#';
		} else {
			s2[i] = c;
		}
	}

	return s2;
}

char *latin1_to_utf8(int n, const char *s)
{
	size_t inbytes = n;
	char *s2 = process_control_chars(n, s);
	char *p2 = s2;

	size_t outbytes = 4 * n;
	char *s3 = (char *)malloc(sizeof(char) * outbytes);
	char *p3 = s3;

	size_t result = iconv(conv, &p2, &inbytes, &p3, &outbytes);

	if (result == (size_t)(-1)) {
		printf("iconv() failed: %d\n", errno);
		free(s3);
		return s2;
	} else {
		free(s2);
		return s3;
	}
}

/*
 * Draw some textual characters.
 */
static errr Term_text_gtk(int x, int y, int n, byte a, const char *s)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;

	char *s2;
	if (conv == NULL)
		s2 = process_control_chars(n, s);
	else
		s2 = latin1_to_utf8(n, s);

	init_cairo_rect(&r, (td->font.w * x), (td->font.h * y),  (td->font.w * n), td->font.h);
	draw_text(td->surface, &td->font, &td->actual, x, y, n, a, s2);
	invalidate_drawing_area(td->drawing_area, r);
	
	free(s2);
	return (0);
}

static errr Term_pict_gtk(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, (td->actual.w * x), (td->actual.h * y),  (td->actual.w * n), (td->actual.h));
	draw_tiles(td->surface, x, y, n, ap, cp, tap, tcp, &td->font, &td->actual, &td->tile);
	invalidate_drawing_area(td->drawing_area, r);
			
	return (0);
}

static errr Term_flush_gtk(void)
{
	gdk_flush();
	return (0);
}

static errr Term_fresh_gtk(void)
{
	term_data *td = (term_data*)(Term->data);
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, 0, 0, max_win_width(td), max_win_height(td));
	invalidate_drawing_area(td->drawing_area, r);
	gdk_window_process_updates(td->win->window, 1);
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
	
	init_cairo_rect(&r, row_in_pixels(td, x)+1, col_in_pixels(td, y) + 1,  (td->actual.w) - 2, (td->actual.h) - 2);
	cairo_cursor(td->surface, r, TERM_SLATE);
	invalidate_drawing_area(td->drawing_area, r);
	
	return (0);
}

/*
 * Hack -- redraw a term_data.
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td)
{
	term *old = Term;
	
	/* Activate the term passed to it, not term 0! */
	Term_activate(&td->t);

	Term_resize(td->cols, td->rows);
	Term_redraw();
	Term_fresh();
	
	Term_activate(old);
}

static void force_redraw()
{
	if (game_in_progress)
	{
		reset_visuals(TRUE);
		Term_key_push(KTRL('R'));
	}
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
			glog( "You may not do that right now.");
			return(FALSE);
		}

		/* Hack -- Forget messages */
		msg_flag = FALSE;
		
		/* Save the game */
		save_game();
	}
	
	return(TRUE);
}

static void hook_quit(const char *str)
{
	gtk_log_fmt(TERM_RED,"%s", str);
	save_prefs();
	release_memory();
	exit(0);
}

gboolean quit_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (save_game_gtk())
	{
		save_prefs();
		quit(NULL);
		exit(0);
		return(FALSE);
	}
	else
		return(TRUE);
}

gboolean destroy_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	quit(NULL);
	exit(0);
	return(FALSE);
}

gboolean new_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if (game_in_progress)
	{
		glog( "You can't start a new game while you're still playing!");
		return(TRUE);
	}
	else
	{
		/* We'll return NEWGAME to the game. */ 
		cmd_insert(CMD_NEWGAME);
		gtk_widget_set_sensitive(widget, FALSE);
		return(FALSE);
	}
}

static void load_font_by_name(term_data *td, const char *font_name)
{	
	PangoFontDescription *temp_font;
	char buf[80];
	unsigned int i, j = 0;

	my_strcpy(td->font.name, font_name, sizeof(td->font.name));

	for (i = 0; i < strlen(font_name); i++)
		if (font_name[i] == ' ') 
			j = i;
	
	for (i = j; i < strlen(font_name); i++)
		buf[i - j] = font_name[i];

	temp_font = pango_font_description_from_string(font_name);
	my_strcpy(td->font.family, pango_font_description_get_family(temp_font), sizeof(td->font.family));
	td->font.size = atof(buf);
	if (td->font.size == 0) td->font.size = 12;
	get_font_size(&td->font);
	
	td->actual.w = td->font.w * tile_width;
	td->actual.h = td->font.h;
	
	td->size.w = td->cols * td->font.w;
	td->size.h = td->rows * td->font.h;
	
	if ((td->initialized == TRUE) && (td->win != NULL)  && (td->visible))
	{
		/* Get a matrix set up to scale the graphics. */
		set_term_matrix(td);
		
		gtk_widget_hide(td->win);
		set_window_size(td);
		gtk_widget_show(td->win);
		
		term_data_redraw(td);
	}
}

void set_term_font(GtkFontButton *widget, gpointer user_data)   
{
	term_data *td;

	const char *font_name;
	const char *s;
	int t;
	
	s = gtk_widget_get_name(GTK_WIDGET(widget));
	sscanf(s, "term_font_%d", &t);

	td = &data[t];

	font_name = gtk_font_button_get_font_name(widget);
	load_font_by_name(td, font_name);
}

void set_xtra_font(GtkFontButton *widget, gpointer user_data)   
{
	const char *temp, *s;
	int i, t = 0;
	xtra_win_data *xd;
	
	s = gtk_widget_get_name(GTK_WIDGET(widget));
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i ++)
	{
		xtra_win_data *xd = &xdata[i];
		if (s == xd->font_button_name) t = i;
	}
	
	xd=&xdata[t];
	
	temp = gtk_font_button_get_font_name(widget);
	my_strcpy(xd->font.name, temp, sizeof(xd->font.name));
	get_font_size(&xd->font);
	if (xd->text_view != NULL)
		gtk_widget_modify_font(GTK_WIDGET(xd->text_view), pango_font_description_from_string(xd->font.name));
	
	if ((xd->surface != NULL) && (xd->event != 0))
	{
		event_signal(xd->event);
	}
}

/*** Callbacks: savefile opening ***/

/* Filter function for the savefile list */
static gboolean file_open_filter(const GtkFileFilterInfo *filter_info, gpointer data)
{
	const char *name = filter_info->display_name;

	(void)data;

	/* Count out known non-savefiles */
	if (!strcmp(name, "Makefile") ||
	    !strcmp(name, "delete.me"))
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
	GtkFileFilter *filter;
	GtkWidget *selector_wid;
	GtkFileChooser *selector;
	bool accepted;
	
	char buf[1024];
	const char *filename;

	/* Forget it if the game is in progress */
	/* XXX Should disable the menu entry XXX */
	if (game_in_progress && !save)
	{
		glog( "You can't open a new game while you're still playing!");
		return(FALSE);
	}
			
	if ((!game_in_progress || !character_generated) && save)
	{
		glog( "You can't save a new game unless you're still playing!");
		return(FALSE);
	}

	if (!inkey_flag && save)
	{
		glog( "You may not do that right now.");
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
		game_saved = TRUE;
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
	if (accepted) cmd_insert(CMD_LOADFILE);
	
	/* Done */
	return;
}

gboolean save_as_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
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

gboolean save_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	bool accepted;
	
	if (game_saved)
		save_game();
	else
	{
		accepted = save_dialog_box(TRUE);
	
		if (accepted)
		{
			Term_flush();
			save_game_gtk();
		}
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
	byte mods;

	int ch = 0;
	guint modifiers = gtk_accelerator_get_default_mod_mask();

	/* Extract four "modifier flags" */
	int mc = ((event->state & modifiers) == GDK_CONTROL_MASK) ? TRUE : FALSE;
	int ms = ((event->state & modifiers) == GDK_SHIFT_MASK) ? TRUE : FALSE;
	int mo = ((event->state & modifiers) == GDK_MOD1_MASK) ? TRUE : FALSE;
	int mx = ((event->state & modifiers) == GDK_MOD3_MASK) ? TRUE : FALSE;
	int kp = FALSE;

	/* see gdk/gdkkeysyms.h */
	// http://www.koders.com/c/fidD9E5E78FD91FE6ABDD6D3F78DA5E4A0FADE79933.aspx
	switch (event->keyval) {
		case GDK_Shift_L: case GDK_Shift_R: case GDK_Control_L:
		case GDK_Control_R: case GDK_Caps_Lock: case GDK_Shift_Lock:
		case GDK_Meta_L: case GDK_Meta_R: case GDK_Alt_L: case GDK_Alt_R:
		case GDK_Super_L: case GDK_Super_R: case GDK_Hyper_L:
		case GDK_Hyper_R:
			/* ignore things that are just modifiers */
			return TRUE;

		case GDK_BackSpace: ch = KC_BACKSPACE; break;
		case GDK_Tab: ch = KC_TAB; break;
		case GDK_Return: ch = KC_RETURN; break;
		case GDK_Escape: ch = ESCAPE; break;
		case GDK_Delete: ch = KC_DELETE; break;

		case GDK_Home: ch = KC_HOME; break;
		case GDK_Left: ch = ARROW_LEFT; break;
		case GDK_Up: ch = ARROW_UP; break;
		case GDK_Right: ch = ARROW_RIGHT; break;
		case GDK_Down: ch = ARROW_DOWN; break;
		case GDK_Page_Up: ch = KC_PGUP; break;
		case GDK_Page_Down: ch = KC_PGDOWN; break;
		case GDK_End: ch = KC_END; break;
		case GDK_Insert: ch = KC_INSERT; break;

		/* keypad */
		case GDK_KP_0: case GDK_KP_1: case GDK_KP_2:
		case GDK_KP_3: case GDK_KP_4: case GDK_KP_5:
		case GDK_KP_6: case GDK_KP_7: case GDK_KP_8:
		case GDK_KP_9: kp = TRUE; break;

		case GDK_KP_Decimal: ch = '.'; kp = TRUE; break;
		case GDK_KP_Divide: ch = '/'; kp = TRUE; break;
		case GDK_KP_Multiply: ch = '*'; kp = TRUE; break;
		case GDK_KP_Subtract: ch = '-'; kp = TRUE; break;
		case GDK_KP_Add: ch = '+'; kp = TRUE; break;
		case GDK_KP_Enter: ch = '\n'; kp = TRUE; break;
		case GDK_KP_Equal: ch = '='; kp = TRUE; break;

		case GDK_F1: ch = KC_F1; break;
		case GDK_F2: ch = KC_F2; break;
		case GDK_F3: ch = KC_F3; break;
		case GDK_F4: ch = KC_F4; break;
		case GDK_F5: ch = KC_F5; break;
		case GDK_F6: ch = KC_F6; break;
		case GDK_F7: ch = KC_F7; break;
		case GDK_F8: ch = KC_F8; break;
		case GDK_F9: ch = KC_F9; break;
		case GDK_F10: ch = KC_F10; break;
		case GDK_F11: ch = KC_F11; break;
		case GDK_F12: ch = KC_F12; break;
		case GDK_F13: ch = KC_F13; break;
		case GDK_F14: ch = KC_F14; break;
		case GDK_F15: ch = KC_F15; break;
	}

	mods = (mo ? KC_MOD_ALT : 0) | (mx ? KC_MOD_META : 0) |
			(kp ? KC_MOD_KEYPAD : 0);

	if (ch) {
		mods |= (mc ? KC_MOD_CONTROL : 0) | (ms ? KC_MOD_SHIFT : 0);
		Term_keypress(ch, mods);
	} else if (event->length == 1) {
		keycode_t code = event->string[0];

		if (mc && MODS_INCLUDE_CONTROL(code)) mods |= KC_MOD_CONTROL;
		if (ms && MODS_INCLUDE_SHIFT(code)) mods |= KC_MOD_SHIFT;

		Term_keypress(code, mods);

		/* Control keys get passed along to menus, are not "handled" */
		return !mc;
	}

	return TRUE;
}

static void save_prefs(void)
{
	ang_file *fff;
	int i;

	#ifdef GTK_DEBUG
	glog( "Saving Prefs.");
	#endif
	
	/* Open the settings file */
	fff = file_open(settings, MODE_WRITE, FTYPE_TEXT);
	if (!fff) return;

	/* Header */
	file_putf(fff, "# %s GTK preferences\n\n", VERSION_NAME);
	
	file_putf(fff, "[General Settings]\n");
	/* Graphics setting */
	file_putf(fff,"Tile set=%d\n", arg_graphics);
	file_putf(fff,"Tile Width=%d\n", tile_width);
	file_putf(fff,"Tile Height=%d\n", tile_height);

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
		file_putf(fff, "visible=%d\n", GTK_WIDGET_VISIBLE(td->win));
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
	const char *str;
	int val;
	char buf[1024];
	
	strnfmt(buf, -1, name, i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	
	if (val <= 0) val = dfault;
		
	return val;
}

static int get_value(const char *buf)
{
	const char *str;
	int i;
	
	str = strstr(buf, "=");
	i = (str != NULL) ? atoi(str + 1) : -1;
	
	return i;
}

static void load_term_prefs()
{
	int t = 0, x = 0, line = 0, val = 0, section = 0;
	const char *str;
	char buf[1024];
	ang_file *fff;
	term_data *td = &data[0];
	xtra_win_data *xd = &xdata[0];

	if (ignore_prefs) return;

	/* Build the filename and open the file */
	path_build(settings, sizeof(settings), ANGBAND_DIR_USER, "gtk-settings.prf");
	fff = file_open(settings, MODE_READ, -1);

	/* File doesn't exist */
	if (!fff) return;

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
				arg_graphics = val;
				continue;
			}
			if (prefix(buf, "Tile Width="))
			{
				val = get_value(buf);
				sscanf(buf, "Tile Width=%d", &val);
				tile_width = val;
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

	file_close(fff);
}

static void load_prefs()
{
	int i;
	load_term_prefs();
	
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		/* deal with env vars */
		td->location.x = check_env_i("ANGBAND_X11_AT_X_%d", i, td->location.x);
		td->location.y = check_env_i("ANGBAND_X11_AT_Y_%d", i,td->location.y);
		td->cols = check_env_i("ANGBAND_X11_COLS_%d", i,  td->cols);
		td->rows = check_env_i("ANGBAND_X11_ROWS_%d", i, td->rows);
		td->initialized = FALSE;
		
		if ((td->font.name == NULL) || (strlen(td->font.name)<2)) 
			my_strcpy(td->font.name, "Monospace 12", sizeof(td->font.name));
		
		load_font_by_name(td, td->font.name);
	
		/* Set defaults if not initialized, or if funny values are there */
		if (td->size.w <= 0) td->size.w = (td->rows * td->actual.h);
		if (td->size.h <= 0) td->size.h = (td->cols * td->actual.w);
		if (td->tile.w <= 0) td->tile.w = td->font.w;
		if (td->tile.h <= 0) td->tile.h = td->font.h;

		/* The main window should always be visible */
		if (i == 0) td->visible = 1;

		/* Discard the old cols & rows values, and recalculate. */
		set_row_and_cols(td);
		
	}

	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		if ((xd->font.name == NULL) || (strlen(xd->font.name)<2)) 
			my_strcpy(xd->font.name, "Monospace 10", sizeof(xd->font.name));

		get_font_size(&xd->font);
		
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

	/* Where is the mouse? */
	int x = event->x;
	int y = event->y;
	z = event->button;
	
	/* The co-ordinates are only used in Angband format. */
	pixel_to_square(&x, &y, x, y);
	Term_mousepress(x, y, z);

	return FALSE;
}

gboolean expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;
	cairo_rectangle_t s;
	GdkRectangle *rects;
	int n_rects;
	int i;
	term_data *td;
	int t = -1;
	const char *str;
	
	str = gtk_widget_get_name(widget);
	sscanf(str, "term_drawing_area_%d", &t);
	
	if (t != -1)
	{
		td = &data[t];
		g_assert(td->drawing_area->window != 0);

		if (td->win)
		{
			g_assert(widget->window != 0);
		
			cr = gdk_cairo_create(td->drawing_area->window);
		
			gdk_region_get_rectangles (event->region, &rects, &n_rects);
		for (i = 0; i < n_rects; i++)
			{
				s = gdk_rect_to_cairo(&rects[i]);
				cairo_draw_from_surface(cr, td->surface,  s);
			}
			g_free (rects);
			cairo_destroy(cr);
		}
	}
	return (TRUE);
}

gboolean xtra_expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;
	cairo_rectangle_t s;
	GdkRectangle *rects;
	int n_rects;
	int i,t;
	xtra_win_data *xd;

	t = xtra_window_from_drawing_area(gtk_widget_get_name(widget));
	
	if (t != -1)
	{
		xd = &xdata[t];
	
		if ((xd->win) && (xd->drawing_area != NULL))
		{
			cr = gdk_cairo_create(xd->drawing_area->window);
		
			gdk_region_get_rectangles (event->region, &rects, &n_rects);
			for (i = 0; i < n_rects; i++)
			{
				s = gdk_rect_to_cairo(&rects[i]);
				cairo_draw_from_surface(cr, xd->surface,  s);
			}
			g_free (rects);
			cairo_destroy(cr);
		}
	}
	return (TRUE);
}

gboolean hide_options(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

gboolean show_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	term_data *td;
	measurements size;
	int t;
	t = term_window_from_widget(widget);
	if (t != -1)
	{
		td = &data[t];
		
		if (td->drawing_area != NULL)
		{
			size.w = max_win_width(td);
			size.h = max_win_height(td);
		
			td->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.w, size.h);
			set_term_matrix(td);
		}
	}
	return(TRUE);
}

gboolean xtra_show_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	xtra_win_data *xd;
	cairo_rectangle_t r;
	int t;
	
	t = xtra_window_from_widget(widget);
	if (t != -1)
	{
		xd = &xdata[t];
		if (xd->drawing_area != NULL)
		{
			xd->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 255 * xd->font.w, 255 * xd->font.h);
			init_cairo_rect(&r, 0, 0, 255 * xd->font.w, 255 * xd->font.h);
			cairo_clear(xd->surface, r, TERM_DARK);
		}
			
		if ((game_in_progress) && (character_generated) && (xd->event != 0))
		{
			event_signal(xd->event);
		}
	}
	return(TRUE);
}

gboolean xtra_hide_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	xtra_win_data *xd;
	int t;
	t = xtra_window_from_widget(widget);
	if (t != -1)
	{
		xd = &xdata[t];
		xd->visible = FALSE;
		if (xd->menu != NULL)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(xd->menu), FALSE);
	
		if ((xd->drawing_area != NULL) && (cairo_surface_get_reference_count(xd->surface) > 0))
		{
			cairo_surface_destroy(xd->surface);
		}
	}
	return(TRUE);
}

gboolean hide_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	term_data *td;
	int t;
	
	t = term_window_from_widget(widget);
	if (t > 0)
	{
		td = &data[t];
		if ((td != NULL) && (td->win == widget))
			set_term_menu(td->number, FALSE);
		
		if ((td->drawing_area != NULL) && (td->surface != NULL))
		{
			int count = cairo_surface_get_reference_count(td->surface);
			if (count > 0) cairo_surface_destroy(td->surface);
		}
	}
	return(TRUE);
}

gboolean toggle_term_window(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	term_data *td;

	int t;
	const char *s;

	s = gtk_widget_get_name(widget);
	sscanf(s, "term_menu_item_%d", &t);
	td = &data[t];

	if (widget != NULL)
		td->visible = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	
	if (td->visible)
	{
		gtk_widget_show(td->win);
		term_data_redraw(td);
	}
	else
	{
		gtk_widget_hide(td->win);
	}

	return td->visible;
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
			
			if (widget != NULL)
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
	int i = 0;
	
	arg_graphics = g;
	
	switch(arg_graphics)
	{
		case GRAPHICS_NONE:
		{
			ANGBAND_GRAF = "none";
			
			use_transparency = FALSE;
			td->tile.w = td->font.w;
			td->tile.h = td-> font.h;
			break;
		}

		case GRAPHICS_ORIGINAL:
		{
			ANGBAND_GRAF = "old";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "8x8.png");
			use_transparency = FALSE;
			td->tile.w = td->tile.h = 8;
			break;
		}

		case GRAPHICS_ADAM_BOLT:
		{
			ANGBAND_GRAF = "new";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "16x16.png");
			use_transparency = TRUE;
			td->tile.w = td->tile.h =16;
			break;
		}

		case GRAPHICS_NOMAD:
		{
			ANGBAND_GRAF = "nomad";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "8x16.png");
			use_transparency = TRUE;
			td->tile.w = td->tile.h =16;
			break;
		}

		case GRAPHICS_DAVID_GERVAIS:
		{
			ANGBAND_GRAF = "david";
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, "32x32.png");
			use_transparency = FALSE;
			td->tile.w = td->tile.h =32;
			break;
		}
	}

	/* Free up old graphics */
	if (graphical_tiles != NULL) cairo_surface_destroy(graphical_tiles);
	if (tile_pattern != NULL) cairo_pattern_destroy(tile_pattern);
		
	graphical_tiles = cairo_image_surface_create_from_png(buf);
	tile_pattern = cairo_pattern_create_for_surface(graphical_tiles);
	
	g_assert(graphical_tiles != NULL);
	g_assert(tile_pattern != NULL);
	
	/* All windows have the same tileset */
	for (i = 0; i < num_term; i++)
	{
		term_data *t = &data[i];
		
		t->tile.w = td->tile.w;
		t->tile.h = td->tile.h;
	}
	
	set_term_matrix(td);
	
	/* Force redraw */
	force_redraw();
} 
#ifdef USE_GTK_BUILDER
static void setup_graphics_menu(GtkBuilder *xml)
#else
static void setup_graphics_menu(GladeXML *xml)
#endif
{
	GtkWidget *menu;
		
	char s[12];
	int i;
	
	// FIXME: we should be using a numerical constant here
	for (i = 0; i < 5; i++)
	{
		bool checked = (i == arg_graphics);

		strnfmt(s, sizeof(s), "graphics_%d", i);
		menu = get_widget(xml, s);
		
		if (checked && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu)))
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu), TRUE); 
	}
}

/* Hooks for graphics menu items */
gboolean on_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	int g;
	const char *s;
	
	s = gtk_widget_get_name(widget);
	sscanf(s, "graphics_%d", &g);

	init_graf(g);
	return FALSE;
}

/*
 * Make text views "Angbandy" 
 */
static void white_on_black_textview(xtra_win_data *xd)
{
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
	xd->font_button_name = xtra_font_button_names[i];
	xd->drawing_area_name = xtra_drawing_names[i];
	xd->event = xtra_events[i];
}

static void init_xtra_windows(void)
{
	int i = 0;
	term_data *main_term = &data[0];
	GtkWidget *font_button;
	
	#ifdef USE_GTK_BUILDER
	GtkBuilder *xml;
	#else
	GladeXML *xml;
	#endif 
	
	char buf[1024];
	 
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, GTK_XML);
	xml = get_gtk_xml(buf, NULL);
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
			
		font_button = get_widget(xml, xd->font_button_name);
		xd->win = get_widget(xml, xd->win_name);
		xd->text_view = get_widget(xml, xd->text_view_name);
		xd->drawing_area = get_widget(xml, xd->drawing_area_name);
		
		gtk_font_button_set_font_name(GTK_FONT_BUTTON(font_button), xd->font.name);
		gtk_widget_realize(xd->win);
		gtk_window_set_title(GTK_WINDOW(xd->win), xd->name);
		
		if (xd->text_view != NULL)
		{
			white_on_black_textview(xd);
			gtk_widget_modify_font(GTK_WIDGET(xd->text_view), pango_font_description_from_string(xd->font.name));
		}
		if (xd->drawing_area != NULL)
		{
			cairo_rectangle_t r;
			gtk_widget_realize(xd->drawing_area); 
		
			gtk_widget_modify_bg(xd->win, GTK_STATE_NORMAL, &black);
			gtk_widget_modify_bg(xd->drawing_area, GTK_STATE_NORMAL, &black);
		
			xd->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 255 * main_term->font.w, 255 * main_term->font.h);
			init_cairo_rect(&r, 0, 0, 255 * main_term->font.w, 255 * main_term->font.h);
			cairo_clear(xd->surface, r, TERM_DARK);
		}
		
	}
	#ifdef USE_GTK_BUILDER
	gtk_builder_connect_signals(xml, NULL);
	#else
	glade_xml_signal_autoconnect(xml);
	#endif
	g_object_unref(G_OBJECT(xml));
}

static void init_gtk_windows(void)
{
	GtkWidget *options, *temp_widget;
	#ifdef USE_GTK_BUILDER
	GtkBuilder *gtk_xml;
	#else
	GladeXML *gtk_xml;
	#endif 

	char buf[1024], logo[1024], temp[20];
	int i = 0;
	
	/* Build the paths */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, GTK_XML); 
	gtk_xml = get_gtk_xml(buf, NULL);
	
	
	if (gtk_xml == NULL)
	{
		gtk_log_fmt(TERM_RED, "%s is Missing. Unrecoverable error. Aborting!", buf);
		quit(NULL);
		exit(0);
	}
			
	path_build(logo, sizeof(logo), ANGBAND_DIR_XTRA_ICON, "att-256.png");
	gtk_window_set_default_icon_from_file(logo, NULL);
	
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		
		if (!MAIN_WINDOW(td))
		{
			#ifdef USE_GTK_BUILDER
			GtkBuilder *xml;
			#else
			GladeXML *xml;
			#endif 
			
			/* Set up the Glade file */
			xml = get_gtk_xml(buf, "term_window");
			
			td->win = get_widget(xml, "term_window");
			td->drawing_area = get_widget(xml, "term_drawing");
			
			strnfmt(temp, sizeof(temp),  "term_menu_item_%i", i);
			td->menu_item = get_widget(gtk_xml, temp);
			
			#ifdef USE_GTK_BUILDER
			gtk_builder_connect_signals(xml, NULL);
			#else
			glade_xml_signal_autoconnect(xml);
			#endif
			
			g_object_unref (xml);
		}
		else
		{
			td->win = get_widget(gtk_xml, "main_window");
			td->drawing_area = get_widget(gtk_xml, "main_drawing");
			options = get_widget(gtk_xml, "option_window");
			toolbar = get_widget(gtk_xml, "main_toolbar");
			g_signal_connect(options, "delete_event", GTK_SIGNAL_FUNC(hide_options), NULL);
			
		}
		/* Set title and name */
		strnfmt(temp, sizeof(temp), "term_window_%d", i);
		gtk_widget_set_name(td->win, temp);
		
		strnfmt(temp, sizeof(temp), "term_drawing_area_%d", i);
		gtk_widget_set_name(td->drawing_area, temp);
		gtk_window_set_title(GTK_WINDOW(td->win), td->name);
	}

	for(i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		xd->menu = get_widget(gtk_xml, xd->item_name);
	}
		
	/* connect signal handlers that aren't passed data */
	#ifdef USE_GTK_BUILDER
	gtk_builder_connect_signals(gtk_xml, NULL);
	#else
	glade_xml_signal_autoconnect(gtk_xml);
	#endif
	
	setup_graphics_menu(gtk_xml);
	big_tile_item = get_widget(gtk_xml, "big_tile_item");
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		gtk_widget_realize(td->win);
		gtk_widget_realize(td->drawing_area); 
		
		gtk_widget_modify_bg(td->win, GTK_STATE_NORMAL, &black);
		gtk_widget_modify_bg(td->drawing_area, GTK_STATE_NORMAL, &black);

		strnfmt(temp, sizeof(temp), "term_font_%d", i);
		temp_widget = get_widget(gtk_xml, temp);
		gtk_widget_realize(temp_widget);
		gtk_font_button_set_font_name(GTK_FONT_BUTTON(temp_widget), td->font.name);
		
		strnfmt(temp, sizeof(temp), "row_%d", i);
		temp_widget = get_widget(gtk_xml, temp);
		gtk_widget_realize(temp_widget);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(temp_widget), td->rows);
		
		strnfmt(temp, sizeof(temp), "col_%d", i);
		temp_widget = get_widget(gtk_xml, temp);
		gtk_widget_realize(temp_widget);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(temp_widget), td->cols);
		
		td->initialized = TRUE;
		set_window_size(td);
	}
	
	g_object_unref(G_OBJECT(gtk_xml));
}

static void show_windows(void)
{
	int i;
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		bool active;
		
		/* Activate the window screen */
		Term_activate(&data[i].t);
			
		if (td->visible)
		{
			gtk_widget_show(td->win);
			term_data_redraw(td);
		}
		
		if (i !=0)
		{
			active = get_term_menu(td->number);
			
			if ( active != td->visible)
				set_term_menu(td->number, td->visible);
		}
		
		Term_clear_gtk(); 
	}
	
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];
		
		set_xtra_window_size(xd);
		if ((xd->visible) && (xd->menu != NULL))
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(xd->menu),TRUE);
	}
	
	if ((tile_width == 2) && (big_tile_item != NULL))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(big_tile_item), TRUE);
}


static void xtra_data_destroy(xtra_win_data *xd)
{
	if (GTK_IS_WIDGET(xd->text_view))
		gtk_widget_destroy(xd->text_view);
	if (GTK_IS_WIDGET(xd->drawing_area))
		gtk_widget_destroy(xd->drawing_area);
	if (GTK_IS_WIDGET(xd->win))
		gtk_widget_destroy(xd->win);
	if (cairo_surface_get_reference_count(xd->surface) > 0)
		cairo_surface_destroy(xd->surface);
}

static void term_data_destroy(term_data *td)
{	
	if (GTK_IS_WIDGET(td->drawing_area))
		gtk_widget_destroy(td->drawing_area);
	if (GTK_IS_WIDGET(td->win))
		gtk_widget_destroy(td->win);
	if (cairo_surface_get_reference_count(td->surface) > 0)
		cairo_surface_destroy(td->surface);
}

static void release_memory()
{
	int i;
	
	if (cairo_pattern_get_reference_count(tile_pattern) > 0)
		cairo_pattern_destroy(tile_pattern);
	if (cairo_surface_get_reference_count(graphical_tiles) > 0)
		cairo_surface_destroy(graphical_tiles);
	for (i = 0; i < MAX_XTRA_WIN_DATA; i++)
	{
		xtra_win_data *xd = &xdata[i];

		xtra_data_destroy(xd);
	}
	
	for (i = num_term; i > 0; i--)
	{
		term_data *td = &data[i];

		term_data_destroy(td);
	}
}

static errr term_data_init(term_data *td, int i)
{
	term *t = &td->t;

	conv = iconv_open("UTF-8", "ISO-8859-1");
	if (conv == (iconv_t)(-1)) {
		printf("iconv_open() failed: %d\n", errno);
		conv = NULL;
	}

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
	if (conv != NULL)
		t->xchar_hook = Term_xchar_gtk;
	
	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);
	
	/* Success */
	return (0);
}

static errr get_init_cmd(bool wait)
{
	Term_fresh();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	CheckEvent(wait);

	return 0;
}

static void handle_leave_init(game_event_type type, game_event_data *data, void *user) 
{
	/* Disable New and Open menu items - to do */
	game_in_progress = TRUE;
}

/*
 * Set up color tags for all the angband colors.
 */
static void init_color_tags(xtra_win_data *xd)
{
	int i;
	char colorname[10] = "color-00";
	char str[10] = "#FFFFFF";

	GtkTextTagTable *tags;

	tags = gtk_text_buffer_get_tag_table(xd->buf);
	
	for (i = 0; i <= MAX_COLORS; i++)
	{
		strnfmt(colorname, sizeof(colorname),  "color-%d", i);
		strnfmt(str, sizeof(str), "#%02x%02x%02x", angband_color_table[i][1], angband_color_table[i][2], angband_color_table[i][3]);

		if (gtk_text_tag_table_lookup(tags, colorname) != NULL)
			gtk_text_tag_table_remove(tags, gtk_text_tag_table_lookup(tags, colorname));
		
		if (gtk_text_tag_table_lookup(tags, colorname) == NULL)
			gtk_text_buffer_create_tag(xd->buf, colorname, "foreground", str, NULL);
	}
}

static void text_view_put(xtra_win_data *xd, const char *str, byte color)
{
	char colorname[10] = "color-00";
	GtkTextIter start, end;
	
	/* Get the color tag ready, and tack a line feed on the line we're printing */
	strnfmt(colorname, sizeof(colorname),  "color-%d", color);
	
	/* Print it at the end of the window */
	gtk_text_buffer_get_bounds(xd->buf, &start, &end);
	gtk_text_buffer_insert_with_tags_by_name(xd->buf, &end, str, strlen(str), colorname,  NULL);
}

static void text_view_print(xtra_win_data *xd, const char *str, byte color)
{
	text_view_put(xd, str, color);
	text_view_put(xd, "\n", color);
}

void gtk_log_fmt(byte c, const char *fmt, ...)
{
	xtra_win_data *xd = &xdata[4];
	
	char str[80];
	va_list vp;
	
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
	
	va_start(vp, fmt);
	(void)vstrnfmt(str, sizeof(str), fmt, vp);
	va_end(vp);

	#ifdef VERBOSE_DEBUG
	if (GTK_IS_TEXT_VIEW(xd->text_view))
		plog(str);
	#endif
	
	if (GTK_IS_TEXT_VIEW(xd->text_view))
		text_view_print(xd, str, c);
	else
		plog(str);
}

static void glog(const char *fmt, ...)
{
	xtra_win_data *xd = &xdata[4];
	
	char str[80];
	va_list vp;
	
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
	
	va_start(vp, fmt);
	(void)vstrnfmt(str, sizeof(str), fmt, vp);
	va_end(vp);

	#ifdef VERBOSE_DEBUG
	if (GTK_IS_TEXT_VIEW(xd->text_view))
		plog(str);
	#endif
	
	if (GTK_IS_TEXT_VIEW(xd->text_view))
		text_view_print(xd, str, TERM_WHITE);
	else
		plog(str);
}

static void reinitialize_text_buffer(xtra_win_data *xd)
{
	if (!GTK_IS_TEXT_BUFFER(xd->buf))
		xd->buf = gtk_text_buffer_new(NULL);
	else
		gtk_text_buffer_set_text(xd->buf, "", 0);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(xd->text_view), xd->buf);
	init_color_tags(xd);
}


/*
 * Update our own personal message window.
 */
static void handle_message(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[0];

	u16b num;
	int i;

	if (!xd) return;

	reinitialize_text_buffer(xd);
	
	num = messages_num();
	
	for (i = 0; i < num; i++)
		text_view_print(xd, message_str(i), message_color(i));
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
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Track */
		z = i + 1;
	}
	
	return z;
}

static void inv_slot(char *str, size_t len, int i, bool equip)
{
	object_type *o_ptr;
	char o_name[80], label[5];
	int name_size = 80;
	
	/* Examine the item */
	o_ptr = &p_ptr->inventory[i];

	/* Is this item "acceptable"? */
	if (item_tester_okay(o_ptr) || equip)
	{
		/* Obtain an item description */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);
			
		/* Obtain the length of the description */
		strnfmt(label, sizeof(label), "%c) ", index_to_label(i));
		
		if (equip)
			name_size = name_size - 19;
		
		/* Display the weight if needed */
		if (o_ptr->weight) 
		{
			int wgt = o_ptr->weight * o_ptr->number;			
			name_size = name_size - 4 - 9 - 9 - 5;
			
			if (equip)
				strnfmt(str, len, "%s%-*s %3d.%1d lb <-- %s", label, name_size, o_name, wgt / 10, wgt % 10, mention_use(i));
			else
				strnfmt(str, len, "%s%-*s %3d.%1d lb", label, name_size, o_name, wgt / 10, wgt % 10);
		}
		else
		{
			name_size = name_size - 4 - 9 - 5;

			if (equip)
				strnfmt(str, len, "%s%-*s <-- %s", label, name_size, o_name, mention_use(i));
			else
				strnfmt(str, len, "%s%-*s", label, name_size, o_name);
		}
	}
}

static void handle_inv(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[1];

	char str[80];
		
	register int i, z;
	byte attr;

	if (!xd) return;

	reinitialize_text_buffer(xd);
	
	z = last_inv_slot();
	
	/* Display the pack */
	for (i = 0; i < z; i++)
	{
		/* Examine the item */
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Get inventory color */
			attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];
			
			inv_slot(str, sizeof(str), i, FALSE);
			text_view_print(xd, str, attr);
		}
	}
}

static void handle_equip(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[2];
	
	register int i;
	byte attr;
	char str[80];
	
	if (!xd) return;
	
	reinitialize_text_buffer(xd);
	
	/* Display the pack */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];
	
		attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		inv_slot(str, sizeof(str), i, TRUE);

		text_view_print(xd, str, attr);
	}
}
/*
 * Display visible monsters in a window
 */
static void handle_mons_list(game_event_type type, game_event_data *data, void *user)
{
	xtra_win_data *xd = &xdata[3];
	int i;
	int line = 1;
	unsigned total_count = 0, disp_count = 0;

	byte attr;

	char *m_name;
	char buf[80];
	char str[80];

	monster_type *m_ptr;
	monster_race *r_ptr;

	u16b *race_count;

	if (!xd) return;

	reinitialize_text_buffer(xd);
	
	/* Allocate the array */
	race_count = C_ZNEW(z_info->r_max, u16b);

	/* Scan the monster list */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		m_ptr = cave_monster(cave, i);

		/* Only visible monsters */
		if (!m_ptr->ml) continue;

		/* Bump the count for this race, and the total count */
		race_count[m_ptr->r_idx]++;
		total_count++;
	}

	/* Note no visible monsters */
	if (!total_count)
	{
		/* Print note */
		strnfmt(str, sizeof(str), "You see no monsters.");
		text_view_print(xd, str, TERM_SLATE);
		
		/* Free up memory */
		FREE(race_count);

		/* Done */
		return;
	}

	strnfmt(str, sizeof(str), "You can see %d monster%s:", total_count, PLURAL(total_count));
	text_view_print(xd, str, 1);
	
	/* Go over in reverse order (so we show harder monsters first) */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_lore *l_ptr = &l_list[i];

		/* No monsters of this race are visible */
		if (!race_count[i]) continue;

		/* Note that these have been displayed */
		disp_count += race_count[i];

		/* Get monster race and name */
		r_ptr = &r_info[i];
		m_name = r_ptr->name;

		/* Display uniques in a special colour */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
			attr = TERM_VIOLET;
		/* If the player has never killed it (ever) AND it is out of depth */
		else if ((!l_ptr->tkills) && (r_ptr->level > p_ptr->depth))
			attr = TERM_RED;
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

static void draw_xtra_cr_text(xtra_win_data *xd, int x, int y, byte color, const char *str)
{
	measurements size;
	cairo_rectangle_t r;
	int n = strlen(str);

	/* Prevent gcc warnings */
	r.width = 0;
	r.height = 0;
	r.x = 0;
	r.y = 0;
	
	/* Set dimensions */
	size.w = xd->font.w;
	size.h = xd->font.h;
	draw_text(xd->surface, &xd->font, &size, x, y, n, color, str);
	
	invalidate_drawing_area(xd->drawing_area, r);
}

static void cr_aligned_text_print(xtra_win_data *xd, int x, int y, const char *str, byte color, const char *str2, byte color2, int length)
{
	int x2;
	
	x2 = x + length - strlen(str2) + 1;
	
	if (x2 > 0)
	{
		draw_xtra_cr_text(xd, x, y, color, str);
		draw_xtra_cr_text(xd, x2, y, color2, str2);
	}
	else
	{
		draw_xtra_cr_text(xd, x, y, color, str);
		draw_xtra_cr_text(xd, x + strlen(str) + 1, y, color2, str2);
	}
}

/*
 * Equippy chars
 */
static void cr_print_equippy(xtra_win_data *xd, int y)
{
	int i, j = 0;

	byte a;
	char c[6];

	object_type *o_ptr;

	/* No equippy chars if  we're in bigtile mode or creating a char */
	if ((tile_width == 2) || (arg_graphics != 0) || (!character_generated))
	{
		return;
	}

	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Object */
		o_ptr = &p_ptr->inventory[i];

		if (o_ptr->kind) {
			a = object_attr(o_ptr);
			strnfmt(c, sizeof(c), "%c",object_char(o_ptr)); 
		} else {
			c [0]= ' ';
			a = TERM_WHITE;
		}

		/* Dump */
		draw_xtra_cr_text(xd, j, y, a, c);
		j++;
	}
	strnfmt(c, sizeof(c), " "); 
	draw_xtra_cr_text(xd, j + 1, y, a, c);
}

static void xtra_drawn_progress_bar(xtra_win_data *xd, int x, int y, float curr, float max, byte color, int size)
{
	cairo_rectangle_t r;
	
	init_cairo_rect(&r, (xd->font.w * x)+ 1, (xd->font.h) * y + 1,  (xd->font.w * size) - 1, xd->font.h - 2);
	
	drawn_progress_bar(xd->surface, &xd->font, x, y, curr, max, color, size);
	
	invalidate_drawing_area(xd->drawing_area, r);
}

static void handle_sidebar(game_event_type type, game_event_data *data, void *user)
{
	char str[80]/*, str2[80]*/;
	cairo_rectangle_t r;
	
	xtra_win_data *xd = &xdata[5];
	long xp = (long)p_ptr->exp;
	monster_type *m_ptr = cave_monster(cave, p_ptr->health_who);
	int i = 0, sidebar_length = 12;

	/* Calculate XP for next level */
	if (p_ptr->lev != 50)
		xp = (long)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L) - p_ptr->exp;
	
	if ((xd->surface != NULL) && (xd->visible == TRUE))
	{
		/* Clear the window */
		init_cairo_rect(&r, 0, 0,  xd->font.w * 255, xd->font.h * 255);
		cairo_clear(xd->surface, r, TERM_DARK);
		
		/* Char Name */
		strnfmt(str, sizeof(str), "%s", op_ptr->full_name); 
		draw_xtra_cr_text(xd, 0, 0, TERM_L_BLUE, str);
		
		/* Char Race */
		strnfmt(str, sizeof(str), "%s", p_ptr->race->name);
		draw_xtra_cr_text(xd, 0, 1, TERM_L_BLUE, str);
		
		/* Char Title*/
		strnfmt(str, sizeof(str), "%s", p_ptr->class->title[(p_ptr->lev - 1) / 5], TERM_L_BLUE); 
		draw_xtra_cr_text(xd, 0, 2, TERM_L_BLUE, str);
		
		/* Char Class */
		strnfmt(str, sizeof(str), "%s", p_ptr->class->name); 
		draw_xtra_cr_text(xd, 0, 3, TERM_L_BLUE, str);

		/* Char Level */
		strnfmt(str, sizeof(str), "%i", p_ptr->lev);
		cr_aligned_text_print(xd, 0, 4, sidebar_text[4], TERM_WHITE, str, TERM_L_GREEN, sidebar_length);
		
		/* Char xp */
		if (p_ptr->lev != 50) 
			strnfmt(sidebar_text[5], sizeof(str), "Next: ");
		else 			 
			strnfmt(sidebar_text[5], sizeof(str), "XP: "); 
		strnfmt(str, sizeof(str), "%ld", xp); 
		cr_aligned_text_print(xd, 0, 5, sidebar_text[5], TERM_WHITE, str, TERM_L_GREEN, sidebar_length);
	
		/* Char Gold */
		strnfmt(str, sizeof(str), "%ld", p_ptr->au); 
		cr_aligned_text_print(xd, 0, 6, sidebar_text[6], TERM_WHITE, str, TERM_L_GREEN, sidebar_length);
		
		/* Equippy chars is 0,7 */
		cr_print_equippy(xd, 7);
		
		/* Char Stats */
		for (i = A_STR; i <= A_CHR; i++)
		{
			cnv_stat(p_ptr->state.stat_use[i], str, sizeof(str));
			cr_aligned_text_print(xd, 0, i+8, sidebar_text[i+8], TERM_WHITE, str, TERM_L_GREEN, sidebar_length);
		}
		
		/* 14 is a blank row */
		
		/* Char AC */
		strnfmt(str, sizeof(str), "%i", p_ptr->state.dis_ac + p_ptr->state.dis_to_a); 
		cr_aligned_text_print(xd, 0, 15, sidebar_text[15], TERM_WHITE, str, TERM_L_GREEN, sidebar_length);
	
		/* Char HP */
		strnfmt(str, sizeof(str), "%4d/%4d", p_ptr->chp, p_ptr->mhp); 
		cr_aligned_text_print(xd, 0, 16, sidebar_text[16], TERM_WHITE, str, player_hp_attr(p_ptr), sidebar_length);
		xtra_drawn_progress_bar(xd, 0, 17, p_ptr->chp, p_ptr->mhp, player_hp_attr(p_ptr), 13);
	
		/* Char MP */
		strnfmt(str, sizeof(str), "%4d/%4d", p_ptr->csp, p_ptr->msp); 
		cr_aligned_text_print(xd, 0, 18,sidebar_text[18], TERM_WHITE, str, player_sp_attr(p_ptr), sidebar_length);
		xtra_drawn_progress_bar(xd, 0, 19, p_ptr->csp, p_ptr->msp, player_sp_attr(p_ptr), 13);
	
		/* 20 is blank */
	
		xtra_drawn_progress_bar(xd, 0, 21, m_ptr->hp, m_ptr->maxhp, monster_health_attr(), 13);

		/* Print the level */
		strnfmt(str, sizeof(str), "%d' (L%d)", p_ptr->depth * 50, p_ptr->depth); 
		strnfmt(sidebar_text[22], sizeof(sidebar_text[22]), "%-*s", sidebar_length, str);
		draw_xtra_cr_text(xd, 0, 22, TERM_WHITE, sidebar_text[22]);
		
		invalidate_drawing_area(xd->drawing_area, r);
	}
}

static void handle_map(game_event_type type, game_event_data *data, void *user)
{
	if (use_graphics != arg_graphics)
	{
		use_graphics = arg_graphics;
		init_graf(arg_graphics);
	}
}


static void handle_game(game_event_type type, game_event_data *data, void *user)
{
	init_graf(arg_graphics);
}

/* These don't do anything right now, but I like having the hooks ready. */
static void handle_moved(game_event_type type, game_event_data *data, void *user) {}
static void handle_mons_target(game_event_type type, game_event_data *data, void *user) {}
static void handle_init_status(game_event_type type, game_event_data *data, void *user) {}
static void handle_birth(game_event_type type, game_event_data *data, void *user) {}
static void handle_store(game_event_type type, game_event_data *data, void *user){}
static void handle_death(game_event_type type, game_event_data *data, void *user){}
static void handle_end(game_event_type type, game_event_data *data, void *user){}
static void handle_splash(game_event_type type, game_event_data *data, void *user){}
static void handle_statusline(game_event_type type, game_event_data *data, void *user) {}


/* Command dispatcher for gtk builds */
static errr gtk_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_INIT) 
		return get_init_cmd(wait);
	else 
		return textui_get_cmd(context, wait);
}

void init_handlers()
{
	
	/* Activate hooks */
	quit_aux = hook_quit;

	/* Set command hook */
	cmd_get_hook = gtk_get_cmd;

	/* I plan to put everything on the sidebar together, as well as the statusline, so... */
	event_add_handler_set(my_player_events, N_ELEMENTS(my_player_events), handle_sidebar, NULL);
	event_add_handler_set(my_statusline_events, N_ELEMENTS(my_statusline_events), handle_statusline, NULL);
	
	event_add_handler(EVENT_MAP, handle_map, NULL);
	event_add_handler(EVENT_INVENTORY, handle_inv, NULL);
	event_add_handler(EVENT_EQUIPMENT, handle_equip, NULL);
	event_add_handler(EVENT_MONSTERLIST, handle_mons_list, NULL);
	event_add_handler(EVENT_MESSAGE, handle_message, NULL);
	event_add_handler(EVENT_ENTER_GAME, handle_game, NULL);
	
	event_add_handler(EVENT_PLAYERMOVED, handle_moved, NULL);
	event_add_handler(EVENT_MONSTERTARGET, handle_mons_target, NULL);
	event_add_handler(EVENT_INITSTATUS, handle_init_status, NULL);
	
	event_add_handler(EVENT_LEAVE_INIT, handle_leave_init, NULL);
	event_add_handler(EVENT_ENTER_BIRTH, handle_birth, NULL);
	event_add_handler(EVENT_ENTER_STORE, handle_store, NULL);
	event_add_handler(EVENT_ENTER_DEATH, handle_death, NULL);
	event_add_handler(EVENT_END, handle_end, NULL);
	event_add_handler(EVENT_ENTER_INIT, handle_splash, NULL);
}

errr init_gtk(int argc, char **argv)
{
	int i, ok;
	game_saved = FALSE;
	toolbar_size = 0;
	
	/* Initialize the environment */
	ok = gtk_init_check(&argc, &argv);
	if (ok == FALSE) return -1;
	
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

	/* Init dirs */
	create_needed_dirs();	
	
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
	
	/* Catch nasty signals, unless we want to see them */
	#ifndef GTK_DEBUG
	signals_init();
	#endif

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	
	Term_fresh();
	game_in_progress = FALSE;
	
	/* Set up the display handlers and things. */
	init_display();
	
	/* Let's play */
	play_game();

	/* Do all the things main() in main.c already does */
	cleanup_angband();
	quit(NULL);
	exit(0); /* just in case */

	/* Success */
	return (0);
}

#endif /* USE_GTK */
