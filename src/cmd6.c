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



#ifdef ALLOW_WIZARD

/*
 * Hack -- Declare the Wizard Routines
 */
extern int do_wiz_command(void);

#endif


#ifdef ALLOW_BORG

/*
 * Hack -- Declare the Ben Borg
 */
extern void borg_ben(void);

#endif



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
            break;

        /* (SPACE) do nothing */
        case ' ':
            break;



        /*** Wizard Commands ***/

        /* Toggle Wizard Mode */
        case KTRL('W'):
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

            /* Redraw "title" */
            p_ptr->redraw |= (PR_TITLE);

            break;


#ifdef ALLOW_WIZARD

        /* Special Wizard Command */
        case KTRL('A'):
            if (wizard) {
                do_wiz_command();
            }
            else {
                msg_print("You are not allowed to do that.");
            }
            break;

#endif


#ifdef ALLOW_BORG

        /* Interact with the Borg */
        case KTRL('Z'):

            /* Interact with the borg */
            borg_ben();

            break;

#endif



        /*** Inventory Commands ***/

        /* Wear/wield equipment */
        case '[':
            do_cmd_wield(); break;

        /* Take off equipment */
        case ']':
            do_cmd_takeoff(); break;

        /* Drop an item */
        case 'd':
            do_cmd_drop(); break;

        /* Destroy an item */
        case 'k':
            do_cmd_destroy(); break;

        /* Equipment list */
        case 'e':
            do_cmd_equip(); break;

        /* Inventory list */
        case 'i':
            do_cmd_inven(); break;


        /*** Various commands ***/

        /* Identify an object */
        case 'I':
            do_cmd_observe(); break;

        /* Hack -- toggle choice window */
        case KTRL('E'):
            do_cmd_toggle_choose(); break;


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


        /*** Running, Resting, Searching, Staying */

        /* Begin Running -- Arg is Max Distance */
        case '.':
            do_cmd_run(); break;

        /* Stay still (usually pick things up) */
        case ',':
            do_cmd_stay(always_pickup); break;

        /* Stay still (usually do not pick up) */
        case 'g':
            do_cmd_stay(!always_pickup); break;

        /* Rest -- Arg is time */
        case 'R':
            do_cmd_rest(); break;

        /* Search for traps/doors */
        case 's':
            do_cmd_search(); break;

        /* Toggle search mode */
        case 'S':
            do_cmd_toggle_search(); break;


        /*** Stairs and Doors and Chests and Traps ***/

        /* Go up staircase */
        case '<':
            do_cmd_go_up(); break;

        /* Go down staircase */
        case '>':
            do_cmd_go_down(); break;

        /* Open a door or chest */
        case 'o':
            do_cmd_open(); break;

        /* Close a door */
        case 'c':
            do_cmd_close(); break;

        /* Jam a door with spikes */
        case 'j':
            do_cmd_spike(); break;

        /* Bash a door */
        case 'B':
            do_cmd_bash(); break;

        /* Disarm a trap or chest */
        case 'D':
            do_cmd_disarm(); break;


        /*** Magic and Prayers ***/

        /* Gain new spells/prayers */
        case 'G':
            do_cmd_study(); break;

        /* Browse a book */
        case 'b':
            do_cmd_browse(); break;

        /* Cast a spell */
        case 'm':
            do_cmd_cast(); break;

        /* Pray a prayer */
        case 'p':
            do_cmd_pray(); break;


        /*** Use various objects ***/

        /* Inscribe an object */
        case '{':
            do_cmd_inscribe(); break;

        /* Uninscribe an object */
        case '}':
            do_cmd_uninscribe(); break;

        /* Activate an artifact */
        case 'A':
            do_cmd_activate(); break;

        /* Eat some food */
        case 'E':
            do_cmd_eat_food(); break;

        /* Fuel your lantern/torch */
        case 'F':
            do_cmd_refill(); break;

        /* Fire an item */
        case 'f':
            do_cmd_fire(); break;

        /* Throw an item */
        case 'v':
            do_cmd_throw(); break;

        /* Aim a wand */
        case 'a':
            do_cmd_aim_wand(); break;

        /* Zap a rod */
        case 'z':
            do_cmd_zap_rod(); break;

        /* Quaff a potion */
        case 'q':
            do_cmd_quaff_potion(); break;

        /* Read a scroll */
        case 'r':
            do_cmd_read_scroll(); break;

        /* Use a staff */
        case 'u':
            do_cmd_use_staff(); break;


        /*** Looking at Things (nearby or on map) ***/

        /* Full dungeon map */
        case 'M':
            do_cmd_view_map(); break;

        /* Locate player on map */	
        case 'L':
            do_cmd_locate(); break;

        /* Look around */
        case 'l':
            do_cmd_look(); break;

        /* Target monster or location */
        case '*':
            do_cmd_target(); break;



        /*** Help and Such ***/

        /* Help */
        case '?':
            do_cmd_help("help.hlp"); break;

        /* Identify symbol */
        case '/':
            do_cmd_query_symbol(); break;

        /* Character description */
        case 'C':
            do_cmd_change_name(); break;


        /*** System Commands ***/

        /* Hack -- User interface */
        case '!':
            (void)Term_user(0); break;

        /* Single line from a pref file */
        case '"':
            do_cmd_pref(); break;

        /* Interact with macros */
        case '@':
            do_cmd_macros(); break;

        /* Interact with visuals */
        case '%':
            do_cmd_visuals(); break;

        /* Interact with colors */
        case '&':
            do_cmd_colors(); break;

        /* Interact with options */
        case '=':
            do_cmd_options(); break;


        /*** Misc Commands ***/

        /* Take notes */
        case ':':
            do_cmd_note(); break;	

        /* Version info */
        case 'V':
            do_cmd_version(); break;

        /* Repeat level feeling */
        case KTRL('F'):
            do_cmd_feeling(); break;

        /* Show previous message */
        case KTRL('O'):
            do_cmd_message_one(); break;

        /* Show previous messages */
        case KTRL('P'):
            do_cmd_messages(); break;

        /* Redraw the screen */
        case KTRL('R'):
            do_cmd_redraw(); break;

