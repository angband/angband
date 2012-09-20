/* File: borg.c */

/* Purpose: an "automatic" player -BEN- */

#include "angband.h"



#ifdef AUTO_PLAY

#include "borg.h"


/*
 * See "borg.h" for general information.
 *
 * This file provides general support for various "Borg" files.
 *
 * Currently, this includes general routines involving dungeon grids,
 * dungeon rooms, the "flow" code, and extracting information from the
 * "dungeon" part of the screen, including determining which grids are
 * in line of sight.  See below.
 *
 * Also, we include a routine that attempts to "parse" object descriptions.
 * Currently, it does pretty well, though sometimes it stores a solitary
 * "tohit" or "todam" bonus as a "pval" (i.e. rings of accuracy/damage).
 * It seems to parse all artifacts and ego-items correctly.  But note
 * that we may fail on items with "excessively" long descriptions, where
 * "excessively" means 75 characters, or 60 characters in equip or shop.
 *
 * Note that the dungeon is assumed smaller than 256 by 256, and thus the
 * total number of possible rooms is assume smaller than 256*256.  In fact,
 * it is assumed to be smaller than 256*256-N, where N is the maximum number
 * of rooms that any one grid can belong to.  Since the dungeon is actually
 * only 200 by 70, we are fine.
 *
 * Currently, we reset the "BORG_WALL" flag every time we view the grid.
 * But often, if we see a "monster" in a wall, the wall is still there.
 * So perhaps we should not clear the "WALL" code if we see a "monster".
 * This would require knowledge about "monster" symbols, however.
 */



/*
 * Possibly problematic rooms (see "borg_build_room()"):
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
 *
 * See the "CROSS_ROOM" define in "borg.h"
 */


bool auto_ready = FALSE;	/* Initialized */

bool auto_active = FALSE;	/* Actually active */

int auto_room_max = 0;		/* First totally free room */

int c_x = 0;			/* Player location (X) */
int c_y = 0;			/* Player location (Y) */

int w_x = 0;			/* Current panel offset (X) */
int w_y = 0;			/* Current panel offset (Y) */

auto_grid **auto_grids;		/* Current "grid list" */

auto_room *auto_rooms;		/* Current "room list" */

auto_item *auto_items;		/* Current "inventory" */

auto_shop *auto_shops;		/* Current "shops" */

auto_room *auto_room_head;	/* &auto_rooms[0] */

auto_room *auto_room_tail;	/* &auto_rooms[AUTO_MAX_ROOMS-1] */

auto_grid *pg = NULL;		/* Player grid */



/*
 * Maintain a set of grids marked as "BORG_VIEW"
 */
u16b view_n = 0;
byte *view_y;
byte *view_x;


/*
 * Maintain a set of grids marked as "BORG_SEEN"
 */
u16b seen_n = 0;
byte *seen_y;
byte *seen_x;



/*
 * Constant "item description parsers" (singles)
 */
static int auto_size_single;		/* Number of "singles" */
static s16b *auto_what_single;		/* Kind indexes for "singles" */
static cptr *auto_text_single;		/* Textual prefixes for "singles" */

/*
 * Constant "item description parsers" (plurals)
 */
static int auto_size_plural;		/* Number of "plurals" */
static s16b *auto_what_plural;		/* Kind index for "plurals" */
static cptr *auto_text_plural;		/* Textual prefixes for "plurals" */

/*
 * Constant "item description parsers" (suffixes)
 */
static int auto_size_artego;		/* Number of "artegos" */
static s16b *auto_what_artego;		/* Indexes for "artegos" */
static cptr *auto_text_artego;		/* Textual prefixes for "artegos" */


/*
 * Log file
 */
FILE *auto_fff = NULL;		/* Log file */




/*
 * Query the "attr/chars" at a given location on the screen
 * We return "TRUE" only if a string of some form existed
 * Note that the string must be done in a single attribute.
 * We will not grab more than "ABS(n)" characters for the string.
 * If "n" is "positive", we will grab exactly "n" chars, or fail.
 * If "n" is "negative", we will grab until the attribute changes.
 * Note that "a" points to a single "attr", "s" to a string of "chars".
 *
 * Assume that the given location is actually on the screen!
 */
errr borg_what_text(int x, int y, int n, byte *a, char *s)
{
    int i;
    byte t_a;
    char t_c;

    /* Max length to scan for */
    int m = ABS(n);

    /* Hack -- Do not run off the screen */
    if (x + m > 80) m = 80 - x;


    /* Direct access to the screen */
    t_a = Term->scr->a[y][x];
    t_c = Term->scr->c[y][x];

    /* Save the attribute */
    (*a) = t_a;

    /* Scan for the rest */
    for (i = 0; i < m; i++) {

        /* Direct access to the screen */
        t_a = Term->scr->a[y][x+i];
        t_c = Term->scr->c[y][x+i];

        /* Verify the "attribute" (or stop) */
        if (t_a != (*a)) break;

        /* Save the first character */
        s[i] = t_c;
    }

    /* Terminate the string */
    s[i] = '\0';

    /* Too short */
    if ((n > 0) && (i != n)) return (1);

    /* Success */
    return (0);
}


/*
 * As above, but automatically convert all "blank" characters into
 * spaces of the appropriate "attr".  This includes leading blanks.
 *
 * Note that we do NOT strip final spaces, so this function will
 * very often read characters all the way to the end of the line.
 *
 * Assume that the given location is actually on the screen!
 */
errr borg_what_text_hack(int x, int y, int n, byte *a, char *s)
{
    int i;
    byte t_a;
    char t_c;

    /* Max length to scan for */
    int m = ABS(n);

    /* Hack -- Do not run off the screen */
    if (x + m > 80) m = 80 - x;


    /* Direct access to the screen */
    t_a = Term->scr->a[y][x];
    t_c = Term->scr->c[y][x];

    /* Save the attribute */
    (*a) = t_a;

    /* Scan for the rest */
    for (i = 0; i < m; i++) {

        /* Direct access to the screen */
        t_a = Term->scr->a[y][x+i];
        t_c = Term->scr->c[y][x+i];

        /* Hack -- Save the first usable attribute */
        if (!(*a)) (*a) = t_a;

        /* Hack -- Convert all "blanks" */
        if (t_c == ' ') t_a = (*a);

        /* Verify the "attribute" (or stop) */
        if (t_a != (*a)) break;

        /* Save the first character */
        s[i] = t_c;
    }

    /* Terminate the string */
    s[i] = '\0';

    /* Too short */
    if ((n > 0) && (i != n)) return (1);

    /* Success */
    return (0);
}




/*
 * Hack -- Dump info to a log file
 */
void borg_info(cptr what)
{
    /* Dump a log file message */
    if (auto_fff) fprintf(auto_fff, "%s\n", what);
}



/*
 * Hack -- Display (and save) a note to the user
 */
void borg_note(cptr what)
{
    /* Log the message */
    borg_info(what);

    /* Add to the message recall */
    message_new(what, -1);

    /* Do not use the recall window unless allowed */
    if (!use_recall_win || !term_recall) return;

    /* Hack -- dump the message in the recall window */
    Term_activate(term_recall);
    Term_clear();
    Term_putstr(0,0,-1,TERM_WHITE,what);
    Term_fresh();
    Term_activate(term_screen);
}


/*
 * Hack -- Stop processing on errors
 */
void borg_oops(cptr what)
{
    /* No longer active */
    auto_active = 0;

    /* Give a warning */
    borg_note(format("The BORG has broken (%s).", what));
}



/*
 * A Queue of keypresses to be sent
 */
