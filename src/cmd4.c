/* File: cmd4.c */

/* Purpose: Interface commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



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
    if (term_recall)
    {
        Term_activate(term_recall);
        Term_clear();
        Term_fresh();
    }

#endif

#ifdef GRAPHIC_CHOICE

    /* Hack -- erase the "choice" window */
    if (term_choice)
    {
        Term_activate(term_choice);
        Term_clear();
        Term_fresh();
    }

#endif

#ifdef GRAPHIC_MIRROR

    /* Hack -- erase the "mirror" window */
    if (term_mirror)
    {
        Term_activate(term_mirror);
        Term_clear();
        Term_fresh();
    }

#endif

    /* Activate the main screen */
    Term_activate(term_screen);

    /* Actual redraw */
    Term_redraw();


    /* Combine and Reorder the pack (later) */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);


    /* Update torch */
    p_ptr->update |= (PU_TORCH);

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Forget lite/view */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update lite/view */
    p_ptr->update |= (PU_VIEW | PU_LITE);

    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw everything */
    p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);

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
    for (flag = FALSE; !flag; )
    {
        /* Display the player */
        display_player(history);

        /* Prompt */
        prt("['C' to change name, 'F' to dump file, 'H' for history, or ESCAPE]", 21, 2);

        /* Query */
        c = inkey();

        /* Handle */
        switch (c)
        {
            case 'C':
            case 'c':
                get_name();
                flag = TRUE;
                break;

            case 'F':
            case 'f':
                sprintf(tmp, "%s.txt", player_base);
                if (get_string("File name: ", tmp, 80))
                {
                    if (tmp[0] && (tmp[0] != ' '))
                    {
                        file_character(tmp, FALSE);
                    }
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

        /* Flush messages */
        msg_print(NULL);
    }

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
    /* Recall one message XXX XXX XXX */
    prt(format("> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user	-BEN-
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 */
void do_cmd_messages(void)
{
    int i, j, k, n;

    char finder[80] = "";


    /* Total messages */
    n = message_num();


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Start on first message */
    i = 0;

    /* Process requests until done */
    while (1)
    {
        /* Clear the screen */
        clear_screen();

        /* Dump up to 20 lines of messages */
        for (j = 0; (j < 20) && (i + j < n); j++)
        {
            byte a = TERM_WHITE;

            cptr str = message_str(i+j);
            
            /* Handle searches XXX XXX XXX */
            if (finder[0] && strstr(str, finder)) a = TERM_YELLOW;

            /* Dump the messages, bottom to top */
            Term_putstr(0, 21-j, -1, a, str);
        }

        /* Display header XXX XXX XXX */
        prt(format("Message Recall (%d-%d of %d)", i, i+j-1, n), 0, 0);

        /* Display prompt */
        prt("[Press 'p' for older, 'n' for newer, '/' for search, or ESCAPE]", 23, 0);

        /* Get a command */
        k = inkey();

        /* Exit on Escape */
        if (k == ESCAPE) break;

        /* Hack -- Save the old index */
        j = i;

        /* Hack -- handle search */
        if (k == '/')
        {
            /* Prompt */
            prt("Find: ", 23, 0);

            /* Get a search string, or cancel search */
            if (!askfor_aux(finder, 80)) finder[0] = '\0';

            /* Okay */
            continue;
        }

        /* Recall more older messages */
        if ((k == 'p') || (k == KTRL('P')))
        {
            /* Go older if legal */
            if (i + 20 < n) i += 20;
        }

        /* Recall more newer messages */
        if ((k == 'n') || (k == KTRL('N')))
        {
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
    for (i = 0; options[i].o_desc; i++)
    {
        /* Notice options on this "page" */
        if (options[i].o_page == page) opt[n++] = i;
    }


    /* Clear the screen */
    clear_screen();

    /* Prompt XXX XXX XXX */
    sprintf(buf, "%s (RET to advance, y/n to set, ESC to accept) ", info);
    prt(buf, 0, 0);

    /* Display the options */
    for (i = 0; i < n; i++)
    {
        sprintf(buf, "%-48s: %s ", options[opt[i]].o_desc,
                (*options[opt[i]].o_var ? "yes" : "no "));
        prt(buf, i + 2, 0);
    }


    /* Start at the first option */
    i = 0;

    /* Interact with the player */
    while (TRUE)
    {
        move_cursor(i + 2, 50);

        ch = inkey();

        switch (ch)
        {
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
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 */
void do_cmd_options(void)
{
    int k;


    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();


    /* Interact */
    while (1)
    {
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
        if (k == '1')
        {
            /* Process the general options */
            do_cmd_options_aux(1, "User Interface Options");
        }

        /* Disturbance Options */
        else if (k == '2')
        {
            /* Process the running options */
            do_cmd_options_aux(2, "Disturbance Options");
        }

        /* Inventory Options */
        else if (k == '3')
        {
            /* Process the running options */
            do_cmd_options_aux(3, "Inventory Options");
        }

        /* Gameplay Options */
        else if (k == '4')
        {
            /* Process the game-play options */
            do_cmd_options_aux(4, "Game-Play Options");
        }

        /* Efficiency Options */
        else if (k == '5')
        {
            /* Process the efficiency options */
            do_cmd_options_aux(5, "Efficiency Options");
        }

        /* Efficiency Options */
        else if (k == '6')
        {
            /* Process the efficiency options */
            do_cmd_options_aux(6, "Special Options");
        }

        /* Cheating Options XXX XXX XXX */
        else if ((k == 'C') && can_be_wizard)
        {
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
        else if (k == 'D')
        {
            /* Prompt */
            prt("Command: Base Delay Speed", 14, 0);

            /* Get a new value */
            while (1)
            {
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
        else if (k == 'H')
        {
            /* Prompt */
            prt("Command: Hitpoint Warning", 14, 0);

            /* Get a new value */
            while (1)
            {
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
        else
        {
            /* Oops */
            bell();
        }

        /* Flush messages */
        msg_print(NULL);
    }


    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;


    /* Verify the keymap */
    keymap_init();
}



/*
 * Ask for a "user pref line" and process it
 *
 * XXX XXX XXX Allow absolute file names?
 */
void do_cmd_pref(void)
{
    char buf[80];

    /* Default */
    strcpy(buf, "");

    /* Ask for a "user pref command" */
    if (!get_string("Pref: ", buf, 80)) return;

    /* Process that pref command */
    (void)process_pref_file_aux(buf);
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
    for (i = 0; i < macro__num; i++)
    {
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
    while (TRUE)
    {
        /* Do not process macros */
        inkey_base = TRUE;

        /* Do not wait for keys */
        inkey_scan = TRUE;

        /* Attempt to read a key */
        i = inkey();

        /* If a key is ready, acquire it */
        if (i)
        {
            /* Reset delay */
            w = 0;

            /* Hack -- process "ascii 28" */
            if (i == 28)
            {
                /* Toggle the "skipping" flag */
                skipping = !skipping;
            }

            /* Save usable keys */
            else if (!skipping)
            {
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
    while (1)
    {
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
        else if (i == '1')
        {
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
        else if (i == '2')
        {
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
        else if (i == '3')
        {
            /* Prompt */
            Term_putstr(0, 14, -1, TERM_WHITE, "Command: Enter a new action");

            /* Go to the correct location */
            Term_gotoxy(0, 20);

            /* Hack -- limit the value */
            tmp[80] = '\0';

            /* Get an encoded action */
            if (!askfor_aux(buf, 80)) continue;

            /* Extract an action */
            text_to_ascii(macro__buf, buf);
        }

        /* Query a macro action */
        else if (i == '4')
        {

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
        else if (i == '5')
        {
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
        else if (i == '6')
        {
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
        else if (i == '7')
        {
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
        else if (i == '8')
        {
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
        else if (i == '9')
        {
            char i1, i2, i3;

            /* Flush the input */
            flush();

            /* Get the trigger */
            if (get_com("Type the trigger keypress: ", &i1) &&
                get_com("Type the resulting keypress: ", &i2) &&
                get_com("Type a direction (or space): ", &i3))
            {
                /* Acquire the array index */
                int k = (byte)(i1) % 128;
                int r = (byte)(i2) % 128;
                int d = (byte)(i3) % 128;

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
        else
        {
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
    while (1)
    {
        /* Clear the screen */
        clear_screen();

        /* Ask for a choice */
        prt("Interact with Visuals", 2, 0);

        /* Give some choices */
        prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_VISUALS
        prt("(2) Dump monster attr/chars", 5, 5);
        prt("(3) Dump object attr/chars", 6, 5);
        prt("(4) Dump feature attr/chars", 7, 5);
        prt("(5) (unused)", 8, 5);
        prt("(6) Change monster attr/chars", 9, 5);
        prt("(7) Change object attr/chars", 10, 5);
        prt("(8) Change feature attr/chars", 11, 5);
        prt("(9) (unused)", 12, 5);
#endif
        prt("(0) Reset visuals", 13, 5);

        /* Prompt */
        prt("Command: ", 15, 0);

        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;

        /* Load a 'pref' file */
        else if (i == '1')
        {
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

#ifdef ALLOW_VISUALS

        /* Dump monster attr/chars */
        else if (i == '2')
        {
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
            for (i = 0; i < MAX_R_IDX; i++)
            {
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
        else if (i == '3')
        {
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
            for (i = 0; i < MAX_K_IDX; i++)
            {
                object_kind *k_ptr = &k_info[i];

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
        else if (i == '4')
        {
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
            for (i = 0; i < MAX_F_IDX; i++)
            {
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
        else if (i == '6')
        {
            static int r = 0;

            /* Prompt */
            prt("Command: Change monster attr/chars", 15, 0);

            /* Hack -- query until done */
            while (1)
            {
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
        else if (i == '7')
        {
            static int k = 0;

            /* Prompt */
            prt("Command: Change object attr/chars", 15, 0);

            /* Hack -- query until done */
            while (1)
            {
                object_kind *k_ptr = &k_info[k];

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
        else if (i == '8')
        {
            static int f = 0;

            /* Prompt */
            prt("Command: Change feature attr/chars", 15, 0);

            /* Hack -- query until done */
            while (1)
            {
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

#endif

        /* Reset visuals */
        else if (i == '0')
        {
            /* Reset */
            reset_visuals();

            /* Message */
            msg_print("Visual attr/char tables reset.");
        }

        /* Unknown option */
        else
        {
           bell();
        }

        /* Flush messages */
        msg_print(NULL);
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
    while (1)
    {
        /* Clear the screen */
        clear_screen();

        /* Ask for a choice */
        prt("Interact with Colors", 2, 0);

        /* Give some choices */
        prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_COLORS
        prt("(2) Dump colors", 5, 5);
        prt("(3) Modify colors", 6, 5);
#endif

        /* Prompt */
        prt("Command: ", 8, 0);

        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;

        /* Load a 'pref' file */
        if (i == '1')
        {
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

#ifdef ALLOW_COLORS

        /* Dump colors */
        else if (i == '2')
        {
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
            for (i = 0; i < 256; i++)
            {
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
        else if (i == '3')
        {
            static int a = 0;

            /* Prompt */
            prt("Command: Modify colors", 8, 0);

            /* Hack -- query until done */
            while (1)
            {
                cptr name;

                /* Clear */
                clear_from(10);

                /* Exhibit the normal colors */
                for (i = 0; i < 16; i++)
                {
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

#endif

        /* Unknown option */
        else
        {
            bell();
        }

        /* Flush messages */
        msg_print(NULL);
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

    /* Default */
    strcpy(buf, "");

    /* Input */
    if (!get_string("Note: ", buf, 60)) return;

    /* Ignore empty notes */
    if (!buf[0] || (buf[0] == ' ')) return;

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
 * Array of feeling strings
 */
static cptr do_cmd_feeling_text[11] =
{
    "Looks like any other level.",
    "You feel there is something special about this level.",
    "You have a superb feeling about this level.",
    "You have an excellent feeling...",
    "You have a very good feeling...",
    "You have a good feeling...",
    "You feel strangely lucky...",
    "You feel your luck is turning...",
    "You like the look of this place...",
    "This level can't be all bad...",
    "What a boring place..."
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling()
{
    /* Verify the feeling */
    if (feeling < 0) feeling = 0;
    if (feeling > 10) feeling = 10;

    /* No useful feeling in town */
    if (!dun_level)
    {
        msg_print("Looks like a typical town.");
        return;
    }

    /* Display the feeling */
    msg_print(do_cmd_feeling_text[feeling]);
}





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
    for (y = 0; okay && (y < 24); y++)
    {
        /* Get a line of data */
        if (my_fgets(fff, buf, 1024)) okay = FALSE;

        /* Show each row */
        for (x = 0; x < 79; x++)
        {
            /* Put the attr/char */
            Term_draw(x, y, TERM_WHITE, buf[x]);
        }
    }

    /* Get the blank line */
    if (my_fgets(fff, buf, 1024)) okay = FALSE;


    /* Dump the screen */
    for (y = 0; okay && (y < 24); y++)
    {
        /* Get a line of data */
        if (my_fgets(fff, buf, 1024)) okay = FALSE;

        /* Dump each row */
        for (x = 0; x < 79; x++)
        {
            /* Get the attr/char */
            (void)(Term_what(x, y, &a, &c));

            /* Look up the attr */
            for (i = 0; i < 16; i++)
            {
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
    for (y = 0; y < 24; y++)
    {
        /* Dump each row */
        for (x = 0; x < 79; x++)
        {
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
    for (y = 0; y < 24; y++)
    {
        /* Dump each row */
        for (x = 0; x < 79; x++)
        {
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




/*
 * Check the status of "artifacts"
 *
 * XXX Consider use of "term_mirror"
 */
void do_cmd_check_artifacts(void)
{
    int i, k, z, x, y;

    FILE *fff;

    char file_name[1024];

    char base_name[80];

    bool okay[MAX_A_IDX];


    /* Temporary file */
    if (path_temp(file_name, 1024)) return;

    /* Open a new file */
    fff = my_fopen(file_name, "w");

    /* Scan the artifacts */
    for (k = 0; k < MAX_A_IDX; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* Default */
        okay[k] = FALSE;

        /* Skip "empty" artifacts */
        if (!a_ptr->name) continue;

        /* Skip "uncreated" artifacts */
        if (!a_ptr->cur_num) continue;

        /* Assume okay */
        okay[k] = TRUE;
    }

    /* Check the dungeon */
    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
            cave_type *c_ptr = &cave[y][x];

            /* Process objects */
            if (c_ptr->i_idx)
            {
                object_type *i_ptr = &i_list[c_ptr->i_idx];

                /* Ignore non-artifacts */
                if (!artifact_p(i_ptr)) continue;

                /* Ignore known items */
                if (object_known_p(i_ptr)) continue;

                /* Note the artifact */
                okay[i_ptr->name1] = FALSE;
            }
        }
    }

    /* Check the inventory */
    for (i = 0; i < INVEN_PACK; i++)
    {
        object_type *i_ptr = &inventory[i];

        /* Ignore non-objects */
        if (!i_ptr->k_idx) continue;

        /* Ignore non-artifacts */
        if (!artifact_p(i_ptr)) continue;

        /* Ignore known items */
        if (object_known_p(i_ptr)) continue;

        /* Note the artifact */
        okay[i_ptr->name1] = FALSE;
    }

    /* Scan the artifacts */
    for (k = 0; k < MAX_A_IDX; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* List "dead" ones */
        if (!okay[k]) continue;

        /* Paranoia */
        strcpy(base_name, "Unknown Artifact");

        /* Obtain the base object type */
        z = lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Real object */
        if (z)
        {
            object_type forge;

            /* Create the object */
            invcopy(&forge, z);

            /* Create the artifact */
            forge.name1 = k;

            /* Describe the artifact */
            object_desc_store(base_name, &forge, FALSE, 0);
        }

        /* Hack -- Build the artifact name */
        fprintf(fff, "     The %s\n", base_name);
    }

    /* Close the file */
    my_fclose(fff);

    /* Display the file contents */
    show_file(file_name, "Artifacts Seen");

    /* Remove the file */
    fd_kill(file_name);
}


/*
 * Check the status of "uniques"
 *
 * XXX Consider use of "term_mirror"
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 */
void do_cmd_check_uniques()
{
    int k;

    FILE *fff;

    char file_name[1024];


    /* Temporary file */
    if (path_temp(file_name, 1024)) return;

    /* Open a new file */
    fff = my_fopen(file_name, "w");

    /* Scan the monster races */
    for (k = 1; k < MAX_R_IDX-1; k++)
    {
        monster_race *r_ptr = &r_info[k];

        /* Only print Uniques */
        if (r_ptr->flags1 & RF1_UNIQUE)
        {
            bool dead = (r_ptr->max_num == 0);

            /* Only display "known" uniques */
            if (dead || cheat_know || r_ptr->r_sights)
            {
                /* Print a message */
                fprintf(fff, "     %s is %s\n",
                        (r_name + r_ptr->name),
                        (dead ? "dead" : "alive"));
            }
        }
    }

    /* Close the file */
    my_fclose(fff);

    /* Display the file contents */
    show_file(file_name, "Known Uniques");

    /* Remove the file */
    fd_kill(file_name);
}



