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
 * Some popular options include "USE_GCU" (allow use with Unix "curses"),
 * "USE_X11" (allow basic use with Unix X11), "USE_XAW" (allow use with
 * Unix X11 plus the Athena Widget set), and "USE_CAP" (allow use with
 * the "termcap" library, or with hard-coded vt100 terminals).
 *
 * The old "USE_NCU" option has been replaced with "USE_GCU".
 *
 * Several other such options are available for non-unix machines,
 * such as "MACINTOSH", "WINDOWS", "USE_IBM", "USE_EMX".
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
 * OPTION: Include "ncurses.h" instead of "curses.h" in "main-gcu.c"
 */
/* #define USE_NCURSES */


/*
 * OPTION: for multi-user machines running the game setuid to some other
 * user (like 'games') this SAFE_SETUID option allows the program to drop
 * its privileges when saving files that allow for user specified pathnames.
 * This lets the game be installed system wide without major security
 * concerns.  There should not be any side effects on any machines.
 *
 * This will handle "gids" correctly once the permissions are set right.
 */
#define SAFE_SETUID


/*
 * This flag enables the "POSIX" methods for "SAFE_SETUID".
 */
#ifdef _POSIX_SAVED_IDS
# define SAFE_SETUID_POSIX
#endif


/*
 * This "fix" is from "Yoshiaki KASAHARA <kasahara@csce.kyushu-u.ac.jp>"
 * It prevents problems on (non-Solaris) Suns using "SAFE_SETUID".
 */
#if defined(sun) && !defined(SOLARIS)
# undef SAFE_SETUID_POSIX
#endif




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
 * the use of savefiles from other platforms, so be careful.
 */
/* #define VERIFY_TIMESTAMP */


/*
 * OPTION: Forbid the "savefile over-write" cheat, in which you simply
 * run another copy of the game, loading a previously saved savefile,
 * and let that copy over-write the "dead" savefile later.  This option
 * either locks the savefile, or creates a fake "xxx.lok" file to prevent
 * the use of the savefile until the file is deleted.  Not ready yet.
 */
/* #define VERIFY_SAVEFILE */



/*
 * OPTION: Hack -- Compile in support for "Cyborg" mode
 */
/* #define ALLOW_BORG */

/*
 * OPTION: Hack -- Compile in support for "Wizard Commands"
 */
/* #define ALLOW_WIZARD */

/*
 * OPTION: Hack -- Compile in support for "Spoiler Generation"
 */
/* #define ALLOW_SPOILERS */


/*
 * OPTION: Allow "macro definition" at run-time
 */
#define ALLOW_MACROS

/*
 * OPTION: Allow "keymap modification" at run-time
 */
#define ALLOW_KEYMAP


/*
 * OPTION: Allow characteres to be "auto-rolled"
 */
#define ALLOW_AUTOROLLER


/*
 * OPTION: Allow monsters to "flee" when hit hard
 */
#define ALLOW_FEAR

/*
 * OPTION: Allow monsters to "flee" from strong players
 */
#define ALLOW_TERROR

/*
 * OPTION: Allow "titles" for the player (based on level and class)
 * Using this option adds a huge table of strings in "tables.c" and
 * prints pretty "titles" such as "Ranger Lord" after the race and
 * class, and on the tombstone, and in the "description" files.
 */
/* #define ALLOW_TITLES */


/*
 * OPTION: Allow parsing of the ascii template files in "init.c".
 * This must be defined if you do not have valid binary image files.
 * It should be usually be defined anyway to allow easy "updating".
 */
#define ALLOW_TEMPLATES

/*
 * OPTION: Allow loading of pre-2.7.0 savefiles.  Note that it takes
 * about 15K of code in "save-old.c" to parse the old savefile format.
 * Angband 2.8.0 will ignore a lot of info from pre-2.7.0 savefiles.
 */
#define ALLOW_OLD_SAVEFILES


/*
 * OPTION: Handle signals
 */
#define HANDLE_SIGNALS


/*
 * Allow "Wizards" to yield "high scores"
 */
/* #define SCORE_WIZARDS */

/*
 * Allow "Borgs" to yield "high scores"
 */
/* #define SCORE_BORGS */

/*
 * Allow "Cheaters" to yield "high scores"
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
 *
 * This option has caused some trouble in several versions, usually
 * related to off-screen monster spell casting or frozen monsters.
 */
/* #define WDT_TRACK_OPTIONS */



/*
 * OPTION: Allow the use of "color" in various places.  Disabling this
 * flag will remove some code, and auto-cast all colors to "White".
 * This will almost certainly speed up the program.  Note that there
 * is a software level flag as well ("use_color") which is almost as
 * good at speeding up the code.
 */
#define USE_COLOR		/* Include full support for color terminals */


/*
 * OPTION: Hack -- Macintosh stuff
 */
#ifdef MACINTOSH

/* Do not handle signals */
# undef HANDLE_SIGNALS

#endif


/*
 * OPTION: Hack -- Windows stuff
 */
#ifdef WINDOWS

/* Do not handle signals */
# undef HANDLE_SIGNALS

/* Use something */
# define USE_ITSYBITSY

#endif



/*
 * OPTION: Set the "default" path to the angband "lib" directory.
 *
 * See "main.c" for usage, and note that this value is only used on
 * certain machines, primarily Unix machines.  If this value is used,
 * it will be over-ridden by the "ANGBAND_PATH" environment variable,
 * if that variable is defined and accessable.  The final slash is
 * optional, but it may eventually be required.
 *
 * Using the value "./lib/" below tells Angband that, by default,
 * the user will run "angband" from the same directory that contains
 * the "lib" directory.  This is a reasonable (but imperfect) default.
 *
 * If at all possible, you should change this value to refer to the
 * actual location of the "lib" folder, for example, "/tmp/angband/lib/"
 * or "/usr/games/lib/angband/", or "/pkg/angband/lib".
 */
#define DEFAULT_PATH "./lib/"


/*
 * On multiuser systems, add the "uid" to savefile names
 */
#ifdef SET_UID
# define SAVEFILE_USE_UID
#endif


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
#if defined(MACINTOSH) || defined(WINDOWS) || defined(AMIGA)
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
 * OPTION: Allow the use of a "Mirror Window", if supported
 */
#define GRAPHIC_MIRROR



/*
 * OPTION: Shimmer Multi-Hued monsters/objects
 */
#define SHIMMER_MONSTERS
#define SHIMMER_OBJECTS


/*
 * OPTION: Person to bother if something goes wrong.
 */
#define MAINTAINER	"benh@voicenet.com"


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



/*
 * OPTION: Attempt to prevent all "cheating"
 */
/* #define VERIFY_HONOR */


/*
 * React to the "VERIFY_HONOR" flag
 */
#ifdef VERIFY_HONOR
# define VERIFY_SAVEFILE
# define VERIFY_CHECKSUMS
# define VERIFY_TIMESTAMPS
#endif


/*
 * OPTION: Attempt to minimize the size of the game
 */
/* #define ANGBAND_LITE */

/*
 * Hack -- React to the "ANGBAND_LITE" flag
 */
#ifdef ANGBAND_LITE
# undef MONSTER_FLOW
# undef WDT_TRACK_OPTIONS
# undef DRS_SMART_OPTIONS
# undef ALLOW_OLD_SAVEFILES
# undef ALLOW_BORG
# undef ALLOW_WIZARD
# undef ALLOW_SPOILERS
# undef ALLOW_TITLES
# undef GRAPHIC_RECALL
# undef GRAPHIC_CHOICE
# undef GRAPHIC_MIRROR
#endif


