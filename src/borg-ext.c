/* File: borg-ext.c */

/* Purpose: Helper file for "borg-ben.c" -BEN- */

#include "angband.h"

#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-obj.h"

#include "borg-map.h"

#include "borg-ext.h"



/*
 * See "borg-ben.c" for more information.
 *
 * One big thing this file does is "object/monster tracking", which
 * attempts to gather information about the objects and monsters in
 * the dungeon, including their identity, location, and state.  This
 * information can then be used to choose logical actions.  And since
 * we now have memory, we can deduce state over time, such as whether
 * a monster is sleeping, or whether an item is embedded in a wall.
 *
 * To Do:
 *   Heuristics for monsters affected by missile/spell attacks
 *   Notice inability to move (walls, monsters, confusion)
 *   Track approximate monster hitpoints (min/max hitpoints?)
 *   If so, factor in regeneration and various spell attacks
 *   Handle the presence of invisible/unknown monsters (!)
 *   Analyze monster resistances before using frost bolt, etc
 *   Try not to wake up sleeping monsters by throwing things
 *   Perhaps even attempt to use the "sleep" spell when useful
 *
 * Bugs:
 *   Groups of monsters may induce faulty monster matching
 *   Teleporting monsters may induce faulty monster matching
 *   Monsters which appear singly and in groups are "weird"
 *   Certain "bizarre" monsters (mimics, trappers) are ignored
 *   Ignore player ghosts, and aborts if they are observed
 *   Currently "charge()" and "caution()" interact badly (!)
 *   The timestamps are not quite in sync properly (?)
 *   Annoyance and Danger are very different things (!)
 */


/*
 * Some information
 */

s16b goal;			/* Goal type */

bool goal_rising;		/* Currently returning to town */

bool goal_fleeing;		/* Currently fleeing the level */

bool goal_ignoring;		/* Currently ignoring monsters */

bool goal_recalling;		/* Currently waiting for recall */

bool stair_less;		/* Use the next "up" staircase */
bool stair_more;		/* Use the next "down" staircase */

s32b auto_began;		/* When this level began */

s32b auto_shock;		/* When last "shocked" */

s16b avoidance = 0;		/* Current danger thresh-hold */


/*
 * Current shopping information
 */
 
s16b goal_shop = -1;		/* Next shop to visit */
s16b goal_ware = -1;		/* Next item to buy there */
s16b goal_item = -1;		/* Next item to sell there */


/*
 * Old values
 */

static s16b old_depth = -1;		/* Previous depth */

static s16b old_chp = -1;	/* Previous hit points */
static s16b old_csp = -1;	/* Previous spell points */


/*
 * Hack -- panic flags
 */

bool panic_death;		/* Panic before Death */

bool panic_stuff;		/* Panic before Junking Stuff */

bool panic_power;		/* Assume monsters get constant spells */


/*
 * Some estimated state variables
 */

s16b my_stat_max[6];	/* Current "maximal" stat values	*/
s16b my_stat_cur[6];	/* Current "natural" stat values	*/
s16b my_stat_use[6];	/* Current "resulting" stat values	*/
s16b my_stat_ind[6];	/* Current "additions" to stat values	*/

s16b my_ac;		/* Base armor		*/
s16b my_to_ac;		/* Plusses to ac	*/
s16b my_to_hit;		/* Plusses to hit	*/
s16b my_to_dam;		/* Plusses to dam	*/

byte my_immune_acid;	/* Immunity to acid		*/
byte my_immune_elec;	/* Immunity to lightning	*/
byte my_immune_fire;	/* Immunity to fire		*/
byte my_immune_cold;	/* Immunity to cold		*/

byte my_resist_acid;	/* Resist acid		*/
byte my_resist_elec;	/* Resist lightning	*/
byte my_resist_fire;	/* Resist fire		*/
byte my_resist_cold;	/* Resist cold		*/
byte my_resist_pois;	/* Resist poison	*/

byte my_resist_conf;	/* Resist confusion	*/
byte my_resist_sound;	/* Resist sound		*/
byte my_resist_lite;	/* Resist light		*/
byte my_resist_dark;	/* Resist darkness	*/
byte my_resist_chaos;	/* Resist chaos		*/
byte my_resist_disen;	/* Resist disenchant	*/
byte my_resist_shard;	/* Resist shards	*/
byte my_resist_nexus;	/* Resist nexus		*/
byte my_resist_blind;	/* Resist blindness	*/
byte my_resist_neth;	/* Resist nether	*/
byte my_resist_fear;	/* Resist fear		*/

byte my_sustain_str;	/* Keep strength	*/
byte my_sustain_int;	/* Keep intelligence	*/
byte my_sustain_wis;	/* Keep wisdom		*/
byte my_sustain_dex;	/* Keep dexterity	*/
byte my_sustain_con;	/* Keep constitution	*/
byte my_sustain_chr;	/* Keep charisma	*/

byte my_aggravate;	/* Aggravate monsters	*/
byte my_teleport;	/* Random teleporting	*/

byte my_ffall;		/* No damage falling	*/
byte my_lite;		/* Permanent light	*/
byte my_free_act;	/* Never paralyzed	*/
byte my_see_inv;		/* Can see invisible	*/
byte my_regenerate;	/* Regenerate hit pts	*/
byte my_hold_life;	/* Resist life draining	*/
byte my_telepathy;	/* Telepathy		*/
byte my_slow_digest;	/* Slower digestion	*/

s16b my_see_infra;	/* Infravision range	*/

s16b my_skill_dis;	/* Skill: Disarming		*/
s16b my_skill_dev;	/* Skill: Magic Devices		*/
s16b my_skill_sav;	/* Skill: Saving throw		*/
s16b my_skill_stl;	/* Skill: Stealth factor	*/
s16b my_skill_srh;	/* Skill: Searching ability	*/
s16b my_skill_fos;	/* Skill: Searching frequency	*/
s16b my_skill_thn;	/* Skill: To hit (normal)	*/
s16b my_skill_thb;	/* Skill: To hit (shooting)	*/
s16b my_skill_tht;	/* Skill: To hit (throwing)	*/
s16b my_skill_dig;	/* Skill: Digging ability	*/

s16b my_num_blow;	/* Number of blows	*/
s16b my_num_fire;	/* Number of shots	*/

s16b my_cur_lite;	/* Radius of lite */
s16b my_cur_view;	/* Radius of view */

s16b my_speed;		/* Approximate speed	*/
s16b my_other;		/* Approximate other	*/

byte my_ammo_tval;	/* Ammo -- "tval"	*/
byte my_ammo_sides;	/* Ammo -- "sides"	*/
s16b my_ammo_power;	/* Shooting multipler	*/
s16b my_ammo_range;	/* Shooting range	*/


/*
 * Potential amounts
 */

s16b amt_fuel;
s16b amt_food;
s16b amt_ident;
s16b amt_recall;
s16b amt_escape;
s16b amt_teleport;

s16b amt_cure_critical;
s16b amt_cure_serious;

s16b amt_detect_trap;
s16b amt_detect_door;

s16b amt_missile;

s16b amt_book[9];

s16b amt_add_stat[6];
s16b amt_fix_stat[6];
s16b amt_fix_exp;
    
s16b amt_enchant_to_a;
s16b amt_enchant_to_d;
s16b amt_enchant_to_h;


/*
 * Potential amounts
 */

s16b num_fuel;
s16b num_food;
s16b num_ident;
s16b num_recall;
s16b num_escape;
s16b num_teleport;

s16b num_cure_critical;
s16b num_cure_serious;

s16b num_missile;

s16b num_book[9];

s16b num_fix_stat[6];

s16b num_fix_exp;
    
s16b num_enchant_to_a;
s16b num_enchant_to_d;
s16b num_enchant_to_h;




/*
 * Some more variables
 */

s16b when_art_lite;	/* When we last activated the Light */

s16b when_call_lite;	/* When we last called light */

s16b when_traps;	/* When we last detected traps */
s16b when_doors;	/* When we last detected doors */

s16b auto_need_enchant_to_a;	/* Need some enchantment */
s16b auto_need_enchant_to_h;	/* Need some enchantment */
s16b auto_need_enchant_to_d;	/* Need some enchantment */


/*
 * Some static variables
 */

static int o_x;			/* Old location */
static int o_y;			/* Old location */

static int g_x;			/* Goal location */
static int g_y;			/* Goal location */

static int count_floor;		/* Number of floor grids */


/*
 * Locate the store doors
 */

static byte *track_shop_x;
static byte *track_shop_y;


/*
 * Track "stairs up"
 */

static s16b track_less_num;
static s16b track_less_size;
static byte *track_less_x;
static byte *track_less_y;


/*
 * Track "stairs down"
 */

static s16b track_more_num;
static s16b track_more_size;
static byte *track_more_x;
static byte *track_more_y;


/*
 * Quick determination of "monster" or "object" or "wank" status.
 */

static bool *auto_is_wank;	/* The character is a "wank" */

static bool *auto_is_take;	/* The character is a "take" */

static bool *auto_is_kill;	/* The character is a "kill" */


/*
 * The terrain list.  This list is used to "track" terrain.
 */

s16b auto_wanks_cnt;

s16b auto_wanks_nxt;

s16b auto_wanks_max;

auto_wank *auto_wanks;


/*
 * The object list.  This list is used to "track" objects.
 */

s16b auto_takes_cnt;

s16b auto_takes_nxt;

s16b auto_takes_max;

auto_take *auto_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

s16b auto_kills_cnt;

s16b auto_kills_nxt;

s16b auto_kills_max;

auto_kill *auto_kills;


/*
 * Hack -- message memory
 */

s16b auto_msg_len;

s16b auto_msg_siz;

char *auto_msg_buf;

s16b auto_msg_num;

s16b auto_msg_max;

s16b *auto_msg_pos;

s16b *auto_msg_use;


/*
 * Hack -- count racial appearances per level
 */

static s16b *auto_race_count;


/*
 * Hack -- count racial kills (for uniques)
 */
 
static s16b *auto_race_death;


/*
 * Hack -- help identify "unique" monster names
 */

static int auto_unique_size;		/* Number of uniques */
static s16b *auto_unique_what;		/* Indexes of uniques */
static cptr *auto_unique_text;		/* Names of uniques */

/*
 * Hack -- help identify "normal" monster names
 */

static int auto_normal_size;		/* Number of normals */
static s16b *auto_normal_what;		/* Indexes of normals */
static cptr *auto_normal_text;		/* Names of normals */



/*
 * Determine if a missile shot from (x1,y1) to (x2,y2) will arrive
 * at the final destination, assuming no monster gets in the way.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 */
static bool borg_projectable(int x1, int y1, int x2, int y2)
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
 * Determine if a missile shot from (x1,y1) to (x2,y2) will arrive
 * at the final destination, without being stopped by monsters.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 */
static bool borg_projectable_pure(int x1, int y1, int x2, int y2)
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

        /* Hack -- Assume termination at "monsters" */
        if (auto_is_kill[(byte)(ag->o_c)]) break;

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
}




/*
 * Attempt to guess what kind of terrain is at the given location.
 *
 * This routine should rarely, if ever, return "zero".
 *
 * Hack -- we use "base level" instead of "allocation levels".
 */
static int borg_guess_wank(int x, int y)
{
    int i, s;
    
    int b_i = 0, b_s = 0;

    auto_grid *ag = grid(x, y);

    /* Find an "acceptable" terrain */
    for (i = 1; i < MAX_F_IDX; i++) {

        feature_type *f_ptr = &f_info[i];

        /* Skip non-objects */
        if (!f_ptr->name) continue;

        /* Verify char */
        if (ag->o_c != f_ptr->z_char) continue;

        /* Verify attr */
        if (ag->o_a != f_ptr->z_attr) continue;
        
        /* Fake score */
        s = 0;
        
        /* Desire "best" possible score */
        if (b_i && (s < b_s)) continue;
        
        /* Track it */
        b_i = i; b_s = s;
    }
    
    /* Result */
    return (b_i);
}


/*
 * Delete an old "wank" record
 */
static void borg_delete_wank(int i)
{
    auto_wank *wank = &auto_wanks[i];

    /* Paranoia -- Already wiped */
    if (!wank->f_idx) return;

    /* Note */
    borg_note(format("# Forgetting a terrain '%s' (%d) at (%d,%d)",
                     (f_name + f_info[wank->f_idx].name), wank->f_idx,
                     wank->x, wank->y));

    /* Kill the object */
    WIPE(wank, auto_wank);

    /* One less object */
    auto_wanks_cnt--;

    /* Wipe goals */
    goal = 0;
}


/*
 * Obtain a new "terrain" index
 */
static int borg_new_wank(int f_idx, int x, int y)
{
    int i, n = -1;

    auto_wank *wank;
    
    auto_grid *ag = grid(x,y);


    /* Look for a "dead" terrain */
    for (i = 0; (n < 0) && (i < auto_wanks_nxt); i++) {

        /* Reuse "dead" objects */
        if (!auto_wanks[i].f_idx) n = i;
    }

    /* Allocate a new object */
    if ((n < 0) && (auto_wanks_nxt < auto_wanks_max)) {

        /* Acquire the entry, advance */
        n = auto_wanks_nxt++;
    }

    /* Hack -- steal an old object */
    if (n < 0) {
    
        /* Note */
        borg_note("# Too many terrains");

        /* Hack -- Pick a random terrain */
        n = rand_int(auto_wanks_nxt);

        /* Erase it */
        WIPE(&auto_wanks[n], auto_wank);

        /* Forget it */
        auto_wanks_cnt--;
    }
    
    
    /* Count new object */
    auto_wanks_cnt++;

    /* Obtain the object */
    wank = &auto_wanks[n];
    
    /* Save the kind */
    wank->f_idx = f_idx;

    /* Save the attr/char */
    wank->o_a = ag->o_a;
    wank->o_c = ag->o_c;
    
    /* Save the location */
    wank->x = x;
    wank->y = y;
    
    /* Hack -- save a fake timestamp */
    wank->when = c_t - 1;

    /* Note */
    borg_note(format("# Creating a terrain '%s' (%d) at (%d,%d)",
                     (f_name + f_info[wank->f_idx].name), wank->f_idx,
                     wank->x, wank->y));

    /* Result */
    return (n);
}



/*
 * Hack -- Note that a grid contains a "different" object
 */
static bool observe_wank_diff(int x, int y)
{
    int i, f_idx;

    auto_wank *wank;
    
    /* Guess the kind */
    f_idx = borg_guess_wank(x, y);

    /* Oops */
    if (!f_idx) return (FALSE);

    /* Make a new object */
    i = borg_new_wank(f_idx, x, y);

    /* Get the terrain */
    wank = &auto_wanks[i];
    
    /* Save the timestamp */
    wank->when = c_t;
    
    /* Okay */
    return (TRUE);
}


/*
 * Attempt to "track" a terrain at the given location
 * Assume that the terrain has not moved more than "d" grids
 * Note that, of course, terrains are never supposed to move.
 */
static bool observe_wank_move(int x, int y, int d)
{
    int i, z, ox, oy;

    auto_grid *ag = grid(x, y);

    feature_type *f_ptr;

    /* Look at the table */
    for (i = 0; i < auto_wanks_nxt; i++) {

        auto_wank *wank = &auto_wanks[i];

        /* Skip dead objects */
        if (!wank->f_idx) continue;

        /* Access the kind */
        f_ptr = &f_info[wank->f_idx];
        
        /* Skip assigned objects */
        if (wank->when == c_t) continue;

        /* Require matching char */
        if (ag->o_c != wank->o_c) continue;

        /* Require matching attr */
        if (ag->o_a != wank->o_a) continue;
        
        /* Extract old location */
        ox = wank->x;
        oy = wank->y;
            
        /* Calculate distance */
        z = distance(ox, oy, x, y);

        /* Possible match */
        if (z > d) continue;

        /* Actual movement (?) */
        if (z) {

            /* Track it */
            wank->x = x;
            wank->y = y;
            
            /* Note */
            borg_note(format("# Tracking an object '%s' (%d) at (%d,%d) from (%d,%d)",
                             (f_name + f_info[wank->f_idx].name), wank->f_idx,
                             wank->x, wank->y, ox, oy));

            /* Clear goals */
            goal = 0;
        }
        
        /* Note when last seen */
        wank->when = c_t;

        /* Done */
        return (TRUE);
    }

    /* Oops */
    return (FALSE);
}




/*
 * Attempt to guess what kind of object is at the given location.
 *
 * This routine should rarely, if ever, return "zero".
 *
 * Hack -- we use "base level" instead of "allocation levels".
 */
