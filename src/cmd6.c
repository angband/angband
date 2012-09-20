/* File: cmd6.c */

/* Purpose: process player commands */

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
    msg_print("You enter a maze of up staircases. ");

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
    msg_print("You enter a maze of down staircases. ");

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
    energy_use = 0;

    Term_flush();

    if (total_winner) {
        if (!get_check("Do you want to retire?")) return;
    }
    else {
        int i;
        if (!get_check("Do you really want to quit?")) return;
        Term_flush();
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

    /* Hack */
    if (p_ptr->image) {
        msg_print("You cannot be sure what is real and what is not!");
    }

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw everything */
    p_ptr->redraw |= (PR_CAVE);

    /* Handle stuff */
    handle_stuff(TRUE);
    
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
 * The tester for "flasks"
 */
static bool item_tester_hook_flask(inven_type *i_ptr)
{
    if (i_ptr->tval != TV_FLASK) return (FALSE);
    return (TRUE);
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(void)
{
    int item;
    bool floor;
    inven_type *i_ptr;
    inven_type *j_ptr;


    /* Assume this will be free */
    energy_use = 0;

    /* Access the lantern */
    j_ptr = &inventory[INVEN_LITE];


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Check for use of the floor */
    floor = item_tester_hook_flask(i_ptr);


    /* Restrict the choices */
    item_tester_hook = item_tester_hook_flask;

    /* Get a flask to refuel with */
    if (get_item(&item, "Refill with which flask? ", 0, inven_ctr - 1, floor)) {

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

    /* Hack -- nothing to refuel with */
    else if (item == -2) {
        msg_print("You have no oil.");
        if (command_see) command_new = command_xxx ? 'i' : 'e';
    }
}




/*
 * The tester for "torches"
 */
static bool item_tester_hook_torch(inven_type *i_ptr)
{
    if (i_ptr->tval != TV_LITE) return (FALSE);
    if (i_ptr->sval != SV_LITE_TORCH) return (FALSE);
    return (TRUE);
}


/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch(void)
{
    int item;
    bool floor;
    inven_type *i_ptr;
    inven_type *j_ptr;


    /* Assume this will be free */
    energy_use = 0;

    /* Access the primary torch */
    j_ptr = &inventory[INVEN_LITE];


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Check for use of the floor */
    floor = item_tester_hook_torch(i_ptr);


    /* Restrict the choices */
    item_tester_hook = item_tester_hook_torch;

    /* Get a torch to refuel with */
    if (get_item(&item, "Refuel with which torch? ", 0, inven_ctr - 1, floor)) {

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
    }

    /* Hack -- nothing to refuel with */
    else if (item == -2) {
        msg_print("You have no extra torches.");
        if (command_see) command_new = command_xxx ? 'i' : 'e';
    }
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
        message("You are not wielding a light", 0);
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
        message("Your light cannot be refilled.", 0);
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
        if (k == CTRL('P')) {
            if (i+20 < n) i += 20;
        }

        /* Recall more newer messages */
        if (k == CTRL('N')) {
            if (i >= 20) i -= 20;
        }

        /* Error of some kind, do not redraw */
        if (i == j) {
            k = 0;
            bell();
        }
    }

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

    /* Be sure we can see */
    if (p_ptr->blind) {
        msg_print("You can't see anything to target!");
    }
    else if (!target_set()) {
        msg_print("Target Aborted");
    }
    else {
        msg_print("Target Selected");
    }

#else

    msg_print("Target code not compiled in this version.");

#endif

}




/*
 * A simple structure to hold some options
 */
typedef struct _opt_desc {
    cptr	o_prompt;
    int         *o_var;
} opt_desc;


/*
 * General User-Interface Options
 */
static opt_desc options_interface[] = {

    { "Rogue-like commands", 			&rogue_like_commands },
    { "Quick messages",		                &quick_messages },
    { "Prompt for various information", 	&other_query_flag },
    { "Prompt before picking things up", 	&carry_query_flag },
    { "Use old target by default",		&use_old_target },
    { "Pick things up by default", 		&always_pickup },
    { "Accept all throw commands",		&always_throw },
    { "Repeat obvious commands",		&always_repeat },

    { "Use new screen layout",                  &new_screen_layout },
    { "Display Equippy chars",		        &equippy_chars },
    { "Show dungeon level in feet",		&depth_in_feet },
    { "Notice mineral seams", 			&notice_seams },

    { "Use color",				&use_color },

    { "Hilite the player",			&hilite_player },

    { "Draw Torch-Lite in yellow",		&view_yellow_lite },
    { "Draw Viewable Lite brightly",		&view_bright_lite },

    { "Ring bell on error",			&ring_bell },

    { "Compress savefiles",			&compress_savefile },

    { NULL,					NULL }
};


/*
 * Disturbance Options -- for running/resting
 */
static opt_desc options_disturb[] = {

    { "Cut known corners",	 		&find_cut },
    { "Examine potential corners",		&find_examine },
    { "Print self during run",			&find_prself },
    { "Stop when map sector changes",		&find_bound },
    { "Run through open doors", 		&find_ignore_doors },
    { "Run past stairs", 			&find_ignore_stairs },

    { "Monster moving nearby disturbs me",	&disturb_near },
    { "Monster moving anywhere disturbs me",	&disturb_move },
    { "Monster appearance disturbs me",		&disturb_enter },
    { "Monster disappearance disturbs me",	&disturb_leave },

    { "Flush input before normal commands",	&flush_command },
    { "Flush input whenever disturbed",		&flush_disturb },
    { "Flush input on various failures",	&flush_failure },

    { "Flush output before all commands",	&fresh_before },
    { "Flush output after all commands",	&fresh_after },
    { "Flush output while running",		&fresh_find },

    { NULL,					NULL }
};


/*
 * Inventory Options -- these slightly affect game-play
 */
static opt_desc options_inventory[] = {

    { "Use Recall window",			&use_recall_win },
    { "Use Choice window",			&use_choice_win },

    { "Recall monster descriptions",		&recall_show_desc },
    { "Recall monster kill counts",		&recall_show_kill },

    { "Show weights in inven (choice)",		&choice_inven_wgt },
    { "Show weights in equip (choice)",		&choice_equip_wgt },

    { "Show something in inven (choice)",	&choice_inven_xtra },
    { "Show something in equip (choice)",	&choice_equip_xtra },

    { "Show weights in inventory list",		&show_inven_weight },
    { "Show weights in equipment list",		&show_equip_weight },
    { "Show weights in stores",			&show_store_weight },
    { "Plain object descriptions",		&plain_descriptions },

    { "Allow weapons and armor to stack",	&stack_allow_items },
    { "Allow wands/staffs/rods to stack",	&stack_allow_wands },
    { "Over-ride inscriptions when stacking",	&stack_force_notes },
    { "Over-ride discounts when stacking",	&stack_force_costs },

    { "Disable haggling in stores",		&no_haggle_flag },
    { "Shuffle store owners",			&shuffle_owners },

    { NULL,					NULL}
};


/*
 * Gameplay Options -- these actually affect game-play
 */
static opt_desc options_gameplay[] = {

    { "Map remembers all illuminated walls",	&view_wall_memory },
    { "Map remembers all important stuff",	&view_xtra_memory },
    { "Map remembers all perma-lit grids",	&view_perma_grids },
    { "Map remembers all torch-lit grids",	&view_torch_grids },

    { "Reduce view-radius when running",	&view_reduce_view },
    { "Reduce lite-radius when running",	&view_reduce_lite },

    { "Show extra spell info",			&show_spell_info },
    { "Show monster health bar",		&show_health_bar },

    { "Create characters in 'maximize' mode",	&begin_maximize },
    { "Create characters in 'preserve' mode",	&begin_preserve },

    { "Align rooms to dungeon panels",		&dungeon_align },
    { "Unused dungeon generation option",	&dungeon_other },

    { "Monsters chase current location",	&flow_by_sound },
    { "Monsters chase recent locations",	&flow_by_smell },

    { "Monsters follow the player",		&track_follow },
    { "Monsters target the player",		&track_target },

    { "Monsters learn from their mistakes",	&smart_learn },
    { "Monsters exploit players weaknesses",	&smart_cheat },

    { NULL,					NULL}
};


/*
 * Cheating Options -- go ahead and cheat
 */
static opt_desc options_cheating[] = {

    { "Peek into object creation",	&cheat_peek },
    { "Peek into monster creation",	&cheat_hear },
    { "Peek into dungeon creation",	&cheat_room },
    { "Peek into something else",	&cheat_xtra },
    { "Know complete monster info",	&cheat_know },
    { "Allow player to avoid death",	&cheat_live },

    { NULL,					NULL}
};


/*
 * Set or unset various boolean options.
 */
static void do_cmd_options_aux(opt_desc *options, cptr info)
{
    int		i, max, ch;

    char	pmt[160];
    char	string[160];


    clear_screen();

    /* Prompt */
    sprintf(pmt, "%s (RET to advance, y/n to set, ESC to accept) ", info);
    prt(pmt, 0, 0);

    /* Prepare the screen, Count the options. */
    for (max = 0; options[max].o_prompt; max++) {
        sprintf(string, "%-38s: %s ", options[max].o_prompt,
                (*options[max].o_var ? "yes" : "no "));
        prt(string, max + 2, 0);
    }

    /* Start at the first option */
    i = 0;

    /* Interact with the player */
    for (;;) {
        move_cursor(i + 2, 40);
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
            put_str("yes ", i + 2, 40);
            *options[i].o_var = TRUE;
            i = (i + 1) % max;
            break;
          case 'n':
          case 'N':
          case '4':
            put_str("no  ", i + 2, 40);
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


    /* Restore the screen */
    restore_screen();


    /* Mega-Hack */
    if (!character_generated) return;


    /* Free turn */
    energy_use = 0;

    /* Combine the pack */
    combine_pack();

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP | PR_BLOCK);
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
        fprintf(fff, "R:%d:%d/%d\n", i, l_ptr->l_attr, l_ptr->l_char);
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
        fprintf(fff, "K:%d:%d/%d\n", i, x_list[i].x_attr, x_list[i].x_char);
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

    /* Interact until done */
    while (1) {

        /* Clear the screen */
        clear_screen();

        /* Give some choices */
        prt("(1) Load a 'pref' file", 2, 5);
        prt("(2) Append macros to a 'pref' file", 3, 5);
        prt("(3) Append something -- not ready yet", 4, 5);
        prt("(4) Append monster attr/chars to a 'pref' file", 5, 5);
        prt("(5) Append object attr/chars to a 'pref' file", 6, 5);
        prt("(6) Change monster attr/chars", 7, 5);
        prt("(7) Change object attr/chars", 8, 5);
        prt("(8) Change tval_to_attr[] -- not ready yet", 9, 5);
        prt("(9) Change tval_to_char[] -- not ready yet", 10, 5);

        /* Ask for a choice */
        prt("Angband Preferences (1-9 or ESC to exit) ", 0, 0);
        i = inkey();
        if (i == ESCAPE) break;

        /* Load a 'pref' file */
        if (i == '1') {

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

            /* Hack -- start on the "urchin" */
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
                            format("Object = %d, Name = %-40.40s",
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
                if (i == 'n') r_idx = (r_idx + MAX_K_IDX + 1) % MAX_K_IDX;
                if (i == 'N') r_idx = (r_idx + MAX_K_IDX - 1) % MAX_K_IDX;
                if (i == 'a') l_ptr->l_attr = (byte)(ca + 1);
                if (i == 'A') l_ptr->l_attr = (byte)(ca - 1);
                if (i == 'c') l_ptr->l_char = (byte)(cc + 1);
                if (i == 'C') l_ptr->l_char = (byte)(cc - 1);
            }
        }

        /* Hack -- Redefine object attr/chars */
        else if (i == '7') {

            /* Hack -- start on the "floor" */
            static int k_idx = OBJ_GRANITE_WALL;

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

    /* Grab priv's */
    safe_setuid_grab();

    /* Free turn */
    energy_use = 0;

    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP | PR_BLOCK);
}


/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
    char buf[128];
    energy_use = 0;
    prt("Note: ", 0, 0);
    if (askfor(buf, 60)) msg_print(format("Note: %s", buf));
}








/*
 * Variable used by the functions below
 */
static int hack_dir;


/*
 * Convert a "Rogue" keypress into an "Angband" keypress
 * Pass extra information as needed via "hack_dir"
 */
static char roguelike_commands(char command)
{
    char b1 = '[', b2 = ']';

    /* Process the command */
    switch (command) {

        /* Movement (rogue keys) */
        case 'b': hack_dir = 1; return (';');
        case 'j': hack_dir = 2; return (';');
        case 'n': hack_dir = 3; return (';');
        case 'h': hack_dir = 4; return (';');
        case 'l': hack_dir = 6; return (';');
        case 'y': hack_dir = 7; return (';');
        case 'k': hack_dir = 8; return (';');
        case 'u': hack_dir = 9; return (';');

        /* Running (shift + rogue keys) */
        case 'B': hack_dir = 1; return ('.');
        case 'J': hack_dir = 2; return ('.');
        case 'N': hack_dir = 3; return ('.');
        case 'H': hack_dir = 4; return ('.');
        case 'L': hack_dir = 6; return ('.');
        case 'Y': hack_dir = 7; return ('.');
        case 'K': hack_dir = 8; return ('.');
        case 'U': hack_dir = 9; return ('.');

        /* Tunnelling (control + rogue keys) */
        case CTRL('B'): hack_dir = 1; return ('+');
        case CTRL('J'): hack_dir = 2; return ('+');
        case CTRL('N'): hack_dir = 3; return ('+');
        case CTRL('H'): hack_dir = 4; return ('+');
        case CTRL('L'): hack_dir = 6; return ('+');
        case CTRL('Y'): hack_dir = 7; return ('+');
        case CTRL('K'): hack_dir = 8; return ('+');
        case CTRL('U'): hack_dir = 9; return ('+');

        /* Hack -- CTRL('M') == return == linefeed == CTRL('J') */
        case CTRL('M'): hack_dir = 2; return ('+');

        /* Zap a staff */
        case 'Z':
            return ('u');

        /* Wield */
        case 'w':
            return (b1);

        /* Take off */
        case 'T':
            return (b2);

        /* Fire */
        case 't':
            return ('f');

        /* Bash */
        case 'f':
            return ('B');

        /* Look */
        case 'x':
            return ('l');

        /* Aim a wand */
        case 'z':
            return ('a');

        /* Zap a rod */
        case 'a':
            return ('z');

        /* Stand still */
        case '.': hack_dir = 5; return (',');

        /* Stand still (That is, Rest one turn) */
        case '5': hack_dir = 5; return (',');

        /* Standard walking */
        case '1': hack_dir = 1; return (';');
        case '2': hack_dir = 2; return (';');
        case '3': hack_dir = 3; return (';');
        case '4': hack_dir = 4; return (';');
        case '6': hack_dir = 6; return (';');
        case '7': hack_dir = 7; return (';');
        case '8': hack_dir = 8; return (';');
        case '9': hack_dir = 9; return (';');
    }

    /* Default */
    return (command);
}


/*
 * Convert an "Original" keypress into an "Angband" keypress
 * Pass direction information back via "hack_dir".
 */
static char original_commands(char command)
{
    char b1 = '[', b2 = ']';

    /* Process the command */
    switch (command) {

        /* White space */
        case CTRL('J'):
        case CTRL('M'):
            return (' ');

        /* Suicide */
        case CTRL('K'):
            return ('Q');

        /* Locate */
        case 'L':
            return ('W');

        /* Search mode */
        case 'S':
            return ('#');

        /* Browse */
        case 'b':
            return ('P');

        /* Spike */
        case 'j':
            return ('S');

        /* Wield */
        case 'w':
            return (b1);

        /* Take off */
        case 't':
            return (b2);

        /* Tunnel digging */
        case 'T':
            return ('+');

        /* Stand still */
        case '5': hack_dir = 5; return (',');

        /* Standard walking */
        case '1': hack_dir = 1; return (';');
        case '2': hack_dir = 2; return (';');
        case '3': hack_dir = 3; return (';');
        case '4': hack_dir = 4; return (';');
        case '6': hack_dir = 6; return (';');
        case '7': hack_dir = 7; return (';');
        case '8': hack_dir = 8; return (';');
        case '9': hack_dir = 9; return (';');
    }

    /* Default */
    return (command);
}


/*
 * Initialize the "keymap" arrays based on the current value of
 * "rogue_like_commands".  Note that all "undefined" keypresses
 * by default map to themselves with no direction.  This allows
 * "standard" commands to use the same keys in both keysets.
 *
 * The keymap arrays map keys to "command_cmd" and "command_dir".
 *
 * If "keymap_dirs[n] == 255" then "command_dir" will be "-1",
 * that is, no direction has been specified yet.
 *
 * Note that it is illegal for keymap_cmds[N] to be zero, except
 * perhaps for keymaps_cmds[0], which is unused.
 */
void keymap_init(void)
{
    int i, k;

    /* Initialize every entry */
    for (i = 0; i < 256; i++) {

        /* Default to "no direction" */
        hack_dir = 255;

        /* Attempt to translate */
        if (rogue_like_commands) {
            k = roguelike_commands(i);
        }
        else {
            k = original_commands(i);
        }

        /* Save the keypress */
        keymap_cmds[i] = k;

        /* Save the direction */
        keymap_dirs[i] = hack_dir;
    }
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
    Term_flush();

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

        /* Hack -- Analyze the "direction" */
        keymap_dirs[k] = keymap_dirs[d];

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
 * Define a new macro
 *
 * If "cmd_flag" is true, then this is a "command macro".
 *
 * Note that this function resembles "inkey_aux()" in "io.c"
 *
 * The "trigger" must be entered with less than 10 milliseconds
 * between keypresses, as in "inkey_aux()".
 */
void do_cmd_macro(bool cmd_flag)
{
    int i, n = 0, w, fix;

    char pat[1024] = "";
    char act[1024] = "";

    char tmp[1024];


    /* Free turn */
    energy_use = 0;

#ifdef ALLOW_MACROS

    /* Handle stuff */
    handle_stuff(TRUE);
    
    /* Flush the input */
    Term_flush();

    /* Show the cursor */
    fix = (0 == Term_show_cursor());

    /* Get the first key */
    prt("Press the macro trigger key: ", 0, 0);

    /* Flush output */
    Term_fresh();
    
    /* Wait for the first key */
    i = Term_inkey();
    pat[n++] = i;
    pat[n] = '\0';

    /* Read the rest of the pattern */
    for (w = 0; TRUE; ) {

        /* If a key is ready, acquire it */
        if (Term_kbhit()) {
            i = Term_inkey();
            pat[n++] = i;
            pat[n] = '\0';
            w = 0;
        }

        /* Wait for it */
        else {
            if (w > 30) break;
            delay(++w);
        }	
    }

    /* Hide cursor if needed */
    if (fix) Term_hide_cursor();


    /* Hack -- verify "normal" characters */
    if (!cmd_flag && (n == 1) && (pat[0] >= 32) && (pat[0] < 127)) {
        if (!get_check("Are you sure you want to redefine that?")) {
            prt("Macro cancelled.", 0, 0);
            return;
        }
    }


    /* Convert the trigger to text */
    ascii_to_text(tmp, pat);

    /* Flush the input */
    Term_flush();

    /* Verify */
    msg_print(format("Macro trigger '%s' accepted.", tmp));
    msg_print("Enter an encoded action.");
    msg_print(NULL);


    /* Prompt for the action */
    prt("Action: ", 0, 0);

    /* Get the encoded action */
    if (askfor(tmp, 70)) {

        /* Analyze the string */
        text_to_ascii(act, tmp);

        /* Add the macro */
        macro_add(pat, act, cmd_flag);

        /* Clear the line */
        prt("Macro accepted.", 0, 0);
    }

    /* Cancel */
    else {

        /* Cancelled */
        prt("Macro cancelled.", 0, 0);
    }

#else

    msg_print("You may not use macros.");

#endif

}




/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void process_command(void)
{
    /* Parse the command */
    switch (command_cmd) {

        /* (ESC) do nothing. */
        case ESCAPE:
            energy_use = 0; break;

        /* (SPACE) do nothing */
        case ' ':
            energy_use = 0; break;



        /*** Wizard Commands ***/

        /* Toggle Wizard Mode */
        case CTRL('W'):
            energy_use = 0;
            if (wizard) {
                wizard = FALSE;
                msg_print("Wizard mode off.");
            }
            else if (!can_be_wizard) {
                msg_print("You are not allowed to do that.");
            }
            else if (enter_wiz_mode()) {
                wizard = TRUE;
                msg_print("Wizard mode on.");
            }
            
            /* Update monsters */
            p_ptr->update |= (PU_MONSTERS);

            /* Redraw stats */
            p_ptr->redraw |= (PR_BLOCK);

            break;

        /* Special Wizard Command */
        case CTRL('A'):
            if (wizard) {
                do_wiz_command();
            }
            else {
                energy_use = 0;
                msg_print("You are not allowed to do that.");
            }
            break;

        /* Interact with the Borg */
        case CTRL('Z'):
            energy_use = 0;
#ifdef AUTO_PLAY
            borg_mode();
#endif
            break;


        /*** Extra commands ***/

        case ':':
            do_cmd_note(); break;	

        case '@':
            do_cmd_macro(FALSE); break;

        case '!':
            do_cmd_macro(TRUE); break;

        case '&':
            do_cmd_keymap(); break;


        /*** Inventory Commands ***/

        /* Wear or wield something */
        case '[':
        case 'w':
            do_cmd_inven_w(); break;

        /* Take something off */
        case ']':
        case 't':
            do_cmd_inven_t(); break;

        /* Drop something */
        case 'd':
            do_cmd_inven_d(); break;

        /* Equipment list */
        case 'e':
            do_cmd_inven_e(); break;

        /* Inventory */
        case 'i':
            do_cmd_inven_i(); break;


        /*** Handle "choice window" ***/

        /* Hack -- toggle choice window */
        case CTRL('E'):

            /* Free command */
            energy_use = 0;

            /* Hack -- flip the current status */
            choice_default = !choice_default;

            /* XXX XXX Hack -- update choice window */
            if (!choice_default) choice_inven(0, inven_ctr - 1);
            else choice_equip(INVEN_WIELD, INVEN_TOTAL - 1);

            break;


        /*** Standard "Movement" Commands ***/

        /* Dig a tunnel */
        case '+':
            do_cmd_tunnel(); break;

        /* Move (usually pick up things) */
        case ';':
            do_cmd_walk(always_pickup); break;

        /* Move (usually do not pick up) */
        case '-':
            do_cmd_walk(!always_pickup); break;


        /*** Commands that "re-interpret" the repeat count ***/

        /* Begin Running -- Arg is Max Distance */
        case '.':
            do_cmd_run(); break;

        /* Stay still (usually pick things up) */
        case ',':
            do_cmd_stay(always_pickup); break;

        /* Stay still (usually do not pick up) */
        case 'g':
            do_cmd_stay(!always_pickup); break;

        /* Begin Resting -- Arg is time */
        case 'R':
            do_cmd_rest(); break;



        /*** Searching, Resting ***/

        /* Search the adjoining grids */
        case 's':
            do_cmd_search(); break;

        /* Toggle search status */
        case '#':
            do_cmd_toggle_search(); break;


        /*** Stairs and Doors and Chests and Traps ***/

        /* Go up staircases */
        case '<':
            do_cmd_go_up(); break;

        /* Go down staircases */
        case '>':
            do_cmd_go_down(); break;

        /* Open something */
        case 'o':
            do_cmd_open(); break;

        /* Close something */
        case 'c':
            do_cmd_close(); break;

        /* Spike a door */
        case 'S':
            do_cmd_spike(); break;

        /* Force a door or Bash a monster. */
        case 'B':
            do_cmd_bash(); break;

        /* Disarm a trap */
        case 'D':
            do_cmd_disarm(); break;


        /*** Magic and Prayers ***/

        /* Peruse a Book */
        case 'P':
            do_cmd_browse(); break;

        /* Gain some spells */
        case 'G':
            do_cmd_study(); break;

        /* Cast a magic spell */
        case 'm':
            do_cmd_cast(); break;

        /* Pray a prayer */
        case 'p':
            do_cmd_pray(); break;


        /*** Use various objects ***/

        /* Inscribe an object */
        case '{':
            do_cmd_inscribe(); break;

        /* Inscribe an object (in a different way) */
        case '}':
            do_cmd_uninscribe(); break;

        /* Activate an artifact */
        case 'A':
            do_cmd_activate(); break;

        /* Eat some food */
        case 'E':
            do_cmd_eat_food(); break;

        /* Fill the lamp */
        case 'F':
            do_cmd_refill(); break;

        /* Throw something */
        case 'f':
            do_cmd_fire(); break;

        /* Zap a wand */
        case 'a':
            do_cmd_aim_wand(); break;

        /* Activate a rod */
        case 'z':
            do_cmd_zap_rod(); break;

        /* Quaff a potion */
        case 'q':
            do_cmd_quaff_potion(); break;

        /* Read a scroll */
        case 'r':
            do_cmd_read_scroll(); break;

        /* Zap a staff */
        case 'u':
            do_cmd_use_staff(); break;


        /*** Looking at Things (nearby or on map) ***/

        /* Full screen Map */
        case 'M':
            do_cmd_view_map(); break;

        /* Locate player on the map */	
        case 'W':
            do_cmd_locate(); break;

        /* Examine surroundings */
        case 'l':
            do_cmd_look(); break;

        /* Examine current target location */
        case 'x':
            do_cmd_examine(); break;

        /* Attempt to select a new target, if compiled */
        case '*':
            do_cmd_target(); break;



        /*** Help and Such ***/

        /* Help */
        case '?':
            do_cmd_help("help.hlp"); break;

        /* Identify Symbol */
        case '/':
            do_cmd_query_symbol(); break;

        /* Character Description */
        case 'C':
            do_cmd_change_name(); break;


        /*** System Commands ***/

        /* Hack -- Game Version */
        case 'V':
            do_cmd_help("version.hlp"); break;

        /* Repeat Feeling */
        case CTRL('F'):
            do_cmd_feeling(); break;

        /* Previous message(s). */
        case CTRL('P'):
            do_cmd_messages(); break;

        /* Commit Suicide and Quit */
        case 'Q':
            do_cmd_suicide(); break;

        /* Save and Quit */
        case CTRL('X'):
            exit_game(); break;

        /* Hack -- Save (no quit) */
        case CTRL('S'):

            /* Hack -- free turn */
            energy_use = 0;
            
            /* The player is not dead */
            (void)strcpy(died_from, "(saved)");

            /* Save the player, note the result */
            prt("Saving game... ", 0, 0);
            if (save_player()) prt("done.", 0, 15);
            else prt("Save failed!", 0, 15);

            /* Forget that the player was saved */
            character_saved = 0;

            /* Note that the player is not dead */
            (void)strcpy(died_from, "(alive and well)");

            /* Done */
            break;

        /* Redraw the screen */
        case CTRL('R'):
            do_cmd_redraw(); break;

        /* Set options */
        case '=':
            do_cmd_options(); break;

        /* Interact with preference files */
        case '%':
            do_cmd_prefs(); break;

        /* Check artifacts */
        case '~':
            do_cmd_check_artifacts(); break;

        /* Check uniques */
        case '|':
            do_cmd_check_uniques(); break;


        /* Hack -- Unknown command */
        default:
            energy_use = 0;
            prt("Type '?' for help.", 0, 0);
            return;
    }


    /* Save the command */
    command_old = command_cmd;
}





/*
 * XXX An explanation of the "Angband Keypress" concept. XXX
 *
 * Inherently, many Angband commands consist not only of a basic action
 * (such as "tunnel"), but also a direction (such as "north-east"), plus
 * other information such as a "repeat" count or "extra argument".
 *
 * These components are thus explicitly represented, with globals.
 *
 * The "base command" (see below) is stored in "command_cmd"
 * The "desired direction" is stored in "command_dir".
 * The "repeat count" is stored in "command_rep"
 * The "numerical argument" is stored in "command_arg"
 *
 * When "command_dir" is set, it overrides all calls to "get*dir()"
 * So we always reset command_dir to -1 before getting a new command.
 * Hack -- a "command_dir" of "zero" means "the current target".
 *
 * Note that "command_arg" is sometimes used to select an argument
 * to a command (whereas "command_rep" actually "repeats" it).
 * Commands using this include "rest", "run", and "wizard" commands.
 *
 * Note that nothing cancels "command_rep", so it must be explicitly
 * canceled by the repeatable functions on "termination" conditions.
 * The simple way to do this is to force the respective do_cmd routines
 * to actually check explicitly for permission to NOT cancel it.
 * The only way to cancel "command_rep" is via the "disturb()" function.
 *
 * Note that, to correctly handle repeating commands, all commands that
 * wish to be repeatable AND to do so with a specific "direction" had
 * better set "command_dir" on their first call to the user's DESIRED
 * direction.  A local copy can then be used to check confusion, etc.
 * The easiest way to do this is to actually USE "command_dir" as the
 * argument holding the direction (see "do_cmd_walk").
 *
 * Note that, however, to correctly handle confusion + repeated commands,
 * it may be necessary to call "get_a_dir" as above, and then use a temp
 * dir to apply confusion, via "dir = command_dir; confuse_dir(&dir,mode);"
 * where mode is, for example, 0x02 for "partial" confusion.
 *
 * The last command successfully executed is stored in "command_old".
 *
 * Eventually, "command_esc" will allow one to track low level "Escapes".
 */



/*
 * Check whether this command can be "repeated".
 *
 * Note -- this routine applies ONLY to "Angband Commands".
 *
 * Repeated commands must be VERY careful to correctly turn off the
 * "repeat" (by calling "disturb()") if they induce an action of any
 * kind that should cancel the "repeat".
 */
static int command_takes_rep(char c)
{
    /* Examine the command */
    switch (c) {

        /* Take a direction */
        case '-': /* Jump */
        case ';': /* Walk */
        case '+': /* Tunnel */
        case 'D': /* Disarm */
        case 'B': /* Bash/Force */
        case 'o': /* Open */
            return TRUE;

        /* Take no direction */
        case ',': /* Stay still */
        case 'g': /* Stay still */
        case 's': /* Search */
            return TRUE;
    }

    /* Assume no count allowed */
    return (FALSE);
}



/*
 * Check whether this command will accept an argument.
 * An "argument" command is one which allows the use of
 * the "repeat" formalism, but does not actually repeat.
 *
 * These commands are supplied an "extra" argument in the global variable
 * "command_arg".  It is (currently) always an integer from 0 to 9999.
 *
 * Note -- this routine applies ONLY to "Angband Commands".
 */
static int command_takes_arg(char c)
{
    /* Examine the command */
    switch (c) {

        /* Normal commands */
        case '.': /* Run */
        case 'R': /* Rest */

        /* Hack -- All Wizard Commands */
        case CTRL('A'): /* Special Wizard Command */
            return TRUE;
    }

    /* Assume no count allowed */
    return (FALSE);
}



/*
 * Apply a keypress to extract "command_cmd" and "command_dir"
 */
static void keymap_apply(char cmd)
{
    int k = (byte)(cmd);

    /* Access the array info */
    command_cmd = keymap_cmds[k];
    command_dir = keymap_dirs[k];

    /* Hack -- notice "undefined" commands */
    if (!command_cmd) command_cmd = ESCAPE;

    /* Hack -- extract "non-directions" if needed */
    if (command_dir > 9) command_dir = -1;
}


/*
 * Request a command from the user.
 *
 * Sets command_cmd, command_dir, command_rep, command_arg.
 *
 * Note that "caret" ("^") is treated special, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 */
void request_command(void)
{
    int i = 0;
    char cmd;


    /* Notice changes in the "rogue_like_commands" flag */
    static old_rogue_like = -1;

    /* Hack -- notice changes in "rogue_like_commands" */
    if (old_rogue_like != rogue_like_commands) keymap_init();

    /* Save the "rogue_like_commands" setting */
    old_rogue_like = rogue_like_commands;


    /* Hack -- Assume no abortion yet */
    command_esc = 0;


    /* Hack -- process "repeated" commands */
    if (command_rep) {

        /* Count this execution */
        command_rep--;

        /* Hack -- Dump repeat count */
        p_ptr->redraw |= (PR_STATE);

        /* Handle stuff */
        handle_stuff(TRUE);

        /* Hack -- Illuminate the player */
        move_cursor_relative(py, px);

        /* Refresh */
        Term_fresh();
        
        /* Hack -- Assume messages were seen */
        msg_flag = FALSE;

        /* Keep the current command */
        return;
    }


    /* No command yet */
    command_cmd = 0;

    /* No "argument" yet */
    command_arg = 0;

    /* Hack -- no direction yet */
    command_dir = -1;


    /* Hack -- Illuminate the player */
    move_cursor_relative(py, px);


    /* Hack -- Optional flush */
    if (flush_command) flush();


    /* Hack -- auto-commands */
    if (command_new) {
        prt("", 0, 0);
        cmd = command_new;
        command_new = 0;
    }

    /* Get a keypress in "command" mode */
    else {
        msg_flag = FALSE;
        inkey_flag = TRUE;
        cmd = inkey();
        inkey_flag = FALSE;
    }


    /* Special command -- Get a "count" for another command */
    if (cmd == '0') {

        /* Begin the input */
        prt("Repeat count:", 0, 0);

        /* Get a command count */
        while (1) {

            /* Get a new keypress */
            cmd = inkey();

            /* Simple editing */
            if (cmd == DELETE || cmd == CTRL('H')) {
                i = i / 10;
                prt(format("Repeat count: %d", i), 0, 0);
            }

            /* Actual numeric data */
            else if (cmd >= '0' && cmd <= '9') {

                /* Allow counts up to 9999 */
                if (i > 999) {
                    bell();
                }

                /* Incorporate that digit */
                else {
                    i = i * 10 + cmd - '0';
                    prt(format("Repeat count: %d", i), 0, 0);
                }
            }

            /* Exit on "unusable" input */
            else {
                break;
            }
        }

        /* Let a "non-count" default to 99 repetitions */
        if (i == 0) {
            i = 99;
            prt(format("Repeat count: %d", i), 0, 0);
        }

        /* Hack -- white-space means "enter command now" */
        if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) {
            if (!get_com("Command:", &cmd)) cmd = ESCAPE;
        }
    }

    /* Hack -- allow "control chars" to be entered */
    if (cmd == '^') {

        /* Get a char to "cast" into a control char */
        if (!get_com("Control-", &cmd)) cmd = ESCAPE;

        /* Hack -- create a control char if legal */
        else if (CTRL(cmd)) cmd = CTRL(cmd);
    }


    /* Hack -- allow "literal" angband chars */
    if (cmd == '\\') {

        /* Get a char to use without casting */
        if (!get_com("Command: ", &cmd)) cmd = ESCAPE;

        /* Save the character */
        command_cmd = cmd;
    }

    /* Otherwise, analyze the keypress */
    else {

        /* Analyze the keypress */
        keymap_apply(cmd);
    }


    /* Move cursor to player char again, in case it moved */
    move_cursor_relative(py, px);

    /* Hack -- let some commands get repeated by default */
    if (always_repeat && (i <= 0)) {

        /* Bash, Disarm, Open, Tunnel get 99 tries */
        if (strchr("BDo+", command_cmd)) i = 99;
    }

    /* Make sure a "Count" is legal for this command */
    if ((i > 0) && (command_cmd != ESCAPE)) {

        /* Commands that can be repeated */
        if (command_takes_rep(command_cmd)) {

            /* Save the count (this time counts) */
            command_rep = i - 1;

            /* Hack -- dump the count */
            p_ptr->redraw |= PR_STATE;

            /* Handle stuff */
            handle_stuff(TRUE);
        }

        /* Commands that take arguments */
        else if (command_takes_arg(command_cmd)) {

            /* Save the argument */
            command_arg = i;
        }

        /* Invalid combination */
        else {

            /* Abort gracefully */
            msg_print("Invalid command with a count.");

            /* Forget the command */
            command_cmd = ESCAPE;
        }
    }

    /* Hack -- erase the message line. */
    prt("", 0, 0);

    /* Hack -- Re-Illuminate the player */
    move_cursor_relative(py, px);
}



