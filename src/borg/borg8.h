/* File: borg8.h */

/* Purpose: Header file for "borg8.c" -BEN- */

#ifndef INCLUDED_BORG8_H
#define INCLUDED_BORG8_H

#include "../angband.h"
#include "../cave.h"
#include "../obj-tval.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg8.c".
 */

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg6.h"
#include "cmds.h"

/*
 * Think about the stores
 */
extern bool borg_think_store(void);

/*
 * Think about the dungeon
 */
extern bool borg_think_dungeon(void);

/*
 * Initialize this file
 */
extern void borg_init_8(void);
extern void borg_clean_8(void);

#endif

#endif
