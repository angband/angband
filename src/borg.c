/* File: borg.c */

/* Purpose: an "automatic" player */

#include "angband.h"


#ifdef AUTO_PLAY

/*
 * Following is a "little project" for school
 *
 * A special command (see "command.c") is used to start, observe,
 * and modify the Borg.  This key defaults to "Control-Z".
 *
 * We allow the "auto-player" to "know" only what is visible on
 * the screen (using the "term.c" screen access functions).  Thus,
 * the "auto-player" could be written as a separate process which
 * runs Angband in a pseudo-terminal and "examines" the "screen".
 * Also, it hooks into "term.c" to send keypresses when requested.
 *
 * I should probably have a "special" mode for being "blind".
 *
 * We "notice" when the "level" changes and re-initialize.
 *
 * Must be careful of:
 *   Gold (or objects) embedded in walls (need to tunnel)
 *   Objects with 's' or ',' symbols (look like monsters)
 *   Traps must be disarmed, Doors must be opened
 *   Stores must be "exited" once "entered"
 *   Doors may be stuck (need to bash)
 *   Must discard junk before trying to pick up more junk
 *   Must be very careful not to run out of food/light
 *   Monsters may be inside walls (need to tunnel)
 *
 * Should be careful of:
 *   Heavy objects may induce infinite looping
 *   If wounded, must run away from monsters, then rest
 *   If we pick up a cursed artifact, we are fucked :-)
 *   Darkness is "ignored" after the first encounter
 *   Try to use a shovel/pick to help with tunnelling
 *
 *
 * Memory usage (300K total, largest chunk 42K):
 *   Rooms: (200*70/8)*24 = 42K
 *   Each row of Grids: 200*16 = 3K, thus 70*3K = 210K total.
 *   Inventory: 34*132 = 5K
 *   Shop: 24*132 = 3K
 *
 * Currently, the auto-player "cheats" in a few situations.  Oops.
 *
 * The "big" cheat is that we "build" an inventory listing without
 * parsing the screen.  We mainly do this because the auto-player
 * is never executed "inside" the inventory mode.
 *
 * Cheats "required" by implementation:
 *   Direct use of "current screen image" / "keypress queue"
 *   Knowledge of when we are being asked for a keypress.
 *   The "space" character can only be used for "darkness".
 *
 * Cheats that could be "overcome" with complex routines:
 *   Direct use of "char_row/char_col" (could acquire via "L" command)
 *   Direct construction of "inven/equip" lists (could do a screen parse)
 *   Direct access to the "current options" and changing them
 *
 * Cheats that could be avoided by duplicating code:
 *   Direct access to the "current viewable space" array (hmm...)
 *   Direct access to the "properties" of the object kinds.
 */
 
 
/*
 * Problem rooms (see "build_room()" algorithm):
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
 */


/*
 * Big cheat -- access the "viewable space" array
 */

extern int auto_cheat_get_view_n(void);
extern byte *auto_cheat_get_view_x(void);
extern byte *auto_cheat_get_view_y(void);




#define GOAL_GOTO	11
#define GOAL_TAKE	21
#define GOAL_KILL	31
#define GOAL_FLOW	91

#define STATE_START	1	/* Analyze "dungeon" screen */
#define STATE_INVEN	2	/* Analyze "inventory" screen */
#define STATE_EQUIP	3	/* Analyze "equipment" screen */
#define STATE_STORE	10	/* Analyze "store" screens */
#define STATE_THINK	20	/* Actually choose an action */

/*
 * Maximum possible dungeon size
 */
#define AUTO_MAX_X	MAX_WIDTH
#define AUTO_MAX_Y	MAX_HEIGHT

/*
 * Maximum number of rooms (could be too small)
 */
#define AUTO_ROOMS	((AUTO_MAX_X * AUTO_MAX_Y) / 8)


/*
 * Forward declare
 */
typedef struct _auto_room auto_room;
typedef struct _auto_grid auto_grid;
typedef struct _auto_item auto_item;


/*
 * A room in the dungeon.  24 bytes.
 */
struct _auto_room {

  u16b self;		/* Hack -- self index */
  u16b free;		/* Hack -- next free room */
  
  u32b when;		/* When last visited (if ever) */

  byte x1, y1;		/* Upper left corner */
  byte x2, y2;		/* Bottom right corner */

  u16b flow;		/* Flow cost */
  byte x, y;		/* Flow location */
  
  auto_room *prev;	/* Prev room (in flow) */
  auto_room *next;	/* Next room (in flow) */
};


/*
 * A grid in the dungeon.  8 bytes.
 *
 * A room index of "zero" means that the grid is not in any room yet
 * A room index N such that 0<N<MAX_ROOMS refers to a single room
 * A room index MAX_ROOMS+N means the grid is inside "N" different rooms
 */
struct _auto_grid {

  byte o_a;		/* Attribute when last seen */
  char o_c;		/* Character when last seen */

  byte f_a;		/* Floor attribute (if known) */
  char f_c;		/* Floor character (if known) */

  u16b room;		/* Room index, see above */

  u16b info;		/* Some info (unused?) */
};



/*
 * A structure holding information about an object in the inventory
 *
 * The "iqty" is zero if the object is "missing"
 * The "tval" is zero if the object is "bizarre" (or missing)
 * The "kind" is zero if the object is "unaware" (or bizarre, etc)
 * The "able" is zero if the object is "unknown" (or unaware, etc)
 *
 * Note that the "Special Objects" should be the only "bizarre" objects.
 * Although certain artifacts and ego weapons may match as well.
 */
struct _auto_item {

  char desc[80];	/* Actual Description		*/

  char note[20];	/* Inscription, if any		*/
  char cost[20];	/* Cost, if in a store		*/
  
  u16b kind;		/* Kind index			*/
  
  byte iqty;		/* Number of items		*/
  bool able;		/* True if item is identified	*/

  byte tval;		/* Item type			*/
  byte sval;		/* Item sub-type		*/
  s16b pval;		/* Item extra-info		*/
  
  bool done;		/* Item has been examined	*/
  bool junk;		/* Item should be trashed 	*/
  bool wear;		/* Item should be worn/removed	*/
  bool cash;		/* Item should be bought/sold	*/
};



static int ready = FALSE;	/* Initialized */

static int state = 0;		/* Current "state" */

static int state_store = 0;	/* Store symbol, if any */

static int goal = 0;		/* Current "goal" */

static int goal_level = 0;	/* If non-zero, level to chase */

static s32b greed = 10L;	/* Minimal greed */

static u32b c_t;		/* Current time */
static int c_x, c_y;		/* Current location */

static int g_x, g_y;		/* Goal location */
static byte g_a;		/* Goal attr */
static char g_c;		/* Goal char */

static int o_x, o_y;		/* Location when goal began */
static u32b o_t;		/* Time when goal began */

static int auto_room_max;	/* No rooms yet */

static bool do_heal = FALSE;
static bool do_food = FALSE;
static bool do_lite = FALSE;
static bool do_torch = FALSE;
static bool do_flask = FALSE;

static bool cheat_inven = TRUE;
static bool cheat_equip = TRUE;

static u32b auto_began = 0L;	/* When this level began */

static char auto_level[16];	/* Current "level" info */

static char auto_gold[16];	/* Current "gold" info */

static auto_grid *pg;		/* Player grid */

static auto_grid **auto_grids;	/* Current "grid list" */

static auto_room *auto_rooms;	/* Current "room list" */

static auto_item *auto_items;	/* Current "inventory" */

static auto_item *auto_wares;	/* Current "store inventory" */


/*
 * Constant "item description parsers"
 */

static u16b i_size;		/* Number of entries */
static u16b *i_kind;		/* Kind index per entry */
static cptr *i_single;		/* Textual prefix for singles */
static cptr *i_plural;		/* Textual prefix for plurals */


/*
 * Constant "information" variables
 */

static int dx[10] = {0, -1, 0, 1, -1, 0, 1, -1, 0, 1};
static int dy[10] = {0, 1, 1, 1, 0, 0, 0, -1, -1, -1};
static int dd[9] =  {2, 8, 6, 4, 3, 1, 9, 7, 5};

static cptr auto_str_take = "^+:;$*?!_-\\|/\"=~{([])},s";
static cptr auto_str_kill = "ABCDEFGHIJKLMNOPQRSTUVWZYZabcdefghijklmnopqrtuvwxyz&";





/*
 * Size of Keypress buffer
 */
#define KEY_SIZE 1024

/*
 * A Queue of keypresses to be sent
 */
static char key_queue[KEY_SIZE];
static int key_head = 0;
static int key_tail = 0;


/*
 * Add a keypress to the "queue" (fake event)
 */
