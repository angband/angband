/* File: generate.c */

/* Purpose: initialize/create a dungeon or town level */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Note that Level generation is *not* a bottleneck...
 *
 * So we emphasize "simplicity" and "correctness" over "speed"
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * In this file, we use some of the "temporary" cave grid flags.
 * They are all cleared before the "cave_gen()" function ends.
 * Well, actually, they are not, but they can be if anyone cares.
 *
 *   GRID_EXT1 -- the grid is in an outer wall of a room
 *   GRID_EXT2 -- the grid may not be pierced by corridors
 *   GRID_EXT3 -- the grid is used by overlapping corridors
 *   GRID_EXT4 -- the grid is already inside a streamer
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks.  As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds.  Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the old 33x11 blocks)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries.  This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen.  It may be annoying enough to go back to 33x11
 * blocks to prevent this visual situation.
 *
 * Note that we use "GRID_EXT2" for the "outer dungeon walls" to
 * prevent attempts by corridors to leave the dungeon, and we also
 * use "GRID_EXT1" to prevent corridors from piercing walls adjacent
 * to the "outer dungeon walls".
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away).
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart.  This prevents "large" (or "silly") doorways.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * Note also that the probablility of generating a greater vault may
 * need to be re-adjusted, since the code is so very different.  Since
 * greater vaults are so incredible, it is probably best if they are
 * NOT very likely, especially at lower levels.
 */




/*
 * Dungeon generation values
 */
#define DUN_ROOMS	50	/* Number of rooms to attempt */
#define DUN_UNUSUAL	200	/* Level/chance of unusual room */	
#define DUN_DEST	15	/* 1/chance of having a destroyed level */

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
#define DUN_AMT_ROOM	9	/* Amount of objects for rooms */
#define DUN_AMT_ITEM	3	/* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD	3	/* Amount of treasure for rooms/corridors */




/*
 * The "size" of a "generation block" in grids
 */
#define BLOCK_HGT	11
#define BLOCK_WID	11

/*
 * Maximum numbers of rooms along each axis (currently 6x6)
 */
#define MAX_ROOMS_ROW	(MAX_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL	(MAX_WID / BLOCK_WID)


/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	100
#define DOOR_MAX	200
#define WALL_MAX	500
#define TUNN_MAX	900

/*
 * Maximal number of room types
 */
#define ROOM_MAX	8



/*
 * Simple structure to hold a map location
 */
typedef struct _coord {
    byte y;
    byte x;
} coord;


/*
 * Room type information
 */
typedef struct _room_data {

    /* Required size in blocks */
    int dy1, dy2, dx1, dx2;

    /* Max number of this kind per level */
    int max;

} room_data;


/*
 * Structure to hold all "dungeon generation" data
 */
typedef struct _dun_data {

    /* Array of centers of rooms */
    int cent_n;
    coord cent[CENT_MAX];

    /* Array of possible door locations */
    int door_n;
    coord door[DOOR_MAX];

    /* Array of wall piercing locations */
    int wall_n;
    coord wall[WALL_MAX];

    /* Array of tunnel grids */
    int tunn_n;
    coord tunn[TUNN_MAX];

    /* Number of blocks along each axis */
    int row_rooms;
    int col_rooms;

    /* Array of which blocks are used */
    bool room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];

    /* Number of rooms of each type */
    int count[ROOM_MAX];

    /* Template for lesser/greater vaults */
    char template[3500];

} dun_data;


/*
 * All the dungeon generation data
 */
static dun_data dun_body;
static dun_data *dun = &dun_body;


/*
 * Array of room types (assumes 11x11 blocks)
 */
static room_data room[ROOM_MAX] = {

    { 0, 0, 0, 0, 0 },		/* Nothing */
    { 0, 0, -1, 1, 100 },	/* Simple (33x11) */
    { 0, 0, -1, 1, 100 },	/* Overlapping (33x11) */
    { 0, 0, -1, 1, 100 },	/* Crossed (33x11) */
    { 0, 0, -1, 1, 100 },	/* Large (33x11) */
    { 0, 0, -1, 1, 1 },		/* Monster pit (33x11) */
    { 0, 1, -1, 1, 10 },	/* Lesser vault (33x22) */
    { -1, 2, -2, 3, 1 }		/* Greater vault (66x44) */
};



/*
 * Always picks a correct direction		
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
    /* Extract vertical and horizontal directions */
    *rdir = (y1 == y2) ? 0 : (y1 < y2) ? 1 : -1;
    *cdir = (x1 == x2) ? 0 : (x1 < x2) ? 1 : -1;

    /* Never move diagonally */
    if (*rdir && *cdir) {
        if (rand_int(2)) {
            *rdir = 0;
        }
        else {
            *cdir = 0;
        }
    }
}


/*
 * Pick a random direction
 */
static void rand_dir(int *rdir, int *cdir)
{
    /* Pick a random direction */
    int i = rand_int(4);

    /* Extract the dy/dx components */
    *rdir = ddy[ddd[i]];
    *cdir = ddx[ddd[i]];
}


/*
 * Returns random co-ordinates for player/monster/object
 */
static void new_player_spot(void)
{
    int        y, x;

    /* Place the player */
    while (1) {

        /* Pick a legal spot */
        y = rand_range(1, cur_hgt - 2);
        x = rand_range(1, cur_wid - 2);

        /* Must be a "naked" floor grid */
        if (!naked_grid_bold(y, x)) continue;

        /* Refuse to start on anti-teleport grids */
        if (cave[y][x].info & GRID_ICKY) continue;

        /* Done */
        break;
    }

    /* Save the new player grid */
    py = y;
    px = x;

    /* Mark the dungeon grid */
    cave[py][px].m_idx = 1;
}



/*
 * Count the number of walls adjacent to the given grid.
 * Note -- Assumes "in_bounds(y, x)"
 */
static int next_to_walls(int y, int x)
{
    int        k = 0;

    if (cave[y+1][x].info & GRID_WALL_MASK) k++;
    if (cave[y-1][x].info & GRID_WALL_MASK) k++;
    if (cave[y][x+1].info & GRID_WALL_MASK) k++;
    if (cave[y][x-1].info & GRID_WALL_MASK) k++;

    return (k);
}



/*
 * Place some rubble at the given location
 */
static void place_rubble(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_RUBBLE);
    i_ptr->iy = y;
    i_ptr->ix = x;
}





/*
 * Place an open door at the given location
 */
static void place_open_door(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_OPEN_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Hack -- nuke any walls */
    c_ptr->info &= ~GRID_WALL_MASK;
}


/*
 * Place a broken door at the given location
 */
static void place_broken_door(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_OPEN_DOOR);
    i_ptr->pval = 1;
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Hack -- nuke any walls */
    c_ptr->info &= ~GRID_WALL_MASK;
}


/*
 * Place a closed door at the given location
 */
static void place_closed_door(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_CLOSED_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Hack -- nuke any walls */
    c_ptr->info &= ~GRID_WALL_MASK;
}


/*
 * Place a locked door at the given location
 */
static void place_locked_door(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_CLOSED_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Lock the door */
    i_ptr->pval = rand_int(10) + 10;

    /* Hack -- nuke any walls */
    c_ptr->info &= ~GRID_WALL_MASK;
}


/*
 * Place a stuck door at the given location
 */
static void place_stuck_door(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_CLOSED_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Stick the door */
    i_ptr->pval = 0 - rand_int(10) - 10;

    /* Hack -- nuke any walls */
    c_ptr->info &= ~GRID_WALL_MASK;
}


/*
 * Place a secret door at the given location
 */
static void place_secret_door(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_SECRET_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Hack -- nuke any walls */
    c_ptr->info &= ~GRID_WALL_MASK;
}


/*
 * Place a door at the given location
 */
static void place_door(int y, int x)
{
    int tmp = rand_int(1000);

    /* Open doors (300/1000) */
    if (tmp < 300) {
        place_open_door(y, x);
    }

    /* Broken doors (100/1000) */
    else if (tmp < 400) {
        place_broken_door(y, x);
    }

    /* Secret doors (200/1000) */
    else if (tmp < 600) {
        place_secret_door(y, x);
    }

    /* Closed doors (300/1000) */
    else if (tmp < 900) {
        place_closed_door(y, x);
    }

    /* Locked doors (99/1000) */
    else if (tmp < 999) {
        place_locked_door(y, x);
    }

    /* Stuck doors (1/1000) */
    else {
        place_stuck_door(y, x);
    }
}


/*
 * Place an up staircase at given location
 */
static void place_up_stairs(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_UP_STAIR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Stairs are permanent */
    c_ptr->info |= GRID_PERM;
}


/*
 * Place a down staircase at given location
 */
static void place_down_stairs(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    if (!valid_grid(y, x)) return;
    delete_object(y, x);

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_DOWN_STAIR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Stairs are permanent */
    c_ptr->info |= GRID_PERM;
}


/*
 * Places some staircases (1=up, 2=down) near walls
 */
static void place_stairs(int typ, int num, int walls)
{
    int                 y, x, i, j, flag;

    /* Place "num" stairs */
    for (i = 0; i < num; i++) {

        /* Place some stairs */
        for (flag = FALSE; !flag; ) {

            /* Try several times, then decrease "walls" */
            for (j = 0; !flag && j <= 3000; j++) {

                /* Pick a random grid */
                y = rand_int(cur_hgt);
                x = rand_int(cur_wid);

                /* Require "naked" floor grid */
                if (!naked_grid_bold(y, x)) continue;

                /* Require a certain number of adjacent walls */
                if (next_to_walls(y, x) < walls) continue;

                /* Place some stairs */
                if (is_quest(dun_level)) {
                    place_up_stairs(y, x);
                }
                else if (typ == 1) {
                    place_up_stairs(y, x);
                }
                else {
                    place_down_stairs(y, x);
                }

                /* All done */
                flag = TRUE;
            }

            /* Require fewer walls */
            if (walls) walls--;
        }
    }
}




/*
 * Semi-Hack -- types of "places" we can allocate (below)
 */
#define ALLOC_SET_CORR	1
#define ALLOC_SET_ROOM	2
#define ALLOC_SET_BOTH	3


/*
 * Allocates an object for tunnels and rooms
 *
 * Type 1 is rubble
 * Type 2 is unused (was visible traps)
 * Type 3 is trap
 * Type 4 is gold
 * Type 5 is object
 */
