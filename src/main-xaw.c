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
 * To use this file, use "Makefile.xaw", which defines "USE_XAW".
 *
 * See also "main-x11.c".
 *
 *
 * The Angband widget is not as self-contained as it really should be.
 * Originally everything was output to a Pixmap which was later copied
 * to the screen when necessary.  The idea was abandoned since Pixmaps
 * create big performance problems for some really old X terminals (such
 * as 3/50's running Xkernel).
 *
 * The default colors used are based on the ones used in main-mac.c, with
 * the main difference being that they are gamma corrected for a gamma of
 * 1.6, since MacOS do gamma correction afterwards, but X uses raw colors.
 * The Gamma of most color screens are about 1.5 - 1.7.  Color 12 was later
 * changed a bit so that it didn't look as similar to color 3/4.
 *
 * This file should really attempt to obey the "angband_colors" table,
 * even at initialization, since it is a better color specifier than the
 * stupid resources, and it must be initialized specially for the special
 * "use_graphics" code.  XXX XXX XXX
 *
 * We should allow interactive color modifications.  XXX XXX XXX
 *
 *
 * Initial framework (and some code) by Ben Harrison (benh@phial,com).
 *
 * Most code by Torbjorn Lindgren (tl@cd.chalmers.se).
 *
 * Graphics support (the "XImage" functions) by Desvignes Sebastien
 * (desvigne@solar12.eerie.fr).
 */


#ifdef USE_XAW

#include "angband.h"


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



/**** Graphics Functions ****/


#ifdef USE_GRAPHICS


/*
 * Read a raw file. XXX XXX XXX
 *
 * Also appears in "main-x11.c".
 */
static XImage *ReadRaw(Display *disp, char Name[], int width, int height)
{
	FILE *f;

	XImage *Res = NULL;

	char *Data;

	int depth, i;


	f = fopen(Name, "r");

	if (f != NULL)
	{
		depth = 4;

		Data = (char *)calloc(width * height * depth / 8, 1);

		if (Data != NULL)
		{
			Res = XCreateImage(disp,
			                   DefaultVisual(disp, DefaultScreen(disp)),
			                   depth, XYPixmap, 0, Data,
			                   width, height, 8, 0);

			if (Res != NULL)
			{
				for (i=0; i<4; i++)
				{
					fread(Data+(3-i)*(width*height/8), width>>3, height, f);
				}
			}
			else
			{
				free(Data);
			}
		}

		fclose(f);
	}

	return Res;
}


/*
 * Remap colors. XXX XXX XXX
 *
 * Also appears in "main-x11.c".
 */
static XImage *RemapColors(Display *disp, XImage *Im, unsigned long ColT[])
{
	XImage *Tmp = NULL;

	char *Data;

	int width, height, depth;

	int x, y;


	width = Im->width;
	height = Im->height;

	depth = DefaultDepth(disp, DefaultScreen(disp));

	x = 1;
	y = (depth-1) >> 2;

	while (y>>=1) x<<=1;

	Data = (char *)malloc(width * height * x);

	if (Data != NULL)
	{
		Tmp = XCreateImage(disp,
		                   DefaultVisual(disp, DefaultScreen(disp)),
		                   depth, ZPixmap, 0, Data, width, height,
		                   32, 0);

		if (Tmp != NULL)
		{
			for (y=0; y<height; y++)
			{
				for (x=0; x<width; x++)
				{
					XPutPixel(Tmp, x, y, ColT[ 16 + XGetPixel(Im, x, y) ]);
				}
			}
		}
		else
		{
			free(Data);
		}
	}

	return Tmp;
}


/*
 * Resize an image. XXX XXX XXX
 *
 * Also appears in "main-x11.c".
 */
static XImage *ResizeImage(Display *disp, XImage *Im,
                           int ix, int iy, int ox, int oy)
{
	int width1, height1, width2, height2;
	int x1, x2, y1, y2, Tx, Ty;
	int *px1, *px2, *dx1, *dx2;
	int *py1, *py2, *dy1, *dy2;

	XImage *Tmp;

	char *Data;


	width1 = Im->width;
	height1 = Im->height;

	width2 = ox * width1 / ix;
	height2 = oy * height1 / iy;

	Data = (char *)malloc(width2 * height2 * Im->bits_per_pixel / 8);

	Tmp = XCreateImage(disp,
	                   DefaultVisual(disp, DefaultScreen(disp)),
	                   Im->depth, ZPixmap, 0, Data, width2, height2,
	                   32, 0);

	if (ix > ox)
	{
		px1 = &x1;
		px2 = &x2;
		dx1 = &ix;
		dx2 = &ox;
	}
	else
	{
		px1 = &x2;
		px2 = &x1;
		dx1 = &ox;
		dx2 = &ix;
	}

	if (iy > oy)
	{
		py1 = &y1;
		py2 = &y2;
		dy1 = &iy;
		dy2 = &oy;
	}
	else
	{
		py1 = &y2;
		py2 = &y1;
		dy1 = &oy;
		dy2 = &iy;
	}

	Ty = *dy1;

	for (y1=0, y2=0; (y1 < height1) && (y2 < height2); )
	{
		Tx = *dx1;

		for (x1=0, x2=0; (x1 < width1) && (x2 < width2); )
		{
			XPutPixel(Tmp, x2, y2, XGetPixel(Im, x1, y1));

			(*px1)++;

			Tx -= *dx2;
			if (Tx < 0)
			{
				Tx += *dx1;
				(*px2)++;
			}
		}

		(*py1)++;

		Ty -= *dy2;
		if (Ty < 0)
		{
			Ty += *dy1;
			(*py2)++;
		}
	}

	return Tmp;
}


#endif /* USE_GRAPHICS */


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


My own X Resources look like this (on a 1152x900 screen):

angband*angband*font:                   12x24
angband*angband*geometry:               +0+-20
angband*recall*font:                    7x13
angband*recall*geometry:                80x10+0+586
angband*choice*font:                    7x13
angband*choice*geometry:                -0-0

It's also possible to change the colors using X Resources, the
standard colors would look like:

angband*color0:                         #000000
angband*color1:                         #ffffff
angband*color2:                         #a6a6a6
angband*color3:                         #ff6302
angband*color4:                         #ca0808
angband*color5:                         #008e18
angband*color6:                         #0000e3
angband*color7:                         #814007
angband*color8:                         #6b6b6b
angband*color9:                         #d6d6d6
angband*color10:                        #5100c2
angband*color11:                        #fdf105
angband*color12:                        #ff9259
angband*color13:                        #26cf17
angband*color14:                        #02b2f2
angband*color15:                        #b28b48

And the newer colors look like:

angband*color0:                         #000000
angband*color1:                         #ffffff
angband*color2:                         #d7d7d7
angband*color3:                         #ff9200
angband*color4:                         #ff0000
angband*color5:                         #00cd00
angband*color6:                         #0000fe
angband*color7:                         #c86400
angband*color8:                         #a3a3a3
angband*color9:                         #ebebeb
angband*color10:                        #a500ff
angband*color11:                        #fffd00
angband*color12:                        #ff00bc
angband*color13:                        #00ff00
angband*color14:                        #00c8ff
angband*color15:                        #ffcc80

And the bitmap colors look like:

xxxxx

angband*color16:                        #000000
angband*color17:                        #F0E0D0
angband*color18:                        #808080
angband*color19:                        #505050
angband*color20:                        #E0B000
angband*color21:                        #C0A070
angband*color22:                        #806040
angband*color23:                        #403020
angband*color24:                        #00A0F0
angband*color25:                        #0000F0
angband*color26:                        #000070
angband*color27:                        #F00000
angband*color28:                        #800000
angband*color29:                        #9000B0
angband*color30:                        #006010
angband*color31:                        #60F040


Some older monochrome monitors have problem with white text on black
background. The new code can handle the reverse situation if the user
wants/needs this.

The following X Resources gives black text on white background using
Angband/Xaw. The other colors (2-15) isn't changed, since they're not
used on a monochrome monitor.

angband*color0: #ffffff
angband*color1: #000000

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
#define XtNcolor0           "color0"
#define XtNcolor1           "color1"
#define XtNcolor2           "color2"
#define XtNcolor3           "color3"
#define XtNcolor4           "color4"
#define XtNcolor5           "color5"
#define XtNcolor6           "color6"
#define XtNcolor7           "color7"
#define XtNcolor8           "color8"
#define XtNcolor9           "color9"
#define XtNcolor10          "color10"
#define XtNcolor11          "color11"
#define XtNcolor12          "color12"
#define XtNcolor13          "color13"
#define XtNcolor14          "color14"
#define XtNcolor15          "color15"
#define XtNcolor16          "color16"
#define XtNcolor17          "color17"
#define XtNcolor18          "color18"
#define XtNcolor19          "color19"
#define XtNcolor20          "color20"
#define XtNcolor21          "color21"
#define XtNcolor22          "color22"
#define XtNcolor23          "color23"
#define XtNcolor24          "color24"
#define XtNcolor25          "color25"
#define XtNcolor26          "color26"
#define XtNcolor27          "color27"
#define XtNcolor28          "color28"
#define XtNcolor29          "color29"
#define XtNcolor30          "color30"
#define XtNcolor31          "color31"
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
	Pixel color[NUM_COLORS];
	XtCallbackList redraw_callbacks;

#ifdef USE_GRAPHICS

	/* Tiles */
	XImage *tiles;

#endif /* USE_GRAPHICS */

	/* Private state */
	XFontStruct *fnt;
	Dimension fontheight;
	Dimension fontwidth;
	Dimension fontascent;

	/* Colors (includes "xor" color) */
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
	offset(font), XtRString, "9x15" },

