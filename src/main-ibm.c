/* File: main-ibm.c */

/* Purpose: Visual Display Support for "term.c", for the IBM */


/*
 * Original code by "Billy Tanksley (wtanksle@ucsd.edu)"
 * Use "Makefile.ibm" to compile Angband using this file.
 *
 * Support for DPMI added by "Chris Tate (ctate@world.std.com)"
 *
 * Support for DJGPP v2 by "Scott Egashira (egashira@u.washington.edu)"
 *
 * Extensive modifications by "Ben Harrison (benh@voicenet.com)",
 * including "collation" of the Watcom C/C++ and DOS-286 patches.
 *
 * Watcom C/C++ changes by "David Boeren (akemi@netcom.com)"
 * Use "Makefile.wat" to compile this file with Watcom C/C++.
 *
 * DOS-286 (conio.h) changes by (Roland Jay Roberts (jay@map.com)
 * Use "Makefile.286" (not ready) to compile this file for DOS-286.
 *
 * Note that, for macro triggers, the left and right shift keys
 * are identical, but using both simultaneously can be detected,
 * though I would imagine this information has little application.
 *
 * This file should work with Angband 2.7.9v2, but needs testing.
 */


#include "angband.h"


/*
 * Use "VirtualScreen"
 */
#define USE_VIRTUAL


/*
 * Use "curs_set()" calls
 */
#define USE_CURS_SET


/*
 * XXX XXX XXX Hack -- Support for Watcom C/C++
 */
#ifdef USE_WAT
# define USE_IBM
#endif


/*
 * XXX XXX XXX Hack -- Support for DOS-286
 */
#ifdef USE_286
# define USE_IBM
# define USE_WAT
# define USE_CONIO
# undef USE_VIRTUAL
# undef USE_CURS_SET
#endif



#ifdef USE_IBM


#ifdef USE_WAT

# include <bios.h>
# include <dos.h>

#else /* USE_WAT */

# ifdef __DJGPP__

/*
 * The following unions/structures are "stolen" from "DOS.H",
 * which cannot simply be included because it induces conflicts
 */

union REGS {

  struct {
        unsigned short di, _upper_di;
        unsigned short si, _upper_si;
        unsigned short bp, _upper_bp;
        unsigned short cflag, _upper_cflag;
        unsigned short bx, _upper_bx;
        unsigned short dx, _upper_dx;
        unsigned short cx, _upper_cx;
        unsigned short ax, _upper_ax;
        unsigned short flags;
  } x;
 
  struct {
        unsigned short di, _upper_di;
        unsigned short si, _upper_si;
        unsigned short bp, _upper_bp;
        unsigned short cflag, _upper_cflag;
        unsigned short bx, _upper_bx;
        unsigned short dx, _upper_dx;
        unsigned short cx, _upper_cx;
        unsigned short ax, _upper_ax;
        unsigned short flags;
  } w;
 
  struct {
        unsigned short di, _upper_di;
        unsigned short si, _upper_si;
        unsigned short bp, _upper_bp;
        unsigned long cflag;
        unsigned char bl;
        unsigned char bh;
        unsigned short _upper_bx;
        unsigned char dl;
        unsigned char dh;
        unsigned short _upper_dx;
        unsigned char cl;
        unsigned char ch;
        unsigned short _upper_cx;
        unsigned char al;
        unsigned char ah;
        unsigned short _upper_ax;
        unsigned short flags;
  } h;
};


# else /* __DJGPP__ */

/*
 * Include the "go32" support
 */
#  include <go32.h>
 
/*
 * Require the "go32" support
 */
#  ifndef __GO32__
#   error This file will not compile on non-extended IBM
#  endif


/*
 * The following unions/structures are "stolen" from "DOS.H",
 * which cannot simply be included because it induces conflicts
 */

