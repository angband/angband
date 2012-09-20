/* File: cave.c */

/* Purpose: mid-level graphics -- colors and symbols and such */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"




/*
 * Is a given location "valid" for placing things?
 * Note that solid rock, doors, and rubble evaluate as "valid".
 * Note that artifacts, store doors, and stairs, do not.
 *
 * This function is usually "combined" with "floor_grid_bold(y,x)",
 * which checks to see if the grid is not a wall or blockage.
 */
bool valid_grid(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    /* Outer wall (and illegal grids) are not "valid" */
    if (!in_bounds(y,x)) return (FALSE);

    /* Get that grid */
    c_ptr = &cave[y][x];

    /* Internal Boundary walls are invalid */
    if (c_ptr->fval == BOUNDARY_WALL) return (FALSE);

    /* Nothing here, this is very desirable */
    if (c_ptr->i_idx == 0) return (TRUE);

    /* Something there */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Stairs and store doors are very important */
    if (i_ptr->tval == TV_STORE_DOOR) return (FALSE);
    if (i_ptr->tval == TV_DOWN_STAIR) return (FALSE);
    if (i_ptr->tval == TV_UP_STAIR) return (FALSE);

    /* Artifacts are really important */
    if (artifact_p(i_ptr)) return (FALSE);

    /* Normal object may be destroyed */
    return (TRUE);
}



/*
 * A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
 * 4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu. 
 *
 * Returns TRUE if a line of sight can be traced from x0, y0 to x1, y1. 
 *
 * The LOS begins at the center of the tile [x0, y0] and ends at the center of
 * the tile [x1, y1].  If los() is to return TRUE, all of the tiles this line
 * passes through must be transparent, WITH THE EXCEPTIONS of the starting
 * and ending tiles. 
 *
 * We don't consider the line to be "passing through" a tile if it only passes
 * across one corner of that tile. 
 */

/*
 * Because this function uses (short) ints for all calculations, overflow may
 * occur if d_x and d_y exceed 90.
 *
 * Also note that this function and the "move towards target" code do NOT
 * share the same properties.  Thus, you can see someone, target them, and
 * then fire a bolt at them, but the bolt may hit a wall, not them.  However,
 * by clever choice of target locations, you can sometimes throw a "curve".
 */
int los(int fromY, int fromX, int toY, int toX)
{
    register int p_x, p_y, d_x, d_y, a_x, a_y;


    /* Extract the offset */    
    d_y = toY - fromY;
    d_x = toX - fromX;

    /* Extract the absolute offset */
    a_y = MY_ABS(d_y);
    a_x = MY_ABS(d_x);


    /* Handle adjacent (or identical) grids */
    if ((a_x < 2) && (a_y < 2)) return (TRUE);


    /* XXX XXX XXX Paranoia -- require "safe" origin */
    if (!in_bounds(fromY, fromX)) return (FALSE);


    /* Directly South/North */
    if (!d_x) {

	register int p_y;

	/* South -- check for walls */
	if (d_y > 0) {
	    for (p_y = fromY + 1; p_y < toY; p_y++) {
		if (!floor_grid_bold(p_y,fromX)) return FALSE;
	    }
	}
	
	/* North -- check for walls */
	else {
	    for (p_y = fromY - 1; p_y > toY; p_y--) {
		if (!floor_grid_bold(p_y,fromX)) return FALSE;
	    }
	}
	
	/* Assume los */
	return TRUE;
    }

    /* Directly East/West */
    if (!d_y) {
    
	register int p_x;

	/* East -- check for walls */
	if (d_x > 0) {
	    for (p_x = fromX + 1; p_x < toX; p_x++) {
		if (!floor_grid_bold(fromY,p_x)) return FALSE;
	    }
	}

	/* West -- check for walls */
	else {
	    for (p_x = fromX - 1; p_x > toX; p_x--) {
		if (!floor_grid_bold(fromY,p_x)) return FALSE;
	    }
	}
		
	/* Assume los */
	return TRUE;
    }


    /* Handle Knightlike shapes -CWS */
    if (a_x == 1) {
	if (d_y == 2) {
	    if (floor_grid_bold(fromY + 1, fromX)) return TRUE;
	}
	else if (d_y == (-2)) {
	    if (floor_grid_bold(fromY - 1, fromX)) return TRUE;
	}
    }
    else if (a_y == 1) {
	if (d_x == 2) {
	    if (floor_grid_bold(fromY, fromX + 1)) return TRUE;
	}
	else if (d_x == (-2)) {
	    if (floor_grid_bold(fromY, fromX - 1)) return TRUE;
	}
    }


/*
 * Now, we've eliminated all the degenerate cases. In the computations below,
 * dy (or dx) and m are multiplied by a scale factor, scale = abs(d_x *
 * d_y * 2), so that we can use integer arithmetic. 
 */

    {
	register int        scale,	/* a scale factor		 */
			    scale2;	/* above scale factor / 2	 */

	int		    xSign,	/* sign of d_x		 */
			    ySign,	/* sign of d_y		 */
			    m;		/* slope or 1/slope of LOS	 */

	scale2 = (a_x * a_y);
	scale = scale2 << 1;

	xSign = (d_x < 0) ? -1 : 1;
	ySign = (d_y < 0) ? -1 : 1;


	/* Travel from one end of the line to the other, */
	/* oriented along the longer axis. */

	if (a_x >= a_y) {

	    register int        dy;  /* "fractional" y position	 */

	/*
	 * We start at the border between the first and second tiles, where
	 * the y offset = .5 * slope.  Remember the scale factor.  We have: 
	 *
	 * m = d_y / d_x * 2 * (d_y * d_x) = 2 * d_y * d_y. 
	 */

	    dy = a_y * a_y;
	    m = dy << 1;
	    p_x = fromX + xSign;

	    /* Consider the special case where slope == 1. */
	    if (dy == scale2) {
		p_y = fromY + ySign;
		dy -= scale;
	    }
	    else {
		p_y = fromY;
	    }

	    /* Note (below) the case (dy == scale2), where */
	    /* the LOS exactly meets the corner of a tile. */
	    while (toX - p_x) {
		if (!floor_grid_bold(p_y,p_x)) return FALSE;
		dy += m;
		if (dy < scale2) {
		    p_x += xSign;
		}
		else if (dy > scale2) {
		    p_y += ySign;
		    if (!floor_grid_bold(p_y,p_x)) return FALSE;
		    dy -= scale;
		    p_x += xSign;
		}
		else {
		    p_y += ySign;
		    dy -= scale;
		    p_x += xSign;
		}
	    }
	    return TRUE;
	}

	else {
	
	    register int        dx;	/* "fractional" x position	 */

	    dx = a_x * a_x;
	    m = dx << 1;

	    p_y = fromY + ySign;
	    if (dx == scale2) {
		p_x = fromX + xSign;
		dx -= scale;
	    }
	    else {
		p_x = fromX;
	    }

	    /* Note (below) the case (dx == scale2), where */
	    /* the LOS exactly meets the corner of a tile. */
	    while (toY - p_y) {
		if (!floor_grid_bold(p_y,p_x)) return FALSE;
		dx += m;
		if (dx < scale2) {
		    p_y += ySign;
		}
		else if (dx > scale2) {
		    p_x += xSign;
		    if (!floor_grid_bold(p_y,p_x)) return FALSE;
		    dx -= scale;
		    p_y += ySign;
		}
		else {
		    p_x += xSign;
		    dx -= scale;
		    p_y += ySign;
		}
	    }
	}
    }

    /* Assume los */
    return TRUE;
}





/*
 * Hack -- Hallucination "symbol"
 */
static char image_char(void)
{
    char c;
    c = rand_range(32,126);
    return (c);
}








/*
 * Get a legal "multi-hued" color
 * Does NOT include White, Black, or any Grays.
 * Should it include "Brown" and "Light Brown"?
 */
static byte mh_attr(void)
{

#ifdef USE_COLOR

    switch (randint(11)) {
	case 1: return (TERM_RED);
	case 2: return (TERM_BLUE);
	case 3: return (TERM_GREEN);
	case 4: return (TERM_YELLOW);
	case 5: return (TERM_ORANGE);
	case 6: return (TERM_VIOLET);
	case 7: return (TERM_UMBER);
	case 8: return (TERM_L_RED);
	case 9: return (TERM_L_BLUE);
	case 10: return (TERM_L_GREEN);
	case 11: return (TERM_L_UMBER);
    }

#endif

    return (TERM_WHITE);
}


/*
 * Hack -- Get a random "hallucination" attr
 */
static byte image_attr(void)
{

#ifdef USE_COLOR

    switch (randint(15)) {
	case 1: return (TERM_RED);
	case 2: return (TERM_BLUE);
	case 3: return (TERM_GREEN);
	case 4: return (TERM_YELLOW);
	case 5: return (TERM_ORANGE);
	case 6: return (TERM_VIOLET);
	case 7: return (TERM_UMBER);
	case 8: return (TERM_L_RED);
	case 9: return (TERM_L_BLUE);
	case 10: return (TERM_L_GREEN);
	case 11: return (TERM_L_UMBER);
	case 12: return (TERM_WHITE);
	case 13: return (TERM_GRAY);
	case 14: return (TERM_L_GRAY);
	case 15: return (TERM_D_GRAY);
    }

#endif

    return (TERM_WHITE);
}



/*
 * Attribute for a item in the inventory
 */
byte inven_attr_by_tval(inven_type *i_ptr)
{

#ifdef USE_COLOR

    /* Run-time mono */
    if (!use_color) return (TERM_WHITE);

    /* Hack -- check the attribute table */
    if (tval_to_attr[i_ptr->tval]) return (tval_to_attr[i_ptr->tval]);
    
    /* Examine the object */
    switch (i_ptr->tval) {
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
	return (TERM_WHITE);
      case TV_CHEST:
	return (TERM_GRAY);
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
	return (TERM_L_UMBER);
      case TV_LITE:
	return (TERM_YELLOW);
      case TV_SPIKE:
	return (TERM_GRAY);
      case TV_BOW:
	return (TERM_UMBER);
      case TV_DIGGING:
	return (TERM_GRAY);
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
	return (TERM_L_GRAY);
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_HELM:
      case TV_SHIELD:
      case TV_CLOAK:
	return (TERM_L_UMBER);
      case TV_SOFT_ARMOR:
      case TV_HARD_ARMOR:
      case TV_DRAG_ARMOR:
	return (TERM_GRAY);
      case TV_AMULET:
	return (TERM_ORANGE);
      case TV_RING:
	return (TERM_RED);
      case TV_STAFF:
	return (TERM_L_UMBER);
      case TV_WAND:
	return (TERM_L_GREEN);
      case TV_ROD:
	return (TERM_L_GRAY);
      case TV_SCROLL:
	return (TERM_WHITE);
      case TV_POTION:
	return (TERM_L_BLUE);
      case TV_FLASK:
	return (TERM_YELLOW);
      case TV_FOOD:
	return (TERM_L_UMBER);
      case TV_MAGIC_BOOK:
	return (TERM_L_RED);
      case TV_PRAYER_BOOK:
	return (TERM_L_GREEN);
    }

#endif

    /* No colors */
    return (TERM_WHITE);
}





