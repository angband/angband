/* File: borg-ext.h */

/* Purpose: Header file for "borg-ext.c" -BEN- */

#ifndef INCLUDED_BORG_EXT_H
#define INCLUDED_BORG_EXT_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg-ext.c".
 */

#include "borg.h"

#include "borg-map.h"

#include "borg-obj.h"



/*
 * Possible values of "goal"
 */
#define GOAL_KILL	11		/* Monster */
#define GOAL_TAKE	12		/* Object */
#define GOAL_MISC	13		/* Stairs/Stores */
#define GOAL_DARK	14		/* Darkness */
#define GOAL_XTRA	15		/* Interesting Grids */
#define GOAL_FLEE	16		/* Fleeing */
#define GOAL_WANK	17		/* Terrain */


/*
 * Minimum "harmless" food
 */

#define SV_FOOD_MIN_OKAY	SV_FOOD_CURE_POISON


/*
 * Terrain information
 */

typedef struct auto_wank auto_wank;

struct auto_wank {

    s16b	f_idx;		/* Feature index */
    
    byte	o_a;		/* Recent attr */
    char	o_c;		/* Recent char */

    bool	known;		/* Feature is not a guess */

    bool	extra;		/* Unused */

    byte	x, y;		/* Location */

    s32b	when;		/* When last seen */
};


/*
 * Object information
 */

typedef struct auto_take auto_take;

struct auto_take {

    s16b	k_idx;		/* Kind index */
    
    byte	o_a;		/* Recent attr */
    char	o_c;		/* Recent char */

    bool	known;		/* Kind is not a guess */

    bool	extra;		/* Unused */

    byte	x, y;		/* Location */

    s32b	when;		/* When last seen */
};


/*
 * Monster information
 */

typedef struct auto_kill auto_kill;

struct auto_kill {

    s16b	r_idx;		/* Race index */    

    byte	o_a;		/* Recent attr */
    char	o_c;		/* Recent char */

    bool	known;		/* Verified race */

    bool	awake;		/* Probably awake */

    byte	x, y;		/* Location */

    byte	ox, oy;		/* Old location */

    byte	speed;		/* Estimated speed */
    byte	moves;		/* Estimates moves */

    s16b	power;		/* Estimated hit-points */
    s16b	other;		/* Estimated something */

    s32b	when;		/* When last seen */
};



/*
 * Some information
 */

extern s16b goal;		/* Flowing (goal type) */

extern bool goal_rising;	/* Currently returning to town */

extern bool goal_fleeing;	/* Currently fleeing the level */

extern bool goal_ignoring;	/* Currently ignoring monsters */

extern bool goal_recalling;	/* Currently waiting for recall */

extern bool stair_less;		/* Use the next "up" staircase */
extern bool stair_more;		/* Use the next "down" staircase */

extern s32b auto_began;		/* When this level began */

extern s32b auto_shock;		/* When last "shocked" */

extern s16b avoidance;		/* Current danger thresh-hold */


/*
 * Shop goals
 */

extern s16b goal_shop;		/* Next shop to visit */
extern s16b goal_ware;		/* Next item to buy there */
extern s16b goal_item;		/* Next item to sell there */


/*
 * Hack -- panic flags
 */

extern bool panic_death;		/* Panic before Death */

extern bool panic_stuff;		/* Panic before Junking Stuff */

extern bool panic_power;		/* Assume monsters get constant spells */



/*
 * Some estimated state variables
 */

extern s16b my_stat_max[6];	/* Current "maximal" stat values	*/
extern s16b my_stat_cur[6];	/* Current "natural" stat values	*/
extern s16b my_stat_use[6];	/* Current "resulting" stat values	*/
extern s16b my_stat_ind[6];	/* Current "additions" to stat values	*/

extern s16b my_ac;		/* Base armor		*/
extern s16b my_to_ac;		/* Plusses to ac	*/
extern s16b my_to_hit;		/* Plusses to hit	*/
extern s16b my_to_dam;		/* Plusses to dam	*/

