/* File: borg8.c */
/* Purpose: High level functions for the Borg -BEN- */

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"

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


byte *test;
byte *best;
s32b *b_home_power;


/* money Scumming is a type of town scumming for money */
bool borg_money_scum(void)
{

    int dir= -1;
	int divisor = 2;

	borg_grid *ag;

	/* Just a quick check to make sure we are supposed to do this */
	if (borg_money_scum_amount == 0) return (FALSE);

	/* Take note */
    borg_note(format("# Waiting for towns people to breed.  I need %d...",borg_money_scum_amount - borg_gold));

	/* I'm not in a store */
	borg_in_shop = FALSE;

    /* Rest for 9 months */
    if (borg_skill[BI_CLEVEL] >= 35)
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('5');
        borg_keypress('0');
        borg_keypress('0');
        borg_keypress('\n');
    }
    else if (borg_skill[BI_CLEVEL] >= 15)
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress('\n');
    }
    else /* Low level, dont want to get mobbed */
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('2');
        borg_keypress('5');
        borg_keypress('\n');
    }

	/* Don't rest too long at night.  We tend to crash the game if too many
	 * townsfolk are on the level
	 */
	/* Night or day up in town */
    if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)) divisor = 5;


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
     return (TRUE);
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
static bool borg_object_similar(borg_item  *o_ptr, borg_item  *j_ptr)
{
    /* NOTE: This assumes the giving of one item at a time */
    int total = o_ptr->iqty + 1;
	int i;


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
        {
            /* Require full knowledge of both items */
            if ((!o_ptr->aware) || (!j_ptr->aware)) return (0);

            /* Fall through */
        }

        /* Missiles */
        case TV_BOLT:
        case TV_ARROW:
        case TV_SHOT:
        {
            /* Require identical "bonuses" */
            if (o_ptr->to_h != j_ptr->to_h) return (FALSE);
            if (o_ptr->to_d != j_ptr->to_d) return (FALSE);
            if (o_ptr->to_a != j_ptr->to_a) return (FALSE);

            /* Require identical "pval" code */
            if (o_ptr->pval != j_ptr->pval) return (FALSE);

            /* Require identical "artifact" names */
            if (o_ptr->name1 != j_ptr->name1) return (FALSE);

            /* Require identical "ego-item" names */
            if (o_ptr->name2 != j_ptr->name2) return (FALSE);

            /* Hack -- Never stack "powerful" items */
            for (i = 0; i < 12 && i < OF_SIZE; i++)
			{
				if (o_ptr->flags[i] || j_ptr->flags[i]) return (FALSE);
			}

            /* Hack -- Never stack recharging items */
            if (o_ptr->timeout || j_ptr->timeout) return (FALSE);

            /* Require identical "values" */
            if (o_ptr->ac != j_ptr->ac) return (FALSE);
            if (o_ptr->dd != j_ptr->dd) return (FALSE);
            if (o_ptr->ds != j_ptr->ds) return (FALSE);

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
    if ((o_ptr->fully_identified) != (j_ptr->fully_identified)) return (0);

    /* The stuff with 'note' is not right but it is close.  I think it */
    /* has him assuming that he can't stack sometimes when he can.  This */
    /* is alright, it just causes him to take a bit more time to do */
    /* some exchanges. */
    /* Hack -- require semi-matching "inscriptions" */
    if (o_ptr->note[0] && j_ptr->note[0] &&
        (!streq(o_ptr->note, j_ptr->note)))
        return (0);

    /* Hack -- normally require matching "inscriptions" */
    if ((!streq(o_ptr->note, j_ptr->note))) return (0);

    /* Hack -- normally require matching "discounts" */
    if ((o_ptr->discount != j_ptr->discount)) return (0);


    /* Maximal "stacking" limit */
    if (total >= MAX_STACK_SIZE) return (0);


    /* They match, so they must be similar */
    return (TRUE);
}

/*
 * Find the mininum amount of some item to buy/sell. For most
 * items this is 1, but for certain items (such as ammunition)
 * it may be higher.  -- RML
 */
static int borg_min_item_quantity(borg_item *item)
{
    /* Only trade in bunches if sufficient cash */
    if (borg_gold < 250) return (1);

    /* Don't trade expensive items in bunches */
    if (item->cost > 5) return (1);

    /* Don't trade non-known items in bunches */
    if (!item->aware) return (1);

    /* Only allow some types */
    switch (item->tval)
    {
    case TV_SPIKE:
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
static void borg_think_home_sell_aux2_slow(  int n, int start_i )
{
    int i;

    /* All done */
    if (n == STORE_INVEN_MAX)
    {
        s32b home_power;

        /* Examine the home  */
        borg_notice_home(NULL, FALSE);

        /* Evaluate the home */
        home_power = borg_power_home();

        /* Track best */
        if (home_power > *b_home_power)
        {
            /* Save the results */
            for (i = 0; i < STORE_INVEN_MAX; i++) best[i] = test[i];

#if 0
            /* dump, for debugging */
            borg_note(format("Trying Combo (best home power %ld)",
                              *b_home_power));
            borg_note(format("             (test home power %ld)",home_power));
            for (i = 0; i < STORE_INVEN_MAX; i++)
            {
                if (borg_shops[7].ware[i].iqty)
                    borg_note(format("store %d %s (qty-%d).",  i,
                                       borg_shops[7].ware[i].desc,
                                       borg_shops[7].ware[i].iqty ));
                else
                    borg_note(format("store %d (empty).",  i));
            }
            borg_note(" "); /* add a blank line */
#endif

            /* Use it */
            *b_home_power = home_power;
        }

        /* Success */
        return;
    }

    /* Note the attempt */
    test[n] = n;

    /* Evaluate the default item */
    borg_think_home_sell_aux2_slow(n + 1, start_i );

    /* if this slot and the previous slot is empty, move on to previous slot*/
    /* this will prevent trying a thing in all the empty slots to see if */
    /* empty slot b is better than empty slot a.*/
    if ((n != 0) && !borg_shops[7].ware[n].iqty && !borg_shops[7].ware[n-1].iqty)
        return;

    /* try other combinations */
    for (i = start_i; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item;
        borg_item *item2;
        bool stacked = FALSE;

        item = &borg_items[i];
        item2= &borg_shops[7].ware[n];

        /* Skip empty items */
        /* Require "aware" */
        /* Require "known" */
        if (!item->iqty || !item->kind || !item->aware)
            continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        if (i==weapon_swap && weapon_swap !=0) continue;
        if (i==armour_swap && armour_swap !=0) continue;

        /* stacking? */
        if (borg_object_similar(item2, item))
        {
            item2->iqty++;
            item->iqty--;
            stacked = TRUE;
        }
        else
        {
            int k;
            bool found_match = FALSE;

            /* eliminate items that would stack else where in the list. */
            for (k = 0; k < STORE_INVEN_MAX; k++)
            {
                if (borg_object_similar(&safe_home[k], item))
                {
                    found_match = TRUE;
                    break;
                }
            }
            if (found_match)
                continue;

            /* replace current item with this item */
            COPY(item2, item, borg_item);

            /* only move one into a non-stack slot */
            item2->iqty = 1;

            /* remove item from pack */
            item->iqty--;
        }

        /* Note the attempt */
        test[n] = i + STORE_INVEN_MAX;

        /* Evaluate the possible item */
        borg_think_home_sell_aux2_slow( n + 1, i+1 );

        /* restore stuff */
        COPY(item2, &safe_home[n], borg_item);

        /* put item back into pack */
        item->iqty++;
    }
}


/*
 * this will see what single addition/substitution is best for the home.
 * The formula is not as nice as the one above because it will
 * not check all possible combinations of items. but it is MUCH faster.
 */

static void borg_think_home_sell_aux2_fast(  int n, int start_i )
{
    borg_item *item;
    borg_item *item2;
    s32b home_power;
    int i, k, p;
    bool stacked = FALSE;
    bool skip_it = FALSE;

    /* get the starting best (current) */
    /* Examine the home  */
    borg_notice_home(NULL, FALSE);

    /* Evaluate the home  */
    *b_home_power = borg_power_home();

    /* try individual substitutions/additions.   */
    for (n = 0; n < STORE_INVEN_MAX; n++)
    {
        item2 = &borg_shops[7].ware[n];
        for (i = 0; i < INVEN_MAX_PACK; i++)
        {
            item = &borg_items[i];

            /* Skip empty items */
            /* Require "aware" */
            /* Require "known" */

            if (!item->iqty || (!item->kind &&!item->aware))
                continue;
            if (i==weapon_swap && weapon_swap !=0) continue;
            if (i==armour_swap && armour_swap !=0) continue;

            /* Do not dump stuff at home that is not fully id'd and should be  */
            /* this is good with random artifacts. */
            if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && !item->fully_identified && item->name1) continue;

            /* Hack -- ignore "worthless" items */
            if (!item->value) continue;

			/* If this item was just bought a the house, don't tell it back to the house */
			for (p = 0; p < bought_item_num; p++)
			{
				if (bought_item_tval[p] == item->tval && bought_item_sval[p] == item->sval &&
					bought_item_pval[p] == item->pval && bought_item_store[p] == 7) skip_it = TRUE;
			}
			if (skip_it == TRUE) continue;

            /* stacking? */
            if (borg_object_similar(item2, item))
            {
                /* if this stacks with what was previously here */
                item2->iqty++;
                stacked = TRUE;
            }
            else
            {
                bool found_match = FALSE;

                /* eliminate items that would stack else where in the list. */
                for (k = 0; k < STORE_INVEN_MAX; k++)
                {
                    if (borg_object_similar(&safe_home[k], item))
                    {
                        found_match = TRUE;
                        break;
                    }
                }
                if (found_match)
                    continue;

                /* replace current item with this item */
                COPY(item2, item, borg_item);

                /* only move one into a non-stack slot */
                item2->iqty = 1;
            }

            /* remove item from pack */
            item->iqty--;

            /* Note the attempt */
            test[n] = i + STORE_INVEN_MAX;

            /* Test to see if this is a good substitution. */
            /* Examine the home  */
            borg_notice_home(NULL, FALSE);

            /* Evaluate the home  */
            home_power = borg_power_home();

            /* Track best */
            if (home_power > *b_home_power)
            {
                /* Save the results */
                for (k = 0; k < STORE_INVEN_MAX; k++) best[k] = test[k];

#if 0
                /* dump, for debugging */
                borg_note(format("Trying Combo (best home power %ld)",
                                    *b_home_power));
                borg_note(format("             (test home power %ld)",
                                    home_power));
                for (i = 0; i < STORE_INVEN_MAX; i++)
                    if (borg_shops[7].ware[i].iqty)
                        borg_note(format("store %d %s (qty-%d).",  i,
                                           borg_shops[7].ware[i].desc,
                                           borg_shops[7].ware[i].iqty ));
                    else
                    borg_note(format("store %d (empty).",  i));

                borg_note(" "); /* add a blank line */
#endif

                /* Use it */
                *b_home_power = home_power;
            }

            /* restore stuff */
            COPY(item2, &safe_home[n], borg_item);

            /* put item back into pack */
            item->iqty++;

            /* put the item back in the test array */
            test[n] = n;
        }
    }
}

/* locate useless item */
static void borg_think_home_sell_aux3( )
{
    int     p, i;
    s32b    borg_empty_home_power;
    s32b    power;

    /* get the starting power */
    borg_notice(TRUE);
    power = borg_power();

    /* get what an empty home would have for power */
    borg_notice_home( NULL, TRUE );
    borg_empty_home_power = borg_power_home();

    /* go through the inventory and eliminate items that either  */
    /* 1) will not increase the power of an empty house. */
    /* 2) will reduce borg_power if given to home */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        int num_items_given;
        num_items_given = 0;

        /* if there is no item here, go to next slot */
        if (!borg_items[i].iqty) continue;

		/* Dont sell back our Best Fit item (avoid loops) */
        if (borg_best_fit_item == borg_items[i].name1) continue;


        /* 1) eliminate garbage items (items that add nothing to an */
        /*     empty house) */
        borg_notice_home( &borg_items[i], FALSE );
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
            borg_notice(FALSE);

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
static bool borg_think_home_sell_aux( bool save_best )
{
    int icky = STORE_INVEN_MAX - 1;

    s32b home_power = -1L;

    int p, i = -1;

    byte test_a[STORE_INVEN_MAX];
    byte best_a[STORE_INVEN_MAX];

    /* if the best is being saved (see borg_think_shop_grab_aux) */
    /* !FIX THIS NEEDS TO BE COMMENTED BETTER */
    if (!save_best)
        b_home_power = &home_power;
    test = test_a;
    best = best_a;

#if 0
	/* if I have not been to home, do not try this yet. */
    if (!borg_shops[7].when) return FALSE;
#endif

    /* Hack -- the home is full */
    /* and pack is full */
    if (borg_shops[7].ware[icky].iqty &&
        borg_items[INVEN_MAX_PACK-1].iqty)
        return (FALSE);

    /* Copy all the store slots */
    for (i = 0; i < STORE_INVEN_MAX; i++)
    {
        /* Save the item */
        COPY(&safe_home[i], &borg_shops[7].ware[i], borg_item);

        /* clear test arrays (test[i] == i is no change) */
        best[i] = test[i] = i;
    }

    /* Hack -- Copy all the slots */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        /* Save the item -- do not consider these */
        if (i==weapon_swap && weapon_swap !=0) continue;
        if (i==armour_swap && armour_swap !=0) continue;
		/* dont consider the item i just found to be my best fit (4-6-07) */
        if (borg_best_fit_item == borg_items[i].name1) continue;

        COPY(&safe_items[i], &borg_items[i], borg_item);
    }

    /* get rid of useless items */
    borg_think_home_sell_aux3();

    /* Examine the borg once more with full inventory then swap in the */
    /* safe_items for the home optimization */
    borg_notice(FALSE);

    /* swap quantities (this should be all that is different) */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        byte save_qty;
        if (i==weapon_swap && weapon_swap !=0) continue;
        if (i==armour_swap && armour_swap !=0) continue;

        save_qty = safe_items[i].iqty;
        safe_items[i].iqty = borg_items[i].iqty;
        borg_items[i].iqty = save_qty;
    }

    *b_home_power = -1;

    /* find best combo for home. */
    if (borg_slow_optimizehome)
    {
        borg_think_home_sell_aux2_slow( 0, 0 );
    }
    else
    {
        borg_think_home_sell_aux2_fast( 0, 0 );
    }

    /* restore bonuses and such */
    for (i = 0; i < STORE_INVEN_MAX; i++)
    {
        COPY(&borg_shops[7].ware[i], &safe_home[i], borg_item);
    }

    for (i = 0; i < INVEN_TOTAL; i++)
    {
        if (i==weapon_swap && weapon_swap !=0) continue;
        if (i==armour_swap && armour_swap !=0) continue;
        COPY(&borg_items[i], &safe_items[i], borg_item);
    }

    borg_notice(FALSE);
    borg_notice_home(NULL, FALSE);

    /* Drop stuff that will stack in the home */
    for (i = 0; i < STORE_INVEN_MAX; i++)
    {
        /* if this is not the item that was there, */
        /* drop off the item that replaces it. */
        if (best[i] != i && best[i] != 255)
        {
            borg_item *item = &borg_items[best[i]-STORE_INVEN_MAX];
            borg_item *item2 = &borg_shops[7].ware[i];

            /* if this item is not the same as what was */
            /* there before take it. */
            if (!borg_object_similar(item2, item))
                continue;

			/* There are limted quantity per slot */
			if (item2->iqty > 90) continue;

            goal_shop = 7;
            goal_item = best[i] - STORE_INVEN_MAX;

            return (TRUE);
        }
    }

    /* Get rid of stuff in house but not in 'best' house if  */
    /* pack is not full */
    if (!borg_items[INVEN_MAX_PACK-1].iqty)
    {
        for (i = 0; i < STORE_INVEN_MAX; i++)
        {
            /* if this is not the item that was there, */
            /* get rid of the item that was there */
            if ((best[i] != i) &&
                (borg_shops[7].ware[i].iqty))
            {
                borg_item *item = &borg_items[best[i]-STORE_INVEN_MAX];
                borg_item *item2 = &borg_shops[7].ware[i];

                /* if this item is not the same as what was */
                /* there before take it. */
                if (borg_object_similar(item, item2))
                    continue;

                /* skip stuff if we sold bought it */
                /* skip stuff if we sold/bought it */
                for (p = 0; p < sold_item_num; p++)
				{
					if (sold_item_tval[p] == item2->tval && sold_item_sval[p] == item2->sval &&
						sold_item_store[p] == 7) return (FALSE);
				}

                goal_shop = 7;
                goal_ware = i;

                return TRUE;
            }
        }
    }

    /* Drop stuff that is in best house but currently in inventory */
    for (i = 0; i < STORE_INVEN_MAX; i++)
    {
        /* if this is not the item that was there,  */
        /* drop off the item that replaces it. */
        if (best[i] != i && best[i] != 255)
        {
            /* hack dont sell DVE */
            if (!borg_items[best[i]-STORE_INVEN_MAX].iqty) return (FALSE);

            goal_shop = 7;
            goal_item = best[i] - STORE_INVEN_MAX;

            return (TRUE);
        }
    }

    /* Return our num_ counts to normal */
    borg_notice_home(NULL, FALSE);

    /* Assume not */
    return (FALSE);
}


/*
 * Determine if an item can be sold in the given store
 *
 * XXX XXX XXX Consider use of "icky" test on items
 */
static bool borg_good_sell(borg_item *item, int who)
{
	int i;

	/* Never sell worthless items */
    if (item->value <= 0) return (FALSE);

	/* Never sell valuable non-id'd items */
	if (strstr(item->note, "magical") ||
		strstr(item->note, "excellent") ||
		strstr(item->note, "ego") ||
		strstr(item->note, "splendid") ||
		strstr(item->note, "special")) return (FALSE);

	/* Worshipping gold or scumming will allow the sale */
    if (item->value > 0 &&
       ((borg_worships_gold || borg_skill[BI_MAXCLEVEL] < 10) ||
        ((borg_money_scum_amount < borg_gold) && borg_money_scum_amount != 0)) &&
        !strstr(item->note, "cursed") &&
		!strstr(item->note, "magical") &&
		!strstr(item->note, "ego") &&
		!strstr(item->note, "splendid") &&
		!strstr(item->note, "special") &&
		!strstr(item->note, "excellent"))
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

	        /* Never sell if not "known" and interesting */
	        if (!item->ident && (borg_skill[BI_MAXDEPTH] > 35)) return (FALSE);

	        /* Spell casters should not sell ResMana to shop unless
	         * they have tons in the house
	         */
	        if (item->tval == TV_POTION &&
	            item->sval == SV_POTION_RESTORE_MANA &&
	            borg_skill[BI_MAXSP] > 100 &&
	            borg_has[POTION_RES_MANA] + num_mana > 99) return (FALSE);

	        break;

	        case TV_FOOD:
	        case TV_ROD:
	        case TV_WAND:
	        case TV_STAFF:
	        case TV_RING:
	        case TV_AMULET:
	        case TV_LIGHT:

				/* Never sell if not "known" */
				if (!item->ident && !borg_item_icky(item) && (borg_skill[BI_MAXDEPTH] > 35)) return (FALSE);

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
	        if (!item->ident && !borg_item_icky(item)) return (FALSE);

	        break;
	    }
	}

    /* Do not sell stuff that is not fully id'd and should be  */
    if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && !item->fully_identified && item->name1)
    {
              /* CHECK THE ARTIFACTS */
                   /* For now check all artifacts */
                      return (FALSE);
    }
    /* Do not sell stuff that is not fully id'd and should be  */
    if (!item->fully_identified && item->name2)
    {
       if (e_info[borg_items[INVEN_OUTER].name2].xtra == OBJECT_XTRA_TYPE_RESIST ||
			e_info[borg_items[INVEN_OUTER].name2].xtra == OBJECT_XTRA_TYPE_POWER)
	   {
			return (FALSE);
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
              borg_note(format("# Choosing not to sell back %s",item->desc));
#endif
			return (FALSE);
		}
	}

    /* Switch on the store */
    switch (who + 1)
    {
        /* General Store */
        case 1:

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
                return (TRUE);
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
                return (TRUE);
            }
            break;

        /* Temple */
        case 4:

            /* Analyze the type */
            switch (item->tval)
            {
                case TV_HAFTED:
                case TV_PRAYER_BOOK:
                case TV_SCROLL:
                case TV_POTION:
                return (TRUE);
            }
            break;

        /* book store --Alchemist */
        case 5:

            /* Analyze the type */
            switch (item->tval)
            {
                case TV_SCROLL:
                case TV_POTION:
                return (TRUE);
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
                return (TRUE);
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
                return (TRUE);
            }
            break;

    }

    /* Assume not */
    return (FALSE);
}



