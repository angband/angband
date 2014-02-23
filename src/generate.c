/*
 * File: generate.c
 * Purpose: Dungeon generation.
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
 */

#include "angband.h"
#include "cave.h"
#include "dungeon.h"
#include "math.h"
#include "files.h"
#include "game-event.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "monster.h"
#include "obj-util.h"
#include "object.h"
#include "parser.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/*
 * Array of pit types
 */
struct pit_profile *pit_info;
struct vault *vaults;


/**
 * Note that Level generation is *not* an important bottleneck, though it can
 * be annoyingly slow on older machines...  Thus we emphasize "simplicity" and
 * "correctness" over "speed".
 *
 * See the "vault.txt" file for more on vault generation.
 * See the "room_template.txt" file for more room templates.
 *
 * In this file, we use the "special" granite and perma-wall sub-types, where
 * "basic" is normal, "inner" is inside a room, "outer" is the outer wall of a
 * room, and "solid" is the outer wall of the dungeon or any walls that may not
 * be pierced by corridors.  Thus the only wall type that may be pierced by a
 * corridor is the "outer granite" type. The "basic granite" type yields the
 * "actual" corridors.
 *
 * We use the special "solid" granite wall type to prevent multiple corridors
 * from piercing a wall in two adjacent locations, which would be messy, and we
 * use the special "outer" granite wall to indicate which walls "surround"
 * rooms, and may thus be "pierced" by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge" of the
 * dungeon in a direction toward that edge will cause "silly" wall piercings,
 * but will have no permanently incorrect effects, as long as the tunnel can
 * eventually exit from another side. And note that the wall may not come back
 * into the room by the hole it left through, so it must bend to the left or
 * right and then optionally re-enter the room (at least 2 grids away). This is
 * not a problem since every room that is large enough to block the passage of
 * tunnels is also large enough to allow the tunnel to pierce the room itself
 * several times.
 *
 * Note that no two corridors may enter a room through adjacent grids, they
 * must either share an entryway or else use entryways at least two grids
 * apart. This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up into "blocks"
 * of 11x11 grids each, and require that all rooms occupy a rectangular group
 * of blocks.  As long as each room type reserves a sufficient number of
 * blocks, the room building routines will not need to check bounds. Note that
 * most of the normal rooms actually only use 23x11 grids, and so reserve 33x11
 * grids.
 *
 * Note that the use of 11x11 blocks (instead of the 33x11 panels) allows more
 * variability in the horizontal placement of rooms, and at the same time has
 * the disadvantage that some rooms (two thirds of the normal rooms) may be
 * "split" by panel boundaries.  This can induce a situation where a player is
 * in a room and part of the room is off the screen.  This can be so annoying
 * that the player must set a special option to enable "non-aligned" room
 * generation.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls.
 */

/**
 * Profile used for generating the town level.
 */
struct cave_profile town_profile = {
    /* name builder dun_rooms dun_unusual max_rarity n_room_profiles */
    "town-default", town_gen, 50, 200, 2, 0,

    /* name rnd chg con pen jct */
    {"tunnel-default", 10, 30, 15, 25, 90},

    /* name den rng mag mc qua qc */
    {"streamer-default", 5, 2, 3, 90, 2, 40},

    /* room_profiles -- not applicable */
    NULL,

    /* cutoff -- not applicable */
    0
};


/* name function height width min-depth pit? rarity %cutoff */
struct room_profile classic_rooms[NUM_CLASSIC_ROOMS] = {
    /* greater vaults only have rarity 1 but they have other checks */
    {"greater vault", build_greater_vault, {4, 0, 0, 0}, {6, 0, 0, 0}, 35, 
	FALSE, 0, 100},

    /* very rare rooms (rarity=2) */
    {"monster pit", build_pit, {1, 0, 0, 0}, {3, 0, 0, 0}, 5, TRUE, 2, 8},
    {"monster nest", build_nest, {1, 0, 0, 0}, {3, 0, 0, 0}, 5, TRUE, 2, 16},
    {"medium vault", build_medium_vault, {2, 0, 0, 0}, {3, 0, 0, 0}, 30, 
	 FALSE, 2, 38},
    {"lesser vault", build_lesser_vault, {2, 0, 0, 0}, {3, 0, 0, 0}, 20, 
	 FALSE, 2, 55},
	

