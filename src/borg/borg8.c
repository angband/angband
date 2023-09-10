/* File: borg8.c */
/* Purpose: High level functions for the Borg -BEN- */

#include "../angband.h"

#ifdef ALLOW_BORG

#include "../game-world.h"
#include "../player-calcs.h"
#include "../ui-command.h"
#include "../cave.h"

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg4.h"
#include "borg5.h"
#include "borg6.h"
#include "borg7.h"
#include "borg8.h"

#ifdef BABLOS
extern bool borg_clock_over;
#endif /* bablos */

uint8_t* test;
uint8_t* best;
int32_t* b_home_power;


/* money Scumming is a type of town scumming for money */
static bool borg_money_scum(void)
{

    int dir = -1;
    int divisor = 2;

    borg_grid* ag;

    /* Just a quick check to make sure we are supposed to do this */
    if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] == 0) return (false);

    /* Take note */
    borg_note(format("# Waiting for towns people to breed.  I need %d...", borg_cfg[BORG_MONEY_SCUM_AMOUNT] - borg_gold));

    /* I'm not in a store */
    borg_in_shop = false;

    /* Rest for 9 months */
    if (borg_skill[BI_CLEVEL] >= 35)
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('5');
        borg_keypress('0');
        borg_keypress('0');
        borg_keypress(KC_ENTER);
    }
    else if (borg_skill[BI_CLEVEL] >= 15)
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress(KC_ENTER);
    }
    else /* Low level, dont want to get mobbed */
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('2');
        borg_keypress('5');
        borg_keypress(KC_ENTER);
    }

    /* Don't rest too long at night.  We tend to crash the game if too many
     * townsfolk are on the level
     */
     /* Night or day up in town */
    if ((turn % (10L * z_info->day_length)) < ((10L * z_info->day_length) / 2)) divisor = 5;


    /* sometimes twitch in order to move around some */
    if (borg_t % divisor)
    {
        borg_keypress(ESCAPE);

        /* Pick a random direction */
        while (dir == -1 || dir == 5 || dir == 0)
        {
            dir = randint0(10);

            /* Hack -- set goal */
            g_x = c_x + ddx[dir];
            g_y = c_y + ddy[dir];

            ag = &borg_grids[g_y][g_x];

            /* Skip walls and shops */
            if (ag->feat != FEAT_FLOOR) dir = -1;
        }


        /* Normally move */
        /* Send direction */
        borg_keypress(I2D(dir));
    }

    /* reset the clocks */
    borg_t = 10;
    time_this_panel = 1;
    borg_began = 1;

    /* Done */
    return (true);
}


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
static bool borg_object_similar(borg_item* o_ptr, borg_item* j_ptr)
{
    /* NOTE: This assumes the giving of one item at a time */
    int total = o_ptr->iqty + 1;

    /* Require identical object types */
    if (o_ptr->kind != j_ptr->kind) return (0);


    /* Analyze the items */
    switch (o_ptr->tval)
    {
        /* Chests */
    case TV_CHEST:
    {
        /* Never okay */
        return (0);
    }

    /* Food and Potions and Scrolls */
    case TV_FOOD:
    case TV_POTION:
    case TV_SCROLL:
    {
        /* Assume okay */
        break;
    }

    /* Staffs and Wands */
    case TV_STAFF:
    case TV_WAND:
    {
        /* Require knowledge */
        if ((!o_ptr->aware) || (!j_ptr->aware)) return (0);

        /* Fall through */
    }

    /* Staffs and Wands and Rods */
    case TV_ROD:
    {
        /* Require permission */
/*            if (!testing_stack) return (0);*/

            /* Require identical charges */
/*            if (o_ptr->pval != j_ptr->pval) return (0); */

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
    case TV_DRAG_ARMOR:
    {
        /* Require permission */
/*            if (!testing_stack) return (0);*/

            /* XXX XXX XXX Require identical "sense" status */
            /* if ((o_ptr->ident & ID_SENSE) != */
            /*     (j_ptr->ident & ID_SENSE)) return (0); */

            /* Fall through */
    }

    /* Rings, Amulets, Lites */
    case TV_RING:
    case TV_AMULET:
    case TV_LIGHT:
        /* Require full knowledge of both items */
        if ((!o_ptr->aware) || (!j_ptr->aware)) return (0);
        /* fall through */
    /* Missiles */
    case TV_BOLT:
    case TV_ARROW:
    case TV_SHOT:
    {
        /* Require identical "bonuses" */
        if (o_ptr->to_h != j_ptr->to_h) return (false);
        if (o_ptr->to_d != j_ptr->to_d) return (false);
        if (o_ptr->to_a != j_ptr->to_a) return (false);

        /* Require identical "pval" code */
        if (o_ptr->pval != j_ptr->pval) return (false);

        /* Require identical "artifact" names */
        if (o_ptr->art_idx != j_ptr->art_idx) return (false);

        /* Require identical "ego-item" names */
        if (o_ptr->ego_idx != j_ptr->ego_idx) return (false);

        /* Hack -- Never stack "powerful" items */
        if (!of_is_empty(o_ptr->flags) || !of_is_empty(j_ptr->flags))
            return false;

        /* Hack -- Never stack recharging items */
        if (o_ptr->timeout || j_ptr->timeout) return (false);

        /* Require identical "values" */
        if (o_ptr->ac != j_ptr->ac) return (false);
        if (o_ptr->dd != j_ptr->dd) return (false);
        if (o_ptr->ds != j_ptr->ds) return (false);

        /* Probably okay */
        break;
    }

    /* Various */
    default:
    {
        /* Require knowledge */
        if ((!o_ptr->aware) || (!j_ptr->aware)) return (0);

        /* Probably okay */
        break;
    }
    }


    /* Hack -- Require identical "broken" status */
    if ((o_ptr->ident) != (j_ptr->ident)) return (0);

    /* The stuff with 'note' is not right but it is close.  I think it */
    /* has him assuming that he can't stack sometimes when he can.  This */
    /* is alright, it just causes him to take a bit more time to do */
    /* some exchanges. */
    /* Hack -- require semi-matching "inscriptions" */
    if ((o_ptr->note && !j_ptr->note) ||
        (!o_ptr->note && j_ptr->note)) return (0);

    if (o_ptr->note && j_ptr->note)
    {
        if (o_ptr->note[0] && j_ptr->note[0] &&
            (!streq(o_ptr->note, j_ptr->note)))
            return (0);

        /* Hack -- normally require matching "inscriptions" */
        if ((!streq(o_ptr->note, j_ptr->note))) return (0);
    }

    /* Maximal "stacking" limit */
    if (total >= k_info[o_ptr->kind].base->max_stack) return (0);

    /* They match, so they must be similar */
    return (true);
}

/*
 * Find the mininum amount of some item to buy/sell. For most
 * items this is 1, but for certain items (such as ammunition)
 * it may be higher.  -- RML
 */
static int borg_min_item_quantity(borg_item* item)
{
    /* Only trade in bunches if sufficient cash */
    if (borg_gold < 250) return (1);

    /* Don't trade expensive items in bunches */
    if (item->value > 5) return (1);

    /* Don't trade non-known items in bunches */
    if (!item->aware) return (1);

    /* Only allow some types */
    switch (item->tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    /* Maximum number of items */
    if (item->iqty < 5)
        return (item->iqty);
    return (5);

    case TV_FOOD:
    if (item->iqty < 3)
        return (item->iqty);
    return (3);
#if 0
    case TV_POTION:
    case TV_SCROLL:
    if (item->iqty < 2)
        return (item->iqty);
    return (2);
#endif

    default:
    return (1);
    }
}

/*
 * This file handles the highest level goals, and store interaction.
 *
 * Store interaction strategy
 *
 *   (1) Sell items to the home (for later use)
 ** optimize the stuff in the home... this involves buying and selling stuff
 ** not in the 'best' list.
 *       We sell anything we may need later (see step 4)
 *
 *   (2) Sell items to the shops (for money)
 *       We sell anything we do not actually need
 *
 *   (3) Buy items from the shops (for the player)
 *       We buy things that we actually need
 *
 *   (4) Buy items from the home (for the player)
 *       We buy things that we actually need (see step 1)
 *
 *   (5) Buy items from the shops (for the home)
 *       We buy things we may need later (see step 1)
 *
 *   (6) Buy items from the home (for the stores)
 *       We buy things we no longer need (see step 2)
 *
 *   The basic principle is that we should always act to improve our
 *   "status", and we should sometimes act to "maintain" our status,
 *   especially if there is a monetary reward.  But first we should
 *   attempt to use the home as a "stockpile", even though that is
 *   not worth any money, since it may save us money eventually.
 */

 /* this optimized the home storage by trying every combination... it was too slow.*/
 /* put this code back when running this on a Cray. */
static void borg_think_home_sell_aux2_slow(int n, int start_i)
{
    int i;

    /* All done */
    if (n == z_info->store_inven_max)
    {
        int32_t home_power;

        /* Examine the home  */
        borg_notice_home(NULL, false);

        /* Evaluate the home */
        home_power = borg_power_home();

        /* Track best */
        if (home_power > *b_home_power)
        {
            /* Save the results */
            for (i = 0; i < z_info->store_inven_max; i++) best[i] = test[i];

#if 0
            /* dump, for debugging */
            borg_note(format("Trying Combo (best home power %ld)",
                *b_home_power));
            borg_note(format("             (test home power %ld)", home_power));
            for (i = 0; i < z_info->store_inven_max; i++)
            {
                if (borg_shops[7].ware[i].iqty)
                    borg_note(format("store %d %s (qty-%d).", i,
                        borg_shops[7].ware[i].desc,
                        borg_shops[7].ware[i].iqty));
                else
                    borg_note(format("store %d (empty).", i));
            }
            borg_note(" "); /* add a blank line */
#endif

            /* Use it */
            * b_home_power = home_power;
        }

        /* Success */
        return;
    }

    /* Note the attempt */
    test[n] = n;

    /* Evaluate the default item */
    borg_think_home_sell_aux2_slow(n + 1, start_i);

    /* if this slot and the previous slot is empty, move on to previous slot*/
    /* this will prevent trying a thing in all the empty slots to see if */
    /* empty slot b is better than empty slot a.*/
    if ((n != 0) && !borg_shops[7].ware[n].iqty && !borg_shops[7].ware[n - 1].iqty)
        return;

    /* try other combinations */
    for (i = start_i; i < z_info->pack_size; i++)
    {
        borg_item* item;
        borg_item* item2;

        item = &borg_items[i];
        item2 = &borg_shops[7].ware[n];

        /* Skip empty items */
        /* Require "aware" */
        /* Require "known" */
        if (!item->iqty || !item->kind || !item->aware)
            continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        if (weapon_swap && i == weapon_swap - 1) continue;
        if (armour_swap && i == armour_swap - 1) continue;

        /* stacking? */
        if (borg_object_similar(item2, item))
        {
            item2->iqty++;
            item->iqty--;
        }
        else
        {
            int k;
            bool found_match = false;

            /* eliminate items that would stack else where in the list. */
            for (k = 0; k < z_info->store_inven_max; k++)
            {
                if (borg_object_similar(&safe_home[k], item))
                {
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

            /* remove item from pack */
            item->iqty--;
        }

        /* Note the attempt */
        test[n] = i + z_info->store_inven_max;

        /* Evaluate the possible item */
        borg_think_home_sell_aux2_slow(n + 1, i + 1);

        /* restore stuff */
        memcpy(item2, &safe_home[n], sizeof(borg_item));

        /* put item back into pack */
        item->iqty++;
    }
}


/*
 * this will see what single addition/substitution is best for the home.
 * The formula is not as nice as the one above because it will
 * not check all possible combinations of items. but it is MUCH faster.
 */

static void borg_think_home_sell_aux2_fast(int n, int start_i)
{
    borg_item* item;
    borg_item* item2;
    int32_t home_power;
    int i, k, p;
    bool skip_it = false;

    /* get the starting best (current) */
    /* Examine the home  */
    borg_notice_home(NULL, false);

    /* Evaluate the home  */
    *b_home_power = borg_power_home();

    /* try individual substitutions/additions.   */
    for (n = 0; n < z_info->store_inven_max; n++)
    {
        item2 = &borg_shops[7].ware[n];
        for (i = 0; i < z_info->pack_size; i++)
        {
            item = &borg_items[i];

            /* Skip empty items */
            /* Require "aware" */
            /* Require "known" */

            if (!item->iqty || (!item->kind && !item->aware))
                continue;
            if (weapon_swap && i == weapon_swap - 1) continue;
            if (armour_swap && i == armour_swap - 1) continue;

            /* Do not dump stuff at home that is not fully id'd and should be  */
            /* this is good with random artifacts. */
            if (OPT(player, birth_randarts) && item->art_idx && !item->ident) continue;

            /* Hack -- ignore "worthless" items */
            if (!item->value) continue;

            /* If this item was just bought a the house, don't tell it back to the house */
            for (p = 0; p < bought_item_num; p++)
            {
                if (bought_item_tval[p] == item->tval && bought_item_sval[p] == item->sval &&
                    bought_item_pval[p] == item->pval && bought_item_store[p] == 7) skip_it = true;
            }
            if (skip_it == true) continue;

            /* stacking? */
            if (borg_object_similar(item2, item))
            {
                /* if this stacks with what was previously here */
                item2->iqty++;
            }
            else
            {
                bool found_match = false;

                /* eliminate items that would stack else where in the list. */
                for (k = 0; k < z_info->store_inven_max; k++)
                {
                    if (borg_object_similar(&safe_home[k], item))
                    {
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
            }

            /* remove item from pack */
            item->iqty--;

            /* Note the attempt */
            test[n] = i + z_info->store_inven_max;

            /* Test to see if this is a good substitution. */
            /* Examine the home  */
            borg_notice_home(NULL, false);

            /* Evaluate the home  */
            home_power = borg_power_home();

            /* Track best */
            if (home_power > *b_home_power)
            {
                /* Save the results */
                for (k = 0; k < z_info->store_inven_max; k++) best[k] = test[k];

#if 0
                /* dump, for debugging */
                borg_note(format("Trying Combo (best home power %ld)",
                    *b_home_power));
                borg_note(format("             (test home power %ld)",
                    home_power));
                for (i = 0; i < z_info->store_inven_max; i++)
                    if (borg_shops[7].ware[i].iqty)
                        borg_note(format("store %d %s (qty-%d).", i,
                            borg_shops[7].ware[i].desc,
                            borg_shops[7].ware[i].iqty));
                    else
                        borg_note(format("store %d (empty).", i));

                borg_note(" "); /* add a blank line */
#endif

                /* Use it */
                * b_home_power = home_power;
            }

            /* restore stuff */
            memcpy(item2, &safe_home[n], sizeof(borg_item));

            /* put item back into pack */
            item->iqty++;

            /* put the item back in the test array */
            test[n] = n;
        }
    }
}

/* locate useless item */
static void borg_think_home_sell_aux3(void)
{
    int     i;
    int32_t    borg_empty_home_power;
    int32_t    power;

    /* get the starting power */
    borg_notice(true);
    power = borg_power();

    /* get what an empty home would have for power */
    borg_notice_home(NULL, true);
    borg_empty_home_power = borg_power_home();

    /* go through the inventory and eliminate items that either  */
    /* 1) will not increase the power of an empty house. */
    /* 2) will reduce borg_power if given to home */
    for (i = 0; i < z_info->pack_size; i++)
    {
        int num_items_given;
        num_items_given = 0;

        /* if there is no item here, go to next slot */
        if (!borg_items[i].iqty) continue;

        /* Dont sell back our Best Fit item (avoid loops) */
        if (borg_best_fit_item && borg_best_fit_item == borg_items[i].art_idx) continue;


        /* 1) eliminate garbage items (items that add nothing to an */
        /*     empty house) */
        borg_notice_home(&borg_items[i], false);
        if (borg_power_home() <= borg_empty_home_power)
        {
            safe_items[i].iqty = 0;
            continue;
        }

        /* 2) will reduce borg_power if given to home */
        while (borg_items[i].iqty)
        {
            /* reduce inventory by this item */
            num_items_given++;
            borg_items[i].iqty--;

            /* Examine borg */
            borg_notice(false);

            /* done if this reduces the borgs power */
            if (borg_power() < power)
            {
                /* we gave up one to many items */
                num_items_given--;
                break;
            }
        }

        /* restore the qty */
        borg_items[i].iqty = safe_items[i].iqty;

        /* set the qty to number given without reducing borg power */
        safe_items[i].iqty = num_items_given;
    }
}

/*
 * Step 1 -- sell "useful" things to the home (for later)
 */
static bool borg_think_home_sell_aux(bool save_best)
{
    int icky = z_info->store_inven_max - 1;

    int32_t home_power = -1L;

    int p, i = -1;

    /* if the best is being saved (see borg_think_shop_grab_aux) */
    /* !FIX THIS NEEDS TO BE COMMENTED BETTER */
    if (!save_best)
        b_home_power = &home_power;

    /* clear out our initial best/test objects */
    memset(test, 0, sizeof(z_info->store_inven_max * sizeof(uint8_t)));
    memset(best, 0, sizeof(z_info->store_inven_max * sizeof(uint8_t)));

    /* Hack -- the home is full */
    /* and pack is full */
    if (borg_shops[7].ware[icky].iqty &&
        borg_items[PACK_SLOTS - 1].iqty)
        return (false);

    /* Copy all the store slots */
    for (i = 0; i < z_info->store_inven_max; i++)
    {
        /* Save the item */
        memcpy(&safe_home[i], &borg_shops[7].ware[i], sizeof(borg_item));

        /* clear test arrays (test[i] == i is no change) */
        best[i] = test[i] = i;
    }

    /* Hack -- Copy all the slots */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        /* Save the item -- do not consider these */
        if (weapon_swap && i == weapon_swap - 1) continue;
        if (armour_swap && i == armour_swap - 1) continue;

        /* dont consider the item i just found to be my best fit (4-6-07) */
        if (borg_best_fit_item && borg_best_fit_item == borg_items[i].art_idx) continue;

        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));
    }

    /* get rid of useless items */
    borg_think_home_sell_aux3();

    /* Examine the borg once more with full inventory then swap in the */
    /* safe_items for the home optimization */
    borg_notice(false);

    /* swap quantities (this should be all that is different) */
    for (i = 0; i < z_info->pack_size; i++)
    {
        uint8_t save_qty;
        if (weapon_swap && i == weapon_swap - 1) continue;
        if (armour_swap && i == armour_swap - 1) continue;

        save_qty = safe_items[i].iqty;
        safe_items[i].iqty = borg_items[i].iqty;
        borg_items[i].iqty = save_qty;
    }

    *b_home_power = -1;

    /* find best combo for home. */
    if (borg_cfg[BORG_SLOW_OPTIMIZEHOME])
    {
        borg_think_home_sell_aux2_slow(0, 0);
    }
    else
    {
        borg_think_home_sell_aux2_fast(0, 0);
    }

    /* restore bonuses and such */
    for (i = 0; i < z_info->store_inven_max; i++)
    {
        memcpy(&borg_shops[7].ware[i], &safe_home[i], sizeof(borg_item));
    }

    for (i = 0; i < INVEN_TOTAL; i++)
    {
        // !FIX !TODO !AJG not sure this is right...  we should probably be 
        // restoring the item anyway and just not considering it at another point
        if (weapon_swap && i == weapon_swap - 1) continue;
        if (armour_swap && i == armour_swap - 1) continue;
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));
    }

    borg_notice(false);
    borg_notice_home(NULL, false);

    /* Drop stuff that will stack in the home */
    for (i = 0; i < z_info->store_inven_max; i++)
    {
        /* if this is not the item that was there, */
        /* drop off the item that replaces it. */
        if (best[i] != i && best[i] != 255)
        {
            borg_item* item = &borg_items[best[i] - z_info->store_inven_max];
            borg_item* item2 = &borg_shops[7].ware[i];

            /* if this item is not the same as what was */
            /* there before take it. */
            if (!borg_object_similar(item2, item))
                continue;

            /* There are limted quantity per slot */
            if (item2->iqty > 90) continue;

            goal_shop = 7;
            goal_item = best[i] - z_info->store_inven_max;

            return (true);
        }
    }

    /* Get rid of stuff in house but not in 'best' house if  */
    /* pack is not full */
    if (!borg_items[PACK_SLOTS - 1].iqty)
    {
        for (i = 0; i < z_info->store_inven_max; i++)
        {
            /* if this is not the item that was there, */
            /* get rid of the item that was there */
            if ((best[i] != i) &&
                (borg_shops[7].ware[i].iqty))
            {
                borg_item* item = &borg_items[best[i] - z_info->store_inven_max];
                borg_item* item2 = &borg_shops[7].ware[i];

                /* if this item is not the same as what was */
                /* there before take it. */
                if (borg_object_similar(item, item2))
                    continue;

                /* skip stuff if we sold bought it */
                /* skip stuff if we sold/bought it */
                for (p = 0; p < sold_item_num; p++)
                {
                    if (sold_item_tval[p] == item2->tval && sold_item_sval[p] == item2->sval &&
                        sold_item_store[p] == 7) return (false);
                }

                goal_shop = 7;
                goal_ware = i;

                return true;
            }
        }
    }

    /* Drop stuff that is in best house but currently in inventory */
    for (i = 0; i < z_info->store_inven_max; i++)
    {
        /* if this is not the item that was there,  */
        /* drop off the item that replaces it. */
        if (best[i] != i && best[i] != 255)
        {
            /* hack dont sell DVE */
            if (!borg_items[best[i] - z_info->store_inven_max].iqty) return (false);

            goal_shop = 7;
            goal_item = best[i] - z_info->store_inven_max;

            return (true);
        }
    }

    /* Return our num_ counts to normal */
    borg_notice_home(NULL, false);

    /* Assume not */
    return (false);
}


