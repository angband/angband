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
#include "effects.h"
#include "game-event.h"
#include "generate.h"
#include "hint.h"
#include "init.h"
#include "mon-init.h"
#include "mon-list.h"
#include "mon-lore.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-ignore.h"
#include "obj-list.h"
#include "obj-make.h"
#include "obj-randart.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "option.h"
#include "parser.h"
#include "player.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-spell.h"
#include "player-timed.h"
#include "project.h"
#include "randname.h"
#include "store.h"
#include "trap.h"

/**
 * Structure (not array) of game constants
 */
struct angband_constants *z_info;

/**
 * Structures to hold all brands and slays possible in the game
 */
struct brand *game_brands;
struct slay *game_slays;

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
char *ANGBAND_DIR_SCORES;
char *ANGBAND_DIR_INFO;

static struct history_chart *histories;

static const char *slots[] = {
	#define EQUIP(a, b, c, d, e, f) #a,
	#include "list-equip-slots.h"
	#undef EQUIP
	NULL
};

static const char *obj_flags[] = {
	"NONE",
	#define STAT(a, b, c, d, e, f, g, h, i) #c,
	#include "list-stats.h"
	#undef STAT
	#define OF(a, b, c, d, e, f) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

static const char *obj_mods[] = {
	#define STAT(a, b, c, d, e, f, g, h, i) #a,
	#include "list-stats.h"
	#undef STAT
	#define OBJ_MOD(a, b, c, d) #a,
	#include "list-object-modifiers.h"
	#undef OBJ_MOD
	NULL
};

static const char *kind_flags[] = {
	#define KF(a, b) #a,
	#include "list-kind-flags.h"
	#undef KF
	NULL
};

static const char *elements[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) #a,
	#include "list-elements.h"
	#undef ELEM
	NULL
};

static const char *slays[] = {
	#define RF(a, b, c) #a,
	#include "list-mon-race-flags.h"
	#undef RF
	NULL
};

static const char *brand_names[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) b,
	#include "list-elements.h"
	#undef ELEM
	NULL
};

static const char *slay_names[] = {
	#define RF(a, b, c) b,
	#include "list-mon-race-flags.h"
	#undef RF
	NULL
};

static const char *effect_list[] = {
	"NONE",
	#define EFFECT(x, a, b, c, d, e)	#x,
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

static const char *player_info_flags[] =
{
	#define PF(a, b, c) #a,
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
		val = effect_param(effect->index, type);
		if (val < 0)
			return PARSE_ERROR_INVALID_VALUE;
		else
			effect->params[0] = val;
	}

	if (parser_hasval(p, "xtra"))
		effect->params[1] = parser_getint(p, "xtra");

	return PARSE_ERROR_NONE;
}

static bool grab_element_flag(struct element_info *info, const char *flag_name)
{
	char prefix[20];
	char suffix[20];
	size_t i;

	if (2 != sscanf(flag_name, "%[^_]_%s", prefix, suffix))
		return false;

	/* Ignore or hate */
	for (i = 0; i < ELEM_MAX; i++)
		if (streq(suffix, elements[i])) {
			if (streq(prefix, "IGNORE")) {
				info[i].flags |= EL_INFO_IGNORE;
				return true;
			}
			if (streq(prefix, "HATES")) {
				info[i].flags |= EL_INFO_HATES;
				return true;
			}
		}

	return false;
}

static struct history_chart *findchart(struct history_chart *hs, unsigned int idx) {
	for (; hs; hs = hs->next)
		if (hs->idx == idx)
			break;
	return hs;
}

static struct activation *findact(const char *act_name) {
	struct activation *act = &activations[1];
	while (act) {
		if (streq(act->name, act_name))
			break;
		act = act->next;
	}
	return act;
}

static enum parser_error write_dummy_object_record(struct artifact *art, const char *name)
{
	struct object_kind *temp, *dummy;
	int i;
	char mod_name[100];

	/* Extend by 1 and realloc */
	z_info->k_max += 1;
	temp = mem_realloc(k_info, (z_info->k_max + 1) * sizeof(*temp));

	/* Copy if no errors */
	if (!temp)
		return PARSE_ERROR_INTERNAL;
	else
		k_info = temp;

	/* Use the (second) last entry for the dummy */
	dummy = &k_info[z_info->k_max - 1];
	memset(dummy, 0, sizeof(*dummy));

	/* Copy the tval and base */
	dummy->tval = art->tval;
	dummy->base = &kb_info[dummy->tval];

	/* Make the name and index */
	my_strcpy(mod_name, format("& %s~", name), sizeof(mod_name));
	dummy->name = string_make(mod_name);
	dummy->kidx = z_info->k_max - 1;

	/* Increase the sval count for this tval, set the new one to the max */
	for (i = 0; i < TV_MAX; i++)
		if (kb_info[i].tval == dummy->tval) {
			kb_info[i].num_svals++;
			dummy->sval = kb_info[i].num_svals;
			break;
		}
	if (i == TV_MAX) return PARSE_ERROR_INTERNAL;

	/* Copy the sval to the artifact info */
	art->sval = dummy->sval;

	/* Give the object default colours (these should be overwritten) */
	dummy->d_char = '*';
	dummy->d_attr = COLOUR_RED;

	/* Register this as an INSTA_ART object */
	kf_on(dummy->kind_flags, KF_INSTA_ART);

	return PARSE_ERROR_NONE;
}

void add_game_brand(struct brand *b)
{
	struct brand *known_b = game_brands;

	if (!known_b) {
		/* Copy the name and element */
		game_brands = mem_zalloc(sizeof(struct brand));
		game_brands->name = string_make(b->name);
		game_brands->element = b->element;
		return;
	}

	while (known_b) {
		/* Same element is all we need */
		if (known_b->element == b->element) return;

		/* Not found, so add it */
		if (!known_b->next) {
			/* Copy the name and element */
			struct brand *new_b = mem_zalloc(sizeof *new_b);
			new_b->name = string_make(b->name);
			new_b->element = b->element;

			/* Attach the new brand */
			new_b->next = game_brands;
			game_brands = new_b;
			return;
		}

		known_b = known_b->next;
	}
}

void add_game_slay(struct slay *s)
{
	struct slay *known_s = game_slays;

	if (!known_s) {
		/* Copy the name and race flag */
		game_slays = mem_zalloc(sizeof(struct slay));
		game_slays->name = string_make(s->name);
		game_slays->race_flag = s->race_flag;
		return;
	}

	while (known_s) {
		/* Name and race flag need to be the same */
		if (streq(known_s->name, s->name) &&
			(known_s->race_flag == s->race_flag)) return;

		/* Not found, so add it */
		if (!known_s->next) {
			/* Copy the name and race flag */
			struct slay *new_s = mem_zalloc(sizeof *new_s);
			new_s->name = string_make(s->name);
			new_s->race_flag = s->race_flag;

			/* Attach the new slay */
			new_s->next = game_slays;
			game_slays = new_s;
			return;
		}

		known_s = known_s->next;
	}
}

