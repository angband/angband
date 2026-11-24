/**
 * \file ui-term.h
 * \brief Declarations for a generic, efficient, terminal window package
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


typedef struct term_win term_win;

/**
 * A term_win holds contents (by row and column) for a Term as well as
 * cursor information.
 *
 * Note that the attr/char pair at (x,y) is a[y][x]/c[y][x]
 * and that the yth row of attr/chars is a[y]/c[y]
 */

struct term_win
{
	bool cu;	/**< true if cursor is useless; false otherwise */
	bool cv;	/**< true if cursor is visible; false otherwise */
	int cx;		/**< cursor x coordinate */
	int cy;		/**< cursor y coordinate */
	int cnx;	/**< cursor width, in columns */
	int cny;	/**< cursor height, in columns */

	int **a;	/**< pointers into va; a[y][x] is the foreground attribute at (x, y) */
	wchar_t **c;	/**< pointers into vc; c[y][x] is the foreground character at (x, y) */

	int *va;	/**< va[y * terminal columns + x] is the foreground attribute at (x, y) */
	wchar_t *vc;	/**< vc[y * terminal columns + x] is the foreground attribute at (x, y) */

	int **ta;	/**< pointers into vta; ta[y][x] is the background (terrain) attribute at (x, y) */
	wchar_t **tc;	/**< pointers into vtc; tc[y][x] is the background (terrain) character at (x, y) */

	int *vta;	/**< vta[y * terminal columns + x] is the background (terrain) attribute at (x, y) */
	wchar_t *vtc;	/**< vtc[y * terminal columns + x] is the background (terrain) character at (x, h) */

	term_win *next;	/**< next saved contents in stack */
};



typedef struct term term;

/**
 * An actual "term" structure
 */
struct term
{
	void *user;	/**< pointer to data managed by the terminal's application */

	void *data;	/**< pointer to the data the front end uses to implement the terminal */

	bool user_flag;	/**< flag used by the terminal's application */

	bool data_flag;	/**< flag used by the front end implementing the terminal */

	bool active_flag;	/**< true if the terminal is active (initialized without a subsequent call to term_nuke()) */
	bool mapped_flag;	/**< true if the terminal is mapped */
	bool total_erase;	/**< if true, the full contents will be sent to
					the front end the next time this
					terminal is refreshed rather than
					only sending what is believed to have
					changed; reset to false in a refresh */
	bool fixed_shape;	/**< if true, this terminal is not allow to resize */
	bool icky_corner;	/**< if true, the bottom right corner gets
					special treatment by the front end;
					typically true for some dumb terminals
					and false for all others */
	bool soft_cursor;	/**< if true, the cursor is handled in software
					rather than being a hardwired aspect
					of a dumb terminal (or emulator for a
					dumb terminal) */
	bool always_pict;	/**< if true, the contents of the terminal are
					pushed to the front end by pict_hook */
	bool higher_pict;	/**< if true, always_pict is false, and a grid's
					attribute has the 0x80 bit set, the
					contents of the grid are pushed to the
					front end by pict_hook; otherwise, the
					grid's contents are pushed to the
					front end by text_hook or wipe_hook */
	bool always_text;	/**< sets how a grid whose attribute is zero
					is pushed to the front end when
					always_pict is false; if false,
					wipe_hook will be used and the grid's
					character is ignored; if true,
					text_hook is used */
	bool xtra_clear;	/**< if true, call Term_xtra(TERM_XTRA_CLEAR, 0)
					the next time the terminal is refreshed
					so the front end clears the terminal;
					reset to false in a refresh */
	bool never_bored;	/**< if true, ui-term.c never calls
					Term_xtra(TERM_XTRA_BORED, 0) */
	bool never_frosh;	/**< if true, ui-term.c does not call
					Term_xtra(TERM_XTRA_FROSH, y) after
					refreshing the yth row */
	int sidebar_mode;	/**< for the main terminal, stores how extra
					data is presented along with the map */
	bool complex_input;	/**< if true, the front end wants to distinguish
					between Enter/^m/^j, Tab/^i, etc. */

	ui_event *key_queue;	/**< circular queue of input events */

	uint16_t key_head;	/**< index, in key_queue, at which to store the
					next input event */
	uint16_t key_tail;	/**< index, in key_queue, of the oldest
					unprocessed event; if key_tail equals
					key_head, the queue is empty */
	uint16_t key_xtra;
	uint16_t key_size;	/**< the maximum number of events that can be
					stored in key_queue */

	int wid;		/**< number of columns */
	int hgt;		/**< number of rows */

	int y1;			/**< row index for first modified row; will
					be hgt if nothing has been modified */
	int y2;			/**< row index for last modified row; will
					be zero if nothing has been modified */

	int *x1;		/**< x1[y] is the column index for the first
					modified column in row y; x1[y] will
					be wid if nothing has been modified in
					the row */
	int *x2;		/**< x2[y] is the column index for the last
					modified column in row y; x2[y] will
					be zero if nothing has been modified
					in the row */

