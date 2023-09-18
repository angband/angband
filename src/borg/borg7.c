/* File: borg7.c */
/* Purpose: High level functions for the Borg -BEN- */

#include "../angband.h"

#ifdef ALLOW_BORG

#include "../cave.h"
#include "../effect-handler.h"
#include "../obj-ignore.h"

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
static int borg_stuff_feeling[] =
{
    50000, /* 0 is no feeling yet given, stick around to get one */
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


/**
 * Work out if it's worth using ID on an item.  Also used in other places
 * as a general litmus test for whether an item is worth keeping hold of
 * while it's not ID'd.
 */
bool borg_item_worth_id(const borg_item *item)
{
	/* Never ID average stuff */
    if (!borg_item_note_needs_id(item))
		return false;

	/** Some stuff should always be ID'd... **/
	switch (item->tval)
	{
		case TV_BOW: case TV_SHOT: case TV_ARROW: case TV_BOLT:
		case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
		case TV_BOOTS: case TV_GLOVES: case TV_HELM:
		case TV_CROWN: case TV_SHIELD: case TV_CLOAK:
		case TV_SOFT_ARMOR: case TV_HARD_ARMOR: case TV_DRAG_ARMOR:

			/* Don't bother IDing unidentified items until they are pseudo'd */
			if (!item->ident) return false;
	}

	/* Not worth IDing magical items if we have better ego/artifact stuff */
    if (borg_item_note_needs_id(item))
	{
		int slot;
		borg_item *inven_item;
	
		/* Obtain the slot of the suspect item */
		slot = borg_wield_slot(item);
		if (slot < 0) return false;
	
		/* Obtain my equipped item in the slot */
		inven_item = &borg_items[slot];	
		if (inven_item->ego_idx || inven_item->art_idx) return false;
	}

	return true;
}


/*
 * Use things in a useful, but non-essential, manner
 */
bool borg_use_things(void)
{
    int i;

    /* Quaff experience restoration potion */
    if (borg_skill[BI_ISFIXEXP] &&
       (borg_spell(REVITALIZE) ||
		borg_spell(REMEMBRANCE) ||
		(borg_skill[BI_CURHP] > 90 && borg_spell(UNHOLY_REPRIEVE)) ||
        borg_activate_item(act_restore_exp) ||
        borg_activate_item(act_restore_st_lev) ||
        borg_activate_item(act_restore_life) ||
        borg_quaff_potion(sv_potion_restore_life)))
    {
        return (true);
    }

    /* just drink the stat gains, at this dlevel we wont need cash */
    if ( borg_quaff_potion(sv_potion_inc_str) ||
         borg_quaff_potion(sv_potion_inc_int) ||
         borg_quaff_potion(sv_potion_inc_wis) ||
         borg_quaff_potion(sv_potion_inc_dex) ||
         borg_quaff_potion(sv_potion_inc_con))
    {
        return (true);
    }

    /* Quaff potions of "restore" stat if needed */
    if ( (borg_skill[BI_ISFIXSTR] &&
         (borg_quaff_potion(sv_potion_inc_str) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_purging)||
          borg_activate_item(act_shroom_purging) ||
          borg_activate_item(act_restore_str) ||
          borg_activate_item(act_restore_all) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_restoring))) ||
        (borg_skill[BI_ISFIXINT] &&
         (borg_quaff_potion(sv_potion_inc_int) ||
          borg_activate_item(act_restore_int) ||
          borg_activate_item(act_restore_all) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_restoring))) ||
        (borg_skill[BI_ISFIXWIS] &&
         (borg_quaff_potion(sv_potion_inc_wis) ||
          borg_activate_item(act_restore_wis) ||
          borg_activate_item(act_restore_all) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_restoring))) ||
        (borg_skill[BI_ISFIXDEX] &&
         (borg_quaff_potion(sv_potion_inc_dex) ||
           borg_activate_item(act_restore_dex) ||
         borg_activate_item(act_restore_all) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_restoring))) ||
        (borg_skill[BI_ISFIXCON] &&
         (borg_quaff_potion(sv_potion_inc_con) ||
          borg_activate_item(act_restore_con) ||
          borg_activate_item(act_restore_all) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_purging) ||
          borg_activate_item(act_shroom_purging) ||
          borg_eat_food(TV_MUSHROOM, sv_mush_restoring))))
    {
        return (true);
    }


    /* Use some items right away */
    for (i = 0; i < z_info->pack_size; i++)
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
				if (item->sval == sv_potion_enlightenment)
				{
					/* Never quaff these in town */
					if (!borg_skill[BI_CDEPTH]) break;
				}
				else if (item->sval == sv_potion_inc_all)
					/* Try quaffing the potion */
					if (borg_quaff_potion(item->sval)) return (true);

                break;
            }
            case TV_SCROLL:
            {
                /* Hack -- check Blind/Confused */
                if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) break;

                /* XXX XXX XXX Dark */

                /* Check the scroll */
				if (item->sval == sv_scroll_mapping ||
					item->sval == sv_scroll_acquirement ||
					item->sval == sv_scroll_star_acquirement)
				{
					/* Never read these in town */
					if (!borg_skill[BI_CDEPTH]) break;

					/* Try reading the scroll */
					if (borg_read_scroll(item->sval)) return (true);
				}

                break;
            }
        }
    }

    /* Eat food */
    if (borg_skill[BI_ISHUNGRY])
    {
        /* Attempt to satisfy hunger */
        if (borg_spell(REMOVE_HUNGER) ||
            borg_spell(HERBAL_CURING) ||
            borg_quaff_potion(sv_potion_slime_mold) ||
            borg_eat_food(TV_FOOD, sv_food_slime_mold) ||
            borg_eat_food(TV_FOOD, sv_food_slice) ||
            borg_eat_food(TV_FOOD, sv_food_apple) ||
            borg_eat_food(TV_FOOD, sv_food_pint) ||            
            borg_eat_food(TV_FOOD, sv_food_handful) ||
            borg_eat_food(TV_FOOD, sv_food_honey_cake) ||
            borg_eat_food(TV_FOOD, sv_food_ration) ||
            borg_eat_food(TV_FOOD, sv_food_waybread) ||
            borg_eat_food(TV_FOOD, sv_food_draught) ||
            borg_activate_item(act_food_waybread))
        {
            return (true);
        }
    }


    /* Nothing to do */
    return (false);
}


/*
 * Check to see if the surrounding dungeon should be illuminated, and if
 * it should, do it.
 *
 * Always case light when we have no torch.
 *
 * This is when resting to heal.  I don't want him teleporting into a room,
 * resting to heal while there is a dragon sitting in a dark corner waiting
 * to breathe on him.
 *
 * Returns true if it did something, false otherwise
 */
bool borg_check_LIGHT_only(void)
{
	/* Never in town, when blind or when hallucinating */
	if (!borg_skill[BI_CDEPTH]) return (false);
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISIMAGE]) return (false);

	/** Use wizard light sometimes **/

	if (!when_wizard_LIGHT || (borg_t - when_wizard_LIGHT >= 1000)) {
		if (borg_activate_item(act_clairvoyance) ||
			borg_activate_item(act_enlightenment) ||
            borg_spell_fail(FUME_OF_MORDOR, 40) ||
			borg_spell_fail(CLAIRVOYANCE, 40)) {
			borg_note("# Wizard lighting the dungeon");
			/* borg_react("SELF:wizard lite", "SELF:wizard lite"); */
			when_wizard_LIGHT = borg_t;
			return true;
		}
	}

	/** Work out if there's any reason to light */

	/* Don't bother because we only just did it */
	if (when_call_LIGHT != 0 && (borg_t - when_call_LIGHT) < 7)
		return false;

	if (borg_skill[BI_CURLITE] == 1) {
		int i;
		int corners = 0;

		/* Scan diagonal neighbors.
		 *
		 * 4 corners   3 corners    2 corners    1 corner    0 corners
		 * ###         ##.  #..     ##.  #..     .#.         .#.  ... .#.
		 * .@.         .@.  .@.     .@.  .@.     .@.         #@#  .@. .@.
		 * ###         ###  ###     ##.  #..     ##.         .#.  ... .#.
		 *
		 * There's actually no way to tell which are rooms and which are
		 * corridors from diagonals except 4 (always a corridor) and
		 * 0 (always a room).  This is why we only use it for radius 1 light.
		 */
		for (i = 4; i < 8; i++) {
			borg_grid *ag;
	
			/* Get location */
			int x = c_x + ddx_ddd[i];
			int y = c_y + ddy_ddd[i];
			
			/* Bounds check */
			if (!square_in_bounds_fully(cave, loc(x, y))) continue;
			
			/* Get grid */
			ag = &borg_grids[y][x];
	
			/* Location must be known */
			if (ag->feat == FEAT_NONE) corners++;
			
			/* Location must be a wall/door */
			if (!borg_cave_floor_grid(ag)) corners++;
		}

		/* This is quite an arbitrary cutoff */
		if (corners > 2)
			return false;
	} else if (borg_skill[BI_CURLITE] > 1) {
		int x, y;
		int floors = 0;

		/*
		 * Scan the surrounding 5x5 area for unlit tiles.
		 *
		 * Radius two light misses out the four corners but otherwise illumates
		 * a 5x5 grid, which is 21 grids illumated incl player.
		 *
		 *  ...
		 * .....
		 * ..@..
		 * .....
		 *  ...
		 */
		for (y = c_y - 2; y <= c_y + 2; y++) {
			for (x = c_x - 2; x <= c_x + 2; x++) {
				borg_grid *ag;

				/* Bounds check */
				if (!square_in_bounds_fully(cave, loc(x, y))) continue;
				
				/* Get grid */
				ag = &borg_grids[y][x];

				/* Must be a floor grid lit by torchlight, not by magic */
				if (borg_cave_floor_grid(ag) &&
						(ag->info & BORG_LIGHT) &&
						!square_isglow(cave, loc(x,y)))
				{
					floors++;
				}
			}
		}

		/* Don't bother unless there are enough unlit floors */
		/* 11 is the empirical cutoff point for sensible behaviour here */
		if (floors < 11) return false;
	}

	/* Light it up! */
	if (borg_activate_item(act_illumination) ||
		borg_activate_item(act_light) ||
		borg_zap_rod(sv_rod_illumination) ||
		borg_use_staff(sv_staff_light) ||
		borg_read_scroll(sv_scroll_light) ||
		borg_spell_fail(LIGHT_ROOM, 40) ||
		borg_spell_fail(CALL_LIGHT, 40)) 
{
		borg_note("# Illuminating the dungeon");
		borg_react("SELF:lite", "SELF:lite");
		when_call_LIGHT = borg_t;
		return true;
	}

	return false;
}


/*
 * Refuel, call lite, detect traps/doors/walls/evil, etc
 *
 * Note that we refuel whenever our lite starts to get low.
 *
 * Note that we detect traps/doors/walls/evil at least once in each
 * panel, as soon as possible after entering a new panel.
 *
 * We use the special "SELF" messages to "borg_react()" to delay the
 * processing of DETECTION and "call lite" until we know if it has
 * worked or not.
 */