static errr Borg_keypress(int k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Store the char, advance the queue */
    key_queue[key_head++] = k;

    /* Circular queue, handle wrap */
    if (key_head == KEY_SIZE) key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (key_head == key_tail) core("Borg overflowed keypress buffer");

    /* Hack -- Overflow may induce circular queue */
    if (key_tail == KEY_SIZE) key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Get the next Borg keypress
 */
static int Borg_inkey(void)
{
    int i;

    /* Nothing ready */
    if (key_head == key_tail) return (0);
        
    /* Extract the keypress, advance the queue */
    i = key_queue[key_tail++];

    /* Circular queue requires wrap-around */
    if (key_tail == KEY_SIZE) key_tail = 0;

    /* Return the key */
    return (i);
}




/*
 * Hack -- Put some information on the screen at a later time
 */
static void auto_tell(cptr what)
{
    cptr s;

    /* Hack -- self note */
    Borg_keypress(':');
    for (s = what; *s; s++) Borg_keypress(*s);
    Borg_keypress('\n');
}


/*
 * Hack -- Put some information in the recall window
 */
static void auto_note(cptr what)
{
    if (!use_recall_win || !term_recall) return;

    Term_activate(term_recall);
    
    Term_clear();
    Term_putstr(0,0,-1,TERM_WHITE,what);
    Term_fresh();

    Term_activate(term_screen);
}


/*
 * Hack -- Stop processing on errors
 */
static void auto_oops(cptr what)
{
    /* Forget the state */
    state = 0;

    /* Give a warning */
    auto_note(format("The BORG has broken (%s).", what));
}


/*
 * Convert a "map location" into an "auto_grid"
 */
static auto_grid *grid(int x, int y)
{
    return (&auto_grids[y][x]);
}


/*
 * Determine if a room contains a location
 */
static bool auto_room_contains(auto_room *ar, int x, int y)
{
    if (x < ar->x1) return (FALSE);
    if (x > ar->x2) return (FALSE);
    if (y < ar->y1) return (FALSE);
    if (y > ar->y2) return (FALSE);
    return (TRUE);
}


/*
 * Hack -- access the "set" of rooms containing a given point
 *
 * We should probably build a "list" of "used rooms", where the "tail"
 * of that list is all the "crossed rooms", and also keep a pointer to
 * that tail for "fast" access to the "crossed rooms" set.
 */
static auto_room *room(bool go, int gx, int gy)
{
    static int x = 0, y = 0, i = 0;

    /* We just got a new grid */
    if (go) {

	auto_grid *ag = grid(gx, gy);
	
	/* Default to no rooms */
	i = auto_room_max;
	
	/* Paranoia -- no rooms */
	if (!ag->room) return (NULL);
	
	/* Efficiency -- Single room */
	if (ag->room < AUTO_ROOMS) return (&auto_rooms[ag->room]);

	/* Scan through multiple rooms */
	x = gx, y = gy, i = 0;
    }

    /* Look for a room */
    for (i++; i < auto_room_max; i++) {

	/* Access the room */
	auto_room *ar = &auto_rooms[i];
	
	/* Skip "dead" rooms */
	if (!ar->when) continue;
	
	/* If the room contains it, use it */
	if (auto_room_contains(ar, x, y)) return (ar);
    }

    /* Default */
    return (NULL);
}





/*
 * Query the "attr/chars" at a given location on the screen
 * We return "TRUE" only if a string of some form existed
 * Note that the string must be done in a single attribute.
 * Normally, the string must be "n" characters long.
 * If "n" is "negative", we will grab until the attribute changes.
 * Note that "a" points to a single "attr", "s" to a string of "chars".
 */
static errr Term_what_text(int x, int y, int n, byte *a, char *s)
{
    int i;
    byte t_a;
    char t_c;

    /* Max length to scan for */
    int m = ABS(n);
    
    /* Hack -- Pre-terminate the string */
    s[0] = '\0';
    
    /* Check the first character, make sure it exists */
    if (Term_what(x, y, &t_a, &t_c)) return (-1);

    /* Save the attribute */
    (*a) = t_a;
    
    /* Scan for it */
    for (i = 0; i < m; i++) {

	/* Ask for the screen contents */
	if (Term_what(x+i, y, &t_a, &t_c)) return (-2);

	/* Hack -- negative "n" stops at attribute change */
	if ((n < 0) && (t_a != (*a))) break;
	
	/* Verify the "attribute" */
	if (t_a != (*a)) return (-3);
		
	/* Save the character */
	s[i] = t_c;
    }

    /* Terminate the string */
    s[i] = '\0';
    
    /* Success */
    return (0);
}


/*
 * Determine if the given grid is a "snake" grid
 * A "snake" is a "normal" section of corridor, with no
 * bends or intersections.  Only the "center" counts.
 * Note that a "1x1" grid (or "diagonal" grid) is a "snake".
 * A "snake" grid cannot touch "unknown" grids.
 */
static bool auto_build_snake(int x, int y)
{
    auto_grid *ag, *ag1, *ag2;

    /* Access the center */
    ag = grid(x, y);

    /* Central grid cannot be unknown */
    if (ag->o_c == ' ') return (FALSE);
    
    /* Central grid must be a non-wall */
    if (strchr("#%", ag->o_c)) return (FALSE);
    
    /* South/North blockage induces a snake */
    ag1 = grid(x, y+1), ag2 = grid(x, y-1);
    if (strchr("#%", ag1->o_c) && strchr("#%", ag2->o_c)) return (TRUE);
    
    /* East/West blockage induces a snake */
    ag1 = grid(x+1, y), ag2 = grid(x-1, y);
    if (strchr("#%", ag1->o_c) && strchr("#%", ag2->o_c)) return (TRUE);
    
    /* No good */
    return (FALSE);
}



/*
 * Determine if the given "box" in the world is fully made of "floor"
 */
static bool auto_build_room_floor(int x1, int y1, int x2, int y2)
{
    int x, y;
    
    /* Check for "easy expand" */
    for (y = y1; y <= y2; y++) {
	for (x = x1; x <= x2; x++) {
	
	    /* Access that grid */
	    auto_grid *ag = grid(x, y);
	    
	    /* Refuse to accept unknown grids */
	    if (ag->o_c == ' ') return (FALSE);

	    /* Refuse to accept walls/seams/doors */
	    if (strchr("#%+'", ag->o_c)) return (FALSE);

	    /* XXX Hack -- Require floors (or stairs) */
	    if (!strchr(".<>", ag->o_c)) return (FALSE);
	}
    }

    /* Must be okay */
    return (TRUE);
}


/*
 * Determine if the given "box" in the world is fully "known"
 */
static bool auto_build_room_known(int x1, int y1, int x2, int y2)
{
    int x, y;
    
    /* Check for "unknown" grids */
    for (y = y1; y <= y2; y++) {
	for (x = x1; x <= x2; x++) {
	
	    /* Access that grid */
	    auto_grid *ag = grid(x, y);
	    
	    /* Refuse to accept unknown grids */
	    if (ag->o_c == ' ') return (FALSE);
	}
    }

    /* Must be okay */
    return (TRUE);
}




/*
 * Attempt to build a "bigger" room for the given location
 *
 * We use a "free list" to simplify "room allocation".
 *
 * We return TRUE if a "new" room was created.
 */
static bool auto_build_room(int x, int y)
{
    uint i, j;

    int x1, y1, x2, y2;

    auto_grid *ag;
    auto_room *ar;


    /* Attempt to expand a 3x3 room */
    if (auto_build_room_floor(x-1, y-1, x+1, y+1)) {
	x1 = x - 1, y1 = y - 1, x2 = x + 1, y2 = y + 1;
    }
    
    /* Or attempt to expand a 3x2 room (south) */
    else if (auto_build_room_floor(x-1, y, x+1, y+1)) {
	x1 = x - 1, y1 = y, x2 = x + 1, y2 = y + 1;
    }
    
    /* Or attempt to expand a 3x2 room (north) */
    else if (auto_build_room_floor(x-1, y-1, x+1, y)) {
	x1 = x - 1, y1 = y - 1, x2 = x + 1, y2 = y;
    }
        
    /* Or attempt to expand a 2x3 room (east) */
    else if (auto_build_room_floor(x, y-1, x+1, y+1)) {
	x1 = x, y1 = y - 1, x2 = x + 1, y2 = y + 1;
    }
    
    /* Or attempt to expand a 2x3 room (west) */
    else if (auto_build_room_floor(x-1, y-1, x, y+1)) {
	x1 = x - 1, y1 = y - 1, x2 = x, y2 = y + 1;
    }
    
    /* Or attempt to expand a 2x2 room (south east) */
    else if (auto_build_room_floor(x, y, x+1, y+1)) {
	x1 = x, y1 = y, x2 = x + 1, y2 = y + 1;
    }
    
    /* Or attempt to expand a 2x2 room (south west) */
    else if (auto_build_room_floor(x-1, y, x, y+1)) {
	x1 = x - 1, y1 = y, x2 = x, y2 = y + 1;
    }
    
    /* Or attempt to expand a 2x2 room (north east) */
    else if (auto_build_room_floor(x, y-1, x+1, y)) {
	x1 = x, y1 = y - 1, x2 = x + 1, y2 = y;
    }
    
    /* Or attempt to expand a 2x2 room (north west) */
    else if (auto_build_room_floor(x-1, y-1, x, y)) {
	x1 = x - 1, y1 = y - 1, x2 = x, y2 = y;
    }

    /* Hack -- only "snake" grids can grow corridors */
    else if (!auto_build_snake(x, y)) {
	x1 = x, y1 = y, x2 = x, y2 = y;
    }
    
    /* Or attempt to extend a corridor (south) */
    else if (auto_build_snake(x, y+1)) {
	x1 = x, y1 = y, x2 = x, y2 = y + 1;
    }

    /* Or attempt to extend a corridor (north) */
    else if (auto_build_snake(x, y-1)) {
	x1 = x, y1 = y - 1, x2 = x, y2 = y;
    }

    /* Or attempt to extend a corridor (east) */
    else if (auto_build_snake(x+1, y)) {
	x1 = x, y1 = y, x2 = x + 1, y2 = y;
    }

    /* Or attempt to extend a corridor (west) */
    else if (auto_build_snake(x-1, y)) {
	x1 = x - 1, y1 = y, x2 = x, y2 = y;
    }


    /* Default to 1x1 grid -- see below */
    else {
	x1 = x, y1 = y, x2 = x, y2 = y;
    }
    
    
    /* Hack -- Single grid rooms are never exciting */
    if ((x1 == x2) && (y1 == y2)) return (FALSE);
    

    /* Expand a north/south corridor */
    if (x1 == x2) {

	/* Grow south/north */
	while (auto_build_snake(x, y2+1)) y2++;
	while (auto_build_snake(x, y1-1)) y1--;
    }
    
    /* Expand a east/west corridor */
    else if (y1 == y2) {

	/* Grow east/west */
	while (auto_build_snake(x2+1, y)) x2++;
	while (auto_build_snake(x1-1, y)) x1--;
    }

    /* Expand a rectangle -- try south/north first */
    else if (randint(2) == 1) {

	/* Grow south/north */
	while (auto_build_room_floor(x1, y2+1, x2, y2+1)) y2++;
	while (auto_build_room_floor(x1, y1-1, x2, y1-1)) y1--;

	/* Grow east/west */
	while (auto_build_room_floor(x2+1, y1, x2+1, y2)) x2++;
	while (auto_build_room_floor(x1-1, y1, x1-1, y2)) x1--;
    }

    /* Expand a rectangle -- try east/west first */
    else {

	/* Grow east/west */
	while (auto_build_room_floor(x2+1, y1, x2+1, y2)) x2++;
	while (auto_build_room_floor(x1-1, y1, x1-1, y2)) x1--;

	/* Grow south/north */
	while (auto_build_room_floor(x1, y2+1, x2, y2+1)) y2++;
	while (auto_build_room_floor(x1, y1-1, x2, y1-1)) y1--;
    }


    /* XXX Hack -- refuse to build rooms touching unknowns */
    if (!auto_build_room_known(x1-1, y2+1, x2+1, y2+1)) return (FALSE);
    if (!auto_build_room_known(x1-1, y1-1, x2+1, y1-1)) return (FALSE);
    if (!auto_build_room_known(x2+1, y1-1, x2+1, y2+1)) return (FALSE);
    if (!auto_build_room_known(x1-1, y1-1, x1-1, y2+1)) return (FALSE);


    /* Make sure this room does not exist and is not contained */
    for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

	/* Never make a room "inside" another room */
	if ((ar->x1 <= x1) && (x2 <= ar->x2) &&
	    (ar->y1 <= y1) && (y2 <= ar->y2)) return (FALSE);
    }


    /* Message */
    auto_note(format("Building a room from %d,%d to %d,%d", x1, y1, x2, y2));
    
    
    /* Acquire a "free" room */
    i = auto_rooms[0].free;
    auto_rooms[0].free = auto_rooms[i].free;
    auto_rooms[i].free = 0;
    
    /* Paranoia */
    if (!i) core("Borg ran out of free rooms");
    
    /* Take note of new "maximums" */
    if (i + 1 > auto_room_max) auto_room_max = i + 1;
    
    /* Access the new room */
    ar = &auto_rooms[i];
	
    /* Initialize the new room */
    ar->x1 = x1;
    ar->x2 = x2;
    ar->y1 = y1;
    ar->y2 = y2;

    /* Paranoia */
    ar->when = c_t;

    /* Forget the flow */
    ar->flow = 0;
    ar->x = ar->y = 0;
    ar->prev = ar->next = NULL;
    
    /* Absorb old rooms */
    for (j = 1; j < auto_room_max; j++) {

	/* Skip the "current" room! */
	if (i == j) continue;
	
	/* Get the room */
	ar = &auto_rooms[j];
	
	/* Skip "free" rooms */
	if (!ar->when) continue;

	/* Skip non-contained rooms */
	if ((ar->x1 < x1) || (ar->y1 < y1)) continue;
	if ((x2 < ar->x2) || (y2 < ar->y2)) continue;

	/* Scan the "contained" room */
	for (y = ar->y1; y <= ar->y2; y++) {
	    for (x = ar->x1; x <= ar->x2; x++) {
	
		/* Get the "contained" grid */
		ag = grid(x, y);

		/* Normal grids "lose" their parents. */
		if (ag->room < AUTO_ROOMS) ag->room = 0;

		/* Cross-rooms lose one parent */
		if (ag->room > AUTO_ROOMS) ag->room--;
	    }
	}

	/* That room is now "gone" */
	ar->when = 0L;
	ar->x1 = ar->x2 = ar->y1 = ar->y2 = 0;
	ar->prev = ar->next = NULL;
	
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
	    ag = grid(x, y);

	    /* Steal "absorbed" grids */
	    if (ag->room == AUTO_ROOMS) ag->room = i;
	    
	    /* Steal "fresh" grids */
	    if (ag->room == 0) ag->room = i;

	    /* Skip grids owned by this room */
	    if (ag->room == i) continue;
	    
	    /* Normal grids become "cross-grids" */
	    if (ag->room < AUTO_ROOMS) ag->room = AUTO_ROOMS + 1;
	    
	    /* All cross-grids now have another parent */
	    ag->room++;
	}
    }

    /* The room set has changed */
    return (TRUE);
}




/*
 * Prepare to think -- "dungeon screen"
 */
