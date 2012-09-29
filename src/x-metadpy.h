/* File: x-metadpy.h */

#ifndef INCLUDED_X_METADPY_H
#define INCLUDED_X_METADPY_H

#include "h-include.h"

/*
 * This file supports x-metadpy.c, and thus provides support for the
 * data type 'metadpy' which allows some basic Display information
 * to be manipulated.
 *
 * It provides a "current metadpy", Metadpy, which is used in
 * conjunction with the other x-files (Clrs, Wins, etc)
 */



/* Include the X definitions */

#ifndef __MAKEDEPEND__
# include <X11/Xlib.h>
#endif /* __MAKEDEPEND__ */


/* Fake define some undefined X structures for lint */

#ifdef lint
struct _XRegion {int x;};
struct _XrmHashBucketRec {int x;};
struct XKeytrans {int x;};
#endif /* ifdef lint */



/* A local version of 'Pixel' for compatibility */

typedef unsigned long Pixell;


/* A constant representing the Xdeg's in a full circle */

#define OVAL_FULL (360*64)




/*** Forward ***/

typedef struct _metadpy metadpy;


/*
 * A structure summarizing a given Display.  Most members public.
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



/**** Available Variables ****/

/* The "current" metadpy (ACCESS ONLY) */
extern metadpy *Metadpy;




/**** Available Functions ****/

/* Initialize a new metadpy */
extern errr Metadpy_init_2();

/* Nuke an existing metadpy */
extern errr Metadpy_nuke();



/**** Available Macros ****/


/* Set current metadpy (Metadpy) to 'M' */
#define Metadpy_set(M) \
	Metadpy = M

/* Get the current metadpy (Metadpy) */
#define Metadpy_get(MP) \
	(*(MP)) = Metadpy


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



#endif

