/* File: main-x11.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with UNIX/X11 computers.
 *
 * To use this file, compile with "USE_XPJ" defined, and link against all
 * the various "X11" libraries which may be needed.
 *
 * See also "main-x11.c" and "main-xaw.c".
 *
 * Part of this file provides a user interface package composed of several
 * pseudo-objects, including "metadpy" (a display), "infowin" (a window),
 * "infoclr" (a color), and "infofnt" (a font).  Actually, the package was
 * originally much more interesting, but it was bastardized to keep this
 * file simple.
 *
 * The rest of this file is an implementation of "main-xxx.c" for X11.
 *
 * Most of this file is by Ben Harrison (benh@phial.com).
 * The old main-x11.c has been changed to use 3d-projection by
 * Steven Fuerst.
 */



#include "angband.h"


#ifdef USE_XPJ

#include "main.h"

#ifndef USE_GRAPHICS
#error Must have USE_GRAPHICS compile-time flag on.
#endif

/*
 * The tile size to use.  (Configure this if you want.)
 *
 * This is 16 by default - but any multiple of 4 works.
 * Try setting this to be 20 or larger if you have a big screen.
 * Settings as small as 8 also work. (However, things are a bit
 * hard to see on that setting.) -SF-
 *
 * XXX XXX This probably could be converted to work with any
 * even number, with a few minor changes.
 */
#define P_TILE_SIZE	16



/*
 * Set this to be 1 if you want the walls to be bright.
 * Bright walls may or may not look better.
 */
#define BRIGHT_WALLS 0




/* Rest of the dependencies */

#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif /* __MAKEDEPEND__ */


/*
 * Include some helpful X11 code.
 */
#include "maid-x11.h"


/*
 * Notes on Colors:
 *
 *   1) On a monochrome (or "fake-monochrome") display, all colors
 *   will be "cast" to "fg," except for the bg color, which is,
 *   obviously, cast to "bg".  Thus, one can ignore this setting.
 *
 *   2) Because of the inner functioning of the color allocation
 *   routines, colors may be specified as (a) a typical color name,
 *   (b) a hexidecimal color specification (preceded by a pound sign),
 *   or (c) by strings such as "fg", "bg", "zg".
 *
 *   3) Due to the workings of the init routines, many colors
 *   may also be dealt with by their actual pixel values.  Note that
 *   the pixel with all bits set is "zg = (1<<metadpy->depth)-1", which
 *   is not necessarily either black or white.
 */



/**** Generic Types ****/


/*
 * An X11 pixell specifier
 */
typedef unsigned long Pixell;

/*
 * The structures defined below
 */
typedef struct metadpy metadpy;
typedef struct infowin infowin;
typedef struct infoclr infoclr;
typedef struct infofnt infofnt;


/*
 * A structure summarizing a given Display.
 *
 *	- The Display itself
 *	- The default Screen for the display
 *	- The virtual root (usually just the root)
 *	- The default colormap (from a macro)
 *
 *	- The "name" of the display
 *
 *	- The socket to listen to for events
 *
 *	- The width of the display screen (from a macro)
 *	- The height of the display screen (from a macro)
 *	- The bit depth of the display screen (from a macro)
 *
 *	- The black Pixell (from a macro)
 *	- The white Pixell (from a macro)
 *
 *	- The background Pixell (default: black)
 *	- The foreground Pixell (default: white)
 *	- The maximal Pixell (Equals: ((2 ^ depth)-1), is usually ugly)
 *
 *	- Bit Flag: Force all colors to black and white (default: !color)
 *	- Bit Flag: Allow the use of color (default: depth > 1)
 *	- Bit Flag: We created 'dpy', and so should nuke it when done.
 */
struct metadpy
{
	Display *dpy;
	Screen *screen;
	Window root;
	Colormap cmap;

	char *name;

	int fd;

	uint width;
	uint height;
	uint depth;

	Pixell black;
	Pixell white;

	Pixell bg;
	Pixell fg;
	Pixell zg;

	uint mono:1;
	uint color:1;
	uint nuke:1;
};



/*
 * A Structure summarizing Window Information.
 *
 * I assume that a window is at most 30000 pixels on a side.
 * I assume that the root windw is also at most 30000 square.
 *
 *	- The Window
 *	- The current Input Event Mask
 *
 *	- The location of the window
 *	- The width, height of the window
 *	- The border width of this window
 *
 *	- Byte: 1st Extra byte
 *
 *	- Bit Flag: This window is currently Mapped
 *	- Bit Flag: This window has been resized
 *
 *	- Bit Flag: We should nuke 'win' when done with it
 *
 *	- Bit Flag: 1st extra flag
 *	- Bit Flag: 2nd extra flag
 *	- Bit Flag: 3rd extra flag
 *	- Bit Flag: 4th extra flag
 *  - Bit Flag: 5th extra flag
 */
struct infowin
{
	Window win;
	long mask;

	s16b ox, oy;

	s16b x, y;
	s16b w, h;
	u16b b;

	byte byte1;

	uint mapped:1;
	uint resize:1;

	uint nuke:1;

	uint flag1:1;
	uint flag2:1;
	uint flag3:1;
	uint flag4:1;
	uint flag5:1;
};






/*
 * A Structure summarizing Operation+Color Information
 *
 *	- The actual GC corresponding to this info
 *
 *	- The Foreground Pixell Value
 *	- The Background Pixell Value
 *
 *	- Num (0-15): The operation code (As in Clear, Xor, etc)
 *	- Bit Flag: The GC is in stipple mode
 *	- Bit Flag: Destroy 'gc' at Nuke time.
 */
struct infoclr
{
	GC gc;

	Pixell fg;
	Pixell bg;

	uint code:4;
	uint stip:1;
	uint nuke:1;
};



/*
 * A Structure to Hold Font Information
 *
 *	- The 'XFontStruct*' (yields the 'Font')
 *
 *	- The font name
 *
 *	- The default character width
 *	- The default character height
 *	- The default character ascent
 *
 *	- Byte: Pixel offset used during fake mono
 *
 *	- Flag: Force monospacing via 'wid'
 *	- Flag: Nuke info when done
 */
struct infofnt
{
	XFontStruct *info;

	cptr name;

	s16b wid;
	s16b hgt;
	s16b asc;

	byte off;

	uint mono:1;
	uint nuke:1;
};



/**** Angband data structures ****/

/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * A structure for each "term"
 */
struct term_data
{
	term t;

	infofnt *fnt;

	infowin *win;

	XImage *tiles;

	/* Tempory storage for overlaying tiles. */
	XImage *TmpImage;

	/* Tempory storage for skewing tiles. */
	XImage *SkewImage;
};


/*
 * The number of term data structures
 */
#define MAX_TERM_DATA 8

/*
 * The array of term data structures
 */
static term_data data[MAX_TERM_DATA];

/* Tables used to rapidly calculate which pixel to plot */
static u16b pj_table1[P_TILE_SIZE][P_TILE_SIZE / 2];
static u16b pj_table2[P_TILE_SIZE][P_TILE_SIZE / 2];

/* Bitflags used in the tables */

/* Transparent diagonal walls */
#define PJ_T_WALL1_T	0x0001
#define PJ_T_WALL2_T	0x0002

/* Transparent Top */
#define PJ_T_TOP_T1		0x0004
#define PJ_T_TOP_T2		0x0008

/* Floor */
#define PJ_T_FLOOR1		0x0010
#define PJ_T_FLOOR2		0x0020

/* Diagonal walls "behind" everything */
#define PJ_T_WALL1		0x0040

/* Overlaying tiles */
#define PJ_T_OVER1		0x0080
#define PJ_T_OVER3		0x0100

/* Horizontal walls in front of overlays */
#define PJ_T_WALLF		0x0200
#define PJ_T_WALLB		0x0400

/* Diagonal walls in front of everything */
#define PJ_T_WALL2		0x0800

/* Overlaying tiles */
#define PJ_T_OVER2		0x1000
#define PJ_T_OVER4		0x2000

/* Ceiling */
#define PJ_T_TOP1		0x4000
#define PJ_T_TOP2		0x8000



/**** Bit numbers ****/

/* Transparent diagonal walls */
#define PJ_WALL1_T		0
#define PJ_WALL2_T		1

/* Transparent Top */
#define PJ_TOP_T1		2
#define PJ_TOP_T2		3

/* Floor */
#define PJ_FLOOR1		4
#define PJ_FLOOR2		5

/* Diagonal walls "behind" everything */
#define PJ_WALL1		6

/* Overlaying tiles */
#define PJ_OVER1		7
#define PJ_OVER3		8

/* Horizontal walls in front of overlays */
#define PJ_WALLF		9
#define PJ_WALLB		10

/* Diagonal walls in front of everything */
#define PJ_WALL2		11

/* Overlaying tiles */
#define PJ_OVER2		12
#define PJ_OVER4		13

/* Ceiling */
#define PJ_TOP1			14
#define PJ_TOP2			15

/* Number of bits used */
#define PJ_MAX			16

static const byte bit_high_lookup[256] =
{
	0, 1, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8
};

static const int t_offsetx1[PJ_MAX] =
{
	0,
	0,
	P_TILE_SIZE / 2,
	P_TILE_SIZE / 2,
	0,
	P_TILE_SIZE,
	0,
	-P_TILE_SIZE / 4,
	(3 * P_TILE_SIZE) / 4,
	P_TILE_SIZE / 2,
	P_TILE_SIZE / 2,
	0,
	P_TILE_SIZE / 4,
	0,
	P_TILE_SIZE / 2,
	P_TILE_SIZE / 2
};

static const int t_offsety1[PJ_MAX] =
{
	P_TILE_SIZE,
	P_TILE_SIZE,
	0,
	0,
	0,
	0,
	P_TILE_SIZE,
	P_TILE_SIZE / 2,
	P_TILE_SIZE / 2,
	0,
	0,
	P_TILE_SIZE,
	-P_TILE_SIZE / 2,
	0,
	0,
	0
};

static const int t_xscale[PJ_MAX] =
{
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-1
};

static const int t_offsetx2[PJ_MAX] =
{
	0,
	0,
	0,
	P_TILE_SIZE,
	P_TILE_SIZE / 2,
	0,
	0,
	P_TILE_SIZE / 4,
	0,
	0,
	0,
	0,
	(3 * P_TILE_SIZE) / 4,
	-P_TILE_SIZE / 4,
	0,
	P_TILE_SIZE
};

static const int t_offsety2[PJ_MAX] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	P_TILE_SIZE / 2,
	0,
	0,
	0,
	0,
	-P_TILE_SIZE / 2,
	-P_TILE_SIZE / 2,
	0,
	0
};

