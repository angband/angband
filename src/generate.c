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
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "trap.h"
#include "z-type.h"

static struct dun_data *dun;

static bool town_gen(struct cave *c, struct player *p);

static bool cave_gen(struct cave *c, struct player *p);
static bool labyrinth_gen(struct cave *c, struct player *p);

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

static void alloc_objects(struct cave *c, int set, int typ, int num, int depth);
static bool alloc_object(struct cave *c, int set, int typ, int depth);

#define ROOM_DEBUG(...) if (1) msg(__VA_ARGS__);

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
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * BUG: Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * BUG: Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening.  But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite.  XXX XXX XXX
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

	/* room_profiles */
	NULL
};


/* name function width height min-depth crowded? rarity %cutoff */
static struct room_profile default_rooms[] = {
	/* greater vaults only have rarity 1 but they have other checks */
	{"greater vault", build_greater_vault, 4, 6, 10, FALSE, 1, 100},

	/* very rare rooms (rarity=2) */
	{"medium vault", build_medium_vault, 2, 2, 5, FALSE, 2, 10},
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
#define NUM_CAVE_PROFILES 2
static struct cave_profile cave_profiles[NUM_CAVE_PROFILES] = {
	{
		/* name builder dun_rooms dun_unusual max_rarity n_room_profiles */
		"cave-default", cave_gen, 50, 200, 2, N_ELEMENTS(default_rooms),

		/* name rnd chg con pen jct */
		{"tunnel-default", 10, 30, 15, 25, 90},

		/* name den rng mag mc qua qc */
		{"streamer-default", 5, 2, 3, 90, 2, 40},

