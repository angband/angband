/*
 * death.c: code executed when player dies 
 *
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

/*
 * some incorrectly define NULL as integer constant, so load this before
 * local includes 
 */

#include <stdio.h>
#include <signal.h>
#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#ifdef Pyramid
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <ctype.h>
#include "config.h"

#ifndef USG
/* only needed for Berkeley UNIX */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
#else
#ifdef __MINT__        
#include <sys/types.h>
#endif
#include <unistd.h>
#endif

#ifdef MSDOS
#include <io.h>
#else
#if !defined(ATARIST_MWC) && !defined(MAC)
#ifndef VMS
#include <pwd.h>
#else
#include <file.h>
#endif
#endif
#endif

#ifdef linux
#include <sys/file.h>
#endif

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#include <fcntl.h>
#endif
#else
#include <strings.h>
#endif

#if defined(USG) || defined(VMS)
#ifndef L_SET
#define L_SET 0
#endif
#endif

#ifndef VMS
#ifndef MAC
#if defined(ultrix) || defined(USG)
void                perror();
void                exit();

#endif
#endif
#endif

#ifndef MAC
#ifdef SYS_V
struct passwd      *getpwuid();

#endif
#endif

/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__
static void  date(char *);
static char *center_string(char *, const char *);
static void  print_tomb(void);
static void  kingly(void);

#else
static void  date();
static char *center_string();
static void  print_tomb();
static void  kingly();

#endif
#endif

#ifndef MAC
char        *getlogin();
#endif

#if !defined(time_t)
#define time_t long
#endif

static void 
date(day)
char *day;
{
    register char *tmp;
    time_t         c;

    c = time((time_t *)0);
    tmp = ctime(&c);
    tmp[10] = '\0';
    (void)strcpy(day, tmp);
}

/* Centers a string within a 31 character string		-JWT-	 */
static char *
center_string(centered_str, in_str)
char       *centered_str;
const char *in_str;
{
    register int i, j;

    i = strlen(in_str);
    j = 15 - i / 2;
    (void)sprintf(centered_str, "%*s%s%*s", j, "", in_str, 31 - i - j, "");
    return centered_str;
}


