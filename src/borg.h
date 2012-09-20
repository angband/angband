/* File: borg.h */

/* Purpose: Header file for "borg.c" -BEN- */

#include "angband.h"



#ifdef AUTO_PLAY

/*
 * This file provides support for "borg.c", an automatic Angband player.
 *
 * The "theory" behind the Borg is that is should be able to run as a
 * separate process, playing Angband in a window just like a human, that
 * is, examining the screen for information, and sending keypresses to
 * the game.  The current Borg does not actually do this, because it would
 * be very slow and would not run except on Unix machines, but as far as
 * possible, I have attempted to make sure that the Borg *could* run that
 * way.  This involves "cheating" as little as possible, where "cheating"
 * means accessing information not available to a normal Angband player.
 *
 * Thus, the Borg COULD be written as a separate process which runs Angband
 * in a pseudo-terminal and "examines" the "screen" and sends keypresses
 * directly (as with a terminal emulator), although it would then have
 * to explicitly "wait" to make sure that the game was completely done
 * sending information.
 *
 * The Borg is allowed to examine the screen directly (usually by an
 * efficient direct access to the "Term->scr->a" and "Term->scr->c")
 * and to send keypresses directly ("Term_keypress()").  The Borg also
 * accesses the cursor location ("Term_locate()") and visibility (via
 * a hack involving "Term_show/hide_cursor()") directly.
 *
 * The Borg should not know when the game is ready for a keypress, but it
 * checks this anyway by distinguishing between the "check for keypress"
 * and "wait for keypress" hooks sent by the "term.c" package.  Otherwise
 * it would have to "pause" between turns for some amount of time to ensure
 * that the game was done processing.  It might be possible to notice when
 * the game is ready for input by some other means, but it seems likely that
 * at least some "waiting" would be necessary, unless the terminal emulation
 * program explicitly sends a "ready" sequence when ready for a keypress.
 *
 * Various other "cheats" are described in "borg.c".
 */



/*
 * OPTION: Allow rooms to intersect.  This is "pretty" but inefficient.
 */
/* #define CROSS_ROOMS */


/*
 * Hack -- goal types
 */
#define GOAL_KILL	11
#define GOAL_TAKE	12
#define GOAL_DARK	13
#define GOAL_XTRA	99


/*
 * Screen location info from "misc.c"
 */

#define ROW_RACE	1
#define ROW_CLASS	2
#define ROW_TITLE	3

#define ROW_LEVEL	4
#define COL_LEVEL	6

#define ROW_EXP		5
#define COL_EXP		4

#define ROW_STAT	7
#define COL_STAT	6

#define ROW_AC		14
#define COL_AC		6

#define ROW_CURHP	15
#define COL_CURHP	6

#define ROW_MAXHP	16
#define COL_MAXHP	6

#define ROW_MANA	17
#define COL_MANA	6

#define ROW_GOLD	18
#define COL_GOLD	3

#define ROW_WINNER	19
#define ROW_EQUIPPY	20

#define ROW_CUT		21
#define ROW_STUN	22

#define ROW_HUNGRY	23

#define COL_HUNGRY	0	/* "Hungry" or "Weak" */
#define COL_BLIND	7	/* "Blind" */
#define COL_CONFUSED	13	/* "Confused" */
#define COL_AFRAID	22	/* "Afraid" */
#define COL_POISONED	29	/* "Poisoned" */
#define COL_STATE	38	/* "Paralyzed!" or "Searching " or <state> */
#define COL_SPEED	49	/* "Slow (-N)" or "Fast (+N)" */
#define COL_STUDY	64	/* "Study" */
#define COL_DEPTH	70	/* "Lev N" or "N ft" (right justified) */



/*
 * Maximum possible dungeon size
 */
#define AUTO_MAX_X	MAX_WID
#define AUTO_MAX_Y	MAX_HGT

/*
 * Maximum number of rooms.  This may be too small.
 * But if AUTO_ROOMS * sizeof(auto_room) > 64K then some
 * machines may not be able to allocate the room array.
 */
#define AUTO_ROOMS	(AUTO_MAX_X * AUTO_MAX_Y / 8)


/*
 * Flags for the "info" field of grids
 */
#define BORG_WALL	0x01	/* Grid is a wall */
#define BORG_GLOW	0x02	/* Grid is illuminated */
#define BORG_REAL	0x04	/* Grid is really usable */
#define BORG_OKAY	0x08	/* Grid is not currently dark */
#define BORG_SEEN	0x10	/* Grid is in a special array */
#define BORG_LITE	0x20	/* Grid is lit by torches */
#define BORG_VIEW	0x40	/* Grid is in view of the player */
#define BORG_XTRA	0x80	/* Grid is easily in view */


