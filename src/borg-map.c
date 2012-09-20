/* File: borg-map.c */

/* Purpose: Helper file for "borg-ben.c" -BEN- */

#include "angband.h"


#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-map.h"


/*
 * See "borg-ben.c" for general information.
 *
 * This file helps the Borg understand mapping the dungeon.
 *
 * Currently, this includes general routines involving dungeon grids,
 * such as analyzing the "map" part of the screen, calculating the "flow"
 * code from place to place, and determining "line of sight".
 *
 * Note that the dungeon is assumed smaller than 256 by 256.
 *
 * Note that the "BORG_WALL" flag is extracted from a variety of sources,
 * and with a variety of assumptions.  For example, we ignore the fact
 * that some monsters "destroy" walls.
 *
 * This file also supplies the (compiled out) support for "automatic
 * room extraction".  This code will automatically group regions of
 * the dungeon into rooms, and do the "flow" navigation on those rooms
 * instead of on grids.  Often, this takes less space, and is faster,
 * howver, it is more complicated, and does not allow "specialized"
 * flow calculations that penalize grids by variable amounts.
 */



/*
 * Some variables
 */

auto_grid **auto_grids;		/* The grids */

auto_data *auto_data_flow;	/* Current "flow" data */

auto_data *auto_data_cost;	/* Current "cost" data */


/*
 * Some local variables
 */

auto_data *auto_data_hard;	/* Constant "hard" data */

auto_data *auto_data_know;	/* Current "know" flags */

auto_data *auto_data_icky;	/* Current "icky" flags */


/*
 * Maintain a set of grids marked as "BORG_VIEW"
 */

s16b auto_view_n = 0;

byte *auto_view_x;
byte *auto_view_y;


/*
 * Maintain a set of grids marked as "BORG_SEEN"
 */

s16b auto_temp_n = 0;

byte *auto_temp_x;
byte *auto_temp_y;


/*
 * Maintain a circular queue of grids
 */

s16b auto_flow_n = 0;

byte *auto_flow_x;
byte *auto_flow_y;



/*
 * Hack -- use "flow" array as a queue
 */

static int flow_head = 0;
static int flow_tail = 0;



/*
 * External flag -- clear danger information
 */
bool auto_danger_wipe = FALSE;

/*
 * External Hook -- Check danger
 */
bool (*auto_danger_hook)(int x, int y) = NULL;




#ifdef BORG_ROOMS



/*
 * Some "map" related variables
 */

int auto_room_max = 0;		/* First totally free room */

auto_room *auto_rooms;		/* Current "room list" */

auto_room *auto_room_head;	/* &auto_rooms[0] */

auto_room *auto_room_tail;	/* &auto_rooms[AUTO_MAX_ROOMS-1] */




/*
 * Hack -- access the "set" of rooms containing a given point
 *
 * We should probably build a "list" of "used rooms", where the "tail"
 * of that list is all the "crossed rooms", and also keep a pointer to
 * that tail for "fast" access to the "crossed rooms" set.
 *
 * This function is necessary because some grids (though not many)
 * can be contained in multiple rooms, because we allow "crossing"
 * rooms.  The Borg uses crossing rooms not only for actual "cross"
 * and "overlap" rooms in the dungeon, but also weird double width
 * corridors, the corners of "ragged edge" rooms, and various regions
 * in the town.  Actually, the worst "offenders" come from the town...
 *
 * Note that this function should be valid even if rooms are destroyed
 * inside a single "search" through the room list.  This is because
 * only the "first" call to this function checks the "room" field.
 */
auto_room *room(bool go, int gx, int gy)
{
    static int x = 0, y = 0, i = 0;

    /* We just got a new grid */
    if (go) {

        auto_grid *ag = grid(gx,gy);

        /* Default to no rooms */
        i = AUTO_ROOMS;

        /* Paranoia -- no rooms */
        if (!ag->room) return (NULL);

        /* Efficiency -- Single room */
        if (ag->room < AUTO_ROOMS) return (&auto_rooms[ag->room]);

        /* Scan through multiple rooms */
        x = gx; y = gy; i = 0;
    }

    /* Look for a room */
    for (i++; i < auto_room_max; i++) {

        /* Access the room */
        auto_room *ar = &auto_rooms[i];

        /* Skip "free" rooms */
        if (ar->free) continue;

        /* If the room contains it, use it */
        if ((x >= ar->x1) && (x <= ar->x2) &&
            (y >= ar->y1) && (y <= ar->y2)) {
            return (ar);
        }
    }

    /* Default */
    return (NULL);
}






/*
 * Mega-Hack -- clean up the "free room" list
 * First, destroy some of the "fake" rooms
 * Then, attempt to compress the list
 */
bool borg_free_room_update(void)
{
    int x, y, n = 0;

    auto_grid *ag;
    auto_room *ar;


    /* Scan the dungeon */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {

            /* Extract the "grid" */
            ag = grid(x, y);

            /* Skip "real" grids */
            if (ag->o_c != ' ') continue;

            /* Skip "viewed" grids */
            if (ag->info & BORG_VIEW) continue;

            /* Only process "roomed" grids */
            if (!ag->room) continue;

            /* Paranoia -- skip cross rooms */
            if (ag->room >= AUTO_ROOMS) continue;

            /* Get the (solitary) room */
            ar = &auto_rooms[ag->room];

            /* Paranoia -- Skip "big" rooms */
            if ((ar->x1 < x) || (ar->x2 > x)) continue;
            if ((ar->y1 < y) || (ar->y2 > y)) continue;

            /* That room is now "gone" */
            ar->when = 0L;
            ar->x1 = ar->x2 = 0;
            ar->y1 = ar->y2 = 0;

            /* Add it to the "free list" */
            auto_rooms[ag->room].free = auto_rooms[0].free;

            /* Drop the room into the "free list" */
            auto_rooms[0].free = ag->room;

            /* No room here */
            ag->room = 0;

            /* Count changes */
            n++;
        }
    }

    /* Nothing done */
    if (!n) return (FALSE);


    /* Message */
    borg_note(format("# Destroyed %d fake rooms.", n));

    /* Rooms destroyed */
    return (TRUE);
}



/*
 * Mega-Hack -- purge the "free room" list
 * Could be a little more "clever", I suppose
 * For example, first nuke all rooms not in view
 */
