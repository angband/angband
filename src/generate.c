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


/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__
static void correct_dir(int *, int *, int, int, int, int);
static void rand_dir(int *, int *);
static void fill_cave(int);
static void place_streamer(int, int);
static cave_type *test_place_obj(int, int);
static void place_open_door(int, int);
static void place_broken_door(int, int);
static void place_closed_door(int, int);
static void place_locked_door(int, int);
static void place_stuck_door(int, int);
static void place_secret_door(int, int);
static void place_door(int, int);
static void place_up_stairs(int, int);
static void place_down_stairs(int, int);
static void place_stairs(int, int, int);
static void vault_trap(int, int, int, int, int);
static void vault_monster(int, int, int);
static void vault_jelly(int, int);
static void vault_orc(int, int, int);
static void vault_troll(int, int, int);
static void vault_undead(int, int);
static void vault_dragon(int, int, int, int);
static void vault_demon(int, int, int);
static void vault_giant(int, int, int);
static void vault_nasty(int, int, int, int, int);
static void build_room(int, int);
static void build_type1(int, int);
static void build_type5(int, int);
static void build_type2(int, int);
static void build_type3(int, int);
static void special_pit(int, int, int);
static void build_tunnel(int, int, int, int);
static int next_to(int, int);
static void try_door(int, int);
static void new_player_spot(void);
static void build_pit(int, int);
static void build_store(int, int, int);
static void place_boundary(void);
static void place_destroyed(void);
static void blank_cave(void);
static void cave_gen(void);
static void town_gen(void);
#endif
#endif

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
    register int tmp;

    tmp = randint(4);
    if (tmp < 3) {
	*cdir = 0;
	*rdir = (-3) + (tmp << 1);   /* tmp=1 -> *rdir=-1; tmp=2 -> *rdir=1 */
    }
    else {
	*rdir = 0;
	*cdir = (-7) + (tmp << 1);   /* tmp=3 -> *cdir=-1; tmp=4 -> *cdir=1 */
    }
}



/* 
 * Semi-Hack -- types of "places" we can allocate (below)
 */
#define ALLOC_SET_CORR	1
#define ALLOC_SET_ROOM	2
#define ALLOC_SET_BOTH	3


/*
 * Allocates an object for tunnels and rooms		-RAK-	 
 */
static void alloc_object(int set, int typ, int num)
{
    register int i, j, k;

    for (k = 0; k < num; k++) {

	/* don't put an object beneath the player, this could cause */
	/* problems if player is standing under rubble, or on a trap */

	/* Pick a "legal" spot */
	while (1) {

	    /* Location */
	    i = rand_int(cur_height);
	    j = rand_int(cur_width);
	    
	    /* Grid must be a clean floor grid */
	    if (!clean_grid(i, j)) continue;

	    /* No putting stuff under the player */
	    if (cave[i][j].m_idx == 1) continue;
	    
	    /* Require corridor? */
	    if ((set == ALLOC_SET_CORR) && (cave[i][j].fval != CORR_FLOOR)) continue;
	    
	    /* Forbid corridor? */
	    if ((set == ALLOC_SET_ROOM) && (cave[i][j].fval == CORR_FLOOR)) continue;

	    /* Accept it */
	    break;
	}

	/* typ == 2 not used, used to be visible traps */
	if (typ < 4) {
	    if (typ == 1)
		place_trap(i, j);
	    else /* (typ == 3) */
		place_rubble(i, j);
	}
	else {
	    object_level = dun_level;
	    if (typ == 4)
		place_gold(i, j);
	    else /* (typ == 5) */
		place_object(i, j);
	}
    }
}


/*
 * Blank a cave -- note new definition
 */
static void blank_cave(void)
{
    register int i;

    /* Clear each row */
    for (i = 0; i < MAX_HEIGHT; ++i) {

	/* Be very careful what you wipe! */
	C_WIPE(cave[i], MAX_WIDTH, cave_type);
    }
}


/*
 * Fills in empty spots with desired rock		-RAK-
 * Note: fval == 9 is a temporary value.				 
 */
static void fill_cave(int fval)
{
    register int        i, j;
    register cave_type *c_ptr;

/* no need to check the border of the cave */

    for (i = cur_height - 2; i > 0; i--) {
	c_ptr = &cave[i][1];
	for (j = cur_width - 2; j > 0; j--) {
	    if ((c_ptr->fval == NULL_WALL) ||
		(c_ptr->fval == TMP1_WALL) ||
		(c_ptr->fval == TMP2_WALL)) {
		c_ptr->fval = fval;
	    }
	    c_ptr++;
	}
    }
}

/*
 * Places indestructible rock around edges of dungeon	-RAK-	 
 * Trust me, efficiency is never more important than simplicity.
 */
static void place_boundary(void)
{
    register int        i;
    register cave_type *c_ptr;

    /* Top and bottom */
    for (i = 0; i < cur_width; i++) {
	c_ptr = &cave[0][i];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr = &cave[cur_height-1][i];
	c_ptr->fval = BOUNDARY_WALL;
    }

    /* Left and right */            
    for (i = 0; i < cur_height; i++) {
	c_ptr = &cave[i][0];
	c_ptr->fval = BOUNDARY_WALL;
	c_ptr = &cave[i][cur_width-1];
	c_ptr->fval = BOUNDARY_WALL;
    }
}


/*
 * Places "streamers" of rock through dungeon		-RAK-	 
 */