bool borg_check_LIGHT(void)
{
    int q_x, q_y;

    bool do_trap;
    bool do_door;
    bool do_wall;
    bool do_evil;
    bool do_obj;

    /* Never in town when mature (scary guy)*/
    if (borg_skill[BI_MAXCLEVEL] > 10 && !borg_skill[BI_CDEPTH]) return (false);

    /* Never when comprimised, save your mana */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE] || borg_skill[BI_ISPOISONED] ||
        borg_skill[BI_ISCUT] || borg_skill[BI_ISWEAK]) return (false);

    /* XXX XXX XXX Dark */


    /* Extract the panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();


    /* Start */
    do_trap = false;

    /* Determine if we need to detect traps */
    if (!borg_detect_trap[q_y+0][q_x+0]) do_trap = true;
    if (!borg_detect_trap[q_y+0][q_x+1]) do_trap = true;
    if (!borg_detect_trap[q_y+1][q_x+0]) do_trap = true;
    if (!borg_detect_trap[q_y+1][q_x+1]) do_trap = true;

    /* Hack -- check traps every few turns anyway */
    /* if (!when_detect_traps || (borg_t - when_detect_traps >= 183)) do_trap = true; */


    /* Start */
    do_door = false;

    /* Determine if we need to detect doors */
    if (!borg_detect_door[q_y+0][q_x+0]) do_door = true;
    if (!borg_detect_door[q_y+0][q_x+1]) do_door = true;
    if (!borg_detect_door[q_y+1][q_x+0]) do_door = true;
    if (!borg_detect_door[q_y+1][q_x+1]) do_door = true;

    /* Hack -- check doors every few turns anyway */
    /* if (!when_detect_doors || (borg_t - when_detect_doors >= 731)) do_door = true; */

    /* Start */
    do_wall = false;

    /* Determine if we need to detect walls */
    if (!borg_detect_wall[q_y+0][q_x+0]) do_wall = true;
    if (!borg_detect_wall[q_y+0][q_x+1]) do_wall = true;
    if (!borg_detect_wall[q_y+1][q_x+0]) do_wall = true;
    if (!borg_detect_wall[q_y+1][q_x+1]) do_wall = true;

    /* Hack -- check walls every few turns anyway */
    /* if (!when_detect_walls || (borg_t - when_detect_walls >= 937)) do_wall = true; */


    /* Start */
    do_evil = false;

    /* Determine if we need to detect evil */
    if (!borg_detect_evil[q_y+0][q_x+0]) do_evil = true;
    if (!borg_detect_evil[q_y+0][q_x+1]) do_evil = true;
    if (!borg_detect_evil[q_y+1][q_x+0]) do_evil = true;
    if (!borg_detect_evil[q_y+1][q_x+1]) do_evil = true;

    /* Start */
    do_obj = false;

    /* Determine if we need to detect evil */
    if (!borg_detect_obj[q_y + 0][q_x + 0]) do_obj = true;
    if (!borg_detect_obj[q_y + 0][q_x + 1]) do_obj = true;
    if (!borg_detect_obj[q_y + 1][q_x + 0]) do_obj = true;
    if (!borg_detect_obj[q_y + 1][q_x + 1]) do_obj = true;

    /* Hack -- check evil every few turns anyway- more fq if low level */
    /* if (!when_detect_evil ||
       (borg_t - when_detect_evil  >= 183 - (80 - borg_skill[BI_MAXCLEVEL]))) do_evil = true; */

	/* Really low level */
    /* if (borg_skill[BI_CLEVEL] <= 3 &&
    	(!when_detect_evil ||
        (borg_t - when_detect_evil  >= 50))) do_evil = true; */

	/* Not too frequent in town */
    /* if (borg_skill[BI_CDEPTH] == 0 &&
    	(!when_detect_evil ||
        (borg_t - when_detect_evil  >= 250))) do_evil = true; */

    /* Dont bother if I have ESP */
    if (borg_skill[BI_ESP]) do_evil = false;

    /* Only look for monsters in town, not walls, etc (scary guy)*/
    if (!borg_skill[BI_CDEPTH])
	{
		do_trap = false;
		do_door = false;
		do_wall = false;
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
        if (borg_activate_item(act_detect_all) ||
			borg_activate_item(act_mapping) ||
			borg_zap_rod(sv_rod_detection) ||
            borg_spell_fail(SENSE_SURROUNDINGS, 40))
        {
            borg_note("# Checking for traps, doors, and evil.");

            borg_react("SELF:TDE", "SELF:TDE");

            when_detect_traps = borg_t;
            when_detect_doors = borg_t;
            when_detect_evil =  borg_t;
            when_detect_obj = borg_t;

            return (true);
        }
    }

    /* Hack -- find evil */
    if (do_evil &&
        (!when_detect_evil || (borg_t - when_detect_evil >= 20)))
    {
        /* Check for evil */
		if (borg_spell_fail(DETECT_EVIL, 40) ||
			borg_spell_fail(DETECT_MONSTERS, 40) ||
			borg_spell_fail(READ_MINDS, 40) ||
			borg_spell_fail(SEEK_BATTLE, 40))
		{
            borg_note("# Checking for monsters.");

            borg_react("SELF:evil", "SELF:evil");

            when_detect_evil = borg_t;

            return (true);
        }
    }

    /* Hack -- find traps and doors (and stairs) */
    if ((do_trap || do_door) &&
        ((!when_detect_traps || (borg_t - when_detect_traps >= 5)) ||
         (!when_detect_doors || (borg_t - when_detect_doors >= 5))) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for traps and doors */
        if (borg_activate_item(act_detect_all) ||
			borg_activate_item(act_mapping) ||
			borg_spell_fail(DETECTION, 40) ||
			borg_spell_fail(FIND_TRAPS_DOORS_STAIRS, 40) ||
			borg_spell_fail(DETECT_STAIRS, 40))
        {
            borg_note("# Checking for traps, doors & stairs.");

            borg_react("SELF:both", "SELF:both");

            when_detect_traps = borg_t;
            when_detect_doors = borg_t;

            return (true);
        }
    }


    /* Hack -- find traps */
    if (do_trap &&
        (!when_detect_traps || (borg_t - when_detect_traps >= 7)) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for traps */
        if (borg_spell_fail(DETECTION, 40) ||
			borg_spell_fail(FIND_TRAPS_DOORS_STAIRS, 40))
        {
            borg_note("# Checking for traps.");

            borg_react("SELF:trap", "SELF:trap");

            when_detect_traps = borg_t;

            return (true);
        }
    }


    /* Hack -- find doors */
    if (do_door &&
        (!when_detect_doors || (borg_t - when_detect_doors >= 9)) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for traps */
        if (borg_activate_item(act_detect_all) ||
			borg_activate_item(act_mapping) ||
			borg_spell_fail(DETECTION, 40) ||
			borg_spell_fail(FIND_TRAPS_DOORS_STAIRS, 40))
        {
            borg_note("# Checking for doors.");

            borg_react("SELF:door", "SELF:door");

            when_detect_doors = borg_t;

            return (true);
        }
    }


    /* Hack -- find walls */
    if (do_wall &&
        (!when_detect_walls || (borg_t - when_detect_walls >= 15)) &&
         borg_skill[BI_CDEPTH]) 	/* Never in town */
    {
        /* Check for walls */
        if (borg_activate_item(act_mapping) ||
            borg_read_scroll(sv_scroll_mapping) ||
            borg_use_staff(sv_staff_mapping) ||
            borg_zap_rod(sv_rod_mapping) ||
			borg_spell(SENSE_SURROUNDINGS))
        {
            borg_note("# Checking for walls.");

            borg_react("SELF:wall", "SELF:wall");

            when_detect_walls = borg_t;

            return (true);
        }
    }

    /* Hack -- find objects */
    if (do_obj &&
        (!when_detect_obj || (borg_t - when_detect_obj >= 20)))
    {
        /* Check for objects */
        if (borg_activate_item(act_detect_objects) ||
            borg_spell_fail(OBJECT_DETECTION, 40))
        {
            borg_note("# Checking for objects.");

            borg_react("SELF:obj", "SELF:obj");

            when_detect_obj = borg_t;

            return (true);
        }
    }

	/* Do the actual illumination bit */
	return borg_check_LIGHT_only();
}

static void borg_pick_armor(int i)
{
    /* Choose from equipment */
    if (i >= INVEN_WIELD)
    {
        /* if there is armor in inventory, you have to press */
        /* '/' to get to equipment */
        bool found = false;
        for (int e = 0; e < z_info->pack_size; e++)
        {
            if (borg_items[e].iqty && 
                (borg_items[e].tval == TV_BOOTS ||
                 borg_items[e].tval == TV_GLOVES ||
                 borg_items[e].tval == TV_CLOAK ||
                 borg_items[e].tval == TV_CROWN ||
                 borg_items[e].tval == TV_HELM ||
                 borg_items[e].tval == TV_SHIELD ||
                 borg_items[e].tval == TV_HELM ||
                 borg_items[e].tval == TV_SOFT_ARMOR ||
                 borg_items[e].tval == TV_HARD_ARMOR ||
                 borg_items[e].tval == TV_DRAG_ARMOR))
            {
                found = true;
                break;
            }
        }
        if (found)
            borg_keypress('/');

        /* Choose that item */
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
    }
    else
        /* Choose that item */
        borg_keypress(all_letters_nohjkl[i]);
}

static void borg_pick_weapon(int i)
{
    /* Choose from equipment */
    if (i < INVEN_WIELD)
    {
        borg_keypress(all_letters_nohjkl[i]);
    }
    else
    {
        /* if there is a weapon in inventory, you have to press */
        /* '/' to get to equipment or '|' to go to quiver */
        bool found = false;
        for (int e = 0; e < z_info->pack_size; e++)
        {
            if (borg_items[e].iqty &&
                (borg_items[e].tval == TV_BOW ||
                    borg_items[e].tval == TV_DIGGING ||
                    borg_items[e].tval == TV_HAFTED ||
                    borg_items[e].tval == TV_POLEARM ||
                    borg_items[e].tval == TV_SWORD))
            {
                found = true;
                break;
            }
        }

        if (i < QUIVER_START)
        {
            if (found)
                borg_keypress('/');

            borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
        }
        else
        {
            /* Quiver Slot */
            if (found || borg_items[INVEN_WIELD].iqty != 0 || borg_items[INVEN_BOW].iqty != 0)
                borg_keypress('|');
            borg_keypress('0' + (i - QUIVER_START));
        }
    }
}

/*
 * Enchant armor, not including my swap armour
 */
static bool borg_enchant_to_a(void)
{
    int i, b_i = -1;
    int a, b_a = 99;

    /* Nothing to enchant */
    if (!my_need_enchant_to_a) return (false);

    /* Need "enchantment" ability */
    if ((!amt_enchant_to_a) &&
        (!amt_enchant_armor)) return (false);


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
        if (borg_spell_okay_fail(ENCHANT_ARMOUR, 65) ||
           amt_enchant_armor >=1)
        {
            if (a >= borg_cfg[BORG_ENCHANT_LIMIT]) continue;
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
    if (b_i < 0) return (false);

    /* Enchant it */
    if (borg_read_scroll(sv_scroll_star_enchant_armor) ||
        borg_read_scroll(sv_scroll_enchant_armor) ||
        borg_spell_fail(ENCHANT_ARMOUR, 65))
    {
        borg_pick_armor(b_i);

        /* Success */
        return (true);
    }

    /* Nothing to do */
    return (false);
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
        !enchant_weapon_swap_to_h) return (false);

    /* Need "enchantment" ability */
    if ( (!amt_enchant_to_h) &&
         (!amt_enchant_weapon) ) return (false);


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
        if (borg_spell_okay_fail(ENCHANT_WEAPON, 65) ||
            amt_enchant_weapon >= 1 )
        {
            if (a >= borg_cfg[BORG_ENCHANT_LIMIT]) continue;
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
			borg_skill[BI_AMMO_POWER] < 3 && /* 3x shooter */
			(!item->art_idx && !item->ego_idx)) /* Not artifact or ego */
			continue;

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a)) continue;

        /* Save the info */
        b_i = i; b_a = a;

    }
    if (weapon_swap)
    {
        bool skip = false;
        borg_item* item = &borg_items[weapon_swap - 1];

        /* Skip my swap digger and anything unid'd */
        if (item->ident && item->tval != TV_DIGGING)
        {
            /* Obtain the bonus */
            s_a = item->to_h;

            /* Skip items that are already enchanted */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65) ||
                amt_enchant_weapon >= 1)
            {
                if (s_a >= borg_cfg[BORG_ENCHANT_LIMIT]) skip = true;
            }
            else
            {
                if (s_a >= 8) skip = true;
            }

            /* Find the least enchanted item */
            if (b_a > s_a && !skip)
            {
                /* Save the info */
                b_i = weapon_swap - 1; b_a = s_a;
            }
        }
    }
    /* Nothing, check ammo */
    if (b_i < 0)
    {
        /* look through inventory for ammo */
        for (i = QUIVER_START; i < QUIVER_END; i++)
        {
            borg_item *item = &borg_items[i];

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (borg_skill[BI_AMMO_POWER] < 3 || (!borg_items[INVEN_BOW].art_idx && !borg_items[INVEN_BOW].ego_idx)) continue;

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5) continue;

            /* Skip non-identified items  */
            if (!item->ident) continue;

            /* Make sure it is the right type if missile */
            if (item->tval != borg_skill[BI_AMMO_TVAL] ) continue;

            /* Obtain the bonus  */
            a = item->to_h;

            /* Skip items that are already enchanted */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65) ||
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
    if (b_i < 0) return (false);

    /* Enchant it */
    if (borg_read_scroll(sv_scroll_star_enchant_weapon) ||
        borg_read_scroll(sv_scroll_enchant_weapon_to_hit) ||
        borg_spell_fail(ENCHANT_WEAPON, 65))
    {
        borg_pick_weapon(b_i);

        /* Success */
        return (true);
    }

    /* Nothing to do */
    return (false);
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
        !enchant_weapon_swap_to_d) return (false);

    /* Need "enchantment" ability */
    if ( (!amt_enchant_to_d) &&
         (!amt_enchant_weapon) ) return (false);


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
        if (borg_spell_okay_fail(ENCHANT_WEAPON, 65) ||
            amt_enchant_weapon >= 1 )
        {
            if (a >= borg_cfg[BORG_ENCHANT_LIMIT]) continue;
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
			borg_skill[BI_AMMO_POWER] < 3 && /* 3x shooter */
			(!item->art_idx && !item->ego_idx)) /* Not artifact or ego */
			continue;

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a)) continue;

        /* Save the info */
        b_i = i; b_a = a;
    }
    if (weapon_swap)
    {
        bool skip = false;
        borg_item *item = &borg_items[weapon_swap-1];

        /* Skip non-identified items and diggers */
        if (item->ident && item->tval != TV_DIGGING)
        {
            /* Obtain the bonus */
            s_a = item->to_d;

            /* Skip "boring" items */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65) ||
                amt_enchant_weapon >= 1)
            {
                if (s_a >= borg_cfg[BORG_ENCHANT_LIMIT]) skip = true;
            }
            else
            {
                if (s_a >= 8) skip = true;
            }

            /* Find the least enchanted item */
            if (b_a > s_a && !skip)
            {
                /* Save the info */
                b_i = weapon_swap - 1; b_a = s_a;
            }
        }
    }
    /* Nothing, check ammo */
    if (b_i < 0)
    {
        /* look through inventory for ammo */
        for (i = QUIVER_START; i < QUIVER_END; i++)
        {
            borg_item *item = &borg_items[i];

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (borg_skill[BI_AMMO_POWER] < 3 || (!borg_items[INVEN_BOW].art_idx && !borg_items[INVEN_BOW].ego_idx)) continue;

			/* Only enchant ammo if we have a good shooter,
			 * otherwise, store the enchants in the home.
			 */
			if (borg_skill[BI_AMMO_POWER] < 3) continue;

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5) continue;

            /* Skip non-identified items  */
            if (!item->ident) continue;

            /* Make sure it is the right type if missile */
            if (item->tval != borg_skill[BI_AMMO_TVAL] ) continue;

            /* Obtain the bonus  */
            a = item->to_d;

            /* Skip items that are already enchanted */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65) ||
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
    if (b_i < 0) return (false);

    /* Enchant it */
    if (borg_read_scroll(sv_scroll_star_enchant_weapon) ||
        borg_read_scroll(sv_scroll_enchant_weapon_to_dam) ||
        borg_spell_fail(ENCHANT_WEAPON, 65))
    {
        borg_pick_weapon(b_i);

        /* Success */
        return (true);
    }

    /* Nothing to do */
    return (false);
}

