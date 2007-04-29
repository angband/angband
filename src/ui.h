
/*
 * Copyright (c) 2007 Pete Mack and others
 * This code released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */



#ifndef UI_H
#define UI_H

/* ============= Constants ============ */


/* Colors for interactive menus */
enum {
	CURS_UNKNOWN = 0,			/* Use gray; dark blue for cursor */
	CURS_KNOWN = 1				/* Use white; light blue for cursor */
};
static const byte curs_attrs[2][2] =
{
		{TERM_SLATE, TERM_BLUE},		/* Greyed row */
		{TERM_WHITE, TERM_L_BLUE}		/* Valid row */
};

/* Standard menu orderings */
extern const char default_choice[];		/* 1234567890A-Za-z */
extern const char lower_case[];			/* abc..z */
extern const char upper_case[];			/* ABC..Z */




/* ============= Defines a visual grouping ============ */
typedef struct
{
	byte tval;
	cptr name;
} grouper;

/* ================== GEOMETRY ====================== */

/* Defines a rectangle on the screen that is bound to a Panel or subpanel */
typedef struct region region;

struct region {
	int col;			/* x-coordinate of upper right corner */
	int row;			/* y-coord of upper right coordinate */
	int width;			/* width of display area. 1 - use system default. */
						/* non-positive - rel to right of screen */
	int page_rows;	/* non-positive value is relative to the bottom of the screen */
};

/* Region that defines the full screen */
static const region SCREEN_REGION = {0, 0, 0, 0};

/* Erase the contents of a region */
void region_erase(const region *loc);
/* Check whether a (mouse) event is inside a region */
bool region_inside(const region *loc, const event_type *key);



/* =================== EVENTS =================== */


typedef struct event_target event_target;
typedef struct event_listener event_listener;
typedef struct event_set event_set;
typedef struct listener_list listener_list; /* Opaque */

/* An event handler member function */
typedef bool (*handler_f)(void *object, const event_type *in);

/* Frees the resources for an owned event listener */
typedef void (*release_f)(void *object);

/* Set of event types to which a particular listener has subscribed */
struct event_set {
	int evt_flags;		/* OR'ed together set of events */
	/* anything else? */
};

/* Base class for event listener */
struct event_listener
{
	int object_id;			/* Identifier used for macros, etc */
	handler_f handler;		/* The handler function to call */
	release_f release;		/* Frees any owned resources */
	void *object;			/* Self-pointer */

	/* properly, this belongs in the listener_list */
	event_set events;		/* Set of events to which this listener has subscribed */
};


/* Event target -- the owner for a list of event listeners */
/* Examples include Windows, Panels (menus), application */
struct event_target {
	/* Allow a target to be a listener as well */
	event_listener self;
	bool is_modal;
	listener_list *observers;
};

void add_listener (event_target *parent, event_listener *child);
void add_target(event_target *parent, event_target *child);

void remove_listener (event_target *parent, event_listener *child);
event_type run_event_loop(event_target *parent, bool forever, const event_type *start);


/* ================= PANEL ============ */
typedef struct panel_type panel_type;

/*
 * An event target bound to a particular screen area.
 * A Panel is a (rectangular) (sub)region, possibly containing a set of event
 * listeners, and responsible for maintaining its own internal layout and
 * dispatching.  A Panel has ownership of a Region, is a Container for Event Listeners,
 * and an Event Target for mouse events).
 * Potential examples include: 
 *  - menu
 *  - window
 *  - map
 */
struct panel_type {
	event_target target;
	void (*refresh)();
	region boundary;
};

/* ================== MENUS ================= */

typedef struct event_action event_action;
typedef struct menu_item menu_item;
typedef struct menu_type menu_type;
typedef struct menu_skin menu_skin;
typedef struct menu_iter menu_iter;


/*
 * Performs an action on object with an optional environment label 
 * Member function of "menu_iter" VTAB
 */
typedef void (*action_f)(void *object, const char *name);

/* 
 * Displays a single row in a menu
 * Driver function for populating a single menu row.
 * Member function for "menu_iter" VTAB
 */
typedef void (*display_row_f) (menu_type *menu, int pos,
							bool cursor, int row, int col, int width);

/* 
 * Driver function for displaying a page of rows.
 * Member function of "menu_skin" VTAB
 */
typedef void (*display_list_f)(menu_type *menu, int cursor, int *top, region *);


/* Primitive menu item with bound action */
struct event_action
{
	int id;				/* Object id used to define macros &c */
	const char *name;	/* Name of the action */
	action_f action;	/* Action to perform, if any */
	void *data;			/* Local environment for the action, if required */
};


/* Decorated menu item with bound action */
struct menu_item
{
	event_action evt;	/* Base type */
	char sel;			/* Character used for selection, if special-purpose bindings */
						/* are desired. */
	int flags;			/* State of the menu item.  See enum menu_flags below */
};


/* TODO: menu registry */
/*
  Together, these classes define the constant properties of
  the various menu classes.
  A menu consists of:
   - menu_iter, which describes the basic
     class on which the menu is operating
      current classes are:
		MN_EVT- event_action array,
		MN_ACTION - menu_item array,
		MN_DBVIEW - general db iterator
 
    - a menu_skin, which describes the layout of the menu on the screen.
       current skins are: 
		MN_COLUMNS - all rows shown at once, in mult-column output.
		MN_SCROLL - only a limited part of the menu is shown, in a scrollable
					list
		MN_NATIVE - not implemented (OS menu)
	Menus also require data-dependent functions:

 */

