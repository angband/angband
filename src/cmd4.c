/* File: cmd4.c */

/* Purpose: more low level code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




/*
 * Hack -- make sure we have a good "ANSI" definition for "CTRL()"
 */
#undef CTRL
#define CTRL(C) ((C)&037)






/*
 * Go up one level					-RAK-	
 */
void do_cmd_go_up(void)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = &cave[py][px];
    i_ptr = &i_list[c_ptr->i_idx];

    /* Verify stairs */
    if (i_ptr->tval != TV_UP_STAIR) {
        msg_print("I see no up staircase here.");
        energy_use = 0;
        return;
    }

    /* Success */
    msg_print("You enter a maze of up staircases.");
    
    /* Go up the stairs */
    dun_level--;
    new_level_flag = TRUE;

    /* Create a way back */
    create_down_stair = TRUE;
}


/*
 * Go down one level
 */
void do_cmd_go_down(void)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = &cave[py][px];
    i_ptr = &i_list[c_ptr->i_idx];

    if (i_ptr->tval != TV_DOWN_STAIR) {
        msg_print("I see no down staircase here.");
        energy_use = 0;
        return;
    }

    /* Success */
    msg_print("You enter a maze of down staircases.");
    
    /* Go down */
    dun_level++;
    new_level_flag = TRUE;

    /* Create a way back */
    create_up_stair = TRUE;
}


/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(void)
{
    /* Free turn */
    energy_use = 0;

    /* Flush input */
    flush();

    /* Verify */
    if (total_winner) {
        if (!get_check("Do you want to retire?")) return;
    }
    else {
        int i;
        if (!get_check("Do you really want to quit?")) return;
        flush();
        prt("Please verify QUITTING by typing the '@' sign: ", 0, 0);
        i = inkey();
        prt("", 0, 0);
        if (i != '@') return;
    }

    death = TRUE;
    
    (void)strcpy(died_from, "Quitting");
}


/*
 * Hack -- redraw the screen
 */
void do_cmd_redraw(void)
{
    /* Free command */
    energy_use = 0;

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw everything */
    p_ptr->redraw |= (PR_CAVE);

    /* Hack -- Redraw the recall and choice windows */
    p_ptr->redraw |= (PR_RECALL | PR_CHOICE);
    
    /* Handle stuff */
    handle_stuff();
    
    /* Hack -- Redraw physically */
    Term_redraw();

    /* Hack -- Flush the output */
    Term_fresh();
}


/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
    energy_use = 0;

    save_screen();
    change_name();
    restore_screen();
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(void)
{
    energy_use = 0;

    if (p_ptr->searching) {
        search_off();
    }
    else {
        search_on();
    }
}



/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(void)
{
    int item;

    inven_type *i_ptr;
    inven_type *j_ptr;


    /* Assume this will be free */
    energy_use = 0;

    /* Access the lantern */
    j_ptr = &inventory[INVEN_LITE];


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Restrict the choices to flasks */
    item_tester_tval = TV_FLASK;

    /* Get a flask to refuel with */
    if (!get_item(&item, "Refill with which flask? ", 0, inven_ctr - 1, TRUE)) {
        if (item == -2) msg_print("You have no flasks of oil.");
        return;
    }
    

    /* Access the item */
    if (item >= 0) i_ptr = &inventory[item];

    
    /* Take a turn */	
    energy_use = 100;

    /* Refuel */
    j_ptr->pval += i_ptr->pval;

    /* Comment */
    if (j_ptr->pval < FUEL_LAMP / 2) {
        msg_print("Your lamp is less than half full.");
    }
    else if (j_ptr->pval < FUEL_LAMP / 2 + FUEL_LAMP / 16) {
        msg_print("Your lamp is about half full.");
    }
    else if (j_ptr->pval < FUEL_LAMP) {
        msg_print("Your lamp is more than half full.");
    }
    else {
        j_ptr->pval = FUEL_LAMP;
        msg_print("Your lamp overflows, spilling oil on the ground.");
        msg_print("Your lamp is full.");
    }

    /* Decrease the item (from the pack) */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else {
        floor_item_increase(py, px, -1);
        floor_item_optimize(py, px);
    }
}




/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch(void)
{
    int item;

    inven_type *i_ptr;
    inven_type *j_ptr;


    /* Assume this will be free */
    energy_use = 0;

    /* Access the primary torch */
    j_ptr = &inventory[INVEN_LITE];


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Restrict the choices */
    item_tester_tval = TV_LITE;
    item_tester_sval = SV_LITE_TORCH;

    /* Get a torch to refuel with */
    if (!get_item(&item, "Refuel with which torch? ", 0, inven_ctr - 1, TRUE)) {
        if (item == -2) msg_print("You have no extra torches.");
        return;
    }
    
    /* Cancel "auto-see" */
    command_see = FALSE;
    
    /* Access the item */
    if (item >= 0) i_ptr = &inventory[item];


    /* Take a turn */	
    energy_use = 100;

    /* Refuel */
    j_ptr->pval += i_ptr->pval;

    /* Message */
    msg_print("You combine the torches.");

    /* Over-fuel message */
    if (j_ptr->pval > FUEL_TORCH) {
        j_ptr->pval = FUEL_TORCH;
        msg_print("Your torch fully fueled.");
    }

    /* Refuel message */
    else {
        msg_print("Your torch glows more brightly.");
    }

    /* Decrease the item (from the pack) */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else {
        floor_item_increase(py, px, -1);
        floor_item_optimize(py, px);
    }

    /* Cancel "auto-see" */
    command_see = FALSE;
}




/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refill(void)
{
    inven_type *i_ptr;

    /* Assume a free turn */
    energy_use = 0;

    /* Get the light */
    i_ptr = &inventory[INVEN_LITE];

    /* It is nothing */
    if (i_ptr->tval != TV_LITE) {
        msg_print("You are not wielding a light.");
    }

    /* It's a lamp */
    else if (i_ptr->sval == SV_LITE_LANTERN) {
        do_cmd_refill_lamp();
    }

    /* It's a torch */
    else if (i_ptr->sval == SV_LITE_TORCH) {
        do_cmd_refill_torch();
    }

    /* No torch to refill */
    else {
        msg_print("Your light cannot be refilled.");
    }
}



