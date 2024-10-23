/**
 * \file borg-store-sell.c
 * \brief Think about what you want to sell as well as store at home
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

#include "borg-store-sell.h"

#ifdef ALLOW_BORG

#include "../obj-util.h"
#include "../ui-menu.h"

#include "borg-home-notice.h"
#include "borg-home-power.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-analyze.h"
#include "borg-item-id.h"
#include "borg-item-val.h"
#include "borg-item-wear.h"
#include "borg-power.h"
#include "borg-store-buy.h"
#include "borg-store.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg.h"

int sold_item_tval[10];
int sold_item_sval[10];
int sold_item_pval[10];
int sold_item_store[10];
int sold_item_num = -1;
int sold_item_nxt = 0;

uint8_t *test_item;
uint8_t *best_item;

/*
 * Determine if an item can "absorb" a second item
 *
 * See "object_absorb()" for the actual "absorption" code.
 *
 * If permitted, we allow wands/staffs (if they are known to have equal
 * charges) and rods (if fully charged) to combine.
 *
 * Note that rods/staffs/wands are then unstacked when they are used.
 *
 * If permitted, we allow weapons/armor to stack, if they both known.
 *
 * Food, potions, scrolls, and "easy know" items always stack.
 *
 * Chests never stack (for various reasons).
 *
 * We do NOT allow activatable items (artifacts or dragon scale mail)
 * to stack, to keep the "activation" code clean.  Artifacts may stack,
 * but only with another identical artifact (which does not exist).
 *
 * Ego items may stack as long as they have the same ego-item type.
 * This is primarily to allow ego-missiles to stack.
 */
static bool borg_object_similar(borg_item *o_ptr, borg_item *j_ptr)
{
    /* NOTE: This assumes the giving of one item at a time */
    int total = o_ptr->iqty + 1;

    /* Require identical object types */
    if (o_ptr->kind != j_ptr->kind)
        return 0;

    /* Analyze the items */
    switch (o_ptr->tval) {
        /* Chests */
    case TV_CHEST: {
        /* Never okay */
        return 0;
    }

    /* Food and Potions and Scrolls */
    case TV_FOOD:
    case TV_POTION:
    case TV_SCROLL: {
        /* Assume okay */
        break;
    }

    /* Staffs and Wands */
    case TV_STAFF:
    case TV_WAND: {
        /* Require knowledge */
        if ((!o_ptr->aware) || (!j_ptr->aware))
            return 0;

        /* Fall through */
    }

    /* Staffs and Wands and Rods */
    case TV_ROD: {
        /* Require permission */
        /*            if (!testing_stack) return 0;*/

        /* Require identical charges */
        /*            if (o_ptr->pval != j_ptr->pval) return 0; */

        /* Probably okay */
        break;
    }

    /* Weapons and Armor */
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR: {
        /* Require permission */
        /*            if (!testing_stack) return 0;*/

        /* XXX XXX XXX Require identical "sense" status */
        /* if ((o_ptr->ident & ID_SENSE) != */
        /*     (j_ptr->ident & ID_SENSE)) return 0; */

        /* Fall through */
    }

    /* Rings, Amulets, Lites */
    case TV_RING:
    case TV_AMULET:
    case TV_LIGHT:
        /* Require full knowledge of both items */
        if ((!o_ptr->aware) || (!j_ptr->aware))
            return 0;
        /* fall through */
        /* Missiles */
    case TV_BOLT:
    case TV_ARROW:
    case TV_SHOT: {
        /* Require identical "bonuses" */
        if (o_ptr->to_h != j_ptr->to_h)
            return false;
        if (o_ptr->to_d != j_ptr->to_d)
            return false;
        if (o_ptr->to_a != j_ptr->to_a)
            return false;

        /* Require identical "pval" code */
        if (o_ptr->pval != j_ptr->pval)
            return false;

        /* Require identical "artifact" names */
        if (o_ptr->art_idx != j_ptr->art_idx)
            return false;

        /* Require identical "ego-item" names */
        if (o_ptr->ego_idx != j_ptr->ego_idx)
            return false;

        /* Hack -- Never stack "powerful" items */
        if (!of_is_empty(o_ptr->flags) || !of_is_empty(j_ptr->flags))
            return false;

        /* Hack -- Never stack recharging items */
        if (o_ptr->timeout || j_ptr->timeout)
            return false;

        /* Require identical "values" */
        if (o_ptr->ac != j_ptr->ac)
            return false;
        if (o_ptr->dd != j_ptr->dd)
            return false;
        if (o_ptr->ds != j_ptr->ds)
            return false;

        /* Probably okay */
        break;
    }

    /* Various */
    default: {
        /* Require knowledge */
        if ((!o_ptr->aware) || (!j_ptr->aware))
            return 0;

        /* Probably okay */
        break;
    }
    }

    /* Hack -- Require identical "broken" status */
    if ((o_ptr->ident) != (j_ptr->ident))
        return 0;

    /* The stuff with 'note' is not right but it is close.  I think it */
    /* has him assuming that he can't stack sometimes when he can.  This */
    /* is alright, it just causes him to take a bit more time to do */
    /* some exchanges. */
    /* Hack -- require semi-matching "inscriptions" */
    if ((o_ptr->note && !j_ptr->note) || (!o_ptr->note && j_ptr->note))
        return 0;

    if (o_ptr->note && j_ptr->note) {
        if (o_ptr->note[0] && j_ptr->note[0]
            && (!streq(o_ptr->note, j_ptr->note)))
            return 0;

        /* Hack -- normally require matching "inscriptions" */
        if ((!streq(o_ptr->note, j_ptr->note)))
            return 0;
    }

    /* Maximal "stacking" limit */
    if (total >= k_info[o_ptr->kind].base->max_stack)
        return 0;

    /* They match, so they must be similar */
    return true;
}

