/* File: borg-aux.h */

/* Purpose: Header file for "borg-aux.c" -BEN- */

#ifndef INCLUDED_BORG_AUX_H
#define INCLUDED_BORG_AUX_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg-aux.c".
 */

#include "borg.h"

#include "borg-map.h"

#include "borg-obj.h"



/*
 * Think about the dungeon
 */
extern bool borg_think_dungeon(void);

/*
 * Think about a store
 */
extern bool borg_think_store(void);


/*
 * Initialize this file
 */
extern void borg_aux_init(void);


#endif

#endif