/* Not touched for Mac port */
void 
display_scores(from, to)
int from, to;
{
    register int i = 0, j, k, l;
    int          fd;
    high_scores  score;

/* MAX_SAVE_HISCORES scores, 2 lines per score */
    char         list[2 * MAX_SAVE_HISCORES][128];
    char         hugebuffer[10000];
    char         string[100];

    vtype        tmp_str;

    if (to < 0)
	to = 20;
    if (to > MAX_SAVE_HISCORES)
	to = MAX_SAVE_HISCORES;
#ifdef SET_UID
    if (1 > (fd = my_topen(ANGBAND_TOP, O_RDONLY | O_CREAT, 0644))) {
#else
    if (1 > (fd = my_topen(ANGBAND_TOP, O_RDONLY | O_CREAT, 0666))) {
#endif
	(void)sprintf(string, "Error opening score file \"%s\"\n", ANGBAND_TOP);
	prt(string, 0, 0);
	return;
    }
    while (0 < read(fd, (char *)&score, sizeof(high_scores))) {
	if (score.uid != -1 && getpwuid(score.uid) != NULL)
	    (void)sprintf(hugebuffer, "%3d) %-7ld %s the %s %s (Level %d), played by %s",
			  i / 2 + 1,
			  (long)score.points, score.name,
			  race[score.prace].trace, class[score.pclass].title,
			  (int)score.lev, getpwuid(score.uid)->pw_name);
	else
	    (void)sprintf(hugebuffer, "%3d) %-7ld %s the %s %s (Level %d)",
			  i / 2 + 1,
			  (long)score.points, score.name,
			  race[score.prace].trace, class[score.pclass].title,
			  (int)score.lev);
	strncpy(list[i], hugebuffer, 127);
	(void)sprintf(hugebuffer,
		      "             Killed by %s on Dungeon Level %d.",
		      score.died_from, score.dun_level);
	strncpy(list[i + 1], hugebuffer, 127);
	i += 2;
	if (i >= (MAX_SAVE_HISCORES * 2))
	    break;
    }

    signal(SIGTSTP, SIG_IGN);
    k = from * 2;
    do {
	if (k > 0) {
	    sprintf(tmp_str, "\t\tAngband Hall of Fame (from position %d)",
		    (k / 2) + 1);
	    put_buffer(tmp_str, 0, 0);
	} else {
	    put_buffer("\t\tAngband Hall of Fame                     ", 0, 0);
	}
	put_buffer("     Score", 1, 0);
	l = 0;
	for (j = k; j < i && j < (to * 2) && j < (k + 20); j++, l++)
	    put_buffer(list[j], l + 2, 0);
	k += 20;
	if (!look_line(23)) {
	/* What happens upon dying.				-RAK-	 */
	    msg_print(NULL);
	    clear_screen();
	    flush();		   /* flush all input */
	    nosignals();	   /* Can't interrupt or suspend. */
	    (void)save_char();	   /* Save the memory at least. */
	    restore_term();
	    exit(0);
	}
	clear_screen();
    } while (k < (to * 2) && k < i);
}

/* Pauses for user response before returning		-RAK-	 */
int 
look_line(prt_line)
int prt_line;
{
    prt("[Press ESC to quit, any other key to continue.]", prt_line, 17);
    if (inkey() == ESCAPE) {
	erase_line(prt_line, 0);
	return 0;
    } else {
	erase_line(prt_line, 0);
	return 1;
    }
}



/* Prints the gravestone of the character		-RAK-	 */
static void 
print_tomb()
{
    vtype                str, tmp_str;
    register int         i;
    char                 day[11];
    register const char *p;
    FILE                *fp;

    if (stricmp(died_from, "Interrupting") && !wizard) {
	sprintf(str, "%s/%d", ANGBAND_BONES, dun_level);
	if ((fp = my_tfopen(str, "r")) == NULL && (dun_level > 1)) {
	    if ((fp = my_tfopen(str, "w")) != NULL) {
#ifndef __MINT__
#ifdef SET_UID
		(void)fchmod(fileno(fp), 0644);
#else
		(void)fchmod(fileno(fp), 0666);
#endif
#endif
		fprintf(fp, "%s\n%d\n%d\n%d",
		  py.misc.name, py.misc.mhp, py.misc.prace, py.misc.pclass);
		if (fp) fclose(fp);
	    }
	} else {
	    if (fp) fclose(fp);
	}
    }
    clear_screen();
    put_buffer("_______________________", 1, 15);
    put_buffer("/", 2, 14);
    put_buffer("\\         ___", 2, 38);
    put_buffer("/", 3, 13);
    put_buffer("\\ ___   /   \\      ___", 3, 39);
    put_buffer("/            RIP            \\   \\  :   :     /   \\", 4, 12);
    put_buffer("/", 5, 11);
    put_buffer("\\  : _;,,,;_    :   :", 5, 41);
    (void)sprintf(str, "/%s\\,;_          _;,,,;_",
		  center_string(tmp_str, py.misc.name));
    put_buffer(str, 6, 10);
    put_buffer("|               the               |   ___", 7, 9);
    if (!total_winner)
	p = title_string();
    else
	p = "Magnificent";
    (void)sprintf(str, "| %s |  /   \\", center_string(tmp_str, p));
    put_buffer(str, 8, 9);
    put_buffer("|", 9, 9);
    put_buffer("|  :   :", 9, 43);
    if (!total_winner)
	p = class[py.misc.pclass].title;
    else if (py.misc.male)
	p = "*King*";
    else
	p = "*Queen*";
    (void)sprintf(str, "| %s | _;,,,;_   ____", center_string(tmp_str, p));
    put_buffer(str, 10, 9);
    (void)sprintf(str, "Level : %d", (int)py.misc.lev);
    (void)sprintf(str, "| %s |          /    \\", center_string(tmp_str, str));
    put_buffer(str, 11, 9);
    (void)sprintf(str, "%ld Exp", (long)py.misc.exp);
    (void)sprintf(str, "| %s |          :    :", center_string(tmp_str, str));
    put_buffer(str, 12, 9);
    (void)sprintf(str, "%ld Au", (long)py.misc.au);
    (void)sprintf(str, "| %s |          :    :", center_string(tmp_str, str));
    put_buffer(str, 13, 9);
    (void)sprintf(str, "Died on Level : %d", dun_level);
    (void)sprintf(str, "| %s |         _;,,,,;_", center_string(tmp_str, str));
    put_buffer(str, 14, 9);
    put_buffer("|            killed by            |", 15, 9);
    p = died_from;
    i = strlen(p);
    died_from[i] = '.';			   /* add a trailing period */
    died_from[i + 1] = '\0';
    (void)sprintf(str, "| %s |", center_string(tmp_str, p));
    put_buffer(str, 16, 9);
    died_from[i] = '\0';		   /* strip off the period */
    date(day);
    (void)sprintf(str, "| %s |", center_string(tmp_str, day));
    put_buffer(str, 17, 9);
    put_buffer("*|   *     *     *    *   *     *  | *", 18, 8);
    put_buffer("________)/\\\\_)_/___(\\/___(//_\\)/_\\//__\\\\(/_|_)_______",
	       19, 0);

    flush();
    put_buffer("(ESC to abort, return to print on screen, or file name)", 23, 0);
    put_buffer("Character record?", 22, 0);
    if (get_string(str, 22, 18, 60)) {
	for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	    inven_type *i_ptr = &inventory[i];
	    if (i_ptr && i_ptr->tval != TV_NOTHING) {
		known1(i_ptr);
		known2(i_ptr);
	    }
	}
	calc_bonuses();
	clear_screen();
	display_char();
	put_buffer("Type ESC to skip the inventory:", 23, 0);
	if (inkey() != ESCAPE) {
	    clear_screen();
	    msg_print("You are using:");
	    (void)show_equip(TRUE, 0);
	    msg_print(NULL);
	    if (inven_ctr) {
		msg_print("You are carrying:");
		clear_from(1);
		(void)show_inven(0, inven_ctr - 1, TRUE, 0, 0);
		msg_print(NULL);
	    }
          msg_print ("You have stored at your house:");
          clear_from (1);
	{ /* show home's inventory... */
            store_type *s_ptr = &store[7]; /* home */
            int ii = 0, j = 0;
            vtype t1, t2;
            
            while ( ii <s_ptr->store_ctr) {
              j = 0;
              sprintf(t2, "(page %d)", (ii==0?1:2));
              prt(t2, 1, 3);
              while ((ii<s_ptr->store_ctr) && (j<12)){
                known1(&s_ptr->store_inven[ii].sitem);
                known2(&s_ptr->store_inven[ii].sitem);
                objdes(t1, &s_ptr->store_inven[ii].sitem, TRUE);
                sprintf(t2, "%c) %s", 'a'+j, t1);
                prt(t2, j+2, 4); 
                j++;
                ii++;
	    } /* items 1-12, 13-24 loop */
              if (ii < s_ptr->store_ctr) { /* if we're done, skip this */
                msg_print(NULL);
                msg_print("Home inventory:");
                clear_from (1);
	    }
	  } /* outer while loop */
            msg_print(NULL);
	} /* scope block of display-home inventory code -CFT */

	}
    }
}