/*
 * Find the minimum amount of some item to buy/sell. For most
 * items this is 1, but for certain items (such as ammunition)
 * it may be higher.  -- RML
 */
int borg_min_item_quantity(borg_item *item)
{
    /* Only trade in bunches if sufficient cash */
    if (borg.trait[BI_GOLD] < 250)
        return 1;

    /* Don't trade expensive items in bunches */
    if (item->value > 5)
        return 1;

    /* Don't trade non-known items in bunches */
    if (!item->aware)
        return 1;

    /* Only allow some types */
    switch (item->tval) {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        /* Maximum number of items */
        if (item->iqty < 5)
            return (item->iqty);
        return 5;

    case TV_FOOD:
        if (item->iqty < 3)
            return (item->iqty);
        return 3;
#if 0
    case TV_POTION:
    case TV_SCROLL:
    if (item->iqty < 2)
        return (item->iqty);
    return 2;
#endif

    default:
        return 1;
    }
}

static bool borg_think_home_sell_bad(int i, int32_t borg_empty_home_power)
{
    borg_item *item = &borg_items[i];

    int charge_each;

    /* Skip empty or unknown items */
    if (!item->iqty || (!item->kind && !item->aware))
        return true;

    /* Skip swap items */
    if (weapon_swap && i == weapon_swap - 1)
        return true;
    if (armour_swap && i == armour_swap - 1)
        return true;

    /* Do not dump stuff at home that is not fully id'd and should be */
    /* this is good with random artifacts. */
    if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
        return true;

    /* Hack -- ignore "worthless" items */
    if (!item->value)
        return true;

    /* If this item was just bought a the house, don't tell it back to
     * the house */
    for (int p = 0; p < bought_item_num; p++) {
        if (bought_item_tval[p] == item->tval
            && bought_item_sval[p] == item->sval
            && bought_item_pval[p] == item->pval
            && bought_item_store[p] == BORG_HOME)
            return true;
    }

    borg_notice_home(item, false);
    if (borg_power_home() <= borg_empty_home_power)
        return true;

    /* for wands and staffs adjust charges */
    if (item->tval == TV_STAFF || item->tval == TV_WAND)
        charge_each = item->pval / item->iqty;

    /* assume one item sold.  */
    item->iqty--;

    /* for wands and staffs adjust charges */
    if (item->tval == TV_STAFF || item->tval == TV_WAND)
        item->pval -= charge_each;

    /* Examine borg */
    borg_notice(true);

    /* if this reduces the borgs power, it is a bad transaction */
    if (borg_power() < borg.power) {
        item->iqty++;

        /* for wands and staffs adjust charges */
        if (item->tval == TV_STAFF || item->tval == TV_WAND)
            item->pval += charge_each;

        return true;
    }
    item->iqty++;

    /* for wands and staffs adjust charges */
    if (item->tval == TV_STAFF || item->tval == TV_WAND)
        item->pval += charge_each;

    return false;
}


