/* File: x-infowin.h */

#include "x-infowin.h"

#include "z-util.h"
#include "z-virt.h"


#ifndef __MAKEDEPEND__
#include <X11/Xutil.h>
#endif /* __MAKEDEPEND__ */



/*** The current infowin ***/

infowin *Infowin = (infowin*)(NULL);




/*
 * Set the name (in the title bar) of Infowin
 */

errr Infowin_set_name (char *name)
{
  Status st;
  XTextProperty tp;
  st = XStringListToTextProperty (&name, 1, &tp);
  if (st) XSetWMName (Metadpy->dpy, Infowin->win, &tp);
  return (0);
}


/*
 * Set the icon name of Infowin
 */

errr Infowin_set_icon_name (char *name)
{
  Status st;
  XTextProperty tp;
  st = XStringListToTextProperty (&name, 1, &tp);
  if (st) XSetWMIconName (Metadpy->dpy, Infowin->win, &tp);
  return (0);
}



/*
 * Nuke Infowin
 */

errr Infowin_nuke ()
{
  infowin *iwin = Infowin;

  /* Nuke if requested */
  if (iwin->nuke)
  {
    /* Destory the old window */
    XDestroyWindow (Metadpy->dpy, iwin->win);
  }

  /* Success */
  return (0);
}





/*
 * Prepare a new 'infowin'.
 */

static errr Infowin_prepare (Window xid)
{
  infowin *iwin = Infowin;

  Window tmp_win;
  XWindowAttributes xwa;
  int x, y, w, h, b, d;

  /* Assign stuff */
  iwin->win = xid;

  /* Check For Error XXX Extract some ACTUAL data from 'xid' */
  XGetGeometry (Metadpy->dpy, xid, &tmp_win, &x, &y, &w, &h, &b, &d);

  /* Apply the above info */
  iwin->x = x;
  iwin->y = y;
  iwin->w = w;
  iwin->h = h;
  iwin->b = b;

  /* Check Error XXX Extract some more ACTUAL data */
  XGetWindowAttributes (Metadpy->dpy, xid, &xwa);

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

errr Infowin_init_real (Window xid)
{
  /* Wipe it clean */
  WIPE(Infowin, infowin);

  /* Start out non-nukable */
  Infowin->nuke = 0;

  /* Attempt to Prepare ourself */
  return (Infowin_prepare (xid));
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

errr Infowin_init_data (Window dad, int x, int y, int w, int h,
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
  xid = XCreateSimpleWindow (Metadpy->dpy, dad, x, y, w, h, b, fg, bg);

  /* Start out selecting No events */
  XSelectInput (Metadpy->dpy, xid, 0L);


  /*** Prepare the new infowin ***/

  /* Mark it as nukable */
  Infowin->nuke = 1;

  /* Attempt to Initialize the infowin */
  return (Infowin_prepare (xid));
}







/*
 * Extract a key (its XID) from an infowin
 */

vptr infowin_datakey (vptr dat)
{
  infowin *iwin = (infowin*)(dat);
  return ((vptr)(&(iwin->win)));
}



/*
 * Hash a infowin's key (xid*) into mod base
 */

uint infowin_keyhash (vptr key, uint base)
{
  huge *xid = (huge*)(key);
  return ((uint)((*xid) % base));
}


/*
 * Compare two infowin's keys (xid*'s) ala strcmp
 */

int infowin_keycomp (vptr key1, vptr key2)
{
  huge *xid1 = (huge*)(key1);
  huge *xid2 = (huge*)(key2);

  /* Equality Test */
  if (*xid1 == *xid2) return (0);

  /* Must be less or more */
  return (((*xid1) < (*xid2)) ? -1 : 1);
}