static bool borg_free_room_purge(void)
{
    int x, y, n;


    /* Purge every room */
    for (n = 1; n < auto_room_max; n++) {

        auto_room *ar = &auto_rooms[n];

        /* That room is now "gone" */
        ar->when = 0L;
        ar->x1 = ar->x2 = 0;
        ar->y1 = ar->y2 = 0;

        /* Reset the "free list" */
        ar->free = n + 1;
    }

    /* Reset the free list */
    auto_room_head->free = 1;

    /* Maximum room index */
    auto_room_max = 1;


    /* Scan the dungeon */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {

            /* Extract the "grid" */
            auto_grid *ag = grid(x, y);

            /* No room here */
            ag->room = 0;
        }
    }


    /* Message */
    borg_note(format("# Purged all %d rooms.", n));

    /* Rooms destroyed */
    return (TRUE);
}



/*
 * Grab a room from the free list and return it
 */
auto_room *borg_free_room(void)
{
    int i;

    auto_room *ar;


    /* Running out of free rooms */
    if (auto_room_head->free == auto_room_tail->self) {

        /* Message */
        borg_note("# Updating room list!");

        /* Try to free some rooms */
        if (!borg_free_room_update()) {

            /* Oops */
            borg_note("# Purging room list!");

            /* Try to free some rooms */
            if (!borg_free_room_purge()) {

                /* Oops */
                borg_oops("broken rooms");

                /* Hack -- Attempt to prevent core dumps */
                return (&auto_rooms[1]);
            }
        }
    }


    /* Acquire a "free" room */
    i = auto_room_head->free;

    /* Access the new room */
    ar = &auto_rooms[i];

    /* Remove the room from the free list */
    auto_room_head->free = ar->free;

    /* Take note of new "maximum" room index */
    if (auto_room_max < i + 1) auto_room_max = i + 1;

    /* The new room is not free */
    ar->free = 0;

    /* Paranoia */
    ar->when = 0L;
    ar->x1 = ar->x1 = 0;
    ar->y1 = ar->y2 = 0;

    /* Return the room */
    return (ar);
}





/*
 * Determine if the given grid is a "snake" grid
 * A "snake" is a "normal" section of corridor, with no
 * bends or intersections.  Only the "center" counts.
 * Note that a "1x1" grid (or "diagonal" grid) is a "snake".
 * A "snake" grid cannot touch "unknown" grids.
 */
static bool borg_build_snake(int x, int y)
{
    auto_grid *ag, *ag1, *ag2;

    /* Access the center */
    ag = grid(x,y);

    /* Central grid cannot be unknown */
    if (ag->o_c == ' ') return (FALSE);

    /* Central grid must be a non-wall */
    if (ag->info & BORG_WALL) return (FALSE);

    /* South/North blockage induces a snake */
    ag1 = grid(x,y+1); ag2 = grid(x,y-1);
    if ((ag1->info & BORG_WALL) && (ag2->info & BORG_WALL)) return (TRUE);

    /* East/West blockage induces a snake */
    ag1 = grid(x+1,y); ag2 = grid(x-1,y);
    if ((ag1->info & BORG_WALL) && (ag2->info & BORG_WALL)) return (TRUE);

    /* No good */
    return (FALSE);
}



/*
 * Determine if the given "box" in the world is fully made of "floor"
 * Currently, we refuse to accept "unknown" grids as "floor" grids
 */
static bool borg_build_room_floor(int x1, int y1, int x2, int y2)
{
    int x, y;

    /* Check for "floors" */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            auto_room *ar;

            /* Access that grid */
            auto_grid *ag = grid(x,y);

            /* Refuse to accept walls */
            if (ag->info & BORG_WALL) return (FALSE);

            /* Refuse to accept unknown grids */
            if (ag->o_c == ' ') return (FALSE);

            /* Mega-Hack -- Refuse to accept stores */
            if (isdigit(ag->o_c)) return (FALSE);

#ifndef CROSS_ROOMS

            /* Hack -- Do not "cross" other (large) rooms */
            for (ar = room(1,x,y); ar; ar = room(0,0,0)) {
                if (ar->x1 != ar->x2) return (FALSE);
                if (ar->y1 != ar->y2) return (FALSE);
            }

#endif

        }
    }

    /* Must be okay */
    return (TRUE);
}


/*
 * Determine if the given "box" in the world is fully "known"
 */
static bool borg_build_room_known(int x1, int y1, int x2, int y2)
{
    int x, y;

    /* Check for "unknown" grids */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            /* Access that grid */
            auto_grid *ag = grid(x,y);

            /* Refuse to accept unknown grids */
            if (ag->o_c == ' ') return (FALSE);
        }
    }

    /* Must be okay */
    return (TRUE);
}




/*
 * Attempt to build a "bigger" room for the given location
 * Return TRUE if a "new" room was created, else FALSE
 *
 * Possibly problematic rooms:
 *
 *   #########
 *   ##.....##		Currently this room is parsed as two
 *   #.......#		large overlapping rectangles.  We should
 *   #.......#		probably consider finding the intersection
 *   #.......#		of those rooms, and subtracting out the rest
 *   ##.....##		resulting in four small and one large room.
 *   #########
 *
 * Note that the "room builder" will automatically find the largest
 * rectangle, and will automatically "absorb" enclosed rooms.  Now
 * all we need to do is "notice" when we are "crossing" another room.
 * As long as we do not cross TWO rooms, we should be fine (?).  But
 * observe the following room:
 *
 *   #########
 *   ###...###		Assume that (somehow) the two "horizontal" rooms
 *   ###...###		are constructed first, and then the "vertical"
 *   #.......#		room is discovered.  Now the room crosses TWO
 *   #.......#		other rooms, and we would like to "extract"
 *   ###...###		from this union all NINE resulting rectangles...
 *   ###...###
 *   #.......#		Note that crossing corridors do NOT have this
 *   #.......#		problem, except when "new holes" are made in
 *   ###...###		the middle of existing corridors.  That is
 *   ###...###		something we might consider noticing.
 *   #########
 *
 * Another question to ask is whether we would rather have:
 *   (1) rooms that cross each other
 *   (2) no crossing rooms, but some "touching" rooms
 *   (3) no crossing rooms and no touching rooms
 *
 * Note that #2 seems to allow for the "nicest" treatment of corridors.
 *
 * Note that a room is NEVER built including "unknown" grids.
 *
 * Also note that corridors are handled by a special routine, and rooms
 * are only constructed out of at least a 2x2 block of grids.  Corridors
 * are never allowed to overlap any other rooms.
 *
 * See the "CROSS_ROOM" compile-time define which determine whether the
 * Borg is allowed to generate "crossing" rooms.  This option allows for
 * "better" room generation, but at the cost of efficiency.
 */
