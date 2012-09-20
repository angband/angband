/* File: term.h */

#ifndef INCLUDED_TERM_H
#define INCLUDED_TERM_H

#include "h-include.h"



/*
 * A term_win is a "window" for a Term
 *
 *  - Window "flags" (unused)
 *
 *	- Window "offset" (unused)
 *
 *	- Window Size (max 256x256)
 *
 *	- Cursor Useless/Visible codes
 *	- Cursor Location (see "Useless")
 *
 *	- Min/Max modified rows (per screen)
 *
 *  - Min/Max modified col (per row)
 *
 *  - Min/Max column with "useful" info (per row)
 *
 *	- Attribute array (see "tw_a()")
 *	- Character array (see "tw_c()")
 *
 * Note that "y1<=y2" iff any changes have occured on the screen.
 * Note that "r1[y]<=r2[y]" iff any changes have occured in row "y".
 * Note that "blanks" written past "rm[y]" in row "y" are ignored.
 */

typedef struct _term_win term_win;

struct _term_win {

    huge flags;

    short x, y;

    byte w, h;

    bool cu, cv;
    byte cx, cy;

    byte y1, y2;

    byte *x1;
    byte *x2;

    byte *a;
    char *c;
};



/*
 * An actual "term" structure
 *
 *	- Have we been activated for the first time
 *	- Do our support routines use a "software cursor"?
 *	- Should we call the "Event Loop" when "bored"?
 *	- Unused
 *
 *	- Keypress Queue -- various data
 *
 *	- Keypress Queue -- pending keys
 *
 *	- Keypress Queue -- current macro
 *
 *	- Current screen image
 *
 *	- Desired screen image
 *
 *
 *	- Extra info (used by application)
 *
 *	- Extra data (used by implementation)
 *
 *	- Hook for init-ing the term
 *	- Hook for nuke-ing the term
 *
 *	- Hook for various actions
 *	- Hook for placing a cursor
 *	- Hook for erasing a block of characters
 *	- Hook for drawing a string of characters
 */

typedef struct _term term;

struct _term {

    bool initialized;
    bool soft_cursor;
    bool scan_events;
    bool unused_flag;

    byte key_head;
    byte key_tail;
    byte key_xtra;
    byte key_size;
    
    char *key_queue;

    cptr key_macro;

    term_win *old;

    term_win *scr;


    vptr info;

    vptr data;
    
    void (*init_hook)(term *t);
    void (*nuke_hook)(term *t);
    
    errr (*xtra_hook)(int n, int v);
    errr (*curs_hook)(int x, int y, int z);
    errr (*wipe_hook)(int x, int y, int w, int h);
    errr (*text_hook)(int x, int y, int n, byte a, cptr s);
};







/**** Available Constants ****/

/* Common keys */
#define DELETE          0x7f
#define ESCAPE          '\033'

/* Standard attributes */
#define TERM_BLACK             0
#define TERM_WHITE             1
#define TERM_GRAY              2
#define TERM_ORANGE            3
#define TERM_RED               4
#define TERM_GREEN             5
#define TERM_BLUE              6
#define TERM_UMBER             7
#define TERM_D_GRAY            8
#define TERM_L_GRAY            9
#define TERM_VIOLET            10
#define TERM_YELLOW            11
#define TERM_L_RED             12
#define TERM_L_GREEN           13
#define TERM_L_BLUE            14
#define TERM_L_UMBER           15

/* Available levels */
#define TERM_LEVEL_HARD_SHUT	1	/* Hardware Suspend */
#define TERM_LEVEL_SOFT_SHUT	2	/* Software Suspend */
#define TERM_LEVEL_SOFT_OPEN	3	/* Software Resume */
#define TERM_LEVEL_HARD_OPEN	4	/* Hardware Resume */

/* Definitions for "Term_xtra" */
#define TERM_XTRA_CHECK	11	/* Check for event */
#define TERM_XTRA_EVENT	12	/* Block until event */
#define TERM_XTRA_NOISE 21	/* Make a noise */
#define TERM_XTRA_FLUSH 22	/* Flush output */
#define TERM_XTRA_INVIS 31	/* Cursor invisible */
#define TERM_XTRA_BEVIS 32	/* Cursor visible */
#define TERM_XTRA_REACT 41	/* React to global veriable changes */
#define TERM_XTRA_LEVEL 91	/* Change the "level" (see above) */

/* Max recursion depth of "screen memory" */
/* Note that unused screens waste only 32 bytes each */
#define MEM_SIZE 16



/**** Available Macros ****/

/* Access to the char/attr at a given location */
/* This can be used for both access AND assignment */
#define tw_a(W,X,Y) ((W)->a[(W)->w*(Y)+(X)])
#define tw_c(W,X,Y) ((W)->c[(W)->w*(Y)+(X)])



/**** Available Variables ****/

extern term *Term;


/**** Available Functions ****/

extern errr term_win_wipe(term_win*);
extern errr term_win_load(term_win*, term_win*);
extern errr term_win_nuke(term_win*);
extern errr term_win_init(term_win*, int, int);

extern errr Term_xtra(int n, int v);
extern errr Term_curs(int x, int y, int z);
extern errr Term_wipe(int x, int y, int w, int h);
extern errr Term_text(int x, int y, int n, byte a, cptr s);

extern int Term_kbhit(void);
extern int Term_inkey(void);
extern errr Term_keypress(int);
extern errr Term_key_push(int);
extern errr Term_flush(void);
extern errr Term_fresh(void);
extern errr Term_redraw(void);
extern errr Term_update(void);
extern errr Term_resize(int,int);
extern errr Term_bell(void);
extern errr Term_save(void);
extern errr Term_load(void);
extern errr Term_show_cursor(void);
extern errr Term_hide_cursor(void);
extern errr Term_gotoxy(int x, int y);
extern errr Term_locate(int *x, int *y);
extern errr Term_draw(int x, int y, byte a, char c);
extern errr Term_what(int x, int y, byte *a, char *c);
extern errr Term_addch(byte a, char c);
extern errr Term_addstr(int n, byte a, cptr s);
extern errr Term_putch(int x, int y, byte a, char c);
extern errr Term_putstr(int x, int y, int n, byte a, cptr s);
extern errr Term_erase(int x1, int y1, int x2, int y2);
extern errr Term_clear(void);

extern errr Term_activate(term*);

extern errr term_nuke(term*);
extern errr term_init(term*, int w, int h, int k);

#endif


