/*
 * File: main-gtk.h
 * Purpose: Header file for the GTK port for Angband
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

#include "object/tvalsval.h"

#ifndef INCLUDED_MAIN_GTK_H 
#define INCLUDED_MAIN_GTK_H 

#define USE_PANGO
#define USE_CAIRO_UTILS

#include "main.h"
#include "cairo-utils.h"
#include "game-cmd.h" 
#include "game-event.h" 
#include "option.h"

#include <iconv.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>

#define MAX_TERM_DATA 8
#define MAX_XTRA_WIN_DATA 6
#define USE_GRAPHICS
#define MAIN_WINDOW(td) (td == &data[0])

#define GENERAL_PREFS 0
#define TERM_PREFS 1
#define XTRA_WIN_PREFS 2

/*#define USE_GUI_SELECTION*/

typedef struct term_data term_data;
typedef struct xtra_win_data xtra_win_data;

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

struct term_data 
{
	term t;
	const char *name;
	int number;
	
	bool visible, initialized;
	int rows;
	int cols;
	point location;
	measurements size; /* of the window */
	measurements tile, actual;
	
	cairo_surface_t *surface;
	cairo_matrix_t matrix;
	font_info font;
	
	GtkWidget *win;
	GtkWidget *drawing_area;
	GtkWidget *menu_item;
};

struct xtra_win_data
{
	int x;
	const char *name, *win_name, *text_view_name, *item_name, *drawing_area_name, *font_button_name;
	bool visible;
	font_info font;
	
	GtkWidget *win, *text_view, *menu, *drawing_area;
	GtkTextBuffer *buf;
	cairo_t *cr;
	cairo_surface_t *surface;
	measurements size;
	point location;
	game_event_type event;
};

static game_event_type xtra_events[]=
{
	EVENT_MESSAGE, 
	EVENT_INVENTORY, 
	EVENT_EQUIPMENT, 
	EVENT_MONSTERLIST, 
	0, 
	EVENT_PLAYERTITLE
};

GdkColor black = { 0, 0x0000, 0x0000, 0x0000 };
GdkColor white = { 0, 0xffff, 0xffff, 0xffff };
	
char xtra_names[MAX_XTRA_WIN_DATA][20] =
{ "Messages", "Inventory", "Equipment", "Monster List", "Debug", "Status" };

char xtra_win_names[MAX_XTRA_WIN_DATA][20] =
{ "message_window", "inv_window", "equip_window", "monst_list_window", "debug_window", "status_window" };

char xtra_text_names[MAX_XTRA_WIN_DATA][20] =
{ "message_text", "inv_text", "equip_text", "monst_list_text", "debug_text", "status_text" };

char xtra_drawing_names[MAX_XTRA_WIN_DATA][24] =
{ "message_drawing_area", "inv_drawing_area", "equip_drawing_area", "monst_list_drawing_area", "debug_drawing_area", "status_drawing_area" };

char xtra_menu_names[MAX_XTRA_WIN_DATA][20] =
{ "message_item", "inv_item", "equip_item", "monst_list_item", "debug_item", "status_item" };

char xtra_font_button_names[MAX_XTRA_WIN_DATA][20] =
{ "message_font", "inv_font", "equip_font", "monst_list_font", "debug_font", "status_font" };

char sidebar_text[24][20] =
{
	"","","","","Level:", "XP: ", "Gold:", "", "STR:", "INT:", "WIS:", "DEX:", "CON:", "CHR:", "", "AC:", "HP:", "", "SP:", "", "","", "", "" 
};
/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];

/*
 * A similar array for my extra, non-term windows.
 */
static xtra_win_data xdata[MAX_XTRA_WIN_DATA];

/* An array of graphic menu items */
/*static GtkWidget *graphics_item[4];*/
static GtkWidget *big_tile_item;
/* 
 * There are a few functions installed to be triggered by several 
 * of the basic player events.  For convenience, these have been grouped 
 * in this list that I swiped from xtra3.c. These really should be in one of the 
 * headers, shouldn't they?
 */
