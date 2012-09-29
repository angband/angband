/* File: death.c */ 

/* Purpose: code executed when player dies */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include <signal.h>

#include "angband.h"


#ifndef USG
# include <sys/param.h>
# include <sys/file.h>
#endif

#if !defined(MACINTOSH) && !defined(ATARIST_MWC)
# ifdef MSDOS
#  include <io.h>
# else
#  ifndef VMS
#   include <pwd.h>
#  else
#   include <file.h>
#  endif
# endif
#endif

#ifdef linux
# include <sys/file.h>
#endif

#if defined(USG) || defined(VMS)
# ifndef L_SET
#  define L_SET 0
# endif
#endif


/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__
static void  date(char *);
static char *center_string(char *, cptr);
static void  print_tomb(void);
static void  make_bones(void);
static void  kingly(void);
#endif
#endif

#ifndef MACINTOSH
char        *getlogin();
#endif

#if 0
# if !defined(time_t)
#  define time_t long
# endif
#endif


/*
 * Hack -- High score list entry
 */

typedef struct _high_score high_score;

struct _high_score {
  int32 pts;		/* Total Points */
  int16 cur_lev;	/* Current Player Level */
  int16 cur_dun;	/* Current Dungeon Level */
  int16 max_lev;	/* Max Player Level */
  int16 max_dun;	/* Max Dungeon Level */
  int16 cur_hit;	/* Current Hit Points */
  int16 max_hit;	/* Max Hit Points */
  int16 uid;		/* Player UID */
  int16 sex;		/* Player Sex */
  int16 p_r;		/* Player Race */
  int16 p_c;		/* Player Class */
  char who[24];		/* Player Name */
  char day[16];		/* Time stamp */
  char how[64];		/* Method of death */
};




/*
 * The "highscore" file descriptor
 */
static int highscore_fd = -1;


/*
 * Open the highscore file.  Allow writing iff "rdwr" is set.
 * Set "highscore_fd" which is used by the "highscore_xxx" functions
 */
static int highscore_open(int rdwr)
{
    int fd;
    char buf[1024];

    /* Permissions for file */
    int mode = 0666;
    
#ifdef SET_UID
    /* Reduce the permissions */
    mode = 0644;
#endif

    /* Extract the name of the High Score File */
    sprintf(buf, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "hiscores"); 

#ifdef MACINTOSH
    /* Global -- "Data file" */
    _ftype = 'DATA';
#endif

    /* Open a BINARY file, with the proper mode */
    if (rdwr) fd = my_topen(buf, O_RDWR | O_CREAT | O_BINARY, mode);
    else      fd = my_topen(buf, O_RDONLY | O_CREAT | O_BINARY, mode);

#ifdef MACINTOSH
    /* Global -- "Save file" */
    _ftype = 'SAVE';
#endif


    /* Check for failure (XXX or is it "< 0"?) */
    if (fd < 1) return (1);

    /* Save the fd */
    highscore_fd = fd;

    /* Success */
    return (0);
}


/*
 * Close the highscore file
 */
static int highscore_close()
{
    /* Already closed */
    if (highscore_fd < 0) return (1);

    /* All done */
    (void)close(highscore_fd);

    /* No fd */
    highscore_fd = -1;

    /* Success */
    return (0);
}


/*
 * Lock the highscore file
 */
static int highscore_lock()
{
    int oops;

#if !defined(MACINTOSH) && !defined(__EMX)

/* First, get a lock on the high score file so no-one else tries */
/* to write to it while we are using it */
/* added sys_v call to lockf - cba */

#ifdef USG
    oops = (lockf(highscore_fd, F_LOCK, 0) != 0);
#else
    oops = (0 != flock(highscore_fd, LOCK_EX));
#endif

    /* Trouble */
    if (oops) {
	msg_print("Error gaining lock for score file");
	return (1);
    }

#endif

    /* Success */
    return (0);
}


/*
 * Unlock the highscore file
 */
