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



typedef struct coords {
    int x, y;
} coords;


static coords doorstk[100];
static int    doorindex;




/*
 * Always picks a correct direction		 
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
    if (y1 < y2)
	*rdir = 1;
    else if (y1 == y2)
	*rdir = 0;
    else
	*rdir = (-1);

    if (x1 < x2)
	*cdir = 1;
    else if (x1 == x2)
	*cdir = 0;
    else
	*cdir = (-1);
	
    if ((*rdir != 0) && (*cdir != 0)) {
	if (randint(2) == 1)
	    *rdir = 0;
	else
	    *cdir = 0;
    }
}


/*
 * Chance of wandering direction			 
 */
static void rand_dir(int *rdir, int *cdir)
{
    /* Assume no motion */
    *cdir = *rdir = 0;
    
    switch (randint(4)) {
        case 1:
            *rdir = -1;
            break;
        case 2:
            *rdir = 1;
            break;
        case 3:
	    *cdir = -1;
            break;
        case 4:
	    *cdir = 1;
            break;
    }
}


/*
 * Returns random co-ordinates for player/monster/object
 */
static void new_player_spot(void)
{
    register int        y, x;
    register cave_type *c_ptr;

    /* Place the player */
    while (1) {

	/* Pick a legal spot */
	y = rand_range(1, cur_height - 2);
	x = rand_range(1, cur_width - 2);

	/* Must be a "naked" floor grid */
	if (!naked_grid_bold(y, x)) continue;

	/* Get the grid */
	c_ptr = &cave[y][x];

	/* Hack -- Never start in a vault */
	if (c_ptr->fval == VAULT_FLOOR) continue;

	/* Done */
	break;
    }    

    /* Save the new player grid */
    char_row = y;
    char_col = x;

    /* Mark the dungeon grid */
    cave[char_row][char_col].m_idx = 1;
}



/*
 * Count walls S/N/E/W of given grid, which is "in_bounds()".
 */
static int next_to_walls(int y, int x)
{
    register int        i = 0;
    register cave_type *c_ptr;

    c_ptr = &cave[y - 1][x];
    if (c_ptr->fval >= MIN_WALL) i++;
    c_ptr = &cave[y + 1][x];
    if (c_ptr->fval >= MIN_WALL) i++;
    c_ptr = &cave[y][x - 1];
    if (c_ptr->fval >= MIN_WALL) i++;
    c_ptr = &cave[y][x + 1];
    if (c_ptr->fval >= MIN_WALL) i++;

    return (i);
}

/*
 * Count corridors adjacent to given grid, which is in_bounds().
 */
static int next_to_corr(int y, int x)
{
    register int        k, j, i = 0;
    register cave_type *c_ptr;
    register inven_type *i_ptr;
    
    /* Scan the neighbors */    
    for (j = y - 1; j <= (y + 1); j++) {
	for (k = x - 1; k <= (x + 1); k++) {

	    /* Access the grid */
	    c_ptr = &cave[j][k];

	    /* Skip non-corridors */
	    if (c_ptr->fval != CORR_FLOOR) continue;

	    /* Access the item */
	    i_ptr = &i_list[c_ptr->i_idx];
	    
	    /* Skip doors */
	    if (i_ptr->tval == TV_OPEN_DOOR) continue;
	    if (i_ptr->tval == TV_CLOSED_DOOR) continue;
	    if (i_ptr->tval == TV_SECRET_DOOR) continue;

	    /* Paranoia -- skip stairs */
	    if (i_ptr->tval == TV_UP_STAIR) continue;
	    if (i_ptr->tval == TV_DOWN_STAIR) continue;
	    
	    /* Count these grids */
	    i++;
	}
    }

    /* Return the number of corridors */
    return (i);
}


/* 
 * Semi-Hack -- types of "places" we can allocate (below)
 */
#define ALLOC_SET_CORR	1
#define ALLOC_SET_ROOM	2
#define ALLOC_SET_BOTH	3


/*
 * Allocates an object for tunnels and rooms		-RAK-	 
 *
 * Type 1 is trap
 * Type 2 is unused (was visible traps)
 * Type 3 is rubble
 * Type 4 is gold
 * Type 5 is object
 */
