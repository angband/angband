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
 * True color palette support by "Mike Marcelais (mrmarcel@eos.ncsu.edu)",
 * with interface to the "color_table" array by Ben Harrison.
 *
 * Both "shift" keys are treated as "identical", and all the modifier keys
 * (control, shift, alt) are ignored when used with "normal" keys, unless
 * they modify the underlying "ascii" value of the key.  You must use the
 * new "user pref files" to be able to interact with the keypad and such.
 *
 * The "lib/user/pref-ibm.prf" file contains macro definitions and possible
 * alternative color set definitions.  The "lib/user/font-ibm.prf" contains
 * attr/char mappings for walls and floors and such.
 *
 * Note the "Term_user_ibm()" function hook, which could allow the user
 * to interact with the "main-ibm.c" visual system.  Currently this hook
 * is unused, but, for example, it could allow the user to toggle "sound"
 * or "graphics" modes, or to select the number of screen rows, with the
 * extra screen rows being used for the mirror window.
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
 * XXX XXX XXX Hack -- Support for DOS-286
 */
#ifdef USE_286
# define USE_CONIO
# undef USE_CURS_SET
#endif


/*
 * XXX XXX XXX Hack -- Support for "conio.h"
 */
#ifdef USE_CONIO
# undef USE_VIRTUAL
#endif



#ifdef USE_WAT

# include <bios.h>
# include <dos.h>
# include <conio.h>

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
# include <graphics.h>

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
 * Foreground color bits (hard-coded by DOS)
 */
#define VID_BLACK	0x00
#define VID_BLUE	0x01
#define VID_GREEN	0x02
#define VID_CYAN	0x03
#define VID_RED		0x04
#define VID_MAGENTA	0x05
#define VID_YELLOW	0x06
#define VID_WHITE	0x07

/*
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT	0x08

/*
 * Background color bits (hard-coded by DOS)
 */
#define VUD_BLACK	0x00
#define VUD_BLUE	0x10
#define VUD_GREEN	0x20
#define VUD_CYAN	0x30
#define VUD_RED		0x40
#define VUD_MAGENTA	0x50
#define VUD_YELLOW	0x60
#define VUD_WHITE	0x70

/*
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BLINK	0x80


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
 * Choose between the "complex" and "simple" color methods
 */
static byte use_color_complex = FALSE;


/*
 * The "complex" color set
 *
 * This table is used by the "palette" code to instantiate the proper
 * Angband colors when using hardware capable of displaying "complex"
 * colors, see "activate_complex_color" below.
 *
 * The values used below are taken from the values in "main-mac.c",
 * gamma corrected based on the tables in "io.c", and then "tweaked"
 * until they "looked good" on the monitor of "Mike Marcelais".
 *
 * The entries are given as "0xBBGGRR" where each of the "blue", "green",
 * and "red" components range from "0x00" to "0x3F".  Note that this is
 * the opposite of many systems which give the values as "0xRRGGBB", and
 * is the opposite of the "R,G,B" codes in the comments.
 */
static long ibm_color_complex[16] = {

    0x000000L,		/* 0 0 0  Dark       */
    0x3f3f3fL,		/* 4 4 4  White      */
    0x232323L,		/* 2 2 2  Slate      */
    0x00233fL,		/* 4 2 0  Orange     */
    0x000023L,		/* 2 0 0  Red        */
    0x112300L,		/* 0 2 1  Green      */
    0x3f0000L,		/* 0 0 4  Blue       */
    0x001123L,		/* 2 1 0  Umber      */
    0x111111L,		/* 1 1 1  Lt. Dark   */
    0x353535L,		/* 3 3 3  Lt. Slate  */
    0x230023L,		/* 2 0 2  Purple     */
    0x003f3fL,		/* 4 4 0  Yellow     */
    0x35113fL,		/* 4 1 3  Lt. Red    */
    0x003f00L,		/* 0 4 0  Lt. Green  */
    0x3f3f00L,		/* 0 4 4  Lt. Blue   */
    0x112335L		/* 3 2 1  Lt. Umber  */
};


