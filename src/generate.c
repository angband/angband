/*
 * File: generate.c
 * Purpose: Dungeon generation.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "math.h"
#include "files.h"
#include "generate.h"
#include "monster/mon-spell.h"
#include "object/tvalsval.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/**
 * This is the global structure representing dungeon generation info.
 */
static struct dun_data *dun;

/**
 * This is a global array of positions in the cave we're currently
 * generaating. It's used to quickly randomize all the current cave positions.
 */
static int *cave_squares = NULL;

static bool town_gen(struct cave *c, struct player *p);

static bool default_gen(struct cave *c, struct player *p);
static bool labyrinth_gen(struct cave *c, struct player *p);
static bool cavern_gen(struct cave *c, struct player *p);

static bool build_simple(struct cave *c, int y0, int x0);
static bool build_circular(struct cave *c, int y0, int x0);
static bool build_overlap(struct cave *c, int y0, int x0);
static bool build_crossed(struct cave *c, int y0, int x0);
static bool build_large(struct cave *c, int y0, int x0);
static bool build_nest(struct cave *c, int y0, int x0);
static bool build_pit(struct cave *c, int y0, int x0);
static bool build_lesser_vault(struct cave *c, int y0, int x0);
static bool build_medium_vault(struct cave *c, int y0, int x0);
static bool build_greater_vault(struct cave *c, int y0, int x0);

static void alloc_objects(struct cave *c, int set, int typ, int num, int depth, byte origin);
static bool alloc_object(struct cave *c, int set, int typ, int depth, byte origin);

#define ROOM_DEBUG(...) if (0) msg(__VA_ARGS__);

#define ROOM_LOG(...) if (OPT(cheat_room)) msg(__VA_ARGS__);

/*
 * Note that Level generation is *not* an important bottleneck, though it can
 * be annoyingly slow on older machines...  Thus we emphasize "simplicity" and
 * "correctness" over "speed".
 *
 * See the "vault.txt" file for more on vault generation.
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


/*
 * Dungeon allocation places and types, used with alloc_object().
 */
#define SET_CORR 1 /* Hallway */
#define SET_ROOM 2 /* Room */
#define SET_BOTH 3 /* Anywhere */

#define TYP_RUBBLE 1 /* Rubble */
#define TYP_TRAP 3 /* Trap */
#define TYP_GOLD 4 /* Gold */
#define TYP_OBJECT 5 /* Object */
#define TYP_GOOD 6 /* Great object */
#define TYP_GREAT 7 /* Great object */

/*
 * Maximum numbers of rooms along each axis (currently 6x18).
 * Used for building fixed-size arrays.
 */
#define MAX_ROOMS_ROW (DUNGEON_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL (DUNGEON_WID / BLOCK_WID)

/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX 100
#define DOOR_MAX 200
#define WALL_MAX 500
#define TUNN_MAX 900


/**
 * Structure to hold all "dungeon generation" data
 */
struct dun_data {
	/* The profile used to generate the level */
	const struct cave_profile *profile;

	/* Array of centers of rooms */
	int cent_n;
	struct loc cent[CENT_MAX];

	/* Array of possible door locations */
	int door_n;
	struct loc door[DOOR_MAX];

	/* Array of wall piercing locations */
	int wall_n;
	struct loc wall[WALL_MAX];

	/* Array of tunnel grids */
	int tunn_n;
	struct loc tunn[TUNN_MAX];

	/* Number of blocks along each axis */
	int row_rooms;
	int col_rooms;

	/* Array of which blocks are used */
	bool room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];

	/* Hack -- there is a pit/nest on this level */
	bool crowded;
};


/*
 * Profile used for generating the town level.
 */
static struct cave_profile town_profile = {
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


/* name function width height min-depth crowded? rarity %cutoff */
static struct room_profile default_rooms[] = {
	/* greater vaults only have rarity 1 but they have other checks */
	{"greater vault", build_greater_vault, 4, 6, 10, FALSE, 1, 100},

	/* very rare rooms (rarity=2) */
	{"medium vault", build_medium_vault, 2, 3, 5, FALSE, 2, 10},
	{"lesser vault", build_lesser_vault, 2, 3, 5, FALSE, 2, 25},
	{"monster pit", build_pit, 1, 3, 5, TRUE, 2, 40},
	{"monster nest", build_nest, 1, 3, 5, TRUE, 2, 50},

	/* unusual rooms (rarity=1) */
	{"large room", build_large, 1, 3, 3, FALSE, 1, 25},
	{"crossed room", build_crossed, 1, 3, 3, FALSE, 1, 50},
	{"circular room", build_circular, 2, 2, 1, FALSE, 1, 60},
	{"overlap room", build_overlap, 1, 3, 1, FALSE, 1, 100},

	/* normal rooms */
	{"simple room", build_simple, 1, 3, 1, FALSE, 0, 100}
};


/*
 * Profiles used for generating dungeon levels.
 */
#define NUM_CAVE_PROFILES 3
static struct cave_profile cave_profiles[NUM_CAVE_PROFILES] = {
	{
		"labyrinth", labyrinth_gen, 0, 200, 0, 0,

		/* tunnels -- not applicable */
		{"tunnel-default", 10, 30, 15, 25, 90},

		/* streamers -- not applicable */
		{"streamer-default", 5, 2, 3, 90, 2, 40},

		/* room_profiles -- not applicable */
		NULL,

		/* cutoff -- unused because of internal checks in labyrinth_gen  */
		100
	},
	{
		"cavern", cavern_gen, 0, 200, 0, 0,

		/* tunnels -- not applicable */
		{"tunnel-default", 10, 30, 15, 25, 90},

		/* streamers -- not applicable */
		{"streamer-default", 5, 2, 3, 90, 2, 40},

		/* room_profiles -- not applicable */
		NULL,

		/* cutoff -- debug  */
		10
	},
	{
		/* name builder dun_rooms dun_unusual max_rarity n_room_profiles */
		"default", default_gen, 50, 200, 2, N_ELEMENTS(default_rooms),

		/* name rnd chg con pen jct */
		{"tunnel-default", 10, 30, 15, 25, 90},

		/* name den rng mag mc qua qc */
		{"streamer-default", 5, 2, 3, 90, 2, 40},

		/* room_profiles */
		default_rooms,

		/* cutoff */
		100
	}
};


/**
 * Shuffle an array using Knuth's shuffle.
 */
static void shuffle(int *arr, int n) {
	int i, j, k;
	for (i = 0; i < n; i++) {
		j = randint0(n - i) + i;
		k = arr[j];
		arr[j] = arr[i];
		arr[i] = k;
	}
}


/**
 * Locate a square in y1 <= y < y2, x1 <= x < x2 which satisfies the given
 * predicate.
 */
static void _find_in_range(struct cave *c, int *y, int y1, int y2,
						   int *x, int x1, int x2, int *squares,
						   cave_predicate pred) {
	int yd = y2 - y1;
	int xd = x2 - x1;
	int i, n = yd * xd;
	bool done = FALSE;

	/* Test each square in (random) order for openness */
	for (i = 0; i < n && !done; i++) {
		int j = randint0(n - i) + i;
		int k = squares[j];
		squares[j] = squares[i];
		squares[i] = k;

		*y = (k / xd) + y1;
		*x = (k % xd) + x1;
		if (pred(c, *y, *x)) done = TRUE;
	}

	/* Deallocate memory, make sure we found an empty square, and return */
	assert(done);
}


/**
 * Locate a square in the dungeon which satisfies the given predicate.
 */
static void cave_find(struct cave *c, int *y, int *x, cave_predicate pred) {
	_find_in_range(c, y, 0, c->height, x, 0, c->width, cave_squares, pred);
}


/**
 * Locate a square in y1 <= y < y2, x1 <= x < x2 which satisfies the given
 * predicate.
 */
static void cave_find_in_range(struct cave *c, int *y, int y1, int y2,
							   int *x, int x1, int x2, cave_predicate pred) {
	int yd = y2 - y1;
	int xd = x2 - x1;
	int i, n = yd * xd;
	int *squares;

	/* Allocate the squares, and randomize their order */
	squares = C_ZNEW(n, int);
	for (i = 0; i < n; i++) squares[i] = i;

	/* Do the actual search */
	_find_in_range(c, y, y1, y2, x, x1, x2, squares, pred);

	/* Deallocate memory, make sure we found an empty square, and return */
	FREE(squares);
}


/**
 * Locate an empty square for 0 <= y < ymax, 0 <= x < xmax.
 */
static void find_empty(struct cave *c, int *y, int *x) {
	cave_find(c, y, x, cave_isempty);
}


/**
 * Locate an empty square for y1 <= y < y2, x1 <= x < x2.
 */
static void find_empty_range(struct cave *c, int *y, int y1, int y2, int *x, int x1, int x2) {
	cave_find_in_range(c, y, y1, y2, x, x1, x2, cave_isempty);
}


/**
 * Locate a grid nearby (y0, x0) within +/- yd, xd.
 */
static void find_nearby_grid(struct cave *c, int *y, int y0, int yd, int *x, int x0, int xd) {
	int y1 = y0 - yd;
	int x1 = x0 - xd;
	int y2 = y0 + yd + 1;
	int x2 = x0 + xd + 1;
	cave_find_in_range(c, y, y1, y2, x, x1, x2, cave_in_bounds);
}


/**
 * Given two points, pick a valid cardinal direction from one to the other.
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2) {
	/* Extract vertical and horizontal directions */
	*rdir = CMP(y2, y1);
	*cdir = CMP(x2, x1);

	/* If we only have one direction to go, then we're done */
	if (!*rdir || !*cdir) return;

	/* If we need to go diagonally, then choose a random direction */
	if (randint0(100) < 50)
		*rdir = 0;
	else
		*cdir = 0;
}


/**
 * Pick a random cardinal direction.
 */
static void rand_dir(int *rdir, int *cdir) {
	/* Pick a random direction and extract the dy/dx components */
	int i = randint0(4);
	*rdir = ddy_ddd[i];
	*cdir = ddx_ddd[i];
}


static bool cave_isstart(struct cave *c, int y, int x) {
	if (!cave_isempty(c, y, x)) return FALSE;
	if (cave_isvault(c, y, x)) return FALSE;
	return TRUE;
}

/**
 * Place the player at a random starting location.
 */
static void new_player_spot(struct cave *c, struct player *p) {
	int y, x;

	/* Try to find a good place to put the player */
	cave_find_in_range(c, &y, 0, c->height, &x, 0, c->width, cave_isstart);

	/* Create stairs the player came down if allowed and necessary */
	if (OPT(birth_no_stairs)) {
	} else if (p->create_down_stair) {
		cave_set_feat(c, y, x, FEAT_MORE);
		p->create_down_stair = FALSE;
	} else if (p->create_up_stair) {
		cave_set_feat(c, y, x, FEAT_LESS);
		p->create_up_stair = FALSE;
	}

	player_place(c, p, y, x);
}


/**
 * Return how many cardinal directions around (x, y) contain walls.
 */
static int next_to_walls(struct cave *c, int y, int x) {
	int k = 0;
	assert(cave_in_bounds(c, y, x));

	if (cave_iswall(c, y + 1, x)) k++;
	if (cave_iswall(c, y - 1, x)) k++;
	if (cave_iswall(c, y, x + 1)) k++;
	if (cave_iswall(c, y, x - 1)) k++;

	return k;
}


/**
 * Place rubble at (x, y).
 */
static void place_rubble(struct cave *c, int y, int x) {
	cave_set_feat(c, y, x, FEAT_RUBBLE);
}


/**
 * Place stairs (of the requested type 'feat' if allowed) at (x, y).
 *
 * All stairs from town go down. All stairs on an unfinished quest level go up.
 */
