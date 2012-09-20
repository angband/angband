/* File: main-xaw.c */

/*
 * Copyright (c) 1997 Ben Harrison, Torbjorn Lindgren, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with UNIX/X11 computers.
 *
 * To use this file, compile with "USE_XAW" defined, and link against all
 * the various "X11" libraries which may be needed.
 *
 * See also "main-x11.c".
 *
 * The Angband widget is not as self-contained as it really should be.
 * Originally everything was output to a Pixmap which was later copied
 * to the screen when necessary.  The idea was abandoned since Pixmaps
 * create big performance problems for some really old X terminals (such
 * as 3/50's running Xkernel).
 *
 * Initial framework (and some code) by Ben Harrison (benh@phial.com).
 *
 * Most of this file is by Torbjorn Lindgren (tl@cd.chalmers.se).
 *
 * Major modifications by Ben Harrison (benh@phial.com).
 */


#include "angband.h"


#ifdef USE_XAW


#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/ShellP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/XawInit.h>
#endif /* __MAKEDEPEND__ */


/*
 * Include some helpful X11 code.
 */
#include "maid-x11.h"



/**** Resources ****/


/*

Name                Class              RepType         Default Value
----                -----              -------         -------------
background          Background         Pixel           XtDefaultBackground
border              BorderColor        Pixel           XtDefaultForeground
borderWidth         BorderWidth        Dimension       1
cursor              Cursor             Cursor          None
cursorName          Cursor             String          NULL
destroyCallback     Callback           Pointer         NULL
height              Height             Dimension       0
insensitiveBorder   Insensitive        Pixmap          Gray
mappedWhenManaged   MappedWhenManaged  Boolean         True
pointerColor        Foreground         Pixel           XtDefaultForeground
pointerColorBackground Background      Pixel           XtDefaultBackground
sensitive           Sensitive          Boolean         True
width               Width              Dimension       0
x                   Position           Position        0
y                   Position           Position        0


The colors can be changed using the standard Angband user pref files,
which can also be used to provide black text on a white background,
by setting color zero to "#FFFFFF" and color one to "#000000", since
the other colors are unused.

*/


/*
 * New resource names
 */
#define XtNstartRows        "startRows"
#define XtNstartColumns     "startColumns"
#define XtNminRows          "minRows"
#define XtNminColumns       "minColumns"
#define XtNmaxRows          "maxRows"
#define XtNmaxColumns       "maxColumns"
#define XtNinternalBorder   "internalBorder"
#define XtNredrawCallback   "redrawCallback"

/*
 * Total normal colors
 */
#define NUM_COLORS 256

/*
 * Special "XOR" color
 */
#define COLOR_XOR 256



/**** The Widget Code ****/


/*
 * Forward declarations
 */
typedef struct AngbandPart AngbandPart;
typedef struct AngbandRec *AngbandWidget;
typedef struct AngbandRec AngbandRec;
typedef struct AngbandClassRec *AngbandWidgetClass;
typedef struct AngbandClassPart AngbandClassPart;
typedef struct AngbandClassRec AngbandClassRec;

typedef struct term_data term_data;


/*
 * A structure for each "term"
 */
struct term_data
{
	term t;

	AngbandWidget widget;
};


/*
 * Maximum number of windows
 */
#define MAX_TERM_DATA 8


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];


/*
 * Current number of windows open
 */
static int num_term = 1;

/*
 * New fields for the Angband widget record
 */
struct AngbandPart
{
	/* Settable resources */
	int start_rows;
	int start_columns;
	int min_rows;
	int min_columns;
	int max_rows;
	int max_columns;
	int internal_border;
	String font;

	XtCallbackList redraw_callbacks;

#ifdef USE_GRAPHICS

	/* Tiles */
	XImage *tiles;

#ifdef USE_TRANSPARENCY

	/* Tempory storage for overlaying tiles. */
	XImage *TmpImage;

#endif

#endif /* USE_GRAPHICS */

	/* Private state */
	XFontStruct *fnt;
	Dimension fontheight;
	Dimension fontwidth;
	Dimension fontascent;

	/* Color info for GC's */
	byte color[NUM_COLORS][4];

	/* GC's (including "xor") */
	GC gc[NUM_COLORS+1];
};


/*
 * Full instance record declaration
 */
struct AngbandRec
{
	CorePart core;
	SimplePart simple;
	AngbandPart angband;
};


/*
 * New fields for the Angband widget class record
 */
struct AngbandClassPart
{
	int dummy;
};


/*
 * Full class record declaration
 */
struct AngbandClassRec
{
	CoreClassPart core_class;
	SimpleClassPart simple_class;
	AngbandClassPart angband_class;
};



/*
 * Hack -- see below
 */
#define offset(field) XtOffsetOf(AngbandRec, angband.field)


/*
 * Fallback resources for Angband widget
 */
