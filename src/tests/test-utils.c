/* test-utils.c
 *
 * Utility functions for tests
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#include "h-basic.h"
#include "config.h"
#include "init.h"
#include "test-utils.h"
#include "z-util.h"

#if defined(SOUND_SDL) || defined(SOUND_SDL2)
#include "sound.h"
#include "snd-sdl.h"

errr init_sound_sdl(struct sound_hooks *hooks, int argc, char **argv)
{
	return (0);
}

#endif

#ifndef SRCROOT
#error Unit tests must be built with SRCROOT set to the directory above src
#endif

/*
 * Call this to initialise Angband's file paths before calling init_angband()
 * or similar.
 */
void set_file_paths(void) {
	char configpath[512], libpath[512], datapath[512];

	my_strcpy(configpath, SRCROOT PATH_SEP "lib", sizeof(configpath));
	my_strcpy(libpath, SRCROOT PATH_SEP "lib", sizeof(libpath));
	my_strcpy(datapath, SRCROOT PATH_SEP "lib", sizeof(datapath));

	configpath[511] = libpath[511] = datapath[511] = '\0';

	if (!suffix(configpath, PATH_SEP))
		my_strcat(configpath, PATH_SEP, sizeof(configpath));
	if (!suffix(libpath, PATH_SEP))
		my_strcat(libpath, PATH_SEP, sizeof(libpath));
	if (!suffix(datapath, PATH_SEP))
		my_strcat(datapath, PATH_SEP, sizeof(datapath));

	init_file_paths(configpath, libpath, datapath);
}

/*
 * Call this function to simulate init_stuff() and populate the *_info arrays
 */
void read_edit_files(void) {
	set_file_paths();
	init_game_constants();
	init_arrays();
}
