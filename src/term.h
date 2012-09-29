/* File: term.h */

#ifndef INCLUDED_TERM_H
#define INCLUDED_TERM_H

#include "h-include.h"


/*
 * A term_win is a "window" for the Term
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

#if 0
    byte *r1;
    byte *r2;
#endif

    byte *a;
    char *c;
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

/* Definitions for "Term_xtra" */
#define TERM_XTRA_NOISE 11	/* Make a noise */
#define TERM_XTRA_FLUSH 12	/* Flush output */
#define TERM_XTRA_INVIS 21	/* Cursor invisible */
#define TERM_XTRA_BEVIS 22	/* Cursor visible */
#define TERM_XTRA_LEAVE 91	/* Temporary suspend */
#define TERM_XTRA_ENTER 92	/* Resume from suspend */


/* Definitions for "Term_method()" */
#define TERM_SOFT_CURSOR	1
#define TERM_SCAN_EVENTS	2


/* Max recursion depth of "screen memory" */
/* Note that unused screens waste only 32 bytes each */
#define MEM_SIZE 16



/**** Available Macros ****/

/* Access to the char/attr at a given location */
/* This can be used for both access AND assignment */
#define tw_a(W,X,Y) ((W)->a[(W)->w*(Y)+(X)])
#define tw_c(W,X,Y) ((W)->c[(W)->w*(Y)+(X)])



/**** Available variables ****/


/*
 * Basic Init/Nuke hooks (optional) that can be provided by the various
 * "graphic modules" to do "module dependant" startup/shutdown when the
 * Term_init() and Term_nuke() routines are called (see below).
 */
extern void (*Term_init_hook)(void);
extern void (*Term_nuke_hook)(void);

 
/*
 * Hooks, provided by particular "graphic modules"
 * See, for example, "main-mac.c" for a Macintosh Module
 * These hooks are used below to provide the "bodies" of functions.
 */
extern void (*Term_wipe_hook)(int x, int y, int w, int h);
extern void (*Term_curs_hook)(int x, int y, int z);
extern void (*Term_text_hook)(int x, int y, int n, byte a, cptr s);
extern void (*Term_scan_hook)(int n);
extern void (*Term_xtra_hook)(int n);







/**** Available Functions ****/

extern errr term_win_wipe(term_win*);
extern errr term_win_load(term_win*, term_win*);
extern errr term_win_init(term_win*, int, int);

extern int Term_method(int, int);

extern int Term_kbhit(void);
extern int Term_inkey(void);
extern errr Term_keypress(int);
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
extern errr Term_init(void);
extern errr Term_nuke(void);
extern void Term_text(int x, int y, int n, byte a, cptr s);
extern void Term_wipe(int x, int y, int w, int h);
extern void Term_curs(int x, int y, int z);
extern void Term_scan(int n);
extern void Term_xtra(int n);

#endif