/*
 * Determine if an item can be sold in the given store
 *
 * XXX XXX XXX Consider use of "icky" test on items
 */
static bool borg_good_sell(borg_item* item, int who)
{
    int i;

    /* Never sell worthless items */
    if (item->value <= 0)
    {
        /* except unidentified potions and scrolls.  Since these can't be IDd, best to sell them */
        if (!((item->tval == TV_POTION || item->tval == TV_SCROLL) && !item->ident))
            return (false);
    }

    /* Never sell valuable non-id'd items */
    if (borg_item_note_needs_id(item)) return (false);

    /* Worshipping gold or scumming will allow the sale */
    if (item->value > 0 &&
        ((borg_cfg[BORG_WORSHIPS_GOLD] || borg_skill[BI_MAXCLEVEL] < 10) ||
            ((borg_cfg[BORG_MONEY_SCUM_AMOUNT] < borg_gold) && 
                borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0)))
    {
        /* Borg is allowed to continue in this routine to sell non-ID items */
    }
    else /* Some items must be ID, or at least 'known' */
    {
        /* Analyze the type */
        switch (item->tval)
        {
        case TV_POTION:
        case TV_SCROLL:

        /* Always sell potions and scrolls, it is the only way to ID other than using */

        /* Spell casters should not sell ResMana to shop unless
         * they have tons in the house
         */
        if (item->tval == TV_POTION &&
            item->sval == sv_potion_restore_mana &&
            borg_skill[BI_MAXSP] > 100 &&
            borg_has[kv_potion_restore_mana] + num_mana > 99) return (false);

        break;

        case TV_FOOD:
        case TV_ROD:
        case TV_WAND:
        case TV_STAFF:
        case TV_RING:
        case TV_AMULET:
        case TV_LIGHT:

        /* Never sell if not "known" */
        if (!item->ident && borg_item_worth_id(item) && (borg_skill[BI_MAXDEPTH] > 35)) return (false);

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

        /* Only sell "known" items (unless "icky") */
        if (!item->ident && borg_item_worth_id(item)) return (false);

        break;
        }
    }

    /* Do not sell stuff that is not fully id'd and should be  */
    if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
    {
        /* CHECK THE ARTIFACTS */
        /* For now check all artifacts */
        return (false);
    }
    /* Do not sell stuff that is not fully id'd and should be  */
    if (!item->ident && item->ego_idx)
    {
        if (borg_ego_has_random_power(&e_info[borg_items[INVEN_OUTER].ego_idx]))
        {
            return (false);
        }
    }

    /* Do not sell it if I just bought one */
    /* do not buy the item if I just sold it. */
    for (i = 0; i < bought_item_num; i++)
    {
        if (bought_item_tval[i] == item->tval && bought_item_sval[i] == item->sval &&
            (bought_item_store[i] == who || who != 7))
        {
#if 0
            borg_note(format("# Choosing not to sell back %s", item->desc));
#endif
            return (false);
        }
    }

    /* Switch on the store */
    switch (who + 1)
    {
        /* General Store */
    case 1:
    /* Analyze the type */
    switch (item->tval)
    {
    case TV_FOOD:
    case TV_MUSHROOM:
    case TV_FLASK:
    return (true);
    }

    /* Won't buy anything */
    break;

    /* Armoury */
    case 2:

    /* Analyze the type */
    switch (item->tval)
    {
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
    return (true);
    }
    break;

    /* Weapon Shop */
    case 3:

    /* Analyze the type */
    switch (item->tval)
    {
    case TV_SHOT:
    case TV_BOLT:
    case TV_ARROW:
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    return (true);
    }
    break;

    /* Bookstore */
    case 4:

    /* Analyze the type */
    switch (item->tval)
    {
    case TV_PRAYER_BOOK:
    case TV_MAGIC_BOOK:
    case TV_NATURE_BOOK:
    case TV_SHADOW_BOOK:
    case TV_OTHER_BOOK:
    return (true);
    }
    break;

    /* book store --Alchemist */
    case 5:

    /* Analyze the type */
    switch (item->tval)
    {
    case TV_SCROLL:
    case TV_POTION:
    return (true);
    }
    break;

    /* Magic Shop */
    case 6:

    /* Analyze the type */
    switch (item->tval)
    {
    case TV_AMULET:
    case TV_RING:
    case TV_SCROLL:
    case TV_POTION:
    case TV_STAFF:
    case TV_WAND:
    case TV_ROD:
    case TV_MAGIC_BOOK:
    return (true);
    }
    break;
    /* Black Market --they buy most things.*/
    case 7:

    /* Analyze the type */
    switch (item->tval)
    {
    case TV_LIGHT:
    case TV_CLOAK:
    case TV_FOOD:
    return (true);
    }
    break;

    }

    /* Assume not */
    return (false);
}



/*
 * Step 2 -- sell "useless" items to a shop (for cash)
 */
static bool borg_think_shop_sell_aux(void)
{
    int icky = z_info->store_inven_max - 1;

    int k, b_k = -1;
    int i, b_i = -1;
    int qty = 1;
    int32_t p, b_p = 0L;
    int32_t c = 0L;
    int32_t b_c = 30001L;

    bool fix = false;


    /* Evaluate */
    b_p = my_power;

    /* Check each shop */
    for (k = 0; k < (z_info->store_max - 1); k++)
    {
        /* Hack -- Skip "full" shops */
        if (borg_shops[k].ware[icky].iqty) continue;

        /* Save the store hole */
        memcpy(&safe_shops[k].ware[icky], &borg_shops[k].ware[icky], sizeof(borg_item));

        /* Sell stuff */
        for (i = 0; i < z_info->pack_size; i++)
        {
            borg_item* item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip some important type items */
            if ((item->tval == borg_skill[BI_AMMO_TVAL] ) && (borg_skill[BI_AMISSILES] < 45)) continue;
            if (item->tval == TV_ROD && item->sval == sv_rod_healing &&
                borg_has[kv_rod_healing] <= 3) continue;

            if (borg_class == CLASS_WARRIOR &&
                item->tval == TV_ROD && item->sval == sv_rod_mapping &&
                item->iqty <= 2) continue;

            /* Avoid selling staff of dest*/
            if (item->tval == TV_STAFF && item->sval == sv_staff_destruction &&
                borg_skill[BI_ASTFDEST] < 2) continue;

            /* Do not sell our attack wands if they still have charges */
            if (item->tval == TV_WAND && borg_skill[BI_CLEVEL] < 35 &&
                (item->sval == sv_wand_magic_missile || item->sval == sv_wand_stinking_cloud ||
                    item->sval == sv_wand_annihilation) && item->pval != 0) continue;

            /* dont sell our swap items */
            if (weapon_swap && i == weapon_swap - 1) continue;
            if (armour_swap && i == armour_swap - 1) continue;

            /* Skip "bad" sales */
            if (!borg_good_sell(item, k)) continue;

            /* Save the item */
            memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

            /* Give the item to the shop */
            memcpy(&borg_shops[k].ware[icky], &safe_items[i], sizeof(borg_item));

            /* get the quantity */
            qty = borg_min_item_quantity(item);

            /* Give a single item */
            borg_shops[k].ware[icky].iqty = qty;

            /* Lose a single item */
            borg_items[i].iqty -= qty;

            /* Fix later */
            fix = true;

            /* Examine the inventory */
            borg_notice(false);

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

            /* Ignore "bad" sales */
            if (p < b_p) continue;

            /* Extract the "price" */
            c = ((item->value < 30000L) ? item->value : 30000L);

            /* sell cheap items first.  This is done because we may have to */
            /* buy the item back in some very strange cercemstances. */
            if ((p == b_p) && (c >= b_c)) continue;

            /* Maintain the "best" */
            b_k = k; b_i = i; b_p = p; b_c = c;
        }

        /* Restore the store hole */
        memcpy(&borg_shops[k].ware[icky], &safe_shops[k].ware[icky], sizeof(borg_item));
    }

    /* Examine the inventory */
    if (fix) borg_notice(true);

    /* Sell something (if useless) */
    if ((b_k >= 0) && (b_i >= 0))
    {
        /* Visit that shop */
        goal_shop = b_k;

        /* Sell that item */
        goal_item = b_i;

        /* Success */
        return (true);
    }

    /* Assume not */
    return (false);
}



/*
 * Help decide if an item should be bought from a real store
 *
 * We prevent the purchase of enchanted (or expensive) ammo,
 * so we do not spend all our money on temporary power.
 *
 * if level 35, who needs cash?  buy the expecive ammo!
 *
 * We prevent the purchase of low level discounted books,
 * so we will not waste slots on cheap books.
 *
 * We prevent the purchase of items from the black market
 * which are often available at normal stores, currently,
 * this includes low level books, and all wands and staffs.
 */
