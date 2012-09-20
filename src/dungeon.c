/* File: dungeon.c */

/* Purpose: the main command interpreter, updating player status */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/* ANGBAND game module					-RAK-	 */
/* The code in this section has gone through many revisions, and */
/* some of it could stand some more hard work.	-RAK-	       */

/* It has had a bit more hard work.			-CJS- */

/* And it has had some more... -BEN- */






/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
static cptr value_check_aux1(inven_type *i_ptr)
{
    /* Artifacts */
    if (artifact_p(i_ptr)) {

        /* Cursed/Broken */
        if (cursed_p(i_ptr) || broken_p(i_ptr)) return "terrible";
        
        /* Normal */
        return "special";
    }

    /* Ego-Items */
    if (ego_item_p(i_ptr)) {
        
        /* Cursed/Broken */
        if (cursed_p(i_ptr) || broken_p(i_ptr)) return "worthless";
        
        /* Normal */
        return "excellent";
    }

    /* Cursed items */
    if (cursed_p(i_ptr)) return "cursed";

    /* Broken items */
    if (broken_p(i_ptr)) return "broken";

    /* Good "armor" bonus */
    if (i_ptr->toac > 0) return "good";
    
    /* Good "weapon" bonus */
    if (i_ptr->tohit + i_ptr->todam > 0) return "good";

    /* Default to "average" */
    return "average";
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
static cptr value_check_aux2(inven_type *i_ptr)
{
    /* Cursed items (all of them) */
    if (cursed_p(i_ptr)) return "cursed";

    /* Broken items (all of them) */
    if (broken_p(i_ptr)) return "broken";

    /* Artifacts -- except cursed/broken ones */
    if (artifact_p(i_ptr)) return "good";

    /* Ego-Items -- except cursed/broken ones */
    if (ego_item_p(i_ptr)) return "good";

    /* Good armor bonus */
    if (i_ptr->toac > 0) return "good";

    /* Good weapon bonuses */
    if (i_ptr->tohit + i_ptr->todam > 0) return "good";

    /* No feeling */
    return (NULL);
}




/*
 * Sense the inventory
 *
 *   Class 0 = Warrior --> fast and heavy
 *   Class 1 = Mage    --> slow and light
 *   Class 2 = Priest  --> fast but light
 *   Class 3 = Rogue   --> okay and heavy
 *   Class 4 = Ranger  --> slow and light
 *   Class 5 = Paladin --> slow but heavy
 */
static void sense_inventory(void)
{
    int		i, penalty = 0;

    int		lev1 = p_ptr->lev;
    int		lev2 = lev1 * lev1;

    cptr	feel;
    
    inven_type *i_ptr;

    char i_name[80];


    /* Certain factors lower the "feeling" rate */
    if (p_ptr->confused || p_ptr->image) return;

    /* Analyze the player class */
    if (p_ptr->pclass == 5) penalty = 80;
    if (p_ptr->pclass == 3) penalty = 20;
    if (p_ptr->pclass == 0) penalty = 9;


    /*** Roll for permission ***/
    
    /* Warriors/Rogues/Paladins */
    if (penalty) {
        if (0 != rand_int(1 + (1000L * penalty) / (lev2 + 40))) return;
    }

    /* Priests */
    else if (p_ptr->pclass == 2) {
        if (0 != rand_int(1 + 10000 / (lev2 + 40))) return;
    }

    /* Mages/Rangers */
    else {
        if (0 != rand_int(10 + 12000 / (lev1 + 5))) return;
    }


    /* Check everything */
    for (i = 0; i < INVEN_TOTAL; i++) {

        i_ptr = &inventory[i];
        
        /* Skip empty slots */
        if (!i_ptr->k_idx) continue;
        
        /* Skip non-wearable items */
        if (!wearable_p(i_ptr)) continue;

        /* Skip Lites and Rings and Amulets */
        if (i_ptr->tval == TV_LITE) continue;
        if (i_ptr->tval == TV_RING) continue;
        if (i_ptr->tval == TV_AMULET) continue;

        /* We know about it already, do not tell us again */
        if (i_ptr->ident & ID_SENSE) continue;

        /* It is fully known, no information needed */
        if (inven_known_p(i_ptr)) continue;

        /* Inventory items have a failure rate */
        if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

        /* Mages have a high failure rate */
        if ((p_ptr->pclass == 1) && (0 != rand_int(20))) continue;

        /* Rangers have a failure rate */
        if ((p_ptr->pclass == 4) && (0 != rand_int(10))) continue;

        /* Check for a feeling */
        feel = (penalty ? value_check_aux1(i_ptr) : value_check_aux2(i_ptr));
                
        /* Skip non-feelings */
        if (!feel) continue;

        /* Stop everything */
        disturb(0, 0);

        /* Get an object description */
        objdes(i_name, i_ptr, FALSE, 0);

        /* Message */
        msg_format("You feel the %s (%c) you are %s %s %s...",
                   i_name, index_to_label(i), describe_use(i),
                   ((i_ptr->number == 1) ? "is" : "are"), feel);

#if 0
        /* Message */
        msg_format("There's something %s about the %s (%c) you are %s...",
                   ((cursed_p(i_ptr) || broken_p(i_ptr)) ? "bad" : "good"),
                   i_name, index_to_label(i), describe_use(i));
#endif

        /* We have "felt" it */
        i_ptr->ident |= (ID_SENSE);

        /* Inscribe it textually */
        if (!i_ptr->note) i_ptr->note = quark_add(feel);

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOICE);
    }
}



