/*
 * File: main-x11.c
 * Purpose: Provide support for the X Windowing System
 *
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "buildid.h"

/*
 * This file helps Angband work with UNIX/X11 computers.
 *
 * To use this file, compile with "USE_X11" defined, and link against all
 * the various "X11" libraries which may be needed.
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
 */

/*
 * The following shell script can be used to launch Angband, assuming that
 * it was extracted into "~/Angband", and compiled using "USE_X11", on a
 * Linux machine, with a 1280x1024 screen, using 6 windows (with the given
 * characteristics), with gamma correction of 1.8 -> (1 / 1.8) * 256 = 142,
 * and without graphics (add "-g" for graphics).  Just copy this comment
 * into a file, remove the leading " * " characters (and the head/tail of
 * this comment), and make the file executable.
 *
 *
 * #!/bin/csh
 *
 * # Describe attempt
 * echo "Launching angband..."
 * sleep 2
 *
 * # Main window
 * setenv ANGBAND_X11_FONT_0 10x20
 * setenv ANGBAND_X11_AT_X_0 5
 * setenv ANGBAND_X11_AT_Y_0 510
 *
 * # Message window
 * setenv ANGBAND_X11_FONT_1 8x13
 * setenv ANGBAND_X11_AT_X_1 5
 * setenv ANGBAND_X11_AT_Y_1 22
 * setenv ANGBAND_X11_ROWS_1 35
 *
 * # Inventory window
 * setenv ANGBAND_X11_FONT_2 8x13
 * setenv ANGBAND_X11_AT_X_2 635
 * setenv ANGBAND_X11_AT_Y_2 182
 * setenv ANGBAND_X11_ROWS_2 23
 *
 * # Equipment window
 * setenv ANGBAND_X11_FONT_3 8x13
 * setenv ANGBAND_X11_AT_X_3 635
 * setenv ANGBAND_X11_AT_Y_3 22
 * setenv ANGBAND_X11_ROWS_3 12
 *
 * # Monster recall window
 * setenv ANGBAND_X11_FONT_4 6x13
 * setenv ANGBAND_X11_AT_X_4 817
 * setenv ANGBAND_X11_AT_Y_4 847
 * setenv ANGBAND_X11_COLS_4 76
 * setenv ANGBAND_X11_ROWS_4 11
 *
 * # Object recall window
 * setenv ANGBAND_X11_FONT_5 6x13
 * setenv ANGBAND_X11_AT_X_5 817
 * setenv ANGBAND_X11_AT_Y_5 520
 * setenv ANGBAND_X11_COLS_5 76
 * setenv ANGBAND_X11_ROWS_5 24
 *
 * # The build directory
 * cd ~/Angband
 *
 * # Gamma correction
 * setenv ANGBAND_X11_GAMMA 142
 *
 * # Launch Angband
 * ./src/angband -mx11 -- -n6 &
 *
 */



#ifdef USE_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "main.h"

#ifndef IsModifierKey

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 * These were stolen from one of the X11 header files
 */

#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#define IsCursorKey(keysym) \
  (((unsigned)(keysym) >= XK_Home)     && ((unsigned)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_F1)    && ((unsigned)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_F1)       && ((unsigned)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_Select)   && ((unsigned)(keysym) <  XK_KP_Space))

#define IsModifierKey(keysym) \
  (((unsigned)(keysym) >= XK_Shift_L)  && ((unsigned)(keysym) <= XK_Hyper_R))

#endif /* IsModifierKey */


/*
 * Checks if the keysym is a special key or a normal key
 * Assume that XK_MISCELLANY keysyms are special
 */
#define IsSpecialKey(keysym) \
  ((unsigned)(keysym) >= 0xFF00)


/*
 * Hack -- avoid some compiler warnings
 */
#define IGNORE_UNUSED_FUNCTIONS


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

	unsigned int width;
	unsigned int height;
	unsigned int depth;

	Pixell black;
	Pixell white;

	Pixell bg;
	Pixell fg;
	Pixell zg;

	unsigned int mono:1;
	unsigned int color:1;
	unsigned int nuke:1;
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
 *	- The saved (startup) location of the window
 *	- The width, height of the window
 *	- The border width of this window
 *
 *	- Byte: 1st Extra byte
 *
 *	- Bit Flag: This window is currently Mapped
 *	- Bit Flag: This window needs to be redrawn
 *	- Bit Flag: This window has been resized
 *
 *	- Bit Flag: We should nuke 'win' when done with it
 *
 *	- Bit Flag: 1st extra flag
 *	- Bit Flag: 2nd extra flag
 *	- Bit Flag: 3rd extra flag
 *	- Bit Flag: 4th extra flag
 */
struct infowin
{
	Window win;
	long mask;

	s16b ox, oy;

	s16b x, y;
	s16b x_save, y_save;
	s16b w, h;
	u16b b;

	byte byte1;

	unsigned int mapped:1;
	unsigned int redraw:1;
	unsigned int resize:1;

	unsigned int nuke:1;

	unsigned int flag1:1;
	unsigned int flag2:1;
	unsigned int flag3:1;
	unsigned int flag4:1;
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

	unsigned int code:4;
	unsigned int stip:1;
	unsigned int nuke:1;
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

	const char *name;

	s16b wid;
	s16b twid;
	s16b hgt;
	s16b asc;

	byte off;

	unsigned int mono:1;
	unsigned int nuke:1;
};



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

	int tile_wid;
	int tile_wid2; /* Tile-width with bigscreen */
	int tile_hgt;

	/* Pointers to allocated data, needed to clear up memory */
	XClassHint *classh;
	XSizeHints *sizeh;
};



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


