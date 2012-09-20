/* death.c: code executed when player dies

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

/* some incorrectly define NULL as integer constant, so load this before
   local includes */
#include <stdio.h>
#include <signal.h>
#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef Pyramid
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <ctype.h>

#ifndef USG
/* only needed for Berkeley UNIX */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
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

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#include <fcntl.h>
#endif
#else
#include <strings.h>
#endif

#ifndef MIN
#define MIN(a, b)	((a < b) ? a : b)
#endif

#ifndef BSD4_3
long lseek();
#else
off_t lseek();
#endif

#if defined(USG) || defined(VMS)
#ifndef L_SET
#define L_SET 0
#endif
#endif

#ifndef VMS
#ifndef MAC
#if defined(ultrix) || defined(USG)
void perror();
void exit ();
#endif
#endif
#endif

#ifndef MAC
#ifdef SYS_V
struct passwd *getpwuid();
#endif
#endif

#if defined(LINT_ARGS)
static void date(char *);
static char *center_string(char *, char *);
static void print_tomb(void);
static void kingly(void);
#else
static int look_line(ARG_INT);
static char *center_string(ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR);
static void date(ARG_CHAR_PTR);
static long total_points(ARG_VOID);
static void kingly(ARG_VOID);
static void print_tomb(ARG_VOID);
static int top_twenty(ARG_VOID);
#endif

#ifndef MAC
char *getlogin();
#ifndef ATARIST_MWC
long time();
#endif
#endif

static void date(day)
char *day;
{
  register char *tmp;
  long clock;

  clock = time((long *) 0);
  tmp = ctime(&clock);
  tmp[10] = '\0';
  (void) strcpy(day, tmp);
}

/* Centers a string within a 31 character string		-JWT-	 */
static char *center_string(centered_str, in_str)
char *centered_str;
char *in_str;
{
  register int i, j;

  i = strlen(in_str);
  j = 15 - i/2;
  (void) sprintf (centered_str, "%*s%s%*s", j, "", in_str, 31 - i - j, "");
  return centered_str;
}


