/* File: z-term.h */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_TERM_H
#define INCLUDED_Z_TERM_H

#include "h-basic.h"
#include "ui-event.h"


/*
 * A term_win is a "window" for a Term
 *
 *	- Cursor Useless/Visible codes
 *	- Cursor Location (see "Useless")
 *
 *	- Array[h] -- Access to the attribute array
 *	- Array[h] -- Access to the character array
 *
 *	- Array[h*w] -- Attribute array
 *	- Array[h*w] -- Character array
 *
 *	- next screen saved
 *	- hook to be called on screen size change
 *
 * Note that the attr/char pair at (x,y) is a[y][x]/c[y][x]
 * and that the row of attr/chars at (0,y) is a[y]/c[y]
 */

typedef struct term_win term_win;

struct term_win
{
	bool cu, cv;
	byte cx, cy;

	byte **a;
	wchar_t **c;

	byte *va;
	wchar_t *vc;

	byte **ta;
	wchar_t **tc;

	byte *vta;
	wchar_t *vtc;

	term_win *next;
};


/*
 * An actual "term" structure
 *
 *	- Extra "user" info (used by application)
 *
 *	- Extra "data" info (used by implementation)
 *
 *
 *	- Flag "user_flag"
 *	  An extra "user" flag (used by application)
 *
 *
 *	- Flag "data_flag"
 *	  An extra "data" flag (used by implementation)
 *
 *
 *	- Flag "active_flag"
 *	  This "term" is "active"
 *
 *	- Flag "mapped_flag"
 *	  This "term" is "mapped"
 *
 *	- Flag "total_erase"
 *	  This "term" should be fully erased
 *
 *	- Flag "fixed_shape"
 *	  This "term" is not allowed to resize
 *
 *	- Flag "icky_corner"
 *	  This "term" has an "icky" corner grid
 *
 *	- Flag "soft_cursor"
 *	  This "term" uses a "software" cursor
 *
 *	- Flag "always_pict"
 *	  Use the "Term_pict()" routine for all text
 *
 *	- Flag "higher_pict"
 *	  Use the "Term_pict()" routine for special text
 *
 *	- Flag "always_text"
 *	  Use the "Term_text()" routine for invisible text
 *
 *	- Flag "unused_flag"
 *	  Reserved for future use
 *
 *	- Flag "never_bored"
 *	  Never call the "TERM_XTRA_BORED" action
 *
 *	- Flag "never_frosh"
 *	  Never call the "TERM_XTRA_FROSH" action
 *
 *
 *	- Value "attr_blank"
 *	  Use this "attr" value for "blank" grids
 *
 *	- Value "char_blank"
 *	  Use this "char" value for "blank" grids
 *
 *	- Flag "complex_input"
 *	  Distinguish between Enter/^m/^j, Tab/^i, etc.
 *
 *	- Ignore this pointer
 *
 *	- Keypress Queue -- various data
 *
 *	- Keypress Queue -- pending keys
 *
 *
 *	- Window Width (max 255)
 *	- Window Height (max 255)
 *
 *	- Minimum modified row
 *	- Maximum modified row
 *
 *	- Minimum modified column (per row)
 *	- Maximum modified column (per row)
 *
 *
 *	- Displayed screen image
 *	- Requested screen image
 *
 *	- Temporary screen image
 *	- Memorized screen image
 *
 *
 *	- Hook for init-ing the term
 *	- Hook for nuke-ing the term
 *
 *	- Hook for extra actions
 *
 *	- Hook for placing the cursor
 *
 *	- Hook for drawing some blank spaces
 *
 *	- Hook for drawing a string of chars using an attr
 *
 *	- Hook for drawing a sequence of special attr/char pairs
 */

typedef struct term term;

struct term
{
	void *user;

	void *data;

	bool user_flag;

	bool data_flag;

	bool active_flag;
	bool mapped_flag;
	bool total_erase;
	bool fixed_shape;
	bool icky_corner;
	bool soft_cursor;
	bool always_pict;
	bool higher_pict;
	bool always_text;
	bool unused_flag;
	bool never_bored;
	bool never_frosh;

	byte attr_blank;
	wchar_t char_blank;

