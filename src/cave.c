/* File: cave.c */

/* Purpose: low level dungeon routines -BEN- */

#include "angband.h"


/*
 * The following variables need to be initialized
 */
static bool ii_init = TRUE;

/*
 * Hack -- fake variables for visual display
 */
static inven_type ii_player;
static inven_type ii_floor;
static inven_type ii_granite_wall;
static inven_type ii_quartz_vein;
static inven_type ii_magma_vein;
static inven_type ii_up_stair;
static inven_type ii_down_stair;
static inven_type ii_open_door;
static inven_type ii_closed_door;




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
 */

/*
 * Because this function uses (short) ints for all calculations, overflow may
 * occur if d_x and d_y exceed 90.
 *
 * Also note that this function and the "move towards target" code do NOT
 * share the same properties.  Thus, you can see someone, target them, and
 * then fire a bolt at them, but the bolt may hit a wall, not them.  However,
 * by clever choice of target locations, you can sometimes throw a "curve".
 *
 * Use the "projectable()" routine to test "spell/missile line of sight".
 */
int los(int y1, int x1, int y2, int x2)
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
 * Note -- "GRID_LITE" is only set if the "torch" has "los()".
 * So, given "GRID_LITE", we know that the grid is "fully visible".
 *
 * Note that "GRID_GLOW" makes little sense for a wall, since it would mean
 * that a wall is visible from any direction.  That would be odd.  Except
 * under wizard light, which might make sense.
 *
 * Note that the test for walls includes a loop which cannot use the
 * "floor_grid_bold()" function, since this function may be called on
 * a grid which is actually in the outer wall.
 *
 * Note that the special processing for checking illumination of walls
 * takes a little extra time, but provides a more "correct" metaphor.
 *
 * Note that it is very important *not* to run the final "wall checker"
 * loop on the outer walls of the dungeon (it will induce a memory fault).
 *
 * To speed up the function, we assume that all "perma-walls", once
 * perma-lit, are illuminated from all sides.  This is correct for all
 * cases except monster vaults and the buildings in town.  But the town
 * is a hack anyway, and the player has more important things on his
 * mind when he is attacking a monster vault.  Note that I hate to do
 * this, even though it is "just a little hack", but the savings in
 * processor time are rather extreme when "view_bright_lite" is set.
 */
