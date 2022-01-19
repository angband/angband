/**
 * \file init.c
 * \brief Various game initialization routines
 *
 * Copyright (c) 1997 Ben Harrison
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
 *
 * This file is used to initialize various variables and arrays for the
 * Angband game.
 *
 * Several of the arrays for Angband are built from data files in the
 * "lib/gamedata" directory.
 */


#include "angband.h"
#include "buildid.h"
#include "cave.h"
#include "cmds.h"
#include "cmd-core.h"
#include "datafile.h"
#include "effects.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "hint.h"
#include "init.h"
#include "mon-init.h"
#include "mon-list.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-summon.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-chest.h"
#include "obj-ignore.h"
#include "obj-init.h"
#include "obj-list.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-randart.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "option.h"
#include "player.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-spell.h"
#include "player-timed.h"
#include "project.h"
#include "randname.h"
#include "store.h"
#include "trap.h"
#include "ui-entry.h"
#include "ui-entry-init.h"
#include "ui-visuals.h"

bool play_again = false;

/**
 * Structure (not array) of game constants
 */
struct angband_constants *z_info;

/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
const char *ANGBAND_SYS = "xxx";

/**
 * Various directories. These are no longer necessarily all subdirs of "lib"
 */
char *ANGBAND_DIR_GAMEDATA;
char *ANGBAND_DIR_CUSTOMIZE;
char *ANGBAND_DIR_HELP;
char *ANGBAND_DIR_SCREENS;
char *ANGBAND_DIR_FONTS;
char *ANGBAND_DIR_TILES;
char *ANGBAND_DIR_SOUNDS;
char *ANGBAND_DIR_ICONS;
char *ANGBAND_DIR_USER;
char *ANGBAND_DIR_SAVE;
char *ANGBAND_DIR_PANIC;
char *ANGBAND_DIR_SCORES;
char *ANGBAND_DIR_INFO;
char *ANGBAND_DIR_ARCHIVE;

static const char *slots[] = {
	#define EQUIP(a, b, c, d, e, f) #a,
	#include "list-equip-slots.h"
	#undef EQUIP
	NULL
};

const char *list_obj_flag_names[] = {
	"NONE",
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

static const char *obj_mods[] = {
	#define STAT(a) #a,
	#include "list-stats.h"
	#undef STAT
	#define OBJ_MOD(a) #a,
	#include "list-object-modifiers.h"
	#undef OBJ_MOD
	NULL
};

const char *list_element_names[] = {
	#define ELEM(a) #a,
	#include "list-elements.h"
	#undef ELEM
	NULL
};

static const char *effect_list[] = {
	"NONE",
	#define EFFECT(x, a, b, c, d, e, f) #x,
	#include "list-effects.h"
	#undef EFFECT
	"MAX"
};

static const char *trap_flags[] =
{
	#define TRF(a, b) #a,
	#include "list-trap-flags.h"
	#undef TRF
    NULL
};

static const char *terrain_flags[] =
{
	#define TF(a, b) #a,
	#include "list-terrain-flags.h"
	#undef TF
    NULL
};

static const char *mon_race_flags[] =
{
	#define RF(a, b, c) #a,
	#include "list-mon-race-flags.h"
	#undef RF
	NULL
};

static const char *player_info_flags[] =
{
	#define PF(a) #a,
	#include "list-player-flags.h"
	#undef PF
	NULL
};

errr grab_effect_data(struct parser *p, struct effect *effect)
{
	const char *type;
	int val;

	if (grab_name("effect", parser_getsym(p, "eff"), effect_list,
				  N_ELEMENTS(effect_list), &val))
		return PARSE_ERROR_INVALID_EFFECT;
	effect->index = val;

	if (parser_hasval(p, "type")) {
		type = parser_getsym(p, "type");

		if (type == NULL)
			return PARSE_ERROR_UNRECOGNISED_PARAMETER;

		/* Check for a value */
		val = effect_subtype(effect->index, type);
		if (val < 0)
			return PARSE_ERROR_INVALID_VALUE;
		else
			effect->subtype = val;
	}

	if (parser_hasval(p, "radius"))
		effect->radius = parser_getint(p, "radius");

	if (parser_hasval(p, "other"))
		effect->other = parser_getint(p, "other");

	return PARSE_ERROR_NONE;
}

static enum parser_error write_book_kind(struct class_book *book,
										 const char *name)
{
	struct object_kind *temp, *kind;
	int i;

	/* Check we haven't already made this book */
	for (i = 0; i < z_info->k_max; i++) {
		if (k_info[i].name && streq(name, k_info[i].name)) {
			book->sval = k_info[i].sval;
			return PARSE_ERROR_NONE;
		}
	}

	/* Extend by 1 and realloc */
	z_info->k_max += 1;
	z_info->ordinary_kind_max += 1;
	temp = mem_realloc(k_info, (z_info->k_max + 1) * sizeof(*temp));

	/* Copy if no errors */
	if (!temp) {
		return PARSE_ERROR_INTERNAL;
	} else {
		k_info = temp;
	}

	/* Add this entry at the end */
	kind = &k_info[z_info->k_max - 1];
	memset(kind, 0, sizeof(*kind));

	/* Copy the tval and base */
	kind->tval = book->tval;
	kind->base = &kb_info[kind->tval];

	/* Make the name and index */
	kind->name = string_make(name);
	kind->kidx = z_info->k_max - 1;

	/* Increase the sval count for this tval, set the new one to the max */
	for (i = 0; i < TV_MAX; i++)
		if (kb_info[i].tval == kind->tval) {
			kb_info[i].num_svals++;
			kind->sval = kb_info[i].num_svals;
			break;
		}
	if (i == TV_MAX) return PARSE_ERROR_INTERNAL;

	/* Copy the sval to the artifact info */
	book->sval = kind->sval;

	/* Set object defaults (graphics should be overwritten) */
	kind->d_char = '*';
	kind->d_attr = COLOUR_RED;
	kind->dd = 1;
	kind->ds = 1;
	kind->weight = 30;

	/* Inherit base flags. */
	kf_union(kind->kind_flags, kb_info[kind->tval].kind_flags);

	/* Dungeon books get extra properties */
	if (book->dungeon) {
		for (i = ELEM_BASE_MIN; i < ELEM_BASE_MAX; i++) {
			kind->el_info[i].flags |= EL_INFO_IGNORE;
		}
		kf_on(kind->kind_flags, KF_GOOD);
	}

	return PARSE_ERROR_NONE;
}

/**
 * Find the default paths to all of our important sub-directories.
 *
 * All of the sub-directories should, for a single-user install, be
 * located inside the main directory, whose location is very system-dependent.
 * For shared installations, typically on Unix or Linux systems, the
 * directories may be scattered - see config.h for more info.
 *
 * This function takes buffers, holding the paths to the "config", "lib",
 * and "data" directories (for example, those could be "/etc/angband/",
 * "/usr/share/angband", and "/var/games/angband").  Some system-dependent
 * expansion/substitution may be done when copying those base paths to the
 * paths Angband uses:  see path_process() in z-file.c for details (Unix
 * implementations, for instance, try to replace a leading ~ or ~username with
 * the path to a home directory).
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "scores" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 */
void init_file_paths(const char *configpath, const char *libpath, const char *datapath)
{
	char buf[1024];
	char *userpath = NULL;

	/*** Free everything ***/

	/* Free the sub-paths */
	string_free(ANGBAND_DIR_GAMEDATA);
	string_free(ANGBAND_DIR_CUSTOMIZE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_SCREENS);
	string_free(ANGBAND_DIR_FONTS);
	string_free(ANGBAND_DIR_TILES);
	string_free(ANGBAND_DIR_SOUNDS);
	string_free(ANGBAND_DIR_ICONS);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PANIC);
	string_free(ANGBAND_DIR_SCORES);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_ARCHIVE);

	/*** Prepare the paths ***/

#define BUILD_DIRECTORY_PATH(dest, basepath, dirname) { \
	path_build(buf, sizeof(buf), (basepath), (dirname)); \
	dest = string_make(buf); \
}

	/* Paths generally containing configuration data for Angband. */
#ifdef GAMEDATA_IN_LIB
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_GAMEDATA, libpath, "gamedata");
#else
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_GAMEDATA, configpath, "gamedata");
#endif
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_CUSTOMIZE, configpath, "customize");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_HELP, libpath, "help");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_SCREENS, libpath, "screens");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_FONTS, libpath, "fonts");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_TILES, libpath, "tiles");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_SOUNDS, libpath, "sounds");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_ICONS, libpath, "icons");

#ifdef PRIVATE_USER_PATH

	/* Build the path to the user specific directory */
	if (strncmp(ANGBAND_SYS, "test", 4) == 0)
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, "Test");
	else
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, VERSION_NAME);
	ANGBAND_DIR_USER = string_make(buf);

#else /* !PRIVATE_USER_PATH */

#ifdef MACH_O_CARBON
	/* Remove any trailing separators, since some deeper path creation functions
	 * don't like directories with trailing slashes. */
	if (suffix(datapath, PATH_SEP)) {
		/* Hacky way to trim the separator. Since this is just for OS X, we can
		 * assume a one char separator. */
		int last_char_index = strlen(datapath) - 1;
		my_strcpy(buf, datapath, sizeof(buf));
		buf[last_char_index] = '\0';
		ANGBAND_DIR_USER = string_make(buf);
	}
	else {
		ANGBAND_DIR_USER = string_make(datapath);
	}
#else /* !MACH_O_CARBON */
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_USER, datapath, "user");
#endif /* MACH_O_CARBON */

#endif /* PRIVATE_USER_PATH */

	/* Build the path to the user info directory */
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_INFO, ANGBAND_DIR_USER, "info");

	/* Build the path to the archive directory. */
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_ARCHIVE, ANGBAND_DIR_USER, "archive");

#ifdef USE_PRIVATE_PATHS
	userpath = ANGBAND_DIR_USER;
#else /* !USE_PRIVATE_PATHS */
	userpath = (char *)datapath;
#endif /* USE_PRIVATE_PATHS */

	/* Build the path to the score and directories */
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_SCORES, userpath, "scores");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_SAVE, userpath, "save");
	BUILD_DIRECTORY_PATH(ANGBAND_DIR_PANIC, userpath, "panic");