union REGS {

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


# endif /* __DJGPP__ */


/*
 * External function -- the "int86()" call in "DOS.H"
 */
extern int int86(int ivec, union REGS *in, union REGS *out);


/*
 * More header files
 */
# include <pc.h>
# include <osfcn.h>
# include <bios.h>

/*
 * Hack -- use "bcopy()" instead of "memcpy()"
 */
# define memcpy(T,S,N) bcopy(S,T,N)


#endif /* USE_WAT */


#ifdef USE_CONIO

# include <conio.h>

/*
 * Hack -- write directly to video card
 */
extern int directvideo = 1;

#endif /* USE_CONIO */


/*
 * Keypress input modifier flags (hard-coded by DOS)
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
 * Basic color bits (hard-coded by DOS)
 */
#define VID_BLACK	0
#define VID_BLUE	1
#define VID_GREEN	2
#define VID_CYAN	3
#define VID_RED		4
#define VID_MAGENTA	5
#define VID_YELLOW	6
#define VID_WHITE	7

/*
 * Extended color bits (hard-coded by DOS)
 */
#define VID_BLINK	8
#define VID_BRIGHT	8




/*
 * Screen Size
 */
static int rows;
static int cols;


/*
 * Physical Screen
 */
#define PhysicalScreen ((byte *)(0xB800 << 4))


#ifdef USE_VIRTUAL

/*
 * Virtual Screen Contents
 */
static byte *VirtualScreen;

#else

/*
 * Physical screen access
 */
#define VirtualScreen PhysicalScreen

#endif


#ifdef USE_CURS_SET

/*
 * Hack -- the cursor "visibility"
 */
static int saved_cur_v;
static int saved_cur_high;
static int saved_cur_low;

#endif


/*
 * This array is used for "wiping" the screen
 */
static short wiper[256];


/*
 * The main screen (currently the only screen)
 */
static term term_screen_body;



/*
 * The Angband color table (see "defines.h")
 */
static const byte ibm_color[16] = {

    VID_BLACK,
    VID_WHITE | VID_BRIGHT,
    VID_WHITE,
    VID_RED | VID_BRIGHT,
    VID_RED,
    VID_GREEN,
    VID_BLUE,
    VID_YELLOW,
    VID_BLACK | VID_BRIGHT,
    VID_CYAN | VID_BRIGHT,
    VID_MAGENTA,
    VID_YELLOW | VID_BRIGHT,
    VID_RED | VID_BRIGHT,
    VID_GREEN | VID_BRIGHT,
    VID_BLUE | VID_BRIGHT,
    VID_YELLOW
};



#ifdef USE_WAT


/*
 * Hack -- Use with Watcom C/C++
 */
#define ScreenRows() \
	25

/*
 * Hack -- Use with Watcom C/C++
 */
#define ScreenCols() \
	80


# ifdef USE_CONIO


/*
 * Hack -- Use with Watcom C/C++
 */
#define ScreenClear() \
	clrscr()

/*
 * Hack -- Use with Watcom C/C++
 */
#define ScreenSetCursor(Y,X) \
	gotoxy((X)+1, (Y)+1)


# else /* USE_CONIO */


/*
 * Hack -- Use with Watcom C/C++
 */
#define bioskey(C) \
	_bios_keybrd(C)

/*
 * Hack -- Use with Watcom C/C++
 */
#define ScreenUpdateLine(B,Y) \
	memcpy(PhysicalScreen + ((Y)*160), (B), 160)

/*
 * Hack -- Use with Watcom C/C++
 */
static void ScreenClear(void)
{
    int iy;

    /* Clear each line */
    for (iy = 0; iy < rows; iy++) {

        /* Clear the line */
        memcpy((VirtualScreen + ((iy*cols) << 1)), wiper, (cols << 1));

#ifdef USE_VIRTUAL

        /* Hack -- Update the line */
        ScreenUpdateLine(wiper, iy);

#endif

    }
}

/*
 * Hack -- Use with Watcom C/C++
 */
static void ScreenSetCursor(int y, int x)
{
    union REGS r;

    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dl = x;
    r.h.dh = y;

    int386(0x10, &r, &r);
}


# endif /* USE_CONIO */


#endif /* USE_WAT */



/*
 * Move the cursor
 *
 * The given parameters are "valid".
 */
static errr Term_curs_ibm(int x, int y)
{
    /* Move the cursor */
    ScreenSetCursor(y,x);

    /* Success */
    return (0);
}


/*
 * Place some text on the screen using an attribute
 *
 * The given parameters are "valid".  Be careful with "a".
 * The string "s" is null terminated, and has length "n".
 */
static errr Term_text_ibm(int x, int y, int n, byte a, cptr s)
{
    register byte attr;
    register byte *dest;

    /* Convert the color */
    attr = ibm_color[a & 0x0F];

#ifdef USE_CONIO

    /* Set the attribute */
    textattr(attr);

    /* Place the cursor */
    gotoxy(x+1, y+1);

    /* Dump the text */
    for ( ; *s; s++) putch(*s);

#else /* USE_286 */

    /* Access the destination */
    dest = VirtualScreen + (((cols * y) + x) << 1);

    /* Build the buffer */
    while (*s) {
        *dest++ = *s++;
        *dest++ = attr;
    }

#endif

    /* Success */
    return (0);
}


/*
 * Erase a block of the screen
 *
 * The given parameters are "valid".
 */
static errr Term_wipe_ibm(int x, int y, int n)
{

#ifdef USE_CONIO

    /* Wipe the region */
    window(x+1,y+1,x+n,y+1);
    clrscr();
    window(1,1,cols,rows);

#else

    /* Wipe part of the virtual screen */
    memcpy(VirtualScreen + ((y*cols + x)<<1), wiper, n<<1);

#endif

    /* Success */
    return (0);
}


#ifdef USE_CURS_SET

/*
 * Hack -- set the cursor "visibility"
 */
static void curs_set(int v)
{
    /* If needed */
    if (saved_cur_v != v) {

        union REGS r;

        /* Set cursor */
        r.h.ah = 1;

        /* Visible */
        if (v) {

            /* Use the saved values */
            r.h.ch = saved_cur_high;
            r.h.cl = saved_cur_low;
        }
    
        /* Invisible */
        else {

            /* Make it invisible */
            r.x.cx = 0x2000;
        }

        /* Make the call */
        int86(0x10, &r, &r);
        
        /* Save the cursor state */
        saved_cur_v = v;
    }
}

#endif


/*
 * Process an event (check for a keypress)
 *
 * The keypress processing code is often the most system dependant part
 * of Angband, since sometimes even the choice of compiler is important.
 *
 * Note that we rely on the special "macro processing" ability of Angband,
 * which allows us to encode most "bizarre" keypresses in any way that
 * we like, as long as we follow the "Angband Macro" formalism, which allows
 * the user to make "macros" that convert the internal formalism into any
 * sequence of keypresses.  For this reason, we treat the "left shift" and
 * "right shift" keys as different keys, and we assume that if the user
 * wants both shift keys to have the same effect then he will make one
 * macro for each shift key.  This is simplified by keeping both "shift"
 * codes next to each other in the macro trigger.
 *
 * Note that we do not have to do any "semantic" analysis of any of the
 * special keypresses, since the user can use a "user pref file" to map
 * the actual key (i.e. Shift+Keypad-6) to any series of Angband keys
 * (i.e. "\" + "." + "6" for "run to the east").  Hopefully, the default
 * "pref-ibm.prf" file will include macros for all of the "standard" keys,
 * including Control, Left Shift, and Right Shift, plus the Keypad keys.
 *
 * We use the "getxkey()" function to handle most of the keypress processing.
 * It waits for the user to press one key, then returns that key, in a special
 * encoded manner, where "alt-key" combinations have 0x100 added to them, and
 * "extended" keys have 0x200 added to them.  Thus, we only use the low "byte".
 *
 * XXX XXX XXX Verify the two "USE_WAT" blocks in the function below, and
 * note that the "getxkey()" function may be a "non-trivial" function.
 */
static errr Term_xtra_ibm_event(int v)
{
    int i, k;

#ifdef USE_WAT
    /* Hack -- Check for a keypress */
    if (!v && !bioskey(1)) return (1);
#else
    /* Hack -- Check for a keypress */
    if (!v && !kbhit()) return (1);
#endif

#ifdef USE_WAT
    /* Wait for a keypress */
    k = bioskey(0);
#else
    /* Wait for a keypress */
    k = getxkey();
#endif

    /* Normal keys */
    if (!(k & 0xFF00)) {

        /* Must be a 'normal' key */
        if (k) Term_keypress(k);
    }

    /* Handle "special" keys */
    else {

        /* Access the "modifiers" */
        i = bioskey(2);

        /* Hack -- begin a "macro trigger" */
        Term_keypress(31);

        /* Hack -- Send the modifiers */
        if (i & K_CTRL) Term_keypress('C');
        if (i & K_LSHIFT) Term_keypress('S');
        if (i & K_RSHIFT) Term_keypress('S');
        if (i & K_ALT) Term_keypress('O');

        /* Hack -- encode the "base" keypress (decimal) */
        Term_keypress('0' + (((k & 0xFF) / 100) % 10));
        Term_keypress('0' + (((k & 0xFF) / 10) % 10));
        Term_keypress('0' + (((k & 0xFF) / 1) % 10));

        /* Hack -- end the "macro trigger" */
        Term_keypress(13);
    }

    /* Success */
    return (0);
}


/*
 * Handle a "special request"
 *
 * The given parameters are "valid".
 *
 * Lastly, we may be able to speed up the I/O by providing another
 * "Term_xtra()" action which says that a certain line should be
 * flushed.  The question is which is faster, dumping a full 80
 * character line once, or dumping three or four much smaller
 * pieces of that line as they arrive.  Probably not worth it...
 */
static errr Term_xtra_ibm(int n, int v)
{
    /* Analyze the request */
    switch (n) {
       
#ifdef USE_CURS_SET

        /* Make the cursor visible */
        case TERM_XTRA_BEVIS:
            curs_set(1);
       	    return (0);

        /* Make the cursor invisible */
        case TERM_XTRA_INVIS:
            curs_set(0);
       	    return (0);

#endif

        /* Make a "bell" noise */
        case TERM_XTRA_NOISE:

            /* Make a bell noise */
            (void)write(1, "\007", 1);

            /* Success */
            return (0);

        /* Process events */
        case TERM_XTRA_EVENT:
        
            /* Process one event */
            return (Term_xtra_ibm_event(v));

        /* Clear the screen */
        case TERM_XTRA_CLEAR:
            ScreenClear();
            return (0);

#ifdef USE_VIRTUAL

        /* Flush one line of output */
        case TERM_XTRA_FROSH:

            /* Apply the virtual screen to the real screen */
            ScreenUpdateLine(VirtualScreen + ((cols*v) << 1), v);

            /* Success */
            return (0);

#endif

        /* Flush events */
        case TERM_XTRA_FLUSH:
            while (!Term_xtra_ibm_event(FALSE));
            return (0);
    }

    /* Unknown request */
    return (1);
}



/*
 * Init a Term
 */
static void Term_init_ibm(term *t)
{
    /* XXX Nothing */
}


/*
 * Nuke a Term
 */
static void Term_nuke_ibm(term *t)
{
    /* Move the cursor to the bottom of the screen */
    ScreenSetCursor(rows-1, 0);

#ifdef USE_CURS_SET

    /* Make the cursor visible */
    curs_set(1);

#endif

}


/*
 * Initialize the IBM "visual module"
 *
 * Hack -- we assume that "blank space" should be "white space"
 * (and not "black space" which might make more sense).
 */
errr init_ibm(void)
{
    int i;

    union REGS r;
 
    term *t = &term_screen_body;

    short attr = (short)(ibm_color[TERM_WHITE]) << 8;
    short blank = attr | (short)(' ');


    /* Acquire the size of the screen */
    rows = ScreenRows();
    cols = ScreenCols();

    /* Paranoia -- require minimum size */
    if ((rows < 24) || (cols < 80)) quit("Screen too small!");
    

    /* Build a "wiper line" of blank spaces */
    for (i = 0; i < 256; i++) wiper[i] = blank;

#ifdef USE_VIRTUAL

    /* Make the virtual screen */
    C_MAKE(VirtualScreen, rows * cols * 2, byte);

    /* Clear the virtual screen */
    for (i = 0; i < rows; i++) {

        /* Wipe that row */
        memcpy(VirtualScreen + ((i*cols) << 1), wiper, (cols << 1));
    }

#endif

    /* Erase the physical screen */
    Term_xtra(TERM_XTRA_CLEAR);


#ifdef USE_CURS_SET

    /* Access the "default" cursor info */   
    r.h.ah = 3;
    r.h.bh = 0;

    /* Make the call */
    int86(0x10, &r, &r);
    
    /* Extract the standard cursor info */
    saved_cur_v = 1;
    saved_cur_high = r.h.ch;
    saved_cur_low = r.h.cl;

#endif


    /* Initialize the term */
    term_init(t, 80, 24, 256);

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


    /* Success */
    return 0;
}


#endif /* USE_IBM */


