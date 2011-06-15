/*
 * File: main.c
 * Purpose: Core game initialisation for UNIX (and other) machines
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
#include "files.h"
#include "init.h"

/*
 * Some machines have a "main()" function in their "main-xxx.c" file,
 * all the others use this file for their "main()" function.
 */

#if defined(WIN32_CONSOLE_MODE) || !defined(WINDOWS) || defined(USE_SDL)

#include "main.h"
#include "textui.h"
#include "init.h"

/*
 * List of the available modules in the order they are tried.
 */
static const struct module modules[] =
{
#ifdef USE_GTK
	{ "gtk", help_gtk, init_gtk },
#endif /* USE_GTK */

#ifdef USE_X11
	{ "x11", help_x11, init_x11 },
#endif /* USE_X11 */

#ifdef USE_SDL
	{ "sdl", help_sdl, init_sdl },
#endif /* USE_SDL */

#ifdef USE_GCU
	{ "gcu", help_gcu, init_gcu },
#endif /* USE_GCU */

#ifdef USE_TEST
	{ "test", help_test, init_test },
#endif /* !USE_TEST */

#ifdef USE_STATS
	{ "stats", help_stats, init_stats },
#endif /* USE_STATS */
};

static int init_sound_dummy(int argc, char *argv[]) {
	return 0;
}

/*
 * List of sound modules in the order they should be tried.
 */
static const struct module sound_modules[] =
{
#ifdef SOUND_SDL
	{ "sdl", "SDL_mixer sound module", init_sound_sdl },
#endif /* SOUND_SDL */

	{ "none", "No sound", init_sound_dummy },
};

/*
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
	for (j = ANGBAND_TERM_MAX - 1; j >= 0; j--)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Nuke it */
		term_nuke(angband_term[j]);
	}
}



/*
 * SDL needs a look-in
 */
#ifdef USE_SDL
# include "SDL.h"
#endif


/*
 * Initialize and verify the file paths, and the score file.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * DEFAULT_PATH, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the DEFAULT_PATH constants.  So be sure that one of
 * these two things works...
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
	if (!suffix(configpath, PATH_SEP)) my_strcat(configpath, PATH_SEP, sizeof(configpath));
	if (!suffix(libpath, PATH_SEP)) my_strcat(libpath, PATH_SEP, sizeof(libpath));
	if (!suffix(datapath, PATH_SEP)) my_strcat(datapath, PATH_SEP, sizeof(datapath));

	/* Initialize */
	init_file_paths(configpath, libpath, datapath);
}



/*
 * Handle a "-d<what>=<path>" option
 *
 * The "<what>" can be any string starting with the same letter as the
 * name of a subdirectory of the "lib" folder (i.e. "i" or "info").
 *
 * The "<path>" can be any legal path for the given system, and should
 * not end in any special path separator (i.e. "/tmp" or "~/.ang-info").
 */
static void change_path(const char *info)
{
	if (!info || !info[0])
		quit_fmt("Try '-d<path>'.", info);

	string_free(ANGBAND_DIR_USER);
	ANGBAND_DIR_USER = string_make(info);
}




#ifdef SET_UID

/*
 * Find a default user name from the system.
 */
static void user_name(char *buf, size_t len, int id)
{
	struct passwd *pw = getpwuid(id);

	/* Default to PLAYER */
	if (!pw)
	{
		my_strcpy(buf, "PLAYER", len);
		return;
	}

	/* Capitalise and copy */
	strnfmt(buf, len, "%^s", pw->pw_name);
}

#endif /* SET_UID */

static bool new_game;

/*
 * Pass the appropriate "Initialisation screen" command to the game,
 * getting user input if needed.
 */
static errr get_init_cmd(void)
{
	/* Wait for response */
	pause_line(Term);

	if (new_game)
		cmd_insert(CMD_NEWGAME);
	else
		/* This might be modified to supply the filename in future. */
		cmd_insert(CMD_LOADFILE);

	/* Everything's OK. */
	return 0;
}

/* Command dispatcher for curses, etc builds */
static errr default_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_INIT) 
		return get_init_cmd();
	else 
		return textui_get_cmd(context, wait);
}

static void debug_opt(const char *arg) {
	if (streq(arg, "mem-poison-alloc"))
		mem_flags |= MEM_POISON_ALLOC;
	else if (streq(arg, "mem-poison-free"))
		mem_flags |= MEM_POISON_FREE;
	else {
		puts("Debug flags:");
		puts("  mem-poison-alloc: Poison all memory allocations");
		puts("   mem-poison-free: Poison all freed memory");
		exit(0);
	}
}

