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
 * Given an item, return a textual "feeling" about the item.
 * But if the item has already been "felt", return NULL.
 */
static cptr value_check(inven_type *i_ptr)
{
    /* Paranoia -- No item */
    if (!i_ptr->tval) return (NULL);

    /* Check for previous feelings */
    if (i_ptr->ident & ID_FELT) return (NULL);

    /* Known items need no feeling */
    if (inven_known_p(i_ptr)) return (NULL);

    /* The item must be wearable to know about it */
    if (!wearable_p(i_ptr)) return (NULL);


    /* Cursed items (including artifacts/ego-weapons) */
    if (cursed_p(i_ptr)) {

        /* Cursed artifacts are terrible */
        if (artifact_p(i_ptr)) return "terrible";

        /* Cursed ego-items are worthless */
        if (i_ptr->name2) return "worthless";

        /* Cursed */
        return "cursed";
    }


    /* Check for uncursed artifacts */
    if (artifact_p(i_ptr)) return "special";

    /* Check for "good" ego-items */
    if (i_ptr->name2) {

        /* Some of the ego-items are excellent */
        if (i_ptr->name2 < EGO_MIN_WORTHLESS) return "excellent";

        /* The rest are awful */
        return "worthless";
    }


    /* Mega-Hack -- Any form of bonus is considered good */
    /* This is often misleading for "broken daggers/swords" */
    if ((i_ptr->toac > 0) || (i_ptr->tohit > 0) || (i_ptr->todam > 0)) {
        return "good";
    }

    /* Default to "average" */
    return "average";
}


/*
 * Acquire a "sense" about an item (fighters, rogues, paladins)
 * Note that this "sense" will "pre-empt" normal "feelings".
 */
static void sense_item(int i)
{
    cptr feel;
    inven_type *i_ptr;
    char tmp_str[160];
    char out_val[160];

    /* Get the item */
    i_ptr = &inventory[i];

    /* Mages, Priests, Rangers lose out */
    if (p_ptr->pclass == 1) return;
    if (p_ptr->pclass == 2) return;
    if (p_ptr->pclass == 4) return;

    /* Only wearable items can be sensed */
    if (!wearable_p(i_ptr)) return;

    /* Rings and Amulets cannot be sensed */
    if (i_ptr->tval == TV_LITE) return;
    if (i_ptr->tval == TV_RING) return;
    if (i_ptr->tval == TV_AMULET) return;

    /* Check it for a feeling */
    feel = value_check(i_ptr);

    /* Skip non-feelings */
    if (!feel) return;

    /* Stop everything */
    disturb(0, 0);

    /* Get an object description */
    objdes(tmp_str, i_ptr, FALSE);
    (void)sprintf(out_val,
                  "You feel the %s (%c) you are %s %s %s...",
                  tmp_str, index_to_label(i), describe_use(i),
                  (i_ptr->number == 1) ? "is" : "are", feel);
    message(out_val, 0x04);

    /* We have "felt" it */
    i_ptr->ident |= ID_FELT;

    /* Inscribe it textually */
    if (!i_ptr->inscrip[0]) inscribe(i_ptr, feel);

    /* Success */
    return;
}


/*
 * Sense the inventory
 *
 *   Class 0 = Warrior
 *   Class 1 = Mage
 *   Class 2 = Priest
 *   Class 3 = Rogue
 *   Class 4 = Ranger
 *   Class 5 = Paladin
 */