/*
 * Regenerate hit points				-RAK-	
 */
static void regenhp(int percent)
{
    s32b        new_chp, new_chp_frac;
    int                   old_chp;

    /* Save the old hitpoints */
    old_chp = p_ptr->chp;

    /* Extract the new hitpoints */
    new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
    p_ptr->chp += new_chp >> 16;   /* div 65536 */

    /* check for overflow */
    if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;
    new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
    if (new_chp_frac >= 0x10000L) {
        p_ptr->chp_frac = new_chp_frac - 0x10000L;
        p_ptr->chp++;
    }
    else {
        p_ptr->chp_frac = new_chp_frac;
    }

    /* Fully healed */
    if (p_ptr->chp >= p_ptr->mhp) {
        p_ptr->chp = p_ptr->mhp;
        p_ptr->chp_frac = 0;
    }

    /* Notice changes */
    if (old_chp != p_ptr->chp) p_ptr->redraw |= (PR_HP);
}


/*
 * Regenerate mana points				-RAK-	
 */
static void regenmana(int percent)
{
    s32b        new_mana, new_mana_frac;
    int                   old_csp;

    old_csp = p_ptr->csp;
    new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
    p_ptr->csp += new_mana >> 16;	/* div 65536 */
    /* check for overflow */
    if ((p_ptr->csp < 0) && (old_csp > 0)) {
        p_ptr->csp = MAX_SHORT;
    }
    new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
    if (new_mana_frac >= 0x10000L) {
        p_ptr->csp_frac = new_mana_frac - 0x10000L;
        p_ptr->csp++;
    }
    else {
        p_ptr->csp_frac = new_mana_frac;
    }

    /* Must set frac to zero even if equal */
    if (p_ptr->csp >= p_ptr->msp) {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }

    /* Redraw mana */
    if (old_csp != p_ptr->csp) p_ptr->redraw |= (PR_MANA);
}






/*
 * Teleport player to a location (presumably near a monster)
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
static void teleport_to(int ny, int nx)
{
    int y, x, dis = 0, ctr = 0;


    /* Find a usable location */
    while (1) {

        /* Pick a nearby legal location */
        while (1) {
            y = rand_spread(ny, dis);
            x = rand_spread(nx, dis);
            if (in_bounds(y, x)) break;
        }

        /* Accept "naked" floor grids */
        if (naked_grid_bold(y, x)) break;

        /* Occasionally advance the distance */
        if (++ctr > (4 * dis * dis + 4 * dis + 1)) {
            ctr = 0;
            dis++;
        }
    }

    /* Move the player */
    move_rec(py, px, y, x);

    /* Check for new panel (redraw map) */
    verify_panel();
}


