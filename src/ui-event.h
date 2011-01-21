#ifndef INCLUDED_UI_EVENT_H
#define INCLUDED_UI_EVENT_H

/* The various UI events that can occur */
typedef enum
{
	EVT_NONE	= 0x0000,

	/* Basic events */
	EVT_KBRD	= 0x0001,	/* Keypress */
	EVT_MOUSE	= 0x0002,	/* Mousepress */
	EVT_RESIZE	= 0x0004,	/* Display resize */

	EVT_BUTTON	= 0x0008,	/* Button press */

	/* 'Abstract' events */
	EVT_ESCAPE	= 0x0010,	/* Get out of this menu */
	EVT_MOVE	= 0x0020,	/* Menu movement */
	EVT_SELECT	= 0x0040	/* Menu selection */
} ui_event_type;

typedef struct
{
	ui_event_type type;
	byte mousex, mousey;
	int mousebutton;
	char key;
} ui_event;

#define EVENT_EMPTY		{ EVT_NONE, 0, 0, 0, 0 }

#endif /* INCLUDED_UI_EVENT_H */