/* Calculates the total number of points earned		-JWT-	 */
long 
total_points()
{
    return (py.misc.max_exp + (100 * py.misc.max_dlv));
}



/* Enters a players name on a hi-score table...    SM */
static int 
top_twenty()
{
    register int        i, j, k;
    high_scores         scores[MAX_SAVE_HISCORES], myscore;

    clear_screen();

    if (wizard || to_be_wizard) {
	display_scores(0, 10);
	(void)save_char();
	restore_term();
	exit(0);
    }
    if (!total_winner && !stricmp(died_from, "Interrupting")) {
	msg_print("Score not registered due to interruption.");
	display_scores(0, 10);
	(void)save_char();
	restore_term();
	exit(0);
    }
    if (!total_winner && !stricmp(died_from, "Quitting")) {
	msg_print("Score not registered due to quitting.");
	display_scores(0, 10);
	(void)save_char();
	restore_term();
	exit(0);
    }
    myscore.points = total_points();
    myscore.dun_level = dun_level;
    myscore.lev = py.misc.lev;
    myscore.max_lev = py.misc.max_dlv;
    myscore.mhp = py.misc.mhp;
    myscore.chp = py.misc.chp;
    myscore.uid = player_uid;
/* First character of sex, lower case */
    myscore.sex = py.misc.male;
    myscore.prace = py.misc.prace;
    myscore.pclass = py.misc.pclass;
    (void)strcpy(myscore.name, py.misc.name);
    (void)strncpy(myscore.died_from, died_from, strlen(died_from));
    myscore.died_from[strlen(died_from)] = '\0';
/* Get rid of '.' at end of death description */

/* First, get a lock on the high score file so no-one else tries */
/* to write to it while we are using it */
/* added sys_v call to lockf - cba */
#ifdef USG
    if (lockf(highscore_fd, F_LOCK, 0) != 0)
#else
    if (0 != flock(highscore_fd, LOCK_EX))
#endif
    {
	perror("Error gaining lock for score file");
	exit_game();
    }
/* Check to see if this score is a high one and where it goes */
    i = 0;
#ifndef BSD4_3
    (void)lseek(highscore_fd, (long)0, L_SET);
#else
    (void)lseek(highscore_fd, (off_t) 0, L_SET);
#endif
    while ((i < MAX_SAVE_HISCORES)
    && (0 != read(highscore_fd, (char *)&scores[i], sizeof(high_scores)))) {
	i++;
    }

    j = 0;
    while (j < i && (scores[j].points >= myscore.points)) {
	j++;
    }
/* i is now how many scores we have, and j is where we put this score */

/* If its the first score, or it gets appended to the file */
    if (!i || (i == j && j < MAX_SAVE_HISCORES)) {
	(void)lseek(highscore_fd, (long)(j * sizeof(high_scores)), L_SET);
	(void)write(highscore_fd, (char *)&myscore, sizeof(high_scores));
    } else if (j < i) {
    /* If it gets inserted in the middle */
    /* Bump all the scores up one place */
	for (k = MY_MIN(i, (MAX_SAVE_HISCORES - 1)); k > j; k--) {
	    (void)lseek(highscore_fd, (long)(k * sizeof(high_scores)), L_SET);
	    (void)write(highscore_fd, (char *)&scores[k - 1], sizeof(high_scores));
	}
    /* Write out your score */
	(void)lseek(highscore_fd, (long)(j * sizeof(high_scores)), L_SET);
	(void)write(highscore_fd, (char *)&myscore, sizeof(high_scores));
    }
/* added usg lockf call - cba */
#ifdef USG
    lockf(highscore_fd, F_ULOCK, 0);
#else
    (void)flock(highscore_fd, LOCK_UN);
#endif
    (void)close(highscore_fd);
    if (j < 10) {
	display_scores(0, 10);
    } else if (j > (i - 5)) {
	display_scores(i - 9, i + 1);
    } else
	display_scores(j - 5, j + 5);
    return (0);
}