static bool borg_good_buy(borg_item* item, int who, int ware)
{
    int p;
    bool dungeon_book = false;

    /* Check the object */
    switch (item->tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    if (borg_skill[BI_CLEVEL] < 35)
    {
        if (item->to_h) return (false);
        if (item->to_d) return (false);
    }
    break;

    case TV_PRAYER_BOOK:
    case TV_MAGIC_BOOK:
    case TV_NATURE_BOOK:
    case TV_SHADOW_BOOK:
    case TV_OTHER_BOOK:
    {
        int i;
        /* not our book */
        if (!obj_kind_can_browse(&k_info[item->kind])) return (false);

        /* keep track of if this is a book from the dungeon */
        for (i = 0; i < player->class->magic.num_books; i++)
        {
            struct class_book book = player->class->magic.books[i];
            if (item->tval == book.tval && item->sval == book.sval && book.dungeon)
            {
                dungeon_book = true;
            }
        }
    }
    break;
    }


    /* Don't buy from the BM until we are rich */
    if (who == 6)
    {
        /* buying Remove Curse scroll is acceptable */
        if (item->tval == TV_SCROLL && item->sval == sv_scroll_remove_curse &&
            borg_skill[BI_FIRST_CURSED]) return (true);

        /* Buying certain special items are acceptable */
        if ((item->tval == TV_POTION &&
            ((item->sval == sv_potion_star_healing) ||
                (item->sval == sv_potion_life) ||
                (item->sval == sv_potion_healing) ||
                (item->sval == sv_potion_inc_str && my_stat_cur[STAT_STR] < (18 + 100)) ||
                (item->sval == sv_potion_inc_int && my_stat_cur[STAT_INT] < (18 + 100)) ||
                (item->sval == sv_potion_inc_wis && my_stat_cur[STAT_WIS] < (18 + 100)) ||
                (item->sval == sv_potion_inc_dex && my_stat_cur[STAT_DEX] < (18 + 100)) ||
                (item->sval == sv_potion_inc_con && my_stat_cur[STAT_CON] < (18 + 100)))) ||
            (item->tval == TV_ROD &&
                ((item->sval == sv_rod_healing) ||
                    /* priests and paladins can cast recall */
                    (item->sval == sv_rod_recall && borg_class != CLASS_PRIEST && borg_class != CLASS_PALADIN) ||
                    /* druid and ranger can cast haste*/
                    (item->sval == sv_rod_speed && borg_class != CLASS_DRUID && borg_class != CLASS_RANGER) ||
                    /* mage and rogue can cast teleport away */
                    (item->sval == sv_rod_teleport_other && borg_class != CLASS_MAGE && borg_class == CLASS_ROGUE) ||
                    (item->sval == sv_rod_illumination && (!borg_skill[BI_ALITE])))) ||
            (obj_kind_can_browse(&k_info[item->kind]) && amt_book[borg_get_book_num(item->sval)] == 0 && dungeon_book) ||
            (item->tval == TV_SCROLL &&
                (item->sval == sv_scroll_teleport_level ||
                    item->sval == sv_scroll_teleport)))
        {
            /* Hack-- Allow the borg to scum for this Item */
            if (borg_cfg[BORG_SELF_SCUM] &&  /* borg is allowed to scum */
                borg_skill[BI_CLEVEL] >= 10 && /* Be of sufficient level */
                borg_skill[BI_LIGHT] &&   /* Have some Perma lite source */
                borg_skill[BI_FOOD] + num_food >= 100 && /* Have plenty of food */
                item->cost <= 85000) /* Its not too expensive */
            {
                if (adj_dex_safe[borg_skill[BI_DEX]] + borg_skill[BI_CLEVEL] > 90) /* Good chance to thwart mugging */
                {
                    /* Record the amount that I need to make purchase */
                    borg_cfg[BORG_MONEY_SCUM_AMOUNT] = item->cost;
                    borg_money_scum_who = who;
                    borg_money_scum_ware = ware;
                }
            }

            /* Ok to buy this */
            return (true);
        }

        if ((borg_skill[BI_CLEVEL] < 15) && (borg_gold < 20000))
            return (false);
        if ((borg_skill[BI_CLEVEL] < 35) && (borg_gold < 15000))
            return (false);
        if (borg_gold < 10000)
            return (false);
    }

    /* do not buy the item if I just sold it. */
    for (p = 0; p < sold_item_num; p++)
    {

        if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval && sold_item_store[p] == who)
        {
            if (borg_cfg[BORG_VERBOSE]) 
                borg_note(format("# Choosing not to buy back %s", item->desc));
            return (false);
        }
    }

    /* Do not buy a second digger */
    if (item->tval == TV_DIGGING)
    {
        int ii;

        /* scan for an existing digger */
        for (ii = 0; ii < z_info->pack_size; ii++)
        {
            borg_item* item2 = &borg_items[ii];


            /* skip non diggers */
            if (item2->tval == TV_DIGGING) return (false);
#if 0
            /* perhaps let him buy a digger with a better
             * pval than his current digger
             */
            {if (item->pval <= item2->pval) return (false); }
#endif
        }
    }

    /* Low level borgs should not waste the money on certain things */
    if (borg_skill[BI_MAXCLEVEL] < 5)
    {
        /* next book, cant read it */
        if (obj_kind_can_browse(&k_info[item->kind]) &&
            item->sval >= 1) return (false);
    }

    /* Not direct spell casters and the extra books */
    /* classes that are direct spell casters get more than 3 books */
    if ((player->class->magic.num_books < 4) &&
        borg_skill[BI_MAXCLEVEL] <= 8)
    {
        if (obj_kind_can_browse(&k_info[item->kind]) &&
            item->sval >= 1) return (false);
    }



    /* Okay */
    return (true);
}



/*
 * Step 3 -- buy "useful" things from a shop (to be used)
 */
static bool borg_think_shop_buy_aux(void)
{
    int hole = borg_first_empty_inventory_slot();

    int slot;
    int qty = 1;

    int k, b_k = -1;
    int n, b_n = -1;
    int32_t p, b_p = 0L;
    int32_t c, b_c = 0L;

    bool fix = false;

    /* Require one empty slot */
    if (hole == -1) return false;

    /* Already have a target 9-4-05*/
    if (goal_ware != -1) return (false);

    /* Extract the "power" */
    b_p = my_power;
    b_p = borg_power();

    /* Check the shops */
    for (k = 0; k < (z_info->store_max - 1); k++)
    {

        /* If I am bad shape up, only see certain stores */
        if ((borg_skill[BI_CURLITE] == 0 || borg_skill[BI_FOOD] == 0) &&
            k != 0 && k != 7) continue;
        if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && k != 3) continue;


        /* Scan the wares */
        for (n = 0; n < z_info->store_inven_max; n++)
        {
            borg_item* item = &borg_shops[k].ware[n];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip "bad" buys */
            if (!borg_good_buy(item, k, n)) continue;

            /* Attempting to scum money, don't buy other stuff unless it is our home or food-store */
            if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] && (k != borg_money_scum_who || n != borg_money_scum_ware)) continue;

            /* Hack -- Require "sufficient" cash */
            if (borg_gold < item->cost) continue;

            /* Skip it if I just sold this item. XXX XXX*/

            /* Special check for 'immediate shopping' */
            if (borg_skill[BI_FOOD] == 0 &&
                (item->tval != TV_FOOD &&
                    (item->tval != TV_SCROLL &&
                        item->sval != sv_scroll_satisfy_hunger))) continue;

            /* Don't fill up on attack wands, its ok to buy a few */
            if (item->tval == TV_WAND &&
                (item->sval == sv_wand_magic_missile || item->sval == sv_wand_stinking_cloud) &&
                amt_cool_wand > 40) continue;

            /* These wands are not useful later on, we need beefier attacks */
            if (item->tval == TV_WAND &&
                (item->sval == sv_wand_magic_missile || item->sval == sv_wand_stinking_cloud) &&
                borg_skill[BI_MAXCLEVEL] > 30) continue;

            /* Save shop item */
            memcpy(&safe_shops[k].ware[n], &borg_shops[k].ware[n], sizeof(borg_item));

            /* Save hole */
            memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

            /* Save the number to trade */
            qty = borg_min_item_quantity(item);

            /* Remove one item from shop (sometimes) */
            borg_shops[k].ware[n].iqty -= qty;

            /* Obtain "slot" */
            slot = borg_wield_slot(item);

            /* XXX what if the item is a ring?  we have 2 ring slots --- copy it from the Home code */

            /* He will not replace his Brightness Torch with a plain one, so he ends up
             * not buying any torches.  Force plain torches for purchase to be seen as
             * fuel only
             */
            if (item->tval == TV_LIGHT && item->sval == sv_light_torch &&
                of_has(borg_items[INVEN_LIGHT].flags, OF_BURNS_OUT))
            {
                slot = -1;
            }

            /* Hack, we keep diggers as a back-up, not to
             * replace our current weapon
             */
            if (item->tval == TV_DIGGING) slot = -1;

            /* Consider new equipment */
            if (slot >= 0)
            {
                /* Save old item */
                memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

                /* Move equipment into inventory */
                memcpy(&borg_items[hole], &safe_items[slot], sizeof(borg_item));

                /* Move new item into equipment */
                memcpy(&borg_items[slot], &safe_shops[k].ware[n], sizeof(borg_item));

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
            else
            {
                /* Move new item into inventory */
                memcpy(&borg_items[hole], &safe_shops[k].ware[n], sizeof(borg_item));

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
            memcpy(&borg_shops[k].ware[n], &safe_shops[k].ware[n], sizeof(borg_item));

            /* Obtain the "cost" of the item */
            c = item->cost * qty;

#if 0
            /* Penalize the cost of expensive items */
            if (c > borg_gold / 10) p -= c;
#endif

            /* Ignore "bad" purchases */
            if (p <= b_p) continue;

            /* Ignore "expensive" purchases */
            if ((p == b_p) && (c >= b_c)) continue;

            /* Save the item and cost */
            b_k = k; b_n = n; b_p = p; b_c = c;
        }
    }


    /* Examine the inventory */
    if (fix) borg_notice(true);

    /* Buy something */
    if ((b_k >= 0) && (b_n >= 0))
    {
        /* Visit that shop */
        goal_shop = b_k;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (true);
    }

    /* Nope */
    return (false);
}


/*
 * Step 4 -- buy "useful" things from the home (to be used)
 */
static bool borg_think_home_buy_aux(void)
{

    int hole;
    int slot, i;
    int stack;
    int qty = 1;
    int n, b_n = -1;
    int32_t p, b_p = 0L;
    int32_t p_left = 0;
    int32_t p_right = 0;

    bool fix = false;
    bool skip_it = false;


    /* Extract the "power" */
    b_p = my_power;

    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++)
    {
        borg_item* item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip it if I just sold it */
        for (i = 0; i < sold_item_num; i++)
        {
            if (sold_item_tval[i] == item->tval && sold_item_sval[i] == item->sval)
            {
                if (borg_cfg[BORG_VERBOSE]) 
                    borg_note(format("# Choosing not to buy back '%s' from home.", item->desc));
                skip_it = true;
            }
        }
        if (skip_it == true) continue;

        /* Reset the 'hole' in case it was changed by the last stacked item.*/
        hole = borg_first_empty_inventory_slot();
        if (hole == -1)
            continue;

        /* borg_note(format("# Considering buying (%d)'%s' (pval=%d) from home.", item->iqty,item->desc, item->pval)); */

        /* Save shop item */
        memcpy(&safe_shops[7].ware[n], &borg_shops[7].ware[n], sizeof(borg_item));

        /* Save hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Save the number */
        qty = borg_min_item_quantity(item);

        /* Remove one item from shop (sometimes) */
        borg_shops[7].ware[n].iqty -= qty;

        /* Obtain "slot" */
        slot = borg_wield_slot(item);
        stack = borg_slot(item->tval, item->sval);

        /* Consider new equipment-- Must check both ring slots */
        p = 0;
        if (slot >= 0)
        {
            /* Require two empty slots */
            if (hole == -1) continue;
            if ((hole + 1) >= PACK_SLOTS) continue;

            /* Check Rings */
            if (slot == INVEN_LEFT)
            {
                /** First Check Left Hand **/

                /* special curse check for left ring */
                if (!borg_items[INVEN_LEFT].one_ring)
                {
                    /* Save old item */
                    memcpy(&safe_items[slot], &borg_items[slot], sizeof(borg_item));

                    /* Move equipment into inventory */
                    memcpy(&borg_items[hole], &safe_items[slot], sizeof(borg_item));

                    /* Move new item into equipment */
                    memcpy(&borg_items[slot], &safe_shops[7].ware[n], sizeof(borg_item));

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
                    borg_note(format("   Against Item %s   (borg_power %ld)", safe_items[slot].desc, my_power));
#endif
                    /* Restore old item */
                    memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));
                }


                /** Second Check Right Hand **/
                /* special curse check for right ring */
                if (!borg_items[INVEN_RIGHT].one_ring)
                {
                    /* Save old item */
                    memcpy(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT], sizeof(borg_item));

                    /* Move equipment into inventory */
                    memcpy(&borg_items[hole], &safe_items[INVEN_RIGHT], sizeof(borg_item));

                    /* Move new item into equipment */
                    memcpy(&borg_items[INVEN_RIGHT], &safe_shops[7].ware[n], sizeof(borg_item));

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
                    borg_note(format("   Against Item %s   (borg_power %ld)", safe_items[INVEN_RIGHT].desc, my_power));
#endif
                    /* Restore old item */
                    memcpy(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], sizeof(borg_item));
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
                memcpy(&borg_items[slot], &safe_shops[7].ware[n], sizeof(borg_item));

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
                borg_note(format("   Against Item %s   (borg_power %ld)", safe_items[slot].desc, my_power));
