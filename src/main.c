/**
 * \file main.c
 * \brief Core game initialisation for UNIX (and other) machines
 *
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "init.h"
#include "savefile.h"
#include "ui-birth.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-init.h"
#include "ui-input.h"
#include "ui-prefs.h"
#include "ui-signals.h"

#ifdef SOUND
#include "sound.h"
#endif

/**
 * locale junk
 */
#include <locale.h>

#if !defined(WINDOWS)
#include <langinfo.h>
#endif

/**
 * Some machines have a "main()" function in their "main-xxx.c" file,
 * all the others use this file for their "main()" function.
 */

#if defined(WIN32_CONSOLE_MODE) || !defined(WINDOWS) || defined(USE_SDL) || defined(USE_SDL2)

#include "main.h"

/*
 * On some platforms, SDL2 uses a macro to replace main() with another name
 * to hook into the platform-specific initialization.  Account for that here.
 */
#ifdef USE_SDL2
#include "SDL_main.h"
#endif

/**
 * List of the available modules in the order they are tried.
 */
static const struct module modules[] =
{
#ifdef USE_X11
	{ "x11", help_x11, init_x11 },
#endif /* USE_X11 */

#ifdef USE_SDL
	{ "sdl", help_sdl, init_sdl },
#endif /* USE_SDL */

#ifdef USE_SDL2
	{ "sdl2", help_sdl2, init_sdl2 },
#endif /* USE_SDL2 */

#ifdef USE_GCU
	{ "gcu", help_gcu, init_gcu },
#endif /* USE_GCU */

#ifdef USE_TEST
	{ "test", help_test, init_test },
#endif /* !USE_TEST */

#ifdef USE_STATS
	{ "stats", help_stats, init_stats },
#endif /* USE_STATS */

#ifdef USE_SPOIL
	{ "spoil", help_spoil, init_spoil },
#endif

#ifdef USE_IBM
	{ "ibm", help_ibm, init_ibm },
#endif /* USE_IBM */
};

/**
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(const char *s)
{
	int j;

	/* Unused parameter */
	(void)s;

	/* Scan windows */
	for (j = ANGBAND_TERM_MAX - 1; j >= 0; j--) {
		/* Unused */
		if (!angband_term[j]) continue;

		/* Nuke it */
		term_nuke(angband_term[j]);
	}
}

/**
 * Initialize and verify the file paths, and the score file.
 *
 * Use the configured DEFAULT_*_PATH constants.  Be sure to
 * set those properly when building...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed,
 * since the "init_file_paths()" function will simply append the
 * relevant "sub-directory names" to the given path.
 *
 * Make sure that the path doesn't overflow the buffer.  We have
 * to leave enough space for the path separator, directory, and
 * filenames.
 */
static void init_stuff(void)
{
	char configpath[512];
	char libpath[512];
	char datapath[512];

	/* Use the angband_path, or a default */
	my_strcpy(configpath, DEFAULT_CONFIG_PATH, sizeof(configpath));
	my_strcpy(libpath, DEFAULT_LIB_PATH, sizeof(libpath));
	my_strcpy(datapath, DEFAULT_DATA_PATH, sizeof(datapath));

	/* Make sure they're terminated */
	configpath[511] = '\0';
	libpath[511] = '\0';
	datapath[511] = '\0';

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(configpath, PATH_SEP)) my_strcat(configpath, PATH_SEP,
												 sizeof(configpath));
	if (!suffix(libpath, PATH_SEP)) my_strcat(libpath, PATH_SEP,
											  sizeof(libpath));
	if (!suffix(datapath, PATH_SEP)) my_strcat(datapath, PATH_SEP,
											   sizeof(datapath));

	/* Initialize */
	init_file_paths(configpath, libpath, datapath);
}


#ifdef SOUND
/* State shared by generic_reinit() and main(). */
static const char *soundstr = NULL;
static int saved_argc = 0;
static char **saved_argv = NULL;
#endif


/**
 * Perform (as ui-game.c's reinit_hook) platform-specific actions necessary
 * when restarting without exiting.  Also called directly at startup.
 */
static void generic_reinit(void)
{
#ifdef SOUND
	/* Initialise sound */
	init_sound(soundstr, saved_argc, saved_argv);
#endif
}