static bool auto_prepare_start(void)
{
    int i, x, y;
    
    auto_grid *ag;
    auto_room *ar;

    byte t_a;

    char buf[128];

    /* Massive cheat */
    int view_n = auto_cheat_get_view_n();
    byte *view_x = auto_cheat_get_view_x();
    byte *view_y = auto_cheat_get_view_y();
    


    /* Increase the "time" */
    c_t++;
    
    /* Cheat -- Load the current location */
    c_y = char_row;
    c_x = char_col;


    /* Clear all the "state flags" */
    do_heal = do_food = do_lite = do_torch = do_flask = FALSE;
    

    /* Check for damage -- use the "hitpoint warning" option */
    if (0 == Term_what_text(6, 15, -6, &t_a, buf)) {
	if (t_a == TERM_RED) do_heal = TRUE;
    }


    /* Check for hunger */
    if (0 == Term_what_text(0, 23, 4, &t_a, buf)) {
        if (streq(buf, "Hung")) do_food = TRUE;
        if (streq(buf, "Weak")) do_food = TRUE;
        if (streq(buf, "Fain")) do_food = TRUE;
    }

    /* Note when we get hungry */
    if (do_food) auto_note("I am hungry.");    
    

    /* Extract the "current level" or abort */
    if (Term_what_text(70, 23, -7, &t_a, buf)) {
	auto_oops("No level");
	return (TRUE);
    }
    

    /* Verify the level */
    if (strcmp(buf, auto_level)) {

	/* Restart the clock */
	c_t = 10000L;

	/* Start a new level */
	auto_began = c_t;

	/* No stairs yet */
	goal_level = 0;
	
	/* No "real" rooms yet */
	auto_room_max = 0;

	/* Clean up the rooms */
	for (i = 0; i < AUTO_ROOMS; i++) {

	    /* Access the room */
	    ar = &auto_rooms[i];

	    /* Never seen it */
	    ar->when = 0L;

	    /* Paranoia -- No location yet */
	    ar->x1 = ar->x2 = ar->y1 = ar->y2 = 0;
	    
	    /* Not in the queue */
	    ar->next = ar->prev = NULL;
	    
	    /* No flow */
	    ar->flow = 0;
	    
	    /* Paranoia -- Visit the corner */
	    ar->x = ar->y = 0;

	    /* Hack -- Prepare the "free list" index */
	    ar->free = i + 1;
	}

	/* Hack -- run out of free space */
	auto_rooms[AUTO_ROOMS-1].free = 0;
	
	/* Clean up the grids */
	for (y = 0; y < AUTO_MAX_Y; y++) {
	    for (x = 0; x < AUTO_MAX_X; x++) {

		/* Access the grid */
		ag = grid(x, y);

		/* No room yet */
		ag->room = 0;

		/* Black means unknown */
		ag->o_a = 0;

		/* Space means unseen */
		ag->o_c = ' ';
	    }
	}

	/* No goal yet */
	goal = 0;
	
	/* Save the new level */
	strcpy(auto_level, buf);
    }


    /* Forget about the player */
    pg = NULL;
    
    /* Cheat -- Analyze the "viewable" part of the screen */
    for (i = 0; i < view_n; i++) {

	/* White means dark */
	byte a = TERM_WHITE;

	/* Space means unseen */
	char c = ' ';
	
	/* Access the "view grid" */
	int x = view_x[i];
	int y = view_y[i];

	/* On screen, look at the "onscreen map" */
	if (panel_contains(y, x)) {

	    /* Examine the "screen" */
	    Term_what(x - panel_col_prt, y - panel_row_prt, &a, &c);
	}

	/* Notice the player */
	if (c == '@') {

	    /* XXX The player is at (x,y) on the screen */
	    /* XXX But the screen may be "scrolled" a bit */
	    
	    /* Get the player's grid */
	    pg = grid(c_x, c_y);

	    /* Treat the grid as "unseen" */
	    c = ' ';
	}
	
	/* Ignore "blank" information */
	if (!a || (c == ' ')) continue;

	/* Hack -- Assume all floors and walls are white */
	if (strchr("#%.", c)) a = TERM_WHITE;
	
	/* Get the auto_grid */
	ag = grid(x, y);

	/* Save the "screen info" */
	ag->o_a = a, ag->o_c = c;
    }
    
    /* Paranoia -- make sure we exist */
    if (!pg) {
	auto_oops("Player missing!");
	return (TRUE);
    }    


    /* Make sure every visited grid has a room */
    if (!pg->room) {

	/* Acquire a "free" room */
	i = auto_rooms[0].free;
	auto_rooms[0].free = auto_rooms[i].free;
	auto_rooms[i].free = 0;

	/* Paranoia */
	if (!i) core("Borg ran out of free rooms");
	
	/* Take note of new "maximums" */
	if (i + 1 > auto_room_max) auto_room_max = i + 1;
	
	/* Access the new room */
	ar = &auto_rooms[i];
	
	/* Initialize the room */
	ar->flow = 0;	
	ar->x1 = ar->x2 = ar->x = c_x;
	ar->y1 = ar->y2 = ar->y = c_y;

	/* Saw this room */
	ar->when = c_t;

	/* Save the room */
	pg->room = i;
    }
    
    
    /* Occasionally, try to build better rooms */
    if (auto_build_room(c_x, c_y)) {

	/* Note that the room builder can kill "flow" goals */
	if (goal == GOAL_FLOW) goal = 0;
    }
    
    
    /* Mark all the "containing rooms" as visited. */
    for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) ar->when = c_t;


    /* XXX XXX Hack -- must cheat */
    cheat_inven = cheat_equip = TRUE;


    /* Send the "inventory" command */
    if (!cheat_inven) Borg_keypress('i');
    
    /* Enter next state */
    state = STATE_INVEN;

    /* All done */
    return (TRUE);
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
 * Given a weapon/armor, find the slot it will be wielded into
 *
 * Returns "-1" if the item cannot (or should not) be wielded
 *
 * Note that "Bows" really are not proper weapons at all...
 */
static int wield_slot(int tval)
{
    /* Slot for equipment */
    switch (tval) {

	case TV_DRAG_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SOFT_ARMOR:
	    return (INVEN_BODY);

	case TV_CLOAK:
	    return (INVEN_OUTER);

	case TV_SHIELD:
	    return (INVEN_ARM);

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
	    return (-1);
	    
	case TV_AMULET:
	    return (-1);
	    
	case TV_LITE:
	    return (INVEN_LITE);
    }

    /* No slot available */
    return (-1);
}




/*
 * Analyze a (clean) auto_item based on a description
 */
static void auto_item_analyze(auto_item *item, cptr desc, cptr cost)
{
    int i;

    cptr base;
    
    char *scan;
    char *tail;
    
    char temp[128];

    char p1 = '(', p2 = ')';
    char b1 = '[', b2 = ']';

    
    /* Wipe it */
    WIPE(item, auto_item);
	
    /* Save the item description */
    strcpy(item->desc, desc);

    /* Save the item cost */
    strcpy(item->cost, cost);

    /* Empty items are done */
    if (!desc[0]) return;

    /* Save the full description */
    strcpy(item->desc, desc);


    /* Assume singular */
    item->iqty = 1;
    
    /* Notice various "prefixes" */
    if (prefix(desc, "The ")) desc += 4;
    else if (prefix(desc, "an ")) desc += 3;
    else if (prefix(desc, "a ")) desc += 2;
    else if (prefix(desc, "1 ")) desc += 2;

    /* Notice "numerical" prefixes */
    else if (isdigit(desc[0])) {
	cptr s = strchr(desc, ' ');
	if (!s) return;
	item->iqty = atoi(desc);
	desc = s + 1;
    }

    /* Extract and remove the inscription */
    strcpy(temp, desc);
    scan = strchr(temp, '{' /* --}-- */);
    if (scan) strcpy(item->note, scan);
    if (scan && (scan[-1] == ' ')) scan[-1] = '\0';


    /* Hack -- non-aware, singular, flavored items */
    if (item->iqty == 1) {
	if (prefix(temp, "Scroll titled ")) item->tval = TV_SCROLL;
	else if (strstr(temp, " Potion")) item->tval = TV_POTION;
	else if (suffix(temp, " Hairy Mold")) item->tval = TV_FOOD;
	else if (suffix(temp, " Mushroom")) item->tval = TV_FOOD;
	else if (suffix(temp, " Amulet")) item->tval = TV_AMULET;
	else if (suffix(temp, " Ring")) item->tval = TV_RING;
	else if (suffix(temp, " Staff")) item->tval = TV_STAFF;
	else if (suffix(temp, " Wand")) item->tval = TV_WAND;
	else if (suffix(temp, " Rod")) item->tval = TV_ROD;
    }

    /* Hack -- non-aware, plural, flavored items */
    else {
	if (prefix(temp, "Scrolls titled ")) item->tval = TV_SCROLL;
	else if (strstr(temp, " Potions")) item->tval = TV_POTION;
	else if (suffix(temp, " Hairy Molds")) item->tval = TV_FOOD;
	else if (suffix(temp, " Mushrooms")) item->tval = TV_FOOD;
	else if (suffix(temp, " Amulets")) item->tval = TV_AMULET;
	else if (suffix(temp, " Rings")) item->tval = TV_RING;
	else if (suffix(temp, " Staffs")) item->tval = TV_STAFF;
	else if (suffix(temp, " Wands")) item->tval = TV_WAND;
	else if (suffix(temp, " Rods")) item->tval = TV_ROD;
    }

    /* Accept non-aware flavored objects */
    if (item->tval) return;
    

    /* Check all the item templates */
    for (i = 0; i < i_size; i++) {

	/* Extract the "base" string */
	base = (item->iqty == 1) ? i_single[i] : i_plural[i];

	/* Check for the proper prefix */
	if (prefix(temp, base)) break;
    }

    /* Oops.  Bizarre item. */
    if (i >= i_size) return;
    
    
    /* Save the item kind */
    item->kind = i_kind[i];

    /* Advance to the "tail", skip spaces */
    tail = temp + strlen(base);
    while (*tail == ' ') tail++;

    /* Extract some info */
    item->tval = k_list[item->kind].tval;
    item->sval = k_list[item->kind].sval;
    

    /* Hack -- Chests are too complicated */
    if (item->tval == TV_CHEST) return;
    

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

    /* Hack -- Examine Rods for charging */
    if (item->tval == TV_ROD) {
	item->able = TRUE;
	if (streq(tail, "(charging)")) item->pval = 999;
    }

    /* Hack -- Examine Rings/Amulets */
    if ((item->tval == TV_RING) || (item->tval == TV_AMULET)) {
    
	/* Some amulets have no extra information */
	if (k_list[item->kind].flags3 & TR3_EASY_KNOW) {
	    item->able = TRUE;
	}

	/* Others need a "pval" in parentheses */
        else if (tail[0] == p1) {
	    item->able = TRUE;
	    item->pval = atoi(tail + 1);
        }

        return;
    }

    /* Hack -- Extract Lites for Light */
    if (item->tval == TV_LITE) {
	i = extract_fuel(tail);
	if (i >= 0) item->able = TRUE;
	if (item->able) item->pval = i;
	return;
    }


    /* Must have a suffix */
    if (!tail[0]) return;
	
    /* Get the slot */
    i = wield_slot(item->tval);
    
    /* Wearable stuff */
    if (i >= 0) {

	bool done = FALSE;
	
	int d1 = 0, d2 = 0, ac = 0, th = 0, td = 0, ta = 0;
	

	/* XXX Check for artifact/ego items */
	/* XXX In particular "(Blessed)", "(Defender)", etc */
	/* XXX Note that "(Defender)" and such use parentheses */


	/* Parse the "damage" string for weapons */
	if ((tail[0] == p1) && (i == INVEN_WIELD)) {
	
	    /* First extract the damage string */
	    for (scan = tail; *scan != p2; scan++); scan++;

	    /* Notice "end of string" */
	    if (scan[0] != ' ') done = TRUE;

	    /* Terminate the string and advance */
	    *scan++ = '\0';

	    /* Parse the damage string, or stop XXX */
	    if (sscanf(tail, "(%dd%d)", &d1, &d2) != 2) return;

	    /* No extra information means not identified */
	    if (done) return;

	    /* Start on the damage bonus string */
	    tail = scan;
	}
	
	/* Parse the "damage" string for bows */
	else if ((tail[0] == p1) && (i == INVEN_BOW)) {
	
	    /* First extract the damage string */
	    for (scan = tail; *scan != p2; scan++); scan++;

	    /* Notice "end of string" */
	    if (scan[0] != ' ') done = TRUE;

	    /* Terminate the string and advance */
	    *scan++ = '\0';

	    /* Parse the damage string, or stop XXX */
	    if (sscanf(tail, "(x%d)", &d1) != 2) return;

	    /* No extra information means not identified */
	    if (done) return;

	    /* Start on the damage bonus string */
	    tail = scan;
	}
	

	/* Parse the "bonus" string */
	if (tail[0] == p1) {
	
	    /* XXX Extract the extra info */
	    for (scan = tail; *scan != p2; scan++); scan++;

	    /* Notice "end of string" */
	    if (scan[0] != ' ') done = TRUE;

	    /* Terminate the damage, advance */
	    *scan++ = '\0';

	    /* Parse the "bonuses" -- XXX Guess at weird ones */
	    if ((sscanf(tail, "(%d,%d)", &th, &td) != 2) &&
	        (sscanf(tail, "(%d)", &th) != 1)) return;

	    /* Known (bonuses) */
	    item->able = TRUE;

	    /* Nothing left */
	    if (done) return;

	    /* Then look for "armor" values */
	    tail = scan;
	}
	

	/* Parse the "bonus" string */
	if (tail[0] == b1) {
	
	    /* XXX Extract the extra info */
	    for (scan = tail; *scan != b2; scan++); scan++;

	    /* Notice "end of string" */
	    if (scan[0] != ' ') done = TRUE;

	    /* Terminate the armor string, advance */
	    *scan++ = '\0';

	    /* Parse the armor, and bonus */
	    if (sscanf(tail, "[%d,%d]", &ac, &ta) == 2) {
		item->able = TRUE;
	    }
	    else if (sscanf(tail, "[%d]", &ac) != 1) {
	        return;
	    }
	
	    /* Nothing left */
	    if (done) return;
	}
	

	/* Parse the "pval" string */
	if (tail[0] == p1) {

	    /* Assume identified */
	    item->able = TRUE;
	    
	    /* Grab it */
	    item->pval = atoi(tail + 1);
	}
    }
}




/*
 * Check the inventory for the quantity of objects of the given type
 */
static int auto_count_items(int tval, int sval)
{
    int i, k = 0;
    
    for (i = 0; i < INVEN_PACK; i++) {
	auto_item *item = &auto_items[i];
	if ((tval >= 0) && (item->tval != tval)) continue;
	if ((sval >= 0) && (item->sval != sval)) continue;
	k += item->iqty;
    }
    
    return (k);
}

 


/*
 * Send a command to inscribe item number "i" with the inscription "str".
 */
static void auto_send_inscribe(int i, cptr str)
{
    cptr s;
    
    /* The "inscribe" command */
    char draw = '{'; /* --}-- */

    /* Label it */
    Borg_keypress(draw);

    /* Hack -- allow "equipment" labelling */
    if (i >= INVEN_WIELD) {
	Borg_keypress('/');
	i -= INVEN_WIELD;
    }

    /* Choose the item */
    Borg_keypress('a' + i);

    /* Send the label */
    for (s = str; *s; s++) Borg_keypress(*s);

    /* End the inscription */
    Borg_keypress('\n');
}




/*
 * Determine if an item can be sold in the current store
 */
