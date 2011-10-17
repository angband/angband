/* File: borg7.c */
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


/*
 * This file handles various high level inventory related goals.
 *
 * Problems:
 *   Use "time stamps" (and not "random" checks) for several routines,
 *   including "kill junk" and "wear stuff", and maybe even "free space".
 *   But be careful with the "free space" routine, wear stuff first.
 *   Make sure nothing is "destroyed" if we do not do them every turn.
 *   Consider some special routines in stores (and in the home).
 *
 * Hack -- We should perhaps consider wearing "harmless" items into empty
 * slots when in the dungeon, to allow rings/amulets to be brought back up
 * to town to be sold.
 *
 * We should take account of possible combinations of equipment.  This may
 * be a potentially expensive computation, but could be done occasionally.
 * It is important to use a "state-less" formula to allow the exchange to
 * be spread over multiple turns.
 *
 * Hack -- We should attempt to only collect non-discounted items, at least
 * for the "expensive" slots, such as books, since we do not want to lose
 * value due to stacking.  We seem to sell the non-discounted items first,
 * and to buy the discounted items first, since they are cheap.  Oh well,
 * we may just be stuck with using discounted books.  Unless we actually
 * do correct "combining" in the simulations, and reward free slots.  Ick!
 *
 * XXX XXX XXX We really need a better "twitchy" function.
 *
 * XXX XXX XXX We need a better "flee this level" function
 *
 * XXX XXX XXX We need to stockpile possible useful items at home.
 *
 * XXX XXX XXX Perhaps we could simply maintain a list of abilities
 * that we might need at some point, such as the ability to identify, and
 * simply allow the Borg to "sell" items to the home which satisfy this
 * desire for "abilities".
 *
 * XXX XXX XXX Also, we should probably attempt to track the "best" item
 * in the home for each equipment slot, using some form of heuristic, and
 * reward that item based on its power, so that the Borg would always
 * have a "backup" item in case of disenchantment.
 *
 * XXX XXX XXX Also, we could reward equipment based on possible enchantment,
 * up to the maximal amount available in the home, which would induce item
 * switching when the item could be enchanted sufficiently.
 *
 * Fleeing from fast spell-casters is probably not a very smart idea, nor is
 * fleeing from fast monsters, nor is attempting to clear a room full of fast
 * moving breeding monsters, such as lice.
 */



/*
 * Hack -- importance of the various "level feelings"
 * Try to explore the level for at least this many turns
 */
static s16b value_feeling[] =
{
    500,
    8000,
    8000,
    6000,
    4000,
    2000,
    1000,
    800,
    600,
    400,
    200,
    0
};




/*
 * Determine if an item is "probably" worthless
 *
 * This (very heuristic) function is a total hack, designed only to prevent
 * a very specific annoying situation described below.
 *
 * Note that a "cautious" priest (or low level mage/ranger) will leave town
 * with a few identify scrolls, wander around dungeon level 1 for a few turns,
 * and use all of the scrolls on leather gloves and broken daggers, and must
 * then return to town for more scrolls.  This may repeat indefinitely.
 *
 * The problem is that some characters (priests, mages, rangers) never get an
 * "average" feeling about items, and have no way to keep track of how long
 * they have been holding a given item for, so they cannot even attempt to
 * gain knowledge from the lack of "good" or "cursed" feelings.  But they
 * cannot afford to just identify everything they find by using scrolls of
 * identify, because, in general, some items are, on average, "icky", and
 * not even worth the price of a new scroll of identify.
 *
 * Even worse, the current algorithm refuses to sell un-identified items, so
 * the poor character will throw out all his good stuff to make room for crap.
 *
 * This function simply examines the item and assumes that certain items are
 * "icky", which is probably a total hack.  Perhaps we could do something like
 * compare the item to the item we are currently wearing, or perhaps we could
 * analyze the expected value of the item, or guess at the likelihood that the
 * item might be a blessed, or something.
 *
 */
bool borg_item_icky(borg_item *item)
{
    int slot;


    /* if its average, dump it if you want to.*/
    if (strstr(item->note, "average")) return (TRUE);

    /* Mega-Hack -- allow "icky" items */
    if (borg_class == CLASS_PRIEST ||
        borg_class == CLASS_RANGER ||
        borg_class == CLASS_MAGE ||
        borg_skill[BI_CLEVEL] < 20)
    {
        /* things that are good/excelent/special */
        if (strstr(item->note, "special") ||
            strstr(item->note, "terrible") ||
            strstr(item->note, "indestructible") ||
            strstr(item->note, "excellent"))
            /* not icky */
            return (FALSE);

#if 0
	 	/* Broken dagger/sword, Filthy rag */
       if (((item->tval == TV_SWORD) && (item->sval == SV_BROKEN_DAGGER)) ||
            ((item->tval == TV_SWORD) && (item->sval == SV_BROKEN_SWORD)) ||
            ((item->tval == TV_SOFT_ARMOR) && (item->sval == SV_FILTHY_RAG)))
        {
            return (TRUE);
        }
#endif
        /* Dagger */
        if ((item->tval == TV_SWORD) && (item->sval == SV_DAGGER))
        {
            return (TRUE);
        }

        /* Sling (and I already got one) */
        if ((item->tval == TV_BOW) && (item->sval == SV_SLING) &&
        	borg_items[INVEN_BOW].tval == TV_BOW)
        {
            return (TRUE);
        }

        /* Cloak, (and I already got one)*/
        if ((item->tval == TV_CLOAK) && (item->sval == SV_CLOAK) &&
        	borg_items[INVEN_OUTER].tval == TV_CLOAK)
        {
            return (TRUE);
        }

        /* Robe (and I already got one)*/
        if ((item->tval == TV_SOFT_ARMOR) && (item->sval == SV_ROBE) &&
        	borg_items[INVEN_BODY].tval >= TV_SOFT_ARMOR)
        {
            return (TRUE);
        }

        /* Leather Gloves (and I already got one)*/
        if ((item->tval == TV_GLOVES) &&
            (item->sval == SV_SET_OF_LEATHER_GLOVES) &&
        	borg_items[INVEN_HANDS].tval == TV_GLOVES)
        {
            return (TRUE);
        }

        /* Assume the item is not icky */
        return (FALSE);
    }

    /* Process other classes which do get pseudo ID */
        /* things that are good/excelent/special/no P-ID */
        if  (strstr(item->note, "special") ||
             strstr(item->note, "terrible") ||
             strstr(item->note, "excellent") ||
             strstr(item->note, "ego") ||
             strstr(item->note, "splendid") ||
             strstr(item->note, "indestructible") ||
             !item->note )  /* no pseudo-id yet */
             /* not icky */
             return (FALSE);


        /*** {magical} items in inven, But I have {excellent} in equip ***/

        if (strstr(item->note, "magical"))
        {
            /* Obtain the slot of the suspect item */
            slot = borg_wield_slot(item);

			/* safety check incase slot = -1 */
			if (slot < 0) return (FALSE);

            /* Obtain my equipped item in the slot */
            item = &borg_items[slot];

            /* Is my item an ego or artifact? */
            if (item->name2 || item->name1) return (TRUE);
        }
    /* Assume not icky, I should have extra ID for the item */
    return (FALSE);
}




/*
 * Use things in a useful, but non-essential, manner
 */
bool borg_use_things(void)
{
    int i;

    /* Quaff experience restoration potion */
    if (borg_skill[BI_ISFIXEXP] &&
       (borg_prayer(6,4) ||
        borg_activate_artifact(EFF_RESTORE_LIFE, INVEN_OUTER) ||
        borg_quaff_potion(SV_POTION_RESTORE_EXP)))
    {
        return (TRUE);
    }

    /* just drink the stat gains, at this dlevel we wont need cash */
    if ( borg_quaff_potion(SV_POTION_INC_STR) ||
         borg_quaff_potion(SV_POTION_INC_INT) ||
         borg_quaff_potion(SV_POTION_INC_WIS) ||
         borg_quaff_potion(SV_POTION_INC_DEX) ||
         borg_quaff_potion(SV_POTION_INC_CON) ||
         borg_quaff_potion(SV_POTION_INC_CHR))
    {
        return (TRUE);
    }

    /* Quaff potions of "restore" stat if needed */
    if ( (borg_skill[BI_ISFIXSTR] &&
         (borg_quaff_potion(SV_POTION_RES_STR) ||
          borg_quaff_potion(SV_POTION_INC_STR) ||
          borg_eat_food(SV_FOOD_PURGING)||
          borg_eat_food(SV_FOOD_RESTORING))) ||
        (borg_skill[BI_ISFIXINT] &&
         (borg_quaff_potion(SV_POTION_RES_INT) ||
          borg_quaff_potion(SV_POTION_INC_INT) ||
          borg_eat_food(SV_FOOD_RESTORING))) ||
        (borg_skill[BI_ISFIXWIS] &&
         (borg_quaff_potion(SV_POTION_RES_WIS) ||
          borg_quaff_potion(SV_POTION_INC_WIS) ||
          borg_eat_food(SV_FOOD_RESTORING))) ||
        (borg_skill[BI_ISFIXDEX] &&
         (borg_quaff_potion(SV_POTION_RES_DEX) ||
          borg_quaff_potion(SV_POTION_INC_DEX) ||
          borg_eat_food(SV_FOOD_RESTORING))) ||
        (borg_skill[BI_ISFIXCON] &&
         (borg_quaff_potion(SV_POTION_RES_CON) ||
          borg_quaff_potion(SV_POTION_INC_CON) ||
          borg_eat_food(SV_FOOD_PURGING) ||
          borg_eat_food(SV_FOOD_RESTORING))) ||
        ((borg_skill[BI_ISFIXCHR]) &&
         (borg_quaff_potion(SV_POTION_RES_CHR) ||
          borg_quaff_potion(SV_POTION_INC_CHR)||
          borg_eat_food(SV_FOOD_RESTORING))))
    {
        return (TRUE);
    }


    /* Use some items right away */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Process "force" items */
        switch (item->tval)
        {
            case TV_POTION:
            {
                /* Check the scroll */
                switch (item->sval)
                {
                    case SV_POTION_ENLIGHTENMENT:
                        /* Never quaff these in town */
                        if (!borg_skill[BI_CDEPTH]) break;

                    case SV_POTION_INC_ALL:
                        /* Try quaffing the potion */
                        if (borg_quaff_potion(item->sval)) return (TRUE);
                }

                break;
            }
            case TV_SCROLL:
            {
                /* Hack -- check Blind/Confused */
                if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) break;

                /* XXX XXX XXX Dark */

                /* Check the scroll */
                switch (item->sval)
                {
                    case SV_SCROLL_MAPPING:
                    case SV_SCROLL_DETECT_TRAP:
                    case SV_SCROLL_DETECT_DOOR:
                    case SV_SCROLL_ACQUIREMENT:
                    case SV_SCROLL_STAR_ACQUIREMENT:
                    {
                        /* Never read these in town */
                        if (!borg_skill[BI_CDEPTH]) break;

                        /* Try reading the scroll */
                        if (borg_read_scroll(item->sval)) return (TRUE);
                        break;
                    }
                }

                break;
            }
        }
    }

    /* Eat food */
    if (borg_skill[BI_ISHUNGRY])
    {
        /* Attempt to satisfy hunger */
        if (borg_spell(2, 0) ||
            borg_prayer(1, 5)||
            borg_eat_food(SV_FOOD_SLIME_MOLD)||
            borg_eat_food(SV_FOOD_WAYBREAD) ||
            borg_eat_food(SV_FOOD_RATION))
        {
            return (TRUE);
        }
    }


    /* Nothing to do */
    return (FALSE);
}



/*
 * Refuel, call lite, detect traps/doors/walls/evil, etc
 *
 * Note that we refuel whenever our lite starts to get low.
 *
 * Note that we detect traps/doors/walls/evil at least once in each
 * panel, as soon as possible after entering a new panel.
 *
 * Note that we call lite whenever the current grid is dark, and
 * all the grids touching the current grid diagonally are known
 * floors, which catches all rooms, including "checkerboard" rooms,
 * and only occasionally calls lite in corridors, and then only once.
 *
 * Note that we also sometimes call lite whenever we are using a
 * lantern or artifact lite, and when all of the grids in the box
 * of grids containing the maximal torch-lit region (the 5x5 or 7x7
 * region centered at the player) are non-glowing floor grids, and
 * when at least one of them is known to be "dark".  This catches
 * most of the "probable rooms" before the standard check succeeds.
 *
 * We use the special "SELF" messages to "borg_react()" to delay the
 * processing of "detection" and "call lite" until we know if it has
 * worked or not.
 *
 * The matching function borg_check_LIGHT_only is used only with resting
 * to heal.  I don't want him teleporting into a room, resting to heal while
 * there is a dragon sitting in a dark corner waiting to breathe on him.
 * So now he will check for lite.
 *
 */