static int highscore_unlock()
{

#if !defined(MACINTOSH) && !defined(___EMX__)

/* added usg lockf call - cba */
#ifdef USG
    lockf(highscore_fd, F_ULOCK, 0);
#else
    (void)flock(highscore_fd, LOCK_UN);
#endif

#endif

    /* Success */
    return (0);
}


/*
 * Seek score 'i' in the highscore file
 */
static int highscore_seek(int i)
{

#ifndef BSD4_3
    (void)lseek(highscore_fd, (long)(i * sizeof(high_score)), L_SET);
#else
    (void)lseek(highscore_fd, (off_t)(i * sizeof(high_score)), L_SET);
#endif

    /* Success */
    return (0);
}


/*
 * Chop all scores from 'i' on from the highscore file
 */
static int highscore_chop(int i)
{

#if defined(sun) || defined(ultrix) || defined(NeXT)
    ftruncate(highscore_fd, i * sizeof(high_score));
#endif

    /* Success */
    return (0);
}


/*
 * Read one score from the highscore file
 */
static int highscore_read(high_score *score)
{
    int num;

    /* Read a record, note failure */
    num = read(highscore_fd, (char*)(score), sizeof(high_score));

    /* Nothing read, means end of file */
    if (!num) return (1);

    /* Partial read, means major error */
    /* if (num < sizeof(high_score)) return (-1); */

    /* Success */
    return (0);
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(high_score *score)
{
    /* XXX Check the return code */
    (void)write(highscore_fd, (char*)(score), sizeof(high_score));

    /* Success */
    return (0);
}


/*
 * Note that this function is called BEFORE "Term_init()"
 * init_scorefile Open the score file while we still have the setuid
 * privileges.  Later when the score is being written out, you must be sure
 * to flock the file so we don't have multiple people trying to write to it
 * at the same time. Craig Norborg (doc)		Mon Aug 10 16:41:59
 * EST 1987 
 */
void init_scorefile()
{
    if (highscore_open(1)) quit("cannot open high score file!");
}


/*
 * Shut down the scorefile
 */
void nuke_scorefile()
{
    /* Close the file */
    if (highscore_close()) quit("cannot close high score file!");
}



static void date(char *day)
{
    register char *tmp;
    time_t         c;

    c = time((time_t *)0);
    tmp = ctime(&c);
    tmp[10] = '\0';
    (void)strcpy(day, tmp);
}


/*
 * Centers a string within a 31 character string		-JWT-	 
 */
static char *center_string(char *centered_str, cptr in_str)
{
    register int i, j;

    i = strlen(in_str);
    j = 15 - i / 2;
    (void)sprintf(centered_str, "%*s%s%*s", j, "", in_str, 31 - i - j, "");
    return centered_str;
}


/*
 * Pauses for user response before returning		-RAK-	 
 */
int look_line(int prt_line)
{
    int i;
    prt("[Press ESC to quit, any other key to continue.]", prt_line, 17);
    i = inkey();
    erase_line(prt_line, 0);
    return ((i == ESCAPE) ? 0 : 1);
}


/*
 * Display the scores 
 */
void display_scores(int from, int to)
{
    register int i = 0, j, k, l;

    high_score  score;

    char         buf[256];

    vtype        tmp_str;

    /* Assume we will show the first 10 */
    if (to < 0) to = 10;
    if (to > MAX_SAVE_HISCORES) to = MAX_SAVE_HISCORES;

    clear_screen();

    /* Close the highscore file (in case its already open) */
    highscore_close();

    /* Open the highscore file, but only for reading */
    if (highscore_open(0)) {
	sprintf(buf, "Can't open score file!\n");
	message(buf,0);
	return;
    }

    /* Seek to the beginning */
    if (highscore_seek(0)) quit("oops#1");

    /* Count the high scores */
    for (i = 0; i < MAX_SAVE_HISCORES; i++) {
	if (highscore_read(&score)) break;
    }

    /* Forget about the last ones */
    if (i > to) i = to;


#ifdef SIGTSTP
    /* Ignore "suspend" signal */
    signal(SIGTSTP, SIG_IGN);
#endif

    /* Show 10 per page */
    for (k = from; k < i; k += 10) {

	put_str("                Angband Hall of Fame", 0, 0);

	if (k > 0) {
	    sprintf(tmp_str, "(from position %d)", k + 1);
	    put_str(tmp_str, 0, 40);
	}

	put_str("     Score", 1, 0);

	for (j = k, l = 0; j < i && l < 10; j++, l++) {

	    if (highscore_seek(j)) quit("oops#2");

	    if (highscore_read(&score)) quit("oops#3");

	    (void)sprintf(buf, "%3d) %-7ld %s the %s %s (Level %d)",
			  j + 1, (long)score.pts, score.who,
			  race[score.p_r].trace, class[score.p_c].title,
			  score.cur_lev);
	    put_str(buf, l*2 + 2, 0);

	    if (score.cur_dun) {
		sprintf(buf,
			"             Killed by %s on Dungeon Level %d.",
			score.how, score.cur_dun);
	    }
	    else {
		sprintf(buf,
			"             Killed by %s on the Town Level.",
			score.how);
	    }

	    put_str(buf, l*2 + 3, 0);
	}

	/* Wait for response.  Hack -- exit on "Escape" */
	if (!look_line(23)) {

	    highscore_close();
	    msg_print(NULL);
	    clear_screen();
	    Term_fresh();

	    /* flush all input */
	    flush();

	    /* No suspending now. */
	    signals_ignore_tstp();

	    /* Save the memory at least. */
	    (void)save_player();

	    /* Exit */
	    quit(NULL);
	}

	/* Clear those */
	clear_screen();
    }

    /* Close the high scores */
    highscore_close();
}


/*
 * Save a "bones" file for a dead character
 * Should probably attempt some form of locking...
 */
static void make_bones(void)
{
    FILE                *fp;

    char                str[1024];


    /* Not interupted, not wizard, make bones file */
    if (stricmp(died_from, "Interrupting") && !wizard) {

	/* If died on "interesting" level... */
	if (dun_level > 1) {

	    /* Get the proper "Bones File" name */
	    sprintf(str, "%s%s%d", ANGBAND_DIR_BONES, PATH_SEP, dun_level);

	    /* Attempt to open the bones file */
	    fp = my_tfopen(str, "r");

	    /* Do not over-write a previous ghost */
	    if (fp) return;

	    /* Try to write a new "Bones File" */
	    fp = my_tfopen(str, "w");

	    /* Not allowed to write it?  Weird. */
	    if (!fp) return;

	    /* Save the info */
	    fprintf(fp, "%s\n", player_name);
	    fprintf(fp, "%d\n", p_ptr->mhp);
	    fprintf(fp, "%d\n", p_ptr->prace);
	    fprintf(fp, "%d\n", p_ptr->pclass);

	    /* Close and save the Bones file */
	    fclose(fp);
	}
    }
}


/*
 * Prints the gravestone of the character  -RAK-
 */
static void print_tomb()
{
    register int         i;
    char                 day[11];
    cptr		 p;
    store_type		*st_ptr;
    int			 j, k;
    vtype		 t1, t2;
    vtype                str, tmp_str;
    inven_type		*i_ptr;


    /* Draw a tombstone */
    clear_screen();
    put_str("_______________________", 1, 15);
    put_str("/", 2, 14);
    put_str("\\         ___", 2, 38);
    put_str("/", 3, 13);
    put_str("\\ ___   /   \\      ___", 3, 39);
    put_str("/            RIP            \\   \\  :   :     /   \\", 4, 12);
    put_str("/", 5, 11);
    put_str("\\  : _;,,,;_    :   :", 5, 41);
    (void)sprintf(str, "/%s\\,;_          _;,,,;_",
		  center_string(tmp_str, player_name));
    put_str(str, 6, 10);
    put_str("|               the               |   ___", 7, 9);
    p = total_winner ? "Magnificent" : title_string();
    (void)sprintf(str, "| %s |  /   \\", center_string(tmp_str, p));
    put_str(str, 8, 9);
    put_str("|", 9, 9);
    put_str("|  :   :", 9, 43);
    if (!total_winner) {
	p = class[p_ptr->pclass].title;
    }
    else if (p_ptr->male) {
	p = "*King*";
    }
    else {
	p = "*Queen*";
    }

    (void)sprintf(str, "| %s | _;,,,;_   ____", center_string(tmp_str, p));
    put_str(str, 10, 9);
    (void)sprintf(str, "Level : %d", (int)p_ptr->lev);
    (void)sprintf(str, "| %s |          /    \\", center_string(tmp_str, str));
    put_str(str, 11, 9);
    (void)sprintf(str, "%ld Exp", (long)p_ptr->exp);
    (void)sprintf(str, "| %s |          :    :", center_string(tmp_str, str));
    put_str(str, 12, 9);
    (void)sprintf(str, "%ld Au", (long)p_ptr->au);
    (void)sprintf(str, "| %s |          :    :", center_string(tmp_str, str));
    put_str(str, 13, 9);
    (void)sprintf(str, "Died on Level : %d", dun_level);
    (void)sprintf(str, "| %s |         _;,,,,;_", center_string(tmp_str, str));
    put_str(str, 14, 9);
    put_str("|            killed by            |", 15, 9);
    p = died_from;
    i = strlen(p);
    died_from[i] = '.';			   /* add a trailing period */
    died_from[i + 1] = '\0';
    (void)sprintf(str, "| %s |", center_string(tmp_str, p));
    put_str(str, 16, 9);
    died_from[i] = '\0';		   /* strip off the period */
    date(day);
    (void)sprintf(str, "| %s |", center_string(tmp_str, day));
    put_str(str, 17, 9);
    put_str("*|   *     *     *    *   *     *  | *", 18, 8);
    put_str("________)/\\\\_)_/___(\\/___(//_\\)/_\\//__\\\\(/_|_)_______",
	       19, 0);

    /* Flush all input keys */
    flush();

    put_str("(ESC to abort, return to print on screen)", 23, 0);
    put_str("Character record?", 22, 0);

    if (!get_string(str, 22, 18, 60)) return;


    /* Know everything the player is wearing/carrying */
    for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	if (i == inven_ctr) i = INVEN_WIELD;
	i_ptr = &inventory[i];
	if (i_ptr && (i_ptr->tval != TV_NOTHING)) {
	    inven_aware(i_ptr);
	    known2(i_ptr);
	}
    }

    /* Show player */
    clear_screen();
    calc_bonuses();
    display_player();

    put_str("Type ESC to skip the inventory:", 23, 0);

    if (inkey() == ESCAPE) return;


    /* Show equipment and inventory */

    /* Equipment XXX Assume there is some */
    if (equip_ctr) {
	clear_screen();
	msg_print("You are using:");
	(void)show_equip(TRUE, 0);
	msg_print(NULL);
    }

    /* Inventory -- no assumptions */
    if (inven_ctr) {
	clear_screen();
	msg_print("You are carrying:");
	(void)show_inven(0, inven_ctr - 1, TRUE, 0, 0);
	msg_print(NULL);
    }

    /* Access the home */
    st_ptr = &store[7];

    /* Only show home if stuff in it */
    if (st_ptr->store_ctr) {

	/* show home's inventory... */
	for (k = 0, i = 0; i < st_ptr->store_ctr; k++) {

	    /* What did they hoard? */
	    clear_screen();
	    sprintf(t2, "You have stored at your house (page %d):", k);
	    msg_print(t2);

	    /* Show 12 (XXX) items (why bother with indexes?) */
	    for (j = 0; j < 12 && i < st_ptr->store_ctr; j++, i++) {
		i_ptr = &st_ptr->store_item[i];
		inven_aware(i_ptr);
		known2(i_ptr);
		objdes(t1, i_ptr, TRUE);
		sprintf(t2, "%c) ", 'a'+j);
		prt(t2, j+2, 4);
		c_prt(inven_attr_by_tval(i_ptr), t1, j+2, 7);
	    }

	    /* Flush it */
	    msg_print(NULL);
	}
    }
}


