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
 * Returns TRUE if a line of sight can be traced from (x1,y1) to (x2,y2).
 *
 * The LOS begins at the center of the tile (x1,y1) and ends at the center of
 * the tile (x2,y2).  If los() is to return TRUE, all of the tiles this line
 * passes through must be floor tiles, except for (x1,y1) and (x2,y2).
 *
 * We assume that the "mathematical corner" of a non-floor tile does not
 * block line of sight.
 *
 * Because this function uses (short) ints for all calculations, overflow may
 * occur if dx and dy exceed 90.
 *
 * Once all the degenerate cases are eliminated, the values "qx", "qy", and
 * "m" are multiplied by a scale factor "f1 = abs(dx * dy * 2)", so that
 * we can use integer arithmetic.
 *
 * We travel from start to finish along the longer axis, starting at the border
 * between the first and second tiles, where the y offset = .5 * slope, taking
 * into account the scale factor.  See below.
 *
 * Also note that this function and the "move towards target" code do NOT
 * share the same properties.  Thus, you can see someone, target them, and
 * then fire a bolt at them, but the bolt may hit a wall, not them.  However,
 * by clever choice of target locations, you can sometimes throw a "curve".
 *
 * Note that "line of sight" is not "reflexive" in all cases.
 *
 * Use the "projectable()" routine to test "spell/missile line of sight".
 *
 * Use the "update_view()" function to determine player line-of-sight.
 */
