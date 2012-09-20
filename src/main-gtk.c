/* File: main-gtk.c */

/*
 * Copyright (c) 2000 Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "angband.h"


#ifdef USE_GTK

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define NO_PADDING 0

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
	GdkFont *font;
	GdkPixmap *pixmap;
	GdkGC *gc;

	int font_wid;
	int font_hgt;

	int rows;
	int cols;

	cptr name;
};


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[8];


/*
 * game in progress
 */
bool game_in_progress = FALSE;


/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	int width, height;

	term_data *td = (term_data*)(Term->data);

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

    /* Find proper dimensions for rectangle */
    gdk_window_get_size(td->drawing_area->window, &width, &height);

    /* Clear the area */
	gdk_draw_rectangle(td->pixmap, td->drawing_area->style->black_gc,
	                   1, 0, 0, width, height);

	/* Copy it to the window */
	gdk_draw_pixmap(td->drawing_area->window, td->gc, td->pixmap,
	                0, 0, 0, 0, width, height);

	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

	gdk_draw_rectangle(td->pixmap, td->drawing_area->style->black_gc,
	                   TRUE, x * td->font_wid, y * td->font_hgt, n * td->font_wid, td->font_hgt);

	/* Copy it to the window */
	gdk_draw_pixmap(td->drawing_area->window, td->gc, td->pixmap,
	                x * td->font_wid, y * td->font_hgt,
	                x * td->font_wid, y * td->font_hgt,
	                n * td->font_wid, td->font_hgt);

	/* Success */
	return (0);
}


/*
 * Draw some textual characters.
 */
static errr Term_text_gtk(int x, int y, int n, byte a, cptr s)
{
	int i;
	term_data *td = (term_data*)(Term->data);
	GdkColor color;

	color.red = angband_color_table[a][1] * 256;
	color.green = angband_color_table[a][2] * 256;
	color.blue = angband_color_table[a][3] * 256;

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

	if (!gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, TRUE, FALSE))
		g_print("Couldn't allocate color.");

	gdk_gc_set_foreground(td->gc, &color);

	/* Clear the line */
	Term_wipe_gtk(x, y, n);

	/* Draw the text to the pixmap */
	for (i = 0; i < n; i++)
	{
		gdk_draw_text(td->pixmap, td->font, td->gc, (x + i) * td->font_wid, td->font->ascent + y * td->font_hgt, s + i, 1);
	}

	/* Copy it to the window */
	gdk_draw_pixmap(td->drawing_area->window, td->gc, td->pixmap,
	                x * td->font_wid, y * td->font_hgt,
	                x * td->font_wid, y * td->font_hgt,
	                n * td->font_wid, td->font_hgt);

	/* Success */
	return (0);
}


static errr CheckEvent(bool wait)
{
	while (gtk_events_pending())
		gtk_main_iteration();

	return (0);
}


static errr Term_flush_gtk(void)
{
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
		case TERM_XTRA_DELAY: usleep(1000 * v); return (0);

		/* React to changes */
		case TERM_XTRA_REACT: return (0);
	}

	/* Unknown */
	return (1);
}


static errr Term_curs_gtk(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	GdkColor color;

	color.red = angband_color_table[TERM_YELLOW][1] * 256;
	color.green = angband_color_table[TERM_YELLOW][2] * 256;
	color.blue = angband_color_table[TERM_YELLOW][3] * 256;

	g_assert(td->pixmap != NULL);
	g_assert(td->drawing_area->window != 0);

	if (!gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, TRUE, FALSE))
		g_print("Couldn't allocate color.");

	gdk_gc_set_foreground(td->gc, &color);

	gdk_draw_rectangle(td->pixmap, td->gc, FALSE,
	                   x * td->font_wid, y * td->font_hgt, td->font_wid - 1, td->font_hgt - 1);

	/* Copy it to the window */
	gdk_draw_pixmap(td->drawing_area->window, td->gc, td->pixmap,
	                x * td->font_wid, y * td->font_hgt,
	                x * td->font_wid, y * td->font_hgt,
	                td->font_wid, td->font_hgt);

	/* Success */
	return (0);
}


static void save_game_gtk(void)
{
	if (game_in_progress && character_generated)
	{
		if (!inkey_flag || !can_save)
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


static void quit_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	save_game_gtk();

	quit(NULL);
}


static void destroy_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	quit(NULL);
}