/*
 * Calculates the total number of points earned		-JWT-	 
 */
long total_points(void)
{
    return (p_ptr->max_exp + (100 * p_ptr->max_dlv));
}


/*
 * Enters a players name on a hi-score table...    SM 
 */
static int top_twenty(void)
{
    register int        i, j = 0, k = 0;
    high_score         score, myscore, tmpscore;

    /* Wipe screen */
    clear_screen();

    /* Wizard only sees scores (and saves himself) */
    if (wizard || to_be_wizard) {
	display_scores(0, 10);
	(void)save_player();
	quit(NULL);
    }

    /* Interupted */    
    if (!total_winner && !stricmp(died_from, "Interrupting")) {
	msg_print("Score not registered due to interruption.");
	display_scores(0, 10);
	(void)save_player();
	quit(NULL);
    }

    /* Quitter */
    if (!total_winner && !stricmp(died_from, "Quitting")) {
	msg_print("Score not registered due to quitting.");
	display_scores(0, 10);
	(void)save_player();
	quit(NULL);
    }

    /* Calculate points */
    myscore.pts = total_points();
    myscore.cur_lev = p_ptr->lev;
    myscore.cur_dun = dun_level;
    myscore.max_lev = p_ptr->max_plv;
    myscore.max_dun = p_ptr->max_dlv;
    myscore.cur_hit = p_ptr->chp;
    myscore.max_hit = p_ptr->mhp;
    myscore.uid = player_uid;
    myscore.sex = p_ptr->male;
    myscore.p_r = p_ptr->prace;
    myscore.p_c = p_ptr->pclass;

    /* Save the time */
    date(myscore.day);

    /* Save the player name */
    (void)sprintf(myscore.who, "%-.15s", player_name);

    /* Save the cause of death */
    (void)sprintf(myscore.how, "%-.63s", died_from);

    /* XXX The highscore file is open from earlier */

    /* Lock the highscore file */
    if (highscore_lock()) quit("oops#6");

    /* Go to the start of the highscore file */
    if (highscore_seek(0)) quit("oops#7");

    /* read until we get to a higher score */
    for (i = 0; i < MAX_SAVE_HISCORES; i++) {
	if (highscore_read(&score)) break;
	if (score.pts < myscore.pts) break;
    }

    /* If there is room, add our score to the list */
    if (i < MAX_SAVE_HISCORES) {

	/* Back up (or stay still) to the score to be replaced */
	if (highscore_seek(i)) quit("oops#8");

	/* Add our score */
	if (highscore_write(&myscore)) quit("oops#9");

	/* Remember where we were for display below */
	j = i;

	/* Set the "nobody left" flag */
	k = 0;

	/* Scan all the other scores */
	for (i++; i < MAX_SAVE_HISCORES; i++) {

	    /* Read the guy who used to be here */
	    if (highscore_read(&tmpscore)) break;

	    /* Back up and dump the score we were holding */
	    if (highscore_seek(i)) quit("oops#10");
	    if (highscore_write(&score)) quit("oops#11");

	    /* Save the old score, for the next pass */
	    score = tmpscore;

	    /* set flag to indicate score left to be written */
	    k=1;
	}

	/* write last guy in */
	if (k && i < MAX_SAVE_HISCORES) {
	    if (highscore_seek(i)) quit("oops#12");
	    if (highscore_write(&score)) quit("oops#13");
	}
    }

    /* Unlock the highscore file */
    highscore_unlock();

    /* Close the highscore file */
    highscore_close();

    /* Note: 'j' is the player's place, and 'i' is the last place */

    if (j < 10) {
	display_scores(0, 10);
    }
    else if (j > (i - 5)) {
	display_scores(i - 9, i + 1);
    }
    else {
	display_scores(j - 5, j + 5);
    }

    return (0);
}