bool borg_check_LIGHT(void)
{
    int i, x, y;
    int corners, floors;
	int floor_goal;
	byte feat;

    int q_x, q_y;

    borg_grid *ag;


    bool do_LIGHT;

    bool do_trap;
    bool do_door;
    bool do_wall;
    bool do_evil;

    bool do_LIGHT_aux = FALSE;

    /* Never in town when mature (scary guy)*/
    if (borg_skill[BI_MAXCLEVEL] > 10 && !borg_skill[BI_CDEPTH]) return (FALSE);

    /* Never when comprimised, save your mana */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE] || borg_skill[BI_ISPOISONED] ||
        borg_skill[BI_ISCUT] || borg_skill[BI_ISWEAK]) return (FALSE);

    /* XXX XXX XXX Dark */


    /* Extract the panel */
    q_x = w_x / PANEL_WID;
    q_y = w_y / PANEL_HGT;


    /* Start */
    do_trap = FALSE;

    /* Determine if we need to detect traps */
    if (!borg_detect_trap[q_y+0][q_x+0]) do_trap = TRUE;
    if (!borg_detect_trap[q_y+0][q_x+1]) do_trap = TRUE;
    if (!borg_detect_trap[q_y+1][q_x+0]) do_trap = TRUE;
    if (!borg_detect_trap[q_y+1][q_x+1]) do_trap = TRUE;

    /* Hack -- check traps every few turns anyway */
    /* if (!when_detect_traps || (borg_t - when_detect_traps >= 183)) do_trap = TRUE; */


    /* Start */
    do_door = FALSE;

    /* Determine if we need to detect doors */
    if (!borg_detect_door[q_y+0][q_x+0]) do_door = TRUE;
    if (!borg_detect_door[q_y+0][q_x+1]) do_door = TRUE;
    if (!borg_detect_door[q_y+1][q_x+0]) do_door = TRUE;
    if (!borg_detect_door[q_y+1][q_x+1]) do_door = TRUE;

    /* Hack -- check doors every few turns anyway */
    /* if (!when_detect_doors || (borg_t - when_detect_doors >= 731)) do_door = TRUE; */

    /* Start */
    do_wall = FALSE;

    /* Determine if we need to detect walls */
    if (!borg_detect_wall[q_y+0][q_x+0]) do_wall = TRUE;
    if (!borg_detect_wall[q_y+0][q_x+1]) do_wall = TRUE;
    if (!borg_detect_wall[q_y+1][q_x+0]) do_wall = TRUE;
    if (!borg_detect_wall[q_y+1][q_x+1]) do_wall = TRUE;

    /* Hack -- check walls every few turns anyway */
    /* if (!when_detect_walls || (borg_t - when_detect_walls >= 937)) do_wall = TRUE; */


    /* Start */
    do_evil = FALSE;

    /* Determine if we need to detect evil */
    if (!borg_detect_evil[q_y+0][q_x+0]) do_evil = TRUE;
    if (!borg_detect_evil[q_y+0][q_x+1]) do_evil = TRUE;
    if (!borg_detect_evil[q_y+1][q_x+0]) do_evil = TRUE;
    if (!borg_detect_evil[q_y+1][q_x+1]) do_evil = TRUE;

    /* Hack -- check evil every few turns anyway- more fq if low level */
    /* if (!when_detect_evil ||
       (borg_t - when_detect_evil  >= 183 - (80 - borg_skill[BI_MAXCLEVEL]))) do_evil = TRUE; */

	/* Really low level */
    /* if (borg_skill[BI_CLEVEL] <= 3 &&
    	(!when_detect_evil ||
        (borg_t - when_detect_evil  >= 50))) do_evil = TRUE; */

	/* Not too frequent in town */
    /* if (borg_skill[BI_CDEPTH] == 0 &&
    	(!when_detect_evil ||
        (borg_t - when_detect_evil  >= 250))) do_evil = TRUE; */

    /* Dont bother if I have ESP */
    if (borg_skill[BI_ESP]) do_evil = FALSE;

    /* Only look for monsters in town, not walls, etc (scary guy)*/
    if (!borg_skill[BI_CDEPTH])
	{
		do_trap = FALSE;
		do_door = FALSE;
		do_wall = FALSE;
		do_LIGHT = FALSE;
	}

	/*** Do Things ***/

    /* Hack -- find traps and doors and evil*/
    if ((do_trap || do_door || do_evil) &&
        ((!when_detect_traps || (borg_t - when_detect_traps >= 5)) ||
         (!when_detect_evil || (borg_t - when_detect_evil >= 5)) ||
         (!when_detect_doors || (borg_t - when_detect_doors >= 5))) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {


        /* Check for traps and doors and evil*/
        if (borg_activate_artifact(EFF_DETECT_ALL, INVEN_WIELD) ||
            borg_zap_rod(SV_ROD_DETECTION) ||
            borg_prayer_fail(5, 1, 40))
        {
            borg_note("# Checking for traps, doors, and evil.");

            borg_react("SELF:TDE", "SELF:TDE");

            when_detect_traps = borg_t;
            when_detect_doors = borg_t;
            when_detect_evil =  borg_t;

            return (TRUE);
        }
    }

    /* Hack -- find evil */
    if (do_evil &&
        (!when_detect_evil || (borg_t - when_detect_evil >= 20)))
    {
        /* Check for evil */
        if (borg_prayer_fail(0, 0, 40) ||
            borg_spell_fail(0, 1, 40))
        {
            borg_note("# Checking for monsters.");

            borg_react("SELF:evil", "SELF:evil");

            when_detect_evil = borg_t;

            return (TRUE);
        }
    }

    /* Hack -- find traps and doors (and stairs) */
    if ((do_trap || do_door) &&
        ((!when_detect_traps || (borg_t - when_detect_traps >= 5)) ||
         (!when_detect_doors || (borg_t - when_detect_doors >= 5))) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for traps and doors */
        if (borg_activate_artifact(EFF_DETECT_ALL, INVEN_WIELD) ||
            borg_spell_fail(0, 6, 40) ||
            borg_prayer_fail(0, 5, 40))
        {
            borg_note("# Checking for traps, doors & stairs.");

            borg_react("SELF:both", "SELF:both");

            when_detect_traps = borg_t;
            when_detect_doors = borg_t;

            return (TRUE);
        }
    }


    /* Hack -- find traps */
    if (do_trap &&
        (!when_detect_traps || (borg_t - when_detect_traps >= 7)) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for traps */
        if (borg_activate_artifact(EFF_DETECT_TRAP, INVEN_WIELD) ||
        	borg_read_scroll(SV_SCROLL_DETECT_TRAP) ||
            borg_zap_rod(SV_ROD_DETECT_TRAP) ||
            borg_prayer_fail(0, 5, 40))
        {
            borg_note("# Checking for traps.");

            borg_react("SELF:trap", "SELF:trap");

            when_detect_traps = borg_t;

            return (TRUE);
        }
    }


    /* Hack -- find doors */
    if (do_door &&
        (!when_detect_doors || (borg_t - when_detect_doors >= 9)) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for traps */
        if (borg_activate_artifact(EFF_DETECT_ALL, INVEN_WIELD) ||
        	borg_read_scroll(SV_SCROLL_DETECT_DOOR) ||
            borg_zap_rod(SV_ROD_DETECT_DOOR) ||
            borg_prayer_fail(0, 5, 40))
        {
            borg_note("# Checking for doors.");

            borg_react("SELF:door", "SELF:door");

            when_detect_doors = borg_t;

            return (TRUE);
        }
    }


    /* Hack -- find walls */
    if (do_wall &&
        (!when_detect_walls || (borg_t - when_detect_walls >= 15)) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for walls */
        if (borg_activate_artifact(EFF_MAPPING, INVEN_LIGHT) ||
            borg_read_scroll(SV_SCROLL_MAPPING) ||
            borg_use_staff(SV_STAFF_MAPPING) ||
            borg_zap_rod(SV_ROD_MAPPING) ||
            borg_prayer(2, 6))
        {
            borg_note("# Checking for walls.");

            borg_react("SELF:wall", "SELF:wall");

            when_detect_walls = borg_t;

            return (TRUE);
        }
    }

    /* Never in town for the rest of this check */
    if (!borg_skill[BI_CDEPTH]) return (FALSE);

    /* Start */
    do_LIGHT = FALSE;

    /* Get central grid */
    ag = &borg_grids[c_y][c_x];

    corners = 0;
    floors = 0;

    /* Scan diagonal neighbors */
    for (i = 4; i < 8; i++)
    {
        /* Get location */
        x = c_x + ddx_ddd[i];
        y = c_y + ddy_ddd[i];

		/* Bounds check */
        if (!in_bounds_fully(y,x)) continue;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Location must be known */
        if (ag->feat == FEAT_NONE) corners ++;

        /* Location must not be a wall/door */
        if (!borg_cave_floor_grid(ag)) corners ++;

     }
     /* Add them up */
     if (corners <= 2) do_LIGHT = TRUE;

    /* Hack are we in a dark room? */
    if (do_LIGHT && (borg_skill[BI_CURLITE] >= 2) &&
        (c_x >= borg_skill[BI_CURLITE]) && (c_x < AUTO_MAX_X - borg_skill[BI_CURLITE]) &&
        (c_y >= borg_skill[BI_CURLITE]) && (c_y < AUTO_MAX_Y - borg_skill[BI_CURLITE]) &&
        (randint0(100) < 90))
    {
		/*       #.#
		 *       #.#
		 *  ######@######
		 *  #...........#
		 *  #...........#
		 *  #...........#
		 *  #...........#
		 *  #...........#
		 *  #############
		 */

		/* Lantern */
		if (borg_skill[BI_CURLITE] ==2) floor_goal = 11;

		/* lantern and glowing items */
		if (borg_skill[BI_CURLITE] == 3) floor_goal = 11;

        floors = 0;
        /* Scan the "local" grids (5x5) 2 same as lantern/torch grid*/
        for (y = c_y - 2; y <= c_y + 2; y++)
        {
            /* Scan the "local" grids (5x5) */
            for (x = c_x - 2; x <= c_x + 2; x++)
            {

				/* Bounds check */
            	if (!in_bounds_fully(y,x)) continue;

            	/* Get grid */
                ag = &borg_grids[y][x];
                feat = cave->feat[y][x]; /* Cheat this grid from game */

                /* Location must be a lit floor */
                if (ag->info & BORG_LIGHT) floors ++;

                /* Location must not be glowing */
                if (ag->info & BORG_GLOW ||
                    feat == CAVE_GLOW) floors --;

                /* Location must not be a wall/door */
                if (!borg_cave_floor_grid(ag)) floors --;

            }
        }
    }
    /* add them up */
    if (floors <= 11) do_LIGHT = do_LIGHT_aux = FALSE;


    /* Hack -- call lite */
    if (do_LIGHT &&
        (!when_call_LIGHT || (borg_t - when_call_LIGHT >= 7)))
    {
        /* Call light */
        if (borg_activate_artifact(EFF_ILLUMINATION, INVEN_LIGHT) ||
            borg_zap_rod(SV_ROD_ILLUMINATION) ||
            borg_use_staff(SV_STAFF_LIGHT) ||
            borg_read_scroll(SV_SCROLL_LIGHT) ||
            borg_spell(0, 3) ||
            borg_prayer(0, 4))
        {
            borg_note("# Illuminating the room");
            borg_react("SELF:lite", "SELF:lite");

            when_call_LIGHT = borg_t;

            return (TRUE);
        }
    }


    /* Hack -- Wizard Lite */
    if (TRUE &&
        (!when_wizard_LIGHT || (borg_t - when_wizard_LIGHT >= 1000)))
    {
        /* Wizard lite */
        if (borg_activate_artifact(EFF_CLAIRVOYANCE, INVEN_LIGHT) ||
            borg_prayer(5, 4))
        {
            borg_note("# Illuminating the dungeon");

            /* borg_react("SELF:wizard lite", "SELF:wizard lite"); */

            when_wizard_LIGHT = borg_t;

            return (TRUE);
        }
    }


    /* Oops */
    return (FALSE);
}
bool borg_check_LIGHT_only(void)
{
    int i, x, y;
    int corners, floors;

    int q_x, q_y;

    borg_grid *ag;


    bool do_LIGHT;

    bool do_LIGHT_aux = FALSE;


    /* Never in town */
    if (!borg_skill[BI_CDEPTH]) return (FALSE);

    /* Never when blind or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISIMAGE]) return (FALSE);

    /* XXX XXX XXX Dark */


    /* Extract the panel */
    q_x = w_x / 33;
    q_y = w_y / 11;

    /* Start */
    do_LIGHT = FALSE;

    /* Get central grid */
    ag = &borg_grids[c_y][c_x];

    corners = 0;
    floors = 0;

    /* Scan diagonal neighbors */
    for (i = 4; i < 8; i++)
    {
        /* Get location */
        x = c_x + ddx_ddd[i];
        y = c_y + ddy_ddd[i];

		/* Bounds check */
        if (!in_bounds_fully(y,x)) continue;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Location must be known */
        if (ag->feat == FEAT_NONE) corners ++;

        /* Location must not be a wall/door */
        if (!borg_cave_floor_grid(ag)) corners ++;

     }
     /* Add them up ..2*/
     if (corners <= 2) do_LIGHT = TRUE;

    /* Hack */
    if (do_LIGHT && (borg_skill[BI_CURLITE] >= 2) &&
        (c_x >= borg_skill[BI_CURLITE]) && (c_x < AUTO_MAX_X - borg_skill[BI_CURLITE]) &&
        (c_y >= borg_skill[BI_CURLITE]) && (c_y < AUTO_MAX_Y - borg_skill[BI_CURLITE]) &&
        (randint0(100) < 90))
    {

        floors = 0;
        /* Scan the "local" grids (5x5) 2 same as torch grid*/
        for (y = c_y - 2; y <= c_y + 2; y++)
        {
            /* Scan the "local" grids (5x5) */
            for (x = c_x - 2; x <= c_x + 2; x++)
            {
				/* Bounds check */
	            if (!in_bounds_fully(y,x)) continue;

                /* Get grid */
                ag = &borg_grids[y][x];

                /* Location must be a lit floor */
                if (ag->info & BORG_LIGHT) floors ++;

                /* Location must not be glowing */
                if (ag->info & BORG_GLOW) floors --;

                /* Location must not be a wall/door */
                if (!borg_cave_floor_grid(ag)) floors --;

            }
        }
    }
    /* add them up */
    if (floors <= 11) do_LIGHT = do_LIGHT_aux = FALSE;

    /* Hack -- call lite */
    if (do_LIGHT &&
        (!when_call_LIGHT || (borg_t - when_call_LIGHT >= 7)))
    {
        /* Call light */
        if (borg_activate_artifact(EFF_ILLUMINATION, INVEN_LIGHT) ||
            borg_zap_rod(SV_ROD_ILLUMINATION) ||
            borg_use_staff(SV_STAFF_LIGHT) ||
            borg_read_scroll(SV_SCROLL_LIGHT) ||
            borg_spell_fail(0, 3, 40) ||
            borg_prayer_fail(0, 4, 40))
        {
            borg_note("# Illuminating the room prior to resting");

            borg_react("SELF:lite", "SELF:lite");

            when_call_LIGHT = borg_t;

            /* dont rest. call light instead */
            return (TRUE);
        }
    }


    /* Hack -- Wizard Lite */
    if (TRUE &&
        (!when_wizard_LIGHT || (borg_t - when_wizard_LIGHT >= 1000)))
    {
        /* Wizard lite */
        if (borg_activate_artifact(EFF_CLAIRVOYANCE, INVEN_LIGHT) ||
            borg_prayer_fail(5, 4, 40))
        {
            borg_note("# Illuminating the dungeon prior to resting");

            /* borg_react("SELF:wizard lite", "SELF:wizard lite"); */

            when_wizard_LIGHT = borg_t;

            return (TRUE);
        }
    }


    /* nothing to light up.  OK to rest. */
    return (FALSE);
}



/*
 * Enchant armor, not including my swap armour
 */
static bool borg_enchant_to_a(void)
{
    int i, b_i = -1;
    int a, b_a = 99;

    /* Nothing to enchant */
    if (!my_need_enchant_to_a) return (FALSE);

    /* Need "enchantment" ability */
    if ((!amt_enchant_to_a) &&
        (!amt_enchant_armor)) return (FALSE);


    /* Look for armor that needs enchanting */
    for (i = INVEN_BODY; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->ident) continue;

        /* Obtain the bonus */
        a = item->to_a;

        /* Skip "boring" items */
        if (borg_spell_okay_fail(7, 2, 65) ||
            borg_prayer_okay_fail(7, 4, 65) ||
           amt_enchant_armor >=1)
        {
            if (a >= borg_enchant_limit) continue;
        }
        else
        {
            if (a >= 8) continue;
        }

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a)) continue;

        /* Save the info */
        b_i = i; b_a = a;

    }

    /* Nothing */
    if (b_i < 0) return (FALSE);

    /* Enchant it */
    if (borg_read_scroll(SV_SCROLL_STAR_ENCHANT_ARMOR) ||
        borg_read_scroll(SV_SCROLL_ENCHANT_ARMOR) ||
        borg_spell_fail(7, 2, 65) ||
        borg_prayer_fail(7, 4, 65))
    {
        /* Choose from equipment */
        if (b_i >= INVEN_WIELD)
        {
            borg_keypress('/');

            /* Choose that item */
            borg_keypress(I2A(b_i - INVEN_WIELD));
        }

        /* Success */
        return (TRUE);
    }

    /* Nothing to do */
    return (FALSE);
}


/*
 * Enchant weapons to hit
 */
static bool borg_enchant_to_h(void)
{
    int i, b_i = -1;
    int a, s_a, b_a = 99;


    /* Nothing to enchant */
    if (!my_need_enchant_to_h &&
        !enchant_weapon_swap_to_h) return (FALSE);

    /* Need "enchantment" ability */
    if ( (!amt_enchant_to_h) &&
         (!amt_enchant_weapon) ) return (FALSE);


    /* Look for a weapon that needs enchanting */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->ident) continue;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING) continue;

        /* Obtain the bonus */
        a = item->to_h;

        /* Skip "boring" items */
        if (borg_prayer_okay_fail(7, 3, 65) ||
            borg_spell_okay_fail(7, 3, 65) ||
            amt_enchant_weapon >= 1 )
        {
            if (a >= borg_enchant_limit) continue;
        }
        else
        {
            if (a >= 8) continue;
        }

		/* Most classes store the enchants until they get
		 * a 3x shooter (like a long bow).
		 * Generally, we do not want the x2 shooter enchanted,
		 * since it wastes scrolls.  But if the sword is at +5
		 * and the sling at +2, then the sling will be selected
		 * because its enchantment is lower.  The borg tries to
		 * enchant the least enchanted item.  This will make sure
		 * the x2 shooter is skipped and the sword is enchanted,
		 * if needed.  If the sword is at +9,+9, and the sling at
		 * +0,+0 and the borg has some enchant scrolls, he should
		 * store them instead of wasting them on the sling.
		 */
		if (i == INVEN_BOW &&  /* bow */
			my_ammo_power < 3 && /* 3x shooter */
			(!item->name1 && !item->name2)) /* Not artifact or ego */
			continue;

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a)) continue;

        /* Save the info */
        b_i = i; b_a = a;

    }
    if (weapon_swap > 1)
    {
    for (i=weapon_swap; i <= weapon_swap; i++)
    {
        borg_item *item = &borg_items[weapon_swap];

        /* Obtain the bonus */
        s_a = item->to_h;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING) continue;

		/* Skip "boring" items */
        if (borg_prayer_okay_fail(7, 3, 65) ||
            borg_spell_okay_fail(7, 3, 65) ||
            amt_enchant_weapon >= 1 )
        {
            if (s_a >= borg_enchant_limit) continue;
        }
        else
        {
            if (s_a >= 8) continue;
        }

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < s_a)) continue;

        /* Save the info */
        b_i = weapon_swap; b_a = s_a;
    }
    }
    /* Nothing, check ammo */
    if (b_i < 0)
    {
        /* look through inventory for ammo */
        for (i = 0; i < QUIVER_END; i++)
        {
            borg_item *item = &borg_items[i];

			/* Skip regular inventory */
			if (i >= INVEN_MAX_PACK && i < QUIVER_START) continue;

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (my_ammo_power < 3 || (!borg_items[INVEN_BOW].name1 && !borg_items[INVEN_BOW].name2)) continue;

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5) continue;

            /* Skip non-identified items  */
            if (!item->ident) continue;

            /* Make sure it is the right type if missile */
            if (item->tval != my_ammo_tval) continue;

            /* Obtain the bonus  */
            a = item->to_h;

            /* Skip items that are already enchanted */
            if (borg_prayer_okay_fail(7, 3, 65) ||
                borg_spell_okay_fail(7, 3, 65) ||
                amt_enchant_weapon >= 1 )
            {
                if (a >= 10) continue;
            }
            else
            {
                if (a >= 8) continue;
            }

            /* Find the least enchanted item */
            if ((b_i >= 0) && (b_a < a)) continue;

            /* Save the info  */
            b_i = i; b_a = a;

        }
    }

    /* Nothing */
    if (b_i < 0) return (FALSE);

    /* Enchant it */
    if (borg_read_scroll(SV_SCROLL_STAR_ENCHANT_ARMOR) ||
        borg_read_scroll(SV_SCROLL_ENCHANT_ARMOR) ||
        borg_prayer_fail(7, 3, 65) ||
        borg_spell_fail(7, 3, 65))
    {
        /* Choose from equipment */
        if (b_i >= INVEN_WIELD)
        {
            borg_keypress('/');

            /* Choose that item */
            borg_keypress(I2A(b_i - INVEN_WIELD));
        }
        else  /* choose the swap or ammo */
        {
            borg_keypress(I2A(b_i));
        }

        /* Success */
        return (TRUE);
    }

    /* Nothing to do */
    return (FALSE);
}


/*
 * Enchant weapons to dam
 */