static void alloc_object(int set, int typ, int num)
{
    register int y, x, k;

    /* Place some objects */
    for (k = 0; k < num; k++) {

	/* Don't put an object beneath the player, this could cause */
	/* problems if player is standing under rubble, or on a trap */

	/* Pick a "legal" spot */
	while (1) {

	    /* Location */
	    y = rand_int(cur_height);
	    x = rand_int(cur_width);

	    /* Require "naked" floor grid */
	    if (!naked_grid_bold(y, x)) continue;

	    /* Require corridor? */
	    if ((set == ALLOC_SET_CORR) && (cave[y][x].fval != CORR_FLOOR)) continue;

	    /* Forbid corridor? */
	    if ((set == ALLOC_SET_ROOM) && (cave[y][x].fval == CORR_FLOOR)) continue;

	    /* Accept it */
	    break;
	}

	if (typ < 4) {
	    if (typ == 1) {
		place_trap(y, x);
	    }
	    else /* (typ == 3) */ {
		place_rubble(y, x);
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
 * Blank a cave -- note new definition
 */
static void blank_cave(void)
{
    register int y;

    /* Clear each row */
    for (y = 0; y < MAX_HEIGHT; ++y) {

	/* Efficiency -- wipe a whole row at a time */
	C_WIPE(cave[y], MAX_WIDTH, cave_type);
    }
}


/*
 * Fills in "empty" spots with desired "floor code"
 * Note that the "outer walls" can not be changed.
 * Note that the "floor code" can be a "wall code".
 */
static void fill_cave(int fval)
{
    register int        y, x;
    register cave_type *c_ptr;

    /* Scan the cave (skip the outer walls) */
    for (y = 1; y < cur_height - 1; y++) {
	for (x = 1; x < cur_width - 1; x++) {
	    c_ptr = &cave[y][x];
	    if ((c_ptr->fval == NULL_WALL) ||
		(c_ptr->fval == TMP1_WALL) ||
		(c_ptr->fval == TMP2_WALL)) {
		c_ptr->fval = fval;
	    }
	}
    }
}


/*
 * Places indestructible rock around edges of dungeon
 */
static void place_boundary(void)
{
    register int        y, x;
    register cave_type *c_ptr;

    /* Top and bottom */
    for (x = 0; x < cur_width; x++) {
	c_ptr = &cave[0][x];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr = &cave[cur_height-1][x];
	c_ptr->fval = BOUNDARY_WALL;
    }

    /* Left and right */            
    for (y = 0; y < cur_height; y++) {
	c_ptr = &cave[y][0];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr = &cave[y][cur_width-1];
	c_ptr->fval = BOUNDARY_WALL;
    }
}


/*
 * Places "streamers" of rock through dungeon		-RAK-	 
 */
static void place_streamer(int fval, int treas_chance)
{
    int		i, tx, ty;
    int		y, x, dir;
    cave_type	*c_ptr;

    /* Hack -- Choose starting point */
    y = (cur_height / 2) + 10 - randint(21);
    x = (cur_width / 2) + 15 - randint(31);

    /* Choose a direction (not including "5") */
    dir = randint(8);
    if (dir > 4) dir++;

    /* Place streamer into dungeon */
    while (mmove(dir, &y, &x)) {

	/* One grid per density */
	for (i = 0; i < DUN_STR_DEN; i++) {
	
	    /* Pick a nearby grid */
	    ty = rand_range(y - DUN_STR_RNG, y + DUN_STR_RNG);
	    tx = rand_range(x - DUN_STR_RNG, x + DUN_STR_RNG);

	    /* Turn granite into streamers */
	    if (in_bounds(ty, tx)) {
		c_ptr = &cave[ty][tx];
		if (c_ptr->fval == GRANITE_WALL) {
		    c_ptr->fval = fval;
		    if (randint(treas_chance) == 1) {
			place_gold(ty, tx);
		    }
		}
	    }
	}
    }
}


/*
 * Similar to use in "spells1.c" but here we do not have to
 * update the view or anything.
 */
static void repl_spot(int y, int x, int typ)
{
    register cave_type *c_ptr;

    c_ptr = &cave[y][x];

    switch (typ) {
      case 1:
      case 2:
      case 3:
	c_ptr->fval = CORR_FLOOR;
	break;
      case 4:
      case 7:
      case 10:
	c_ptr->fval = GRANITE_WALL;
	break;
      case 5:
      case 8:
      case 11:
	c_ptr->fval = MAGMA_WALL;
	break;
      case 6:
      case 9:
      case 12:
	c_ptr->fval = QUARTZ_WALL;
	break;
    }

    /* This is no longer part of a room */
    c_ptr->info &= ~CAVE_LR;
    c_ptr->info &= ~CAVE_PL;
    c_ptr->info &= ~CAVE_FM;

    /* Delete any object at that location */
    delete_object(y, x);

    /* Delete any monster at that location */
    delete_monster(y, x);
}


/*
 * Build a destroyed level
 */
static void place_destroyed()
{
    register int y1, x1;
    register int y, x, k;
    int          n;

    /* Drop a few epi-centers (usually about two) */
    for (n = 1; n <= randint(5); n++) {

	/* Pick an epi-center */
	x1 = randint(cur_width - 32) + 15;
	y1 = randint(cur_height - 32) + 15;

	/* Earthquake! */
	for (y = (y1 - 15); y <= (y1 + 15); y++) {
	    for (x = (x1 - 15); x <= (x1 + 15); x++) {

		/* Do not destroy important (or illegal) stuff */
		if (valid_grid(y, x)) {
		    k = distance(y, x, y1, x1);
		    if (y == char_row && x == char_col) repl_spot(y, x, 1);
		    else if (k < 13) repl_spot(y, x, (int)randint(6));
		    else if (k < 16) repl_spot(y, x, (int)randint(9));
		}
	    }
	}
    }
}


/*
 * Chris Tate (fixer@faxcsl.dcrt.nih.gov) - optimized this code for size!
 * This code performs the common test in all of the place_* functions,
 * and returns c_ptr if we can go ahead and place the object, or NULL
 * if we can't.
 *
 * Note the use of "valid_grid()" to prevent artifact destruction
 * or stair (or store door) removal.  It also verifies "in_bounds(y,x)"
 */
static cave_type *test_place_obj(int y, int x)
{
    /* Don't hurt artifacts, or stairs */
    if (!valid_grid(y, x)) return (NULL);

    /* Destroy any object already there */
    delete_object(y, x);

    /* Return the cave pointer */
    return (&cave[y][x]);
}


static void place_open_door(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_OPEN_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;
    c_ptr->fval = CORR_FLOOR;
}


static void place_broken_door(int y, int x)
{
    int		cur_pos;
    cave_type	*c_ptr;
    inven_type	*i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_OPEN_DOOR);
    i_ptr->pval = 1;
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Hack -- nuke any walls */
    c_ptr->fval = CORR_FLOOR;
}


static void place_closed_door(int y, int x)
{
    int		cur_pos;
    cave_type	*c_ptr;
    inven_type	*i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_CLOSED_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Hack -- nuke any walls */
    c_ptr->fval = CORR_FLOOR;
}


static void place_locked_door(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_CLOSED_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Lock the door */
    i_ptr->pval = randint(10) + 10;

    /* Hack -- nuke any walls */
    c_ptr->fval = CORR_FLOOR;
}


static void place_stuck_door(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_CLOSED_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Stick the door */
    i_ptr->pval = (-randint(10) - 10);

    /* Hack -- nuke any walls */
    c_ptr->fval = CORR_FLOOR;
}


static void place_secret_door(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_SECRET_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Put the object in the cave */
    c_ptr->i_idx = cur_pos;

    /* Hack -- nuke any walls */
    c_ptr->fval = CORR_FLOOR;
}


static void place_door(int y, int x)
{
    register int        tmp;

    tmp = randint(8);
    if (tmp < 4) {
	if (randint(4) == 1)
	    place_broken_door(y, x);
	else
	    place_open_door(y, x);
    }
    else if (tmp < 7) {
	tmp = randint(100);
	if (tmp > 25)
	    place_closed_door(y, x);
	else if (tmp == 3)
	    place_stuck_door(y, x);
	else
	    place_locked_door(y, x);
    }
    else {
	place_secret_door(y, x);
    }
}


/*
 * Place an up staircase at given y, x			-RAK-	 
 */
static void place_up_stairs(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_UP_STAIR);
    i_ptr->iy = y;
    i_ptr->ix = x;
}


/*
 * Place a down staircase at given y, x			-RAK-	 
 */
static void place_down_stairs(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    /* Hack -- no stairs on quest levels */
    if (is_quest(dun_level)) {
	place_up_stairs(y, x);
	return;
    }

    c_ptr = test_place_obj(y,x);
    if (!c_ptr) return;

    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_DOWN_STAIR);
    i_ptr->iy = y;
    i_ptr->ix = x;
}


/*
 * Places a staircase 1=up, 2=down			-RAK-	 
 *
 * Note that all the function in this file seem to like "do...while".
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
		y = rand_int(cur_height);
		x = rand_int(cur_width);

		/* Require "naked" floor grid */
		if (!naked_grid_bold(y, x)) continue;

		/* Require a certain number of adjacent walls */
		if (next_to_walls(y, x) < walls) continue;

		/* Put the stairs here */
		if (typ == 1) {
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
 * Place a trap with a given displacement of point
 */
static void vault_trap_aux(int y, int x, int yd, int xd)
{
    int		count, y1, x1;

    /* Place traps */
    for (count = 0; count <= 5; count++) {

	/* Get a location */
	y1 = rand_spread(y, yd);
	x1 = rand_spread(x, xd);

	/* Require a legal grid */
	if (!in_bounds(y1, x1)) continue;
	
	/* Require "naked" floor grids */
	if (!naked_grid_bold(y1, x1)) continue;

	/* Hack -- see elsewhere for info */
	if (cave[y1][x1].fval == NULL_WALL) continue;

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
 * Place some monsters at the given location
 */
static void vault_monster(int y1, int x1, int num)
{
    int          i, y, x;

    /* Try to summon "num" monsters "near" the given location */
    for (i = 0; i < num; i++) {
	y = y1;
	x = x1;
	(void)summon_monster(&y1, &x1, TRUE);
    }
}

/*
 * Place a jelly at the given location
 */
static void vault_jelly(int y, int x)
{
    /* Hack -- allocate a simple sleeping jelly */
    while (1) {
	int m = rand_int(MAX_R_IDX-1);
	if (!strchr("jmi,", r_list[m].r_char)) continue;
	if (r_list[m].cflags2 & MF2_UNIQUE) continue;
	if (r_list[m].cflags2 & MF2_EVIL) continue;
	place_monster(y, x, m, TRUE);
	break;
    }
}

/*
 * Place an undead creature at the given location
 */
static void vault_undead(int y, int x)
{
    /* Hack -- allocate a sleeping non-unique undead */
    while (1) {
	int m = rand_int(MAX_R_IDX-1);
	if (!(r_list[m].cflags2 & MF2_UNDEAD)) continue;
	if (r_list[m].cflags2 & MF2_UNIQUE) continue;
	place_monster(y, x, m, TRUE);
	break;
    }
}



/*
 * Place the lowest level monster with matching name
 */
static void vault_aux(int y, int x, cptr what)
{
    register int i;

    /* Find and place the first matching monster */
    for (i = 0; i < MAX_R_IDX-1; i++) {
	if (!stricmp(r_list[i].name, what)) {
	    place_monster(y, x, i, FALSE);
	    break;
	}
    }
}


static cptr vault_orc_names[] = {
    "Snaga", "Black orc", "Black orc",
    "Uruk-Hai", "Uruk-Hai", "Orc captain"
};

static void vault_orc(int y, int x, int rank)
{
    vault_aux(y, x, vault_orc_names[rank-1]);
}


static cptr vault_troll_names[] = {
    "Forest troll", "Stone troll", "Ice troll",
    "Cave troll", "Water troll", "Olog-Hai"
};

static void vault_troll(int y, int x, int rank)
{
    vault_aux(y, x, vault_troll_names[rank-1]);
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
    "Nalfeshnee", "Marilith", "Lesser balrog"
};

static void vault_demon(int y, int x, int rank)
{
    vault_aux(y, x, vault_demon_names[rank-1]);
}


static cptr vault_giant_names[] = {
    "Hill giant", "Frost giant", "Fire giant",
    "Stone giant", "Cloud giant", "Storm giant"
};

static void vault_giant(int y, int x, int rank)
{
    vault_aux(y, x, vault_giant_names[rank-1]);
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
	vault_undead(y, x);
	break;
      case 5:
	vault_dragon(y, x, rank, colour);
	break;
      case 6:
	vault_demon(y, x, rank);
	break;
      case 7:
	vault_giant(y, x, rank);
	break;
    }
}







/*
 * Builds a room at a row, column coordinate		-RAK-	 
 *
 * for paranoia's sake: bounds-check!  Memory errors caused by accessing
 * cave[][] out-of-bounds are nearly impossible to spot!  -CFT 
 */
static void build_room(int yval, int xval)
{
    int			y, x, y2, x2;
    int                 y1, x1;
    byte               floor;
    bool		light;
    cave_type		*c_ptr;

    /* Choose lite or dark */
    light = (dun_level <= randint(25));
    floor = ROOM_FLOOR;

    /* Pick a room size */
    y1 = yval - randint(4);
    y2 = yval + randint(3);
    x1 = xval - randint(11);
    x2 = xval + randint(11);

    /* Paranoia -- check bounds! */
    if (y1 < 1) y1 = 1;
    if (x1 < 1) x1 = 1;
    if (y2 >= (cur_height - 1)) y2 = cur_height - 2;
    if (x2 >= (cur_width - 1)) x2 = cur_width - 2;


    /* Place a full floor under the room */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	for (x = x1 - 1; x <= x2 + 1; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->fval = floor;
	    c_ptr->info |= CAVE_LR;
	    if (light) c_ptr->info |= CAVE_PL;
	}
    }

    /* Walls around the room */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	c_ptr = &cave[y][x1 - 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr = &cave[y][x2 + 1];
	c_ptr->fval = GRANITE_WALL;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
	c_ptr = &cave[y1 - 1][x];
	c_ptr->fval = GRANITE_WALL;
	c_ptr = &cave[y2 + 1][x];
	c_ptr->fval = GRANITE_WALL;
    }


    /* Hack -- Occasional pillar room */
    if (randint(20) == 1) {
	for (y = y1; y <= y2; y += 2) {
	    for (x = x1; x <= x2; x += 2) {
		c_ptr = &cave[y][x];
		c_ptr->fval = TMP1_WALL;
	    }
	}
    }

    /* Hack -- Occasional ragged-edge room */
    else if (randint(50) == 1) {
	for (y = y1 + 2; y <= y2 - 2; y += 2) {
	    c_ptr = &cave[y][x1];
	    c_ptr->fval = TMP1_WALL;
	    c_ptr = &cave[y][x2];
	    c_ptr->fval = TMP1_WALL;
	}
	for (x = x1 + 2; x <= x2 - 2; x += 2) {
	    c_ptr = &cave[y1][x];
	    c_ptr->fval = TMP1_WALL;
	    c_ptr = &cave[y2][x];
	    c_ptr->fval = TMP1_WALL;
	}
    }
}


/*
 * Builds a room at a row, column coordinate		-RAK-
 * Type 1 unusual rooms are several overlapping rectangular ones	 
 */
static void build_type1(int yval, int xval)
{
    int                 y, x, y1, y2, x1, x2, i, limit;
    byte                floor;
    bool		light;
    cave_type		*c_ptr;
    int			y1a[3], y2a[3], x1a[3], x2a[3];
    
    /* Choose lite or dark */
    light = (dun_level <= randint(25));
    floor = ROOM_FLOOR;

    /* Let two or three rooms intersect */
    limit = 1 + randint(2);

    /* Lay down the rooms */
    for (i = 0; i < limit; i++) {

	/* Pick a room size */
	y1 = yval - randint(4);
	y2 = yval + randint(3);
	x1 = xval - randint(11);
	x2 = xval + randint(11);

	/* Paranoia -- bounds check! */
	if (y1 < 1) y1 = 1;
	if (x1 < 1) x1 = 1;
	if (y2 >= (cur_height - 1)) y2 = cur_height - 2;
	if (x2 >= (cur_width - 1)) x2 = cur_width - 2;

	/* Lay down a full floor */
	for (y = y1 - 1; y <= y2 + 1; y++) {
	    for (x = x1 - 1; x <= x2 + 1; x++) {
		c_ptr = &cave[y][x];
		c_ptr->fval = floor;
		c_ptr->info |= CAVE_LR;
		if (light) c_ptr->info |= CAVE_PL;
	    }
	}

	/* Walls */
	for (y = y1 - 1; y <= y2 + 1; y++) {
	    c_ptr = &cave[y][x1 - 1];
	    c_ptr->fval = GRANITE_WALL;
	    c_ptr = &cave[y][x2 + 1];
	    c_ptr->fval = GRANITE_WALL;
	}
	for (x = x1 - 1; x <= x2 + 1; x++) {
	    c_ptr = &cave[y1 - 1][x];
	    c_ptr->fval = GRANITE_WALL;
	    c_ptr = &cave[y2 + 1][x];
	    c_ptr->fval = GRANITE_WALL;
	}

	/* Save the room for use below */
	y1a[i] = y1, y2a[i] = y2, x1a[i] = x1, x2a[i] = x2;
    }
    

    /* Now erase all walls "inside" the rooms */
    for (i = 0; i < limit; i++) {

	/* Extract the room saved above */
	y1 = y1a[i], y2 = y2a[i], x1 = x1a[i], x2 = x2a[i];

	/* Lay down the interior floor */
	for (y = y1; y <= y2; y++) {
	    for (x = x1; x <= x2; x++) {
		c_ptr = &cave[y][x];
		c_ptr->fval = floor;
	    }
	}
    }
}


/*
 * Builds an "unusual room" at a row, column coordinate	-RAK-
 *
 * Type 2 unusual rooms all have an inner room:
 *	1 - Just an inner room with one door
 *	2 - An inner room within an inner room
 *	3 - An inner room with pillar(s)
 *	4 - Inner room has a maze
 *	5 - A set of four inner rooms
 */
static void build_type2(int yval, int xval)
{
    register int        y, x, y1, x1;
    int                 y2, x2, tmp;
    byte                floor;
    bool		light;
    cave_type		*c_ptr;


    /* Large room */
    y1 = yval - 4;
    y2 = yval + 4;
    x1 = xval - 11;
    x2 = xval + 11;

    /* Tight fit?  Use a simpler room instead */
    if (!in_bounds(y1, x1) || !in_bounds(y2, x2)) {
	build_type1(yval, xval);
	return;
    }


    /* Choose lite or dark */
    light = (dun_level <= randint(25));
    floor = ROOM_FLOOR;

   
    /* Place a full floor under the room */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	for (x = x1 - 1; x <= x2 + 1; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->fval = floor;
	    c_ptr->info |= CAVE_LR;
	    if (light) c_ptr->info |= CAVE_PL;
	}
    }

    /* Outer Walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	c_ptr = &cave[y][x1 - 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr = &cave[y][x2 + 1];
	c_ptr->fval = GRANITE_WALL;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
	c_ptr = &cave[y1 - 1][x];
	c_ptr->fval = GRANITE_WALL;
	c_ptr = &cave[y2 + 1][x];
	c_ptr->fval = GRANITE_WALL;
    }


    /* The inner room */
    y1 = y1 + 2;
    y2 = y2 - 2;
    x1 = x1 + 2;
    x2 = x2 - 2;

    /* Inner Walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	cave[y][x1 - 1].fval = TMP1_WALL;
	cave[y][x2 + 1].fval = TMP1_WALL;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
	cave[y1 - 1][x].fval = TMP1_WALL;
	cave[y2 + 1][x].fval = TMP1_WALL;
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
		cave[y][x].fval = TMP1_WALL;
	    }
	}

	/* Place a locked door on the inner room */
	switch (randint(4)) {
	    case 1: place_locked_door(yval - 1, xval); break;
	    case 2: place_locked_door(yval + 1, xval); break;
	    case 3: place_locked_door(yval, xval - 1); break;
	    case 4: place_locked_door(yval, xval + 1); break;
	}

	/* Place an object in the treasure vault	 */
	tmp = randint(10);
	if (tmp > 2) {
	    place_object(yval, xval);
	}
	else if (tmp == 2) {
	    place_down_stairs(yval, xval);
	}
	else /* tmp == 1 */ {
	    place_up_stairs(yval, xval);
	}

	/* Monsters to guard the treasure */
	vault_monster(yval, xval, 2 + randint(3));

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
		c_ptr = &cave[y][x];
		c_ptr->fval = TMP1_WALL;
	    }
	}

	/* Occasionally, two more Large Inner Pillars */
	if (randint(2) == 1) {
	    tmp = randint(2);
	    for (y = yval - 1; y <= yval + 1; y++) {
		for (x = xval - 5 - tmp; x <= xval - 3 - tmp; x++) {
		    c_ptr = &cave[y][x];
		    c_ptr->fval = TMP1_WALL;
		}
		for (x = xval + 3 + tmp; x <= xval + 5 + tmp; x++) {
		    c_ptr = &cave[y][x];
		    c_ptr->fval = TMP1_WALL;
		}
	    }
	}

	/* Occasionally, some Inner rooms */
	if (randint(3) == 1) {
	
	    /* Long horizontal walls */
	    for (x = xval - 5; x <= xval + 5; x++) {
		c_ptr = &cave[yval - 1][x];
		c_ptr->fval = TMP1_WALL;
		c_ptr = &cave[yval + 1][x];
		c_ptr->fval = TMP1_WALL;
	    }

	    /* Close off the left/right edges */
	    cave[yval][xval - 5].fval = TMP1_WALL;
	    cave[yval][xval + 5].fval = TMP1_WALL;

	    /* Secret doors (random top/bottom) */
	    place_secret_door(yval - 3 + (randint(2) << 1), xval - 3);
	    place_secret_door(yval - 3 + (randint(2) << 1), xval + 3);

	    /* Objects */
	    if (randint(3) == 1) place_object(yval, xval - 2);
	    if (randint(3) == 1) place_object(yval, xval + 2);
	    
	    /* Monsters */
	    vault_monster(yval, xval - 2, randint(2));
	    vault_monster(yval, xval + 2, randint(2));
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
		    cave[y][x].fval = TMP1_WALL;
		}
	    }
	}

	/* Monsters just love mazes.		 */
	vault_monster(yval, xval - 5, randint(3));
	vault_monster(yval, xval + 5, randint(3));

	/* Traps make them entertaining.	 */
	vault_trap(yval, xval - 3, 2, 8, randint(3));
	vault_trap(yval, xval + 3, 2, 8, randint(3));

	/* Mazes should have some treasure too..	 */
	for (y = 0; y < 3; y++) random_object(yval, xval, 1);
	
	break;


      /* Four small rooms. */
      case 5:

	/* Inner "cross" */
	for (y = y1; y <= y2; y++) {
	    cave[y][xval].fval = TMP1_WALL;
	}
	for (x = x1; x <= x2; x++) {
	    cave[yval][x].fval = TMP1_WALL;
	}

	/* Doors into the rooms */
	if (randint(2) == 1) {
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
	random_object(yval, xval, 2 + randint(2));

	/* Gotta have some monsters. */
	vault_monster(yval + 2, xval - 4, randint(2));
	vault_monster(yval + 2, xval + 4, randint(2));
	vault_monster(yval - 2, xval - 4, randint(2));
	vault_monster(yval - 2, xval + 4, randint(2));

	break;
    }
}


