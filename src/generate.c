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
#include "files.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "trap.h"
#include "z-type.h"

/*
 * Note that Level generation is *not* an important bottleneck,
 * though it can be annoyingly slow on older machines...  Thus
 * we emphasize "simplicity" and "correctness" over "speed".
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * Consider the "vault.txt" file for vault generation.
 *
 * In this file, we use the "special" granite and perma-wall sub-types,
 * where "basic" is normal, "inner" is inside a room, "outer" is the
 * outer wall of a room, and "solid" is the outer wall of the dungeon
 * or any walls that may not be pierced by corridors.  Thus the only
 * wall type that may be pierced by a corridor is the "outer granite"
 * type.  The "basic granite" type yields the "actual" corridors.
 *
 * Note that we use the special "solid" granite wall type to prevent
 * multiple corridors from piercing a wall in two adjacent locations,
 * which would be messy, and we use the special "outer" granite wall
 * to indicate which walls "surround" rooms, and may thus be "pierced"
 * by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away).  This
 * is not a problem since every room that is large enough to block
 * the passage of tunnels is also large enough to allow the tunnel
 * to pierce the room itself several times.
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart.  This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks.  As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds.  Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the 33x11 panels)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries.  This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen.  This can be so annoying that the player must set
 * a special option to enable "non-aligned" room generation.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * XXX XXX XXX Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * XXX XXX XXX Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening.  But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * Note that (2.7.9) monster pits have been split into monster "nests"
 * and monster "pits".  The "nests" have a collection of monsters of a
 * given type strewn randomly around the room (jelly, animal, or undead),
 * while the "pits" have a collection of monsters of a given type placed
 * around the room in an organized manner (orc, troll, giant, dragon, or
 * demon).  Note that both "nests" and "pits" are now "level dependant",
 * and both make 16 "expensive" calls to the "get_mon_num()" function.
 *
 * Note that the cave grid flags changed in a rather drastic manner
 * for Angband 2.8.0 (and 2.7.9+), in particular, dungeon terrain
 * features, such as doors and stairs and traps and rubble and walls,
 * are all handled as a set of 64 possible "terrain features", and
 * not as "fake" objects (440-479) as in pre-2.8.0 versions.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite.  XXX XXX XXX
 */


/*
 * Dungeon generation values
 */
#define DUN_ROOMS	50	/* Number of rooms to attempt */
#define DUN_UNUSUAL	200	/* Level/chance of unusual room */
#define DUN_DEST	30	/* 1/chance of having a destroyed level */

/*
 * Dungeon tunnel generation values
 */
#define DUN_TUN_RND	10	/* Chance of random direction */
#define DUN_TUN_CHG	30	/* Chance of changing direction */
#define DUN_TUN_CON	15	/* Chance of extra tunneling */
#define DUN_TUN_PEN	25	/* Chance of doors at room entrances */
#define DUN_TUN_JCT	90	/* Chance of doors at tunnel junctions */

/*
 * Dungeon streamer generation values
 */
#define DUN_STR_DEN	5	/* Density of streamers */
#define DUN_STR_RNG	2	/* Width of streamers */
#define DUN_STR_MAG	3	/* Number of magma streamers */
#define DUN_STR_MC	90	/* 1/chance of treasure per magma */
#define DUN_STR_QUA	2	/* Number of quartz streamers */
#define DUN_STR_QC	40	/* 1/chance of treasure per quartz */

/*
 * Dungeon treausre allocation values
 */
#define DUN_AMT_ROOM	7	/* Number of objects for rooms */
#define DUN_AMT_ITEM	2	/* Number of objects for rooms/corridors */
#define DUN_AMT_GOLD	3	/* Amount of treasure for rooms/corridors */

/*
 * Hack -- Dungeon allocation "places"
 */
#define ALLOC_SET_CORR		1	/* Hallway */
#define ALLOC_SET_ROOM		2	/* Room */
#define ALLOC_SET_BOTH		3	/* Anywhere */

/*
 * Hack -- Dungeon allocation "types"
 */
#define ALLOC_TYP_RUBBLE	1	/* Rubble */
#define ALLOC_TYP_TRAP		3	/* Trap */
#define ALLOC_TYP_GOLD		4	/* Gold */
#define ALLOC_TYP_OBJECT	5	/* Object */



/*
 * Maximum numbers of rooms along each axis (currently 6x18)
 */
#define MAX_ROOMS_ROW	(DUNGEON_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL	(DUNGEON_WID / BLOCK_WID)


/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	100
#define DOOR_MAX	200
#define WALL_MAX	500
#define TUNN_MAX	900


/*
 * Maximum number of room types
 */
#define ROOM_MAX	10

/*
 * Treasure allocation probabilities for nests
 */
#define JELLY_NEST_OBJ 30
#define ANIMAL_NEST_OBJ 10
#define UNDEAD_NEST_OBJ 5

/*
 * Room type information
 */

typedef struct room_data room_data;

struct room_data
{
	/* Required size in blocks */
	s16b dy1, dy2, dx1, dx2;

	/* Hack -- minimum level */
	s16b level;
};


/*
 * Structure to hold all "dungeon generation" data
 */

