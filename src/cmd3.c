/* File: cmd3.c */

/* Purpose: Inventory commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




/*
 * Move an item from equipment list to pack
 * Note that only one item at a time can be wielded per slot.
 * Note that taking off an item when "full" will cause that item
 * to fall to the ground.
 */
static void inven_takeoff(int item, int amt)
{
    int			posn;

    object_type		*i_ptr;
    object_type		tmp_obj;

    cptr		act;

    char		i_name[80];


    /* Get the item to take off */
    i_ptr = &inventory[item];

    /* Paranoia */
    if (amt <= 0) return;

    /* Verify */
    if (amt > i_ptr->number) amt = i_ptr->number;

    /* Make a copy to carry */
    tmp_obj = *i_ptr;
    tmp_obj.number = amt;

    /* What are we "doing" with the object */
    if (amt < i_ptr->number)
    {
        act = "Took off";
    }
    else if (item == INVEN_WIELD)
    {
        act = "Was wielding";
    }
    else if (item == INVEN_BOW)
    {
        act = "Was shooting with";
    }
    else if (item == INVEN_LITE)
    {
        act = "Light source was";
    }
    else
    {
        act = "Was wearing";
    }

    /* Carry the object, saving the slot it went in */
    posn = inven_carry(&tmp_obj);

    /* Describe the result */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("%^s %s (%c).", act, i_name, index_to_label(posn));

    /* Delete (part of) it */
    inven_item_increase(item, -amt);
    inven_item_optimize(item);
}




/*
 * Drops (some of) an item from inventory to "near" the current location
 */
static void inven_drop(int item, int amt)
{
    object_type		*i_ptr;
    object_type		 tmp_obj;

    cptr		act;

    char		i_name[80];


    /* Access the slot to be dropped */
    i_ptr = &inventory[item];

    /* Error check */
    if (amt <= 0) return;

    /* Not too many */
    if (amt > i_ptr->number) amt = i_ptr->number;

    /* Nothing done? */
    if (amt <= 0) return;

    /* Make a "fake" object */
    tmp_obj = *i_ptr;
    tmp_obj.number = amt;

    /* What are we "doing" with the object */
    if (amt < i_ptr->number)
    {
        act = "Dropped";
    }
    else if (item == INVEN_WIELD)
    {
        act = "Was wielding";
    }
    else if (item == INVEN_BOW)
    {
        act = "Was shooting with";
    }
    else if (item == INVEN_LITE)
    {
        act = "Light source was";
    }
    else if (item >= INVEN_WIELD)
    {
        act = "Was wearing";
    }
    else
    {
        act = "Dropped";
    }

    /* Message */
    object_desc(i_name, &tmp_obj, TRUE, 3);

    /* Message */
    msg_format("%^s %s (%c).", act, i_name, index_to_label(item));

    /* Drop it (carefully) near the player */
    drop_near(&tmp_obj, 0, py, px);

    /* Decrease the item, optimize. */
    inven_item_increase(item, -amt);
    inven_item_describe(item);
    inven_item_optimize(item);
}





/*
 * Display inventory
 */
void do_cmd_inven(void)
{
    char out_val[160];


    /* Note that we are in "inventory" mode */
    command_wrk = FALSE;


    /* Save the screen */
    Term_save();

    /* Hack -- show empty slots */
    item_tester_full = TRUE;

    /* Display the inventory */
    show_inven();

    /* Hack -- hide empty slots */
    item_tester_full = FALSE;

    /* Build a prompt */
    sprintf(out_val, "Inventory (carrying %d.%d pounds). Command: ",
            total_weight / 10, total_weight % 10);

    /* Get a command */
    prt(out_val, 0, 0);

    /* Get a new command */
    command_new = inkey();

    /* Restore the screen */
    Term_load();


    /* Process "Escape" */
    if (command_new == ESCAPE)
    {
        /* Reset stuff */
        command_new = 0;
        command_gap = 50;
    }

    /* Process normal keys */
    else
    {
        /* Hack -- Use "display" mode */
        command_see = TRUE;
    }
}


/*
 * Display equipment
 */
void do_cmd_equip(void)
{
    char out_val[160];


    /* Note that we are in "equipment" mode */
    command_wrk = TRUE;


    /* Save the screen */
    Term_save();

    /* Hack -- show empty slots */
    item_tester_full = TRUE;

    /* Display the equipment */
    show_equip();

    /* Hack -- undo the hack above */
    item_tester_full = FALSE;

    /* Build a prompt */
    sprintf(out_val, "Equipment (carrying %d.%d pounds). Command: ",
            total_weight / 10, total_weight % 10);

    /* Get a command */
    prt(out_val, 0, 0);

    /* Get a new command */
    command_new = inkey();

    /* Restore the screen */
    Term_load();


    /* Process "Escape" */
    if (command_new == ESCAPE)
    {
        /* Reset stuff */
        command_new = 0;
        command_gap = 50;
    }

    /* Process normal keys */
    else
    {
        /* Enter "display" mode */
        command_see = TRUE;
    }
}


