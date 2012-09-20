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
 * Go up one level					-RAK-	
 */
void do_cmd_go_up(void)
{
    cave_type *c_ptr;

    /* Player grid */
    c_ptr = &cave[py][px];

    /* Verify stairs */
    if ((c_ptr->feat & 0x3F) != 0x06) {
        msg_print("I see no up staircase here.");
        return;
    }

    /* Hack -- take a turn */
    energy_use = 100;
    
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

    /* Player grid */
    c_ptr = &cave[py][px];

    /* Verify stairs */
    if ((c_ptr->feat & 0x3F) != 0x07) {
        msg_print("I see no down staircase here.");
        return;
    }

    /* Hack -- take a turn */
    energy_use = 100;
    
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
    int i;

    /* Flush input */
    flush();

    /* Verify Retirement */
    if (total_winner) {

        /* Verify */
        if (!get_check("Do you want to retire? ")) return;
    }

    /* Verify Suicide */
    else {

        /* Verify */
        if (!get_check("Do you really want to quit? ")) return;

        /* Special Verify */
        prt("Please verify QUITTING by typing the '@' sign: ", 0, 0);
        flush();
        i = inkey();
        prt("", 0, 0);
        if (i != '@') return;
    }

    /* Stop playing */
    alive = FALSE;

    /* Kill the player */
    death = TRUE;

    /* Cause of death */
    (void)strcpy(died_from, "Quitting");
}


/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 */
void do_cmd_redraw(void)
{
    /* Hack -- react to changes */
    Term_xtra(TERM_XTRA_REACT, 0);


    /* Verify the keymap */
    keymap_init();


#ifdef GRAPHIC_RECALL
    
    /* Hack -- erase the "recall" window */
    if (term_recall) {
        Term_activate(term_recall);
        Term_clear();
        Term_fresh();
    }

#endif
    
#ifdef GRAPHIC_CHOICE

    /* Hack -- erase the "choice" window */
    if (term_choice) {
        Term_activate(term_choice);
        Term_clear();
        Term_fresh();
    }

#endif

#ifdef GRAPHIC_MIRROR

    /* Hack -- erase the "mirror" window */
    if (term_mirror) {
        Term_activate(term_mirror);
        Term_clear();
        Term_fresh();
    }

#endif

    /* Activate the main screen */
    Term_activate(term_screen);

    /* Actual redraw */
    Term_redraw();


    /* Combine and Reorder the pack */
    p_ptr->update |= (PU_COMBINE | PU_REORDER);

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Update stuff */
    p_ptr->update |= (PU_NOTE | PU_VIEW | PU_LITE);
    
    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw everything */
    p_ptr->redraw |= (PR_WIPE | PR_MAP | PR_BASIC | PR_EXTRA);

    /* Redraw the recall window */
    p_ptr->redraw |= (PR_RECENT);
    
    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Hack -- update */
    handle_stuff();

    /* Hack -- refresh */
    Term_fresh();
}


/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
    char	c;

    bool	flag;

    bool	history = FALSE;
    
    char	tmp[160];


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Get command */
    for (flag = FALSE; !flag; ) {

        /* Display the player */
        display_player(history);

        /* Prompt */
        prt("['C' to change name, 'F' to dump file, 'H' for history, or ESCAPE]", 21, 2);

        /* Query */
        c = inkey();

        /* Handle */
        switch (c) {

          case 'C':
          case 'c':
            get_name();
            flag = TRUE;
            break;

          case 'F':
          case 'f':
            prt("File name: ", 0, 0);
            sprintf(tmp, "%s.txt", player_base);
            if (askfor_aux(tmp, 60) && tmp[0]) {
                file_character(tmp, FALSE);
            }
            break;

          case 'H':
          case 'h':
            history = !history;
            break;

          case ESCAPE:
          case ' ':
          case '\n':
          case '\r':
            flag = TRUE;
            break;

          default:
            bell();
            break;
        }
    }

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(void)
{
    if (p_ptr->searching) {
        search_off();
    }
    else {
        search_on();
    }
}



/*
 * An "item_tester_hook" for refilling lanterns
 */
static bool item_tester_refill_lantern(inven_type *i_ptr)
{
    /* Flasks of oil are okay */
    if (i_ptr->tval == TV_FLASK) return (TRUE);

    /* Assume not okay */
    return (FALSE);
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(void)
{
    int item;

    inven_type *i_ptr;
    inven_type *j_ptr;


    /* Restrict the choices */
    item_tester_hook = item_tester_refill_lantern;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Refill with which flask? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have no flasks of oil.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
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
    if (j_ptr->pval >= FUEL_LAMP) {
        j_ptr->pval = FUEL_LAMP;
        msg_print("Your lamp is full.");
    }

    /* Decrease the item (from the pack) */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else {
        floor_item_increase(0 - item, -1);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }
}



/*
 * An "item_tester_hook" for refilling torches
 */
static bool item_tester_refill_torch(inven_type *i_ptr)
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

    inven_type *i_ptr;
    inven_type *j_ptr;


    /* Restrict the choices */
    item_tester_hook = item_tester_refill_torch;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Refuel with which torch? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have no extra torches.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Take a partial turn */	
    energy_use = 50;

    /* Access the primary torch */
    j_ptr = &inventory[INVEN_LITE];

    /* Refuel */
    j_ptr->pval += i_ptr->pval;

    /* Message */
    msg_print("You combine the torches.");

    /* Over-fuel message */
    if (j_ptr->pval >= FUEL_TORCH) {
        j_ptr->pval = FUEL_TORCH;
        msg_print("Your torch is fully fueled.");
    }

    /* Refuel message */
    else {
        msg_print("Your torch glows more brightly.");
    }

    /* Decrease the item (from the pack) */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else {
        floor_item_increase(0 - item, -1);
        floor_item_describe(0 - item);
        floor_item_optimize(0 - item);
    }
}