extern byte my_immune_acid;	/* Immunity to acid		*/
extern byte my_immune_elec;	/* Immunity to lightning	*/
extern byte my_immune_fire;	/* Immunity to fire		*/
extern byte my_immune_cold;	/* Immunity to cold		*/
extern byte my_immune_pois;	/* Immunity to poison		*/

extern byte my_resist_acid;	/* Resist acid		*/
extern byte my_resist_elec;	/* Resist lightning	*/
extern byte my_resist_fire;	/* Resist fire		*/
extern byte my_resist_cold;	/* Resist cold		*/
extern byte my_resist_pois;	/* Resist poison	*/

extern byte my_resist_conf;	/* Resist confusion	*/
extern byte my_resist_sound;	/* Resist sound		*/
extern byte my_resist_lite;	/* Resist light		*/
extern byte my_resist_dark;	/* Resist darkness	*/
extern byte my_resist_chaos;	/* Resist chaos		*/
extern byte my_resist_disen;	/* Resist disenchant	*/
extern byte my_resist_shard;	/* Resist shards	*/
extern byte my_resist_nexus;	/* Resist nexus		*/
extern byte my_resist_blind;	/* Resist blindness	*/
extern byte my_resist_neth;	/* Resist nether	*/
extern byte my_resist_fear;	/* Resist fear		*/

extern byte my_sustain_str;	/* Keep strength	*/
extern byte my_sustain_int;	/* Keep intelligence	*/
extern byte my_sustain_wis;	/* Keep wisdom		*/
extern byte my_sustain_dex;	/* Keep dexterity	*/
extern byte my_sustain_con;	/* Keep constitution	*/
extern byte my_sustain_chr;	/* Keep charisma	*/

extern byte my_aggravate;	/* Aggravate monsters	*/
extern byte my_teleport;	/* Random teleporting	*/

extern byte my_ffall;		/* No damage falling	*/
extern byte my_lite;		/* Permanent light	*/
extern byte my_free_act;	/* Never paralyzed	*/
extern byte my_see_inv;		/* Can see invisible	*/
extern byte my_regenerate;	/* Regenerate hit pts	*/
extern byte my_hold_life;	/* Resist life draining	*/
extern byte my_telepathy;	/* Telepathy		*/
extern byte my_slow_digest;	/* Slower digestion	*/

extern s16b my_see_infra;	/* Infravision range	*/

extern s16b my_skill_dis;	/* Skill: Disarming		*/
extern s16b my_skill_dev;	/* Skill: Magic Devices		*/
extern s16b my_skill_sav;	/* Skill: Saving throw		*/
extern s16b my_skill_stl;	/* Skill: Stealth factor	*/
extern s16b my_skill_srh;	/* Skill: Searching ability	*/
extern s16b my_skill_fos;	/* Skill: Searching frequency	*/
extern s16b my_skill_thn;	/* Skill: Combat (normal)	*/
extern s16b my_skill_thb;	/* Skill: Combat (shooting)	*/
extern s16b my_skill_tht;	/* Skill: Combat (throwing)	*/
extern s16b my_skill_dig;	/* Skill: Digging		*/

extern s16b my_num_blow;	/* Number of blows	*/
extern s16b my_num_fire;	/* Number of shots	*/

extern s16b my_cur_lite;	/* Radius of lite	*/
extern s16b my_cur_view;	/* Radius of view	*/

extern s16b my_speed;		/* Approximate speed	*/
extern s16b my_other;		/* Approximate other	*/

extern byte my_ammo_tval;	/* Ammo -- "tval"	*/
extern byte my_ammo_sides;	/* Ammo -- "sides"	*/
extern s16b my_ammo_power;	/* Shooting multipler	*/
extern s16b my_ammo_range;	/* Shooting range	*/


/*
 * Amounts of ability in the pack
 */

extern s16b amt_fuel;
extern s16b amt_food;
extern s16b amt_ident;
extern s16b amt_recall;
extern s16b amt_escape;
extern s16b amt_teleport;

