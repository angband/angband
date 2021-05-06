/**
 * \file ui-term.h
 *
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


/**
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
	int cx, cy;

	int **a;
	wchar_t **c;

	int *va;
	wchar_t *vc;

	int **ta;
	wchar_t **tc;

	int *vta;
	wchar_t *vtc;

	term_win *next;
};


/**
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
 *
 *      - Hook to test if an attr/char pair is a double-height tile
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
	int sidebar_mode;

	int attr_blank;
	wchar_t char_blank;

	bool complex_input;

	ui_event *key_queue;

	u16b key_head;
	u16b key_tail;
	u16b key_xtra;
	u16b key_size;

	int wid;
	int hgt;

	int y1;
	int y2;

	int *x1;
	int *x2;

	/* Offsets used by the map subwindows */
	int offset_x;
	int offset_y;

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

	errr (*text_hook)(int x, int y, int n, int a, const wchar_t *s);

	errr (*pict_hook)(int x, int y, int n, const int *ap, const wchar_t *cp, const int *tap, const wchar_t *tcp);

	void (*view_map_hook)(term *t);

        int (*dblh_hook)(int a, wchar_t c);

};


/**
 * ------------------------------------------------------------------------
 * Available Constants
 * ------------------------------------------------------------------------ */


/**
 * Maximum number of Angband windows
 */
#define ANGBAND_TERM_MAX 8

#define SIDEBAR_MODE (angband_term[0]->sidebar_mode)

#define SIDEBAR_LEFT 0
#define SIDEBAR_TOP  1
#define SIDEBAR_NONE 2
#define SIDEBAR_MAX  (SIDEBAR_NONE+1)

extern int row_map[SIDEBAR_MAX];
extern int col_map[SIDEBAR_MAX];

#define ROW_MAP	(row_map[Term->sidebar_mode])
#define COL_MAP	(col_map[Term->sidebar_mode])

/**
 * Number of text rows in each map screen, regardless of tile size
 */
#define SCREEN_ROWS	(Term->hgt - ROW_MAP - 1) 

/**
 * Number of grids in each screen (vertically)
 */
#define SCREEN_HGT    ((int) (SCREEN_ROWS / tile_height))

/**
 * Number of grids in each screen (horizontally)
 */
#define SCREEN_WID	((int)((Term->wid - COL_MAP - 1) / tile_width))

/**
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

/**
 * Bit flags for the "window_flag" variable.
 */
#define PW_INVEN            0x00000001L /* Display inven/equip */
#define PW_EQUIP            0x00000002L /* Display equip/inven */
#define PW_PLAYER_0         0x00000004L /* Display player (basic) */
#define PW_PLAYER_1         0x00000008L /* Display player (extra) */
#define PW_PLAYER_2         0x00000010L /* Display player (compact) */
#define PW_MAP              0x00000020L /* Display dungeon map */
#define PW_MESSAGE          0x00000040L /* Display messages */
#define PW_OVERHEAD         0x00000080L /* Display overhead view */
#define PW_MONSTER          0x00000100L /* Display monster recall */
#define PW_OBJECT           0x00000200L /* Display object recall */
#define PW_MONLIST          0x00000400L /* Display monster list */
#define PW_STATUS           0x00000800L /* Display status */
#define PW_ITEMLIST         0x00001000L /* Display item list */
#define PW_PLAYER_3         0x00002000L /* Display player (topbar) */

#define PW_MAPS (PW_MAP | PW_OVERHEAD)

#define PW_MAX_FLAGS		16


/**
 * sketchy key logging pt. 1
 */
#define KEYLOG_SIZE 8
extern int log_i;
extern int log_size;
extern struct keypress keylog[KEYLOG_SIZE];


/**
 * ------------------------------------------------------------------------
 *  Available Variables
 * ------------------------------------------------------------------------ */

extern term *Term;
extern byte tile_width;
extern byte tile_height;
extern bool bigcurs;
extern bool smlcurs;
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][16];
extern u32b window_flag[ANGBAND_TERM_MAX];

/**
 * Hack -- The main "screen"
 */
#define term_screen	(angband_term[0])



/**
 * ------------------------------------------------------------------------
 *  Available Functions
 * ------------------------------------------------------------------------ */

extern errr Term_xtra(int n, int v);

extern void Term_queue_char(term *t, int x, int y, int a, wchar_t c, int ta, wchar_t tc);
extern void Term_big_queue_char(term *t, int x, int y, int clipy,
	int a, wchar_t c, int a1, wchar_t c1);
extern void Term_queue_chars(int x, int y, int n, int a, const wchar_t *s);

extern errr Term_fresh(void);
extern errr Term_set_cursor(bool v);
extern errr Term_gotoxy(int x, int y);
extern errr Term_draw(int x, int y, int a, wchar_t c);
extern errr Term_addch(int a, wchar_t c);
extern errr Term_addstr(int n, int a, const char *s);
extern errr Term_putch(int x, int y, int a, wchar_t c);
extern void Term_big_putch(int x, int y, int a, wchar_t c);
extern errr Term_putstr(int x, int y, int n, int a, const char *s);
extern errr Term_erase(int x, int y, int n);
extern errr Term_clear(void);
extern errr Term_redraw(void);
extern errr Term_redraw_section(int x1, int y1, int x2, int y2);
extern errr Term_mark(int x, int y);

extern errr Term_get_cursor(bool *v);
extern errr Term_get_size(int *w, int *h);
extern errr Term_locate(int *x, int *y);
extern errr Term_what(int x, int y, int *a, wchar_t *c);

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

extern int big_pad(int col, int row, byte a, wchar_t c);

#endif /* INCLUDED_Z_TERM_H */