static const bool wall_flip[PJ_MAX] =
{
	1,
	1,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	0
};

/*
 * For optimisation purposes - two arrays of bits for a row,
 * marking whether or not this 8x16 block has changed.
 */
static u32b pj_row1[64];
static u32b pj_row2[64];
static int pj_cur_row;

/* Font data */
static XImage *font_data;

/* Number of bytes per pixel */
static int bytes_per_pixel;

/*
 * Function pointer that points to the function that
 * moves the correctly sized pixels to the overlay block.
 * This optimises away the need for the slow
 * getpixel and putpixel pairs.
 */
static void (*draw_block)(void *tiles[PJ_MAX],
	 u16b pj_table[P_TILE_SIZE][P_TILE_SIZE / 2], u16b mask,
	 term_data *td);

/*
 * Hack -- cursor color
 */
static infoclr *xor;

/*
 * Actual color table
 */
static infoclr *clr[256];

/*
 * Color info (unused, red, green, blue).
 */
static byte color_table[256][4];

/*
 * The "blank" pixel - used for transparency
 */
static Pixell pix_blank;

/**** Generic Macros ****/



/* Set current metadpy (Metadpy) to 'M' */
#define Metadpy_set(M) \
	Metadpy = M


/* Initialize 'M' using Display 'D' */
#define Metadpy_init_dpy(D) \
	Metadpy_init_2(D,cNULL)

/* Initialize 'M' using a Display named 'N' */
#define Metadpy_init_name(N) \
	Metadpy_init_2((Display*)(NULL),N)

/* Initialize 'M' using the standard Display */
#define Metadpy_init() \
	Metadpy_init_name("")


/* Init an infowin by giving father as an (info_win*) (or NULL), and data */
#define Infowin_init_dad(D,X,Y,W,H,B,FG,BG) \
	Infowin_init_data(((D) ? ((D)->win) : (Window)(None)), \
	                  X,Y,W,H,B,FG,BG)


/* Init a top level infowin by pos,size,bord,Colors */
#define Infowin_init_top(X,Y,W,H,B,FG,BG) \
	Infowin_init_data(None,X,Y,W,H,B,FG,BG)


/* Request a new standard window by giving Dad infowin and X,Y,W,H */
#define Infowin_init_std(D,X,Y,W,H,B) \
	Infowin_init_dad(D,X,Y,W,H,B,Metadpy->fg,Metadpy->bg)


/* Set the current Infowin */
#define Infowin_set(I) \
	(Infowin = (I))


/* Set the current Infoclr */
#define Infoclr_set(C) \
	(Infoclr = (C))


#define Infoclr_init_ppo(F,B,O,M) \
	Infoclr_init_data(F,B,O,M)

#define Infoclr_init_cco(F,B,O,M) \
	Infoclr_init_ppo(Infoclr_Pixell(F),Infoclr_Pixell(B),O,M)

#define Infoclr_init_ppn(F,B,O,M) \
	Infoclr_init_ppo(F,B,Infoclr_Opcode(O),M)

#define Infoclr_init_ccn(F,B,O,M) \
	Infoclr_init_cco(F,B,Infoclr_Opcode(O),M)


/* Set the current infofnt */
#define Infofnt_set(I) \
	(Infofnt = (I))



/**** Generic Globals ****/


/*
 * The "default" values
 */
static metadpy metadpy_default;


/*
 * The "current" variables
 */
static metadpy *Metadpy = &metadpy_default;
static infowin *Infowin = (infowin*)(NULL);
static infoclr *Infoclr = (infoclr*)(NULL);
static infofnt *Infofnt = (infofnt*)(NULL);



/**** Generic code ****/


/*
 * Init the current metadpy, with various initialization stuff.
 *
 * Inputs:
 *	dpy:  The Display* to use (if NULL, create it)
 *	name: The name of the Display (if NULL, the current)
 *
 * Notes:
 *	If 'name' is NULL, but 'dpy' is set, extract name from dpy
 *	If 'dpy' is NULL, then Create the named Display
 *	If 'name' is NULL, and so is 'dpy', use current Display
 *
 * Return -1 if no Display given, and none can be opened.
 */
static errr Metadpy_init_2(Display *dpy, cptr name)
{
	metadpy *m = Metadpy;

	/*** Open the display if needed ***/

	/* If no Display given, attempt to Create one */
	if (!dpy)
	{
		/* Attempt to open the display */
		dpy = XOpenDisplay(name);

		/* Failure */
		if (!dpy) return (-1);

		/* We will have to nuke it when done */
		m->nuke = 1;
	}

	/* Since the Display was given, use it */
	else
	{
		/* We will not have to nuke it when done */
		m->nuke = 0;
	}


	/*** Save some information ***/

	/* Save the Display itself */
	m->dpy = dpy;

	/* Get the Screen and Virtual Root Window */
	m->screen = DefaultScreenOfDisplay(dpy);
	m->root = RootWindowOfScreen(m->screen);

	/* Get the default colormap */
	m->cmap = DefaultColormapOfScreen(m->screen);

	/* Extract the true name of the display */
	m->name = DisplayString(dpy);

	/* Extract the fd */
	m->fd = ConnectionNumber(Metadpy->dpy);

	/* Save the Size and Depth of the screen */
	m->width = WidthOfScreen(m->screen);
	m->height = HeightOfScreen(m->screen);
	m->depth = DefaultDepthOfScreen(m->screen);

	/* Save the Standard Colors */
	m->black = BlackPixelOfScreen(m->screen);
	m->white = WhitePixelOfScreen(m->screen);

	/*** Make some clever Guesses ***/

	/* Guess at the desired 'fg' and 'bg' Pixell's */
	m->bg = m->black;
	m->fg = m->white;

	/* Calculate the Maximum allowed Pixel value.  */
	m->zg = ((Pixell)1 << m->depth) - 1;

	/* Save various default Flag Settings */
	m->color = ((m->depth > 1) ? 1 : 0);
	m->mono = ((m->color) ? 0 : 1);

	/* Return "success" */
	return (0);
}


/*
 * General Flush/ Sync/ Discard routine
 */
static errr Metadpy_update(int flush, int sync, int discard)
{
	/* Flush if desired */
	if (flush) XFlush(Metadpy->dpy);

	/* Sync if desired, using 'discard' */
	if (sync) XSync(Metadpy->dpy, discard);

	/* Clear the arrays used for optimisation */
	(void)C_WIPE(pj_row1, 64, u32b);
	(void)C_WIPE(pj_row2, 64, u32b);

	/* Hack - use crazy row to mark "nothing entered yet" */
	pj_cur_row = -255;

	/* Success */
	return (0);
}


/*
 * Make a simple beep
 */
static errr Metadpy_do_beep(void)
{
	/* Make a simple beep */
	XBell(Metadpy->dpy, 100);

	return (0);
}



/*
 * Set the name (in the title bar) of Infowin
 */
static errr Infowin_set_name(cptr name)
{
	Status st;
	XTextProperty tp;
	char buf[128];
	char *bp = buf;
	my_strcpy(buf, name, sizeof(buf));
	st = XStringListToTextProperty(&bp, 1, &tp);
	if (st) XSetWMName(Metadpy->dpy, Infowin->win, &tp);
	return (0);
}


/*
 * Prepare a new 'infowin'.
 */
static errr Infowin_prepare(Window xid)
{
	infowin *iwin = Infowin;

	Window tmp_win;
	XWindowAttributes xwa;
	int x, y;
	unsigned int w, h, b, d;

	/* Assign stuff */
	iwin->win = xid;

	/* Check For Error XXX Extract some ACTUAL data from 'xid' */
	XGetGeometry(Metadpy->dpy, xid, &tmp_win, &x, &y, &w, &h, &b, &d);

	/* Apply the above info */
	iwin->x = x;
	iwin->y = y;
	iwin->w = w;
	iwin->h = h;
	iwin->b = b;

	/* Check Error XXX Extract some more ACTUAL data */
	XGetWindowAttributes(Metadpy->dpy, xid, &xwa);

	/* Apply the above info */
	iwin->mask = xwa.your_event_mask;
	iwin->mapped = ((xwa.map_state == IsUnmapped) ? 0 : 1);

	/* Success */
	return (0);
}



/*
 * Init an infowin by giving some data.
 *
 * Inputs:
 *	dad: The Window that should own this Window (if any)
 *	x,y: The position of this Window
 *	w,h: The size of this Window
 *	b,d: The border width and pixel depth
 *
 * Notes:
 *	If 'dad == None' assume 'dad == root'
 */
static errr Infowin_init_data(Window dad, int x, int y, int w, int h,
                              int b, Pixell fg, Pixell bg)
{
	Window xid;

	/* Wipe it clean */
	(void)WIPE(Infowin, infowin);


	/*** Error Check XXX ***/


	/*** Create the Window 'xid' from data ***/

	/* What happened here?  XXX XXX XXX */

	/* If no parent given, depend on root */
	if (dad == None) dad = Metadpy->root;

	/* Create the Window XXX Error Check */
	xid = XCreateSimpleWindow(Metadpy->dpy, dad, x, y, w, h, b, fg, bg);

	/* Start out selecting No events */
	XSelectInput(Metadpy->dpy, xid, 0L);


	/*** Prepare the new infowin ***/

	/* Mark it as nukable */
	Infowin->nuke = 1;

	/* Attempt to Initialize the infowin */
	return (Infowin_prepare(xid));
}



/*
 * Modify the event mask of an Infowin
 */
static errr Infowin_set_mask(long mask)
{
	/* Save the new setting */
	Infowin->mask = mask;

	/* Execute the Mapping */
	XSelectInput(Metadpy->dpy, Infowin->win, Infowin->mask);

	/* Success */
	return (0);
}


/*
 * Request that Infowin be mapped
 */
static errr Infowin_map(void)
{
	/* Execute the Mapping */
	XMapWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}



/*
 * Request that Infowin be raised
 */
static errr Infowin_raise(void)
{
	/* Raise towards visibility */
	XRaiseWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}



/*
 * Request that Infowin be moved to a new location
 */
static errr Infowin_impell(int x, int y)
{
	/* Execute the request */
	XMoveWindow(Metadpy->dpy, Infowin->win, x, y);

	/* Success */
	return (0);
}


/*
 * Resize an infowin
 */
static errr Infowin_resize(int w, int h)
{
	/* Execute the request */
	XResizeWindow(Metadpy->dpy, Infowin->win, w, h);

	/* Success */
	return (0);
}


/*
 * Visually clear Infowin
 */
