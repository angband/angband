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
 * Is a given location "open" like a floor?
 * That is, can it legally be "walked" on?
 * Note that "illegal" locations are treated as "walls".
 * Note that "open doors" count as "floor space".
 * But walls, rubble, and closed doors do not.
 */
bool floor_grid(int y, int x)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    /* Out of bounds */
    if (!in_bounds(y,x)) return (FALSE);

    /* Get that location */
    c_ptr = &cave[y][x];

    /* Solid rock (and doors) are invalid */
    if (c_ptr->fval >= MIN_WALL) return (FALSE);

    /* No object, must be a floor */
    if (c_ptr->i_idx == 0) return (TRUE);
    
    /* Get the object */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Rubble and closed (and secret) doors are blockages */
    if (i_ptr->tval == TV_RUBBLE) return (FALSE);
    if (i_ptr->tval == TV_CLOSED_DOOR) return (FALSE);
    if (i_ptr->tval == TV_SECRET_DOOR) return (FALSE);
    
    /* Must be open floor */
    return (TRUE);
}


/*
 * Determine if a given location is a floor with no objects on it.
 */
bool clean_grid(int y, int x)
{
    cave_type *c_ptr;

    /* Out of bounds */
    if (!in_bounds(y,x)) return (FALSE);

    /* Get that location */
    c_ptr = &cave[y][x];

    /* Objects are "messy" */
    if (c_ptr->i_idx) return (FALSE);
    
    /* Blockage is "messy" */
    if (c_ptr->fval >= MIN_WALL) return (FALSE);
    
    /* Assume clean */
    return (TRUE);
}


/*
 * Is a given location "valid" for placing things?
 * Note that solid rock, doors, and rubble evaluate as "valid".
 * Note that artifacts, store doors, and stairs, do not.
 * See also "clean_grid()" and "floor_grid()" above.
 * This function is usually "combined" with "floor_grid()".
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
 * occur if deltaX and deltaY exceed 90.
 *
 * Also note that this function and the "move towards target" code do NOT
 * share the same properties.  Thus, you can see someone, target them, and
 * then fire a bolt at them, and the bolt will hit a wall, not them.  XXX.
 */

int los(int fromY, int fromX, int toY, int toX)
{
    register int deltaX, deltaY;

    deltaX = toX - fromX;
    deltaY = toY - fromY;

    /* Adjacent?  Or equal? */
    if ((deltaX < 2) && (deltaX > -2) && (deltaY < 2) && (deltaY > -2)) {
	return TRUE;
    }

    /* Along a vertical path */
    if (deltaX == 0) {
	register int p_y;
	if (deltaY < 0) {
	    register int tmp;
	    tmp = fromY;
	    fromY = toY;
	    toY = tmp;
	}
	for (p_y = fromY + 1; p_y < toY; p_y++) {
	    if (!floor_grid(p_y,fromX)) return FALSE;
	}
	return TRUE;
    }

    /* Along a horizontal path */
    if (deltaY == 0) {
	register int p_x;
	if (deltaX < 0) {
	    register int tmp;
	    tmp = fromX;
	    fromX = toX;
	    toX = tmp;
	}
	for (p_x = fromX + 1; p_x < toX; p_x++) {
	    if (!floor_grid(fromY,p_x)) return FALSE;
	}
	return TRUE;
    }

    /* handle Knightlike shapes -CWS */
    if (MY_ABS(deltaX) == 1) {
	if (deltaY == 2) {
	    if (floor_grid(fromY + 1,fromX)) return TRUE;
	}
	else if (deltaY == (-2)) {
	    if (floor_grid(fromY - 1,fromX)) return TRUE;
	}
    }
    else if (MY_ABS(deltaY) == 1) {
	if (deltaX == 2) {
	    if (floor_grid(fromY,fromX + 1)) return TRUE;
	}
	else if (deltaX == (-2)) {
	    if (floor_grid(fromY,fromX - 1)) return TRUE;
	}
    }

/*
 * Now, we've eliminated all the degenerate cases. In the computations below,
 * dy (or dx) and m are multiplied by a scale factor, scale = abs(deltaX *
 * deltaY * 2), so that we can use integer arithmetic. 
 */

    {
	register int        p_x,	/* x position			 */
			    p_y,	/* y position			 */
			    scale2;	/* above scale factor / 2	 */
	int                 scale,	/* above scale factor		 */
			    xSign,	/* sign of deltaX		 */
			    ySign,	/* sign of deltaY		 */
			    m;		/* slope or 1/slope of LOS	 */

	scale2 = MY_ABS(deltaX * deltaY);
	scale = scale2 << 1;
	xSign = (deltaX < 0) ? -1 : 1;
	ySign = (deltaY < 0) ? -1 : 1;


	/* Travel from one end of the line to the other, */
	/* oriented along the longer axis. */

	if (MY_ABS(deltaX) >= MY_ABS(deltaY)) {
	    register int        dy;  /* "fractional" y position	 */

	/*
	 * We start at the border between the first and second tiles, where
	 * the y offset = .5 * slope.  Remember the scale factor.  We have: 
	 *
	 * m = deltaY / deltaX * 2 * (deltaY * deltaX) = 2 * deltaY * deltaY. 
	 */

	    dy = deltaY * deltaY;
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
		if (!floor_grid(p_y,p_x)) return FALSE;
		dy += m;
		if (dy < scale2) {
		    p_x += xSign;
		}
		else if (dy > scale2) {
		    p_y += ySign;
		    if (!floor_grid(p_y,p_x)) return FALSE;
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
	    register int        dx;/* "fractional" x position	 */

	    dx = deltaX * deltaX;
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
		if (!floor_grid(p_y,p_x)) return FALSE;
		dx += m;
		if (dx < scale2) {
		    p_y += ySign;
		}
		else if (dx > scale2) {
		    p_x += xSign;
		    if (!floor_grid(p_y,p_x)) return FALSE;
		    dx -= scale;
		    p_y += ySign;
		}
		else {
		    p_x += xSign;
		    dx -= scale;
		    p_y += ySign;
		}
	    }
	    return TRUE;
	}
    }
}