#endif
                /* Restore old item */
                memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));
            } /* non rings */
        } /* equip */

        /* Consider new inventory.*/
        /* note, we may grab an equipable if, for example, we want to ID it */
        if (p <= b_p)
        {
            /* Restore hole if we are trying an item in inventory that didn't work equipped */
            if (slot >= 0)
                memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

            if (stack != -1) hole = stack;

            /* Require two empty slots */
            if (stack == -1 && hole == -1) continue;
            if (stack == -1 && (hole + 1) >= PACK_SLOTS) continue;

            /* Save hole (could be either empty slot or stack */
            memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

            /* Move new item into inventory */
            memcpy(&borg_items[hole], &safe_shops[7].ware[n], sizeof(borg_item));

            /* Is this new item merging into an exisiting stack? */
            if (stack != -1)
            {
                /* Add a quantity to the stack */
                borg_items[hole].iqty = safe_items[hole].iqty + qty;
            }
            else
            {
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
        memcpy(&borg_shops[7].ware[n], &safe_shops[7].ware[n], sizeof(borg_item));

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and cost */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(true);

    /* Buy something */
    if ((b_n >= 0) && (b_p > my_power))
    {
        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (true);
    }

    /* Nope */
    return (false);
}



/*
 * Step 5 -- buy "interesting" things from a shop (to be used later)
 */
static bool borg_think_shop_grab_aux(void)
{

    int k, b_k = -1;
    int n, b_n = -1;
    int qty = 1;

    int32_t s, b_s = 0L;
    int32_t c, b_c = 0L;
    int32_t borg_empty_home_power;
    int hole;

    /* Dont do this if Sauron is dead */
    if (borg_race_death[546] != 0) return (false);

    /* not until later-- use that money for better equipment */
    if (borg_skill[BI_CLEVEL] < 15) return (false);

    /* get what an empty home would have for power */
    borg_notice_home(NULL, true);
    borg_empty_home_power = borg_power_home();

    b_home_power = &s;

    hole = borg_first_empty_inventory_slot();

    /* Require two empty slots */
    if (hole == -1) return (false);
    if (hole + 1 >= PACK_SLOTS) return (false);

    /* Examine the home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    b_s = borg_power_home();

    /* Check the shops */
    for (k = 0; k < (z_info->store_max - 1); k++)
    {
        /* Scan the wares */
        for (n = 0; n < z_info->store_inven_max; n++)
        {
            borg_item* item = &borg_shops[k].ware[n];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip "bad" buys */
            if (!borg_good_buy(item, k, n)) continue;

            /* Dont buy easy spell books late in the game */
            /* Hack -- Require some "extra" cash */
            if (borg_gold < 1000L + item->cost * 5) continue;

            /* make this the next to last item that the player has */
            /* (can't make it the last or it thinks that both player and */
            /*  home are full) */
            memcpy(&borg_items[hole], &borg_shops[k].ware[n], sizeof(borg_item));

            /* Save the number */
            qty = borg_min_item_quantity(item);

            /* Give a single item */
            borg_items[hole].iqty = qty;

            /* make sure this item would help an empty home */
            borg_notice_home(&borg_shops[k].ware[n], false);
            if (borg_empty_home_power >= borg_power_home()) continue;

            /* optimize the home inventory */
            if (!borg_think_home_sell_aux(true)) continue;

            /* Obtain the "cost" of the item */
            c = item->cost * qty;

            /* Penalize expensive items */
            if (c > borg_gold / 10) s -= c;

            /* Ignore "bad" sales */
            if (s < b_s) continue;

            /* Ignore "expensive" purchases */
            if ((s == b_s) && (c >= b_c)) continue;

            /* Save the item and cost */
            b_k = k; b_n = n; b_s = s; b_c = c;
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
    goal_shop = goal_ware = goal_item = -1;

    /* Buy something */
    if ((b_k >= 0) && (b_n >= 0))
    {
        /* Visit that shop */
        goal_shop = b_k;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (true);
    }

    /* Nope */
    return (false);
}


/*
 * Step 6 -- take "useless" things from the home (to be sold)
 */
static bool borg_think_home_grab_aux(void)
{
    int p, n, b_n = -1;
    int32_t s, b_s = 0L;
    int qty = 1;
    bool skip_it = false;
    int hole = borg_first_empty_inventory_slot();

    /* Require two empty slots */
    if (hole == -1) return (false);
    if (hole + 1 >= PACK_SLOTS) return (false);


    /* Examine the home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    b_s = borg_power_home();


    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++)
    {
        borg_item* item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* skip stuff if we sold bought it */
        for (p = 0; p < sold_item_num; p++)
        {
            if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval && sold_item_store[p] == 7) skip_it = true;
        }
        if (skip_it == true)
            continue;

        /* Save shop item */
        memcpy(&safe_shops[7].ware[n], &borg_shops[7].ware[n], sizeof(borg_item));

        /* Save the number */
        qty = borg_min_item_quantity(item);

        /* Remove one item from shop */
        borg_shops[7].ware[n].iqty -= qty;

        /* Examine the home */
        borg_notice_home(NULL, false);

        /* Evaluate the home */
        s = borg_power_home();

        /* Restore shop item */
        memcpy(&borg_shops[7].ware[n], &safe_shops[7].ware[n], sizeof(borg_item));

        /* Ignore "bad" sales */
        if (s < b_s) continue;

        /* Maintain the "best" */
        b_n = n; b_s = s;
    }

    /* Examine the home */
    borg_notice_home(NULL, false);

    /* Evaluate the home */
    s = borg_power_home();

    /* Stockpile */
    if (b_n >= 0)
    {
        /* Visit the home */
        goal_shop = 7;

        /* Grab that item */
        goal_ware = b_n;

        /* Success */
        return (true);
    }

    /* Assume not */
    return (false);
}

/*
 * Step 7A -- buy "useful" weapons from the home (to be used as a swap)
 */
static bool borg_think_home_buy_swap_weapon(void)
{
    int hole;

    int slot;
    int old_weapon_swap;
    int32_t old_weapon_swap_value;
    int old_armour_swap;
    int32_t old_armour_swap_value;
    int n, b_n = -1;
    int32_t p = 0L, b_p = 0L;

    bool fix = false;


    /* save the current values */
    old_weapon_swap = weapon_swap;
    old_weapon_swap_value = weapon_swap_value;
    old_armour_swap = armour_swap;
    old_armour_swap_value = armour_swap_value;

    if (weapon_swap <= 0 || weapon_swap_value <= 0)
    {
        hole = borg_first_empty_inventory_slot();
        weapon_swap_value = -1L;
    }
    else
    {
        hole = weapon_swap-1;
    }
    if (hole == -1)
        return (false);

    /* Extract the "power" */
    b_p = weapon_swap_value;

    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++)
    {
        borg_item* item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Obtain "slot" make sure its a weapon */
        slot = borg_wield_slot(item);
        if (slot != INVEN_WIELD) continue;

        /* Save shop item */
        memcpy(&safe_shops[7].ware[n], &borg_shops[7].ware[n], sizeof(borg_item));

        /* Save hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Remove one item from shop */
        borg_shops[7].ware[n].iqty--;


        /* Consider new equipment */
        if (slot == INVEN_WIELD)
        {
            /* Move new item into inventory */
            memcpy(&borg_items[hole], &safe_shops[7].ware[n], sizeof(borg_item));

            /* Only a single item */
            borg_items[hole].iqty = 1;

            /* Fix later */
            fix = true;

            /* Examine the iventory and swap value*/
            borg_notice(true);

            /* Evaluate the new equipment */
            p = weapon_swap_value;
        }

        /* Restore hole */
        memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

        /* Restore shop item */
        memcpy(&borg_shops[7].ware[n], &safe_shops[7].ware[n], sizeof(borg_item));

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and value */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(true);

    /* Buy something */
    if ((b_n >= 0) && (b_p > weapon_swap_value))
    {
        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Restore the values */
        weapon_swap = old_weapon_swap;
        weapon_swap_value = old_weapon_swap_value;
        armour_swap = old_armour_swap;
        armour_swap_value = old_armour_swap_value;

        /* Success */
        return (true);
    }

    /* Restore the values */
    weapon_swap = old_weapon_swap;
    weapon_swap_value = old_weapon_swap_value;
    armour_swap = old_armour_swap;
    armour_swap_value = old_armour_swap_value;

    /* Nope */
    return (false);
}
/*
 * Step 7B -- buy "useful" armour from the home (to be used as a swap)
 */
static bool borg_think_home_buy_swap_armour(void)
{
    int hole;

    int n, b_n = -1;
    int32_t p, b_p = 0L;
    bool fix = false;
    int old_weapon_swap;
    int32_t old_weapon_swap_value;
    int old_armour_swap;
    int32_t old_armour_swap_value;

    /* save the current values */
    old_weapon_swap = weapon_swap;
    old_weapon_swap_value = weapon_swap_value;
    old_armour_swap = armour_swap;
    old_armour_swap_value = armour_swap_value;

    if (armour_swap <= 0 || armour_swap_value <= 0)
    {
        hole = borg_first_empty_inventory_slot();
        armour_swap_value = -1L;
    }
    else
    {
        hole = armour_swap-1;
    }

    if (hole == -1)
        return (false);

    /* Extract the "power" */
    b_p = armour_swap_value;


    /* Scan the home */
    for (n = 0; n < z_info->store_inven_max; n++)
    {
        borg_item* item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Save shop item */
        memcpy(&safe_shops[7].ware[n], &borg_shops[7].ware[n], sizeof(borg_item));

        /* Save hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Remove one item from shop */
        borg_shops[7].ware[n].iqty--;

        /* Move new item into inventory */
        memcpy(&borg_items[hole], &safe_shops[7].ware[n], sizeof(borg_item));

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
        memcpy(&borg_shops[7].ware[n], &safe_shops[7].ware[n], sizeof(borg_item));

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and value */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(true);

    /* Buy something */
    if ((b_n >= 0) && (b_p > armour_swap_value))
    {
        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Restore the values */
        weapon_swap = old_weapon_swap;
        weapon_swap_value = old_weapon_swap_value;
        armour_swap = old_armour_swap;
        armour_swap_value = old_armour_swap_value;

        /* Success */
        return (true);
    }
    /* Restore the values */
    weapon_swap = old_weapon_swap;
    weapon_swap_value = old_weapon_swap_value;
    armour_swap = old_armour_swap;
    armour_swap_value = old_armour_swap_value;

    /* Nope */
    return (false);
}




/*
 * Choose a shop to visit (see above)
 */
static bool borg_choose_shop(void)
{
    /* Must be in town */
    if (borg_skill[BI_CDEPTH]) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (false);
    if (time_this_panel > 1350) return (false);

    /* Already flowing to a store to sell something */
    if (goal_shop != -1 && goal_ware != -1) return (true);

    /* If poisoned or bleeding -- flow to temple */
    if (borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) goal_shop = 3;

    /* If Starving  -- flow to general store */
    if (borg_skill[BI_FOOD] == 0 ||
        (borg_skill[BI_CURLITE] == 0 && borg_skill[BI_CLEVEL] >= 2))
    {
        /* G Store first */
        goal_shop = 0;
    }

    /* Do a quick cheat of the shops and inventory */
    borg_cheat_store();
    borg_notice(true);


    /* if No Lantern -- flow to general store */
    if (borg_skill[BI_CURLITE] == 1 && borg_gold >= 100
        /*  && !borg_shops[0].when */) goal_shop = 0;

    /* If poisoned, bleeding, or needing to shop instantly
     * Buy items straight away, without having to see each shop
     */
    if ((borg_skill[BI_CURLITE] == 0 || borg_skill[BI_FOOD] == 0 ||
        borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) ||
        (borg_skill[BI_CURLITE] == 1 && borg_gold >= 100 && borg_skill[BI_CLEVEL] < 10))
    {
        if (borg_think_shop_buy_aux())
        {
            /* Message */
            borg_note(format("# Buying '%s' at '%s' immediately",
                borg_shops[goal_shop].ware[goal_ware].desc,
                f_info[stores[goal_shop].feat].name));

            /* Set my Quick Shopping flag */
            borg_needs_quick_shopping = true;

            /* Success */
            return (true);
        }

        /* if temple is out of healing stuff, try the house */
        if (borg_think_home_buy_aux())
        {
            /* Message */
            borg_note(format("# Buying '%s' from the home immediately.",
                borg_shops[goal_shop].ware[goal_ware].desc));

            /* Success */
            return (true);
        }
    }

#if 0
    /* Must have visited all shops first---complete information */
    for (i = 0; i < (MAX_STORES); i++)
    {
        borg_shop* shop = &borg_shops[i];

        /* Skip "visited" shops */
        if (!shop->when && !goal_shop) return (false);
    }
#endif

    /* if we are already flowing toward a shop do not check again... */
    if (goal_shop != -1 && goal_ware != -1)
        return true;

    /* Assume no important shop */
    goal_shop = goal_ware = goal_item = -1;
    borg_needs_quick_shopping = false;

    /* if the borg is scumming for cash for the human player and not himself,
     * we dont want him messing with the home inventory
     */
    if (borg_gold < borg_cfg[BORG_MONEY_SCUM_AMOUNT] && 
        borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 &&
        !borg_skill[BI_CDEPTH] && borg_skill[BI_LIGHT] && 
        !borg_cfg[BORG_SELF_SCUM])
    {
        /* Step 0 -- Buy items from the shops (for the player while scumming) */
        if (borg_think_shop_buy_aux())
        {
            /* Message */
            borg_note(format("# Buying '%s' at '%s' (money scumming)",
                borg_shops[goal_shop].ware[goal_ware].desc,
                f_info[stores[goal_shop].feat].name));

            /* Success */
            return (true);
        }
        else return (false);
    }

    /* Step 1 -- Sell items to the home */
    if (borg_think_home_sell_aux(false))
    {
        /* Message */
        if (goal_item != -1)
            borg_note(format("# Selling '%s' to the home",
                borg_items[goal_item].desc));
        else
            borg_note(format("# Buying '%s' from the home (step 1)",
                borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }


    /* Step 2 -- Sell items to the shops */
    if (borg_think_shop_sell_aux())
    {
        /* Message */
        borg_note(format("# Selling '%s' at '%s'",
            borg_items[goal_item].desc,
            f_info[stores[goal_shop].feat].name));

        /* Success */
        return (true);
    }

    /* Step 3 -- Buy items from the shops (for the player) */
    if (borg_think_shop_buy_aux())
    {

        /* Message */
        borg_note(format("# Buying '%s'(%c) at '%s' (for player 'b')",
            borg_shops[goal_shop].ware[goal_ware].desc, shop_menu_items[goal_ware],
            f_info[stores[goal_shop].feat].name));

        /* Success */
        return (true);
    }


    /* Step 4 -- Buy items from the home (for the player) */
    if (borg_think_home_buy_aux())
    {
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
    if (borg_think_home_grab_aux())
    {
        /* Message */
        borg_note(format("# Grabbing (to sell) '%s' from the home",
            borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }

    /* Do not Stock Up the home while money scumming */
    if (borg_cfg[BORG_MONEY_SCUM_AMOUNT]) return (false);

    /* Step 6 -- Buy items from the shops (for the home) */
    if (borg_think_shop_grab_aux())
    {
        /* Message */
        borg_note(format("# Grabbing (for home) '%s' at '%s'",
            borg_shops[goal_shop].ware[goal_ware].desc,
            f_info[stores[goal_shop].feat].name));

        /* Success */
        return (true);
    }

    /* Step 7A -- Buy weapons from the home (as a backup item) */
    if (borg_cfg[BORG_USES_SWAPS] && borg_think_home_buy_swap_weapon())
    {
        /* Message */
        borg_note(format("# Buying '%s' from the home as a backup",
            borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (true);
    }
    /* Step 7B -- Buy armour from the home (as a backup item) */
    if (borg_cfg[BORG_USES_SWAPS] && borg_think_home_buy_swap_armour())
    {
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
 * Sell items to the current shop, if desired
 */
static bool borg_think_shop_sell(void)
{
    int qty = 1;


    /* Sell something if requested */
    if ((goal_shop == shop_num) && (goal_item >= 0))
    {
        borg_item* item = &borg_items[goal_item];

        qty = borg_min_item_quantity(item);

        /* Log */
        borg_note(format("# Selling %s", item->desc));

        /* Sell an item */
        borg_keypress('s');

        /* Sell the desired item */
        borg_keypress(all_letters_nohjkl[goal_item]);

        /* Hack -- Sell a single item */
        if (item->iqty > 1 || qty >= 2)
        {
            if (qty == 5) borg_keypress('5');
            if (qty == 4) borg_keypress('4');
            if (qty == 3) borg_keypress('3');
            if (qty == 2) borg_keypress('2');
            borg_keypress(KC_ENTER);
        }

        /* Mega-Hack -- Accept the price */
        if (goal_shop != 7)
        {
            borg_keypress(KC_ENTER);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
        }

        /* Mark our last item sold */
        if (sold_item_nxt >= 9) sold_item_nxt = 0;
        sold_item_pval[sold_item_nxt] = item->pval;
        sold_item_tval[sold_item_nxt] = item->tval;
        sold_item_sval[sold_item_nxt] = item->sval;
        sold_item_store[sold_item_nxt] = goal_shop;
        sold_item_num = sold_item_nxt;
        sold_item_nxt++;


        /* The purchase is complete */
        goal_shop = goal_ware = goal_item = -1;

        /* tick the anti-loop clock */
        time_this_panel++;

        /* I'm not in a store */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_in_shop = false;
        borg_do_inven = true;
        /* Success */
        return (true);
    }

    /* Nope */
    return (false);
}


/*
 * Buy items from the current shop, if desired
 */
static bool borg_think_shop_buy(void)
{
    char purchase_target = '0';

    /* Buy something if requested */
    if ((goal_shop == shop_num) && (goal_ware >= 0))
    {
        borg_shop* shop = &borg_shops[goal_shop];

        borg_item* item = &shop->ware[goal_ware];

        purchase_target = shop_menu_items[goal_ware];

        /* Paranoid */
        if (item->tval == 0)
        {
            /* The purchase is complete */
            goal_shop = goal_ware = goal_item = -1;

            /* Increment our clock to avoid loops */
            time_this_panel++;

            return (false);
        }

        /* Log */
        borg_note(format("# Buying %s.", item->desc));

        /* Buy the desired item */
        borg_keypress(purchase_target);
        borg_keypress('p');


        /* Mega-Hack -- Accept the price */
        borg_keypress(KC_ENTER);
        borg_keypress(KC_ENTER);
        borg_keypress(' ');
        borg_keypress(' ');
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* if the borg is scumming and bought it.,
         * reset the scum amount.
         */
        if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] &&
            (item->cost >= borg_cfg[BORG_MONEY_SCUM_AMOUNT] * 9 / 10))
        {
            borg_cfg[BORG_MONEY_SCUM_AMOUNT] = 0;

            /* Log */
            borg_note(format("# Setting Money Scum to %d.", borg_cfg[BORG_MONEY_SCUM_AMOUNT]));

        }


        /* Remember what we bought to avoid buy/sell loops */
        if (bought_item_nxt >= 9) bought_item_nxt = 0;
        bought_item_pval[bought_item_nxt] = item->pval;
        bought_item_tval[bought_item_nxt] = item->tval;
        bought_item_sval[bought_item_nxt] = item->sval;
        bought_item_store[bought_item_nxt] = goal_shop;
        bought_item_num = bought_item_nxt;
        bought_item_nxt++;

        /* The purchase is complete */
        goal_shop = goal_ware = goal_item = -1;

        /* Increment our clock to avoid loops */
        time_this_panel++;

        /* leave the store */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* I'm not in a store */
        borg_in_shop = false;

        /* Success */
        return (true);
    }

    /* Nothing to buy */
    return (false);
}


/*
 * Deal with being in a store
 */
bool borg_think_store(void)
{
    /* Hack -- prevent clock wrapping */
    if (borg_t >= 20000 && borg_t <= 20010)
    {
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
    if (borg_best_stuff()) return (true);

    /* If using a digger, Wear "useful" equipment.
     * unless that digger is an artifact, then treat
     * it as a normal weapon
     */
    if (borg_items[INVEN_WIELD].tval == TV_DIGGING &&
        !borg_items[INVEN_WIELD].art_idx &&
        borg_wear_stuff()) return (true);

    /* Choose a shop to visit.  Goal_shop indicates he is trying to sell something somewhere. */
    if (borg_choose_shop())
    {
        /* Note Pref. */
        if (shop_num != goal_shop)
            borg_note(format("# Currently in store '%d' would prefer '%d'.", shop_num + 1, goal_shop + 1));
        else
            borg_note(format("# Currently in prefered store '%d'.", goal_shop + 1));


        /* Try to sell stuff */
        if (borg_think_shop_sell()) return (true);

        /* Try to buy stuff */
        if (borg_think_shop_buy()) return (true);
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

/* Attempt a series of maneuvers to stay alive when you run out of light */
static bool borg_think_dungeon_light(void)
{
    int ii, x, y;
    bool not_safe = false;
    borg_grid* ag;

    /* Consume needed things */
    if (borg_skill[BI_ISHUNGRY] && borg_use_things()) return (true);

    if (!borg_skill[BI_LIGHT] &&
        (borg_skill[BI_CURLITE] <= 0 || borg_items[INVEN_LIGHT].timeout <= 3) &&
        borg_skill[BI_CDEPTH] >= 1)
    {
        enum borg_need need;

        /* I am recalling, sit here till it engages. */
        if (goal_recalling)
        {
            /* just wait */
            borg_keypress('R');
            borg_keypress('9');
            borg_keypress(KC_ENTER);
            return (true);
        }

        /* wear stuff and see if it glows */
        if (borg_wear_stuff()) return (true);
// Things are now automatically put in the quiver !FIX !TODO !AJG double check
//        if (borg_wear_quiver()) return (true);

        /* attempt to refuel/swap */
        need = borg_maintain_light();
        if (need == BORG_MET_NEED) return (true);
        if (need == BORG_NO_NEED) return (false);

        /* Can I recall out with a rod */
        if (!goal_recalling && borg_zap_rod(sv_rod_recall)) return (true);

        /* Can I recall out with a spell */
        if (!goal_recalling && borg_recall()) return (true);

        /* Log */
        borg_note("# Testing for stairs .");

        /* Test for stairs */
        borg_keypress('<');

        /* If on a glowing grid, got some food, and low mana, then rest here */
        if ((borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] && borg_skill[BI_MAXSP] > 0) &&
            (borg_grids[c_y][c_x].info & BORG_GLOW) &&
            !borg_skill[BI_ISWEAK] &&
            (borg_spell_legal(HERBAL_CURING) || borg_spell_legal(REMOVE_HUNGER) ||
                borg_skill[BI_FOOD] >= borg_skill[BI_CDEPTH]) &&
            (borg_spell_legal(CALL_LIGHT) || borg_spell_legal(LIGHT_ROOM)) &&
            (!borg_spell_okay(CALL_LIGHT) && !borg_spell_okay(LIGHT_ROOM)))
        {
            /* Scan grids adjacent to me */
            for (ii = 0; ii < 8; ii++)
            {
                x = c_x + ddx_ddd[ii];
                y = c_y + ddy_ddd[ii];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(x, y))) continue;

                /* Access the grid */
                ag = &borg_grids[y][x];

                /* Check for adjacent Monster */
                if (ag->kill)
                {
                    not_safe = true;
                }
            }

            /* Be concerned about the Regional Fear. */
            if (borg_fear_region[c_y / 11][c_x / 11] > borg_skill[BI_CURHP] / 10) not_safe = true;

            /* Be concerned about the Monster Fear. */
            if (borg_fear_monsters[c_y][c_x] > borg_skill[BI_CURHP] / 10) not_safe = true;

            /* rest here to gain some mana */
            if (!not_safe)
            {
                borg_note("# Resting on this Glowing Grid to gain mana.");
                borg_keypress('R');
                borg_keypress('*');
                borg_keypress(KC_ENTER);
                return (true);
            }
        }

        /* If I have the capacity to Call Light, then do so if adjacent to a dark grid.
         * We can illuminate the entire dungeon, looking for stairs.
         */
         /* Scan grids adjacent to me */
        for (ii = 0; ii < 8; ii++)
        {
            x = c_x + ddx_ddd[ii];
            y = c_y + ddy_ddd[ii];


            /* Bounds check */
            if (!square_in_bounds_fully(cave, loc(x, y))) continue;

            /* Access the grid */
            ag = &borg_grids[y][x];

            /* skip the Wall grids */
            if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_PERM) continue;

            /* Problem with casting Call Light on Open Doors */
            if ((ag->feat == FEAT_OPEN || ag->feat == FEAT_BROKEN) &&
                (y == c_y && x == c_x))
            {
                /* Cheat the grid info to see if the door is lit */
                if (square_isglow(cave, loc(c_x, c_y))) ag->info |= BORG_GLOW;
                continue;
            }

            /* Look for a dark one */
            if ((ag->info & BORG_DARK) || /* Known to be dark */
                ag->feat == FEAT_NONE || /* Nothing known about feature */
                !(ag->info & BORG_MARK) || /* Nothing known about info */
                !(ag->info & BORG_GLOW))   /* not glowing */
            {
                /* Attempt to Call Light */
                if (borg_activate_item(act_illumination) ||
                    borg_activate_item(act_light) ||
                    borg_zap_rod(sv_rod_illumination) ||
                    borg_use_staff(sv_staff_light) ||
                    borg_read_scroll(sv_scroll_light) ||
                    borg_spell(CALL_LIGHT) ||
                    borg_spell(LIGHT_ROOM))
                {
                    borg_note("# Illuminating the region while dark.");
                    borg_react("SELF:lite", "SELF:lite");

                    return (true);
                }

                /* Attempt to use Light Beam requiring a direction. */
                if (borg_LIGHT_beam(false)) return (true);
            }
        }


        /* Try to flow to upstairs if on level one */
        if (borg_flow_stair_less(GOAL_FLEE, false))
        {
            /* Take the stairs */
            /* Log */
            borg_note("# Taking up Stairs stairs (low Light).");
            borg_keypress('<');
            return (true);
        }

        /* Try to flow to a lite */
        if (borg_skill[BI_RECALL] && borg_flow_light(GOAL_FLEE))
        {
            return (true);
        }

    }
    /* Nothing to do */
    return (false);
}
/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to grab items close to the stair.
 */
static bool borg_think_stair_scum(bool from_town)
{
    int j, b_j = -1;
    int i;

    borg_grid* ag = &borg_grids[c_y][c_x];

    uint8_t feat = square(cave, loc(c_x, c_y))->feat;

    enum borg_need need;

    /* examine equipment and swaps */
    borg_notice(true);

    /* No scumming mode if starving or in town */
    if (borg_skill[BI_CDEPTH] == 0 ||
        borg_skill[BI_ISWEAK])
    {
        borg_note("# Leaving Scumming Mode. (Town or Weak)");
        borg_lunal_mode = false;
        return (false);
    }

    /* No scumming if inventory is full.  Require one empty slot */
    if (borg_items[PACK_SLOTS - 1].iqty) return (false);

    /* if borg is just starting on this level, he may not
     * know that a stair is under him.  Cheat to see if one is
     * there
     */
    if (feat == FEAT_MORE && ag->feat != FEAT_MORE)
    {

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            /* We already knew about that one */
            if ((track_more.x[i] == c_x) && (track_more.y[i] == c_y)) break;
        }

        /* Track the newly discovered "down stairs" */
        if ((i == track_more.num) && (i < track_more.size))
        {
            track_more.x[i] = c_x;
            track_more.y[i] = c_y;
            track_more.num++;
        }
        /* tell the array */
        ag->feat = FEAT_MORE;

    }

    if (feat == FEAT_LESS && ag->feat != FEAT_LESS)
    {

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            /* We already knew about this one */
            if ((track_less.x[i] == c_x) && (track_less.y[i] == c_y)) continue;
        }

        /* Track the newly discovered "up stairs" */
        if ((i == track_less.num) && (i < track_less.size))
        {
            track_less.x[i] = c_x;
            track_less.y[i] = c_y;
            track_less.num++;
        }

        /* Tell the array */
        ag->feat = FEAT_LESS;

    }

    /** First deal with staying alive **/

    /* Hack -- require light */
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note("# Scum. (need fuel)");

    /** Track down some interesting gear **/
/* XXX Should we allow him great flexibility in retreiving loot? (not always safe?)*/
    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a (viewable) object */
    if (borg_flow_take_scum(true, 6)) return (true);

    /*leave level right away. */
    borg_note("# Fleeing level. Scumming Mode");
    goal_fleeing = true;

    /* Scumming Mode - Going down */
    if (track_more.num &&
        (ag->feat == FEAT_MORE ||
            ag->feat == FEAT_LESS ||
            borg_skill[BI_CDEPTH] < 30))
    {
        int y, x;

        if (track_more.num >= 2) borg_note("# Scumming Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            x = track_more.x[i];
            y = track_more.y[i];

            /* How far is the nearest down stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }


        /* if the downstair is close and path is safe, continue on */
        if (b_j < 8 ||
            ag->feat == FEAT_MORE ||
            borg_skill[BI_CDEPTH] < 30)
        {
            /* Note */
            borg_note("# Scumming Mode.  Power Diving. ");

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to DownStair */
            if (borg_flow_stair_more(GOAL_FLEE, false, false)) return (true);

            /* if standing on a stair */
            if (ag->feat == FEAT_MORE)
            {
                /* Take the DownStair */
                borg_on_upstairs = true;
                borg_keypress('>');

                return (true);
            }
        }
    }

    /* Scumming Mode - Going up */
    if (track_less.num && borg_skill[BI_CDEPTH] != 1 &&
        (ag->feat == FEAT_MORE ||
            ag->feat == FEAT_LESS))
    {
        int y, x;

        borg_grid* tmp_ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }

        /* if the upstair is close and safe path, continue */
        if (b_j < 8 ||
            tmp_ag->feat == FEAT_LESS)
        {

            /* Note */
            borg_note("# Scumming Mode.  Power Climb. ");

            /* Set to help borg move better */
            goal_less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                borg_note("# Looking for stairs. Scumming Mode.");

                /* Success */
                return (true);
            }

            if (tmp_ag->feat == FEAT_LESS)
            {
                /* Take the Up Stair */
                borg_on_dnstairs = true;
                borg_keypress('<');
                return (true);
            }

        }
    }

    /* Special case where the borg is off a stair and there
     * is a monster in LOS.  He could freeze and unhook, or
     * move to the closest stair and risk the run.
     */
    if (borg_skill[BI_CDEPTH] >= 2)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Note */
        borg_note("# Scumming Mode.  Any Stair. ");

        /* Try to find some stairs */
        if (borg_flow_stair_both(GOAL_FLEE, true)) return (true);
    }

    /* return to normal borg_think_dungeon */
    return (false);
}

/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to get to the bottom of the dungeon asap.
 * Once down there, he can be told to do something.
 *
 * Dive if stairs are close and safe.
 * Retreat off level if monster has LOS to borg.
 * Fill Lantern
 * Eat food
 * Call light.  might be dangerous because monster get a chance to hit us.
 */
static bool borg_think_dungeon_lunal(void)
{
    bool safe_place = false;

    int j, b_j = -1;
    int i;

    borg_grid* ag = &borg_grids[c_y][c_x];

    uint8_t feat = square(cave, loc(c_x, c_y))->feat;

    enum borg_need need;

    /* examine equipment and swaps */
    borg_notice(true);

    /* No Lunal mode if starving or in town */
    if (borg_skill[BI_CDEPTH] == 0 ||
        borg_skill[BI_ISWEAK])
    {
        borg_note("# Leaving Lunal Mode. (Town or Weak)");
        borg_lunal_mode = false;
        return (false);
    }

    /* if borg is just starting on this level, he may not
     * know that a stair is under him.  Cheat to see if one is
     * there
     */
    if (feat == FEAT_MORE && ag->feat != FEAT_MORE)
    {

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            /* We already knew about that one */
            if ((track_more.x[i] == c_x) && (track_more.y[i] == c_y)) break;
        }

        /* Track the newly discovered "down stairs" */
        if ((i == track_more.num) && (i < track_more.size))
        {
            track_more.x[i] = c_x;
            track_more.y[i] = c_y;
            track_more.num++;
        }
        /* tell the array */
        ag->feat = FEAT_MORE;

    }

    if (feat == FEAT_LESS && ag->feat != FEAT_LESS)
    {

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            /* We already knew about this one */
            if ((track_less.x[i] == c_x) && (track_less.y[i] == c_y)) continue;
        }

        /* Track the newly discovered "up stairs" */
        if ((i == track_less.num) && (i < track_less.size))
        {
            track_less.x[i] = c_x;
            track_less.y[i] = c_y;
            track_less.num++;
        }

        /* Tell the array */
        ag->feat = FEAT_LESS;

    }

    /* Act normal on 1 unless stairs are seen*/
    if (borg_skill[BI_CDEPTH] == 1 && track_more.num == 0)
    {
        borg_lunal_mode = false;
        return (false);
    }

    /* If no down stair is known, act normal */
    if (track_more.num == 0 && track_less.num == 0)
    {
        borg_note("# Leaving Lunal Mode. (No Stairs seen)");
        borg_lunal_mode = false;
        return (false);
    }

    /* If self scumming and getting closer to zone, act normal */
    if (borg_cfg[BORG_SELF_LUNAL])
    {
        if (borg_skill[BI_MAXDEPTH] <= borg_skill[BI_CDEPTH] + 15 ||
            (char*)NULL != borg_prepared(borg_skill[BI_CDEPTH] - 5) ||
            borg_skill[BI_CDEPTH] >= 50 ||
            borg_skill[BI_CDEPTH] == 0 ||
            borg_skill[BI_ISWEAK])
        {
            borg_lunal_mode = false;
            goal_fleeing = false;
            goal_fleeing_lunal = false;
            borg_note("# Self Lunal mode disengaged normally.");
            return (false);
        }
    }


    /** First deal with staying alive **/

    /* Hack -- require light */
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note("# Lunal. (need fuel)");

    /* No Light at all */
    if (borg_skill[BI_CURLITE] == 0 && borg_items[INVEN_LIGHT].tval == 0)
    {
        borg_note("# No Light at all.");
        return (false);
    }

    /* Define if safe_place is true or not */
    safe_place = borg_check_rest(c_y, c_x);

    /* Light Room, looking for monsters */
    /* if (safe_place && borg_check_LIGHT_only()) return (true); */

    /* Check for stairs and doors and such */
    /* if (safe_place && borg_check_LIGHT()) return (true); */

    /* Recover from any nasty condition */
    if (safe_place && borg_recover()) return (true);

    /* Consume needed things */
    if (safe_place && borg_use_things()) return (true);

    /* Consume needed things */
    if (borg_skill[BI_ISHUNGRY] && borg_use_things()) return (true);

    /* Crush junk if convienent */
    if (safe_place && borg_crush_junk()) return (true);

    /** Track down some interesting gear **/
/* XXX Should we allow him great flexibility in retreiving loot? (not always safe?)*/
    /* Continue flowing towards objects */
    if (safe_place && borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a (viewable) object */
    if (safe_place && borg_flow_take_lunal(true, 4)) return (true);

    /*leave level right away. */
    borg_note("# Fleeing level. Lunal Mode");
    goal_fleeing_lunal = true;
    goal_fleeing = true;

    /* Full of Items - Going up */
    if (track_less.num && borg_items[PACK_SLOTS - 2].iqty &&
        (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS))
    {
        int y, x;
        int closeness = 8;

        borg_grid* tmp_ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }

        /* if on depth 1, try to venture more to get back to town */
        if (borg_skill[BI_CDEPTH] == 1)
        {
            if (track_less.num)
            {
                closeness = 20;
            }
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
            tmp_ag->feat == FEAT_LESS)
        {

            /* Note */
            borg_note("# Lunal Mode.  Power Climb (needing to sell). ");

            /* Set to help borg move better */
            goal_less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                borg_note("# Looking for stairs. Lunal Mode (needing to sell).");

                /* Success */
                return (true);
            }

            if (tmp_ag->feat == FEAT_LESS)
            {
                /* Take the Up Stair */
                borg_on_dnstairs = true;
                borg_keypress('<');
                return (true);
            }

        }
    }

    /* Lunal Mode - Going down */
    if (track_more.num &&
        (safe_place || ag->feat == FEAT_MORE ||
            ag->feat == FEAT_LESS ||
            borg_skill[BI_CDEPTH] < 30))
    {
        int y, x;

        if (track_more.num >= 2) borg_note("# Lunal Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            x = track_more.x[i];
            y = track_more.y[i];

            /* How far is the nearest down stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }


        /* if the downstair is close and path is safe, continue on */
        if ((b_j < 8 && safe_place) ||
            ag->feat == FEAT_MORE ||
            borg_skill[BI_CDEPTH] < 30)
        {
            /* Note */
            borg_note("# Lunal Mode.  Power Diving. ");

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to DownStair */
            if (borg_flow_stair_more(GOAL_FLEE, true, false)) return (true);

            /* if standing on a stair */
            if (ag->feat == FEAT_MORE)
            {
                /* Take the DownStair */
                borg_on_upstairs = true;
                borg_keypress('>');

                return (true);
            }
        }
    }

    /* Lunal Mode - Going up */
    if (track_less.num && borg_skill[BI_CDEPTH] != 1 &&
        (safe_place || ag->feat == FEAT_MORE ||
            ag->feat == FEAT_LESS))
    {
        int y, x;

        borg_grid* tmp_ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < 8 && safe_place) ||
            tmp_ag->feat == FEAT_LESS)
        {

            /* Note */
            borg_note("# Lunal Mode.  Power Climb. ");

            /* Set to help borg move better */
            goal_less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                borg_note("# Looking for stairs. Lunal Mode.");

                /* Success */
                return (true);
            }

            if (tmp_ag->feat == FEAT_LESS)
            {
                /* Take the Up Stair */
                borg_on_dnstairs = true;
                borg_keypress('<');
                return (true);
            }

        }
    }

    /* Special case where the borg is off a stair and there
     * is a monster in LOS.  He could freeze and unhook, or
     * move to the closest stair and risk the run.
     */
    if (borg_skill[BI_CDEPTH] >= 2)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Note */
        borg_note("# Lunal Mode.  Any Stair. ");

        /* Try to find some stairs */
        if (borg_flow_stair_both(GOAL_FLEE, true)) return (true);
    }


    /* Lunal Mode - Reached 99 */
    if (borg_skill[BI_CDEPTH] == 99)
    {
        borg_note("# Lunal Mode ended at depth.");
    }

    /* Unable to do it */
    if (borg_skill[BI_CDEPTH] > 1)
    {
        borg_note("# Lunal Mode ended incorrectly.");
    }

    /* return to normal borg_think_dungeon */
    borg_note("Leaving Lunal Mode. (End of Lunal Mode)");
    borg_lunal_mode = false;
    goal_fleeing = goal_fleeing_lunal = false;
    return (false);
}

