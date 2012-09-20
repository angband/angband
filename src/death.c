/* File: death.c */ 

/* Purpose: code executed when player dies */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"

#ifndef USG
# include <sys/param.h>
# include <sys/file.h>
#endif

#if !defined(MACINTOSH) && !defined(ATARIST_MWC) && !defined(AMIGA)
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

#ifndef L_SET
# define L_SET 0
#endif



/*
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct _high_score high_score;

struct _high_score {

  char what[8];		/* Version info (string) */
  
  char pts[10];		/* Total Score (number) */

  char gold[10];	/* Total Gold (number) */

  char turns[10];	/* Turns Taken (number) */
  
  char day[10];		/* Time stamp (string) */

  char who[16];		/* Player Name (string) */

  char uid[8];		/* Player UID (number) */

  char sex[2];		/* Player Sex (string) */
  char p_r[3];		/* Player Race (number) */
  char p_c[3];		/* Player Class (number) */

  char cur_lev[4];	/* Current Player Level (number) */
  char cur_dun[4];	/* Current Dungeon Level (number) */
  char max_lev[4];	/* Max Player Level (number) */
  char max_dun[4];	/* Max Dungeon Level (number) */

  char how[32];		/* Method of death (string) */
};




/*
 * The "highscore" file descriptor
 */
static int highscore_fd = -1;


/*
 * Open the highscore file (for reading/writing).  Create if needed.
 * Set "highscore_fd" which is used by the "highscore_*" functions.
 */
static errr highscore_open(void)
{
    int fd;
    char buf[1024];

    /* Permissions for file */
    int mode = 0666;
    
#ifdef SET_UID
    /* Reduce the permissions */
    mode = 0644;
#endif

    /* Extract the name of the High Score File (not really a "raw" file) */
    sprintf(buf, "%s%s%s", ANGBAND_DIR_DATA, PATH_SEP, "scores.raw"); 

#ifdef MACINTOSH
    /* Global -- "Data file" */
    _ftype = 'DATA';
#endif

    /* Open (or create) a BINARY file, for reading/writing */
    fd = my_topen(buf, O_RDWR | O_CREAT | O_BINARY, mode);

    /* Save the fd */
    highscore_fd = fd;

    /* Check for success */
    if (fd >= 0) return (0);


    /* Warning message */
    plog_fmt("cannot create the '%s' score file", buf);

    /* No "scores", but yes "news" */
    if (!my_tfopen(ANGBAND_NEWS, "r")) {

	/* Warning message */
	plog_fmt("cannot access the '%s' news file", ANGBAND_NEWS);
    }

    /* Abort */
    quit("fatal error attempting to access the Angband 'lib' directory");

    /* Failure */
    return (1);
}


/*
 * Close the highscore file
 */
static errr highscore_close()
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
static errr highscore_lock(void)
{
    int oops;

    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (1);
    
#if !defined(MACINTOSH) && !defined(MSDOS) && !defined(AMIGA) && \
    !defined(_Windows) && !defined(__EMX__)

/* First, get a lock on the high score file so no-one else tries */
/* to write to it while we are using it */
/* added sys_v call to lockf - cba */

#ifdef USG
    oops = (lockf(highscore_fd, F_LOCK, 0) != 0);
#else
    oops = (0 != flock(highscore_fd, LOCK_EX));
#endif

    /* Trouble */
    if (oops) return (1);

#endif

    /* Success */
    return (0);
}


/*
 * Unlock the highscore file
 */
static errr highscore_unlock()
{
    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (1);
    
#if !defined(MACINTOSH) && !defined(MSDOS) && !defined(AMIGA) && \
    !defined(_Windows) && !defined(__EMX__)

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
    long p = (long)(i) * sizeof(high_score);
    
    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (1);
    
    /* Seek for the requested record */
    if (lseek(highscore_fd, p, L_SET) < 0) return (2);

    /* Success */
    return (0);
}


#if 0

/*
 * Chop all scores from 'i' on from the highscore file
 */
static errr highscore_chop(int i)
{
    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (1);
    
#if defined(sun) || defined(ultrix) || defined(NeXT)
    ftruncate(highscore_fd, i * sizeof(high_score));
#endif

    /* Success */
    return (0);
}

#endif


/*
 * Read one score from the highscore file
 */