#ifndef VERIFY_SAVEFILE
        /* Hack -- Save and don't quit */
        case KTRL('S'):
            do_cmd_save_game(); break;
#endif

        /* Save and quit */
        case KTRL('X'):
            alive = FALSE; break;

        /* Quit (commit suicide) */
        case 'Q':
            do_cmd_suicide(); break;

        /* Check artifacts */
        case '~':
            do_cmd_check_artifacts(); break;

        /* Check uniques */
        case '|':
            do_cmd_check_uniques(); break;

#ifndef ANGBAND_LITE

        /* Load "screen dump" */
        case '(':
            do_cmd_load_screen(); break;

        /* Save "screen dump" */
        case ')':
            do_cmd_save_screen(); break;

#endif

        /* Hack -- Unknown command */
        default:
            prt("Type '?' for help.", 0, 0);
            return;
    }


    /* Optional fresh */
    if (fresh_after) {

        /* Handle stuff */
        handle_stuff();

        /* Hack -- Hilite the player */
        move_cursor_relative(py, px);

        /* Refresh */
        Term_fresh();
    }
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
    char cmd;


    /* Hack -- process "repeated" commands */
    if (command_rep) {

        /* Count this execution */
        command_rep--;

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);

        /* Handle stuff */
        handle_stuff();

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

    /* No "direction" yet */
    command_dir = 0;


    /* Hack -- Optional flush */
    if (flush_command) flush();


    /* Hack -- auto-commands */
    if (command_new) {
        msg_print(NULL);
        prt("", 0, 0);
        cmd = command_new;
        command_new = 0;
    }

    /* Get a keypress in "command" mode */
    else {
        msg_flag = FALSE;
        move_cursor_relative(py, px);
        inkey_flag = TRUE;
        cmd = inkey();
        inkey_flag = FALSE;
    }


    /* Command Count */
    if (cmd == '0') {

        /* Begin the input */
        prt("Repeat count:", 0, 0);

        /* Get a command count */
        while (1) {

            /* Get a new keypress */
            cmd = inkey();

            /* Simple editing (delete or backspace) */
            if ((cmd == 0x7F) || (cmd == KTRL('H'))) {

                /* Delete a digit */
                command_arg = command_arg / 10;

                /* Show current count */
                prt(format("Repeat count: %d", command_arg), 0, 0);
            }

            /* Actual numeric data */
            else if (cmd >= '0' && cmd <= '9') {

                /* Stop count at 9999 */
                if (command_arg >= 1000) {

                    /* Warn */
                    bell();

                    /* Limit */
                    command_arg = 9999;
                }

                /* Increase count */
                else {

                    /* Incorporate that digit */
                    command_arg = command_arg * 10 + D2I(cmd);
                }

                /* Show current count */
                prt(format("Repeat count: %d", command_arg), 0, 0);
            }

            /* Exit on "unusable" input */
            else {
                break;
            }
        }

        /* Handle "zero" */
        if (command_arg == 0) {

            /* Default to 99 */
            command_arg = 99;

            /* Show current count */
            prt(format("Repeat count: %d", command_arg), 0, 0);
        }

        /* Hack -- white-space means "enter command now" */
        if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) {
        
            /* Get a real command */
            (void)(get_com("Command: ", &cmd));
        }
    }


    /* Bypass "keymap" */
    if (cmd == '\\') {

        /* Get a char to use without casting */
        (void)(get_com("Command: ", &cmd));

        /* Hack -- allow "control chars" to be entered */
        if (cmd == '^') {

            /* Get a char to "cast" into a control char */
            (void)(get_com("Command: Control-", &cmd));

            /* Hack -- create a control char if legal */
            if (KTRL(cmd)) cmd = KTRL(cmd);
        }

        /* Use the key directly */
        command_cmd = cmd;
    }

    /* Utilize "keymap" */
    else {

        /* Hack -- allow "control chars" to be entered */
        if (cmd == '^') {

            /* Get a char to "cast" into a control char */
            (void)(get_com("Control-", &cmd));

            /* Hack -- create a control char if legal */
            if (KTRL(cmd)) cmd = KTRL(cmd);
        }

        /* Access the array info */
        command_cmd = keymap_cmds[(byte)(cmd)];
        command_dir = keymap_dirs[(byte)(cmd)];

        /* Hack -- notice "undefined" commands */
        if (!command_cmd) command_cmd = ESCAPE;
    }


    /* Hack -- Auto-repeat certain commands */
    if (always_repeat && (command_arg <= 0)) {

        /* Bash, Disarm, Open, Tunnel get 99 attempts */
        if (strchr("BDo+", command_cmd)) command_arg = 99;
    }


    /* Hack -- erase the message line. */
    prt("", 0, 0);

    /* Hack -- Re-Illuminate the player */
    move_cursor_relative(py, px);
}