/*
 * Builds a room at a row, column coordinate
 * Type 3 unusual rooms are cross shaped	
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
    byte		floor;
    bool		light;
    cave_type		*c_ptr;


    /* Tight fit?  Try a simpler room.  This only happens near "edges". */
    if (!in_bounds(yval - 5, xval - 5) || !in_bounds(yval + 5, xval + 5)) {
	build_type1(yval, xval);
	return;
    }


    /* Choose lite or dark */
    light = (dun_level <= randint(25));
    floor = ROOM_FLOOR;


    /* For now, always 3x3 */
    wx = wy = 1;
    
    /* Pick max vertical size */
    dy = rand_range(3, 4);

    /* Pick max horizontal size */
    dx = rand_range(3, 11);


    /* Determine extents of the north/south room */
    y1a = yval - dy;
    y2a = yval + dy;
    x1a = xval - wx;
    x2a = xval + wx;

    /* Make sure it fits! */
    if (y1a < 1) y1a = 1;
    if (y2a > (cur_height - 2)) y2a = cur_height - 2;


    /* Determine extents of the east/west room */
    y1b = yval - wy;
    y2b = yval + wy;
    x1b = xval - dx;
    x2b = xval + dx;

    /* Paranoia -- bounds check */
    if (x1b < 1) x1b = 1;
    if (x2b > (cur_width - 2)) x2b = cur_width - 2;



    /* Place a full floor for room "a" */
    for (y = y1a - 1; y <= y2a + 1; y++) {
	for (x = x1a - 1; x <= x2a + 1; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->fval = floor;
	    c_ptr->info |= CAVE_LR;
	    if (light) c_ptr->info |= CAVE_PL;
	}
    }

    /* Place a full floor for room "b" */
    for (y = y1b - 1; y <= y2b + 1; y++) {
	for (x = x1b - 1; x <= x2b + 1; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->fval = floor;
	    c_ptr->info |= CAVE_LR;
	    if (light) c_ptr->info |= CAVE_PL;
	}
    }


    /* Place the walls around room "a" */
    for (y = y1a - 1; y <= y2a + 1; y++) {
	cave[y][x1a - 1].fval = GRANITE_WALL;
	cave[y][x2a + 1].fval = GRANITE_WALL;
    }
    for (x = x1a - 1; x <= x2a + 1; x++) {
	cave[y1a - 1][x].fval = GRANITE_WALL;
	cave[y2a + 1][x].fval = GRANITE_WALL;
    }

    /* Place the walls around room "b" */
    for (y = y1b - 1; y <= y2b + 1; y++) {
	cave[y][x1b - 1].fval = GRANITE_WALL;
	cave[y][x2b + 1].fval = GRANITE_WALL;
    }
    for (x = x1b - 1; x <= x2b + 1; x++) {
	cave[y1b - 1][x].fval = GRANITE_WALL;
	cave[y2b + 1][x].fval = GRANITE_WALL;
    }


    /* Replace the floor for room "a" */
    for (y = y1a; y <= y2a; y++) {
	for (x = x1a; x <= x2a; x++) {
	    cave[y][x].fval = floor;
	}
    }

    /* Replace the floor for room "b" */
    for (y = y1b; y <= y2b; y++) {
	for (x = x1b; x <= x2b; x++) {
	    cave[y][x].fval = floor;
	}
    }



    /* Special features. */
    switch (randint(4)) {

      /* Large middle pillar */
      case 1:
	for (y = y1b; y <= y2b; y++) {
	    for (x = x1a; x <= x2a; x++) {
		cave[y][x].fval = TMP1_WALL;
	    }
	}
	break;

      /* Inner treasure vault */
      case 2:

	/* Build the vault */
	for (y = y1b; y <= y2b; y++) {
	    cave[y][x1a].fval = TMP1_WALL;
	    cave[y][x2a].fval = TMP1_WALL;
	}
	for (x = x1a; x <= x2a; x++) {
	    cave[y1b][x].fval = TMP1_WALL;
	    cave[y2b][x].fval = TMP1_WALL;
	}

	/* Place a secret door on the inner room */
	switch (randint(4)) {
	    case 1: place_secret_door(y1b, xval); break;
	    case 2: place_secret_door(y2b, xval); break;
	    case 3: place_secret_door(yval, x1a); break;
	    case 4: place_secret_door(yval, x2a); break;
	}

	/* Place a treasure in the vault */
	place_object(yval, xval);

	/* Let's guard the treasure well */
	vault_monster(yval, xval, 2 + randint(2));

	/* Traps naturally */
	vault_trap(yval, xval, 4, 4, 1 + randint(3));

	break;


      /* Something else */
      case 3:

	/* Occasionally pinch the center shut */
	if (randint(3) == 1) {
	
	    /* Pinch the east/west sides */
	    for (y = y1b; y <= y2b; y++) {
		if (y == yval) continue;
		cave[y][x1a - 1].fval = TMP1_WALL;
		cave[y][x1a + 1].fval = TMP1_WALL;
	    }
	    
	    /* Pinch the north/south sides */
	    for (x = x1a; x <= x2a; x++) {
		if (x == xval) continue;
		cave[y1b - 1][x].fval = TMP1_WALL;
		cave[y2b + 1][x].fval = TMP1_WALL;
	    }
	    
	    /* Sometimes shut using secret doors */
	    if (randint(3) == 1) {
		place_secret_door(yval, x1a - 1);
		place_secret_door(yval, x2a + 1);
		place_secret_door(y1b - 1, xval);
		place_secret_door(y2b + 1, xval);
	    }
	}
	
	/* Occasionally put a "plus" in the center */
	else if (randint(3) == 1) {
	    cave[yval][xval].fval = TMP1_WALL;
	    cave[y1b][xval].fval = TMP1_WALL;
	    cave[y2b][xval].fval = TMP1_WALL;
	    cave[yval][x1a].fval = TMP1_WALL;
	    cave[yval][x2a].fval = TMP1_WALL;
	}

	/* Occasionally put a pillar in the center */
	else if (randint(3) == 1) {
	    cave[yval][xval].fval = TMP1_WALL;
	}

	break;

      /* Or, just be a normal cross */
      case 4:
	break;
    }
}