bool borg_build_room(int x, int y)
{
    uint i, j;

    int x1, y1, x2, y2;

    auto_grid *ag;
    auto_room *ar;

    bool change = FALSE;


    /* Attempt to expand a 3x3 room */
    if (borg_build_room_floor(x-1, y-1, x+1, y+1)) {
        x1 = x - 1; y1 = y - 1; x2 = x + 1; y2 = y + 1;
    }

    /* Or attempt to expand a 3x2 room (south) */
    else if (borg_build_room_floor(x-1, y, x+1, y+1)) {
        x1 = x - 1; y1 = y; x2 = x + 1; y2 = y + 1;
    }

    /* Or attempt to expand a 3x2 room (north) */
    else if (borg_build_room_floor(x-1, y-1, x+1, y)) {
        x1 = x - 1; y1 = y - 1; x2 = x + 1; y2 = y;
    }

    /* Or attempt to expand a 2x3 room (east) */
    else if (borg_build_room_floor(x, y-1, x+1, y+1)) {
        x1 = x; y1 = y - 1; x2 = x + 1; y2 = y + 1;
    }

    /* Or attempt to expand a 2x3 room (west) */
    else if (borg_build_room_floor(x-1, y-1, x, y+1)) {
        x1 = x - 1; y1 = y - 1; x2 = x; y2 = y + 1;
    }

    /* Or attempt to expand a 2x2 room (south east) */
    else if (borg_build_room_floor(x, y, x+1, y+1)) {
        x1 = x; y1 = y; x2 = x + 1; y2 = y + 1;
    }

    /* Or attempt to expand a 2x2 room (south west) */
    else if (borg_build_room_floor(x-1, y, x, y+1)) {
        x1 = x - 1; y1 = y; x2 = x; y2 = y + 1;
    }

    /* Or attempt to expand a 2x2 room (north east) */
    else if (borg_build_room_floor(x, y-1, x+1, y)) {
        x1 = x; y1 = y - 1; x2 = x + 1; y2 = y;
    }

    /* Or attempt to expand a 2x2 room (north west) */
    else if (borg_build_room_floor(x-1, y-1, x, y)) {
        x1 = x - 1; y1 = y - 1; x2 = x; y2 = y;
    }

    /* Hack -- only "snake" grids can grow corridors */
    else if (!borg_build_snake(x, y)) {
        x1 = x; y1 = y; x2 = x; y2 = y;
    }

    /* Or attempt to extend a corridor (south) */
    else if (borg_build_snake(x, y+1)) {
        x1 = x; y1 = y; x2 = x; y2 = y + 1;
    }

    /* Or attempt to extend a corridor (north) */
    else if (borg_build_snake(x, y-1)) {
        x1 = x; y1 = y - 1; x2 = x; y2 = y;
    }

    /* Or attempt to extend a corridor (east) */
    else if (borg_build_snake(x+1, y)) {
        x1 = x; y1 = y; x2 = x + 1; y2 = y;
    }

    /* Or attempt to extend a corridor (west) */
    else if (borg_build_snake(x-1, y)) {
        x1 = x - 1; y1 = y; x2 = x; y2 = y;
    }


    /* Default to 1x1 grid */
    else {
        x1 = x; y1 = y; x2 = x; y2 = y;
    }


    /* Hack -- Single grid (1x1) rooms are boring */
    if ((x1 == x2) && (y1 == y2)) return (FALSE);


    /* Expand a north/south corridor */
    if (x1 == x2) {

        /* Grow south/north */
        while (borg_build_snake(x, y2+1)) y2++;
        while (borg_build_snake(x, y1-1)) y1--;
    }

    /* Expand a east/west corridor */
    else if (y1 == y2) {

        /* Grow east/west */
        while (borg_build_snake(x2+1, y)) x2++;
        while (borg_build_snake(x1-1, y)) x1--;
    }

    /* Expand a rectangle -- try south/north first */
    else if (rand_int(100) < 50) {

        /* Grow south/north */
        while (borg_build_room_floor(x1, y2+1, x2, y2+1)) y2++;
        while (borg_build_room_floor(x1, y1-1, x2, y1-1)) y1--;

        /* Grow east/west */
        while (borg_build_room_floor(x2+1, y1, x2+1, y2)) x2++;
        while (borg_build_room_floor(x1-1, y1, x1-1, y2)) x1--;
    }

    /* Expand a rectangle -- try east/west first */
    else {

        /* Grow east/west */
        while (borg_build_room_floor(x2+1, y1, x2+1, y2)) x2++;
        while (borg_build_room_floor(x1-1, y1, x1-1, y2)) x1--;

        /* Grow south/north */
        while (borg_build_room_floor(x1, y2+1, x2, y2+1)) y2++;
        while (borg_build_room_floor(x1, y1-1, x2, y1-1)) y1--;
    }


    /* Hack -- refuse to build rooms touching unknowns */
    if (!borg_build_room_known(x1-1, y2+1, x2+1, y2+1)) return (FALSE);
    if (!borg_build_room_known(x1-1, y1-1, x2+1, y1-1)) return (FALSE);
    if (!borg_build_room_known(x2+1, y1-1, x2+1, y2+1)) return (FALSE);
    if (!borg_build_room_known(x1-1, y1-1, x1-1, y2+1)) return (FALSE);


    /* Make sure this room does not exist and is not contained */
    for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

        /* Never make a room "inside" another room */
        if ((ar->x1 <= x1) && (x2 <= ar->x2) &&
            (ar->y1 <= y1) && (y2 <= ar->y2)) {

            /* The room already exists */
            return (FALSE);
        }
    }


    /* Message */
    borg_note(format("# Room (%d,%d to %d,%d)", x1, y1, x2, y2));

    /* Access a free room */
    ar = borg_free_room();

    /* Initialize the new room */
    ar->x = ar->x1 = x1; ar->x2 = x2;
    ar->y = ar->y1 = y1; ar->y2 = y2;

    /* Save the room index */
    i = ar->self;

    /* Absorb old rooms */
    for (j = 1; j < auto_room_max; j++) {

        /* Skip the "current" room! */
        if (i == j) continue;

        /* Get the room */
        ar = &auto_rooms[j];

        /* Skip "free" rooms */
        if (ar->free) continue;

        /* Skip non-contained rooms */
        if ((ar->x1 < x1) || (ar->y1 < y1)) continue;
        if ((x2 < ar->x2) || (y2 < ar->y2)) continue;

        /* Scan the "contained" room */
        for (y = ar->y1; y <= ar->y2; y++) {
            for (x = ar->x1; x <= ar->x2; x++) {

                /* Get the "contained" grid */
                ag = grid(x,y);

                /* Normal grids "lose" their parents. */
                if (ag->room < AUTO_ROOMS) ag->room = 0;

                /* Cross-rooms lose one parent */
                if (ag->room > AUTO_ROOMS) ag->room--;
            }
        }

        /* That room is now "gone" */
        ar->when = 0L;
        ar->x1 = ar->x2 = ar->x = 0;
        ar->y1 = ar->y2 = ar->y = 0;

        /* Add it to the "free list" */
        auto_rooms[j].free = auto_rooms[0].free;
        auto_rooms[0].free = j;
    }


    /* Access the new room */
    ar = &auto_rooms[i];

    /* Scan the grids contained in the new room */
    for (y = ar->y1; y <= ar->y2; y++) {
        for (x = ar->x1; x <= ar->x2; x++) {

            /* Get the "contained" grid */
            ag = grid(x,y);

            /* Steal "absorbed" grids */
            if (ag->room == AUTO_ROOMS) ag->room = ar->self;

            /* Steal "fresh" grids */
            if (ag->room == 0) ag->room = ar->self;

            /* Skip grids owned by this room */
            if (ag->room == i) continue;

            /* Normal grids become "cross-grids" (one parent) */
            if (ag->room < AUTO_ROOMS) ag->room = AUTO_ROOMS + 1;

            /* All cross-grids now have another parent */
            ag->room++;
        }
    }

    /* Changed */
    change = TRUE;

    /* Changed */
    return (change);
}



