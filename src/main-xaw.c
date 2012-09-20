/* File: main-xaw.c */

/* Purpose: Support for X Athena Widget based Angband */
/* Most code written by Torbjörn Lindgren (tl@cd.chalmers.se) */

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


/*****************************************************
 *
 * Resource description
 *
 *****************************************************/

/* Resources:

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

*/

/*

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

Some older monochrome monitors have problem with white text on black
background. The new code can handle the reverse situation if the user
wants/needs this.

The following X Resources gives black text on white background using
Angband/Xaw. The other colors (2-15) isn't changed, since they're not
used on a monochrome monitor.

angband*color0: #ffffff
angband*color1: #000000

*/

/* New resource names */
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
#define XtNredrawCallback   "redrawCallback"

/* External definitions */
#define COLOR_XOR 16
#define NUM_COLORS 16

/* C Widget type definition */

typedef struct AngbandRec *AngbandWidget;

/* C Widget class type definition */

typedef struct AngbandClassRec *AngbandWidgetClass;


/*
 * New fields for the Angband widget record
 */

typedef struct
{
    /* Settable resources */
    int               start_rows;
    int               start_columns;
    int               min_rows;
    int               min_columns;
    int               max_rows;
    int               max_columns;
    int               internal_border;
    String            font;
    Pixel             color[NUM_COLORS];
    XtCallbackList    redraw_callbacks;

    /* Private state */
    XFontStruct       *fnt;
    Dimension         fontheight;
    Dimension         fontwidth;
    Dimension         fontascent;
    GC                gc[NUM_COLORS+1];  /* Includes a special 'xor' color */

} AngbandPart;


/*
 * Full instance record declaration
 */

typedef struct AngbandRec AngbandRec;

struct AngbandRec
{
    CorePart          core;
    SimplePart        simple;
    AngbandPart       angband;
};


/*
 * New fields for the Angband widget class record
 */

typedef struct AngbandClassPart AngbandClassPart;

struct AngbandClassPart
{
    int               dummy;
};


/*
 * Full class record declaration
 */

typedef struct AngbandClassRec AngbandClassRec;

struct AngbandClassRec
{
    CoreClassPart     core_class;
    SimpleClassPart   simple_class;
    AngbandClassPart  angband_class;
};



/* Angband widget, Created by Torbjörn Lindgren (tl@cd.chalmers.se) */

/*
 * Note that it isn't as self-contained as it really should be,
 * originally everything was output to a Pixmap which was later copied
 * to the screen when necessary. I had to abandon that idea since
 * Pixmaps creates big performance problems for some really old X
 * terminals (such as 3/50's running Xkernel).
 */


/*
 * The default colors used is based on the ones used in main-mac.c.
 * The main difference is that they are gamma corrected for a gamma of
 * 1.6.  MacOS do gamma correction afterwards, but X uses raw
 * colors. The Gamma of most color screens are about 1.5 - 1.7.
 * Color 12 was later changed a bit so that it didn't look as similar
 * to color 3/4.
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

#if 0

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

    { XtNredrawCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
      offset(redraw_callbacks), XtRCallback, (XtPointer)NULL }
};

#undef offset

/* Forward declarations for Widget functions */
static void Initialize(AngbandWidget request, AngbandWidget new);
static void Redisplay(AngbandWidget w, XEvent *event, Region region);
static Boolean SetValues(AngbandWidget current, AngbandWidget request,
                         AngbandWidget new, ArgList args, Cardinal *num_args);
static void Destroy(AngbandWidget widget);

/* Forward declaration for internal functions */
static void calculateSizeHints(AngbandWidget new);
static XFontStruct *getFont(AngbandWidget widget,
                            String font, Boolean fallback);


