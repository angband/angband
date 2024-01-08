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

#ifndef INCLUDED_BORG_ITEM_USE_H
#define INCLUDED_BORG_ITEM_USE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Item usage functions
 */
enum borg_need {
    BORG_NO_NEED,
    BORG_MET_NEED,
    BORG_UNMET_NEED,
};

/*
 * Attempt to quaff a potion of cure critical wounds.
 */
extern bool borg_quaff_crit(bool no_check);

/*
 * Attempt to quaff the given potion (by sval)
 */
extern bool borg_quaff_potion(int sval);

/*
 * Attempt to quaff an unknown potion
 */
extern bool borg_quaff_unknown(void);

/*
 * Attempt to read the given scroll (by sval)
 */
extern bool borg_read_scroll(int sval);

/*
 * Attempt to read an unknown scroll
 */
extern bool borg_read_unknown(void);

/*
 * Attempt to eat the given food or mushroom
 */
extern bool borg_eat(int tval, int sval);

/*
 * Attempt to eat an unknown food/mushroom.
 */
extern bool borg_eat_unknown(void);

/*
 * Prevent starvation by any means possible
 */
extern bool borg_eat_food_any(void);

/*
 * Checks for rod (by sval)
 */
extern bool borg_equips_rod(int sval);

/*
 * Attempt to zap the given (charged) rod (by sval)
 */
extern bool borg_zap_rod(int sval);

/*
 * Attempt to use the given (charged) staff (by sval)
 */
extern bool borg_use_staff(int sval);

/*
 * Attempt to use an unknown staff.
 */
extern bool borg_use_unknown(void);

/*
 * Attempt to use the given (charged) staff (by sval) and
 * make a fail check on it.
 */
extern bool borg_use_staff_fail(int sval);

/*
 * Checks for staff (by sval) and make a "will I fail" check on it.
 */
extern bool borg_equips_staff_fail(int sval);

/*
 * Attempt to aim the given (charged) wand (by sval)
 */
extern bool borg_aim_wand(int sval);

/*
 * Check and see if borg is wielding a ring with fail check.
 */
extern bool borg_equips_ring(int ring_sval);

/*
 *  Attempt to use the given ring
 */
extern bool borg_activate_ring(int ring_sval);

/*
 * Check if borg is wielding a dragon armor with fail check.
 */
extern bool borg_equips_dragon(int drag_sval);

/*
 *  Attempt to use the given dragon armour
 */
extern bool borg_activate_dragon(int drag_sval);

/*
 * Attempt to use the given artifact
 */
extern bool borg_activate_item(int activation);

/*
 * check and see if borg is wielding an item with this activation
 */
extern bool borg_equips_item(int activation, bool check_charge);

/*
 * Return the relative chance for failure to activate an item.
 */
extern int borg_activate_failure(int tval, int sval);

/*
 * Use things in a useful, but non-essential, manner
 */
extern bool borg_use_things(void);

/*
 * Recharge things
 */
extern bool borg_recharging(void);

#endif
#endif