static void alloc_object(int set, int typ, int num)
{
    int y, x, k;

    /* Place some objects */
    for (k = 0; k < num; k++) {

        /* Pick a "legal" spot */
        while (TRUE) {

            bool room;

            /* Location */
            y = rand_int(cur_hgt);
            x = rand_int(cur_wid);

            /* Require "naked" floor grid */
            if (!naked_grid_bold(y, x)) continue;

            /* Check for "room" */
            room = (cave[y][x].info & GRID_ROOM) ? TRUE : FALSE;

            /* Require corridor? */
            if ((set == ALLOC_SET_CORR) && room) continue;

            /* Require room? */
            if ((set == ALLOC_SET_ROOM) && !room) continue;

            /* Accept it */
            break;
        }

        /* Place an object */
        if (typ < 4) {
            if (typ == 1) {
                place_rubble(y, x);
            }
            else /* (typ == 3) */ {
                place_trap(y, x);
            }
        }
        else {
            if (typ == 4) {
                place_gold(y, x);
            }
            else /* (typ == 5) */ {
                place_object(y, x);
            }
        }
    }
}



/*
 * Places "streamers" of rock through dungeon		-RAK-	
 */
static void build_streamer(int type, int chance)
{
    int		i, tx, ty;
    int		y, x, dir;

    /* Hack -- Choose starting point near the center of the dungeon */
    y = rand_spread(cur_hgt / 2, 10);
    x = rand_spread(cur_wid / 2, 15);

    /* Choose a random compass direction */
    dir = ddd[rand_int(8)];

    /* Place streamer into dungeon */
    while (1) {

        /* One grid per density */
        for (i = 0; i < DUN_STR_DEN; i++) {

            int d = DUN_STR_RNG;

            /* Pick a nearby grid */
            while (1) {
                ty = rand_spread(y, d);
                tx = rand_spread(x, d);
                if (!in_bounds2(ty, tx)) continue;
                break;
            }

            /* Turn granite into streamers */
            if (TRUE) {

                cave_type *c_ptr = &cave[ty][tx];

                /* Skip non-walls */
                if (!(c_ptr->info & GRID_WALL_MASK)) continue;

                /* Skip permanent walls */
                if (c_ptr->info & GRID_PERM) continue;

                /* Skip existing streamers */
                if (c_ptr->info & GRID_EXT4) continue;

                /* Destroy the granite */
                c_ptr->info &= ~GRID_WALL_MASK;

                /* Add some treasure */
                if (rand_int(chance) == 0) place_gold(ty, tx);

                /* Create a streamer (on top of the treasure) */
                c_ptr->info |= type;

                /* Note that we are a streamer */
                c_ptr->info |= GRID_EXT4;
            }
        }

        /* Advance the streamer */
        y += ddy[dir];
        x += ddx[dir];

        /* Quit before leaving the dungeon */
        if (!in_bounds(y, x)) break;
    }
}


/*
 * Build a destroyed level
 */
static void destroy_level()
{
    int y1, x1, y, x, k, t, n;

    /* Note destroyed levels */
    if (cheat_room) msg_print("Destroyed Level");

    /* Drop a few epi-centers (usually about two) */
    for (n = 0; n < randint(5); n++) {

        /* Pick a (central) epi-center */
        x1 = rand_range(16, cur_wid - 17);
        y1 = rand_range(16, cur_hgt - 17);

        /* Big area of affect */
        for (y = (y1 - 15); y <= (y1 + 15); y++) {
            for (x = (x1 - 15); x <= (x1 + 15); x++) {

                /* Skip the epicenter */
                if ((y == y1) && (x == x1)) continue;

                /* Require a legal, valid, grid */
                if (!valid_grid(y, x)) continue;

                /* Extract the distance */
                k = distance(y1, x1, y, x);

                /* Stay in the circle of death */
                if (k < 16) {

                    cave_type *c_ptr = &cave[y][x];

                    /* Replace the ground */
                    t = ((k < 13) ? randint(6) : randint(9));

                    /* Clear the walls */
                    c_ptr->info &= ~GRID_WALL_MASK;

                    /* Make new walls */
                    switch (t) {
                      case 1: case 2: case 3:
                        break;
                      case 4: case 7: case 10:
                        c_ptr->info |= GRID_WALL_GRANITE;
                        break;
                      case 5: case 8: case 11:
                        c_ptr->info |= GRID_WALL_MAGMA;
                        break;
                      case 6: case 9: case 12:
                        c_ptr->info |= GRID_WALL_QUARTZ;
                        break;
                    }

                    /* No longer part of a room */
                    c_ptr->info &= ~GRID_ROOM;
                    c_ptr->info &= ~GRID_MARK;
                    c_ptr->info &= ~GRID_GLOW;

                    /* Delete the object (if any) */
                    delete_object(y, x);

                    /* Delete the monster (if any) */
                    delete_monster(y, x);
                }
            }
        }
    }
}



/*
 * Create up to "num" objects near the given coordinates
 * Only really called by some of the "vault" routines.
 */
static void vault_treasure(int y, int x, int num)
{
    int        i, j, k;

    /* Attempt to place 'num' objects */
    for (; num > 0; --num) {

        /* Try up to 11 spots looking for empty space */
        for (i = 0; i < 11; ++i) {

            /* Pick a random location */
            while (1) {
                j = rand_spread(y, 2);
                k = rand_spread(x, 3);
                if (!in_bounds(j,k)) continue;
                break;
            }

            /* Require "clean" floor space */
            if (!clean_grid_bold(j,k)) continue;

            /* Place something */
            if (randint(100) < 75) {
                place_object(j, k);
            }
            else {
                place_gold(j, k);
            }

            /* Placement accomplished */
            break;
        }
    }
}


/*
 * Place a trap with a given displacement of point
 */
static void vault_trap_aux(int y, int x, int yd, int xd)
{
    int		count, y1, x1;

    /* Place traps */
    for (count = 0; count <= 5; count++) {

        /* Get a location */
        while (1) {
            y1 = rand_spread(y, yd);
            x1 = rand_spread(x, xd);
            if (!in_bounds(y1, x1)) continue;
            break;
        }

        /* Require "naked" floor grids */
        if (!naked_grid_bold(y1, x1)) continue;

        /* Place the trap */
        place_trap(y1, x1);

        /* Done */
        break;
    }
}


/*
 * Place some traps with a given displacement of given location
 */
static void vault_trap(int y, int x, int yd, int xd, int num)
{
    int i;

    for (i = 0; i < num; i++) {
        vault_trap_aux(y, x, yd, xd);
    }
}


/*
 * Hack -- Place some sleeping monsters near the given location
 */
static void vault_monster(int y1, int x1, int num)
{
    int          k, i, y, x, r_idx;

    monster_race	*r_ptr;


    /* Try to summon "num" monsters "near" the given location */
    for (k = 0; k < num; k++) {

        /* Try nine locations */
        for (i = 0; i < 9; i++) {

            int d = 1;

            /* Pick a nearby location */
            while (1) {
                y = rand_spread(y1, d);
                x = rand_spread(x1, d);
                if (!in_bounds(y, x)) continue;
                if (distance(y1, x1, y, x) > d) continue;
                if (los(y1, x1, y, x)) break;
            }

            /* Require "empty" floor grids */
            if (!empty_grid_bold(y, x)) continue;

            /* Pick a monster race (slightly out of depth) */
            r_idx = get_mon_num(dun_level + 2);

            /* Get the race */
            r_ptr = &r_list[r_idx];

            /* Place the monster */
            if (r_ptr->rflags1 & RF1_FRIENDS) {
                place_group(y, x, r_idx, TRUE);
            }
            else {
                place_monster(y, x, r_idx, TRUE);
            }
        }
    }
}


/*
 * Place a sleeping jelly at the given location
 * Note the total hack we use to choose a jelly
 */
static void vault_jelly(int y, int x)
{
    int i, r_idx;

    monster_race *r_ptr;

    /* Hack -- allocate a simple sleeping jelly */
    for (i = 0; i < 10000; i++) {

        monster_level = dun_level + 10;
        r_idx = get_mon_num(monster_level);
        monster_level = dun_level;

        r_ptr = &r_list[r_idx];

        if (!strchr("jmi,", r_ptr->r_char)) continue;
        if (r_ptr->rflags1 & RF1_UNIQUE) continue;
        if (r_ptr->rflags3 & RF3_EVIL) continue;

        place_monster(y, x, r_idx, TRUE);

        break;
    }
}


/*
 * Place a sleeping undead creature at the given location
 * XXX XXX XXX Hack -- we choose random monster races until happy
 */
static void vault_undead(int y, int x)
{
    int i, r_idx;

    monster_race *r_ptr;

    /* Hack -- allocate a simple sleeping undead monster */
    for (i = 0; i < 10000; i++) {

        monster_level = dun_level + 20;
        r_idx = get_mon_num(monster_level);
        monster_level = dun_level;

        r_ptr = &r_list[r_idx];

        if (!(r_ptr->rflags3 & RF3_UNDEAD)) continue;

        if (r_ptr->rflags1 & RF1_UNIQUE) continue;

        place_monster(y, x, r_idx, TRUE);

        break;
    }
}



/*
 * XXX XXX XXX Hack -- This whole section is a hack
 *
 * This is the only (?) place where the actual monster names matter.
 * This is a bad thing.  Perhaps we could use something such as
 * "the third non-unique monster whose symbol is 'o'" (for example).
 * Below are the order in which the desired monsters appear, ignoring
 * unique monsters.
 *
 * Orcs ('o'): 1, 5, 5, 8, 8, 7
 * Giants ('P'): 1, 2, 3, 4, 5, 6
 * Trolls ('T'): 1, 2, 5, 6, 8, 9
 * Demons ('&'): 2, 3, 4, 5, 6, 7
 */


/*
 * Dragons: want Young,Young,Young,Mature,Mature,Ancient
 * Must restrict selection to proper "color".  Must avoid
 * pseudo-dragons, and weird-colored dragons, plus wyrms.
 * Note that the ancient dragons use the 'D' symbol
 * May be able to chack actual "BR_*" flags, and require
 * exactly one of the useful flags (?).
 *
 * XXX Perhaps we can ignore "multi-hued dragon pits", or possibly
 * use the "ATTR_MULTI" flag to notice multi-hued dragons, or just
 * require all 5 of the useful flags (see above)
 *
 * Currently, we use the color and rank to construct the name.
 */


/*
 * Place the lowest level monster with matching name
 * XXX XXX Note that these monsters are NOT sleeping
 * XXX XXX XXX XXX Note direct use of "r_list[]".
 */
static void vault_aux(int y, int x, cptr what)
{
    int i;

    /* Find and place the first matching monster */
    for (i = 1; i < MAX_R_IDX-1; i++) {
        monster_race *r_ptr = &r_list[i];
        if (streq(r_ptr->name, what)) {
            place_monster(y, x, i, FALSE);
            break;
        }
    }
}


static cptr vault_orc_names[] = {
    "Snaga", "Black orc", "Black orc",
    "Uruk", "Uruk", "Orc captain"
};

static void vault_orc(int y, int x, int rank)
{
    vault_aux(y, x, vault_orc_names[rank-1]);
}