static void place_streamer(int fval, int treas_chance)
{
    register int        i, tx, ty;
    int                 y, x, t1, t2, dir;
    register cave_type *c_ptr;

   /* Choose starting point and direction */
    y = (cur_height / 2) + 11 - randint(23);
    x = (cur_width / 2) + 16 - randint(33);

    /* Number 1-4, 6-9	 */
    dir = randint(8);
    if (dir > 4) dir = dir + 1;

    /* Place streamer into dungeon */
    /* Constants	 */
    t1 = 2 * DUN_STR_RNG + 1;
    t2 = DUN_STR_RNG + 1;

    do {
	for (i = 0; i < DUN_STR_DEN; i++) {
	    ty = y + randint(t1) - t2;
	    tx = x + randint(t1) - t2;
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
    while (mmove(dir, &y, &x));
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
    register int y, x;
    register int i, j, k;
    int          n;

    /* Drop a few epi-centers (usually about two) */
    for (n = 1; n <= randint(5); n++) {

	/* Pick an epi-center */
	x = randint(cur_width - 32) + 15;
	y = randint(cur_height - 32) + 15;

	/* Earthquake! */
	for (i = (y - 15); i <= (y + 15); i++) {
	    for (j = (x - 15); j <= (x + 15); j++) {

		/* Do not destroy important (or illegal) stuff */
		if (valid_grid(i, j)) {
		    k = distance(i, j, y, x);
		    if (i == char_row && j == char_col) repl_spot(i, j, 1);
		    else if (k < 13) repl_spot(i, j, (int)randint(6));
		    else if (k < 16) repl_spot(i, j, (int)randint(9));
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
    cave_type *c_ptr;

    /* Don't hurt artifacts, or stairs */
    if (!valid_grid(y, x)) return (NULL);

    /* Get the cave location */
    c_ptr = &cave[y][x];

    /* Destroy any object already there */
    delete_object(y, x);

    /* Return the cave pointer */
    return c_ptr;
}


static void place_open_door(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    if (!(c_ptr = test_place_obj(y,x))) return;
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
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    if (!(c_ptr = test_place_obj(y,x))) return;
    cur_pos = i_pop();
    c_ptr->i_idx = cur_pos;
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_OPEN_DOOR);
    i_ptr->iy = y;
    i_ptr->ix = x;
    c_ptr->fval = CORR_FLOOR;
    i_ptr->pval = 1;
}


static void place_closed_door(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    if (!(c_ptr = test_place_obj(y,x))) return;
    
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

    if (!(c_ptr = test_place_obj(y,x))) return;

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

    if (!(c_ptr = test_place_obj(y,x))) return;

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

    if (!(c_ptr = test_place_obj(y,x))) return;

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
    } else if (tmp < 7) {
	tmp = randint(100);
	if (tmp > 25)
	    place_closed_door(y, x);
	else if (tmp == 3)
	    place_stuck_door(y, x);
	else
	    place_locked_door(y, x);
    } else
	place_secret_door(y, x);
}


/*
 * Place an up staircase at given y, x			-RAK-	 
 */
static void place_up_stairs(int y, int x)
{
    register int        cur_pos;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    if (!(c_ptr = test_place_obj(y,x))) return;
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

    if (is_quest(dun_level)) {
	place_up_stairs(y, x);
	return;
    }

    if (!(c_ptr = test_place_obj(y,x))) return;
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
    int                 i, j, flag;
    register int        y1, x1, y2, x2;

    for (i = 0; i < num; i++) {
	flag = FALSE;
	do {
	    j = 0;
	    do {

		/* Note: don't let y2==cur_height-1, or x2==cur_width-1, */
		/* or y1==0, or x1==0, these values are always BOUNDARY_ROCK. */

		y1 = randint(cur_height - 14);
		x1 = randint(cur_width - 14);
		y2 = y1 + 12;
		x2 = x1 + 12;

		do {
		    do {
			if (clean_grid(y1, x1) &&
			    (next_to_walls(y1, x1) >= walls)) {

			    flag = TRUE;

			    if (typ == 1) {
				place_up_stairs(y1, x1);
			    }
			    else {
				place_down_stairs(y1, x1);
			    }
			}
			x1++;
		    }
		    while ((x1 != x2) && (!flag));
		    x1 = x2 - 12;
		    y1++;
		}
		while ((y1 != y2) && (!flag));
		j++;
	    }
	    while ((!flag) && (j <= 30));
	    walls--;
	}
	while (!flag);
    }
}


/*
 * Place a trap with a given displacement of point	-RAK-	 
 */
static void vault_trap(int y, int x, int yd, int xd, int num)
{
    register int        count, y1, x1;
    int                 i, flag;
    register cave_type *c_ptr;

    for (i = 0; i < num; i++) {

	for (flag = FALSE, count = 0; (!flag) && (count <= 5); count++) {

	    do {		   /* add another bounds check -CFT */
		y1 = y - yd - 1 + randint(2 * yd + 1);
		x1 = x - xd - 1 + randint(2 * xd + 1);
	    } while (!in_bounds(y1, x1));

	    c_ptr = &cave[y1][x1];

	    /* Hack -- what is this for, anyway? */
	    if (c_ptr->fval == NULL_WALL) continue;

	    /* Only use empty floor space */
	    if (clean_grid(y1, x1)) {
		place_trap(y1, x1);
		flag = TRUE;
	    }
	}
    }
}


/*
 * Place a monster with a given displacement of point	-RAK-	 
 */
static void vault_monster(int y, int x, int num)
{
    register int i;
    int          y1, x1;

    for (i = 0; i < num; i++) {
	y1 = y;
	x1 = x;
	(void)summon_monster(&y1, &x1, TRUE);
    }
}

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
    int r_idx = -1, r_lev = -1;

    for (i = 0; i < MAX_R_IDX-1; ++i) {
	cptr name = r_list[i].name;
	if (stricmp(name, what)) continue;
	if (r_list[i].level > r_lev) continue;
	r_lev = r_list[i].level;
	r_idx = i;
    }    

    /* Place the monster (if any found) */
    if (r_idx >= 0) place_monster(y, x, r_idx, FALSE);
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
    register int        i, j, y_depth, x_right;
    int                 y_height, x_left;
    int8u               floor;
    bool		light;
    register cave_type *c_ptr, *d_ptr;

    if (dun_level <= randint(25)) {
	floor = LITE_FLOOR;	   /* Floor with light	 */
	light = TRUE;
    }
    else {
	floor = DARK_FLOOR;	   /* Dark floor		 */
	light = FALSE;
    }
    
    /* Pick a room size */
    y_height = yval - randint(4);
    y_depth = yval + randint(3);
    x_left = xval - randint(11);
    x_right = xval + randint(11);

    /* Paranoia -- check bounds! */
    if (y_height < 1) y_height = 1;
    if (x_left < 1) x_left = 1;
    if (y_depth >= (cur_height - 1)) y_depth = cur_height - 2;
    if (x_right >= (cur_width - 1)) x_right = cur_width - 2;

/*
 * the x dim of rooms tends to be much larger than the y dim,
 * so don't bother rewriting the y loop 
 */
    for (i = y_height; i <= y_depth; i++) {
	c_ptr = &cave[i][x_left];
	for (j = x_left; j <= x_right; j++) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = floor;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	    c_ptr++;
	}
    }

    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	c_ptr = &cave[i][x_left - 1];
	c_ptr->info |= CAVE_LR;
	c_ptr->fval = GRANITE_WALL;
	/* if (light) c_ptr->info |= CAVE_PL; */
	c_ptr = &cave[i][x_right + 1];
	c_ptr->info |= CAVE_LR;
	c_ptr->fval = GRANITE_WALL;
	/* if (light) c_ptr->info |= CAVE_PL; */
    }

    c_ptr = &cave[y_height - 1][x_left];
    d_ptr = &cave[y_depth + 1][x_left];
    for (i = x_left; i <= x_right; i++) {
	c_ptr->info |= CAVE_LR;
	c_ptr->fval = GRANITE_WALL;
	/* if (light) c_ptr->info |= CAVE_PL; */
	c_ptr++;
	d_ptr->info |= CAVE_LR;
	d_ptr->fval = GRANITE_WALL;
	/* if (light) c_ptr->info |= CAVE_PL; */
	d_ptr++;
    }

/* Every so often fill a normal room with pillars - Decado */

    if (randint(20) == 2) {
	for (i = y_height; i <= y_depth; i += 2) {
	    for (j = x_left; j <= x_right; j += 2) {
		c_ptr = &cave[i][j];
		c_ptr->fval = TMP1_WALL;
		c_ptr->info |= CAVE_LR;
	    }
	}
    }

    /* XXX XXX XXX XXX XXX Now where did these come from? */
    cave[y_height-1][x_left-1].info |= CAVE_LR;
    cave[y_height-1][x_right+1].info |= CAVE_LR;
}


/*
 * Builds a room at a row, column coordinate		-RAK-
 * Type 1 unusual rooms are several overlapping rectangular ones	 
 */
static void build_type1(int yval, int xval)
{
    int                 y_height, y_depth;
    int                 x_left, x_right, limit;
    register int        i0, i, j;
    int8u               floor;
    bool		light;
    register cave_type *c_ptr, *d_ptr;

    /* See above */
    if (dun_level <= randint(25)) {
	floor = LITE_FLOOR;	   /* Floor with light	 */
	light = TRUE;
    }
    else {
	floor = DARK_FLOOR;	   /* Dark floor		 */
	light = FALSE;
    }
    
    limit = 1 + randint(2);
    for (i0 = 0; i0 < limit; i0++) {

	/* Pick a room size */
	y_height = yval - randint(4);
	y_depth = yval + randint(3);
	x_left = xval - randint(11);
	x_right = xval + randint(11);

        /* Paranoia -- bounds check! */
	if (y_height < 1) y_height = 1;
	if (x_left < 1) x_left = 1;
	if (y_depth >= (cur_height - 1)) y_depth = cur_height - 2;
	if (x_right >= (cur_width - 1)) x_right = cur_width - 2;

    /* the x dim of rooms tends to be much larger than the y dim, so don't
     * bother rewriting the y loop 
     */

	for (i = y_height; i <= y_depth; i++) {
	    c_ptr = &cave[i][x_left];
	    for (j = x_left; j <= x_right; j++) {
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		c_ptr++;
	    }
	}
	for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	    c_ptr = &cave[i][x_left - 1];
	    if (c_ptr->fval != floor) {
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = GRANITE_WALL;
		/* if (light) c_ptr->info |= CAVE_PL; */
	    }
	    c_ptr = &cave[i][x_right + 1];
	    if (c_ptr->fval != floor) {
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = GRANITE_WALL;
		/* if (light) c_ptr->info |= CAVE_PL; */
	    }
	}
	c_ptr = &cave[y_height - 1][x_left];
	d_ptr = &cave[y_depth + 1][x_left];
	for (i = x_left; i <= x_right; i++) {
	    if (c_ptr->fval != floor) {
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = GRANITE_WALL;
		/* if (light) c_ptr->info |= CAVE_PL; */
	    }
	    c_ptr++;
	    if (d_ptr->fval != floor) {
		d_ptr->info |= CAVE_LR;
		d_ptr->fval = GRANITE_WALL;
		/* if (light) d_ptr->info |= CAVE_PL; */
	    }
	    d_ptr++;
	}
    }
}