static bool borg_enchant_to_d(void)
{
    int i, b_i = -1;
    int a, s_a, b_a = 99;


    /* Nothing to enchant */
    if (!my_need_enchant_to_d &&
        !enchant_weapon_swap_to_d) return (FALSE);

    /* Need "enchantment" ability */
    if ( (!amt_enchant_to_d) &&
         (!amt_enchant_weapon) ) return (FALSE);


    /* Look for a weapon that needs enchanting */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->ident) continue;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING) continue;

        /* Obtain the bonus */
        a = item->to_d;

        /* Skip "boring" items */
        if (borg_prayer_okay_fail(7, 3, 65) ||
            borg_spell_okay_fail(7, 3, 65) ||
            amt_enchant_weapon >= 1 )
        {
            if (a >= borg_enchant_limit) continue;
        }
        else
        {
            if (a >= 8) continue;
        }

		/* Most classes store the enchants until they get
		 * a 3x shooter (like a long bow).
		 * Generally, we do not want the x2 shooter enchanted,
		 * since it wastes scrolls.  But if the sword is at +5
		 * and the sling at +2, then the sling will be selected
		 * because its enchantment is lower.  The borg tries to
		 * enchant the least enchanted item.  This will make sure
		 * the x2 shooter is skipped and the sword is enchanted,
		 * if needed.  If the sword is at +9,+9, and the sling at
		 * +0,+0 and the borg has some enchant scrolls, he should
		 * store them instead of wasting them on the sling.
		 */
		if (i == INVEN_BOW &&  /* bow */
			my_ammo_power < 3 && /* 3x shooter */
			(!item->name1 && !item->name2)) /* Not artifact or ego */
			continue;

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a)) continue;

        /* Save the info */
        b_i = i; b_a = a;
    }
    if (weapon_swap > 1)
    {
    for (i = weapon_swap; i <= weapon_swap; i++)
    {
        borg_item *item = &borg_items[weapon_swap];

        /* Skip non-identified items */
        if (!item->ident) continue;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING) continue;

		/* Obtain the bonus */
        s_a = item->to_d;

        /* Skip "boring" items */
        if (borg_prayer_okay_fail(7, 3, 65) ||
            borg_spell_okay_fail(7, 3, 65) ||
            amt_enchant_weapon >= 1 )
        {
            if (s_a >= borg_enchant_limit) continue;
        }
        else
        {
            if (s_a >= 8) continue;
        }

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < s_a)) continue;

        /* Save the info */
        b_i = weapon_swap; b_a = s_a;
    }
    }
    /* Nothing, check ammo */
    if (b_i < 0)
    {
        /* look through inventory for ammo */
        for (i = 0; i < QUIVER_END; i++)
        {
            borg_item *item = &borg_items[i];

			/* Skip regular inventory */
			if (i >= INVEN_MAX_PACK && i < QUIVER_START) continue;

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (my_ammo_power < 3 || (!borg_items[INVEN_BOW].name1 && !borg_items[INVEN_BOW].name2)) continue;

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (my_ammo_power < 3) continue;

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5) continue;

            /* Skip non-identified items  */
            if (!item->ident) continue;

            /* Make sure it is the right type if missile */
            if (item->tval != my_ammo_tval) continue;

            /* Obtain the bonus  */
            a = item->to_d;

            /* Skip items that are already enchanted */
            if (borg_prayer_okay_fail(7, 3, 65) ||
                borg_spell_okay_fail(7, 3, 65) ||
                amt_enchant_weapon >= 1 )
            {
                if (a >= 10) continue;
            }
            else
            {
                if (a >= 8) continue;
            }

            /* Find the least enchanted item */
            if ((b_i >= 0) && (b_a < a)) continue;

            /* Save the info  */
            b_i = i; b_a = a;

        }
    }

    /* Nothing */
    if (b_i < 0) return (FALSE);

    /* Enchant it */
    if (borg_read_scroll(SV_SCROLL_STAR_ENCHANT_WEAPON) ||
        borg_read_scroll(SV_SCROLL_ENCHANT_WEAPON_TO_DAM) ||
        borg_prayer_fail(7, 3, 65) ||
        borg_spell_fail(7, 3, 65))
    {
        /* Choose from equipment */
        if (b_i >= INVEN_WIELD)
        {
            borg_keypress('/');

            /* Choose that item */
            borg_keypress(I2A(b_i - INVEN_WIELD));
        }
        else  /* choose the swap or ammo */
        {
            borg_keypress(I2A(b_i));
        }

        /* Success */
        return (TRUE);
    }

    /* Nothing to do */
    return (FALSE);
}

/*
 * Brand Bolts
 */
static bool borg_brand_weapon(void)
{
    int i, b_i = -1;
    int a, b_a = 0;

    /* Nothing to brand */
    if (!my_need_brand_weapon) return (FALSE);

    /* Need "brand" ability */
    if (!amt_brand_weapon) return (FALSE);

    /* look through inventory for ammo */
        for (i = QUIVER_START; i < QUIVER_END; i++)
        {
            borg_item *item = &borg_items[i];

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5) continue;

            /* Skip non-identified items  */
            if (!item->ident) continue;

            /* Make sure it is the right type if missile */
            if (item->tval != my_ammo_tval) continue;

            /* Obtain the bonus  */
            a = item->to_h;

            /* Skip branded items */
            if (item->name2) continue;

            /* Find the most enchanted item */
            if ((b_i >= 0) && (b_a > a)) continue;

            /* Save the info  */
            b_i = i; b_a = a;

    }

	/* Nothing to Brand */
	if (b_i == -1) return (FALSE);

    /* Enchant it */
    if (borg_activate_artifact(EFF_FIREBRAND, INVEN_BOW) ||
        borg_spell_fail(7, 5, 65))
    {

        /* 291 would like a location of the brand */
        /* choose the or ammo */
        {
            borg_keypress('/');
            borg_keypress(I2A(b_i - INVEN_WIELD));;
        }

        /* Success */
        return (TRUE);
    }

    /* Nothing to do */
    return (FALSE);
}

/*
 * Remove Curse
 */
static bool borg_decurse_armour(void)
{
    /* Nothing to decurse */
    if ((borg_uses_swaps && decurse_armour_swap == -1) && !borg_wearing_cursed) return (FALSE);

    /* Ability for heavy curse */
    if (borg_uses_swaps && decurse_armour_swap == 1)
    {
        if (-1 == borg_slot(TV_SCROLL,SV_SCROLL_STAR_REMOVE_CURSE) &&
            !borg_prayer_okay_fail(7,2,40))
        {
                return (FALSE);
        }

        else if (borg_uses_swaps && decurse_armour_swap ==1)
        {

            /* First wear the item */
			borg_note("# Decursing armor.");
            borg_keypress('w');
            borg_keypress(I2A(armour_swap));

            /* ooops it feels deathly cold */
            borg_keypress(' ');
        }

        /* remove the curse */
        if (borg_read_scroll(SV_SCROLL_STAR_REMOVE_CURSE) ||
            borg_prayer(7,2) )
        {
            /* Shekockazol! */
            borg_wearing_cursed = FALSE;
            return (TRUE);
        }

    }

    /* Ability for light curse */
    if ((borg_uses_swaps && decurse_armour_swap == 0) || borg_wearing_cursed)
    {
        if (-1 == borg_slot(TV_SCROLL,SV_SCROLL_REMOVE_CURSE) &&
           (-1 == borg_slot(TV_STAFF,SV_STAFF_REMOVE_CURSE) &&
            -1 == borg_items[borg_slot(TV_STAFF, SV_STAFF_REMOVE_CURSE)].pval) &&
           !borg_prayer_okay_fail(1,6,40) )
            {
                return (FALSE);
            }

        if (borg_wearing_cursed)
        {
            /* no need to wear it first */
        }
        else
        {
            /* First wear the item */
			borg_note("# Decursing armor.");
            borg_keypress('w');
            borg_keypress(I2A(armour_swap));

            /* ooops it feels deathly cold */
            borg_keypress(' ');
        }
        /* remove the curse */
        if (borg_read_scroll(SV_SCROLL_REMOVE_CURSE) ||
            borg_use_staff(SV_STAFF_REMOVE_CURSE)||
            borg_prayer(1,6) )
        {
            /* Shekockazol! */
            borg_wearing_cursed = FALSE;
            return (TRUE);
        }
    }

    /* Nothing to do */
    return (FALSE);
}
/*
 * Remove Curse
 *
 */
static bool borg_decurse_weapon(void)
{
    /* Nothing to decurse */
    if (borg_uses_swaps && decurse_weapon_swap == -1) return (FALSE);

    /* Ability for heavy curse */
    if (borg_uses_swaps && decurse_weapon_swap == 1)
    {
        if (-1 == borg_slot(TV_SCROLL,SV_SCROLL_STAR_REMOVE_CURSE) &&
            !borg_prayer_okay_fail(7,2,40))
            {
                return (FALSE);
            }

        /* First wear the item */
		borg_note("# Decursing weapon.");
		borg_keypress('w');
        borg_keypress(I2A(weapon_swap));

        /* ooops it feels deathly cold */
        borg_keypress(' ');

        /* remove the curse */
        if (borg_read_scroll(SV_SCROLL_STAR_REMOVE_CURSE) ||
            borg_prayer(7,2) )
            {
                /* Shekockazol! */
                return (TRUE);
            }
    }

    /* Ability for light curse */
    if (borg_uses_swaps && decurse_weapon_swap == 0)
    {
        if (-1 == borg_slot(TV_SCROLL,SV_SCROLL_REMOVE_CURSE) &&
           (-1 == borg_slot(TV_STAFF,SV_STAFF_REMOVE_CURSE) &&
            -1 == borg_items[borg_slot(TV_STAFF, SV_STAFF_REMOVE_CURSE)].pval) &&
           !borg_prayer_okay_fail(1,6,40) )
        {
            return (FALSE);
        }

		/* First wear the item */
		borg_note("# Decursing weapon.");
		borg_keypress('w');
		borg_keypress(I2A(weapon_swap));

		/* ooops it feels deathly cold */
		borg_keypress(' ');

		/* remove the curse */
		if (borg_read_scroll(SV_SCROLL_REMOVE_CURSE) ||
			borg_use_staff(SV_STAFF_REMOVE_CURSE)||
			borg_prayer(1,6) )
		{
			/* Shekockazol! */
			return (TRUE);
		}
    }

    /* Nothing to do */
    return (FALSE);
}

/*
 * Enchant things
 */
bool borg_enchanting(void)
{
    /* Forbid blind/confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (FALSE);


    /* Forbid if been sitting on level forever */
    /*    Just come back and finish the job later */
    if ((borg_t - borg_began > 5500 && borg_skill[BI_CDEPTH] >= 1) ||
        (borg_t-borg_began > 3501 && borg_skill[BI_CDEPTH] == 0)) return (FALSE);

    /* Remove Curses */
    if (borg_decurse_armour()) return (TRUE);
    if (borg_decurse_weapon()) return (TRUE);

	/* Only in town */
	if (borg_skill[BI_CDEPTH]) return (FALSE);

    /* Enchant things */
    if (borg_brand_weapon()) return (TRUE);
    if (borg_enchant_to_h()) return (TRUE);
    if (borg_enchant_to_d()) return (TRUE);
    if (borg_enchant_to_a()) return (TRUE);

    /* Nope */
    return (FALSE);
}


/*
 * Recharge things
 *
 * XXX XXX XXX Prioritize available items
 */
bool borg_recharging(void)
{
    int i = -1;
    bool charge = FALSE;


    /* Forbid blind/confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (FALSE);

    /* XXX XXX XXX Dark */

    /* Look for an item to recharge */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->ident && !(strstr(item->note, "empty"))) continue;

        /* assume we can't charge it. */
        charge = FALSE;

        /* Wands with no charges can be charged */
        if ((item->tval == TV_WAND) && (item->pval <= 1))
            charge = TRUE;

        /* recharge staves sometimes */
        if (item->tval == TV_STAFF)
        {
            /* Allow staves to be recharged at 2 charges if
             * the borg has the big recharge spell. And its not a *Dest*
             */
            if ((item->pval < 2) &&
                (borg_spell_okay_fail(7, 4, 96) || borg_spell_okay_fail(2, 1, 96)) &&
                item->sval < SV_STAFF_POWER)
                charge = TRUE;

            /* recharge any staff at 0 charges */
            if (item->pval <= 1)
                charge = TRUE;

            /* Staves of teleport get recharged at 2 charges in town */
            if ((item->sval == SV_STAFF_TELEPORTATION) &&
                (item->pval < 3) &&
                !borg_skill[BI_CDEPTH])
                charge = TRUE;

            /* They stack.  If quantity is 4 and pval is 1, then there are 4 charges. */
            if ((item->iqty + item->pval >= 4) && item->pval >= 1) charge = FALSE;
        }

        /* get the next item if we are not to charge this one */
        if (!charge) continue;

        /* Attempt to recharge */
        if (borg_spell_fail(7, 4, 96) ||
            borg_read_scroll(SV_SCROLL_RECHARGING) ||
            borg_spell_fail(2, 1, 96) ||
            borg_prayer_fail(7, 1, 96) ||
            borg_activate_artifact(EFF_RECHARGE, INVEN_OUTER))
        {
            /* Message */
            borg_note(format("Recharging %s with current charge of %d", item->desc, item->pval));

            /* Recharge the item */
            borg_keypress(I2A(i));

			/* Remove the {empty} if present */
			if (strstr(item->note, "empty")) borg_send_deinscribe(i);

			/* Success */
            return (TRUE);
        }
        else
            /* if we fail once, no need to try again. */
            break;
    }

    /* Nope */
    return (FALSE);
}


/*
 * Attempt to consume an item as a method of destroying it.
 */
static bool borg_consume(int i)
{
    borg_item *item = &borg_items[i];


    /* Special destruction */
    switch (item->tval)
    {
        case TV_POTION:

        /* Check the potion */
        switch (item->sval)
        {
            case SV_POTION_SLIME_MOLD:
            case SV_POTION_CURE_LIGHT:
            case SV_POTION_CURE_SERIOUS:
            case SV_POTION_CURE_CRITICAL:
            case SV_POTION_HEALING:
            case SV_POTION_STAR_HEALING:
            case SV_POTION_LIFE:
            case SV_POTION_RES_STR:
            case SV_POTION_RES_INT:
            case SV_POTION_RES_WIS:
            case SV_POTION_RES_DEX:
            case SV_POTION_RES_CON:
            case SV_POTION_RES_CHR:
            case SV_POTION_RESTORE_EXP:
            case SV_POTION_RESTORE_MANA:
            case SV_POTION_HEROISM:
            case SV_POTION_BERSERK_STRENGTH:
            case SV_POTION_RESIST_HEAT:
            case SV_POTION_RESIST_COLD:
            case SV_POTION_INFRAVISION:
            case SV_POTION_DETECT_INVIS:
            case SV_POTION_CURE_POISON:
            case SV_POTION_SPEED:
			case SV_POTION_INC_EXP:
				{
			        /* Try quaffing the potion */
				    if (borg_quaff_potion(item->sval)) return (TRUE);
				}
				break;

			/* Gain one/lose one potions */
			case SV_POTION_INC_STR2:
				{
					/* Maxed out no need. Don't lose another stat */
					if (borg_skill[BI_CSTR] >= 118) return (FALSE);

					/* This class does not want to risk losing a different stat */
					if (borg_class == CLASS_MAGE) return (FALSE);

					/* Otherwise, it should be ok */
				    if (borg_quaff_potion(item->sval)) return (TRUE);
				}
				break;

			case SV_POTION_INC_INT2:
				{
					/* Maxed out no need. Don't lose another stat */
					if (borg_skill[BI_CINT] >= 118) return (FALSE);

					/* This class does not want to risk losing a different stat */
					if (borg_class == CLASS_WARRIOR || borg_class == CLASS_PRIEST ||
						borg_class == CLASS_PALADIN || borg_class == CLASS_ROGUE) return (FALSE);

					/* Otherwise, it should be ok */
				    if (borg_quaff_potion(item->sval)) return (TRUE);
				}
				break;

			case SV_POTION_INC_WIS2:
				{
					/* Maxed out no need. Don't lose another stat */
					if (borg_skill[BI_CWIS] >= 118) return (FALSE);

					/* This class does not want to risk losing a different stat */
					if (borg_class == CLASS_WARRIOR || borg_class == CLASS_MAGE || borg_class == CLASS_RANGER ||
						borg_class == CLASS_ROGUE) return (FALSE);

					/* Otherwise, it should be ok */
				    if (borg_quaff_potion(item->sval)) return (TRUE);
				}
				break;

			case SV_POTION_INC_DEX2:
				{
					/* Maxed out no need. Don't lose another stat */
					if (borg_skill[BI_CDEX] >= 118) return (FALSE);

					/* This class does not want to risk losing a different stat */
					if (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST) return (FALSE);

					/* Otherwise, it should be ok */
				    if (borg_quaff_potion(item->sval)) return (TRUE);
				}
				break;

			case SV_POTION_INC_CON2:
				{
					/* Maxed out no need. Don't lose another stat */
					if (borg_skill[BI_CCON] >= 118) return (FALSE);

					/* Otherwise, it should be ok */
				    if (borg_quaff_potion(item->sval)) return (TRUE);
				}
				break;

        }

        break;

        case TV_SCROLL:

        /* Check the scroll */
        switch (item->sval)
        {
            case SV_SCROLL_REMOVE_CURSE:
            case SV_SCROLL_LIGHT:
            case SV_SCROLL_MONSTER_CONFUSION:
            case SV_SCROLL_STAR_REMOVE_CURSE:
            case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
            case SV_SCROLL_SATISFY_HUNGER:
            case SV_SCROLL_DISPEL_UNDEAD:
            case SV_SCROLL_BLESSING:
            case SV_SCROLL_HOLY_CHANT:
            case SV_SCROLL_HOLY_PRAYER:
			{
				/* Try reading the scroll */
				if (borg_read_scroll(item->sval)) return (TRUE);
				break;
			}
        }

        break;

        case TV_FOOD:

        /* Check the grub */
        switch (item->sval)
        {
			case SV_FOOD_SECOND_SIGHT:
            case SV_FOOD_FAST_RECOVERY:
            case SV_FOOD_CURE_MIND:
            case SV_FOOD_RESTORING:
            case SV_FOOD_EMERGENCY:
            case SV_FOOD_TERROR:
            case SV_FOOD_STONESKIN:
            case SV_FOOD_DEBILITY:
            case SV_FOOD_SPRINTING:
            case SV_FOOD_PURGING:
            case SV_FOOD_RATION:
            case SV_FOOD_SLIME_MOLD:
            case SV_FOOD_WAYBREAD:

            /* Try eating the food (unless Bloated) */
            if (!borg_skill[BI_ISFULL] && borg_eat_food(item->sval)) return (TRUE);
        }

        break;
    }


    /* Nope */
    return (FALSE);
}




/*
 * Destroy "junk" items
 */
