/* File: borg-map.h */

/* Purpose: Header file for "borg-map.c" -BEN- */

#ifndef INCLUDED_BORG_MAP_H
#define INCLUDED_BORG_MAP_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg-map.c".
 */

#include "borg.h"


/*
 * OPTION: Use the "room" support
 *
 * Note -- this option causes slight bugs in "borg-ext.c" that
 * I have not yet tracked down.  You have been warned...
 */
/* #define BORG_ROOMS */


/*
 * Maximum possible dungeon size
 */
#define AUTO_MAX_X	MAX_WID
#define AUTO_MAX_Y	MAX_HGT



#ifdef BORG_ROOMS


/*
 * OPTION: Allow rooms to intersect.  This is "pretty" but inefficient.
 */
/* #define CROSS_ROOMS */


/*
 * Maximum number of rooms.  This may be too small.
 * But if AUTO_ROOMS * sizeof(auto_room) > 64K then some
 * machines may not be able to allocate the room array.
 */
#define AUTO_ROOMS	(AUTO_MAX_X * AUTO_MAX_Y / 8)


#endif



/*
 * Flags for the "info" field of grids
 */
#define BORG_WALL	0x01	/* Grid is a wall */
#define BORG_OKAY	0x02	/* Grid is lit on this panel */
/* xxx */
/* xxx */
/* xxx */
/* xxx */
#define BORG_VIEW	0x40	/* Grid is in view of the player */
#define BORG_XTRA	0x80	/* Grid is easily in view of the player */


/*
 * Maximum size of the "view" array
 */
#define AUTO_VIEW_MAX 1536


/*
 * Number of grids in the "seen" array
 */
#define AUTO_SEEN_MAX 1536


/*
 * Number of grids in the "flow" array
 */
#define AUTO_FLOW_MAX 1536




/*
 * Determine if a grid is "legal"
 */
#define grid_legal(X,Y) \
    (((X) >= 0) && ((X) < AUTO_MAX_X) && ((Y) >= 0) && ((Y) < AUTO_MAX_Y))

/*
 * Obtain a pointer to the grid at a location
 * Should be a function but a macro is faster
 */
#define grid(X,Y) \
    (&auto_grids[Y][X])


/*
 * Determine "twice" the distance between two points
 * This results in "diagonals" being "correctly" ranged,
 * that is, a diagonal appears "furthur" than an adjacent.
 */
 #define double_distance(Y1,X1,Y2,X2) \
    (distance(((int)(Y1))<<1,((int)(X1))<<1,((int)(Y2))<<1,((int)(X2))<<1))





#ifdef BORG_ROOMS


/*
 * Forward declare
 */
typedef struct auto_room auto_room;


/*
 * A room in the dungeon.  20 bytes.
 *
 * The "rooms" are designed to (1) optimize the storage of information
 * about the map and (2) simplify navigation.  Thus, we create rooms by
 * grouping relevant rectangles of grids together.  This includes all
 * "normal" rooms in the dungeon, most "corridors", occasional "thick"
 * corridors, all left-over floor grids, and many regions in the town.
 *
 * A room index of "zero" means that the grid is not in any room yet
 * A room index N such that 0<N<MAX_ROOMS refers to a single room
 * A room index MAX_ROOMS+N means the grid is inside "N" different rooms
 *
 * A few options in the code itself determine if two rooms can overlap.
 * Likewise, the code handles issues like one room absorbing another,
 * and destroying any rooms that are suddenly found to include a grid
 * which is actually a wall.  This allows the Borg to recover from
 * earthquakes, and also lets it assume that all monsters and objects
 * are actually on floors, since if they move, revealing a wall, the
 * Borg will simply take back all previous assumptions.
 *
 * The rooms are maintained in a "room array" of a size AUTO_ROOMS.
 * The first and last rooms in this array are used to maintain two
 * in-place lists, a stack of "free" rooms, and a priority queue
 * of rooms used to propagate the current "flow".
 *
 * The "self" field gives the index of a room.
 *
 * The "free" field is either zero, meaning that the room is being used,
 * or it holds the index of the next free room.  Note that the first
 * room always points to the next free room, and the last room always
 * points to itself (indicating we have run out of rooms).
 *
 * The "when" field allows us to make decisions based on the actual
 * "time" at which a room was visited.  Currently this field is used
 * only by the "revisit" function to choose "interesting" rooms.  We
 * can probably delete this field and still function.
 *
 * The "location" of the corners of the room are encoded in x1/y1/x2/y2.
 */
struct auto_room {

  s16b self;		/* Self index */

  s16b free;		/* Index of next room in free room list */

  s16b when;		/* When last visited */

