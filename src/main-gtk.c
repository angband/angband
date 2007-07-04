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

#include "main.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>

#define MAX_TERM_DATA 1
/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data
{
	term t;

	GtkWidget *window;
	GtkWidget *drawing_area;
	
	cairo_t *cairo_draw;
	PangoFontDescription *font;
	PangoLayout *layout;
	
	GdkPixmap *pixmap;
	GdkGC *gc, *back_color;

	int font_wid;
	int font_hgt;
	
	int tile_wid;
	int tile_hgt;
	GdkPixmap *tiles;
	GdkPixmap *mask;
	int rows;
	int cols;

	cptr name;
};


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];

/*
 * game in progress
 */
static bool game_in_progress = FALSE;

/*
 * Number of active terms
 */
static int num_term = 1;

/* Our glade file */
GladeXML *xml;


/*
 * Path to the Gtk settings file
 */
char settings[1024];

/*
 * Set foreground color
 */
static void set_foreground_color(term_data *td, byte a)
{
	static unsigned int failed = 0;
	GdkColor color;
	/*int red, green, blue;*/

	color.red     = angband_color_table[a][1] * 256;
	color.green = angband_color_table[a][2] * 256;
	color.blue    = angband_color_table[a][3] * 256;
	
	g_assert(td->cairo_draw != 0);

	if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, FALSE, TRUE)) 
	{
		gdk_gc_set_foreground(td->gc, &color);
		cairo_set_source_rgb (td->cairo_draw, color.red, color.green, color.blue);
	}
	else if (!failed++)
		g_print("Couldn't allocate color.\n");
}

/* 
 * Set a GdkRectangle 
 */

static void init_gdk_rect(GdkRectangle *r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->width = w;
	r->height = h;
}

/*
 * Draw a cairo rectangle from a GdkRectangle object.
 */
static void c_rect(cairo_t *cr, GdkRectangle r)
{
	cairo_rectangle (cr, r.x, r.y, r.width, r.height);
}

/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	GdkRectangle r;

	term_data *td = (term_data*)(Term->data);
	
	/* Set dimensions of the window */
	init_gdk_rect(&r, 0, 0, td->cols * td->font_wid, td->rows * td->font_hgt);
	
	g_assert(td->drawing_area->window != 0);
	
	/* Clear the area */
	cairo_save(td->cairo_draw);
	
	c_rect(td->cairo_draw, r);
	set_foreground_color(td, TERM_DARK);
	cairo_fill(td->cairo_draw);
	cairo_close_path(td->cairo_draw);

	cairo_restore(td->cairo_draw);
	
	gdk_window_invalidate_rect(td->drawing_area->window, &r, TRUE);
	gdk_window_process_updates(td->drawing_area->window, TRUE);

	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	GdkRectangle r;
	term_data *td = (term_data*)(Term->data);
	
	/* Set dimensions */
	init_gdk_rect(&r, x * td->font_wid, y * td->font_hgt,  td->font_wid * n, td->font_hgt);
	
	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);
	
	cairo_save(td->cairo_draw);
	
	c_rect(td->cairo_draw, r);
	set_foreground_color(td, TERM_DARK);
	cairo_fill(td->cairo_draw);
	cairo_close_path(td->cairo_draw);
	
	cairo_restore(td->cairo_draw);
	
	gdk_window_invalidate_rect(td->drawing_area->window, &r, TRUE);
	gdk_window_process_updates(td->drawing_area->window, TRUE);

	/* Success */
	return (0);
}

/*
 * Draw some textual characters.
 */