char auto_key_queue[KEY_SIZE];
s16b auto_key_head = 0;
s16b auto_key_tail = 0;


/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypress(int k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Hack -- store the keypress */
    if (auto_fff) borg_info(format("Key: '%c'", k));

    /* Store the char, advance the queue */
    auto_key_queue[auto_key_head++] = k;

    /* Circular queue, handle wrap */
    if (auto_key_head == KEY_SIZE) auto_key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (auto_key_head == auto_key_tail) borg_oops("Overflow!");

    /* Hack -- Overflow may induce circular queue */
    if (auto_key_tail == KEY_SIZE) auto_key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Get the next Borg keypress
 */
char borg_inkey(void)
{
    int i;

    /* Nothing ready */
    if (auto_key_head == auto_key_tail) return (0);

    /* Extract the keypress, advance the queue */
    i = auto_key_queue[auto_key_tail++];

    /* Circular queue requires wrap-around */
    if (auto_key_tail == KEY_SIZE) auto_key_tail = 0;

    /* Return the key */
    return (i);
}



/*
 * Get the next Borg keypress
 */
void borg_flush(void)
{
    /* Hack -- store the keypress */
    borg_info("Flushing key-buffer.");

    /* Simply forget old keys */
    auto_key_tail = auto_key_head;
}



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
            ar->cost = 0;
            ar->x1 = ar->x2 = ar->x = 0;
            ar->y1 = ar->y2 = ar->y = 0;
            ar->prev = ar->next = 0;

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
    borg_note(format("Destroyed %d fake rooms.", n));

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
        ar->cost = 0;
        ar->x1 = ar->x2 = ar->x = 0;
        ar->y1 = ar->y2 = ar->y = 0;
        ar->prev = ar->next = 0;

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
    borg_note(format("Purged all %d rooms.", n));

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
        borg_note("Updating room list!");

        /* Try to free some rooms */
        if (!borg_free_room_update()) {

            /* Oops */
            borg_note("Purging room list!");

            /* Try to free some rooms */
            if (!borg_free_room_purge()) {

                /* Oops */
                borg_oops("Broken room list!");

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
    ar->cost = 0;
    ar->x1 = ar->x1 = ar->x = 0;
    ar->y1 = ar->y2 = ar->y = 0;
    ar->prev = ar->next = 0;

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
    else if (rand_int(2) == 0) {

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
    borg_note(format("Room (%d,%d to %d,%d)", x1, y1, x2, y2));

    /* XXX XXX XXX Hack -- clean up the "free room" list first */
    /* borg_free_room_update(); */

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
        ar->cost = 0;
        ar->x1 = ar->x2 = ar->x = 0;
        ar->y1 = ar->y2 = ar->y = 0;
        ar->prev = ar->next = 0;

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

        /* Note significant changes */
        if (ar->cost < 999) res = TRUE;

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
        ar->cost = 0;
        ar->x1 = ar->x2 = ar->x = 0;
        ar->y1 = ar->y2 = ar->y = 0;
        ar->prev = ar->next = 0;

        /* Add the room to the "free list" */
        auto_rooms[ar->self].free = auto_rooms[0].free;

        /* Add the room to the "free list" */
        auto_rooms[0].free = ar->self;
    }


    /* Result */
    return (res);
}



/*
 * Determine if a missile shot from (x1,y1) to (x2,y2) will arrive
 * at the final destination, assuming no monster gets in the way.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 */
bool borg_projectable(int x1, int y1, int x2, int y2)
{
    int dist, y, x;
    auto_grid *ag;

    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist < MAX_RANGE; dist++) {

        /* Get the grid */
        ag = grid(x,y);

        /* Never pass through walls */
        if (dist && (ag->info & BORG_WALL)) break;

        /* Hack -- assume unknown grids are walls */
        if (ag->o_c == ' ') break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
}


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
 * Extract the "NNN" from "(with NNN turns of lite)", or return (-1)
 */
static int extract_fuel(cptr tail)
{
    /* Require the prefix and suffix */
    if (!prefix(tail, "(with ")) return (-1);
    if (!suffix(tail, " of light)")) return (-1);

    /* Extract and return the "turns of lite" */
    return (atoi(tail + 6));
}


/*
 * Extract the "NNN" from "(NNN charges)", or return (-1)
 */
static int extract_charges(cptr tail)
{
    /* Require the prefix and suffix */
    if (!prefix(tail, "(")) return (-1); /* --(-- */
    if (!suffix(tail, " charge)") && !suffix(tail, " charges)")) return (-1);

    /* Extract and return the "charges" */
    return (atoi(tail + 1));
}



/*
 * Determine if an item kind is "easy" to identify
 * We can assume that the item kind is known.
 */
static bool obvious_kind(int kind)
{
    /* Analyze the "tval" */
    switch (k_list[kind].tval) {
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        case TV_FLASK:
        case TV_FOOD:
        case TV_POTION:
        case TV_SCROLL:
        case TV_SPIKE:
        case TV_SKELETON:
        case TV_BOTTLE:
        case TV_JUNK:
            return (TRUE);
    }

    /* Nope */
    return (FALSE);
}



/*
 * Return the slot that items of the given type are wielded into
 *
 * Note that "rings" are now automatically wielded into the left hand
 *
 * Returns "-1" if the item cannot (or should not) be wielded
 */
int borg_wield_slot(auto_item *item)
{
    /* Slot for equipment */
    switch (item->tval) {

        case TV_DRAG_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
            return (INVEN_BODY);

        case TV_CLOAK:
            return (INVEN_OUTER);

        case TV_SHIELD:
            return (INVEN_ARM);

        case TV_CROWN:
        case TV_HELM:
            return (INVEN_HEAD);

        case TV_GLOVES:
            return (INVEN_HANDS);

        case TV_BOOTS:
            return (INVEN_FEET);

        case TV_SWORD:
        case TV_POLEARM:
        case TV_HAFTED:
        case TV_DIGGING:
            return (INVEN_WIELD);

        case TV_BOW:
            return (INVEN_BOW);

        case TV_RING:
            return (INVEN_LEFT);

        case TV_AMULET:
            return (INVEN_NECK);

        case TV_LITE:
            return (INVEN_LITE);
    }

    /* No slot available */
    return (-1);
}




/*
 * Analyze an auto_item based on a description and cost
 *
 * We do a simple binary search on the arrays of object base names,
 * relying on the fact that they are sorted in reverse order, and on
 * the fact that efficiency is only important when the parse succeeds
 * (which it always does), and on some facts about "prefixes".
 *
 * Note that we will fail if the "description" was "partial", though
 * we will correctly handle a description with a "partial inscription",
 * so the actual item description must exceed 75 chars for us to fail,
 * though we will only get 60 characters if the item is in the equipment
 * or in a shop, and the "long" description items tend to get worn.  :-)
 */
void borg_item_analyze(auto_item *item, cptr desc, cptr cost)
{
    int i, j, m, n;

    char *scan;
    char *tail;

    char temp[128];

    char c1 = '{'; /* c2 = '}' */
    char p1 = '(', p2 = ')';
    char b1 = '[', b2 = ']';


    /* Wipe it */
    WIPE(item, auto_item);


    /* Save the item description */
    strcpy(item->desc, desc);

    /* Extract the item cost */
    item->cost = atol(cost);


    /* Advance to the "inscription" or end of string */
    for (scan = item->desc; *scan && *scan != c1; scan++);

    /* Save a pointer to the inscription */
    item->note = scan;


    /* Do not process empty items */
    if (!desc[0]) return;


    /* Assume singular */
    item->iqty = 1;

    /* Notice prefix "a " */
    if ((desc[0] == 'a') && (desc[1] == ' ')) {

        /* Skip "a " */
        desc += 2;
    }

    /* Notice prefix "a " */
    else if ((desc[0] == 'a') && (desc[1] == 'n') && (desc[2] == ' ')) {

        /* Skip "an " */
        desc += 3;
    }

    /* Notice prefix "The " */
    else if ((desc[0] == 'T') && (desc[1] == 'h') &&
             (desc[2] == 'e') && (desc[3] == ' ')) {

        /* Skip "The " */
        desc += 4;
    }

    /* Notice "numerical" prefixes */
    else if (isdigit(desc[0])) {

        cptr s;

        /* Find the first space */
        for (s = desc; *s && (*s != ' '); s++);

        /* Paranoia -- Catch sillyness */
        if (*s != ' ') return;

        /* Extract a quantity */
        item->iqty = atoi(desc);

        /* Skip the quantity and space */
        desc = s + 1;
    }


    /* Paranoia -- catch "broken" descriptions */
    if (!desc[0]) return;

    /* Obtain a copy of the description */
    strcpy(temp, desc);

    /* Advance to the "inscription" or end of string */
    for (scan = temp; *scan && (*scan != c1); scan++);

    /* Nuke the space before the inscription */
    if ((scan[0] == c1) && (scan[-1] == ' ')) *--scan = '\0';

    /* Note that "scan" points at the "tail" of "temp" */

    /* Hack -- non-aware, singular, flavored items */
    if (item->iqty == 1) {
        if (prefix(temp, "Scroll titled ")) item->tval = TV_SCROLL;
        else if (streq(scan-7, " Potion")) item->tval = TV_POTION;
        else if (streq(scan-6, " Staff")) item->tval = TV_STAFF;
        else if (streq(scan-5, " Wand")) item->tval = TV_WAND;
        else if (streq(scan-4, " Rod")) item->tval = TV_ROD;
        else if (streq(scan-5, " Ring")) item->tval = TV_RING;
        else if (streq(scan-7, " Amulet")) item->tval = TV_AMULET;
        else if (streq(scan-9, " Mushroom")) item->tval = TV_FOOD;
        else if (streq(scan-11, " Hairy Mold")) item->tval = TV_FOOD;
    }

    /* Hack -- non-aware, plural, flavored items */
    else {
        if (prefix(temp, "Scrolls titled ")) item->tval = TV_SCROLL;
        else if (streq(scan-8, " Potions")) item->tval = TV_POTION;
        else if (streq(scan-7, " Staffs")) item->tval = TV_STAFF;
        else if (streq(scan-6, " Wands")) item->tval = TV_WAND;
        else if (streq(scan-5, " Rods")) item->tval = TV_ROD;
        else if (streq(scan-6, " Rings")) item->tval = TV_RING;
        else if (streq(scan-8, " Amulets")) item->tval = TV_AMULET;
        else if (streq(scan-10, " Mushrooms")) item->tval = TV_FOOD;
        else if (streq(scan-12, " Hairy Molds")) item->tval = TV_FOOD;
    }

    /* Accept non-aware flavored objects */
    if (item->tval) return;


    /* Start at the beginning */
    tail = temp;

    /* Check singular items */
    if (item->iqty == 1) {

        /* Start the search */
        m = 0; n = auto_size_single;

        /* Simple binary search */
        while (m < n - 4) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Found a new minimum */
            if (strcmp(tail, auto_text_single[i]) < 0) {
                m = i;
            }

            /* Found a new maximum */
            else {
                n = i;
            }
        }

        /* Search for a prefix */
        for (i = m; i < auto_size_single; i++) {

            /* Check for proper prefix */
            if (prefix(tail, auto_text_single[i])) break;
        }

        /* Oops.  Bizarre item. */
        if (i >= auto_size_single) {
            borg_oops("Bizarre object!");
            return;
        }

        /* Save the item kind */
        item->kind = auto_what_single[i];

        /* Skip past the base name */
        tail += strlen(auto_text_single[i]);
    }

    /* Check plural items */
    else {

        /* Start the search */
        m = 0; n = auto_size_plural;

        /* Simple binary search */
        while (m < n - 4) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Found a new minimum */
            if (strcmp(tail, auto_text_plural[i]) < 0) {
                m = i;
            }

            /* Found a new maximum */
            else {
                n = i;
            }
        }

        /* Search for a prefix */
        for (i = m; i < auto_size_plural; i++) {

            /* Check for proper prefix */
            if (prefix(tail, auto_text_plural[i])) break;
        }

        /* Oops.  Bizarre item. */
        if (i >= auto_size_plural) {
            borg_oops("Bizarre object!");
            return;
        }

        /* Save the item kind */
        item->kind = auto_what_plural[i];

        /* Skip past the base name */
        tail += strlen(auto_text_plural[i]);
    }


    /* Extract some info */
    item->tval = k_list[item->kind].tval;
    item->sval = k_list[item->kind].sval;


    /* Hack -- check for ego-items and artifacts */
    if ((tail[0] == ' ') &&
        (item->tval >= TV_MIN_WEAR) && (item->tval <= TV_MAX_WEAR)) {

        /* Start the search */
        m = 0; n = auto_size_artego;

        /* XXX XXX XXX Binary search */
        while (m < n - 4) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Found a new minimum */
            if (strcmp(tail, auto_text_artego[i]) < 0) {
                m = i;
            }

            /* Found a new maximum */
            else {
                n = i;
            }
        }

        /* XXX XXX XXX Search for a prefix */
        for (i = m; i < m + 12; i++) {

            /* Check for proper prefix */
            if (prefix(tail, auto_text_artego[i])) {

                /* Paranoia -- Item is known */
                item->able = TRUE;

                /* Save the artifact name */
                if (auto_what_artego[i] > 0) {
                    item->name1 = auto_what_artego[i];
                }

                /* Save the ego-item name */
                else {
                    item->name2 = 0 - auto_what_artego[i];
                }

                /* Skip the space and the ego-item name */
                tail += strlen(auto_text_artego[i]);

                /* Done */
                break;
            }
        }


        /* Hack -- examine ego-items */
        if (item->name2) {

            /* XXX XXX Hack -- fix weird "missiles" */
            if ((item->tval == TV_BOLT) ||
                (item->tval == TV_ARROW) ||
                (item->tval == TV_SHOT)) {

                /* Fix missile ego-items */
                if (item->name2 == EGO_FIRE) {
                    item->name2 = EGO_AMMO_FIRE;
                }
                else if (item->name2 == EGO_SLAYING) {
                    item->name2 = EGO_AMMO_SLAYING;
                }
                else if (item->name2 == EGO_SLAY_EVIL) {
                    item->name2 = EGO_AMMO_EVIL;
                }
            }

            /* XXX XXX Hack -- fix weird "robes" */
            if ((item->tval == TV_SOFT_ARMOR) &&
                (item->sval == SV_ROBE)) {

                /* Fix "robes of the magi" */
                if (item->name2 == EGO_MAGI) {
                    item->name2 = EGO_ROBE_MAGI;
                }
            }
        }


        /* Hack -- examine artifacts */
        if (item->name1) {

            /* XXX XXX Hack -- fix "weird" artifacts */
            if ((item->tval != v_list[item->name1].tval) ||
                (item->sval != v_list[item->name1].sval)) {

                /* Find the correct "kind" */
                for (j = 0; j < MAX_K_IDX; j++) {

                    /* Found the correct "kind" */
                    if ((k_list[j].tval == v_list[item->name1].tval) &&
                        (k_list[j].sval == v_list[item->name1].sval)) {

                        /* Save the kind */
                        item->kind = j;

                        /* Save the tval/sval */
                        item->tval = k_list[j].tval;
                        item->sval = k_list[j].sval;

                        /* Stop looking */
                        break;
                    }
                }
            }
        }
    }


    /* Mega-Hack -- skip spaces */
    while (tail[0] == ' ') tail++;


    /* XXX XXX Hack -- Chests are too complicated */
    if (item->tval == TV_CHEST) {
        return;
    }


    /* Hack -- Some kinds of objects are always obvious */
    if (obvious_kind(item->kind)) {
        item->able = TRUE;
        return;
    }

    /* Hack -- Examine Wands/Staffs for charges */
    if ((item->tval == TV_WAND) || (item->tval == TV_STAFF)) {
        i = extract_charges(tail);
        if (i >= 0) item->able = TRUE;
        if (item->able) item->pval = i;
        return;
    }

    /* Mega-Hack -- Examine Rods for charging */
    if (item->tval == TV_ROD) {

        /* Rods are always known (if aware) */
        item->able = TRUE;

        /* Mega-Hack -- fake "charges" */
        item->pval = 1;

        /* Mega-Hack -- "charging" means no "charges" */
        if (streq(tail, "(charging)")) item->pval = 0;
    }


    /* Hack -- Extract Lites for Light */
    if (item->tval == TV_LITE) {

        /* Fuels yields known (and fuel) */
        i = extract_fuel(tail);
        if (i >= 0) item->pval = i;
        if (i >= 0) item->able = TRUE;

        /* Hack -- Artifacts have infinite fuel */
        if (item->name1) item->pval = 29999;
        if (item->name1) item->able = TRUE;

        /* Done */
        return;
    }


    /* Wearable stuff */
    if ((item->tval >= TV_MIN_WEAR) && (item->tval <= TV_MAX_WEAR)) {

        bool done = FALSE;

        int d1 = 0, d2 = 0, ac = 0, th = 0, td = 0, ta = 0;


        /* Hack -- examine the "easy know" flag */
        if (k_list[item->kind].flags3 & TR3_EASY_KNOW) {
            item->able = TRUE;
        }


        /* Must have a suffix */
        if (!tail[0]) return;


        /* Parse "weapon-style" damage strings */
        if ((tail[0] == p1) &&
            ((item->tval == TV_HAFTED) ||	
             (item->tval == TV_POLEARM) ||	
             (item->tval == TV_SWORD) ||	
             (item->tval == TV_DIGGING) ||	
             (item->tval == TV_BOLT) ||	
             (item->tval == TV_ARROW) ||	
             (item->tval == TV_SHOT))) {

            /* First extract the damage string */
            for (scan = tail; *scan != p2; scan++); scan++;

            /* Hack -- Notice "end of string" */
            if (scan[0] != ' ') done = TRUE;

            /* Terminate the string and advance */
            *scan++ = '\0';

            /* Parse the damage string, or stop XXX */
            if (sscanf(tail, "(%dd%d)", &d1, &d2) != 2) return;

            /* Save the values */
            item->dd = d1; item->ds = d2;

            /* No extra information means not identified */
            if (done) return;

            /* Skip the "damage" info */
            tail = scan;
        }

        /* Parse the "damage" string for bows */
        else if ((tail[0] == p1) &&
                 (item->tval == TV_BOW)) {

            /* First extract the damage string */
            for (scan = tail; *scan != p2; scan++); scan++;

            /* Hack -- Notice "end of string" */
            if (scan[0] != ' ') done = TRUE;

            /* Terminate the string and advance */
            *scan++ = '\0';

            /* Parse the multiplier string, or stop */
            if (sscanf(tail, "(x%d)", &d1) != 1) return;

            /* Hack -- save it in "damage dice" */
            item->dd = d1;

            /* No extra information means not identified */
            if (done) return;

            /* Skip the "damage" info */
            tail = scan;
        }


        /* Parse the "bonus" string */
        if (tail[0] == p1) {

            /* Extract the extra info */
            for (scan = tail; *scan != p2; scan++); scan++;

            /* Hack -- Notice "end of string" */
            if (scan[0] != ' ') done = TRUE;

            /* Terminate the damage, advance */
            *scan++ = '\0';

            /* Parse standard "bonuses" */
            if (sscanf(tail, "(%d,%d)", &th, &td) == 2) {
                item->to_h = th; item->to_d = td;
                item->able = TRUE;
            }

            /* XXX XXX Hack -- assume non-final bonuses are "to_hit" */
            else if (!done && sscanf(tail, "(%d)", &th) == 1) {
                item->to_h = th;
                item->able = TRUE;
            }

            /* XXX XXX Hack -- assume final bonuses are "pval" codes */
            else if (done) {
                item->pval = atoi(tail + 1);
                item->able = TRUE;
            }

            /* Oops */
            else {
                return;
            }

            /* Nothing left */
            if (done) return;

            /* Skip the "damage bonus" info */
            tail = scan;
        }


        /* Parse the "bonus" string */
        if (tail[0] == b1) {

            /* Extract the extra info */
            for (scan = tail; *scan != b2; scan++); scan++;

            /* Hack -- Notice "end of string" */
            if (scan[0] != ' ') done = TRUE;

            /* Terminate the armor string, advance */
            *scan++ = '\0';

            /* Parse the armor, and bonus */
            if (sscanf(tail, "[%d,%d]", &ac, &ta) == 2) {
                item->ac = ac;
                item->to_a = ta;
                item->able = TRUE;
            }

            /* Negative armor bonus */
            else if (sscanf(tail, "[-%d]", &ta) == 1) {
                item->to_a = ta;
                item->able = TRUE;
            }

            /* Positive armor bonus */
            else if (sscanf(tail, "[+%d]", &ta) == 1) {
                item->to_a = ta;
                item->able = TRUE;
            }

            /* Just base armor */
            else if (sscanf(tail, "[%d]", &ac) == 1) {
                item->ac = ac;
            }

            /* Oops */
            else {
                return;
            }

            /* Nothing left */
            if (done) return;

            /* Skip the "armor" data */
            tail = scan;
        }


        /* Parse the final "pval" string, if any */
        if (tail[0] == p1) {

            /* Assume identified */
            item->able = TRUE;

            /* Grab it */
            item->pval = atoi(tail + 1);
        }
    }
}



