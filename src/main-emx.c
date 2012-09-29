/* File: main-emx.c */

/* Purpose: Support for OS/2 EMX Angband */

#ifdef __EMX__

#include <stdlib.h>
#include <sys/kbdscan.h>
#include <sys/video.h>

#include "angband.h"


/*
 * Current cursor "size"
 */
static int curs_start=0;
static int curs_end=0;

/*
 * Color conversion table
 *
 * XXX The lack of "Yellow" may be significant.
 * XXX Consider losing "Light Brown" instead.
 * XXX Also, "Orange" can be done as "Yellow".
 *
 * Note that "Light Green"/"Yellow"/"Red" are used for
 * "good"/"fair"/"awful" stats and conditions.
 *
 * But "brown"/"light brown"/"orange" are really only used
 * for "objects" (such as doors and spellbooks), and these
 * values can in fact be re-defined.
 *
 * Future versions of Angband will probably allow the
 * "use" of more that 16 colors, so "extra" entries can be
 * made at the end of this table in preparation.
 */
static int colors[16]=
{
    F_BLACK,			/* Black */
    F_WHITE|INTENSITY,		/* White */
    F_WHITE,			/* Gray */
    F_RED|INTENSITY,		/* Orange */
    F_RED,			/* Red */
    F_GREEN,			/* Green */
    F_BLUE,			/* Blue */
    F_BROWN,			/* Brown */
    F_WHITE,			/* Dark-grey */
    F_WHITE,			/* Light gray */
    F_MAGENTA,			/* Purple */
    F_BROWN|INTENSITY,		/* Yellow */
    F_RED|INTENSITY,		/* Light Red */
    F_GREEN|INTENSITY,		/* Light Green */
    F_BLUE|INTENSITY,		/* Light Blue */
    F_BROWN|INTENSITY		/* Light brown */
};



/*
 * Check for events -- called by "Term_scan_emx()"
 */
static int CheckEvents(void)
{
    int k;

    /* get key but don't wait on it */
    k=_read_kbd(0,0,0);

    /* key available */
    if (k>=0)
    {
        int ke = 0;

        /* Get an extended scan code */
        if (k==0) ke=_read_kbd(0,1,0);

        /* Mega-Hack -- Convert Arrow keys into directions */
        switch (ke)
        {
            case K_LEFT:    k = rogue_like_commands ? 'h' : '4';    break;
            case K_RIGHT:   k = rogue_like_commands ? 'l' : '6';    break;
            case K_UP:      k = rogue_like_commands ? 'k' : '8';    break;
            case K_DOWN:    k = rogue_like_commands ? 'j' : '2';    break;
            case K_HOME:    k = rogue_like_commands ? 'y' : '7';    break;
            case K_PAGEUP:  k = rogue_like_commands ? 'u' : '9';    break;
            case K_PAGEDOWN:k = rogue_like_commands ? 'n' : '3';    break;
            case K_END:     k = rogue_like_commands ? 'b' : '1';    break;
            case K_CENTER:  k = rogue_like_commands ? '.' : '5';    break;
        }

        /* Notice the keypress */
        if (k)
        {
            Term_keypress(k);
            return 1;
        }
    }

    /* Nothing ready */
    return 0;
}


/* 
 * Do a special thing (beep, flush, etc)
 */
static void Term_xtra_emx(int n)
{
    switch (n)
    {
        case TERM_XTRA_NOISE:   putchar(7);                         break;
        case TERM_XTRA_FLUSH:   /* ignore */                        break;
        case TERM_XTRA_INVIS:   v_hidecursor();                     break;
        case TERM_XTRA_BEVIS:   v_ctype(curs_start,curs_end);       break;
    }
}


/*
 * Scan for, and process, some events
 */
static void Term_scan_emx(int n)
{
    if (n < 0)
    {
        /* Scan until no events left */
        while (CheckEvents());
    }                                                  

    else
    {
        /* Scan until 'n' events have been handled */
        while (n > 0) if (CheckEvents()) n--;
    }
}


/*
 * Display a cursor, on top of a given attr/char
 */
static void Term_curs_emx(int x, int y, int z)
{
    v_gotoxy(x,y);
    v_ctype(curs_start,curs_end);
}


/*
 * Erase a grid of space (as if spaces were printed)
 */
static void Term_wipe_emx(int x, int y, int w, int h)
{
    int t;

    /* Put spaces one row at a time */
    for (t=y; t<y+h; t++)
    {
        v_gotoxy(x,t);
        v_putn(' ',w);
    }
}


/*
 * Draw some text, wiping behind it first
 *
 * XXX Note that "__EMX__" codes for floor/wall are:
 *   Floor (250) = Tiny centered dot
 *   Wall  (176) = Solid gray block
 */
static void Term_text_emx(int x, int y, int n, unsigned char a, cptr s)
{
    /* Convert the color and put the text */
    v_attrib(colors[a]);
    v_gotoxy(x,y);
    v_putm(s,n);
}


/*
 * EMX initialization
 */
static void Term_init_emx(void)
{
    v_init();
    v_getctype(&curs_start,&curs_end);
}


/*
 * EMX shutdown
 */
static void Term_nuke_emx(void)
{
    /* XXX Restore the cursor? */
}



/*
 * Prepare "term.c" to use "__EMX__" built-in video library
 */
errr init_emx(void)
{
    /* Add the hooks */
    Term_init_hook = Term_init_emx;
    Term_nuke_hook = Term_nuke_emx;
    Term_text_hook = Term_text_emx;
    Term_wipe_hook = Term_wipe_emx;
    Term_curs_hook = Term_curs_emx;
    Term_scan_hook = Term_scan_emx;
    Term_xtra_hook = Term_xtra_emx;

    /* Success */
    return (0);
}


#endif   /* __EMX__*/

