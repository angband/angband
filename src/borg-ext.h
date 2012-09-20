/* File: borg-ext.h */

/* Purpose: Header file for "borg-ext.c" -BEN- */

#ifndef INCLUDED_BORG_EXT_H
#define INCLUDED_BORG_EXT_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg-ext.c".
 */

#include "borg.h"

#include "borg-map.h"

#include "borg-obj.h"



/*
 * Possible values of "goal"
 */
#define GOAL_KILL	11
#define GOAL_TAKE	12
#define GOAL_DARK	13
#define GOAL_XTRA	99



/*
 * Possible values of the "auto_char" array.
 */

#define AUTO_CHAR_ICKY		0	/* Char is icky */
#define AUTO_CHAR_OKAY		1	/* Char is boring */
#define AUTO_CHAR_XTRA		5	/* Char is weird */
#define AUTO_CHAR_KILL		11	/* Char should be killed */
#define AUTO_CHAR_TAKE		21	/* Char should be gotten */
#define AUTO_CHAR_SHOP		31	/* Char is a shop */
#define AUTO_CHAR_WALL		91	/* Char is a wall */


/*
 * Efficient analysis of various "character codes".
 */

extern byte auto_char[256];	/* Analysis of various char codes */


/*
 * Variables
 */

extern int goal;		/* Current "goal" */

extern int goal_rising;		/* Currently fleeing to town */

extern int last_visit;		/* Last purchase visit */

extern int stair_less;		/* Use the next "up" staircase */
extern int stair_more;		/* Use the next "down" staircase */

extern int count_floor;		/* Number of floor grids */
extern int count_less;		/* Number of stairs (up) */
extern int count_more;		/* Number of stairs (down) */
extern int count_kill;		/* Number of monsters */
extern int count_take;		/* Number of objects */

extern s32b auto_began;		/* When this level began */

extern s32b auto_shock;		/* When last "shocked" */



/*
 * Hack -- forget stuff
 */
extern void borg_ext_wipe(void);

/*
 * Update based on current map
 */
extern void borg_update(void);


/*
 * Low level Goals
 */
extern bool borg_play_old_goal(void);
extern bool borg_caution(void);
extern bool borg_attack(void);
extern bool borg_play_fire(void);


/*
 * High Level goals
 */
extern bool borg_flow_kill(void);
extern bool borg_flow_take(void);
extern bool borg_flow_dark(void);
extern bool borg_flow_explore(void);
extern bool borg_flow_kill_any(void);
extern bool borg_flow_take_any(void);
extern bool borg_flow_symbol(char what);
extern bool borg_flow_revisit(void);
extern bool borg_flow_spastic(void);



/*
 * Init this file
 */
extern void borg_ext_init(void);




#endif

#endif