static cptr vault_troll_names[] = {
    "Forest troll", "Stone troll", "Ice troll",
    "Cave troll", "Water troll", "Olog"
};

static void vault_troll(int y, int x, int rank)
{
    vault_aux(y, x, vault_troll_names[rank-1]);
}


static cptr vault_giant_names[] = {
    "Hill giant", "Frost giant", "Fire giant",
    "Stone giant", "Cloud giant", "Storm giant"
};

static void vault_giant(int y, int x, int rank)
{
    vault_aux(y, x, vault_giant_names[rank-1]);
}


static cptr vault_dragon_names_1[] = {
    "Young", "Young", "Young",
    "Mature", "Mature", "Ancient"
};

static cptr vault_dragon_names_2[] = {
    "blue dragon", "white dragon", "green dragon"
    "black dragon", "red dragon", "Multi-Hued Dragon"
};

static void vault_dragon(int y, int x, int rank, int type)
{
    char what[128];

    /* Construct a dragon name */
    sprintf(what, "%s %s",
            vault_dragon_names_1[rank-1],
            vault_dragon_names_2[type-1]);

    /* Allocate one */
    vault_aux(y, x, what);
}


static cptr vault_demon_names[] = {
    "Vrock", "Hezrou", "Glabrezu",
    "Nalfeshnee", "Marilith", "Lesser Balrog"
};

static void vault_demon(int y, int x, int rank)
{
    vault_aux(y, x, vault_demon_names[rank-1]);
}