static void place_stairs(struct cave *c, int y, int x, int feat) {
	if (!c->depth)
		cave_set_feat(c, y, x, FEAT_MORE);
	else if (is_quest(c->depth) || c->depth >= MAX_DEPTH - 1)
		cave_set_feat(c, y, x, FEAT_LESS);
	else
		cave_set_feat(c, y, x, feat);
}


/**
 * Place random stairs at (x, y).
 */
static void place_random_stairs(struct cave *c, int y, int x) {
	int feat = randint0(100) < 50 ? FEAT_LESS : FEAT_MORE;
	if (cave_canputitem(c, y, x))
		place_stairs(c, y, x, feat);
}


/**
 * Place a random object at (x, y).
 */
void place_object(struct cave *c, int y, int x, int level, bool good,
	bool great, byte origin)
{
	s32b rating = 0;
	object_type otype;

	assert(cave_in_bounds(c, y, x));

	if (!cave_canputitem(c, y, x)) return;

	object_wipe(&otype);
	rating = make_object(c, &otype, level, good, great);

	if (otype.kind) {
		otype.origin = origin;
		otype.origin_depth = c->depth;

		/* Give it to the floor */
		/* XXX Should this be done in floor_carry? */
		if (!floor_carry(c, y, x, &otype)) {
			if (otype.artifact)
				otype.artifact->created = FALSE;
			return;
		} else {
 			if (otype.artifact)
				c->good_item = TRUE;
			c->obj_rating += rating;
		}
	}
}


/**
 * Place a random amount of gold at (x, y).
 */
void place_gold(struct cave *c, int y, int x, int level, byte origin) {
	object_type *i_ptr;
	object_type object_type_body;

	assert(cave_in_bounds(c, y, x));

	if (!cave_canputitem(c, y, x)) return;

	i_ptr = &object_type_body;
	object_wipe(i_ptr);
	make_gold(i_ptr, level, SV_GOLD_ANY);

	i_ptr->origin = origin;
	i_ptr->origin_depth = level;

	floor_carry(c, y, x, i_ptr);
}


/**
 * Place a secret door at (x, y).
 */
void place_secret_door(struct cave *c, int y, int x) {
	cave_set_feat(c, y, x, FEAT_SECRET);
}


/**
 * Place a closed door at (x, y).
 */
void place_closed_door(struct cave *c, int y, int x) {
	int tmp = randint0(400);

	if (tmp < 300)
		cave_set_feat(c, y, x, FEAT_DOOR_HEAD + 0x00);
	else if (tmp < 399)
		cave_set_feat(c, y, x, FEAT_DOOR_HEAD + randint1(7));
	else
		cave_set_feat(c, y, x, FEAT_DOOR_HEAD + 0x08 + randint0(8));
}


/**
 * Place a random door at (x, y).
 *
 * The door generated could be closed, open, broken, or secret.
 */
void place_random_door(struct cave *c, int y, int x) {
	int tmp = randint0(100);

	if (tmp < 30)
		cave_set_feat(c, y, x, FEAT_OPEN);
	else if (tmp < 40)
		cave_set_feat(c, y, x, FEAT_BROKEN);
	else if (tmp < 60)
		cave_set_feat(c, y, x, FEAT_SECRET);
	else
		place_closed_door(c, y, x);
}


/**
 * Chooses a vault of a particular kind at random.
 * 
 * Each vault has equal probability of being chosen. One weird thing is that
 * currently the v->typ indices are one off from the room type indices, which
 * means that build_greater_vault will call this function with "typ=8".
 *
 * TODO: Fix the weird type-off-by-one issue.
 */
struct vault *random_vault(int typ) {
	struct vault *v = vaults;
	struct vault *r = NULL;
	int n = 1;
	do {
		if (v->typ == typ) {
			if (one_in_(n)) r = v;
			n++;
		}
		v = v->next;
	} while(v);
	return r;
}


/**
 * Place some staircases near walls.
 */
static void alloc_stairs(struct cave *c, int feat, int num, int walls) {
	int y, x, i, j, done;

	/* Place "num" stairs */
	for (i = 0; i < num; i++) {
		/* Place some stairs */
		for (done = FALSE; !done; ) {
			/* Try several times, then decrease "walls" */
			for (j = 0; !done && j <= 1000; j++) {
				find_empty(c, &y, &x);

				if (next_to_walls(c, y, x) < walls) continue;

				place_stairs(c, y, x, feat);
				done = TRUE;
			}

			/* Require fewer walls */
			if (walls) walls--;
		}
	}
}


/**
 * Allocates 'num' random objects in the dungeon.
 *
 * See alloc_object() for more information.
 */
static void alloc_objects(struct cave *c, int set, int typ, int num, int depth,
	byte origin)
{
	int k;
	int l = 0;
	for (k = 0; k < num; k++) {
		bool ok = alloc_object(c, set, typ, depth, origin);
		if (!ok) l++;
	}
}


/**
 * Allocates a single random object in the dungeon.
 *
 * 'set' controls where the object is placed (corridor, room, either).
 * 'typ' conrols the kind of object (rubble, trap, gold, item).
 */
static bool alloc_object(struct cave *c, int set, int typ, int depth,
	byte origin)
{
	int x, y;
	int tries = 0;
	bool room;

	/* Pick a "legal" spot */
	while (tries < 2000) {
		tries++;

		find_empty(c, &y, &x);

		/* See if our spot is in a room or not */
		room = (c->info[y][x] & CAVE_ROOM) ? TRUE : FALSE;

		/* If we are ok with a corridor and we're in one, we're done */
		if (set & SET_CORR && !room) break;

		/* If we are ok with a room and we're in one, we're done */
		if (set & SET_ROOM && room) break;
	}

	if (tries == 2000) return FALSE;

	/* Place something */
	switch (typ) {
		case TYP_RUBBLE: place_rubble(c, y, x); break;
		case TYP_TRAP: place_trap(c, y, x); break;
		case TYP_GOLD: place_gold(c, y, x, depth, origin); break;
		case TYP_OBJECT: place_object(c, y, x, depth, FALSE, FALSE, origin); break;
		case TYP_GOOD: place_object(c, y, x, depth, TRUE, FALSE, origin); break;
		case TYP_GREAT: place_object(c, y, x, depth, TRUE, TRUE, origin); break;
	}
	return TRUE;
}



/**
 * Places a streamer of rock through dungeon.
 *
 * Note that their are actually six different terrain features used to
 * represent streamers. Three each of magma and quartz, one for basic vein, one
 * with hidden gold, and one with known gold. The hidden gold types are
 * currently unused.
 */
static void build_streamer(struct cave *c, int feat, int chance) {
	int i, tx, ty;
	int y, x, dir;

	/* Hack -- Choose starting point */
	y = rand_spread(DUNGEON_HGT / 2, 10);
	x = rand_spread(DUNGEON_WID / 2, 15);

	/* Choose a random direction */
	dir = ddd[randint0(8)];

	/* Place streamer into dungeon */
	while (TRUE) {
		/* One grid per density */
		for (i = 0; i < dun->profile->str.den; i++) {
			int d = dun->profile->str.rng;

			/* Pick a nearby grid */
			find_nearby_grid(c, &ty, y, d, &tx, x, d);

			/* Only convert walls */
			if (cave_isrock(c, ty, tx)) {
				/* Turn the rock into the vein type */
				cave_set_feat(c, ty, tx, feat);

				/* Sometimes add known treasure */
				if (one_in_(chance)) upgrade_mineral(c, ty, tx);
			}
		}

		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Stop at dungeon edge */
		if (!cave_in_bounds(c, y, x)) break;
	}
}


/**
 * Create up to 'num' objects near the given coordinates in a vault.
 */
static void vault_objects(struct cave *c, int y, int x, int depth, int num) {
	int i, j, k;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num) {
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i) {
			/* Pick a random location */
			find_nearby_grid(c, &j, y, 2, &k, x, 3);

			/* Require "clean" floor space */
			if (!cave_canputitem(c, j, k)) continue;

			/* Place an item or gold */
			if (randint0(100) < 75)
				place_object(c, j, k, depth, FALSE, FALSE, ORIGIN_SPECIAL);
			else
				place_gold(c, j, k, depth, ORIGIN_VAULT);

			/* Placement accomplished */
			break;
		}
	}
}

/**
 * Place a trap near (x, y), with a given displacement.
 */
static void vault_trap_aux(struct cave *c, int y, int x, int yd, int xd) {
	int tries, y1, x1;

	/* Find a nearby empty grid and place a trap */
	for (tries = 0; tries <= 5; tries++) {
		find_nearby_grid(c, &y1, y, yd, &x1, x, xd);
		if (!cave_isempty(c, y1, x1)) continue;

		place_trap(c, y1, x1);
		break;
	}
}


/**
 * Place 'num' traps near (x, y), with a given displacement.
 */
static void vault_traps(struct cave *c, int y, int x, int yd, int xd, int num) {
	int i;
	for (i = 0; i < num; i++)
		vault_trap_aux(c, y, x, yd, xd);
}


/**
 * Place 'num' sleeping monsters near (x, y).
 */
static void vault_monsters(struct cave *c, int y1, int x1, int depth, int num) {
	int k, i, y, x;

	/* Try to summon "num" monsters "near" the given location */
	for (k = 0; k < num; k++) {
		/* Try nine locations */
		for (i = 0; i < 9; i++) {
			int d = 1;

			/* Pick a nearby location */
			scatter(&y, &x, y1, x1, d, 0);

			/* Require "empty" floor grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Place the monster (allow groups) */
			place_monster(c, y, x, depth, TRUE, TRUE, ORIGIN_DROP_SPECIAL);

			break;
		}
	}
}


/**
 * Mark squares as being in a room, and optionally light them.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive.
 */
static void generate_room(struct cave *c, int y1, int x1, int y2, int x2, int light) {
	int y, x;
	int add = CAVE_ROOM | (light ? CAVE_GLOW : 0);
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			c->info[y][x] |= add;
}


/**
 * Fill a rectangle with a feature.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive.
 */
static void fill_rectangle(struct cave *c, int y1, int x1, int y2, int x2, int feat) {
	int y, x;
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			cave_set_feat(c, y, x, feat);
}


/**
 * Fill the edges of a rectangle with a feature.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive.
 */
static void draw_rectangle(struct cave *c, int y1, int x1, int y2, int x2, int feat) {
	int y, x;

	for (y = y1; y <= y2; y++) {
		cave_set_feat(c, y, x1, feat);
		cave_set_feat(c, y, x2, feat);
	}

	for (x = x1; x <= x2; x++) {
		cave_set_feat(c, y1, x, feat);
		cave_set_feat(c, y2, x, feat);
	}
}


static void fill_xrange(struct cave *c, int y, int x1, int x2, int feat, int info) {
	int x;
	for (x = x1; x <= x2; x++) {
		cave_set_feat(c, y, x, feat);
		c->info[y][x] |= info;
	}
}


static void fill_yrange(struct cave *c, int x, int y1, int y2, int feat, int info) {
	int y;
	for (y = y1; y <= y2; y++) {
		cave_set_feat(c, y, x, feat);
		c->info[y][x] |= info;
	}
}


static void fill_circle(struct cave *c, int y0, int x0, int radius, int border, int feat, int info) {
	int i, last = 0;
	int r2 = radius * radius;
	for(i = 0; i <= radius; i++) {
		double j = sqrt(r2 - (i * i));
		int k = (int)(j + 0.5);

		int b = border;
		if (border && last > k) b++;
		
		fill_xrange(c, y0 - i, x0 - k - b, x0 + k + b, feat, info);
		fill_xrange(c, y0 + i, x0 - k - b, x0 + k + b, feat, info);
		fill_yrange(c, x0 - i, y0 - k - b, y0 + k + b, feat, info);
		fill_yrange(c, x0 + i, y0 - k - b, y0 + k + b, feat, info);
		last = k;
	}
}