/*
 * Build various complicated vaults - Decado
 * This might be a good function to use external files.
 *
 * Vault char codes:
 *   Note that "space" means "not part of the room"
 */
static void build_type5(int yval, int xval)
{
    register int        x, y, x1, y1, vault;
    int                 width = 0, height = 0;

    byte		floor, wall;
    bool		light;

    cave_type		*c_ptr;


    /* Scan through the template */
    register char      *t;

    /* Note max size for rooms is set here */
    char		template[40*20+1];


    /* Assume Not a vault */
    vault = 0;

    /* Pick a large room */
    switch (randint(8)) {
    
      case 1:
	width = 20;
	height = 12;
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
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
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
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
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
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
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
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
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
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
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
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
	rating += 5;
	sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
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
	
      case 8:

	vault = TRUE;

	switch (randint(4)) {

	  case 4:
	    width = 40;
	    height = 18;
	    rating += 25;
	    sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
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
	    rating += 35;
	    sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
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
	    rating += 30;
	    sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
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
	    rating += 25;
	    sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
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

	break;
    }


    /* Choose lite or dark, and extract wall/floor codes */
    light = (dun_level <= randint(25));

    /* The walls in vaults must be permanent */
    wall = vault ? BOUNDARY_WALL : TMP1_WALL;

    /* The floors in vaults must be non-teleportable */
    floor = vault ? VAULT_FLOOR : ROOM_FLOOR;


    /* XXX XXX XXX XXX XXX XXX */
    /* Mega-Hack -- what the hell is this for? */
    if (vault) { xval += 4; yval += 4; }


    /* Tight fit?  Use a simpler room */
    if (!in_bounds(yval - (height / 2), xval - (width / 2)) ||
	!in_bounds(yval + (height / 2), xval + (width / 2))) {
	build_type2(yval, xval);
	return;
    }


    /* DO NOT CHANGE yval or xval AFTER THIS LINE */


    /* (Sometimes) Cause a special feeling */
    if ((dun_level <= 40) ||
	(randint((dun_level - 30)*(dun_level - 30) + 1) < 400)) {
	good_item_flag = TRUE;
    }


    /* Scan the vault, grid by grid, symbol by symbol */
    for (t = template, y = 0; y < height; y++) {
	for (x = 0; x < width; x++, t++) {

	    /* Do something */
	    x1 = xval - (width / 2) + x;
	    y1 = yval - (height / 2) + y;
	    c_ptr = &cave[y1][x1];

	    /* Hack -- skip "non-grids" */
	    if (*t == ' ') continue;
	    
	    /* Start with every grid being a floor in the room */
	    c_ptr->fval = floor;
	    c_ptr->info |= CAVE_LR;
	    
	    /* XXX This may not work */
	    if (light) c_ptr->info |= CAVE_PL;

	    /* Analyze the grid */
	    switch (*t) {

		/* Solid wall */	    
	      case '#':
		c_ptr->fval = wall;
		break;
		
		/* Temp wall */
	      case 'X':
		c_ptr->fval = TMP1_WALL;
		break;
		
		/* Granite wall */
	      case '%':
		c_ptr->fval = GRANITE_WALL;
		break;
		
	      case '*':	   /* treasure/trap */
		if (randint(20) > 7) {
		    place_object(y1, x1);
		}
		else if (randint(10) > 2) {
		    place_trap(y1, x1);
		}
		else if (randint(2) == 1 && !vault) {
		    place_down_stairs(y1, x1);
		}
		else if (!vault) {
		    place_up_stairs(y1, x1);
		}
		break;

	      case '+':	   /* secret doors */
		place_secret_door(y1, x1);
		break;

	      /* Trap */
	      case '^':
		place_trap(y1, x1);
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
#if 0
		quit_fmt("Illegal vault creation char '%c'", *t);
#endif
		break;
	    }
	}
    }


    /* Scan the vault, grid by grid, symbol by symbol -- place monsters */
    for (t = template, y = 0; y < height; y++) {
	for (x = 0; x < width; x++, t++) {

	    /* Extract the grid */
	    x1 = xval - (width/2) + x;
	    y1 = yval - (height/2) + y;
	    c_ptr = &cave[y1][x1];

	    /* Analyze the symbol */
	    switch (*t) {

		/* Monster */
	      case '&':
		place_monster(y1, x1,
			      get_mons_num(dun_level + MON_SUMMON_ADJ + 2 + vault),
			      TRUE);
		break;
		
		/* Meaner monster */
	      case '@':
		place_monster(y1, x1,
			      get_nmons_num(dun_level + MON_SUMMON_ADJ + 7),
			      TRUE);
		break;
		
		/* Meaner monster, plus treasure */
	      case '8':
		c_ptr->fval = floor;
		place_monster(y1, x1,
			      get_nmons_num(dun_level + MON_SUMMON_ADJ + 7),
			      TRUE);
		object_level = dun_level + 7;
		place_good(y1, x1, FALSE);
		object_level = dun_level;
		break;

		/* Nasty monster and treasure */
	      case 'O':
		place_monster(y1, x1,
			      get_nmons_num(dun_level + MON_SUMMON_ADJ + 40),
			      TRUE);
		object_level = dun_level + MON_SUMMON_ADJ + 20;
		place_good(y1, x1, TRUE);
		object_level = dun_level;
		break;
		
		/* Monster and/or object */
	      case ',':
		if (randint(2) == 1) {
		    place_monster(y1, x1,
				  get_mons_num(dun_level + MON_SUMMON_ADJ + vault),
				  TRUE);
		}
		if (randint(2) == 1) {
		    object_level = dun_level + 7;
		    place_object(y1, x1);
		    object_level = dun_level;
		}
		break;
	    }
	}
    }
}


/*
 * Help build a pit
 */
static void build_pit_aux(int yval, int xval, int type, int colour)
{
    register int        y, x, y1, x1;
    int                 y2, x2;
    byte               floor;
    bool		light;
    cave_type		*c_ptr;


    /* Large room */
    y1 = yval - 4;
    y2 = yval + 4;
    x1 = xval - 11;
    x2 = xval + 11;


    /* Tight fit?  Try a simpler room */
    if (!in_bounds(y1, x1) || !in_bounds(y2, x2)) {
	build_type1(yval,xval);
	return;
    }


    /* Pits are always dark */
    light = FALSE;

    /* Floor */
    floor = ROOM_FLOOR;


    /* Increase the level rating */
    rating += 10;
    

    /* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
    if ((randint(dun_level*dun_level + 1) < 300) && (dun_level <= 40)) {
	good_item_flag = TRUE;
    }


    /* Place the floor area */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	for (x = x1 - 1; x <= x2 + 1; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->fval = floor;
	    c_ptr->info |= CAVE_LR;
	    
	    /* XXX This may not work */
	    if (light) c_ptr->info |= CAVE_PL;
	}
    }

    /* Place the walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	cave[y][x1 - 1].fval = GRANITE_WALL;
	cave[y][x2 + 1].fval = GRANITE_WALL;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
	cave[y1 - 1][x].fval = GRANITE_WALL;
	cave[y2 + 1][x].fval = GRANITE_WALL;
    }


    /* Advance to the center room */
    y1 = y1 + 2;
    y2 = y2 - 2;
    x1 = x1 + 2;
    x2 = x2 - 2;

    /* The inner walls */
    for (y = y1 - 1; y <= y2 + 1; y++) {
	cave[y][x1 - 1].fval = TMP1_WALL;
	cave[y][x2 + 1].fval = TMP1_WALL;
    }
    for (x = x1 - 1; x <= x2 + 1; x++) {
	cave[y1 - 1][x].fval = TMP1_WALL;
	cave[y2 + 1][x].fval = TMP1_WALL;
    }


    /* Place a secret door */
    switch (randint(4)) {
	case 1: place_secret_door(y1 - 1, xval); break;
	case 2: place_secret_door(y2 + 1, xval); break;
	case 3: place_secret_door(yval, x1 - 1); break;
	case 4: place_secret_door(yval, x2 + 1); break;
    }


    /* Hack -- peek inside message */
    if (wizard || peek) {
	msg_print(format("Monster pit (type %d, colour %d)", type, colour));
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


static void build_pit(int yval, int xval)
{
    int tmp;

    tmp = randint(dun_level > 80 ? 80 : dun_level);
    
    if (tmp < 10) {
	build_pit_aux(yval, xval, 1, 0);
    }
    else if (tmp < 20) {
	build_pit_aux(yval, xval, 2, 0);
    }
    else if (tmp < 43) {
	if (randint(3) == 1) {
	    build_pit_aux(yval, xval, 7, 0);
	}
	else {
	    build_pit_aux(yval, xval, 3, 0);
	}
    }
    else if (tmp < 57) {
	build_pit_aux(yval, xval, 4, 0);
    }
    else if (tmp < 73) {
	build_pit_aux(yval, xval, 5, randint(6));
    }
    else {
	build_pit_aux(yval, xval, 6, 0);
    }
}




/*
 * Constructs a tunnel between two points
 *
 * Note that this function is called BEFORE any streamers are made. 
 * So fval's "QUARTZ_WALL" and "MAGMA_WALL" cannot be encountered.
 */
static void build_tunnel(int row1, int col1, int row2, int col2)
{
    register int        tmp_row, tmp_col, i, y, x;
    cave_type		*c_ptr;
    coords              tunstk[1000], wallstk[1000];
    coords		*cp;
    int                 row_dir, col_dir, tunindex, wallindex;
    int                 stop_flag, door_flag, main_loop_count;
    int                 start_row, start_col;

    stop_flag = FALSE;
    door_flag = FALSE;
    tunindex = 0;
    wallindex = 0;
    main_loop_count = 0;
    start_row = row1;
    start_col = col1;
    correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

    do {

	/* Paranoia -- prevent infinite loops */
	if (main_loop_count++ > 2000) stop_flag = TRUE;

	if (randint(100) > DUN_TUN_CHG) {
	    if (randint(DUN_TUN_RND) == 1)
		rand_dir(&row_dir, &col_dir);
	    else
		correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
	}
	tmp_row = row1 + row_dir;
	tmp_col = col1 + col_dir;
	while (!in_bounds(tmp_row, tmp_col)) {
	    if (randint(DUN_TUN_RND) == 1)
		rand_dir(&row_dir, &col_dir);
	    else
		correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
	    tmp_row = row1 + row_dir;
	    tmp_col = col1 + col_dir;
	}
	c_ptr = &cave[tmp_row][tmp_col];
	if (c_ptr->fval == NULL_WALL) {
	    row1 = tmp_row;
	    col1 = tmp_col;
	    if (tunindex < 1000) {
		tunstk[tunindex].y = row1;
		tunstk[tunindex].x = col1;
		tunindex++;
	    }
	    door_flag = FALSE;
	}
	else if (c_ptr->fval == TMP2_WALL) {
	    /* do nothing */
	}
	else if (c_ptr->fval == GRANITE_WALL) {

	    cave_type *d_ptr = &cave[tmp_row + row_dir][tmp_col + col_dir];

	    if ((d_ptr->fval == GRANITE_WALL) || (d_ptr->fval == TMP2_WALL)) {
		c_ptr->fval = TMP2_WALL;
	    }

	/* if can not pass completely through wall don't try... And mark as
	 * impassible for future -KOC 
	 */
	    else {

		/* Save the wall data */
		row1 = tmp_row;
		col1 = tmp_col;
		if (wallindex < 1000) {
		    wallstk[wallindex].y = row1;
		    wallstk[wallindex].x = col1;
		    wallindex++;
		}

		for (y = row1 - 1; y <= row1 + 1; y++) {
		    for (x = col1 - 1; x <= col1 + 1; x++) {
			if (in_bounds(y, x)) {
			    d_ptr = &cave[y][x];
			    if (d_ptr->fval == GRANITE_WALL) {
				d_ptr->fval = TMP2_WALL;
			    }
			}
		    }
		}
	    }
	}

	/* Check for corridor (or doors/rubble) */	
	else if (c_ptr->fval == CORR_FLOOR ||
		 (i_list[c_ptr->i_idx].tval == TV_RUBBLE) ||
		 (i_list[c_ptr->i_idx].tval == TV_CLOSED_DOOR) ||
		 (i_list[c_ptr->i_idx].tval == TV_SECRET_DOOR)) {

	    row1 = tmp_row;
	    col1 = tmp_col;
	    if (!door_flag) {
		if (doorindex < 100) {
		    doorstk[doorindex].y = row1;
		    doorstk[doorindex].x = col1;
		    doorindex++;
		}
		door_flag = TRUE;
	    }

	    /* make sure that tunnel has gone a reasonable distance before
	     * stopping it, this helps prevent isolated rooms 
	     */
	    if (randint(100) > DUN_TUN_CON) {
		tmp_row = row1 - start_row;
		if (tmp_row < 0) {
		    tmp_row = (-tmp_row);
		}
		tmp_col = col1 - start_col;
		if (tmp_col < 0) {
		    tmp_col = (-tmp_col);
		}
		if (tmp_row > 10 || tmp_col > 10) {
		    stop_flag = TRUE;
		}
	    }
	}

	/* c_ptr->fval != NULL, TMP2, GRANITE, CORR */
	else {
	    row1 = tmp_row;
	    col1 = tmp_col;
	}
    }
    while (((row1 != row2) || (col1 != col2)) && (!stop_flag));


    for (cp = &tunstk[0], i = 0; i < tunindex; i++, cp++) {
	cave[cp->y][cp->x].fval = CORR_FLOOR;
    }

    for (i = 0; i < wallindex; i++) {
	c_ptr = &cave[wallstk[i].y][wallstk[i].x];
	if (c_ptr->fval == TMP2_WALL) {
	    if (randint(100) < DUN_TUN_PEN) {
		place_door(wallstk[i].y, wallstk[i].x);
	    }
	    else {
		/* these have to be doorways to rooms */
		c_ptr->fval = CORR_FLOOR;
	    }
	}
    }
}


/*
 * Determine if the given location is between two walls,
 * and next to two corridor spaces.
 */
static bool possible_doorway(int y, int x)
{
    bool next = FALSE;

    /* Paranoia */
    if (!in_bounds(y, x)) return FALSE;

    /* Count the adjacent corridors */
    if (next_to_corr(y, x) > 2) {
	if ((cave[y - 1][x].fval >= MIN_WALL) &&
	    (cave[y + 1][x].fval >= MIN_WALL)) {
	    next = TRUE;
	}
	else if ((cave[y][x - 1].fval >= MIN_WALL) &&
		 (cave[y][x + 1].fval >= MIN_WALL)) {
	    next = TRUE;
	}
	else {
	    next = FALSE;
	}
    }

    return (next);
}

/*
 * Places door at y, x position if at least 2 walls found	 
 */
static void try_door(int y, int x)
{
    /* Paranoia */
    if (!in_bounds(y, x)) return;

    /* Floor, next to walls, and corridors */
    if ((cave[y][x].fval == CORR_FLOOR) &&
	(randint(100) > DUN_TUN_JCT) &&
	possible_doorway(y, x)) {

	/* Place a door */
	place_door(y, x);
    }
}


/*
 * Cave logic flow for generation of new dungeon
 */
static void cave_gen(void)
{
    int         room_map[20][20];
    int		i, k, pick1, pick2, tmp;
    int         y, x, y1, x1, y2, x2;
    int         row_rooms, col_rooms, alloc_level;
    s16b       yloc[400], xloc[400];

    /* Assume not destroyed */
    bool destroyed = FALSE;

    /* Assume no pits allowed */
    bool pit_ok = FALSE;

    if ((dun_level > 10) && (!is_quest(dun_level)) && (randint(DUN_DEST) == 1)) {
	if (wizard) msg_print("Destroyed Level");
	destroyed = TRUE;
    }
    else {
	pit_ok = TRUE;
    }

    row_rooms = 2 * (cur_height / SCREEN_HEIGHT);
    col_rooms = 2 * (cur_width / SCREEN_WIDTH);
    for (y = 0; y < row_rooms; y++) {
	for (x = 0; x < col_rooms; x++) {
	    room_map[y][x] = FALSE;
	}
    }

    k = randnor(DUN_ROO_MEA, 2);
    for (y = 0; y < k; y++) {
	room_map[randint(row_rooms) - 1][randint(col_rooms) - 1] = TRUE;
    }

    k = 0;
    for (y = 0; y < row_rooms; y++) {
	for (x = 0; x < col_rooms; x++) {
	    if (room_map[y][x]) {
		yloc[k] = y * (SCREEN_HEIGHT >> 1) + (SCREEN_HEIGHT >> 2);
		xloc[k] = x * (SCREEN_WIDTH >> 1) + (SCREEN_WIDTH >> 2);
		if (dun_level > randint(DUN_UNUSUAL)) {
		    tmp = randint(5);
		    if ((tmp == 1) || destroyed) {
			build_type1(yloc[k], xloc[k]);
		    }
		    else if (tmp == 2) {
			build_type2(yloc[k], xloc[k]);
		    }
		    else if (tmp == 3) {
			build_type3(yloc[k], xloc[k]);
		    }
		    else if ((tmp == 4) && (dun_level > randint(DUN_UNUSUAL))) {
			build_type5(yloc[k], xloc[k]);
			if (x + 1 < col_rooms) {
			    room_map[y][x + 1] = FALSE;
			}
			if (x + 1 < col_rooms && y + 1 < row_rooms) {
			    room_map[y + 1][x + 1] = FALSE;
			}
			if (x > 0 && y + 1 < row_rooms) {
			    room_map[y + 1][x - 1] = FALSE;
			}
			if (y + 1 < row_rooms) {
			    room_map[y + 1][x] = FALSE;
			}
		    }
		    else if (pit_ok && (dun_level > randint(DUN_UNUSUAL))) {
			build_pit(yloc[k], xloc[k]);
			pit_ok = FALSE;
		    }
		    else {
			build_room(yloc[k], xloc[k]);
		    }
		}
		else {
		    build_room(yloc[k], xloc[k]);
		}
		k++;
	    }
	}
    }

    for (y = 0; y < k; y++) {
	pick1 = rand_int(k);
	pick2 = rand_int(k);
	y1 = yloc[pick1];
	x1 = xloc[pick1];
	yloc[pick1] = yloc[pick2];
	xloc[pick1] = xloc[pick2];
	yloc[pick2] = y1;
	xloc[pick2] = x1;
    }

    doorindex = 0;

    /* move zero entry to k, so that can call build_tunnel all k times */
    yloc[k] = yloc[0];
    xloc[k] = xloc[0];
    for (i = 0; i < k; i++) {
	y1 = yloc[i];
	x1 = xloc[i];
	y2 = yloc[i + 1];
	x2 = xloc[i + 1];
	build_tunnel(y2, x2, y1, x1);
    }

    /* Fill the dungeon with walls */
    fill_cave(GRANITE_WALL);

    /* Add some streamers */
    for (i = 0; i < DUN_STR_MAG; i++) place_streamer(MAGMA_WALL, DUN_STR_MC);
    for (i = 0; i < DUN_STR_QUA; i++) place_streamer(QUARTZ_WALL, DUN_STR_QC);

    /* Place the boundary walls */
    place_boundary();

    /* Place intersection doors	 */
    for (i = 0; i < doorindex; i++) {
	try_door(doorstk[i].y, doorstk[i].x - 1);
	try_door(doorstk[i].y, doorstk[i].x + 1);
	try_door(doorstk[i].y - 1, doorstk[i].x);
	try_door(doorstk[i].y + 1, doorstk[i].x);
    }

    /* Destroy the level if necessary */
    if (destroyed) place_destroyed();

    /* What is this used for? */
    alloc_level = (dun_level / 3);
    if (alloc_level < 2) alloc_level = 2;
    else if (alloc_level > 10) alloc_level = 10;

    /* Always place some stairs */    
    place_stairs(2, rand_range(3,4), 3);
    place_stairs(1, rand_range(1,2), 3);

    /* Determine the character location */
    new_player_spot();

    /* Allocate some monsters */
    alloc_monster((randint(8) + MIN_M_ALLOC_LEVEL + alloc_level), 0, TRUE);

    /* Put some treasures in corridors */
    alloc_object(ALLOC_SET_CORR, 3, randint(alloc_level));

    /* Put some treasures in rooms */
    alloc_object(ALLOC_SET_ROOM, 5, randnor(TREAS_ROOM_ALLOC, 3));

    /* Put some treasures on the floor (not on doors) */
    alloc_object(ALLOC_SET_BOTH, 5, randnor(TREAS_ANY_ALLOC, 3));
    alloc_object(ALLOC_SET_BOTH, 4, randnor(TREAS_GOLD_ALLOC, 3));
    alloc_object(ALLOC_SET_BOTH, 1, randint(alloc_level));


    /* Ghosts love to inhabit destroyed levels, but will live elsewhere */
    i = (destroyed) ? 11 : 1;

    /* Try to place the ghost */
    while (i-- > 0) {

	/* Attempt to place a ghost */
	if (place_ghost()) {

	    /* A ghost makes the level special */
	    good_item_flag = TRUE;

	    /* Stop trying to place the ghost */
	    break;
	}
    }


    /* XXX XXX Hack -- Possibly place the winning monster */
    /* Perhaps should just use the normal monster allocators. */
    /* Note that they will automatically generate him eventually. */
    /* Also note that the code below means that he will NOT appear */
    /* if the dungeon level is entered without "making" him. */
    if ((dun_level >= WIN_MON_APPEAR) && (randint(5) < 4)) {
	place_win_monster();
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
    int                 y, x, cur_pos, tmp;
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

	    /* The buildings are invincible */
	    c_ptr->fval = BOUNDARY_WALL;

	    /* Hack -- The buildings are illuminated */
	    c_ptr->info |= CAVE_PL;

	    /* Hack -- The buildings are always known */
	    c_ptr->info |= CAVE_FM;
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
    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_STORE_LIST + store_num);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Make the store door be a floor plus the "door trap" */
    c_ptr = &cave[y][x];
    c_ptr->fval = CORR_FLOOR;
    c_ptr->i_idx = cur_pos;
}


/*
 * Places a down-staircase in the town
 *
 * By not using the standard "place_stairs()" function, we minimize
 * the number of functions affected by the RNG hack used in town_gen().
 */
static void place_town_stairs(void)
{
    int		y, x, cur_pos;

    cave_type	*c_ptr;
    inven_type	*i_ptr;


    /* Place the stairs */
    while (1) {

	/* Pick a location at least "three" from the outer walls */
	y = rand_range(3, cur_height - 4);
	x = rand_range(3, cur_width - 4);

	/* Require a "naked" floor grid */
	if (naked_grid_bold(y, x)) break;
    }

    /* Create a "stair" object */
    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_DOWN_STAIR);
    i_ptr->iy = y;
    i_ptr->ix = x;

    /* Place it in this grid */
    c_ptr = &cave[y][x];
    c_ptr->i_idx = cur_pos;

    /* Hack -- The stairs are "field marked" */
    c_ptr->info |= CAVE_FM;
}



/*
 * Generate the "consistent" town features
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 */
static void town_gen_hack(void)
{
    register int        y, x, k, n;
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

    /* Place the stairs. */
    place_town_stairs();

    /* Hack -- undo the hack above */
    reset_seed();
}




/*
 * Town logic flow for generation of new town		 
 *
 * Note that town_gen_hack() plays games with the R.N.G.
 *
 * This function does NOT do anything about the owners of the stores,
 * nor the contents thereof.  It only handles the physical layout.
 *
 * Note that "town_gen()", unlike "cave_gen()", actually takes note of
 * the "perma-lit" state of the town structures, and floors.
 *
 * Something similar needs to be done for "rooms".  Currently, for "rooms"
 * in "caves", we do a horrible hack involving "CAVE_PL" and "CAVE_INIT".
 * I would really prefer something involving just "CAVE_INIT".  That way,
 * the player can "see" lit rooms as he comes down the hall.  And a whole
 * set of "messy" issues are done away with.  Note that "CAVE_INIT" is
 * cleared at the start of every new level/session.  Note that we have to
 * be careful not to set "CAVE_FM" from "CAVE_PL" until the "CAVE_INIT"
 * code is executed.  Unless the savefile is pre-2.7.0.
 *
 * All the nasty details are explained in "cave.c".
 *
 * Hack -- since the player always leaves the dungeon by the stairs,
 * he is always placed on the stairs, even if he left the dungeon via
 * word of recall or teleport level.
 */
static void town_gen(void)
{
    register int        y, x;
    register cave_type *c_ptr;

    bool go = TRUE;


    /* Perma-walls -- North/South */
    for (x = 0; x < cur_width; x++) {

	/* North wall */
	c_ptr = &cave[0][x];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr->info |= CAVE_PL;
	c_ptr->info |= CAVE_FM;

	/* South wall */
	c_ptr = &cave[cur_height-1][x];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr->info |= CAVE_PL;
	c_ptr->info |= CAVE_FM;
    }

    /* Perma-walls -- West/East */
    for (y = 0; y < cur_height; y++) {

	/* West wall */
	c_ptr = &cave[y][0];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr->info |= CAVE_PL;
	c_ptr->info |= CAVE_FM;

	/* East wall */
	c_ptr = &cave[y][cur_width-1];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr->info |= CAVE_PL;
	c_ptr->info |= CAVE_FM;
    }


    /* Put dark floors inside the walls */
    for (y = 1; y < cur_height - 1; y++) {
	for (x = 1; x < cur_width - 1; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->fval = CORR_FLOOR;
	}
    }



    /* Hack -- Build the buildings/stairs (from memory) */
    town_gen_hack();


    /* Find the stairs */
    for (y = 1; go && y < cur_height-1; y++) {
	for (x = 1; go && x < cur_width-1; x++) {

	    /* Check for the stairs */
	    if (i_list[cave[y][x].i_idx].tval == TV_DOWN_STAIR) {

		/* Stop looking */
		go = FALSE;

		/* Save the new player grid */
		char_row = y;
		char_col = x;

		/* Mark the dungeon grid */
		cave[char_row][char_col].m_idx = 1;
	    }
	}
    }

    /* Paranoia -- Place the player in the town */
    if (go) new_player_spot();



    /* Day -- lite up the town */
    if ((turn % TOWN_DAWN) < (TOWN_DAWN / 2)) {

	/* Lite up the town */
	for (y = 0; y < cur_height; y++) {
	    for (x = 0; x < cur_width; x++) {

		c_ptr = &cave[y][x];

		/* Perma-Lite and Memorize */
		c_ptr->info |= CAVE_PL;
		c_ptr->info |= CAVE_FM;
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
    /* No current panel yet (actually a 1x1 panel) */
    panel_row_min = 0;
    panel_row_max = 0;
    panel_col_min = 0;
    panel_col_max = 0;

    /* No player yet (will be set before monster generation) */
    char_row = 0;
    char_col = 0;

    /* Totally wipe the object list */
    wipe_i_list();

    /* Totally wipe the monster list */
    wipe_m_list();

    /* Start with a blank cave */
    blank_cave();

    /* Important -- Reset the object generation level */
    object_level = dun_level;

    /* Nothing special here yet */
    good_item_flag = FALSE;

    /* Nothing good here yet */
    rating = 0;

    /* Build the town */
    if (!dun_level) {
	cur_height = SCREEN_HEIGHT;
	cur_width = SCREEN_WIDTH;
	max_panel_rows = (cur_height / SCREEN_HEIGHT) * 2 - 2;
	max_panel_cols = (cur_width / SCREEN_WIDTH) * 2 - 2;
	panel_row = max_panel_rows;
	panel_col = max_panel_cols;
	town_gen();
    }

    /* Build a real level */
    else {
	cur_height = MAX_HEIGHT;
	cur_width = MAX_WIDTH;
	max_panel_rows = (cur_height / SCREEN_HEIGHT) * 2 - 2;
	max_panel_cols = (cur_width / SCREEN_WIDTH) * 2 - 2;
	panel_row = max_panel_rows;
	panel_col = max_panel_cols;
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
    if (good_item_flag) feeling = 1;

    /* It takes 100 turns for "feelings" to recharge */
    if ((turn - old_turn) < 100) feeling = 0;

    /* Hack -- no feeling in the town */
    if (!dun_level) feeling = 0;

    /* Remember when this level was "created" */
    old_turn = turn;
}