bool borg_crush_junk(void)
{
    int i;
    bool fix = FALSE;
    s32b p;
    s32b value;
	int count;

	/* Hack -- no need */
    if (!borg_do_crush_junk) return (FALSE);

    /* No crush if even slightly dangerous */
    if (borg_danger(c_y,c_x,1, TRUE, FALSE) > borg_skill[BI_CURHP] / 10) return (FALSE);

    /* Destroy actual "junk" items */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* dont crush the swap weapon */
        if (i == weapon_swap && item->iqty == 1 && item->tval != TV_FOOD) continue;
        if (i == armour_swap && item->tval != TV_FOOD) continue;

        /* Dont crush weapons if we are weilding a digger */
#if 0
            if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;
#endif

        /* dont crush our spell books */
        if (item->tval == p_ptr->class->spell_book) continue;

		/* Do not crush unID'd Scrolls, sell them in town */
		if (item->tval == TV_SCROLL && (!item->ident && !item->kind)) continue;

		/* Do not crush Boots, they could be SPEED */
		if (item->tval == TV_BOOTS && !item->ident) continue;

		/* save the items value */
        value = item->value;

		/* Crush Stacked Wands and Staves that are empty.
		 * ie. 5 Staffs of Teleportation (2 charges).
		 * Only 2 charges in 5 staves means top 3 are empty.
		 */
		if ((item->tval == TV_STAFF || item->tval == TV_WAND) &&
			(item->ident || strstr(item->note, "empty")))
		{
			if (item->iqty > item->pval) value = 0L;
		}

        /* Skip non "worthless" items */
        if (item->tval >= TV_CHEST)
        {
            /* unknown and not worthless */
            if (!item->ident && !strstr(item->note, "average") &&
                 value > 0)
                continue;

            /* skip items that are 'valuable'.  This is level dependent */
            /* try to make the borg junk +1,+1 dagger at level 40 */

            /* if the item gives a bonus to a stat, boost its value */
            if (((of_has(item->flags, OF_STR)) ||
                (of_has(item->flags, OF_INT)) ||
                (of_has(item->flags, OF_WIS)) ||
                (of_has(item->flags, OF_STR)) ||
                (of_has(item->flags, OF_CON))) && value > 0)
            {
                value += 2000L;
            }

            /* Keep some stuff */
            if ( (item->tval == my_ammo_tval && value > 0) ||
            ((item->tval == TV_POTION && item->sval == SV_POTION_RESTORE_MANA) &&
             (borg_skill[BI_MAXSP] >= 1)) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_HEALING) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_STAR_HEALING) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_LIFE) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_SPEED) ||
            (item->tval == TV_ROD && item->sval == SV_ROD_DRAIN_LIFE)||
            (item->tval == TV_ROD && item->sval == SV_ROD_HEALING)  ||
            (item->tval == TV_ROD && item->sval == SV_ROD_MAPPING && borg_class == CLASS_WARRIOR) ||
            (item->tval == TV_STAFF && item->sval == SV_STAFF_DISPEL_EVIL) ||
            (item->tval == TV_STAFF && item->sval == SV_STAFF_POWER) ||
            (item->tval == TV_STAFF && item->sval == SV_STAFF_HOLINESS) ||
            (item->tval == TV_WAND && item->sval == SV_WAND_DRAIN_LIFE) ||
            (item->tval == TV_WAND && item->sval == SV_WAND_ANNIHILATION) ||
            (item->tval == TV_WAND && item->sval == SV_WAND_TELEPORT_AWAY && borg_class == CLASS_WARRIOR) ||
            (item->tval == TV_CLOAK && item->name2 == EGO_AMAN) ||
            (item->tval == TV_SCROLL && item->sval == SV_SCROLL_TELEPORT_LEVEL &&
             borg_skill[BI_ATELEPORTLVL] < 1000) ||
            (item->tval == TV_SCROLL && item->sval == SV_SCROLL_PROTECTION_FROM_EVIL))

            {
                value +=5000L;
            }

			/* Go Ahead and crush diggers */
			if (item->tval == TV_DIGGING) value = 0L;

	        /* Crush missiles that aren't mine */
	        if (item->tval == TV_SHOT ||
	            item->tval == TV_ARROW ||
	            item->tval == TV_BOLT)
	        {
	            if (item->tval != my_ammo_tval) value = 0L;
	        }

            /* borg_worships_gold will sell all kinds of stuff,
             * except {cursed} is junk
             */
            if (item->value > 0 &&
                ((borg_worships_gold || borg_skill[BI_MAXCLEVEL] < 10) ||
                 ((borg_money_scum_amount < borg_gold) && borg_money_scum_amount != 0)) &&
                borg_skill[BI_MAXCLEVEL] <= 20 &&
                !(strstr(item->note, "cursed}")) ) continue;


            /* up to level 5, keep anything of any value */
            if (borg_skill[BI_CDEPTH] < 5 && value > 0)
                continue;
			/* up to level 10, keep anything of any value */
            if (borg_skill[BI_CDEPTH] < 10 && value > 15)
                continue;
            /* up to level 15, keep anything of value 100 or better */
            if (borg_skill[BI_CDEPTH] < 15 && value > 100)
                continue;
            /* up to level 30, keep anything of value 500 or better */
            if (borg_skill[BI_CDEPTH] < 30 && value > 500)
                continue;
            /* up to level 40, keep anything of value 1000 or better */
            if (borg_skill[BI_CDEPTH] < 40 && value > 1000)
                continue;
            /* up to level 60, keep anything of value 1200 or better */
            if (borg_skill[BI_CDEPTH] < 60 && value > 1200)
                continue;
            /* up to level 80, keep anything of value 1400 or better */
            if (borg_skill[BI_CDEPTH] < 80 && value > 1400)
                continue;
            /* up to level 90, keep anything of value 1600 or better */
            if (borg_skill[BI_CDEPTH] < 90 && value > 1600)
                continue;
            /* up to level 95, keep anything of value 4800 or better */
            if (borg_skill[BI_CDEPTH] < 95 && value > 4800)
                continue;
            /* below level 127, keep anything of value 2000 or better */
            if (borg_skill[BI_CDEPTH] < 127 && value > 5600)
                continue;

            /* Save the item */
            COPY(&safe_items[i], &borg_items[i], borg_item);

            /* Destroy the item */
            WIPE(&borg_items[i], borg_item);

            /* Fix later */
            fix = TRUE;

            /* Examine the inventory */
            borg_notice(FALSE);

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            COPY(&borg_items[i], &safe_items[i], borg_item);

            /* skip things we are using */
            if (p < my_power) continue;
        }

        /* re-examine the inventory */
        if (fix) borg_notice(TRUE);

        /* Hack -- skip good un-id'd "artifacts" */
        if (strstr(item->note, "special")) continue;
        if (strstr(item->note, "terrible")) continue;
        if (strstr(item->note, "indestructible")) continue;

        /* hack --  with random artifacts some are good and bad */
        /*         so check them all */
        if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && item->name1 && !item->fully_identified) continue;

        /* Message */
        borg_note(format("# Junking junk (valued at %d)",value));
        /* Message */
        borg_note(format("# Destroying %s.", item->desc));

        if (item->tval == TV_STAFF || item->tval == TV_WAND)
        {
	        /* Destroy one item */
	        borg_keypresses("01");
			borg_keypress('\n');
		}
		else
		{
			/* Destroy all items */
        	borg_keypresses("099");
			borg_keypress('\n');
		}

        /* Destroy that item */
        if (!item->name1)
                borg_keypress('k');
        else
        {
            int a;

            /* worthless artifacts are dropped. */
            borg_keypress('d');

            /* mark the spot that the object was dropped so that  */
            /* it will not be picked up again. */
            count = bad_obj_cnt;
			for (a = 0; a <= count; a++)
            {
                if (bad_obj_x[a] != -1) continue;
                if (bad_obj_y[a] != -1) continue;

                bad_obj_x[a] = c_x;
                bad_obj_y[a] = c_y;
                borg_note(format("# Crappy artifact at %d,%d",bad_obj_x[a],bad_obj_y[a]));
				bad_obj_cnt ++;

                break;
            }
        }
        borg_keypress(I2A(i));

        /* Verify destruction */
        borg_keypress('y');

        /* Success */
        return (TRUE);
    }

    /* re-examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Hack -- no need */
    borg_do_crush_junk = FALSE;

    /* Nothing to destroy */
    return (FALSE);
}



/*
 * Destroy something to make a free inventory slot.
 *
 * This function evaluates the possible worlds that result from
 * the destruction of each inventory slot, and attempts to destroy
 * that slot which causes the best possible resulting world.
 *
 * We attempt to avoid destroying unknown items by "rewarding" the
 * presence of unknown items by a massively heuristic value.
 *
 * If the Borg cannot find something to destroy, which should only
 * happen if he fills up with artifacts, then he will probably act
 * rather twitchy for a while.
 *
 * This function does not have to be very efficient.
 */
bool borg_crush_hole(void)
{
    int i, b_i = -1;
    s32b p, b_p = 0L;

    s32b value;

    bool fix = FALSE;


    /* Do not destroy items unless we need the space */
    if (!borg_items[INVEN_MAX_PACK-1].iqty) return (FALSE);

    /* No crush if even slightly dangerous */
    if (borg_skill[BI_CDEPTH] && (borg_danger(c_y,c_x,1, TRUE, FALSE) > borg_skill[BI_CURHP] / 10 &&
         (borg_skill[BI_CURHP] != borg_skill[BI_MAXHP] ||
         borg_danger(c_y,c_x,1, TRUE, FALSE) > (borg_skill[BI_CURHP] * 2) / 3)))
        return (FALSE);

    /* Scan the inventory */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip "artifacts" */
        if (item->name1) continue;

		/* skip food */
		if (item->tval == TV_FOOD && borg_skill[BI_FOOD] < 5) continue;

        /* dont crush the swap weapon */
        if (i == weapon_swap && item->tval != TV_FOOD) continue;
        if (i == armour_swap && item->tval != TV_FOOD) continue;

        /* dont crush our spell books */
        if (item->tval == p_ptr->class->spell_book) continue;

		/* Do not crush Boots, they could be SPEED */
		if (item->tval == TV_BOOTS && !item->ident) continue;

        /* Dont crush weapons if we are weilding a digger */
        if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;

        /* Hack -- skip "artifacts" */
        if (item->name1 && !item->fully_identified) continue;
        if (strstr(item->note, "special")) continue;
        if (strstr(item->note, "terrible")) continue;
        if (strstr(item->note, "indestructible")) continue;

        /* never crush cool stuff that we might be needing later */
        if ((item->tval == TV_POTION && item->sval == SV_POTION_RESTORE_MANA) &&
            (borg_skill[BI_MAXSP] >= 1)) continue;
        if (item->tval == TV_POTION && item->sval == SV_POTION_HEALING) continue;
        if (item->tval == TV_POTION && item->sval == SV_POTION_STAR_HEALING) continue;
        if (item->tval == TV_POTION && item->sval == SV_POTION_LIFE) continue;
        if (item->tval == TV_POTION && item->sval == SV_POTION_SPEED) continue;
        if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_PROTECTION_FROM_EVIL) continue;
        if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_RUNE_OF_PROTECTION) continue;
        if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_TELEPORT_LEVEL &&
            borg_skill[BI_ATELEPORTLVL] < 1000 ) continue;
        if (item->tval == TV_CLOAK && item->name2 == EGO_AMAN) continue;
        if (item->tval == TV_ROD && (item->sval == SV_ROD_HEALING ||
            (item->sval == SV_ROD_MAPPING && borg_class == CLASS_WARRIOR)) &&
            item->iqty <= 5) continue;
        if (item->tval == TV_WAND && item->sval == SV_WAND_TELEPORT_AWAY &&
            borg_class == CLASS_WARRIOR && borg_skill[BI_ATPORTOTHER] <= 8) continue;
        if (item->tval == TV_ROD && (item->sval == SV_ROD_LIGHT &&
            borg_skill[BI_CURLITE] <= 0)) continue;

        /* save the items value */
        value = item->value;

        /* Save the item */
        COPY(&safe_items[i], &borg_items[i], borg_item);

        /* Destroy the item */
        WIPE(&borg_items[i], borg_item);

        /* Fix later */
        fix = TRUE;

        /* Examine the inventory */
        borg_notice(FALSE);

        /* Evaluate the inventory */
        p = borg_power();

        /* power is much more important than gold. */
        p *= 100;

		/* If I have no food, and in town, I must have a free spot to buy food */
		if (borg_skill[BI_CDEPTH] == 0 && borg_skill[BI_FOOD] ==0)
		{
			/* Power is way more important than gold */
			p *= 500;
		}
        /* Restore the item */
        COPY(&borg_items[i], &safe_items[i], borg_item);

        /* Penalize loss of "gold" */

        /* if the item gives a bonus to a stat, boost its value */
        if ((of_has(item->flags, OF_STR)) ||
            (of_has(item->flags, OF_INT)) ||
            (of_has(item->flags, OF_WIS)) ||
            (of_has(item->flags, OF_DEX)) ||
            (of_has(item->flags, OF_CON)))
        {
            value += 20000L;
        }

        /* Keep the correct types of missiles which have value
         * if we do have have tons already. unless in town, you can junk em in town.
         */
        if ((item->tval == my_ammo_tval) && (value > 0) &&
            (borg_skill[BI_AMISSILES] <= 35) &&
            borg_skill[BI_CDEPTH] >=1)
        {
            value +=5000L;
        }

        /* Hack  show prefrence for destroying things we will not use */
        /* if we are high enough level not to worry about gold. */
        if (borg_skill[BI_CLEVEL] > 35)
        {
            switch (item->tval)
            {
                /* rings are under valued. */
                case TV_RING:
                    p -= (item->iqty * value * 10);
                    break;

                case TV_AMULET:
                case TV_BOW:
                case TV_HAFTED:
                case TV_POLEARM:
                case TV_SWORD:
                case TV_BOOTS:
                case TV_GLOVES:
                case TV_HELM:
                case TV_CROWN:
                case TV_SHIELD:
                case TV_SOFT_ARMOR:
                case TV_HARD_ARMOR:
                case TV_DRAG_ARMOR:
                    p -= (item->iqty * value * 5);
                    break;
                case TV_CLOAK:
                    if (item->name2 != EGO_AMAN)
                    p -=(item->iqty *(300000L));
                    else
                    p -= (item->iqty * value);
                    break;

                case TV_ROD:
                    /* BIG HACK! don't crush cool stuff. */
                    if ((item->sval != SV_ROD_DRAIN_LIFE) ||
                        (item->sval != SV_ROD_ACID_BALL) ||
                        (item->sval != SV_ROD_ELEC_BALL) ||
                        (item->sval != SV_ROD_FIRE_BALL) ||
                        (item->sval != SV_ROD_COLD_BALL) )
                        p -= (item->iqty * (300000L));  /* value at 30k */
                    else
                    p -= (item->iqty * value);
                    break;

                case TV_STAFF:
                    /* BIG HACK! don't crush cool stuff. */
                    if (item->sval != SV_STAFF_DISPEL_EVIL ||
                        ((item->sval != SV_STAFF_POWER ||
                          item->sval != SV_STAFF_HOLINESS) &&
                          amt_cool_staff < 2) ||
                        (item->sval != SV_STAFF_DESTRUCTION &&
                         borg_skill[BI_ASTFDEST] < 2))
                        p -= (item->iqty * (300000L));  /* value at 30k */
                    else
                    p -= (item->iqty * (value/2));
                case TV_WAND:
                    /* BIG HACK! don't crush cool stuff. */
                    if ((item->sval != SV_WAND_DRAIN_LIFE) ||
                        (item->sval != SV_WAND_TELEPORT_AWAY) ||
                        (item->sval != SV_WAND_ACID_BALL) ||
                        (item->sval != SV_WAND_ELEC_BALL) ||
                        (item->sval != SV_WAND_FIRE_BALL) ||
                        (item->sval != SV_WAND_COLD_BALL) ||
                        (item->sval != SV_WAND_ANNIHILATION) ||
                        (item->sval != SV_WAND_DRAGON_FIRE) ||
                        (item->sval != SV_WAND_DRAGON_COLD) )
                        p -= (item->iqty * (300000L));  /* value at 30k */
                    else
                    p -= (item->iqty * (value/2));
                    break;

                /* scrolls and potions crush easy */
                case TV_SCROLL:
                    if ((item->sval != SV_SCROLL_PROTECTION_FROM_EVIL) ||
                        (item->sval != SV_SCROLL_RUNE_OF_PROTECTION))
                        p -=(item->iqty * (30000L));
                    else
                        p -= (item->iqty * (value/10));
                    break;

                case TV_POTION:
                    /* BIG HACK! don't crush heal/mana potions.  It could be */
                    /* that we are in town and are collecting them. */
                    if ((item->sval != SV_POTION_HEALING) ||
                        (item->sval != SV_POTION_STAR_HEALING) ||
                        (item->sval != SV_POTION_LIFE) ||
                        (item->sval != SV_POTION_RESTORE_MANA))
                        p -= (item->iqty * (300000L));  /* value at 30k */
                    else
                        p -= (item->iqty * (value/10));
                    break;

                default:
                    p -= (item->iqty * (value/3));
                    break;
            }
        }
        else
        {
            p -= (item->iqty * value);
        }

        /* Hack -- try not to destroy "unaware" items
		 * unless deep
		 */
        if (!item->kind && (value > 0))
        {
            /* Hack -- Reward "unaware" items */
            switch (item->tval)
            {
                case TV_RING:
                case TV_AMULET:
                p -= (borg_skill[BI_MAXDEPTH] * 5000L);
                break;

                case TV_ROD:
                p -= (borg_skill[BI_MAXDEPTH] * 3000L);
                break;

                case TV_STAFF:
                case TV_WAND:
                p -= (borg_skill[BI_MAXDEPTH] * 2000L);
                break;

                case TV_SCROLL:
                case TV_POTION:
                p -= (borg_skill[BI_MAXDEPTH] * 500L);
                break;

                case TV_FOOD:
                p -= (borg_skill[BI_MAXDEPTH] * 10L);
                break;
            }
        }

        /* Hack -- try not to destroy "unknown" items (unless "icky") */
        if (!item->ident && (value > 0) && !borg_item_icky(item))
        {
            /* Reward "unknown" items */
            switch (item->tval)
            {
                case TV_SHOT:
                case TV_ARROW:
                case TV_BOLT:
                p -= 100L;
                break;

                case TV_BOW:
                p -= 20000L;
                break;

                case TV_DIGGING:
                p -= 10L;
                break;

                case TV_HAFTED:
                case TV_POLEARM:
                case TV_SWORD:
                p -= 10000L;
                break;

                case TV_BOOTS:
                case TV_GLOVES:
                case TV_HELM:
                case TV_CROWN:
                case TV_SHIELD:
                case TV_CLOAK:
                p -= 15000L;
                break;

                case TV_SOFT_ARMOR:
                case TV_HARD_ARMOR:
                case TV_DRAG_ARMOR:
                p -= 15000L;
                break;

                case TV_AMULET:
                case TV_RING:
                p -= 5000L;
                break;

                case TV_STAFF:
                case TV_WAND:
                p -= 1000L;
                break;
            }
        }

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Attempt to destroy it */
    if (b_i >= 0)
    {
        borg_item *item = &borg_items[b_i];

        /* Debug */
        borg_note(format("# Junking %ld gold (full)", my_power*100 - b_p));

        /* Try to consume the junk */
        if (borg_consume(b_i)) return (TRUE);

        /* Message */
        borg_note(format("# Destroying %s.", item->desc));

        /* Destroy all items */
        borg_keypresses("099");
        borg_keypress('\n');

        /* Destroy that item */
        borg_keypress('k');
        borg_keypress(I2A(b_i));

        /* Verify destruction */
        borg_keypress('y');

        /* Success */
        return (TRUE);
    }

     /* Hack -- no need */
     borg_do_crush_hole = FALSE;

    /* Paranoia */
    return (FALSE);
}