/*
 * Maximum size of the "view" array
 */
#define VIEW_MAX 2000


/*
 * Number of grids in the "seen" array
 */
#define SEEN_MAX 2000



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




/*
 * Forward declare
 */
typedef struct _auto_room auto_room;
typedef struct _auto_grid auto_grid;
typedef struct _auto_item auto_item;
typedef struct _auto_shop auto_shop;


/*
 * A room in the dungeon.  20 bytes.
 *
 * The "rooms" are designed to (1) optimize the storage of information
 * about the map and (2) simplify navigation.  Thus, we create rooms by
 * grouping relevant rectangles of grids together.  This includes all
 * "normal" rooms in the dungeon, most "corridors", occasional "thick"
 * corridors, all left-over floor grids, and many regions in the town.
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
 * The "prev"/"next" fields are used to facilitate a priority queue of
 * rooms during the preparation of a "flow".  See various code below.
 *
 * The "when" field allows us to make decisions based on the actual
 * "time" at which a room was visited.  Currently this field is used
 * only by the "revisit" function to choose "interesting" rooms.  We
 * can probably delete this field and still function.
 *
 * The "location" of the corners of the room are encoded in x1/y1/x2/y2.
 *
 * For navigation, we use a single "location" (x/y) which is usually not
 * inside the room, but is always inside or touching the room.  Whenever
 * the Borg is inside the room, it attempts to approach that location,
 * which it can always do because the rooms are rectangular and the Borg
 * attempts axis motion before diagonals, and thus never leaves the room
 * until it actually steps onto that location.  During navigation, the
 * cost of reaching the final goal is "cost" plus the distance to "x,y".
 * If "cost" is zero than "x,y" is the final destination.  The drawback
 * to this method is that the Borg can only have a single destination in
 * each room.  This is handled when the navigation ("flow") is prepared
 * by arbitrarily choosing the location in each room which is physically
 * closest to the player.  This may be invalid if the only available path
 * to the room enters the room from the far side, but it seems to work.
 */
struct _auto_room {

  u16b self;		/* Self index */

  u16b free;		/* Index of next room in free room list */

  u16b prev;		/* Index of prev room in priority queue */
  u16b next;		/* Index of next room in priority queue */

  s32b when;		/* When last visited */

  byte x1, y1;		/* Upper left corner */
  byte x2, y2;		/* Bottom right corner */

  byte x, y;		/* Flow location */
  u16b cost;		/* Flow cost */
};


/*
 * A grid in the dungeon.  8 bytes.
 *
 * A room index of "zero" means that the grid is not in any room yet
 * A room index N such that 0<N<MAX_ROOMS refers to a single room
 * A room index MAX_ROOMS+N means the grid is inside "N" different rooms
 *
 * The attr/char (o_a/o_c) is the attr/char that the grid appeared to
 * possess the last time the "okay" flag was true.
 *
 * There is a set of eight bit flags (see below) called "info".
 */
struct _auto_grid {

  u16b room;		/* Room index, see above */

  byte o_a;		/* Recent attribute */
  char o_c;		/* Recent character */

  byte info;		/* Some bit flags (see BORG_NNNN) */

  byte xtra;		/* Extra field for actual strategy */
};



/*
 * A structure holding information about an object in the inventory
 *
 * The "iqty" is zero if the object is "missing"
 * The "tval" is zero if the object is "missing"
 * The "kind" is zero if the object is "unaware" (or missing)
 * The "able" is zero if the object is "unknown" (or unaware or missing)
 */
struct _auto_item {

  char desc[80];	/* Actual Description		*/

  cptr note;		/* Pointer to tail of 'desc'	*/

  u16b kind;		/* Kind index			*/

  byte iqty;		/* Number of items		*/
  bool able;		/* True if item is identified	*/

  byte tval;		/* Item type			*/
  byte sval;		/* Item sub-type		*/
  s16b pval;		/* Item extra-info		*/

  s32b cost;		/* Announced cost (if known)	*/

  s16b to_h;		/* Bonus to hit			*/
  s16b to_d;		/* Bonus to dam			*/
  s16b to_a;		/* Bonus to ac			*/
  s16b ac;		/* Armor class			*/
  byte dd;		/* Damage dice			*/
  byte ds;		/* Damage sides			*/