static errr highscore_read(high_score *score)
{
    int num;

    /* Paranoia -- it may not have opened */
    if (highscore_fd < 0) return (1);
    
    /* Read a record, note failure */
    num = read(highscore_fd, (char*)(score), sizeof(high_score));

    /* Nothing read, means end of file */
    if (!num) return (1);

    /* Partial read, means major error */
    if (num != sizeof(high_score)) return (-1);

    /* Success */
    return (0);
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(high_score *score)
{
    int num;
    
    /* Paranoia -- it may not have been opened */
    if (highscore_fd < 0) return (1);
    
    /* Write the record */
    num = write(highscore_fd, (char*)(score), sizeof(high_score));

    /* Fail */
    if (num != sizeof(high_score)) return (-1);
    
    /* Success */
    return (0);
}



/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static int highscore_where(high_score *score)
{
    int			i;
    high_score		the_score;

    /* Go to the start of the highscore file */
    if (highscore_seek(0)) return (-1);

    /* Read until we get to a higher score */
    for (i = 0; i < MAX_SAVE_HISCORES; i++) {
	if (highscore_read(&the_score)) return (i);
	if (strcmp(the_score.pts, score->pts) < 0) return (i);
    }

    /* The "last" entry is always usable */
    return (MAX_SAVE_HISCORES - 1);
}

 
/*
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(high_score *score)
{
    int			i, slot;
    bool		done = FALSE;
	
    high_score		the_score, tmpscore;


    /* Determine where the score should go */
    slot = highscore_where(score);

    /* Hack -- Not on the list */
    if (slot < 0) return (-1);
        
    /* Hack -- prepare to dump the new score */
    the_score = (*score);
    
    /* Slide all the scores down one */
    for (i = slot; !done && (i < MAX_SAVE_HISCORES); i++) {

	/* Read the old guy, note errors */
	if (highscore_seek(i)) return (-1);
	if (highscore_read(&tmpscore)) done = TRUE;
	
	/* Back up and dump the score we were holding */
	if (highscore_seek(i)) return (-1);
	if (highscore_write(&the_score)) return (-1);

	/* Hack -- Save the old score, for the next pass */
	the_score = tmpscore;
    }

    /* Return location used */
    return (slot);
}

 



/*
 * Note that this function is called BEFORE "Term_init()"
 *
 * Open the score file while we still have the setuid privileges.
 * Later when the score is being written out, you must be sure
 * to flock the file so we don't have multiple people trying to
 * write to it at the same time.
 *
 * Notice that a failure to open the high score file often indicates
 * incorrect directory structure or starting directory or permissions.
 *
 * Note that a LOT of functions in this file assume that this
 * function call will succeed, so if not, we quit.
 */
void init_scorefile()
{
    /* Open the scorefile */
    (void)highscore_open();
}


/*
 * Shut down the scorefile
 */
void nuke_scorefile()
{
    /* Close the high score file */
    (void)(highscore_close());
}





/*
 * Hack -- Check the hours file vs. what time it is	-Doc
 */
void read_times(void)
{

#ifdef CHECK_HOURS

    register int i;
    vtype        in_line, buf;
    FILE        *file1;

/* Attempt to read hours.dat.	 If it does not exist,	   */
/* inform the user so he can tell the wizard about it	 */

    file1 = my_tfopen(ANGBAND_HOURS, "r");
    if (file1) {
	while (fgets(in_line, 80, file1)) {
	    if (strlen(in_line) > 3) {
		if (!strncmp(in_line, "SUN:", 4))
		    (void)strcpy(days[0], in_line);
		else if (!strncmp(in_line, "MON:", 4))
		    (void)strcpy(days[1], in_line);
		else if (!strncmp(in_line, "TUE:", 4))
		    (void)strcpy(days[2], in_line);
		else if (!strncmp(in_line, "WED:", 4))
		    (void)strcpy(days[3], in_line);
		else if (!strncmp(in_line, "THU:", 4))
		    (void)strcpy(days[4], in_line);
		else if (!strncmp(in_line, "FRI:", 4))
		    (void)strcpy(days[5], in_line);
		else if (!strncmp(in_line, "SAT:", 4))
		    (void)strcpy(days[6], in_line);
	    }
	}
	(void)fclose(file1);
    }

    else {
	sprintf(buf,"There is no hours file \"%s\".\n", ANGBAND_HOURS);
	message(buf, 0);
	message(NULL,0);
	exit_game();
    }

/* Check the hours, if closed	then exit. */
    if (!check_time()) {
	file1 = my_tfopen(ANGBAND_HOURS, "r");
	if (file1) {
	    clear_screen();
	    for (i = 0; fgets(in_line, 80, file1); i++) {
		put_str(in_line, i, 0);
	    }
	    (void)fclose(file1);
	    pause_line(23);
	}
	exit_game();
    }
#endif				   /* CHECK_HOURS */

}