struct dun_data {
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

static struct dun_data *dun;


/*
 * Array of room types (assumes 11x11 blocks)
 */
static const room_data room[ROOM_MAX] =
{
	{ 0, 0, 0, 0, 0 },		/* 0 = Nothing */
	{ 0, 0, -1, 1, 1 },		/* 1 = Simple (33x11) */
	{ 0, 0, -1, 1, 1 },		/* 2 = Overlapping (33x11) */
	{ 0, 0, -1, 1, 3 },		/* 3 = Crossed (33x11) */
	{ 0, 0, -1, 1, 3 },		/* 4 = Large (33x11) */
	{ 0, 0, -1, 1, 5 },		/* 5 = Monster nest (33x11) */
	{ 0, 0, -1, 1, 5 },		/* 6 = Monster pit (33x11) */
	{ 0, 1, -1, 1, 5 },		/* 7 = Lesser vault (33x22) */
	{ 0, 1, -1, 1, 5 },		/* 8 = Medium vault (33x22) */
	{ -1, 2, -2, 3, 10 }	/* 9 = Greater vault (66x44) */
};



/*
 * Always picks a correct direction
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
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


/*
 * Pick a random direction
 */
static void rand_dir(int *rdir, int *cdir)
{
	/* Pick a random direction */
	int i = randint0(4);

	/* Extract the dy/dx components */
	*rdir = ddy_ddd[i];
	*cdir = ddx_ddd[i];
}


/*
 * Returns random co-ordinates for player/monster/object
 */
static void new_player_spot(struct cave *c, struct player *p)
{
	int y, x;

	assert(c);

	/* Place the player */
	while (1)
	{
		/* Pick a legal spot */
		y = rand_range(1, c->height - 2);
		x = rand_range(1, c->width - 2);

		if (!cave_isempty(c, y, x))
			continue;

		/* Refuse to start on anti-teleport grids */
		if (c->info[y][x] & CAVE_ICKY)
			continue;

		/* Done */
		break;
	}

	if (!OPT(birth_no_stairs)) {
		if (p->create_down_stair) {
			cave_set_feat(c, y, x, FEAT_MORE);
			p->create_down_stair = FALSE;
		} else if (p->create_up_stair) {
			cave_set_feat(c, y, x, FEAT_LESS);
			p->create_up_stair = FALSE;
		}
	}

	player_place(c, p, y, x);
}

static int next_to_walls(struct cave *c, int y, int x)
{
	int k = 0;

	assert(in_bounds_fully(y, x));

	if (c->feat[y+1][x] >= FEAT_WALL_EXTRA) k++;
	if (c->feat[y-1][x] >= FEAT_WALL_EXTRA) k++;
	if (c->feat[y][x+1] >= FEAT_WALL_EXTRA) k++;
	if (c->feat[y][x-1] >= FEAT_WALL_EXTRA) k++;

	return k;
}

static void place_rubble(struct cave *c, int y, int x)
{
	cave_set_feat(c, y, x, FEAT_RUBBLE);
}

static void place_up_stairs(struct cave *c, int y, int x)
{
	cave_set_feat(c, y, x, FEAT_LESS);
}

static void place_down_stairs(struct cave *c, int y, int x)
{
	cave_set_feat(c, y, x, FEAT_MORE);
}

static void place_random_stairs(struct cave *c, int y, int x)
{
	if (!cave_canputitem(c, y, x))
		return;

	if (!c->depth)
		place_down_stairs(c, y, x);
	else if (is_quest(c->depth) || (c->depth >= MAX_DEPTH-1))
		place_up_stairs(c, y, x);
	else if (randint0(100) < 50)
		place_down_stairs(c, y, x);
	else
		place_up_stairs(c, y, x);
}

void place_object(struct cave *c, int y, int x, int level, bool good, bool great)
{
	object_type otype;

	assert(cave_in_bounds(c, y, x));

	if (!cave_canputitem(c, y, x))
		return;

	object_wipe(&otype);
	if (make_object(c, &otype, level, good, great)) {
		otype.origin = ORIGIN_FLOOR;
		otype.origin_depth = c->depth;

		/* Give it to the floor */
		if (!floor_carry(c, y, x, &otype) && otype.name1) {
			/* XXX Should this be done in floor_carry? */
			a_info[otype.name1].created = FALSE;
		}
	}
}

void place_gold(struct cave *c, int y, int x, int level)
{
	object_type *i_ptr;
	object_type object_type_body;

	assert(cave_in_bounds(c, y, x));

	if (!cave_canputitem(c, y, x))
		return;

	i_ptr = &object_type_body;
	object_wipe(i_ptr);
	make_gold(i_ptr, level, SV_GOLD_ANY);
	floor_carry(c, y, x, i_ptr);
}

void place_secret_door(struct cave *c, int y, int x)
{
	cave_set_feat(c, y, x, FEAT_SECRET);
}

void place_closed_door(struct cave *c, int y, int x)
{
	int tmp = randint0(400);

	if (tmp < 300)
		cave_set_feat(c, y, x, FEAT_DOOR_HEAD + 0x00);
	else if (tmp < 399)
		cave_set_feat(c, y, x, FEAT_DOOR_HEAD + randint1(7));
	else
		cave_set_feat(c, y, x, FEAT_DOOR_HEAD + 0x08 + randint0(8));
}

void place_random_door(struct cave *c, int y, int x)
{
	int tmp = randint0(1000);

	if (tmp < 300)
		cave_set_feat(c, y, x, FEAT_OPEN);
	else if (tmp < 400)
		cave_set_feat(c, y, x, FEAT_BROKEN);
	else if (tmp < 600)
		cave_set_feat(c, y, x, FEAT_SECRET);
	else
		place_closed_door(c, y, x);
}

/*
 * Places some staircases near walls
 * XXX: Might loop infinitely if there are no free spaces for stairs. This
 * method is stupid - we should just collect all the plausible places and
 * allocate stairs into those in random order. Fix me!
 */
static void alloc_stairs(struct cave *c, int feat, int num, int walls)
{
	int y, x, i, j, flag;


	/* Place "num" stairs */
	for (i = 0; i < num; i++)
	{
		/* Place some stairs */
		for (flag = FALSE; !flag; )
		{
			/* Try several times, then decrease "walls" */
			for (j = 0; !flag && j <= 3000; j++)
			{
				/* Pick a random grid */
				y = randint0(c->height);
				x = randint0(c->width);

				if (!cave_isempty(c, y, x))
					continue;

				if (next_to_walls(c, y, x) < walls)
					continue;

				/* Town -- must go down */
				if (!c->depth)
					cave_set_feat(c, y, x, FEAT_MORE);
				else if (is_quest(c->depth) || (c->depth >= MAX_DEPTH-1))
					cave_set_feat(c, y, x, FEAT_LESS);
				else
					cave_set_feat(c, y, x, feat);

				/* All done */
				flag = TRUE;
			}

			/* Require fewer walls */
			if (walls) walls--;
		}
	}
}



/*
 * Allocates some objects (using "place" and "type")
 */
static void alloc_object(struct cave *c, int set, int typ, int num, int depth)
{
	int y, x, k, tries;
	bool room;

	/* Place some objects */
	for (k = 0; k < num; k++)
	{
		tries = 0;

		/* Pick a "legal" spot */
		while (tries < 10000)
		{
			tries++;

			/* Location */
			y = randint0(c->height);
			x = randint0(c->width);

			/* Require "naked" floor grid */
			if (!cave_isempty(c, y, x)) continue;

			/* Check for "room" */
			room = (c->info[y][x] & CAVE_ROOM) ? TRUE : FALSE;

			/* Require corridor? */
			if ((set == ALLOC_SET_CORR) && room) continue;

			/* Require room? */
			if ((set == ALLOC_SET_ROOM) && !room) continue;

			/* Accept it */
			break;
		}

		/* Place something */
		switch (typ)
		{
			case ALLOC_TYP_RUBBLE:
			{
				place_rubble(c, y, x);
				break;
			}

			case ALLOC_TYP_TRAP:
			{
				place_trap(c, y, x);
				break;
			}

			case ALLOC_TYP_GOLD:
			{
				place_gold(c, y, x, depth);
				break;
			}

			case ALLOC_TYP_OBJECT:
			{
				place_object(c, y, x, depth, FALSE, FALSE);
				break;
			}
		}
	}
}



/*
 * Places "streamers" of rock through dungeon
 *
 * Note that their are actually six different terrain features used
 * to represent streamers.  Three each of magma and quartz, one for
 * basic vein, one with hidden gold, and one with known gold.  The
 * hidden gold types are currently unused.
 */
static void build_streamer(struct cave *c, int feat, int chance)
{
	int i, tx, ty;
	int y, x, dir;


	/* Hack -- Choose starting point */
	y = rand_spread(DUNGEON_HGT / 2, 10);
	x = rand_spread(DUNGEON_WID / 2, 15);

	/* Choose a random compass direction */
	dir = ddd[randint0(8)];

	/* Place streamer into dungeon */
	while (TRUE)
	{
		/* One grid per density */
		for (i = 0; i < DUN_STR_DEN; i++)
		{
			int d = DUN_STR_RNG;

			/* Pick a nearby grid */
			while (1)
			{
				ty = rand_spread(y, d);
				tx = rand_spread(x, d);
				if (!cave_in_bounds(c, ty, tx)) continue;
				break;
			}

			/* Only convert "granite" walls */
			if (c->feat[ty][tx] < FEAT_WALL_EXTRA) continue;
			if (c->feat[ty][tx] > FEAT_WALL_SOLID) continue;

			/* Clear previous contents, add proper vein type */
			cave_set_feat(c, ty, tx, feat);

			/* Hack -- Add some (known) treasure */
			/* XXX: 0x04? Seriously? */
			if (one_in_(chance)) c->feat[ty][tx] += 0x04;
		}

		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Stop at dungeon edge */
		if (!cave_in_bounds(c, y, x)) break;
	}
}


/*
 * Create up to "num" objects near the given coordinates
 * Only really called by some of the "vault" routines.
 */
static void vault_objects(struct cave *c, int y, int x, int depth, int num)
{
	int i, j, k;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num)
	{
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i)
		{
			/* Pick a random location */
			while (1)
			{
				j = rand_spread(y, 2);
				k = rand_spread(x, 3);
				if (!in_bounds(j, k)) continue;
				break;
			}

			/* Require "clean" floor space */
			if (!cave_canputitem(c, j, k)) continue;

			/* Place an item */
			if (randint0(100) < 75)
			{
				place_object(c, j, k, depth, FALSE, FALSE);
			}

			/* Place gold */
			else
			{
				place_gold(c, j, k, depth);
			}

			/* Placement accomplished */
			break;
		}
	}
}


/*
 * Place a trap with a given displacement of point
 */
static void vault_trap_aux(struct cave *c, int y, int x, int yd, int xd)
{
	int count, y1, x1;

	for (count = 0; count <= 5; count++) {
		while (1) {
			y1 = rand_spread(y, yd);
			x1 = rand_spread(x, xd);
			if (!cave_in_bounds(c, y1, x1))
				continue;
			break;
		}

		if (!cave_isempty(c, y1, x1))
			continue;

		place_trap(c, y1, x1);
		break;
	}
}


/*
 * Place some traps with a given displacement of given location
 */
static void vault_traps(struct cave *c, int y, int x, int yd, int xd, int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		vault_trap_aux(c, y, x, yd, xd);
	}
}


/*
 * Place some sleeping monsters near the given location
 */
static void vault_monsters(struct cave *c, int y1, int x1, int depth, int num)
{
	int k, i, y, x;

	/* Try to summon "num" monsters "near" the given location */
	for (k = 0; k < num; k++)
	{
		/* Try nine locations */
		for (i = 0; i < 9; i++)
		{
			int d = 1;

			/* Pick a nearby location */
			scatter(&y, &x, y1, x1, d, 0);

			/* Require "empty" floor grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Place the monster (allow groups) */
			(void)place_monster(c, y, x, depth, TRUE, TRUE);

			break;
		}
	}
}



/*
 * Generate helper -- create a new room with optional light
 */
static void generate_room(struct cave *c, int y1, int x1, int y2, int x2, int light)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			c->info[y][x] |= CAVE_ROOM;
			if (light)
				c->info[y][x] |= CAVE_GLOW;
		}
	}
}