/*
 * Destroy "junk" when slow (in the dungeon).
 *
 * We penalize the loss of both power and monetary value, and reward
 * the loss of weight that may be slowing us down.  The weight loss
 * is worth one gold per tenth of a pound.  This causes things like
 * lanterns and chests and spikes to be considered "annoying".
 */
bool borg_crush_slow(void)
{
    int i, b_i = -1;
    s32b p, b_p = 0L;

    s32b temp;

    s32b greed;

    bool fix = FALSE;

    /* No crush if even slightly dangerous */
    if (borg_danger(c_y,c_x,1, TRUE, FALSE) > borg_skill[BI_CURHP] / 20) return (FALSE);

    /* Hack -- never in town */
    if (borg_skill[BI_CDEPTH] == 0) return (FALSE);

    /* Do not crush items unless we are slow */
    if (borg_skill[BI_SPEED] >= 110) return (FALSE);

	/* Not if in munchkin mode */
	if (borg_munchkin_mode) return (FALSE);

    /* Calculate "greed" factor */
    greed = (borg_gold / 100L) + 100L;

    /* Minimal and maximal greed */
    if (greed < 500L && borg_skill[BI_CLEVEL] > 35) greed = 500L;
    if (greed > 25000L) greed = 25000L;

    /* Decrease greed by our slowness */
    greed -= (110 - borg_skill[BI_SPEED]) * 500;
    if (greed <=0) greed = 0L;

    /* Scan for junk */
    for (i = 0; i < ALL_INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Skip our equipment, but do not skip the quiver */
		if (i >= INVEN_MAX_PACK && i <= INVEN_FEET) continue;

        /* dont crush the swap weapon */
        if (i == weapon_swap && item->iqty == 1) continue;
        if (i == armour_swap) continue;

        /* Skip "good" unknown items (unless "icky") */
        if (!item->ident && !borg_item_icky(item)) continue;

		/* Do not crush Boots, they could be SPEED */
		if (item->tval == TV_BOOTS && !item->ident) continue;

		/* Hack -- Skip artifacts */
        if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && item->name1 && !item->fully_identified) continue;
        if (strstr(item->note, "special")) continue;
        if (strstr(item->note, "terrible")) continue;
        if (strstr(item->note, "indestructible")) continue;

        /* Dont crush weapons if we are weilding a digger */
        if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;

		/* Dont crush it if it is our only source of light */
		if (item->tval == TV_ROD && (item->sval == SV_ROD_LIGHT &&
            borg_skill[BI_CURLITE] <= 0)) continue;

		/* Rods of healing are too hard to come by */
		if (item->tval == TV_ROD && item->sval == SV_ROD_HEALING) continue;

        /* Save the item */
        COPY(&safe_items[i], &borg_items[i], borg_item);

        /* Destroy one of the items */
        borg_items[i].iqty--;

        /* Fix later */
        fix = TRUE;

        /* Examine the inventory */
        borg_notice(FALSE);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        COPY(&borg_items[i], &safe_items[i], borg_item);

        /* Obtain the base price */
        temp = ((item->value < 30000L) ? item->value : 30000L);

        /* Hack -- ignore very cheap items */
        if (temp < greed) temp = 0L;

        /* Penalize */
        p -= temp;

        /* Obtain the base weight */
        temp = item->weight;

        /* Reward */
        p += (temp*100);

        /* Ignore "bad" swaps */
        if (p < b_p) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Examine the inventory */
    if (fix) borg_notice(TRUE);

    /* Destroy "useless" things */
    if ((b_i >= 0) && (b_p >= (my_power)))
    {
        borg_item *item = &borg_items[b_i];

        /* Message */
        borg_note(format("# Junking %ld gold (slow)", (my_power) - b_p));

        /* Attempt to consume it */
        if (borg_consume(b_i)) return (TRUE);

        /* Message */
        borg_note(format("# Destroying %s.", item->desc));

        /* Destroy one item */
        borg_keypress('0');
        borg_keypress('1');
        borg_keypress('\n');

        /* Destroy that item */
        borg_keypress('k');
		/* Quiver Slot */
		if (b_i >= QUIVER_START)
		{
			borg_keypress('/');
			borg_keypress((b_i+73));
		}
		else
		{
			borg_keypress(I2A(b_i));
		}

        /* Verify destruction */
        borg_keypress('y');
    }

    /* Hack -- no need */
    borg_do_crush_slow = FALSE;

    /* Nothing to destroy */
    return (FALSE);
}


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
    int i, b_i = -1;
    s32b v, b_v = -1;
	bool OK_toID = FALSE;

    /* don't ID stuff when you can't recover spent spell point immediately */
    if (((borg_skill[BI_CURSP] < 50 && borg_spell_legal(2, 5)) ||
         (borg_skill[BI_CURSP] < 50 && borg_prayer_legal(5, 2))) &&
        !borg_check_rest(c_y, c_x))
        return (FALSE);

    /* No ID if in danger */
    if (borg_danger(c_y,c_x,1, TRUE, FALSE) > 1) return (FALSE);

    /* Look for an item to identify (equipment) */
    for (i = INVEN_WIELD; i < QUIVER_END; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Reset the value */
		v = -1;

        if (item->fully_identified) continue;
        if (item->ident && item->needs_I) continue;

		if (e_info[borg_items[INVEN_OUTER].name2].xtra == OBJECT_XTRA_TYPE_RESIST ||
				e_info[borg_items[INVEN_OUTER].name2].xtra == OBJECT_XTRA_TYPE_POWER)
	        {
					v = item->value + 100000L;
            }
            if (item->name1)
            {
                switch (item->name1)
                {
                    /* we will id all artifacts */
                    default:
			        /* Get the value */
					v = item->value + 150000L;
                       break;
                }
            }
		/* Prioritize the ID */
        if (strstr(item->note, "magical")) v = item->value +1000L;
        else if (strstr(item->note, "excellent")) v = item->value + 20000L;
        else if (strstr(item->note, "ego")) v = item->value + 20000L;
        else if (strstr(item->note, "splendid")) v = item->value + 20000L;
        else if (strstr(item->note, "special")) v = item->value + 50000L;
        else if (strstr(item->note, "terrible")) v = item->value + 50000L;
        else if (strstr(item->note, "indestructible")) v = item->value + 50000L;
        else if (strstr(item->note, "tried")) v = item->value + 2500L;

		/* Track the best */
        if (v <= b_v) continue;

        /* Track it */
        b_i = i; b_v = v;

    }



    /* Look for an item to identify (inventory) */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Assume nothing */
        v = 0;

        if (item->fully_identified) continue;
        if (item->ident && !item->needs_I) continue;

        if (item->name1)
        {
              switch (item->name1)
              {
                  /* check all artifacts */
                  default:
		        /* Get the value */
				v = item->value + 150000L;
                     break;
              }
        }

        /* Identify "good" (and "terrible") items */
        if (strstr(item->note, "magical")) v = item->value +1000L;
        else if (strstr(item->note, "excellent")) v = item->value + 20000L;
        else if (strstr(item->note, "ego")) v = item->value + 20000L;
        else if (strstr(item->note, "splendid")) v = item->value + 20000L;
        else if (strstr(item->note, "special")) v = item->value + 50000L;
        else if (strstr(item->note, "terrible")) v = item->value + 50000L;
        else if (strstr(item->note, "indestructible")) v = item->value + 50000L;
        else if (strstr(item->note, "tried")) v = item->value + 2500L;

        /* Hack -- reward "unaware" items */
        if (!item->kind)
        {
            /* Analyze the type */
            switch (item->tval)
            {
                case TV_RING:
                case TV_AMULET:

                /* Hack -- reward depth */
                v += (borg_skill[BI_MAXDEPTH] * 5000L);

                break;

                case TV_ROD:

                /* Hack -- reward depth */
                v += (borg_skill[BI_MAXDEPTH] * 3000L);

                break;

                case TV_WAND:
                case TV_STAFF:

                /* Hack -- reward depth */
                v += (borg_skill[BI_MAXDEPTH] * 2000L);

                break;

                case TV_POTION:
                case TV_SCROLL:

                /* Hack -- boring levels */
                if (borg_skill[BI_MAXDEPTH] < 5) break;

                /* Hack -- reward depth */
                v += (borg_skill[BI_MAXDEPTH] * 500L);

                break;

                case TV_FOOD:

                /* Hack -- reward depth */
                v += (borg_skill[BI_MAXDEPTH] * 10L);

                break;
            }
        }

        /* Analyze the type */
        switch (item->tval)
        {
            case TV_CHEST:

            /* Hack -- Always identify chests */
            v = item->value;
            break;

            case TV_WAND:
            case TV_STAFF:

            /* Hack -- Always identify (get charges) */
            v = item->value;
            break;

            case TV_RING:
            case TV_AMULET:

            /* Hack -- Always identify (get information) */
            v = item->value;
            break;

            case TV_LIGHT:

            /* Hack -- Always identify (get artifact info) */
            v = item->value;
            break;

            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
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

            /* Mega-Hack -- use identify spell/prayer */
            if (borg_spell_legal(2, 5) || borg_prayer_legal(5, 2) ||
                borg_equips_artifact(EFF_IDENTIFY, INVEN_WIELD))
            {
                v = item->value;
            }

			/* Certain items needs ID'ing if low level */
			if (borg_skill[BI_CLEVEL] <= 5)
			{
                /* Mega-Hack -- ignore "icky" items */
                if (!borg_item_icky(item)) v = item->value;
			}

            /* Mega-Hack -- mages get bored */
            if ((borg_class == CLASS_MAGE) && (randint0(1000) < borg_skill[BI_CLEVEL]))
            {

                /* Mega-Hack -- ignore "icky" items */
                if (!borg_item_icky(item)) v = item->value;
            }

            /* Mega-Hack -- rangers get bored */
            if ((borg_class == CLASS_RANGER) && (randint0(3000) < borg_skill[BI_CLEVEL]))
            {

                /* Mega-Hack -- ignore "icky" items */
                if (!borg_item_icky(item)) v = item->value;
            }

            /* Mega-Hack -- priests get bored */
            if ((borg_class == CLASS_PRIEST) && (randint0(5000) < borg_skill[BI_CLEVEL]))
            {

                /* Mega-Hack -- ignore "icky" items */
                if (!borg_item_icky(item)) v = item->value;
            }

            /* try to ID shovels */
            if (item->tval == TV_DIGGING) v = item->value;

            /* try to ID my ammo type to stack them up */
            if (item->tval == my_ammo_tval && borg_skill[BI_CLEVEL] >= 15) v = item->value;

            break;
        }

        /* Ignore */
        if (!v) continue;

        /* Track the best */
        if (v <= b_v) continue;

        /* Track it */
        b_i = i; b_v = v;
    }

    /* Found something */
    if (b_i >= 0)
    {
        borg_item *item = &borg_items[b_i];

            /* Use a artifact or activatable item to Identify */
	        if (borg_activate_artifact(EFF_IDENTIFY, INVEN_WIELD))
	        {
                /* Log -- may be cancelled */
                borg_note(format("# Identifying %s.", item->desc));

                /* Equipment */
                if (b_i >= INVEN_WIELD)
                {

                    /* Select the item */
                    borg_keypress(I2A(b_i - INVEN_WIELD));

                    /* HACK need to recheck stats if we id something on us. */
                    for (i = 0; i < 6; i++)
                    {
                        my_need_stat_check[i] = TRUE;
                        my_stat_max[i] = 0;
                    }
                }

                /* Inventory */
                else
                {
                    /* Select the inventory */
                    borg_keypress('/');

                    /* Select the item */
                    borg_keypress(I2A(b_i));
                }

                borg_keypress(ESCAPE);
                /* Success */
                return (TRUE);
            }

            /* Use a Spell/Prayer/Rod/Staff/Scroll of Identify */
            if (borg_spell(2, 5) ||
                borg_prayer(5, 2) ||
                borg_zap_rod(SV_ROD_IDENTIFY) ||
                borg_use_staff(SV_STAFF_IDENTIFY) ||
                borg_read_scroll(SV_SCROLL_IDENTIFY) )
            {
                /* Log -- may be cancelled */
                borg_note(format("# Identifying %s.", item->desc));

                /* Equipment */
                if (b_i >= INVEN_WIELD)
                {
                    /* Select the equipment */
                    borg_keypress('/');

                    /* Select the item */
                    borg_keypress(I2A(b_i - INVEN_WIELD));

                    /* HACK need to recheck stats if we id something on us. */
                    for (i = 0; i < 6; i++)
                    {
                        my_need_stat_check[i] = TRUE;
                        my_stat_max[i] = 0;
                    }
                }

                /* Inventory */
                else
                {
                    /* Select the item */
                    borg_keypress(I2A(b_i));
                }

                borg_keypress(ESCAPE);
                /* Success */
                return (TRUE);
            }
    }

    /* Nothing to do */
    return (FALSE);
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
    int hole = INVEN_MAX_PACK - 1;
    int icky = INVEN_MAX_PACK - 2;
	int i;

    s32b v1, v2;

    bool fix = FALSE;

	char current_right_ring[80];
	char current_left_ring[80];

    borg_item *item;

    /*** Check conditions ***/

    /* Require two empty slots */
    if (borg_items[icky].iqty) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 1000) return (FALSE);
    if (borg_skill[BI_CDEPTH] != 0) return (FALSE);

	/* Forbid if cursed */
	if (borg_wearing_cursed) return (FALSE);

    /*** Remove naked "loose" rings ***/

    /* Remove any naked loose ring */
    if (borg_items[INVEN_LEFT].iqty &&
        !borg_items[INVEN_RIGHT].iqty &&
         borg_items[INVEN_LEFT].activation != EFF_BIZARRE)
    {
        /* Log */
        borg_note("# Taking off naked loose ring.");

        /* Remove it */
        borg_keypress('t');
        borg_keypress(I2A(INVEN_LEFT - INVEN_WIELD));

        /* Success */
        return (TRUE);
    }


    /*** Check conditions ***/

    /* Require "tight" ring */
    if (!borg_items[INVEN_RIGHT].iqty) return (FALSE);


    /* Cannot remove the One Ring */
    if (!borg_items[INVEN_RIGHT].activation == EFF_BIZARRE) return (FALSE);


    /*** Remove nasty "tight" rings ***/

    /* Save the hole */
    COPY(&safe_items[hole], &borg_items[hole], borg_item);

    /* Save the ring */
    COPY(&safe_items[INVEN_LEFT], &borg_items[INVEN_LEFT], borg_item);

    /* Take off the ring */
    COPY(&borg_items[hole], &borg_items[INVEN_LEFT], borg_item);

    /* Erase left ring */
    WIPE(&borg_items[INVEN_LEFT], borg_item);

    /* Fix later */
    fix = TRUE;

    /* Examine the inventory */
    borg_notice(FALSE);

    /* Evaluate the inventory */
    v1 = borg_power();

    /* Restore the ring */
    COPY(&borg_items[INVEN_LEFT], &safe_items[INVEN_LEFT], borg_item);

    /* Restore the hole */
    COPY(&borg_items[hole], &safe_items[hole], borg_item);


    /*** Consider taking off the "right" ring ***/

    /* Save the hole */
    COPY(&safe_items[hole], &borg_items[hole], borg_item);

    /* Save the ring */
    COPY(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT], borg_item);

    /* Take off the ring */
    COPY(&borg_items[hole], &borg_items[INVEN_RIGHT], borg_item);

    /* Erase the ring */
    WIPE(&borg_items[INVEN_RIGHT], borg_item);

    /* Fix later */
    fix = TRUE;

    /* Examine the inventory */
    borg_notice(FALSE);

    /* Evaluate the inventory */
    v2 = borg_power();

    /* Restore the ring */
    COPY(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], borg_item);

    /* Restore the hole */
    COPY(&borg_items[hole], &safe_items[hole], borg_item);



    /*** Swap rings if necessary ***/

	/* Define the rings and descriptions.  */
	strcpy(current_right_ring, borg_items[INVEN_RIGHT].desc);
	strcpy(current_left_ring, borg_items[INVEN_LEFT].desc);

    /* Remove "useless" ring */
    if (v2 > v1)
    {
        /* Log */
        borg_note("# Taking off less valuable right ring.");

        /* Take it off */
		if (borg_items[INVEN_RIGHT].iqty)
		{
				borg_keypress('t');
				borg_keypress(I2A(INVEN_RIGHT - INVEN_WIELD));
				borg_keypress(' ');
		}

		/* make sure one is on the left */
		if (borg_items[INVEN_LEFT].iqty)
		{
			borg_note("# Taking off more valuable left ring.");
			borg_keypress('t');
			borg_keypress(I2A(INVEN_LEFT - INVEN_WIELD));
	        borg_keypress(' ');
		}

        /* Success */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
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
    int slot;
    int hole = INVEN_MAX_PACK - 1;

    s32b p, b_p = 0L;

    int i, b_i = -1;

    borg_item *item;

    bool fix = FALSE;


    /* Require no rings */
    if (borg_items[INVEN_LEFT].iqty) return (FALSE);
    if (borg_items[INVEN_RIGHT].iqty) return (FALSE);

    /* Require two empty slots */
    if (borg_items[hole-1].iqty) return (FALSE);

    /* hack prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);

    /* Scan inventory */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        item = &borg_items[i];


        /* Skip empty items */
        if (!item->iqty) continue;


        /* Require "aware" */
        if (!item->kind) continue;

        /* Require "known" (or average) */
        if (!item->ident && !strstr(item->note, "average")) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        /* skip artifact rings not star id'd  */
        if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && !item->fully_identified && item->name1) continue;

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* Only process "rings" */
        if (slot != INVEN_LEFT) continue;

        /* Occassionally evaluate swapping into the tight finger */
        if (randint0(100) > 75 || item->activation == EFF_BIZARRE)
        {
            slot = INVEN_RIGHT;
        }

        /* Need to be careful not to put the One Ring onto
         * the Left Hand
         */
        if (item->activation == EFF_BIZARRE &&
           (borg_items[INVEN_RIGHT].iqty))
            continue;

        /* Save the old item (empty) */
        COPY(&safe_items[slot], &borg_items[slot], borg_item);

        /* Save the new item */
        COPY(&safe_items[i], &borg_items[i], borg_item);

        /* Wear new item */
        COPY(&borg_items[slot], &safe_items[i], borg_item);

        /* Only a single item */
        borg_items[slot].iqty = 1;

        /* Reduce the inventory quantity by one */
        borg_items[i].iqty--;

        /* Fix later */
        fix = TRUE;

        /* Examine the inventory */
        borg_notice(FALSE);

        /* Evaluate the inventory */
        p = borg_power();

		/* the One Ring would be awsome */
		if (item->activation == EFF_BIZARRE) p = my_power * 2;

        /* Restore the old item (empty) */
        COPY(&borg_items[slot], &safe_items[slot], borg_item);

        /* Restore the new item */
        COPY(&borg_items[i], &safe_items[i], borg_item);

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Restore bonuses */
    if (fix) borg_notice(TRUE);

    /* No item */
    if ((b_i >= 0) && (b_p > my_power))
    {
        /* Get the item */
        item = &borg_items[b_i];

        /* Log */
        borg_note("# Putting on best tight ring.");

        /* Log */
        borg_note(format("# Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(I2A(b_i));

        /* Did something */
        time_this_panel ++;
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
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

    s32b b_p = 0L;
    s32b b_p1 = 0L;
    s32b b_p2 = 0L;

    int i;

	int save_rconf = 0;
	int save_rblind = 0;
	int save_fract = 0;

    borg_item *item;

    bool fix = FALSE;

    /* hack prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (time_this_panel > 300) return (FALSE);

    /* make sure we have an appropriate swap */
    if (armour_swap < 1 && weapon_swap < 1) return (FALSE);

	/* Save our normal condition */
	save_rconf = borg_skill[BI_RCONF];
	save_rblind = borg_skill[BI_RBLIND];
	save_fract = borg_skill[BI_FRACT];

    /* Check the items, first armour then weapon */
    i = armour_swap;

    /* make sure it is not a -1 */
    if (i == -1) i = 0;

    /* get the item */
    item = &borg_items[i];

    /* Where does it go */
    slot = borg_wield_slot(item);

	/* safety check incase slot = -1 */
	if (slot < 0) return (FALSE);

    /* Save the old item (empty) */
    COPY(&safe_items[slot], &borg_items[slot], borg_item);

    /* Save the new item */
    COPY(&safe_items[i], &borg_items[i], borg_item);

    /* Wear new item */
    COPY(&borg_items[slot], &safe_items[i], borg_item);

    /* Only a single item */
    borg_items[slot].iqty = 1;

    /* Reduce the inventory quantity by one */
    borg_items[i].iqty--;

    /* Fix later */
    fix = TRUE;

    /* Examine the benefits of the swap item */
    borg_notice(FALSE);

    /* Evaluate the power with the new item worn */
    b_p1 = borg_danger(c_y,c_x,1, TRUE, FALSE);

    /* Restore the old item (empty) */
    COPY(&borg_items[slot], &safe_items[slot], borg_item);

    /* Restore the new item */
    COPY(&borg_items[i], &safe_items[i], borg_item);

	/* Examine the critical skills */
	if ((save_rconf) && borg_skill[BI_RCONF] == 0) b_p1 = 9999;
	if ((save_rblind) &&
	    (!borg_skill[BI_RBLIND] &&
	     !borg_skill[BI_RLITE] &&
	     !borg_skill[BI_RDARK] &&
	      borg_skill[BI_SAV] < 100)) b_p1 = 9999;
	if ((save_fract) &&
		(!borg_skill[BI_FRACT] &&
		  borg_skill[BI_SAV] < 100)) b_p1 = 9999;

    /* Restore bonuses */
    if (fix) borg_notice(TRUE);

    /*  skip random artifact not star id'd  */
    if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && !item->fully_identified && item->name1) b_p1 = 9999;

    /* skip it if it has not been decursed */
    if ((item->cursed) ||
        (of_has(item->flags, OF_HEAVY_CURSE))) b_p1 = 9999;

    /* Now we check the weapon */

    /* get the item */
    i = weapon_swap;

    /* make sure it is not a -1 */
    if (i == -1) i = 0;

    item = &borg_items[i];

    /* Where does it go */
    slot = borg_wield_slot(item);

	/* safety check incase slot = -1 */
	if (slot < 0) return (FALSE);

    /* Save the old item (empty) */
    COPY(&safe_items[slot], &borg_items[slot], borg_item);

    /* Save the new item */
    COPY(&safe_items[i], &borg_items[i], borg_item);

    /* Wear new item */
    COPY(&borg_items[slot], &safe_items[i], borg_item);

    /* Only a single item */
    borg_items[slot].iqty = 1;

    /* Reduce the inventory quantity by one */
    borg_items[i].iqty--;

    /* Fix later */
    fix = TRUE;

    /* Examine the inventory */
    borg_notice(FALSE);


    /* Evaluate the power with the new item worn */
    b_p2 = borg_danger(c_y,c_x,1, TRUE, FALSE);

	/* Examine the critical skills */
	/* Examine the critical skills */
	if ((save_rconf) && borg_skill[BI_RCONF] == 0) b_p2 = 9999;
	if ((save_rblind) &&
	    (!borg_skill[BI_RBLIND] &&
	     !borg_skill[BI_RLITE] &&
	     !borg_skill[BI_RDARK] &&
	      borg_skill[BI_SAV] < 100)) b_p2 = 9999;
	if ((save_fract) &&
		(!borg_skill[BI_FRACT] &&
		  borg_skill[BI_SAV] < 100)) b_p2 = 9999;

    /* Restore the old item (empty) */
    COPY(&borg_items[slot], &safe_items[slot], borg_item);

    /* Restore the new item */
    COPY(&borg_items[i], &safe_items[i], borg_item);

    /* Restore bonuses */
    if (fix) borg_notice(TRUE);

    /*  skip random artifact not star id'd  */
    if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && !item->fully_identified && item->name1) b_p2 = 9999;

    /* skip it if it has not been decursed */
    if ((item->cursed) ||
        (of_has(item->flags, OF_HEAVY_CURSE))) b_p2 = 9999;

    /* Pass on the swap which yields the best result */
    if (b_p1 <= b_p2)
    {
        b_p = b_p1;
        swap = armour_swap;
    }
    else
    {
        b_p = b_p2;
        swap = weapon_swap;
    }

    /* good swap.  Make sure it helps a significant amount */
    if (p > b_p &&
        b_p <= (borg_fighting_unique?((avoidance*2)/3): (avoidance/2)))
    {
        /* Log */
        borg_note(format("# Swapping backup.  (%d < %d).", b_p, p));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(I2A(swap));

        /* Did something */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}
/*
 * Examine the quiver and unwield any quiver slots that ought to stack
 *
 * Borg will scan the quiver slots for items which match exactly to another
 * slot.  Then unwield that slot.  On his next round, he will enter borg_wear()
 * and wield that stack of missiles, they should then stack into an exisiting
 * quiver slot.
 */
bool borg_stack_quiver(void)
{
	int i, b_i = -1;
	int th, p;
	int b_th = 0;

    borg_item *item;
	borg_item *slot;

    /* hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);
    if (time_this_panel > 150) return (FALSE);

	/* Not if full, we need a free inventory slot */
	if (borg_items[INVEN_PACK].iqty) return (FALSE);

    /* Scan equip */
    for (i = QUIVER_END-1; i >= QUIVER_START; i--)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Skip the non_ID ones */
		if (item->ident == FALSE) continue;

		/* compare this slot to the other known slots. */
		for (p = QUIVER_END-1; p >= QUIVER_START; p--)
		{
			slot = &borg_items[p];

			/* Skip empty items */
			if (!slot->iqty) continue;

			/* Skip the non_ID ones */
			if (item->ident == FALSE) continue;

			/* Dont compare this slot to itself */
			if (p == i) continue;

			/* Compare the slots */
			if (item->name2 == slot->name2 &&
				item->to_d == slot->to_d &&
				item->to_h == slot->to_h &&
				item->dd == slot->dd &&
				item->ds == slot->ds &&
				streq(item->note, slot->note) &&
				(item->iqty + slot->iqty <= 99))
			{
				/* These are essentially the same */
				borg_note(format("# Unwielding %s.  Combining quiver slots %c & %c.",
					item->desc, i+73, p+73));

				/* Wear it */
				borg_keypress('t');
				borg_keypress(i + 73);

				/* Did something */
				time_this_panel ++;
				return (TRUE);
			}
		}
	}

    /* Nope */
    return (FALSE);

}