		/* room_profiles */
		default_rooms
	},
	{
		"cave-labyrinth", labyrinth_gen, 0, 200, 0, 0,

		/* name rnd chg con pen jct */
		{"tunnel-default", 10, 30, 15, 25, 90},

		/* name den rng mag mc qua qc */
		{"streamer-default", 5, 2, 3, 90, 2, 40},

		NULL
	}
};


/* FAILSAFE defines the maximum number of iterations that the find_* functions
 * will run before dying.
 *
 * TODO: Fix the find_* functions so they are guaranteed to test all legal open
 * squares in a reasonable amount of time.
 */
#define FAILSAFE 10000000


/**
 * Locate an empty square for 0 <= y < ymax, 0 <= x < xmax.
 *
 * This function will crash if it can't find an empty square after 100M
 * attempts. If there is a chance that there isn't an empty square available
 * you should do something else.
 *
 * TODO: Rewrite this function so that it tries every cave location exactly
 * once in a random order and only crashes if there are no empty squares.
 */
static void find_empty(struct cave *c, int *y, int ymax, int *x, int xmax) {
	int tries = 0;
	while (tries < FAILSAFE) {
		*y = randint0(ymax);
		*x = randint0(xmax);
		if (cave_isempty(c, *y, *x)) return;
		tries++;
	}
	quit_fmt("find_empty <%d, %d> failed", ymax, xmax);
}


/**
 * Locate an empty square for y1 <= y <= y2, x1 <= x <= x2.
 *
 * This function has similar behavior as find_empty() and the same caveats.
 */
static void find_empty_range(struct cave *c, int *y, int y1, int y2, int *x, int x1, int x2) {
	int tries = 0;
	while (tries < FAILSAFE) {
		*y = rand_range(y1, y2);
		*x = rand_range(x1, x2);
		if (cave_isempty(c, *y, *x)) return;
		tries++;
	}
	quit_fmt("find_empty_range <%d, %d, %d, %d> failed", y1, y2, x1, x2);
}


/**
 * Locate a grid nearby (y0, x0) within +/- yd, xd.
 */
static void find_nearby_grid(struct cave *c, int *y, int y0, int yd, int *x, int x0, int xd) {
	int tries = 0;
	while (tries < FAILSAFE) {
		*y = rand_spread(y0, yd);
		*x = rand_spread(x0, xd);
		if (cave_in_bounds(c, *y, *x)) return;
		tries++;
	}
	quit_fmt("find_nearby_grid <%d, %d, %d, %d> failed", y0, yd, x0, xd);
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


/**
 * Place the player at a random starting location.
 */
static void new_player_spot(struct cave *c, struct player *p) {
	int y, x;
	int tries = 0;

	assert(c);

	/* Find empty squares that aren't in a vault to start on */
	do {
		find_empty_range(c, &y, 1, c->height - 2, &x, 1, c->width - 2);
		tries++;
	} while (c->info[y][x] & CAVE_ICKY && tries < 100);

	if (tries == 100) quit_fmt("couldn't place the player");

	/* Create stairs if allowed and necessary */
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

	assert(in_bounds_fully(y, x));

	if (c->feat[y+1][x] >= FEAT_WALL_EXTRA) k++;
	if (c->feat[y-1][x] >= FEAT_WALL_EXTRA) k++;
	if (c->feat[y][x+1] >= FEAT_WALL_EXTRA) k++;
	if (c->feat[y][x-1] >= FEAT_WALL_EXTRA) k++;

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
void place_object(struct cave *c, int y, int x, int level, bool good, bool great) {
	object_type otype;

	assert(cave_in_bounds(c, y, x));

	if (!cave_canputitem(c, y, x)) return;

	object_wipe(&otype);
	if (make_object(c, &otype, level, good, great)) {
		otype.origin = ORIGIN_FLOOR;
		otype.origin_depth = c->depth;

		/* Give it to the floor */
		/* XXX Should this be done in floor_carry? */
		if (!floor_carry(c, y, x, &otype) && otype.name1)
			a_info[otype.name1].created = FALSE;
	}
}


/**
 * Place a random amount of gold at (x, y).
 */
void place_gold(struct cave *c, int y, int x, int level) {
	object_type *i_ptr;
	object_type object_type_body;

	assert(cave_in_bounds(c, y, x));

	if (!cave_canputitem(c, y, x)) return;

	i_ptr = &object_type_body;
	object_wipe(i_ptr);
	make_gold(i_ptr, level, SV_GOLD_ANY);
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
			for (j = 0; !done && j <= 300; j++) {
				find_empty(c, &y, c->height, &x, c->width);

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
static void alloc_objects(struct cave *c, int set, int typ, int num, int depth) {
	int k;
	for (k = 0; k < num; k++)
		alloc_object(c, set, typ, depth);
}


/**
 * Allocates a single random object in the dungeon.
 *
 * 'set' controls where the object is placed (corridor, room, either).
 * 'typ' conrols the kind of object (rubble, trap, gold, item).
 */
static bool alloc_object(struct cave *c, int set, int typ, int depth) {
	int x, y;
	int tries = 0;
	bool room;

	/* Pick a "legal" spot */
	while (tries < 2000) {
		tries++;

		find_empty(c, &y, c->height, &x, c->width);

		/* See if our spot is in a room, and whether we care */
		room = (c->info[y][x] & CAVE_ROOM) ? TRUE : FALSE;
		if (set & SET_CORR && room) continue;
		if (set & SET_ROOM && !room) continue;

		break;
	}

	if (tries == 2000) return FALSE;

	/* Place something */
	switch (typ) {
		case TYP_RUBBLE: place_rubble(c, y, x); break;
		case TYP_TRAP: place_trap(c, y, x); break;
		case TYP_GOLD: place_gold(c, y, x, depth); break;
		case TYP_OBJECT: place_object(c, y, x, depth, FALSE, FALSE); break;
	}
	return TRUE;
}


/**
 * Add visible treasure to a mineral square.
 */
static void upgrade_mineral(struct cave *c, int y, int x) {
	switch (c->feat[y][x]) {
		case FEAT_MAGMA: cave_set_feat(c, y, x, FEAT_MAGMA_K); break;
		case FEAT_QUARTZ: cave_set_feat(c, y, x, FEAT_QUARTZ_K); break;
	}
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
			switch (c->feat[ty][tx]) {
				case FEAT_WALL_EXTRA:
				case FEAT_WALL_INNER:
				case FEAT_WALL_OUTER:
				case FEAT_WALL_SOLID: {
					/* Clear previous contents, add proper vein type */
					cave_set_feat(c, ty, tx, feat);

					/* Hack -- Add some (known) treasure */
					if (one_in_(chance)) upgrade_mineral(c, ty, tx);
				}
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
				place_object(c, j, k, depth, FALSE, FALSE);
			else
				place_gold(c, j, k, depth);

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
			place_monster(c, y, x, depth, TRUE, TRUE);

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


static void fill_circle(struct cave *c, int y0, int x0, int radius, int feat, int info) {
	int i;
	int r2 = radius * radius;
	for(i = -radius; i <= radius; i++) {
		double j = sqrt(r2 - (i * i));
		int k = round(j);
		fill_xrange(c, y0 + i, x0 - k, x0 + k, feat, info);
		fill_yrange(c, x0 + i, y0 - k, y0 + k, feat, info);
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
	fill_circle(c, y0, x0, radius + 1, FEAT_WALL_OUTER, 0);
	fill_circle(c, y0, x0, radius, FEAT_FLOOR, info);

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
			place_object(c, y0, x0, c->depth, FALSE, FALSE);

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
				place_object(c, y0, x0, c->depth, FALSE, FALSE);
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
				if (one_in_(3)) place_object(c, y0, x0 - 2, c->depth, FALSE, FALSE);
				if (one_in_(3)) place_object(c, y0, x0 + 2, c->depth, FALSE, FALSE);
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


/*
 * The following functions are used to determine if the given monster
 * is appropriate for inclusion in a monster nest or monster pit or
 * the given type.
 *
 * None of the pits/nests are allowed to include "unique" monsters.
 *
 * The old method used monster "names", which was bad, but the new
 * method uses monster race characters, which is also bad.  XXX XXX XXX
 */


/**
 * For a given flag, this function returns true if the given r_idx represents
 * a non-unique monster with that flag.
 */
static bool vault_aux_flag(int r_idx, bitflag flag) {
	monster_race *r_ptr = &r_info[r_idx];
	if (rf_has(r_ptr->flags, RF_UNIQUE))
		return FALSE;
	else if (!rf_has(r_ptr->flags, flag))
		return FALSE;
	else
		return TRUE;
}


/**
 * For a given string, this function returns true if the given r_idx represents
 * a non-unique monster whose symbol is present in the string.
 */
static bool vault_aux_str(int r_idx, const char *s) {
	monster_race *r_ptr = &r_info[r_idx];
	if (rf_has(r_ptr->flags, RF_UNIQUE))
		return FALSE;
	else if (!strchr(s, r_ptr->d_char))
		return FALSE;
	else
		return TRUE;
}


/**
 * Helper function for "monster nest (jelly)"
 */
static bool vault_aux_jelly(int r_idx) {
	return vault_aux_str(r_idx, "ijm,");
}


/**
 * Helper function for "monster nest (animal)"
 */
static bool vault_aux_animal(int r_idx) {
	return vault_aux_flag(r_idx, RF_ANIMAL);
}


/**
 * Helper function for "monster nest (undead)"
 */
static bool vault_aux_undead(int r_idx) {
	return vault_aux_flag(r_idx, RF_UNDEAD);
}


/**
 * Helper function for "monster pit (orc)"
 */
static bool vault_aux_orc(int r_idx) {
	return vault_aux_str(r_idx, "o");
}


/**
 * Helper function for "monster pit (troll)"
 */
static bool vault_aux_troll(int r_idx) {
	return vault_aux_str(r_idx, "T");
}

/**
 * Helper function for "monster pit (giant)"
 */
static bool vault_aux_giant(int r_idx) {
	return vault_aux_str(r_idx, "P");
}


/*
 * Hack -- breath type for "vault_aux_dragon()"
 */
static bitflag vault_aux_dragon_mask[RSF_SIZE];


/**
 * Helper function for "monster pit (dragon)"
 */
static bool vault_aux_dragon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];
	bitflag mon_breath[RSF_SIZE];

	if (!vault_aux_str(r_idx, "Dd")) return FALSE;

	/* Hack -- Require correct "breath attack" */
	rsf_copy(mon_breath, r_ptr->spell_flags);
	flags_mask(mon_breath, RSF_SIZE, RSF_BREATH_MASK, FLAG_END);

	return rsf_is_equal(mon_breath, vault_aux_dragon_mask);
}


/*
 * Helper function for "monster pit (demon)"
 */
static bool vault_aux_demon(int r_idx) {
	return vault_aux_str(r_idx, "U");
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
 * Currently, a monster nest is one of
 *   a nest of "jelly" monsters (dungeon level 5 and deeper)
 *   a nest of "animal" monsters (dungeon level 30 and deeper)
 *   a nest of "undead" monsters (dungeon level 50 and deeper)
 *
 * Note that get_mon_num() function can fail, in which case the nest will be
 * empty, and will not affect the level rating.
 *
 * Monster nests will never contain unique monsters.
 */
static bool build_nest(struct cave *c, int y0, int x0) {
	int y, x, y1, x1, y2, x2;
	int tmp, i;
	int alloc_obj;
	s16b what[64];
	cptr name;
	bool empty = FALSE;
	int light = FALSE;

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

	/* Choose a nest type */
	tmp = randint1(c->depth);

	if (tmp < 30) {
		/* Monster nest (jelly) */
		name = "jelly";
		get_mon_num_hook = vault_aux_jelly;
		alloc_obj = 30;

	} else if (tmp < 50) {
		/* Monster nest (animal) */
		name = "animal";
		get_mon_num_hook = vault_aux_animal;
		alloc_obj = 10;

	} else {
		/* Monster nest (undead) */
		name = "undead";
		get_mon_num_hook = vault_aux_undead;
		alloc_obj = 5;
	}

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
	ROOM_LOG("Monster nest (%s)", name);

	/* Increase the level rating */
	c->rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Nests") */
	if (c->depth <= 40 && randint1(c->depth * c->depth + 1) < 300)
		c->good_item = TRUE;

	/* Place some monsters */
	for (y = y0 - 2; y <= y0 + 2; y++) {
		for (x = x0 - 9; x <= x0 + 9; x++) {
			/* Figure out what monster is being used, and place that monster */
			int r_idx = what[randint0(64)];
			place_monster_aux(c, y, x, r_idx, FALSE, FALSE);

			/* Occasionally place an item, making it good 1/3 of the time */
			if (one_in_(alloc_obj)) 
				place_object(c, y, x, c->depth + 10, one_in_(3), FALSE);
		}
	}

	return TRUE;
}



/**
 * Build a monster pits
 *
 * Monster pits are laid-out similarly to monster nests.
 *
 * Monster types in the pit
 *   orc pit (dungeon Level 5 and deeper)
 *   troll pit (dungeon Level 20 and deeper)
 *   giant pit	(dungeon Level 40 and deeper)
 *   dragon pit (dungeon Level 60 and deeper)
 *   demon pit (dungeon Level 80 and deeper)
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
 * All of the dragons in a dragon pit must be the same color, which is handled
 * by requiring a specific "breath" attack for all of the dragons. This may
 * include "multi-hued" breath.  Note that "wyrms" may be present in many of
 * the dragon pits, if they have the proper breath.
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
	int tmp, what[16];
	int i, j, y, x, y1, x1, y2, x2;
	bool empty = FALSE;
	int light = FALSE;
	cptr name;

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

	/* Choose a pit type */
	tmp = randint1(c->depth);

	if (tmp < 20) {
		/* Orc pit */
		name = "orc";
		get_mon_num_hook = vault_aux_orc;
	} else if (tmp < 40) {
		/* Troll pit */
		name = "troll";
		get_mon_num_hook = vault_aux_troll;
	} else if (tmp < 60) {
		/* Giant pit */
		name = "giant";
		get_mon_num_hook = vault_aux_giant;
	} else if (tmp < 80) {
		/* Pick type of dragon pit */
		switch (randint0(6)) {
			case 0: {
				name = "acid dragon";
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_ACID, FLAG_END);
				break;
			}

			case 1: {
				name = "electric dragon";
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_ELEC, FLAG_END);
				break;
			}

			case 2: {
				name = "fire dragon";
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_FIRE, FLAG_END);
				break;
			}

			case 3: {
				name = "cold dragon";
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_COLD, FLAG_END);
				break;
			}

			case 4: {
				name = "poison dragon";
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_POIS, FLAG_END);
				break;
			}

			default: {
				name = "multi-hued dragon";
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_ACID, RSF_BR_ELEC,
				           RSF_BR_FIRE, RSF_BR_COLD, RSF_BR_POIS, FLAG_END);
				break;
			}

		}

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_dragon;
	} else {
		name = "demon";
		get_mon_num_hook = vault_aux_demon;
	}

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
	if (empty) return FALSE;

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

	ROOM_LOG("Monster pit (%s)", name);

	/* Increase the level rating */
	c->rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
	if (c->depth <= 40 && randint1(c->depth * c->depth + 1) < 300)
		c->good_item = TRUE;

	/* Top and bottom rows */
	for (x = x0 - 9; x <= x0 + 9; x++) {
		place_monster_aux(c, y0 - 2, x, what[0], FALSE, FALSE);
		place_monster_aux(c, y0 + 2, x, what[0], FALSE, FALSE);
	}

	/* Middle columns */
	for (y = y0 - 1; y <= y0 + 1; y++) {
		place_monster_aux(c, y, x0 - 9, what[0], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 9, what[0], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 8, what[1], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 8, what[1], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 7, what[1], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 7, what[1], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 6, what[2], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 6, what[2], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 5, what[2], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 5, what[2], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 4, what[3], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 4, what[3], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 3, what[3], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 3, what[3], FALSE, FALSE);

		place_monster_aux(c, y, x0 - 2, what[4], FALSE, FALSE);
		place_monster_aux(c, y, x0 + 2, what[4], FALSE, FALSE);
	}

	/* Above/Below the center monster */
	for (x = x0 - 1; x <= x0 + 1; x++) {
		place_monster_aux(c, y0 + 1, x, what[5], FALSE, FALSE);
		place_monster_aux(c, y0 - 1, x, what[5], FALSE, FALSE);
	}

	/* Next to the center monster */
	place_monster_aux(c, y0, x0 + 1, what[6], FALSE, FALSE);
	place_monster_aux(c, y0, x0 - 1, what[6], FALSE, FALSE);

	/* Center monster */
	place_monster_aux(c, y0, x0, what[7], FALSE, FALSE);

	return TRUE;
}


/**
 * Build a vault from its string representation.
 */
static void build_vault(struct cave *c, int y0, int x0, int ymax, int xmax, cptr data) {
	int dx, dy, x, y;
	cptr t;

	assert(c);

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++) {
		for (dx = 0; dx < xmax; dx++, t++) {
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Skip non-grids */
			if (*t == ' ') continue;

			/* Lay down a floor */
			cave_set_feat(c, y, x, FEAT_FLOOR);

			/* Part of a vault */
			c->info[y][x] |= (CAVE_ROOM | CAVE_ICKY);

			/* Analyze the grid */
			switch (*t) {
				case '%': cave_set_feat(c, y, x, FEAT_WALL_OUTER); break;
				case '#': cave_set_feat(c, y, x, FEAT_WALL_INNER); break;
				case 'X': cave_set_feat(c, y, x, FEAT_PERM_INNER); break;
				case '+': place_secret_door(c, y, x); break;
				case '^': place_trap(c, y, x); break;
				case '*': {
					/* Treasure or a trap */
					if (randint0(100) < 75)
						place_object(c, y, x, c->depth, FALSE, FALSE);
					else
						place_trap(c, y, x);
					break;
				}
			}
		}
	}


	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax; dy++) {
		for (dx = 0; dx < xmax; dx++, t++) {
			/* Extract the grid */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the symbol */
			switch (*t) {
				case '&': place_monster(c, y, x, c->depth + 5, TRUE, TRUE); break;
				case '@': place_monster(c, y, x, c->depth + 11, TRUE, TRUE); break;

				case '9': {
					/* Meaner monster, plus treasure */
					place_monster(c, y, x, c->depth + 9, TRUE, TRUE);
					place_object(c, y, x, c->depth + 7, TRUE, FALSE);
					break;
				}

				case '8': {
					/* Nasty monster and treasure */
					place_monster(c, y, x, c->depth + 40, TRUE, TRUE);
					place_object(c, y, x, c->depth + 20, TRUE, FALSE);
					break;
				}

				case ',': {
					/* Monster and/or object */
					if (randint0(100) < 50)
						place_monster(c, y, x, c->depth + 3, TRUE, TRUE);
					if (randint0(100) < 50)
						place_object(c, y, x, c->depth + 7, FALSE, FALSE);
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

	/* Boost the rating and sometimes cause a special feeling */
	c->rating += v_ptr->rat;
	if (c->depth <= 50 || randint0(45) + 60 > c->depth) c->good_item = TRUE;

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

		while (!in_bounds_fully(tmp_row, tmp_col)) {
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
		if (c->feat[tmp_row][tmp_col] == FEAT_PERM_SOLID) continue;

		/* Avoid the edge of vaults */
		if (c->feat[tmp_row][tmp_col] == FEAT_PERM_OUTER) continue;

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
	int i, y, x, k = 0;

	assert(cave_in_bounds_fully(c, y1, x1));

	/* Scan adjacent grids */
	for (i = 0; i < 4; i++) {
		/* Extract the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Skip non-floors, non-empty floors, and rooms */
		if (!cave_isfloor(c, y, x)) continue;
		if (c->feat[y][x] != FEAT_FLOOR) continue;
		if (c->info[y][x] & CAVE_ROOM) continue;

		/* Count these grids */
		k++;
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
	assert(cave_in_bounds_fully(c, y, x));

	if (next_to_corr(c, y, x) < 2)
		return FALSE;
	else if (c->feat[y-1][x] >= FEAT_MAGMA && c->feat[y+1][x] >= FEAT_MAGMA)
		return TRUE;
	else if (c->feat[y][x-1] >= FEAT_MAGMA && c->feat[y][x+1] >= FEAT_MAGMA)
		return TRUE;
	else
		return FALSE;
}


/**
 * Places door at y, x position if at least 2 walls found
 */
static void try_door(struct cave *c, int y, int x) {
	assert(cave_in_bounds(c, y, x));

	if (c->feat[y][x] >= FEAT_MAGMA) return;
	if (c->info[y][x] & CAVE_ROOM) return;

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
	for (by = by1; by <= by2; by++)
		for (bx = bx1; bx <= bx2; bx++)
			if (dun->room_map[by][bx])
				return FALSE;

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
	for (by = by1; by <= by2; by++)
		for (bx = bx1; bx <= bx2; bx++)
			dun->room_map[by][bx] = TRUE;

	/* Count "crowded" rooms */
	if (profile.crowded) dun->crowded = TRUE;

	/* Success */
	return TRUE;
}

/**
 * Generate a new dungeon level.
 */
#define DUN_AMT_ROOM 7	/* Number of objects for rooms */
#define DUN_AMT_ITEM 2 /* Number of objects for rooms/corridors */
#define DUN_AMT_GOLD 3 /* Amount of treasure for rooms/corridors */
static bool cave_gen(struct cave *c, struct player *p) {
	int i, j, k, l, y, x, y1, x1;
	int by, bx, key, rarity, tries;
	int num_rooms, size_percent;
	int dun_unusual = dun->profile->dun_unusual;

	bool blocks_tried[MAX_ROOMS_ROW][MAX_ROOMS_COL];

	/* Possibly generate fewer rooms in a smaller area via a scaling factor.
	 * Since we scale row_rooms and col_rooms by the same amount, DUN_ROOMS
	 * gives the same "room density" no matter what size the level turns out
	 * to be. TODO: vary room density slightly? */
	/* XXX: Until vault generation is improved, scaling variance is reduced */
	i = randint1(10);
	if (is_quest(c->depth)) size_percent = 100;
	else if (i < 2) size_percent = 75;
	else if (i < 3) size_percent = 80;
	else if (i < 4) size_percent = 85;
	else if (i < 5) size_percent = 90;
	else if (i < 6) size_percent = 95;
	else size_percent = 100;
	size_percent = 100;

	/* scale the various generation variables */
	num_rooms = dun->profile->dun_rooms * size_percent / 100;
	c->height = DUNGEON_HGT * size_percent / 100;
	c->width  = DUNGEON_WID * size_percent / 100;

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
	tries = 0;
	while(tries < num_rooms) {
		tries++;

		/* Count the room blocks we haven't tried yet. */
		j = 0;
		for(by=0; by < dun->row_rooms; by++)
			for(bx=0; bx < dun->col_rooms; bx++)
				if (!blocks_tried[by][bx]) j++;

		/* If we've tried all blocks we're done. */
		if (j == 0) break;

		/* Choose one of the j untried blocks, saving its coordinates. */
		k = randint0(j);
		l = 0;
		for(by = 0; by < dun->row_rooms; by++) {
			for(bx = 0; bx < dun->col_rooms; bx++) {
				if (blocks_tried[by][bx]) continue;
				if (l == k) break;
				l++;
			}
			if (l == k) break;
		}

		/* Mark that we are trying this block. */
		blocks_tried[by][bx] = TRUE;

		/* Roll for random key (to be compared against a profile's cutoff) */
		key = randint0(100);

		/* We generate a rarity number to figure out how exotic to make the
		 * room. This number has a depth/DUN_UNUSUAL chance of being > 0,
		 * a depth^2/DUN_UNUSUAL^2 chance of being > 1, up to MAX_RARITY.
		 */
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
		 * a room that we can build successfully or we exhaust the profiles.
         */
		i = 0;
		for (i = 0; i < dun->profile->n_room_profiles; i++) {
			struct room_profile profile = dun->profile->room_profiles[i];
			if (profile.rarity > rarity) continue;
			if (profile.cutoff <= key) continue;
			
			if (room_build(c, by, bx, profile)) break;
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
	alloc_objects(c, SET_CORR, TYP_RUBBLE, randint1(k), c->depth);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint1(8) + k;

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

	/* Ensure quest monsters */
	if (is_quest(c->depth)) {
		for (i = 1; i < z_info->r_max; i++) {
			monster_race *r_ptr = &r_info[i];
			int y, x;
			
			/* The monster must be an unseen quest monster of this depth. */
			if (r_ptr->cur_num > 0) continue;
			if (!rf_has(r_ptr->flags, RF_QUESTOR)) continue;
			if (r_ptr->level != c->depth) continue;

			/* Pick a location and place the monster */
			find_empty(c, &y, c->height, &x, c->width);
			place_monster_aux(c, y, x, i, TRUE, TRUE);
		}
	}

	/* Put some objects in rooms */
	alloc_objects(c, SET_ROOM, TYP_OBJECT, Rand_normal(DUN_AMT_ROOM, 3), c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(DUN_AMT_ITEM, 3), c->depth);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(DUN_AMT_GOLD, 3), c->depth);

	return TRUE;
}


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
 *
 */
static bool labyrinth_gen(struct cave *c, struct player *p) {
	int i, j, k, y, x;

	/* Height and width must be odd. */
	int h = c->height = 17;
	int w = c->width  = 59;

	int n = h * w;

	/* NOTE: these arrays are too large... we really only need to use about
	 * 1/4 as much memory. However, in that case, the addressing math becomes
	 * a lot more complicated, so let's just stick with this because it's
	 * easier to read. */

	/* sets tracks connectedness; if sets[i] == sets[j] then cells i and j
	 * are connected to each other in the maze. */
	int sets[n];

	/* walls is a list of wall coordinates which we will randomize */
	int walls[n];

	/* Don't try this on quest levels, kids... */
	if (is_quest(c->depth)) return FALSE;

	/* Fill whole level with perma-rock */
	fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);

	/* Fill the labyrinth area with rock */
	fill_rectangle(c, 1, 1, h, w, FEAT_WALL_SOLID);

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
		}
	}

	/* Shuffle the walls, using Knuth's shuffle. */
	for (i = 0; i < n; i++) {
		j = randint0(n - i);
		k = walls[i];
		walls[i] = walls[j];
		walls[j] = k;
	}

	/* For each adjoining wall, look at the cells it divides. If they aren't
	 * in the same set, remove the wall and join their sets.
	 *
	 * This is a randomized version of Kruskal's algorithm.
	 */
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

			for (k = 0; k < n; k++) {
				if (sets[k] == sb) sets[k] = sa;
			}
		}
	}

	/* Place 1-2 down stairs near some walls */
	if (OPT(birth_no_stairs) || !p->create_up_stair)
		alloc_stairs(c, FEAT_MORE, 1, 3);
    
	/* Place 1 up stairs near some walls */
	if (OPT(birth_no_stairs) || !p->create_down_stair)
		alloc_stairs(c, FEAT_LESS, 1, 3);

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint1(8) + k;

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(6, 3), c->depth);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(6, 3), c->depth);

	return TRUE;
}

