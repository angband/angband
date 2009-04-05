#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

/*** Some really important things you ought to change ***/

/*
 * Defines the default path to the Angband "lib" directory, for ports that use
 * the main.c file.
 *
 * The configure script overrides this value.  Check the "--prefix=<dir>"
 * option of the configure script.
 *
 * This value will be over-ridden by the "ANGBAND_PATH" environment
 * variable, if that variable is defined and accessable.  The final
 * "slash" is required if the value supplied is in fact a directory.
 *
 * Using the value "./lib/" below tells Angband that, by default,
 * the user will run "angband" from the same directory that contains
 * the "lib" directory.  This is a reasonable (but imperfect) default.
 *
 * If at all possible, you should change this value to refer to the
 * actual location of the "lib" folder, for example, "/tmp/angband/lib/"
 * or "/usr/games/lib/angband/", or "/pkg/angband/lib".
 */
#ifndef DEFAULT_PATH
# define DEFAULT_PATH "." PATH_SEP "lib" PATH_SEP
#endif /* DEFAULT_PATH */


/*
 * OPTION: Create and use a hidden directory in the users home directory
 * for storing pref-files and character-dumps.
 */
#ifdef SET_UID
# ifndef PRIVATE_USER_PATH
#  define PRIVATE_USER_PATH "~/.angband"
# endif /* PRIVATE_USER_PATH */
#endif /* SET_UID */


/*
 * OPTION: Create and use hidden directories in the users home directory
 * for storing save files, data files, and high-scores
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

/* Allow chaning macros at run-time */
#define ALLOW_MACROS

/* Allow parsing of the lib/edit/ files. */
#define ALLOW_TEMPLATES



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



/*
 * OPTION: Allow output of 'parsable' ascii template files.
 * This can be used to help change the ascii template format, and to make
 * changes to the data in the parsed files within Angband itself.
 *
 * Files are output to lib/user with the same file names as lib/edit.
 */
/* #define ALLOW_TEMPLATES_OUTPUT */


/*
 * OPTION: use "power" rating algorithm to determine prices of wearable
 * items (weapons, armour, jewelry, light sources, ammo). If this option is
 * used, only wearable items will be priced this way - consumables will still
 * use prices from text files.
 */
#define POWER_PRICING


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