static XtResource resources[] =
{
	{ XtNstartRows, XtCValue, XtRInt, sizeof(int),
	  offset(start_rows), XtRImmediate, (XtPointer) 24 },
	{ XtNstartColumns, XtCValue, XtRInt, sizeof(int),
	  offset(start_columns), XtRImmediate, (XtPointer) 80 },
	{ XtNminRows, XtCValue, XtRInt, sizeof(int),
	  offset(min_rows), XtRImmediate, (XtPointer) 1 },
	{ XtNminColumns, XtCValue, XtRInt, sizeof(int),
	  offset(min_columns), XtRImmediate, (XtPointer) 1 },
	{ XtNmaxRows, XtCValue, XtRInt, sizeof(int),
	  offset(max_rows), XtRImmediate, (XtPointer) 24 },
	{ XtNmaxColumns, XtCValue, XtRInt, sizeof(int),
	  offset(max_columns), XtRImmediate, (XtPointer) 80 },
	{ XtNinternalBorder, XtCValue, XtRInt, sizeof(int),
	  offset(internal_border), XtRImmediate, (XtPointer) 2 },
	{ XtNfont, XtCFont, XtRString, sizeof(char *),
	  offset(font), XtRString, DEFAULT_X11_FONT },
	{ XtNredrawCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(redraw_callbacks), XtRCallback, (XtPointer)NULL }
};


/*
 * Hack -- see above
 */
#undef offset


/*
 * Forward declarations for Widget functions
 */
static void Initialize(AngbandWidget request, AngbandWidget wnew);
static void Redisplay(AngbandWidget w, XEvent *event, Region region);
static Boolean SetValues(AngbandWidget current, AngbandWidget request,
                         AngbandWidget wnew, ArgList args, Cardinal *num_args);
static void Destroy(AngbandWidget widget);
static void Resize_term(AngbandWidget wnew);

/*
 * Forward declaration for internal functions
 */
static void calculateSizeHints(AngbandWidget wnew);
static XFontStruct *getFont(AngbandWidget widget,
                            String font, Boolean fallback);


/*
 * Hack -- see below
 */
#define superclass (&simpleClassRec)


/*
 * Class record constanst
 */
AngbandClassRec angbandClassRec =
{
	{
		/* Core class fields initialization */
		/* superclass           */      (WidgetClass) superclass,
		/* class_name           */      "Angband",
		/* widget_size          */      sizeof(AngbandRec),
		/* class_initialize     */      NULL,
		/* class_part_initialize*/      NULL,
		/* class_inited         */      FALSE,
		/* initialize           */      (XtInitProc) Initialize,
		/* initialize_hook      */      NULL,
		/* realize              */      XtInheritRealize,
		/* actions              */      NULL,
		/* num_actions          */      0,
		/* resources            */      resources,
		/* num_resources        */      XtNumber(resources),
		/* xrm_class            */      NULLQUARK,
		/* compress_motion      */      TRUE,
		/* compress_exposure    */      XtExposeCompressMultiple,
		/* compress_enterleave  */      TRUE,
		/* visible_interest     */      FALSE,
		/* destroy              */      (XtWidgetProc) Destroy,
		/* resize               */      (XtWidgetProc) Resize_term,
		/* expose               */      (XtExposeProc) Redisplay,
		/* set_values           */      (XtSetValuesFunc) SetValues,
		/* set_values_hook      */      NULL,
		/* set_values_almost    */      XtInheritSetValuesAlmost,
		/* get_values_hook      */      NULL,
		/* accept_focus         */      NULL,
		/* version              */      XtVersion,
		/* callback_private     */      NULL,
		/* tm_table             */      NULL,
		/* query_geometry       */      NULL,
		/* display_accelerator  */      XtInheritDisplayAccelerator,
		/* extension            */      NULL
	},
	/* Simple class fields initialization */
	{
		/* change_sensitive     */      XtInheritChangeSensitive,

#ifndef OLDXAW
		/* extension            */      NULL
#endif /* OLDXAW */

	},
	/* Angband class fields initialization */
	{
		/* nothing              */      0
	}
};

/*
 * Hack -- see above
 */
#undef superclass


/*
 * Class record pointer
 */
WidgetClass angbandWidgetClass = (WidgetClass) &angbandClassRec;


/*
 * Public procedures
 */


/*
 * Clear an area
 */
static void AngbandClearArea(AngbandWidget widget,
                             int x, int y, int w, int h, int a)
{
	/* Figure out which area to clear */
	y = y * widget->angband.fontheight + widget->angband.internal_border;
	x = x * widget->angband.fontwidth + widget->angband.internal_border;

	/* Clear the area */
	XFillRectangle(XtDisplay(widget), XtWindow(widget),
	               widget->angband.gc[a],
	               x, y,
	               widget->angband.fontwidth * w,
	               widget->angband.fontheight * h);
}



/*
 * Output some text
 */
static void AngbandOutputText(AngbandWidget widget, int x, int y,
                              String txt, int len, int a)
{
	/* Do nothing if the string is null */
	if (!txt || !*txt) return;

	/* Check the length, and fix it if it's below zero */
	if (len < 0) len = strlen(txt);

	/* Figure out where to place the text */
	y = (y * widget->angband.fontheight + widget->angband.fontascent +
	     widget->angband.internal_border);
	x = (x * widget->angband.fontwidth + widget->angband.internal_border);

	/* Place the string */
	XDrawImageString(XtDisplay(widget), XtWindow(widget),
	                 widget->angband.gc[a], x, y, txt, len);
}


#ifdef USE_GRAPHICS

/*
 * Draw some graphical characters.
 */
# ifdef USE_TRANSPARENCY
static void AngbandOutputPict(AngbandWidget widget, int x, int y, int n,
 const byte *ap, const char *cp, const byte *tap, const char *tcp)
# else /* USE_TRANSPARENCY */
static void AngbandOutputPict(AngbandWidget widget, int x, int y, int n,
 const byte *ap, const char *cp)
# endif /* USE_TRANSPARENCY */


