/* File: xtra-x11.h */

/* Purpose: somewhat generic support for main-x11.c */

#ifndef INCLUDED_XTRA_X11_H
#define INCLUDED_XTRA_X11_H

#include "h-include.h"



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



/* Include the X definitions */

#ifndef __MAKEDEPEND__
# include <X11/Xlib.h>
# include <X11/Xutil.h>
#endif /* __MAKEDEPEND__ */




/**** Available Constants ****/

/*
 * Number of X11-degrees in a circle
 */
#define OVAL_FULL (360*64)

/*
 * Some simple font names
 */
#define fnt_small       "fixed"
#define fnt_medium      "-misc-fixed-medium-r-*-15-*"
#define fnt_large       "*-courier-bold-r-*-24-*"




/**** Available Types ****/

/*
 * An X11 pixell specifier
 */
typedef unsigned long Pixell;

/*
 * The structures defined below
 */
typedef struct _metadpy metadpy;
typedef struct _infowin infowin;
typedef struct _infoclr infoclr;
typedef struct _infofnt infofnt;


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

struct _metadpy {

  Display	*dpy;
  Screen	*screen;
  Window	root;
  Colormap	cmap;

  char		*name;

  int		fd;

  uint		width;
  uint		height;
  uint		depth;

  Pixell	black;
  Pixell	white;

  Pixell	bg;
  Pixell	fg;
  Pixell	zg;

  uint		mono:1;
  uint		color:1;
  uint		nuke:1;
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

struct _infowin {

  Window		win;
  long			mask;

  s16b			x, y;
  s16b			w, h;
  u16b			b;

  byte			byte1;

  uint			mapped:1;
  uint			redraw:1;
  uint			resize:1;

  uint			nuke:1;

  uint			flag1:1;
  uint			flag2:1;
  uint			flag3:1;
  uint			flag4:1;
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

struct _infoclr {

  GC			gc;

  Pixell		fg;
  Pixell		bg;

  uint			code:4;
  uint			stip:1;
  uint			nuke:1;
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

struct _infofnt {

  XFontStruct	*info;

  cptr			name;

  s16b			wid;
  s16b			hgt;
  s16b			asc;

  byte			off;