/*
 * The "wearable" tester
 */
static bool item_tester_hook_wear(object_type *i_ptr)
{
    /* Check for a usable slot */
    if (wield_slot(i_ptr) >= INVEN_WIELD) return (TRUE);

    /* Assume not wearable */
    return (FALSE);
}


/*
 * Wield or wear a single item from the pack or floor
 */
void do_cmd_wield(void)
{
    int item, slot;
    object_type tmp_obj;
    object_type *i_ptr;

    cptr act;

    char i_name[80];


    /* Restrict the choices */
    item_tester_hook = item_tester_hook_wear;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Wear/Wield which item? ", FALSE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing you can wear or wield.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Check the slot */
    slot = wield_slot(i_ptr);

    /* Prevent wielding into a cursed slot */
    if (cursed_p(&inventory[slot]))
    {

        /* Describe it */
        object_desc(i_name, &inventory[slot], FALSE, 0);

        /* Message */
        msg_format("The %s you are %s appears to be cursed.",
                   i_name, describe_use(slot));

        /* Cancel the command */
        return;
    }

    /* Verify potential overflow */
    if ((inven_cnt >= INVEN_PACK) &&
        ((item < 0) || (i_ptr->number > 1)))
    {
        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack may overflow.  Continue? ")) return;
    }


    /* Take a turn */
    energy_use = 100;

    /* Get a copy of the object to wield */
    tmp_obj = *i_ptr;
    tmp_obj.number = 1;

    /* Decrease the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -1);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -1);
        floor_item_optimize(0 - item);
    }

    /* Access the wield slot */
    i_ptr = &inventory[slot];

    /* Take off the "entire" item if one is there */
    if (inventory[slot].k_idx) inven_takeoff(slot, 255);

    /*** Could make procedure "inven_wield()" ***/

    /* Wear the new stuff */
    *i_ptr = tmp_obj;

    /* Increase the weight */
    total_weight += i_ptr->weight;

    /* Increment the equip counter by hand */
    equip_cnt++;

    /* Where is the item now */
    if (slot == INVEN_WIELD)
    {
        act = "You are wielding";
    }
    else if (slot == INVEN_BOW)
    {
        act = "You are shooting with";
    }
    else if (slot == INVEN_LITE)
    {
        act = "Your light source is";
    }
    else
    {
        act = "You are wearing";
    }

    /* Describe the result */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("%^s %s (%c).", act, i_name, index_to_label(slot));

    /* Cursed! */
    if (cursed_p(i_ptr))
    {
        /* Warn the player */
        msg_print("Oops! It feels deathly cold!");

        /* Note the curse */
        i_ptr->ident |= ID_SENSE;
    }

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Recalculate torch */
    p_ptr->update |= (PU_TORCH);

    /* Recalculate mana */
    p_ptr->update |= (PU_MANA);

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);
}



/*
 * Take off an item
 */
