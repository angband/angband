/* test-utils.c
 *
 * Utility functions for tests
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#include "h-basic.h"
#include "cave.h"
#include "config.h"
#include "init.h"
#include "mon-make.h"
#include "mon-util.h"
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

/*
 * Call this to initialise Angband's file paths before calling init_angband()
 * or similar.
 */
void set_file_paths(void) {
	char configpath[512], libpath[512], datapath[512];

	my_strcpy(configpath, DEFAULT_CONFIG_PATH, sizeof(configpath));
	my_strcpy(libpath, DEFAULT_LIB_PATH, sizeof(libpath));
	my_strcpy(datapath, DEFAULT_DATA_PATH, sizeof(datapath));

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

struct chunk *t_build_arena(int height, int width) {
	if (!height)
		height = z_info->dungeon_hgt;
	if (!width)
		width = z_info->dungeon_wid;
	struct chunk *c = cave_new(height, width);

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			square_set_feat(c, loc(x, y), FEAT_FLOOR);

	for (int y = 0; y < height; y++) {
		square_set_feat(c, loc(0, y), FEAT_PERM);
		square_set_feat(c, loc(width - 1, y), FEAT_PERM);
	}

	for (int x = 0; x < width; x++) {
		square_set_feat(c, loc(x, 0), FEAT_PERM);
		square_set_feat(c, loc(x, height - 1), FEAT_PERM);
	}

	return c;
}

struct monster *t_add_monster(struct chunk *c, struct loc g, const char *race) {
	struct monster_race *r = lookup_monster(race);
	struct monster_group_info info = { 0, 0 };
	place_new_monster(c, g, r, false, false, info, ORIGIN_DROP);
	struct monster *m = square_monster(c, g);
	assert(m);
	return m;
}