/*
 * This function "guesses" at the "value" of non-aware items
 */
static s32b borg_item_value_base(auto_item *item)
{
    /* Aware items can assume template cost */
    if (item->kind) return (k_list[item->kind].cost);

    /* Unknown food is cheap */
    if (item->tval == TV_FOOD) return (1L);

    /* Unknown Scrolls are cheap */
    if (item->tval == TV_SCROLL) return (20L);

    /* Unknown Potions are cheap */
    if (item->tval == TV_POTION) return (20L);

    /* Unknown Rings are cheap */
    if (item->tval == TV_RING) return (45L);

    /* Unknown Amulets are cheap */
    if (item->tval == TV_AMULET) return (45L);

    /* Unknown Wands are Cheap */
    if (item->tval == TV_WAND) return (50L);

    /* Unknown Staffs are Cheap */
    if (item->tval == TV_STAFF) return (70L);

    /* Unknown Rods are Cheap */
    if (item->tval == TV_ROD) return (75L);

    /* Hack -- Oops */
    return (0L);
}



/*
 * Hack -- bonus "cost" of (non-worthless) ego-items
 */
s32b auto_ego_item_cost[EGO_MIN_WORTHLESS] = {

    0L		/* NULL */,
    12500L	/* "of Resistance" */,
    1000L	/* "of Resist Acid" */,
    600L	/* "of Resist Fire" */,
    600L	/* "of Resist Cold" */,
    500L	/* "of Resist Lightning" */,
    20000L	/* "(Holy Avenger)" */,
    15000L	/* "(Defender)" */,
    2000L	/* "of Animal Slaying" */,	/* weapon */
    4000L	/* "of Dragon Slaying" */,	/* weapon */
    4000L	/* "of Slay Evil" */,		/* XXX - weapon */
    3000L	/* "of Slay Undead" */,		/* XXX - weapon */
    3000L	/* "of Flame" */,
    2500L	/* "of Frost" */,
    1000L	/* "of Free Action" */,
    1500L	/* "of Slaying" */,
    0L		/* NULL */,
    0L		/* NULL */,
    250L	/* "of Slow Descent" */,
    200000L	/* "of Speed" */,		/* XXX */
    500L	/* "of Stealth" */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    500L	/* "of Intelligence" */,
    500L	/* "of Wisdom" */,
    500L	/* "of Infra-Vision" */,
    2000L	/* "of Might" */,
    2000L	/* "of Lordliness" */,
    7500L	/* "of the Magi" */,
    1000L	/* "of Beauty" */,
    1000L	/* "of Seeing" */,		/* XXX */
    1500L	/* "of Regeneration" */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    30000L	/* "of the Magi" */,		/* robe (new) */
    250L	/* "of Protection" */,		/* cloak */
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    2000L	/* "of Fire" */,
    25L		/* "of Slay Evil" */,		/* ammo */
    35L		/* "of Slay Dragon" */,		/* ammo */
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    25L		/* "of Fire" */,		/* ammo (new) */
    0L		/* NULL */,
    45L		/* "of Slaying" */,		/* ammo (new) */
    0L		/* NULL */,
    0L		/* NULL */,
    30L		/* "of Slay Animal" */,		/* ammo */
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    10000L	/* "of Extra Might" */,		/* launcher */
    10000L	/* "of Extra Shots" */,		/* launcher */
    0L		/* NULL */,
    0L		/* NULL */,
    1000L	/* "of Velocity" */,		/* launcher (new) */
    1000L	/* "of Accuracy" */,		/* launcher */
    0L		/* NULL */,
    1200L	/* "of Orc Slaying" */,		/* weapon */
    2500L	/* "of Power" */,		/* gloves */
    0L		/* NULL */,
    0L		/* NULL */,			
    20000L	/* "of Westernesse" */,		/* XXX */
    5000L	/* "(Blessed)" */,
    1200L	/* "of Demon Slaying" */,	/* weapon */
    1200L	/* "of Troll Slaying" */,	/* weapon */
    0L		/* NULL */,			
    0L		/* NULL */,
    30L		/* "of Wounding" */,		/* ammo */
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    500L	/* "of Light" */,
    1000L	/* "of Agility" */,
    0L		/* NULL */,
    0L		/* NULL */,
    1200L	/* "of Giant Slaying" */,	/* weapon */
    50000L	/* "of Telepathy" */,
    15000L	/* "of Elvenkind" */,
    0L		/* NULL */,
    0L		/* NULL */,
    10000L	/* "of Extra Attacks" */,
    4000L	/* "of Aman" */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
    0L		/* NULL */,
};