/*
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
 * Later, we can use this flag for noting the "first observation" of a grid:
 *   CAVE_SEEN: The grid has been observed by the player
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
 * on the "floor_grid()" and "view" of exactly two other grids (the one along the
 * nearest diagonal, and the one next to that one, see "update_view_aux()"...).
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
 * This is the "set" of grids marked as "CAVE_SEEN".
 * These grids have many interpretations, see below.
 *
 * Note that we must be at least 1500 long for "CAVE_VIEW" below.
 */
static int seen_n = 0;
static byte seen_y[2000];
static byte seen_x[2000];





/*
 * This routine adds a grid to the "seen" set.
 *
 * This routine had better not be called when the "seen" array is "full".
 */
static void cave_seen_add(int y, int x)
{
    /* Already in the set */
    if (cave[y][x].info & CAVE_SEEN) return;

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
 * This routine clears the entire "seen" set.
 *
 * This routine will Perma-Lite all "seen" grids.
 *
 * This routine is used by "lite_room()"
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

	/* Update only non-CAVE_TL grids */
	if (cave[y][x].info & CAVE_PL) continue;

	/* Perma-Lite */
	cave[y][x].info |= CAVE_PL;
	
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
    
    
    /* Hack -- be very efficient with "radius one" light */
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
    return (!floor_grid(y, x));
}


/*
 * Another "helper" function, used for "main" grids
 */
static bool update_view_aux_2(int y, int x)
{
    cave[y][x].info |= CAVE_XTRA;
    cave_view(y, x);
    return (!floor_grid(y, x));
}


/*
 * We are checking the "viewability" of grid (y,x) by the player.
 *
 * Grid (y1,x1) is on the "diagonal" between (py,px) and (y,x)
 * Grid (y2,x2) is "adjacent", also between (py,px) and (y,x).
 *
 * We must be rather careful about "falling off the map"
 *
 * Note that we are using the "CAVE_XTRA" field for marking grids as
 * "easily viewable".  We can (easily) clear this field in "update_view()".
 *
 * This function now returns "TRUE" if vision is "blocked" by grid (y,x).
 */
static bool update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, b1, b2;
    
    /* Check the walls */
    f1 = floor_grid(y1,x1);
    f2 = floor_grid(y2,x2);
    
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
 * onservations involving when a grid might possibly be viewable from
 * a given grid, and on the order in which the strips are processed.
 *
 * Note the use of the mathematical facts that:
 *   if (manhatten(dy,dx) < R) then (dist(dy,dx) < R)
 *   if (manhatten(dy,dx) > R*3/2) then (dist(dy,dx) > R)
 *
 * This is based on the observation that (1 < 1.415 < 1.5) and the speed
 * of the "manhatten()" function.  Look for "20" and "30" in the code below.
 * Also look for "15", when "propagating", since "n" is used for dx AND dy.
 *
 * This "optimization" replaces a "circle" with an "octagon" that completely
 * includes the circle, but gets pretty close to it at the major edges.
 * In this function, we only have to do the manhatten checks one-eighth of
 * the time, since the "octagon" is symetrical.  By "skipping" the corners
 * of the octagon (with very little speed cost), we skip one-eighth of the
 * possible 1681 grids, or 210 of 1681.  So the array only has to hold 1475
 * grids, call it 1500.
 *
 * Note that this algorithm is hard-coded to a "maximum sight" of 20 grids.
 * Changing this would require calculating "max", "max/2", and "max*3/2".
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
 */