/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to get to a sweet spot to gather items.
 *
 * Dive if stairs are close and safe.
 * Retreat off level if monster has LOS to borg.
 * Fill Lantern
 * Eat food
 * Call light.  might be dangerous because monster get a chance to hit us.
 */
static bool borg_think_dungeon_munchkin(void)
{
    bool safe_place = false;
    int bb_j = z_info->max_range;
    int j, b_j = -1;
    int i, ii, x, y;
    int closeness = 8;

    borg_grid* ag = &borg_grids[c_y][c_x];

    uint8_t feat = square(cave, loc(c_x, c_y))->feat;

    enum borg_need need;

    /* examine equipment and swaps */
    borg_notice(true);

    /* Not if starving or in town */
    if (borg_skill[BI_CDEPTH] == 0 ||
        borg_skill[BI_ISWEAK])
    {
        borg_note("# Leaving munchkin Mode. (Town or Weak)");
        borg_munchkin_mode = false;
        return (false);
    }

    /* if borg is just starting on this level, he may not
     * know that a stair is under him.  Cheat to see if one is
     * there
     */
    if (feat == FEAT_MORE && ag->feat != FEAT_MORE)
    {

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            /* We already knew about that one */
            if ((track_more.x[i] == c_x) && (track_more.y[i] == c_y)) break;
        }

        /* Track the newly discovered "down stairs" */
        if ((i == track_more.num) && (i < track_more.size))
        {
            track_more.x[i] = c_x;
            track_more.y[i] = c_y;
            track_more.num++;
        }
        /* tell the array */
        ag->feat = FEAT_MORE;

    }

    if (feat == FEAT_LESS && ag->feat != FEAT_LESS)
    {

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            /* We already knew about this one */
            if ((track_less.x[i] == c_x) && (track_less.y[i] == c_y)) continue;
        }

        /* Track the newly discovered "up stairs" */
        if ((i == track_less.num) && (i < track_less.size))
        {
            track_less.x[i] = c_x;
            track_less.y[i] = c_y;
            track_less.num++;
        }

        /* Tell the array */
        ag->feat = FEAT_LESS;

    }

    /* Act normal on 1 unless stairs are seen*/
    if (borg_skill[BI_CDEPTH] == 1 && track_more.num == 0)
    {
        borg_munchkin_mode = false;
        return (false);
    }

    /* If no down stair is known, act normal */
    if (track_more.num == 0 && track_less.num == 0)
    {
        borg_note("# Leaving Munchkin Mode. (No Stairs seen)");
        borg_munchkin_mode = false;
        return (false);
    }

    /** First deal with staying alive **/

    /* Hack -- require light */
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note("# Munchkin. (need fuel)");

    /* No Light at all */
    if (borg_skill[BI_CURLITE] == 0)
    {
        borg_note("# No Light at all.");
    }

    /* Define if safe_place is true or not */
    safe_place = borg_check_rest(c_y, c_x);

    /* Can do a little attacking. */
    if (borg_munchkin_mage()) return (true);
    if (borg_munchkin_melee()) return (true);

    /* Consume needed things */
    if (safe_place && borg_use_things()) return (true);

    /* Consume needed things */
    if (borg_skill[BI_ISHUNGRY] && borg_use_things()) return (true);

    /* Wear stuff and see if it's good */
    if (safe_place && borg_wear_stuff()) return (true);