/*
 * Attempt to open, and display, the intro "news" file		-RAK-
 */
void show_news(void)
{
    register int i;
    vtype        in_line, buf;
    FILE        *file1;


    /* Try to open the News file */
    file1 = my_tfopen(ANGBAND_NEWS, "r");

    /* Error? */
    if (!file1) {

	sprintf(buf, "Cannot read '%s'!\n", ANGBAND_NEWS);
	message(buf, 0);
	message(NULL,0);

	quit("cannot open 'news' file");
    }

    /* Clear the screen */    
    clear_screen();

    /* Dump the file (nuking newlines) */
    for (i = 0; (i < 24) && fgets(in_line, 80, file1); i++) {
	in_line[strlen(in_line)-1]=0;
	put_str(in_line, i, 0);
    }

    /* Flush it */
    Term_fresh();

    /* Close */
    (void)fclose(file1);
}


/*
 * File perusal.	    -CJS- primitive, but portable 
 */
void helpfile(cptr filename)
{
    bigvtype tmp_str;
    FILE    *file;
    char     input;
    int      i;

    file = my_tfopen(filename, "r");
    if (!file) {
	(void)sprintf(tmp_str, "Can not find help file \"%s\".\n", filename);
	message(tmp_str, 0);
	return;
    }

    save_screen();

    while (!feof(file))
    {
	clear_screen();
	for (i = 0; i < 23; i++) {
	    if (fgets(tmp_str, BIGVTYPESIZ - 1, file)) {
		tmp_str[strlen(tmp_str)-1] = '\0';
		put_str(tmp_str, i, 0);
	    }
	}

	prt("[Press any key to continue.]", 23, 23);
	input = inkey();
	if (input == ESCAPE) break;
    }

    (void)fclose(file);

    restore_screen();
}





/*
 * Print the character to a file or device.
 */