    /* unusual rooms (rarity=1) */
    {"large room", build_large, {1, 0, 0, 0}, {3, 0, 0, 0}, 3, FALSE, 1, 15},
    {"crossed room", build_crossed, {1, 0, 0, 0}, {3, 0, 0, 0}, 3, 
	 FALSE, 1, 35},
    {"circular room", build_circular, {2, 0, 0, 0}, {2, 0, 0, 0}, 1, 
	 FALSE, 1, 50},
    {"overlap room", build_overlap, {1, 0, 0, 0}, {3, 0, 0, 0}, 1, 
	 FALSE, 1, 70},
    {"room template", build_template, {1, 0, 0, 0}, {3, 0, 0, 0}, 5, 
	 FALSE, 1, 100},

    /* normal rooms */
    {"simple room", build_simple, {1, 0, 0, 0}, {3, 0, 0, 0}, 1, FALSE, 0, 100}
};

/* name function height width min-depth pit? rarity %cutoff */
struct room_profile unused_rooms[] = {
	{"room of chambers", build_room_of_chambers, {2, 1, 3, 0}, {2, 1, 5, 0}, 10,
	 FALSE, 0, 100},
	{"huge room", build_huge, {2, 1, 2, 0}, {3, 1, 6, 0}, 40, FALSE, 0, 100}
};

/**
 * Profiles used for generating dungeon levels.
 */
struct cave_profile cave_profiles[NUM_CAVE_PROFILES] = {
    {
		"labyrinth", labyrinth_gen, 0, 200, 0, 0,

		/* tunnels -- not applicable */
		{"tunnel-null", 0, 0, 0, 0, 0},

		/* streamers -- not applicable */
		{"streamer-null", 0, 0, 0, 0, 0, 0},

		/* room_profiles -- not applicable */
		NULL,

		/* cutoff -- unused because of internal checks in labyrinth_gen  */
		100
    },
    {
		"cavern", cavern_gen, 0, 200, 0, 0,

		/* tunnels -- not applicable */
		{"tunnel-null", 0, 0, 0, 0, 0},

		/* streamers -- not applicable */
		{"streamer-null", 0, 0, 0, 0, 0, 0},

		/* room_profiles -- not applicable */
		NULL,

		/* cutoff -- debug  */
		10
    },
    {
		/* name builder dun_rooms dun_unusual max_rarity n_room_profiles */
		"classic", classic_gen, 50, 200, 2, N_ELEMENTS(classic_rooms),

		/* name rnd chg con pen jct */
		{"tunnel-classic", 10, 30, 15, 25, 90},

		/* name den rng mag mc qua qc */
		{"streamer-classic", 5, 2, 3, 90, 2, 40},

		/* room_profiles */
		classic_rooms,

		/* cutoff */
		100
    }
};


/* Parsing functions for room_template.txt */
static enum parser_error parse_room_n(struct parser *p) {
    struct room_template *h = parser_priv(p);
    struct room_template *t = mem_zalloc(sizeof *t);