#undef BUILD_DIRECTORY_PATH
}


/**
 * Create any missing directories. We create only those dirs which may be
 * empty (user/, save/, scores/, info/, help/). The others are assumed 
 * to contain required files and therefore must exist at startup 
 * (edit/, pref/, file/, xtra/).
 *
 * ToDo: Only create the directories when actually writing files.
 */
void create_needed_dirs(void)
{
	char dirpath[512];

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_USER, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_SAVE, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_PANIC, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_SCORES, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_INFO, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_ARCHIVE, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

}

/**
 * ------------------------------------------------------------------------
 * Initialize game constants
 * ------------------------------------------------------------------------ */

static enum parser_error parse_constants_level_max(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "monsters"))
		z->level_monster_max = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_mon_gen(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "chance"))
		z->alloc_monster_chance = value;
	else if (streq(label, "level-min"))
		z->level_monster_min = value;
	else if (streq(label, "town-day"))
		z->town_monsters_day = value;
	else if (streq(label, "town-night"))
		z->town_monsters_night = value;
	else if (streq(label, "repro-max"))
		z->repro_monster_max = value;
	else if (streq(label, "ood-chance"))
		z->ood_monster_chance = value;
	else if (streq(label, "ood-amount"))
		z->ood_monster_amount = value;
	else if (streq(label, "group-max"))
		z->monster_group_max = value;
	else if (streq(label, "group-dist"))
		z->monster_group_dist = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_mon_play(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "break-glyph"))
		z->glyph_hardness = value;
	else if (streq(label, "mult-rate"))
		z->repro_monster_rate = value;
	else if (streq(label, "life-drain"))
		z->life_drain_percent = value;
	else if (streq(label, "flee-range"))
		z->flee_range = value;
	else if (streq(label, "turn-range"))
		z->turn_range = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_dun_gen(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "cent-max"))
		z->level_room_max = value;
	else if (streq(label, "door-max"))
		z->level_door_max = value;
	else if (streq(label, "wall-max"))
		z->wall_pierce_max = value;
	else if (streq(label, "tunn-max"))
		z->tunn_grid_max = value;
	else if (streq(label, "amt-room"))
		z->room_item_av = value;
	else if (streq(label, "amt-item"))
		z->both_item_av = value;
	else if (streq(label, "amt-gold"))
		z->both_gold_av = value;
	else if (streq(label, "pit-max"))
		z->level_pit_max = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_world(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "max-depth"))
		z->max_depth = value;
	else if (streq(label, "day-length"))
		z->day_length = value;
	else if (streq(label, "dungeon-hgt"))
		z->dungeon_hgt = value;
	else if (streq(label, "dungeon-wid"))
		z->dungeon_wid = value;
	else if (streq(label, "town-hgt"))
		z->town_hgt = value;
	else if (streq(label, "town-wid"))
		z->town_wid = value;
	else if (streq(label, "feeling-total"))
		z->feeling_total = value;
	else if (streq(label, "feeling-need"))
		z->feeling_need = value;
	else if (streq(label, "stair-skip"))
		z->stair_skip = value;
	else if (streq(label, "move-energy"))
		z->move_energy = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_carry_cap(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "pack-size"))
		z->pack_size = value;
	else if (streq(label, "quiver-size"))
		z->quiver_size = value;
	else if (streq(label, "quiver-slot-size"))
		z->quiver_slot_size = value;
	else if (streq(label, "thrown-quiver-mult"))
		z->thrown_quiver_mult = value;
	else if (streq(label, "floor-size"))
		z->floor_size = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_store(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "inven-max"))
		z->store_inven_max = value;
	else if (streq(label, "turns"))
		z->store_turns = value;
	else if (streq(label, "shuffle"))
		z->store_shuffle = value;
	else if (streq(label, "magic-level"))
		z->store_magic_level = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_obj_make(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "max-depth"))
		z->max_obj_depth = value;
	else if (streq(label, "great-obj"))
		z->great_obj = value;
	else if (streq(label, "great-ego"))
		z->great_ego = value;
	else if (streq(label, "fuel-torch"))
		z->fuel_torch = value;
	else if (streq(label, "fuel-lamp"))
		z->fuel_lamp = value;
	else if (streq(label, "default-lamp"))
		z->default_lamp = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_constants_player(struct parser *p) {
	struct angband_constants *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "max-sight"))
		z->max_sight = value;
	else if (streq(label, "max-range"))
		z->max_range = value;
	else if (streq(label, "start-gold"))
		z->start_gold = value;
	else if (streq(label, "food-value"))
		z->food_value = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

struct parser *init_parse_constants(void) {
	struct angband_constants *z = mem_zalloc(sizeof *z);
	struct parser *p = parser_new();

	parser_setpriv(p, z);
	parser_reg(p, "level-max sym label int value", parse_constants_level_max);
	parser_reg(p, "mon-gen sym label int value", parse_constants_mon_gen);
	parser_reg(p, "mon-play sym label int value", parse_constants_mon_play);
	parser_reg(p, "dun-gen sym label int value", parse_constants_dun_gen);
	parser_reg(p, "world sym label int value", parse_constants_world);
	parser_reg(p, "carry-cap sym label int value", parse_constants_carry_cap);
	parser_reg(p, "store sym label int value", parse_constants_store);
	parser_reg(p, "obj-make sym label int value", parse_constants_obj_make);
	parser_reg(p, "player sym label int value", parse_constants_player);
	return p;
}

static errr run_parse_constants(struct parser *p) {
	return parse_file_quit_not_found(p, "constants");
}

