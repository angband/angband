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
 * Use "Makefile.wat" to compile this file with Watcom C/C++, and
 * be sure to define "USE_IBM" and "USE_WAT".
 *
 * DOS-286 (conio.h) changes by (Roland Jay Roberts (jay@map.com)
 * Use "Makefile.286" (not ready) to compile this file for DOS-286,
 * and be sure to define "USE_IBM", "USE_WAT", and "USE_286".
 *
 * Note that, for macro triggers, the left and right shift keys
 * are identical, but using both simultaneously can be detected,
 * though I would imagine this information has little application.
 *
 * This file, and "pref-ibm.prf", should work with Angband 2.7.9v3.
 *
 * Note the extensive changes to the "keypress" code, and note that
 * you must use the new "pref-ibm.prf" file to use the keypad, etc.
 *
 * Note the use of the "Term_user_ibm()" function hook, which allows
 * the user to interact with the "main-ibm.c" visual system.  Currently
 * this only allows choosing a "color table", from a set of predefined
 * tables, but other possibilities should be obvious, such as allowing
 * the specification of color table entries directly, or selecting the
 * number of screen rows (with support for the mirror window).
 */


#include "angband.h"


#ifdef USE_IBM


/*
 * Use a "virtual" screen to "buffer" screen writes
 */
#define USE_VIRTUAL


/*
 * Use "curs_set()" calls, which use "int86()" calls
 */
#define USE_CURS_SET


/*
 * Use the "bioskey(1)" call, not the "kbhit()" call
 * The "kbhit()" call does not work with Watcom.
 */
/* #define USE_BIOSKEY_1 */


/*
 * Use the "bioskey(0)" call, not the "bioskey(0x10)" call
 * The "bioskey(0x10)" call is better but only works on machines
 * made after "6/1/86".
 */
/* #define USE_BIOSKEY_0 */


/*
 * XXX XXX XXX Hack -- Support for DOS-286
 */
#ifdef USE_286
# define USE_CONIO
# define USE_BIOSKEY_0
# undef USE_CURS_SET
#endif


/*
 * XXX XXX XXX Hack -- Support for "conio.h"
 */
#ifdef USE_CONIO
# undef USE_VIRTUAL
#endif


#ifdef USE_WAT

# define USE_BIOSKEY_1

# include <bios.h>
# include <dos.h>

# ifndef USE_CONIO
#  include <graph.h>
# endif

# define int86(a,b,c) int386(a,b,c)

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


#ifndef USE_CONIO

/*
 * This array is used for "wiping" the screen
 */
static byte wiper[160];

#endif


/*
 * The main screen (currently the only screen)
 */
static term term_screen_body;



/*
 * The Angband color table (see below)
 */
static byte ibm_color[16];


/*
 * Hack -- maximum color sets defined
 */
#define COLOR_SETS 4


/*
 * Hack -- the default color sets	-BEN-
 *
 * The IBM does not directly support the Angband color set, which is
 * described in "defines.h", and which includes multiple shades of
 * "gray" and "umber", which are not really available on the IBM.
 *
 * Therefore, there is not an "obvious" color table, and we must try
 * to provide a "good" match.  There are several possibilities, and
 * for simplicity, we simply provide a table full of them, and let
 * the user pick one (if he does not like the default table) using
 * the new "Term_user()" interface.
 *
 * The default set uses "obvious" colors for the most part, but uses "white"
 * (not "bright white") for "White", "light magenta" for "Light Red" (not
 * "bright red"), and "light red" (not "yellow") for "Orange", and also
 * uses "cyan" and "bright cyan" for the "Slate" and "Light Slate" colors,
 * and "brown/yellow" for both "Umber" and "Light Umber".
 *
 * Some people would argue that "Light Slate" should be "white" and "White"
 * should be "bright white", but others have complained that "bright white"
 * is too bright.  Also, many people feel that both "bright black" and "blue"
 * are too dark on most IBM terminals.
 *
 * Some of these concerns are addressed by some of the "alternative" color
 * sets.  A "XXX" comment indicates a debatable match or an overused color.
 */
static const byte ibm_color_set[COLOR_SETS][16] = {

  /* Default Set -- optimal colors (?) */
  { 
    VID_BLACK,			/* Dark */
    VID_WHITE,			/* White */
    VID_CYAN,			/* Slate XXX */
    VID_RED | VID_BRIGHT,	/* Orange XXX */
    VID_RED,			/* Red */
    VID_GREEN,			/* Green */
    VID_BLUE,			/* Blue */
    VID_YELLOW,			/* Umber XXX */
    VID_BLACK | VID_BRIGHT,	/* Light Dark */
    VID_CYAN | VID_BRIGHT,	/* Light Slate XXX */
    VID_MAGENTA,		/* Violet */
    VID_YELLOW | VID_BRIGHT,	/* Yellow */
    VID_MAGENTA | VID_BRIGHT,	/* Light Red XXX */
    VID_GREEN | VID_BRIGHT,	/* Light Green */
    VID_BLUE | VID_BRIGHT,	/* Light Blue */
    VID_YELLOW			/* Light Umber XXX */
  },

  /* Alternative Set -- intuitive colors (?) */
  { 
    VID_BLACK,			/* Dark */
    VID_WHITE,			/* White */
    VID_CYAN,			/* Slate XXX */
    VID_YELLOW,			/* Orange XXX */
    VID_RED,			/* Red */
    VID_GREEN,			/* Green */
    VID_BLUE,			/* Blue */
    VID_YELLOW,			/* Umber XXX */
    VID_BLACK | VID_BRIGHT,	/* Light Dark */
    VID_CYAN | VID_BRIGHT,	/* Light Slate XXX */
    VID_MAGENTA,		/* Violet */
    VID_YELLOW | VID_BRIGHT,	/* Yellow */
    VID_RED | VID_BRIGHT,	/* Light Red XXX */
    VID_GREEN | VID_BRIGHT,	/* Light Green */
    VID_BLUE | VID_BRIGHT,	/* Light Blue */
    VID_YELLOW			/* Light Umber XXX */
  },

  /* Alternative Set -- no dark colors */
  { 
    VID_BLACK,			/* Dark */
    VID_WHITE,			/* White */
    VID_CYAN | VID_BRIGHT,	/* Slate XXX */
    VID_RED | VID_BRIGHT,	/* Orange XXX */
    VID_RED,			/* Red */
    VID_GREEN,			/* Green */
    VID_BLUE | VID_BRIGHT,	/* Blue XXX */
    VID_YELLOW,			/* Umber XXX */
    VID_CYAN,			/* Light Dark XXX */
    VID_CYAN | VID_BRIGHT,	/* Light Slate XXX */
    VID_MAGENTA,		/* Violet */
    VID_YELLOW | VID_BRIGHT,	/* Yellow */
    VID_MAGENTA | VID_BRIGHT,	/* Light Red XXX */
    VID_GREEN | VID_BRIGHT,	/* Light Green */
    VID_BLUE | VID_BRIGHT,	/* Light Blue */
    VID_YELLOW			/* Light Umber XXX */
  },

  /* Alternative Set -- Angband 2.7.8 colors */
  { 
    VID_BLACK,			/* Dark */
    VID_WHITE,			/* White */
    VID_WHITE,			/* Slate XXX */
    VID_YELLOW | VID_BRIGHT,	/* Orange XXX */
    VID_RED,			/* Red */
    VID_GREEN,			/* Green */
    VID_BLUE,			/* Blue */
    VID_YELLOW,			/* Umber XXX */
    VID_BLACK | VID_BRIGHT,	/* Light Dark XXX */
    VID_CYAN | VID_BRIGHT,	/* Light Slate */
    VID_MAGENTA,		/* Violet */
    VID_YELLOW | VID_BRIGHT,	/* Yellow */
    VID_RED | VID_BRIGHT,	/* Light Red XXX */
    VID_GREEN | VID_BRIGHT,	/* Light Green */
    VID_BLUE | VID_BRIGHT,	/* Light Blue */
    VID_YELLOW			/* Light Umber XXX */
  }
};



/*
 * Hack -- the color names
 */
static cptr color_names[16] = {
    "Dark",
    "White",
    "Slate",
    "Orange",
    "Red",
    "Green",
    "Blue",
    "Umber",
    "Light Dark",
    "Light Slate",
    "Violet",
    "Yellow",
    "Light Red",
    "Light Green",
    "Light Blue",
    "Light Umber"
};

/*
 * Hack -- select a color set
 */
static errr Term_user_ibm_color(void)
{
    int i, k;

    /* Interact */
    while (1) {

        /* Clear the screen */
        Term_clear();
    
        /* Title the screen */
        Term_putstr(0, 0, -1, TERM_WHITE, "Pick a color set");

        /* Display the color sets */
        for (i = 0; i < COLOR_SETS; i++) {

            Term_putstr(5, 5+i, -1, TERM_WHITE, format("(%d)", i));
            Term_putstr(10, 5+i, -1, TERM_WHITE, "....................");
            Term_putstr(10, 5+i, -1, TERM_WHITE, color_names[i]);    
            Term_putstr(30, 5+i, -1, i, "###**XXXOO+|8|+OOXXX**###");
        }
    
        /* Ask the question */
        Term_putstr(5, 8+COLOR_SETS, -1, TERM_WHITE, "Command: ");
        
        /* Get a choice */
        i = inkey();
        
        /* Escape */
        if (i == ESCAPE) break;

        /* Convert */
        k = (i - '0');

        /* Error */
        if ((k < 0) || (k >= COLOR_SETS)) continue;

        /* Activate */
        for (i = 0; i < 16; i++) {
        
            /* Use that color set */
            ibm_color[i] = ibm_color_set[k][i];
        }
    }

    /* Success */
    return (0);
}


/*
 * Hack -- interact with the user
 */
static errr Term_user_ibm(int n)
{
    int i;

    /* Save the screen */
    Term_save();

    /* Interact */
    while (1) {

        /* Clear the screen */
        Term_clear();
    
        /* Title the screen */
        Term_putstr(0, 0, -1, TERM_WHITE, "Interact with 'main-ibm.c'");

        /* Selections */
        Term_putstr(5, 5, -1, TERM_WHITE, "(1) Choose a color set");
    
        /* Prompt */
        Term_putstr(5, 10, -1, TERM_WHITE, "Command: ");
    
        /* Get a keypress */
        i = inkey();

        /* Escape */
        if (i == ESCAPE) break;

        /* Choose a color set */
        if (i == '1') {
            Term_user_ibm_color();
        }
    
        /* Oops */
        else {
            bell();
        }    
    }

    /* Restore the screen */
    Term_load();

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
            r.h.ch = 0x20;
            r.h.cl = 0x00;
        }

        /* Make the call */
        int86(0x10, &r, &r);
        
        /* Save the cursor state */
        saved_cur_v = v;
    }
}