/**
 * Find the default paths to all of our important sub-directories.
 *
 * All of the sub-directories should, by default, be located inside
 * the main directory, whose location is very system dependant and is 
 * set by the ANGBAND_PATH environment variable, if it exists. (On multi-
 * user systems such as Linux this is not the default - see config.h for
 * more info.)
 *
 * This function takes a writable buffers, initially containing the
 * "path" to the "config", "lib" and "data" directories, for example, 
 * "/etc/angband/", "/usr/share/angband" and "/var/games/angband" -
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
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
	string_free(ANGBAND_DIR_SCORES);
	string_free(ANGBAND_DIR_INFO);

	/*** Prepare the paths ***/

	/* Build path names */
	ANGBAND_DIR_GAMEDATA = string_make(format("%sgamedata", configpath));
	ANGBAND_DIR_CUSTOMIZE = string_make(format("%scustomize", configpath));
	ANGBAND_DIR_HELP = string_make(format("%shelp", libpath));
	ANGBAND_DIR_SCREENS = string_make(format("%sscreens", libpath));
	ANGBAND_DIR_FONTS = string_make(format("%sfonts", libpath));
	ANGBAND_DIR_TILES = string_make(format("%stiles", libpath));
	ANGBAND_DIR_SOUNDS = string_make(format("%ssounds", libpath));
	ANGBAND_DIR_ICONS = string_make(format("%sicons", libpath));

#ifdef PRIVATE_USER_PATH

	/* Build the path to the user specific directory */
	if (strncmp(ANGBAND_SYS, "test", 4) == 0)
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, "Test");
	else
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, VERSION_NAME);
	ANGBAND_DIR_USER = string_make(buf);

#else /* !PRIVATE_USER_PATH */

#ifdef MACH_O_CARBON
	ANGBAND_DIR_USER = string_make(datapath);
#else /* !MACH_O_CARBON */
	ANGBAND_DIR_USER = string_make(format("%suser", datapath));
#endif /* MACH_O_CARBON */

#endif /* PRIVATE_USER_PATH */

	/* Build the path to the user info directory */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "info");
	ANGBAND_DIR_INFO = string_make(buf);

#ifdef USE_PRIVATE_PATHS

    /* Build the path to the score and save directories */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores");
	ANGBAND_DIR_SCORES = string_make(buf);

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "save");
	ANGBAND_DIR_SAVE = string_make(buf);

#else /* !USE_PRIVATE_PATHS */

	/* Build pathnames */
	ANGBAND_DIR_SCORES = string_make(format("%sscores", datapath));
	ANGBAND_DIR_SAVE = string_make(format("%ssave", datapath));

#endif /* USE_PRIVATE_PATHS */
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

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_SCORES, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_INFO, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_HELP, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);
}

/**
 * Parsing functions for constants.txt
 */
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
	else if (streq(label, "flow-depth"))
		z->max_flow_depth = value;
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
	else if (streq(label, "floor-size"))
		z->floor_size = value;
	else if (streq(label, "stack-size"))
		z->stack_size = value;
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
	return parse_file(p, "constants");
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
 * Parsing functions for object_base.txt
 */
struct kb_parsedata {
	struct object_base defaults;
	struct object_base *kb;
};

static enum parser_error parse_object_base_defaults(struct parser *p) {
	const char *label;
	int value;