static errr Term_text_gtk(int x, int y, int n, byte a, cptr s)
{
	term_data *td = (term_data*)(Term->data);
	GdkRectangle r;
	
	/* Set dimensions */
	init_gdk_rect(&r, x * td->font_wid, y * td->font_hgt,  td->font_wid * n, td->font_hgt);
	
	g_assert(td->layout != NULL);
	
	/* Clear the line */
	Term_wipe_gtk(x, y, n);
	
	cairo_save(td->cairo_draw);
	
	/* Create a PangoLayout, set the font and text */
	set_foreground_color(td, a);
	pango_layout_set_text(td->layout, s, n);
	pango_layout_set_font_description(td->layout, td->font);
	
	/* Draw the text to the pixmap */
	cairo_move_to(td->cairo_draw, r.x, r.y);
	pango_cairo_show_layout(td->cairo_draw, td->layout);
	
	cairo_restore(td->cairo_draw);
	
	gdk_window_invalidate_rect(td->drawing_area->window, &r, TRUE);
	gdk_window_process_updates(td->drawing_area->window, TRUE);
			
	/* Success */
	return (0);
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


static errr Term_flush_gtk(void)
{
	gdk_flush();
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
		case TERM_XTRA_NOISE: return (0);

		/* Flush the output */
		case TERM_XTRA_FRESH: return (0);

		/* Process random events */
		case TERM_XTRA_BORED: return (CheckEvent(0));

		/* Process Events */
		case TERM_XTRA_EVENT: return (CheckEvent(v));

		/* Flush the events */
		case TERM_XTRA_FLUSH: return (Term_flush_gtk());

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: return (0);

		/* Clear the screen */
		case TERM_XTRA_CLEAR: return (Term_clear_gtk());

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
			if (v > 0) usleep(1000 * v);
			return (0);

		/* React to changes */
		case TERM_XTRA_REACT: return (0);
	}

	/* Unknown */
	return (1);
}


static errr Term_curs_gtk(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	GdkRectangle r;

	/* Set dimensions */
	init_gdk_rect(&r, x * td->font_wid, y * td->font_hgt,  td->font_wid, td->font_hgt);
	
	cairo_save(td->cairo_draw);
	
	c_rect(td->cairo_draw, r);
	set_foreground_color(td, TERM_YELLOW);
	cairo_stroke(td->cairo_draw);
	
	cairo_restore(td->cairo_draw);
	
	gdk_window_invalidate_rect(td->drawing_area->window, &r, TRUE);
	gdk_window_process_updates(td->drawing_area->window, TRUE);
	gdk_flush();
	
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

static void save_game_gtk(void)
{
	if (game_in_progress && character_generated)
	{
		if (!inkey_flag)
		{
			plog("You may not do that right now.");
			return;
		}

		/* Hack -- Forget messages */
		msg_flag = FALSE;
		
		/* Save the game */
#ifdef ZANGBAND
		do_cmd_save_game(FALSE);
#else /* ZANGBAND */
		do_cmd_save_game();
#endif /* ZANGBAND */
	}
}


static void hook_quit(cptr str)
{
	gtk_exit(0);
}


void quit_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	save_game_gtk();
	quit(NULL);
	gtk_exit(0);
}


void destroy_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	quit(NULL);
	gtk_exit(0);
}


void hide_event_handler(GtkWidget *window, gpointer user_data)
{
	gtk_widget_hide(window);
}


void new_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	if (game_in_progress)
	{
		plog("You can't start a new game while you're still playing!");
	}
	else
	{
		game_in_progress = TRUE;
		Term_flush();
		play_game(TRUE);
#ifdef HAS_CLEANUP
		cleanup_angband();
#endif /* HAS_CLEANUP */
		quit(NULL);
	}
}



/*** Callbacks: font selector */