/**
 * Fill the lines of a cross/plus with a feature.
 *
 * The boundaries (y1, x1, y2, x2) are inclusive. When combined with
 * draw_rectangle() this will generate a large rectangular room which is split
 * into four sub-rooms.
 */
static void generate_plus(struct cave *c, int y1, int x1, int y2, int x2, int feat) {
	int y, x;

	/* Find the center */
	int y0 = (y1 + y2) / 2;
	int x0 = (x1 + x2) / 2;

	assert(c);

	for (y = y1; y <= y2; y++) cave_set_feat(c, y, x0, feat);
	for (x = x1; x <= x2; x++) cave_set_feat(c, y0, x, feat);
}


/**
 * Generate helper -- open all sides of a rectangle with a feature
 */
static void generate_open(struct cave *c, int y1, int x1, int y2, int x2, int feat) {
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open all sides */
	cave_set_feat(c, y1, x0, feat);
	cave_set_feat(c, y0, x1, feat);
	cave_set_feat(c, y2, x0, feat);
	cave_set_feat(c, y0, x2, feat);
}


/**
 * Generate helper -- open one side of a rectangle with a feature
 */
static void generate_hole(struct cave *c, int y1, int x1, int y2, int x2, int feat) {
	/* Find the center */
	int y0 = (y1 + y2) / 2;
	int x0 = (x1 + x2) / 2;

	assert(c);

	/* Open random side */
	switch (randint0(4)) {
		case 0: cave_set_feat(c, y1, x0, feat); break;
		case 1: cave_set_feat(c, y0, x1, feat); break;
		case 2: cave_set_feat(c, y2, x0, feat); break;
		case 3: cave_set_feat(c, y0, x2, feat); break;
	}
}


/**
 * Build a circular room (interior radius 4-7).
 */
static bool build_circular(struct cave *c, int y0, int x0) {
	/* Pick a room size */
	int radius = 2 + randint1(2) + randint1(3);

	/* Occasional light */
	bool light = c->depth <= randint1(25) ? TRUE : FALSE;

	/* Mark interior squares as being in a room (optionally lit) */
	int info = CAVE_ROOM | (light ? CAVE_GLOW : 0);

	/* Generate outer walls and inner floors */
	fill_circle(c, y0, x0, radius + 1, 1, FEAT_WALL_OUTER, info);
	fill_circle(c, y0, x0, radius, 0, FEAT_FLOOR, info);

	/* Especially large circular rooms will have a middle chamber */
	if (radius - 4 > 0 && randint0(4) < radius - 4) {
		/* choose a random direction */
		int cd, rd;
		rand_dir(&rd, &cd);

		/* draw a room with a secret door on a random side */
		draw_rectangle(c, y0 - 2, x0 - 2, y0 + 2, x0 + 2, FEAT_WALL_INNER);
		cave_set_feat(c, y0 + cd * 2, x0 + rd * 2, FEAT_SECRET);

		/* Place a treasure in the vault */
		vault_objects(c, y0, x0, c->depth, randint0(2));

		/* create some monsterss */
		vault_monsters(c, y0, x0, c->depth + 1, randint0(3));
	}

	return TRUE;
}


/**
 * Builds a normal rectangular room.
 */
static bool build_simple(struct cave *c, int y0, int x0) {
	int y, x;
	int light = FALSE;

	/* Pick a room size */
	int y1 = y0 - randint1(4);
	int x1 = x0 - randint1(11);
	int y2 = y0 + randint1(3);
	int x2 = x0 + randint1(11);

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls and inner floors */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	if (one_in_(20)) {
		/* Sometimes make a pillar room */
		for (y = y1; y <= y2; y += 2)
			for (x = x1; x <= x2; x += 2)
				cave_set_feat(c, y, x, FEAT_WALL_INNER);

	} else if (one_in_(50)) {
		/* Sometimes make a ragged-edge room */
		for (y = y1 + 2; y <= y2 - 2; y += 2) {
			cave_set_feat(c, y, x1, FEAT_WALL_INNER);
			cave_set_feat(c, y, x2, FEAT_WALL_INNER);
		}

		for (x = x1 + 2; x <= x2 - 2; x += 2) {
			cave_set_feat(c, y1, x, FEAT_WALL_INNER);
			cave_set_feat(c, y2, x, FEAT_WALL_INNER);
		}
	}
	return TRUE;
}


/**
 * Builds an overlapping rectangular room.
 */