void update_view(void)
{
    int d, n;
    int y, x;

    int se, sw, ne, nw, es, en, ws, wn;
    
    int over, full = MAX_SIGHT;


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
	int ypn = y + n;
	int ymn = y - n;
	int xpn = x + n;
	int xmn = x - n;
	
	
	/* Scan south strip, east side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ypn+d,xpn,ypn+d-1,xpn-1,ypn+d-1,xpn)) {
		if (n+d > se) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > se) se = n+d;


	/* Scan south strip, west side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ypn+d,xmn,ypn+d-1,xmn+1,ypn+d-1,xmn)) {
		if (n+d > sw) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > sw) sw = n+d;


	/* Scan north strip, east side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ymn-d,xpn,ymn-d+1,xpn-1,ymn-d+1,xpn)) {
		if (n+d > ne) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > ne) ne = n+d;


	/* Scan north strip, west side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ymn-d,xmn,ymn-d+1,xmn+1,ymn-d+1,xmn)) {
		if (n+d > nw) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > nw) nw = n+d;


	/* Scan east strip, south side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ypn,xpn+d,ypn-1,xpn+d-1,ypn,xpn+d-1)) {
		if (n+d > es) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > es) es = n+d;


	/* Scan east strip, north side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ymn,xpn+d,ymn+1,xpn+d-1,ymn,xpn+d-1)) {
		if (n+d > en) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > en) en = n+d;


	/* Scan west strip, south side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ypn,xmn-d,ypn-1,xmn-d+1,ypn,xmn-d+1)) {
		if (n+d > ws) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > ws) ws = n+d;


	/* Scan west strip, north side */
	for (d = 1; (d <= full_n) && (d <= over_n_n); d++) {

	    /* Check grid "d" in strip "n", abort if totally blocked */
            if (update_view_aux(ymn,xmn-d,ymn+1,xmn-d+1,ymn,xmn-d+1)) {
		if (n+d > wn) break;
	    }
        }

	/* Maintain the "strip blocker" */
	if (n+d > wn) wn = n+d;
    }
    

#if 0

    /* XXX Old, non "optimizing" method */
    
    /* Now fill in the holes, one group of strips at a time */
    for (n = 1; n <= 15; n++) {

	/* Now scan the next strips */
	for (d = 1; (d <= 20-n) && (d <= 30-n-n); d++) {

	    /* South (east side) */
            update_view_aux(y+n+d,x+n,y+n+d-1,x+n-1,y+n+d-1,x+n);
	    
	    /* South (west side) */
            update_view_aux(y+n+d,x-n,y+n+d-1,x-n+1,y+n+d-1,x-n);

	    /* North (east side) */
            update_view_aux(y-n-d,x+n,y-n-d+1,x+n-1,y-n-d+1,x+n);
	    
	    /* North (west side) */
            update_view_aux(y-n-d,x-n,y-n-d+1,x-n+1,y-n-d+1,x-n);

	    /* East (south side) */
            update_view_aux(y+n,x+n+d,y+n-1,x+n+d-1,y+n,x+n+d-1);
	    
	    /* East (north side) */
            update_view_aux(y-n,x+n+d,y-n+1,x+n+d-1,y-n,x+n+d-1);

	    /* West (south side) */
            update_view_aux(y+n,x-n-d,y+n-1,x-n-d+1,y+n,x-n-d+1);
	    
	    /* West (north side) */
            update_view_aux(y-n,x-n-d,y-n+1,x-n-d+1,y-n,x-n-d+1);
	}
    }    

#endif


    /*** Step 5 -- Remove the "old" view space ***/
    cave_seen_wipe_view();
}






/*
 * Aux function -- see below
 */
static void lite_room_aux(int y, int x, int n)
{
    cave_type *c_ptr = &cave[y][x];


    /* Avoid infinite recursion */
    if (c_ptr->info & CAVE_SEEN) return;
    
    /* Not part of a room */
    if (!(c_ptr->info & CAVE_LR)) return;

    /* Hack -- Refuse to recurse too "deep" */
    if (!n) return;


    /* Collect "processed" grids */
    cave_seen_add(y, x);


    /* Hack -- Dark to Lite */
    if (c_ptr->fval == NT_DARK_FLOOR) c_ptr->fval = NT_LITE_FLOOR;
    else if (c_ptr->fval == DARK_FLOOR) c_ptr->fval = LITE_FLOOR;


    /* Previously dark grids are "lit", waking up the monsters */
    if (!(c_ptr->info & CAVE_PL)) {    

	/* Check the monsters */
	monster_type *m_ptr = &m_list[c_ptr->m_idx];

	/* Perhaps wake up */
	if ((r_list[m_ptr->r_idx].cflags2 & MF2_INTELLIGENT) ||
	    (!(r_list[m_ptr->r_idx].cflags2 & MF2_MINDLESS) && (randint(3) == 1)) ||
	    (randint(10) == 1)) {

	    /* Wake up! */
	    m_ptr->csleep = 0;
	}
    }


    
    /* Stop at walls */
    if (!floor_grid(y, x)) return;


    /* Spread adjacent */
    lite_room_aux(y + 1, x, n - 1);
    lite_room_aux(y - 1, x, n - 1);
    lite_room_aux(y, x + 1, n - 1);
    lite_room_aux(y, x - 1, n - 1);

    /* Spread diagonal */
    lite_room_aux(y + 1, x + 1, n - 1);
    lite_room_aux(y - 1, x - 1, n - 1);
    lite_room_aux(y - 1, x + 1, n - 1);
    lite_room_aux(y + 1, x - 1, n - 1);
}