static void load_font_by_name(term_data *td, cptr fontname)
{	
	PangoRectangle r;
	PangoLayout *temp;
	
	cairo_t *cr;
	cairo_surface_t *surface;
	
	GdkPixmap *temp_pix;
	
	/* Create a temp pango/cairo context to play in */
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 200);
	cr = cairo_create(surface);
	temp = pango_cairo_create_layout(cr);
	td->font =  pango_font_description_from_string(fontname);
	
	g_assert(surface != NULL);
	g_assert(cr != NULL);
	g_assert(td->font != NULL);
	
	plog(fontname);
	
	/* Draw an @, and measure it */
	pango_layout_set_font_description(temp, td->font);
	pango_layout_set_text(temp, "@", 1);
	pango_cairo_show_layout(cr, temp);
	pango_layout_get_pixel_extents(temp, NULL, &r);
	
	/* Set the char width and height */
	td->font_wid = r.width;
	td->font_hgt = r.height;
	plog_fmt("Width & Height: %i, %i", td->font_wid, td->font_hgt);
	
	/* Blow up our temp variables */
	g_object_unref(temp);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	
	if (td->pixmap != NULL) 
	{
	/* Resize the term window accordingly */
	gtk_widget_set_size_request(GTK_WIDGET(td->drawing_area), td->cols * td->font_wid + 1, td->rows * td->font_hgt + 1);
	/* Hack - the values I'm giving it are smaller then the size the window should be;
	    and as such, it sets it to the smallest size it can while showing everything,
	    which is actually what I want it at. */
	gtk_window_resize(GTK_WINDOW(td->window), td->cols * td->font_wid + 1, td->rows * td->font_hgt + 1);
	
		
	/* Move out old pixmap, etc... to different vars */
	temp_pix = td->pixmap;
	cr = td->cairo_draw;
	temp = td->layout;
	
	/* Make a new pixmap, cairo context, etc */
	td->pixmap = gdk_pixmap_new(td->drawing_area->window, td->cols * td->font_wid, td->rows * td->font_hgt, -1);
	g_object_set_data(G_OBJECT(td->drawing_area), "pixmap", td->pixmap);
	td->cairo_draw = gdk_cairo_create(td->pixmap);
	td->layout = pango_cairo_create_layout (td->cairo_draw); 
	
	/* Destroy the old ones */
	g_object_unref(temp);
	cairo_destroy(cr);
	g_object_unref(temp_pix);
		
	Term_flush();
	term_data_redraw(td);
	}
	plog("Cleaned up.");
}

void font_ok_callback(GtkWidget *widget, GtkWidget *font_selector)
{
	gchar *fontname;
	term_data *td = g_object_get_data(G_OBJECT(font_selector), "term_data");

	g_assert(td != NULL);

	fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(font_selector));
	
	load_font_by_name(td, fontname);
	g_assert(td->font != NULL);
}


void change_font_event_handler(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *font_selector;
	GtkFontSelectionDialog *dialog;
	term_data *td = (term_data*)(Term->data);
	char *fontname = pango_font_description_to_string(td->font);
	
	font_selector = glade_xml_get_widget(xml, "font-window");
	
	g_object_set_data(G_OBJECT(font_selector), "term_data", user_data);
	
	dialog = GTK_FONT_SELECTION_DIALOG(font_selector);
	
	gtk_font_selection_dialog_set_font_name(dialog, fontname);
	
	g_signal_connect(G_OBJECT(dialog->ok_button), "clicked", G_CALLBACK(font_ok_callback), 
				   (gpointer)font_selector);

	/* Ensure that the dialog box is destroyed when the user clicks a button. */
	g_signal_connect_swapped(G_OBJECT(dialog->ok_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_hide),
						   (gpointer)font_selector);

	g_signal_connect_swapped(G_OBJECT(dialog->cancel_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_hide),
						  (gpointer)font_selector);

	gtk_widget_show(GTK_WIDGET(font_selector));
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
	/* XXX Should disable the menu entry */
	if (game_in_progress && !save)
	{
		plog("You can't open a new game while you're still playing!");
		return FALSE;
	}
			
	if ((!game_in_progress || !character_generated) && save)
	{
		plog("You can't save a new game unless you're still playing!");
		return FALSE;
	}

	if (!inkey_flag && save)
	{
		plog("You may not do that right now.");
		return FALSE;
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
	plog(buf);

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
		/* Start playing the game */
		game_in_progress = TRUE;
		Term_flush();
		play_game(FALSE);
#ifdef HAS_CLEANUP
		cleanup_angband();
#endif /* HAS_CLEANUP */
		quit(NULL);
	}
	
	/* Done */
	return;
}
void save_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	bool accepted;
	
	accepted = save_dialog_box(TRUE);
	
	if (accepted)
	{
		Term_flush();
		save_game_gtk();
	}
	
	/* Done */
	return;
}
gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	save_game_gtk();
	return (FALSE);
}
gboolean toggle_menu(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return(FALSE);
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
		sprintf(msg, "%cS_%X%c", 31, event->keyval, 13);

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

gboolean expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	term_data *td = user_data;
	
	GdkPixmap *pixmap = g_object_get_data(G_OBJECT(widget), "pixmap");
	if (pixmap)
	{
		g_assert(widget->window != 0);
		
		gdk_draw_drawable(widget->window, td->gc, pixmap,
		                event->area.x, event->area.y,
		                event->area.x, event->area.y,
		                event->area.width, event->area.height);
	}

	return (TRUE);
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

	t->xtra_hook = Term_xtra_gtk;
	t->text_hook = Term_text_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->curs_hook = Term_curs_gtk;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/* Success */
	return (0);
}