/*
 * this will see what single addition/substitution is best for the home.
 */
static void borg_think_home_sell_best(int32_t * best_home_power)
{
    borg_item *item;
    borg_item *item2;
    int32_t    home_power;
    int32_t    borg_empty_home_power;
    int        i, k, n;
    int        charge_each = 0;

    /* get what an empty home would have for power */
    borg_notice_home(NULL, true);
    borg_empty_home_power = borg_power_home();


    /* get the starting best (current) */
    /* Examine the home  */
    borg_notice_home(NULL, false);

    /* Evaluate the home  */
    *best_home_power = borg_power_home();

    int first_empty;
    for (first_empty = 0; first_empty < z_info->store_inven_max; first_empty++) {
        if (borg_shops[BORG_HOME].ware[first_empty].iqty == 0)
            break;
    }
    if (first_empty < z_info->store_inven_max)
        first_empty++;

    /* try individual substitutions/additions.   */
    for (n = 0; n < first_empty; n++) {
        item2 = &borg_shops[BORG_HOME].ware[n];
        for (i = 0; i < z_info->pack_size; i++) {
            item = &borg_items[i];

            if (borg_think_home_sell_bad(i, borg_empty_home_power))
                continue;

            /* for wands and staffs adjust charges */
            if (item->tval == TV_STAFF || item->tval == TV_WAND) 
                charge_each = item->pval / item->iqty;

            /* stacking? */
            if (borg_object_similar(item2, item)) {
                /* if this stacks with what was previously here */
                item2->iqty++;
                if (item->tval == TV_STAFF || item->tval == TV_WAND)
                    item2->pval += charge_each;
            } else {
                bool found_match = false;

                /* eliminate items that would stack else where in the list. */
                for (k = 0; k < z_info->store_inven_max; k++) {
                    if (borg_object_similar(
                            &borg_safe_shops[BORG_HOME].ware[k], item)) {
                        found_match = true;
                        break;
                    }
                }
                if (found_match)
                    continue;

                /* replace current item with this item */
                memcpy(item2, item, sizeof(borg_item));

                /* only move one into a non-stack slot */
                item2->iqty = 1;
                if (item->tval == TV_STAFF || item->tval == TV_WAND)
                    item2->pval = charge_each;

            }

            /* remove item from pack */
            item->iqty--;

            if (item->tval == TV_STAFF || item->tval == TV_WAND)
                item->pval -= charge_each;

            /* Note the attempt */
            test_item[n] = i + z_info->store_inven_max;

            /* Test to see if this is a good substitution. */
            /* Examine the home  */
            borg_notice_home(NULL, false);

            /* Evaluate the home  */
            home_power = borg_power_home();

            /* Track best */
            if (home_power > *best_home_power) {
                /* Save the results */
                for (k = 0; k < z_info->store_inven_max; k++)
                    best_item[k] = test_item[k];

#if 0
                /* dump, for debugging */
                borg_note(format("Trying Combo (best home power %ld)",
                    *b_home_power));
                borg_note(format("             (test home power %ld)",
                    best_home_power));
                for (i = 0; i < z_info->store_inven_max; i++)
                    if (borg_shops[BORG_HOME].ware[i].iqty)
                        borg_note(format("store %d %s (qty-%d).", i,
                            borg_shops[BORG_HOME].ware[i].desc,
                            borg_shops[BORG_HOME].ware[i].iqty));
                    else
                        borg_note(format("store %d (empty).", i));

                borg_note(" "); /* add a blank line */
#endif

                /* Use it */
                *best_home_power = home_power;
            }

            /* restore stuff */
            memcpy(
                item2, &borg_safe_shops[BORG_HOME].ware[n], sizeof(borg_item));

            /* put item back into pack */
            item->iqty++;

            /* put the item back in the test array */
            test_item[n] = n;
        }
    }
}