/*
 * Step 2 -- sell "useless" items to a shop (for cash)
 */
static bool borg_think_shop_sell_aux(void)
{
     int icky = STORE_INVEN_MAX - 1;

    int k, b_k = -1;
    int i, b_i = -1;
    int qty = 1;
    s32b p, b_p = 0L;
    s32b c = 0L;
    s32b b_c = 30001L;

    bool fix = FALSE;


    /* Evaluate */
    b_p = my_power;

    /* Check each shop */
    for (k = 0; k < (MAX_STORES -1) ; k++)
    {
        /* Hack -- Skip "full" shops */
        if (borg_shops[k].ware[icky].iqty) continue;

        /* Save the store hole */
        COPY(&safe_shops[k].ware[icky], &borg_shops[k].ware[icky], borg_item);

        /* Sell stuff */
        for (i = 0; i < INVEN_MAX_PACK; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip some important type items */
            if ((item->tval == my_ammo_tval) && (borg_skill[BI_AMISSILES] < 45)) continue;
            if (item->tval == TV_ROD && item->sval == SV_ROD_HEALING &&
                borg_has[ROD_HEAL] <= 3) continue;

            if (borg_class == CLASS_WARRIOR &&
                item->tval == TV_ROD && item->sval == SV_ROD_MAPPING &&
                item->iqty <= 2) continue;

            /* Avoid selling staff of dest*/
            if (item->tval == TV_STAFF && item->sval == SV_STAFF_DESTRUCTION &&
                borg_skill[BI_ASTFDEST] < 2) continue;

			/* Do not sell our attack wands if they still have charges */
			if (item->tval == TV_WAND && borg_skill[BI_CLEVEL] < 35 &&
				(item->sval == SV_WAND_MAGIC_MISSILE || item->sval == SV_WAND_STINKING_CLOUD ||
				 item->sval == SV_WAND_ANNIHILATION) && item->pval != 0) continue;

			/* dont sell our swap items */
            if (i==weapon_swap && weapon_swap !=0) continue;
            if (i==armour_swap && armour_swap !=0) continue;

            /* Skip "bad" sales */
            if (!borg_good_sell(item, k)) continue;

            /* Save the item */
            COPY(&safe_items[i], &borg_items[i], borg_item);

            /* Give the item to the shop */
            COPY(&borg_shops[k].ware[icky], &safe_items[i], borg_item);

            /* get the quantity */
            qty = borg_min_item_quantity(item);

            /* Give a single item */
            borg_shops[k].ware[icky].iqty = qty;

            /* Lose a single item */
            borg_items[i].iqty -=qty;

            /* Fix later */
            fix = TRUE;

            /* Examine the inventory */
            borg_notice(FALSE);

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            COPY(&borg_items[i], &safe_items[i], borg_item);

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
        COPY(&borg_shops[k].ware[icky], &safe_shops[k].ware[icky], borg_item);
    }

    /* Examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Sell something (if useless) */
    if ((b_k >= 0) && (b_i >= 0))
    {
        /* Visit that shop */
        goal_shop = b_k;

        /* Sell that item */
        goal_item = b_i;

        /* Success */
        return (TRUE);
    }

    /* Assume not */
    return (FALSE);
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
static bool borg_good_buy(borg_item *item, int who, int ware)
{
	int p;

    /* Check the object */
    switch (item->tval)
    {
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        if (borg_skill[BI_CLEVEL] < 35)
        {
            if (item->to_h) return (FALSE);
            if (item->to_d) return (FALSE);
        }
        break;

        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        if (item->sval >= 4) break;
        if (item->discount) return (FALSE);
        break;

        case TV_WAND:
        case TV_STAFF:
        break;
    }

    /* Don't buy from the BM until we are rich */
    if (who == 6)
    {
        /* buying Remove Curse scroll is acceptable */
        if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_REMOVE_CURSE &&
            borg_wearing_cursed) return (TRUE);

		/* Buying certain special items are acceptable */
		if ((item->tval == TV_POTION &&
			 ((item->sval == SV_POTION_STAR_HEALING) ||
			  (item->sval == SV_POTION_LIFE) ||
			  (item->sval == SV_POTION_HEALING) ||
			  (item->sval == SV_POTION_INC_STR && my_stat_cur[A_STR] < (18+100)) ||
			  (item->sval == SV_POTION_INC_INT && my_stat_cur[A_INT] < (18+100)) ||
			  (item->sval == SV_POTION_INC_WIS && my_stat_cur[A_WIS] < (18+100)) ||
			  (item->sval == SV_POTION_INC_DEX && my_stat_cur[A_DEX] < (18+100)) ||
			  (item->sval == SV_POTION_INC_CON && my_stat_cur[A_CON] < (18+100)))) ||
			(item->tval == TV_ROD &&
			 ((item->sval == SV_ROD_HEALING) ||
			  (item->sval == SV_ROD_RECALL && (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)) ||
			  (item->sval == SV_ROD_SPEED && (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)) ||
			  (item->sval == SV_ROD_TELEPORT_OTHER && (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)) ||
			  (item->sval == SV_ROD_ILLUMINATION && (!borg_skill[BI_ALITE])))) ||
			(item->tval == TV_PRAYER_BOOK && p_ptr->class->spell_book == TV_PRAYER_BOOK &&
			  (amt_book[item->sval] == 0 && item->sval >= 5)) ||
			(item->tval == TV_MAGIC_BOOK && p_ptr->class->spell_book == TV_MAGIC_BOOK &&
			  (amt_book[item->sval] == 0 && item->sval >= 5)) ||
			(item->tval == TV_SCROLL &&
			  (item->sval == SV_SCROLL_TELEPORT_LEVEL ||
			   item->sval == SV_SCROLL_TELEPORT)))
		{
			/* Hack-- Allow the borg to scum for this Item */
			if (borg_self_scum &&  /* borg is allowed to scum */
			    borg_skill[BI_CLEVEL] >= 10 && /* Be of sufficient level */
				borg_skill[BI_LIGHT] &&   /* Have some Perma lite source */
				borg_skill[BI_FOOD] + num_food >= 100 && /* Have plenty of food */
				item->cost <= 85000) /* Its not too expensive */


			{
			    int save_throw;
			    /* Set some Savings Throw Percents based on Class */
			    if (borg_class == CLASS_ROGUE || borg_class == CLASS_WARRIOR)
			    {
					save_throw = 95;
				}
				else
				{
					save_throw = 90;
				}
			    if (adj_dex_safe[borg_skill[BI_DEX]] + borg_skill[BI_CLEVEL] > save_throw ) /* Good chance to thwart mugging */
			    {
					/* Record the amount that I need to make purchase */
					borg_money_scum_amount = item->cost;
					borg_money_scum_who = who;
					borg_money_scum_ware = ware;
				}
			}

			/* Ok to buy this */
			return (TRUE);
		}

        if ((borg_skill[BI_CLEVEL] < 15) && (borg_gold < 20000))
            return (FALSE);
        if ((borg_skill[BI_CLEVEL] < 35) && (borg_gold < 15000))
            return (FALSE);
        if (borg_gold < 10000)
            return (FALSE);
    }

    /* do not buy the item if I just sold it. */
    for (p = 0; p < sold_item_num; p++)
	{

		if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval && sold_item_store[p] == who)
		{
			if (borg_verbose) borg_note(format("# Choosing not to buy back %s",item->desc));
			return (FALSE);
		}
	}

    /* Do not buy a second digger */
    if (item->tval == TV_DIGGING)
    {
        int ii;

        /* scan for an existing digger */
         for (ii = 0; ii < INVEN_MAX_PACK; ii++)
         {
             borg_item *item2 = &borg_items[ii];


            /* skip non diggers */
            if (item2->tval == TV_DIGGING) return (FALSE);
#if 0
            /* perhaps let him buy a digger with a better
             * pval than his current digger
             */
            {if (item->pval <= item2->pval) return (FALSE);}
#endif
        }
     }

    /* Low level borgs should not waste the money on certain things */
    if (borg_skill[BI_MAXCLEVEL] < 5)
    {
        /* next book, cant read it */
        if ((item->tval == TV_MAGIC_BOOK || item->tval == TV_PRAYER_BOOK) &&
            item->sval >=1) return (FALSE);
    }

    /* Rangers and Paladins and the extra books */
    if ((borg_class == CLASS_PALADIN || borg_class == CLASS_RANGER) &&
        borg_skill[BI_MAXCLEVEL] <= 8)
    {
        if ((item->tval == TV_MAGIC_BOOK || item->tval == TV_PRAYER_BOOK) &&
            item->sval >=1) return (FALSE);
    }



    /* Okay */
    return (TRUE);
}



/*
 * Step 3 -- buy "useful" things from a shop (to be used)
 */
