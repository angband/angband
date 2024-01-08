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

#ifndef INCLUDED_BORG_STORE_BUY_H
#define INCLUDED_BORG_STORE_BUY_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

extern int bought_item_tval[10];
extern int bought_item_sval[10];
extern int bought_item_pval[10];
extern int bought_item_store[10];
extern int bought_item_num;
extern int bought_item_nxt;

/*
 * Buy "useful" things from a shop (to be used)
 */
extern bool borg_think_shop_buy_useful(void);

/*
 * Buy "useful" things from the home (to be used)
 */
extern bool borg_think_home_buy_useful(void);

/*
 * Buy "interesting" things from a shop (to be used later)
 */
extern bool borg_think_shop_grab_interesting(void);

/*
 * Take "useless" things from the home (to be sold)
 */
extern bool borg_think_home_grab_useless(void);

/*
 * Buy "useful" weapons from the home (to be used as a swap)
 */
extern bool borg_think_home_buy_swap_weapon(void);

/*
 * By "useful" armour from the home (to be used as a swap)
 */
extern bool borg_think_home_buy_swap_armour(void);

/*
 * Buy items from the current shop, if desired
 */
extern bool borg_think_shop_buy(void);

#endif
#endif