int file_character(cptr filename1)
{
    register int		i;
    int				j;
    int				fd = -1;
    inven_type			*i_ptr;
    cptr			p;
    cptr			colon = ":";
    cptr			blank = " ";

    register FILE		*file1;

    int			tmp;
    int                 xbth, xbthb, xfos, xsrh;
    int			xstl, xdis, xsave, xdev;
    char                xinfra[32];

    int			show_tohit, show_todam;

    vtype			out_val, prt1;
    bigvtype			prt2;


#ifdef MACINTOSH

    /* Global file type */
    _ftype = 'TEXT';
    
    /* Open the file (already verified by mac_file_character) */
    file1 = fopen(filename1, "w");

#else

    fd = my_topen(filename1, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0 && errno == EEXIST) {
	(void)sprintf(out_val, "Replace existing file %s?", filename1);
	if (get_check(out_val)) {
	    fd = my_topen(filename1, O_WRONLY, 0644);
	}
    }
    if (fd >= 0) {
	/* on some non-unix machines, fdopen() is not reliable, */
	/* hence must call close() and then fopen()  */
	(void)close(fd);
	file1 = my_tfopen(filename1, "w");
    }
    else {
	file1 = NULL;
    }

#endif


    /* XXX XXX XXX Skill with current weapon */
    tmp = p_ptr->ptohit + inventory[INVEN_WIELD].tohit;
    xbth = p_ptr->bth + tmp * BTH_PLUS_ADJ +
	   (class_level_adj[p_ptr->pclass][CLA_BTH] * p_ptr->lev);

    /* XXX XXX XXX Skill with current bow */
    tmp = p_ptr->ptohit + inventory[INVEN_BOW].tohit;
    xbthb = p_ptr->bthb + tmp * BTH_PLUS_ADJ +
	    (class_level_adj[p_ptr->pclass][CLA_BTHB] * p_ptr->lev);

    /* Basic abilities */
    xfos = 40 - p_ptr->fos;
    if (xfos < 0) xfos = 0;
    xsrh = p_ptr->srh;
    xstl = p_ptr->stl + 1;
    xdis = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT) +
	   (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
    xsave = p_ptr->save + stat_adj(A_WIS) +
	    (class_level_adj[p_ptr->pclass][CLA_SAVE] * p_ptr->lev / 3);
    xdev = p_ptr->save + stat_adj(A_INT) +
	   (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3);

    /* Infravision string */
    (void)sprintf(xinfra, "%d feet", p_ptr->see_infra * 10);

    /* Basic to hit/dam bonuses */
    show_tohit = p_ptr->dis_th;
    show_todam = p_ptr->dis_td;

    /* Check the weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Hack -- add in weapon info if known */
    if (known2_p(i_ptr)) show_tohit += i_ptr->tohit;
    if (known2_p(i_ptr)) show_tohit += i_ptr->todam;


    /* Dump a character sheet */
    if (file1) {

	prt("Writing character sheet...", 0, 0);
	Term_fresh();

	colon = ":";
	blank = " ";

	(void)fprintf(file1, " Name%9s %-23s", colon, player_name);
	(void)fprintf(file1, "Age%11s %6d ", colon, (int)p_ptr->age);
	cnv_stat(p_ptr->use_stat[A_STR], prt1);
	(void)fprintf(file1, "   STR : %s\n", prt1);
	(void)fprintf(file1, " Race%9s %-23s", colon, race[p_ptr->prace].trace);
	(void)fprintf(file1, "Height%8s %6d ", colon, (int)p_ptr->ht);
	cnv_stat(p_ptr->use_stat[A_INT], prt1);
	(void)fprintf(file1, "   INT : %s\n", prt1);
	(void)fprintf(file1, " Sex%10s %-23s", colon,
		      (p_ptr->male ? "Male" : "Female"));
	(void)fprintf(file1, "Weight%8s %6d ", colon, (int)p_ptr->wt);
	cnv_stat(p_ptr->use_stat[A_WIS], prt1);
	(void)fprintf(file1, "   WIS : %s\n", prt1);
	(void)fprintf(file1, " Class%8s %-23s", colon,
		      class[p_ptr->pclass].title);
	(void)fprintf(file1, "Social Class : %6d ", p_ptr->sc);
	cnv_stat(p_ptr->use_stat[A_DEX], prt1);
	(void)fprintf(file1, "   DEX : %s\n", prt1);
	(void)fprintf(file1, " Title%8s %-23s", colon, title_string());
	(void)fprintf(file1, "%22s", blank);
	cnv_stat(p_ptr->use_stat[A_CON], prt1);
	(void)fprintf(file1, "   CON : %s\n", prt1);
	(void)fprintf(file1, "%34s", blank);
	(void)fprintf(file1, "%26s", blank);
	cnv_stat(p_ptr->use_stat[A_CHR], prt1);
	(void)fprintf(file1, "   CHR : %s\n\n", prt1);

	(void)fprintf(file1, " + To Hit    : %6d", show_tohit);
	(void)fprintf(file1, "%7sLevel      :%9d", blank, (int)p_ptr->lev);
	(void)fprintf(file1, "   Max Hit Points : %6d\n", p_ptr->mhp);
	(void)fprintf(file1, " + To Damage : %6d", show_todam);
	(void)fprintf(file1, "%7sExperience :%9ld", blank, (long)p_ptr->exp);
	(void)fprintf(file1, "   Cur Hit Points : %6d\n", p_ptr->chp);
	(void)fprintf(file1, " + To AC     : %6d", p_ptr->dis_tac);
	(void)fprintf(file1, "%7sMax Exp    :%9ld", blank, (long)p_ptr->max_exp);
	(void)fprintf(file1, "   Max Mana%8s %6d\n", colon, p_ptr->mana);
	(void)fprintf(file1, "   Total AC  : %6d", p_ptr->dis_ac);
	
	if (p_ptr->lev >= MAX_PLAYER_LEVEL) {
	    (void)fprintf(file1, "%7sExp to Adv.:%9s", blank, "****");
	}
	else {
	    (void)fprintf(file1, "%7sExp to Adv.:%9ld", blank,
			  (long) (player_exp[p_ptr->lev - 1] *
				   p_ptr->expfact / 100L));
	}

	(void)fprintf(file1, "   Cur Mana%8s %6d\n", colon, p_ptr->cmana);
	(void)fprintf(file1, "%28sGold%8s%9ld\n", blank, colon, (long)p_ptr->au);

	(void)fprintf(file1, "\n(Miscellaneous Abilities)\n");
	(void)fprintf(file1, " Fighting    : %-10s", likert(xbth, 12));
	(void)fprintf(file1, "   Stealth     : %-10s", likert(xstl, 1));
	(void)fprintf(file1, "   Perception  : %s\n", likert(xfos, 3));
	(void)fprintf(file1, " Bows/Throw  : %-10s", likert(xbthb, 12));
	(void)fprintf(file1, "   Disarming   : %-10s", likert(xdis, 8));
	(void)fprintf(file1, "   Searching   : %s\n", likert(xsrh, 6));
	(void)fprintf(file1, " Saving Throw: %-10s", likert(xsave, 6));
	(void)fprintf(file1, "   Magic Device: %-10s", likert(xdev, 6));
	(void)fprintf(file1, "   Infra-Vision: %s\n\n", xinfra);

	/* Write out the character's history     */
	(void)fprintf(file1, "Character Background\n");
	for (i = 0; i < 4; i++) {
	    (void)fprintf(file1, " %s\n", history[i]);
	}

	/* Write out the equipment list.	     */
	(void)fprintf(file1, "\n  [Character's Equipment List]\n\n");
	if (!equip_ctr) {
	    (void)fprintf(file1, "  Character has no equipment in use.\n");
	}
	else {
	    for (j = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		i_ptr = &inventory[i];
		if (i_ptr->tval) {
		    p = mention_use(i);
		    objdes(prt2, &inventory[i], TRUE);
		    (void)fprintf(file1, "  %c) %-19s: %s\n", j + 'a', p, prt2);
		    j++;
		}
	    }
	}

	/* Write out the character's inventory.	     */
	(void)fprintf(file1, "\n\n");
	(void)fprintf(file1, "  [General Inventory List]\n\n");
	if (!inven_ctr) {
	    (void)fprintf(file1, "  Character has no objects in inventory.\n");
	}
	else {
	    for (i = 0; i < inven_ctr; i++) {
		objdes(prt2, &inventory[i], TRUE);
		(void)fprintf(file1, "%c) %s\n", i + 'a', prt2);
	    }
	}
	(void)fprintf(file1, "\n\n");

	fprintf(file1, "  [%s%s Home Inventory]\n\n", player_name,
		(toupper(player_name[strlen(player_name)-1]) == 'S' ? "'" : "'s"));
	if (store[MAX_STORES-1].store_ctr == 0) {
	    (void) fprintf(file1, "  Character has no objects at home.\n");
	}
	else {
	    for (i = 0; i < store[MAX_STORES-1].store_ctr; i++) {
		if (i == 12) fprintf(file1, "\n");  
		objdes_store(prt2, &store[MAX_STORES-1].store_item[i], TRUE);
		(void) fprintf(file1, "%c) %s\n", (i%12)+'a', prt2);
	    }
	}
	(void) fprintf(file1, "\n");

	(void)fclose(file1);
	message("Completed.", 0);
	return TRUE;
    }

    else {

	(void)sprintf(out_val, "Can't open file: %s", filename1);
	message(out_val, 0);
	return FALSE;
    }
}




