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

#ifndef INCLUDED_BORG_STORE_SELL_H
#define INCLUDED_BORG_STORE_SELL_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item.h"

extern int sold_item_tval[10];
extern int sold_item_sval[10];
extern int sold_item_pval[10];
extern int sold_item_store[10];
extern int sold_item_num;
extern int sold_item_nxt;

/*
 * Find the mininum amount of some item to buy/sell.
 */
extern int borg_min_item_quantity(borg_item *item);

/*
 * Sell "useful" things to the home (for later)
 */
extern bool borg_think_home_sell_useful(bool save_best);

/*
 * Sell "useless" items to a shop (for cash)
 */
extern bool borg_think_shop_sell_useless(void);

/*
 * Sell items to the current shop, if desired
 */
extern bool borg_think_shop_sell(void);

/*
 * Estimate the number of items worth "selling"
 */
extern int borg_count_sell(void);

extern void borg_init_store_sell(void);
extern void borg_free_store_sell(void);

#endif
#endif