/*
 * Teleport the player to a new location, up to "dis" units away.
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 * A previous version of this function caused infinite loops.
 */
static void teleport(int dis)
{
    int x, y, d, i, min;

    bool look = TRUE;

    /* Minimum distance */
    min = dis / 2;
        
    /* Look until done */
    while (look) {

        /* Verify max distance */
        if (dis > 200) dis = 200;
        
        /* Try several locations */
        for (i = 0; i < 500; i++) {

            /* Pick a (possibly illegal) location */
            while (1) {
                y = rand_spread(py, dis);
                x = rand_spread(px, dis);
                d = distance(py, px, y, x);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if (!in_bounds(y, x)) continue;

            /* Require "naked" floor space */
            if (!naked_grid_bold(y, x)) continue;

            /* No teleporting into vaults and such */
            if (cave[y][x].info & GRID_ICKY) continue;

            /* This grid looks good */
            look = FALSE;

            /* Stop looking */
            break;
        }

        /* Increase the maximum distance */
        dis = dis * 2;
        
        /* Decrease the minimum distance */
        min = min / 2;
    }

    /* Move the player */
    move_rec(py, px, y, x);
}


/*
 * Handle teleportation
 */
static void handle_teleport(void)
{
    /* No teleport needed */
    if (!teleport_flag) return;

    /* Basic teleport */
    if (teleport_dist) teleport(teleport_dist);

    /* Directed teleport */
    else teleport_to(teleport_to_y, teleport_to_x);

    /* Teleport complete */
    teleport_flag = FALSE;

    /* Check for new panel (redraw map) */
    verify_panel();

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
    p_ptr->update |= (PU_DISTANCE);
}




/*
 * Handle certain things once every 10 game turns
 */
static void process_world(void)
{
    int x, y, i = 0;
    
    int regen_amount;

    cave_type *c_ptr;



    /* Every 10 turns */
    if (turn % 10) return;


    /*** Check the Time and Load ***/

    if (TRUE) {
    
        /* Only check occasionally */
        if ((!(turn % 1000)) &&
             ((0 != check_time()) || (0 != check_load()))) {

            if (closing_flag <= 2) {
                disturb(0, 0);
                closing_flag++;
                msg_print("The gates to ANGBAND are closing...");
                msg_print("Please finish up and/or save your game.");
            }

            else {
                msg_print("The gates to ANGBAND are now closed.");
                (void)strcpy(died_from, "(closing gate: saved)");
                if (save_player()) quit(NULL);
                (void)strcpy(died_from, "a slammed gate");
                death = TRUE;
                exit_game();
            }
        }
    }
    

    /* While in town */
    if (!dun_level) {
    
        /* Hack -- Daybreak/Nighfall in town */
        if (!(turn % ((10L * TOWN_DAWN) / 2))) {

            bool dawn;

            /* Check for dawn */
            dawn = (!(turn % (10L * TOWN_DAWN)));

            /* Night falls */
            if (dawn) {
                msg_print("The sun has risen.");
            }

            /* Day breaks */
            else {
                msg_print("The sun has fallen.");
            }

            /* Hack -- Scan the town, switch the visibility */
            for (y = 0; y < cur_hgt; y++) {
                for (x = 0; x < cur_wid; x++) {

                    /* Get the cave grid */
                    c_ptr = &cave[y][x];

                    /* Assume lit */
                    c_ptr->info |= GRID_GLOW;

                    /* Assume marked if allowed */
                    if (view_perma_grids) c_ptr->info |= GRID_MARK;

                    /* All done if dawn */
                    if (dawn) continue;

                    /* Hack -- skip "permanent" objects */
                    if (c_ptr->info & GRID_PERM) continue;

                    /* Hack -- Make everything else dark */
                    c_ptr->info &= ~GRID_GLOW;
                    c_ptr->info &= ~GRID_MARK;
                }
            }

            /* Update the monsters */
            p_ptr->update |= (PU_MONSTERS);

            /* Redraw stuff */
            p_ptr->redraw |= (PR_MAP);
        }
    }
    

    /* While in the dungeon */    
    else {

        /*** XXX XXX XXX Update the Stores ***/

        /* Update the stores once a day (while in dungeon) */
        if (!(turn % (10L * STORE_TURNS))) {

            if (cheat_xtra) msg_print("Updating Stores...");

            store_maint();

            if (shuffle_owners && (rand_int(STORE_SHUFFLE) == 0)) {
                if (cheat_xtra) msg_print("Shuffling a Store...");
                store_shuffle();
            }

            if (cheat_xtra) msg_print("Done.");
        }
    }
    

    /*** Damage over Time ***/
    
    /* Take damage from poison */
    if (p_ptr->poisoned) {

        /* Take damage */
        disturb(1, 0);
        take_hit(1, "poison");
    }

    /* Take damage from cuts */
    if (p_ptr->cut) {

        /* Mortal wound or Deep Gash */
        if (p_ptr->cut > 200) {
            i = 3;
        }

        /* Severe cut */
        else if (p_ptr->cut > 100) {
            i = 2;
        }

        /* Other cuts */
        else {
            i = 1;
        }
        
        /* Take damage */
        take_hit(i, "a fatal wound");
        disturb(1, 0);
    }


    /*** Check the Food, and Regenerate ***/

    /* Digest some food (each game turn) */
    p_ptr->food -= p_ptr->food_digested;

    /* Starve to death (slowly) */
    if (p_ptr->food < 0) {
        i = 0 - p_ptr->food;
        take_hit(i / 16, "starvation");
        disturb(1, 0);
    }

    /* Default regeneration */
    regen_amount = PY_REGEN_NORMAL;

    /* Getting Weak */
    if (p_ptr->food < PY_FOOD_WEAK) {

        /* Lower regeneration */
        if (p_ptr->food < 0) {
            regen_amount = 0;
        }
        else if (p_ptr->food < PY_FOOD_FAINT) {
            regen_amount = PY_REGEN_FAINT;
        }
        else {
            regen_amount = PY_REGEN_WEAK;
        }

        /* Getting Faint */
        if (p_ptr->food < PY_FOOD_FAINT) {

            /* Faint occasionally */
            if (rand_int(100) < 10) {

                /* Message */
                msg_print("You faint from the lack of food.");
                disturb(1, 0);

                /* Hack -- Bypass "free action" */
                p_ptr->paralysis = 1 + rand_int(5);
            }
        }
    }

    /* Regeneration ability */
    if (p_ptr->regenerate) {
        regen_amount = regen_amount * 2;
    }

    /* Searching or Resting */
    if (p_ptr->searching || p_ptr->rest) {
        regen_amount = regen_amount * 2;
    }

    /* Regenerate the mana */
    if (p_ptr->csp < p_ptr->msp) {
        regenmana(regen_amount);
    }

    /* Poisoned or cut yields no healing */
    if (p_ptr->poisoned) regen_amount = 0;
    if (p_ptr->cut) regen_amount = 0;

    /* Regenerate Hit Points if needed */
    if (p_ptr->chp < p_ptr->mhp) {
        regenhp(regen_amount);
    }


    /*** Timeout Various Things ***/

    /* Hack -- Hallucinating */
    if (p_ptr->image) {
        p_ptr->image--;
    }

    /* Blindness */
    if (p_ptr->blind) {
        p_ptr->blind--;
    }

    /* Times see-invisible */
    if (p_ptr->tim_invis) {
        p_ptr->tim_invis--;
    }

    /* Timed infra-vision */
    if (p_ptr->tim_infra) {
        p_ptr->tim_infra--;
    }

    /* Paralysis */
    if (p_ptr->paralysis) {
        p_ptr->paralysis--;
    }

    /* Confusion */
    if (p_ptr->confused) {
        p_ptr->confused--;
    }

    /* Afraid */
    if (p_ptr->fear) {
        p_ptr->fear--;
    }
    
    /* Fast */
    if (p_ptr->fast) {
        p_ptr->fast--;
    }

    /* Slow */
    if (p_ptr->slow) {
        p_ptr->slow--;
    }

    /* Protection from evil */
    if (p_ptr->protevil) {
        p_ptr->protevil--;
    }

    /* Invulnerability */
    if (p_ptr->invuln) {
        p_ptr->invuln--;
    }

    /* Heroism */
    if (p_ptr->hero) {
        p_ptr->hero--;
    }

    /* Super Heroism */
    if (p_ptr->shero) {
        p_ptr->shero--;
    }

    /* Blessed */
    if (p_ptr->blessed) {
        p_ptr->blessed--;
    }

    /* Shield */
    if (p_ptr->shield) {
        p_ptr->shield--;
    }

    /* Oppose Acid */    
    if (p_ptr->oppose_acid) {
        p_ptr->oppose_acid--;
    }

    /* Oppose Lightning */    
    if (p_ptr->oppose_elec) {
        p_ptr->oppose_elec--;
    }

    /* Oppose Fire */    
    if (p_ptr->oppose_fire) {
        p_ptr->oppose_fire--;
    }

    /* Oppose Cold */    
    if (p_ptr->oppose_cold) {
        p_ptr->oppose_cold--;
    }

    /* Oppose Poison */    
    if (p_ptr->oppose_pois) {
        p_ptr->oppose_pois--;
    }


    /*** Poison and Stun and Cut ***/
    
    /* Poison */
    if (p_ptr->poisoned) {

        int adjust = (adj_con_fix[stat_index(A_CON)] / 2 + 1);

        if (p_ptr->poisoned > adjust) {
            p_ptr->poisoned -= adjust;
        }
        else {
            p_ptr->poisoned = 0;
        }
    }
    
    /* Stun */
    if (p_ptr->stun) {

        int adjust = (adj_con_fix[stat_index(A_CON)] / 2 + 1);

        if (p_ptr->stun > adjust) {
            p_ptr->stun -= adjust;
        }
        else {
            p_ptr->stun = 0;
        }
    }

    /* Cut */
    if (p_ptr->cut) {

        int adjust = (adj_con_fix[stat_index(A_CON)] + 1);

        /* Mortal wound (no healing!) */
        if (p_ptr->cut > 1000) adjust = 0;

        /* Apply some healing */
        if (p_ptr->cut > adjust) {
            p_ptr->cut -= adjust;
        }
        else {
            p_ptr->cut = 0;
        }
    }
}



/*
 * Process the player
 */
static void process_player()
{
    int			i, j;

    inven_type		*i_ptr;


    /* Give the player some energy */
    p_ptr->energy += extract_energy[p_ptr->pspeed];

    /* No turn yet */
    if (p_ptr->energy < 100) return;


#ifdef RANDOM_BOOST
    /* Hack -- small "energy boost" (see "creature.c") */
    p_ptr->energy += rand_int(5);
#endif


    /* XXX XXX XXX Note that most of this code is actually based */
    /* on "game time" not "player time" and so it looks silly */


    /*** Handle Resting ***/

    /* Check "Resting" status */
    if (p_ptr->rest) {

        /* +n -> rest for n turns */
        if (p_ptr->rest > 0) {
            p_ptr->rest--;
            p_ptr->redraw |= (PR_STATE);
            if (p_ptr->rest == 0) {
                rest_off();
            }
        }

        /* -1 -> rest until HP/mana restored */
        else if (p_ptr->rest == -1) {
            if ((p_ptr->chp == p_ptr->mhp) &&
                (p_ptr->csp == p_ptr->msp)) {

                rest_off();
            }
        }

        /* -2 -> like -1, plus blind/conf/fear/stun/slow/stone/halluc/recall */
        /* Note: stop (via "disturb") as soon as blind or recall is done */
        else if (p_ptr->rest == -2) {
            if ((p_ptr->chp == p_ptr->mhp) &&
                (p_ptr->csp == p_ptr->msp) &&
                !p_ptr->blind && !p_ptr->confused &&
                !p_ptr->fear && !p_ptr->stun &&
                !p_ptr->slow && !p_ptr->paralysis &&
                !p_ptr->image && !p_ptr->word_recall) {                

                rest_off();
            }
        }
    }


    /*** Process Light ***/

    /* Check for light being wielded */
    i_ptr = &inventory[INVEN_LITE];

    /* Burn some fuel in the current lite */
    if (i_ptr->tval == TV_LITE) {

        /* Hack -- Use some fuel (except on artifacts) */
        if (!artifact_p(i_ptr) && (i_ptr->pval > 0)) {

            /* Decrease life-span */
            i_ptr->pval--;

            /* Hack -- notice interesting fuel steps */
            if ((i_ptr->pval < 100) || (!(i_ptr->pval % 100))) {
                /* Redraw the choice window */
                p_ptr->redraw |= (PR_CHOICE);
            }
            
            /* Hack -- Special treatment when blind */
            if (p_ptr->blind) {
                /* Hack -- save some light for later */
                if (i_ptr->pval == 0) i_ptr->pval++;
            }

            /* The light is now out */
            else if (i_ptr->pval == 0) {
                disturb(0, 0);
                msg_print("Your light has gone out!");
            }

            /* The light is getting dim */
            else if ((i_ptr->pval < 100) && (!(i_ptr->pval % 10))) {
                disturb(0, 0);
                msg_print("Your light is growing faint.");
            }
        }
    }

    /* Extract the current lite radius */
    extract_cur_lite();

    /* Any "view/lite" change should induce "update_monsters()" */
    if (old_lite != cur_lite) {

        /* Update the lite */
        p_ptr->update |= (PU_LITE);

        /* Update the monsters */
        p_ptr->update |= (PU_MONSTERS);


        /* Sudden loss of lite (or blindness) */
        if (cur_lite <= 0) {

            /* Is this grid dark now? */
            if (no_lite()) {

                /* Notice the darkness */
                msg_print("You can no longer see!");

                /* That is disturbing */
                disturb(0, 0);
            }
        }

        /* Remember the old lite */
        old_lite = cur_lite;
    }


    /*** Process Inventory ***/

    /* Process equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Get the object */
        i_ptr = &inventory[i];

        /* Skip non-objects */
        if (!i_ptr->k_idx) continue;

        /* Recharge activatable objects */
        if (i_ptr->timeout > 0) {

            /* Recharge */
            i_ptr->timeout--;

            /* Update choice window to reflect chance in "timeout" */
            /* if (!(i_ptr->timeout)) p_ptr->redraw |= (PR_CHOICE); */
        }

        /* Mega-Hack -- Process "Drain Experience" flag */
        if (i_ptr->flags3 & TR3_DRAIN_EXP) {
            if ((rand_int(20) == 0) && (p_ptr->exp > 0)) {
                p_ptr->exp--;
                p_ptr->max_exp--;
                check_experience();
            }
        }
    }

    /* Recharge rods */
    for (j = 0, i = 0; i < inven_ctr; i++) {

        i_ptr = &inventory[i];

        /* Examine all charging rods */
        if ((i_ptr->tval == TV_ROD) && (i_ptr->pval)) {

            /* Charge it */
            i_ptr->pval--;

            /* Notice when done */
            if (!(i_ptr->pval)) j++;
        }
    }

    /* Combine pack whenever rods recharge */
    if (j) {

        /* Redraw the Choice Window */
        p_ptr->redraw |= (PR_CHOICE);
        
        /* Combine the pack */    
        combine_pack();
    }
    
    /* Feel the inventory */
    sense_inventory();


    /*** Involuntary Movement ***/

    /* Delayed Word-of-Recall */
    if (p_ptr->word_recall) {

        /* Count down towards recall */
        p_ptr->word_recall--;

        /* Activate the recall */
        if (!p_ptr->word_recall) {

            /* Determine the level */
            if (dun_level) {
                msg_print("You feel yourself yanked upwards!");
                dun_level = 0;
                new_level_flag = TRUE;
            }
            else if (p_ptr->max_dlv) {
                msg_print("You feel yourself yanked downwards!");
                dun_level = p_ptr->max_dlv;
                new_level_flag = TRUE;
            }
            else {
                msg_print("You feel tension leave the air.");
            }
        }
    }


    /*** Handle actual user input ***/

    /* Hack -- Check for "player interrupts" */
    if (command_rep || find_flag || p_ptr->rest) {

        /* Do not wait */
        inkey_scan = TRUE;
        
        /* Check for a key */
        if (inkey()) {

            /* Flush input */
            flush();

            /* Hack -- Show a Message */
            msg_print("Cancelled.");

            /* Disturb the resting, running, or repeating */
            disturb(0, 0);

            /* Hack -- Redraw the state */
            p_ptr->redraw |= (PR_STATE);
        }
    }


    /* Mega-Hack -- Random teleportation */
    if ((p_ptr->teleport) && (rand_int(100) == 0)) {

        /* Short range teleport */
        teleport_flag = TRUE;
        teleport_dist = 40;
    }


    /* Repeat until out of energy */
    while (p_ptr->energy >= 100) {


        /* Notice death, and new levels */
        if (death || new_level_flag) break;

        /* Hack -- Process Teleportation */
        if (teleport_flag) handle_teleport();

        /* Hack -- constant hallucination */
        if (p_ptr->image) p_ptr->redraw |= (PR_MAP);


        /* Notice stuff */
        notice_stuff();
        
        /* Handle stuff */
        handle_stuff();

        /* Hilite the player */
        move_cursor_relative(py, px);

        /* Refresh (optional) */
        if (fresh_before) Term_fresh();

            
        /* Hack -- cancel "lurking browse mode" */
        if (!command_new) command_see = FALSE;


        /* Commands are assumed to take a full turn */
        energy_use = 100;


        /* Special command -- "resting" */
        if ((p_ptr->rest) ||
            (p_ptr->paralysis) ||
            (p_ptr->stun >= 100)) {

            /* Refresh */
            Term_fresh();
        }

        /* Special command -- "running" */
        else if (find_flag) {

            /* Take a step */
            find_step();
        }

        /* Normal command */
        else {

            /* Get a command (new or old) */
            request_command();

            /* Process the command */
            process_command();
        }


        /* Hack -- handle pack over-flow */
        if (inventory[INVEN_PACK].k_idx) {

            int		amt;

            char	i_name[80];


            /* Choose an item to spill */
            i = INVEN_PACK;

            /* Access the slot to be dropped */
            i_ptr = &inventory[i];

            /* Drop all of that item */
            amt = i_ptr->number;

            /* Disturbing */
            disturb(0, 0);
            
            /* Warning */
            msg_print("Your pack overflows!");

            /* Describe */
            objdes(i_name, i_ptr, TRUE, 3);

            /* Message */
            msg_format("You drop %s.", i_name);

            /* Drop it (carefully) near the player */
            drop_near(i_ptr, 0, py, px);

            /* Decrease the item, optimize. */
            inven_item_increase(i, -amt);
            inven_item_optimize(i);
        }


        /* Use a chunk of energy */
        p_ptr->energy -= energy_use;
    }

        
    /* Hack -- notice death and new levels */
    if (death || new_level_flag) return;

    /* Hack -- Process Teleportation */
    if (teleport_flag) handle_teleport();

    /* Notice stuff (one last time) */
    notice_stuff();

    /* Handle stuff (one last time) */
    handle_stuff();
}