  byte name1;		/* Artifact index (if any)	*/
  byte name2;		/* Ego-item index (if any)	*/

  bool junk;		/* Item should be trashed 	*/
  bool cash;		/* Item should be bought/sold	*/
  bool test;		/* Item should be identified	*/
  bool wear;		/* Item should be worn/wielded	*/
};


/*
 * A store
 */
struct _auto_shop {

  int visit;		/* Number of visits */
  int extra;		/* Something unused */

  int page;		/* Current page */
  int more;		/* Number of pages */

  auto_item ware[24];	/* Store contents */
};




/*
 * Some variables
 */

extern bool auto_ready;		/* Initialized */

extern bool auto_active;	/* Actually active */

extern int auto_room_max;	/* First totally free room */

extern auto_grid **auto_grids;	/* Current "grid list" */

extern auto_room *auto_rooms;	/* Current "room list" */

extern auto_item *auto_items;	/* Current "inventory" */

extern auto_shop *auto_shops;	/* Current "shops" */

extern auto_room *auto_room_head;	/* &auto_rooms[0] */

extern auto_room *auto_room_tail;	/* &auto_rooms[AUTO_MAX_ROOMS-1] */

extern auto_grid *pg;		/* Player grid */

extern int c_x;			/* Player location (X) */
extern int c_y;			/* Player location (Y) */

extern int w_x;			/* Current panel offset (X) */
extern int w_y;			/* Current panel offset (Y) */


/*
 * Log file
 */
extern FILE *auto_fff;		/* Log file */


/*
 * Maintain a set of grids marked as "BORG_VIEW"
 */
extern u16b view_n;
extern byte *view_y;
extern byte *view_x;


/*
 * Maintain a set of grids marked as "BORG_SEEN"
 */
extern u16b seen_n;
extern byte *seen_y;
extern byte *seen_x;



/*
 * Size of Keypress buffer
 */
#define KEY_SIZE 1024

/*
 * A Queue of keypresses to be sent
 */
extern char auto_key_queue[KEY_SIZE];
extern s16b auto_key_head;
extern s16b auto_key_tail;

/*
 * Queue a keypress
 */
extern errr borg_keypress(int k);

/*
 * Dequeue a keypress
 */
extern char borg_inkey(void);

/*
 * Flush the keypresses
 */
extern void borg_flush(void);


/*
 * Obtain some text from the screen
 */
extern errr borg_what_text(int x, int y, int n, byte *a, char *s);

/*
 * Obtain some text from the screen, accepting any (non-final) spaces
 */
extern errr borg_what_text_hack(int x, int y, int n, byte *a, char *s);


/*
 * Log a message to a file
 */
extern void borg_info(cptr what);

/*
 * Show a message to the user (and log it)
 */
extern void borg_note(cptr what);

/*
 * Show a message and turn off "auto_active"
 */
extern void borg_oops(cptr what);


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


/*
 * Check a path for missile clearance
 */
extern bool borg_projectable(int x1, int y1, int x2, int y2);

/*
 * Check a path for line of sight
 */
extern bool borg_los(int x1, int y1, int x2, int y2);

/*
 * Determine which slot an item could be wielded into
 */
extern int borg_wield_slot(auto_item *item);

/*
 * Analyze an item, given a textual description and (optional) cost
 */
extern void borg_item_analyze(auto_item *item, cptr desc, cptr cost);

/*
 * Determine the "value" of an item in a shop-keeper's eyes
 */
extern s32b borg_item_value(auto_item *item);

/*
 * Choose a "decent" direction that will avoid walls
 */
extern int borg_goto_dir(int x1, int y1, int x2, int y2);


/*
 * Determine the "cost" of the given grid
 */
extern int borg_flow_cost(int x, int y);

/*
 * Begin a new flow
 */
extern void borg_flow_enqueue_grid(int x, int y);

/*
 * Spread a flow
 */
extern void borg_flow_spread(bool optimize);

/*
 * Clear the flow
 */
extern void borg_flow_clear(void);

/*
 * Flow outwards from the player
 */
extern void borg_flow_reverse(void);

/*
 * Forget the old "map" info (new level)
 */
extern void borg_forget_map(void);

/*
 * Update the "map" info from the screen
 */
extern void borg_update_map(void);

/*
 * Update the "view"
 */
extern void borg_update_view(void);

/*
 * Forget the "view"
 */
extern void borg_forget_view(void);

/*
 * Init some arrays
 */
extern void borg_init_arrays(void);


#endif