// Things are now automatically put in the quiver !FIX !TODO !AJG double check
//    if (safe_place && borg_wear_quiver()) return (true);
    if (safe_place && borg_remove_stuff()) return (true);

    /* Crush junk if convienent */
    if (safe_place && borg_crush_junk()) return (true);

    /* Learn learn and test useful spells */
    if (safe_place && borg_play_magic(true)) return (true);

    /** Track down some interesting gear **/
/* XXX Should we allow him great flexibility in retreiving loot? (not always safe?)*/
    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Borg may be off the stair and a monster showed up. */

    /* Find a (viewable) object */
    if (safe_place && borg_flow_take_lunal(true, 5)) return (true);

    /* Recover from any nasty condition */
    if (safe_place && borg_recover()) return (true);

    /*leave level right away. */
    borg_note("# Fleeing level. Munchkin Mode");
    goal_fleeing_munchkin = true;
    goal_fleeing = true;

    /* Increase the range of the borg a bit */
    if (borg_skill[BI_CDEPTH] <= 10) closeness += (borg_skill[BI_CLEVEL] - 10) +
        (10 - borg_skill[BI_CDEPTH]);

    /* Full of Items - Going up */
    if (track_less.num && (borg_items[PACK_SLOTS - 2].iqty) &&
        (safe_place || ag->feat == FEAT_LESS || borg_skill[BI_CURLITE] == 0))
    {
        borg_grid* tmp_ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }

        /* if on depth 1, try to venture more to get back to town */
        if (borg_skill[BI_CDEPTH] == 1)
        {
            if (track_less.num)
            {
                closeness = 20;
            }
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
            tmp_ag->feat == FEAT_LESS)
        {

            /* Note */
            borg_note("# Munchkin Mode.  Power Climb (needing to sell). ");

            /* Set to help borg move better */
            goal_less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, true))
            {
                borg_note("# Looking for stairs. Munchkin Mode (needing to sell).");

                /* Success */
                return (true);
            }

            if (tmp_ag->feat == FEAT_LESS)
            {
                /* Take the Up Stair */
                borg_on_dnstairs = true;
                borg_keypress('<');
                return (true);
            }

        }
    }

    /* Too deep. trying to gradually move shallow.  Going up */
    if ((track_less.num && borg_skill[BI_CDEPTH] > borg_cfg[BORG_MUNCHKIN_DEPTH]) && 
        (safe_place || ag->feat == FEAT_LESS))
    {

        borg_grid* tmp_ag = &borg_grids[c_y][c_x];

        /* Reset */
        b_j = -1;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
            if (b_j < bb_j) bb_j = b_j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
            tmp_ag->feat == FEAT_LESS)
        {

            /* Note */
            borg_note("# Munchkin Mode.  Power Climb. ");

            /* Set to help borg move better */
            goal_less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, true))
            {
                borg_note("# Looking for stairs. Munchkin Mode.");

                /* Success */
                return (true);
            }

            if (tmp_ag->feat == FEAT_LESS)
            {
                /* Take the Up Stair */
                borg_on_dnstairs = true;
                borg_keypress('<');
                return (true);
            }

        }
    }

    /* Going down */
    if ((track_more.num && borg_skill[BI_CDEPTH] < borg_cfg[BORG_MUNCHKIN_DEPTH]) &&
        (safe_place || ag->feat == FEAT_MORE))
    {
        /* Reset */
        b_j = -1;

        if (track_more.num >= 1) borg_note("# Munchkin Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            x = track_more.x[i];
            y = track_more.y[i];

            /* How far is the nearest down stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }


        /* if the downstair is close and path is safe, continue on */
        if ((b_j < closeness && safe_place) || ag->feat == FEAT_MORE || borg_skill[BI_CDEPTH] == 1)
        {
            /* Note */
            borg_note("# Munchkin Mode.  Power Diving. ");

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to DownStair */
            if (borg_flow_stair_more(GOAL_FLEE, true, false)) return (true);

            /* if standing on a stair */
            if (ag->feat == FEAT_MORE)
            {
                /* Take the DownStair */
                borg_on_upstairs = true;
                borg_keypress('>');

                return (true);
            }
        }
    }

    /* Going up */
    if ((track_less.num && borg_skill[BI_CDEPTH] != 1 &&
        safe_place) || ag->feat == FEAT_LESS)
    {
        borg_grid* tmp_ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
            tmp_ag->feat == FEAT_LESS)
        {

            /* Note */
            borg_note("# Munchkin Mode.  Power Climb. ");

            /* Set to help borg move better */
            goal_less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE)) return (true);

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, true))
            {
                borg_note("# Looking for stairs. Munchkin Mode.");

                /* Success */
                return (true);
            }

            if (tmp_ag->feat == FEAT_LESS)
            {
                /* Take the Up Stair */
                borg_on_dnstairs = true;
                borg_keypress('<');
                return (true);
            }

        }
    }

    /* Special case where the borg is off a stair and there
     * is a monster in LOS.  He could freeze and unhook, or
     * move to the closest stair and risk the run.
     */
    if (borg_skill[BI_CDEPTH] >= 2 || !safe_place)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Note */
        borg_note("# Munchkin Mode.  Any Stair. ");


        /* Adjacent Monster.  Either attack it, or try to outrun it */
        for (i = 0; i < 8; i++)
        {
            y = c_y + ddy_ddd[i];
            x = c_x + ddx_ddd[i];

            /* Bounds check */
            if (!square_in_bounds(cave, loc(x, y))) continue;

            /* Get the grid */
            ag = &borg_grids[y][x];

            /* Monster is adjacent to the borg */
            if (ag->kill)
            {
                /* Check for an existing "up stairs" */
                for (ii = 0; ii < track_less.num; ii++)
                {
                    x = track_less.x[ii];
                    y = track_less.y[ii];

                    /* How far is the nearest up stairs */
                    j = borg_distance(c_y, c_x, y, x);

                    /* Is it reachable or behind a wall? */
                    if (!borg_projectable(y, x, c_y, c_x)) continue;

                    /* skip the far ones */
                    if (b_j <= j && b_j != -1) continue;

                    /* track it */
                    b_j = j;
                }

                /* Check for an existing "down stairs" */
                for (ii = 0; ii < track_more.num; ii++)
                {
                    x = track_more.x[ii];
                    y = track_more.y[ii];

                    /* How far is the nearest down stairs */
                    j = borg_distance(c_y, c_x, y, x);

                    /* Is it reachable or behind a wall? */
                    if (!borg_projectable(y, x, c_y, c_x)) continue;

                    /* skip the far ones */
                    if (b_j <= j && b_j != -1) continue;

                    /* track it */
                    b_j = j;
                }

                /* Can the borg risk the run? */
                if (b_j <= 3)
                {
                    /* Try to find some stairs */
                    if (borg_flow_stair_both(GOAL_FLEE, false)) return (true);
                }
                else
                {
                    /* Try to kill it */
                    if (borg_attack(false)) return (true);
                }
            } /* Adjacent to kill */
        } /* Scanning neighboring grids */

        /* Try to find some stairs */
        if (borg_flow_stair_both(GOAL_FLEE, false)) return (true);
        if (ag->feat == FEAT_LESS)
        {
            /* Take the Up Stair */
            borg_on_dnstairs = true;
            borg_keypress('<');
            return (true);
        }
        if (ag->feat == FEAT_MORE)
        {
            /* Take the Stair */
            borg_on_upstairs = true;
            borg_keypress('>');
            return (true);
        }
    }

    /* return to normal borg_think_dungeon */
    borg_note("Leaving Munchkin Mode. (End of Mode)");
    borg_munchkin_mode = false;
    goal_fleeing = goal_fleeing_munchkin = false;
    return (false);
}

/*
 * Hack -- perform an action in the dungeon under boosted bravery
 *
 * This function is a sub-set of the standard dungeon goals, and is
 * only executed when all of the standard dungeon goals fail, because
 * of excessive danger, or because the level is "bizarre".
 */
static bool borg_think_dungeon_brave(void)
{
    /*** Local stuff ***/
    int p1 = borg_danger(c_y, c_x, 1, true, false);

    /* Try a defence manuever on 100 */
    if (borg_skill[BI_CDEPTH] == 100 &&
        borg_defend(p1)) return true;

    /* Attack monsters */
    if (borg_attack(true)) return (true);

    /* Cast a light beam to remove fear of an area */
    if (borg_LIGHT_beam(false)) return (true);

    /*** Flee (or leave) the level ***/

    /* Take stairs down */
    /* Usable stairs */
    if (borg_grids[c_y][c_x].feat == FEAT_MORE)
    {
        /* Take the stairs */
        borg_on_upstairs = true;
        borg_note("# Fleeing via stairs.");
        borg_keypress('>');

        /* Success */
        return (true);
    }

    /* Return to Stairs, but not use them */
    if (goal_less)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs */
        if (scaryguy_on_level && !borg_skill[BI_CDEPTH] && borg_flow_stair_both(GOAL_FLEE, false)) return (true);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, false))
        {
            borg_note("# Looking for stairs. Goal_less, brave.");
            return (true);
        }
    }


    /* Flee the level */
    if (goal_fleeing || goal_leaving || scaryguy_on_level)
    {
        /* Hack -- Take the next stairs */
        stair_less = goal_fleeing;

        if (borg_ready_morgoth == 0)
            stair_less = true;

        if (stair_less == true)
        {
            borg_note("# Fleeing and leaving the level. Brave Thinking.");
        }

        /* Go down if fleeing or prepared. */
        stair_more = goal_fleeing;
        if ((char*)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 1))
            stair_more = true;

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs up */
        if (stair_less)
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                borg_note("# Looking for stairs. Flee, brave.");
                return (true);
            }

        /* Try to find some stairs down */
        if (stair_more)
            if (borg_flow_stair_more(GOAL_FLEE, false, true)) return (true);

    }

    /* Do short looks on special levels */
    if (vault_on_level)
    {
        /* Continue flowing towards monsters */
        if (borg_flow_old(GOAL_KILL)) return (true);

        /* Find a (viewable) monster */
        if (borg_flow_kill(true, 35)) return (true);

        /* Continue flowing towards objects */
        if (borg_flow_old(GOAL_TAKE)) return (true);

        /* Find a (viewable) object */
        if (borg_flow_take(true, 35)) return (true);
        if (borg_flow_vein(true, 35)) return (true);

        /* Continue to dig out a vault */
        if (borg_flow_old(GOAL_VAULT)) return (true);

        /* Find a vault to excavate */
        if (borg_flow_vault(35)) return (true);
    }

    /* Continue flowing towards monsters */
    if (borg_flow_old(GOAL_KILL)) return (true);

    /* Find a (viewable) monster */
    if (borg_flow_kill(true, 250)) return (true);

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a (viewable) object */
    if (borg_flow_take(true, 250)) return (true);
    if (borg_flow_vein(true, 250)) return (true);

    /*** Exploration ***/

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_MISC)) return (true);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_DARK)) return (true);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_XTRA)) return (true);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_BORE)) return (true);


    /*** Explore the dungeon ***/

    /* Explore interesting grids */
    if (borg_flow_dark(true)) return (true);

    /* Explore interesting grids */
    if (borg_flow_dark(false)) return (true);

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
        if (borg_check_LIGHT()) return (true);
    }

    /*** Track down old stuff ***/

    /* Chase old objects */
    if (borg_flow_take(false, 250)) return (true);
    if (borg_flow_vein(false, 250)) return (true);

    /* Chase old monsters */
    if (borg_flow_kill(false, 250)) return (true);

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
        if (borg_check_LIGHT()) return (true);
    }

    /* Attempt to leave the level */
    if (borg_leave_level(true)) return (true);

    /* Search for secret doors */
    if (borg_flow_spastic(true)) return (true);


    /* Nothing */
    return (false);
}