#if 1

	{ XtNcolor0, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[0]), XtRString, "black" },
	{ XtNcolor1, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[1]), XtRString, "white" },
	{ XtNcolor2, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[2]), XtRString, "#d7d7d7" },
	{ XtNcolor3, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[3]), XtRString, "#ff9200" },
	{ XtNcolor4, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[4]), XtRString, "#ff0000" },
	{ XtNcolor5, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[5]), XtRString, "#00cd00" },
	{ XtNcolor6, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[6]), XtRString, "#0000fe" },
	{ XtNcolor7, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[7]), XtRString, "#c86400" },
	{ XtNcolor8, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[8]), XtRString, "#a3a3a3" },
	{ XtNcolor9, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[9]), XtRString, "#ebebeb" },
	{ XtNcolor10, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[10]), XtRString, "#a500ff" },
	{ XtNcolor11, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[11]), XtRString, "#fffd00" },
	{ XtNcolor12, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[12]), XtRString, "#ff00bc" },
	{ XtNcolor13, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[13]), XtRString, "#00ff00" },
	{ XtNcolor14, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[14]), XtRString, "#00c8ff" },
	{ XtNcolor15, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[15]), XtRString, "#ffcc80" },

#else

	{ XtNcolor0, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[0]), XtRString, "black" },
	{ XtNcolor1, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[1]), XtRString, "white" },
	{ XtNcolor2, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[2]), XtRString, "#a6a6a6" },
	{ XtNcolor3, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[3]), XtRString, "#ff6302" },
	{ XtNcolor4, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[4]), XtRString, "#ca0808" },
	{ XtNcolor5, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[5]), XtRString, "#008e18" },
	{ XtNcolor6, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[6]), XtRString, "#0000e3" },
	{ XtNcolor7, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[7]), XtRString, "#814007" },
	{ XtNcolor8, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[8]), XtRString, "#6b6b6b" },
	{ XtNcolor9, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[9]), XtRString, "#d6d6d6" },
	{ XtNcolor10, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[10]), XtRString, "#5100c2" },
	{ XtNcolor11, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[11]), XtRString, "#fdf105" },
	{ XtNcolor12, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[12]), XtRString, "#ff9259" },
	{ XtNcolor13, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[13]), XtRString, "#26cf17" },
	{ XtNcolor14, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[14]), XtRString, "#02b2f2" },
	{ XtNcolor15, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[15]), XtRString, "#b28b48" },