static bool auto_good_sell(auto_item *item)
{
    int tval = item->tval;

    /* Switch on the store */
    switch (state_store) {

      /* General Store */
      case 1:

	/* Analyze the type */
	switch (tval) {
	  case TV_DIGGING:
	  case TV_CLOAK:
	  case TV_FOOD:
	  case TV_FLASK:
	  case TV_LITE:
	  case TV_SPIKE:
	    return (TRUE);
	  default:
	    return (FALSE);
	}

      /* Armoury */
      case 2:

	/* Analyze the type */
	switch (tval) {
	  case TV_BOOTS:
	  case TV_GLOVES:
	  case TV_HELM:
	  case TV_SHIELD:
	  case TV_SOFT_ARMOR:
	  case TV_HARD_ARMOR:
	  case TV_DRAG_ARMOR:
	    return (TRUE);
	  default:
	    return (FALSE);
	}

      /* Weapon Shop */
      case 3:

	/* Analyze the type */
	switch (tval) {
	  case TV_SHOT:
	  case TV_BOLT:
	  case TV_ARROW:
	  case TV_BOW:
	  case TV_HAFTED:
	  case TV_POLEARM:
	  case TV_SWORD:
	    return (TRUE);
	  default:
	    return (FALSE);
	}

      /* Temple */
      case 4:

	/* Analyze the type */
	switch (tval) {
	  case TV_HAFTED:
	  case TV_SCROLL:
	  case TV_POTION:
	  case TV_PRAYER_BOOK:
	    return (TRUE);
	  default:
	    return (FALSE);
	}

      /* Alchemist */
      case 5:

	/* Analyze the type */
	switch (tval) {
	  case TV_SCROLL:
	  case TV_POTION:
	    return (TRUE);
	  default:
	    return (FALSE);
	}

      /* Magic Shop */
      case 6:

	/* Analyze the type */
	switch (tval) {
	  case TV_AMULET:
	  case TV_RING:
	  case TV_STAFF:
	  case TV_WAND:
	  case TV_SCROLL:
	  case TV_POTION:
	  case TV_MAGIC_BOOK:
	  case TV_ROD:
	    return (TRUE);
	  default:
	    return (FALSE);
	}
    }

    /* XXX */
    return (FALSE);
}



/*
 * Determine if an item should be bought from a store
 */
static bool auto_good_buy(auto_item *ware)
{
    int slot = -1;
    auto_item *worn = NULL;


    /* Determine where the item would be worn */
    slot = wield_slot(ware->tval);

    /* Extract the item currently in that slot */
    if (slot >= 0) worn = &auto_items[slot];

    /* Hack -- notice empty slots */
    if (!worn->iqty) worn = NULL;
    

    /* Food */
    if (ware->tval == TV_FOOD) {

	/* Only buy "normal" food */
	if (ware->sval < SV_FOOD_MIN_FOOD) return (FALSE);
	
	/* Always have at least 50 food */
	if (auto_count_items(TV_FOOD, -1) >= 50) return (FALSE);

	/* Hack -- always buy food */
	return (TRUE);
    }
    
    /* Flasks */
    else if (ware->tval == TV_FLASK) {

	/* Never buy more than 50 flasks */
	if (auto_count_items(TV_FLASK, -1) >= 50) return (FALSE);
	
	/* Hack -- examine the lite */
	worn = &auto_items[INVEN_LITE];
	if (!worn->iqty) worn = NULL;
	
	/* Hack -- buy flasks for lanterns */
	if (worn && (worn->sval == SV_LITE_LANTERN)) return (TRUE);
	
	/* Never buy random flasks */
	return (FALSE);
    }

#if 0
    /* Scrolls */
    else if (ware->tval == TV_SCROLL) {

	/* Only buy scrolls of identify */
	if (ware->sval != TV_SCROLL_IDENTIFY) return (FALSE);
	
	/* Never buy more than 50 scrolls */
	if (auto_count_items(ware->tval, ware->sval) >= 50) return (FALSE);

	/* Hack -- buy it */
	return (TRUE);
    }
#endif
    
    /* Process Torches */
    else if ((ware->tval == TV_LITE) && (ware->sval == SV_LITE_TORCH)) {

	/* Never buy more than 20 torches */
	if (auto_count_items(TV_LITE, SV_LITE_TORCH) >= 20) return (FALSE);

	/* Hack -- Torches are defeated by lanterns with fuel */
	if (worn && (worn->sval == SV_LITE_LANTERN)) {
	    if (auto_count_items(TV_FLASK, -1) >= 10) return (FALSE);
	}
	
	/* Hack -- always buy torches */
	return (TRUE);
    }

    /* Process Lanterns */
    else if ((ware->tval == TV_LITE) && (ware->sval == SV_LITE_LANTERN)) {

	/* Never buy a lantern when wielding one already */
	if (worn && (worn->sval == SV_LITE_LANTERN)) return (FALSE);

	/* Never buy more than 1 lantern */
	if (auto_count_items(TV_LITE, SV_LITE_LANTERN) >= 1) return (FALSE);

	/* Hack -- always buy a lantern */
	return (TRUE);
    }

    /* Process equipment */
    else if (slot >= 0) {

	/* Prefer "expensive" equipment */
        if (!worn || (k_list[ware->kind].cost > k_list[worn->kind].cost)) {

	    /* Hack -- Buy it */
	    return (TRUE);	
	}
    }

    /* Assume useless */
    return (FALSE);
}


/*
 * Examine changed items in the inventory
 */
static void auto_notice(void)
{
    int i, slot;
    
    /* Analyze the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

	auto_item *item = &auto_items[i];
	auto_item *worn = NULL;


	/* Skip "empty"/"bizarre" items */
	if (!item->iqty || !item->tval) continue;

	/* Skip items we have already examined */
	if (item->done) continue;

	/* This item has been examined */
	item->done = TRUE;
	
	/* Clear the flags */
	item->wear = item->cash = item->junk = FALSE;

	
	/* See what slot that item could go in */
	slot = wield_slot(item->tval);
		
	/* Extract the item currently in that slot */
	if (slot >= 0) worn = &auto_items[slot];

	/* Hack -- ignore empty slots */
	if (!worn->iqty) worn = NULL;
	

	/* Mark "junk" as needing to be thrown away */
	if ((suffix(item->desc, " {junk}")) ||
	    (suffix(item->desc, " {cursed}")) ||
	    (item->tval <= TV_SPIKE) ||
	    (item->kind && !k_list[item->kind].cost)) {

	    /* This is junk */
	    item->junk = TRUE;

	    continue;
	}

	/* Process potions/scrolls */
	if ((item->tval == TV_SCROLL) ||
	    (item->tval == TV_POTION)) {

	    /* Destroy known crap */
	    if (item->kind && (k_list[item->kind].cost < greed)) {
		item->junk = TRUE;
	    }

	    /* Sell the rest */
	    else {
		item->cash = TRUE;
	    }

	    continue;
	}

	/* Process rings/amulets */
	if ((item->tval == TV_RING) ||
	    (item->tval == TV_AMULET)) {

	    /* Destroy known crap */
	    if (item->kind && (k_list[item->kind].cost < greed)) {
		item->junk = TRUE;
	    }

	    /* Sell the rest */
	    else {
		item->cash = TRUE;
	    }

	    continue;
	}

	/* Process rings/amulets */
	if ((item->tval == TV_ROD) ||
	    (item->tval == TV_WAND) ||
	    (item->tval == TV_STAFF)) {

	    /* Destroy known crap */
	    if (item->kind && (k_list[item->kind].cost < greed)) {
		item->junk = TRUE;
	    }

	    /* Sell the rest */
	    else {
		item->cash = TRUE;
	    }

	    continue;
	}


	/* Ignore unaware items */
	if (!item->kind) continue;


	/* Process Lite's */
	if (item->tval == TV_LITE) {

	    /* Hack -- never wear "empty" Lites */
	    if (!item->pval) continue;
	    
	    /* Always replace empty lites */
	    if (!worn || !worn->pval) {
		item->wear = TRUE;
	    }
	    
	    /* Prefer lanterns to torches */
	    else if (k_list[item->kind].cost > k_list[worn->kind].cost) {
		item->wear = TRUE;
	    }

	    /* Notice "junky" torches */
	    else if (item->sval == SV_LITE_TORCH) {

		/* Never keep more than 20 torches */
		if (auto_count_items(TV_LITE, SV_LITE_TORCH) > 20) {
		    item->cash = TRUE;
		}

		/* Lantern plus fuel pre-empts torches */
		else if (worn->sval == SV_LITE_LANTERN) {
		    if (auto_count_items(TV_FLASK, -1) > 10) {
			item->cash = TRUE;
		    }
		}
	    }
	    	    
	    /* Notice "extra" lanterns */
	    else if (item->sval == SV_LITE_LANTERN) {

		/* Already wielding a lantern */
		if (worn->sval == SV_LITE_LANTERN) {
		    item->cash = TRUE;
		}

		/* Already carrying a lantern */
		if (auto_count_items(TV_LITE, SV_LITE_LANTERN) > 1) {
		    item->cash = TRUE;
		}
	    }
	    	    
	    continue;
	}

	
	/* Ignore unwearable items */
	if (slot < 0) continue;
	
	/* Known (or average) items can be worn */
	if (item->able || suffix(item->desc, " {average}")) {

	    /* Compare to the current item, wear it if better */
	    if (k_list[item->kind].cost > k_list[worn->kind].cost) {
		item->wear = TRUE;
	    }

	    /* Throw away "junk" */
	    else if (k_list[item->kind].cost < greed) {
		item->junk = TRUE;
	    }

	    /* Otherwise sell it */	    
	    else {
		item->cash = TRUE;
	    }
	    
	    continue;
	}
    }
}



/*
 * Destroy everything we know we don't want
 */
static bool auto_throw_junk(void)
{
    int i;

    /* Throw away "junk" */
    for (i = 0; i < INVEN_PACK; i++) {

	/* Skip "unknown" items */
	if (auto_items[i].junk) {

	    /* Throw it at a monster or myself */
	    Borg_keypress('f');
	    Borg_keypress('a' + i);
	    Borg_keypress('*');
	    Borg_keypress('t');

	    /* Did something */
	    return (TRUE);
	}
    }


    /* Nothing to destroy */
    return (FALSE);
}

    

/*
 * Make sure we have at least one free inventory slot
 * Return TRUE if an action was performed.
 */
static bool auto_free_space(void)
{
    int i;
    
    u32b limit = 10L;


    /* Throw junk until done */
    while (auto_items[21].iqty) {

	int k = 0;
	
	/* Try for junk first */
	if (auto_throw_junk()) return (TRUE);


	/* Prevent infinite loops */
	for (i = 0; i < INVEN_PACK; i++) {
	    auto_item *item = &auto_items[i];
	    if (item->cash) k++;
	}
	
	/* Hack -- Crash if confused */
	if (!k) {
	    auto_oops("Too much stuff!");
	    return (TRUE);
	}


	/* Mark items as junk */
	for (i = 0; i < INVEN_PACK; i++) {

	    u32b cost;
	
	    auto_item *item = &auto_items[i];
	
	    /* Examine "sell" items */
	    if (!item->cash) continue;

	    /* Unknowns are worth 200 gold */
	    if (!item->kind) cost = rand_range(100,300);
	
	    /* Extract the XXX base cost */
	    else cost = k_list[item->kind].cost;

	    /* Multiply by the quantity */
	    cost *= item->iqty;
	    
	    /* Save expensive items */
	    if (cost > limit) continue;

	    /* Debug */
	    auto_note(format("Junking %ld gold", cost));
	    
	    /* Mark it as junk */
	    item->junk = TRUE;
	}

	/* Increase the limit */
	limit *= 2;
    }


    /* Success */
    return (FALSE);
}

    


/*
 * Maintain a "useful" inventory
 */
static bool auto_wear_stuff(void)
{
    int i;
        

    /* Look at the "lite" slot */    
    i = INVEN_LITE;

    /* Check the current "light source" */
    if (!auto_items[i].iqty) {
	do_lite = TRUE;
    }
    else if (auto_items[i].sval == SV_LITE_TORCH) {
	if (auto_items[i].pval < 1000) do_torch = TRUE;
	if (auto_items[i].pval < 1) do_lite = TRUE;
    }
    else if (auto_items[i].sval == SV_LITE_LANTERN) {
	if (auto_items[i].pval < 5000) do_flask = TRUE;
	if (auto_items[i].pval < 1) do_lite = TRUE;
    }


    /* Refuel a torch */
    if (do_torch) {

	/* Try to wield some other lite */
	for (i = 0; i < INVEN_PACK; i++) {
	    if ((auto_items[i].tval == TV_LITE) &&
		(auto_items[i].sval == SV_LITE_LANTERN)) {
		Borg_keypress('w');
		Borg_keypress('a' + i);
		return (TRUE);
	    }
	}

	/* Scan the inventory -- note "empties" are gone */
	for (i = 0; i < INVEN_PACK; i++) {
	    if ((auto_items[i].tval == TV_LITE) &&
		(auto_items[i].sval == SV_LITE_TORCH)) {
		Borg_keypress('F');
		return (TRUE);
	    }
	}
    }


    /* Refuel a lantern */
    if (do_flask) {

	/* Scan the inventory */
	for (i = 0; i < INVEN_PACK; i++) {
	    if (auto_items[i].tval == TV_FLASK) {
		Borg_keypress('F');
		return (TRUE);
	    }
	}
    }


    /* Get a new lite */
    if (do_lite) {

	/* Scan the inventory looking for (non-empty) lanterns */
	for (i = 0; i < INVEN_PACK; i++) {
	    if ((auto_items[i].tval == TV_LITE) &&
		(auto_items[i].sval == SV_LITE_LANTERN)) {
		Borg_keypress('w');
		Borg_keypress('a' + i);
		return (TRUE);
	    }
	}

	/* Scan the inventory looking for (non-empty) torches */
	for (i = 0; i < INVEN_PACK; i++) {
	    if ((auto_items[i].tval == TV_LITE) &&
		(auto_items[i].sval == SV_LITE_TORCH)) {
		Borg_keypress('w');
		Borg_keypress('a' + i);
		return (TRUE);
	    }
	}
    }


    /* Wear stuff (top down) */
    for (i = 0; i < INVEN_PACK; i++) {

	/* Skip "unknown" items */
	if (auto_items[i].wear) {

	    /* Wear it */
	    Borg_keypress('w');
	    Borg_keypress('a' + i);

	    /* Did something */
	    return (TRUE);
	}
    }


    /* Nothing to do */
    return (FALSE);
}