/*
 * Perform an action in the dungeon
 *
 * Return true if a "meaningful" action was performed
 * Otherwise, return false so we will be called again
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 *   Explore the dungeon and revisit old grids
 *
 * Fleeing:
 *   Use word of recall when level is "scary"
 *   Flee to stairs when there is a chance of death
 *   Avoid "stair bouncing" if at all possible
 *
 * Note that the various "flow" actions allow the Borg to flow
 * "through" closed doors, which will be opened when he attempts
 * to pass through them, so we do not have to pursue terrain until
 * all monsters and objects have been dealt with.
 *
 * XXX XXX XXX The poor Borg often kills a nasty monster, and
 * then takes a nap to recover from damage, but gets yanked
 * back to town before he can collect his reward.
 */
bool borg_think_dungeon(void)
{
    int i, j;
    int b_j = -1;

    /* Delay Factor */
    int msec = ((player->opts.delay_factor * player->opts.delay_factor) +
        (borg_cfg[BORG_DELAY_FACTOR] * borg_cfg[BORG_DELAY_FACTOR]));

    /* HACK allows user to stop the borg on certain levels */
    if (borg_skill[BI_CDEPTH] == borg_cfg[BORG_STOP_DLEVEL]) 
        borg_oops("Auto-stop for user DLevel.");

    if (borg_skill[BI_CLEVEL] == borg_cfg[BORG_STOP_CLEVEL]) 
        borg_oops("Auto-stop for user CLevel.");

    /* HACK to end all hacks,,, allow the borg to stop if money scumming */
    if (borg_gold > borg_cfg[BORG_MONEY_SCUM_AMOUNT] && 
        borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 &&
        !borg_skill[BI_CDEPTH] && 
        !borg_cfg[BORG_SELF_SCUM])
    {
        borg_oops("Money Scum complete.");
    }

    /* Hack -- Stop the borg if money scumming and the shops are out of food. */
    if (!borg_skill[BI_CDEPTH] && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 &&
        (borg_food_onsale == 0 && borg_skill[BI_FOOD] < 5))
    {
        /* Town out of food.  If player initiated borg, stop here */
        if (borg_cfg[BORG_SELF_SCUM] == false)
        {
            borg_oops("Money Scum stopped.  No more food in shop.");
            return (true);
        }
        else
            /* Borg doing it himself */
        {
            /* move money goal to 0 and leave the level */
            borg_cfg[BORG_MONEY_SCUM_AMOUNT] = 0;
        }
    }

    /* Hack -- prevent clock wrapping Step 1*/
    if ((borg_t >= 12000 && borg_t <= 12025) ||
        (borg_t >= 25000 && borg_t <= 25025))
    {
        /* Clear Possible errors */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Re-examine inven and equip */
        borg_do_inven = true;
        borg_do_equip = true;

        /* enter a special routine to handle this behavior.  Messing with
         * the old_level forces him to re-explore this level, and reshop,
         * if in town.
         */
        old_depth = 126;

        /* Continue on */
        return (true);
    }

    /* Hack -- prevent clock wrapping Step 2*/
    if (borg_t >= 30000)
    {
        /* Panic */
        borg_oops("clock overflow");

#ifdef BABLOS
        /* Clock overflow escape code */
        printf("Clock overflow code!\n");
        player->playing = false;
        player->leaving = true;
        borg_clock_over = true;
#endif /* BABLOS */

        /* Oops */
        return (true);
    }

    /* Allow respawning borgs to update their variables */
    if (borg_respawning > 1)
    {
        borg_note(format("# Pressing 'escape' to catch up and get in sync (%d).", borg_respawning));
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_respawning--;
        return (true);
    }

    /* add a short pause to slow the borg down for viewing */
    Term_xtra(TERM_XTRA_DELAY, msec);

    /* redraw the screen if we need to */
    if (my_need_redraw)
    {
        borg_note(format("#  Redrawing screen."));
        do_cmd_redraw();
        my_need_redraw = false;
    }

    /* Prevent clock overflow */
    if (borg_t - borg_began >= 10000)
    {
        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (boredom)");

            /* Start leaving */
            goal_leaving = true;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (boredom)");

            /* Start fleeing */
            goal_fleeing = true;
        }
    }

    /* am I fighting a unique or a summoner, or scaryguy? */
    borg_near_monster_type(borg_skill[BI_MAXCLEVEL] < 15 ? z_info->max_sight : 12);

    /* Allow borg to jump back up to town if needed.  He probably fled town because
     * he saw a scaryguy (BSV, SER, Maggot).  Since he is here on depth 1, do a quick
     * check for items near the stairs that I can pick up before I return to town.
     */
    if (borg_skill[BI_CDEPTH] == 1 && goal_fleeing_to_town)
    {

        /* Try to grab a close item while I'm down here */
        if (borg_think_stair_scum(true)) return (true);

        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (finish shopping)");

            /* Start leaving */
            goal_leaving = true;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (finish shopping)");

            /* Start fleeing */
            goal_fleeing = true;
        }
    }

    /* Prevent a "bouncing Borg" bug. Where borg with telepathy
     * will sit in a narrow area bouncing between 2 or 3 places
     * tracking and flowing to a bouncing monster behind a wall.
     * First, reset goals.
     * Second, clear all known monsters/takes
     * Third, Flee the level
     */
    if (borg_skill[BI_CDEPTH] &&
        (time_this_panel >= 300 && time_this_panel <= 303))
    {
        /* Clear Goals */
        goal = 0;
    }
    if (borg_skill[BI_CDEPTH] &&
        (time_this_panel >= 500 && time_this_panel <= 503))
    {
        /* Forget old objects */
        for (i = 1; i < borg_takes_nxt; i++)
            borg_delete_take(i);

        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Forget old monsters */
        for (i = 1; i < borg_kills_nxt; i++)
            borg_delete_kill(i);

        /* No monsters here */
        borg_kills_cnt = 0;
        borg_kills_nxt = 1;
    }

    if (borg_skill[BI_CDEPTH] &&
        (time_this_panel >= 700))
    {
        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (bouncing-borg)");

            /* Start leaving */
            goal_leaving = true;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (bouncing-borg)");

            /* Start fleeing */
            goal_fleeing = true;
        }

    }


    /* Count the awake breeders */
    for (j = 0, i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Skip sleeping monsters */
        if (!kill->awake) continue;

        /* Count the monsters which are "breeders" */
        if (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY)) j++;
    }

    /* hack -- close doors on breeder levels */
    if (j >= 3)
    {
        /* set the flag to close doors */
        breeder_level = true;
    }


    /* Hack -- caution from breeders*/
    if ((j >= MIN(borg_skill[BI_CLEVEL] + 2, 5)) &&
        (borg_skill[BI_RECALL] <= 0 || borg_skill[BI_CLEVEL] < 35))
    {
        /* Ignore monsters from caution */
        if (!goal_ignoring && borg_t >= 2500)
        {
            /* Flee */
            borg_note("# Ignoring breeders (no recall)");

            /* Ignore multipliers */
            goal_ignoring = true;
        }

        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (no recall)");

            /* Start leaving */
            goal_leaving = true;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (no recall)");

            /* Start fleeing */
            goal_fleeing = true;
        }


    }

    /* Reset avoidance */
    if (avoidance != borg_skill[BI_CURHP])
    {
        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Forget goals */
    }

    /* Keep borg on a short leash */
    if (track_less.num &&
        (borg_skill[BI_MAXHP] < 30 || borg_skill[BI_CLEVEL] < 15) &&
        borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5)
    {
        int y, x;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j = j;
        }

        /* Return to the upstair-- too far away? */
        if ((!goal_less) && b_j > borg_skill[BI_CLEVEL] * 3 + 14)
        {
            /* Return to Stairs */
            if (!goal_less)
            {
                /* Note */
                borg_note(format("# Return to Stair (wandered too far.  Leash: %d)", borg_skill[BI_CLEVEL] * 3 + 14));

                /* Start returning */
                goal_less = true;
            }

        }

        /* Clear the flag to Return to the upstair-- we are close enough now */
        else if (goal_less && b_j < 3)
        {
            /* Note */
            borg_note("# Close enough to Stair.");

            /* Clear the flag */
            goal_less = false;
            goal = 0;

        }
    }

    /* Quick check to see if borg needs to engage his lunal mode */
    if (borg_cfg[BORG_SELF_LUNAL] && 
       !borg_cfg[BORG_PLAYS_RISKY])  /* Risky borg in a hurry */
    {
        if ((char*)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 15) && /* Prepared */
            borg_skill[BI_MAXDEPTH] >= borg_skill[BI_CDEPTH] + 15 && /* Right zone */
            borg_skill[BI_CDEPTH] >= 1 && /* In dungeon fully */
            borg_skill[BI_CDEPTH] > borg_skill[BI_CLEVEL] / 3) /* Not shallow */
        {
            borg_lunal_mode = true;

            /* Enter the Lunal scumming mode */
            if (borg_lunal_mode && borg_think_dungeon_lunal()) return (true);
        }
    }

    /* Quick check to see if borg needs to engage his lunal mode for munchkin_start */
    if (borg_cfg[BORG_MUNCHKIN_START] && borg_skill[BI_MAXCLEVEL] < 12)
    {
        if (borg_skill[BI_CDEPTH] >= 1)
        {
            borg_munchkin_mode = true;

            /* Enter the Lunal scumming mode */
            if (borg_think_dungeon_munchkin()) return (true);
        }

        /* Must not be in munchkin mode then */
        borg_munchkin_mode = false;
    }

    /* Keep borg on a suitable level */
    if (track_less.num && borg_skill[BI_CLEVEL] < 10 &&
        !goal_less && (char*)NULL != borg_prepared(borg_skill[BI_CDEPTH]))
    {
        /* Note */
        borg_note("# Needing to get back on correct depth");

        /* Start returning */
        goal_less = true;

        /* Take stairs */
        if (borg_grids[c_y][c_x].feat == FEAT_LESS)
        {
            borg_keypress('<');
            return (true);
        }
    }

    /*** crucial goals ***/

    /* examine equipment and swaps */
    borg_notice(true);

    /* require light-- Special handle for being out of a light source.*/
    if (borg_think_dungeon_light()) return (true);

    /* Decrease the amount of time not allowed to retreat */
    if (borg_no_retreat > 0)
        borg_no_retreat--;

    /*** Important goals ***/

    /* Continue flowing towards good anti-summon grid */
    if (borg_flow_old(GOAL_DIGGING)) return (true);

    /* Try not to die */
    if (borg_caution()) return (true);

    /*** if returning from dungeon in bad shape...***/
    if (borg_skill[BI_CURLITE] == 0 || borg_skill[BI_ISCUT] ||
        borg_skill[BI_ISPOISONED] || borg_skill[BI_FOOD] == 0)
    {
        /* First try to wear something */
        if (borg_skill[BI_CURLITE] == 0)
        {
            /* attempt to refuel/swap */
            if (borg_maintain_light() == BORG_MET_NEED) return (true);

            /* wear stuff and see if it glows */
            if (borg_wear_stuff()) return (true);
// Things are now automatically put in the quiver !FIX !TODO !AJG double check
//            if (borg_wear_quiver()) return (true);
        }

        /* Recover from damage */
        if (borg_recover()) return (true);

        /* If full of items, we wont be able to buy stuff, crush stuff */
        if (borg_items[PACK_SLOTS - 1].iqty && borg_crush_hole()) return (true);

        /* shop for something that will help us */
        if (borg_flow_shop_visit()) return (true);

        if (borg_choose_shop())
        {
            /* Try and visit a shop, if so desired */
            if (borg_flow_shop_entry(goal_shop)) return (true);
        }
    }

    /* if I must go to town without delay */
    if ((char*)NULL != borg_restock(borg_skill[BI_CDEPTH]))
    {
        if (borg_leave_level(false)) return (true);
    }

    /* Learn useful spells immediately */
    if (borg_play_magic(false)) return (true);

    /* If using a digger, Wear "useful" equipment before fighting monsters */
    if (borg_items[INVEN_WIELD].tval == TV_DIGGING && borg_wear_stuff()) return (true);

    /* If not using anything, Wear "useful" equipment before fighting monsters */
    if (!borg_items[INVEN_WIELD].tval && borg_wear_stuff()) return (true);

    /* If not wielding any missiles, load up the quiver */
// Things are now automatically put in the quiver !FIX !TODO !AJG double check
//    if (borg_items[INVEN_BOW].iqty && !borg_items[QUIVER_START].tval && borg_wear_quiver()) return (true);

    /* Dig an anti-summon corridor */
    if (borg_flow_kill_corridor_1(true)) return (true);

    /* Attack monsters */
    if (borg_attack(false)) return (true);

    /* Wear things that need to be worn, but try to avoid swap loops */
    /* if (borg_best_stuff()) return (true); */
    if (borg_wear_stuff()) return (true);
