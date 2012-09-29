/* File: x-infowin.h */

#ifndef _X_INFOWIN_H
#define _X_INFOWIN_H

#include "h-include.h"

/*****************************************************************
 *
 * This file provides support for the infowin structure.
 *
 *****************************************************************/

#include "x-metadpy.h"



/*** Forward Declare ***/

typedef struct _infowin infowin;



/*
 * A Structure (20 bytes) to Hold Window Information.
 * I assume that a window is at most 30000 pixels on a side.
 * I assume that the root windw is also at most 30000 square.
 *
 *	- The Window
 *	- The current Input Event Mask
 *
 *	- The location of the window
 *	- The width, height of the window
 *	- The border width of this window
 *
 *	- Byte: 1st Extra byte
 *
 *	- Bit Flag: This window is currently Mapped
 *	- Bit Flag: This window needs to be redrawn
 *	- Bit Flag: This window has been resized
 *
 *	- Bit Flag: We should nuke 'win' when done with it
 *
 *	- Bit Flag: 1st extra flag
 *	- Bit Flag: 2nd extra flag
 *	- Bit Flag: 3rd extra flag
 *	- Bit Flag: 4th extra flag
 */

struct _infowin {

  Window		win;
  long			mask;

  s16b			x, y;
  s16b			w, h;
  u16b			b;

  byte			byte1;

  uint			mapped:1;
  uint			redraw:1;
  uint			resize:1;

  uint			nuke:1;

  uint			flag1:1;
  uint			flag2:1;
  uint			flag3:1;
  uint			flag4:1;
};




/**** Available Variables ****/

extern infowin *Infowin;


/**** Available Functions ****/

extern errr Infowin_nuke ();
extern errr Infowin_init_real ();
extern errr Infowin_init_data ();

extern errr Infowin_set_name ();
extern errr Infowin_set_icon_name ();


/**** Passable Functions for ADT's and such ***/

extern vptr infowin_datakey(/*W*/);
extern uint infowin_keyhash(/*K,S*/);
extern int infowin_keycomp(/*K1,K2*/);


/**** Available Macros ****/

/* Init an infowin by giving father as an (info_win*) (or NULL), and data */
#define Infowin_init_dad(D,X,Y,W,H,B,FG,BG) \
	Infowin_init_data(((D) ? ((D)->win) : (Window)(None)), \
	X,Y,W,H,B,FG,BG)


/* Init a top level infowin by pos,size,bord,Colors */
#define Infowin_init_top(X,Y,W,H,B,FG,BG) \
	Infowin_init_data(None,X,Y,W,H,B,FG,BG)


/* Request a new standard window by giving Dad infowin and X,Y,W,H */
#define Infowin_init_std(D,X,Y,W,H,B) \
	Infowin_init_dad(D,X,Y,W,H,B,Metadpy->fg,Metadpy->bg)


/* Set the current Infowin */
#define Infowin_set(I) \
	(Infowin = (I))

/* Set the current Infowin */
#define Infowin_get(IP) \
	((*(IP)) = Infowin)




#endif


