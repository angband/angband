/**
 * \file generate.c
 * \brief Dungeon generation.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Erik Osheim, Nick McConnell
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
 * This is the top level dungeon generation file, which contains room profiles
 * (for determining what rooms are available and their parameters), cave
 * profiles (for determining the level generation function and parameters for
 * different styles of levels), initialisation functions for template rooms and
 * vaults, and the main level generation function (which calls the level
 * builders from gen-cave.c).
 *
 * See the "vault.txt" file for more on vault generation.
 * See the "room_template.txt" file for more room templates.
 */

#include "angband.h"
#include "cave.h"
#include "datafile.h"
#include "game-event.h"
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "math.h"
#include "mon-make.h"
#include "mon-move.h"
#include "mon-spell.h"
#include "monster.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-util.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/*
 * Array of pit types
 */
struct pit_profile *pit_info;
struct vault *vaults;
static struct cave_profile *cave_profiles;
struct dun_data *dun;
struct room_template *room_templates;

static const struct {
	const char *name;
	cave_builder builder;
} cave_builders[] = {
	#define DUN(a, b) { a, b##_gen },
	#include "list-dun-profiles.h"
	#undef DUN
};

static const struct {
	const char *name;
	int max_height;
	int max_width;
	room_builder builder;
} room_builders[] = {
	#define ROOM(a, b, c, d) { a, b, c, build_##d },
	#include "list-rooms.h"
	#undef ROOM
};

static const char *room_flags[] = {
	"NONE",
	#define ROOMF(a, b) #a,
	#include "list-room-flags.h"
	#undef ROOMF
	NULL
};


/**
 * Parsing functions for dungeon_profile.txt
 */
static enum parser_error parse_profile_name(struct parser *p) {
	struct cave_profile *h = parser_priv(p);
	struct cave_profile *c = mem_zalloc(sizeof *c);
	size_t i;

	c->name = string_make(parser_getstr(p, "name"));
	for (i = 0; i < N_ELEMENTS(cave_builders); i++)
		if (streq(c->name, cave_builders[i].name))
			break;