{
	int i, x1, y1;

	byte a;
	char c;

#ifdef USE_TRANSPARENCY
	byte ta;
	char tc;

	int x2, y2;
	int k,l;

	unsigned long pixel, blank;
#endif /* USE_TRANSPARENCY */

	/* Figure out where to place the text */
	y = (y * widget->angband.fontheight + widget->angband.internal_border);
	x = (x * widget->angband.fontwidth + widget->angband.internal_border);

	for (i = 0; i < n; ++i)
	{
		a = *ap++;
		c = *cp++;

		/* For extra speed - cache these values */
		x1 = (c&0x7F) * widget->angband.fontwidth;
		y1 = (a&0x7F) * widget->angband.fontheight;

#ifdef USE_TRANSPARENCY

		ta = *tap++;
		tc = *tcp++;

		/* For extra speed - cache these values */
		x2 = (tc&0x7F) * widget->angband.fontwidth;
		y2 = (ta&0x7F) * widget->angband.fontheight;

		/* Optimise the common case */
		if ((x1 == x2) && (y1 == y2))
		{
			/* Draw object / terrain */
			XPutImage(XtDisplay(widget), XtWindow(widget),
		  	        widget->angband.gc[0],
		    	      widget->angband.tiles,
		    	      x1, y1,
		    	      x, y,
		    	      widget->angband.fontwidth,
		 	      widget->angband.fontheight);
		}
		else
		{
			/* Mega Hack^2 - assume the top left corner is "black" */
			blank = XGetPixel(widget->angband.tiles,
				 0, widget->angband.fontheight * 6);

			for (k = 0; k < widget->angband.fontwidth; k++)
			{
				for (l = 0; l < widget->angband.fontheight; l++)
				{
					/* If mask set... */
					if ((pixel = XGetPixel(widget->angband.tiles,
						 x1 + k, y1 + l)) == blank)
					{

						/* Output from the terrain */
						pixel = XGetPixel(widget->angband.tiles,
							 x2 + k, y2 + l);
					}

					/* Store into the temp storage. */
					XPutPixel(widget->angband.TmpImage,
						 k, l, pixel);
				}
			}


			/* Draw to screen */

			/* Draw object / terrain */
			XPutImage(XtDisplay(widget), XtWindow(widget),
			          widget->angband.gc[0],
			          widget->angband.TmpImage,
			          0, 0,
			          x, y,
			          widget->angband.fontwidth,
			          widget->angband.fontheight);
		}

#else /* USE_TRANSPARENCY */

		/* Draw object / terrain */
		XPutImage(XtDisplay(widget), XtWindow(widget),
		          widget->angband.gc[0],
		          widget->angband.tiles,
		          x1, y1,
		          x, y,
		          widget->angband.fontwidth,
		          widget->angband.fontheight);

#endif /* USE_TRANSPARENCY */

		x += widget->angband.fontwidth;
	}
}

#endif /* USE_GRAPHICS */

/*
 * Private procedures
 */


/*
 * Procedure Initialize() is called during the widget creation
 * process.  Initialize() load fonts and calculates window geometry.
 * The request parameter is filled in by parents to this widget.  The
 * wnew parameter is the request parameter plus data filled in by this
 * widget. All changes should be done to the wnew parameter.
 */
static void Initialize(AngbandWidget request, AngbandWidget wnew)
{
	Display *dpy = XtDisplay(wnew);

	int depth = DefaultDepthOfScreen(XtScreen((Widget) wnew));

	XGCValues gcv;
	TopLevelShellWidget parent =
	(TopLevelShellWidget)XtParent((Widget) wnew);
	int i;

	/* Default background pixel */
	unsigned long bg = create_pixel(dpy,
	                                angband_color_table[0][1],
	                                angband_color_table[0][2],
	                                angband_color_table[0][3]);

	/* Default foreground pixel */
	unsigned long fg = create_pixel(dpy,
	                                angband_color_table[1][1],
	                                angband_color_table[1][2],
	                                angband_color_table[1][3]);

	/* Ignore this parameter */
	(void) request;

	/* Fix the background color */
	wnew->core.background_pixel = bg;

	/* Get some information about the font */
	wnew->angband.fnt = getFont(wnew, wnew->angband.font, TRUE);
	wnew->angband.fontheight = wnew->angband.fnt->ascent +
		wnew->angband.fnt->descent;
	wnew->angband.fontwidth = wnew->angband.fnt->max_bounds.width;
	wnew->angband.fontascent = wnew->angband.fnt->ascent;

	/* Create and initialize the graphics contexts */ /* GXset? */
	gcv.font = wnew->angband.fnt->fid;
	gcv.graphics_exposures = FALSE;
	gcv.background = bg;

	for (i = 0; i < NUM_COLORS; i++)
	{
		unsigned long pixel;

		/* Acquire Angband colors */
		wnew->angband.color[i][0] = angband_color_table[i][0];
		wnew->angband.color[i][1] = angband_color_table[i][1];
		wnew->angband.color[i][2] = angband_color_table[i][2];
		wnew->angband.color[i][3] = angband_color_table[i][3];

		if (depth > 1)
		{
			/* Create pixel */
			pixel = create_pixel(dpy,
			                     wnew->angband.color[i][1],
			                     wnew->angband.color[i][2],
			                     wnew->angband.color[i][3]);
		}
		else
		{
			/* Use background or foreground */
			pixel = ((i == 0) ? bg : fg);
		}

		gcv.foreground = pixel;

		/* Copy */
		gcv.function = 3;

		wnew->angband.gc[i] = XtGetGC((Widget)wnew,
		                              (GCFont | GCForeground | GCFunction |
		                               GCBackground | GCGraphicsExposures),
		                              &gcv);
	}

	/* Create a special GC for highlighting */
	gcv.foreground = (BlackPixelOfScreen(XtScreen((Widget)wnew)) ^
	                  WhitePixelOfScreen(XtScreen((Widget)wnew)));
	gcv.background = 0;

	gcv.function = GXxor;
	wnew->angband.gc[COLOR_XOR] = XtGetGC((Widget)wnew,
	                                      (GCFunction | GCForeground | GCBackground |
	                                       GCGraphicsExposures),
	                                      &gcv);

	/* Calculate window geometry */
	wnew->core.height = (wnew->angband.start_rows * wnew->angband.fontheight +
	                     2 * wnew->angband.internal_border);
	wnew->core.width = (wnew->angband.start_columns * wnew->angband.fontwidth +
	                    2 * wnew->angband.internal_border);

	/* We need to be able to resize the Widget if the user wants to */
	/* change font on the fly! */
	parent->shell.allow_shell_resize = TRUE;

	/* Calculates all the size hints */
	calculateSizeHints(wnew);
}