/*
 * Wish me LUCK! - Decado
 */
static void build_type5(int yval, int xval)
{
    register int        x, y, x1, y1, vault;
    int                 width = 0, height = 0;

    int8u               floor;
    bool		light;

    int8u               wall;
    register cave_type *c_ptr;

    /* Scan through the template */
    register char      *t;

    /* Note max size for rooms is set here */
    char		template[40*20+1];

    /* See above */
    if (dun_level <= randint(25)) {
	floor = LITE_FLOOR;	   /* Floor with light */
	light = TRUE;
    }
    else {
	floor = DARK_FLOOR;	   /* Dark floor       */
	light = FALSE;
    }


    vault = 0;
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

/* check these rooms will fit on the map! */

    wall = TMP1_WALL;
    if (vault) {
	xval += 4;		   /* neat kludge deccy... */
	yval += 4;
	wall = BOUNDARY_WALL;
	if (floor == LITE_FLOOR) {
	    floor = NT_LITE_FLOOR;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
	else {
	    floor = NT_DARK_FLOOR;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
    }

    /* DO NOT CHANGE yval or xval after this check... */
    if (!(in_bounds(yval - (height / 2), xval - (width / 2)) &&
	  in_bounds(yval + (height / 2), xval + (width / 2)))) {
	build_type2(yval, xval);
	return;
    }

    /* (Sometimes) Cause a special feeling */
    if ((dun_level <= 40) ||
	(randint((dun_level - 30)*(dun_level - 30) + 1) < 400)) {
	good_item_flag = TRUE;
    }

    /* this avoids memory troubles... see above -CFT */
    t = template;
    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    x1 = xval - (width / 2) + x;
	    y1 = yval - (height / 2) + y;
	    c_ptr = &cave[y1][x1];
	    switch (*t++) {
	      case '#':	   /* lit up wall */
		c_ptr->fval = wall;
		c_ptr->info |= CAVE_LR;
		break;
	      case 'X':
		c_ptr->fval = TMP1_WALL;
		c_ptr->info |= CAVE_LR;
		break;
	      case '%':	   /* lit up wall */
		c_ptr->fval = GRANITE_WALL; /* Not temp, since this may have doors in */
		c_ptr->info |= CAVE_LR;
		break;
	      case '.':	   /* lit up/ not lit up floor */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		break;
	      case '*':	   /* treasure/trap */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		if (randint(20) > 7) {
		    object_level = dun_level;
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
		c_ptr->info |= CAVE_LR;
		break;
	      case '^':
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		place_trap(y1, x1);
		break;
	      case ' ':
		break;
	      case '&': case '@': case '8': case 'O': case ',':
		/* do nothing for now... cannot place monster until whole
		 * area is built, OW group monsters screw things up by being
		 * placed where walls will be placed on top of them  -CFT
		 */
		break;
	      default:
#if 0
		sprintf(buf, "Cockup='%c'", *t);
		msg_print(buf);
#endif
		break;
	    }
	}
    }

    /* now we go back and place the monsters, */
    /* hopefully this makes everything happy... -CFT */
    t = template;
    for (y=0; y<height; y++) {
	for (x=0; x<width; x++) {
	    x1=xval-(width/2)+x;
	    y1=yval-(height/2)+y;
	    c_ptr = &cave[y1][x1];
	    switch (*t++) {
	      case '#': case 'X': case '%': case '*':
	      case '+': case '^': case '.':
		/* we already placed all of these... -CFT */
		break;

	      case '&':	   /* Monster */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		place_monster(y1, x1,
			      get_mons_num(dun_level + MON_SUMMON_ADJ + 2 + vault),
			      TRUE);
		break;
	      case '@':	   /* Meaner monster */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		place_monster(y1, x1,
			      get_nmons_num(dun_level + MON_SUMMON_ADJ + 7),
			      TRUE);
		break;
	      case '8':	   /* Meaner monster */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		place_monster(y1, x1,
			      get_nmons_num(dun_level + MON_SUMMON_ADJ + 7),
			      TRUE);
		place_good(y1, x1, FALSE);
		break;
	      case 'O':	   /* Nasty monster and treasure */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		place_monster(y1, x1,
			      get_nmons_num(dun_level + MON_SUMMON_ADJ + 40),
			      TRUE);
		object_level = dun_level + MON_SUMMON_ADJ + 20;
		place_good(y1, x1, TRUE);
		object_level = dun_level + 7;
		break;
	      case ',':	   /* Monster/object */
		c_ptr->info |= CAVE_LR;
		c_ptr->fval = floor;
		/* if (light) c_ptr->info |= CAVE_PL; */
		if (randint(2) == 1) {
		    place_monster(y1, x1,
				  get_mons_num(dun_level + MON_SUMMON_ADJ + vault),
				  TRUE);
		}
		if (randint(2) == 1) {
		    object_level = dun_level + 7;
		    place_object(y1, x1);
		}
		break;
	      case ' ':
		break;
	      default:
#if 0
		sprintf(buf, "Cockup='%c'", *t);
		msg_print(buf);
#endif
		break;
	    }
	}
    }
}

/*
 * Builds an unusual room at a row, column coordinate	-RAK-
 *
 * Type 2 unusual rooms all have an inner room:
 * 1 - Just an inner room with one door
 * 2 - An inner room within an inner room
 * 3 - An inner room with pillar(s)
 * 4 - Inner room has a maze
 * 5 - A set of four inner rooms
 */
static void build_type2(int yval, int xval)
{
    register int        i, j, y_height, x_left;
    int                 y_depth, x_right, tmp;
    int8u               floor;
    bool		light;
    register cave_type *c_ptr, *d_ptr;

    /* See above */
    if (dun_level <= randint(25)) {
	floor = LITE_FLOOR;	   /* Floor with light	 */
	light = TRUE;
    }
    else {
	floor = DARK_FLOOR;	   /* Dark floor		 */
	light = FALSE;
    }

    y_height = yval - 4;
    y_depth = yval + 4;
    x_left = xval - 11;
    x_right = xval + 11;

/* paranoia bounds-check...   if the room can't fit in the cave, punt out and
 * build a simpler room type (type1 is already bounds-checked) This should
 * help solve MORE memory troubles! -CFT 
 */
    if (!in_bounds(y_height, x_left) || !in_bounds(y_depth, x_right)) {
	build_type1(yval, xval);
	return;
    }

/*
 * the x dim of rooms tends to be much larger than the y dim, so don't bother
 * rewriting the y loop 
 */

    for (i = y_height; i <= y_depth; i++) {
	c_ptr = &cave[i][x_left];
	for (j = x_left; j <= x_right; j++) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = floor;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	    c_ptr++;
	}
    }
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	c_ptr = &cave[i][x_left - 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
	c_ptr = &cave[i][x_right + 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
    }
    c_ptr = &cave[y_height - 1][x_left];
    d_ptr = &cave[y_depth + 1][x_left];
    for (i = x_left; i <= x_right; i++) {
	c_ptr->info |= CAVE_LR;
	c_ptr->fval = GRANITE_WALL;
	c_ptr++;
	d_ptr->info |= CAVE_LR;
	d_ptr->fval = GRANITE_WALL;
	d_ptr++;
    }
/* The inner room		 */
    y_height = y_height + 2;
    y_depth = y_depth - 2;
    x_left = x_left + 2;
    x_right = x_right - 2;
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	cave[i][x_left - 1].fval = TMP1_WALL;
	cave[i][x_right + 1].fval = TMP1_WALL;
    }
    c_ptr = &cave[y_height - 1][x_left];
    d_ptr = &cave[y_depth + 1][x_left];
    for (i = x_left; i <= x_right; i++) {
	c_ptr->fval = TMP1_WALL;
	c_ptr++;
	d_ptr->fval = TMP1_WALL;
	d_ptr++;
    }

/* Inner room variations		 */
    switch (randint(5)) {
      case 1:			   /* Just an inner room.	 */
	tmp = randint(4);
	if (tmp < 3) {		   /* Place a door	 */
	    if (tmp == 1)
		place_secret_door(y_height - 1, xval);
	    else
		place_secret_door(y_depth + 1, xval);
	}
	else {
	    if (tmp == 3)
		place_secret_door(yval, x_left - 1);
	    else
		place_secret_door(yval, x_right + 1);
	}
	vault_monster(yval, xval, 1);
	break;

      case 2:			   /* Treasure Vault	 */
	tmp = randint(4);
	if (tmp < 3) {		   /* Place a door	 */
	    if (tmp == 1)
		place_secret_door(y_height - 1, xval);
	    else
		place_secret_door(y_depth + 1, xval);
	}
	else {
	    if (tmp == 3)
		place_secret_door(yval, x_left - 1);
	    else
		place_secret_door(yval, x_right + 1);
	}

	for (i = yval - 1; i <= yval + 1; i++) {
	    cave[i][xval - 1].fval = TMP1_WALL;
	    cave[i][xval + 1].fval = TMP1_WALL;
	}
	cave[yval - 1][xval].fval = TMP1_WALL;
	cave[yval + 1][xval].fval = TMP1_WALL;

	tmp = randint(4);	   /* Place a door	 */
	if (tmp < 3)
	    place_locked_door(yval - 3 + (tmp << 1), xval); /* 1 -> yval-1; 2 -> yval+1 */
	else
	    place_locked_door(yval, xval - 7 + (tmp << 1));

    /* Place an object in the treasure vault	 */
	tmp = randint(10);
	if (tmp > 2) {
	    object_level = dun_level;
	    place_object(yval, xval);
	}
	else if (tmp == 2)
	    place_down_stairs(yval, xval);
	else
	    place_up_stairs(yval, xval);

    /* Guard the treasure well		 */
	vault_monster(yval, xval, 2 + randint(3));
    /* If the monsters don't get 'em.	 */
	vault_trap(yval, xval, 4, 10, 2 + randint(3));
	break;

      case 3:			   /* Inner pillar(s).	 */
	tmp = randint(4);
	if (tmp < 3) {		   /* Place a door	 */
	    if (tmp == 1)
		place_secret_door(y_height - 1, xval);
	    else
		place_secret_door(y_depth + 1, xval);
	}
	else {
	    if (tmp == 3)
		place_secret_door(yval, x_left - 1);
	    else
		place_secret_door(yval, x_right + 1);
	}

	for (i = yval - 1; i <= yval + 1; i++) {
	    c_ptr = &cave[i][xval - 1];
	    for (j = xval - 1; j <= xval + 1; j++) {
		c_ptr->fval = TMP1_WALL;
		c_ptr++;
	    }
	}
	if (randint(2) == 1) {
	    tmp = randint(2);
	    for (i = yval - 1; i <= yval + 1; i++) {
		c_ptr = &cave[i][xval - 5 - tmp];
		for (j = xval - 5 - tmp; j <= xval - 3 - tmp; j++) {
		    c_ptr->fval = TMP1_WALL;
		    c_ptr++;
		}
	    }
	    for (i = yval - 1; i <= yval + 1; i++) {
		c_ptr = &cave[i][xval + 3 + tmp];
		for (j = xval + 3 + tmp; j <= xval + 5 + tmp; j++) {
		    c_ptr->fval = TMP1_WALL;
		    c_ptr++;
		}
	    }
	}
	
	/* Inner rooms	 */
	if (randint(3) == 1) {
	    c_ptr = &cave[yval - 1][xval - 5];
	    d_ptr = &cave[yval + 1][xval - 5];
	    for (i = xval - 5; i <= xval + 5; i++) {
		c_ptr->fval = TMP1_WALL;
		c_ptr++;
		d_ptr->fval = TMP1_WALL;
		d_ptr++;
	    }
	    cave[yval][xval - 5].fval = TMP1_WALL;
	    cave[yval][xval + 5].fval = TMP1_WALL;
	    place_secret_door(yval - 3 + (randint(2) << 1), xval - 3);
	    place_secret_door(yval - 3 + (randint(2) << 1), xval + 3);
	    object_level = dun_level;
	    if (randint(3) == 1)
		place_object(yval, xval - 2);
	    if (randint(3) == 1)
		place_object(yval, xval + 2);
	    vault_monster(yval, xval - 2, randint(2));
	    vault_monster(yval, xval + 2, randint(2));
	}
	break;

      case 4:			   /* Maze inside.	 */
	tmp = randint(4);
	if (tmp < 3) {		   /* Place a door	 */
	    if (tmp == 1)
		place_secret_door(y_height - 1, xval);
	    else
		place_secret_door(y_depth + 1, xval);
	}
	else {
	    if (tmp == 3)
		place_secret_door(yval, x_left - 1);
	    else
		place_secret_door(yval, x_right + 1);
	}

	for (i = y_height; i <= y_depth; i++)
	    for (j = x_left; j <= x_right; j++)
		if (0x1 & (j + i))
		    cave[i][j].fval = TMP1_WALL;

    /* Monsters just love mazes.		 */
	vault_monster(yval, xval - 5, randint(3));
	vault_monster(yval, xval + 5, randint(3));
    /* Traps make them entertaining.	 */
	vault_trap(yval, xval - 3, 2, 8, randint(3));
	vault_trap(yval, xval + 3, 2, 8, randint(3));
    /* Mazes should have some treasure too..	 */
	for (i = 0; i < 3; i++)
	    random_object(yval, xval, 1);
	break;

      case 5:			   /* Four small rooms.	 */
	for (i = y_height; i <= y_depth; i++)
	    cave[i][xval].fval = TMP1_WALL;

	c_ptr = &cave[yval][x_left];
	for (i = x_left; i <= x_right; i++) {
	    c_ptr->fval = TMP1_WALL;
	    c_ptr++;
	}

	if (randint(2) == 1) {
	    i = randint(10);
	    place_secret_door(y_height - 1, xval - i);
	    place_secret_door(y_height - 1, xval + i);
	    place_secret_door(y_depth + 1, xval - i);
	    place_secret_door(y_depth + 1, xval + i);
	}
	else {
	    i = randint(3);
	    place_secret_door(yval + i, x_left - 1);
	    place_secret_door(yval - i, x_left - 1);
	    place_secret_door(yval + i, x_right + 1);
	    place_secret_door(yval - i, x_right + 1);
	}

    /* Treasure in each one.		 */
	random_object(yval, xval, 2 + randint(2));
	
    /* Gotta have some monsters.		 */
	vault_monster(yval + 2, xval - 4, randint(2));
	vault_monster(yval + 2, xval + 4, randint(2));
	vault_monster(yval - 2, xval - 4, randint(2));
	vault_monster(yval - 2, xval + 4, randint(2));
	break;
    }
}