static const struct {
	const char *name;
	char **path;
	bool setgid_ok;
} change_path_values[] = {
	{ "scores", &ANGBAND_DIR_SCORES, false },
	{ "gamedata", &ANGBAND_DIR_GAMEDATA, true },
	{ "screens", &ANGBAND_DIR_SCREENS, true },
	{ "help", &ANGBAND_DIR_HELP, true },
	{ "pref", &ANGBAND_DIR_CUSTOMIZE, true },
	{ "fonts", &ANGBAND_DIR_FONTS, true },
	{ "tiles", &ANGBAND_DIR_TILES, true },
	{ "sounds", &ANGBAND_DIR_SOUNDS, true },
	{ "icons", &ANGBAND_DIR_ICONS, true },
	{ "user", &ANGBAND_DIR_USER, true },
	{ "save", &ANGBAND_DIR_SAVE, false },
	{ "panic", &ANGBAND_DIR_PANIC, false },
	{ "archive", &ANGBAND_DIR_ARCHIVE, false },
};

/**
 * Handle a "-d<dir>=<path>" option.
 *
 * Sets any of angband's special directories to <path>.
 *
 * The "<path>" can be any legal path for the given system, and should
 * not end in any special path separator (i.e. "/tmp" or "~/.ang-info").
 */
static void change_path(const char *info)
{
	char *info_copy = NULL;
	char *path = NULL;
	char *dir = NULL;
	unsigned int i = 0;
	char dirpath[512];

	if (!info || !info[0])
		quit_fmt("Try '-d<dir>=<path>'.");

	info_copy = string_make(info);
	path = strtok(info_copy, "=");
	dir = strtok(NULL, "=");

	for (i = 0; i < N_ELEMENTS(change_path_values); i++) {
		if (my_stricmp(path, change_path_values[i].name) == 0) {
#ifdef SETGID
			if (!change_path_values[i].setgid_ok)
				quit_fmt("Can't redefine path to %s dir on multiuser setup",
						 path);
#endif

			string_free(*change_path_values[i].path);
			*change_path_values[i].path = string_make(dir);

			/* the directory may not exist and may need to be created. */
			path_build(dirpath, sizeof(dirpath), dir, "");
			if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);
			string_free(info_copy);
			return;
		}
	}

	quit_fmt("Unrecognised -d parameter %s", path);
}




#ifdef UNIX

/**
 * Find a default user name from the system.
 */
static void user_name(char *buf, size_t len, int id)
{
	struct passwd *pw = getpwuid(id);

	/* Default to PLAYER */
	if (!pw || !pw->pw_name || !pw->pw_name[0] ) {
		my_strcpy(buf, "PLAYER", len);
		return;
	}

	/* Copy and capitalise */
	my_strcpy(buf, pw->pw_name, len);
	my_strcap(buf);
}

#endif /* UNIX */


/**
 * List all savefiles this player can access.
 */
static void list_saves(void)
{
	savefile_getter g = NULL;

	if (!got_savefile(&g)) {
		bool nodir = !got_savefile_dir(g);

		cleanup_savefile_getter(g);
		if (nodir) {
			quit_fmt("Cannot open savefile directory");
		}
		printf("There are no savefiles you can use.\n");
		return;
	}

	printf("Savefiles you can use are:\n");
	do {
		const struct savefile_details *details =
			get_savefile_details(g);

		if (details->desc) {
			printf(" %-15s  %s\n", details->fnam + details->foff,
				details->desc);
		} else {
			printf(" %-15s\n", details->fnam + details->foff);
		}
	} while (got_savefile(&g));
	printf("\nUse angband -u<name> to use savefile <name>.\n");

	cleanup_savefile_getter(g);
}

/**
 * Simple "main" function for multiple platforms.
 *
 * Note the special "--" option which terminates the processing of
 * standard options.  All non-standard options (if any) are passed
 * directly to the "init_xxx()" function.
 */