    t->tidx = parser_getuint(p, "index");
    t->name = string_make(parser_getstr(p, "name"));
    t->next = h;
    parser_setpriv(p, t);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_room_x(struct parser *p) {
    struct room_template *t = parser_priv(p);

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->typ = parser_getuint(p, "type");
    t->rat = parser_getint(p, "rating");
    t->hgt = parser_getuint(p, "height");
    t->wid = parser_getuint(p, "width");
    t->dor = parser_getuint(p, "doors");
    t->tval = parser_getuint(p, "tval");

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
    parser_reg(p, "V sym version", ignored);
    parser_reg(p, "N uint index str name", parse_room_n);
    parser_reg(p, "X uint type int rating uint height uint width uint doors uint tval", parse_room_x);
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

static void run_room_parser(void) {
    event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (room templates)");
    if (run_parser(&room_parser))
		quit("Cannot initialize room templates");
}




/**
 * Clear the dungeon, ready for generation to begin.
 */
static void cave_clear(struct cave *c, struct player *p) {
    int x, y;

    wipe_o_list(c);
    wipe_mon_list(c, p);
	wipe_trap_list(c);


    /* Clear flags and flow information. */
    for (y = 0; y < DUNGEON_HGT; y++) {
		for (x = 0; x < DUNGEON_WID; x++) {
			/* Erase features */
			c->feat[y][x] = 0;

			/* Erase flags */
			sqinfo_wipe(c->info[y][x]);

			/* Erase flow */
			c->cost[y][x] = 0;
			c->when[y][x] = 0;

			/* Erase monsters/player */
			c->m_idx[y][x] = 0;

			/* Erase items */
			c->o_idx[y][x] = 0;
		}
    }

    /* Unset the player's coordinates */
    p->px = p->py = 0;

    /* Nothing special here yet */
    c->good_item = FALSE;

    /* Nothing good here yet */
    c->mon_rating = 0;
    c->obj_rating = 0;
}

/**
 * Place hidden squares that will be used to generate feeling
 */
static void place_feeling(struct cave *c)
{
    int y,x,i,j;
    int tries = 500;
	
    for (i = 0; i < FEELING_TOTAL; i++) {
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
			sqinfo_on(c->info[y][x], SQUARE_FEEL);
			
			break;
		}
    }

    /* Reset number of feeling squares */
    c->feeling_squares = 0;
}


/**
 * Calculate the level feeling for objects.
 */
static int calc_obj_feeling(struct cave *c)
{
    u32b x;

    /* Town gets no feeling */
    if (c->depth == 0) return 0;

    /* Artifacts trigger a special feeling when preserve=no */
    if (c->good_item && OPT(birth_no_preserve)) return 10;

    /* Check the loot adjusted for depth */
    x = c->obj_rating / c->depth;

    /* Apply a minimum feeling if there's an artifact on the level */
    if (c->good_item && x < 64001) return 60;

    if (x > 16000000) return 20;
    if (x > 4000000) return 30;
    if (x > 1000000) return 40;
    if (x > 250000) return 50;
    if (x > 64000) return 60;
    if (x > 16000) return 70;
    if (x > 4000) return 80;
    if (x > 1000) return 90;
    return 100;
}

/**
 * Calculate the level feeling for monsters.
 */
static int calc_mon_feeling(struct cave *c)
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
 * Generate a random level.
 *
 * Confusingly, this function also generate the town level (level 0).
 */
void cave_generate(struct cave *c, struct player *p) {
    const char *error = "no generation";
    int y, x, tries = 0;

    assert(c);

    c->depth = p->depth;

    /* Generate */
    for (tries = 0; tries < 100 && error; tries++) {
		struct dun_data dun_body;

		error = NULL;
		cave_clear(c, p);

		/* Mark the dungeon as being unready (to avoid artifact loss, etc) */
		character_dungeon = FALSE;

		/* Allocate global data (will be freed when we leave the loop) */
		dun = &dun_body;

		if (p->depth == 0) {
			dun->profile = &town_profile;
			dun->profile->builder(c, p);
		} else if (is_quest(c->depth)) {
		
			/* Quest levels must be normal levels */
			dun->profile = &cave_profiles[NUM_CAVE_PROFILES - 1];
			dun->profile->builder(c, p);
			
		} else {	
			int perc = randint0(100);
			int last = NUM_CAVE_PROFILES - 1;
			int i;
			for (i = 0; i < NUM_CAVE_PROFILES; i++) {
				bool ok;
				const struct cave_profile *profile;

				profile = dun->profile = &cave_profiles[i];
				if (i < last && profile->cutoff < perc) continue;

				ok = dun->profile->builder(c, p);
				if (ok) break;
			}
		}

		/* Ensure quest monsters */
		if (is_quest(c->depth)) {
			int i;
			for (i = 1; i < z_info->r_max; i++) {
				monster_race *r_ptr = &r_info[i];
				int y, x;
				
				/* The monster must be an unseen quest monster of this depth. */
				if (r_ptr->cur_num > 0) continue;
				if (!rf_has(r_ptr->flags, RF_QUESTOR)) continue;
				if (r_ptr->level != c->depth) continue;
	
				/* Pick a location and place the monster */
				find_empty(c, &y, &x);
				place_new_monster(c, y, x, r_ptr, TRUE, TRUE, ORIGIN_DROP);
			}
		}

		/* Place dungeon squares to trigger feeling (not in town) */
		if (player->depth)
			place_feeling(c);
		
		c->feeling = calc_obj_feeling(c) + calc_mon_feeling(c);

		/* Regenerate levels that overflow their maxima */
		if (o_max >= z_info->o_max) 
			error = "too many objects";
		if (cave_monster_max(cave) >= z_info->m_max)
			error = "too many monsters";

		if (error) ROOM_LOG("Generation restarted: %s.", error);
    }

    FREE(cave_squares);
    cave_squares = NULL;

    if (error) quit_fmt("cave_generate() failed 100 times!");

	/* Clear generation flags. */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			sqinfo_off(c->info[y][x], SQUARE_WALL_INNER);
			sqinfo_off(c->info[y][x], SQUARE_WALL_OUTER);
			sqinfo_off(c->info[y][x], SQUARE_WALL_SOLID);
			sqinfo_off(c->info[y][x], SQUARE_MON_RESTRICT);
		}
	}

    /* The dungeon is ready */
    character_dungeon = TRUE;

    c->created_at = turn;
}



struct init_module generate_module = {
    .name = "generate",
    .init = run_room_parser,
    .cleanup = NULL
};