/* Errr: Expose Infowin */
#define Infowin_expose() \
	(!(Infowin->redraw = 1))

/* Errr: Unxpose Infowin */
#define Infowin_unexpose() \
	(Infowin->redraw = 0)



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


/*
 * Actual color table
 */
static infoclr *clr[MAX_COLORS];



/**** Code imported from the old maid-x11.c ****/

#ifdef SUPPORT_GAMMA
static bool gamma_table_ready = FALSE;
static int gamma_val = 0;
#endif /* SUPPORT_GAMMA */


/*
 * Hack -- Convert an RGB value to an X11 Pixel, or die.
 */
static u32b create_pixel(Display *dpy, byte red, byte green, byte blue)
{
	Colormap cmap = DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy));

	XColor xcolour;

#ifdef SUPPORT_GAMMA

	if (!gamma_table_ready)
	{
		const char *str = getenv("ANGBAND_X11_GAMMA");
		if (str != NULL) gamma_val = atoi(str);

		gamma_table_ready = TRUE;

		/* Only need to build the table if gamma exists */
		if (gamma_val) build_gamma_table(gamma_val);
	}

	/* Hack -- Gamma Correction */
	if (gamma_val > 0)
	{
		red = gamma_table[red];
		green = gamma_table[green];
		blue = gamma_table[blue];
	}

#endif /* SUPPORT_GAMMA */

	/* Build the color */

	xcolour.red   = red * 257;
	xcolour.green = green * 257;
	xcolour.blue  = blue * 257;
	xcolour.flags = DoRed | DoGreen | DoBlue;

	/* Attempt to Allocate the Parsed color */
	if (!(XAllocColor(dpy, cmap, &xcolour)))
	{
		quit_fmt("Couldn't allocate bitmap color #%04x%04x%04x\n",
		         xcolour.red, xcolour.green, xcolour.blue);
	}

	return (xcolour.pixel);
}


/*
 * Get the name of the default font to use for the term.
 */
static const char *get_default_font(int term_num)
{
	const char *font;
	char buf[80];

	/* Window specific font name */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_FONT_%d", term_num);

	/* Check environment for that font */
	font = getenv(buf);
	if (font) return font;

	/* Check environment for "base" font */
	font = getenv("ANGBAND_X11_FONT");
	if (font) return font;

	switch (term_num)
	{
		case 0:
			return DEFAULT_X11_FONT_0;
		case 1:
			return DEFAULT_X11_FONT_1;
		case 2:
			return DEFAULT_X11_FONT_2;
		case 3:
			return DEFAULT_X11_FONT_3;
		case 4:
			return DEFAULT_X11_FONT_4;
		case 5:
			return DEFAULT_X11_FONT_5;
		case 6:
			return DEFAULT_X11_FONT_6;
		case 7:
			return DEFAULT_X11_FONT_7;
	}

	return DEFAULT_X11_FONT;
}



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
static errr Metadpy_init_2(Display *dpy, const char *name)
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
 * Nuke the current metadpy
 */