/*
 * The "simple" color set
 *
 * This table is used by the "color" code to instantiate the "approximate"
 * Angband colors using the only colors available on crappy monitors.
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that values from 16 to 255 are extremely ugly.
 *
 * The values below came from various sources, if you do not like them,
 * get a better monitor, or edit "pref-ibm.prf" to use different codes.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static byte ibm_color_simple[16] = {

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
};



/*
 * Activate the "ibm_color_complex" palette information.
 *
 * On Watcom machines, we can simply use the special "_remapallpalette()"
 * function, which not only sets both palette lists (see below) but also
 * checks for legality of the monitor mode.
 *
 * WARNING:   -- Mike Marcelais
 *
 *   On VGA cards, colors go through a double-indirection when looking
 *   up the `real' color when in 16 color mode.  The color value in the
 *   attribute is looked up in the EGA color registers.  Then that value
 *   is looked up in the VGA color registers.  Then the color is displayed.
 *   This is done for compatability.  However, the EGA registers are
 *   initialized by default to 0..5, 14, 7, 38..3F and not 0..F which means
 *   that unless these are reset, the VGA setpalette function will not
 *   update the correct palette register!
 *
 *   DJGPP's GrSetColor() does _not_ set the EGA palette list, only the
 *      VGA color list.
 *
 *   Source: The programmer's guide to the EGA and VGA video cards.  [Farraro]
 *
 *   Note that the "traditional" method, using "int86(0x10)", is very slow
 *   when called in protected mode, so we use a faster method using video
 *   ports instead.
 */
static bool activate_color_complex(void)
{

#ifdef USE_WAT

    /* Use a special Watcom function */
    return (_remapallpalette(ibm_color_complex));

#else /* USE_WAT */

    int i;

# if 1

    /* Edit the EGA palette */
    inportb(0x3da);

    /* Edit the colors */
    for (i = 0; i < 16; i++) {

        /* Set color "i" */
        outportb(0x3c0, i);

        /* To value "i" */
        outportb(0x3c0, i);
    };

    /* Use that EGA palette */
    outportb(0x3c0, 0x20);

    /* Edit VGA palette, starting at color zero */
    outportb(0x3c8, 0);

    /* Send the colors */
    for (i = 0; i < 16; i++) {

        /* Send the red, green, blue components */
        outportb(0x3c9, ((ibm_color_complex[i]) & 0xFF));
        outportb(0x3c9, ((ibm_color_complex[i] >> 8) & 0xFF));
        outportb(0x3c9, ((ibm_color_complex[i] >> 16) & 0xFF));
    };

# else /* 1 */

    /* Set the colors */
    for (i = 0; i < 16; i++) {

        union REGS r;

        /* Set EGA color */
        r.h.ah = 0x10;
        r.h.al = 0x00;

        /* Set color "i" */
        r.h.bl = i;

        /* To value "i" */
        r.h.bh = i;

        /* Do it */
        int86(0x10, &r, &r);

        /* Set VGA color */
        r.h.ah = 0x10;
        r.h.al = 0x10;

        /* Set color "i" */
        r.h.bh = 0x00;
        r.h.bl = i;

        /* Use this "green" value */
        r.h.ch = ((ibm_color_complex[i] >> 8) & 0xFF);
        
        /* Use this "blue" value */
        r.h.cl = ((ibm_color_complex[i] >> 16) & 0xFF);
        
        /* Use this "red" value */
        r.h.dh = ((ibm_color_complex[i]) & 0xFF);

        /* Do it */
        int86(0x10, &r, &r);
    };

# endif /* 1 */

#endif /* USE_WAT */

    /* Success */
    return (TRUE);
};


/*
 * React to changes in global variables
 *
 * Currently, this includes only changes in the desired color set
 *
 * Note the use of "(x >> 2)" to convert an 8 bit value to a 6 bit value
 * without losing much precision.
 */
