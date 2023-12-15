/**
 * \file borg-store.c
 * \brief Definitions of stores (shops) and store items
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

#include "borg-store.h"

#ifdef ALLOW_BORG

#include "../init.h"
#include "../obj-desc.h"
#include "../obj-power.h"
#include "../obj-tval.h"
#include "../store.h"

#include "borg-item-analyze.h"
#include "borg-item-val.h"
#include "borg-store-sell.h"

const char *SHOP_MENU_ITEMS = "acfhjmnoqruvyzABDFGHJKLMNOPQRSTUVWXYZ";

borg_shop *borg_shops; /* Current "shops" */
borg_shop *safe_shops; /* Safety (save) "shops" */

int borg_food_onsale = -1; /* Are shops selling food? */
int borg_fuel_onsale = -1; /* Are shops selling fuel? */

static int32_t borg_price_item(
    const struct object *obj, bool store_buying, int qty, int this_store)
{
    int           adjust = 100;
    int           price;
    struct owner *proprietor;

    if (this_store == f_info[FEAT_HOME].shopnum - 1) {
        return 0;
    }

    proprietor = stores[this_store].owner;

    /* Get the value of the stack of wands, or a single item */
    if (tval_can_have_charges(obj)) {
        price = MIN(object_value_real(obj, qty), object_value(obj, qty));
    } else {
        price = MIN(object_value_real(obj, 1), object_value(obj, 1));
    }

    /* Worthless items */
    if (price <= 0) {
        return 0;
    }

    /* The black market is always a worse deal */
    if (this_store == f_info[FEAT_STORE_BLACK].shopnum - 1)
        adjust = 150;

    /* Shop is buying */
    if (store_buying) {
        /* Set the factor */
        adjust = 100 + (100 - adjust);
        if (adjust > 100) {
            adjust = 100;
        }

        /* Shops now pay 2/3 of true value */
        price = price * 2 / 3;

        /* Black market sucks */
        if (this_store == f_info[FEAT_STORE_BLACK].shopnum - 1) {
            price = price / 2;
        }

        /* Check for no_selling option */
        if (OPT(player, birth_no_selling)) {
            return 0;
        }
    } else {
        /* Re-evaluate if we're selling */
        if (tval_can_have_charges(obj)) {
            price = object_value_real(obj, qty);
        } else {
            price = object_value_real(obj, 1);
        }

        /* Black market sucks */
        if (this_store == f_info[FEAT_STORE_BLACK].shopnum - 1) {
            price = price * 2;
        }
    }

    /* Compute the final price (with rounding) */
    price = (price * adjust + 50L) / 100L;

    /* Now convert price to total price for non-wands */
    if (!tval_can_have_charges(obj)) {
        price *= qty;
    }

    /* Now limit the price to the purse limit */
    if (store_buying && (price > proprietor->max_cost * qty)) {
        price = proprietor->max_cost * qty;
    }

    /* Note -- Never become "free" */
    if (price <= 0) {
        return qty;
    }

    /* Return the price */
    return price;
}

/*
 * Cheat the "Store" screen
 */
void borg_cheat_store(void)
{
    int             slot, i;
    int             store_num;
    struct object  *o_ptr;
    struct object **list
        = mem_zalloc(sizeof(struct object *) * z_info->store_inven_max);

    /* Scan each store */
    for (store_num = 0; store_num < z_info->store_max; store_num++) {
        struct store *st_ptr = &stores[store_num];

        /* Clear the Inventory from memory */
        for (i = 0; i < z_info->store_inven_max; i++) {
            /* Wipe the ware */
            memset(&borg_shops[store_num].ware[i], 0, sizeof(borg_item));
        }

        store_stock_list(st_ptr, list, z_info->store_inven_max);

        /* Check each existing object in this store */
        for (slot = 0; slot < z_info->store_inven_max && list[slot]; slot++) {
            o_ptr             = list[slot];
            borg_item *b_item = &borg_shops[store_num].ware[slot];
            char       buf[120];

            /* Describe the item */
            object_desc(
                buf, sizeof(buf), o_ptr, ODESC_FULL | ODESC_STORE, player);
            if (streq(buf, "(nothing)"))
                break;

            /* Analyze the item */
            borg_item_analyze(
                b_item, o_ptr, buf, store_num == 7 ? false : true);

            /* Check if the general store has certain items */
            if (store_num == 0) {
                /* Food -- needed for money scumming */
                if (b_item->tval == TV_FOOD && b_item->sval == sv_food_ration)
                    borg_food_onsale = b_item->iqty;

                /* Fuel for lanterns */
                if (b_item->tval == TV_FLASK
                    && borg_items[INVEN_LIGHT].sval == sv_light_lantern)
                    borg_fuel_onsale = b_item->iqty;

                /* Fuel for lanterns */
                if (b_item->tval == TV_LIGHT
                    && borg_items[INVEN_LIGHT].sval == sv_light_torch)
                    borg_fuel_onsale = b_item->iqty;
            }

            /* Hack -- Save the declared cost */
            b_item->cost = borg_price_item(o_ptr, false, 1, store_num);
        }
    }
    mem_free(list);
}

void borg_init_store(void)
{
    /* Make the stores in the town */
    borg_shops = mem_zalloc(9 * sizeof(borg_shop));

    /* Make the "safe" stores in the town */
    safe_shops = mem_zalloc(8 * sizeof(borg_shop));

    borg_init_store_sell();
}

void borg_free_store(void)
{
    borg_free_store_sell();

    mem_free(safe_shops);
    safe_shops = NULL;
    mem_free(borg_shops);
    borg_shops = NULL;
}

#endif