/*
 * Spread lite through a room.
 *
 * Dark grids are illuminated, and monsters notice it.
 * Monsters that are intelligent wake up all the time,
 * non-MF2_MINDLESS monsters wake up 1/3 the time,
 * and MF2_MINDLESS monsters wake up 1/10 the time -CWS
 *
 * Mega-Hack -- we use "torch lite" to mark processed grids,
 * so we must first forget the view to avoid updating them.
 */
void lite_room(int y, int x)
{
    /* Collect all the "room grids" */
    lite_room_aux(y, x, 128);

    /* Now, update each of those grids */
    cave_seen_room_lite();

    /* Hack -- check the view */
    check_view();
}


/*
 * Aux function -- see below
 *
 * Note -- "darkness" now "forgets" the room walls, which is
 * okay, because "lite" restores them, and because the "init"
 * function is only needed on unseen "lit" rooms, which this
 * will no longer be...
 */
static void unlite_room_aux(int y, int x, int n)
{
    cave_type *c_ptr = &cave[y][x];


    /* Avoid infinite recursion */
    if (c_ptr->info & CAVE_SEEN) return;
    
    /* Not part of a room */
    if (!(c_ptr->info & CAVE_LR)) return;

    /* Refuse to recurse too "deep" */
    if (!n) return;


    /* Collect "processed" grids */
    cave_seen_add(y, x);

    
    /* Turn Lite to Dark */
    if (c_ptr->fval == NT_LITE_FLOOR) c_ptr->fval = NT_DARK_FLOOR;
    else if (c_ptr->fval == LITE_FLOOR) c_ptr->fval = DARK_FLOOR;


    /* Stop at walls */
    if (!floor_grid(y,x)) return;


    /* Spread to adjacent grids */
    unlite_room_aux(y + 1, x, n - 1);
    unlite_room_aux(y - 1, x, n - 1);
    unlite_room_aux(y, x + 1, n - 1);
    unlite_room_aux(y, x - 1, n - 1);

    /* Spread to diagonal grids */
    unlite_room_aux(y + 1, x + 1, n - 1);
    unlite_room_aux(y - 1, x - 1, n - 1);
    unlite_room_aux(y - 1, x + 1, n - 1);
    unlite_room_aux(y + 1, x - 1, n - 1);
}




/*
 * Darken the room containing the given location
 *
 * Mega-Hack -- we use "torch lite" to mark processed grids,
 * so we must first forget the view to avoid updating them.
 */
void unlite_room(int y, int x)
{
    /* Darken the room */
    unlite_room_aux(y, x, 128);

    /* Now, update each of those grids */
    cave_seen_room_unlite();
}









/*
 * Does the player have "line of sight" to a grid
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
    
    /* Call the "los()" function */
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
bool player_can_see(int y, int x)
{
    /* Blind players see nothing */
    if (p_ptr->blind > 0) return (FALSE);

    /* Hack -- off screen cannot be seen */
    if (!panel_contains(y, x)) return (FALSE);

    /* Hack -- "torch-lite" implies "viewable" */
    if (cave[y][x].info & CAVE_TL) return (TRUE);

    /* No "torch-lite" and no "perma-lite" yields "dark" */
    if (!(cave[y][x].info & CAVE_PL)) return (FALSE);
    
    /* Check for "line of sight" */
    if (player_has_los(y, x)) return (TRUE);

    /* Default to unseen */
    return (FALSE);    
}


/*
 * Tests a spot for light or field mark status		-RAK-	
 */
int test_lite(int y, int x)
{
    register cave_type *c_ptr;
    c_ptr = &cave[y][x];
    if (c_ptr->info & CAVE_PL) return (TRUE);
    if (c_ptr->info & CAVE_TL) return (TRUE);
    if (c_ptr->info & CAVE_FM) return (TRUE);
    return (FALSE);
}


/*
 * Returns true if player cannot see himself.
 */
int no_lite(void)
{
    return (!player_can_see(char_row, char_col));
}







/*
 * Map the current panel (plus some) ala "detection"
 * This function is a big hack -- see "update_map()".
 */