/*
 * Builds a room at a row, column coordinate		-RAK-
 * Type 3 unusual rooms are cross shaped			 
 */
static void build_type3(int yval, int xval)
{
    int                 y_height, y_depth;
    int                 x_left, x_right;
    register int        tmp, i, j;
    int8u               floor;
    bool		light;
    register cave_type *c_ptr;

    /* Tight fit?  Try a simpler room */
    if (!in_bounds(yval - 3, xval - 3) || !in_bounds(yval + 3, xval + 3)) {
	build_type1(yval, xval);
	return;
    }

    if (dun_level <= randint(25)) {
	floor = LITE_FLOOR;	   /* Floor with light	 */
	light = TRUE;
    }
    else {
	floor = DARK_FLOOR;	   /* Dark floor	 */
	light = FALSE;
    }
    	
    /* Pick room size */
    tmp = 2 + randint(2);
    y_height = yval - tmp;
    y_depth = yval + tmp;
    x_left = xval - 1;
    x_right = xval + 1;

/* for paranoia's sake: bounds-check!  Memory errors caused by accessing
 * cave[][] out-of-bounds are nearly impossible to spot!  -CFT 
 */
    /* Paranoia -- Bounds check! */
    if (y_height < 1) y_height = 1;
    if (y_depth >= (cur_height - 1)) y_depth = cur_height - 2;
    
    for (i = y_height; i <= y_depth; i++) {
	for (j = x_left; j <= x_right; j++) {
	    c_ptr = &cave[i][j];
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = floor;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
    }
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	c_ptr = &cave[i][x_left - 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
	c_ptr = &cave[i][x_right + 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
    }
    for (i = x_left; i <= x_right; i++) {
	c_ptr = &cave[y_height - 1][i];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
	c_ptr = &cave[y_depth + 1][i];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
    }

    /* Pick location */
    tmp = 2 + randint(9);
    y_height = yval - 1;
    y_depth = yval + 1;
    x_left = xval - tmp;
    x_right = xval + tmp;

    /* Paranoia -- bounds check */
    if (x_left < 1) x_left = 1;
    if (x_right >= (cur_width - 1)) x_right = cur_width - 2;

    for (i = y_height; i <= y_depth; i++) {
	for (j = x_left; j <= x_right; j++) {
	    c_ptr = &cave[i][j];
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = floor;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
    }
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	c_ptr = &cave[i][x_left - 1];
	if (c_ptr->fval != floor) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = GRANITE_WALL;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
	c_ptr = &cave[i][x_right + 1];
	if (c_ptr->fval != floor) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = GRANITE_WALL;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
    }
    for (i = x_left; i <= x_right; i++) {
	c_ptr = &cave[y_height - 1][i];
	if (c_ptr->fval != floor) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = GRANITE_WALL;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
	c_ptr = &cave[y_depth + 1][i];
	if (c_ptr->fval != floor) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = GRANITE_WALL;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	}
    }

/* Special features.			 */
    switch (randint(4)) {
      case 1:			   /* Large middle pillar		 */
	for (i = yval - 1; i <= yval + 1; i++) {
	    c_ptr = &cave[i][xval - 1];
	    for (j = xval - 1; j <= xval + 1; j++) {
		c_ptr->fval = TMP1_WALL;
		c_ptr++;
	    }
	}
	break;

      case 2:			   /* Inner treasure vault		 */
	for (i = yval - 1; i <= yval + 1; i++) {
	    cave[i][xval - 1].fval = TMP1_WALL;
	    cave[i][xval + 1].fval = TMP1_WALL;
	}
	cave[yval - 1][xval].fval = TMP1_WALL;
	cave[yval + 1][xval].fval = TMP1_WALL;

	tmp = randint(4);	   /* Place a door	 */
	if (tmp < 3)
	    place_secret_door(yval - 3 + (tmp << 1), xval);
	else
	    place_secret_door(yval, xval - 7 + (tmp << 1));

    /* Place a treasure in the vault		 */
	object_level = dun_level;
	place_object(yval, xval);
    /* Let's guard the treasure well.	 */
	vault_monster(yval, xval, 2 + randint(2));
    /* Traps naturally			 */
	vault_trap(yval, xval, 4, 4, 1 + randint(3));
	break;

      case 3:
	if (randint(3) == 1) {
	    cave[yval - 1][xval - 2].fval = TMP1_WALL;
	    cave[yval + 1][xval - 2].fval = TMP1_WALL;
	    cave[yval - 1][xval + 2].fval = TMP1_WALL;
	    cave[yval + 1][xval + 2].fval = TMP1_WALL;
	    cave[yval - 2][xval - 1].fval = TMP1_WALL;
	    cave[yval - 2][xval + 1].fval = TMP1_WALL;
	    cave[yval + 2][xval - 1].fval = TMP1_WALL;
	    cave[yval + 2][xval + 1].fval = TMP1_WALL;
	    if (randint(3) == 1) {
		place_secret_door(yval, xval - 2);
		place_secret_door(yval, xval + 2);
		place_secret_door(yval - 2, xval);
		place_secret_door(yval + 2, xval);
	    }
	}
	else if (randint(3) == 1) {
	    cave[yval][xval].fval = TMP1_WALL;
	    cave[yval - 1][xval].fval = TMP1_WALL;
	    cave[yval + 1][xval].fval = TMP1_WALL;
	    cave[yval][xval - 1].fval = TMP1_WALL;
	    cave[yval][xval + 1].fval = TMP1_WALL;
	}
	else if (randint(3) == 1)
	    cave[yval][xval].fval = TMP1_WALL;
	break;

      case 4:
	break;
    }
}

static void special_pit(int yval, int xval, int type)
{
    register int        i, j, y_height, x_left;
    int                 y_depth, x_right, colour;
    int8u               floor;
    bool		light;
    register cave_type *c_ptr, *d_ptr;

    /* Pits are always dark */
    floor = DARK_FLOOR;
    light = FALSE;
    
    /* Pick a room size */
    y_height = yval - 4;
    y_depth = yval + 4;
    x_left = xval - 11;
    x_right = xval + 11;

    /* Tight fit?  Try a simpler room */
    if ((y_height < 1) || (y_depth >= (cur_height-1)) ||
	(x_left < 1) || (x_right >= (cur_width-1))) {

	/* type1 is heavily bounds-checked, and considered */
	/* safe to use as a fall-back room type -CFT */
	build_type1(yval,xval);
	return;
    }

    /* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
    if ((randint(dun_level*dun_level + 1) < 300) && (dun_level <= 40)) {
	good_item_flag = TRUE;
    }

/* the x dim of rooms tends to be much larger than the y dim, so don't bother
 * rewriting the y loop 
 */

    for (i = y_height; i <= y_depth; i++) {
	c_ptr = &cave[i][x_left];
	for (j = x_left; j <= x_right; j++) {
	    c_ptr->info |= CAVE_LR;
	    c_ptr->fval = floor;
	    /* if (light) c_ptr->info |= CAVE_PL; */
	    c_ptr++;
	}
    }
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	c_ptr = &cave[i][x_left - 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
	c_ptr = &cave[i][x_right + 1];
	c_ptr->fval = GRANITE_WALL;
	c_ptr->info |= CAVE_LR;
    }
    c_ptr = &cave[y_height - 1][x_left];
    d_ptr = &cave[y_depth + 1][x_left];
    for (i = x_left; i <= x_right; i++) {
	c_ptr->info |= CAVE_LR;
	c_ptr->fval = GRANITE_WALL;
	c_ptr++;
	d_ptr->info |= CAVE_LR;
	d_ptr->fval = GRANITE_WALL;
	d_ptr++;
    }

/* The inner room		 */
    y_height = y_height + 2;
    y_depth = y_depth - 2;
    x_left = x_left + 2;
    x_right = x_right - 2;
    for (i = (y_height - 1); i <= (y_depth + 1); i++) {
	cave[i][x_left - 1].fval = TMP1_WALL;
	cave[i][x_right + 1].fval = TMP1_WALL;
    }
    c_ptr = &cave[y_height - 1][x_left];
    d_ptr = &cave[y_depth + 1][x_left];
    for (i = x_left; i <= x_right; i++) {
	c_ptr->fval = TMP1_WALL;
	c_ptr++;
	d_ptr->fval = TMP1_WALL;
	d_ptr++;
    }
    switch (randint(4)) {
      case 1:
	place_secret_door(y_height - 1, xval);
	break;
      case 2:
	place_secret_door(y_depth + 1, xval);
	break;
      case 3:
	place_secret_door(yval, x_left - 1);
	break;
      case 4:
	place_secret_door(yval, x_right + 1);
	break;
    }

    /* Random colour (for dragons) */    
    colour = randint(6);

    if (wizard || peek) {
	switch (type) {
	  case 1:
	    msg_print("A Slime Pit");
	    break;
	  case 2:
	    msg_print("An Orc Pit");
	    break;
	  case 3:
	    msg_print("A Troll Pit");
	    break;
	  case 4:
	    msg_print("A Graveyard");
	    break;
	  case 5:
	    message("A ", 0x02);
	    message(vault_dragon_names_2[colour-1], 0x02);
	    message(" Pit", 0);
	    break;
	  case 6:
	    msg_print("A Demon Pit");
	    break;
	  case 7:
	    msg_print("A Giant Pit");
	    break;
	}
    }

    j = y_height;
    for (i = x_left; i <= x_right; i++)
	vault_nasty(j, i, type, 1, colour);
    j = y_depth;
    for (i = x_left; i <= x_right; i++)
	vault_nasty(j, i, type, 1, colour);
    i = x_left;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 1, colour);
    i = x_right;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 1, colour);
    i = x_left + 1;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 2, colour);
    i = x_left + 2;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 2, colour);
    i = x_right - 1;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 2, colour);
    i = x_right - 2;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 2, colour);
    i = x_left + 3;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 3, colour);
    i = x_left + 4;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 3, colour);
    i = x_right - 3;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 3, colour);
    i = x_right - 4;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 3, colour);
    i = x_left + 5;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 4, colour);
    i = x_left + 6;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 4, colour);
    i = x_right - 5;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 4, colour);
    i = x_right - 6;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 4, colour);
    i = x_left + 7;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 5, colour);
    i = x_left + 8;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 5, colour);
    i = x_right - 7;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 5, colour);
    i = x_right - 8;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 5, colour);
    i = x_right - 9;
    for (j = (y_height + 1); j <= (y_depth - 1); j++)
	vault_nasty(j, i, type, 6, colour);
}



