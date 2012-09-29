/* File: r-infowin.h */

#ifndef _R_INFOWIN_H
#define _R_INFOWIN_H

#include "h-include.h"

/*****************************************************************
 *
 * Optional support routines for the infowin structure
 *
 *****************************************************************/

#include "x-infowin.h"




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



#endif