static errr Infowin_wipe(void)
{
	/* Execute the request */
	XClearWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}


/*
 * A NULL terminated pair list of legal "operation names"
 *
 * Pairs of values, first is texttual name, second is the string
 * holding the decimal value that the operation corresponds to.
 */
static cptr opcode_pairs[] =
{
	"cpy", "3",
	"xor", "6",
	"and", "1",
	"ior", "7",
	"nor", "8",
	"inv", "10",
	"clr", "0",
	"set", "15",

	"src", "3",
	"dst", "5",

	"+andReverse", "2",
	"+andInverted", "4",
	"+noop", "5",
	"+equiv", "9",
	"+orReverse", "11",
	"+copyInverted", "12",
	"+orInverted", "13",
	"+nand", "14",
	NULL
};


/*
 * Parse a word into an operation "code"
 *
 * Inputs:
 *	str: A string, hopefully representing an Operation
 *
 * Output:
 *	0-15: if 'str' is a valid Operation
 *	-1:   if 'str' could not be parsed
 */
static int Infoclr_Opcode(cptr str)
{
	register int i;

	/* Scan through all legal operation names */
	for (i = 0; opcode_pairs[i*2]; ++i)
	{
		/* Is this the right oprname? */
		if (streq(opcode_pairs[i*2], str))
		{
			/* Convert the second element in the pair into a Code */
			return (atoi(opcode_pairs[i*2+1]));
		}
	}

	/* The code was not found, return -1 */
	return (-1);
}



/*
 * Initialize an infoclr with some data
 *
 * Inputs:
 *	fg:   The Pixell for the requested Foreground (see above)
 *	bg:   The Pixell for the requested Background (see above)
 *	op:   The Opcode for the requested Operation (see above)
 *	stip: The stipple mode
 */
static errr Infoclr_init_data(Pixell fg, Pixell bg, int op, int stip)
{
	infoclr *iclr = Infoclr;

	GC gc;
	XGCValues gcv;
	unsigned long gc_mask;



	/*** Simple error checking of opr and clr ***/

	/* Check the 'Pixells' for realism */
	if (bg > Metadpy->zg) return (-1);
	if (fg > Metadpy->zg) return (-1);

	/* Check the data for trueness */
	if ((op < 0) || (op > 15)) return (-1);


	/*** Create the requested 'GC' ***/

	/* Assign the proper GC function */
	gcv.function = op;

	/* Assign the proper GC background */
	gcv.background = bg;

	/* Assign the proper GC foreground */
	gcv.foreground = fg;

	/* Hack -- Handle XOR (xor is code 6) by hacking bg and fg */
	if (op == 6) gcv.background = 0;
	if (op == 6) gcv.foreground = (bg ^ fg);

	/* Assign the proper GC Fill Style */
	gcv.fill_style = (stip ? FillStippled : FillSolid);

	/* Turn off 'Give exposure events for pixmap copying' */
	gcv.graphics_exposures = False;

	/* Set up the GC mask */
	gc_mask = (GCFunction | GCBackground | GCForeground |
	           GCFillStyle | GCGraphicsExposures);

	/* Create the GC detailed above */
	gc = XCreateGC(Metadpy->dpy, Metadpy->root, gc_mask, &gcv);


	/*** Initialize ***/

	/* Wipe the iclr clean */
	(void)WIPE(iclr, infoclr);

	/* Assign the GC */
	iclr->gc = gc;

	/* Nuke it when done */
	iclr->nuke = 1;

	/* Assign the parms */
	iclr->fg = fg;
	iclr->bg = bg;
	iclr->code = op;
	iclr->stip = stip ? 1 : 0;

	/* Success */
	return (0);
}



/*
 * Change the 'fg' for an infoclr
 *
 * Inputs:
 *	fg:   The Pixell for the requested Foreground (see above)
 */
static errr Infoclr_change_fg(Pixell fg)
{
	infoclr *iclr = Infoclr;


	/*** Simple error checking of opr and clr ***/

	/* Check the 'Pixells' for realism */
	if (fg > Metadpy->zg) return (-1);


	/*** Change ***/

	/* Change */
	XSetForeground(Metadpy->dpy, iclr->gc, fg);

	/* Success */
	return (0);
}



/*
 * Prepare a new 'infofnt'
 */
static errr Infofnt_prepare(XFontStruct *info)
{
	infofnt *ifnt = Infofnt;

	XCharStruct *cs;

	/* Assign the struct */
	ifnt->info = info;

	/* Jump into the max bouonds thing */
	cs = &(info->max_bounds);

	/* Extract default sizing info */
	ifnt->asc = info->ascent;
	ifnt->hgt = info->ascent + info->descent;
	ifnt->wid = cs->width;

	/* Success */
	return (0);
}



/*
 * Init an infofnt by its Name
 *
 * Inputs:
 *	name: The name of the requested Font
 */
static errr Infofnt_init_data(cptr name)
{
	XFontStruct *info;


	/*** Load the info Fresh, using the name ***/

	/* If the name is not given, report an error */
	if (!name) return (-1);

	/* Attempt to load the font */
	info = XLoadQueryFont(Metadpy->dpy, name);

	/* The load failed, try to recover */
	if (!info) return (-1);


	/*** Init the font ***/

	/* Wipe the thing */
	(void)WIPE(Infofnt, infofnt);

	/* Attempt to prepare it */
	if (Infofnt_prepare(info))
	{
		/* Free the font */
		XFreeFont(Metadpy->dpy, info);

		/* Fail */
		return (-1);
	}

	/* Save a copy of the font name */
	Infofnt->name = string_make(name);

	/* Mark it as nukable */
	Infofnt->nuke = 1;

	/* Success */
	return (0);
}


/*
 * Standard Text
 */
static errr Infofnt_text_std(int x, int y, cptr str, int len)
{
	int i;


	/*** Do a brief info analysis ***/

	/* Do nothing if the string is null */
	if (!str || !*str) return (-1);

	/* Get the length of the string */
	if (len < 0) len = strlen(str);

	/* Ignore Vertical Justifications */
	y = y * Infofnt->hgt + Infofnt->asc + Infowin->oy;


	/*** Decide where to place the string, horizontally ***/

	/* Line up with x at left edge of column 'x' */
	x = x * Infofnt->wid + Infowin->ox;

	/*** Actually draw 'str' onto the infowin ***/

	/* Be sure the correct font is ready */
	XSetFont(Metadpy->dpy, Infoclr->gc, Infofnt->info->fid);


	/*** Handle the fake mono we can enforce on fonts ***/

	/* Monotize the font */
	if (Infofnt->mono)
	{
		/* Do each character */
		for (i = 0; i < len; ++i)
		{
			/* Note that the Infoclr is set up to contain the Infofnt */
			XDrawImageString(Metadpy->dpy, Infowin->win, Infoclr->gc,
			                 x + i * Infofnt->wid + Infofnt->off, y, str + i, 1);
		}
	}

	/* Assume monospaced font */
	else
	{
		/* Note that the Infoclr is set up to contain the Infofnt */
		XDrawImageString(Metadpy->dpy, Infowin->win, Infoclr->gc,
		                 x, y, str, len);
	}

	/* Success */
	return (0);
}


/*************************************************************************/


/*
 * Angband specific code follows... (ANGBAND)
 */



/*
 * Process a keypress event
 *
 * Also appears in "main-xaw.c".
 */