/* 
 * Return a color to use for the bolt/ball spells
 */
byte spell_color(int type)
{

#ifdef USE_COLOR

    if (!use_color) return (TERM_WHITE);

    switch (type) {

	case GF_MISSILE:
		return (mh_attr());		/* multihued */
	case GF_ELEC:
		return (TERM_YELLOW);
	case GF_POIS:
		return (TERM_GREEN);
	case GF_ACID:
		return (TERM_GRAY);
	case GF_COLD:
		return (TERM_L_BLUE);
	case GF_FIRE:
		return (TERM_RED);
	case GF_HOLY_ORB:
		return (TERM_D_GRAY);
	case GF_ARROW:
		return (TERM_L_UMBER);
	case GF_PLASMA:
		return (TERM_RED);
	case GF_NETHER:
		return (TERM_VIOLET);
	case GF_WATER:
		return (TERM_BLUE);
	case GF_CHAOS:
		return (mh_attr());		/* multihued */
	case GF_SHARDS:
		return (TERM_L_UMBER);
	case GF_SOUND:
		return (TERM_ORANGE);
	case GF_CONFUSION:
		return (mh_attr());		/* multihued */
	case GF_DISENCHANT:
		return (TERM_VIOLET);
	case GF_NEXUS:
		return (TERM_L_RED);
	case GF_FORCE:
		return (TERM_WHITE);
	case GF_INERTIA:
		return (TERM_L_GRAY);
	case GF_LITE_WEAK:
	case GF_LITE:
		return (TERM_YELLOW);
	case GF_DARK_WEAK:
	case GF_DARK:
		return (TERM_D_GRAY);
	case GF_TIME:
		return (TERM_L_BLUE);
	case GF_GRAVITY:
		return (TERM_GRAY);
	case GF_MANA:
		return (TERM_L_RED);
	case GF_METEOR:
		return (TERM_ORANGE);
	case GF_ICE:
		return (TERM_L_BLUE);
    }

#endif

    /* Standard "color" */
    return (TERM_WHITE);
}



/*
 * Does the player have "line of sight" to a grid
 *
 * The new "VIEW" code maintains a "current viewable space", so that it
 * can very quickly answer the questions "can the player see a grid?"
 * and "could the player see a grid assuming that it was lit?"
 *
 * Note that pre-calculating the "viewable space" ahead of time has some
 * rather obvious advantages.  On of the most notable is that, since the
 * "maximal view area" is precalculated, it takes no time at all to note
 * that a given grid is NOT viewable, which otherwise takes up a lot of
 * processing power when many monsters are nearby.
 *
 * Also note that unless the player "moves" or "opens doors", there is
 * no need to recalculate the "viewable space", and even when doors are
 * opened, it only matters if that door itself was already viewable.
 * So, if the player is, say, resting, then the monsters can move around
 * without ever having to check "los()" from the player.
 *
 * I wouldn't be surprised if slight modifications could prepare some field
 * in the cave array to indicate which grids have "los()" on the player...
 */
bool player_has_los(int y, int x)
{
    /* Must be a "legal location" (including edges) */
    if (!in_bounds2(y, x)) return (FALSE);

    /* New method -- use "precomputed" view */
    if (view_pre_compute) {

	/* Check the "pre-computed" view */
	return ((cave[y][x].info & CAVE_VIEW) ? TRUE : FALSE);
    }

    /* Check distance */
    if (distance(char_row, char_col, y, x) > MAX_SIGHT) return (FALSE);

    /* Semi-Hack -- Call the "los()" function */
    return (los(char_row, char_col, y, x));
}


/*
 * Can the player "see" the given grid in detail?
 *
 * He must have vision, illumination, and line of sight.
 *
 * Note -- "CAVE_TL" is only set if the "torch" has "los()".
 * So, given "CAVE_TL", we know that the grid is "fully visible".
 *
 * Note that "CAVE_PL" makes little sense for a wall, since it would mean
 * that a wall is visible from any direction.  That would be odd.  Except
 * under wizard light, which might make sense.
 *
 * Note that the test for walls includes a loop which cannot use the
 * "floor_grid_bold()" function, since this function may be called on
 * a grid which is actually in the outer wall.
 *
 * XXX XXX XXX XXX Potential problem...
 * Note that the non-pre-computed view must be handled in a special
 * way to prevent "monster light tracks" in dark rooms.  Unfortunately,
 * this results in potential problems with the "beam of light" spell
 * when it enters a room.  Until the player actually enters the room,
 * he will not necessarily see monsters that are in the room.
 */
bool player_can_see(int y, int x)
{
    int yy, xx;
    
    /* Blind players see nothing */
    if (p_ptr->blind > 0) return (FALSE);

    /* Require "legal" location */
    if (!in_bounds2(y, x)) return (FALSE);
    
    /* Hack -- off screen cannot be "seen" */
    /* if (!panel_contains(y, x)) return (FALSE); */

    /* Hack -- "torch-lite" implies "viewable" */
    if (cave[y][x].info & CAVE_TL) return (TRUE);

    /* No "torch-lite" and no "perma-lite" yields "dark" */
    if (!(cave[y][x].info & CAVE_PL)) return (FALSE);

    /* Mega-Hack -- non-pre-compute-view has some more requirements */
    if (!view_pre_compute && (cave[y][x].info & CAVE_LR) &&
	(!(cave[y][x].info & CAVE_INIT))) return (FALSE);
    
    /* Non-walls merely require line-of-sight */
    if ((cave[y][x].fval < MIN_WALL) && player_has_los(y, x)) return (TRUE);

    /* Hack -- require line of sight when pre-computing view */
    if (view_pre_compute && !player_has_los(y, x)) return (FALSE);

    /* Walls require line-of-sight to an adjacent, perma-lit, non-wall */
    for (yy = y - 1; yy <= y + 1; yy++) {
	for (xx = x - 1; xx <= x + 1; xx++) {
	    if (!in_bounds(yy, xx)) continue;
	    if (cave[yy][xx].fval >= MIN_WALL) continue;
	    if (!(cave[yy][xx].info & CAVE_PL)) continue;
	    if (player_has_los(yy, xx)) return (TRUE);
	}
    }
    
    /* Assume not visible */
    return (FALSE);
}


/*
 * Tests a spot for light or field mark status		-RAK-	
 */
int test_lite(int y, int x)
{
    /* Hack -- player remembers grid */
    if (cave[y][x].info & CAVE_FM) return (TRUE);

    /* Player can see the grid and it is lit */
    return (player_can_see(y, x));
}


/*
 * Returns true if player cannot see himself.
 */
int no_lite(void)
{
    return (!player_can_see(char_row, char_col));
}









/*
 * This function helps decide what attr/char to use for a grid.
 *
 * Warning: This function may ONLY be called from "map_info()".
 *
 * Note the "fake" objects such as "OBJ_FLOOR" and "OBJ_GRANITE".
 *
 * Colors (used for floors, walls, quartz):
 *   Defaults to "white"
 *   Option "view_yellow_lite" draws "torch radius" in yellow
 *   Option "view_require_los" draws "hidden grids" dimmer.
 *   Option "view_run_quickly" cancels both those modes.
 *
 * Note that "view_bright_lite" requires "view_pre_compute".
 *
 * Need to handle "MULTI_HUED" property better.  Probably best to
 * use another flag on "wearable objects".  If we can find one.
 *
 * Hack -- only floors, walls, and seams are drawn yellow/bright.
 */
static void map_info_aux(int y, int x, byte *ap, char *cp)
{
    register cave_type *c_ptr;

    /* The "object" to draw */
    inven_type *i_ptr;

    /* Should the object be "dimmed"? */
    bool darken = FALSE;
    bool yellow = FALSE;

    /* Used at the end of the function */
    int mattr, mchar;

    /* Hack -- some fake objects */
    static bool prepare = TRUE;
    static inven_type ii_floor;
    static inven_type ii_granite;
    static inven_type ii_quartz;
    static inven_type ii_magma;


    /* Initialize the fake objects */
    if (prepare) {
	invcopy(&ii_floor, OBJ_FLOOR);
	invcopy(&ii_granite, OBJ_GRANITE_WALL);
	invcopy(&ii_quartz, OBJ_QUARTZ_VEIN);
	invcopy(&ii_magma, OBJ_MAGMA_VEIN);
	prepare = FALSE;
    }


    /* Get the cave */
    c_ptr = &cave[y][x];


    /* Hack -- skip "Field Marks" */
    if (!(c_ptr->info & CAVE_FM)) {

	/* Not illuminated */
	if (!player_can_see(y, x)) {

#ifdef NEW_MAPS
	    (*ap) = c_ptr->mattr;
	    (*cp) = c_ptr->mchar;
#else
	    (*ap) = TERM_WHITE;
	    (*cp) = ' ';
#endif

	    /* Done */
	    return;
	}
    }


#ifdef USE_COLOR

    /* XXX Note -- view_yellow_fast and view_bright_fast suck */

    /* Option -- Draw the "torch-radius" in yellow */
    if (view_yellow_lite && (!find_flag || !view_yellow_fast) &&
	(c_ptr->info & CAVE_TL)) yellow = TRUE;

    /* Option -- Sometimes only visible grids are bright */
    else if (view_pre_compute && view_bright_lite &&
	     !player_can_see(y, x)) darken = TRUE;

#endif


    /* Hack -- invisible traps (treat as "floor") */
    if (i_list[c_ptr->i_idx].tval == TV_INVIS_TRAP) {
	i_ptr = &ii_floor;
    }

    /* Hack -- secret doors (treat as "granite wall") */
    else if (i_list[c_ptr->i_idx].tval == TV_SECRET_DOOR) {
	i_ptr = &ii_granite;
    }

    /* Use the actual object (no color change) */
    else if (c_ptr->i_idx) {
	i_ptr = &i_list[c_ptr->i_idx];
	yellow = darken = FALSE;
    }

    /* Non walls yield "floors" */
    else if (c_ptr->fval < MIN_WALL) {
	i_ptr = &ii_floor;
    }

    /* Handle illuminated normal seams */
    else if (notice_seams && (c_ptr->fval == QUARTZ_WALL)) {
	i_ptr = &ii_quartz;
    }

    /* Handle illuminated "magma" seams */
    else if (notice_seams && (c_ptr->fval == MAGMA_WALL)) {
	i_ptr = &ii_magma;
    }

    /* Handle all left-over rocks */
    else {
	i_ptr = &ii_granite;
    }


    /* Apply the info */
    mchar = inven_char(i_ptr);

#ifdef USE_COLOR
    /* Extract the color */
    mattr = inven_attr(i_ptr);
    if (yellow) mattr = TERM_YELLOW;
    else if (darken) mattr = TERM_GRAY;
    if (!use_color) mattr = TERM_WHITE;
#else
    mattr = TERM_WHITE;
#endif

    /* Accept */
    (*ap) = mattr;
    (*cp) = mchar;
}





