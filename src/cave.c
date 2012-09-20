/* File: cave.c */

/* Purpose: low level dungeon routines -BEN- */

#include "angband.h"




/*
 * Approximate Distance between two points.
 *
 * When either the X or Y component dwarfs the other component,
 * this function is almost perfect, and otherwise, it tends to
 * over-estimate about one grid per fifteen grids of distance.
 *
 * Algorithm: hypot(dy,dx) = max(dy,dx) + min(dy,dx) / 2
 */
int distance(int y1, int x1, int y2, int x2)
{
    int dy, dx, d;

    /* Find the absolute y/x distance components */
    dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
    dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);

    /* Hack -- approximate the distance */
    d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

    /* Return the distance */
    return (d);
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
 *
 * Because this function uses (short) ints for all calculations, overflow may
 * occur if d_x and d_y exceed 90.
 *
 * Also note that this function and the "move towards target" code do NOT
 * share the same properties.  Thus, you can see someone, target them, and
 * then fire a bolt at them, but the bolt may hit a wall, not them.  However,
 * by clever choice of target locations, you can sometimes throw a "curve".
 *
 * Use the "projectable()" routine to test "spell/missile line of sight".
 *
 * Use the "update_view()" function to determine player line-of-sight.
 */
bool los(int y1, int x1, int y2, int x2)
{
    int p_x, p_y, d_x, d_y, a_x, a_y;


    /* Extract the offset */
    d_y = y2 - y1;
    d_x = x2 - x1;

    /* Extract the absolute offset */
    a_y = ABS(d_y);
    a_x = ABS(d_x);


    /* Handle adjacent (or identical) grids */
    if ((a_x < 2) && (a_y < 2)) return (TRUE);


    /* Paranoia -- require "safe" origin */
    /* if (!in_bounds(y1, x1)) return (FALSE); */


    /* Directly South/North */
    if (!d_x) {

        int p_y;

        /* South -- check for walls */
        if (d_y > 0) {
            for (p_y = y1 + 1; p_y < y2; p_y++) {
                if (!floor_grid_bold(p_y,x1)) return FALSE;
            }
        }

        /* North -- check for walls */
        else {
            for (p_y = y1 - 1; p_y > y2; p_y--) {
                if (!floor_grid_bold(p_y,x1)) return FALSE;
            }
        }

        /* Assume los */
        return TRUE;
    }

    /* Directly East/West */
    if (!d_y) {

        int p_x;

        /* East -- check for walls */
        if (d_x > 0) {
            for (p_x = x1 + 1; p_x < x2; p_x++) {
                if (!floor_grid_bold(y1,p_x)) return FALSE;
            }
        }

        /* West -- check for walls */
        else {
            for (p_x = x1 - 1; p_x > x2; p_x--) {
                if (!floor_grid_bold(y1,p_x)) return FALSE;
            }
        }

        /* Assume los */
        return TRUE;
    }


    /* Handle Knightlike shapes -CWS */
    if (a_x == 1) {
        if (d_y == 2) {
            if (floor_grid_bold(y1 + 1, x1)) return TRUE;
        }
        else if (d_y == (-2)) {
            if (floor_grid_bold(y1 - 1, x1)) return TRUE;
        }
    }
    else if (a_y == 1) {
        if (d_x == 2) {
            if (floor_grid_bold(y1, x1 + 1)) return TRUE;
        }
        else if (d_x == (-2)) {
            if (floor_grid_bold(y1, x1 - 1)) return TRUE;
        }
    }


/*
 * Now, we've eliminated all the degenerate cases. In the computations below,
 * dy (or dx) and m are multiplied by a scale factor, scale = abs(d_x *
 * d_y * 2), so that we can use integer arithmetic.
 */

    {
        int        scale,	/* a scale factor		 */
                            scale2;	/* above scale factor / 2	 */

        int		    xSign,	/* sign of d_x		 */
                            ySign,	/* sign of d_y		 */
                            m;		/* slope or 1/slope of LOS	 */

        scale2 = (a_x * a_y);
        scale = scale2 << 1;	/* (scale2 * 2) */

        xSign = (d_x < 0) ? -1 : 1;
        ySign = (d_y < 0) ? -1 : 1;


        /* Travel from one end of the line to the other, */
        /* oriented along the longer axis. */

        if (a_x >= a_y) {

            int        dy;  /* "fractional" y position	 */

        /*
         * We start at the border between the first and second tiles, where
         * the y offset = .5 * slope.  Remember the scale factor.  We have:
         *
         * m = d_y / d_x * 2 * (d_y * d_x) = 2 * d_y * d_y.
         */

            dy = a_y * a_y;
            m = dy << 1;	/* (dy * 2) */
            p_x = x1 + xSign;

            /* Consider the special case where slope == 1. */
            if (dy == scale2) {
                p_y = y1 + ySign;
                dy -= scale;
            }
            else {
                p_y = y1;
            }

            /* Note (below) the case (dy == scale2), where */
            /* the LOS exactly meets the corner of a tile. */
            while (x2 - p_x) {
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

            int        dx;	/* "fractional" x position	 */

            dx = a_x * a_x;
            m = dx << 1;	/* (dx * 2) */

            p_y = y1 + ySign;
            if (dx == scale2) {
                p_x = x1 + xSign;
                dx -= scale;
            }
            else {
                p_x = x1;
            }

            /* Note (below) the case (dx == scale2), where */
            /* the LOS exactly meets the corner of a tile. */
            while (y2 - p_y) {
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
 * The new "VIEW" code maintains a "current viewable space", which
 * is used by the "player_has_los_bold()" macro to determine if the
 * player has line of sight to a grid.
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
 * without ever having to check "los()" from the player.  This results in
 * such a speedup over the old method that the old code has been removed.
 *
 * I wouldn't be surprised if slight modifications could prepare some field
 * in the cave array to indicate which grids have "los()" on the player...
 */




/*
 * Can the player "see" the given grid in detail?
 *
 * He must have vision, illumination, and line of sight.
 *
 * Note -- "CAVE_LITE" is only set if the "torch" has "los()".
 * So, given "CAVE_LITE", we know that the grid is "fully visible".
 *
 * Note that "CAVE_GLOW" makes little sense for a wall, since it would mean
 * that a wall is visible from any direction.  That would be odd.  Except
 * under wizard light, which might make sense.  Thus, for walls, we require
 * not only that they be "CAVE_GLOW", but also, that they be adjacent to a
 * grid which is not only "CAVE_GLOW", but which is a non-wall, and which is
 * in line of sight of the player.
 *
 * This extra check is expensive, but it provides a more "correct" semantics.
 *
 * Note that we should not run this check on walls which are "outer walls" of
 * the dungeon, or we will induce a memory fault, but actually verifying all
 * of the locations would be extremely expensive.
 *
 * Thus, to speed up the function, we assume that all "perma-walls" which are
 * "CAVE_GLOW" are "illuminated" from all sides.  This is correct for all cases
 * except "vaults" and the "buildings" in town.  But the town is a hack anyway,
 * and the player has more important things on his mind when he is attacking a
 * monster vault.  It is annoying, but an extremely important optimization.
 *
 * We could check the four adjacent neighbors instead of all eight, but this
 * would cause the "corners" of illuminated rooms to appear "dark".
 */
bool player_can_see_bold(int y, int x)
{
    int i;

    cave_type *c_ptr;
    
    /* Blind players see nothing */
    if (p_ptr->blind) return (FALSE);

    /* Access the cave grid */
    c_ptr = &cave[y][x];
    
    /* Note that "torch-lite" yields "illumination" */
    if (c_ptr->feat & CAVE_LITE) return (TRUE);

    /* Require line of sight to the grid */
    if (!player_has_los_bold(y, x)) return (FALSE);

    /* Require "perma-lite" of the grid */
    if (!(c_ptr->feat & CAVE_GLOW)) return (FALSE);

    /* Hack -- allow "translucent" walls */
    if (optimize_display) return (TRUE);

    /* Assume perma-lit viewable floors are illuminated */
    if (floor_grid_bold(y, x)) return (TRUE);

    /* Mega-Hack -- Prevent memory faults (see above) */
    if ((c_ptr->feat & 0x3F) == 0x3F) return (TRUE);

    /* Hack -- verify walls */
    for (i = 0; i < 8; i++) {

        /* Extract adjacent (legal) location */
        int yy = y + ddy_ddd[i];
        int xx = x + ddx_ddd[i];

        /* Check for adjacent perma-lit viewable floor */
        if ((floor_grid_bold(yy, xx)) &&
            (player_has_los_bold(yy, xx)) &&
            (cave[yy][xx].feat & CAVE_GLOW)) {

            /* Assume the wall is really illuminated */
            return (TRUE);
        }
    }

    /* Assume not visible */
    return (FALSE);
}



/*
 * Returns true if the player's grid is dark
 */
bool no_lite(void)
{
    return (!player_can_see_bold(py, px));
}









/*
 * Get a legal "multi-hued" color
 */
static byte mh_attr(void)
{
    /* Anything but black */
    switch (randint(15)) {
        case 1: return (TERM_WHITE);
        case 2: return (TERM_SLATE);
        case 3: return (TERM_ORANGE);
        case 4: return (TERM_RED);
        case 5: return (TERM_GREEN);
        case 6: return (TERM_BLUE);
        case 7: return (TERM_UMBER);
        case 8: return (TERM_L_DARK);
        case 9: return (TERM_L_WHITE);
        case 10: return (TERM_VIOLET);
        case 11: return (TERM_YELLOW);
        case 12: return (TERM_L_RED);
        case 13: return (TERM_L_GREEN);
        case 14: return (TERM_L_BLUE);
        case 15: return (TERM_L_UMBER);
    }

    /* Assume white */
    return (TERM_WHITE);
}


/*
 * XXX XXX XXX Mega-Hack -- Hallucinatory monster
 */
static void image_monster(byte *ap, char *cp)
{
    int i;
    
    /* Hack -- Choose a monster */
    for (i = 0; i < 100; i++) {

        /* Pick a random race */
        int r = get_mon_num(dun_level);

        /* Acquire the race */
        monster_race *r_ptr = &r_info[r];

        /* Skip "bizarre" monster races */
        if (r_ptr->flags1 & RF1_ATTR_CLEAR) continue;
        if (r_ptr->flags1 & RF1_CHAR_CLEAR) continue;
        if (r_ptr->flags1 & RF1_ATTR_MULTI) continue;
        if (r_ptr->flags1 & RF1_CHAR_MULTI) continue;

        /* Use that monster */
        (*ap) = r_ptr->l_attr;
        (*cp) = r_ptr->l_char;

        /* Stop */
        return;
    }
}


/*
 * XXX XXX XXX Mega-Hack -- Hallucinatory object
 */
static void image_object(byte *ap, char *cp)
{
    int i;
    
    /* Hack -- Choose an object */
    for (i = 0; i < 100; i++) {

        /* Pick a random object */
        int k = get_obj_num(dun_level);

        /* Access the kind */
        inven_kind *k_ptr = &k_info[k];
                
        /* Use the attr/char */
        (*ap) = k_ptr->x_attr;
        (*cp) = k_ptr->x_char;

        /* Done */
        return;
    }
}


/*
 * Hack -- Random hallucination
 */
static void image_random(byte *ap, char *cp)
{
    /* Normally, assume monsters */
    if (rand_int(100) < 75) {
        image_monster(ap, cp);
    }

    /* Otherwise, assume objects */
    else {
        image_object(ap, cp);
    }
}



/*
 * Extract the attr/char to display at the given (legal) map location
 *
 * Note that monsters can have some "special" flags, including "ATTR_MULTI",
 * which means their color changes, and "ATTR_CLEAR", which means they take
 * the color of whatever is under them.  Technically, the "special" flags
 * "CHAR_CLEAR" and "CHAR_MULTI" are supposed to indicate that a monster
 * cannot be targetted, and looks strange when examined, but neither of
 * these fields are currently used.
 *
 * Currently, we do nothing with multi-hued objects.  We should probably
 * just add a flag to wearable object, or even to all objects, now that
 * everyone can use the same flags.  Then the "SHIMMER_OBJECT" code can
 * be used to request occasional "redraw" of those objects.
 *
 * Note the use of the "fake objects" to help draw the "player", "floor"
 * "walls", "mineral veins", "invisible traps", and "secret doors".
 *
 * Special colors (used for floors, walls, quartz):
 *   Option "view_yellow_lite" draws "torch radius" in yellow
 *   Option "view_bright_lite" draws "unseen grids" dimmer.
 *
 * Note the effects of hallucination.
 *
 * Note the use of the new "terrain feature" visual information
 *
 * XXX XXX XXX XXX XXX But note the hard coded index values
 *
 * Hack -- note that "loose rocks" are displayed as "glyph of warding",
 * but they should probably be converted into "poison pits" or some such
 */
static void map_info(int y, int x, byte *ap, char *cp)
{
    cave_type *c_ptr;

    byte tmp_a;
    char tmp_c;


    /* Default to "white" */
    (*ap) = TERM_WHITE;

    /* Default to "space" */
    (*cp) = ' ';


#ifdef USE_COLOR

    /* Mega-Hack -- Fake mono-chrome ignores the attr */
    if (!use_color) ap = &tmp_a;
    
#else

    /* Mega-Hack -- Fake mono-chrome ignores the attr */
    ap = &tmp_a;

#endif


    /* Get the cave */
    c_ptr = &cave[y][x];


    /* Handle player XXX XXX XXX */
    if ((y == py) && (x == px)) {
    
        /* Unless "optimizing" and "running" */
        if (!running || !optimize_running) {

            /* Get the player char */
            (*cp) = '@';

            /* Get the player attr */
            (*ap) = TERM_WHITE;

            /* Done */
            return;
        }
    }


    /* Handle monsters */
    if (c_ptr->m_idx) {

        monster_type *m_ptr = &m_list[c_ptr->m_idx];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Invisible monster */
        if (!m_ptr->ml) {

            /* Do nothing */
        }
        
        /* Hack -- hallucination */
        else if (p_ptr->image) {

            /* Hallucinatory monster */
            image_monster(ap, cp);
            
            /* Done */
            return;
        }

        /* Hack -- use_graphics */
        else if (use_graphics) {

            /* Normal symbol */
            (*cp) = r_ptr->l_char;

            /* Extract an attribute */
            (*ap) = r_ptr->l_attr;
            
            /* Done */
            return;
        }
        
        /* Hack -- Clear monster */
        else if (r_ptr->flags1 & RF1_ATTR_CLEAR) {

            /* Normal symbol */
            (*cp) = r_ptr->l_char;

            /* Mega-Hack -- bypass the floor char */
            cp = &tmp_c;
        }

        /* Hack -- Multi-hued monster */
        else if (r_ptr->flags1 & RF1_ATTR_MULTI) {
        
            /* Normal symbol */
            (*cp) = r_ptr->l_char;

            /* Multi-hued */
            (*ap) = mh_attr();
            
            /* Done */
            return;
        }

        /* Normal */
        else {

            /* Normal attr/char */
            (*cp) = r_ptr->l_char;
            (*ap) = r_ptr->l_attr;

            /* Done */
            return;
        }
    }


    /* Hack -- rare random hallucination (except outer walls) */
    if (p_ptr->image && (!rand_int(256)) && in_bounds(y,x)) {

        /* Hallucinate */
        image_random(ap, cp);

        /* Done */
        return;
    }


    /* Non-memorized grids */
    if (!(c_ptr->feat & CAVE_MARK)) {

        /* Non-illuminated grids are "unknown" */
        if (!player_can_see_bold(y, x)) {
        
            /* Done */
            return;
        }
    }

    /* Objects */
    if (c_ptr->i_idx) {

        /* Get the actual item, if any */
        inven_type *i_ptr = &i_list[c_ptr->i_idx];

        /* Hack -- Handle hallucination on "normal" objects */
        if (p_ptr->image) {
        
            /* Hallucinate */
            image_object(ap, cp);

            /* Done */
            return;
        }

        /* Extract an attr/char */
        (*cp) = inven_char(i_ptr);
        (*ap) = inven_attr(i_ptr);
    }

    /* Handle terrain */
    else {

        int f = (c_ptr->feat & 0x3F);

        /* Extract an attr/char */
        (*cp) = f_info[f].z_char;
        (*ap) = f_info[f].z_attr;

#ifdef USE_COLOR

        /* Allow "shadows" on some kinds of terrain (if allowed) */
        if (use_color && !use_graphics &&
            ((f <= 0x02) || (f >= 0x30))) {

            /* Option -- Draw "torch lite" in "yellow" */
            if (view_yellow_lite &&
                (c_ptr->feat & CAVE_LITE) && !p_ptr->blind) {

                /* Yellow from the torch */
                (*ap) = TERM_YELLOW;
            }

            /* Option -- Darken "non-viewable" grids */
            else if (view_bright_lite &&
                     (!player_can_see_bold(y, x))) {

                /* Gray (dim) from lack of light */
                (*ap) = TERM_SLATE;
            }
        }

#endif

    }
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



/*
 * Place an attr/char pair at the given map coordinate, if legal.
 */
void print_rel(char c, byte a, int y, int x)
{
    /* Only do "legal" locations */
    if (panel_contains(y, x)) {

#ifdef USE_COLOR

        /* Run-time color choosing */
        if (!use_color) a = TERM_WHITE;

        /* Draw the char using the attr */
        Term_draw(x-panel_col_prt, y-panel_row_prt, a, c);

#else

        /* Draw the char (always white) */
        Term_draw(x-panel_col_prt, y-panel_row_prt, TERM_WHITE, c);

#endif

    }
}





/*
 * Memorize the given spot if it is "interesting"
 *
 * This function should only be called on "legal" grids.
 *
 * Currently, this function is only called by "update_lite()",
 * "update_view()", and the two functions that illuminate cave
 * grids, "lite_room()" and "project_i()".
 *
 * XXX XXX XXX Note that two of the "options" have been removed,
 * view_wall_grids and view_xtra_grids, and now you are forced
 * to memorize all "terrain features" except floors and invisible
 * traps.
 */
void note_spot(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Only need to memorize a grid once */
    if (c_ptr->feat & CAVE_MARK) return;

    /* Hack -- Only memorize grids that can be seen */
    if (!player_can_see_bold(y, x)) return;

    /* Hack -- memorize all "objects" */
    if (c_ptr->i_idx) {
        c_ptr->feat |= CAVE_MARK;
        return;
    }
    
    /* Hack -- memorize all interesting "terrain features" */
    if ((c_ptr->feat & 0x3F) > 0x02) {
        c_ptr->feat |= CAVE_MARK;
        return;
    }

    /* Option -- memorize all perma-lit floors */
    if (view_perma_grids && (c_ptr->feat & CAVE_GLOW)) {
        c_ptr->feat |= CAVE_MARK;
        return;
    }

    /* Option -- memorize all torch-lit floors */
    if (view_torch_grids && (c_ptr->feat & CAVE_LITE)) {
        c_ptr->feat |= CAVE_MARK;
        return;
    }
}


/*
 * Redraw (on the screen) a given MAP location
 */
void lite_spot(int y, int x)
{
    /* Redraw if on screen */
    if (panel_contains(y, x)) {

        byte a;
        char c;

        /* Examine the contents of that grid */
        map_info(y, x, &a, &c);

#ifdef USE_COLOR

        /* Force mono-chrome */
        if (!use_color) a = TERM_WHITE;

        /* Efficiency -- immitate "print_rel()" */
        Term_draw(x-panel_col_prt, y-panel_row_prt, a, c);

#else

        /* Efficiency -- immitate "print_rel()" */
        Term_draw(x-panel_col_prt, y-panel_row_prt, TERM_WHITE, c);

#endif

    }
}




/*
 * Prints the map of the dungeon
 *
 * Note that, for efficiency, we contain an "optimized" version
 * of both "lite_spot()" and "print_rel()".
 */
void prt_map(void)
{
    int x, y;

    int okay;

    /* Hide the cursor */
    okay = Term_hide_cursor();

    /* Dump the map */
    for (y = panel_row_min; y <= panel_row_max; y++) {

        /* Scan the columns of row "y" */
        for (x = panel_col_min; x <= panel_col_max; x++) {

            byte a;
            char c;

            /* Determine what is there */
            map_info(y, x, &a, &c);

#ifdef USE_COLOR

            /* Force mono-chrome */
            if (!use_color) a = TERM_WHITE;

            /* Efficiency -- Redraw that grid of the map */
            Term_draw(x-panel_col_prt, y-panel_row_prt, a, c);

#else

            /* Efficiency -- Redraw that grid of the map */
            Term_draw(x-panel_col_prt, y-panel_row_prt, a, c);

#endif

        }
    }

    /* Show the cursor (if necessary) */
    if (!okay) Term_show_cursor();
}












/*
 * Some comments on the cave grid flags.  -BEN-
 *
 * The "update_view()" function maintains the "CAVE_VIEW" flag for each
 * grid and maintains an array of all "CAVE_VIEW" grids.
 *
 * This set of grids is the complete set of all grids within line of sight
 * of the player, allowing the "player_has_los()" macro to work very fast.
 *
 * The current algorithm uses the "CAVE_XTRA" flag as a temporary internal
 * flag to mark those grids which are not only in view, but which are also
 * "easily" in line of sight of the player.  This flag is always cleared
 * when we are done.
 *
 * The "update_lite()" function maintains the "CAVE_LITE" (torch lit) flag
 * for each grid, and maintains an array of all "CAVE_LITE" grids.  These
 * grids are the ones which are "illuminated" by the player's "torch".
 * Note that every "CAVE_LITE" grid is also a "CAVE_VIEW" grid, and in
 * fact, the player (unless blind) can always "see" all "CAVE_LITE" grids,
 * except (perhaps) if they are "off screen" (on a different panel).
 *
 * Any grid can be marked as "CAVE_GLOW" which means that the grid itself is
 * in some way permanently lit.  However, for the player to "see" anything
 * in the grid, as determined by "player_can_see()", the player must not be
 * blind, the grid must be marked as "CAVE_VIEW", and, in addition, "wall"
 * grids, even if marked as "perma lit", are only illuminated if they touch
 * a grid which is not a wall and is marked both "CAVE_GLOW" and "CAVE_VIEW".
 *
 * To simplify various things, a grid may be marked as "CAVE_MARK" which means
 * that even if the player cannot "see" the grid, he "knows" what is there.
 * This is used to "remember" walls/doors/stairs/floors/objects, and to
 * implement most of the "detection" spells.
 *
 * A grid may be marked as "CAVE_ROOM" which means that it is part of a "room",
 * and should be illuminated by "lite room" and "darkness" spells.  It may also
 * be marked as "CAVE_ICKY" which means it is part of a "vault".
 *
 * For various reasons, a grid may be temporarily marked as "CAVE_XTRA",
 * which is a temporary flag, currently used only by "update_view()".
 *
 * For various reasons, a grid may be marked as "CAVE_TEMP" and put into the
 * array of "CAVE_TEMP" grids.  This is a temporary flag/array, primarily used
 * for optimizing "update_view()" and "update_lite()", for spreading lite/dark
 * during "lite_room()" / "unlite_room()", and for calculating monster flow.
 *
 * The "view_perma_grids" allows the player to "memorize" every perma-lit grid
 * which is observed, and the "view_torch_grids" allows the player to "memorize"
 * every torch-lit grid.  The player will automatically memorize important walls,
 * doors, stairs, and other terrain features, as well as any "detected" grids.
 *
 * Note that the new "update_view()" method allows, among other things, a room
 * to be "partially" seen as the player approaches it, with a growing cone of
 * floor appearing as the player gets closer to the door.  Also, by not turning
 * on the "memorize perma-lit grids" option, the player will only "see" those
 * floor grids which are actually in line of sight.
 *
 * And my favorite "plus" is that you can now use a special option to draw the
 * walls/floors in the "viewable region" brightly (actually, to draw the *other*
 * grids dimly), providing a "pretty" effect as the player runs around.  This is
 * not recommended for slow machines, as it is one of the more abusive routines.
 */



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite(void)
{
    int i;

    /* Clear them all */
    for (i = 0; i < temp_n; i++) {

        int y = temp_y[i];
        int x = temp_x[i];

        cave_type *c_ptr = &cave[y][x];
        
        /* No longer in the array */
        c_ptr->feat &= ~CAVE_TEMP;

        /* Update only non-CAVE_GLOW grids */
        /* if (c_ptr->feat & CAVE_GLOW) continue; */

        /* Perma-Lite */
        c_ptr->feat |= CAVE_GLOW;

        /* Process affected monsters */
        if (c_ptr->m_idx) {

            monster_type	*m_ptr = &m_list[c_ptr->m_idx];

            monster_race	*r_ptr = &r_info[m_ptr->r_idx];

            /* Update the monster */
            update_mon(c_ptr->m_idx, FALSE);
            
            /* Sometimes monsters wake up */
            if (m_ptr->csleep &&
                (((r_ptr->flags2 & RF2_STUPID) && (rand_int(100) < 10)) ||
                 (rand_int(100) < 25) ||
                 (r_ptr->flags2 & RF2_SMART))) {

                /* Wake up! */
                m_ptr->csleep = 0;

                /* Notice the "waking up" */
                if (m_ptr->ml) {

                    char m_name[80];

                    /* Acquire the monster name */
                    monster_desc(m_name, m_ptr, 0);

                    /* Dump a message */
                    msg_format("%^s wakes up.", m_name);
                }
            }
        }

        /* Note */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite(void)
{
    int i;

    /* Clear them all */
    for (i = 0; i < temp_n; i++) {

        int y = temp_y[i];
        int x = temp_x[i];

        cave_type *c_ptr = &cave[y][x];

        /* No longer in the array */
        c_ptr->feat &= ~CAVE_TEMP;

        /* Darken the grid */
        c_ptr->feat &= ~CAVE_GLOW;

        /* XXX XXX XXX Hack -- Forget "boring" grids */
        if (((c_ptr->feat & 0x3F) <= 0x02) && !c_ptr->i_idx) {

            /* Forget the grid */
            c_ptr->feat &= ~CAVE_MARK;

            /* Notice */
            note_spot(y, x);
        }

        /* Process affected monsters */
        if (c_ptr->m_idx) {

            /* Update the monster */
            update_mon(c_ptr->m_idx, FALSE);
        }

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}






/*
 * Actually erase the entire "lite" array, redrawing every grid
 */
void forget_lite(void)
{
    int i, x, y;

    /* None to forget */
    if (!lite_n) return;

    /* Clear them all */
    for (i = 0; i < lite_n; i++) {

        y = lite_y[i];
        x = lite_x[i];
        
        /* Forget "LITE" flag */
        cave[y][x].feat &= ~CAVE_LITE;

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    lite_n = 0;
}


/*
 * Mark the given grid as "illuminated by torch lite"
 * Never call this routine when the "lite" array is "full".
 * This routine does a "lite_spot()" on grids for whom the
 * "CAVE_TEMP" flag is NOT set.
 *
 * We should probably do "wake_monster(y, x)" in some cases
 */
static void cave_lite(int y, int x)
{
    /* Already lit */
    if (cave[y][x].feat & CAVE_LITE) return;

    /* Set the flag */
    cave[y][x].feat |= CAVE_LITE;

    /* Add to queue */
    lite_y[lite_n] = y;
    lite_x[lite_n] = x;
    lite_n++;
}



/*
 * Update the set of grids "illuminated" by the player's lite.
 *
 * This routine may only work after "update_view()" has been called,
 * but since it is only called by "handle_stuff()", we are okay...
 *
 * Note that "blindness" does NOT affect "torch lite".  Be careful!
 *
 * We optimize most lites (all non-artifact lites) by using "obvious"
 * facts about the results of "small" lite radius.
 *
 * We optimize artifact lites (radius 3) by using a special "inline"
 * version of the "distance()" function.
 *
 * We will correctly handle "large" radius lites, though currently,
 * it is impossible for the player to have this much lite...
 *
 * We assume that "radius zero" lite is in fact no lite at all.
 *
 *     Torch     Lantern     Artifacts
 *     (etc)
 *                              ***
 *                 ***         *****
 *      ***       *****       *******
 *      *@*       **@**       ***@***
 *      ***       *****       *******
 *                 ***         *****
 *                              ***
 */
void update_lite(void)
{
    int i, x, y, min_x, max_x, min_y, max_y;


    /*** Special cases ***/

    /* Hack -- Player has no lite */
    if (p_ptr->cur_lite <= 0) {

        /* Forget the old lite */
        forget_lite();

        /* Hack -- Draw the player */
        lite_spot(py, px);

        /* All done */
        return;
    }


    /*** Save the old "lite" grids for later ***/

    /* Clear them all */
    for (i = 0; i < lite_n; i++) {

        y = lite_y[i];
        x = lite_x[i];

        /* Mark the grid as not "lite" */
        cave[y][x].feat &= ~CAVE_LITE;

        /* Mark the grid as "seen" */
        cave[y][x].feat |= CAVE_TEMP;

        /* Add it to the "seen" set */
        temp_y[temp_n] = y;
        temp_x[temp_n] = x;
        temp_n++;
    }

    /* None left */
    lite_n = 0;


    /*** Collect the new "lite" grids ***/

    /* Efficiency -- torch radius */
    if (p_ptr->cur_lite == 1) {

        /* Player grid */
        cave_lite(py, px);

        /* Adjacent to player grid */
        cave_lite(py+1, px);
        cave_lite(py-1, px);
        cave_lite(py, px+1);
        cave_lite(py, px-1);

        /* Diagonally adjacent to player grid */
        cave_lite(py+1, px+1);
        cave_lite(py+1, px-1);
        cave_lite(py-1, px+1);
        cave_lite(py-1, px-1);
    }

    /* Efficiency -- lantern radius */
    else if (p_ptr->cur_lite == 2) {

        /* Player grid */
        cave_lite(py, px);

        /* Adjacent to player grid */
        cave_lite(py+1, px);
        cave_lite(py-1, px);
        cave_lite(py, px+1);
        cave_lite(py, px-1);

        /* Diagonally adjacent to player grid */
        cave_lite(py+1, px+1);
        cave_lite(py+1, px-1);
        cave_lite(py-1, px+1);
        cave_lite(py-1, px-1);

        /* South of the player */
        if (floor_grid_bold(py+1, px)) {
            cave_lite(py+2, px);
            cave_lite(py+2, px+1);
            cave_lite(py+2, px-1);
        }

        /* North of the player */
        if (floor_grid_bold(py-1, px)) {
            cave_lite(py-2, px);
            cave_lite(py-2, px+1);
            cave_lite(py-2, px-1);
        }

        /* East of the player */
        if (floor_grid_bold(py, px+1)) {
            cave_lite(py, px+2);
            cave_lite(py+1, px+2);
            cave_lite(py-1, px+2);
        }

        /* West of the player */
        if (floor_grid_bold(py, px-1)) {
            cave_lite(py, px-2);
            cave_lite(py+1, px-2);
            cave_lite(py-1, px-2);
        }
    }

    /* Hack -- Efficiency -- artifact radius */
    else /* if (p_ptr->cur_lite == 3) */ {

        /* Extract maximal legal bounded light box */
        min_y = MAX(0, py-3);
        max_y = MIN(cur_hgt-1, py+3);
        min_x = MAX(0, px-3);
        max_x = MIN(cur_wid-1, px+3);

        /* Scan that box (the viewable, reachable, portions of it) */
        for (y = min_y; y <= max_y; y++) {
            for (x = min_x; x <= max_x; x++) {

                int dy = (py > y) ? (py - y) : (y - py);
                int dx = (px > x) ? (px - x) : (x - px);

                /* Viewable, nearby, grids get "torch lit" */
                if (player_has_los_bold(y, x) && (dx + dy <= 4)) {

                    /* This grid is "torch lit" */
                    cave_lite(y, x);
                }
            }
        }
    }

#if 0

    /* Unused -- Larger radius */
    else {

        /* Paranoia -- see "LITE_MAX" */
        if (p_ptr->cur_lite > 5) p_ptr->cur_lite = 5;

        /* Extract maximal legal bounded light box */
        min_y = MAX(0, (py - p_ptr->cur_lite));
        max_y = MIN(cur_hgt-1, (py + p_ptr->cur_lite));
        min_x = MAX(0, (px - p_ptr->cur_lite));
        max_x = MIN(cur_wid-1, (px + p_ptr->cur_lite));

        /* Scan that box (the viewable, reachable, portions of it) */
        for (y = min_y; y <= max_y; y++) {
            for (x = min_x; x <= max_x; x++) {

                /* Viewable, nearby, grids get "torch lit" */
                if (player_has_los_bold(y, x) &&
                    (distance(py, px, y, x) <= p_ptr->cur_lite)) {

                    /* This grid is "torch lit" */
                    cave_lite(y, x);
                }
            }
        }
    }

#endif


    /*** Complete the algorithm ***/

    /* Draw the new grids */
    for (i = 0; i < lite_n; i++) {

        y = lite_y[i];
        x = lite_x[i];

        /* Update fresh grids */
        if (cave[y][x].feat & CAVE_TEMP) continue;
        
        /* Note */
        note_spot(y, x);
        
        /* Redraw */
        lite_spot(y, x);
    }

    /* Clear them all */
    for (i = 0; i < temp_n; i++) {

        y = temp_y[i];
        x = temp_x[i];

        /* No longer in the array */
        cave[y][x].feat &= ~CAVE_TEMP;

        /* Update stale grids */
        if (cave[y][x].feat & CAVE_LITE) continue;
        
        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}







/*
 * Some comments on the "update_view()" algorithm...
 *
 * The algorithm is very fast, since it spreads "obvious" grids very quickly,
 * and only has to call "los()" on the borderline cases.  The major axes/diags
 * even terminate early when they hit walls.  I need to find a quick way
 * to "terminate" the other scans.
 *
 * Note that in the worst case (a big empty area with maybe 5% scattered walls),
 * each of the 1500 or so nearby grids is checked once, with most of them getting
 * an "instant" rating, and only a small portion requiring a call to "los()".
 * Note that the "town" provides a "pretty bad" case.
 *
 * In the "best" case (say, a normal stretch of corridor), the algorithm
 * makes one check for each viewable grid, and makes no calls to "los()".
 * So running in corridors is very fast, even if a monster pit is nearby.
 *
 * The only time that the algorithm appears to be "noticeably" too slow is
 * when running, and usually only in "town".  Note that resting, most normal
 * commands, and several forms of running, plus all commands executed near
 * large groups of monsters, are strictly more efficient with "update_view()"
 * that with the old "compute los() on demand" method.  So it is important to
 * optimize the "running" case.  One way to do this is to use one of the options
 * to reduce the view range when running.  This provides a gradual reduction of
 * the view range from 20 to 10 and then instantly back up to 20 when the player
 * stops running for whatever reason.  This is useful in town.  Actually, the
 * algorithm is pretty efficient, and may not need any more optimizations.
 */


/*
 * More comments on the update_view() algorithm...
 *
 * Note that we no longer have to do as many "los()" checks, since once the
 * "view" region has been built, very few things cause it to be "changed"
 * (player movement, and the opening/closing of doors, changes in wall status).
 * Note that door/wall changes are only relavant when the door/wall itself is
 * in the "view" region.
 *
 * The algorithm seems to only call "los()" from zero to ten times, usually only
 * when coming down a corridor into a room, or standing in a room, just misaligned
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
 * we prevent "erase + redraw" ineffiencies via the "seen" set.  These bytes are
 * also used by other routines, thus amortizing the cost.
 *
 * A similar thing is done for "forget_lite()" in which case the savings are
 * much less, but save us from doing bizarre maintenance checking.
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
 */



/*
 * Clear the viewable space
 */
void forget_view(void)
{
    int i;

    cave_type *c_ptr;

    /* None to forget */
    if (!view_n) return;

    /* Clear them all */
    for (i = 0; i < view_n; i++) {

        int y = view_y[i];
        int x = view_x[i];

        /* Access the grid */
        c_ptr = &cave[y][x];
        
        /* Forget that the grid is viewable */
        c_ptr->feat &= ~CAVE_VIEW;

        /* Update the screen */
        lite_spot(y, x);
    }

    /* None left */
    view_n = 0;
}



/*
 * XXX XXX XXX
 *
 * Hack -- Optimized version of "floor_grid_bold(Y,X)"
 */
#define floor_grid_hack(C) \
    (!((C)->feat & 0x20))

/*
 * XXX XXX XXX
 * Hack -- Optimized inline version of "cave_view()"
 */
#define cave_view_hack(C,Y,X) \
    if (!((C)->feat & CAVE_VIEW)) { \
        (C)->feat |= CAVE_VIEW; \
        view_y[view_n] = (Y); \
        view_x[view_n] = (X); \
        view_n++; \
    }

/*
 * Helper function for "update_view()" below
 *
 * We are checking the "viewability" of grid (y,x) by the player.
 *
 * This function assumes that (y,x) is legal (i.e. on the map).
 *
 * Grid (y1,x1) is on the "diagonal" between (py,px) and (y,x)
 * Grid (y2,x2) is "adjacent", also between (py,px) and (y,x).
 *
 * Note that we are using the "CAVE_XTRA" field for marking grids as
 * "easily viewable".  This bit is cleared at the end of "update_view()".
 *
 * This function adds (y,x) to the "viewable set" if necessary.
 *
 * This function now returns "TRUE" if vision is "blocked" by grid (y,x).
 */
static bool update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, z1, z2, wall;

    cave_type *c_ptr;

    cave_type *g1_c_ptr;
    cave_type *g2_c_ptr;
    

    /* Access the grids */
    g1_c_ptr = &cave[y1][x1];
    g2_c_ptr = &cave[y2][x2];


    /* Check for walls */
    f1 = (floor_grid_hack(g1_c_ptr));
    f2 = (floor_grid_hack(g2_c_ptr));

    /* Totally blocked by physical walls */
    if (!f1 && !f2) return (TRUE);
    

    /* Check for visibility */
    v1 = (f1 && (g1_c_ptr->feat & CAVE_VIEW));
    v2 = (f2 && (g2_c_ptr->feat & CAVE_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);


    /* Access the grid */
    c_ptr = &cave[y][x];


    /* Check for walls */
    wall = (!floor_grid_hack(c_ptr));


    /* Check the "ease" of visibility */
    z1 = (v1 && (g1_c_ptr->feat & CAVE_XTRA));
    z2 = (v2 && (g2_c_ptr->feat & CAVE_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (z1 && z2) {

        c_ptr->feat |= CAVE_XTRA;

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (z1) {

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2) {

        /* c_ptr->feat |= CAVE_XTRA; */

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Mega-Hack -- the "los()" function works poorly on walls */
    if (wall) {

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Hack -- check line of sight */
    if (los(py, px, y, x)) {

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Assume no line of sight. */
    return (TRUE);
}



/*
 * Calculate the viewable space
 *
 *  1: Process the player
 *  1a: The player is always (easily) viewable
 *  2: Process the diagonals
 *  2a: The diagonals are (easily) viewable up to the first wall
 *  2b: But never go more than 2/3 of the "full" distance
 *  3: Process the main axes
 *  3a: The main axes are (easily) viewable up to the first wall
 *  3b: But never go more than the "full" distance
 *  4: Process sequential "strips" in each of the eight octants
 *  4a: Each strip runs along the previous strip
 *  4b: The main axes are "previous" to the first strip
 *  4c: Process both "sides" of each "direction" of each strip
 *  4c1: Each side aborts as soon as possible
 *  4c2: Each side tells the next strip how far it has to check
 *
 * Note that the octant processing involves some pretty interesting
 * observations involving when a grid might possibly be viewable from
 * a given grid, and on the order in which the strips are processed.
 *
 * Note the use of the mathematical facts shown below, which derive
 * from the fact that (1 < sqrt(2) < 1.5), and that the length of the
 * hypotenuse of a right triangle is primarily determined by the length
 * of the longest side, when one side is small, and is strictly less
 * than one-and-a-half times as long as the longest side when both of
 * the sides are large.
 * 
 *   if (manhatten(dy,dx) < R) then (hypot(dy,dx) < R)
 *   if (manhatten(dy,dx) > R*3/2) then (hypot(dy,dx) > R)
 *
 *   hypot(dy,dx) is approximated by (dx+dy+MAX(dx,dy)) / 2
 *
 * These observations are important because the calculation of the actual
 * value of "hypot(dx,dy)" is extremely expensive, involving square roots,
 * while for small values (up to about 20 or so), the approximations above
 * are correct to within an error of at most one grid or so.
 *
 * Observe the use of "full" and "over" in the code below, and the use of
 * the specialized calculation involving "limit", all of which derive from
 * the observations given above.  Basically, we note that the "circle" of
 * view is completely contained in an "octagon" whose bounds are easy to
 * determine, and that only a few steps are needed to derive the actual
 * bounds of the circle given the bounds of the octagon.
 *
 * Note that by skipping all the grids in the corners of the octagon, we
 * place an upper limit on the number of grids in the field of view, given
 * that "full" is never more than 20.  Of the 1681 grids in the "square" of
 * view, only about 1475 of these are in the "octagon" of view, and even
 * fewer are in the "circle" of view, so 1500 or 1536 is more than enough
 * entries to completely contain the actual field of view.
 *
 * Note also the care taken to prevent "running off the map".  The use of
 * explicit checks on the "validity" of the "diagonal", and the fact that
 * the loops are never allowed to "leave" the map, lets "update_view_aux()"
 * use the optimized "floor_grid_bold()" macro, and to avoid the overhead
 * of multiple checks on the validity of grids.
 *
 * Note the "optimizations" involving the "se","sw","ne","nw","es","en",
 * "ws","wn" variables.  They work like this: While travelling down the
 * south-bound strip just to the east of the main south axis, as soon as
 * we get to a grid which does not "transmit" viewing, if all of the strips
 * preceding us (in this case, just the main axis) had terminated at or before
 * the same point, then we can stop, and reset the "max distance" to ourself.
 * So, each strip (named by major axis plus offset, thus "se" in this case)
 * maintains a "blockage" variable, initialized during the main axis step,
 * and checks it whenever a blockage is observed.  After processing each
 * strip as far as the previous strip told us to process, the next strip is
 * told not to go farther than the current strip's farthest viewable grid,
 * unless open space is still available.  This uses the "k" variable.
 *
 * Note the use of "inline" macros for efficiency.  The "floor_grid_hack()"
 * macro is a replacement for "floor_grid_bold()" which takes a pointer to
 * a cave grid instead of its location.  The "cave_view_hack()" macro is a
 * chunk of code which adds the given location to the "view" array if it
 * is not already there, using both the actual location and a pointer to
 * the cave grid.  See above.
 *
 * By the way, the purpose of this code is to reduce the dependancy on the
 * "los()" function which is slow, and, in some cases, not very accurate.
 * 
 * It is very possible that I am the only person who fully understands this
 * function, and for that I am truly sorry, but efficiency was very important
 * and the "simple" version of this function was just not fast enough.  I am
 * more than willing to replace this function with a simpler one, if it is
 * equally efficient, and especially willing if the new function happens to
 * derive "reverse-line-of-sight" at the same time, since currently monsters
 * just use an optimized hack of "you see me, so I see you", and then use the
 * actual "projectable()" function to check spell attacks.
 */
void update_view(void)
{
    int n, m, d, k, y, x;

    int se, sw, ne, nw, es, en, ws, wn;

    int limit, over, full;

    cave_type *c_ptr;


    /* Extract the "view" radius */
    full = p_ptr->cur_view;

    /* Extract the "octagon" limits */
    over = full * 3 / 2;


    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < view_n; n++) {

        y = view_y[n];
        x = view_x[n];

        /* Access the grid */
        c_ptr = &cave[y][x];
        
        /* Mark the grid as not in "view" */
        c_ptr->feat &= ~(CAVE_VIEW);

        /* Mark the grid as "seen" */
        c_ptr->feat |= CAVE_TEMP;

        /* Add it to the "seen" set */
        temp_y[temp_n] = y;
        temp_x[temp_n] = x;
        temp_n++;
    }

    /* Start over with the "view" array */
    view_n = 0;


    /*** Step 1 -- adjacent grids ***/

    /* Now start on the player */
    y = py;
    x = px;

    /* Access the grid */
    c_ptr = &cave[y][x];

    /* Assume the player grid is easily viewable */
    c_ptr->feat |= CAVE_XTRA;

    /* Assume the player grid is viewable */
    cave_view_hack(c_ptr, y, x);


    /*** Step 2 -- Major Diagonals ***/

    /* Hack -- Limit */
    limit = full * 2 / 3;

    /* Scan south-east */
    for (d = 1; d <= limit; d++) {
        c_ptr = &cave[y+d][x+d];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y+d, x+d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Scan south-west */
    for (d = 1; d <= limit; d++) {
        c_ptr = &cave[y+d][x-d];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y+d, x-d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Scan north-east */
    for (d = 1; d <= limit; d++) {
        c_ptr = &cave[y-d][x+d];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y-d, x+d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Scan north-west */
    for (d = 1; d <= limit; d++) {
        c_ptr = &cave[y-d][x-d];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y-d, x-d);
        if (!floor_grid_hack(c_ptr)) break;
    }


    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++) {
        c_ptr = &cave[y+d][x];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y+d, x);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++) {
        c_ptr = &cave[y-d][x];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y-d, x);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++) {
        c_ptr = &cave[y][x+d];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y, x+d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++) {
        c_ptr = &cave[y][x-d];
        c_ptr->feat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y, x-d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;


    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++) {

        int ypn, ymn, xpn, xmn;


        /* Acquire the "bounds" of the maximal circle */
        limit = over - n - n;
        if (limit > full - n) limit = full - n;	
        while ((limit + n + (n>>1)) > full) limit--;


        /* Access the four diagonal grids */
        ypn = y + n;
        ymn = y - n;
        xpn = x + n;
        xmn = x - n;


        /* South strip */
        if (ypn < cur_hgt-1) {

            /* Maximum distance */
            m = MIN(limit, (cur_hgt-1) - ypn);

            /* East side */
            if ((xpn <= cur_wid-1) && (n < se)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn+d,xpn,ypn+d-1,xpn-1,ypn+d-1,xpn)) {
                        if (n + d >= se) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                se = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < sw)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn+d,xmn,ypn+d-1,xmn+1,ypn+d-1,xmn)) {
                        if (n + d >= sw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                sw = k + 1;
            }
        }


        /* North strip */
        if (ymn > 0) {

            /* Maximum distance */
            m = MIN(limit, ymn);

            /* East side */
            if ((xpn <= cur_wid-1) && (n < ne)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn-d,xpn,ymn-d+1,xpn-1,ymn-d+1,xpn)) {
                        if (n + d >= ne) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ne = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < nw)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn-d,xmn,ymn-d+1,xmn+1,ymn-d+1,xmn)) {
                        if (n + d >= nw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                nw = k + 1;
            }
        }


        /* East strip */
        if (xpn < cur_wid-1) {

            /* Maximum distance */
            m = MIN(limit, (cur_wid-1) - xpn);

            /* South side */
            if ((ypn <= cur_hgt-1) && (n < es)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn,xpn+d,ypn-1,xpn+d-1,ypn,xpn+d-1)) {
                        if (n + d >= es) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                es = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < en)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn,xpn+d,ymn+1,xpn+d-1,ymn,xpn+d-1)) {
                        if (n + d >= en) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                en = k + 1;
            }
        }


        /* West strip */
        if (xmn > 0) {

            /* Maximum distance */
            m = MIN(limit, xmn);

            /* South side */
            if ((ypn <= cur_hgt-1) && (n < ws)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn,xmn-d,ypn-1,xmn-d+1,ypn,xmn-d+1)) {
                        if (n + d >= ws) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ws = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < wn)) {

                /* Scan */
                for (k = n, d = 1; d <= m; d++) {

                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn,xmn-d,ymn+1,xmn-d+1,ymn,xmn-d+1)) {
                        if (n + d >= wn) break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                wn = k + 1;
            }
        }
    }


    /*** Step 5 -- Complete the algorithm ***/

    /* Update all the new grids */
    for (n = 0; n < view_n; n++) {

        y = view_y[n];
        x = view_x[n];

        /* Access the grid */
        c_ptr = &cave[y][x];
        
        /* Clear the "CAVE_XTRA" flag */
        c_ptr->feat &= ~CAVE_XTRA;

        /* Update only newly viewed grids */
        if (c_ptr->feat & CAVE_TEMP) continue;
        
        /* Note */
        note_spot(y, x);
        
        /* Redraw */
        lite_spot(y, x);
    }

    /* Wipe the old grids, update as needed */
    for (n = 0; n < temp_n; n++) {

        y = temp_y[n];
        x = temp_x[n];

        /* Access the grid */
        c_ptr = &cave[y][x];
        
        /* No longer in the array */
        c_ptr->feat &= ~CAVE_TEMP;

        /* Update only non-viewable grids */
        if (c_ptr->feat & CAVE_VIEW) continue;
        
        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}






/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Avoid infinite recursion */
    if (c_ptr->feat & CAVE_TEMP) return;

    /* Do not "leave" the current room */
    if (!(c_ptr->feat & CAVE_ROOM)) return;

    /* Paranoia -- verify space */
    if (temp_n == TEMP_MAX) return;

    /* Mark the grid as "seen" */
    c_ptr->feat |= CAVE_TEMP;

    /* Add it to the "seen" set */
    temp_y[temp_n] = y;
    temp_x[temp_n] = x;
    temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* While grids are in the queue, add their neighbors */
    for (i = 0; i < temp_n; i++) {

        x = temp_x[i], y = temp_y[i];

        /* Walls get lit, but stop light */
        if (!floor_grid_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, lite them all up at once */
    cave_temp_room_lite();
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y1, int x1)
{
    int i, x, y;

    /* Add the initial grid */
    cave_temp_room_aux(y1, x1);

    /* Spread, breadth first */
    for (i = 0; i < temp_n; i++) {

        x = temp_x[i], y = temp_y[i];

        /* Walls get dark, but stop darkness */
        if (!floor_grid_bold(y, x)) continue;

        /* Spread adjacent */
        cave_temp_room_aux(y + 1, x);
        cave_temp_room_aux(y - 1, x);
        cave_temp_room_aux(y, x + 1);
        cave_temp_room_aux(y, x - 1);

        /* Spread diagonal */
        cave_temp_room_aux(y + 1, x + 1);
        cave_temp_room_aux(y - 1, x - 1);
        cave_temp_room_aux(y - 1, x + 1);
        cave_temp_room_aux(y + 1, x - 1);
    }

    /* Now, darken them all at once */
    cave_temp_room_unlite();
}





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
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

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
 * Take note of a reachable grid.  Assume grid is legal.
 */
static void update_flow_aux(int y, int x, int n)
{
    cave_type *c_ptr;
    
    int old_head = flow_head;


    /* Get the grid */
    c_ptr = &cave[y][x];
    
    /* Ignore "pre-stamped" entries */
    if (c_ptr->when == flow_n) return;

    /* Ignore "walls" and "rubble" */
    if ((c_ptr->feat & 0x3F) >= 0x31) return;

    /* Save the time-stamp */
    c_ptr->when = flow_n;

    /* Save the flow cost */
    c_ptr->cost = n;

    /* Hack -- limit flow depth */
    if (n == MONSTER_FLOW_DEPTH) return;

    /* Enqueue that entry */
    temp_y[flow_head] = y;
    temp_x[flow_head] = x;

    /* Advance the queue */
    if (++flow_head == TEMP_MAX) flow_head = 0;

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
    if (temp_n) return;

    /* Cycle the old entries (once per 128 updates) */
    if (flow_n == 255) {

        /* Rotate the time-stamps */
        for (y = 0; y < cur_hgt; y++) {
            for (x = 0; x < cur_wid; x++) {
                int w = cave[y][x].when;
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
    update_flow_aux(py, px, 0);

    /* Now process the queue */
    while (flow_head != flow_tail) {

        /* Extract the next entry */
        y = temp_y[flow_tail];
        x = temp_x[flow_tail];

        /* Forget that entry */
        if (++flow_tail == TEMP_MAX) flow_tail = 0;

        /* Add the "children" */
        for (d = 0; d < 8; d++) {

            /* Add that child if "legal" */
            update_flow_aux(y+ddy_ddd[d], x+ddx_ddd[d], cave[y][x].cost+1);
        }
    }

    /* Forget the flow info */
    flow_head = flow_tail = 0;

#endif

}







/*
 * Hack -- map the current panel (plus some) ala "magic mapping"
 */
void map_area(void)
{
    int		i, x, y, y1, y2, x1, x2;

    cave_type	*c_ptr;
    

    /* Pick an area to map */
    y1 = panel_row_min - randint(10);
    y2 = panel_row_max + randint(10);
    x1 = panel_col_min - randint(20);
    x2 = panel_col_max + randint(20);

    /* Speed -- shrink to fit legal bounds */
    if (y1 < 1) y1 = 1;
    if (y2 > cur_hgt-2) y2 = cur_hgt-2;
    if (x1 < 1) x1 = 1;
    if (x2 > cur_wid-2) x2 = cur_wid-2;

    /* Scan that area */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            c_ptr = &cave[y][x];

            /* All non-walls are "checked" */
            if ((c_ptr->feat & 0x3F) < 0x30) {

                /* Memorize landmarks */
                if ((c_ptr->feat & 0x3F) >= 0x03) {

                    /* Memorize the object */
                    c_ptr->feat |= CAVE_MARK;
                }

                /* Field-Mark the "true" walls */
                for (i = 0; i < 8; i++) {

                    c_ptr = &cave[y+ddy_ddd[i]][x+ddx_ddd[i]];

                    /* Memorize the "interesting" walls */
                    if ((c_ptr->feat & 0x3F) >= 0x30) {

                        /* Memorize the walls */
                        c_ptr->feat |= CAVE_MARK;
                    }
                }
            }
        }
    }

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
}



/*
 * Light up the dungeon.
 */
void wiz_lite(void)
{
    cave_type *c_ptr;
    int        yy, xx, y, x;

    /* Perma-light all open space and adjacent walls */
    for (y = 1; y < cur_hgt-1; y++) {
        for (x = 1; x < cur_wid-1; x++) {

            /* Access the grid */
            c_ptr = &cave[y][x];
            
            /* Process all non-walls */
            if ((c_ptr->feat & 0x3F) < 0x30) {

                /* Perma-lite all grids touching those grids */
                for (yy = y - 1; yy <= y + 1; yy++) {
                    for (xx = x - 1; xx <= x + 1; xx++) {

                        /* Get the grid */
                        c_ptr = &cave[yy][xx];

                        /* Perma-lite and memorize the grid */
                        c_ptr->feat |= (CAVE_GLOW | CAVE_MARK);
                    }
                }
            }
        }
    }

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
}


/*
 * Forget the dungeon map (ala "Thinking of Maud...").
 */
void wiz_dark(void)
{
    int        y, x;

    /* Forget every grid */
    for (y = 0; y < cur_hgt; y++) {
        for (x = 0; x < cur_wid; x++) {

            /* Forget the grid */
            cave[y][x].feat &= ~CAVE_MARK;
        }
    }

    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_NOTE);
    
    /* Update the view and lite */
    p_ptr->update |= (PU_VIEW | PU_LITE);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
}





/*
 * Display highest priority object in the RATIO by RATIO area
 */
#define	RATIO 3

/*
 * Display the entire map
 */
#define MAP_HGT (MAX_HGT / RATIO)
#define MAP_WID (MAX_WID / RATIO)

/*
 * Hack -- priority array
 */
static byte priority_table[][2] = {

    /* Stairs */
    { 0x06, 25 },
    { 0x07, 25 },

    /* Hidden gold */
    { 0x37, 19 },
    { 0x36, 19 },

    /* Closed doors */
    { 0x20, 17 },

    /* Open doors */
    { 0x05, 15 },
    { 0x04, 15 },

    /* Rubble */
    { 0x31, 13 },

    /* Magma */
    { 0x32, 12 },

    /* Quartz */
    { 0x33, 11 },

    /* Walls */
    { 0x30, 10 },

    /* Floors */
    { 0x01, 5 },

    /* End */
    { 0, 0 }
};


/*
 * Hack -- a priority function
 */
static int priority(byte a, char c)
{
    int i;
    
    /* Nothing */
    if (c == ' ') return (1);
    
    /* Player XXX XXX XXX */
    if (c == '@') return (30);

    /* Scan the table */
    for (i = 0; priority_table[i][0]; i++) {

        int p1 = priority_table[i][0];
        int p2 = priority_table[i][1];

        /* Found a match */
        if ((c == f_info[p1].z_char) && (a == f_info[p1].z_attr)) return (p2);
    }
    
    /* Default */
    return (20);
}


/*
 * Display a "small-scale" map of the dungeon
 *
 * Note that we must cancel the "lighting" options during this
 * function or the "priority()" code will not work correctly.
 */
void do_cmd_view_map(void)
{
    int i, j, x, y;

    byte ta;
    char tc;

    bool old_view_yellow_lite = view_yellow_lite;
    bool old_view_bright_lite = view_bright_lite;
    
    char mc[MAP_HGT + 2][MAP_WID + 2];
    char ma[MAP_HGT + 2][MAP_WID + 2];


    /* Hack -- Cancel the options */
    view_yellow_lite = FALSE;
    view_bright_lite = FALSE;


    /* Clear the chars and attributes */
    for (y = 0; y < MAP_HGT+2; ++y) {
        for (x = 0; x < MAP_WID+2; ++x) {
            ma[y][x] = TERM_WHITE;
            mc[y][x] = ' ';
        }
    }

    /* Fill in the map */
    for (i = 0; i < cur_wid; ++i) {
        for (j = 0; j < cur_hgt; ++j) {

            /* Index into mc/ma */
            x = i / RATIO + 1;
            y = j / RATIO + 1;

            /* Extract the current attr/char at that map location */
            map_info(j, i, &ta, &tc);

            /* If this thing is more important, save it instead */
            if (priority(ma[y][x], mc[y][x]) < priority(ta, tc)) {
                ma[y][x] = ta;
                mc[y][x] = tc;
            }
        }
    }

    /* Corners */
    x = MAP_WID + 1;
    y = MAP_HGT + 1;

    /* Draw the corners */
    mc[0][0] = mc[0][x] = mc[y][0] = mc[y][x] = '+';

    /* Draw the horizontal edges */
    for (x = 1; x <= MAP_WID; x++) mc[0][x] = mc[y][x] = '-';

    /* Draw the vertical edges */
    for (y = 1; y <= MAP_HGT; y++) mc[y][0] = mc[y][x] = '|';


    /* Paranoia */
    msg_print(NULL);

    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Clear the screen */
    Term_clear();

    /* Display each map line in order */
    for (y = 0; y < MAP_HGT+2; ++y) {

        /* Start a new line */
        Term_gotoxy(0, y);

        /* Display the line */
        for (x = 0; x < MAP_WID+2; ++x) {

            ta = ma[y][x];
            tc = mc[y][x];

#ifdef USE_COLOR

            /* Force mono-chrome */
            if (!use_color) ta = TERM_WHITE;

            /* Add the character */
            Term_addch(ta, tc);
#else

            /* Add the character (in white) */
            Term_addch(TERM_WHITE, tc);
#endif

        }
    }

    /* Wait for it */
    put_str("Hit any key to continue", 23, 23);

    /* Hilite the player */
    move_cursor(py / RATIO + 1, px / RATIO + 1);

    /* Get any key */
    inkey();

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;


    /* Hack -- Restore the options */
    view_yellow_lite = old_view_yellow_lite;
    view_bright_lite = old_view_bright_lite;
}





