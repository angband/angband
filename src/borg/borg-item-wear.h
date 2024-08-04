/**
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_ITEM_WEAR_H
#define INCLUDED_BORG_ITEM_WEAR_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Track the items worn to avoid loops
 */
extern int16_t track_worn_num;

/*
 * Identify items if possible
 */
extern bool borg_test_stuff(void);

/*
 * Left right ring swap.
 */
extern bool borg_swap_rings(void);

/*
 * Place our "best" ring on the "tight" finger if needed
 */
extern bool borg_wear_rings(void);

/*
 * Wear our "swap" if needed.
 */
extern bool borg_backup_swap(int p);

/*
 * Wear useful equipment.
 */
extern bool borg_wear_stuff(void);

/*
 * Attempt to instantiate the *best* possible equipment.
 */
extern bool borg_best_stuff(void);

/*
 * clear the optimal equipment list
 */
extern void borg_clear_best(void);

/*
 * Wear stuff so it recharges.
 */
extern bool borg_wear_recharge(void);

/*
 *  check an item for being ammo.
 */
extern bool borg_is_ammo(int tval);

extern void borg_init_item_wear(void);
extern void borg_free_item_wear(void);

#endif
#endif