static void init_gtk_window(term_data *td, int i)
{
	cptr font;
	char buf[1024];
	
	bool main_window = (i == 0) ? TRUE : FALSE;
	GtkWidget *menu_item;

	/* Default to the default monospace font in Gtk */
	font = "Monospace 12";
	load_font_by_name(td, font);
	
	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA, "angband.glade");
	
	plog_fmt("ANGBAND_XTRA path = %s",buf);
	
	/* Set up the Glade file */
	xml = glade_xml_new(buf, NULL, NULL);
	
	if (main_window)
	{
		td->window = glade_xml_get_widget(xml, "main-window");
		td->drawing_area = glade_xml_get_widget(xml, "drawingarea1");
		menu_item = glade_xml_get_widget(xml, "font_menu_item");
		g_signal_connect (menu_item, "activate", G_CALLBACK(change_font_event_handler), td);
		
	}
	else
	{
		td->window = glade_xml_get_widget(xml, "term-window");
		td->drawing_area = glade_xml_get_widget(xml, "drawingarea2");
		g_signal_connect(td->window, "hide_event", G_CALLBACK(hide_event_handler), td);
	}
	
	g_signal_connect(td->drawing_area, "expose_event", G_CALLBACK(expose_event_handler), td);
	
	g_assert(td->window != NULL);
	g_assert(td->drawing_area != NULL);
	
	/* connect signal handlers that aren't passed data */
	glade_xml_signal_autoconnect(xml);
	
	/* Set attributes */
	gtk_window_set_title(GTK_WINDOW(td->window), td->name);
	gtk_widget_set_size_request(GTK_WIDGET(td->drawing_area), td->cols * td->font_wid + 1, td->rows * td->font_hgt + 1);
	gtk_window_move( GTK_WINDOW(td->window), 100, 100);

	/* Create a pixmap as buffer for screen updates */
	td->pixmap = gdk_pixmap_new(td->drawing_area->window, td->cols * td->font_wid, td->rows * td->font_hgt, -1);
	g_object_set_data(G_OBJECT(td->drawing_area), "pixmap", td->pixmap);
	td->gc = gdk_gc_new(td->drawing_area->window);
	td->cairo_draw = gdk_cairo_create(td->pixmap);
	td->layout = pango_cairo_create_layout (td->cairo_draw); 
	
	g_assert(td->pixmap != NULL);
	g_assert(td->gc != NULL);
	g_assert(td->cairo_draw != NULL);
	g_assert(td->layout != NULL);
	
	/* Show the widgets */
	gtk_widget_show_all(td->window);
}


const char help_gtk[] =
	"GTK for X11, subopts -n<windows> and standard GTK options";


/*
 * Initialization function
 */
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

		plog_fmt("Ignoring option: %s", argv[i]);
	}
	
	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;

		/* Init the window */
		init_gtk_window(td, i);
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Activate hooks */
	quit_aux = hook_quit;

	/* Set the system suffix */
	ANGBAND_SYS = "gtk";

	/* Catch nasty signals */
	signals_init();

	/* Initialize */
	init_angband();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	Term_fresh();

	/* Processing loop */
	gtk_main();

	/* Stop now */
	exit(0);

	/* Success */
	return (0);
}

#endif /* USE_GTK */
