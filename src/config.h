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
 * and the variable initialization file "variable.c".  If you change
 * anything in "variable.c", you only need to recompile that file.
 *
 * And finally, remember that the "Makefile" will specify some rather
 * important compile time options, like what visual module to use.
 */


/*
 * OPTION: See the Makefile(s), where several options may be declared.
 *
 * These options control the choice of which graphic systems to
 * compile support for, and include "USE_X11", "USE_NCU", "USE_GCU",
 * and the more or less obsolete "USE_CUR".  Note that "USE_NCU"
 * is geared more towards linux/sys-v machines, and "USE_GCU" is
 * geared more towards general case machines.  Note that "USE_CAP"
 * is not usable at this time.
 *
 * Several other such options are available for non-unix machines,
 * in particular, "MACINTOSH", and "USE_IBM", "USE_EMX", "USE_WIN".
 *
 * You may also need to specify the "system", using defines such as
 * "SOLARIS" (for Solaris), etc, see "h-config.h" for more info.
 */


/*
 * OPTION: define "SPECIAL_BSD" for using certain versions of UNIX
 * that use the 4.4BSD Lite version of Curses in "main-gcu.c"
 */
/* #define SPECIAL_BSD */


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
 * OPTION: for multi-user machines running the game setuid to some other
 * user (like 'games') this SAFE_SETUID option allows the program to drop
 * its privileges when saving files that allow for user specified pathnames.
 * This lets the game be installed system wide without major security
 * concerns.  There should not be any side effects on any machines.
 *
 * Note: this also handles setgid's properly.
 *
 * The "SAFE_SETUID" code is by "Michael H. Price II" <mhp1@Ra.MsState.Edu>
 */
#define SAFE_SETUID


/*
 * OPTION: for the AFS distributed file system, define this to ensure that
 * the program is secure with respect to the setuid code.  This option has
 * not been tested (to the best of my knowledge).  This option may require
 * some weird tricks with "player_uid" and such involving "defines".
 * Note that this option used the AFS library routines Authenticate(),
 * bePlayer(), beGames() to enforce the proper priviledges.
 * You may need to turn "SAFE_SETUID" off to use this option.
 */
/* #define SECURE */


/*
 * OPTION: Verify savefile Checksums (Angband 2.7.0 and up)
 * This option can help prevent "corruption" of savefiles, and also
 * stop intentional modification by amateur users.
 */
#define VERIFY_CHECKSUMS


/*
 * OPTION: Forbid the use of "fiddled" savefiles.  As far as I can tell,
 * a fiddled savefile is one with an internal timestamp different from
 * the actual timestamp.  Thus, turning this option on forbids one from
 * copying a savefile to a different name.  Combined with disabling the
 * ability to save the game without quitting, and with some method of
 * stopping the user from killing the process at the tombstone screen,
 * this should prevent the use of backup savefiles.  It may also stop
 * the use savefiles from other platforms, so be careful.
 */
/* #define VERIFY_TIMESTAMP */



/*
 * OPTION: Hack -- Compile in support for "Spoiler Generation"
 */
/* #define ALLOW_SPOILERS */

/*
 * OPTION: Hack -- Compile in support for "Wizard Commands"
 */
/* #define ALLOW_WIZARD */

/*
 * OPTION: Hack -- Compile in support for "Cyborg" mode
 */
/* #define AUTO_PLAY */

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
#define ALLOW_AUTOROLLER

/*
 * OPTION: Allow locations and monsters to be "targetted"
 */
#define ALLOW_TARGET

/*
 * OPTION: Allow monsters to "flee" when hit hard
 */
#define ALLOW_FEAR

/*
 * OPTION: Allow monsters to "flee" from strong players
 */
#define ALLOW_TERROR




/*
 * Allow "Wizards" to yield "high scores" (see "wizard")
 */
/* #define SCORE_WIZARDS */

/*
 * Allow "Cheaters" to yield "high scores" (see "cheat_xxxx")
 */
/* #define SCORE_CHEATERS */




/*
 * OPTION: Allow use of the "flow_by_smell" and "flow_by_sound"
 * software options, which enable "monster flowing".
 */
#define MONSTER_FLOW