static bool bad_labyrinth_gen(struct cave *c, struct player *p) {
	/* must be odd? */
	int height = c->height = 21;
	int width = c->width  = 65;

	int complexity = (75 * 5 * (height + width)) / 100;
	int density = (75 * (height / 2 + width / 2)) / 100;

	int i, j, k;

	/* Don't try this on quest levels, kids... */
	if (is_quest(c->depth)) return FALSE;

	/* Fill whole level with perma-rock, then labyrinth area with open space */
	fill_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);
	fill_rectangle(c, 1, 1, height - 2, width - 2, FEAT_FLOOR);

	/* Generate the actual labyrinth structure. */
	for (i = 0; i < density; i++) {
		/* Choose a random set of even coordinates */
		int x = randint0(width / 2) * 2;
		int y = randint0(height / 2) * 2;

		cave_set_feat(c, y, x, FEAT_WALL_SOLID);

		for (j = 0; j < complexity; j++) {
			/* We are going to choose a random neighbor, so we need to track
			 * n (the number of neighbors so far), and (nx, ny) the coordinates
			 * the neighbor we currently have.
			 */
			int n = 0;
			int nx = 0;
			int ny = 0;

			for (k = 0; k < 4; k++) {
				/* Choose one of the four cardinal directions, and get the
				 * neighbor that is two moves in that direction */
				int kx = x - ddx_ddd[k] * 2;
				int ky = y - ddy_ddd[k] * 2;

				/* We can only use coordinates within the labyrinth */
				if (kx < 0 || kx >= width || ky < 0 || ky >= height) continue;

				/* Possibly update the neighbor we've chosen */
				n++;
				if (one_in_(n)) {
					nx = kx;
					ny = ky;
				}
			}

			/* If the neighbor isn't a wall yet, fill both it and the
			 * intermediate square. */
			if (c->feat[ny][nx] == FEAT_FLOOR) {
				int ox = (nx + x) / 2;
				int oy = (ny + y) / 2;
				cave_set_feat(c, oy, ox, FEAT_WALL_SOLID);
				cave_set_feat(c, ny, nx, FEAT_WALL_SOLID);
			}

			/* Continue traversing from our neighbor */
			x = nx;
			y = ny;
		}
	}

	/* XYZ: Generate permanent walls around the edge of the dungeon */
	draw_rectangle(c, 0, 0, DUNGEON_HGT - 1, DUNGEON_WID - 1, FEAT_PERM_SOLID);

	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(3, 4), 3);

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2), 3);

	/* General amount of rubble, traps and monsters */
	k = MAX(MIN(c->depth / 3, 10), 2);

	/* Put some rubble in corridors */
	alloc_objects(c, SET_BOTH, TYP_RUBBLE, randint1(k), c->depth);

	/* Place some traps in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_TRAP, randint1(k), c->depth);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint1(8) + k;

	/* Put some monsters in the dungeon */
	for (; i > 0; i--)
		alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_objects(c, SET_BOTH, TYP_OBJECT, Rand_normal(DUN_AMT_ITEM, 3), c->depth);
	alloc_objects(c, SET_BOTH, TYP_GOLD, Rand_normal(DUN_AMT_GOLD, 3), c->depth);

	return TRUE;
}


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
	find_empty_range(c, &y, 3, TOWN_HGT - 4, &x, 3, TOWN_WID - 4);

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

	c->height = TOWN_HGT;
	c->width = TOWN_WID;

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
		}
	}

	/* Unset the player's coordinates */
	p->px = p->py = 0;

	/* Nothing special here yet */
	c->good_item = FALSE;

	/* Nothing good here yet */
	c->rating = 0;
}