static void vault_nasty(int y, int x, int type, int rank, int colour)
{
    switch (type) {
      case 1:
        vault_jelly(y, x);
        break;
      case 2:
        vault_orc(y, x, rank);
        break;
      case 3:
        vault_troll(y, x, rank);
        break;
      case 4:
        vault_giant(y, x, rank);
        break;
      case 5:
        vault_undead(y, x);
        break;
      case 6:
        vault_dragon(y, x, rank, colour);
        break;
      case 7:
        vault_demon(y, x, rank);
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
 *   5 -- monster pits
 *   6 -- simple vaults
 *   7 -- greater vaults
 */


/*
 * Type 1 -- normal rectangular rooms
 */
static void build_type1(int yval, int xval)
{
    int			y, x, y2, x2;
    int                 y1, x1;
    bool		light;


    /* Choose lite or dark */
    light = (dun_level <= randint(25));


    /* Pick a room size */
    y1 = yval - randint(4);
    y2 = yval + randint(3);
    x1 = xval - randint(11);
    x2 = xval + randint(11);


    /* Place a full floor under the room */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        for (x = x1 - 1; x <= x2 + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }

    /* Walls around the room */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        cave[y][x1-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
        cave[y1-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }


    /* Hack -- Occasional pillar room */
    if (rand_int(20) == 0) {
        for (y = y1; y <= y2; y += 2) {
            for (x = x1; x <= x2; x += 2) {
                cave[y][x].info |= GRID_WALL_GRANITE;
            }
        }
    }

    /* Hack -- Occasional ragged-edge room */
    else if (rand_int(50) == 0) {
        for (y = y1 + 2; y <= y2 - 2; y += 2) {
            cave[y][x1].info |= GRID_WALL_GRANITE;
            cave[y][x2].info |= GRID_WALL_GRANITE;
        }
        for (x = x1 + 2; x <= x2 - 2; x += 2) {
            cave[y1][x].info |= GRID_WALL_GRANITE;
            cave[y2][x].info |= GRID_WALL_GRANITE;
        }
    }
}


/*
 * Type 2 -- Overlapping rectangular rooms
 */
static void build_type2(int yval, int xval)
{
    int			y, x;
    int			y1a, x1a, y2a, x2a;
    int			y1b, x1b, y2b, x2b;
    bool		light;


    /* Choose lite or dark */
    light = (dun_level <= randint(25));


    /* Determine extents of the first room */
    y1a = yval - randint(4);
    y2a = yval + randint(3);
    x1a = xval - randint(11);
    x2a = xval + randint(10);

    /* Determine extents of the second room */
    y1b = yval - randint(3);
    y2b = yval + randint(4);
    x1b = xval - randint(10);
    x2b = xval + randint(11);


    /* Place a full floor for room "a" */
    for (y = y1a - 1; y <= y2a + 1; y++) {
        for (x = x1a - 1; x <= x2a + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }

    /* Place a full floor for room "b" */
    for (y = y1b - 1; y <= y2b + 1; y++) {
        for (x = x1b - 1; x <= x2b + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }


    /* Place the walls around room "a" */
    for (y = y1a - 1; y <= y2a + 1; y++) {
        cave[y][x1a-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2a+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1a - 1; x <= x2a + 1; x++) {
        cave[y1a-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2a+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }

    /* Place the walls around room "b" */
    for (y = y1b - 1; y <= y2b + 1; y++) {
        cave[y][x1b-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2b+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1b - 1; x <= x2b + 1; x++) {
        cave[y1b-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2b+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }


    /* Replace the floor for room "a" */
    for (y = y1a; y <= y2a; y++) {
        for (x = x1a; x <= x2a; x++) {
            cave[y][x].info &= ~(GRID_WALL_MASK | GRID_EXT1);
        }
    }

    /* Replace the floor for room "b" */
    for (y = y1b; y <= y2b; y++) {
        for (x = x1b; x <= x2b; x++) {
            cave[y][x].info &= ~(GRID_WALL_MASK | GRID_EXT1);
        }
    }
}

#if 0

/*
 * Type 2 -- Overlapping rectangular rooms
 *
 * XXX XXX XXX This version of the function for some unknown reason
 * does not set the "GRID_ROOM" flag correctly.  There is something
 * extremely wrong going on here.  I have absolutely no fucking idea
 * what is wrong with this function, but it might be important.
 */
static void build_type2(int yval, int xval)
{
    bool		light;
    int                 y, x, y1, y2, x1, x2, i, limit;
    int			y1a[4], y2a[4], x1a[4], x2a[4];


    /* Choose lite or dark */
    light = (dun_level <= randint(25));

    /* Pick room dimensions */
    for (i = 0; i < 2; i++) {

        /* Pick (and save) a room size */
        y1a[i] = yval - randint(4);
        y2a[i] = yval + randint(3);
        x1a[i] = xval - randint(11);
        x2a[i] = xval + randint(11);
    }

    /* Lay down the rooms */
    for (i = 0; i < 2; i++) {

        /* Extract the room saved above */
        y1 = y1a[i];
        y2 = y2a[i];
        x1 = x1a[i];
        x2 = x2a[i];

        /* Place a full floor under the room */
        for (y = y1 - 1; y <= y2 + 1; y++) {
            for (x = x1 - 1; x <= x2 + 1; x++) {
                cave[y][x].info &= ~GRID_WALL_MASK;
                cave[y][x].info |= GRID_ROOM;
                if (light) cave[y][x].info |= GRID_GLOW;
            }
        }

        /* Walls */
        for (y = y1 - 1; y <= y2 + 1; y++) {
            cave[y][x1-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
            cave[y][x2+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        }
        for (x = x1 - 1; x <= x2 + 1; x++) {
            cave[y1-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
            cave[y2+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        }
    }

    /* Now erase all walls "inside" the rooms */
    for (i = 0; i < 2; i++) {

        /* Extract the room saved above */
        y1 = y1a[i];
        y2 = y2a[i];
        x1 = x1a[i];
        x2 = x2a[i];

        /* Lay down the interior floor */
        for (y = y1; y <= y2; y++) {
            for (x = x1; x <= x2; x++) {
                cave[y][x].info &= ~GRID_WALL_MASK;
                cave[y][x].info &= ~GRID_EXT1;
            }
        }
    }
}

#endif


/*
 * Type 3 -- Cross shaped rooms
 *
 * Builds a room at a row, column coordinate
 *
 * Room "a" runs north/south, and Room "b" runs east/east
 * So the "central pillar" runs from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that
 * the code below will work (with "bounds checking") for 5x5, or even
 * for unsymetric values like 4x3 or 5x3 or 3x4 or 3x5, or even larger.
 */
static void build_type3(int yval, int xval)
{
    int			y, x, dy, dx, wy, wx;
    int			y1a, x1a, y2a, x2a;
    int			y1b, x1b, y2b, x2b;
    bool		light;


    /* Choose lite or dark */
    light = (dun_level <= randint(25));


    /* For now, always 3x3 */
    wx = wy = 1;

    /* Pick max vertical size (at most 4) */
    dy = rand_range(3, 4);

    /* Pick max horizontal size (at most 15) */
    dx = rand_range(3, 11);


    /* Determine extents of the north/south room */
    y1a = yval - dy;
    y2a = yval + dy;
    x1a = xval - wx;
    x2a = xval + wx;

    /* Determine extents of the east/west room */
    y1b = yval - wy;
    y2b = yval + wy;
    x1b = xval - dx;
    x2b = xval + dx;


    /* Place a full floor for room "a" */
    for (y = y1a - 1; y <= y2a + 1; y++) {
        for (x = x1a - 1; x <= x2a + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }

    /* Place a full floor for room "b" */
    for (y = y1b - 1; y <= y2b + 1; y++) {
        for (x = x1b - 1; x <= x2b + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }


    /* Place the walls around room "a" */
    for (y = y1a - 1; y <= y2a + 1; y++) {
        cave[y][x1a-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2a+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1a - 1; x <= x2a + 1; x++) {
        cave[y1a-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2a+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }

    /* Place the walls around room "b" */
    for (y = y1b - 1; y <= y2b + 1; y++) {
        cave[y][x1b-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2b+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1b - 1; x <= x2b + 1; x++) {
        cave[y1b-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2b+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }


    /* Replace the floor for room "a" */
    for (y = y1a; y <= y2a; y++) {
        for (x = x1a; x <= x2a; x++) {
            cave[y][x].info &= ~(GRID_WALL_MASK | GRID_EXT1);
        }
    }

    /* Replace the floor for room "b" */
    for (y = y1b; y <= y2b; y++) {
        for (x = x1b; x <= x2b; x++) {
            cave[y][x].info &= ~(GRID_WALL_MASK | GRID_EXT1);
        }
    }



    /* Special features (3/4) */
    switch (rand_int(4)) {

      /* Large solid middle pillar */
      case 1:
        for (y = y1b; y <= y2b; y++) {
            for (x = x1a; x <= x2a; x++) {
                cave[y][x].info |= GRID_WALL_GRANITE;
            }
        }
        break;

      /* Inner treasure vault */
      case 2:

        /* Build the vault */
        for (y = y1b; y <= y2b; y++) {
            cave[y][x1a].info |= GRID_WALL_GRANITE;
            cave[y][x2a].info |= GRID_WALL_GRANITE;
        }
        for (x = x1a; x <= x2a; x++) {
            cave[y1b][x].info |= GRID_WALL_GRANITE;
            cave[y2b][x].info |= GRID_WALL_GRANITE;
        }

        /* Place a secret door on the inner room */
        switch (rand_int(4)) {
            case 0: place_secret_door(y1b, xval); break;
            case 1: place_secret_door(y2b, xval); break;
            case 2: place_secret_door(yval, x1a); break;
            case 3: place_secret_door(yval, x2a); break;
        }

        /* Place a treasure in the vault */
        place_object(yval, xval);

        /* Let's guard the treasure well */
        vault_monster(yval, xval, rand_int(2) + 3);

        /* Traps naturally */
        vault_trap(yval, xval, 4, 4, rand_int(3) + 2);

        break;


      /* Something else */
      case 3:

        /* Occasionally pinch the center shut */
        if (rand_int(3) == 0) {

            /* Pinch the east/west sides */
            for (y = y1b; y <= y2b; y++) {
                if (y == yval) continue;
                cave[y][x1a - 1].info |= GRID_WALL_GRANITE;
                cave[y][x2a + 1].info |= GRID_WALL_GRANITE;
            }

            /* Pinch the north/south sides */
            for (x = x1a; x <= x2a; x++) {
                if (x == xval) continue;
                cave[y1b - 1][x].info |= GRID_WALL_GRANITE;
                cave[y2b + 1][x].info |= GRID_WALL_GRANITE;
            }

            /* Sometimes shut using secret doors */
            if (rand_int(3) == 0) {
                place_secret_door(yval, x1a - 1);
                place_secret_door(yval, x2a + 1);
                place_secret_door(y1b - 1, xval);
                place_secret_door(y2b + 1, xval);
            }
        }

        /* Occasionally put a "plus" in the center */
        else if (rand_int(3) == 0) {
            cave[yval][xval].info |= GRID_WALL_GRANITE;
            cave[y1b][xval].info |= GRID_WALL_GRANITE;
            cave[y2b][xval].info |= GRID_WALL_GRANITE;
            cave[yval][x1a].info |= GRID_WALL_GRANITE;
            cave[yval][x2a].info |= GRID_WALL_GRANITE;
        }

        /* Occasionally put a pillar in the center */
        else if (rand_int(3) == 0) {
            cave[yval][xval].info |= GRID_WALL_GRANITE;
        }

        break;
    }
}


/*
 * Type 4 -- Large room with inner features
 *
 * Possible sub-types:
 *	1 - Just an inner room with one door
 *	2 - An inner room within an inner room
 *	3 - An inner room with pillar(s)
 *	4 - Inner room has a maze
 *	5 - A set of four inner rooms
 */
static void build_type4(int yval, int xval)
{
    int        y, x, y1, x1;
    int                 y2, x2, tmp;
    bool		light;


    /* Choose lite or dark */
    light = (dun_level <= randint(25));


    /* Large room */
    y1 = yval - 4;
    y2 = yval + 4;
    x1 = xval - 11;
    x2 = xval + 11;


    /* Place a full floor under the room */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        for (x = x1 - 1; x <= x2 + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }

    /* Outer Walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        cave[y][x1-1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2+1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
        cave[y1-1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2+1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }


    /* The inner room */
    y1 = y1 + 2;
    y2 = y2 - 2;
    x1 = x1 + 2;
    x2 = x2 - 2;

    /* Inner Walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        cave[y][x1 - 1].info |= GRID_WALL_GRANITE;
        cave[y][x2 + 1].info |= GRID_WALL_GRANITE;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
        cave[y1 - 1][x].info |= GRID_WALL_GRANITE;
        cave[y2 + 1][x].info |= GRID_WALL_GRANITE;
    }


    /* Inner room variations */
    switch (randint(5)) {

      /* Just an inner room with a monster */
      case 1:

        /* Place a secret door */
        switch (randint(4)) {
            case 1: place_secret_door(y1 - 1, xval); break;
            case 2: place_secret_door(y2 + 1, xval); break;
            case 3: place_secret_door(yval, x1 - 1); break;
            case 4: place_secret_door(yval, x2 + 1); break;
        }

        /* Place a monster in the room */
        vault_monster(yval, xval, 1);

        break;


      /* Treasure Vault (with a door) */
      case 2:

        /* Place a secret door */
        switch (randint(4)) {
            case 1: place_secret_door(y1 - 1, xval); break;
            case 2: place_secret_door(y2 + 1, xval); break;
            case 3: place_secret_door(yval, x1 - 1); break;
            case 4: place_secret_door(yval, x2 + 1); break;
        }

        /* Place another inner room */
        for (y = yval - 1; y <= yval + 1; y++) {
            for (x = xval -  1; x <= xval + 1; x++) {
                if ((x == xval) && (y == yval)) continue;
                cave[y][x].info |= GRID_WALL_GRANITE;
            }
        }

        /* Place a locked door on the inner room */
        switch (randint(4)) {
            case 1: place_locked_door(yval - 1, xval); break;
            case 2: place_locked_door(yval + 1, xval); break;
            case 3: place_locked_door(yval, xval - 1); break;
            case 4: place_locked_door(yval, xval + 1); break;
        }

        /* Monsters to guard the "treasure" */
        vault_monster(yval, xval, randint(3) + 2);

        /* Object or stairs */
        if (rand_int(100) < 80) {
            place_object(yval, xval);
        }
        else if (is_quest(dun_level)) {
            place_up_stairs(yval, xval);
        }
        else if (rand_int(2) == 0) {
            place_up_stairs(yval, xval);
        }
        else {
            place_down_stairs(yval, xval);
        }

        /* Traps to protect the treasure */
        vault_trap(yval, xval, 4, 10, 2 + randint(3));

        break;


      /* Inner pillar(s). */
      case 3:

        /* Place a secret door */
        switch (randint(4)) {
            case 1: place_secret_door(y1 - 1, xval); break;
            case 2: place_secret_door(y2 + 1, xval); break;
            case 3: place_secret_door(yval, x1 - 1); break;
            case 4: place_secret_door(yval, x2 + 1); break;
        }

        /* Large Inner Pillar */
        for (y = yval - 1; y <= yval + 1; y++) {
            for (x = xval - 1; x <= xval + 1; x++) {
                cave[y][x].info |= GRID_WALL_GRANITE;
            }
        }

        /* Occasionally, two more Large Inner Pillars */
        if (rand_int(2) == 0) {
            tmp = randint(2);
            for (y = yval - 1; y <= yval + 1; y++) {
                for (x = xval - 5 - tmp; x <= xval - 3 - tmp; x++) {
                    cave[y][x].info |= GRID_WALL_GRANITE;
                }
                for (x = xval + 3 + tmp; x <= xval + 5 + tmp; x++) {
                    cave[y][x].info |= GRID_WALL_GRANITE;
                }
            }
        }

        /* Occasionally, some Inner rooms */
        if (rand_int(3) == 0) {

            /* Long horizontal walls */
            for (x = xval - 5; x <= xval + 5; x++) {
                cave[yval-1][x].info |= GRID_WALL_GRANITE;
                cave[yval+1][x].info |= GRID_WALL_GRANITE;
            }

            /* Close off the left/right edges */
            cave[yval][xval - 5].info |= GRID_WALL_GRANITE;
            cave[yval][xval + 5].info |= GRID_WALL_GRANITE;

            /* Secret doors (random top/bottom) */
            place_secret_door(yval - 3 + (randint(2) << 1), xval - 3);
            place_secret_door(yval - 3 + (randint(2) << 1), xval + 3);

            /* Monsters */
            vault_monster(yval, xval - 2, randint(2));
            vault_monster(yval, xval + 2, randint(2));

            /* Objects */
            if (rand_int(3) == 0) place_object(yval, xval - 2);
            if (rand_int(3) == 0) place_object(yval, xval + 2);
        }

        break;


      /* Maze inside. */
      case 4:

        /* Place a secret door */
        switch (randint(4)) {
            case 1: place_secret_door(y1 - 1, xval); break;
            case 2: place_secret_door(y2 + 1, xval); break;
            case 3: place_secret_door(yval, x1 - 1); break;
            case 4: place_secret_door(yval, x2 + 1); break;
        }

        /* Maze (really a checkerboard) */
        for (y = y1; y <= y2; y++) {
            for (x = x1; x <= x2; x++) {
                if (0x1 & (x + y)) {
                    cave[y][x].info |= GRID_WALL_GRANITE;
                }
            }
        }

        /* Monsters just love mazes. */
        vault_monster(yval, xval - 5, randint(3));
        vault_monster(yval, xval + 5, randint(3));

        /* Traps make them entertaining. */
        vault_trap(yval, xval - 3, 2, 8, randint(3));
        vault_trap(yval, xval + 3, 2, 8, randint(3));

        /* Mazes should have some treasure too. */
        vault_treasure(yval, xval, 3);

        break;


      /* Four small rooms. */
      case 5:

        /* Inner "cross" */
        for (y = y1; y <= y2; y++) {
            cave[y][xval].info |= GRID_WALL_GRANITE;
        }
        for (x = x1; x <= x2; x++) {
            cave[yval][x].info |= GRID_WALL_GRANITE;
        }

        /* Doors into the rooms */
        if (rand_int(2) == 0) {
            int i = randint(10);
            place_secret_door(y1 - 1, xval - i);
            place_secret_door(y1 - 1, xval + i);
            place_secret_door(y2 + 1, xval - i);
            place_secret_door(y2 + 1, xval + i);
        }
        else {
            int i = randint(3);
            place_secret_door(yval + i, x1 - 1);
            place_secret_door(yval - i, x1 - 1);
            place_secret_door(yval + i, x2 + 1);
            place_secret_door(yval - i, x2 + 1);
        }

        /* Treasure, centered at the center of the cross */
        vault_treasure(yval, xval, 2 + randint(2));

        /* Gotta have some monsters. */
        vault_monster(yval + 2, xval - 4, randint(2));
        vault_monster(yval + 2, xval + 4, randint(2));
        vault_monster(yval - 2, xval - 4, randint(2));
        vault_monster(yval - 2, xval + 4, randint(2));

        break;
    }
}


/*
 * Type 5 -- Monster pits
 *
 * Monster types in the pit
 *   1 = jelly pit
 *   2 = orc pit	(Dungeon Level 10 and deeper)
 *   3 = troll pit	(Dungeon Level 20 and deeper)
 *   4 = giant pit	(Dungeon Level 30 and deeper)
 *   5 = undead pit	(Dungeon Level 45 and deeper)
 *   6 = dragon pit	(Dungeon Level 60 and deeper)
 *   7 = demon pit	(Dungeon Level 75 and deeper)
 */
static void build_type5(int yval, int xval)
{
    int			y, x, y1, x1, y2, x2;
    int			tmp, type, colour;
    bool		light = FALSE;


    /* Large room */
    y1 = yval - 4;
    y2 = yval + 4;
    x1 = xval - 11;
    x2 = xval + 11;


    /* Place the floor area */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        for (x = x1 - 1; x <= x2 + 1; x++) {
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;
        }
    }

    /* Place the outer walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        cave[y][x1 - 1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y][x2 + 1].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
        cave[y1 - 1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
        cave[y2 + 1][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
    }


    /* Advance to the center room */
    y1 = y1 + 2;
    y2 = y2 - 2;
    x1 = x1 + 2;
    x2 = x2 - 2;

    /* The inner walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
        cave[y][x1 - 1].info |= GRID_WALL_GRANITE;
        cave[y][x2 + 1].info |= GRID_WALL_GRANITE;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
        cave[y1 - 1][x].info |= GRID_WALL_GRANITE;
        cave[y2 + 1][x].info |= GRID_WALL_GRANITE;
    }


    /* Place a secret door */
    switch (randint(4)) {
        case 1: place_secret_door(y1 - 1, xval); break;
        case 2: place_secret_door(y2 + 1, xval); break;
        case 3: place_secret_door(yval, x1 - 1); break;
        case 4: place_secret_door(yval, x2 + 1); break;
    }


    /* Assume no type, no colour */
    type = 0;
    colour = 0;

    /* Choose a pit type */
    tmp = randint(dun_level > 80 ? 80 : dun_level);

    /* Determine monster type from pit type */
    if (tmp < 10) {
        type = 1;
        if (cheat_room) msg_print("Jelly Pit");
    }
    else if (tmp < 20) {
        type = 2;
        if (cheat_room) msg_print("Orc Pit");
    }
    else if (tmp < 30) {
        type = 3;
        if (cheat_room) msg_print("Troll Pit");
    }
    else if (tmp < 45) {
        type = 4;
        if (cheat_room) msg_print("Giant Pit");
    }
    else if (tmp < 60) {
        type = 5;
        if (cheat_room) msg_print("Undead Pit");
    }
    else if (tmp < 75) {
        type = 6;
        colour = randint(6);
        if (cheat_room) msg_print("Dragon Pit");
    }
    else {
        type = 7;
        if (cheat_room) msg_print("Demon Pit");
    }


    /* Increase the level rating */
    rating += 10;

    /* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
    if ((randint(dun_level*dun_level + 1) < 300) && (dun_level <= 40)) {
        good_item_flag = TRUE;
    }


    /* Fill the pit with monsters */
    y = y1;
    for (x = x1; x <= x2; x++) vault_nasty(y, x, type, 1, colour);
    y = y2;
    for (x = x1; x <= x2; x++) vault_nasty(y, x, type, 1, colour);
    x = x1;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 1, colour);
    x = x2;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 1, colour);
    x = x1 + 1;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 2, colour);
    x = x1 + 2;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 2, colour);
    x = x2 - 1;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 2, colour);
    x = x2 - 2;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 2, colour);
    x = x1 + 3;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 3, colour);
    x = x1 + 4;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 3, colour);
    x = x2 - 3;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 3, colour);
    x = x2 - 4;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 3, colour);
    x = x1 + 5;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 4, colour);
    x = x1 + 6;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 4, colour);
    x = x2 - 5;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 4, colour);
    x = x2 - 6;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 4, colour);
    x = x1 + 7;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 5, colour);
    x = x1 + 8;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 5, colour);
    x = x2 - 7;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 5, colour);
    x = x2 - 8;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 5, colour);
    x = x2 - 9;
    for (y = (y1 + 1); y <= (y2 - 1); y++) vault_nasty(y, x, type, 6, colour);
}