#ifndef MACINTOSH
/*
 * Remove an entry from the hi-score table     SM	 
 * Note that the first entry is given as (which == 1)
 * XXX This is a hack, could use much less space.
 */
void delete_entry(int which)
{
    register int i, j;

    int k = which - 1;

    high_score  scores[MAX_SAVE_HISCORES];


    /* XXX The highscore file is open from earlier */

    /* Lock it */
    if (highscore_lock()) quit("oops#21");

    /* Go to the start -- XXX necessary? */
    if (highscore_seek(0)) quit("oops#22");

    /* Count the high scores, and save them */
    for (i = 0; i < MAX_SAVE_HISCORES; i++) {
	if (highscore_read(&scores[i])) break;
    }

    if (k < i) {

	/* Chop the file (all of the entries) */
	if (highscore_chop(0)) quit("oops#23");

	/* Go back to the beginning */
	if (highscore_seek(0)) quit("oops#24");

	/* Restore the first part */
	for (j = 0; j < which; ++j) {
	    highscore_write(&scores[j]);
	}

	/* Restore the second part */
	for (j = k + 1; j < i; ++j) {
	    highscore_write(&scores[j]);
	}
    }

    else {
	msg_print("The high score table does not have that many entries!");
    }


    /* Unlock the highscore file */
    highscore_unlock();

    /* Close the highscore file */
    highscore_close();


    /* Display what is left */
    if (k < 10) {
	display_scores(0, 10);
    }
    else if (k + 10 > i) {
	display_scores(i - 10, i);
    }
    else {
	display_scores(k - 5, k + 5);
    }
}

