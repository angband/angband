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


#if !defined(MACINTOSH) && !defined(_Windows)

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
 * Verify the name
 */
static bool name_okay(cptr s)
{
    cptr a;

    /* Cannot be too long */
    if (strlen(s) > 15) {
        plog_fmt("The name '%s' is too long for Angband", s);
        return (FALSE);
    }

    /* Cannot contain "icky" characters */
    for (a = s; *a; a++) {

        /* No control characters */
        if (iscntrl(*a)) {
            plog_fmt("The name '%s' contains control characters", s);
            return (FALSE);
        }
    }

    /* Acceptable */
    return (TRUE);
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
            
          case 'd':
          case 'D':
            ANGBAND_DIR_SAVE = &argv[0][2];
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
            puts("Extra options: [-c<path>] [-d<path>]");
            puts("  c<path> Look for pref files in the directory <path>");
            puts("  d<path> Look for save files in the directory <path>");
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

    /* Verify the "player name" */
    if (!name_okay(player_name)) quit("bad player name");


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

    /* XXX XXX Verify the "player name" */
    if (streq(player_name, "")) strcpy(player_name, "PLAYER");

#ifdef SAVEFILE_USE_UID
    /* Load the savefile name */
    (void)sprintf(savefile, "%s%s%d%s",
                  ANGBAND_DIR_SAVE, PATH_SEP, player_uid, player_name);
#else
    /* Load the savefile name */
    (void)sprintf(savefile, "%s%s%s",
                  ANGBAND_DIR_SAVE, PATH_SEP, player_name);
#endif

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
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit()
{
    int		i, j;
    inven_type	inven_init;
    inven_type	*i_ptr = &inven_init;


    /* Give the player some food */
    invcopy(i_ptr, OBJ_FOOD_RATION);
    i_ptr->number = rand_range(3,7);
    inven_aware(i_ptr);
    inven_known(i_ptr);
    (void)inven_carry(i_ptr);

    /* Give the player some torches */
    invcopy(i_ptr, OBJ_TORCH);
    i_ptr->number = rand_range(3,7);
    i_ptr->pval = rand_range(3,7) * 500;
    inven_known(i_ptr);
    (void)inven_carry(i_ptr);

    /* Give the player three useful objects */
    for (i = 0; i < 3; i++) {
        j = player_init[p_ptr->pclass][i];
        invcopy(i_ptr, j);
        inven_aware(i_ptr);
        inven_known(i_ptr);
        (void)inven_carry(i_ptr);
    }
}


/*
 * Actually play a game
 */
void play_game()
{
    int i;
    int generate;
    int result = FALSE;


    /* Hack -- turn off the cursor */
    Term_hide_cursor();

    /* Grab a random seed from the clock */
    init_seeds();


    /*
     * This restoration of a saved character may get ONLY the monster memory.
     * In this case, load_player returns false. It may also resurrect a dead
     * character (if you are a wizard). In this case, it returns true, but
     * also sets the parameter "generate" to true, as it does not recover
     * any cave details.
     */

    /* Hack -- restore dead players (Unix) */
    if (fiddle) {
        result = load_player(&generate);
        if (result) save_player();
        quit(NULL);
    }

    /* If "restore game" requested, attempt to do so */
    if (!new_game) result = load_player(&generate);

    /* Enter wizard mode AFTER "resurrection" (if any) is complete */
    if (to_be_wizard && enter_wiz_mode()) wizard = TRUE;

    
    /* See above */
    if (!new_game && result) {

        /* Recalculate some stuff */
        p_ptr->update |= (PU_BONUS);
    
        /* Handle (non-visual) stuff */
        handle_stuff(FALSE);

        /* Display character, allow name change */
        change_name();

        /* Hack -- delayed death induced by certain signals */
        if (p_ptr->chp < 0) death = TRUE;
    }

    /* Create character */
    else {

        /* Roll up a new character */
        player_birth();

        /* Force "level generation" */
        generate = TRUE;

        /* Give him some stuff */
        player_outfit();

        /* Init the stores */
        store_init();

        /* Maintain the stores (ten times) */
        for (i = 0; i < 10; i++) store_maint();
    }


    /* Flavor the objects */
    flavor_init();


    /* Reset "rogue_like_commands" if requested */
    if (force_keyset) rogue_like_commands = force_keyset_arg;


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
    sprintf(buf, "%s.prf", player_name);

    /* Attempt to process that file */
    process_pref_file(buf);


    /* Begin the game */
    clear_screen();

    /* Show the frame */
    p_ptr->redraw |= (PR_BLOCK);
    
    /* Handle stuff */
    handle_stuff(TRUE);

    /* Fresh */
    Term_fresh();
    
    /* Generate a new level */
    if (generate) {

        /* Hack -- Flash a message */
        prt("Generating a new level...", 10, 20);

        /* Hack -- Flush the message */
        Term_fresh();
        
        /* Make a new level */
        generate_cave();
    }

    /* Character is now "complete" */
    character_generated = 1;


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

#ifdef _Windows
    /* Use the "pref-mac.prf" file */
    ANGBAND_SYS = "win";
#endif

    /* Play a game */
    play_game();
}