/*
 * Type 6 -- simple vaults
 *
 * This might be a good function to use external files.
 *
 * We can use the entire 33x33 region centered at (yval,xval).
 */
static void build_type6(int yval, int xval)
{
    int			x, y, x1, y1;
    int                 width = 0, height = 0;
    bool		light;
    cave_type		*c_ptr;
    char		*t;


    /* Choose lite or dark */
    light = (dun_level <= randint(25));


    /* Pick a large room */
    switch (randint(7)) {

      case 1:
        width = 20;
        height = 12;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s",
                "       %%%%%%       ",
                "    %%%..##..%%%    ",
                "  %%....####....%%  ",
                " %......#**#......% ",
                "%...,.##+##+##.,...%",
                "%.,.,.#*#*&#*#.,.,.%",
                "%.,.,.#*#&*#*#.,.,.%",
                "%...,.##+##+##.,...%",
                " %......#**#......% ",
                "  %%....####....%%  ",
                "    %%%..##..%%%    ",
                "       %%%%%%       ");
        break;

      case 2:
        width = 20;
        height = 14;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                "   %%%%%%%%%%%%%%   ",
                "  %%.##########.%%  ",
                " %%..#..,,,,..#..%% ",
                "%%,..#.,####,.#..,%%",
                "%....#.,#**#,.#....%",
                "%.###+,##&&##,+###.%",
                "%.#..,,#*&**#,,..#.%",
                "%.#..,,#**&*#,,..#.%",
                "%.###+,##&&##,+###.%",
                "%....#.,#**#,.#....%",
                "%%,..#.,####,.#..,%%",
                " %%..#..,,,,..#..%% ",
                "  %%.##########.%%  ",
                "   %%%%%%%%%%%%%%   ");
        break;

      case 3:
        width = 20;
        height = 12;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s",
                "    %%%%%%%%%%%%    ",
                " %%%%..........%%%% ",
                " %...###+##+###...% ",
                "%%...#,,#,,#,,#...%%",
                "%.###+##+##+##+###.%",
                "%.#,,#&&#**#&&#,,#.%",
                "%.#,,#&&#**#&&#,,#.%",
                "%.###+##+##+##+###.%",
                "%%...#,,#,,#,,#...%%",
                " %...###+##+###...% ",
                " %%%%..........%%%% ",
                "    %%%%%%%%%%%%    ");
        break;

      case 4:
        width = 20;
        height = 12;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s",
                "%%%%%%%%%%%%%%%%%%%%",
                "%*.......&........*%",
                "%.################.%",
                "%.#,.,.,.,.,.,.,.#.%",
                "%.#.############,#.%",
                "%.#,+,&&+**#&*,#.#&%",
                "%&#.#,*&#**+&&,+,#.%",
                "%.#,############.#.%",
                "%.#.,.,.,.,.,.,.,#.%",
                "%.################.%",
                "%*........&.......*%",
                "%%%%%%%%%%%%%%%%%%%%");
        break;

      case 5:
        width = 20;
        height = 12;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s",
                "%%%%%%%%%%%%%%%%%   ",
                "%,,,##,,,,##....%%  ",
                "%,,,,##,,,,##....%% ",
                "%#,,,,##,,,,##....%%",
                "%##,,,,##,,,,##....%",
                "%.##,,,,,,,,,,#+...%",
                "%..#+,,,,,,,,,,##..%",
                "%...##,,,,##,,,,##.%",
                "%%...##,,,,##,,,,##%",
                " %%...##,,,,##,,,,#%",
                "  %%...##,,,,##,,,,%",
                "   %%%%%%%%%%%%%%%%%");
        break;

      case 6:
        width = 20;
        height = 12;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s",
                "   %%%%%%%%%%%%%%%%%",
                "  %%....##,,,,##,,,%",
                " %%....##,,,,##,,,,%",
                "%%....##,,,,##,,,,#%",
                "%....##,,,,##,,,,##%",
                "%...+#,,,,,,,,,,##.%",
                "%..##,,,,,,,,,,+#..%",
                "%.##,,,,##,,,,##...%",
                "%##,,,,##,,,,##...%%",
                "%#,,,,##,,,,##...%% ",
                "%,,,,##,,,,##...%%  ",
                "%%%%%%%%%%%%%%%%%   ");
        break;

      case 7:
        width = 20;
        height = 12;
        sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s",
                "%%%%%%%%%%%%%%%%%%%%",
                "%,################,%",
                "%^#.*...&..,....,#^%",
                "%^#...,......&...#^%",
                "%^#######++#######^%",
                "%^+.,..&+,*+*....+^%",
                "%^+..*.,+.&+.,.&.+^%",
                "%^#######++#######^%",
                "%^#....,.,.....,.#^%",
                "%^#..&......*....#^%",
                "%,################,%",
                "%%%%%%%%%%%%%%%%%%%%");
        break;
    }


    /* Small rating boost */
    rating += 5;

    /* Message */
    if (cheat_room) msg_print("Lesser Vault");


    /* (Sometimes) Cause a special feeling */
    if ((dun_level <= 40) ||
        (randint((dun_level-30) * (dun_level-30) + 1) < 400)) {
        good_item_flag = TRUE;
    }


    /* Scan the vault, grid by grid, symbol by symbol */
    for (t = dun->template, y1 = 0; y1 < height; y1++) {
        for (x1 = 0; x1 < width; x1++, t++) {

            /* Extract the location */
            x = xval - (width / 2) + x1;
            y = yval - (height / 2) + y1;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Skip grids not "inside" the room */
            if (*t == ' ') continue;

            /* Start with every grid being a floor in the room */
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;

            /* Analyze the grid */
            switch (*t) {

                /* Outer wall */
              case '%':
                c_ptr->info |= (GRID_WALL_GRANITE | GRID_EXT1);
                break;

                /* Inner wall */	
              case '#':
                c_ptr->info |= GRID_WALL_GRANITE;
                break;

              case '*':	   /* treasure/trap */
                if (randint(20) > 7) {
                    place_object(y, x);
                }
                else if (randint(10) > 2) {
                    place_trap(y, x);
                }
                else if (is_quest(dun_level)) {
                    place_up_stairs(y, x);
                }
                else if (rand_int(2) == 0) {
                    place_down_stairs(y, x);
                }
                else {
                    place_up_stairs(y, x);
                }
                break;

              case '+':	   /* secret doors */
                place_secret_door(y, x);
                break;

              /* Trap */
              case '^':
                place_trap(y, x);
                break;

              case '.':	   /* floor */
                break;

              /* Monsters (see below) */
              case '&':
              case ',':
                break;

              default:
                quit_fmt("Illegal vault creation char '%c'", *t);
                break;
            }
        }
    }


    /* Scan the vault, grid by grid, symbol by symbol -- place monsters */
    for (t = dun->template, y1 = 0; y1 < height; y1++) {
        for (x1 = 0; x1 < width; x1++, t++) {

            /* Extract the grid */
            x = xval - (width/2) + x1;
            y = yval - (height/2) + y1;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Analyze the symbol */
            switch (*t) {

                /* Monster */
              case '&':
                monster_level = dun_level + 4;
                place_monster(y, x, get_mon_num(monster_level), TRUE);
                monster_level = dun_level;
                break;

                /* Monster and/or object */
              case ',':
                if (rand_int(2) == 0) {
                    monster_level = dun_level + 2;
                    place_monster(y, x, get_mon_num(monster_level), TRUE);
                    monster_level = dun_level;
                }
                if (rand_int(2) == 0) {
                    object_level = dun_level + 7;
                    place_object(y, x);
                    object_level = dun_level;
                }
                break;
            }
        }
    }
}



