/* File: main-ibm.c */
 
/* Purpose: Termcap support for "term.c", for the IBM */
/* Author: tanksley@coyote.csusm.edu (Billy Tanksley) */
 
#include "angband.h"
 
 
 
/*
 * Author: Billy Tanksley <tanksley@coyote.csusm.edu>
 *
 * Use "Makefile.ibm" to compile Angband using this file.
 */
 
 
#ifdef USE_IBM
 
#ifndef __GO32__
# error This file will not compile on non-extended IBM
#endif
 
#include <string.h>
#include <pc.h>
#include <osfcn.h>
#include <bios.h>
 
 
/*
 * This file allows use of the IBM-compatible video without requiring the
 * "curses" routines.
 *
 * This file will not function on anything besides MSDOS machines. :)
 */
 
 
/*
 * Screen-size macros
 */
#define rows ScreenRows()
#define cols ScreenCols()
 
 
/**********************************************************************
 *  
 *  defines for color video attributes in DOS
 *
 *********************************************************************/
 
# define VID_BLACK       0
# define VID_BLUE        1
# define VID_GREEN       2
# define VID_CYAN        3
# define VID_RED         4
# define VID_MAGENTA     5
# define VID_YELLOW      6
# define VID_WHITE       7
 
# define VID_BLINK       8
# define VID_BRIGHT      8
 
# define IBM_BLACK             VID_BLACK
# define IBM_WHITE             VID_WHITE
# define IBM_GRAY              VID_WHITE
# define IBM_ORANGE            (VID_YELLOW | VID_BRIGHT)
# define IBM_RED               VID_RED
# define IBM_GREEN             VID_GREEN
# define IBM_BLUE              VID_BLUE
# define IBM_UMBER             VID_YELLOW
# define IBM_D_GRAY            (VID_BLACK | VID_BRIGHT)
# define IBM_L_GRAY            (VID_CYAN | VID_BRIGHT)
# define IBM_VIOLET            VID_MAGENTA
# define IBM_YELLOW            (VID_YELLOW | VID_BRIGHT)
# define IBM_L_RED             (VID_RED | VID_BRIGHT)
# define IBM_L_GREEN           (VID_GREEN | VID_BRIGHT)
# define IBM_L_BLUE            (VID_BLUE | VID_BRIGHT)
# define IBM_L_UMBER           VID_YELLOW
 
 
/*
 * The Angband color table
 */
static const byte ibm_color[] = {
 
    IBM_BLACK, IBM_WHITE, IBM_GRAY, IBM_ORANGE,
    IBM_RED, IBM_GREEN, IBM_BLUE, IBM_UMBER,
    IBM_D_GRAY, IBM_L_GRAY, IBM_VIOLET, IBM_YELLOW,
    IBM_L_RED, IBM_L_GREEN, IBM_L_BLUE, IBM_L_UMBER
   /* there may be 64 official Angband colors someday, so don't
    * fill these in with stuff you'll need.  Use the colors after 64.
    */
};
 
 
/*
 * This array is used for "wiping" the screen
 * It is initialized in "init_ibm()"
 */
static short wiper[256];
 
 
/*
 * The main screen
 */
static term term_screen_body;
 
 
 
 
/*
 * Move the cursor
 */
static errr Term_curs_ibm(int x, int y, int z)
{
    ScreenSetCursor(y,x);
    return (0);
}
 
 
/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_ibm(int x, int y, int n, byte a, cptr s)
{
    int i; byte tempch;
 
    unsigned short attr = ((short)(ibm_color[a])) << 8;
 
    unsigned short temp[256];
 
    /* Process "-1" case */
    if (n < 0) n = strlen(s);
 
    /* Force legal */
    if (n > cols-x) n = cols-x;
 
    /* Dump the attr/char pairs, one at a time, into fast memory */
    /* I belive I found a compiler bug here --WDT*/
    for (i=0; (i < n) && s[i]; i++)
    {
        tempch = s[i];
        temp[i] = (attr | tempch);
    }
   
 
    /* Now blast them to the screen (slower memory) */
    bcopy((void*)temp, (void*)ScreenPrimary + ((y*cols + x)<<1), n<<1);
 
    return (0);
}
 
 
 