/*
 * Brand Bolts
 */
static bool borg_brand_weapon(void)
{
    int i, b_i = -1;
    int a, b_a = 0;

    /* Nothing to brand */
    if (!my_need_brand_weapon) return (false);

    /* Need "brand" ability */
    if (!amt_brand_weapon) return (false);

    /* look through inventory for ammo */
        for (i = QUIVER_START; i < QUIVER_END; i++)
        {
            borg_item *item = &borg_items[i];

            /* Only enchant if qty >= 20 */
            if (item->iqty < 20) continue;

            /* Skip non-identified items  */
            if (!item->ident) continue;

            /* Make sure it is the right type if missile */
            if (item->tval != borg_skill[BI_AMMO_TVAL] ) continue;

            /* Obtain the bonus  */
            a = item->to_h;

            /* Skip branded items */
            if (item->ego_idx) continue;

            /* Find the most enchanted item */
            if ((b_i >= 0) && (b_a > a)) continue;

            /* Save the info  */
            b_i = i; b_a = a;

    }

	/* Nothing to Brand */
	if (b_i == -1) return (false);

    /* Enchant it */
    if (borg_activate_item(act_firebrand) ||
        borg_spell_fail(BRAND_AMMUNITION, 65))
    {
        borg_pick_weapon(b_i);

        /* Success */
        return (true);
    }

    /* Nothing to do */
    return (false);
}

/*
 * Remove Curse swap armour
 */
static bool borg_decurse_armour(void)
{
    /* Nothing to decurse */
    if (borg_cfg[BORG_USES_SWAPS] && !decurse_armour_swap) return (false);

    if (-1 == borg_slot(TV_SCROLL, sv_scroll_remove_curse) &&
        !borg_equips_staff_fail(sv_staff_remove_curse) &&
        !borg_spell_okay_fail(REMOVE_CURSE, 40) &&
        -1 == borg_slot(TV_SCROLL, sv_scroll_star_remove_curse) &&
        !borg_equips_item(act_remove_curse, true) && 
        !borg_equips_item(act_remove_curse2, true))
    {
        return (false);
    }

    /* remove the curse */
    if (borg_read_scroll(sv_scroll_remove_curse) ||
        borg_use_staff(sv_staff_remove_curse) ||
        borg_spell(REMOVE_CURSE) ||
        borg_read_scroll(sv_scroll_star_remove_curse) || 
        borg_activate_item(act_remove_curse) || 
        borg_activate_item(act_remove_curse2))
    {
        /* pick the item */
        borg_keypress(all_letters_nohjkl[armour_swap-1]);
        /* pick first curse */
        borg_keypress(KC_ENTER);

        /* Shekockazol! */
        return (true);
    }

    /* Nothing to do */
    return (false);
}

/*
 * Remove Curse swap weapon
 *
 */
static bool borg_decurse_weapon(void)
{
    /* Nothing to decurse */
    if (borg_cfg[BORG_USES_SWAPS] && !decurse_weapon_swap) return (false);

    /* Ability for curses */
    if (borg_cfg[BORG_USES_SWAPS] && decurse_weapon_swap)
    {
        if (-1 == borg_slot(TV_SCROLL,sv_scroll_remove_curse) &&
            !borg_equips_staff_fail(sv_staff_remove_curse) &&
            !borg_spell_okay_fail(REMOVE_CURSE,40) &&
            - 1 == borg_slot(TV_SCROLL, sv_scroll_star_remove_curse) &&
            !borg_equips_item(act_remove_curse, true) &&
            !borg_equips_item(act_remove_curse2, true))
        {
            return (false);
        }

		/* remove the curse */
		if (borg_read_scroll(sv_scroll_remove_curse) ||
			borg_use_staff(sv_staff_remove_curse)||
			borg_spell(REMOVE_CURSE) ||
            borg_read_scroll(sv_scroll_star_remove_curse) ||
            borg_activate_item(act_remove_curse) ||
            borg_activate_item(act_remove_curse2))
		{
            borg_keypress(all_letters_nohjkl[weapon_swap-1]);
            /* pick first curse */
            borg_keypress(KC_ENTER);

            /* Shekockazol! */
			return (true);
		}
    }

    /* Nothing to do */
    return (false);
}
/*
 * Remove Curse any
 */
static bool borg_decurse_any(void)
{
    if (borg_skill[BI_FIRST_CURSED])
    {
        if (-1 == borg_slot(TV_SCROLL, sv_scroll_remove_curse) &&
            !borg_equips_staff_fail(sv_staff_remove_curse) &&
            !borg_spell_okay_fail(REMOVE_CURSE, 40) &&
            -1 == borg_slot(TV_SCROLL, sv_scroll_star_remove_curse) &&
            !borg_equips_item(act_remove_curse, true) &&
            !borg_equips_item(act_remove_curse2, true))
        {
            return (false);
        }

        /* remove the curse */
        if (borg_read_scroll(sv_scroll_remove_curse) ||
            borg_use_staff(sv_staff_remove_curse) ||
            borg_spell(REMOVE_CURSE) ||
            borg_read_scroll(sv_scroll_star_remove_curse) ||
            borg_activate_item(act_remove_curse) ||
            borg_activate_item(act_remove_curse2))
        {
            /* pick the item */
            if (borg_skill[BI_FIRST_CURSED] <= INVEN_WIELD)
            {
                borg_keypress(all_letters_nohjkl[borg_skill[BI_FIRST_CURSED] - 1]);
            }
            else if (borg_skill[BI_FIRST_CURSED] <= QUIVER_START)
            {
                if (borg_skill[BI_WHERE_CURSED] & BORG_INVEN)
                    borg_keypress('/');

                borg_keypress(all_letters_nohjkl[borg_skill[BI_FIRST_CURSED] - INVEN_WIELD - 1]);
            }
            else
            {
                if (borg_skill[BI_WHERE_CURSED] & 1 || borg_skill[BI_WHERE_CURSED] & BORG_EQUIP)
                    borg_keypress('|');
                borg_keypress('0' + (borg_skill[BI_FIRST_CURSED] - 1 - QUIVER_START));
            }
            /* pick first curse */
            borg_keypress(KC_ENTER);

            /* Shekockazol! */
            return (true);
        }
    }

    /* Nothing to do */
    return (false);
}
/*
 * Enchant things
 */
bool borg_enchanting(void)
{
    /* Forbid blind/confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (false);


    /* Forbid if been sitting on level forever */
    /*    Just come back and finish the job later */
    if ((borg_t - borg_began > 5500 && borg_skill[BI_CDEPTH] >= 1) ||
        (borg_t-borg_began > 3501 && borg_skill[BI_CDEPTH] == 0)) return (false);

    /* Remove Curses */
    if (borg_decurse_armour()) return (true);
    if (borg_decurse_weapon()) return (true);
    if (borg_decurse_any()) return (true);

	/* Only in town */
	if (borg_skill[BI_CDEPTH]) return (false);

    /* Enchant things */
    if (borg_brand_weapon()) return (true);
    if (borg_enchant_to_h()) return (true);
    if (borg_enchant_to_d()) return (true);
    if (borg_enchant_to_a()) return (true);

    /* Nope */
    return (false);
}


/*
 * Recharge things
 *
 * XXX XXX XXX Prioritize available items
 */
bool borg_recharging(void)
{
    int i = -1;
    bool charge = false;


    /* Forbid blind/confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (false);

    /* XXX XXX XXX Dark */

    /* Look for an item to recharge */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->ident) continue;

        if (item->note && strstr(item->note, "empty")) continue;

        /* assume we can't charge it. */
        charge = false;

        /* Wands with no charges can be charged */
        if ((item->tval == TV_WAND) && (item->pval <= 1))
            charge = true;

        /* recharge staves sometimes */
        if (item->tval == TV_STAFF)
        {
            /* Allow staves to be recharged at 2 charges if
             * the borg has the big recharge spell. And its not a *Dest*
             */
            if ((item->pval < 2) &&
                (borg_spell_okay_fail(RECHARGING, 96)) &&
                item->sval < sv_staff_power)
                charge = true;

            /* recharge any staff at 0 charges */
            if (item->pval <= 1)
                charge = true;

            /* Staves of teleport get recharged at 2 charges in town */
            if ((item->sval == sv_staff_teleportation) &&
                (item->pval < 3) &&
                !borg_skill[BI_CDEPTH])
                charge = true;

            /* They stack.  If quantity is 4 and pval is 1, then there are 4 charges. */
            if ((item->iqty + item->pval >= 4) && item->pval >= 1) charge = false;
        }

        /* get the next item if we are not to charge this one */
        if (!charge) continue;

        /* Attempt to recharge */
        if (borg_read_scroll(sv_scroll_recharging) ||
            borg_spell_fail(RECHARGING, 96) ||
            borg_activate_item(act_recharge))
        {
            /* Message */
            borg_note(format("Recharging %s with current charge of %d", item->desc, item->pval));

            /* Recharge the item */
            borg_keypress(all_letters_nohjkl[i]);

			/* Remove the {empty} if present */
			if (item->note && strstr(item->note, "empty")) borg_send_deinscribe(i);

			/* Success */
            return (true);
        }
        else
            /* if we fail once, no need to try again. */
            break;
    }

    /* Nope */
    return (false);
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
            if (item->sval == sv_potion_slime_mold ||
				item->sval ==  sv_potion_cure_light ||
				item->sval ==  sv_potion_cure_serious ||
				item->sval ==  sv_potion_cure_critical ||
				item->sval ==  sv_potion_healing ||
				item->sval ==  sv_potion_star_healing ||
				item->sval ==  sv_potion_life ||
				item->sval ==  sv_potion_restore_life ||
				item->sval ==  sv_potion_restore_mana ||
				item->sval ==  sv_potion_heroism ||
				item->sval ==  sv_potion_berserk ||
				item->sval ==  sv_potion_resist_heat ||
				item->sval ==  sv_potion_resist_cold ||
				item->sval ==  sv_potion_infravision ||
				item->sval ==  sv_potion_detect_invis ||
				item->sval ==  sv_potion_cure_poison ||
				item->sval ==  sv_potion_speed ||
				item->sval ==  sv_potion_inc_exp)
			{
			    /* Try quaffing the potion */
				if (borg_quaff_potion(item->sval)) return (true);
				break;
			}

			/* Gain one/lose one potions */
			if (item->sval == sv_potion_inc_str2)
			{
				/* Maxed out no need. Don't lose another stat */
				if (borg_skill[BI_CSTR] >= 118) return (false);

				/* This class does not want to risk losing a different stat */
				if (borg_class == CLASS_MAGE || 
					borg_class == CLASS_DRUID || 
					borg_class == CLASS_NECROMANCER) 
					return (false);

				/* Otherwise, it should be ok */
				if (borg_quaff_potion(item->sval)) return (true);
			}

			if (item->sval == sv_potion_inc_int2)
			{
				/* Maxed out no need. Don't lose another stat */
				if (borg_skill[BI_CINT] >= 118) return (false);

				/* This class does not want to risk losing a different stat */
				if (borg_class != CLASS_MAGE && borg_class != CLASS_NECROMANCER) return (false);

				/* Otherwise, it should be ok */
				if (borg_quaff_potion(item->sval)) return (true);
				break;
			}

			if (item->sval == sv_potion_inc_wis2)
			{
				/* Maxed out no need. Don't lose another stat */
				if (borg_skill[BI_CWIS] >= 118) return (false);

				/* This class does not want to risk losing a different stat */
				if (borg_class != CLASS_PRIEST && borg_class != CLASS_DRUID) return (false);

				/* Otherwise, it should be ok */
				if (borg_quaff_potion(item->sval)) return (true);
				break;
			}

			if (item->sval == sv_potion_inc_dex2)
			{
				/* Maxed out no need. Don't lose another stat */
				if (borg_skill[BI_CDEX] >= 118) return (false);

				/* This class does not want to risk losing a different stat */
				if (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST || borg_class == CLASS_DRUID ||
					borg_class == CLASS_NECROMANCER) return (false);

				/* Otherwise, it should be ok */
				if (borg_quaff_potion(item->sval)) return (true);
				break;
			}

			if (item->sval == sv_potion_inc_con2)
			{
				/* Maxed out no need. Don't lose another stat */
				if (borg_skill[BI_CCON] >= 118) return (false);

				/* Otherwise, it should be ok */
				if (borg_quaff_potion(item->sval)) return (true);
				break;
			}

        break;

        case TV_SCROLL:

        /* Check the scroll */
            if (item->sval == sv_scroll_light ||
				item->sval == sv_scroll_monster_confusion ||
				item->sval == sv_scroll_trap_door_destruction ||
				item->sval == sv_scroll_satisfy_hunger ||
				item->sval == sv_scroll_dispel_undead ||
				item->sval == sv_scroll_blessing ||
				item->sval == sv_scroll_holy_chant ||
				item->sval == sv_scroll_holy_prayer)
			{
				/* Try reading the scroll */
				if (borg_read_scroll(item->sval)) return (true);
			}

	        break;

        case TV_FOOD:
        /* Check the grub */
        if (item->sval == sv_food_ration ||
            item->sval == sv_food_slime_mold ||
            item->sval == sv_food_waybread)

            /* Try eating the food (unless Bloated) */
            if (!borg_skill[BI_ISFULL] && borg_eat_food(item->tval, item->sval)) return (true);

        break;

        case TV_MUSHROOM:

        /* Check the grub */
		if (item->sval == sv_mush_second_sight ||
			item->sval == sv_mush_fast_recovery ||
			item->sval == sv_mush_cure_mind ||
			item->sval == sv_mush_restoring ||
			item->sval == sv_mush_emergency ||
			item->sval == sv_mush_terror ||
			item->sval == sv_mush_stoneskin ||
			item->sval == sv_mush_debility ||
			item->sval == sv_mush_sprinting ||
			item->sval == sv_mush_purging)

            /* Try eating the food (unless Bloated) */
            if (!borg_skill[BI_ISFULL] && borg_eat_food(item->tval, item->sval)) return (true);

        break;
    }


    /* Nope */
    return (false);
}

