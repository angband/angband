/* File: xtra-x11.c */

/* Purpose: somewhat generic support for main-x11.c */

#include "angband.h"

#ifdef USE_X11

#include "z-util.h"
#include "z-virt.h"
#include "z-form.h"

#include "xtra-x11.h"


/*
 * The "default" values
 */
static metadpy metadpy_default;


/*
 * The "current" variables
 */
metadpy *Metadpy = &metadpy_default;
infowin *Infowin = (infowin*)(NULL);
infoclr *Infoclr = (infoclr*)(NULL);
infofnt *Infofnt = (infofnt*)(NULL);





/* OPEN: x-metadpy.c */


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

errr Metadpy_init_2 (Display *dpy, char *name)
{
  metadpy *m = Metadpy;


  /*** Open the display if needed ***/

  /* If no Display given, attempt to Create one */
  if (!dpy)
  {
    /* Attempt to open the display */
    dpy = XOpenDisplay (name);

    /* Failure */
    if (!dpy)
    {
      /* No name given, extract DISPLAY */
      if (!name) name = getenv ("DISPLAY");

      /* No DISPLAY extracted, use default */
      if (!name) name = "(default)";

#if 0
      /* Indicate that we could not open that display */
      plog_fmt("Unable to open the display '%s'", name);
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
  m->screen = DefaultScreenOfDisplay (dpy);
  m->root = RootWindowOfScreen (m->screen);

  /* Get the default colormap */
  m->cmap = DefaultColormapOfScreen (m->screen);

  /* Extract the true name of the display */
  m->name = DisplayString (dpy);

  /* Extract the fd */
  m->fd = ConnectionNumber (Metadpy->dpy);

  /* Save the Size and Depth of the screen */
  m->width = WidthOfScreen (m->screen);
  m->height = HeightOfScreen (m->screen);
  m->depth = DefaultDepthOfScreen (m->screen);

  /* Save the Standard Colors */
  m->black = BlackPixelOfScreen (m->screen);
  m->white = WhitePixelOfScreen (m->screen);


  /*** Make some clever Guesses ***/

  /* Guess at the desired 'fg' and 'bg' Pixell's */
  m->bg = m->black;
  m->fg = m->white;

  /* Calculate the Maximum allowed Pixel value.  */
  m->zg = (1 << m->depth) - 1;

  /* Save various default Flag Settings */
  m->color = ((m->depth > 1) ? 1 : 0);
  m->mono = ((m->color) ? 0 : 1);


  /*** All done ***/

  /* Return "success" ***/
  return (0);
}



/*
 * Nuke the current metadpy
 */

errr Metadpy_nuke ()
{
  metadpy *m = Metadpy;


  /* If required, Free the Display */
  if (m->nuke)
  {
    /* Close the Display */
    XCloseDisplay (m->dpy);

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

errr Metadpy_update (int flush, int sync, int discard)
{
  /* Flush if desired */
  if (flush) XFlush (Metadpy->dpy);

  /* Sync if desired, using 'discard' */
  if (sync) XSync (Metadpy->dpy, discard);

  /* Success */
  return (0);
}




/*
 * Set the pitch and duration for succeeding Beeps
 *
 * The volume is a base percent (0 to 100)
 * The pitch is given in Hertz (0 to 10000)
 * The duration is in millisecs (0 to 1000)
 */

errr Metadpy_prepare_sound (int vol, int pit, int dur)
{
  XKeyboardControl data;
  unsigned long mask;

  /* Set the important fields */
  data.bell_percent =  vol;
  data.bell_pitch =    pit;
  data.bell_duration = dur;

  /* Set the mask fields */
  mask = KBBellPercent | KBBellPitch | KBBellDuration;

  /* Apply the change */
  XChangeKeyboardControl (Metadpy->dpy, mask, &data);

  /* Success */
  return (0);
}




/*
 * Make a sound prepared above, vol is "percent of base"
 */

errr Metadpy_produce_sound (int vol)
{
  /* Make a sound */
  XBell (Metadpy->dpy, vol);

  return (0);
}



/*
 * Make a simple beep
 */

errr Metadpy_do_beep ()
{
  /* Make a simple beep */
  XBell (Metadpy->dpy, 100);

  return (0);
}



/* SHUT: x-metadpy.c */


/* OPEN: x-metadpy.c */


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


/* SHUT: x-infowin.c */


/* OPEN: x-infoclr.c */


/*
 * A NULL terminated pair list of legal "operation names"
 *
 * Pairs of values, first is texttual name, second is the string
 * holding the decimal value that the operation corresponds to.
 */

static cptr opcode_pairs[] = {

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

  "dst & src", "1",
  "src & dst", "1",

  "dst | src", "7",
  "src | dst", "7",

  "dst ^ src", "6",
  "src ^ dst", "6",

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

int Infoclr_Opcode (cptr str)
{
  register int i;

  /* Scan through all legal operation names */
  for (i = 0; opcode_pairs[i*2]; ++i)
  {
    /* Is this the right oprname? */
    if (streq (opcode_pairs[i*2], str))
    {
      /* Convert the second element in the pair into a Code */
      return (atoi (opcode_pairs[i*2+1]));
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

Pixell Infoclr_Pixell(cptr name)
{
  XColor scrn;


  /* Attempt to Parse the name */
  if (name && name[0])
  {
    /* The 'bg' color is available */
    if (streq (name, "bg")) return (Metadpy->bg);

    /* The 'fg' color is available */
    if (streq (name, "fg")) return (Metadpy->fg);

    /* The 'zg' color is available */
    if (streq (name, "zg")) return (Metadpy->zg);

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

errr Infoclr_init_1 (GC gc)
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

errr Infoclr_nuke ()
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

errr Infoclr_init_data (Pixell fg, Pixell bg, int op, int stip)
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
  gc_mask = GCFunction | GCBackground | GCForeground |
	    GCFillStyle | GCGraphicsExposures;

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


/* SHUT: x-infoclr.c */


/* OPEN: x-infofnt.c */


/*
 * Nuke an old 'infofnt'.
 */

errr Infofnt_nuke ()
{
  infofnt *ifnt = Infofnt;

  /* Deal with 'name' */
  if (ifnt->name)
  {
    /* Free the name */
    string_free (ifnt->name);
  }

  /* Nuke info if needed */
  if (ifnt->nuke)
  {
    /* Free the font */
    XFreeFont (Metadpy->dpy, ifnt->info);
  }

  /* Success */
  return (0);
}



/*
 * Prepare a new 'infofnt'
 */

static errr Infofnt_prepare (XFontStruct *info)
{
  infofnt *ifnt = Infofnt;

  XCharStruct *cs;

  /* Assign the struct */
  ifnt->info = info;

  /* Jump into the max bouonds thing */
  cs = &(info->max_bounds);

  /* Extract default sizing info */
  ifnt->asc = cs->ascent;
  ifnt->hgt = (cs->ascent + cs->descent);
  ifnt->wid = cs->width;

  /* Success */
  return (0);
}





/*
 * Initialize a new 'infofnt'.
 */

errr Infofnt_init_real (XFontStruct *info)
{
  /* Wipe the thing */
  WIPE(Infofnt, infofnt);

  /* No nuking */
  Infofnt->nuke = 0;

  /* Attempt to prepare it */
  return (Infofnt_prepare (info));
}




/*
 * Init an infofnt by its Name
 *
 * Inputs:
 *	name: The name of the requested Font
 */

errr Infofnt_init_data (char *name)
{
  XFontStruct *info;


  /*** Load the info Fresh, using the name ***/

  /* If the name is not given, report an error */
  if (!name) return (-1);

  /* Attempt to load the font */
  info = XLoadQueryFont (Metadpy->dpy, name);

  /* The load failed, try to recover */
  if (!info) return (-1);


  /*** Init the font ***/

  /* Wipe the thing */
  WIPE(Infofnt, infofnt);

  /* Attempt to prepare it */
  if (Infofnt_prepare (info))
  {
    /* Free the font */
    XFreeFont (Metadpy->dpy, info);

    /* Fail */
    return (-1);
  }

  /* Save a copy of the font name */
  Infofnt->name = string_make (name);

  /* Mark it as nukable */
  Infofnt->nuke = 1;

  /* Success */
  return (0);
}


/* SHUT: x-infofnt.c */







/* OPEN: r-metadpy.c */


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

/* SHUT: r-metadpy.c */

/* OPEN: r-infowin.c */

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


/* SHUT: r-infowin.c */


/* OPEN: r-infoclr.c */


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


/* SHUT: r-infoclr.c */


/* OPEN: r-infofnt.c */



/*
 * Standard Text
 */

errr Infofnt_text_std (int x, int y, cptr str, int len, uint mode)
{
  int i;
  int dir, asc, desc;
  XCharStruct ov;


  /*** Do a brief info analysis ***/

  /* Do nothing if the string is null */
  if (!str || !*str) return (-1);

  /* Get the length of the string */
  if (len < 0) len = strlen (str);


  /*** Decide where to place the string ***/

  /* Both TEXT_GRID and TEXT_QUERY makes no sense */
  if ((mode & TEXT_QUERY) && (mode & TEXT_GRID))
  {
    /* Not very graceful */
    core("Infofnt: Impossible Request: QUERY+ GRID\n");
  }


  /* Assume TEXT_QUERY over-rides TEXT_GRID */
  else if (mode & TEXT_QUERY)
  {
    /*** Query the Info - Why? ***/

    /* Get the "size" of the string */
    XTextExtents (Infofnt->info, str, len, &dir, &asc, &desc, &ov);



    /*** Decide where to place the string, vertically ***/

    /* Use y as the baseline location */
    if ((mode & TEXT_J_UP) && (mode & TEXT_J_DN))
    {
      /* Use the standard Baseline */
    }

    /* Use y as the top location */
    else if (mode & TEXT_J_UP)
    {
      y = y + ov.ascent;
    }

    /* Use y as the bottom location */
    else if (mode & TEXT_J_DN)
    {
      y = y - ov.descent;
    }

    /* Center vertically around y */
    else
    {
      y = y + (ov.ascent - ov.descent) / 2;
    }


    /*** Decide where to place the string, horizontally ***/

    /* Same as Centering below */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = x - (ov.width / 2);
    }

    /* Line up with x at left edge (XXX cleanly?) */
    else if (mode & TEXT_J_LT)
    {
      /* x = x + ov.lbearing; */
    }

    /* Line up with x at right edge (XXX cleanly?) */
    else if (mode & TEXT_J_RT)
    {
      x = x - (ov.width);
    }

    /* Default -- Center horizontally (XXX cleanly?) */
    else
    {
      x = x - (ov.width / 2);
    }
  }


  /* Use (row,col) positional info */
  else if (mode & TEXT_GRID)
  {
    /*** Decide where to place the string, vertically ***/

    /* Ignore Vertical Justifications */
    y = (y * Infofnt->hgt) + Infofnt->asc;


    /*** Decide where to place the string, horizontally ***/

    /* Center, allow half-grid column leeway */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = (x * Infofnt->wid) - (len * Infofnt->wid / 2);
    }

    /* Line up with x at left edge of column 'x' */
    else if (mode & TEXT_J_LT)
    {
      x = (x * Infofnt->wid);
    }

    /* Line up with x at right edge of column 'x' */
    else if (mode & TEXT_J_RT)
    {
      x = (x - (len - 1)) * Infofnt->wid;
    }

    /* Center horizontally, snap to grid columns */
    else
    {
      x = (x - (len / 2)) * Infofnt->wid;
    }
  }


  /* Neither TEXT_GRID nor TEXT_QUERY, this is very common */
  else
  {
    /*** Decide where to place the string, vertically ***/

    /* Use y as the baseline location */
    if ((mode & TEXT_J_UP) && (mode & TEXT_J_DN))
    {
      /* Use standard base line */
    }

    /* Use y as the top location */
    else if (mode & TEXT_J_UP)
    {
      y = y + Infofnt->asc;
    }

    /* Use y as the bottom location */
    else if (mode & TEXT_J_DN)
    {
      y = y + Infofnt->asc - Infofnt->hgt;
    }

    /* Center vertically around y */
    else
    {
      y = y + Infofnt->asc - (Infofnt->hgt / 2);
    }


    /*** Decide where to place the string, horizontally ***/

    /* Same as standard centering */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = x - ((len * Infofnt->wid) / 2);
    }

    /* Line up with x at left edge (XXX cleanly?) */
    else if (mode & TEXT_J_LT)
    {
      /* Just use (messy) left edge */
    }

    /* Line up with x at right edge (XXX cleanly?) */
    else if (mode & TEXT_J_RT)
    {
      x = x - (len * Infofnt->wid);
    }

    /* Center horizontally (XXX cleanly?) */
    else
    {
      x = x - ((len * Infofnt->wid) / 2);
    }
  }


  /*** Actually draw 'str' onto the infowin ***/

  /* Be sure the correct font is ready */
  XSetFont (Metadpy->dpy, Infoclr->gc, Infofnt->info->fid);


  /*** Handle the fake mono we can enforce on fonts ***/

  /* Monotize the font */
  if (Infofnt->mono)
  {
    /* Do each character */
    for (i = 0; i < len; ++i)
    {
      /* Perhaps draw it and clear behind it */
      if (mode & TEXT_WIPE)
      {
	/* Note that the Infoclr is set up to contain the Infofnt */
	XDrawImageString (Metadpy->dpy, Infowin->win, Infoclr->gc,
			  x + i * Infofnt->wid + Infofnt->off, y, str + i, 1);
      }

      /* Else draw it without clearing behind it */
      else
      {
	/* Note that the Infoclr is set up to contain the Infofnt */
	XDrawString (Metadpy->dpy, Infowin->win, Infoclr->gc,
		     x + i * Infofnt->wid + Infofnt->off, y, str + i, 1);
      }
    }
  }

  /* Assume monoospaced font */
  else
  {
    /* Perhaps draw it and clear behind it */
    if (mode & TEXT_WIPE)
    {
      /* Note that the Infoclr is set up to contain the Infofnt */
      XDrawImageString (Metadpy->dpy, Infowin->win, Infoclr->gc,
			x, y, str, len);
    }

    /* Else draw it without clearing behind it */
    else
    {
      /* Note that the Infoclr is set up to contain the Infofnt */
      XDrawString (Metadpy->dpy, Infowin->win, Infoclr->gc,
		   x, y, str, len);
    }
  }


  /* Success */
  return (0);
}






/*
 * Painting where text would be (assume TEXT_GRID)
 */

errr Infofnt_text_non (int x, int y, cptr str, int len, uint mode)
{
  int w, h;


  /*** Do a brief info analysis ***/

  /* If 'str' is NULL, fill all the way to the Infowin edge */
  if (!str)
  {
    /*** Find the X and W dimensions ***/

    /* Fill entire row */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      w = Infowin->w;
      x = 0;
    }

    /* Line up with x at left edge of column 'x', fill to right edge */
    else if (mode & TEXT_J_LT)
    {
      x = x * Infofnt->wid;
      w = Infowin->w - x;
    }

    /* Line up with x at right edge of column 'x', fill to left edge */
    else if (mode & TEXT_J_RT)
    {
      w = x * Infofnt->wid;
      x = 0;
    }

    /* Fill the entire Row */
    else
    {
      w = Infowin->w;
      x = 0;
    }
  }


  /* Otherwise, perform like (TEXT_GRID + not TEXT_FILL) above */
  else
  {
    /*** Find the width ***/

    /* Negative length is a flag to count the characters in str */
    if (len < 0) len = strlen (str);

    /* The total width will be 'len' chars * standard width */
    w = len * Infofnt->wid;


    /*** Find the X dimensions ***/

    /* Center horizontally, allow half-frid column leeway */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = (x * Infofnt->wid) - (w / 2);
    }

    /* Line up with x at left edge of column 'x' */
    else if (mode & TEXT_J_LT)
    {
      x = x * Infofnt->wid;
    }

    /* Line up with x at right edge of column 'x' */
    else if (mode & TEXT_J_RT)
    {
      x = (x - (len - 1)) * Infofnt->wid;
    }

    /* Center horizontally, snap to grid. */
    else
    {
      x = (x - (len / 2)) * Infofnt->wid;
    }
  }


  /*** Find other dimensions ***/

  /* Simply do 'Infofnt->hgt' (a single row) high */
  h = Infofnt->hgt;

  /* Simply do "at top" in row 'y' */
  y = y * h;


  /*** Actually 'paint' the area ***/

  /* Just do a Fill Rectangle */
  XFillRectangle (Metadpy->dpy, Infowin->win, Infoclr->gc, x, y, w, h);

  /* Success */
  return (0);
}



/*
 * Draw something resembling text somewhere with various options
 *
 * Globals:
 *   Metadpy: The base metadpy to draw on
 *   Infowin: The base infowin to draw on
 *   Infofnt: The base infofnt to draw with
 *   Infoclr: The base infoclr to draw with
 *
 * Inputs:
 *   x, y: The (x,y) location (in some representation)
 *   str:  The string to use, if any (else NULL)
 *   len:  The length of str, if known (else -1)
 *   mode: The mode (see header file) to employ
 *
 * Position:
 *   The (x,y) can be a pixel location, or a (row,col) position
 *   The text can be centered, or anchored to the left or right
 *   The text can be vertically centered, or anchored weirdly
 *   Vertical anchors include top, bottom, and baseline
 *   The default is to be centered horizontally and vertically
 *   The size of the letters can be accepted, or queried
 *   Text can be drawn, wiped, or painted
 *
 * Note:
 *   When in the 'fill_text' routine, if 'str' is NULL, then
 *   assume that the "string" extends forever towards various
 *   edges (both if neither RIGHT nor LEFT justified)
 */

errr Infofnt_text (int x, int y, cptr str, int len, uint mode)
{
  int i;

  /* Branch on mode */
  i = ((mode & TEXT_FILL) ?
       (Infofnt_text_non (x, y, str, len, mode)) :
       (Infofnt_text_std (x, y, str, len, mode)));

  /* Return result */
  return (i);
}


/* SHUT: r-infofnt.c */


#endif