/*
 * Extract the attr and char of a given MAP location
 *
 * Many objects are "obvious" (yourself, dark grids, etc)
 * Others need more complex processing, done in "c_loc_symbol()"
 * above (includes "possibly known" floors, walls, and objects).
 *
 * If the object there is Multi-Hued, let it change a lot
 * But in that case, also set "m" to indicate this.
 *
 * Note that monsters can have some "special" flags, including "ATTR_MULTI",
 * which means their color changes, and "ATTR_CLEAR", which means they take
 * the color of whatever is under them, and "CHAR_CLEAR", which means they
 * take the symbol of whatever is under them.  And "CHAR_MULTI" (undefined).
 */
static void map_info(int y, int x, int *m, byte *a, char *c)
{
    byte ma;
    char mc;
    
    cave_type *c_ptr;    


    /* Default to NOT "multihued" */
    (*m) = 0;

    /* Default to "white" */
    (*a) = TERM_WHITE;

    /* Default to "space" */
    (*c) = ' ';


    /* Off the map -- invisible */
    if (!in_bounds2(y, x)) return;


    /* Get the cave */
    c_ptr = &cave[y][x];


    /* Hack -- player is always "visible" unless "running blind" */
    if ((c_ptr->m_idx == 1) && (!find_flag || find_prself)) {

	static bool prepare = TRUE;
	static inven_type ii_player;

	/* Initialize */
	if (prepare) {
	    invcopy(&ii_player, OBJ_PLAYER);
	    prepare = FALSE;
	}

	/* Get the player char/attr */
	(*c) = inven_char(&ii_player);
	(*a) = inven_attr(&ii_player);

	/* All done */
	return;
    }


    /* Mega-Hack -- Blind people see nothing (except themselves) */
    if (p_ptr->blind > 0) return;


    /* Hallucination kicks in occasionally */
    if ((p_ptr->image > 0) && (randint(12) == 1)) {
	(*c) = image_char();
	(*a) = image_attr();
	return;
    }


    /* Examine the ground / object */
    map_info_aux(y, x, &ma, &mc);


    /* Normal, visible monsters "block" the object they are on */
    if ((c_ptr->m_idx > 1) && (m_list[c_ptr->m_idx].ml)) {

	/* Get the monster, and race */
	monster_type *m_ptr = &m_list[c_ptr->m_idx];
	monster_race *r_ptr = &r_list[m_ptr->r_idx];

	/* Use the given attribute unless "clear" */
	if (!(r_ptr->cflags1 & MF1_ATTR_CLEAR)) {

	    /* Extract an attribute */
	    ma = r_attr[m_ptr->r_idx];

	    /* Apply the "multi-hued" flag */
	    if (r_ptr->cflags1 & MF1_ATTR_MULTI) {
		(*m) = 1;
		ma = mh_attr();
	    }
	}

	/* Use the given symbol unless "clear" */
	if (!(r_ptr->cflags1 & MF1_CHAR_CLEAR)) {

	    /* Extract a character */
	    mc = r_char[m_ptr->r_idx];

	    /* Apply the "mimic" flag */
	    if (r_ptr->cflags1 & MF1_ATTR_MULTI) {
		/* XXX */
	    }
	}
    }


#ifdef USE_COLOR

    /* Accept the "attr" */
    (*a) = ma;

#endif

    /* Accept the "char" */
    (*c) = mc;
}






/*
 * Moves the cursor to a given MAP (y,x) location
 */

void move_cursor_relative(int row, int col)
{
    /* Real co-ords convert to screen positions */
    row -= panel_row_prt;
    col -= panel_col_prt;

    /* Go there */
    Term_gotoxy(col, row);
}




#ifdef USE_COLOR
#ifdef USE_MULTIHUED

/*
 * Maximum number of Multi-Hued things
 */
#define MH_MAX 100

/*
 * Hack -- A set of "probably" multi-hued screen locations
 * Each has a location (x,y), and a attr (a) and a char (c).
 * Note that each multi-hued object has its own color progression.
 * Currently used only for multi-hued monsters.
 */
static int mh_y[MH_MAX];
static int mh_x[MH_MAX];
static byte mh_a[MH_MAX];
static char mh_c[MH_MAX];

/*
 * Number of locations in the set
 */
static int mh_n = 0;




/*
 * Forget all the multi-hued info
 * Useful to make sure old data was nuked
 */
void mh_forget(void)
{
    /* Just forget them */
    mh_n = 0;
}


/*
 * Cycle the multi-hued info, taking note of most changes
 * Note that changes are supposed to be noticed and caused
 * by using "mh_print" to display chars.  Note that all of
 * "lite_spot", "lite_monster", and "prt_map" use "mh_print"
 * If not, we will hopefully notice it anyway.
 *
 * This routine will miss changes in which the char is "changed"
 * to the same symbol, and just happens to keep the color that
 * was assigned at the most recent "mh_cycle" or "mh_print"
 *
 * I am thinking that it would be much more efficient to ASSUME
 * that all printing is done by mh_print, and so we do NOT have
 * to check the current contents of the screen.  This would be
 * true, if not for the frequent use of, say, clear_screen().
 *
 * In any case, note that currently this function is dependant on
 * a correctly functioning "Term_what()" function.
 */
void mh_cycle(void)
{
    int i, okay, cx, cy;

    /* If nothing is multihued, skip this function */
    if (mh_n == 0) return;

    /* Find the cursor */
    Term_locate(&cx, &cy);

    /* Turn off the cursor */
    okay = Term_hide_cursor();

    /* Scan the multi-hued set */
    for (i = 0; i<mh_n; ++i)
    {
	int x = mh_x[i];
	int y = mh_y[i];
	byte a = mh_a[i];
	char c = mh_c[i];

	byte sa;
	char sc;

	/* Get the current screen info at that location */
	Term_what(x, y, &sa, &sc);

	/* Notice (usually) when somebody else nukes a multi-hued char */
	if ((sc != c) || (sa != a))
	{
	    /* One less multi-hued char */
	    mh_n--;

	    /* Nuke it, and fill the hole */
	    mh_x[i] = mh_x[mh_n];
	    mh_y[i] = mh_y[mh_n];
	    mh_a[i] = mh_a[mh_n];
	    mh_c[i] = mh_c[mh_n];

	    /* Be sure to redo the new one */
	    i--;
	}

	/* Looks good, change its color */
	else
	{
	    /* Choose a new color */
	    a = mh_attr();

	    /* Save the new multi-hued data */
	    mh_a[i] = a;

	    /* Redisplay the character */
	    Term_draw(x, y, a, c);
	}
    }

    /* Find the cursor */
    Term_gotoxy(cx, cy);

    /* Turn the cursor back on */
    if (!okay) Term_show_cursor();    
}




/*
 * Display an attr/char on the screen, at the given location
 * If "m" is set, add that screen location to our "shimmer set".
 * Otherwise, be sure to remove it from the same set.
 *
 * Hack -- Handle "Multi-Hued" pseudo-attribute
 * When "m" is set, the attribute should "shimmer" over time,
 * starting from "a", for example, when mh_cycle() is called.
 *
 * Note: when very few multicolored items are visible, this
 * routine is barely slowed down at all.  But each shimmering
 * character adds a LOT of overhead.
 *
 * Much of this can be saved by relying on "mh_cycle()" to
 * catch all cases of "loss of shimmer", but that is risky.
 */
void mh_print(char c, byte a, int m, int y, int x)
{
    int i;

    /* Hack -- look up this location in the multi-hued table */
    for (i=0; i<mh_n; i++)
    {
	/* Found it */
	if ((x == mh_x[i]) && (y == mh_y[i])) break;
    }

    /* Handle Multi-Hued requests (unless full) */
    if (m && (i < MH_MAX))
    {
	/* New entry */
	if (i == mh_n)
	{
	    /* One more multi-hued */
	    mh_n++;

	    /* Save the location */
	    mh_x[i] = x;
	    mh_y[i] = y;
	}

	/* Save the data */
	mh_a[i] = a;
	mh_c[i] = c;
    }

    /* Handle Non-Multi-Hued requests */
    else
    {
	if (i < mh_n)
	{
	    /* One less multi-hued char */
	    mh_n--;

	    /* Nuke it, and fill the hole */
	    mh_x[i] = mh_x[mh_n];
	    mh_y[i] = mh_y[mh_n];
	    mh_a[i] = mh_a[mh_n];
	    mh_c[i] = mh_c[mh_n];
	}
    }

    /* Run-time color choosing */
    if (!use_color) a = TERM_WHITE;

    /* Display it */
    Term_draw(x, y, a, c);
}

#else /* USE_MULTIHUED */

void mh_forget(void) {}

void mh_cycle(void) {}

/*
 */
void mh_print(char c, byte a, int m, int y, int x)
{
    /* Run-time color choosing */
    if (!use_color) a = TERM_WHITE;

    /* Simply place the character using the attribute */
    Term_draw(x, y, a, c);
}

#endif /* USE_MULTIHUED */

#else /* USE_COLOR */

void mh_forget(void) {}

void mh_cycle(void) {}

