/* File: main-ibm.c */
 
/* Purpose: Termcap support for "term.c", for the IBM */
/* Author: tanksley@coyote.csusm.edu (Billy Tanksley) */

#include "angband.h"
 
#ifdef USE_IBM
 
#ifndef __GO32__
# error This file will not compile on non-extended IBM
#endif
 
#include <string.h>
#include <pc.h>
#include <osfcn.h>
#include <bios.h>
#include <ctype.h>
 

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
 

/*
 * Move the cursor
 */
static void Term_curs_ibm(int x, int y, int z)
{
    ScreenSetCursor(y,x);
}
 
/*
 * Erase a grid of space
 */
static void Term_wipe_ibm(int x, int y, int w, int h)
{
   int iy,offset;
 
   h = (rows < (h+y)) ? (rows-y) : h;
   w = (cols < (w+x)) ? (cols-x) : w;
   
   w = w<<1;
 
   /* Hack -- handle "full screen" case */
   if (!x && !y && (w >= cols) && (h >= rows))
   {
       ScreenClear();
       return;
   }

   /* Line at a time direct memory writes */
   for (iy = y; iy <= y+h; iy++)
   {
       bzero( (void*)ScreenPrimary + (iy*cols + x)<<1, w);
   }
}
 
 
/*
 * Place some text on the screen using an attribute
 */
static void Term_text_ibm(int x, int y, int n, byte a, cptr s)
{
    int i;

    /* Process "-1" case */
    if (n < 0) n = strlen(s);

    /* Force legal */
    if (n > cols-x) n = cols;

    /* Dump the chars, one at a time */
    for (i=0; (i < n) && s[i]; i++, x++)
    {
       if (!iscntrl(s[i])) ScreenPutChar(s[i], a, x, y);
    }
}
 
 
/*
 * Scan for events.  If "n" is "-1", scan until done.
 * Otherwise, block until "n" events have been handled.
 * XXX Need to choose a behavior if "n" is zero...
 */
static void Term_scan_ibm(int n)
{
    int i;
   
    /* Scan events until done */
    if (n < 0)
    {
        /* Enqueue keys while available */
        while (i = bioskey(1)) Term_keypress(i);

        /* All done */
        return;
    }  
 
    /* Scan events until 'n' are processed */
    while (n-- > 0) 
    {
        /* Get a keypress */
        i = bioskey(0);

        /* Enqueue the keypress */
        Term_keypress(i);
    }
}
 
 
 
/*
 * Handle a "special request" (only one implemented)
 */
static void Term_xtra_ibm(int n)
{
    /* Analyze the request */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE: (void)write(1, "\007", 1); break;
    }
}
 
 
/*
 * A routine to prepare all the hooks above
 */
errr init_ibm()
{
    Term_text_hook = Term_text_ibm;
    Term_wipe_hook = Term_wipe_ibm;
    Term_curs_hook = Term_curs_ibm;
    Term_scan_hook = Term_scan_ibm;
    Term_xtra_hook = Term_xtra_ibm;
}


#endif /* USE_IBM */