/*
 * Hack -- Calculates the total number of points earned		-JWT-	 
 */
long total_points(void)
{
    return (p_ptr->max_exp + (100 * p_ptr->max_dlv));
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
 * Save a "bones" file for a dead character
 * Should probably attempt some form of locking...
 */
static void make_bones(void)
{
    FILE                *fp;

    char                str[1024];


    /* Dead non-wizards create a bones file */
    if (death && !wizard) {

	/* Ignore people who die in town */
	if (dun_level > 0) {

	    /* XXX Perhaps the player's level should be taken into account */

	    /* Get the proper "Bones File" name */
	    sprintf(str, "%s%s%d", ANGBAND_DIR_BONES, PATH_SEP, dun_level);

	    /* Attempt to open the bones file */
	    fp = my_tfopen(str, "r");

	    /* Do not over-write a previous ghost */
	    if (fp) return;

#ifdef MACINTOSH
	    _ftype = 'TEXT';
#endif

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
    char                 day[32];
    cptr		 p;
    vtype                str, tmp_str;

    time_t ct = time((time_t)0);


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
    sprintf(day, "%-.24s", ctime(&ct));
    (void)sprintf(str, "| %s |", center_string(tmp_str, day));
    put_str(str, 17, 9);
    put_str("*|   *     *     *    *   *     *  | *", 18, 8);
    put_str("________)/\\\\_)_/___(\\/___(//_\\)/_\\//__\\\\(/_|_)_______",
	       19, 0);
}


/*
 * Display some character info
 */
static void show_info(void)
{
    int i, j, k;
    inven_type *i_ptr;
    store_type *st_ptr;
    vtype t1, t2, str;

    /* Flush all input keys */
    flush();

    put_str("(ESC to abort, return to print on screen)", 23, 0);
    put_str("Character record?", 22, 0);

    if (!get_string(str, 22, 18, 60)) return;


    /* Know everything the player is wearing/carrying */
    for (i = 0; i < INVEN_TOTAL; i++) {
	i_ptr = &inventory[i];
	if (i_ptr && i_ptr->tval) {
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
	show_equip(INVEN_WIELD, INVEN_TOTAL-1);
	msg_print(NULL);
    }

    /* Inventory -- no assumptions */
    if (inven_ctr) {
	clear_screen();
	msg_print("You are carrying:");
	show_inven(0, inven_ctr - 1);
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
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
static void display_scores_aux(int from, int to, int note, high_score *score)
{
    int i, j, k, n, attr, place;

    high_score  the_score;

    char         buf[256];

    vtype        tmp_str;


    /* Assume we will show the first 10 */
    if (from < 0) from = 0;
    if (to < 0) to = 10;
    if (to > MAX_SAVE_HISCORES) to = MAX_SAVE_HISCORES;


    /* Seek to the beginning */
    if (highscore_seek(0)) return;

    /* Hack -- Count the high scores */
    for (i = 0; i < MAX_SAVE_HISCORES; i++) {
	if (highscore_read(&the_score)) break;
    }

    /* Hack -- allow "fake" entry to be last */
    if ((note == i) && score) i++;

    /* Forget about the last entries */
    if (i > to) i = to;


    /* Show 5 per page, until "done" */
    for (k = from, place = k+1; k < i; k += 5) {

	/* Clear those */
	clear_screen();

	put_str("                Angband Hall of Fame", 0, 0);

	/* Indicate non-top scores */
	if (k > 0) {
	    sprintf(tmp_str, "(from position %d)", k + 1);
	    put_str(tmp_str, 0, 40);
	}

	/* Dump 5 entries */
	for (j = k, n = 0; j < i && n < 5; place++, j++, n++) {

	    int pr, pc, clev, mlev, cdun, mdun;

	    cptr user, gold, when, aged;
	    

	    /* Hack -- indicate death in yellow */
	    attr = (j == note) ? TERM_YELLOW : TERM_WHITE;
	    

	    /* Mega-Hack -- insert a "fake" record */
	    if ((note == j) && score) {
		the_score = (*score);
		attr = TERM_L_GREEN;
		score = NULL;
		note = -1;
		j--;
	    }

	    /* Read a normal record */
	    else {
		/* Read the proper record */
		if (highscore_seek(j)) break;
		if (highscore_read(&the_score)) break;
	    }
	    
	    /* Extract the race/class */
	    pr = atoi(the_score.p_r);
	    pc = atoi(the_score.p_c);

	    /* Extract the level info */
	    clev = atoi(the_score.cur_lev);
	    mlev = atoi(the_score.max_lev);
	    cdun = atoi(the_score.cur_dun);
	    mdun = atoi(the_score.max_dun);

	    /* Hack -- extract the gold and such */
	    for (user = the_score.uid; isspace(*user); user++);
	    for (when = the_score.day; isspace(*when); when++);
	    for (gold = the_score.gold; isspace(*gold); gold++);
	    for (aged = the_score.turns; isspace(*aged); aged++);
	    
	    /* Dump some info */
	    (void)sprintf(buf, "%3d.%9s  %s the %s %s, Level %d",
			  place, the_score.pts, the_score.who,
			  race[pr].trace, class[pc].title,
			  clev);

	    /* Append a "maximum level" */
	    if (mlev > clev) strcat(buf, format(" (Max %d)", mlev));

	    /* Dump the first line */
	    c_put_str(attr, buf, n*4 + 2, 0);

	    /* Another line of info */
	    sprintf(buf, "               Killed by %s on %s %d",
		    the_score.how, "Dungeon Level", cdun);
	    
	    /* Hack -- some people die in the town */
	    if (!cdun) {
		sprintf(buf, "               Killed by %s in the Town",
			the_score.how);
	    }

	    /* Append a "maximum level" */
	    if (mdun > cdun) strcat(buf, format(" (Max %d)", mdun));

	    /* Dump the info */
	    c_put_str(attr, buf, n*4 + 3, 0);

	    /* And still another line of info */
	    sprintf(buf, "               (User %s, Date %s, Gold %s, Turn %s).",
		    user, when, gold, aged);
	    c_put_str(attr, buf, n*4 + 4, 0);
	}


	/* Wait for response. */
	prt("[Press ESC to quit, any other key to continue.]", 23, 17);
	j = inkey();
	erase_line(23, 0);

	/* Hack -- notice Escape */
	if (j == ESCAPE) break;
    }
}


/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 *
 * We may want to be slightly more "clean" on empty score lists.
 */
void display_scores(int from, int to)
{
    display_scores_aux(from, to, -1, NULL);
}



/*
 * Enters a players name on a hi-score table, if "legal".
 */
static errr top_twenty(void)
{
    int          j;

    high_score   the_score;

    time_t ct = time((time_t*)0);


    /* Wipe screen */
    clear_screen();

    /* Wizard-mode pre-empts scoring */
    if (noscore) {
	msg_print("Score not registered for wizards.");
	display_scores(0, 10);
	return (0);
    }

    /* Interupted */    
    if (!total_winner && !stricmp(died_from, "Interrupting")) {
	msg_print("Score not registered due to interruption.");
	display_scores(0, 10);
	return (0);
    }

    /* Quitter */
    if (!total_winner && !stricmp(died_from, "Quitting")) {
	msg_print("Score not registered due to quitting.");
	display_scores(0, 10);
	return (0);
    }


    /* Clear the record */
    WIPE(&the_score, high_score);
    
    /* Save the version */
    sprintf(the_score.what, "%u.%u.%u",
	    CUR_VERSION_MAJ, CUR_VERSION_MIN, CUR_PATCH_LEVEL);
    
    /* Calculate and save the points */
    sprintf(the_score.pts, "%9lu", (long)total_points());

    /* Save the current gold */
    sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
    
    /* Save the current turn */
    sprintf(the_score.turns, "%9lu", (long)turn);
    
#ifdef HIGHSCORE_DATE_HACK
    /* Save the date in a hacked up form (9 chars) */
    sprintf(the_score.day, "%-.6s %-.2s", ctime(&ct) + 4, ctime(&ct) + 22);
#else
    /* Save the date in standard form (8 chars) */
    strftime(the_score.day, 9, "%m/%d/%y", localtime(&ct));
#endif

    /* Save the player name (15 chars) */
    sprintf(the_score.who, "%-.15s", player_name);

    /* Save the player info */
    sprintf(the_score.uid, "%7u", player_uid);
    sprintf(the_score.sex, "%c", (p_ptr->male ? 'm' : 'f'));
    sprintf(the_score.p_r, "%2d", p_ptr->prace);
    sprintf(the_score.p_c, "%2d", p_ptr->pclass);

    /* Save the level and such */
    sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
    sprintf(the_score.cur_dun, "%3d", dun_level);
    sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
    sprintf(the_score.max_dun, "%3d", p_ptr->max_dlv);

    /* Save the cause of death (31 chars) */
    sprintf(the_score.how, "%-.31s", died_from);


    /* Ignore "suspend" signal */
    signals_ignore_tstp();

    /* Lock the highscore file, or fail */
    if (highscore_lock()) return (1);

    /* Add a new entry to the score list, see where it went */
    j = highscore_add(&the_score);
    
    /* Unlock the highscore file */
    highscore_unlock();


    /* Hack -- Display the top fifteen scores */
    if (j < 10) {
	display_scores_aux(0, 15, j, NULL);
    }
    
    /* Display the scores surrounding the player */
    else {
	display_scores_aux(0, 5, j, NULL);
	display_scores_aux(j - 2, j + 7, j, NULL);
    }
    

    /* Success */
    return (0);
}


/*
 * Predict the players location, and display it.
 */
static errr predict_score(void)
{
    int          j;

    high_score   the_score;


    /* Save the version */
    sprintf(the_score.what, "%u.%u.%u",
	    CUR_VERSION_MAJ, CUR_VERSION_MIN, CUR_PATCH_LEVEL);
    
    /* Calculate and save the points */
    sprintf(the_score.pts, "%9lu", (long)total_points());

    /* Save the current gold */
    sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
    
    /* Save the current turn */
    sprintf(the_score.turns, "%9lu", (long)turn);
    
    /* Hack -- no time needed */
    strcpy(the_score.day, "TODAY");

    /* Save the player name (15 chars) */
    sprintf(the_score.who, "%-.15s", player_name);

    /* Save the player info */
    sprintf(the_score.uid, "%7u", player_uid);
    sprintf(the_score.sex, "%c", (p_ptr->male ? 'm' : 'f'));
    sprintf(the_score.p_r, "%2d", p_ptr->prace);
    sprintf(the_score.p_c, "%2d", p_ptr->pclass);

    /* Save the level and such */
    sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
    sprintf(the_score.cur_dun, "%3d", dun_level);
    sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
    sprintf(the_score.max_dun, "%3d", p_ptr->max_dlv);

    /* Hack -- no cause of death */
    strcpy(the_score.how, "nobody (yet!)");


    /* See where the entry would be placed */
    j = highscore_where(&the_score);
    

    /* Hack -- Display the top fifteen scores */
    if (j < 10) {
	display_scores_aux(0, 15, j, &the_score);
    }
    
    /* Display some "useful" scores */
    else {
	display_scores_aux(0, 5, -1, NULL);
	display_scores_aux(j - 2, j + 7, j, &the_score);
    }

    
    /* Success */
    return (0);
}








/*
 * Change the player into a King!			-RAK-	 
 */
static void kingly()
{
    register cptr p;

    /* Hack -- retire in town */
    dun_level = 0;

    /* Change the character attributes.		 */
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
 *
 * Note -- the player does not have to be dead.
 */
void exit_game(void)
{
    /* Flush the messages */
    msg_print(NULL);

    /* Flush the input */
    flush();

    /* No suspending now */
    signals_ignore_tstp();


    /* XXX This may never happen */
    if (!turn) {
	display_scores(0, 10);
	erase_line(23, 0);
	quit(NULL);
    }


    /* Handle death */
    if (death) {

	/* Handle retirement */
	if (total_winner) kingly();

	/* Dump bones file */
	make_bones();

	/* You are dead */
	print_tomb();

	/* Show more info */
	show_info();
    
	/* Handle score, show Top scores */
	top_twenty();

	/* Save the player or his memories */
	(void)save_player();
    }

    /* Still alive */
    else {

	/* Begin the save */
	msg_print("Saving game...");

	/* Not dead yet */
	(void)strcpy(died_from, "(saved)");

	/* Save the player. */
	(void)save_player();

	/* Predict the player's score (allow "escape") */
	msg_flag = FALSE;
	prt("Saving game... done.  Press Return. ", 0, 0);
	if (inkey() != ESCAPE) predict_score();	
    }
    

    /* Actually stop the process */
    quit(NULL);
}


/* 
 * Handle abrupt death of the visual system
 * This routine is called only in very rare situations
 */
void exit_game_panic(void)
{
    /* If nothing important has happened, just quit */
    if (!character_generated || character_saved) quit("end of input");

    /* Hack -- prevent infinite loops */
    msg_flag = FALSE;
    
    /* Hack -- turn off some things */
    disturb(1, 0);

    /* Panic save */
    panic_save = 1;

    /* XXX XXX Hack -- clear the death flag when creating a HANGUP */
    /* save file so that player can see tombstone when restart. */

    /* Hack -- Not dead yet */
    death = FALSE;
    
    /* Panic save */
    (void)strcpy(died_from, "(end of input: panic saved)");
    if (save_player()) quit("end of input: panic save succeeded");

    /* Unsuccessful panic save.  Oh well. */
    quit("end of input: panic save failed!");
}