void mh_print(char c, byte a, int m, int y, int x)
{
    /* Simply place the character using no attributes */
    Term_draw(x, y, TERM_WHITE, c);
}

#endif /* USE_COLOR */



/*
 * Simply call "mh_print" (above),
 * but account for "panel-relative" in location
 */
void mh_print_rel(char c, byte a, int m, int y, int x)
{
    /* Only do "legal" locations */
    if (panel_contains(y, x)) {

	/* Do a "real" call to "mh_print()" */
	mh_print(c, a, m, y-panel_row_prt, x-panel_col_prt);
    }
}





/*
 * Allow the player to "take notes" on the terrain
 *
 * This function is only called by "lite_spot()"
 * This function is only called on "legal" grids.
 *
 * This function is basically a giant hack to allow people to play
 * without turning "view_pre_compute" on.
 *
 * This is the ONLY way a player can "remember" things that he cannot
 * currently "see" (at least until "NEW_MAPS" is compiled in).  And even
 * then, this will be the routine that builds the "map".  Conveniently,
 * only one function (lite_spot) updates the screen, and it always calls
 * us first, to take notes on the terrain.
 *
 * Note that we will NOT take notes on "unknown" grids.
 *
 * This function works very well with "view_pre_compute" but I am
 * afraid that otherwise moving monsters nearby in a lit, unexplored
 * room will cause some problems.
 */
static void update_map(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];


    /* Only need to memorize a grid once */
    if (c_ptr->info & CAVE_FM) return;


    /* Hack -- Only memorize grids that can be seen */
    if (!player_can_see(y, x)) return;
    

    /* Option -- field mark every torch-lit grid */
    if (view_torch_grids && (c_ptr->info & CAVE_TL)) {
	c_ptr->info |= CAVE_FM;
	return;
    }
    
    /* Option -- field mark every perma-lit grid */
    if (view_perma_grids && (c_ptr->info & CAVE_PL)) {
	c_ptr->info |= CAVE_FM;
	return;
    }
    
    /* Option -- field mark all "non-floor" grids */
    if (view_wall_memory && !floor_grid_bold(y, x)) {
	c_ptr->info |= CAVE_FM;
	return;
    }

    /* Option -- field mark all "floor landmarks" */
    if (view_xtra_memory && floor_grid_bold(y, x) &&
        (i_list[c_ptr->i_idx].tval >= TV_MIN_VISIBLE)) {
	c_ptr->info |= CAVE_FM;
	return;
    }
}



/*
 * Redraw (on the screen) a given MAP location
 */
void lite_spot(int y, int x)
{
    /* Hack -- Update the map */
    update_map(y, x);

    /* Redraw if on screen */
    if (panel_contains(y, x)) {

	int m;
	byte a;
	char c;

	/* Determine the multi-hued-ity, and attr/char there */
	map_info(y, x, &m, &a, &c);

	/* Redraw the proper thing at the proper relative location */
	mh_print_rel(c, a, m, y, x);
    }
}


/*
 * Draw a monster (at his current location)
 */
void lite_monster(monster_type *m_ptr)
{
    int m_x = (int)m_ptr->fx;
    int m_y = (int)m_ptr->fy;

    /* Hack -- assume telepathy */
    bool ml = m_ptr->ml;

    /* Set the visibility */
    m_ptr->ml = TRUE;

    /* Update this grid */
    lite_spot(m_y, m_x);

    /* Fix the visibility */
    m_ptr->ml = ml;
}





/*
 * Prints the map of the dungeon
 */
void prt_map(void)
{
    register int x, y;

    int m;
    byte a;
    char c;

    int okay;

    /* Hide the cursor */
    okay = Term_hide_cursor();

    /* Forget all the multi-hued stuff (redone below) */
    mh_forget();

    /* Dump the map */    
    for (y = panel_row_min; y <= panel_row_max; y++) {

	/* Erase the map line (needed?) */
	erase_line(1+y-panel_row_min, 13);

	/* Scan the columns of row "y" */
	for (x = panel_col_min; x <= panel_col_max; x++) {

	    /* Determine what is there */
	    map_info(y, x, &m, &a, &c);

	    /* Redraw that grid of the map */
	    mh_print_rel(c, a, m, y, x);
	}
    }

    /* Show the cursor (if necessary) */
    if (!okay) Term_show_cursor();
}














/*
 * And now for something completely different...
 *
 * This suggests a rethinking of the "permanent light" / "temp light"
 * concept.  Perhaps "temp light" should be replaced by "reachable space",
 * that is, we collect a set of "grids which can be seen by the player".
 * They are all marked "temp light" which now means "seen by player", and
 * whenever something major (like player motion) occurs, this "set" of grids
 * is first all marked as "not lit" and then "reconstructed" by combining
 * "lamp radius" with "permanent light" to result in "viewable grids".
 *
 * The algorithm marks grids as "CAVE_VIEW" if the player has "los()" to
 * the grid, and it marks them as "CAVE_XTRA" if, in addition, the "los()"
 * is very "obvious" or "easily determinable".
 *
 * The algorithm is very fast, since it spreads "obvious" grids very quickly,
 * and only has to call "los()" on the borderline cases.  The major axes/diags
 * even terminate early when they hit walls.  I need to find a quick way
 * to "terminate" the other scans.
 *
 * Note that in the worst case (a big empty area with maybe 5% scattered walls),
 * each of the under 1500 grids is checked once, with most of them getting an
 * "instant" rating, and only a small portion requiring a call to "los()".
 *
 * In the "best" case (say, a corridor, with no room at the end), no calls to
 * "los()" are made, even for updating the monsters just outside the corridor.
 * So all I need to do is teach the algorithm to "stop" at "solid walls"...
 *
 * There are two separate flags for "lite" (combining to make four "states"):
 *   CAVE_PL: Perma-Lite: The grid is permanently "illuminated"
 *   CAVE_TL: Torch-Lite: The grid is within the player's "torch" radius
 *
 * There is a hacked flag for "omniscient sight" of a grid:
 *   CAVE_FM: Field-Mark: The grid contents are "remembered" even if not lit
 *
 * There is a special flag for various "temporary" things:
 *   CAVE_SEEN: A temporary flag
 *
 * Hack -- for now, walls, when first seen, are marked as "CAVE_PL" AND "CAVE_FM".
 * This has the effect that they are "always visible" once seen for the first time.
 *
 * Walls, doors, stairs, and "detected objects" are marked as "CAVE_FM" when
 * first "seen" or "detected".
 *
 * Technically, we should have:
 *   CAVE_PL + CAVE_FM: The wall is in a lite room, and has been seen.
 *   CAVE_PL: The wall is in a lite room, but has not been noticed yet.
 *   CAVE_FM: The wall has been noticed, but not lit (?).
 *
 * However, instead, we use a hack involving:
 *   CAVE_INIT: This grid has been "initialized" by the system
 *
 * Floors in lit rooms are not actually set to "Perma-Lit" until the player enters
 * them, at which time "CAVE_INIT" is used to spread "initialization" through the
 * room, also marking walls and landmarks as "CAVE_FM", and marking the whole room
 * as "CAVE_PL".  Note that "CAVE_PL" is NOT as powerful as "CAVE_FM".
 *
 * This will allow "seeing" a wall (in a lit room) before the room itself is
 * actually "displayed".  This will require "pre-scanning" all the lit rooms.
 * And observe that the floors of lit rooms can also be marked as "lit", but
 * not "field marked", until the player enters the room, via "check_view()".
 *
 * Potential advantage: never "field mark" objects, so that they "disappear" when
 * the player leaves the room.  No more "magic doors" or "screaming treasure".
 *
 * Note that the "efficiency issue" is VERY important when the player is running.
 * I am thinking that we could literally "reduce" the "vision" parameter, say,
 * to 1 or 2 or 3, and thus have a much smaller "viewable space".  The player
 * would then NOT see monsters as he ran down halls.  Or, we could turn off the
 * "construct view" flag, and default to using the true "los()" routine for, say,
 * checking monster visibility.
 *
 * Note that running implies two separate hacks:
 *   (1) The light radius is auto-reduced to one grid
 *   (2) The viewable space algorithm needs to be "neutered".
 *
 * Note that we would no longer have to do as many "los()" checks, since once
 * the region has been built, very few things cause it to be "changed" (mostly
 * "light" changes, plus player movement, and the opening/closing of doors).
 * If the player was willing to "forgive" small inconsistancies (until he stood
 * still, or moved), then we could ignore the latter.  Note that technically, the
 * "viewable space" is important, as it indicates which grids will be "noticed"
 * by the player, for things like treasure dropping, and such.  Also, the actual
 * algorithm seems to only call "los()" from zero to ten times, usually only when
 * coming down a corridor into a room, or standing in a room, just misaligned
 * with a corridor.  So if, say, there are five "nearby" monsters, we will be
 * reducing the calls to "los()".
 *
 * I am thinking in terms of an algorithm that "walks" from the central point out
 * to the maximal "distance", at each point, determining the "view" code (above).
 * Note that for each grid not on a major axis or diagonal, the "view" code depends
 * on the "floor_grid_bold()" and "view" of exactly two other grids (the one along
 * the nearest diagonal, and the one next to that one, see "update_view_aux()"...).
 *
 * Notice that we "memorize" the viewable space array, so that at the cost of
 * under 3000 bytes, we reduce the time taken by "forget_view()" to one assignment
 * for each grid actually in the "viewable space".  And for another 3000 bytes,
 * we prevent "erase + redraw" ineffiencies via the "seen" set.
 *
 * A similar thing is done for "forget_lite()" in which case the savings are
 * much less, but save us from doing bizarre "temp_lite" maintenance checking.
 *
 * In the worst case (in the middle of the town), the reachable space actually
 * reaches to the largest possible "circle" of view, just under 1500 grids.
 *
 * Each grid G examines the "state" of two (?) other (adjacent) grids, G1 & G2.
 * If G1 is lite, G is lite.  Else if G2 is lite, G is half.  Else if G1 and G2
 * are both half, G is half.  Else G is dark.  It only takes 2 (or 4) bits to
 * "name" a grid, so (for MAX_RAD of 20) we could use 1600 bytes, and scan the
 * entire possible space (including initialization) in one step per grid.  If
 * we do the "clearing" as a separate step (and use an array of "view" grids),
 * then the clearing will take as many steps as grids that were viewed, and the
 * algorithm will be able to "stop" scanning at various points.
 * Oh, and outside of the "torch radius", only "lite" grids need to be scanned.
 *
 * And note that only "CAVE_PL" and "CAVE_LR" and "CAVE_FM" need to be saved
 * in the savefile, along with "CAVE_SEEN" if we use it, or "CAVE_INIT" if we
 * do not.  In fact, "CAVE_SEEN" and "CAVE_INIT" are very similar...
 *
 * A major "test of worth" is going to be "running".  We will need to "reduce"
 * in some way the "view space" when running.  I am wondering about just lowering
 * the "view distance" to 10 or something.
 *
 * Note that (for free) we get a method of drawing, say, in yellow, the grids 
 * which are hit by the torch, and in orange the ones that we can currently "see",
 * and in "green" the ones that we remember but cannot currently see.
 */