void do_cmd_takeoff(void)
{
    int item;

    object_type *i_ptr;


    /* Verify potential overflow */
    if (inven_cnt >= INVEN_PACK)
    {
        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack may overflow.  Continue? ")) return;
    }


    /* Get an item (from equip) */
    if (!get_item(&item, "Take off which item? ", TRUE, FALSE, FALSE))
    {
        if (item == -2) msg_print("You are not wearing anything to take off.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Item is cursed */
    if (cursed_p(i_ptr))
    {
        /* Oops */
        msg_print("Hmmm, it seems to be cursed.");

        /* Nope */
        return;
    }


    /* Take a partial turn */
    energy_use = 50;

    /* Take off the item */
    inven_takeoff(item, 255);
}


/*
 * Drop an item
 */
void do_cmd_drop(void)
{
    int item, amt = 1;

    object_type *i_ptr;


    /* Get an item (from equip or inven) */
    if (!get_item(&item, "Drop which item? ", TRUE, TRUE, FALSE))
    {
        if (item == -2) msg_print("You have nothing to drop.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Cannot remove cursed items */
    if ((item >= INVEN_WIELD) && cursed_p(i_ptr))
    {
        /* Oops */
        msg_print("Hmmm, it seems to be cursed.");

        /* Nope */
        return;
    }


    /* See how many items */
    if (i_ptr->number > 1)
    {
        /* Get a quantity */
        amt = get_quantity(NULL, i_ptr->number);

        /* Allow user abort */
        if (amt <= 0) return;
    }


    /* Mega-Hack -- verify "dangerous" drops */
    if (cave[py][px].i_idx)
    {
        /* XXX XXX Verify with the player */
        if (other_query_flag &&
            !get_check("The item may disappear.  Continue? ")) return;
    }


    /* Take a partial turn */
    energy_use = 50;

    /* Drop (some of) the item */
    inven_drop(item, amt);
}



/*
 * Destroy an item
 */
void do_cmd_destroy(void)
{
    int			item, amt = 1;
    int			old_number;

    object_type		*i_ptr;

    char		i_name[80];

    char		out_val[160];


    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Destroy which item? ", FALSE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to destroy.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* See how many items */
    if (i_ptr->number > 1)
    {
        /* Get a quantity */
        amt = get_quantity(NULL, i_ptr->number);

        /* Allow user abort */
        if (amt <= 0) return;
    }


    /* Describe the object */
    old_number = i_ptr->number;
    i_ptr->number = amt;
    object_desc(i_name, i_ptr, TRUE, 3);
    i_ptr->number = old_number;

    /* Make a verification */
    sprintf(out_val, "Really destroy %s? ", i_name);
    if (!get_check(out_val)) return;


    /* Take a turn */
    energy_use = 100;

    /* Artifacts cannot be destroyed */
    if (artifact_p(i_ptr))
    {
        cptr feel = "special";

        /* Message */
        msg_format("You cannot destroy %s.", i_name);

        /* Hack -- Handle icky artifacts */
        if (cursed_p(i_ptr) || broken_p(i_ptr)) feel = "terrible";

        /* Hack -- inscribe the artifact */
        i_ptr->note = quark_add(feel);

        /* We have "felt" it (again) */
        i_ptr->ident |= (ID_SENSE);

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);

        /* Combine the pack */
        p_ptr->notice |= (PN_COMBINE);

        /* Done */
        return;
    }

    /* Message */
    msg_format("You destroy %s.", i_name);

    /* Eliminate the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -amt);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Eliminate the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -amt);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }
}


/*
 * Observe an item which has been *identify*-ed
 */
void do_cmd_observe(void)
{
    int			item;

    object_type		*i_ptr;

    char		i_name[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Examine which item? ", TRUE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to examine.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Require full knowledge */
    if (!(i_ptr->ident & ID_MENTAL))
    {
        msg_print("You have no special knowledge about that item.");
        return;
    }


    /* Description */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Describe */
    msg_format("Examining %s...", i_name);

    /* Describe it fully */
    if (!identify_fully_aux(i_ptr)) msg_print("You see nothing special.");
}



/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe(void)
{
    int   item;

    object_type *i_ptr;


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Un-inscribe which item? ", TRUE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to un-inscribe.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }

    /* Nothing to remove */
    if (!i_ptr->note)
    {
        msg_print("That item had no inscription to remove.");
        return;
    }

    /* Message */
    msg_print("Inscription removed.");

    /* Remove the incription */
    i_ptr->note = 0;

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Combine the pack */
    p_ptr->notice |= (PN_COMBINE);
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe(void)
{
    int			item;

    object_type		*i_ptr;

    char		i_name[80];

    char		out_val[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Inscribe which item? ", TRUE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have nothing to inscribe.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }

    /* Describe the activity */
    object_desc(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("Inscribing %s.", i_name);
    msg_print(NULL);

    /* Start with nothing */
    strcpy(out_val, "");

    /* Use old inscription */
    if (i_ptr->note)
    {
        /* Start with the old inscription */
        strcpy(out_val, quark_str(i_ptr->note));
    }

    /* Get a new inscription (possibly empty) */
    if (get_string("Inscription: ", out_val, 80))
    {
        /* Save the inscription */
        i_ptr->note = quark_add(out_val);

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);

        /* Combine the pack */
        p_ptr->notice |= (PN_COMBINE);
    }
}



/*
 * An "item_tester_hook" for refilling lanterns
 */
static bool item_tester_refill_lantern(object_type *i_ptr)
{
    /* Flasks of oil are okay */
    if (i_ptr->tval == TV_FLASK) return (TRUE);

    /* Torches are okay */
    if ((i_ptr->tval == TV_LITE) &&
        (i_ptr->sval == SV_LITE_LANTERN)) return (TRUE);

    /* Assume not okay */
    return (FALSE);
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(void)
{
    int item;

    object_type *i_ptr;
    object_type *j_ptr;


    /* Restrict the choices */
    item_tester_hook = item_tester_refill_lantern;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Refill with which flask? ", FALSE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have no flasks of oil.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Take a partial turn */
    energy_use = 50;

    /* Access the lantern */
    j_ptr = &inventory[INVEN_LITE];

    /* Refuel */
    j_ptr->pval += i_ptr->pval;

    /* Message */
    msg_print("You fuel your lamp.");

    /* Comment */
    if (j_ptr->pval >= FUEL_LAMP)
    {
        j_ptr->pval = FUEL_LAMP;
        msg_print("Your lamp is full.");
    }

    /* Decrease the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -1);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }

    /* Recalculate torch */
    p_ptr->update |= (PU_TORCH);
}



/*
 * An "item_tester_hook" for refilling torches
 */
static bool item_tester_refill_torch(object_type *i_ptr)
{
    /* Torches are okay */
    if ((i_ptr->tval == TV_LITE) &&
        (i_ptr->sval == SV_LITE_TORCH)) return (TRUE);

    /* Assume not okay */
    return (FALSE);
}


/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch(void)
{
    int item;

    object_type *i_ptr;
    object_type *j_ptr;


    /* Restrict the choices */
    item_tester_hook = item_tester_refill_torch;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Refuel with which torch? ", FALSE, TRUE, TRUE))
    {
        if (item == -2) msg_print("You have no extra torches.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        i_ptr = &i_list[0 - item];
    }


    /* Take a partial turn */	
    energy_use = 50;

    /* Access the primary torch */
    j_ptr = &inventory[INVEN_LITE];

    /* Refuel */
    j_ptr->pval += i_ptr->pval + 5;

    /* Message */
    msg_print("You combine the torches.");

    /* Over-fuel message */
    if (j_ptr->pval >= FUEL_TORCH)
    {
        j_ptr->pval = FUEL_TORCH;
        msg_print("Your torch is fully fueled.");
    }

    /* Refuel message */
    else
    {
        msg_print("Your torch glows more brightly.");
    }

    /* Decrease the item (from the pack) */
    if (item >= 0)
    {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else
    {
        floor_item_increase(0 - item, -1);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }

    /* Recalculate torch */
    p_ptr->update |= (PU_TORCH);
}




/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refill(void)
{
    object_type *i_ptr;

    /* Get the light */
    i_ptr = &inventory[INVEN_LITE];

    /* It is nothing */
    if (i_ptr->tval != TV_LITE)
    {
        msg_print("You are not wielding a light.");
    }

    /* It's a lamp */
    else if (i_ptr->sval == SV_LITE_LANTERN)
    {
        do_cmd_refill_lamp();
    }

    /* It's a torch */
    else if (i_ptr->sval == SV_LITE_TORCH)
    {
        do_cmd_refill_torch();
    }

    /* No torch to refill */
    else
    {
        msg_print("Your light cannot be refilled.");
    }
}



/*
 * Toggle "mode" for the "choice" window
 */
void do_cmd_toggle_choose(void)
{
    /* Hack -- flip the current status */
    choose_default = !choose_default;

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);
}





/*
 * Target command
 */
void do_cmd_target(void)
{
    /* Set the target */
    if (target_set())
    {
        msg_print("Target Selected.");
    }
    else
    {
        msg_print("Target Aborted.");
    }
}



/*
 * Look at a monster
 */
static cptr look_mon_desc(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    bool          living = TRUE;
    int           perc;


    /* Determine if the monster is "living" (vs "undead") */
    if (r_ptr->flags3 & RF3_UNDEAD) living = FALSE;
    if (r_ptr->flags3 & RF3_DEMON) living = FALSE;
    if (strchr("Egv", r_ptr->r_char)) living = FALSE;


    /* Healthy monsters */
    if (m_ptr->hp >= m_ptr->maxhp)
    {
        /* No damage */
        return (living ? "unhurt" : "undamaged");
    }


    /* Calculate a health "percentage" */
    perc = 100L * m_ptr->hp / m_ptr->maxhp;

    if (perc >= 60)
    {
        return (living ? "somewhat wounded" : "somewhat damaged");
    }

    if (perc >= 25)
    {
        return (living ? "wounded" : "damaged");
    }

    if (perc >= 10)
    {
        return (living ? "badly wounded" : "badly damaged");
    }

    return (living ? "almost dead" : "almost destroyed");
}





/*
 * Examine a grid, return a keypress.
 *
 * Assume the player is not blind or hallucinating.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * XXX XXX XXX We should allow the use of a "set target" command.
 *
 * XXX XXX XXX Note new terrain features for 2.8.0, and note that
 * eventually, we may allow objects and terrain features in the
 * same grid, or multiple objects per grid.
 *
 * We may assume that the grid is supposed to be "interesting".
 */
static int do_cmd_look_aux(int y, int x)
{
    cave_type		*c_ptr = &cave[y][x];

    object_type		*i_ptr = &i_list[c_ptr->i_idx];
    monster_type	*m_ptr = &m_list[c_ptr->m_idx];
    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    cptr		s1 = "You see ", s2 = "";

    bool		prep = FALSE;

    int			f = (c_ptr->ftyp);

    int			query = ' ';

    char		m_name[80];
    char		i_name[80];

    char		out_val[160];


    /* Hack -- looking under the player */
    if ((y == py) && (x == px)) s1 = "You are on ";


    /* Hack -- Convert secret doors to walls */
    if (f == 0x30) f = 0x38;

    /* Hack -- Convert invisible traps to floors */
    if (f == 0x02) f = 0x01;


    /* Actual monsters */
    if (c_ptr->m_idx && m_ptr->ml)
    {
        /* Get the monster name ("a kobold") */
        monster_desc(m_name, m_ptr, 0x08);

        /* Hack -- track this monster race */
        recent_track(m_ptr->r_idx);

        /* Hack -- health bar for this monster */
        health_track(c_ptr->m_idx);

        /* Hack -- handle stuff */
        handle_stuff();

        /* Describe, and prompt for recall */
        sprintf(out_val, "%s%s%s (%s) [(r)ecall]",
                s1, s2, m_name, look_mon_desc(c_ptr->m_idx));
        prt(out_val, 0, 0);

        /* Get a command */
        move_cursor_relative(y, x);
        query = inkey();

        /* Recall as needed */
        while (query == 'r')
        {
            /* Recall */
            Term_save();
            screen_roff(m_ptr->r_idx);
            Term_addstr(-1, TERM_WHITE, "  --pause--");
            query = inkey();
            Term_load();

            /* Continue if desired */
            if (query != ' ') return (query);

            /* Get a new command */
            move_cursor_relative(y, x);
            query = inkey();
        }

        /* Continue if allowed */
        if (query != ' ') return (query);

        /* Change the intro */
        s1 = "It is ";

        /* Hack -- take account of gender */
        if (r_ptr->flags1 & RF1_FEMALE) s1 = "She is ";
        else if (r_ptr->flags1 & RF1_MALE) s1 = "He is ";

        /* Use a preposition with objects */
        s2 = "on ";

        /* Use a preposition with terrain */
        prep = TRUE;

        /* Ignore floors */
        if (f == 0x01) f = 0;
    }


    /* Actual items */
    if (c_ptr->i_idx && i_ptr->marked)
    {
        /* Obtain an object description */
        object_desc(i_name, i_ptr, TRUE, 3);

        /* Describe the object */
        sprintf(out_val, "%s%s%s.  --pause--", s1, s2, i_name);
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Use "space" to advance */
        if (query != ' ') return (query);

        /* Change the intro */
        s1 = "It is ";

        /* Plurals */
        if (i_ptr->number > 1) s1 = "They are ";

        /* Use a preposition with terrain */
        prep = TRUE;

        /* Ignore floors */
        if (f == 0x01) f = 0;
    }


    /* Describe terrain (if memorized) */
    if (f && (c_ptr->fdat & CAVE_MARK))
    {
        cptr p1 = "";

        cptr p2 = "a ";

        cptr name = f_name + f_info[f].name;

        /* Pick a prefix */
        if (prep) p2 = ((f >= 0x20) ? "in " : "on ");

        /* Note leading vowel */
        if (is_a_vowel(name[0])) p2 = "an ";

        /* Hack -- store doors */
        if ((f >= 0x08) && (f <= 0x0F)) p2 = "the entrance to the ";

        /* Display a message */
        sprintf(out_val, "%s%s%s%s.  --pause--", s1, p1, p2, name);
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Continue if allowed */
        if (query != ' ') return (query);
    }


    /* Keep going */
    return (query);
}




/*
 * Hack -- determine if a given location is "interesting"
 */
static bool do_cmd_look_accept(int y, int x)
{
    cave_type *c_ptr;

    /* Examine the grid */
    c_ptr = &cave[y][x];

    /* Visible monsters */
    if (c_ptr->m_idx)
    {
        monster_type *m_ptr = &m_list[c_ptr->m_idx];

        /* Visible monsters */
        if (m_ptr->ml) return (TRUE);
    }

    /* Objects */
    if (c_ptr->i_idx)
    {
        object_type *i_ptr = &i_list[c_ptr->i_idx];

        /* Memorized object */
        if (i_ptr->marked) return (TRUE);
    }

    /* Memorized features (no floors) */
    if (c_ptr->fdat & CAVE_MARK)
    {
        /* Ignore floors and invisible traps */
        if (c_ptr->ftyp <= 0x02) return (FALSE);

        /* Notice doors, traps, stores, etc */
        if (c_ptr->ftyp <= 0x2F) return (TRUE);

        /* Ignore secret doors */
        if (c_ptr->ftyp <= 0x30) return (FALSE);

        /* Notice rubble */
        if (c_ptr->ftyp <= 0x31) return (TRUE);

        /* Ignore veins */
        if (c_ptr->ftyp <= 0x35) return (FALSE);

        /* Notice treasure veins */
        if (c_ptr->ftyp <= 0x37) return (TRUE);

        /* Ignore granite */
        return (FALSE);
    }

    /* Nope */
    return (FALSE);
}



/*
 * A new "look" command, similar to the "target" command.
 */
void do_cmd_look(void)
{
    int		i, d, m;

    bool	done = FALSE;

    char	query;


    /* Blind */
    if (p_ptr->blind)
    {
        msg_print("You can't see a damn thing!");
        return;
    }

    /* Hallucinating */
    if (p_ptr->image)
    {
        msg_print("You can't believe what you are seeing!");
        return;
    }


    /* Reset "temp" array */
    temp_n = 0;

    /* Collect viewable grids */
    for (i = 0; i < view_n; i++)
    {
        int x = view_x[i];
        int y = view_y[i];

        /* Skip off-screen locations */
        if (!panel_contains(y,x)) continue;

        /* Skip invalid locations */
        if (!do_cmd_look_accept(y,x)) continue;

        /* Save the location */
        temp_x[temp_n] = x;
        temp_y[temp_n] = y;
        temp_n++;
    }


    /* Nothing to see */
    if (!temp_n)
    {
        msg_print("You see nothing special.");
        return;
    }


    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_distance;
    ang_sort_swap = ang_sort_swap_distance;

    /* Sort the positions */
    ang_sort(temp_x, temp_y, temp_n);


    /* Start near the player */
    m = 0;

    /* Interact */
    while (!done)
    {
        /* Describe and Prompt */
        query = do_cmd_look_aux(temp_y[m], temp_x[m]);

        /* Assume no "direction" */
        d = 0;

        /* Analyze (non "recall") command */
        switch (query)
        {
            case ESCAPE:
            case 'q':
                done = TRUE;
                break;

            case ' ':
                if (++m == temp_n) m = 0;
                break;

            case '-':
                if (m-- == 0) m = temp_n - 1;
                break;

            case '1': case 'b': d = 1; break;
            case '2': case 'j': d = 2; break;
            case '3': case 'n': d = 3; break;
            case '4': case 'h': d = 4; break;
            case '6': case 'l': d = 6; break;
            case '7': case 'y': d = 7; break;
            case '8': case 'k': d = 8; break;
            case '9': case 'u': d = 9; break;

            default:
                bell();
        }

        /* Hack -- move around */
        if (d)
        {
            /* Find a new grid if possible */
            i = target_pick(temp_y[m], temp_x[m], ddy[d], ddx[d]);

            /* Use that grid */
            if (i >= 0) m = i;
        }
    }

    /* Clear the prompt */
    prt("", 0, 0);
}




/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate()
{
    int		dir, y1, x1, y2, x2;

    char	tmp_val[80];

    char	out_val[160];


    /* Start at current panel */
    y2 = y1 = panel_row;
    x2 = x1 = panel_col;

    /* Show panels until done */
    while (1)
    {
        /* Describe the location */
        if ((y2 == y1) && (x2 == x1))
        {
            tmp_val[0] = '\0';
        }
        else
        {
            sprintf(tmp_val, "%s%s of",
                    ((y2 < y1) ? " North" : (y2 > y1) ? " South" : ""),
                    ((x2 < x1) ? " West" : (x2 > x1) ? " East" : ""));
        }

        /* Prepare to ask which way to look */
        sprintf(out_val,
                "Map sector [%d,%d], which is%s your sector.  Direction?",
                y2, x2, tmp_val);

        /* Assume no direction */
        dir = 0;

        /* Get a direction */
        while (!dir)
        {
            char command;

            /* Get a command (or Cancel) */
            if (!get_com(out_val, &command)) break;

            /* Analyze the keypress */
            switch (command)
            {
                /* Convert roguelike directions */
                case 'B': case 'b': case '1': dir = 1; break;
                case 'J': case 'j': case '2': dir = 2; break;
                case 'N': case 'n': case '3': dir = 3; break;
                case 'H': case 'h': case '4': dir = 4; break;
                case 'L': case 'l': case '6': dir = 6; break;
                case 'Y': case 'y': case '7': dir = 7; break;
                case 'K': case 'k': case '8': dir = 8; break;
                case 'U': case 'u': case '9': dir = 9; break;
            }

            /* Error */
            if (!dir) bell();
        }

        /* No direction */
        if (!dir) break;

        /* Apply the motion */
        y2 += ddy[dir];
        x2 += ddx[dir];

        /* Verify the row */
        if (y2 > max_panel_rows) y2 = max_panel_rows;
        else if (y2 < 0) y2 = 0;

        /* Verify the col */
        if (x2 > max_panel_cols) x2 = max_panel_cols;
        else if (x2 < 0) x2 = 0;

        /* Handle "changes" */
        if ((y2 != panel_row) || (x2 != panel_col))
        {
            /* Save the new panel info */
            panel_row = y2;
            panel_col = x2;

            /* Recalculate the boundaries */
            panel_bounds();

            /* Update stuff */
            p_ptr->update |= (PU_MONSTERS);

            /* Redraw map */
            p_ptr->redraw |= (PR_MAP);

            /* Handle stuff */
            handle_stuff();
        }
    }


    /* Recenter the map around the player */
    verify_panel();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);

    /* Handle stuff */
    handle_stuff();
}






/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 */
static cptr ident_info[] =
{
    " :A dark grid",
    "!:A potion (or oil)",
    "\":An amulet (or necklace)",
    "#:A wall (or secret door)",
    "$:Treasure (gold or gems)",
    "%:A vein (magma or quartz)",
        /* "&:unused", */
    "':An open door",
    "(:Soft armor",
    "):A shield",
    "*:A vein with treasure",
    "+:A closed door",
    ",:Food (or mushroom patch)",
    "-:A wand (or rod)",
    ".:Floor",
    "/:A polearm (Axe/Pike/etc)",
        /* "0:unused", */
    "1:Entrance to General Store",
    "2:Entrance to Armory",
    "3:Entrance to Weaponsmith",
    "4:Entrance to Temple",
    "5:Entrance to Alchemy shop",
    "6:Entrance to Magic store",
    "7:Entrance to Black Market",
    "8:Entrance to your home",
        /* "9:unused", */
    "::Rubble",
    ";:A glyph of warding",
    "<:An up staircase",
    "=:A ring",
    ">:A down staircase",
    "?:A scroll",
    "@:You",
    "A:Angel",
    "B:Bird",
    "C:Canine",
    "D:Ancient Dragon/Wyrm",
    "E:Elemental",
    "F:Dragon Fly",
    "G:Ghost",
    "H:Hybrid",
    "I:Insect",
    "J:Snake",
    "K:Killer Beetle",
    "L:Lich",
    "M:Multi-Headed Reptile",
        /* "N:unused", */
    "O:Ogre",
    "P:Giant Humanoid",
    "Q:Quylthulg (Pulsing Flesh Mound)",
    "R:Reptile/Amphibian",
    "S:Spider/Scorpion/Tick",
    "T:Troll",
    "U:Major Demon",
    "V:Vampire",
    "W:Wight/Wraith/etc",
    "X:Xorn/Xaren/etc",
    "Y:Yeti",
    "Z:Zephyr Hound",
    "[:Hard armor",
    "\\:A hafted weapon (mace/whip/etc)",
    "]:Misc. armor",
    "^:A trap",
    "_:A staff",
        /* "`:unused", */
    "a:Ant",
    "b:Bat",
    "c:Centipede",
    "d:Dragon",
    "e:Floating Eye",
    "f:Feline",
    "g:Golem",
    "h:Hobbit/Elf/Dwarf",
    "i:Icky Thing",
    "j:Jelly",
    "k:Kobold",
    "l:Louse",
    "m:Mold",
    "n:Naga",
    "o:Orc",
    "p:Person/Human",
    "q:Quadruped",
    "r:Rodent",
    "s:Skeleton",
    "t:Townsperson",
    "u:Minor Demon",
    "v:Vortex",
    "w:Worm/Worm-Mass",
        /* "x:unused", */
    "y:Yeek",
    "z:Zombie/Mummy",
    "{:A missile (arrow/bolt/shot)",
    "|:An edged weapon (sword/dagger/etc)",
    "}:A launcher (bow/crossbow/sling)",
    "~:A tool (or miscellaneous item)",
    NULL
};



/*
 * Sorting hook -- Comp function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform on "u".
 */
static bool ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
    u16b *who = (u16b*)(u);

    u16b *why = (u16b*)(v);

    int w1 = who[a];
    int w2 = who[b];

    int z1, z2;


    /* Sort by player kills */
    if (*why >= 4)
    {
        /* Extract player kills */
        z1 = r_info[w1].r_pkills;
        z2 = r_info[w2].r_pkills;

        /* Compare player kills */
        if (z1 < z2) return (TRUE);
        if (z1 > z2) return (FALSE);
    }


    /* Sort by total kills */
    if (*why >= 3)
    {
        /* Extract total kills */
        z1 = r_info[w1].r_tkills;
        z2 = r_info[w2].r_tkills;

        /* Compare total kills */
        if (z1 < z2) return (TRUE);
        if (z1 > z2) return (FALSE);
    }


    /* Sort by monster level */
    if (*why >= 2)
    {
        /* Extract levels */
        z1 = r_info[w1].level;
        z2 = r_info[w2].level;

        /* Compare levels */
        if (z1 < z2) return (TRUE);
        if (z1 > z2) return (FALSE);
    }


    /* Sort by monster experience */
    if (*why >= 1)
    {
        /* Extract experience */
        z1 = r_info[w1].mexp;
        z2 = r_info[w2].mexp;

        /* Compare experience */
        if (z1 < z2) return (TRUE);
        if (z1 > z2) return (FALSE);
    }


    /* Compare indexes */
    return (w1 <= w2);
}


/*
 * Sorting hook -- Swap function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform.
 */
static void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
    u16b *who = (u16b*)(u);

    u16b holder;

    /* XXX XXX */
    v = v ? v : 0;

    /* Swap */
    holder = who[a];
    who[a] = who[b];
    who[b] = holder;
}



/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void roff_top(int r_idx)
{
    monster_race	*r_ptr = &r_info[r_idx];

    byte		a1, a2;
    char		c1, c2;


    /* Access the chars */
    c1 = r_ptr->r_char;
    c2 = r_ptr->l_char;

    /* Assume white */
    a1 = TERM_WHITE;
    a2 = TERM_WHITE;

#ifdef USE_COLOR

    /* Access the attrs */
    if (use_color)
    {
        a1 = r_ptr->r_attr;
        a2 = r_ptr->l_attr;
    }

#endif


    /* Clear the top line */
    Term_erase(0, 0, 80, 1);

    /* Reset the cursor */
    Term_gotoxy(0, 0);

    /* A title (use "The" for non-uniques) */
    if (!(r_ptr->flags1 & RF1_UNIQUE))
    {
        Term_addstr(-1, TERM_WHITE, "The ");
    }

    /* Dump the name */
    Term_addstr(-1, TERM_WHITE, (r_name + r_ptr->name));

    /* Append the "standard" attr/char info */
    Term_addstr(-1, TERM_WHITE, " ('");
    Term_addch(a1, c1);
    Term_addstr(-1, TERM_WHITE, "')");

    /* Append the "optional" attr/char info */
    Term_addstr(-1, TERM_WHITE, "/('");
    Term_addch(a2, c2);
    Term_addstr(-1, TERM_WHITE, "'):");
}


/*
 * Identify a character, allow recall of monsters
 *
 * Several "special" responses recall "mulitple" monsters:
 *   ^A (all monsters)
 *   ^U (all unique monsters)
 *   ^N (all non-unique monsters)
 *
 * The responses may be sorted in several ways, see below.
 *
 * Note that the player ghosts are ignored. XXX XXX XXX
 */
void do_cmd_query_symbol(void)
{
    int		i, n, r_idx;
    char	sym, query;
    char	buf[128];

    bool	all = FALSE;
    bool	uniq = FALSE;
    bool	norm = FALSE;

    bool	recall = FALSE;

    u16b	why = 0;
    u16b	who[MAX_R_IDX];


    /* Get a character, or abort */
    if (!get_com("Enter character to be identified: ", &sym)) return;

    /* Find that character info, and describe it */
    for (i = 0; ident_info[i]; ++i)
    {
        if (sym == ident_info[i][0]) break;
    }

    /* Describe */
    if (sym == KTRL('A'))
    {
        all = TRUE;
        strcpy(buf, "Full monster list.");
    }
    else if (sym == KTRL('U'))
    {
        all = uniq = TRUE;
        strcpy(buf, "Unique monster list.");
    }
    else if (sym == KTRL('N'))
    {
        all = norm = TRUE;
        strcpy(buf, "Non-unique monster list.");
    }
    else if (ident_info[i])
    {
        sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
    }
    else
    {
        sprintf(buf, "%c - %s.", sym, "Unknown Symbol");
    }

    /* Display the result */
    prt(buf, 0, 0);


    /* Collect matching monsters */
    for (n = 0, i = 1; i < MAX_R_IDX-1; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Nothing to recall */
        if (!cheat_know && !r_ptr->r_sights) continue;

        /* Require non-unique monsters if needed */
        if (norm && (r_ptr->flags1 & RF1_UNIQUE)) continue;

        /* Require unique monsters if needed */
        if (uniq && !(r_ptr->flags1 & RF1_UNIQUE)) continue;

        /* Collect "appropriate" monsters */
        if (all || (r_ptr->r_char == sym)) who[n++] = i;
    }

    /* Nothing to recall */
    if (!n) return;


    /* Prompt XXX XXX XXX */
    put_str("Recall details? (k/p/y/n): ", 0, 40);

    /* Query */
    query = inkey();

    /* Restore */
    prt(buf, 0, 0);


    /* Sort by kills (and level) */
    if (query == 'k')
    {
        why = 4;
        query = 'y';
    }

    /* Sort by level */
    if (query == 'p')
    {
        why = 2;
        query = 'y';
    }

    /* Catch "escape" */
    if (query != 'y') return;


    /* Sort if needed */
    if (why)
    {
        /* Select the sort method */
        ang_sort_comp = ang_sort_comp_hook;
        ang_sort_swap = ang_sort_swap_hook;

        /* Sort the array */
        ang_sort(who, &why, n);
    }


    /* Start at the end */
    i = n - 1;

    /* Scan the monster memory. */
    while ((0 <= i) && (i <= n - 1))
    {
        /* Validate index */
        for (i = i % n; i < 0; i += n) ;

        /* Extract a race */
        r_idx = who[i];

        /* Hack -- Auto-recall */
        recent_track(r_idx);

        /* Hack -- Handle stuff */
        handle_stuff();

        /* Hack -- Begin the prompt */
        roff_top(r_idx);

        /* Hack -- Complete the prompt */
        Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");

        /* Interact */
        while (1)
        {
            /* Recall */
            if (recall)
            {
                /* Save the screen */
                Term_save();

                /* Recall on screen */
                screen_roff(who[i]);

                /* Hack -- Complete the prompt (again) */
                Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");
            }

            /* Command */
            query = inkey();

            /* Unrecall */
            if (recall)
            {
                /* Restore */
                Term_load();
            }

            /* Normal commands */
            if (query != 'r') break;

            /* Toggle recall */
            recall = !recall;
        }

        /* Stop scanning */
        if (query == ESCAPE) break;

        /* Move to the "next" or "prev" monster */
        i = i + ((query == '-') ? 1 : -1);
    }


    /* Re-display the identity */
    prt(buf, 0, 0);
}