// Things are now automatically put in the quiver !FIX !TODO !AJG double check
//    if (borg_wear_quiver()) return (true);
    if (borg_swap_rings()) return (true);
    if (borg_wear_rings()) return (true);

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a really close object */
    if (borg_flow_take(true, 5)) return (true);

    /* Remove "backwards" rings */
    /* Only do this in Stores to avoid loops     if (borg_swap_rings()) return (true); */

    /* Repair "backwards" rings */
    if (borg_wear_rings()) return (true);

    /* Remove stuff that is useless or detrimental */
    if (borg_remove_stuff()) return (true);
    if (borg_dump_quiver()) return (true);
 
    /* Check the light */
    if (borg_check_LIGHT()) return (true);

    /* Continue flowing to a safe grid on which I may recover */
    if (borg_flow_old(GOAL_RECOVER)) return (true);

    /* Recover from damage */
    if (borg_recover()) return (true);

    /* Attempt to find a grid which is safe and I can recover on it.  This should work closely with borg_recover. */
    if (borg_flow_recover(false, 50)) return (true);

    /* Perform "cool" perma spells */
    if (borg_perma_spell()) return (true);

    /* Try to stick close to stairs if weak */
    if (borg_skill[BI_CLEVEL] < 10 && borg_skill[BI_MAXSP] &&
        borg_skill[BI_CURSP] == 0 && borg_no_rest_prep <= 1 &&
        !borg_bless && !borg_hero && !borg_berserk && !borg_fastcast &&
        !player_has(player, PF_COMBAT_REGEN))
    {
        if (borg_skill[BI_CDEPTH])
        {
            int tmp_i, y, x;

            /* Check for an existing "up stairs" */
            for (tmp_i = 0; tmp_i < track_less.num; tmp_i++)
            {
                x = track_less.x[tmp_i];
                y = track_less.y[tmp_i];

                /* Not on a stair */
                if (c_y != y || c_x != x) continue;

                /* I am standing on a stair */

                /* reset the goal_less flag */
                goal_less = false;

                /* if not dangerous, wait here */
                if (borg_danger(c_y, c_x, 1, true, false) == 0)
                {
                    /* rest here a moment */
                    borg_note("# Resting on stair to gain Mana.");
                    borg_keypress(',');
                    return (true);
                }
            }
        }
        else /* in town */
        {
            int tmp_i, y, x;

            /* Check for an existing "dn stairs" */
            for (tmp_i = 0; tmp_i < track_more.num; tmp_i++)
            {
                x = track_more.x[tmp_i];
                y = track_more.y[tmp_i];

                /* Not on a stair */
                if (c_y != y || c_x != x) continue;

                /* I am standing on a stair */

                /* if not dangerous, wait here */
                if (borg_danger(c_y, c_x, 1, true, false) == 0)
                {
                    /* rest here a moment */
                    borg_note("# Resting on town stair to gain Mana.");
                    borg_keypress(',');
                    return (true);
                }
            }
        }

        /* In town, standing on stairs, sit tight */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, true))
        {
            borg_note("# Looking for stairs. Stair hugging.");
            return (true);
        }

    }

    /* If in town and have no money, and nothing to sell,
     * then do not stay in town, its too dangerous.
     */
    if (borg_skill[BI_CDEPTH] == 0 &&
        borg_skill[BI_CLEVEL] < 6 && borg_gold < 10 &&
        borg_count_sell() < 5)
    {
        borg_note("# Nothing to sell in town (leaving).");
        goal_leaving = true;

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs down */
        if (borg_flow_stair_more(GOAL_FLEE, false, false)) return (true);
    }

    /*** Flee the level XXX XXX XXX ***/

    /* Return to Stairs, but not use them */
    if (goal_less)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs */
        if (scaryguy_on_level && borg_flow_stair_both(GOAL_FLEE, false)) return (true);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, false))
        {
            borg_note("# Looking for stairs. Goal_less, Fleeing.");
            return (true);
        }
    }

    /* Flee the level */
    if (goal_fleeing && !goal_recalling)
    {
        /* Hack -- Take the next stairs */
        stair_less = stair_more = true;
        borg_note("# Fleeing and leaving the level. (Looking for any stair)");

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs */
        if (scaryguy_on_level &&
            borg_flow_stair_both(GOAL_FLEE, false)) return (true);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, false))
        {
            borg_note("# Looking for stairs. Fleeing.");
            return (true);
        }

        /* Try to find some stairs down */
        if (borg_flow_stair_more(GOAL_FLEE, false, false)) return (true);

    }

    /* Flee to a safe Morgoth grid if appropriate */
    if (!borg_skill[BI_KING] && morgoth_on_level && !borg_morgoth_position &&
        (borg_skill[BI_AGLYPH] >= 10 &&
            (!borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCONFUSED])))
    {
        /* Continue flowing towards good morgoth grid */
        if (borg_flow_old(GOAL_MISC)) return (true);

        /* Attempt to locate a good Glyphed grid */
        if (borg_flow_glyph(GOAL_MISC)) return (true);

        /* Have the borg excavate the dungeon with Stone to Mud */

    }

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a really close object */
    if (borg_flow_take(true, 5)) return (true);
    if (borg_flow_vein(true, 5)) return (true);

    /* Continue flowing towards (the hopefully close) monsters */
    if (borg_flow_old(GOAL_KILL)) return (true);

    /* Find a really close monster */
    if (borg_flow_kill(true, 20)) return (true);

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a really close object */
    if (borg_flow_take(false, 10)) return (true);
    if (borg_flow_vein(false, 10)) return (true);

    /* Continue flowing towards monsters */
    if (borg_flow_old(GOAL_KILL)) return (true);

    /* Continue towards a vault */
    if (borg_flow_old(GOAL_VAULT)) return (true);

    /* Find a viewable monster and line up a shot on him */
    if (borg_flow_kill_aim(true)) return (true);

    /*** Deal with inventory objects ***/

    /* check for anything that should be inscribed */
    /* if (borg_inscribe_food()) return (true); */

    /* Use things */
    if (borg_use_things()) return (true);

    /* Identify unknown things */
    if (borg_test_stuff()) return (true);

    /* Enchant things */
    if (borg_enchanting()) return (true);

    /* Recharge things */
    if (borg_recharging()) return (true);

    /* Destroy junk */
    if (borg_crush_junk()) return (true);

    /* Destroy items to make space */
    if (borg_crush_hole()) return (true);

    /* Destroy items if we are slow */
    if (borg_crush_slow()) return (true);


    /*** Flow towards objects ***/

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (true);

    /* Find a (viewable) object */
    if (borg_flow_take(true, 250)) return (true);
    if (borg_flow_vein(true, 250)) return (true);


    /*** Leave the level XXX XXX XXX ***/


    /* Leave the level */
    if ((goal_leaving && !goal_recalling && !unique_on_level) ||
        (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
            borg_count_sell() >= 13))
    {
        /* Hack -- Take the next stairs */
        if (borg_ready_morgoth == 0)
        {
            borg_note("# Fleeing and leaving the level (Looking for Up Stair).");
            stair_less = true;
        }

        /* Only go down if fleeing or prepared. */
        if ((char*)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 1))
            stair_more = true;

        /* Continue leaving the level */
        if (borg_flow_old(GOAL_FLEE)) return (true);

        /* Try to find some stairs up */
        if (stair_less)
        {
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                borg_note("# Looking for stairs. Goal_Leaving.");

                return (true);
            }
        }

        /* Only go up if needing to sell */
        if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
            borg_count_sell() >= 13) stair_more = false;

        /* Try to find some stairs down */
        if (stair_more)
            if (borg_flow_stair_more(GOAL_FLEE, false, false)) return (true);
    }

    /* Power dive if I am playing too shallow
     * This is also seen in leave_level().  If
     * this formula is modified here, change it
     * in leave_level too.
     */
    if (borg_skill[BI_CDEPTH] != 0 &&
        (char*)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 5) && !stair_less)
    {
        /* Take next stairs */
        stair_more = true;

        /* Continue leaving the level */
        if (borg_flow_old(GOAL_BORE)) return (true);

        /* No down if needing to sell */
        if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
            borg_count_sell() >= 13)
        {
            stair_more = false;
        }

        /* Attempt to use those stairs */
        if (stair_more && borg_flow_stair_more(GOAL_BORE, true, false))
        {
            /* Leave a note */
            borg_note("# Powerdiving.");
            return (true);
        }
    }

    /*** Exploration ***/

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_MISC)) return (true);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_DARK)) return (true);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_XTRA)) return (true);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_BORE)) return (true);

    if (borg_flow_old(GOAL_VAULT)) return (true);


    /*** Explore the dungeon ***/

    if (vault_on_level)
    {

        /* Chase close monsters */
        if (borg_flow_kill(false, z_info->max_range + 1)) return (true);

        /* Chase close objects */
        if (borg_flow_take(false, 35)) return (true);
        if (borg_flow_vein(false, 35)) return (true);

        /* Excavate a vault safely */
        if (borg_excavate_vault(z_info->max_range - 2)) return (true);

        /* Find a vault to excavate */
        if (borg_flow_vault(35)) return (true);

        /* Explore close interesting grids */
        if (borg_flow_dark(true)) return (true);
    }

    /* Chase old monsters */
    if (borg_flow_kill(false, 250)) return (true);

    /* Chase old objects */
    if (borg_flow_take(false, 250)) return (true);
    if (borg_flow_vein(false, 250)) return (true);

    /* Explore interesting grids */
    if (borg_flow_dark(true)) return (true);

    /* Leave the level (if needed) */
    if (borg_gold < borg_cfg[BORG_MONEY_SCUM_AMOUNT] && 
        borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 &&
        !borg_skill[BI_CDEPTH] && borg_skill[BI_LIGHT])
    {
        /* Stay in town and scum for money after shopping */
    }
    else
    {
        if (borg_leave_level(false)) return (true);
    }


    /* Explore interesting grids */
    if (borg_flow_dark(false)) return (true);


    /*** Deal with shops ***/

    /* Hack -- visit all the shops */
    if (borg_flow_shop_visit()) return (true);

    /* Hack -- Visit the shops */
    if (borg_choose_shop())
    {
        /* Try and visit a shop, if so desired */
        if (borg_flow_shop_entry(goal_shop)) return (true);
    }


    /*** Leave the Level ***/

    /* Study/Test boring spells/prayers */
    if (!goal_fleeing && borg_play_magic(true)) return (true);

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
        if (borg_check_LIGHT()) return (true);
    }

    /* Search for secret doors */
    if (borg_flow_spastic(false)) return (true);

    /* Flow directly to a monster if not able to be spastic */
    if (borg_flow_kill_direct(false, false)) return (true);

    /* Recharge items before leaving the level */
    if (borg_wear_recharge()) return (true);

    /* Leave the level (if possible) */
    if (borg_gold < borg_cfg[BORG_MONEY_SCUM_AMOUNT] && 
        borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 &&
        !borg_skill[BI_CDEPTH] && borg_skill[BI_LIGHT] &&
        !borg_cfg[BORG_PLAYS_RISKY]) /* risky borgs are in a hurry */
    {
        /* Stay in town, scum for money now that shopping is done. */
        if (borg_money_scum()) return (true);
    }
    else
    {
        if (borg_leave_level(true)) return (true);
    }

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
        if (borg_check_LIGHT()) return (true);
    }

    /* Search for secret doors */
    if (borg_flow_spastic(true)) return (true);

    /* Flow directly to a monster if not able to be spastic */
    if (borg_flow_kill_direct(true, false)) return (true);

    /*** Wait for recall ***/

    /* Wait for recall, unless in danger */
    if (goal_recalling && (borg_danger(c_y, c_x, 1, true, false) <= 0))
    {
        /* Take note */
        borg_note("# Waiting for Recall...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('9');
        borg_keypress(KC_ENTER);

        /* Done */
        return (true);
    }

    /*** Nothing to do ***/

    /* Twitching in town can be fatal.  Really he should not become twitchy
     * but sometimes he cant recall to the dungeon and that may induce the
     * twitchy behavior.  So we reset the level if this happens.  That will
     * force him to go shopping all over again.
     */
    if ((borg_skill[BI_CDEPTH] == 0 && borg_t - borg_began > 800) || borg_t > 28000) old_depth = 126;

    /* Set a flag that the borg is  not allowed to retreat for 5 rounds */
    borg_no_retreat = 5;

    /* Boost slightly */
    if (avoidance < borg_skill[BI_CURHP] * 2)
    {
        bool done = false;

        /* Note */
        borg_note(format("# Boosting bravery (1) from %d to %d!",
            avoidance, borg_skill[BI_CURHP] * 2));

        /* Hack -- ignore some danger */
        avoidance = (borg_skill[BI_CURHP] * 2);

        /* Forget the danger fields */
        borg_danger_wipe = true;

        /* Try anything */
        if (borg_think_dungeon_brave()) done = true;

        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Forget goals */
/*        goal = 0;*/

        /* Done */
        if (done) return (true);
    }

    /* try phase before boosting bravery further and acting goofy */
    borg_times_twitch++;

    /* Phase to get out of being twitchy up to 3 times per level. */
    if (borg_times_twitch < 3)
    {
        borg_note("# Considering Phase (twitchy)");

        /* Phase */
        if (borg_allow_teleport() &&
            (borg_spell(PHASE_DOOR) ||
                borg_activate_item(act_tele_phase) ||
                borg_read_scroll(sv_scroll_phase_door) ||
                borg_dimension_door(90) ||
                borg_spell(TELEPORT_SELF) ||
                borg_spell(PORTAL) ||
                borg_shadow_shift(90)))
        {
            /* Success */
            return (true);
        }
    }

    /* Set a flag that the borg is not allowed */
    /*  to retreat for 10 rounds */
    borg_no_retreat = 10;

    /* Boost some more */
    if (avoidance < borg_skill[BI_MAXHP] * 4)
    {
        bool done = false;

        /* Note */
        borg_note(format("# Boosting bravery (2) from %d to %d!",
            avoidance, borg_skill[BI_MAXHP] * 4));

        /* Hack -- ignore some danger */
        avoidance = (borg_skill[BI_MAXHP] * 4);

        /* Forget the danger fields */
        borg_danger_wipe = true;

        /* Try anything */
        if (borg_think_dungeon_brave()) done = true;

        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Forget goals */
/*        goal = 0;*/

        /* Done */
        if (done) return (true);
    }

    /* Boost a lot */
    if (avoidance < 30000)
    {
        bool done = false;

        /* Note */
        borg_note(format("# Boosting bravery (3) from %d to %d!",
            avoidance, 30000));

        /* Hack -- ignore some danger */
        avoidance = 30000;

        /* Forget the danger fields */
        borg_danger_wipe = true;

        /* Reset multiple factors to jumpstart the borg */
        unique_on_level = 0;
        scaryguy_on_level = false;

        /* reset our breeder flag */
        breeder_level = false;

        /* Forget goals */
        goal = 0;

        /* Hack -- cannot rise past town */
        if (!borg_skill[BI_CDEPTH]) goal_rising = false;

        /* Assume not ignoring monsters */
        goal_ignoring = false;

        /* No known stairs */
        track_less.num = 0;
        track_more.num = 0;

        /* No known glyph */
        track_glyph.num = 0;

        /* No known steps */
        track_step.num = 0;

        /* No known doors */
        track_door.num = 0;

        /* No known doors */
        track_closed.num = 0;

        /* No mineral veins */
        track_vein.num = 0;

        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Try anything */
        if (borg_think_dungeon_brave()) done = true;

        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Done */
        if (done) return (true);
    }

    /* try teleporting before acting goofy */
    borg_times_twitch++;

    /* Teleport to get out of being twitchy up to 5 times per level. */
    if (borg_times_twitch < 5)
    {
        /* Teleport */
        if (borg_allow_teleport() &&
            (borg_dimension_door(90) ||
                borg_spell(TELEPORT_SELF) ||
                borg_spell(PORTAL) ||
                borg_shadow_shift(90) ||
                borg_use_staff(sv_staff_teleportation) ||
                borg_read_scroll(sv_scroll_teleport) ||
                borg_read_scroll(sv_scroll_teleport_level)))
        {
            /* Success */
            borg_note("# Teleport (twitchy)");
            return (true);
        }
    }

    /* Recall to town */
    if (borg_skill[BI_CDEPTH] && (borg_recall()))
    {
        /* Note */
        borg_note("# Recalling (twitchy)");

        /* Success */
        return (true);
    }


    /* Reset multiple factors to jumpstart the borg */
    unique_on_level = 0;
    scaryguy_on_level = false;

    /* reset our breeder flag */
    breeder_level = false;

    /* No objects here */
    borg_takes_cnt = 0;
    borg_takes_nxt = 1;

    /* No monsters here */
    borg_kills_cnt = 0;
    borg_kills_nxt = 1;

    /* if we twitch a lot, time to leave */
    if (borg_times_twitch > 20)
        goal_fleeing = true;

    /* Attempt to dig to the center of the dungeon */
    if (borg_flow_kill_direct(true, true)) return (true);

    /* Twitch around */
    if (borg_twitchy()) return (true);

    /* Oops */
    return (false);
}


/*
 * Initialize this file
 */
void borg_init_8(void)
{
    test = mem_zalloc(z_info->store_inven_max * sizeof(uint8_t));
    best = mem_zalloc(z_info->store_inven_max * sizeof(uint8_t));
}

/*
 * Release resources allocated by borg_init_8().
 */
void borg_clean_8(void)
{
    mem_free(best);
    best = NULL;
    mem_free(test);
    test = NULL;
}

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif /* ALLOW_BORG */