static bool build_overlap(struct cave *c, int y0, int x0) {
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int light = FALSE;

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Determine extents of room (a) */
	y1a = y0 - randint1(4);
	x1a = x0 - randint1(11);
	y2a = y0 + randint1(3);
	x2a = x0 + randint1(10);

	/* Determine extents of room (b) */
	y1b = y0 - randint1(3);
	x1b = x0 - randint1(10);
	y2b = y0 + randint1(4);
	x2b = x0 + randint1(11);

	/* Generate new room (a) */
	generate_room(c, y1a-1, x1a-1, y2a+1, x2a+1, light);

	/* Generate new room (b) */
	generate_room(c, y1b-1, x1b-1, y2b+1, x2b+1, light);

	/* Generate outer walls (a) */
	draw_rectangle(c, y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	draw_rectangle(c, y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	fill_rectangle(c, y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	fill_rectangle(c, y1b, x1b, y2b, x2b, FEAT_FLOOR);

	return TRUE;
}


/**
 * Builds a cross-shaped room.
 *
 * Room "a" runs north/south, and Room "b" runs east/east 
 * So a "central pillar" would run from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that the code
 * below will work for 5x5 (and perhaps even for unsymetric values like 4x3 or
 * 5x3 or 3x4 or 3x5).
 */
static bool build_crossed(struct cave *c, int y0, int x0) {
	int y, x;

	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int dy, dx, wy, wx;

	int light = FALSE;

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Pick inner dimension */
	wy = 1;
	wx = 1;

	/* Pick outer dimension */
	dy = rand_range(3, 4);
	dx = rand_range(3, 11);

	/* Determine extents of room (a) */
	y1a = y0 - dy;
	x1a = x0 - wx;
	y2a = y0 + dy;
	x2a = x0 + wx;

	/* Determine extents of room (b) */
	y1b = y0 - wy;
	x1b = x0 - dx;
	y2b = y0 + wy;
	x2b = x0 + dx;

	/* Generate new room (a) */
	generate_room(c, y1a-1, x1a-1, y2a+1, x2a+1, light);

	/* Generate new room (b) */
	generate_room(c, y1b-1, x1b-1, y2b+1, x2b+1, light);

	/* Generate outer walls (a) */
	draw_rectangle(c, y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	draw_rectangle(c, y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	fill_rectangle(c, y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	fill_rectangle(c, y1b, x1b, y2b, x2b, FEAT_FLOOR);

	/* Special features */
	switch (randint1(4)) {
		/* Nothing */
		case 1: break;

		/* Large solid middle pillar */
		case 2: {
			fill_rectangle(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);
			break;
		}

		/* Inner treasure vault */
		case 3: {
			/* Generate a small inner vault */
			draw_rectangle(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			/* Open the inner vault with a secret door */
			generate_hole(c, y1b, x1a, y2b, x2a, FEAT_SECRET);

			/* Place a treasure in the vault */
			place_object(c, y0, x0, c->depth, FALSE, FALSE, ORIGIN_SPECIAL);

			/* Let's guard the treasure well */
			vault_monsters(c, y0, x0, c->depth + 2, randint0(2) + 3);

			/* Traps naturally */
			vault_traps(c, y0, x0, 4, 4, randint0(3) + 2);

			break;
		}

		/* Something else */
		case 4: {
			if (one_in_(3)) {
				/* Occasionally pinch the center shut */

				/* Pinch the east/west sides */
				for (y = y1b; y <= y2b; y++) {
					if (y == y0) continue;
					cave_set_feat(c, y, x1a - 1, FEAT_WALL_INNER);
					cave_set_feat(c, y, x2a + 1, FEAT_WALL_INNER);
				}

				/* Pinch the north/south sides */
				for (x = x1a; x <= x2a; x++) {
					if (x == x0) continue;
					cave_set_feat(c, y1b - 1, x, FEAT_WALL_INNER);
					cave_set_feat(c, y2b + 1, x, FEAT_WALL_INNER);
				}

				/* Open sides with secret doors */
				if (one_in_(3))
					generate_open(c, y1b-1, x1a-1, y2b+1, x2a+1, FEAT_SECRET);

			} else if (one_in_(3)) {
				/* Occasionally put a "plus" in the center */
				generate_plus(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			} else if (one_in_(3)) {
				/* Occasionally put a "pillar" in the center */
				cave_set_feat(c, y0, x0, FEAT_WALL_INNER);
			}

			break;
		}
	}

	return TRUE;
}


/**
 * Build a large room with an inner room.
 *
 * Possible sub-types:
 *	1 - An inner room
 *	2 - An inner room with a small inner room
 *	3 - An inner room with a pillar or pillars
 *	4 - An inner room with a checkerboard
 *	5 - An inner room with four compartments
 */
static bool build_large(struct cave *c, int y0, int x0) {
	int y, x, y1, x1, y2, x2;

	int light = FALSE;

	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;

	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Inner room variations */
	switch (randint1(5)) {
		/* An inner room */
		case 1: {
			/* Open the inner room with a secret door and place a monster */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);
			vault_monsters(c, y0, x0, c->depth + 2, 1);
			break;
		}


		/* An inner room with a small inner room */
		case 2: {
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Place another inner room */
			draw_rectangle(c, y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Open the inner room with a locked door */
			generate_hole(c, y0-1, x0-1, y0+1, x0+1, FEAT_DOOR_HEAD + randint1(7));

			/* Monsters to guard the treasure */
			vault_monsters(c, y0, x0, c->depth + 2, randint1(3) + 2);

			/* Object (80%) or Stairs (20%) */
			if (randint0(100) < 80)
				place_object(c, y0, x0, c->depth, FALSE, FALSE, ORIGIN_SPECIAL);
			else
				place_random_stairs(c, y0, x0);

			/* Traps to protect the treasure */
			vault_traps(c, y0, x0, 4, 10, 2 + randint1(3));

			break;
		}


		/* An inner room with an inner pillar or pillars */
		case 3: {
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Inner pillar */
			fill_rectangle(c, y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Occasionally, two more Large Inner Pillars */
			if (one_in_(2)) {
				if (one_in_(2)) {
					fill_rectangle(c, y0-1, x0-7, y0+1, x0-5, FEAT_WALL_INNER);
					fill_rectangle(c, y0-1, x0+5, y0+1, x0+7, FEAT_WALL_INNER);
				} else {
					fill_rectangle(c, y0-1, x0-6, y0+1, x0-4, FEAT_WALL_INNER);
					fill_rectangle(c, y0-1, x0+4, y0+1, x0+6, FEAT_WALL_INNER);
				}
			}

			/* Occasionally, some Inner rooms */
			if (one_in_(3)) {
				/* Inner rectangle */
				draw_rectangle(c, y0-1, x0-5, y0+1, x0+5, FEAT_WALL_INNER);

				/* Secret doors (random top/bottom) */
				place_secret_door(c, y0 - 3 + (randint1(2) * 2), x0 - 3);
				place_secret_door(c, y0 - 3 + (randint1(2) * 2), x0 + 3);

				/* Monsters */
				vault_monsters(c, y0, x0 - 2, c->depth + 2, randint1(2));
				vault_monsters(c, y0, x0 + 2, c->depth + 2, randint1(2));

				/* Objects */
				if (one_in_(3))
					place_object(c, y0, x0 - 2, c->depth, FALSE, FALSE,
						ORIGIN_SPECIAL);
				if (one_in_(3))
					place_object(c, y0, x0 + 2, c->depth, FALSE, FALSE,
						ORIGIN_SPECIAL);
			}

			break;
		}


		/* An inner room with a checkerboard */
		case 4: {
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Checkerboard */
			for (y = y1; y <= y2; y++)
				for (x = x1; x <= x2; x++)
					if ((x + y) & 0x01)
						cave_set_feat(c, y, x, FEAT_WALL_INNER);

			/* Monsters just love mazes. */
			vault_monsters(c, y0, x0 - 5, c->depth + 2, randint1(3));
			vault_monsters(c, y0, x0 + 5, c->depth + 2, randint1(3));

			/* Traps make them entertaining. */
			vault_traps(c, y0, x0 - 3, 2, 8, randint1(3));
			vault_traps(c, y0, x0 + 3, 2, 8, randint1(3));

			/* Mazes should have some treasure too. */
			vault_objects(c, y0, x0, c->depth, 3);

			break;
		}


		/* Four small rooms. */
		case 5: {
			/* Inner "cross" */
			generate_plus(c, y1, x1, y2, x2, FEAT_WALL_INNER);

			/* Doors into the rooms */
			if (randint0(100) < 50) {
				int i = randint1(10);
				place_secret_door(c, y1 - 1, x0 - i);
				place_secret_door(c, y1 - 1, x0 + i);
				place_secret_door(c, y2 + 1, x0 - i);
				place_secret_door(c, y2 + 1, x0 + i);
			} else {
				int i = randint1(3);
				place_secret_door(c, y0 + i, x1 - 1);
				place_secret_door(c, y0 - i, x1 - 1);
				place_secret_door(c, y0 + i, x2 + 1);
				place_secret_door(c, y0 - i, x2 + 1);
			}

			/* Treasure, centered at the center of the cross */
			vault_objects(c, y0, x0, c->depth, 2 + randint1(2));

			/* Gotta have some monsters */
			vault_monsters(c, y0 + 1, x0 - 4, c->depth + 2, randint1(4));
			vault_monsters(c, y0 + 1, x0 + 4, c->depth + 2, randint1(4));
			vault_monsters(c, y0 - 1, x0 - 4, c->depth + 2, randint1(4));
			vault_monsters(c, y0 - 1, x0 + 4, c->depth + 2, randint1(4)); 

			break;
		}
	}

	return TRUE;
}


/* Hook for which type of pit we are building */
pit_profile *pit_type = NULL;

/**
 * Hook for picking monsters appropriate to a nest/pit.
 *
 * Requires pit_type to be set.
 */
static bool mon_pit_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* pit_type needs to be set */
	assert(pit_type);

	if (rf_has(r_ptr->flags, RF_UNIQUE))
		return FALSE;
	else if (!rf_is_subset(r_ptr->flags, pit_type->flags))
		return FALSE;
	else if (!rsf_is_subset(r_ptr->spell_flags, pit_type->spell_flags))
		return FALSE;
	else if (rsf_is_inter(r_ptr->spell_flags, pit_type->forbidden_spell_flags))
		return FALSE;
	else if (pit_type->n_bases > 0) {
		int i;
		for (i = 0; i < pit_type->n_bases; i++) {
			if (r_ptr->base == pit_type->base[i])
				return TRUE;
		}
		
		return FALSE;
	}

	return TRUE;
}

/**
 * Pick a type of monster pit, based on the level.
 *
 * We scan through all pits, and for each one generate a random depth
 * using a normal distribution, with the mean given in pit.txt, and a
 * standard deviation of 10. Then we pick the pit that gave us a depth that
 * is closest to the player's actual depth.
 *
 * Sets pit_type, which is required for mon_pit_hook.
 * Returns the index of the chosen pit.
 */
static int set_pit_type(int depth, int type)
{
	int i;
	int pit_idx = 0;
	
	/* Hack -- set initial distance large */
	int pit_dist = 999;
	
	for (i = 0; i < z_info->pit_max; i++)
	{
		int offset, dist;
		pit_profile *pit = &pit_info[i];
		
		/* Skip empty pits or pits of the wrong room type */
		if (!pit->name || pit->room_type != type) continue;
		
		offset = Rand_normal(pit->ave, 10);
		dist = ABS(offset - depth);
		
		if (dist < pit_dist && one_in_(pit->rarity))
		{
			/* This pit is the closest so far */
			pit_idx = i;
			pit_dist = dist;
		}
	}

	pit_type = &pit_info[pit_idx];
	get_mon_num_hook = mon_pit_hook;
        
	return pit_idx;
}


/**
 * Build a monster nest
 *
 * A monster nest consists of a rectangular moat around a room containing
 * monsters of a given type.
 *
 * The monsters are chosen from a set of 64 randomly selected monster races,
 * to allow the nest creation to fail instead of having "holes".
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the nest.
 *
 * The available monster nests are specified in edit/pit.txt.
 *
 * Note that get_mon_num() function can fail, in which case the nest will be
 * empty, and will not affect the level rating.
 *
 * Monster nests will never contain unique monsters.
 */
static bool build_nest(struct cave *c, int y0, int x0) {
	int y, x, y1, x1, y2, x2;
	int i;
	int alloc_obj;
	s16b what[64];
	bool empty = FALSE;
	int light = FALSE;
	int pit_idx;

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;

	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

	/* Set get_mon_num_hook */
	pit_idx = set_pit_type(c->depth, 2);

	/* Chance of objects on the floor */
	alloc_obj = pit_info[pit_idx].obj_rarity;
	
	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick some monster types */
	for (i = 0; i < 64; i++) {
		/* Get a (hard) monster type */
		what[i] = get_mon_num(c->depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}

	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Oops */
	if (empty) return FALSE;

	/* Describe */
	ROOM_LOG("Monster nest (%s)", pit_info[pit_idx].name);

	/* Increase the level rating */
	c->mon_rating += (5 + pit_info[pit_idx].ave / 10);

	/* Place some monsters */
	for (y = y0 - 2; y <= y0 + 2; y++) {
		for (x = x0 - 9; x <= x0 + 9; x++) {
			/* Figure out what monster is being used, and place that monster */
			int r_idx = what[randint0(64)];
			place_monster_aux(c, y, x, r_idx, FALSE, FALSE, ORIGIN_DROP_PIT);

			/* Occasionally place an item, making it good 1/3 of the time */
			if (one_in_(alloc_obj)) 
				place_object(c, y, x, c->depth + 10, one_in_(3), FALSE,
					ORIGIN_PIT);
		}
	}

	return TRUE;
}

/**
 * Build a monster pit
 *
 * Monster pits are laid-out similarly to monster nests.
 *
 * The available monster pits are specified in edit/pit.txt.
 *
 * The inside room in a monster pit appears as shown below, where the
 * actual monsters in each location depend on the type of the pit
 *
 *   #####################
 *   #0000000000000000000#
 *   #0112233455543322110#
 *   #0112233467643322110#
 *   #0112233455543322110#
 *   #0000000000000000000#
 *   #####################
 *
 * Note that the monsters in the pit are chosen by using get_mon_num() to
 * request 16 "appropriate" monsters, sorting them by level, and using the
 * "even" entries in this sorted list for the contents of the pit.
 *
 * Note the use of the get_mon_num_prep() function, and the special
 * get_mon_num_hook() restriction function, to prepare the monster allocation
 * table in such a way as to optimize the selection of appropriate non-unique
 * monsters for the pit.
 *
 * The get_mon_num() function can fail, in which case the pit will be empty,
 * and will not effect the level rating.
 *
 * Like monster nests, monster pits will never contain unique monsters.
 */
static bool build_pit(struct cave *c, int y0, int x0) {
	int what[16];
	int i, j, y, x, y1, x1, y2, x2;
	bool empty = FALSE;
	int light = FALSE;
	int pit_idx;

	/* Large room */
	y1 = y0 - 4;
	y2 = y0 + 4;
	x1 = x0 - 11;
	x2 = x0 + 11;

	/* Generate new room, outer walls and inner floor */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);
	fill_rectangle(c, y1, x1, y2, x2, FEAT_FLOOR);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls, and open with a secret door */
	draw_rectangle(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);
	generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

	/* Set get_mon_num_hook */
	pit_idx = set_pit_type(c->depth, 1);

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick some monster types */
	for (i = 0; i < 16; i++) {
		/* Get a (hard) monster type */
		what[i] = get_mon_num(c->depth + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}

	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Oops */
	if (empty)
		return FALSE;

	ROOM_LOG("Monster pit (%s)", pit_info[pit_idx].name);

	/* Sort the entries XXX XXX XXX */
	for (i = 0; i < 16 - 1; i++) {
		/* Sort the entries */
		for (j = 0; j < 16 - 1; j++) {
			int i1 = j;
			int i2 = j + 1;

			int p1 = r_info[what[i1]].level;
			int p2 = r_info[what[i2]].level;

			/* Bubble */
			if (p1 > p2) {
				int tmp = what[i1];
				what[i1] = what[i2];
				what[i2] = tmp;
			}
		}
	}

	/* Select every other entry */
	for (i = 0; i < 8; i++)
		what[i] = what[i * 2];

	/* Increase the level rating */
	c->mon_rating += (5 + pit_info[pit_idx].ave / 10);

	/* Top and bottom rows */
	for (x = x0 - 9; x <= x0 + 9; x++) {
		place_monster_aux(c, y0 - 2, x, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y0 + 2, x, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);
	}

	/* Middle columns */
	for (y = y0 - 1; y <= y0 + 1; y++) {
		place_monster_aux(c, y, x0 - 9, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 9, what[0], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 8, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 8, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 7, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 7, what[1], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 6, what[2], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 6, what[2], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 5, what[2], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 5, what[2], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 4, what[3], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 4, what[3], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 3, what[3], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 3, what[3], FALSE, FALSE, ORIGIN_DROP_PIT);

		place_monster_aux(c, y, x0 - 2, what[4], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y, x0 + 2, what[4], FALSE, FALSE, ORIGIN_DROP_PIT);
	}

	/* Above/Below the center monster */
	for (x = x0 - 1; x <= x0 + 1; x++) {
		place_monster_aux(c, y0 + 1, x, what[5], FALSE, FALSE, ORIGIN_DROP_PIT);
		place_monster_aux(c, y0 - 1, x, what[5], FALSE, FALSE, ORIGIN_DROP_PIT);
	}

	/* Next to the center monster */
	place_monster_aux(c, y0, x0 + 1, what[6], FALSE, FALSE, ORIGIN_DROP_PIT);
	place_monster_aux(c, y0, x0 - 1, what[6], FALSE, FALSE, ORIGIN_DROP_PIT);

	/* Center monster */
	place_monster_aux(c, y0, x0, what[7], FALSE, FALSE, ORIGIN_DROP_PIT);

	return TRUE;
}


/**
 * Build a vault from its string representation.
 */
static void build_vault(struct cave *c, int y0, int x0, int ymax, int xmax, const char *data) {
	int dx, dy, x, y;
	const char *t;
	bool icky;

	assert(c);

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax && *t; dy++) {
		for (dx = 0; dx < xmax && *t; dx++, t++) {
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Skip non-grids */
			if (*t == ' ') continue;

			/* Lay down a floor */
			cave_set_feat(c, y, x, FEAT_FLOOR);

			/* Debugging assertion */
			assert(cave_isempty(c, y, x));

			/* By default vault squares are marked icky */
			icky = TRUE;

			/* Analyze the grid */
			switch (*t) {
				case '%': {
					/* In this case, the square isn't really part of the
					 * vault, but rather is part of the "door step" to the
					 * vault. We don't mark it icky so that the tunneling
					 * code knows its allowed to remove this wall. */
					cave_set_feat(c, y, x, FEAT_WALL_OUTER);
					icky = FALSE;
					break;
				}
				case '#': cave_set_feat(c, y, x, FEAT_WALL_INNER); break;
				case 'X': cave_set_feat(c, y, x, FEAT_PERM_INNER); break;
				case '+': place_secret_door(c, y, x); break;
				case '^': place_trap(c, y, x); break;
				case '*': {
					/* Treasure or a trap */
					if (randint0(100) < 75)
						place_object(c, y, x, c->depth, FALSE, FALSE, ORIGIN_VAULT);
					else
						place_trap(c, y, x);
					break;
				}
			}

			/* Part of a vault */
			c->info[y][x] |= CAVE_ROOM;
			if (icky) c->info[y][x] |= CAVE_ICKY;
		}
	}


	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax && *t; dy++) {
		for (dx = 0; dx < xmax && *t; dx++, t++) {
			/* Extract the grid */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the symbol */
			switch (*t) {
				case '&': place_monster(c, y, x, c->depth + 5, TRUE, TRUE,
					ORIGIN_DROP_VAULT); break;
				case '@': place_monster(c, y, x, c->depth + 11, TRUE, TRUE,
					ORIGIN_DROP_VAULT); break;

				case '9': {
					/* Meaner monster, plus treasure */
					place_monster(c, y, x, c->depth + 9, TRUE, TRUE,
						ORIGIN_DROP_VAULT);
					place_object(c, y, x, c->depth + 7, TRUE, FALSE,
						ORIGIN_VAULT);
					break;
				}

				case '8': {
					/* Nasty monster and treasure */
					place_monster(c, y, x, c->depth + 40, TRUE, TRUE,
						ORIGIN_DROP_VAULT);
					place_object(c, y, x, c->depth + 20, TRUE, TRUE,
						ORIGIN_VAULT);
					break;
				}

				case ',': {
					/* Monster and/or object */
					if (randint0(100) < 50)
						place_monster(c, y, x, c->depth + 3, TRUE, TRUE,
							ORIGIN_DROP_VAULT);
					if (randint0(100) < 50)
						place_object(c, y, x, c->depth + 7, FALSE, FALSE,
							ORIGIN_VAULT);
					break;
				}
			}
		}
	}
}


/**
 * Helper function for building vaults.
 */
static bool build_vault_type(struct cave*c, int y0, int x0, int typ, const char *label) {
	vault_type *v_ptr = random_vault(typ);
	if (v_ptr == NULL) {
		/*quit_fmt("got NULL from random_vault(%d)", typ);*/
		return FALSE;
	}

	ROOM_LOG("%s (%s)", label, v_ptr->name);

	/* Boost the rating */
	c->mon_rating += v_ptr->rat;

	/* Build the vault */
	build_vault(c, y0, x0, v_ptr->hgt, v_ptr->wid, v_ptr->text);

	return TRUE;
}


/**
 * Build a lesser vault.
 */
static bool build_lesser_vault(struct cave *c, int y0, int x0) {
	return build_vault_type(c, y0, x0, 6, "Lesser vault");
}


/**
 * Build a (medium) vault.
 */
static bool build_medium_vault(struct cave *c, int y0, int x0) {
	return build_vault_type(c, y0, x0, 7, "Medium vault");
}


/**
 * Build a greater vaults.
 *
 * Since Greater Vaults are so large (4x6 blocks, in a 6x18 dungeon) there is
 * a 63% chance that a randomly chosen quadrant to start a GV on won't work.
 * To balance this, we give Greater Vaults an artificially high probability
 * of being attempted, and then in this function use a depth check to cancel
 * vault creation except at deep depths.
 *
 * The following code should make a greater vault with frequencies:
 * dlvl  freq
 * 100+  18.0%
 * 90-99 16.0 - 18.0%
 * 80-89 10.0 - 11.0%
 * 70-79  5.7 -  6.5%
 * 60-69  3.3 -  3.8%
 * 50-59  1.8 -  2.1%
 * 0-49   0.0 -  1.0%
 */
static bool build_greater_vault(struct cave *c, int y0, int x0) {
	int i;
	int numerator   = 2;
	int denominator = 3;
	
	/* Only try to build a GV as the first room. */
	if (dun->cent_n > 0) return FALSE;

	/* Level 90+ has a 2/3 chance, level 80-89 has 4/9, ... */
	for(i = 90; i > c->depth; i -= 10) {
		numerator *= 2;
		denominator *= 3;
	}

	/* Attempt to pass the depth check and build a GV */
	if (randint0(denominator) >= numerator) return FALSE;

	return build_vault_type(c, y0, x0, 8, "Greater vault");
}


/**
 * Constructs a tunnel between two points
 *
 * This function must be called BEFORE any streamers are created, since we use
 * the special "granite wall" sub-types to keep track of legal places for
 * corridors to pierce rooms.
 *
 * We queue the tunnel grids to prevent door creation along a corridor which
 * intersects itself.
 *
 * We queue the wall piercing grids to prevent a corridor from leaving
 * a room and then coming back in through the same entrance.
 *
 * We pierce grids which are outer walls of rooms, and when we do so, we change
 * all adjacent outer walls of rooms into solid walls so that no two corridors
 * may use adjacent grids for exits.
 *
 * The solid wall check prevents corridors from chopping the corners of rooms
 * off, as well as silly door placement, and excessively wide room entrances.
 */
static void build_tunnel(struct cave *c, int row1, int col1, int row2, int col2) {
	int i, y, x;
	int tmp_row, tmp_col;
	int row_dir, col_dir;
	int start_row, start_col;
	int main_loop_count = 0;

	/* Used to prevent excessive door creation along overlapping corridors. */
	bool door_flag = FALSE;
	
	/* Reset the arrays */
	dun->tunn_n = 0;
	dun->wall_n = 0;
	
	/* Save the starting location */
	start_row = row1;
	start_col = col1;

	/* Start out in the correct direction */
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

	/* Keep going until done (or bored) */
	while ((row1 != row2) || (col1 != col2)) {
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) break;

		/* Allow bends in the tunnel */
		if (randint0(100) < dun->profile->tun.chg) {
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (randint0(100) < dun->profile->tun.rnd)
				rand_dir(&row_dir, &col_dir);
		}

		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;

		while (!cave_in_bounds(c, tmp_row, tmp_col)) {
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (randint0(100) < dun->profile->tun.rnd)
				rand_dir(&row_dir, &col_dir);

			/* Get the next location */
			tmp_row = row1 + row_dir;
			tmp_col = col1 + col_dir;
		}


		/* Avoid the edge of the dungeon */
		if (cave_isperm(c, tmp_row, tmp_col)) continue;

		/* Avoid "solid" granite walls */
		if (c->feat[tmp_row][tmp_col] == FEAT_WALL_SOLID) continue;

		/* Pierce "outer" walls of rooms */
		if (c->feat[tmp_row][tmp_col] == FEAT_WALL_OUTER) {
			/* Get the "next" location */
			y = tmp_row + row_dir;
			x = tmp_col + col_dir;

			/* Hack -- Avoid outer/solid permanent walls */
			if (c->feat[y][x] == FEAT_PERM_SOLID) continue;
			if (c->feat[y][x] == FEAT_PERM_OUTER) continue;

			/* Hack -- Avoid outer/solid granite walls */
			if (c->feat[y][x] == FEAT_WALL_OUTER) continue;
			if (c->feat[y][x] == FEAT_WALL_SOLID) continue;

			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the wall location */
			if (dun->wall_n < WALL_MAX) {
				dun->wall[dun->wall_n].y = row1;
				dun->wall[dun->wall_n].x = col1;
				dun->wall_n++;
			}

			/* Forbid re-entry near this piercing */
			for (y = row1 - 1; y <= row1 + 1; y++)
				for (x = col1 - 1; x <= col1 + 1; x++)
					if (c->feat[y][x] == FEAT_WALL_OUTER)
						cave_set_feat(c, y, x, FEAT_WALL_SOLID);

		} else if (c->info[tmp_row][tmp_col] & (CAVE_ROOM)) {
			/* Travel quickly through rooms */
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

		} else if (c->feat[tmp_row][tmp_col] >= FEAT_WALL_EXTRA) {
			/* Tunnel through all other walls */
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX) {
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			door_flag = FALSE;

		} else {
			/* Handle corridor intersections or overlaps */
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Collect legal door locations */
			if (!door_flag) {
				/* Save the door location */
				if (dun->door_n < DOOR_MAX) {
					dun->door[dun->door_n].y = row1;
					dun->door[dun->door_n].x = col1;
					dun->door_n++;
				}

				/* No door in next grid */
				door_flag = TRUE;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if (randint0(100) >= dun->profile->tun.con) {
				/* Distance between row1 and start_row */
				tmp_row = row1 - start_row;
				if (tmp_row < 0) tmp_row = (-tmp_row);

				/* Distance between col1 and start_col */
				tmp_col = col1 - start_col;
				if (tmp_col < 0) tmp_col = (-tmp_col);

				/* Terminate the tunnel */
				if ((tmp_row > 10) || (tmp_col > 10)) break;
			}
		}
	}


	/* Turn the tunnel into corridor */
	for (i = 0; i < dun->tunn_n; i++) {
		/* Get the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Clear previous contents, add a floor */
		cave_set_feat(c, y, x, FEAT_FLOOR);
	}


	/* Apply the piercings that we found */
	for (i = 0; i < dun->wall_n; i++) {
		/* Get the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Convert to floor grid */
		cave_set_feat(c, y, x, FEAT_FLOOR);

		/* Place a random door */
		if (randint0(100) < dun->profile->tun.pen)
			place_random_door(c, y, x);
	}
}

/**
 * Count the number of corridor grids adjacent to the given grid.
 *
 * This routine currently only counts actual "empty floor" grids which are not
 * in rooms.
 *
 * TODO: count stairs, open doors, closed doors?
 */
static int next_to_corr(struct cave *c, int y1, int x1) {
	int i, k = 0;
	assert(cave_in_bounds(c, y1, x1));

	/* Scan adjacent grids */
	for (i = 0; i < 4; i++) {
		/* Extract the location */
		int y = y1 + ddy_ddd[i];
		int x = x1 + ddx_ddd[i];

		/* Count only floors which aren't part of rooms */
		if (cave_isfloor(c, y, x) && !cave_isroom(c, y, x)) k++;
	}

	/* Return the number of corridors */
	return k;
}

/**
 * Returns whether a doorway can be built in a space.
 *
 * To have a doorway, a space must be adjacent to at least two corridors and be
 * between two walls.
 */
static bool possible_doorway(struct cave *c, int y, int x) {
	assert(cave_in_bounds(c, y, x));

	if (next_to_corr(c, y, x) < 2)
		return FALSE;

	if (cave_isstrongwall(c, y - 1, x) && cave_isstrongwall(c, y + 1, x))
		return TRUE;

	if (cave_isstrongwall(c, y, x - 1) && cave_isstrongwall(c, y, x + 1))
		return TRUE;

	return FALSE;
}


/**
 * Places door at y, x position if at least 2 walls found
 */
static void try_door(struct cave *c, int y, int x) {
	assert(cave_in_bounds(c, y, x));

	if (cave_isstrongwall(c, y, x)) return;
	if (cave_isroom(c, y, x)) return;

	if (randint0(100) < dun->profile->tun.jct && possible_doorway(c, y, x))
		place_random_door(c, y, x);
}


/**
 * Attempt to build a room of the given type at the given block
 *
 * Note that we restrict the number of "crowded" rooms to reduce
 * the chance of overflowing the monster list during level creation.
 */
static bool room_build(struct cave *c, int by0, int bx0, struct room_profile profile) {
	/* Extract blocks */
	int by1 = by0;
	int bx1 = bx0;
	int by2 = by0 + profile.height;
	int bx2 = bx0 + profile.width;

	int allocated;
	int y, x;
	int by, bx;

	/* Enforce the room profile's minimum depth */
	if (c->depth < profile.level) return FALSE;

	/* Only allow one crowded room per level */
	if (dun->crowded && profile.crowded) return FALSE;

	/* Never run off the screen */
	if (by1 < 0 || by2 >= dun->row_rooms) return FALSE;
	if (bx1 < 0 || bx2 >= dun->col_rooms) return FALSE;

	/* Verify open space */
	for (by = by1; by <= by2; by++) {
		for (bx = bx1; bx <= bx2; bx++) {
			if (1) {
				/* previous rooms prevent new ones */
				if (dun->room_map[by][bx]) return FALSE;
			} else {
				return FALSE; /* XYZ */
			}
		}
	}

	/* Get the location of the room */
	y = ((by1 + by2 + 1) * BLOCK_HGT) / 2;
	x = ((bx1 + bx2 + 1) * BLOCK_WID) / 2;

	/* Try to build a room */
	if (!profile.builder(c, y, x)) return FALSE;

	/* Save the room location */
	if (dun->cent_n < CENT_MAX) {
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;
	}

	/* Reserve some blocks */
	allocated = 0;
	for (by = by1; by < by2; by++) {
		for (bx = bx1; bx < bx2; bx++) {
			dun->room_map[by][bx] = TRUE;
			allocated++;
		}
	}

	/* Count "crowded" rooms */
	if (profile.crowded) dun->crowded = TRUE;

	/* Success */
	return TRUE;
}

static void set_cave_dimensions(struct cave *c, int h, int w) {
	int i, n = h * w;
	c->height = h;
	c->width  = w;
	if (cave_squares != NULL) FREE(cave_squares);
	cave_squares = C_ZNEW(n, int);
	for (i = 0; i < n; i++) cave_squares[i] = i;
}


/**
 * Generate a new dungeon level.
 */
#define DUN_AMT_ROOM 7 /* Number of objects for rooms */
#define DUN_AMT_ITEM 2 /* Number of objects for rooms/corridors */
#define DUN_AMT_GOLD 3 /* Amount of treasure for rooms/corridors */
static bool default_gen(struct cave *c, struct player *p) {
	int i, j, k, y, x, y1, x1;
	int by, bx = 0, tby, tbx, key, rarity, built;
	int num_rooms, size_percent;
	int dun_unusual = dun->profile->dun_unusual;

	bool blocks_tried[MAX_ROOMS_ROW][MAX_ROOMS_COL];
	for (by = 0; by < MAX_ROOMS_ROW; by++) {
		for (bx = 0; bx < MAX_ROOMS_COL; bx++) {
			blocks_tried[by][bx] = FALSE;
		}
	}

	/* Possibly generate fewer rooms in a smaller area via a scaling factor.
	 * Since we scale row_rooms and col_rooms by the same amount, DUN_ROOMS
	 * gives the same "room density" no matter what size the level turns out
	 * to be. TODO: vary room density slightly? */

	/* XXX: Until vault generation is improved, scaling variance is removed */
	i = randint1(10) + c->depth / 24;
	if (is_quest(c->depth)) size_percent = 100;
	else if (i < 2) size_percent = 75;
	else if (i < 3) size_percent = 80;
	else if (i < 4) size_percent = 85;
	else if (i < 5) size_percent = 90;
	else if (i < 6) size_percent = 95;
	else size_percent = 100;

	/* scale the various generation variables */
	num_rooms = (dun->profile->dun_rooms * size_percent) / 100;
	set_cave_dimensions(c, DUNGEON_HGT, DUNGEON_WID);
	//ROOM_LOG("height=%d  width=%d  nrooms=%d", c->height, c->width, num_rooms);

	/* Initially fill with basic granite */
	fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_WALL_EXTRA);

	/* Actual maximum number of rooms on this level */
	dun->row_rooms = c->height / BLOCK_HGT;
	dun->col_rooms = c->width / BLOCK_WID;

	/* Initialize the room table */
	for (by = 0; by < dun->row_rooms; by++)
		for (bx = 0; bx < dun->col_rooms; bx++)
			dun->room_map[by][bx] = blocks_tried[by][bx]  = FALSE;

	/* No rooms yet, crowded or otherwise. */
	dun->crowded = FALSE;
	dun->cent_n = 0;

	/* Build some rooms */
	built = 0;
	while(built < num_rooms) {

		/* Count the room blocks we haven't tried yet. */
		j = 0;
		tby = 0;
		tbx = 0;
		for(by = 0; by < dun->row_rooms; by++) {
			for(bx = 0; bx < dun->col_rooms; bx++) {
				if (blocks_tried[by][bx]) continue;
				j++;
				if (one_in_(j)) {
					tby = by;
					tbx = bx;
				}
			} 
		}
		bx = tbx;
		by = tby;

		/* If we've tried all blocks we're done. */
		if (j == 0) break;

		if (blocks_tried[by][bx]) quit_fmt("generation: inconsistent blocks");

		/* Mark that we are trying this block. */
		blocks_tried[by][bx] = TRUE;

		/* Roll for random key (to be compared against a profile's cutoff) */
		key = randint0(100);

		/* We generate a rarity number to figure out how exotic to make the
		 * room. This number has a depth/DUN_UNUSUAL chance of being > 0,
		 * a depth^2/DUN_UNUSUAL^2 chance of being > 1, up to MAX_RARITY. */
		i = 0;
		rarity = 0;
		while (i == rarity && i < dun->profile->max_rarity) {
			if (randint0(dun_unusual) < c->depth) rarity++;
			i++;
		}

		/* Once we have a key and a rarity, we iterate through out list of
		 * room profiles looking for a match (whose cutoff > key and whose
		 * rarity > this rarity). We try building the room, and if it works
		 * then we are done with this iteration. We keep going until we find
		 * a room that we can build successfully or we exhaust the profiles. */
		i = 0;
		for (i = 0; i < dun->profile->n_room_profiles; i++) {
			struct room_profile profile = dun->profile->room_profiles[i];
			if (profile.rarity > rarity) continue;
			if (profile.cutoff <= key) continue;
			
			if (room_build(c, by, bx, profile)) {
				built++;
				break;
			}
		}
	}

	/* Generate permanent walls around the edge of the dungeon */
	draw_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);

	/* Hack -- Scramble the room order */
	for (i = 0; i < dun->cent_n; i++) {
		int pick1 = randint0(dun->cent_n);
		int pick2 = randint0(dun->cent_n);
		y1 = dun->cent[pick1].y;
		x1 = dun->cent[pick1].x;
		dun->cent[pick1].y = dun->cent[pick2].y;
		dun->cent[pick1].x = dun->cent[pick2].x;
		dun->cent[pick2].y = y1;
		dun->cent[pick2].x = x1;
	}

	/* Start with no tunnel doors */
	dun->door_n = 0;

	/* Hack -- connect the first room to the last room */
	y = dun->cent[dun->cent_n-1].y;
	x = dun->cent[dun->cent_n-1].x;

	/* Connect all the rooms together */
	for (i = 0; i < dun->cent_n; i++) {
		/* Connect the room to the previous room */
		build_tunnel(c, dun->cent[i].y, dun->cent[i].x, y, x);

		/* Remember the "previous" room */
		y = dun->cent[i].y;
		x = dun->cent[i].x;
	}

	/* Place intersection doors */
	for (i = 0; i < dun->door_n; i++) {
		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try placing doors */
		try_door(c, y, x - 1);
		try_door(c, y, x + 1);
		try_door(c, y - 1, x);
		try_door(c, y + 1, x);
	}

	ensure_connectedness(c);

	/* Add some magma streamers */
	for (i = 0; i < dun->profile->str.mag; i++)
		build_streamer(c, FEAT_MAGMA, dun->profile->str.mc);

	/* Add some quartz streamers */
	for (i = 0; i < dun->profile->str.qua; i++)
		build_streamer(c, FEAT_QUARTZ, dun->profile->str.qc);

	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(3, 4), 3);

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2), 3);

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth, 0);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint1(8) + k;

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT, Rand_normal(DUN_AMT_ROOM, 3),
		c->depth, ORIGIN_FLOOR);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(DUN_AMT_ITEM, 3),
		c->depth, ORIGIN_FLOOR);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(DUN_AMT_GOLD, 3),
		c->depth, ORIGIN_FLOOR);

	return TRUE;
}