void map_area()
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

	    /* If that grid holds a (legal) floor */
	    if (floor_grid(y,x)) {

		c_ptr = &cave[y][x];

		/* Field mark any visible object */
		if (c_ptr->i_idx) {
		    inven_type *i_ptr = &i_list[c_ptr->i_idx];
		    if ((i_ptr->tval >= TV_MIN_VISIBLE) &&
			(i_ptr->tval <= TV_MAX_VISIBLE)) {
			
			/* Memorize the object */
			c_ptr->info |= CAVE_FM;
		    }
		}

		/* Field-Mark the "true" walls */
		for (dy = -1; dy <= 1; dy++) {
		    for (dx = -1; dx <= 1; dx++) {
			if (!dx && !dy) continue;

			c_ptr = &cave[y+dy][x+dx];

			/* Perma-light the walls touching spaces */
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
    for (y = 0; y < cur_height; y++) {
	for (x = 0; x < cur_width; x++) {

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
		    }
		}
	    }
	}
    }

    /* Redraw the map */    
    prt_map();
}


/*
 * Hack -- Darken the dungeon.
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
	    if (c_ptr->fval == NT_LITE_FLOOR) c_ptr->fval = NT_DARK_FLOOR;
	    else if (c_ptr->fval == LITE_FLOOR) c_ptr->fval = DARK_FLOOR;
	}
    }

    /* Re-examine lite */
    update_lite();
    
    /* Redraw the map */    
    prt_map();
    
    
    /* Check the view */
    check_view();
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
static int8u mh_attr(void)
{

#ifdef USE_COLOR

    switch (randint(11)) {
	case 1: return (COLOR_RED);
	case 2: return (COLOR_BLUE);
	case 3: return (COLOR_GREEN);
	case 4: return (COLOR_YELLOW);
	case 5: return (COLOR_ORANGE);
	case 6: return (COLOR_VIOLET);
	case 7: return (COLOR_BROWN);
	case 8: return (COLOR_L_RED);
	case 9: return (COLOR_L_BLUE);
	case 10: return (COLOR_L_GREEN);
	case 11: return (COLOR_L_BROWN);
    }

#endif

    return (COLOR_WHITE);
}


/*
 * Hack -- Get a random "hallucination" attr
 */
static int8u image_attr(void)
{

#ifdef USE_COLOR

    switch (randint(15)) {
	case 1: return (COLOR_RED);
	case 2: return (COLOR_BLUE);
	case 3: return (COLOR_GREEN);
	case 4: return (COLOR_YELLOW);
	case 5: return (COLOR_ORANGE);
	case 6: return (COLOR_VIOLET);
	case 7: return (COLOR_BROWN);
	case 8: return (COLOR_L_RED);
	case 9: return (COLOR_L_BLUE);
	case 10: return (COLOR_L_GREEN);
	case 11: return (COLOR_L_BROWN);
	case 12: return (COLOR_WHITE);
	case 13: return (COLOR_GRAY);
	case 14: return (COLOR_L_GRAY);
	case 15: return (COLOR_D_GRAY);
    }

#endif

    return (COLOR_WHITE);
}



/*
 * Attribute for a item in the inventory
 */
byte inven_attr_by_tval(inven_type *i_ptr)
{

#ifdef USE_COLOR

    /* Run-time mono */
    if (!use_color) return (COLOR_WHITE);

    /* Examine the object */
    switch (i_ptr->tval) {
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
	return (COLOR_WHITE);
      case TV_CHEST:
	return (COLOR_GRAY);
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
	return (COLOR_L_BROWN);
      case TV_LITE:
	return (COLOR_YELLOW);
      case TV_SPIKE:
	return (COLOR_GRAY);
      case TV_BOW:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
	return (COLOR_L_GRAY);  /* weapons */
      case TV_DIGGING:
	return (COLOR_GRAY);
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_HELM:
      case TV_SHIELD:
      case TV_CLOAK:
	return (COLOR_L_BROWN);
      case TV_SOFT_ARMOR:
      case TV_HARD_ARMOR:
      case TV_DRAG_ARMOR:
	return (COLOR_GRAY);
      case TV_AMULET:
	return (COLOR_ORANGE);
      case TV_RING:
	return (COLOR_RED);
      case TV_STAFF:
	return (COLOR_L_BROWN);
      case TV_WAND:
	return (COLOR_L_GREEN);
      case TV_ROD:
	return (COLOR_L_GRAY);
      case TV_SCROLL:
	return (COLOR_WHITE);
      case TV_POTION:
	return (COLOR_L_BLUE);
      case TV_FLASK:
	return (COLOR_YELLOW);
      case TV_FOOD:
	return (COLOR_L_BROWN);
      case TV_MAGIC_BOOK:
	return (COLOR_ORANGE);
      case TV_PRAYER_BOOK:
	return (COLOR_L_GREEN);
    }

#endif

    /* No colors */
    return (COLOR_WHITE);
}





/* 
 * Return a color to use for the bolt/ball spells
 */
