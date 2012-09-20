/* File: main-emx.c */

/* Purpose: Support for OS/2 EMX Angband */
/* Author: ekraemer@pluto.nasim.cube.net (Ekkehard Kraemer) */

#ifdef __EMX__

/*
 * === Instructions for using Angband 2.7.X with OS/2 ===
 *
 * The patches (file "main-emx.c") to compile Angband 2.7.X under OS/2
 * were written by ekraemer@pluto.nasim.cube.net (Ekkehard Kraemer).
 *
 * TO COMPILE:
 *
 * - untar the archive into /angband (or /games/angband or whatever)
 * - change directory to /angband/src
 * - remove makefile, and rename makefile.emx to makefile
 * - run dmake (not gmake or make)
 * - change directory to /angband
 * - start angband.exe
 *
 * I used EMX 0.9a, but every EMX compiler since 0.8g or so should work
 * fine. EMX is available at ftp-os2.cdrom.com ("Hobbes"), as is dmake.
 *
 *  dmake:    ftp://ftp-os2.cdrom.com/all/program/dmake38X.zip
 *  EMX:      ftp://ftp-os2.cdrom.com/2_x/unix/emx???/   (most probably)
 *
 * Old savefiles must be renamed to follow the new "savefile" naming
 * conventions.  Either rename the savefile to "PLAYER", or start the
 * program with the "-uName" flag.  See "main.c" for details.  The
 * savefiles are stores in "./lib/save" if you forgot the old names...
 */

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
 * Angband color conversion table
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
    F_BLACK,                    /* Black */
    F_WHITE|INTENSITY,          /* White */
    F_WHITE,                    /* XXX Gray */
    F_RED|INTENSITY,            /* Orange */
    F_RED,                      /* Red */
    F_GREEN,                    /* Green */
    F_BLUE,                     /* Blue */
    F_BROWN,                    /* Brown */
    F_BLACK|INTENSITY,          /* Dark-grey */
    F_WHITE,                    /* XXX Light gray */
    F_MAGENTA,                  /* Purple */
    F_YELLOW|INTENSITY,         /* Yellow */
    F_RED|INTENSITY,            /* Light Red */
    F_GREEN|INTENSITY,          /* Light Green */
    F_BLUE|INTENSITY,           /* Light Blue */
    F_BROWN|INTENSITY           /* Light brown */
};


/*
 * The main screen
 */
static term term_screen_body;



/*
 * Check for events -- called by "Term_scan_emx()"
 *
 * Note -- this is probably NOT the most efficient way
 * to "wait" for a keypress (TERM_XTRA_EVENT).
 *
 * _read_kbd(0,0,0) does this:
 *       - if no key is available, return -1
 *       - if extended key available, return 0
 *       - normal key available, return ASCII value (1 ... 255)
 *
 *  _read_kbd(0,1,0) waits on a key and then returns
 *       - 0, if it's an extended key
 *       - the ASCII value, if it's a normal key
 *
 *  *If* _read_kbd() returns 0, *then*, and only then, the next
 *  call to _read_kbd() will return the extended scan code.
 *
 * See "main-ibm.c" for a "better" use of "macro sequences".
 *
 * Note that this file does *NOT* currently extract modifiers
 * (such as Control and Shift).  See "main-ibm.c" for a method.
 */
