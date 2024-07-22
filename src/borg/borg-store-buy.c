/**
 * \file borg-store-buy.c
 * \brief Shopping and buying items or getting them from the home
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

#include "borg-store-buy.h"

#ifdef ALLOW_BORG

#include "../obj-util.h"
#include "../player-calcs.h"
#include "../ui-event.h"

#include "borg-flow-kill.h"
#include "borg-home-notice.h"
#include "borg-home-power.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-val.h"
#include "borg-item.h"
#include "borg-magic.h"
#include "borg-power.h"
#include "borg-store-sell.h"
#include "borg-store.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg.h"

static int borg_money_scum_who;
static int borg_money_scum_ware;

int bought_item_tval[10];
int bought_item_sval[10];
int bought_item_pval[10];
int bought_item_store[10];
int bought_item_num = -1;
int bought_item_nxt = 0;

/*
 * Help decide if an item should be bought from a real store
 *
 * We prevent the purchase of enchanted (or expensive) ammo,
 * so we do not spend all our money on temporary power.
 *
 * if level 35, who needs cash?  buy the expensive ammo!
 *
 * We prevent the purchase of low level discounted books,
 * so we will not waste slots on cheap books.
 *
 * We prevent the purchase of items from the black market
 * which are often available at normal stores, currently,
 * this includes low level books, and all wands and staffs.
 */