/*
 * Destroy all rooms containing the given location
 */
bool borg_clear_room(int x, int y)
{
    uint xx, yy;

    auto_grid *ag;
    auto_room *ar;

    bool res = FALSE;


    /* Absorb old rooms */
    for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

        /* Scan the "contained" room */
        for (yy = ar->y1; yy <= ar->y2; yy++) {
            for (xx = ar->x1; xx <= ar->x2; xx++) {

                /* Get the "contained" grid */
                ag = grid(xx,yy);

                /* Normal grids "lose" their parents. */
                if (ag->room < AUTO_ROOMS) ag->room = 0;

                /* Cross-rooms lose one parent */
                if (ag->room > AUTO_ROOMS) ag->room--;
            }
        }

        /* That room is now "gone" */
        ar->when = 0L;
        ar->x1 = ar->x2 = 0;
        ar->y1 = ar->y2 = 0;

        /* Add the room to the "free list" */
        auto_rooms[ar->self].free = auto_rooms[0].free;

        /* Add the room to the "free list" */
        auto_rooms[0].free = ar->self;
    }


    /* Result */
    return (res);
}




/*
 * Clear out the "room" array
 */
static void borg_wipe_rooms(void)
{
    int i, x, y;

    auto_grid *ag;
    auto_room *ar;


    /* Clean up the old used rooms */
    for (i = 1; i < auto_room_max; i++) {

        /* Access the room */
        ar = &auto_rooms[i];

        /* Place the room in the free room list */
        ar->free = i + 1;

        /* No location */
        ar->x1 = ar->x2 = ar->y1 = ar->y2 = 0;

        /* Never seen it */
        ar->when = 0L;
    }

    /* Reset the free list */
    auto_room_head->free = 1;

    /* Maximum room index */
    auto_room_max = 1;
}

#endif



/*
 * Version of "los()" for the Borg.
 */
