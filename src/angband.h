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


/*
 * Metaphor -- There are "two" layers, the "game" and the "play".
 *
 * The outer layer deals with stuff like creating windows, managing menus,
 * preparing to handle signals, and cleaning up, while the inner "play"
 * layer actually does things related to game playing (including loading
 * and saving character files, but NOT preference files).  This makes
 * it easier to replace the outer layer without changing the "play" layer.
 *
 * Thus, there is a "main()" function (i.e. below), which takes a different
 * form on every system.  It parses its arguments/ preference files,
 * and then invokes the game playing function, "play_game()". There
 * is an "escape" function called "exit_game()" which completes the
 * game, showing scores and such.  The "main()" function can also set up
 * a special "hook" for the "quit()" function to execute at "exit()"...
 *
 * Unfortunately, Angband does not "want" to be event driven, so events
 * are hacked by inverting them, and having requests for specific event
 * information cycle the event loop, and then extract any needed info.
 * The game will thus "hang" inside long loops.  Luckily, frequent calls
 * to the "get key" or "check key" routines are made, flushing the events.
 * See 'term.c' for how this is done.
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
 * The only machine that I *know* of that needs this is Macintosh.
 * Just add "|| defined(xxx)" as necessary...
 */
#if defined(MACINTOSH)
# include "random.h"
#endif


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



#endif



