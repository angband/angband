/* File: r-infoclr.h */

#ifndef _R_INFOCLR_H
#define _R_INFOCLR_H

#include "h-include.h"

/*****************************************************************
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
 *	esch:  An escher pattern around a rectangle
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
 *
 *****************************************************************/

#include "x-infoclr.h"


/**** Available Functions ****/

/* Draw a Rectangle around Non Ordered Endpoints */
extern errr Infoclr_draw_rect_8 (/*X1,Y1,X2,Y2*/);

/* Draw an Oval around Non Ordered Endpoints */
extern errr Infoclr_draw_oval_8 (/*X1,Y1,X2,Y2*/);


/* Draw an Eschered Rectangle surrounding (X1,Y1) to (X2,Y2) */
extern errr Infoclr_draw_esch_4 (/*X1,Y1,X2,Y2,D*/);

/* Draw a Rectangle around Non Ordered Endpoints */
extern errr Infoclr_draw_esch_8 (/*X1,Y1,X2,Y2,D*/);


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

#define Infoclr_draw_esch_2(X,Y,W,H) \
	(Infoclr_draw_esch_4(X,Y,X+W,Y+H))


/**** Available Macros (Initial Point, Final Point) ****/

#define Infoclr_draw_rect_4(X1,Y1,X2,Y2) \
	(Infoclr_draw_rect_2(X1,Y1,X2-X1,Y2-Y1))

#define Infoclr_draw_oval_4(X1,Y1,X2,Y2) \
	(Infoclr_draw_oval_2(X1,Y1,X2-X1,Y2-Y1))


/**** Available Macros (Initial Point, Single Side) ****/

#define Infoclr_draw_esch_1(X,Y,S) \
	(Infoclr_draw_esch_2(X,Y,S,S))

#define Infoclr_draw_rect_1(X,Y,S) \
	(Infoclr_draw_rect_2(X,Y,S,S))

#define Infoclr_draw_oval_1(X,Y,S) \
	(Infoclr_draw_oval_2(X,Y,S,S))


/**** Available Macros (Initial Point, Radius) ****/

#define Infoclr_draw_esch_0(X,Y,R) \
	(Infoclr_draw_esch_2(X-R,Y-R,R+R,R+R))

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



#endif