	int offset_x;		/**< if this terminal represents a map, this is
					the x coordinate in the cave
					corresponding to the upper left corner
					of the map */
	int offset_y;		/**< if this terminal represents a map, this is
					the y coordinate in the cave
					corresponding to the upper right corner
					of the map */

	term_win *old;		/**< the contents of the terminal at the last
					refresh */
	term_win *scr;		/**< the contents of the terminal to be
					displayed at the next refresh */

	term_win *tmp;		/**< temporary screen image */
	term_win *mem;		/**< head of stack of stored screen images */

	uint8_t saved;		/**< number of stored screen images */

	/**
	 * front end's function to complete initialization of the terminal;
	 * not used if NULL
	 */
	void (*init_hook)(term *t);

	/**
	 * front end's function to clean up what it allocated for a terminal;
	 * not used if NULL
	 */
	void (*nuke_hook)(term *t);

	/**
	 * front end's function to perform extra actions; not used if NULL but
	 * then it is not possible to request an input event from the front end
	 */
	errr (*xtra_hook)(int n, int v);

	/**
	 * front end's function to draw (when soft_cursor is true) or move
	 * (when soft_cursor is false) the cursor
	 */
	errr (*curs_hook)(int x, int y);

	/**
	 * front end's function to draw a big cursor; may be NULL if soft_cursor
	 * is false
	 */
	errr (*bigcurs_hook)(int x, int y);

	/**
	 * front end's function to render grids whose attribute is zero;
	 * may be NULL if always_pict is true or always_text is true
	 */
	errr (*wipe_hook)(int x, int y, int n);

	/**
	 * front end's function to render text; may be NULL if always_pict is
	 * true
	 */
	errr (*text_hook)(int x, int y, int n, int a, const wchar_t *s);

	/**
	 * front end's function to render tiles; may be NULL if always_pict
	 * and higher_pict are false
	 */
	errr (*pict_hook)(int x, int y, int n, const int *ap, const wchar_t *cp, const int *tap, const wchar_t *tcp);

	/**
	 * front end's function to render a map of the full level; may be NULL
	 * so that the default rendering in ui-map.c's display_map() is used
	 */
	void (*view_map_hook)(term *t);

