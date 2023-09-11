/* File: borg5.h */
/* Purpose: Header file for "borg5.c" -BEN- */

#ifndef INCLUDED_BORG5_H
#define INCLUDED_BORG5_H

#include "angband.h"
#include "obj-tval.h"
#include "cave.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg5.c".
 */

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"

 /*
  * Possible values of "goal"
  */
#define GOAL_KILL   1       /* Monsters */
#define GOAL_TAKE   2       /* Objects */
#define GOAL_MISC   3       /* Stores */
#define GOAL_DARK   4       /* Exploring */
#define GOAL_XTRA   5       /* Searching */
#define GOAL_BORE   6       /* Leaving */
#define GOAL_FLEE   7       /* Fleeing */
#define GOAL_VAULT  8		/* Vaults */
#define GOAL_RECOVER 9		/* Resting safely */
#define GOAL_DIGGING 10		/* Anti-summon Corridor */


  /*
   * Update state based on current "map"
   */
extern void borg_update(void);


/*
 * React to various "important" messages
 */
extern void borg_react(const char* msg, const char* buf);
extern void borg_delete_kill(int i);
extern void borg_delete_take(int i);
extern void borg_clear_reactions(void);



/*
 * Initialize this file
 */
extern void borg_init_5(void);
extern void borg_clean_5(void);

/* forward declare from borg6.c */
extern bool borg_target_unknown_wall(int g_y, int g_x);

extern int borg_panel_hgt(void);
extern int borg_panel_wid(void);

#endif

#endif

