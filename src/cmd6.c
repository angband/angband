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




#ifdef ALLOW_BORG

/*
 * Hack -- Declare the Ben Borg
 */
extern void borg_ben(void);

#endif



/*
 * Verify desire to be a wizard, and do so if verified
 * This routine should only be called if "can_be_wizard"
 */
int enter_wiz_mode(void)
{
    int answer = FALSE;

    /* Already been asked */
    if (noscore & 0x0002) return (TRUE);

    /* Verify request */
    msg_print("Wizard mode is for debugging and experimenting.");
    msg_print("The game will not be scored if you enter wizard mode.");
    answer = get_check("Are you sure you want to enter wizard mode?");

    /* Never Mind */
    if (!answer) return (FALSE);

    /* Remember old setting */
    noscore |= 0x0002;

    /* Make me a wizard */
    return (TRUE);
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

            /* Redraw "title" */
            p_ptr->redraw |= (PR_TITLE);

            break;


#ifdef ALLOW_WIZARD

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

#endif


#ifdef ALLOW_BORG

        /* Interact with the Borg */
        case CTRL('Z'):

            /* Interact with the borg */
            borg_ben();

            /* Free command */
            energy_use = 0;

            break;

#endif



        /*** Inventory Commands ***/

        /* Wear or wield something */
        case '[':
            do_cmd_wield(); break;

        /* Take something off */
        case ']':
            do_cmd_takeoff(); break;

        /* Drop something */
        case 'd':
            do_cmd_drop(); break;

        /* Equipment list */
        case 'e':
            do_cmd_equip(); break;

        /* Inventory */
        case 'i':
            do_cmd_inven(); break;


        /*** Some temporary commands ***/
        
        /* Destory an inventory slot */
        case 'k':
            do_cmd_destroy(); break;
            
        /* Identify an object */
        case 'I':
            do_cmd_observe(); break;
            

        /*** Handle "choice window" ***/

        /* Hack -- toggle choice window */
        case CTRL('E'):

            /* Free command */
            energy_use = 0;

            /* Hack -- flip the current status */
            choice_default = !choice_default;

            /* Redraw choice window */
            p_ptr->redraw |= (PR_CHOICE);

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

        /* Begin Resting -- Arg is time */
        case 'R':
            do_cmd_rest(); break;

        /* Search the adjoining grids */
        case 's':
            do_cmd_search(); break;

        /* Toggle search status */
        case 'S':
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
        case 'j':
            do_cmd_spike(); break;

        /* Force a door or Bash a monster. */
        case 'B':
            do_cmd_bash(); break;

        /* Disarm a trap */
        case 'D':
            do_cmd_disarm(); break;


        /*** Magic and Prayers ***/

        /* Gain some spells */
        case 'G':
            do_cmd_study(); break;

        /* Peruse a Book */
        case 'b':
            do_cmd_browse(); break;

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
        case 'L':
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

        case '@':
            do_cmd_macro(FALSE); break;

        case '!':
            do_cmd_macro(TRUE); break;

        case '&':
            do_cmd_keymap(); break;

        /* Set options */
        case '=':
            do_cmd_options(); break;

        /* Interact with preference files */
        case '%':
            do_cmd_prefs(); break;


        /*** Misc Commands ***/
        
        case ':':
            do_cmd_note(); break;	

        /* Hack -- Game Version */
        case 'V':
            do_cmd_version(); break;

        /* Repeat Feeling */
        case CTRL('F'):
            do_cmd_feeling(); break;

        /* Previous message(s). */
        case CTRL('P'):
            do_cmd_messages(); break;

        /* Commit Suicide and Quit */
        case CTRL('K'):
            do_cmd_suicide(); break;

        /* Save and Quit */
        case CTRL('X'):
            exit_game(); break;

#ifndef VERIFY_SAVEFILE
        /* Hack -- Save (no quit) */
        case CTRL('S'):
            do_cmd_save_game(); break;
#endif

        /* Redraw the screen */
        case CTRL('R'):
            do_cmd_redraw(); break;

        /* Check artifacts */
        case '~':
            do_cmd_check_artifacts(); break;

        /* Check uniques */
        case '|':
            do_cmd_check_uniques(); break;

        /* Dump the screen (monochrome) */
        case '(':
            do_cmd_dump(FALSE); break;
            
        /* Dump the screen (with colors) */
        case ')':
            do_cmd_dump(TRUE); break;


        /* Hack -- Unknown command */
        default:
            energy_use = 0;
            prt("Type '?' for help.", 0, 0);
            return;
    }


    /* Save the command */
    command_old = command_cmd;


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


    /* Hack -- Assume no abortion yet */
    command_esc = 0;


    /* Hack -- process "repeated" commands */
    if (command_rep) {

        /* Count this execution */
        command_rep--;

        /* Hack -- Dump repeat count */
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

    /* Hack -- no direction yet */
    command_dir = -1;


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
        move_cursor_relative(py, px);
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


    /* Bypass "keymap" */
    if (cmd == '\\') {

        /* Get a char to use without casting */
        if (!get_com("Command: ", &cmd)) cmd = ESCAPE;

        /* Hack -- allow "control chars" to be entered */
        if (cmd == '^') {

            /* Get a char to "cast" into a control char */
            if (!get_com("Command: Control-", &cmd)) cmd = ESCAPE;

            /* Hack -- create a control char if legal */
            else if (CTRL(cmd)) cmd = CTRL(cmd);
        }

        /* Use the key directly */
        command_cmd = cmd;
    }

    /* Utilize "keymap" */
    else {

        /* Hack -- allow "control chars" to be entered */
        if (cmd == '^') {

            /* Get a char to "cast" into a control char */
            if (!get_com("Control-", &cmd)) cmd = ESCAPE;

            /* Hack -- create a control char if legal */
            else if (CTRL(cmd)) cmd = CTRL(cmd);
        }

        /* Access the array info */
        command_cmd = keymap_cmds[(byte)(cmd)];
        command_dir = keymap_dirs[(byte)(cmd)];

        /* Hack -- notice "undefined" commands */
        if (!command_cmd) command_cmd = ESCAPE;

        /* Hack -- extract "non-directions" if needed */
        if ((command_dir < 1) || (command_dir > 9)) command_dir = -1;
    }


    /* Some commands can be "auto-repeated" by default */
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
            p_ptr->redraw |= (PR_STATE);

            /* Handle stuff */
            handle_stuff();
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



