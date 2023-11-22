/**
 * \file borg-think-store.
 * \brief Prepare to perform an action while in a store
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

#include "borg-think-store.h"

#ifdef ALLOW_BORG

#include "../store.h"
#include "../ui-menu.h"

#include "borg-io.h"
#include "borg-item-wear.h"
#include "borg-item.h"
#include "borg-store-buy.h"
#include "borg-store-sell.h"
#include "borg-store.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Choose a shop to visit
 */
bool borg_choose_shop(void)
{
    /* Must be in town */
    if (borg_trait[BI_CDEPTH])
        return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000)
        return (false);
    if (time_this_panel > 1350)
        return (false);

    /* Already flowing to a store to sell something */
    if (goal_shop != -1 && goal_ware != -1)
        return (true);

    /* If poisoned or bleeding -- flow to temple */
    if (borg_trait[BI_ISCUT] || borg_trait[BI_ISPOISONED])
        goal_shop = 3;

    /* If Starving  -- flow to general store */
    if (borg_trait[BI_FOOD] == 0
        || (borg_trait[BI_CURLITE] == 0 && borg_trait[BI_CLEVEL] >= 2)) {
        /* G Store first */
        goal_shop = 0;
    }

    /* Do a quick cheat of the shops and inventory */
    borg_cheat_store();
    borg_notice(true);

    /* if No Lantern -- flow to general store */
    if (borg_trait[BI_CURLITE] == 1 && borg_trait[BI_GOLD] >= 100)
        goal_shop = 0;

    /* If poisoned, bleeding, or needing to shop instantly
     * Buy items straight away, without having to see each shop
     */
    if ((borg_trait[BI_CURLITE] == 0 || borg_trait[BI_FOOD] == 0
            || borg_trait[BI_ISCUT] || borg_trait[BI_ISPOISONED])
        || (borg_trait[BI_CURLITE] == 1 && borg_trait[BI_GOLD] >= 100
            && borg_trait[BI_CLEVEL] < 10)) {
        if (borg_think_shop_buy_useful()) {
            /* Message */
            borg_note(format("# Buying '%s' at '%s' immediately",
                borg_shops[goal_shop].ware[goal_ware].desc,
                f_info[stores[goal_shop].feat].name));

            /* Success */
            return (true);
        }

        /* if temple is out of healing stuff, try the house */
        if (borg_think_home_buy_useful()) {
            /* Message */
            borg_note(format("# Buying '%s' from the home immediately.",
                borg_shops[goal_shop].ware[goal_ware].desc));

            /* Success */
            return (true);
        }
    }

    /* if we are already flowing toward a shop do not check again... */
    if (goal_shop != -1 && goal_ware != -1)
        return true;

    /* Assume no important shop */
    goal_shop = goal_ware = goal_item = -1;

    /* if the borg is scumming for cash for the human player and not himself,
     * we dont want him messing with the home inventory
     */
    if (borg_trait[BI_GOLD] < borg_cfg[BORG_MONEY_SCUM_AMOUNT]
        && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 && !borg_trait[BI_CDEPTH]
        && borg_trait[BI_LIGHT] && !borg_cfg[BORG_SELF_SCUM]) {
        /* Step 0 -- Buy items from the shops (for the player while scumming) */
        if (borg_think_shop_buy_useful()) {
            /* Message */
            borg_note(format("# Buying '%s' at '%s' (money scumming)",
                borg_shops[goal_shop].ware[goal_ware].desc,
                f_info[stores[goal_shop].feat].name));

            /* Success */
            return (true);
        } else
            return (false);
    }

    /* Step 1 -- Sell items to the home */
    if (borg_think_home_sell_useful(false)) {
        /* Message */
        if (goal_item != -1)
            borg_note(format(
                "# Selling '%s' to the home", borg_items[goal_item].desc));
        else
            borg_note(format("# Buying '%s' from the home (step 1)",
                borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }

    /* Step 2 -- Sell items to the shops */
    if (borg_think_shop_sell_useless()) {
        /* Message */
        borg_note(format("# Selling '%s' at '%s'", borg_items[goal_item].desc,
            f_info[stores[goal_shop].feat].name));

        /* Success */
        return (true);
    }

    /* Step 3 -- Buy items from the shops (for the player) */
    if (borg_think_shop_buy_useful()) {

        /* Message */
        borg_note(format("# Buying '%s'(%c) at '%s' (for player 'b')",
            borg_shops[goal_shop].ware[goal_ware].desc,
            SHOP_MENU_ITEMS[goal_ware], f_info[stores[goal_shop].feat].name));

        /* Success */
        return (true);
    }

    /* Step 4 -- Buy items from the home (for the player) */
    if (borg_think_home_buy_useful()) {
        /* Message */
        borg_note(format("# Buying '%s' from the home (step 4)",
            borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }

    /* get rid of junk from home first.  That way the home is 'uncluttered' */
    /* before you buy stuff for it.  This will prevent the problem where an */
    /* item has become a negative value and swapping in a '0' gain item */
    /* (like pottery) is better. */

    /* Step 5 -- Grab items from the home (for the shops) */
    if (borg_think_home_grab_useless()) {
        /* Message */
        borg_note(format("# Grabbing (to sell) '%s' from the home",
            borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }

    /* Do not Stock Up the home while money scumming */
    if (borg_cfg[BORG_MONEY_SCUM_AMOUNT])
        return (false);

    /* Step 6 -- Buy items from the shops (for the home) */
    if (borg_think_shop_grab_interesting()) {
        /* Message */
        borg_note(format("# Grabbing (for home) '%s' at '%s'",
            borg_shops[goal_shop].ware[goal_ware].desc,
            f_info[stores[goal_shop].feat].name));

        /* Success */
        return (true);
    }

    /* Step 7A -- Buy weapons from the home (as a backup item) */
    if (borg_cfg[BORG_USES_SWAPS] && borg_think_home_buy_swap_weapon()) {
        /* Message */
        borg_note(format("# Buying '%s' from the home as a backup",
            borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }
    /* Step 7B -- Buy armour from the home (as a backup item) */
    if (borg_cfg[BORG_USES_SWAPS] && borg_think_home_buy_swap_armour()) {
        /* Message */
        borg_note(format("# Buying '%s' from the home as a backup",
            borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }

    /* Failure */
    return (false);
}

/*
 * Deal with being in a store
 */
bool borg_think_store(void)
{
    /* Hack -- prevent clock wrapping */
    if (borg_t >= 20000 && borg_t <= 20010) {
        /* Clear Possible errors */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Re-examine inven and equip */
        borg_do_inven = true;
        borg_do_equip = true;
    }

    /* update all my equipment and swap items */
    borg_do_inven = true;
    borg_do_equip = true;
    borg_notice(true);

#if 0
    /* Stamp the shop with a time stamp */
    borg_shops[shop_num].when = borg_t;
#endif

    /* Wear "optimal" equipment */
    if (borg_best_stuff())
        return (true);

    /* If using a digger, Wear "useful" equipment.
     * unless that digger is an artifact, then treat
     * it as a normal weapon
     */
    if (borg_items[INVEN_WIELD].tval == TV_DIGGING
        && !borg_items[INVEN_WIELD].art_idx && borg_wear_stuff())
        return (true);

    /* Choose a shop to visit.  Goal_shop indicates he is trying to sell
     * something somewhere. */
    if (borg_choose_shop()) {
        /* Note Pref. */
        if (shop_num != goal_shop)
            borg_note(format("# Currently in store '%d' would prefer '%d'.",
                shop_num + 1, goal_shop + 1));
        else
            borg_note(
                format("# Currently in preferred store '%d'.", goal_shop+1));

        /* Try to sell stuff */
        if (borg_think_shop_sell())
            return (true);

        /* Try to buy stuff */
        if (borg_think_shop_buy())
            return (true);
    }

    /* No shop */
    goal_shop = goal_ware = goal_item = -1;

    /* Leave the store */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Done */
    return (true);
}

#endif