/*
 * Type 7 -- greater vaults
 *
 * This might be a good function to use external files.
 *
 * We can use the entire 99x33 region centered at (yval,xval).
 */
static void build_type7(int yval, int xval)
{
    int			x, y, x1, y1;
    int                 width = 0, height = 0;
    bool		light;
    cave_type		*c_ptr;
    char		*t;


    /* Choose lite or dark, and extract wall/floor codes */
    light = (dun_level <= randint(25));


    /* Pick a greater vault */
    switch (randint(4)) {

        case 4:
            width = 40;
            height = 18;
            sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
                    "%######################################%",
                    "%#*8..&##***++^^^^^^^^^^^^++***##&..*8#%",
                    "%#8..&##,,,,,##^^^^^^^^^^##,,,,,X#&..*#%",
                    "%#..&X#.....,.##^^^^^^^^##..&....##&..#%",
                    "%#.&##..,.&....##^^^^^^##..,...&..##&.#%",
                    "%#&##..*...&.^..##^^^^##..*....,..,##&#%",
                    "%####+#############++#############+####%",
                    "%+....,.,.#&&&&***+88+***&&&&#,.,.,...+%",
                    "%+...,.,.,#&&&&***+88+***&&&&#.,.,....+%",
                    "%####+#############++#############+####%",
                    "%#&##..*....&...##^^^^##...*...&,..X#&#%",
                    "%#.&##..&.^....##^^^^^^##....&....##&.#%",
                    "%#..&##....&..##^^^^^^^^##..,..*.##&..#%",
                    "%#*..&X#,,,,,##^^^^^^^^^^##,,,,,##&..8#%",
                    "%#8*..&##***++^^^^^^^^^^^^++***##&..*8#%",
                    "%######################################%",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
            break;

        case 3:
            width = 39;
            height = 17;
            rating += 20;
            sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
                    "%#####################################%",
                    "%+&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XO#%",
                    "%###################################X#%",
                    "%#OX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&#%",
                    "%#X###################################%",
                    "%#&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XO#%",
                    "%###################################X#%",
                    "%#OX&XOX&XOX&XOOOOOOOOOOOXOX&XOX&XOX&#%",
                    "%#X###################################%",
                    "%#&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XO#%",
                    "%###################################X#%",
                    "%#OX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&#%",
                    "%#X###################################%",
                    "%#&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&X&+%",
                    "%#####################################%",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
            break;

        case 2:
            width = 40;
            height = 18;
            rating += 10;
            sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
                    "%######################################%",
                    "%#,X,X,X,X,X,X,X,X*@@*X,X,X,X,X,X,X,X,#%",
                    "%#+################XX################+#%",
                    "%#.,..,.#&.&.,*##******##*,.&.&#.,...,X%",
                    "%#..,.^^#....,##***@@***##,....#^^..,.X%",
                    "%######+#^&.&##***@XX@***##&.&^#+######%",
                    "%#,.&.^^#+####***@X##X@***####+#^^.,..#%",
                    "%#..,&,.#^^^@#**@X#OO#X@**X@^^^#.,..&,#%",
                    "%#.,....#^^^@X**@X#OO#X@**#@^^^#.&.,..#%",
                    "%#...,^^#+####***@X##X@***####+#^^..,.#%",
                    "%######+#^&.&##***@XX@***##&.&^#+######%",
                    "%X.,..^^#.....##***@@***##,....#^^.,..#%",
                    "%X...,..#&.&.,*##******##*,.&.&#..,..,#%",
                    "%#+################XX################+#%",
                    "%#,X,X,X,X,X,X,X,X*@@*X,X,X,X,X,X,X,X,#%",
                    "%######################################%",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
            break;

        case 1:
            width = 40;
            height = 15;
            sprintf(dun->template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
                    "%&+.^..^..^..^..^..^..^..^..^..^..^..+&%",
                    "%+####################################+%",
                    "%.#.&.^,#&^&^#****+^*^@^#.*.&..#..*.,#.%",
                    "%^#.,.&^+^&^@#^^^^#@^*^*#....*^+.^...#^%",
                    "%.#*..,.###+####+####+###.&.^..#..&,.#.%",
                    "%^#..^.*#*..^&&@@*#,,,,,####+###,....#^%",
                    "%.##+##############,*O*,#,,,,,,###+###.%",
                    "%^#*&#.&,*.#,*&^*^#,,,,,#,,,,,,#....,#^%",
                    "%.#&,+....*+,*&^*^##########+###.,...+.%",
                    "%^#.,#.*.&.#,*&^*^+.,.&.^*.&^&^#.....#^%",
                    "%.#^*#.,..,#,*&^*^#*.^*.,..&&&^#,..,.#.%",
                    "%+####################################+%",
                    "%&+..^..^..^..^..^..^..^..^..^..^..^.+&%",
                    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
            break;
    }


    /* Standard rating boost */
    rating += 25;

    /* Message */
    if (cheat_room) msg_print("Greater Vault");

    /* (Sometimes) Cause a special feeling */
    if ((dun_level <= 50) ||
        (randint((dun_level-40) * (dun_level-40) + 1) < 400)) {
        good_item_flag = TRUE;
    }


    /* Scan the vault, grid by grid, symbol by symbol */
    for (t = dun->template, y1 = 0; y1 < height; y1++) {
        for (x1 = 0; x1 < width; x1++, t++) {

            /* Extract the location */
            x = xval - (width / 2) + x1;
            y = yval - (height / 2) + y1;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Hack -- skip "non-grids" */
            if (*t == ' ') continue;

            /* Start with every grid being a floor in the room */
            cave[y][x].info &= ~GRID_WALL_MASK;
            cave[y][x].info |= GRID_ROOM;
            if (light) cave[y][x].info |= GRID_GLOW;

            /* Analyze the grid */
            switch (*t) {

                /* Outer wall */
              case '%':
                cave[y][x].info |= (GRID_WALL_GRANITE | GRID_EXT1);
                break;

                /* Perma wall */	
              case '#':
                cave[y][x].info |= (GRID_WALL_GRANITE | GRID_PERM);
                break;

                /* Inner wall */
              case 'X':
                cave[y][x].info |= GRID_WALL_GRANITE;
                break;

              case '*':	   /* treasure/trap */
                if (randint(20) > 7) {
                    place_object(y, x);
                }
                else {
                    place_trap(y, x);
                }
                break;

              case '+':	   /* secret doors */
                place_secret_door(y, x);
                break;

              /* Trap */
              case '^':
                place_trap(y, x);
                break;

              case '.':	   /* floor */
                break;

              /* Monsters (see below) */
              case '&':
              case '@':
              case '8':
              case 'O':
              case ',':
                break;

              default:
                quit_fmt("Illegal vault creation char '%c'", *t);
                break;
            }
        }
    }


    /* Scan the vault, grid by grid, symbol by symbol -- place monsters */
    for (t = dun->template, y1 = 0; y1 < height; y1++) {
        for (x1 = 0; x1 < width; x1++, t++) {

            /* Extract the grid */
            x = xval - (width/2) + x1;
            y = yval - (height/2) + y1;

            /* Access the grid */
            c_ptr = &cave[y][x];

            /* Analyze the symbol */
            switch (*t) {

                /* Monster */
              case '&':
                monster_level = dun_level + 5;
                place_monster(y, x, get_mon_num(monster_level), TRUE);
                monster_level = dun_level;
                break;

                /* Meaner monster */
              case '@':
                monster_level = dun_level + 11;
                place_monster(y, x, get_mon_num(monster_level), TRUE);
                monster_level = dun_level;
                break;

                /* Meaner monster, plus treasure */
              case '8':
                monster_level = dun_level + 9;
                place_monster(y, x, get_mon_num(monster_level), TRUE);
                monster_level = dun_level;
                object_level = dun_level + 7;
                place_good(y, x, FALSE);
                object_level = dun_level;
                break;

                /* Nasty monster and treasure */
              case 'O':
                monster_level = dun_level + 40;
                place_monster(y, x, get_mon_num(monster_level), TRUE);
                monster_level = dun_level;
                object_level = dun_level + 20;
                place_good(y, x, TRUE);
                object_level = dun_level;
                break;

                /* Monster and/or object */
              case ',':
                if (rand_int(2) == 0) {
                    monster_level = dun_level + 3;
                    place_monster(y, x, get_mon_num(monster_level), TRUE);
                    monster_level = dun_level;
                }
                if (rand_int(2) == 0) {
                    object_level = dun_level + 7;
                    place_object(y, x);
                    object_level = dun_level;
                }
                break;
            }
        }
    }
}


/*
 * Constructs a tunnel between two points
 *
 * Note that this function is called BEFORE any streamers are made.
 *
 * We use the special flag "GRID_EXT1" to mark outer walls of rooms
 * We use the special flag "GRID_EXT2" to prevent corridor piercing
 *
 * We use the special flag "GRID_EXT3" for debugging
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
 * We permanently mark grids next to wall piercings to prevent two
 * different corridors from entering a room through adjacent grids,
 * which could result in "huge" entrances into rooms, and "silly"
 * placement of doors.
 *
 * Note that the GRID_EXT1 and GRID_EXT2 flags are used ONLY to control
 * the generation of tunnels by this function.
 */
static void build_tunnel(int row1, int col1, int row2, int col2)
{
    int        i, y, x;
    int			tmp_row, tmp_col;
    int                 row_dir, col_dir;
    int                 start_row, start_col;
    int			main_loop_count = 0;
    bool		door_flag = FALSE;
    cave_type		*c_ptr;


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
        if (rand_int(100) < DUN_TUN_CHG) {

            /* Acquire the correct direction */
            correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

            /* Random direction */
            if (rand_int(100) < DUN_TUN_RND) {
                rand_dir(&row_dir, &col_dir);
            }
        }

        /* Get the next location */
        tmp_row = row1 + row_dir;
        tmp_col = col1 + col_dir;


        /* Extremely Important -- do not leave the dungeon */
        while (!in_bounds(tmp_row, tmp_col)) {

            /* Acquire the correct direction */
            correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

            /* Random direction */
            if (rand_int(100) < DUN_TUN_RND) {
                rand_dir(&row_dir, &col_dir);
            }

            /* Get the next location */
            tmp_row = row1 + row_dir;
            tmp_col = col1 + col_dir;
        }


        /* Access the location */
        c_ptr = &cave[tmp_row][tmp_col];


        /* Mega-Hack -- avoid dangerous piercings */
        if (c_ptr->info & GRID_EXT2) {

            /* Do NOT use this location */
            continue;
        }

        /* Pierce the outer wall of a room */
        else if (c_ptr->info & GRID_EXT1) {

            /* Acquire the "next" location */
            y = tmp_row + row_dir;
            x = tmp_col + col_dir;

            /* Hack -- Avoid "dangerous" piercing paths */
            if (cave[y][x].info & GRID_EXT1) {

                /* Forbid entry */
                c_ptr->info |= GRID_EXT2;

                /* Do NOT use this location */
                continue;
            }

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
            for (y = row1 - 1; y <= row1 + 1; y++) {
                for (x = col1 - 1; x <= col1 + 1; x++) {

                    /* Forbid piercing of adjacent outer walls */
                    if (cave[y][x].info & GRID_EXT1) {

                        /* Do not pierce this wall */
                        cave[y][x].info |= GRID_EXT2;
                    }
                }
            }
        }

        /* Travel quickly through rooms */
        else if (c_ptr->info & GRID_ROOM) {

            /* Accept the location */
            row1 = tmp_row;
            col1 = tmp_col;
        }

        /* Tunnel through unused walls */
        else if (c_ptr->info & GRID_WALL_MASK) {

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
        }

        /* Handle corridor intersections or overlaps */
        else {

            /* Accept the location */
            row1 = tmp_row;
            col1 = tmp_col;

            /* Hack -- remember the location */
            c_ptr->info |= GRID_EXT3;

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
            if (randint(100) > DUN_TUN_CON) {

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

        /* Access the grid */
        y = dun->tunn[i].y;
        x = dun->tunn[i].x;

        /* Remove the wall */
        cave[y][x].info &= ~GRID_WALL_MASK;
    }


    /* Apply the piercings that we found */
    for (i = 0; i < dun->wall_n; i++) {

        /* Access the grid */
        y = dun->wall[i].y;
        x = dun->wall[i].x;

        /* Open a hole in the wall */
        cave[y][x].info &= ~GRID_WALL_MASK;

        /* Occasional doorway */
        if (rand_int(100) < DUN_TUN_PEN) {
            place_door(y, x);
        }

        /* Hack -- No longer an outer wall */
        cave[y][x].info &= ~GRID_EXT2;
        cave[y][x].info &= ~GRID_EXT1;
    }
}




/*
 * Count the number of "corridor grids" adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y1, x1)"
 *
 * XXX XXX Perhaps this routine should accept "closed doors"
 */
static int next_to_corr(int y1, int x1)
{
    int i, y, x, k = 0;

    /* Scan adjacent grids */
    for (i = 0; i < 4; i++) {

        /* Extract the grid location */
        y = y1 + ddy[ddd[i]];
        x = x1 + ddx[ddd[i]];

        /* Skip non floors */		
        if (!floor_grid_bold(y, x)) continue;

        /* Skip room floors (and doors) */
        if (cave[y][x].info & GRID_ROOM) continue;

        /* Skip "permanent" grids (stairs) */
        if (cave[y][x].info & GRID_PERM) continue;

        /* Count these grids */
        k++;
    }

    /* Return the number of corridors */
    return (k);
}


/*
 * Determine if the given location is "between" two walls,
 * and "next to" two corridor spaces.
 *
 * Assumes "in_bounds(y,x)"
 */
static bool possible_doorway(int y, int x)
{
    /* Count the adjacent corridors */
    if (next_to_corr(y, x) >= 2) {

        /* Vertical */
        if ((cave[y-1][x].info & GRID_WALL_MASK) &&
            (cave[y+1][x].info & GRID_WALL_MASK)) {
            return (TRUE);
        }

        /* Horizontal */
        if ((cave[y][x-1].info & GRID_WALL_MASK) &&
            (cave[y][x+1].info & GRID_WALL_MASK)) {
            return (TRUE);
        }
    }

    /* No doorway */
    return (FALSE);
}


/*
 * Places door at y, x position if at least 2 walls found	
 */
static void try_door(int y, int x)
{
    /* Paranoia */
    if (!in_bounds(y, x)) return;

    /* Ignore walls */
    if (cave[y][x].info & GRID_WALL_MASK) return;

    /* Ignore rooms */
    if (cave[y][x].info & GRID_ROOM) return;

    /* Occasional door (if allowed) */
    if ((rand_int(100) < DUN_TUN_JCT) && possible_doorway(y, x)) {

        /* Place a door */
        place_door(y, x);
    }
}




/*
 * Attempt to build a room of the given type at the given block
 */
static bool room_build(int y0, int x0, int typ)
{
    int y, x;

    int y1 = y0 + room[typ].dy1;
    int y2 = y0 + room[typ].dy2;
    int x1 = x0 + room[typ].dx1;
    int x2 = x0 + room[typ].dx2;

    /* Enforce maximums */
    if (dun->count[typ] >= room[typ].max) return (FALSE);

    /* Never run off the screen */
    if ((y1 < 0) || (y2 >= dun->row_rooms)) return (FALSE);
    if ((x1 < 0) || (x2 >= dun->col_rooms)) return (FALSE);

    /* Verify open space */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            if (dun->room_map[y][x]) return (FALSE);
        }
    }

    /* XXX XXX XXX It is *extremely important that the following */
    /* calculation is *exactly* correct to prevent memory errors */

    /* Acquire the location of the room */
    y = ((y1 + y2 + 1) * BLOCK_HGT) / 2;
    x = ((x1 + x2 + 1) * BLOCK_WID) / 2;

    /* Build a room */
    switch (typ) {

        /* Build an appropriate room */
        case 7: build_type7(y, x); break;
        case 6: build_type6(y, x); break;
        case 5: build_type5(y, x); break;
        case 4: build_type4(y, x); break;
        case 3: build_type3(y, x); break;
        case 2: build_type2(y, x); break;
        case 1: build_type1(y, x); break;

        /* Paranoia */
        default: return (FALSE);
    }

    /* Save the room location */
    if (dun->cent_n < CENT_MAX) {
        dun->cent[dun->cent_n].y = y;
        dun->cent[dun->cent_n].x = x;
        dun->cent_n++;
    }

    /* Reserve some blocks */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            dun->room_map[y][x] = TRUE;
        }
    }

    /* Count the rooms of this type */
    dun->count[typ]++;

    /* Success */
    return (TRUE);
}