static bool borg_good_buy(borg_item *item, int who, int ware)
{
    /* Check the object */
    switch (item->tval) {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        if (borg.trait[BI_CLEVEL] < 35) {
            if (item->to_h)
                return false;
            if (item->to_d)
                return false;
        }
        break;

    case TV_PRAYER_BOOK:
    case TV_MAGIC_BOOK:
    case TV_NATURE_BOOK:
    case TV_SHADOW_BOOK:
    case TV_OTHER_BOOK: 
        /* not our book */
        if (!obj_kind_can_browse(&k_info[item->kind]))
            return false;
        break;
    }

    /* Don't buy from the BM until we are rich */
    if (who == 6) {
        /* buying Remove Curse scroll is acceptable */
        if (item->tval == TV_SCROLL && item->sval == sv_scroll_remove_curse
            && borg.trait[BI_FIRST_CURSED])
            return true;

        /* Buying certain special items are acceptable */
        if ((item->tval == TV_POTION
                && ((item->sval == sv_potion_star_healing)
                    || (item->sval == sv_potion_life)
                    || (item->sval == sv_potion_healing)
                    || (item->sval == sv_potion_inc_str
                        && borg.stat_cur[STAT_STR] < (18 + 100))
                    || (item->sval == sv_potion_inc_int
                        && borg.stat_cur[STAT_INT] < (18 + 100))
                    || (item->sval == sv_potion_inc_wis
                        && borg.stat_cur[STAT_WIS] < (18 + 100))
                    || (item->sval == sv_potion_inc_dex
                        && borg.stat_cur[STAT_DEX] < (18 + 100))
                    || (item->sval == sv_potion_inc_con
                        && borg.stat_cur[STAT_CON] < (18 + 100))))
            || (item->tval == TV_ROD
                && ((item->sval == sv_rod_healing) ||
                    /* priests and paladins can cast recall */
                    (item->sval == sv_rod_recall
                        && borg.trait[BI_CLASS] != CLASS_PRIEST
                        && borg.trait[BI_CLASS] != CLASS_PALADIN)
                    ||
                    /* druid and ranger can cast haste*/
                    (item->sval == sv_rod_speed
                        && borg.trait[BI_CLASS] != CLASS_DRUID
                        && borg.trait[BI_CLASS] != CLASS_RANGER)
                    ||
                    /* mage and rogue can cast teleport away */
                    (item->sval == sv_rod_teleport_other
                        && borg.trait[BI_CLASS] != CLASS_MAGE
                        && borg.trait[BI_CLASS] == CLASS_ROGUE)
                    || (item->sval == sv_rod_illumination
                        && (!borg.trait[BI_ALITE]))))
            || (obj_kind_can_browse(&k_info[item->kind])
                && borg.amt_book[borg_get_book_num(item->sval)] == 0
                && borg_is_dungeon_book(item->tval, item->sval))
            || (item->tval == TV_SCROLL
                && (item->sval == sv_scroll_teleport_level
                    || item->sval == sv_scroll_teleport))) {
            /* Hack-- Allow the borg to scum for this Item */
            if (borg_cfg[BORG_SELF_SCUM] && /* borg is allowed to scum */
                borg.trait[BI_CLEVEL] >= 10 && /* Be of sufficient level */
                borg.trait[BI_LIGHT] && /* Have some Perma lite source */
                borg.trait[BI_FOOD] + num_food >= 100
                && /* Have plenty of food */
                item->cost <= 85000) /* Its not too expensive */
            {
                if (adj_dex_safe[borg.trait[BI_DEX]] + borg.trait[BI_CLEVEL]
                    > 90) /* Good chance to thwart mugging */
                {
                    /* Record the amount that I need to make purchase */
                    borg_cfg[BORG_MONEY_SCUM_AMOUNT] = item->cost;
                    borg_money_scum_who              = who;
                    borg_money_scum_ware             = ware;
                }
            }

            /* Ok to buy this */
            return true;
        }

        if ((borg.trait[BI_CLEVEL] < 15) && (borg.trait[BI_GOLD] < 20000))
            return false;
        if ((borg.trait[BI_CLEVEL] < 35) && (borg.trait[BI_GOLD] < 15000))
            return false;
        if (borg.trait[BI_GOLD] < 10000)
            return false;
    }

    /* do not buy the item if I just sold it. */
    for (int p = 0; p < sold_item_num; p++) {

        if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval
            && sold_item_store[p] == who) {
            if (borg_cfg[BORG_VERBOSE])
                borg_note(format("# Choosing not to buy back %s", item->desc));
            return false;
        }
    }

    /* Do not buy a second digger */
    if (item->tval == TV_DIGGING) {
        int ii;

        /* scan for an existing digger */
        for (ii = 0; ii < z_info->pack_size; ii++) {
            borg_item *item2 = &borg_items[ii];

            /* skip non diggers */
            if (item2->tval == TV_DIGGING)
                return false;
#if 0
            /* perhaps let him buy a digger with a better
             * pval than his current digger
             */
            { if (item->pval <= item2->pval) return false; }
#endif
        }
    }

    /* Low level borgs should not waste the money on certain things */
    if (borg.trait[BI_MAXCLEVEL] < 5) {
        /* next book, cant read it */
        if (obj_kind_can_browse(&k_info[item->kind]) && item->sval >= 1)
            return false;
    }

    /* Not direct spell casters and the extra books */
    /* classes that are direct spell casters get more than 3 books */
    if (!borg_primarily_caster() && borg.trait[BI_MAXCLEVEL] <= 8) {
        if (obj_kind_can_browse(&k_info[item->kind]) && item->sval >= 1)
            return false;
    }

    /* Okay */
    return true;
}

/*
 * Step 3 -- buy "useful" things from a shop (to be used)
 */
