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
#include "game-event.h"
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "math.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "monster.h"
#include "obj-identify.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "parser.h"
#include "player-history.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/*
 * Array of pit types
 */
struct pit_profile *pit_info;
struct vault *vaults;
struct cave_profile *cave_profiles;


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
    r->height = parser_getint(p, "height");
    r->width = parser_getint(p, "width");
    r->level = parser_getint(p, "level");
    r->pit = (parser_getint(p, "pit") == 1);
    r->rarity = parser_getint(p, "rarity");
    r->cutoff = parser_getint(p, "cutoff");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_profile_cutoff(struct parser *p) {
    struct cave_profile *c = parser_priv(p);

	if (!c)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	c->cutoff = parser_getint(p, "cutoff");
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_profile(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_profile_name);
	parser_reg(p, "params int block int rooms int unusual int rarity", parse_profile_params);
	parser_reg(p, "tunnel int rnd int chg int con int pen int jct", parse_profile_tunnel);
	parser_reg(p, "streamer int den int rng int mag int mc int qua int qc", parse_profile_streamer);
	parser_reg(p, "room sym name int height int width int level int pit int rarity int cutoff", parse_profile_room);
	parser_reg(p, "cutoff int cutoff", parse_profile_cutoff);
	return p;
}

static errr run_parse_profile(struct parser *p) {
	return parse_file(p, "dungeon_profile");
}