bool borg_los(int x1, int y1, int x2, int y2)
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


    /* Directly South/North */
    if (!d_x) {

        int p_y;

        /* South -- check for walls */
        if (d_y > 0) {
            for (p_y = y1 + 1; p_y < y2; p_y++) {
                if (grid(x1,p_y)->info & BORG_WALL) return FALSE;
            }
        }

        /* North -- check for walls */
        else {
            for (p_y = y1 - 1; p_y > y2; p_y--) {
                if (grid(x1,p_y)->info & BORG_WALL) return FALSE;
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
                if (grid(p_x,y1)->info & BORG_WALL) return FALSE;
            }
        }

        /* West -- check for walls */
        else {
            for (p_x = x1 - 1; p_x > x2; p_x--) {
                if (grid(p_x,y1)->info & BORG_WALL) return FALSE;
            }
        }

        /* Assume los */
        return TRUE;
    }


    /* Handle Knightlike shapes -CWS */
    if (a_x == 1) {
        if (d_y == 2) {
            if (!(grid(x1,y1+1)->info & BORG_WALL)) return TRUE;
        }
        else if (d_y == (-2)) {
            if (!(grid(x1,y1-1)->info & BORG_WALL)) return TRUE;
        }
    }
    else if (a_y == 1) {
        if (d_x == 2) {
            if (!(grid(x1+1,y1)->info & BORG_WALL)) return TRUE;
        }
        else if (d_x == (-2)) {
            if (!(grid(x1-1,y1)->info & BORG_WALL)) return TRUE;
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
                if (grid(p_x,p_y)->info & BORG_WALL) return FALSE;
                dy += m;
                if (dy < scale2) {
                    p_x += xSign;
                }
                else if (dy > scale2) {
                    p_y += ySign;
                    if (grid(p_x,p_y)->info & BORG_WALL) return FALSE;
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
                if (grid(p_x,p_y)->info & BORG_WALL) return FALSE;
                dx += m;
                if (dx < scale2) {
                    p_y += ySign;
                }
                else if (dx > scale2) {
                    p_x += xSign;
                    if (grid(p_x,p_y)->info & BORG_WALL) return FALSE;
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
 * Clear the viewable space
 */
static void borg_forget_view(void)
{
    int i;

    /* None to forget */
    if (!auto_view_n) return;

    /* Clear them all */
    for (i = 0; i < auto_view_n; i++) {

        int y = auto_view_y[i];
        int x = auto_view_x[i];

        /* Forget that the grid is viewable */
        grid(x,y)->info &= ~BORG_VIEW;
        grid(x,y)->info &= ~BORG_XTRA;
    }

    /* None left */
    auto_view_n = 0;
}



/*
 * Set the "view" flag of the given cave grid
 * Never call this function when the "view" array is full.
 * Never call this function with an "illegal" location.
 * This function assumes that "AUTO_VIEW_N" is large enough.
 */
static void borg_cave_view(int x, int y)
{
    auto_grid *ag = grid(x,y);

    /* Can only be set once */
    if (ag->info & BORG_VIEW) return;

    /* Set the flag */
    ag->info |= BORG_VIEW;

    /* Add to queue */
    auto_view_y[auto_view_n] = y;
    auto_view_x[auto_view_n] = x;
    auto_view_n++;
}



/*
 * Update the view (see "cave.c")
 */
static bool borg_update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, z1, z2, wall;

    auto_grid *ag = grid(x,y);

    auto_grid *ag1 = grid(x1,y1);
    auto_grid *ag2 = grid(x2,y2);


    /* Examine the given grid */
    wall = (ag->info & BORG_WALL);


    /* Check the walls */
    f1 = (!(ag1->info & BORG_WALL));
    f2 = (!(ag2->info & BORG_WALL));

    /* Totally blocked by physical walls */
    if (!f1 && !f2) return (TRUE);


    /* Check the visibility */
    v1 = (f1 && (ag1->info & BORG_VIEW));
    v2 = (f2 && (ag2->info & BORG_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);


    /* Check the "ease" of visibility */
    z1 = (v1 && (ag1->info & BORG_XTRA));
    z2 = (v2 && (ag2->info & BORG_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (z1 && z2) {

        ag->info |= BORG_XTRA;
        borg_cave_view(x,y);
        return (wall);
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (z1) {

        borg_cave_view(x,y);
        return (wall);
    }


    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2) {

        /* ag->info |= BORG_XTRA; */
        borg_cave_view(x,y);
        return (wall);
    }


    /* Hack -- the "los()" works poorly on walls */
    if (wall) {

        borg_cave_view(x,y);
        return (wall);
    }


    /* Hack -- check line of sight */
    if (borg_los(c_x, c_y, x, y)) {

        borg_cave_view(x,y);
        return (wall);
    }


    /* Assume no line of sight. */
    return (TRUE);
}



/*
 * Update the view, return TRUE if map changed
 */
static void borg_update_view(void)
{
    int n, m, d, k, y, x;

    int se, sw, ne, nw, es, en, ws, wn;

    int limit, over, full;


    /* Start with full vision */
    full = MAX_SIGHT;

    /* Extract the "octagon" limits */
    over = full * 3 / 2;


    /*** Step 0 -- Begin ***/

    /* Forget the old "view" grids */
    for (n = 0; n < auto_view_n; n++) {

        int y = auto_view_y[n];
        int x = auto_view_x[n];

        auto_grid *ag = grid(x,y);

        /* Mark the grid as not in "view" */
        ag->info &= ~(BORG_VIEW | BORG_XTRA);
    }

    /* Wipe the "view" array */
    auto_view_n = 0;


    /*** Step 1 -- adjacent grids ***/

    /* Start on the player */
    x = c_x;
    y = c_y;

    /* Assume the player grid is easily viewable */
    grid(x,y)->info |= BORG_XTRA;

    /* Assume the player grid is viewable */
    borg_cave_view(x,y);


    /*** Step 2 -- Major Diagonals ***/

    /* Hack -- Limit */
    limit = full * 2 / 3;

    /* Scan south-east */
    for (d = 1; d <= limit; d++) {
        grid(x+d,y+d)->info |= BORG_XTRA;
        borg_cave_view(x+d,y+d);
        if (grid(x+d,y+d)->info & BORG_WALL) break;
    }

    /* Scan south-west */
    for (d = 1; d <= limit; d++) {
        grid(x-d,y+d)->info |= BORG_XTRA;
        borg_cave_view(x-d,y+d);
        if (grid(x-d,y+d)->info & BORG_WALL) break;
    }

    /* Scan north-east */
    for (d = 1; d <= limit; d++) {
        grid(x+d,y-d)->info |= BORG_XTRA;
        borg_cave_view(x+d,y-d);
        if (grid(x+d,y-d)->info & BORG_WALL) break;
    }

    /* Scan north-west */
    for (d = 1; d <= limit; d++) {
        grid(x-d,y-d)->info |= BORG_XTRA;
        borg_cave_view(x-d,y-d);
        if (grid(x-d,y-d)->info & BORG_WALL) break;
    }


    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++) {
        grid(x,y+d)->info |= BORG_XTRA;
        borg_cave_view(x,y+d);
        if (grid(x,y+d)->info & BORG_WALL) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++) {
        grid(x,y-d)->info |= BORG_XTRA;
        borg_cave_view(x,y-d);
        if (grid(x,y-d)->info & BORG_WALL) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++) {
        grid(x+d,y)->info |= BORG_XTRA;
        borg_cave_view(x+d,y);
        if (grid(x+d,y)->info & BORG_WALL) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++) {
        grid(x-d,y)->info |= BORG_XTRA;
        borg_cave_view(x-d,y);
        if (grid(x-d,y)->info & BORG_WALL) break;
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
                    if (borg_update_view_aux(ypn+d,xpn,ypn+d-1,xpn-1,ypn+d-1,xpn)) {
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
                    if (borg_update_view_aux(ypn+d,xmn,ypn+d-1,xmn+1,ypn+d-1,xmn)) {
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
                    if (borg_update_view_aux(ymn-d,xpn,ymn-d+1,xpn-1,ymn-d+1,xpn)) {
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
                    if (borg_update_view_aux(ymn-d,xmn,ymn-d+1,xmn+1,ymn-d+1,xmn)) {
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
                    if (borg_update_view_aux(ypn,xpn+d,ypn-1,xpn+d-1,ypn,xpn+d-1)) {
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
                    if (borg_update_view_aux(ymn,xpn+d,ymn+1,xpn+d-1,ymn,xpn+d-1)) {
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
                    if (borg_update_view_aux(ypn,xmn-d,ypn-1,xmn-d+1,ypn,xmn-d+1)) {
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
                    if (borg_update_view_aux(ymn,xmn-d,ymn+1,xmn-d+1,ymn,xmn-d+1)) {
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
}




/*
 * Hack -- make a "safe" assumption about the level
 * Should only be used in the town, or when really bored.
 */
static void borg_assume_map(void)
{
    int x, y;

    auto_grid *ag;

    /* Clean up the grids */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {

            /* Access the grid */
            ag = grid(x, y);

            /* Mega-Hack -- assume floors */
            ag->o_a = TERM_WHITE;
            ag->o_c = '.';
        }
    }

    /* Hack -- mark the edge of the map as walls */
    for (y = 0; y < AUTO_MAX_Y; y++) {

        /* West edge */
        grid(0,y)->o_c = '#';

        /* East edge */
        grid(AUTO_MAX_X-1,y)->o_c = '#';
    }	

    /* Hack -- mark the edge of the map as walls */
    for (x = 0; x < AUTO_MAX_X; x++) {

        /* West edge */
        grid(x,0)->o_c = '#';

        /* East edge */
        grid(x,AUTO_MAX_Y-1)->o_c = '#';
    }	
}



/*
 * Update the Borg based on the current "map"
 */
void borg_forget_map(void)
{
    int x, y;

    auto_grid *ag;


#ifdef BORG_ROOMS

    /* Wipe the "rooms" */
    borg_wipe_rooms();

#endif


    /* Clean up the grids */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {

            /* Access the grid */
            ag = grid(x, y);

            /* Unknown */
            ag->o_a = 0;
            ag->o_c = ' ';

            /* No bit flags */
            ag->info = 0;

            /* No extra info */
            ag->xtra = 0;

#ifdef BORG_ROOMS

            /* Hack -- no room */
            ag->room = 0;

#endif	/* BORG_ROOMS */

        }
    }

    
    /* Hack -- mark the edge of the map as walls */
    for (y = 0; y < AUTO_MAX_Y; y++) {

        /* West edge (walls, and icky) */
        grid(0,y)->info |= (BORG_WALL);

        /* East edge (walls, and icky) */
        grid(AUTO_MAX_X-1,y)->info |= (BORG_WALL);
    }	

    /* Hack -- mark the edge of the map as walls */
    for (x = 0; x < AUTO_MAX_X; x++) {

        /* West edge (walls, and icky) */
        grid(x,0)->info |= (BORG_WALL);

        /* East edge (walls, and icky) */
        grid(x,AUTO_MAX_Y-1)->info |= (BORG_WALL);
    }	


    /* Reset "auto_data_cost" */
    COPY(auto_data_cost, auto_data_hard, auto_data);

    /* Reset "auto_data_flow" */
    COPY(auto_data_flow, auto_data_hard, auto_data);


    /* Clear "auto_data_know" */
    WIPE(auto_data_know, auto_data);
    
    /* Clear "auto_data_icky" */
    WIPE(auto_data_icky, auto_data);
    

    /* Forget the view */
    borg_forget_view();


    /* Mega-Hack -- Make assumptions about the town */
    if (!auto_depth) borg_assume_map();
}




/*
 * Update the Borg based on the current "map"
 *
 * Maintain the "BORG_WALL" and "BORG_OKAY" flags, and also update
 * the "view", maintaining the "BORG_VIEW" and "BORG_XTRA" flags.
 *
 * Must be called AFTER "update_frame()" to notice that the depth
 * has changed.  Actually, a "borg_map_wipe()" would be better...
 *
 * Hack -- The player grid contents are always "unknown"
 *
 * XXX XXX Hack -- note the fast direct access to the screen.
 */
void borg_update_map(void)
{
    int x, y, dx, dy;

    int num_players = 0;

    auto_grid *ag;

    byte a;
    char c;

    byte *aa;
    char *cc;

    static int o_w_x, o_w_y;
    static int o_c_x, o_c_y;


    /* Forget the previous (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Access the actual location */
            x = o_w_x + dx;
            y = o_w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Clear the "okay" field */
            ag->info &= ~BORG_OKAY;
        }
    }


    /* Analyze the current (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
    
        /* Direct access XXX XXX XXX */
        aa = &(Term->scr->a[dy+1][13]);
        cc = &(Term->scr->c[dy+1][13]);
        
        /* Scan the row */
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Access */
            a = *aa++;
            c = *cc++;

            /* Mega-Hack */
            if (!c) c = ' ';

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);


            /* Notice the player */
            if (c == '@') {

                /* Count the players */
                num_players++;

                /* Save the player grid */
                c_x = x;
                c_y = y;

                /* Assume dark */
                a = 0;
                c = ' ';

                /* Mega-Hack -- assume initial knowledge */
                if (ag->o_a == 0) a = TERM_WHITE;
                if (ag->o_c == ' ') c = '.';
            }


            /* Do not process dark grids */
            if (!a || (c == ' ')) continue;


            /* Notice "information" */
            ag->info |= BORG_OKAY;

            /* Save the info */
            ag->o_a = a; ag->o_c = c;


            /* Hack -- Walls, Seams, Doors, Rubble, Hidden */
            if ((c == '#') || (c == '%') || 
                (c == '+') || (c == ':') || (c == '*')) {

                /* We are a wall */
                ag->info |= BORG_WALL;
            }

            /* Hack -- Floors, Doors, Stairs */
            else if ((c == '.') || (c == '\'') || 
                     (c == '<') || (c == '>')) {

                /* We are not a wall */
                ag->info &= ~BORG_WALL;
            }
        }
    }


    /* Mega-Oops */
    if (num_players != 1) {

        /* Assume Hallucinating */
        do_image = TRUE;

        /* Mega-Hack -- prevent crashes */
        c_x = o_c_x;
        c_y = o_c_y;
    }


    /* Mega-Hack -- handle bashing */
    grid(c_x,c_y)->info &= ~BORG_WALL;


    /* Update the view */
    borg_update_view();


    /* Save the "old" data */
    o_w_x = w_x;
    o_w_y = w_y;
    o_c_x = c_x;
    o_c_y = c_y;
}







/*
 * Is a grid "okay" for us to run into
 *
 * This routine is ONLY called by "borg_goto_dir()", and then only
 * if the destination grid is not adjacent to the player.
 */
static bool borg_okay_grid(int x, int y)
{
    auto_grid *ag = grid(x, y);

    /* Avoid unknown grids */
    if (ag->o_c == ' ') return (FALSE);

#if 0
    /* Floors are okay */
    if (ag->o_c == '.') return (TRUE);

    /* Hack -- Open doors are okay */
    if (ag->o_c == '\'') return (TRUE);

    /* Hack -- Stairs are okay */
    if (ag->o_c == '<') return (TRUE);
    if (ag->o_c == '>') return (TRUE);

    /* Assume not okay */
    return (FALSE);
#endif

#if 0
    /* Avoid walls/seams/doors/rubble */
    if (strchr("#%+:", ag->o_c)) return (FALSE);

    /* Avoid normal "monsters" */
    if (isalpha(ag->o_c) || (ag->o_c == '&')) return (FALSE);

    /* Avoid Hack -- Prefer not to walk into stores */
    if (isdigit(ag->o_c)) return (FALSE);
#endif

    /* Assume okay */
    return (TRUE);
}


/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 */
static int borg_extract_dir(int x1, int y1, int x2, int y2)
{
    /* Stay still */
    if ((y1 == y2) && (x1 == x2)) return (5);

    /* No x-motion */
    if (x1 == x2) return ((y1 < y2) ? 2 : 8);

    /* No y-motion */
    if (y1 == y2) return ((x1 < x2) ? 6 : 4);

    /* South */
    if (y1 < y2) return ((x1 < x2) ? 3 : 1);

    /* North */
    if (y1 > y2) return ((x1 < x2) ? 9 : 7);

    /* Oops */
    return (5);
}


/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 * Attempt to avoid walls if possible.  Return "0" if none.
 */
int borg_goto_dir(int x1, int y1, int x2, int y2)
{
    int d;


    /* Special case -- next to (or on) the goal */
    if ((ABS(y2-y1) <= 1) && (ABS(x2-x1) <= 1)) {
        return (borg_extract_dir(x1, y1, x2, y2));
    }


    /* Try to do the "horizontal" */
    if (ABS(y2 - y1) < ABS(x2 - x1)) {
        d = borg_extract_dir(x1, y1, x2, y1);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);
    }

    /* Try to do the "vertical" */
    if (ABS(y2 - y1) > ABS(x2 - x1)) {
        d = borg_extract_dir(x1, y1, x1, y2);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);
    }


    /* Try to walk "directly" there */
    d = borg_extract_dir(x1, y1, x2, y2);

    /* Check for walls */
    if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);


    /* Try the "vertical" instead (includes "diagonal") */
    if (ABS(y2 - y1) <= ABS(x2 - x1)) {
        d = borg_extract_dir(x1, y1, x1, y2);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);
    }

    /* Try the "horizontal" instead (includes "diagonal") */
    if (ABS(y2 - y1) >= ABS(x2 - x1)) {
        d = borg_extract_dir(x1, y1, x2, y1);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);
    }


    /* Hack -- directly "vertical", try "shaking" */
    if (x2 == x1) {

        /* Shake to the east */
        d = borg_extract_dir(x1, y1, x1+1, y2);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);

        /* Shake to the west */
        d = borg_extract_dir(x1, y1, x1-1, y2);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);
    }

    /* Hack -- directly "horizontal", try "shaking" */
    if (y2 == y1) {

        /* Shake to the south */
        d = borg_extract_dir(x1, y1, x2, y1+1);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);

        /* Shake to the north */
        d = borg_extract_dir(x1, y1, x2, y1-1);
        if (borg_okay_grid(x1 + ddx[d], y1 + ddy[d])) return (d);
    }


    /* Hack -- Surrounded by obstacles. */
    d = borg_extract_dir(x1, y1, x2, y2);

    /* Let the calling routine check the result */
    return (d);
}