/*
 * Support code for the "CTRL('P')" recall command
 *
 * If "command_arg" is set, recall that many messages.
 * Otherwise, recall until there are no more messages.
 *
 * The screen format uses line 0 for prompts, skips line 1,
 * uses line 2 and 23 for "[continued]" messages, and uses
 * lines 3 to 22 for the display of old messages.
 */
void do_cmd_messages(void)
{
    int i = 0, j = 0, n = 0, k = 0;

    cptr pmt1 = "[... older messages continued above, use ^P to scroll ...]";
    cptr pmt2 = "[... newer messages continued below, use ^N to scroll ...]";

    /* Free move */
    energy_use = 0;

    /* Assume "full recall" */
    n = message_num();

    /* Hack -- no messages */
    if (n <= 0) {
        prt("There are no messages to recall.", 0, 0);
        return;
    }

    /* Hack -- allow "quick recall" of a single message */
    if ((!command_arg) && (command_old != CTRL('P'))) n = 1;

    /* Hack -- quick recall of simple messages */
    if (n <= 1) {
        put_str(">", 0, 0);
        prt(message_str(0), 0, 1);
        return;
    }

    /* Save the screen */
    save_screen();

    /* Enter "icky" mode */
    character_icky = TRUE;
    
    /* Hack -- force "clear" */
    k = ' ';

    /* Process requests until done */
    while (1) {

        /* Only redraw if needed */
        if (k) {

            /* Clear the screen */
            clear_screen();

            /* Dump the current batch of messages */
            for (j = 0; j < 20 && (i+j < n); j++) {

                /* Dump the messages, bottom to top */
                put_str(message_str(i+j), 22-j, 0);
            }

            /* Indicate extra messages */
            if (i + 20 < n) prt(pmt1, 2, 0);

            /* Indicate extra messages */
            if (i > 0) prt(pmt2, 23, 0);
        }

        /* Display a simple prompt */
        prt("Message Recall [press ESCAPE to exit] ", 0, 0);

        /* Get a command */
        k = inkey();

        /* Exit on Escape */
        if (k == ESCAPE) break;

        /* Save the old index */
        j = i;

        /* Recall more older messages */
        if ((k == 'p') || (k == CTRL('P'))) {
            if (i+20 < n) i += 20;
        }

        /* Recall more newer messages */
        if ((k == 'n') || (k == CTRL('N'))) {
            if (i >= 20) i -= 20;
        }

        /* Error of some kind, do not redraw */
        if (i == j) {
            k = 0;
            bell();
        }
    }

    /* Leave "icky" mode */
    character_icky = FALSE;
    
    /* Restore the screen */
    restore_screen();
}


/*
 * Target command
 */
void do_cmd_target(void)
{
    /* Free move */
    energy_use = 0;

#ifdef ALLOW_TARGET

    /* Set the target */
    if (target_set()) {
        msg_print("Target Selected");
    }
    else {
        msg_print("Target Aborted");
    }

#else

    /* Oops */
    msg_print("Target code not compiled in this version.");

#endif

}




/*
 * A simple structure to hold some options
 */
typedef struct _opt_desc {
    cptr	o_prompt;
    bool	*o_var;
} opt_desc;


/*
 * General User-Interface Options
 */
static opt_desc options_interface[] = {

    { "Rogue-like commands", 				&rogue_like_commands },
    { "Quick messages",					&quick_messages },
    { "Prompt for various information", 		&other_query_flag },
    { "Prompt before picking things up", 		&carry_query_flag },
    { "Use old target by default",			&use_old_target },
    { "Pick things up by default", 			&always_pickup },
    { "Accept all throw commands",			&always_throw },
    { "Repeat obvious commands",			&always_repeat },

    { "Plain object descriptions",			&plain_descriptions },
    { "Display Equippy Characters",			&equippy_chars },
    { "Display Monster Health Bar",			&show_health_bar },
    { "Show extra spell info",				&show_spell_info },
    { "Show dungeon level in feet",			&depth_in_feet },
    { "Notice mineral seams", 				&notice_seams },

    { "Compress savefiles",				&compress_savefile },
    { "Ring bell on error",				&ring_bell },

    { "Draw Torch-Lite in yellow (slow)",		&view_yellow_lite },
    { "Draw Viewable Lite brightly (v.slow)",		&view_bright_lite },
    { "Hilite the player with the cursor",		&hilite_player },
    { "Use color if possible (slow)",			&use_color },

    { NULL,						NULL }
};


/*
 * Disturbance Options -- for running/resting
 */
static opt_desc options_disturb[] = {

    { "Cut known corners",	 			&find_cut },
    { "Examine potential corners",			&find_examine },
    { "Print self during run (slow)",			&find_prself },
    { "Stop when map sector changes",			&find_bound },
    { "Run through open doors", 			&find_ignore_doors },
    { "Run past stairs", 				&find_ignore_stairs },

    { "Monster moving nearby disturbs me",		&disturb_near },
    { "Monster moving anywhere disturbs me",		&disturb_move },
    { "Monster appearance disturbs me",			&disturb_enter },
    { "Monster disappearance disturbs me",		&disturb_leave },

    { "Flush input before normal commands",		&flush_command },
    { "Flush input whenever disturbed",			&flush_disturb },
    { "Flush input on various failures",		&flush_failure },

    { "Flush output before all commands",		&fresh_before },
    { "Flush output after all commands",		&fresh_after },
    { "Flush output while running (slow)",		&fresh_find },

    { NULL,						NULL }
};


/*
 * Inventory Options -- these slightly affect game-play
 */
static opt_desc options_inventory[] = {

    { "Use Recall window (if available)",		&use_recall_win },
    { "Use Choice window (if available)",		&use_choice_win },

    { "Recall monster descriptions",			&recall_show_desc },
    { "Recall monster kill counts",			&recall_show_kill },

    { "Show spells in choice window",			&choice_show_spells },
    { "Show info in choice window",			&choice_show_info },
    { "Show labels in choice window",			&choice_show_label },
    { "Show weights in choice window",			&choice_show_weight },

    { "Show weights in inventory list",			&show_inven_weight },
    { "Show weights in equipment list",			&show_equip_weight },
    { "Show weights in stores",				&show_store_weight },

    { "Unused option",					&unused_option },

    { "Allow weapons and armor to stack",		&stack_allow_items },
    { "Allow wands/staffs/rods to stack",		&stack_allow_wands },
    { "Over-ride inscriptions when stacking",		&stack_force_notes },
    { "Over-ride discounts when stacking",		&stack_force_costs },

    { "Disable haggling in stores",			&no_haggle_flag },
    { "Shuffle store owners",				&shuffle_owners },

    { NULL,						NULL}
};