typedef enum {

	/* Appearance & behavior */

	MN_REL_TAGS	= 0x0100, /* Tags are associated with the view, not the element */
	MN_NO_TAGS	= 0x0200, /* No tags -- movement key and mouse browsing only */
	MN_PVT_TAGS = 0x0400, /* Tags to be generated by the display function */

	MN_DBL_TAP	= 0x1000, /* double tap for selection; single tap is cursor movement */
	MN_NO_ACT	= 0x2000, /* Do not invoke the specified action; menu selection only */
	MN_PAGE		= 0x4000, /* Use full-page scrolling rather than small increment */
	MN_NO_CURSOR = 0x8000, /* No cursor movement */

	/* Reserved for rows in action_menu structure. */
	MN_DISABLED		= 0x0100000,	/* Neither action nor selection is permitted */
	MN_GRAYED		= 0x0200000,	/* Row is displayed with CURS_UNKNOWN colors */
	MN_SELECTED		= 0x0400000,	/* Row is currently selected */
	MN_SELECTABLE	= 0x0800000,	/* Row is permitted to be selected */
	MN_HIDDEN		= 0x1000000		/* Row is hidden, but may be selected via */
									/* key-binding. (Useful for lower case alternante) */
} menu_flags;

/* Identifier for the type of menu layout to use */
typedef enum
{
	/* Skins */
	MN_SCROLL	= 0x0000, /* Ordinary scrollable single-column list */
	MN_COLUMNS	= 0x0002, /* multicolumn view */
	MN_NATIVE	= 0x0003, /* Not implemented -- OS menu */
	MN_KEY_ONLY = 0x0004, /* No display */
	MN_USER		= 0x0005  /* Anonymous, user defined. */
} skin_id;

/* Class functions for menu layout */
struct menu_skin {
	skin_id id;					/* Identifier from the above list */
	/* Determines the cursor index given a (mouse) location */
	int (*get_cursor)(int row, int col, int n, int top, region *loc);
	/* Displays the current list of visible menu items */
	display_list_f display_list;
	/* Specifies the relative menu item given the state of the menu */
	char (*get_tag)(menu_type *menu, int pos);
	/* Superclass pointer. Not currently used */
	const menu_skin *super;
};


/* Identifiers for canned row iterator implementations */
typedef enum
{
	MN_ACT		= 0x1, /* selectable menu with per-row flags (see below) */
	MN_EVT		= 0x2, /* simple event action list */
	MN_STRING	= 0x3  /* display an array of strings for selection */
} menu_iter_id;


/* Class functions for menu row-level accessor functions */
struct menu_iter {
	menu_iter_id id;					/* Type identifier from above set */
	/* Optional selection tag function */
	char (*get_tag)(menu_type *menu, int oid);
	/* Optional validity checker.  All rows are assumed valid if not present. */
 	/* To support "hidden" items, it uses 3-level logic: 0 = no  1 = yes 2 = hide */
	int (*valid_row)(menu_type *menu, int oid);
	/* Displays a menu row at specified location */
	display_row_f display_row;
	/* Handler function called for selection or command key events */
	bool (*row_handler)(char cmd, void *db, int oid);
};


/* A menu defines either an action
 * or db row event
 */
struct menu_type
{
	/* menu inherits from panel */
	event_target target;
	void (*refresh)();
	region boundary;

	/* set of commands that may be performed on a menu item */
	const char *cmd_keys;


	/* Public variables */
	const char *title;
	const char *prompt;


	/* Keyboard shortcuts for menu selection */
	/* IMPORTANT: this cannot intersect with cmd_keys */
	const char *selections; 

	/* Flags specifying the behavior of this menu. See enum MENU_FLAGS */
	int flags;
	int filter_count;		/* number of rows in current view */
	const int *object_list;	/* optional filter (view) of menu objects */
	int count;				/* number of rows in underlying data set */
	const void *menu_data;	/* the data used to access rows. */

  	/* auxiliary browser help function */
	void (*browse_hook)(int oid, void *db, const region *loc);


	/* These are "protected" - not visible for canned menu classes, */
	/* The per-row functions  */
	const menu_iter *row_funcs;


	/* State variables for the menu */
	int cursor;				/* Currently selected row */
	int top;				/* Position in list for partial display */
	region active;			/* Subregion actually active for selection */

	/* helper functions for layout information. */
	const menu_skin *skin;  /* Defines menu appearance */
};


/* 
 * Select a row from a menu.
 * 
 * Arguments:
 *  object_list - optional ordered subset of menu OID.  If NULL, cursor is used for OID
 *  cursor - current (absolute) position in menu.  Modified on selection.
 *  loc - location to display the menu.
 * Return: A command key; cursor is set to the corresponding row.
 * (This is a stand-in for a menu event)
 * reserved commands are 0xff for selection and ESCAPE for escape.
 */
event_type menu_select(menu_type *menu, int *cursor, int no_handle);

/* TODO: This belongs in the VTAB */
bool menu_layout(menu_type *menu, const region *loc);

/* accessor & utility functions */
void menu_set_filter(menu_type *menu, const int object_list[], int n);
void menu_release_filter(menu_type *menu);
void menu_set_id(menu_type *menu, int id);

/* Set up structures for canned menu actions */
/* Bind the vtab for a menu */
bool menu_init(menu_type *menu, skin_id skin, menu_iter_id iter,
				const region *loc);

/* Initialize a menu given (anonymous) menu iter */
bool menu_init2(menu_type *menu, const menu_skin *skin,
				const menu_iter *iter, const region *loc);

void menu_refresh(menu_type *menu);
void menu_destroy(menu_type *menu);

/* Menu VTAB registry */
const menu_iter *find_menu_iter(menu_iter_id iter_id);
const menu_skin *find_menu_skin(skin_id skin_id);

void add_menu_skin(const menu_skin *skin, skin_id id);
void add_menu_iter(const menu_iter *skin, menu_iter_id id);


#endif /* UI_H */

