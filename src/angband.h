/* File: angband.h */

/* Main "Angband" header file */

#ifndef INCLUDED_ANGBAND_H
#define INCLUDED_ANGBAND_H

/*
 * Copyright (c) 1989 James E. Wilson
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */



/*
 * First, include the low-level includes.  Be sure to edit "h-config.h"
 * to reflect any hardware, operating system, or compiler nuances.
 */
#include "h-include.h"

/*
 * Then, include the header files for the low-level code
 */
#include "z-util.h"
#include "z-virt.h"
#include "z-form.h"



/*
 * Include the header file for the "terminal" stuff
 */
#include "term.h"


/*
 * Include the header file for the random number generator
 * Note that ALL machines can safely include this file now,
 * it simply "redefines" the standard name space to prevent
 * conflicts.  If you include the header (random.h) you must
 * compile the source (random.c).
 */
#include "random.h"


/*
 * Include the "Angband" configuration header
 */
#include "config.h"


/*
 * Now, include the define's, the type's, and the extern's
 */
#include "defines.h"
#include "types.h"
#include "externs.h"



/***** Some copyright messages follow below *****/

/*
 * UNIX ANGBAND Version 5.0
 */


/* Original copyright message follows. */

/* ANGBAND Version 4.8	COPYRIGHT (c) Robert Alan Koeneke		*/
/*									 */
/*	 I lovingly dedicate this game to hackers and adventurers	 */
/*	 everywhere...							 */
/*									 */
/*									 */
/*	 Designer and Programmer : Robert Alan Koeneke			 */
/*				   University of Oklahoma		 */
/*									 */
/*	 Assistant Programmers	 : Jimmey Wayne Todd			 */
/*				   University of Oklahoma		 */
/*									 */
/*				   Gary D. McAdoo			 */
/*				   University of Oklahoma		 */
/*									 */
/*	 UNIX Port		 : James E. Wilson			 */
/*				   UC Berkeley				 */
/*				   wilson@ernie.Berkeley.EDU		 */
/*				   ucbvax!ucbernie!wilson		 */
/*									 */
/*	 MSDOS Port		 : Don Kneller				 */
/*				   1349 - 10th ave			 */
/*				   San Francisco, CA 94122		 */
/*				   kneller@cgl.ucsf.EDU			 */
/*				   ...ucbvax!ucsfcgl!kneller		 */
/*				   kneller@ucsf-cgl.BITNET		 */
/*									 */
/*	 BRUCE ANGBAND		 : Christopher Stuart			 */
/*				   Monash University			 */
/*				   Melbourne, Victoria, AUSTRALIA	 */
/*				   cjs@moncsbruce.oz			 */
/*									 */
/*	 ANGBAND may be copied and modified freely as long as the above	 */
/*	 credits are retained.	No one who-so-ever may sell or market	 */
/*	 this software in any form without the expressed written consent */
/*	 of the author Robert Alan Koeneke.				 */
/*									 */


#endif