/*
 * Gameplay Options -- these actually affect game-play
 */
static opt_desc options_gameplay[] = {

    { "Map remembers all illuminated walls",		&view_wall_memory },
    { "Map remembers all important stuff",		&view_xtra_memory },
    { "Map remembers all perma-lit grids",		&view_perma_grids },
    { "Map remembers all torch-lit grids",		&view_torch_grids },

    { "Reduce view-radius when running",		&view_reduce_view },
    { "Reduce lite-radius when running",		&view_reduce_lite },

    { "Create characters in 'maximize' mode",		&begin_maximize },
    { "Create characters in 'preserve' mode",		&begin_preserve },

    { "Generate dungeons with aligned rooms",		&dungeon_align },
    { "Generate dungeons with connected stairs",	&dungeon_stair },

#ifdef MONSTER_FLOW
    { "Monsters chase current location (v.slow)",	&flow_by_sound },
    { "Monsters chase recent locations (v.slow)",	&flow_by_smell },
#endif

#ifdef WDT_TRACK_OPTIONS
    { "Monsters follow the player (beta)",		&track_follow },
    { "Monsters target the player (beta)",		&track_target },
#endif

#ifdef DRS_SMART_OPTIONS
    { "Monsters learn from their mistakes",		&smart_learn },
    { "Monsters exploit players weaknesses",		&smart_cheat },
#endif

    { NULL,						NULL}
};


/*
 * Cheating Options -- go ahead and cheat
 */
static opt_desc options_cheating[] = {

    { "Peek into object creation",		&cheat_peek },
    { "Peek into monster creation",		&cheat_hear },
    { "Peek into dungeon creation",		&cheat_room },
    { "Peek into something else",		&cheat_xtra },

    { "Know complete monster info",		&cheat_know },
    { "Allow player to avoid death",		&cheat_live },

    { NULL,					NULL}
};


/*
 * Set or unset various boolean options.
 */
static void do_cmd_options_aux(opt_desc *options, cptr info)
{
    int		i, max, ch;

    char	pmt[80];
    char	dat[80];


    /* Clear the screen */
    clear_screen();

    /* Prompt */
    sprintf(pmt, "%s (RET to advance, y/n to set, ESC to accept) ", info);
    prt(pmt, 0, 0);

    /* Prepare the screen, Count the options. */
    for (max = 0; options[max].o_prompt; max++) {
        sprintf(dat, "%-48s: %s ", options[max].o_prompt,
                (*options[max].o_var ? "yes" : "no "));
        prt(dat, max + 2, 0);
    }

    /* Start at the first option */
    i = 0;

    /* Interact with the player */
    for (;;) {
        move_cursor(i + 2, 50);
        ch = inkey();
        switch (ch) {
          case ESCAPE:	
            return;
          case '-':
          case '8':
            i = (max + i - 1) % max;
            break;
          case ' ':
          case '\n':
          case '\r':
          case '2':
            i = (i + 1) % max;
            break;
          case 'y':
          case 'Y':
          case '6':
            put_str("yes ", i + 2, 50);
            *options[i].o_var = TRUE;
            i = (i + 1) % max;
            break;
          case 'n':
          case 'N':
          case '4':
            put_str("no  ", i + 2, 50);
            *options[i].o_var = FALSE;
            i = (i + 1) % max;
            break;
          default:
            bell();
            break;
        }
    }
}


/*
 * Set or unset various options.  Redraw screen when done.
 */
void do_cmd_options(void)
{
    int i;


    /* Save the screen */
    save_screen();

    /* Enter "icky" mode */
    character_icky = TRUE;
    

    /* Interact */
    while (1) {

        clear_screen();

        /* Give some choices */
        prt("(1) User Interface Options", 2, 5);
        prt("(2) Disturbance Options", 3, 5);
        prt("(3) Inventory Options", 4, 5);
        prt("(4) Game-Play Options", 5, 5);
        prt("(5) Base Delay Speed", 6, 5);
        prt("(6) Hitpoint Warning", 7, 5);

        /* Cheating */
        if (can_be_wizard) prt("(C) Cheating Options", 8, 5);

        /* Ask for a choice */
        prt("Angband Options (1-6 or ESC to exit) ", 0, 0);
        i = inkey();

        /* Exit */
        if (i == ESCAPE) break;

        /* General Options */
        if (i == '1') {

            /* Process the general options */
            do_cmd_options_aux(options_interface, "User Interface Options");
        }

        /* Disturbance Options */
        else if (i == '2') {

            /* Process the running options */
            do_cmd_options_aux(options_disturb, "Disturbance Options");
        }

        /* Inventory Options */
        else if (i == '3') {

            /* Process the running options */
            do_cmd_options_aux(options_inventory, "Inventory Options");
        }

        /* Gameplay Options */
        else if (i == '4') {

            /* Process the game-play options */
            do_cmd_options_aux(options_gameplay, "Game-Play Options");
        }

        /* Hack -- Delay Speed */
        else if (i == '5') {

            clear_screen();

            /* Get a new value */
            while (1) {
                char buf[128];
                sprintf(buf, "Current delay speed: %d milliseconds", delay_spd);
                prt(buf, 5, 5);
                prt("Delay Speed (0-9 or ESC to accept) ", 0, 0);
                i = inkey();
                if (i == ESCAPE) break;
                if (isdigit(i)) delay_spd = (i - '0');
                else bell();
            }
        }

        /* Hack -- hitpoint warning factor */
        else if (i == '6') {

            clear_screen();

            /* Get a new value */
            while (1) {
                char buf[128];
                sprintf(buf, "Current hitpoint warning: %d0%%", hitpoint_warn);
                prt(buf, 5, 5);
                prt("Hitpoint Warning (0-9 or ESC to accept) ", 0, 0);
                i = inkey();
                if (i == ESCAPE) break;
                if (isdigit(i)) hitpoint_warn = (i - '0');
                else bell();
            }
        }

        /* Cheating Options */
        else if ((i == 'C') && can_be_wizard) {

            /* Process the cheating options */
            do_cmd_options_aux(options_cheating, "Cheating Options");

            /* Hack -- note use of "cheat" options */
            if (cheat_peek) noscore |= 0x0100;
            if (cheat_hear) noscore |= 0x0200;
            if (cheat_room) noscore |= 0x0400;
            if (cheat_xtra) noscore |= 0x0800;
            if (cheat_know) noscore |= 0x1000;
            if (cheat_live) noscore |= 0x2000;
        }

        /* Unknown option */
        else {
            bell();
        }
    }


    /* Leave "icky" mode */
    character_icky = FALSE;
    
    /* Restore the screen */
    restore_screen();


    /* Verify the keymap */
    keymap_init();
    
    
    /* Mega-Hack -- React */
    Term_xtra(TERM_XTRA_REACT, 0);


    /* XXX XXX Mega-Hack (see "birth.c") */
    if (!character_generated) return;


    /* Free turn */
    energy_use = 0;

    /* Combine the pack */
    combine_pack();

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw the recall and choice windows */
    p_ptr->redraw |= (PR_RECALL | PR_CHOICE);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_CAVE);
}