/*
 * ------------------ LABYRINTH ----------------
 */

/**
 * Used to convert (x, y) into an array index (i) in labyrinth_gen().
 */
static int lab_toi(int y, int x, int w) {
	return y * w + x;
}

/**
 * Used to convert an array index (i) into (x, y) in labyrinth_gen().
 */
static void lab_toyx(int i, int w, int *y, int *x) {
	*y = i / w;
	*x = i % w;
}

/**
 * Given an adjoining wall (a wall which separates two labyrinth cells)
 * set a and b to point to the cell indices which are separated. Used by
 * labyrinth_gen().
 */
static void lab_get_adjoin(int i, int w, int *a, int *b) {
	int y, x;
	lab_toyx(i, w, &y, &x);
	if (x % 2 == 0) {
		*a = lab_toi(y - 1, x, w);
		*b = lab_toi(y + 1, x, w);
	} else {
		*a = lab_toi(y, x - 1, w);
		*b = lab_toi(y, x + 1, w);
	}
}

/**
 * Return whether (x, y) is in a tunnel.
 *
 * For our purposes a tunnel is a horizontal or vertical path, not an
 * intersection. Thus, we want the squares on either side to walls in one
 * case (e.g. up/down) and open in the other case (e.g. left/right). We don't
 * want a square that represents an intersection point.
 *
 * The high-level idea is that these are squares which can't be avoided (by
 * walking diagonally around them).
 */