	bool complex_input;

	ui_event *key_queue;

	u16b key_head;
	u16b key_tail;
	u16b key_xtra;
	u16b key_size;

	byte wid;
	byte hgt;

	byte y1;
	byte y2;

	byte *x1;
	byte *x2;

	/* Offsets used by the map subwindows */
	byte offset_x;
	byte offset_y;

	term_win *old;
	term_win *scr;

	term_win *tmp;
	term_win *mem;

	/* Number of times saved */
	byte saved;

	void (*init_hook)(term *t);
	void (*nuke_hook)(term *t);

	errr (*xtra_hook)(int n, int v);

	errr (*curs_hook)(int x, int y);

	errr (*bigcurs_hook)(int x, int y);

	errr (*wipe_hook)(int x, int y, int n);

	errr (*text_hook)(int x, int y, int n, byte a, const wchar_t *s);

	errr (*pict_hook)(int x, int y, int n, const byte *ap, const wchar_t *cp, const byte *tap, const wchar_t *tcp);

	size_t (*mbcs_hook)(wchar_t *dest, const char *src, int n);
};





/**** Available Constants ****/


/*
 * Definitions for the "actions" of "Term_xtra()"
 *
 * These values may be used as the first parameter of "Term_xtra()",
 * with the second parameter depending on the "action" itself.  Many
 * of the actions shown below are optional on at least one platform.
 *
 * The "TERM_XTRA_EVENT" action uses "v" to "wait" for an event
 * The "TERM_XTRA_SHAPE" action uses "v" to "show" the cursor
 * The "TERM_XTRA_FROSH" action uses "v" for the index of the row
 * The "TERM_XTRA_ALIVE" action uses "v" to "activate" (or "close")
 * The "TERM_XTRA_LEVEL" action uses "v" to "resume" (or "suspend")
 * The "TERM_XTRA_DELAY" action uses "v" as a "millisecond" value
 *
 * The other actions do not need a "v" code, so "zero" is used.
 */
#define TERM_XTRA_EVENT    1    /* Process some pending events */
#define TERM_XTRA_FLUSH 2    /* Flush all pending events */
#define TERM_XTRA_CLEAR 3    /* Clear the entire window */
#define TERM_XTRA_SHAPE 4    /* Set cursor shape (optional) */
#define TERM_XTRA_FROSH 5    /* Flush one row (optional) */
#define TERM_XTRA_FRESH 6    /* Flush all rows (optional) */
#define TERM_XTRA_NOISE 7    /* Make a noise (optional) */
#define TERM_XTRA_BORED 9    /* Handle stuff when bored (optional) */
#define TERM_XTRA_REACT 10    /* React to global changes (optional) */
#define TERM_XTRA_ALIVE 11    /* Change the "hard" level (optional) */
#define TERM_XTRA_LEVEL 12    /* Change the "soft" level (optional) */
#define TERM_XTRA_DELAY 13    /* Delay some milliseconds (optional) */


/*** Color constants ***/


/*
 * Angband "attributes" (with symbols, and base (R,G,B) codes)
 *
 * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
 * and should "gamma corrected" on most (non-Macintosh) machines.
 */
#define TERM_DARK     0  /* d */    /* 0 0 0 */
#define TERM_WHITE    1  /* w */    /* 4 4 4 */
#define TERM_SLATE    2  /* s */    /* 2 2 2 */
#define TERM_ORANGE   3  /* o */    /* 4 2 0 */
#define TERM_RED      4  /* r */    /* 3 0 0 */
#define TERM_GREEN    5  /* g */    /* 0 2 1 */
#define TERM_BLUE     6  /* b */    /* 0 0 4 */
#define TERM_UMBER    7  /* u */    /* 2 1 0 */
#define TERM_L_DARK   8  /* D */    /* 1 1 1 */
#define TERM_L_WHITE  9  /* W */    /* 3 3 3 */
#define TERM_L_PURPLE 10 /* P */    /* ? ? ? */
#define TERM_YELLOW   11 /* y */    /* 4 4 0 */
#define TERM_L_RED    12 /* R */    /* 4 0 0 */
#define TERM_L_GREEN  13 /* G */    /* 0 4 0 */
#define TERM_L_BLUE   14 /* B */    /* 0 4 4 */
#define TERM_L_UMBER  15 /* U */    /* 3 2 1 */