/*
 * Procedure Destroy() is called during the destruction of the widget.
 * Destroy() releases and frees GCs, frees the pixmaps and frees the
 * fonts.
 */
static void Destroy(AngbandWidget widget)
{
	int n;

	/* Free all GC's */
	for (n = 0; n < NUM_COLORS + 1; n++)
	{
		XtReleaseGC((Widget)widget, widget->angband.gc[n]);
	}

	/* Free the font */
	XFreeFont(XtDisplay((Widget)widget), widget->angband.fnt);
}


static void Resize_term(AngbandWidget wnew)
{
	int cols, rows, wid, hgt;

	int ox = wnew->angband.internal_border;
	int oy = wnew->angband.internal_border;

	int i;
	term_data *old_td = (term_data*)(Term->data);
	term_data *td = &data[0];

	/* Hack - Find the term to activate */
	for (i = 0; i < num_term; i++)
	{
		td = &data[i];

		/* Have we found it? */
		if (td->widget == wnew) break;

		/* Paranoia:  none of the widgets matched */
		if (!td) return;
	}

	/* Activate the proper Term */
	Term_activate(&td->t);

	/* Determine "proper" number of rows/cols */
	cols = ((wnew->core.width - (ox + ox)) / wnew->angband.fontwidth);
	rows = ((wnew->core.height - (oy + oy)) / wnew->angband.fontheight);

	/* Hack -- minimal size */
	if (cols < 1) cols = 1;
	if (rows < 1) rows = 1;

	if (i == 0)
	{
		/* Hack the main window must be at least 80x24 */
		if (cols < 80) cols = 80;
		if (rows < 24) rows = 24;
	}

	/* Desired size of window */
	wid = cols * wnew->angband.fontwidth + (ox + ox);
	hgt = rows * wnew->angband.fontheight + (oy + oy);

	/* Resize the Term (if needed) */
	(void) Term_resize(cols, rows);

	/* Activate the old term */
	Term_activate(&old_td->t);
}

/*
 * Procedure Redisplay() is called as the result of an Expose event.
 * Use the redraw callback to do a full redraw
 */
static void Redisplay(AngbandWidget wnew, XEvent *xev, Region region)
{
	int x1, x2, y1, y2;

	int i;

	term_data *old_td = (term_data*)(Term->data);
	term_data *td = &data[0];

	/* Ignore parameter */
	(void) region;

	/* Hack - Find the term to activate */
	for (i = 0; i < num_term; i++)
	{
		td = &data[i];

		/* Have we found it? */
		if (td->widget == wnew) break;

		/* Paranoia:  none of the widgets matched */
		if (!td) return;
	}

	/* Activate the proper Term */
	Term_activate(&td->t);

	/* Find the bounds of the exposed region */

	/*
	 * This probably could be obtained from the Region parameter -
	 * but I don't know anything about XAW.
	 */
	x1 = (xev->xexpose.x - wnew->angband.internal_border)
		/wnew->angband.fontwidth;
	x2 = (xev->xexpose.x + xev->xexpose.width -
		wnew->angband.internal_border)/wnew->angband.fontwidth;

	y1 = (xev->xexpose.y - wnew->angband.internal_border)
		/wnew->angband.fontheight;
	y2 = (xev->xexpose.y + xev->xexpose.height -
		wnew->angband.internal_border)/wnew->angband.fontheight;

	Term_redraw_section(x1, y1, x2, y2);

	/* Activate the old term */
	Term_activate(&old_td->t);


#if 0
	if (XtHasCallbacks((Widget)widget, XtNredrawCallback) == XtCallbackHasSome)
	{
		XtCallCallbacks((Widget)widget, XtNredrawCallback, NULL);
	}
#endif /* 0 */
}


/*
 * Font and internal_border can be changed on the fly.
 *
 * The entire widget is redrawn if any of those parameters change (all
 * can potentially have effects that spans the whole widget).
 *
 * Color changes are handled elsewhere.
 *
 * This function is very underspecified, in terms of how these changes can
 * occur, and what is true about the various AngbandWidget's passed in.  It
 * is very likely that this code no longer works.
 */