#endif

	{ XtNcolor16, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[16]), XtRString, "#000000" },
	{ XtNcolor17, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[17]), XtRString, "#F0E0D0" },
	{ XtNcolor18, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[18]), XtRString, "#808080" },
	{ XtNcolor19, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[19]), XtRString, "#505050" },
	{ XtNcolor20, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[20]), XtRString, "#E0B000" },
	{ XtNcolor21, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[21]), XtRString, "#C0A070" },
	{ XtNcolor22, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[22]), XtRString, "#806040" },
	{ XtNcolor23, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[23]), XtRString, "#403020" },
	{ XtNcolor24, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[24]), XtRString, "#00A0F0" },
	{ XtNcolor25, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[25]), XtRString, "#0000F0" },
	{ XtNcolor26, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[26]), XtRString, "#000070" },
	{ XtNcolor27, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[27]), XtRString, "#F00000" },
	{ XtNcolor28, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[28]), XtRString, "#800000" },
	{ XtNcolor29, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[29]), XtRString, "#9000B0" },
	{ XtNcolor30, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[30]), XtRString, "#006010" },
	{ XtNcolor31, XtCColor, XtRPixel, sizeof(Pixel),
	offset(color[31]), XtRString, "#60F040" },

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
		/* resize               */      NULL,
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
		/* change_sensitive     */      XtInheritChangeSensitive
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
                             int x, int y, int w, int h, int color)
{
	/* Figure out which area to clear */
	y = y * widget->angband.fontheight + widget->angband.internal_border;
	x = x * widget->angband.fontwidth + widget->angband.internal_border;

	/* Clear the area */
	XFillRectangle(XtDisplay(widget), XtWindow(widget),
	               widget->angband.gc[color],
	               x, y, widget->angband.fontwidth*w,
	               widget->angband.fontheight*h);
}