/*
 * Simple "main" function for multiple platforms.
 *
 * Note the special "--" option which terminates the processing of
 * standard options.  All non-standard options (if any) are passed
 * directly to the "init_xxx()" function.
 */
int main(int argc, char *argv[])
{
	int i;

	bool done = FALSE;

	const char *mstr = NULL;
	const char *soundstr = NULL;

	bool args = TRUE;


	/* Save the "program name" XXX XXX XXX */
	argv0 = argv[0];


#ifdef SET_UID

	/* Default permissions on files */
	(void)umask(022);

#endif /* SET_UID */


#ifdef SET_UID

	/* Get the user id */
	player_uid = getuid();

	/* Save the effective GID for later recall */
	player_egid = getegid();

#endif /* SET_UID */


	/* Drop permissions */
	safe_setuid_drop();


	/* Process the command line arguments */
	for (i = 1; args && (i < argc); i++)
	{
		const char *arg = argv[i];

		/* Require proper options */
		if (*arg++ != '-') goto usage;

		/* Analyze option */
		switch (*arg++)
		{
			case 'n':
				new_game = TRUE;
				break;

			case 'w':
				arg_wizard = TRUE;
				break;

			case 'r':
				arg_rebalance = TRUE;
				break;

			case 'g':
				/* Default graphics tile */
				arg_graphics = GRAPHICS_ADAM_BOLT;
				break;

			case 'u':
				if (!*arg) goto usage;

				/* Get the savefile name */
				my_strcpy(op_ptr->full_name, arg, sizeof(op_ptr->full_name));
				continue;

			case 'm':
				if (!*arg) goto usage;
				mstr = arg;
				continue;

			case 's':
				if (!*arg) goto usage;
				soundstr = arg;
				continue;

			case 'd':
				change_path(arg);
				continue;

			case 'x':
				debug_opt(arg);
				continue;

			case '-':
				argv[i] = argv[0];
				argc = argc - i;
				argv = argv + i;
				args = FALSE;
				break;

			default:
			usage:
				puts("Usage: angband [options] [-- subopts]");
				puts("  -n             Start a new character (WARNING: overwrites default savefile without -u)");
				puts("  -w             Resurrect dead character (marks savefile)");
				puts("  -r             Rebalance monsters");
				puts("  -g             Request graphics mode");
				puts("  -x<opt>        Debug options; see -xhelp");
				puts("  -u<who>        Use your <who> savefile");
				puts("  -d<path>       Store pref files and screendumps in <path>");
				puts("  -s<mod>        Use sound module <sys>:");
				for (i = 0; i < (int)N_ELEMENTS(sound_modules); i++)
					printf("     %s   %s\n", sound_modules[i].name,
					       sound_modules[i].help);
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
	if (args)
	{
		argc = 1;
		argv[1] = NULL;
	}

	/* Install "quit" hook */
	quit_aux = quit_hook;

	/* If we were told which mode to use, then use it */
	if (mstr)
		ANGBAND_SYS = mstr;

	/* Get the file paths */
	init_stuff();

	/* Try the modules in the order specified by modules[] */
	for (i = 0; i < (int)N_ELEMENTS(modules); i++)
	{
		/* User requested a specific module? */
		if (!mstr || (streq(mstr, modules[i].name)))
		{
			ANGBAND_SYS = modules[i].name;
			if (0 == modules[i].init(argc, argv))
			{
				done = TRUE;
				break;
			}
		}
	}

	/* Make sure we have a display! */
	if (!done) quit("Unable to prepare any 'display module'!");

#ifdef SET_UID

	/* Get the "user name" as a default player name, unless set with -u switch */
	if (!op_ptr->full_name[0])
	{
		user_name(op_ptr->full_name, sizeof(op_ptr->full_name), player_uid);
	}

	/* Create any missing directories */
	create_needed_dirs();

#endif /* SET_UID */

	/* Process the player name */
	process_player_name(TRUE);

	/* Try the modules in the order specified by sound_modules[] */
	for (i = 0; i < (int)N_ELEMENTS(sound_modules); i++)
		if (!soundstr || streq(soundstr, sound_modules[i].name))
			if (0 == sound_modules[i].init(argc, argv))
				break;

	/* Catch nasty signals */
	signals_init();

	/* Set up the command hook */
	cmd_get_hook = default_get_cmd;

	/* Set up the display handlers and things. */
	init_display();

	/* Play the game */
	play_game();

	/* Free resources */
	cleanup_angband();

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

#endif