static bool borg_think_shop_buy_aux(void)
{
    int hole = INVEN_MAX_PACK - 1;

    int slot;
    int qty =1;

    int k, b_k = -1;
    int n, b_n = -1;
    s32b p, b_p = 0L;
    s32b c, b_c = 0L;

    bool fix = FALSE;

    /* Require one empty slot */
    if (borg_items[hole].iqty) return (FALSE);
    if (borg_items[INVEN_MAX_PACK-1].iqty) return (FALSE);

	/* Already have a target 9-4-05*/
	if (goal_ware != -1) return (FALSE);

    /* Extract the "power" */
    b_p = my_power;
	b_p = borg_power();

	/* Check the shops */
    for (k = 0; k < (MAX_STORES -1 ); k++)
    {

		/* If I am bad shape up, only see certain stores */
	    if ((borg_skill[BI_CURLITE] == 0 || borg_skill[BI_FOOD] == 0) &&
			k !=0 && k != 7) continue;
		if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && k != 3) continue;


        /* Scan the wares */
        for (n = 0; n < STORE_INVEN_MAX; n++)
        {
            borg_item *item = &borg_shops[k].ware[n];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip "bad" buys */
			if (!borg_good_buy(item, k, n)) continue;

			/* Attempting to scum money, don't buy other stuff unless it is our home or food-store */
			if (borg_money_scum_amount && (k != borg_money_scum_who || n != borg_money_scum_ware)) continue;

			/* Hack -- Require "sufficient" cash */
            if (borg_gold < item->cost) continue;

            /* Skip it if I just sold this item. XXX XXX*/

			/* Special check for 'immediate shopping' */
			if (borg_skill[BI_FOOD] == 0 &&
				(item->tval != TV_FOOD &&
				(item->tval != TV_SCROLL &&
				 item->sval != SV_SCROLL_SATISFY_HUNGER))) continue;

			/* Don't fill up on attack wands, its ok to buy a few */
			if (item->tval == TV_WAND &&
				(item->sval == SV_WAND_MAGIC_MISSILE || item->sval == SV_WAND_STINKING_CLOUD) &&
				amt_cool_wand > 40) continue;

			/* These wands are not useful later on, we need beefier attacks */
			if (item->tval == TV_WAND &&
				(item->sval == SV_WAND_MAGIC_MISSILE || item->sval == SV_WAND_STINKING_CLOUD) &&
				borg_skill[BI_MAXCLEVEL] > 30) continue;

			/* Save shop item */
            COPY(&safe_shops[k].ware[n], &borg_shops[k].ware[n], borg_item);

            /* Save hole */
            COPY(&safe_items[hole], &borg_items[hole], borg_item);

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
			if (item->tval == TV_LIGHT && item->sval == SV_LIGHT_TORCH &&
				of_has(borg_items[INVEN_LIGHT].flags, OF_LIGHT))
			{
				slot = -1;
			}

			/* Hack, we keep diggers as a back-up, not to
             * replace our current weapon
             */
            if (item->tval == TV_DIGGING) slot = -1;

            /* if our current equip is cursed, then I can't
             * buy a new replacement.
             * XXX  Perhaps he should not buy anything but save
             * money for the Remove Curse Scroll.
             */
            if (slot >= INVEN_WIELD)
            {
                if (borg_items[slot].cursed) continue;
                if (of_has(borg_items[slot].flags, OF_HEAVY_CURSE)) continue;
                if (of_has(borg_items[slot].flags, OF_PERMA_CURSE)) continue;
            }

            /* Consider new equipment */
            if (slot >= 0)
            {
                /* Save old item */
                COPY(&safe_items[slot], &borg_items[slot], borg_item);

                /* Move equipment into inventory */
                COPY(&borg_items[hole], &safe_items[slot], borg_item);

                /* Move new item into equipment */
                COPY(&borg_items[slot], &safe_shops[k].ware[n], borg_item);

                /* Only a single item */
                borg_items[slot].iqty = qty;

                /* Fix later */
                fix = TRUE;

                /* Examine the inventory */
                borg_notice(FALSE);

                /* Evaluate the inventory */
                p = borg_power();

                /* Restore old item */
                COPY(&borg_items[slot], &safe_items[slot], borg_item);
            }

            /* Consider new inventory */
            else
            {
                /* Move new item into inventory */
                COPY(&borg_items[hole], &safe_shops[k].ware[n], borg_item);

                /* Only a single item */
                borg_items[hole].iqty = qty;

                /* Fix later */
                fix = TRUE;

                /* Examine the inventory */
                borg_notice(FALSE);

                /* Evaluate the equipment */
                p = borg_power();
            }


            /* Restore hole */
            COPY(&borg_items[hole], &safe_items[hole], borg_item);

            /* Restore shop item */
            COPY(&borg_shops[k].ware[n], &safe_shops[k].ware[n], borg_item);

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
    if (fix) borg_notice(TRUE);

    /* Buy something */
    if ((b_k >= 0) && (b_n >= 0))
    {
        /* Visit that shop */
        goal_shop = b_k;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Step 4 -- buy "useful" things from the home (to be used)
 */
static bool borg_think_home_buy_aux(void)
{

	int hole = INVEN_MAX_PACK - 1;
    int slot, i;
    int stack;
    int qty=1;
    int n, b_n = -1;
    s32b p, b_p = 0L;
    s32b p_left = 0;
    s32b p_right = 0;

    bool fix = FALSE;
	bool skip_it = FALSE;


    /* Extract the "power" */
    b_p = my_power;

    /* Scan the home */
    for (n = 0; n < STORE_INVEN_MAX; n++)
    {
        borg_item *item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip it if I just sold it */
        for (i = 0;  i < sold_item_num; i++)
		{
			if (sold_item_tval[i] == item->tval && sold_item_sval[i] == item->sval)
			{
				if (borg_verbose) borg_note(format("# Choosing not to buy back '%s' from home.", item->desc));
				skip_it = TRUE;
			}
		}
		if (skip_it == TRUE) continue;

		/* Reset the 'hole' in case it was changed by the last stacked item.*/
		hole = INVEN_MAX_PACK - 1;

		/* borg_note(format("# Considering buying (%d)'%s' (pval=%d) from home.", item->iqty,item->desc, item->pval)); */

        /* Save shop item */
        COPY(&safe_shops[7].ware[n], &borg_shops[7].ware[n], borg_item);

        /* Save hole */
        COPY(&safe_items[hole], &borg_items[hole], borg_item);

        /* Save the number */
        qty = borg_min_item_quantity(item);

        /* Remove one item from shop (sometimes) */
        borg_shops[7].ware[n].iqty -= qty;

        /* Obtain "slot" */
        slot = borg_wield_slot(item);
		stack = borg_slot(item->tval, item->sval);

        /* Consider new equipment-- Must check both ring slots */
        if (slot >= 0)
        {

			 /* Require one empty slot */
			if (borg_items[INVEN_MAX_PACK-1].iqty) continue;
			if (borg_items[INVEN_MAX_PACK-2].iqty) continue;

            /* Check Rings */
            if (slot == INVEN_LEFT)
            {
                /** First Check Left Hand **/

                /* special curse check for left ring */
                if (!borg_items[INVEN_LEFT].cursed)
                {
                    /* Save old item */
                    COPY(&safe_items[slot], &borg_items[slot], borg_item);

                    /* Move equipment into inventory */
                    COPY(&borg_items[hole], &safe_items[slot], borg_item);

                    /* Move new item into equipment */
                    COPY(&borg_items[slot], &safe_shops[7].ware[n], borg_item);

                    /* Only a single item */
                    borg_items[slot].iqty = qty;

                    /* Fix later */
                    fix = TRUE;

                    /* Examine the inventory */
                    borg_notice(FALSE);

                    /* Evaluate the inventory */
                    p_left = borg_power();
#if 0
            /* dump list and power...  for debugging */
            borg_note(format("Trying Item %s (best power %ld)",borg_items[slot].desc, p_left));
            borg_note(format("   Against Item %s   (borg_power %ld)",safe_items[slot].desc, my_power));
#endif
                    /* Restore old item */
                    COPY(&borg_items[slot], &safe_items[slot], borg_item);
                }


                /** Second Check Right Hand **/
                /* special curse check for right ring */
                if (!borg_items[INVEN_RIGHT].cursed)
                {
                    /* Save old item */
                    COPY(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT], borg_item);

                    /* Move equipment into inventory */
                    COPY(&borg_items[hole], &safe_items[INVEN_RIGHT], borg_item);

                    /* Move new item into equipment */
                    COPY(&borg_items[INVEN_RIGHT], &safe_shops[7].ware[n], borg_item);

                    /* Only a single item */
                    borg_items[INVEN_RIGHT].iqty = qty;

                    /* Fix later */
                    fix = TRUE;

                    /* Examine the inventory */
                    borg_notice(FALSE);

                    /* Evaluate the inventory */
                    p_right = borg_power();

#if 0
					/* dump list and power...  for debugging */
                    borg_note(format("Trying Item %s (best power %ld)",borg_items[INVEN_RIGHT].desc, p_right));
                    borg_note(format("   Against Item %s   (borg_power %ld)",safe_items[INVEN_RIGHT].desc, my_power));
#endif
                    /* Restore old item */
                    COPY(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], borg_item);
                }

                /* Is this ring better than one of mine? */
                p = MAX(p_right, p_left);

            }

            else /* non rings */
            {

                /* do not consider if my current item is cursed */
                if (slot != -1 && borg_items[slot].cursed) continue;

                /* Save old item */
                COPY(&safe_items[slot], &borg_items[slot], borg_item);

                /* Move equipment into inventory */
                COPY(&borg_items[hole], &safe_items[slot], borg_item);

                /* Move new item into equipment */
                COPY(&borg_items[slot], &safe_shops[7].ware[n], borg_item);

                /* Only a single item */
                borg_items[slot].iqty = qty;

                /* Fix later */
                fix = TRUE;

                /* Examine the inventory */
                borg_notice(FALSE);

                /* Evaluate the inventory */
                p = borg_power();
#if 0
                /* dump list and power...  for debugging */
                borg_note(format("Trying Item %s (best power %ld)",borg_items[slot].desc, p));
                borg_note(format("   Against Item %s   (borg_power %ld)",safe_items[slot].desc, my_power));
#endif
                /* Restore old item */
                COPY(&borg_items[slot], &safe_items[slot], borg_item);
            } /* non rings */
        } /* equip */

        /* Consider new inventory */
        else
        {
			if (stack != -1) hole = stack;

		    /* Require one empty slot */
			if (stack == -1 && borg_items[INVEN_MAX_PACK-1].iqty) continue;
			if (stack == -1 && borg_items[INVEN_MAX_PACK-2].iqty) continue;

			/* Save hole (could be either empty slot or stack */
		    COPY(&safe_items[hole], &borg_items[hole], borg_item);

			/* Move new item into inventory */
			COPY(&borg_items[hole], &safe_shops[7].ware[n], borg_item);

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
			fix = TRUE;

			/* Examine the inventory */
			borg_notice(FALSE);

			/* Evaluate the equipment */
			p = borg_power();
        }

        /* Restore hole */
        COPY(&borg_items[hole], &safe_items[hole], borg_item);

        /* Restore shop item */
        COPY(&borg_shops[7].ware[n], &safe_shops[7].ware[n], borg_item);

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and cost */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Buy something */
    if ((b_n >= 0) && (b_p > my_power))
    {
        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}



/*
 * Step 5 -- buy "interesting" things from a shop (to be used later)
 */
static bool borg_think_shop_grab_aux(void)
{

    int k, b_k = -1;
    int n, b_n = -1;
    int qty=1;

    s32b s, b_s = 0L;
    s32b c, b_c = 0L;
    s32b borg_empty_home_power;


    /* Dont do this if Sauron is dead */
    if (borg_race_death[546] != 0) return (FALSE);

	/* not until later-- use that money for better equipment */
	if (borg_skill[BI_CLEVEL] < 15) return (FALSE);

    /* get what an empty home would have for power */
    borg_notice_home( NULL, TRUE );
    borg_empty_home_power = borg_power_home();

    b_home_power = &s;

    /* Require two empty slots */
    if (borg_items[INVEN_MAX_PACK-1].iqty) return (FALSE);
    if (borg_items[INVEN_MAX_PACK-2].iqty) return (FALSE);

    /* Examine the home */
    borg_notice_home(NULL, FALSE);

    /* Evaluate the home */
    b_s = borg_power_home();

    /* Check the shops */
    for (k = 0; k < (MAX_STORES-1); k++)
    {
        /* Scan the wares */
        for (n = 0; n < STORE_INVEN_MAX; n++)
        {
            borg_item *item = &borg_shops[k].ware[n];

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
            COPY(&borg_items[INVEN_MAX_PACK-2], &borg_shops[k].ware[n], borg_item);

            /* Save the number */
            qty = borg_min_item_quantity(item);

            /* Give a single item */
            borg_items[INVEN_MAX_PACK-2].iqty = qty;

            /* make sure this item would help an empty home */
            borg_notice_home( &borg_shops[k].ware[n], FALSE );
            if (borg_empty_home_power >= borg_power_home()) continue;

            /* optimize the home inventory */
            if (!borg_think_home_sell_aux( TRUE )) continue;

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
    borg_items[INVEN_MAX_PACK-2].iqty = 0;

    /* Examine the real home */
    borg_notice_home(NULL, FALSE);

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
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Step 6 -- take "useless" things from the home (to be sold)
 */
static bool borg_think_home_grab_aux(void)
{
    int p, n, b_n = -1;
    s32b s, b_s = 0L;
    int qty=1;
	bool skip_it = FALSE;

    /* Require two empty slots */
    if (borg_items[INVEN_MAX_PACK-1].iqty) return (FALSE);
    if (borg_items[INVEN_MAX_PACK-2].iqty) return (FALSE);


    /* Examine the home */
    borg_notice_home(NULL, FALSE);

    /* Evaluate the home */
    b_s = borg_power_home();


    /* Scan the home */
    for (n = 0; n < STORE_INVEN_MAX; n++)
    {
        borg_item *item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* skip stuff if we sold bought it */
        for (p = 0;  p < sold_item_num; p++)
		{
			if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval && sold_item_store[p] == 7) skip_it = TRUE;
		}
		if (skip_it == TRUE); continue;

        /* Save shop item */
        COPY(&safe_shops[7].ware[n], &borg_shops[7].ware[n], borg_item);

        /* Save the number */
        qty = borg_min_item_quantity(item);

        /* Remove one item from shop */
        borg_shops[7].ware[n].iqty -= qty;

        /* Examine the home */
        borg_notice_home(NULL, FALSE);

        /* Evaluate the home */
        s = borg_power_home();

        /* Restore shop item */
        COPY(&borg_shops[7].ware[n], &safe_shops[7].ware[n], borg_item);

        /* Ignore "bad" sales */
        if (s < b_s) continue;

        /* Maintain the "best" */
        b_n = n; b_s = s;
    }

    /* Examine the home */
    borg_notice_home(NULL, FALSE);

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
        return (TRUE);
    }

    /* Assume not */
    return (FALSE);
}

/*
 * Step 7A -- buy "useful" weapons from the home (to be used as a swap)
 */
static bool borg_think_home_buy_swap_weapon(void)
{
    int hole;

    int slot;
    int old_weapon_swap;
    s32b old_weapon_swap_value;
    int old_armour_swap;
    s32b old_armour_swap_value;
    int n, b_n = -1;
    s32b p, b_p = 0L;

    bool fix = FALSE;


    /* save the current values */
    old_weapon_swap = weapon_swap;
    old_weapon_swap_value =  weapon_swap_value;
    old_armour_swap = armour_swap;
    old_armour_swap_value =  armour_swap_value;

    if (weapon_swap <= 0 || weapon_swap_value <=0)
    {
        hole = INVEN_MAX_PACK - 1;
        weapon_swap_value = -1L;
    }
    else
    {
        hole = weapon_swap;
    }

    /* Extract the "power" */
    b_p = weapon_swap_value;

    /* Scan the home */
    for (n = 0; n < STORE_INVEN_MAX; n++)
    {
        borg_item *item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Obtain "slot" make sure its a weapon */
        slot = borg_wield_slot(item);
        if (slot != INVEN_WIELD) continue;

        /* Save shop item */
        COPY(&safe_shops[7].ware[n], &borg_shops[7].ware[n], borg_item);

        /* Save hole */
        COPY(&safe_items[hole], &borg_items[hole], borg_item);

        /* Remove one item from shop */
        borg_shops[7].ware[n].iqty--;


        /* Consider new equipment */
        if (slot == INVEN_WIELD)
        {
            /* Move new item into inventory */
            COPY(&borg_items[hole], &safe_shops[7].ware[n], borg_item);

            /* Only a single item */
            borg_items[hole].iqty = 1;

            /* Fix later */
            fix = TRUE;

            /* Examine the iventory and swap value*/
            borg_notice(TRUE);

            /* Evaluate the new equipment */
            p = weapon_swap_value;
        }

        /* Restore hole */
        COPY(&borg_items[hole], &safe_items[hole], borg_item);

        /* Restore shop item */
        COPY(&borg_shops[7].ware[n], &safe_shops[7].ware[n], borg_item);

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and value */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Buy something */
    if ((b_n >= 0) && (b_p > weapon_swap_value))
    {
        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Restore the values */
        weapon_swap = old_weapon_swap;
        weapon_swap_value =  old_weapon_swap_value;
        armour_swap = old_armour_swap;
        armour_swap_value =  old_armour_swap_value;

        /* Success */
        return (TRUE);
    }

    /* Restore the values */
        weapon_swap = old_weapon_swap;
        weapon_swap_value =  old_weapon_swap_value;
        armour_swap = old_armour_swap;
        armour_swap_value =  old_armour_swap_value;

    /* Nope */
    return (FALSE);
}
/*
 * Step 7B -- buy "useful" armour from the home (to be used as a swap)
 */
static bool borg_think_home_buy_swap_armour(void)
{
    int hole;

    int slot;

    int n, b_n = -1;
    s32b p, b_p = 0L;
    bool fix = FALSE;
    int old_weapon_swap;
    s32b old_weapon_swap_value;
    int old_armour_swap;
    s32b old_armour_swap_value;



    /* save the current values */
    old_weapon_swap = weapon_swap;
    old_weapon_swap_value =  weapon_swap_value;
    old_armour_swap = armour_swap;
    old_armour_swap_value =  armour_swap_value;

    if (armour_swap <= 1 || armour_swap_value <=0 )
    {
        hole = INVEN_MAX_PACK - 1;
        armour_swap_value = -1L;
    }
    else
    {
        hole = armour_swap;
    }


    /* Extract the "power" */
    b_p = armour_swap_value;


    /* Scan the home */
    for (n = 0; n < STORE_INVEN_MAX; n++)
    {
        borg_item *item = &borg_shops[7].ware[n];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Obtain "slot".  Elimination of non armours in borg4.c*/
        slot = borg_wield_slot(item);


        /* Save shop item */
        COPY(&safe_shops[7].ware[n], &borg_shops[7].ware[n], borg_item);

        /* Save hole */
        COPY(&safe_items[hole], &borg_items[hole], borg_item);

        /* Remove one item from shop */
        borg_shops[7].ware[n].iqty--;

        /* Move new item into inventory */
        COPY(&borg_items[hole], &safe_shops[7].ware[n], borg_item);

        /* Only a single item */
        borg_items[hole].iqty = 1;

        /* Fix later */
        fix = TRUE;

        /* Examine the inventory (false)*/
        borg_notice(TRUE);

        /* Evaluate the new equipment */
        p = armour_swap_value;

        /* Restore hole */
        COPY(&borg_items[hole], &safe_items[hole], borg_item);

        /* Restore shop item */
        COPY(&borg_shops[7].ware[n], &safe_shops[7].ware[n], borg_item);

        /* Ignore "silly" purchases */
        if (p <= b_p) continue;

        /* Save the item and value */
        b_n = n; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Buy something */
    if ((b_n >= 0) && (b_p > armour_swap_value))
    {
        /* Go to the home */
        goal_shop = 7;

        /* Buy that item */
        goal_ware = b_n;

        /* Restore the values */
        weapon_swap = old_weapon_swap;
        weapon_swap_value =  old_weapon_swap_value;
        armour_swap = old_armour_swap;
        armour_swap_value =  old_armour_swap_value;

        /* Success */
        return (TRUE);
    }
    /* Restore the values */
    weapon_swap = old_weapon_swap;
    weapon_swap_value =  old_weapon_swap_value;
    armour_swap = old_armour_swap;
    armour_swap_value =  old_armour_swap_value;

    /* Nope */
    return (FALSE);
}




/*
 * Choose a shop to visit (see above)
 */
static bool borg_choose_shop(void)
{
    int i;


    /* Must be in town */
    if (borg_skill[BI_CDEPTH]) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);
    if (time_this_panel > 1350) return (FALSE);

	/* Already flowing to a store to sell something */
	if (goal_shop != -1 && goal_ware != -1) return (TRUE);

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
	borg_notice(TRUE);


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
                         f_info[0x08+goal_shop].name));

			/* Set my Quick Shopping flag */
			borg_needs_quick_shopping = TRUE;

            /* Success */
            return (TRUE);
        }

        /* if temple is out of healing stuff, try the house */
        if (borg_think_home_buy_aux())
        {
            /* Message */
            borg_note(format("# Buying '%s' from the home immediately.",
                         borg_shops[goal_shop].ware[goal_ware].desc));

            /* Success */
            return (TRUE);
        }
    }

#if 0
	/* Must have visited all shops first---complete information */
    for (i = 0; i < (MAX_STORES); i++)
    {
        borg_shop *shop = &borg_shops[i];

        /* Skip "visited" shops */
        if (!shop->when && !goal_shop) return (FALSE);
    }
#endif

    /* if we are already flowing toward a shop do not check again... */
    if (goal_shop != -1 && goal_ware != -1)
        return TRUE;

    /* Assume no important shop */
    goal_shop = goal_ware = goal_item = -1;
    borg_needs_quick_shopping= FALSE;

    /* if the borg is scumming for cash for the human player and not himself,
     * we dont want him messing with the home inventory
     */
    if (borg_gold < borg_money_scum_amount && borg_money_scum_amount != 0 &&
            !borg_skill[BI_CDEPTH] && borg_skill[BI_LIGHT] && !borg_self_scum)
    {
        /* Step 0 -- Buy items from the shops (for the player while scumming) */
        if (borg_think_shop_buy_aux())
        {
            /* Message */
            borg_note(format("# Buying '%s' at '%s' (money scumming)",
                             borg_shops[goal_shop].ware[goal_ware].desc,
                             f_info[0x08+goal_shop].name));

            /* Success */
            return (TRUE);
        }
        else return (FALSE);
    }

    /* Step 1 -- Sell items to the home */
    if (borg_think_home_sell_aux( FALSE ))
    {
        /* Message */
        if (goal_item != -1)
            borg_note(format("# Selling '%s' to the home",
                             borg_items[goal_item].desc));
        else
            borg_note(format("# Buying '%s' from the home (step 1)",
                             borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (TRUE);
    }


    /* Step 2 -- Sell items to the shops */
    if (borg_think_shop_sell_aux())
    {
        /* Message */
        borg_note(format("# Selling '%s' at '%s'",
                         borg_items[goal_item].desc,
                         f_info[0x08+goal_shop].name));

        /* Success */
        return (TRUE);
    }

    /* Step 3 -- Buy items from the shops (for the player) */
    if (borg_think_shop_buy_aux())
    {

        /* Message */
        borg_note(format("# Buying '%s'(%c) at '%s' (for player 'b')",
                         borg_shops[goal_shop].ware[goal_ware].desc,shop_orig[goal_ware],
                         f_info[0x08+goal_shop].name));

        /* Success */
        return (TRUE);
    }


    /* Step 4 -- Buy items from the home (for the player) */
    if (borg_think_home_buy_aux())
    {
        /* Message */
        borg_note(format("# Buying '%s' from the home (step 4)",
                         borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (TRUE);
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
        return (TRUE);
    }

	/* Do not Stock Up the home while money scumming */
	if (borg_money_scum_amount) return (FALSE);

    /* Step 6 -- Buy items from the shops (for the home) */
    if (borg_think_shop_grab_aux())
    {
        /* Message */
        borg_note(format("# Grabbing (for home) '%s' at '%s'",
                         borg_shops[goal_shop].ware[goal_ware].desc,
                         f_info[0x08+goal_shop].name));

        /* Success */
        return (TRUE);
    }

    /* Step 7A -- Buy weapons from the home (as a backup item) */
    if (borg_uses_swaps && borg_think_home_buy_swap_weapon())
    {
        /* Message */
        borg_note(format("# Buying '%s' from the home as a backup",
                         borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (TRUE);
    }
    /* Step 7B -- Buy armour from the home (as a backup item) */
    if (borg_uses_swaps && borg_think_home_buy_swap_armour())
    {
        /* Message */
        borg_note(format("# Buying '%s' from the home as a backup",
                         borg_shops[goal_shop].ware[goal_ware].desc));

        /* Success */
        return (TRUE);
    }


    /* Failure */
    return (FALSE);

}




/*
 * Sell items to the current shop, if desired
 */
static bool borg_think_shop_sell(void)
{
    int qty= 1;


    /* Sell something if requested */
    if ((goal_shop == shop_num) && (goal_item >= 0))
    {
        borg_item *item = &borg_items[goal_item];

        qty = borg_min_item_quantity(item);

        /* Remove the inscription */
        /* 309 does not allow for } while in the store */
        /* if (item->tval == TV_FOOD) borg_send_deinscribe(goal_item); */


        /* Log */
        borg_note(format("# Selling %s", item->desc));

        /* Buy an item */
        borg_keypress('s');

        /* Buy the desired item */
        borg_keypress(I2A(goal_item));

        /* Hack -- Sell a single item */
        if (item->iqty > 1 || qty >= 2)
        {
            if (qty == 5) borg_keypress('5');
            if (qty == 4) borg_keypress('4');
            if (qty == 3) borg_keypress('3');
            if (qty == 2) borg_keypress('2');
	        borg_keypress('\n');
        }

        /* Mega-Hack -- Accept the price */
        if (goal_shop != 7)
		{
			borg_keypress('\n');
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
		sold_item_nxt ++;


        /* The purchase is complete */
        goal_shop = goal_ware = goal_item = -1;

        /* tick the anti-loop clock */
        time_this_panel ++;

		/* I'm not in a store */
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_in_shop = FALSE;
		borg_do_inven = TRUE;
		/* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Buy items from the current shop, if desired
 */
static bool borg_think_shop_buy(void)
{
    int qty =1;
	char purchase_target = '0';

    /* Buy something if requested */
    if ((goal_shop == shop_num) && (goal_ware >= 0))
    {
        borg_shop *shop = &borg_shops[goal_shop];

        borg_item *item = &shop->ware[goal_ware];

        qty = borg_min_item_quantity(item);

		purchase_target = shop_orig[goal_ware];

        /* Paranoid */
        if (item->tval == 0)
        {
            /* The purchase is complete */
			goal_shop = goal_ware = goal_item = -1;

            /* Increment our clock to avoid loops */
            time_this_panel ++;

            return (FALSE);
        }

        /* Log */
        borg_note(format("# Buying %s.", item->desc));

        /* Buy the desired item */
        borg_keypress(purchase_target);
        borg_keypress('p');


        /* Mega-Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress(' ');
        borg_keypress(' ');
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

		/* if the borg is scumming and bought it.,
		 * reset the scum amount.
		 */
		if (borg_money_scum_amount &&
			(item->cost >= borg_money_scum_amount * 9 / 10))
		{
			borg_money_scum_amount = 0;

	        /* Log */
	        borg_note(format("# Setting Money Scum to %s.", borg_money_scum_amount));

		}


        /* Remember what we bought to avoid buy/sell loops */
		if (bought_item_nxt >= 9) bought_item_nxt = 0;
		bought_item_pval[bought_item_nxt] = item->pval;
        bought_item_tval[bought_item_nxt] = item->tval;
        bought_item_sval[bought_item_nxt] = item->sval;
        bought_item_store[bought_item_nxt] = goal_shop;
		bought_item_num = bought_item_nxt;
		bought_item_nxt ++;

        /* The purchase is complete */
        goal_shop = goal_ware = goal_item = -1;

		/* Increment our clock to avoid loops */
        time_this_panel ++;

        /* leave the store */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

		/* I'm not in a store */
		borg_in_shop = FALSE;

        /* Success */
        return (TRUE);
    }

    /* Nothing to buy */
    return (FALSE);
}


/*
 * Deal with being in a store
 */
bool borg_think_store(void)
{
    /* Hack -- prevent clock wrapping */
    if (borg_t >= 20000 && borg_t <=20010)
    {
        /* Clear Possible errors */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Re-examine inven and equip */
        borg_do_inven = TRUE;
        borg_do_equip = TRUE;

    }

    /* update all my equipment and swap items */
	borg_do_inven = TRUE;
	borg_do_equip = TRUE;
	borg_notice(TRUE);

#if 0
	/* Stamp the shop with a time stamp */
    borg_shops[shop_num].when = borg_t;
#endif

    /* Wear "optimal" equipment */
    if (borg_best_stuff()) return (TRUE);

    /* If using a digger, Wear "useful" equipment.
     * unless that digger is an artifact, then treat
     * it as a normal weapon
     */
    if (borg_items[INVEN_WIELD].tval == TV_DIGGING &&
        !borg_items[INVEN_WIELD].name1 &&
        borg_wear_stuff()) return (TRUE);

    /* Choose a shop to visit.  Goal_shop indicates he is trying to sell something somewhere. */
    if (borg_choose_shop())
    {
		/* Note Pref. */
		borg_note(format("# Currently in store '%d' would prefere '%d'.",shop_num+1,goal_shop+1));

       	/* Try to sell stuff */
		if (borg_think_shop_sell()) return (TRUE);

        /* Try to buy stuff */
        if (borg_think_shop_buy()) return (TRUE);
	}

    /* No shop */
	goal_shop = goal_ware = goal_item = -1;


    /* Leave the store */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);


    /* Done */
    return (TRUE);
}

/* Attempt a series of maneuvers to stay alive when you run out of light */
bool borg_think_dungeon_light(void)
{
	int ii, x, y;
	bool not_safe = FALSE;
    borg_grid *ag;

	/* Consume needed things */
	if (borg_skill[BI_ISHUNGRY] && borg_use_things()) return (TRUE);

	if (!borg_skill[BI_LIGHT] &&
		(borg_skill[BI_CURLITE] <= 0 || borg_items[INVEN_LIGHT].timeout <= 3) &&
		borg_skill[BI_CDEPTH] >= 1)
    {
        /* I am recalling, sit here till it engages. */
        if (goal_recalling)
        {
            /* just wait */
            borg_keypress('R');
            borg_keypress('9');
            borg_keypress('\n');
            return (TRUE);
        }

        /* wear stuff and see if it glows */
        if (borg_wear_stuff()) return (TRUE);
	    if (borg_wear_quiver()) return (TRUE);

        /* attempt to refuel */
        if (borg_refuel_torch() || borg_refuel_lantern()) return (TRUE);

        /* Can I recall out with a rod */
        if (!goal_recalling && borg_zap_rod(SV_ROD_RECALL)) return (TRUE);

        /* Can I recall out with a spell */
        if (!goal_recalling && borg_recall()) return (TRUE);

		/* Log */
		borg_note("# Testing for stairs .");

        /* Test for stairs */
        borg_keypress('<');

		/* If on a glowing grid, got some food, and low mana, then rest here */
		if ((borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] && borg_skill[BI_MAXSP] > 0) &&
		    (borg_grids[c_y][c_x].info & BORG_GLOW) &&
		     !borg_skill[BI_ISWEAK] &&
			(borg_prayer_legal(1, 5) || borg_spell_legal(2, 0) ||
			 borg_skill[BI_FOOD] >= borg_skill[BI_CDEPTH]) &&
		    (borg_prayer_legal(0, 4) || borg_spell_legal(0, 3)) &&
		    (!borg_prayer_okay(0, 4) && !borg_spell_okay(0, 3)))
		{
			/* Scan grids adjacent to me */
			for (ii = 0; ii < 8; ii++)
			{
	           	x = c_x + ddx_ddd[ii];
	           	y = c_y + ddy_ddd[ii];

				/* Bounds check */
	            if (!in_bounds_fully(y,x)) continue;

	           	/* Access the grid */
	           	ag = &borg_grids[y][x];

	           	/* Check for adjacent Monster */
	           	if (ag->kill)
	           	{
					not_safe = TRUE;
				}
			}

			/* Be concerned about the Regional Fear. */
			if (borg_fear_region[c_y/11][c_x/11] > borg_skill[BI_CURHP] / 10) not_safe = TRUE;

			/* Be concerned about the Monster Fear. */
			if (borg_fear_monsters[c_y][c_x] > borg_skill[BI_CURHP] / 10) not_safe = TRUE;

			/* rest here to gain some mana */
			if (!not_safe)
			{
				borg_note("# Resting on this Glowing Grid to gain mana.");
        		borg_keypress('R');
	        	borg_keypress('*');
	        	borg_keypress('\n');
	        	return (TRUE);
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
            if (!in_bounds_fully(y,x)) continue;

           	/* Access the grid */
           	ag = &borg_grids[y][x];

			/* skip the Wall grids */
			if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_PERM_SOLID) continue;

			/* Problem with casting Call Light on Open Doors */
			if ((ag->feat == FEAT_OPEN || ag->feat == FEAT_BROKEN) &&
				(y == c_y && x == c_x))
			{
				/* Cheat the grid info to see if the door is lit */
				if (cave->feat[c_y][c_x] == CAVE_GLOW) ag->info |= BORG_GLOW;
				continue;
			}

			/* Look for a dark one */
        	if ((ag->info & BORG_DARK) || /* Known to be dark */
				 ag->feat == FEAT_NONE || /* Nothing known about feature */
				 !(ag->info & BORG_MARK) || /* Nothing known about info */
				 !(ag->info & BORG_GLOW))   /* not glowing */
        	{
				/* Attempt to Call Light */
	        	if (borg_activate_artifact(EFF_ILLUMINATION, INVEN_LIGHT) ||
	        	    borg_zap_rod(SV_ROD_ILLUMINATION) ||
	        	    borg_use_staff(SV_STAFF_LIGHT) ||
	        	    borg_read_scroll(SV_SCROLL_LIGHT) ||
	        	    borg_spell(0, 3) ||
	        	    borg_prayer(0, 4))
	        	{
	        	    borg_note("# Illuminating the region while dark.");
		            borg_react("SELF:lite", "SELF:lite");

	        	    return (TRUE);
	        	}

				/* Attempt to use Light Beam requiring a direction. */
				if (borg_LIGHT_beam(FALSE)) return (TRUE);
			}
		}


        /* Try to flow to upstairs if on level one */
        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
        {
            /* Take the stairs */
			/* Log */
			borg_note("# Taking up Stairs stairs (low Light).");
            borg_keypress('<');
            return (TRUE);
        }

        /* Try to flow to a lite */
        if (borg_skill[BI_RECALL] && borg_flow_light(GOAL_FLEE))
        {
            return (TRUE);
        }

    }
	/* Nothing to do */
	return (FALSE);
}
/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to grab items close to the stair.
 */
bool borg_think_stair_scum(bool from_town)
{
	int j, b_j = -1;
	int i;

	borg_grid *ag = &borg_grids[c_y][c_x];

    byte feat = cave->feat[c_y][c_x];

	borg_item *item = &borg_items[INVEN_LIGHT];

    /* examine equipment and swaps */
    borg_notice(TRUE);

	/* No scumming mode if starving or in town */
	if (borg_skill[BI_CDEPTH] == 0 ||
		borg_skill[BI_ISWEAK])
	{
		borg_note("# Leaving Scumming Mode. (Town or Weak)");
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* No scumming if inventory is full.  Require one empty slot */
    if (borg_items[INVEN_MAX_PACK-1].iqty) return (FALSE);

	/* if borg is just starting on this level, he may not
	 * know that a stair is under him.  Cheat to see if one is
	 * there
	 */
	if (feat == FEAT_MORE && ag->feat != FEAT_MORE)
	{

       /* Check for an existing "down stairs" */
       for (i = 0; i < track_more_num; i++)
       {
           /* We already knew about that one */
           if ((track_more_x[i] == c_x) && (track_more_y[i] == c_y)) break;
       }

       /* Track the newly discovered "down stairs" */
       if ((i == track_more_num) && (i < track_more_size))
       {
          track_more_x[i] = c_x;
          track_more_y[i] = c_y;
          track_more_num++;
       }
       /* tell the array */
       ag->feat = FEAT_MORE;

	}

	if (feat == FEAT_LESS && ag->feat != FEAT_LESS)
	{

       /* Check for an existing "up stairs" */
       for (i = 0; i < track_less_num; i++)
       {
            /* We already knew about this one */
            if ((track_less_x[i] == c_x) && (track_less_y[i] == c_y)) continue;
		}

	   /* Track the newly discovered "up stairs" */
 	   if ((i == track_less_num) && (i < track_less_size))
       {
          track_less_x[i] = c_x;
          track_less_y[i] = c_y;
          track_less_num++;
       }

		/* Tell the array */
       ag->feat = FEAT_LESS;

	}

	/** First deal with staying alive **/

    /* Hack -- require light */
    if (item->tval == TV_LIGHT &&
		(item->sval == SV_LIGHT_TORCH || item->sval == SV_LIGHT_LANTERN))
    {

        /* Must have light -- Refuel current torch */
        if ((item->tval == TV_LIGHT) && (item->sval == SV_LIGHT_TORCH))
        {
            /* Try to refuel the torch */
            if ((item->timeout < 500) &&
                 borg_refuel_torch()) return (TRUE);
        }

        /* Must have light -- Refuel current lantern */
        if ((item->tval == TV_LIGHT) && (item->sval == SV_LIGHT_LANTERN))
        {
            /* Try to refill the lantern */
            if ((item->timeout < 1000) && borg_refuel_lantern()) return (TRUE);
        }

        if (item->timeout < 250)
        {
            borg_note("# Scum. (need fuel)");
        }
    }


	/** Track down some interesting gear **/
/* XXX Should we allow him great flexibility in retreiving loot? (not always safe?)*/
    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a (viewable) object */
    if (borg_flow_take_scum(TRUE, 6)) return (TRUE);

	/*leave level right away. */
	borg_note("# Fleeing level. Scumming Mode");
	goal_fleeing = TRUE;

    /* Scumming Mode - Going down */
    if (track_more_num &&
        (ag->feat == FEAT_MORE ||
         ag->feat == FEAT_LESS ||
         borg_skill[BI_CDEPTH] < 30))
    {
        int y, x;

		if (track_more_num >= 2) borg_note("# Scumming Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more_num; i++)
        {
            x = track_more_x[i];
            y = track_more_y[i];

            /* How far is the nearest down stairs */
            j = distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }


        /* if the downstair is close and path is safe, continue on */
        if (b_j < 8  ||
             ag->feat == FEAT_MORE ||
             borg_skill[BI_CDEPTH] < 30)
		{
        	/* Note */
        	borg_note("# Scumming Mode.  Power Diving. ");

	       	/* Continue leaving the level */
        	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to DownStair */
        	if (borg_flow_stair_more(GOAL_FLEE, FALSE, FALSE)) return (TRUE);

			/* if standing on a stair */
			if (ag->feat == FEAT_MORE)
			{
				/* Take the DownStair */
	        	borg_on_upstairs = TRUE;
	        	borg_keypress('>');

	        	return (TRUE);
			}
		}
    }

    /* Scumming Mode - Going up */
    if (track_less_num && borg_skill[BI_CDEPTH] != 1 &&
        (ag->feat == FEAT_MORE ||
         ag->feat == FEAT_LESS))
    {
        int y, x;

	    borg_grid *ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }

        /* if the upstair is close and safe path, continue */
        if (b_j < 8 ||
        	ag->feat == FEAT_LESS)
        {

	        /* Note */
	        borg_note("# Scumming Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

	        /* Continue leaving the level */
	        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to UpStair */
	        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
	        {
                borg_note("# Looking for stairs. Scumming Mode.");

		        /* Success */
		        return (TRUE);
			}

			if (ag->feat == FEAT_LESS)
			{
				/* Take the Up Stair */
		        borg_on_dnstairs = TRUE;
		        borg_keypress('<');
				return (TRUE);
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
    	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Note */
        borg_note("# Scumming Mode.  Any Stair. ");

    	/* Try to find some stairs */
    	if (borg_flow_stair_both(GOAL_FLEE, TRUE)) return (TRUE);
	}

	/* return to normal borg_think_dungeon */
	return (FALSE);
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
bool borg_think_dungeon_lunal(void)
{
	bool safe_place = FALSE;

	int j, b_j = -1;
	int i;

	borg_grid *ag = &borg_grids[c_y][c_x];

    byte feat = cave->feat[c_y][c_x];

	borg_item *item = &borg_items[INVEN_LIGHT];

    /* examine equipment and swaps */
    borg_notice(TRUE);

	/* No Lunal mode if starving or in town */
	if (borg_skill[BI_CDEPTH] == 0 ||
		borg_skill[BI_ISWEAK])
	{
		borg_note("# Leaving Lunal Mode. (Town or Weak)");
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* if borg is just starting on this level, he may not
	 * know that a stair is under him.  Cheat to see if one is
	 * there
	 */
	if (feat == FEAT_MORE && ag->feat != FEAT_MORE)
	{

       /* Check for an existing "down stairs" */
       for (i = 0; i < track_more_num; i++)
       {
           /* We already knew about that one */
           if ((track_more_x[i] == c_x) && (track_more_y[i] == c_y)) break;
       }

       /* Track the newly discovered "down stairs" */
       if ((i == track_more_num) && (i < track_more_size))
       {
          track_more_x[i] = c_x;
          track_more_y[i] = c_y;
          track_more_num++;
       }
       /* tell the array */
       ag->feat = FEAT_MORE;

	}

	if (feat == FEAT_LESS && ag->feat != FEAT_LESS)
	{

       /* Check for an existing "up stairs" */
       for (i = 0; i < track_less_num; i++)
       {
            /* We already knew about this one */
            if ((track_less_x[i] == c_x) && (track_less_y[i] == c_y)) continue;
		}

	   /* Track the newly discovered "up stairs" */
 	   if ((i == track_less_num) && (i < track_less_size))
       {
          track_less_x[i] = c_x;
          track_less_y[i] = c_y;
          track_less_num++;
       }

		/* Tell the array */
       ag->feat = FEAT_LESS;

	}

	/* Act normal on 1 unless stairs are seen*/
	if (borg_skill[BI_CDEPTH] == 1 && track_more_num == 0)
	{
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* If no down stair is known, act normal */
	if (track_more_num ==0 && track_less_num == 0)
	{
		borg_note("# Leaving Lunal Mode. (No Stairs seen)");
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* If self scumming and getting closer to zone, act normal */
    if (borg_self_lunal)
    {
		if (borg_skill[BI_MAXDEPTH] <= borg_skill[BI_CDEPTH] + 15 ||
    	    (cptr)NULL != borg_prepared(borg_skill[BI_CDEPTH] - 5) ||
    	    borg_skill[BI_CDEPTH] >= 50 ||
	        borg_skill[BI_CDEPTH] == 0 ||
			borg_skill[BI_ISWEAK])
    	{
			borg_lunal_mode = FALSE;
			goal_fleeing = FALSE;
			goal_fleeing_lunal = FALSE;
			borg_note("# Self Lunal mode disengaged normally.");
			return (FALSE);
		}
	}


	/** First deal with staying alive **/

    /* Hack -- require light */
    if (item->tval == TV_LIGHT &&
		(item->sval == SV_LIGHT_TORCH || item->sval == SV_LIGHT_LANTERN))
    {

        /* Must have light -- Refuel current torch */
        if ((item->tval == TV_LIGHT) && (item->sval == SV_LIGHT_TORCH))
        {
            /* Try to refuel the torch */
            if ((item->timeout < 500) &&
                 borg_refuel_torch()) return (TRUE);
        }

        /* Must have light -- Refuel current lantern */
        if ((item->tval == TV_LIGHT) && (item->sval == SV_LIGHT_LANTERN))
        {
            /* Try to refill the lantern */
            if ((item->timeout < 1000) && borg_refuel_lantern()) return (TRUE);
        }

        if (item->timeout < 250)
        {
            borg_note("# Lunal. (need fuel)");
        }
    }

	/* No Light at all */
	if (borg_skill[BI_CURLITE] == 0 && borg_items[INVEN_LIGHT].tval == 0)
	{
		borg_note("# No Light at all.");
		return (FALSE);
	}

	/* Define if safe_place is true or not */
	safe_place = borg_check_rest(c_y, c_x);

	/* Light Room, looking for monsters */
	/* if (safe_place && borg_check_LIGHT_only()) return (TRUE); */

	/* Check for stairs and doors and such */
	/* if (safe_place && borg_check_LIGHT()) return (TRUE); */

	/* Recover from any nasty condition */
	if (safe_place && borg_recover()) return (TRUE);

	/* Consume needed things */
	if (safe_place && borg_use_things()) return (TRUE);

	/* Consume needed things */
	if (borg_skill[BI_ISHUNGRY] && borg_use_things()) return (TRUE);

	/* Crush junk if convienent */
	if (safe_place && borg_crush_junk()) return (TRUE);

	/** Track down some interesting gear **/
/* XXX Should we allow him great flexibility in retreiving loot? (not always safe?)*/
    /* Continue flowing towards objects */
    if (safe_place && borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a (viewable) object */
    if (safe_place && borg_flow_take_lunal(TRUE, 4)) return (TRUE);

	/*leave level right away. */
	borg_note("# Fleeing level. Lunal Mode");
	goal_fleeing_lunal = TRUE;
	goal_fleeing = TRUE;

    /* Full of Items - Going up */
    if (track_less_num && borg_items[INVEN_MAX_PACK-2].iqty &&
        (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS))
    {
        int y, x;
		int closeness = 8;

	    borg_grid *ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }

        /* if on depth 1, try to venture more to get back to town */
		if (borg_skill[BI_CDEPTH] == 1)
		{
			if (track_less_num)
			{
				closeness = 20;
			}
		}

		/* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
        	ag->feat == FEAT_LESS)
        {

	        /* Note */
	        borg_note("# Lunal Mode.  Power Climb (needing to sell). ");

			/* Set to help borg move better */
			goal_less = TRUE;

	        /* Continue leaving the level */
	        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to UpStair */
	        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
	        {
                borg_note("# Looking for stairs. Lunal Mode (needing to sell).");

		        /* Success */
		        return (TRUE);
			}

			if (ag->feat == FEAT_LESS)
			{
				/* Take the Up Stair */
		        borg_on_dnstairs = TRUE;
		        borg_keypress('<');
				return (TRUE);
			}

		}
	}

    /* Lunal Mode - Going down */
    if (track_more_num &&
        (safe_place || ag->feat == FEAT_MORE ||
         ag->feat == FEAT_LESS ||
         borg_skill[BI_CDEPTH] < 30))
    {
        int y, x;

		if (track_more_num >= 2) borg_note("# Lunal Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more_num; i++)
        {
            x = track_more_x[i];
            y = track_more_y[i];

            /* How far is the nearest down stairs */
            j = distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }


        /* if the downstair is close and path is safe, continue on */
        if ((b_j < 8 && safe_place) ||
             ag->feat == FEAT_MORE ||
             borg_skill[BI_CDEPTH] < 30)
		{
        	/* Note */
        	borg_note("# Lunal Mode.  Power Diving. ");

	       	/* Continue leaving the level */
        	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to DownStair */
        	if (borg_flow_stair_more(GOAL_FLEE, TRUE, FALSE)) return (TRUE);

			/* if standing on a stair */
			if (ag->feat == FEAT_MORE)
			{
				/* Take the DownStair */
	        	borg_on_upstairs = TRUE;
	        	borg_keypress('>');

	        	return (TRUE);
			}
		}
    }

    /* Lunal Mode - Going up */
    if (track_less_num && borg_skill[BI_CDEPTH] != 1 &&
        (safe_place || ag->feat == FEAT_MORE ||
         ag->feat == FEAT_LESS))
    {
        int y, x;

	    borg_grid *ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < 8 && safe_place) ||
        	ag->feat == FEAT_LESS)
        {

	        /* Note */
	        borg_note("# Lunal Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

	        /* Continue leaving the level */
	        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to UpStair */
	        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
	        {
                borg_note("# Looking for stairs. Lunal Mode.");

		        /* Success */
		        return (TRUE);
			}

			if (ag->feat == FEAT_LESS)
			{
				/* Take the Up Stair */
		        borg_on_dnstairs = TRUE;
		        borg_keypress('<');
				return (TRUE);
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
    	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Note */
        borg_note("# Lunal Mode.  Any Stair. ");

    	/* Try to find some stairs */
    	if (borg_flow_stair_both(GOAL_FLEE, TRUE)) return (TRUE);
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
	borg_lunal_mode = FALSE;
	goal_fleeing = goal_fleeing_lunal = FALSE;
	return (FALSE);
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
bool borg_think_dungeon_munchkin(void)
{
	bool safe_place = FALSE;
	int bb_j = MAX_RANGE;
	int j, b_j = -1;
	int i,ii, x, y;
	int closeness = 8;
	bool adjacent_monster = FALSE;

	borg_grid *ag = &borg_grids[c_y][c_x];

    byte feat = cave->feat[c_y][c_x];

	borg_item *item = &borg_items[INVEN_LIGHT];

    /* examine equipment and swaps */
    borg_notice(TRUE);

	/* Not if starving or in town */
	if (borg_skill[BI_CDEPTH] == 0 ||
		borg_skill[BI_ISWEAK])
	{
		borg_note("# Leaving munchkin Mode. (Town or Weak)");
		borg_munchkin_mode = FALSE;
		return (FALSE);
	}

	/* if borg is just starting on this level, he may not
	 * know that a stair is under him.  Cheat to see if one is
	 * there
	 */
	if (feat == FEAT_MORE && ag->feat != FEAT_MORE)
	{

       /* Check for an existing "down stairs" */
       for (i = 0; i < track_more_num; i++)
       {
           /* We already knew about that one */
           if ((track_more_x[i] == c_x) && (track_more_y[i] == c_y)) break;
       }

       /* Track the newly discovered "down stairs" */
       if ((i == track_more_num) && (i < track_more_size))
       {
          track_more_x[i] = c_x;
          track_more_y[i] = c_y;
          track_more_num++;
       }
       /* tell the array */
       ag->feat = FEAT_MORE;

	}

	if (feat == FEAT_LESS && ag->feat != FEAT_LESS)
	{

       /* Check for an existing "up stairs" */
       for (i = 0; i < track_less_num; i++)
       {
            /* We already knew about this one */
            if ((track_less_x[i] == c_x) && (track_less_y[i] == c_y)) continue;
		}

	   /* Track the newly discovered "up stairs" */
 	   if ((i == track_less_num) && (i < track_less_size))
       {
          track_less_x[i] = c_x;
          track_less_y[i] = c_y;
          track_less_num++;
       }

		/* Tell the array */
       ag->feat = FEAT_LESS;

	}

	/* Act normal on 1 unless stairs are seen*/
	if (borg_skill[BI_CDEPTH] == 1 && track_more_num == 0)
	{
		borg_munchkin_mode = FALSE;
		return (FALSE);
	}

	/* If no down stair is known, act normal */
	if (track_more_num ==0 && track_less_num == 0)
	{
		borg_note("# Leaving Munchkin Mode. (No Stairs seen)");
		borg_munchkin_mode = FALSE;
		return (FALSE);
	}

	/** First deal with staying alive **/

    /* Hack -- require light */
    if (item->tval == TV_LIGHT &&
		(item->sval == SV_LIGHT_TORCH || item->sval == SV_LIGHT_LANTERN))
    {

        /* Must have light -- Refuel current torch */
        if ((item->tval == TV_LIGHT) && (item->sval == SV_LIGHT_TORCH))
        {
            /* Try to refuel the torch */
            if ((item->timeout < 500) &&
                 borg_refuel_torch()) return (TRUE);
        }

        /* Must have light -- Refuel current lantern */
        if ((item->tval == TV_LIGHT) && (item->sval == SV_LIGHT_LANTERN))
        {
            /* Try to refill the lantern */
            if ((item->timeout < 1000) && borg_refuel_lantern()) return (TRUE);
        }

        if (item->timeout < 250)
        {
            borg_note("# Munchkin. (need fuel)");
        }
    }

	/* No Light at all */
	if (borg_skill[BI_CURLITE] == 0)
	{
		borg_note("# No Light at all.");
	}

	/* Define if safe_place is true or not */
	safe_place = borg_check_rest(c_y, c_x);

	/* Can do a little attacking. */
	if (borg_munchkin_mage()) return (TRUE);
	if (borg_munchkin_melee()) return (TRUE);

	/* Consume needed things */
	if (safe_place && borg_use_things()) return (TRUE);

	/* Consume needed things */
	if (borg_skill[BI_ISHUNGRY] && borg_use_things()) return (TRUE);

    /* Wear stuff and see if it's good */
	if (safe_place && borg_wear_stuff()) return (TRUE);
    if (safe_place && borg_wear_quiver()) return (TRUE);
	if (safe_place && borg_remove_stuff()) return (TRUE);

	/* Crush junk if convienent */
	if (safe_place && borg_crush_junk()) return (TRUE);

	/* Learn learn and test useful spells */
	if (safe_place && borg_play_magic(TRUE)) return (TRUE);

	/** Track down some interesting gear **/
/* XXX Should we allow him great flexibility in retreiving loot? (not always safe?)*/
    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

	/* Borg may be off the stair and a monster showed up. */

    /* Find a (viewable) object */
    if (safe_place && borg_flow_take_lunal(TRUE, 5)) return (TRUE);

	/* Recover from any nasty condition */
	if (safe_place && borg_recover()) return (TRUE);

	/*leave level right away. */
	borg_note("# Fleeing level. Munchkin Mode");
	goal_fleeing_munchkin = TRUE;
	goal_fleeing = TRUE;

	/* Increase the range of the borg a bit */
	if (borg_skill[BI_CDEPTH] <= 10) closeness += (borg_skill[BI_CLEVEL] - 10) +
		(10 - borg_skill[BI_CDEPTH]);

    /* Full of Items - Going up */
    if (track_less_num && (borg_items[INVEN_PACK-2].iqty) &&
        (safe_place || ag->feat == FEAT_LESS || borg_skill[BI_CURLITE] == 0))
    {
        int y, x;

	    borg_grid *ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }

        /* if on depth 1, try to venture more to get back to town */
		if (borg_skill[BI_CDEPTH] == 1)
		{
			if (track_less_num)
			{
				closeness = 20;
			}
		}

		/* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
        	ag->feat == FEAT_LESS)
        {

	        /* Note */
	        borg_note("# Munchkin Mode.  Power Climb (needing to sell). ");

			/* Set to help borg move better */
			goal_less = TRUE;

	        /* Continue leaving the level */
	        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to UpStair */
	        if (borg_flow_stair_less(GOAL_FLEE, TRUE))
	        {
                borg_note("# Looking for stairs. Munchkin Mode (needing to sell).");

		        /* Success */
		        return (TRUE);
			}

			if (ag->feat == FEAT_LESS)
			{
				/* Take the Up Stair */
		        borg_on_dnstairs = TRUE;
		        borg_keypress('<');
				return (TRUE);
			}

		}
	}

    /* Too deep. trying to gradually move shallow.  Going up */
    if ((track_less_num && borg_skill[BI_CDEPTH] > borg_munchkin_depth) && (safe_place || ag->feat == FEAT_LESS))
    {

	    borg_grid *ag = &borg_grids[c_y][c_x];

		/* Reset */
		b_j = -1;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
			if (b_j < bb_j) bb_j = b_j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
        	ag->feat == FEAT_LESS)
        {

	        /* Note */
	        borg_note("# Munchkin Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

	        /* Continue leaving the level */
	        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to UpStair */
	        if (borg_flow_stair_less(GOAL_FLEE, TRUE))
	        {
                borg_note("# Looking for stairs. Munchkin Mode.");

		        /* Success */
		        return (TRUE);
			}

			if (ag->feat == FEAT_LESS)
			{
				/* Take the Up Stair */
		        borg_on_dnstairs = TRUE;
		        borg_keypress('<');
				return (TRUE);
			}

		}
    }

    /* Going down */
    if ((track_more_num && borg_skill[BI_CDEPTH] < borg_munchkin_depth) &&
        ( safe_place || ag->feat == FEAT_MORE))
    {
        int y, x;

		/* Reset */
		b_j = -1;

		if (track_more_num >= 1) borg_note("# Munchkin Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more_num; i++)
        {
            x = track_more_x[i];
            y = track_more_y[i];

            /* How far is the nearest down stairs */
            j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }


        /* if the downstair is close and path is safe, continue on */
        if ((b_j < closeness && safe_place) || ag->feat == FEAT_MORE || borg_skill[BI_CDEPTH] == 1)
		{
        	/* Note */
        	borg_note("# Munchkin Mode.  Power Diving. ");

	       	/* Continue leaving the level */
        	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to DownStair */
        	if (borg_flow_stair_more(GOAL_FLEE, TRUE, FALSE)) return (TRUE);

			/* if standing on a stair */
			if (ag->feat == FEAT_MORE)
			{
				/* Take the DownStair */
	        	borg_on_upstairs = TRUE;
	        	borg_keypress('>');

	        	return (TRUE);
			}
		}
    }

    /* Going up */
    if ((track_less_num && borg_skill[BI_CDEPTH] != 1 &&
        safe_place) || ag->feat == FEAT_LESS)
    {
        int y, x;

	    borg_grid *ag = &borg_grids[c_y][c_x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x)) continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) ||
        	ag->feat == FEAT_LESS)
        {

	        /* Note */
	        borg_note("# Munchkin Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

	        /* Continue leaving the level */
	        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to UpStair */
	        if (borg_flow_stair_less(GOAL_FLEE, TRUE))
	        {
                borg_note("# Looking for stairs. Munchkin Mode.");

		        /* Success */
		        return (TRUE);
			}

			if (ag->feat == FEAT_LESS)
			{
				/* Take the Up Stair */
		        borg_on_dnstairs = TRUE;
		        borg_keypress('<');
				return (TRUE);
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
    	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Note */
        borg_note("# Munchkin Mode.  Any Stair. ");


		/* Adjacent Monster.  Either attack it, or try to outrun it */
		for (i = 0;  i < 8; i++)
		{
			y = c_y + ddy_ddd[i];
			x = c_x + ddx_ddd[i];

			/* Bounds check */
			if (!in_bounds(y, x)) continue;

			/* Get the grid */
			ag = &borg_grids[y][x];

			/* Monster is adjacent to the borg */
			if (ag->kill)
			{
				/* Check for an existing "up stairs" */
				for (ii = 0; ii < track_less_num; ii++)
				{
					x = track_less_x[ii];
					y = track_less_y[ii];

					/* How far is the nearest up stairs */
					j = distance(c_y, c_x, y, x);

					/* Is it reachable or behind a wall? */
					if (!borg_projectable(y, x, c_y, c_x)) continue;

					/* skip the far ones */
					if (b_j <= j && b_j != -1) continue;

					/* track it */
					b_j =j;
				}

				/* Check for an existing "down stairs" */
				for (ii = 0; ii < track_more_num; ii++)
				{
					x = track_more_x[ii];
					y = track_more_y[ii];

					/* How far is the nearest down stairs */
					j = distance(c_y, c_x, y, x);

					/* Is it reachable or behind a wall? */
					if (!borg_projectable(y, x, c_y, c_x)) continue;

					/* skip the far ones */
					if (b_j <= j && b_j != -1) continue;

					/* track it */
					b_j =j;
				}

				/* Can the borg risk the run? */
				if (b_j <= 3)
				{
    				/* Try to find some stairs */
    				if (borg_flow_stair_both(GOAL_FLEE, FALSE)) return (TRUE);
				}
				else
				{
    				/* Try to kill it */
    				if (borg_attack(FALSE)) return (TRUE);
				}
			} /* Adjacent to kill */
		} /* Scanning neighboring grids */

		/* Try to find some stairs */
		if (borg_flow_stair_both(GOAL_FLEE, FALSE)) return (TRUE);
		if (ag->feat == FEAT_LESS)
		{
			/* Take the Up Stair */
	        borg_on_dnstairs = TRUE;
	        borg_keypress('<');
			return (TRUE);
		}
		if (ag->feat == FEAT_MORE)
		{
			/* Take the Stair */
	        borg_on_upstairs = TRUE;
	        borg_keypress('<');
			return (TRUE);
		}
	}

	/* return to normal borg_think_dungeon */
	borg_note("Leaving Munchkin Mode. (End of Mode)");
	borg_munchkin_mode = FALSE;
	goal_fleeing = goal_fleeing_munchkin = FALSE;
	return (FALSE);
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
	int p1 =  borg_danger(c_y, c_x, 1, TRUE, FALSE);

	/* Try a defence manuever on 100 */
	if (borg_skill[BI_CDEPTH] == 100 &&
	    borg_defend(p1)) return TRUE;

    /* Attack monsters */
    if (borg_attack(TRUE)) return (TRUE);

    /* Cast a light beam to remove fear of an area */
    if (borg_LIGHT_beam(FALSE)) return (TRUE);

    /*** Flee (or leave) the level ***/

    /* Take stairs down */
    /* Usable stairs */
	if (borg_grids[c_y][c_x].feat == FEAT_MORE)
    {
        /* Take the stairs */
        borg_on_upstairs = TRUE;
        borg_note("# Fleeing via stairs.");
        borg_keypress('>');

        /* Success */
        return (TRUE);
    }

    /* Return to Stairs, but not use them */
    if (goal_less)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Try to find some stairs */
        if (scaryguy_on_level && !borg_skill[BI_CDEPTH] && borg_flow_stair_both(GOAL_FLEE, FALSE)) return (TRUE);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
        {
                borg_note("# Looking for stairs. Goal_less, brave.");
				return (TRUE);
		}
    }


    /* Flee the level */
    if (goal_fleeing || goal_leaving || scaryguy_on_level)
    {
        /* Hack -- Take the next stairs */
        stair_less = goal_fleeing;

        if (borg_ready_morgoth == 0)
            stair_less = TRUE;

		if (stair_less == TRUE)
		{
			borg_note("# Fleeing and leaving the level. Brave Thinking.");
		}

        /* Go down if fleeing or prepared. */
        stair_more = goal_fleeing;
        if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH]+1))
            stair_more = TRUE;

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Try to find some stairs up */
        if (stair_less)
            if (borg_flow_stair_less(GOAL_FLEE, FALSE))
            {
                borg_note("# Looking for stairs. Flee, brave.");
				return (TRUE);
			}

        /* Try to find some stairs down */
        if (stair_more)
            if (borg_flow_stair_more(GOAL_FLEE, FALSE, TRUE)) return (TRUE);

    }

    /* Do short looks on special levels */
    if (vault_on_level)
    {
        /* Continue flowing towards monsters */
        if (borg_flow_old(GOAL_KILL)) return (TRUE);

        /* Find a (viewable) monster */
        if (borg_flow_kill(TRUE, 35)) return (TRUE);

        /* Continue flowing towards objects */
        if (borg_flow_old(GOAL_TAKE)) return (TRUE);

        /* Find a (viewable) object */
        if (borg_flow_take(TRUE, 35)) return (TRUE);
        if (borg_flow_vein(TRUE, 35)) return (TRUE);

		/* Continue to dig out a vault */
		if (borg_flow_old(GOAL_VAULT)) return (TRUE);

		/* Find a vault to excavate */
		if (borg_flow_vault(35)) return (TRUE);
	}

    /* Continue flowing towards monsters */
    if (borg_flow_old(GOAL_KILL)) return (TRUE);

    /* Find a (viewable) monster */
    if (borg_flow_kill(TRUE, 250)) return (TRUE);

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a (viewable) object */
    if (borg_flow_take(TRUE, 250)) return (TRUE);
    if (borg_flow_vein(TRUE, 250)) return (TRUE);

    /*** Exploration ***/

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_MISC)) return (TRUE);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_DARK)) return (TRUE);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_XTRA)) return (TRUE);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_BORE)) return (TRUE);


    /*** Explore the dungeon ***/

    /* Explore interesting grids */
    if (borg_flow_dark(TRUE)) return (TRUE);

    /* Explore interesting grids */
    if (borg_flow_dark(FALSE)) return (TRUE);

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
		if (borg_check_LIGHT()) return (TRUE);
	}

    /*** Track down old stuff ***/

    /* Chase old objects */
    if (borg_flow_take(FALSE,250)) return (TRUE);
    if (borg_flow_vein(FALSE,250)) return (TRUE);

    /* Chase old monsters */
    if (borg_flow_kill(FALSE,250)) return (TRUE);

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
		if (borg_check_LIGHT()) return (TRUE);
	}

	/* Attempt to leave the level */
    if (borg_leave_level(TRUE)) return (TRUE);

	/* Search for secret doors */
    if (borg_flow_spastic(TRUE)) return (TRUE);


    /* Nothing */
    return (FALSE);
}


/*
 * Perform an action in the dungeon
 *
 * Return TRUE if a "meaningful" action was performed
 * Otherwise, return FALSE so we will be called again
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
	int y,x;

    byte feat = cave->feat[c_y][c_x];


	/* Delay Factor */
    int msec = ((op_ptr->delay_factor * op_ptr->delay_factor) +
                (borg_delay_factor * borg_delay_factor));

	/* HACK allows user to stop the borg on certain levels */
    if (borg_skill[BI_CDEPTH] == borg_stop_dlevel) borg_oops("Auto-stop for user DLevel.");
    if (borg_skill[BI_CLEVEL] == borg_stop_clevel) borg_oops("Auto-stop for user CLevel.");

    /* HACK to end all hacks,,, allow the borg to stop if money scumming */
    if (borg_gold > borg_money_scum_amount && borg_money_scum_amount != 0 &&
        !borg_skill[BI_CDEPTH] && !borg_self_scum)
    {
        borg_oops("Money Scum complete.");
    }

    /* Hack -- Stop the borg if money scumming and the shops are out of food. */
    if (!borg_skill[BI_CDEPTH] && borg_money_scum_amount != 0 &&
        (borg_food_onsale == 0 && borg_skill[BI_FOOD] < 5))
    {
 		/* Town out of food.  If player initiated borg, stop here */
 		if (borg_self_scum == FALSE)
 		{
			borg_oops("Money Scum stopped.  No more food in shop.");
			return (TRUE);
		}
		else
		/* Borg doing it himself */
		{
			/* move money goal to 0 and leave the level */
			borg_money_scum_amount = 0;
		}
    }

    /* Hack -- prevent clock wrapping Step 1*/
    if ((borg_t >= 12000 && borg_t <=12025) ||
		(borg_t >= 25000 && borg_t <=25025))
    {
        /* Clear Possible errors */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Re-examine inven and equip */
        borg_do_inven = TRUE;
        borg_do_equip = TRUE;

        /* enter a special routine to handle this behavior.  Messing with
         * the old_level forces him to re-explore this level, and reshop,
         * if in town.
         */
        old_depth = 126;

        /* Continue on */
        return (TRUE);
    }

    /* Hack -- prevent clock wrapping Step 2*/
    if (borg_t >= 30000)
    {
        /* Panic */
        borg_oops("clock overflow");

#ifdef BABLOS
    /* Clock overflow escape code */
    printf("Clock overflow code!\n");
    p_ptr->playing = FALSE;
    p_ptr->leaving = TRUE;
    borg_clock_over = TRUE;
#endif /* BABLOS */

        /* Oops */
        return (TRUE);
    }

    /* Allow respawning borgs to update their variables */
    if (borg_respawning > 1)
    {
        borg_note(format("# Pressing 'escape' to catch up and get in sync (%d).", borg_respawning));
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_respawning --;
        return (TRUE);
    }

    /* add a short pause to slow the borg down for viewing */
    Term_xtra(TERM_XTRA_DELAY, msec);

    /* redraw the screen if we need to */
    if (my_need_redraw)
    {
        borg_note(format("#  Redrawing screen."));
        do_cmd_redraw();
        my_need_redraw = FALSE;
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
            goal_leaving = TRUE;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (boredom)");

            /* Start fleeing */
            goal_fleeing = TRUE;
        }
    }

    /* am I fighting a unique or a summoner, or scaryguy? */
    borg_near_monster_type(borg_skill[BI_MAXCLEVEL] < 15 ? MAX_SIGHT : 12);

    /* Allow borg to jump back up to town if needed.  He probably fled town because
	 * he saw a scaryguy (BSV, SER, Maggot).  Since he is here on depth 1, do a quick
	 * check for items near the stairs that I can pick up before I return to town.
	 */
    if (borg_skill[BI_CDEPTH] == 1 && borg_fleeing_town)
    {

		/* Try to grab a close item while I'm down here */
		if (borg_think_stair_scum(TRUE)) return (TRUE);

		/* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (finish shopping)");

            /* Start leaving */
            goal_leaving = TRUE;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (finish shopping)");

            /* Start fleeing */
            goal_fleeing = TRUE;
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
        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Forget old objects */
        C_WIPE(borg_takes, 256, borg_take);

        /* No monsters here */
        borg_kills_cnt = 0;
        borg_kills_nxt = 1;

        /* Forget old monsters */
        C_WIPE(borg_kills, 256, borg_kill);
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
            goal_leaving = TRUE;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (bouncing-borg)");

            /* Start fleeing */
            goal_fleeing = TRUE;
        }

    }


    /* Count the awake breeders */
    for (j = 0, i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill *kill = &borg_kills[i];

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
        breeder_level = TRUE;
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
            goal_ignoring = TRUE;
        }

        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (no recall)");

            /* Start leaving */
            goal_leaving = TRUE;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (no recall)");

            /* Start fleeing */
            goal_fleeing = TRUE;
        }


    }

    /* Reset avoidance */
    if (avoidance != borg_skill[BI_CURHP])
    {
        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = TRUE;

        /* Forget goals */
    }

    /* Keep borg on a short leash */
    if (track_less_num &&
        (borg_skill[BI_MAXHP] < 30 || borg_skill[BI_CLEVEL] < 15)  &&
		borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5)
    {
        int y, x;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less_num; i++)
        {
            x = track_less_x[i];
            y = track_less_y[i];

            /* How far is the nearest up stairs */
            j = distance(c_y, c_x, y, x);

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
                borg_note(format("# Return to Stair (wandered too far.  Leash: %d)",borg_skill[BI_CLEVEL] * 3 + 14));

                /* Start returning */
                goal_less = TRUE;
            }

        }

		/* Clear the flag to Return to the upstair-- we are close enough now */
        else if (goal_less && b_j < 3)
        {
            /* Note */
            borg_note("# Close enough to Stair.");

                /* Clear the flag */
                goal_less = FALSE;
				goal = 0;

        }
    }

	/* Quick check to see if borg needs to engage his lunal mode */
    if (borg_self_lunal && !borg_plays_risky)  /* Risky borg in a hurry */
    {
		if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 15) && /* Prepared */
			borg_skill[BI_MAXDEPTH] >= borg_skill[BI_CDEPTH] + 15 && /* Right zone */
			borg_skill[BI_CDEPTH] >=1  && /* In dungeon fully */
			borg_skill[BI_CDEPTH] > borg_skill[BI_CLEVEL] / 3) /* Not shallow */
	    {
			borg_lunal_mode = TRUE;

			/* Enter the Lunal scumming mode */
			if (borg_lunal_mode && borg_think_dungeon_lunal()) return (TRUE);
		}
	}

	/* Quick check to see if borg needs to engage his lunal mode for munchkin_start */
    if (borg_munchkin_start && borg_skill[BI_MAXCLEVEL] < 12)
    {
		if (borg_skill[BI_CDEPTH] >=1)
	    {
			borg_munchkin_mode = TRUE;

			/* Enter the Lunal scumming mode */
			if (borg_think_dungeon_munchkin()) return (TRUE);
		}

		/* Must not be in munchkin mode then */
		borg_munchkin_mode = FALSE;
	}

    /* Keep borg on a suitable level */
    if (track_less_num && borg_skill[BI_CLEVEL] < 10 &&
        !goal_less && (cptr)NULL != borg_prepared(borg_skill[BI_CDEPTH]))
    {
        /* Note */
        borg_note("# Needing to get back on correct depth");

        /* Start returning */
        goal_less = TRUE;

        /* Take stairs */
		if (borg_grids[c_y][c_x].feat == FEAT_LESS)
		{
			borg_keypress('<');
			return (TRUE);
		}
	}

	/*** crucial goals ***/

    /* examine equipment and swaps */
    borg_notice(TRUE);

    /* require light-- Special handle for being out of a light source.*/
	if (borg_think_dungeon_light()) return (TRUE);

    /* Decrease the amount of time not allowed to retreat */
    if (borg_no_retreat > 0)
        borg_no_retreat--;

    /*** Important goals ***/

	/* Continue flowing towards good anti-summon grid */
	if (borg_flow_old(GOAL_DIGGING)) return (TRUE);

    /* Try not to die */
    if (borg_caution()) return (TRUE);

	/*** if returning from dungeon in bad shape...***/
    if (borg_skill[BI_CURLITE] == 0 || borg_skill[BI_ISCUT] ||
        borg_skill[BI_ISPOISONED] || borg_skill[BI_FOOD] == 0)
    {
        /* First try to wear something */
        if (borg_skill[BI_CURLITE] == 0)
        {
            /* attempt to refuel */
            if (borg_refuel_torch() || borg_refuel_lantern()) return (TRUE);

            /* wear stuff and see if it glows */
            if (borg_wear_stuff()) return (TRUE);
            if (borg_wear_quiver()) return (TRUE);
        }

        /* Recover from damage */
        if (borg_recover()) return (TRUE);

		/* If full of items, we wont be able to buy stuff, crush stuff */
	    if (borg_items[INVEN_MAX_PACK-1].iqty && borg_crush_hole()) return (TRUE);

        /* shop for something that will help us */
        if (borg_flow_shop_visit()) return (TRUE);

        if (borg_choose_shop())
        {
            /* Try and visit a shop, if so desired */
            if (borg_flow_shop_entry(goal_shop)) return (TRUE);
        }
    }

	/* if I must go to town without delay */
    if ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH]))
    {
		if (borg_leave_level(FALSE)) return (TRUE);
	}

    /* Learn useful spells immediately */
    if (borg_play_magic(FALSE)) return (TRUE);

    /* If using a digger, Wear "useful" equipment before fighting monsters */
    if (borg_items[INVEN_WIELD].tval == TV_DIGGING && borg_wear_stuff()) return (TRUE);

    /* If not using anything, Wear "useful" equipment before fighting monsters */
    if (!borg_items[INVEN_WIELD].tval && borg_wear_stuff()) return (TRUE);

	/* If not wielding any missiles, load up the quiver */
	if (borg_items[INVEN_BOW].iqty && !borg_items[QUIVER_START].tval && borg_wear_quiver()) return (TRUE);

	/* Dig an anti-summon corridor */
    if (borg_flow_kill_corridor_1(TRUE)) return (TRUE);

    /* Attack monsters */
    if (borg_attack(FALSE)) return (TRUE);

    /* Wear things that need to be worn, but try to avoid swap loops */
    /* if (borg_best_stuff()) return (TRUE); */
    if (borg_wear_stuff()) return (TRUE);
    if (borg_wear_quiver()) return (TRUE);
    if (borg_swap_rings()) return (TRUE);
    if (borg_wear_rings()) return (TRUE);

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a really close object */
    if (borg_flow_take(TRUE, 5)) return (TRUE);

	/* Remove "backwards" rings */
    /* Only do this in Stores to avoid loops     if (borg_swap_rings()) return (TRUE); */

    /* Repair "backwards" rings */
    if (borg_wear_rings()) return (TRUE);

    /* Remove stuff that is useless or detrimental */
    if (borg_remove_stuff()) return (TRUE);
	if (borg_dump_quiver()) return (TRUE);
	if (borg_stack_quiver()) return (TRUE);

    /* Check the light */
    if (borg_check_LIGHT()) return (TRUE);

	/* Continue flowing to a safe grid on which I may recover */
	if (borg_flow_old(GOAL_RECOVER)) return (TRUE);

    /* Recover from damage */
    if (borg_recover()) return (TRUE);

	/* Attempt to find a grid which is safe and I can recover on it.  This should work closely with borg_recover. */
	if (borg_flow_recover(FALSE, 50)) return (TRUE);

    /* Perform "cool" perma spells */
    if (borg_perma_spell()) return (TRUE);

    /* Try to stick close to stairs if weak */
    if (borg_skill[BI_CLEVEL] < 10 && borg_skill[BI_MAXSP] &&
        borg_skill[BI_CURSP] == 0 && borg_no_rest_prep <= 1  &&
        !borg_bless && !borg_hero && !borg_berserk)
    {
        if (borg_skill[BI_CDEPTH])
        {
            int i, y, x;

            /* Check for an existing "up stairs" */
            for (i = 0; i < track_less_num; i++)
            {
                x = track_less_x[i];
                y = track_less_y[i];

                /* Not on a stair */
                if (c_y != y || c_x != x) continue;

                /* I am standing on a stair */

                /* reset the goal_less flag */
                goal_less = FALSE;

                /* if not dangerous, wait here */
                if (borg_danger(c_y,c_x,1, TRUE, FALSE) == 0)
                {
                    /* rest here a moment */
                    borg_note("# Resting on stair to gain Mana.");
                    borg_keypress(',');
                    return (TRUE);
                }
            }
        }
        else /* in town */
        {
            int i, y, x;

            /* Check for an existing "dn stairs" */
            for (i = 0; i < track_more_num; i++)
            {
                x = track_more_x[i];
                y = track_more_y[i];

                /* Not on a stair */
                if (c_y != y || c_x != x) continue;

                /* I am standing on a stair */

                /* if not dangerous, wait here */
                if (borg_danger(c_y,c_x,1, TRUE, FALSE) == 0)
                {
                    /* rest here a moment */
                    borg_note("# Resting on town stair to gain Mana.");
                    borg_keypress(',');
                    return (TRUE);
                }
            }
        }

        /* In town, standing on stairs, sit tight */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, TRUE))
        {
		    borg_note("# Looking for stairs. Stair hugging.");
			return (TRUE);
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
        goal_leaving = TRUE;

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Try to find some stairs down */
        if (borg_flow_stair_more(GOAL_FLEE, FALSE, FALSE)) return (TRUE);
	}

    /*** Flee the level XXX XXX XXX ***/

    /* Return to Stairs, but not use them */
    if (goal_less)
    {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Try to find some stairs */
        if (scaryguy_on_level && borg_flow_stair_both(GOAL_FLEE, FALSE)) return (TRUE);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
        {
			borg_note("# Looking for stairs. Goal_less, Fleeing.");
			return (TRUE);
		}
    }

    /* Flee the level */
    if (goal_fleeing && !goal_recalling)
    {
        /* Hack -- Take the next stairs */
        stair_less = stair_more = TRUE;
		borg_note("# Fleeing and leaving the level. (Looking for any stair)");

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

        /* Try to find some stairs */
        if (scaryguy_on_level &&
            borg_flow_stair_both(GOAL_FLEE, FALSE)) return (TRUE);

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, FALSE))
        {
			borg_note("# Looking for stairs. Fleeing.");
			return (TRUE);
		}

        /* Try to find some stairs down */
        if (borg_flow_stair_more(GOAL_FLEE, FALSE, FALSE)) return (TRUE);

    }

	/* Flee to a safe Morgoth grid if appropriate */
	if (!borg_skill[BI_KING] && morgoth_on_level && !borg_morgoth_position &&
	    (borg_skill[BI_AGLYPH] >= 10 &&
         (!borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCONFUSED])))
	{
		/* Continue flowing towards good morgoth grid */
		if (borg_flow_old(GOAL_MISC)) return (TRUE);

		/* Attempt to locate a good Glyphed grid */
		if (borg_flow_glyph(GOAL_MISC)) return (TRUE);

		/* Have the borg excavate the dungeon with Stone to Mud */

	}

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a really close object */
    if (borg_flow_take(TRUE, 5)) return (TRUE);
    if (borg_flow_vein(TRUE, 5)) return (TRUE);

	/* Continue flowing towards (the hopefully close) monsters */
    if (borg_flow_old(GOAL_KILL)) return (TRUE);

    /* Find a really close monster */
    if (borg_flow_kill(TRUE, 20)) return (TRUE);

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a really close object */
    if (borg_flow_take(FALSE, 10)) return (TRUE);
    if (borg_flow_vein(FALSE, 10)) return (TRUE);

    /* Continue flowing towards monsters */
    if (borg_flow_old(GOAL_KILL)) return (TRUE);

	/* Continue towards a vault */
	if (borg_flow_old(GOAL_VAULT)) return (TRUE);

    /* Find a viewable monster and line up a shot on him */
    if (borg_flow_kill_aim(TRUE)) return (TRUE);

	/*** Deal with inventory objects ***/

    /* check for anything that should be inscribed */
    /* if (borg_inscribe_food()) return (TRUE); */

    /* Use things */
    if (borg_use_things()) return (TRUE);

    /* Identify unknown things */
    if (borg_test_stuff()) return (TRUE);

    /* Enchant things */
    if (borg_enchanting()) return (TRUE);

    /* Recharge things */
    if (borg_recharging()) return (TRUE);

    /* Destroy junk */
    if (borg_crush_junk()) return (TRUE);

    /* Destroy items to make space */
    if (borg_crush_hole()) return (TRUE);

    /* Destroy items if we are slow */
    if (borg_crush_slow()) return (TRUE);


    /*** Flow towards objects ***/

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE)) return (TRUE);

    /* Find a (viewable) object */
    if (borg_flow_take(TRUE, 250)) return (TRUE);
    if (borg_flow_vein(TRUE, 250)) return (TRUE);


	/*** Leave the level XXX XXX XXX ***/


	/* Leave the level */
    if ((goal_leaving && !goal_recalling && !unique_on_level)  ||
		 (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
 	      borg_count_sell() >= 13))
    {
        /* Hack -- Take the next stairs */
        if (borg_ready_morgoth == 0)
        {
            borg_note("# Fleeing and leaving the level (Looking for Up Stair).");
			stair_less = TRUE;
		}

        /* Only go down if fleeing or prepared. */
        if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH]+1))
            stair_more = TRUE;

        /* Continue leaving the level */
        if (borg_flow_old(GOAL_FLEE)) return (TRUE);

		/* Try to find some stairs up */
        if (stair_less)
        {
            if (borg_flow_stair_less(GOAL_FLEE, FALSE))
            {
				borg_note("# Looking for stairs. Goal_Leaving.");

				return (TRUE);
			}
		}

		/* Only go up if needing to sell */
		if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
 	      borg_count_sell() >= 13) stair_more = FALSE;

		/* Try to find some stairs down */
        if (stair_more)
            if (borg_flow_stair_more(GOAL_FLEE, FALSE, FALSE)) return (TRUE);
    }

    /* Power dive if I am playing too shallow
     * This is also seen in leave_level().  If
     * this formula is modified here, change it
     * in leave_level too.
     */
    if (borg_skill[BI_CDEPTH] != 0 &&
        (cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 5) && !stair_less)
    {
        /* Take next stairs */
        stair_more = TRUE;

        /* Continue leaving the level */
        if (borg_flow_old(GOAL_BORE)) return (TRUE);

		/* No down if needing to sell */
		if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
 	      borg_count_sell() >= 13)
		{
			stair_more = FALSE;
		}

        /* Attempt to use those stairs */
        if (stair_more && borg_flow_stair_more(GOAL_BORE, TRUE, FALSE))
        {
			/* Leave a note */
			borg_note("# Powerdiving.");
			return (TRUE);
		}
    }

    /*** Exploration ***/

	/* Continue flowing (see below) */
    if (borg_flow_old(GOAL_MISC)) return (TRUE);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_DARK)) return (TRUE);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_XTRA)) return (TRUE);

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_BORE)) return (TRUE);

	if (borg_flow_old(GOAL_VAULT)) return (TRUE);


	/*** Explore the dungeon ***/

	if (vault_on_level)
    {

        /* Chase close monsters */
        if (borg_flow_kill(FALSE, MAX_RANGE + 1)) return (TRUE);

        /* Chase close objects */
        if (borg_flow_take(FALSE, 35)) return (TRUE);
        if (borg_flow_vein(FALSE, 35)) return (TRUE);

		/* Excavate a vault safely */
		if (borg_excavate_vault(MAX_RANGE-2)) return (TRUE);

		/* Find a vault to excavate */
		if (borg_flow_vault(35)) return (TRUE);

        /* Explore close interesting grids */
        if (borg_flow_dark(TRUE)) return (TRUE);
    }

    /* Chase old monsters */
    if (borg_flow_kill(FALSE, 250)) return (TRUE);

    /* Chase old objects */
    if (borg_flow_take(FALSE, 250)) return (TRUE);
    if (borg_flow_vein(FALSE, 250)) return (TRUE);

    /* Explore interesting grids */
    if (borg_flow_dark(TRUE)) return (TRUE);

    /* Leave the level (if needed) */
    if (borg_gold < borg_money_scum_amount && borg_money_scum_amount != 0 &&
            !borg_skill[BI_CDEPTH] && borg_skill[BI_LIGHT])
    {
        /* Stay in town and scum for money after shopping */
    }
    else
    {
        if (borg_leave_level(FALSE)) return (TRUE);
    }


    /* Explore interesting grids */
    if (borg_flow_dark(FALSE)) return (TRUE);


    /*** Deal with shops ***/

	/* Hack -- visit all the shops */
    if (borg_flow_shop_visit()) return (TRUE);

    /* Hack -- Visit the shops */
    if (borg_choose_shop())
    {
        /* Try and visit a shop, if so desired */
        if (borg_flow_shop_entry(goal_shop)) return (TRUE);
    }


    /*** Leave the Level ***/

    /* Study/Test boring spells/prayers */
    if (!goal_fleeing && borg_play_magic(TRUE)) return (TRUE);

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
		if (borg_check_LIGHT()) return (TRUE);
	}

    /* Search for secret doors */
    if (borg_flow_spastic(FALSE)) return (TRUE);

	/* Flow directly to a monster if not able to be spastic */
	if (borg_flow_kill_direct(FALSE, FALSE)) return (TRUE);

    /* Recharge items before leaving the level */
    if (borg_wear_recharge()) return (TRUE);

    /* Leave the level (if possible) */
    if (borg_gold < borg_money_scum_amount && borg_money_scum_amount != 0 &&
            !borg_skill[BI_CDEPTH] && borg_skill[BI_LIGHT] &&
            !borg_plays_risky) /* risky borgs are in a hurry */
    {
        /* Stay in town, scum for money now that shopping is done. */
        if (borg_money_scum()) return (TRUE);
    }
    else
    {
        if (borg_leave_level(TRUE)) return (TRUE);
    }

    /* Search for secret door via spell before spastic */
    if (!when_detect_doors || (borg_t - when_detect_doors >= 500))
    {
		if (borg_check_LIGHT()) return (TRUE);
	}

    /* Search for secret doors */
    if (borg_flow_spastic(TRUE)) return (TRUE);

	/* Flow directly to a monster if not able to be spastic */
	if (borg_flow_kill_direct(TRUE, FALSE)) return (TRUE);

    /*** Wait for recall ***/

    /* Wait for recall, unless in danger */
    if (goal_recalling && (borg_danger(c_y, c_x, 1, TRUE, FALSE) <= 0))
    {
        /* Take note */
        borg_note("# Waiting for Recall...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('9');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
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
        bool done = FALSE;

        /* Note */
        borg_note(format("# Boosting bravery (1) from %d to %d!",
                         avoidance, borg_skill[BI_CURHP] * 2));

        /* Hack -- ignore some danger */
        avoidance = (borg_skill[BI_CURHP] * 2);

        /* Forget the danger fields */
        borg_danger_wipe = TRUE;

        /* Try anything */
        if (borg_think_dungeon_brave()) done = TRUE;

        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = TRUE;

        /* Forget goals */
/*        goal = 0;*/

        /* Done */
        if (done) return (TRUE);
    }

    /* try phase before boosting bravery further and acting goofy */
    borg_times_twitch++;

    /* Phase to get out of being twitchy up to 3 times per level. */
    if (borg_times_twitch < 3)
    {
        borg_note("# Considering Phase (twitchy)");

        /* Phase */
        if (borg_spell(0, 2)  ||
            borg_prayer(4, 0) ||
            borg_activate_artifact(EFF_TELE_PHASE,INVEN_BODY)||
            borg_read_scroll(SV_SCROLL_PHASE_DOOR) ||
            borg_spell(1, 5) ||
            borg_prayer(1, 1) ||
            borg_prayer(4, 1))
        {
            /* Success */
            return (TRUE);
        }
    }

    /* Set a flag that the borg is not allowed */
    /*  to retreat for 10 rounds */
    borg_no_retreat = 10;

    /* Boost some more */
    if (avoidance < borg_skill[BI_MAXHP] * 4)
    {
        bool done = FALSE;

        /* Note */
        borg_note(format("# Boosting bravery (2) from %d to %d!",
                         avoidance, borg_skill[BI_MAXHP] * 4));

        /* Hack -- ignore some danger */
        avoidance = (borg_skill[BI_MAXHP] * 4);

        /* Forget the danger fields */
        borg_danger_wipe = TRUE;

        /* Try anything */
        if (borg_think_dungeon_brave()) done = TRUE;

        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = TRUE;

        /* Forget goals */
/*        goal = 0;*/

        /* Done */
        if (done) return (TRUE);
    }

    /* Boost a lot */
    if (avoidance < 30000)
    {
        bool done = FALSE;

        /* Note */
        borg_note(format("# Boosting bravery (3) from %d to %d!",
                         avoidance, 30000));

        /* Hack -- ignore some danger */
        avoidance = 30000;

        /* Forget the danger fields */
        borg_danger_wipe = TRUE;

		/* Reset multiple factors to jumpstart the borg */
        unique_on_level = 0;
        scaryguy_on_level = FALSE;

        /* reset our breeder flag */
        breeder_level = FALSE;

        /* Forget goals */
        goal = 0;

        /* Hack -- cannot rise past town */
        if (!borg_skill[BI_CDEPTH]) goal_rising = FALSE;

        /* Assume not ignoring monsters */
        goal_ignoring = FALSE;

        /* No known stairs */
        track_less_num = 0;
        track_more_num = 0;

        /* No known glyph */
        track_glyph_num = 0;

        /* No known steps */
        track_step_num = 0;

        /* No known doors */
        track_door_num = 0;

        /* No known doors */
        track_closed_num = 0;

		/* No mineral veins */
		track_vein_num = 0;

        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Try anything */
        if (borg_think_dungeon_brave()) done = TRUE;

        /* Reset "avoidance" */
        avoidance = borg_skill[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = TRUE;

        /* Done */
        if (done) return (TRUE);
    }

    /* try teleporting before acting goofy */
    borg_times_twitch++;

    /* Teleport to get out of being twitchy up to 5 times per level. */
    if (borg_times_twitch < 5)
    {

        /* Teleport */
        if ( borg_spell(1, 5) ||
             borg_prayer(4, 1) ||
             borg_prayer(1, 1) ||
             borg_use_staff(SV_STAFF_TELEPORTATION) ||
             borg_read_scroll(SV_SCROLL_TELEPORT) ||
             borg_read_scroll(SV_SCROLL_TELEPORT_LEVEL) )
        {
            /* Success */
	        borg_note("# Teleport (twitchy)");
            return (TRUE);
        }
    }

    /* Recall to town */
    if (borg_skill[BI_CDEPTH] && (borg_recall()))
    {
        /* Note */
        borg_note("# Recalling (twitchy)");

        /* Success */
        return (TRUE);
    }


	/* Reset multiple factors to jumpstart the borg */
    unique_on_level = 0;
    scaryguy_on_level = FALSE;

    /* reset our breeder flag */
    breeder_level = FALSE;

    /* No objects here */
    borg_takes_cnt = 0;
    borg_takes_nxt = 1;

    /* No monsters here */
    borg_kills_cnt = 0;
    borg_kills_nxt = 1;

	/* Attempt to dig to the center of the dungeon */
	if (borg_flow_kill_direct(TRUE, TRUE)) return (TRUE);

    /* Twitch around */
    if (borg_twitchy()) return (TRUE);

    /* Oops */
    return (FALSE);
}




/*
 * Initialize this file
 */
void borg_init_8(void)
{
    /* Nothing */
}



#ifdef MACINTOSH
static int HACK = 0;
#endif