/*
 * Erase a grid of space.
 * Can never be called before "init_ibm()"
 */
static errr Term_wipe_ibm(int x, int y, int w, int h)
{
    int iy;
 
    /* Paranoia -- Verify the dimensions */
    h = (rows < (h+y)) ? (rows-y) : h;
    w = (cols < (w+x)) ? (cols-x) : w;
 
    /* Efficiency -- handle "full screen" case */
    if (!x && !y && (w >= cols-1) && (h >= rows-1))
    {
        ScreenClear();
        return (0);
    }
 
    /* Line at a time direct memory writes */
    for (iy = y; iy < y+h; iy++)
    {
        /* Now blast the blank lines to the screen (slower memory) */
        bcopy((void*)wiper, (void*)ScreenPrimary + ((iy*cols + x)<<1), w<<1);
    }
 
    return (0);
}
 
 
/*
 *      Returns the shift state:
 * 
 *         7654 3210  Meaning
 *         
 *         ---- ---X  Right shift key down
 *         ---- --X-  Left shift key down
 *         ---- -X--  Ctrl key down
 *         ---- X---  Alt key down
 *         ---X ----  Scroll lock on
 *         --X- ----  Num lock on
 *         -X-- ----  Caps lock on
 *         X--- ----  Insert on
 */
#define MOD_RIGHT 1
#define MOD_LEFT 2
#define MOD_CTRL 4
#define MOD_ALT 8
 
/*
 * Handle a "special request"
 *
 * Hack -- treat "right shift" as a "special" modifier
 */
errr Term_xtra_ibm(int n)
{
    int i, j;
    
    /* Analyze the request */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE:
            (void)write(1, "\007", 1);
            return 0;
        
        /* Check for a keypress (no waiting) */
        case TERM_XTRA_CHECK:
 
            /* Check for a keypress */
            i = bioskey(1);
 
            /* Notice "no key ready", or fall through */
            if (!i) return 1;
 
        /* Wait for a single event */
        case TERM_XTRA_EVENT:
 
            /* Wait for a keypress */
            i = bioskey(0);
 
            /* It is a "normal" character */
            if ((i & 0xFF) != 0) {
 
                /* Enqueue the normal keypress */
                Term_keypress(i);
            }
            
            /* It is a "special key" */
            else {
                
                /* Check the modifiers */
                j = bioskey(2);
 
                /* Hack -- a special "macro introducer" */
                Term_keypress(31);
                
                /* Send the modifiers */
                if (j & CTRL) Term_keypress('C');
                if (j & LEFT) Term_keypress('S');
                if (j & ALT) Term_keypress('O');
                if (j & RIGHT) Term_keypress('X');
 
                /* The upper 8 bits are the "special key" */
                i = ((i >> 8) & 0xFF);
 
                /* Hack -- encode the keypress (in decimal) */
                Term_keypress('0' + (i % 1000) / 100);
                Term_keypress('0' + (i % 100) / 10);
                Term_keypress('0' + (i % 10));
 
                /* End the macro (with "return") */
                Term_keypress(15);
            }
 
            /* Success */
            return 0;
    }
 
    return (1);
}
 
 
/*
 * Initialize the IBM "visual module"
 */
errr init_ibm()
{
    int i;
 
    term *t = &term_screen_body;
    
    short attr = (short)(ibm_color[TERM_WHITE]) << 8;
    short blank = attr | (short)(' ');
 
    static int done = FALSE;
 
    /* Already done */
    if (done) return (-1);
 
    /* Build a "wiper line" of blank spaces */
    for (i = 0; i < 256; i++) wiper[i] = blank;
 
    /* Erase the screen */
    ScreenClear();
    
    /* Initialize the term */
    term_init(t, 80, 24, 64);
 
    /* Connect the hooks */
    t->text_hook = Term_text_ibm;
    t->wipe_hook = Term_wipe_ibm;
    t->curs_hook = Term_curs_ibm;
    t->xtra_hook = Term_xtra_ibm;
 
    /* Save it */
    term_screen = t;
    
    /* Activate it */
    Term_activate(term_screen);
    
    /* Done */
    done = TRUE;
 
    /* Success */
    return 0;
}

 
#endif /* USE_IBM */