/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refill(void)
{
    inven_type *i_ptr;

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
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
    /* Recall */
    prt(format("> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user	-BEN-
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * Hack -- attempt to combine identical messages ala "[xNN]" suffix,
 * as in "You hit the monster. [x4]", if the user has requested this.
 */
void do_cmd_messages(void)
{
    int i = 0, j = 0, k = 0, n = 0, t = 0;

    bool find = FALSE;
    
    char finder[80] = "";

    cptr str[MESSAGE_MAX];
    
    u16b cnt[MESSAGE_MAX];
    

    /* Total messages */
    t = message_num();

    /* Prepare the arrays */
    for (j = 0; j < t; j++) {

        /* Get the message */
        cptr msg = message_str(j);
        
        /* Process identical messages (if requested) */
        if (n && streq(str[n-1], msg) && optimize_various) {

            /* Count identical messages */
            cnt[n-1]++;
        }
        
        /* Process new messages */
        else {

            /* Remember the message */
            str[n] = msg;

            /* Count the message */
            cnt[n] = 0;
            
            /* Advance */
            n++;
        }
    }


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Process requests until done */
    while (1) {

        /* Clear the screen */
        clear_screen();

        /* Dump up to 20 lines of messages */
        for (j = 0; (j < 20) && (i + j < n); j++) {

            byte a = TERM_WHITE;

            /* Handle searches */
            if (find && strstr(str[i+j], finder)) a = TERM_YELLOW;

            /* Begin the next message */
            Term_gotoxy(0, 21-j);
            
            /* Dump the messages, bottom to top */
            Term_addstr(-1, a, str[i+j]);

            /* Hack -- add the message counter */
            if (cnt[i+j]) {

                /* Hack -- add the message counter */
                Term_addstr(-1, a, format(" [x%d]", cnt[i+j] + 1));
            }
        }

        /* Display header */
        prt(format("Message Recall (%d total, %d unique)", t, n), 0, 0);

        /* Display prompt */
        prt("[Press 'p' for older, 'n' for newer, '/' for search, or ESCAPE]", 23, 0);

        /* Get a command */
        k = inkey();

        /* Exit on Escape */
        if (k == ESCAPE) break;

        /* Hack -- Save the old index */
        j = i;

        /* Hack -- handle search */
        if (k == '/') {

            /* Cancel find */
            find = FALSE;

            /* Prompt */
            prt("Find: ", 23, 0);
                    
            /* Get a search stringm activate search */
            if (askfor_aux(finder, 80)) find = TRUE;
            
            /* Okay */
            continue;
        }

        /* Recall more older messages */
        if ((k == 'p') || (k == KTRL('P'))) {

            /* Go older if legal */
            if (i + 20 < n) i += 20;
        }

        /* Recall more newer messages */
        if ((k == 'n') || (k == KTRL('N'))) {

            /* Go newer if legal */
            if (i >= 20) i -= 20;
        }

        /* Hack -- Error of some kind */
        if (i == j) bell();
    }

    /* Restore the screen */
    Term_load();
    
    /* Leave "icky" mode */
    character_icky = FALSE;
}


/*
 * Target command
 */
void do_cmd_target(void)
{
    /* Set the target */
    if (target_set()) {
        msg_print("Target Selected.");
    }
    else {
        msg_print("Target Aborted.");
    }
}



/*
 * Interact with some options
 */
static void do_cmd_options_aux(int page, cptr info)
{
    char	ch;

    int		i, n = 0;

    int		opt[24];

    char	buf[80];


    /* Lookup the options */
    for (i = 0; i < 24; i++) opt[i] = 0;

    /* Scan the options */
    for (i = 0; options[i].o_desc; i++) {

        /* Notice options on this "page" */
        if (options[i].o_page == page) opt[n++] = i;
    }


    /* Clear the screen */
    clear_screen();

    /* Prompt */
    sprintf(buf, "%s (RET to advance, y/n to set, ESC to accept) ", info);
    prt(buf, 0, 0);

    /* Display the options */
    for (i = 0; i < n; i++) {
        sprintf(buf, "%-48s: %s ", options[opt[i]].o_desc,
                (*options[opt[i]].o_var ? "yes" : "no "));
        prt(buf, i + 2, 0);
    }


    /* Start at the first option */
    i = 0;

    /* Interact with the player */
    while (TRUE) {

        move_cursor(i + 2, 50);

        ch = inkey();

        switch (ch) {

          case ESCAPE:	
            return;

          case '-':
          case '8':
            i = (n + i - 1) % n;
            break;

          case ' ':
          case '\n':
          case '\r':
          case '2':
            i = (i + 1) % n;
            break;

          case 'y':
          case 'Y':
          case '6':
            put_str("yes ", i + 2, 50);
            (*options[opt[i]].o_var) = TRUE;
            i = (i + 1) % n;
            break;

          case 'n':
          case 'N':
          case '4':
            put_str("no  ", i + 2, 50);
            (*options[opt[i]].o_var) = FALSE;
            i = (i + 1) % n;
            break;

          default:
            bell();
            break;
        }
    }
}


/*
 * Set or unset various options.
 *
 * Hack -- user must sometimes hit "Ctrl-R" when done
 */
void do_cmd_options(void)
{
    int k;


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();


    /* Interact */
    while (1) {

        /* Clear screen */
        clear_screen();

        /* Why are we here */
        prt("Angband options", 2, 0);

        /* Give some choices */
        prt("(1) User Interface Options", 4, 5);
        prt("(2) Disturbance Options", 5, 5);
        prt("(3) Inventory Options", 6, 5);
        prt("(4) Game-Play Options", 7, 5);
        prt("(5) Efficiency Options", 8, 5);
        prt("(6) Special Options", 9, 5);

        /* Cheating */
        if (can_be_wizard) prt("(C) Cheating Options", 10, 5);

        /* Special choices */
        prt("(D) Base Delay Speed", 11, 5);
        prt("(H) Hitpoint Warning", 12, 5);

        /* Prompt */
        prt("Command: ", 14, 0);
        
        /* Get command */
        k = inkey();

        /* Exit */
        if (k == ESCAPE) break;

        /* General Options */
        if (k == '1') {

            /* Process the general options */
            do_cmd_options_aux(1, "User Interface Options");
        }

        /* Disturbance Options */
        else if (k == '2') {

            /* Process the running options */
            do_cmd_options_aux(2, "Disturbance Options");
        }

        /* Inventory Options */
        else if (k == '3') {

            /* Process the running options */
            do_cmd_options_aux(3, "Inventory Options");
        }

        /* Gameplay Options */
        else if (k == '4') {

            /* Process the game-play options */
            do_cmd_options_aux(4, "Game-Play Options");
        }

        /* Efficiency Options */
        else if (k == '5') {

            /* Process the efficiency options */
            do_cmd_options_aux(5, "Efficiency Options");
        }

        /* Efficiency Options */
        else if (k == '6') {

            /* Process the efficiency options */
            do_cmd_options_aux(6, "Special Options");
        }

        /* Cheating Options XXX XXX XXX */
        else if ((k == 'C') && can_be_wizard) {

            /* Process the cheating options */
            do_cmd_options_aux(255, "Cheating Options");

            /* Hack -- note use of "cheat" options */
            if (cheat_peek) noscore |= 0x0100;
            if (cheat_hear) noscore |= 0x0200;
            if (cheat_room) noscore |= 0x0400;
            if (cheat_xtra) noscore |= 0x0800;
            if (cheat_know) noscore |= 0x1000;
            if (cheat_live) noscore |= 0x2000;
        }

        /* Hack -- Delay Speed */
        else if (k == 'D') {

            /* Prompt */
            prt("Command: Base Delay Speed", 14, 0);
        
            /* Get a new value */
            while (1) {
                prt(format("Current delay speed: %d milliseconds",
                           delay_spd), 18, 0);
                prt("Delay Speed (0-9 or ESC to accept): ", 16, 0);
                k = inkey();
                if (k == ESCAPE) break;
                if (isdigit(k)) delay_spd = D2I(k);
                else bell();
            }
        }

        /* Hack -- hitpoint warning factor */
        else if (k == 'H') {

            /* Prompt */
            prt("Command: Hitpoint Warning", 14, 0);
        
            /* Get a new value */
            while (1) {
                prt(format("Current hitpoint warning: %d0%%",
                           hitpoint_warn), 18, 0);
                prt("Hitpoint Warning (0-9 or ESC to accept): ", 16, 0);
                k = inkey();
                if (k == ESCAPE) break;
                if (isdigit(k)) hitpoint_warn = D2I(k);
                else bell();
            }
        }

        /* Unknown option */
        else {

            /* Oops */
            bell();
        }
    }


    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}



/*
 * Ask for a "user pref line" and process it
 */
void do_cmd_pref(void)
{
    char buf[80];

    /* Wipe it */
    strcpy(buf, "");

    /* Prompt */
    prt("Pref: ", 0, 0);

    /* Ask */
    if (askfor_aux(buf, 80)) {

        /* Process XXX XXX XXX */
        (void)process_pref_file_aux(buf);
    }

    /* Clear */
    prt("", 0, 0);
}


#ifdef ALLOW_MACROS

/*
 * Hack -- append all current macros to the given file
 */
static errr macro_dump(cptr fname)
{
    int i;

    FILE *fff;

    char buf[1024];


    /* Build the filename */
    strcpy(buf, ANGBAND_DIR_USER);
    strcat(buf, fname);
    
#if defined(MACINTOSH) && !defined(applec)
    /* Global -- "text file" */
    _ftype = 'TEXT';
#endif

    /* Append to the file */
    fff = my_fopen(buf, "a");

    /* Failure */
    if (!fff) return (-1);


    /* Skip space */
    fprintf(fff, "\n\n");
    
    /* Start dumping */
    fprintf(fff, "# Automatic macro dump\n\n");

    /* Dump them */
    for (i = 0; i < macro__num; i++) {

        /* Start the macro */
        fprintf(fff, "# Macro '%d'\n\n", i);

        /* Extract the action */
        ascii_to_text(buf, macro__act[i]);

        /* Dump the macro */
        fprintf(fff, "A:%s\n", buf);

        /* Extract the action */
        ascii_to_text(buf, macro__pat[i]);

        /* Dump command macros */
        if (macro__cmd[i]) fprintf(fff, "C:%s\n", buf);

        /* Dump normal macros */
        else fprintf(fff, "P:%s\n", buf);

        /* End the macro */
        fprintf(fff, "\n\n");		
    }

    /* Start dumping */
    fprintf(fff, "\n\n\n\n");


    /* Close */
    my_fclose(fff);

    /* Success */
    return (0);
}


/*
 * Hack -- ask for a "trigger" (see below)
 *
 * Note the complex use of the "inkey()" function from "io.c".
 *
 * The "trigger" must be entered with less than 10 milliseconds
 * between keypresses, as in "inkey_aux()".
 */
static void do_cmd_macro_aux(char *buf)
{
    int i, n = 0, w = 0;

    bool skipping = FALSE;

    char tmp[1024];
    

    /* Important -- Flush input */
    flush();

    /* Do not process macros */
    inkey_base = TRUE;

    /* Attempt to read a key */
    i = inkey();

    /* Save the key */
    buf[n++] = i;

    /* Read the pattern */
    while (TRUE) {

        /* Do not process macros */
        inkey_base = TRUE;

        /* Do not wait for keys */
        inkey_scan = TRUE;

        /* Attempt to read a key */
        i = inkey();

        /* If a key is ready, acquire it */
        if (i) {

            /* Reset delay */
            w = 0;

            /* Hack -- process "ascii 28" */
            if (i == 28) {

                /* Toggle the "skipping" flag */
                skipping = !skipping;
            }

            /* Save usable keys */
            else if (!skipping) {

                /* Save the key */
                buf[n++] = i;
            }

            /* Next */
            continue;
        }

        /* Excessive delay */
        if (w > 30) break;

        /* Increment and delay */
        delay(++w);
    }

    /* Terminate */
    buf[n] = '\0';

    /* Important -- Flush the input */
    flush();
    
    /* Convert the trigger */
    ascii_to_text(tmp, buf);
    
    /* Hack -- display the trigger */
    Term_addstr(-1, TERM_WHITE, tmp);
}

#endif


/*
 * Interact with "macros"
 *
 * Note that the macro "action" must be defined before the trigger.
 *
 * XXX XXX XXX Need messages for success, plus "helpful" info
 */
void do_cmd_macros(void)
{
    int i;

    char tmp[160];

    char buf[1024];


#if defined(MACINTOSH) && !defined(applec)
    _ftype = 'TEXT';
#endif


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Process requests until done */
    while (1) {

        /* Clear the screen */
        clear_screen();

        /* Describe */
        Term_putstr(0, 2, -1, TERM_WHITE, "Interact with Macros");


        /* Describe that action */
        Term_putstr(0, 18, -1, TERM_WHITE, "Current action (if any) shown below:");

        /* Analyze the current action */
        ascii_to_text(buf, macro__buf);

        /* Display the current action */
        Term_putstr(0, 20, -1, TERM_WHITE, buf);


        /* Selections */
        Term_putstr(5,  4, -1, TERM_WHITE, "(1) Load a user pref file");
#ifdef ALLOW_MACROS
        Term_putstr(5,  5, -1, TERM_WHITE, "(2) Dump macros");
        Term_putstr(5,  6, -1, TERM_WHITE, "(3) Enter a new action");
        Term_putstr(5,  7, -1, TERM_WHITE, "(4) Query a macro action");
        Term_putstr(5,  8, -1, TERM_WHITE, "(5) Create a command macro");
        Term_putstr(5,  9, -1, TERM_WHITE, "(6) Create a normal macro");
        Term_putstr(5, 10, -1, TERM_WHITE, "(7) Create an identity macro");
        Term_putstr(5, 11, -1, TERM_WHITE, "(8) Create an empty macro");
        Term_putstr(5, 12, -1, TERM_WHITE, "(9) Define a special keymap");
#endif /* ALLOW_MACROS */

        /* Prompt */
        Term_putstr(0, 14, -1, TERM_WHITE, "Command: ");
        
        /* Get a command */
        i = inkey();

        /* Leave */
        if (i == ESCAPE) break;

        /* Load a 'macro' file */
        else if (i == '1') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Load a user pref file");

            /* Get a filename, handle ESCAPE */
            Term_putstr(0, 16, -1, TERM_WHITE, "File: ");

            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Ask for a file */
            if (!askfor_aux(tmp, 70)) continue;

            /* Process the given filename */
            (void)process_pref_file(tmp);
        }

#ifdef ALLOW_MACROS

        /* Save a 'macro' file */
        else if (i == '2') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Save a macro file");

            /* Get a filename, handle ESCAPE */
            Term_putstr(0, 16, -1, TERM_WHITE, "File: ");

            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Ask for a file */
            if (!askfor_aux(tmp, 70)) continue;

            /* Drop priv's */
            safe_setuid_drop();

            /* Dump the macros */
            (void)macro_dump(tmp);

            /* Grab priv's */
            safe_setuid_grab();
        }
        
        /* Enter a new action */
        else if (i == '3') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Enter a new action");
            
            /* Go to the correct location */
            Term_gotoxy(0, 20);

            /* Hack -- limit the value */
            tmp[80] = '\0';

            /* Get an encoded action */
            if (askfor_aux(buf, 80)) text_to_ascii(macro__buf, buf);
        }

        /* Query a macro action */
        else if (i == '4') {

#if 0
            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Query a macro action");

            /* Prompt */
            Term_putstr(0, 16, -1, TERM_WHITE, "Enter a macro trigger: ");

            /* Get a macro trigger */
            do_cmd_macro_aux(buf);
#endif

            /* XXX XXX XXX */
            msg_print("Command not ready.");
        }
        
        /* Create a command macro */
        else if (i == '5') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create a command macro");

            /* Prompt */
            Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

            /* Get a macro trigger */
            do_cmd_macro_aux(buf);
            
            /* Link the macro */
            macro_add(buf, macro__buf, TRUE);
            
            /* Message */
            msg_print("Created a new command macro.");
        }
        
        /* Create a normal macro */
        else if (i == '6') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create a normal macro");

            /* Prompt */
            Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

            /* Get a macro trigger */
            do_cmd_macro_aux(buf);
            
            /* Link the macro */
            macro_add(buf, macro__buf, FALSE);
            
            /* Message */
            msg_print("Created a new normal macro.");
        }
        
        /* Create an identity macro */
        else if (i == '7') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create an identity macro");

            /* Prompt */
            Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

            /* Get a macro trigger */
            do_cmd_macro_aux(buf);
            
            /* Link the macro */
            macro_add(buf, buf, FALSE);
            
            /* Message */
            msg_print("Created a new identity macro.");
        }
        
        /* Create an empty macro */
        else if (i == '8') {

            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Create an empty macro");

            /* Prompt */
            Term_putstr(0, 16, -1, TERM_WHITE, "Trigger: ");

            /* Get a macro trigger */
            do_cmd_macro_aux(buf);
            
            /* Link the macro */
            macro_add(buf, "", FALSE);
            
            /* Message */
            msg_print("Created a new empty macro.");
        }

        /* Define a keymap */
        else if (i == '9') {

            char i1, i2, i3;

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
                msg_format("Keypress 0x%02x mapped to 0x%02x/%d",
                           k, keymap_cmds[k], keymap_dirs[k]);
            }
        }

