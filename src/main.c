/* File: main.c */

/* Purpose: initialization, main() function and main loop */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Hack -- Local "game mode" vars
 */
static int new_game = FALSE;
static int fiddle = FALSE;
static int force_keyset = FALSE;
static int force_keyset_arg = FALSE;


/*
 * General use filename buffer
 */
static char buf[1024];


#if !defined(MACINTOSH) && !defined(WINDOWS)

/*
 * Unix machines need to "check wizard permissions"
 */
static bool is_wizard(int uid)
{
    int		test;
    FILE	*fp;

    bool allow = FALSE;


    /* Access the "r_list.txt" file */
    sprintf(buf, "%s%s%s", ANGBAND_DIR_FILE, PATH_SEP, "wizards.txt");

    /* Open the wizard file */
    fp = my_tfopen(buf, "r");

    /* No wizard file, so no wizards */
    if (!fp) return (FALSE);

    /* Scan the wizard file */
    while (!allow && fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#') continue;
        if (buf[0] == '*') allow = TRUE;
        if (sscanf(buf, "%d", &test) != 1) continue;
        if (test == uid) allow = TRUE;
    }

    /* Close the file */
    fclose(fp);

    /* Result */
    return (allow);
}



/*
 * A hook for "quit()".
 * Close down, then fall back into "quit()".
 */
static void quit_hook(cptr s)
{
    /* Close the high score file */
    nuke_scorefile();

    /* Shut down the term windows */
    if (term_choice) term_nuke(term_choice);
    if (term_recall) term_nuke(term_recall);
    if (term_screen) term_nuke(term_screen);
}



/*
 * Set the stack size for Amiga's
 */
#ifdef AMIGA
# include <dos.h>
__near long __stack = 100000;
#endif


/*
 * Studly machines can actually parse command line args
 */
int main(int argc, char *argv[])
{
    bool done = FALSE;

    /* Dump score list (num lines)? */
    int show_score = 0;


    /* Save the "program name" */
    argv0 = argv[0];


#ifdef SET_UID
    /* Default permissions on files */
    (void)umask(022);
#endif


#if defined(SET_UID) && defined(SECURE)
    /* Authenticate */
    Authenticate();
#endif


    /* Get the file paths */
    get_file_paths();


    /* Prepare the "high scores" file */
    init_scorefile();


    /* Get the user id (?) */
    player_uid = getuid();

#if defined(SET_UID) && defined(SAFE_SETUID)

# ifdef _POSIX_SAVED_IDS

    /* Save some info for later */
    player_euid = geteuid();
    player_egid = getegid();

# endif

#if 0	/* XXX XXX XXX */
    /* Redundant setting necessary in case root is running the game */
    /* If not root or game not setuid the following two calls do nothing */

    if (setgid(getegid()) != 0) {
      quit("setgid(): cannot set permissions correctly!");
    }

    if (setuid(geteuid()) != 0) {
      quit("setuid(): cannot set permissions correctly!");
    }
#endif

#endif


    /* Check for "Wizard" permission */
    can_be_wizard = is_wizard(player_uid);

    /* Acquire the "user name" as a default player name */
    user_name(player_name, player_uid);


    /* check for user interface option */
    for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
        switch (argv[0][1]) {

          case 'c':
          case 'C':
            ANGBAND_DIR_PREF = &argv[0][2];
            break;

#ifndef VERIFY_SAVEFILE
          case 'd':
          case 'D':
            ANGBAND_DIR_SAVE = &argv[0][2];
            break;
#endif

          case 'i':
          case 'I':
            ANGBAND_DIR_INFO = &argv[0][2];
            break;

          case 'N':
          case 'n':
            new_game = TRUE;
            break;

          case 'R':
          case 'r':
            force_keyset = TRUE;
            force_keyset_arg = TRUE;
            break;

          case 'O':
          case 'o':
            force_keyset = TRUE;
            force_keyset_arg = FALSE;
            break;

          case 'S':
          case 's':
            show_score = atoi(&argv[0][2]);
            if (show_score <= 0) show_score = 10;
            break;

          case 'F':
          case 'f':
            if (!can_be_wizard) goto usage;
            fiddle = to_be_wizard = TRUE;
            break;

#ifdef SAVEFILE_USE_UID
          case 'P':
          case 'p':
            if (!can_be_wizard) goto usage;
            if (isdigit((int)argv[0][2])) {
                player_uid = atoi(&argv[0][2]);
                user_name(player_name, player_uid);
            }
            break;
#endif

          case 'W':
          case 'w':
            if (!can_be_wizard) goto usage;
            to_be_wizard = TRUE;
            break;
            
          case 'u':
          case 'U':
            if (!argv[0][2]) goto usage;
            strcpy(player_name, &argv[0][2]);
            break;

          default:
          usage:

            /* Note -- the Term is NOT initialized */
            puts("Usage: angband [-n] [-o] [-r] [-u<name>] [-s<num>]");
            puts("  n       Start a new character");
            puts("  o       Use the original command set");
            puts("  r       Use the rogue-like command set");
            puts("  u<name> Play with your <name> savefile");
            puts("  s<num>  Show high scores.  Show <num> scores, or first 10");
            puts("");

            /* Less common options */
            puts("Extra options: [-c<path>] [-d<path>] [-i<path>]");
            puts("  c<path> Look for pref files in the directory <path>");
            puts("  d<path> Look for save files in the directory <path>");
            puts("  i<path> Look for info files in the directory <path>");
            puts("");

            /* Extra wizard options */
            if (can_be_wizard) {
                puts("Extra wizard options: [-f] [-w] [-p<uid>]");
                puts("  f       Activate 'fiddle' mode");
                puts("  w       Activate 'wizard' mode");
                puts("  p<uid>  Pretend to have the player uid number <uid>");
                puts("");
            }

            /* Actually abort the process */
            quit(NULL);
        }
    }


    /* Process the player name */
    process_player_name(TRUE);



    /* Drop privs (so X11 will work correctly) */
    safe_setuid_drop();