/*
 * Determine the base price of a known item (see below)
 */
static s32b borg_item_value_known(auto_item *item)
{
    s32b value;


    /* Extract the base value */
    value = k_list[item->kind].cost;

    /* Known worthless items are worthless */
    if (value <= 0L) return (0L);


    /* Hack -- use artifact base costs */
    if (item->name1) value = v_list[item->name1].cost;

    /* Known worthless items are worthless */
    if (value <= 0L) return (0L);


    /* Hack -- catch worthless ego-items */
    if (item->name2 >= EGO_MIN_WORTHLESS) return (0L);

    /* Mega-Hack -- guess at ego-item bonus cost */
    if (item->name2) value += auto_ego_item_cost[item->name2];


    /* Wands/Staffs -- pay extra for charges */
    if ((item->tval == TV_WAND) ||
        (item->tval == TV_STAFF)) {

        /* Reward charges */
        value += ((value / 20) * item->pval);
    }

    /* Rings/Amulets -- pay extra for bonuses */
    else if ((item->tval == TV_RING) ||
             (item->tval == TV_AMULET)) {

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);

        /* XXX XXX Ring of Speed */
    }

    /* Armour -- pay extra for armor bonuses */
    else if ((item->tval == TV_BOOTS) ||
             (item->tval == TV_GLOVES) ||
             (item->tval == TV_CLOAK) ||
             (item->tval == TV_HELM) ||
             (item->tval == TV_CROWN) ||
             (item->tval == TV_SHIELD) ||
             (item->tval == TV_SOFT_ARMOR) ||
             (item->tval == TV_HARD_ARMOR) ||
             (item->tval == TV_DRAG_ARMOR)) {

        /* Hack -- negative armor bonus */
        if (item->to_a < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);
    }

    /* Weapons -- pay extra for all three bonuses */
    else if ((item->tval == TV_DIGGING) ||
             (item->tval == TV_HAFTED) ||
             (item->tval == TV_SWORD) ||
             (item->tval == TV_POLEARM)) {

        /* Allow negatives to be overcome */
        if (item->to_h + item->to_d < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);
    }

    /* Bows -- pay extra for all three bonuses */
    else if (item->tval == TV_BOW) {

        /* Allow negatives to be overcome */
        if (item->to_h + item->to_d < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);
    }

    /* Ammo -- pay extra for all three bonuses.  Hack -- 1/20 normal weapons */
    else if ((item->tval == TV_SHOT) ||
             (item->tval == TV_ARROW) ||
             (item->tval == TV_BOLT)) {

        /* Allow negatives to be overcome */
        if (item->to_h + item->to_d < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
    }


    /* Return the value */
    return (value);
}


