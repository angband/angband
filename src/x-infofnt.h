/* File: x-infofnt.h */

#ifndef _X_INFOFNT_H
#define _X_INFOFNT_H

#include "h-include.h"


/*****************************************************************
 *
 * This file provides generic x-metadpy.h support for Fonts
 *
 * Functions provided deal with Text, and its display.
 *
 *****************************************************************/


#include "x-metadpy.h"


/*** Forward Declare ***/

typedef struct _infofnt infofnt;


/*****************************************************************
 * A Structure (? bytes) to Hold Font Information
 *
 *	- The 'XFontStruct*' (yields the 'Font')
 *
 *	- The font name
 *
 *	- The default character width
 *	- The default character height
 *	- The default character ascent
 *
 *	- Byte: Pixel offset used during fake mono
 *
 *	- Flag: Force monospacing via 'wid'
 *	- Flag: Nuke info when done
 */

struct _infofnt {

  XFontStruct	*info;

  cptr			name;

  s16b			wid;
  s16b			hgt;
  s16b			asc;

  byte			off;

  uint			mono:1;
  uint			nuke:1;
};







/**** Available Constants ****/

#define fnt_small       "fixed"
#define fnt_medium      "-misc-fixed-medium-r-*-15-*"
#define fnt_large       "*-courier-bold-r-*-24-*"



/**** Available Variables ****/

extern infofnt *Infofnt;


/**** Available Functions ****/

extern errr Infofnt_nuke();
extern errr Infofnt_init_real();
extern errr Infofnt_init_data();



/**** Available Macros ****/

/* Set the current infofnt */
#define Infofnt_set(I) \
	(Infofnt = (I))

/* Get the current infofnt */
#define Infofnt_get(IP) \
	((*(IP)) = Infofnt)



#endif