/*
 * Append monster attr/char definitions to the given file
 */
static void pref_dump_race(cptr fname)
{
    int i;
    FILE *fff;

#ifdef MACINTOSH
    _ftype = 'TEXT';
#endif

    /* Append to the file */
    fff = my_tfopen(fname, "a");

    /* Failure */
    if (!fff) return;

    /* Start dumping */
    fprintf(fff, "\n\n# Monster attr/char definitions\n\n");

    /* Dump them (including ghost) */
    for (i = 1; i < MAX_R_IDX; i++) {

        monster_lore *l_ptr = &l_list[i];

        /* Dump the monster attr/char info */
        fprintf(fff, "R:%d:%d/%d\n", i, l_ptr->l_attr, (byte)(l_ptr->l_char));
    }

    /* Start dumping */
    fprintf(fff, "\n\n\n\n");

    /* Close */
    fclose(fff);
}


/*
 * Append object attr/char definitions to the given file
 */
static void pref_dump_kind(cptr fname)
{
    int i;
    FILE *fff;

#ifdef MACINTOSH
    _ftype = 'TEXT';
#endif

    /* Append to the file */
    fff = my_tfopen(fname, "a");

    /* Failure */
    if (!fff) return;

    /* Start dumping */
    fprintf(fff, "\n\n# Object attr/char definitions\n\n");

    /* Dump them */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Dump the monster attr/char info */
        fprintf(fff, "K:%d:%d/%d\n", i, x_list[i].x_attr, (byte)(x_list[i].x_char));
    }

    /* Start dumping */
    fprintf(fff, "\n\n\n\n");

    /* Close */
    fclose(fff);
}


/*
 * Interact with the "pref" files
 */