static void hide_event_handler(GtkWidget *window, gpointer user_data)
{
	gtk_widget_hide(window);
}


static void new_event_handler(GtkButton *was_clicked, gpointer user_data)
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
		cleanup_angband();
		quit(NULL);
	}
}


static void load_font(term_data *td, cptr fontname)
{
	td->font = gdk_font_load(fontname);

	/* Calculate the size of the font XXX */
	td->font_wid = gdk_char_width(td->font, '@');
	td->font_hgt = td->font->ascent + td->font->descent;
}


static void font_ok_callback(GtkWidget *widget, GtkWidget *font_selector)
{
	gchar *fontname;
	term_data *td = gtk_object_get_data(GTK_OBJECT(font_selector), "term_data");

	g_assert(td != NULL);

	fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(font_selector));

	g_assert(fontname != NULL);

	load_font(td, fontname);
}


static void change_font_event_handler(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *font_selector;

	gchar *spacings[] = { "c", "m", NULL };

	font_selector = gtk_font_selection_dialog_new("Select font");

	gtk_object_set_data(GTK_OBJECT(font_selector), "term_data", user_data);

	/* Filter to show only fixed-width fonts */
	gtk_font_selection_dialog_set_filter(GTK_FONT_SELECTION_DIALOG(font_selector),
	                                    GTK_FONT_FILTER_BASE, GTK_FONT_ALL,
	                                    NULL, NULL, NULL, NULL, spacings, NULL);

	gtk_signal_connect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(font_selector)->ok_button),
	                   "clicked", font_ok_callback, (gpointer)font_selector);

	/* Ensure that the dialog box is destroyed when the user clicks a button. */
	gtk_signal_connect_object(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(font_selector)->ok_button),
	                          "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
	                          (gpointer)font_selector);

	gtk_signal_connect_object(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(font_selector)->cancel_button),
	                          "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
	                         (gpointer)font_selector);

	gtk_widget_show(GTK_WIDGET(font_selector));
}


static void file_ok_callback(GtkWidget *widget, GtkWidget *file_selector)
{
	strcpy(savefile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector)));

	gtk_widget_destroy(file_selector);

	game_in_progress = TRUE;
	Term_flush();
	play_game(FALSE);
	cleanup_angband();
	quit(NULL);
}


static void open_event_handler(GtkButton *was_clicked, gpointer user_data)
{
	GtkWidget *file_selector;
	char buf[1024];

	if (game_in_progress)
	{
		plog("You can't open a new game while you're still playing!");
	}
	else
	{
		/* Prepare the savefile path */
		path_build(buf, 1024, ANGBAND_DIR_SAVE, "*");

		file_selector = gtk_file_selection_new("Select a savefile");
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selector), buf);
		gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		                   "clicked", file_ok_callback, (gpointer)file_selector);

		/* Ensure that the dialog box is destroyed when the user clicks a button. */
		gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		                          "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
		                          (gpointer)file_selector);

		gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
		                          "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
		                          (gpointer)file_selector);

		gtk_window_set_modal(GTK_WINDOW(file_selector), TRUE);
		gtk_widget_show(GTK_WIDGET(file_selector));
	}
}


static gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	save_game_gtk();

	/* Don't prevent closure */
	return (FALSE);
}


static gboolean keypress_event_handler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	int i, mc, ms, mo, mx;

	char msg[128];

	/* Extract four "modifier flags" */
	mc = (event->state & GDK_CONTROL_MASK) ? TRUE : FALSE;
	ms = (event->state & GDK_SHIFT_MASK) ? TRUE : FALSE;
	mo = (event->state & GDK_MOD1_MASK) ? TRUE : FALSE;
	mx = (event->state & GDK_MOD2_MASK) ? TRUE : FALSE;

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
	}

	/* Build the macro trigger string */
	sprintf(msg, "%c%s%s%s%s_%s%c", 31,
	        mc ? "N" : "", ms ? "S" : "",
	        mo ? "O" : "", mx ? "M" : "",
	        gdk_keyval_name(event->keyval), 13);

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

	return (TRUE);
}