/*
 * Hack -- forget the "flow" information
 *
 * This function is called a lot, and it would probably benefit
 * from any optimization at all, for example, moving the "cost"
 * and "flow" fields into separate arrays, and using the WIPE()
 * and COPY() macros, or using pointer math, not dereferencing.
 */
void borg_flow_clear(void)
{
    /* Reset the "cost" fields */
    COPY(auto_data_cost, auto_data_hard, auto_data);

    /* Wipe costs and danger */
    if (auto_danger_wipe) {

        /* Wipe the "know" flags */
        WIPE(auto_data_know, auto_data);
            
        /* Wipe the "icky" flags */
        WIPE(auto_data_icky, auto_data);
            
        /* Wipe complete */
        auto_danger_wipe = FALSE;
    }

    /* Start over */
    flow_head = 0;
    flow_tail = 0;
}




/*
 * Hack -- fill in the "cost" field of every grid that the player
 * can "reach" with the number of steps needed to reach that grid.
 * This also yields the "distance" of the player from every grid.
 *
 * Note that the "cost" is stored as a byte, so any grid more
 * than 255 grids away will be considered unreachable.
 *
 * Hack -- use the "flow" array as a "circular queue".
 *
 * We do not need a priority queue because the cost from grid
 * to grid is always "one" and we process them in order.
 *
 * Hack -- Handle "danger" by allowing a grid to be marked as "ICKY",
 * in which case we will "avoid" it.  Also, allow this function to
 * calculate the danger of a grid, by assuming that any grid not
 * marked as "KNOW" is of unknown danger, and should be checked by
 * the "auto_danger_hook(x,y)" function, which should return "TRUE"
 * if the Borg should avoid that grid.  In either case, that grid
 * will then be marked as "KNOW" to prevent multiple danger checks.
 *
 * Hopefully, this complex method, at the cost of only 2 bits per grid,
 * will allow us to minimize the number of expensive "danger" checks.
 *
 * This function is extremely expensive, due to the number of calls
 * to the "borg_danger()" function, even though those calls are kept
 * to a minimum by only asking about grids we may need to pass through.
 */