	struct kb_parsedata *d = parser_priv(p);
	assert(d);

	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (streq(label, "break-chance"))
		d->defaults.break_perc = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_name(struct parser *p) {
	struct object_base *kb;

	struct kb_parsedata *d = parser_priv(p);
	assert(d);

	kb = mem_alloc(sizeof *kb);
	memcpy(kb, &d->defaults, sizeof(*kb));
	kb->next = d->kb;
	d->kb = kb;

	kb->tval = tval_find_idx(parser_getsym(p, "tval"));
	if (kb->tval == -1)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	if (parser_hasval(p, "name"))
		kb->name = string_make(parser_getstr(p, "name"));
	kb->num_svals = 0;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_graphics(struct parser *p) {
	struct object_base *kb;
	const char *color;

	struct kb_parsedata *d = parser_priv(p);
	assert(d);

	kb = d->kb;
	assert(kb);

	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		kb->attr = color_text_to_attr(color);
	else
		kb->attr = color_char_to_attr(color[0]);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_break(struct parser *p) {
	struct object_base *kb;

	struct kb_parsedata *d = parser_priv(p);
	assert(d);

	kb = d->kb;
	assert(kb);

	kb->break_perc = parser_getint(p, "breakage");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_flags(struct parser *p) {
	struct object_base *kb;
	char *s, *t;

	struct kb_parsedata *d = parser_priv(p);
	assert(d);

	kb = d->kb;
	assert(kb);

	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(kb->flags, OF_SIZE, obj_flags, t))
			found = true;
		if (!grab_flag(kb->kind_flags, KF_SIZE, kind_flags, t))
			found = true;
		if (grab_element_flag(kb->el_info, t))
			found = true;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);

	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

struct parser *init_parse_object_base(void) {
	struct parser *p = parser_new();

	struct kb_parsedata *d = mem_zalloc(sizeof(*d));
	parser_setpriv(p, d);

	parser_reg(p, "default sym label int value", parse_object_base_defaults);
	parser_reg(p, "name sym tval ?str name", parse_object_base_name);
	parser_reg(p, "graphics sym color", parse_object_base_graphics);
	parser_reg(p, "break int breakage", parse_object_base_break);
	parser_reg(p, "flags str flags", parse_object_base_flags);
	return p;
}

static errr run_parse_object_base(struct parser *p) {
	return parse_file(p, "object_base");
}

static errr finish_parse_object_base(struct parser *p) {
	struct object_base *kb;
	struct object_base *next = NULL;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);

	kb_info = mem_zalloc(TV_MAX * sizeof(*kb_info));

	for (kb = d->kb; kb; kb = next) {
		if (kb->tval >= TV_MAX)
			continue;
		memcpy(&kb_info[kb->tval], kb, sizeof(*kb));
		next = kb->next;
		mem_free(kb);
	}

	mem_free(d);
	parser_destroy(p);
	return 0;
}

static void cleanup_object_base(void)
{
	int idx;
	for (idx = 0; idx < TV_MAX; idx++)
	{
		string_free(kb_info[idx].name);
	}
	mem_free(kb_info);
}

static struct file_parser object_base_parser = {
	"object_base",
	init_parse_object_base,
	run_parse_object_base,
	finish_parse_object_base,
	cleanup_object_base
};



/**
 * Parsing functions for object.txt
 */

/* Generic object kinds */
struct object_kind *unknown_item_kind;
struct object_kind *unknown_gold_kind;
struct object_kind *pile_kind;

static enum parser_error parse_object_name(struct parser *p) {
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct object_kind *h = parser_priv(p);

	struct object_kind *k = mem_zalloc(sizeof *k);
	k->next = h;
	parser_setpriv(p, k);
	k->kidx = idx;
	k->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_graphics(struct parser *p) {
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->d_char = glyph;
	if (strlen(color) > 1)
		k->d_attr = color_text_to_attr(color);
	else
		k->d_attr = color_char_to_attr(color[0]);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_type(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	int tval;

	assert(k);

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	k->tval = tval;
	k->base = &kb_info[k->tval];
	k->base->num_svals++;
	k->sval = k->base->num_svals;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_properties(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->level = parser_getint(p, "level");
	k->weight = parser_getint(p, "weight");
	k->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_alloc(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;
	assert(k);

	k->alloc_prob = parser_getint(p, "common");
	if (sscanf(tmp, "%d to %d", &amin, &amax) != 2)
		return PARSE_ERROR_INVALID_ALLOCATION;

	k->alloc_min = amin;
	k->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_combat(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");
	assert(k);

	k->ac = parser_getint(p, "ac");
	k->dd = hd.dice;
	k->ds = hd.sides;
	k->to_h = parser_getrand(p, "to-h");
	k->to_d = parser_getrand(p, "to-d");
	k->to_a = parser_getrand(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_charges(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->charge = parser_getrand(p, "charges");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_pile(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->gen_mult_prob = parser_getint(p, "prob");
	k->stack_size = parser_getrand(p, "stack");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_flags(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	char *s = string_make(parser_getstr(p, "flags"));
	char *t;
	assert(k);

	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(k->flags, OF_SIZE, obj_flags, t))
			found = true;
		if (!grab_flag(k->kind_flags, KF_SIZE, kind_flags, t))
			found = true;
		if (grab_element_flag(k->el_info, t))
			found = true;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_object_power(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->power = parser_getint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_effect(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));

	if (!k)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (k->effect) {
		effect = k->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		k->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_object_param(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct effect *effect = k->effect;

	if (!k)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->params[1] = parser_getint(p, "p2");

	if (parser_hasval(p, "p3"))
		effect->params[2] = parser_getint(p, "p3");

	return PARSE_ERROR_NONE;
}


static enum parser_error parse_object_dice(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	dice_t *dice = NULL;
	struct effect *effect = k->effect;
	const char *string = NULL;

	if (!k)
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

static enum parser_error parse_object_expr(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct effect *effect = k->effect;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!k)
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

	function = spell_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_msg(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);
	k->effect_msg = string_append(k->effect_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_time(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->time = parser_getrand(p, "time");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_desc(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);
	k->text = string_append(k->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_pval(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->pval = parser_getrand(p, "pval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_values(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	char *s;
	char *t;
	assert(k);

	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		int value = 0;
		int index = 0;
		char *name;
		bool found = false;
		if (!grab_rand_value(k->modifiers, obj_mods, t))
			found = true;
		if (!grab_index_and_int(&value, &index, elements, "BRAND_", t)) {
			struct brand *b;
			found = true;
			b = mem_zalloc(sizeof *b);
			b->name = string_make(brand_names[index]);
			b->element = index;
			b->multiplier = value;
			b->next = k->brands;
			k->brands = b;
			add_game_brand(b);
		}
		if (!grab_index_and_int(&value, &index, slays, "SLAY_", t)) {
			struct slay *s;
			found = true;
			s = mem_zalloc(sizeof *s);
			s->name = string_make(slay_names[index]);
			s->race_flag = index;
			s->multiplier = value;
			s->next = k->slays;
			k->slays = s;
			add_game_slay(s);
		} else if (!grab_base_and_int(&value, &name, t)) {
			struct slay *s;
			found = true;
			s = mem_zalloc(sizeof *s);
			s->name = string_make(name);
			s->multiplier = value;
			s->next = k->slays;
			k->slays = s;
			add_game_slay(s);
		}
		if (!grab_index_and_int(&value, &index, elements, "RES_", t)) {
			found = true;
			k->el_info[index].res_level = value;
		}
		if (!found)
			break;

		t = strtok(NULL, " |");
	}

	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}


struct parser *init_parse_object(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name int index str name", parse_object_name);
	parser_reg(p, "graphics char glyph sym color", parse_object_graphics);
	parser_reg(p, "type sym tval", parse_object_type);
	parser_reg(p, "properties int level int weight int cost", parse_object_properties);
	parser_reg(p, "alloc int common str minmax", parse_object_alloc);
	parser_reg(p, "combat int ac rand hd rand to-h rand to-d rand to-a", parse_object_combat);
	parser_reg(p, "charges rand charges", parse_object_charges);
	parser_reg(p, "pile int prob rand stack", parse_object_pile);
	parser_reg(p, "flags str flags", parse_object_flags);
	parser_reg(p, "power int power", parse_object_power);
	parser_reg(p, "effect sym eff ?sym type ?int xtra", parse_object_effect);
	parser_reg(p, "param int p2 ?int p3", parse_object_param);
	parser_reg(p, "dice str dice", parse_object_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_object_expr);
	parser_reg(p, "msg str text", parse_object_msg);
	parser_reg(p, "time rand time", parse_object_time);
	parser_reg(p, "pval rand pval", parse_object_pval);
	parser_reg(p, "values str values", parse_object_values);
	parser_reg(p, "desc str text", parse_object_desc);
	return p;
}

static errr run_parse_object(struct parser *p) {
	return parse_file(p, "object");
}

static errr finish_parse_object(struct parser *p) {
	struct object_kind *k, *next = NULL;

	/* scan the list for the max id */
	z_info->k_max = 0;
	k = parser_priv(p);
	while (k) {
		if (k->kidx > z_info->k_max)
			z_info->k_max = k->kidx;
		k = k->next;
	}

	/* allocate the direct access list and copy the data to it */
	k_info = mem_zalloc((z_info->k_max + 1) * sizeof(*k));
	for (k = parser_priv(p); k; k = next) {
		memcpy(&k_info[k->kidx], k, sizeof(*k));

		/* Add base kind flags to kind kind flags */
		kf_union(k_info[k->kidx].kind_flags, kb_info[k->tval].kind_flags);

		next = k->next;
		if (next)
			k_info[k->kidx].next = &k_info[next->kidx];
		else
			k_info[k->kidx].next = NULL;
		mem_free(k);
	}
	z_info->k_max += 1;

	/*objkinds = parser_priv(p); not used yet, when used, remove the mem_free(k); above */
	parser_destroy(p);
	return 0;
}

static void cleanup_object(void)
{
	int idx;
	for (idx = 0; idx < z_info->k_max; idx++) {
		string_free(k_info[idx].name);
		mem_free(k_info[idx].text);
		mem_free(k_info[idx].effect_msg);
		free_brand(k_info[idx].brands);
		free_slay(k_info[idx].slays);
		free_effect(k_info[idx].effect);
	}
	mem_free(k_info);
}

static struct file_parser object_parser = {
	"object",
	init_parse_object,
	run_parse_object,
	finish_parse_object,
	cleanup_object
};

/**
 * Parsing functions for activation.txt
 */
static enum parser_error parse_act_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct activation *h = parser_priv(p);

	struct activation *act = mem_zalloc(sizeof *act);
	act->next = h;
	parser_setpriv(p, act);
	act->name = string_make(name);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_aim(struct parser *p) {
	struct activation *act = parser_priv(p);
	int val;
	assert(act);

	val = parser_getuint(p, "aim");
	act->aim = val ? true : false;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_power(struct parser *p) {
	struct activation *act = parser_priv(p);
	assert(act);

	act->power = parser_getuint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_effect(struct parser *p) {
	struct activation *act = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));

	if (!act)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (act->effect) {
		effect = act->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		act->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_act_param(struct parser *p) {
	struct activation *act = parser_priv(p);
	struct effect *effect = act->effect;

	if (!act)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->params[1] = parser_getint(p, "p2");

	if (parser_hasval(p, "p3"))
		effect->params[2] = parser_getint(p, "p3");

	return PARSE_ERROR_NONE;
}


static enum parser_error parse_act_dice(struct parser *p) {
	struct activation *act = parser_priv(p);
	struct effect *effect = act->effect;
	dice_t *dice = NULL;
	const char *string = NULL;

	if (!act)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	/* Go to the correct effect */
	while (effect->next) effect = effect->next;

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

static enum parser_error parse_act_expr(struct parser *p) {
	struct activation *act = parser_priv(p);
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	struct effect *effect = act->effect;
	const char *name;
	const char *base;
	const char *expr;

	if (!act)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there are no dice, assume that this is human and not parser error. */
	if (act->effect->dice == NULL)
		return PARSE_ERROR_NONE;

	/* Go to the correct effect */
	while (effect->next) effect = effect->next;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = spell_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_msg(struct parser *p) {
	struct activation *act = parser_priv(p);
	assert(act);
	act->message = string_append(act->message, parser_getstr(p, "msg"));
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_act_desc(struct parser *p) {
	struct activation *act = parser_priv(p);

	if (!act)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	act->desc = string_append(act->desc, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_act(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_act_name);
	parser_reg(p, "aim uint aim", parse_act_aim);
	parser_reg(p, "power uint power", parse_act_power);
	parser_reg(p, "effect sym eff ?sym type ?int xtra", parse_act_effect);
	parser_reg(p, "param int p2 ?int p3", parse_act_param);
	parser_reg(p, "dice str dice", parse_act_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_act_expr);
	parser_reg(p, "msg str msg", parse_act_msg);
	parser_reg(p, "desc str desc", parse_act_desc);
	return p;
}

static errr run_parse_act(struct parser *p) {
	return parse_file(p, "activation");
}

static errr finish_parse_act(struct parser *p) {
	struct activation *act, *next = NULL;
	int count = 1;

	/* Count the entries */
	z_info->act_max = 0;
	act = parser_priv(p);
	while (act) {
		z_info->act_max++;
		act = act->next;
	}

	/* Allocate the direct access list and copy the data to it */
	activations = mem_zalloc((z_info->act_max + 1) * sizeof(*act));
	for (act = parser_priv(p); act; act = next, count++) {
		memcpy(&activations[count], act, sizeof(*act));
		activations[count].index = count;
		next = act->next;
		if (next)
			activations[count].next = &activations[count + 1];
		else
			activations[count].next = NULL;

		mem_free(act);
	}
	z_info->act_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_act(void)
{
	int idx;
	for (idx = 0; idx < z_info->act_max; idx++) {
		string_free(activations[idx].name);
		mem_free(activations[idx].desc);
		mem_free(activations[idx].message);
		free_effect(activations[idx].effect);
	}
	mem_free(activations);
}

static struct file_parser act_parser = {
	"activation",
	init_parse_act,
	run_parse_act,
	finish_parse_act,
	cleanup_act
};

/**
 * Parsing functions for artifact.txt
 */
static enum parser_error parse_artifact_name(struct parser *p) {
	size_t i;
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct artifact *h = parser_priv(p);

	struct artifact *a = mem_zalloc(sizeof *a);
	a->next = h;
	parser_setpriv(p, a);
	a->aidx = idx;
	a->name = string_make(name);

	/* Ignore all base elements */
	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++)
		a->el_info[i].flags |= EL_INFO_IGNORE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_base_object(struct parser *p) {
	struct artifact *a = parser_priv(p);
	int tval, sval;
	const char *sval_name;

	assert(a);

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	a->tval = tval;

	sval_name = parser_getsym(p, "sval");
	sval = lookup_sval(a->tval, sval_name);
	if (sval < 0)
		return write_dummy_object_record(a, sval_name);
	a->sval = sval;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_graphics(struct parser *p) {
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	struct artifact *a = parser_priv(p);
	struct object_kind *k = lookup_kind(a->tval, a->sval);
	assert(a);
	assert(k);

	if (!kf_has(k->kind_flags, KF_INSTA_ART))
		return PARSE_ERROR_NOT_SPECIAL_ARTIFACT;

	k->d_char = glyph;
	if (strlen(color) > 1)
		k->d_attr = color_text_to_attr(color);
	else
		k->d_attr = color_char_to_attr(color[0]);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_info(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->level = parser_getint(p, "level");
	a->weight = parser_getint(p, "weight");
	a->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_alloc(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;
	assert(a);

	a->alloc_prob = parser_getint(p, "common");
	if (sscanf(tmp, "%d to %d", &amin, &amax) != 2)
		return PARSE_ERROR_INVALID_ALLOCATION;

	if (amin > 255 || amax > 255 || amin < 0 || amax < 0)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	a->alloc_min = amin;
	a->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_power(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");
	assert(a);

	a->ac = parser_getint(p, "ac");
	a->dd = hd.dice;
	a->ds = hd.sides;
	a->to_h = parser_getint(p, "to-h");
	a->to_d = parser_getint(p, "to-d");
	a->to_a = parser_getint(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_flags(struct parser *p) {
	struct artifact *a = parser_priv(p);
	char *s;
	char *t;
	assert(a);

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));

	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(a->flags, OF_SIZE, obj_flags, t))
			found = true;
		if (grab_element_flag(a->el_info, t))
			found = true;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_act(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *name = parser_getstr(p, "name");

	if (!a)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	a->activation = findact(name);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_time(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->time = parser_getrand(p, "time");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_msg(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->alt_msg = string_append(a->alt_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_values(struct parser *p) {
	struct artifact *a = parser_priv(p);
	char *s; 
	char *t;
	assert(a);

	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		bool found = false;
		int value = 0;
		int index = 0;
		char *name;
		if (!grab_int_value(a->modifiers, obj_mods, t))
			found = true;
		if (!grab_index_and_int(&value, &index, elements, "BRAND_", t)) {
			struct brand *b;
			found = true;
			b = mem_zalloc(sizeof *b);
			b->name = string_make(brand_names[index]);
			b->element = index;
			b->multiplier = value;
			b->next = a->brands;
			a->brands = b;
			add_game_brand(b);
		}
		if (!grab_index_and_int(&value, &index, slays, "SLAY_", t)) {
			struct slay *s;
			found = true;
			s = mem_zalloc(sizeof *s);
			s->name = string_make(slay_names[index]);
			s->race_flag = index;
			s->multiplier = value;
			s->next = a->slays;
			a->slays = s;
			add_game_slay(s);
		} else if (!grab_base_and_int(&value, &name, t)) {
			struct slay *s;
			found = true;
			s = mem_zalloc(sizeof *s);
			s->name = string_make(name);
			s->multiplier = value;
			s->next = a->slays;
			a->slays = s;
			add_game_slay(s);
		}
		if (!grab_index_and_int(&value, &index, elements, "RES_", t)) {
			found = true;
			a->el_info[index].res_level = value;
		}
		if (!found)
			break;

		t = strtok(NULL, " |");
	}

	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_desc(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->text = string_append(a->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_artifact(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name int index str name", parse_artifact_name);
	parser_reg(p, "base-object sym tval sym sval", parse_artifact_base_object);
	parser_reg(p, "graphics char glyph sym color", parse_artifact_graphics);
	parser_reg(p, "info int level int weight int cost", parse_artifact_info);
	parser_reg(p, "alloc int common str minmax", parse_artifact_alloc);
	parser_reg(p, "power int ac rand hd int to-h int to-d int to-a",
			   parse_artifact_power);
	parser_reg(p, "flags ?str flags", parse_artifact_flags);
	parser_reg(p, "act str name", parse_artifact_act);
	parser_reg(p, "time rand time", parse_artifact_time);
	parser_reg(p, "msg str text", parse_artifact_msg);
	parser_reg(p, "values str values", parse_artifact_values);
	parser_reg(p, "desc str text", parse_artifact_desc);
	return p;
}

static errr run_parse_artifact(struct parser *p) {
	return parse_file(p, "artifact");
}

static errr finish_parse_artifact(struct parser *p) {
	struct artifact *a, *n;
	int none;

	/* scan the list for the max id */
	z_info->a_max = 0;
	a = parser_priv(p);
	while (a) {
		if (a->aidx > z_info->a_max)
			z_info->a_max = a->aidx;
		a = a->next;
	}

	/* allocate the direct access list and copy the data to it */
	a_info = mem_zalloc((z_info->a_max + 1) * sizeof(*a));
	for (a = parser_priv(p); a; a = n) {
		memcpy(&a_info[a->aidx], a, sizeof(*a));
		n = a->next;
		if (n)
			a_info[a->aidx].next = &a_info[n->aidx];
		else
			a_info[a->aidx].next = NULL;

		mem_free(a);
	}
	z_info->a_max += 1;

	/* Now we're done with object kinds, record kinds for generic objects */
	none = tval_find_idx("none");
	unknown_item_kind = lookup_kind(none, lookup_sval(none, "<unknown item>"));
	unknown_gold_kind = lookup_kind(none,
									lookup_sval(none, "<unknown treasure>"));
	pile_kind = lookup_kind(none, lookup_sval(none, "<pile>"));

	parser_destroy(p);
	return 0;
}

static void cleanup_artifact(void)
{
	int idx;
	for (idx = 0; idx < z_info->a_max; idx++) {
		string_free(a_info[idx].name);
		mem_free(a_info[idx].alt_msg);
		mem_free(a_info[idx].text);
		free_brand(a_info[idx].brands);
		free_slay(a_info[idx].slays);
	}
	mem_free(a_info);
}

static struct file_parser artifact_parser = {
	"artifact",
	init_parse_artifact,
	run_parse_artifact,
	finish_parse_artifact,
	cleanup_artifact
};

/**
 * Parsing functions for names.txt (random name fragments)
 */
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
	return parse_file(p, "names");
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
 * Parsing functions for trap.txt
 */
static enum parser_error parse_trap_name(struct parser *p) {
    int idx = parser_getuint(p, "index");
    const char *name = parser_getsym(p, "name");
    const char *desc = parser_getstr(p, "desc");
    struct trap_kind *h = parser_priv(p);

    struct trap_kind *t = mem_zalloc(sizeof *t);
    t->next = h;
    t->tidx = idx;
    t->name = string_make(name);
	t->desc = string_make(desc);
    parser_setpriv(p, t);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_graphics(struct parser *p) {
    char glyph = parser_getchar(p, "glyph");
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
			mem_free(s);
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

	function = spell_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_trap_desc(struct parser *p) {
    struct trap_kind *t = parser_priv(p);
    assert(t);

    t->text = string_append(t->text, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

struct parser *init_parse_trap(void) {
    struct parser *p = parser_new();
    parser_setpriv(p, NULL);
    parser_reg(p, "name uint index sym name str desc", parse_trap_name);
    parser_reg(p, "graphics char glyph sym color", parse_trap_graphics);
    parser_reg(p, "appear uint rarity uint mindepth uint maxnum", parse_trap_appear);
    parser_reg(p, "flags ?str flags", parse_trap_flags);
	parser_reg(p, "effect sym eff ?sym type ?int xtra", parse_trap_effect);
	parser_reg(p, "dice str dice", parse_trap_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_trap_expr);
    parser_reg(p, "desc str text", parse_trap_desc);
    return p;
}

static errr run_parse_trap(struct parser *p) {
    return parse_file(p, "trap");
}

static errr finish_parse_trap(struct parser *p) {
	struct trap_kind *t, *n;
	
	/* scan the list for the max id */
	z_info->trap_max = 0;
	t = parser_priv(p);
	while (t) {
		if (t->tidx > z_info->trap_max)
			z_info->trap_max = t->tidx;
		t = t->next;
	}

	z_info->trap_max += 1;
	trap_info = mem_zalloc((z_info->trap_max) * sizeof(*t));
    for (t = parser_priv(p); t; t = t->next) {
		if (t->tidx >= z_info->trap_max)
			continue;
		memcpy(&trap_info[t->tidx], t, sizeof(*t));
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
		free_effect(trap_info[i].effect);
		string_free(trap_info[i].desc);
	}
	mem_free(trap_info);
}

struct file_parser trap_parser = {
    "trap",
    init_parse_trap,
    run_parse_trap,
    finish_parse_trap,
    cleanup_trap
};

/**
 * Parsing functions for terrain.txt
 */
static enum parser_error parse_feat_name(struct parser *p) {
	int idx = parser_getuint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct feature *h = parser_priv(p);

	struct feature *f = mem_zalloc(sizeof *f);
	f->next = h;
	f->fidx = idx;
	f->mimic = idx;
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
	unsigned int idx = parser_getuint(p, "index");
	struct feature *f = parser_priv(p);

	if (!f)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f->mimic = idx;
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

struct parser *init_parse_feat(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name uint index str name", parse_feat_name);
	parser_reg(p, "graphics char glyph sym color", parse_feat_graphics);
	parser_reg(p, "mimic uint index", parse_feat_mimic);
	parser_reg(p, "priority uint priority", parse_feat_priority);
	parser_reg(p, "flags ?str flags", parse_feat_flags);
	parser_reg(p, "info int shopnum int dig", parse_feat_info);
    parser_reg(p, "desc str text", parse_feat_desc);
	return p;
}

static errr run_parse_feat(struct parser *p) {
	return parse_file(p, "terrain");
}

static errr finish_parse_feat(struct parser *p) {
	struct feature *f, *n;

	/* scan the list for the max id */
	z_info->f_max = 0;
	f = parser_priv(p);
	while (f) {
		if (f->fidx > z_info->f_max)
			z_info->f_max = f->fidx;
		f = f->next;
	}

	/* allocate the direct access list and copy the data to it */
	f_info = mem_zalloc((z_info->f_max + 1) * sizeof(*f));
	for (f = parser_priv(p); f; f = n) {
		memcpy(&f_info[f->fidx], f, sizeof(*f));
		n = f->next;
		if (n)
			f_info[f->fidx].next = &f_info[n->fidx];
		else
			f_info[f->fidx].next = NULL;
		mem_free(f);
	}
	z_info->f_max += 1;

	/* Set the terrain constants */
	set_terrain();

	parser_destroy(p);
	return 0;
}

static void cleanup_feat(void) {
	int idx;
	for (idx = 0; idx < z_info->f_max; idx++) {
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
 * Parsing functions for ego-item.txt
 */
static enum parser_error parse_ego_name(struct parser *p) {
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct ego_item *h = parser_priv(p);

	struct ego_item *e = mem_zalloc(sizeof *e);
	e->next = h;
	parser_setpriv(p, e);
	e->eidx = idx;
	e->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_info(struct parser *p) {
	int level = parser_getint(p, "level");
	int rarity = parser_getint(p, "rarity");
	int cost = parser_getint(p, "cost");
	int rating = parser_getint(p, "rating");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->level = level;
	e->rarity = rarity;
	e->cost = cost;
	e->rating = rating;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_alloc(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;

	e->alloc_prob = parser_getint(p, "common");
	if (sscanf(tmp, "%d to %d", &amin, &amax) != 2)
		return PARSE_ERROR_INVALID_ALLOCATION;

	if (amin > 255 || amax > 255 || amin < 0 || amax < 0)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	e->alloc_min = amin;
	e->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_type(struct parser *p) {
	struct ego_poss_item *poss;
	int i;
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	bool found_one_kind = false;

	struct ego_item *e = parser_priv(p);
	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	/* Find all the right object kinds */
	for (i = 0; i < z_info->k_max; i++) {
		if (k_info[i].tval != tval) continue;
		poss = mem_zalloc(sizeof(struct ego_poss_item));
		poss->kidx = i;
		poss->next = e->poss_items;
		e->poss_items = poss;
		found_one_kind = true;
	}

	if (!found_one_kind)
		return PARSE_ERROR_NO_KIND_FOR_EGO_TYPE;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_item(struct parser *p) {
	struct ego_poss_item *poss;
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	int sval = lookup_sval(tval, parser_getsym(p, "sval"));

	struct ego_item *e = parser_priv(p);
	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	poss = mem_zalloc(sizeof(struct ego_poss_item));
	poss->kidx = lookup_kind(tval, sval)->kidx;
	poss->next = e->poss_items;
	e->poss_items = poss;

	if (poss->kidx <= 0)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_combat(struct parser *p) {
	struct random th = parser_getrand(p, "th");
	struct random td = parser_getrand(p, "td");
	struct random ta = parser_getrand(p, "ta");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	e->to_h = th;
	e->to_d = td;
	e->to_a = ta;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_min(struct parser *p) {
	int th = parser_getint(p, "th");
	int td = parser_getint(p, "td");
	int ta = parser_getint(p, "ta");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	e->min_to_h = th;
	e->min_to_d = td;
	e->min_to_a = ta;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_effect(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (e->effect) {
		effect = e->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		e->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_ego_dice(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	dice_t *dice = NULL;
	const char *string = NULL;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		e->effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_time(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	assert(e);

	e->time = parser_getrand(p, "time");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_flags(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *flags;
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	t = strtok(flags, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(e->flags, OF_SIZE, obj_flags, t))
			found = true;
		if (!grab_flag(e->kind_flags, KF_SIZE, kind_flags, t))
			found = true;
		if (grab_element_flag(e->el_info, t))
			found = true;
		if (!found)
			break;
		t = strtok(NULL, " |");
	}
	mem_free(flags);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_flags_off(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *flags;
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	t = strtok(flags, " |");
	while (t) {
		if (grab_flag(e->flags_off, OF_SIZE, obj_flags, t))
			return PARSE_ERROR_INVALID_FLAG;
		t = strtok(NULL, " |");
	}
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_values(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *s; 
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "values"))
		return PARSE_ERROR_MISSING_FIELD;

	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		bool found = false;
		int value = 0;
		int index = 0;
		char *name;
		if (!grab_rand_value(e->modifiers, obj_mods, t))
			found = true;
		if (!grab_index_and_int(&value, &index, elements, "BRAND_", t)) {
			struct brand *b;
			found = true;
			b = mem_zalloc(sizeof *b);
			b->name = string_make(brand_names[index]);
			b->element = index;
			b->multiplier = value;
			b->next = e->brands;
			e->brands = b;
			add_game_brand(b);
		}
		if (!grab_index_and_int(&value, &index, slays, "SLAY_", t)) {
			struct slay *s;
			found = true;
			s = mem_zalloc(sizeof *s);
			s->name = string_make(slay_names[index]);
			s->race_flag = index;
			s->multiplier = value;
			s->next = e->slays;
			e->slays = s;
			add_game_slay(s);
		} else if (!grab_base_and_int(&value, &name, t)) {
			struct slay *s;
			found = true;
			s = mem_zalloc(sizeof *s);
			s->name = string_make(name);
			s->multiplier = value;
			s->next = e->slays;
			e->slays = s;
			add_game_slay(s);
		}
		if (!grab_index_and_int(&value, &index, elements, "RES_", t)) {
			found = true;
			e->el_info[index].res_level = value;
		}
		if (!found)
			break;

		t = strtok(NULL, " |");
	}

	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_min_val(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *s; 
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "min_values"))
		return PARSE_ERROR_MISSING_FIELD;

	s = string_make(parser_getstr(p, "min_values"));
	t = strtok(s, " |");

	while (t) {
		bool found = false;
		if (!grab_int_value(e->min_modifiers, obj_mods, t))
			found = true;
		if (!found)
			break;

		t = strtok(NULL, " |");
	}

	mem_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_desc(struct parser *p) {
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->text = string_append(e->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_ego(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name int index str name", parse_ego_name);
	parser_reg(p, "info int level int rarity int cost int rating", parse_ego_info);
	parser_reg(p, "alloc int common str minmax", parse_ego_alloc);
	parser_reg(p, "type sym tval", parse_ego_type);
	parser_reg(p, "item sym tval sym sval", parse_ego_item);
	parser_reg(p, "combat rand th rand td rand ta", parse_ego_combat);
	parser_reg(p, "min-combat int th int td int ta", parse_ego_min);
	parser_reg(p, "effect sym eff ?sym type ?int xtra", parse_ego_effect);
	parser_reg(p, "dice str dice", parse_ego_dice);
	parser_reg(p, "time rand time", parse_ego_time);
	parser_reg(p, "flags ?str flags", parse_ego_flags);
	parser_reg(p, "flags-off ?str flags", parse_ego_flags_off);
	parser_reg(p, "values str values", parse_ego_values);
	parser_reg(p, "min-values str min_values", parse_ego_min_val);
	parser_reg(p, "desc str text", parse_ego_desc);
	return p;
}

static errr run_parse_ego(struct parser *p) {
	return parse_file(p, "ego_item");
}

static errr finish_parse_ego(struct parser *p) {
	struct ego_item *e, *n;

	/* scan the list for the max id */
	z_info->e_max = 0;
	e = parser_priv(p);
	while (e) {
		if (e->eidx > z_info->e_max)
			z_info->e_max = e->eidx;
		e = e->next;
	}

	/* allocate the direct access list and copy the data to it */
	e_info = mem_zalloc((z_info->e_max + 1) * sizeof(*e));
	for (e = parser_priv(p); e; e = n) {
		memcpy(&e_info[e->eidx], e, sizeof(*e));
		n = e->next;
		if (n)
			e_info[e->eidx].next = &e_info[n->eidx];
		else
			e_info[e->eidx].next = NULL;
		mem_free(e);
	}
	z_info->e_max += 1;

	create_slay_cache(e_info);

	parser_destroy(p);
	return 0;
}

static void cleanup_ego(void)
{
	int idx;
	struct ego_poss_item *poss, *pn;
	for (idx = 0; idx < z_info->e_max; idx++) {
		string_free(e_info[idx].name);
		mem_free(e_info[idx].text);
		free_brand(e_info[idx].brands);
		free_slay(e_info[idx].slays);
		free_effect(e_info[idx].effect);
		poss = e_info[idx].poss_items;
		while (poss) {
			pn = poss->next;
			mem_free(poss);
			poss = pn;
		}
	}
	mem_free(e_info);
	free_slay_cache();
}

static struct file_parser ego_parser = {
	"ego_item",
	init_parse_ego,
	run_parse_ego,
	finish_parse_ego,
	cleanup_ego
};

/**
 * Parsing functions for body.txt
 */
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

struct parser *init_parse_body(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "body str name", parse_body_body);
	parser_reg(p, "slot sym slot sym name", parse_body_slot);
	return p;
}

static errr run_parse_body(struct parser *p) {
	return parse_file(p, "body");
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
 * Parsing functions for prace.txt
 */
static enum parser_error parse_p_race_name(struct parser *p) {
	struct player_race *h = parser_priv(p);
	struct player_race *r = mem_zalloc(sizeof *r);

	r->next = h;
	r->ridx = parser_getuint(p, "index");
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

static enum parser_error parse_p_race_skill_disarm(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_DISARM] = parser_getint(p, "disarm");
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

static enum parser_error parse_p_race_skill_search_freq(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "freq");
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

static enum parser_error parse_p_race_info(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->r_mhp = parser_getint(p, "mhp");
	r->r_exp = parser_getint(p, "exp");
	r->infra = parser_getint(p, "infra");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_p_race_history(struct parser *p) {
	struct player_race *r = parser_priv(p);
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->history = findchart(histories, parser_getuint(p, "hist"));
	r->b_age = parser_getint(p, "b-age");
	r->m_age = parser_getint(p, "m-age");
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
		if (grab_flag(r->flags, OF_SIZE, obj_flags, s))
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
		if (!grab_index_and_int(&value, &index, elements, "RES_", t)) {
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
	parser_reg(p, "name uint index str name", parse_p_race_name);
	parser_reg(p, "stats int str int int int wis int dex int con", parse_p_race_stats);
	parser_reg(p, "skill-disarm int disarm", parse_p_race_skill_disarm);
	parser_reg(p, "skill-device int device", parse_p_race_skill_device);
	parser_reg(p, "skill-save int save", parse_p_race_skill_save);
	parser_reg(p, "skill-stealth int stealth", parse_p_race_skill_stealth);
	parser_reg(p, "skill-search int search", parse_p_race_skill_search);
	parser_reg(p, "skill-search-freq int freq", parse_p_race_skill_search_freq);
	parser_reg(p, "skill-melee int melee", parse_p_race_skill_melee);
	parser_reg(p, "skill-shoot int shoot", parse_p_race_skill_shoot);
	parser_reg(p, "skill-throw int throw", parse_p_race_skill_throw);
	parser_reg(p, "skill-dig int dig", parse_p_race_skill_dig);
	parser_reg(p, "info int mhp int exp int infra", parse_p_race_info);
	parser_reg(p, "history uint hist int b-age int m-age", parse_p_race_history);
	parser_reg(p, "height int base_hgt int mod_hgt", parse_p_race_height);
	parser_reg(p, "weight int base_wgt int mod_wgt", parse_p_race_weight);
	parser_reg(p, "obj-flags ?str flags", parse_p_race_obj_flags);
	parser_reg(p, "player-flags ?str flags", parse_p_race_play_flags);
	parser_reg(p, "values str values", parse_p_race_values);
	return p;
}

static errr run_parse_p_race(struct parser *p) {
	return parse_file(p, "p_race");
}

static errr finish_parse_p_race(struct parser *p) {
	races = parser_priv(p);
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
 * Parsing functions for class.txt
 */
static enum parser_error parse_class_name(struct parser *p) {
	struct player_class *h = parser_priv(p);
	struct player_class *c = mem_zalloc(sizeof *c);
	c->cidx = parser_getuint(p, "index");
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

static enum parser_error parse_class_skill_disarm(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_DISARM] = parser_getint(p, "base");
	c->x_skills[SKILL_DISARM] = parser_getint(p, "incr");
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

static enum parser_error parse_class_skill_search_freq(struct parser *p) {
	struct player_class *c = parser_priv(p);
	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "base");
	c->x_skills[SKILL_SEARCH_FREQUENCY] = parser_getint(p, "incr");
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

static enum parser_error parse_class_info(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->c_mhp = parser_getint(p, "mhp");
	c->c_exp = parser_getint(p, "exp");
	c->sense_base = parser_getint(p, "sense-base");
	c->sense_div = parser_getint(p, "sense-div");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_attack(struct parser *p) {
	struct player_class *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->max_attacks = parser_getint(p, "max-attacks");
	c->min_weight = parser_getint(p, "min-weight");
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

static enum parser_error parse_class_equip(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct start_item *si;
	int tval, sval;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	si = mem_zalloc(sizeof *si);
	si->kind = lookup_kind(tval, sval);
	si->min = parser_getuint(p, "min");
	si->max = parser_getuint(p, "max");

	if (si->min > 99 || si->max > 99) {
		mem_free(si->kind);
		return PARSE_ERROR_INVALID_ITEM_NUMBER;
	}

	si->next = c->start_items;
	c->start_items = si;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_flags(struct parser *p) {
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
	c->magic.spell_realm = &realms[parser_getuint(p, "realm")];
	num_books = parser_getuint(p, "books");
	c->magic.books = mem_zalloc(num_books * sizeof(struct class_book));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_book(struct parser *p) {
	struct player_class *c = parser_priv(p);
	int tval, sval, spells;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	c->magic.books[c->magic.num_books].tval = tval;
	c->magic.books[c->magic.num_books].sval = sval;
	spells = parser_getuint(p, "spells");
	c->magic.books[c->magic.num_books].spells =
		mem_zalloc(spells * sizeof(struct class_spell));
	c->magic.books[c->magic.num_books++].realm = parser_getuint(p, "realm");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_class_spell(struct parser *p) {
	struct player_class *c = parser_priv(p);
	struct class_book *book = &c->magic.books[c->magic.num_books - 1];

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

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

static enum parser_error parse_class_param(struct parser *p) {
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
	effect->params[1] = parser_getint(p, "p2");

	if (parser_hasval(p, "p3"))
		effect->params[2] = parser_getint(p, "p3");

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

	function = spell_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

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
	parser_reg(p, "name uint index str name", parse_class_name);
	parser_reg(p, "stats int str int int int wis int dex int con", parse_class_stats);
	parser_reg(p, "skill-disarm int base int incr", parse_class_skill_disarm);
	parser_reg(p, "skill-device int base int incr", parse_class_skill_device);
	parser_reg(p, "skill-save int base int incr", parse_class_skill_save);
	parser_reg(p, "skill-stealth int base int incr", parse_class_skill_stealth);
	parser_reg(p, "skill-search int base int incr", parse_class_skill_search);
	parser_reg(p, "skill-search-freq int base int incr", parse_class_skill_search_freq);
	parser_reg(p, "skill-melee int base int incr", parse_class_skill_melee);
	parser_reg(p, "skill-shoot int base int incr", parse_class_skill_shoot);
	parser_reg(p, "skill-throw int base int incr", parse_class_skill_throw);
	parser_reg(p, "skill-dig int base int incr", parse_class_skill_dig);
	parser_reg(p, "info int mhp int exp int sense-base int sense-div", parse_class_info);
	parser_reg(p, "attack int max-attacks int min-weight int att-multiply", parse_class_attack);
	parser_reg(p, "title str title", parse_class_title);
	parser_reg(p, "equip sym tval sym sval uint min uint max", parse_class_equip);
	parser_reg(p, "flags ?str flags", parse_class_flags);
	parser_reg(p, "magic uint first uint weight uint realm uint books", parse_class_magic);
	parser_reg(p, "book sym tval sym sval uint spells uint realm", parse_class_book);
	parser_reg(p, "spell sym name int level int mana int fail int exp", parse_class_spell);
	parser_reg(p, "effect sym eff ?sym type ?int xtra", parse_class_effect);
	parser_reg(p, "param int p2 ?int p3", parse_class_param);
	parser_reg(p, "dice str dice", parse_class_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_class_expr);
	parser_reg(p, "desc str desc", parse_class_desc);
	return p;
}

static errr run_parse_class(struct parser *p) {
	return parse_file(p, "class");
}

static errr finish_parse_class(struct parser *p) {
	classes = parser_priv(p);
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
 * Parsing functions for history.txt
 */
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
	return parse_file(p, "history");
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
 * Parsing functions for flavor.txt
 */
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
	return parse_file(p, "flavor");
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

static struct file_parser flavor_parser = {
	"flavor",
	init_parse_flavor,
	run_parse_flavor,
	finish_parse_flavor,
	cleanup_flavor
};


/**
 * Initialize hints
 */
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
	return parse_file(p, "hints");
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
 * Initialize monster pain messages
 */
static enum parser_error parse_pain_type(struct parser *p) {
	struct monster_pain *h = parser_priv(p);
	struct monster_pain *mp = mem_zalloc(sizeof *mp);
	mp->next = h;
	mp->pain_idx = parser_getuint(p, "index");
	parser_setpriv(p, mp);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pain_message(struct parser *p) {
	struct monster_pain *mp = parser_priv(p);
	int i;

	if (!mp)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < 7; i++)
		if (!mp->messages[i])
			break;
	if (i == 7)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	mp->messages[i] = string_make(parser_getstr(p, "message"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_pain(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "type uint index", parse_pain_type);
	parser_reg(p, "message str message", parse_pain_message);
	return p;
}

static errr run_parse_pain(struct parser *p) {
	return parse_file(p, "pain");
}

static errr finish_parse_pain(struct parser *p) {
	struct monster_pain *mp, *n;
		
	/* scan the list for the max id */
	z_info->mp_max = 0;
	mp = parser_priv(p);
	while (mp) {
		if (mp->pain_idx > z_info->mp_max)
			z_info->mp_max = mp->pain_idx;
		mp = mp->next;
	}

	/* allocate the direct access list and copy the data to it */
	pain_messages = mem_zalloc((z_info->mp_max + 1) * sizeof(*mp));
	for (mp = parser_priv(p); mp; mp = n) {
		memcpy(&pain_messages[mp->pain_idx], mp, sizeof(*mp));
		n = mp->next;
		if (n)
			pain_messages[mp->pain_idx].next = &pain_messages[n->pain_idx];
		else
			pain_messages[mp->pain_idx].next = NULL;
		mem_free(mp);
	}
	z_info->mp_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_pain(void)
{
	int idx, i;
	for (idx = 0; idx < z_info->mp_max; idx++) {
		for (i = 0; i < 7; i++) {
			string_free((char *)pain_messages[idx].messages[i]);
		}
	}
	mem_free(pain_messages);
}

static struct file_parser pain_parser = {
	"pain messages",
	init_parse_pain,
	run_parse_pain,
	finish_parse_pain,
	cleanup_pain
};


/**
 * Initialize monster pits
 */
static enum parser_error parse_pit_name(struct parser *p) {
	struct pit_profile *h = parser_priv(p);
	struct pit_profile *pit = mem_zalloc(sizeof *pit);
	pit->next = h;
	pit->pit_idx = parser_getuint(p, "index");
	pit->name = string_make(parser_getstr(p, "name"));
	pit->colors = NULL;
	pit->forbidden_monsters = NULL;
	parser_setpriv(p, pit);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_room(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pit->room_type = parser_getuint(p, "type");
	return PARSE_ERROR_NONE;
}
static enum parser_error parse_pit_alloc(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pit->rarity = parser_getuint(p, "rarity");
	pit->ave = parser_getuint(p, "level");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_obj_rarity(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pit->obj_rarity = parser_getuint(p, "obj_rarity");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_mon_base(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	struct pit_monster_profile *bases;
	struct monster_base *base = lookup_monster_base(parser_getsym(p, "base"));

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	else if (!base)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	bases = mem_zalloc(sizeof *bases);
	bases->base = base;
	bases->next = pit->bases;
	pit->bases = bases;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_mon_ban(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	struct pit_forbidden_monster *monsters;
	struct monster_race *r = lookup_monster(parser_getsym(p, "race"));

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	monsters = mem_zalloc(sizeof *monsters);
	monsters->race = r;
	monsters->next = pit->forbidden_monsters;
	pit->forbidden_monsters = monsters;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_color(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	struct pit_color_profile *colors;
	const char *color;
	int attr;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;

	colors = mem_zalloc(sizeof *colors);
	colors->color = attr;
	colors->next = pit->colors;
	pit->colors = colors;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_flags_req(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_flags_ban(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->forbidden_flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_spell_req(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_spell_ban(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->forbidden_spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_pit(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name uint index str name", parse_pit_name);
	parser_reg(p, "room uint type", parse_pit_room);
	parser_reg(p, "alloc uint rarity uint level", parse_pit_alloc);
	parser_reg(p, "obj-rarity uint obj_rarity", parse_pit_obj_rarity);
	parser_reg(p, "mon-base sym base", parse_pit_mon_base);
	parser_reg(p, "mon-ban sym race", parse_pit_mon_ban);
	parser_reg(p, "color sym color", parse_pit_color);
	parser_reg(p, "flags-req ?str flags", parse_pit_flags_req);
	parser_reg(p, "flags-ban ?str flags", parse_pit_flags_ban);
	parser_reg(p, "spell-req ?str spells", parse_pit_spell_req);
	parser_reg(p, "spell-ban ?str spells", parse_pit_spell_ban);
	return p;
}

static errr run_parse_pit(struct parser *p) {
	return parse_file(p, "pit");
}
 
static errr finish_parse_pit(struct parser *p) {
	struct pit_profile *pit, *n;
		
	/* scan the list for the max id */
	z_info->pit_max = 0;
	pit = parser_priv(p);
	while (pit) {
		if (pit->pit_idx > z_info->pit_max)
			z_info->pit_max = pit->pit_idx;
		pit = pit->next;
	}

	/* allocate the direct access list and copy the data to it */
	pit_info = mem_zalloc((z_info->pit_max + 1) * sizeof(*pit));
	for (pit = parser_priv(p); pit; pit = n) {
		memcpy(&pit_info[pit->pit_idx], pit, sizeof(*pit));
		n = pit->next;
		if (n)
			pit_info[pit->pit_idx].next = &pit_info[n->pit_idx];
		else
			pit_info[pit->pit_idx].next = NULL;

		mem_free(pit);
	}
	z_info->pit_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_pits(void)
{
	int idx;
	
	for (idx = 0; idx < z_info->pit_max; idx++) {
		struct pit_profile *pit = &pit_info[idx];
		struct pit_color_profile *c, *cn;
		struct pit_forbidden_monster *m, *mn;
		struct pit_monster_profile *b, *bn;
		
		c = pit->colors;
		while (c) {
			cn = c->next;
			mem_free(c);
			c = cn;
		}
		m = pit->forbidden_monsters;
		while (m) {
			mn = m->next;
			mem_free(m);
			m = mn;
		}
		b = pit->bases;
		while (b) {
			bn = b->next;
			mem_free(b);
			b = bn;
		}
		string_free((char *)pit_info[idx].name);
		
	}
	mem_free(pit_info);
}

static struct file_parser pit_parser = {
	"pits",
	init_parse_pit,
	run_parse_pit,
	finish_parse_pit,
	cleanup_pits
};


/**
 * A list of all the above parsers, plus those found in src/mon-init.c
 */
static struct {
	const char *name;
	struct file_parser *parser;
} pl[] = {
	{ "traps", &trap_parser },
	{ "features", &feat_parser },
	{ "object bases", &object_base_parser },
	{ "objects", &object_parser },
	{ "activations", &act_parser },
	{ "ego-items", &ego_parser },
	{ "artifacts", &artifact_parser },
	{ "monster pain messages", &pain_parser },
	{ "monster spells", &mon_spell_parser },
	{ "monster bases", &mon_base_parser },
	{ "monsters", &monster_parser },
	{ "monster pits" , &pit_parser },
	{ "monster lore" , &lore_parser },
	{ "quests", &quests_parser },
	{ "history charts", &history_parser },
	{ "bodies", &body_parser },
	{ "player races", &p_race_parser },
	{ "player classes", &class_parser },
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
extern struct init_module monmsg_module;

static struct init_module *modules[] = {
	&z_quark_module,
	&messages_module,
	&player_module,
	&arrays_module,
	&generate_module,
	&rune_module,
	&obj_make_module,
	&ignore_module,
	&mon_make_module,
	&store_module,
	&options_module,
	&monmsg_module,
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
	for (i = 0; modules[i]; i++)
		if (modules[i]->cleanup)
			modules[i]->cleanup();

	event_remove_all_handlers();

	/* Free the chunk list */
	for (i = 0; i < chunk_list_max; i++)
		cave_free(chunk_list[i]);
	mem_free(chunk_list);

	/* Free the main cave */
	if (cave_k) {
		cave_free(cave_k);
		cave_k = NULL;
	}
	if (cave) {
		cave_free(cave);
		cave = NULL;
	}

	/* Free the history */
	history_clear();

	monster_list_finalize();
	object_list_finalize();

	cleanup_game_constants();

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
	string_free(ANGBAND_DIR_SCORES);
	string_free(ANGBAND_DIR_INFO);
}
