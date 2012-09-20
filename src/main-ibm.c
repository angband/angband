/* File: main-ibm.c */

/* Purpose: Visual Display Support for "term.c", for the IBM */


#include "angband.h"

#include <go32.h>
union REGS { /* This is a verbatim copy of the declarations from
	      * DOS.H; there are several conflicts if I simply include
	      * the file. */
  struct {
    unsigned long ax;
    unsigned long bx;
    unsigned long cx;
    unsigned long dx;
    unsigned long si;
    unsigned long di;
    unsigned long cflag;
    unsigned long flags;
  } x;
  struct {
    unsigned char al;
    unsigned char ah;
    unsigned short upper_ax;
    unsigned char bl;
    unsigned char bh;
    unsigned short upper_bx;
    unsigned char cl;
    unsigned char ch;
    unsigned short upper_cx;
    unsigned char dl;
    unsigned char dh;
    unsigned short upper_dx;
  } h;
};

int int86(int ivec, union REGS *in, union REGS *out);


/*
 * Author: Billy Tanksley <tanksley@coyote.csusm.edu>
 *
 * Support for DPMI added by ctate@world.std.com (Chris Tate)
 *
 * Angband 2.7.5 modifications by benh@voicenet.com
 *
 * Use "Makefile.ibm" to compile Angband using this file.
 *
 * This file will not function on anything besides MSDOS machines. :)
 * In addition, it will not compile on non-extended IBM machines
 */



#ifdef USE_IBM

#ifndef __GO32__
# error This file will not compile on non-extended IBM
#endif

#include <string.h>
#include <pc.h>
#include <osfcn.h>
#include <bios.h>

#undef CTRL
#define CTRL(X)		((X)&037)



/*
 * Virtual Screen Size
 */
static int rows, cols;

/*
 * Virtual Screen
 */
static byte *VirtualScreen;


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
# define IBM_WHITE             (VID_WHITE | VID_BRIGHT)
# define IBM_GRAY              VID_WHITE
# define IBM_ORANGE            (VID_RED | VID_BRIGHT)
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
    register int i;
    register byte attr;
    register byte *dest;

    /* Convert the color */
    attr = ibm_color[a];

    /* Access the destination */
    dest = VirtualScreen + (((cols * y) + x) << 1);

    /* Virtually write the string */
    for (i = 0; (i < n) && s[i]; i++) {
        *dest++ = s[i];
        *dest++ = attr;
    }

    /* Dump the line */
    ScreenUpdateLine(VirtualScreen + ((cols*y) << 1), y);

    /* Success */
    return (0);
}


/*
 * Erase a block of the screen
 */
static errr Term_wipe_ibm(int x, int y, int w, int h)
{
    int iy;

    /* Paranoia -- Verify the dimensions */
    if (rows < (h+y)) h = (rows-y);
    if (cols < (w+x)) w = (cols-x);

    /* Efficiency -- handle "full screen" case */
    if (!x && !y && (w >= cols-1) && (h >= rows-1)) {

        /* Clear the screen */
        ScreenClear();

        /* Clear the Virtual Screen */
        for (iy = 0; iy < rows; iy++) {
            bcopy(wiper, (VirtualScreen + ((iy*cols) << 1)), (cols << 1));
        }

        /* Success */
        return (0);
    }

    /* Erase the Block one Line at a time */
    for (iy = y; iy < y+h; iy++) {

        /* Wipe part of the virtual screen, and update */
        bcopy((void*)wiper, (void*)VirtualScreen + ((iy*cols + x)<<1), w<<1);
        ScreenUpdateLine(VirtualScreen + ((cols*iy) << 1), iy);
    }

    /* Success */
    return (0);
}



/*
 * Modifier flags (see below):
 */
#define K_RSHIFT	0	/* Right shift key down */
#define K_LSHIFT	1	/* Left shift key down */
#define K_CTRL		2	/* Ctrl key down */
#define K_ALT		3	/* Alt key down */
#define K_SCROLL	4	/* Scroll lock on */
#define K_NUM		5	/* Num lock on */
#define K_CAPS		6	/* Caps lock on */
#define K_INSERT	7	/* Insert on */