static int borg_guess_kind(int x, int y)
{
    int i, s;
    
    int b_i = 0, b_s = 0;

    auto_grid *ag = grid(x, y);

    
    /* Find an "acceptable" object */
    for (i = 1; i < MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Skip non-objects */
        if (!k_ptr->name) continue;


        /* Base score */
        s = 10000;


        /* Hack -- penalize "extremely" out of depth */
        if (k_ptr->level > auto_depth + 50) s = s - 500;

        /* Hack -- penalize "very" out of depth */
        if (k_ptr->level > auto_depth + 15) s = s - 100;

        /* Hack -- penalize "rather" out of depth */
        if (k_ptr->level > auto_depth + 5) s = s - 50;

        /* Hack -- penalize "somewhat" out of depth */
        if (k_ptr->level > auto_depth) s = s - 10;

        /* Hack -- Penalize "depth miss" */
        s = s - ABS(k_ptr->level - auto_depth);


        /* Hack -- Penalize INSTA_ART items */
        if (k_ptr->flags3 & TR3_INSTA_ART) s = s - 1000;


        /* Hack -- Penalize CURSED items */
        if (k_ptr->flags3 & TR3_CURSED) s = s - 5000;
        
        /* Hack -- Penalize BROKEN items */
        if (k_ptr->cost <= 0) s = s - 5000;
        

        /* Verify char */
        if (ag->o_c != k_ptr->x_char) continue;

        /* Flavored objects */
        if (k_ptr->has_flavor) {

            /* Hack -- penalize "flavored" objects */
            s = s - 20;
        }

        /* Normal objects */
        else {
        
            /* Verify attr */
            if (ag->o_a != k_ptr->x_attr) continue;
        }
        

        /* Desire "best" possible score */
        if (b_i && (s < b_s)) continue;
        
        /* Track it */
        b_i = i; b_s = s;
    }
    
    /* Result */
    return (b_i);
}


/*
 * Delete an old "object" record
 */
static void borg_delete_take(int i)
{
    auto_take *take = &auto_takes[i];

    /* Paranoia -- Already wiped */
    if (!take->k_idx) return;

    /* Note */
    borg_note(format("# Forgetting an object '%s' (%d) at (%d,%d)",
                     (k_name + k_info[take->k_idx].name), take->k_idx,
                     take->x, take->y));

    /* Kill the object */
    WIPE(take, auto_take);

    /* One less object */
    auto_takes_cnt--;

    /* Wipe goals */
    goal = 0;
}


/*
 * Obtain a new "object" index
 */
static int borg_new_take(int k_idx, int x, int y)
{
    int i, n = -1;

    auto_take *take;
    
    auto_grid *ag = grid(x,y);


    /* Look for a "dead" object */
    for (i = 0; (n < 0) && (i < auto_takes_nxt); i++) {

        /* Reuse "dead" objects */
        if (!auto_takes[i].k_idx) n = i;
    }

    /* Allocate a new object */
    if ((n < 0) && (auto_takes_nxt < auto_takes_max)) {

        /* Acquire the entry, advance */
        n = auto_takes_nxt++;
    }

    /* Hack -- steal an old object */
    if (n < 0) {
    
        /* Note */
        borg_note("# Too many objects");

        /* Hack -- Pick a random object */
        n = rand_int(auto_takes_nxt);

        /* Erase it */
        WIPE(&auto_takes[n], auto_take);

        /* Forget it */
        auto_takes_cnt--;
    }
    
    
    /* Count new object */
    auto_takes_cnt++;

    /* Obtain the object */
    take = &auto_takes[n];
    
    /* Save the kind */
    take->k_idx = k_idx;

    /* Save the attr/char */
    take->o_a = ag->o_a;
    take->o_c = ag->o_c;
    
    /* Save the location */
    take->x = x;
    take->y = y;
    
    /* Hack -- save a fake timestamp */
    take->when = c_t - 1;

    /* Note */
    borg_note(format("# Creating an object '%s' (%d) at (%d,%d)",
                     (k_name + k_info[take->k_idx].name), take->k_idx,
                     take->x, take->y));

    /* Result */
    return (n);
}



/*
 * Hack -- Note that a grid contains a "different" object
 */
static bool observe_take_diff(int x, int y)
{
    int i, k_idx;

    auto_take *take;
    
    /* Guess the kind */
    k_idx = borg_guess_kind(x, y);

    /* Oops */
    if (!k_idx) return (FALSE);

    /* Make a new object */
    i = borg_new_take(k_idx, x, y);

    /* Get the object */
    take = &auto_takes[i];
    
    /* Save the timestamp */
    take->when = c_t;
    
    /* Okay */
    return (TRUE);
}


/*
 * Attempt to "track" an object at the given location
 * Assume that the object has not moved more than "d" grids
 * Note that, of course, objects are never supposed to move,
 * but we may want to take account of "falling" missiles later.
 */
static bool observe_take_move(int x, int y, int d)
{
    int i, z, ox, oy;

    auto_grid *ag = grid(x, y);

    inven_kind *k_ptr;

    /* Look at the table */
    for (i = 0; i < auto_takes_nxt; i++) {

        auto_take *take = &auto_takes[i];

        /* Skip dead objects */
        if (!take->k_idx) continue;

        /* Access the kind */
        k_ptr = &k_info[take->k_idx];
        
        /* Skip assigned objects */
        if (take->when == c_t) continue;

        /* Require matching char */
        if (ag->o_c != take->o_c) continue;

        /* Require matching attr */
        if (!k_ptr->has_flavor) {
        
            /* Require matching attr */
            if (ag->o_a != take->o_a) continue;
        }
        
        /* Extract old location */
        ox = take->x;
        oy = take->y;
            
        /* Calculate distance */
        z = distance(ox, oy, x, y);

        /* Possible match */
        if (z > d) continue;

        /* Actual movement (?) */
        if (z) {

            /* Track it */
            take->x = x;
            take->y = y;
            
            /* Note */
            borg_note(format("# Tracking an object '%s' (%d) at (%d,%d) from (%d,%d)",
                             (k_name + k_info[take->k_idx].name), take->k_idx,
                             take->x, take->y, ox, oy));

            /* Clear goals */
            goal = 0;
        }
        
        /* Note when last seen */
        take->when = c_t;

        /* Done */
        return (TRUE);
    }

    /* Oops */
    return (FALSE);
}




/*
 * Attempt to guess what type of monster is at the given location.
 *
 * If we are unable to think of any monster that could be at the
 * given location, we will assume the monster is a player ghost.
 * This is a total hack, but may prevent crashes.
 *
 * The guess can be improved by the judicious use of a specialized
 * "attr/char" mapping, especially for unique monsters.  Currently,
 * the Borg does not stoop to such redefinitions.
 *
 * We will probably fail to identify "trapper" and "lurker" monsters,
 * since they look like whatever they are standing on.  Now we will
 * probably just assume they are player ghosts.  XXX XXX XXX
 *
 * Note that "town" monsters may only appear in town, and in "town",
 * only "town" monsters may appear, unless we summon or polymorph
 * a monster while in town, which should never happen.
 *
 * To guess which monster is at the given location, we consider every
 * possible race, keeping the race (if any) with the best "score".
 *
 * Certain monster races are "impossible", including town monsters
 * in the dungeon, dungeon monsters in the town, unique monsters
 * known to be dead, monsters more than 50 levels out of depth,
 * and monsters with an impossible char, or an impossible attr.
 *
 * Certain aspects of a monster race are penalized, including extreme
 * out of depth, minor out of depth, clear/multihued attrs.
 *
 * Certain aspects of a monster race are rewarded, including monsters
 * that appear in groups, monsters that reproduce, monsters that have
 * been seen on this level a lot.
 *
 * The actual rewards and penalties probably need some tweaking.
 *
 * Hack -- try not to choose "unique" monsters, or we will flee a lot.
 */
static int borg_guess_race(int x, int y)
{
    int i, s, n;
    
    int b_i = 0, b_s = 0;

    auto_grid *ag = grid(x, y);
    

    /* Find an "acceptable" monster */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;


        /* Base score */
        s = 10000;


        /* Check uniques */
        if (r_ptr->flags1 & RF1_UNIQUE) {
        
            /* Hack -- Dead uniques stay dead */
            if (auto_race_death[i] > 0) continue;
            
            /* Hack -- handle "panic_power" */
            s = s + (panic_power ? 5 : -10);
        }


        /* Town must have town monsters */
        if (r_ptr->level && !auto_depth) continue;
        
        /* Town monsters must appear in town */
        if (!r_ptr->level && auto_depth) continue;

        /* Hack -- penalize "extremely" out of depth */
        if (r_ptr->level > auto_depth + 50) continue;

        /* Hack -- penalize "very" out of depth */
        if (r_ptr->level > auto_depth + 15) s = s - 100;

        /* Hack -- penalize "rather" out of depth */
        if (r_ptr->level > auto_depth + 5) s = s - 50;

        /* Hack -- penalize "somewhat" out of depth */
        if (r_ptr->level > auto_depth) s = s - 10;

        /* Penalize "depth miss" */
        s = s - ABS(r_ptr->level - auto_depth);


        /* Verify char */
        if (ag->o_c != r_ptr->l_char) continue;

        /* Clear or multi-hued monsters */
        if (r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_ATTR_CLEAR)) {

            /* Hack -- penalize "weird" monsters */
            s = s - 20;
        }

        /* Normal monsters */
        else {
        
            /* Verify attr */
            if (ag->o_a != r_ptr->l_attr) continue;
        }


        /* Hack -- Reward group monsters */
        if (r_ptr->flags1 & (RF1_FRIEND | RF1_FRIENDS)) s = s + 5;
        
        /* Hack -- Reward multiplying monsters */
        if (r_ptr->flags2 & RF2_MULTIPLY) s = s + 10;


        /* Count occurances */
        n = auto_race_count[i];

        /* Mega-Hack -- Reward occurances */
        s = s + (n / 100) + (((n < 100) ? n : 100) / 10) + ((n < 10) ? n : 10);
        

        /* Desire "best" possible score */
        if (b_i && (s < b_s)) continue;
        
        /* Track it */
        b_i = i; b_s = s;
    }

    /* Success */
    if (b_i) return (b_i);


    /* Message */
    borg_note("# Assuming player ghost (?)");

    /* Assume player ghost */
    return (MAX_R_IDX - 1);
}


/*
 * Attempt to convert a monster name into a race index
 *
 * First we check for all possible "unique" monsters, including
 * ones we have killed, and even if the monster name is "prefixed"
 * (as in "The Tarrasque" and "The Lernean Hydra").  Since we use
 * a fast binary search, this is acceptable paranoia.
 *
 * Otherwise, we attempt to match monsters named "The xxx" to the
 * most "likely" non-unique monster, taking into account various
 * heuristic measures.
 *
 * If we cannot find a "possible" match, we assume the monster is
 * a plater ghost, which is a hack, but may prevent crashes.
 *
 * Note the use of a binary search which is guaranteed to either
 * find the *last* matching entry, or fail, and point to an entry
 * which would follow the matching entry if there was one, unless
 * the matching entry would follow all the existing entries, in
 * which case it will find the final entry in the list.  Thus, we
 * can search *backwards* from the result of the search, and know
 * that we will access all of the matching entries.
 */
static int borg_guess_race_name(cptr who)
{
    int k, m, n;
    
    int i, b_i = 0;
    int s, b_s = 0;
    
    monster_race *r_ptr;


    /* Start the search */
    m = 0; n = auto_unique_size;

    /* Binary search */
    while (m < n - 1) {

        /* Pick a "middle" entry */
        i = (m + n) / 2;

        /* Search to the right (or here) */
        if (strcmp(auto_unique_text[i], who) <= 0) {
             m = i;
        }

        /* Search to the left */
        else {
             n = i;
        }
    }

    /* Check for equality */
    if (streq(who, auto_unique_text[m])) {

        /* Use this monster */
        return (auto_unique_what[m]);
    }


    /* Assume player ghost */
    if (!prefix(who, "The ")) {
    
        /* Abort */
        borg_note("# Assuming a player ghost (?)");
        
        /* Oops */
        return (MAX_R_IDX-1);
    }

    /* Skip the prefix */
    who += 4;


    /* Start the search */
    m = 0; n = auto_normal_size;

    /* Binary search */
    while (m < n - 1) {

        /* Pick a "middle" entry */
        i = (m + n) / 2;

        /* Search to the right (or here) */
        if (strcmp(auto_normal_text[i], who) <= 0) {
             m = i;
        }

        /* Search to the left */
        else {
             n = i;
        }
    }

    /* Scan possible "normal" monsters */
    for (k = m; k >= 0; k--) {

        /* Stop when done */
        if (!streq(who, auto_normal_text[k])) break;

        /* Extract the monster */
        i = auto_normal_what[k];
        
        /* Access the monster */
        r_ptr = &r_info[i];

        /* Basic score */
        s = 1000;
        
        /* Penalize "depth miss" */
        s = s - ABS(r_ptr->level - auto_depth);

        /* Track best */
        if (b_i && (s < b_s)) continue;
        
        /* Track it */
        b_i = i; b_s = s;
    }

    /* Success */
    if (b_i) return (b_i);
    

    /* Oops */
    borg_note("# Assuming a player ghost (?)");

    /* Oops */
    return (MAX_R_IDX-1);
}


/*
 * Hack -- Update a "new" monster
 */
static void borg_update_kill_new(int i)
{
    auto_kill *kill = &auto_kills[i];

    monster_race *r_ptr = &r_info[kill->r_idx];
    

    /* Guess at the monster speed */
    kill->speed = (r_ptr->speed);

    /* Hack -- assume optimal racial variety */
    if (!(r_ptr->flags1 & RF1_UNIQUE)) {

        /* Hack -- Assume full speed bonus */
        kill->speed += (extract_energy[kill->speed] / 10);
    }


    /* Extract max hitpoints */
    kill->power = r_ptr->hdice * r_ptr->hside;
}


/*
 * Hack -- Update a "old" monster
 */
static void borg_update_kill_old(int i)
{
    int p, t, e, m;

    auto_kill *kill = &auto_kills[i];


    /* Player energy per game turn */
    p = extract_energy[auto_speed];

    /* Time passes after next player move (round up) */
    for (t = 100 / p; t * p < 100; t++) ;


    /* Extract monster energy per game turn */
    e = extract_energy[kill->speed];

    /* Monster gets some moves too (round up) */
    for (m = 1; t * e > 100 * m; m++) ;

    /* Save the (estimated) monster moves */
    kill->moves = m;
}


/*
 * Delete an old "monster" record
 */
static void borg_delete_kill(int i)
{
    auto_kill *kill = &auto_kills[i];

    /* Paranoia -- Already wiped */
    if (!kill->r_idx) return;

    /* Note */
    borg_note(format("# Forgetting a monster '%s' (%d) at (%d,%d)",
                     (r_name + r_info[kill->r_idx].name), kill->r_idx,
                     kill->x, kill->y));

    /* Kill the object */
    WIPE(kill, auto_kill);

    /* One less object */
    auto_kills_cnt--;

    /* Recalculate danger */
    auto_danger_wipe = TRUE;

    /* Wipe goals */
    goal = 0;
}



/*
 * Obtain a new "monster" index, and initialize that monster
 */
static int borg_new_kill(int r_idx, int x, int y)
{
    int i, n = -1;

    auto_kill *kill;

    auto_grid *ag = grid(x,y);
        

    /* Look for a "dead" monster */
    for (i = 0; (n < 0) && (i < auto_kills_nxt); i++) {

        /* Skip real entries */
        if (!auto_kills[i].r_idx) n = i;
    }

    /* Allocate a new monster */
    if ((n < 0) && (auto_kills_nxt < auto_kills_max)) {

        /* Acquire the entry, advance */
        n = auto_kills_nxt++;
    }

    /* Hack -- steal an old monster */
    if (n < 0) {
    
        /* Note */
        borg_note("# Too many monsters");

        /* Hack -- Pick a random monster */
        n = rand_int(auto_kills_nxt);

        /* Wipe it */
        WIPE(&auto_takes[n], auto_kill);
        
        /* Forget it */
        auto_kills_cnt--;
    }
    

    /* Count the monsters */
    auto_kills_cnt++;
    
    /* Access the monster */
    kill = &auto_kills[n];
    
    /* Save the race */
    kill->r_idx = r_idx;

    /* Save the attr/char */
    kill->o_a = ag->o_a;
    kill->o_c = ag->o_c;
    
    /* Location */
    kill->ox = kill->x = x;
    kill->oy = kill->y = y;

    /* Hack -- save a fake timestamp */
    kill->when = c_t - 1;

    /* Update the monster */
    borg_update_kill_new(n);

    /* Update the monster */
    borg_update_kill_old(n);

    /* Recalculate danger */
    auto_danger_wipe = TRUE;

    /* Clear goals */
    goal = 0;

    /* Note */
    borg_note(format("# Creating a monster '%s' (%d) at (%d,%d)",
                     (r_name + r_info[kill->r_idx].name), kill->r_idx,
                     kill->x, kill->y));

    /* Return the monster */
    return (n);
}