void do_cmd_prefs(void)
{
    int i;

    char buf[1024];


    /* Drop priv's */
    safe_setuid_drop();

    /* Enter "icky" mode */
    character_icky = TRUE;
    
    /* Interact until done */
    while (1) {

        /* Clear the screen */
        clear_screen();

        /* Give some choices */
        prt("(0) Reset visuals", 2, 5);
        prt("(1) Load a 'pref' file", 3, 5);
        prt("(2) Append macros to a 'pref' file", 4, 5);
        prt("(3) Append something -- not ready yet", 5, 5);
        prt("(4) Append monster attr/chars to a 'pref' file", 6, 5);
        prt("(5) Append object attr/chars to a 'pref' file", 7, 5);
        prt("(6) Change monster attr/chars", 8, 5);
        prt("(7) Change object attr/chars", 9, 5);

#if 0
        prt("(8) Change tval_to_attr[] -- not ready yet", 10, 5);
        prt("(9) Change tval_to_char[] -- not ready yet", 11, 5);
#endif

        /* Ask for a choice */
        prt("Angband Preferences (0-7 or ESC to exit) ", 0, 0);
        i = inkey();
        if (i == ESCAPE) break;

        /* Reset visuals */
        if (i == '0') {
        
            /* Reset */
            reset_visuals();
            
            /* Message */
            msg_print("Reset visual attr/char tables.");
        }
        
        /* Load a 'pref' file */
        else if (i == '1') {

            /* Clear */
            clear_screen();

            /* Info */
            prt("Load an existing 'pref' file.", 0, 0);

            /* Get a filename, handle ESCAPE */
            prt("File: ", 5, 0);
            strcpy(buf, "pref.prf");
            if (!askfor_aux(buf, 70)) continue;

            /* Process the given filename */
            if (process_pref_file(buf)) msg_print("Error...");
        }

        /* Dump macros */
        else if (i == '2') {

            /* Clear */
            clear_screen();

            /* Info */
            prt("Appends all current macros to a file.", 0, 0);

            /* Get a filename, dump the macros to it */
            prt("File: ", 5, 0);
            strcpy(buf, "macro.prf");
            if (!askfor_aux(buf, 70)) continue;

            /* Dump the macros */
            macro_dump(buf);
        }

        /* Dump monster attr/chars */
        else if (i == '4') {

            /* Clear */
            clear_screen();

            /* Info */
            prt("Append all monster attr/char data to a file.", 0, 0);

            /* Get a filename, dump to it */
            prt("File: ", 5, 0);
            strcpy(buf, "race.prf");
            if (!askfor_aux(buf, 70)) continue;

            /* Hack -- open the file */
            pref_dump_race(buf);
        }

        /* Dump object attr/chars */
        else if (i == '5') {

            /* Clear */
            clear_screen();

            /* Info */
            prt("Append all object attr/char data to a file.", 0, 0);

            /* Get a filename, dump to it */
            prt("File: ", 5, 0);
            strcpy(buf, "kind.prf");
            if (!askfor_aux(buf, 70)) continue;

            /* Hack -- open the file */
            pref_dump_kind(buf);
        }

        /* Hack -- monsters */
        else if (i == '6') {

            /* Hack -- start on "nobody" */
            static int r_idx = 0;

            /* Clear the screen */
            clear_screen();

            /* Hack -- query until done */
            while (1) {

                int r = r_idx;

                monster_race *r_ptr = &r_list[r];
                monster_lore *l_ptr = &l_list[r];

                int da = (byte)(r_ptr->r_attr);
                int dc = (byte)(r_ptr->r_char);
                int ca = (byte)(l_ptr->l_attr);
                int cc = (byte)(l_ptr->l_char);

                /* Label the object */
                Term_putstr(5, 10, -1, TERM_WHITE,
                            format("Monster = %d, Name = %-40.40s",
                                   r, r_ptr->name));

                /* Label the Default values */
                Term_putstr(10, 12, -1, TERM_WHITE,
                            format("Default attr/char = %3u / %3u", da, dc));
                Term_putstr(40, 12, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 12, da, dc);

                /* Label the Current values */
                Term_putstr(10, 14, -1, TERM_WHITE,
                            format("Current attr/char = %3u / %3u", ca, cc));
                Term_putstr(40, 14, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 14, ca, cc);

                /* Directions */
                Term_putstr(5, 2, -1, TERM_WHITE, "n/N = change index");
                Term_putstr(5, 3, -1, TERM_WHITE, "a/A = change attr");
                Term_putstr(5, 4, -1, TERM_WHITE, "c/C = change char");

                /* Prompt */
                Term_putstr(0, 0, -1, TERM_WHITE,
                            "Object attr/char (n/N/a/A/c/C): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') r_idx = (r_idx + MAX_R_IDX + 1) % MAX_R_IDX;
                if (i == 'N') r_idx = (r_idx + MAX_R_IDX - 1) % MAX_R_IDX;
                if (i == 'a') l_ptr->l_attr = (byte)(ca + 1);
                if (i == 'A') l_ptr->l_attr = (byte)(ca - 1);
                if (i == 'c') l_ptr->l_char = (byte)(cc + 1);
                if (i == 'C') l_ptr->l_char = (byte)(cc - 1);
            }
        }

        /* Hack -- Redefine object attr/chars */
        else if (i == '7') {

            /* Hack -- start on "nothing" */
            static int k_idx = 0;

            /* Clear the screen */
            clear_screen();

            /* Hack -- query until done */
            while (1) {

                int k = k_idx;

                int da = (byte)k_list[k].k_attr;
                int dc = (byte)k_list[k].k_char;
                int ca = (byte)x_list[k].x_attr;
                int cc = (byte)x_list[k].x_char;

                /* Label the object */
                Term_putstr(5, 10, -1, TERM_WHITE,
                            format("Object = %d, Name = %-40.40s",
                                   k, k_list[k].name));

                /* Label the Default values */
                Term_putstr(10, 12, -1, TERM_WHITE,
                            format("Default attr/char = %3d / %3d", da, dc));
                Term_putstr(40, 12, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 12, da, dc);

                /* Label the Current values */
                Term_putstr(10, 14, -1, TERM_WHITE,
                            format("Current attr/char = %3d / %3d", ca, cc));
                Term_putstr(40, 14, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 14, ca, cc);

                /* Directions */
                Term_putstr(5, 2, -1, TERM_WHITE, "n/N = change index");
                Term_putstr(5, 3, -1, TERM_WHITE, "a/A = change attr");
                Term_putstr(5, 4, -1, TERM_WHITE, "c/C = change char");

                /* Prompt */
                Term_putstr(0, 0, -1, TERM_WHITE,
                            "Object attr/char (n/N/a/A/c/C): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') k_idx = (k_idx + MAX_K_IDX + 1) % MAX_K_IDX;
                if (i == 'N') k_idx = (k_idx + MAX_K_IDX - 1) % MAX_K_IDX;
                if (i == 'a') x_list[k].x_attr = (byte)(ca + 1);
                if (i == 'A') x_list[k].x_attr = (byte)(ca - 1);
                if (i == 'c') x_list[k].x_char = (byte)(cc + 1);
                if (i == 'C') x_list[k].x_char = (byte)(cc - 1);
            }
        }

        /* Unknown option */
        else {
            bell();
        }
    }

    /* Leave "icky" mode */
    character_icky = FALSE;
    
    /* Grab priv's */
    safe_setuid_grab();

    /* Free turn */
    energy_use = 0;

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_CAVE);
}


/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
    char buf[80];

    /* Free turn */
    energy_use = 0;

    /* Get a "note" */
    prt("Note: ", 0, 0);
    if (!askfor(buf, 60)) return;
    
    /* Add the note to the message recall */
    msg_format("Note: %s", buf);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
    /* Free turn */
    energy_use = 0;
    
    /* Silly message */
    msg_format("You are playing Angband %d.%d.%d.  Type '?' for more info.",
               CUR_VERSION_MAJ, CUR_VERSION_MIN, CUR_PATCH_LEVEL);
}



/*
 * Save the game
 */
void do_cmd_save_game(void)
{
    /* Hack -- free turn */
    energy_use = 0;
            
    /* Disturb the player */
    disturb(1, 0);

    /* Clear messages */
    msg_print(NULL);
    
    /* Handle stuff */
    handle_stuff();

    /* Message */
    prt("Saving game...", 0, 0);

    /* Refresh */
    Term_fresh();
    
    /* The player is not dead */
    (void)strcpy(died_from, "(saved)");

    /* Save the player, note the result */
    if (save_player()) prt("done.", 0, 15);
    else prt("Save failed!", 0, 15);

    /* Hack -- Forget that the player was saved */
    character_saved = FALSE;

    /* Note that the player is not dead */
    (void)strcpy(died_from, "(alive and well)");
}



/*
 * Destroy an item
 */