/**
 * Calculate the level feeling.
 */
static int calculate_feeling(struct cave *c) {
	/* Town gets no feeling */
	if (c->depth == 0) return 0;

	/* Artifacts trigger a special feeling when preserve=no */
	if (c->good_item && OPT(birth_no_preserve)) return 1;

	if (c->rating > 50 + c->depth) return 2;
	if (c->rating > 40 + 4 * c->depth / 5) return 3;
	if (c->rating > 30 + 3 * c->depth / 5) return 4;
	if (c->rating > 20 + 2 * c->depth / 5) return 5;
	if (c->rating > 15 + c->depth / 3) return 6;
	if (c->rating > 10 + c->depth / 5) return 7;
	if (c->rating > 5 + c->depth / 10) return 8;
	if (c->rating > 0) return 9;
	return 10;
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
		
		if (!p->depth) {
			dun->profile = &town_profile;
		} else {
			//dun->profile = &cave_profiles[0];
			dun->profile = &cave_profiles[1];
		}

		dun->profile->builder(c, p);

		c->feeling = calculate_feeling(c);

		/* Regenerate levels that overflow their maxima */
		if (o_max >= z_info->o_max) 
			error = "too many objects";
		if (mon_max >= z_info->m_max)
			error = "too many monsters";

		if (error) ROOM_LOG("Generation restarted: %s.", error);
	}

	if (error) quit_fmt("cave_generate() failed 100 times!");

	/* The dungeon is ready */
	character_dungeon = TRUE;

	c->created_at = turn;
}
