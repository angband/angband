/* File: borg7.h */

/* Purpose: Header file for "borg7.c" -BEN- */

#ifndef INCLUDED_BORG7_H
#define INCLUDED_BORG7_H

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg7.c".
 */

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"


/*
 * Determine if an item is "icky"
 */
extern bool borg_item_icky(borg_item *item);

/*
 * Various functions
 */
extern bool borg_use_things(void);
extern bool borg_check_LIGHT(void);
extern bool borg_check_LIGHT_only(void);

extern bool borg_enchanting(void);
extern bool borg_recharging(void);
extern bool borg_crush_junk(void);
extern bool borg_crush_hole(void);
extern bool borg_crush_slow(void);
extern bool borg_test_stuff(void);
extern bool borg_takeoff_stuff(void);
extern bool borg_swap_rings(void);
extern bool borg_wear_rings(void);
extern bool borg_wear_stuff(void);
extern bool borg_wear_quiver(void);
extern bool borg_best_stuff(void);
extern bool borg_play_magic(bool bored);
extern bool borg_remove_stuff(void);
extern bool borg_dump_quiver(void);
extern bool borg_stack_quiver(void);
extern bool borg_wear_recharge(void);
extern int borg_count_sell(void);
/*
 * Attempt to leave the level
 */
extern bool borg_leave_level(bool bored);


/*
 * Initialize this file
 */
extern void borg_init_7(void);


#endif

#endif

