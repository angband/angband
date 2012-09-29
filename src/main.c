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


#if !defined(USG) || !defined(ibm032)
# include <time.h>
#endif

#ifdef ultrix
# include <sys/stat.h>
#endif


/*
 * Hack -- Local "game mode" vars
 */
static int new_game = FALSE;
static int fiddle = FALSE;
static int force_rogue_like = FALSE;
static int force_keys_to = FALSE;



#if defined(MACINTOSH) || defined(_Windows)

/*
 * This is the Macintosh "hook" into "play_game()"
 * See macintosh.c for actual "main()" function 
 */

void play_game_mac (int ng)
{
    /* No name (yet) */
    strcpy(player_name, "");

    /* Hack -- assume wizard permissions */
    can_be_wizard = TRUE;

    /* Hack -- extract a flag */
    new_game = ng;

    /* Play a game */
    play_game();
}

#else /* MACINTOSH */

/*
 * Unix machines need to "check wizard permissions"
 */
static bool is_wizard(int uid)
{
    FILE *fp;
    char  buf[100];

    int   test = FALSE;

    /* Open the wizard file */
    fp = my_tfopen(ANGBAND_WIZ, "r");

    /* No wizard file, so no wizards */
    if (!fp) return (FALSE);

    /* Scan the wizard file */
    while (fgets(buf, sizeof(buf), fp)) {
	if (buf[0] != '#') {
	    if (sscanf(buf, "%d", &test)) {
		if (test == uid) {
		    test = TRUE;
		    break;
		}
	    }
	}
    }

    /* Close the file */
    fclose(fp);

    /* Return TRUE if found */
    return (test);
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

    /* Shut down the terminal */
    Term_nuke();
}


/*
 * Studly machines can actually parse command line args
 */