/*
 * Generate helper -- fill a rectangle with a feature
 */
static void generate_fill(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave_set_feat(c, y, x, feat);
		}
	}
}


/*
 * Generate helper -- draw a rectangle with a feature
 */
static void generate_draw(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(c, y, x1, feat);
		cave_set_feat(c, y, x2, feat);
	}

	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(c, y1, x, feat);
		cave_set_feat(c, y2, x, feat);
	}
}


/*
 * Generate helper -- split a rectangle with a feature
 */
static void generate_plus(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y, x;
	int y0, x0;

	assert(c);

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(c, y, x0, feat);
	}

	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(c, y0, x, feat);
	}
}


/*
 * Generate helper -- open all sides of a rectangle with a feature
 */
static void generate_open(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
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


/*
 * Generate helper -- open one side of a rectangle with a feature
 */
static void generate_hole(struct cave *c, int y1, int x1, int y2, int x2, int feat)
{
	int y0, x0;

	assert(c);

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open random side */
	switch (randint0(4))
	{
		case 0:
			cave_set_feat(c, y1, x0, feat);
			break;
		case 1:
			cave_set_feat(c, y0, x1, feat);
			break;
		case 2:
			cave_set_feat(c, y2, x0, feat);
			break;
		case 3:
			cave_set_feat(c, y0, x2, feat);
			break;
	}
}


/*
 * Room building routines.
 *
 * Six basic room types:
 *   1 -- normal
 *   2 -- overlapping
 *   3 -- cross shaped
 *   4 -- large room with features
 *   5 -- monster nests
 *   6 -- monster pits
 *   7 -- simple vaults
 *   8 -- greater vaults
 */


/*
 * Type 1 -- normal rectangular rooms
 */
static void build_type1(struct cave *c, int y0, int x0)
{
	int y, x;

	int y1, x1, y2, x2;

	int light = FALSE;


	/* Occasional light */
	if (c->depth <= randint1(25)) light = TRUE;


	/* Pick a room size */
	y1 = y0 - randint1(4);
	x1 = x0 - randint1(11);
	y2 = y0 + randint1(3);
	x2 = x0 + randint1(11);


	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(c, y1, x1, y2, x2, FEAT_FLOOR);


	/* Hack -- Occasional pillar room */
	if (one_in_(20))
	{
		for (y = y1; y <= y2; y += 2)
		{
			for (x = x1; x <= x2; x += 2)
			{
				cave_set_feat(c, y, x, FEAT_WALL_INNER);
			}
		}
	}

	/* Hack -- Occasional ragged-edge room */
	else if (one_in_(50))
	{
		for (y = y1 + 2; y <= y2 - 2; y += 2)
		{
			cave_set_feat(c, y, x1, FEAT_WALL_INNER);
			cave_set_feat(c, y, x2, FEAT_WALL_INNER);
		}

		for (x = x1 + 2; x <= x2 - 2; x += 2)
		{
			cave_set_feat(c, y1, x, FEAT_WALL_INNER);
			cave_set_feat(c, y2, x, FEAT_WALL_INNER);
		}
	}
}


/*
 * Type 2 -- Overlapping rectangular rooms
 */
static void build_type2(struct cave *c, int y0, int x0)
{
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
	generate_draw(c, y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_draw(c, y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	generate_fill(c, y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(c, y1b, x1b, y2b, x2b, FEAT_FLOOR);
}



/*
 * Type 3 -- Cross shaped rooms
 *
 * Room "a" runs north/south, and Room "b" runs east/east
 * So a "central pillar" would run from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that
 * the code below will work for 5x5 (and perhaps even for unsymetric
 * values like 4x3 or 5x3 or 3x4 or 3x5).
 */
static void build_type3(struct cave *c, int y0, int x0)
{
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
	generate_draw(c, y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_draw(c, y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	generate_fill(c, y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(c, y1b, x1b, y2b, x2b, FEAT_FLOOR);


	/* Special features */
	switch (randint1(4))
	{
		/* Nothing */
		case 1:
		{
			break;
		}

		/* Large solid middle pillar */
		case 2:
		{
			/* Generate a small inner solid pillar */
			generate_fill(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			break;
		}

		/* Inner treasure vault */
		case 3:
		{
			/* Generate a small inner vault */
			generate_draw(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

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
		case 4:
		{
			/* Occasionally pinch the center shut */
			if (one_in_(3))
			{
				/* Pinch the east/west sides */
				for (y = y1b; y <= y2b; y++)
				{
					if (y == y0) continue;
					cave_set_feat(c, y, x1a - 1, FEAT_WALL_INNER);
					cave_set_feat(c, y, x2a + 1, FEAT_WALL_INNER);
				}

				/* Pinch the north/south sides */
				for (x = x1a; x <= x2a; x++)
				{
					if (x == x0) continue;
					cave_set_feat(c, y1b - 1, x, FEAT_WALL_INNER);
					cave_set_feat(c, y2b + 1, x, FEAT_WALL_INNER);
				}

				/* Open sides with secret doors */
				if (one_in_(3))
				{
					generate_open(c, y1b-1, x1a-1, y2b+1, x2a+1, FEAT_SECRET);
				}
			}

			/* Occasionally put a "plus" in the center */
			else if (one_in_(3))
			{
				generate_plus(c, y1b, x1a, y2b, x2a, FEAT_WALL_INNER);
			}

			/* Occasionally put a "pillar" in the center */
			else if (one_in_(3))
			{
				cave_set_feat(c, y0, x0, FEAT_WALL_INNER);
			}

			break;
		}
	}
}


/*
 * Type 4 -- Large room with an inner room
 *
 * Possible sub-types:
 *	1 - An inner room
 *	2 - An inner room with a small inner room
 *	3 - An inner room with a pillar or pillars
 *	4 - An inner room with a checkerboard
 *	5 - An inner room with four compartments
 */
static void build_type4(struct cave *c, int y0, int x0)
{
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
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(c, y1, x1, y2, x2, FEAT_FLOOR);


	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);


	/* Inner room variations */
	switch (randint1(5))
	{
		/* An inner room */
		case 1:
		{
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Place a monster in the room */
			vault_monsters(c, y0, x0, c->depth + 2, 1);

			break;
		}


		/* An inner room with a small inner room */
		case 2:
		{
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Place another inner room */
			generate_draw(c, y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Open the inner room with a locked door */
			generate_hole(c, y0-1, x0-1, y0+1, x0+1, FEAT_DOOR_HEAD + randint1(7));

			/* Monsters to guard the treasure */
			vault_monsters(c, y0, x0, c->depth + 2, randint1(3) + 2);

			/* Object (80%) */
			if (randint0(100) < 80)
			{
				place_object(c, y0, x0, c->depth, FALSE, FALSE);
			}

			/* Stairs (20%) */
			else
			{
				place_random_stairs(c, y0, x0);
			}

			/* Traps to protect the treasure */
			vault_traps(c, y0, x0, 4, 10, 2 + randint1(3));

			break;
		}


		/* An inner room with an inner pillar or pillars */
		case 3:
		{
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Inner pillar */
			generate_fill(c, y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Occasionally, two more Large Inner Pillars */
			if (one_in_(2))
			{
				/* Three spaces */
				if (one_in_(2))
				{
					/* Inner pillar */
					generate_fill(c, y0-1, x0-7, y0+1, x0-5, FEAT_WALL_INNER);

					/* Inner pillar */
					generate_fill(c, y0-1, x0+5, y0+1, x0+7, FEAT_WALL_INNER);
				}

				/* Two spaces */
				else
				{
					/* Inner pillar */
					generate_fill(c, y0-1, x0-6, y0+1, x0-4, FEAT_WALL_INNER);

					/* Inner pillar */
					generate_fill(c, y0-1, x0+4, y0+1, x0+6, FEAT_WALL_INNER);
				}
			}

			/* Occasionally, some Inner rooms */
			if (one_in_(3))
			{
				/* Inner rectangle */
				generate_draw(c, y0-1, x0-5, y0+1, x0+5, FEAT_WALL_INNER);

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
		case 4:
		{
			/* Open the inner room with a secret door */
			generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Checkerboard */
			for (y = y1; y <= y2; y++)
			{
				for (x = x1; x <= x2; x++)
				{
					if ((x + y) & 0x01)
					{
						cave_set_feat(c, y, x, FEAT_WALL_INNER);
					}
				}
			}

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
		case 5:
		{
			/* Inner "cross" */
			generate_plus(c, y1, x1, y2, x2, FEAT_WALL_INNER);

			/* Doors into the rooms */
			if (randint0(100) < 50)
			{
				int i = randint1(10);
				place_secret_door(c, y1 - 1, x0 - i);
				place_secret_door(c, y1 - 1, x0 + i);
				place_secret_door(c, y2 + 1, x0 - i);
				place_secret_door(c, y2 + 1, x0 + i);
			}
			else
			{
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
}


/*
 * The following functions are used to determine if the given monster
 * is appropriate for inclusion in a monster nest or monster pit or
 * the given type.
 *
 * None of the pits/nests are allowed to include "unique" monsters,
 * or monsters which can "multiply".
 *
 * Some of the pits/nests are asked to avoid monsters which can blink
 * away or which are invisible.  This is probably a hack.
 *
 * The old method used monster "names", which was bad, but the new
 * method uses monster race characters, which is also bad.  XXX XXX XXX
 */


/*
 * Helper function for "monster nest (jelly)"
 */
static bool vault_aux_jelly(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Require icky thing, jelly, mold, or mushroom */
	if (!strchr("ijm,", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (animal)"
 */
static bool vault_aux_animal(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Require "animal" flag */
	if (!rf_has(r_ptr->flags, RF_ANIMAL)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (undead)"
 */
static bool vault_aux_undead(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Require Undead */
	if (!rf_has(r_ptr->flags, RF_UNDEAD)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (orc)"
 */
static bool vault_aux_orc(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Hack -- Require "o" monsters */
	if (!strchr("o", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (troll)"
 */
static bool vault_aux_troll(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Hack -- Require "T" monsters */
	if (!strchr("T", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (giant)"
 */
static bool vault_aux_giant(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Hack -- Require "P" monsters */
	if (!strchr("P", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- breath type for "vault_aux_dragon()"
 */
static bitflag vault_aux_dragon_mask[RSF_SIZE];


/*
 * Helper function for "monster pit (dragon)"
 */
static bool vault_aux_dragon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];
	bitflag mon_breath[RSF_SIZE];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Hack -- Require "d" or "D" monsters */
	if (!strchr("Dd", r_ptr->d_char)) return (FALSE);

	/* Hack -- Require correct "breath attack" */
	rsf_copy(mon_breath, r_ptr->spell_flags);
	flags_mask(mon_breath, RSF_SIZE, RSF_BREATH_MASK, FLAG_END);

	if (!rsf_is_equal(mon_breath, vault_aux_dragon_mask)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (demon)"
 */
static bool vault_aux_demon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Hack -- Require "U" monsters */
	if (!strchr("U", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}



/*
 * Type 5 -- Monster nests
 *
 * A monster nest is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type strewn about the room.
 *
 * The monsters are chosen from a set of 64 randomly selected monster
 * races, to allow the nest creation to fail instead of having "holes".
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the nest.
 *
 * Currently, a monster nest is one of
 *   a nest of "jelly" monsters   (Dungeon level 5 and deeper)
 *   a nest of "animal" monsters  (Dungeon level 30 and deeper)
 *   a nest of "undead" monsters  (Dungeon level 50 and deeper)
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which
 * case the nest will be empty, and will not affect the level rating.
 *
 * Note that "monster nests" will never contain "unique" monsters.
 */
static void build_type5(struct cave *c, int y0, int x0)
{
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
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(c, y1, x1, y2, x2, FEAT_FLOOR);


	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);


	/* Hack -- Choose a nest type */
	tmp = randint1(c->depth);

	/* Monster nest (jelly) */
	if (tmp < 30)
	{
		/* Describe */
		name = "jelly";

		/* Restrict to jelly */
		get_mon_num_hook = vault_aux_jelly;

		/* Get treasure probability */
		alloc_obj = JELLY_NEST_OBJ;
	}

	/* Monster nest (animal) */
	else if (tmp < 50)
	{
		/* Describe */
		name = "animal";

		/* Restrict to animal */
		get_mon_num_hook = vault_aux_animal;

		/* Get treasure probability */
		alloc_obj = ANIMAL_NEST_OBJ;
	}

	/* Monster nest (undead) */
	else
	{
		/* Describe */
		name = "undead";

		/* Restrict to undead */
		get_mon_num_hook = vault_aux_undead;

		/* Get treasure probability */
		alloc_obj = UNDEAD_NEST_OBJ;
	}

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick some monster types */
	for (i = 0; i < 64; i++)
	{
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
	if (empty) return;


	/* Describe */
	if (OPT(cheat_room))
	{
		/* Room type */
		msg("Monster nest (%s)", name);
	}


	/* Increase the level rating */
	c->rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Nests") */
	if ((c->depth <= 40) &&
	    (randint1(c->depth * c->depth + 1) < 300))
	{
		c->good_item = TRUE;
	}


	/* Place some monsters */
	for (y = y0 - 2; y <= y0 + 2; y++)
	{
		for (x = x0 - 9; x <= x0 + 9; x++)
		{
			int r_idx = what[randint0(64)];

			/* Place that "random" monster (no groups) */
			(void)place_monster_aux(c, y, x, r_idx, FALSE, FALSE);

			/* Occasionally place an item, making it good 1/3 of the time */
			if (one_in_(alloc_obj)) 
				place_object(c, y, x, c->depth + 10, one_in_(3), FALSE);
		}
	}
}



/*
 * Type 6 -- Monster pits
 *
 * A monster pit is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type organized in the room.
 *
 * Monster types in the pit
 *   orc pit	(Dungeon Level 5 and deeper)
 *   troll pit	(Dungeon Level 20 and deeper)
 *   giant pit	(Dungeon Level 40 and deeper)
 *   dragon pit	(Dungeon Level 60 and deeper)
 *   demon pit	(Dungeon Level 80 and deeper)
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
 * Note that the monsters in the pit are now chosen by using "get_mon_num()"
 * to request 16 "appropriate" monsters, sorting them by level, and using
 * the "even" entries in this sorted list for the contents of the pit.
 *
 * Hack -- all of the "dragons" in a "dragon" pit must be the same "color",
 * which is handled by requiring a specific "breath" attack for all of the
 * dragons.  This may include "multi-hued" breath.  Note that "wyrms" may
 * be present in many of the dragon pits, if they have the proper breath.
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the pit.
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which case
 * the pit will be empty, and will not effect the level rating.
 *
 * Note that "monster pits" will never contain "unique" monsters.
 */
static void build_type6(struct cave *c, int y0, int x0)
{
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


	/* Generate new room */
	generate_room(c, y1-1, x1-1, y2+1, x2+1, light);

	/* Generate outer walls */
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(c, y1, x1, y2, x2, FEAT_FLOOR);


	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_draw(c, y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(c, y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);


	/* Choose a pit type */
	tmp = randint1(c->depth);

	/* Orc pit */
	if (tmp < 20)
	{
		/* Message */
		name = "orc";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_orc;
	}

	/* Troll pit */
	else if (tmp < 40)
	{
		/* Message */
		name = "troll";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_troll;
	}

	/* Giant pit */
	else if (tmp < 60)
	{
		/* Message */
		name = "giant";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_giant;
	}

	/* Dragon pit */
	else if (tmp < 80)
	{
		/* Pick dragon type */
		switch (randint0(6))
		{
			/* Black */
			case 0:
			{
				/* Message */
				name = "acid dragon";

				/* Restrict dragon breath type */
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_ACID, FLAG_END);

				/* Done */
				break;
			}

			/* Blue */
			case 1:
			{
				/* Message */
				name = "electric dragon";

				/* Restrict dragon breath type */
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_ELEC, FLAG_END);

				/* Done */
				break;
			}

			/* Red */
			case 2:
			{
				/* Message */
				name = "fire dragon";

				/* Restrict dragon breath type */
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_FIRE, FLAG_END);

				/* Done */
				break;
			}

			/* White */
			case 3:
			{
				/* Message */
				name = "cold dragon";

				/* Restrict dragon breath type */
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_COLD, FLAG_END);

				/* Done */
				break;
			}

			/* Green */
			case 4:
			{
				/* Message */
				name = "poison dragon";

				/* Restrict dragon breath type */
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_POIS, FLAG_END);

				/* Done */
				break;
			}

			/* Multi-hued */
			default:
			{
				/* Message */
				name = "multi-hued dragon";

				/* Restrict dragon breath type */
				flags_init(vault_aux_dragon_mask, RSF_SIZE, RSF_BR_ACID, RSF_BR_ELEC,
				           RSF_BR_FIRE, RSF_BR_COLD, RSF_BR_POIS, FLAG_END);

				/* Done */
				break;
			}

		}

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_dragon;
	}

	/* Demon pit */
	else
	{
		/* Message */
		name = "demon";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_demon;
	}

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick some monster types */
	for (i = 0; i < 16; i++)
	{
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
	if (empty) return;


	/* Sort the entries XXX XXX XXX */
	for (i = 0; i < 16 - 1; i++)
	{
		/* Sort the entries */
		for (j = 0; j < 16 - 1; j++)
		{
			int i1 = j;
			int i2 = j + 1;

			int p1 = r_info[what[i1]].level;
			int p2 = r_info[what[i2]].level;

			/* Bubble */
			if (p1 > p2)
			{
				int tmp = what[i1];
				what[i1] = what[i2];
				what[i2] = tmp;
			}
		}
	}

	/* Select the entries */
	for (i = 0; i < 8; i++)
	{
		/* Every other entry */
		what[i] = what[i * 2];
	}


	/* Message */
	if (OPT(cheat_room))
	{
		/* Room type */
		msg("Monster pit (%s)", name);
	}


	/* Increase the level rating */
	c->rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
	if ((c->depth <= 40) &&
	    (randint1(c->depth * c->depth + 1) < 300))
	{
		c->good_item = TRUE;
	}


	/* Top and bottom rows */
	for (x = x0 - 9; x <= x0 + 9; x++)
	{
		place_monster_aux(c, y0 - 2, x, what[0], FALSE, FALSE);
		place_monster_aux(c, y0 + 2, x, what[0], FALSE, FALSE);
	}

	/* Middle columns */
	for (y = y0 - 1; y <= y0 + 1; y++)
	{
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
	for (x = x0 - 1; x <= x0 + 1; x++)
	{
		place_monster_aux(c, y0 + 1, x, what[5], FALSE, FALSE);
		place_monster_aux(c, y0 - 1, x, what[5], FALSE, FALSE);
	}

	/* Next to the center monster */
	place_monster_aux(c, y0, x0 + 1, what[6], FALSE, FALSE);
	place_monster_aux(c, y0, x0 - 1, what[6], FALSE, FALSE);

	/* Center monster */
	place_monster_aux(c, y0, x0, what[7], FALSE, FALSE);
}



/*
 * Hack -- fill in "vault" rooms
 */
static void build_vault(struct cave *c, int y0, int x0, int ymax, int xmax, cptr data)
{
	int dx, dy, x, y;
	cptr t;

	assert(c);

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the location */
			x = x0 - (xmax / 2) + dx;
			y = y0 - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Lay down a floor */
			cave_set_feat(c, y, x, FEAT_FLOOR);

			/* Part of a vault */
			c->info[y][x] |= (CAVE_ROOM | CAVE_ICKY);

			/* Analyze the grid */
			switch (*t)
			{
				/* Granite wall (outer) */
				case '%':
				{
					cave_set_feat(c, y, x, FEAT_WALL_OUTER);
					break;
				}

				/* Granite wall (inner) */
				case '#':
				{
					cave_set_feat(c, y, x, FEAT_WALL_INNER);
					break;
				}

				/* Permanent wall (inner) */
				case 'X':
				{
					cave_set_feat(c, y, x, FEAT_PERM_INNER);
					break;
				}

				/* Treasure/trap */
				case '*':
				{
					if (randint0(100) < 75)
					{
						place_object(c, y, x, c->depth, FALSE, FALSE);
					}
					else
					{
						place_trap(c, y, x);
					}
					break;
				}

				/* Secret doors */
				case '+':
				{
					place_secret_door(c, y, x);
					break;
				}

				/* Trap */
				case '^':
				{
					place_trap(c, y, x);
					break;
				}
			}
		}
	}


	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the grid */
			x = x0 - (xmax/2) + dx;
			y = y0 - (ymax/2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the symbol */
			switch (*t)
			{
				/* Monster */
				case '&':
				{
					place_monster(c, y, x, c->depth + 5, TRUE, TRUE);
					break;
				}

				/* Meaner monster */
				case '@':
				{
					place_monster(c, y, x, c->depth + 11, TRUE, TRUE);
					break;
				}

				/* Meaner monster, plus treasure */
				case '9':
				{
					place_monster(c, y, x, c->depth + 9, TRUE, TRUE);
					place_object(c, y, x, c->depth + 7, TRUE, FALSE);
					break;
				}

				/* Nasty monster and treasure */
				case '8':
				{
					place_monster(c, y, x, c->depth + 40, TRUE, TRUE);
					place_object(c, y, x, c->depth + 20, TRUE, FALSE);
					break;
				}

				/* Monster and/or object */
				case ',':
				{
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

/* Chooses a vault at random; each vault has equal probability of being chosen.
 * Inductive proof of correctness:
 * Base case: n = 1: The only vault is chosen with probability 1/1, so we are
 * done.
 * Inductive step:
 *   Assume that if there are n vaults, each is chosen with probability 1/n. Let
 *   this set of n vaults be V_1 ... V_n.
 *   Add a new vault V_{n + 1}.
 *   At step n + 1, we choose a member of V_n (i.e., leave the selection the
 *   same) with probability n / n + 1, and choose V_{n + 1} (i.e., choose the
 *   new vault) with probability 1 / n + 1.
 *   Let V_i \in { V_1, ..., V_n }. By the inductive hypothesis, V_i is chosen
 *   with probability 1 / n, and we left the selection the same with probability
 *   n / n + 1, so the probability that V_i is chosen is now 1 / n + 1.
 *   Therefore, each vault of V_1 ... V_{n + 1} is chosen with uniform
 *   probability 1 / n + 1.
 */
struct vault *random_vault(void) {
	struct vault *v, *r = NULL;
	int n;
	for (v = vaults, n = 1; v; v = v->next, n++)
		if (one_in_(n))
			r = v;
	return r;
}

/*
 * Type 7 -- simple vaults (see "vault.txt")
 */
static void build_type7(struct cave *c, int y0, int x0)
{
	vault_type *v_ptr;

	/* Pick a lesser vault */
	while (TRUE) {
		v_ptr = random_vault();
		if (v_ptr->typ == 6) break;
	}

	/* Message */
	if (OPT(cheat_room)) msg("Lesser vault (%s)", v_ptr->name);

	/* Boost the rating */
	c->rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((c->depth <= 50) ||
	    (randint1((c->depth-40) * (c->depth-40) + 1) < 400))
	{
		c->good_item = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(c, y0, x0, v_ptr->hgt, v_ptr->wid, v_ptr->text);
}



/*
 * Type 8 -- medium vaults (see "vault.txt")
 */
static void build_type8(struct cave *c, int y0, int x0)
{
	vault_type *v_ptr;

	/* Pick a medium vault */
	while (TRUE) {
		v_ptr = random_vault();
		if (v_ptr->typ == 7) break;
	}

	/* Message */
	if (OPT(cheat_room)) msg("Medium vault (%s)", v_ptr->name);

	/* Boost the rating */
	c->rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((c->depth <= 50) ||
	    (randint1((c->depth-40) * (c->depth-40) + 1) < 400))
	{
		c->good_item = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(c, y0, x0, v_ptr->hgt, v_ptr->wid, v_ptr->text);
}

/*
 * Type 9 -- greater vaults (see "vault.txt")
 */
static void build_type9(struct cave *c, int y0, int x0)
{
	vault_type *v_ptr;

	/* Pick a greater vault */
	while (TRUE) {
		v_ptr = random_vault();
		if (v_ptr->typ == 8) break;
	}

	/* Message */
	if (OPT(cheat_room)) msg("Greater vault (%s)", v_ptr->name);

	/* Boost the rating */
	c->rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((c->depth <= 50) ||
	    (randint1((c->depth-40) * (c->depth-40) + 1) < 400))
	{
		c->good_item = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(c, y0, x0, v_ptr->hgt, v_ptr->wid, v_ptr->text);
}

/*
 * Constructs a tunnel between two points
 *
 * This function must be called BEFORE any streamers are created,
 * since we use the special "granite wall" sub-types to keep track
 * of legal places for corridors to pierce rooms.
 *
 * We use "door_flag" to prevent excessive construction of doors
 * along overlapping corridors.
 *
 * We queue the tunnel grids to prevent door creation along a corridor
 * which intersects itself.
 *
 * We queue the wall piercing grids to prevent a corridor from leaving
 * a room and then coming back in through the same entrance.
 *
 * We "pierce" grids which are "outer" walls of rooms, and when we
 * do so, we change all adjacent "outer" walls of rooms into "solid"
 * walls so that no two corridors may use adjacent grids for exits.
 *
 * The "solid" wall check prevents corridors from "chopping" the
 * corners of rooms off, as well as "silly" door placement, and
 * "excessively wide" room entrances.
 *
 * Useful "feat" values:
 *   FEAT_WALL_EXTRA -- granite walls
 *   FEAT_WALL_INNER -- inner room walls
 *   FEAT_WALL_OUTER -- outer room walls
 *   FEAT_WALL_SOLID -- solid room walls
 *   FEAT_PERM_EXTRA -- shop walls (perma)
 *   FEAT_PERM_INNER -- inner room walls (perma)
 *   FEAT_PERM_OUTER -- outer room walls (perma)
 *   FEAT_PERM_SOLID -- dungeon border (perma)
 */
static void build_tunnel(struct cave *c, int row1, int col1, int row2, int col2)
{
	int i, y, x;
	int tmp_row, tmp_col;
	int row_dir, col_dir;
	int start_row, start_col;
	int main_loop_count = 0;

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
	while ((row1 != row2) || (col1 != col2))
	{
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) break;

		/* Allow bends in the tunnel */
		if (randint0(100) < DUN_TUN_CHG)
		{
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (randint0(100) < DUN_TUN_RND)
			{
				rand_dir(&row_dir, &col_dir);
			}
		}

		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;

		while (!in_bounds_fully(tmp_row, tmp_col))
		{
			/* Get the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (randint0(100) < DUN_TUN_RND)
			{
				rand_dir(&row_dir, &col_dir);
			}

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
		if (c->feat[tmp_row][tmp_col] == FEAT_WALL_OUTER)
		{
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
			if (dun->wall_n < WALL_MAX)
			{
				dun->wall[dun->wall_n].y = row1;
				dun->wall[dun->wall_n].x = col1;
				dun->wall_n++;
			}

			/* Forbid re-entry near this piercing */
			for (y = row1 - 1; y <= row1 + 1; y++)
			{
				for (x = col1 - 1; x <= col1 + 1; x++)
				{
					/* Convert adjacent "outer" walls as "solid" walls */
					if (c->feat[y][x] == FEAT_WALL_OUTER)
					{
						/* Change the wall to a "solid" wall */
						cave_set_feat(c, y, x, FEAT_WALL_SOLID);
					}
				}
			}
		}

		/* Travel quickly through rooms */
		else if (c->info[tmp_row][tmp_col] & (CAVE_ROOM))
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;
		}

		/* Tunnel through all other walls */
		else if (c->feat[tmp_row][tmp_col] >= FEAT_WALL_EXTRA)
		{
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX)
			{
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			door_flag = FALSE;
		}

		/* Handle corridor intersections or overlaps */
		else
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Collect legal door locations */
			if (!door_flag)
			{
				/* Save the door location */
				if (dun->door_n < DOOR_MAX)
				{
					dun->door[dun->door_n].y = row1;
					dun->door[dun->door_n].x = col1;
					dun->door_n++;
				}

				/* No door in next grid */
				door_flag = TRUE;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if (randint0(100) >= DUN_TUN_CON)
			{
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
	for (i = 0; i < dun->tunn_n; i++)
	{
		/* Get the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Clear previous contents, add a floor */
		cave_set_feat(c, y, x, FEAT_FLOOR);
	}


	/* Apply the piercings that we found */
	for (i = 0; i < dun->wall_n; i++)
	{
		/* Get the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Convert to floor grid */
		cave_set_feat(c, y, x, FEAT_FLOOR);

		/* Occasional doorway */
		if (randint0(100) < DUN_TUN_PEN)
		{
			/* Place a random door */
			place_random_door(c, y, x);
		}
	}
}

/*
 * Count the number of "corridor" grids adjacent to the given grid.
 *
 * This routine currently only counts actual "empty floor" grids
 * which are not in rooms.  We might want to also count stairs,
 * open doors, closed doors, etc.  XXX XXX
 */
static int next_to_corr(struct cave *c, int y1, int x1)
{
	int i, y, x, k = 0;

	assert(cave_in_bounds_fully(c, y1, x1));

	/* Scan adjacent grids */
	for (i = 0; i < 4; i++)
	{
		/* Extract the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Skip non floors */
		if (!cave_isfloor(c, y, x)) continue;

		/* Skip non "empty floor" grids */
		if (c->feat[y][x] != FEAT_FLOOR) continue;

		/* Skip grids inside rooms */
		if (c->info[y][x] & CAVE_ROOM) continue;

		/* Count these grids */
		k++;
	}

	/* Return the number of corridors */
	return k;
}

/* Returns whether a doorway can be built in a space. To have a doorway, a space
 * must be adjacent to at least two corridors and be between two walls.
 */
static bool possible_doorway(struct cave *c, int y, int x)
{
	assert(cave_in_bounds_fully(c, y, x));

	if (next_to_corr(c, y, x) < 2)
		return FALSE;

	/* Check Vertical */
	if ((c->feat[y-1][x] >= FEAT_MAGMA) &&
	    (c->feat[y+1][x] >= FEAT_MAGMA))
		return TRUE;

	/* Check Horizontal */
	if ((c->feat[y][x-1] >= FEAT_MAGMA) &&
	    (c->feat[y][x+1] >= FEAT_MAGMA))
		return TRUE;

	return FALSE;
}


/*
 * Places door at y, x position if at least 2 walls found
 */
static void try_door(struct cave *c, int y, int x)
{
	assert(cave_in_bounds(c, y, x));

	/* Ignore walls */
	if (c->feat[y][x] >= FEAT_MAGMA)
		return;

	/* Ignore room grids */
	if (c->info[y][x] & CAVE_ROOM)
		return;

	/* Occasional door (if allowed) */
	if ((randint0(100) < DUN_TUN_JCT) && possible_doorway(c, y, x))
		place_random_door(c, y, x);
}




/*
 * Attempt to build a room of the given type at the given block
 *
 * Note that we restrict the number of "crowded" rooms to reduce
 * the chance of overflowing the monster list during level creation.
 */
static bool room_build(struct cave *c, int by0, int bx0, int typ)
{
	int y, x;
	int by, bx;
	int by1, bx1, by2, bx2;

	assert(typ > 0 && typ <= 8);

	/* Restrict level */
	if (c->depth < room[typ].level) return (FALSE);

	/* Restrict "crowded" rooms */
	if (dun->crowded && ((typ == 5) || (typ == 6))) return (FALSE);

	/* Extract blocks */
	by1 = by0 + room[typ].dy1;
	bx1 = bx0 + room[typ].dx1;
	by2 = by0 + room[typ].dy2;
	bx2 = bx0 + room[typ].dx2;

	/* Never run off the screen */
	if ((by1 < 0) || (by2 >= dun->row_rooms)) return (FALSE);
	if ((bx1 < 0) || (bx2 >= dun->col_rooms)) return (FALSE);

	/* Verify open space */
	for (by = by1; by <= by2; by++)
	{
		for (bx = bx1; bx <= bx2; bx++)
		{
			if (dun->room_map[by][bx]) return (FALSE);
		}
	}

	/* Get the location of the room */
	y = ((by1 + by2 + 1) * BLOCK_HGT) / 2;
	x = ((bx1 + bx2 + 1) * BLOCK_WID) / 2;

	/* Build a room */
	switch (typ)
	{
		/* Build an appropriate room */
		case 9: build_type9(c, y, x); break;
		case 8: build_type8(c, y, x); break;
		case 7: build_type7(c, y, x); break;
		case 6: build_type6(c, y, x); break;
		case 5: build_type5(c, y, x); break;
		case 4: build_type4(c, y, x); break;
		case 3: build_type3(c, y, x); break;
		case 2: build_type2(c, y, x); break;
		case 1: build_type1(c, y, x); break;
	}

	/* Save the room location */
	if (dun->cent_n < CENT_MAX)
	{
		dun->cent[dun->cent_n].y = y;
		dun->cent[dun->cent_n].x = x;
		dun->cent_n++;
	}

	/* Reserve some blocks */
	for (by = by1; by <= by2; by++)
	{
		for (bx = bx1; bx <= bx2; bx++)
		{
			dun->room_map[by][bx] = TRUE;
		}
	}

	/* Count "crowded" rooms */
	if ((typ == 5) || (typ == 6)) dun->crowded = TRUE;

	/* Success */
	return TRUE;
}


/** @brief Generate a new dungeon level
 *  XXX: Needs refactoring.
 */
static void cave_gen(struct cave *c, struct player *p)
{
	int i, j, k, l, y, x, y1, x1;
	int by, bx;
	int num_rooms, size_percent;

	bool blocks_tried[MAX_ROOMS_ROW][MAX_ROOMS_COL];

	struct dun_data dun_body;

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
	num_rooms = DUN_ROOMS * size_percent / 100;
	c->height = DUNGEON_HGT * size_percent / 100;
	c->width  = DUNGEON_WID * size_percent / 100;

	/* Global data */
	dun = &dun_body;

	/* Hack -- Start with basic granite */
	for (y = 0; y < DUNGEON_HGT; y++)
		for (x = 0; x < DUNGEON_WID; x++)
			cave_set_feat(c, y, x, FEAT_WALL_EXTRA);

	/* Actual maximum number of rooms on this level */
	dun->row_rooms = c->height / BLOCK_HGT;
	dun->col_rooms = c->width / BLOCK_WID;

	/* Initialize the room table */
	for (by = 0; by < dun->row_rooms; by++)
		for (bx = 0; bx < dun->col_rooms; bx++)
			dun->room_map[by][bx] = blocks_tried[by][bx]  = FALSE;

	/* No "crowded" rooms yet */
	dun->crowded = FALSE;


	/* No rooms yet */
	dun->cent_n = 0;

	/* Build some rooms */
	i = 0;
	while(i < num_rooms)
	{
		i++;

		/* Pick a block for the room; j counts blocks we haven't tried */
		j = 0;
		for(by=0; by < dun->row_rooms; by++)
			for(bx=0; bx < dun->col_rooms; bx++)
				if (!blocks_tried[by][bx]) j++;

		/* If we've tried all blocks we're done */
		if (j == 0) break;

		/* OK, choose one of the j blocks we haven't tried. Then figure out */
		/* which one that actually was */
		k = randint0(j);
		l = 0;
		for(by=0; by < dun->row_rooms; by++)
		{
			for(bx=0; bx < dun->col_rooms; bx++)
			{
				if (blocks_tried[by][bx]) continue;
				if (l == k) break;
				l++;
			}
			if (l == k) break;
		}
		
		blocks_tried[by][bx] = TRUE;

		/* Move GV vault creation to the beginning */
		/* There are two problems with GV creation, overlapping other rooms, 
		 * and running off the boundaries.  Moving GV creation to the beginning 
		 * automatically solves the overlapping problem, but makes
		 * no claims that the block picked will fit on the map.  
		 * The dungeon is 6 blocks tall and 18 wide and the size of a GV is 4 tall
		 * and 6 wide.  That means a GV will fall out of bounds 63% of the time!
		 * The following code should therefore make a greater vault frequencies like:
		 * Dlevel         GV frequency
		 * 100		18%
		 * 90-99		16-18%
		 * 80-89		10 -11%
		 * 70-79		5.7 - 6.5%
		 * 60-69		3.3 - 3.8%
		 * 50-59		1.8 - 2.1%
		 * and less than 1% below 50 */
		
		/* Only attempt a GV if you are on the first room */ 
		if (i == 1 && randint0(DUN_UNUSUAL) < c->depth)
		{
			int i;
			int numerator   = 2;
			int denominator = 3;

			/* For building greater vaults, we make a check based on depth:
			 * At level 90 and above, you have a 2/3 chance of trying to build
			 * a GV. At levels 80-89 you have a 4/9 chance, and so on... */
			for(i = 90; i > c->depth; i -= 10)
			{
				numerator *= 2;
				denominator *= 3;
			}

			/* Attempt to pass the depth check and build a GV */
			if (randint0(denominator) < numerator && room_build(c, by, bx, 9))
				continue;
		}

		/* Attempt an "unusual" room */
		if (randint0(DUN_UNUSUAL) < c->depth)
		{
			/* Roll for room type */
			k = randint0(100);

			/* Attempt a very unusual room */
			if (randint0(DUN_UNUSUAL) < c->depth)
			{
				/* Type 8 -- Medium vault (10%) */
				if ((k < 10) && room_build(c, by, bx, 8)) continue;

				/* Type 7 -- Lesser vault (15%) */
				if ((k < 25) && room_build(c, by, bx, 7)) continue;

				/* Type 6 -- Monster pit (15%) */
				if ((k < 40) && room_build(c, by, bx, 6)) continue;

				/* Type 5 -- Monster nest (10%) */
				if ((k < 50) && room_build(c, by, bx, 5)) continue;
			}

			/* Type 4 -- Large room (25%) */
			if ((k < 25) && room_build(c, by, bx, 4)) continue;

			/* Type 3 -- Cross room (25%) */
			if ((k < 50) && room_build(c, by, bx, 3)) continue;

			/* Type 2 -- Overlapping (50%) */
			if ((k < 100) && room_build(c, by, bx, 2)) continue;
		}

		/* Attempt a trivial room */
		if (room_build(c, by, bx, 1)) continue;
	}

	/* Special boundary walls -- Bottom */
	for (x = 0; x < DUNGEON_WID; x++)
	{
		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(c, 0, x, FEAT_PERM_SOLID);
		cave_set_feat(c, DUNGEON_HGT - 1, x, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- Left */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(c, y, 0, FEAT_PERM_SOLID);
		cave_set_feat(c, y, DUNGEON_WID - 1, FEAT_PERM_SOLID);
	}

	/* Hack -- Scramble the room order */
	for (i = 0; i < dun->cent_n; i++)
	{
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
	for (i = 0; i < dun->cent_n; i++)
	{
		/* Connect the room to the previous room */
		build_tunnel(c, dun->cent[i].y, dun->cent[i].x, y, x);

		/* Remember the "previous" room */
		y = dun->cent[i].y;
		x = dun->cent[i].x;
	}

	/* Place intersection doors */
	for (i = 0; i < dun->door_n; i++)
	{
		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try placing doors */
		try_door(c, y, x - 1);
		try_door(c, y, x + 1);
		try_door(c, y - 1, x);
		try_door(c, y + 1, x);
	}


	/* Hack -- Add some magma streamers */
	for (i = 0; i < DUN_STR_MAG; i++)
	{
		build_streamer(c, FEAT_MAGMA, DUN_STR_MC);
	}

	/* Hack -- Add some quartz streamers */
	for (i = 0; i < DUN_STR_QUA; i++)
	{
		build_streamer(c, FEAT_QUARTZ, DUN_STR_QC);
	}


	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(c, FEAT_MORE, rand_range(3, 4), 3);

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(c, FEAT_LESS, rand_range(1, 2), 3);


	/* Basic "amount" */
	k = c->depth / 3;
	if (k > 10) k = 10;
	if (k < 2) k = 2;

	/* Put some rubble in corridors */
	alloc_object(c, ALLOC_SET_CORR, ALLOC_TYP_RUBBLE, randint1(k), c->depth);

	/* Place some traps in the dungeon */
	alloc_object(c, ALLOC_SET_BOTH, ALLOC_TYP_TRAP, randint1(k), c->depth);

	/* Determine the character location */
	new_player_spot(c, p);

	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint1(8);

	/* Put some monsters in the dungeon */
	for (i = i + k; i > 0; i--)
	{
		alloc_monster(c, loc(p->px, p->py), 0, TRUE, c->depth);
	}

	/* Ensure quest monsters */
	if (is_quest(c->depth))
	{
		/* Ensure quest monsters */
		for (i = 1; i < z_info->r_max; i++)
		{
			monster_race *r_ptr = &r_info[i];

			/* Ensure quest monsters */
			if (rf_has(r_ptr->flags, RF_QUESTOR) &&
			    r_ptr->level == c->depth &&
			    r_ptr->cur_num <= 0)
			{
				int y, x;

				/* Pick a location */
				while (1)
				{
					y = randint0(c->height);
					x = randint0(c->width);

					if (cave_isempty(c, y, x)) break;
				}

				/* Place the questor */
				place_monster_aux(c, y, x, i, TRUE, TRUE);
			}
		}
	}


	/* Put some objects in rooms */
	alloc_object(c, ALLOC_SET_ROOM,
	             ALLOC_TYP_OBJECT, Rand_normal(DUN_AMT_ROOM, 3),
	             c->depth);

	/* Put some objects/gold in the dungeon */
	alloc_object(c, ALLOC_SET_BOTH,
	             ALLOC_TYP_OBJECT, Rand_normal(DUN_AMT_ITEM, 3),
	             c->depth);
	alloc_object(c, ALLOC_SET_BOTH,
	             ALLOC_TYP_GOLD, Rand_normal(DUN_AMT_GOLD, 3),
	             c->depth);
}



/*
 * Builds a store at a given pseudo-location
 *
 * As of 2.7.4 (?) the stores are placed in a more "user friendly"
 * configuration, such that the four "center" buildings always
 * have at least four grids between them, to allow easy running,
 * and the store doors tend to face the middle of town.
 *
 * The stores now lie inside boxes from 3-9 and 12-18 vertically,
 * and from 7-17, 21-31, 35-45, 49-59.  Note that there are thus
 * always at least 2 open grids between any disconnected walls.
 *
 * Note the use of "town_illuminate()" to handle all "illumination"
 * and "memorization" issues.
 */
static void build_store(struct cave *c, int n, int yy, int xx)
{
	int y, x, y0, x0, y1, x1, y2, x2, tmp;


	/* Find the "center" of the store */
	y0 = yy * 9 + 6;
	x0 = xx * 14 + 12;

	/* Determine the store boundaries */
	y1 = y0 - randint1((yy == 0) ? 3 : 2);
	y2 = y0 + randint1((yy == 1) ? 3 : 2);
	x1 = x0 - randint1(5);
	x2 = x0 + randint1(5);

	/* Build an invulnerable rectangular building */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Create the building */
			cave_set_feat(c, y, x, FEAT_PERM_EXTRA);
		}
	}

	/* Pick a door direction (S,N,E,W) */
	tmp = randint0(4);

	/* Re-roll "annoying" doors */
	if (((tmp == 0) && (yy == 1)) ||
	    ((tmp == 1) && (yy == 0)) ||
	    ((tmp == 2) && (xx == 3)) ||
	    ((tmp == 3) && (xx == 0)))
	{
		/* Pick a new direction */
		tmp = randint0(4);
	}

	/* Extract a "door location" */
	switch (tmp)
	{
		/* Bottom side */
		case 0:
		{
			y = y2;
			x = rand_range(x1, x2);
			break;
		}

		/* Top side */
		case 1:
		{
			y = y1;
			x = rand_range(x1, x2);
			break;
		}

		/* Right side */
		case 2:
		{
			y = rand_range(y1, y2);
			x = x2;
			break;
		}

		/* Left side */
		default:
		{
			y = rand_range(y1, y2);
			x = x1;
			break;
		}
	}

	/* Clear previous contents, add a store door */
	cave_set_feat(c, y, x, FEAT_SHOP_HEAD + n);
}

/*
 * Generate the "consistent" town features, and place the player
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 *
 * XXX: Remove this gross hack when this piece of code is fully reentrant -
 * i.e., when all we need to do is swing a pointer to change caves, we just need
 * to generate the town once.
 */
static void town_gen_hack(struct cave *c, struct player *p)
{
	int y, x, k, n;

	int rooms[MAX_STORES];


	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant town layout */
	Rand_value = seed_town;


	/* Prepare an Array of "remaining stores", and count them */
	for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

	/* Place two rows of stores */
	for (y = 0; y < 2; y++)
	{
		/* Place four stores per row */
		for (x = 0; x < 4; x++)
		{
			/* Pick a random unplaced store */
			k = ((n <= 1) ? 0 : randint0(n));

			/* Build that store at the proper location */
			build_store(c, rooms[k], y, x);

			/* Shift the stores down, remove one store */
			rooms[k] = rooms[--n];
		}
	}


	/* Place the stairs */
	while (TRUE)
	{
		/* Pick a location at least "three" from the outer walls */
		y = rand_range(3, TOWN_HGT - 4);
		x = rand_range(3, TOWN_WID - 4);

		/* Require a "naked" floor grid */
		if (cave_isempty(c, y, x)) break;
	}

	/* Clear previous contents, add down stairs */
	cave_set_feat(c, y, x, FEAT_MORE);


	/* Place the player */
	player_place(c, p, y, x);


	/* Hack -- use the "complex" RNG */
	Rand_quick = FALSE;
}

/*
 * Town logic flow for generation of new town
 *
 * We start with a fully wiped cave of normal floors.
 *
 * Note that town_gen_hack() plays games with the R.N.G.
 *
 * This function does NOT do anything about the owners of the stores,
 * nor the contents thereof.  It only handles the physical layout.
 *
 * We place the player on the stairs at the same time we make them.
 *
 * Hack -- since the player always leaves the dungeon by the stairs,
 * he is always placed on the stairs, even if he left the dungeon via
 * word of recall or teleport level.
 */
static void town_gen(struct cave *c, struct player *p)
{
	int i, y, x;
	int residents;
	bool daytime;

	assert(c);

	c->height = TOWN_HGT;
	c->width = TOWN_WID;

	if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)) {
		daytime = TRUE;
		residents = MIN_M_ALLOC_TD;
	} else {
		daytime = FALSE;
		residents = MIN_M_ALLOC_TN;
	}

	/* Start with solid walls. We can't use c->height and c->width here
	 * because then there'll be a bunch of empty space in the level that
	 * monsters might spawn in (or teleport might take you to, or whatever).
	 * XXX: fix this to use c->height and c->width when all the 'choose
	 * random location' things honor them.
	 */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Create "solid" perma-wall */
			cave_set_feat(c, y, x, FEAT_PERM_SOLID);
		}
	}

	/* Then place some floors */
	for (y = 1; y < c->height - 1; y++)
	{
		for (x = 1; x < c->width - 1; x++)
		{
			/* Create empty floor */
			cave_set_feat(c, y, x, FEAT_FLOOR);
		}
	}

	/* Build stuff */
	town_gen_hack(c, p);

	/* Apply illumination */
	cave_illuminate(c, daytime);

	/* Make some residents */
	for (i = 0; i < residents; i++)
		alloc_monster(c, loc(p->px, p->py), 3, TRUE, c->depth);
}

/*
 * Clear the dungeon, ready for generation to begin.
 */
static void cave_clear(struct cave *c, struct player *p)
{
	int x, y;

	wipe_o_list(c);
	wipe_mon_list(c, p);

	/* Clear flags and flow information. */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* No features */
			c->feat[y][x] = 0;

			/* No flags */
			c->info[y][x] = 0;
			c->info2[y][x] = 0;

			/* No flow */
			c->cost[y][x] = 0;
			c->when[y][x] = 0;

			/* Clear any left-over monsters (should be none) and the player. */
			c->m_idx[y][x] = 0;
		}
	}

	/* Mega-Hack -- no player in dungeon yet */
	p->px = p->py = 0;

	/* Hack -- illegal panel */
#ifdef WTF
	Term->offset_y = DUNGEON_HGT;
	Term->offset_x = DUNGEON_WID;
#endif

	/* Nothing special here yet */
	c->good_item = FALSE;

	/* Nothing good here yet */
	c->rating = 0;
}

/*
 * Calculate the level feeling, using a "rating" and the player's depth.
 */
static int calculate_feeling(struct cave *c)
{
	int feeling;

	/* Town gets no feeling */
	if (c->depth == 0) return 0;

	/* Extract the feeling */
	if      (c->rating > 50 +     c->depth    ) feeling = 2;
	else if (c->rating > 40 + 4 * c->depth / 5) feeling = 3;
	else if (c->rating > 30 + 3 * c->depth / 5) feeling = 4;
	else if (c->rating > 20 + 2 * c->depth / 5) feeling = 5;
	else if (c->rating > 15 + 1 * c->depth / 3) feeling = 6;
	else if (c->rating > 10 + 1 * c->depth / 5) feeling = 7;
	else if (c->rating >  5 + 1 * c->depth /10) feeling = 8;
	else if (c->rating >  0) feeling = 9;
	else feeling = 10;

	/* Hack -- Have a special feeling sometimes */
	if (c->good_item && OPT(birth_no_preserve)) feeling = 1;

	return feeling;
}

/*
 * Generate a random dungeon level
 *
 * Hack -- regenerate any "overflow" levels
 *
 * Hack -- allow auto-scumming via a gameplay option.
 */
void cave_generate(struct cave *c, struct player *p)
{
	const char *error = "no generation";
	int counter = 0;

	assert(c);

	c->depth = p->depth;

	/* Generate */
	while (error)
	{
		error = NULL;
		cave_clear(c, p);

		/* The dungeon is not ready - we set this after calling clear_
		   cave for the first time, so that unpreserved artifacts are
		   lost when leaving real levels, but not when abandoning
		   levels through errors in generation - see wipe_o_list in
		   obj-util.c */
		character_dungeon = FALSE;

		if (!p->depth)
			town_gen(c, p);
		else
			cave_gen(c, p);


		/* It takes 1000 game turns for "feelings" to recharge.
		 * XXX: This doesn't make much sense with an idempotent
		 *      cave_generate(). Feelings should be computed at level
		 *      start unconditionally and just not displayed until the
		 *      PC can see them.
		 */
#if 0
		if (((turn - old_turn) < 1000) && (old_turn > 1))
			c->feeling = 0;
		else
			c->feeling = calculate_feeling(c, depth);
#endif
		c->feeling = calculate_feeling(c);

		/* Hack -- regenerate "over-flow" levels */
		if (o_max >= z_info->o_max)
			error = "too many objects";
		if (mon_max >= z_info->m_max)
			error = "too many monsters";


		if (OPT(cheat_room) && error)
			msg("Generation restarted: %s.", error);

		counter++;
		if (counter > 100)
		{
			msg("cave_gen() failed 100 times!");
			exit_game_panic();
		}
	}

	/* The dungeon is ready */
	character_dungeon = TRUE;

	c->created_at = turn;
}
