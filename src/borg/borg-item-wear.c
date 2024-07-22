/**
 * \file borg-item-wear.c
 * \brief Wear items
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

#include "borg-item-wear.h"

#ifdef ALLOW_BORG

#include "../ui-menu.h"

#include "borg-danger.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-analyze.h"
#include "borg-item-id.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-power.h"
#include "borg-store-sell.h"
#include "borg-store.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Track the items worn to avoid loops
 */
int16_t  track_worn_num;
int16_t  track_worn_size;
uint8_t *track_worn_name1;
int16_t  track_worn_time;

/* Item to be worn.  Index used to note which item not to sell */
int16_t borg_best_fit_item = -1; 

/*
 * Identify items if possible
 *
 * Note that "borg_parse()" will "cancel" the identification if it
 * detects a "You failed..." message.  This is VERY important!!!
 * Otherwise the "identify" might induce bizarre actions by sending
 * the "index" of an item as a command.
 *
 * Hack -- recover from mind blanking by re-identifying the equipment.
 *
 * We instantly identify items known to be "good" (or "terrible").
 *
 * We identify most un-aware items as soon as possible.
 *
 * We identify most un-known items as soon as possible.
 *
 * We play games with items that get "feelings" to try and wait for
 * "sensing" to take place if possible.
 *
 * XXX XXX XXX Make sure not to sell "non-aware" items, unless
 * we are really sure we want to lose them.  For example, we should
 * wait for feelings on (non-icky) wearable items or else make sure
 * that we identify them before we try and sell them.
 *
 * Mega-Hack -- the whole "sometimes identify things" code is a total
 * hack.  Slightly less bizarre would be some form of "occasionally,
 * pick a random item and identify it if necessary", which might lower
 * the preference for identifying items that appear early in the pack.
 * Also, preventing inventory motion would allow proper time-stamping.
 *
 * This function is also repeated to *ID* objects.  Right now only objects
 * with random high resist or random powers are *ID*'d
 */
bool borg_test_stuff(void)
{
    int  i;
    int  b_i = -1, b_v = -1;
    bool inv_item_needs_id = false;
    bool free_id           = borg_spell_legal(IDENTIFY_RUNE);

    /* don't ID stuff when you can't recover spent spell point immediately */
    if (borg.trait[BI_CURSP] < 50 && borg_spell_legal(IDENTIFY_RUNE)
        && !borg_check_rest(borg.c.y, borg.c.x))
        return false;

    /* No ID if in danger */
    if (borg_danger(borg.c.y, borg.c.x, 1, true, false) > 1)
        return false;

    /* Look for an item to identify (equipment) */
    for (i = INVEN_WIELD; i < QUIVER_END; i++) {
        int        v    = 0;
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;
        if (!item->needs_ident)
            continue;

        /* Preferentially ID egos and artifacts */
        if (item->art_idx)
            v = item->value + 150000L;

        if (borg_ego_has_random_power(&e_info[item->ego_idx])) {
            v = item->value + 100000L;
        }

        /* Prioritize the ID */
        if (borg_item_note_needs_id(item))
            v = item->value + 20000L;

        /* Ignore */
        if (!v)
            continue;

        /* Track the best */
        if (v <= b_v)
            continue;

        /* Track it */
        b_i = i;
        b_v = v;
    }

    /* Look for an item to identify  */
    for (i = 0; i < QUIVER_END; i++) {
        int        v    = 0;
        borg_item *item = &borg_items[i];

        /* Skip empty and ID'd items */
        if (!item->iqty)
            continue;
        if (!item->needs_ident)
            continue;

        if (i < z_info->pack_size)
            inv_item_needs_id = true;

        /* Preferentially ID artifacts */
        if (item->art_idx)
            v = item->value + 150000L;

        /* Identify "good" items */
        if (borg_item_note_needs_id(item))
            v = item->value + 20000L;
        else if (free_id || borg_item_worth_id(item))
            v = item->value;

        /* Hack -- reward "unaware" items */
        if (!item->kind) {
            /* Analyze the type */
            switch (item->tval) {
            case TV_RING:
            case TV_AMULET:
                v += (borg.trait[BI_MAXDEPTH] * 5000L);
                break;

            case TV_ROD:
                v += (borg.trait[BI_MAXDEPTH] * 3000L);
                break;

            case TV_WAND:
            case TV_STAFF:
                v += (borg.trait[BI_MAXDEPTH] * 2000L);
                break;

            case TV_POTION:
            case TV_SCROLL:
                /* Hack -- boring levels */
                if (borg.trait[BI_MAXDEPTH] < 5)
                    break;

                /* Hack -- reward depth */
                v += (borg.trait[BI_MAXDEPTH] * 500L);
                break;

            case TV_FOOD:
                v += (borg.trait[BI_MAXDEPTH] * 10L);
                break;
            }
        }

        /* Ignore */
        if (!v)
            continue;

        /* Track the best */
        if (v <= b_v)
            continue;

        /* Track it */
        b_i = i;
        b_v = v;
    }

    /* Found something */
    if (b_i >= 0) {
        borg_item *item = &borg_items[b_i];

        /* Use an item to identify */
        if (borg_spell(IDENTIFY_RUNE) || borg_read_scroll(sv_scroll_identify)) {
            /* Log -- may be cancelled */
            borg_note(format("# Identifying %s.", item->desc));

            /* Equipment */
            if (b_i >= INVEN_WIELD && b_i < QUIVER_START) {
                if (inv_item_needs_id)
                    borg_keypress('/');

                /* Select the item */
                borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);

                /* HACK need to recheck stats if we id something on us. */
                for (i = 0; i < STAT_MAX; i++) {
                    //                    my_need_stat_check[i] = true;
                    borg.stat_max[i] = 0;
                }
            } else if (b_i >= QUIVER_START) {
                /* Select quiver */
                borg_keypress('|');

                /* Select the item */
                borg_keypress(I2D(b_i - QUIVER_START));
            }
            /* Inventory */
            else {
                /* Select the item */
                borg_keypress(all_letters_nohjkl[b_i]);
            }

            borg_keypress(ESCAPE);

            return true;
        }
    }

    /* Nothing to do */
    return false;
}

