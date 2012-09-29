/* File: r-infoclr.c */

#include "r-infoclr.h"

#include "x-metadpy.h"

#include "x-infoclr.h"
#include "x-infowin.h"





/*
 * Draw a rectangle (in free floating corners format)
 */

errr Infoclr_draw_rect_8 (int x1, int y1, int x2, int y2)
{
  register int tmp;

  /* Force (x1<x2) and (y1<y2) */
  if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
  if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

  /* Send to the ordered corner routine */
  return (Infoclr_draw_rect_4 (x1, y1, x2, y2));
}



/*
 * Draw an oval (in free floating corners format)
 */

errr Infoclr_draw_oval_8 (int x1, int y1, int x2, int y2)
{
  register int tmp;

  /* Force (x1<x2) and (y1<y2) */
  if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
  if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

  /* Send to the ordered corner routine */
  return (Infoclr_draw_oval_4 (x1, y1, x2, y2));
}



/*
 * Fill a rectangle (in free floating corners format)
 */

errr Infoclr_fill_rect_8 (int x1, int y1, int x2, int y2)
{
  register int tmp;

  /* Force (x1<x2) and (y1<y2) */
  if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
  if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

  /* Send to the ordered corner routine */
  return (Infoclr_fill_rect_4 (x1, y1, x2, y2));
}



/*
 * Fill an oval (in free floating corners format)
 */

errr Infoclr_fill_oval_8 (int x1, int y1, int x2, int y2)
{
  register int tmp;

  /* Force (x1<x2) and (y1<y2) */
  if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
  if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

  /* Send to the ordered corner routine */
  return (Infoclr_fill_oval_4 (x1, y1, x2, y2));
}







/*
 * Data for the M.C.Escher Impossible Frame
 */

#define MAX_ESCHER 4

static int en[MAX_ESCHER] =
  { 1, 2, 3, 0 };

static int ex[MAX_ESCHER][MAX_ESCHER] =
  { {0, 0, 1, 2}, {0, 1, 2, 1}, {0, 0, -1, -2}, {0, -1, -2, -1}};

static int ey[MAX_ESCHER][MAX_ESCHER] =
  {{0, 1, 2, 1}, {0, 0, -1, -2}, {0, -1, -2, -1}, {0, 0, 1, 2}};



/*
 * Draw an Escher Box around the rectangle (x1,y1) to (x2,y2)
 *
 * Inputs:
 *	x1,y1: The top left corner
 *	x2,y2: The bottom right corner
 *	d:     The Distance between Lines
 */

errr Infoclr_draw_esch_4 (int x1, int y1, int x2, int y2, int d)
{
  /* Current corner */
  register int c;

  /* Next corner */
  register int n;

  /* Corner arrays */
  int px[4], py[4];


  /* Innermost edge surrounds a rectangle */
  Infoclr_draw_rect_4 (x1, y1, x2, y2);

  /* If 'frame' is too small, just use the rectangle */
  if (d < 2) return (-1);

  /* Get the initial corner points */
  px[0] = x2; py[0] = y2;
  px[1] = x2; py[1] = y1;
  px[2] = x1; py[2] = y1;
  px[3] = x1; py[3] = y2;

  /* Connect each corner to itself and the next corner */
  for (c = 0; c < MAX_ESCHER; ++c)
  {
    /* Access: Number of Next Corner */
    n = en[c];

    /* Draw: 2c to 3c self-connect */
    Infoclr_draw_line_8 (
      px[c] + (d * ex[c][2]), py[c] + (d * ey[c][2]),
      px[c] + (d * ex[c][3]), py[c] + (d * ey[c][3])
    );

    /* Draw: 1c to 0n */
    Infoclr_draw_line_8 (
      px[c] + (d * ex[c][1]), py[c] + (d * ey[c][1]),
      px[n] + (d * ex[n][0]), py[n] + (d * ey[n][0])
    );

    /* Draw: 2c to 1n */
    Infoclr_draw_line_8 (
      px[c] + (d * ex[c][2]), py[c] + (d * ey[c][2]),
      px[n] + (d * ex[n][1]), py[n] + (d * ey[n][1])
    );

    /* Draw: 3c to 2n */
    Infoclr_draw_line_8 (
      px[c] + (d * ex[c][3]), py[c] + (d * ey[c][3]),
      px[n] + (d * ex[n][2]), py[n] + (d * ey[n][2])
    );
  }

  /* Success */
  return (0);
}


/*
 * Draw an Escher'd rectangle (in free floating corners format)
 */

errr Infoclr_draw_esch_8 (int x1, int y1, int x2, int y2, int d)
{
  register int tmp;

  /* Force (x1<x2) and (y1<y2) */
  if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
  if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

  /* Send to the ordered corner routine */
  return (Infoclr_draw_esch_4 (x1, y1, x2, y2, d));
}






/*
 * Draw the top plane of a True Pixmap
 */

errr Infoclr_draw_pixmap (int x, int y, uint w, uint h, Drawable pixmap)
{
  /* Copy the pixmap onto the screen at topleft (x,y) size (w,h) */
  XCopyPlane (Metadpy->dpy, pixmap, Infowin->win, Infoclr->gc,
	      0, 0, w, h, x, y, 0x1);

  /* Success */
  return (0);
}