static void react_keypress(XKeyEvent *ev)
{
	int i, n, mc, ms, mo, mx;

	uint ks1;

	KeySym ks;

	char buf[128];
	char msg[128];


	/* Check for "normal" keypresses */
	n = XLookupString(ev, buf, 125, &ks, NULL);

	/* Terminate */
	buf[n] = '\0';


	/* Hack -- Ignore "modifier keys" */
	if (IsModifierKey(ks)) return;


	/* Hack -- convert into an unsigned int */
	ks1 = (uint)(ks);

	/* Extract four "modifier flags" */
	mc = (ev->state & ControlMask) ? TRUE : FALSE;
	ms = (ev->state & ShiftMask) ? TRUE : FALSE;
	mo = (ev->state & Mod1Mask) ? TRUE : FALSE;
	mx = (ev->state & Mod2Mask) ? TRUE : FALSE;


	/* Normal keys with no modifiers */
	if (n && !mo && !mx && !IsSpecialKey(ks))
	{
		/* Enqueue the normal key(s) */
		for (i = 0; buf[i]; i++) Term_keypress(buf[i]);

		/* All done */
		return;
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch (ks1)
	{
		case XK_Escape:
		{
			Term_keypress(ESCAPE);
			return;
		}

		case XK_Return:
		{
			Term_keypress('\r');
			return;
		}

		case XK_Tab:
		{
			Term_keypress('\t');
			return;
		}

		case XK_Delete:
		case XK_BackSpace:
		{
			Term_keypress('\010');
			return;
		}
	}


	/* Hack -- Use the KeySym */
	if (ks)
	{
		strnfmt(msg, sizeof(msg), "%c%s%s%s%s_%lX%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        (unsigned long)(ks), 13);
	}

	/* Hack -- Use the Keycode */
	else
	{
		strnfmt(msg, sizeof(msg), "%c%s%s%s%sK_%X%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        ev->keycode, 13);
	}

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);


	/* Hack -- auto-define macros as needed */
	if (n && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, buf);
	}
}




/*
 * Process events
 */
static errr CheckEvent(bool wait)
{
	term_data *old_td = (term_data*)(Term->data);

	XEvent xev_body, *xev = &xev_body;

	term_data *td = NULL;
	infowin *iwin = NULL;

	int i, x, y;
	int window = 0;

	/* Do not wait unless requested */
	if (!wait && !XPending(Metadpy->dpy)) return (1);

	/* Load the Event */
	XNextEvent(Metadpy->dpy, xev);


	/* Notice new keymaps */
	if (xev->type == MappingNotify)
	{
		XRefreshKeyboardMapping(&xev->xmapping);
		return 0;
	}


	/* Scan the windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (xev->xany.window == data[i].win->win)
		{
			td = &data[i];
			iwin = td->win;
			window = i;
			break;
		}
	}

	/* Unknown window */
	if (!td || !iwin) return (0);


	/* Hack -- activate the Term */
	Term_activate(&td->t);

	/* Hack -- activate the window */
	Infowin_set(iwin);


	/* Switch on the Type */
	switch (xev->type)
	{

#if 0

		case ButtonPress:
		case ButtonRelease:
		{
			int z = 0;

			/* Which button is involved */
			if (xev->xbutton.button == Button1) z = 1;
			else if (xev->xbutton.button == Button2) z = 2;
			else if (xev->xbutton.button == Button3) z = 3;
			else if (xev->xbutton.button == Button4) z = 4;
			else if (xev->xbutton.button == Button5) z = 5;

			/* Where is the mouse */
			x = xev->xbutton.x;
			y = xev->xbutton.y;

			/* XXX Handle */

			break;
		}

		case EnterNotify:
		case LeaveNotify:
		{
			/* Where is the mouse */
			x = xev->xcrossing.x;
			y = xev->xcrossing.y;

			/* XXX Handle */

			break;
		}

		case MotionNotify:
		{
			/* Where is the mouse */
			x = xev->xmotion.x;
			y = xev->xmotion.y;

			/* XXX Handle */

			break;
		}

		case KeyRelease:
		{
			/* Nothing */
			break;
		}

#endif

		case KeyPress:
		{
			/* Save the mouse location */
			x = xev->xkey.x;
			y = xev->xkey.y;

			/* Hack -- use "old" term */
			Term_activate(&old_td->t);

			/* Process the key */
			react_keypress(&(xev->xkey));

			break;
		}

		case Expose:
		{
			int x1, x2, y1, y2;

			/* Ignore "extra" exposes */
			/*if (xev->xexpose.count) break;*/

			/* Clear the window */
			/*Infowin_wipe();*/



			/* Redraw */
			if (window == 0)
			{
				x1 = (xev->xexpose.x - Infowin->ox)/P_TILE_SIZE;
				x2 = (xev->xexpose.x + xev->xexpose.width -
					 Infowin->ox)/P_TILE_SIZE;

				y1 = (xev->xexpose.y - Infowin->oy)/P_TILE_SIZE;
				y2 = (xev->xexpose.y + xev->xexpose.height -
					 Infowin->oy)/P_TILE_SIZE;

				/* Hack - area invalidated on main window is not a rectangle */
				Term_redraw_section(x1 - y2 / 2, y1, x2 + y2 / 2, y2);
				/* Term_redraw(); */
			}
			else
			{
				x1 = (xev->xexpose.x - Infowin->ox)/Infofnt->wid;
				x2 = (xev->xexpose.x + xev->xexpose.width -
					 Infowin->ox)/Infofnt->wid;

				y1 = (xev->xexpose.y - Infowin->oy)/Infofnt->hgt;
				y2 = (xev->xexpose.y + xev->xexpose.height -
					 Infowin->oy)/Infofnt->hgt;

				Term_redraw_section(x1, y1, x2, y2);
			}

			break;
		}

		case MapNotify:
		{
			Infowin->mapped = 1;
			Term->mapped_flag = TRUE;
			break;
		}

		case UnmapNotify:
		{
			Infowin->mapped = 0;
			Term->mapped_flag = FALSE;
			break;
		}

		/* Move and/or Resize */
		case ConfigureNotify:
		{
			int cols, rows, wid, hgt;

			int ox = Infowin->ox;
			int oy = Infowin->oy;

			/* Save the new Window Parms */
			Infowin->x = xev->xconfigure.x;
			Infowin->y = xev->xconfigure.y;
			Infowin->w = xev->xconfigure.width;
			Infowin->h = xev->xconfigure.height;

			if (window == 0)
			{
				/* Determine "proper" number of rows/cols */
#if 0
				rows = 24;
				cols = 80;
#endif /* 0 */

				rows = (Infowin->h - (oy + oy)) / P_TILE_SIZE - 1;
				cols = (Infowin->w - (ox + ox) - rows * P_TILE_SIZE / 2)
					 / P_TILE_SIZE - 1;
			}
			else
			{
				/* Determine "proper" number of rows/cols */
				cols = (Infowin->w - (ox + ox)) / td->fnt->wid;
				rows = (Infowin->h - (oy + oy)) / td->fnt->hgt;
			}

			if (window == 0)
			{
				/* Hack the main window must be at least 80x24 */
				if (cols < 80) cols = 80;
				if (rows < 24) rows = 24;
			}
			else
			{
				/* Hack -- minimal size for normal windows */
				if (cols < 1) cols = 1;
				if (rows < 1) rows = 1;
			}

			if (window == 0)
			{
				/* Desired size of window */
				wid = (cols + 1) * P_TILE_SIZE + rows * P_TILE_SIZE / 2
					+ ox * 2;
				hgt = (rows + 1) * P_TILE_SIZE + oy * 2;
			}
			else
			{
				/* Desired size of window */
				wid = cols * td->fnt->wid + ox * 2;
				hgt = rows * td->fnt->hgt + oy * 2;
			}

			/* Resize the Term (if needed) */
			(void)Term_resize(cols, rows);

			/* Resize the windows if any "change" is needed */
			if ((Infowin->w != wid) || (Infowin->h != hgt))
			{
				/* Resize window */
				Infowin_set(td->win);
				Infowin_resize(wid, hgt);
			}

			break;
		}
	}


	/* Hack -- Activate the old term */
	Term_activate(&old_td->t);

	/* Hack -- Activate the proper window */
	Infowin_set(old_td->win);


	/* Success */
	return (0);
}


/*
 * Handle "activation" of a term
 */
static errr Term_xtra_xpj_level(int v)
{
	term_data *td = (term_data*)(Term->data);

	/* Handle "activate" */
	if (v)
	{
		/* Activate the window */
		Infowin_set(td->win);

		/* Activate the font */
		Infofnt_set(td->fnt);
	}

	/* Success */
	return (0);
}


/*
 * React to changes
 */
static errr Term_xtra_xpj_react(void)
{
	int i;

	int j, k;

	if (Metadpy->color)
	{
		/* Check the colors */
		for (i = 0; i < 256; i++)
		{
			if ((color_table[i][0] != angband_color_table[i][0]) ||
			    (color_table[i][1] != angband_color_table[i][1]) ||
			    (color_table[i][2] != angband_color_table[i][2]) ||
			    (color_table[i][3] != angband_color_table[i][3]))
			{
				Pixell pixel;

				/* Save new values */
				color_table[i][0] = angband_color_table[i][0];
				color_table[i][1] = angband_color_table[i][1];
				color_table[i][2] = angband_color_table[i][2];
				color_table[i][3] = angband_color_table[i][3];

				/* Create pixel */
				pixel = create_pixel(Metadpy->dpy,
				                     color_table[i][1],
				                     color_table[i][2],
				                     color_table[i][3]);

				/* Change the foreground */
				Infoclr_set(clr[i]);
				Infoclr_change_fg(pixel);

				/* Only need to readjust fonts for lower 32 colours */
				if (i > 31) continue;

				/* Need to redo the font metrics */
				for (j = 0; j < P_TILE_SIZE; j++)
				{
					for (k = 0; k < 128 * P_TILE_SIZE; k++)
					{
						/* Recolour the changed pixels */
						if (XGetPixel(font_data, i * P_TILE_SIZE + j, k))
						{
							XPutPixel(font_data, i * P_TILE_SIZE + j, k,
							          pixel);
						}
					}
				}
			}
		}
	}

	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_xpj(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE: Metadpy_do_beep(); return (0);

		/* Flush the output XXX XXX */
		case TERM_XTRA_FRESH: Metadpy_update(1, 0, 0); return (0);

		/* Process random events XXX */
		case TERM_XTRA_BORED: return (CheckEvent(0));

		/* Process Events XXX */
		case TERM_XTRA_EVENT: return (CheckEvent(v));

		/* Flush the events XXX */
		case TERM_XTRA_FLUSH: while (!CheckEvent(FALSE)); return (0);

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: return (Term_xtra_xpj_level(v));

		/* Clear the screen */
		case TERM_XTRA_CLEAR: Infowin_wipe(); return (0);

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY: usleep(1000 * v); return (0);

		/* React to changes */
		case TERM_XTRA_REACT: return (Term_xtra_xpj_react());
	}

	/* Unknown */
	return (1);
}


/*
 * Draw the cursor as an inverted rectangle.
 *
 * Consider a rectangular outline like "main-mac.c".  XXX XXX
 */
static errr Term_curs_xpj(int x, int y)
{
	/* Are we on the main window? */
	if (Term->data == &data[0])
	{
		XDrawRectangle(Metadpy->dpy, Infowin->win, xor->gc,
		               x * P_TILE_SIZE + y * P_TILE_SIZE / 2 +
		               P_TILE_SIZE  / 4 + Infowin->ox,
		               y * P_TILE_SIZE + P_TILE_SIZE / 2 + Infowin->oy,
		               P_TILE_SIZE - 1, P_TILE_SIZE - 1);
	}
	else
	{
		XDrawRectangle(Metadpy->dpy, Infowin->win, xor->gc,
		               x * Infofnt->wid + Infowin->ox,
		               y * Infofnt->hgt + Infowin->oy,
		               Infofnt->wid - 1, Infofnt->hgt - 1);
	}

	/* Success */
	return (0);
}




/*
 * Draw some textual characters.
 */
static errr Term_text_xpj(int x, int y, int n, byte a, cptr s)
{
	/* Draw the text */
	Infoclr_set(clr[a]);

	/* Draw the text */
	Infofnt_text_std(x, y, s, n);

	/* Success */
	return (0);
}


/*
 * Draw a block with byte-sized pixels
 */
static void draw_block8(void *tiles[PJ_MAX],
	 u16b pj_table[P_TILE_SIZE][P_TILE_SIZE / 2], u16b mask,
	 term_data *td)
{
	int i, j;
	byte pixel, val;

	/* List of possibly visible tiles */
	u16b value;

	/* Plot the pixels onto the bitmap */
	for (i = 0; i < P_TILE_SIZE / 2; i++)
	{
		for (j = 0; j < P_TILE_SIZE; j++)
		{
			/* Get tiles on this pixel */
			value = pj_table[j][i] & mask;

			pixel = 0;

			while (!pixel)
			{
				/* Get the number of the bit to look at */
				val = bit_high_lookup[value / 256];

				if (val)
				{
					val += 7;
				}
				else
				{
					val = bit_high_lookup[value];

					/* Check for null case */
					if (!val)
					{
						/* No allowable tile - use (0,0) */
						pixel = pix_blank;
						break;
					}

					val--;
				}


				/*
				 * Update the bit so that transparency works
				 * (by getting rid of the bit we are using now.)
				 */
				value &= (~(1 << val));

				/*
				 * Pick the pixel to use.
				 *
				 * This is majorly optimised.
				 * The offsets have been moved into the pointers passed
				 * by tiles.  All that remains are the parts dependant
				 * on i and j.
				 */
				pixel = ((byte *) tiles[val])
					[t_xscale[val] * j / 2 + i * (wall_flip[val] + 1)
					 	+ (j - 2 * i * wall_flip[val]) * 32 * P_TILE_SIZE];
			}

			/* Write the pixel onto the bitmap */
			((byte *)td->SkewImage->data)[i + j * P_TILE_SIZE / 2] = pixel;
		}
	}
}


/*
 * Draw a block with 16bit pixels
 */
static void draw_block16(void *tiles[PJ_MAX],
	 u16b pj_table[P_TILE_SIZE][P_TILE_SIZE / 2], u16b mask,
	 term_data *td)
{
	int i, j;
	byte val;

	u16b pixel;

	/* List of possibly visible tiles */
	u16b value;

	/* Plot the pixels onto the bitmap */
	for (i = 0; i < P_TILE_SIZE / 2; i++)
	{
		for (j = 0; j < P_TILE_SIZE; j++)
		{
			/* Get tiles on this pixel */
			value = pj_table[j][i] & mask;

			pixel = 0;

			while (!pixel)
			{
				/* Get the number of the bit to look at */
				val = bit_high_lookup[value / 256];

				if (val)
				{
					val += 7;
				}
				else
				{
					val = bit_high_lookup[value];

					/* Check for null case */
					if (!val)
					{
						/* No allowable tile - use (0,0) */
						pixel = pix_blank;
						break;
					}

					val--;
				}


				/*
				 * Update the mask so that transparency works
				 * (by getting rid of the particular bit we are using now.)
				 */
				value &= (~(1 << val));

				/*
				 * Pick the pixel to use.
				 *
				 * This is majorly optimised.
				 * The offsets have been moved into the pointers passed
				 * by tiles.  All that remains are the parts dependant
				 * on i and j.
				 */
				pixel = ((u16b *)tiles[val])
					[t_xscale[val] * j / 2 + i * (wall_flip[val] + 1)
					 	+ (j - 2 * i * wall_flip[val]) * 32 * P_TILE_SIZE];
			}

			/* Write the pixel onto the bitmap */
			((u16b *) td->SkewImage->data)[i + j * P_TILE_SIZE / 2] = pixel;
		}
	}
}


/* 
 * Draw a block with 32bit pixels
 */
static void draw_block32(void *tiles[PJ_MAX],
	 u16b pj_table[P_TILE_SIZE][P_TILE_SIZE / 2], u16b mask,
	 term_data *td)
{
	int i, j;
	byte val;
	u32b pixel;

	/* List of possibly visible tiles */
	u16b value;

	/* Plot the pixels onto the bitmap */
	for (i = 0; i < P_TILE_SIZE / 2; i++)
	{
		for (j = 0; j < P_TILE_SIZE; j++)
		{
			/* Get tiles on this pixel */
			value = pj_table[j][i] & mask;

			pixel = 0;

			while (!pixel)
			{
				/* Get the number of the bit to look at */
				val = bit_high_lookup[value / 256];

				if (val)
				{
					val += 7;
				}
				else
				{
					val = bit_high_lookup[value];

					/* Check for null case */
					if (!val)
					{
						/* No allowable tile - use (0,0) */
						pixel = pix_blank;
						break;
					}

					val--;
				}


				/*
				 * Update the bit so that transparency works
				 * (by getting rid of the bit we are using now.)
				 */
				value &= (~(1 << val));

				/*
				 * Pick the pixel to use.
				 *
				 * This is majorly optimised.
				 * The offsets have been moved into the pointers passed
				 * by tiles.  All that remains are the parts dependant
				 * on i and j.
				 */
				pixel = ((u32b *)tiles[val])
					[t_xscale[val] * j / 2 + i * (wall_flip[val]+1)
					 	+ (j - 2 * i * wall_flip[val]) * 32 * P_TILE_SIZE];
			}

			/* Write the pixel onto the bitmap */
			((u32b *) td->SkewImage->data)[i + j * P_TILE_SIZE / 2] = pixel;
		}
	}
}

/* Macro used to set the tile information */
#define set_tile1(N, X, Y) table[N] = &(((byte *)td->tiles->data)\
	[(t_offsetx1[N] + (X) * P_TILE_SIZE\
		+ (t_offsety1[N] + (Y) * P_TILE_SIZE) * 32 * P_TILE_SIZE)\
		 * bytes_per_pixel])