/*
 * Analyze the "inventory" screen
 */
static bool auto_prepare_inven(void)
{
    int i, notice = FALSE;
    
    char buf[256];

    bool done = FALSE;
    
    
    /* Extract the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

	/* Default to "nothing" */
	buf[0] = '\0';

	/* Cheat -- extract the inventory directly */
	if (cheat_inven) {

	    /* Extract a real item */
	    if (inventory[i].tval) {
		objdes(buf, &inventory[i], TRUE);
		buf[75] = '\0';
	    }
	}

	/* Actually parse the screen */
	else if (!done) {

	    /* XXX Not implemented -- see store parser */
	    /* XXX Be sure to strip any trailing spaces */
	}

	/* Ignore "unchanged" items */
	if (streq(buf, auto_items[i].desc)) continue;
	
	/* Hack -- remember to "notice" everything */
	notice = TRUE;
	
	/* Analyze the item (no price) */
	auto_item_analyze(&auto_items[i], buf, "");
    }

    /* When the inventory changes, make sure to recheck everything */
    if (notice) {
	for (i = 0; i < INVEN_TOTAL; i++) auto_items[i].done = FALSE;
    }
    
    /* Show the "equipment screen" */
    if (!cheat_equip) Borg_keypress('e');

    /* Prepare to parse the equipment */
    state = STATE_EQUIP;

    /* Done */
    return (TRUE);
}


/*
 * Analyze the "equipment" screen
 */
static bool auto_prepare_equip(void)
{
    int i, notice = FALSE;
    
    char buf[160];

    /* Extract the inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

	/* Default to "nothing" */
	buf[0] = '\0';

	/* Cheat -- extract the inventory directly */
	if (cheat_equip) {

	    /* Extract a real item */
	    if (inventory[i].tval) {
		objdes(buf, &inventory[i], TRUE);
		buf[75] = '\0';
	    }
	}

	/* Actually parse the screen */
	else {

	    /* XXX Not implemented */
	    /* XXX Be sure to strip trailing spaces */

	    /* Use the nice "show_empty_slots" flag */
	    if (streq(buf, "(nothing)")) strcpy(buf, "");
	}

	/* Ignore "unchanged" items */
	if (streq(buf, auto_items[i].desc)) continue;

	/* Remember to re-examine everything */
	notice = TRUE;
	
	/* Analyze the item (no price) */
	auto_item_analyze(&auto_items[i], buf, "");
    }
    
    /* When the inventory changes, make sure to recheck everything */
    if (notice) {
	for (i = 0; i < INVEN_TOTAL; i++) auto_items[i].done = FALSE;
    }
    
    /* Leave the "equipment screen" */
    if (!cheat_inven || !cheat_equip) Borg_keypress(ESCAPE);

    /* Enter the "think" state */
    state = STATE_THINK;

    /* Hack -- go to the "store" state instead */
    if (state_store) state = STATE_STORE;

    /* Success */
    return (TRUE);
}





/*
 * Analyze a "store" screen
 */
static bool auto_prepare_store(void)
{
    int i;

    byte t_a;

    cptr s;
    
    char what[32];

    char desc[80];
    char cost[10];
    
    char buf[256];

    /* Multiple pages? */
    int more = 0;

    /* Current page */
    int page = 0;

    /* Hack -- make sure both pages are seen */
    static int browse = TRUE;
    
    /* What store did I *think* I was in */
    static int old_store = 0;

    /* How many pages did I *think* there were */
    static int old_more = 0;


    /* Extract the "store" name (or "Home") */
    if (Term_what_text(50, 3, -20, &t_a, what)) what[0] = '\0';
    if (!what[0]) strcpy(what, "Home");

    /* React to new stores */
    if (old_store != state_store) {

	/* Clear all the items */
	for (i = 0; i < 24; i++) {

	    /* XXX Wipe the ware */
	    WIPE(&auto_wares[i], auto_item);
	}
	
	/* Save the store */
	old_store = state_store;
    }
        
    
    /* Extract the "page", if any */
    if ((0 == Term_what_text(20, 5, 8, &t_a, buf)) &&
	(prefix(buf, "(Page "))) /* --)-- */ {

	/* Take note of the page */
	more = 1, page = (buf[6] - '0') - 1;
    }

    /* React to disappearing pages */
    if (old_more != more) {

	/* Clear the second page */
	for (i = 12; i < 24; i++) {

	    /* XXX Wipe the ware */
	    WIPE(&auto_wares[i], auto_item);
	}
	
	/* Save the new one */
	old_more = more;
    }
    

    /* Extract the current gold (unless in home) */
    if (0 == Term_what_text(68, 19, -9, &t_a, buf)) {

	/* Ignore this field in the home */
	if (state_store != 8) strcpy(auto_gold, buf);
    }


    /* Note */
    auto_note(format("In store %s (%d), page %d/%d, with %s gold",
		     what, state_store, page+1, more+1, auto_gold));


    /* Parse the store (or home) inventory */
    for (i = 0; i < 12; i++) {

	char /* p1 = '(', */ p2 = ')';

	/* Default to "empty" */
	desc[0] = '\0';
	cost[0] = '\0';
	
	/* Verify "intro" to the item */
	if ((0 == Term_what_text(0, i + 6, 3, &t_a, buf)) &&
	    (buf[0] == 'a' + i) && (buf[1] == p2) && (buf[2] == ' ')) {

	    /* Extract the item description */
	    if (Term_what_text(3, i + 6, -65, &t_a, desc)) desc[0] = '\0';

	    /* XXX Make sure trailing spaces get stripped (?) */
	    	
	    /* Extract the item cost */
	    if (Term_what_text(68, i + 6, -9, &t_a, cost)) cost[0] = '\0';

	    /* Hack -- forget the cost in the home */
	    if (state_store == 8) cost[0] = '\0';
	}

	/* Ignore "unchanged" descriptions */
	if (streq(desc, auto_wares[page*12+i].desc)) continue;

	/* Analyze it (including the cost) */
	auto_item_analyze(&auto_wares[page*12+i], desc, cost);
    }


    /* Hack -- browse as needed */
    if (more && browse) {
	Borg_keypress(' ');
	browse = FALSE;
	return (TRUE);
    }

    /* Hack -- must browse later */
    browse = TRUE;
    

    /* Examine the inventory */
    auto_notice();
    
    /* Wear things */
    if (auto_wear_stuff()) {

	/* Send the "inventory" command */
	if (!cheat_inven) Borg_keypress('i');

	/* Prepare to parse */
	state = STATE_INVEN;

	/* Success */
	return (TRUE);
    }

    
    /* Sell stuff */
    for (i = 0; i < INVEN_PACK; i++) {

	auto_item *item = &auto_items[i];

	/* XXX Hack -- notice "full" store */
	if (auto_wares[23].iqty) break;
	
	/* Item must be marked as sellable */
	if (!item->iqty) continue;
	if (!item->cash) continue;

	/* Do not try to make "bad" sales */
	if (!auto_good_sell(item)) continue;

	/* Note */
	auto_note(format("Selling %s to the %s", item->desc, what));

	/* Hack -- sell it */
	Borg_keypress('s');
	
	/* Sell that item */
	Borg_keypress('a' + i);

	/* XXX XXX Hack -- ignore inscriptions */
	if (strcmp(item->note, "")) Borg_keypress('y');

	/* Hack -- Sell a single item */
	if (item->iqty > 1) Borg_keypress('\n');

	/* Accept the price, skip the messages */
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');

	/* Send the "inventory" command */
	if (!cheat_inven) Borg_keypress('i');

	/* Prepare to parse */
	state = STATE_INVEN;

	/* Success */
	return (TRUE);
    }

    /* Buy stuff */
    for (i = 0; i < 24; i++) {

	auto_item *ware = &auto_wares[i];

	/* Notice end of shop inventory */
	if (!ware->iqty) break;
	
	/* XXX Hack -- notice "full" inventory */
	if (auto_items[21].iqty) break;

	/* We must have enough cash */
	if (strcmp(auto_gold, ware->cost) < 0) continue;

	/* Only buy useful stuff */
	if (!auto_good_buy(ware)) continue;

	/* Note */
	auto_note(format("Buying %s from the %s", ware->desc, what));

        /* Hack -- wrong page */
        if ((i / 12) != page) Borg_keypress(' ');
        
	/* Buy an item */
	Borg_keypress('p');

	/* Buy the desired item */
	Borg_keypress('a' + (i % 12));

	/* Buy a single item */
	if (ware->iqty > 1) Borg_keypress('\n');

	/* Skip the messages */
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');
	Borg_keypress('\n');

	/* Send the "inventory" command */
	if (!cheat_inven) Borg_keypress('i');
	
	/* Prepare to parse */
	state = STATE_INVEN;

	/* Success */
	return (TRUE);
    }
    
    

    /* Leave the store */
    Borg_keypress(ESCAPE);

    /* Forget that we are in a store */
    state_store = 0;
    
    /* Prepare to start over */
    state = STATE_START;

    /* Done */
    return (TRUE);
}





/*
 * Is a grid "okay" for us to run into
 *
 * This routine is ONLY called by "auto_goto_dir()".
 */
static bool auto_okay_grid(int x, int y)
{
    auto_grid *ag = grid(x, y);
    
    /* Try not to walk into walls (but do so if commanded) */
    if (strchr("#%", ag->o_c)) return (FALSE);
    
    /* Try not to walk into stores (but do so if commanded) */
    if (strchr("12345678", ag->o_c)) return (FALSE);
    
    /* Avoid monsters unless in "killing" mode (or commanded) */
    if ((goal != GOAL_KILL) && strchr(auto_str_kill, ag->o_c)) return (FALSE);

    /* Assume okay */
    return (TRUE);
}


/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 */
static int auto_extract_dir(int x1, int y1, int x2, int y2)
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
static int auto_goto_dir(int x1, int y1, int x2, int y2)
{
    int d;
    

    /* Special case -- next to (or on) the goal */
    if ((ABS(y2-y1) <= 1) && (ABS(x2-x1) <= 1)) {
	return (auto_extract_dir(x1, y1, x2, y2));
    }

    
    /* Try to do the "horizontal" */
    if (ABS(y2 - y1) < ABS(x2 - x1)) {
	d = auto_extract_dir(x1, y1, x2, y1);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    }
    
    /* Try to do the "vertical" */
    if (ABS(y2 - y1) > ABS(x2 - x1)) {
	d = auto_extract_dir(x1, y1, x1, y2);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    }
    

    /* Try to walk "directly" there */
    d = auto_extract_dir(x1, y1, x2, y2);
    
    /* Check for walls */
    if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    
    
    /* Try the "vertical" instead (includes "diagonal") */
    if (ABS(y2 - y1) <= ABS(x2 - x1)) {
	d = auto_extract_dir(x1, y1, x1, y2);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    }

    /* Try the "horizontal" instead (includes "diagonal") */
    if (ABS(y2 - y1) >= ABS(x2 - x1)) {
	d = auto_extract_dir(x1, y1, x2, y1);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    }
    
    
    /* Hack -- directly "vertical", try "shaking" */
    if (x2 == x1) {
    
	/* Shake to the east */
	d = auto_extract_dir(x1, y1, x1+1, y2);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    
	/* Shake to the west */
	d = auto_extract_dir(x1, y1, x1-1, y2);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    }
    
    /* Hack -- directly "horizontal", try "shaking" */
    if (y2 == y1) {
    
	/* Shake to the south */
	d = auto_extract_dir(x1, y1, x2, y1+1);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    
	/* Shake to the north */
	d = auto_extract_dir(x1, y1, x2, y1-1);
	if (auto_okay_grid(x1 + dx[d], y1 + dy[d])) return (d);
    }
    

    /* Hack -- Surrounded by walls.  Dig through them. */
    d = auto_extract_dir(x1, y1, x2, y2);
    
    /* Let the calling routine check the result */
    return (d);
}


/*
 * Process a "goto" goal, return "TRUE" if goal is still okay.
 */