static errr finish_parse_constants(struct parser *p) {
	z_info = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_constants(void)
{
	mem_free(z_info);
}

static struct file_parser constants_parser = {
	"constants",
	init_parse_constants,
	run_parse_constants,
	finish_parse_constants,
	cleanup_constants
};

/**
 * Initialize game constants.
 *
 * Assumption: Paths are set up correctly before calling this function.
 */
void init_game_constants(void)
{
	event_signal_message(EVENT_INITSTATUS, 0, "Initializing constants");
	if (run_parser(&constants_parser))
		quit_fmt("Cannot initialize constants.");
}

/**
 * Free the game constants
 */
static void cleanup_game_constants(void)
{
	cleanup_parser(&constants_parser);
}

/**
 * ------------------------------------------------------------------------
 * Intialize world map
 * ------------------------------------------------------------------------ */
static enum parser_error parse_world_level(struct parser *p) {
	const int depth = parser_getint(p, "depth");
	const char *name = parser_getsym(p, "name");
	const char *up = parser_getsym(p, "up");
	const char *down = parser_getsym(p, "down");
	struct level *last = parser_priv(p);
	struct level *lev = mem_zalloc(sizeof *lev);

	if (last) {
		last->next = lev;
	} else {
		world = lev;
	}
	lev->depth = depth;
	lev->name = string_make(name);
	lev->up = streq(up, "None") ? NULL : string_make(up);
	lev->down = streq(down, "None") ? NULL : string_make(down);
	parser_setpriv(p, lev);
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_world(void) {
	struct parser *p = parser_new();

	parser_reg(p, "level int depth sym name sym up sym down",
			   parse_world_level);
	return p;
}

static errr run_parse_world(struct parser *p) {
	return parse_file_quit_not_found(p, "world");
}

static errr finish_parse_world(struct parser *p) {
	struct level *level_check;

	/* Check that all levels referred to exist */
	for (level_check = world; level_check; level_check = level_check->next) {
		struct level *level_find = world;

		/* Check upwards */
		if (level_check->up) {
			while (level_find && !streq(level_check->up, level_find->name)) {
				level_find = level_find->next;
			}
			if (!level_find) {
				quit_fmt("Invalid level reference %s", level_check->up);
			}
		}

		/* Check downwards */
		level_find = world;
		if (level_check->down) {
			while (level_find && !streq(level_check->down, level_find->name)) {
				level_find = level_find->next;
			}
			if (!level_find) {
				quit_fmt("Invalid level reference %s", level_check->down);
			}
		}
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_world(void)
{
	struct level *level = world;
	while (level) {
		struct level *old = level;
		string_free(level->name);
		string_free(level->up);
		string_free(level->down);
		level = level->next;
		mem_free(old);
	}
}

static struct file_parser world_parser = {
	"world",
	init_parse_world,
	run_parse_world,
	finish_parse_world,
	cleanup_world
};


/**
 * ------------------------------------------------------------------------
 * Initialize player properties
 * ------------------------------------------------------------------------ */
/*
 * Keep track of UI entries to be bound to an ability while parsing.  Bind them
 * at the end of parsing and don't pass them along to the stored player_ability
 * structures.
 */
struct player_bound_ui {
	char *name;
	struct player_bound_ui *next;
	int value;
	bool isaux;
	bool isspecial;
};
struct embryo_player_ability {
	struct player_ability ability;
	struct player_bound_ui *boundui;
	struct embryo_player_ability *next;
};
static struct embryo_player_ability  *embryo_player_abilities = NULL;

static enum parser_error parse_player_prop_type(struct parser *p) {
	const char *type = parser_getstr(p, "type");
	struct embryo_player_ability *h = parser_priv(p);
	struct embryo_player_ability *embryo = mem_zalloc(sizeof *embryo);

	if (h) {
		h->next = embryo;
	} else {
		embryo_player_abilities = embryo;
	}
	parser_setpriv(p, embryo);
	embryo->ability.type = string_make(type);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_prop_code(struct parser *p) {
	const char *code = parser_getstr(p, "code");
	struct embryo_player_ability *embryo = parser_priv(p);
	int index = -1;

	if (!embryo)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!embryo->ability.type)
		return PARSE_ERROR_MISSING_PLAY_PROP_TYPE;

	if (streq(embryo->ability.type, "player")) {
		index = code_index_in_array(player_info_flags, code);
	} else if (streq(embryo->ability.type, "object")) {
		index = code_index_in_array(list_obj_flag_names, code);
	}
	if (index >= 0) {
		embryo->ability.index = index;
	} else {
		return PARSE_ERROR_INVALID_PLAY_PROP_CODE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_prop_desc(struct parser *p) {
	struct embryo_player_ability *embryo = parser_priv(p);
	if (!embryo)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	embryo->ability.desc = string_append(embryo->ability.desc, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_prop_name(struct parser *p) {
	const char *desc = parser_getstr(p, "desc");
	struct embryo_player_ability *embryo = parser_priv(p);
	if (!embryo)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	embryo->ability.name = string_make(desc);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_prop_value(struct parser *p) {
	struct embryo_player_ability *embryo = parser_priv(p);
	if (!embryo)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	embryo->ability.value = parser_getint(p, "value");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_prop_bindui(struct parser *p) {
	const char *name = parser_getsym(p, "ui");
	const char *value = parser_getsym(p, "uival");
	bool isaux = (parser_getint(p, "aux") != 0);
	struct embryo_player_ability *embryo = parser_priv(p);
	struct player_bound_ui *boundui;
	if (!embryo)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	boundui = mem_alloc(sizeof(*boundui));
	boundui->name = string_make(name);
	if (streq(value, "special")) {
		boundui->value = 0;
		boundui->isspecial = true;
	} else {
		long v;
		char* end;

		v = strtol(value, &end, 10);
		if (! *value || *end) {
			string_free(boundui->name);
			mem_free(boundui);
			return PARSE_ERROR_NOT_NUMBER;
		}
		if (v < INT_MIN || v > INT_MAX) {
			string_free(boundui->name);
			mem_free(boundui);
			return PARSE_ERROR_INVALID_VALUE;
		}
		boundui->value = (int) v;
		boundui->isspecial = false;
	}
	boundui->isaux = isaux;
	boundui->next = embryo->boundui;
	embryo->boundui = boundui;
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_player_prop(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "type str type", parse_player_prop_type);
	parser_reg(p, "code str code", parse_player_prop_code);
	parser_reg(p, "desc str desc", parse_player_prop_desc);
	parser_reg(p, "name str desc", parse_player_prop_name);
	parser_reg(p, "value int value", parse_player_prop_value);
	parser_reg(p, "bindui sym ui int aux sym uival", parse_player_prop_bindui);
	return p;
}

static errr run_parse_player_prop(struct parser *p) {
	return parse_file_quit_not_found(p, "player_property");
}

static errr finish_parse_player_prop(struct parser *p) {
	struct embryo_player_ability *embryo = embryo_player_abilities;
	struct embryo_player_ability *target;
	struct player_bound_ui *boundui_cursor;
	struct player_ability *new, *previous = NULL;

	embryo_player_abilities = NULL;
	/* Copy abilities over, making multiple copies for element types */
	player_abilities = mem_zalloc(sizeof(*player_abilities));
	new = player_abilities;
	while (embryo) {
		if (streq(embryo->ability.type, "element")) {
			uint16_t i, n;
			assert(N_ELEMENTS(list_element_names) < 65536);
			n = (uint16_t) N_ELEMENTS(list_element_names);
			for (i = 0; i < n - 1; i++) {
				char *name = projections[i].name;
				new->index = i;
				new->type = string_make(embryo->ability.type);
				new->desc = string_make(format("%s %s.", embryo->ability.desc, name));
				my_strcap(name);
				new->name = string_make(format("%s %s", name, embryo->ability.name));
				new->value = embryo->ability.value;
				boundui_cursor = embryo->boundui;
				while (boundui_cursor) {
					name = string_make(format("%s<%s>", boundui_cursor->name, list_element_names[i]));
					(void) bind_player_ability_to_ui_entry_by_name(name, new, boundui_cursor->value, !boundui_cursor->isspecial, boundui_cursor->isaux);
					string_free(name);
					boundui_cursor = boundui_cursor->next;
				}
				if ((i != n - 2) || embryo->next) {
					previous = new;
					new = mem_zalloc(sizeof(*new));
					previous->next = new;
				}
			}
			string_free(embryo->ability.type);
			string_free(embryo->ability.desc);
			string_free(embryo->ability.name);
			while (embryo->boundui) {
				boundui_cursor = embryo->boundui;
				embryo->boundui = embryo->boundui->next;
				string_free(boundui_cursor->name);
				mem_free(boundui_cursor);
			}
		} else {
			new->type = embryo->ability.type;
			new->index = embryo->ability.index;
			new->desc = embryo->ability.desc;
			new->name = embryo->ability.name;
			while (embryo->boundui) {
				boundui_cursor = embryo->boundui;
				embryo->boundui = embryo->boundui->next;
				(void) bind_player_ability_to_ui_entry_by_name(boundui_cursor->name, new, boundui_cursor->value, !boundui_cursor->isspecial, boundui_cursor->isaux);
				string_free(boundui_cursor->name);
				mem_free(boundui_cursor);
			}
			if (embryo->next) {
				previous = new;
				new = mem_zalloc(sizeof(*new));
				previous->next = new;
			}
		}
		target = embryo;
		embryo = embryo->next;
		mem_free(target);
	}
	parser_destroy(p);
	return 0;
}

static void cleanup_player_prop(void)
{
	struct player_ability *ability = player_abilities;
	while (ability) {
		struct player_ability *totrash = ability;
		ability = ability->next;
		string_free(totrash->type);
		string_free(totrash->desc);
		string_free(totrash->name);
		mem_free(totrash);
	}
}

static struct file_parser player_property_parser = {
	"player_property",
	init_parse_player_prop,
	run_parse_player_prop,
	finish_parse_player_prop,
	cleanup_player_prop
};

/**
 * ------------------------------------------------------------------------
 * Intialize random names
 * ------------------------------------------------------------------------ */

struct name {
	struct name *next;
	char *str;
};

struct names_parse {
	unsigned int section;
	unsigned int nnames[RANDNAME_NUM_TYPES];
	struct name *names[RANDNAME_NUM_TYPES];
};

static enum parser_error parse_names_section(struct parser *p) {
	unsigned int section = parser_getint(p, "section");
	struct names_parse *s = parser_priv(p);
	if (s->section >= RANDNAME_NUM_TYPES)
		return PARSE_ERROR_OUT_OF_BOUNDS;
	s->section = section;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_names_word(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct names_parse *s = parser_priv(p);
	struct name *ns = mem_zalloc(sizeof *ns);

	s->nnames[s->section]++;
	ns->next = s->names[s->section];
	ns->str = string_make(name);
	s->names[s->section] = ns;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_names(void) {
	struct parser *p = parser_new();
	struct names_parse *n = mem_zalloc(sizeof *n);
	n->section = 0;
	parser_setpriv(p, n);
	parser_reg(p, "section int section", parse_names_section);
	parser_reg(p, "word str name", parse_names_word);
	return p;
}

static errr run_parse_names(struct parser *p) {
	return parse_file_quit_not_found(p, "names");
}

static errr finish_parse_names(struct parser *p) {
	int i;
	unsigned int j;
	struct names_parse *n = parser_priv(p);
	struct name *nm;
	name_sections = mem_zalloc(sizeof(char**) * RANDNAME_NUM_TYPES);
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		name_sections[i] = mem_alloc(sizeof(char*) * (n->nnames[i] + 1));
		for (nm = n->names[i], j = 0; nm && j < n->nnames[i]; nm = nm->next, j++) {
			name_sections[i][j] = nm->str;
		}
		name_sections[i][n->nnames[i]] = NULL;
		while (n->names[i]) {
			nm = n->names[i]->next;
			mem_free(n->names[i]);
			n->names[i] = nm;
		}
	}
	mem_free(n);
	parser_destroy(p);
	return 0;
}

static void cleanup_names(void)
{
	int i, j;
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		for (j = 0; name_sections[i][j]; j++) {
			string_free((char *)name_sections[i][j]);
		}
		mem_free(name_sections[i]);
	}
	mem_free(name_sections);
}

static struct file_parser names_parser = {
	"names",
	init_parse_names,
	run_parse_names,
	finish_parse_names,
	cleanup_names
};

/**
 * ------------------------------------------------------------------------
 * Intialize traps
 * ------------------------------------------------------------------------ */

static enum parser_error parse_trap_name(struct parser *p) {
    const char *name = parser_getsym(p, "name");
    const char *desc = parser_getstr(p, "desc");
    struct trap_kind *h = parser_priv(p);

    struct trap_kind *t = mem_zalloc(sizeof *t);
    t->next = h;
    t->name = string_make(name);
	t->desc = string_make(desc);
    parser_setpriv(p, t);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_graphics(struct parser *p) {
    wchar_t glyph = parser_getchar(p, "glyph");
    const char *color = parser_getsym(p, "color");
    int attr = 0;
    struct trap_kind *t = parser_priv(p);

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->d_char = glyph;
    if (strlen(color) > 1)
		attr = color_text_to_attr(color);
    else
		attr = color_char_to_attr(color[0]);
    if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
    t->d_attr = attr;
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_appear(struct parser *p) {
    struct trap_kind *t = parser_priv(p);

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->rarity =  parser_getuint(p, "rarity");
    t->min_depth =  parser_getuint(p, "mindepth");
    t->max_num =  parser_getuint(p, "maxnum");
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_visibility(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
	char *s;
	dice_t *dice;

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Get a rewritable string */
	s = string_make(parser_getstr(p, "visibility"));

	dice = dice_new();
	if (!dice_parse_string(dice, s)) {
		string_free(s);
		dice_free(dice);
		return PARSE_ERROR_NOT_RANDOM;
	}
	dice_random_value(dice, &t->power);

	string_free(s);
	dice_free(dice);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_flags(struct parser *p) {
    char *flags;
    struct trap_kind *t = parser_priv(p);
    char *s;

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

    if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
    flags = string_make(parser_getstr(p, "flags"));

    s = strtok(flags, " |");
    while (s) {
		if (grab_flag(t->flags, TRF_SIZE, trap_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
    }

    mem_free(flags);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_effect(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (t->effect) {
		effect = t->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		t->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_trap_effect_yx(struct parser *p) {
	struct trap_kind *t = parser_priv(p);
	struct effect *effect = t->effect;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_dice(struct parser *p) {
	struct trap_kind *t = parser_priv(p);
	dice_t *dice = NULL;
	struct effect *effect = t->effect;
	const char *string = NULL;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_expr(struct parser *p) {
	struct trap_kind *t = parser_priv(p);
	struct effect *effect = t->effect;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL)
		return PARSE_ERROR_NONE;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = effect_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_effect_xtra(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (t->effect_xtra) {
		effect = t->effect_xtra;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		t->effect_xtra = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_trap_effect_yx_xtra(struct parser *p) {
	struct trap_kind *t = parser_priv(p);
	struct effect *effect = t->effect;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_dice_xtra(struct parser *p) {
	struct trap_kind *t = parser_priv(p);
	dice_t *dice = NULL;
	struct effect *effect = t->effect_xtra;
	const char *string = NULL;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_expr_xtra(struct parser *p) {
	struct trap_kind *t = parser_priv(p);
	struct effect *effect = t->effect_xtra;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL)
		return PARSE_ERROR_NONE;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = effect_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_save_flags(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
	char *s = string_make(parser_getstr(p, "flags"));
	char *u;
	assert(t);

	u = strtok(s, " |");
	while (u) {
		bool found = false;
		if (!grab_flag(t->save_flags, OF_SIZE, list_obj_flag_names, u))
			found = true;
		if (!found)
			break;
		u = strtok(NULL, " |");
	}
	mem_free(s);
	return u ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_desc(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
    assert(t);

    t->text = string_append(t->text, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_msg(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
    assert(t);

    t->msg = string_append(t->msg, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_msg_good(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
    assert(t);

    t->msg_good = string_append(t->msg_good, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_msg_bad(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
    assert(t);

    t->msg_bad = string_append(t->msg_bad, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_msg_xtra(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
    assert(t);

    t->msg_xtra = string_append(t->msg_xtra, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

struct parser *init_parse_trap(void) {
    struct parser *p = parser_new();
    parser_setpriv(p, NULL);
    parser_reg(p, "name sym name str desc", parse_trap_name);
    parser_reg(p, "graphics char glyph sym color", parse_trap_graphics);
    parser_reg(p, "appear uint rarity uint mindepth uint maxnum", parse_trap_appear);
    parser_reg(p, "visibility str visibility", parse_trap_visibility);
    parser_reg(p, "flags ?str flags", parse_trap_flags);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_trap_effect);
	parser_reg(p, "effect-yx int y int x", parse_trap_effect_yx);
	parser_reg(p, "dice str dice", parse_trap_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_trap_expr);
	parser_reg(p, "effect-xtra sym eff ?sym type ?int radius ?int other", parse_trap_effect_xtra);
	parser_reg(p, "effect-yx-xtra int y int x", parse_trap_effect_yx_xtra);
	parser_reg(p, "dice-xtra str dice", parse_trap_dice_xtra);
	parser_reg(p, "expr-xtra sym name sym base str expr", parse_trap_expr_xtra);
	parser_reg(p, "save str flags", parse_trap_save_flags);
	parser_reg(p, "desc str text", parse_trap_desc);
	parser_reg(p, "msg str text", parse_trap_msg);
	parser_reg(p, "msg-good str text", parse_trap_msg_good);
	parser_reg(p, "msg-bad str text", parse_trap_msg_bad);
	parser_reg(p, "msg-xtra str text", parse_trap_msg_xtra);
    return p;
}

static errr run_parse_trap(struct parser *p) {
    return parse_file_quit_not_found(p, "trap");
}

static errr finish_parse_trap(struct parser *p) {
	struct trap_kind *t, *n;
	int tidx;
	
	/* Scan the list for the max id */
	z_info->trap_max = 0;
	t = parser_priv(p);
	while (t) {
		z_info->trap_max++;
		t = t->next;
	}

	trap_info = mem_zalloc((z_info->trap_max + 1) * sizeof(*t));
	tidx = z_info->trap_max - 1;
    for (t = parser_priv(p); t; t = t->next, tidx--) {
		assert(tidx >= 0);

		memcpy(&trap_info[tidx], t, sizeof(*t));
		trap_info[tidx].tidx = tidx;
		if (tidx < z_info->trap_max - 1)
			trap_info[tidx].next = &trap_info[tidx + 1];
		else
			trap_info[tidx].next = NULL;
    }

    t = parser_priv(p);
    while (t) {
		n = t->next;
		mem_free(t);
		t = n;
    }

    parser_destroy(p);
    return 0;
}

static void cleanup_trap(void)
{
	int i;
	for (i = 0; i < z_info->trap_max; i++) {
		string_free(trap_info[i].name);
		mem_free(trap_info[i].text);
		string_free(trap_info[i].desc);
		string_free(trap_info[i].msg);
		string_free(trap_info[i].msg_good);
		string_free(trap_info[i].msg_bad);
		string_free(trap_info[i].msg_xtra);
		free_effect(trap_info[i].effect);
		free_effect(trap_info[i].effect_xtra);
	}
	mem_free(trap_info);
}

static struct file_parser trap_parser = {
    "trap",
    init_parse_trap,
    run_parse_trap,
    finish_parse_trap,
    cleanup_trap
};

/**
 * ------------------------------------------------------------------------
 * Intialize terrain
 * ------------------------------------------------------------------------ */

static enum parser_error parse_feat_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct feature *h = parser_priv(p);

	struct feature *f = mem_zalloc(sizeof *f);
	f->next = h;
	f->name = string_make(name);
	parser_setpriv(p, f);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_graphics(struct parser *p) {
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	int attr = 0;
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->d_char = glyph;
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	f->d_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_mimic(struct parser *p) {
	const char *mimic_feat = parser_getstr(p, "feat");
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->mimic = string_make(mimic_feat);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_priority(struct parser *p) {
	unsigned int priority = parser_getuint(p, "priority");
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->priority = priority;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_flags(struct parser *p) {
	char *flags;
	struct feature *f = parser_priv(p);
	char *s;

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));

	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(f->flags, TF_SIZE, terrain_flags, s)) {
			mem_free(flags);
			quit_fmt("bad f-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_info(struct parser *p) {
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->shopnum = parser_getint(p, "shopnum");
	f->dig = parser_getint(p, "dig");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_desc(struct parser *p) {
    struct feature *f = parser_priv(p);
    assert(f);

    f->desc = string_append(f->desc, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_walk_msg(struct parser *p) {
    struct feature *f = parser_priv(p);
    assert(f);

    f->walk_msg = string_append(f->walk_msg, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_run_msg(struct parser *p) {
    struct feature *f = parser_priv(p);
    assert(f);

    f->run_msg = string_append(f->run_msg, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_hurt_msg(struct parser *p) {
    struct feature *f = parser_priv(p);
    assert(f);

    f->hurt_msg = string_append(f->hurt_msg, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_die_msg(struct parser *p) {
    struct feature *f = parser_priv(p);
    assert(f);

    f->die_msg = string_append(f->die_msg, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_confused_msg(struct parser *p) {
	struct feature *f = parser_priv(p);
	assert(f);

	f->confused_msg =
		string_append(f->confused_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_look_prefix(struct parser *p) {
	struct feature *f = parser_priv(p);
	assert(f);

	f->look_prefix =
		string_append(f->look_prefix, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_look_in_preposition(struct parser *p) {
	struct feature *f = parser_priv(p);
	assert(f);

	f->look_in_preposition =
		string_append(f->look_in_preposition, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_feat_resist_flag(struct parser *p) {
	int flag;
    struct feature *f = parser_priv(p);
    assert(f);

	flag = lookup_flag(mon_race_flags, parser_getsym(p, "flag"));

	if (flag == FLAG_END) {
		return PARSE_ERROR_INVALID_FLAG;
	} else {
		f->resist_flag = flag;
	}

	return PARSE_ERROR_NONE;
}

struct parser *init_parse_feat(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_feat_name);
	parser_reg(p, "graphics char glyph sym color", parse_feat_graphics);
	parser_reg(p, "mimic str feat", parse_feat_mimic);
	parser_reg(p, "priority uint priority", parse_feat_priority);
	parser_reg(p, "flags ?str flags", parse_feat_flags);
	parser_reg(p, "info int shopnum int dig", parse_feat_info);
    parser_reg(p, "desc str text", parse_feat_desc);
    parser_reg(p, "walk-msg str text", parse_feat_walk_msg);
    parser_reg(p, "run-msg str text", parse_feat_run_msg);
    parser_reg(p, "hurt-msg str text", parse_feat_hurt_msg);
    parser_reg(p, "die-msg str text", parse_feat_die_msg);
	parser_reg(p, "confused-msg str text", parse_feat_confused_msg);
	parser_reg(p, "look-prefix str text", parse_feat_look_prefix);
	parser_reg(p, "look-in-preposition str text", parse_feat_look_in_preposition);
	parser_reg(p, "resist-flag sym flag", parse_feat_resist_flag);
	return p;
}

static errr run_parse_feat(struct parser *p) {
	return parse_file_quit_not_found(p, "terrain");
}

static errr finish_parse_feat(struct parser *p) {
	struct feature *f, *n;
	int fidx;

	/* Scan the list for the max id */
	z_info->f_max = 0;
	f = parser_priv(p);
	while (f) {
		z_info->f_max++;
		f = f->next;
	}

	/* Allocate the direct access list and copy the data to it */
	f_info = mem_zalloc((z_info->f_max + 1) * sizeof(*f));
	fidx = z_info->f_max - 1;
	for (f = parser_priv(p); f; f = n, fidx--) {
		assert(fidx >= 0);

		memcpy(&f_info[fidx], f, sizeof(*f));
		/* Add trailing space for ease of use with targeting code. */
		if (f_info[fidx].look_prefix) {
			f_info[fidx].look_prefix =
				string_append(f_info[fidx].look_prefix, " ");
		}
		if (f_info[fidx].look_in_preposition) {
			f_info[fidx].look_in_preposition =
				string_append(f_info[fidx].look_in_preposition, " ");
		}
		f_info[fidx].fidx = fidx;
		n = f->next;
		if (fidx < z_info->f_max - 1)
			f_info[fidx].next = &f_info[fidx + 1];
		else
			f_info[fidx].next = NULL;
		mem_free(f);
	}

	/* Set the terrain constants */
	set_terrain();

	parser_destroy(p);
	return 0;
}

static void cleanup_feat(void) {
	int idx;
	for (idx = 0; idx < z_info->f_max; idx++) {
		string_free(f_info[idx].look_in_preposition);
		string_free(f_info[idx].look_prefix);
		string_free(f_info[idx].confused_msg);
		string_free(f_info[idx].die_msg);
		string_free(f_info[idx].hurt_msg);
		string_free(f_info[idx].run_msg);
		string_free(f_info[idx].walk_msg);
		string_free(f_info[idx].mimic);
		string_free(f_info[idx].desc);
		string_free(f_info[idx].name);
	}
	mem_free(f_info);
}

static struct file_parser feat_parser = {
	"terrain",
	init_parse_feat,
	run_parse_feat,
	finish_parse_feat,
	cleanup_feat
};

/**
 * ------------------------------------------------------------------------
 * Intialize player bodies
 * ------------------------------------------------------------------------ */

static enum parser_error parse_body_body(struct parser *p) {
	struct player_body *h = parser_priv(p);
	struct player_body *b = mem_zalloc(sizeof *b);

	b->next = h;
	b->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, b);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_body_slot(struct parser *p) {
	struct player_body *b = parser_priv(p);
	struct equip_slot *slot = b->slots;
	char *slot_type;
	int n;

	if (!b)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the last valid slot, then allocate a new one */
	if (!slot) {
		b->slots = mem_zalloc(sizeof(struct equip_slot));
		slot = b->slots;
	} else {
		while (slot->next)
			slot = slot->next;
		slot->next = mem_zalloc(sizeof(struct equip_slot));
		slot = slot->next;
	}

	slot_type = string_make(parser_getsym(p, "slot"));
	n = lookup_flag(slots, slot_type);
	if (!n)
		return PARSE_ERROR_INVALID_FLAG;
	slot->type = n;
	slot->name = string_make(parser_getsym(p, "name"));
	b->count++;
	mem_free(slot_type);
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_body(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "body str name", parse_body_body);
	parser_reg(p, "slot sym slot sym name", parse_body_slot);
	return p;
}

static errr run_parse_body(struct parser *p) {
	return parse_file_quit_not_found(p, "body");
}

static errr finish_parse_body(struct parser *p) {
	struct player_body *b;
	int i;
	bodies = parser_priv(p);

	/* Scan the list for the max slots */
	z_info->equip_slots_max = 0;
	for (b = bodies; b; b = b->next) {
		if (b->count > z_info->equip_slots_max)
			z_info->equip_slots_max = b->count;
	}

	/* Allocate the slot list and copy */
	for (b = bodies; b; b = b->next) {
		struct equip_slot *s_new;

		s_new = mem_zalloc(z_info->equip_slots_max * sizeof(*s_new));
		if (b->slots) {
			struct equip_slot *s_temp, *s_old = b->slots;

			/* Allocate space and copy */
			for (i = 0; i < z_info->equip_slots_max; i++) {
				memcpy(&s_new[i], s_old, sizeof(*s_old));
				s_old = s_old->next;
				if (!s_old) break;
			}

			/* Make next point correctly */
			for (i = 0; i < z_info->equip_slots_max; i++)
				if (s_new[i].next)
					s_new[i].next = &s_new[i + 1];

			/* Tidy up */
			s_old = b->slots;
			s_temp = s_old;
			while (s_temp) {
				s_temp = s_old->next;
				mem_free(s_old);
				s_old = s_temp;
			}
		}
		b->slots = s_new;
	}
	parser_destroy(p);
	return 0;
}

static void cleanup_body(void)
{
	struct player_body *b = bodies;
	struct player_body *next;
	int i;

	while (b) {
		next = b->next;
		string_free((char *)b->name);
		for (i = 0; i < b->count; i++)
			string_free((char *)b->slots[i].name);
		mem_free(b->slots);
		mem_free(b);
		b = next;
	}
}

static struct file_parser body_parser = {
	"body",
	init_parse_body,
	run_parse_body,
	finish_parse_body,
	cleanup_body
};

/**
 * ------------------------------------------------------------------------
 * Initialize player histories
 * ------------------------------------------------------------------------ */

static struct history_chart *histories;

static struct history_chart *findchart(struct history_chart *hs,
									   unsigned int idx) {
	for (; hs; hs = hs->next)
		if (hs->idx == idx)
			break;
	return hs;
}

static enum parser_error parse_history_chart(struct parser *p) {
	struct history_chart *oc = parser_priv(p);
	struct history_chart *c;
	struct history_entry *e = mem_zalloc(sizeof *e);
	unsigned int idx = parser_getuint(p, "chart");
	
	if (!(c = findchart(oc, idx))) {
		c = mem_zalloc(sizeof *c);
		c->next = oc;
		c->idx = idx;
		parser_setpriv(p, c);
	}

	e->isucc = parser_getint(p, "next");
	e->roll = parser_getint(p, "roll");

	e->next = c->entries;
	c->entries = e;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_history_phrase(struct parser *p) {
	struct history_chart *h = parser_priv(p);

	if (!h)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	assert(h->entries);
	h->entries->text = string_append(h->entries->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_history(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "chart uint chart int next int roll", parse_history_chart);
	parser_reg(p, "phrase str text", parse_history_phrase);
	return p;
}

static errr run_parse_history(struct parser *p) {
	return parse_file_quit_not_found(p, "history");
}

static errr finish_parse_history(struct parser *p) {
	struct history_chart *c;
	struct history_entry *e, *prev, *next;
	histories = parser_priv(p);

	/* Go fix up the entry successor pointers. We can't compute them at
	 * load-time since we may not have seen the successor history yet. Also,
	 * we need to put the entries in the right order; the parser actually
	 * stores them backwards, which is not desirable.
	 */
	for (c = histories; c; c = c->next) {
		e = c->entries;
		prev = NULL;
		while (e) {
			next = e->next;
			e->next = prev;
			prev = e;
			e = next;
		}
		c->entries = prev;
		for (e = c->entries; e; e = e->next) {
			if (!e->isucc)
				continue;
			e->succ = findchart(histories, e->isucc);
			if (!e->succ) {
				return -1;
			}
		}
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_history(void)
{
	struct history_chart *c, *next_c;
	struct history_entry *e, *next_e;

	c = histories;
	while (c) {
		next_c = c->next;
		e = c->entries;
		while (e) {
			next_e = e->next;
			mem_free(e->text);
			mem_free(e);
			e = next_e;
		}
		mem_free(c);
		c = next_c;
	}
}

static struct file_parser history_parser = {
	"history",
	init_parse_history,
	run_parse_history,
	finish_parse_history,
	cleanup_history
};

/**
 * ------------------------------------------------------------------------
 * Intialize player races
 * ------------------------------------------------------------------------ */

static enum parser_error parse_p_race_name(struct parser *p) {
	struct player_race *h = parser_priv(p);
	struct player_race *r = mem_zalloc(sizeof *r);

	r->next = h;
	r->name = string_make(parser_getstr(p, "name"));
	/* Default body is humanoid */
	r->body = 0;
	parser_setpriv(p, r);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_stats(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_adj[STAT_STR] = parser_getint(p, "str");
	r->r_adj[STAT_DEX] = parser_getint(p, "dex");
	r->r_adj[STAT_CON] = parser_getint(p, "con");
	r->r_adj[STAT_INT] = parser_getint(p, "int");
	r->r_adj[STAT_WIS] = parser_getint(p, "wis");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_disarm_phys(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_DISARM_PHYS] = parser_getint(p, "disarm");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_disarm_magic(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_DISARM_MAGIC] = parser_getint(p, "disarm");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_device(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_DEVICE] = parser_getint(p, "device");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_save(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_SAVE] = parser_getint(p, "save");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_stealth(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_STEALTH] = parser_getint(p, "stealth");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_search(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_SEARCH] = parser_getint(p, "search");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_melee(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "melee");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_shoot(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "shoot");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_throw(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_TO_HIT_THROW] = parser_getint(p, "throw");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_skill_dig(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_DIGGING] = parser_getint(p, "dig");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_hitdie(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_mhp = parser_getint(p, "mhp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_exp(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_exp = parser_getint(p, "exp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_infravision(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->infra = parser_getint(p, "infra");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_history(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->history = findchart(histories, parser_getuint(p, "hist"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_age(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->b_age = parser_getint(p, "base_age");
	r->m_age = parser_getint(p, "mod_age");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_height(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->base_hgt = parser_getint(p, "base_hgt");
	r->mod_hgt = parser_getint(p, "mod_hgt");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_weight(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->base_wgt = parser_getint(p, "base_wgt");
	r->mod_wgt = parser_getint(p, "mod_wgt");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_obj_flags(struct parser *p) {
	struct player_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->flags, OF_SIZE, list_obj_flag_names, s))
			break;
		s = strtok(NULL, " |");
	}
	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_play_flags(struct parser *p) {
	struct player_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->pflags, PF_SIZE, player_info_flags, s))
			break;
		s = strtok(NULL, " |");
	}
	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_values(struct parser *p) {
	struct player_race *r = parser_priv(p);
	char *s;
	char *t;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		int value = 0;
		int index = 0;
		bool found = false;
		if (!grab_index_and_int(&value, &index, list_element_names, "RES_", t)) {
			found = true;
			r->el_info[index].res_level = value;
		}
		if (!found)
			break;

		t = strtok(NULL, " |");
	}

	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

struct parser *init_parse_p_race(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_p_race_name);
	parser_reg(p, "stats int str int int int wis int dex int con", parse_p_race_stats);
	parser_reg(p, "skill-disarm-phys int disarm", parse_p_race_skill_disarm_phys);
	parser_reg(p, "skill-disarm-magic int disarm", parse_p_race_skill_disarm_magic);
	parser_reg(p, "skill-device int device", parse_p_race_skill_device);
	parser_reg(p, "skill-save int save", parse_p_race_skill_save);
	parser_reg(p, "skill-stealth int stealth", parse_p_race_skill_stealth);
	parser_reg(p, "skill-search int search", parse_p_race_skill_search);
	parser_reg(p, "skill-melee int melee", parse_p_race_skill_melee);
	parser_reg(p, "skill-shoot int shoot", parse_p_race_skill_shoot);
	parser_reg(p, "skill-throw int throw", parse_p_race_skill_throw);
	parser_reg(p, "skill-dig int dig", parse_p_race_skill_dig);
	parser_reg(p, "hitdie int mhp", parse_p_race_hitdie);
	parser_reg(p, "exp int exp", parse_p_race_exp);
	parser_reg(p, "infravision int infra", parse_p_race_infravision);
	parser_reg(p, "history uint hist", parse_p_race_history);
	parser_reg(p, "age int base_age int mod_age", parse_p_race_age);
	parser_reg(p, "height int base_hgt int mod_hgt", parse_p_race_height);
	parser_reg(p, "weight int base_wgt int mod_wgt", parse_p_race_weight);
	parser_reg(p, "obj-flags ?str flags", parse_p_race_obj_flags);
	parser_reg(p, "player-flags ?str flags", parse_p_race_play_flags);
	parser_reg(p, "values str values", parse_p_race_values);
	return p;
}

static errr run_parse_p_race(struct parser *p) {
	return parse_file_quit_not_found(p, "p_race");
}

static errr finish_parse_p_race(struct parser *p) {
	struct player_race *r;
	int num = 0;
	races = parser_priv(p);
	for (r = races; r; r = r->next) num++;
	for (r = races; r; r = r->next, num--) {
		assert(num);
		r->ridx = num - 1;
	}
	parser_destroy(p);
	return 0;
}

static void cleanup_p_race(void)
{
	struct player_race *p = races;
	struct player_race *next;

	while (p) {
		next = p->next;
		string_free((char *)p->name);
		mem_free(p);
		p = next;
	}
}

static struct file_parser p_race_parser = {
	"p_race",
	init_parse_p_race,
	run_parse_p_race,
	finish_parse_p_race,
	cleanup_p_race
};

/**
 * ------------------------------------------------------------------------
 * Initialize player magic realms
 * ------------------------------------------------------------------------ */
static enum parser_error parse_realm_name(struct parser *p) {
	struct magic_realm *h = parser_priv(p);
	struct magic_realm *realm = mem_zalloc(sizeof *realm);
	const char *name = parser_getstr(p, "name");

	realm->next = h;
	parser_setpriv(p, realm);
	realm->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_realm_stat(struct parser *p) {
	struct magic_realm *realm = parser_priv(p);

	realm->stat = stat_name_to_idx(parser_getsym(p, "stat"));
	if (realm->stat < 0)
		return PARSE_ERROR_INVALID_SPELL_STAT;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_realm_verb(struct parser *p) {
	const char *verb = parser_getstr(p, "verb");
	struct magic_realm *realm = parser_priv(p);
	if (!realm)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	realm->verb = string_make(verb);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_realm_spell_noun(struct parser *p) {
	const char *spell = parser_getstr(p, "spell");
	struct magic_realm *realm = parser_priv(p);
	if (!realm)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	realm->spell_noun = string_make(spell);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_realm_book_noun(struct parser *p) {
	const char *book = parser_getstr(p, "book");
	struct magic_realm *realm = parser_priv(p);
	if (!realm)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	realm->book_noun = string_make(book);
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_realm(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_realm_name);
	parser_reg(p, "stat sym stat", parse_realm_stat);
	parser_reg(p, "verb str verb", parse_realm_verb);
	parser_reg(p, "spell-noun str spell", parse_realm_spell_noun);
	parser_reg(p, "book-noun str book", parse_realm_book_noun);
	return p;
}

static errr run_parse_realm(struct parser *p) {
	return parse_file_quit_not_found(p, "realm");
}

static errr finish_parse_realm(struct parser *p) {
	realms = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_realm(void)
{
	struct magic_realm *p = realms;
	struct magic_realm *next;

	while (p) {
		next = p->next;
		string_free(p->name);
		string_free(p->verb);
		string_free(p->spell_noun);
		string_free(p->book_noun);
		mem_free(p);
		p = next;
	}
}

static struct file_parser realm_parser = {
	"realm",
	init_parse_realm,
	run_parse_realm,
	finish_parse_realm,
	cleanup_realm
};

/**
 * ------------------------------------------------------------------------
 * Intialize player shapechange shapes
 * ------------------------------------------------------------------------ */

static enum parser_error parse_shape_name(struct parser *p) {
	struct player_shape *h = parser_priv(p);
	struct player_shape *shape = mem_zalloc(sizeof *shape);

	shape->next = h;
	shape->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, shape);
	shape->sidx = z_info->shape_max++;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_combat(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	shape->to_h = parser_getint(p, "to-h");
	shape->to_d = parser_getint(p, "to-d");
	shape->to_a = parser_getint(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_disarm_phys(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_DISARM_PHYS] = parser_getint(p, "disarm");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_disarm_magic(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_DISARM_MAGIC] = parser_getint(p, "disarm");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_save(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_SAVE] = parser_getint(p, "save");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_stealth(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_STEALTH] = parser_getint(p, "stealth");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_search(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_SEARCH] = parser_getint(p, "search");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_melee(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "melee");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_throw(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_TO_HIT_THROW] = parser_getint(p, "throw");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_skill_dig(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	shape->skills[SKILL_DIGGING] = parser_getint(p, "dig");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_obj_flags(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	char *flags;
	char *s;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(shape->flags, OF_SIZE, list_obj_flag_names, s))
			break;
		s = strtok(NULL, " |");
	}
	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_play_flags(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	char *flags;
	char *s;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(shape->pflags, PF_SIZE, player_info_flags, s))
			break;
		s = strtok(NULL, " |");
	}
	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_values(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	char *s;
	char *t;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		int value = 0;
		int index = 0;
		bool found = false;
		if (!grab_int_value(shape->modifiers, obj_mods, t))
			found = true;
		if (!grab_index_and_int(&value, &index, list_element_names, "RES_", t)) {
			found = true;
			shape->el_info[index].res_level = value;
		}
		if (!found)
			break;

		t = strtok(NULL, " |");
	}

	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_effect(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*effect));

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (shape->effect) {
		effect = shape->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		shape->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_shape_effect_yx(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	struct effect *effect = shape->effect;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_dice(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	struct effect *effect = shape->effect;
	dice_t *dice = NULL;
	const char *string = NULL;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_expr(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	struct effect *effect = shape->effect;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL)
		return PARSE_ERROR_NONE;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = effect_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_effect_msg(struct parser *p) {
	struct player_shape *shape = parser_priv(p);
	struct effect *effect = shape->effect;

	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	effect->msg = string_append(effect->msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_shape_blow(struct parser *p) {
	const char *verb = parser_getstr(p, "blow");
	struct player_blow *blow = mem_zalloc(sizeof(*blow));
	struct player_shape *shape = parser_priv(p);
	if (!shape)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	blow->name = string_make(verb);
	blow->next = shape->blows;
	shape->blows = blow;
	shape->num_blows++;
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_shape(void) {
	struct parser *p = parser_new();
	z_info->shape_max = 0;
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_shape_name);
	parser_reg(p, "combat int to-h int to-d int to-a", parse_shape_combat);
	parser_reg(p, "skill-disarm-phys int disarm", parse_shape_skill_disarm_phys);
	parser_reg(p, "skill-disarm-magic int disarm", parse_shape_skill_disarm_magic);
	parser_reg(p, "skill-save int save", parse_shape_skill_save);
	parser_reg(p, "skill-stealth int stealth", parse_shape_skill_stealth);
	parser_reg(p, "skill-search int search", parse_shape_skill_search);
	parser_reg(p, "skill-melee int melee", parse_shape_skill_melee);
	parser_reg(p, "skill-throw int throw", parse_shape_skill_throw);
	parser_reg(p, "skill-dig int dig", parse_shape_skill_dig);
	parser_reg(p, "obj-flags ?str flags", parse_shape_obj_flags);
	parser_reg(p, "player-flags ?str flags", parse_shape_play_flags);
	parser_reg(p, "values str values", parse_shape_values);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_shape_effect);
	parser_reg(p, "effect-yx int y int x", parse_shape_effect_yx);
	parser_reg(p, "dice str dice", parse_shape_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_shape_expr);
	parser_reg(p, "effect-msg str text", parse_shape_effect_msg);
	parser_reg(p, "blow str blow", parse_shape_blow);
	return p;
}

static errr run_parse_shape(struct parser *p) {
	return parse_file_quit_not_found(p, "shape");
}

static errr finish_parse_shape(struct parser *p) {
	shapes = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_shape(void)
{
	struct player_shape *shape = shapes;
	struct player_shape *next;

	while (shape) {
		struct player_blow *blow = shape->blows;
		next = shape->next;
		string_free((char *)shape->name);
		free_effect(shape->effect);
		while (blow) {
			struct player_blow *next_blow = blow->next;
			string_free(blow->name);
			mem_free(blow);
			blow = next_blow;
		}
		mem_free(shape);
		shape = next;
	}
}

static struct file_parser shape_parser = {
	"shape",
	init_parse_shape,
	run_parse_shape,
	finish_parse_shape,
	cleanup_shape
};

/**
 * ------------------------------------------------------------------------
 * Initialize player classes
 * ------------------------------------------------------------------------ */

static enum parser_error parse_class_name(struct parser *p) {
	struct player_class *h = parser_priv(p);
	struct player_class *c = mem_zalloc(sizeof *c);
	c->name = string_make(parser_getstr(p, "name"));
	c->next = h;
	parser_setpriv(p, c);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_stats(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	c->c_adj[STAT_STR] = parser_getint(p, "str");
	c->c_adj[STAT_INT] = parser_getint(p, "int");
	c->c_adj[STAT_WIS] = parser_getint(p, "wis");
	c->c_adj[STAT_DEX] = parser_getint(p, "dex");
	c->c_adj[STAT_CON] = parser_getint(p, "con");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_disarm_phys(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_DISARM_PHYS] = parser_getint(p, "base");
	c->x_skills[SKILL_DISARM_PHYS] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_disarm_magic(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_DISARM_MAGIC] = parser_getint(p, "base");
	c->x_skills[SKILL_DISARM_MAGIC] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_device(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_DEVICE] = parser_getint(p, "base");
	c->x_skills[SKILL_DEVICE] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_save(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_SAVE] = parser_getint(p, "base");
	c->x_skills[SKILL_SAVE] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_stealth(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_STEALTH] = parser_getint(p, "base");
	c->x_skills[SKILL_STEALTH] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_search(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_SEARCH] = parser_getint(p, "base");
	c->x_skills[SKILL_SEARCH] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_melee(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "base");
	c->x_skills[SKILL_TO_HIT_MELEE] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_shoot(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "base");
	c->x_skills[SKILL_TO_HIT_BOW] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_throw(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_TO_HIT_THROW] = parser_getint(p, "base");
	c->x_skills[SKILL_TO_HIT_THROW] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_skill_dig(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_DIGGING] = parser_getint(p, "base");
	c->x_skills[SKILL_DIGGING] = parser_getint(p, "incr");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_hitdie(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_mhp = parser_getint(p, "mhp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_exp(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_exp = parser_getint(p, "exp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_max_attacks(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->max_attacks = parser_getint(p, "max-attacks");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_min_weight(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->min_weight = parser_getint(p, "min-weight");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_str_mult(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->att_multiply = parser_getint(p, "att-multiply");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_title(struct parser *p) {
	struct player_class *c = parser_priv(p);
	int i;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < PY_MAX_LEVEL / 5; i++) {
		if (!c->title[i]) {
			c->title[i] = string_make(parser_getstr(p, "title"));
			break;
		}
	}

	if (i >= PY_MAX_LEVEL / 5)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	return PARSE_ERROR_NONE;
}

static int lookup_option(const char *name)
{
	int result = 1;

	while (1) {
		if (result >= OPT_MAX) {
			return 0;
		}
		if (streq(option_name(result), name)) {
			return result;
		}
		++result;
	}
}

static enum parser_error parse_class_equip(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct start_item *si;
	int tval, sval;
	char *eopts;
	char *s;
	int *einds;
	int nind, nalloc;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	eopts = string_make(parser_getsym(p, "eopts"));
	einds = NULL;
	nind = 0;
	nalloc = 0;
	s = strtok(eopts, " |");
	while (s) {
		bool negated = false;
		int ind;

		if (prefix(s, "NOT-")) {
			negated = true;
			s += 4;
		}
		ind = lookup_option(s);
		if (ind > 0 && option_type(ind) == OP_BIRTH) {
			if (nind >= nalloc - 2) {
				if (nalloc == 0) {
					nalloc = 2;
				} else {
					nalloc *= 2;
				}
				einds = mem_realloc(einds,
					nalloc * sizeof(*einds));
			}
			einds[nind] = (negated) ? -ind : ind;
			einds[nind + 1] = 0;
			++nind;
		} else if (!streq(s, "none")) {
			mem_free(einds);
			mem_free(eopts);
			quit_fmt("bad option name: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	mem_free(eopts);

	si = mem_zalloc(sizeof *si);
	si->tval = tval;
	si->sval = sval;
	si->min = parser_getuint(p, "min");
	si->max = parser_getuint(p, "max");
	si->eopts = einds;

	if (si->min > 99 || si->max > 99) {
		mem_free(si->eopts);
		mem_free(si);
		return PARSE_ERROR_INVALID_ITEM_NUMBER;
	}

	si->next = c->start_items;
	c->start_items = si;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_obj_flags(struct parser *p) {
	struct player_class *c = parser_priv(p);
	char *flags;
	char *s;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(c->flags, OF_SIZE, list_obj_flag_names, s))
			break;
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_class_play_flags(struct parser *p) {
	struct player_class *c = parser_priv(p);
	char *flags;
	char *s;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(c->pflags, PF_SIZE, player_info_flags, s))
			break;
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return s ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_class_magic(struct parser *p) {
	struct player_class *c = parser_priv(p);
	int num_books;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->magic.spell_first = parser_getuint(p, "first");
	c->magic.spell_weight = parser_getuint(p, "weight");
	num_books = parser_getuint(p, "books");
	c->magic.books = mem_zalloc(num_books * sizeof(struct class_book));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_book(struct parser *p) {
	struct player_class *c = parser_priv(p);
	int tval, spells;
	const char *name, *quality;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	c->magic.books[c->magic.num_books].tval = tval;

	quality = parser_getsym(p, "quality");
	if (streq(quality, "dungeon")) {
		c->magic.books[c->magic.num_books].dungeon = true;
	}
	name = parser_getsym(p, "name");
	write_book_kind(&c->magic.books[c->magic.num_books], name);

	spells = parser_getuint(p, "spells");
	c->magic.books[c->magic.num_books].spells =
		mem_zalloc(spells * sizeof(struct class_spell));
	c->magic.books[c->magic.num_books++].realm =
		lookup_realm(parser_getstr(p, "realm"));

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_book_graphics(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *b = &c->magic.books[c->magic.num_books - 1];
	struct object_kind *k = lookup_kind(b->tval, b->sval);
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");

	k->d_char = glyph;
	if (strlen(color) > 1) {
		k->d_attr = color_text_to_attr(color);
	} else {
		k->d_attr = color_char_to_attr(color[0]);
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_book_properties(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *b = &c->magic.books[c->magic.num_books - 1];
	struct object_kind *k = lookup_kind(b->tval, b->sval);
	const char *tmp;
	int amin, amax;

	assert(k);
	k->cost = parser_getint(p, "cost");
	k->alloc_prob = parser_getint(p, "common");

	tmp = parser_getstr(p, "minmax");
	if (sscanf(tmp, "%d to %d", &amin, &amax) != 2)
		return PARSE_ERROR_INVALID_ALLOCATION;
	k->level = amin;
	k->alloc_min = amin;
	k->alloc_max = amax;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_spell(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	book->spells[book->num_spells].realm = book->realm;

	book->spells[book->num_spells].name = string_make(parser_getsym(p, "name"));
	book->spells[book->num_spells].sidx = c->magic.total_spells;
	c->magic.total_spells++;
	book->spells[book->num_spells].bidx = c->magic.num_books - 1;
	book->spells[book->num_spells].slevel = parser_getint(p, "level");
	book->spells[book->num_spells].smana = parser_getint(p, "mana");
	book->spells[book->num_spells].sfail = parser_getint(p, "fail");
	book->spells[book->num_spells++].sexp = parser_getint(p, "exp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_effect(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];
	struct class_spell *spell = &book->spells[book->num_spells - 1];
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*effect));

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (spell->effect) {
		effect = spell->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		spell->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_class_effect_yx(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];
	struct class_spell *spell = &book->spells[book->num_spells - 1];
	struct effect *effect = spell->effect;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_dice(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];
	struct class_spell *spell = &book->spells[book->num_spells - 1];
	struct effect *effect = spell->effect;
	dice_t *dice = NULL;
	const char *string = NULL;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_expr(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];
	struct class_spell *spell = &book->spells[book->num_spells - 1];
	struct effect *effect = spell->effect;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL)
		return PARSE_ERROR_NONE;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = effect_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_effect_msg(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];
	struct class_spell *spell = &book->spells[book->num_spells - 1];
	struct effect *effect = spell->effect;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	effect->msg = string_append(effect->msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_desc(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];
	struct class_spell *spell = &book->spells[book->num_spells - 1];

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	spell->text = string_append(spell->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_class(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_class_name);
	parser_reg(p, "stats int str int int int wis int dex int con",
			   parse_class_stats);
	parser_reg(p, "skill-disarm-phys int base int incr",
			   parse_class_skill_disarm_phys);
	parser_reg(p, "skill-disarm-magic int base int incr",
			   parse_class_skill_disarm_magic);
	parser_reg(p, "skill-device int base int incr", parse_class_skill_device);
	parser_reg(p, "skill-save int base int incr", parse_class_skill_save);
	parser_reg(p, "skill-stealth int base int incr", parse_class_skill_stealth);
	parser_reg(p, "skill-search int base int incr", parse_class_skill_search);
	parser_reg(p, "skill-melee int base int incr", parse_class_skill_melee);
	parser_reg(p, "skill-shoot int base int incr", parse_class_skill_shoot);
	parser_reg(p, "skill-throw int base int incr", parse_class_skill_throw);
	parser_reg(p, "skill-dig int base int incr", parse_class_skill_dig);
	parser_reg(p, "hitdie int mhp", parse_class_hitdie);
	parser_reg(p, "exp int exp", parse_class_exp);
	parser_reg(p, "max-attacks int max-attacks", parse_class_max_attacks);
	parser_reg(p, "min-weight int min-weight", parse_class_min_weight);
	parser_reg(p, "strength-multiplier int att-multiply", parse_class_str_mult);
	parser_reg(p, "title str title", parse_class_title);
	parser_reg(p, "equip sym tval sym sval uint min uint max sym eopts",
			   parse_class_equip);
	parser_reg(p, "obj-flags ?str flags", parse_class_obj_flags);
	parser_reg(p, "player-flags ?str flags", parse_class_play_flags);
	parser_reg(p, "magic uint first uint weight uint books", parse_class_magic);
	parser_reg(p, "book sym tval sym quality sym name uint spells str realm",
			   parse_class_book);
	parser_reg(p, "book-graphics char glyph sym color",
			   parse_class_book_graphics);
	parser_reg(p, "book-properties int cost int common str minmax",
			   parse_class_book_properties);
	parser_reg(p, "spell sym name int level int mana int fail int exp",
			   parse_class_spell);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_class_effect);
	parser_reg(p, "effect-yx int y int x", parse_class_effect_yx);
	parser_reg(p, "dice str dice", parse_class_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_class_expr);
	parser_reg(p, "effect-msg str text", parse_class_effect_msg);
	parser_reg(p, "desc str desc", parse_class_desc);
	return p;
}

static errr run_parse_class(struct parser *p) {
	return parse_file_quit_not_found(p, "class");
}

static errr finish_parse_class(struct parser *p) {
	struct player_class *c;
	int num = 0;
	classes = parser_priv(p);
	for (c = classes; c; c = c->next) num++;
	for (c = classes; c; c = c->next, num--) {
		assert(num);
		c->cidx = num - 1;
	}
	parser_destroy(p);
	return 0;
}

static void cleanup_class(void)
{
	struct player_class *c = classes;
	struct player_class *next;
	struct start_item *item, *item_next;
	struct class_spell *spell;
	struct class_book *book;
	int i, j;

	while (c) {
		next = c->next;
		item = c->start_items;
		while(item) {
			item_next = item->next;
			mem_free(item->eopts);
			mem_free(item);
			item = item_next;
		}
		for (i = 0; i < c->magic.num_books; i++) {
			book = &c->magic.books[i];
			for (j = 0; j < book->num_spells; j++) {
				spell = &book->spells[j];
				string_free(spell->name);
				string_free(spell->text);
				free_effect(spell->effect);
			}
			mem_free(book->spells);
		}
		mem_free(c->magic.books);
		for (i = 0; i < PY_MAX_LEVEL / 5; i++) {
			string_free((char *)c->title[i]);
		}
		mem_free((char *)c->name);
		mem_free(c);
		c = next;
	}
}

static struct file_parser class_parser = {
	"class",
	init_parse_class,
	run_parse_class,
	finish_parse_class,
	cleanup_class
};

/**
 * ------------------------------------------------------------------------
 * Intialize flavors
 * ------------------------------------------------------------------------ */

static wchar_t flavor_glyph;
static unsigned int flavor_tval;

static enum parser_error parse_flavor_flavor(struct parser *p) {
	struct flavor *h = parser_priv(p);
	struct flavor *f = mem_zalloc(sizeof *f);

	const char *attr;
	int d_attr;

	f->next = h;

	f->fidx = parser_getuint(p, "index");
	f->tval = flavor_tval;
	f->d_char = flavor_glyph;

	if (parser_hasval(p, "sval"))
		f->sval = lookup_sval(f->tval, parser_getsym(p, "sval"));
	else
		f->sval = SV_UNKNOWN;

	attr = parser_getsym(p, "attr");
	if (strlen(attr) == 1)
		d_attr = color_char_to_attr(attr[0]);
	else
		d_attr = color_text_to_attr(attr);

	if (d_attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	f->d_attr = d_attr;

	if (parser_hasval(p, "desc"))
		f->text = string_append(f->text, parser_getstr(p, "desc"));

	parser_setpriv(p, f);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_flavor_kind(struct parser *p) {
	flavor_glyph = parser_getchar(p, "glyph");
	flavor_tval = tval_find_idx(parser_getsym(p, "tval"));
	if (!flavor_tval)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	return PARSE_ERROR_NONE;
}

struct parser *init_parse_flavor(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "kind sym tval char glyph", parse_flavor_kind);
	parser_reg(p, "flavor uint index sym attr ?str desc", parse_flavor_flavor);
	parser_reg(p, "fixed uint index sym sval sym attr ?str desc", parse_flavor_flavor);

	return p;
}

static errr run_parse_flavor(struct parser *p) {
	return parse_file_quit_not_found(p, "flavor");
}

static errr finish_parse_flavor(struct parser *p) {
	flavors = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_flavor(void)
{
	struct flavor *f, *next;

	f = flavors;
	while(f) {
		next = f->next;
		/* Hack - scrolls get randomly-generated names */
		if (f->tval != TV_SCROLL)
			mem_free(f->text);
		mem_free(f);
		f = next;
	}
}

struct file_parser flavor_parser = {
	"flavor",
	init_parse_flavor,
	run_parse_flavor,
	finish_parse_flavor,
	cleanup_flavor
};


/**
 * ------------------------------------------------------------------------
 * Initialize hints
 * ------------------------------------------------------------------------ */

static enum parser_error parse_hint(struct parser *p) {
	struct hint *h = parser_priv(p);
	struct hint *new = mem_zalloc(sizeof *new);

	new->hint = string_make(parser_getstr(p, "text"));
	new->next = h;

	parser_setpriv(p, new);
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_hints(void) {
	struct parser *p = parser_new();
	parser_reg(p, "H str text", parse_hint);
	return p;
}

static errr run_parse_hints(struct parser *p) {
	return parse_file_quit_not_found(p, "hints");
}

static errr finish_parse_hints(struct parser *p) {
	hints = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_hints(void)
{
	struct hint *h, *next;

	h = hints;
	while(h) {
		next = h->next;
		string_free(h->hint);
		mem_free(h);
		h = next;
	}
}

static struct file_parser hints_parser = {
	"hints",
	init_parse_hints,
	run_parse_hints,
	finish_parse_hints,
	cleanup_hints
};

/**
 * ------------------------------------------------------------------------
 * Game data initialization
 * ------------------------------------------------------------------------ */

/**
 * A list of all the above parsers, plus those found in mon-init.c and
 * obj-init.c
 */
static struct {
	const char *name;
	struct file_parser *parser;
} pl[] = {
	{ "world", &world_parser },
	{ "projections", &projection_parser },
	{ "ui renderers", &ui_entry_renderer_parser },
	{ "ui entries", &ui_entry_parser },
	{ "player properties", &player_property_parser },
	{ "features", &feat_parser },
	{ "object bases", &object_base_parser },
	{ "slays", &slay_parser },
	{ "brands", &brand_parser },
	{ "monster pain messages", &pain_parser },
	{ "monster bases", &mon_base_parser },
	{ "summons", &summon_parser },
	{ "curses", &curse_parser },
	{ "player shapes", &shape_parser },
	{ "objects", &object_parser },
	{ "activations", &act_parser },
	{ "ego-items", &ego_parser },
	{ "history charts", &history_parser },
	{ "bodies", &body_parser },
	{ "player races", &p_race_parser },
	{ "magic realms", &realm_parser },
	{ "player classes", &class_parser },
	{ "artifacts", &artifact_parser },
	{ "object properties", &object_property_parser },
	{ "timed effects", &player_timed_parser },
	{ "blow methods", &meth_parser },
	{ "blow effects", &eff_parser },
	{ "monster spells", &mon_spell_parser },
	{ "monsters", &monster_parser },
	{ "monster pits" , &pit_parser },
	{ "monster lore" , &lore_parser },
	{ "traps", &trap_parser },
	{ "chest_traps", &chest_trap_parser },
	{ "quests", &quests_parser },
	{ "flavours", &flavor_parser },
	{ "hints", &hints_parser },
	{ "random names", &names_parser }
};

/**
 * Initialize just the internal arrays.
 * This should be callable by the test suite, without relying on input, or
 * anything to do with a user or savefiles.
 *
 * Assumption: Paths are set up correctly before calling this function.
 */
void init_arrays(void)
{
	unsigned int i;

	for (i = 0; i < N_ELEMENTS(pl); i++) {
		char *msg = string_make(format("Initializing %s...", pl[i].name));
		event_signal_message(EVENT_INITSTATUS, 0, msg);
		string_free(msg);
		if (run_parser(pl[i].parser))
			quit_fmt("Cannot initialize %s.", pl[i].name);
	}
}

/**
 * Free all the internal arrays
 */
static void cleanup_arrays(void)
{
	unsigned int i;

	for (i = 1; i < N_ELEMENTS(pl); i++)
		cleanup_parser(pl[i].parser);

	cleanup_parser(pl[0].parser);
}

static struct init_module arrays_module = {
	.name = "arrays",
	.init = init_arrays,
	.cleanup = cleanup_arrays
};


extern struct init_module z_quark_module;
extern struct init_module generate_module;
extern struct init_module rune_module;
extern struct init_module obj_make_module;
extern struct init_module ignore_module;
extern struct init_module mon_make_module;
extern struct init_module player_module;
extern struct init_module store_module;
extern struct init_module messages_module;
extern struct init_module options_module;
extern struct init_module ui_player_module;
extern struct init_module ui_equip_cmp_module;

static struct init_module *modules[] = {
	&z_quark_module,
	&messages_module,
	&ui_visuals_module, /* This needs to load before monsters and objects. */
	&arrays_module,
	&player_module,
	&generate_module,
	&rune_module,
	&obj_make_module,
	&ignore_module,
	&mon_make_module,
	&store_module,
	&options_module,
	&ui_player_module,
	&ui_equip_cmp_module,
	NULL
};

/**
 * Initialise Angband's data stores and allocate memory for structures,
 * etc, so that the game can get started.
 *
 * The only input/output in this file should be via event_signal_string().
 * We cannot rely on any particular UI as this part should be UI-agnostic.
 * We also cannot rely on anything else having being initialised into any
 * particlar state.  Which is why you'd be calling this function in the 
 * first place.
 *
 * Old comment, not sure if still accurate:
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 */
bool init_angband(void)
{
	int i;

	event_signal(EVENT_ENTER_INIT);

	init_game_constants();

	/* Initialise modules */
	for (i = 0; modules[i]; i++)
		if (modules[i]->init)
			modules[i]->init();

	/* Initialize some other things */
	event_signal_message(EVENT_INITSTATUS, 0, "Initializing other stuff...");

	/* List display codes */
	monster_list_init();
	object_list_init();

	/* Initialise RNG */
	event_signal_message(EVENT_INITSTATUS, 0, "Getting the dice rolling...");
	Rand_init();

	return true;
}

/**
 * Free all the stuff initialised in init_angband()
 */
void cleanup_angband(void)
{
	int i;

	/* Free the chunk list */
	for (i = 0; i < chunk_list_max; i++) {
		wipe_mon_list(chunk_list[i], player);
		cave_free(chunk_list[i]);
	}
	mem_free(chunk_list);
	chunk_list = NULL;

	for (i = 0; modules[i]; i++)
		if (modules[i]->cleanup)
			modules[i]->cleanup();

	event_remove_all_handlers();

	/* Free the main cave */
	if (cave) {
		cave_free(cave);
		cave = NULL;
	}

	monster_list_finalize();
	object_list_finalize();

	cleanup_game_constants();

	cmdq_flush();

	if (play_again) return;

	/* Free the format() buffer */
	vformat_kill();

	/* Free the directories */
	string_free(ANGBAND_DIR_GAMEDATA);
	string_free(ANGBAND_DIR_CUSTOMIZE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_SCREENS);
	string_free(ANGBAND_DIR_FONTS);
	string_free(ANGBAND_DIR_TILES);
	string_free(ANGBAND_DIR_SOUNDS);
	string_free(ANGBAND_DIR_ICONS);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PANIC);
	string_free(ANGBAND_DIR_SCORES);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_ARCHIVE);
}