/* Macro used to set the tile (font-based) information */
#define set_font1(N, Y, X) table[N] = &(((byte *)font_data->data)\
	[(t_offsetx1[N] + (X) * P_TILE_SIZE\
		+ (t_offsety1[N] + (Y) * P_TILE_SIZE) * 32 * P_TILE_SIZE)\
		 * bytes_per_pixel])

/* Hack XXX XXX - convert to illuminated wall */
#define ILLUMINATE(X) (((((X & 0x3f) - 1) / 3) * 3) + 2 + BRIGHT_WALLS)


static errr draw_rect_t1(int x, int y, term_data *td, int xp, int yp)
{
	term_win *window = td->t.scr;

	void *table[PJ_MAX];

	/* List of possibly visible tiles */
	u16b mask = 0;

	/* The tile attr / char pairs */
	byte a;
	char c;
	byte ta;
	char tc;

	bool floor_blank1 = FALSE, floor_blank2 = FALSE;

	/* Look to see if we are already drawn */
	int cur_col = x / 16;
	u32b row_mask = (1L << (x % 16));

	/* Paranoia */
	if ((x < -1) || (y < -1) || (x >= td->t.wid) || (y >= td->t.hgt))
	{
		return (0);
	}

	/* Look to see if we are already drawn */
	if (y == pj_cur_row)
	{
		/* Are we drawn? */
		if (pj_row1[cur_col] & row_mask) return (0);

		/* Set "drawn" flag */
		pj_row1[cur_col] |= row_mask;

	}
	else if (y == pj_cur_row - 1)
	{
		/* Are we drawn? */
		if (pj_row2[cur_col] & row_mask) return (0);

		/* Set "drawn" flag */
		pj_row2[cur_col] |= row_mask;
	}
	else if (y == pj_cur_row + 1)
	{
		/* We've moved to another row */

		/* Copy the contents of the pj_row1[] array */
		C_COPY(pj_row2, pj_row1, 64, u32b);

		/* Wipe the old "current row" */
		(void)C_WIPE(pj_row1, 64, u32b);

		/* We are now at a larger row */
		pj_cur_row++;

		/* Set "drawn" flag */
		pj_row1[cur_col] |= row_mask;
	}
	else
	{
		/* Clear the arrays used for optimisation */
		(void)C_WIPE(pj_row1, 64, u32b);
		(void)C_WIPE(pj_row2, 64, u32b);

		/* We are at row y */
		pj_cur_row = y;

		/* Set "drawn" flag */
		pj_row1[cur_col] |= row_mask;
	}

	if ((x > 0) && (y >= 0))
	{
		/* Get tiles we need */
		a = window->a[y][x - 1];
		c = window->c[y][x - 1];
		ta = window->ta[y][x - 1];
		tc = window->tc[y][x - 1];

		/* Are we overlaying anything? */
		if ((a != ta) || (c != tc) || !(a & 0x80))
		{
			mask |= PJ_T_OVER3;

			if (a & 0x80)
			{
				set_tile1(PJ_OVER3, c & 0x3F, a & 0x7F);
			}
			else
			{
				set_font1(PJ_OVER3, c & 0x7F, a & 0x1F);
			}
		}

		/* Solid wall? */
		else if (tc & 0x40)
		{
			mask |= (PJ_T_WALLF | PJ_T_WALL1 | PJ_T_WALL1_T);

			set_tile1(PJ_WALLF, tc & 0x3F, ta & 0x7F);
			set_tile1(PJ_WALL1, tc & 0x3F, ta & 0x7F);
			set_tile1(PJ_WALL1_T, tc & 0x3F, ta & 0x7F);
		}

		/* Is the terrain null? */
		if ((ta & 0x80) && !(tc & 0x40))
		{
			mask |= PJ_T_FLOOR2;
			set_tile1(PJ_FLOOR2, tc & 0x3F, ta & 0x7F);

			/* Blank floor? */
			if (!((tc & 0x3F) || (ta & 0x7F))) floor_blank2 = TRUE;
		}
	}

	if ((x >= 0) && (y >= 0))
	{
		/* Get tiles we need */
		a = window->a[y][x];
		c = window->c[y][x];
		ta = window->ta[y][x];
		tc = window->tc[y][x];

		/* Are we overlaying anything? */
		if ((a != ta) || (c != tc) || !(a & 0x80))
		{
			mask |= PJ_T_OVER1;

			if (a & 0x80)
			{
				set_tile1(PJ_OVER1, c & 0x3F, a & 0x7F);
			}
			else
			{
				set_font1(PJ_OVER1, c & 0x7F, a & 0x1F);
			}
		}

		/* Solid wall? */
		else if (tc & 0x40)
		{
			if (mask & PJ_T_WALL1)
			{
				mask &= ~(PJ_T_WALL1 | PJ_T_WALL1_T);
			}
			else
			{
				mask |= (PJ_T_WALL1 | PJ_T_WALL1_T);

				set_tile1(PJ_WALL1, tc & 0x3F, ta & 0x7F);
				set_tile1(PJ_WALL1_T, tc & 0x3F, ta & 0x7F);
			}
		}

		/* Is the terrain null? */
		if ((ta & 0x80) && !(tc & 0x40))
		{
			mask |= PJ_T_FLOOR1;
			set_tile1(PJ_FLOOR1, tc & 0x3F, ta & 0x7F);

			/* Blank floor? */
			if (!((tc & 0x3F) || (ta & 0x7F))) floor_blank1 = TRUE;
		}
	}

	/* Check below */
	if ((x > 0) && (y < td->t.hgt - 1))
	{
		/* Get tiles we need */
		a = window->a[y + 1][x - 1];
		c = window->c[y + 1][x - 1];
		ta = window->ta[y + 1][x - 1];
		tc = window->tc[y + 1][x - 1];

		/* Are we overlaying anything? */
		if ((a != ta) || (c != tc) || !(a & 0x80))
		{
			mask |= PJ_T_OVER2;

			if (a & 0x80)
			{
				set_tile1(PJ_OVER2, c & 0x3F, a & 0x7F);
			}
			else
			{
				set_font1(PJ_OVER2, c & 0x7F, a & 0x1F);
			}
		}

		/* Solid wall? */
		else if (tc & 0x40)
		{
			mask |= PJ_T_TOP1 | PJ_T_TOP_T1;

			if (tc & 0x80)
			{
				/* Special lighting for walls */
				set_tile1(PJ_TOP1, ILLUMINATE(tc), ta & 0x7F);
				set_tile1(PJ_TOP_T1, ILLUMINATE(tc), ta & 0x7F);
			}
			else
			{
				set_tile1(PJ_TOP1, tc & 0x3F, ta & 0x7F);
				set_tile1(PJ_TOP_T1, tc & 0x3F, ta & 0x7F);
			}

			if (mask & PJ_T_WALLF)
			{
				mask &= ~(PJ_T_WALLF);
			}
			else
			{
				mask |= PJ_T_WALLB;
				set_tile1(PJ_WALLB, tc & 0x3F, ta & 0x7F);
			}

			/* Hack - check for "blank floor" */
			if (floor_blank1)
			{
				mask &= ~(PJ_T_FLOOR1);
			}

			if (floor_blank2)
			{
				mask &= ~(PJ_T_FLOOR2);
			}
		}
	}

	/* Draw the overlay block */
	draw_block(table, pj_table1, mask, td);

	/* Copy to screen */
	XPutImage(Metadpy->dpy, td->win->win,
	  	     clr[0]->gc,
    	     td->SkewImage,
			 0, 0, xp, yp,
		     P_TILE_SIZE / 2, P_TILE_SIZE);

	return (0);
}