/*
 * Examine the quiver and dump any worthless items
 *
 * Borg will scan the quiver slots for items which are cursed or have
 * negative bonuses.  Then shoot those iems to get rid of them.
 * He needs to do so when safe.
 */
bool borg_dump_quiver(void)
{
	int i, b_i = -1;
	int th;
	int b_th = 0;
	int quiver_capacity;

    borg_item *item;

    /* hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);
    if (time_this_panel > 150) return (FALSE);

	/* How many should I carry */
	if (borg_class == CLASS_RANGER || borg_class == CLASS_WARRIOR) quiver_capacity = 198;
	else quiver_capacity = 99;

    /* Scan equip */
    for (i = QUIVER_END-1; i >= QUIVER_START; i--)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip if it is not cursed and matches my ammo.  If it is not cursed but does
		 * not match my ammo, then it is dumped.
		 */
        if (!item->cursed && item->tval == my_ammo_tval)
		{
			/* It has some value */
			if (item->to_d > 0 && item->to_h > 0) continue;
			if (strstr(item->note, "magical") ||
				strstr(item->note, "excellent") ||
				strstr(item->note, "ego") ||
				strstr(item->note, "splendid") ||
				strstr(item->note, "special")) continue;

			/* Limit the amount of missiles carried */
			if (borg_skill[BI_AMISSILES] <= quiver_capacity && item->to_d >= 0 && item->to_h >= 0) continue;
		}

		/* Track a crappy one */
		b_i = i;
	}

    /* No item */
    if (b_i >= 0)
    {
        /* Get the item */
        item = &borg_items[b_i];

        /* Log */
        borg_note(format("# Dumping %s.  Bad ammo in quiver.",
            item->desc));

        /* Wear it */
        borg_keypress('v');
        borg_keypress('/');
        borg_keypress(b_i + 73);
		borg_keypress('2');

        /* Did something */
        time_this_panel ++;
        return (TRUE);
    }

    /* Nope */
    return (FALSE);

}

/*
 * Remove useless equipment.
 *
 * Look through the inventory for equipment that is reducing power.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are removed, and we take
 * one step towards the world in which we have the most "power".
 */