/*
 * Maintain an array of "torch lit" cave grids
 * Note that "radius 11" light only fills 480 grids
 *
 * Currently this accesses "CAVE_TL"
 * Could call this flag "CAVE_LITE"
 */
static int lite_n = 0;
static byte lite_y[500];
static byte lite_x[500];


/*
 * Maintain an array of "viewable cave grids" (see below)
 * The "view radius" must NEVER be larger than 20...
 */
static int view_n = 0;
static byte view_y[1500];
static byte view_x[1500];


/*
 * Number of grids in the "seen" array
 */
#define SEEN_MAX 2000

/*
 * This is the "set" of grids marked as "CAVE_SEEN".
 * These grids have many interpretations, see below.
 *
 * Note that we must be at least 1500 long for "CAVE_VIEW" below.
 *
 * We must also be as large as the largest illuminatable room.
 */
static int seen_n = 0;
static byte seen_y[SEEN_MAX];
static byte seen_x[SEEN_MAX];



#ifdef AUTO_PLAY

extern int auto_cheat_get_view_n(void);
extern byte *auto_cheat_get_view_x(void);
extern byte *auto_cheat_get_view_y(void);

int auto_cheat_get_view_n(void) { return (view_n); }
byte *auto_cheat_get_view_x(void) { return (view_x); }
byte *auto_cheat_get_view_y(void) { return (view_y); }

#endif


/*
 * This routine adds a grid to the "seen" set.
 *
 * This routine had better not be called when the "seen" array is "full".
 */
static void cave_seen_add(int y, int x)
{
    /* Already in the set */
    if (cave[y][x].info & CAVE_SEEN) return;

    /* Paranoia -- verify space */
    if (seen_n == SEEN_MAX) return;
    
    /* Mark the grid as being in the set */
    cave[y][x].info |= CAVE_SEEN;

    /* Add to queue */
    seen_y[seen_n] = y;
    seen_x[seen_n] = x;
    seen_n++;
}



/*
 * This routine clears the entire "seen" set.
 * This routine will "lite_spot()" all non-CAVE_TL grids
 *
 * This routine is used by "update_lite()"
 */
