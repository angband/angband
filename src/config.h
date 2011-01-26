#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

/*** Some really important things you ought to change ***/

/*
 * Defines the default paths to the Angband directories, for ports that use
 * the main.c file.
 *
 * "config path" is for per-installation configurable data, like the game's
 * edit files and system-wide preferences.
 *
 * "lib path" is for static data, like sounds, graphics and fonts.
 *
 * "data path" is for variable data, like save files and scores. On single-
 * user systems, this also includes user preferences and dumps (on multi-
 * user systems these go under the user's home directory).
 *
 * The configure script overrides these values. Check the "--prefix=<dir>"
 * option of the configure script.
 *
 * These values will be over-ridden by the "ANGBAND_PATH" environment
 * variable, if that variable is defined and accessible.  The final
 * "slash" is required if the value supplied is in fact a directory.
 *
 * Using the value "./lib/" below tells Angband that, by default,
 * the user will run "angband" from the same directory that contains
 * the "lib" directory.  This is a reasonable (but imperfect) default.
 *
 * If at all possible, you should change this value to refer to the
 * actual location of the folders, for example, "/etc/angband/"
 * or "/usr/share/angband/", or "/var/games/angband/". In fact, if at all
 * possible you should use a packaging system which does this for you.
 *
 * N.B. The data path is only used if USE_PRIVATE_PATHS is not defined.
 * The other two are always used. 
 */
#ifndef DEFAULT_CONFIG_PATH
# define DEFAULT_CONFIG_PATH "." PATH_SEP "lib" PATH_SEP
#endif 

#ifndef DEFAULT_LIB_PATH
# define DEFAULT_LIB_PATH "." PATH_SEP "lib" PATH_SEP
#endif 

#ifndef DEFAULT_DATA_PATH
# define DEFAULT_DATA_PATH "." PATH_SEP "lib" PATH_SEP
#endif 


/*
 * OPTION: Create and use a hidden directory in the users home directory
 * for storing pref files and character dumps.
 */
#ifdef SET_UID
# ifndef PRIVATE_USER_PATH
#  define PRIVATE_USER_PATH "~/.angband"
# endif /* PRIVATE_USER_PATH */
#endif /* SET_UID */


/*
 * OPTION: Create and use hidden directories in the users home directory
 * for storing save files and high-scores
 */
#ifdef PRIVATE_USER_PATH
/* # define USE_PRIVATE_PATHS */
#endif /* PRIVATE_USER_PATH */



/*** Some no-brainer defines ***/

/* Allow the game to make noises correlating to what the player does in-game */
#define USE_SOUND

/* Allow the use of graphics rather than only having a text-mode */
#define USE_GRAPHICS

/* Compile in support for debug commands */
#define ALLOW_DEBUG

/* Compile in support for spoiler generation */
#define ALLOW_SPOILERS

/* Allow changing colours at runtime */
#define ALLOW_COLORS

/* Allow changing "visuals" at runtime */
#define ALLOW_VISUALS



/*** Borg ***/

/* Compile in support for the borg. */
/* #define ALLOW_BORG */

/* Allow borgs to yield "high scores"? */
/* #define SCORE_BORGS */


/*
 * Allow the Borg to use graphics.
 */
#if defined(ALLOW_BORG) && defined(USE_GRAPHICS)
# define ALLOW_BORG_GRAPHICS
#endif


/*** X11 settings ***/

/*
 * OPTION: Gamma correct colours (with X11)
 */
#define SUPPORT_GAMMA


/*
 * OPTION: Default font (when using X11).
 */
#define DEFAULT_X11_FONT		"9x15"


/*
 * OPTION: Default fonts (when using X11)
 */
#define DEFAULT_X11_FONT_0		"10x20"
#define DEFAULT_X11_FONT_1		"9x15"
#define DEFAULT_X11_FONT_2		"9x15"
#define DEFAULT_X11_FONT_3		"5x8"
#define DEFAULT_X11_FONT_4		"5x8"
#define DEFAULT_X11_FONT_5		"5x8"
#define DEFAULT_X11_FONT_6		"5x8"
#define DEFAULT_X11_FONT_7		"5x8"


#endif /* !INCLUDED_CONFIG_H */