/*
 * OPTION: Maximum flow depth when using "MONSTER_FLOW"
 */
#define MONSTER_FLOW_DEPTH 32



/*
 * OPTION: Allow use of extended spell info	-DRS-
 */
#define DRS_SHOW_SPELL_INFO

/*
 * OPTION: Allow use of the monster health bar	-DRS-
 */
#define DRS_SHOW_HEALTH_BAR


/*
 * OPTION: Enable the "smart_learn" and "smart_cheat" options.
 * They let monsters make more "intelligent" choices about attacks
 * (including spell attacks) based on their observations of the
 * player's reactions to previous attacks.  The "smart_cheat" option
 * lets the monster know how the player would react to an attack
 * without actually needing to make the attack.  The "smart_learn"
 * option requires that a monster make a "failed" attack before
 * learning that the player is not harmed by that attack.
 *
 * This adds about 3K to the memory and about 5K to the executable.
 */
#define DRS_SMART_OPTIONS



/*
 * OPTION: Enable the "track_follow" and "track_target" options.
 * They let monsters follow the player's foot-prints, or remember
 * the player's recent locations.
 *
 * This adds about 33K to the memory and 1K to the executable.
 */
#define WDT_TRACK_OPTIONS



/*
 * OPTION: Compile in all necessary color code.  Undefining either of
 * these will result in a decrease in code size and an increase in
 * execution speed, but will totally disable the use of color.
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
 *
 * Note: this value is ignored by Macintosh, Windows, and Amiga, see
 * the file "init.c" for details.
 *
 * Note that the "ANGBAND_PATH" environment variable over-rides this.
 * Angband will use this value if it cannot getenv("ANGBAND_PATH").
 * Note that the final slash is optional in any case.
 *
 * By default, the system expects the "angband" program to be located
 * in the same directory as the "lib" directory.  This can be changed.
 * One common alternative is "/usr/games/lib/angband/".
 */
#define DEFAULT_PATH "./lib/"


/*
 * On multiuser systems, add the "uid" to savefile names
 */
#ifdef SET_UID
# define SAVEFILE_USE_UID
#endif


/*
 * OPTION: Attempt to do "quick" array initialization from "binary"
 * files (in the "data" directory).  These files are not portable
 * between platforms, and are regenerated if missing or out of date.
 */
#define BINARY_ARRAY_IMAGES


/*
 * OPTION: Check the "time" against "lib/file/hours.txt"
 */
/* #define CHECK_TIME */

/*
 * OPTION: Check the "load" against "lib/file/load.txt"
 * This may require the 'rpcsvs' library
 */
/* #define CHECK_LOAD */


/*
 * OPTION: For some brain-dead computers with no command line interface,
 * namely Macintosh, there has to be some way of "naming" your savefiles.
 * The current "Macintosh" hack is to make it so whenever the character
 * name changes, the savefile is renamed accordingly.  But on normal
 * machines, once you manage to "load" a savefile, it stays that way.
 * Macintosh is particularly weird because you can load savefiles that
 * are not contained in the "lib:save:" folder, and if you change the
 * player's name, it will then save the savefile elsewhere.  Note that
 * this also gives a method of "bypassing" the "VERIFY_TIMESTAMP" code.
 */
#if defined(MACINTOSH) || defined(_Windows) || defined(AMIGA)
# define SAVEFILE_MUTABLE
#endif


/*
 * OPTION: Capitalize the "user_name" (for "default" player name)
 * This option is only relevant on SET_UID machines.
 */
#define CAPITALIZE_USER_NAME


/*
 * OPTION: Allow the use of a "Recall Window", if supported
 */
#define GRAPHIC_RECALL


/*
 * OPTION: Allow the use of a "Choice Window", if supported
 */
#define GRAPHIC_CHOICE






/*
 * OPTION: Person to bother if something goes wrong.
 */
#define MAINTAINER	"benh@linc.cis.upenn.edu"


/*
 * OPTION: Default font (when using X11).
 */
#define DEFAULT_X11_FONT		"9x15"

/*
 * OPTION: Default fonts (when using X11)
 */
#define DEFAULT_X11_FONT_SCREEN		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_RECALL		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_CHOICE		DEFAULT_X11_FONT


