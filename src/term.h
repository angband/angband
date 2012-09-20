/* File: term.h */

#ifndef INCLUDED_TERM_H
#define INCLUDED_TERM_H

#include "h-basic.h"



/*
 * A term_win is a "window" for a Term
 *
 *	- Window Size (max 256x256)
 *
 *	- Flag -- Erase the window at next refresh
 *	- Flag -- Unused (flush something or other)
 *
 *	- Cursor Useless/Visible codes
 *	- Cursor Location (see "Useless")
 *
 *	- Min/Max modified rows (per window)
 *
 *	- Array[h] -- Min/Max modified cols (per row)
 *
 *	- Array[h] -- Access to the attribute array
 *	- Array[h] -- Access to the character array
 *
 *	- Array[h*w] -- Attribute array
 *	- Array[h*w] -- Character array
 *
 * Note that the attr/char pair at (x,y) is a[y][x]/c[y][x]
 * and that the row of attr/chars at (0,y) is a[y]/c[y]
 *
 * Note that "y1<=y2" iff any changes have occured on the screen.
 * Note that "r1[y]<=r2[y]" iff any changes have occured in row "y".
 *
 * Note that "blanks" written past "rm[y]" in row "y" are ignored.
 * That is, they would be if we were actually using this array
 */

typedef struct term_win term_win;

struct term_win {

    u16b w, h;

    bool erase;
    bool flush;

    bool cu, cv;
    byte cx, cy;

    byte y1, y2;

    byte *x1;
    byte *x2;

    byte **a;
    char **c;

    byte *va;
    char *vc;
};



/*
 * An actual "term" structure
 *
 *	- Extra info (used by application)
 *
 *	- Extra data (used by implementation)
 *
 *	- Have we been activated for the first time?
 *	- Do our support routines use a "software cursor"?
 *	- Should we call the "Event Loop" when "bored"?
 *	- May we ignore the "underlying" color of "spaces"?
 *
 *	- Keypress Queue -- various data
 *
 *	- Keypress Queue -- pending keys
 *
 *	- Current screen image
 *
 *	- Desired screen image
 *
 *	- Hook for init-ing the term
 *	- Hook for nuke-ing the term
 *
 *	- Hook for user actions
 *	- Hook for extra actions
 *
 *	- Hook for placing the cursor
 *
 *	- Hook for drawing some blank spaces
 *
 *	- Hook for drawing a special picture
 *	- Hook for drawing a string of characters
 */

typedef struct term term;

struct term {

    vptr user;

    vptr data;

    bool initialized;
    bool soft_cursor;
    bool scan_events;
    bool dark_blanks;

    char *key_queue;

    u16b key_head;
    u16b key_tail;
    u16b key_xtra;
    u16b key_size;

    term_win *old;
    term_win *scr;

    void (*init_hook)(term *t);
    void (*nuke_hook)(term *t);

    errr (*user_hook)(int n);
    errr (*xtra_hook)(int n, int v);

    errr (*curs_hook)(int x, int y);
    
    errr (*wipe_hook)(int x, int y, int n);

    errr (*pict_hook)(int x, int y, int p);
    errr (*text_hook)(int x, int y, int n, byte a, cptr s);
};







/**** Available Constants ****/


/*
 * Max recursion depth of "screen memory"
 *
 * Note that "unused" screens waste only 32 bytes each
 */
#define MEM_SIZE 8


/*
 * Definitions for the "actions" of "Term_xtra()"
 *
 * These values may be used as the first parameter of "Term_xtra()",
 * with the second parameter depending on the "action" itself.  Many
 * of the actions shown below are optional on at least one platform.
 */
#define TERM_XTRA_CLEAR 1	/* Clear the screen */
#define TERM_XTRA_EVENT	2	/* Process some pending events */
#define TERM_XTRA_FLUSH 3	/* Flush all pending events */
#define TERM_XTRA_FROSH 4	/* Flush one row (optional) */
#define TERM_XTRA_FRESH 5	/* Flush all rows (optional) */
#define TERM_XTRA_INVIS 6	/* Make cursor invisible (optional) */
#define TERM_XTRA_BEVIS 7	/* Make cursor visible (optional) */
#define TERM_XTRA_NOISE 8	/* Make a noise (optional) */
#define TERM_XTRA_SOUND 9	/* Make a sound (optional) */
#define TERM_XTRA_ALIVE 10	/* Change the "hard" level (optional) */
#define TERM_XTRA_LEVEL 11	/* Change the "soft" level (optional) */



/**** Available Variables ****/

extern term *Term;


/**** Available Functions ****/

extern errr term_win_wipe(term_win *t);
extern errr term_win_load(term_win *t, term_win *s);
extern errr term_win_nuke(term_win *t);
extern errr term_win_init(term_win *t, int w, int h);
extern errr Term_user(int n);
extern errr Term_xtra(int n, int v);
extern errr Term_erase(int x, int y, int w, int h);
extern errr Term_clear(void);
extern errr Term_redraw(void);
extern errr Term_save(void);
extern errr Term_load(void);
extern errr Term_fresh(void);
extern errr Term_update(void);
extern errr Term_resize(int w, int h);
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
extern errr Term_flush(void);
extern errr Term_keypress(int k);
extern errr Term_key_push(int k);
extern errr Term_inkey(char *ch, bool wait, bool take);
extern errr Term_activate(term *t);
extern errr term_nuke(term *t);
extern errr term_init(term *t, int w, int h, int k);

#endif