static bool lab_is_tunnel(struct cave *c, int y, int x) {
	bool west = cave_isopen(c, y, x - 1);
	bool east = cave_isopen(c, y, x + 1);
	bool north = cave_isopen(c, y - 1, x);
	bool south = cave_isopen(c, y + 1, x);

	return north == south && west == east && north != west;
}


/**
 * Build a labyrinth level.
 *
 * Note that if the function returns FALSE, a level wasn't generated.
 * Labyrinths use the dungeon level's number to determine whether to generate
 * themselves (which means certain level numbers are more likely to generate
 * labyrinths than others).
 */
static bool labyrinth_gen(struct cave *c, struct player *p) {
	int i, j, k, y, x;

	/* Size of the actual labyrinth part must be odd. */
	/* NOTE: these are not the actual dungeon size, but rather the size of the
	 * area we're genearting a labyrinth in (which doesn't count theh enclosing
	 * outer walls. */
	int h = 15 + randint0(c->depth / 10) * 2;
	int w = 51 + randint0(c->depth / 10) * 2;

	/* This is the number of squares in the labyrinth */
	int n = h * w;

	/* NOTE: 'sets' and 'walls' are too large... we only need to use about
	 * 1/4 as much memory. However, in that case, the addressing math becomes
	 * a lot more complicated, so let's just stick with this because it's
	 * easier to read. */

	/* 'sets' tracks connectedness; if sets[i] == sets[j] then cells i and j
	 * are connected to each other in the maze. */
	int *sets;

	/* 'walls' is a list of wall coordinates which we will randomize */
	int *walls;

	/* Most labyrinths are lit */
	bool lit = randint0(c->depth) < 25 || randint0(2) < 1;

	/* Many labyrinths are known */
	bool known = lit && randint0(c->depth) < 25;

	/* Most labyrinths have soft (diggable) walls */
	bool soft = randint0(c->depth) < 35 || randint0(3) < 2;

	/* There's a base 1 in 100 to accept the labyrinth */
	int chance = 1;

	/* If we're too shallow then don't do it */
	if (c->depth < 13) return FALSE;

	/* Don't try this on quest levels, kids... */
	if (is_quest(c->depth)) return FALSE;

	/* Certain numbers increase the chance of having a labyrinth */
	if (c->depth % 3 == 0) chance += 1;
	if (c->depth % 5 == 0) chance += 1;
	if (c->depth % 7 == 0) chance += 1;
	if (c->depth % 11 == 0) chance += 1;
	if (c->depth % 13 == 0) chance += 1;

	/* Only generate the level if we pass a check */
	/* NOTE: This test gets performed after we pass the test to use the
	 * labyrinth cave profile. */
	if (randint0(100) >= chance) return FALSE;

	/* allocate our arrays */
	sets = C_ZNEW(n, int);
	walls = C_ZNEW(n, int);

	/* This is the dungeon size, which does include the enclosing walls */
	set_cave_dimensions(c, h + 2, w + 2);

	/* Fill whole level with perma-rock */
	fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);

	/* Fill the labyrinth area with rock */
	fill_rectangle(c, 1, 1, h, w, soft ? FEAT_WALL_SOLID : FEAT_PERM_SOLID);

	/* Initialize each wall. */
	for (i = 0; i < n; i++) {
		walls[i] = i;
		sets[i] = -1;
	}

	/* Cut out a grid of 1x1 rooms which we will call "cells" */
	for (y = 0; y < h; y += 2) {
		for (x = 0; x < w; x += 2) {
			int k = lab_toi(y, x, w);
			sets[k] = k;
			cave_set_feat(c, y + 1, x + 1, FEAT_FLOOR);
			if (lit) c->info[y + 1][x + 1] |= CAVE_GLOW;
		}
	}

	/* Shuffle the walls, using Knuth's shuffle. */
	shuffle(walls, n);

	/* For each adjoining wall, look at the cells it divides. If they aren't
	 * in the same set, remove the wall and join their sets.
	 *
	 * This is a randomized version of Kruskal's algorithm. */
	for (i = 0; i < n; i++) {
		int a, b, x, y;

		j = walls[i];

		/* If this cell isn't an adjoining wall, skip it */
		lab_toyx(j, w, &y, &x);
		if ((x < 1 && y < 1) || (x > w - 2 && y > h - 2)) continue;
		if (x % 2 == y % 2) continue;

		/* Figure out which cells are separated by this wall */
		lab_get_adjoin(j, w, &a, &b);

		/* If the cells aren't connected, kill the wall and join the sets */
		if (sets[a] != sets[b]) {
			int sa = sets[a];
			int sb = sets[b];
			cave_set_feat(c, y + 1, x + 1, FEAT_FLOOR);
			if (lit) c->info[y + 1][x + 1] |= CAVE_GLOW;

			for (k = 0; k < n; k++) {
				if (sets[k] == sb) sets[k] = sa;
			}
		}
	}

	/* Determine the character location */
	new_player_spot(c, p);

	/* The level should have exactly one down and one up staircase */
	if (OPT(birth_no_stairs)) {
		/* new_player_spot() won't have created stairs, so make both*/
		alloc_stairs(c, FEAT_MORE, 1, 3);
		alloc_stairs(c, FEAT_LESS, 1, 3);
	} else if (p->create_down_stair) {
		/* new_player_spot() will have created down, so only create up */
		alloc_stairs(c, FEAT_LESS, 1, 3);
	} else {
		/* new_player_spot() will have created up, so only create down */
		alloc_stairs(c, FEAT_MORE, 1, 3);
	}

	/* Generate a door for every 100 squares in the labyrinth */
	for (i = n / 100; i > 0; i--) {
		/* Try 10 times to find a useful place for a door, then place it */
		for (j = 0; j < 10; j++) {
			find_empty(c, &y, &x);
			if (lab_is_tunnel(c, y, x)) break;

		}

		place_closed_door(c, y, x);
	}

	/* General some rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Scale number of monsters items by labyrinth size */
	k = (3 * k * (h * w)) / (DUNGEON_HGT * DUNGEON_WID);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth, 0);

	/* Put some monsters in the dungeon */
	for (i = MIN_M_ALLOC_LEVEL + randint1(8) + k; i > 0; i--)
		alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(6, 3), c->depth,
		ORIGIN_LABYRINTH);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(6, 3), c->depth,
		ORIGIN_LABYRINTH);
	alloc_objects(c, SET_BOTH, TYP_GOOD, randint0(2), c->depth,
		ORIGIN_LABYRINTH);

	/* Unlit labyrinths will have some good items */
	if (!lit)
		alloc_objects(c, SET_BOTH, TYP_GOOD, Rand_normal(3, 2), c->depth,
			ORIGIN_LABYRINTH);

	/* Hard (non-diggable) labyrinths will have some great items */
	if (!soft)
		alloc_objects(c, SET_BOTH, TYP_GREAT, Rand_normal(2, 1), c->depth,
			ORIGIN_LABYRINTH);

	/* If we want the players to see the maze layout, do that now */
	if (known) wiz_light();

	/* Deallocate our lists */
	FREE(sets);
	FREE(walls);

	return TRUE;
}