#ifdef USE_IBM
    /* Attempt to use the "main-ibm.c" support */
    if (!done) {
        extern errr init_ibm(void);
        if (0 == init_ibm()) done = TRUE;
        if (done) ANGBAND_SYS = "ibm";
    }
#endif

#ifdef __EMX__
    /* Attempt to use the "main-emx.c" support */
    if (!done) {
        extern errr init_emx(void);
        if (0 == init_emx()) done = TRUE;
        if (done) ANGBAND_SYS = "emx";
    }
#endif

#ifdef USE_WAT
    /* Attempt to use the "main-wat.c" support */
    if (!done) {
        extern errr init_wat(void);
        if (0 == init_wat()) done = TRUE;
        if (done) ANGBAND_SYS = "wat";
    }
#endif

    /* XXX Default to "X11" if available XXX */

#ifdef USE_XAW
    /* Attempt to use the "main-xaw.c" support */
    if (!done) {
        extern errr init_xaw(void);
        if (0 == init_xaw()) done = TRUE;
        if (done) ANGBAND_SYS = "xaw";
    }
#endif

#ifdef USE_X11
    /* Attempt to use the "main-x11.c" support */
    if (!done) {
        extern errr init_x11(void);
        if (0 == init_x11()) done = TRUE;
        if (done) ANGBAND_SYS = "x11";
    }
#endif

#ifdef USE_GCU
    /* Attempt to use the "main-gcu.c" support */
    if (!done) {
        extern errr init_gcu(void);
        if (0 == init_gcu()) done = TRUE;
        if (done) ANGBAND_SYS = "gcu";
    }
#endif

#ifdef USE_NCU
    /* Attempt to use the "main-ncu.c" support */
    if (!done) {
        extern errr init_ncu(void);
        /* When "init_ncu()" fails, it quits (?) */
        if (0 == init_ncu()) done = TRUE;
        if (done) ANGBAND_SYS = "ncu";
    }
#endif

#ifdef USE_CUR
    /* Attempt to use the "main-cur.c" support */
    if (!done) {
        extern errr init_cur(void);
        /* When "init_cur()" fails, it quits (?) */
        if (0 == init_cur()) done = TRUE;
        if (done) ANGBAND_SYS = "cur";
    }
