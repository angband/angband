#ifndef INCLUDED_UI_EVENT_H
#define INCLUDED_UI_EVENT_H

/**
 * The various UI events that can occur.
 */
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


/**
 * Key modifiers
 */
#define KC_MOD_CONTROL  0x01
#define KC_MOD_SHIFT    0x02
#define KC_MOD_ALT      0x04
#define KC_MOD_META     0x08
#define KC_MOD_KEYPAD   0x10


/**
 * Keyset mappings for various keys.
 */
#define ESCAPE        0x1B
#define KC_RETURN     0x0A /* ASCII \n */
#define KC_ENTER      0x0D /* ASCII \r */
#define KC_TAB        0x09 /* ASCII \t */
#define KC_DELETE     0x7F
#define KC_BACKSPACE  0x08 /* ASCII \h */

#define ARROW_DOWN    0x80
#define ARROW_LEFT    0x81
#define ARROW_RIGHT   0x82
#define ARROW_UP      0x83

#define KC_F1         0x84
#define KC_F2         0x85
#define KC_F3         0x86
#define KC_F4         0x87
#define KC_F5         0x88
#define KC_F6         0x88
#define KC_F7         0x89
#define KC_F8         0x8A
#define KC_F9         0x8B
#define KC_F10        0x8C
#define KC_F11        0x8D
#define KC_F12        0x8E
#define KC_F13        0x8F
#define KC_F14        0x90
#define KC_F15        0x91

#define KC_HELP       0x92
#define KC_HOME       0x93
#define KC_PGUP       0x94
#define KC_END        0x95
#define KC_PGDOWN     0x96
#define KC_INSERT     0x97
#define KC_PAUSE      0x98
#define KC_BREAK      0x99

/* we have up until 0x9F before we start edging into displayable Unicode */
/* then we could move into private use area 1, 0xE000 onwards */

/* Analogous to isdigit() etc in ctypes */
#define isarrow(c)  ((c >= ARROW_DOWN) && (c <= ARROW_UP))

typedef u32b keycode_t;

typedef struct
{
	ui_event_type type;
	struct {
		byte x;
		byte y;
		byte button;
	} mouse;
	keycode_t key;
	byte keymods;
} ui_event;

#define EVENT_EMPTY		{ 0 }

#endif /* INCLUDED_UI_EVENT_H */