int main(int argc, char *argv[])
{
	int i;
	bool new_game = false, select_game = false;
	bool done = false;

	const char *mstr = NULL;
	bool args = true;

	/* Save the "program name" XXX XXX XXX */
	argv0 = argv[0];

#ifdef UNIX

	/* Default permissions on files */
	(void)umask(022);

	/* Get the user id */
	player_uid = getuid();

#endif /* UNIX */

#ifdef SETGID

	/* Save the effective GID for later recall */
	player_egid = getegid();

#endif /* UNIX */


	/* Drop permissions */
	safe_setuid_drop();

	/* Get the file paths 
	 * Paths may be overriden by -d options, so this has to occur *before* 
	 * processing command line args */
	init_stuff();

	/* Process the command line arguments */
	for (i = 1; args && (i < argc); i++) {
		const char *arg = argv[i];

		/* Require proper options */
		if (*arg++ != '-') goto usage;

		/* Analyze option */
		switch (*arg++)
		{
			case 'c':
				select_game = true;
				break;

			case 'l':
				list_saves();
				exit(0);

			case 'n':
				new_game = true;
				break;

			case 'w':
				arg_wizard = true;
				break;

			case 'g':
				/* Default graphics tile */
				/* in graphics.txt, 2 corresponds to adam bolt's tiles */
				arg_graphics = 2; 
				if (*arg) arg_graphics = atoi(arg);
				break;

			case 'u': {
				if (!*arg) goto usage;

				my_strcpy(arg_name, arg, sizeof(arg_name));

				/* The difference here is because on setgid we have to be
				 * careful to only let the player have savefiles stored in
				 * the central save directory.  Sanitising input using
				 * player_safe_name() removes anything like that.
				 *
				 * But if the player is running with per-user saves, they
				 * can do whatever the hell they want.
				 */
#ifdef SETGID
				savefile_set_name(arg, true, false);
#else
				savefile_set_name(arg, false, false);
#endif /* SETGID */

				continue;
			}

			case 'f':
				arg_force_name = true;
				break;

			case 'm':
				if (!*arg) goto usage;
				mstr = arg;
				continue;
#ifdef SOUND
			case 's':
				if (!*arg) goto usage;
				soundstr = arg;
				continue;
#endif
			case 'd':
				change_path(arg);
				continue;

			case '-':
				argv[i] = argv[0];
				argc = argc - i;
				argv = argv + i;
				args = false;
				break;

			default:
			usage:
				puts("Usage: angband [options] [-- subopts]");
				puts("  -c             Select savefile with a menu; overrides -n");
				puts("  -n             Start a new character (WARNING: overwrites default savefile without -u)");
				puts("  -l             Lists all savefiles you can play");
				puts("  -w             Resurrect dead character (marks savefile)");
				puts("  -g             Request graphics mode");
				puts("  -u<who>        Use your <who> savefile");
				puts("  -d<dir>=<path> Override a specific directory with <path>. <path> can be:");
				for (i = 0; i < (int)N_ELEMENTS(change_path_values); i++) {
#ifdef SETGID
					if (!change_path_values[i].setgid_ok) continue;
#endif
					printf("    %s (default is %s)\n", change_path_values[i].name, *change_path_values[i].path);
				}
				puts("                 Multiple -d options are allowed.");
#ifdef SOUND
				puts("  -s<mod>        Use sound module <sys>:");
				print_sound_help();
#endif
				puts("  -m<sys>        Use module <sys>, where <sys> can be:");

				/* Print the name and help for each available module */
				for (i = 0; i < (int)N_ELEMENTS(modules); i++)
					printf("     %s   %s\n",
					       modules[i].name, modules[i].help);

				/* Actually abort the process */
				quit(NULL);
		}
		if (*arg) goto usage;
	}

	/* Hack -- Forget standard args */
	if (args) {
		argc = 1;
		argv[1] = NULL;
	}

	/* Install "quit" hook */
	quit_aux = quit_hook;

	/* If we were told which mode to use, then use it */
	if (mstr)
		ANGBAND_SYS = mstr;
#if !defined(WINDOWS) && !defined(DJGPP)
	if (setlocale(LC_CTYPE, "")) {
		/* Require UTF-8 */
		if (!streq(nl_langinfo(CODESET), "UTF-8"))
			quit("Angband requires UTF-8 support");
	}
#endif

#ifdef UNIX

	/* Get the "user name" as default player name, unless set with -u switch */
	if (!arg_name[0]) {
		user_name(arg_name, sizeof(arg_name), player_uid);

		/* Sanitise name and set as savefile */
		savefile_set_name(arg_name, true, false);
	}

	/* Create any missing directories */
	create_needed_dirs();

#endif /* UNIX */

	/* Try the modules in the order specified by modules[] */
	for (i = 0; i < (int)N_ELEMENTS(modules); i++) {
		/* User requested a specific module? */
		if (!mstr || (streq(mstr, modules[i].name))) {
			ANGBAND_SYS = modules[i].name;
			if (0 == modules[i].init(argc, argv)) {
				done = true;
				break;
			}
		}
	}

	/* Make sure we have a display! */
	if (!done) quit("Unable to prepare any 'display module'!");

	/* Catch nasty signals */
	signals_init();

	/* Set up the command hook */
	cmd_get_hook = textui_get_cmd;

	/*
	 * Set action that needs to be done if restarting without exiting.
	 * Also need to do it now.
	 */
#ifdef SOUND
	saved_argc = argc;
	saved_argv = argv;
#endif
	reinit_hook = generic_reinit;
	generic_reinit();

	/* Set up the display handlers and things. */
	init_display();
	init_angband();
	textui_init();

	/* Wait for response */
	pause_line(Term);

	/* Play the game */
	play_game((select_game) ?
		GAME_SELECT : ((new_game) ? GAME_NEW : GAME_LOAD));

	/* Free resources */
	textui_cleanup();
	cleanup_angband();
#ifdef SOUND
	close_sound();
#endif

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

#endif