/* Enters a players name on the hi-score table     SM	 */
void 
delete_entry(which)
int which;
{
    register int i;
    high_scores  scores[MAX_SAVE_HISCORES];

/* added usg lockf call - cba */
#ifdef USG
    if (lockf(highscore_fd, F_LOCK, 0) != 0)
#else
    if (0 != flock(highscore_fd, LOCK_EX))
#endif
    {
	perror("Error gaining lock for score file");
	exit_game();
    }
/* Check to see if this score is a high one and where it goes */
    i = 0;
    (void)lseek(highscore_fd, (off_t) 0, L_SET);
    while ((i < MAX_SAVE_HISCORES) &&
	 (0 != read(highscore_fd, (char *)&scores[i], sizeof(high_scores))))
	i++;

    if (i >= which) {

#if defined(sun) || defined(ultrix) || defined(NeXT)
	ftruncate(highscore_fd, 0);
#endif

/* If its the first score, or it gets appended to the file */
	lseek(highscore_fd, 0, L_SET);
	if (which > 0)
	    write(highscore_fd, (char *)&scores[0],
		  (which - 1) * sizeof(high_scores));
	if (i > which)
	    write(highscore_fd, (char *)&scores[which],
		  (i - which) * sizeof(high_scores));
	} else {
	    puts(" The high score table does not have that many entries!");
	}

/* added usg lockf call - cba */
#ifdef USG
    lockf(highscore_fd, F_ULOCK, 0);
#else
    (void)flock(highscore_fd, LOCK_UN);
#endif
    (void)close(highscore_fd);
    if (which < 10) {
	display_scores(0, 10);
    } else if (which > (i - 10)) {
	display_scores(i - 10, i);
    } else
	display_scores(which - 5, which + 5);
}