/*
 * Output some text
 */
static void AngbandOutputText(AngbandWidget widget, int x, int y,
                              String txt, int len, int color)
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
	                 widget->angband.gc[color], x, y, txt, len);
}


#ifdef USE_GRAPHICS

/*
 * Draw some graphical characters.
 */
static void AngbandOutputPict(AngbandWidget widget, int x, int y, int n,
                              const byte *ap, const char *cp)
{
	int i;

	byte a;
	char c;

	/* Figure out where to place the text */
	y = (y * widget->angband.fontheight + widget->angband.internal_border);
	x = (x * widget->angband.fontwidth + widget->angband.internal_border);

	for (i = 0; i < n; ++i)
	{
		a = *ap++;
		c = *cp++;

		XPutImage(XtDisplay(widget), XtWindow(widget),
		          widget->angband.gc[17],
		          widget->angband.tiles,
		          (c&0x7F) * widget->angband.fontwidth + 1,
		          (a&0x7F) * widget->angband.fontheight + 1,
		          x, y,
		          widget->angband.fontwidth,
		          widget->angband.fontheight);

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
	XGCValues gcv;
	int depth = DefaultDepthOfScreen(XtScreen((Widget) wnew));
	TopLevelShellWidget parent =
	(TopLevelShellWidget)XtParent((Widget) wnew);
	int n;

	/* Fix the background color */
	wnew->core.background_pixel = wnew->angband.color[0];

	/* Get some information about the font */
	wnew->angband.fnt = getFont(wnew, wnew->angband.font, TRUE);
	wnew->angband.fontheight = wnew->angband.fnt->ascent +
	wnew->angband.fnt->descent;
	wnew->angband.fontwidth = wnew->angband.fnt->max_bounds.width;
	wnew->angband.fontascent = wnew->angband.fnt->ascent;

	/* Create and initialize the graphics contexts */ /* GXset? */
	gcv.font = wnew->angband.fnt->fid;
	gcv.graphics_exposures = FALSE;
	gcv.background = wnew->angband.color[0];
	for (n = 0; n < NUM_COLORS; n++)
	{
		if (depth == 1 && n >= 1)
		{
			gcv.foreground = wnew->angband.color[1];
		}
		else
		{
			gcv.foreground = wnew->angband.color[n];
		}

		wnew->angband.gc[n] = XtGetGC((Widget)wnew, GCFont | GCForeground |
		                             GCBackground | GCGraphicsExposures,
		                             &gcv);
	}

	/* Create a special GC for highlighting */
	gcv.foreground = (BlackPixelOfScreen(XtScreen((Widget)wnew)) ^
	                  WhitePixelOfScreen(XtScreen((Widget)wnew)));
	gcv.function = GXxor;
	wnew->angband.gc[COLOR_XOR] = XtGetGC((Widget)wnew, GCFunction |
	                                      GCGraphicsExposures |
	                                      GCForeground, &gcv);

	/* Calculate window geometry */
	wnew->core.height = (wnew->angband.start_rows * wnew->angband.fontheight +
	                     2 * wnew->angband.internal_border);
	wnew->core.width = (wnew->angband.start_columns * wnew->angband.fontwidth +
	                   2 * wnew->angband.internal_border);

	/* We need to be able to resize the Widget if the user want's to
	change font on the fly! */
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
	for (n = 0; n < NUM_COLORS+1; n++)
	{
		XtReleaseGC((Widget)widget, widget->angband.gc[n]);
	}

	/* Free the font */
	XFreeFont(XtDisplay((Widget)widget), widget->angband.fnt);
}


/*
 * Procedure Redisplay() is called as the result of an Expose event.
 * Use the redraw callback to do a full redraw
 */
static void Redisplay(AngbandWidget widget, XEvent *event, Region region)
{
	if (XtHasCallbacks((Widget)widget, XtNredrawCallback) == XtCallbackHasSome)
	{
		XtCallCallbacks((Widget)widget, XtNredrawCallback, NULL);
	}
}


/*
 * Font, colors and internal_border can be changed on the fly.
 * The entire widget is redrawn if any of those parameters change (all
 * can potentially have effects that spans the whole widget).
 */
static Boolean SetValues(AngbandWidget current, AngbandWidget request,
                         AngbandWidget wnew, ArgList args,
                         Cardinal *num_args)
{
	int depth = DefaultDepthOfScreen(XtScreen((Widget) wnew));
	Boolean font_changed = FALSE;
	Boolean border_changed = FALSE;
	Boolean color_changed = FALSE;
	XGCValues gcv;
	int height, width;
	int n;

	/* Changed font? */
	if (current->angband.font != wnew->angband.font)
	{
		/* Check if the font exists */
		wnew->angband.fnt = getFont(wnew, wnew->angband.font, FALSE);

		/* The font didn't exist */
		if (wnew->angband.fnt == NULL)
		{
			wnew->angband.fnt = current->angband.fnt;
			wnew->angband.font = current->angband.font;
			XtWarning("Couldn't find the request font!");
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

	/* Check all colors, if one or more has changed the redo all GC's */
	for (n = 0; n < NUM_COLORS; n++)
	{
		if (current->angband.color[n] != wnew->angband.color[n])
		{
			color_changed = TRUE;
		}
	}

	/* Change all GC's if color or font has changed */
	if (color_changed || font_changed)
	{
		gcv.font = wnew->angband.fnt->fid;
		gcv.graphics_exposures = FALSE;
		gcv.background = wnew->angband.color[0];

		/* Do all GC's */
		for (n = 0; n < NUM_COLORS; n++)
		{
			if (depth == 1 && n >= 1)
			{
				gcv.foreground = wnew->angband.color[1];
			}
			else
			{
				gcv.foreground = wnew->angband.color[n];
			}

			/* Release the old GC */
			XtReleaseGC((Widget)current, current->angband.gc[n]);

			/* Get the new GC */
			wnew->angband.gc[n] = XtGetGC((Widget)wnew, GCFont | GCForeground |
			                             GCBackground | GCGraphicsExposures,
			                             &gcv);
		}

		/* Replace the old XOR/highlighting GC */
		gcv.foreground = (BlackPixelOfScreen(XtScreen((Widget)wnew)) ^
				  WhitePixelOfScreen(XtScreen((Widget)wnew)));
		gcv.function = GXxor;
		XtReleaseGC((Widget)current, current->angband.gc[COLOR_XOR]);
		wnew->angband.gc[NUM_COLORS] = XtGetGC((Widget)wnew, GCFunction |
		                                      GCGraphicsExposures |
		                                      GCForeground, &gcv);

		/* Fix the background color */
		wnew->core.background_pixel = wnew->angband.color[0];
	}

	/* Check if internal border width has changed, used later */
	if (current->angband.internal_border != wnew->angband.internal_border)
	{
		border_changed = TRUE;
	}


	/* If the font or the internal border has changed, all geometry
	has to be recalculated */
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
	return (font_changed || color_changed || border_changed);
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



#ifndef IsModifierKey

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 * These were stolen from one of the X11 header files
 *
 * Also appears in "main-x11.c".
 */

#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#define IsCursorKey(keysym) \
  (((unsigned)(keysym) >= XK_Home)     && ((unsigned)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_F1)     && ((unsigned)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_F1)       && ((unsigned)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_Select)   && ((unsigned)(keysym) <  XK_KP_Space))

#define IsModifierKey(keysym) \
  (((unsigned)(keysym) >= XK_Shift_L)  && ((unsigned)(keysym) <= XK_Hyper_R))

#endif


/*
 * Checks if the keysym is a special key or a normal key
 * Assume that XK_MISCELLANY keysyms are special
 *
 * Also appears in "main-x11.c".
 */
#define IsSpecialKey(keysym) \
  ((unsigned)(keysym) >= 0xFF00)



/*
 * Maximum number of windows
 */
#define MAX_TERM_DATA 8


/*
 * Number of fallback resources per window
 */
#define TERM_FALLBACKS 6


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

	AngbandWidget widget;
};


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];


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
	{ XtNmaxRows,      24},
	{ XtNmaxColumns,   80}
};


/*
 * The default Arg's
 */
Arg defaultArgs[TERM_FALLBACKS] =
{
	{ XtNstartRows,    24},
	{ XtNstartColumns, 80},
	{ XtNminRows,      1},
	{ XtNminColumns,   1},
	{ XtNmaxRows,      24},
	{ XtNmaxColumns,   80}
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
errr CheckEvent(bool wait)
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
 * Handle a "special request"
 */
static errr Term_xtra_xaw(int n, int v)
{
	int i;

	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE:
		XBell(XtDisplay((Widget)data[0].widget), 100);
		return (0);

		/* Flush the output */
		case TERM_XTRA_FRESH:
		XFlush(XtDisplay((Widget)data[0].widget));
		/* Nonblock event-check so the flushed events can be showed */
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

		/* Delay */
		case TERM_XTRA_DELAY:
		usleep(1000 * v);
		return (0);

		/* Clear the window */
		case TERM_XTRA_CLEAR:
		for (i=0; i<MAX_TERM_DATA; i++)
		{
		    if (Term == &data[i].t)
			XClearWindow(XtDisplay((Widget)data[i].widget),
			             XtWindow((Widget)data[i].widget));
		}
		return (0);
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
 */
static errr Term_curs_xaw(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	/* Hilite the cursor character */
	AngbandClearArea(td->widget, x, y, 1, 1, COLOR_XOR);

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
	AngbandOutputText(td->widget, x, y, (String)s, n, (a & 0x0F));

	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS

/*
 * Draw some graphical characters.
 */
static errr Term_pict_xaw(int x, int y, int n, const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	/* Draw the pictures */
	AngbandOutputPict(td->widget, x, y, n, ap, cp);

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
                           ArgList widget_arg, Cardinal widget_arg_no)
{
	Widget parent;
	term *t = &td->t;

	/* Create the shell widget */
	parent = XtCreatePopupShell(name, topLevelShellWidgetClass, topLevel,
	                            NULL, 0);

	/* Create the interior widget */
	td->widget = (AngbandWidget)
	XtCreateManagedWidget(name, angbandWidgetClass,
	                      parent, widget_arg, widget_arg_no);

	/* Initialize the term (full size) */
	term_init(t, 80, 24, key_buf);

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

#ifdef USE_GRAPHICS

	if (use_graphics)
	{
		t->pict_hook = Term_pict_xaw;

		t->higher_pict = TRUE;
	}

#endif /* USE_GRAPHICS */

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

	/* Make it visible */
	XtPopup(parent, XtGrabNone);

	/* Activate (important) */
	Term_activate(t);

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

#ifdef USE_GRAPHICS

	char filename[1024];

	XImage *tiles_good = NULL;

#endif /* USE_GRAPHICS */


	/* Attempt to open the local display */
	dpy = XOpenDisplay("");

	/* Failure -- assume no X11 available */
	if (!dpy) return (-1);

	/* Close the local display */
	XCloseDisplay(dpy);


#ifdef USE_XAW_LANG
	/* Support locale processing */
	XtSetLanguageProc(NULL, NULL, NULL);
#endif


#ifdef USE_GRAPHICS

	/* Try graphics */
	if (arg_graphics)
	{
		/* Build the name of the "tiles.raw" file */
		path_build(filename, 1024, ANGBAND_DIR_XTRA, "tiles.raw");

		/* Use graphics if bitmap file exists */
		if (0 == fd_close(fd_open(filename, O_RDONLY)))
		{
			/* Use graphics */
			use_graphics = TRUE;
		}
	}

#endif /* USE_GRAPHICS */


	/* Load colors */
	if (use_graphics)
	{
		/* Process "graf-x11.prf" XXX XXX XXX */
		(void)process_pref_file("graf-x11.prf");
	}
	else
	{
		/* Process "font-x11.prf" XXX XXX XXX */
		(void)process_pref_file("font-x11.prf");
	}


	/* Initialize the toolkit XXX XXX XXX */
	topLevel = XtAppInitialize(&appcon, "Angband", NULL, 0, &argc, argv,
	                           fallback, NULL, 0);


	/* Initialize the windows */
	for (i=0; i<MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		term_data_init(td, topLevel, 1024, termNames[i],
		               (i == 0) ? specialArgs : defaultArgs,
		               TERM_FALLBACKS);

		angband_term[i] = Term;
	}


#ifdef USE_GRAPHICS

	/* Load graphics */
	if (use_graphics)
	{
		term_data *td = &data[0];

		Widget widget = (Widget)(td->widget);

		unsigned long ColTable[256];

		XImage *tiles_raw;

		dpy = XtDisplay(widget);

		/* Prepare color table */
		for (i = 0; i < 256; ++i)
		{
			XGCValues xgcv;

			XGetGCValues(dpy, td->widget->angband.gc[i],
			             GCForeground, &xgcv);

			ColTable[i] = xgcv.foreground;
		}

		/* Load the graphics XXX XXX XXX */
		tiles_raw = ReadRaw(dpy, filename, 256, 256);
		tiles_good = RemapColors(dpy, tiles_raw, ColTable);
		XDestroyImage(tiles_raw);
	}

	/* Load graphics */
	if (use_graphics)
	{
		/* Initialize the windows */
		for (i=0; i<MAX_TERM_DATA; i++)
		{
			term_data *td = &data[i];

			Widget widget = (Widget)(td->widget);

			dpy = XtDisplay(widget);

			/* Resize tiles */
			td->widget->angband.tiles =
			(ResizeImage(dpy, tiles_good, 8, 8,
			             td->widget->angband.fontwidth,
			             td->widget->angband.fontheight));
		}
	}

#endif /* USE_GRAPHICS */


	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Raise the "Angband" window */
	term_raise(&data[0]);

	/* Success */
	return (0);
}

#endif


