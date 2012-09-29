/* File: config.h */

/* Purpose: Angband specific configuration stuff */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


/*
 * Look through the following lines, and where a comment includes the
 * tag "OPTION:", examine the associated "#define" statements, and decide
 * whether you wish to keep, comment, or uncomment them.  You should not
 * have to modify any lines not indicated by "OPTION".
 *
 * Note: Also examine the "system" configuration file "h-config.h"
 * and the variable initialization file "variables.c".  If you change
 * anything in "variables.c", you only need to recompile that file.
 */



/*
 * OPTION: Person to bother if something goes wrong.
 */
#define WIZARD	"benh@linc.cis.upenn.edu"



/*
 * OPTION: for the AFS distributed file system, define this to ensure that
 * the program is secure with respect to the setuid code, this prohibits
 * inferior shells, also does not relinquish setuid priviledges at the start,
 * but instead calls the AFS library routines bePlayer(), beGames(),
 * and Authenticate().
 */
/* #define SECURE */






/*
 * OPTION: Other miscellaneous defines that can be configured as the local
 * maintainer wishes.  If you want any of them, define the respective flag.
 * Note that some of these are turned off later if they make no sense.
 */
#define ALLOW_FIDDLING		/* Allow the players to copy save files */
#define ALLOW_SCORE		/* Allow the user to check his score (v-key) */
#define ALLOW_ARTIFACT_CHECK	/* Allow the user to check artifacts */
#define ALLOW_CHECK_UNIQUES	/* Allow player to check (dead) uniques */
#define AUTOROLLER		/* Allow autorolling of characters */
#define TARGET			/* Enable targeting mode */
#define TAGGER			/* Enable inventory tags -BEN- */
#undef  KEYMAP			/* The "keymapper" (not ready yet) -BEN- */


/*
 * OPTION: Assume we may use "floating point" for better distribution
 */
#define USE_FLOATING_POINT

/*
 * Turn it off on Windows machines
 */
#ifdef _Windows
# undef USE_FLOATING_POINT
#endif

/*
 * OPTION: Compile for X11 instead of Curses
 *
 * If you define it here, do not define it in the Makefile too.
 */
/* #define USE_X11 */


/*
 * OPTION: Set the "default" path to the angband "lib" directory.
 * Angband will use this value if it cannot getenv("ANGBAND_PATH").
 * The final slash is optional in either case.  Not used on Macintosh.
 */
#define DEFAULT_PATH "/tmp/angband/lib/"


/*
 * OPTION: On multiuser systems, default to checking the "Hours"
 * and "Load" files, being nice, and to using the player UID in
 * the savefile names, and to NOT allowing the savefile names to
 * be changed.  On single user machines, do just the opposite.
 */
#ifdef SET_UID
# define NICE			/* Be nice while auto-rolling */
# define CHECK_HOURS		/* Check the "Hours" file */
# define CHECK_LOAD		/* Check the "Load" file */
# define SAVEFILE_USE_UID	/* Use the "uid" in savefiles */
#else
# undef NICE			/* Be nice while auto-rolling */
# undef CHECK_HOURS		/* Check the "Hours" file */
# undef CHECK_LOAD		/* Check the "Load" file */
# undef SAVEFILE_USE_UID	/* Use the "uid" in savefiles */
#endif


/*
 * OPTION: Attempt to do "quick" array initialization from "binary"
 * files (in the "data" directory).  Be sure to remove these files
 * every time the "monster race" or "object kind" structures are
 * modified.  And never send these files to other platforms.
 */
#define BINARY_ARRAY_IMAGES


/*
 * OPTION: Hack -- (Don't) check the Hours or Load files
 */
#undef CHECK_HOURS		/* Check the 'hours' file */
#undef CHECK_LOAD		/* Check the 'load' file (needs '-lrpcsvs') */


/*
 * OPTION: For some brain-dead computers with no command line interface,
 * namely Macintosh, there has to be some way of "naming" your savefiles.
 * The current "Macintosh" hack is to make it so whenever the character
 * name changes, the savefile is renamed accordingly.  But on normal
 * machines, once you manage to "load" a savefile, it stays that way.
 * Macintosh is particularly weird because you can load savefiles that
 * are not contained in the "lib:save:" folder, and if you change the
 * player's name, it will then save the savefile elsewhere.
 */
#ifdef MACINTOSH
# define SAVEFILE_MUTABLE	/* Player "name changes" affect savefile name */
#endif



/*
 * OPTION: Compile support code for the "Curses" system.  This is a
 * compilation option, note that the "main()" function can choose
 * not to "activate" the "curses" code.  But defining "USE_CURSES"
 * will ALLOW the "executable" to run using "curses" if so desired.
 * Note that the platform must support curses to use this option.
 */
/* #define USE_CURSES */


/*
 * The Macintosh supports both the "RECALL" and "CHOICE" windows.
 * The Macintosh defaults to using both colors and "shimmer".
 * Also, we can NEVER use "curses" routines on the Macintosh.
 */
#ifdef MACINTOSH
# define GRAPHIC_RECALL		/* Monster Recall */
# define GRAPHIC_CHOICE		/* Not quite ready */
# define USE_COLOR		/* Include full support for color terminals */
# define USE_MULTIHUED		/* Include full "MULTIHUED" support */
# undef USE_CURSES		/* Cannot use Curses */
#endif


/*
 * EMX terminal using video.a.  -EK-
 * OPTION: Refuse to work with Curses.
 */
#ifdef __EMX__
# undef USE_CURSES
#endif

/*
 * Note that the "__EMX__" makefile requires a certain directory
 */
#ifdef __EMX__
# undef DEFAULT_PATH
# define DEFAULT_PATH "./lib/"
#endif


/*
 * OPTION: If "USE_X11" has been defined elsewhere, the options here
 * can be used to prepare a set of "default" compilation options for
 * the "X11" support.  Note that both "USE_X11" and "USE_CURSES" can
 * both be compiled at once.  Note also that the support for the
 * "RECALL" and "CHOICE" windows is not ready yet (see "main-x11.c").
 * Supporting X11 defaults to supporting "Color" and "Shimmer".
 */
#ifdef USE_X11
# define USE_COLOR 		/* Include full support for color terminals */
# define USE_MULTIHUED		/* Include full "MULTIHUED" support */
#endif

/*
 * OPTION: This is the "Default" font when using X11.
 */
#ifdef USE_X11
# define USE_X11_FONT		"9x15"
#endif




/*
 * Prepare to use the "Secure" routines
 */
#ifdef SECURE
  extern int PlayerUID;
# define getuid() PlayerUID
# define geteuid() PlayerUID
#endif


/*
 * Make sure that "usleep()" works.
 */
#if !defined(HPUX) && !defined(ultrix) && !defined(SOLARIS)
# define HAS_USLEEP
#endif