/*
 * Step 1 -- sell "useful" things to the home (for later)
 */
bool borg_think_home_sell_useful(int32_t *best_home_power)
{
    int p, i = -1;

    /* clear out our initial best/test objects */
    memset(test_item, 0, sizeof(z_info->store_inven_max * sizeof(uint8_t)));
    memset(best_item, 0, sizeof(z_info->store_inven_max * sizeof(uint8_t)));

    /* Hack -- the home is full */
    /* and pack is full */
    if (borg_shops[BORG_HOME].ware[z_info->store_inven_max - 1].iqty
        && borg_items[PACK_SLOTS - 1].iqty)
        return false;

    /* clear test arrays (test[i] == i is no change) */
    for (i = 0; i < z_info->store_inven_max; i++) 
        /* clear test arrays (test[i] == i is no change) */
        best_item[i] = test_item[i] = i;

    *best_home_power = -1;

    /* find best combo for home. */
    borg_think_home_sell_best(best_home_power);

    /* restore bonuses and such */
    for (i = 0; i < z_info->store_inven_max; i++) {
        memcpy(&borg_shops[BORG_HOME].ware[i],
            &borg_safe_shops[BORG_HOME].ware[i], sizeof(borg_item));
    }

    for (i = 0; i < INVEN_TOTAL; i++)
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

    borg_notice(true);

    /* Drop stuff that will stack in the home */
    for (i = 0; i < z_info->store_inven_max; i++) {
        /* if this is not the item that was there, */
        /* drop off the item that replaces it. */
        if (best_item[i] != i && best_item[i] != 255) {
            borg_item *item
                = &borg_items[best_item[i] - z_info->store_inven_max];
            borg_item *item2 = &borg_shops[BORG_HOME].ware[i];

            /* if this item is not the same as what was */
            /* there before take it. */
            if (!borg_object_similar(item2, item))
                continue;

            /* There are limted quantity per slot */
            if (item2->iqty > 90)
                continue;

            borg.goal.shop = BORG_HOME;
            borg.goal.item = best_item[i] - z_info->store_inven_max;

            return true;
        }
    }

    /* Get rid of stuff in house but not in 'best' house if  */
    /* pack is not full */
    if (!borg_items[PACK_SLOTS - 1].iqty) {
        for (i = 0; i < z_info->store_inven_max; i++) {
            /* if this is not the item that was there, */
            /* get rid of the item that was there */
            if ((best_item[i] != i) && (borg_shops[BORG_HOME].ware[i].iqty)) {
                borg_item *item
                    = &borg_items[best_item[i] - z_info->store_inven_max];
                borg_item *item2 = &borg_shops[BORG_HOME].ware[i];

                /* if this item is not the same as what was */
                /* there before take it. */
                if (borg_object_similar(item, item2))
                    continue;

                /* skip stuff if we sold bought it */
                /* skip stuff if we sold/bought it */
                for (p = 0; p < sold_item_num; p++) {
                    if (sold_item_tval[p] == item2->tval
                        && sold_item_sval[p] == item2->sval
                        && sold_item_store[p] == BORG_HOME)
                        return false;
                }

                borg.goal.shop = BORG_HOME;
                borg.goal.ware = i;

                return true;
            }
        }
    }

    /* Drop stuff that is in best house but currently in inventory */
    for (i = 0; i < z_info->store_inven_max; i++) {
        /* if this is not the item that was there,  */
        /* drop off the item that replaces it. */
        if (best_item[i] != i && best_item[i] != 255) {
            /* hack don't sell DVE */
            if (!borg_items[best_item[i] - z_info->store_inven_max].iqty)
                return false;

            borg.goal.shop = BORG_HOME;
            borg.goal.item = best_item[i] - z_info->store_inven_max;

            return true;
        }
    }

    /* Return our num_ counts to normal */
    borg_notice_home(NULL, false);

    /* Assume not */
    return false;
}

