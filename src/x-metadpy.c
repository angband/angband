/* File x-metadpy.c */

#include "x-metadpy.h"

#include "z-util.h"
#include "z-virt.h"
#include "z-form.h"


#ifdef SUPPORT_VIRTUAL_ROOT
#include "z-xroot.h"
#endif


/* The Global "default metadpy" variable */

static metadpy metadpy_default;


/* The Global "current metadpy" variable */

metadpy *Metadpy = &metadpy_default;





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

      /* Indicate that we could not open that display */
      plog_fmt("Unable to open the display '%s'", name);

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

#ifdef SUPPORT_VIRTUAL_ROOT
  m->root = VirtualRootWindowOfScreen (m->screen);
#endif

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