static void cave_seen_wipe_lite(void)
{
    int i;

    /* None to forget */
    if (!seen_n) return;

    /* Clear them all */
    for (i = 0; i < seen_n; i++) {

	int y = seen_y[i];
	int x = seen_x[i];

	/* No longer in the array */
	cave[y][x].info &= ~CAVE_SEEN;

	/* Update only non-CAVE_TL grids */
	if (!(cave[y][x].info & CAVE_TL)) lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
}



/*
 * Copy the "lite" set into the "seen" set, clearing the "CAVE_TL" flag.
 */
static void cave_seen_grab_lite(void)
{
    int i;

    /* None to forget */
    if (!lite_n) return;

    /* Clear them all */
    for (i = 0; i < lite_n; i++) {

	int y = lite_y[i];
	int x = lite_x[i];

	/* Remove it from the "lite" set */
	cave[y][x].info &= ~CAVE_TL;

	/* Add it to the "seen" set */
	cave_seen_add(y, x);
    }

    /* None left */
    lite_n = 0;
}


/*
 * This routine clears the entire "seen" set.
 * This routine will "lite_spot()" all non-CAVE_VIEW grids
 *
 * This routine is used by "update_view()"
 */
static void cave_seen_wipe_view(void)
{
    int i;

    /* None to forget */
    if (!seen_n) return;

    /* Clear them all */
    for (i = 0; i < seen_n; i++) {

	int y = seen_y[i];
	int x = seen_x[i];

	/* No longer in the array */
	cave[y][x].info &= ~CAVE_SEEN;

	/* Update only non-CAVE_TL grids */
	if (!(cave[y][x].info & CAVE_VIEW)) lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
}



/*
 * Copy the "view" set into the "seen" set, clearing the "CAVE_VIEW" flag.
 */
static void cave_seen_grab_view(void)
{
    int i;

    /* None to forget */
    if (!view_n) return;

    /* Clear them all */
    for (i = 0; i < view_n; i++) {

	int y = view_y[i];
	int x = view_x[i];

	/* Remove it from the "view" set */
	cave[y][x].info &= ~CAVE_VIEW;
	cave[y][x].info &= ~CAVE_XTRA;

	/* Add it to the "seen" set */
	cave_seen_add(y, x);
    }

    /* None left */
    view_n = 0;
}


/*
 * The given grid just got lit, wake up monsters
 */
static void wake_monster(int y, int x)
{
    /* Check the monsters */
    monster_type *m_ptr = &m_list[cave[y][x].m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];
	
    /* Perhaps wake up */
    if ((r_ptr->cflags2 & MF2_INTELLIGENT) ||
	(!(r_ptr->cflags2 & MF2_MINDLESS) && (randint(3) == 1)) ||
	(randint(10) == 1)) {

	/* Wake up! */
	m_ptr->csleep = 0;
    }
}


/*
 * This routine clears the entire "seen" set.
 *
 * This routine will Perma-Lite all "seen" grids.
 *
 * This routine is used by "lite_room()"
 *
 * Dark grids are illuminated, and monsters notice it.
 * Monsters that are intelligent wake up all the time,
 * non-MF2_MINDLESS monsters wake up 1/3 the time,
 * and MF2_MINDLESS monsters wake up 1/10 the time -CWS
 */
static void cave_seen_room_lite(void)
{
    int i;

    /* None to forget */
    if (!seen_n) return;

    /* Clear them all */
    for (i = 0; i < seen_n; i++) {

	int y = seen_y[i];
	int x = seen_x[i];

	/* No longer in the array */
	cave[y][x].info &= ~CAVE_SEEN;

	/* Update only non-CAVE_PL grids */
	if (cave[y][x].info & CAVE_PL) continue;

	/* Perma-Lite */
	cave[y][x].info |= CAVE_PL;

	/* Hack -- Update the monster (visibility change) */
	if (cave[y][x].m_idx > 1) update_mon(cave[y][x].m_idx);

	/* Attempt to wake up monsters in that grid */
	wake_monster(y, x);

	/* Lite that spot */
	lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
}



/*
 * This routine clears the entire "seen" set.
 *
 * This routine will Perma-Lite all "seen" grids.
 *
 * This routine is used by "unlite_room()"
 */
static void cave_seen_room_unlite(void)
{
    int i;

    /* None to forget */
    if (!seen_n) return;

    /* Clear them all */
    for (i = 0; i < seen_n; i++) {

	int y = seen_y[i];
	int x = seen_x[i];

	/* No longer in the array */
	cave[y][x].info &= ~CAVE_SEEN;

	/* Only update grids which are changing */
	if (!(cave[y][x].info & (CAVE_PL | CAVE_FM))) continue;

	/* Forget about the grid */
	cave[y][x].info &= ~CAVE_PL;
	cave[y][x].info &= ~CAVE_FM;

	/* Hack -- Update the monster (visibility change) */
	if (cave[y][x].m_idx > 1) update_mon(cave[y][x].m_idx);

	/* Lite that spot */
	lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
}



/*
 * This routine clears the entire "seen" set.
 *
 * This routine will display all "lit" grids.
 *
 * This routine is used by "check_view()"
 *
 * XXX This routine may be broken.
 */
static void cave_seen_room_init(void)
{
    int i;

    /* None to forget */
    if (!seen_n) return;

    /* Clear them all */
    for (i = 0; i < seen_n; i++) {

	int y = seen_y[i];
	int x = seen_x[i];

	/* No longer in the array */
	cave[y][x].info &= ~CAVE_SEEN;

	/* Only handle lit grids */
	if (!(cave[y][x].info & CAVE_PL)) continue;

	/* Hack -- Only do a grid once */
	if (cave[y][x].info & CAVE_INIT) continue;

	/* Memorize the grid */
	cave[y][x].info |= CAVE_FM;
	cave[y][x].info |= CAVE_INIT;

	/* Hack -- Update the monster (visibility change) */
	if (cave[y][x].m_idx > 1) update_mon(cave[y][x].m_idx);

	/* Lite that spot */
	lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
}





/*
 * Actually erase the entire "lite" array, redrawing every grid
 */
void forget_lite(void)
{
    int i;

    /* None to forget */
    if (!lite_n) return;

    /* Clear them all */
    for (i = 0; i < lite_n; i++) {

	/* Forget that we can see it */
	cave[lite_y[i]][lite_x[i]].info &= ~CAVE_TL;

	/* Hack -- update the screen */
	lite_spot(lite_y[i], lite_x[i]);
    }

    /* None left */
    lite_n = 0;
}


/*
 * Mark the given grid as "illuminated by torch lite"
 * Never call this routine when the "lite" array is "full".
 * This routine does a "lite_spot()" on grids for whom the
 * "CAVE_SEEN" flag is NOT set.
 *
 * We should probably do "wake_monster(y, x)" in some cases
 */
static void cave_lite(int y, int x)
{
    /* Already lit */
    if (cave[y][x].info & CAVE_TL) return;

    /* Set the flag */
    cave[y][x].info |= CAVE_TL;

    /* Add to queue */
    lite_y[lite_n] = y;
    lite_x[lite_n] = x;
    lite_n++;

    /* Lite "un-seen" spots */
    if (!(cave[y][x].info & CAVE_SEEN)) lite_spot(y, x);
}



/*
 * Update the current "lite radius" and "illuminate" nearby grids.
 *
 * This routine may only work after "update_view()" has been called.
 *
 * Note that "torch lite" and "perma-lite" are separate things.
 */
void update_lite(void)
{
    int x, y, min_x, max_x, min_y, max_y;

    inven_type *i_ptr;


    /* Check for light being wielded */
    i_ptr = &inventory[INVEN_LITE];

    /* Start with no light */
    cur_lite = 0;

    /* Extract "max radius" from current lite */
    if (i_ptr->tval == TV_LITE) {

	/* Default radius */
	cur_lite = 1;

	/* Lanterns have more power */
	if (i_ptr->sval == SV_LITE_LANTERN) cur_lite = 2;

	/* Most lite's need fuel */
	if (i_ptr->pval <= 0) cur_lite = 0;

	/* Artifact Lites last forever */
	if (i_ptr->sval == SV_LITE_GALADRIEL)    cur_lite = 3;
	else if (i_ptr->sval == SV_LITE_ELENDIL) cur_lite = 2;
	else if (i_ptr->sval == SV_LITE_THRAIN)  cur_lite = 3;
    }

    /* Player is glowing */
    if (p_ptr->lite && (cur_lite < 1)) cur_lite = 1;

    /* Hack -- player is running, reduce the lite */
    if (view_reduce_lite && find_flag && (cur_lite > 1)) cur_lite = 1;

    /* Hack -- player is blind */
    if (p_ptr->blind) cur_lite = 0;


    /* Hack -- player is in a store */
    if (in_store_flag) return;


    /* XXX Hack -- Blind or Dark */
    if (cur_lite <= 0) {

	/* Forget the old lite */
	forget_lite();

	/* Hack -- Draw the player's grid */
	lite_spot(char_row, char_col);

	/* All done */
	return;
    }


    /* Grab the "old" lite */
    cave_seen_grab_lite();


    /* Efficiency -- small lite radius */
    if (cur_lite == 1) {

	/* Scan the "radius one" box */
	for (y = char_row - 1; y <= char_row + 1; y++) {
	    for (x = char_col - 1; x <= char_col + 1; x++) {

		/* Radius one lite is simple */
		cave_lite(y, x);
	    }
	}
    }

    /* Larger radius */    
    else {

	/* Extract maximal legal bounded light box */
	min_y = MY_MAX(0, (char_row - cur_lite));
	max_y = MY_MIN(cur_height-1, (char_row + cur_lite));
	min_x = MY_MAX(0, (char_col - cur_lite));
	max_x = MY_MIN(cur_width-1, (char_col + cur_lite));

	/* Scan that box (the viewable, reachable, portions of it) */
	for (y = min_y; y <= max_y; y++) {
	    for (x = min_x; x <= max_x; x++) {

		/* Viewable, nearby, grids get "torch lit" */
		if (player_has_los(y, x) &&
		    (distance(char_row, char_col, y, x) <= cur_lite)) {

		    /* This grid is "torch lit" */
		    cave_lite(y, x);
		}
	    }
	}
    }


    /* Now "erase" the "old" lite */
    cave_seen_wipe_lite();
}






/*
 * Clear the viewable space
 */
void forget_view(void)
{
    int i;

    /* None to forget */
    if (!view_n) return;

    /* Clear them all */
    for (i = 0; i < view_n; i++) {

	int y = view_y[i];
	int x = view_x[i];

	/* Forget that the grid is viewable */
	cave[y][x].info &= ~CAVE_VIEW;
	cave[y][x].info &= ~CAVE_XTRA;

	/* Hack -- update the screen */
	lite_spot(y, x);
    }

    /* None left */
    view_n = 0;
}



/*
 * Set the "view" flag of the given cave grid
 * Never call this function when the "view" array is full.
 * Never call this function with an "illegal" locations.
 */
static void cave_view(int y, int x)
{
    /* Can only be set once */
    if (cave[y][x].info & CAVE_VIEW) return;

    /* Set the value */
    cave[y][x].info |= CAVE_VIEW;

    /* Add to queue */
    view_y[view_n] = y;
    view_x[view_n] = x;
    view_n++;

    /* Lite "un-seen" spots */
    if (!(cave[y][x].info & CAVE_SEEN)) lite_spot(y, x);
}



/*
 * Another "helper" function, used for "main" grids
 */
static bool update_view_aux_1(int y, int x)
{
    cave_view(y, x);
    return (!floor_grid_bold(y, x));
}


/*
 * Another "helper" function, used for "main" grids
 */
static bool update_view_aux_2(int y, int x)
{
    cave[y][x].info |= CAVE_XTRA;
    cave_view(y, x);
    return (!floor_grid_bold(y, x));
}


/*
 * We are checking the "viewability" of grid (y,x) by the player.
 *
 * We must be rather careful about "falling off the map".  This
 * function assumes that (y,x) is either on the map, or perhaps
 * just over the edge, and in either case, (y1,x1) and (y2,x2)
 * are both on the map.  Thus "floor_grid_bold()".
 *
 * Grid (y1,x1) is on the "diagonal" between (py,px) and (y,x)
 * Grid (y2,x2) is "adjacent", also between (py,px) and (y,x).
 *
 * Note that we are using the "CAVE_XTRA" field for marking grids as
 * "easily viewable".  We can (easily) clear this field in "update_view()".
 *
 * This function adds (y,x) to the "viewable set" if necessary.
 *
 * This function now returns "TRUE" if vision is "blocked" by grid (y,x).
 */
static bool update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    register bool f1, f2, v1, v2, b1, b2;


    /* Check the walls */
    f1 = (floor_grid_bold(y1,x1));
    f2 = (floor_grid_bold(y2,x2));

    /* Totally blocked by physical walls */
    if (!f1 && !f2) return (TRUE);


    /* Check the visibility */
    v1 = (f1 && (cave[y1][x1].info & CAVE_VIEW));
    v2 = (f2 && (cave[y2][x2].info & CAVE_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);

    
    /* Check the "ease" of visibility */
    b1 = (v1 && (cave[y1][x1].info & CAVE_XTRA));
    b2 = (v2 && (cave[y2][x2].info & CAVE_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (b1 && b2) {

	/* Non-floors block vision */
	return (update_view_aux_2(y,x));
    }

    /* Hack -- primary "easy" yields "viewed" */
    else if (b1) {

	/* Non-floors block vision */
	return (update_view_aux_1(y,x));
    }

    /* Hack -- "view" plus "view" yields "view" */
    else if (v1 && v2) {

	/* Non-floors block vision */
	return (update_view_aux_1(y,x));
    }

    /* Hack -- check line of sight */
    else if (los(char_row, char_col, y, x)) {

	/* Non-floors block vision */
	return (update_view_aux_1(y,x));
    }
    
    /* We must not be in "line of sight".  Block vision. */
    return (TRUE);
}



/*
 * Calculate the viewable space
 *
 * Step 1: The player is viewable
 * Step 2: Process the main axes, stop at blockage and prepare Step 4
 * Step 3: Process the main diagonals (stop at first blockage)
 * Step 4: Process each of the eight "octants" (stay inside the "octagon")
 *      4a: Process each strip (each starts adjacent to the diagonal)
 *      4a1: Process each octant's strip, aborting as soon as possible
 *
 * Note that the octant processing involves some pretty interesting
 * observations involving when a grid might possibly be viewable from
 * a given grid, and on the order in which the strips are processed.
 *
 * Note the use of the mathematical facts that:
 *   if (manhatten(dy,dx) < R) then (dist(dy,dx) < R)
 *   if (manhatten(dy,dx) > R*3/2) then (dist(dy,dx) > R)
 *
 * This is based on the observation that (1 < 1.415 < 1.5) and the speed
 * of the "manhatten()" function.  Look for "20" and "30" in the code below.
 * Also look for "15", when "propagating", since "n" is used for dx AND dy.
 * These values are now scaled to "MAX_SIGHT" (assumed to equal 20).
 *
 * A similar calculation is used by the "distance()" function to determine
 * that hypot(dx, dy) is almost equal to ((dx+dy+MAX(dx,dy)) / 2).
 *
 * This "optimization" replaces a "circle" with an "octagon" that completely
 * includes the circle, but gets pretty close to it at the major edges.
 * In this function, we only have to do the manhatten checks one-eighth of
 * the time, since the "octagon" is symetrical.  By "skipping" the corners
 * of the octagon (with very little speed cost), we skip one-eighth of the
 * possible 1681 grids, or 210 of 1681.  So the array only has to hold 1475
 * grids, call it 1500.
 *
 * Note the "optimizations" involving the "se","sw","ne","nw","es","en",
 * "ws","wn" variables.  They work like this: While travelling down the
 * south-bound strip just to the east of the main south axis, as soon as
 * we get to a grid which does not "transmit" viewing, if all of the strips
 * preceding us (in this case, just the main axis) had terminated at or before
 * the same point, then we can stop, and reset the "max distance" to ourself.
 * So, each strip (named by major axis plus offset, thus "se" in this case)
 * maintains a "blockage" variable, initialized during the main axis step,
 * and checks it whenever a blockage is observed.
 *
 * Note also the care taken to prevent "running off the map".  This allows
 * the use of "floor_grid_bold()" in the "update_view_aux()" function.
 */
void update_view(void)
{
    int d, n, m;
    int y, x;

    int se, sw, ne, nw, es, en, ws, wn;

    int over, full = MAX_SIGHT;


    /* Hack -- always recompute the flow first */
    update_flow();
    
    
    /* Hack -- If not pre-computing, just clear it */
    if (!view_pre_compute) {
	forget_view();
	return;
    }


    /* Hack -- reduce view when running */
    if (view_reduce_view && find_flag) {
	full = full - (find_flag-1);
	if (full < 10) full = 10;
    }

    /* Extract the "octagon" limits */
    over = full * 3 / 2;


    /*** Step 0 -- save the old view ***/

    /* Wipe the old stuff */
    cave_seen_grab_view();


    /*** Step 1 -- adjacent grids ***/

    /* Now start on the player */
    y = char_row;
    x = char_col;

    /* Hack -- note obstructed player grid */
    if (update_view_aux_2(y,x)) {
	/* This should never happen */
    }


    /*** Step 2 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y+d,x)) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y-d,x)) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y,x+d)) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y,x-d)) break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;


    /*** Step 3 -- Major Diags ***/

    /* Scan south-east */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y+d,x+d)) break;
    }

    /* Scan south-west */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y+d,x-d)) break;
    }

    /* Scan north-east */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y-d,x+d)) break;
    }

    /* Scan north-west */
    for (d = 1; d <= full; d++) {
	if (update_view_aux_2(y-d,x-d)) break;
    }


    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++) {

	int full_n = full - n;
	int over_n_n = over - n - n;
	int limit = MY_MIN(full_n, over_n_n);
	
	int ypn = y + n;
	int ymn = y - n;
	int xpn = x + n;
	int xmn = x - n;


	/* South strip */
	if (ypn < cur_height-1) {

	    /* Maximum distance */
	    m = MY_MIN(limit, (cur_height-1) - ypn);
	
	    /* East side */
	    if (xpn <= cur_width-1) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ypn+d,xpn,ypn+d-1,xpn-1,ypn+d-1,xpn)) {
			if (n+d >= se) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > se) se = n+d;
	    }

	    /* West side */
	    if (xmn >= 0) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ypn+d,xmn,ypn+d-1,xmn+1,ypn+d-1,xmn)) {
			if (n+d >= sw) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > sw) sw = n+d;
	    }
	}


	/* North strip */
	if (ymn > 0) {

	    /* Maximum distance */
	    m = MY_MIN(limit, ymn);
	
	    /* East side */
	    if (xpn <= cur_width-1) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ymn-d,xpn,ymn-d+1,xpn-1,ymn-d+1,xpn)) {
			if (n+d >= ne) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > ne) ne = n+d;
	    }

	    /* West side */
	    if (xmn >= 0) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ymn-d,xmn,ymn-d+1,xmn+1,ymn-d+1,xmn)) {
			if (n+d >= nw) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > nw) nw = n+d;
	    }
	}
	

	/* East strip */
	if (xpn < cur_width-1) {

	    /* Maximum distance */
	    m = MY_MIN(limit, (cur_width-1) - xpn);
	
	    /* South side */
	    if (ypn <= cur_height-1) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ypn,xpn+d,ypn-1,xpn+d-1,ypn,xpn+d-1)) {
			if (n+d >= es) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > es) es = n+d;
	    }

	    /* North side */
	    if (ymn >= 0) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ymn,xpn+d,ymn+1,xpn+d-1,ymn,xpn+d-1)) {
			if (n+d >= en) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > en) en = n+d;
	    }
	}


	/* West strip */
	if (xmn > 0) {

	    /* Maximum distance */
	    m = MY_MIN(limit, xmn);
	
	    /* South side */
	    if (ypn <= cur_height-1) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ypn,xmn-d,ypn-1,xmn-d+1,ypn,xmn-d+1)) {
			if (n+d >= ws) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > ws) ws = n+d;
	    }

	    /* North side */
	    if (ymn >= 0) {

		/* Scan */
		for (d = 1; d <= m; d++) {

		    /* Check grid "d" in strip "n", abort if totally blocked */
		    if (update_view_aux(ymn,xmn-d,ymn+1,xmn-d+1,ymn,xmn-d+1)) {
			if (n+d >= wn) break;
		    }
		}

		/* Maintain the "strip blocker" */
		if (n+d > wn) wn = n+d;
	    }
	}
    }


    /*** Step 5 -- Remove the "old" view space ***/
    cave_seen_wipe_view();
}