/* 
 * scan inventory and equipment to see if we have 
 * more than one of this 
 */
static bool borg_has_mutiple(borg_item *in_item)
{
    int i;
    if (in_item->iqty > 1)
        return true;

    /* Some types of items we only want to count as a dupe in a stack */
    /* for example two unIDd swords may look the same (both rapiers) */
    /* but have different powers */
    if (!in_item->ident) {
        switch (in_item->tval) {
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SHOT:
        case TV_BOLT:
        case TV_ARROW:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
           return false;
        }
    }

    /* check the pack */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];
        if (item == in_item)
            continue;
        if (item->tval == in_item->tval 
            && item->sval == in_item->sval
            && item->iqty != 0)
            return true;
    }

    /* check equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
        borg_item *item = &borg_items[i];
        if (item->tval == in_item->tval
            && item->sval == in_item->sval
            && item->iqty != 0)
            return true;
    }
    return false;
}

static bool borg_store_buys(borg_item *item, int who)
{
    switch (who + 1) {
        /* General Store */
    case 1:
        /* Analyze the type */
        switch (item->tval) {
        case TV_FOOD:
        case TV_MUSHROOM:
        case TV_FLASK:
            return true;
        }
        return false;

    /* Armory */
    case 2:

        /* Analyze the type */
        switch (item->tval) {
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
            return true;
        }
        return false;

    /* Weapon Shop */
    case 3:

        /* Analyze the type */
        switch (item->tval) {
        case TV_SHOT:
        case TV_BOLT:
        case TV_ARROW:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
            return true;
        }
        return false;

    /* Bookstore */
    case 4:

        /* Analyze the type */
        switch (item->tval) {
        case TV_PRAYER_BOOK:
        case TV_MAGIC_BOOK:
        case TV_NATURE_BOOK:
        case TV_SHADOW_BOOK:
        case TV_OTHER_BOOK:
            return true;
        }
        return false;

    /* book store --Alchemist */
    case 5:

        /* Analyze the type */
        switch (item->tval) {
        case TV_SCROLL:
        case TV_POTION:
            return true;
        }
        return false;

    /* Magic Shop */
    case 6:

        /* Analyze the type */
        switch (item->tval) {
        case TV_AMULET:
        case TV_RING:
        case TV_SCROLL:
        case TV_POTION:
        case TV_STAFF:
        case TV_WAND:
        case TV_ROD:
        case TV_MAGIC_BOOK:
            return true;
        }
        return false;

    /* Black Market */
    case 7:

        /* Analyze the type */
        switch (item->tval) {
        case TV_LIGHT:
        case TV_CLOAK:
        case TV_FOOD:
            return true;
        }
        return false;
    }
    return false;
}

/*
 * Determine if an item should be sold
 */