static Boolean SetValues(AngbandWidget current, AngbandWidget request,
                         AngbandWidget wnew, ArgList args,
                         Cardinal *num_args)
{
	Display *dpy = XtDisplay(wnew);

	Boolean font_changed = FALSE;
	Boolean border_changed = FALSE;
	int height, width;
	int i;

	/* Ignore parameters */
	(void) request;
	(void) args;
	(void) num_args;

	/* Handle font change */
	if (wnew->angband.font != current->angband.font)
	{
		/* Check if the font exists */
		wnew->angband.fnt = getFont(wnew, wnew->angband.font, FALSE);

		/* The font didn't exist */
		if (wnew->angband.fnt == NULL)
		{
			wnew->angband.fnt = current->angband.fnt;
			wnew->angband.font = current->angband.font;
			XtWarning("Couldn't find the requested font!");
		}
		else
		{
			font_changed = TRUE;

			/* Free the old font */
			XFreeFont(XtDisplay((Widget)wnew), current->angband.fnt);
			/* Update font information */
			wnew->angband.fontheight = wnew->angband.fnt->ascent +
				wnew->angband.fnt->descent;
			wnew->angband.fontwidth = wnew->angband.fnt->max_bounds.width;
			wnew->angband.fontascent = wnew->angband.fnt->ascent;
		}
	}

	/* Handle font change */
	if (font_changed)
	{
		/* Update all GC's */
		for (i = 0; i < NUM_COLORS; i++)
		{
			/* Steal the old GC */
			wnew->angband.gc[i] = current->angband.gc[i];
			current->angband.gc[i] = NULL;

			/* Be sure the correct font is ready */
			XSetFont(dpy, wnew->angband.gc[i], wnew->angband.fnt->fid);

			/* Steal the old GC */
			wnew->angband.gc[NUM_COLORS] = current->angband.gc[NUM_COLORS];
			current->angband.gc[NUM_COLORS] = NULL;
		}
	}


	/* Check if internal border width has changed, used later */
	if (current->angband.internal_border != wnew->angband.internal_border)
	{
		border_changed = TRUE;
	}


	/* If the font or the internal border has changed, all geometry */
	/* has to be recalculated */
	if (font_changed || border_changed)
	{
		/* Change window size */
		height = ((current->core.height - 2 * current->angband.internal_border) /
		          current->angband.fontheight * wnew->angband.fontheight +
		          2 * current->angband.internal_border);
		width = ((current->core.width -  2 * current->angband.internal_border) /
		         current->angband.fontwidth * wnew->angband.fontwidth +
		         2 * wnew->angband.internal_border);

		/* Get the new width */
		if (XtMakeResizeRequest((Widget)wnew, width, height, NULL, NULL) ==
		    XtGeometryNo)
		{
			/* Not allowed */
			XtWarning("Size change denied!");
		}
		else
		{
			/* Recalculate size hints */
			calculateSizeHints(wnew);
		}
	}

	/* Tell it to redraw the widget if anything has changed */
	return (font_changed || border_changed);
}


/*
 * Calculate size hints
 */
static void calculateSizeHints(AngbandWidget wnew)
{
	TopLevelShellWidget parent =
	(TopLevelShellWidget)XtParent((Widget) wnew);

	/* Calculate minimum size */
	parent->wm.size_hints.min_height =
	(wnew->angband.min_rows * wnew->angband.fontheight +
	 2 * wnew->angband.internal_border);

	/* Calculate minimum size */
	parent->wm.size_hints.min_width =
	(wnew->angband.min_columns * wnew->angband.fontwidth +
	 2 * wnew->angband.internal_border);

	/* Calculate minimum size */
	parent->wm.size_hints.flags |= PMinSize;

	/* Calculate maximum size */
	parent->wm.size_hints.max_height =
	(wnew->angband.max_rows * wnew->angband.fontheight +
	 2 * wnew->angband.internal_border);

	/* Calculate maximum size */
	parent->wm.size_hints.max_width =
	(wnew->angband.max_columns * wnew->angband.fontwidth +
	 2 * wnew->angband.internal_border);

	/* Calculate maximum size */
	parent->wm.size_hints.flags |= PMaxSize;

	/* Calculate increment size */
	parent->wm.size_hints.height_inc = wnew->angband.fontheight;
	parent->wm.size_hints.width_inc = wnew->angband.fontwidth;
	parent->wm.size_hints.flags |= PResizeInc;

	/* Calculate base size */
	parent->wm.base_height = 2 * wnew->angband.internal_border;
	parent->wm.base_width = 2 * wnew->angband.internal_border;
	parent->wm.size_hints.flags |= PBaseSize;
}


/*
 * Load a font
 */
static XFontStruct *getFont(AngbandWidget widget,
                            String font, Boolean fallback)
{
	Display *dpy = XtDisplay((Widget) widget);
	char buf[256];
	XFontStruct *fnt = NULL;

	if (!(fnt = XLoadQueryFont(dpy, font)) && fallback)
	{
		sprintf(buf, "Can't find the font \"%s\", trying fixed\n", font);
		XtWarning(buf);
		if (!(fnt = XLoadQueryFont(dpy, "fixed")))
		{
			XtError("Can't fint the font \"fixed\"!, bailing out\n");
		}
	}

	return fnt;
}



/*** The Angband code ****/





/*
 * Number of fallback resources per window
 */
#define TERM_FALLBACKS 7



/*
 * The names of the term_data's
 */
char *termNames[MAX_TERM_DATA] =
{
	"angband",
	"term-1",
	"term-2",
	"term-3",
	"term-4",
	"term-5",
	"term-6",
	"term-7"
};


/*
 * The special Arg's
 */
