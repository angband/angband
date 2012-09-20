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
static int force_rogue_like = FALSE;
static int force_keys_to = FALSE;




#if !defined(MACINTOSH) && !defined(_Windows)

/*
 * Unix machines need to "check wizard permissions"
 */
static bool is_wizard(int uid)
{
    int		test;
    FILE	*fp;
    char	buf[100];

    bool allow = FALSE;


    /* Open the wizard file */
    fp = my_tfopen(ANGBAND_WIZ, "r");

    /* No wizard file, so no wizards */
    if (!fp) return (FALSE);

    /* Scan the wizard file */
    while (!allow && fgets(buf, sizeof(buf), fp)) {
	if (buf[0] == '#') continue;
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
	    /* rogue_like_commands may be set in load_player() */
	    /* so delay this until after read savefile if any */
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
	  case 'x':
	  case 'X':
	    ANGBAND_A_LIST = &argv[0][2];
	    break;

	  default:
	  usage:

	    /* Note -- the Term is NOT initialized */
	    puts("Usage: angband [-nor] [-s<num>] [-u<name>] [-x<file>]");
	    puts("  n       Start a new character");
	    puts("  o       Use original command set");
	    puts("  r       Use the \"rogue-like\" command set");
	    puts("  s<num>  Show high scores.  Show <num> scores, or first 10");
	    puts("  u<name> Play with your <name> savefile");
	    puts("  x<file> Parse the attr/char info in the file <file>");
	    puts("Each option must be listed separately (ie '-r -n', not '-rn')");

	    /* Extra wizard options */
	    if (can_be_wizard) {
		puts("Extra wizard options: [-afw] [-p<uid>]");
		puts("  a       Activate \"peek\" mode");
		puts("  f       Enter \"fiddle\" mode");
		puts("  p<num>  Pretend to have the player uid number <num>");
		puts("  w       Start in wizard mode");
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

    /* XXX Default to "X11" if available XXX */

#ifdef USE_X11
    /* Attempt to use the "main-x11.c" support */
    if (!done) {
	extern errr init_x11(void);
	if (0 == init_x11()) done = TRUE;
    }
#endif

#ifdef USE_GCU
    /* Attempt to use the "main-gcu.c" support */
    if (!done) {
	extern errr init_gcu(void);
	if (0 == init_gcu()) done = TRUE;
    }
#endif

#ifdef USE_NCU
    /* Attempt to use the "main-ncu.c" support */
    if (!done) {
	extern errr init_ncu(void);
	/* When "init_ncu()" fails, it quits (?) */
	if (0 == init_ncu()) done = TRUE;
    }
#endif

#ifdef USE_CUR
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

    /* Check operating hours */
    read_times();

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
    register int i, j;
    inven_type inven_init;
    inven_type *i_ptr = &inven_init;


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
    int i;
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
    if (to_be_wizard && !enter_wiz_mode()) to_be_wizard = FALSE;


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

	/* Maintain the stores (ten times) */
	for (i = 0; i < 10; i++) store_maint();
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

    /* Play a game */
    play_game();
}