bool player_can_see_bold(int y, int x)
{
    int i;

    /* Blind players see nothing */
    if (p_ptr->blind) return (FALSE);

    /* Note that "torch-lite" yields "illumination" */
    if (cave[y][x].info & GRID_LITE) return (TRUE);

    /* Require line of sight to the grid */
    if (!player_has_los_bold(y, x)) return (FALSE);

    /* Require "perma-lite" of the grid */
    if (!(cave[y][x].info & GRID_GLOW)) return (FALSE);

    /* Assume perma-lit viewable floors are illuminated */
    if (floor_grid_bold(y, x)) return (TRUE);

    /* XXX XXX Prevent the following loop from crashing (see above) */
    /* Hack -- Assume perma-lit viewable perma-walls are illuminated */
    /* This is always true except for certain internal perma-walls */
    /* such as those found surrounding many greater vaults */
    if (cave[y][x].info & GRID_PERM) return (TRUE);

    /* Walls require line-of-sight to an adjacent, perma-lit, non-wall */
    for (i = 0; i < 8; i++) {

        /* Extract adjacent (legal) location */
        int yy = y + ddy[ddd[i]];
        int xx = x + ddx[ddd[i]];

        /* Check for adjacent perma-lit viewable floor */
        if ((floor_grid_bold(yy, xx)) &&
            (player_has_los_bold(yy, xx)) &&
            (cave[yy][xx].info & GRID_GLOW)) {

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
int no_lite(void)
{
    return (!player_can_see_bold(py, px));
}




/*
 * Initialize the global variables for the "fake objects"
 */
static void ii_prepare(void)
{
    if (!ii_init) return;

    invcopy(&ii_player, OBJ_PLAYER);

    invcopy(&ii_floor, OBJ_FLOOR);
    invcopy(&ii_granite_wall, OBJ_GRANITE_WALL);
    invcopy(&ii_quartz_vein, OBJ_QUARTZ_VEIN);
    invcopy(&ii_magma_vein, OBJ_MAGMA_VEIN);

    invcopy(&ii_up_stair, OBJ_UP_STAIR);
    invcopy(&ii_down_stair, OBJ_DOWN_STAIR);
    invcopy(&ii_open_door, OBJ_OPEN_DOOR);
    invcopy(&ii_closed_door, OBJ_CLOSED_DOOR);

    ii_init = FALSE;
}


/*
 * This function helps decide what attr/char to use for a grid.
 *
 * Warning: This function may ONLY be called from "map_info()".
 *
 * Note that we always start with (White,Space) as input
 *
 * Need to handle "MULTI_HUED" property better.  Probably best to
 * use another flag on "wearable objects".  If we can find one.
 *
 * Note the use of the "fake objects" to help draw the "floor" and
 * "walls" and "mineral veins".
 *
 * Special colors (used for floors, walls, quartz):
 *   Option "view_yellow_lite" draws "torch radius" in yellow
 *   Option "view_bright_lite" draws "hidden grids" dimmer.
 */
static void map_info_aux(int y, int x, byte *ap, char *cp)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    /* Allow yellow/bright */
    bool allow = FALSE;

    /* Extract the wall type */
    int wall;


    /* Get the cave */
    c_ptr = &cave[y][x];


    /* Non-memorized grids */
    if (!(c_ptr->info & GRID_MARK)) {

        /* Non-illuminated grids are "unknown" */
        if (!player_can_see_bold(y, x)) return;
    }


    /* Extract the wall type */
    wall = (c_ptr->info & GRID_WALL_MASK);

    /* Start with the actual object in the grid */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Hack -- invisible traps (treat as "floor") */
    if (i_ptr->tval == TV_INVIS_TRAP) {
        i_ptr = &ii_floor;
        allow = TRUE;
    }

    /* Hack -- secret doors (treat as "granite wall") */
    else if (i_ptr->tval == TV_SECRET_DOOR) {
        i_ptr = &ii_granite_wall;
        allow = TRUE;
    }

    /* Normal objects */
    else if (i_ptr->tval) {
        /* Use the actual object */
    }

    /* Non walls yield "floors" */
    else if (!wall) {
        i_ptr = &ii_floor;
        allow = TRUE;
    }

    /* Handle illuminated normal seams */
    else if (notice_seams && (wall == GRID_WALL_QUARTZ)) {
        i_ptr = &ii_quartz_vein;
        allow = TRUE;
    }

    /* Handle illuminated "magma" seams */
    else if (notice_seams && (wall == GRID_WALL_MAGMA)) {
        i_ptr = &ii_magma_vein;
        allow = TRUE;
    }

    /* Handle all left-over rocks */
    else {
        i_ptr = &ii_granite_wall;
        allow = TRUE;
    }


    /* Extract the proper symbol */
    (*cp) = inven_char(i_ptr);


#ifdef USE_COLOR

    /* Fake mono-chrome */
    if (!use_color) return;

    /* Option -- Draw the "torch-radius" in yellow */
    if (allow && view_yellow_lite &&
        (c_ptr->info & GRID_LITE)) {

        /* Yellow from the torch */
        (*ap) = TERM_YELLOW;
    }

    /* Option -- Darken the non-illuminated grids */
    else if (allow && view_bright_lite &&
             (!player_can_see_bold(y, x))) {

        /* Gray (dim) from lack of light */
        (*ap) = TERM_GRAY;
    }

    /* Default -- normal object color */
    else {

        /* Extract the color */
        (*ap) = inven_attr(i_ptr);
    }

#endif

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
 * Extract the attr and char of a given MAP location
 *
 * Assume given location is "legal".
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
static void map_info(int y, int x, int *mp, byte *ap, char *cp)
{
    cave_type *c_ptr = &cave[y][x];

    monster_type *m_ptr = &m_list[c_ptr->m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];


    /* Default to NOT "multihued" */
    (*mp) = 0;

    /* Default to "white" */
    (*ap) = TERM_WHITE;

    /* Default to "space" */
    (*cp) = ' ';


    /* Initialize the "fake objects" */
    if (ii_init) ii_prepare();


    /* Hack -- player is always "visible" unless "running blind" */
    if ((c_ptr->m_idx == 1) && (!find_flag || find_prself)) {

        /* Get the player char/attr */
        (*cp) = inven_char(&ii_player);
        (*ap) = inven_attr(&ii_player);

        /* All done */
        return;
    }


    /* Mega-Hack -- Blind people see nothing (except themselves) */
    if (p_ptr->blind) return;


    /* Hallucination kicks in occasionally */
    if (p_ptr->image && (0 == rand_int(12))) {

        /* Get a fake char/attr */
        (*cp) = image_char();
        (*ap) = image_attr();

        /* All done */
        return;
    }


    /* Normal, visible monsters "block" the object they are on */
    if ((c_ptr->m_idx > 1) && (m_ptr->ml)) {

        bool done = FALSE;

#ifdef USE_COLOR

        /* Clear monster */
        if (r_ptr->rflags1 & RF1_ATTR_CLEAR) {

            /* Examine the ground / object */
            map_info_aux(y, x, ap, cp);

            /* Remember we did it */
            done = TRUE;
        }

        /* Normal monster */
        else {

            /* Extract an attribute */
            (*ap) = l_ptr->l_attr;

            /* Apply the "multi-hued" flag */
            if (r_ptr->rflags1 & RF1_ATTR_MULTI) {
                (*mp) = 1;
                (*ap) = mh_attr();
            }
        }

#endif

        /* Use the given symbol unless "clear" */
        if (r_ptr->rflags1 & RF1_CHAR_CLEAR) {

            /* Examine the ground / object */
            if (!done) map_info_aux(y, x, ap, cp);
        }

        /* Normal monster */	
        else {

            /* Extract a character */
            (*cp) = l_ptr->l_char;

#if 0
            /* Apply the "mimic" flag */
            if (r_ptr->rflags1 & RF1_ATTR_MULTI) {
                /* XXX XXX */
            }
#endif

        }

        /* Done */
        return;
    }


    /* Examine the ground / object */
    map_info_aux(y, x, ap, cp);
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
 *
 * The "multi-hued" code originates from an idea in MacAngband 2.6.1
 * (thus some code is adapted from Keith Randall).
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
 *
 * This routine will miss changes in which the char is "changed"
 * to the same symbol, and just happens to keep the color that
 * was assigned at the most recent "mh_cycle" or "mh_print".
 * Technically, this represents a (minor) problem. XXX XXX XXX
 *
 * I am thinking that it would be much more efficient to ASSUME
 * that all printing is done by mh_print, and so we do NOT have
 * to check the current contents of the screen.  This would be
 * true, if not for the frequent use of, say, clear_screen().
 *
 * In any case, note that currently this function is dependant on
 * a correctly functioning "Term_what()" function.
 *
 * This whole function may be un-necessary, assuming that the only
 * "multi-hued" objects are monsters, or that an object only needs
 * to "shimmer" occasionally.  In any case, a run-time "no shimmer"
 * option would allow optimization of "prt_map()" and "lite_spot()".
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

    /* Apply software "mono" */
    if (!use_color) m = 0, a = TERM_WHITE;

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
 * Simply call "mh_print" (above), but account for "panel-relative"
 * locations, and also verify that the given location is "on the map".
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
 * This function is only called by "()"
 * This function is only called on "legal" grids.
 *
 * This is the ONLY way a player can "remember" things that he cannot
 * currently "see" (but consider true "memorization" of attr/chars).
 * This routine would be the one to maintain the "memorization" fields.
 * Only one function () updates the screen, and it always calls
 * us first, to take notes on the terrain.
 *
 * Note that we will NOT take notes on "unknown" grids.
 */
static void update_map(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];


    /* Only need to memorize a grid once */
    if (c_ptr->info & GRID_MARK) return;


    /* Hack -- Only memorize grids that can be seen */
    if (!player_can_see_bold(y, x)) return;


    /* Option -- field mark every torch-lit grid */
    if (view_torch_grids && (c_ptr->info & GRID_LITE)) {
        c_ptr->info |= GRID_MARK;
        return;
    }

    /* Option -- field mark every perma-lit grid */
    if (view_perma_grids && (c_ptr->info & GRID_GLOW)) {
        c_ptr->info |= GRID_MARK;
        return;
    }

    /* Option -- field mark all "non-floor" grids */
    if (view_wall_memory && !floor_grid_bold(y, x)) {
        c_ptr->info |= GRID_MARK;
        return;
    }

    /* Option -- field mark all "floor landmarks" */
    if (view_xtra_memory && floor_grid_bold(y, x) &&
        (i_list[c_ptr->i_idx].tval >= TV_MIN_VISIBLE)) {
        c_ptr->info |= GRID_MARK;
        return;
    }
}



/*
 * Redraw (on the screen) a given MAP location
 */
void lite_spot(int y, int x)
{
    int m;
    byte a;
    char c;

    /* Hack -- Update the map */
    update_map(y, x);

    /* Redraw if on screen */
    if (panel_contains(y, x)) {

        /* Examine the contents of that grid */
        map_info(y, x, &m, &a, &c);

        /* Efficiency -- immitate "mh_print_rel()" */
        mh_print(c, a, m, y-panel_row_prt, x-panel_col_prt);
    }
}




/*
 * Prints the map of the dungeon
 *
 * Note that we contain an "inline" version of "lite_spot()"
 * and "mh_print_rel()", since this function is called a lot.
 */
void prt_map(void)
{
    int x, y;

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
        prt("", 1+y-panel_row_min, 13);

        /* Scan the columns of row "y" */
        for (x = panel_col_min; x <= panel_col_max; x++) {

            /* Hack -- update the map */
            update_map(y, x);

            /* Determine what is there */
            map_info(y, x, &m, &a, &c);

            /* Efficiency -- Redraw that grid of the map */
            mh_print(c, a, m, y-panel_row_prt, x-panel_col_prt);
        }
    }

    /* Show the cursor (if necessary) */
    if (!okay) Term_show_cursor();
}














/*
 * And now for info on the "update_view()" function...  -BEN-
 *
 * The "update_view()" function maintains the "GRID_VIEW" and "GRID_XTRA"
 * flags for each grid, and maintains an array of all "GRID_VIEW" grids.
 * This set of grids is the complete set of all grids within line of sight
 * of the player, allowing the "player_has_los()" macro to work very fast.
 * In addition, the current algorithm sets "GRID_XTRA" for those grids
 * which are "easily" in line of sight of the player.
 *
 * The "update_lite()" function maintains the "GRID_LITE" (torch lit) flag
 * for each grid, and maintains an array of all "GRID_LITE" grids.  These
 * grids are the ones which are "illuminated" by the player's "torch".
 * Note that every "GRID_LITE" grid is also a "GRID_VIEW" grid, and in
 * fact, the player (unless blind) can always "see" all "GRID_LITE" grids,
 * except (perhaps) if they are "off screen" (on a different panel).
 *
 * Any grid can be marked as "GRID_GLOW" which means that the grid itself is
 * in some way permanently lit.  However, for the player to "see" anything
 * in the grid, as determined by "player_can_see()", the player must not be
 * blind, the grid must be marked as "GRID_VIEW", and, in addition, "wall"
 * grids, even if marked as "perma lit", are only illuminated if they touch
 * a grid which is not a wall and is marked both "GRID_GLOW" and "GRID_VIEW".
 *
 * To simplify various things, a grid may be marked as "GRID_MARK" which means
 * that even if the player cannot "see" the grid, he "knows" what is there.
 * This is used to "remember" walls/doors/stairs/floors/objects, and to
 * implement most of the "detection" spells.
 *
 * A grid may be marked as "GRID_ROOM" which means that it is part of a "room",
 * and should be illuminated by "lite room" and "darkness" spells.
 *
 * For various reasons, a grid may be marked as "GRID_SEEN" and put into the
 * array of "GRID_SEEN" grids.  This is a temporary flag/array, primarily used
 * for optimizing "update_view()" and "update_lite()", for spreading lite/dark
 * during "lite_room()" / "unlite_room()", and for calculating monster flow.
 *
 * The old "CAVE_INIT" flag is saved in savefiles, but is currently unused.
 * Note that the old system used "CAVE_INIT" because grids were not marked
 * as "GRID_GLOW" until they were first "seen", due to a brain dead method of
 * displaying rooms only when first seen, and of using the "GRID_GLOW" flag to
 * mark a room (or walls) as "known".  This was also done by "GRID_MARK", so
 * there was a lot of redundancy.  The old "check_view()" routine was called
 * a lot, and it "flooded" rooms with light when they were first encountered.
 * The "CAVE_INIT" flag allowed 2.7.0-2.7.3 to keep track of which rooms had
 * been flooded.  This was scrapped in 2.7.4v2 in favor of "update_view()",
 * previously optional on the "pre_compute_view" flag.
 *
 * Several flags are available to control which grids are "memorized", in
 * addition to those detected by various spells.  The "update_map()" function
 * allows the user to memorize all walls, all landmarks, all perma-lit grids,
 * and/or all torch-lit grids.  Note that turning off all four options is only
 * possible now that "update_view()" is required, and provides an "interesting"
 * way to only see the "currently known" grids.  Maybe the "borg" will use this.
 *
 * Note that the new "update_view()" method allows, among other things, a room
 * to be "partially" seen as the player approaches it, with a growing cone of
 * floor appearing as the player gets closer to the door.  Also, by not turning
 * on the "memorize perma-lit grids" option, the player will no longer have to
 * deal with "magic treasure".  An even better approach might be to add fields
 * to the "cave_type" structure to associate a "memory" (attr/char) with every
 * grid, and to use that "memory" whenever the "GRID_MARK" flag is set.  This
 * would also avoid the "magic object" problem.
 *
 * And my favorite "plus" is that you can now use a special option to draw the
 * walls/floors in the "viewable region" brightly (actually, to draw the *other*
 * grids dimly), providing a "pretty" effect as the player runs around.  This is
 * not recommended for slow machines, as it is one of the more abusive routines.
 */



/*
 * Some comments on the "update_view()" algorithm...
 *
 * The algorithm marks grids as "GRID_VIEW" if the player has "los()" to
 * the grid, and it marks them as "GRID_XTRA" if, in addition, the "los()"
 * is very "obvious" or "easily determinable".
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
 * And note that only "GRID_GLOW" and "GRID_ROOM" and "GRID_MARK" need to be saved
 * in the savefile, leaving us with a free bit-flag to save later.
 */





/*
 * Maximum size of the "lite" array
 * Note that "radius 11" lite only fills 480 grids, so we
 * will never need even close to 500 entries in the array.
 */
#define LITE_MAX 500

/*
 * Maintain an array of "CAVE_LITE" cave grids (see below)
 */
static int lite_n = 0;
static byte lite_y[LITE_MAX];
static byte lite_x[LITE_MAX];


/*
 * Maximum size of the "view" array
 * We assume that the "view radius" will NEVER exceed 20, so
 * we will never need more than 1500 entries in the array.
 */
#define VIEW_MAX 1500

/*
 * Maintain an array of "CAVE_VIEW" cave grids (see below)
 */
static int view_n = 0;
static byte view_y[VIEW_MAX];
static byte view_x[VIEW_MAX];


/*
 * Number of grids in the "seen" array
 * We must be as large as "VIEW_MAX" and "LITE_MAX" for proper
 * functioning of "update_view()" and "update_lite()".
 * We must also be as large as the largest illuminatable room,
 * but the largest vault is only 40*20 = 800 grids in size.
 */
#define SEEN_MAX 2000

/*
 * This is the "set" of grids marked as "GRID_SEEN".
 * These grids have many interpretations, see below.
 */
static int seen_n = 0;
static byte seen_y[SEEN_MAX];
static byte seen_x[SEEN_MAX];







/*
 * The given grid just got lit, wake up monsters
 */
static void wake_monster(int y, int x)
{
    cave_type		*c_ptr = &cave[y][x];

    monster_type	*m_ptr = &m_list[c_ptr->m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];


    /* Stupid monsters wake up occasionally */
    if (((r_ptr->rflags2 & RF2_STUPID) && (0 == rand_int(10))) ||
        (r_ptr->rflags2 & RF2_SMART) || (0 == rand_int(3))) {

        /* Wake up! */
        m_ptr->csleep = 0;

        /* XXX XXX XXX Message */
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
 * SMART monsters always wake up,
 * NORMAL monsters wake up 1/3 the time, and
 * STUPID monsters wake up 1/10 the time -CWS
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
        cave[y][x].info &= ~GRID_SEEN;

        /* Update only non-GRID_GLOW grids */
        if (cave[y][x].info & GRID_GLOW) continue;

        /* Perma-Lite */
        cave[y][x].info |= GRID_GLOW;

        /* Redraw */
        lite_spot(y, x);
        
        /* Attempt to wake up monsters in that grid */
        wake_monster(y, x);
    }

    /* None left */
    seen_n = 0;
}



/*
 * This routine clears the entire "seen" set.
 *
 * This routine will Un-Perma-Lite all "seen" grids.
 * In addition, some of these grids are un-field-marked.
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
        cave[y][x].info &= ~GRID_SEEN;

        /* Only update grids which are changing */
        if (!(cave[y][x].info & (GRID_GLOW | GRID_MARK))) continue;

        /* Darken the grid */
        cave[y][x].info &= ~GRID_GLOW;

        /* Forget some grids */
        if (!(cave[y][x].info & GRID_WALL_MASK) &&
            (i_list[cave[y][x].i_idx].tval < TV_MIN_VISIBLE)) {
            cave[y][x].info &= ~GRID_MARK;
        }

        /* Redraw */
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
        cave[lite_y[i]][lite_x[i]].info &= ~GRID_LITE;

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
 * "GRID_SEEN" flag is NOT set.
 *
 * We should probably do "wake_monster(y, x)" in some cases
 */
static void cave_lite(int y, int x)
{
    /* Already lit */
    if (cave[y][x].info & GRID_LITE) return;

    /* Set the flag */
    cave[y][x].info |= GRID_LITE;

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
    if (cur_lite <= 0) {

        /* Forget the old lite */
        forget_lite();

        /* Hack -- Draw the player's grid anyway */
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
        cave[y][x].info &= ~GRID_LITE;

        /* Mark the grid as "seen" */
        cave[y][x].info |= GRID_SEEN;

        /* Add it to the "seen" set */
        seen_y[seen_n] = y;
        seen_x[seen_n] = x;
        seen_n++;
    }

    /* None left */
    lite_n = 0;


    /*** Collect the new "lite" grids ***/

    /* Efficiency -- torch radius */
    if (cur_lite == 1) {

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
    else if (cur_lite == 2) {

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
    else if (cur_lite == 3) {

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

    /* Unused -- Larger radius */
    else {

        /* Extract maximal legal bounded light box */
        min_y = MAX(0, (py - cur_lite));
        max_y = MIN(cur_hgt-1, (py + cur_lite));
        min_x = MAX(0, (px - cur_lite));
        max_x = MIN(cur_wid-1, (px + cur_lite));

        /* Scan that box (the viewable, reachable, portions of it) */
        for (y = min_y; y <= max_y; y++) {
            for (x = min_x; x <= max_x; x++) {

                /* Viewable, nearby, grids get "torch lit" */
                if (player_has_los_bold(y, x) &&
                    (distance(py, px, y, x) <= cur_lite)) {

                    /* This grid is "torch lit" */
                    cave_lite(y, x);
                }
            }
        }
    }


    /*** Complete the algorithm ***/

    /* Draw the new grids */
    for (i = 0; i < lite_n; i++) {

        y = lite_y[i];
        x = lite_x[i];

        /* Update fresh grids */
        if (!(cave[y][x].info & GRID_SEEN)) lite_spot(y, x);
    }

    /* Clear them all */
    for (i = 0; i < seen_n; i++) {

        y = seen_y[i];
        x = seen_x[i];

        /* No longer in the array */
        cave[y][x].info &= ~GRID_SEEN;

        /* Update stale grids */
        if (!(cave[y][x].info & GRID_LITE)) lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
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
        cave[y][x].info &= ~GRID_VIEW;
        cave[y][x].info &= ~GRID_XTRA;

        /* Update the screen */
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
    if (cave[y][x].info & GRID_VIEW) return;

    /* Set the value */
    cave[y][x].info |= GRID_VIEW;

    /* Add to queue */
    view_y[view_n] = y;
    view_x[view_n] = x;
    view_n++;
}



/*
 * We are checking the "viewability" of grid (y,x) by the player.
 *
 * This function assumes that (y,x) is on the map.
 *
 * Grid (y1,x1) is on the "diagonal" between (py,px) and (y,x)
 * Grid (y2,x2) is "adjacent", also between (py,px) and (y,x).
 *
 * Note that we are using the "GRID_XTRA" field for marking grids as
 * "easily viewable".  We can (easily) clear this field in "update_view()".
 *
 * This function adds (y,x) to the "viewable set" if necessary.
 *
 * This function now returns "TRUE" if vision is "blocked" by grid (y,x).
 */
static bool update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, z1, z2;


    /* Examine the given grid */
    bool wall = (!floor_grid_bold(y,x));


    /* Check the walls */
    f1 = (floor_grid_bold(y1,x1));
    f2 = (floor_grid_bold(y2,x2));

    /* Totally blocked by physical walls */
    if (!f1 && !f2) return (TRUE);


    /* Check the visibility */
    v1 = (f1 && (cave[y1][x1].info & GRID_VIEW));
    v2 = (f2 && (cave[y2][x2].info & GRID_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);


    /* Check the "ease" of visibility */
    z1 = (v1 && (cave[y1][x1].info & GRID_XTRA));
    z2 = (v2 && (cave[y2][x2].info & GRID_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (z1 && z2) {

        cave[y][x].info |= GRID_XTRA;
        cave_view(y, x);
        return (wall);
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (z1) {

        cave_view(y, x);
        return (wall);
    }


    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2) {

        /* cave[y][x].info |= GRID_XTRA; */
        cave_view(y, x);
        return (wall);
    }


    /* Mega-Hack -- the "los()" function works poorly on walls */
    if (wall) {

        cave_view(y, x);
        return (wall);
    }


    /* Hack -- check line of sight */
    if (los(py, px, y, x)) {

        cave_view(y, x);
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
 *  3: Process the main axes
 *  3a: The main axes are (easily) viewable up to the first wall
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
 * The disadvantage of this is that it is now possible for grids which are
 * more than 20 grids away to be viewable.  In particular, in the town, this
 * algorithm thinks that certain grids which are 21 or 22 grids away are
 * actually in line of sight.  This causes problems with "update_mon()".
 * This also happens at the edges of "large" rooms.
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
 * Note also the care taken to prevent "running off the map".  The use of
 * explicit checks on the "validity" of the "diagonal", and the fact that
 * the loops are never allowed to "leave" the map lets "update_view_aux()"
 * use the optimized version of "floor_grid_bold()".
 */
void update_view(void)
{
    int n, m, d, k, y, x;

    int se, sw, ne, nw, es, en, ws, wn;

    int over, full;


    /* Start with full vision */
    full = MAX_SIGHT;

#if 0
    /* XXX XXX Hack -- reduce view when running */
    /* Note that "find_flag" is now a "boolean" */
    if (view_reduce_view && find_flag) {
        full = full - (find_flag-1);
        if (full < 10) full = 10;
    }
#endif

    /* Extract the "octagon" limits */
    over = full * 3 / 2;


    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < view_n; n++) {

        int y = view_y[n];
        int x = view_x[n];

        /* Mark the grid as not in "view" */
        cave[y][x].info &= ~(GRID_VIEW | GRID_XTRA);

        /* Mark the grid as "seen" */
        cave[y][x].info |= GRID_SEEN;

        /* Add it to the "seen" set */
        seen_y[seen_n] = y;
        seen_x[seen_n] = x;
        seen_n++;
    }

    /* Start over with the "view" array */
    view_n = 0;


    /*** Step 1 -- adjacent grids ***/

    /* Now start on the player */
    y = py;
    x = px;

    /* Assume the player grid is easily viewable */
    cave[y][x].info |= GRID_XTRA;

    /* Assume the player grid is viewable */
    cave_view(y, x);


    /*** Step 2 -- Major Diagonals ***/

    /* Scan south-east */
    for (d = 1; d <= full; d++) {
        cave[y+d][x+d].info |= GRID_XTRA;
        cave_view(y+d, x+d);
        if (!floor_grid_bold(y+d, x+d)) break;
    }

    /* Scan south-west */
    for (d = 1; d <= full; d++) {
        cave[y+d][x-d].info |= GRID_XTRA;
        cave_view(y+d, x-d);
        if (!floor_grid_bold(y+d, x-d)) break;
    }

    /* Scan north-east */
    for (d = 1; d <= full; d++) {
        cave[y-d][x+d].info |= GRID_XTRA;
        cave_view(y-d, x+d);
        if (!floor_grid_bold(y-d, x+d)) break;
    }

    /* Scan north-west */
    for (d = 1; d <= full; d++) {
        cave[y-d][x-d].info |= GRID_XTRA;
        cave_view(y-d, x-d);
        if (!floor_grid_bold(y-d, x-d)) break;
    }


    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++) {
        cave[y+d][x].info |= GRID_XTRA;
        cave_view(y+d, x);
        if (!floor_grid_bold(y+d, x)) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++) {
        cave[y-d][x].info |= GRID_XTRA;
        cave_view(y-d, x);
        if (!floor_grid_bold(y-d, x)) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++) {
        cave[y][x+d].info |= GRID_XTRA;
        cave_view(y, x+d);
        if (!floor_grid_bold(y, x+d)) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++) {
        cave[y][x-d].info |= GRID_XTRA;
        cave_view(y, x-d);
        if (!floor_grid_bold(y, x-d)) break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;


    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++) {

        int limit, ypn, ymn, xpn, xmn;


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

        int y = view_y[n];
        int x = view_x[n];

        /* Update only newly viewed grids */
        if (!(cave[y][x].info & GRID_SEEN)) lite_spot(y, x);
    }

    /* Wipe the old grids, update as needed */
    for (n = 0; n < seen_n; n++) {

        int y = seen_y[n];
        int x = seen_x[n];

        /* No longer in the array */
        cave[y][x].info &= ~GRID_SEEN;

        /* Update only non-GRID_LITE grids */
        if (!(cave[y][x].info & GRID_VIEW)) lite_spot(y, x);
    }

    /* None left */
    seen_n = 0;
}






/*
 * Aux function -- see below
 */
static void cave_seen_room_aux(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Avoid infinite recursion */
    if (c_ptr->info & GRID_SEEN) return;

    /* Do not "leave" the current room */
    if (!(c_ptr->info & GRID_ROOM)) return;

    /* Paranoia -- verify space */
    if (seen_n == SEEN_MAX) return;

    /* Mark the grid as "seen" */
    c_ptr->info |= GRID_SEEN;

    /* Add it to the "seen" set */
    seen_y[seen_n] = y;
    seen_x[seen_n] = x;
    seen_n++;
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

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);
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

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);
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
    int old_head = flow_head;

    /* Ignore "pre-stamped" entries */
    if (cave[y][x].when == flow_n) return;

    /* Ignore "wall" grids (but not doors) */
    if (cave[y][x].info & GRID_WALL_MASK) return;

    /* Hack -- Ignore "rubble" since no monsters "eat" rubble */
    /* if (i_list[cave[y][x].i_idx].tval == TV_RUBBLE) return; */

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
        y = seen_y[flow_tail];
        x = seen_x[flow_tail];

        /* Forget that entry */
        if (++flow_tail == SEEN_MAX) flow_tail = 0;

        /* Add the "children" */
        for (d = 0; d < 8; d++) {

            /* Add that child if "legal" */
            update_flow_aux(y+ddy[ddd[d]], x+ddx[ddd[d]], cave[y][x].cost+1);
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
    cave_type *c_ptr;
    int        dx, dy, x, y;
    int                 y1, y2, x1, x2;

    /* Pick an area to map */
    y1 = panel_row_min - randint(10);
    y2 = panel_row_max + randint(10);
    x1 = panel_col_min - randint(20);
    x2 = panel_col_max + randint(20);

    /* Speed -- shrink to fit legal bounds (for floors) */
    if (y1 < 1) y1 = 1;
    if (y2 > cur_hgt-2) y2 = cur_hgt-2;
    if (x1 < 1) x1 = 1;
    if (x2 > cur_wid-2) x2 = cur_wid-2;

    /* Scan that area */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            c_ptr = &cave[y][x];

            /* All non-walls are "checked" */
            if (!(c_ptr->info & GRID_WALL_MASK)) {

                /* Field mark "landmark" objects */
                if (i_list[c_ptr->i_idx].tval >= TV_MIN_VISIBLE) {

                    /* Memorize the object */
                    c_ptr->info |= GRID_MARK;
                }

                /* Field-Mark the "true" walls */
                for (dy = -1; dy <= 1; dy++) {
                    for (dx = -1; dx <= 1; dx++) {

                        if (!dx && !dy) continue;

                        c_ptr = &cave[y+dy][x+dx];

                        /* Memorize the "interesting" walls */
                        if (c_ptr->info & GRID_WALL_MASK) {

                            /* Memorize the walls */
                            c_ptr->info |= GRID_MARK;
                        }
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
 * Light up the dungeon.
 */
void wiz_lite(void)
{
    cave_type *c_ptr;
    int        yy, xx, y, x;

    /* Perma-light all open space and adjacent walls */
    for (y = 1; y < cur_hgt-1; y++) {
        for (x = 1; x < cur_wid-1; x++) {

            /* Process all non-walls */
            if (!(cave[y][x].info & GRID_WALL_MASK)) {

                /* Perma-lite all grids touching those grids */
                for (yy = y - 1; yy <= y + 1; yy++) {
                    for (xx = x - 1; xx <= x + 1; xx++) {

                        /* Get the grid */
                        c_ptr = &cave[yy][xx];

                        /* Perma-lite all the grid */
                        c_ptr->info |= GRID_GLOW;

                        /* Hack -- Field Mark all of the objects */
                        c_ptr->info |= GRID_MARK;
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
            cave[y][x].info &= ~GRID_MARK;
        }
    }

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
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
#define MAP_HGT (MAX_HGT / RATIO)
#define MAP_WID (MAX_WID / RATIO)

/*
 * Display a "small-scale" map of the dungeon
 *
 * This function is also adapted from MacAngband 2.6.1, but
 * has been almost completely rewritten.
 */
void screen_map(void)
{
    int i, j, x, y;

    byte ta;
    char tc;

    int m, okay;

    char mc[MAP_HGT + 2][MAP_WID + 2];
    char ma[MAP_HGT + 2][MAP_WID + 2];

    int myrow = -1, mycol = -1;

    byte priority[256];


    /* Initialize the fake objects */
    if (ii_init) ii_prepare();


    /* Default priority for most objects */
    for (i = 0; i < 256; i++) priority[i] = 20;

    /* Some special things are *more* important */
    priority[(byte)inven_char(&ii_player)] = 30;
    priority[(byte)inven_char(&ii_up_stair)] = 25;
    priority[(byte)inven_char(&ii_down_stair)] = 25;

    /* Some special things are *less* important */
    priority[(byte)inven_char(&ii_closed_door)] = 17;
    priority[(byte)inven_char(&ii_open_door)] = 15;
    priority[(byte)inven_char(&ii_magma_vein)] = 12;
    priority[(byte)inven_char(&ii_quartz_vein)] = 11;
    priority[(byte)inven_char(&ii_granite_wall)] = 10;
    priority[(byte)inven_char(&ii_floor)] = 5;

    /* Emptiness always loses */
    priority[(byte)(' ')] = 0;


    /* Clear the chars and attributes */
    for (y = 0; y < MAP_HGT+2; ++y) {
        for (x = 0; x < MAP_WID+2; ++x) {
            ma[y][x] = TERM_WHITE;
            mc[y][x] = ' ';
        }
    }

    x = MAP_WID + 1;
    y = MAP_HGT + 1;

    /* Draw the corners */
    mc[0][0] = CH_TL;
    mc[0][x] = CH_TR;
    mc[y][0] = CH_BL;
    mc[y][x] = CH_BR;

    /* Draw the edges */
    for (x = 1; x <= MAP_WID; x++) mc[0][x] = mc[y][x] = CH_HE;
    for (y = 1; y <= MAP_HGT; y++) mc[y][0] = mc[y][x] = CH_VE;

    /* Fill in the map */
    for (i = 0; i < cur_wid; ++i) {
        for (j = 0; j < cur_hgt; ++j) {

            /* Index into mc/ma */
            x = i / RATIO + 1;
            y = j / RATIO + 1;

            /* Extract the current attr/char at that map location */
            map_info(j, i, &m, &ta, &tc);

            /* If this thing is more important, save it instead */
            if (priority[(byte)(mc[y][x])] < priority[(byte)(tc)]) {
                mc[y][x] = tc; ma[y][x] = ta;
            }

            /* Remember where we saw the "player" */
            if ((i == px) && (j == py)) {
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