static game_event_type my_player_events[] =
{
	EVENT_RACE_CLASS,
	EVENT_PLAYERTITLE,
	EVENT_EXPERIENCE,
	EVENT_PLAYERLEVEL,
	EVENT_GOLD,
	EVENT_EQUIPMENT,  /* For equippy chars */
	EVENT_STATS,
	EVENT_HP,
	EVENT_MANA,
	EVENT_AC,

	EVENT_MONSTERHEALTH,

	EVENT_PLAYERSPEED, 
	EVENT_DUNGEONLEVEL,
};

static game_event_type my_statusline_events[] =
{
	EVENT_STUDYSTATUS,
	EVENT_STATUS,
	EVENT_DETECTIONSTATUS,
	EVENT_STATE,
};

/*  Various variables  */
static void save_prefs(void);
static bool ignore_prefs = FALSE;
static bool game_in_progress = FALSE;
static int num_term = 8;

/* Command line help */
const char help_gtk[] = "GTK for X11, subopts -n<windows>, -i to ignore prefs, and standard GTK options";

/*  Path to the Gtk settings file */
static char settings[1024];
bool game_saved;

static GtkWidget *toolbar;
static GtkWidget *toolbar_items[12];
static int toolbar_size;
static unsigned char toolbar_keypress[12];

/* Abstracted out for future changes */
static int max_win_width(term_data *td);
static int max_win_height(term_data *td);
/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int * const x, int * const y, const int ox, const int oy);

void set_term_matrix(term_data *td);
/* Cairo's rect type to Gdks */
static GdkRectangle cairo_rect_to_gdk(cairo_rectangle_t *r);

/* And vice-versa */
static cairo_rectangle_t gdk_rect_to_cairo(GdkRectangle *r);

/* Mark part of a window as invalid, so it gets redrawn */
static void invalidate_drawing_area(GtkWidget *widget, cairo_rectangle_t r);

/* Get the position of a term window when it changes */
gboolean configure_event_handler(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);

/* Get the position of an extra window when it changes */
gboolean xtra_configure_event_handler(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);

/* Set the window geometry */
static void set_window_defaults(term_data *td);

/* Set the term window size */
static void set_window_size(term_data *td);

/* Set the extra window size */
static void set_xtra_window_size(xtra_win_data *xd);

/* If a term window is shown, do a few checks */
gboolean show_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

/* Erase the whole term. */
static errr Term_clear_gtk(void);

/* Erase some characters. */
static errr Term_wipe_gtk(int x, int y, int n);

/* Draw some textual characters. */
static errr Term_text_gtk(int x, int y, int n, byte a, const char *s);

/* Draw pretty pictures instead. */
static errr Term_pict_gtk(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);

/* Do gtk things */
static errr Term_flush_gtk(void);

/* Refresh things */
static errr Term_fresh_gtk(void);

/*  Handle a "special request" */
static errr Term_xtra_gtk(int n, int v);

/* Draw the cursor */
static errr Term_curs_gtk(int x, int y);

/*
 * Hack -- redraw a term_data
 *
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td);

/* Check for events - Traditional */
static errr CheckEvent(bool wait);

/* Save the game */
static bool save_game_gtk(void);

/* Quit the game - from Angband */
static void hook_quit(const char *str);