int8u spell_color(int type)
{

#ifdef USE_COLOR

    if (!use_color) return (COLOR_WHITE);

    switch (type) {

	case GF_MISSILE:
		return (mh_attr());		/* multihued */
	case GF_ELEC:
		return (COLOR_YELLOW);
	case GF_POIS:
		return (COLOR_GREEN);
	case GF_ACID:
		return (COLOR_GRAY);
	case GF_COLD:
		return (COLOR_L_BLUE);
	case GF_FIRE:
		return (COLOR_RED);
	case GF_HOLY_ORB:
		return (COLOR_D_GRAY);
	case GF_ARROW:
		return (COLOR_L_BROWN);
	case GF_PLASMA:
		return (COLOR_RED);
	case GF_NETHER:
		return (COLOR_VIOLET);
	case GF_WATER:
		return (COLOR_BLUE);
	case GF_CHAOS:
		return (mh_attr());		/* multihued */
	case GF_SHARDS:
		return (COLOR_L_BROWN);
	case GF_SOUND:
		return (COLOR_ORANGE);
	case GF_CONFUSION:
		return (mh_attr());		/* multihued */
	case GF_DISENCHANT:
		return (COLOR_VIOLET);
	case GF_NEXUS:
		return (COLOR_L_RED);
	case GF_FORCE:
		return (COLOR_WHITE);
	case GF_INERTIA:
		return (COLOR_L_GRAY);
	case GF_LITE_WEAK:
	case GF_LITE:
		return (COLOR_YELLOW);
	case GF_DARK_WEAK:
	case GF_DARK:
		return (COLOR_D_GRAY);
	case GF_TIME:
		return (COLOR_L_BLUE);
	case GF_GRAVITY:
		return (COLOR_GRAY);
	case GF_MANA:
		return (COLOR_L_RED);
	case GF_METEOR:
		return (COLOR_ORANGE);
	case GF_ICE:
		return (COLOR_L_BLUE);
    }

#endif

    /* Standard "color" */
    return (COLOR_WHITE);
}





/*
 * Minor Hack -- color character constructor from (Color, Char)
 */

#define CC(a,b) ((((int16u)(a))<<8) | (int16u)(b))




/*
 * Originally set highlight bit to display mineral veins
 * but that is not portable, now use the percent sign instead.
 *
 * Note the "fake" objects such as "OBJ_FLOOR" and "OBJ_GRANITE".
 *
 * Colors (used for floors, walls, quartz):
 *   Defaults to "white"
 *   Option "view_yellow_lite" draws "torch radius" in yellow
 *   Option "view_require_los" draws "hidden grids" dimmer.
 *   Option "view_run_quickly" cancels both those modes.
 *
 * Note that "view_require_los" does not really work unless either
 * "view_pre_compute" is set, or the player hits "Redraw" a lot.
 *
 * Warning: This function may ONLY be called from "map_info()".
 *
 * Need to handle "MULTI_HUED" property better.  Probably best to
 * use another flag on "wearable objects".  If we can find one.
 */
static int16u c_loc_symbol(int y, int x)
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
	    return CC(c_ptr->mattr,c_ptr->mchar);
#endif

	    /* Blank space */
	    return CC(COLOR_WHITE, ' ');
	}
    }
    

#ifdef USE_COLOR

    /* XXX Note -- view_yellow_fast and view_bright_fast suck */
    
    /* Option -- Draw the "torch-radius" in yellow */
    if (view_yellow_lite && (!find_flag || !view_yellow_fast) &&
        (c_ptr->info & CAVE_TL)) yellow = TRUE;

    /* Option -- Dim unseen grids in certain situations */
    else if (view_pre_compute && view_bright_lite &&
             (!(c_ptr->info & CAVE_VIEW))) darken = TRUE;

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
    if (yellow) mattr = COLOR_YELLOW;
    else if (darken) mattr = COLOR_GRAY;
    if (!use_color) mattr = COLOR_WHITE;
#else
    mattr = COLOR_WHITE;