/*
 * Determine the approximate "price" of one instance of an item
 * Adapted from "store.c:item_value()" and "object.c:apply_magic()".
 *
 * This routine is used to determine how much gold the Borg would get
 * for selling an item, ignoring "charisma" related issues, and the
 * maximum purchase limits of the various shop-keepers.
 *
 * This function correctly handles artifacts and ego-items, though
 * it will (slightly) undervalue a few ego-items.  This function will
 * not "sufficiently" reward rings/boots of speed, though they are
 * already worth much, much more than any other objects.
 *
 * This function correctly handles "cursed" items, and attempts to
 * apply relevant "discounts" if known.
 *
 * This function is remarkably accurate, considering the complexity...
 */
s32b borg_item_value(auto_item *item)
{
    s32b value;

    int discount = 0;


    /* Non-aware items */
    if (item->kind && item->able) {

        /* Process various fields */
        value = borg_item_value_known(item);
    }

    /* Known items */
    else {

        /* Do what "store.c" does */
        value = borg_item_value_base(item);
    }

    /* Worthless items */
    if (value <= 0L) return (0L);


    /* Parse various "inscriptions" */
    if (item->note[0]) {

        /* Cursed indicators */
        if (streq(item->note, "{cursed}")) return (0L);
        if (streq(item->note, "{terrible}")) return (0L);
        if (streq(item->note, "{worthless}")) return (0L);

        /* Ignore certain feelings */
        /* "{average}" */
        /* "{blessed}" */
        /* "{good}" */
        /* "{excellent}" */
        /* "{special}" */

        /* Ignore special inscriptions */
        /* "{empty}", "{tried}" */

        /* Special "discount" */
        if (streq(item->note, "{on sale}")) discount = 50;

        /* Standard "discounts" */
        else if (streq(item->note, "{25% off}")) discount = 25;
        else if (streq(item->note, "{50% off}")) discount = 50;
        else if (streq(item->note, "{75% off}")) discount = 75;
        else if (streq(item->note, "{90% off}")) discount = 90;
    }


    /* Apply "discount" if any */
    if (discount) value -= value * discount / 100;


    /* Return the value */
    return (value);
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
 * Hack -- compute the "cost" of the given grid
 */
int borg_flow_cost(int x, int y)
{
    auto_room *ar;

    int cost, best = 999;

    /* Scan all the rooms holding this room */
    for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

        int dx = ar->x - x;
        int dy = ar->y - y;

        int ax = ABS(dx);
        int ay = ABS(dy);

        /* Calculate the cost */
        cost = ar->cost + MAX(ax, ay);

        /* Calculate the cost */
        if (cost >= best) continue;

        /* Save the best cost */
        best = cost;
    }

    /* Return the cost */
    return (best);
}





