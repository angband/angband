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
 *
 * And finally, remember that the "Makefile" will specify some rather
 * important compile time options, like what visual module to use.
 */



/*
 * OPTION: Person to bother if something goes wrong.
 */
#define WIZARD	"benh@linc.cis.upenn.edu"


/*
 * OPTION: Hack -- Compile in support for "Wizard Commands"
 */
#define ALLOW_WIZARD


/*
 * OPTION: Hack -- Compile in support for "Cyborg" mode
 */
/* #define AUTO_PLAY */


/*
 * OPTION: for the AFS distributed file system, define this to ensure that
 * the program is secure with respect to the setuid code, this prohibits
 * inferior shells, also does not relinquish setuid priviledges at the start,
 * but instead calls the AFS library routines bePlayer(), beGames(),
 * and Authenticate().
 */
/* #define SECURE */




/*
 * OPTION: Verify savefile Checksums (Angband 2.7.0 and up)
 */
#define VERIFY_CHECKSUMS


/*
 * OPTION: Allow the player to copy save files.  Turning this off
 * may or may not stop players from doing it, though.  In fact, this
 * option is not even referenced on SET_UID machines.
 */
#define ALLOW_FIDDLING


/*
 * OPTION: Allow checking of artifacts (in town)
 */
#define ALLOW_CHECK_ARTIFACTS
 
/*
 * OPTION: Allow checking of dead uniques
 */
#define ALLOW_CHECK_UNIQUES

/*
 * OPTION: Allow "inventory tagging" via inscriptions
 */
#define ALLOW_TAGS

/*
 * OPTION: Compile support for simple macro expansion
 */
#define ALLOW_MACROS

/*
 * OPTION: Compile support for keymap modification
 */
#define ALLOW_KEYMAP

/*
 * OPTION: Allow characteres to be "auto-rolled"
 */
#define AUTOROLLER

/*
 * OPTION: Allow locations and monsters to be "targetted"
 */
#define TARGET


/*
 * OPTION: Hack -- allow "proper" memorization of dungeon features
 */
/* #define NEW_MAP */


/*
 * OPTION: Hack -- allow "monster flowing" to the given depth, if any
 */
#define MONSTER_FLOW


/*
 * OPTION: Maximum flow depth when using "MONSTER_FLOW"
 */
#define MONSTER_FLOW_DEPTH 32


/*
 * OPTION: Compile in all necessary color code.  Undefining either of
 * these will result in a decrease in code size and an increase in
 * execution speed.
 */
#define USE_COLOR		/* Include full support for color terminals */
#define USE_MULTIHUED		/* Include full "MULTIHUED" support */


/*
 * OPTION: Hack -- something for Windows
 */
#if defined(_Windows)
# define USE_ITSYBITSY
#endif


/*
 * OPTION: Hack -- "Raybould's Amiga curses" has broken colors (?)
 */
#if defined(AMIGA)
# undef USE_COLOR
#endif


/*
 * OPTION: Set the "default" path to the angband "lib" directory.
 * Angband will use this value if it cannot getenv("ANGBAND_PATH").
 * The final slash is optional in either case.  Not used on Macintosh.
 * By default, the system expects the "angband" program to be located
 * in the same directory as the "lib" directory.  This can be changed.
 * Note that the "ANGBAND_PATH" environment variable over-rides this.
 * Note: this value is ignored by Macintosh, Windows, and Amiga, see
 * the file "arrays.c" for details.
 */
#define DEFAULT_PATH "./lib/"


/*
 * OPTION: On multiuser systems, be "nice" when autorolling.
 */
#ifdef SET_UID
# define NICE
#endif

/*
 * OPTION: On multiuser systems, add the "uid" to savefile names
 */
#ifdef SET_UID
# define SAVEFILE_USE_UID
#endif


/*
 * OPTION: Attempt to do "quick" array initialization from "binary"
 * files (in the "data" directory).  Be sure to remove these files
 * every time the "monster race" or "object kind" structures are
 * modified.  And never send these files to other platforms.
 *
 * This whole concept needs to be optimized some more, it takes too
 * long to initialize on Macintosh / IBM machines...
 */
#define BINARY_ARRAY_IMAGES


/*
 * OPTION: Check the "hours" file
 */
#undef CHECK_HOURS

/*
 * OPTION: Check the "hours" file (may need the 'rpcsvs' library)
 */
#undef CHECK_LOAD


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
#if defined(MACINTOSH) || defined(_Windows) || defined(AMIGA)
# define SAVEFILE_MUTABLE
#endif


/*
 * OPTION: Capitalize the "user_name" (used as the "default" player name)
 */
#define CAPITALIZE_USER_NAME


/*
 * OPTION: See the Makefile, where several options may be declared.
 * These options control the choice of which graphic systems to
 * compile support for, and include "USE_X11", "USE_NCU", "USE_GCU",
 * and the more or less obsolete "USE_CUR".  Note that "USE_NCU"
 * is geared more towards linux/sys-v machines, and "USE_GCU" is
 * geared more towards general case machines.
 *
 * Several other such options are available for non-unix machines,
 * in particular, "MACINTOSH", and "USE_IBM", "USE_EMX", "USE_WIN".
 *
 * In addition, "SPECIAL_BSD" can be defined for using certain versions
 * of "BSD" unix that use a slightly odd version of Curses (main-gcu.c).
 */


/*
 * OPTION: Use the POSIX "termios" methods in "main-gcu.c"
 */
/* #define USE_TPOSIX */

/*
 * OPTION: Use the "termio" methods in "main-gcu.c"
 */
/* #define USE_TERMIO */

/*
 * OPTION: Use the icky BSD "tchars" methods in "main-gcu.c"
 */
/* #define USE_TCHARS */


/*
 * OPTION: Use "blocking getch() calls" in "main-gcu.c".
 * Hack -- Note that this option will NOT work on many BSD machines
 * Currently used whenever available, if you get a warning about
 * "nodelay()" undefined, then make sure to undefine this.
 */
#if defined(SYS_V) || defined(AMIGA)
# define USE_GETCH
#endif


/*
 * OPTION: Use the "curs_set()" call in "main-gcu.c".
 * Hack -- This option will not work on most BSD machines
 */
#ifdef SYS_V
# define USE_CURS_SET
#endif



/*
 * OPTION: Allow the use of a "Recall Window", if supported
 */
#define GRAPHIC_RECALL


/*
 * OPTION: Allow the use of a "Choice Window", if supported
 */
#define GRAPHIC_CHOICE


/*
 * OPTION: This is the "Default" font when using X11.
 */
#define DEFAULT_X11_FONT	"9x15"




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
 *
 * In general, this is only referenced by "Unix" machines.
 */
#if !defined(HPUX) && !defined(ultrix) && !defined(SOLARIS) && !defined(SGI)
# define HAS_USLEEP
#endif