/*
 * Aux function -- see below
 */
static void cave_seen_room_aux(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Avoid infinite recursion */
    if (c_ptr->info & CAVE_SEEN) return;

    /* Do not "leave" the current room */
    if (!(c_ptr->info & CAVE_LR)) return;

    /* Add this grid to the queue */
    cave_seen_add(y, x);
}





/*
 * This function "initializes" a lit room when first "entered".
 *
 * This function is NOT used if the "view_pre_compute" flag is set.
 * Those players get the advantage of seeing the room ahead of time
 * as they walk down the corridor, but lose the ability to get a
 * full picture of vaults and pillar filled rooms upon entry.
 *
 * Systems which are not actually recomputing the view must somehow
 * notice when a new "region" has entered the "viewable space".  This
 * function provides such a hook, allowing never-before seen rooms to
 * be displayed.  Note that only the "lit" grids are actually displayed,
 * but this is okay, since the dark ones will be caught at a later time.
 * 
 * Note that this function will check each grid in a dungeon at
 * most once, and always in "groups" (logical "rooms").  Once
 * a grid (and its "friends") have been checked by this function,
 * they are marked as "CAVE_INIT".  Note that since "CAVE_INIT" is
 * NOT saved in the save-file, restoring a game causes every room
 * to require re-CAVE_INIT-ing, but that causes no harm.
 *
 * This is a big hack, to allow "proper" functioning when:
 * "view_pre_compute" is FALSE and "view_perma_grids" is TRUE.
 * Note that this is the "normal" state of the game.  This function
 * is required so that "perma-lit" rooms that the player has never
 * seen can be "pre-memorized" when he enters that room.
 *
 * Note that if "view_pre_compute" is FALSE, and "view_perma_grids" is
 * also FALSE, then all sorts of weird things may happen.  Actually,
 * it appears as though the only effect will be to NOT let the player
 * memorize the "floors" of perma-lit rooms.
 *
 * This function gives a player "permanent knowledge" of a room when
 * it is first "entered" (see "check_view()").  This includes permanent
 * knowledge about the "floor" of the room.  In addition, it "displays"
 * the room, even if some of the grids are not "visible" to the player.
 *
 * Note that "wiz_lite()" must mark rooms as "seen", since otherwise
 * the player will not see monsters correctly.
 *
 * We are only called when the player "notices" that he is inside
 * a room that he has never seen before.
 */
void check_view(void)
{
    int i, x, y;

    int y1 = char_row;
    int x1 = char_col;
    

    /* All done if blind */
    if (p_ptr->blind) return;

    /* Do not check view if pre-computing view */
    if (view_pre_compute) return;

    
    /* Add the initial grid */
    cave_seen_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < seen_n; i++) {

	x = seen_x[i], y = seen_y[i];
	
	/* Walls get lit, but stop light */
	if (!floor_grid_bold(y, x)) continue;
	
	/* Spread adjacent */
	cave_seen_room_aux(y + 1, x);
	cave_seen_room_aux(y - 1, x);
	cave_seen_room_aux(y, x + 1);
	cave_seen_room_aux(y, x - 1);

	/* Spread diagonal */
	cave_seen_room_aux(y + 1, x + 1);
	cave_seen_room_aux(y - 1, x - 1);
	cave_seen_room_aux(y - 1, x + 1);
	cave_seen_room_aux(y + 1, x - 1);
    }
    
    /* Now, lite them all up at once */
    cave_seen_room_init();
}


/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y1, int x1)
{
    int i, x, y;
    
    /* Add the initial grid */
    cave_seen_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < seen_n; i++) {

	x = seen_x[i], y = seen_y[i];
	
	/* Walls get lit, but stop light */
	if (!floor_grid_bold(y, x)) continue;
	
	/* Spread adjacent */
	cave_seen_room_aux(y + 1, x);
	cave_seen_room_aux(y - 1, x);
	cave_seen_room_aux(y, x + 1);
	cave_seen_room_aux(y, x - 1);

	/* Spread diagonal */
	cave_seen_room_aux(y + 1, x + 1);
	cave_seen_room_aux(y - 1, x - 1);
	cave_seen_room_aux(y - 1, x + 1);
	cave_seen_room_aux(y + 1, x - 1);
    }
    
    /* Now, lite them all up at once */
    cave_seen_room_lite();

    /* Hack -- check the view */
    check_view();
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y1, int x1)
{
    int i, x, y;
    
    /* Add the initial grid */
    cave_seen_room_aux(y1, x1);

    /* Spread, breadth first */
    for (i = 0; i < seen_n; i++) {

	x = seen_x[i], y = seen_y[i];
	
	/* Walls get dark, but stop darkness */
	if (!floor_grid_bold(y, x)) continue;
	
	/* Spread adjacent */
	cave_seen_room_aux(y + 1, x);
	cave_seen_room_aux(y - 1, x);
	cave_seen_room_aux(y, x + 1);
	cave_seen_room_aux(y, x - 1);

	/* Spread diagonal */
	cave_seen_room_aux(y + 1, x + 1);
	cave_seen_room_aux(y - 1, x - 1);
	cave_seen_room_aux(y - 1, x + 1);
	cave_seen_room_aux(y + 1, x - 1);
    }
    
    /* Now, darken them all at once */
    cave_seen_room_unlite();
    
    /* Hack -- check the view */
    check_view();
}



/*
 * Convert "directions" into "offsets"
 */ 
static int dx[10] = {0, -1, 0, 1, -1, 0, 1, -1, 0, 1};
static int dy[10] = {0, 1, 1, 1, 0, 0, 0, -1, -1, -1};
static int dd[9] =  {2, 8, 6, 4, 3, 1, 9, 7, 5};


/*
 * Hack -- provide some "speed" for the "flow" code
 * This entry is the "current index" for the "when" field
 * Note that a "when" value of "zero" means "not used".
 *
 * Note that the "cost" indexes from 1 to 127 are for
 * "old" data, and from 128 to 255 are for "new" data.
 *
 * This means that as long as the player does not "teleport",
 * then any monster up to 128 + MONSTER_FLOW_DEPTH will be
 * able to track down the player, and in general, will be
 * able to track down either the player or a position recently
 * occupied by the player.
 */
static int flow_n = 0;


/*
 * Hack -- forget the "flow" information
 */
void forget_flow(void)
{

#ifdef MONSTER_FLOW

    int x, y;

    /* Nothing to forget */
    if (!flow_n) return;

    /* Check the entire dungeon */
    for (y = 0; y < cur_height; y++) {
	for (x = 0; x < cur_width; x++) {

	    /* Forget the old data */
	    cave[y][x].cost = 0;
	    cave[y][x].when = 0;
	}
    }

    /* Start over */
    flow_n = 0;
    
#endif

}


#ifdef MONSTER_FLOW

/*
 * Hack -- Allow us to treat the "seen" array as a queue
 */
static int flow_head = 0;
static int flow_tail = 0;


/*
 * Take note of a reachable grid
 */
static void update_flow_aux(int y, int x, int n)
{
    int old_head = flow_head;

    /* Paranoia -- require valid grids */
    if (!in_bounds(y, x)) return;
        
    /* Ignore "pre-stamped" entries */
    if (cave[y][x].when == flow_n) return;

    /* Ignore "wall" grids (but not doors) */
    if (cave[y][x].fval >= MIN_WALL) return;

    /* Hack -- Ignore "rubble" since no monsters eat rubble */
    if (i_list[cave[y][x].i_idx].tval == TV_RUBBLE) return;
    
    /* Save the time-stamp */
    cave[y][x].when = flow_n;

    /* Save the flow cost */
    cave[y][x].cost = n;

    /* Hack -- limit flow depth */
    if (n == MONSTER_FLOW_DEPTH) return;
    
    /* Enqueue that entry */
    seen_y[flow_head] = y;
    seen_x[flow_head] = x;

    /* Advance the queue */
    if (++flow_head == SEEN_MAX) flow_head = 0;

    /* Hack -- notice overflow by forgetting new entry */
    if (flow_head == flow_tail) flow_head = old_head;
}