/* Class record constanst */
AngbandClassRec angbandClassRec =
{
    {
/* Core class fields initialization */
#define superclass              (&simpleClassRec)
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

/* Class record pointer */
WidgetClass angbandWidgetClass = (WidgetClass) &angbandClassRec;


/*
 * Public procedures
 */
static void AngbandOutputText(AngbandWidget widget, int x, int y,
                              String txt, int len, int color)
{

    /* Do nothing if the string is null */
    if (!txt || !*txt)
        return;

    /* Check the lenght, and fix it if it's below zero */
    if (len < 0)
        len = strlen(txt);

    /* Figure out where to place the text */
    y = y * widget->angband.fontheight + widget->angband.fontascent +
        widget->angband.internal_border;
    x = x * widget->angband.fontwidth + widget->angband.internal_border;

    /* Place the string */
    XDrawImageString (XtDisplay(widget), XtWindow(widget),
                      widget->angband.gc[color], x, y, txt, len);
}

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
 * Private procedures
 */

/*
 * Procedure Initialize() is called during the widget creation
 * process.  Initialize() load fonts and calculates window geometry.
 * The request parameter is filled in by parents to this widget.  The
 * new parameter is the request parameter plus data filled in by this
 * widget. All changes should be done to the new parameter.
 */
static void Initialize(AngbandWidget request, AngbandWidget new)
{
    XGCValues gcv;
    int depth = DefaultDepthOfScreen(XtScreen((Widget) new));
    TopLevelShellWidget parent =
        (TopLevelShellWidget)XtParent((Widget) new);
    int n;

    /* Fix the background color */
    new->core.background_pixel = new->angband.color[0];

    /* Get some information about the font */
    new->angband.fnt = getFont(new, new->angband.font, TRUE);
    new->angband.fontheight = new->angband.fnt->ascent +
                              new->angband.fnt->descent;
    new->angband.fontwidth = new->angband.fnt->max_bounds.width;
    new->angband.fontascent = new->angband.fnt->ascent;

    /* Create and initialize the graphics contexts */ /* GXset? */
    gcv.font = new->angband.fnt->fid;
    gcv.graphics_exposures = FALSE;
    gcv.background = new->angband.color[0];
    for (n = 0; n < NUM_COLORS; n++)
    {
        if (depth == 1 && n >= 1)
            gcv.foreground = new->angband.color[1];
        else
            gcv.foreground = new->angband.color[n];
        new->angband.gc[n] = XtGetGC((Widget)new, GCFont | GCForeground |
                                     GCBackground | GCGraphicsExposures,
                                     &gcv);
    }

    /* Create a special GC for highlighting */
    gcv.foreground = BlackPixelOfScreen(XtScreen((Widget)new)) ^
        WhitePixelOfScreen(XtScreen((Widget)new));
    gcv.function = GXxor;
    new->angband.gc[NUM_COLORS] = XtGetGC((Widget)new, GCFunction |
                                          GCGraphicsExposures |
                                          GCForeground, &gcv);

    /* Calculate window geometry */
    new->core.height = new->angband.start_rows * new->angband.fontheight +
        2 * new->angband.internal_border;
    new->core.width = new->angband.start_columns * new->angband.fontwidth +
        2 * new->angband.internal_border;

    /* We need to be able to resize the Widget if the user want's to
       change font on the fly! */
    parent->shell.allow_shell_resize = TRUE;

    /* Calculates all the size hints */
    calculateSizeHints(new);
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
        XtReleaseGC((Widget)widget, widget->angband.gc[n]);

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
        XtCallCallbacks((Widget)widget, XtNredrawCallback, NULL);
}

/*
 * Font, colors and internal_border can be changed on the fly.
 * The entire widget is redrawn if any of those parameters change (all
 * can potentially have effects that spans the whole widget).
 */
static Boolean SetValues(AngbandWidget current, AngbandWidget request,
                         AngbandWidget new, ArgList args,
                         Cardinal *num_args)
{
    int depth = DefaultDepthOfScreen(XtScreen((Widget) new));
    Boolean font_changed = FALSE;
    Boolean border_changed = FALSE;
    Boolean color_changed = FALSE;
    XGCValues gcv;
    int height, width;
    int n;

    /* Changed font? */
    if (current->angband.font != new->angband.font)
    {
        /* Check if the font exists */
        new->angband.fnt = getFont(new, new->angband.font, FALSE);

        /* The font didn't exist */
        if (new->angband.fnt == NULL)
        {
            new->angband.fnt = current->angband.fnt;
            new->angband.font = current->angband.font;
            XtWarning("Couldn't find the request font!");
        }
        else
        {
            font_changed = TRUE;
            /* Free the old font */
            XFreeFont(XtDisplay((Widget)new), current->angband.fnt);
            /* Update font information */
            new->angband.fontheight = new->angband.fnt->ascent +
                new->angband.fnt->descent;
            new->angband.fontwidth = new->angband.fnt->max_bounds.width;
            new->angband.fontascent = new->angband.fnt->ascent;
        }
    }

    /* Check all colors, if one or more has changed the redo all GC's */
    for (n = 0; n < NUM_COLORS; n++)
        if (current->angband.color[n] != new->angband.color[n])
            color_changed = TRUE;

    /* Change all GC's if color or font has changed */
    if (color_changed || font_changed)
    {
        gcv.font = new->angband.fnt->fid;
        gcv.graphics_exposures = FALSE;
        gcv.background = new->angband.color[0];

        /* Do all GC's */
        for (n = 0; n < NUM_COLORS; n++)
        {
            if (depth == 1 && n >= 1)
                gcv.foreground = new->angband.color[1];
            else
                gcv.foreground = new->angband.color[n];
            /* Release the old GC */
            XtReleaseGC((Widget)current, current->angband.gc[n]);
            /* Get the new GC */
            new->angband.gc[n] = XtGetGC((Widget)new, GCFont | GCForeground |
                                         GCBackground | GCGraphicsExposures,
                                         &gcv);
        }

        /* Replace the old XOR/highlighting GC */
        gcv.foreground = BlackPixelOfScreen(XtScreen((Widget)new)) ^
            WhitePixelOfScreen(XtScreen((Widget)new));
        gcv.function = GXxor;
        XtReleaseGC((Widget)current, current->angband.gc[NUM_COLORS]);
        new->angband.gc[NUM_COLORS] = XtGetGC((Widget)new, GCFunction |
                                              GCGraphicsExposures |
                                              GCForeground, &gcv);
        /* Fix the background color */
        new->core.background_pixel = new->angband.color[0];
    }

    /* Check if internal border width has changed, used later */
    if (current->angband.internal_border != new->angband.internal_border)
        border_changed = TRUE;


    /* If the font or the internal border has changed, all geometry
       has to be recalculated */
    if (font_changed || border_changed)
    {
        /* Change window size */
        height = (current->core.height - 2 * current->angband.internal_border) /
            current->angband.fontheight * new->angband.fontheight +
            2 * current->angband.internal_border;
        width = (current->core.width -  2 * current->angband.internal_border) /
            current->angband.fontwidth * new->angband.fontwidth +
            2 * new->angband.internal_border;

        /* Get the new width */
        if (XtMakeResizeRequest((Widget)new, width, height, NULL, NULL) ==
            XtGeometryNo)
        {
            /* Not allowed */
            XtWarning("Size change denied!");
        }
        else
        {
            /* Recalculate size hints */
            calculateSizeHints(new);
        }
    }

    /* Tell it to redraw the widget if anything has changed */
    return (font_changed || color_changed || border_changed);
}

/*
 * Calculate size hints
 */
static void calculateSizeHints(AngbandWidget new)
{
    TopLevelShellWidget parent =
        (TopLevelShellWidget)XtParent((Widget) new);

    /* Calculate minimum size */
    parent->wm.size_hints.min_height =
        new->angband.min_rows * new->angband.fontheight +
        2 * new->angband.internal_border;
    parent->wm.size_hints.min_width =
        new->angband.min_columns * new->angband.fontwidth +
        2 * new->angband.internal_border;
    parent->wm.size_hints.flags |= PMinSize;

    /* Calculate maximum size */
    parent->wm.size_hints.max_height =
        new->angband.max_rows * new->angband.fontheight +
        2 * new->angband.internal_border;
    parent->wm.size_hints.max_width =
        new->angband.max_columns * new->angband.fontwidth +
        2 * new->angband.internal_border;
    parent->wm.size_hints.flags |= PMaxSize;

    /* Calculate increment size */
    parent->wm.size_hints.height_inc = new->angband.fontheight;
    parent->wm.size_hints.width_inc = new->angband.fontwidth;
    parent->wm.size_hints.flags |= PResizeInc;

    /* Calculate base size */
    parent->wm.base_height = 2 * new->angband.internal_border;
    parent->wm.base_width = 2 * new->angband.internal_border;
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

  if (!(fnt = XLoadQueryFont(dpy,font)) && fallback)
  {
      sprintf(buf, "Can't find the font \"%s\", trying fixed\n", font);
      XtWarning(buf);
      if (!(fnt = XLoadQueryFont(dpy, "fixed")))
          XtError("Can't fint the font \"fixed\"!, bailing out\n");
  }

  return fnt;
}



/*** The non-widget code ****/

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
 */
#define IsSpecialKey(keysym) \
  ((unsigned)(keysym) >= 0xFF00)


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
 * The main screen
 */

static term_data screen;

static Arg angbandArgs[] =
{
    { XtNstartRows,    24},
    { XtNstartColumns, 80},
    { XtNminRows,      24},
    { XtNminColumns,   80},
    { XtNmaxRows,      24},
    { XtNmaxColumns,   80}
};


#ifdef GRAPHIC_MIRROR

/*
 * The mirror window
 */

static term_data mirror;

Arg mirrorArgs[] =
{
    { XtNstartRows,    24},
    { XtNstartColumns, 80},
    { XtNminRows,      1},
    { XtNminColumns,   1},
    { XtNmaxRows,      24},
    { XtNmaxColumns,   80}
};

#endif /* GRAPHIC_MIRROR */

#ifdef GRAPHIC_RECALL

/*
 * The "recall" window
 */

static term_data recall;

Arg recallArgs[] =
{
    { XtNstartRows,    8},
    { XtNstartColumns, 80},
    { XtNminRows,      1},
    { XtNminColumns,   1},
    { XtNmaxRows,      24},
    { XtNmaxColumns,   80}
};

#endif /* GRAPHIC_RECALL */

#ifdef GRAPHIC_CHOICE

/*
 * The "choice" window
 */

static term_data choice;

Arg choiceArgs[] =
{
    { XtNstartRows,    24},
    { XtNstartColumns, 80},
    { XtNminRows,      1},
    { XtNminColumns,   1},
    { XtNmaxRows,      24},
    { XtNmaxColumns,   80}
};

#endif /* GRAPHIC_CHOICE */



/*
 * The application context
 */
XtAppContext appcon;

/*
 * User changable information about widgets
 */
static String fallback[] =
{
    "Angband.angband.iconName:            Angband",
    "Angband.angband.title:               Angband",
    "Angband.mirror.iconName:             Mirror",
    "Angband.mirror.title:                Mirror",
    "Angband.recall.iconName:             Recall",
    "Angband.recall.title:                Recall",
    "Angband.choice.iconName:             Choice",
    "Angband.choice.title:                Choice",
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
 */
static void react_keypress(XKeyEvent *ev)
{
    int i, n, mc, ms, mo, mx;

    KeySym ks;

    char buf[128];
    char msg[128];


    /* Check for "normal" keypresses */
    n = XLookupString(ev, buf, 125, &ks, NULL);

    /* Terminate */
    buf[n] = '\0';

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
    switch (ks)
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
    for (i = 0; msg[i]; i++)
        Term_keypress(msg[i]);

    /* Hack -- dump an "extra" string */
    if (n)
    {
        /* Start the "extra" string */
        Term_keypress(28);

        /* Enqueue the "real" string */
        for (i = 0; buf[i]; i++)
            Term_keypress(buf[i]);

        /* End the "extra" string */
        Term_keypress(28);
    }
}


static void handle_event (Widget widget, XtPointer client_data, XEvent *event,
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
            react_keypress(&(event->xkey));
            *continue_to_dispatch = FALSE; /* We took care of the event */
            break;

        default:
            break;  /* Huh? Shouldn't happen! */
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
    /* Handle a subset of the legal requests */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE:
            XBell(XtDisplay((Widget)screen.widget), 100);
            return (0);

        /* Flush the output */
        case TERM_XTRA_FRESH:
            XFlush(XtDisplay((Widget)screen.widget));
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

        /* Clear the window */
        case TERM_XTRA_CLEAR:

            /* Screen */
            if ( Term == &screen.t )
            {
                XClearWindow(XtDisplay((Widget)screen.widget),
                             XtWindow((Widget)screen.widget));
            }

#ifdef GRAPHIC_MIRROR
            /* Mirror */
            if ( Term == &mirror.t )
            {
                XClearWindow(XtDisplay((Widget)mirror.widget),
                             XtWindow((Widget)mirror.widget));
            }
#endif /* GRAPHIC_MIRROR */

#ifdef GRAPHIC_RECALL
            /* Recall */
            if ( Term == &recall.t )
            {
                XClearWindow(XtDisplay((Widget)recall.widget),
                             XtWindow((Widget)recall.widget));
            }
#endif /* GRAPHIC_RECALL */

#ifdef GRAPHIC_CHOICE
            /* Choice */
            if ( Term == &choice.t )
            {
                XClearWindow(XtDisplay((Widget)choice.widget),
                             XtWindow((Widget)choice.widget));
            }
#endif /* GRAPHIC_CHOICE */

            return(0);
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
 * Draw the cursor (XXX by hiliting)
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


/*
 * Raise a term
 */
static void term_raise(term_data win)
{
    Widget widget = (Widget)win.widget;

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
        XtCreateManagedWidget (name, angbandWidgetClass,
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
 * Mega-Hack -- we are not given the actual "argc" and "argv" from
 * the main program, so we fake one.  Thus, we are unable to parse
 * any "display" requests for external devices.  This is okay, since
 * we need to verify the display anyway, to work with "main.c".
 */
errr init_xaw(void)
{
    int argc;
    char *argv[2];
    Widget topLevel;
    Display *dpy;


    /* One fake argument */
    argc = 1;

    /* Save the program name */
    argv[0] = argv0;

    /* Terminate */
    argv[1] = NULL;


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

    /* Initialize the toolkit */
    topLevel = XtAppInitialize (&appcon, "Angband", NULL, 0, &argc, argv,
                                fallback, NULL, 0);

    /* Initialize the main window */
    term_data_init (&screen, topLevel, 1024, "angband",
                    angbandArgs, XtNumber(angbandArgs));
    term_screen = Term;

#ifdef GRAPHIC_MIRROR
    /* Initialize the mirror window */
    term_data_init (&mirror, topLevel, 16, "mirror",
                    mirrorArgs, XtNumber(mirrorArgs));
    term_mirror = Term;
#endif /* GRAPHIC_MIRROR */

#ifdef GRAPHIC_RECALL
    /* Initialize the recall window */
    term_data_init (&recall, topLevel, 16, "recall",
                    recallArgs, XtNumber(recallArgs));
    term_recall = Term;
#endif /* GRAPHIC_RECALL */

#ifdef GRAPHIC_CHOICE
    /* Initialize the choice window */
    term_data_init (&choice, topLevel, 16, "choice",
                    choiceArgs, XtNumber(choiceArgs));
    term_choice = Term;
#endif /* GRAPHIC_CHOICE */

    /* Activate the "Angband" window screen */
    Term_activate(&screen.t);

    /* Raise the "Angband" window */
    term_raise(screen);

    /* Success */
    return (0);
}

#endif