static bool auto_play_step(int x, int y)
{
    auto_grid *ag;
    
    int dir;


    /* We have arrived */
    if ((c_x == x) && (c_y == y)) return (FALSE);
    
    /* Get a direction (may be a wall there) */
    dir = auto_goto_dir(c_x, c_y, x, y);


    /* Access the grid we are stepping on */
    ag = grid(c_x + dx[dir], c_y + dy[dir]);

    /* Must "disarm" traps */
    if (ag->o_c == '^') {
	Borg_keypress('#');
        Borg_keypress('D');
    }

    /* Must "open" (or "bash") doors */
    else if (ag->o_c == '+') {
	Borg_keypress('#');
        if (randint(10) == 1) Borg_keypress('B');
        else if (ag->o_c == '+') Borg_keypress('o');
    }

    /* Tunnel through rubble */
    else if (strchr(":", ag->o_c)) {
	Borg_keypress('#');
        Borg_keypress('T');
    }

    /* Tunnel through walls/seams */
    /* Hack -- eventually, give up */
    else if (strchr("#%", ag->o_c)) {
	if (randint(5) == 1) goal = 0;
	Borg_keypress('#');
        Borg_keypress('T');
    }

    /* XXX Hack -- Occasionally, tunnel for gold */
    else if (strchr("$*", ag->o_c) && (randint(10) == 1)) {
	Borg_keypress('#');
        Borg_keypress('T');
    }
    
    /* XXX XXX Hack -- Occasionally, tunnel for objects */
    else if (strchr(auto_str_take, ag->o_c) && (randint(20) == 1)) {
	Borg_keypress('#');
        Borg_keypress('T');
    }
    
    /* XXX XXX Hack -- Occasionally, tunnel into monsters */
    else if (strchr(auto_str_kill, ag->o_c) && (randint(30) == 1)) {
	Borg_keypress('#');
        Borg_keypress('T');
    }
    
    /* Walk in that direction */
    Borg_keypress('0' + dir);
    
    /* Sometimes prepare to enter a "store" */
    if (strchr("12345678", ag->o_c)) {
	state_store = (ag->o_c - '0');
        state = STATE_STORE;
    }
    
    /* Hack -- prepare to take stairs if desired */
    if ((ag->o_c == '<') && (goal_level < 0)) Borg_keypress('<');
    if ((ag->o_c == '>') && (goal_level > 0)) Borg_keypress('>');
    
    /* Did something */
    return (TRUE);
}


/*
 * Attempt to fire at the given location
 */
static bool auto_play_fire(int x2, int y2)
{
    int i, x1 = c_x, y1 = c_y;

    auto_grid *ag;
    
    /* Only a one in five chance */
    if (randint(5) != 1) return (FALSE);
    
    /* Must be "on screen" */
    if (!panel_contains(y2, x2)) return (FALSE);
    
    /* Must not be adjacent */
    if (distance(x1, y1, x2, y2) <= 1) return (FALSE);
    
    /* Hack -- do not "fire" at mushrooms/skeletons */
    ag = grid(x2, y2);
    if (ag->o_c == ',') return (FALSE);
    if (ag->o_c == 's') return (FALSE);
    
    /* Find an arrow or something */
    for (i = 0; i < INVEN_PACK; i++) {
	if (auto_items[i].tval == TV_BOLT) break;
	if (auto_items[i].tval == TV_ARROW) break;
	if (auto_items[i].tval == TV_SHOT) break;
    }

    /* Nothing to fire */
    if (i == INVEN_PACK) return (FALSE);

    /* Fire! */
    Borg_keypress('f');
    Borg_keypress('a' + i);

    /* Target the location */
    Borg_keypress('*');
    Borg_keypress('l');
    
    /* Start at the player */
    x1 = c_x, y1 = c_y;
    
    /* Move to the location */
    for ( ; y1 < y2; y1++) Borg_keypress('2');
    for ( ; y1 > y2; y1--) Borg_keypress('8');
    for ( ; x1 < x2; x1++) Borg_keypress('6');
    for ( ; x1 > x2; x1--) Borg_keypress('4');

    /* Select the target */
    Borg_keypress('t');
    
    /* Success */
    return (TRUE);
}


/*
 * Process the current goal
 *
 * Note that "goto", "take", and "kill" are "identical".
 *
 * Return TRUE if this goal is still "okay".
 * Otherwise, cancel the goal and return FALSE.
 */
static bool auto_play_old_goal(void)
{
    auto_grid *ag;
    auto_room *ar;
    
    /* Process "KILL" goals */
    if (goal == GOAL_KILL) {

	/* Get the goal grid */
	ag = grid(g_x, g_y);
    
	/* Verify match, attempt a step */
	if ((ag->o_a == g_a) && (ag->o_c == g_c)) {

	    /* Try shooting */
	    if (auto_play_fire(g_x, g_y)) return (TRUE);
	    
	    /* Try walking */
	    if (auto_play_step(g_x, g_y)) return (TRUE);
	}
    }

    /* Process "GOTO" goals */
    else if (goal == GOAL_GOTO) {

	/* Get the goal grid */
	ag = grid(g_x, g_y);
    
	/* Verify match, attempt a step */
	if ((ag->o_a == g_a) && (ag->o_c == g_c) &&
	    auto_play_step(g_x, g_y)) return (TRUE);
    }

    /* Process "TAKE" goals */
    else if (goal == GOAL_TAKE) {

	/* Get the goal grid */
	ag = grid(g_x, g_y);
    
	/* Verify match, attempt a step */
	if ((ag->o_a == g_a) && (ag->o_c == g_c) &&
	    auto_play_step(g_x, g_y)) return (TRUE);
    }

    /* Process "FLOW" goals */
    else if (goal == GOAL_FLOW) {

	int x = c_x, y = c_y, cost = MAX_SHORT;

	/* Scan all the rooms we are in */
	for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {
	    if (ar->flow >= cost) continue;
	    x = ar->x, y = ar->y, cost = ar->flow;
	}
	
	/* Try to take a "step", or cancel the goal */
	if ((cost < MAX_SHORT) && (auto_play_step(x, y))) return (TRUE);
    }
    

    /* Cancel the goal */
    goal = 0;

    /* Nothing to do */
    return (FALSE);
}



/*
 * Some "fake" sentinel nodes for the queue
 */
static auto_room auto_flow_head;
static auto_room auto_flow_tail;


/*
 * Determine if the queue is empty
 */
static bool auto_flow_empty(void)
{
    /* Semi-Hack -- Test for Empty Queue. */
    if (auto_flow_head.next == &auto_flow_tail) return (TRUE);
    if (auto_flow_tail.prev == &auto_flow_head) return (TRUE);

    /* Must have entries */
    return (FALSE);
}



/*
 * Dequeue from the "priority queue" of "auto_flow" records
 */
static auto_room *auto_flow_dequeue(void)
{
    auto_room *ar;

    /* Hack -- check for empty */
    if (auto_flow_empty()) return (NULL);
        
    /* Access the first node in the queue */
    ar = auto_flow_head.next;
    
    /* Dequeue the entry */
    ar->next->prev = ar->prev;
    ar->prev->next = ar->next;
    
    /* Forget the links */
    ar->next = ar->prev = NULL;
    
    /* Return the room */
    return (ar);
}


/*
 * Enqueue into the "priority queue" of "auto_flow" records
 */
static void auto_flow_enqueue(auto_room *ar)
{
    auto_room *node;

    /* Start at the sentinel head */
    node = &auto_flow_head;
    
    /* Hack -- Find a good location in the queue */
    while (ar->flow >= node->next->flow) node = node->next;

    /* Tell the node */
    ar->prev = node;
    ar->next = node->next;

    /* Tell the queue */
    ar->prev->next = ar;
    ar->next->prev = ar;
}

 

/*
 * Aux function -- Given a room "ar1" and a location (x,y) along the
 * edge of that room, and a "direction" of motion, find all rooms that
 * include the resulting grid, and attempt to enqueue them.
 *
 * Looks like we have to make sure that crossing rooms do not attempt
 * to recurse back on themselves or anything weird.
 *
 * Note that we assume that the "direction" is not a "diagonal".
 */
static void auto_flow_spread_aux(auto_room *ar1, int x, int y, int d)
{
    int flow;

    int x2 = x + dx[d];
    int y2 = y + dy[d];
        
    auto_grid *ag;
    auto_room *ar;
    
    /* Look to the given direction */
    ag = grid(x2, y2);

    /* Ignore unknown grids, walls, seams, and unroomed grids */
    if (ag->o_c == ' ') return;
    if (strchr("#%", ag->o_c)) return;
    if (!ag->room) return;
    
    /* Extract the "cost" between the two points */
    flow = ar1->flow + distance(x, y, ar1->x, ar1->y);
    flow = flow + ABS(dx[d]) + ABS(dy[d]);
    
    /* Now find every room containing that grid */
    for (ar = room(1,x2,y2); ar; ar = room(0,0,0)) {

	/* Paranoia -- Skip grids that contain the destination */
	/* if (auto_room_contains(ar, x, y)) continue; */
	
	/* Do not "lose" previous gains */
	if (flow >= ar->flow) continue;

	/* Save the "destination" location */
	ar->x = x, ar->y = y;

	/* Save the new "cheaper" cost */
	ar->flow = flow;

	/* Hack -- Handle "upgrades" */
	if (ar->prev) {

	    /* Hack -- may not have to change anything */
	    if (ar->prev->flow <= ar->flow) continue;
	    
	    /* Dequeue the entry */
	    ar->next->prev = ar->prev;
	    ar->prev->next = ar->next;
    
	    /* Forget the links */
	    ar->next = ar->prev = NULL;
	}

	/* Enqueue the room */
	auto_flow_enqueue(ar);
    }
}


/*
 * Spread the "flow", assuming some rooms have been enqueued.
 *
 * We assume that the "flow" fields of all the rooms have been
 * initialized via "auto_flow_clear()", and that then the "flow"
 * fields of the rooms in the queue were set to "zero" (or any
 * other number) before being enqueued.
 *
 * We also assume that none of the icky grids have the "prev"
 * or "next" pointers set to anything.
 */
static void auto_flow_spread(void)
{
    int x, y;

    auto_room *ar;


    /* Keep going until the queue is empty */
    while (!auto_flow_empty()) {

	/* Dequeue the next room */	
	ar = auto_flow_dequeue();

	/* Scan the south/north edges */
	for (x = ar->x1; x <= ar->x2; x++) {
		
	    /* South edge */
	    y = ar->y2;
	
	    /* Look to the south */
	    auto_flow_spread_aux(ar, x, y, 2);

	    /* North edge */
	    y = ar->y1;
	
	    /* Look to the north */
	    auto_flow_spread_aux(ar, x, y, 8);
	}
	
	/* Scan the east/west edges */
	for (y = ar->y1; y <= ar->y2; y++) {
		
	    /* East edge */
	    x = ar->x2;
	
	    /* Look to the east */
	    auto_flow_spread_aux(ar, x, y, 6);

	    /* West edge */
	    x = ar->x1;
	
	    /* Look to the west */
	    auto_flow_spread_aux(ar, x, y, 4);
	}

	
	/* Look along the South-East corner */
	auto_flow_spread_aux(ar, ar->x2, ar->y2, 3);
	
	/* Look along the South-West corner */
	auto_flow_spread_aux(ar, ar->x1, ar->y2, 1);
	
	/* Look along the North-East corner */
	auto_flow_spread_aux(ar, ar->x2, ar->y1, 9);
	
	/* Look along the North-West corner */
	auto_flow_spread_aux(ar, ar->x1, ar->y1, 7);
    }
}


/*
 * Reset the "flow codes" of all "real" rooms to a large number
 */
static void auto_flow_clear(void)
{
    int i;
    
    /* Reset the cost of all the (real) rooms */
    for (i = 0; i < auto_room_max; i++) {

	/* Get the room */
	auto_room *ar = &auto_rooms[i];

	/* Skip "dead" rooms */	
	if (!ar->when) continue;
	
	/* Initialize the "flow" */
	ar->flow = MAX_SHORT;

	/* Paranoia -- clear the pointers */
	ar->next = ar->prev = NULL;
    }
}



/*
 * Do a "reverse" flow -- find unreachable rooms
 */
static void auto_flow_reverse()
{
    auto_room *ar;


    /* Clear the flow codes */
    auto_flow_clear();
    
    /* Enqueue the player's rooms */
    for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

	/* Mark rooms as "cheap" */
	ar->flow = 0;
	
	/* Save the player's location */
	ar->x = c_x, ar->y = c_y;
		
	/* Enqueue the room */
	auto_flow_enqueue(ar);
    }
    
    /* Attempt to spread, or fail */
    auto_flow_spread();
}


/*
 * Prepare to "flow" towards "interesting" things
 */
static bool auto_flow_symbols(cptr what)
{
    int x, y, n = 0;

    auto_grid *ag;
    auto_room *ar;
        

    /* Clear the flow codes */
    auto_flow_clear();


    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
	for (x = 1; x < AUTO_MAX_X-1; x++) {

	    /* Get the grid */
	    ag = grid(x, y);

	    /* Skip current location */
	    if (pg == ag) continue;
	    
	    /* Ignore unknown (and unroomed) grids */
	    if ((ag->o_c == ' ') || !ag->room) continue;
	    
	    /* Require symbols from the given string */
	    if (!strchr(what, ag->o_c)) continue;

	    /* Scan all the rooms that hold it */
	    for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

	    	/* Skip rooms we already added */
		if (!ar->flow) continue;
			    
		/* Mark rooms as "cheap" */
		ar->flow = 0;
	
		/* Save the location */
		ar->x = x, ar->y = y;
		
		/* Enqueue the room */
		auto_flow_enqueue(ar);

		/* Count the rooms */
		n++;
	    }
	}
    }
    
    /* Nothing to spread */
    if (!n) return (FALSE);
    
    /* Spread the flow */
    auto_flow_spread();

    /* Hmmm -- we may not be able to get to the grids */
    for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

	/* See if we can flow there */
	if (ar->flow < MAX_SHORT) {
	
	    /* Note */
	    auto_note(format("Flowing toward '%s' symbols, at cost %d", what, ar->flow));
    
	    /* Set the "goal" */
	    goal = GOAL_FLOW;

	    /* Success */
	    return (TRUE);
	}
    }
    
    /* Oops.  We must be stuck */
    return (FALSE);
}