/*
 * Handle a "special request"
 *
 * Below, note that you can't get away with just sending the ASCII code,
 * since certain ways of pressing the keypad return no ASCII (only scan).
 * For instance, SHIFT while numlock is on.
 *
 * if a keypad key is pressed, we check whether it was numeric
 * (i.e. directional).  If so, we need to send some sort of directional
 * command.  The possible directional commands are walk and dig.  Also,
 * the shift key can cause any directional command to be repeated, so
 * that walk becomes run and dig becomes repeated digging.
 *
 *
 * This function was rewritten because nobody is allowed to reference
 * the "roguelike" flag except the inner circle.  -BEN-
 *
 * Hack -- This function automatically processes Control or Shift plus a
 * keypad key, instead of using a nice clean macro.  However, we still
 * send a leading "control-caret" so it is still possible (though ugly)
 * for the user to redefine control-keypad or shift-keypad.
 *
 * Note that we can send actual "numerical" directions, since all of the
 * Angband code is designed to accept them.  We use the "special command"
 * backslash ('\\') to specify the underlying form of tunneling ('+') and
 * running ('.').  Both of these take a numerical direction.
 *
 * So, all keypad keys first send the keypad signal (control-caret),
 * plus the "escaper" symbol ("\\"), plus the run ('.') or tunnel ('+')
 * command (if any is requested), plus the actual ascii value of the key
 * (usually numerical).  A much better method would probably be to send
 * the encoded "control-underscore" sequences when any modifier keys are
 * being pressed, but the "control-caret" method is slightly simpler for
 * the user to catch with macro patterns ("^^\\+5" for "control-keypad-5").
 *
 * Note that the "proper" way to handle this "mapping" thing is to
 * use the new "command macro" facility to make macros that map the
 * various control-underscore sequnces to the appropriate actions.
 *
 * Lastly, we may be able to speed up the I/O by providing another
 * "Term_xtra()" action which says that a certain line should be
 * flushed.  The question is which is faster, dumping a full 80
 * character line once, or dumping three or four much smaller
 * pieces of that line as they arrive.
 */


/* getxkey description:
 * Waits for the user to press one key, then returns that key.  Alt-key
 * combinations have 0x100 added to them, and extended keys have 0x200
 * added to them.
 */

static int saved_cur_high, saved_cur_low;

static errr Term_xtra_ibm(int n, int v)
{
    int i;
    int key;
    union REGS r;

    /* Analyze the request */
    switch (n) {
       
        case TERM_XTRA_BEVIS:
            r.h.ah = 1;
       	    r.h.ch = saved_cur_high;
            r.h.cl = saved_cur_low;
       	    int86(0x10, &r, &r);
       	    break;

        case TERM_XTRA_INVIS:
            r.h.ah = 1;
            r.x.cx = 0x2000;
       	    int86(0x10, &r, &r);
       	    break;

        /* Make a noise */
        case TERM_XTRA_NOISE:

            (void)write(1, "\007", 1);
            return 0;

        /* Check for a keypress (no waiting) */
        case TERM_XTRA_CHECK:

            /* Check for a keypress */
            if (!kbhit()) return (1);

            /* Fall through */

        /* Wait for a single event */
        case TERM_XTRA_EVENT:

            /* Wait for a keypress */
            key = getxkey();

            /* Normal keys */
            if (!(key & 0xFF00)) {

                /* Must be a 'normal' key */
                Term_keypress(key);
            }

            /* Handle "special" keys */
            else {

                /* Recieve the modifiers (shift, etc) */
                i = bioskey(2);

                /* Hack -- a special "macro introducer" */
                Term_keypress(31);

                /* Send the modifiers */
                if (i & K_CTRL) Term_keypress('C');
                if (i & K_LSHIFT) Term_keypress('S');
                if (i & K_ALT) Term_keypress('O');
                if (i & K_RSHIFT) Term_keypress('X');

                /* The upper 8 bits are the "special key" */
                /* Hack -- encode the keypress (in decimal) */
                Term_keypress('0' + (key % 1000) / 100);
                Term_keypress('0' + (key % 100) / 10);
                Term_keypress('0' + (key % 10));

                /* End the macro (with "return") */
                Term_keypress(13);
            }

            /* Success */
            return 0;
    }

    /* Unknown request */
    return (1);
}



/*
 * Init a Term
 */
static void Term_init_ibm(term *t)
{
    union REGS r;
 
    /* Extract the "normal" cursor */   
    r.h.ah = 3;
    r.h.bh = 0;
    int86(0x10, &r, &r);
    saved_cur_high = r.h.ch;
    saved_cur_low = r.h.cl;

    /* Erase the screen */
    ScreenClear();
}


/*
 * Nuke a Term
 */
static void Term_nuke_ibm(term *t)
{
    union REGS r;

    /* Make the cursor visible */
    r.h.ah = 1;
    r.h.ch = saved_cur_high;
    r.h.cl = saved_cur_low;
    int86(0x10, &r, &r);

    /* Erase the screen */
    ScreenClear();
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


    /* Paranoia -- Already done */
    if (done) return (-1);


    /* Build a "wiper line" of blank spaces */
    for (i = 0; i < 256; i++) wiper[i] = blank;


    /* Acquire the size of the screen */
    rows = ScreenRows();
    cols = ScreenCols();

    /* Paranoia -- require minimum size */
    if (cols < 80) quit("Screen must be 80 columns!");
    
    /* Make the virtual screen */
    C_MAKE(VirtualScreen, rows * cols * 2, byte);

    /* Clear the virtual screen */
    for (i = 0; i < rows; i++) {

        /* Wipe that row */
        bcopy(wiper, (VirtualScreen + ((i*cols) << 1)), (cols << 1));
    }


    /* Initialize the term -- very large key buffer */
    term_init(t, cols, rows - 1, 1024);

    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_ibm;
    t->nuke_hook = Term_nuke_ibm;

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