static errr CheckEvents(int returnImmediately)
{
    int k = 0, ke = 0, ka = 0;

    /* Keyboard polling is BAD for multitasking systems */
    if (returnImmediately)
    {
        /* Check for a keypress (no waiting) */
        k = _read_kbd(0,0,0);

        /* Nothing ready */
        if (k < 0) return (1);
    }

    /* Wait for a keypress */
    else
    {
        /* Wait for a keypress */
        k = _read_kbd(0,1,0);
    }

    /* Get an extended scan code */
    if (!k) ke = _read_kbd(0,1,0);


    /* Mega-Hack -- Convert Arrow keys into directions */
    switch (ke)
    {
        case K_LEFT:     ka = '4'; break;
        case K_RIGHT:    ka = '6'; break;
        case K_UP:       ka = '8'; break;
        case K_DOWN:     ka = '2'; break;
        case K_HOME:     ka = '7'; break;
        case K_PAGEUP:   ka = '9'; break;
        case K_PAGEDOWN: ka = '3'; break;
        case K_END:      ka = '1'; break;
        case K_CENTER:   ka = '5'; break;
    }


    /* Special arrow keys */
    if (ka)
    {
        /* Hack -- Keypad key introducer */
        Term_keypress(30);

        /* Send the "numerical direction" */
        Term_keypress(ka);

        /* Success */
        return (0);
    }


    /* Hack -- normal keypresses */
    if (k)
    {
        /* Enqueue the key */
        Term_keypress(k);

        /* Success */
        return (0);
    }


    /* Hack -- introduce a macro sequence */
    Term_keypress(31);

    /* Hack -- send the key sequence */
    Term_keypress('0' + (ke % 1000) / 100);
    Term_keypress('0' + (ke % 100) / 10);
    Term_keypress('0' + (ke % 10));

    /* Hack --  end the macro sequence */
    Term_keypress(13);


    /* Success */
    return (0);
}


/*
 * Do a special thing (beep, flush, etc)
 */
static errr Term_xtra_emx(int n, int v)
{
    switch (n)
    {
        case TERM_XTRA_NOISE: putchar(7); return (0);
        case TERM_XTRA_INVIS: v_hidecursor(); return (0);
        case TERM_XTRA_BEVIS: v_ctype(curs_start,curs_end); return (0);
        case TERM_XTRA_CHECK: return (CheckEvents(TRUE));
        case TERM_XTRA_EVENT: return (CheckEvents(FALSE));
    }

    return (1);
}



/*
 * Display a cursor, on top of a given attr/char
 */
static errr Term_curs_emx(int x, int y, int z)
{
    v_gotoxy(x,y);
    v_ctype(curs_start,curs_end);
    return (0);
}


/*
 * Erase a grid of space (as if spaces were printed)
 */
static errr Term_wipe_emx(int x, int y, int w, int h)
{
    int t;

    /* Put spaces one row at a time */
    for (t=y; t<y+h; t++)
    {
        v_gotoxy(x,t);
        v_putn(' ',w);
    }

    return (0);
}


/*
 * Draw some text, wiping behind it first
 *
 * XXX Place the following lines in "a_list.txt":
 *
 *  # Normal Floor (white, tiny centered dot)
 *  K:441:1/250
 *
 *  # Granite Wall (white, solid gray block)
 *  K:442:1/177
 */
static errr Term_text_emx(int x, int y, int n, unsigned char a, cptr s)
{
    /* Convert the color and put the text */
    v_attrib(colors[a]);
    v_gotoxy(x,y);
    v_putm(s,n);
    return (0);
}


/*
 * EMX initialization
 */
static void Term_init_emx(term *t)
{
    v_init();
    v_getctype(&curs_start,&curs_end);
    v_clear();
}


/*
 * EMX shutdown
 */
static void Term_nuke_emx(term *t)
{
    /* Move the cursor to bottom of screen */
    v_gotoxy(0,23);

    /* Restore the cursor (not necessary) */
    v_ctype(curs_start,curs_end);

    /* Set attribute to gray on black */
    v_attrib(F_WHITE);

    /* Clear the screen */
    v_clear();
}



/*
 * Prepare "term.c" to use "__EMX__" built-in video library
 */
errr init_emx(void)
{
    term *t = &term_screen_body;


    /* Initialize the term -- big key buffer */
    term_init(t, 80, 24, 1024);

    /* Special hooks */
    t->init_hook = Term_init_emx;
    t->nuke_hook = Term_nuke_emx;

    /* Add the hooks */
    t->text_hook = Term_text_emx;
    t->wipe_hook = Term_wipe_emx;
    t->curs_hook = Term_curs_emx;
    t->xtra_hook = Term_xtra_emx;

    /* Save it */
    term_screen = t;

    /* Activate it */
    Term_activate(t);

    /* Success */
    return (0);
}


#endif   /* __EMX__ */

