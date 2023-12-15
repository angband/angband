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

#ifndef INCLUDED_BORG_INVENTORY_H
#define INCLUDED_BORG_INVENTORY_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item.h"

/*
 * track if we need to crush junk
 */
extern bool borg_do_crush_junk;

/*
 * Determine which slot an item could be wielded into
 */
extern int borg_wield_slot(const borg_item *item);

/*
 * Find an item with a given tval/sval
 */
extern int borg_slot(int tval, int sval);

/*
 * Cheat/Parse the "equip" and "inven" screens.
 */
extern void borg_cheat_equip(void);
extern void borg_cheat_inven(void);

/*
 * Helper to find the first empty slot
 */
extern int borg_first_empty_inventory_slot(void);

/*
 * Determine if an item is likely to be worthless
 */
extern bool borg_item_worth_id(const borg_item *item);

#endif
#endif