#define TERM_PURPLE      16    /* p */
#define TERM_VIOLET      17    /* v */
#define TERM_TEAL        18    /* t */
#define TERM_MUD         19    /* m */
#define TERM_L_YELLOW    20    /* Y */
#define TERM_MAGENTA     21    /* i */
#define TERM_L_TEAL      22    /* T */
#define TERM_L_VIOLET    23    /* V */
#define TERM_L_PINK      24    /* I */
#define TERM_MUSTARD     25    /* M */
#define TERM_BLUE_SLATE  26    /* z */
#define TERM_DEEP_L_BLUE 27    /* Z */

/* The following allow color 'translations' to support environments with a limited color depth
 * as well as translate colours to alternates for e.g. menu highlighting. */

#define ATTR_FULL        0    /* full color translation */
#define ATTR_MONO        1    /* mono color translation */
#define ATTR_VGA         2    /* 16 color translation */
#define ATTR_BLIND       3    /* "Blind" color translation */
#define ATTR_LIGHT       4    /* "Torchlit" color translation */
#define ATTR_DARK        5    /* "Dark" color translation */
#define ATTR_HIGH        6    /* "Highlight" color translation */
#define ATTR_METAL       7    /* "Metallic" color translation */
#define ATTR_MISC        8    /* "Miscellaneous" color translation - see misc_to_attr */

#define MAX_ATTR        9

/*
 * Maximum number of colours, and number of "basic" Angband colours
 */ 
#define MAX_COLORS        256
#define BASIC_COLORS    28



/* sketchy key logging pt. 1 */
#define KEYLOG_SIZE 8
extern int log_i;
extern int log_size;
struct keypress keylog[KEYLOG_SIZE];


/**** Available Variables ****/

extern term *Term;
extern byte tile_width;
extern byte tile_height;
extern bool bigcurs;
extern bool smlcurs;

/**** Available Functions ****/

extern errr Term_xtra(int n, int v);
extern size_t Term_mbstowcs(wchar_t *dest, const char *src, int n);

extern void Term_queue_char(term *t, int x, int y, byte a, wchar_t c, byte ta, wchar_t tc);
extern void Term_big_queue_char(term *t, int x, int y, byte a, wchar_t c, byte a1, wchar_t c1);
extern void Term_queue_chars(int x, int y, int n, byte a, const wchar_t *s);

extern errr Term_fresh(void);
extern errr Term_set_cursor(bool v);
extern errr Term_gotoxy(int x, int y);
extern errr Term_draw(int x, int y, byte a, wchar_t c);
extern errr Term_addch(byte a, wchar_t c);
extern errr Term_addstr(int n, byte a, const char *s);
extern errr Term_putch(int x, int y, byte a, wchar_t c);
extern void Term_big_putch(int x, int y, byte a, wchar_t c);
extern errr Term_putstr(int x, int y, int n, byte a, const char *s);
extern errr Term_erase(int x, int y, int n);
extern errr Term_clear(void);
extern errr Term_redraw(void);
extern errr Term_redraw_section(int x1, int y1, int x2, int y2);
extern errr Term_mark(int x, int y);

extern errr Term_get_cursor(bool *v);
extern errr Term_get_size(int *w, int *h);
extern errr Term_locate(int *x, int *y);
extern errr Term_what(int x, int y, byte *a, wchar_t *c);

extern errr Term_flush(void);
extern errr Term_mousepress(int x, int y, char button);
extern errr Term_keypress(keycode_t k, byte mods);
extern errr Term_key_push(int k);
extern errr Term_event_push(const ui_event *ke);
extern errr Term_inkey(ui_event *ch, bool wait, bool take);

extern errr Term_save(void);
extern errr Term_load(void);

extern errr Term_resize(int w, int h);

extern errr Term_activate(term *t);

extern errr term_nuke(term *t);
extern errr term_init(term *t, int w, int h, int k);

extern bool panel_contains(unsigned int y, unsigned int x);

#endif /* INCLUDED_Z_TERM_H */
