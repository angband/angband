/* File: x-infoclr.h */

#ifndef _X_INFOCLR_H
#define _X_INFOCLR_H

#include "h-include.h"

/*****************************************************************
 *
 * This file provides support for the infoclr portion of
 * the 'metadpy' structure.
 *
 *****************************************************************/

#include "x-metadpy.h"



/*****************************************************************
 * Notes on Colors:
 *
 *   1) On a monochrome (or "fake-monochrome") display, all colors
 *   will be "cast" to "fg," except for the bg color, which is,
 *   obviously, cast to "bg".  Thus, one can ignore this setting.
 *
 *   2) Because of the inner functioning of the color allocation
 *   routines, colors may be specified as (a) a typical color name,
 *   (b) a hexidecimal color specification (preceded by a pound sign),
 *   or (c) by any of the strings "fg", "bg", etc.
 *
 *   3) Due to the workings of the init routines, many colors
 *   may also be dealt with by their actual pixel values.  Note that
 *   the pixel with all bits set is "zg" = (1<<metadpy->depth)-1, which
 *   is not necessarily either black or white.
 */




/*****************************************************************
 * A Structure (16 bytes) to hold Operation+Color Information
 *
 *	- The actual GC corresponding to this info
 *
 *	- The Foreground Pixell Value
 *	- The Background Pixell Value
 *
 *	- Num (0-15): The operation code (As in Clear, Xor, etc)
 *	- Bit Flag: The GC is in stipple mode
 *	- Bit Flag: Destroy 'gc' at Nuke time.
 */

typedef struct _infoclr infoclr;

struct _infoclr {

  GC			gc;

  Pixell		fg;
  Pixell		bg;

  uint			code:4;
  uint			stip:1;
  uint			nuke:1;
};




/**** Available Variables ****/

extern infoclr *Infoclr;



/**** Available Functions ****/

/* Convert an Opcode name to an Opcode (i.e. "cpy") */
extern int Infoclr_Opcode(/*N*/);

/* Convert a Pixell Name to a Pixell (i.e. "red") */
extern Pixell Infoclr_Pixell(/*N*/);


/* Nuke Infoclr */
extern errr Infoclr_nuke();

/* Initialize Infoclr from a 'gc' */
extern errr Infoclr_init_1(/*G*/);

/* Initialize Infoclr from some data (Op, Fg, Bg, Stip) */
extern errr Infoclr_init_data(/*F,B,O,S*/);



/**** Available Macros  ****/

/* Set the current Infoclr */
#define Infoclr_set(C) \
	(Infoclr = (C))

/* Set the current Infoclr */
#define Infoclr_get(CP) \
	((*(CP)) = Infoclr)



/**** Available Macros (Requests) ****/

#define Infoclr_init_ppo(F,B,O,M) \
	Infoclr_init_data(F,B,O,M)

#define Infoclr_init_cco(F,B,O,M) \
	Infoclr_init_ppo(Infoclr_Pixell(F),Infoclr_Pixell(B),O,M)

#define Infoclr_init_ppn(F,B,O,M) \
	Infoclr_init_ppo(F,B,Infoclr_Opcode(O),M)

#define Infoclr_init_ccn(F,B,O,M) \
	Infoclr_init_cco(F,B,Infoclr_Opcode(O),M)




#endif