int main(int argc, char *argv[])
{
    bool done = FALSE;
    
    /* Dump score list (num lines)? */
    int show_score = 0;

    /* Remove score (which one)? */
    int kill_score = 0;

#ifndef __MINT__
#ifdef CHECK_LOAD
    FILE *fp;
    char temphost[MAXHOSTNAMELEN+1];
    char thishost[MAXHOSTNAMELEN+1];
    char discard[120];
#endif
#endif


    /* Save the "program name" */
    argv0 = argv[0];
    

#ifndef SET_UID
# if !defined(MSDOS) && !defined(__EMX__)
    (void) umask(0);
# endif
#endif

#ifdef SECURE
    Authenticate();
#endif

    /* Get the file paths */
    get_file_paths();

    /* Prepare the "hiscore" file (while we have permission) */
    init_scorefile();

    /* Get the user id (?) */
    player_uid = getuid();

#if defined(SET_UID) && !defined(SECURE)
    /* Set the user id or quit */
    if (setuid(geteuid()) != 0) {
	quit("setuid(): cannot set permissions correctly!");
    }
#endif

    /* Check for "Wizard" permission */
    can_be_wizard = is_wizard(player_uid);

#ifdef CHECK_LOAD
    (void)gethostname(thishost, (sizeof thishost) - 1);	/* get host */
    fp = my_tfopen(ANGBAND_LOAD, "r");
    if (!fp) quit("cannot get load-check!");

    /* Find ourself */
    while (1) {
	if (fscanf(fp, "%s%d", temphost, &LOAD) == EOF) {
	    LOAD=100;
	    break;
	}

	/* Hack -- Discard comments */
	if (temphost[0]=='#') {
	    (void)fgets(discard, (sizeof discard)-1, fp);
	    continue;
	}

	if (!strcmp(temphost,thishost) || !strcmp(temphost,"localhost")) break;
    }

    fclose(fp);
#endif

    /* Acquire the "user name" as a default player name */
    user_name(player_name, player_uid);

    /* check for user interface option */
    for (--argc, ++argv; argc > 0 && argv[0][0] == '-'; --argc, ++argv) {
	switch (argv[0][1]) {
	  case 'A':
	  case 'a':
	    if (!can_be_wizard) goto usage;
	    peek=TRUE;
	    break;
	  case 'N':
	  case 'n':
	    new_game = TRUE;
	    break;
	  case 'O':
	  case 'o':
	    /* rogue_like_commands may be set in load_player(), so delay this
	       until after read savefile if any */
	    force_rogue_like = TRUE;
	    force_keys_to = FALSE;
	    break;
	  case 'R':
	  case 'r':
	    force_rogue_like = TRUE;
	    force_keys_to = TRUE;
	    break;
	  case 'S':
	  case 's':
	    show_score = atoi(&argv[0][2]);
	    if (show_score <= 0) show_score = 10;
	  case 'D':
	  case 'd':
	    if (!can_be_wizard) goto usage;
	    kill_score = atoi(&argv[0][2]);
	    if (kill_score <= 0) show_score = 10;
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
	    
	    if (can_be_wizard) {
		puts("Usage: angband [-afnorw] [-u<name>] [-s<num>] [-d<num>] [-p<uid>]");
		puts("  a       Activate \"peek\" mode");
		puts("  d<num>  Delete high score number <num>");
		puts("  f       Enter \"fiddle\" mode");
		puts("  n       Start a new character");
		puts("  o       Use original command set");
		puts("  p<num>  Pretend to have the player uid number <num>");
		puts("  r       Use the \"rogue-like\" command set");
		puts("  s<num>  Show high scores.  Show <num> scores, or first 10");
		puts("  u<name> Play with your <name> savefile");
		puts("  w       Start in wizard mode");
		puts("Each option must be listed separately (ie '-r -n', not '-rn')");
	    }
	    else {
		puts("Usage: angband [-nor] [-s<num>] [-u<name>]");
		puts("  n       Start a new character");
		puts("  o       Use original command set");
		puts("  r       Use the \"rogue-like\" command set");
		puts("  s<num>  Show high scores.  Show <num> scores, or first 10");
		puts("  u<name> Play with your <name> savefile");
		puts("Each option must be listed separately (ie '-r -n', not '-rn')");
	    }

	    /* Actually abort the process */
	    quit(NULL);
	}
    }

    /* Verify the "player name" */
    if (!name_okay(player_name)) quit("bad player name");


#ifdef USE_IBM
    /* Attempt to use the "main-ibm.c" support */
    if (!done) {
	extern errr init_ibm(void);
        if (0 == init_ibm()) done = TRUE;
    }
#endif

#ifdef __EMX__
    /* Attempt to use the "main-emx.c" support */
    if (!done) {
	extern errr init_emx(void);
        if (0 == init_emx()) done = TRUE;
    }
#endif

    /* XXX Default to "X11" if available -- should have a "command line choice" */

#ifdef USE_X11
    /* Attempt to use the "main-x11.c" support */
    if (!done) {
	extern errr init_x11(cptr, cptr);
	/* Note that a "warning" will be produced if there is no */
	/* Display to open.  This will annoy "curses" users, I assume. */
        if (0 == init_x11("", USE_X11_FONT)) done = TRUE;
    }
#endif

#ifdef USE_CURSES
    /* Attempt to use the "main-cur.c" support */
    if (!done) {
	extern errr init_cur(void);
	/* When "init_cur()" fails, it quits (?) */
	if (0 == init_cur()) done = TRUE;
    }
#endif

    /* Make sure we have a display! */
    if (!done) quit("Unable to prepare any 'display module'!");


#ifndef USE_COLOR
    /* Turn off color */
    use_color = FALSE;
#endif


    /* Set up the Terminal (using whatever was installed above) */
    Term_init();

    /* Tell "quit()" to call "Term_nuke()" */
    quit_aux = quit_hook;
    

    /* Handle "delete score" requests */
    if (kill_score > 0) {
	delete_entry(kill_score);
	quit(NULL);
    }

    /* Handle "score list" requests */
    if (show_score > 0) {
	display_scores(0, show_score);
	quit(NULL);
    }

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

    /* Check operating hours */
    read_times();

    /* Show news file */
    show_news();

    /* Label the task (can take a while) */
    prt("[Initializing arrays...]", 23, 20);
    Term_fresh();
    init_some_arrays();
    pause_line(23);

    /* Call the main function */
    play_game();

    /* Exit (never gets here) */
    return (0);
}