#endif

    /* Return the result */
    return CC(mattr,mchar);
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
void map_info(int y, int x, int *m, byte *a, char *c)
{
    register int16u tmp;
    register int ma, mc;
    register cave_type *c_ptr;    


    /* Default to NOT "multihued" */
    (*m) = 0;

    /* Default to "white" */
    (*a) = COLOR_WHITE;
    
    /* Default to "space" */
    (*c) = ' ';
    

    /* Off the map -- invisible */
    if (!in_bounds2(y, x)) return;
    
    
    /* Get the cave */
    c_ptr = &cave[y][x];
    
    /* Hack -- player is "visible" unless "running blind" */
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

    /* Blind people see nothing (except themselves) */
    if (p_ptr->blind > 0) return;
    
    /* Hallucination kicks in occasionally */
    if ((p_ptr->image > 0) && (randint(12) == 1)) {
	(*c) = image_char();
	(*a) = image_attr();
	return;
    }


    /* Call "c_loc_symbol()" above */
    tmp = c_loc_symbol(y, x);
    mc = (tmp & 0xFF);
    ma = (tmp >> 8);


    /* Normal, visible monsters "block" the object they are on */
    if ((c_ptr->m_idx > 1) && (m_list[c_ptr->m_idx].ml)) {

	/* Get the monster, and race */
	monster_type *m_ptr = &m_list[c_ptr->m_idx];
	monster_race *r_ptr = &r_list[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	/* Use the given attribute unless "clear" */
	if (!(r_ptr->cflags1 & MF1_ATTR_CLEAR)) {
	    ma = l_ptr->l_attr;
	    if (!ma) ma = r_ptr->r_attr;
	    if (r_ptr->cflags1 & MF1_ATTR_MULTI) {
		(*m) = 1;
	        ma = mh_attr();
	    }
	}
	
	/* Use the given symbol unless "clear" */
	if (!(r_ptr->cflags1 & MF1_CHAR_CLEAR)) {
	    mc = l_ptr->l_char;
	    if (!mc) mc = r_ptr->r_char;
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
static int8u mh_a[MH_MAX];
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

    /* Flush */
    Term_fresh();

    /* Scan the multi-hued set */
    for (i = 0; i<mh_n; ++i)
    {
	int x = mh_x[i];
	int y = mh_y[i];
	int8u a = mh_a[i];
	char c = mh_c[i];

	int8u sa;
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

    /* Flush */
    Term_fresh();
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
    if (!use_color) a = COLOR_WHITE;

    /* Display it */
    Term_draw(x, y, a, c);
}

#else /* USE_MULTIHUED */

void mh_forget(void) {}

void mh_cycle(void) {}

/*
 */
void mh_print(char c, int8u a, int m, int y, int x)
{
    /* Run-time color choosing */
    if (!use_color) a = COLOR_WHITE;

    /* Simply place the character using the attribute */
    Term_draw(x, y, a, c);
}

#endif /* USE_MULTIHUED */

#else /* USE_COLOR */

void mh_forget(void) {}

void mh_cycle(void) {}

void mh_print(char c, int8u a, int m, int y, int x)
{
    /* Simply place the character using no attributes */
    Term_draw(x, y, COLOR_WHITE, c);
}

#endif /* USE_COLOR */



/*
 * Simply call "mh_print" (above),
 * but account for "panel-relative" in location
 */
void mh_print_rel(char c, byte a, int m, int y, int x)
{
    mh_print(c, a, m, y-panel_row_prt, x-panel_col_prt);
}





/*
 * Allow the player to "take notes" on the terrain
 * This routine is only called by "lite_spot()"
 *
 * This is the ONLY way a player can "remember" things that he cannot
 * currently "see" (at least until "NEW_MAPS" is compiled in).  And even
 * then, this will be the routine that builds the "map".  Conveniently,
 * only one function (lite_spot) updates the screen, and it always calls
 * us first, to take notes on the terrain.
 *
 * Note that we will NOT take notes on "unknown" grids.
 */
static void update_map(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Already marked */
    if (c_ptr->info & CAVE_FM) return;

    /* No ilumination */
    if (!(c_ptr->info & CAVE_PL) && !(c_ptr->info & CAVE_TL)) return;

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

    /* Always memorize walls */
    if (c_ptr->fval >= MIN_WALL) {
	c_ptr->info |= CAVE_FM;
	return;
    }

    /* Always memorize certain landmarks */
    if (c_ptr->i_idx &&
	(i_list[c_ptr->i_idx].tval >= TV_MIN_VISIBLE) &&
	(i_list[c_ptr->i_idx].tval <= TV_MAX_VISIBLE)) {
	c_ptr->info |= CAVE_FM;
	return;
    }
}



/*
 * Redraw (on the screen) a given MAP location
 */
void lite_spot(int y, int x)
{
    /* Update the map */
    update_map(y, x);

    /* Redraw if on screen */
    if (panel_contains(y, x)) {

	int m;
	int8u a;
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
 * Prints the map of the dungeon (speed this up!)
 * Hack -- Handles MULTIHUED objects
 */
void prt_map(void)
{
    register int x, y;

    int m;
    int8u a;
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
 */
void screen_map(void)
{
    register int i, j, x, y;

    register int8u ta;
    register char tc;

    int m, okay;

    char mc[MAP_HGT + 2][MAP_WID + 2];
    char ma[MAP_HGT + 2][MAP_WID + 2];

    int myrow = -1, mycol = -1;

    static int prioritized = 0;
    static int   priority[256];



    /* Set up the priority table */
    if (!prioritized)
    {
	/* Everything defaults to zero */
	for (i = 0; i < 256; i++) priority[i] = 0;

	/* A few things are special */    
	priority['<'] = 5;
	priority['>'] = 5;
	priority['@'] = 10;
	priority['#'] = (-5);
	priority['.'] = (-10);
	priority['x'] = (-1);
	priority['\''] = (-3);
	priority[' '] = (-15);

	/* Only do it once */
	prioritized = 1;
    }


    /* Clear the chars and attributes */
    for (y = 0; y < MAP_HGT+2; ++y) {
	for (x = 0; x < MAP_WID+2; ++x) {
	    ma[y][x] = COLOR_WHITE;
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
	    if (tc == '@') {
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
	    if (!use_color) ta = COLOR_WHITE;
#else
	    ta = COLOR_WHITE;
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





/* I may have written the town level code, but I'm not exactly	 */
/* proud of it.	 Adding the stores required some real slucky	 */
/* hooks which I have not had time to re-think.		 -RAK-	 */








/*
 * Hack -- count grid updates
 */
static int check_view_num = 0;


/*
 * This function "initializes" a lit room when first "entered".
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
 * seen can be "pre-memorized" when he enters that room.  Note that
 * if "view_pre_compute" is TRUE, the rooms could be pre-initialized
 * to "perma-lit", and this function would be un-necessary.  Note
 * that if "view_pre_compute" is FALSE, and "view_perma_grids" is
 * also FALSE, then all sorts of weird things may happen.  Actually,
 * it appears as though the only effect will be to NOT let the player
 * memorize the "floors" of perma-lit rooms.
 *
 * This is a purely "visual" thing, to prevent "room walls" from
 * being visible before they are seen.  It is also a big hack.
 *
 * The "first time" a room is "lit" some important stuff happens.
 *
 * Note that this function is DIFFERENT from "lite_a_dark_room()".
 * This function is a hack used to "initialize" rooms when seen for
 * the first time.
 *
 * The big "problem" is that we use "pl" to say if the player "knows"
 * about a wall.  We should probably be using "fm" instead.  Then we
 * can use "pl" to mean "the wall is illuminated" and "fm" to mean
 * "the player remembers the wall".
 *
 * This function gives a player "permanent knowledge" of a room when
 * it is first "entered" (see "check_view()").  This includes permanent
 * knowledge about the "floor" of the room.  In addition, it "displays"
 * the room, even if some of the grids are not "visible" to the player.
 *
 * Note that "wiz_lite()" bypasses this function entirely, but it is
 * still true that this function is activated when the various "pre-lit"
 * rooms are "entered".
 *
 * We are only called when the player "notices" that he is inside
 * (or possibly next to) a lit room that he has never seen before.
 */
static void check_view_aux(int y, int x)
{
    cave_type *c_ptr;

    /* Get the grid */
    c_ptr = &cave[y][x];


    /* Only initialize something once */
    if (c_ptr->info & CAVE_INIT) return;

    /* Only initialize "rooms" */
    if (!(c_ptr->info & CAVE_LR)) return;


    /* Do not initialize "Dark Rooms" */
    if (c_ptr->fval == DARK_FLOOR) return;
    if (c_ptr->fval == NT_DARK_FLOOR) return;


    /* Count the grids */
    check_view_num++;
    
    /* This grid has been initialized */
    c_ptr->info |= CAVE_INIT;
    
    /* Initialize the grid as "perma-lit" */
    c_ptr->info |= CAVE_PL;


    /* Redraw */
    lite_spot(y, x);


    /* Stop at walls */
    if (!floor_grid(y, x)) return;


    /* Spread adjacent */
    check_view_aux(y + 1, x);
    check_view_aux(y - 1, x);
    check_view_aux(y, x + 1);
    check_view_aux(y, x - 1);

    /* Spread diagonal */
    check_view_aux(y + 1, x + 1);
    check_view_aux(y - 1, x - 1);
    check_view_aux(y - 1, x + 1);
    check_view_aux(y + 1, x - 1);
}



/*
 * The new "check_view()" routine ONLY checks for certain "inconsistancies" on
 * the map.  In particular, "uninitialized" lit rooms get "initialized".
 *
 * An interesting thing to note is that to make rooms light up on
 * entry, we must also have "beam of light" and "call light" light
 * up rooms (before we enter them).  Otherwise, with the current method,
 * these rooms will not lite up when you DO enter them.
 */
void check_view()
{
    register int        y, x;


    /* All done if blind */
    if (p_ptr->blind) return;
    
    
    /* Hack -- "initialize" adjacent lite rooms */
    for (y = (char_row - 1); y <= (char_row + 1); y++) {
	for (x = (char_col - 1); x <= (char_col + 1); x++) {
	    check_view_aux(y, x);
	}
    }

    /* Hack -- extra "lite" requires monster redraw */
    if (check_view_num) {

	/* Clear the count */
        check_view_num = 0;

	/* Redraw everything */
	update_monsters();
    }
}