static errr Metadpy_nuke(void)
{
	metadpy *m = Metadpy;


	/* If required, Free the Display */
	if (m->nuke)
	{
		/* Close the Display */
		XCloseDisplay(m->dpy);

		/* Forget the Display */
		m->dpy = (Display*)(NULL);

		/* Do not nuke it again */
		m->nuke = 0;
	}

	/* Return Success */
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
static errr Infowin_set_name(const char *name)
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


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Set the icon name of Infowin
 */
static errr Infowin_set_icon_name(const char *name)
{
	Status st;
	XTextProperty tp;
	char buf[128];
	char *bp = buf;
	my_strcpy(buf, name, sizeof(buf));
	st = XStringListToTextProperty(&bp, 1, &tp);
	if (st) XSetWMIconName(Metadpy->dpy, Infowin->win, &tp);
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Nuke Infowin
 */
static errr Infowin_nuke(void)
{
	infowin *iwin = Infowin;

	/* Nuke if requested */
	if (iwin->nuke)
	{
		/* Destory the old window */
		XDestroyWindow(Metadpy->dpy, iwin->win);
	}

	/* Success */
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
	iwin->x_save = x;
	iwin->y_save = y;
	iwin->w = w;
	iwin->h = h;
	iwin->b = b;

	/* Check Error XXX Extract some more ACTUAL data */
	XGetWindowAttributes(Metadpy->dpy, xid, &xwa);

	/* Apply the above info */
	iwin->mask = xwa.your_event_mask;
	iwin->mapped = ((xwa.map_state == IsUnmapped) ? 0 : 1);

	/* And assume that we are exposed */
	iwin->redraw = 1;

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Initialize a new 'infowin'.
 */
static errr Infowin_init_real(Window xid)
{
	/* Wipe it clean */
	(void)WIPE(Infowin, infowin);

	/* Start out non-nukable */
	Infowin->nuke = 0;

	/* Attempt to Prepare ourself */
	return (Infowin_prepare(xid));
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


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
	if (dad == None)
		dad = Metadpy->root;

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


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Request that Infowin be unmapped
 */
static errr Infowin_unmap(void)
{
	/* Execute the Un-Mapping */
	XUnmapWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


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


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Request that Infowin be lowered
 */
static errr Infowin_lower(void)
{
	/* Lower towards invisibility */
	XLowerWindow(Metadpy->dpy, Infowin->win);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


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


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Move and Resize an infowin
 */
static errr Infowin_locate(int x, int y, int w, int h)
{
	/* Execute the request */
	XMoveResizeWindow(Metadpy->dpy, Infowin->win, x, y, w, h);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


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


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Visually Paint Infowin with the current color
 */
static errr Infowin_fill(void)
{
	/* Execute the request */
	XFillRectangle(Metadpy->dpy, Infowin->win, Infoclr->gc,
	               0, 0, Infowin->w, Infowin->h);

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * A NULL terminated pair list of legal "operation names"
 *
 * Pairs of values, first is texttual name, second is the string
 * holding the decimal value that the operation corresponds to.
 */
static const char *opcode_pairs[] =
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
static int Infoclr_Opcode(const char *str)
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


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Request a Pixell by name.  Note: uses 'Metadpy'.
 *
 * Inputs:
 *      name: The name of the color to try to load (see below)
 *
 * Output:
 *	The Pixell value that metched the given name
 *	'Metadpy->fg' if the name was unparseable
 *
 * Valid forms for 'name':
 *	'fg', 'bg', 'zg', '<name>' and '#<code>'
 */
static Pixell Infoclr_Pixell(const char *name)
{
	XColor scrn;

	/* Attempt to Parse the name */
	if (name && name[0])
	{
		/* The 'bg' color is available */
		if (streq(name, "bg")) return (Metadpy->bg);

		/* The 'fg' color is available */
		if (streq(name, "fg")) return (Metadpy->fg);

		/* The 'zg' color is available */
		if (streq(name, "zg")) return (Metadpy->zg);

		/* The 'white' color is available */
		if (streq(name, "white")) return (Metadpy->white);

		/* The 'black' color is available */
		if (streq(name, "black")) return (Metadpy->black);

		/* Attempt to parse 'name' into 'scrn' */
		if (!(XParseColor(Metadpy->dpy, Metadpy->cmap, name, &scrn)))
		{
			plog_fmt("Warning: Couldn't parse color '%s'\n", name);
		}

		/* Attempt to Allocate the Parsed color */
		if (!(XAllocColor(Metadpy->dpy, Metadpy->cmap, &scrn)))
		{
			plog_fmt("Warning: Couldn't allocate color '%s'\n", name);
		}

		/* The Pixel was Allocated correctly */
		else return (scrn.pixel);
	}

	/* Warn about the Default being Used */
	plog_fmt("Warning: Using 'fg' for unknown color '%s'\n", name);

	/* Default to the 'Foreground' color */
	return (Metadpy->fg);
}


/*
 * Initialize a new 'infoclr' with a real GC.
 */
static errr Infoclr_init_1(GC gc)
{
	infoclr *iclr = Infoclr;

	/* Wipe the iclr clean */
	(void)WIPE(iclr, infoclr);

	/* Assign the GC */
	iclr->gc = gc;

	/* Success */
	return (0);
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Nuke an old 'infoclr'.
 */
static errr Infoclr_nuke(void)
{
	infoclr *iclr = Infoclr;

	/* Deal with 'GC' */
	if (iclr->nuke)
	{
		/* Free the GC */
		XFreeGC(Metadpy->dpy, iclr->gc);
	}

	/* Forget the current */
	Infoclr = (infoclr*)(NULL);

	/* Success */
	return (0);
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
 * Nuke an old 'infofnt'.
 */
static errr Infofnt_nuke(void)
{
	infofnt *ifnt = Infofnt;

	/* Deal with 'name' */
	if (ifnt->name)
	{
		/* Free the name */
		string_free((void *) ifnt->name);
	}

	/* Nuke info if needed */
	if (ifnt->nuke)
	{
		/* Free the font */
		XFreeFont(Metadpy->dpy, ifnt->info);
	}

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
	ifnt->twid = cs->width;

	/* Success */
	return (0);
}


#ifndef IGNORE_UNUSED_FUNCTIONS

/*
 * Initialize a new 'infofnt'.
 */
static errr Infofnt_init_real(XFontStruct *info)
{
	/* Wipe the thing */
	(void)WIPE(Infofnt, infofnt);

	/* No nuking */
	Infofnt->nuke = 0;

	/* Attempt to prepare it */
	return (Infofnt_prepare(info));
}

#endif /* IGNORE_UNUSED_FUNCTIONS */


/*
 * Init an infofnt by its Name
 *
 * Inputs:
 *	name: The name of the requested Font
 */
static errr Infofnt_init_data(const char *name)
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

	/* HACK - force all fonts to be printed character by character */
	Infofnt->mono = 1;

	/* Success */
	return (0);
}


/*
 * Standard Text
 */
static errr Infofnt_text_std(int x, int y, const char *str, int len)
{
	int i;
	int w, h;

	term_data *td = (term_data*)(Term->data);

	/*** Do a brief info analysis ***/

	/* Do nothing if the string is null */
	if (!str || !*str) return (-1);

	/* Get the length of the string */
	if (len < 0) len = strlen(str);

	/*** Decide where to place the string, vertically ***/

	/* Ignore Vertical Justifications */
	y = (y * td->tile_hgt) + Infowin->oy;


	/*** Decide where to place the string, horizontally ***/

	/* Line up with x at left edge of column 'x' */
	x = (x * td->tile_wid) + Infowin->ox;


	/*** Erase the background ***/

	/* The total width will be 'len' chars * standard width */
	w = len * td->tile_wid;

	/* Simply do 'td->tile_hgt' (a single row) high */
	h = td->tile_hgt;

	/* Fill the background */
	XFillRectangle(Metadpy->dpy, Infowin->win, clr[TERM_DARK]->gc, x, y, w, h);


	/*** Actually draw 'str' onto the infowin ***/

	/* Be sure the correct font is ready */
	XSetFont(Metadpy->dpy, Infoclr->gc, Infofnt->info->fid);


	y += Infofnt->asc;


	/*** Handle the fake mono we can enforce on fonts ***/

	/* Monotize the font */
	if (Infofnt->mono)
	{
		/* Do each character */
		for (i = 0; i < len; ++i)
		{
			/* Note that the Infoclr is set up to contain the Infofnt */
			XDrawImageString(Metadpy->dpy, Infowin->win, Infoclr->gc,
			                 x + i * td->tile_wid + Infofnt->off, y, str + i, 1);
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


/*
 * Painting where text would be
 */
static errr Infofnt_text_non(int x, int y, const char *str, int len)
{
	int w, h;

	term_data *td = (term_data*)(Term->data);

	/*** Find the width ***/

	/* Negative length is a flag to count the characters in str */
	if (len < 0) len = strlen(str);

	/* The total width will be 'len' chars * standard width */
	w = len * td->tile_wid;


	/*** Find the X dimensions ***/

	/* Line up with x at left edge of column 'x' */
	x = x * td->tile_wid + Infowin->ox;


	/*** Find other dimensions ***/

	/* Simply do 'td->tile_hgt' (a single row) high */
	h = td->tile_hgt;

	/* Simply do "at top" in row 'y' */
	y = y * h + Infowin->oy;


	/*** Actually 'paint' the area ***/

	/* Just do a Fill Rectangle */
	XFillRectangle(Metadpy->dpy, Infowin->win, Infoclr->gc, x, y, w, h);

	/* Success */
	return (0);
}



/*************************************************************************/


/*
 * Angband specific code follows... (ANGBAND)
 */


/*
 * Hack -- cursor color
 */
static infoclr *xor;


/*
 * Color info (unused, red, green, blue).
 */
static byte color_table_x11[MAX_COLORS][4];


/*
 * The number of term data structures
 */
#define MAX_TERM_DATA 8

/*
 * The array of term data structures
 */
static term_data data[MAX_TERM_DATA];


/*
 * Path to the X11 settings file
 */
static const char *x11_prefs = "x11-settings.prf";
char settings[1024];



/*
 * Remember the number of terminal windows open
 */
static int term_windows_open;



/*
 * Process a keypress event
 */
static void react_keypress(XKeyEvent *ev)
{
	int n, ch = 0;

	KeySym ks;

	char buf[128];

	/* Extract four "modifier flags" */
	int mc = (ev->state & ControlMask) ? TRUE : FALSE;
	int ms = (ev->state & ShiftMask) ? TRUE : FALSE;
	int mo = (ev->state & Mod1Mask) ? TRUE : FALSE;
	int mx = (ev->state & Mod2Mask) ? TRUE : FALSE;
	int kp = FALSE;

	byte mods = (mo ? KC_MOD_ALT : 0) | (mx ? KC_MOD_META : 0);

	/* Check for "normal" keypresses */
	n = XLookupString(ev, buf, 125, &ks, NULL);
	buf[n] = '\0';

	/* Ignore modifier keys by themselves */
	if (IsModifierKey(ks)) return;

	switch (ks) {
		case XK_BackSpace: ch = KC_BACKSPACE; break;
		case XK_Tab: ch = KC_TAB; break;
		case XK_Return: ch = KC_ENTER; break;
		case XK_Escape: ch = ESCAPE; break;

		case XK_Delete: ch = KC_DELETE; break;
		case XK_Home: ch = KC_HOME; break;
		case XK_Left: ch = ARROW_LEFT; break;
		case XK_Up: ch = ARROW_UP; break;
		case XK_Right: ch = ARROW_RIGHT; break;
		case XK_Down: ch = ARROW_DOWN; break;
		case XK_Page_Up: ch = KC_PGUP; break;
		case XK_Page_Down: ch = KC_PGDOWN; break;
		case XK_End: ch = KC_END; break;
		case XK_Insert: ch = KC_INSERT; break;
		case XK_Pause: ch = KC_PAUSE; break;
		case XK_Break: ch = KC_BREAK; break;

		/* keypad */
		case XK_KP_0: ch = '0'; kp = TRUE; break;
		case XK_KP_1: ch = '1'; kp = TRUE; break;
		case XK_KP_2: ch = '2'; kp = TRUE; break;
		case XK_KP_3: ch = '3'; kp = TRUE; break;
		case XK_KP_4: ch = '4'; kp = TRUE; break;
		case XK_KP_5: ch = '5'; kp = TRUE; break;
		case XK_KP_6: ch = '6'; kp = TRUE; break;
		case XK_KP_7: ch = '7'; kp = TRUE; break;
		case XK_KP_8: ch = '8'; kp = TRUE; break;
		case XK_KP_9: ch = '9'; kp = TRUE; break;

		case XK_KP_Decimal: ch = '.'; kp = TRUE; break;
		case XK_KP_Divide: ch = '/'; kp = TRUE; break;
		case XK_KP_Multiply: ch = '*'; kp = TRUE; break;
		case XK_KP_Subtract: ch = '-'; kp = TRUE; break;
		case XK_KP_Add: ch = '+'; kp = TRUE; break;
		case XK_KP_Enter: ch = '\n'; kp = TRUE; break;
		case XK_KP_Equal: ch = '='; kp = TRUE; break;

		case XK_KP_Delete: ch = KC_DELETE; kp = TRUE; break;
		case XK_KP_Home: ch = KC_HOME; kp = TRUE; break;
		case XK_KP_Left: ch = ARROW_LEFT; kp = TRUE; break;
		case XK_KP_Up: ch = ARROW_UP; kp = TRUE; break;
		case XK_KP_Right: ch = ARROW_RIGHT; kp = TRUE; break;
		case XK_KP_Down: ch = ARROW_DOWN; kp = TRUE; break;
		case XK_KP_Page_Up: ch = KC_PGUP; kp = TRUE; break;
		case XK_KP_Page_Down: ch = KC_PGDOWN; kp = TRUE; break;
		case XK_KP_End: ch = KC_END; kp = TRUE; break;
		case XK_KP_Insert: ch = KC_INSERT; kp = TRUE; break;
		case XK_KP_Begin: ch = KC_BEGIN; kp = TRUE; break;

		case XK_F1: ch = KC_F1; break;
		case XK_F2: ch = KC_F2; break;
		case XK_F3: ch = KC_F3; break;
		case XK_F4: ch = KC_F4; break;
		case XK_F5: ch = KC_F5; break;
		case XK_F6: ch = KC_F6; break;
		case XK_F7: ch = KC_F7; break;
		case XK_F8: ch = KC_F8; break;
		case XK_F9: ch = KC_F9; break;
		case XK_F10: ch = KC_F10; break;
		case XK_F11: ch = KC_F11; break;
		case XK_F12: ch = KC_F12; break;
		case XK_F13: ch = KC_F13; break;
		case XK_F14: ch = KC_F14; break;
		case XK_F15: ch = KC_F15; break;
	}

	if (kp) mods |= KC_MOD_KEYPAD;

	if (ch) {
		if (mc) mods |= KC_MOD_CONTROL;
		if (ms) mods |= KC_MOD_SHIFT;
		Term_keypress(ch, mods);
		return;
	} else if (n && !IsSpecialKey(ks)) {
		keycode_t code = buf[0];

		if (mc && MODS_INCLUDE_CONTROL(code)) mods |= KC_MOD_CONTROL;
		if (ms && MODS_INCLUDE_SHIFT(code)) mods |= KC_MOD_SHIFT;

		Term_keypress(code, mods);
	}
}


/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int * const x, int * const y,
                            const int ox, const int oy)
{
	term_data *td = (term_data*)(Term->data);

	(*x) = (ox - Infowin->ox) / td->tile_wid;
	(*y) = (oy - Infowin->oy) / td->tile_hgt;
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

	int i;
	int window = 0;

	/* Do not wait unless requested */
	if (!wait && !XPending(Metadpy->dpy)) return (1);

	/* Wait in 0.02s increments while updating animations every 0.2s */
	i = 0;
	while (!XPending(Metadpy->dpy))
	{
		if (i == 0) idle_update();
		usleep(20000);
		i = (i + 1) % 10;
	}

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
		case ButtonPress:
		{
			bool press = (xev->type == ButtonPress);

			int z = 0;

			/* Where is the mouse */
			int x = xev->xbutton.x;
			int y = xev->xbutton.y;

			/* Which button is involved */
			if (xev->xbutton.button == Button1) z = 1;
			else if (xev->xbutton.button == Button2) z = 2;
			else if (xev->xbutton.button == Button3) z = 3;
			else if (xev->xbutton.button == Button4) z = 4;
			else if (xev->xbutton.button == Button5) z = 5;
			else z = 0;

			/* The co-ordinates are only used in Angband format. */
			pixel_to_square(&x, &y, x, y);
			if (press) Term_mousepress(x, y, z);

			break;
		}

		case KeyPress:
		{
			/* Hack -- use "old" term */
			Term_activate(&old_td->t);

			/* Process the key */
			react_keypress(&(xev->xkey));

			break;
		}

		case Expose:
		{
			int x1, x2, y1, y2;

			x1 = (xev->xexpose.x - Infowin->ox) / td->tile_wid;
			x2 = (xev->xexpose.x + xev->xexpose.width - Infowin->ox) / td->tile_wid;

			y1 = (xev->xexpose.y - Infowin->oy) / td->tile_hgt;
			y2 = (xev->xexpose.y + xev->xexpose.height - Infowin->oy) / td->tile_hgt;

			Term_redraw_section(x1, y1, x2, y2);

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
                        int cols, rows, wid, hgt, force_resize;

			int ox = Infowin->ox;
			int oy = Infowin->oy;

			/* Save the new Window Parms */
			Infowin->x = xev->xconfigure.x;
			Infowin->y = xev->xconfigure.y;
			Infowin->w = xev->xconfigure.width;
			Infowin->h = xev->xconfigure.height;

			/* Determine "proper" number of rows/cols */
			cols = ((Infowin->w - (ox + ox)) / td->tile_wid);
			rows = ((Infowin->h - (oy + oy)) / td->tile_hgt);

			/* Hack -- minimal size */
			if (cols < 1) cols = 1;
			if (rows < 1) rows = 1;

			if (window == 0)
			{
				/* Hack the main window must be at least 80x24 */
                                force_resize = FALSE;
                                if (cols < 80) { cols = 80; force_resize = TRUE; }
				if (rows < 24) { rows = 24; force_resize = TRUE; }

                                /* Resize the windows if any "change" is needed */
                                if (force_resize)
                                  {
			/* Desired size of window */
			wid = cols * td->tile_wid + (ox + ox);
			hgt = rows * td->tile_hgt + (oy + oy);

				/* Resize window */
				Infowin_set(td->win);
				Infowin_resize(wid, hgt);
			}
			}

			/* Resize the Term (if needed) */
			(void)Term_resize(cols, rows);

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
static errr Term_xtra_x11_level(int v)
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
static errr Term_xtra_x11_react(void)
{
	int i;

	if (Metadpy->color)
	{
		/* Check the colors */
		for (i = 0; i < MAX_COLORS; i++)
		{
			if ((color_table_x11[i][0] != angband_color_table[i][0]) ||
				(color_table_x11[i][1] != angband_color_table[i][1]) ||
				(color_table_x11[i][2] != angband_color_table[i][2]) ||
				(color_table_x11[i][3] != angband_color_table[i][3]))
			{
				Pixell pixel;

				/* Save new values */
				color_table_x11[i][0] = angband_color_table[i][0];
				color_table_x11[i][1] = angband_color_table[i][1];
				color_table_x11[i][2] = angband_color_table[i][2];
				color_table_x11[i][3] = angband_color_table[i][3];

				/* Create pixel */
				pixel = create_pixel(Metadpy->dpy,
									 color_table_x11[i][1],
									 color_table_x11[i][2],
									 color_table_x11[i][3]);

				/* Change the foreground */
				Infoclr_set(clr[i]);
				Infoclr_change_fg(pixel);
			}
		}
	}

	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_x11(int n, int v)
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
		case TERM_XTRA_LEVEL: return (Term_xtra_x11_level(v));

		/* Clear the screen */
		case TERM_XTRA_CLEAR: Infowin_wipe(); return (0);

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
			if (v > 0) usleep(1000 * v);
			return (0);

		/* React to changes */
		case TERM_XTRA_REACT: return (Term_xtra_x11_react());
	}

	/* Unknown */
	return (1);
}


/*
 * Draw the cursor as a rectangular outline
 */
static errr Term_curs_x11(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	XDrawRectangle(Metadpy->dpy, Infowin->win, xor->gc,
			 x * td->tile_wid + Infowin->ox,
			 y * td->tile_hgt + Infowin->oy,
			 td->tile_wid - 1, td->tile_hgt - 1);

	/* Success */
	return (0);
}


/*
 * Draw the double width cursor as a rectangular outline
 */
static errr Term_bigcurs_x11(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	XDrawRectangle(Metadpy->dpy, Infowin->win, xor->gc,
			 x * td->tile_wid + Infowin->ox,
			 y * td->tile_hgt + Infowin->oy,
			 td->tile_wid2 - 1, td->tile_hgt - 1);

	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_x11(int x, int y, int n)
{
	/* Erase (use black) */
	Infoclr_set(clr[TERM_DARK]);

	/* Mega-Hack -- Erase some space */
	Infofnt_text_non(x, y, "", n);

	/* Success */
	return (0);
}


/*
 * Draw some textual characters.
 */
static errr Term_text_x11(int x, int y, int n, byte a, const char *s)
{
	/* Draw the text */
	Infoclr_set(clr[a]);

	/* Draw the text */
	Infofnt_text_std(x, y, s, n);

	/* Success */
	return (0);
}




static void save_prefs(void)
{
	ang_file *fff;
	int i;

	/* Open the settings file */
	fff = file_open(settings, MODE_WRITE, FTYPE_TEXT);
	if (!fff) return;

	/* Header */
	file_putf(fff, "# %s X11 settings\n\n", VERSION_NAME);

	/* Number of term windows to open */
	file_putf(fff, "TERM_WINS=%d\n\n", term_windows_open);

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		if (!td->t.mapped_flag) continue;

		/* Header */
		file_putf(fff, "# Term %d\n", i);

		/*
		 * This doesn't seem to work under various WMs
		 * since the decoration messes the position up
		 *
		 * Hack -- Use saved window positions.
		 * This means that we won't remember ingame repositioned
		 * windows, but also means that WMs won't screw predefined
		 * positions up. -CJN-
		 */

		/* Window specific location (x) */
		file_putf(fff, "AT_X_%d=%d\n", i, td->win->x_save);

		/* Window specific location (y) */
		file_putf(fff, "AT_Y_%d=%d\n", i, td->win->y_save);

		/* Window specific cols */
		file_putf(fff, "COLS_%d=%d\n", i, td->t.wid);

		/* Window specific rows */
		file_putf(fff, "ROWS_%d=%d\n", i, td->t.hgt);

		/* Window specific inner border offset (ox) */
		file_putf(fff, "IBOX_%d=%d\n", i, td->win->ox);

		/* Window specific inner border offset (oy) */
		file_putf(fff, "IBOY_%d=%d\n", i, td->win->oy);

		/* Window specific font name */
		file_putf(fff, "FONT_%d=%s\n", i, td->fnt->name);

		/* Window specific tile width */
		file_putf(fff, "TILE_WIDTH_%d=%d\n", i, td->tile_wid);

		/* Window specific tile height */
		file_putf(fff, "TILE_HEIGHT_%d=%d\n", i, td->tile_hgt);

		/* Footer */
		file_putf(fff, "\n");
	}

	/* Close */
	file_close(fff);
}


/*
 * Initialize a term_data
 */
static errr term_data_init(term_data *td, int i)
{
	term *t = &td->t;

	const char *name = angband_term_name[i];

	const char *font;

	int x = 0;
	int y = 0;

	int cols = 80;
	int rows = 24;

	int ox = 1;
	int oy = 1;

	int wid, hgt, num;

	const char *str;

	int val;

	XClassHint *ch;

	char res_name[20];
	char res_class[20];

	XSizeHints *sh;

	ang_file *fff;

	char buf[1024];
	char cmd[40];
	char font_name[256];

	int line = 0;

	/* Get default font for this term */
	font = get_default_font(i);

	/* Open the file */
	fff = file_open(settings, MODE_READ, -1);

	/* File exists */
	if (fff)
	{
		/* Process the file */
		while (file_getl(fff, buf, sizeof(buf)))
		{
			/* Count lines */
			line++;

			/* Skip "empty" lines */
			if (!buf[0]) continue;

			/* Skip "blank" lines */
			if (isspace((unsigned char)buf[0])) continue;

			/* Skip comments */
			if (buf[0] == '#') continue;

			/* Window specific location (x) */
			strnfmt(cmd, sizeof(cmd), "AT_X_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				x = (str != NULL) ? atoi(str + 1) : -1;
				continue;
			}

			/* Window specific location (y) */
			strnfmt(cmd, sizeof(cmd), "AT_Y_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				y = (str != NULL) ? atoi(str + 1) : -1;
				continue;
			}

			/* Window specific cols */
			strnfmt(cmd, sizeof(cmd), "COLS_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) cols = val;
				continue;
			}

			/* Window specific rows */
			strnfmt(cmd, sizeof(cmd), "ROWS_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) rows = val;
				continue;
			}

			/* Window specific inner border offset (ox) */
			strnfmt(cmd, sizeof(cmd), "IBOX_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) ox = val;
				continue;
			}

			/* Window specific inner border offset (oy) */
			strnfmt(cmd, sizeof(cmd), "IBOY_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) oy = val;
				continue;
			}

			/* Window specific font name */
			strnfmt(cmd, sizeof(cmd), "FONT_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				if (str != NULL)
				{
					my_strcpy(font_name, str + 1, sizeof(font_name));
					font = font_name;
				}
				continue;
			}

			/* Window specific tile width */
			strnfmt(cmd, sizeof(cmd), "TILE_WIDTH_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) td->tile_wid = val;
				continue;
			}

			/* Window specific tile height */
			strnfmt(cmd, sizeof(cmd), "TILE_HEIGHT_%d", i);

			if (prefix(buf, cmd))
			{
				str = strstr(buf, "=");
				val = (str != NULL) ? atoi(str + 1) : -1;
				if (val > 0) td->tile_hgt = val;
				continue;
			}
		}

		/* Close */
		file_close(fff);
	}

	/*
	 * Env-vars overwrite the settings in the settings file
	 */

	/* Window specific location (x) */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_AT_X_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) x = val;

	/* Window specific location (y) */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_AT_Y_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) y = val;

	/* Window specific cols */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_COLS_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) cols = val;

	/* Window specific rows */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_ROWS_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) rows = val;

	/* Window specific inner border offset (ox) */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_IBOX_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) ox = val;

	/* Window specific inner border offset (oy) */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_IBOY_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) oy = val;

	/* Window specific font name */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_FONT_%d", i);
	str = getenv(buf);
	if (str) font = str;

	/* Hack the main window must be at least 80x24 */
	if (!i)
	{
		if (cols < 80) cols = 80;
		if (rows < 24) rows = 24;
	}

	/* Prepare the standard font */
	td->fnt = ZNEW(infofnt);
	Infofnt_set(td->fnt);
	if (Infofnt_init_data(font)) quit_fmt("Couldn't load the requested font. (%s)", font);

	/* Use proper tile size */
	if (td->tile_wid <= 0) td->tile_wid = td->fnt->twid;
	if (td->tile_hgt <= 0) td->tile_hgt = td->fnt->hgt;

	/* Don't allow bigtile mode - one day maybe NRM */
	td->tile_wid2 = td->tile_wid;

	/* Hack -- key buffer size */
	num = ((i == 0) ? 1024 : 16);

	/* Assume full size windows */
	wid = cols * td->tile_wid + (ox + ox);
	hgt = rows * td->tile_hgt + (oy + oy);

	/* Create a top-window */
	td->win = ZNEW(infowin);
	Infowin_set(td->win);
	Infowin_init_top(x, y, wid, hgt, 0,
	                 Metadpy->fg, Metadpy->bg);

	/* Ask for certain events */
	Infowin_set_mask(ExposureMask | StructureNotifyMask | KeyPressMask
			 | ButtonPressMask);

	/* Set the window name */
	Infowin_set_name(name);

	/* Save the inner border */
	Infowin->ox = ox;
	Infowin->oy = oy;

	/* Make Class Hints */
	ch = XAllocClassHint();

	if (ch == NULL) quit("XAllocClassHint failed");

	my_strcpy(res_name, name, sizeof(res_name));
	res_name[0] = tolower((unsigned char)res_name[0]);
	ch->res_name = res_name;

	my_strcpy(res_class, "Angband", sizeof(res_class));
	ch->res_class = res_class;

	XSetClassHint(Metadpy->dpy, Infowin->win, ch);

	/* Make Size Hints */
	sh = XAllocSizeHints();

	/* Oops */
	if (sh == NULL) quit("XAllocSizeHints failed");

	if (x || y)
		sh->flags = USPosition;
	else
		sh->flags = 0;

	/* Main window has a differing minimum size */
	if (i == 0)
	{
		/* Main window min size is 80x24 */
		sh->flags |= (PMinSize | PMaxSize);
		sh->min_width = 80 * td->tile_wid + (ox + ox);
		sh->min_height = 24 * td->tile_hgt + (oy + oy);
		sh->max_width = 255 * td->tile_wid + (ox + ox);
		sh->max_height = 255 * td->tile_hgt + (oy + oy);
	}

	/* Other windows can be shrunk to 1x1 */
	else
	{
		/* Other windows */
		sh->flags |= (PMinSize | PMaxSize);
		sh->min_width = td->tile_wid + (ox + ox);
		sh->min_height = td->tile_hgt + (oy + oy);
		sh->max_width = 255 * td->tile_wid + (ox + ox);
		sh->max_height = 255 * td->tile_hgt + (oy + oy);
	}

	/* Resize increment */
	sh->flags |= PResizeInc;
	sh->width_inc = td->tile_wid;
	sh->height_inc = td->tile_hgt;

	/* Base window size */
	sh->flags |= PBaseSize;
	sh->base_width = (ox + ox);
	sh->base_height = (oy + oy);

	/* Use the size hints */
	XSetWMNormalHints(Metadpy->dpy, Infowin->win, sh);

	/* Map the window */
	Infowin_map();

	/* Set pointers to allocated data */
	td->sizeh = sh;
	td->classh = ch;

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
	t->xtra_hook = Term_xtra_x11;
	t->curs_hook = Term_curs_x11;
	t->bigcurs_hook = Term_bigcurs_x11;
	t->wipe_hook = Term_wipe_x11;
	t->text_hook = Term_text_x11;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/* Success */
	return (0);
}


const char help_x11[] = "Basic X11, subopts -d<display> -n<windows> -x<file>";

static void hook_quit(const char *str)
{
	int i;

	/* Unused */
	(void)str;

	save_prefs();

	/* Free allocated data */
	for (i = 0; i < term_windows_open; i++)
	{
		term_data *td = &data[i];
		term *t = &td->t;

		/* Free size hints */
		XFree(td->sizeh);

		/* Free class hints */
		XFree(td->classh);

		/* Free fonts */
		Infofnt_set(td->fnt);
		(void)Infofnt_nuke();
		FREE(td->fnt);

		/* Free window */
		Infowin_set(td->win);
		(void)Infowin_nuke();
		FREE(td->win);

		/* Free term */
		(void)term_nuke(t);
	}

	/* Free colors */
	Infoclr_set(xor);
	(void)Infoclr_nuke();
	FREE(xor);

	for (i = 0; i < MAX_COLORS; ++i)
	{
		Infoclr_set(clr[i]);
		(void)Infoclr_nuke();
		FREE(clr[i]);
	}

	/* Close link to display */
	(void)Metadpy_nuke();
}


/*
 * Initialization function for an "X11" module to Angband
 */
errr init_x11(int argc, char **argv)
{
	int i;

	const char *dpy_name = "";

	int num_term = -1;

	ang_file *fff;

	char buf[1024];
	const char *str;
	int val;
	int line = 0;

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-d"))
		{
			dpy_name = &argv[i][2];
			continue;
		}

		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		if (prefix(argv[i], "-x"))
		{
			x11_prefs = argv[i] + 2;
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}


	if (num_term == -1)
	{
		num_term = 1;

		/* Build the filename */
		(void)path_build(settings, sizeof(settings), ANGBAND_DIR_USER, "x11-settings.prf");

		/* Open the file */
		fff = file_open(settings, MODE_READ, -1);

		/* File exists */
		if (fff)
		{
			/* Process the file */
			while (file_getl(fff, buf, sizeof(buf)))
			{
				/* Count lines */
				line++;
	
				/* Skip "empty" lines */
				if (!buf[0]) continue;
	
				/* Skip "blank" lines */
				if (isspace((unsigned char)buf[0])) continue;
	
				/* Skip comments */
				if (buf[0] == '#') continue;
	
				/* Number of terminal windows */
				if (prefix(buf, "TERM_WINS"))
				{
					str = strstr(buf, "=");
					val = (str != NULL) ? atoi(str + 1) : -1;
					if (val > 0) num_term = val;
					continue;
				}
			}
	
			/* Close */
			(void)file_close(fff);
		}
	}


	/* Init the Metadpy if possible */
	if (Metadpy_init_name(dpy_name)) return (-1);

	/* Remember the number of terminal windows */
	term_windows_open = num_term;

	/* Prepare cursor color */
	xor = ZNEW(infoclr);
	Infoclr_set(xor);
	Infoclr_init_ppn(Metadpy->fg, Metadpy->bg, "xor", 0);


	/* Prepare normal colors */
	for (i = 0; i < 256; ++i)
	{
		Pixell pixel;

		clr[i] = ZNEW(infoclr);

		Infoclr_set(clr[i]);

		/* Acquire Angband colors */
		color_table_x11[i][0] = angband_color_table[i][0];
		color_table_x11[i][1] = angband_color_table[i][1];
		color_table_x11[i][2] = angband_color_table[i][2];
		color_table_x11[i][3] = angband_color_table[i][3];

		/* Default to monochrome */
		pixel = ((i == 0) ? Metadpy->bg : Metadpy->fg);

		/* Handle color */
		if (Metadpy->color)
		{
			/* Create pixel */
			pixel = create_pixel(Metadpy->dpy,
								 color_table_x11[i][1],
								 color_table_x11[i][2],
								 color_table_x11[i][3]);
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


	/* Activate hook */
	quit_aux = hook_quit;

	/* Success */
	return (0);
}

#endif /* USE_X11 */