/*
 * ---------------- CAVERNS ----------------------
 */

/**
 * Initialize the dungeon array, with a random percentage of squares open.
 */
static void init_cavern(struct cave *c, struct player *p, int density) {
	int h = c->height;
	int w = c->width;
	int size = h * w;
	
	int count = (size * density) / 100;
	
	/* Fill the edges with perma-rock, and rest with rock */
	draw_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);
	fill_rectangle(c, 1, 1, DUNGEON_HGT - 2, DUNGEON_WID - 2, FEAT_WALL_SOLID);
	
	while (count > 0) {
		int y = randint1(h - 2);
		int x = randint1(w - 2);
		if (cave_isrock(c, y, x)) {
			cave_set_feat(c, y, x, FEAT_FLOOR);
			count--;
		}
	}
}

/**
 * Return the number of walls (0-8) adjacent to this square.
 */
static int count_adj_walls(struct cave *c, int y, int x) {
	int yd, xd;
	int count = 0;

	for (yd = -1; yd <= 1; yd++) {
		for (xd = -1; xd <= 1; xd++) {
			if (yd == 0 && xd == 0) continue;
			if (cave_isfloor(c, y + yd, x + xd)) continue;
			count++;
		}
	}

	return count;
}

/**
 * Run a single pass of the cellular automata rules (4,5) on the dungeon.
 */
static void mutate_cavern(struct cave *c) {
	int y, x;
	int h = c->height;
	int w = c->width;

	int *temp = C_ZNEW(h * w, int);

	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			int count = count_adj_walls(c, y, x);
			if (count > 5)
				temp[y * w + x] = FEAT_WALL_SOLID;
			else if (count < 4)
				temp[y * w + x] = FEAT_FLOOR;
			else
				temp[y * w + x] = cave->feat[y][x];
		}
	}

	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < w - 1; x++) {
			cave_set_feat(c, y, x, temp[y * w + x]);
		}
	}

	FREE(temp);
}

/**
 * Fill an int[] with a single value.
 */
static void array_filler(int data[], int value, int size) {
	int i;
	for (i = 0; i < size; i++) data[i] = value;
}

/**
 * Determine if we need to worry about coloring a point, or can ignore it.
 */
static int ignore_point(struct cave *c, int colors[], int y, int x) {
	int h = c->height;
	int w = c->width;
	int n = lab_toi(y, x, w);

	if (y < 0 || x < 0 || y >= h || x >= w) return TRUE;
	if (colors[n]) return TRUE;
	//if (cave_isvault(c, y, x)) return TRUE;
	if (cave_isvault(c, y, x)) return FALSE;
	if (cave_ispassable(c, y, x)) return FALSE;
	if (cave_isdoor(c, y, x)) return FALSE;
	return TRUE;
}

static int xds[] = {0, 0, 1, -1, -1, -1, 1, 1};
static int yds[] = {1, -1, 0, 0, -1, 1, -1, 1};

#if 0 /* XXX d_m - is this meant to be in use? */
static void glow_point(struct cave *c, int y, int x) {
	int i, j;
	for (i = -1; i <= -1; i++)
		for (j = -1; j <= -1; j++)
			c->info[y + i][x + j] |= CAVE_GLOW;
}
#endif

/**
 * Color a particular point, and all adjacent points.
 */
static void build_color_point(struct cave *c, int colors[], int counts[], int y, int x, int color, bool diagonal) {
	int h = c->height;
	int w = c->width;
	int size = h * w;
	struct queue *queue = q_new(size);

	int dslimit = diagonal ? 8 : 4;

	int *added = C_ZNEW(size, int);
	array_filler(added, 0, size);

	q_push_int(queue, lab_toi(y, x, w));

	counts[color] = 0;

	while (q_len(queue) > 0) {
		int i, y2, x2;
		int n2 = q_pop_int(queue);

		lab_toyx(n2, w, &y2, &x2);

		if (ignore_point(cave, colors, y2, x2)) continue;

		colors[n2] = color;
		counts[color]++;

		/*if (lit) glow_point(c, y2, x2);*/

		for (i = 0; i < dslimit; i++) {
			int y3 = y2 + yds[i];
			int x3 = x2 + xds[i];
			int n3 = lab_toi(y3, x3, w);
			if (ignore_point(cave, colors, y3, x3)) continue;
			if (added[n3]) continue;

			q_push_int(queue, n3);
			added[n3] = 1;
		}
	}

	FREE(added);
	q_free(queue);
}

/**
 * Create a color for each "NESW contiguous" region of the dungeon.
 */
static void build_colors(struct cave *c, int colors[], int counts[], bool diagonal) {
	int y, x;
	int h = c->height;
	int w = c->width;
	int color = 1;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (ignore_point(cave, colors, y, x)) continue;
			build_color_point(cave, colors, counts, y, x, color, diagonal);
			color++;
		}
	}
}

/**
 * Find and delete all small (<9 square) open regions.
 */
static void clear_small_regions(struct cave *c, int colors[], int counts[]) {
	int i;
	int h = c->height;
	int w = c->width;
	int size = h * w;

	int *deleted = C_ZNEW(size, int);
	array_filler(deleted, 0, size);

	for (i = 0; i < size; i++) {
		if (counts[i] < 9) {
			deleted[i] = 1;
			counts[i] = 0;
		}
	}

	for (i = 0; i < size; i++) {
		if (deleted[colors[i]]) {
			int x, y;
			lab_toyx(i, w, &y, &x);
			colors[i] = 0;
			cave_set_feat(c, y, x, FEAT_WALL_SOLID);
		}
	}
	FREE(deleted);
}

/**
 * Return the number of colors which have active cells.
 */
static int count_colors(int counts[], int size) {
	int i;
	int num = 0;
	for (i = 0; i < size; i++) if (counts[i] > 0) num++;
	return num;
}

/**
 * Return the first color which has one or more active cells.
 */
static int first_color(int counts[], int size) {
	int i;
	for (i = 0; i < size; i++) if (counts[i] > 0) return i;
	return -1;
}

/**
 * Find all cells of 'fromcolor' and repaint them to 'tocolor'.
 */
