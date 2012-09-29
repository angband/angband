/* File: x-infoclr.c */

#include "x-infoclr.h"

#include "z-util.h"
#include "z-virt.h"



/*** The current infoclr ***/

infoclr *Infoclr = (infoclr*)(NULL);



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
    if (streq (name, "white")) return (Metadpy->white);

    /* The 'black' color is available */
    if (streq (name, "black")) return (Metadpy->black);

    /* Attempt to parse 'name' into 'scrn' */
    if (!(XParseColor (Metadpy->dpy, Metadpy->cmap, name, &scrn)))
    {
      fprintf (stderr, "Warning: Couldn't parse color '%s'\n", name);
    }

    /* Attempt to Allocate the Parsed color */
    if (!(XAllocColor (Metadpy->dpy, Metadpy->cmap, &scrn)))
    {
      fprintf (stderr, "Warning: Couldn't allocate color '%s'\n", name);
    }

    /* The Pixel was Allocated correctly */
    else return (scrn.pixel);
  }

  /* Warn about the Default being Used */
  fprintf (stderr, "Warning: Using 'fg' for unknown color '%s'\n", name);

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