#endif


/*
 * Hack -- fill in the "cost" field of every grid that the player
 * can "reach" with the number of steps needed to reach that grid.
 * This also yields the "distance" of the player from every grid.
 *
 * In addition, mark the "when" of the grids that can reach
 * the player with the incremented value of "flow_n".
 *
 * Hack -- use the "seen" array as a "circular queue".
 *
 * We do not need a priority queue because the cost from grid
 * to grid is always "one" and we process them in order.
 */
void update_flow(void)
{

#ifdef MONSTER_FLOW

    int x, y, d;

    /* Hack -- disabled */
    if (!flow_by_sound) return;
    
    /* Paranoia -- make sure the array is empty */
    if (seen_n) return;
    
    /* Cycle the old entries (once per 128 updates) */
    if (flow_n == 255) {

	/* Rotate the time-stamps */
	for (y = 0; y < cur_height; y++) {
	    for (x = 0; x < cur_width; x++) {
		register int w = cave[y][x].when;
		cave[y][x].when = (w > 128) ? (w - 128) : 0;
	    }
	}

	/* Restart */
	flow_n = 127;
    }

    /* Start a new flow (never use "zero") */
    flow_n++;
    

    /* Reset the "queue" */
    flow_head = flow_tail = 0;
    
    /* Add the player's grid to the queue */
    update_flow_aux(char_row, char_col, 0);

    /* Now process the queue */
    while (flow_head != flow_tail) {

	/* Extract the next entry */
	y = seen_y[flow_tail];
	x = seen_x[flow_tail];

	/* Forget that entry */
	if (++flow_tail == SEEN_MAX) flow_tail = 0;

	/* Add the "children" */
	for (d = 0; d < 8; d++) {

	    /* Add that child if "legal" */
	    update_flow_aux(y+dy[dd[d]], x+dx[dd[d]], cave[y][x].cost+1);
	}
    }
    
    /* Forget the flow info */
    flow_head = flow_tail = 0;
#endif

}







/*
 * Map the current panel (plus some) ala "detection"
 * This function is a big hack -- see "update_map()".
 */
void map_area(void)
{
    register cave_type *c_ptr;
    register int        dx, dy, x, y;
    int                 y1, y2, x1, x2;

    /* Pick an area to map */
    y1 = panel_row_min - randint(10);
    y2 = panel_row_max + randint(10);
    x1 = panel_col_min - randint(20);
    x2 = panel_col_max + randint(20);

    /* Speed -- shrink to fit legal bounds (for floors) */
    if (y1 < 1) y1 = 1;
    if (y2 > cur_height-2) y2 = cur_height-2;
    if (x1 < 1) x1 = 1;
    if (x2 > cur_width-2) x2 = cur_width-2;

    /* Scan that area */
    for (y = y1; y <= y2; y++) {
	for (x = x1; x <= x2; x++) {

	    c_ptr = &cave[y][x];

	    /* All non-walls are "checked" */
	    if (c_ptr->fval < MIN_WALL) {

		/* Field mark "landmark" objects */
		if (c_ptr->i_idx) {

		    /* Notice landmark objects */
		    if (i_list[c_ptr->i_idx].tval >= TV_MIN_VISIBLE) {

			/* Memorize the object */
			c_ptr->info |= CAVE_FM;
		    }
		}

		/* Field-Mark the "true" walls */
		for (dy = -1; dy <= 1; dy++) {
		    for (dx = -1; dx <= 1; dx++) {

			if (!dx && !dy) continue;

			c_ptr = &cave[y+dy][x+dx];

			/* Perma-light "significant" walls */
			if (c_ptr->fval >= MIN_WALL) {

			    /* Memorize the walls */
			    c_ptr->info |= CAVE_FM;
			}
		    }
		}
	    }
	}
    }

    /* Redraw the map */
    prt_map();
}



/*
 * Light up the dungeon.
 */
void wiz_lite(void)
{
    register cave_type *c_ptr;
    register int        yy, xx, y, x;

    /* Perma-light all open space and adjacent walls */
    for (y = 1; y < cur_height-1; y++) {
	for (x = 1; x < cur_width-1; x++) {

	    /* Process all non-walls */
	    if (cave[y][x].fval < MIN_WALL) {

		/* Perma-lite all grids touching those grids */
		for (yy = y - 1; yy <= y + 1; yy++) {
		    for (xx = x - 1; xx <= x + 1; xx++) {

			/* Get the grid */
			c_ptr = &cave[yy][xx];

			/* Perma-lite all the grid */
			c_ptr->info |= CAVE_PL;

			/* Hack -- Field Mark all of the objects */
			c_ptr->info |= CAVE_FM;

			/* Mega-Hack -- mark rooms as initialized */
			if (c_ptr->info & CAVE_LR) c_ptr->info |= CAVE_INIT;
		    }
		}
	    }
	}
    }

    /* Redraw the map */    
    prt_map();
}


/*
 * Hack -- Darken the dungeon.  And erase the map.
 * This function makes very little sense...
 */
void wiz_dark(void)
{
    register int        y, x;
    register cave_type *c_ptr;

    /* Forget lite */
    forget_lite();

    /* Darken everything */
    for (y = 0; y < cur_height; y++) {
	for (x = 0; x < cur_width; x++) {
	    c_ptr = &cave[y][x];
	    c_ptr->info &= ~CAVE_PL;
	    c_ptr->info &= ~CAVE_FM;
	}
    }

    /* Re-examine lite */
    update_lite();

    /* Redraw the map */    
    prt_map();


    /* Check the view */
    check_view();
}








/* "symbol" definitions used by screen_map() */

#if defined(MSDOS) && defined(ANSI)

# define CH_TL (ansi ? 201 : '+')
# define CH_TR (ansi ? 187 : '+')
# define CH_BL (ansi ? 200 : '+')
# define CH_BR (ansi ? 188 : '+')
# define CH_HE (ansi ? 205 : '-')
# define CH_VE (ansi ? 186 : '|')

#else

# define CH_TL '+'
# define CH_TR '+'
# define CH_BL '+'
# define CH_BR '+'
# define CH_HE '-'
# define CH_VE '|'

#endif

/* Display highest priority object in the RATIO by RATIO area */
#define	RATIO 3

/* Display the entire map */
#define MAP_HGT (MAX_HEIGHT / RATIO)
#define MAP_WID (MAX_WIDTH / RATIO)

/*
 * Display a small-scale map of the dungeon
 *
 * Verify that the "priority" function works.
 */
void screen_map(void)
{
    register int i, j, x, y;

    register byte ta;
    register char tc;

    int m, okay;

    char mc[MAP_HGT + 2][MAP_WID + 2];
    char ma[MAP_HGT + 2][MAP_WID + 2];

    int myrow = -1, mycol = -1;

    byte priority[256];



    /* Default priority used for objects and such */
    for (i = 0; i < 256; i++) priority[i] = 20;

    /* A few things are special */    
    priority[(byte)k_char[OBJ_PLAYER]] = 30;
    priority[(byte)k_char[OBJ_UP_STAIR]] = 25;
    priority[(byte)k_char[OBJ_DOWN_STAIR]] = 25;
    priority[(byte)k_char[OBJ_CLOSED_DOOR]] = 19;
    priority[(byte)k_char[OBJ_OPEN_DOOR]] = 17;
    priority[(byte)k_char[OBJ_MAGMA_VEIN]] = 16;
    priority[(byte)k_char[OBJ_QUARTZ_VEIN]] = 16;
    priority[(byte)k_char[OBJ_GRANITE_WALL]] = 15;
    priority[(byte)k_char[OBJ_FLOOR]] = 10;
    priority[(byte)(' ')] = 5;


    /* Clear the chars and attributes */
    for (y = 0; y < MAP_HGT+2; ++y) {
	for (x = 0; x < MAP_WID+2; ++x) {
	    ma[y][x] = TERM_WHITE;
	    mc[y][x] = ' ';
	}
    }

    x = MAP_WID + 1;
    y = MAP_HGT + 1;

    mc[0][0] = CH_TL;
    mc[0][x] = CH_TR;
    mc[y][0] = CH_BL;
    mc[y][x] = CH_BR;

    for (x = 1; x <= MAP_WID; x++) mc[0][x] = mc[y][x] = CH_HE;
    for (y = 1; y <= MAP_HGT; y++) mc[y][0] = mc[y][x] = CH_VE;

    for (i = 0; i < cur_width; ++i) {
	for (j = 0; j < cur_height; ++j) {

	    /* Index into mc/ma */
	    x = i / RATIO + 1;
	    y = j / RATIO + 1;

	    /* Extract the current attr/char at that map location */
	    map_info(j, i, &m, &ta, &tc);

	    /* If this thing is more important, save it instead */
	    if (priority[(int)(mc[y][x])] < priority[(int)(tc)]) {
		mc[y][x] = tc; ma[y][x] = ta;
	    }

	    /* Remember where we saw the "player" */
	    if (tc == k_char[OBJ_PLAYER]) {
		mycol = x; myrow = y;
	    }
	}
    }

    /* Save the screen */
    save_screen();

    /* Hide the cursor */
    okay = Term_hide_cursor();

    /* Clear the screen */
    Term_clear();

    /* Display each map line in order */
    for (y = 0; y < MAP_HGT+2; ++y) {
	Term_gotoxy(0, y);
	for (x = 0; x < MAP_WID+2; ++x) {

	    ta = ma[y][x];
	    tc = mc[y][x];

#ifdef USE_COLOR
	    if (!use_color) ta = TERM_WHITE;
#else
	    ta = TERM_WHITE;
#endif

	    Term_addch(ta, tc);
	}
    }

    /* Wait for it */
    put_str("Hit any key to continue", 23, 23);

    /* Hilite the player */
    if (mycol >= 0) move_cursor(myrow, mycol);

    /* Show the cursor */
    if (!okay) Term_show_cursor();

    /* Get any key */
    inkey();

    /* Restore the screen */
    restore_screen();
}