/*
 * This function is responsible for making sure that, if possible,
 * the "best" ring we have is always on the "right" (tight) finger,
 * so that the other functions, such as "borg_best_stuff()", do not
 * have to think about the "tight" ring, but instead, can just assume
 * that the "right" ring is "good for us" and should never be removed.
 *
 * In general, this will mean that our "best" ring of speed will end
 * up on the "right" finger, if we have one, or a ring of free action,
 * or a ring of see invisible, or some other "useful" ring.
 *
 */
bool borg_swap_rings(void)
{
    int hole = borg_first_empty_inventory_slot();

    int32_t v1, v2;

    /*** Check conditions ***/

    /* Require two empty slots */
    if (hole == -1)
        return false;

    if ((hole + 1) >= PACK_SLOTS)
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 1000)
        return false;
    if (borg.trait[BI_CDEPTH] != 0)
        return false;

    /*** Remove naked "loose" rings ***/

    /* Remove any naked loose ring */
    if (borg_items[INVEN_LEFT].iqty && !borg_items[INVEN_RIGHT].iqty
        && !borg_items[INVEN_LEFT].one_ring) {
        /* Log */
        borg_note("# Taking off naked loose ring.");

        /* Remove it */
        borg_keypress('t');
        borg_keypress(all_letters_nohjkl[INVEN_LEFT - INVEN_WIELD]);

        /* Success */
        return true;
    }

    /*** Check conditions ***/

    /* Require "tight" ring */
    if (!borg_items[INVEN_RIGHT].iqty)
        return false;

    /* Cannot remove the One Ring */
    if (borg_items[INVEN_RIGHT].one_ring)
        return false;

    /*** Remove nasty "tight" rings ***/

    /* Save the hole */
    memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

    /* Save the ring */
    memcpy(&safe_items[INVEN_LEFT], &borg_items[INVEN_LEFT], sizeof(borg_item));

    /* Take off the ring */
    memcpy(&borg_items[hole], &borg_items[INVEN_LEFT], sizeof(borg_item));

    /* Erase left ring */
    memset(&borg_items[INVEN_LEFT], 0, sizeof(borg_item));

    /* Examine the inventory */
    borg_notice(false);

    /* Evaluate the inventory */
    v1 = borg_power();

    /* Restore the ring */
    memcpy(&borg_items[INVEN_LEFT], &safe_items[INVEN_LEFT], sizeof(borg_item));

    /* Restore the hole */
    memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

    /*** Consider taking off the "right" ring ***/

    /* Save the hole */
    memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

    /* Save the ring */
    memcpy(
        &safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT], sizeof(borg_item));

    /* Take off the ring */
    memcpy(&borg_items[hole], &borg_items[INVEN_RIGHT], sizeof(borg_item));

    /* Erase the ring */
    memset(&borg_items[INVEN_RIGHT], 0, sizeof(borg_item));

    /* Examine the inventory */
    borg_notice(false);

    /* Evaluate the inventory */
    v2 = borg_power();

    /* Restore the ring */
    memcpy(
        &borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], sizeof(borg_item));

    /* Restore the hole */
    memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

    /*** Swap rings if necessary ***/

    /* Remove "useless" ring */
    if (v2 > v1) {
        /* Log */
        borg_note("# Taking off less valuable right ring.");

        /* Take it off */
        if (borg_items[INVEN_RIGHT].iqty) {
            borg_keypress('t');
            borg_keypress(all_letters_nohjkl[INVEN_RIGHT - INVEN_WIELD]);
        }

        /* make sure one is on the left */
        if (borg_items[INVEN_LEFT].iqty) {
            borg_note("# Taking off more valuable left ring.");
            borg_keypress('t');
            borg_keypress(all_letters_nohjkl[INVEN_LEFT - INVEN_WIELD]);
        }

        /* Success */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Place our "best" ring on the "tight" finger if needed
 *
 * This function is adopted from "borg_wear_stuff()"
 *
 * Basically, we evaluate the world in which each ring is added
 * to the current set of equipment, and we wear the ring, if any,
 * that gives us the most "power".
 *
 * The "borg_swap_rings()" code above occasionally allows us to remove
 * both rings, at which point this function will place the "best" ring
 * on the "tight" finger, and then the "borg_best_stuff()" function will
 * allow us to put on our second "best" ring on the "loose" finger.
 *
 * This function should only be used when a ring finger is empty.
 */
bool borg_wear_rings(void)
{
    int hole = borg_first_empty_inventory_slot();

    int32_t p, b_p = 0L;

    int i, b_i     = -1;

    borg_item *item;

    bool fix = false;

    if (hole == -1)
        return false;

    /* Require no rings */
    if (borg_items[INVEN_LEFT].iqty)
        return false;
    if (borg_items[INVEN_RIGHT].iqty)
        return false;

    /* Require two empty slots */
    if (hole + 1 >= PACK_SLOTS)
        return false;
    if (borg_items[hole + 1].iqty)
        return false;

    /* hack prevent the swap till you drop loop */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000)
        return false;

    /* Scan inventory */
    for (i = 0; i < z_info->pack_size; i++) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require "aware" */
        if (!item->kind)
            continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value)
            continue;

        /* skip artifact rings not star id'd  */
        if (OPT(player, birth_randarts) && !item->ident && item->art_idx)
            continue;

        /* Only process "rings" */
        if (item->tval != TV_RING)
            continue;

        /* Wear new item */
        memcpy(&borg_items[INVEN_LEFT], item, sizeof(borg_item));

        /* Only a single item */
        borg_items[INVEN_LEFT].iqty = 1;

        /* Reduce the inventory quantity by one */
        item->iqty--;

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* the One Ring would be awesome */
        if (item->one_ring)
            p = borg.power * 2;

        /* Restore the old item (empty) */
        borg_items[INVEN_LEFT].iqty = 0;

        /* Restore the item in inventory */
        item->iqty++;

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p))
            continue;

        /* Maintain the "best" */
        b_i = i;
        b_p = p;
    }

    /* Restore bonuses */
    if (fix)
        borg_notice(true);

    /* No item */
    if ((b_i >= 0) && (b_p > borg.power)) {
        /* Get the item */
        item = &borg_items[b_i];

        /* Log */
        borg_note("# Putting on best tight ring.");

        /* Log */
        borg_note(format("# Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[b_i]);

        /* Did something */
        borg.time_this_panel++;
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Place our "swap" if needed.   We check both the armour and the weapon
 * then wear the one that give the best result (lowest danger).
 * This function is adopted from "borg_wear_stuff()" and borg_wear_rings
 *
 * Basically, we evaluate the world in which the swap is added
 * to the current set of equipment, and we use weapon,
 * that gives the largest drop in danger---based mostly on resists.
 *
 * The borg is forbidden to swap out certain resistances.
 *
 */
bool borg_backup_swap(int p)
{
    int slot;
    int swap;

    int32_t b_p  = 0L;
    int32_t b_p1 = 0L;
    int32_t b_p2 = 0L;

    int i;

    int save_rconf  = 0;
    int save_rblind = 0;
    int save_fract  = 0;

    borg_item *item;

    bool fix = false;

    /* hack prevent the swap till you drop loop */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg.time_this_panel > 300)
        return false;

    /* make sure we have an appropriate swap */
    if (!armour_swap && !weapon_swap)
        return false;

    if (armour_swap) {
        /* Save our normal condition */
        save_rconf  = borg.trait[BI_RCONF];
        save_rblind = borg.trait[BI_RBLIND];
        save_fract  = borg.trait[BI_FRACT];

        /* Check the items, first armour then weapon */
        i = armour_swap - 1;

        /* make sure it is not a -1 */
        if (i == -1)
            i = 0;

        /* get the item */
        item = &borg_items[i];

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* safety check incase slot = -1 */
        if (slot < 0)
            return false;

        /* Save the old item (empty) */
        memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

        /* Save the new item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Wear new item */
        memcpy(&borg_items[slot], &safe_items[i], sizeof(borg_item));

        /* Only a single item */
        borg_items[slot].iqty = 1;

        /* Reduce the inventory quantity by one */
        borg_items[i].iqty--;

        /* Fix later */
        fix = true;

        /* Examine the benefits of the swap item */
        borg_notice(false);

        /* Evaluate the power with the new item worn */
        b_p1 = borg_danger(borg.c.y, borg.c.x, 1, true, false);

        /* Restore the old item (empty) */
        memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

        /* Restore the new item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Examine the critical skills */
        if ((save_rconf) && borg.trait[BI_RCONF] == 0)
            b_p1 = 9999;
        if ((save_rblind)
            && (!borg.trait[BI_RBLIND] && !borg.trait[BI_RLITE]
                && !borg.trait[BI_RDARK] && borg.trait[BI_SAV] < 100))
            b_p1 = 9999;
        if ((save_fract) && (!borg.trait[BI_FRACT] && borg.trait[BI_SAV] < 100))
            b_p1 = 9999;

        /* Restore bonuses */
        if (fix)
            borg_notice(true);

        /*  skip random artifact not star id'd  */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
            b_p1 = 9999;

        /* skip it if it has not been decursed */
        if (item->cursed && !item->uncursable)
            b_p1 = 9999;
    }

    /* Now we check the weapon */
    if (weapon_swap) {
        /* get the item */
        i = weapon_swap - 1;

        /* make sure it is not a -1 */
        if (i == -1)
            i = 0;

        item = &borg_items[i];

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* safety check incase slot = -1 */
        if (slot < 0)
            return false;

        /* Save the old item (empty) */
        memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

        /* Save the new item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Wear new item */
        memcpy(&borg_items[slot], &safe_items[i], sizeof(borg_item));

        /* Only a single item */
        borg_items[slot].iqty = 1;

        /* Reduce the inventory quantity by one */
        borg_items[i].iqty--;

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the power with the new item worn */
        b_p2 = borg_danger(borg.c.y, borg.c.x, 1, true, false);

        /* Examine the critical skills */
        /* Examine the critical skills */
        if ((save_rconf) && borg.trait[BI_RCONF] == 0)
            b_p2 = 9999;
        if ((save_rblind)
            && (!borg.trait[BI_RBLIND] && !borg.trait[BI_RLITE]
                && !borg.trait[BI_RDARK] && borg.trait[BI_SAV] < 100))
            b_p2 = 9999;
        if ((save_fract) && (!borg.trait[BI_FRACT] && borg.trait[BI_SAV] < 100))
            b_p2 = 9999;

        /* Restore the old item (empty) */
        memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

        /* Restore the new item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Restore bonuses */
        if (fix)
            borg_notice(true);

        /*  skip random artifact not star id'd  */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
            b_p2 = 9999;

        /* skip it if it has not been decursed */
        if (item->cursed && !item->uncursable)
            b_p2 = 9999;
    }

    /* Pass on the swap which yields the best result */
    if (b_p1 <= b_p2) {
        b_p  = b_p2;
        swap = weapon_swap - 1;
    } else {
        b_p  = b_p1;
        swap = armour_swap - 1;
    }

    /* good swap.  Make sure it helps a significant amount */
    if (p > b_p
        && b_p <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                        : (avoidance / 2))) {
        /* Log */
        borg_note(format("# Swapping backup.  (%d < %d).", b_p, p));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[swap]);

        /* Did something */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Wear useful equipment.
 *
 * Look through the inventory for equipment that is better than
 * the current equipment, and wear it, in an optimal order.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are used instead of the items they would replace, and we take
 * one step towards the world in which we have the most "power".
 *
 * The "borg_swap_rings()" code above occasionally allows us to remove
 * both rings, at which point this function will replace the "best" ring
 * on the "tight" finger, and the second "best" ring on the "loose" finger.
 */
bool borg_wear_stuff(void)
{
    int hole = 0;

    int  slot;
    int  d;
    int  o;
    bool recently_worn = false;

    int32_t p, b_p = 0L;

    int i, b_i     = -1;
    int ii, b_ii   = -1;
    int danger;

    char target_ring_desc[80];

    borg_item *item;

    bool fix = false;

    /* Start with current power */
    b_p = borg.power;

    /*  hack to prevent the swap till you drop loop */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        return false;

    /* We need an empty slot to simulate pushing equipment */
    hole = borg_first_empty_inventory_slot();
    if (hole == -1)
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000)
        return false;
    if (borg.time_this_panel > 1300)
        return false;

    /* Scan inventory */
    for (i = 0; i < z_info->pack_size; i++) {
        item = &borg_items[i];

        /* reset this item marker */
        recently_worn = false;

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require "aware" */
        if (!item->kind)
            continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value)
            continue;

        /* do not wear not *idd* artifacts */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
            continue;

        /* Do not consider wearing this item if I worn it already this level,
         * I might be stuck in a loop.
         */
        for (o = 0; o < track_worn_num; o++) {
            /* Examine the worn list */
            if (track_worn_num >= 1 && track_worn_name1[o] == item->art_idx
                && track_worn_time > borg_t - 10) {
                /* Recently worn item */
                recently_worn = true;
            }
        }

        /* Note and fail out */
        if (recently_worn == true) {
            borg_note(format(
                "# Not considering a %s; it was recently worn.", item->desc));
            continue;
        }

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* Cannot wear this item */
        if (slot < 0)
            continue;

        /* Do not wear certain items if I am over weight limit.  It induces
         * loops */
        if (borg.trait[BI_ISENCUMB]) {
            /* Compare Str bonuses */
            if (borg_items[slot].modifiers[OBJ_MOD_STR]
                > item->modifiers[OBJ_MOD_STR])
                continue;
        }

        /* Obtain danger */
        danger = borg_danger(borg.c.y, borg.c.x, 1, true, false);

        /* If this is a ring and both hands are full, then check each hand
         * and compare the two.  If needed the tight ring can be removed then
         * the better ring placed there on.
         */

        /*** Process regular items and non full rings ***/

        /* Non ring, non full hands */
        if (slot != INVEN_LEFT
            || (!borg_items[INVEN_LEFT].iqty
                || !borg_items[INVEN_RIGHT].iqty)) {
            /* Save the old item */
            memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

            /* Save the new item */
            memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

            /* Save the hole */
            memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

            /* Take off old item */
            memcpy(&borg_items[hole], &safe_items[slot], sizeof(borg_item));

            /* Wear new item */
            memcpy(&borg_items[slot], &safe_items[i], sizeof(borg_item));

            /* Only a single item */
            borg_items[slot].iqty = 1;

            /* Reduce the inventory quantity by one */
            borg_items[i].iqty--;

            /* Fix later */
            fix = true;

            /* Examine the inventory */
            borg_notice(false);

            /* Evaluate the inventory */
            p = borg_power();

            /* Evaluate local danger */
            d = borg_danger(borg.c.y, borg.c.x, 1, true, false);

#if 0
            if (borg_cfg[BORG_VERBOSE]) {
                /* dump list and power...  for debugging */
                borg_note(format("Trying  Item %s (best power %ld)",
                    borg_items[slot].desc, (long int)p));
                borg_note(format("Against Item %s (borg_power %ld)",
                    safe_items[slot].desc, (long int)b_p));
            }
#endif

            /* Restore the old item */
            memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

            /* Restore the new item */
            memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

            /* Restore the hole */
            memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

            /* Need to be careful not to put the One Ring onto
             * the Left Hand
             */
            if (item->one_ring && !borg_items[INVEN_LEFT].tval)
                p = -99999;

            /* Ignore if more dangerous */
            if (danger < d)
                continue;

            /* XXX XXX XXX Consider if slot is empty */

            /* Hack -- Ignore "essentially equal" swaps */
            if (p <= b_p + 50)
                continue;

            /* Maintain the "best" */
            b_i = i;
            b_p = p;
        } /* non-rings, non full */

        if (randint0(100) == 10 || item->one_ring) {
            /* ring, full hands */
            if (slot == INVEN_LEFT && borg_items[INVEN_LEFT].iqty
                && borg_items[INVEN_RIGHT].iqty) {
                for (ii = INVEN_RIGHT; ii <= INVEN_LEFT; ii++) {
                    slot = ii;

                    /* Does One Ring need to be handled here? */

                    /* Save the old item */
                    memcpy(&safe_items[slot], &borg_items[slot],
                        sizeof(borg_item));

                    /* Save the new item */
                    memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

                    /* Save the hole */
                    memcpy(&safe_items[hole], &borg_items[hole],
                        sizeof(borg_item));

                    /* Take off old item */
                    memcpy(&borg_items[hole], &safe_items[slot],
                        sizeof(borg_item));

                    /* Wear new item */
                    memcpy(
                        &borg_items[slot], &safe_items[i], sizeof(borg_item));

                    /* Only a single item */
                    borg_items[slot].iqty = 1;

                    /* Reduce the inventory quantity by one */
                    borg_items[i].iqty--;

                    /* Fix later */
                    fix = true;

                    /* Examine the inventory */
                    borg_notice(false);

                    /* Evaluate the inventory */
                    p = borg_power();

                    /* Evaluate local danger */
                    d = borg_danger(borg.c.y, borg.c.x, 1, true, false);

                    /* Restore the old item */
                    memcpy(&borg_items[slot], &safe_items[slot],
                        sizeof(borg_item));

                    /* Restore the new item */
                    memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

                    /* Restore the hole */
                    memcpy(&borg_items[hole], &safe_items[hole],
                        sizeof(borg_item));

                    /* Need to be careful not to put the One Ring onto
                     * the Left Hand
                     */
                    if (ii == INVEN_LEFT && item->one_ring)
                        p = -99999;

                    /* Ignore "bad" swaps */
                    if (p < b_p)
                        continue;

                    /* no swapping into more danger */
                    if (danger <= d && danger != 0)
                        continue;

                    /* Maintain the "best" */
                    b_i  = i;
                    b_p  = p;
                    b_ii = ii;
                }
            } /* ring, looking at replacing each ring */
        } /* Random ring check */

    } /* end scanning inventory */

    /* Restore bonuses */
    if (fix)
        borg_notice(true);

    /* item */
    if ((b_i >= 0) && (b_p > borg.power)) {
        /* Get the item */
        item = &borg_items[b_i];

        /* Define the desc of the nice ring */
        my_strcpy(target_ring_desc, item->desc, sizeof(target_ring_desc));

        /* Remove old ring to make room for good one */
        if (b_ii >= INVEN_RIGHT && item->tval == TV_RING) {
            /* Log */
            borg_note(format("# Removing %s to make room for %s.",
                borg_items[b_ii].desc, item->desc));

            /* Make room */
            borg_keypress('t');
            borg_keypress(all_letters_nohjkl[b_ii - INVEN_WIELD]);

            /* Did something */
            borg.time_this_panel++;
            return true;
        }

        /* Log */
        borg_note(format("# Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[b_i]);
        borg.time_this_panel++;

        /* Track the newly worn artifact item to avoid loops */
        if (item->art_idx && (track_worn_num < track_worn_size)) {
            borg_note("# Noting the wearing of artifact.");
            track_worn_name1[track_worn_num] = item->art_idx;
            track_worn_time                  = borg_t;
            track_worn_num++;
        }
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Hack -- order of the slots
 *
 * XXX XXX XXX Note that we ignore the "tight" ring, and we
 * assume that we will always be wearing our "best" ring on
 * our "right" (tight) finger, and if we are not, then the
 * "borg_swap_rings()" function will remove both the rings,
 * which will induce the "borg_best_stuff()" function to put
 * the rings back on in the "optimal" order.
 */
static uint16_t borg_best_stuff_order(int n)
{
    switch (n) {
    case 0:
        return INVEN_BOW;
    case 1:
        return INVEN_WIELD;
    case 2:
        return INVEN_BODY;
    case 3:
        return INVEN_OUTER;
    case 4:
        return INVEN_ARM;
    case 5:
        return INVEN_HEAD;
    case 6:
        return INVEN_HANDS;
    case 7:
        return INVEN_FEET;
    case 8:
        return INVEN_LEFT;
    case 9:
        return INVEN_LIGHT;
    case 10:
        return INVEN_NECK;
    default:
        return 255;
    }
}

/*
 * Helper function (see below)
 */
static void borg_best_stuff_aux(
    int n, uint8_t *test, uint8_t *best, int32_t *vp)
{
    int i;

    int slot;

    /* Extract the slot */
    slot = borg_best_stuff_order(n);

    /* All done */
    if (slot == 255) {
        int32_t p;

        /* Examine */
        borg_notice(false);

        /* Evaluate */
        p = borg_power();

        /* Track best */
        if (p > *vp) {
            if (borg_cfg[BORG_VERBOSE]) {
                /* dump list and power...  for debugging */
                borg_note(
                    format("Trying Combo (best power %ld)", (long int)*vp));
                borg_note(format("             (borg_power %ld)", (long int)p));
                for (i = 0; i < z_info->pack_size; i++)
                    borg_note(format("inv %d %s.", i, borg_items[i].desc));
                for (i = 0; borg_best_stuff_order(i) != 255; i++)
                    borg_note(format("stuff %s.",
                        borg_items[borg_best_stuff_order(i)].desc));
            }
            /* Save the results */
            for (i = 0; i < n; i++)
                best[i] = test[i];

            /* Use it */
            *vp = p;
        }

        /* Success */
        return;
    }

    /* Note the attempt */
    test[n] = slot;

    /* Evaluate the default item */
    borg_best_stuff_aux(n + 1, test, best, vp);

    /* Try other possible objects */
    for (i = 0; i < ((shop_num == BORG_HOME)
                         ? (z_info->pack_size + z_info->store_inven_max)
                         : z_info->pack_size);
         i++) {
        borg_item *item;
        if (i < z_info->pack_size)
            item = &borg_items[i];
        else
            item = &borg_shops[BORG_HOME].ware[i - z_info->pack_size];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require "aware" */
        if (!item->kind)
            continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value)
            continue;

        /* Skip it if it is not decursable */
        if (item->cursed && !item->uncursable)
            continue;

        /* Do not wear not *idd* artifacts */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
            continue;

        /* Make sure it goes in this slot, special consideration for checking
         * rings */
        if (slot != borg_wield_slot(item))
            continue;

        /* Make sure that slot does not have a cursed item */
        if (borg_items[slot].one_ring)
            continue;

        /* Do not wear certain items if I am over weight limit.  It induces
         * loops */
        if (borg.trait[BI_ISENCUMB]) {
            /* Compare Str bonuses */
            if (borg_items[slot].modifiers[OBJ_MOD_STR]
                > item->modifiers[OBJ_MOD_STR])
                continue;
        }

        /* Wear the new item */
        memcpy(&borg_items[slot], item, sizeof(borg_item));

        /* Note the attempt */
        if (i < z_info->pack_size)
            test[n] = i;
        else
            /* if in home, note by adding 100 to item number. */
            test[n] = (i - z_info->pack_size) + 100;

        /* Evaluate the possible item */
        borg_best_stuff_aux(n + 1, test, best, vp);

        /* Restore equipment */
        memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));
    }
}

/*
 * Attempt to instantiate the *best* possible equipment.
 */
bool borg_best_stuff(void)
{
    int     hole;
    char    purchase_target[1];
    int     k;
    uint8_t t_a;
    char    buf[1024];
    int     p;

    int32_t value;

    int i;

    uint8_t test[12];
    uint8_t best[12];

    /* Hack -- Anti-loop */
    if (borg.time_this_panel >= 300)
        return false;

    /* Hack -- Initialize */
    for (k = 0; k < 12; k++) {
        /* Initialize */
        best[k] = test[k] = 255;
    }

    /* Hack -- Copy all the slots */
    for (i = 0; i < INVEN_TOTAL; i++) {
        /* Skip quiver slots */
        if (i >= z_info->pack_size && i < INVEN_WIELD)
            continue;

        /* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));
    }

    if (shop_num == BORG_HOME) {
        /* Hack -- Copy all the store slots */
        for (i = 0; i < z_info->store_inven_max; i++) {
            /* Save the item */
            memcpy(&safe_shops[BORG_HOME].ware[i], &borg_shops[BORG_HOME].ware[i],
                sizeof(borg_item));
        }
    }

    /* Evaluate the inventory */
    value = borg.power;

    /* Determine the best possible equipment */
    (void)borg_best_stuff_aux(0, test, best, &value);

    /* Restore bonuses */
    borg_notice(true);

    /* Make first change. */
    for (k = 0; k < 12; k++) {
        /* Get choice */
        i = best[k];

        /* Ignore non-changes */
        if (i == borg_best_stuff_order(k) || 255 == i)
            continue;

        if (i < 100) {
            borg_item *item = &borg_items[i];

            /* Catch the keyboard flush induced from the 'w' */
            if ((0 == borg_what_text(0, 0, 6, &t_a, buf))
                && (streq(buf, "(Inven"))) {
                borg_keypress(all_letters_nohjkl[i]);

                /* Track the newly worn artifact item to avoid loops */
                if (item->art_idx && (track_worn_num < track_worn_size)) {
                    borg_note("# Noting the wearing of artifact.");
                    track_worn_name1[track_worn_num] = item->art_idx;
                    track_worn_time                  = borg_t;
                    track_worn_num++;
                }
            } else {
                /* wield the item */
                borg_note(format("# Best Combo %s.", item->desc));
                borg_keypress('w');
                borg_keypress(all_letters_nohjkl[i]);
                return true;
            }

            borg.time_this_panel++;

            return true;
        } else {
            borg_item *item;

            /* can't get an item if full. */
            hole = borg_first_empty_inventory_slot();
            if (hole == -1)
                return false;

            i -= 100;

            item = &borg_shops[BORG_HOME].ware[i];

            /* Dont do it if you just sold this item */
            for (p = 0; p < sold_item_num; p++) {
                if (sold_item_tval[p] == item->tval
                    && sold_item_sval[p] == item->sval
                    && sold_item_store[p] == BORG_HOME)
                    return false;
            }

            /* Get the item */
            borg_note(format("# Getting (Best Fit) %s.", item->desc));

            /* Define the special key */
            purchase_target[0] = SHOP_MENU_ITEMS[i];

            /* Purchase that item */
            borg_keypress(purchase_target[0]);
            borg_keypress('p');
            /* press ENTER twice (multiple objects) */
            borg_keypress(KC_ENTER);
            borg_keypress(KC_ENTER);

            /* leave the building */
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);

            /* tick the clock */
            borg.time_this_panel++;

            /* Note that this is a nice item and not to sell it right away */
            borg_best_fit_item = item->art_idx;

            return true;
        }
    }

    /* Nope */
    return false;
}