static bool borg_good_sell(borg_item *item, int who)
{
    int i;
    /* do I have more than one, even if not stacked */
    /* only used for non-IDd items */
    bool multiple = false;

    /* Never attempt to sell worthless items, shops won't buy them */
    if (item->value <= 0) {
        /* except unidentified potions and scrolls.  Since these can't be IDd,
         * best to sell them */
        if (!((item->tval == TV_POTION || item->tval == TV_SCROLL)
                && !item->ident))
            return false;
    }

    /* make sure we are in the shop that buys this */
    if (!borg_store_buys(item, who))
        return false;

    /* Never sell valuable non-id'd items */
    /* unless you have a stack, in which case, sell one to ID them */
    if (borg_item_note_needs_id(item)) {
        /* note if we have more than one */
        multiple = borg_has_mutiple(item);
        if (!multiple)
            return false;
    }

    /* Worshipping gold or scumming will allow the sale */
    if (item->value > 0
        && ((borg_cfg[BORG_WORSHIPS_GOLD] || borg.trait[BI_MAXCLEVEL] < 10)
            || ((borg_cfg[BORG_MONEY_SCUM_AMOUNT] < borg.trait[BI_GOLD])
                && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0))) {
        /* Borg is allowed to continue in this routine to sell */
    } else /* Some items must be ID, or at least 'known' */
    {
        /* Analyze the type */
        switch (item->tval) {
        case TV_POTION:
        case TV_SCROLL:

            /* Always sell potions and scrolls, it is the only way to ID other
             * than using */
            if (!item->ident)
                return true;

            /* Spell casters should not sell ResMana to shop unless
             * they have tons in the house
             */
            if (item->tval == TV_POTION && item->sval == sv_potion_restore_mana
                && borg.trait[BI_MAXSP] > 100
                && borg.has[kv_potion_restore_mana] + num_mana > 99)
                return false;

            break;

        case TV_FOOD:
        case TV_ROD:
        case TV_WAND:
        case TV_STAFF:
        case TV_RING:
        case TV_AMULET:
        case TV_LIGHT:

            /* Never sell if not "known" */
            /* unless we have more than one or are deep */
            if (borg_item_worth_id(item) && (borg.trait[BI_MAXDEPTH] < 35)
                && !multiple)
                return false;

            break;

        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:

            /* Only sell "known" items (unless we have more than one) */
            if (borg_item_worth_id(item) && !multiple)
                return false;

            break;
        }
    }

    /* Do not sell stuff that is not fully id'd and should be  */
    if (OPT(player, birth_randarts) && item->art_idx && !item->ident) {
        /* CHECK THE ARTIFACTS */
        /* For now check all artifacts */
        return false;
    }

    /* Do not sell stuff that is not fully id'd and should be  */
    if (!item->ident && item->ego_idx && item->iqty < 2) {
        if (borg_ego_has_random_power(
                &e_info[borg_items[INVEN_OUTER].ego_idx])) {
            return false;
        }
    }

    /* Do not sell it if I just bought one */
    /* do not buy the item if I just sold it. */
    for (i = 0; i < bought_item_num; i++) {
        if (bought_item_tval[i] == item->tval
            && bought_item_sval[i] == item->sval
            && (bought_item_store[i] == who || who != BORG_HOME)) {
#if 0
            borg_note(format("# Choosing not to sell back %s", item->desc));
#endif
            return false;
        }
    }

    return true;
}

/*
 * Step 2 -- sell "useless" items to a shop (for cash)
 */