/* Macro used to set the tile information */
#define set_tile2(N, X, Y) table[N] = &(((byte *)td->tiles->data)\
	[(t_offsetx2[N] + (X) * P_TILE_SIZE\
		+ (t_offsety2[N] + (Y) * P_TILE_SIZE) * 32 * P_TILE_SIZE)\
			 * bytes_per_pixel])

/* Macro used to set the tile (font-based) information */
#define set_font2(N, Y, X) table[N] = &(((byte *)font_data->data)\
	[(t_offsetx2[N] + (X) * P_TILE_SIZE\
		+ (t_offsety2[N] + (Y) * P_TILE_SIZE) * 32 * P_TILE_SIZE)\
		 * bytes_per_pixel])


static errr draw_rect_t2(int x, int y, term_data *td, int xp, int yp)
{
	term_win *window = td->t.scr;

	/* Locations of tiles in bitmap */
	void *table[PJ_MAX];

	/* List of possibly visible tiles */
	u16b mask = 0;

	/* The tile attr / char pairs */
	byte a;
	char c;
	byte ta;
	char tc;

	bool floor_blank = FALSE;

	/* Look to see if we are already drawn */
	int cur_col = x / 16;
	u32b row_mask = (1L << (x % 16 + 16));

	/* Paranoia */
	if ((x < -1) || (y < -1) || (x >= td->t.wid) || (y >= td->t.hgt))
	{
		return (0);
	}

	if (y == pj_cur_row)
	{
		/* Are we drawn? */
		if (pj_row1[cur_col] & row_mask) return (0);

		/* Not drawn - mark we are drawn */
		pj_row1[cur_col] |= row_mask;
	}
	else if (y == pj_cur_row - 1)
	{
		/* Are we drawn? */
		if (pj_row2[cur_col] & row_mask) return (0);

		/* Not drawn - mark we are drawn */
		pj_row2[cur_col] |= row_mask;
	}
	/* This should never happen */
#if 0
	else
	{
		/* Clear the arrays used for optimisation */
		(void)C_WIPE(pj_row1, 64, u32b);
		(void)C_WIPE(pj_row2, 64, u32b);

		/* We are at row y */
		pj_cur_row = y;
	}
#endif /* 0 */

	if ((x >= 0) && (y >= 0))
	{
		/* Get tiles we need */
		a = window->a[y][x];
		c = window->c[y][x];
		ta = window->ta[y][x];
		tc = window->tc[y][x];

		/* Are we overlaying anything? */
		if ((a != ta) || (c != tc) || !(a & 0x80))
		{
			mask |= PJ_T_OVER1;

			if (a & 0x80)
			{
				set_tile2(PJ_OVER1, c & 0x3F, a & 0x7F);
			}
			else
			{
				set_font2(PJ_OVER1, c & 0x7F, a & 0x1F);
			}
		}

		/* Solid wall? */
		else if (tc & 0x40)
		{
			mask |= (PJ_T_WALLF);

			set_tile2(PJ_WALLF, tc & 0x3F, ta & 0x7F);
		}

		/* Is the terrain null? */
		if ((ta & 0x80) && !(tc & 0x40))
		{
			mask |= PJ_T_FLOOR1;
			set_tile2(PJ_FLOOR1, tc & 0x3F, ta & 0x7F);

			/* Blank floor? */
			if (!((tc & 0x3F) || (ta & 0x7F))) floor_blank = TRUE;
		}
	}

	/* Check to the left */
	if ((x > 0) && (y < td->t.hgt - 1))
	{
		/* Get tiles we need */
		a = window->a[y + 1][x - 1];
		c = window->c[y + 1][x - 1];
		ta = window->ta[y + 1][x - 1];
		tc = window->tc[y + 1][x - 1];

		/* Are we overlaying anything? */
		if ((a != ta) || (c != tc) || !(a & 0x80))
		{
			mask |= PJ_T_OVER2;

			if (a & 0x80)
			{
				set_tile2(PJ_OVER2, c & 0x3F, a & 0x7F);
			}
			else
			{
				set_font2(PJ_OVER2, c & 0x7F, a & 0x1F);
			}
		}

		/* Solid wall? */
		else if (tc & 0x40)
		{
			mask |= (PJ_T_WALL2 | PJ_T_WALL2_T | PJ_T_TOP2 | PJ_T_TOP_T2);
			set_tile2(PJ_WALL2, tc & 0x3F, ta & 0x7F);
			set_tile2(PJ_WALL2_T, tc & 0x3F, ta & 0x7F);

			if (tc & 0x80)
			{
				/* Special lighting for walls */
				set_tile2(PJ_TOP2, ILLUMINATE(tc), ta & 0x7F);
				set_tile2(PJ_TOP_T2, ILLUMINATE(tc), ta & 0x7F);
			}
			else
			{
				set_tile2(PJ_TOP2, tc & 0x3F, ta & 0x7F);
				set_tile2(PJ_TOP_T2, tc & 0x3F, ta & 0x7F);
			}

			/* Hack - check for "blank floor" */
			if (floor_blank)
			{
				mask &= ~(PJ_T_FLOOR1);
			}
		}
	}

	/* Check below */
	if ((x >= 0) && (y < td->t.hgt - 1))
	{
		/* Get tiles we need */
		a = window->a[y + 1][x];
		c = window->c[y + 1][x];
		ta = window->ta[y + 1][x];
		tc = window->tc[y + 1][x];

		/* Are we overlaying anything? */
		if ((a != ta) || (c != tc) || !(a & 0x80))
		{
			mask |= PJ_T_OVER4;

			if (a & 0x80)
			{
				set_tile2(PJ_OVER4, c & 0x3F, a & 0x7F);
			}
			else
			{
				set_font2(PJ_OVER4, c & 0x7F, a & 0x1F);
			}
		}

		/* Solid wall? */
		else if (tc & 0x40)
		{
			mask |= PJ_T_TOP1 | PJ_T_TOP_T1;

			if (tc & 0x80)
			{
				/* Special lighting for walls */
				set_tile2(PJ_TOP1, ILLUMINATE(tc), ta & 0x7F);
				set_tile2(PJ_TOP_T1, ILLUMINATE(tc), ta & 0x7F);
			}
			else
			{
				set_tile2(PJ_TOP1, tc & 0x3F, ta & 0x7F);
				set_tile2(PJ_TOP_T1, tc & 0x3F, ta & 0x7F);
			}

			/* Hack - check for "blank floor" */
			if (floor_blank)
			{
				mask &= ~(PJ_T_FLOOR1);
			}

			if (mask & PJ_T_WALL2)
			{
				mask &= ~(PJ_T_WALL2 | PJ_T_WALL2_T);
			}
			else
			{
				mask |= (PJ_T_WALL2 | PJ_T_WALL2_T);
				set_tile2(PJ_WALL2, tc & 0x3F, ta & 0x7F);
				set_tile2(PJ_WALL2_T, tc & 0x3F, ta & 0x7F);
			}

			if (mask & PJ_T_WALLF)
			{
				mask &= (~PJ_T_WALLF);
			}
			else
			{
				mask |= PJ_T_WALLB;
				set_tile2(PJ_WALLB, tc & 0x3F, ta & 0x7F);
			}
		}
	}

	/* Draw the overlay block */
	draw_block(table, pj_table2, mask, td);

	/* Copy to screen */
	XPutImage(Metadpy->dpy, td->win->win,
	  	     clr[0]->gc,
    	     td->SkewImage,
			 0, 0, xp, yp,
		     P_TILE_SIZE / 2, P_TILE_SIZE);

	return (0);
}


static errr Term_skew_xpj(int x, int y, int n,
                          const byte *ap, const char *cp,
                          const byte *tap, const char *tcp)
{
	int i, xp, yp;

	term_data *td = (term_data*)(Term->data);

	/* Hack - ignore some of the parameters */
	(void)ap;
	(void)cp;
	(void)tap;
	(void)tcp;

	xp = x * P_TILE_SIZE + y * P_TILE_SIZE / 2 + Infowin->ox;
	yp = (y + 1) * P_TILE_SIZE + Infowin->oy;

	/* Draw left 8x16 bock */
	draw_rect_t1(x, y, td, xp, yp);

	/* Draw center 8x16 block */
	draw_rect_t2(x, y, td, xp + P_TILE_SIZE / 2, yp);

	/* Draw upper left */
	draw_rect_t2(x, y - 1, td, xp, yp - P_TILE_SIZE);

	/* Draw upper center */
	draw_rect_t1(x + 1, y - 1, td, xp + P_TILE_SIZE / 2, yp - P_TILE_SIZE);

	/* Draw the middle section */
	for (i = 0; i < n - 1; i++)
	{
		x++;
		xp += P_TILE_SIZE;

		/* Draw end 8x16 block */
		draw_rect_t1(x, y, td, xp, yp);

		/* Draw center 8x16 block */
		draw_rect_t2(x, y, td, xp + P_TILE_SIZE / 2, yp);

		/* Draw upper right */
		draw_rect_t2(x, y - 1, td, xp, yp - P_TILE_SIZE);

		/* Draw upper center */
		draw_rect_t1(x + 1, y - 1, td, xp + P_TILE_SIZE / 2, yp - P_TILE_SIZE);
	}

	x++;
	xp += P_TILE_SIZE;

	/* Draw end 8x16 block */
	draw_rect_t1(x, y, td, xp, yp);

	/* Draw upper right */
	draw_rect_t2(x, y - 1, td, xp, yp - P_TILE_SIZE);

	/* Done */
	return (0);
}

