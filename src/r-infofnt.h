/* File: r-infofnt.h */

#ifndef _R_INFOFNT_H
#define _R_INFOFNT_H

#include "h-include.h"

/*****************************************************************
 *
 * This file provides optional support for x-infofnt.h
 *
 * Functions provided deal with Text drawing and stuff.
 *
 *****************************************************************/

#include "x-infofnt.h"



/*
 * Explanation of Text Flags:
 *
 * Horizontal placement:
 *   TEXT_J_LT:    Line up text with its leftmost edge at 'x'
 *   TEXT_J_RT:    Line up text with its right-most edge at 'x'
 *   Neither:      Center text, snap to GRID, exact FILL
 *   Both:         Center text, shift GRID, infinite FILL
 *
 * Vertical placement:
 *   TEXT_J_UP:    Line up text with its top at 'y'
 *   TEXT_J_DN:    Line up text with its bottom at 'y'
 *   Default:      Center text around 'y'
 *   Both:         Line up text with its baseline at 'y'
 *
 * Special:
 *   TEXT_QUERY:   Query for font info (vs Use Infofnt)
 *   TEXT_GRID:    Count rows and columns (vs Count Pixels)
 *   TEXT_FILL:    Paint a rectangle (vs Draw Actual Text)
 *   TEXT_WIPE:    Erase before drawing (vs Stipple Draw)
 */


/**** Available Constants ****/

/* Simple Flag Combinations for the Text Drawing routines */

#define TEXT_NONE       0x00

#define TEXT_QUERY      0x01

#define TEXT_GRID       0x02
#define TEXT_FILL       0x04
#define TEXT_WIPE       0x08

#define TEXT_J_LT       0x10
#define TEXT_J_RT       0x20
#define TEXT_J_UP       0x40
#define TEXT_J_DN       0x80



/**** Available Functions ****/


/* Draw (or Fill) Text (or Paint) on the Screen (using various flags) */
extern errr Infofnt_text (/*X,Y,S,L,M*/);


#endif