Arg specialArgs[TERM_FALLBACKS] =
{
	{ XtNstartRows,    24},
	{ XtNstartColumns, 80},
	{ XtNminRows,      24},
	{ XtNminColumns,   80},
	{ XtNmaxRows,      255},
	{ XtNmaxColumns,   255},
	{ XtNinternalBorder, 2}
};


/*
 * The default Arg's
 */
Arg defaultArgs[TERM_FALLBACKS] =
{
	{ XtNstartRows,      24},
	{ XtNstartColumns,   80},
	{ XtNminRows,        1},
	{ XtNminColumns,     1},
	{ XtNmaxRows,        255},
	{ XtNmaxColumns,     255},
	{ XtNinternalBorder, 2}
};


/*
 * The application context
 */
XtAppContext appcon;


/*
 * User changable information about widgets
 */
static String fallback[] =
{
	"Angband.angband.iconName:   Angband",
	"Angband.angband.title:      Angband",
	"Angband.term-1.iconName:    Term 1",
	"Angband.term-1.title:       Term 1",
	"Angband.term-2.iconName:    Term 2",
	"Angband.term-2.title:       Term 2",
	"Angband.term-3.iconName:    Term 3",
	"Angband.term-3.title:       Term 3",
	"Angband.term-4.iconName:    Term 4",
	"Angband.term-4.title:       Term 4",
	"Angband.term-5.iconName:    Term 5",
	"Angband.term-5.title:       Term 5",
	"Angband.term-6.iconName:    Term 6",
	"Angband.term-6.title:       Term 6",
	"Angband.term-7.iconName:    Term 7",
	"Angband.term-7.title:       Term 7",
	NULL
};



/*
 * Do a redraw
 */
static void react_redraw(Widget widget,
                         XtPointer client_data, XtPointer call_data)
{
	term_data *old_td = (term_data*)(Term->data);
	term_data *td = (term_data*)client_data;

	/* Ignore parameters */
	(void) widget;
	(void) call_data;

	/* Activate the proper Term */
	Term_activate(&td->t);

	/* Request a redraw */
	Term_redraw();

	/* Activate the old Term */
	Term_activate(&old_td->t);
}



/*
 * Process a keypress event
 *
 * Also appears in "main-x11.c".
 */
static void react_keypress(XKeyEvent *xev)
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
 * Handle an event
 */
static void handle_event(Widget widget, XtPointer client_data, XEvent *event,
                         Boolean *continue_to_dispatch)
{
	term_data *old_td = (term_data*)(Term->data);
	term_data *td = (term_data *)client_data;

	/* Ignore parameter */
	(void) widget;

	/* Continue to process the event by default */
	*continue_to_dispatch = TRUE;

	/* Activate the Term */
	Term_activate(&td->t);

	switch (event->type)
	{
		case KeyPress:
		{
			/* Hack -- use old term */
			Term_activate(&old_td->t);

			/* Handle the keypress */
			react_keypress(&(event->xkey));

			/* We took care of the event */
			*continue_to_dispatch = FALSE;

			break;
		}

		/* Oops */
		default:
		{
			break;
		}
	}

	/* Activate the old term */
	Term_activate(&old_td->t);

	return;
}


/*
 * Process an event (or just check for one)
 */
static errr CheckEvent(bool wait)
{
	XEvent event;

	/* No events ready, and told to just check */
	if (!wait && !XtAppPending(appcon)) return 1;

	/* Process */
	while (1)
	{
		XtAppNextEvent(appcon, &event);
		XtDispatchEvent(&event);
		if (!XtAppPending(appcon)) break;
	}

	return (0);
}


/*
 * Monstrous hack.
 */
static void Term_xtra_xaw_react_aux(term_data *td)
{
	AngbandWidget wnew = td->widget;

	Display *dpy = XtDisplay((Widget) wnew);

	int depth = DefaultDepthOfScreen(XtScreen((Widget) wnew));

	int i;

	/* See if any colors need to be changed */
	for (i = 0; i < NUM_COLORS; i++)
	{
		if (depth > 1)
		{
			if ((wnew->angband.color[i][0] != angband_color_table[i][0]) ||
			    (wnew->angband.color[i][1] != angband_color_table[i][1]) ||
			    (wnew->angband.color[i][2] != angband_color_table[i][2]) ||
			    (wnew->angband.color[i][3] != angband_color_table[i][3]))
			{
				unsigned long pixel;

				/* Save new values */
				wnew->angband.color[i][0] = angband_color_table[i][0];
				wnew->angband.color[i][1] = angband_color_table[i][1];
				wnew->angband.color[i][2] = angband_color_table[i][2];
				wnew->angband.color[i][3] = angband_color_table[i][3];

				/* Create pixel */
				pixel = create_pixel(dpy,
				                     wnew->angband.color[i][1],
				                     wnew->angband.color[i][2],
				                     wnew->angband.color[i][3]);


				/* Change */
				XSetForeground(dpy, wnew->angband.gc[i], pixel);
			}
		}
	}
}


/*
 * Monstrous hack.
 */
