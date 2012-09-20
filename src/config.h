 /* config.h: configuration definitions

   Copyright (c) 1989 James E. Wilson

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

/* Person to bother if something goes wrong. */
/* Recompile files.c and misc2.c if these change. */
#define WIZARD	"root"

/* Note that any reasonably modern compiler does better when you *don't* use
 * "register".  I've hacked it out here because I don't want to change every
 * arg list in the game.  You might want to undo this if your compiler sucks.
 *                   -CWS
 */

#define register


/* There's a bug that results in invisible monsters for some reason.  I have a
 * workaround that may fix this, but it is a HACK and may result in other
 * problems, as I have not tested it enough.  Comment out the
 * "#define GROSS_HACK" to disable this.  (this is in creature.c at line 73)
 */

#define GROSS_HACK


/* Other miscellaneous defines that can be configured as the local maintainer
 * wishes.
 */

#define SET_UID		         /* define on multi-user systems */
#undef CHECKHOURS            /* define if checking the 'hours' file */
#define ALLOW_FIDDLING       /* Allow the players to copy save files */
#define ALLOW_SCORE          /* Allow the user to check his score (v-key) */
#define ALLOW_ARTIFACT_CHECK /* Allow the user to check artifacts */
#define ALLOW_CHECK_UNIQUES  /* Allow player to check (dead) unique monsters */
#define TARGET               /* Targeting mode */
#define AUTOROLLER           /* Allow autorolling of characters */


/* files used by moria, set these to valid pathnames */

/* Try to fix filename inclusion in a portable fashion.
 * John Whitly@cs.Buffalo.edu says this works under gcc 2.5.5, but my
 * older version chokes.  I dunno. -CWS
 */

#ifdef __STDC__
#define LIBDIR(FILE) "/User/games/lib/angband/" #FILE
#else
#define LIBDIR(FILE) "/User/games/lib/angband/FILE"
#endif

/* probably unix */
#define ANGBAND_TST       LIBDIR(test)
#define ANGBAND_HOU       LIBDIR(files/hours)
#define ANGBAND_MOR       LIBDIR(files/news)
#define ANGBAND_TOP       LIBDIR(files/newscores)
#define ANGBAND_BONES     LIBDIR(bones)
#define ANGBAND_HELP      LIBDIR(files/roglcmds.hlp)
#define ANGBAND_ORIG_HELP LIBDIR(files/origcmds.hlp)
#define ANGBAND_WIZ_HELP  LIBDIR(files/rwizcmds.hlp)
#define ANGBAND_OWIZ_HELP LIBDIR(files/owizcmds.hlp)
#define ANGBAND_WELCOME   LIBDIR(files/welcome.hlp)
#define ANGBAND_LOG       LIBDIR(files/ANGBAND.log)
#define ANGBAND_VER       LIBDIR(files/version.hlp)
#define ANGBAND_LOAD      LIBDIR(files/loadcheck)
#define ANGBAND_WIZ       LIBDIR(files/wizards)
#define ANGBAND_SAV       LIBDIR(save)


/* this sets the default user interface
 * to use the original key bindings (keypad for movement) set ROGUE_LIKE
 * to FALSE, to use the rogue-like key bindings (vi style movement)
 * set ROGUE_LIKE to TRUE
 * if you change this, you only need to recompile main.c */

#define ROGUE_LIKE TRUE


/* for the ANDREW distributed file system, define this to ensure that
   the program is secure with respect to the setuid code, this prohibits
   inferior shells, also does not relinquish setuid priviledges at the start,
   but instead calls the ANDREW library routines bePlayer(), beGames(),
   and Authenticate() */

/* #define SECURE */


/* this allows intelligent compilers to do better, as they know more
 * about how certain functions behave -CWS */

#if !(defined(__GNUC__) || defined(__STDC__))
#define const
#endif


/* no system definitions are needed for 4.3BSD, SUN OS, DG/UX */

/* if you are compiling on an ultrix/4.2BSD/Dynix/etc. version of UNIX,
   define this, not needed for SUNs */
/* #ifndef ultrix
#define ultrix
#endif */

/* if you are compiling on a SYS V version of UNIX, define this */
/* #define SYS_V */

/* if you are compiling on a SYS III version of UNIX, define this */
/* #define SYS_III */

/* if you are compiling on an ATARI ST with Mark Williams C, define this */
/* #define ATARIST_MWC */

/* if you are compiling on a Macintosh with MPW C 3.0, define this */
/* #define MAC */

/* if you are compiling on a HPUX version of UNIX, define this */
/* #define HPUX */

/****************************************************************************
 * System dependent defines follow, you should not need to change anything  *
 * below (if you have a supported system).  If you run into problems during *
 * compilation, you might want to check the defines below.                  *
 ****************************************************************************/

