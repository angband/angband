/* File: main-x11.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: One (awful) way to run Angband under X11 -BEN- */

/*
 * Most of this file provides a user interface package composed of
 * several pseudo-objects, including "metadpy" (a display), "infowin"
 * (a window), "infoclr" (a color), and "infofnt" (a font).  Actually,
 * the package was originally much more interesting, but I bastardized
 * it to keep this file simple.
 *
 * The rest of the file (search for 'ANGBAND') provides a simple
 * implementation of the "main-xxx.c" file for Angband using this
 * user interface package.
 */


#include "angband.h"


#ifdef USE_X11


#include "z-util.h"
#include "z-virt.h"
#include "z-form.h"


#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif /* __MAKEDEPEND__ */



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





/**** Available Types ****/

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

	s16b x, y;
	s16b w, h;
	u16b b;

	byte byte1;

	uint mapped:1;
	uint redraw:1;
	uint resize:1;

	uint nuke:1;

	uint flag1:1;
	uint flag2:1;
	uint flag3:1;
	uint flag4:1;
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



/**** Available Macros ****/


#ifndef IsModifierKey

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 * These were stolen from one of the X11 header files
 */

#define IsKeypadKey(keysym) \
    (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#define IsCursorKey(keysym) \
    (((unsigned)(keysym) >= XK_Home) && ((unsigned)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
    (((unsigned)(keysym) >= XK_KP_F1) && ((unsigned)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
    (((unsigned)(keysym) >= XK_F1) && ((unsigned)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
    (((unsigned)(keysym) >= XK_Select) && ((unsigned)(keysym) <  XK_KP_Space))

#define IsModifierKey(keysym) \
    (((unsigned)(keysym) >= XK_Shift_L) && ((unsigned)(keysym) <= XK_Hyper_R))

#endif


/*
 * Checks if the keysym is a special key or a normal key
 * Assume that XK_MISCELLANY keysyms are special
 */
#define IsSpecialKey(keysym) \
    ((unsigned)(keysym) >= 0xFF00)



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



/**** Available Globals ****/


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



/**** Available code ****/


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
		if (!dpy)
		{

#if 0

			/* No name given, extract DISPLAY */
			if (!name) name = getenv("DISPLAY");

			/* No DISPLAY extracted, use default */
			if (!name) name = "(default)";

			/* Oops */
			plog_fmt("Cannot open display '%s'", name);

#endif

			/* Error */
			return (-1);
		}

		/* We WILL have to Nuke it when done */
		m->nuke = 1;
	}

	/* Since the Display was given, use it */
	else
	{
		/* We will NOT have to Nuke it when done */
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
	m->zg = (1 << m->depth) - 1;

	/* Save various default Flag Settings */
	m->color = ((m->depth > 1) ? 1 : 0);
	m->mono = ((m->color) ? 0 : 1);

	/* Return "success" ***/
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
static errr Infowin_set_name(cptr name)
{
	Status st;
	XTextProperty tp;
	char buf[128];
	char *bp = buf;
	strcpy(buf, name);
	st = XStringListToTextProperty(&bp, 1, &tp);
	if (st) XSetWMName(Metadpy->dpy, Infowin->win, &tp);
	return (0);
}


/*
 * Set the icon name of Infowin
 */
static errr Infowin_set_icon_name(cptr name)
{
	Status st;
	XTextProperty tp;
	char buf[128];
	char *bp = buf;
	strcpy(buf, name);
	st = XStringListToTextProperty(&bp, 1, &tp);
	if (st) XSetWMIconName(Metadpy->dpy, Infowin->win, &tp);
	return (0);
}


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
	int x, y, w, h, b, d;

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

	/* And assume that we are exposed */
	iwin->redraw = 1;

	/* Success */
	return (0);
}


/*
 * Initialize a new 'infowin'.
 */
static errr Infowin_init_real(Window xid)
{
	/* Wipe it clean */
	WIPE(Infowin, infowin);

	/* Start out non-nukable */
	Infowin->nuke = 0;

	/* Attempt to Prepare ourself */
	return (Infowin_prepare(xid));
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
	WIPE(Infowin, infowin);


	/*** Error Check XXX ***/


	/*** Create the Window 'xid' from data ***/

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
 * Request that Infowin be unmapped
 */
static errr Infowin_unmap(void)
{
	/* Execute the Un-Mapping */
	XUnmapWindow(Metadpy->dpy, Infowin->win);

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
 * Request that Infowin be lowered
 */
static errr Infowin_lower(void)
{
	/* Lower towards invisibility */
	XLowerWindow(Metadpy->dpy, Infowin->win);

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
 * Move and Resize an infowin
 */
static errr Infowin_locate(int x, int y, int w, int h)
{
	/* Execute the request */
	XMoveResizeWindow(Metadpy->dpy, Infowin->win, x, y, w, h);

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


/*
 * Set the size hints of Infowin
 */
static errr Infowin_set_size(int w, int h, int r_w, int r_h, bool fixed)
{
	XSizeHints *sh;

	/* Make Size Hints */
	sh = XAllocSizeHints();

	/* Oops */
	if (!sh) return (1);

	/* Fixed window size */
	if (fixed)
	{
		sh->flags = PMinSize | PMaxSize;
		sh->min_width = sh->max_width = w;
		sh->min_height = sh->max_height = h;
	}

	/* Variable window size */
	else
	{
		sh->flags = PMinSize;
		sh->min_width = r_w + 2;
		sh->min_height = r_h + 2;
	}

	/* Standard fields */
	sh->width = w;
	sh->height = h;
	sh->width_inc = r_w;
	sh->height_inc = r_h;
	sh->base_width = 2;
	sh->base_height = 2;

	/* Useful settings */
	sh->flags |= PSize | PResizeInc | PBaseSize;

	/* Use the size hints */
	XSetWMNormalHints(Metadpy->dpy, Infowin->win, sh);

	/* Success */
	return 0;
}


/*
 * Set the name (in the title bar) of Infowin
 */
static errr Infowin_set_class_hint(cptr name)
{
	XClassHint *ch;

	char res_name[20];
	char res_class[20];

	ch = XAllocClassHint();
	if (ch == NULL) return (1);

	strcpy(res_name, name);
	res_name[0] = FORCELOWER(res_name[0]);
	ch->res_name = res_name;

	strcpy(res_class, "Angband");
	ch->res_class = res_class;

	XSetClassHint(Metadpy->dpy, Infowin->win, ch);

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
static Pixell Infoclr_Pixell(cptr name)
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
	WIPE(iclr, infoclr);

	/* Assign the GC */
	iclr->gc = gc;

	/* Success */
	return (0);
}


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
	WIPE(iclr, infoclr);

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
		string_free(ifnt->name);
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

#ifdef OBSOLETE_SIZING_METHOD
	/* Extract default sizing info */
	ifnt->asc = cs->ascent;
	ifnt->hgt = (cs->ascent + cs->descent);
	ifnt->wid = cs->width;
#endif

	/* Success */
	return (0);
}


/*
 * Initialize a new 'infofnt'.
 */
static errr Infofnt_init_real(XFontStruct *info)
{
	/* Wipe the thing */
	WIPE(Infofnt, infofnt);

	/* No nuking */
	Infofnt->nuke = 0;

	/* Attempt to prepare it */
	return (Infofnt_prepare(info));
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
	WIPE(Infofnt, infofnt);

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


	/*** Decide where to place the string, vertically ***/

	/* Ignore Vertical Justifications */
	y = (y * Infofnt->hgt) + Infofnt->asc;


	/*** Decide where to place the string, horizontally ***/

	/* Line up with x at left edge of column 'x' */
	x = (x * Infofnt->wid);


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

	/* Assume monoospaced font */
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
static errr Infofnt_text_non(int x, int y, cptr str, int len)
{
	int w, h;


	/*** Find the width ***/

	/* Negative length is a flag to count the characters in str */
	if (len < 0) len = strlen(str);

	/* The total width will be 'len' chars * standard width */
	w = len * Infofnt->wid;


	/*** Find the X dimensions ***/

	/* Line up with x at left edge of column 'x' */
	x = x * Infofnt->wid;


	/*** Find other dimensions ***/

	/* Simply do 'Infofnt->hgt' (a single row) high */
	h = Infofnt->hgt;

	/* Simply do "at top" in row 'y' */
	y = y * h;


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
 * Color table
 */
static infoclr *clr[256];

/*
 * Color info
 */
static char color_info[256][8];


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

	infowin *outer;
	infowin *inner;
};


/*
 * The number of term data structures
 */
#define MAX_TERM_DATA 8

/*
 * The array of term data structures
 */
static term_data data[MAX_TERM_DATA];


/*
 * Process a keypress event
 */
static void react_keypress(XEvent *xev)
{
	int i, n, mc, ms, mo, mx;

	uint ks1;

	XKeyEvent *ev = (XKeyEvent*)(xev);

	KeySym ks;

	char buf[128];
	char msg[128];


	/* Check for "normal" keypresses */
	n = XLookupString(ev, buf, 125, &ks, NULL);

	/* Terminate */
	buf[n] = '\0';

	/* Hack -- convert into an unsigned int */
	ks1 = (uint)(ks);

	/* Extract four "modifier flags" */
	mc = (ev->state & ControlMask) ? TRUE : FALSE;
	ms = (ev->state & ShiftMask) ? TRUE : FALSE;
	mo = (ev->state & Mod1Mask) ? TRUE : FALSE;
	mx = (ev->state & Mod2Mask) ? TRUE : FALSE;


	/* Hack -- Ignore "modifier keys" */
	if (IsModifierKey(ks)) return;


	/* Normal keys with no modifiers */
	if (n && !mo && !mx && !IsSpecialKey(ks))
	{
		/* Enqueue the normal key(s) */
		for (i = 0; buf[i]; i++) Term_keypress(buf[i]);

		/* All done */
		return;
	}


	/* Handle a few standard keys */
	switch (ks1)
	{
		case XK_Escape:
		Term_keypress(ESCAPE); return;

		case XK_Return:
		Term_keypress('\r'); return;

		case XK_Tab:
		Term_keypress('\t'); return;

		case XK_Delete:
		case XK_BackSpace:
		Term_keypress('\010'); return;
	}


#if 0
	/* Hack -- Handle a few special KeySym codes */
	switch (ks1)
	{
		case XK_Up:
		Term_keypress(30); Term_keypress('8'); return;

		case XK_Down:
		Term_keypress(30); Term_keypress('2'); return;

		case XK_Left:
		Term_keypress(30); Term_keypress('4'); return;

		case XK_Right:
		Term_keypress(30); Term_keypress('6'); return;
	}
#endif


	/* Hack -- Use the KeySym */
	if (ks)
	{
		sprintf(msg, "%c%s%s%s%s_%lX%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        (unsigned long)(ks), 13);
	}

	/* Hack -- Use the Keycode */
	else
	{
		sprintf(msg, "%c%s%s%s%sK_%X%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        ev->keycode, 13);
	}

	/* Enqueue the "fake" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);


	/* Hack -- dump an "extra" string */
	if (n)
	{
		/* Start the "extra" string */
		Term_keypress(28);

		/* Enqueue the "real" string */
		for (i = 0; buf[i]; i++) Term_keypress(buf[i]);

		/* End the "extra" string */
		Term_keypress(28);
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

	int flag = 0;

	int i, x, y, z;


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
		/* Inner window */
		if (xev->xany.window == data[i].inner->win)
		{
			td = &data[i];
			iwin = td->inner;
			break;
		}

		/* Outer window */
		if (xev->xany.window == data[i].outer->win)
		{
			td = &data[i];
			iwin = td->outer;
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
		/* A Button Press Event */
		case ButtonPress:
		{
			/* Set flag, then fall through */
			flag = 1;
		}

		/* A Button Release (or ButtonPress) Event */
		case ButtonRelease:
		{
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

		/* An Enter Event */
		case EnterNotify:
		{
			/* Note the Enter, Fall into 'Leave' */
			flag = 1;
		}

		/* A Leave (or Enter) Event */
		case LeaveNotify:
		{
			/* Where is the mouse */
			x = xev->xcrossing.x;
			y = xev->xcrossing.y;

			/* XXX Handle */

			break;
		}

		/* A Motion Event */
		case MotionNotify:
		{
			/* Where is the mouse */
			x = xev->xmotion.x;
			y = xev->xmotion.y;

			/* XXX Handle */

			break;
		}

		/* A KeyRelease */
		case KeyRelease:
		{
			/* Nothing */
			break;
		}

		/* A KeyPress */
		case KeyPress:
		{
			/* Save the mouse location */
			x = xev->xkey.x;
			y = xev->xkey.y;

			/* Hack -- use "old" term */
			Term_activate(&old_td->t);

			/* Process the key */
			react_keypress(xev);

			break;
		}

		/* An Expose Event */
		case Expose:
		{
			/* Ignore "extra" exposes */
			if (xev->xexpose.count) break;

			/* Clear the window */
			Infowin_wipe();

			/* Redraw (if allowed) */
			if (iwin == td->inner) Term_redraw();

			break;
		}

		/* A Mapping Event */
		case MapNotify:
		{
			Infowin->mapped = 1;
			break;
		}

		/* An UnMap Event */
		case UnmapNotify:
		{
			/* Save the mapped-ness */
			Infowin->mapped = 0;
			break;
		}

		/* A Move AND/OR Resize Event */
		case ConfigureNotify:
		{
			int x1, y1, w1, h1;
			int cols, rows, wid, hgt;

			/* Save the Old information */
			x1 = Infowin->x;
			y1 = Infowin->y;
			w1 = Infowin->w;
			h1 = Infowin->h;

			/* Save the new Window Parms */
			Infowin->x = xev->xconfigure.x;
			Infowin->y = xev->xconfigure.y;
			Infowin->w = xev->xconfigure.width;
			Infowin->h = xev->xconfigure.height;

			/* Detemine "proper" number of rows/cols */
			cols = ((Infowin->w - 2) / td->fnt->wid);
			rows = ((Infowin->h - 2) / td->fnt->hgt);

			/* Hack -- do not allow resize of main screen */
			if (td == &data[0]) cols = 80;
			if (td == &data[0]) rows = 24;

			/* Hack -- minimal size */
			if (cols < 1) cols = 1;
			if (rows < 1) rows = 1;

			/* Desired size of "outer" window */
			wid = cols * td->fnt->wid;
			hgt = rows * td->fnt->hgt;

			/* Resize the windows if any "change" is needed */
			if ((Infowin->w != wid + 2) || (Infowin->h != hgt + 2))
			{
				Infowin_set(td->outer);
				Infowin_resize(wid + 2, hgt + 2);
				Infowin_set(td->inner);
				Infowin_resize(wid, hgt);
			}

			break;
		}
	}


	/* Hack -- Activate the old term */
	Term_activate(&old_td->t);

	/* Hack -- Activate the proper "inner" window */
	Infowin_set(old_td->inner);


	/* XXX XXX Hack -- map/unmap as needed */


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
		/* Activate the "inner" window */
		Infowin_set(td->inner);

		/* Activate the "inner" font */
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

	/* Check the colors */
	for (i = 0; i < 256; i++)
	{
		if (Metadpy->color)
		{
			char cname[8];

			sprintf(cname, "#%02x%02x%02x",
			        angband_color_table[i][1],
			        angband_color_table[i][2],
			        angband_color_table[i][3]);

			if (!streq(color_info[i], cname))
			{
				Infoclr_set(clr[i]);

				/* Change the color */
				Infoclr_change_fg(Infoclr_Pixell(cname));

				/* Save the color info */
				strcpy(color_info[i], cname);
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

		/* Flush the output XXX XXX XXX */
		case TERM_XTRA_FRESH: Metadpy_update(1, 0, 0); return (0);

		/* Process random events XXX XXX XXX */
		case TERM_XTRA_BORED: return (CheckEvent(0));

		/* Process Events XXX XXX XXX */
		case TERM_XTRA_EVENT: return (CheckEvent(v));

		/* Flush the events XXX XXX XXX */
		case TERM_XTRA_FLUSH: while (!CheckEvent(FALSE)); return (0);

		/* Handle change in the "level" */
		case TERM_XTRA_LEVEL: return (Term_xtra_x11_level(v));

		/* Clear the screen */
		case TERM_XTRA_CLEAR: Infowin_wipe(); return (0);

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY: usleep(1000 * v); return (0);

		/* React to changes XXX XXX XXX */
		case TERM_XTRA_REACT: return (Term_xtra_x11_react());
	}

	/* Unknown */
	return (1);
}


/*
 * Draw the cursor (XXX by hiliting)
 */
static errr Term_curs_x11(int x, int y)
{
	/* Draw the cursor */
	Infoclr_set(xor);

	/* Hilite the cursor character */
	Infofnt_text_non(x, y, " ", 1);

	/* Success */
	return (0);
}


/*
 * Erase a number of characters
 */
static errr Term_wipe_x11(int x, int y, int n)
{
	/* Erase (use black) */
	Infoclr_set(clr[0]);

	/* Mega-Hack -- Erase some space */
	Infofnt_text_non(x, y, "", n);

	/* Success */
	return (0);
}


/*
 * Draw a number of characters (XXX Consider using "cpy" mode)
 */
static errr Term_text_x11(int x, int y, int n, byte a, cptr s)
{
	/* Draw the text in Xor */
	Infoclr_set(clr[a]);

	/* Draw the text */
	Infofnt_text_std(x, y, s, n);

	/* Success */
	return (0);
}


/*
 * Initialize a term_data
 */
static errr term_data_init(term_data *td, bool fixed, cptr name, cptr font)
{
	term *t = &td->t;

	int wid, hgt, num;

	/* Prepare the standard font */
	MAKE(td->fnt, infofnt);
	Infofnt_set(td->fnt);
	Infofnt_init_data(font);

	/* Hack -- key buffer size */
	num = (fixed ? 1024 : 16);

	/* Assume full size windows */
	wid = 80 * td->fnt->wid;
	hgt = 24 * td->fnt->hgt;

	/* Create a top-window */
	MAKE(td->outer, infowin);
	Infowin_set(td->outer);
	Infowin_init_top(0, 0, wid + 2, hgt + 2, 1, Metadpy->fg, Metadpy->bg);
	Infowin_set_mask(StructureNotifyMask | KeyPressMask);
	Infowin_set_name(name);
	Infowin_set_class_hint(name);
	Infowin_set_size(wid+2, hgt+2, td->fnt->wid, td->fnt->hgt, fixed);
	Infowin_map();

	/* Create a sub-window */
	MAKE(td->inner, infowin);
	Infowin_set(td->inner);
	Infowin_init_std(td->outer, 1, 1, wid, hgt, 0);
	Infowin_set_mask(ExposureMask);
	Infowin_map();

	/* Initialize the term */
	term_init(t, 80, 24, num);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Hooks */
	t->xtra_hook = Term_xtra_x11;
	t->curs_hook = Term_curs_x11;
	t->wipe_hook = Term_wipe_x11;
	t->text_hook = Term_text_x11;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/* Success */
	return (0);
}


/*
 * Initialization function for an "X11" module to Angband
 */
errr init_x11(int argc, char *argv[])
{
	int i;

	cptr dpy_name = "";

	int num_term = MAX_TERM_DATA;

	char buf[80];


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
			if (num_term < 1)
			{
				num_term = 1;
			}
			if (num_term > MAX_TERM_DATA)
			{
				num_term = MAX_TERM_DATA;
			}
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}


	/* Init the Metadpy if possible */
	if (Metadpy_init_name(dpy_name)) return (-1);


	/* Prepare cursor color */
	MAKE(xor, infoclr);
	Infoclr_set(xor);
	Infoclr_init_ccn("fg", "bg", "xor", 0);

	/* Prepare normal colors */
	for (i = 0; i < 256; ++i)
	{
		char cname[8];

		MAKE(clr[i], infoclr);

		Infoclr_set(clr[i]);

		strcpy(buf, (i ? "bg" : "fg"));

		if (Metadpy->color)
		{
			sprintf(cname, "#%02x%02x%02x",
			        angband_color_table[i][1],
			        angband_color_table[i][2],
			        angband_color_table[i][3]);
		}

		/* Initialize the color */
		Infoclr_init_ccn(cname, "bg", "cpy", 0);

		/* Save the color info */
		strcpy(color_info[i], cname);
	}


	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		cptr fnt_name = NULL;

		cptr name = angband_term_name[i];

		/* Window specific font name */
		sprintf(buf, "ANGBAND_X11_FONT_%s", name);

		/* Check environment for that font */
		if (!fnt_name) fnt_name = getenv(buf);

		/* Window specific font name */
		sprintf(buf, "ANGBAND_X11_FONT_%d", i);

		/* Check environment for that font */
		if (!fnt_name) fnt_name = getenv(buf);

		/* Check environment for "base" font */
		if (!fnt_name) fnt_name = getenv("ANGBAND_X11_FONT");

		/* No environment variables, use default font */
		if (!fnt_name) fnt_name = DEFAULT_X11_FONT_SCREEN;

		/* Initialize the term_data */
		term_data_init(&data[i], TRUE, name, fnt_name);

		/* Save global entry */
		angband_term[i] = Term;
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Raise the "Angband" window */
	Infowin_set(data[0].outer);
	Infowin_raise();


	/* Success */
	return (0);
}


#endif