	if (i == N_ELEMENTS(cave_builders))
		return PARSE_ERROR_NO_BUILDER_FOUND;
	c->builder = cave_builders[i].builder;
	c->next = h;
	parser_setpriv(p, c);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_params(struct parser *p) {
	struct cave_profile *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->block_size = parser_getint(p, "block");
	c->dun_rooms = parser_getint(p, "rooms");
	c->dun_unusual = parser_getint(p, "unusual");
	c->max_rarity = parser_getint(p, "rarity");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_tunnel(struct parser *p) {
	struct cave_profile *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->tun.rnd = parser_getint(p, "rnd");
	c->tun.chg = parser_getint(p, "chg");
	c->tun.con = parser_getint(p, "con");
	c->tun.pen = parser_getint(p, "pen");
	c->tun.jct = parser_getint(p, "jct");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_streamer(struct parser *p) {
	struct cave_profile *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->str.den = parser_getint(p, "den");
	c->str.rng = parser_getint(p, "rng");
	c->str.mag = parser_getint(p, "mag");
	c->str.mc  = parser_getint(p, "mc");
	c->str.qua = parser_getint(p, "qua");
	c->str.qc  = parser_getint(p, "qc");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_room(struct parser *p) {
	struct cave_profile *c = parser_priv(p);
	struct room_profile *r = c->room_profiles;
	size_t i;

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the last valid room profile, then allocate a new one */
	if (!r) {
		c->room_profiles = mem_zalloc(sizeof(struct room_profile));
		r = c->room_profiles;
	} else {
		while (r->next)
			r = r->next;
		r->next = mem_zalloc(sizeof(struct room_profile));
		r = r->next;
	}

	/* Now read the data */
	r->name = string_make(parser_getsym(p, "name"));
	for (i = 0; i < N_ELEMENTS(room_builders); i++)
		if (streq(r->name, room_builders[i].name))
			break;

	if (i == N_ELEMENTS(room_builders))
		return PARSE_ERROR_NO_ROOM_FOUND;
	r->builder = room_builders[i].builder;
	r->rating = parser_getint(p, "rating");
	r->height = parser_getint(p, "height");
	r->width = parser_getint(p, "width");
	r->level = parser_getint(p, "level");
	r->pit = (parser_getint(p, "pit") == 1);
	r->rarity = parser_getint(p, "rarity");
	r->cutoff = parser_getint(p, "cutoff");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_min_level(struct parser *p) {
	struct cave_profile *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->min_level = parser_getint(p, "min");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_alloc(struct parser *p) {
	struct cave_profile *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->alloc = parser_getint(p, "alloc");
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_profile(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_profile_name);
	parser_reg(p, "params int block int rooms int unusual int rarity", parse_profile_params);
	parser_reg(p, "tunnel int rnd int chg int con int pen int jct", parse_profile_tunnel);
	parser_reg(p, "streamer int den int rng int mag int mc int qua int qc", parse_profile_streamer);
	parser_reg(p, "room sym name int rating int height int width int level int pit int rarity int cutoff", parse_profile_room);
	parser_reg(p, "min-level int min", parse_profile_min_level);
	parser_reg(p, "alloc int alloc", parse_profile_alloc);
	return p;
}

static errr run_parse_profile(struct parser *p) {
	return parse_file_quit_not_found(p, "dungeon_profile");
}

static errr finish_parse_profile(struct parser *p) {
	struct cave_profile *n, *c = parser_priv(p);
	int num;

	/* Count the list */
	z_info->profile_max = 0;
	while (c) {
		z_info->profile_max++;
		c = c->next;
	}

	/* Allocate the array and copy the records to it */
	cave_profiles = mem_zalloc(z_info->profile_max * sizeof(*c));
	num = z_info->profile_max - 1;
	for (c = parser_priv(p); c; c = n) {
		/* Main record */
		memcpy(&cave_profiles[num], c, sizeof(*c));
		n = c->next;
		if (num < z_info->profile_max - 1) {
			cave_profiles[num].next = &cave_profiles[num + 1];
		} else {
			cave_profiles[num].next = NULL;
		}

		if (c->room_profiles) {
			struct room_profile *r_old = c->room_profiles;
			struct room_profile *r_new;
			int i;

			/* Count the room profiles */
			cave_profiles[num].n_room_profiles = 0;
			while (r_old) {
				++cave_profiles[num].n_room_profiles;
				r_old = r_old->next;
			}

			/* Now allocate the room profile array */
			r_new = mem_zalloc(cave_profiles[num].n_room_profiles
				* sizeof(*r_new));

			r_old = c->room_profiles;
			for (i = 0; i < cave_profiles[num].n_room_profiles; i++) {
				struct room_profile *r_temp = r_old;

				/* Copy from the linked list to the array */
				memcpy(&r_new[i], r_old, sizeof(*r_old));

				/* Set the next profile pointer correctly. */
				if (r_new[i].next) {
					r_new[i].next = &r_new[i + 1];
				}

				/* Tidy up and advance to the next profile. */
				r_old = r_old->next;
				mem_free(r_temp);
			}

			cave_profiles[num].room_profiles = r_new;
		} else {
			cave_profiles[num].n_room_profiles = 0;
			cave_profiles[num].room_profiles = NULL;
		}

		mem_free(c);
		num--;
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_profile(void)
{
	int i, j;
	for (i = 0; i < z_info->profile_max; i++) {
		for (j = 0; j < cave_profiles[i].n_room_profiles; j++)
			string_free((char *) cave_profiles[i].room_profiles[j].name);
		mem_free(cave_profiles[i].room_profiles);
		string_free((char *) cave_profiles[i].name);
	}
	mem_free(cave_profiles);
}

static struct file_parser profile_parser = {
	"dungeon_profile",
	init_parse_profile,
	run_parse_profile,
	finish_parse_profile,
	cleanup_profile
};


/**
 * Parsing functions for room_template.txt
 */
static enum parser_error parse_room_name(struct parser *p) {
	struct room_template *h = parser_priv(p);
	struct room_template *t = mem_zalloc(sizeof *t);

	t->name = string_make(parser_getstr(p, "name"));
	t->next = h;
	parser_setpriv(p, t);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_type(struct parser *p) {
	struct room_template *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->typ = parser_getuint(p, "type");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_rating(struct parser *p) {
	struct room_template *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->rat = parser_getint(p, "rating");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_height(struct parser *p) {
	struct room_template *t = parser_priv(p);
	size_t i;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->hgt = parser_getuint(p, "height");

	/* Make sure rooms are no higher than the room profiles allow. */
	for (i = 0; i < N_ELEMENTS(room_builders); i++)
		if (streq("room template", room_builders[i].name))
			break;
	if (i == N_ELEMENTS(room_builders))
		return PARSE_ERROR_NO_ROOM_FOUND;
	if (t->hgt > room_builders[i].max_height)
		return PARSE_ERROR_VAULT_TOO_BIG;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_width(struct parser *p) {
	struct room_template *t = parser_priv(p);
	size_t i;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->wid = parser_getuint(p, "width");

	/* Make sure rooms are no wider than the room profiles allow. */
	for (i = 0; i < N_ELEMENTS(room_builders); i++)
		if (streq("room template", room_builders[i].name))
			break;
	if (i == N_ELEMENTS(room_builders))
		return PARSE_ERROR_NO_ROOM_FOUND;
	if (t->wid > room_builders[i].max_width)
		return PARSE_ERROR_VAULT_TOO_BIG;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_doors(struct parser *p) {
	struct room_template *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->dor = parser_getuint(p, "doors");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_tval(struct parser *p) {
	struct room_template *t = parser_priv(p);
	int tval;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	t->tval = tval;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_flags(struct parser *p) {
	struct room_template *t = parser_priv(p);
	char *s, *st;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	s = string_make(parser_getstr(p, "flags"));
	st = strtok(s, " |");
	while (st && !grab_flag(t->flags, ROOMF_SIZE, room_flags, st)) {
		st = strtok(NULL, " |");
	}
	mem_free(s);

	return st ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_room_d(struct parser *p) {
	struct room_template *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	t->text = string_append(t->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_room(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_room_name);
	parser_reg(p, "type uint type", parse_room_type);
	parser_reg(p, "rating int rating", parse_room_rating);
	parser_reg(p, "rows uint height", parse_room_height);
	parser_reg(p, "columns uint width", parse_room_width);
	parser_reg(p, "doors uint doors", parse_room_doors);
	parser_reg(p, "tval sym tval", parse_room_tval);
	parser_reg(p, "flags str flags", parse_room_flags);
	parser_reg(p, "D str text", parse_room_d);
	return p;
}

static errr run_parse_room(struct parser *p) {
	return parse_file_quit_not_found(p, "room_template");
}

static errr finish_parse_room(struct parser *p) {
	room_templates = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_room(void)
{
	struct room_template *t, *next;
	for (t = room_templates; t; t = next) {
		next = t->next;
		mem_free(t->name);
		mem_free(t->text);
		mem_free(t);
	}
}

static struct file_parser room_parser = {
	"room_template",
	init_parse_room,
	run_parse_room,
	finish_parse_room,
	cleanup_room
};


/**
 * Parsing functions for vault.txt
 */
static enum parser_error parse_vault_name(struct parser *p) {
	struct vault *h = parser_priv(p);
	struct vault *v = mem_zalloc(sizeof *v);

	v->name = string_make(parser_getstr(p, "name"));
	v->next = h;
	parser_setpriv(p, v);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_type(struct parser *p) {
	struct vault *v = parser_priv(p);

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->typ = string_make(parser_getstr(p, "type"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_rating(struct parser *p) {
	struct vault *v = parser_priv(p);

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->rat = parser_getint(p, "rating");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_rows(struct parser *p) {
	struct vault *v = parser_priv(p);
	size_t i;

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->hgt = parser_getuint(p, "height");

	/* Make sure vaults are no higher than the room profiles allow. */
	for (i = 0; i < N_ELEMENTS(room_builders); i++)
		if (streq(v->typ, room_builders[i].name))
			break;
	if (i == N_ELEMENTS(room_builders))
		return PARSE_ERROR_NO_ROOM_FOUND;
	if (v->hgt > room_builders[i].max_height)
		return PARSE_ERROR_VAULT_TOO_BIG;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_columns(struct parser *p) {
	struct vault *v = parser_priv(p);
	size_t i;

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->wid = parser_getuint(p, "width");

	/* Make sure vaults are no wider than the room profiles allow. */
	for (i = 0; i < N_ELEMENTS(room_builders); i++)
		if (streq(v->typ, room_builders[i].name))
			break;
	if (i == N_ELEMENTS(room_builders))
		return PARSE_ERROR_NO_ROOM_FOUND;
	if (v->wid > room_builders[i].max_width)
		return PARSE_ERROR_VAULT_TOO_BIG;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_min_depth(struct parser *p) {
	struct vault *v = parser_priv(p);

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	v->min_lev = parser_getuint(p, "min_lev");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_max_depth(struct parser *p) {
	struct vault *v = parser_priv(p);
	int max_lev;

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	max_lev = parser_getuint(p, "max_lev");
	v->max_lev = max_lev ? max_lev : z_info->max_depth;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_flags(struct parser *p) {
	struct vault *v = parser_priv(p);
	char *s, *st;

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	s = string_make(parser_getstr(p, "flags"));
	st = strtok(s, " |");
	while (st && !grab_flag(v->flags, ROOMF_SIZE, room_flags, st)) {
		st = strtok(NULL, " |");
	}
	mem_free(s);

	return st ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_vault_d(struct parser *p) {
	struct vault *v = parser_priv(p);
	const char *desc;

	if (!v)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	desc = parser_getstr(p, "text");
	if (strlen(desc) != v->wid)
		return PARSE_ERROR_VAULT_DESC_WRONG_LENGTH;
	else
		v->text = string_append(v->text, desc);
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_vault(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_vault_name);
	parser_reg(p, "type str type", parse_vault_type);
	parser_reg(p, "rating int rating", parse_vault_rating);
	parser_reg(p, "rows uint height", parse_vault_rows);
	parser_reg(p, "columns uint width", parse_vault_columns);
	parser_reg(p, "min-depth uint min_lev", parse_vault_min_depth);
	parser_reg(p, "max-depth uint max_lev", parse_vault_max_depth);
	parser_reg(p, "flags str flags", parse_vault_flags);
	parser_reg(p, "D str text", parse_vault_d);
	return p;
}

static errr run_parse_vault(struct parser *p) {
	return parse_file_quit_not_found(p, "vault");
}

static errr finish_parse_vault(struct parser *p) {
	vaults = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_vault(void)
{
	struct vault *v, *next;
	for (v = vaults; v; v = next) {
		next = v->next;
		mem_free(v->name);
		mem_free(v->typ);
		mem_free(v->text);
		mem_free(v);
	}
}

static struct file_parser vault_parser = {
	"vault",
	init_parse_vault,
	run_parse_vault,
	finish_parse_vault,
	cleanup_vault
};

static void run_template_parser(void) {
	/* Initialize room info */
	event_signal_message(EVENT_INITSTATUS, 0,
						 "Initializing arrays... (dungeon profiles)");
	if (run_parser(&profile_parser))
		quit("Cannot initialize dungeon profiles");

	/* Initialize room info */
	event_signal_message(EVENT_INITSTATUS, 0,
						 "Initializing arrays... (room templates)");
	if (run_parser(&room_parser))
		quit("Cannot initialize room templates");

	/* Initialize vault info */
	event_signal_message(EVENT_INITSTATUS, 0,
						 "Initializing arrays... (vaults)");
	if (run_parser(&vault_parser))
		quit("Cannot initialize vaults");
}


/**
 * Free the template arrays
 */
static void cleanup_template_parser(void)
{
	cleanup_parser(&profile_parser);
	cleanup_parser(&room_parser);
	cleanup_parser(&vault_parser);
}


/**
 * Place hidden squares that will be used to generate feeling
 * \param c is the cave struct the feeling squares are being placed in
 */
static void place_feeling(struct chunk *c)
{
	int i, j;
	int tries = 500;
	
	for (i = 0; i < z_info->feeling_total; i++) {
		for (j = 0; j < tries; j++) {
			/* Pick a random dungeon coordinate */
			struct loc grid = loc(randint0(c->width), randint0(c->height));

			/* Check to see if it is not passable */
			if (!square_ispassable(c, grid))
				continue;

			/* Check to see if it is already marked */
			if (square_isfeel(c, grid))
				continue;

			/* Set the cave square appropriately */
			sqinfo_on(square(c, grid)->info, SQUARE_FEEL);
			
			break;
		}
	}

	/* Reset number of feeling squares */
	c->feeling_squares = 0;
}


/**
 * Calculate the level feeling for objects.
 * \param c is the cave where the feeling is being measured
 */
static int calc_obj_feeling(struct chunk *c, struct player *p)
{
	uint32_t x;

	/* Town gets no feeling */
	if (c->depth == 0) return 0;

	/* Artifacts trigger a special feeling when they can be easily lost */
	if (c->good_item && OPT(p, birth_lose_arts)) return 10;

	/* Check the loot adjusted for depth */
	x = c->obj_rating / c->depth;

	/* Apply a minimum feeling if there's an artifact on the level */
	if (c->good_item && x < 641) return 60;

	if (x > 160000) return 20;
	if (x > 40000) return 30;
	if (x > 10000) return 40;
	if (x > 2500) return 50;
	if (x > 640) return 60;
	if (x > 160) return 70;
	if (x > 40) return 80;
	if (x > 10) return 90;
	return 100;
}

/**
 * Calculate the level feeling for monsters.
 * \param c is the cave where the feeling is being measured
 */
static int calc_mon_feeling(struct chunk *c)
{
	uint32_t x;

	/* Town gets no feeling */
	if (c->depth == 0) return 0;

	/* Check the monster power adjusted for depth */
	x = c->mon_rating / c->depth;

	if (x > 7000) return 1;
	if (x > 4500) return 2;
	if (x > 2500) return 3;
	if (x > 1500) return 4;
	if (x > 800) return 5;
	if (x > 400) return 6;
	if (x > 150) return 7;
	if (x > 50) return 8;
	return 9;
}

/**
 * Find a cave_profile by name
 * \param name is the name of the cave_profile being looked for
 */
static const struct cave_profile *find_cave_profile(const char *name)
{
	int i;

	for (i = 0; i < z_info->profile_max; i++) {
		const struct cave_profile *profile;

		profile = &cave_profiles[i];
		if (streq(name, profile->name))
			return profile;
	}

	/* Not there */
	return NULL;
}

/**
 * Do d_m's prime check for labyrinths
 * \param depth is the depth where we're trying to generate a labyrinth
 */
static bool labyrinth_check(int depth)
{
	/* There's a base 2 in 100 to accept the labyrinth */
	int chance = 2;

	/* If we're too shallow then don't do it */
	if (depth < 13) return false;

	/* Certain numbers increase the chance of having a labyrinth */
	if (depth % 3 == 0) chance += 1;
	if (depth % 5 == 0) chance += 1;
	if (depth % 7 == 0) chance += 1;
	if (depth % 11 == 0) chance += 1;
	if (depth % 13 == 0) chance += 1;

	/* Only generate the level if we pass a check */
	if (randint0(100) >= chance) return false;

	/* Successfully ran the gauntlet! */
	return true;
}

/**
 * Choose a cave profile
 * \param p is the player
 */
static const struct cave_profile *choose_profile(struct player *p)
{
	const struct cave_profile *profile = NULL;
	int moria_alloc = find_cave_profile("moria")->alloc;
	int labyrinth_alloc = find_cave_profile("labyrinth")->alloc;

	/* A bit of a hack, but worth it for now NRM */
	if (p->noscore & NOSCORE_JUMPING) {
		char name[30] = "";

		/* Cancel the query */
		p->noscore &= ~(NOSCORE_JUMPING);

		/* Ask debug players for the profile they want */
		if (get_string("Profile name (eg classic): ", name, sizeof(name)))
			profile = find_cave_profile(name);

		/* If no valid profile name given, fall through */
		if (profile) return profile;
	}

	/* Make the profile choice */
	if (p->depth == 0) {
		profile = find_cave_profile("town");
	} else if (is_quest(p, p->depth)) {
		/* Quest levels must be normal levels */
		profile = find_cave_profile("classic");
	} else if (labyrinth_check(p->depth) &&
			(labyrinth_alloc > 0 || labyrinth_alloc == -1)) {
		profile = find_cave_profile("labyrinth");
	} else if ((p->depth >= 10) && (p->depth < 40) && one_in_(40) &&
			(moria_alloc > 0 || moria_alloc == -1)) {
		profile = find_cave_profile("moria");
	} else {
		int total_alloc = 0;
		size_t i;

		/*
		 * Use PowerWyrm's selection algorithm from
		 * get_random_monster_object() so the selection can be done in
		 * one pass and without auxiliary storage (at the cost of more
		 * calls to randint0()).  The mth valid profile out of n valid
		 * profiles appears with probability, alloc(m) /
		 * sum(i = 0 to m, alloc(i)) * product(j = m + 1 to n - 1,
		 * 1 - alloc(j) / sum(k = 0 to j, alloc(k))) which is equal to
		 * alloc(m) / sum(i = 0 to m, alloc(i)) * product(j = m + 1 to
		 * n - 1, sum(k = 0 to j - 1, alloc(k)) / sum(l = 0 to j,
		 * alloc(l))) which, by the canceling of successive numerators
		 * and denominators is alloc(m) / sum(l = 0 to n - 1, alloc(l)).
		 */
		for (i = 0; i < z_info->profile_max; i++) {
			struct cave_profile *test_profile = &cave_profiles[i];
			if (test_profile->alloc <= 0 ||
				 p->depth < test_profile->min_level) continue;
			total_alloc += test_profile->alloc;
			if (randint0(total_alloc) < test_profile->alloc) {
				profile = test_profile;
			}
		}
		if (!profile) {
			profile = find_cave_profile("classic");
		}
	}

	/* Return the profile or fail horribly */
	if (profile)
		return profile;
	else
		quit("Failed to find cave profile!");

	return NULL;
}

/**
 * Get information for constructing stairs in the correct places
 */
static void get_join_info(struct player *p, struct dun_data *dd)
{
	struct level *lev = NULL;

	/* Check level above */
	lev = level_by_depth(p->depth - 1);
	if (lev) {
		struct chunk *check = chunk_find_name(lev->name);
		if (check) {
			struct connector *join = check->join;
			while (join) {
				if (join->feat == FEAT_MORE) {
					struct connector *new = mem_zalloc(sizeof *new);
					new->grid.y = join->grid.y;
					new->grid.x = join->grid.x;
					new->feat = FEAT_LESS;
					new->next = dd->join;
					dd->join = new;
				}
				join = join->next;
			}
		}
	} else if ((lev = level_by_depth(p->depth - 2))) {
		/*
		 * When there isn't a level above but there is one two levels
		 * up, remember where the down staircases are so up staircases
		 * on this level won't conflict with them if the level above is
		 * ever generated.
		 */
		struct chunk *check = chunk_find_name(lev->name);

		if (check) {
			struct connector *join;

			for (join = check->join; join; join = join->next) {
				if (join->feat == FEAT_MORE) {
					struct connector *nc =
						mem_zalloc(sizeof(*nc));

					nc->grid = join->grid;
					nc->feat = FEAT_MORE;
					nc->next = dd->one_off_above;
					dd->one_off_above = nc;
				}
			}
		}
	}

	/* Check level below */
	lev = level_by_depth(p->depth + 1);
	if (lev) {
		struct chunk *check = chunk_find_name(lev->name);
		if (check) {
			struct connector *join = check->join;
			while (join) {
				if (join->feat == FEAT_LESS) {
					struct connector *new = mem_zalloc(sizeof *new);
					new->grid.y = join->grid.y;
					new->grid.x = join->grid.x;
					new->feat = FEAT_MORE;
					new->next = dd->join;
					dd->join = new;
				}
				join = join->next;
			}
		}
	} else if ((lev = level_by_depth(p->depth + 2))) {
		/* Same logic as above for looking one past the next level */
		struct chunk *check = chunk_find_name(lev->name);

		if (check) {
			struct connector *join;

			for (join = check->join; join; join = join->next) {
				if (join->feat == FEAT_LESS) {
					struct connector *nc =
						mem_zalloc(sizeof(*nc));

					nc->grid = join->grid;
					nc->feat = FEAT_LESS;
					nc->next = dd->one_off_below;
					dd->one_off_below = nc;
				}
			}
		}
	}
}

/**
 * Check the size of the level above or below the next level to be generated
 * to make sure stairs can connect
 */
static void get_min_level_size(struct chunk *check, int *min_height,
							   int *min_width, bool above)
{
	struct connector *join = check->join;

	while (join) {
		if ((above && (join->feat == FEAT_MORE)) ||
			(!above && (join->feat == FEAT_LESS))) {
			*min_height = MAX(*min_height, join->grid.y + 2);
			*min_width = MAX(*min_width, join->grid.x + 2);
		}
		join = join->next;
	}
}


/**
 * Store a dungeon level for reloading
 */
static void cave_store(struct chunk *c, bool known, bool keep_all)
{
	struct chunk *stored;
	if (keep_all) {
		stored = c;
	} else {
		stored = chunk_write(c);
	}
	if (stored->name) {
		string_free(stored->name);
	}
	stored->name = string_make(level_by_depth(c->depth)->name);
	if (known) {
		stored->name = string_append(stored->name, " known");
	}
	stored->turn = turn;
	chunk_list_add(stored);
}


/**
 * Clear the dungeon, ready for generation to begin.
 */
static void cave_clear(struct chunk *c, struct player *p)
{
	/* Clear the monsters */
	wipe_mon_list(c, p);

	/* Free the chunk */
	cave_free(c);
}


/**
 * Release the dynamically allocated resources in a dun_data structure.
 */
static void cleanup_dun_data(struct dun_data *dd)
{
	int i;

	cave_connectors_free(dun->join);
	cave_connectors_free(dun->one_off_above);
	cave_connectors_free(dun->one_off_below);
	mem_free(dun->cent);
	mem_free(dun->ent_n);
	for (i = 0; i < z_info->level_room_max; ++i) {
		mem_free(dun->ent[i]);
	}
	mem_free(dun->ent);
	if (dun->ent2room) {
		for (i = 0; dun->ent2room[i]; ++i) {
			mem_free(dun->ent2room[i]);
		}
		mem_free(dun->ent2room);
	}
	mem_free(dun->door);
	mem_free(dun->wall);
	mem_free(dun->tunn);
}


/**
 * Generate a random level.
 *
 * Confusingly, this function also generates the town level (level 0).
 * \param p is the current player struct, in practice the global player
 * \return a pointer to the new level
 */
static struct chunk *cave_generate(struct player *p, int height, int width)
{
	const char *error = "no generation";
	int i, tries = 0;
	struct chunk *chunk = NULL;

	/* Arena levels handled separately */
	if (p->upkeep->arena_level) {
		/* Generate level */
		event_signal_string(EVENT_GEN_LEVEL_START, "arena");
		chunk = arena_gen(p, height, width);

		/* Allocate new known level, light it if requested */
		p->cave = cave_new(chunk->height, chunk->width);
		p->cave->depth = chunk->depth;
		p->cave->objects = mem_realloc(p->cave->objects, (chunk->obj_max + 1)
									   * sizeof(struct object*));
		p->cave->obj_max = chunk->obj_max;
		for (i = 0; i <= p->cave->obj_max; i++) {
			p->cave->objects[i] = NULL;
		}

		wiz_light(chunk, p, false);
		chunk->turn = turn;

		return chunk;
	}

	/* Generate */
	for (tries = 0; tries < 100 && error; tries++) {
		int y, x;
		struct dun_data dun_body;

		error = NULL;

		/* Mark the dungeon as being unready (to avoid artifact loss, etc) */
		character_dungeon = false;

		/* Allocate global data (will be freed when we leave the loop) */
		dun = &dun_body;
		dun->cent = mem_zalloc(z_info->level_room_max * sizeof(struct loc));
		dun->ent_n = mem_zalloc(z_info->level_room_max * sizeof(*dun->ent_n));
		dun->ent = mem_zalloc(z_info->level_room_max * sizeof(*dun->ent));
		dun->ent2room = NULL;
		dun->door = mem_zalloc(z_info->level_door_max * sizeof(struct loc));
		dun->wall = mem_zalloc(z_info->wall_pierce_max * sizeof(struct loc));
		dun->tunn = mem_zalloc(z_info->tunn_grid_max * sizeof(struct loc));
		dun->join = NULL;
		dun->one_off_above = NULL;
		dun->one_off_below = NULL;
		dun->curr_join = NULL;
		dun->nstair_room = 0;
		dun->quest = is_quest(p, p->depth);

		/* Get connector info for persistent levels */
		if (OPT(p, birth_levels_persist)) {
			dun->persist = true;
			get_join_info(p, dun);
		} else {
			dun->persist = false;
		}


		/* Choose a profile and build the level */
		dun->profile = choose_profile(p);
		event_signal_string(EVENT_GEN_LEVEL_START, dun->profile->name);
		chunk = dun->profile->builder(p, height, width);
		if (!chunk) {
			error = "Failed to find builder";
			cleanup_dun_data(dun);
			event_signal_flag(EVENT_GEN_LEVEL_END, false);
			continue;
		}

		/* Ensure quest monsters */
		if (dun->quest) {
			int i2;
			for (i2 = 1; i2 < z_info->r_max; i2++) {
				struct monster_race *race = &r_info[i2];
				struct monster_group_info info = { 0, 0 };
				struct loc grid;

				/* The monster must be an unseen quest monster of this depth. */
				if (race->cur_num > 0) continue;
				if (!rf_has(race->flags, RF_QUESTOR)) continue;
				if (race->level != chunk->depth) continue;
	
				/* Pick a location and place the monster */
				find_empty(chunk, &grid);
				place_new_monster(chunk, grid, race, true, true, info,
								  ORIGIN_DROP);
			}
		}

		/* Clear generation flags, add connecting info */
		for (y = 0; y < chunk->height; y++) {
			for (x = 0; x < chunk->width; x++) {
				struct loc grid = loc(x, y);

				sqinfo_off(square(chunk, grid)->info, SQUARE_WALL_INNER);
				sqinfo_off(square(chunk, grid)->info, SQUARE_WALL_OUTER);
				sqinfo_off(square(chunk, grid)->info, SQUARE_WALL_SOLID);
				sqinfo_off(square(chunk, grid)->info, SQUARE_MON_RESTRICT);

				if (square_isstairs(chunk, grid)) {
					size_t n;
					struct connector *new = mem_zalloc(sizeof *new);
					new->grid = grid;
					new->feat = square_feat(chunk, grid)->fidx;
					new->info = mem_zalloc(SQUARE_SIZE * sizeof(bitflag));
					for (n = 0; n < SQUARE_SIZE; n++) {
						new->info[n] = square(chunk, grid)->info[n];
					}
					new->next = chunk->join;
					chunk->join = new;
				}
			}
		}

		/* Regenerate levels that overflow their maxima */
		if (cave_monster_max(chunk) >= z_info->level_monster_max)
			error = "too many monsters";

		if (error) {
			if (OPT(p, cheat_room)) {
				msg("Generation restarted: %s.", error);
			}
			cave_clear(chunk, p);
			event_signal_flag(EVENT_GEN_LEVEL_END, false);
		}

		cleanup_dun_data(dun);
	}

	if (error) quit_fmt("cave_generate() failed 100 times!");

	/* Place dungeon squares to trigger feeling (not in town) */
	if (p->depth) {
		place_feeling(chunk);
	}

	chunk->feeling = calc_obj_feeling(chunk, p) + calc_mon_feeling(chunk);

	/* Validate the dungeon (we could use more checks here) */
	chunk_validate_objects(chunk);

	/* Allocate new known level, light it if requested */
	p->cave = cave_new(chunk->height, chunk->width);
	p->cave->depth = chunk->depth;
	p->cave->objects = mem_realloc(p->cave->objects, (chunk->obj_max + 1)
								   * sizeof(struct object*));
	p->cave->obj_max = chunk->obj_max;
	for (i = 0; i <= p->cave->obj_max; i++) {
		p->cave->objects[i] = NULL;
	}
	if (p->upkeep->light_level) {
		wiz_light(chunk, p, false);
		p->upkeep->light_level = false;
	}

	chunk->turn = turn;

	return chunk;
}

static void sanitize_player_loc(struct chunk *c, struct player *p)
{
	/* TODO potential problem: stairs in vaults? */

	/* allow direct transfer if target location is teleportable */
	if (square_in_bounds_fully(c, p->grid)
		&& square_isarrivable(c, p->grid)
		&& !square_isvault(c, p->grid)) {
		return;
	}

	/* TODO should use something similar to teleport code, but this will
	 *  do for now as a quick'n dirty fix
	 */
	int tx, ty; // test locations
	int ix, iy; // initial location
	int vx = 1, vy = 1; // fallback vault location
	int try = 1000; // attempts

	/* a bunch of random locations */
	while (try) {
		try = try - 1;
		tx = randint0(c->width - 1) + 1;
		ty = randint0(c->height - 1) + 1;
		if (square_isempty(c, loc(tx, ty))
			&& !square_isvault(c, loc(tx, ty))) {
			p->grid.y = ty;
			p->grid.x = tx;
			return;
		}
	}

	/* whelp, that didnt work */
	ix = randint0(c->width - 1) + 1;
	iy = randint0(c->height - 1) + 1;
	ty = iy;
	tx = ix + 1;
	if (tx >= c->width - 1) {
		tx = 1;
		ty = ty + 1;
		if (ty >= c->height -1) {
			ty = 1;
		}
	}

	while (1) {		//until full loop through dungeon
		if (square_isempty(c, loc(tx, ty))) {
			if (!square_isvault(c, loc(tx, ty))) {
				// ok location
				p->grid.y = ty;
				p->grid.x = tx;
				return;
			}
			// vault, but lets remember it just in case
			vy = ty;
			vx = tx;
		}
		// oops tried *every* tile...
		if (tx == ix && ty == iy) {
			break;
		}
		tx = tx + 1;
		if (tx >= c->width - 1) {
			tx = 1;
			ty = ty + 1;
			if (ty >= c->height -1) {
				ty = 1;
			}
		}
	}

	// fallback vault location (or at least a non-crashy square)
	p->grid.x = vx;
	p->grid.y = vy;
}

/**
 * Prepare the level the player is about to enter, either by generating
 * or reloading
 *
 * \param c is the level we're going to end up with, in practice the global cave
 * \param p is the current player struct, in practice the global player
*/
void prepare_next_level(struct player *p)
{
	bool persist = OPT(p, birth_levels_persist) || p->upkeep->arena_level;

	/* Deal with any existing current level */
	if (character_dungeon) {
		assert (p->cave);

		if (persist) {
			/* Arenas don't get stored */
			if (!cave->name || !streq(cave->name, "arena")) {
				/* Tidy up */
				compact_monsters(cave, 0);
				if (!p->upkeep->arena_level) {
					/* Leave the player marker if going to an arena */
					square_set_mon(cave, p->grid, 0);
				}

				/* Save level and known level */
				cave_store(cave, false, true);
				cave_store(p->cave, true, true);
			}
		} else {
			/* Save the town */
			if (!cave->depth && !chunk_find_name("Town")) {
				cave_store(cave, false, false);
			}

			/* Forget knowledge of old level */
			if (p->cave) {
				int x, y;

				/* Deal with artifacts */
				for (y = 0; y < cave->height; y++) {
					for (x = 0; x < cave->width; x++) {
						struct object *obj = square_object(cave, loc(x, y));
						while (obj) {
							if (obj->artifact) {
								bool found = obj_is_known_artifact(obj);
								if (OPT(p, birth_lose_arts) || found) {
									history_lose_artifact(p, obj->artifact);
									mark_artifact_created(obj->artifact, true);
								} else {
									mark_artifact_created(obj->artifact, false);
								}
							}

							obj = obj->next;
						}
					}
				}

				/* Free the known cave */
				cave_free(p->cave);
				p->cave = NULL;
			}

			/* Clear the old cave */
			if (cave) {
				cave_clear(cave, p);
				cave = NULL;
			}
		}
	}

	/* Prepare the new level */
	if (persist) {
		char *name = level_by_depth(p->depth)->name;
		struct chunk *old_level = chunk_find_name(name);

		/* If we found an old level, load the known level and assign */
		if (old_level && (old_level != cave)) {
			int i;
			bool arena = cave->name && streq(cave->name, "arena");
			char *known_name = string_make(format("%s known", name));
			struct chunk *old_known = chunk_find_name(known_name);
			assert(old_known);

			/* Assign the new ones */
			cave = old_level;
			p->cave = old_known;

			/* Associate known objects */
			for (i = 0; i < p->cave->obj_max; i++) {
				if (cave->objects[i] && p->cave->objects[i]) {
					cave->objects[i]->known = p->cave->objects[i];
				}
			}

			/* Allow monsters to recover */
			restore_monsters();

			/* Leaving arenas requires special treatment */
			if (arena) {
				int y, x;
				bool found = false;

				/* Use the stored player grid */
				if (!loc_eq(p->old_grid, loc(0, 0))) {
					p->grid = p->old_grid;
					p->old_grid = loc(0, 0);
					found = true;
				}

				/* Look for the old player mark, place them by hand */
				if (!found) {
					for (y = 0; y < cave->height; y++) {
						for (x = 0; x < cave->width; x++) {
							struct loc grid = loc(x, y);
							if (square(cave, grid)->mon == -1) {
								p->grid = grid;
								found = true;
								break;
							}
						}
						if (found) break;
					}
				}

				/* Failed to find, try near the killed monster */
				if (!found) {
					int k;
					int ty = cave->monsters[1].grid.y;
					int tx = cave->monsters[1].grid.x;
					for (k = 1; k < 10; k++) {
						for (y = ty - k; y <= ty + k; y++) {
							for (x = tx - k; x <= tx + k; x++) {
								struct loc grid = loc(x, y);
								if (square_in_bounds_fully(cave, grid) &&
									square_isempty(cave, grid) &&
									!square_isvault(cave, grid)) {
									p->grid = grid;
									found = true;
									break;
								}
							}
							if (found) break;
						}
						if (found) break;
					}
				}

				/* Still failed to find, try anywhere */
				if (!found) {
					p->grid = cave->monsters[1].grid;
					sanitize_player_loc(cave, p);
				}

				square_set_mon(cave, p->grid, -1);;
			} else {
				/* Map boundary changes may not cooperate with level teleport */
				sanitize_player_loc(cave, p);

				/* Place the player */
				player_place(cave, p, p->grid);
			}

			/* Remove from the list */
			chunk_list_remove(name);
			chunk_list_remove(known_name);
			string_free(known_name);
		} else if (p->upkeep->arena_level) {
			/* We're creating a new arena level */
			cave = cave_generate(p, 6, 6);
			event_signal_flag(EVENT_GEN_LEVEL_END, true);
		} else {
			/* Check dimensions */
			struct level *lev;
			int min_height = 0, min_width = 0;

			/* Check level above */
			lev = level_by_depth(p->depth - 1);
			if (lev) {
				struct chunk *check = chunk_find_name(lev->name);
				if (check) {
					get_min_level_size(check, &min_height, &min_width, true);
				}
			}

			/* Check level below */
			lev = level_by_depth(p->depth + 1);
			if (lev) {
				struct chunk *check = chunk_find_name(lev->name);
				if (check) {
					get_min_level_size(check, &min_height, &min_width, false);
				}
			}

			/* Generate a new level */
			cave = cave_generate(p, min_height, min_width);
			event_signal_flag(EVENT_GEN_LEVEL_END, true);
		}
	} else {
		/* Generate a new level */
		cave = cave_generate(p, 0, 0);
		event_signal_flag(EVENT_GEN_LEVEL_END, true);
	}

	/* Know the town */
	if (!(p->depth)) {
		cave_known(p);
		if (persist) {
			cave_illuminate(cave, is_daytime());
		}

	}

	/* The dungeon is ready */
	character_dungeon = true;
}

/**
 * Return the number of room builders available.
 */
int get_room_builder_count(void)
{
	return (int) N_ELEMENTS(room_builders);
}

/**
 * Convert the name of a room builder into its index.  Return -1 if the
 * name does not match any of the room builders.
 */
int get_room_builder_index_from_name(const char *name)
{
	int i = 0;

	while (1) {
		if (i >= (int) N_ELEMENTS(room_builders)) {
			return -1;
		}
		if (streq(name, room_builders[i].name)) {
			return i;
		}
		++i;
	}
}

/**
 * Get the name of a room builder given its index.  Return NULL if the index
 * is out of bounds (less than one or greater than or equal to
 * get_room_builder_count()).
 */
const char *get_room_builder_name_from_index(int i)
{
	return (i >= 0 && i < (int) get_room_builder_count()) ?
		room_builders[i].name : NULL;
}

/**
 * Convert the name of a level profile into its index in the cave_profiles
 * list.  Return -1 if the name does not match any of the profiles.
 */
int get_level_profile_index_from_name(const char *name)
{
	const struct cave_profile *p = find_cave_profile(name);

	return (p) ? (int) (p - cave_profiles) : -1;
}

/**
 * Get the name of a level profile given its index.  Return NULL if the index
 * is out of bounds (less than one or greater than or equal to
 * z_info->profile_max).
 */
const char *get_level_profile_name_from_index(int i)
{
	return (i >= 0 && i < z_info->profile_max) ?
		cave_profiles[i].name : NULL;
}

/**
 * The generate module, which initialises template rooms and vaults
 * Should it clean up?
 */
struct init_module generate_module = {
	.name = "generate",
	.init = run_template_parser,
	.cleanup = cleanup_template_parser
};