static errr Term_xtra_xaw_react(void)
{
	int i;

	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		if (!td) break;

		Term_xtra_xaw_react_aux(td);
	}

	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_xaw(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	Widget widget = (Widget)(td->widget);

	Display *dpy = XtDisplay(widget);

	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE:
		XBell(dpy, 100);
		return (0);

		/* Flush the output */
		case TERM_XTRA_FRESH:
		XFlush(dpy);
		/* Allow flushed events to be showed */
		CheckEvent(FALSE);
		return (0);

		/* Process random events */
		case TERM_XTRA_BORED:
		return (CheckEvent(0));

		/* Process events */
		case TERM_XTRA_EVENT:
		return (CheckEvent(v));

		/* Flush events */
		case TERM_XTRA_FLUSH:
		while (!CheckEvent(FALSE));
		return (0);

		/* Clear the window */
		case TERM_XTRA_CLEAR:
		XClearWindow(dpy, XtWindow(widget));
		return (0);

		/* Delay */
		case TERM_XTRA_DELAY:
		usleep(1000 * v);
		return (0);

		case TERM_XTRA_REACT:
		return (Term_xtra_xaw_react());
	}

	/* Unknown */
	return (1);
}



/*
 * Erase a number of characters
 */
static errr Term_wipe_xaw(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	/* Erase using color 0 */
	AngbandClearArea(td->widget, x, y, n, 1, 0);

	/* Success */
	return (0);
}



/*
 * Draw the cursor, by hiliting with XOR
 *
 * Should perhaps use rectangle outline, ala "main-mac.c".  XXX XXX XXX
 */
static errr Term_curs_xaw(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	/* Hilite the cursor character with a box */
	XDrawRectangle(XtDisplay(td->widget), XtWindow(td->widget),
		td->widget->angband.gc[COLOR_XOR],
		x * td->widget->angband.fontwidth +
			td->widget->angband.internal_border,
		y * td->widget->angband.fontheight +
			td->widget->angband.internal_border,
		td->widget->angband.fontwidth - 1, td->widget->angband.fontheight - 1);

	/* Success */
	return (0);
}


/*
 * Draw a number of characters
 */