#endif /* ALLOW_MACROS */

        /* Oops */
        else {

            /* Oops */        
            bell();
        }
        
        /* Flush messages */
        msg_print(NULL);
    }
    

    /* Load the screen */
    Term_load();
    
    /* Leave "icky" mode */
    character_icky = FALSE;
}



/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
    int i;

    FILE *fff;
    
    char tmp[160];

    char buf[1024];


#if defined(MACINTOSH) && !defined(applec)
    _ftype = 'TEXT';
#endif


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();


    /* Interact until done */
    while (1) {

        /* Clear the screen */
        clear_screen();

        /* Ask for a choice */
        prt("Interact with Visuals", 2, 0);

        /* Give some choices */
        prt("(1) Load a user pref file", 4, 5);
#ifndef ANGBAND_LITE
        prt("(2) Dump monster attr/chars", 5, 5);
        prt("(3) Dump object attr/chars", 6, 5);
        prt("(4) Dump feature attr/chars", 7, 5);
        prt("(5) (unused)", 8, 5);
        prt("(6) Change monster attr/chars", 9, 5);
        prt("(7) Change object attr/chars", 10, 5);
        prt("(8) Change feature attr/chars", 11, 5);
        prt("(9) (unused)", 12, 5);
#endif /* ANGBAND_LITE */
        prt("(0) Reset visuals", 13, 5);

        /* Prompt */
        prt("Command: ", 15, 0);

        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;

        /* Load a 'pref' file */
        else if (i == '1') {

            /* Prompt */
            prt("Command: Load a user pref file", 15, 0);

            /* Prompt */
            prt("File: ", 17, 0);
            
            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Query */
            if (!askfor_aux(tmp, 70)) continue;

            /* Process the given filename */
            (void)process_pref_file(tmp);
        }

#ifndef ANGBAND_LITE

        /* Dump monster attr/chars */
        else if (i == '2') {

            /* Prompt */
            prt("Command: Dump monster attr/chars", 15, 0);

            /* Prompt */
            prt("File: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Get a filename */
            if (!askfor_aux(tmp, 70)) continue;

            /* Build the filename */
            strcpy(buf, ANGBAND_DIR_USER);
            strcat(buf, tmp);
    
            /* Drop priv's */
            safe_setuid_drop();

            /* Append to the file */
            fff = my_fopen(buf, "a");

            /* Grab priv's */
            safe_setuid_grab();

            /* Failure */
            if (!fff) continue;

            /* Start dumping */
            fprintf(fff, "\n\n");
            fprintf(fff, "# Monster attr/char definitions\n\n");

            /* Dump monsters */
            for (i = 0; i < MAX_R_IDX; i++) {

                monster_race *r_ptr = &r_info[i];

                /* Skip non-entries */
                if (!r_ptr->name) continue;

                /* Dump a comment */
                fprintf(fff, "# %s\n", (r_name + r_ptr->name));
                
                /* Dump the monster attr/char info */
                fprintf(fff, "R:%d:0x%02X:0x%02X\n\n", i,
                        (byte)(r_ptr->l_attr), (byte)(r_ptr->l_char));
            }

            /* All done */
            fprintf(fff, "\n\n\n\n");

            /* Close */
            my_fclose(fff);

            /* Message */
            msg_print("Dumped monster attr/chars.");
        }

        /* Dump object attr/chars */
        else if (i == '3') {

            /* Prompt */
            prt("Command: Dump object attr/chars", 15, 0);

            /* Prompt */
            prt("File: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Get a filename */
            if (!askfor_aux(tmp, 70)) continue;

            /* Build the filename */
            strcpy(buf, ANGBAND_DIR_USER);
            strcat(buf, tmp);
    
            /* Drop priv's */
            safe_setuid_drop();

            /* Append to the file */
            fff = my_fopen(buf, "a");

            /* Grab priv's */
            safe_setuid_grab();

            /* Failure */
            if (!fff) continue;

            /* Start dumping */
            fprintf(fff, "\n\n");
            fprintf(fff, "# Object attr/char definitions\n\n");

            /* Dump objects */
            for (i = 0; i < MAX_K_IDX; i++) {

                inven_kind *k_ptr = &k_info[i];

                /* Skip non-entries */
                if (!k_ptr->name) continue;

                /* Dump a comment */
                fprintf(fff, "# %s\n", (k_name + k_ptr->name));
                
                /* Dump the object attr/char info */
                fprintf(fff, "K:%d:0x%02X:0x%02X\n\n", i,
                        (byte)(k_ptr->x_attr), (byte)(k_ptr->x_char));
            }

            /* All done */
            fprintf(fff, "\n\n\n\n");

            /* Close */
            my_fclose(fff);

            /* Message */
            msg_print("Dumped object attr/chars.");
        }
        
        /* Dump feature attr/chars */
        else if (i == '4') {

            /* Prompt */
            prt("Command: Dump feature attr/chars", 15, 0);

            /* Prompt */
            prt("File: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Get a filename */
            if (!askfor_aux(tmp, 70)) continue;

            /* Build the filename */
            strcpy(buf, ANGBAND_DIR_USER);
            strcat(buf, tmp);
    
            /* Drop priv's */
            safe_setuid_drop();

            /* Append to the file */
            fff = my_fopen(buf, "a");

            /* Grab priv's */
            safe_setuid_grab();

            /* Failure */
            if (!fff) continue;

            /* Start dumping */
            fprintf(fff, "\n\n");
            fprintf(fff, "# Feature attr/char definitions\n\n");

            /* Dump features */
            for (i = 0; i < MAX_F_IDX; i++) {

                feature_type *f_ptr = &f_info[i];

                /* Skip non-entries */
                if (!f_ptr->name) continue;

                /* Dump a comment */
                fprintf(fff, "# %s\n", (f_name + f_ptr->name));
                
                /* Dump the feature attr/char info */
                fprintf(fff, "F:%d:0x%02X:0x%02X\n\n", i,
                        (byte)(f_ptr->z_attr), (byte)(f_ptr->z_char));
            }

            /* All done */
            fprintf(fff, "\n\n\n\n");

            /* Close */
            my_fclose(fff);

            /* Message */
            msg_print("Dumped feature attr/chars.");
        }

        /* Modify monster attr/chars */
        else if (i == '6') {

            static int r = 0;

            /* Prompt */
            prt("Command: Change monster attr/chars", 15, 0);

            /* Hack -- query until done */
            while (1) {

                monster_race *r_ptr = &r_info[r];

                int da = (byte)(r_ptr->r_attr);
                int dc = (byte)(r_ptr->r_char);
                int ca = (byte)(r_ptr->l_attr);
                int cc = (byte)(r_ptr->l_char);

                /* Label the object */
                Term_putstr(5, 17, -1, TERM_WHITE,
                            format("Monster = %d, Name = %-40.40s",
                                   r, (r_name + r_ptr->name)));

                /* Label the Default values */
                Term_putstr(10, 19, -1, TERM_WHITE,
                            format("Default attr/char = %3u / %3u", da, dc));
                Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 19, da, dc);

                /* Label the Current values */
                Term_putstr(10, 20, -1, TERM_WHITE,
                            format("Current attr/char = %3u / %3u", ca, cc));
                Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 20, ca, cc);

                /* Prompt */
                Term_putstr(0, 22, -1, TERM_WHITE,
                            "Command (n/N/a/A/c/C): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') r = (r + MAX_R_IDX + 1) % MAX_R_IDX;
                if (i == 'N') r = (r + MAX_R_IDX - 1) % MAX_R_IDX;
                if (i == 'a') r_ptr->l_attr = (byte)(ca + 1);
                if (i == 'A') r_ptr->l_attr = (byte)(ca - 1);
                if (i == 'c') r_ptr->l_char = (byte)(cc + 1);
                if (i == 'C') r_ptr->l_char = (byte)(cc - 1);
            }
        }

        /* Modify object attr/chars */
        else if (i == '7') {

            static int k = 0;

            /* Prompt */
            prt("Command: Change object attr/chars", 15, 0);

            /* Hack -- query until done */
            while (1) {

                inven_kind *k_ptr = &k_info[k];

                int da = (byte)k_ptr->k_attr;
                int dc = (byte)k_ptr->k_char;
                int ca = (byte)k_ptr->x_attr;
                int cc = (byte)k_ptr->x_char;

                /* Label the object */
                Term_putstr(5, 17, -1, TERM_WHITE,
                            format("Object = %d, Name = %-40.40s",
                                   k, (k_name + k_ptr->name)));

                /* Label the Default values */
                Term_putstr(10, 19, -1, TERM_WHITE,
                            format("Default attr/char = %3d / %3d", da, dc));
                Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 19, da, dc);

                /* Label the Current values */
                Term_putstr(10, 20, -1, TERM_WHITE,
                            format("Current attr/char = %3d / %3d", ca, cc));
                Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 20, ca, cc);

                /* Prompt */
                Term_putstr(0, 22, -1, TERM_WHITE,
                            "Command (n/N/a/A/c/C): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') k = (k + MAX_K_IDX + 1) % MAX_K_IDX;
                if (i == 'N') k = (k + MAX_K_IDX - 1) % MAX_K_IDX;
                if (i == 'a') k_info[k].x_attr = (byte)(ca + 1);
                if (i == 'A') k_info[k].x_attr = (byte)(ca - 1);
                if (i == 'c') k_info[k].x_char = (byte)(cc + 1);
                if (i == 'C') k_info[k].x_char = (byte)(cc - 1);
            }
        }

        /* Modify feature attr/chars */
        else if (i == '8') {

            static int f = 0;

            /* Prompt */
            prt("Command: Change feature attr/chars", 15, 0);

            /* Hack -- query until done */
            while (1) {

                feature_type *f_ptr = &f_info[f];

                int da = (byte)f_ptr->f_attr;
                int dc = (byte)f_ptr->f_char;
                int ca = (byte)f_ptr->z_attr;
                int cc = (byte)f_ptr->z_char;

                /* Label the object */
                Term_putstr(5, 17, -1, TERM_WHITE,
                            format("Terrain = %d, Name = %-40.40s",
                                   f, (f_name + f_ptr->name)));

                /* Label the Default values */
                Term_putstr(10, 19, -1, TERM_WHITE,
                            format("Default attr/char = %3d / %3d", da, dc));
                Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 19, da, dc);

                /* Label the Current values */
                Term_putstr(10, 20, -1, TERM_WHITE,
                            format("Current attr/char = %3d / %3d", ca, cc));
                Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
                Term_putch(43, 20, ca, cc);

                /* Prompt */
                Term_putstr(0, 22, -1, TERM_WHITE,
                            "Command (n/N/a/A/c/C): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') f = (f + MAX_F_IDX + 1) % MAX_F_IDX;
                if (i == 'N') f = (f + MAX_F_IDX - 1) % MAX_F_IDX;
                if (i == 'a') f_info[f].z_attr = (byte)(ca + 1);
                if (i == 'A') f_info[f].z_attr = (byte)(ca - 1);
                if (i == 'c') f_info[f].z_char = (byte)(cc + 1);
                if (i == 'C') f_info[f].z_char = (byte)(cc - 1);
            }
        }

#endif /* ANGBAND_LITE */

        /* Reset visuals */
        else if (i == '0') {

            /* Reset */
            reset_visuals();

            /* Message */
            msg_print("Visual attr/char tables reset.");
        }

        /* Unknown option */
        else {
            bell();
        }
    }


    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
    int i;

    FILE *fff;
    
    char tmp[160];
    
    char buf[1024];


#if defined(MACINTOSH) && !defined(applec)
    _ftype = 'TEXT';
#endif


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();


    /* Interact until done */
    while (1) {

        /* Clear the screen */
        clear_screen();

        /* Ask for a choice */
        prt("Interact with Colors", 2, 0);

        /* Give some choices */
        prt("(1) Load a user pref file", 4, 5);
#ifndef ANGBAND_LITE
        prt("(2) Dump colors", 5, 5);
        prt("(3) Modify colors", 6, 5);
#endif /* ANGBAND_LITE */
        
        /* Prompt */
        prt("Command: ", 8, 0);

        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;

        /* Load a 'pref' file */
        if (i == '1') {

            /* Prompt */
            prt("Command: Load a user pref file", 8, 0);

            /* Prompt */
            prt("File: ", 10, 0);
            
            /* Default file */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Query */
            if (!askfor_aux(tmp, 70)) continue;

            /* Process the given filename */
            (void)process_pref_file(tmp);

            /* Mega-Hack -- react to changes */
            Term_xtra(TERM_XTRA_REACT, 0);

            /* Mega-Hack -- redraw */
            Term_redraw();
        }

#ifndef ANGBAND_LITE

        /* Dump colors */
        else if (i == '2') {

            /* Prompt */
            prt("Command: Dump colors", 8, 0);

            /* Prompt */
            prt("File: ", 10, 0);

            /* Default filename */
            sprintf(tmp, "user-%s.prf", ANGBAND_SYS);

            /* Get a filename */
            if (!askfor_aux(tmp, 70)) continue;

            /* Build the filename */
            strcpy(buf, ANGBAND_DIR_USER);
            strcat(buf, tmp);
    
            /* Drop priv's */
            safe_setuid_drop();

            /* Append to the file */
            fff = my_fopen(buf, "a");

            /* Grab priv's */
            safe_setuid_grab();

            /* Failure */
            if (!fff) continue;

            /* Start dumping */
            fprintf(fff, "\n\n");
            fprintf(fff, "# Color redefinitions\n\n");

            /* Dump colors */
            for (i = 0; i < 256; i++) {

                int kv = color_table[i][0];
                int rv = color_table[i][1];
                int gv = color_table[i][2];
                int bv = color_table[i][3];
                
                cptr name = "unknown";

                /* Skip non-entries */
                if (!kv && !rv && !gv && !bv) continue;

                /* Extract the color name */
                if (i < 16) name = color_names[i];
                
                /* Dump a comment */
                fprintf(fff, "# Color '%s'\n", name);
                
                /* Dump the monster attr/char info */
                fprintf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
                        i, kv, rv, gv, bv);
            }

            /* All done */
            fprintf(fff, "\n\n\n\n");

            /* Close */
            my_fclose(fff);

            /* Message */
            msg_print("Dumped color redefinitions.");
        }

        /* Edit colors */
        else if (i == '3') {

            static int a = 0;

            /* Prompt */
            prt("Command: Modify colors", 8, 0);

            /* Hack -- query until done */
            while (1) {

                cptr name;
                
                /* Clear */
                clear_from(10);
                
                /* Exhibit the normal colors */
                for (i = 0; i < 16; i++) {

                    /* Exhibit this color */
                    Term_putstr(i*4, 20, -1, a, "###");

                    /* Exhibit all colors */
                    Term_putstr(i*4, 22, -1, i, format("%3d", i));
                }

                /* Describe the color */
                name = ((a < 16) ? color_names[a] : "undefined");

                /* Describe the color */
                Term_putstr(5, 10, -1, TERM_WHITE,
                            format("Color = %d, Name = %s", a, name));

                /* Label the Current values */
                Term_putstr(5, 12, -1, TERM_WHITE,
                            format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
                                   color_table[a][0], color_table[a][1],
                                   color_table[a][2], color_table[a][3]));

                /* Prompt */
                Term_putstr(0, 14, -1, TERM_WHITE,
                            "Command (n/N/k/K/r/R/g/G/b/B): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') a = (byte)(a + 1);
                if (i == 'N') a = (byte)(a - 1);
                if (i == 'k') color_table[a][0] = (byte)(color_table[a][0] + 1);
                if (i == 'K') color_table[a][0] = (byte)(color_table[a][0] - 1);
                if (i == 'r') color_table[a][1] = (byte)(color_table[a][1] + 1);
                if (i == 'R') color_table[a][1] = (byte)(color_table[a][1] - 1);
                if (i == 'g') color_table[a][2] = (byte)(color_table[a][2] + 1);
                if (i == 'G') color_table[a][2] = (byte)(color_table[a][2] - 1);
                if (i == 'b') color_table[a][3] = (byte)(color_table[a][3] + 1);
                if (i == 'B') color_table[a][3] = (byte)(color_table[a][3] - 1);
                
                /* Hack -- react to changes */
                Term_xtra(TERM_XTRA_REACT, 0);

                /* Hack -- redraw */
                Term_redraw();
            }
        }

#endif /* ANGBAND_LITE */

        /* Unknown option */
        else {
            bell();
        }
    }


    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}