static void sense_inventory(void)
{
    int i, i_f, lev2, penalty = 0;

    inven_type *i_ptr;

    char tmp_str[160];
    char out_val[160];


    /* Square the players level */
    lev2 = p_ptr->lev * p_ptr->lev;


    /*** Warriors, Rogues and Paladins get a "great" identifier ***/

    /* Extract a "penalty" */
    if (p_ptr->pclass == 5) penalty = 80;
    if (p_ptr->pclass == 3) penalty = 20;
    if (p_ptr->pclass == 0) penalty = 9;

    /* Warriors, Rogues, Paladins get a "special" identifier */
    if (penalty) {

        /* The feeling just "kicks in" every one in a while */
        if (0 == rand_int(1 + 1000L * penalty / (lev2 + 40))) {

            /* Check everything */
            for (i = 0; i < INVEN_TOTAL; i++) {

                /* Hack -- Skip "missing" things */
                if (!inventory[i].tval) continue;

                /* Inventory items only get felt 1 in 5 times */
                if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

                /* Attempt to sense it */
                sense_item(i);
            }
        }

        /* Do NOT do the "simpler" identification */
        return;
    }


    /*** Mages, Priests, Rangers ***/

    /* Priests use a calculation like "warriors" above */
    if (p_ptr->pclass == 2) {
        if (0 != rand_int((10000 / (lev2 + 40)) + 1)) return;
    }

    /* Mages/Rangers use a weird formula */
    else {
        if (0 != rand_int(10 + 12000 / (5 + p_ptr->lev))) return;
    }

    /* Scan the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {

        /* Get the item */
        i_ptr = &inventory[i];

        /* Skip non-wearable items */
        if (!wearable_p(i_ptr)) continue;

        /* Paranoia -- No item given */
        if (!i_ptr->tval) continue;

        /* We know about it already, do not tell us again */
        if (i_ptr->ident & ID_FELT) continue;

        /* It is fully known, no information needed */
        if (inven_known_p(i_ptr)) continue;

        /* We can only feel wearable items */
        if (!wearable_p(i_ptr)) continue;

        /* We cannot "feel" Amulets or Rings or Lites */
        if (i_ptr->tval == TV_LITE) continue;
        if (i_ptr->tval == TV_RING) continue;
        if (i_ptr->tval == TV_AMULET) continue;

        /* Inventory only works 1/5 the time */
        if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

        /* Mages/Rangers only have a 1/10 chance of feeling */
        if ((p_ptr->pclass != 2) && (0 != rand_int(10))) continue;

        /* Default to normal */
        i_f = 0;

        /* Always notice cursed items (including Calris) */
        if (cursed_p(i_ptr)) i_f = -1;

        /* Hack -- Uncursed artifacts feel good */
        else if (artifact_p(i_ptr)) i_f = 1;

        /* Hack -- All "good" ego-items feel good */
        else if (i_ptr->name2 && (i_ptr->name2 < EGO_MIN_WORTHLESS)) i_f = 1;

        /* Sometimes an item just "feels" good */
        else if (i_ptr->tohit>0 || i_ptr->todam>0 || i_ptr->toac>0) i_f = 1;

        /* Skip "unfelt" objects */
        if (!i_f) continue;

        /* Disturb everything */
        disturb(0, 0);

        /* Get an object description */
        objdes(tmp_str, i_ptr, FALSE);
        sprintf(out_val,
                "There's something %s about the %s (%c) you are %s...",
                (i_f > 0 ? "good" : "bad"),
                tmp_str, index_to_label(i), describe_use(i));
        message(out_val, 0x04);

        /* We have "felt" it */
        i_ptr->ident |= ID_FELT;

        /* Inscribe a feeling */
        if (cursed_p(i_ptr)) {
            if (!i_ptr->inscrip[0]) inscribe(i_ptr, "cursed");
        }
        else {
            if (!i_ptr->inscrip[0]) inscribe(i_ptr, "blessed");
        }
    }
}



/*
 * Regenerate hit points				-RAK-	
 */
static void regenhp(int percent)
{
    s32b        new_chp, new_chp_frac;
    int                   old_chp;

    old_chp = p_ptr->chp;
    new_chp = ((long)p_ptr->mhp) * percent + PLAYER_REGEN_HPBASE;
    p_ptr->chp += new_chp >> 16;   /* div 65536 */

    /* check for overflow */
    if (p_ptr->chp < 0 && old_chp > 0) p_ptr->chp = MAX_SHORT;
    new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
    if (new_chp_frac >= 0x10000L) {
        p_ptr->chp_frac = new_chp_frac - 0x10000L;
        p_ptr->chp++;
    }
    else {
        p_ptr->chp_frac = new_chp_frac;
    }

    /* Must set frac to zero even if equal */
    if (p_ptr->chp >= p_ptr->mhp) {
        p_ptr->chp = p_ptr->mhp;
        p_ptr->chp_frac = 0;
    }

    /* Notice changes */
    if (old_chp != p_ptr->chp) p_ptr->redraw |= PR_HP;
}


/*
 * Regenerate mana points				-RAK-	
 */
static void regenmana(int percent)
{
    s32b        new_mana, new_mana_frac;
    int                   old_cmana;

    old_cmana = p_ptr->cmana;
    new_mana = ((long)p_ptr->mana) * percent + PLAYER_REGEN_MNBASE;
    p_ptr->cmana += new_mana >> 16;	/* div 65536 */
    /* check for overflow */
    if ((p_ptr->cmana < 0) && (old_cmana > 0)) {
        p_ptr->cmana = MAX_SHORT;
    }
    new_mana_frac = (new_mana & 0xFFFF) + p_ptr->cmana_frac;	/* mod 65536 */
    if (new_mana_frac >= 0x10000L) {
        p_ptr->cmana_frac = new_mana_frac - 0x10000L;
        p_ptr->cmana++;
    }
    else {
        p_ptr->cmana_frac = new_mana_frac;
    }

    /* Must set frac to zero even if equal */
    if (p_ptr->cmana >= p_ptr->mana) {
        p_ptr->cmana = p_ptr->mana;
        p_ptr->cmana_frac = 0;
    }

    /* Redraw mana */
    if (old_cmana != p_ptr->cmana) p_ptr->redraw |= PR_MANA;
}