#endif



/*
 * Hack -- convert a number (0 to 15) to a uppercase hecidecimal digit
 */
static char hexsym[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


#ifdef USE_WAT

# ifndef USE_CONIO

/*
 * Hack -- Use with Watcom C/C++
 */
#define bioskey(C) \
	_bios_keybrd(C)

# endif /* USE_CONIO */

#endif /* USE_WAT */


/*
 * Process an event (check for a keypress)
 *
 * The keypress processing code is often the most system dependant part
 * of Angband, since sometimes even the choice of compiler is important.
 *
 * For the IBM, we divide all keypresses into two catagories, first, the
 * "normal" keys, including all keys required to play Angband, and second,
 * the "special" keys, such as keypad keys, function keys, and various keys
 * used in combination with various modifier keys.
 *
 * To simplify this file, we use Angband's "macro processing" ability, in
 * combination with a specialized "pref-ibm.prf" file, to handle most of the
 * "special" keys, instead of attempting to fully analyze them here.  This
 * file only has to determine when a "special" key has been pressed, and
 * translate it into a simple string which signals the use of a "special"
 * key, the set of modifiers used, if any, and the hardware scan code of
 * the actual key which was pressed.  To simplify life for the user, we
 * treat both "shift" keys as identical modifiers.
 *
 * The final encoding is "^_MMMxSS\r", where "MMM" encodes the modifiers
 * ("C" for control, "S" for shift, "A" for alt, or any ordered combination),
 * and "SS" encodes the keypress (as the two "digit" hexidecimal encoding of
 * the scan code of the key that was pressed), and the "^_" and "x" and "\r"
 * delimit the encoding for recognition by the macro processing code.
 *
 * Some important facts about scan codes follow.  All "normal" keys use
 * scan codes from 1-58.  The "function" keys use 59-68 (and 133-134).
 * The "keypad" keys use 69-83.  Escape uses 1.  Enter uses 28.  Control
 * uses 29.  Left Shift uses 42.  Right Shift uses 54.  PrtScrn uses 55.
 * Alt uses 56.  Space uses 57.  CapsLock uses 58.  NumLock uses 69.
 * ScrollLock uses 70.  The "keypad" keys which use scan codes 71-83
 * are ordered KP7,KP8,KP9,KP-,KP4,KP5,KP6,KP+,KP1,KP2,KP3,INS,DEL.
 *
 * Using "bioskey(0x10)" instead of "bioskey(0)" apparently provides more
 * information, including better access to the keypad keys in combination
 * with various modifiers, but only works on "PC's after 6/1/86", and there
 * is no way to determine if the function is provided on a machine.  I have
 * been told that without it you cannot detect, for example, control-left.
 * The basic scan code + ascii value pairs returned by the keypad follow,
 * with values in parentheses only available to "bioskey(0x10)".
 *
 *         /      *      -      +      1      2      3      4
 * Norm:  352f   372a   4a2d   4e2b   4f00   5000   5100   4b00
 * Shft:  352f   372a   4a2d   4e2b   4f31   5032   5133   4b34
 * Ctrl: (9500) (9600) (8e00) (9000)  7500  (9100)  7600   7300
 * 
 *         5      6      7      8      9      0      .     Enter
 * Norm: (4c00)  4d00   4700   4800   4900   5200   5300  (e00d)
 * Shft:  4c35   4d36   4737   4838   4939   5230   532e  (e00d)
 * Ctrl: (8f00)  7400   7700  (8d00)  8400  (9200) (9300) (e00a)
 *
 * See "pref-ibm.prf" for the "standard" macros for various keys.
 */
static errr Term_xtra_ibm_event(int v)
{
    int i, k, s;

    bool mc = FALSE;
    bool ms = FALSE;
    bool ma = FALSE;


#ifdef USE_BIOSKEY_1
    /* Hack -- Check for a keypress */
    if (!v && !bioskey(1)) return (1);
#else
    /* Hack -- Check for a keypress */
    if (!v && !kbhit()) return (1);
#endif

#ifdef USE_BIOSKEY_0
    /* Wait for a keypress */
    k = bioskey(0);
#else
    /* Wait for a keypress */
    k = bioskey(0x10);
#endif

    /* Access the "modifiers" */
    i = bioskey(2);

    /* Extract the "scan code" */
    s = ((k >> 8) & 0xFF);

    /* Extract the "ascii value" */
    k = (k & 0xFF);

    /* Extract the modifier flags */
    if (i & (1 << K_CTRL)) mc = TRUE;
    if (i & (1 << K_LSHIFT)) ms = TRUE;
    if (i & (1 << K_RSHIFT)) ms = TRUE;
    if (i & (1 << K_ALT)) ma = TRUE;


    /* Process "normal" keys */
    if ((s <= 58) && !ma) {

        /* Enqueue it */
        if (k) Term_keypress(k);

        /* Success */
        return (0);
    }


    /* Begin a "macro trigger" */
    Term_keypress(31);

    /* Hack -- Send the modifiers */
    if (mc) Term_keypress('C');
    if (ms) Term_keypress('S');
    if (ma) Term_keypress('A');

    /* Introduce the hexidecimal scan code */
    Term_keypress('x');
        
    /* Encode the hexidecimal scan code */
    Term_keypress(hexsym[s/16]);
    Term_keypress(hexsym[s%16]);

    /* End the "macro trigger" */
    Term_keypress(13);

    /* Success */
    return (0);
}


/*
 * Handle a "special request"
 *
 * The given parameters are "valid".
 */
static errr Term_xtra_ibm(int n, int v)
{
    int i;

    /* Analyze the request */
    switch (n) {
       
        /* Make a "bell" noise */
        case TERM_XTRA_NOISE:

            /* Make a bell noise */
            (void)write(1, "\007", 1);

            /* Success */
            return (0);

#ifdef USE_CURS_SET

        /* Make the cursor visible */
        case TERM_XTRA_BEVIS:
        
            /* Set cursor visible */
            curs_set(1);

            /* Success */
       	    return (0);

        /* Make the cursor invisible */
        case TERM_XTRA_INVIS:

            /* Set cursor invisible */
            curs_set(0);

            /* Success */
       	    return (0);

#endif

#ifdef USE_VIRTUAL

        /* Flush one line of output */
        case TERM_XTRA_FROSH:

# ifdef USE_WAT

            /* Copy the virtual screen to the physical screen */
            memcpy(PhysicalScreen + (v*160), VirtualScreen + (v*160), 160);

# else /* USE_WAT */

            /* Apply the virtual screen to the physical screen */
            ScreenUpdateLine(VirtualScreen + ((v*cols) << 1), v);

# endif /* USE_WAT */

            /* Success */
            return (0);

#endif /* USE_VIRTUAL */

        /* Clear the screen */
        case TERM_XTRA_CLEAR:

#ifdef USE_CONIO

            /* Clear the screen */
            clrscr();

#else /* USE_CONIO */

            /* Clear each line (virtual or physical) */
            for (i = 0; i < rows; i++) {

                /* Clear the line */
                memcpy((VirtualScreen + ((i*cols) << 1)), wiper, (cols << 1));
            }

# ifdef USE_VIRTUAL

#  ifdef USE_WAT

            /* Copy the virtual screen to the physical screen */
            memcpy(PhysicalScreen, VirtualScreen, 25*80*2);

#  else /* USE_WAT */

            /* Erase the physical screen */
            ScreenClear();

#  endif /* USE_WAT */

# endif /* USE_VIRTUAL */

#endif /* USE_CONIO */

            /* Success */
            return (0);

        /* Process events */
        case TERM_XTRA_EVENT:
        
            /* Process one event */
            return (Term_xtra_ibm_event(v));

        /* Flush events */
        case TERM_XTRA_FLUSH:

            /* Strip events */
            while (!Term_xtra_ibm_event(FALSE));

            /* Success */
            return (0);
    }

    /* Unknown request */
    return (1);
}



/*
 * Move the cursor
 *
 * The given parameters are "valid".
 */
static errr Term_curs_ibm(int x, int y)
{

#ifdef USE_WAT

# ifdef USE_CONIO

    /* Place the cursor */
    gotoxy(x+1, y+1);

# else /* USE_CONIO */

    union REGS r;

    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dl = x;
    r.h.dh = y;

    /* Place the cursor */
    int386(0x10, &r, &r);

# endif /* USE_CONIO */

#else /* USE_WAT */

    /* Move the cursor */
    ScreenSetCursor(y,x);

#endif /* USE_WAT */

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

#else /* USE_CONIO */

    /* Wipe part of the virtual (or physical) screen */
    memcpy(VirtualScreen + ((cols*y + x)<<1), wiper, n<<1);

#endif /* USE_CONIO */

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

#else /* USE_CONIO */

    /* Access the virtual (or physical) screen */
    dest = VirtualScreen + (((cols * y) + x) << 1);

    /* Build the buffer */
    while (*s) {
        *dest++ = *s++;
        *dest++ = attr;
    }

#endif /* USE_CONIO */

    /* Success */
    return (0);
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
    Term_curs_ibm(0, rows-1);

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

    term *t = &term_screen_body;

#ifdef USE_CURS_SET

    union REGS r;

#endif
 

#ifdef USE_WAT

#ifndef USE_CONIO
    /* Force 25 line mode */
    _settextrows(25);
#endif

    /* Assume the size of the screen */
    rows = 25;
    cols = 80;

#else /* USE_WAT */

    /* Acquire the size of the screen */
    rows = ScreenRows();
    cols = ScreenCols();

    /* Paranoia -- require minimum size */
    if ((rows < 24) || (cols < 80)) quit("Screen too small!");
    
#endif


    /* Initialize the colors */
    for (i = 0; i < 16; i++) {
    
        /* Use the "default" color set */
        ibm_color[i] = ibm_color_set[0][i];
    }


#ifndef USE_CONIO

    /* Build a "wiper line" */
    for (i = 0; i < 80; i++) {

        /* Space */
        wiper[2*i] = ' ';

        /* White */
        wiper[2*i+1] = ibm_color[TERM_WHITE];
    }

#endif


#ifdef USE_VIRTUAL

    /* Make the virtual screen */
    C_MAKE(VirtualScreen, rows * cols * 2, byte);

#endif


    /* Erase the screen */
    Term_xtra_ibm(TERM_XTRA_CLEAR, 0);


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
    t->user_hook = Term_user_ibm;
    t->xtra_hook = Term_xtra_ibm;
    t->curs_hook = Term_curs_ibm;
    t->wipe_hook = Term_wipe_ibm;
    t->text_hook = Term_text_ibm;

    /* Save it */
    term_screen = t;

    /* Activate it */
    Term_activate(term_screen);


    /* Success */
    return 0;
}


#endif /* USE_IBM */