static errr Term_text_xaw(int x, int y, int n, byte a, cptr s)
{
	term_data *td = (term_data*)(Term->data);

	/* Draw the text */
	AngbandOutputText(td->widget, x, y, (String)s, n, a);

	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS

/*
 * Draw some graphical characters.
 */
# ifdef USE_TRANSPARENCY
static errr Term_pict_xaw(int x, int y, int n, const byte *ap, const char *cp,
	const byte *tap, const char *tcp)
# else /* USE_TRANSPARENCY */
static errr Term_pict_xaw(int x, int y, int n, const byte *ap, const char *cp)
# endif /* USE_TRANSPARENCY */
{
	term_data *td = (term_data*)(Term->data);

	/* Draw the pictures */
# ifdef USE_TRANSPARENCY
	AngbandOutputPict(td->widget, x, y, n, ap, cp, tap, tcp);
# else /* USE_TRANSPARENCY */
	AngbandOutputPict(td->widget, x, y, n, ap, cp);
# endif /* USE_TRANSPARENCY */

	/* Success */
	return (0);
}

#endif /* USE_GRAPHICS */


/*
 * Raise a term
 */
static void term_raise(term_data *td)
{
	Widget widget = (Widget)(td->widget);

	XRaiseWindow(XtDisplay(XtParent(widget)), XtWindow(XtParent(widget)));
}


/*
 * Initialize a term_data
 */
static errr term_data_init(term_data *td, Widget topLevel,
                           int key_buf, String name,
                           ArgList widget_arg, Cardinal widget_arg_no, int i)
{
	Widget parent;
	term *t = &td->t;

	int cols = 80;
	int rows = 24;

	char buf[80];
	cptr str;

	int val;

	/* Create the shell widget */
	parent = XtCreatePopupShell(name, topLevelShellWidgetClass, topLevel,
	                            NULL, 0);

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
	if (i == 0)
	{
		if (cols < 80) cols = 80;
		if (rows < 24) rows = 24;
	}

	/* Reset the initial size */
	widget_arg[0].value = rows;
	widget_arg[1].value = cols;

	/* Hack  ox==oy in xaw port */

	/* Window specific inner border offset (ox) */
	sprintf(buf, "ANGBAND_X11_IBOX_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) widget_arg[6].value = val;

	/* Window specific inner border offset (oy) */
	sprintf(buf, "ANGBAND_X11_IBOY_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) widget_arg[6].value = val;

	/* Create the interior widget */
	td->widget = (AngbandWidget)
	XtCreateManagedWidget(name, angbandWidgetClass,
	                      parent, widget_arg, widget_arg_no);

	/* Initialize the term (full size) */
	term_init(t, cols, rows, key_buf);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Hooks */
	t->xtra_hook = Term_xtra_xaw;
	t->curs_hook = Term_curs_xaw;
	t->wipe_hook = Term_wipe_xaw;
	t->text_hook = Term_text_xaw;

	/* Save the data */
	t->data = td;

	/* Register the keypress event handler */
	XtAddEventHandler((Widget)td->widget, KeyPressMask,
	                  False, (XtEventHandler) handle_event, td);

	/* Redraw callback */
	XtAddCallback((Widget)td->widget, XtNredrawCallback,
	              react_redraw, td);

	/* Realize the widget */
	XtRealizeWidget(parent);

	/* Have we redefined the font? */
	if (streq(td->widget->angband.font, DEFAULT_X11_FONT))
	{
		XFontStruct *fnt;

		/* Check if the font exists */
		fnt = getFont(td->widget, (String) get_default_font(i), FALSE);

		/* The font didn't exist */
		if (fnt == NULL)
		{
			XtWarning("Couldn't find the requested font!");
		}
		else
		{
			int height, width;

			/* Free the old font */
			XFreeFont(XtDisplay((Widget)td->widget), td->widget->angband.fnt);

			/* Update font information */
			td->widget->angband.fontheight = fnt->ascent + fnt->descent;
			td->widget->angband.fontwidth = fnt->max_bounds.width;
			td->widget->angband.fontascent = fnt->ascent;

			for (i = 0; i < NUM_COLORS; i++)
			{
				/* Be sure the correct font is ready */
				XSetFont(XtDisplay((Widget)td->widget),
					 td->widget->angband.gc[i], fnt->fid);
			}

			/* Get the window shape */
			height = (td->widget->angband.start_rows *
				td->widget->angband.fontheight +
				2 * td->widget->angband.internal_border);
			width = (td->widget->angband.start_columns *
				td->widget->angband.fontwidth +
				2 * td->widget->angband.internal_border);

			/* Request a change to the new shape */
			if (XtMakeResizeRequest((Widget)td->widget,
				 width, height, NULL, NULL) == XtGeometryNo)
			{
				/* Not allowed */
				XtWarning("Size change denied!");
			}
			else
			{
				/* Recalculate size hints */
				calculateSizeHints(td->widget);
			}
		}
	}

	/* Make it visible */
	XtPopup(parent, XtGrabNone);

	/* Activate (important) */
	Term_activate(t);


	Resize_term(td->widget);

	return 0;
}


/*
 * Initialization function for an X Athena Widget module to Angband
 *
 * We should accept "-d<dpy>" requests in the "argv" array.  XXX XXX XXX
 */
errr init_xaw(int argc, char *argv[])
{
	int i;
	Widget topLevel;
	Display *dpy;

	cptr dpy_name = "";


#ifdef USE_GRAPHICS

	char filename[1024];

	int pict_wid = 0;
	int pict_hgt = 0;

#ifdef USE_TRANSPARENCY

	char *TmpData;
#endif /* USE_TRANSPARENCY */

#endif /* USE_GRAPHICS */

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-d"))
		{
			dpy_name = &argv[i][2];
			continue;
		}

#ifdef USE_GRAPHICS
		if (prefix(argv[i], "-s"))
		{
			smoothRescaling = FALSE;
			continue;
		}
#endif /* USE_GRAPHICS */

		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}


	/* Attempt to open the local display */
	dpy = XOpenDisplay(dpy_name);

	/* Failure -- assume no X11 available */
	if (!dpy) return (-1);

	/* Close the local display */
	XCloseDisplay(dpy);


#ifdef USE_XAW_LANG

	/* Support locale processing */
	XtSetLanguageProc(NULL, NULL, NULL);

#endif /* USE_XAW_LANG */


	/* Initialize the toolkit */
	topLevel = XtAppInitialize(&appcon, "Angband", NULL, 0, &argc, argv,
	                           fallback, NULL, 0);


	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		term_data_init(td, topLevel, 1024, termNames[i],
		               (i == 0) ? specialArgs : defaultArgs,
		               TERM_FALLBACKS, i);

		angband_term[i] = Term;
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Raise the "Angband" window */
	term_raise(&data[0]);


#ifdef USE_GRAPHICS

	/* Try graphics */
	if (arg_graphics)
	{
		/* Try the "16x16.bmp" file */
		path_build(filename, 1024, ANGBAND_DIR_XTRA, "graf/16x16.bmp");

		/* Use the "16x16.bmp" file if it exists */
		if (0 == fd_close(fd_open(filename, O_RDONLY)))
		{
			/* Use graphics */
			use_graphics = TRUE;

			use_transparency = TRUE;

			pict_wid = pict_hgt = 16;

			ANGBAND_GRAF = "new";
		}
		else
		{
			/* Try the "8x8.bmp" file */
			path_build(filename, 1024, ANGBAND_DIR_XTRA, "graf/8x8.bmp");

			/* Use the "8x8.bmp" file if it exists */
			if (0 == fd_close(fd_open(filename, O_RDONLY)))
			{
				/* Use graphics */
				use_graphics = TRUE;

				pict_wid = pict_hgt = 8;

				ANGBAND_GRAF = "old";
			}
		}
	}

	/* Load graphics */
	if (use_graphics)
	{
		/* Hack -- Get the Display */
		term_data *td = &data[0];
		Widget widget = (Widget)(td->widget);
		Display *dpy = XtDisplay(widget);

		XImage *tiles_raw;

		/* Load the graphical tiles */
		tiles_raw = ReadBMP(dpy, filename);

		/* Initialize the windows */
		for (i = 0; i < num_term; i++)
		{
			term_data *td = &data[i];

			term *t = &td->t;

			t->pict_hook = Term_pict_xaw;

			t->higher_pict = TRUE;

			/* Resize tiles */
			td->widget->angband.tiles =
			ResizeImage(dpy, tiles_raw,
			            pict_wid, pict_hgt,
			            td->widget->angband.fontwidth,
			            td->widget->angband.fontheight);
		}

#ifdef USE_TRANSPARENCY
		/* Initialize the transparency temp storage*/
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
			total = td->widget->angband.fontwidth *
				 td->widget->angband.fontheight * ii;


			TmpData = (char *)malloc(total);

			td->widget->angband.TmpImage = XCreateImage(dpy,
				visual,depth,
				ZPixmap, 0, TmpData,
				td->widget->angband.fontwidth,
			        td->widget->angband.fontheight, 8, 0);

		}
#endif /* USE_TRANSPARENCY */


		/* Free tiles_raw? XXX XXX */
	}

#endif /* USE_GRAPHICS */

	/* Success */
	return (0);
}

#endif /* USE_XAW */