void borg_flow_spread(bool optimize)
{
    int i, n;

    int auto_flow_max = 255;


    /* Now process the queue */
    while (flow_head != flow_tail) {

        /* Extract the next entry */
        int x1 = auto_flow_x[flow_tail];
        int y1 = auto_flow_y[flow_tail];


        /* Forget that entry */
        if (++flow_tail == AUTO_FLOW_MAX) flow_tail = 0;


        /* Cost (one per movement grid) */
        n = auto_data_cost->data[y1][x1] + 1;

        /* Paranoia */
        if (n >= 250) continue;


        /* Add the "children" */
        for (i = 0; i < 8; i++) {

            int old_head = flow_head;

            /* Direction */
            int x = x1 + ddx_ddd[i];
            int y = y1 + ddy_ddd[i];

            auto_grid *ag = grid(x,y);

            /* Hack -- limit flow depth */
            if (n > auto_flow_max) continue;

            /* Skip "reached" grids */
            if (auto_data_cost->data[y][x] <= n) continue;

            /* Ignore "wall" grids */
            if (ag->info & BORG_WALL) continue;

            /* Ignore "icky" grids */
            if (auto_data_icky->data[y][x]) continue;

            /* Check danger if needed */
            if (!auto_data_know->data[y][x]) {

                /* Assume Known */
                auto_data_know->data[y][x] = TRUE;

                /* Check the danger */
                if (auto_danger_hook && ((*auto_danger_hook)(x,y))) {

                    /* Mark as icky */
                    auto_data_icky->data[y][x] = TRUE;

                    /* Ignore this grid */
                    continue;
                }
            }

            /* Save the flow cost */
            auto_data_cost->data[y][x] = n;

            /* Enqueue that entry */
            auto_flow_x[flow_head] = x;
            auto_flow_y[flow_head] = y;

            /* Advance the queue */
            if (++flow_head == AUTO_FLOW_MAX) flow_head = 0;

            /* Hack -- notice overflow by forgetting new entry */
            if (flow_head == flow_tail) flow_head = old_head;

            /* Optimize (if requested) */
            if (optimize) auto_flow_max = auto_data_cost->data[c_y][c_x];
        }
    }

    /* Forget the flow info */
    flow_head = flow_tail = 0;
}