/*
 * Draw some graphical characters.
 */
static errr Term_pict_xpj(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	int i, x1, y1;

	byte a;
	char c;

	byte wid, hgt;

	byte ta;
	char tc;

	int x2, y2;
	int k,l;

	Pixell pixel;

	term_data *td = (term_data*)(Term->data);

	hgt = Infofnt->hgt;
	wid = Infofnt->wid;

	y *= hgt;
	x *= wid;

	/* Add in affect of window boundaries */
	y += Infowin->oy;
	x += Infowin->ox;

	for (i = 0; i < n; ++i)
	{
		a = *ap++;
		c = *cp++;

		/* For extra speed - cache these values */
		x1 = (c&0x7F) * wid;
		y1 = (a&0x7F) * hgt;

		ta = *tap++;
		tc = *tcp++;

		/* For extra speed - cache these values */
		x2 = (tc&0x7F) * wid;
		y2 = (ta&0x7F) * hgt;

		/* Optimise the common case */
		if ((x1 == x2) && (y1 == y2))
		{
			/* Draw object / terrain */
			XPutImage(Metadpy->dpy, td->win->win,
		  	        clr[0]->gc,
		  	        td->tiles,
		  	        x1, y1,
		  	        x, y,
		  	        wid, hgt);
		}
		else
		{
			for (k = 0; k < wid; k++)
			{
				for (l = 0; l < hgt; l++)
				{
					/* If mask set... */
					if ((pixel = XGetPixel(td->tiles, x1 + k, y1 + l))
						 == pix_blank)
					{
						/* Output from the terrain */
						pixel = XGetPixel(td->tiles, x2 + k, y2 + l);
					}

					/* Store into the temp storage. */
					XPutPixel(td->TmpImage, k, l, pixel);
				}
			}


			/* Draw to screen */

			XPutImage(Metadpy->dpy, td->win->win,
		    	      clr[0]->gc,
		     	     td->TmpImage,
		     	     0, 0, x, y,
		     	     wid, hgt);
		}

		x += wid;
	}

	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_xpj(int x, int y, int n)
{
#if 0
	/* Erase (use black) */
	Infoclr_set(clr[TERM_DARK]);

	/* Mega-Hack -- Erase some space */
	Infofnt_text_non(x, y, "", n);

#endif /* 0 */

	byte dummy[3] = {0x80, 0x80, 0x80};
	int i;

	for (i = 0; i < n; i++)
	{
		/* Mega-hack */
		Term_pict_xpj(x, y, 1, &dummy[1], (char *) &dummy[1],
			 &dummy[1], (char *) &dummy[1]);
	}

	/* Success */
	return (0);
}


/* Load the font data */
static XImage *ReadFONT(Display *dpy, char *Name, u16b size)
{
	Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));

	int depth = DefaultDepth(dpy, DefaultScreen(dpy));

	FILE *fp;

	XImage *Res = NULL;

	Pixell pixel;

	char *Data;
	char buf[1024];

	char line[16];

	char tmp_string[100];

	int i, j, k, m, total;
	int num = 0, error_idx = -1;

	/* Determine total bytes needed for image */
	total = 32 * size * 128 * size * bytes_per_pixel;


	/* Open the BMP file */
	fp = fopen(Name, "r");

	/* No such file */
	if (fp == NULL)
	{
		return (NULL);
	}

	/* Allocate image memory */
	C_MAKE(Data, total, char);

	Res = XCreateImage(dpy, visual, depth, ZPixmap, 0,
	                   Data, size * 32, size * 128,
	                   32, 0);

	/* Failure */
	if (Res == NULL)
	{
		KILL(Data);
		fclose(fp);

		return (NULL);
	}

	/* Reset the counters for use in parsing the font data */
	i = 0;
	j = 16;

	/* Process the file */
	while (0 == my_fgets(fp, buf, sizeof(buf)))
	{
		/* Count lines */
		num++;

		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace((unsigned char)buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;

		/* Look at the line */

		/* Verify correct "colon" format */
		if (buf[1] != ':')
		{
			sprintf(tmp_string, "Incorrect font file format on line %d", num);
			quit(tmp_string);
		}

		/* Get number */
		if (buf[0] == 'N')
		{
			/* Get the index */
			i = atoi(buf+2);

			/* Verify information */
			if (i <= error_idx)
			{
				sprintf(tmp_string,
					"Incorrect font file numbering on line %d", num);
				quit(tmp_string);
			}

			error_idx = i;

			/* Verify information */
			if (i >= 128)
			{
				sprintf(tmp_string,
					"Incorrect font file numbering on line %d", num);
				quit(tmp_string);
			}

			/* Verify information */
			if (j != size)
			{
				sprintf(tmp_string,
					"Incorrect font size on line %d", num);
				quit(tmp_string);
			}

			/* Start from the top */
			j = 0;
		}

		/* Get font data */
		if (buf[0] == 'F')
		{
			/* Verify information */
			if (j >= size)
			{
				sprintf(tmp_string,
					"Incorrect font size length on line %d", num);
				quit(tmp_string);
			}

			if (((int) strlen(buf)) != size + 2)
			{
				sprintf(tmp_string,
					"Incorrect font size width on line %d", num);
				quit(tmp_string);
			}

			/* Create the line */
			for (k = 0; k < size; k++)
			{
				line[k] = buf[k + 2];
			}

			/* Copy it to the 32 coloured locations */
			for (k = 0; k < 32; k++)
			{
				pixel = clr[k]->fg;

				for (m = 0; m < size; m++)
				{
					if (line[m] == '*')
					{
						/* Coloured pixel */
						XPutPixel(Res, k * size + m, i * size + j, pixel);
					}
					else
					{
						/* Blank pixel */
						XPutPixel(Res, k * size + m, i * size + j, 0);
					}
				}
			}

			/* Next line of the character */
			j++;
		}
	}

	/* Close the file */
	my_fclose(fp);

	/* Paranoia - does the file end early? */
	if ((i != 127) || (j != size)) return (NULL);

	return (Res);
}


/*
 * Initialize a term_data
 */
