/*
 * Copyright (c) 2007 Pete Mack and others
 * This code released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */



#ifndef INCLUDED_UI_EVENT_H
#define INCLUDED_UI_EVENT_H

/* The various UI events that can occur */
typedef enum 
{
	EVT_NONE	= 0x0,
	EVT_ESCAPE	= 0x0001,	/* Synonym for KBRD + key = ESCAPE */
	EVT_KBRD	= 0x0002,	/* keypress */
	EVT_MOUSE	= 0x0004,	/* mousepress */
	EVT_BACK	= 0x0008,	/* Up one level in heirarchical menus. */
	EVT_MOVE	= 0x0010,	/* menu movement */
	EVT_SELECT	= 0x0020,	/* Menu selection */
	EVT_BUTTON	= 0x0040,	/* button press */
	EVT_CMD		= 0x0080,	/* Command key execute */
	EVT_OK		= 0x0100,	/* Callback successful */
					/* For example, a command key action. */
	EVT_REFRESH	= 0x0200,	/* Display refresh */
	EVT_RESIZE	= 0x0400,	/* Display resize */

	EVT_AGAIN	= 0x4000000,	/* Retry notification */
	EVT_STOP	= 0x8000000	/* Loop stopped (never handled) */
} ui_event_type;

typedef struct ui_event_data ui_event_data;

struct ui_event_data
{
	ui_event_type type;
	byte mousex, mousey;
	char key; 
	short index;
};

#define EVENT_EMPTY		{ EVT_NONE, 0, 0, 0, 0 }


typedef struct event_target event_target;
typedef struct event_listener event_listener;
typedef struct event_set event_set;
typedef struct listener_list listener_list; /* Opaque */

/* An event handler member function */
typedef bool (*handler_f)(void *object, const ui_event_data *in);

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
ui_event_data run_event_loop(event_target *parent, bool forever, const ui_event_data *start);


#endif /* INCLUDED_UI_EVENT_H */