bool los(int y1, int x1, int y2, int x2)
{
    /* Delta */
    int dx, dy;

    /* Absolute */    
    int ax, ay;

    /* Signs */
    int sx, sy;
        
    /* Fractions */
    int qx, qy;
        
    /* Scanners */
    int tx, ty;
    
    /* Scale factors */
    int f1, f2;

    /* Slope, or 1/Slope, of LOS */
    int m;


    /* Extract the offset */
    dy = y2 - y1;
    dx = x2 - x1;

    /* Extract the absolute offset */
    ay = ABS(dy);
    ax = ABS(dx);


    /* Handle adjacent (or identical) grids */
    if ((ax < 2) && (ay < 2)) return (TRUE);


    /* Paranoia -- require "safe" origin */
    /* if (!in_bounds(y1, x1)) return (FALSE); */


    /* Directly South/North */
    if (!dx)
    {
        /* South -- check for walls */
        if (dy > 0)
        {
            for (ty = y1 + 1; ty < y2; ty++)
            {
                if (!floor_grid_bold(ty,x1)) return (FALSE);
            }
        }

        /* North -- check for walls */
        else
        {
            for (ty = y1 - 1; ty > y2; ty--)
            {
                if (!floor_grid_bold(ty,x1)) return (FALSE);
            }
        }

        /* Assume los */
        return (TRUE);
    }

    /* Directly East/West */
    if (!dy)
    {
        /* East -- check for walls */
        if (dx > 0)
        {
            for (tx = x1 + 1; tx < x2; tx++)
            {
                if (!floor_grid_bold(y1,tx)) return (FALSE);
            }
        }

        /* West -- check for walls */
        else
        {
            for (tx = x1 - 1; tx > x2; tx--)
            {
                if (!floor_grid_bold(y1,tx)) return (FALSE);
            }
        }

        /* Assume los */
        return (TRUE);
    }


    /* Extract some signs */
    sx = (dx < 0) ? -1 : 1;
    sy = (dy < 0) ? -1 : 1;


    /* Vertical "knights" */
    if (ax == 1)
    {
        if (ay == 2)
        {
            if (floor_grid_bold(y1 + sy, x1)) return (TRUE);
        }
    }

    /* Horizontal "knights" */
    else if (ay == 1)
    {
        if (ax == 2)
        {
            if (floor_grid_bold(y1, x1 + sx)) return (TRUE);
        }
    }


    /* Calculate scale factor div 2 */
    f2 = (ax * ay);

    /* Calculate scale factor */
    f1 = f2 << 1;


    /* Travel horizontally */
    if (ax >= ay)
    {
        /* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
        qy = ay * ay;
        m = qy << 1;
        
        tx = x1 + sx;

        /* Consider the special case where slope == 1. */
        if (qy == f2)
        {
            ty = y1 + sy;
            qy -= f1;
        }
        else
        {
            ty = y1;
        }

        /* Note (below) the case (qy == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (x2 - tx)
        {
            if (!floor_grid_bold(ty,tx)) return (FALSE);
            
            qy += m;

            if (qy < f2)
            {
                tx += sx;
            }
            else if (qy > f2)
            {
                ty += sy;
                if (!floor_grid_bold(ty,tx)) return (FALSE);
                qy -= f1;
                tx += sx;
            }
            else
            {
                ty += sy;
                qy -= f1;
                tx += sx;
            }
        }
    }

    /* Travel vertically */
    else
    {
        /* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
        qx = ax * ax;
        m = qx << 1;

        ty = y1 + sy;
        
        if (qx == f2)
        {
            tx = x1 + sx;
            qx -= f1;
        }
        else
        {
            tx = x1;
        }

        /* Note (below) the case (qx == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (y2 - ty)
        {
            if (!floor_grid_bold(ty,tx)) return (FALSE);
            
            qx += m;
            
            if (qx < f2)
            {
                ty += sy;
            }
            else if (qx > f2)
            {
                tx += sx;
                if (!floor_grid_bold(ty,tx)) return (FALSE);
                qx -= f1;
                ty += sy;
            }
            else
            {
                tx += sx;
                qx -= f1;
                ty += sy;
            }
        }
    }

    /* Assume los */
    return (TRUE);
}






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
    if (c_ptr->fdat & CAVE_LITE) return (TRUE);

    /* Require line of sight to the grid */
    if (!player_has_los_bold(y, x)) return (FALSE);

    /* Require "perma-lite" of the grid */
    if (!(c_ptr->fdat & CAVE_GLOW)) return (FALSE);

    /* Hack -- allow "translucent" walls */
    if (optimize_display) return (TRUE);

    /* Assume perma-lit viewable floors are illuminated */
    if (floor_grid_bold(y, x)) return (TRUE);

    /* Mega-Hack -- Prevent memory faults (see above) */
    if (c_ptr->ftyp == 0x3F) return (TRUE);

    /* Hack -- verify walls */
    for (i = 0; i < 8; i++)
    {
        /* Extract adjacent (legal) location */
        int yy = y + ddy_ddd[i];
        int xx = x + ddx_ddd[i];

        /* Check for adjacent perma-lit viewable floor */
        if ((floor_grid_bold(yy, xx)) &&
            (player_has_los_bold(yy, xx)) &&
            (cave[yy][xx].fdat & CAVE_GLOW))
        {
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
 * Hack -- Legal monster codes
 */
static cptr image_monster_hack = \
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
 * Mega-Hack -- Hallucinatory monster
 */
static void image_monster(byte *ap, char *cp)
{
    int i;

    /* Pick a symbol */
    i = rand_int(strlen(image_monster_hack));

    /* Random letter */
    (*cp) = (image_monster_hack[i]);

    /* Random color */
    (*ap) = randint(15);
}


/*
 * Hack -- Legal object codes
 */
static cptr image_object_hack = \
    "?/|\\\"!$()_-=[]{},~";

/*
 * Mega-Hack -- Hallucinatory object
 */
static void image_object(byte *ap, char *cp)
{
    int i;

    /* Pick a symbol */
    i = rand_int(strlen(image_object_hack));

    /* Random letter */
    (*cp) = (image_object_hack[i]);

    /* Random color */
    (*ap) = randint(15);
}


/*
 * Hack -- Random hallucination
 */
static void image_random(byte *ap, char *cp)
{
    /* Normally, assume monsters */
    if (rand_int(100) < 75)
    {
        image_monster(ap, cp);
    }

    /* Otherwise, assume objects */
    else
    {
        image_object(ap, cp);
    }
}



/*
 * Extract the attr/char to display at the given (legal) map location
 *
 * Note the use of "monster zero" for the player attr/char, "object zero"
 * for the "stack" attr/char, and "feature zero" for the "nothing" attr/char.
 *
 * Note that the player can redefine the "hidden treasure" attr/char, and
 * the "secret door" attr/char, in ways that should be considered cheating.
 *
 * Note that monsters can have some "special" flags, including "ATTR_MULTI",
 * which means their color changes, and "ATTR_CLEAR", which means they take
 * the color of whatever is under them, and "CHAR_CLEAR", which means that
 * they take the symbol of whatever is under them.  Technically, the flag
 * "CHAR_MULTI" is supposed to indicate that a monster looks strange when
 * examined, but this flag is currently ignored.
 *
 * Currently, we do nothing with multi-hued objects.  We should probably
 * just add a flag to wearable object, or even to all objects, now that
 * everyone can use the same flags.  Then the "SHIMMER_OBJECT" code can
 * be used to request occasional "redraw" of those objects. It will be
 * very hard to associate flags with the "flavored" objects, so maybe
 * they will never be "multi-hued".
 *
 * Note the effects of hallucination.  Objects always appear as random
 * objects, monsters as random monsters, and normal grids occasionally
 * appear as random monsters or objects.
 *
 * Special colors (used for floors and invisible traps):
 *   Option "view_yellow_lite" draws "torch radius" in yellow
 *   Option "view_bright_lite" draws "unseen grids" dimmer.
 *
 * Note the use of the new "terrain feature" information.  Note that the
 * assumption that all interesting "objects" and "terrain features" are
 * memorized allows extremely optimized processing below.  Note the use
 * of separate flags on objects to mark them as memorized allows a grid
 * to have memorized "terrain" without granting knowledge of any object
 * which may appear in that grid.
 *
 * Note that eventually we may use the "&" symbol for embedded treasure,
 * and use the "*" symbol to indicate multiple objects, though this will
 * have to wait for Angband 2.8.0 or later.  Note that currently, this
 * is not important, since only one object or terrain feature is allowed
 * in each grid.  When it is done, "k_info[0]" will hold the attr/char.
 *
 * Note the efficient code used to determine if a "floor" grid is
 * "memorized" or "viewable" by the player, where the test for the
 * grid being "viewable" is based on the facts that (1) the grid
 * must be "lit" (torch-lit or perma-lit), (2) the grid must be in
 * line of sight, and (3) the player must not be blind, and uses the
 * assumption that all torch-lit grids are in line of sight.
 *
 * Note that the special "lighting" effects only affect "floor" grids
 * (and invisible traps).  This induces some *serious* efficiency.  As
 * a side effect, the code also uses the attr/char codes of "f_info[1]"
 * for both "floors" and "invisible traps", preventing the user from
 * requesting a special visual code for invisible traps.
 *
 * Note that the special "lighting" effects, the "multi-hued" monsters,
 * and the "clear" monsters, are not handled when "use_graphics" is set,
 * since they might cause potential visual glitches.
 *
 * Note the assumption that doing "x_ptr = &x_info[f]" plus a few of
 * "x_ptr->f", is quicker than "x_info[x].f", if this is incorrect
 * then a whole lot of code should be changed...  XXX XXX
 *
 * XXX XXX XXX We need to do something about "use_graphics"
 */
void map_info(int y, int x, byte *ap, char *cp)
{
    cave_type *c_ptr;

    byte tmp_a;
    char tmp_c;


    /* Default to "nothing" */
    if (TRUE)
    {
        feature_type *f_ptr = &f_info[0];

        /* Get the "nothing" char */
        (*cp) = f_ptr->z_char;

        /* Get the "nothing" attr */
        (*ap) = f_ptr->z_attr;
    }


    /* Handle "player" */
    if ((y == py) && (x == px))
    {
        monster_race *r_ptr = &r_info[0];

        /* Get the "player" char */
        (*cp) = r_ptr->l_char;

        /* Get the "player" attr */
        (*ap) = r_ptr->l_attr;

        /* Done */
        return;
    }


    /* Get the cave */
    c_ptr = &cave[y][x];


    /* Handle monsters */
    if (c_ptr->m_idx)
    {
        monster_type *m_ptr = &m_list[c_ptr->m_idx];
        monster_race *r_ptr = &r_info[m_ptr->r_idx];

        /* Invisible monster */
        if (!m_ptr->ml)
        {
            /* Do nothing */
        }

        /* Hack -- hallucination */
        else if (p_ptr->image)
        {
            /* Hallucinatory monster */
            image_monster(ap, cp);

            /* Done */
            return;
        }

        /* Hack -- use_graphics */
        else if (use_graphics)
        {
            /* Normal char */
            (*cp) = r_ptr->l_char;

            /* Normal attr */
            (*ap) = r_ptr->l_attr;

            /* Done */
            return;
        }

#if 0

        /* Hack -- graphics */
        else if (use_graphics && (r_ptr->l_char & 0x80) && (r_ptr->l_attr & 0x80))
        {
            /* Normal char */
            (*cp) = r_ptr->l_char;

            /* Normal attr */
            (*ap) = r_ptr->l_attr;

            /* Done */
            return;
        }

#endif

        /* Hack -- Clear "char" monster */
        else if (r_ptr->flags1 & RF1_CHAR_CLEAR)
        {
            /* Hack -- See below */
            if (!(r_ptr->flags1 & RF1_ATTR_CLEAR))
            {
                /* Normal attr */
                (*ap) = r_ptr->l_attr;

                /* Mega-Hack -- bypass the "floor" attr */
                ap = &tmp_a;
            }
        }

        /* Hack -- Clear "attr" monster */
        else if (r_ptr->flags1 & RF1_ATTR_CLEAR)
        {
            /* Normal char */
            (*cp) = r_ptr->l_char;

            /* Mega-Hack -- bypass the "floor" char */
            cp = &tmp_c;
        }

        /* Hack -- Multi-hued monster */
        else if (r_ptr->flags1 & RF1_ATTR_MULTI)
        {
            /* Normal char */
            (*cp) = r_ptr->l_char;

            /* Multi-hued attr */
            (*ap) = randint(15);

            /* Done */
            return;
        }

        /* Normal */
        else
        {
            /* Normal char */
            (*cp) = r_ptr->l_char;

            /* Normal attr */
            (*ap) = r_ptr->l_attr;

            /* Done */
            return;
        }
    }


    /* Objects */
    if (c_ptr->i_idx)
    {
        /* Get the actual item, if any */
        object_type *i_ptr = &i_list[c_ptr->i_idx];

        /* Memorized objects */
        if (i_ptr->marked)
        {
            /* Hack -- Handle hallucination on "normal" objects */
            if (p_ptr->image)
            {
                /* Hallucinate */
                image_object(ap, cp);

                /* Done */
                return;
            }

            /* Normal char */
            (*cp) = object_char(i_ptr);

            /* Normal attr */
            (*ap) = object_attr(i_ptr);

            /* Done */
            return;
        }
    }


    /* Hack -- rare random hallucination (except outer walls) */
    if (p_ptr->image && (!rand_int(256)) && in_bounds(y,x))
    {
        /* Hallucinate */
        image_random(ap, cp);

        /* Done */
        return;
    }


    /* Handle terrain */
    if (TRUE)
    {
        int f = c_ptr->ftyp;

        /* Interesting terrain feature */
        if (f > 0x02)
        {
            /* Memorized terrain */
            if (c_ptr->fdat & CAVE_MARK)
            {
                feature_type *f_ptr = &f_info[f];

                /* Normal char */
                (*cp) = f_ptr->z_char;

                /* Normal attr */
                (*ap) = f_ptr->z_attr;
            }
        }

        /* Boring terrain feature */
        else
        {
            /* Efficiency -- Viewable floor */
            if ((c_ptr->fdat & CAVE_MARK) ||
                (((c_ptr->fdat & CAVE_LITE) ||
                  ((c_ptr->fdat & CAVE_GLOW) &&
                   (c_ptr->fdat & CAVE_VIEW))) &&
                 (!p_ptr->blind)))
            {
                feature_type *f_ptr = &f_info[1];

                /* Normal "floor" char */
                (*cp) = f_ptr->z_char;

                /* Normal "floor" attr */
                (*ap) = f_ptr->z_attr;

                /* Handle "shadows" */
                if (!use_graphics)
                {
                    /* Hack -- handle "blindness" */
                    if (p_ptr->blind)
                    {
                        /* Option -- Darken "unseen" grids */
                        if (view_bright_lite)
                        {
                            /* Use "gray" attr */
                            (*ap) = TERM_SLATE;
                        }
                    }

                    /* Handle "torch-lit" grids */
                    else if (c_ptr->fdat & CAVE_LITE)
                    {
                        /* Option -- Yellow "torch lite" */
                        if (view_yellow_lite)
                        {
                            /* Use "yellow" attr */
                            (*ap) = TERM_YELLOW;
                        }
                    }

                    /* Handle "un-view-able" grids */
                    else if (!((c_ptr->fdat & CAVE_GLOW) &&
                               (c_ptr->fdat & CAVE_VIEW)))
                    {
                        /* Option -- Darken "unseen" grids */
                        if (view_bright_lite)
                        {
                            /* Use "gray" */
                            (*ap) = TERM_SLATE;
                        }
                    }
                }
            }
        }
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
    if (panel_contains(y, x))
    {

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
 * Memorize the given grid (or object) if it is "interesting"
 *
 * This function should only be called on "legal" grids.
 *
 * This function should be called every time the "memorization" of
 * a grid (or the object in a grid) is called into question.
 *
 * Note that the player always memorized all "objects" which are seen,
 * using a different method than the one used for terrain features,
 * which not only allows a lot of optimization, but also prevents the
 * player from "knowing" when objects are dropped out of sight but in
 * memorized grids.
 *
 * Note that the player always memorizes "interesting" terrain features
 * (everything but floors and invisible traps).  This allows incredible
 * amounts of optimization in various places.
 *
 * Note that the player is allowed to memorize floors and invisible
 * traps under various circumstances, and with various options set.
 *
 * This function is slightly non-optimal, since it memorizes objects
 * and terrain features separately, though both are dependant on the
 * "player_can_see_bold()" macro.
 */
void note_spot(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];


    /* Hack -- memorize objects */
    if (c_ptr->i_idx)
    {
        object_type *i_ptr = &i_list[c_ptr->i_idx];

        /* Only memorize once */
        if (!(i_ptr->marked))
        {
            /* Memorize visible objects */
            if (player_can_see_bold(y, x))
            {
                /* Memorize */
                i_ptr->marked = TRUE;
            }
        }
    }


    /* Hack -- memorize grids */
    if (!(c_ptr->fdat & CAVE_MARK))
    {
        /* Memorize visible grids */
        if (player_can_see_bold(y, x))
        {
            /* Memorize all interesting "terrain features" */
            if (c_ptr->ftyp > 0x02)
            {
                /* Memorize */
                c_ptr->fdat |= CAVE_MARK;
            }

            /* Option -- memorize all perma-lit floors */
            else if (view_perma_grids && (c_ptr->fdat & CAVE_GLOW))
            {
                /* Memorize */
                c_ptr->fdat |= CAVE_MARK;
            }

            /* Option -- memorize all torch-lit floors */
            else if (view_torch_grids && (c_ptr->fdat & CAVE_LITE))
            {
                /* Memorize */
                c_ptr->fdat |= CAVE_MARK;
            }
        }
    }
}


/*
 * Redraw (on the screen) a given MAP location
 */
void lite_spot(int y, int x)
{
    /* Redraw if on screen */
    if (panel_contains(y, x))
    {
        byte a;
        char c;

        /* Examine the contents of that grid */
        map_info(y, x, &a, &c);

#ifdef USE_COLOR
        /* Fake mono-chrome */
        if (!use_color) a = TERM_WHITE;
#else
        /* Always mono-chrome */
        a = TERM_WHITE;
#endif

        /* Efficiency -- immitate "print_rel()" */
        Term_draw(x-panel_col_prt, y-panel_row_prt, a, c);
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

    int v;

    /* Access the cursor state */
    (void)Term_get_cursor(&v);

    /* Hide the cursor */
    (void)Term_set_cursor(0);

    /* Dump the map */
    for (y = panel_row_min; y <= panel_row_max; y++)
    {
        /* Scan the columns of row "y" */
        for (x = panel_col_min; x <= panel_col_max; x++)
        {
            byte a;
            char c;

            /* Determine what is there */
            map_info(y, x, &a, &c);

#ifdef USE_COLOR
            /* Fake mono-chrome */
            if (!use_color) a = TERM_WHITE;
#else
            /* Always mono-chrome */
            a = TERM_WHITE;
#endif

            /* Efficiency -- Redraw that grid of the map */
            Term_draw(x-panel_col_prt, y-panel_row_prt, a, c);
        }
    }

    /* Restore the cursor */
    (void)Term_set_cursor(v);
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
 * Hack -- priority array (see below)
 */
static byte priority_table[][2] =
{
    /* Floors */
    { 0x01, 5 },

    /* Walls */
    { 0x30, 10 },

    /* Quartz */
    { 0x33, 11 },

    /* Magma */
    { 0x32, 12 },

    /* Rubble */
    { 0x31, 13 },

    /* Open doors */
    { 0x05, 15 },
    { 0x04, 15 },

    /* Closed doors */
    { 0x20, 17 },

    /* Hidden gold */
    { 0x37, 19 },
    { 0x36, 19 },

    /* Stairs */
    { 0x06, 25 },
    { 0x07, 25 },

    /* End */
    { 0, 0 }
};


/*
 * Hack -- a priority function
 */
static byte priority(byte a, char c)
{
    byte i;

    feature_type *f_ptr;

    monster_race *r_ptr;


    /* Hack -- Nothing */
    if (c == ' ') return (1);


    /* Access nothing */
    f_ptr = &f_info[0];

    /* Compare to nothing */
    if ((c == f_ptr->z_char) && (a == f_ptr->z_attr)) return (1);


    /* Scan the table */
    for (i = 0; priority_table[i][0]; i++)
    {
        int p1 = priority_table[i][0];
        int p2 = priority_table[i][1];

        /* Access the feature */
        f_ptr = &f_info[p1];

        /* Found a match */
        if ((c == f_ptr->z_char) && (a == f_ptr->z_attr)) return (p2);
    }


    /* Access player */
    r_ptr = &r_info[0];

    /* Compare to player */
    if ((c == r_ptr->l_char) && (a == r_ptr->l_attr)) return (30);


    /* Default */
    return (20);
}


/*
 * Display a "small-scale" map of the dungeon in the active Term
 *
 * Note that we must cancel the "lighting" options during this
 * function or the "priority()" code will not work correctly.
 *
 * Note that the "map_info()" function must return fully colorized
 * data or this function will not work correctly.
 *
 * Note the use of a specialized "priority" function to allow this
 * function to work with any graphic attr/char mappings, and the
 * attempts to optimize this function where possible.
 */
void display_map(void)
{
    int i, j, x, y;

    byte ta;
    char tc;

    byte tp;

    byte ma[MAP_HGT + 2][MAP_WID + 2];
    char mc[MAP_HGT + 2][MAP_WID + 2];

    byte mp[MAP_HGT + 2][MAP_WID + 2];

    bool old_view_yellow_lite = view_yellow_lite;
    bool old_view_bright_lite = view_bright_lite;


    /* Hack -- Cancel the options */
    view_yellow_lite = FALSE;
    view_bright_lite = FALSE;


    /* Clear the chars and attributes */
    for (y = 0; y < MAP_HGT+2; ++y)
    {
        for (x = 0; x < MAP_WID+2; ++x)
        {
            /* Nothing here */
            ma[y][x] = TERM_WHITE;
            mc[y][x] = ' ';

            /* No priority */
            mp[y][x] = 0;
        }
    }

    /* Fill in the map */
    for (i = 0; i < cur_wid; ++i)
    {
        for (j = 0; j < cur_hgt; ++j)
        {
            /* Index into mc/ma */
            x = i / RATIO + 1;
            y = j / RATIO + 1;

            /* Extract the current attr/char at that map location */
            map_info(j, i, &ta, &tc);

            /* Extract the priority of that attr/char */
            tp = priority(ta, tc);

            /* Save "best" */
            if (mp[y][x] < tp)
            {
                /* Save the char */
                mc[y][x] = tc;

                /* Save the attr */
                ma[y][x] = ta;

                /* Save priority */
                mp[y][x] = tp;
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


    /* Display each map line in order */
    for (y = 0; y < MAP_HGT+2; ++y)
    {
        /* Start a new line */
        Term_gotoxy(0, y);

        /* Display the line */
        for (x = 0; x < MAP_WID+2; ++x)
        {
            ta = ma[y][x];
            tc = mc[y][x];

#ifdef USE_COLOR
            /* Fake Monochrome */
            if (!use_color) ta = TERM_WHITE;
#else
            /* Monochrome */
            ta = TERM_WHITE;
#endif

            /* Add the character */
            Term_addch(ta, tc);
        }
    }


    /* Hack -- Restore the options */
    view_yellow_lite = old_view_yellow_lite;
    view_bright_lite = old_view_bright_lite;
}


/*
 * Display a "small-scale" map of the dungeon for the player
 */
void do_cmd_view_map(void)
{
    /* Enter "icky" mode */
    character_icky = TRUE;

    /* Save the screen */
    Term_save();

    /* Note */
    prt("Please wait...", 0, 0);

    /* Flush */
    Term_fresh();

    /* Clear the screen */
    Term_clear();

    /* Display the map */
    display_map();

    /* Wait for it */
    put_str("Hit any key to continue", 23, 23);

    /* Hack -- Hilite the player */
    move_cursor(py / RATIO + 1, px / RATIO + 1);

    /* Get any key */
    inkey();

    /* Restore the screen */
    Term_load();

    /* Leave "icky" mode */
    character_icky = FALSE;
}










/*
 * Some comments on the cave grid flags.  -BEN-
 *
 *
 * One of the major bottlenecks in previous versions of Angband was in
 * the calculation of "line of sight" from the player to various grids,
 * such as monsters.  This was such a nasty bottleneck that a lot of
 * silly things were done to reduce the dependancy on "line of sight",
 * for example, you could not "see" any grids in a lit room until you
 * actually entered the room, and there were all kinds of bizarre grid
 * flags to enable this behavior.  This is also why the "call light"
 * spells always lit an entire room.
 *
 * The code below provides functions to calculate the "field of view"
 * for the player, which, once calculated, provides extremely fast
 * calculation of "line of sight from the player", and to calculate
 * the "field of torch lite", which, again, once calculated, provides
 * extremely fast calculation of "which grids are lit by the player's
 * lite source".  In addition to marking grids as "GRID_VIEW" and/or
 * "GRID_LITE", as appropriate, these functions maintain an array for
 * each of these two flags, each array containing the locations of all
 * of the grids marked with the appropriate flag, which can be used to
 * very quickly scan through all of the grids in a given set.
 *
 * To allow more "semantically valid" field of view semantics, whenever
 * the field of view (or the set of torch lit grids) changes, all of the
 * grids in the field of view (or the set of torch lit grids) are "drawn"
 * so that changes in the world will become apparent as soon as possible.
 * This has been optimized so that only grids which actually "change" are
 * redrawn, using the "temp" array and the "GRID_TEMP" flag to keep track
 * of the grids which are entering or leaving the relevent set of grids.
 *
 * These new methods are so efficient that the old nasty code was removed.
 *
 * Note that there is no reason to "update" the "viewable space" unless
 * the player "moves", or walls/doors are created/destroyed, and there
 * is no reason to "update" the "torch lit grids" unless the field of
 * view changes, or the "light radius" changes.  This means that when
 * the player is resting, or digging, or doing anything that does not
 * involve movement or changing the state of the dungeon, there is no
 * need to update the "view" or the "lite" regions, which is nice.
 *
 * Note that the calls to the nasty "los()" function have been reduced
 * to a bare minimum by the use of the new "field of view" calculations.
 *
 * I wouldn't be surprised if slight modifications to the "update_view()"
 * function would allow us to determine "reverse line-of-sight" as well
 * as "normal line-of-sight", which would allow monsters to use a more
 * "correct" calculation to determine if they can "see" the player.  For
 * now, monsters simply "cheat" somewhat and assume that if the player
 * has "line of sight" to the monster, then the monster can "pretend"
 * that it has "line of sight" to the player.
 *
 *
 * The "update_lite()" function maintains the "CAVE_LITE" flag for each
 * grid and maintains an array of all "CAVE_LITE" grids.
 *
 * This set of grids is the complete set of all grids which are lit by
 * the players light source, which allows the "player_can_see_bold()"
 * function to work very quickly.
 *
 * Note that every "CAVE_LITE" grid is also a "CAVE_VIEW" grid, and in
 * fact, the player (unless blind) can always "see" all grids which are
 * marked as "CAVE_LITE", unless they are "off screen".
 *
 *
 * The "update_view()" function maintains the "CAVE_VIEW" flag for each
 * grid and maintains an array of all "CAVE_VIEW" grids.
 *
 * This set of grids is the complete set of all grids within line of sight
 * of the player, allowing the "player_has_los_bold()" macro to work very
 * quickly.
 *
 *
 * The current "update_view()" algorithm uses the "CAVE_XTRA" flag as a
 * temporary internal flag to mark those grids which are not only in view,
 * but which are also "easily" in line of sight of the player.  This flag
 * is always cleared when we are done.
 *
 *
 * The current "update_lite()" and "update_view()" algorithms use the
 * "CAVE_TEMP" flag, and the array of grids which are marked as "CAVE_TEMP",
 * to keep track of which grids were previously marked as "CAVE_LITE" or
 * "CAVE_VIEW", which allows us to optimize the "screen updates".
 *
 * The "CAVE_TEMP" flag, and the array of "CAVE_TEMP" grids, is also used
 * for various other purposes, such as spreading lite or darkness during
 * "lite_room()" / "unlite_room()", and for calculating monster flow.
 *
 *
 * Any grid can be marked as "CAVE_GLOW" which means that the grid itself is
 * in some way permanently lit.  However, for the player to "see" anything
 * in the grid, as determined by "player_can_see()", the player must not be
 * blind, the grid must be marked as "CAVE_VIEW", and, in addition, "wall"
 * grids, even if marked as "perma lit", are only illuminated if they touch
 * a grid which is not a wall and is marked both "CAVE_GLOW" and "CAVE_VIEW".
 *
 *
 * To simplify various things, a grid may be marked as "CAVE_MARK", meaning
 * that even if the player cannot "see" the grid, he "knows" the terrain in
 * that grid.  This is used to "remember" walls/doors/stairs/floors when they
 * are "seen" or "detected", and also to "memorize" floors, after "wiz_lite()",
 * or when one of the "memorize floor grids" options induces memorization.
 *
 * Objects are "memorized" in a different way, using a special "marked" flag
 * on the object itself, which is set when an object is observed or detected.
 *
 *
 * A grid may be marked as "CAVE_ROOM" which means that it is part of a "room",
 * and should be illuminated by "lite room" and "darkness" spells.
 *
 *
 * A grid may be marked as "CAVE_ICKY" which means it is part of a "vault",
 * and should be unavailable for "teleportation" destinations.
 *
 *
 * The "view_perma_grids" allows the player to "memorize" every perma-lit grid
 * which is observed, and the "view_torch_grids" allows the player to memorize
 * every torch-lit grid.  The player will always memorize important walls,
 * doors, stairs, and other terrain features, as well as any "detected" grids.
 *
 * Note that the new "update_view()" method allows, among other things, a room
 * to be "partially" seen as the player approaches it, with a growing cone of
 * floor appearing as the player gets closer to the door.  Also, by not turning
 * on the "memorize perma-lit grids" option, the player will only "see" those
 * floor grids which are actually in line of sight.
 *
 * And my favorite "plus" is that you can now use a special option to draw the
 * "floors" in the "viewable region" brightly (actually, to draw the *other*
 * grids dimly), providing a "pretty" effect as the player runs around, and
 * to efficiently display the "torch lite" in a special color.
 *
 *
 * Some comments on the "update_view()" algorithm...
 *
 * The algorithm is very fast, since it spreads "obvious" grids very quickly,
 * and only has to call "los()" on the borderline cases.  The major axes/diags
 * even terminate early when they hit walls.  I need to find a quick way
 * to "terminate" the other scans.
 *
 * Note that in the worst case (a big empty area with say 5% scattered walls),
 * each of the 1500 or so nearby grids is checked once, most of them getting
 * an "instant" rating, and only a small portion requiring a call to "los()".
 *
 * The only time that the algorithm appears to be "noticeably" too slow is
 * when running, and this is usually only important in town, since the town
 * provides about the worst scenario possible, with large open regions and
 * a few scattered obstructions.  There is a special "efficiency" option to
 * allow the player to reduce his field of view in town, if needed.
 *
 * In the "best" case (say, a normal stretch of corridor), the algorithm
 * makes one check for each viewable grid, and makes no calls to "los()".
 * So running in corridors is very fast, and if a lot of monsters are
 * nearby, it is much faster than the old methods.
 *
 * Note that resting, most normal commands, and several forms of running,
 * plus all commands executed near large groups of monsters, are strictly
 * more efficient with "update_view()" that with the old "compute los() on
 * demand" method, primarily because once the "field of view" has been
 * calculated, it does not have to be recalculated until the player moves
 * (or a wall or door is created or destroyed).
 *
 * Note that we no longer have to do as many "los()" checks, since once the
 * "view" region has been built, very few things cause it to be "changed"
 * (player movement, and the opening/closing of doors, changes in wall status).
 * Note that door/wall changes are only relevant when the door/wall itself is
 * in the "view" region.
 *
 * The algorithm seems to only call "los()" from zero to ten times, usually
 * only when coming down a corridor into a room, or standing in a room, just
 * misaligned with a corridor.  So if, say, there are five "nearby" monsters,
 * we will be reducing the calls to "los()".
 *
 * I am thinking in terms of an algorithm that "walks" from the central point
 * out to the maximal "distance", at each point, determining the "view" code
 * (above).  For each grid not on a major axis or diagonal, the "view" code
 * depends on the "floor_grid_bold()" and "view" of exactly two other grids
 * (the one along the nearest diagonal, and the one next to that one, see
 * "update_view_aux()"...).
 *
 * We "memorize" the viewable space array, so that at the cost of under 3000
 * bytes, we reduce the time taken by "forget_view()" to one assignment for
 * each grid actually in the "viewable space".  And for another 3000 bytes,
 * we prevent "erase + redraw" ineffiencies via the "seen" set.  These bytes
 * are also used by other routines, thus reducing the cost to almost nothing.
 *
 * A similar thing is done for "forget_lite()" in which case the savings are
 * much less, but save us from doing bizarre maintenance checking.
 *
 * In the worst "normal" case (in the middle of the town), the reachable space
 * actually reaches to more than half of the largest possible "circle" of view,
 * or about 800 grids, and in the worse case (in the middle of a dungeon level
 * where all the walls have been removed), the reachable space actually reaches
 * the theoretical maximum size of just under 1500 grids.
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
 * Actually erase the entire "lite" array, redrawing every grid
 */
void forget_lite(void)
{
    int i, x, y;

    /* None to forget */
    if (!lite_n) return;

    /* Clear them all */
    for (i = 0; i < lite_n; i++)
    {
        y = lite_y[i];
        x = lite_x[i];

        /* Forget "LITE" flag */
        cave[y][x].fdat &= ~CAVE_LITE;

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    lite_n = 0;
}


/*
 * XXX XXX XXX
 *
 * This macro allows us to efficiently add a grid to the "lite" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "lite" array, and we are never
 * called when the "lite" array is full.
 */
#define cave_lite_hack(Y,X) \
    cave[Y][X].fdat |= CAVE_LITE; \
    lite_y[lite_n] = (Y); \
    lite_x[lite_n] = (X); \
    lite_n++



/*
 * Update the set of grids "illuminated" by the player's lite.
 *
 * This routine needs to use the results of "update_view()"
 *
 * Note that "blindness" does NOT affect "torch lite".  Be careful!
 *
 * We optimize most lites (all non-artifact lites) by using "obvious"
 * facts about the results of "small" lite radius, and we attempt to
 * list the "nearby" grids before the more "distant" ones in the
 * array of torch-lit grids.
 *
 * We will correctly handle "large" radius lites, though currently,
 * it is impossible for the player to have more than radius 3 lite.
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


    /*** Special case ***/

    /* Hack -- Player has no lite */
    if (p_ptr->cur_lite <= 0)
    {
        /* Forget the old lite */
        forget_lite();

        /* Draw the player */
        lite_spot(py, px);

        /* All done */
        return;
    }


    /*** Save the old "lite" grids for later ***/

    /* Clear them all */
    for (i = 0; i < lite_n; i++)
    {
        y = lite_y[i];
        x = lite_x[i];

        /* Mark the grid as not "lite" */
        cave[y][x].fdat &= ~CAVE_LITE;

        /* Mark the grid as "seen" */
        cave[y][x].fdat |= CAVE_TEMP;

        /* Add it to the "seen" set */
        temp_y[temp_n] = y;
        temp_x[temp_n] = x;
        temp_n++;
    }

    /* None left */
    lite_n = 0;


    /*** Collect the new "lite" grids ***/

    /* Player grid */
    cave_lite_hack(py, px);
    
    /* Radius 1 -- torch radius */
    if (p_ptr->cur_lite >= 1)
    {
        /* Adjacent grid */
        cave_lite_hack(py+1, px);
        cave_lite_hack(py-1, px);
        cave_lite_hack(py, px+1);
        cave_lite_hack(py, px-1);

        /* Diagonal grids */
        cave_lite_hack(py+1, px+1);
        cave_lite_hack(py+1, px-1);
        cave_lite_hack(py-1, px+1);
        cave_lite_hack(py-1, px-1);
    }

    /* Radius 2 -- lantern radius */
    if (p_ptr->cur_lite >= 2)
    {
        /* South of the player */
        if (floor_grid_bold(py+1, px))
        {
            cave_lite_hack(py+2, px);
            cave_lite_hack(py+2, px+1);
            cave_lite_hack(py+2, px-1);
        }

        /* North of the player */
        if (floor_grid_bold(py-1, px))
        {
            cave_lite_hack(py-2, px);
            cave_lite_hack(py-2, px+1);
            cave_lite_hack(py-2, px-1);
        }

        /* East of the player */
        if (floor_grid_bold(py, px+1))
        {
            cave_lite_hack(py, px+2);
            cave_lite_hack(py+1, px+2);
            cave_lite_hack(py-1, px+2);
        }

        /* West of the player */
        if (floor_grid_bold(py, px-1))
        {
            cave_lite_hack(py, px-2);
            cave_lite_hack(py+1, px-2);
            cave_lite_hack(py-1, px-2);
        }
    }

    /* Radius 3+ -- artifact radius */
    if (p_ptr->cur_lite >= 3)
    {
        int d, p;
        
        /* Maximal radius */
        p = p_ptr->cur_lite;
        
        /* Paranoia -- see "LITE_MAX" */
        if (p > 5) p = 5;
        
        /* South-East of the player */
        if (floor_grid_bold(py+1, px+1))
        {
            cave_lite_hack(py+2, px+2);
        }

        /* South-West of the player */
        if (floor_grid_bold(py+1, px-1))
        {
            cave_lite_hack(py+2, px-2);
        }

        /* North-East of the player */
        if (floor_grid_bold(py-1, px+1))
        {
            cave_lite_hack(py-2, px+2);
        }

        /* North-West of the player */
        if (floor_grid_bold(py-1, px-1))
        {
            cave_lite_hack(py-2, px-2);
        }

        /* Maximal north */
        min_y = py - p;
        if (min_y < 0) min_y = 0;

        /* Maximal south */
        max_y = py + p;
        if (max_y > cur_hgt-1) max_y = cur_hgt-1;

        /* Maximal west */
        min_x = px - p;
        if (min_x < 0) min_x = 0;
        
        /* Maximal east */
        max_x = px + p;
        if (max_x > cur_wid-1) max_x = cur_wid-1;

        /* Scan the maximal box */
        for (y = min_y; y <= max_y; y++)
        {
            for (x = min_x; x <= max_x; x++)
            {
                int dy = (py > y) ? (py - y) : (y - py);
                int dx = (px > x) ? (px - x) : (x - px);

                /* Skip the "central" grids (above) */
                if ((dy <= 2) && (dx <= 2)) continue;

                /* Hack -- approximate the distance */
                d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

                /* Skip distant grids */
                if (d > p) continue;

                /* Viewable, nearby, grids get "torch lit" */
                if (player_has_los_bold(y, x))
                {
                    /* This grid is "torch lit" */
                    cave_lite_hack(y, x);
                }
            }
        }
    }


    /*** Complete the algorithm ***/

    /* Draw the new grids */
    for (i = 0; i < lite_n; i++)
    {
        y = lite_y[i];
        x = lite_x[i];

        /* Update fresh grids */
        if (cave[y][x].fdat & CAVE_TEMP) continue;

        /* Note */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }

    /* Clear them all */
    for (i = 0; i < temp_n; i++)
    {
        y = temp_y[i];
        x = temp_x[i];

        /* No longer in the array */
        cave[y][x].fdat &= ~CAVE_TEMP;

        /* Update stale grids */
        if (cave[y][x].fdat & CAVE_LITE) continue;

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
}







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
    for (i = 0; i < view_n; i++)
    {
        int y = view_y[i];
        int x = view_x[i];

        /* Access the grid */
        c_ptr = &cave[y][x];

        /* Forget that the grid is viewable */
        c_ptr->fdat &= ~CAVE_VIEW;

        /* Update the screen */
        lite_spot(y, x);
    }

    /* None left */
    view_n = 0;
}



/*
 * Hack -- Local version of "floor_grid_bold(Y,X)"
 */
#define floor_grid_hack(C) \
    (!((C)->ftyp & 0x20))

/*
 * This macro allows us to efficiently add a grid to the "view" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "view" array, and we are never
 * called when the "view" array is full.
 */
#define cave_view_hack(C,Y,X) \
    (C)->fdat |= CAVE_VIEW; \
    view_y[view_n] = (Y); \
    view_x[view_n] = (X); \
    view_n++



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
    v1 = (f1 && (g1_c_ptr->fdat & CAVE_VIEW));
    v2 = (f2 && (g2_c_ptr->fdat & CAVE_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);


    /* Access the grid */
    c_ptr = &cave[y][x];


    /* Check for walls */
    wall = (!floor_grid_hack(c_ptr));


    /* Check the "ease" of visibility */
    z1 = (v1 && (g1_c_ptr->fdat & CAVE_XTRA));
    z2 = (v2 && (g2_c_ptr->fdat & CAVE_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (z1 && z2)
    {
        c_ptr->fdat |= CAVE_XTRA;

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (z1)
    {
        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2)
    {
        /* c_ptr->fdat |= CAVE_XTRA; */

        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Mega-Hack -- the "los()" function works poorly on walls */
    if (wall)
    {
        cave_view_hack(c_ptr, y, x);

        return (wall);
    }


    /* Hack -- check line of sight */
    if (los(py, px, y, x))
    {
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
    int n, m, d, k, y, x, z;

    int se, sw, ne, nw, es, en, ws, wn;

    int full, over;

    cave_type *c_ptr;


    /*** Initialize ***/

    /* Optimize */
    if (view_reduce_view && !dun_level)
    {
        /* Full radius (10) */
        full = MAX_SIGHT / 2;

        /* Octagon factor (15) */
        over = MAX_SIGHT * 3 / 4;
    }

    /* Normal */
    else
    {
        /* Full radius (20) */
        full = MAX_SIGHT;

        /* Octagon factor (30) */
        over = MAX_SIGHT * 3 / 2;
    }


    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < view_n; n++)
    {
        y = view_y[n];
        x = view_x[n];

        /* Access the grid */
        c_ptr = &cave[y][x];

        /* Mark the grid as not in "view" */
        c_ptr->fdat &= ~(CAVE_VIEW);

        /* Mark the grid as "seen" */
        c_ptr->fdat |= CAVE_TEMP;

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
    c_ptr->fdat |= CAVE_XTRA;

    /* Assume the player grid is viewable */
    cave_view_hack(c_ptr, y, x);


    /*** Step 2 -- Major Diagonals ***/

    /* Hack -- Limit */
    z = full * 2 / 3;

    /* Scan south-east */
    for (d = 1; d <= z; d++)
    {
        c_ptr = &cave[y+d][x+d];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y+d, x+d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Scan south-west */
    for (d = 1; d <= z; d++)
    {
        c_ptr = &cave[y+d][x-d];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y+d, x-d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Scan north-east */
    for (d = 1; d <= z; d++)
    {
        c_ptr = &cave[y-d][x+d];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y-d, x+d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Scan north-west */
    for (d = 1; d <= z; d++)
    {
        c_ptr = &cave[y-d][x-d];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y-d, x-d);
        if (!floor_grid_hack(c_ptr)) break;
    }


    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++)
    {
        c_ptr = &cave[y+d][x];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y+d, x);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++)
    {
        c_ptr = &cave[y-d][x];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y-d, x);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++)
    {
        c_ptr = &cave[y][x+d];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y, x+d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++)
    {
        c_ptr = &cave[y][x-d];
        c_ptr->fdat |= CAVE_XTRA;
        cave_view_hack(c_ptr, y, x-d);
        if (!floor_grid_hack(c_ptr)) break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;


    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++)
    {
        int ypn, ymn, xpn, xmn;


        /* Acquire the "bounds" of the maximal circle */
        z = over - n - n;
        if (z > full - n) z = full - n;	
        while ((z + n + (n>>1)) > full) z--;


        /* Access the four diagonal grids */
        ypn = y + n;
        ymn = y - n;
        xpn = x + n;
        xmn = x - n;


        /* South strip */
        if (ypn < cur_hgt-1)
        {
            /* Maximum distance */
            m = MIN(z, (cur_hgt-1) - ypn);

            /* East side */
            if ((xpn <= cur_wid-1) && (n < se))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn+d,xpn,ypn+d-1,xpn-1,ypn+d-1,xpn))
                    {
                        if (n + d >= se) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                se = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < sw))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn+d,xmn,ypn+d-1,xmn+1,ypn+d-1,xmn))
                    {
                        if (n + d >= sw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                sw = k + 1;
            }
        }


        /* North strip */
        if (ymn > 0)
        {
            /* Maximum distance */
            m = MIN(z, ymn);

            /* East side */
            if ((xpn <= cur_wid-1) && (n < ne))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn-d,xpn,ymn-d+1,xpn-1,ymn-d+1,xpn))
                    {
                        if (n + d >= ne) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ne = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < nw))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn-d,xmn,ymn-d+1,xmn+1,ymn-d+1,xmn))
                    {
                        if (n + d >= nw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                nw = k + 1;
            }
        }


        /* East strip */
        if (xpn < cur_wid-1)
        {
            /* Maximum distance */
            m = MIN(z, (cur_wid-1) - xpn);

            /* South side */
            if ((ypn <= cur_hgt-1) && (n < es))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn,xpn+d,ypn-1,xpn+d-1,ypn,xpn+d-1))
                    {
                        if (n + d >= es) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                es = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < en))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn,xpn+d,ymn+1,xpn+d-1,ymn,xpn+d-1))
                    {
                        if (n + d >= en) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                en = k + 1;
            }
        }


        /* West strip */
        if (xmn > 0)
        {
            /* Maximum distance */
            m = MIN(z, xmn);

            /* South side */
            if ((ypn <= cur_hgt-1) && (n < ws))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ypn,xmn-d,ypn-1,xmn-d+1,ypn,xmn-d+1))
                    {
                        if (n + d >= ws) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ws = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < wn))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (update_view_aux(ymn,xmn-d,ymn+1,xmn-d+1,ymn,xmn-d+1))
                    {
                        if (n + d >= wn) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
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
    for (n = 0; n < view_n; n++)
    {
        y = view_y[n];
        x = view_x[n];

        /* Access the grid */
        c_ptr = &cave[y][x];

        /* Clear the "CAVE_XTRA" flag */
        c_ptr->fdat &= ~CAVE_XTRA;

        /* Update only newly viewed grids */
        if (c_ptr->fdat & CAVE_TEMP) continue;

        /* Note */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }

    /* Wipe the old grids, update as needed */
    for (n = 0; n < temp_n; n++)
    {
        y = temp_y[n];
        x = temp_x[n];

        /* Access the grid */
        c_ptr = &cave[y][x];

        /* No longer in the array */
        c_ptr->fdat &= ~CAVE_TEMP;

        /* Update only non-viewable grids */
        if (c_ptr->fdat & CAVE_VIEW) continue;

        /* Redraw */
        lite_spot(y, x);
    }

    /* None left */
    temp_n = 0;
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
    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
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
    if (c_ptr->ftyp >= 0x31) return;

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
    if (flow_n == 255)
    {
        /* Rotate the time-stamps */
        for (y = 0; y < cur_hgt; y++)
        {
            for (x = 0; x < cur_wid; x++)
            {
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
    while (flow_head != flow_tail)
    {
        /* Extract the next entry */
        y = temp_y[flow_tail];
        x = temp_x[flow_tail];

        /* Forget that entry */
        if (++flow_tail == TEMP_MAX) flow_tail = 0;

        /* Add the "children" */
        for (d = 0; d < 8; d++)
        {
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
    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            c_ptr = &cave[y][x];

            /* All non-walls are "checked" */
            if (c_ptr->ftyp < 0x30)
            {
                /* Memorize landmarks */
                if (c_ptr->ftyp > 0x02)
                {
                    /* Memorize the object */
                    c_ptr->fdat |= CAVE_MARK;
                }

                /* Memorize "useful" walls */
                for (i = 0; i < 8; i++)
                {
                    c_ptr = &cave[y+ddy_ddd[i]][x+ddx_ddd[i]];

                    /* Memorize the "interesting" walls */
                    if (c_ptr->ftyp >= 0x30)
                    {
                        /* Memorize the walls */
                        c_ptr->fdat |= CAVE_MARK;
                    }
                }
            }
        }
    }

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);
}



/*
 * Light up the dungeon.
 *
 * XXX XXX XXX Hack -- This function is basically a hack.
 */
void wiz_lite(void)
{
    int		yy, xx, y, x;

    cave_type	*c_ptr;


    /* Perma-light all open space and adjacent walls */
    for (y = 1; y < cur_hgt-1; y++)
    {
        for (x = 1; x < cur_wid-1; x++)
        {
            /* Access the grid */
            c_ptr = &cave[y][x];

            /* XXX XXX XXX Memorize all objects */
            if (c_ptr->i_idx)
            {
                object_type *i_ptr = &i_list[c_ptr->i_idx];

                /* Memorize */
                i_ptr->marked = TRUE;
            }

            /* Process all non-walls */
            if (c_ptr->ftyp < 0x30)
            {
                /* Perma-lite all grids touching those grids */
                for (yy = y - 1; yy <= y + 1; yy++)
                {
                    for (xx = x - 1; xx <= x + 1; xx++)
                    {
                        /* Get the grid */
                        c_ptr = &cave[yy][xx];

                        /* Perma-lite the grid */
                        c_ptr->fdat |= (CAVE_GLOW);

                        /* XXX XXX XXX Hack -- memorize landmarks */
                        if (c_ptr->ftyp > 0x02)
                        {
                            c_ptr->fdat |= CAVE_MARK;
                        }

                        /* XXX XXX XXX Hack -- memorize if requested */
                        if (view_perma_grids) c_ptr->fdat |= CAVE_MARK;
                    }
                }
            }
        }
    }

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);
}


/*
 * Forget the dungeon map (ala "Thinking of Maud...").
 */
void wiz_dark(void)
{
    int        y, x;


    /* Forget every grid */
    for (y = 0; y < cur_hgt; y++)
    {
        for (x = 0; x < cur_wid; x++)
        {
            cave_type *c_ptr = &cave[y][x];

            /* Process the grid */
            c_ptr->fdat &= ~CAVE_MARK;

            /* Forget the object */
            if (c_ptr->i_idx)
            {
                object_type *i_ptr = &i_list[c_ptr->i_idx];

                /* Forget the object */
                i_ptr->marked = FALSE;
            }
        }
    }

    /* Mega-Hack -- Forget the view and lite */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update the view and lite */
    p_ptr->update |= (PU_VIEW | PU_LITE);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw map */
    p_ptr->redraw |= (PR_MAP | PR_AROUND);
}





/*
 * Calculate "incremental motion". Used by project() and shoot().
 * Assumes that (*y,*x) lies on the path from (y1,x1) to (y2,x2).
 */
void mmove2(int *y, int *x, int y1, int x1, int y2, int x2)
{
    int dy, dx, dist, shift;

    /* Extract the distance travelled */
    dy = (*y < y1) ? y1 - *y : *y - y1;
    dx = (*x < x1) ? x1 - *x : *x - x1;
    
    /* Number of steps */
    dist = (dy > dx) ? dy : dx;

    /* We are calculating the next location */
    dist++;


    /* Calculate the total distance along each axis */
    dy = (y2 < y1) ? (y1 - y2) : (y2 - y1);
    dx = (x2 < x1) ? (x1 - x2) : (x2 - x1);

    /* Paranoia -- Hack -- no motion */
    if (!dy && !dx) return;


    /* Move mostly vertically */
    if (dy > dx)
    {

#if 0

        int k;

        /* Starting shift factor */
        shift = dy >> 1;

        /* Extract a shift factor */
        for (k = 0; k < dist; k++)
        {
            if (shift <= 0) shift += dy;
            shift -= dx;
        }

        /* Sometimes move along minor axis */
        if (shift <= 0) (*x) = (x2 < x1) ? (*x - 1) : (*x + 1);

        /* Always move along major axis */
        (*y) = (y2 < y1) ? (*y - 1) : (*y + 1);

#endif

        /* Extract a shift factor */
        shift = (dist * dx + (dy-1) / 2) / dy;

        /* Sometimes move along the minor axis */
        (*x) = (x2 < x1) ? (x1 - shift) : (x1 + shift);

        /* Always move along major axis */
        (*y) = (y2 < y1) ? (y1 - dist) : (y1 + dist);
    }

    /* Move mostly horizontally */
    else
    {

#if 0

        int k;

        /* Starting shift factor */
        shift = dx >> 1;

        /* Extract a shift factor */
        for (k = 0; k < dist; k++)
        {
            if (shift <= 0) shift += dx;
            shift -= dy;
        }

        /* Sometimes move along minor axis */
        if (shift <= 0) (*y) = (y2 < y1) ? (*y - 1) : (*y + 1);

        /* Always move along major axis */
        (*x) = (x2 < x1) ? (*x - 1) : (*x + 1);

#endif

        /* Extract a shift factor */
        shift = (dist * dy + (dx-1) / 2) / dx;

        /* Sometimes move along the minor axis */
        (*y) = (y2 < y1) ? (y1 - shift) : (y1 + shift);

        /* Always move along major axis */
        (*x) = (x2 < x1) ? (x1 - dist) : (x1 + dist);
    }
}


/*
 * Determine if a bolt spell cast from (y1,x1) to (y2,x2) will arrive
 * at the final destination, assuming no monster gets in the way.
 *
 * This is slightly (but significantly) different from "los(y1,x1,y2,x2)".
 */
bool projectable(int y1, int x1, int y2, int x2)
{
    int dist, y, x;

    /* Start at the initial location */
    y = y1, x = x1;

    /* See "project()" */
    for (dist = 0; dist < MAX_RANGE; dist++)
    {
        /* Never pass through walls */
        if (dist && !floor_grid_bold(y, x)) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }


    /* Assume obstruction */
    return (FALSE);
}



/*
 * Standard "find me a location" function
 *
 * Obtains a legal location within the given distance of the initial
 * location, and with "los()" from the source to destination location
 *
 * This function is often called from inside a loop which searches for
 * locations while increasing the "d" distance.
 *
 * Currently the "m" parameter is unused.
 */
void scatter(int *yp, int *xp, int y, int x, int d, int m)
{
    int nx, ny;

    /* Unused */
    m = m;

    /* Pick a location */
    while (TRUE)
    {
        /* Pick a new location */
        ny = rand_spread(y, d);
        nx = rand_spread(x, d);

        /* Ignore illegal locations and outer walls */
        if (!in_bounds(y, x)) continue;

        /* Ignore "excessively distant" locations */
        if ((d > 1) && (distance(y, x, ny, nx) > d)) continue;

        /* Require "line of sight" */
        if (los(y, x, ny, nx)) break;
    }

    /* Save the location */
    (*yp) = ny;
    (*xp) = nx;
}




/*
 * Track a new monster
 */
void health_track(int m_idx)
{
    /* Track a new guy */
    health_who = m_idx;

    /* Redraw (later) */
    p_ptr->redraw |= (PR_HEALTH);
}



/*
 * Hack -- track the given monster race
 */
void recent_track(int r_idx)
{
    /* Save this monster ID */
    recent_idx = r_idx;

    /* Update later */
    p_ptr->redraw |= (PR_RECENT);
}




/*
 * Something has happened to disturb the player.
 *
 * The first arg indicates a major disturbance, which affects search.
 *
 * The second arg is currently unused, but could induce output flush.
 *
 * All disturbance cancels repeated commands, resting, and running.
 */
void disturb(int stop_search, int unused_flag)
{
    /* Unused */
    unused_flag = unused_flag;

    /* Cancel auto-commands */
    /* command_new = 0; */

    /* Cancel repeated commands */
    if (command_rep)
    {
        /* Cancel */
        command_rep = 0;

        /* Redraw the state (later) */
        p_ptr->redraw |= (PR_STATE);
    }

    /* Cancel Resting */
    if (resting)
    {
        /* Cancel */
        resting = 0;

        /* Redraw the state (later) */
        p_ptr->redraw |= (PR_STATE);
    }

    /* Cancel running */
    if (running)
    {
        /* Cancel */
        running = 0;

        /* Calculate torch radius */
        p_ptr->update |= (PU_TORCH);
    }

    /* Cancel searching if requested */
    if (stop_search && p_ptr->searching)
    {
        /* Cancel */
        p_ptr->searching = FALSE;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redraw the state */
        p_ptr->redraw |= (PR_STATE);
    }

    /* Flush the input if requested */
    if (flush_disturb) flush();
}





/*
 * Hack -- Check if a level is a "quest" level
 */
bool is_quest(int level)
{
    int i;

    /* Town is never a quest */
    if (!level) return (FALSE);

    /* Check quests */
    for (i = 0; i < MAX_Q_IDX; i++)
    {
        /* Check for quest */
        if (q_list[i].level == level) return (TRUE);
    }

    /* Nope */
    return (FALSE);
}