extern s16b amt_cure_critical;
extern s16b amt_cure_serious;

extern s16b amt_detect_trap;
extern s16b amt_detect_door;

extern s16b amt_missile;

extern s16b amt_book[9];

extern s16b amt_add_stat[6];
extern s16b amt_fix_stat[6];

extern s16b amt_fix_exp;

extern s16b amt_enchant_to_a;
extern s16b amt_enchant_to_d;
extern s16b amt_enchant_to_h;


/*
 * Amounts of ability in the home
 */

extern s16b num_fuel;
extern s16b num_food;
extern s16b num_ident;
extern s16b num_recall;
extern s16b num_escape;
extern s16b num_teleport;

extern s16b num_cure_critical;
extern s16b num_cure_serious;

extern s16b num_missile;

extern s16b num_book[9];

extern s16b num_fix_stat[6];

extern s16b num_fix_exp;

extern s16b num_enchant_to_a;
extern s16b num_enchant_to_d;
extern s16b num_enchant_to_h;



/*
 * Some more variables
 */

extern s16b when_art_lite;	/* When we last activated the Light */

extern s16b when_call_lite;	/* When we last called light */

extern s16b when_traps;		/* When we last detected traps */
extern s16b when_doors;		/* When we last detected doors */


extern s16b auto_need_enchant_to_a;	/* Need some enchantment */
extern s16b auto_need_enchant_to_h;	/* Need some enchantment */
extern s16b auto_need_enchant_to_d;	/* Need some enchantment */



/*
 * The feature list.  This list is used to "track" features.
 */

extern s16b auto_wanks_cnt;

extern s16b auto_wanks_nxt;

extern s16b auto_wanks_max;

extern auto_wank *auto_wanks;


/*
 * The object list.  This list is used to "track" objects.
 */

extern s16b auto_takes_cnt;

extern s16b auto_takes_nxt;

extern s16b auto_takes_max;

extern auto_take *auto_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

extern s16b auto_kills_cnt;

extern s16b auto_kills_nxt;

extern s16b auto_kills_max;

extern auto_kill *auto_kills;


/*
 * Hack -- message memory
 */

extern s16b auto_msg_len;

extern s16b auto_msg_siz;

extern char *auto_msg_buf;

extern s16b auto_msg_num;

extern s16b auto_msg_max;

extern s16b *auto_msg_pos;

extern s16b *auto_msg_use;




/*
 * Update state based on current "map"
 */
extern void borg_update(void);


/*
 * React to various "important" messages
 */
extern void borg_react(cptr msg, cptr buf);


/*
 * Low level goals
 */
extern bool borg_caution(void);
extern bool borg_attack(void);
extern bool borg_launch(void);
extern bool borg_recover(void);

/*
 * Low level goals
 */
extern bool borg_inspect(void);

/*
 * Twitchy goals
 */
extern bool borg_charge_kill(void);
extern bool borg_charge_take(void);
extern bool borg_twitchy(void);



/*
 * Continue a high level goal
 */
extern bool borg_play_old_goal(void);

/*
 * Flee the level
 */
extern bool borg_flow_stair_both(void);

/*
 * Flow to stairs
 */
extern bool borg_flow_stair_less(void);
extern bool borg_flow_stair_more(void);

/*
 * Flow to shops
 */
extern bool borg_flow_shop_visit(void);
extern bool borg_flow_shop_entry(int n);

/*
 * Flow towards monsters/objects/terrain
 */
extern bool borg_flow_kill(bool viewable);
extern bool borg_flow_take(bool viewable);
extern bool borg_flow_wank(bool viewable);

/*
 * Flow towards "interesting" grids
 */
extern bool borg_flow_dark(void);
extern bool borg_flow_explore(void);
extern bool borg_flow_revisit(void);
extern bool borg_flow_spastic(bool bored);



/*
 * Init this file
 */
extern void borg_ext_init(void);




#endif

#endif