/*
 * Prepare to "flow" towards "interesting" things
 */
static bool auto_flow_explore(void)
{
    int x, y, d, i, n = 0;

    auto_room *ar;
    
    auto_grid *ag, *ag1;
    

    /* Clear the flow codes */
    auto_flow_clear();


    /* Examine every known grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
	for (x = 1; x < AUTO_MAX_X-1; x++) {

	    /* Get the grid */
	    ag = grid(x, y);

	    /* Skip unknown/unroomed grids, plus walls/seams */			
	    if ((ag->o_c == ' ') || !ag->room) continue;			
	    if (strchr("#%", ag->o_c)) continue;
	    		    
	    /* Examine the four IMPORTANT neighbors */
	    for (i = 0; i < 4; i++) {

		/* Extract the next direction */
		d = dd[i];
		    
		/* Get the grid */
		ag1 = grid(x+dx[d], y+dy[d]);

		/* Note that walls/seams are boring */
		if (strchr("#%", ag1->o_c)) continue;
		
		/* Skip "known", "roomed" grids */
		if ((ag1->o_c != ' ') && ag1->room) continue;

		/* Scan all the rooms that hold the "known" grid */
		for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

		    /* Paranoia -- skip dead rooms */
		    if (!ar->when) continue;
		    
		    /* Hack -- skip rooms we already added */
		    if (ar->flow < MAX_SHORT) continue;
		    
		    /* Mark rooms as "cheap" */
		    ar->flow = 0;
		
		    /* Save the location */
		    ar->x = x, ar->y = y;
	    
		    /* Enqueue the room */
		    auto_flow_enqueue(ar);
		    
		    /* Count the rooms */
		    n++;
		}

		/* And then stop */
		break;
	    }
	}
    }
    
    /* Nothing to spread */
    if (!n) return (FALSE);
    
    /* Spread the flow */
    auto_flow_spread();

    /* Hmmm -- we may not be able to get to the grids */
    for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

	/* See if we can flow there */
	if (ar->flow < MAX_SHORT) {
	
	    /* Note */
	    auto_note(format("Exploring, at cost %d", ar->flow));
        
	    /* Set the "goal" */
	    goal = GOAL_FLOW;

	    /* Success */
	    return (TRUE);
	}
    }
    
    /* Oops.  We must be stuck */
    return (FALSE);
}


/*
 * Prepare to "flow" towards "old" rooms.
 */
static bool auto_flow_revisit(void)
{
    int x, y, i;

    auto_room *ar;
    
    int r_n = -1;
    u32b r_age = 0L;

    u32b age;


    /* Hack -- first find the reachable spaces */
    auto_flow_reverse();
     
    
    /* Re-visit "old" rooms */
    for (i = 0; i < auto_room_max; i++) {

	/* Access the "room" */
	ar = &auto_rooms[i];

 	/* Skip "dead" rooms */
	if (!ar->when) continue;

	/* Skip "unreachable" rooms */
	if (ar->flow == MAX_SHORT) continue;
	
	/* Reward "age" and "distance" and "luck" */
	age = (c_t - ar->when) + (ar->flow / 2);
	
	/* Skip "recent" rooms */
	if ((r_n >= 0) && (age < r_age)) continue;
	
	/* Save the index, and the age */
	r_n = i, r_age = age;
    }

    /* Clear the flow codes */
    auto_flow_clear();

    /* Hack -- No rooms to visit */
    if (r_n < 0) return (FALSE);
    
    /* Get the room */
    ar = &auto_rooms[r_n];

    /* Visit a random grid of that room */
    x = ar->x = rand_range(ar->x1, ar->x2);
    y = ar->y = rand_range(ar->y1, ar->y2);

    /* Mark the room as "cheap" */
    ar->flow = 0;

    /* Enqueue the room */
    auto_flow_enqueue(ar);
    	    
    /* Spread the flow */
    auto_flow_spread();

    /* Hmmm -- we may not be able to get to the grids */
    for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

	/* See if we can flow there */
	if (ar->flow < MAX_SHORT) {
	
	    /* Note */
	    auto_note(format("Revisting (%d,%d) with age %ld, at cost %d",
			     x, y, r_age, ar->flow));
    
	    /* Set the "goal" */
	    goal = GOAL_FLOW;

	    /* Success */
	    return (TRUE);
	}
    }

    /* Success */
    return (FALSE);
}


/*
 * Act spastic, go somewhere silly
 */
static bool auto_flow_spastic(void)
{
    int i, n = 0;

    auto_room *ar;
    

    int r_n = -1;


    /* Hack -- first find the reachable spaces */
    auto_flow_reverse();
    
    /* Pick a "random" room */
    for (i = 0; i < auto_room_max; i++) {

	/* Access the "room" */
	ar = &auto_rooms[i];

 	/* Skip "dead" rooms */
	if (!ar->when) continue;

	/* Skip "unreachable" rooms */
	if (ar->flow == MAX_SHORT) continue;
	
	/* Skip annoying rooms */
	if ((ar->x1 == ar->x2) || (ar->y1 == ar->y2)) continue;
	
	/* Pick a random room */
	if (!rand_int(++n)) r_n = i;
    }

    /* Clear the flow codes */
    auto_flow_clear();

    /* Hack -- No rooms to visit */
    if (r_n < 0) return (FALSE);
    
    /* Get the room */
    ar = &auto_rooms[r_n];

    /* Visit a random grid along the wall */
    if (randint(2) == 1) {
	ar->x = rand_int(2) ? ar->x1 : ar->x2;
	ar->y = rand_range(ar->y1, ar->y2);
    }
    else {
	ar->x = rand_range(ar->x1, ar->x2);
	ar->y = rand_int(2) ? ar->y1 : ar->y2;
    }
    
    /* Note */
    auto_note(format("Spastic twitch towards (%d,%d)", ar->x, ar->y));
    
    /* Mark the room as "cheap" */
    ar->flow = 0;

    /* Enqueue the room */
    auto_flow_enqueue(ar);
    	    
    /* Spread the flow */
    auto_flow_spread();

    /* Set the "goal" */
    goal = GOAL_FLOW;

    /* Success */
    return (TRUE);
}



/*
 * Perform one "useful" action
 *
 * Return "TRUE" if successful, "FALSE" if failed.
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 */
static bool auto_prepare_think(void)
{
    int view_n = auto_cheat_get_view_n();
    byte *view_x = auto_cheat_get_view_x();
    byte *view_y = auto_cheat_get_view_y();
    
    int i, x, y, f;

    auto_grid *ag;

    /* Best "monster" data */
    int m_f = -1;

    /* Best "object" data */
    int i_f = -1;
    
    /* Best "unknown" data */
    int u_f = -1;
    

    /*** Hack -- restart every time ***/
    
    /* Assume we will go back to the start state */
    state = STATE_START;

    /* Hack -- Beware of "near death" */
    if (do_heal) {
	auto_oops("near death");
	return (TRUE);
    }
    
    
    /*** Analyze the inventory ***/
    

    /* Eat */
    if (do_food) {

	auto_note("I am REALLY hungry.");
	
	/* Scan the inventory */
	for (i = 0; i < INVEN_PACK; i++) {

	    /* Skip non-food */
	    if (auto_items[i].tval != TV_FOOD) continue;
	    
	    /* Eat food rations and such */
	    if (auto_items[i].sval >= SV_FOOD_MIN_FOOD) {

		auto_tell("About to eat...");
		Borg_keypress('E');
		Borg_keypress('a' + i);
		auto_tell("Finished eating...");

		auto_note("I am preparing to eat.");
	
		return (TRUE);
	    }
	}
    
	/* Hack -- Require food! */
	auto_oops("starving");
	return (TRUE);
    }
    


    /*** Reactive Scan (Monsters, Items, Doors, etc) ***/
    
    /* Remember and prefer "current" monster, if useful */
    if ((goal == GOAL_KILL) && (c_x != g_x) && (c_y != g_y)) {

	/* Calculate the distance */
	m_f = distance(c_y, c_x, g_y, g_x);
    }

    /* Cheat -- Scan the "viewable space" */
    for (i = 0; i < view_n; i++) {

	/* Access the "view grid" */
	y = view_y[i];
	x = view_x[i];

	/* Get the auto_grid */
	ag = grid(x, y);

	/* Hack -- Skip the player */
	if (ag == pg) continue;
	
	/* Notice monsters */
	if (strchr(auto_str_kill, ag->o_c)) {

	    /* Calculate the distance */
	    f = distance(c_y, c_x, y, x);
	
	    /* Keep track of the "nearest" monster */
	    if ((m_f < 0) || (f < m_f)) {

		/* Memorize the distance and location */
		m_f = f, g_x = x, g_y = y;

		/* Memorize the goal contents */
		g_a = ag->o_a, g_c = ag->o_c;

		/* Reset the "origin" data */
		o_y = c_y, o_x = c_x, o_t = c_t;

		/* Kill it */
		goal = GOAL_KILL;
	    }
	}
    }

    /* Chase and Kill the "closest" monster */
    if (m_f >= 0) return (auto_play_old_goal());


    /*** Then verify the inventory ***/

    /* Examine the inventory */
    auto_notice();
    
    /* Always have a free space available */
    if (auto_free_space()) return (TRUE);
    
    /* Attempt to keep the inventory in good shape */
    if (auto_wear_stuff()) return (TRUE);
    

    /*** Then try objects ***/
    
    /* Remember and prefer "current" object, if useful */
    if ((goal == GOAL_TAKE) && (c_x != g_x) && (c_y != g_y)) {

	/* Calculate the distance */
	i_f = distance(c_y, c_x, g_y, g_x);
    }

    /* Cheat -- Scan the "viewable space" */
    for (i = 0; i < view_n; i++) {

	/* Access the "view grid" */
	y = view_y[i];
	x = view_x[i];

	/* Get the auto_grid */
	ag = grid(x, y);

	/* Skip the player */
	if (ag == pg) continue;
	
	/* Notice "good" objects, doors, traps, rubble */
	if (strchr(auto_str_take, ag->o_c)) {

	    /* Calculate the distance */
	    f = distance(c_y, c_x, y, x);
	
	    /* Keep track of the "nearest" door */
	    if ((i_f < 0) || (f < i_f)) {

		/* Save the distance and location */
		i_f = f, g_x = x, g_y = y;

		/* Memorize the goal contents */
		g_a = ag->o_a, g_c = ag->o_c;

		/* Reset the "origin" data */
		o_y = c_y, o_x = c_x, o_t = c_t;

		/* Take it or Open it */
		goal = GOAL_TAKE;	
	    }
	}
    }

    /* Grab the "closest" item */
    if (i_f >= 0) return (auto_play_old_goal());
        


    /*** And then try local exploring ***/

    /* Remember and prefer "current" destination, if useful */
    if ((goal == GOAL_GOTO) && (c_x != g_x) && (c_y != g_y)) {

	/* Calculate the distance */
	u_f = distance(c_y, c_x, g_y, g_x);
    }

    /* Cheat -- Scan the "viewable space" */
    for (i = 0; i < view_n; i++) {

	/* Access the "view grid" */
	y = view_y[i];
	x = view_x[i];

	/* Get the auto_grid */
	ag = grid(x, y);

	/* Skip the player */
	if (ag == pg) continue;
	
	/* Notice "unknown" grids */
	if ((ag->o_c == ' ') || (!ag->room && !strchr("#%", ag->o_c))) {

	    /* Calculate the distance */
	    f = distance(c_y, c_x, y, x);
	
	    /* Keep track of the "nearest" one */
	    if ((u_f < 0) || (f < u_f)) {

		/* Save the distance and location */
		u_f = f, g_x = x, g_y = y;

		/* Hack -- Memorize the goal "contents" */
		g_a = ag->o_a, g_c = ag->o_c;

		/* Reset the "origin" data */
		o_y = c_y, o_x = c_x, o_t = c_t;

		/* Attempt to "go" there */
		goal = GOAL_GOTO;
	    }
	}
    }

    /* Approach "unknown" grids */
    if (u_f >= 0) return (auto_play_old_goal());
    

    /*** More inventory maintenance ***/
    
    /* Occasionally clear out all useless items */
    if (auto_throw_junk()) return (TRUE);
    


    /*** All out of reactive goals ***/

    
    /* Hack -- perhaps search */
    if (randint(100) == 1) {
	Borg_keypress('#');
	Borg_keypress('5');
	Borg_keypress('s');
	return (TRUE);
    }

    
    /* Maintain old goals (explore, symbols, spastic, revisit) */
    if (goal == GOAL_FLOW) return (auto_play_old_goal());


    /* Explore */
    if (auto_flow_explore()) return (auto_play_old_goal());


#if 0
    /* Message */
    auto_note(format("I have spent %ld turns on this level...",
    		     (c_t - auto_began)));
#endif
    

    /* Get bored eventually, leave the level */
    if (c_t - auto_began > 2000L) {
	cptr what;
	goal_level = -1;
	if (streq(auto_level, "   Town")) goal_level = 1;
	if (!auto_items[17].iqty) goal_level = 1;
	what = (goal_level > 0) ? ">" : "<";
	if (auto_flow_symbols(what)) return (auto_play_old_goal());
    }


    /* Occasionally, act stupid */
    if ((c_t - auto_began > 1000L) && (randint(3) == 1)) {
	if (auto_flow_spastic()) return (auto_play_old_goal());
    }
    
    
    /* Re-visit old rooms */
    if (auto_flow_revisit()) return (auto_play_old_goal());


    /* XXX Hack -- Twitch */
    Borg_keypress('s');
    if (randint(5) == 1) Borg_keypress('T');
    Borg_keypress('0' + randint(9));
    return (TRUE);
}