static int Term_xtra_ibm_react(void)
{
    int i;

    /* Complex method */
    if (use_color_complex) {

        long rv, gv, bv, code;

        bool change = FALSE;

        /* Save the default colors */
        for (i = 0; i < 16; i++) {

            /* Extract desired values */
            rv = color_table[i][1] >> 2;
            gv = color_table[i][2] >> 2;
            bv = color_table[i][3] >> 2;

            /* Extract a full color code */
            code = ((rv) | (gv << 8) | (bv << 16));

            /* Activate changes */
            if (ibm_color_complex[i] != code) {

                /* Note the change */
                change = TRUE;

                /* Apply the desired color */
                ibm_color_complex[i] = code;
            }
        }

        /* Activate the palette if needed */
        if (change) activate_color_complex();
    }

    /* Simple method */
    else {

        /* Save the default colors */
        for (i = 0; i < 16; i++) {

            /* Simply accept the desired colors */
            ibm_color_simple[i] = color_table[i][0];
        }
    }
    
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
 *
 * Certain "bizarre" keypad keys (such as "enter") return a "scan code"
 * of "0xE0", and a "usable" ascii value.  These keys should be treated
 * like the normal keys, see below.  XXX XXX XXX Note that these "special"
 * keys could be prefixed with an optional "ctrl-^" which would allow them
 * to be used in macros without hurting their use in normal situations.
 */
static errr Term_xtra_ibm_event(int v)
{
    int i, k, s;

    bool mc = FALSE;
    bool ms = FALSE;
    bool ma = FALSE;


    /* Hack -- Check for a keypress */
    if (!v && !bioskey(1)) return (1);

    /* Wait for a keypress */
    k = bioskey(0x10);

    /* Access the "modifiers" */
    i = bioskey(2);

    /* Extract the "scan code" */
    s = ((k >> 8) & 0xFF);

    /* Extract the "ascii value" */
    k = (k & 0xFF);

    /* Process "normal" keys */
    if ((s <= 58) || (s == 0xE0)) {

        /* Enqueue it */
        if (k) Term_keypress(k);

        /* Success */
        return (0);
    }

    /* Extract the modifier flags */
    if (i & (1 << K_CTRL)) mc = TRUE;
    if (i & (1 << K_LSHIFT)) ms = TRUE;
    if (i & (1 << K_RSHIFT)) ms = TRUE;
    if (i & (1 << K_ALT)) ma = TRUE;


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

        /* Set cursor visibility */
        case TERM_XTRA_SHAPE:
        
            /* Set cursor visibility */
            curs_set(v);

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

        /* React to global changes */
        case TERM_XTRA_REACT:
        
            /* React to "color_table" changes */
            return (Term_xtra_ibm_react());
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


    /* Handle "complex" color */
    if (use_color_complex) {

        /* Extract a color index */
        attr = (a & 0x0F);
    }

    /* Handle "simple" color */
    else {

        /* Extract a color value */
        attr = ibm_color_simple[a & 0x0F];
    }

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

    /* Save the data */
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
    union REGS r;

    /* Move the cursor to the bottom of the screen */
    Term_curs_ibm(0, rows-1);

#ifdef USE_CURS_SET

    /* Make the cursor visible */
    curs_set(1);

    /* Restore the original video mode */
#ifdef USE_WAT
    _setvideomode( _DEFAULTMODE );
#else
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);
#endif

#endif

}


/*
 * Initialize the IBM "visual module"
 *
 * Hack -- we assume that "blank space" should be "white space"
 * (and not "black space" which might make more sense).
 *
 * Note the use of "((x << 2) | (x >> 4))" to "expand" a 6 bit value
 * into an 8 bit value, without losing much precision, by using the 2
 * most significant bits as the least significant bits in the new value.
 */
errr init_ibm(void)
{
    int i;
    int mode;

    term *t = &term_screen_body;

    union REGS r;

#ifdef USE_WAT

# ifndef USE_CONIO
    /* Force 25 line mode */
    _settextrows(25);
# endif

    /* Assume the size of the screen */
    rows = 25;
    cols = 80;

    /* Instantiate the color set, check for success */
    if (activate_color_complex()) use_color_complex = TRUE;

#else /* USE_WAT */

    /* Get video mode */
    r.h.ah = 0x00;
    r.h.al = 0x13;
    int86(0x10, &r, &r);
    mode = ScreenMode();
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    /* Acquire the size of the screen */
    rows = ScreenRows();
    cols = ScreenCols();

    /* Paranoia -- require minimum size */
    if ((rows < 24) || (cols < 80)) quit("Screen too small!");

    /* Check video mode */
    if (mode == 0x13) {

        /* Instantiate the color set, check for success */
        if (activate_color_complex()) use_color_complex = TRUE;
    }

#endif /* USE_WAT */


    /* Initialize "color_table" */
    for (i = 0; i < 16; i++) {

        int rv, gv, bv;

        /* Extract the "complex" codes */
        rv = ((ibm_color_complex[i]) & 0xFF);
        gv = ((ibm_color_complex[i] >> 8) & 0xFF);
        bv = ((ibm_color_complex[i] >> 16) & 0xFF);

        /* Save the "complex" codes */
        color_table[i][1] = ((rv << 2) | (rv >> 4));
        color_table[i][2] = ((gv << 2) | (gv >> 4));
        color_table[i][3] = ((bv << 2) | (bv >> 4));

        /* Save the "simple" code */
        color_table[i][0] = ibm_color_simple[i];
    }
    

#ifndef USE_CONIO

    /* Build a "wiper line" */
    for (i = 0; i < 80; i++) {

        /* Hack -- space */
        wiper[2*i] = ' ';

        /* Hack -- black */
        wiper[2*i+1] = 0;
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

    /* Erase with "black space" */
    t->attr_blank = TERM_DARK;
    t->char_blank = ' ';

    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_ibm;
    t->nuke_hook = Term_nuke_ibm;

    /* Connect the hooks */
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