#endif


    /* Grab privs (dropped above for X11) */
    safe_setuid_grab();


    /* Make sure we have a display! */
    if (!done) quit("Unable to prepare any 'display module'!");


    /* Tell "quit()" to call "Term_nuke()" */
    quit_aux = quit_hook;


    /* Handle "score list" requests */
    if (show_score > 0) {
        display_scores(0, show_score);
        quit(NULL);
    }


    /* catch those nasty signals (assumes "Term_init()") */
    signals_init();

    /* Initialize the checkers */
    check_time_init();
    check_load_init();

    /* Show news file */
    show_news();

    /* Initialize the arrays */
    init_some_arrays();

    /* Wait for response */
    pause_line(23);

    /* Call the main function */
    play_game();

    /* Exit (never gets here) */
    return (0);
}

#endif





/*
 * Actually play a game
 *
 * Note the global "character_dungeon" which is TRUE when a dungeon has
 * been built for the player.  If FALSE, then a new level must be generated.
 */
void play_game()
{
    int result = FALSE;


    /* Character is "icky" */
    character_icky = TRUE;


    /* Hack -- turn off the cursor */
    Term_hide_cursor();

    /* Grab a random seed from the clock */
    init_seeds();


    /* Hack -- restore dead players */
    if (fiddle) {
        result = load_player();
        if (result) save_player();
        quit(NULL);
    }

    /* If "restore game" requested, attempt to do so */
    if (!new_game) {

        /* Load the player */
        result = load_player();

        /* Start new file on failure */
        if (!result) new_game = TRUE;
        
        /* Process the player name */
        process_player_name(FALSE);
    }

    /* Enter wizard mode AFTER "resurrection" (if any) is complete */
    if (to_be_wizard && enter_wiz_mode()) wizard = TRUE;

    
    /* Pick new "seeds" if needed */
    if (new_game) {

        /* Hack -- reset seeds */
        town_seed = random();
        randes_seed = random();
    }

    /* Flavor the objects */
    flavor_init();

    /* Reset the visual mappings */
    reset_visuals();

    /* Roll up a new character if needed */
    if (new_game) {
    
        /* Roll up a new character */
        player_birth();
    }
    
    
    /* Recalculate some stuff */
    p_ptr->update |= (PU_BONUS);

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);

    /* Handle stuff */
    handle_stuff();

    /* Display character (briefly) */
    display_player();

    /* Flash a message */
    prt("Please wait...", 0, 0);

    /* Flush the message */
    Term_fresh();


    /* Reset "rogue_like_commands" if requested */
    if (force_keyset) rogue_like_commands = force_keyset_arg;

    /* Verify the keymap (before loading preferences!) */
    keymap_init();


    /* Access the system "pref" file */
    sprintf(buf, "%s.prf", "pref");

    /* Process the default pref file */
    process_pref_file(buf);
    
    /* Access the system "pref" file */
    sprintf(buf, "pref-%s.prf", ANGBAND_SYS);

    /* Attempt to process that file */
    process_pref_file(buf);

    /* Access the class's "pref" file */
    sprintf(buf, "%s.prf", cp_ptr->title);

    /* Attempt to process that file */
    process_pref_file(buf);

    /* Access the character's "pref" file */
    sprintf(buf, "%s.prf", player_base);

    /* Attempt to process that file */
    process_pref_file(buf);


    /* Make the first level (the town) */
    if (!character_dungeon) {

        /* Make a new level */
        generate_cave();

        /* The dungeon is ready */
        character_dungeon = TRUE;
    }

    /* Character is now "complete" */
    character_generated = TRUE;


    /* Character is no longer "icky" */
    character_icky = FALSE;
    
    
    /* Hack -- enforce "delayed death" */
    if (p_ptr->chp < 0) death = TRUE;

    /* Loop till dead */
    while (!death) {

        /* Process the level */
        dungeon();

        /* Make the New level */
        if (!death) generate_cave();
    }

    /* Display death, Save the game, and exit */
    exit_game();
}


/*
 * This is a "hook" into "play_game()" for "menu based" systems.
 *
 * See "main-mac.c" and other files for actual "main()" functions
 */
void play_game_mac(int ng)
{
    /* No name (yet) */
    strcpy(player_name, "");

    /* Hack -- assume wizard permissions */
    can_be_wizard = TRUE;

    /* Hack -- extract a flag */
    new_game = ng;

#ifdef MACINTOSH
    /* Use the "pref-mac.prf" file */
    ANGBAND_SYS = "mac";
#endif

#ifdef WINDOWS
    /* Choose a "pref-xxx.prf" file */
    ANGBAND_SYS = (use_graphics ? "gfw" : "txw");
#endif

    /* Play a game */
    play_game();
}


