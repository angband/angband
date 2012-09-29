/* File: r-infowin.c */

#include "r-infowin.h"

#include "x-metadpy.h"

#include "x-infoclr.h"
#include "x-infowin.h"



/*
 * Modify the event mask of an Infowin
 */

errr Infowin_set_mask (long mask)
{
  /* Save the new setting */
  Infowin->mask = mask;

  /* Execute the Mapping */
  XSelectInput (Metadpy->dpy, Infowin->win, Infowin->mask);

  /* Success */
  return (0);
}








/*
 * Request that Infowin be mapped
 */

errr Infowin_map ()
{
  /* Execute the Mapping */
  XMapWindow (Metadpy->dpy, Infowin->win);

  /* Success */
  return (0);
}


/*
 * Request that Infowin be unmapped
 */

errr Infowin_unmap ()
{
  /* Execute the Un-Mapping */
  XUnmapWindow (Metadpy->dpy, Infowin->win);

  /* Success */
  return (0);
}



/*
 * Request that Infowin be raised
 */

errr Infowin_raise ()
{
  /* Raise towards visibility */
  XRaiseWindow (Metadpy->dpy, Infowin->win);

  /* Success */
  return (0);
}


/*
 * Request that Infowin be lowered
 */

errr Infowin_lower ()
{
  /* Lower towards invisibility */
  XLowerWindow (Metadpy->dpy, Infowin->win);

  /* Success */
  return (0);
}





/*
 * Request that Infowin be moved to a new location
 */

errr Infowin_impell (int x, int y)
{
  /* Execute the request */
  XMoveWindow (Metadpy->dpy, Infowin->win, x, y);

  /* Success */
  return (0);
}



/*
 * Resize an infowin
 */

errr Infowin_resize (int w, int h)
{
  /* Execute the request */
  XResizeWindow (Metadpy->dpy, Infowin->win, w, h);

  /* Success */
  return (0);
}


/*
 * Move and Resize an infowin
 */

errr Infowin_locate (int x, int y, int w, int h)
{
  /* Execute the request */
  XMoveResizeWindow (Metadpy->dpy, Infowin->win, x, y, w, h);

  /* Success */
  return (0);
}






/*
 * Visually clear Infowin
 */

errr Infowin_wipe ()
{
  /* Execute the request */
  XClearWindow (Metadpy->dpy, Infowin->win);

  /* Success */
  return (0);
}


/*
 * Visually Paint Infowin with the current color
 */

errr Infowin_fill ()
{
  /* Execute the request */
  XFillRectangle (Metadpy->dpy, Infowin->win, Infoclr->gc,
		  0, 0, Infowin->w, Infowin->h);

  /* Success */
  return (0);
}


/*
 * Refresh Infowin by briefly covering it.
 * May only works for the root window infowin.
 */

errr Infowin_refresh ()
{
  Window cover;
  XSetWindowAttributes xswa;

  /* Use the same Pixmap the Window uses */
  xswa.background_pixmap = ParentRelative;

  /* Allow instant Map/Lower, etc */
  xswa.override_redirect = True;

  /* Create an obscuring Window */
  cover = XCreateWindow (Metadpy->dpy, Infowin->win,
			 0, 0, Infowin->w, Infowin->h, 0,
			 CopyFromParent, InputOutput, CopyFromParent,
			 CWBackPixmap | CWOverrideRedirect, &xswa);

  /* Lower the cover Window to the bottom */
  XLowerWindow (Metadpy->dpy, cover);

  /* Map the cover Window */
  XMapWindow (Metadpy->dpy, cover);

  /* Kill the cover window */
  XDestroyWindow (Metadpy->dpy, cover);

  /* Flush these refreshing events */
  XFlush (Metadpy->dpy);

  /* Success */
  return (0);
}