/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
    char buf[80];

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
    /* Silly message */
    msg_format("You are playing Angband %d.%d.%d.  Type '?' for more info.",
               VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}



/*
 * Save the game
 */
void do_cmd_save_game(void)
{
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

    /* Save the player */
    if (save_player()) {
        prt("Saving game... done.", 0, 0);
    }

    /* Save failed (oops) */
    else {
        prt("Saving game... failed!", 0, 0);
    }
    
    /* Refresh */
    Term_fresh();

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
    int			item, amt = 1;
    int			old_number;

    inven_type		*i_ptr;

    char		i_name[80];

    char		out_val[160];


    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Destroy which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to destroy.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* See how many items */
    if (i_ptr->number > 1) {

        /* Get a quantity */
        amt = get_quantity(NULL, i_ptr->number);

        /* Allow user abort */
        if (amt <= 0) return;
    }


    /* Describe the object */
    old_number = i_ptr->number;
    i_ptr->number = amt;
    objdes(i_name, i_ptr, TRUE, 3);
    i_ptr->number = old_number;

    /* Make a verification */
    sprintf(out_val, "Really destroy %s? ", i_name);
    if (!get_check(out_val)) return;


    /* Take a turn */
    energy_use = 100;

    /* Artifacts cannot be destroyed */
    if (artifact_p(i_ptr)) {

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
        p_ptr->update |= (PU_COMBINE);

        /* Done */
        return;
    }

    /* Message */
    msg_format("You destroy %s.", i_name);

    /* Eliminate the item (from the pack) */
    if (item >= 0) {
        inven_item_increase(item, -amt);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Eliminate the item (from the floor) */
    else {
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

    inven_type		*i_ptr;

    char		i_name[80];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Examine which item? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to examine.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


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
 * Move an item from equipment list to pack
 * Note that only one item at a time can be wielded per slot.
 * Note that taking off an item when "full" will cause that item
 * to fall to the ground.
 */
static void inven_takeoff(int item, int amt)
{
    int			posn;

    inven_type		*i_ptr;
    inven_type		tmp_obj;

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
    if (amt < i_ptr->number) {
        act = "Took off";
    }
    else if (item == INVEN_WIELD) {
        act = "Was wielding";
    }
    else if (item == INVEN_BOW) {
        act = "Was shooting with";
    }
    else if (item == INVEN_LITE) {
        act = "Light source was";
    }
    else {
        act = "Was wearing";
    }

    /* Carry the object, saving the slot it went in */
    posn = inven_carry(&tmp_obj);

    /* Describe the result */
    objdes(i_name, i_ptr, TRUE, 3);

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
    inven_type		*i_ptr;
    inven_type		 tmp_obj;

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
    if (amt < i_ptr->number) {
        act = "Dropped";
    }
    else if (item == INVEN_WIELD) {
        act = "Was wielding";
    }
    else if (item == INVEN_BOW) {
        act = "Was shooting with";
    }
    else if (item == INVEN_LITE) {
        act = "Light source was";
    }
    else if (item >= INVEN_WIELD) {
        act = "Was wearing";
    }
    else {
        act = "Dropped";
    }

    /* Message */
    objdes(i_name, &tmp_obj, TRUE, 3);

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
    if (command_new == ESCAPE) {

        /* Reset stuff */
        command_new = 0;
        command_gap = 50;
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
    if (command_new == ESCAPE) {

        /* Reset stuff */
        command_new = 0;
        command_gap = 50;
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
    inven_type tmp_obj;
    inven_type *i_ptr;

    cptr act;

    char i_name[80];


    /* Restrict the choices */
    item_tester_hook = item_tester_hook_wear;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Wear/Wield which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing you can wear or wield.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Check the slot */
    slot = wield_slot(i_ptr);

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

    /* Hack -- Verify potential overflow */
    if ((inven_cnt >= INVEN_PACK) &&
        ((item < 0) || (i_ptr->number > 1))) {

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
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_optimize(item);
    }

    /* Decrease the item (from the floor) */
    else {
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
    if (slot == INVEN_WIELD) {
        act = "You are wielding";
    }
    else if (slot == INVEN_BOW) {
        act = "You are shooting with";
    }
    else if (slot == INVEN_LITE) {
        act = "Your light source is";
    }
    else {
        act = "You are wearing";
    }

    /* Describe the result */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("%^s %s (%c).", act, i_name, index_to_label(slot));

    /* Cursed! */
    if (cursed_p(i_ptr)) {

        /* Warn the player */
        msg_print("Oops! It feels deathly cold!");

        /* Note the curse */
        i_ptr->ident |= ID_SENSE;
    }

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Recalculate mana */
    p_ptr->update |= (PU_MANA);

    /* Redraw equippy chars */
    p_ptr->redraw |= (PR_EQUIPPY);
    
    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);
}



/*
 * Take off an item
 */
void do_cmd_takeoff(void)
{
    int item;

    inven_type *i_ptr;


    /* Hack -- verify potential overflow */
    if (inven_cnt >= INVEN_PACK) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack may overflow.  Continue? ")) return;
    }


    /* Get an item (from equip) */
    if (!get_item(&item, "Take off which item? ", TRUE, FALSE, FALSE)) {
        if (item == -2) msg_print("You are not wearing anything to take off.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Item is cursed */
    if (cursed_p(i_ptr)) {

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

    inven_type *i_ptr;


    /* Get an item (from equip or inven) */
    if (!get_item(&item, "Drop which item? ", TRUE, TRUE, FALSE)) {
        if (item == -2) msg_print("You have nothing to drop.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Cannot remove cursed items */
    if ((item >= INVEN_WIELD) && cursed_p(i_ptr)) {

        /* Oops */
        msg_print("Hmmm, it seems to be cursed.");
        
        /* Nope */
        return;
    }


    /* See how many items */
    if (i_ptr->number > 1) {

        /* Get a quantity */
        amt = get_quantity(NULL, i_ptr->number);

        /* Allow user abort */
        if (amt <= 0) return;
    }


    /* Mega-Hack -- verify "dangerous" drops */
    if (cave[py][px].i_idx) {

        /* XXX XXX Verify with the player */
        if (other_query_flag &&
            !get_check("The item may disappear.  Continue? ")) return;
    }


    /* Take a partial turn */
    energy_use = 50;

    /* Drop (some of) the item */
    inven_drop(item, amt);
}



#ifndef ANGBAND_LITE


/*
 * Encode the screen colors
 */
static char hack[17] = "dwsorgbuDWvyRGBU";


/*
 * Hack -- load a screen dump from a file
 */
void do_cmd_load_screen(void)
{
    int i, y, x;

    byte a = 0;
    char c = ' ';

    bool okay = TRUE;

    FILE *fff;

    char buf[1024];


    /* Build the path name */
    strcpy(buf, ANGBAND_DIR_USER);
    strcat(buf, "dump.txt");

    /* Append to the file */
    fff = my_fopen(buf, "r");

    /* Oops */
    if (!fff) return;


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();
    
    /* Clear the screen */
    Term_clear();

    
    /* Load the screen */
    for (y = 0; okay && (y < 24); y++) {

        /* Get a line of data */
        if (my_fgets(fff, buf, 1024)) okay = FALSE;
        
        /* Show each row */
        for (x = 0; x < 79; x++) {

            /* Put the attr/char */
            Term_draw(x, y, TERM_WHITE, buf[x]);
        }
    }

    /* Get the blank line */
    if (my_fgets(fff, buf, 1024)) okay = FALSE;


    /* Dump the screen */
    for (y = 0; okay && (y < 24); y++) {

        /* Get a line of data */
        if (my_fgets(fff, buf, 1024)) okay = FALSE;
        
        /* Dump each row */
        for (x = 0; x < 79; x++) {

            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Look up the attr */
            for (i = 0; i < 16; i++) {
            
                /* Use attr matches */
                if (hack[i] == buf[x]) a = i;
            }

            /* Put the attr/char */
            Term_draw(x, y, a, c);
        }
            
        /* End the row */
        fprintf(fff, "\n");
    }

    /* Get the blank line */
    if (my_fgets(fff, buf, 1024)) okay = FALSE;


    /* Close it */
    my_fclose(fff);


    /* Message */
    msg_print("Screen dump loaded.");
    msg_print(NULL);


    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}


/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
    int y, x;

    byte a = 0;
    char c = ' ';
    
    FILE *fff;

    char buf[1024];


    /* Build the path name */
    strcpy(buf, ANGBAND_DIR_USER);
    strcat(buf, "dump.txt");

#if defined(MACINTOSH) && !defined(applec)
    _ftype = 'TEXT';
#endif

    /* Hack -- drop permissions */
    safe_setuid_drop();

    /* Append to the file */
    fff = my_fopen(buf, "w");

    /* Hack -- grab permissions */
    safe_setuid_grab();

    /* Oops */
    if (!fff) return;


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();
    

    /* Dump the screen */
    for (y = 0; y < 24; y++) {

        /* Dump each row */
        for (x = 0; x < 79; x++) {

            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Dump it */
            buf[x] = c;
        }

        /* Terminate */
        buf[x] = '\0';
        
        /* End the row */
        fprintf(fff, "%s\n", buf);
    }

    /* Skip a line */
    fprintf(fff, "\n");


    /* Dump the screen */
    for (y = 0; y < 24; y++) {

        /* Dump each row */
        for (x = 0; x < 79; x++) {

            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Dump it */
            buf[x] = hack[a&0x0F];
        }
        
        /* Terminate */
        buf[x] = '\0';
        
        /* End the row */
        fprintf(fff, "%s\n", buf);
    }

    /* Skip a line */
    fprintf(fff, "\n");


    /* Close it */
    my_fclose(fff);


    /* Message */
    msg_print("Screen dump saved.");
    msg_print(NULL);


    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}


#endif