void do_cmd_destroy(void)
{
    int			item_val, amt = 1;
    int			old_number;

    cave_type		*c_ptr = &cave[py][px];
    inven_type		*i_ptr = &i_list[c_ptr->i_idx];

    cptr		pmt = "Destroy which item? ";

    char		i_name[80];

    char		out_val[160];


    /* Assume this will be free */
    energy_use = 0;


    /* Get an item (allow floor) or abort */
    if (!get_item(&item_val, pmt, 0, inven_ctr, TRUE)) {
        if (item_val == -2) msg_print("You have nothing to destroy.");
        return;
    }

    /* Get the item (if in inven/equip) */
    if (item_val >= 0) i_ptr = &inventory[item_val];


    /* See how many items */
    if (i_ptr->number > 1) {

        /* Prompt for the quantity */
        sprintf(out_val, "Quantity (1-%d): ", i_ptr->number);
        prt(out_val, 0, 0);
            
        /* Get the quantity */
        sprintf(out_val, "%d", amt);
        if (!askfor_aux(out_val, 3)) return;

        /* Extract a number */
        amt = atoi(out_val);

        /* A letter means "all" */
        if (isalpha(out_val[0])) amt = 99;

        /* Keep the entry valid */
        if (amt > i_ptr->number) amt = i_ptr->number;

        /* Allow bizarre "abort" */
        if (amt <= 0) return;
    }
    

    /* Describe the object */
    old_number = i_ptr->number;
    i_ptr->number = amt;
    objdes(i_name, i_ptr, TRUE, 3);
    i_ptr->number = old_number;
    
    /* Make a verification */
    sprintf(out_val, "Really destroy %s?", i_name);
    if (!get_check(out_val)) return;


    /* Take a turn */
    energy_use = 100;

    /* Artifacts cannot be destroyed */
    if (artifact_p(i_ptr)) {

        /* Message */
        msg_format("Cannot destroy %s.", i_name);

        /* Done */
        return;
    }
    
    /* Message */
    msg_format("Destroyed %s.", i_name);
    
    /* Eliminate the item (from the pack) */
    if (item_val >= 0) {
        inven_item_increase(item_val, -amt);
        inven_item_optimize(item_val);
    }

    /* Eliminate the item (from the floor) */
    else {
        floor_item_increase(py, px, -amt);
        floor_item_optimize(py, px);
    }


    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);
}


/*
 * Observe an item which has been *identify*-ed
 */
void do_cmd_observe(void)
{
    int			item_val;

    cave_type		*c_ptr = &cave[py][px];
    inven_type		*i_ptr = &i_list[c_ptr->i_idx];

    cptr		pmt = "Examine which item? ";

    char		i_name[80];
    

    /* Assume free */
    energy_use = 0;
    
    
    /* Get an item to identify (allow floor) or abort */
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1, TRUE)) {
        if (item_val == -2) msg_print("You have nothing to examine.");
        return;
    }

    /* Get the item (if in inven/equip) */
    if (item_val >= 0) i_ptr = &inventory[item_val];


    /* Require full knowledge */
    if (!(i_ptr->ident & ID_MENTAL)) {
        msg_print("You have no special knowledge about that item.");
        return;
    }
    
    
    /* Description */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Describe */
    msg_format("Examining %s...", i_name);

    /* Describe it fully */
    if (!identify_fully_aux(i_ptr)) msg_print("You see nothing special.");
}










/*
 * Define a new "keymap" entry
 */
void do_cmd_keymap(void)
{
    char i1, i2, i3;

    /* Free turn */
    energy_use = 0;

#ifdef ALLOW_KEYMAP

    /* Flush the input */
    flush();

    /* Get the trigger */
    if (get_com("Type the trigger keypress: ", &i1) &&
        get_com("Type the resulting keypress: ", &i2) &&
        get_com("Type a direction (or space): ", &i3)) {

        /* Acquire the array index */
        int k = (byte)(i1);
        int r = (byte)(i2);
        int d = (byte)(i3);

        /* Analyze the result key */
        keymap_cmds[k] = r;

        /* Hack -- Analyze the "direction" (always allow numbers) */
        keymap_dirs[k] = (isdigit(d) ? (d - '0') : keymap_dirs[d]);

        /* Success */
        prt(format("Keypress 0x%02x mapped to 0x%02x/%d",
                   k, keymap_cmds[k], keymap_dirs[k]), 0, 0);
    }

    /* Forget it */
    else {
        prt("Cancelled.", 0, 0);
    }

#else

    msg_print("You are not allowed to do that.");

#endif

}



/*
 * Hack -- dump the screen to a file
 */
void do_cmd_dump(bool color)
{
    int y, x;

    FILE *fff;

#ifdef MACINTOSH
    _ftype = 'TEXT';
#endif

    /* Hack -- drop permissions */
    safe_setuid_drop();

    /* Free */
    fff = fopen("dump.txt", "w");

    /* Hack -- grab permissions */
    safe_setuid_grab();

    /* Oops */
    if (!fff) {
        msg_print("Unable to dump the screen to 'dump.txt'");
        return;
    }

    /* Dump the screen */
    for (y = 0; y < 24; y++) {

        /* Dump each row */
        for (x = 0; x < 79; x++) {

            byte a = 0;
            char c = ' ';
                        
            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Dump it */
            fprintf(fff, "%c", c);
        }

        /* End the row */
        fprintf(fff, "\n");
        
        /* No color */
        if (!color) continue;

        /* Dump each row */
        for (x = 0; x < 79; x++) {

            byte a = 0;
            char c = ' ';
                        
            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Dump it */
            fprintf(fff, "%x", a);
        }

        /* End the row */
        fprintf(fff, "\n");
    }
    
    /* Close it */
    fclose(fff);
    
    /* Message */
    msg_print("Dumped the screen to 'dump.txt'");
}







/*
 * Move an item from equipment list to pack
 * Note that only one item at a time can be wielded per slot.
 * Note that taking off an item when "full" will cause that item
 * to fall to the ground.
 */