/*
 * Constructs a tunnel between two points
 *
 * Note that this function is called BEFORE any streamers are made. 
 * So fval's "QUARTZ_WALL" and "MAGMA_WALL" cannot be encountered.
 */
static void build_tunnel(int row1, int col1, int row2, int col2)
{
    register int        tmp_row, tmp_col, i, j;
    register cave_type *c_ptr, *d_ptr;
    coords              tunstk[1000], wallstk[1000];
    coords             *cp;
    int                 row_dir, col_dir, tunindex, wallindex;
    int                 stop_flag, door_flag, main_loop_count;
    int                 start_row, start_col;

/* Main procedure for Tunnel			 */
/* Note: 9 is a temporary value		 */
    stop_flag = FALSE;
    door_flag = FALSE;
    tunindex = 0;
    wallindex = 0;
    main_loop_count = 0;
    start_row = row1;
    start_col = col1;
    correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

    do {
    /* prevent infinite loops, just in case */
	main_loop_count++;
	if (main_loop_count > 2000)
	    stop_flag = TRUE;

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
	    d_ptr = &cave[tmp_row + row_dir][tmp_col + col_dir];

	    if ((d_ptr->fval == GRANITE_WALL) || (d_ptr->fval == TMP2_WALL)) {
		c_ptr->fval = TMP2_WALL;
	    }

	/* if can not pass completely through wall don't try... And mark as
	 * impassible for future -KOC 
	 */
	    else {
		row1 = tmp_row;
		col1 = tmp_col;
		if (wallindex < 1000) {
		    wallstk[wallindex].y = row1;
		    wallstk[wallindex].x = col1;
		    wallindex++;
		}
		for (i = row1 - 1; i <= row1 + 1; i++) {
		    for (j = col1 - 1; j <= col1 + 1; j++) {
			if (in_bounds(i, j)) {
			    d_ptr = &cave[i][j];

			/*
			 * values 11 and 12 are impossible here,
			 * place_streamer is never run before build_tunnel 
			 */
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

    cp = &tunstk[0];
    for (i = 0; i < tunindex; i++) {
	d_ptr = &cave[cp->y][cp->x];
	d_ptr->fval = CORR_FLOOR;
	cp++;
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


static int next_to(int y, int x)
{
    register int next;

    /* abort! -CFT */
    if (!in_bounds(y, x)) return 0;
    
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
    else {
	next = FALSE;
    }

    return (next);
}

/*
 * Places door at y, x position if at least 2 walls found	 
 */
static void try_door(int y, int x)
{
    /* abort! -CFT */
    if (!in_bounds(y, x)) return;

    if ((cave[y][x].fval == CORR_FLOOR) &&
        (randint(100) > DUN_TUN_JCT) &&
	next_to(y, x)) {

	place_door(y, x);
    }
}


/*
 * Returns random co-ordinates for player/monster/object
 */
static void new_player_spot(void)
{
    register int        i, j;
    register cave_type *c_ptr;

    while (1) {

	/* Pick a legal spot */
	i = rand_range(1, cur_height - 2);
	j = rand_range(1, cur_width - 2);

	/* Must be a "clean floor" */
	if (!clean_grid(i, j)) continue;

	c_ptr = &cave[i][j];

	if (c_ptr->m_idx) continue;

	/* Can't teleport? This is probably just used for player */        
	if (c_ptr->fval == NT_LITE_FLOOR) continue;
	if (c_ptr->fval == NT_DARK_FLOOR) continue;

	break;
    }    

    /* Save the new player grid */
    char_row = i;
    char_col = j;

    /* Mark the dungeon grid */
    cave[char_row][char_col].m_idx = 1;
}

static void build_pit(int yval, int xval)
{
    int tmp;

    tmp = randint(dun_level > 80 ? 80 : dun_level);
    rating += 10;
    if (tmp < 10) {
	special_pit(yval, xval, 1);
    }
    else if (tmp < 20) {
	special_pit(yval, xval, 2);
    }
    else if (tmp < 43) {
	if (randint(3) == 1) {
	    special_pit(yval, xval, 7);
	}
	else {
	    special_pit(yval, xval, 3);
	}
    }
    else if (tmp < 57) {
	special_pit(yval, xval, 4);
    }
    else if (tmp < 73) {
	special_pit(yval, xval, 5);
    }
    else {
	special_pit(yval, xval, 6);
    }
}

/*
 * Cave logic flow for generation of new dungeon
 */
static void cave_gen(void)
{
    int          room_map[20][20];
    register int i, j, k;
    int          y1, x1, y2, x2, pick1, pick2, tmp;
    int          row_rooms, col_rooms, alloc_level;
    int16        yloc[400], xloc[400];

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
    for (i = 0; i < row_rooms; i++) {
	for (j = 0; j < col_rooms; j++) {
	    room_map[i][j] = FALSE;
	}
    }

    k = randnor(DUN_ROO_MEA, 2);
    for (i = 0; i < k; i++) {
	room_map[randint(row_rooms) - 1][randint(col_rooms) - 1] = TRUE;
    }

    k = 0;
    for (i = 0; i < row_rooms; i++) {
	for (j = 0; j < col_rooms; j++) {
	    if (room_map[i][j] == TRUE) {
		yloc[k] = i * (SCREEN_HEIGHT >> 1) + (SCREEN_HEIGHT >> 2);
		xloc[k] = j * (SCREEN_WIDTH >> 1) + (SCREEN_WIDTH >> 2);
		if (dun_level > randint(DUN_UNUSUAL)) {
		    tmp = randint(5);
		    if ((tmp == 1) || destroyed)
			build_type1(yloc[k], xloc[k]);
		    else if (tmp == 2)
			build_type2(yloc[k], xloc[k]);
		    else if (tmp == 3)
			build_type3(yloc[k], xloc[k]);
		    else if ((tmp == 4) && dun_level > randint(DUN_UNUSUAL)) {
			build_type5(yloc[k], xloc[k]);
			if (j + 1 < col_rooms)
			    room_map[i][j + 1] = FALSE;
			if (j + 1 < col_rooms && i + 1 < row_rooms)
			    room_map[i + 1][j + 1] = FALSE;
			if (j > 0 && i + 1 < row_rooms)
			    room_map[i + 1][j - 1] = FALSE;
			if (i + 1 < row_rooms)
			    room_map[i + 1][j] = FALSE;
		    }
		    else if (dun_level > randint(DUN_UNUSUAL) && pit_ok) {
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

    for (i = 0; i < k; i++) {
	pick1 = randint(k) - 1;
	pick2 = randint(k) - 1;
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
    for (i = 0; i < DUN_STR_MAG; i++)
	place_streamer(MAGMA_WALL, DUN_STR_MC);
    for (i = 0; i < DUN_STR_QUA; i++)
	place_streamer(QUARTZ_WALL, DUN_STR_QC);

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
 * Builds a store at a row, column coordinate
 *
 * We no longer (2.7.0) favors top left corners for store doors
 */
static void build_store(int store_num, int y, int x)
{
    int                 y1, y2, x1, x2;
    register int        i, j;
    int                 cur_pos, tmp;
    register cave_type *c_ptr;
    inven_type *i_ptr;

    i = y * 10 + 5;
    j = x * 14 + 12;

    y1 = i - randint(3);
    y2 = i + randint(4);
    x1 = j - randint(4);
    x2 = j + randint(4);

    /* Build an invulnerable rectangular building */    
    for (i = y1; i <= y2; i++) {
	for (j = x1; j <= x2; j++) {
	    c_ptr = &cave[i][j];
	    c_ptr->fval = BOUNDARY_WALL;
	}
    }

    /* Pick a location for the door  */
    tmp = randint(4);

    /* Apply that location */
    switch (tmp) {

	/* Left side (including top left, but not bottom left) */
	case 1:
	    i = rand_range(y1,y2-1);
	    j = x1;
	    break;

	/* Top side (including top right, but not top left) */
	case 4:
	    i = y1;
	    j = rand_range(x1+1,x2);
	    break;

	/* Right side (including bottom right, but not top right) */
	case 2:
	    i = rand_range(y1+1,y2);
	    j = x2;
	    break;

	/* Bottom side (including bottom left, but not bottom right) */
	case 3:
	    i = y2;
	    j = rand_range(x1,x2-1);
	    break;
    }

    /* Create a "door trap" */
    cur_pos = i_pop();
    i_ptr = &i_list[cur_pos];
    invcopy(i_ptr, OBJ_STORE_LIST + store_num);
    i_ptr->iy = i;
    i_ptr->ix = j;

    /* Make the store door be a floor plus the "door trap" */
    c_ptr = &cave[i][j];
    c_ptr->fval = CORR_FLOOR;
    c_ptr->i_idx = cur_pos;
}


/*
 * Places a down-staircase in the town
 *
 * By not using the standard "place_stairs()" function, we
 * minimizes dependance of OTHER functions on the random number
 * generation hack used in town_gen().
 */
static void place_town_stairs(void)
{
    register cave_type *c_ptr;
    register int        y, x, y1, x1;

    /* Repeat until stairs are placed */
    while (1) {

	/* Pick a 12x12 "box" FULLY inside the dungeon */
	y1 = randint(cur_height - 14);
	x1 = randint(cur_width - 14);

	/* Scan that box, trying to drop some stairs */
	for (y = y1; y < y1 + 12; y++) {
	    for (x = x1; x < x1 + 12; x++) {

		/* Avoid building walls and doors */
		if (clean_grid(y, x)) {

		    register int cur_pos;
		    inven_type *i_ptr;

		    /* Create a "stair" object */
		    cur_pos = i_pop();
		    i_ptr = &i_list[cur_pos];
		    invcopy(i_ptr, OBJ_DOWN_STAIR);
		    i_ptr->iy = y;
		    i_ptr->ix = x;

		    /* Place it in this grid */
		    c_ptr = &cave[y][x];
		    c_ptr->i_idx = cur_pos;

		    /* Hack -- The stairs are "perma-lit" */
		    c_ptr->info |= CAVE_PL;

		    /* Hack -- The stairs are "field marked" */
		    c_ptr->info |= CAVE_FM;

		    /* All done */
		    return;
		}
	    }
	}
    }
}



/*
 * Generate the "consistent" town features
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings and
 * the doorways, and the stairs.
 *
 * Note that "rand_int(n)" is NOT the same as "randint(n) - 1"
 * because of this hack.  Usually, it is equivalent.
 */
static void town_gen_hack(void)
{
    register int        i, j, k, n;
    int                 rooms[MAX_STORES];


    /* Hack -- make the same town every time */
    set_seed(town_seed);

    /* Prepare an Array of "remaining stores", and count them */
    for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

    /* Place two rows of stores */
    for (i = 0; i < 2; i++) {

	/* Place four stores per row */
	for (j = 0; j < 4; j++) {

	    /* Pick a random unplaced store */
	    k = rand_int(n);

	    /* Build that store at the proper location */
	    build_store(rooms[k], i, j);

	    /* Shift the stores down, remove one store */
	    /* Hack -- do not change method, even though it would */
	    /* be prettier just to do "rooms[k] = rooms[--n];" */
	    for (--n; k < n; k++) rooms[k] = rooms[k + 1];
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
 * in caves, we do a horrible hack involving "CAVE_PL" and "CAVE_INIT".
 * I would really prefer something involving just "CAVE_INIT".  That way,
 * the player can "see" lit rooms as he comes down the hall.  And a whole
 * set of "messy" issues are done away with.  Note that "CAVE_INIT" is
 * cleared at the start of every new level/session.  Note that we have to
 * be careful not to set "CAVE_FM" from "CAVE_PL" until the "CAVE_INIT"
 * code is executed.  Unless the savefile is pre-2.7.0.
 */
static void town_gen(void)
{
    register int        i, j;
    register cave_type *c_ptr;


    /* Hack -- Build (or rebuild) the permanent physical objects */
    town_gen_hack();


    /* Place the external walls */
    place_boundary();

    /* Fill the cave with "Dark Floor" */
    fill_cave(DARK_FLOOR);


    /* XXX Should consider always being on the stairs */

    /* Place the player in the town */
    new_player_spot();


    /* Night */
    if (0x1 & (turn / 5000)) {
	for (i = 0; i < cur_height; i++) {
	    c_ptr = &cave[i][0];
	    for (j = 0; j < cur_width; j++) {

		/* Ignore "dark floors" */
		if (c_ptr->fval != DARK_FLOOR) {

		    /* Hack -- Pre-Perma-Lite permanent landmarks */
		    c_ptr->info |= CAVE_PL;

		    /* Hack -- Memorize permanent landmarks */
		    c_ptr->info |= CAVE_FM;
		}

		/* Advance */
		c_ptr++;
	    }
	}

	/* Make some night-time residents */
	alloc_monster(MIN_M_ALLOC_TN, 3, TRUE);
    }

    /* Day */
    else {
	for (i = 0; i < cur_height; i++) {
	    c_ptr = &cave[i][0];
	    for (j = 0; j < cur_width; j++) {
	    
	        /* Pre-Perma-Lite everything */
		c_ptr->info |= CAVE_PL;

		/* Hack -- Pre-Memorize Everything */
		c_ptr->info |= CAVE_FM;

		/* Advance */
		c_ptr++;
	    }
	}

	/* Make some daytime residents */
	alloc_monster(MIN_M_ALLOC_TD, 3, TRUE);
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

    /* Reset the object level */
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