	/**
	 * front end's function to test if an attribute/character pair
	 * corresponds to a double-height tile; may be NULL if always_pict
	 * and higher_pict are false or double-height tiles are not used
	 */
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

extern int row_top_map[SIDEBAR_MAX];
extern int row_bottom_map[SIDEBAR_MAX];
extern int col_map[SIDEBAR_MAX];

#define ROW_MAP	(row_top_map[Term->sidebar_mode])
#define ROW_BOTTOM_MAP (row_bottom_map[Term->sidebar_mode])
#define COL_MAP	(col_map[Term->sidebar_mode])

/**
 * Number of text rows in each map screen, regardless of tile size
 */
#define SCREEN_ROWS	(Term->hgt - ROW_MAP - ROW_BOTTOM_MAP)

/**
 * Number of grids in each screen (vertically)
 */
#define SCREEN_HGT    ((int) (SCREEN_ROWS / tile_height))

/**
 * Number of grids in each screen (horizontally)
 */
#define SCREEN_WID	((int)((Term->wid - COL_MAP - 1) / tile_width))

/**
 * Value for the first parameter of Term_xtra() to request processing some
 * pending input events; if the second parameter of Term_xtra() is non-zero,
 * wait for an event if none is available; otherwise, return immediately if
 * there are no events available
 */
#define TERM_XTRA_EVENT 1
/**
 * Value for the first parameter of Term_xtra() to request flushing all
 * pending input events; the second parameter of Term_xtra is unused
 */
#define TERM_XTRA_FLUSH 2
/**
 * Value for the first parameter of Term_xtra() to request clearing the
 * entire window; the second parameter of Term_xtra is unused
 */
#define TERM_XTRA_CLEAR 3
/**
 * Value for the first parameter of Term_xtra() to request changing the
 * "shape" of the cursor; currently two values are used for the second
 * parameter of Term_xtra():  zero to request an invisible cursor and one
 * to request a normal and visible cursor; implementation by the front end
 * is optional
 */
#define TERM_XTRA_SHAPE 4
/**
 * Value for the first parameter of Term_xtra() to signal that a row has
 * been refreshed; the second parameter of Term_xtra() is the index of the
 * row; implementation by the front end is optional, and the overhead of
 * Term_xtra(TERM_XTRA_FROSH, ...) calls can be avoided by setting never_frosh
 * to true on the terminal
 */
#define TERM_XTRA_FROSH 5
/**
 * Value for the first parameter of Term_xtra() to signal that a refresh has
 * completed; the second parameter of Term_xtra is unused; implementation by
 * the front end is optional
 */
#define TERM_XTRA_FRESH 6
/**
 * Value for the first parameter of Term_xtra() to request that the front
 * end make a noise (ring the terminal bell); the second parameter of Term_xtra
 * is unused; implementation by the front end is optional
 */
#define TERM_XTRA_NOISE 7
/**
 * Value for the first parameter of Term_xtra() to request that the front
 * end handle stuff when nothing else is being done; the second parameter of
 * Term_xtra is unused; implementation by the front end is optional, and the
 * overhead of Term_xtra(TERM_XTRA_BORED, ...) calls can be avoided by setting
 * never_bored to true on the terminal
 */
#define TERM_XTRA_BORED 9
/**
 * Value for the first parameter of Term_xtra() to request that the front
 * end respond to a global change of some sort (color table change, tile set,
 * tile size, ...); the second parameter of the Term_xtra is unused;
 * implementation by the front end is optional
 */
#define TERM_XTRA_REACT 10
/**
 * Value for the first parameter of Term_xtra() to request changing the
 * "hard" level; two values are used for the second parameter of Term_xtra():
 * zero to indicate that the application is being suspended and one to
 * indicate that the application is being resumed; implementation by the
 * front end is optional
 */
#define TERM_XTRA_ALIVE 11
/**
 * Value for the first parameter of Term_xtra() to request changing the
 * "soft" level; two values are used for the second parameter of Term_xtra():
 * zero to indicate that the current active terminal, Term, is being replaced
 * and one to indicate that the switch to a new active terminal, Term, is
 * complete; implementation by the front end is optional
 */
#define TERM_XTRA_LEVEL 12
/**
 * Value for the first parameter of Term_xtra() to request that the front end
 * wait for the number of milliseconds specified in Term_xtra()'s second
 * parameter; implementation by the front end is optional
 */
#define TERM_XTRA_DELAY 13

/** bit flag for window_flag; display inven/equip */
#define PW_INVEN            0x00000001L
/** bit flag for window_flag; display equip/inven */
#define PW_EQUIP            0x00000002L
/** bit flag for window_flag; display player (basic) */
#define PW_PLAYER_0         0x00000004L
/** bit flag for window_flag; display player (extra) */
#define PW_PLAYER_1         0x00000008L
/** bit flag for window_flag; display player (compact) */
#define PW_PLAYER_2         0x00000010L
/** bit flag for window_flag; display dungeon map */
#define PW_MAP              0x00000020L
/** bit flag for window_flag; display messages */
#define PW_MESSAGE          0x00000040L
/** bit flag for window_flag; display overhead view */
#define PW_OVERHEAD         0x00000080L
/** bit flag for window_flag; display monster recall */
#define PW_MONSTER          0x00000100L
/** bit flag for window_flag; display object recall */
#define PW_OBJECT           0x00000200L
/** bit flag for window_flag; display monster list */
#define PW_MONLIST          0x00000400L
/** bit flag for window_flag; display status */
#define PW_STATUS           0x00000800L
/** bit flag for window_flag; display item list */
#define PW_ITEMLIST         0x00001000L
/** bit flag for window_flag; display player (topbar) */
#define PW_PLAYER_3         0x00002000L
#ifdef ALLOW_BORG
/** bitflag for window_flag; display borg messages */
#define PW_BORG_1           0x00004000L
/** bitflag for window_flag; display borg status */
#define PW_BORG_2           0x00008000L
#endif

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
extern uint8_t tile_width;
extern uint8_t tile_height;
extern bool bigcurs;
extern bool smlcurs;
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][16];
extern uint32_t window_flag[ANGBAND_TERM_MAX];
/**
 * If true, the front end's response to Term_xtra(TERM_XTRA_CLEAR, 0) affects
 * all terminals.  If false (the default value), that response only affects
 * the active terminal.
 */
extern bool term_global_clear;
/**
 * Flags whether the terminals are in a "disconnecting" state.  Modifies the
 * behavior of Term_inkey().
 */
extern volatile sig_atomic_t terms_disconnecting;
/**
 * Flags whether there's been a request to suspend the terminals.  Used by
 * front ends that do not use the default response to SIGTSTP.  See main.h's
 * struct module for more details about how to implement such front ends.
 */
extern volatile sig_atomic_t terms_suspending;

/**
 * The main "screen"
 */
#define term_screen	(angband_term[0])



/**
 * ------------------------------------------------------------------------
 *  Available Functions
 * ------------------------------------------------------------------------ */

extern errr Term_clear_all(void);
extern errr Term_fresh_all(void);
extern errr Term_redraw_all(void);
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
extern errr Term_keypress(keycode_t k, uint8_t mods);
extern errr Term_key_push(keycode_t k, uint8_t mods);
extern errr Term_event_push(const ui_event *ke);
extern errr Term_inkey(ui_event *ch, bool wait, bool take);

extern errr Term_save(void);
extern errr Term_load(void);
extern errr Term_load_all(void);

extern errr Term_resize(int w, int h);

extern errr Term_activate(term *t);

extern errr term_nuke(term *t);
extern errr term_init(term *t, int w, int h, int k);

extern int big_pad(int col, int row, uint8_t a, wchar_t c);

extern int Term_get_first_tile_row(term *t);

#endif /* INCLUDED_Z_TERM_H */