/*
 * Scan the item list and recharge items before leaving the
 * level.  Right now rod are not recharged from this.
 */
bool borg_wear_recharge(void)
{
    int i, b_i = -1;
    int slot   = -1;
    int b_slot = -1;

    /* No resting in danger */
    if (!borg_check_rest(borg.c.y, borg.c.x))
        return false;

    /* Not if hungry */
    if (borg.trait[BI_ISWEAK])
        return false;

    /* Look for an (wearable- non rod) item to recharge */
    for (i = 0; i < INVEN_TOTAL; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* skip items that are charged */
        if (!item->timeout)
            continue;

        /* Where can it be worn? */
        slot = borg_wield_slot(item);

        /* skip non-ego lights, No need to rest to recharge a torch, which uses
         * fuels turns in o_ptr->timeout */
        if (item->tval == TV_LIGHT
            && (item->sval == sv_light_torch || item->sval == sv_light_lantern))
            continue;

        /* note this one */
        b_i    = i;
        b_slot = slot;
    }

    if (b_i >= INVEN_WIELD) {
        /* Item is worn, no swap is nec. */
        borg_note(
            format("# Waiting for '%s' to Recharge.", borg_items[b_i].desc));

        /* Rest for a while */
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress(KC_ENTER);

        /* done */
        return true;
    }
    /* Item must be worn to be recharged
     */
    if (b_slot >= INVEN_WIELD) {

        /* wear the item */
        borg_note("# Swapping Item for Recharge.");
        borg_keypress(ESCAPE);
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[b_i]);

        /* rest for a while */
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress(KC_ENTER);

        /* done */
        return true;
    }

    /* nothing to recharge */
    return false;
}

/*
 *  check an item for being ammo.
 */
bool borg_is_ammo(int tval)
{
    switch (tval) {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        return true;
    default:
        return false;
    }
}

void borg_init_item_wear(void)
{
    /* Track the worn items to avoid loops */
    track_worn_num   = 0;
    track_worn_size  = 10;
    track_worn_time  = 0;
    track_worn_name1 = mem_zalloc(track_worn_size * sizeof(uint8_t));
}

void borg_free_item_wear(void)
{
    mem_free(track_worn_name1);
    track_worn_name1 = NULL;
}

#endif