bool borg_remove_stuff(void)
{
    int hole = INVEN_MAX_PACK - 1;

    s32b p, b_p = 0L, w_p= 0L;

    int i, b_i = -1;

    borg_item *item;

    bool fix = FALSE;

    /*  hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

	/* We need room to work */
	if (borg_items[hole].iqty) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);
    if (time_this_panel > 150) return (FALSE);

    /* Start with good power */
	b_p = my_power;

    /* Scan equip */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        item = &borg_items[i];


        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (!item->kind) continue;

        /* Require "known" (or average, good, etc) */
        if (!item->ident &&
            !strstr(item->note, "average") &&
            !strstr(item->note, "magical") &&
            !strstr(item->note, "ego") &&
            !strstr(item->note, "splendid") &&
            !strstr(item->note, "excellent") &&
            !strstr(item->note, "indestructible") &&
            !strstr(item->note, "special")) continue;

        /* skip it if it has not been decursed */
        if (item->cursed) continue;


        /* Save the hole */
        COPY(&safe_items[hole], &borg_items[hole], borg_item);

        /* Save the item */
        COPY(&safe_items[i], &borg_items[i], borg_item);

        /* Take off the item */
        COPY(&borg_items[hole], &safe_items[i], borg_item);

        /* Erase the item from equip */
        WIPE(&borg_items[i], borg_item);

        /* Fix later */
        fix = TRUE;

        /* Examine the inventory */
        borg_notice(FALSE);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        COPY(&borg_items[i], &safe_items[i], borg_item);

        /* Restore the hole */
        COPY(&borg_items[hole], &safe_items[hole], borg_item);


        /* Track the crappy items */
        /* crappy includes things that do not add to power */

        if (p >= b_p)
        {
            b_i = i;
            w_p = p;
        }

    }

    /* Restore bonuses */
    if (fix) borg_notice(TRUE);

    /* No item */
    if (b_i >= 0)
    {
        /* Get the item */
        item = &borg_items[b_i];

#if 0
		/* dump list and power...  for debugging */
        borg_note(format("Equip Item %d %s.",  i, safe_items[i].desc));
        borg_note(format("With Item     (borg_power %ld)", b_p));
        borg_note(format("Removed Item  (best power %ld)", p));
#endif

        /* Log */
        borg_note(format("# Removing %s.  Power with: (%ld) Power w/o (%ld)",
            item->desc, b_p, w_p));

        /* Wear it */
        borg_keypress('t');
        borg_keypress(I2A(b_i- INVEN_WIELD));

        /* Did something */
        time_this_panel ++;
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
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
    int hole = INVEN_MAX_PACK - 1;

    int slot;
    int d;
	int o;
	bool recently_worn = FALSE;

    s32b p, b_p = 0L;

    int i, b_i = -1;
    int ii, b_ii =  -1;
    int danger;

	char target_ring_desc[80];

    borg_item *item;

    bool fix = FALSE;

	/* Start with current power */
	b_p = my_power;

    /*  hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

	/* We need an empty slot to simulate pushing equipment */
	if (borg_items[hole].iqty) return (FALSE);

	/* Forbid if cursed */
    if (borg_wearing_cursed) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);
    if (time_this_panel > 1300) return (FALSE);

    /* Scan inventory */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        item = &borg_items[i];

		/* reset this item marker */
		recently_worn = FALSE;

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (!item->kind) continue;

        /* Require "known" (or average, good, etc) for armors.
		 * It is ok to wear unknown weapons/ bows because we can pseudoID them through use
		 */
        if (!item->ident &&
            !strstr(item->note, "average") &&
            !strstr(item->note, "magical") &&
            !strstr(item->note, "ego") &&
            !strstr(item->note, "splendid") &&
            !strstr(item->note, "excellent") &&
            !strstr(item->note, "indestructible") &&
            !strstr(item->note, "special")) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        /* do not wear not *idd* artifacts */
        if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) &&
            !item->fully_identified && item->name1) continue;

        /* skip it if it has not been decursed, unless the One Ring */
        if ((item->cursed) &&
            (item->activation != EFF_BIZARRE)) continue;

    	/* Do not consider wearing this item if I worn it already this level,
    	 * I might be stuck in a loop.
     	 */
    	for (o = 0; o < track_worn_num; o++)
	    {
        	/* Examine the worn list */
	        if (track_worn_num >=1 && track_worn_name1[o] == item->name1 &&
				track_worn_time > borg_t - 10)
        	{
            	/* Recently worn item */
	            recently_worn = TRUE;
        	}
     	}

		/* Note and fail out */
		if (recently_worn == TRUE)
		{
			borg_note(format("# Not considering a %s; it was recently worn.", item->desc));
			continue;
		}

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* Cannot wear this item */
        if (slot < 0) continue;

        /* Make sure that slot does not have a cursed item */
        if (borg_items[slot].cursed ||
            of_has(item->flags, OF_LIGHT_CURSE)||
            of_has(item->flags, OF_HEAVY_CURSE)||
            of_has(item->flags, OF_PERMA_CURSE)) continue;

		/* Do not wear certain items if I am over weight limit.  It induces loops */
		if (borg_skill[BI_ISENCUMB])
		{
			/* Compare Str bonuses */
			if (of_has(borg_items[slot].flags, OF_STR) &&
				!of_has(item->flags, OF_STR)) continue;
			/* Compare Str bonuses */
			else if (of_has(borg_items[slot].flags, OF_STR) &&
				of_has(item->flags, OF_STR) &&
				borg_items[slot].pval > item->pval) continue;
		}

        /* Obtain danger */
        danger = borg_danger(c_y,c_x,1, TRUE, FALSE);

        /* If this is a ring and both hands are full, then check each hand
         * and compare the two.  If needed the tight ring can be removed then
         * the better ring placed there on.
         */

        /*** Process regular items and non full rings ***/

        /* Non ring, non full hands */
        if (slot != INVEN_LEFT ||
            (!borg_items[INVEN_LEFT].tval || !borg_items[INVEN_RIGHT].tval))
        {
            /* Save the old item */
            COPY(&safe_items[slot], &borg_items[slot], borg_item);

            /* Save the new item */
            COPY(&safe_items[i], &borg_items[i], borg_item);

            /* Save the hole */
            COPY(&safe_items[hole], &borg_items[hole], borg_item);

            /* Take off old item */
            COPY(&borg_items[hole], &safe_items[slot], borg_item);

            /* Wear new item */
            COPY(&borg_items[slot], &safe_items[i], borg_item);

            /* Only a single item */
            borg_items[slot].iqty = 1;

            /* Reduce the inventory quantity by one */
            borg_items[i].iqty--;

            /* Fix later */
            fix = TRUE;

            /* Examine the inventory */
            borg_notice(FALSE);

            /* Evaluate the inventory */
            p = borg_power();

            /* Evaluate local danger */
            d = borg_danger(c_y,c_x,1, TRUE, FALSE);

			if (borg_verbose)
			{
				/* dump list and power...  for debugging */
				borg_note(format("Trying Item %s (best power %ld)",borg_items[slot].desc, p));
				borg_note(format("Against Item %s   (borg_power %ld)",safe_items[slot].desc, b_p));
			}

            /* Restore the old item */
            COPY(&borg_items[slot], &safe_items[slot], borg_item);

            /* Restore the new item */
            COPY(&borg_items[i], &safe_items[i], borg_item);

            /* Restore the hole */
            COPY(&borg_items[hole], &safe_items[hole], borg_item);

	        /* Need to be careful not to put the One Ring onto
	         * the Left Hand
	         */
	        if (item->activation == EFF_BIZARRE &&
	        	!borg_items[INVEN_LEFT].tval)
	            p = -99999;

            /* Ignore if more dangerous */
            if (danger < d) continue;

            /* XXX XXX XXX Consider if slot is empty */

            /* Hack -- Ignore "essentially equal" swaps */
            if (p <= b_p+50) continue;

            /* Maintain the "best" */
            b_i = i; b_p = p;
        } /* non-rings, non full */


	if (randint0(100)==10 || item->activation == EFF_BIZARRE)
	{
        /* ring, full hands */
        if (slot == INVEN_LEFT &&
            borg_items[INVEN_LEFT].tval && borg_items[INVEN_RIGHT].tval)
        {
                for (ii = INVEN_LEFT; ii <= INVEN_RIGHT; ii++)
                {
                    slot = ii;

                    /* Does One Ring need to be handled here? */

                    /* Save the old item */
                    COPY(&safe_items[slot], &borg_items[slot], borg_item);

                    /* Save the new item */
                    COPY(&safe_items[i], &borg_items[i], borg_item);

                    /* Save the hole */
                    COPY(&safe_items[hole], &borg_items[hole], borg_item);

                    /* Take off old item */
                    COPY(&borg_items[hole], &safe_items[slot], borg_item);

                    /* Wear new item */
                    COPY(&borg_items[slot], &safe_items[i], borg_item);

                    /* Only a single item */
                    borg_items[slot].iqty = 1;

                    /* Reduce the inventory quantity by one */
                    borg_items[i].iqty--;

                    /* Fix later */
                    fix = TRUE;

                    /* Examine the inventory */
                    borg_notice(FALSE);

                    /* Evaluate the inventory */
                    p = borg_power();

                    /* Evaluate local danger */
                    d = borg_danger(c_y,c_x,1, TRUE, FALSE);

                    /* Restore the old item */
                    COPY(&borg_items[slot], &safe_items[slot], borg_item);

                    /* Restore the new item */
                    COPY(&borg_items[i], &safe_items[i], borg_item);

                    /* Restore the hole */
                    COPY(&borg_items[hole], &safe_items[hole], borg_item);

			        /* Need to be careful not to put the One Ring onto
			         * the Left Hand
			         */
			        if (ii == INVEN_LEFT &&
			            item->activation == EFF_BIZARRE)
			            p = -99999;

                    /* Ignore "bad" swaps */
                    if (p < b_p) continue;

                    /* no swapping into more danger */
                    if (danger <= d && danger != 0) continue;

                    /* Maintain the "best" */
                    b_i = i; b_p = p; b_ii = ii;

                }
            } /* ring, looking at replacing each ring */
		} /* Random ring check */

    } /* end scanning inventory */

    /* Restore bonuses */
    if (fix) borg_notice(TRUE);

    /* item */
    if ((b_i >= 0) && (b_p > my_power))
    {
        /* Get the item */
        item = &borg_items[b_i];

		/* Define the desc of the nice ring */
		strcpy(target_ring_desc, item->desc);

        /* Remove old ring to make room for good one */
        if (b_ii >= INVEN_RIGHT && item->tval == TV_RING)
        {
            /* Log */
            borg_note(format("# Removing %s to make room for %s.", &borg_items[b_ii].desc, item->desc));

            /* Make room */
            borg_keypress('t');
            borg_keypress(I2A(b_ii-INVEN_WIELD));

	        /* Did something */
	        time_this_panel ++;
	        return (TRUE);
        }

        /* Log */
        borg_note(format("# Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(I2A(b_i));
        time_this_panel ++;

        /* Track the newly worn artifact item to avoid loops */
        if (item->name1 && (track_worn_num < track_worn_size))
        {
            borg_note("# Noting the wearing of artifact.");
            track_worn_name1[track_worn_num] = item->name1;
			track_worn_time = borg_t;
            track_worn_num++;
        }
        return (TRUE);

	}

    /* Nope */
    return (FALSE);
}

/*
 * Equip useful missiles.
 *
 * Look through the inventory for missiles that need to be wielded.
 * The quiver has 10 slots and can carry 99 of the identical missiles.
 */
bool borg_wear_quiver(void)
{
    int hole = QUIVER_END-1;
	int p;
	borg_item *slot;
    int i, b_i = -1;
	char target_desc[80];
    borg_item *item;
    bool fix = FALSE;
	bool match = FALSE;

    /* hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (FALSE);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (FALSE);
    if (time_this_panel > 1300) return (FALSE);

    /* Scan inventory */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (item->tval != my_ammo_tval) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

		/* if the quivers and full and this missile won't stack into an existing
		 * quiver stack, then leave it.
		 * For the most part, an arrow will be placed into an occupied quiver slot
		 * if it matches the type in the slot.
		 */
	    if (borg_items[hole].iqty)
		{
			for (p = QUIVER_START; p < QUIVER_END; p++)
			{
				slot = &borg_items[p];

				if (item->name2 == slot->name2 &&
					item->dd  == slot->dd &&
					item->ds  == slot->ds &&
					item->to_d  == slot->to_d &&
					item->to_h  == slot->to_h)
				{
					match = TRUE;
					b_i = i;
				}
			}
		}
		else
		{
			/* Load these missiles into the non-full quiver */
			b_i = i;
		}
    } /* end scanning inventory */

    /* Restore bonuses */
    if (fix) borg_notice(TRUE);

    /* item */
    if (b_i >= 0)
    {
        /* Get the item */
        item = &borg_items[b_i];

		/* Define the desc of the nice ring */
		strcpy(target_desc, item->desc);

        /* Log */
        borg_note(format("# Loading Quiver %s.", item->desc));

        /* Wear it */
		borg_keypress(ESCAPE);
        borg_keypress('w');
        borg_keypress(I2A(b_i));
        time_this_panel ++;
        return (TRUE);

	}

    /* Nope */
    return (FALSE);
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
static byte borg_best_stuff_order[] =
{
    INVEN_BOW,
    INVEN_WIELD,
    INVEN_BODY,
    INVEN_OUTER,
    INVEN_ARM,
    INVEN_HEAD,
    INVEN_HANDS,
    INVEN_FEET,
    INVEN_LEFT,
    INVEN_LIGHT,
    INVEN_NECK,

    255
};


/*
 * Helper function (see below)
 */
static void borg_best_stuff_aux(int n, byte *test, byte *best, s32b *vp)
{
    int i;

    int slot;


    /* Extract the slot */
    slot = borg_best_stuff_order[n];

    /* All done */
    if (slot == 255)
    {
        s32b p;

        /* Examine */
        borg_notice(FALSE);

        /* Evaluate */
        p = borg_power();

        /* Track best */
        if (p > *vp)
        {

#if 0
            /* dump list and power...  for debugging */
            borg_note(format("Trying Combo (best power %ld)", *vp));
            borg_note(format("             (borg_power %ld)", p));
            for (i = 0; i < INVEN_MAX_PACK; i++)
                borg_note(format("inv %d %s.",  i, borg_items[i].desc));
            for (i=0; borg_best_stuff_order[i] != 255; i++)
                borg_note(format("stuff %s.",
                    borg_items[borg_best_stuff_order[i]].desc));
#endif
            /* Save the results */
            for (i = 0; i < n; i++) best[i] = test[i];

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
    for (i = 0; i < ((shop_num == 7) ? (INVEN_MAX_PACK + STORE_INVEN_MAX) : INVEN_MAX_PACK); i++)
    {
        borg_item *item;
        if (i < INVEN_MAX_PACK)
            item = &borg_items[i];
        else
            item = &borg_shops[7].ware[i - INVEN_MAX_PACK];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (!item->kind) continue;

        /* Require "known" (or average, good, etc) */
        if (!item->ident &&
            !strstr(item->note, "average") &&
            !strstr(item->note, "magical") &&
            !strstr(item->note, "ego") &&
            !strstr(item->note, "splendid") &&
            !strstr(item->note, "excellent") &&
            !strstr(item->note, "indestructible") &&
            !strstr(item->note, "special")) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        /* Skip it if it has not been decursed */
        if (item->cursed ||
            of_has(item->flags, OF_LIGHT_CURSE)||
            of_has(item->flags, OF_HEAVY_CURSE)||
            of_has(item->flags, OF_PERMA_CURSE)) continue;

        /* Do not wear not *idd* artifacts */
        if ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) &&
            !item->fully_identified && item->name1) continue;

        /* Make sure it goes in this slot, special consideration for checking rings */
        if (slot != borg_wield_slot(item)) continue;

        /* Make sure that slot does not have a cursed item */
        if (borg_items[slot].cursed ||
            of_has(item->flags, OF_LIGHT_CURSE)||
            of_has(item->flags, OF_HEAVY_CURSE)||
            of_has(item->flags, OF_PERMA_CURSE)) continue;

		/* Do not wear certain items if I am over weight limit.  It induces loops */
		if (borg_skill[BI_ISENCUMB])
		{
			/* Compare Str bonuses */
			if (of_has(borg_items[slot].flags, OF_STR) &&
				!of_has(item->flags, OF_STR)) continue;
			/* Compare Str bonuses */
			else if (of_has(borg_items[slot].flags, OF_STR) &&
				of_has(item->flags, OF_STR) &&
				borg_items[slot].pval > item->pval) continue;
		}

        /* Wear the new item */
        COPY(&borg_items[slot], item, borg_item);

        /* Note the attempt */
        if (i < INVEN_MAX_PACK)
            test[n] = i;
        else
            /* if in home, note by adding 100 to item number. */
            test[n] = (i - INVEN_MAX_PACK) + 100;


        /* Evaluate the possible item */
        borg_best_stuff_aux(n + 1, test, best, vp);

        /* Restore equipment */
        COPY(&borg_items[slot], &safe_items[slot], borg_item);
    }
}


/*
 * Attempt to instantiate the *best* possible equipment.
 */
bool borg_best_stuff(void)
{
    int hole = INVEN_MAX_PACK - 1;
	char purchase_target[1];
    int k;
	byte t_a;
    char buf[1024];
	int p;

    s32b value;

    int i;

    byte test[12];
    byte best[12];


	/* Hack -- Anti-loop */
    if (time_this_panel >= 300) return (FALSE);

    /* Hack -- Initialize */
    for (k = 0; k < 12; k++)
    {
        /* Initialize */
        best[k] = test[k] = 255;
    }

    /* Hack -- Copy all the slots */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
		/* Skip quiver slots */
		if (i >= INVEN_MAX_PACK && i < INVEN_WIELD) continue;

		/* Save the item */
        COPY(&safe_items[i], &borg_items[i], borg_item);
    }

    if (shop_num == 7)
    {
        /* Hack -- Copy all the store slots */
        for (i = 0; i < STORE_INVEN_MAX; i++)
        {
            /* Save the item */
            COPY(&safe_home[i], &borg_shops[7].ware[i], borg_item);
        }
    }

    /* Evaluate the inventory */
    value = my_power;

    /* Determine the best possible equipment */
    (void)borg_best_stuff_aux(0, test, best, &value);

    /* Restore bonuses */
    borg_notice(TRUE);

    /* Make first change. */
    for (k = 0; k < 12; k++)
    {
        /* Get choice */
        i = best[k];

        /* Ignore non-changes */
        if (i == borg_best_stuff_order[k] || 255 == i) continue;

        if (i < 100)
        {
			borg_item *item = &borg_items[i];

			/* Catch the keyboard flush induced from the 'w' */
	        if ((0 == borg_what_text(0, 0, 6, &t_a, buf)) &&
				(streq(buf, "(Inven")))
			{
				borg_keypress(I2A(i));

				/* Track the newly worn artifact item to avoid loops */
				if (item->name1 && (track_worn_num < track_worn_size))
				{
					borg_note("# Noting the wearing of artifact.");
					track_worn_name1[track_worn_num] = item->name1;
					track_worn_time = borg_t;
					track_worn_num++;
				}
			}
			else
			{
				/* weild the item */
				borg_note(format("# Best Combo %s.", item->desc));
				borg_keypress('w');
				borg_best_item = i;
				return (TRUE);
			}

			/* Full rings?  Select which to replace */
#if 0
			if (item->tval == TV_RING && borg_items[INVEN_RIGHT].iqty && borg_items[INVEN_LEFT].iqty)
			{
				/* Left Ring */
				borg_keypress('c');
			}
#endif
            time_this_panel ++;


			return (TRUE);
	    }
        else
        {
            borg_item *item;

            /* can't get an item if full. */
            if (borg_items[hole].iqty) return (FALSE);

            i-=100;

            item = &borg_shops[7].ware[i];

            /* Dont do it if you just sold this item */
            for (p = 0; p < sold_item_num; p++)
			{
				if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval &&
                    sold_item_store[p] == 7) return (FALSE);
			}

            /* Get the item */
            borg_note(format("# Getting (Best Fit) %s.", item->desc));

			/* Define the special key */
			purchase_target[0] = shop_orig[i];

	        /* Purchase that item */
			borg_keypress(purchase_target[0]);
            borg_keypress('p');

            /* press SPACE a few time (mulitple objects) */
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);


            /* tick the clock */
            time_this_panel ++;

			/* Note that this is a nice item and not to sell it right away */
			borg_best_fit_item = item->name1;

            return (TRUE);
        }
    }

    /* Nope */
    return (FALSE);
}






/*
 * Study and/or Test spells/prayers
 */
bool borg_play_magic(bool bored)
{
    int book, b_book = -1;
    int what;

    int i, b_i = -1;
    int j, b_j = -1;
    int r, b_r = -1;


    /* Hack -- must use magic or prayers */
    if (!p_ptr->class->spell_book) return (FALSE);


    /* Hack -- blind/confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (FALSE);

    /* Dark */
    if (!borg_skill[BI_CURLITE]) return (FALSE);
    if (borg_grids[c_y][c_x].info == BORG_DARK) return (FALSE);

    /* Check each book (backwards) */
    for (book = 9-1 ; book >= 0; book--)
    {
        /* Look for the book in inventory*/
        i = borg_book[book];

        /* No such book */
        if (i < 0) continue;

        /* Check each spells */
        for (what = 9 - 1 ; what >= 0; what--)
        {
            borg_magic *as = &borg_magics[book][what];

            /* Require "learnable" status */
            if (as->status != BORG_MAGIC_OKAY) continue;

            /* Obtain "index" */
            j = what;

            /* Obtain "rating" */
            r = as->rating;

            /* Skip "boring" spells/prayers */
            if (!bored && (r <= 50)) continue;

            /* Skip "icky" spells/prayers */
            if (r <= 0) continue;

            /* Skip "worse" spells/prayers */
            if (r <= b_r) continue;

            /* Track it */
            b_i = i;
            b_j = j;
            b_r = r;
            b_book = book;
        }
    }


    /* Study */
    if (borg_skill[BI_ISSTUDY] && (b_r > 0))
    {
        borg_magic *as = &borg_magics[b_book][b_j];

        /* Debugging Info */
        borg_note(format("# Studying spell/prayer %s.", as->name));

        /* Learn the spell */
        borg_keypress('G');

        /* Specify the book */
        borg_keypress(I2A(b_i));

        /* Specify the spell (but not the prayer) */
        if (player_has(PF_CHOOSE_SPELLS))
        {
            /* Specify the spell */
            borg_keypress(I2A(b_j));
        }

        /* Success */
        return (TRUE);
    }


    /* Hack -- only in town */
    if (borg_skill[BI_CDEPTH] && !borg_munchkin_mode) return (FALSE);

    /* Hack -- only when bored */
    if (!bored) return (FALSE);


    /* Check each book (backwards) */
    for (book = 9 - 1; book >= 0; book--)
    {
        /* Look for the book */
        i = borg_book[book];

        /* No such book */
        if (i < 0) continue;

        /* Check every spell (backwards) */
        for (what = 9 - 1; what >= 0; what--)
        {
            borg_magic *as = &borg_magics[book][what];

            /* Only try "untried" spells/prayers */
            if (as->status != BORG_MAGIC_TEST) continue;

            /* Ignore "bizarre" spells/prayers */
            if (as->method == BORG_MAGIC_OBJ) continue;

            /* Make sure I have enough mana */
            if (borg_skill[BI_CURSP] <= as->power) continue;

			/* Some spells should not be tested in munchkin mode */
			if (borg_munchkin_mode)
			{
				/* Priest type */
				if (player_has(PF_CHOOSE_SPELLS) &&
					book == 1 && what == 1) continue;

				/* Mage types */
				if (!player_has(PF_CHOOSE_SPELLS) &&
					book == 0 && what == 2) continue;
			}

            /* Note */
            borg_note("# Testing untried spell/prayer");

            /* Hack -- Use spell or prayer */
            if (borg_spell(book, what) ||
                borg_prayer(book, what))
            {
                /* Hack -- Allow attack spells */
                if (as->method == BORG_MAGIC_AIM)
                {
                    /* Hack -- target self */
                    borg_keypress('*');
                    borg_keypress('p');
                    borg_keypress('t');
                }

                /* Hack -- Allow genocide spells */
                if (as->method == BORG_MAGIC_WHO)
                {
                    /* Hack -- target Maggot */
                    borg_keypress('h');
                }

                /* Success */
                return (TRUE);
            }
        }
    }

    /* Nope */
    return (FALSE);
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

    s32b price;
    s32b greed;


    /* Calculate "greed" factor */
    greed = (borg_gold / 100L) + 100L;

    /* Minimal greed */
    if (greed < 1000L) greed = 1000L;
    if (greed > 25000L) greed = 25000L;
	if (borg_skill[BI_MAXDEPTH] >= 50) greed = 75000;
	if (borg_skill[BI_CLEVEL] < 25) greed = (borg_gold / 100L) + 50L;
	if (borg_skill[BI_CLEVEL] < 20) greed = (borg_gold / 100L) + 35L;
	if (borg_skill[BI_CLEVEL] < 15) greed = (borg_gold / 100L) + 20L;
	if (borg_skill[BI_CLEVEL] < 13) greed = (borg_gold / 100L) + 10L;
	if (borg_skill[BI_CLEVEL] < 10) greed = (borg_gold / 100L) + 5L;
	if (borg_skill[BI_CLEVEL] < 5) greed = (borg_gold / 100L);

    /* Count "sellable" items */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "crappy" items */
        if (item->value <= 0) continue;

        /* skip our swap weapon */
        if (i == weapon_swap) continue;
        if (i == armour_swap) continue;

		/* Dont sell my ammo */
		if (item->tval == my_ammo_tval) continue;

		/* Don't sell my books */
        if (item->tval == p_ptr->class->spell_book ) continue;

		/* Don't sell my needed potion/wands/staff/scroll collection */
		if ((item->tval == TV_POTION && item->sval == SV_POTION_CURE_SERIOUS) ||
			(item->tval == TV_POTION && item->sval == SV_POTION_CURE_CRITICAL) ||
			(item->tval == TV_POTION && item->sval == SV_POTION_HEALING) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_STAR_HEALING) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_LIFE) ||
            (item->tval == TV_POTION && item->sval == SV_POTION_SPEED) ||
            (item->tval == TV_STAFF && item->sval == SV_STAFF_TELEPORTATION) ||
            (item->tval == TV_WAND && item->sval == SV_WAND_DRAIN_LIFE) ||
            (item->tval == TV_WAND && item->sval == SV_WAND_ANNIHILATION) ||
            (item->tval == TV_SCROLL && item->sval == SV_SCROLL_TELEPORT)) continue;
        /* Obtain the base price */
        price = ((item->value < 30000L) ? item->value : 30000L);

        /* Skip cheap "known" (or "average") items */
        if ((price * item->iqty < greed) &&
            (item->ident || strstr(item->note, "average"))) continue;

        /* Count remaining items */
        k++;
    }

    /* Result */
    return (k);
}


