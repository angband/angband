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
 * Some machines have a "main()" function in their "main-xxx.c" file,
 * all the others use this file for their "main()" function.
 */

 
#if !defined(MACINTOSH) && !defined(WINDOWS) && !defined(ACORN)


#ifdef SET_UID

/*
 * Check "wizard permissions"
 */
static bool is_wizard(int uid)
{
    FILE	*fp;

    bool	allow = FALSE;

    char	buf[1024];


    /* Access the "wizards.txt" file */
    sprintf(buf, "%s%s", ANGBAND_DIR_FILE, "wizards.txt");

    /* Open the wizard file */
    fp = my_fopen(buf, "r");

    /* No file, allow everyone */
    if (!fp) return (TRUE);

    /* Scan the wizard file */
    while (!allow && fgets(buf, sizeof(buf), fp)) {

        int test;

        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Look for valid entries */
        if (sscanf(buf, "%d", &test) != 1) continue;

        /* Look for matching entries */
        if (test == uid) allow = TRUE;
    }

    /* Close the file */
    my_fclose(fp);

    /* Result */
    return (allow);
}

#endif


/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(cptr s)
{
    /* Shut down the term windows */
    if (term_choice) term_nuke(term_choice);
    if (term_recall) term_nuke(term_recall);
    if (term_mirror) term_nuke(term_mirror);
    if (term_screen) term_nuke(term_screen);
}



/*
 * Set the stack size (for the Amiga)
 */
#ifdef AMIGA
# include <dos.h>
__near long __stack = 32768L;
#endif


/*
 * Set the stack size (see main-286.c")
 */
#ifdef USE_286
# define delay delay_hack
# include <dos.h>
# undef delay
extern unsigned _stklen = 32768U;
#endif


/*
 * Initialize and verify the file paths, and the score file.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * DEFAULT_PATH, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the DEFAULT_PATH constant.  So be sure that one of
 * these two things works...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed.
 */
static void init_stuff(void)
{
    char path[1024];


#if defined(AMIGA)

    /* Hack -- prepare "path" */
    strcpy(path, "Angband:");

#else

    cptr tail;
    
    /* Get the environment variable */
    tail = getenv("ANGBAND_PATH");

    /* Use the angband_path, or a default */
    strcpy(path, tail ? tail : DEFAULT_PATH);

    /* Hack -- Add a path separator (only if needed) */
    if (!suffix(path, PATH_SEP)) strcat(path, PATH_SEP);

#endif

    /* Initialize */
    init_file_paths(path);
}



/*
 * Studly machines can actually parse command line args
 *
 * XXX XXX XXX The "-c", "-d", and "-i" options should probably require
 * that their "arguments" end in the appropriate path separator.
 */
int main(int argc, char *argv[])
{
    bool done = FALSE;

    bool new_game = FALSE;
    
    int show_score = 0;


    /* Save the "program name" */
    argv0 = argv[0];


#ifdef SET_UID

    /* Default permissions on files */
    (void)umask(022);

# ifdef SECURE
    /* Authenticate */
    Authenticate();
# endif

#endif


    /* Get the file paths */
    init_stuff();


#ifdef SET_UID

    /* Get the user id (?) */
    player_uid = getuid();

#ifdef VMS
    /* Mega-Hack -- Factor group id */
    player_uid += (getgid() * 1000);
#endif

# ifdef SAFE_SETUID

#  ifdef _POSIX_SAVED_IDS

    /* Save some info for later */
    player_euid = geteuid();
    player_egid = getegid();

#  endif

#  if 0	/* XXX XXX XXX */

    /* Redundant setting necessary in case root is running the game */
    /* If not root or game not setuid the following two calls do nothing */

    if (setgid(getegid()) != 0) {
      quit("setgid(): cannot set permissions correctly!");
    }

    if (setuid(geteuid()) != 0) {
      quit("setuid(): cannot set permissions correctly!");
    }

#  endif

# endif

#endif


    /* Assume "Wizard" permission */
    can_be_wizard = TRUE;

#ifdef SET_UID

    /* Check for "Wizard" permission */
    can_be_wizard = is_wizard(player_uid);

    /* Initialize the "time" checker */
    if (check_time_init() || check_time()) {
        quit("The gates to Angband are closed (bad time).");
    }

    /* Initialize the "load" checker */
    if (check_load_init() || check_load()) {
        quit("The gates to Angband are closed (bad load).");
    }

    /* Acquire the "user name" as a default player name */
    user_name(player_name, player_uid);

#endif


    /* check for user interface option */
    for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
        switch (argv[0][1]) {

          case 'c':
          case 'C':
            ANGBAND_DIR_USER = &argv[0][2];
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
            arg_force_roguelike = TRUE;
            break;

          case 'O':
          case 'o':
            arg_force_original = TRUE;
            break;

          case 'S':
          case 's':
            show_score = atoi(&argv[0][2]);
            if (show_score <= 0) show_score = 10;
            break;

          case 'F':
          case 'f':
            if (!can_be_wizard) goto usage;
            arg_fiddle = arg_wizard = TRUE;
            break;

#ifdef SET_UID
# ifdef SAVEFILE_USE_UID
          case 'P':
          case 'p':
            if (!can_be_wizard) goto usage;
            if (isdigit((int)argv[0][2])) {
                player_uid = atoi(&argv[0][2]);
                user_name(player_name, player_uid);
            }
            break;
# endif
#endif

          case 'W':
          case 'w':
            if (!can_be_wizard) goto usage;
            arg_wizard = TRUE;
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

            /* XXX XXX XXX Command line option setters? */

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

#ifdef USE_CAP
    /* Attempt to use the "main-cap.c" support */
    if (!done) {
        extern errr init_cap(void);
        if (0 == init_cap()) done = TRUE;
        if (done) ANGBAND_SYS = "cap";
    }
#endif


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


#ifdef USE_AMY
    /* Attempt to use the "main-amy.c" support */
    if (!done) {
        extern errr init_amy(void);
        if (0 == init_amy()) done = TRUE;
        if (done) ANGBAND_SYS = "amy";
    }
#endif


#ifdef USE_LSL
    /* Attempt to use the "main-lsl.c" support */
    if (!done) {
        extern errr init_lsl(void);
        if (0 == init_lsl()) done = TRUE;
        if (done) ANGBAND_SYS="lsl";
    }
#endif


    /* Grab privs (dropped above for X11) */
    safe_setuid_grab();


    /* Make sure we have a display! */
    if (!done) quit("Unable to prepare any 'display module'!");


    /* Tell "quit()" to call "Term_nuke()" */
    quit_aux = quit_hook;


    /* If requested, display scores and quit */
    if (show_score > 0) display_scores(0, show_score);


    /* Catch nasty signals */
    signals_init();

    /* Display the 'news' file */
    show_news();

    /* Initialize the arrays */
    init_some_arrays();

    /* Wait for response */
    pause_line(23);

    /* Play the game */
    play_game(new_game);

    /* Quit */
    quit(NULL);

    /* Exit */
    return (0);
}

#endif