/*
 * Heal the monsters (once per 200 game turns)
 */
static void regen_monsters(void)
{
    int i, frac;

    /* Regenerate everyone */
    for (i = MIN_M_IDX; i < m_max; i++) {

        /* Check the i'th monster */
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];
        
        /* Paranoia -- skip dead monsters */
        if (m_ptr->dead) continue;

        /* Allow regeneration (if needed) */
        if (m_ptr->hp < m_ptr->maxhp) {

            /* Base regeneration */
            frac = m_ptr->maxhp / 50;

            /* Minimal regeneration rate */
            if (!frac) frac = 1;

            /* Some monsters regenerate quickly */
            if (r_ptr->rflags2 & RF2_REGENERATE) frac *= 2;

            /* Regenerate */
            m_ptr->hp += frac;

            /* Do not over-regenerate */
            if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;
        }
    }
}



/*
 * Extract and set the current "lite radius"
 *
 * Perhaps use "timeout" instead of "pval" for torches/lanterns.
 * This would "clean up" the semantics of "pval" quite a bit...
 */
void extract_cur_lite(void)
{
    inven_type *i_ptr = &inventory[INVEN_LITE];

    /* Assume no light */
    cur_lite = 0;

    /* Player is glowing */
    if (p_ptr->lite) cur_lite = 1;

    /* All done if no other lite */
    if (i_ptr->tval != TV_LITE) return;

    /* Torches (with fuel) provide some lite */
    if ((i_ptr->sval == SV_LITE_TORCH) && (i_ptr->pval > 0)) cur_lite = 1;

    /* Lanterns (with fuel) provide more lite */
    if ((i_ptr->sval == SV_LITE_LANTERN) && (i_ptr->pval > 0)) cur_lite = 2;

    /* Artifact Lites provide permanent, bright, lite */
    if (artifact_p(i_ptr)) cur_lite = 3;

    /* Reduce lite when running if requested */
    if (find_flag && view_reduce_lite && (cur_lite > 1)) cur_lite = 1;
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
    int x, y, d, count, hack, look, min;

    /* Look until done */
    for (min = dis / 2, look = TRUE; look; min = min / 2) {

        /* Try several locations from "min" to "dis" units away */
        for (count = 0; count < 1000; count++) {

            /* Pick a (possibly illegal) location */
            for (hack = 0; hack < 1000; hack++) {
                y = rand_spread(py, dis);
                x = rand_spread(px, dis);
                d = distance(py, px, y, x);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Hack -- try again */
            if (hack == 1000) continue;

            /* Ignore illegal locations */
            if (!in_bounds(y, x)) continue;

            /* Require "naked" floor space */
            if (!naked_grid_bold(y, x)) continue;

            /* No teleporting into vaults and such */
            if (cave[y][x].info & GRID_ICKY) continue;

            /* This grid looks good */
            look = FALSE;
            break;
        }

        /* Increase the distance if possible */
        if (dis < 200) dis = dis * 2;
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
 * Hack -- process the player
 *
 * Note that some things in this function should actually be done
 * in the "dungeon()" loop, not the "process_player()" loop...
 */
static void process_player()
{
    int			i, j;

    inven_type		*i_ptr;

    /* Regenerate hp and mana */
    int                    regen_amount;

    /* Only update stun/cut when needed */
    static int		old_cut = 0;
    static int		old_stun = 0;


    /* Give the player some energy */
    p_ptr->energy += extract_energy[p_ptr->pspeed];

    /* No turn yet */
    if (p_ptr->energy < 100) return;


    /* XXX XXX XXX Note that most of this code is actually based */
    /* on "game time" not "player time" and so it looks silly */


#ifdef RANDOM_BOOST
    /* Hack -- small "energy boost" (see "creature.c") */
    p_ptr->energy += rand_int(5);
#endif


    /*** Handle visibility ***/

    /* Hack -- Hallucinating */
    if (p_ptr->image) {
        p_ptr->image--;
        p_ptr->redraw |= (PR_MAP);
    }

    /* Blindness */
    if (p_ptr->blind) {
        p_ptr->blind--;
    }

    /* Detect Invisible */
    if (p_ptr->detect_inv) {
        p_ptr->detect_inv--;
    }

    /* Timed infra-vision */
    if (p_ptr->tim_infra) {
        p_ptr->tim_infra--;
    }


    /*** Assorted Maladies ***/

    /* Paralysis */
    if (p_ptr->paralysis) {
        p_ptr->paralysis--;
    }

    /* Confusion */
    if (p_ptr->confused) {
        p_ptr->confused--;
    }

    /* Poisoned */
    if (p_ptr->poisoned) {
        p_ptr->poisoned--;
        if (p_ptr->immune_pois ||
            p_ptr->resist_pois ||
            p_ptr->oppose_pois) {
            p_ptr->poisoned = 0;
        }
    }

    /* Afraid */
    if (p_ptr->afraid) {
        p_ptr->afraid--;
        if (p_ptr->shero || p_ptr->hero) p_ptr->afraid = 0;
        if (p_ptr->resist_fear) p_ptr->afraid = 0;
    }


    /*** Stun and Cut ***/
    
    /* Stun */
    if (p_ptr->stun) {
        int adjust = (con_adj() / 2 + 1);
        if (adjust < 1) adjust = 1;

        if (adjust >= p_ptr->stun) {
            msg_print("Your are no longer stunned.");
            p_ptr->stun = 0;
        }
        else {
            p_ptr->stun = p_ptr->stun - adjust;
        }

        p_ptr->update |= PU_BONUS;
    }

    /* Redraw the stun */
    if (p_ptr->stun != old_stun) {
        p_ptr->redraw |= PR_BLOCK;
        old_stun = p_ptr->stun;
    }


    /* Cut */
    if (p_ptr->cut) {
        int damage = 1;
        int adjust = con_adj() + 1;
        if (adjust < 1) adjust = 1;

        /* Mortal wound */
        if (p_ptr->cut > 1000) {
            adjust = 0;
            damage = 3;
        }

        /* Deep gash */
        else if (p_ptr->cut > 200) {
            damage = 3;
        }

        /* Severe cut */
        else if (p_ptr->cut > 100) {
            damage = 2;
        }

        /* Take damage */
        take_hit(damage, "a fatal wound");
        disturb(1, 0);

        /* Apply some healing */
        if (adjust >= p_ptr->cut) {
            p_ptr->cut = 0;
            if (p_ptr->chp >= 0) msg_print("Your wound heals.");
        }
        else {
            p_ptr->cut -= adjust;
        }
    }

    /* Display the cut */
    if (p_ptr->cut != old_cut) {
        p_ptr->redraw |= PR_BLOCK;
        old_cut = p_ptr->cut;
    }


    /*** Check the Speed ***/

    /* Fast */
    if (p_ptr->fast) {
        p_ptr->fast--;
    }

    /* Slow */
    if (p_ptr->slow) {
        p_ptr->slow--;
    }


    /*** All good things must come to an end... ***/

    /* XXX XXX XXX Protection from evil counter */
    if (p_ptr->protevil) {
        p_ptr->protevil--;
        if (!p_ptr->protevil) {
            msg_print("You no longer feel safe from evil.");
        }
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

    /* XXX XXX XXX Shield */
    if (p_ptr->shield) {
        p_ptr->shield--;
        if (!p_ptr->shield) {
            msg_print("Your mystic shield crumbles away.");
            disturb(0, 0);
            p_ptr->update |= PU_BONUS;
        }
    }



    /*** Timed resistance must end eventually ***/

    if (p_ptr->oppose_fire) {
        p_ptr->oppose_fire--;
        if (!p_ptr->oppose_fire) {
            msg_print("You feel less resistant to fire.");
        }
    }

    if (p_ptr->oppose_cold) {
        p_ptr->oppose_cold--;
        if (!p_ptr->oppose_cold) {
            msg_print("You feel less resistant to cold.");
        }
    }

    if (p_ptr->oppose_acid) {
        p_ptr->oppose_acid--;
        if (!p_ptr->oppose_acid) {
            msg_print("You feel less resistant to acid.");
        }
    }

    if (p_ptr->oppose_elec) {
        p_ptr->oppose_elec--;
        if (!p_ptr->oppose_elec) {
            msg_print("You feel less resistant to lightning.");
        }
    }

    if (p_ptr->oppose_pois) {
        p_ptr->oppose_pois--;
        if (!p_ptr->oppose_pois) {
            msg_print("You feel less resistant to poison.");
        }
    }


    /*** Check the Food, and Regenerate ***/

    /* Default regeneration */
    regen_amount = PLAYER_REGEN_NORMAL;

    /* Getting Hungry */
    if (p_ptr->food < PLAYER_FOOD_ALERT) {

        /* Getting Weak */
        if (p_ptr->food < PLAYER_FOOD_WEAK) {

            if (p_ptr->food < 0) {
                regen_amount = 0;
            }
            else if (p_ptr->food < PLAYER_FOOD_FAINT) {
                regen_amount = PLAYER_REGEN_FAINT;
            }
            else if (p_ptr->food < PLAYER_FOOD_WEAK) {
                regen_amount = PLAYER_REGEN_WEAK;
            }

            /* Notice onset of weakness */
            if (!(PN_WEAK & p_ptr->notice)) {
                p_ptr->notice |= PN_HUNGRY;
                p_ptr->notice |= PN_WEAK;
                msg_print("You are getting weak from hunger.");
                disturb(0, 0);
                p_ptr->redraw |= PR_HUNGER;
            }

            /* Faint for a few turns */
            if ((p_ptr->food < PLAYER_FOOD_FAINT) && (rand_int(8) == 0)) {
                p_ptr->paralysis += rand_int(5) + 1;
                msg_print("You faint from the lack of food.");
                disturb(1, 0);
            }
        }

        /* Only hungry */
        else {

            /* No longer weak */
            if (PN_WEAK & p_ptr->notice) {
                p_ptr->notice &= ~PN_WEAK;
                p_ptr->redraw |= PR_HUNGER;
            }

            /* Note onset of hunger */
            if (!(PN_HUNGRY & p_ptr->notice)) {
                p_ptr->notice |= PN_HUNGRY;
                msg_print("You are getting hungry.");
                disturb(0, 0);
                p_ptr->redraw |= PR_HUNGER;
            }
        }
    }

    /* Well fed */
    else {

        /* No longer hungry */
        if (PN_HUNGRY & p_ptr->notice) {

            /* No longer weak/hungry */
            p_ptr->notice &= ~PN_WEAK;
            p_ptr->notice &= ~PN_HUNGRY;

            /* Update hunger */
            p_ptr->redraw |= PR_HUNGER;
        }
    }


    /* Food consumption XXX XXX XXX XXX */
    /* Note: Speeded up characters really burn up the food!  */
    /* now summation, not square, since spd less powerful -CFT */
    /* Hack -- Note that Speed is different now (2.7.3) */

    /* Fast players consume slightly more food */
    if (p_ptr->pspeed > 110) {
        int ospeed = (110 - p_ptr->pspeed) / 10;
        p_ptr->food -= (ospeed * ospeed - ospeed) / 2;
    }

    /* Digest some food */
    p_ptr->food -= p_ptr->food_digested;

    /* Starve to death */
    if (p_ptr->food < 0) {
        take_hit(-(p_ptr->food / 16), "starvation");	/* -CJS- */
        disturb(1, 0);
    }

    /* Regeneration ability */
    if (p_ptr->regenerate) {
        regen_amount = regen_amount * 3 / 2;
    }

    /* Searching or Resting */
    if (p_ptr->searching || p_ptr->rest) {
        regen_amount = regen_amount * 2;
    }

    /* Regenerate the mana */
    if (p_ptr->cmana < p_ptr->mana) {
        regenmana(regen_amount);
    }

    /* Poisoned or cut yields no healing */
    if (p_ptr->poisoned) regen_amount = 0;
    if (p_ptr->cut) regen_amount = 0;

    /* Regenerate Hit Points if needed */
    if (p_ptr->chp < p_ptr->mhp) {
        regenhp(regen_amount);
    }


    /*** Handle Resting ***/

    /* Check "Resting" status */
    if (p_ptr->rest) {

        /* +n -> rest for n turns */
        if (p_ptr->rest > 0) {
            p_ptr->rest--;
            p_ptr->redraw |= PR_STATE;
            if (p_ptr->rest == 0) {
                rest_off();
            }
        }

        /* -1 -> rest until HP/mana restored */
        else if (p_ptr->rest == -1) {
            if ((p_ptr->chp == p_ptr->mhp) &&
                (p_ptr->cmana == p_ptr->mana)) {

                rest_off();
            }
        }

        /* -2 -> like -1, plus stone/blind/conf/fear/stun/halluc/recall/slow */
        /* Note: stop (via "disturb") as soon as blind or recall is done */
        else if (p_ptr->rest == -2) {
            if ((p_ptr->blind < 1) && (p_ptr->confused < 1) &&
                (p_ptr->afraid < 1) && (p_ptr->stun < 1) &&
                (p_ptr->image < 1) && (p_ptr->word_recall < 1) &&
                (p_ptr->slow < 1) && (p_ptr->paralysis < 1) &&
                (p_ptr->chp == p_ptr->mhp) &&
                (p_ptr->cmana == p_ptr->mana)) {

                rest_off();
            }
        }
    }


    /*** Check the light radius ***/

    /* Check for light being wielded */
    i_ptr = &inventory[INVEN_LITE];

    /* Burn some fuel in the current lite */
    if (i_ptr->tval == TV_LITE) {

        /* Hack -- Use some fuel (unless "permanent") */
        if (!(i_ptr->flags3 & TR3_LITE) && (i_ptr->pval > 0)) {

            /* Decrease life-span */
            i_ptr->pval--;

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
            else if ((i_ptr->pval < 40) && (rand_int(5) == 0)) {
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

        /* Skip fake objects */
        if (!i_ptr->tval) continue;

        /* Let activatable objects recharge */
        if (i_ptr->timeout > 0) i_ptr->timeout--;

        /* Hack -- Process "Drain Experience" flag */
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

        /* Examine ALL charging rods */
        if ((i_ptr->tval == TV_ROD) && (i_ptr->pval)) {

            /* Charge it */
            i_ptr->pval--;

            /* Notice when done */
            if (i_ptr->pval == 0) j++;
        }
    }

    /* Hack -- Combine pack whenever rods recharge */
    if (j) combine_pack();


    /*** Auto-Detect-Enchantment ***/

    /* Have some "feelings" about the inventory */
    if (!p_ptr->confused && !p_ptr->blind) {

        /* Feel the inventory */
        sense_inventory();
    }


    /*** Update the Display ***/

#ifdef USE_MULTIHUED

    /* Visually Cycle the Multi-Hued things */
    mh_cycle();

#endif


    /*** Involuntary Movement ***/

    /* Delayed Word-of-Recall */
    if (p_ptr->word_recall) {

        /* Count down towards recall */
        p_ptr->word_recall--;

        /* Activate the recall */
        if (!p_ptr->word_recall) {

            /* New level */
            new_level_flag = TRUE;

            /* Determine the level */
            if (dun_level) {
                dun_level = 0;
                msg_print("You feel yourself yanked upwards! ");
            }
            else if (p_ptr->max_dlv) {
                dun_level = p_ptr->max_dlv;
                msg_print("You feel yourself yanked downwards! ");
            }
            else {
                new_level_flag = FALSE;
                msg_print("You feel tension leave the air. ");
            }
        }
    }


    /*** Handle actual user input ***/

    /* Hack -- Check for "player interrupts" */
    if (command_rep || find_flag || p_ptr->rest) {

        /* Hack -- move the cursor onto the player */
        move_cursor_relative(py, px);

        /* Flush output, check for keypress */
        if (Term_kbhit()) {

            /* Flush input */
            Term_flush();

            /* Hack -- Show a Message */
            msg_print("Cancelled.");

            /* Disturb the resting, running, or repeating */
            disturb(0, 0);

            /* Redraw the state */
            p_ptr->redraw |= PR_STATE;
        }
    }


    /* Mega-Hack -- Random teleportation */
    if ((p_ptr->teleport) && (rand_int(100) == 0)) {

        teleport_flag = TRUE;
        teleport_dist = 40;
    }


    /* Repeat until out of energy */
    while (TRUE) {


        /* Notice death, teleport, and new levels */
        if (death || teleport_flag || new_level_flag) break;


        /* Special "commands" */
        if ((p_ptr->rest) ||
            (p_ptr->paralysis) ||
            (p_ptr->stun >= 100)) {

            /* Handle stuff */
            handle_stuff(TRUE);
            
            /* Hilite the player */
            move_cursor_relative(py, px);

            /* Refresh */
            Term_fresh();
            
            /* Turn is done */
            break;
        }


        /* Hack -- Notice "disturbances" */
        if (screen_change) {

            /* Forget the interuption */
            screen_change = FALSE;

            /* Handle stuff */
            handle_stuff(TRUE);
            
            /* Hilite the player */
            move_cursor_relative(py, px);

            /* Refresh */
            Term_fresh();
            
            /* Verify "continuing" command */
            if (command_new) {

                /* Hack -- flush input */
                Term_flush();

                /* Allow clean "cancel" XXX XXX XXX */
                if (!get_check("Continue with previous command?")) {

                    /* Cancel command */
                    command_new = 0;

                    /* Cancel "list" mode */
                    command_see = FALSE;
                }
            }
        }


        /* Commands are assumed to take time */
        energy_use = 100;

        /* Hack -- If running, run some more */
        if (find_flag) {

            /* Handle stuff */
            handle_stuff(TRUE);

            /* Take a step */
            find_step();

            /* Handle stuff */
            handle_stuff(TRUE);

            /* Hack -- Hilite the player */
            move_cursor_relative(py, px);
            
            /* Hack -- refresh */
            if (fresh_find) Term_fresh();
        }

        /* Else, get a command from the user */
        else {

            /* XXX XXX Mega-Hack -- update choice window */
            if (!choice_default) choice_inven(0, inven_ctr - 1);
            else choice_equip(INVEN_WIELD, INVEN_TOTAL - 1);

            /* Handle stuff */
            handle_stuff(TRUE);
            
            /* Optional fresh */
            if (fresh_before) Term_fresh();

            /* Get a command (new or old) */
            request_command();

            /* Process the command */
            process_command();

            /* Handle stuff */
            handle_stuff(TRUE);
            
            /* Optional fresh */
            if (fresh_after) Term_fresh();
        }


        /* Hack -- handle pack over-flow */
        if (inventory[INVEN_PACK].tval) {

            int		amt;

            char	prt1[160];

            /* Choose an item to spill */
            i = INVEN_PACK;

            /* Access the slot to be dropped */
            i_ptr = &inventory[i];

            /* Drop all of that item */
            amt = i_ptr->number;

            /* Warning */
            msg_print("Your pack overflows!");

            /* Message */
            objdes(prt1, i_ptr, TRUE);
            msg_print(format("You drop %s.", prt1));

            /* Drop it (carefully) near the player */
            drop_near(i_ptr, 0, py, px);

            /* Decrease the item, optimize. */
            inven_item_increase(i, -amt);
            inven_item_optimize(i);
        }


        /* Stop when energy is used */
        if (energy_use) break;
    }


    /* XXX XXX XXX Use a chunk of energy */
    p_ptr->energy -= 100;


    /* Hack -- notice death and new levels */
    if (death || new_level_flag) return;


    /* Process Teleportation */
    handle_teleport();
}



/*
 * This is the main function of this file -- it places the user on the
 * current level and processes user input until the level is completed,
 * the user dies, or the game is terminated.
 *
 * XXX XXX XXX Note that a lot of this function should be extracted
 * into a "process_player()" function (ala "process_monsters()") and
 * a lot of the "turn" code should be divided by about ten and made
 * to work on "game time" instead of "player time".  After all, for
 * many things, it is the "game turns" and not the "player turns" that
 * actually matter.  For example, regeneration of monsters.
 *
 * I *think* I have successfully pulled the player stuff into the
 * "process_player()" function, and changed "turn" so that it counts
 * "game turns".  This was simplified by assuming that things that
 * used to happen once per player turn should actually happen every
 * tenth game turn.  There are still more things that need extraction.
 */
void dungeon(void)
{
    int x, y;

    cave_type *c_ptr;

    inven_type *i_ptr;


    /* Reset flags and initialize variables (most of it is overkill) */
    new_level_flag	= FALSE;
    teleport_flag	= FALSE;
    find_flag		= 0;

    /* Reset the "command" vars (again, mostly overkill) */
    command_cmd		= 0;
    command_old		= 0;
    command_esc		= 0;
    command_rep		= 0;
    command_arg		= 0;
    command_dir		= -1;


    /* Cancel the target */
    target_who = 0;


#ifdef USE_MULTIHUED
    /* Forget all of the old Multi-Hued things */
    mh_forget();	
#endif


    /* Turn off searching */
    search_off();


    /* Remember deepest dungeon level visited */
    if (dun_level > (unsigned)(p_ptr->max_dlv)) {
        p_ptr->max_dlv = dun_level;
    }


    /* Paranoia -- no stairs from town */
    if (!dun_level) create_down_stair = create_up_stair = FALSE;

    /* No stairs down from Quest */
    if (is_quest(dun_level)) create_down_stair = FALSE;

    /* Make a stairway. */
    if (create_up_stair || create_down_stair) {

        /* Place a stairway */
        if (valid_grid(py, px)) {

            /* Delete the old object */
            delete_object(py, px);

            /* Access the grid */
            c_ptr = &cave[py][px];

            /* Make a new object */
            c_ptr->i_idx = i_pop();

            /* Access the object */
            i_ptr = &i_list[c_ptr->i_idx];

            /* Make it into a staircase */
            if (create_up_stair) {
                invcopy(i_ptr, OBJ_UP_STAIR);
            }
            else {
                invcopy(i_ptr, OBJ_DOWN_STAIR);
            }

            /* Save the location */
            i_ptr->iy = py;
            i_ptr->ix = px;

            /* Make it permanent */
            c_ptr->info |= GRID_PERM;
        }

        /* Cancel the stair request */
        create_down_stair = FALSE;
        create_up_stair = FALSE;
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


    /* Redraw stuff */
    p_ptr->redraw |= (PR_CAVE);

    /* Handle stuff */
    handle_stuff(TRUE);


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
    p_ptr->update |= (PU_DISTANCE);
    
    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
    
    /* Handle stuff */
    handle_stuff(TRUE);

    /* Refresh */
    Term_fresh();
    

    /* Announce (or repeat) the feeling, unless in town */
    if (dun_level) do_cmd_feeling();


    /*** Process this dungeon level ***/


    /* Main loop */
    while (!death) {


        /* Count game turns (not player turns!) */
        turn++;


        /*** Check the Time and Load ***/

        /* Only check occasionally */
        if (((turn % 1000) == 1) &&
             ((0 != check_time()) || (0 != check_load()))) {

            if (closing_flag > 2) {
                msg_print("The gates to ANGBAND are now closed.");
                (void)strcpy(died_from, "(closing gate: saved)");
                if (save_player()) quit(NULL);
                (void)strcpy(died_from, "a slammed gate");
                death = TRUE;
                exit_game();
            }

            else {
                disturb(0, 0);
                closing_flag++;
                msg_print("The gates to ANGBAND are closing...");
                msg_print("Please finish up and/or save your game.");
            }
        }


        /*** Update the Stores ***/

        /* XXX XXX XXX XXX Hack -- Daybreak/Nighfall in town */
        if (!dun_level && (turn % ((10L * TOWN_DAWN) / 2) == 0)) {

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


        /*** XXX XXX XXX XXX Update the Stores ***/

        /* Perhaps only/always do this at dawn? */
        /* Update the stores once a day */
        if ((dun_level) && ((turn % (10 * STORE_TURNS)) == 0)) {

            if (cheat_xtra) msg_print("Updating Stores...");

            store_maint();

            if (shuffle_owners && (rand_int(STORE_SHUFFLE) == 0)) {
                if (cheat_xtra) msg_print("Shuffling a Store...");
                store_shuffle();
            }

            if (cheat_xtra) msg_print("Done.");
        }



        /*** Process the player ***/

        /* Take damage XXX XXX XXX */
        if (p_ptr->poisoned) {

            int i = 0;

            /* Damage based on constitution */
            switch (con_adj()) {
                case -4: i = ((turn % 2) == 0); break;
                case -3:
                case -2: i = ((turn % 3) == 0); break;
                case -1: i = ((turn % 5) == 0); break;
                case 0: i = ((turn % 10) == 0); break;
                case 1:
                case 2:
                case 3: i = ((turn % 20) == 0); break;
                case 4:
                case 5: i = ((turn % 30) == 0); break;
                case 6: i = ((turn % 40) == 0); break;
                case 7: i = ((turn % 50) == 0); break;
                default: i = ((turn % 60) == 0); break;
            }

            /* Take damage */
            if (i) {
                take_hit(i, "poison");
                disturb(1, 0);
            }
        }


        /* Process the player */
        process_player();

        /* Handle stuff */
        handle_stuff(TRUE);
        
        /* Hack -- Notice death and new levels */
        if (death || new_level_flag) break;


        /*** Process the monsters ***/

        /* Check for creature generation */
        if (!(turn % 10) && (rand_int(MAX_M_ALLOC_CHANCE) == 0)) {
            alloc_monster(1, MAX_SIGHT + 5, FALSE);
        }

        /* XXX XXX XXX XXX Check for creature regeneration */
        if (!(turn % 200)) regen_monsters();

        /* Process all of the monsters */
        process_monsters();

        /* Handle stuff */
        handle_stuff(TRUE);


        /*** Process the objects ***/

        /* Try to prevent risky compaction */
        tighten_i_list();	
    }


    /* Handle stuff */
    handle_stuff(TRUE);

    /* Forget the old lite */
    forget_lite();

    /* Forget the old view */
    forget_view();

    /* Wipe all the items */
    wipe_i_list();

    /* Wipe all the monsters */
    wipe_m_list();

    /* Refresh */
    Term_fresh();
}


