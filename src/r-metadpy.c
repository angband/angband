/* File r-metadpy.c */

#include "r-metadpy.h"

#ifndef __MAKEDEPEND__
#include <X11/Xutil.h>
#endif /* __MAKEDEPEND__ */

#include "z-util.h"
#include "z-form.h"



/*
 * Attempt to create a Pixmap from data on Metadpy
 *
 * Note that the "optimal" sized pixmap is created.
 * So be sure that 'drawing' uses the original size.
 *
 * Inputs:
 *   bits: The bits (NULL for empty Pixmap)
 *   w, h: The minimum size of the bitmap
 */

Pixmap Pixmap_from_data (byte *bits, int w, int h)
{
  Pixmap temp, map1;
  int w1, h1;

  GC tempgc;


  /* Illegal size induces "None" Pixmap */
  if ((w <= 0) || (h <= 0)) return (None);


  /* If 'bits' are given */
  if (bits)
  {
    /* First create the bitmap as a template */
    temp = XCreateBitmapFromData (Metadpy->dpy, Metadpy->root, bits, w, h);

    /* Make sure the template took */
    if (temp == None)
    {
      plog("Pixmap_from_data(): Unable to create Bitmap");
      return (None);
    }
  }

  /* Make sure the server will allow a big enough pixmap */
  if (!XQueryBestSize (Metadpy->dpy, TileShape, Metadpy->root,
		       w, h, &w1, &h1))
  {
    plog("Pixmap_from_data(): Unable to find size");
    return (None);
  }

  /* Oh well, the server is annoying */
  if (w1 < w || h1 < h)
  {
    plog("Pixmap_from_data(): Xserver small sized us: Ignoring.");
    w1 = w; h1 = h;
  }


  /* Create the actual Pixmap (Depth 1) */
  map1 = XCreatePixmap (Metadpy->dpy, Metadpy->root, w1, h1, 1);

  /* Oh well, silly server */
  if (!map1)
  {
    plog("Pixmap_from_data(): Unable to create True pixmap");
    return (None);
  }

  /* Allocate a temporary GC */
  tempgc = XCreateGC (Metadpy->dpy, map1, 0L, NULL);
  XSetForeground (Metadpy->dpy, tempgc, 0L);

  /* Clean out the new pixmap */
  XFillRectangle (Metadpy->dpy, map1, tempgc, 0, 0, w1, h1);


  /* Use the bits if any were given */
  if (bits)
  {
    /* Save the created pixmap data */
    XCopyArea (Metadpy->dpy, temp, map1, tempgc, 0, 0, w, h, 0, 0);

    /* Release the temporary pixmap */
    XFreePixmap (Metadpy->dpy, temp);
  }

  /* Return the New Pixmap */
  return (map1);
}




/*
 * Attempt to makes a Cursor from data on Metadpy
 */

Cursor Cursor_from_data (byte *bits, byte *mask, int w, int h,
			 int x, int y, Pixell fg, Pixell bg)
{
  XColor fcol, bcol;
  Pixmap bpix, mpix;
  Cursor cursor = None;

  /* Try to Make the pixmap for the cursor */
  bpix = Pixmap_from_data (bits, w, h);

  /* Be sure it worked */
  if (!bpix)
  {
    plog("Failed to make Cursor Bitmap");
    return (cursor);
  }

  /* Make the Mask for the cursor */
  mpix = Pixmap_from_data (mask, w, h);

  /* Try to make the Mask */
  if (!mpix)
  {
    plog("Failed to make Cursor Mask");
    XFreePixmap (Metadpy->dpy, bpix);
    return (cursor);
  }

  /* Build the Foreground XColor */
  fcol.pixel = fg;
  XQueryColor (Metadpy->dpy, Metadpy->cmap, &fcol);

  /* Build the Background XColor */
  bcol.pixel = bg;
  XQueryColor (Metadpy->dpy, Metadpy->cmap, &bcol);

  /* Create the Cursor */
  cursor = XCreatePixmapCursor (Metadpy->dpy,
	       bpix, mpix, &fcol, &bcol, x, y);

  /* Free up space */
  XFreePixmap (Metadpy->dpy, bpix);
  XFreePixmap (Metadpy->dpy, mpix);

  /* Return new Cursor (or the Failure) */
  return (cursor);
}




/*
 * My X Error Handler's Constants
 *
 *	- The Error Message to look for in the Database
 *	- The Function to call (with the Display) for Cleanup
 */

static func_errr Metadpy_Error_Cleanup = NULL;


/*
 * My X Error Handler: Similar to Signal Handlers
 *
 * Inputs:
 *	dpy: The Display* that the error occured on
 *	err: The actual code of the Error
 */

static int Metadpy_Error_Handler (Display *dpy, XErrorEvent *err)
{
  char buf[1024];

  /* The default description of the error */
  cptr silly = "?!? Clueless Database ?!?";

  /* XXX Describe the Source (Is the Arg right?) */
  plog_fmt("XError on Display '%s'", XDisplayName(buf));

  /* Get the error */
  XGetErrorText (dpy, err->error_code, buf, 1024);

  /* Describe the Error */
  plog_fmt("XError Text: '%s'", buf);

  /* Describe the Database Answers */
  if (argv0)
  {
    /* Get the Text from the Database */
    XGetErrorDatabaseText (dpy, argv0, "XRequest", silly, buf, 1024);

    /* Display the info */
    plog_fmt("Serial %d/%d, Opcode %d/%d (%s), id $%lx",
	     err->serial, 0,
	     err->request_code, err->minor_code, buf,
	     err->resourceid);
  }

  /* Clean Up, if requested */
  if (Metadpy_Error_Cleanup) (*Metadpy_Error_Cleanup)(dpy);

  /* Dump Core */
  core("Crashing!\n");

  /* Keep 'lint' happy */
  return (0);
}



/*
 * Hack -- Prepare to catch errors on ANY Metadpy
 *
 * Inputs:
 *   func: A Cleanup Function on Metadpy
 */

errr Metadpy_Error_Catch (func_errr func)
{
  /* Save the Variables */
  Metadpy_Error_Cleanup = func;

  /* Activate the Error Trapper */
  XSetErrorHandler (Metadpy_Error_Handler);

  return (0);
}