  uint			mono:1;
  uint			nuke:1;
};






/**** Available Variables ****/


/*
 * The "current" structures
 */
extern metadpy *Metadpy;
extern infowin *Infowin;
extern infoclr *Infoclr;
extern infofnt *Infofnt;







/* OPEN: x-metadpy.h */




/**** Available Functions ****/

/* Initialize a new metadpy */
extern errr Metadpy_init_2();

/* Nuke an existing metadpy */
extern errr Metadpy_nuke();



/**** Available Macros ****/


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



/**** Other Functions ****/

/* Do Flush/Sync/Discard */
extern errr Metadpy_update (/*F,S,D*/);

/* Prepare/Produce a sound */
extern errr Metadpy_prepare_sound (/*V,P,D*/);
extern errr Metadpy_produce_sound (/*V*/);

/* Make a beep */
extern errr Metadpy_do_beep ();


/* SHUT: x-metadpy.h */


/* OPEN: x-infowin.h */



/**** Available Functions ****/

extern errr Infowin_nuke ();
extern errr Infowin_init_real ();
extern errr Infowin_init_data ();

extern errr Infowin_set_name ();
extern errr Infowin_set_icon_name ();


/**** Passable Functions for ADT's and such ***/

extern vptr infowin_datakey(/*W*/);
extern uint infowin_keyhash(/*K,S*/);
extern int infowin_keycomp(/*K1,K2*/);


/**** Available Macros ****/

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



/* SHUT: x-infowin.h */


/* OPEN: x-infoclr.h */



/**** Available Functions ****/

/* Convert an Opcode name to an Opcode (i.e. "cpy") */
extern int Infoclr_Opcode(/*N*/);

/* Convert a Pixell Name to a Pixell (i.e. "red") */
extern Pixell Infoclr_Pixell(/*N*/);


/* Nuke Infoclr */
extern errr Infoclr_nuke();

/* Initialize Infoclr from a 'gc' */
extern errr Infoclr_init_1(/*G*/);

/* Initialize Infoclr from some data (Op, Fg, Bg, Stip) */
extern errr Infoclr_init_data(/*F,B,O,S*/);



/**** Available Macros  ****/

/* Set the current Infoclr */
#define Infoclr_set(C) \
	(Infoclr = (C))



/**** Available Macros (Requests) ****/

#define Infoclr_init_ppo(F,B,O,M) \
	Infoclr_init_data(F,B,O,M)

#define Infoclr_init_cco(F,B,O,M) \
	Infoclr_init_ppo(Infoclr_Pixell(F),Infoclr_Pixell(B),O,M)

#define Infoclr_init_ppn(F,B,O,M) \
	Infoclr_init_ppo(F,B,Infoclr_Opcode(O),M)

#define Infoclr_init_ccn(F,B,O,M) \
	Infoclr_init_cco(F,B,Infoclr_Opcode(O),M)


/* SHUT: x-infoclr.h */


/* OPEN: x-infofnt.h */



/**** Available Functions ****/

extern errr Infofnt_nuke();
extern errr Infofnt_init_real();
extern errr Infofnt_init_data();



/**** Available Macros ****/

/* Set the current infofnt */
#define Infofnt_set(I) \
	(Infofnt = (I))


/* SHUT: x-infofnt.h */



/* OPEN: r-metadpy.h */

/**** Available Functions ****/

extern Pixmap Pixmap_from_data (/*bits,w,h*/);
extern Cursor Cursor_from_data (/*bits,mask,w,h,x,y*/);


/* SHUT: r-metadpy.h */


/* OPEN: r-infowin.h */

/**** Available Functions ****/


/* Select events for an Infowin */
extern errr Infowin_set_mask(/*M*/);


/* Map/Unmap an Infowin */
extern errr Infowin_map();
extern errr Infowin_unmap();

/* Raise/Lower Infowin */
extern errr Infowin_raise();
extern errr Infowin_lower();

/* Impell Infowin to a new location */
extern errr Infowin_impell(/*X,Y*/);

/* Resize Infowin to a new size */
extern errr Infowin_resize(/*W,H*/);

/* Impell AND Resize an info_win to a new location & size */
extern errr Infowin_locate(/*X,Y,W,H*/);


/* Wipe an info_win clean */
extern errr Infowin_wipe();

/* Fill an info_win */
extern errr Infowin_fill();

/* Refresh an info_win by a weird Root based) method */
extern errr Infowin_refresh();


#if 0

/* Copy a Pixmap onto an info_win */
extern errr Metadpy_draw_pixmap(/*X,Y,W,H,P*/);

#endif



/**** Available macros ****/

/* Errr: Expose Infowin */
#define Infowin_expose() \
	(!(Infowin->redraw = 1))

/* Errr: Unxpose Infowin */
#define Infowin_unexpose() \
	(Infowin->redraw = 0)


/* SHUT: r-infowin.h */


/* OPEN: r-infoclr.h */

/*
 *
 * Optional support for the infoclr structure
 *
 * Functions included deal with drawing/filling lines and shapes.
 *
 * There are two types of routines: 'draw' and 'fill' routines.
 *
 * The routines are named 'Infoclr_OPER_TYPE_MODE (ARGS)'
 *	Ex: Infoclr_draw_line_8 (X1,Y1,X2,Y2)
 *
 * The arguments are:
 *	W:    An (info_win*) saying where to do it
 *	C:    An (info_clr*) saying how (what color, etc) to do it
 *	ARGS: Various parms, like Coordinates (see 'MODE' below)
 *
 * The OPER's are:
 *	draw: Usually involve drawing an outline (or a line/ point)
 *	fill: Usually involve filling something
 *
 * The TYPE's are:
 *	point: A single point (no 'MODE' allowed)
 *	line:  A line (mode 8 == mode 4)
 *	rect:  A rectangle (see 'square')
 *	oval:  An oval (see 'circle')
 *
 * The MODE's are:
 *	0: Takes a center point and a radius
 *	1: Takes a top left point and a side length
 *	2: Takes a top left point and a width+height
 *	4: Takes a top left point and a bottom right point
 *	8: Takes any two points, no ordering necessary
 *
 * General note:
 *	Keep in mind that due to the functioning of internal routines,
 *	filling a rectangle is one pixel different from drawing it.
 *	This could perhaps be fixed via changing the params to draw.
 */



/**** Available Functions ****/

/* Draw a Rectangle around Non Ordered Endpoints */
extern errr Infoclr_draw_rect_8 (/*X1,Y1,X2,Y2*/);

/* Draw an Oval around Non Ordered Endpoints */
extern errr Infoclr_draw_oval_8 (/*X1,Y1,X2,Y2*/);


/* Draw a pixmap (P) at (X,Y) with size (W,H) */
extern errr Infoclr_draw_pixmap(/*X,Y,W,H,P*/);




/**** Available Macros (that Map directly to X commands) ****/

#define Infoclr_draw_point(X,Y) \
	(XDrawPoint(Metadpy->dpy,Infowin->win,Infoclr->gc,X,Y),0)

#define Infoclr_draw_line_4(X1,Y1,X2,Y2) \
	(XDrawLine(Metadpy->dpy,Infowin->win,Infoclr->gc,X1,Y1,X2,Y2),0)

#define Infoclr_draw_rect_2(X,Y,W,H) \
	(XDrawRectangle(Metadpy->dpy,Infowin->win,Infoclr->gc,X,Y,W,H),0)

#define Infoclr_draw_oval_2(X,Y,W,H) \
	(XDrawArc(Metadpy->dpy,Infowin->win,Infoclr->gc,X,Y,W,H,0,OVAL_FULL),0)


/**** Available Macros (Initial Point, Width Height) ****/

#define Infoclr_draw_line_2(X,Y,W,H) \
	(Infoclr_draw_line_4(X,Y,X+W,Y+H))


/**** Available Macros (Initial Point, Final Point) ****/

#define Infoclr_draw_rect_4(X1,Y1,X2,Y2) \
	(Infoclr_draw_rect_2(X1,Y1,X2-X1,Y2-Y1))

#define Infoclr_draw_oval_4(X1,Y1,X2,Y2) \
	(Infoclr_draw_oval_2(X1,Y1,X2-X1,Y2-Y1))


/**** Available Macros (Initial Point, Single Side) ****/

#define Infoclr_draw_rect_1(X,Y,S) \
	(Infoclr_draw_rect_2(X,Y,S,S))

#define Infoclr_draw_oval_1(X,Y,S) \
	(Infoclr_draw_oval_2(X,Y,S,S))


/**** Available Macros (Initial Point, Radius) ****/

#define Infoclr_draw_rect_0(X,Y,R) \
	(Infoclr_draw_rect_2(X-R,Y-R,R+R,R+R))

#define Infoclr_draw_oval_0(X,Y,R) \
	(Infoclr_draw_oval_2(X-R,Y-R,R+R,R+R))



/**** Simple (Draw) Aliases ****/

#define Infoclr_draw_line_8	Infoclr_draw_line_4

#define Infoclr_draw_square_1	Infoclr_draw_rect_1
#define Infoclr_draw_square_0	Infoclr_draw_rect_0

#define Infoclr_draw_circle_1	Infoclr_draw_oval_1
#define Infoclr_draw_circle_0	Infoclr_draw_oval_0





/**** Available (Fill) Functions ****/

/* Fill a Rectangle around Non Ordered Endpoints */
extern errr Infoclr_fill_rect_8 (/*X1,Y1,X2,Y2*/);

/* Fill a Rectangle around Non Ordered Endpoints */
extern errr Infoclr_fill_oval_8 (/*X1,Y1,X2,Y2*/);




/**** Available (Fill) Macros (that Map directly to X commands) ****/

#define Infoclr_fill_rect_2(X,Y,W,H) \
	(XFillRectangle(Metadpy->dpy,Infowin->win,Infoclr->gc,X,Y,W,H),0)

#define Infoclr_fill_oval_2(X,Y,W,H) \
	(XFillArc(Metadpy->dpy,Infowin->win,Infoclr->gc,X,Y,W,H,0,OVAL_FULL),0)


/**** Available (Fill) Macros (Initial Point, Final Point) ****/

#define Infoclr_fill_rect_4(X1,Y1,X2,Y2) \
	(Infoclr_fill_rect_2(X1,Y1,X2-X1,Y2-Y1))

#define Infoclr_fill_oval_4(X1,Y1,X2,Y2) \
	(Infoclr_fill_oval_2(X1,Y1,X2-X1,Y2-Y1))


/**** Available (Fill) Macros (Initial Point, Single Side) ****/

#define Infoclr_fill_rect_1(X,Y,S) \
	(Infoclr_fill_rect_2(X,Y,S,S))

#define Infoclr_fill_oval_1(X,Y,S) \
	(Infoclr_fill_oval_2(X,Y,S,S))


/**** Available (Fill) Macros (Initial Point, Radius) ****/

#define Infoclr_fill_rect_0(X,Y,R) \
	(Infoclr_fill_rect_2(X-R,Y-R,R+R,R+R))

#define Infoclr_fill_oval_0(X,Y,R) \
	(Infoclr_fill_oval_2(X-R,Y-R,R+R,R+R))




/**** Simple (Fill) Aliases ****/

#define Infoclr_fill_square_1	Infoclr_fill_rect_1
#define Infoclr_fill_square_0	Infoclr_fill_rect_0

#define Infoclr_fill_circle_1	Infoclr_fill_oval_1
#define Infoclr_fill_circle_0	Infoclr_fill_oval_0


/* SHUT: r-infoclr.h */


/* OPEN: r-infofnt.h */

/*
 * Explanation of Text Flags:
 *
 * Horizontal placement:
 *   TEXT_J_LT:    Line up text with its leftmost edge at 'x'
 *   TEXT_J_RT:    Line up text with its right-most edge at 'x'
 *   Neither:      Center text, snap to GRID, exact FILL
 *   Both:         Center text, shift GRID, infinite FILL
 *
 * Vertical placement:
 *   TEXT_J_UP:    Line up text with its top at 'y'
 *   TEXT_J_DN:    Line up text with its bottom at 'y'
 *   Default:      Center text around 'y'
 *   Both:         Line up text with its baseline at 'y'
 *
 * Special:
 *   TEXT_QUERY:   Query for font info (vs Use Infofnt)
 *   TEXT_GRID:    Count rows and columns (vs Count Pixels)
 *   TEXT_FILL:    Paint a rectangle (vs Draw Actual Text)
 *   TEXT_WIPE:    Erase before drawing (vs Stipple Draw)
 */


/**** Available Constants ****/

/* Simple Flag Combinations for the Text Drawing routines */

#define TEXT_NONE       0x00

#define TEXT_QUERY      0x01

#define TEXT_GRID       0x02
#define TEXT_FILL       0x04
#define TEXT_WIPE       0x08

#define TEXT_J_LT       0x10
#define TEXT_J_RT       0x20
#define TEXT_J_UP       0x40
#define TEXT_J_DN       0x80



/**** Available Functions ****/


/* Draw (or Fill) Text (or Paint) on the Screen (using various flags) */
extern errr Infofnt_text (/*X,Y,S,L,M*/);


/* SHUT: r-infofnt.h */


#endif	/* INCLUDED_XTRA_X11_H */