/*
 * Enqueue a fresh (legal) starting grid
 */
void borg_flow_enqueue_grid(int x, int y)
{
    /* New grid */
    if (auto_data_cost->data[y][x]) {

        int old_head = flow_head;

        /* Cheap */
        auto_data_cost->data[y][x] = 0;

        /* Enqueue that entry */
        auto_flow_y[flow_head] = y;
        auto_flow_x[flow_head] = x;

        /* Advance the queue */
        if (++flow_head == AUTO_FLOW_MAX) flow_head = 0;

        /* Mega-Hack -- notice overflow by forgetting new entry */
        if (flow_head == flow_tail) flow_head = old_head;
    }
}




/*
 * Do a "reverse" flow from the player outwards
 */
void borg_flow_reverse(void)
{
    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the player's grid */
    borg_flow_enqueue_grid(c_x, c_y);

    /* Spread, but do NOT optimize */
    borg_flow_spread(FALSE);
}







/*
 * Target a location.  Can be used alone or at "Direction?" prompt.
 *
 * Warning -- This will only work for locations on the current panel
 */
bool borg_target(int x, int y)
{
    int x1, y1, x2, y2;

    /* Log */
    borg_note("# Targetting a location.");

    /* Target mode */
    borg_keypress('*');

    /* Target a location */
    borg_keypress('p');

    /* Determine "path" */
    x1 = c_x;
    y1 = c_y;
    x2 = x;
    y2 = y;

    /* Move to the location (diagonals) */
    for ( ; (y1 < y2) && (x1 < x2); y1++, x1++) borg_keypress('3');
    for ( ; (y1 < y2) && (x1 > x2); y1++, x1--) borg_keypress('1');
    for ( ; (y1 > y2) && (x1 < x2); y1--, x1++) borg_keypress('9');
    for ( ; (y1 > y2) && (x1 > x2); y1--, x1--) borg_keypress('7');

    /* Move to the location */
    for ( ; y1 < y2; y1++) borg_keypress('2');
    for ( ; y1 > y2; y1--) borg_keypress('8');
    for ( ; x1 < x2; x1++) borg_keypress('6');
    for ( ; x1 > x2; x1--) borg_keypress('4');

    /* Select the target */
    borg_keypress('5');

    /* Success */
    return (TRUE);
}





/*
 * Init this file.
 */
void borg_map_init(void)
{
    int x, y;


#ifdef BORG_ROOMS

    /*** Rooms ***/

    /* Make the array of rooms */
    C_MAKE(auto_rooms, AUTO_ROOMS, auto_room);

    /* Initialize the rooms */
    for (i = 0; i < AUTO_ROOMS; i++) {

        /* Save our own index */
        auto_rooms[i].self = i;

        /* Initialize the "free" list */
        auto_rooms[i].free = i + 1;
    }

    /* Save the head/tail of the room array */
    auto_room_head = &auto_rooms[0];
    auto_room_tail = &auto_rooms[AUTO_ROOMS-1];

    /* Prepare the "tail" of the free list */
    auto_room_tail->free = auto_room_tail->self;

    /* Reset the free list */
    auto_room_head->free = 1;

    /* Maximum room index */
    auto_room_max = 1;

#endif


    /*** Grid data ***/
    
    /* Make the "flow" data */
    MAKE(auto_data_flow, auto_data);
    
    /* Make the "cost" data */
    MAKE(auto_data_cost, auto_data);

    /* Make the "know" flags */
    MAKE(auto_data_know, auto_data);

    /* Make the "icky" flags */
    MAKE(auto_data_icky, auto_data);

    /* Make the "hard" data */
    MAKE(auto_data_hard, auto_data);

    /* Prepare "auto_data_hard" */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {

            /* Prepare "auto_data_hard" */
            auto_data_hard->data[y][x] = 255;
        }
    }
    

    /*** Grids ***/

    /* Make the array of grids */
    C_MAKE(auto_grids, AUTO_MAX_Y, auto_grid*);

    /* Make each row of grids */
    for (y = 0; y < AUTO_MAX_Y; y++) {

        /* Make each row */
        C_MAKE(auto_grids[y], AUTO_MAX_X, auto_grid);
    }

    /* Forget the map */
    borg_forget_map();


    /*** Locations ***/

    /* Hack -- array of "seen" grids */
    C_MAKE(auto_temp_x, AUTO_SEEN_MAX, byte);
    C_MAKE(auto_temp_y, AUTO_SEEN_MAX, byte);

    /* Hack -- array of "view" grids */
    C_MAKE(auto_view_x, AUTO_VIEW_MAX, byte);
    C_MAKE(auto_view_y, AUTO_VIEW_MAX, byte);

    /* Hack -- array of "flow" grids */
    C_MAKE(auto_flow_x, AUTO_FLOW_MAX, byte);
    C_MAKE(auto_flow_y, AUTO_FLOW_MAX, byte);
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