static void inven_takeoff(int item_val, int amt)
{
    int			posn;

    inven_type		*i_ptr;
    inven_type		tmp_obj;

    cptr		act;

    char		i_name[80];
    

    /* Get the item to take off */
    i_ptr = &inventory[item_val];

    /* Paranoia */
    if (amt <= 0) return;

    /* Verify */
    if (amt > i_ptr->number) amt = i_ptr->number;

    /* Make a copy to carry */
    tmp_obj = *i_ptr;
    tmp_obj.number = amt;

    /* What are we "doing" with the object */
    if (amt < i_ptr->number) {
        act = "Took off ";
    }
    else if (item_val == INVEN_WIELD) {
        act = "Was wielding ";
    }
    else if (item_val == INVEN_BOW) {
        act = "Was shooting with ";
    }
    else if (item_val == INVEN_LITE) {
        act = "Light source was ";
    }
    else {
        act = "Was wearing ";
    }

    /* Carry the object, saving the slot it went in */
    posn = inven_carry(&tmp_obj);

    /* Describe the result */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("%^s%s (%c).", act, i_name, index_to_label(posn));

    /* Delete (part of) it */
    inven_item_increase(item_val, -amt);
    inven_item_optimize(item_val);
}




/*
 * Drops (some of) an item from inventory to "near" the current location
 */
static void inven_drop(int item_val, int amt)
{
    inven_type		*i_ptr;
    inven_type		 tmp_obj;

    cptr		act;

    char		i_name[80];
    

    /* Access the slot to be dropped */
    i_ptr = &inventory[item_val];

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
    if (amt < i_ptr->number) {
        act = "Dropped";
    }
    else if (item_val == INVEN_WIELD) {
        act = "Was wielding";
    }
    else if (item_val == INVEN_BOW) {
        act = "Was shooting with";
    }
    else if (item_val == INVEN_LITE) {
        act = "Light source was";
    }
    else if (item_val >= INVEN_WIELD) {
        act = "Was wearing";
    }
    else {
        act = "Dropped";
    }

    /* Message */
    objdes(i_name, &tmp_obj, TRUE, 3);

    /* Message */
    msg_format("%^s %s (%c).", act, i_name, index_to_label(item_val));

    /* Drop it (carefully) near the player */
    drop_near(&tmp_obj, 0, py, px);

    /* Decrease the item, optimize. */
    inven_item_increase(item_val, -amt);
    inven_item_describe(item_val);
    inven_item_optimize(item_val);
}


/*
 * Given an item, find a slot to wield it in
 */
static int wield_slot(inven_type *i_ptr)
{
    /* Slot for equipment */
    switch (i_ptr->tval) {

        case TV_SHOT: case TV_BOLT: case TV_ARROW:
            return (-1);

        case TV_BOW:
            return (INVEN_BOW);

        case TV_DIGGING: case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
            return (INVEN_WIELD);

        case TV_LITE:
            return (INVEN_LITE);

        case TV_BOOTS:
            return (INVEN_FEET);

        case TV_GLOVES:
            return (INVEN_HANDS);

        case TV_CLOAK:
            return (INVEN_OUTER);

        case TV_CROWN:
        case TV_HELM:
            return (INVEN_HEAD);

        case TV_SHIELD:
            return (INVEN_ARM);

        case TV_DRAG_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
            return (INVEN_BODY);

        case TV_AMULET:
            return (INVEN_NECK);

        case TV_RING:

            /* Use the right hand first */
            if (!inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);

            /* And then the left hand */
            if (!inventory[INVEN_LEFT].k_idx) return (INVEN_LEFT);

            /* Use the left hand for swapping (by default) */
            if (!other_query_flag) return (INVEN_LEFT);

            /* Hack -- ask for a hand */
            while (1) {

                char query;
                int slot = -1;

                get_com("Put ring on which hand (l/r/L/R)?", &query);

                if (query == ESCAPE) return (-1);

                if (query == 'l') return (INVEN_LEFT);
                if (query == 'r') return (INVEN_RIGHT);

                if (query == 'L') slot = INVEN_LEFT;
                if (query == 'R') slot = INVEN_RIGHT;

                if (slot >= 0) {
                    if (!verify("Replace", slot)) break;
                    return (slot);
                }

                bell();
            }

            /* Hack -- no selection */
            return (-1);
    }

    /* Weird request */
    msg_print("You can't wear/wield that item!");

    /* No slot available */
    return (-1);
}





/*
 * Display inventory
 */