  byte x1, y1;		/* Upper left corner */
  byte x2, y2;		/* Bottom right corner */
};


#endif


/*
 * Forward declare
 */
typedef struct auto_grid auto_grid;

/*
 * A grid in the dungeon.  4 bytes.
 *
 * The attr/char (o_a/o_c) is the attr/char that the grid appeared to
 * possess the last time the "okay" flag was true.
 *
 * Hack -- note that the "char" zero will often crash the system!
 *
 * There is a set of eight bit flags (see below) called "info".
 *
 * There is a byte "xtra" for various user strategies, like searching.
 *
 * To perform "navigation" from one place to another, the "flow" routines
 * are used, which place "cost" information into the "cost" fields.  Then,
 * if the path is clear, the "cost" information is copied into the "flow"
 * fields, which are used for the actual navigation.  This allows multiple
 * routines to check for "possible" flowing, without hurting the current
 * flow, which may have taken a long time to construct.  We also assume
 * that the Borg never needs to follow a path longer than 250 grids long.
 */
struct auto_grid {

    byte o_a;		/* Recent attribute */
    char o_c;		/* Recent character */

#ifdef BORG_ROOMS
    s16b room;		/* Room index */
#endif

    byte info;		/* Some bit flags (see BORG_NNNN) */

    byte xtra;		/* Extra field (search count) */
};




/*
 * Forward declare
 */
typedef struct auto_data auto_data;

/*
 * Hack -- one byte of info per grid
 *
 * We use a structure to encapsulate the data into a "typed" form.
 *
 * Allows optimized processing of "flow" info via structure copy
 * commands, see "borg-ext.c" for details.
 */
struct auto_data {

    byte data[AUTO_MAX_Y][AUTO_MAX_X];
};



/*
 * Some variables
 */

extern auto_grid **auto_grids;		/* Current "grid list" */

extern auto_data *auto_data_flow;	/* Current "flow" data */

extern auto_data *auto_data_cost;	/* Current "cost" data */


#ifdef BORG_ROOMS

/*
 * Some variables
 */

extern int auto_room_max;		/* First totally free room */

extern auto_room *auto_rooms;		/* Current "room list" */

extern auto_room *auto_room_head;	/* &auto_rooms[0] */

extern auto_room *auto_room_tail;	/* &auto_rooms[AUTO_MAX_ROOMS-1] */


#endif


/*
 * Maintain a set of grids (viewable grids)
 */

extern s16b auto_view_n;
extern byte *auto_view_y;
extern byte *auto_view_x;


/*
 * Maintain a set of grids (scanning arrays)
 */

extern s16b auto_temp_n;
extern byte *auto_temp_y;
extern byte *auto_temp_x;


/*
 * Maintain a set of grids (flow calculations)
 */

extern s16b auto_flow_n;
extern byte *auto_flow_y;
extern byte *auto_flow_x;



/*
 * External flag -- clear danger information
 */
extern bool auto_danger_wipe;

/*
 * External value -- current danger thresh-hold
 */
extern s32b auto_danger_xtra;

/*
 * External hook -- check danger of a grid
 */
extern bool (*auto_danger_hook)(int x, int y);



#ifdef BORG_ROOMS

/*
 * Obtain rooms containing a grid
 */
extern auto_room *room(bool go, int gx, int gy);

/*
 * Clean up the "free room" list
 */
extern bool borg_free_room_update(void);

/*
 * Obtain the next "free" room
 */
extern auto_room *borg_free_room(void);

/*
 * Attempt to build a bigger room at the location
 */
extern bool borg_build_room(int x, int y);

/*
 * Attempt to clear all rooms containing the location
 */
extern bool borg_clear_room(int x, int y);


#endif



/*
 * Check a path for line of sight
 */
extern bool borg_los(int x1, int y1, int x2, int y2);

/*
 * Forget the old "map" info (new level)
 */
extern void borg_forget_map(void);

/*
 * Update the "map" info from the screen
 */
extern void borg_update_map(void);

/*
 * Choose a "decent" direction that will avoid walls
 */
extern int borg_goto_dir(int x1, int y1, int x2, int y2);


/*
 * Clear the flow
 */
extern void borg_flow_clear(void);

/*
 * Spread a flow
 */
extern void borg_flow_spread(bool optimize);

/*
 * Begin a new flow
 */
extern void borg_flow_enqueue_grid(int x, int y);

/*
 * Flow outwards from the player
 */
extern void borg_flow_reverse(void);

/*
 * Hack -- Place the "target" at the given location
 */
extern bool borg_target(int x, int y);

/*
 * Initialize this file
 */
extern void borg_map_init(void);


#endif


#endif