/*
 * Hack -- maximum flow cost
 */
static int auto_flow_max = 999;



/*
 * Determine if the queue is empty
 */
static bool borg_flow_empty(void)
{
    /* Test for Empty Queue. */
    if (auto_room_head->next == auto_room_tail->self) return (TRUE);

    /* Must not be empty */
    return (FALSE);
}



/*
 * Dequeue from the "priority queue" of "auto_flow" records
 */
static auto_room *borg_flow_dequeue(void)
{
    auto_room *prev = auto_room_head;
    auto_room *this = &auto_rooms[prev->next];
    auto_room *next = &auto_rooms[this->next];

    /* Oops -- The queue is empty */
    if (this == next) return (NULL);

    /* Dequeue the entry */
    prev->next = next->self;
    next->prev = prev->self;

    /* No longer in the queue */
    this->next = this->prev = 0;

    /* Return the room */
    return (this);
}


/*
 * Enqueue into the "priority queue" of "auto_flow" records
 */
static void borg_flow_enqueue(auto_room *ar)
{
    auto_room *prev = auto_room_head;
    auto_room *next = &auto_rooms[prev->next];

    /* Find a "good" location in the queue */
    while (ar->cost >= next->cost) {
        prev = next;
        next = &auto_rooms[prev->next];
    }

    /* Tell the node */
    ar->prev = prev->self;
    ar->next = next->self;

    /* Tell the queue */
    prev->next = ar->self;
    next->prev = ar->self;
}


/*
 * Enqueue a fresh starting grid
 */
void borg_flow_enqueue_grid(int x, int y)
{
    auto_room *ar;

    /* Scan all the rooms that hold it */
    for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

        /* New room */
        if (ar->cost) {

            /* Mark rooms as "cheap" */
            ar->cost = 0;

            /* Save the location */
            ar->x = x; ar->y = y;

            /* Enqueue the room */
            borg_flow_enqueue(ar);
        }

        /* Old room, better location */
        else {

            int new = double_distance(c_y, c_x, y, x);
            int old = double_distance(c_y, c_x, ar->y, ar->x);

            /* Use the "closest" grid */
            if (new < old) {
                ar->x = x; ar->y = y;
            }
        }
    }
}



/*
 * Aux function -- Given a room "ar1" and a location (x,y) along the
 * edge of that room, and a "direction" of motion, find all rooms that
 * include the resulting grid, and attempt to enqueue them.
 *
 * Looks like we have to make sure that crossing rooms do not attempt
 * to recurse back on themselves or anything weird.
 */
static void borg_flow_spread_aux(auto_room *ar1, int x, int y, int d)
{
    int cost;

    int dx = ar1->x - x;
    int dy = ar1->y - y;

    int ax = ABS(dx);
    int ay = ABS(dy);

    int x2 = x + ddx[d];
    int y2 = y + ddy[d];

    auto_grid *ag;
    auto_room *ar;


    /* Look to the given direction */
    ag = grid(x2, y2);

    /* Ignore walls */
    if (ag->info & BORG_WALL) return;

    /* Ignore unroomed grids */
    if (!ag->room) return;

    /* Extract the "approximate" travel cost XXX XXX */
    /* Could use "ABS(ddx[d]) + ABS(ddy[d])" instead of "1" */
    cost = ar1->cost + MAX(ax, ay) + 1;

    /* Efficiency -- do not enqueue "deep" flows */
    if (cost > auto_flow_max) return;

    /* Now find every room containing that grid */
    for (ar = room(1,x2,y2); ar; ar = room(0,0,0)) {

#if 0
        /* Paranoia -- Skip rooms that contain the destination */
        if ((ar->x1 <= x) && (x <= ar->x2) &&
            (ar->y1 <= y) && (y <= ar->y2)) {

            borg_note("Overlap");
            continue;
        }
#endif

        /* Do not "lose" previous gains */
        if (cost >= ar->cost) continue;

        /* Save the "destination" location */
        ar->x = x; ar->y = y;

        /* Save the new "cheaper" cost */
        ar->cost = cost;

        /* Mega-Hack -- Handle "upgrades" */
        if (ar->next) {

            auto_room *prev = &auto_rooms[ar->prev];
            auto_room *next = &auto_rooms[ar->next];

            /* Efficiency -- no change needed */
            if (prev->cost <= ar->cost) continue;

            /* Dequeue the entry */
            prev->next = next->self;
            next->prev = prev->self;

            /* No longer in the queue */
            ar->next = ar->prev = 0;
        }

        /* Enqueue the room */
        borg_flow_enqueue(ar);
    }
}


/*
 * Spread the "flow", assuming some rooms have been enqueued.
 *
 * We assume that the "flow" fields of all the rooms have been
 * initialized via "borg_flow_clear()", and that then the "flow"
 * fields of the rooms in the queue were set to "zero" before
 * being enqueued (see below).
 *
 * We assume that none of the icky grids have the "prev" or "next"
 * pointers set to anything, which is a pretty safe assumption.
 */
void borg_flow_spread(bool optimize)
{
    int x, y;

    auto_room *ar;


    /* Hack -- Reset the max depth */
    auto_flow_max = 999;

    /* Keep going until the queue is empty */
    while (!borg_flow_empty()) {

        /* Dequeue the next room */	
        ar = borg_flow_dequeue();

        /* Scan the south/north edges */
        for (x = ar->x1; x <= ar->x2; x++) {

            /* South edge */
            y = ar->y2;

            /* Look to the south */
            borg_flow_spread_aux(ar, x, y, 2);

            /* North edge */
            y = ar->y1;

            /* Look to the north */
            borg_flow_spread_aux(ar, x, y, 8);
        }

        /* Scan the east/west edges */
        for (y = ar->y1; y <= ar->y2; y++) {

            /* East edge */
            x = ar->x2;

            /* Look to the east */
            borg_flow_spread_aux(ar, x, y, 6);

            /* West edge */
            x = ar->x1;

            /* Look to the west */
            borg_flow_spread_aux(ar, x, y, 4);
        }


        /* Look along the South-East corner */
        borg_flow_spread_aux(ar, ar->x2, ar->y2, 3);

        /* Look along the South-West corner */
        borg_flow_spread_aux(ar, ar->x1, ar->y2, 1);

        /* Look along the North-East corner */
        borg_flow_spread_aux(ar, ar->x2, ar->y1, 9);

        /* Look along the North-West corner */
        borg_flow_spread_aux(ar, ar->x1, ar->y1, 7);


        /* Optimize if requested */
        if (optimize) {

            /* Efficiency -- maintain maximum necessary depth */
            for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

                /* Maintain maximum depth */
                if (ar->cost < auto_flow_max) auto_flow_max = ar->cost;
            }
        }
    }
}