/* Note that you'll be happier if you have a case-insensitive string
 * comparision routine on your system.  I'd imagine that a lot of subtle
 * things will go wrong without one.  Define stricmp to something appropriate,
 * and send me mail about what that is and what system you have so that this
 * can get adjusted correctly on all systems. -CWS
 */

#if defined (NeXT) || defined(HPUX) || defined(ultrix) \
|| defined(NCR3K) || defined(linux)
#define stricmp strcasecmp
#endif

/* this takes care of almost all "implicit declaration" warnings -CWS */

#if defined(NeXT)
#include <libc.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#endif

#ifdef __MINT__
#include <support.h>
#endif


/* fix systems lacking usleep() -CWS (thanks to cba) */

#if defined(HPUX) || defined(ultrix)
#define NEEDS_USLEEP
#endif

#ifdef NEEDS_USLEEP
#define usleep microsleep

#ifdef __STDC__
int microsleep(unsigned long);
#else
int microsleep();
#endif /* __STDC__ */

#endif

/* substitute strchr for index on USG versions of UNIX */
#if defined(SYS_V) || defined(MSDOS) || defined(MAC)
#define index strchr
#endif

#ifdef SYS_III
char *index();
#endif

#if defined(SYS_III) || defined(SYS_V) || defined(MSDOS) || defined(MAC) || defined(HPUX)
#ifndef USG
#define USG
#endif
#endif

#if defined(ATARIST_MWC) || defined (__MINT__)
#ifndef USG
#define USG
#endif
#endif

/* Pyramid runs 4.2BSD-like UNIX version */
#if defined(Pyramid)
#define ultrix
#endif

#ifdef MSDOS
#define register      /* MSC 4.0 still has a problem with register bugs ... */
#endif

#ifdef MAC
#ifdef RSRC
#define MACRSRC		/* i.e., we're building the resources */
#else
#define MACGAME		/* i.e., we're building the game */
#endif
#endif

#ifdef MAC
/* Screen dimensions */
#define SCRN_ROWS	24
#define SCRN_COLS	80
#endif

#if vms
#define getch _getch
#define unlink delete
#define index strchr
#define lstat stat
#define exit uexit
#endif

#if defined(SYS_V) && defined(lint)
/* to prevent <string.h> from including <NLchar.h>, this prevents a bunch
   of lint errors. */
#define RTPC_NO_NLS
#endif

#ifdef SECURE
extern int PlayerUID;
#define getuid() PlayerUID
#define geteuid() PlayerUID
#endif

/*****************************************************************************/

/* Here's some functions that've been macroized rather than being called
 * from everywhere.  They're short enough so that inlining them will probably
 * result in a smaller executable, and speed things up, to boot. -CWS
 */

#define MY_MAX(a,b) ((a) > (b) ? (a) : (b))
#define MY_MIN(a,b) ((a) < (b) ? (a) : (b))

/* Checks a co-ordinate for in bounds status		-RAK-	*/
#define in_bounds(y, x) \
   ((((y) > 0) && ((y) < cur_height-1) && ((x) > 0) && ((x) < cur_width-1)) ? \
    (TRUE) : (FALSE))

/* Checks if we can see this point (includes map edges) -CWS */
#define in_bounds2(y, x) \
   ((((y) >= 0) && ((y) < cur_height) && ((x) >= 0) && ((x) < cur_width)) ? \
    (TRUE) : (FALSE))

/* Tests a given point to see if it is within the screen -RAK-	*/
/* boundaries.							  */
#define panel_contains(y, x) \
  ((((y) >= panel_row_min) && ((y) <= panel_row_max) && \
    ((x) >= panel_col_min) && ((x) <= panel_col_max)) ? (TRUE) : (FALSE))

/* Generates a random integer X where 1<=X<=MAXVAL	-RAK-	*/
#define randint(maxval) (((maxval) < 1) ? (1) : ((random() % (maxval)) + 1))

#if 0
/* You would think that most compilers can do an integral abs() quickly,
 * wouldn't you?  Nope.  [But fabs is a lot worse on most machines!] -CWS */
#define MY_ABS(x) (((x)<0) ? (-x) : (x))

/* Distance between two points				-RAK-	*/
/* common subexpression elimination'll have a field day with this when you
 * optimize; it'll be loads faster than a function call.... -CWS */

#define distance(y1, x1, y2, x2) \
   (( ((MY_ABS((y1) - (y2)) + MY_ABS((x1) - (x2))) << 1) - \
     ( (MY_ABS((y1) - (y2)) > MY_ABS((x1) - (x2))) ? MY_ABS((x1) - (x2)) : \
      MY_ABS((y1) - (y2)))) >> 1)
#endif

/*****************************************************************************/