/*
 * Cave logic flow for generation of new dungeon
 */
static void cave_gen(void)
{
    int		i, k, y, x, y1, x1;
    int		alloc_level;

    /* Assume not destroyed */
    bool destroyed = FALSE;


    /* Hack -- Start with granite */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {
            cave[y][x].info |= GRID_WALL_GRANITE;
        }
    }


    /* Special boundary walls -- Top and Bottom */
    for (x = 0; x < cur_wid; x++) {
        cave[0][x].info |= (GRID_PERM | GRID_EXT1 | GRID_EXT2);
        cave[cur_hgt-1][x].info |= (GRID_PERM | GRID_EXT1 | GRID_EXT2);
    }

    /* Special boundary walls -- Left and Right */
    for (y = 0; y < cur_hgt; y++) {
        cave[y][0].info |= (GRID_PERM | GRID_EXT1 | GRID_EXT2);
        cave[y][cur_wid-1].info |= (GRID_PERM | GRID_EXT1 | GRID_EXT2);
    }


    /* Possible "destroyed" level */
    if ((dun_level > 10) && (rand_int(DUN_DEST) == 0)) destroyed = TRUE;

    /* No destroyed "quest" levels */
    if (is_quest(dun_level)) destroyed = FALSE;


    /* Actual maximum number of rooms on this level */
    dun->row_rooms = cur_hgt / BLOCK_HGT;
    dun->col_rooms = cur_wid / BLOCK_WID;

    /* Initialize the room table */
    for (y = 0; y < dun->row_rooms; y++) {
        for (x = 0; x < dun->col_rooms; x++) {
            dun->room_map[y][x] = FALSE;
        }
    }

    /* Initialize the room count */
    for (i = 0; i < ROOM_MAX; i++) dun->count[i] = 0;


    /* No rooms yet */
    dun->cent_n = 0;

    /* Build some rooms */
    for (i = 0; i < DUN_ROOMS; i++) {

        /* Pick a block for the room */
        y = rand_int(dun->row_rooms);
        x = rand_int(dun->col_rooms);

        /* Align dungeon rooms */
        if (dungeon_align) {

            /* Slide some rooms right */
            if ((x % 3) == 0) x++;

            /* Slide some rooms left */
            if ((x % 3) == 2) x--;
        }

        /* Destroyed levels are boring */
        if (destroyed) {

            /* Attempt a "trivial" room */
            if (room_build(y,x,1)) continue;

            /* Never mind */
            continue;
        }

        /* Attempt an unusual room */
        if (dun_level > randint(DUN_UNUSUAL)) {

            /* Attempt a very unusual room */
            if (dun_level > randint(DUN_UNUSUAL)) {

                /* Attempt a greater vault */
                if ((rand_int(50) == 0) && room_build(y,x,7)) continue;

                /* Attempt a lesser vault */
                if ((rand_int(5) == 0) && room_build(y,x,6)) continue;

                /* Attempt a monster pit */
                if ((rand_int(5) == 0) && room_build(y,x,5)) continue;
            }

            /* Try an interesting room */
            k = rand_range(2, 4);

            /* Attempt to build it */
            if (room_build(y,x,k)) continue;
        }

        /* Attempt a trivial room */
        if (room_build(y,x,1)) continue;
    }


    /* Hack -- Scramble the room order */
    for (i = 0; i < dun->cent_n; i++) {
        int pick1 = rand_int(dun->cent_n);
        int pick2 = rand_int(dun->cent_n);
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
        build_tunnel(dun->cent[i].y, dun->cent[i].x, y, x);

        /* Remember the "previous" room */
        y = dun->cent[i].y;
        x = dun->cent[i].x;
    }


    /* Place intersection doors	 */
    for (i = 0; i < dun->door_n; i++) {

        /* Extract junction location */
        y = dun->door[i].y;
        x = dun->door[i].x;

        /* Try placing doors */
        try_door(y, x - 1);
        try_door(y, x + 1);
        try_door(y - 1, x);
        try_door(y + 1, x);
    }


    /* Add some magma streamers */
    for (i = 0; i < DUN_STR_MAG; i++) {
        build_streamer(GRID_WALL_MAGMA, DUN_STR_MC);
    }

    /* Add some quartz streamers */
    for (i = 0; i < DUN_STR_QUA; i++) {
        build_streamer(GRID_WALL_QUARTZ, DUN_STR_QC);
    }


    /* Destroy the level if necessary */
    if (destroyed) destroy_level();


    /* Always place some stairs */
    place_stairs(2, rand_range(3,4), 3);
    place_stairs(1, rand_range(1,2), 3);


    /* Determine the character location */
    new_player_spot();


    /* Number of monsters/objects */
    alloc_level = (dun_level / 3);
    if (alloc_level < 2) alloc_level = 2;
    else if (alloc_level > 10) alloc_level = 10;


    /* Put some monsters in the dungeon */
    alloc_monster((randint(8) + MIN_M_ALLOC_LEVEL + alloc_level), 0, TRUE);


    /* Put some rubble in corridors */
    alloc_object(ALLOC_SET_CORR, 1, randint(alloc_level));

    /* Put some objects in rooms */
    alloc_object(ALLOC_SET_ROOM, 5, randnor(DUN_AMT_ROOM, 3));

    /* Put some objects/gold in the dungeon */
    alloc_object(ALLOC_SET_BOTH, 5, randnor(DUN_AMT_ITEM, 3));
    alloc_object(ALLOC_SET_BOTH, 4, randnor(DUN_AMT_GOLD, 3));

    /* Place some traps in the dungeon */
    alloc_object(ALLOC_SET_BOTH, 3, randint(alloc_level));