void do_cmd_inven(void)
{
    char out_val[160];


    /* Free command */
    energy_use = 0;

    /* Note that we are in "inventory" mode */
    command_wrk = TRUE;


    /* Save the screen */
    save_screen();

    /* Display the inventory */
    show_inven(0, inven_ctr - 1);

    /* Build a prompt */
    sprintf(out_val, "Inventory (carrying %d.%d / %d.%d pounds). Command: ",
            inven_weight / 10, inven_weight % 10,
            weight_limit() / 10, weight_limit() % 10);

    /* Get a command */
    prt(out_val, 0, 0);

    /* Get a new command */
    command_new = inkey();

    /* Restore the screen */
    restore_screen();


    /* Process "Escape" */
    if (command_new == ESCAPE) {

        /* Reset stuff */
        command_new = 0;
        command_gap = 50;
    }

    /* Process "space" */
    else if (command_new == ' ') {

        /* Do "inventory" again */
        command_new = 'i';
    }

    /* Process normal keys */
    else {

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


    /* Free command */
    energy_use = 0;

    /* Note that we are in "equipment" mode */
    command_wrk = FALSE;


    /* Save the screen */
    save_screen();

    /* Hack -- show empty slots */
    item_tester_full = TRUE;
    
    /* Display the equipment */
    show_equip(INVEN_WIELD, INVEN_TOTAL-1);

    /* Hack -- undo the hack above */
    item_tester_full = FALSE;
    
    /* Build a prompt */
    sprintf(out_val, "Equipment (carrying %d.%d / %d.%d pounds). Command: ",
            inven_weight / 10, inven_weight % 10,
            weight_limit() / 10, weight_limit() % 10);

    /* Get a command */
    prt(out_val, 0, 0);

    /* Get a new command */
    command_new = inkey();

    /* Restore the screen */
    restore_screen();


    /* Process "Escape" */
    if (command_new == ESCAPE) {

        /* Reset stuff */
        command_new = 0;
        command_gap = 50;
    }

    /* Hack -- Ignore "space" */
    else if (command_new == ' ') {

        /* Do "equipment" again */
        command_new = 'e';
    }

    /* Process normal keys */
    else {

        /* Enter "display" mode */
        command_see = TRUE;
    }
}


/*
 * The "wearable" tester
 */
static bool item_tester_hook_wear(inven_type *i_ptr)
{
    /* Check the tval */
    switch (i_ptr->tval) {
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
        case TV_LITE:
        case TV_AMULET:
        case TV_RING:
            return (TRUE);
    }
    
    /* Assume not wearable */
    return (FALSE);
}


/*
 * Wield or wear a single item from the pack or floor
 */
void do_cmd_wield(void)
{
    int item, slot;
    inven_type tmp_obj;
    inven_type *i_ptr;
    cptr location;

    char i_name[80];


    /* Assume this will be free */
    energy_use = 0;


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Restrict the choices */
    item_tester_hook = item_tester_hook_wear;

    /* Get an item to wear or wield (if possible) */
    if (!get_item(&item, "Wear/Wield which item? ", 0, inven_ctr - 1, TRUE)) {
        if (item == -2) msg_print("You have nothing you can wear or wield.");
        return;
    }

    /* Access the item */
    if (item >= 0) i_ptr = &inventory[item];

    /* Check the slot */
    slot = wield_slot(i_ptr);

    /* Paranoia -- No slot! */
    if (slot < 0) return;

    /* Prevent wielding into a cursed slot */
    if (cursed_p(&inventory[slot])) {

        /* Describe it */
        objdes(i_name, &inventory[slot], FALSE, 0);

        /* Message */
        msg_format("The %s you are %s appears to be cursed.",
                   i_name, describe_use(slot));

        /* Cancel the command */
        return;
    }

    /* XXX XXX XXX Hack -- Verify potential overflow */
    if ((inven_ctr >= INVEN_PACK) &&
        ((item < 0) || (i_ptr->number > 1))) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack may overflow.  Continue?")) return;
    }


    /* Take a turn */
    energy_use = 100;

    /* Get a copy of the object to wield */
    tmp_obj = *i_ptr;
    tmp_obj.number = 1;

    /* Decrease the item (from the pack) */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else {
        floor_item_increase(py, px, -1);
        floor_item_optimize(py, px);
    }

    /* Access the wield slot */
    i_ptr = &inventory[slot];

    /* Take off the "entire" item if one is there */
    if (inventory[slot].k_idx) inven_takeoff(slot, 255);

    /*** Could make procedure "inven_wield()" ***/

    /* Wear the new stuff */
    *i_ptr = tmp_obj;

    /* Increase the weight */
    inven_weight += i_ptr->weight;

    /* Increment the equip counter by hand */
    equip_ctr++;

    /* Where is the item now */
    if (slot == INVEN_WIELD) {
        location = "You are wielding";
    }
    else if (slot == INVEN_BOW) {
        location = "You are shooting with";
    }
    else if (slot == INVEN_LITE) {
        location = "Your light source is";
    }
    else {
        location = "You are wearing";
    }

    /* Describe the result */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("%^s %s. (%c)", location, i_name, index_to_label(slot));

    /* Cursed! */
    if (cursed_p(i_ptr)) {
        msg_print("Oops! It feels deathly cold!");
        i_ptr->ident |= ID_SENSE;
    }

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_EQUIPPY | PR_CHOICE);
}



/*
 * Take off an item
 */
void do_cmd_takeoff(void)
{
    int item;

    inven_type *i_ptr;


    /* Assume this will be free */
    energy_use = 0;

    /* XXX XXX XXX Hack -- verify potential overflow */
    if (inven_ctr >= INVEN_PACK) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack may overflow.  Continue?")) return;
    }

    /* Get an item to take off */
    if (!get_item(&item, "Take off which item? ", INVEN_WIELD, INVEN_TOTAL-1, FALSE)) {
        if (item == -2) msg_print("You are not wearing anything to take off.");
        return;
    }

    /* XXX XXX Indent me */
    if (TRUE) {
    
        /* Access the item */
        i_ptr = &inventory[item];

        /* Not gonna happen */
        if (cursed_p(i_ptr)) {
            msg_print("Hmmm, it seems to be cursed.");
        }

        /* Item selected? */
        else {

            /* This turn is not free */
            energy_use = 100;

            /* Take off the "entire" item */
            inven_takeoff(item, 255);

            /* Update stuff */
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
            
            /* Redraw equippy chars */
            p_ptr->redraw |= (PR_EQUIPPY | PR_CHOICE);
        }
    }
}


/*
 * Drop an item
 */
void do_cmd_drop(void)
{
    int item, amt = 1;

    inven_type *i_ptr;

    char	out_val[160];
    

    /* Assume this will be free */
    energy_use = 0;

    /* Get an item to take off */
    if (!get_item(&item, "Drop which item? ", 0, INVEN_TOTAL-1, FALSE)) {
        if (item == -2) msg_print("You have nothing to drop.");
        return;
    }

    /* Get the item */
    i_ptr = &inventory[item];

    /* Not gonna happen XXX inscribe */
    if ((item >= INVEN_WIELD) && cursed_p(i_ptr)) {
        energy_use = 100;
        msg_print("Hmmm, it seems to be cursed.");
        return;
    }

    /* See how many items */
    if (i_ptr->number > 1) {

        /* Prompt for the quantity */
        sprintf(out_val, "Quantity (1-%d): ", i_ptr->number);
        prt(out_val, 0, 0);
        
        /* Get the quantity */
        sprintf(out_val, "%d", amt);
        if (!askfor_aux(out_val, 3)) return;

        /* Extract a number */
        amt = atoi(out_val);

        /* A letter means "all" */
        if (isalpha(out_val[0])) amt = 99;

        /* Keep the entry valid */
        if (amt > i_ptr->number) amt = i_ptr->number;

        /* Allow bizarre "abort" */
        if (amt <= 0) return;
    }

    /* Mega-Hack -- verify "dangerous" drops */
    if (cave[py][px].i_idx) {

        /* XXX XXX Verify with the player */
        if (other_query_flag &&
            !get_check("The item may disappear.  Continue?")) return;
    }


    /* This turn is not free */
    energy_use = 100;

    /* Drop (some of) the item */
    inven_drop(item, amt);

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
        
    /* Redraw equippy chars */
    p_ptr->redraw |= (PR_EQUIPPY | PR_CHOICE);
}