bool borg_think_shop_buy_useful(void)
{
    int hole = borg_first_empty_inventory_slot();

    int slot;
    int qty = 1;

    int     k, b_k = -1;
    int     n, b_n = -1;
    int32_t p, b_p = 0L;
    int32_t c, b_c = 0L;

    bool fix = false;

    /* Require one empty slot */
    if (hole == -1)
        return false;

    /* Already have a target 9-4-05*/
    if (borg.goal.ware != -1)
        return false;

    /* Extract the "power" */
    b_p = borg.power;
    b_p = borg_power();

    /* Check the shops */
    for (k = 0; k < (z_info->store_max - 1); k++) {

        /* If I am bad shape up, only see certain stores */
        if ((borg.trait[BI_CURLITE] == 0 || borg.trait[BI_FOOD] == 0) && k != 0
            && k != BORG_HOME)
            continue;
        if ((borg.trait[BI_ISCUT] || borg.trait[BI_ISPOISONED]) && k != 3)
            continue;

        /* Scan the wares */
        for (n = 0; n < z_info->store_inven_max; n++) {
            borg_item *item = &borg_shops[k].ware[n];

            /* Skip empty items */
            if (!item->iqty)
                continue;

            /* Skip "bad" buys */
            if (!borg_good_buy(item, k, n))
                continue;

            /* Attempting to scum money, don't buy other stuff unless it is our
             * home or food-store */
            if (borg_cfg[BORG_MONEY_SCUM_AMOUNT]
                && (k != borg_money_scum_who || n != borg_money_scum_ware))
                continue;

            /* Hack -- Require "sufficient" cash */
            if (borg.trait[BI_GOLD] < item->cost)
                continue;

            /* Skip it if I just sold this item. XXX XXX*/

            /* Special check for 'immediate shopping' */
            if (borg.trait[BI_FOOD] == 0
                && (item->tval != TV_FOOD
                    && (item->tval != TV_SCROLL
                        && item->sval != sv_scroll_satisfy_hunger)))
                continue;

            /* Don't fill up on attack wands, its ok to buy a few */
            if (item->tval == TV_WAND
                && (item->sval == sv_wand_magic_missile
                    || item->sval == sv_wand_stinking_cloud)
                && borg.trait[BI_GOOD_W_CHG] > 40)
                continue;

            /* These wands are not useful later on, we need beefier attacks */
            if (item->tval == TV_WAND
                && (item->sval == sv_wand_magic_missile
                    || item->sval == sv_wand_stinking_cloud)
                && borg.trait[BI_MAXCLEVEL] > 30)
                continue;

            /* Save shop item */
            memcpy(&safe_shops[k].ware[n], &borg_shops[k].ware[n],
                sizeof(borg_item));

            /* Save hole */
            memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

            /* Save the number to trade */
            qty = borg_min_item_quantity(item);

            /* Remove one item from shop (sometimes) */
            borg_shops[k].ware[n].iqty -= qty;

            /* Obtain "slot" */
            slot = borg_wield_slot(item);

            /* XXX what if the item is a ring?  we have 2 ring slots --- copy it
             * from the Home code */

            /* He will not replace his Brightness Torch with a plain one, so he
             * ends up not buying any torches.  Force plain torches for purchase
             * to be seen as fuel only
             */
            if (item->tval == TV_LIGHT && item->sval == sv_light_torch
                && of_has(borg_items[INVEN_LIGHT].flags, OF_BURNS_OUT)) {
                slot = -1;
            }

            /* Hack, we keep diggers as a back-up, not to
             * replace our current weapon
             */
            if (item->tval == TV_DIGGING)
                slot = -1;

            /* Consider new equipment */
            if (slot >= 0) {
                /* Save old item */
                memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

                /* Move equipment into inventory */
                memcpy(&borg_items[hole], &safe_items[slot], sizeof(borg_item));

                /* Move new item into equipment */
                memcpy(&borg_items[slot], &safe_shops[k].ware[n],
                    sizeof(borg_item));

                /* Only a single item */
                borg_items[slot].iqty = qty;

                /* Fix later */
                fix = true;

                /* Examine the inventory */
                borg_notice(false);

                /* Evaluate the inventory */
                p = borg_power();

                /* Restore old item */
                memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));
            }

            /* Consider new inventory */
            else {
                /* Move new item into inventory */
                memcpy(&borg_items[hole], &safe_shops[k].ware[n],
                    sizeof(borg_item));

                /* Only a single item */
                borg_items[hole].iqty = qty;

                /* Fix later */
                fix = true;

                /* Examine the inventory */
                borg_notice(false);

                /* Evaluate the equipment */
                p = borg_power();
            }

            /* Restore hole */
            memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

            /* Restore shop item */
            memcpy(&borg_shops[k].ware[n], &safe_shops[k].ware[n],
                sizeof(borg_item));

            /* Obtain the "cost" of the item */
            c = item->cost * qty;

#if 0
            /* Penalize the cost of expensive items */
            if (c > borg_gold / 10) p -= c;
#endif

            /* Ignore "bad" purchases */
            if (p <= b_p)
                continue;

            /* Ignore "expensive" purchases */
            if ((p == b_p) && (c >= b_c))
                continue;

            /* Save the item and cost */
            b_k = k;
            b_n = n;
            b_p = p;
            b_c = c;
        }
    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Buy something */
    if ((b_k >= 0) && (b_n >= 0)) {
        /* Visit that shop */
        borg.goal.shop = b_k;

        /* Buy that item */
        borg.goal.ware = b_n;

        /* Success */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Step 4 -- buy "useful" things from the home (to be used)
 */
bool borg_think_home_buy_useful(void)
{

    int     hole;
    int     slot, i;
    int     stack;
    int     qty = 1;
    int     n, b_n = -1;
    int32_t p, b_p = 0L;
    int32_t p_left  = 0;
    int32_t p_right = 0;

    bool fix        = false;
    bool skip_it    = false;

    /* Extract the "power" */
    b_p = borg.power;

    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++) {
        borg_item *item = &borg_shops[BORG_HOME].ware[n];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip it if I just sold it */
        for (i = 0; i < sold_item_num; i++) {
            if (sold_item_tval[i] == item->tval
                && sold_item_sval[i] == item->sval) {
                if (borg_cfg[BORG_VERBOSE])
                    borg_note(
                        format("# Choosing not to buy back '%s' from home.",
                            item->desc));
                skip_it = true;
            }
        }
        if (skip_it == true)
            continue;

        /* Reset the 'hole' in case it was changed by the last stacked item.*/
        hole = borg_first_empty_inventory_slot();
        if (hole == -1)
            continue;

        /* borg_note(format("# Considering buying (%d)'%s' (pval=%d) from
         * home.", item->iqty,item->desc, item->pval)); */

        /* Save shop item */
        memcpy(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Save hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Save the number */
        qty = borg_min_item_quantity(item);

        /* Remove one item from shop (sometimes) */
        borg_shops[BORG_HOME].ware[n].iqty -= qty;

        /* Obtain "slot" */
        slot  = borg_wield_slot(item);
        stack = borg_slot(item->tval, item->sval);

        /* Consider new equipment-- Must check both ring slots */
        p = 0;
        if (slot >= 0) {
            /* Require two empty slots */
            if (hole == -1)
                continue;
            if ((hole + 1) >= PACK_SLOTS)
                continue;

            /* Check Rings */
            if (slot == INVEN_LEFT) {
                /** First Check Left Hand **/

                /* special curse check for left ring */
                if (!borg_items[INVEN_LEFT].one_ring) {
                    /* Save old item */
                    memcpy(&safe_items[slot], &borg_items[slot],
                        sizeof(borg_item));

                    /* Move equipment into inventory */
                    memcpy(&borg_items[hole], &safe_items[slot],
                        sizeof(borg_item));

                    /* Move new item into equipment */
                    memcpy(&borg_items[slot], &safe_shops[BORG_HOME].ware[n],
                        sizeof(borg_item));

                    /* Only a single item */
                    borg_items[slot].iqty = qty;

                    /* Fix later */
                    fix = true;

                    /* Examine the inventory */
                    borg_notice(false);

                    /* Evaluate the inventory */
                    p_left = borg_power();
#if 0
                    /* dump list and power...  for debugging */
                    borg_note(format("Trying Item %s (best power %ld)", borg_items[slot].desc, p_left));
                    borg_note(format("   Against Item %s   (borg_power %ld)", safe_items[slot].desc, borg.power));
#endif
                    /* Restore old item */
                    memcpy(&borg_items[slot], &safe_items[slot],
                        sizeof(borg_item));
                }

                /** Second Check Right Hand **/
                /* special curse check for right ring */
                if (!borg_items[INVEN_RIGHT].one_ring) {
                    /* Save old item */
                    memcpy(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT],
                        sizeof(borg_item));

                    /* Move equipment into inventory */
                    memcpy(&borg_items[hole], &safe_items[INVEN_RIGHT],
                        sizeof(borg_item));

                    /* Move new item into equipment */
                    memcpy(&borg_items[INVEN_RIGHT],
                        &safe_shops[BORG_HOME].ware[n], sizeof(borg_item));

                    /* Only a single item */
                    borg_items[INVEN_RIGHT].iqty = qty;

                    /* Fix later */
                    fix = true;

                    /* Examine the inventory */
                    borg_notice(false);

                    /* Evaluate the inventory */
                    p_right = borg_power();

#if 0
                    /* dump list and power...  for debugging */
                    borg_note(format("Trying Item %s (best power %ld)", borg_items[INVEN_RIGHT].desc, p_right));
                    borg_note(format("   Against Item %s   (borg_power %ld)", safe_items[INVEN_RIGHT].desc, borg.power));
#endif
                    /* Restore old item */
                    memcpy(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT],
                        sizeof(borg_item));
                }

                /* Is this ring better than one of mine? */
                p = MAX(p_right, p_left);

                /* Restore hole */
                memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));
            }

            else /* non rings */
            {
                /* Save old item */
                memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

                /* Move equipment into inventory */
                memcpy(&borg_items[hole], &safe_items[slot], sizeof(borg_item));

                /* Move new item into equipment */
                memcpy(&borg_items[slot], &safe_shops[BORG_HOME].ware[n],
                    sizeof(borg_item));

                /* Only a single item */
                borg_items[slot].iqty = qty;

                /* Fix later */
                fix = true;

                /* Examine the inventory */
                borg_notice(false);

                /* Evaluate the inventory */
                p = borg_power();
#if 0
                /* dump list and power...  for debugging */
                borg_note(format("Trying Item %s (best power %ld)", borg_items[slot].desc, p));
                borg_note(format("   Against Item %s   (borg_power %ld)", safe_items[slot].desc, borg.power));
#endif
                /* Restore old item */
                memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));
            } /* non rings */
        } /* equip */

        /* Consider new inventory.*/
        /* note, we may grab an equip able if, for example, we want to ID it */
        if (p <= b_p) {
            /* Restore hole if we are trying an item in inventory that didn't
             * work equipped */
            if (slot >= 0)
                memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

            if (stack != -1)
                hole = stack;

            /* Require two empty slots */
            if (stack == -1 && hole == -1)
                continue;
            if (stack == -1 && (hole + 1) >= PACK_SLOTS)
                continue;

            /* Save hole (could be either empty slot or stack */
            memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

            /* Move new item into inventory */
            memcpy(&borg_items[hole], &safe_shops[BORG_HOME].ware[n],
                sizeof(borg_item));

            /* Is this new item merging into an existing stack? */
            if (stack != -1) {
                /* Add a quantity to the stack */
                borg_items[hole].iqty = safe_items[hole].iqty + qty;
            } else {
                /* Only a single item */
                borg_items[hole].iqty = qty;
            }

            /* Fix later */
            fix = true;

            /* Examine the inventory */
            borg_notice(false);

            /* Evaluate the equipment */
            p = borg_power();
        }

        /* Restore hole */
        memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

        /* Restore shop item */
        memcpy(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Ignore "silly" purchases */
        if (p <= b_p)
            continue;

        /* Save the item and cost */
        b_n = n;
        b_p = p;
    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Buy something */
    if ((b_n >= 0) && (b_p > borg.power)) {
        /* Go to the home */
        borg.goal.shop = BORG_HOME;

        /* Buy that item */
        borg.goal.ware = b_n;

        /* Success */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Step 5 -- buy "interesting" things from a shop (to be used later)
 */
bool borg_think_shop_grab_interesting(void)
{
    int k, b_k = -1;
    int n, b_n = -1;
    int qty   = 1;

    int32_t s = 0L, b_s = 0L;
    int32_t c, b_c      = 0L;
    int32_t borg_empty_home_power;
    int     hole;

    /* Don't do this if Sauron is dead */
    if (borg.trait[BI_SAURON_DEAD])
        return false;

    /* not until later-- use that money for better equipment */
    if (borg.trait[BI_CLEVEL] < 15)
        return false;

    /* get what an empty home would have for power */
    borg_notice_home(NULL, true);
    borg_empty_home_power = borg_power_home();

    hole                  = borg_first_empty_inventory_slot();

    /* Require two empty slots */
    if (hole == -1)
        return false;
    if (hole + 1 >= PACK_SLOTS)
        return false;

    /* Examine the home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    b_s = borg_power_home();

    /* Check the shops */
    for (k = 0; k < (z_info->store_max - 1); k++) {
        /* Scan the wares */
        for (n = 0; n < z_info->store_inven_max; n++) {
            borg_item *item = &borg_shops[k].ware[n];

            /* Skip empty items */
            if (!item->iqty)
                continue;

            /* Skip "bad" buys */
            if (!borg_good_buy(item, k, n))
                continue;

            /* Don't buy easy spell books late in the game */
            /* Hack -- Require some "extra" cash */
            if (borg.trait[BI_GOLD] < 1000L + item->cost * 5)
                continue;

            /* make this the next to last item that the player has */
            /* (can't make it the last or it thinks that both player and */
            /*  home are full) */
            memcpy(
                &borg_items[hole], &borg_shops[k].ware[n], sizeof(borg_item));

            /* Save the number */
            qty = borg_min_item_quantity(item);

            /* Give a single item */
            borg_items[hole].iqty = qty;

            /* make sure this item would help an empty home */
            borg_notice_home(&borg_shops[k].ware[n], false);
            if (borg_empty_home_power >= borg_power_home())
                continue;

            /* optimize the home inventory */
            if (!borg_think_home_sell_useful(true))
                continue;

            /* Obtain the "cost" of the item */
            c = item->cost * qty;

            /* Penalize expensive items */
            if (c > borg.trait[BI_GOLD] / 10)
                s -= c;

            /* Ignore "bad" sales */
            if (s < b_s)
                continue;

            /* Ignore "expensive" purchases */
            if ((s == b_s) && (c >= b_c))
                continue;

            /* Save the item and cost */
            b_k = k;
            b_n = n;
            b_s = s;
            b_c = c;
        }
    }

    /* restore inventory hole (just make sure the last slot goes back to */
    /* empty) */
    borg_items[hole].iqty = 0;

    /* Examine the real home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    s = borg_power_home();

    /* remove the target that optimizing the home gave */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Buy something */
    if ((b_k >= 0) && (b_n >= 0)) {
        /* Visit that shop */
        borg.goal.shop = b_k;

        /* Buy that item */
        borg.goal.ware = b_n;

        /* Success */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Step 6 -- take "useless" things from the home (to be sold)
 */
bool borg_think_home_grab_useless(void)
{
    int     p, n, b_n = -1;
    int32_t s, b_s = 0L;
    int     qty     = 1;
    bool    skip_it = false;
    int     hole    = borg_first_empty_inventory_slot();

    /* Require two empty slots */
    if (hole == -1)
        return false;
    if (hole + 1 >= PACK_SLOTS)
        return false;

    /* Examine the home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    b_s = borg_power_home();

    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++) {
        borg_item *item = &borg_shops[BORG_HOME].ware[n];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* skip stuff if we sold bought it */
        for (p = 0; p < sold_item_num; p++) {
            if (sold_item_tval[p] == item->tval
                && sold_item_sval[p] == item->sval
                && sold_item_store[p] == BORG_HOME)
                skip_it = true;
        }
        if (skip_it == true)
            continue;

        /* Save shop item */
        memcpy(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Save the number */
        qty = borg_min_item_quantity(item);

        /* Remove one item from shop */
        borg_shops[BORG_HOME].ware[n].iqty -= qty;

        /* Examine the home */
        borg_notice_home(NULL, false);

        /* Evaluate the home */
        s = borg_power_home();

        /* Restore shop item */
        memcpy(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Ignore "bad" sales */
        if (s < b_s)
            continue;

        /* Maintain the "best" */
        b_n = n;
        b_s = s;
    }

    /* Examine the home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    s = borg_power_home();

    /* Stockpile */
    if (b_n >= 0) {
        /* Visit the home */
        borg.goal.shop = BORG_HOME;

        /* Grab that item */
        borg.goal.ware = b_n;

        /* Success */
        return true;
    }

    /* Assume not */
    return false;
}

/*
 * Step 7A -- buy "useful" weapons from the home (to be used as a swap)
 */
bool borg_think_home_buy_swap_weapon(void)
{
    int hole;

    int     slot;
    int     old_weapon_swap;
    int32_t old_weapon_swap_value;
    int     old_armour_swap;
    int32_t old_armour_swap_value;
    int     n, b_n = -1;
    int32_t p = 0L, b_p = 0L;

    bool fix = false;

    /* save the current values */
    old_weapon_swap       = weapon_swap;
    old_weapon_swap_value = weapon_swap_value;
    old_armour_swap       = armour_swap;
    old_armour_swap_value = armour_swap_value;

    if (weapon_swap <= 0 || weapon_swap_value <= 0) {
        hole              = borg_first_empty_inventory_slot();
        weapon_swap_value = -1L;
    } else {
        hole = weapon_swap - 1;
    }
    if (hole == -1)
        return false;

    /* Extract the "power" */
    b_p = weapon_swap_value;

    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++) {
        borg_item *item = &borg_shops[BORG_HOME].ware[n];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Obtain "slot" make sure its a weapon */
        slot = borg_wield_slot(item);
        if (slot != INVEN_WIELD)
            continue;

        /* Save shop item */
        memcpy(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Save hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Remove one item from shop */
        borg_shops[BORG_HOME].ware[n].iqty--;

        /* Consider new equipment */
        if (slot == INVEN_WIELD) {
            /* Move new item into inventory */
            memcpy(&borg_items[hole], &safe_shops[BORG_HOME].ware[n],
                sizeof(borg_item));

            /* Only a single item */
            borg_items[hole].iqty = 1;

            /* Fix later */
            fix = true;

            /* Examine the inventory and swap value*/
            borg_notice(true);

            /* Evaluate the new equipment */
            p = weapon_swap_value;
        }

        /* Restore hole */
        memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

        /* Restore shop item */
        memcpy(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Ignore "silly" purchases */
        if (p <= b_p)
            continue;

        /* Save the item and value */
        b_n = n;
        b_p = p;
    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Buy something */
    if ((b_n >= 0) && (b_p > weapon_swap_value)) {
        /* Go to the home */
        borg.goal.shop = BORG_HOME;

        /* Buy that item */
        borg.goal.ware = b_n;

        /* Restore the values */
        weapon_swap       = old_weapon_swap;
        weapon_swap_value = old_weapon_swap_value;
        armour_swap       = old_armour_swap;
        armour_swap_value = old_armour_swap_value;

        /* Success */
        return true;
    }

    /* Restore the values */
    weapon_swap       = old_weapon_swap;
    weapon_swap_value = old_weapon_swap_value;
    armour_swap       = old_armour_swap;
    armour_swap_value = old_armour_swap_value;

    /* Nope */
    return false;
}

/*
 * Step 7B -- buy "useful" armour from the home (to be used as a swap)
 */
bool borg_think_home_buy_swap_armour(void)
{
    int hole;

    int     n, b_n = -1;
    int32_t p, b_p = 0L;
    bool    fix = false;
    int     old_weapon_swap;
    int32_t old_weapon_swap_value;
    int     old_armour_swap;
    int32_t old_armour_swap_value;

    /* save the current values */
    old_weapon_swap       = weapon_swap;
    old_weapon_swap_value = weapon_swap_value;
    old_armour_swap       = armour_swap;
    old_armour_swap_value = armour_swap_value;

    if (armour_swap <= 0 || armour_swap_value <= 0) {
        hole              = borg_first_empty_inventory_slot();
        armour_swap_value = -1L;
    } else {
        hole = armour_swap - 1;
    }

    if (hole == -1)
        return false;

    /* Extract the "power" */
    b_p = armour_swap_value;

    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++) {
        borg_item *item = &borg_shops[BORG_HOME].ware[n];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Save shop item */
        memcpy(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Save hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Remove one item from shop */
        borg_shops[BORG_HOME].ware[n].iqty--;

        /* Move new item into inventory */
        memcpy(&borg_items[hole], &safe_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Only a single item */
        borg_items[hole].iqty = 1;

        /* Fix later */
        fix = true;

        /* Examine the inventory (false)*/
        borg_notice(true);

        /* Evaluate the new equipment */
        p = armour_swap_value;

        /* Restore hole */
        memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

        /* Restore shop item */
        memcpy(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
            sizeof(borg_item));

        /* Ignore "silly" purchases */
        if (p <= b_p)
            continue;

        /* Save the item and value */
        b_n = n;
        b_p = p;
    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Buy something */
    if ((b_n >= 0) && (b_p > armour_swap_value)) {
        /* Go to the home */
        borg.goal.shop = BORG_HOME;

        /* Buy that item */
        borg.goal.ware = b_n;

        /* Restore the values */
        weapon_swap       = old_weapon_swap;
        weapon_swap_value = old_weapon_swap_value;
        armour_swap       = old_armour_swap;
        armour_swap_value = old_armour_swap_value;

        /* Success */
        return true;
    }
    /* Restore the values */
    weapon_swap       = old_weapon_swap;
    weapon_swap_value = old_weapon_swap_value;
    armour_swap       = old_armour_swap;
    armour_swap_value = old_armour_swap_value;

    /* Nope */
    return false;
}

/*
 * Buy items from the current shop, if desired
 */
bool borg_think_shop_buy(void)
{
    char purchase_target = '0';

    /* Buy something if requested */
    if ((borg.goal.shop == shop_num) && (borg.goal.ware >= 0)) {
        borg_shop *shop = &borg_shops[borg.goal.shop];

        borg_item *item = &shop->ware[borg.goal.ware];

        purchase_target = SHOP_MENU_ITEMS[borg.goal.ware];

        /* Paranoid */
        if (item->tval == 0) {
            /* The purchase is complete */
            borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

            /* Increment our clock to avoid loops */
            borg.time_this_panel++;

            return false;
        }

        /* Log */
        borg_note(format("# Buying %s.", item->desc));

        /* Buy the desired item */
        borg_keypress(purchase_target);
        borg_keypress('p');

        /* Mega-Hack -- Accept the purchase */
        borg_keypress(KC_ENTER);
        borg_keypress(KC_ENTER);

        /* if the borg is scumming and bought it.,
         * reset the scum amount.
         */
        if (borg_cfg[BORG_MONEY_SCUM_AMOUNT]
            && (item->cost >= borg_cfg[BORG_MONEY_SCUM_AMOUNT] * 9 / 10)) {
            borg_cfg[BORG_MONEY_SCUM_AMOUNT] = 0;

            /* Log */
            borg_note(format("# Setting Money Scum to %d.",
                borg_cfg[BORG_MONEY_SCUM_AMOUNT]));
        }

        /* Remember what we bought to avoid buy/sell loops */
        if (bought_item_nxt >= 9)
            bought_item_nxt = 0;
        bought_item_pval[bought_item_nxt]  = item->pval;
        bought_item_tval[bought_item_nxt]  = item->tval;
        bought_item_sval[bought_item_nxt]  = item->sval;
        bought_item_store[bought_item_nxt] = borg.goal.shop;
        bought_item_num                    = bought_item_nxt;
        bought_item_nxt++;

        /* The purchase is complete */
        borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

        /* Increment our clock to avoid loops */
        borg.time_this_panel++;

        /* leave the store */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* I'm not in a store */
        borg.in_shop = false;

        /* Success */
        return true;
    }

    /* Nothing to buy */
    return false;
}

#endif