static gboolean expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	term_data *td = user_data;

	GdkPixmap *pixmap = gtk_object_get_data(GTK_OBJECT(widget), "pixmap");

	if (pixmap)
	{
		/* Restrict the area - Don't forget to reset it! XXX */
		/* gdk_gc_set_clip_rectangle(td->gc, &(event->area)); */

		g_assert(widget->window != 0);

		gdk_draw_pixmap(widget->window, td->gc, pixmap,
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

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
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


static void init_gtk_window(term_data *td, bool main)
{
	GtkWidget *menu_bar, *file_item, *file_menu, *box;
	GtkWidget *seperator_item, *file_exit_item, *file_new_item, *file_open_item;
	GtkWidget *options_item, *options_menu, *options_font_item;

	load_font(td, DEFAULT_X11_FONT_0);

	/* Create widgets */
	td->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	box = gtk_vbox_new(FALSE, 0);
	td->drawing_area = gtk_drawing_area_new();

	/* Create menu */
	if (main)
	{
		menu_bar = gtk_menu_bar_new();
		file_item = gtk_menu_item_new_with_label("File");
		file_menu = gtk_menu_new();
		file_new_item = gtk_menu_item_new_with_label("New");
		file_open_item = gtk_menu_item_new_with_label("Open");
		seperator_item = gtk_menu_item_new();
		file_exit_item = gtk_menu_item_new_with_label("Exit");
		options_item = gtk_menu_item_new_with_label("Options");
		options_menu = gtk_menu_new();
		options_font_item = gtk_menu_item_new_with_label("Font");
	 }

	/* Set attributes */
	gtk_window_set_title(GTK_WINDOW(td->window), td->name);
	gtk_drawing_area_size(GTK_DRAWING_AREA(td->drawing_area), td->cols * td->font_wid, td->rows * td->font_hgt);

	/* Register callbacks */
	if (main)
	{
		gtk_signal_connect(GTK_OBJECT(file_exit_item), "activate", quit_event_handler, NULL);
		gtk_signal_connect(GTK_OBJECT(file_new_item), "activate", new_event_handler, NULL);
		gtk_signal_connect(GTK_OBJECT(file_open_item), "activate", open_event_handler, NULL);
		gtk_signal_connect(GTK_OBJECT(options_font_item), "activate", change_font_event_handler, td);
	}

	gtk_signal_connect(GTK_OBJECT(td->window), "delete-event", GTK_SIGNAL_FUNC(delete_event_handler), NULL);
	gtk_signal_connect(GTK_OBJECT(td->window), "key-press-event", GTK_SIGNAL_FUNC(keypress_event_handler), NULL);
	gtk_signal_connect(GTK_OBJECT(td->drawing_area), "expose-event", GTK_SIGNAL_FUNC(expose_event_handler), td);

	if (main)
		gtk_signal_connect(GTK_OBJECT(td->window), "destroy", GTK_SIGNAL_FUNC(destroy_event_handler), NULL);
	else
		gtk_signal_connect(GTK_OBJECT(td->window), "destroy", GTK_SIGNAL_FUNC(hide_event_handler), td);

	/* Pack widgets */
	gtk_container_add(GTK_CONTAINER(td->window), box);

	if (main)
	{
		gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, NO_PADDING);
	}

	gtk_box_pack_start_defaults(GTK_BOX(box), td->drawing_area);

	/* Pack the menu bar */
	if (main)
	{
		gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), file_item);
		gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), options_item);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(options_item), options_menu);
		gtk_menu_append(GTK_MENU(file_menu), file_new_item);
		gtk_menu_append(GTK_MENU(file_menu), file_open_item);
		gtk_menu_append(GTK_MENU(file_menu), seperator_item);
		gtk_menu_append(GTK_MENU(file_menu), file_exit_item);
		gtk_menu_append(GTK_MENU(options_menu), options_font_item);
	}

	/* Show the widgets */
	gtk_widget_show_all(td->window);

	/* Create a pixmap as buffer for screenupdates */
	td->pixmap = gdk_pixmap_new(td->drawing_area->window, td->cols * td->font_wid, td->rows * td->font_hgt, -1);
	gtk_object_set_data(GTK_OBJECT(td->drawing_area), "pixmap", td->pixmap);
	td->gc = gdk_gc_new(td->drawing_area->window);
}


/*
 * Initialization function
 */
errr init_gtk(int argc, char **argv)
{
	int i;

	/* Initialize the environment */
	gtk_init(&argc, &argv);

	/* Initialize the windows */
	for (i = 0; i < 8; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;

		/* Init the window */
		init_gtk_window(td, (i == 0));
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Activate hooks */
	quit_aux = hook_quit;
	core_aux = hook_quit;

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