/*
 * Reset the "flow codes" of all "real" rooms to a large number
 */
void borg_flow_clear(void)
{
    int i;

    /* Reset the cost of all the (real) rooms */
    for (i = 1; i < auto_room_max; i++) {

        /* Get the room */
        auto_room *ar = &auto_rooms[i];

        /* Skip "dead" rooms */	
        if (ar->free) continue;

        /* Initialize the "flow" */
        ar->cost = 999;

        /* Clear the pointers */
        ar->next = ar->prev = 0;
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
 * Set the "view" flag of the given cave grid
 * Never call this function when the "view" array is full.
 * Never call this function with an "illegal" location.
 */
static void borg_cave_view(int x, int y)
{
    auto_grid *ag = grid(x,y);

    /* Can only be set once */
    if (ag->info & BORG_VIEW) return;

    /* Set the flag */
    ag->info |= BORG_VIEW;

    /* Add to queue */
    view_y[view_n] = y;
    view_x[view_n] = x;
    view_n++;
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
void borg_update_view(void)
{
    int n, m, d, k, y, x;

    int se, sw, ne, nw, es, en, ws, wn;

    int over, full;


    /* Start with full vision */
    full = MAX_SIGHT;

    /* Extract the "octagon" limits */
    over = full * 3 / 2;


    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < view_n; n++) {

        int y = view_y[n];
        int x = view_x[n];

        auto_grid *ag = grid(x,y);

        /* Mark the grid as not in "view" */
        ag->info &= ~(BORG_VIEW | BORG_XTRA);
    }

    /* Start over with the "view" array */
    view_n = 0;


    /*** Step 1 -- adjacent grids ***/

    /* Now start on the player */
    x = c_x; y = c_y;

    /* Assume the player grid is easily viewable */
    pg->info |= BORG_XTRA;

    /* Assume the player grid is viewable */
    borg_cave_view(x,y);


    /*** Step 2 -- Major Diagonals ***/

    /* Scan south-east */
    for (d = 1; d <= full; d++) {
        grid(x+d,y+d)->info |= BORG_XTRA;
        borg_cave_view(x+d,y+d);
        if (grid(x+d,y+d)->info & BORG_WALL) break;
    }

    /* Scan south-west */
    for (d = 1; d <= full; d++) {
        grid(x-d,y+d)->info |= BORG_XTRA;
        borg_cave_view(x-d,y+d);
        if (grid(x-d,y+d)->info & BORG_WALL) break;
    }

    /* Scan north-east */
    for (d = 1; d <= full; d++) {
        grid(x+d,y-d)->info |= BORG_XTRA;
        borg_cave_view(x+d,y-d);
        if (grid(x+d,y-d)->info & BORG_WALL) break;
    }

    /* Scan north-west */
    for (d = 1; d <= full; d++) {
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
 * Clear the viewable space
 */
void borg_forget_view(void)
{
    int i;

    /* None to forget */
    if (!view_n) return;

    /* Clear them all */
    for (i = 0; i < view_n; i++) {

        int y = view_y[i];
        int x = view_x[i];

        /* Forget that the grid is viewable */
        grid(x,y)->info &= ~BORG_VIEW;
        grid(x,y)->info &= ~BORG_XTRA;
    }

    /* None left */
    view_n = 0;
}





/*
 * Previous information
 */
static int o_w_x, o_w_y;	/* Previous panel */
static int o_c_x, o_c_y;	/* Previous location */
static int wiped = FALSE;	/* Arrays are wiped */


/*
 * Update the Borg based on the current "map"
 * Wipe all arrays if "wipe" is TRUE
 */
void borg_forget_map(void)
{
    int i, x, y;

    auto_grid *ag;
    auto_room *ar;


    /* Wipe the arrays */
    if (!wiped) {

        /* Clean up the shops */
        for (i = 0; i < 8; i++) {

            /* Hack -- not visited */
            auto_shops[i].visit = 0;
        }

        /* Clean up the old used rooms */
        for (i = 1; i < auto_room_max; i++) {

            /* Access the room */
            ar = &auto_rooms[i];

            /* Place the room in the free room list */
            ar->free = i + 1;

            /* Remove the room from the priority queue */
            ar->next = ar->prev = 0;

            /* No flow cost */
            ar->cost = 0;

            /* No flow location */
            ar->x = ar->y = 0;

            /* No location */
            ar->x1 = ar->x2 = ar->y1 = ar->y2 = 0;

            /* Never seen it */
            ar->when = 0L;
        }

        /* Reset the free list */
        auto_room_head->free = 1;

        /* Maximum room index */
        auto_room_max = 1;


        /* Clean up the grids */
        for (y = 0; y < AUTO_MAX_Y; y++) {
            for (x = 0; x < AUTO_MAX_X; x++) {

                /* Access the grid */
                ag = grid(x, y);

                /* No room */
                ag->room = 0;

                /* Black */
                ag->o_a = 0;

                /* Unknown */
                ag->o_c = ' ';

                /* No bit flags */
                ag->info = 0;

                /* No extra info */
                ag->xtra = 0;
            }
        }

        /* Hack -- mark the edge of the map as walls */
        for (y = 0; y < AUTO_MAX_Y; y++) {

            /* West edge */
            grid(0,y)->info |= BORG_WALL;

            /* East edge */
            grid(AUTO_MAX_X-1,y)->info |= BORG_WALL;
        }	

        /* Hack -- mark the edge of the map as walls */
        for (x = 0; x < AUTO_MAX_X; x++) {

            /* West edge */
            grid(x,0)->info |= BORG_WALL;

            /* East edge */
            grid(x,AUTO_MAX_Y-1)->info |= BORG_WALL;
        }	


        /* Forget the view */
        borg_forget_view();


        /* Clear the arrays */
        view_n = seen_n = 0;	


        /* Note the wipe */
        wiped = TRUE;
    }
}


/*
 * Update the Borg based on the current "map"
 * The player grid contents are always "unknown"
 */
void borg_update_map(void)
{
    int x, y, dx, dy;

    auto_grid *ag;

    byte a;
    char c;


    /* Forget old info */
    if (!wiped) {

        /* Hack -- Forget the previous (66x22 grid) map sector */
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
    }


    /* Forget the old player grid */
    pg = NULL;

    /* Analyze the current (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {


            /* Direct access to the screen */
            a = Term->scr->a[dy+1][dx+13];
            c = Term->scr->c[dy+1][dx+13];


            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);


            /* Notice the player */
            if (c == '@') {

                /* Extract the player grid */
                c_x = x; c_y = y;

                /* Oops */
                if (pg) borg_oops("Multiple Players!");

                /* Note the player's grid */
                pg = ag;

                /* Assume dark */
                a = 0;
                c = ' ';

                /* Mega-Hack -- assume initial knowledge */
                if (pg->o_a == 0) a = TERM_WHITE;
                if (pg->o_c == ' ') c = '.';

                /* Mega-Hack -- handle bashing and blind-ness */
                if (pg->info & BORG_WALL) pg->info &= ~BORG_WALL;
            }


            /* Do not process dark grids */
            if (!a || (c == ' ')) continue;


            /* Hack -- undo special "lighting" */
            if ((c == '.') || (c == '%') || (c == '#')) a = TERM_WHITE;


            /* Notice "information" */
            ag->info |= BORG_OKAY;

            /* Save the info */
            ag->o_a = a; ag->o_c = c;


            /* Notice "walls" (and such) */
            if ((c == '#') || (c == '%') || (c == '+') || (c == ':')) {

                /* We are a wall */
                ag->info |= BORG_WALL;
            }

            /* Clear the "wall" code */
            else {

                /* We are not a wall */
                ag->info &= ~BORG_WALL;
            }
        }
    }


    /* Save the "old" data */
    o_w_x = w_x; o_w_y = w_y;
    o_c_x = c_x; o_c_y = c_y;


    /* No longer wiped */
    wiped = FALSE;


    /* Paranoia -- make sure we exist */
    if (!pg) borg_oops("Player missing!");
}


/*
 * Init the Borg arrays
 *
 * Note that the Borg will never find Grond/Morgoth, but we
 * prepare the item parsers for them anyway.  Actually, the
 * Borg might get lucky and find some of the special artifacts,
 * so it is always best to prepare for a situation if it does
 * not cost any effort.
 *
 * Note that all six artifact "Rings" will parse as "kind 506"
 * (the first artifact ring) and both artifact "Amulets" will
 * parse as "kind 503" (the first of the two artifact amulets),
 * but as long as we use the "name1" field (and not the "kind"
 * or "sval" fields) we should be okay.
 *
 * We sort the two arrays of items names in reverse order, so that
 * we will catch "mace of disruption" before "mace", "Scythe of
 * Slicing" before "Scythe", and for "Ring of XXX" before "Ring".
 *
 * Note that we do not have to parse "plural artifacts" (!)
 */
void borg_init_arrays(void)
{
    int i, j, k, z, n;

    int size;

    sint what[512];
    cptr text[512];

    char buf[256];


    /*** Dungeon Arrays ***/

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

    /* Prepare the priority queue head */
    auto_room_head->cost = 0;
    auto_room_head->prev = auto_room_head->self;
    auto_room_head->next = auto_room_tail->self;

    /* Prepare the priority queue tail */
    auto_room_tail->cost = 30000;
    auto_room_tail->prev = auto_room_head->self;
    auto_room_tail->next = auto_room_tail->self;

    /* Make the array of grids */
    C_MAKE(auto_grids, AUTO_MAX_Y, auto_grid*);

    /* Make each row of grids */
    for (i = 0; i < AUTO_MAX_Y; i++) {
        C_MAKE(auto_grids[i], AUTO_MAX_X, auto_grid);
    }


    /*** Make two special arrays ***/

    /* Hack -- array of "seen" grids */
    C_MAKE(seen_x, SEEN_MAX, byte);
    C_MAKE(seen_y, SEEN_MAX, byte);

    /* Hack -- array of "view" grids */
    C_MAKE(view_x, VIEW_MAX, byte);
    C_MAKE(view_y, VIEW_MAX, byte);


    /*** Item/Ware arrays ***/

    /* Make the inventory array */
    C_MAKE(auto_items, INVEN_TOTAL, auto_item);

    /* Make the stores in the town */
    C_MAKE(auto_shops, 8, auto_shop);


    /*** Plural Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 0; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_list[k];

        /* Skip non-items */
        if (!k_ptr->tval) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval >= TV_GOLD) continue;

        /* Skip "artifacts" */
        if (k_ptr->flags3 & TR3_INSTA_ART) continue;

        /* Hack -- make an item */
        invcopy(&hack, k);

        /* Describe a "plural" object */
        hack.number = 2;
        objdes_store(buf, &hack, FALSE);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Sort entries (in reverse order) by text */
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {

            int i1 = j;
            int i2 = j + 1;

            s16b k1 = what[i1];
            s16b k2 = what[i2];

            cptr t1 = text[i1];
            cptr t2 = text[i2];

            /* Enforce (reverse) order */
            if (strcmp(t1, t2) < 0) {

                /* Swap "kind" */
                what[i1] = k2;
                what[i2] = k1;

                /* Swap "text" */
                text[i1] = t2;
                text[i2] = t1;
            }
        }
    }

    /* Save the size */
    auto_size_plural = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_what_plural, auto_size_plural, s16b);
    C_MAKE(auto_text_plural, auto_size_plural, cptr);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_text_plural[i] = text[i];
    for (i = 0; i < size; i++) auto_what_plural[i] = what[i];


    /*** Singular Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 0; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_list[k];

        /* Skip non-items */
        if (!k_ptr->tval) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval >= TV_GOLD) continue;

        /* Skip "artifacts" */
        if (k_ptr->flags3 & TR3_INSTA_ART) continue;

        /* Hack -- make an item */
        invcopy(&hack, k);

        /* Describe a "singular" object */
        hack.number = 1;
        objdes_store(buf, &hack, FALSE);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Analyze more "item kinds" (the artifacts) */
    for (k = 0; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_list[k];

        /* Skip non-items */
        if (!k_ptr->tval) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval >= TV_GOLD) continue;

        /* Skip "non-artifacts" */
        if (!(k_ptr->flags3 & TR3_INSTA_ART)) continue;

        /* Hack -- make an item */
        invcopy(&hack, k);

        /* No length yet */
        n = 0;

        /* Mega-Hack -- find the artifact index */
        for (z = 1; z < ART_MAX; z++) {

            /* Save the correct artifact index */
            if (v_list[z].tval != hack.tval) continue;
            if (v_list[z].sval != hack.sval) continue;

            /* Save the index */
            hack.name1 = z;

            /* Extract the "suffix" length */
            n = strlen(v_list[z].name) + 1;

            /* Done */
            break;
        }

        /* Describe a "singular" object */
        hack.number = 1;
        objdes_store(buf, &hack, FALSE);
        buf[strlen(buf) - n] = '\0';

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Sort entries (in reverse order) by text */
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {

            int i1 = j;
            int i2 = j + 1;

            s16b k1 = what[i1];
            s16b k2 = what[i2];

            cptr t1 = text[i1];
            cptr t2 = text[i2];

            /* Enforce (reverse) order */
            if (strcmp(t1, t2) < 0) {

                /* Swap "kind" */
                what[i1] = k2;
                what[i2] = k1;

                /* Swap "text" */
                text[i1] = t2;
                text[i2] = t1;
            }
        }
    }

    /* Save the size */
    auto_size_single = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_what_single, auto_size_single, s16b);
    C_MAKE(auto_text_single, auto_size_single, cptr);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_text_single[i] = text[i];
    for (i = 0; i < size; i++) auto_what_single[i] = what[i];


    /*** Artifact and Ego-Item Parsers ***/

    /* No entries yet */
    size = 0;

    /* Collect the "artifact names" */
    for (k = 1; k < ART_MAX; k++) {

        /* Skip non-items */
        if (!v_list[k].name) continue;

        /* Extract a string */
        sprintf(buf, " %s", v_list[k].name);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Collect the "ego-item names" */
    for (k = 1; k < EGO_MAX; k++) {

        /* Skip non-items */
        if (!ego_names[k]) continue;

        /* Extract a string */
        sprintf(buf, " %s", ego_names[k]);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = 0 - k;
        size++;
    }

    /* Sort entries (in reverse order) by text */
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {

            int i1 = j;
            int i2 = j + 1;

            s16b k1 = what[i1];
            s16b k2 = what[i2];

            cptr t1 = text[i1];
            cptr t2 = text[i2];

            /* Enforce (reverse) order */
            if (strcmp(t1, t2) < 0) {

                /* Swap "kind" */
                what[i1] = k2;
                what[i2] = k1;

                /* Swap "text" */
                text[i1] = t2;
                text[i2] = t1;
            }
        }
    }

    /* Save the size */
    auto_size_artego = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_what_artego, auto_size_artego, s16b);
    C_MAKE(auto_text_artego, auto_size_artego, cptr);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_text_artego[i] = text[i];
    for (i = 0; i < size; i++) auto_what_artego[i] = what[i];
}




#endif