/* Not touched for Mac port */
void display_scores(from, to)
  int from, to;
{
  register int i = 0, j, k, l;
  int fd;
  high_scores score;
 /* MAX_SAVE_HISCORES scores, 2 lines per score */
  char list[2*MAX_SAVE_HISCORES][128];
  char string[100];

  vtype tmp_str;

  if (to<0) to=20;
  if (to>MAX_SAVE_HISCORES) to=MAX_SAVE_HISCORES;
#ifdef MSDOS
  if (1 > (fd = open(ANGBAND_TOP, O_RDONLY | O_BINARY, 0666))) {
	/* binary mode to avoid lf/cr conversions, which cause havoc -CFT */
#else
#ifdef SET_UID
  if (1 > (fd = open(ANGBAND_TOP, O_RDONLY, 0644))) {
#else
  if (1 > (fd = open(ANGBAND_TOP, O_RDONLY, 0666))) {
#endif
#endif
    (void) sprintf(string, "Error opening score file \"%s\"\n", ANGBAND_TOP);
    prt(string, 0, 0);
    return ;
  }

  while (0 < read(fd, (char *)&score, sizeof(high_scores))) {
    int tt; 
    (void) sprintf(list[i], "%3d) %-7ld %*s the %*s %*s (Level %d)",
		   i/2+1,
		   score.points,
		   ((tt = strlen(score.name)) > 35 ? 35 : tt),
		   score.name,
		   ((tt = strlen(race[score.prace].trace)) > 16 ? 16 : tt),
		   race[score.prace].trace,
		   ((tt = strlen(class[score.pclass].title)) > 16 ? 16 : tt),
		   class[score.pclass].title,
		   (int)score.lev);
    (void) sprintf(list[i+1], "    Killed by %s on Dungeon Level %d.",
		   score.died_from, score.dun_level);
    i+=2;
    if (i>=(MAX_SAVE_HISCORES*2)) break;
  }

#ifndef MSDOS
  signal(SIGTSTP,SIG_IGN);  
#endif
  k = from*2;
  do {
    if (k>0) {
      sprintf(tmp_str, "                Angband Hall of Fame (from position %d)", 
	      (k/2)+1);
      put_buffer(tmp_str, 0, 0);
    } else {
      put_buffer("                Angband Hall of Fame                     ", 0, 0);
    }
    put_buffer("Score", 1, 0);
    l=0;
    for (j = k; j<i && j<(to*2) && j<(k+20); j++, l++)
      put_buffer(list[j], l+2, 0);
    k+=20;
    if (!look_line(23)) {
      register int i;

      /* What happens upon dying.				-RAK-	 */
      msg_print(NULL);
      clear_screen();
      flush ();  /* flush all input */
      nosignals ();	 /* Can't interrupt or suspend. */
      (void) save_char ();		/* Save the memory at least. */
      restore_term();
      exit(0);
    }
    clear_screen();
  } while (k<(to*2) && k<i);
}

/* Pauses for user response before returning		-RAK-	*/
static int look_line(prt_line)
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
static void print_tomb()
{
  vtype str, tmp_str;
  register int i;
  char day[11];
  register char *p;
  FILE *fp;

  if (strcmp(died_from, "Interrupting") && !wizard) {
    sprintf(str, "%s%d", ANGBAND_BONES, dun_level);
    if ((fp = fopen(str, "r")) == NULL && (dun_level>1)) {
      if ((fp = fopen(str, "w")) != NULL) {
#ifndef MSDOS   /* why bother for PCs? -CFT */
	(void) fchmod(fileno(fp), 0666);
#endif
	fprintf(fp, "%s\n%d\n%d\n%d", 
		py.misc.name, py.misc.mhp, py.misc.prace, py.misc.pclass);
	fclose(fp);
      }
    } else {
      fclose(fp);
    }
  }
  clear_screen();
  put_buffer ("_______________________", 1, 15);
  put_buffer ("/", 2, 14);
  put_buffer ("\\         ___", 2, 38);
  put_buffer ("/", 3, 13);
  put_buffer ("\\ ___   /   \\      ___", 3, 39);
  put_buffer ("/            RIP            \\   \\  :   :     /   \\", 4, 12);
  put_buffer ("/", 5, 11);
  put_buffer ("\\  : _;,,,;_    :   :", 5, 41);
  (void) sprintf (str, "/%s\\,;_          _;,,,;_",
		  center_string (tmp_str, py.misc.name));
  put_buffer (str, 6, 10);
  put_buffer ("|               the               |   ___", 7, 9);
  if (!total_winner)
    p = title_string ();
  else
    p = "Magnificent";
  (void) sprintf (str, "| %s |  /   \\", center_string (tmp_str, p));
  put_buffer (str, 8, 9);
  put_buffer ("|", 9, 9);
  put_buffer ("|  :   :", 9, 43);
  if (!total_winner)
    p = class[py.misc.pclass].title;
  else if (py.misc.male)
    p = "*King*";
  else
    p = "*Queen*";
  (void) sprintf(str,"| %s | _;,,,;_   ____", center_string (tmp_str, p));
  put_buffer (str, 10, 9);
  (void) sprintf (str, "Level : %d", (int) py.misc.lev);
  (void) sprintf (str,"| %s |          /    \\", center_string (tmp_str, str));
  put_buffer (str, 11, 9);
  (void) sprintf(str, "%ld Exp", py.misc.exp);
  (void) sprintf(str,"| %s |          :    :", center_string (tmp_str, str));
  put_buffer (str, 12, 9);
  (void) sprintf(str, "%ld Au", py.misc.au);
  (void) sprintf(str,"| %s |          :    :", center_string (tmp_str, str));
  put_buffer (str, 13, 9);
  (void) sprintf(str, "Died on Level : %d", dun_level);
  (void) sprintf(str,"| %s |         _;,,,,;_", center_string (tmp_str, str));
  put_buffer (str, 14, 9);
  put_buffer ("|            killed by            |", 15, 9);
  p = died_from;
  i = strlen (p);
  p[i] = '.';  /* add a trailing period */
  p[i+1] = '\0';
  (void) sprintf(str, "| %s |", center_string (tmp_str, p));
  put_buffer (str, 16, 9);
  p[i] = '\0';	 /* strip off the period */
  date(day);
  (void) sprintf(str, "| %s |", center_string (tmp_str, day));
  put_buffer (str, 17, 9);
  put_buffer ("*|   *     *     *    *   *     *  | *", 18, 8);
  put_buffer ("________)/\\\\_)_/___(\\/___(//_\\)/_\\//__\\\\(/_|_)_______",
	      19, 0);

 retry:
  flush();
  put_buffer ("(ESC to abort, return to print on screen)", 23, 0);
  put_buffer ("Character record?", 22, 0);
  if (get_string (str, 22, 18, 60))
    {
      for (i = 0; i < INVEN_ARRAY_SIZE; i++)
	{
	  known1(&inventory[i]);
	  known2(&inventory[i]);
	}
      calc_bonuses ();
      clear_screen ();
      display_char ();
      put_buffer ("Type ESC to skip the inventory:", 23, 0);
      if (inkey() != ESCAPE)
	{
	  clear_screen ();
	  msg_print ("You are using:");
	  (void) show_equip (TRUE, 0);
	  msg_print ("You are carrying:");
	  clear_from (1);
	  (void) show_inven (0, inven_ctr-1, TRUE, 0, 0);
	  msg_print (NULL);
	}
    }
}


/* Calculates the total number of points earned		-JWT-	 */
long total_points()
{
  return (py.misc.max_exp + (100 * py.misc.max_dlv));
}



/* Enters a players name on a hi-score table...    SM */
static int top_twenty()
{
  register int i, j, k;
  high_scores scores[MAX_SAVE_HISCORES], myscore;
  char *tmp;
#if defined(MSDOS) || defined(VMS) || defined(AMIGA) || defined(MAC)
  FILE *highscore_fp; /* used in opening file instead of locking it */
  vtype string; /* used in error msgs -CFT */
#endif
  
  clear_screen();

  if (wizard || to_be_wizard) {
    display_scores (0, 10);
    (void) save_char();
    restore_term();
    exit(0);
  }

  if (!total_winner && !strcmp(died_from, "Interrupting")) {
    msg_print("Score not registered due to interruption.");
    display_scores (0, 10);
    (void) save_char();
    restore_term();
    exit(0);
  }

  if (!total_winner && !strcmp(died_from, "Quitting")) {
    msg_print("Score not registered due to quitting.");
    display_scores (0, 10);
    (void) save_char();
    restore_term();
    exit(0);
  }

  myscore.points = total_points();
  myscore.dun_level = dun_level;
  myscore.lev = py.misc.lev;
  myscore.max_lev = py.misc.max_dlv;
  myscore.mhp = py.misc.mhp;
  myscore.chp = py.misc.chp;
  myscore.uid = -1;
  /* First character of sex, lower case */
  myscore.sex = py.misc.male;
  myscore.prace = py.misc.prace;
  myscore.pclass = py.misc.pclass;
  (void) strcpy(myscore.name, py.misc.name);
  (void) strncpy(myscore.died_from, died_from, strlen(died_from));
  myscore.died_from[strlen(died_from)] = '\0';
  /* Get rid of '.' at end of death description */


  /*  First, get a lock on the high score file so no-one else tries */
  /*  to write to it while we are using it, on VMS and IBMPCs only one
      process can have the file open at a time, so we just open it here */
#if defined(MSDOS) || defined(VMS) || defined(AMIGA) || defined(MAC)
#if defined(MAC) || defined(MSDOS)
  if ((highscore_fp = fopen(ANGBAND_TOP, "rb+")) == NULL)
#else
  if ((highscore_fp = fopen(ANGBAND_TOP, "r+")) == NULL)
#endif
    {
      (void) sprintf (string, "Error opening score file \"%s\"\n", ANGBAND_TOP);
      perror(string);
      exit_game();
    }
  highscore_fd = fileno(highscore_fp); /* get fd from fp...  This must
  					  happen bacause rest of code
  					  assumes fd, not fp  -CFT */
#else
#ifdef ATARIST_TC
  /* 'lock' always succeeds on the Atari ST */
#else
  if (0 != flock((int)fileno(highscore_fp), LOCK_EX))
    {
      perror("Error gaining lock for score file");
      exit_game();
    }
#endif
#endif

  /*  Check to see if this score is a high one and where it goes */
  i = 0;
#ifndef BSD4_3
  (void) lseek(highscore_fd, (long)0, L_SET);
#else
  (void) lseek(highscore_fd, (off_t)0, L_SET);
#endif
  while ((i < MAX_SAVE_HISCORES)
	&& (0 != read(highscore_fd, (char *)&scores[i], sizeof(high_scores))))
    {
      i++;
    }

  j = 0;
  while (j < i && (scores[j].points >= myscore.points))
    {
      j++;
    }
  /* i is now how many scores we have, and j is where we put this score */

  /* If its the first score, or it gets appended to the file */
  if (!i || (i == j && j < MAX_SAVE_HISCORES)) {
    (void) lseek(highscore_fd, (long)(j * sizeof(high_scores)), L_SET);
    (void) write(highscore_fd, (char *)&myscore, sizeof(high_scores));
  } else if (j < i) {
    /* If it gets inserted in the middle */
    /* Bump all the scores up one place */
    for (k = MIN(i, (MAX_SAVE_HISCORES-1)); k > j ; k--)	{
      (void) lseek(highscore_fd, (long)(k * sizeof(high_scores)), L_SET);
      (void) write(highscore_fd, (char *)&scores[k - 1], sizeof(high_scores));
    }
    /* Write out your score */
    (void) lseek(highscore_fd, (long)(j * sizeof(high_scores)), L_SET);
    (void) write(highscore_fd, (char *)&myscore, sizeof(high_scores));
  }

#ifndef MSDOS /* we never locked, so don't unlock...  Should probably also
		  add in VMS, AMIGA, etc...  -CFT */
  (void) flock(highscore_fd, LOCK_UN);
#endif
  (void) close(highscore_fd);
  if (j<10) {
    display_scores(0, 10);
  } else if (j>(i-10)) {
    display_scores(i-10, i);
  } else display_scores(j-5, j+5);
}

/* Enters a players name on the hi-score table     SM	 */
delete_entry(which)
  int which;
{
  register int i, j, k;
  high_scores scores[MAX_SAVE_HISCORES];
  char *tmp;
#if defined(MSDOS) || defined(VMS) || defined(AMIGA) || defined(MAC)
  FILE *highscore_fp; /* used in opening file instead of locking it */
  vtype string; /* used in error msgs -CFT */
#endif

  /*  First, get a lock on the high score file so no-one else tries */
  /*  to write to it while we are using it, on VMS and IBMPCs only one
      process can have the file open at a time, so we just open it here */
#if defined(MSDOS) || defined(VMS) || defined(AMIGA) || defined(MAC)
#if defined(MAC) || defined(MSDOS)
  if ((highscore_fp = fopen(ANGBAND_TOP, "rb+")) == NULL)
#else
  if ((highscore_fp = fopen(ANGBAND_TOP, "r+")) == NULL)
#endif
    {
      (void) sprintf (string, "Error opening score file \"%s\"\n", ANGBAND_TOP);
      perror(string);
      exit_game();
    }
  highscore_fd = fileno(highscore_fp); /* get fd from fp...  This must
  					  happen bacause rest of code
  					  assumes fd, not fp  -CFT */
#else
#ifdef ATARIST_TC
  /* 'lock' always succeeds on the Atari ST */
#else
  if (0 != flock((int)fileno(highscore_fp), LOCK_EX))
    {
      perror("Error gaining lock for score file");
      exit_game();
    }
#endif
#endif

  /*  Check to see if this score is a high one and where it goes */
  i = 0;
#ifndef BSD4_3
  (void) lseek(highscore_fd, (long)0, L_SET);
#else
  (void) lseek(highscore_fd, (off_t)0, L_SET);
#endif
  while ((i<MAX_SAVE_HISCORES) && 
	 (0 != read(highscore_fd, (char *)&scores[i], sizeof(high_scores))))
    i++;
  
  /* If its the first score, or it gets appended to the file */
  lseek(highscore_fd, 0, L_SET);
  write(highscore_fd, (char *)&scores[0], (which-1)*sizeof(high_scores));
  write(highscore_fd, (char *)&scores[which], (i-which)*sizeof(high_scores));

#ifndef MSDOS  /* again, we don't lock, so we don't unlock.  Add in VMS,
		  AMIGA, etc for portability once sure this works.. -CFT */
  (void) flock(highscore_fd, LOCK_UN);
#endif
  (void) close(highscore_fd);
  if (which<10) {
    display_scores(0, 10);
  } else if (which>(i-10)) {
    display_scores(i-10, i);
  } else display_scores(which-5, which+5);
}

/* Change the player into a King!			-RAK-	 */
static void kingly()
{
  register struct misc *p_ptr;
  register char *p;

  /* Change the character attributes.		 */
  dun_level = 0;
  (void) strcpy(died_from, "Ripe Old Age");
  p_ptr = &py.misc;
  (void) restore_level ();
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
void exit_game ()
{
  register int i;

#ifdef MAC
  /* Prevent strange things from happening */
  enablefilemenu(FALSE);
#endif

  /* What happens upon dying.				-RAK-	 */
  msg_print(NULL);
  flush ();  /* flush all input */
  nosignals ();	 /* Can't interrupt or suspend. */
  if (turn >= 0)
    {
      if (total_winner)
	kingly();
      print_tomb();
      if (!wizard && !to_be_wizard) 
	top_twenty();
      else msg_print("Score not registered.");
    }
  i = log_index;
  (void) save_char ();		/* Save the memory at least. */
  if (i > 0)
    display_scores (0, 10);
  erase_line (23, 0);
  restore_term();
  exit(0);
}