static errr term_data_init(term_data *td, int i)
{
	term *t = &td->t;

	cptr name = angband_term_name[i];

	cptr font;

	int x = 0;
	int y = 0;

	int cols = 80;
	int rows = 24;

	int ox = 1;
	int oy = 1;

	int wid, hgt, num;

	char buf[80];

	cptr str;

	int val;

	XClassHint *ch;

	char res_name[20];
	char res_class[20];

	XSizeHints *sh;

	/* Get default font for this term */
	font = get_default_font(i);

	/* Window specific location (x) */
	sprintf(buf, "ANGBAND_X11_AT_X_%d", i);
	str = getenv(buf);
	x = (str != NULL) ? atoi(str) : -1;

	/* Window specific location (y) */
	sprintf(buf, "ANGBAND_X11_AT_Y_%d", i);
	str = getenv(buf);
	y = (str != NULL) ? atoi(str) : -1;

	/* Window specific cols */
	sprintf(buf, "ANGBAND_X11_COLS_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) cols = val;

	/* Window specific rows */
	sprintf(buf, "ANGBAND_X11_ROWS_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) rows = val;

	/* Hack the main window must be at least 80x24 */
	if (!i)
	{
		if (cols < 80) cols = 80;
		if (rows < 24) rows = 24;
	}

	/* Window specific inner border offset (ox) */
	sprintf(buf, "ANGBAND_X11_IBOX_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) ox = val;

	/* Window specific inner border offset (oy) */
	sprintf(buf, "ANGBAND_X11_IBOY_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) oy = val;


	/* Prepare the standard font */
	MAKE(td->fnt, infofnt);
	Infofnt_set(td->fnt);
	Infofnt_init_data(font);

	/* Hack -- key buffer size */
	num = ((i == 0) ? 1024 : 16);

	if (i == 0)
	{
		wid = (cols + 1) * P_TILE_SIZE + rows * P_TILE_SIZE / 2 + ox * 2;
		hgt = (rows + 1) * P_TILE_SIZE + oy * 2;
	}
	else
	{
		/* Assume full size windows */
		wid = cols * td->fnt->wid + (ox + ox);
		hgt = rows * td->fnt->hgt + (oy + oy);
	}

	/* Create a top-window */
	MAKE(td->win, infowin);
	Infowin_set(td->win);
	Infowin_init_top(x, y, wid, hgt, 0,
	                 Metadpy->fg, Metadpy->bg);

	/* Ask for certain events */
	Infowin_set_mask(ExposureMask | StructureNotifyMask | KeyPressMask);

	/* Set the window name */
	Infowin_set_name(name);

	/* Save the inner border */
	Infowin->ox = ox;
	Infowin->oy = oy;

	/* Make Class Hints */
	ch = XAllocClassHint();

	if (ch == NULL) quit("XAllocClassHint failed");

	my_strcpy(res_name, name, sizeof(res_name));
	res_name[0] = FORCELOWER(res_name[0]);
	ch->res_name = res_name;

	strcpy(res_class, "Angband");
	ch->res_class = res_class;

	XSetClassHint(Metadpy->dpy, Infowin->win, ch);

	/* Make Size Hints */
	sh = XAllocSizeHints();

	/* Oops */
	if (sh == NULL) quit("XAllocSizeHints failed");

	/* Main window has a differing minimum size */
	if (i == 0)
	{
		sh->flags = PMinSize | PMaxSize;
		sh->min_height = (24 + 1) * P_TILE_SIZE + oy * 2;
		sh->min_width = (80 + 1) * P_TILE_SIZE + ox * 2;
		sh->max_height = (255 + 1) * P_TILE_SIZE + oy * 2;
		sh->max_width = (255 + 1) * P_TILE_SIZE + ox * 2 + 255 * P_TILE_SIZE / 2;
		/* Resize increment */
		sh->flags |= PResizeInc;
		sh->width_inc = P_TILE_SIZE * 3 / 2;
		sh->height_inc = P_TILE_SIZE;
	}

	/* Other windows can be shrunk to 1x1 */
	else
	{
		/* Other windows */
		sh->flags = PMinSize | PMaxSize;
		sh->min_width = td->fnt->wid + ox * 2;
		sh->min_height = td->fnt->hgt + oy * 2;
		sh->max_width = 255 * td->fnt->wid + ox * 2;
		sh->max_height = 255 * td->fnt->hgt + oy * 2;

		/* Resize increment */
		sh->flags |= PResizeInc;
		sh->width_inc = td->fnt->wid;
		sh->height_inc = td->fnt->hgt;
	}



	/* Base window size */
	sh->flags |= PBaseSize;
	sh->base_width = (ox + ox);
	sh->base_height = (oy + oy);

	/* Use the size hints */
	XSetWMNormalHints(Metadpy->dpy, Infowin->win, sh);

	/* Map the window */
	Infowin_map();


	/* Move the window to requested location */
	if ((x >= 0) && (y >= 0)) Infowin_impell(x, y);


	/* Initialize the term */
	term_init(t, cols, rows, num);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Hooks */
	t->xtra_hook = Term_xtra_xpj;
	t->curs_hook = Term_curs_xpj;
	t->wipe_hook = Term_wipe_xpj;
	t->text_hook = Term_text_xpj;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/* Success */
	return (0);
}


const char help_xpj[] = "X11 Projected View, subopts -d<display> -n<windows>"
#ifdef USE_GRAPHICS
                " -s(moothRescale)"
#endif
                ;


/*
 * Initialization function for an "XPJ" module to Angband
 */
errr init_xpj(int argc, char **argv)
{
	int i, j;

	cptr dpy_name = "";

	int num_term = 3;

	char filename[1024];

	int pict_wid = 0;
	int pict_hgt = 0;

	char *TmpData;

	/* Load graphics */
	Display *dpy;

	XImage *tiles_raw;
	XImage *font_raw;

	/* Check tile size */
	if (P_TILE_SIZE % 4)
	{
		quit("Need to compile with P_TILE_SIZE as a multiple of four.");
	}

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-d"))
		{
			dpy_name = &argv[i][2];
			continue;
		}

		if (prefix(argv[i], "-s"))
		{
			smoothRescaling = FALSE;
			continue;
		}

		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}


	/* Init the Metadpy if possible */
	if (Metadpy_init_name(dpy_name)) return (-1);


	/* Prepare cursor color */
	MAKE(xor, infoclr);
	Infoclr_set(xor);
	Infoclr_init_ppn(Metadpy->fg, Metadpy->bg, "xor", 0);


	/* Prepare normal colors */
	for (i = 0; i < 256; ++i)
	{
		Pixell pixel;

		MAKE(clr[i], infoclr);

		Infoclr_set(clr[i]);

		/* Acquire Angband colors */
		color_table[i][0] = angband_color_table[i][0];
		color_table[i][1] = angband_color_table[i][1];
		color_table[i][2] = angband_color_table[i][2];
		color_table[i][3] = angband_color_table[i][3];

		/* Default to monochrome */
		pixel = ((i == 0) ? Metadpy->bg : Metadpy->fg);

		/* Handle color */
		if (Metadpy->color)
		{
			/* Create pixel */
			pixel = create_pixel(Metadpy->dpy,
			                     color_table[i][1],
			                     color_table[i][2],
			                     color_table[i][3]);
		}

		/* Initialize the color */
		Infoclr_init_ppn(pixel, Metadpy->bg, "cpy", 0);
	}


	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;
	}

	/* Raise the "Angband" window */
	Infowin_set(data[0].win);
	Infowin_raise();

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Try the "16x16.bmp" file */
	path_build(filename, sizeof(filename), ANGBAND_DIR_XTRA, "graf/16x16.bmp");

	/* Use the "16x16.bmp" file if it exists */
	if (0 == fd_close(fd_open(filename, O_RDONLY)))
	{
		if (arg_graphics)
		{
			/* Use graphics */
			use_graphics = TRUE;

			/* And use tiles */
			ANGBAND_GRAF = "new";
		}
		else
		{
			/* Use graphics */
			use_graphics = TRUE;
			arg_graphics = TRUE;

			/* But not for monsters / items */
			ANGBAND_GRAF = "none";
		}

		use_transparency = TRUE;

		pict_wid = pict_hgt = 16;


	}
	else
	{
		quit("Could not initialise graphics!");
	}

	dpy = Metadpy->dpy;

	/* Load the graphical tiles */
	tiles_raw = ReadBMP(dpy, filename);

	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		term *t = &td->t;

		/* Use graphics sometimes */
		t->higher_pict = TRUE;

		if (i == 0)
		{
			/* Graphics hook */
			t->pict_hook = Term_skew_xpj;

			/* Always use graphics */
			t->always_pict = TRUE;

			/* Resize tiles */
			td->tiles =
				ResizeImage(dpy, tiles_raw,
				pict_wid, pict_hgt,
				P_TILE_SIZE, P_TILE_SIZE);
		}
		else
		{
			/* Graphics hook */
			t->pict_hook = Term_pict_xpj;

			/* Resize tiles */
			td->tiles =
				ResizeImage(dpy, tiles_raw,
				pict_wid, pict_hgt,
				td->fnt->wid, td->fnt->hgt);
		}
	}

	/* Initialize the transparency masks */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];
		int ii, jj;
		int depth = DefaultDepth(dpy, DefaultScreen(dpy));
		Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));
		int total;


		/* Determine total bytes needed for image */
		ii = 1;
		jj = (depth - 1) >> 2;
		while (jj >>= 1) ii <<= 1;

		if (i == 0)
		{
			total = P_TILE_SIZE * P_TILE_SIZE * ii;
		}
		else
		{
			total = td->fnt->wid * td->fnt->hgt * ii;
		}

		/* Save number of bytes per pixel */
		bytes_per_pixel = ii;

		switch (bytes_per_pixel)
		{
			case 1:
			{
				draw_block = draw_block8;

				/* Mega Hack^2 - assume the top left corner is "black" */
				pix_blank = ((byte *) data[0].tiles->data)
					[6 * P_TILE_SIZE * 32];
				break;
			}
			case 2:
			{
				draw_block = draw_block16;

				/* Mega Hack^2 - assume the top left corner is "black" */
				pix_blank = ((u16b *) data[0].tiles->data)
					[6 * P_TILE_SIZE * 32];
				break;
			}
			case 4:
			{
				draw_block = draw_block32;

				/* Mega Hack^2 - assume the top left corner is "black" */
				pix_blank = ((u32b *) data[0].tiles->data)
					[6 * P_TILE_SIZE * 32];
				break;
			}
			default:
			{
				quit("Unsupported bytes per pixel format of screen");
				break;
			}
		}


		TmpData = (char *)malloc(total);

		if (i == 0)
		{
			/* Normal tiles */
			td->TmpImage = XCreateImage(dpy,visual,depth,
				ZPixmap, 0, TmpData,
				P_TILE_SIZE, P_TILE_SIZE, 32, 0);

			/* Skewed tiles */
			TmpData = (char *)malloc(total / 2);

			td->SkewImage = XCreateImage(dpy, visual, depth,
				ZPixmap, 0, TmpData,
				P_TILE_SIZE / 2, P_TILE_SIZE, 32, 0);
		}
		else
		{
			td->TmpImage = XCreateImage(dpy, visual, depth,
				ZPixmap, 0, TmpData,
				td->fnt->wid, td->fnt->hgt, 32, 0);
		}

	}

	/* Free tiles_raw? XXX XXX */

	/*
	 * Precalculate the tables used to draw the tiles
	 *
	 * This caches the position of each possibility of
	 * where tiles can be. By masking the value in the table,
	 * you can quickly determine what to draw.
	 */
	for (i = 0; i < P_TILE_SIZE / 2; i++)
	{
		for (j = 0; j < P_TILE_SIZE; j++)
		{
			/* Table 1 */
			pj_table1[j][i] = PJ_T_WALLF;

			if ((j + i) % 2)
			{
				pj_table1[j][i] |= PJ_T_TOP1;
				pj_table1[j][i] |= PJ_T_WALLB;
			}
			else
			{
				pj_table1[j][i] |= PJ_T_TOP_T1;
			}

			if (j >= P_TILE_SIZE / 2)
			{
				pj_table1[j][i] |= PJ_T_OVER2;
			}
			else if (i < P_TILE_SIZE / 4)
			{
				pj_table1[j][i] |= PJ_T_OVER3;
			}
			else
			{
				pj_table1[j][i] |= PJ_T_OVER1;
			}

			if (i - j / 2 >= 0)
			{
				pj_table1[j][i] |= PJ_T_FLOOR1;

				if ((i + j) % 2)
				{
					pj_table1[j][i] |= PJ_T_WALL1;
				}
				else
				{
					pj_table1[j][i] |= PJ_T_WALL1_T;
				}
			}
			else
			{
				pj_table1[j][i] |= PJ_T_FLOOR2;
			}


			/* Table 2 */
			pj_table2[j][i] = PJ_T_FLOOR1 | PJ_T_WALLF;

			if ((j + i) % 2)
			{
				pj_table2[j][i] |= PJ_T_WALLB;
			}

			if (j < P_TILE_SIZE / 2)
			{
				pj_table2[j][i] |= PJ_T_OVER1;
			}
			else if (i < P_TILE_SIZE / 4)
			{
				pj_table2[j][i] |= PJ_T_OVER2;
			}
			else
			{
				pj_table2[j][i] |= PJ_T_OVER4;
			}

			if (i - j / 2 >= 0)
			{
				if ((i + j) % 2)
				{
					pj_table2[j][i] |= PJ_T_TOP1;
				}
				else
				{
					pj_table2[j][i] |= PJ_T_TOP_T1;
				}
			}
			else
			{
				if ((i + j) % 2)
				{
					pj_table2[j][i] |= PJ_T_TOP2 | PJ_T_WALL2;
				}
				else
				{
					pj_table2[j][i] |= PJ_T_TOP_T2 | PJ_T_WALL2_T;
				}
			}
		}
	}

	/* Clear the arrays used for optimisation */
	(void)C_WIPE(pj_row1, 64, u32b);
	(void)C_WIPE(pj_row2, 64, u32b);

	/* Hack - use crazy row to mark "nothing entered yet" */
	pj_cur_row = -255;

	/* Try the "16x16.bmp" file */
	path_build(filename, sizeof(filename), ANGBAND_DIR_XTRA, "font/16x16.txt");

	/* Use the "16x16.bmp" file if it exists */
	if (0 == fd_close(fd_open(filename, O_RDONLY)))
	{
		Display *dpy = Metadpy->dpy;

		/* Load the graphical tiles */
		font_raw = ReadFONT(dpy, filename, 16);

		if (!font_raw) quit("Could not allocate font metrics!");

		/* Hack - Resize font */
		font_data = ResizeImage(dpy, font_raw,
			16, 16,
			P_TILE_SIZE, P_TILE_SIZE);
	}
	else
	{
		quit("Could not initialise font metrics!");
	}

	/* Success */
	return (0);
}

#endif /* USE_XPJ */