bool borg_think_shop_sell_useless(void)
{
    int icky = z_info->store_inven_max - 1;

    int     k, b_k = -1;
    int     i, b_i = -1;
    int     qty = 1;

    int32_t p, b_p = 0L;
    int32_t c   = 0L;
    int32_t b_c = 30001L;

    bool fix    = false;

    /* Evaluate */
    b_p = borg.power;

    /* Check each shop */
    for (k = 0; k < (z_info->store_max - 1); k++) {
        /* Hack -- Skip "full" shops */
        if (borg_shops[k].ware[icky].iqty)
            continue;

        /* Sell stuff */
        for (i = 0; i < z_info->pack_size; i++) {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty)
                continue;

            /* Skip some important type items */
            if ((item->tval == borg.trait[BI_AMMO_TVAL])
                && (borg.trait[BI_AMISSILES] < 45))
                continue;
            if (item->tval == TV_ROD && item->sval == sv_rod_healing
                && borg.has[kv_rod_healing] <= 3)
                continue;

            if (borg.trait[BI_CLASS] == CLASS_WARRIOR && item->tval == TV_ROD
                && item->sval == sv_rod_mapping && item->iqty <= 2)
                continue;

            /* Do not sell our attack wands if they still have charges */
            if (item->tval == TV_WAND && borg.trait[BI_CLEVEL] < 35
                && (item->sval == sv_wand_magic_missile
                    || item->sval == sv_wand_stinking_cloud
                    || item->sval == sv_wand_annihilation)
                && item->pval != 0)
                continue;

            /* don't sell our swap items */
            if (weapon_swap && i == weapon_swap - 1)
                continue;
            if (armour_swap && i == armour_swap - 1)
                continue;

            /* Skip "bad" sales */
            if (!borg_good_sell(item, k))
                continue;

            /* Give the item to the shop */
            memcpy(
                &borg_shops[k].ware[icky], &safe_items[i], sizeof(borg_item));

            /* get the quantity */
            qty = borg_min_item_quantity(item);

            /* Give "qty" items */
            borg_shops[k].ware[icky].iqty = qty;

            /* Lose a single item */
            borg_items[i].iqty -= qty;

            /* for wands and staffs adjust charges */
            if (item->tval == TV_STAFF || item->tval == TV_WAND) {
                int charge_loss = safe_items[i].pval/safe_items[i].iqty;
                borg_shops[k].ware[icky].pval = qty * charge_loss;
                borg_items[i].pval -= (qty * charge_loss);
            }

            /* Fix later */
            fix = true;

            /* Examine the inventory */
            borg_notice(true);

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

            /* Restore the store item */
            memcpy(&borg_shops[k].ware[icky], &borg_safe_shops[k].ware[icky],
                sizeof(borg_item));

            /* Ignore "bad" sales */
            if (p < b_p)
                continue;

            /* Extract the "price" */
            c = ((item->value < 30000L) ? item->value : 30000L);

            /* sell cheap items first.  This is done because we may have to */
            /* buy the item back in some very strange circumstances. */
            if ((p == b_p) && (c >= b_c))
                continue;

            /* Maintain the "best" */
            b_k = k;
            b_i = i;
            b_p = p;
            b_c = c;
        }

    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Sell something (if useless) */
    if ((b_k >= 0) && (b_i >= 0)) {
        /* Visit that shop */
        borg.goal.shop = b_k;

        /* Sell that item */
        borg.goal.item = b_i;

        /* Success */
        return true;
    }

    /* Assume not */
    return false;
}

/*
 * Sell items to the current shop, if desired
 */