#endif /* MACINTOSH */




/*
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit()
{
    register int i, j;
    inven_type inven_init;
    inven_type *i_ptr = &inven_init;


    /* No items */
    inven_ctr = equip_ctr = 0;

    /* No weight */
    inven_weight = 0;

    /* Clear the inventory */
    for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	invcopy(&inventory[i], OBJ_NOTHING);
    }


    /* Give the player some food */
    invcopy(i_ptr, OBJ_FOOD_RATION);
    i_ptr->number = rand_range(3,7);
    inven_aware(i_ptr);
    known2(i_ptr);
    (void)inven_carry(i_ptr);

    /* Give the player some torches */
    invcopy(i_ptr, OBJ_TORCH);
    i_ptr->number = rand_range(3,7);
    i_ptr->pval = rand_range(3,7) * 500;
    known2(i_ptr);
    (void)inven_carry(i_ptr);

    /* Give the player three useful objects */
    for (i = 0; i < 3; i++) {
	j = player_init[p_ptr->pclass][i];
	invcopy(i_ptr, j);
	inven_aware(i_ptr);
	known2(i_ptr);
	(void)inven_carry(i_ptr);
    }
}


/*
 * Actually play a game
 */
void play_game()
{
    int generate;
    int result = FALSE;

    /* Hack -- turn off the cursor */
    Term_hide_cursor();

    /* Grab a random seed from the clock          */
    init_seeds();

    /* Load and re-save a player's character (only Unix) */
    if (fiddle) {
	if (load_player(&generate)) save_player();
	quit(NULL);
    }

    /*
     * This restoration of a saved character may get ONLY the monster memory.
     * In this case, load_player returns false. It may also resurrect a dead
     * character (if you are a wizard). In this case, it returns true, but
     * also sets the parameter "generate" to true, as it does not recover
     * any cave details.
     */

    /* If "restore game" requested, attempt to do so */
    if (!new_game) result = load_player(&generate);

    /* Enter wizard mode AFTER "resurrection" (if any) is complete */
    if (to_be_wizard) {
	/* Enter wizard mode or forget about it */
	if (!enter_wiz_mode()) to_be_wizard = FALSE;
    }

    /* See above */
    if (!new_game && result) {

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

	/* Maintain the stores (three times) */
	store_maint();
	store_maint();
	store_maint();
    }

    /* Reset "rogue_like_commands" */
    if (force_rogue_like) {
	rogue_like_commands = force_keys_to;
    }

    /* Prep the object descriptions */
    flavor_init();

    /* Begin the game */
    clear_screen();
    prt_stat_block();

    /* Hack -- Flash a message */
    if (generate) prt("Generating a new level...", 10, 20);

    /* Flush it */
    Term_fresh();

    /* Make a level */
    if (generate) generate_cave();

    /* Character is now complete */
    character_generated = 1;


    /* Loop till dead, or exit			*/
    while (!death) {

	/* Process the level */
	dungeon();

	/* check for eof here, see inkey() in io.c */
	/* eof can occur if the process gets a HANGUP signal */
	if (eof_flag) {

	    (void) strcpy(died_from, "(end of input: saved)");
	    if (save_player()) quit(NULL);

	    /* should not reach here, by if we do, this guarantees exit */
	    (void) strcpy(died_from, "unexpected eof");
	    death = TRUE;
	}

	/* Make the New level */
	if (!death) generate_cave();
    }

    /* Display death, Save the game, and exit */
    exit_game();
}