/* HACK is it safe to crush an item here... must be on an empty floor square */
static bool borg_safe_crush(void)
{
    if (borg_grids[c_y][c_x].feat != FEAT_FLOOR) return (false);

    /* hack check for invisible traps */
    if (square_trap(cave, loc(c_x, c_y))) return (false);

    /* **HACK** don't drop on top of a previously ignored item */
    /* this is because if you drop something then ignore it then drop another */
    /* on top of it, the second item combines with the first and just disappears */
    struct object* obj = square_object(cave, loc(c_x, c_y));
    while (obj)
    {
        if (obj->known->notice & OBJ_NOTICE_IGNORE)
            return (false);
        if (obj->kind->ignore)
            return (false);
        obj = obj->next;
    }

    return (true);
}


/*
 * Destroy "junk" items
 */
bool borg_crush_junk(void)
{
    int i;
    bool fix = false;
    int32_t p;
    int32_t value;

	/* Hack -- no need */
    if (!borg_do_crush_junk) return (false);

    /* is it safe to crush junk here */
    if (!borg_safe_crush()) return (false);

    /* No crush if even slightly dangerous */
    if (borg_danger(c_y,c_x,1, true, false) > borg_skill[BI_CURHP] / 10) return (false);

    /* Destroy actual "junk" items */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* dont crush the swap */
        if (weapon_swap && i == weapon_swap-1) continue;
        if (armour_swap && i == armour_swap-1) continue;

        /* Dont crush weapons if we are weilding a digger */
#if 0
            if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;
#endif

        /* dont crush our spell books */
        if (obj_kind_can_browse(&k_info[item->kind])) continue;

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
			(item->ident || (item->note && strstr(item->note, "empty"))))
		{
			if (item->iqty > item->pval) value = 0L;
		}

        /* Skip non "worthless" items */
        if (item->tval >= TV_CHEST)
        {
            /* unknown and not worthless */
            if (!item->ident && value > 0)
                continue;

            /* skip items that are 'valuable'.  This is level dependent */
            /* try to make the borg junk +1,+1 dagger at level 40 */

			/* if the item gives a bonus to a stat, boost its value */
            if ((item->modifiers[OBJ_MOD_STR] > 0 ||
				item->modifiers[OBJ_MOD_INT] > 0 ||
				item->modifiers[OBJ_MOD_WIS] > 0 ||
				item->modifiers[OBJ_MOD_DEX] > 0 ||
				item->modifiers[OBJ_MOD_CON] > 0) && value > 0)
            {
                value += 2000L;
            }

            /* Keep some stuff */
            if ( (item->tval == borg_skill[BI_AMMO_TVAL]  && value > 0) ||
            ((item->tval == TV_POTION && item->sval == sv_potion_restore_mana) &&
             (borg_skill[BI_MAXSP] >= 1)) ||
            (item->tval == TV_POTION && item->sval == sv_potion_healing) ||
            (item->tval == TV_POTION && item->sval == sv_potion_star_healing) ||
            (item->tval == TV_POTION && item->sval == sv_potion_life) ||
            (item->tval == TV_POTION && item->sval == sv_potion_speed) ||
            (item->tval == TV_ROD && item->sval == sv_rod_drain_life)||
            (item->tval == TV_ROD && item->sval == sv_rod_healing)  ||
            (item->tval == TV_ROD && item->sval == sv_rod_mapping && borg_class == CLASS_WARRIOR) ||
            (item->tval == TV_STAFF && item->sval == sv_staff_dispel_evil) ||
            (item->tval == TV_STAFF && item->sval == sv_staff_power) ||
            (item->tval == TV_STAFF && item->sval == sv_staff_holiness) ||
            (item->tval == TV_WAND && item->sval == sv_wand_drain_life) ||
            (item->tval == TV_WAND && item->sval == sv_wand_annihilation) ||
            (item->tval == TV_WAND && item->sval == sv_wand_teleport_away && borg_class == CLASS_WARRIOR) ||
            (item->ego_idx && borg_ego_has_random_power(&e_info[item->ego_idx])) ||
            (item->tval == TV_SCROLL && item->sval == sv_scroll_teleport_level &&
             borg_skill[BI_ATELEPORTLVL] < 1000) ||
            (item->tval == TV_SCROLL && item->sval == sv_scroll_protection_from_evil))

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
	            if (item->tval != borg_skill[BI_AMMO_TVAL] ) value = 0L;
	        }

            /* borg_worships_gold will sell all kinds of stuff,
             * except {cursed} is junk
             */
            if (item->value > 0 &&
                ((borg_cfg[BORG_WORSHIPS_GOLD] || borg_skill[BI_MAXCLEVEL] < 10) ||
                 ((borg_cfg[BORG_MONEY_SCUM_AMOUNT] < borg_gold) && 
                     borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0)) &&
                borg_skill[BI_MAXCLEVEL] <= 20 &&
                !item->cursed) continue;


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
            memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

            /* Destroy the item */
            memset(&borg_items[i], 0, sizeof(borg_item));

            /* Fix later */
            fix = true;

            /* Examine the inventory */
            borg_notice(false);

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

            /* skip things we are using */
            if (p < my_power) continue;
        }

        /* re-examine the inventory */
        if (fix) borg_notice(true);

        /* Hack -- skip good un-id'd "artifacts" */
        if (borg_item_note_needs_id(item)) continue;

        /* hack --  with random artifacts some are good and bad */
        /*         so check them all */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident) continue;

        /* Message */
        borg_note(format("# Junking junk (valued at %d)",value));
        /* Message */
        borg_note(format("# Destroying %s.", item->desc));

        /* drop it then ignore it */
        borg_keypress('d');
        borg_keypress(all_letters_nohjkl[i]);

        if (item->iqty > 1)
        {
            borg_keypress('1');
            borg_keypress(KC_ENTER);
        }

        /* ignore it now */
        borg_keypress('k');
        borg_keypress('-');
        borg_keypress('a');
        borg_keypress('a');

        /* Success */
        return (true);
    }

    /* re-examine the inventory */
    if (fix) borg_notice(true);

    /* Hack -- no need */
    borg_do_crush_junk = false;

    /* Nothing to destroy */
    return (false);
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
    int32_t p, b_p = 0L;
    int32_t w, b_w = 0L;

    int32_t value;

    bool fix = false;


    /* Do not destroy items unless we need the space */
    if (!borg_items[PACK_SLOTS-1].iqty) return (false);

    /* No crush if even slightly dangerous */
    if (borg_skill[BI_CDEPTH] && (borg_danger(c_y,c_x,1, true, false) > borg_skill[BI_CURHP] / 10 &&
         (borg_skill[BI_CURHP] != borg_skill[BI_MAXHP] ||
         borg_danger(c_y,c_x,1, true, false) > (borg_skill[BI_CURHP] * 2) / 3)))
        return (false);

    /* must be a good place to crush stuff */
    if (!borg_safe_crush()) return (false);

    /* Scan the inventory */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Hack -- skip "artifacts" */
        if (item->art_idx) continue;

		/* skip food */
		if (item->tval == TV_FOOD && borg_skill[BI_FOOD] < 5) continue;

        /* dont crush the swap weapon */
        if (weapon_swap && i == weapon_swap -1) continue;
        if (armour_swap && i == armour_swap -1) continue;

        /* dont crush our spell books */
        if (obj_kind_can_browse(&k_info[item->kind])) continue;

		/* Do not crush Boots, they could be SPEED */
		if (item->tval == TV_BOOTS && !item->ident) continue;

        /* Dont crush weapons if we are weilding a digger */
        if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;

        /* Hack -- skip "artifacts" */
        if (item->art_idx && !item->ident) continue;
        if (borg_item_note_needs_id(item)) continue;

        /* never crush cool stuff that we might be needing later */
        if ((item->tval == TV_POTION && item->sval == sv_potion_restore_mana) &&
            (borg_skill[BI_MAXSP] >= 1)) continue;
        if (item->tval == TV_POTION && item->sval == sv_potion_healing) continue;
        if (item->tval == TV_POTION && item->sval == sv_potion_star_healing) continue;
        if (item->tval == TV_POTION && item->sval == sv_potion_life) continue;
        if (item->tval == TV_POTION && item->sval == sv_potion_speed) continue;
        if (item->tval == TV_SCROLL && item->sval == sv_scroll_protection_from_evil) continue;
        if (item->tval == TV_SCROLL && item->sval == sv_scroll_rune_of_protection) continue;
        if (item->tval == TV_SCROLL && item->sval == sv_scroll_teleport_level &&
            borg_skill[BI_ATELEPORTLVL] < 1000 ) continue;
        if (item->tval == TV_ROD && (item->sval == sv_rod_healing ||
            (item->sval == sv_rod_mapping && borg_class == CLASS_WARRIOR)) &&
            item->iqty <= 5) continue;
        if (item->tval == TV_WAND && item->sval == sv_wand_teleport_away &&
            borg_class == CLASS_WARRIOR && borg_skill[BI_ATPORTOTHER] <= 8) continue;
        if (item->tval == TV_ROD && (item->sval == sv_rod_light &&
            borg_skill[BI_CURLITE] <= 0)) continue;

        /* a boost for things with random powers */
        if (item->ego_idx && borg_ego_has_random_power(&e_info[item->ego_idx]) 
            && !item->ident)
            continue;

        /* save the items value */
        value = item->value;

        /* save the items wieght */
        w = item->weight * item->iqty;

        /* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Destroy the item */
        memset(&borg_items[i], 0, sizeof(borg_item));

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Penalize loss of "gold" */

        /* if the item gives a bonus to a stat, boost its value */
        if (item->modifiers[OBJ_MOD_STR] > 0 ||
			item->modifiers[OBJ_MOD_INT] > 0 ||
			item->modifiers[OBJ_MOD_WIS] > 0 ||
			item->modifiers[OBJ_MOD_DEX] > 0 ||
			item->modifiers[OBJ_MOD_CON] > 0)
        {
            value += 20000L;
        }

        /* Keep the correct types of missiles which have value
         * if we do have have tons already. unless in town, you can junk em in town.
         */
        if ((item->tval == borg_skill[BI_AMMO_TVAL] ) && (value > 0) &&
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
            value = (item->iqty * value * 10);
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
            value = (item->iqty * value * 5);
            break;

            case TV_CLOAK:
            if (item->ego_idx && borg_ego_has_random_power(&e_info[item->ego_idx]))
                value = (item->iqty * (300000L));
            else
                value = (item->iqty * value);
            break;

            case TV_ROD:
            /* BIG HACK! don't crush cool stuff. */
            if ((item->sval != sv_rod_drain_life) ||
                (item->sval != sv_rod_acid_ball) ||
                (item->sval != sv_rod_elec_ball) ||
                (item->sval != sv_rod_fire_ball) ||
                (item->sval != sv_rod_cold_ball))
                value = (item->iqty * (300000L));  /* value at 30k */
            else
                value = (item->iqty * value);
            break;

            case TV_STAFF:
            /* BIG HACK! don't crush cool stuff. */
            if (item->sval != sv_staff_dispel_evil ||
                ((item->sval != sv_staff_power ||
                    item->sval != sv_staff_holiness) &&
                    amt_cool_staff < 2) ||
                (item->sval != sv_staff_destruction &&
                    borg_skill[BI_ASTFDEST] < 2))
                value = (item->iqty * (300000L));  /* value at 30k */
            else
                value = (item->iqty * (value / 2));
            break;

            case TV_WAND:
            /* BIG HACK! don't crush cool stuff. */
            if ((item->sval != sv_wand_drain_life) ||
                (item->sval != sv_wand_teleport_away) ||
                (item->sval != sv_wand_acid_ball) ||
                (item->sval != sv_wand_elec_ball) ||
                (item->sval != sv_wand_fire_ball) ||
                (item->sval != sv_wand_cold_ball) ||
                (item->sval != sv_wand_annihilation) ||
                (item->sval != sv_wand_dragon_fire) ||
                (item->sval != sv_wand_dragon_cold))
                value = (item->iqty * (300000L));  /* value at 30k */
            else
                value = (item->iqty * (value / 2));
            break;

            /* scrolls and potions crush easy */
            case TV_SCROLL:
            if ((item->sval != sv_scroll_protection_from_evil) ||
                (item->sval != sv_scroll_rune_of_protection))
                value = (item->iqty * (30000L));
            else
                value = (item->iqty * (value / 10));
            break;

            case TV_POTION:
            /* BIG HACK! don't crush heal/mana potions.  It could be */
            /* that we are in town and are collecting them. */
            if ((item->sval != sv_potion_healing) ||
                (item->sval != sv_potion_star_healing) ||
                (item->sval != sv_potion_life) ||
                (item->sval != sv_potion_restore_mana))
                value = (item->iqty * (300000L));  /* value at 30k */
            else
                value = (item->iqty * (value / 10));
            break;

            default:
            value = (item->iqty * (value / 3));
            break;
            }
        }
        else
        {
            value = (item->iqty * value);
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
                value = (borg_skill[BI_MAXDEPTH] * 5000L);
                break;

                case TV_ROD:
                value = (borg_skill[BI_MAXDEPTH] * 3000L);
                break;

                case TV_STAFF:
                case TV_WAND:
                value = (borg_skill[BI_MAXDEPTH] * 2000L);
                break;

                case TV_SCROLL:
                case TV_POTION:
                value = (borg_skill[BI_MAXDEPTH] * 500L);
                break;

                case TV_FOOD:
                value = (borg_skill[BI_MAXDEPTH] * 10L);
                break;
            }
        }

        /* Hack -- try not to destroy "unknown" items */
        if (!item->ident && (value > 0) && borg_item_worth_id(item))
        {
            /* Reward "unknown" items */
            switch (item->tval)
            {
                case TV_SHOT:
                case TV_ARROW:
                case TV_BOLT:
                value += 100L;
                break;

                case TV_BOW:
                value += 20000L;
                break;

                case TV_DIGGING:
                value += 10L;
                break;

                case TV_HAFTED:
                case TV_POLEARM:
                case TV_SWORD:
                value += 10000L;
                break;

                case TV_BOOTS:
                case TV_GLOVES:
                case TV_HELM:
                case TV_CROWN:
                case TV_SHIELD:
                case TV_CLOAK:
                value += 15000L;
                break;

                case TV_SOFT_ARMOR:
                case TV_HARD_ARMOR:
                case TV_DRAG_ARMOR:
                value += 15000L;
                break;

                case TV_AMULET:
                case TV_RING:
                value += 5000L;
                break;

                case TV_STAFF:
                case TV_WAND:
                value += 1000L;
                break;
            }
        }

        /* power is much more important than gold. */
        value = value / 100;

        /* If I have no food, and in town, I must have a free spot to buy food */
        if (borg_skill[BI_CDEPTH] == 0 && borg_skill[BI_FOOD] == 0)
        {
            /* Power is way more important than gold */
            value = value / 500;
        }

        p -= value;

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* all things being equal, get rid of heavy stuff first */
        if (b_p == p && w < b_w) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p; b_w = w;
    }

    /* Examine the inventory */
    if (fix) borg_notice(true);

    /* Attempt to destroy it */
    if (b_i >= 0)
    {
        borg_item *item = &borg_items[b_i];

        /* Debug */
        borg_note(format("# Junking %ld gold (full)", (long int) my_power*100 - b_p));

        /* Try to consume the junk */
        if (borg_consume(b_i)) return (true);

        /* Message */
        borg_note(format("# Destroying %s.", item->desc));

        /* Destroy that item */
        borg_keypress('k');
        borg_keypress(all_letters_nohjkl[b_i]);

        /* This item only */
        borg_keypress('a');

        /* Success */
        return (true);
    }

     /* Hack -- no need */
     borg_do_crush_hole = false;

    /* Paranoia */
    return (false);
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
    int32_t p, b_p = 0L;

    int32_t temp;

    int32_t greed;

    bool fix = false;

    /* No crush if even slightly dangerous */
    if (borg_danger(c_y,c_x,1, true, false) > borg_skill[BI_CURHP] / 20) return (false);

    /* Hack -- never in town */
    if (borg_skill[BI_CDEPTH] == 0) return (false);

    /* Do not crush items unless we are slow */
    if (borg_skill[BI_SPEED] >= 110) return (false);

	/* Not if in munchkin mode */
	if (borg_munchkin_mode) return (false);

    /* must be a good place to crush stuff */
    if (!borg_safe_crush()) return (false);

    /* Calculate "greed" factor */
    greed = (borg_gold / 100L) + 100L;

    /* Minimal and maximal greed */
    if (greed < 500L && borg_skill[BI_CLEVEL] > 35) greed = 500L;
    if (greed > 25000L) greed = 25000L;

    /* Decrease greed by our slowness */
    greed -= (110 - borg_skill[BI_SPEED]) * 500;
    if (greed <=0) greed = 0L;

    /* Scan for junk */
    for (i = 0; i < QUIVER_END; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

		/* Skip our equipment, but do not skip the quiver */
		if (i >= z_info->pack_size && i <= INVEN_FEET) continue;

        /* dont crush the swap weapon */
        if (weapon_swap && i == weapon_swap - 1) continue;
        if (armour_swap && i == armour_swap - 1) continue;

        /* Skip "good" unknown items (unless "icky") */
        if (!item->ident && borg_item_worth_id(item)) continue;

		/* Do not crush Boots, they could be SPEED */
		if (item->tval == TV_BOOTS && !item->ident) continue;

		/* Hack -- Skip artifacts */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident) continue;
        if (borg_item_note_needs_id(item)) continue;

        /* Dont crush weapons if we are weilding a digger */
        if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;

		/* Dont crush it if it is our only source of light */
		if (item->tval == TV_ROD && (item->sval == sv_rod_light &&
            borg_skill[BI_CURLITE] <= 0)) continue;

		/* Rods of healing are too hard to come by */
		if (item->tval == TV_ROD && item->sval == sv_rod_healing) continue;

        /* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Destroy one of the items */
        borg_items[i].iqty--;

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

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
    if (fix) borg_notice(true);

    /* Destroy "useless" things */
    if ((b_i >= 0) && (b_p >= (my_power)))
    {
        borg_item *item = &borg_items[b_i];

        /* Message */
        borg_note(format("# Junking %ld power (slow) value %d", (long int) b_p - my_power, item->value));

        /* Attempt to consume it */
        if (borg_consume(b_i)) return (true);

        /* Message */
        borg_note(format("# Destroying %s.", item->desc));

        /* Drop one item */
        borg_keypress('d');
        if (b_i < INVEN_WIELD)
        {
            borg_keypress(all_letters_nohjkl[b_i]);
        }
        else if (b_i < QUIVER_START)
        {
            borg_keypress('/');

            borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);
        }
        else
        {
            /* Quiver Slot */
            borg_keypress('|');
            borg_keypress('0' + (b_i - QUIVER_START));
        }
        if (item->iqty > 1)
        {
            borg_keypress('1');
            borg_keypress(KC_ENTER);
        }
        /* Destroy that item */
        borg_keypress('k');
        /* Now on the floor */
        borg_keypress('-');
        /* Assume first */
        borg_keypress('a');
        /* This item only */
        borg_keypress('a');
    }

    /* Hack -- no need */
    borg_do_crush_slow = false;

    /* Nothing to destroy */
    return (false);
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
    int i;
    int b_i = -1, b_v = -1;
    bool inv_item_needs_id = false;
    bool free_id = borg_spell_legal(IDENTIFY_RUNE);

    /* don't ID stuff when you can't recover spent spell point immediately */
    if (borg_skill[BI_CURSP] < 50 && borg_spell_legal(IDENTIFY_RUNE) &&
        !borg_check_rest(c_y, c_x))
        return (false);

    /* No ID if in danger */
    if (borg_danger(c_y, c_x, 1, true, false) > 1) return (false);

    /* Look for an item to identify (equipment) */
    for (i = INVEN_WIELD; i < QUIVER_END; i++)
    {
    	int v = 0;
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;
        if (!item->needs_ident) continue;

		/* Preferentially ID egos and artifacts */
		if (item->art_idx)
			v = item->value + 150000L;

		if (borg_ego_has_random_power(&e_info[item->ego_idx]))
		{
			v = item->value + 100000L;
		}

		/* Prioritize the ID */
        if (borg_item_note_needs_id(item)) v = item->value + 20000L;

        /* Ignore */
        if (!v) continue;

		/* Track the best */
        if (v <= b_v) continue;

        /* Track it */
        b_i = i; b_v = v;

    }

    /* Look for an item to identify  */
    for (i = 0; i < QUIVER_END; i++)
    {
    	int v = 0;
        borg_item *item = &borg_items[i];

        /* Skip empty and ID'd items */
        if (!item->iqty) continue;
        if (!item->needs_ident) continue;

        if (i < z_info->pack_size)
            inv_item_needs_id = true;

		/* Preferentially ID artifacts */
		if (item->art_idx)
			v = item->value + 150000L;

        /* Identify "good" items */
        if (borg_item_note_needs_id(item)) v = item->value + 20000L;
 		else if (free_id || borg_item_worth_id(item)) v = item->value;

        /* Hack -- reward "unaware" items */
        if (!item->kind)
        {
            /* Analyze the type */
            switch (item->tval)
            {
                case TV_RING:
                case TV_AMULET:
	                v += (borg_skill[BI_MAXDEPTH] * 5000L);
	                break;

                case TV_ROD:
	                v += (borg_skill[BI_MAXDEPTH] * 3000L);
	                break;

                case TV_WAND:
                case TV_STAFF:
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
					v += (borg_skill[BI_MAXDEPTH] * 10L);	
					break;
            }
        }

        /* Ignore */
        if (!v) continue;

        /* Track the best */
        if (v <= b_v) continue;

        /* Track it */
        b_i = i;
        b_v = v;
    }


    /* Found something */
    if (b_i >= 0)
    {
        borg_item *item = &borg_items[b_i];

		/* Use an item to identify */
		if (borg_spell(IDENTIFY_RUNE) || borg_read_scroll(sv_scroll_identify))
		{
			/* Log -- may be cancelled */
			borg_note(format("# Identifying %s.", item->desc));

			/* Equipment */
			if (b_i >= INVEN_WIELD && b_i < QUIVER_START)
			{
                if (inv_item_needs_id)
                    borg_keypress('/');

                /* Select the item */
                borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);

				/* HACK need to recheck stats if we id something on us. */
				for (i = 0; i < STAT_MAX; i++)
				{
					my_need_stat_check[i] = true;
					my_stat_max[i] = 0;
				}
			}
            else if (b_i >= QUIVER_START)
            {
                /* Select quiver */
                borg_keypress('|');

                /* Select the item */
                borg_keypress(I2D(b_i - QUIVER_START));
            }
			/* Inventory */
			else
			{
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

	char current_right_ring[80];
	char current_left_ring[80];

    /*** Check conditions ***/

    /* Require two empty slots */
    if (hole == -1)
        return (false);

    if ((hole+1) >= PACK_SLOTS)
        return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 1000) return (false);
    if (borg_skill[BI_CDEPTH] != 0) return (false);

    /*** Remove naked "loose" rings ***/

    /* Remove any naked loose ring */
    if (borg_items[INVEN_LEFT].iqty &&
        !borg_items[INVEN_RIGHT].iqty &&
        !borg_items[INVEN_LEFT].one_ring)
    {
        /* Log */
        borg_note("# Taking off naked loose ring.");

        /* Remove it */
        borg_keypress('t');
        borg_keypress(all_letters_nohjkl[INVEN_LEFT - INVEN_WIELD]);

        /* Success */
        return (true);
    }


    /*** Check conditions ***/

    /* Require "tight" ring */
    if (!borg_items[INVEN_RIGHT].iqty) return (false);


    /* Cannot remove the One Ring */
    if (borg_items[INVEN_RIGHT].one_ring) return (false);


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
	memcpy(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT], sizeof(borg_item));

    /* Take off the ring */
	memcpy(&borg_items[hole], &borg_items[INVEN_RIGHT], sizeof(borg_item));

    /* Erase the ring */
    memset(&borg_items[INVEN_RIGHT], 0, sizeof(borg_item));

    /* Examine the inventory */
    borg_notice(false);

    /* Evaluate the inventory */
    v2 = borg_power();

    /* Restore the ring */
	memcpy(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], sizeof(borg_item));

    /* Restore the hole */
	memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));



    /*** Swap rings if necessary ***/

	/* Define the rings and descriptions.  */
	my_strcpy(current_right_ring, borg_items[INVEN_RIGHT].desc, sizeof(current_right_ring));
	my_strcpy(current_left_ring, borg_items[INVEN_LEFT].desc, sizeof(current_left_ring));

    /* Remove "useless" ring */
    if (v2 > v1)
    {
        /* Log */
        borg_note("# Taking off less valuable right ring.");

        /* Take it off */
		if (borg_items[INVEN_RIGHT].iqty)
		{
				borg_keypress('t');
                borg_keypress(all_letters_nohjkl[INVEN_RIGHT - INVEN_WIELD]);
                borg_keypress(' ');
		}

		/* make sure one is on the left */
		if (borg_items[INVEN_LEFT].iqty)
		{
			borg_note("# Taking off more valuable left ring.");
			borg_keypress('t');
            borg_keypress(all_letters_nohjkl[INVEN_LEFT - INVEN_WIELD]);
            borg_keypress(' ');
		}

        /* Success */
        return (true);
    }

    /* Nope */
    return (false);
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
    int hole = borg_first_empty_inventory_slot();

    int32_t p, b_p = 0L;

    int i, b_i = -1;

    borg_item *item;

    bool fix = false;

    if (hole == -1) return (false);

    /* Require no rings */
    if (borg_items[INVEN_LEFT].iqty) return (false);
    if (borg_items[INVEN_RIGHT].iqty) return (false);

    /* Require two empty slots */
    if (hole + 1 >= PACK_SLOTS) return (false);
    if (borg_items[hole+1].iqty) return (false);

    /* hack prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (false);

    /* Scan inventory */
    for (i = 0; i < z_info->pack_size; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (!item->kind) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        /* skip artifact rings not star id'd  */
        if (OPT(player, birth_randarts) && !item->ident && item->art_idx) continue;

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* Only process "rings" */
        if (slot != INVEN_LEFT) continue;

        /* Occassionally evaluate swapping into the tight finger */
        if (randint0(100) > 75 || item->one_ring)
        {
            slot = INVEN_RIGHT;
        }

        /* Need to be careful not to put the One Ring onto
         * the Left Hand
         */
        if (item->one_ring &&
           (borg_items[INVEN_RIGHT].iqty))
            continue;

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

        /* Evaluate the inventory */
        p = borg_power();

		/* the One Ring would be awsome */
		if (item->one_ring) p = my_power * 2;

        /* Restore the old item (empty) */
        memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

        /* Restore the new item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the "best" */
        b_i = i; b_p = p;
    }

    /* Restore bonuses */
    if (fix) borg_notice(true);

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
        borg_keypress(all_letters_nohjkl[b_i]);

        /* Did something */
        time_this_panel ++;
        return (true);
    }

    /* Nope */
    return (false);
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

    int32_t b_p = 0L;
    int32_t b_p1 = 0L;
    int32_t b_p2 = 0L;

    int i;

	int save_rconf = 0;
	int save_rblind = 0;
	int save_fract = 0;

    borg_item *item;

    bool fix = false;

    /* hack prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (time_this_panel > 300) return (false);

    /* make sure we have an appropriate swap */
    if (!armour_swap && !weapon_swap) return (false);

    if (armour_swap)
    {
        /* Save our normal condition */
        save_rconf = borg_skill[BI_RCONF];
        save_rblind = borg_skill[BI_RBLIND];
        save_fract = borg_skill[BI_FRACT];

        /* Check the items, first armour then weapon */
        i = armour_swap - 1;

        /* make sure it is not a -1 */
        if (i == -1) i = 0;

        /* get the item */
        item = &borg_items[i];

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* safety check incase slot = -1 */
        if (slot < 0) return (false);

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
        b_p1 = borg_danger(c_y, c_x, 1, true, false);

        /* Restore the old item (empty) */
        memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

        /* Restore the new item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

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
        if (fix) borg_notice(true);

        /*  skip random artifact not star id'd  */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident) b_p1 = 9999;

        /* skip it if it has not been decursed */
        if (item->cursed && !item->uncursable) b_p1 = 9999;
    }

    /* Now we check the weapon */
    if (weapon_swap)
    {
        /* get the item */
        i = weapon_swap - 1;

        /* make sure it is not a -1 */
        if (i == -1) i = 0;

        item = &borg_items[i];

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* safety check incase slot = -1 */
        if (slot < 0) return (false);

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
        b_p2 = borg_danger(c_y, c_x, 1, true, false);

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
        memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

        /* Restore the new item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Restore bonuses */
        if (fix) borg_notice(true);

        /*  skip random artifact not star id'd  */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident) b_p2 = 9999;

        /* skip it if it has not been decursed */
        if (item->cursed && !item->uncursable) b_p2 = 9999;
    }

    /* Pass on the swap which yields the best result */
    if (b_p1 <= b_p2)
    {
        b_p = b_p2;
        swap = weapon_swap - 1;
    }
    else
    {
        b_p = b_p1;
        swap = armour_swap - 1;
    }

    /* good swap.  Make sure it helps a significant amount */
    if (p > b_p &&
        b_p <= (borg_fighting_unique?((avoidance*2)/3): (avoidance/2)))
    {
        /* Log */
        borg_note(format("# Swapping backup.  (%d < %d).", b_p, p));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[swap]);

        /* Did something */
        return (true);
    }

    /* Nope */
    return (false);
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
	int quiver_capacity;

    borg_item *item;

    /* hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (false);
    if (time_this_panel > 150) return (false);

    /* don't crush stuff unless we are on a floor */
    if (borg_grids[c_y][c_x].feat != FEAT_FLOOR) return (false);

	/* How many should I carry */
    if (borg_class == CLASS_RANGER || borg_class == CLASS_WARRIOR) 
        quiver_capacity = (kb_info[TV_ARROW].max_stack - 1) * 2;
    else 
        quiver_capacity = kb_info[TV_ARROW].max_stack - 1;

    quiver_capacity *= z_info->quiver_size;

    /* Scan equip */
    for (i = QUIVER_END-1; i >= QUIVER_START; i--)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip if it is not cursed and matches my ammo.  If it is not cursed but does
		 * not match my ammo, then it is dumped.
		 */
        if (!item->cursed && item->tval == borg_skill[BI_AMMO_TVAL] )
		{
			/* It has some value */
			if (item->to_d > 0 && item->to_h > 0) continue;
            if (borg_item_note_needs_id(item)) continue;

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

        /* Drop it */
        borg_keypress('k');
        borg_keypress('|');
        borg_keypress(b_i - QUIVER_START + '0');
        borg_keypress('a');
        item->iqty = 0;

        /* Did something */
        time_this_panel ++;
        return (true);
    }

    /* Nope */
    return (false);

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
    int hole = borg_first_empty_inventory_slot();

    int32_t p, b_p = 0L, w_p= 0L;

    int i, b_i = -1;

    borg_item *item;

    bool fix = false;

    /* if there was no hole, done */
    if (hole == -1) return (false);

    /*  hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (false);
    if (time_this_panel > 150) return (false);

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

        /* Require "known" (or needs id) */
        if (borg_item_note_needs_id(item)) continue;

        /* skip it if it has not been decursed */
        if (item->one_ring) continue;

        /* Save the hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Save the item */
		memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Take off the item */
		memcpy(&borg_items[hole], &safe_items[i], sizeof(borg_item));

        /* Erase the item from equip */
        memset(&borg_items[i], 0, sizeof(borg_item));

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
		memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Restore the hole */
		memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));


        /* Track the crappy items */
        /* crappy includes things that do not add to power */

        if (p >= b_p)
        {
            b_i = i;
            w_p = p;
        }

    }

    /* Restore bonuses */
    if (fix) borg_notice(true);

    /* No item */
    if (b_i >= 0)
    {
        /* Get the item */
        item = &borg_items[b_i];

        if (borg_cfg[BORG_VERBOSE])
        {
            /* dump list and power...  for debugging */
            borg_note(format("Equip Item %d %s.", i, safe_items[i].desc));
            borg_note(format("With Item     (borg_power %ld)", (long int) b_p));
            borg_note(format("Removed Item  (best power %ld)", (long int) p));
        }

        /* Log */
        borg_note(format("# Removing %s.  Power with: (%ld) Power w/o (%ld)",
            item->desc, (long int) b_p, (long int) w_p));

        /* Wear it */
        borg_keypress('t');
        borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);

        /* Did something */
        time_this_panel ++;
        return (true);
    }

    /* Nope */
    return (false);
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

    int slot;
    int d;
	int o;
	bool recently_worn = false;

    int32_t p, b_p = 0L;

    int i, b_i = -1;
    int ii, b_ii =  -1;
    int danger;

	char target_ring_desc[80];

    borg_item *item;

    bool fix = false;

	/* Start with current power */
	b_p = my_power;

    /*  hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

	/* We need an empty slot to simulate pushing equipment */
    hole = borg_first_empty_inventory_slot();
	if (hole == -1) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (false);
    if (time_this_panel > 1300) return (false);

    /* Scan inventory */
    for (i = 0; i < z_info->pack_size; i++)
    {
        item = &borg_items[i];

		/* reset this item marker */
		recently_worn = false;

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (!item->kind) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        /* do not wear not *idd* artifacts */
		if (OPT(player, birth_randarts) && item->art_idx && !item->ident) continue;

    	/* Do not consider wearing this item if I worn it already this level,
    	 * I might be stuck in a loop.
     	 */
    	for (o = 0; o < track_worn_num; o++)
	    {
        	/* Examine the worn list */
	        if (track_worn_num >=1 && track_worn_name1[o] == item->art_idx &&
				track_worn_time > borg_t - 10)
        	{
            	/* Recently worn item */
	            recently_worn = true;
        	}
     	}

		/* Note and fail out */
		if (recently_worn == true)
		{
			borg_note(format("# Not considering a %s; it was recently worn.", item->desc));
			continue;
		}

        /* Where does it go */
        slot = borg_wield_slot(item);

        /* Cannot wear this item */
        if (slot < 0) continue;

		/* Do not wear certain items if I am over weight limit.  It induces loops */
		if (borg_skill[BI_ISENCUMB])
		{
			/* Compare Str bonuses */
			if (borg_items[slot].modifiers[OBJ_MOD_STR] >
				item->modifiers[OBJ_MOD_STR]) continue;
		}

        /* Obtain danger */
        danger = borg_danger(c_y,c_x,1, true, false);

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
            d = borg_danger(c_y,c_x,1, true, false);

			if (borg_cfg[BORG_VERBOSE])
			{
				/* dump list and power...  for debugging */
				borg_note(format("Trying  Item %s (best power %ld)",borg_items[slot].desc, (long int) p));
				borg_note(format("Against Item %s (borg_power %ld)",safe_items[slot].desc, (long int) b_p));
			}

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
            if (danger < d) continue;

            /* XXX XXX XXX Consider if slot is empty */

            /* Hack -- Ignore "essentially equal" swaps */
            if (p <= b_p+50) continue;

            /* Maintain the "best" */
            b_i = i; b_p = p;
        } /* non-rings, non full */


	if (randint0(100)==10 || item->one_ring)
	{
        /* ring, full hands */
        if (slot == INVEN_LEFT &&
            borg_items[INVEN_LEFT].tval && borg_items[INVEN_RIGHT].tval)
        {
                for (ii = INVEN_RIGHT; ii <= INVEN_LEFT; ii++)
                {
                    slot = ii;

                    /* Does One Ring need to be handled here? */

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
                    d = borg_danger(c_y,c_x,1, true, false);

                    /* Restore the old item */
                    memcpy(&borg_items[slot], &safe_items[slot], sizeof(borg_item));

                    /* Restore the new item */
                    memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

                    /* Restore the hole */
                    memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

			        /* Need to be careful not to put the One Ring onto
			         * the Left Hand
			         */
			        if (ii == INVEN_LEFT && item->one_ring)
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
    if (fix) borg_notice(true);

    /* item */
    if ((b_i >= 0) && (b_p > my_power))
    {
        /* Get the item */
        item = &borg_items[b_i];

		/* Define the desc of the nice ring */
		my_strcpy(target_ring_desc, item->desc, sizeof(target_ring_desc ));

        /* Remove old ring to make room for good one */
        if (b_ii >= INVEN_RIGHT && item->tval == TV_RING)
        {
            /* Log */
            borg_note(format("# Removing %s to make room for %s.", borg_items[b_ii].desc, item->desc));

            /* Make room */
            borg_keypress('t');
            borg_keypress(all_letters_nohjkl[b_ii - INVEN_WIELD]);

	        /* Did something */
	        time_this_panel ++;
	        return (true);
        }

        /* Log */
        borg_note(format("# Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[b_i]);
        time_this_panel ++;

        /* Track the newly worn artifact item to avoid loops */
        if (item->art_idx && (track_worn_num < track_worn_size))
        {
            borg_note("# Noting the wearing of artifact.");
            track_worn_name1[track_worn_num] = item->art_idx;
			track_worn_time = borg_t;
            track_worn_num++;
        }
        return (true);

	}

    /* Nope */
    return (false);
}

#if false
/*
 * Equip useful missiles.
 *
 * Look through the inventory for missiles that need to be wielded.
 * The quiver has 10 slots and can carry MAX_STACK_SIZE-1 of the identical missiles.
 */
bool borg_wear_quiver(void)
{
    int hole = QUIVER_END-1;
	int p;
	borg_item *slot;
    int i, b_i = -1;
	char target_desc[80];
    borg_item *item;
    bool fix = false;

    /* hack to prevent the swap till you drop loop */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000) return (false);
    if (time_this_panel > 1300) return (false);

    /* Scan inventory */
    for (i = 0; i < z_info->pack_size; i++)
    {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (item->tval != borg_skill[BI_AMMO_TVAL] ) continue;

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

				if (item->ego_idx == slot->ego_idx &&
					item->dd  == slot->dd &&
					item->ds  == slot->ds &&
					item->to_d  == slot->to_d &&
					item->to_h  == slot->to_h)
				{
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
    if (fix) borg_notice(true);

    /* item */
    if (b_i >= 0)
    {
        /* Get the item */
        item = &borg_items[b_i];

		/* Define the desc of the nice ring */
		my_strcpy(target_desc, item->desc, sizeof(target_desc));

        /* Log */
        borg_note(format("# Loading Quiver %s.", item->desc));

        /* Wear it */
		borg_keypress(ESCAPE);
        borg_keypress('w');
//        borg_keypress(I2A(b_i));
        borg_keypress(all_letters_nohjkl[b_i]);
        time_this_panel ++;
        return (true);

	}

    /* Nope */
    return (false);
}
#endif


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
	switch (n)
	{
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
static void borg_best_stuff_aux(int n, uint8_t *test, uint8_t *best, int32_t *vp)
{
    int i;

    int slot;


    /* Extract the slot */
    slot = borg_best_stuff_order(n);

    /* All done */
    if (slot == 255)
    {
        int32_t p;

        /* Examine */
        borg_notice(false);

        /* Evaluate */
        p = borg_power();

        /* Track best */
        if (p > *vp)
        {
            if (borg_cfg[BORG_VERBOSE])
            {
                /* dump list and power...  for debugging */
                borg_note(format("Trying Combo (best power %ld)", (long int) *vp));
                borg_note(format("             (borg_power %ld)", (long int) p));
                for (i = 0; i < z_info->pack_size; i++)
                    borg_note(format("inv %d %s.", i, borg_items[i].desc));
                for (i = 0; borg_best_stuff_order(i) != 255; i++)
                    borg_note(format("stuff %s.",
                        borg_items[borg_best_stuff_order(i)].desc));
            }
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
    for (i = 0; i < ((shop_num == 7) ? (z_info->pack_size + z_info->store_inven_max) : z_info->pack_size); i++)
    {
        borg_item *item;
        if (i < z_info->pack_size)
            item = &borg_items[i];
        else
            item = &borg_shops[7].ware[i - z_info->pack_size];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require "aware" */
        if (!item->kind) continue;

        /* Hack -- ignore "worthless" items */
        if (!item->value) continue;

        /* Skip it if it is not decursable */
        if (item->cursed && !item->uncursable) continue;

        /* Do not wear not *idd* artifacts */
		if (OPT(player, birth_randarts) && item->art_idx && !item->ident) continue;

        /* Make sure it goes in this slot, special consideration for checking rings */
        if (slot != borg_wield_slot(item)) continue;

        /* Make sure that slot does not have a cursed item */
        if (borg_items[slot].one_ring) continue;

		/* Do not wear certain items if I am over weight limit.  It induces loops */
		if (borg_skill[BI_ISENCUMB])
		{
			/* Compare Str bonuses */
			if (borg_items[slot].modifiers[OBJ_MOD_STR] > item->modifiers[OBJ_MOD_STR])
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
    int hole;
	char purchase_target[1];
    int k;
	uint8_t t_a;
    char buf[1024];
	int p;

    int32_t value;

    int i;

    uint8_t test[12];
    uint8_t best[12];

	/* Hack -- Anti-loop */
    if (time_this_panel >= 300) return (false);

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
		if (i >= z_info->pack_size && i < INVEN_WIELD) continue;

		/* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));
    }

    if (shop_num == 7)
    {
        /* Hack -- Copy all the store slots */
        for (i = 0; i < z_info->store_inven_max; i++)
        {
            /* Save the item */
            memcpy(&safe_home[i], &borg_shops[7].ware[i], sizeof(borg_item));
        }
    }

    /* Evaluate the inventory */
    value = my_power;

    /* Determine the best possible equipment */
    (void)borg_best_stuff_aux(0, test, best, &value);

    /* Restore bonuses */
    borg_notice(true);

    /* Make first change. */
    for (k = 0; k < 12; k++)
    {
        /* Get choice */
        i = best[k];

        /* Ignore non-changes */
        if (i == borg_best_stuff_order(k) || 255 == i) continue;

        if (i < 100)
        {
			borg_item *item = &borg_items[i];

			/* Catch the keyboard flush induced from the 'w' */
	        if ((0 == borg_what_text(0, 0, 6, &t_a, buf)) &&
				(streq(buf, "(Inven")))
			{
                borg_keypress(all_letters_nohjkl[i]);

				/* Track the newly worn artifact item to avoid loops */
				if (item->art_idx && (track_worn_num < track_worn_size))
				{
					borg_note("# Noting the wearing of artifact.");
					track_worn_name1[track_worn_num] = item->art_idx;
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
				return (true);
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


			return (true);
	    }
        else
        {
            borg_item *item;

            /* can't get an item if full. */
            hole = borg_first_empty_inventory_slot();
            if (hole == -1) return false;

            i-=100;

            item = &borg_shops[7].ware[i];

            /* Dont do it if you just sold this item */
            for (p = 0; p < sold_item_num; p++)
			{
				if (sold_item_tval[p] == item->tval && sold_item_sval[p] == item->sval &&
                    sold_item_store[p] == 7) return (false);
			}

            /* Get the item */
            borg_note(format("# Getting (Best Fit) %s.", item->desc));

			/* Define the special key */
			purchase_target[0] = shop_menu_items[i];

	        /* Purchase that item */
			borg_keypress(purchase_target[0]);   
            borg_keypress('p');
            /* press ENTER twice (mulitple objects) */
            borg_keypress(KC_ENTER);
            borg_keypress(KC_ENTER);

            /* leave the building */
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);
            borg_keypress(ESCAPE);

            /* tick the clock */
            time_this_panel ++;

			/* Note that this is a nice item and not to sell it right away */
			borg_best_fit_item = item->art_idx;

            return (true);
        }
    }

    /* Nope */
    return (false);
}

static const struct effect_kind effects[] =
{
	{ EF_NONE, false, NULL, NULL, NULL, NULL },
	#define F(x) effect_handler_##x
	#define EFFECT(x, a, b, c, d, e, f)	{ EF_##x, a, b, F(x), e, f },
	#include "list-effects.h"
	#undef EFFECT
	#undef F
	{ EF_MAX, false, NULL, NULL, NULL, NULL }
};

static bool borg_can_play_spell(borg_magic* as)
{
    /* There are some spells not worth testing */
/* they can just be used when the borg is ready to use it. */
    switch (as->spell_enum)
    {
    case LIGHTNING_STRIKE:
    case TAP_UNLIFE:
    case TAP_MAGICAL_ENERGY:
    case REMOVE_CURSE:
    case TREMOR:
    case WORD_OF_DESTRUCTION:
    case GRONDS_BLOW:
    case DECOY:
    case GLYPH_OF_WARDING:
    return false;
    default:
    break;
    }

    if (as->effect_index == EF_BRAND_BOLTS)
        return false; // !FIX !TODO !AJG check for a bolt
    if (as->effect_index == EF_CREATE_ARROWS)
        return false; // !FIX !TODO !AJG check for a staff
    if (as->effect_index == EF_BRAND_AMMO)
        return false; // !FIX !TODO !AJG check for ammo
    if (as->effect_index == EF_ENCHANT)
        return false; // !FIX !TODO !AJG check something to enchant 
    if (as->effect_index == EF_IDENTIFY)
        return false; // !FIX !TODO !AJG check something to identify
    if (as->effect_index == EF_RECHARGE)
        return false; // !FIX !TODO !AJG check for a wand or rod or staff
    return true;
}

/*
 * Study and/or Test spells/prayers
 */
bool borg_play_magic(bool bored)
{
	int r, b_r = -1;
	int spell_num, b_spell_num;

	/* Hack -- must use magic or prayers */
	if (!player->class->magic.total_spells) return (false);

    /* Hack -- blind/confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (false);

    /* Dark */
    if (!borg_skill[BI_CURLITE]) return (false);
    if (borg_grids[c_y][c_x].info == BORG_DARK) return (false);

	/* loop through spells backward */
	for (spell_num = player->class->magic.total_spells - 1; spell_num >= 0; spell_num--)
	{
		borg_magic *as = &borg_magics[spell_num];

		/* Look for the book in inventory*/
		if (borg_book[as->book] < 0) continue;

		/* Require "learnable" status */
		if (as->status != BORG_MAGIC_OKAY) continue;

		/* Obtain "rating" */
		r = as->rating;

		/* Skip "boring" spells/prayers */
		if (!bored && (r <= 50)) continue;

		/* Skip "icky" spells/prayers */
		if (r <= 0) continue;

		/* Skip "worse" spells/prayers */
		if (r <= b_r) continue;

		/* Track it */
		b_r = r;
		b_spell_num = spell_num;
    }


    /* Study */
    if (borg_skill[BI_ISSTUDY] && (b_r > 0))
    {
		borg_magic *as = &borg_magics[b_spell_num];

		/* Debugging Info */
        borg_note(format("# Studying spell/prayer %s.", as->name));

        /* Learn the spell */
        borg_keypress('G');

		/* Specify the book */
        borg_keypress(all_letters_nohjkl[borg_book[as->book]]);

		/* Specify the spell  */
		if (player_has(player, PF_CHOOSE_SPELLS))
		{
			/* Specify the spell */
            borg_keypress(all_letters_nohjkl[as->book_offset]);
        }

		/* Success */
        return (true);
    }

	/* Hack -- only in town */
    if (borg_skill[BI_CDEPTH] && !borg_munchkin_mode) return (false);

    /* Hack -- only when bored */
    if (!bored) return (false);

    /* Check each spell (backwards) */
	for (spell_num = player->class->magic.total_spells - 1; spell_num >= 0; spell_num--)
    {
		borg_magic *as = &borg_magics[spell_num];

		/* No such book */
		if (borg_book[as->book] < 0) continue;

		/* Only try "untried" spells/prayers */
		if (as->status != BORG_MAGIC_TEST) continue;

        /* some spells can't be "played with" in town */
        if (!borg_can_play_spell(as)) continue;

		/* Some spells should not be tested in munchkin mode */
		if (borg_munchkin_mode)
		{
			/* Mage types */
			if (player_has(player, PF_CHOOSE_SPELLS) &&
				as->spell_enum == MAGIC_MISSILE) continue;

			/* Priest type */
			if (!player_has(player, PF_CHOOSE_SPELLS) &&
				as->spell_enum == BLESS) continue;
		}

		/* Note */
		borg_note("# Testing untried spell/prayer");

		/* Hack -- Use spell or prayer */
		if (borg_spell(as->spell_enum))
		{
			/* Hack -- Allow attack spells */
            /* MEGAHACK -- assume "Random" is shooting.  */
			if (effects[as->effect_index].aim || 
                as->effect_index == EF_RANDOM ||
                as->effect_index == EF_TELEPORT_TO)
            {
				/* Hack -- target self */
				borg_keypress('*');
				borg_keypress('t');
			}

			/* Hack -- Allow spells that require selection of a monster type */
			if (as->effect_index == EF_BANISH)
			{
				/* Hack -- target Maggot */
				borg_keypress('h');
			}

			/* Success */
			return (true);
		}
	}

    /* Nope */
    return (false);
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
    int p, sv_qty;


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
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip "crappy" items */
        if (item->value <= 0) continue;

        /* skip our swap weapon */
        if (weapon_swap && i == weapon_swap-1) continue;
        if (armour_swap && i == armour_swap-1) continue;

		/* Dont sell my ammo */
		if (item->tval == borg_skill[BI_AMMO_TVAL] ) continue;

		/* Don't sell my books */
        if (obj_kind_can_browse(&k_info[item->kind])) continue;

		/* Don't sell my needed potion/wands/staff/scroll collection */
		if ((item->tval == TV_POTION && item->sval == sv_potion_cure_serious) ||
			(item->tval == TV_POTION && item->sval == sv_potion_cure_critical) ||
			(item->tval == TV_POTION && item->sval == sv_potion_healing) ||
            (item->tval == TV_POTION && item->sval == sv_potion_star_healing) ||
            (item->tval == TV_POTION && item->sval == sv_potion_life) ||
            (item->tval == TV_POTION && item->sval == sv_potion_speed) ||
            (item->tval == TV_STAFF && item->sval == sv_staff_teleportation) ||
            (item->tval == TV_WAND && item->sval == sv_wand_drain_life) ||
            (item->tval == TV_WAND && item->sval == sv_wand_annihilation) ||
            (item->tval == TV_SCROLL && item->sval == sv_scroll_teleport)) continue;
        /* Obtain the base price */
        price = ((item->value < 30000L) ? item->value : 30000L);

        /* Skip cheap "known" (or "average") items */
        if ((price * item->iqty < greed) && 
            !borg_item_note_needs_id(item)) continue;

        /* only mark things as sellable if getting rid of them doesn't reduce our power much */
        sv_qty = item->iqty;
        item->iqty = 0;
        borg_notice(true);
        p = borg_power();
        item->iqty = sv_qty;;
        if (p + 50 < my_power) continue;

        /* Count remaining items */
        k++;
    }
    /* reset the results */
    borg_notice(true);

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
    if (!borg_check_rest(c_y, c_x)) return (false);

    /* Not if hungry */
    if (borg_skill[BI_ISWEAK]) return (false);

    /* Look for an (wearable- non rod) item to recharge */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* skip items that are charged */
        if (!item->timeout) continue;

		/* Where can it be worn? */
		slot = borg_wield_slot(item);

		/* skip non-ego lights, No need to rest to recharge a torch, which uses fuels turns in o_ptr->timeout */
		if (item->tval == TV_LIGHT && 
            (item->sval == sv_light_torch || 
             item->sval == sv_light_lantern)) 
            continue;

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
        borg_keypress(KC_ENTER);

        /* done */
        return (true);
    }
    /* Item must be worn to be recharged
     */
    if (b_slot >= INVEN_WIELD)
    {

        /* wear the item */
        borg_note("# Swapping Item for Recharge.");
        borg_keypress(ESCAPE);
        borg_keypress('w');
        borg_keypress(all_letters_nohjkl[b_i]);
        borg_keypress(' ');
        borg_keypress(' ');

        /* rest for a while */
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress(KC_ENTER);

        /* done */
        return (true);

    }

    /* nothing to recharge */
    return (false);
}

/*
 * how long should the borg explore?
 */
static int borg_time_to_stay_on_level(bool bored)
{
    /* at low level, don't stay too long, */
    /* but long enough to hope for a feeling */
    if (borg_skill[BI_MAXCLEVEL] < 20)
        return z_info->feeling_need * 100;

    /* at very low level, stay less time */
    if (borg_skill[BI_CLEVEL] < 10)
        return borg_skill[BI_CLEVEL] * 250;

    /* at slightly low level, try not to run out of food staying */
    if (borg_skill[BI_CLEVEL] < 15)
        return borg_skill[BI_REG] ? 2000 : 2500;

    if (bored)
        return borg_stuff_feeling[borg_feeling_stuff] / 10;

    return borg_stuff_feeling[borg_feeling_stuff];
}

/*
 * Leave the level if necessary (or bored)
 * Scumming defined in borg_prepared.
 */
bool borg_leave_level(bool bored)
{
    int sellable_item_count, g = 0;
    bool try_not_to_descend = false;

	bool need_restock = false;

    /* Hack -- waiting for "recall" other than depth 1 */
    if (goal_recalling && borg_skill[BI_CDEPTH] > 1) return (false);

	/* Not bored if I have seen Morgoth recently */
	if (borg_skill[BI_CDEPTH] == 100 && morgoth_on_level &&
		(borg_t - borg_t_morgoth < 5000))
	{
		goal_leaving = false;
		goal_rising = false;
		bored = false;
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
        goal_rising = false;

        /* Wait until bored */
        if (!bored) return (false);

        /* Case for those who cannot Teleport Level */
        if (borg_skill[BI_MAXDEPTH] == 100 && 
           !borg_cfg[BORG_PLAYS_RISKY])
        {
	        if (borg_skill[BI_ATELEPORTLVL] == 0)
            {
                /* These pple must crawl down to 100, Sorry */
                goal_fleeing = true;
                goal_leaving = true;
                stair_more = true;

                /* Note */
                borg_note("# Borg must crawl to deep dungeon- no recall to 100.");

                /* Attempt to use those stairs */
                if (borg_flow_stair_more(GOAL_BORE, false, false)) return (true);

                /* Oops */
                return (false);
            }
        }


        /* Hack -- Recall into dungeon */
        if ((borg_skill[BI_MAXDEPTH] >= (borg_cfg[BORG_WORSHIPS_GOLD] ? 10 : 8)) &&
             (borg_skill[BI_RECALL] >= 3) &&
             (((char *)NULL == borg_prepared(borg_skill[BI_MAXDEPTH]*6/10))||
                borg_cfg[BORG_PLAYS_RISKY]) &&
            borg_recall())
        {
            /* Note */
            borg_note("# Recalling into dungeon.");

            /* Give it a shot */
            return (true);
        }
        else
        {
            /* note why we didn't recall. */
            if (borg_skill[BI_MAXDEPTH] < (borg_cfg[BORG_WORSHIPS_GOLD] ? 10 : 8))
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
                    if ((char *)NULL != borg_prepared(borg_skill[BI_MAXDEPTH]*6/10))
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
        goal_fleeing = true;
        goal_leaving = true;
        }

        stair_more = true;

        /* Attempt to use those stairs */
        if (borg_flow_stair_more(GOAL_BORE, false, false)) return (true);

        /* Oops */
        return (false);
    }

    /** In the Dungeon **/
    const char* prep_cur_depth = borg_prepared(borg_skill[BI_CDEPTH]);
    const char* prep_next_depth = borg_prepared(borg_skill[BI_CDEPTH] + 1);

    /* if not prepared for this level, head upward */
    if (NULL != prep_cur_depth)
    {
        g = -1;
        borg_note(format("# heading up, not prep for current level: %s)",
            prep_cur_depth));
    }

    /* Count sellable items */
    sellable_item_count = borg_count_sell();

    /* Do not dive when "full" of items */
    if (sellable_item_count >= 12)
        try_not_to_descend = true;

    /* Do not dive when drained */
    if (g && borg_skill[BI_ISFIXEXP])
        try_not_to_descend = true;

    /* Rise a level if bored and unable to dive. */
    if (bored && (NULL != prep_next_depth))
    {
        g = -1;
        borg_note(format("# heading up (bored and unable to dive: %s)",
            prep_next_depth));
    }

    /* Rise a level if bored and stastic. */
    else if (bored && avoidance > borg_skill[BI_CURHP])
    {
        if (NULL != prep_next_depth)
        {
            g = -1;
            borg_note("# heading up (bored and spastic).");
        }
        else
        {
            g = 1;
            borg_note("# heading down (bored and spastic).");
        }
    }

	/* Power dive if I am playing too shallow*/
    else if (!try_not_to_descend && NULL == borg_prepared(borg_skill[BI_CDEPTH] + 5) && sellable_item_count < 13)
    {
        g = 1;
        borg_note("# power dive, playing too shallow.");
    }

    /* Power dive if I am playing deep */
    else if (!try_not_to_descend && NULL == prep_next_depth &&
         borg_skill[BI_CDEPTH] >= 75 && borg_skill[BI_CDEPTH] < 100) 
    {
        g = 1;
        borg_note("# power dive, head deep.");
    }

    /* Hack -- Power-climb upwards when needed */
    if (NULL != prep_cur_depth)
    {
		/* Certain checks are bypassed if Unique monster on level */
		if (!unique_on_level)
		{
	        /* if I am really out of depth go to town */
	        if (!g && NULL != borg_prepared(borg_skill[BI_MAXDEPTH]*5/10) &&
	            borg_skill[BI_MAXDEPTH] > 65)
	        {
	            borg_note(format("# Returning to town (too deep: %s)",
                        prep_cur_depth));
	            goal_rising = true;
	        }
            else
            {
                borg_note(format("# Climbing (too deep: %s)", prep_cur_depth));
                g = -1;
            }
		}

        /* if I must  go to town without delay */
        if (NULL != borg_restock(borg_skill[BI_CDEPTH]))
        {
            borg_note(format("# returning to town to restock(too deep: %s)",
                            borg_restock(borg_skill[BI_CDEPTH])));
            goal_rising = true;
            need_restock = true;
        }

		/* I must return to collect stock from the house. */
        if (strstr(prep_cur_depth, "Collect from house"))
        {
            borg_note("# Returning to town to Collect stock.");
            goal_rising = true;
            need_restock = true;
        }
	}

    /* Hack -- if I am playing way too shallow return to town */
    if (NULL == borg_prepared(borg_skill[BI_CDEPTH] + 20) &&
        NULL == borg_prepared(borg_skill[BI_MAXDEPTH] *6/10) &&
        borg_skill[BI_MAXDEPTH] > borg_skill[BI_CDEPTH] + 20 &&
		(borg_skill[BI_RECALL] >= 3 || borg_gold > 2000) )
    {
        borg_note("# returning to town to recall back down (too shallow)");
        goal_rising = true;
    }


    /* Return to town to sell stuff -- No recall allowed.*/
    if (((borg_cfg[BORG_WORSHIPS_GOLD] || borg_skill[BI_MAXCLEVEL] < 15) &&
         borg_skill[BI_MAXCLEVEL] <= 25) && 
        (sellable_item_count >= 12))
    {
        borg_note("# Going to town (Sell Stuff, Worshipping Gold).");
        goal_rising = true;
    }

    /* Return to town to sell stuff (use Recall) */
    if ((bored && borg_skill[BI_MAXCLEVEL] >= 26) && 
        (sellable_item_count >= 12))
    {
        borg_note("# Going to town (Sell Stuff).");
        goal_rising = true;
    }

    /* Return to town when level drained */
    if (borg_skill[BI_ISFIXLEV])
    {
        borg_note("# Going to town (Fix Level).");
        goal_rising = true;
    }

    /* Return to town to restore experience */
    if (bored && borg_skill[BI_ISFIXEXP] && borg_skill[BI_CLEVEL] !=50)
    {
        borg_note("# Going to town (Fix Experience).");
        goal_rising = true;
    }

    /* return to town if it has been a while */
    if ((!goal_rising && bored && !vault_on_level && !borg_fighting_unique &&
         borg_time_town + borg_t - borg_began > 8000) ||
        (borg_time_town + borg_t - borg_began > 12000))
    {
        borg_note("# Going to town (I miss my home).");
        goal_rising = true;
    }

    /* return to town if been scumming for a bit */
    if (borg_skill[BI_MAXDEPTH] >= borg_skill[BI_CDEPTH] + 10 &&
        borg_skill[BI_CDEPTH] <= 12 &&
        borg_time_town + borg_t - borg_began > 3500)
    {
        borg_note("# Going to town (scumming check).");
        goal_rising = true;
    }

    /* Return to town to drop off some scumming stuff */
    if (borg_scumming_pots && !vault_on_level &&
		(borg_skill[BI_AEZHEAL] >= 3 ||
		 borg_skill[BI_ALIFE] >= 1))
    {
        borg_note("# Going to town (Dropping off Potions).");
        goal_rising = true;
    }

    /* Hack -- It is much safer to scum for items on 98
     * Check to see if depth 99, if Sauron is dead and Im not read to fight
     * the final battle
     */
    if (borg_skill[BI_CDEPTH] == 99 && borg_race_death[546] == 1 &&
        borg_ready_morgoth != 1)
    {
        borg_note("# Returning to level 98 to scum for items.");
        g = -1;
    }

    /* Power dive if Morgoth is dead */
    if (borg_skill[BI_KING]) 
        g = 1;

    /* Power dive to 99 if ready */
    if (borg_skill[BI_CDEPTH] < 100 && NULL == borg_prepared(99)) 
        g = 1;

    /* Climb if deeper than I want to be */
    if (!g && borg_skill[BI_CDEPTH] > borg_cfg[BORG_NO_DEEPER])
    {
        borg_note(format("# Going up a bit (No Deeper than %d).", borg_cfg[BORG_NO_DEEPER]));
        g = -1;
    }

	/* if returning to town, try to go upstairs */
    if (!g && goal_rising) 
        g = -1;

    /* Mega-Hack -- spend time on the first level to rotate shops */
    if (borg_skill[BI_CLEVEL] > 10 &&
        (borg_skill[BI_CDEPTH] == 1) &&
        (borg_t - borg_began < 200) &&
        (g < 0) &&
        (borg_skill[BI_FOOD] > 1))
    {
        borg_note("# Staying on level 1 to rotate shops.");
        g = 0;
    }

    /* do not hangout on boring levels for *too* long */
    if (!g && (borg_t - borg_began) > borg_time_to_stay_on_level(bored))
    {
        /* Note */
        borg_note(format("# Spent too long (%d) on level, leaving.", borg_t - borg_began));

        /* if we are trying not to go down, go up*/
        if (try_not_to_descend)
            g = -1;
        else
            /* otherwise use random stairs */
            g = ((randint0(100) < 50) ? -1 : 1);
    }

    /* Go Up */
    if (g < 0)
    {
        /* Take next stairs */
		borg_note("# Looking for up stairs.  Going up.");
        stair_less = true;

        /* Hack -- recall if going to town */
        if (goal_rising &&
            ((borg_time_town + (borg_t - borg_began)) > 200) &&
            (borg_skill[BI_CDEPTH] >= 5) &&
            borg_recall())
        {
            borg_note("# Recalling to town (goal rising)");
            return (true);
        }

		/* Hack -- Recall if needing to Restock */
		if (need_restock &&
		    borg_skill[BI_CDEPTH] >= 5 &&
			borg_recall())
		{
			borg_note("# Recalling to town (need to restock)");
		}

        /* Attempt to use stairs */
        if (borg_flow_stair_less(GOAL_BORE, false))
        {
                borg_note("# Looking for stairs. I'm bored.");
				return (true);
		}

        /* Cannot find any stairs */
        if (goal_rising && bored && (borg_t - borg_began) >= 1000)
        {
            if (borg_recall())
            {
                borg_note("# Recalling to town (no stairs)");
                return (true);
            }
        }

		/* No up stairs found. do down then back up */
		if (track_less.num == 0) g = 1;
    }


    /* Go Down */
    if (g > 0)
    {
        /* Take next stairs */
        stair_more = true;

        /* Attempt to use those stairs */
        if (borg_flow_stair_more(GOAL_BORE, false, false)) return (true);
    }


    /* Failure */
    return (false);
}





/*
 * Initialize this file
 */
void borg_init_7(void)
{
    /* Nothing */
}

/*
 * Release resources allocated by borg_init_7().
 */
void borg_clean_7(void)
{
    /* Nothing */
}

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif /*ALLOW_BORG */