bool borg_think_shop_sell(void)
{
    int qty = 1;

    /* Sell something if requested */
    if ((borg.goal.shop == shop_num) && (borg.goal.item >= 0)) {
        borg_item *item = &borg_items[borg.goal.item];

        qty             = borg_min_item_quantity(item);

        /* Log */
        borg_note(format("# Selling %s", item->desc));

        /* Sell an item */
        borg_keypress('s');

        /* Sell the desired item */
        borg_keypress(all_letters_nohjkl[borg.goal.item]);

        /* Hack -- Sell a single item */
        if (item->iqty > 1 || qty >= 2) {
            if (qty == 5)
                borg_keypress('5');
            if (qty == 4)
                borg_keypress('4');
            if (qty == 3)
                borg_keypress('3');
            if (qty == 2)
                borg_keypress('2');
            borg_keypress(KC_ENTER);
        }

        /* Mega-Hack -- Accept the price */
        if (borg.goal.shop != BORG_HOME) {
            borg_keypress(KC_ENTER);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
        }

        /* Mark our last item sold */
        if (sold_item_nxt >= 9)
            sold_item_nxt = 0;
        sold_item_pval[sold_item_nxt]  = item->pval;
        sold_item_tval[sold_item_nxt]  = item->tval;
        sold_item_sval[sold_item_nxt]  = item->sval;
        sold_item_store[sold_item_nxt] = borg.goal.shop;
        sold_item_num                  = sold_item_nxt;
        sold_item_nxt++;

        /* The purchase is complete */
        borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

        /* tick the anti-loop clock */
        borg.time_this_panel++;

        /* I'm not in a store */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg.in_shop  = false;
        borg_do_inven = true;
        /* Success */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Count the number of items worth "selling"
 *
 * This determines the choice of stairs.
 *
 */
int borg_count_sell(void)
{
    int i, k = 0;

    int32_t price;
    int32_t greed;
    int     p, sv_qty;

    /* Calculate "greed" factor */
    greed = (borg.trait[BI_GOLD] / 100L) + 100L;

    /* Minimal greed */
    if (greed < 1000L)
        greed = 1000L;
    if (greed > 25000L)
        greed = 25000L;
    if (borg.trait[BI_MAXDEPTH] >= 50)
        greed = 75000;
    if (borg.trait[BI_CLEVEL] < 25)
        greed = (borg.trait[BI_GOLD] / 100L) + 50L;
    if (borg.trait[BI_CLEVEL] < 20)
        greed = (borg.trait[BI_GOLD] / 100L) + 35L;
    if (borg.trait[BI_CLEVEL] < 15)
        greed = (borg.trait[BI_GOLD] / 100L) + 20L;
    if (borg.trait[BI_CLEVEL] < 13)
        greed = (borg.trait[BI_GOLD] / 100L) + 10L;
    if (borg.trait[BI_CLEVEL] < 10)
        greed = (borg.trait[BI_GOLD] / 100L) + 5L;
    if (borg.trait[BI_CLEVEL] < 5)
        greed = (borg.trait[BI_GOLD] / 100L);

    /* Count "sellable" items */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip "crappy" items */
        if (item->value <= 0)
            continue;

        /* skip our swap weapon */
        if (weapon_swap && i == weapon_swap - 1)
            continue;
        if (armour_swap && i == armour_swap - 1)
            continue;

        /* Don't sell my ammo */
        if (item->tval == borg.trait[BI_AMMO_TVAL])
            continue;

        /* Don't sell my books */
        if (obj_kind_can_browse(&k_info[item->kind]))
            continue;

        /* Never sell valuable non-id'd items */
        /* unless you have a stack, in which case, sell one to ID them */
        if (borg_item_note_needs_id(item)) {
            /* note if we have more than one */
            if (!borg_has_mutiple(item))
                return false;
        }

        /* Don't sell my needed potion/wands/staff/scroll collection */
        if ((item->tval == TV_POTION && item->sval == sv_potion_cure_serious)
            || (item->tval == TV_POTION
                && item->sval == sv_potion_cure_critical)
            || (item->tval == TV_POTION && item->sval == sv_potion_healing)
            || (item->tval == TV_POTION && item->sval == sv_potion_star_healing)
            || (item->tval == TV_POTION && item->sval == sv_potion_life)
            || (item->tval == TV_POTION && item->sval == sv_potion_speed)
            || (item->tval == TV_STAFF && item->sval == sv_staff_teleportation)
            || (item->tval == TV_WAND && item->sval == sv_wand_drain_life)
            || (item->tval == TV_WAND && item->sval == sv_wand_annihilation)
            || (item->tval == TV_SCROLL && item->sval == sv_scroll_teleport))
            continue;
        /* Obtain the base price */
        price = ((item->value < 30000L) ? item->value : 30000L);

        /* Skip cheap "known" (or "average") items */
        if ((price * item->iqty < greed) && !borg_item_note_needs_id(item))
            continue;

        /* only mark things as sellable if getting rid of them doesn't reduce
         * our power much */
        sv_qty     = item->iqty;
        item->iqty = 0;
        borg_notice(true);
        p          = borg_power();
        item->iqty = sv_qty;
        ;
        if (p + 50 < borg.power)
            continue;

        /* Count remaining items */
        k++;
    }
    /* reset the results */
    borg_notice(true);

    /* Result */
    return (k);
}

void borg_init_store_sell(void)
{
    test_item = mem_zalloc(z_info->store_inven_max * sizeof(uint8_t));
    best_item = mem_zalloc(z_info->store_inven_max * sizeof(uint8_t));
}

void borg_free_store_sell(void)
{
    mem_free(best_item);
    best_item = NULL;
    mem_free(test_item);
    test_item = NULL;
}

#endif