/*
 * Hack -- Note that a grid contains a "different" monster
 */
static bool observe_kill_diff(int x, int y)
{
    int i, r_idx;

    auto_kill *kill;
    
    /* Guess the race */
    r_idx = borg_guess_race(x, y);

    /* Oops */
    /* if (!r_idx) return (FALSE); */

    /* Create a new monster */
    i = borg_new_kill(r_idx, x, y);

    /* Get the object */
    kill = &auto_kills[i];
    
    /* Save the timestamp */
    kill->when = c_t;

    /* Done */
    return (TRUE);
}


/*
 * Attempt to "track" a monster at the given location
 * Assume that the monster moved at most 'd' grids
 */
static bool observe_kill_move(int x, int y, int d)
{
    int i, z, ox, oy;

    auto_kill *kill;

    monster_race *r_ptr;

    auto_grid *ag = grid(x, y);


    /* Look at the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Skip assigned monsters */
        if (kill->when == c_t) continue;

        /* Access the monster race */
        r_ptr = &r_info[kill->r_idx];
        
        /* Require matching char */
        if (ag->o_c != kill->o_c) continue;

        /* Require matching attr (usually) */
        if (!(r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_ATTR_CLEAR))) {

            /* Require matching attr */
            if (ag->o_a != kill->o_a) continue;
        }

        /* Old location */
        ox = kill->x;
        oy = kill->y;
        
        /* Calculate distance */
        z = distance(ox, oy, x, y);

        /* Possible match */
        if (z > d) continue;
        
        /* Hack -- Restrict distance to realistic values */
        if (z > kill->moves) continue;

        /* Actual movement */
        if (z) {

            /* Save the old Location */
            kill->ox = kill->x;
            kill->oy = kill->y;

            /* Save the Location */
            kill->x = x;
            kill->y = y;
            
            /* Note */
            borg_note(format("# Tracking a monster '%s' (%d) at (%d,%d) from (%d,%d)",
                             (r_name + r_ptr->name), kill->r_idx,
                             kill->x, kill->y, ox, oy));

            /* Recalculate danger */
            auto_danger_wipe = TRUE;

            /* Clear goals */
            goal = 0;
        }
        
        /* Note when last seen */
        kill->when = c_t;

        /* Update the monster */
        borg_update_kill_old(i);

        /* Done */
        return (TRUE);
    }

    /* Oops */
    return (FALSE);
}


/*
 * React to the touch (or death) of a "nearby" monster
 *
 * XXX XXX XXX Hack -- To prevent fatal situations, every time we think
 * there may be a monster nearby, we look for a nearby object which could
 * be the indicated monster, and convert it into that monster.
 *
 * XXX XXX XXX When surrounded by multiple monsters of the same type,
 * we will ascribe most messages to one of those monsters, and ignore
 * the existance of all the other similar monsters.
 */