/*
 * This is the main function of this file -- it places the user on the
 * current level and processes user input until the level is completed,
 * the user dies, or the game is terminated.
 *
 * I *think* I have successfully pulled the player stuff into the
 * "process_player()" function, and changed "turn" so that it counts
 * "game turns".  This was simplified by assuming that things that
 * used to happen once per player turn should actually happen every
 * tenth game turn.  There are still more things that need extraction.
 */
void dungeon(void)
{
    /* Reset various flags */
    new_level_flag = FALSE;
    teleport_flag = FALSE;
    find_flag = 0;

    /* Reset the "command" vars */
    command_cmd	= 0;
    command_old	= 0;
    command_new	= 0;
    command_esc	= 0;
    command_rep	= 0;
    command_arg	= 0;
    command_dir	= -1;


    /* Cancel the target */
    target_who = 0;

    /* Cancel the health bar */
    health_track(0);
    

    /* Turn off searching */
    search_off();


    /* Remember deepest dungeon level visited */
    if (dun_level > (unsigned)(p_ptr->max_dlv)) {
        p_ptr->max_dlv = dun_level;
    }


    /* Paranoia -- No stairs down from Quest */
    if (is_quest(dun_level)) create_down_stair = FALSE;

    /* Paranoia -- no stairs from town */
    if (!dun_level) create_down_stair = create_up_stair = FALSE;

    /* Option -- no connected stairs */
    if (!dungeon_stair) create_down_stair = create_up_stair = FALSE;
    
    /* Make a stairway. */
    if (create_up_stair || create_down_stair) {

        /* Place a stairway */
        if (valid_grid(py, px)) {

            int k_idx;
            
            cave_type *c_ptr;

            inven_type *i_ptr;

            /* Delete the old object */
            delete_object(py, px);

            /* Choose staircase type */
            if (create_down_stair) {
                k_idx = OBJ_DOWN_STAIR;
            }
            else {
                k_idx = OBJ_UP_STAIR;
            }

            /* Make a new "stair" object */
            c_ptr = &cave[py][px];
            c_ptr->i_idx = i_pop();
            i_ptr = &i_list[c_ptr->i_idx];
            invcopy(i_ptr, k_idx);
            i_ptr->iy = py;
            i_ptr->ix = px;

            /* Make it permanent */
            c_ptr->info |= GRID_PERM;
        }

        /* Cancel the stair request */
        create_down_stair = create_up_stair = FALSE;
    }


    /* Choose a panel row */
    panel_row = ((py - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
    if (panel_row > max_panel_rows) panel_row = max_panel_rows;
    else if (panel_row < 0) panel_row = 0;

    /* Choose a panel col */
    panel_col = ((px - SCREEN_WID / 4) / (SCREEN_WID / 2));
    if (panel_col > max_panel_cols) panel_col = max_panel_cols;
    else if (panel_col < 0) panel_col = 0;

    /* Recalculate the boundaries */
    panel_bounds();


    /* Clear the screen */
    clear_screen();

    /* Redraw everything */
    p_ptr->redraw |= (PR_CAVE);
    p_ptr->redraw |= (PR_CHOICE | PR_RECALL);
    
    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_DISTANCE);
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    
    /* Handle stuff */
    handle_stuff();

    /* Refresh */
    Term_fresh();
    

    /* Announce (or repeat) the feeling, unless in town */
    if (dun_level) do_cmd_feeling();


    /*** Process this dungeon level ***/


    /* Main loop */
    while (TRUE) {

        /* Count game turns */
        turn++;

        /* Process the world */
        process_world();
        
        /* Notice death, and new levels */
        if (death || new_level_flag) break;

        /* Process all of the objects */
        process_objects();	

        /* Notice death, and new levels */
        if (death || new_level_flag) break;

        /* Process the player */
        process_player();

        /* Notice death, and new levels */
        if (death || new_level_flag) break;

        /* Process all of the monsters */
        process_monsters();

        /* Notice death, and new levels */
        if (death || new_level_flag) break;
    }



    /* Handle stuff */
    handle_stuff();
    

    /* Cancel the target */
    target_who = 0;

    /* Cancel the health bar */
    health_track(0);
    

    /* Forget the old lite */
    forget_lite();

    /* Forget the old view */
    forget_view();


    /* Wipe all the items */
    wipe_i_list();

    /* Wipe all the monsters */
    wipe_m_list();
}