#if 0
    /* XXX XXX XXX */
    /* Make sure to uncomment this block if anyone else wants */
    /* to use any of the "temporary flags".  I have left the */
    /* flags set for debugging purposes.  They are not needed. */

    /* Clear all the "temp" flags */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {
            cave[y][x].info &= ~GRID_EXT1;
            cave[y][x].info &= ~GRID_EXT2;
            cave[y][x].info &= ~GRID_EXT3;
            cave[y][x].info &= ~GRID_EXT4;
        }
    }
#endif


    /* Ghosts love to inhabit destroyed levels, but will live elsewhere */
    i = (destroyed) ? 11 : 1;

    /* Try to place the ghost */
    while (i-- > 0) {

        /* Attempt to place a ghost */
        if (place_ghost()) {

            /* Hack -- increase the rating */
            rating += 10;

            /* A ghost makes the level special */
            good_item_flag = TRUE;

            /* Stop trying to place the ghost */
            break;
        }
    }
}



/*
 * Builds a store at a given (row, column)
 *
 * Note that we never put a door in the "perma-wall" side.
 * This will allow us to "extend" the perma-walls by one grid.
 *
 * Note that the perma-walls are at x=0/65 and y=0/21
 * Thus the two "rows" of stores run from 2-9 and 12-19
 * And the four "columns" run 8-16, 22-30, 36-44, 50-58
 *
 * I am changing things to be a little more "interesting".
 *
 * The stores now lie inside boxes from 3-9 and 12-18 vertically,
 * and from 7-17, 21-31, 35-45, 49-59.  Note that there are thus
 * always at least 2 open grids between any disconnected walls.
 * And the stores can be a little bit "larger" in the horizontal.
 *
 * Note as well that the four "center" buildings will ALWAYS have
 * at least FOUR grids between them (to allow easy "running").
 */
static void build_store(int store_num, int yy, int xx)
{
    int                 y0, x0, y1, x1, y2, x2;
    int                 y = 0, x = 0, tmp;
    cave_type		*c_ptr;
    inven_type		*i_ptr;

    /* Find the "center" of the store */
    y0 = yy * 9 + 6;
    x0 = xx * 14 + 12;

    /* Determine the store boundaries */
    y1 = y0 - randint(2 + (yy == 0));
    y2 = y0 + randint(2 + (yy == 1));
    x1 = x0 - randint(5);
    x2 = x0 + randint(5);

    /* Build an invulnerable rectangular building */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            /* Get the grid */
            c_ptr = &cave[y][x];

            /* The buildings are granite */
            c_ptr->info |= GRID_WALL_GRANITE;

            /* The buildings are invincible */
            c_ptr->info |= GRID_PERM;

            /* Hack -- The buildings are illuminated */
            c_ptr->info |= GRID_GLOW;

            /* Hack -- The buildings are always known */
            c_ptr->info |= GRID_MARK;
        }
    }

    /* Pick a door direction (S,N,E,W) */
    tmp = rand_int(4);

    /* Pick again if the door is near the walls */
    if (((tmp == 0) && (yy == 1)) ||
        ((tmp == 1) && (yy == 0)) ||
        ((tmp == 2) && (xx == 3)) ||
        ((tmp == 3) && (xx == 0))) {

        /* Pick a new direction */
        tmp = rand_int(4);
    }

    /* Extract a "door location" */
    switch (tmp) {

        /* Bottom side */
        case 0:
            y = y2;
            x = rand_range(x1,x2);
            break;

        /* Top side */
        case 1:
            y = y1;
            x = rand_range(x1,x2);
            break;

        /* Right side */
        case 2:
            y = rand_range(y1,y2);
            x = x2;
            break;

        /* Left side */
        case 3:
            y = rand_range(y1,y2);
            x = x1;
            break;
    }


    /* Create a "door trap" */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_STORE_LIST + store_num);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Remove the granite wall */
    c_ptr->info &= ~GRID_WALL_MASK;
}




/*
 * Generate the "consistent" town features, and place the player
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 */
static void town_gen_hack(void)
{
    int        y, x, k, n;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    int                 rooms[MAX_STORES];


    /* Hack -- make the same town every time */
    set_seed(town_seed);

    /* Prepare an Array of "remaining stores", and count them */
    for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

    /* Place two rows of stores */
    for (y = 0; y < 2; y++) {

        /* Place four stores per row */
        for (x = 0; x < 4; x++) {

            /* Pick a random unplaced store */
            k = rand_int(n);

            /* Build that store at the proper location */
            build_store(rooms[k], y, x);

            /* Shift the stores down, remove one store */
            rooms[k] = rooms[--n];
        }
    }


    /* Place the stairs */
    while (TRUE) {

        /* Pick a location at least "three" from the outer walls */
        y = rand_range(3, cur_hgt - 4);
        x = rand_range(3, cur_wid - 4);

        /* Require a "naked" floor grid */
        if (naked_grid_bold(y, x)) break;
    }

    /* Create a "stair" object */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_DOWN_STAIR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Stairs are permanent */
    c_ptr->info |= GRID_PERM;

    /* Memorize the stairs */
    c_ptr->info |= GRID_MARK;


    /* Hack -- the player starts on the stairs */
    py = y;
    px = x;

    /* Mark the dungeon grid */
    cave[py][px].m_idx = 1;


    /* Hack -- undo the hack above */
    reset_seed();
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
static void town_gen(void)
{
    int        y, x;
    cave_type *c_ptr;


    /* Perma-walls -- North/South */
    for (x = 0; x < cur_wid; x++) {

        /* North wall */
        c_ptr = &cave[0][x];
        c_ptr->info |= GRID_WALL_GRANITE;
        c_ptr->info |= GRID_PERM;
        c_ptr->info |= GRID_GLOW;
        c_ptr->info |= GRID_MARK;

        /* South wall */
        c_ptr = &cave[cur_hgt-1][x];
        c_ptr->info |= GRID_WALL_GRANITE;
        c_ptr->info |= GRID_PERM;
        c_ptr->info |= GRID_GLOW;
        c_ptr->info |= GRID_MARK;
    }

    /* Perma-walls -- West/East */
    for (y = 0; y < cur_hgt; y++) {

        /* West wall */
        c_ptr = &cave[y][0];
        c_ptr->info |= GRID_WALL_GRANITE;
        c_ptr->info |= GRID_PERM;
        c_ptr->info |= GRID_GLOW;
        c_ptr->info |= GRID_MARK;

        /* East wall */
        c_ptr = &cave[y][cur_wid-1];
        c_ptr->info |= GRID_WALL_GRANITE;
        c_ptr->info |= GRID_PERM;
        c_ptr->info |= GRID_GLOW;
        c_ptr->info |= GRID_MARK;
    }


    /* Hack -- Build the buildings/stairs (from memory) */
    town_gen_hack();


    /* Day -- lite up the town */
    if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)) {

        /* Lite up the town */
        for (y = 0; y < cur_hgt; y++) {
            for (x = 0; x < cur_wid; x++) {

                /* Perma-Lite */
                cave[y][x].info |= GRID_GLOW;

                /* Memorize */
                if (view_perma_grids) cave[y][x].info |= GRID_MARK;
            }
        }

        /* Make some daytime residents */
        alloc_monster(MIN_M_ALLOC_TD, 3, TRUE);
    }

    /* Night -- make it dark */
    else {

        /* Make some night-time residents */
        alloc_monster(MIN_M_ALLOC_TN, 3, TRUE);
    }


    /* Potential ghost */
    place_ghost();
}


/*
 * Generates a random dungeon level			-RAK-	
 */
void generate_cave()
{
    int i;


    /* Hack -- Reset heaps */
    i_max = MIN_I_IDX;
    m_max = MIN_M_IDX;


    /* Start with a blank cave */
    for (i = 0; i < MAX_HGT; i++) {

        /* Wipe a whole row at a time */
        C_WIPE(cave[i], MAX_WID, cave_type);
    }


    /* Mega-Hack -- no panel yet */
    panel_row_min = 0;
    panel_row_max = 0;
    panel_col_min = 0;
    panel_col_max = 0;


    /* Reset the monster generation level */
    monster_level = dun_level;

    /* Reset the object generation level */
    object_level = dun_level;

    /* Nothing special here yet */
    good_item_flag = FALSE;

    /* Nothing good here yet */
    rating = 0;


    /* Build the town */
    if (!dun_level) {

        /* Small town */
        cur_hgt = SCREEN_HGT;
        cur_wid = SCREEN_WID;

        /* Determine number of panels */
        max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
        max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

        /* Assume illegal panel */
        panel_row = max_panel_rows;
        panel_col = max_panel_cols;

        /* Make a town */
        town_gen();
    }

    /* Build a real level */
    else {

        /* Big dungeon */
        cur_hgt = MAX_HGT;
        cur_wid = MAX_WID;

        /* Determine number of panels */
        max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
        max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

        /* Assume illegal panel */
        panel_row = max_panel_rows;
        panel_col = max_panel_cols;

        /* Make a dungeon */
        cave_gen();
    }

    /* Extract the feeling */
    if (rating > 100) feeling = 2;
    else if (rating > 80) feeling = 3;
    else if (rating > 60) feeling = 4;
    else if (rating > 40) feeling = 5;
    else if (rating > 30) feeling = 6;
    else if (rating > 20) feeling = 7;
    else if (rating > 10) feeling = 8;
    else if (rating > 0) feeling = 9;
    else feeling = 10;

    /* Have a special feeling, explicitly */
    if (good_item_flag && !p_ptr->preserve) feeling = 1;

    /* It takes 1000 game turns for "feelings" to recharge */
    if ((turn - old_turn) < 1000) feeling = 0;

    /* Hack -- no feeling in the town */
    if (!dun_level) feeling = 0;

    /* Remember when this level was "created" */
    old_turn = turn;
}