#endif


/*
 * Change the player into a King!			-RAK-	 
 */
static void kingly()
{
    register cptr p;

    /* Change the character attributes.		 */
    dun_level = 0;
    (void)strcpy(died_from, "Ripe Old Age");
    (void)restore_level();
    p_ptr->lev += MAX_PLAYER_LEVEL;
    p_ptr->au += 250000L;
    p_ptr->max_exp += 5000000L;
    p_ptr->exp = p_ptr->max_exp;

    /* Let the player know that he did good.	 */
    clear_screen();
    put_str("#", 1, 34);
    put_str("#####", 2, 32);
    put_str("#", 3, 34);
    put_str(",,,  $$$  ,,,", 4, 28);
    put_str(",,=$   \"$$$$$\"   $=,,", 5, 24);
    put_str(",$$        $$$        $$,", 6, 22);
    put_str("*>         <*>         <*", 7, 22);
    put_str("$$         $$$         $$", 8, 22);
    put_str("\"$$        $$$        $$\"", 9, 22);
    put_str("\"$$       $$$       $$\"", 10, 23);
    p = "*#########*#########*";
    put_str(p, 11, 24);
    put_str(p, 12, 24);
    put_str("Veni, Vidi, Vici!", 15, 26);
    put_str("I came, I saw, I conquered!", 16, 21);
    if (p_ptr->male) {
	put_str("All Hail the Mighty King!", 17, 22);
    }
    else {
	put_str("All Hail the Mighty Queen!", 17, 22);
    }
    flush();
    pause_line(23);
}


/*
 * Exit the game
 *   Save the player to a save file
 *   Display a gravestone / death info
 *   Display the top twenty scores
 */
void exit_game(void)
{
    /* Flush the messages */
    msg_print(NULL);

    /* Flush the input */
    flush();

    /* No suspending now */
    signals_ignore_tstp();

    /* Player must be dead */
    if (turn > 0) {

	/* But he may have won first */
	if (total_winner) kingly();

	/* Dump bones file */
	make_bones();

	/* You are dead */
	print_tomb();

	/* Handle score, show Top scores */
	if (!wizard && !to_be_wizard) {
	    top_twenty();
	}
	else {
	    msg_print("Wizard scores not registered.");
	}
    }

    /* On Ctrl('C'), for example */
    else {
	display_scores(0, 10);
	erase_line(23, 0);
    }

    /* Save the player's memories */
    (void)save_player();

    /* Actually stop the process */
    quit(NULL);
}