/*
 * Let the "auto player" enqueue some keypresses
 */
static void auto_play_perform(void)
{
    /* Uses original keypress commands */
    rogue_like_commands = FALSE;

    /* Uses the viewable space array directly */
    view_pre_compute = TRUE;

    /* Uses color to track monsters and for "damage" sensing */
    use_color = TRUE;

    /* Hack -- uses hilite to determine state */
    hilite_player = FALSE;

    /* Auto-player gets confused by auto-repeat */
    always_repeat = FALSE;

    /* Paranoia -- require explicit acknowledgement */
    quick_messages = FALSE;

    /* Auto-player gets confused by extra info */
    plain_descriptions = TRUE;

    /* Buy/Sell without haggling */
    no_haggle_flag = TRUE;

    /* Cancel stupid messages */
    always_throw = TRUE;
    

    /* Loop until something has been done */
    while (state) {

	/* Handle "stores" when necessary */
	if ((state == STATE_STORE) && auto_prepare_store()) break;

	/* And then check the dungeon */
	if ((state == STATE_START) && auto_prepare_start()) break;

	/* And then check the inventory and equipment */
	if ((state == STATE_INVEN) && auto_prepare_inven()) break;
	if ((state == STATE_EQUIP) && auto_prepare_equip()) break;
	
	/* And then actually think */
	if ((state == STATE_THINK) && auto_prepare_think()) break;
    }
}





/*
 * Maintain the "old" hook
 */
static errr (*Term_xtra_hook_old)(int n, int v) = NULL;


/*
 * Our own hook.  Allow thinking when no keys ready.
 */
static errr Term_xtra_borg(int n, int v)
{
    /* Hack -- The Borg pre-empts keypresses */
    while (state && (n == TERM_XTRA_EVENT)) {
    
	int i, x, y;
	byte t_a;
	char buf[128];

	errr res = 0;    

	bool visible;

	static inside = 0;

	/* Hack -- pause when main window hidden */
	if (Term != term_screen) break;

	/* Hack -- pause in wizard mode */
	if (wizard) break;
	
	/* Hack -- no recursion */
	if (inside) {
	    Term_keypress(' ');
	    return (0);
	}
	
	/* Hack -- Extract the cursor visibility */
	visible = (!Term_hide_cursor());
	if (visible) Term_show_cursor();
    
	/* Hack -- Pass most methods through */
	res = (*Term_xtra_hook_old)(TERM_XTRA_CHECK, v);
    
	/* Hack -- If the cursor is visible... */
	/* And the cursor is on the top line... */
	/* And there is text before the cursor... */
	/* And that text is "-more-", then clear it */
	if (visible &&
	    (!Term_locate(&x, &y) && (y == 0) && (x >= 6)) &&
	    (!Term_what_text(x-6, y, 6, &t_a, buf)) &&
	    (streq(buf, "-more-"))) {

	    /* Clear the message */
	    Term_keypress(' ');

	    /* Done */
	    return (0);
	}

	/* Check for a Borg keypress */
	i = Borg_inkey();

	/* Take the next keypress */
	if (i) {

	    /* Enqueue the keypress */
	    Term_keypress(i);

	    /* Success */
	    return (0);
	}
	
	/* Inside */
	inside++;
	
	/* Think */
	auto_play_perform();

	/* Outside */
	inside--;
    }

    /* Hack -- Usually just pass the call through */
    return ((*Term_xtra_hook_old)(n, v));
}




/*
 * Initialize the Borg
 */
void Borg_init(void)
{
    int i, k;

    char buf[256];
	

    /* Only initialize once */
    if (ready) return;

    /* Message */
    msg_print("Initializing the Borg...");
    Term_fresh();
    

    /*** Input/Output ***/
        
    /* Remember the "normal" event scanner */
    Term_xtra_hook_old = Term->xtra_hook;
    
    /* Cheat -- drop a hook into the "event scanner" */
    Term->xtra_hook = Term_xtra_borg;


    /*** Dungeon Arrays ***/
    
    /* Make the array of rooms */
    C_MAKE(auto_rooms, AUTO_ROOMS, auto_room);

    /* Initialize the rooms */
    for (i = 0; i < AUTO_ROOMS; i++) auto_rooms[i].self = i;

    /* Make the array of grids */
    C_MAKE(auto_grids, AUTO_MAX_Y, auto_grid*);

    /* Make each row of grids */
    for (i = 0; i < AUTO_MAX_Y; i++) {
	C_MAKE(auto_grids[i], AUTO_MAX_X, auto_grid);
    }


    /*** Flow queue ***/
    
    /* The head is always first */
    auto_flow_head.flow = 0;

    /* The tail is always last */
    auto_flow_tail.flow = MAX_SHORT;
    
    /* Empty queue */
    auto_flow_head.next = &auto_flow_tail;
    auto_flow_tail.prev = &auto_flow_head;
    

    /*** Item/Ware arrays ***/
    
    /* Make some item/ware arrays */
    C_MAKE(auto_items, INVEN_TOTAL, auto_item);
    C_MAKE(auto_wares, 24, auto_item);


    /*** Item description parsers ***/
    
    /* Count the useful "item kinds" */
    for (k = 0; k < MAX_K_IDX; k++) {

	/* Get the kind */
	inven_kind *k_ptr = &k_list[k];
	
	/* Skip non-items */
	if (!k_ptr->tval) continue;
	
	/* Skip "dungeon terrain" objects */
	if (k_ptr->tval >= TV_GOLD) continue;

	/* Skip "special artifacts" (including the Phial) */
	if (k_ptr->flags3 & TR3_INSTA_ART) continue;
	
	/* Count the items */
	i_size++;
    }

    /* Allocate the "item parsing arrays" */
    C_MAKE(i_kind, i_size, u16b);
    C_MAKE(i_single, i_size, cptr);
    C_MAKE(i_plural, i_size, cptr);
        
    /* Analyze the "item kinds" -- Hack -- reverse order */
    for (i = 0, k = MAX_K_IDX-1; k >= 0; k--) {

	inven_type hack;
	
	/* Get the kind */
	inven_kind *k_ptr = &k_list[k];
	
	/* Skip non-items */
	if (!k_ptr->tval) continue;
	
	/* Skip "dungeon terrain" objects */
	if (k_ptr->tval >= TV_GOLD) continue;

	/* Skip "special artifacts" (including the Phial) */
	if ((k_ptr->tval >= TV_MIN_WEAR) &&
	    (k_ptr->tval <= TV_MAX_WEAR) &&
	    (k_ptr->flags3 & TR3_INSTA_ART)) continue;
	
	/* Save the object kind */
	i_kind[i] = k;

	/* Hack -- make an item */
	invcopy(&hack, k);
	
	/* Hack -- Force "known" */
	hack.ident |= ID_KNOWN;

	/* Describe a "plural" object */
	hack.number = 2;
	objdes(buf, &hack, FALSE);
	i_plural[i] = string_make(buf);

	/* Describe a "singular" object */
	hack.number = 1;
	objdes(buf, &hack, FALSE);
	i_single[i] = string_make(buf);
	
	/* Advance */
	i++;
    }
    
    /* Hack -- Unknown level */
    strcpy(auto_level, "?x?x?x?x?");

    /* Done initialization */
    msg_print("done.");

    /* Now it is ready */
    ready = TRUE;
}


/* 
 * Hack -- interact with the Borg.  Includes "initialization" call.
 */
void Borg_mode(void)
{
    char cmd;


    /* Not initialized? */
    if (!ready) return;
    

    /* Hack -- Never interupt the "special states" */
    if (state && (state != STATE_START)) return;
    
    /* Clear the state */
    state = 0;
    
    /* Get a special command */    
    if (!get_com("Borg command: ", &cmd)) return;

    
    /* Command: Resume */
    if (cmd == 'r') {
	wizard = FALSE;
	state = STATE_START;
    }

    /* Command: Resume, but Wipe goals */
    else if (cmd == 'w') {
	wizard = FALSE;
	state = STATE_START;
	goal_level = 0;
	goal = 0;
    }
    
    /* Command: Restart -- use an "illegal" level */
    else if (cmd == 'z') {
	wizard = FALSE;
	state = STATE_START;
	goal_level = 0;
	goal = 0;
	sprintf(auto_level, "?x?-%ld-?x?", (long)turn);
    }


    /* Command: toggle "cheat" for "inventory" */
    else if (cmd == 'i') {
	cheat_inven = !cheat_inven;
    }
    
    /* Command: toggle "cheat" for "equipment" */
    else if (cmd == 'e') {
	cheat_equip = !cheat_equip;
    }
    

    /* Command: Show all Rooms (Containers) */
    else if (cmd == 's') {

	int n, k, x, y;

	auto_room *ar;

	/* Examine all the rooms */
	for (n = 0; n < auto_room_max; n++) {

	    int i = 0;
	    
	    /* Access the n'th room */
	    ar = &auto_rooms[n];

	    /* Skip "dead" rooms */
	    if (!ar->when) continue;

	    /* Hack -- hilite the room -- count draws */
	    for (y = ar->y1; y <= ar->y2; y++) {	    
		for (x = ar->x1; x <= ar->x2; x++) {	    
		    if (panel_contains(y, x)) i++;
		    mh_print_rel('*', TERM_RED, 0, y, x);
		}
	    }

	    /* Nothing done -- skip this room */
	    if (!i) continue;

	    /* Describe and wait */
	    Term_putstr(0, 0, -1, TERM_WHITE,
	    		format("Room %d (%dx%d). ", n,
	    		(1 + ar->x2 - ar->x1), (1 + ar->y2 - ar->y1)));
	    Term_fresh();

	    /* Get a key */
	    k = Term_inkey();
	    
	    /* Hack -- fix the room */
	    for (y = ar->y1; y <= ar->y2; y++) {	    
		for (x = ar->x1; x <= ar->x2; x++) {	    
		    lite_spot(y, x);
		}
	    }
	    
	    /* Flush the erase */
	    Term_fresh();

	    /* Leave this loop */
	    if (k == ESCAPE) break;
	}
    }

    /* Command: Rooms (Containers) */
    else if (cmd == 'c') {

	int n, w, h;

	auto_room *ar;

	int used = 0, n_1x1 = 0, n_1xN = 0, n_Nx1 = 0;
	int n_2x2 = 0, n_2xN = 0, n_Nx2 = 0, n_NxN = 0;
	
	/* Examine all the rooms */
	for (n = 0; n < AUTO_ROOMS; n++) {

	    /* Access the n'th room */
	    ar = &auto_rooms[n];

	    /* Skip "dead" rooms */
	    if (!ar->when) continue;
	    
	    /* Count the "used" rooms */
	    used++;

	    /* Extract the "size" */
	    w = 1 + ar->x2 - ar->x1;
	    h = 1 + ar->y2 - ar->y1;

	    /* Count the "singles" */
	    if ((w == 1) && (h == 1)) n_1x1++;
	    else if (w == 1) n_1xN++;
	    else if (h == 1) n_Nx1++;
	    else if ((w == 2) && (h == 2)) n_2x2++;
	    else if (w == 2) n_2xN++;
	    else if (h == 2) n_Nx2++;
	    else n_NxN++;
	}


	/* Display some info */	
	msg_print(format("Rooms: %d/%d used.", used, auto_room_max));
	msg_print(format("Corridors: 1xN (%d), Nx1 (%d)", n_1xN, n_Nx1));
	msg_print(format("Thickies: 2x2 (%d), 2xN (%d), Nx2 (%d)",
			 n_2x2, n_2xN, n_Nx2));
	msg_print(format("Singles: %d.  Normals: %d.", n_1x1, n_NxN));
	msg_print(NULL);
    }

    /* Command: Grid Info */
    else if (cmd == 'g') {

	int x, y, n;

	auto_grid *ag;
	auto_room *ar;

	int tg = 0;
	int tr = 0;

	int cc[16];
	
	/* Count the crossing factors */
	cc[0] = cc[1] = cc[2] = cc[3] = cc[4] = cc[5] = cc[6] = cc[7] = cc[8] = 0;
	
	/* Total explored grids */
	for (y = 0; y < AUTO_MAX_Y; y++) {
	    for (x = 0; x < AUTO_MAX_X; x++) {

		/* Get the grid */
		ag = grid(x, y);

		/* Skip unknown grids */
		if (ag->o_c == ' ') continue;

		/* Count them */
		tg++;

		/* Count the rooms this grid is in */
		for (n = 0, ar = room(1,x,y); ar && (n<16); ar = room(0,0,0)) n++;

		/* Hack -- Mention some locations */
		if ((n > 1) && !cc[n]) {
		    msg_print(format("The grid at %d,%d is in %d rooms.", x, y, n));
		}

		/* Count them */
		if (n > 0) tr++;
		
		/* Count the number of grids in that many rooms */
		cc[n]++;
	    }
	}

	/* Display some info */	
	msg_print(format("Grids: %d known, %d in rooms.", tg, tr));
	msg_print(format("Roomies: %d, %d, %d, %d, %d, %d, %d, %d, %d.",
			 cc[0], cc[1], cc[2], cc[3], cc[4],
			 cc[5], cc[6], cc[7], cc[8]));
	msg_print(NULL);
    }
}


#endif

