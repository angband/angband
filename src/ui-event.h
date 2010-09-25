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
	EVT_RESIZE	= 0x0400	/* Display resize */
} ui_event_type;

typedef struct
{
	ui_event_type type;
	byte mousex, mousey;
	char key;
	short index;
} ui_event_data;

#define EVENT_EMPTY		{ EVT_NONE, 0, 0, 0, 0 }

#endif /* INCLUDED_UI_EVENT_H */
