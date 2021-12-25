/**
 * \file config.h
 * \brief Configuration options
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Chris Carr, Andi Sidwell
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

/**
 * ------------------------------------------------------------------------
 * Some really important things you ought to change
 * ------------------------------------------------------------------------ */


/**
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
 * option of the configure script.  The final "slash" is required if the
 * value supplied is in fact a directory.
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


/**
 * OPTION: Create and use a hidden directory in the users home directory
 * for storing pref files and character dumps.
 */
#if defined(UNIX) && !defined(MACH_O_CARBON) && !defined(PRIVATE_USER_PATH)
# define PRIVATE_USER_PATH "~/.angband"
#endif



#endif /* !INCLUDED_CONFIG_H */