static bool borg_nearby_kill(cptr who, bool dead)
{
    int i, d, r_idx;

    int x = o_x;
    int y = o_y;

    int b_i, b_d;
    
    auto_grid *ag;

    auto_take *take;
    auto_kill *kill;

    monster_race *r_ptr;


    /* Handle invisible monsters */
    if (streq(who, "It") ||
        streq(who, "Someone") ||
        streq(who, "Something")) {

        /* Note */
        borg_note("# Invisible monster nearby");

        /* Hack -- flee this level */
        if (rand_int(100) < 2) goal_fleeing = TRUE;
        
        /* Done */
        return (TRUE);
    }


    /* Guess the monster race */
    r_idx = borg_guess_race_name(who);

    /* Note */
    borg_note(format("# There is a%s monster '%s' (%d) near (%d,%d)",
                     (dead ? " dead" : ""),
                     (r_name + r_info[r_idx].name), r_idx,
                     x, y));

    /* Hack -- ignore "weird" monsters */
    /* if (!r_idx) return (FALSE); */


    /* Hack -- count racial appearances */
    if (auto_race_count[r_idx] < MAX_SHORT) auto_race_count[r_idx]++;
    
    /* Hack -- count racial deaths */
    if (dead && (auto_race_death[r_idx] < MAX_SHORT)) auto_race_death[r_idx]++;
    
    /* Access the monster race */
    r_ptr = &r_info[r_idx];


    /*** Hack -- Find a similar object ***/

    /* Nothing yet */
    b_i = -1; b_d = 999;

    /* Scan the objects */
    for (i = 0; i < auto_takes_nxt; i++) {

        take = &auto_takes[i];

        /* Skip "dead" objects */
        if (!take->k_idx) continue;

        /* Access the grid */
        ag = grid(take->x, take->y);

        /* Verify char */
        if (take->o_c != r_ptr->l_char) continue;

        /* Verify attr (unless clear or multi-hued) */
        if (!(r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_ATTR_CLEAR))) {

            /* Verify attr */
            if (take->o_a != r_ptr->l_attr) continue;
        }

        /* Distance away */
        d = distance(take->x, take->y, x, y);

        /* Hack -- Skip "wrong" objects */
        if (d > 20) continue;

        /* Track closest one */
        if (d > b_d) continue;
        
        /* Track it */
        b_i = i; b_d = d;
    }
    
    /* Found one */
    if (b_i >= 0) {
    
        take = &auto_takes[b_i];

        /* Note */
        borg_note(format("# Converting an object '%s' (%d) at (%d,%d)",
                         (k_name + k_info[take->k_idx].name), take->k_idx,
                         take->x, take->y));

        /* Save location */
        x = take->x;
        y = take->y;

        /* Delete the object */
        borg_delete_take(b_i);

        /* Make a new monster */
        b_i = borg_new_kill(r_idx, x, y);

        /* Kill the monster */
        if (dead) borg_delete_kill(b_i);
        
        /* Done */    
        return (TRUE);
    }


    /*** Hack -- Find a similar monster ***/
    
    /* Nothing yet */
    b_i = -1; b_d = 999;

    /* Scan the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        kill = &auto_kills[i];

        /* Skip "dead" monsters */
        if (!kill->r_idx) continue;

        /* Skip "matching" monsters */
        if (kill->r_idx == r_idx) continue;

        /* Access the grid */
        ag = grid(kill->x, kill->y);

        /* Verify char */
        if (kill->o_c != r_ptr->l_char) continue;

        /* Verify attr (unless clear or multi-hued) */
        if (!(r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_ATTR_CLEAR))) {

            /* Verify attr */
            if (kill->o_a != r_ptr->l_attr) continue;
        }

        /* Distance away */
        d = distance(kill->x, kill->y, x, y);

        /* Hack -- Skip "wrong" monsters */
        if (d > 20) continue;

        /* Track closest one */
        if (d > b_d) continue;
        
        /* Track it */
        b_i = i; b_d = d;
    }
    
    /* Found one */
    if (b_i >= 0) {
    
        kill = &auto_kills[b_i];

        /* Note */
        borg_note(format("# Converting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Change the race */
        kill->r_idx = r_idx;

        /* Update the monster */
        borg_update_kill_new(b_i);
        
        /* Update the monster */
        borg_update_kill_old(b_i);
        
        /* Kill the monster */
        if (dead) borg_delete_kill(b_i);

        /* Recalculate danger */
        auto_danger_wipe = TRUE;

        /* Clear goals */
        goal = 0;

        /* Success */
        return (TRUE);
    }


    /*** Hack -- Find an existing monster ***/
    
    /* Nothing yet */
    b_i = -1; b_d = 999;

    /* Scan the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        kill = &auto_kills[i];

        /* Skip "dead" monsters */
        if (!kill->r_idx) continue;

        /* Skip "different" monsters */
        if (kill->r_idx != r_idx) continue;

        /* Distance away */
        d = distance(kill->x, kill->y, x, y);

        /* Skip "silly" entries */
        if (d > 20) continue;

        /* Track closest one */
        if (d > b_d) continue;
        
        /* Track it */
        b_i = i; b_d = d;
    }
    
    /* Found one */
    if (b_i >= 0) {

        kill = &auto_kills[b_i];

        /* Note */
        borg_note(format("# Matched a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Kill the monster */
        if (dead) borg_delete_kill(b_i);

        /* Done */
        return (TRUE);
    }


    /*** Oops ***/
    
    /* Uh oh */
    borg_note(format("# Ignoring a%s monster '%s' (%d) near (%d,%d)",
                     (dead ? " dead" : ""),
                     (r_name + r_info[r_idx].name), r_idx,
                     x, y));

    /* Oops */
    return (FALSE);
}



/*
 * React to the touch (or death) of an "obvious" monster
 *
 * Since confusion may result in accidental attacks, if we touch
 * a monster when confused, we simply pretend that it touched us.
 */
static bool borg_verify_kill(cptr who, bool dead)
{
    int i, r_idx;

    int x = g_x;
    int y = g_y;

    auto_take *take;
    auto_kill *kill;

    monster_race *r_ptr;
    

    /* Handle confusion */
    if (do_confused) return (borg_nearby_kill(who, dead));


    /* Handle invisible monsters */
    if (streq(who, "It") ||
        streq(who, "Someone") ||
        streq(who, "Something")) {

        /* Note */
        borg_note("# Invisible monster nearby");

        /* Hack -- flee this level */
        if (rand_int(100) < 2) goal_fleeing = TRUE;

        /* Done */
        return (TRUE);
    }


    /* Determine the monster race */
    r_idx = borg_guess_race_name(who);

    /* Note */
    borg_note(format("# There is a%s '%s' (%d) at (%d,%d)",
                     (dead ? " dead" : ""),
                     (r_name + r_info[r_idx].name),
                     r_idx, x, y));

    /* Hack -- ignore "weird" monsters */
    /* if (!r_idx) return (FALSE); */

    
    /* Hack -- count racial appearances */
    if (auto_race_count[r_idx] < MAX_SHORT) auto_race_count[r_idx]++;
    
    /* Hack -- count racial deaths */
    if (dead && (auto_race_death[r_idx] < MAX_SHORT)) auto_race_death[r_idx]++;

    /* Get the monster race */
    r_ptr = &r_info[r_idx];
    

    /*** Hack -- Find an existing monster ***/
    
    /* Scan the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        auto_kill *kill = &auto_kills[i];

        /* Skip "dead" monsters */
        if (!kill->r_idx) continue;

        /* Skip "wrong" monsters */
        if ((kill->x != x) || (kill->y != y)) continue;

        /* Skip "wrong" monsters */
        if (kill->r_idx != r_idx) continue;
        
        /* Note */
        borg_note(format("# Matched a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Kill the monster */
        if (dead) borg_delete_kill(i);

        /* Hack -- We already knew that */
        return (TRUE);
    }


    /*** Hack -- Find a similar monster ***/
    
    /* Scan the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        kill = &auto_kills[i];

        /* Skip "dead" monsters */
        if (!kill->r_idx) continue;

        /* Skip "wrong" monsters */
        if ((kill->x != x) || (kill->y != y)) continue;

        /* Skip "matching" monsters */
        if (kill->r_idx == r_idx) continue;

        /* Note */
        borg_note(format("# Converting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Change the race */
        kill->r_idx = r_idx;

        /* Update the monster */
        borg_update_kill_new(i);
        
        /* Update the monster */
        borg_update_kill_old(i);
        
        /* Kill the monster */
        if (dead) borg_delete_kill(i);

        /* Success */
        return (TRUE);
    }


    /*** Hack -- Find a similar object ***/
    
    /* Scan the objects */
    for (i = 0; i < auto_takes_nxt; i++) {

        take = &auto_takes[i];

        /* Skip "dead" objects */
        if (!take->k_idx) continue;

        /* Skip "wrong" objects */
        if ((take->x != x) || (take->y != y)) continue;

        /* Verify char */
        if (take->o_c != r_ptr->l_char) continue;

        /* Verify attr (unless clear or multi-hued) */
        if (!(r_ptr->flags1 & (RF1_ATTR_MULTI | RF1_ATTR_CLEAR))) {

            /* Verify attr */
            if (take->o_a != r_ptr->l_attr) continue;
        }

        /* Note */
        borg_note(format("# Converting an object '%s' (%d) at (%d,%d)",
                         (k_name + k_info[take->k_idx].name), take->k_idx,
                         take->x, take->y));

        /* Kill the object */
        borg_delete_take(i);

        /* Make a new monster */
        i = borg_new_kill(r_idx, x, y);

        /* Kill the monster */
        if (dead) borg_delete_kill(i);

        /* Done */
        return (TRUE);
    }


    /*** Oops ***/
    
    /* Notice invisible monsters (?) */
    borg_note(format("# Converting the '<floor>' at (%d,%d)",
                     x, y));

    /* Make a new monster */
    i = borg_new_kill(r_idx, x, y);

    /* Kill the monster */
    if (dead) borg_delete_kill(i);
    
    /* Done */
    return (TRUE);
}





/*
 * Look at the screen and update the borg
 *
 * Uses the "panel" info (w_x, w_y) obtained earlier
 *
 * Note that all the "important" messages that occured after our last
 * action have been "queued" in a usable form.  We must attempt to use
 * these messages to update our knowledge about the world, keeping in
 * mind that the world may have changed in drastic ways.
 *
 * Note that the "c_t" variable corresponds *roughly* to player turns,
 * except that resting and "repeated" commands count as a single turn,
 * and "free" moves (including "illegal" moves, such as attempted moves
 * into walls, or tunneling into monsters) are counted as turns.
 *
 * Also note that "c_t" is not incremented until the Borg is about to
 * do something, so nothing ever has a time-stamp of the current time.
 *
 * XXX XXX XXX The basic problem with timestamping the monsters
 * and objects is that we often get a message about a monster, and so
 * we want to timestamp it, but then we cannot use the timestamp to
 * indicate that the monster has not been "checked" yet.  Perhaps
 * we need to do something like give each monster a "moved" flag,
 * and clear all the flags to FALSE each turn before tracking. (?)
 */
void borg_update(void)
{
    int i, x, y, dx, dy;

    cptr msg;

    auto_grid *ag;
    auto_grid *pg;

    byte old_attr[SCREEN_HGT][SCREEN_WID];
    char old_char[SCREEN_HGT][SCREEN_WID];


    /*** Handle messages ***/

    /* Process messages */
    for (i = 0; i < auto_msg_num; i++) {

        /* Skip parsed messages */
        if (auto_msg_use[i]) continue;
        
        /* Get the message */
        msg = auto_msg_buf + auto_msg_pos[i];
        
        /* Handle "You hit xxx." */
        if (prefix(msg, "HIT:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_verify_kill(msg+4, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "You miss xxx." */
        else if (prefix(msg, "MISS:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_verify_kill(msg+5, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "You have killed xxx." */
        else if (prefix(msg, "KILL:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_verify_kill(msg+5, TRUE)) auto_msg_use[i] = 1;
        }

        /* Handle "xxx dies." */
        else if (prefix(msg, "DIED:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+5, TRUE)) auto_msg_use[i] = 1;
        }
    
        /* Handle "xxx hits you." */
        else if (prefix(msg, "HIT_BY:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+7, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "xxx misses you." */
        else if (prefix(msg, "MISS_BY:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+8, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE:SLEEP:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+12, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "awake" */
        else if (prefix(msg, "STATE:AWAKE:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+12, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE:FEAR:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+11, FALSE)) auto_msg_use[i] = 1;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE:BOLD:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+11, FALSE)) auto_msg_use[i] = 1;
        }

        /* Hack -- Handle "spell" */
        else if (prefix(msg, "SPELL:")) {
            borg_note(format("# %s (1)", msg));
            if (borg_nearby_kill(msg+6, FALSE)) auto_msg_use[i] = 1;
        }
    }


    /*** Handle new levels ***/

    /* Hack -- note new levels */
    if (old_depth != auto_depth) {

        /* Hack -- Restart the clock */
        c_t = 1000;

        /* When level was begun */
        auto_began = c_t;

        /* When last shocked */
        auto_shock = c_t;

        /* New danger thresh-hold */
        avoidance = auto_chp;

        /* Recalculate danger */
        auto_danger_wipe = TRUE;

        /* Mega-Hack -- Clear "activate light" stamp */
        when_art_lite = 0;

        /* Mega-Hack -- Clear "call light" stamp */
        when_call_lite = 0;

        /* Mega-Hack -- Clear "detect traps" stamp */
        when_traps = 0;

        /* Mega-Hack -- Clear "detect doors" stamp */
        when_doors = 0;

        /* Hack -- Clear "shop visit" stamps */
        for (i = 0; i < 8; i++) auto_shops[i].when = 0;

        /* No goal yet */
        goal = 0;

        /* Hack -- Clear "shop" goals */
        goal_shop = goal_ware = goal_item = -1;

        /* Forget "feeling" */
        auto_feeling = 0;

        /* No floors seen yet */
        count_floor = 0;

        /* Do not use any stairs */
        stair_less = stair_more = FALSE;

        /* Hack -- cannot rise past town */
        if (!auto_depth) goal_rising = FALSE;

        /* Assume not fleeing the level */
        goal_fleeing = FALSE;

        /* Assume not ignoring monsters */
        goal_ignoring = FALSE;

        /* No known stairs */
        track_less_num = 0;
        track_more_num = 0;

        /* Forget old terrains */
        C_WIPE(auto_wanks, auto_wanks_max, auto_wank);

        /* No objects here */
        auto_wanks_cnt = 0;
        auto_wanks_nxt = 0;

        /* Forget old objects */
        C_WIPE(auto_takes, auto_takes_max, auto_take);

        /* No objects here */
        auto_takes_cnt = 0;
        auto_takes_nxt = 0;

        /* Forget old monsters */
        C_WIPE(auto_kills, auto_kills_max, auto_kill);

        /* No monsters here */
        auto_kills_cnt = 0;
        auto_kills_nxt = 0;

        /* Hack -- Forget race counters */
        C_WIPE(auto_race_count, MAX_R_IDX, s16b);
        
        /* Forget the map */
        borg_forget_map();

        /* Hack -- forget locations */
        o_x = o_y = g_x = g_y = 0;

        /* Save new depth */
        old_depth = auto_depth;
    }


    /*** Update the map information ***/
    
    /* Memorize the "current" (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Save the current knowledge */
            old_attr[dy][dx] = ag->o_a;
            old_char[dy][dx] = ag->o_c;
        }
    }


    /* Analyze the map */
    borg_update_map();

    /* Paranoia -- make sure we exist */
    if (do_image) {
        borg_oops("hallucination");
        return;
    }

    /* Hack -- fake old location */
    if (!o_x && !o_y) { o_x = c_x; o_y = c_y; }

    /* Hack -- fake goal location */
    if (!g_x && !g_y) { g_x = c_x; g_y = c_y; }


#ifdef BORG_ROOMS

    /* Make "fake" rooms for all viewable unknown grids */
    for (n = 0; n < auto_view_n; n++) {

        /* Access the location */
        y = auto_view_y[n];
        x = auto_view_x[n];

        /* Access the grid */
        ag = grid(x,y);

        /* Skip walls */
        if (ag->info & BORG_WALL) continue;

        /* Skip "known" grids */
        if (ag->o_c != ' ') continue;

        /* Make "fake" rooms as needed */
        if (!ag->room) {

            auto_room *ar;

            /* Get a new room */
            ar = borg_free_room();

            /* Initialize the room */
            ar->x1 = ar->x2 = x;
            ar->y1 = ar->y2 = y;

            /* Save the room */
            ag->room = ar->self;
        }
    }

#endif


    /*** Analyze the current map panel ***/

    /* Hack -- no monster/object grids yet */
    auto_temp_n = 0;
    
    /* Analyze the current (66x22 grid) map sector */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        for (dx = 0; dx < SCREEN_WID; dx++) {

            byte oa, na;
            char oc, nc;

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Get the auto_grid */
            ag = grid(x, y);

            /* Skip "dark" grids */
            if (!(ag->info & BORG_OKAY)) continue;

            /* Old contents */
            oa = old_attr[dy][dx];
            oc = old_char[dy][dx];

            /* New contents */
            na = ag->o_a;
            nc = ag->o_c;


            /* Hack -- Collect objects/monsters */
            if (auto_is_kill[(byte)(nc)] ||
                auto_is_take[(byte)(nc)] ||
                auto_is_wank[(byte)(nc)]) {

                /* Careful -- Remember it */
                auto_temp_x[auto_temp_n] = x;
                auto_temp_y[auto_temp_n] = y;
                auto_temp_n++;
            }


            /* Skip unchanged grids */
            if ((oa == na) && (oc == nc)) continue;


            /* Hack -- Notice first "knowledge" */
            if (oc == ' ') auto_shock = c_t;


            /* Lost old "floor" grids */
            if (oc == '.') count_floor--;

            /* Found new "floor" grids */		
            if (nc == '.') count_floor++;


            /* Track the "up" stairs */
            if (nc == '<') {

                /* Check for an existing 'less' */
                for (i = 0; i < track_less_num; i++) {

                    /* We already knew about that one */
                    if ((track_less_x[i] == x) && (track_less_y[i] == y)) break;
                }

                /* Save new information */
                if ((i == track_less_num) && (i < track_less_size)) {
                    track_less_x[i] = x;
                    track_less_y[i] = y;
                    track_less_num++;
                }
            }

            /* Track the "down" stairs */
            if (nc == '>') {

                /* Check for an existing 'more' */
                for (i = 0; i < track_more_num; i++) {

                    /* We already knew about that one */
                    if ((track_more_x[i] == x) && (track_more_y[i] == y)) break;
                }

                /* Save new information */
                if ((i == track_more_num) && (i < track_more_size)) {
                    track_more_x[i] = x;
                    track_more_y[i] = y;
                    track_more_num++;
                }
            }

            /* Track the shops */
            if ((nc >= '1') && (nc <= '8')) {

                /* Obtain the shop index */
                i = (nc - '1');

                /* Save new information */
                track_shop_x[i] = x;
                track_shop_y[i] = y;
            }


#ifdef BORG_ROOMS

            /* Clear all "broken" rooms */
            if (ag->info & BORG_WALL) {

                /* Clear all rooms containing walls */
                if (ag->room) {

                    /* Clear all rooms containing walls */
                    if (borg_clear_room(x, y)) goal = 0;
                }
            }

            /* Create "fake" rooms as needed */
            else {

                /* Mega-Hack -- super-fake rooms */
                if (!ag->room) {

                    /* Acquire a new room */
                    auto_room *ar = borg_free_room();

                    /* Initialize the room */
                    ar->x1 = ar->x2 = x;
                    ar->y1 = ar->y2 = y;

                    /* Save the room */
                    ag->room = ar->self;
                }	
            }

#endif

            /* XXX XXX XXX Can we ignore this? */

            /* Viewable changes kill goals */
            /* if (ag->info & BORG_VIEW) goal = 0; */
        }
    }	
    

    /*** Track objects and monsters ***/

    /* Pass 0 -- stationary terrain */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];
        
        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track stationary monsters */
        if ((auto_is_wank[(byte)(ag->o_c)]) &&
            (observe_wank_move(x, y, 0))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 1 -- stationary monsters */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];
        
        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track stationary monsters */
        if ((auto_is_kill[(byte)(ag->o_c)]) &&
            (observe_kill_move(x, y, 0))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 2 -- stationary objects */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];
        
        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track stationary objects */
        if ((auto_is_take[(byte)(ag->o_c)]) &&
            (observe_take_move(x, y, 0))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 3a -- moving monsters (distance 1) */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];
        
        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track moving monsters */
        if ((auto_is_kill[(byte)(ag->o_c)]) &&
            (observe_kill_move(x, y, 1))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 3b -- moving monsters (distance 2) */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track moving monsters */
        if ((auto_is_kill[(byte)(ag->o_c)]) &&
            (observe_kill_move(x, y, 2))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 3c -- moving monsters (distance 3) */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track moving monsters */
        if ((auto_is_kill[(byte)(ag->o_c)]) &&
            (observe_kill_move(x, y, 3))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 4 -- new objects */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track new objects */
        if ((auto_is_take[(byte)(ag->o_c)]) &&
            (observe_take_diff(x, y))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 5 -- new monsters */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track new monsters */
        if ((auto_is_kill[(byte)(ag->o_c)]) &&
            (observe_kill_diff(x, y))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }

    /* Pass 6 -- new terrains */
    for (i = auto_temp_n - 1; i >= 0; i--) {

        /* Get the grid */
        x = auto_temp_x[i];
        y = auto_temp_y[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Track new objects */
        if ((auto_is_wank[(byte)(ag->o_c)]) &&
            (observe_wank_diff(x, y))) {

            /* Hack -- excise the grid */
            auto_temp_n--;
            auto_temp_x[i] = auto_temp_x[auto_temp_n];
            auto_temp_y[i] = auto_temp_y[auto_temp_n];
        }
    }


    /*** Handle messages ***/
    
    /* Process messages */
    for (i = 0; i < auto_msg_num; i++) {

        /* Skip parsed messages */
        if (auto_msg_use[i]) continue;

        /* Get the message */
        msg = auto_msg_buf + auto_msg_pos[i];

        /* Handle "You hit xxx." */
        if (prefix(msg, "HIT:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_verify_kill(msg+4, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "You miss xxx." */
        else if (prefix(msg, "MISS:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_verify_kill(msg+5, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "xxx hits you." */
        else if (prefix(msg, "HIT_BY:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+7, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "xxx misses you." */
        else if (prefix(msg, "MISS_BY:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+8, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE:SLEEP:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+12, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "awake" */
        else if (prefix(msg, "STATE:AWAKE:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+12, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE:FEAR:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+11, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE:BOLD:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+11, FALSE)) auto_msg_use[i] = 2;
        }

        /* Hack -- Handle "spell" */
        else if (prefix(msg, "SPELL:")) {
            borg_note(format("# %s (2)", msg));
            if (borg_nearby_kill(msg+6, FALSE)) auto_msg_use[i] = 2;
        }

        /* Handle "You feel..." */
        else if (prefix(msg, "FEELING:")) {
            borg_note(format("# Feeling <%s>", msg+8));
            auto_feeling = atoi(msg+8);
            auto_msg_use[i] = 2;
        }
    }

    /* Process messages */
    for (i = 0; i < auto_msg_num; i++) {

        /* Skip parsed messages */
        if (auto_msg_use[i]) continue;

        /* Get the message */
        msg = auto_msg_buf + auto_msg_pos[i];

        /* Note invalid message */
        borg_note(format("# %s (?)", msg));
    }


    /*** Notice missing objects/monsters ***/
    
    /* Access the player grid */
    pg = grid(c_x,c_y);

    /* Scan the terrain list */
    for (i = 0; i < auto_wanks_nxt; i++) {

        auto_wank *wank = &auto_wanks[i];

        /* Skip dead terrains */
        if (!wank->f_idx) continue;

        /* Skip seen terrains */
        if (wank->when == c_t) continue;

        /* Access location */
        x = wank->x;
        y = wank->y;

        /* Access the grid */
        ag = grid(x, y);

        /* Skip dark grids */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Skip non-viewable grids */
        if (!(ag->info & BORG_VIEW)) continue;

        /* XXX XXX XXX Skip "monster" grids */
        if (auto_is_kill[(byte)(ag->o_c)]) continue;

        /* Kill the object */
        borg_delete_wank(i);
    }

    /* Scan the object list */
    for (i = 0; i < auto_takes_nxt; i++) {

        auto_take *take = &auto_takes[i];

        /* Skip dead objects */
        if (!take->k_idx) continue;

        /* Skip seen objects */
        if (take->when == c_t) continue;

        /* Access location */
        x = take->x;
        y = take->y;

        /* Access the grid */
        ag = grid(x, y);

        /* Skip dark grids */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Skip non-viewable grids */
        if (!(ag->info & BORG_VIEW)) continue;

        /* XXX XXX XXX Skip "monster" grids */
        if (auto_is_kill[(byte)(ag->o_c)]) continue;

        /* Kill the object */
        borg_delete_take(i);
    }

    /* Scan the monster list */
    for (i = 0; i < auto_kills_nxt; i++) {

        auto_kill *kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Skip seen monsters */
        if (kill->when == c_t) continue;

        /* Access location */
        x = kill->x;
        y = kill->y;

        /* Access the grid */
        ag = grid(x, y);

        /* Skip dark grids (except under player) */
        if (!(ag->info & BORG_OKAY) && (ag != pg)) continue;

        /* Skip non-viewable grids */
        if (!(ag->info & BORG_VIEW)) continue;

        /* XXX XXX XXX Many monsters "step" out of view */
        /* Currently we just assume they are dead (ick!) */

        /* Kill the monster */
        borg_delete_kill(i);
    }


    /*** Remove ancient objects/monsters ***/
    
    /* Hack -- Remove ancient terrains */
    for (i = 0; i < auto_wanks_nxt; i++) {

        auto_wank *wank = &auto_wanks[i];

        /* Skip dead terrains */
        if (!wank->f_idx) continue;

        /* Skip recently seen objects */
        if (c_t - wank->when < 2000) continue;

        /* Note */
        borg_note(format("# Expiring a terrain '%s' (%d) at (%d,%d)",
                         (f_name + f_info[wank->f_idx].name), wank->f_idx,
                         wank->x, wank->y));

        /* Kill the object */
        borg_delete_wank(i);
    }

    /* Hack -- Remove ancient objects */
    for (i = 0; i < auto_takes_nxt; i++) {

        auto_take *take = &auto_takes[i];

        /* Skip dead objects */
        if (!take->k_idx) continue;

        /* Skip recently seen objects */
        if (c_t - take->when < 2000) continue;

        /* Note */
        borg_note(format("# Expiring an object '%s' (%d) at (%d,%d)",
                         (k_name + k_info[take->k_idx].name), take->k_idx,
                         take->x, take->y));

        /* Kill the object */
        borg_delete_take(i);
    }

    /* Hack -- Remove ancient monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        auto_kill *kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Skip recently seen monsters */
        if (c_t - kill->when < 2000) continue;

        /* Note */
        borg_note(format("# Expiring a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Kill the monster */
        borg_delete_kill(i);
    }


#ifdef BORG_ROOMS

    /*** Maintain room info ***/

    /* Paranoia -- require a self room */
    if (!pg->room) {

        /* Acquire a new room */
        auto_room *ar = borg_free_room();

        /* Initialize the room */
        ar->x1 = ar->x2 = c_x;
        ar->y1 = ar->y2 = c_y;

        /* Save the room */
        pg->room = ar->self;
    }

    /* Build a "bigger" room around the player */
    if (borg_build_room(c_x, c_y)) goal = 0;

    /* Hack */
    if (TRUE) {

        auto_room *ar;

        /* Mark all the "containing rooms" as visited. */
        for (ar = room(1,c_x,c_y); ar; ar = room(0,0,0)) {

            /* Note the visit */
            ar->when = c_t;
        }
    }

#endif


    /*** Various things ***/
    
    /* Hack -- Note when a grid has been visited */
    if (pg->xtra < 1) pg->xtra = 1;

    /* Hack -- Forget goals while "impaired" in any way */
    if (do_blind || do_confused || do_afraid || do_image) goal = 0;

    /* Hack -- Forget goals while "bleeding" in any way */
    if (do_weak || do_poisoned || do_cut || do_stun) goal = 0;

    /* Mega-Hack -- Clear state when HP or SP change */
    if ((auto_chp != old_chp) || (auto_csp != old_csp)) {

        /* Reset the "brave-ness" counter */
        avoidance = auto_chp;

        /* Re-calculate danger */
        auto_danger_wipe = TRUE;

        /* Forget goals */
        goal = 0;
    }

    /* Save the hit points */
    old_chp = auto_chp;

    /* Save the spell points */
    old_csp = auto_csp;

    /* Forget the messages */
    auto_msg_len = 0;
    auto_msg_num = 0;

    /* Save location */
    g_x = o_x = c_x;
    g_y = o_y = c_y;
}


/*
 * Handle various "important" messages
 *
 * Actually, we simply "queue" them for later analysis
 */
void borg_react(cptr msg, cptr buf)
{
    int len;
    
    /* Note actual message */
    borg_note(format("> %s", msg));

    /* Extract length of parsed message */
    len = strlen(buf);
    
    /* Verify space */
    if (auto_msg_num + 1 > auto_msg_max) {
        borg_oops("too many messages");
        return;
    }

    /* Verify space */
    if (auto_msg_len + len + 1 > auto_msg_siz) {
        borg_oops("too much messages");
        return;
    }

    /* Assume not used yet */
    auto_msg_use[auto_msg_num] = 0;
            
    /* Save the message position */
    auto_msg_pos[auto_msg_num] = auto_msg_len;
    
    /* Save the message text */
    strcpy(auto_msg_buf + auto_msg_len, buf);
    
    /* Advance the buf */
    auto_msg_len += len + 1;
    
    /* Advance the pos */
    auto_msg_num++;
}





/*
 * Calculate danger (to a given grid) from a monster's physical attacks
 *
 * Note that we are paranoid, especially about "monster speed".
 */
static int borg_danger_aux1(int x, int y, int i)
{
    int d, m, k, n = 0;

    auto_kill *kill = &auto_kills[i];

    monster_race *r_ptr = &r_info[kill->r_idx];

    int x9 = kill->x;
    int y9 = kill->y;


    /* Paranoia -- skip dead monsters */
    if (!kill->r_idx) return (0);
    
    /* No attacks for some monsters */
    if (r_ptr->flags1 & RF1_NEVER_BLOW) return (0);


    /* Hack -- Distance needed to move */
    d = MAX(ABS(x9-x),ABS(y9-y));

    /* Paranoia -- assume distance */
    if (d < 1) d = 1;

    /* No movement for some monsters */
    if ((r_ptr->flags1 & RF1_NEVER_MOVE) && (d > 1)) return (0);


    /* Extract moves */
    m = kill->moves;

    /* Physical attacks require proximity */
    if (m < d) return (0);


    /* Analyze each physical attack */
    for (k = 0; k < 4; k++) {

        monster_blow *b_ptr = &r_ptr->blow[k];

        /* Done */
        if (!b_ptr->method) break;

        /* Assume maximum damage */
        n += (b_ptr->d_dice * b_ptr->d_side);

        /* Analyze the attack */
        switch (b_ptr->effect) {

            case RBE_EAT_ITEM:
                if (auto_stat[A_DEX] >= 18+150) break;
                n += 20;
                break;

            case RBE_EAT_GOLD:
                if (auto_stat[A_DEX] >= 18+150) break;
                if (auto_gold < 100) break;
                n += 10;
                break;

            case RBE_BLIND:
                if (my_resist_blind) break;
                n += 10;
                break;

            case RBE_CONFUSE:
                if (my_resist_conf) break;
                n += 10;
                break;

            case RBE_TERRIFY:
                if (my_resist_fear) break;
                n += 10;
                break;

            case RBE_POISON:
                if (my_resist_pois) break;
                n += 10;
                break;

            case RBE_PARALYZE:
                if (my_free_act) break;
                n += 20;
                break;

            case RBE_ACID:
                if (my_immune_acid) break;
                n += 40;
                break;

            case RBE_ELEC:
                if (my_immune_elec) break;
                n += 20;
                break;

            case RBE_FIRE:
                if (my_immune_fire) break;
                n += 40;
                break;

            case RBE_COLD:
                if (my_immune_cold) break;
                n += 20;
                break;

            case RBE_LOSE_STR:
                if (my_sustain_str) break;
                if (auto_stat[A_STR] <= 3) break;
                n += 50;
                break;
                
            case RBE_LOSE_DEX:
                if (my_sustain_dex) break;
                if (auto_stat[A_DEX] <= 3) break;
                n += 50;
                break;
                
            case RBE_LOSE_CON:
                if (my_sustain_con) break;
                if (auto_stat[A_CON] <= 3) break;
                n += 50;
                break;
                
            case RBE_LOSE_INT:
                if (my_sustain_int) break;
                if (auto_stat[A_INT] <= 3) break;
                n += 50;
                break;
                
            case RBE_LOSE_WIS:
                if (my_sustain_wis) break;
                if (auto_stat[A_WIS] <= 3) break;
                n += 50;
                break;
                
            case RBE_LOSE_CHR:
                if (my_sustain_chr) break;
                if (auto_stat[A_CHR] <= 3) break;
                n += 50;
                break;

            case RBE_EXP_10:
                n += 100;
                break;
                
            case RBE_EXP_20:
                n += 150;
                break;
                
            case RBE_EXP_40:
                n += 200;
                break;
                
            case RBE_EXP_80:
                n += 250;
                break;

            case RBE_LOSE_ALL:
                n += 500;
                break;

            case RBE_UN_BONUS:
                n += 500;
                break;
        }
    }

    /* Assume full attacks after movement */
    n *= (m - (d - 1));

    /* Return danger */
    return (n);
}


/*
 * Calculate danger (to a given grid) from a monster's spell attacks
 *
 * Need to consider possibility of movement and then spells
 */
static int borg_danger_aux2(int x, int y, int i)
{
    int m, z, k, n = 0;

    int lev, hp;

    byte spell[96], num = 0;

    auto_kill *kill = &auto_kills[i];

    monster_race *r_ptr = &r_info[kill->r_idx];

    int x9 = kill->x;
    int y9 = kill->y;


    /* Paranoia -- skip dead monsters */
    if (!kill->r_idx) return (0);
    
    /* Never cast spells */
    if (!r_ptr->freq_inate && !r_ptr->freq_spell) return (0);

    /* Mega-Hack -- verify distance XXX XXX XXX */
    if (distance(x9,y9,x,y) > MAX_RANGE) return (0);

    /* Mega-Hack -- verify line of sight */
    if (!borg_projectable(x9, y9, x, y)) return (0);


    /* Extract the "inate" spells */
    for (k = 0; k < 32; k++) {
        if (r_ptr->flags4 & (1L << k)) spell[num++] = k + 32 * 3;
    }
    
    /* Extract the "normal" spells */
    for (k = 0; k < 32; k++) {
        if (r_ptr->flags5 & (1L << k)) spell[num++] = k + 32 * 4;
    }
    
    /* Extract the "bizarre" spells */
    for (k = 0; k < 32; k++) {
        if (r_ptr->flags6 & (1L << k)) spell[num++] = k + 32 * 5;
    }

    /* Paranoia -- Nothing to cast */
    if (!num) return (0);


    /* Extract number of moves */
    m = kill->moves;

    /* Extract the level */
    lev = r_ptr->level;

    /* Assume fully healed */
    hp = maxroll(r_ptr->hdice, r_ptr->hside);


    /* Analyze the spells */
    for (z = 0; z < num; z++) {

        int p = 0;

        /* Cast the spell. */
        switch (spell[z]) {

          case 96+0:    /* RF4_SHRIEK */
            p += 10;
            break;

          case 96+1:    /* RF4_XXX2X4 */
            break;

          case 96+2:    /* RF4_XXX3X4 */
            break;

          case 96+3:    /* RF4_XXX4X4 */
            break;

          case 96+4:    /* RF4_ARROW_1 */
            p += (1 * 6);
            break;

          case 96+5:    /* RF4_ARROW_2 */
            p += (3 * 6);
            break;

          case 96+6:    /* RF4_ARROW_3 */
            p += (5 * 6);
            break;

          case 96+7:    /* RF4_ARROW_4 */
            p += (7 * 6);
            break;

          case 96+8:    /* RF4_BR_ACID */
            p += (hp / 3);
            p += 40;
            break;

          case 96+9:    /* RF4_BR_ELEC */
            p += (hp / 3);
            p += 20;
            break;

          case 96+10:    /* RF4_BR_FIRE */
            p += (hp / 3);
            p += 40;
            break;

          case 96+11:    /* RF4_BR_COLD */
            p += (hp / 3);
            p += 20;
            break;

          case 96+12:    /* RF4_BR_POIS */
            p += (hp / 3);
            p += 20;
            break;

          case 96+13:    /* RF4_BR_NETH */
            p += (hp / 6);
            p += 500;
            break;

          case 96+14:    /* RF4_BR_LITE */
            p += (hp / 6);
            p += 20;
            break;

          case 96+15:    /* RF4_BR_DARK */
            p += (hp / 6);
            p += 20;
            break;

          case 96+16:    /* RF4_BR_CONF */
            p += (hp / 6);
            p += 100;
            break;

          case 96+17:    /* RF4_BR_SOUN */
            p += (hp / 6);
            p += 100;
            break;

          case 96+18:    /* RF4_BR_CHAO */
            p += (hp / 6);
            p += 500;
            break;

          case 96+19:    /* RF4_BR_DISE */
            p += (hp / 6);
            p += 500;
            break;

          case 96+20:    /* RF4_BR_NEXU */
            p += (hp / 3);
            p += 100;
            break;

          case 96+21:    /* RF4_BR_TIME */
            p += (hp / 3);
            p += 100;
            break;

          case 96+22:    /* RF4_BR_INER */
            p += (hp / 6);
            break;

          case 96+23:    /* RF4_BR_GRAV */
            p += (hp / 3);
            break;

          case 96+24:    /* RF4_BR_SHAR */
            p += (hp / 6);
            break;

          case 96+25:    /* RF4_BR_PLAS */
            p += (hp / 6);
            break;

          case 96+26:    /* RF4_BR_WALL */
            p += (hp / 6);
            break;

          case 96+27:    /* RF4_BR_MANA */
            /* XXX XXX XXX */
            break;

          case 96+28:    /* RF4_XXX5X4 */
            break;

          case 96+29:    /* RF4_XXX6X4 */
            break;

          case 96+30:    /* RF4_XXX7X4 */
            break;

          case 96+31:    /* RF4_XXX8X4 */
            break;



          case 128+0:    /* RF5_BA_ACID */
            p += (lev * 3) + 15;
            p += 40;
            break;

          case 128+1:    /* RF5_BA_ELEC */
            p += (lev * 3) / 2 + 8;
            p += 20;
            break;

          case 128+2:    /* RF5_BA_FIRE */
            p += (lev * 7) / 2 + 10;
            p += 40;
            break;

          case 128+3:    /* RF5_BA_COLD */
            p += (lev * 3) / 2 + 10;
            p += 20;
            break;

          case 128+4:    /* RF5_BA_POIS */
            p += (12 * 2);
            p += 20;
            break;

          case 128+5:    /* RF5_BA_NETH */
            p += (50 + (10 * 10) + lev);
            p += 200;
            break;

          case 128+6:    /* RF5_BA_WATE */
            p += ((lev * 5) / 2) + 50;
            break;

          case 128+7:    /* RF5_BA_MANA */
            p += ((lev * 5) + (10 * 10));
            break;

          case 128+8:    /* RF5_BA_DARK */
            p += ((lev * 5) + (10 * 10));
            p += 10;
            break;

          case 128+9:    /* RF5_DRAIN_MANA */
            if (auto_csp) p += 10;
            break;

          case 128+10:    /* RF5_MIND_BLAST */
            p += 20;
            break;

          case 128+11:    /* RF5_BRAIN_SMASH */
            p += (12 * 15);
            p += 100;
            break;

          case 128+12:    /* RF5_CAUSE_1 */
            p += (3 * 8);
            break;

          case 128+13:    /* RF5_CAUSE_2 */
            p += (8 * 8);
            break;

          case 128+14:    /* RF5_CAUSE_3 */
            p += (10 * 15);
            break;

          case 128+15:    /* RF5_CAUSE_4 */
            p += (15 * 15);
            p += 100;
            break;

          case 128+16:    /* RF5_BO_ACID */
            p += ((7 * 8) + (lev / 3));
            p += 40;
            break;

          case 128+17:    /* RF5_BO_ELEC */
            p += ((4 * 8) + (lev / 3));
            p += 20;
            break;

          case 128+18:    /* RF5_BO_FIRE */
            p += ((9 * 8) + (lev / 3));
            p += 40;
            break;

          case 128+19:    /* RF5_BO_COLD */
            p += ((6 * 8) + (lev / 3));
            p += 20;
            break;

          case 128+20:    /* RF5_BO_POIS */
            /* XXX XXX XXX */
            break;

          case 128+21:    /* RF5_BO_NETH */
            p += (30 + (5 * 5) + (lev * 3) / 2);
            p += 200;
            break;

          case 128+22:    /* RF5_BO_WATE */
            p += ((10 * 10) + (lev));
            break;

          case 128+23:    /* RF5_BO_MANA */
            p += ((lev * 7) / 2) + 50;
            break;

          case 128+24:    /* RF5_BO_PLAS */
            p += (10 + (8 * 7) + (lev));
            break;

          case 128+25:    /* RF5_BO_ICEE */
            p += ((6 * 6) + (lev));
            p += 20;
            break;

          case 128+26:    /* RF5_MISSILE */
            p += ((2 * 6) + (lev / 3));
            break;

          case 128+27:    /* RF5_SCARE */
            p += 10;
            break;

          case 128+28:    /* RF5_BLIND */
            p += 10;
            break;

          case 128+29:    /* RF5_CONF */
            p += 10;
            break;

          case 128+30:    /* RF5_SLOW */
            p += 5;
            break;

          case 128+31:    /* RF5_HOLD */
            p += 20;
            break;



          case 160+0:    /* RF6_HASTE */
            p += 10;
            break;

          case 160+1:    /* RF6_XXX1X6 */
            break;

          case 160+2:    /* RF6_HEAL */
            p += 10;
            break;

          case 160+3:    /* RF6_XXX2X6 */
            break;

          case 160+4:    /* RF6_BLINK */
            break;

          case 160+5:    /* RF6_TPORT */
            break;

          case 160+6:    /* RF6_XXX3X6 */
            break;

          case 160+7:    /* RF6_XXX4X6 */
            break;

          case 160+8:    /* RF6_TELE_TO */
            p += 20;
            break;

          case 160+9:    /* RF6_TELE_AWAY */
            p += 10;
            break;

          case 160+10:    /* RF6_TELE_LEVEL */
            p += 50;
            break;

          case 160+11:    /* RF6_XXX5 */
            break;

          case 160+12:    /* RF6_DARKNESS */
            p += 5;
            break;

          case 160+13:    /* RF6_TRAPS */
            p += 50;
            break;

          case 160+14:    /* RF6_FORGET */
            p += 500;
            break;

          case 160+15:    /* RF6_XXX6X6 */
            break;

          case 160+16:    /* RF6_XXX7X6 */
            break;

          case 160+17:    /* RF6_XXX8X6 */
            break;

          case 160+18:    /* RF6_S_MONSTER */
            p += (lev) * 10;
            break;

          case 160+19:    /* RF6_S_MONSTERS */
            p += (lev) * 20;
            break;

          case 160+20:    /* RF6_S_ANT */
            p += (lev) * 20;
            break;

          case 160+21:    /* RF6_S_SPIDER */
            p += (lev) * 20;
            break;

          case 160+22:    /* RF6_S_HOUND */
            p += (lev) * 20;
            break;

          case 160+23:    /* RF6_S_HYDRA */
            p += (lev) * 20;
            break;

          case 160+24:    /* RF6_S_ANGEL */
            p += (lev) * 30;
            break;

          case 160+25:    /* RF6_S_DEMON */
            p += (lev) * 30;
            break;

          case 160+26:    /* RF6_S_UNDEAD */
            p += (lev) * 30;
            break;

          case 160+27:    /* RF6_S_DRAGON */
            p += (lev) * 30;
            break;

          case 160+28:    /* RF6_S_HI_UNDEAD */
            p += (lev) * 50;
            break;

          case 160+29:    /* RF6_S_HI_DRAGON */
            p += (lev) * 50;
            break;

          case 160+30:    /* RF6_S_WRAITH */
            p += (lev) * 50;
            break;

          case 160+31:    /* RF6_S_UNIQUE */
            p += (lev) * 50;
            break;
        }

        /* Track most dangerous spell */
        if (p > n) n = p;
    }

    /* Hack -- be paranoid */
    if (panic_power) n *= m;

    /* Danger */
    return (n);
}


/*
 * Hack -- Calculate the "danger" of the given grid.
 *
 * Note -- we ignore any monster actually in the given grid.
 *
 * Currently based on the physical power of nearby monsters, as well
 * as the spell power of monsters within line of sight.
 *
 * This function is extremely expensive, mostly due to the number of
 * times it is called, and also to the fact that it calls its helper
 * functions about thirty times each per call.
 */
static int borg_danger(int x, int y)
{
    int i, p = 0;

    auto_grid *ag;


    /* Look at the current grid */
    ag = grid(x, y);

    /* Hack -- traps are dangerous */
    if (ag->o_c == '^') {

        int v1 = 0;

        /* Hack -- Guess the trap */
        for (i = 0x10; i <= 0x1F; i++) {

            int v2 = 0;

            /* Verify attr/char */
            if (ag->o_c != f_info[i].z_char) continue;
            if (ag->o_a != f_info[i].z_attr) continue;

            /* Analyze */
            switch (i) {

                case 0x10: v2 = 50; break;
                case 0x11: v2 = 10; break;
                case 0x12: v2 = 30; break;
                case 0x13: v2 = 40; break;
                case 0x04: v2 = 20; break;
                case 0x05: v2 = 10; break;
                case 0x06: v2 = 50; break;
                case 0x07: v2 = 50; break;
                case 0x08: v2 = 10; break;
                case 0x09: v2 = 50; break;
                case 0x0A: v2 = 50; break;
                case 0x0B: v2 = 50; break;
                case 0x1C: v2 = 25; break;
                case 0x1D: v2 = 25; break;
                case 0x1E: v2 = 25; break;
                case 0x1F: v2 = 25; break;
            }

            /* Maintain max danger */
            if (v1 < v2) v1 = v2;
        }

        /* Add in the danger */
        p += v1;
    }


    /* Examine all the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        int v1, v2;
        
        auto_kill *kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;
        
        /* Danger from physical attacks */
        v1 = borg_danger_aux1(x, y, i);

        /* Danger from spell attacks */
        v2 = borg_danger_aux2(x, y, i);

        /* Assume maximal danger */
        p += MAX(v1, v2);
    }


    /* Return the danger */
    return (p);
}



/*
 * Mega-Hack -- evaluate the "freedom" of the given location
 *
 * This may help avoid getting stuck in corners
 *
 * XXX XXX XXX We need a better method (!)
 */
static int borg_freedom(int x, int y)
{
    int i, d, f = 0;

    /* Mega-Hack -- chase stairs in town */
    if (!auto_depth && track_more_num) {

        /* Love the stairs! */
        d = double_distance(x, y, track_more_x[0], track_more_y[0]);

        /* Distance is a bad thing */
        f += (1000 - d);

        /* Zero distance yields much freedom */
        if (!d) f += 1000;

        /* Small distance yields some freedom */
        if (d <= 2) f += 500;
    }

    /* Desire free space */
    for (i = 0; i < 8; i++) {

        int x1 = x + ddx_ddd[i];
        int y1 = y + ddy_ddd[i];

        auto_grid *ag = grid(x1, y1);

        /* Count open space */
        if (!(ag->info & BORG_WALL)) f++;
    }

    /* Freedom */
    return (f);
}




/*
 * Be "cautious" and attempt to prevent death.
 *
 * This routine attempts to remove life threatening problems, such
 * as starvation, poison, monsters, etc, in dangerous situations,
 * either by curing or fleeing from the cause.  In non-dangerous
 * situations, we let "borg_recover()" cure annoying maladies.
 *
 * Note that the "flow" routines attempt to avoid entering into
 * situations that are dangerous, but sometimes we do not see the
 * danger coming, and then we must attempt to survive by any means.
 *
 * Invisible monsters may cause all kinds of nasty behaviors, since
 * technically we do not know where they are, and so we cannot take
 * account of the "danger" they supply.
 *
 * Note that, for example, resting (to heal damage) near an invisible
 * monster is silly, since the monster will just keep attacking.  This
 * is similar to the problem which occurs when we are standing between
 * a creeping copper coins and an embedded copper coins, since we often
 * get the identities wrong, and keep "attacking" the wall which being
 * damaged by the coins.  Luckily, attacking a wall now takes a turn...
 *
 * XXX XXX XXX We should probably attempt to assign "danger" to any
 * situation which might include "hidden" danger, including invisible
 * monsters, and monsters not visible to the player, and monsters that
 * cannot be parsed, such as the player ghost, or mimics, or trappers.
 *
 * XXX XXX XXX Note that it should be possible to do some kind of nasty
 * "flow" algorithm which would use a priority queue, or some reasonably
 * efficient normal queue stuff, to determine the path which incurs the
 * smallest "cumulative danger", and minimizes the total path length.
 * It may even be sufficient to treat each step as having a cost equal
 * to the danger of the destination grid, plus one for the actual step.
 * This would allow the Borg to prefer a ten step path passing through
 * one grid with danger 10, to a five step path, where each step has
 * danger 9.  Currently, he often chooses paths of constant danger over
 * paths with small amounts of high danger.
 */
bool borg_caution(void)
{
    int p;
    
    int k, b_k = -1;
    int i, b_i = -1;
    int x, b_x = c_x;
    int y, b_y = c_y;
    
    auto_grid *ag;


    /* Look around */
    p = borg_danger(c_x, c_y);


    /* Hack -- Ignore "easy" monsters */
    if (p <= avoidance / 4) return (FALSE);

    /* Hack -- Often ignore "medium" monsters */
    if ((p <= avoidance / 2) && (rand_int(100) < 80)) return (FALSE);

    /* Hack -- Sometimes ignore "hard" monsters */
    if ((p <= avoidance) && (rand_int(100) < 20)) return (FALSE);



    /* Hack -- Check for stairs */
    if (p > avoidance) {

        ag = grid(c_x,c_y);

        /* Mega-Hack */
        if (ag->o_c == '<') {

            /* Flee! */
            borg_note("# Fleeing up some stairs");
            borg_keypress('<');
            return (TRUE);
        }

        /* Mega-Hack */
        if (ag->o_c == '>') {

            /* Flee! */
            borg_note("# Fleeing down some stairs");
            borg_keypress('>');
            return (TRUE);
        }
        
        /* Mega-Hack */
        goal_fleeing = TRUE;
    }


    /* Attempt to escape */
    if ((auto_chp <= auto_mhp / 4) && (p > avoidance)) {

        /* Prefer "fastest" methods */
        if (borg_read_scroll(SV_SCROLL_TELEPORT) ||
            borg_use_staff(SV_STAFF_TELEPORTATION)) {

            /* Hack -- reset the "goal" location */
            g_x = g_y = 0;
            
            /* Success */
            return (TRUE);
        }
    }

    /* Attempt to teleport */
    if ((auto_chp <= auto_mhp / 2) && (p > avoidance / 2)) {

        /* Prefer "cheapest" methods */
        if (borg_spell(1,5) ||
            borg_prayer(1,1) ||
            borg_use_staff(SV_STAFF_TELEPORTATION) ||
            borg_read_scroll(SV_SCROLL_TELEPORT) ||
            borg_spell(0,2) ||
            borg_read_scroll(SV_SCROLL_PHASE_DOOR)) {

            /* Hack -- reset the "goal" location */
            g_x = g_y = 0;

            /* Success */
            return (TRUE);
        }
    }


    /* Hack -- heal when wounded */
    if ((auto_chp <= auto_mhp / 2) && (rand_int(100) < 10)) {
        if (borg_quaff_potion(SV_POTION_CURE_CRITICAL) ||
            borg_quaff_potion(SV_POTION_CURE_SERIOUS)) {
            return (TRUE);
        }
    }

    /* Hack -- heal when blind/confused */
    if ((do_blind || do_confused) && (rand_int(100) < 10)) {
        if (borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
            borg_quaff_potion(SV_POTION_CURE_CRITICAL)) {
            return (TRUE);
        }
    }

    /* Hack -- cure poison when poisoned */
    if (do_poisoned && (rand_int(100) < 10)) {
        if (borg_spell(1,4) ||
            borg_prayer(2,0) ||
            borg_quaff_potion(SV_POTION_CURE_POISON)) {
            return (TRUE);
        }
    }

    /* Hack -- cure fear when afraid */
    if (do_afraid && (rand_int(100) < 10)) {
        if (borg_prayer(0,3) ||
            borg_quaff_potion(SV_POTION_BOLDNESS)) {
            return (TRUE);
        }
    }


    /* Attempt to find a better grid */
    for (i = 0; i < 8; i++) {

        x = c_x + ddx_ddd[i];
        y = c_y + ddy_ddd[i];

        /* Access the grid */
        ag = grid(x, y);

        /* Skip walls (!) */
        if (ag->info & BORG_WALL) continue;

        /* Skip unknown grids (?) */
        if (ag->o_c == ' ') continue;

        /* XXX XXX XXX Skip monsters */
        if (auto_is_kill[(byte)(ag->o_c)]) continue;

        /* Hack -- skip stores in town */
        if (!auto_depth && (isdigit(ag->o_c))) continue;

        /* Extract the danger there */
        k = borg_danger(x, y);

        /* Ignore "worse" choices */
        if ((b_i >= 0) && (k > b_k)) continue;

        /* Hack -- Compare "equal" choices */
        if ((b_i >= 0) && (k == b_k)) {

            int new = borg_freedom(x,y);
            int old = borg_freedom(b_x, b_y);

            /* Hack -- run for freedom */
            if (new < old) continue;
        }

        /* Save the info */
        b_k = k; b_i = i; b_x = x; b_y = y;
    }


    /* XXX XXX XXX XXX */

    /* Somewhere "useful" to flee */
    if ((b_i >= 0) && (p > b_k + (b_k / 4))) {

        /* Note */
        borg_note(format("# Caution (%d > %d)", p, b_k));

        /* Hack -- set goal */
        g_x = b_x;
        g_y = b_y;

        /* Hack -- Flee! */
        borg_keypress('0' + ddd[b_i]);

        /* Success */
        return (TRUE);
    }

    /* Note */
    borg_note(format("# Cornered (danger %d)", p));

    /* Hack -- flee this level */
    goal_fleeing = TRUE;

    /* Nothing */
    return (FALSE);
}


/*
 * Attack monsters (before firing)
 */
bool borg_attack(void)
{
    int x, y, v1, v2, dir;

    int b_n = 0;

    int i, b_i = -1;
    int p, b_p = -1;

    auto_grid *ag;

    auto_kill *kill;


    /* Examine all the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Acquire the grid */
        ag = grid(x, y);

        /* Require "adjacent" */
        if (distance(c_x, c_y, x, y) > 1) continue;

        /* Hack -- Danger from physical attacks */
        v1 = borg_danger_aux1(x, y, i);

        /* Hack -- Danger from spell attacks */
        v2 = borg_danger_aux2(x, y, i);

        /* Assume maximal danger */
        p = MAX(v1, v2);

        /* Attack the "hardest" monster */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Hack -- reset chooser */
        if ((b_i >= 0) && (p > b_p)) b_n = 0;

        /* Apply the randomizer */
        if ((++b_n > 1) && (rand_int(b_n) != 0)) continue;
        
        /* Save the info */
        b_i = i; b_p = p;
    }

    /* Nothing to attack */
    if (b_i < 0) return (FALSE);


    /* Acquire the target */
    kill = &auto_kills[b_i];


    /* Hack -- ignoring monsters */
    if (goal_ignoring) {
        borg_note(format("# Ignoring a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));
        return (FALSE);
    }


    /* Mega-Hack -- Do not attack when afraid */
    if (do_afraid) {
        borg_note(format("# Fearing a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));
        return (FALSE);
    }

    /* Mention monster */
    borg_note(format("# Attacking a monster '%s' (%d) at (%d,%d)",
                     (r_name + r_info[kill->r_idx].name), kill->r_idx,
                     kill->x, kill->y));

    /* Get a direction (may be a wall there) */
    dir = borg_goto_dir(c_x, c_y, kill->x, kill->y);

    /* Hack -- set goal */
    g_x = c_x + ddx[dir];
    g_y = c_y + ddy[dir];

    /* Hack -- Attack! */
    borg_keypress('0' + dir);

    /* Success */
    return (TRUE);
}




/*
 * Fire a missile, if possible
 */
static bool borg_fire_missile(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip bad missiles */
        if (item->tval != my_ammo_tval) continue;

        /* Skip worthless missiles */
        if (item->value <= 0) continue;
        
        /* Skip un-identified, non-average, missiles */
        if (!item->able && !streq(item->note, "{average}")) continue;

        /* Find the smallest pile */
        if ((n < 0) || (item->iqty < auto_items[n].iqty)) n = i;
    }

    /* Use that missile */
    if (n >= 0) {

        borg_note("# Firing standard missile");

        borg_keypress('f');
        borg_keypress(I2A(n));

        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}





/*
 * Attempt to launch distance attacks at monsters
 *
 * XXX XXX Need to match attack damage against monster power and
 * monster danger, also, consider ball attacks on monster clumps.
 *
 * XXX XXX XXX Need to consider monster "immunity" or "resistance".
 */
bool borg_launch(void)
{
    int i, x, y, d, p;

    int v1, v2;

    int f_n = -1;
    int f_p = -1;
    int f_d = -1;
    int f_x = c_x;
    int f_y = c_y;

    auto_grid *ag;

    auto_kill *kill;


    /* Examine all the monsters */
    for (i = 0; i < auto_kills_nxt; i++) {

        kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Acquire the grid */
        ag = grid(x, y);

        /* Never shoot walls */
        if (ag->info & BORG_WALL) continue;

        /* Never shoot at "dark" grids */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Require line of sight */
        if (!(ag->info & BORG_VIEW)) continue;

        /* Check distance */
        d = distance(c_y, c_x, y, x);

        /* Must be within "range" */
        if (d > 8) continue;

        /* Must have "missile line of sight" */
        if (!borg_projectable_pure(c_x, c_y, x, y)) continue;

        /* Paranoia -- Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Hack -- Danger from physical attacks */
        v1 = borg_danger_aux1(x, y, i);

        /* Hack -- Danger from spell attacks */
        v2 = borg_danger_aux2(x, y, i);

        /* Assume maximal danger */
        p = MAX(v1, v2);

        /* Hack -- ignore "easy" town monsters */
        if (!auto_depth && (p <= avoidance / 2)) continue;

        /* Choose hardest monster */
        if (p < f_p) continue;

        /* Save the distance */
        f_n = i; f_p = p; f_d = d; f_x = x; f_y = y;
    }

    /* Nothing to attack */
    if (f_n < 0) return (FALSE);


    /* Access the monster */
    kill = &auto_kills[f_n];


    /* Unless "worried", do not fire a lot */
    if (!do_afraid && (f_p <= avoidance / 2)) {

        /* Skip close monsters, and most monsters */
        if ((f_d <= 1) || (rand_int(100) < 20)) return (FALSE);
    }


    /* Hack -- ignoring monsters */
    if (goal_ignoring) {
        borg_note(format("# Ignoring a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));
        return (FALSE);
    }


#if 0

    /* XXX XXX Frost Bolt (if needed) */
    if ((kill->power > 16) && borg_spell_safe(1,7)) {

        /* Note */
        borg_note(format("# Targetting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Target the monster */
        (void)borg_target(f_x, f_y);

        /* Save the location */
        g_x = f_x;
        g_y = f_y;

        /* Success */
        return (TRUE);
    }

    /* XXX XXX Lightning Bolt (if needed) */
    if ((kill->power > 8) && borg_spell_safe(1,1)) {

        /* Note */
        borg_note(format("# Targetting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Target the monster */
        (void)borg_target(f_x, f_y);

        /* Save the location */
        g_x = f_x;
        g_y = f_y;

        /* Success */
        return (TRUE);
    }

#endif

    /* Magic Missile */
    if (borg_spell_safe(0,0)) {

        /* Note */
        borg_note(format("# Targetting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Target the monster */
        (void)borg_target(f_x, f_y);

        /* Save the location */
        g_x = f_x;
        g_y = f_y;

        /* Success */
        return (TRUE);
    }

    /* Physical missile */
    if (borg_fire_missile()) {

        /* Note */
        borg_note(format("# Targetting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Target the monster */
        (void)borg_target(f_x, f_y);

        /* Save the location */
        g_x = f_x;
        g_y = f_y;

        /* Success */
        return (TRUE);
    }

#if 0

    /* XXX XXX XXX Stinking Cloud */
    if ((kill->power > 8) && borg_spell_safe(0,8)) {

        /* Note */
        borg_note(format("# Targetting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Target the monster */
        (void)borg_target(f_x, f_y);

        /* Save the location */
        g_x = f_x;
        g_y = f_y;

        /* Success */
        return (TRUE);
    }

#endif

    /* XXX XXX XXX Orb of draining */
    if ((kill->power > 8) && borg_prayer_safe(2,1)) {

        /* Note */
        borg_note(format("# Targetting a monster '%s' (%d) at (%d,%d)",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->x, kill->y));

        /* Target the monster */
        (void)borg_target(f_x, f_y);

        /* Save the location */
        g_x = f_x;
        g_y = f_y;

        /* Success */
        return (TRUE);
    }


    /* Oh well */
    return (FALSE);
}



/*
 * Attempt to recover from damage and such after a battle
 *
 * Note that resting with invisible (or "unknown") monsters nearby
 * can be hazardous to ones health.  XXX XXX XXX
 */
bool borg_recover(void)
{
    int p;


    /*** Do not recover when in danger ***/

    /* Look around for danger */
    p = borg_danger(c_x, c_y);

    /* Never recover in dangerous situations */
    if (p > avoidance / 4) return (FALSE);

    /* XXX XXX XXX Handle "invisible" monsters */


    /* Hack -- ignoring monsters */
    if (goal_ignoring) {
        borg_note("# Ignoring chance to recover");
        return (FALSE);
    }


    /*** Life threatening problems ***/

    /* Hack -- cure poison */
    if (do_poisoned && (rand_int(100) < 90)) {
        if (borg_spell(1,4) ||
            borg_prayer(2,0)) {
            return (TRUE);
        }
    }

    /* Hack -- cure poison */
    if (do_poisoned && (rand_int(100) < 10)) {
        if (borg_quaff_potion(SV_POTION_CURE_POISON)) {
            return (TRUE);
        }
    }


    /*** Minor annoyances ***/

    /* Hack -- cure fear */
    if (do_afraid && (rand_int(100) < 50)) {
        if (borg_prayer(0,3)) {
            return (TRUE);
        }
    }

    /* Hack -- satisfy hunger */
    if (do_hungry && (rand_int(100) < 90)) {
        if (borg_spell(2,0) ||
            borg_prayer(1,5)) {
            return (TRUE);
        }
    }


    /*** Rest (etc) until healed ***/

    /* Hack -- cure poison when poisoned */
    if (do_poisoned && (rand_int(100) < 20)) {
        if (borg_spell(1,4) ||
            borg_prayer(2,0) ||
            borg_quaff_potion(SV_POTION_CURE_POISON)) {
            return (TRUE);
        }
    }

    /* Hack -- cure fear when afraid */
    if (do_afraid && (rand_int(100) < 20)) {
        if (borg_prayer(0,3) ||
            borg_quaff_potion(SV_POTION_BOLDNESS)) {
            return (TRUE);
        }
    }

    /* Hack -- heal when wounded */
    if ((auto_chp <= auto_mhp / 4) &&
        (do_poisoned || do_cut || do_blind || do_confused) &&
        (rand_int(100) < 10)) {
        if (borg_quaff_potion(SV_POTION_CURE_CRITICAL) ||
            borg_quaff_potion(SV_POTION_CURE_SERIOUS)) {
            return (TRUE);
        }
    }

    /* Hack -- Rest to fix various "problems" */
    if ((do_blind || do_confused || do_image ||
         do_poisoned || do_cut || do_afraid || do_stun ||
         (auto_chp < auto_mhp) || (auto_csp < auto_msp)) &&
        (rand_int(100) < 90)) {

        /* XXX XXX XXX */

        /* Take note */
        borg_note(format("# Resting (danger %d)...", p));

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}







/*
 * Take one "step" towards the given location, return TRUE if possible
 */
static bool borg_play_step(int x2, int y2)
{
    auto_grid *ag;

    int dir, x, y;


    /* We have arrived */
    if ((c_x == x2) && (c_y == y2)) return (FALSE);


    /* Get a direction */
    dir = borg_goto_dir(c_x, c_y, x2, y2);

    /* We are confused */
    if ((dir == 0) || (dir == 5)) return (FALSE);


    /* Obtain the destination */
    x = c_x + ddx[dir];
    y = c_y + ddy[dir];

    /* Access the grid we are stepping on */
    ag = grid(x, y);


    /* Traps -- disarm */
    if (ag->o_c == '^') {

        /* Hack -- set goal */
        g_x = c_x + ddx[dir];
        g_y = c_y + ddy[dir];

        /* Disarm */
        borg_note("# Disarming a trap");
        borg_keypress('D');
        borg_keypress('0' + dir);
        return (TRUE);
    }


    /* Doors -- Bash occasionally */
    if ((ag->o_c == '+') && (rand_int(100) < 10)) {

        /* Mega-Hack -- prevent infinite loops */
        if (rand_int(100) < 10) return (FALSE);

        /* Hack -- set goal */
        g_x = c_x + ddx[dir];
        g_y = c_y + ddy[dir];

        /* Hack -- cancel wall */
        ag->info &= ~BORG_WALL;

        /* Bash */
        borg_note("# Bashing a door");
        borg_keypress('B');
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Doors -- Open */
    if (ag->o_c == '+') {

        /* Mega-Hack -- prevent infinite loops */
        if (rand_int(100) < 10) return (FALSE);

        /* Hack -- cancel wall */
        ag->info &= ~BORG_WALL;

        /* Open */
        borg_note("# Opening a door");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('o');
        borg_keypress('0' + dir);
        return (TRUE);
    }


    /* Rubble, Treasure, Seams, Walls -- Tunnel or Melt */
    if ((ag->o_c == ':') || (ag->o_c == '*') ||
        (ag->o_c == '#') || (ag->o_c == '%')) {

        /* Mega-Hack -- prevent infinite loops */
        if (rand_int(100) < 10) return (FALSE);

        /* Hack -- cancel wall */
        ag->info &= ~BORG_WALL;

        /* Mega-Hack -- allow "stone to mud" */
        if (borg_spell(1,8)) {
            borg_note("# Melting a wall/etc");
            borg_keypress('0' + dir);
            return (TRUE);
        }

        /* Tunnel */
        borg_note("# Digging through wall/etc");
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');
        borg_keypress('0' + dir);
        return (TRUE);
    }


    /* Shops -- Enter */
    if (isdigit(ag->o_c)) {

        /* Hack -- set goal */
        g_x = c_x + ddx[dir];
        g_y = c_y + ddy[dir];

        /* Enter the shop */
        borg_note(format("# Entering a '%c' shop", ag->o_c));
        borg_keypress('0' + dir);
        return (TRUE);
    }


    /* Objects -- Take */
    if (auto_is_take[(byte)(ag->o_c)]) {


        /* Hack -- set goal */
        g_x = c_x + ddx[dir];
        g_y = c_y + ddy[dir];

        /* Walk onto it */
        borg_note(format("# Walking onto a '%c' object", ag->o_c));
        borg_keypress('0' + dir);
        return (TRUE);
    }

    /* Monsters -- Attack */
    if (auto_is_kill[(byte)(ag->o_c)]) {

        /* Hack -- set goal */
        g_x = c_x + ddx[dir];
        g_y = c_y + ddy[dir];

        /* Walk into it */
        borg_note(format("# Walking into a '%c' monster", ag->o_c));
        borg_keypress('0' + dir);
        return (TRUE);
    }


    /* Hack -- set goal */
    g_x = c_x + ddx[dir];
    g_y = c_y + ddy[dir];

    /* Walk in that direction */
    borg_keypress('0' + dir);


    /* Did something */
    return (TRUE);
}




/*
 * Act twitchy
 */
bool borg_twitchy(void)
{
    int dir;

    /* This is a bad thing */
    borg_note("# Twitchy!");

    /* Pick a random direction */
    dir = randint(9);

    /* Hack -- set goal */
    g_x = c_x + ddx[dir];
    g_y = c_y + ddy[dir];

    /* Move */
    borg_keypress('0' + dir);

    /* We did something */
    return (TRUE);
}





/*
 * Hack -- danger calculation hook
 */
static bool borg_danger_hook(int x, int y)
{
    int p;

    /* Determine danger */
    p = borg_danger(x, y);

    /* Ignore Non-Dangerous grids */
    if (p <= avoidance / 2) return (FALSE);

    /* Avoid this grid */
    return (TRUE);
}



/*
 * Commit the current "flow"
 */
static bool borg_flow_commit(cptr who, int why)
{
    int cost;
    
    /* Cost of current grid */
    cost = auto_data_cost->data[c_y][c_x];

    /* Verify the total "cost" */
    if (cost >= 250) return (FALSE);

    /* Message */
    if (who) borg_note(format("# Flowing toward %s at cost %d", who, cost));

    /* Obtain the "flow" information */
    COPY(auto_data_flow, auto_data_cost, auto_data);

    /* Save the goal type */
    goal = why;

    /* Success */
    return (TRUE);
}





/*
 * Attempt to take a step towards the current goal location, if any.
 *
 * If it works, return TRUE, otherwise, cancel the goal and return FALSE.
 */
bool borg_play_old_goal(void)
{
    int x, y;
        
    auto_grid *ag;


    /* Flow towards the goal */
    if (goal) {

        int b_n = 0;
                
        int i, b_i = 0;

        int here, best;

        /* Flow cost of current grid, minus one */
        best = auto_data_flow->data[c_y][c_x] - 1;
        
        /* Look around */
        for (i = 0; i < 8; i++) {

            /* Grid in that direction */
            x = c_x + ddx_ddd[i];
            y = c_y + ddy_ddd[i];

            /* Access the grid */
            ag = grid(x, y);
            
            /* Skip known walls */
            if (ag->o_c == '#') continue;
            if (ag->o_c == '%') continue;
            
            /* Flow cost at that grid */
            here = auto_data_flow->data[y][x];

            /* Ignore "icky" values */
            if (here > best) continue;

            /* Notice new best value */
            if (here < best) b_n = 0;
            
            /* Apply the randomizer */
            if ((++b_n > 1) && (rand_int(b_n) != 0)) continue;
            
            /* Track it */
            b_i = i; best = here;
        }

        /* Try it */
        if (b_n) {
        
            /* Access the location */
            x = c_x + ddx_ddd[b_i];
            y = c_y + ddy_ddd[b_i];

            /* Attempt motion */
            if (borg_play_step(x, y)) return (TRUE);
        }
        
        /* Cancel goal */
        goal = 0;
    }

    /* Nothing to do */
    return (FALSE);
}




/*
 * Prepare to flee the level via stairs
 */
bool borg_flow_stair_both(void)
{
    int i;

    /* None to flow to */
    if (!track_less_num && !track_more_num) return (FALSE);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_less_num; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_less_x[i], track_less_y[i]);
    }

    /* Enqueue useful grids */
    for (i = 0; i < track_more_num; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_more_x[i], track_more_y[i]);
    }

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Attempt to Commit the flow */
    return (borg_flow_commit("stairs", GOAL_FLEE));
}




/*
 * Prepare to flow towards "up" stairs
 */
bool borg_flow_stair_less(void)
{
    int i;

    /* None to flow to */
    if (!track_less_num) return (FALSE);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_less_num; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_less_x[i], track_less_y[i]);
    }

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Attempt to Commit the flow */
    return (borg_flow_commit("up-stairs", GOAL_MISC));
}


/*
 * Prepare to flow towards "down" stairs
 */
bool borg_flow_stair_more(void)
{
    int i;

    /* None to flow to */
    if (!track_more_num) return (FALSE);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_more_num; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_more_x[i], track_more_y[i]);
    }

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Attempt to Commit the flow */
    return (borg_flow_commit("down-stairs", GOAL_MISC));
}


/*
 * Prepare to "flow" towards any non-visited shop
 */
bool borg_flow_shop_visit(void)
{
    int i, x, y;
    
    /* Must be in town */
    if (auto_depth) return (FALSE);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Visit the shops */
    for (i = 0; i < 8; i++) {
    
        /* Must not be visited */
        if (auto_shops[i].when) continue;
        
        /* Obtain the location */
        x = track_shop_x[i];
        y = track_shop_y[i];

        /* Hack -- Must be known and not under the player */
        if (!x || !y || ((c_x == x) && (c_y == y))) continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(x, y);
    }

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Attempt to Commit the flow */
    return (borg_flow_commit("shops", GOAL_MISC));
}


/*
 * Prepare to "flow" towards a specific shop entry
 */
bool borg_flow_shop_entry(int i)
{
    int x, y;

    cptr name = (f_name + f_info[0x08+i].name);

    /* Must be in town */
    if (auto_depth) return (FALSE);

    /* Obtain the location */
    x = track_shop_x[i];
    y = track_shop_y[i];

    /* Hack -- Must be known */
    if (!x || !y) return (FALSE);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the grid */
    borg_flow_enqueue_grid(x, y);

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);

    /* Attempt to Commit the flow */
    return (borg_flow_commit(name, GOAL_MISC));
}






/*
 * Prepare to "flow" towards monsters to "kill"
 */
bool borg_flow_kill(bool viewable)
{
    int i, x, y, p, v1, v2;

    auto_grid *ag;


    /* Efficiency -- Nothing to kill */
    if (!auto_kills_cnt) return (FALSE);


    /* Nothing found */
    auto_temp_n = 0;

    /* Scan the monster list */
    for (i = 0; i < auto_kills_nxt; i++) {

        auto_kill *kill = &auto_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Access the location */
        x = kill->x;
        y = kill->y;

        /* Get the grid */
        ag = grid(x, y);

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Paranoia -- Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Hack -- Danger from physical attacks */
        v1 = borg_danger_aux1(x, y, i);

        /* Hack -- Danger from spell attacks */
        v2 = borg_danger_aux2(x, y, i);

        /* Assume the most damage possible */
        p = MAX(v1, v2);

        /* Hack -- Skip "deadly" monsters */
        if (p > avoidance / 2) continue;

        /* Careful -- Remember it */
        auto_temp_x[auto_temp_n] = x;
        auto_temp_y[auto_temp_n] = y;
        auto_temp_n++;
    }

    /* Nothing to kill */
    if (!auto_temp_n) return (FALSE);


    /* Hack -- ignoring monsters */
    if (goal_ignoring) {
        borg_note(format("# Ignoring %d monsters", auto_temp_n));
        auto_temp_n = 0;
        return (FALSE);
    }


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to kill */
    for (i = 0; i < auto_temp_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(auto_temp_x[i], auto_temp_y[i]);
    }

    /* Clear the seen array */
    auto_temp_n = 0;

    /* Spread the flow a little */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit(NULL, GOAL_KILL));
}



/*
 * Prepare to "flow" towards objects to "take"
 */
bool borg_flow_take(bool viewable)
{
    int i, x, y;

    auto_grid *ag;


    /* Efficiency -- Nothing to take */
    if (!auto_takes_cnt) return (FALSE);


    /* Nothing yet */
    auto_temp_n = 0;

    /* Scan the object list */
    for (i = 0; i < auto_takes_nxt; i++) {

        auto_take *take = &auto_takes[i];

        /* Skip dead objects */
        if (!take->k_idx) continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* Get the grid */
        ag = grid(x, y);

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Paranoia -- Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Careful -- Remember it */
        auto_temp_x[auto_temp_n] = x;
        auto_temp_y[auto_temp_n] = y;
        auto_temp_n++;
    }

    /* Nothing to take */
    if (!auto_temp_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < auto_temp_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(auto_temp_x[i], auto_temp_y[i]);
    }

    /* Clear the seen array */
    auto_temp_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit(NULL, GOAL_TAKE));
}


/*
 * Prepare to "flow" towards terrain to "handle"
 */
bool borg_flow_wank(bool viewable)
{
    int i, x, y;

    auto_grid *ag;


    /* Efficiency -- Nothing to take */
    if (!auto_wanks_cnt) return (FALSE);


    /* Nothing yet */
    auto_temp_n = 0;

    /* Scan the terrain list */
    for (i = 0; i < auto_wanks_nxt; i++) {

        auto_wank *wank = &auto_wanks[i];

        /* Skip dead terrain */
        if (!wank->f_idx) continue;

        /* Access the location */
        x = wank->x;
        y = wank->y;

        /* Get the grid */
        ag = grid(x, y);

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Paranoia -- Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* XXX XXX Hack -- must be strong to dig for gold */
        if ((ag->o_c == '*') &&
            (!borg_spell_okay(1,8) && (my_skill_dig < 25))) {

            continue;
        }

        /* XXX XXX Hack -- must be healthy to disarm traps */
        if ((ag->o_c == '^') &&
            (do_blind || do_confused || do_poisoned || do_image ||
             (auto_chp < auto_mhp))) {

            continue;
        }

        /* Careful -- Remember it */
        auto_temp_x[auto_temp_n] = x;
        auto_temp_y[auto_temp_n] = y;
        auto_temp_n++;
    }

    /* Nothing to take */
    if (!auto_temp_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < auto_temp_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(auto_temp_x[i], auto_temp_y[i]);
    }

    /* Clear the seen array */
    auto_temp_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit(NULL, GOAL_WANK));
}


/*
 * Prepare to "flow" towards "dark" or "unknown" grids
 */
bool borg_flow_dark()
{
    int i, j, x, y;

    auto_grid *ag;


    /* Nothing yet */
    auto_temp_n = 0;

    /* Look for something unknown */
    for (i = 0; i < auto_view_n; i++) {

        /* Access the "view grid" */
        y = auto_view_y[i];
        x = auto_view_x[i];

        /* Get the auto_grid */
        ag = grid(x, y);

        /* Skip the player grid */
        if ((c_x == x) && (c_y == y)) continue;

        /* Skip unknown grids */
        if (ag->o_c == ' ') continue;

        /* Cannot explore walls */
        if (ag->info & BORG_WALL) continue;

        /* Scan all eight neighbors */
        for (j = 0; j < 8; j++) {

            /* Get the location */
            int x2 = x + ddx_ddd[j];
            int y2 = y + ddy_ddd[j];

            /* Get the grid */
            auto_grid *ag2 = grid(x2, y2);

            /* Unknown grids are interesting */
            if (ag2->o_c == ' ') break;
        }

        /* Skip "boring" grids */
        if (j == 8) continue;

        /* Careful -- Remember it */
        auto_temp_x[auto_temp_n] = x;
        auto_temp_y[auto_temp_n] = y;
        auto_temp_n++;
    }

    /* Nothing dark */
    if (!auto_temp_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < auto_temp_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(auto_temp_x[i], auto_temp_y[i]);
    }

    /* Clear the seen array */
    auto_temp_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit(NULL, GOAL_DARK));
}


/*
 * Prepare to "flow" towards "interesting" things
 */
bool borg_flow_explore(void)
{
    int i, j, x, y;

    auto_grid *ag;


    /* Hack -- not in town */
    if (!auto_depth) return (FALSE);
    
    
    /* Nothing yet */
    auto_temp_n = 0;

    /* Examine every "legal" grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Get the grid */
            ag = grid(x, y);

            /* Skip the player grid */
            if ((c_x == x) && (c_y == y)) continue;

            /* Skip unknowns */
            if (ag->o_c == ' ') continue;

            /* Cannot explore walls */
            if (ag->info & BORG_WALL) continue;

            /* Scan all eight neighbors */
            for (j = 0; j < 8; j++) {

                /* Get the location */
                int x2 = x + ddx_ddd[j];
                int y2 = y + ddy_ddd[j];

                /* Get the grid */
                auto_grid *ag2 = grid(x2, y2);

                /* Unknown grids are interesting */
                if (ag2->o_c == ' ') break;
            }

            /* Skip "boring" grids */
            if (j == 8) continue;

            /* Careful -- Remember it */
            auto_temp_x[auto_temp_n] = x;
            auto_temp_y[auto_temp_n] = y;
            auto_temp_n++;

            /* Paranoia -- Check for overflow */
            if (auto_temp_n == AUTO_SEEN_MAX) {

                /* Hack -- Double break */
                y = AUTO_MAX_Y;
                x = AUTO_MAX_X;
                break;
            }
        }
    }

    /* Nothing useful */
    if (!auto_temp_n) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < auto_temp_n; i++) {

        /* Enqueue the grid */
        borg_flow_enqueue_grid(auto_temp_x[i], auto_temp_y[i]);
    }

    /* Clear the seen array */
    auto_temp_n = 0;

    /* Spread the flow (a little) */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit("unknowns", GOAL_DARK));
}






/*
 * Walk around the dungeon looking for monsters
 */
bool borg_flow_revisit(void)
{
    int x, y;

    int p;
    
    int cost;
    
    int r_n = 0;

    int r_x = 0;
    int r_y = 0;

    auto_grid *ag;


    /* Hack -- not in town */
    if (!auto_depth) return (FALSE);
    
    
    /* Look around */
    p = borg_danger(c_x, c_y);

    /* Hack -- careful of danger */
    if (p > avoidance / 4) return (FALSE);


    /* Reverse flow */
    borg_flow_reverse();

    /* Examine every legal grid */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Obtain the cost */
            cost = auto_data_cost->data[y][x];
            
            /* Skip "unreachable" grids */
            if (cost >= 250) continue;

            /* Hack -- Skip "nearby" grids */
            if (cost < 50) continue;

            /* Get the grid */
            ag = grid(x, y);

            /* Hack -- Skip "visited" grids */
            if (ag->xtra) continue;

            /* Apply the randomizer */
            if ((++r_n > 1) && (rand_int(r_n) != 0)) continue;

            /* Careful -- Remember it */
            r_x = x;
            r_y = y;
        }
    }

    /* Nothing to revisit */
    if (r_n <= 0) return (FALSE);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the grid */
    borg_flow_enqueue_grid(r_x, r_y);

    /* Spread the flow a little */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit("old grids", GOAL_XTRA));
}


/*
 * Heuristic -- Help locate secret doors (see below)
 *
 * Determine the "likelihood" of a secret door touching "(x,y)",
 * which is "d" grids away from the player.
 *
 * Assume grid is legal, non-wall, and reachable.
 * Assume grid neighbors are legal and known.
 */
static int borg_secrecy(int x, int y, int d, bool bored)
{
    int		i, v;
    int		wall = 0, supp = 0, diag = 0;

    char	cc[8];

    auto_grid	*ag;


    /* No secret doors in town */
    if (!auto_depth) return (0);


    /* Get the central grid */
    ag = grid(x, y);

    /* Tweak -- Limit total searches */
    if (ag->xtra > 50) return (0);


    /* Extract adjacent locations */
    for (i = 0; i < 8; i++) {

        /* Extract the location */
        int xx = x + ddx_ddd[i];
        int yy = y + ddy_ddd[i];

        /* Get the grid */
        cc[i] = grid(xx,yy)->o_c;
    }


    /* Count possible door locations */
    for (i = 0; i < 4; i++) if (cc[i] == '#') wall++;

    /* No possible secret doors */
    if (wall <= 0) return (0);


    /* Count supporting evidence for secret doors */
    for (i = 0; i < 4; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) supp++;
        else if ((cc[i] == '+') || (cc[i] == '\'')) supp++;
    }

    /* Count supporting evidence for secret doors */
    for (i = 4; i < 8; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) diag++;
    }


    /* Tweak -- Reward walls, punish visitation and distance */
    v = (supp * 300) + (diag * 100) - (ag->xtra * 20) - (d * 1);

    /* Hack -- Tweak -- Reward "boredom" */
    if (bored) v += 500;

    /* Result */
    return (v);
}



/*
 * Search carefully for secret doors and such
 */
bool borg_flow_spastic(bool bored)
{
    int x, y, v;

    int b_x = c_x;
    int b_y = c_y;
    int b_v = -1;

    int p;
    int cost;

    int boredom;

    auto_grid *ag;


    /* Hack -- not in town */
    if (!auto_depth) return (FALSE);
    
    
    /* Hack -- Determine "boredom" */
    boredom = count_floor - 500;

    /* Hack -- Tweak -- Maximal boredom */
    if (boredom > 800) boredom = 800;

    /* Hack -- Tweak -- Minimal boredom (unless bored) */
    if (!bored && (boredom < 200)) boredom = 200;

    /* Hack -- Only spastic when bored */
    if (c_t - auto_shock < boredom) return (FALSE);


    /* Look around */
    p = borg_danger(c_x, c_y);

    /* Hack -- careful of danger */
    if (p > avoidance / 4) return (FALSE);


    /* Reverse flow */
    borg_flow_reverse();

    /* Scan the entire map */
    for (y = 1; y < AUTO_MAX_Y-1; y++) {
        for (x = 1; x < AUTO_MAX_X-1; x++) {

            /* Acquire the grid */
            ag = grid(x, y);

            /* Skip unknown grids */
            if (ag->o_c == ' ') continue;

            /* Skip walls */
            if (ag->info & BORG_WALL) continue;

            /* Acquire the cost */
            cost = auto_data_cost->data[y][x];
            
            /* Skip "unreachable" grids */
            if (cost >= 250) continue;

            /* Get the "searchability" */
            v = borg_secrecy(x, y, cost, bored);

            /* The grid is not searchable */
            if (v <= 0) continue;

            /* Skip non-perfect grids */
            if ((b_v >= 0) && (v < b_v)) continue;

            /* Save the data */
            b_v = v; b_x = x; b_y = y;
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (b_v < 0) return (FALSE);


    /* We have arrived */
    if ((c_x == b_x) && (c_y == b_y)) {

        /* Acquire the grid */
        ag = grid(c_x,c_y);
        
        /* Take note */
        borg_note(format("# Spastic Searching at (%d,%d)...", c_x, c_y));

        /* Tweak -- Remember the search */
        if (ag->xtra < 100) ag->xtra += 9;

        /* Tweak -- Search a little */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }


    /* Enqueue the grid */
    borg_flow_enqueue_grid(b_x, b_y);

    /* Spread the flow a little */
    borg_flow_spread(TRUE);


    /* Attempt to Commit the flow */
    return (borg_flow_commit("spastic", GOAL_XTRA));
}




/*
 * Sorting hook -- comp function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
static bool ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
    cptr *text = (cptr*)(u);
    s16b *what = (s16b*)(v);

    int cmp;
    
    /* Compare the two strings */
    cmp = (strcmp(text[a], text[b]));
    
    /* Strictly less */
    if (cmp < 0) return (TRUE);
    
    /* Strictly more */
    if (cmp > 0) return (FALSE);
    
    /* Enforce "stable" sort */
    return (what[a] <= what[b]);
}


/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
static void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
    cptr *text = (cptr*)(u);
    s16b *what = (s16b*)(v);

    cptr texttmp;
    s16b whattmp;
    
    /* Swap "text" */
    texttmp = text[a];
    text[a] = text[b];
    text[b] = texttmp;

    /* Swap "what" */
    whattmp = what[a];
    what[a] = what[b];
    what[b] = whattmp;
}



/*
 * Initialize this file
 */
void borg_ext_init(void)
{
    int i;

    byte t_a;

    char buf[80];

    int size;
    
    s16b what[512];
    cptr text[512];


    /*** Hack -- assign the "danger" hook ***/

    /* Assign the "danger" hook */
    auto_danger_hook = borg_danger_hook;


    /*** Hack -- Extract race ***/

    /* Check for textual race */
    if (0 == borg_what_text(COL_RACE, ROW_RACE, -12, &t_a, buf)) {

        /* Scan the races */
        for (i = 0; i < 10; i++) {

            /* Check the race */
            if (prefix(buf, race_info[i].title)) {

                /* We got one */
                auto_race = i;
                break;
            }
        }
    }

    /* Extract the race pointer */
    rb_ptr = &race_info[auto_race];


    /*** Hack -- Extract class ***/

    /* Check for textual class */
    if (0 == borg_what_text(COL_CLASS, ROW_CLASS, -12, &t_a, buf)) {

        /* Scan the classes */
        for (i = 0; i < 6; i++) {

            /* Check the race */
            if (prefix(buf, class_info[i].title)) {

                /* We got one */
                auto_class = i;
                break;
            }
        }
    }

    /* Extract the class pointer */
    cb_ptr = &class_info[auto_class];

    /* Extract the magic pointer */
    mb_ptr = &magic_info[auto_class];


    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    prepare_race_class_info();


    /*** Very special "tracking" array ***/

    /* Track the shop locations */
    C_MAKE(track_shop_x, 8, byte);
    C_MAKE(track_shop_y, 8, byte);


    /*** Special "tracking" arrays ***/

    /* Track "up" stairs */
    track_less_num = 0;
    track_less_size = 16;
    C_MAKE(track_less_x, track_less_size, byte);
    C_MAKE(track_less_y, track_less_size, byte);

    /* Track "down" stairs */
    track_more_num = 0;
    track_more_size = 16;
    C_MAKE(track_more_x, track_more_size, byte);
    C_MAKE(track_more_y, track_more_size, byte);


    /*** Terrain tracking ***/
    
    /* Terrain checker */
    C_MAKE(auto_is_wank, 256, bool);

    /* No terrain yet */
    auto_wanks_cnt = 0;
    auto_wanks_nxt = 0;

    /* Allow some terrain */
    auto_wanks_max = 256;

    /* Array of terrain */
    C_MAKE(auto_wanks, auto_wanks_max, auto_wank);

    /* Hack -- wank "closed doors" */
    auto_is_wank[(byte)('+')] = TRUE;
        
    /* Hack -- wank "hidden gold" */
    auto_is_wank[(byte)('*')] = TRUE;

    /* Hack -- wank "rubble" */
    auto_is_wank[(byte)(':')] = TRUE;
        
    /* Hack -- wank "traps" */
    auto_is_wank[(byte)('^')] = TRUE;


    /*** Object tracking ***/

    /* Object checker */
    C_MAKE(auto_is_take, 256, bool);

    /* No objects yet */
    auto_takes_cnt = 0;
    auto_takes_nxt = 0;

    /* Allow some objects */
    auto_takes_max = 256;

    /* Array of objects */
    C_MAKE(auto_takes, auto_takes_max, auto_take);

    /* Scan the objects */
    for (i = 1; i < MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Skip non-items */
        if (!k_ptr->name) continue;

        /* Notice this object */
        auto_is_take[(byte)(k_ptr->x_char)] = TRUE;
    }


    /*** Monster tracking ***/

    /* No monsters yet */
    auto_kills_cnt = 0;
    auto_kills_nxt = 0;

    /* Allow some monsters */
    auto_kills_max = 512;

    /* Array of monsters */
    C_MAKE(auto_kills, auto_kills_max, auto_kill);

    /* Monster checker */
    C_MAKE(auto_is_kill, 256, bool);

    /* Scan the monsters */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;

        /* Hack -- Skip "mimic" monsters XXX XXX XXX */
        if (r_ptr->flags1 & RF1_CHAR_MULTI) continue;

        /* Hack -- Skip "clear" monsters XXX XXX XXX */
        if (r_ptr->flags1 & RF1_CHAR_CLEAR) continue;

        /* Notice this monster */
        auto_is_kill[(byte)(r_ptr->l_char)] = TRUE;
    }


    /*** Message tracking ***/
    
    /* No chars saved yet */
    auto_msg_len = 0;

    /* Maximum buffer size */
    auto_msg_siz = 4096;
    
    /* Allocate a buffer */
    C_MAKE(auto_msg_buf, auto_msg_siz, char);
    
    /* No msg's saved yet */
    auto_msg_num = 0;

    /* Maximum number of messages */
    auto_msg_max = 128;
    
    /* Allocate array of positions */
    C_MAKE(auto_msg_pos, auto_msg_max, s16b);
    
    /* Allocate array of use-types */
    C_MAKE(auto_msg_use, auto_msg_max, s16b);
    
    
    /*** Special counters ***/

    /* Count racial appearances */
    C_MAKE(auto_race_count, MAX_R_IDX, s16b);

    /* Count racial deaths */
    C_MAKE(auto_race_death, MAX_R_IDX, s16b);


    /*** Parse "unique" monster names ***/
    
    /* Start over */
    size = 0;
    
    /* Collect "unique" monsters */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;

        /* Skip non-unique monsters */
        if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;

        /* Use it */
        text[size] = r_name + r_ptr->name;
        what[size] = i;
        size++;
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort */
    ang_sort(text, what, size);

    /* Save the size */
    auto_unique_size = size;

    /* Allocate the arrays */
    C_MAKE(auto_unique_text, auto_unique_size, cptr);
    C_MAKE(auto_unique_what, auto_unique_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_unique_text[i] = text[i];
    for (i = 0; i < size; i++) auto_unique_what[i] = what[i];


    /*** Parse "normal" monster names ***/
    
    /* Start over */
    size = 0;
    
    /* Collect "normal" monsters */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;

        /* Skip unique monsters */
        if (r_ptr->flags1 & RF1_UNIQUE) continue;

        /* Use it */
        text[size] = r_name + r_ptr->name;
        what[size] = i;
        size++;
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort */
    ang_sort(text, what, size);

    /* Save the size */
    auto_normal_size = size;

    /* Allocate the arrays */
    C_MAKE(auto_normal_text, auto_normal_size, cptr);
    C_MAKE(auto_normal_what, auto_normal_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_normal_text[i] = text[i];
    for (i = 0; i < size; i++) auto_normal_what[i] = what[i];


    /*** XXX XXX XXX Hack -- Cheat ***/
    
    /* Hack -- Extract dead uniques */
    for (i = 1; i < MAX_R_IDX-1; i++) {

        monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name) continue;

        /* Skip non-uniques */
        if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;

        /* Mega-Hack -- Access "dead unique" list */
        if (r_ptr->max_num == 0) auto_race_death[i] = 1;
    }
    
    /* Hack -- Access max depth */
    auto_max_depth = p_ptr->max_dlv;
}



#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