static errr finish_parse_profile(struct parser *p) {
	struct cave_profile *n, *c = parser_priv(p);
	int i, num;

	z_info->profile_max = 0;
	/* Count the list */
	while (c) {
		struct room_profile *r = c->room_profiles;
		c->n_room_profiles = 0;

		z_info->profile_max++;
		c = c->next;
		while (r) {
			c->n_room_profiles++;
			r = r->next;
		}
	}

	/* Allocate the array and copy the records to it */
	cave_profiles = mem_zalloc(z_info->profile_max * sizeof(*c));
	num = z_info->profile_max - 1;
	for (c = parser_priv(p); c; c = n) {
		struct room_profile *r_new = NULL;

		/* Main record */
		memcpy(&cave_profiles[num], c, sizeof(*c));
		n = c->next;
		if (num < z_info->profile_max - 1)
			cave_profiles[num].next = &cave_profiles[num + 1];
		else
			cave_profiles[num].next = NULL;

		/* Count the room profiles */
		if (c->room_profiles) {
			struct room_profile *r = c->room_profiles;
			c->n_room_profiles = 0;

			while (r) {
				c->n_room_profiles++;
				r = r->next;
			}
		}

		/* Now allocate the room profile array */
		if (c->room_profiles) {
			struct room_profile *r_temp, *r_old = c->room_profiles;

			/* Allocate space and copy */
			r_new = mem_zalloc(c->n_room_profiles * sizeof(*r_new));
			for (i = 0; i < c->n_room_profiles; i++) {
				memcpy(&r_new[i], r_old, sizeof(*r_old));
				r_old = r_old->next;
				if (!r_old) break;
			}

			/* Make next point correctly */
			for (i = 0; i < c->n_room_profiles; i++)
				if (r_new[i].next)
					r_new[i].next = &r_new[i + 1];

			/* Tidy up */
			r_old = c->room_profiles;
			r_temp = r_old;
			while (r_temp) {
				r_temp = r_old->next;
				mem_free(r_old);
				r_old = r_temp;
			}
		}
		cave_profiles[num].room_profiles = r_new;
		cave_profiles[num].n_room_profiles = c->n_room_profiles;

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
	parser_reg(p, "D str text", parse_room_d);
	return p;
}

static errr run_parse_room(struct parser *p) {
	return parse_file(p, "room_template");
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
	parser_reg(p, "D str text", parse_vault_d);
	return p;
}

static errr run_parse_vault(struct parser *p) {
	return parse_file(p, "vault");
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
	int y,x,i,j;
	int tries = 500;
	
	for (i = 0; i < z_info->feeling_total; i++) {
		for (j = 0; j < tries; j++) {
			/* Pick a random dungeon coordinate */
			y = randint0(c->height);
			x = randint0(c->width);

			/* Check to see if it is not a wall */
			if (square_iswall(c, y, x))
				continue;

			/* Check to see if it is already marked */
			if (square_isfeel(c, y, x))
				continue;

			/* Set the cave square appropriately */
			sqinfo_on(c->squares[y][x].info, SQUARE_FEEL);
			
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
static int calc_obj_feeling(struct chunk *c)
{
	u32b x;

	/* Town gets no feeling */
	if (c->depth == 0) return 0;

	/* Artifacts trigger a special feeling when preserve=no */
	if (c->good_item && OPT(birth_no_preserve)) return 10;

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
	u32b x;

	/* Town gets no feeling */
	if (c->depth == 0) return 0;

	/* Check the monster power adjusted for depth */
	x = c->mon_rating / (c->depth * c->depth);

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
 * Do d_m's prime check for labyrinths
 * \param depth is the depth where we're trying to generate a labyrinth
 */
bool labyrinth_check(int depth)
{
	/* There's a base 2 in 100 to accept the labyrinth */
	int chance = 2;

	/* If we're too shallow then don't do it */
	if (depth < 13) return false;

	/* Don't try this on quest levels, kids... */
	if (is_quest(depth)) return false;

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
 * Find a cave_profile by name
 * \param name is the name of the cave_profile being looked for
 */
const struct cave_profile *find_cave_profile(char *name)
{
	int i;

	for (i = 0; i < z_info->profile_max; i++) {
		const struct cave_profile *profile;

		profile = &cave_profiles[i];
		if (!strcmp(name, profile->name))
			return profile;
	}

	/* Not there */
	return NULL;
}

/**
 * Choose a cave profile
 * \param depth is the depth of the cave the profile will be used to generate
 */
const struct cave_profile *choose_profile(int depth)
{
	const struct cave_profile *profile = NULL;

	/* A bit of a hack, but worth it for now NRM */
	if (player->noscore & NOSCORE_JUMPING) {
		char name[30];

		/* Cancel the query */
		player->noscore &= ~(NOSCORE_JUMPING);

		/* Ask debug players for the profile they want */
		if (get_string("Profile name (eg classic): ", name, sizeof(name)))
			profile = find_cave_profile(name);

		/* If no valid profile name given, fall through */
		if (profile) return profile;
	}

	/* Make the profile choice */
	if (depth == 0)
		profile = find_cave_profile("town");
	else if (is_quest(depth))
		/* Quest levels must be normal levels */
		profile = find_cave_profile("classic");
	else if (labyrinth_check(depth))
		profile = find_cave_profile("labyrinth");
	else {
		int perc = randint0(100);
		size_t i;
		for (i = 0; i < z_info->profile_max; i++) {
			profile = &cave_profiles[i];
			if (profile->cutoff >= perc) break;
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
 * Clear the dungeon, ready for generation to begin.
 */
static void cave_clear(struct chunk *c, struct player *p)
{
	int x, y;

	/* Clear the monsters */
	wipe_mon_list(c, p);

	/* Deal with artifacts */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			struct object *obj = square_object(c, y, x);
			while (obj) {
				if (obj->artifact) {
					bool found = obj->known && obj->known->artifact;
					if (!OPT(birth_no_preserve) && !found)
						obj->artifact->created = false;
					else
						history_lose_artifact(obj->artifact);
				}
				obj = obj->next;
			}
		}
	}
	/* Free the chunk */
	cave_free(c);
}


/**
 * Generate a random level.
 *
 * Confusingly, this function also generate the town level (level 0).
 * \param c is the level we're going to end up with, in practice the global cave
 * \param p is the current player struct, in practice the global player
 */
void cave_generate(struct chunk **c, struct player *p)
{
	const char *error = "no generation";
	int i, y, x, tries = 0;
	struct chunk *chunk;

	assert(c);

	/* Generate */
	for (tries = 0; tries < 100 && error; tries++) {
		struct dun_data dun_body;

		error = NULL;

		/* Mark the dungeon as being unready (to avoid artifact loss, etc) */
		character_dungeon = false;

		/* Allocate global data (will be freed when we leave the loop) */
		dun = &dun_body;
		dun->cent = mem_zalloc(z_info->level_room_max * sizeof(struct loc));
		dun->door = mem_zalloc(z_info->level_door_max * sizeof(struct loc));
		dun->wall = mem_zalloc(z_info->wall_pierce_max * sizeof(struct loc));
		dun->tunn = mem_zalloc(z_info->tunn_grid_max * sizeof(struct loc));

		/* Choose a profile and build the level */
		dun->profile = choose_profile(p->depth);
		chunk = dun->profile->builder(p);
		if (!chunk) {
			error = "Failed to find builder";
			mem_free(dun->cent);
			mem_free(dun->door);
			mem_free(dun->wall);
			mem_free(dun->tunn);
			continue;
		}

		/* Ensure quest monsters */
		if (is_quest(chunk->depth)) {
			int i;
			for (i = 1; i < z_info->r_max; i++) {
				struct monster_race *race = &r_info[i];
				int y, x;
				
				/* The monster must be an unseen quest monster of this depth. */
				if (race->cur_num > 0) continue;
				if (!rf_has(race->flags, RF_QUESTOR)) continue;
				if (race->level != chunk->depth) continue;
	
				/* Pick a location and place the monster */
				find_empty(chunk, &y, &x);
				place_new_monster(chunk, y, x, race, true, true, ORIGIN_DROP);
			}
		}

		/* Clear generation flags. */
		for (y = 0; y < chunk->height; y++) {
			for (x = 0; x < chunk->width; x++) {
				sqinfo_off(chunk->squares[y][x].info, SQUARE_WALL_INNER);
				sqinfo_off(chunk->squares[y][x].info, SQUARE_WALL_OUTER);
				sqinfo_off(chunk->squares[y][x].info, SQUARE_WALL_SOLID);
				sqinfo_off(chunk->squares[y][x].info, SQUARE_MON_RESTRICT);
			}
		}

		/* Regenerate levels that overflow their maxima */
		if (cave_monster_max(chunk) >= z_info->level_monster_max)
			error = "too many monsters";

		if (error) {
			ROOM_LOG("Generation restarted: %s.", error);
			cave_clear(chunk, p);
		}

		mem_free(dun->cent);
		mem_free(dun->door);
		mem_free(dun->wall);
		mem_free(dun->tunn);
	}

	if (error) quit_fmt("cave_generate() failed 100 times!");

	/* Free old known level */
	if (cave_k && (*c == cave)) {
		cave_free(cave_k);
		cave_k = NULL;
	}

	/* Free the old cave, use the new one */
	if (*c)
		cave_clear(*c, p);
	*c = chunk;

	/* Place dungeon squares to trigger feeling (not in town) */
	if (player->depth)
		place_feeling(*c);

	/* Save the town */
	else if (!chunk_find_name("Town")) {
		struct chunk *town = chunk_write(0, 0, z_info->town_hgt,
										 z_info->town_wid, false, false, false);
		town->name = string_make("Town");
		chunk_list_add(town);
	}

	(*c)->feeling = calc_obj_feeling(*c) + calc_mon_feeling(*c);

	/* Validate the dungeon (we could use more checks here) */
	chunk_validate_objects(*c);

	/* The dungeon is ready */
	character_dungeon = true;

	/* Allocate new known level */
	if (*c == cave) {
		cave_k = cave_new((*c)->height, (*c)->width);
		cave_k->objects = mem_realloc(cave_k->objects, ((*c)->obj_max + 1)
									  * sizeof(struct object*));
		cave_k->obj_max = (*c)->obj_max;
		for (i = 0; i <= cave_k->obj_max; i++)
			cave_k->objects[i] = NULL;
		if (!((*c)->depth))
			cave_known();
	}

	(*c)->created_at = turn;
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
