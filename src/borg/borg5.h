/* File: borg5.h */
/* Purpose: Header file for "borg5.c" -BEN- */

#ifndef INCLUDED_BORG5_H
#define INCLUDED_BORG5_H

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg5.c".
 */

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg6.h"


/*
 * Update state based on current "map"
 */
extern void borg_update(void);


/*
 * React to various "important" messages
 */
extern void borg_react(cptr msg, cptr buf);
extern void borg_delete_kill(int i);
extern void borg_delete_take(int i);


/*
 * Initialize this file
 */
extern void borg_init_5(void);




#endif

#endif

