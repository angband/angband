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
 * Key modifiers.
 */
#define KC_MOD_CONTROL  0x01
#define KC_MOD_SHIFT    0x02
#define KC_MOD_ALT      0x04
#define KC_MOD_META     0x08
#define KC_MOD_KEYPAD   0x10


/**
 * The game assumes that in certain cases, the effect of a modifer key will
 * be encoded in the keycode itself (e.g. 'A' is shift-'a').  In these cases
 * (specified below), a keypress' 'mods' value should not encode them also.
 *
 * If the character has come from the keypad:
 *   Include all mods
 * Else if the character is in the range 0x01-0x1F, and the keypress was
 * from a key that without modifiers would be in the range 0x40-0x5F:
 *   CONTROL is encoded in the keycode, and should not be in mods
 * Else if the character is in the range 0x21-0x2F, 0x3A-0x60 or 0x7B-0x7E:
 *   SHIFT is often used to produce these should not be encoded in mods
 *
 * (All ranges are inclusive.)
 *
 * You can use these macros for part of the above conditions.
 */
#define MODS_INCLUDE_CONTROL(v) \
	(((v) >= 0x01 && (v) <= 0x1F) ? FALSE : TRUE)

#define MODS_INCLUDE_SHIFT(v) \
	((((v) >= 0x21 && (v) <= 0x2F) || \
			((v) >= 0x3A && (v) <= 0x60) || \
			((v) >= 0x7B && (v) <= 0x7E)) ? FALSE : TRUE)


/**
 * If keycode you're trying to apply control to is between 0x40-0x5F
 * inclusive, then you should take 0x40 from the keycode and leave
 * KC_MOD_CONTROL unset.  Otherwise, leave the keycode alone and set
 * KC_MOD_CONTROL in mods.
 *
 * This macro returns TRUE in the former case and FALSE in the latter.
 */
#define ENCODE_KTRL(v) \
	(((v) >= 0x40 && (v) <= 0x5F) ? TRUE : FALSE)


/**
 * Given a character X, turn it into a control character.
 */
#define KTRL(X) \
	((X) & 0x1F)


/**
 * Given a control character X, turn it into its uppercase ASCII equivalent.
 */
#define UN_KTRL(X) \
	((X) + 64)


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
#define KC_F6         0x89
#define KC_F7         0x8A
#define KC_F8         0x8B
#define KC_F9         0x8C
#define KC_F10        0x8D
#define KC_F11        0x8E
#define KC_F12        0x8F
#define KC_F13        0x90
#define KC_F14        0x91
#define KC_F15        0x92

#define KC_HELP       0x93
#define KC_HOME       0x94
#define KC_PGUP       0x95
#define KC_END        0x96
#define KC_PGDOWN     0x97
#define KC_INSERT     0x98
#define KC_PAUSE      0x99
#define KC_BREAK      0x9a
#define KC_BEGIN      0x9b

/* we have up until 0x9F before we start edging into displayable Unicode */
/* then we could move into private use area 1, 0xE000 onwards */

/* Analogous to isdigit() etc in ctypes */
#define isarrow(c)  ((c >= ARROW_DOWN) && (c <= ARROW_UP))


/**
 * Type capable of holding any input key we might want to use.
 */
typedef u32b keycode_t;


/**
 * Struct holding all relevant info for keypresses.
 */
struct keypress {
	ui_event_type type;
	keycode_t code;
	byte mods;
};

/**
 * Struct holding all relevant info for mouse clicks.
 */
struct mouseclick {
	ui_event_type type;
	byte x;
	byte y;
	byte button;
};

/**
 * Union type to hold information about any given event.
 */
typedef union {
	ui_event_type type;
	struct mouseclick mouse;
	struct keypress key;
} ui_event;

/** Easy way to initialise a ui_event without seeing the gory bits. */
#define EVENT_EMPTY		{ 0 }


/*** Functions ***/

/** Given a string (and that string's length), return the corresponding keycode */
keycode_t keycode_find_code(const char *str, size_t len);

/** Given a keycode, return its description */
const char *keycode_find_desc(keycode_t kc);

/** Convert a string of keypresses into their textual representation */
void keypress_to_text(char *buf, size_t len, const struct keypress *src,
	bool expand_backslash);

/** Covert a textual representation of keypresses into actual keypresses */
void keypress_from_text(struct keypress *buf, size_t len, const char *str);

#endif /* INCLUDED_UI_EVENT_H */