/* Quit the game - from Gtk */
gboolean quit_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/* Kill the Kin... err... the game */
gboolean destroy_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/* Hide away, term windows */
gboolean hide_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/* Hide away, extra windows */
gboolean xtra_hide_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/* Huzzah! A new game. */
gboolean new_event_handler(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/* Given a term window and font, load the font */
static void load_font_by_name(term_data *td, const char *font_name);

/* Set the name of a font for a term window and load it. */
void set_term_font(GtkFontButton *widget, gpointer user_data);

/* Set the name of a font for an extra window and load it. */
void set_xtra_font(GtkFontButton *widget, gpointer user_data);

/* Filter function for the savefile list */
static gboolean file_open_filter(const GtkFileFilterInfo *filter_info, gpointer data);

/* Open/Save Dialog box */
static bool save_dialog_box(bool save);

/* Open a saved game */
void open_event_handler(GtkButton *was_clicked, gpointer user_data);

/* Save the game */
gboolean save_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/* Save the game before deleting? */
gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/* Register keypresses */
gboolean keypress_event_handler(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

/* Prefs - copied straight from main-x11.c and hacked. */
static void save_prefs(void);

/* Util functions for prefs */
static int check_env_i(char* name, int i, int dfault);
static int get_value(const char *buf);
static void load_term_prefs();

/* Load the prefs */
static void load_prefs();

/* Register a mouse click */
gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/* Redraw a term window */
gboolean expose_event_handler(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

/* Hide the options window */
gboolean hide_options(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/* Toggle the appropriate checkmark for a term menu */
gboolean toggle_term_window(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/* Toggle the appropriate checkmark for the exra menu */
gboolean toggle_xtra_window(GtkWidget *widget, GdkEvent *event, gpointer user_data);

/* Set up graphics */
static void init_graf(int g);

/* Initially choose arg_graphics */
static void setup_graphics_menu();

/* Hooks for graphics menu items */
gboolean on_graphics_activate(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

/*  Make text views "Angbandy"  */
static void white_on_black_textview(xtra_win_data *xd);

/* Init data for extra windows */
static void xtra_data_init(xtra_win_data *xd, int i);

/* Init the windows themselves */
static void init_xtra_windows(void);

/* Init the rest of the windows */
static void init_gtk_windows(void);

/* Show everything */
static void show_windows();

/* Init the term window data */
static errr term_data_init(term_data *td, int i);

 /* Stubs */
 static void handle_map(game_event_type type, game_event_data *data, void *user);
 static void handle_moved(game_event_type type, game_event_data *data, void *user);
 static void handle_mons_target(game_event_type type, game_event_data *data, void *user);
 
 /* Set up color tags for all the angband colors. */
static void init_color_tags(xtra_win_data *xd);

/* Put text in a textview without a \n */
static void text_view_put(xtra_win_data *xd, const char *str, byte color);

/* Put text in a textview with a \n */
static void text_view_print(xtra_win_data *xd, const char *str, byte color);

/* plog_fmt reborn */
void gtk_log_fmt(byte c, const char *fmt, ...);

/* Update our own personal message window. */
static void handle_message(game_event_type type, game_event_data *data, void *user);

/* Return the last inventory slot. */
static int last_inv_slot(void);

/* Return an inventory slot */
static void inv_slot(char *str, size_t len, int i, bool equip);

/* Print the inventory */
static void handle_inv(game_event_type type, game_event_data *data, void *user);

/* Print the equipment list */
static void handle_equip(game_event_type type, game_event_data *data, void *user);

/* Print the monster window */
static void handle_mons_list(game_event_type type, game_event_data *data, void *user);

/* Print the sidebar */
static void handle_sidebar(game_event_type type, game_event_data *data, void *user);
static void glog(const char *fmt, ...);

/* More Stubs */
static void handle_init_status(game_event_type type, game_event_data *data, void *user);
static void handle_birth(game_event_type type, game_event_data *data, void *user);
static void handle_game(game_event_type type, game_event_data *data, void *user);
static void handle_store(game_event_type type, game_event_data *data, void *user);
static void handle_death(game_event_type type, game_event_data *data, void *user);
static void handle_end(game_event_type type, game_event_data *data, void *user);
static void handle_splash(game_event_type type, game_event_data *data, void *user);
static void handle_statusline(game_event_type type, game_event_data *data, void *user);

/* init the handlers */
static void init_handlers();

/* Init gtk */
errr init_gtk(int argc, char **argv);

/* Nuke things */
static void xtra_data_destroy(xtra_win_data *xd);
static void term_data_destroy(term_data *td);
static void release_memory();

static void force_redraw();

/* From xtra3.h */
extern byte monster_health_attr();
#endif /* INCLUDED_MAIN_GTK_H */ 