static void fix_colors(int colors[], int counts[], int from, int to, int size) {
	int i;
	for (i = 0; i < size; i++) if (colors[i] == from) colors[i] = to;
	counts[to] += counts[from];
	counts[from] = 0;
}

/**
 * Create a tunnel connecting a region to one of its nearest neighbors.
 */
static void join_region(struct cave *c, int colors[], int counts[], int color) {
	int i;
	int h = c->height;
	int w = c->width;
	int size = h * w;

	struct queue *queue = q_new(size);

	int *previous = C_ZNEW(size, int);
	array_filler(previous, -1, size);

	for (i = 0; i < size; i++) {
		if (colors[i] == color) {
			q_push_int(queue, i);
			previous[i] = i;
		}
	}

	while (q_len(queue) > 0) {
		int n = q_pop_int(queue);

		int color2 = colors[n];
		if (color2 && color2 != color) {
			while (colors[n] != color) {
				int x, y;
				lab_toyx(n, w, &y, &x);
				colors[n] = color;
				if (!cave_isperm(c, y, x) && !cave_isvault(c, y, x)) {
					cave_set_feat(c, y, x, FEAT_FLOOR);
				}
				n = previous[n];
			}
			fix_colors(colors, counts, color2, color, size);
			break;
		}

		for (i = 0; i < 4; i++) {
			int y, x, y2, x2, n2;
			lab_toyx(n, w, &y, &x);

			y2 = y + yds[i];
			x2 = x + xds[i];

			/* make sure we stay inside the boundaries */
			if (y2 < 0 || y2 >= h) continue;
			if (x2 < 0 || x2 >= w) continue;

			/* permanent walls and vault squares should not be handled */
			//if (cave_isperm(c, y2, x2)) continue;
			//if (cave_isvault(c, y2, x2)) continue;

			n2 = lab_toi(y2, x2, w);
			if (previous[n2] >= 0) continue;

			q_push_int(queue, n2);
			previous[n2] = n;
		}
	}

	q_free(queue);
	FREE(previous);
}

/**
 * Start connecting regions, stopping when the cave is entirely connected.
 */
static void join_regions(struct cave *c, int colors[], int counts[]) {
	int h = c->height;
	int w = c->width;
	int size = h * w;
	int num = count_colors(counts, size);

	while (num > 1) {
		int color = first_color(counts, size);
		join_region(c, colors, counts, color);
		num--;
	}
}


static int open_count(struct cave *c) {
	int x, y;
	int h = c->height;
	int w = c->width;
	int num = 0;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			if (cave_ispassable(c, y, x)) num++;
	return num;
}


void ensure_connectedness(struct cave *c) {
	int size = c->height * c->width;
	int *colors = C_ZNEW(size, int);
	int *counts = C_ZNEW(size, int);

	build_colors(c, colors, counts, TRUE);
	join_regions(c, colors, counts);

	FREE(colors);
	FREE(counts);
}


#define MAX_CAVERN_TRIES 10
/**
 * The program's main function.
 */
bool cavern_gen(struct cave *c, struct player *p) {
	int i, k, openc;

	int h = rand_range(DUNGEON_HGT / 2, (DUNGEON_HGT * 3) / 4);
	int w = rand_range(DUNGEON_WID / 2, (DUNGEON_WID * 3) / 4);
	int size = h * w;
	int limit = size / 13;

	int density = rand_range(25, 30);
	int times = rand_range(3, 6);

	int *colors = C_ZNEW(size, int);
	int *counts = C_ZNEW(size, int);

	int tries = 0;

	bool ok = TRUE;

	set_cave_dimensions(c, h, w);
	ROOM_LOG("cavern h=%d w=%d size=%d density=%d times=%d", h, w, size, density, times);

	if (c->depth < 15) {
		/* If we're too shallow then don't do it */
		ok = FALSE;

	} else {
		/* Start trying to build caverns */
		array_filler(colors, 0, size);
		array_filler(counts, 0, size);
	
		for (tries = 0; tries < MAX_CAVERN_TRIES; tries++) {
			/* Build a random cavern and mutate it a number of times */
			init_cavern(c, p, density);
			for (i = 0; i < times; i++) mutate_cavern(c);
	
			/* If there are enough open squares then we're done */
			openc = open_count(c);
			if (openc >= limit) {
				ROOM_LOG("cavern ok (%d vs %d)", openc, limit);
				break;
			}
			ROOM_LOG("cavern failed--try again (%d vs %d)", openc, limit);
		}

		/* If we couldn't make a big enough cavern then fail */
		if (tries == MAX_CAVERN_TRIES) ok = FALSE;
	}

	if (ok) {
		build_colors(c, colors, counts, FALSE);
		clear_small_regions(c, colors, counts);
		join_regions(c, colors, counts);
	
		/* Place 2-3 down stairs near some walls */
		alloc_stairs(c, FEAT_MORE, rand_range(1, 3), 3);
	
		/* Place 1-2 up stairs near some walls */
		alloc_stairs(c, FEAT_LESS, rand_range(1, 2), 3);
	
		/* General some rubble, traps and monsters */
		k = MAX(MIN(c->depth / 3, 10), 2);
	
		/* Scale number of monsters items by cavern size */
		k = (2 * k * (h *  w)) / (DUNGEON_HGT * DUNGEON_WID);
	
		/* Put some rubble in corridors */
		alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth, 0);
	
		/* Place some traps in the dungeon */
		alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth, 0);
	
		/* Determine the character location */
		new_player_spot(c, p);
	
		/* Put some monsters in the dungeon */
		for (i = MIN_M_ALLOC_LEVEL + randint1(8) + k; i > 0; i--)
			alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);
	
		/* Put some objects/gold in the dungeon */
		alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(6, 3), c->depth,
			ORIGIN_CAVERN);
		alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(6, 3), c->depth,
			ORIGIN_CAVERN);
		alloc_objects(c, SET_BOTH, TYP_GOOD, randint0(2), c->depth,
			ORIGIN_CAVERN);
	}

	FREE(colors);
	FREE(counts);

	return ok;
}

/*
 * ------------------ TOWN ----------------
 */

/**
 * Builds a store at a given pseudo-location
 *
 * Currently, there is a main street horizontally through the middle of town,
 * and all the shops face it (e.g. the shops on the north side face south).
 */
static void build_store(struct cave *c, int n, int yy, int xx) {
	/* Find the "center" of the store */
	int y0 = yy * 9 + 6;
	int x0 = xx * 14 + 12;

	/* Determine the store boundaries */
	int y1 = y0 - randint1((yy == 0) ? 3 : 2);
	int y2 = y0 + randint1((yy == 1) ? 3 : 2);
	int x1 = x0 - randint1(5);
	int x2 = x0 + randint1(5);

	/* Determine door location, based on which side of the street we're on */
	int dy = (yy == 0) ? y2 : y1;
	int dx = rand_range(x1, x2);

	/* Build an invulnerable rectangular building */
	fill_rectangle(c, y1, x1, y2, x2, FEAT_PERM_EXTRA);

	/* Clear previous contents, add a store door */
	cave_set_feat(c, dy, dx, FEAT_SHOP_HEAD + n);
}


/**
 * Generate the "consistent" town features, and place the player
 *
 * HACK: We seed the simple RNG, so we always get the same town layout,
 * including the size and shape of the buildings, the locations of the
 * doorways, and the location of the stairs. This means that if any of the
 * functions used to build the town change the way they use the RNG, the
 * town layout will be generated differently.
 *
 * XXX: Remove this gross hack when this piece of code is fully reentrant -
 * i.e., when all we need to do is swing a pointer to change caves, we just
 * need to generate the town once (we will also need to save/load the town).
 */
static void town_gen_hack(struct cave *c, struct player *p) {
	int y, x, n, k;
	int rooms[MAX_STORES];

	int n_rows = 2;
	int n_cols = (MAX_STORES + 1) / n_rows;

	/* Switch to the "simple" RNG and use our original town seed */
	Rand_quick = TRUE;
	Rand_value = seed_town;

	/* Prepare an Array of "remaining stores", and count them */
	for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

	/* Place rows of stores */
	for (y = 0; y < n_rows; y++) {
		for (x = 0; x < n_cols; x++) {
			if (n < 1) break;

			/* Pick a remaining store */
			k = randint0(n);

			/* Build that store at the proper location */
			build_store(c, rooms[k], y, x);

			/* Shift the stores down, remove one store */
			rooms[k] = rooms[--n];
		}
	}

	/* Place the stairs */
	find_empty_range(c, &y, 3, TOWN_HGT - 3, &x, 3, TOWN_WID - 3);

	/* Clear previous contents, add down stairs */
	cave_set_feat(c, y, x, FEAT_MORE);

	/* Place the player */
	player_place(c, p, y, x);

	/* go back to using the "complex" RNG */
	Rand_quick = FALSE;
}


/**
 * Town logic flow for generation of new town.
 *
 * We start with a fully wiped cave of normal floors. This function does NOT do
 * anything about the owners of the stores, nor the contents thereof. It only
 * handles the physical layout.
 */
static bool town_gen(struct cave *c, struct player *p) {
	int i;
	bool daytime = turn % (10 * TOWN_DAWN) < (10 * TOWN_DUSK);
	int residents = daytime ? MIN_M_ALLOC_TD : MIN_M_ALLOC_TN;

	assert(c);

	set_cave_dimensions(c, TOWN_HGT, TOWN_WID);

	/* NOTE: We can't use c->height and c->width here because then there'll be
	 * a bunch of empty space in the level that monsters might spawn in (or
	 * teleport might take you to, or whatever).
	 *
	 * TODO: fix this to use c->height and c->width when all the 'choose
	 * random location' things honor them.
	 */

	/* Start with solid walls, and then create some floor in the middle */
	fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);
	fill_rectangle(c, 1, 1, c->height -2, c->width - 2, FEAT_FLOOR);

	/* Build stuff */
	town_gen_hack(c, p);

	/* Apply illumination */
	cave_illuminate(c, daytime);

	/* Make some residents */
	for (i = 0; i < residents; i++)
		alloc_monster(c, loc(p->px, p->py), 3, TRUE, c->depth);

	return TRUE;
}


/**
 * Clear the dungeon, ready for generation to begin.
 */
static void cave_clear(struct cave *c, struct player *p) {
	int x, y;

	wipe_o_list(c);
	wipe_mon_list(c, p);

	/* Clear flags and flow information. */
	for (y = 0; y < DUNGEON_HGT; y++) {
		for (x = 0; x < DUNGEON_WID; x++) {
			/* Erase features */
			c->feat[y][x] = 0;

			/* Erase flags */
			c->info[y][x] = 0;
			c->info2[y][x] = 0;

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

	if (x > 6000) return 20;
	if (x > 3500) return 30;
	if (x > 2000) return 40;
	if (x > 1000) return 50;
	if (x > 500) return 60;
	if (x > 300) return 70;
	if (x > 200) return 80;
	if (x > 100) return 90;
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

static void clear_dun_data(struct dun_data *d) {
	int bx, by;
	for (by = 0; by < MAX_ROOMS_ROW; by++) {
		for (bx = 0; bx < MAX_ROOMS_COL; bx++) {
			d->room_map[by][bx] = FALSE;
		}
	}
}

/**
 * Generate a random level.
 *
 * Confusingly, this function also generate the town level (level 0).
 */
void cave_generate(struct cave *c, struct player *p) {
	const char *error = "no generation";
	int tries = 0;

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
		clear_dun_data(dun);

		if (p->depth == 0) {
			dun->profile = &town_profile;
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
				place_monster_aux(c, y, x, i, TRUE, TRUE, ORIGIN_DROP);
			}
		}

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

	/* The dungeon is ready */
	character_dungeon = TRUE;

	c->created_at = turn;
}