/* Change the player into a King!			-RAK-	 */
static void 
kingly()
{
    register struct misc *p_ptr;
    register const char  *p;

/* Change the character attributes.		 */
    dun_level = 0;
    (void)strcpy(died_from, "Ripe Old Age");
    p_ptr = &py.misc;
    (void)restore_level();
    p_ptr->lev += MAX_PLAYER_LEVEL;
    p_ptr->au += 250000L;
    p_ptr->max_exp += 5000000L;
    p_ptr->exp = p_ptr->max_exp;

/* Let the player know that he did good.	 */
    clear_screen();
    put_buffer("#", 1, 34);
    put_buffer("#####", 2, 32);
    put_buffer("#", 3, 34);
    put_buffer(",,,  $$$  ,,,", 4, 28);
    put_buffer(",,=$   \"$$$$$\"   $=,,", 5, 24);
    put_buffer(",$$        $$$        $$,", 6, 22);
    put_buffer("*>         <*>         <*", 7, 22);
    put_buffer("$$         $$$         $$", 8, 22);
    put_buffer("\"$$        $$$        $$\"", 9, 22);
    put_buffer("\"$$       $$$       $$\"", 10, 23);
    p = "*#########*#########*";
    put_buffer(p, 11, 24);
    put_buffer(p, 12, 24);
    put_buffer("Veni, Vidi, Vici!", 15, 26);
    put_buffer("I came, I saw, I conquered!", 16, 21);
    if (p_ptr->male)
	put_buffer("All Hail the Mighty King!", 17, 22);
    else
	put_buffer("All Hail the Mighty Queen!", 17, 22);
    flush();
    pause_line(23);
}


/* Handles the gravestone end top-twenty routines	-RAK-	 */
void 
exit_game()
{
    register int        i;

#ifdef MAC
/* Prevent strange things from happening */
    enablefilemenu(FALSE);
#endif

/* What happens upon dying.				-RAK-	 */
    msg_print(NULL);
    flush();			   /* flush all input */
    nosignals();		   /* Can't interrupt or suspend. */
    if (turn >= 0) {
	if (total_winner)
	    kingly();
	print_tomb();
	if (!wizard && !to_be_wizard)
	    top_twenty();
	else
	    msg_print("Score not registered.");
    }
    i = log_index;
    (void)save_char();		   /* Save the memory at least. */
    if (i > 0)
	display_scores(0, 10);
    erase_line(23, 0);
    restore_term();
    exit(0);
}