/*
 * Scan the item list and recharge items before leaving the
 * level.  Right now rod are not recharged from this.
 */
bool borg_wear_recharge(void)
{
    int i, b_i = -1;
	int slot = -1;
	int b_slot = -1;


    /* No resting in danger */
    if (!borg_check_rest(c_y, c_x)) return (FALSE);

    /* Not if hungry */
    if (borg_skill[BI_ISWEAK]) return (FALSE);

    /* Look for an (wearable- non rod) item to recharge */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];
        object_type *o_ptr;  /* cheat */
        o_ptr = &p_ptr->inventory[i]; /* cheat */

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Skip the quiver slots from the inventory */
		if (i >= INVEN_MAX_PACK && i < INVEN_WIELD) continue;

        /* skip items that are charged */
        if (!item->timeout) continue;

		/* Where can it be worn? */
		slot = borg_wield_slot(item);

		/* skip non-ego lights, No need to rest to recharge a torch, which uses fuels turns in o_ptr->timeout */
		if (item->tval == TV_LIGHT && !of_has(item->flags, OF_NO_FUEL)) continue;

        /* note this one */
        b_i = i;
		b_slot = slot;
    }

    if (b_i >= INVEN_WIELD)
    {
        /* Item is worn, no swap is nec. */
        borg_note(format("# Waiting for '%s' to Recharge.", borg_items[b_i].desc));

        /* Rest for a while */
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress('\n');

        /* done */
        return (TRUE);
    }
    /* Item must be worn to be recharged
     * But, none if some equip is cursed
     */
    if (b_slot >= INVEN_WIELD && !borg_wearing_cursed)
    {

        /* wear the item */
        borg_note("# Swapping Item for Recharge.");
        borg_keypress(ESCAPE);
        borg_keypress('w');
        borg_keypress(I2A(b_i));
        borg_keypress(' ');
        borg_keypress(' ');

        /* rest for a while */
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress('\n');

        /* done */
        return (TRUE);

    }

    /* nothing to recharge */
    return (FALSE);
}

/*
 * Leave the level if necessary (or bored)
 * Scumming defined in borg_prepared.
 */
bool borg_leave_level(bool bored)
{
    int k, g = 0;

	bool need_restock = FALSE;

    /* Hack -- waiting for "recall" other than depth 1 */
    if (goal_recalling && borg_skill[BI_CDEPTH] > 1) return (FALSE);

	/* Not bored if I have seen Morgoth recently */
	if (borg_skill[BI_CDEPTH] == 100 && morgoth_on_level &&
		(borg_t - borg_t_morgoth < 5000))
	{
		goal_leaving = FALSE;
		goal_rising = FALSE;
		bored = FALSE;
	}

    /* There is a great concern about recalling back to level 100.
     * Often the borg will fall down a trap door to level 100 when he is not
     * prepared to be there.  Some classes can use Teleport Level to get
     * back up to 99,  But Warriors cannot.  Realistically the borg needs
     * be be able to scum deep in the dungeon.  But he cannot risk being
     * on 100 and using the few *Healing* pots that he managed to collect.
     * It is better for warriors to walk all the way down to 98 and scum.
     * It seems like a long and nasty crawl, but it is the best way to
     * make sure the borg survives.  Along the way he will collect the
     * Healing, Life and *Healing* that he needs.
     *
     * The other classes (or at least those who can use the Teleport Level
     * spell) will not need to do this nasty crawl.  Risky Borgs will
     * not crawl either.
     */

    /* Town */
    if (!borg_skill[BI_CDEPTH])
    {
        /* Cancel rising */
        goal_rising = FALSE;

        /* Wait until bored */
        if (!bored) return (FALSE);

        /* Case for those who cannot Teleport Level */
        if (borg_skill[BI_MAXDEPTH] == 100 && !borg_plays_risky)
        {
	        if (borg_skill[BI_ATELEPORTLVL] == 0)
            {
                /* These pple must crawl down to 100, Sorry */
                goal_fleeing = TRUE;
                goal_leaving = TRUE;
                stair_more = TRUE;

                /* Note */
                borg_note("# Borg must crawl to deep dungeon- no recall to 100.");

                /* Attempt to use those stairs */
                if (borg_flow_stair_more(GOAL_BORE, FALSE, FALSE)) return (TRUE);

                /* Oops */
                return (FALSE);
            }
        }


        /* Hack -- Recall into dungeon */
        if ((borg_skill[BI_MAXDEPTH] >= (borg_worships_gold ? 10 : 8)) &&
             (borg_skill[BI_RECALL] >= 3) &&
             (((cptr)NULL == borg_prepared(borg_skill[BI_MAXDEPTH]*6/10))||
                borg_plays_risky) &&
            borg_recall())
        {
            /* Note */
            borg_note("# Recalling into dungeon.");

            /* Give it a shot */
            return (TRUE);
        }
        else
        {
            /* note why we didn't recall. */
            if (borg_skill[BI_MAXDEPTH] < (borg_worships_gold ? 10 : 8))
            {
				borg_note("# Not deep enough to recall");
			}
            else
            {
                if (borg_skill[BI_RECALL] <= 2)
                {
					borg_note("# Not enough recalls to recall");
				}
                else
                {
                	/* recall unless way out of our league */
                    if ((cptr)NULL != borg_prepared(borg_skill[BI_MAXDEPTH]*6/10))
                    {
                        borg_note(format("# Way too scary to recall down there!   %s",
                            borg_prepared(borg_skill[BI_MAXDEPTH])));
                    }
                    else
                    {
						borg_note("# failed to recall when I wanted to");
					}
                }
			}
        goal_fleeing = TRUE;
        goal_leaving = TRUE;
        }

        stair_more = TRUE;

        /* Attempt to use those stairs */
        if (borg_flow_stair_more(GOAL_BORE, FALSE, FALSE)) return (TRUE);

        /* Oops */
        return (FALSE);
    }

    /** In the Dungeon **/

    /* do not hangout on boring levels for *too* long */
    if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 1)) g = 1;

    /* Count sellable items */
    k = borg_count_sell();

    /* Do not dive when "full" of items */
    if (g && (k >= 12)) g = 0;

    /* Do not dive when drained */
    if (g && borg_skill[BI_ISFIXEXP]) g = 0;


    /* Hack -- Stay on each level for a minimal amount of time */
    if (g != 0 && borg_skill[BI_MAXCLEVEL] >= 20 &&
        (borg_t - borg_began < value_feeling[borg_feeling]))
    {
        g = 0;
    }

    /* Rise a level if bored and unable to dive. */
    if (bored && ((cptr)NULL != borg_prepared(borg_skill[BI_CDEPTH] + 1)))
    {
         g = -1;
        borg_slow_return = TRUE;
        borg_note(format("# heading up (bored and unable to dive: %s)",
                    borg_prepared(borg_skill[BI_CDEPTH] + 1)));
        borg_slow_return = FALSE;
    }

    /* Rise a level if bored and stastic. */
    if (bored && avoidance > borg_skill[BI_CURHP])
    {
         g = -1;
        borg_slow_return = TRUE;
        borg_note("# heading up (bored and spastic).");
        borg_slow_return = FALSE;
    }

	/* Power dive if I am playing too shallow*/
    if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 5) && k < 13) g = 1;

    /* Power dive if I am playing deep */
    if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 1) &&
         borg_skill[BI_CDEPTH] >= 75) g = 1;

    /* Hack -- Power-climb upwards when needed */
    if ((cptr)NULL != borg_prepared(borg_skill[BI_CDEPTH]))
    {
		/* Certain checks are bypassed if Unique monster on level */
		if (!unique_on_level)
		{
	        borg_slow_return = TRUE;
	        borg_note(format("# Climbing (too deep: %s)",
                            borg_prepared(borg_skill[BI_CDEPTH])));
        	borg_slow_return = FALSE;
        	g = -1;

	        /* if I am really out of depth go to town */
	        if ((cptr)NULL != borg_prepared(borg_skill[BI_MAXDEPTH]*5/10) &&
	            borg_skill[BI_MAXDEPTH] > 65)
	        {
	            borg_slow_return = TRUE;
	            borg_note(format("# Returning to town (too deep: %s)",
	                            borg_prepared(borg_skill[BI_CDEPTH])));
	            goal_rising = TRUE;
	            borg_slow_return = FALSE;
	        }
		}

        /* if I must  go to town without delay */
        if ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH]))
        {
            borg_note(format("# returning to town to restock(too deep: %s)",
                            borg_restock(borg_skill[BI_CDEPTH])));
            goal_rising = TRUE;
            need_restock = TRUE;
        }

		/* I must return to collect stock from the house. */
        if (strstr(borg_prepared(borg_skill[BI_CDEPTH]), "Collect from house"))
        {
            borg_note("# Returning to town to Collect stock.");
            goal_rising = TRUE;
            need_restock = TRUE;
        }

	}

    /* Hack -- if I am playing way too shallow return to town */
    if ((cptr)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 20) &&
        (cptr)NULL == borg_prepared(borg_skill[BI_MAXDEPTH] *6/10) &&
        borg_skill[BI_MAXDEPTH] > borg_skill[BI_CDEPTH] + 20 &&
		(borg_skill[BI_RECALL] >= 3 || borg_gold > 2000) )
    {
        borg_note("# returning to town to recall back down (too shallow)");
        goal_rising = TRUE;
    }

    /* Hack -- It is much safer to scum for items on 98
     * Check to see if depth 99, if Sauron is dead and Im not read to fight
     * the final battle
     */
    if (borg_skill[BI_CDEPTH] == 99 && borg_race_death[546] == 1 &&
        borg_ready_morgoth !=1)
    {
        borg_note("# Returning to level 98 to scum for items.");
        g = -1;
    }

    /* Power dive if Morgoth is dead */
    if (borg_skill[BI_KING]) g = 1;

    /* Power dive to 99 if ready */
    if ((cptr)NULL == borg_prepared(99)) g = 1;

	/* Climb if deeper than I want to be */
	if (borg_skill[BI_CDEPTH] > borg_no_deeper)
    {
        borg_note(format("# Going up a bit (No Deeper than %d).", borg_no_deeper));
        g = -1;
    }

    /* Return to town to sell stuff -- No recall allowed.*/
    if (((borg_worships_gold || borg_skill[BI_MAXCLEVEL] < 15) &&
         borg_skill[BI_MAXCLEVEL] <= 25) && (k >= 12))
    {
        borg_note("# Going to town (Sell Stuff, Worshipping Gold).");
        g = -1;
    }

    /* Return to town to sell stuff (use Recall) */
    if ((bored && borg_skill[BI_MAXCLEVEL] >= 26) && (k >= 12))
    {
        borg_note("# Going to town (Sell Stuff).");
        goal_rising = TRUE;
    }

    /* Return to town when level drained */
    if (borg_skill[BI_ISFIXLEV])
    {
        borg_note("# Going to town (Fix Level).");
        goal_rising = TRUE;
    }

    /* Return to town to restore experience */
    if (bored && borg_skill[BI_ISFIXEXP] && borg_skill[BI_CLEVEL] !=50)
    {
        borg_note("# Going to town (Fix Experience).");
        goal_rising = TRUE;
    }

    /* return to town if it has been a while */
    if ((!goal_rising && bored && !vault_on_level && !borg_fighting_unique &&
         borg_time_town + borg_t - borg_began > 8000) ||
        (borg_time_town + borg_t - borg_began > 12000))
    {
        borg_note("# Going to town (I miss my home).");
        goal_rising = TRUE;
    }

    /* return to town if been scumming for a bit */
    if (borg_skill[BI_MAXDEPTH] >= borg_skill[BI_CDEPTH] + 10 &&
        borg_skill[BI_CDEPTH] <= 12 &&
        borg_time_town + borg_t - borg_began > 3500)
    {
        borg_note("# Going to town (scumming check).");
        goal_rising = TRUE;
    }

	/* Low level dudes need to visit town more frequently to keep up on food */
	if (borg_skill[BI_CLEVEL] < 15)
	{
		if (borg_time_town + (borg_t - borg_began) > (borg_skill[BI_CLEVEL] * 250) ||
			borg_time_town + (borg_t - borg_began) > 2500 ||
			(borg_time_town + (borg_t - borg_began) > 2000 && borg_skill[BI_REG]))
		{
	        borg_note("# Going to town (short trips).");
	        g = -1;
	    }
	}

    /* Return to town to drop off some scumming stuff */
    if (borg_scumming_pots && !vault_on_level &&
		(borg_skill[BI_AEZHEAL] >= 3 ||
		 borg_skill[BI_ALIFE] >= 1))
    {
        borg_note("# Going to town (Dropping off Potions).");
        goal_rising = TRUE;
    }

	/* if returning to town, try to go upstairs */
    if (goal_rising) g = -1;

    /* Mega-Hack -- spend time on the first level to rotate shops */
    if (borg_skill[BI_CLEVEL] > 10 &&
        (borg_skill[BI_CDEPTH] == 1) &&
        (borg_t - borg_began < 100) &&
        (g < 0) &&
        (borg_skill[BI_FOOD] > 1))
    {
        borg_note("# Staying on level to rotate shops.");
        g = 0;
    }

    /* Use random stairs when really bored */
    if (bored && (borg_t - borg_began >= 5000))
    {
        /* Note */
        borg_note("# Choosing random stairs.");

        /* Use random stairs */
        g = ((randint0(100) < 50) ? -1 : 1);
    }

    /* Go Up */
    if (g < 0)
    {
        /* Take next stairs */
		borg_note("# Looking for up stairs.  Going up.");
        stair_less = TRUE;

        /* Hack -- recall if going to town */
        if (goal_rising &&
            ((borg_time_town + (borg_t - borg_began)) > 200) &&
            (borg_skill[BI_CDEPTH] >= 5) &&
            borg_recall())
        {
            borg_note("# Recalling to town (goal rising)");
            return (TRUE);
        }

		/* Hack -- Recall if needing to Restock */
		if (need_restock &&
		    borg_skill[BI_CDEPTH] >= 5 &&
			borg_recall())
		{
			borg_note("# Recalling to town (need to restock)");
		}

        /* Attempt to use stairs */
        if (borg_flow_stair_less(GOAL_BORE, FALSE))
        {
                borg_note("# Looking for stairs. I'm bored.");
				return (TRUE);
		}

        /* Cannot find any stairs */
        if (goal_rising && bored && (borg_t - borg_began) >= 1000)
        {
            if (borg_recall())
            {
                borg_note("# Recalling to town (no stairs)");
                return (TRUE);
            }
        }

		/* No up stairs found. do down then back up */
		if (track_less_num == 0) g = 1;
    }


    /* Go Down */
    if (g > 0)
    {
        /* Take next stairs */
        stair_more = TRUE;

        /* Attempt to use those stairs */
        if (borg_flow_stair_more(GOAL_BORE, FALSE, FALSE)) return (TRUE);
    }


    /* Failure */
    return (FALSE);
}





/*
 * Initialize this file
 */
void borg_init_7(void)
{
    /* Nothing */
}



#ifdef MACINTOSH
static int HACK = 0;
#endif
