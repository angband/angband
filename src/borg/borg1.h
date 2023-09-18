
/* File: borg1.h */
/* Purpose: Header file for "borg1.c" -BEN- */

#ifndef INCLUDED_BORG1_H
#define INCLUDED_BORG1_H

#include "angband.h"
#include "init.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "cave.h"
#include "monster.h"
#include "player.h"
#include "player-timed.h"
#include "ui-menu.h"
#include "ui-term.h"


#ifdef ALLOW_BORG

/* Mega-Hack - indices of the player classes */
#define CLASS_WARRIOR		0
#define CLASS_MAGE          1
#define CLASS_DRUID			2
#define CLASS_PRIEST        3
#define CLASS_NECROMANCER	4
#define CLASS_PALADIN		5
#define CLASS_ROGUE         6
#define CLASS_RANGER        7
#define CLASS_BLACKGUARD	8

#define MAX_CLASSES			9 /* Max # of classes 0 = warrior, 5 = Paladin */

#include "../effects.h"

/*
* Indexes used for various "equipment" slots (hard-coded by savefiles, etc).
*/
#define INVEN_WIELD		z_info->pack_size
#define INVEN_BOW       (z_info->pack_size+1)
#define INVEN_RIGHT     (z_info->pack_size+2)
#define INVEN_LEFT      (z_info->pack_size+3)
#define INVEN_NECK      (z_info->pack_size+4)
#define INVEN_LIGHT     (z_info->pack_size+5)
#define INVEN_BODY      (z_info->pack_size+6)
#define INVEN_OUTER     (z_info->pack_size+7)
#define INVEN_ARM       (z_info->pack_size+8)
#define INVEN_HEAD      (z_info->pack_size+9)
#define INVEN_HANDS     (z_info->pack_size+10)
#define INVEN_FEET      (z_info->pack_size+11)

/*
 * Total number of inventory slots.
 */
#define INVEN_TOTAL	(z_info->pack_size+12)

/*
 * Total number of pack slots available (not used by quiver) 
 */
#define PACK_SLOTS (z_info->pack_size - borg_skill[BI_QUIVER_SLOTS])

 /* Quiver */
#define QUIVER_START 	INVEN_TOTAL
#define QUIVER_SIZE  	(z_info->quiver_size)
#define QUIVER_END   	(QUIVER_START+QUIVER_SIZE)


/* s_val's now dynamically loaded */
extern int sv_food_apple;
extern int sv_food_ration;
extern int sv_food_slime_mold;
extern int sv_food_draught;
extern int sv_food_pint;
extern int sv_food_sip;
extern int sv_food_waybread;
extern int sv_food_honey_cake;
extern int sv_food_slice;
extern int sv_food_handful;

extern int sv_mush_second_sight;
extern int sv_mush_fast_recovery;
extern int sv_mush_restoring;
extern int sv_mush_mana;
extern int sv_mush_emergency;
extern int sv_mush_terror;
extern int sv_mush_stoneskin;
extern int kv_mush_stoneskin;
extern int sv_mush_debility;
extern int sv_mush_sprinting;
extern int sv_mush_cure_mind;
extern int sv_mush_purging;

extern int sv_light_lantern;
extern int sv_light_torch;

extern int sv_flask_oil;
extern int kv_flask_oil;

extern int sv_potion_cure_critical;
extern int sv_potion_cure_serious;
extern int sv_potion_cure_light;
extern int sv_potion_healing;
extern int kv_potion_healing;
extern int sv_potion_star_healing;
extern int sv_potion_life;
extern int sv_potion_restore_mana;
extern int kv_potion_restore_mana;
extern int sv_potion_cure_poison;
extern int sv_potion_resist_heat;
extern int sv_potion_resist_cold;
extern int sv_potion_resist_pois;
extern int sv_potion_inc_str;
extern int sv_potion_inc_int;
extern int sv_potion_inc_wis;
extern int sv_potion_inc_dex;
extern int sv_potion_inc_con;
extern int sv_potion_inc_str2;
extern int sv_potion_inc_int2;
extern int sv_potion_inc_wis2;
extern int sv_potion_inc_dex2;
extern int sv_potion_inc_con2;
extern int sv_potion_inc_all;
extern int sv_potion_restore_life;
extern int sv_potion_speed;
extern int sv_potion_berserk;
extern int sv_potion_sleep;
extern int sv_potion_slowness;
extern int sv_potion_poison;
extern int sv_potion_blindness;
extern int sv_potion_confusion;
extern int sv_potion_heroism;
extern int sv_potion_boldness;
extern int sv_potion_detect_invis;
extern int sv_potion_enlightenment;
extern int sv_potion_slime_mold;
extern int sv_potion_infravision;
extern int sv_potion_inc_exp;

extern int sv_scroll_identify;
extern int sv_scroll_phase_door;
extern int sv_scroll_teleport;
extern int sv_scroll_word_of_recall;
extern int sv_scroll_enchant_armor;
extern int sv_scroll_enchant_weapon_to_hit;
extern int sv_scroll_enchant_weapon_to_dam;
extern int sv_scroll_star_enchant_weapon;
extern int sv_scroll_star_enchant_armor;
extern int sv_scroll_protection_from_evil;
extern int sv_scroll_rune_of_protection;
extern int sv_scroll_teleport_level;
extern int sv_scroll_recharging;
extern int sv_scroll_banishment;
extern int sv_scroll_mass_banishment;
extern int kv_scroll_mass_banishment;
extern int sv_scroll_blessing;
extern int sv_scroll_holy_chant;
extern int sv_scroll_holy_prayer;
extern int sv_scroll_detect_invis;
extern int sv_scroll_satisfy_hunger;
extern int sv_scroll_light;
extern int sv_scroll_mapping;
extern int sv_scroll_acquirement;
extern int sv_scroll_star_acquirement;
extern int sv_scroll_remove_curse;
extern int kv_scroll_remove_curse;
extern int sv_scroll_star_remove_curse;
extern int kv_scroll_star_remove_curse;
extern int sv_scroll_monster_confusion;
extern int sv_scroll_trap_door_destruction;
extern int sv_scroll_dispel_undead;

extern int sv_ring_flames;
extern int sv_ring_ice;
extern int sv_ring_acid;
extern int sv_ring_lightning;
extern int sv_ring_digging;
extern int sv_ring_speed;
extern int sv_ring_damage;
extern int sv_ring_dog;

extern int sv_amulet_teleportation;

extern int sv_rod_recall;
extern int kv_rod_recall;
extern int sv_rod_detection;
extern int sv_rod_illumination;
extern int sv_rod_speed;
extern int sv_rod_mapping;
extern int sv_rod_healing;
extern int kv_rod_healing;
extern int sv_rod_light;
extern int sv_rod_fire_bolt;
extern int sv_rod_elec_bolt;
extern int sv_rod_cold_bolt;
extern int sv_rod_acid_bolt;
extern int sv_rod_drain_life;
extern int sv_rod_fire_ball;
extern int sv_rod_elec_ball;
extern int sv_rod_cold_ball;
extern int sv_rod_acid_ball;
extern int sv_rod_teleport_other;
extern int sv_rod_slow_monster;
extern int sv_rod_sleep_monster;
extern int sv_rod_curing;

extern int sv_staff_teleportation;
extern int sv_staff_destruction;
extern int sv_staff_speed;
extern int sv_staff_healing;
extern int sv_staff_the_magi;
extern int sv_staff_power;
extern int sv_staff_curing;
extern int sv_staff_holiness;
extern int kv_staff_holiness;
extern int sv_staff_sleep_monsters;
extern int sv_staff_slow_monsters;
extern int sv_staff_detect_invis;
extern int sv_staff_detect_evil;
extern int sv_staff_dispel_evil;
extern int sv_staff_banishment;
extern int sv_staff_light;
extern int sv_staff_mapping;
extern int sv_staff_remove_curse;

extern int sv_wand_light;
extern int sv_wand_teleport_away;
extern int sv_wand_stinking_cloud;
extern int kv_wand_stinking_cloud;
extern int sv_wand_magic_missile;
extern int kv_wand_magic_missile;
extern int sv_wand_annihilation;
extern int kv_wand_annihilation;
extern int sv_wand_stone_to_mud;
extern int sv_wand_wonder;
extern int sv_wand_hold_monster;
extern int sv_wand_slow_monster;
extern int sv_wand_fear_monster;
extern int sv_wand_confuse_monster;
extern int sv_wand_fire_bolt;
extern int sv_wand_cold_bolt;
extern int sv_wand_acid_bolt;
extern int sv_wand_elec_bolt;
extern int sv_wand_fire_ball;
extern int sv_wand_cold_ball;
extern int sv_wand_acid_ball;
extern int sv_wand_elec_ball;
extern int sv_wand_dragon_cold;
extern int sv_wand_dragon_fire;
extern int sv_wand_drain_life;

extern int sv_dagger;

extern int sv_sling;
extern int sv_short_bow;
extern int sv_long_bow;
extern int sv_light_xbow;
extern int sv_heavy_xbow;

extern int sv_arrow_seeker;
extern int sv_arrow_mithril;

extern int sv_bolt_seeker;
extern int sv_bolt_mithril;

extern int sv_set_of_leather_gloves;

extern int sv_cloak;

extern int sv_robe;

extern int sv_iron_crown;

extern int sv_dragon_blue;
extern int sv_dragon_black;
extern int sv_dragon_white;
extern int sv_dragon_red;
extern int sv_dragon_green;
extern int sv_dragon_multihued;
extern int sv_dragon_shining;
extern int sv_dragon_law;
extern int sv_dragon_gold;
extern int sv_dragon_chaos;
extern int sv_dragon_balance;
extern int sv_dragon_power;

/* dynamically loaded activations */
extern int act_dragon_power;
extern int act_dragon_shining;
extern int act_dragon_balance;
extern int act_dragon_law;
extern int act_dragon_chaos;
extern int act_dragon_gold;
extern int act_dragon_multihued;
extern int act_dragon_red;
extern int act_dragon_green;
extern int act_dragon_blue;
extern int act_ring_lightning;
extern int act_ring_ice;
extern int act_ring_flames;
extern int act_ring_acid;
extern int act_shroom_purging;
extern int act_shroom_sprinting;
extern int act_shroom_debility;
extern int act_shroom_stone;
extern int act_shroom_terror;
extern int act_shroom_emergency;
extern int act_food_waybread;
extern int act_drink_breath;
extern int act_staff_holy;
extern int act_staff_magi;
extern int act_wand_breath;
extern int act_wonder;
extern int act_berserker;
extern int act_starlight2;
extern int act_starlight;
extern int act_polymorph;
extern int act_door_dest;
extern int act_disable_traps;
extern int act_light_line;
extern int act_mon_scare;
extern int act_sleep_all;
extern int act_mon_confuse;
extern int act_mon_slow;
extern int act_confuse2;
extern int act_tele_other;
extern int act_stone_to_mud;
extern int act_stinking_cloud;
extern int act_arrow;
extern int act_bizarre;
extern int act_mana_bolt;
extern int act_missile;
extern int act_drain_life4;
extern int act_drain_life3;
extern int act_drain_life2;
extern int act_drain_life1;
extern int act_elec_ball2;
extern int act_elec_ball;
extern int act_elec_bolt;
extern int act_acid_ball;
extern int act_acid_bolt3;
extern int act_acid_bolt2;
extern int act_acid_bolt;
extern int act_cold_ball160;
extern int act_cold_ball100;
extern int act_cold_ball50;
extern int act_cold_ball2;
extern int act_cold_bolt2;
extern int act_cold_bolt;
extern int act_fire_ball200;
extern int act_fire_ball2;
extern int act_fire_ball;
extern int act_fire_bolt72;
extern int act_fire_bolt3;
extern int act_fire_bolt2;
extern int act_fire_bolt;
extern int act_firebrand;
extern int act_rem_fear_pois;
extern int act_restore_life;
extern int act_rage_bless_resist;
extern int act_star_ball;
extern int act_sleepii;
extern int act_dispel_all;
extern int act_dispel_undead;
extern int act_dispel_evil60;
extern int act_dispel_evil;
extern int act_haste2;
extern int act_haste1;
extern int act_haste;
extern int act_probing;
extern int act_clairvoyance;
extern int act_illumination;
extern int act_loskill;
extern int act_losconf;
extern int act_lossleep;
extern int act_losslow;
extern int act_destruction2;
extern int act_earthquakes;
extern int act_deep_descent;
extern int act_recall;
extern int act_blessing3;
extern int act_blessing2;
extern int act_blessing;
extern int act_satisfy;
extern int act_protevil;
extern int act_banishment;
extern int act_recharge;
extern int act_destroy_doors;
extern int act_glyph;
extern int act_mapping;
extern int act_confusing;
extern int act_tele_level;
extern int act_tele_long;
extern int act_tele_phase;
extern int act_light;
extern int act_remove_curse2;
extern int act_remove_curse;
extern int act_enchant_armor2;
extern int act_enchant_armor;
extern int act_enchant_weapon;
extern int act_enchant_todam;
extern int act_enchant_tohit;
extern int act_detect_objects;
extern int act_detect_all;
extern int act_detect_evil;
extern int act_detect_invis;
extern int act_detect_treasure;
extern int act_resist_all;
extern int act_resist_pois;
extern int act_resist_cold;
extern int act_resist_fire;
extern int act_resist_elec;
extern int act_resist_acid;
extern int act_shero;
extern int act_hero;
extern int act_enlightenment;
extern int act_tmd_esp;
extern int act_tmd_sinvis;
extern int act_tmd_infra;
extern int act_tmd_free_act;
extern int act_restore_st_lev;
extern int act_restore_all;
extern int act_restore_con;
extern int act_restore_dex;
extern int act_restore_wis;
extern int act_restore_int;
extern int act_restore_str;
extern int act_nimbleness;
extern int act_toughness;
extern int act_contemplation;
extern int act_intellect;
extern int act_brawn;
extern int act_restore_mana;
extern int act_restore_exp;
extern int act_heal3;
extern int act_heal2;
extern int act_heal1;
extern int act_cure_temp;
extern int act_cure_nonorlybig;
extern int act_cure_full2;
extern int act_cure_full;
extern int act_cure_critical;
extern int act_cure_serious;
extern int act_cure_light;
extern int act_cure_body;
extern int act_cure_mind;
extern int act_cure_confusion;
extern int act_cure_paranoia;


/* WARNING: If you mess with the objects.txt or the monster.txt and change
 * the locations of things, then you must make those changes in borg.txt
 * as well as in this borg code.  The borg is very concerned about item
 * index locations.  ie: borgs_has[POTION_HEAL] is looking for a Potion of Healing.
 * The borg is concerned over several items, broken swords, and several
 * monster locations (Tarresque, Sauron, Morgoth).
 */

 /*
  * This file provides support for "borg1.c".
  */

  /*** Some constants ***/


  /*
   * Maximum possible dungeon size
   */
   /* NOTE: this corrisponds to z_info->dungeon_hgt/dungeon_wid */
  /* a test is done at the start of borg to make sure the values are right */
#define DUNGEON_WID 198
#define DUNGEON_HGT 66 

#define AUTO_MAX_X  DUNGEON_WID
#define AUTO_MAX_Y  DUNGEON_HGT

enum BORG_MONBLOW
{
    MONBLOW_NONE,
    MONBLOW_HURT,
    MONBLOW_POISON,
    MONBLOW_DISENCHANT,
    MONBLOW_DRAIN_CHARGES,
    MONBLOW_EAT_GOLD,
    MONBLOW_EAT_ITEM,
    MONBLOW_EAT_FOOD,
    MONBLOW_EAT_LIGHT,
    MONBLOW_ACID,
    MONBLOW_ELEC,
    MONBLOW_FIRE,
    MONBLOW_COLD,
    MONBLOW_BLIND,
    MONBLOW_CONFUSE,
    MONBLOW_TERRIFY,
    MONBLOW_PARALYZE,
    MONBLOW_LOSE_STR,
    MONBLOW_LOSE_INT,
    MONBLOW_LOSE_WIS,
    MONBLOW_LOSE_DEX,
    MONBLOW_LOSE_CON,
    MONBLOW_LOSE_ALL,
    MONBLOW_SHATTER,
    MONBLOW_EXP_10,
    MONBLOW_EXP_20,
    MONBLOW_EXP_40,
    MONBLOW_EXP_80,
    MONBLOW_HALLU,
    MONBLOW_BLACK_BREATH,
    MONBLOW_UNDEFINED
};

/*
 * Flags for the "info" field of grids
 *
 * Note that some of the flags below are not "perfect", in particular,
 * several of the flags should be treated as "best guesses", see below.
 *
 * The "BORG_MARK" flag means that the grid has been "observed", though
 * the terrain feature may or may not be memorized.  Note the use of the
 * "FEAT_NONE" and "FEAT_FLOOR" feature codes below.
 *
 * The "BORG_GLOW" flag means that a grid is probably "perma-lit", but
 * sometimes it is really only "recently" perma-lit, but was then made
 * dark with a darkness attack, and it is now torch-lit or off-screen.
 *
 * The "BORG_DARK" flag means that a grid is probably not "perma-lit",
 * but sometimes it is really only "recently" dark, but was then made
 * "lite" with a "call lite" spell, and it is now torch-lit or off-screen.
 *
 * The "BORG_LIGHT" flag means that a grid is probably lit by the player
 * torch, but this may not be true if the nearby "BORG_VIEW" flags are
 * not correct, or if the "lite radius" has changed recently.
 *
 * The "BORG_VIEW" flag means that a grid is probably in line of sight
 * of the player, but this may not be true if some of the grids between
 * the player and the grid contain previously unseen walls/doors/etc.
 *
 * The "BORG_TEMP" flag means that a grid has been added to the array
 * "borg_temp_x"/"borg_temp_y", though normally we ignore this flag.
 *
 * The "BORG_XTRA" flag is used for various "extra" purposes, primarily
 * to assist with the "update_view()" code.
 */
#define BORG_MARK   0x01    /* observed grid */
#define BORG_GLOW   0x02    /* probably perma-lit */
#define BORG_DARK   0x04    /* probably not perma-lit */
#define BORG_OKAY   0x08    /* on the current panel */
#define BORG_LIGHT   0x10    /* lit by the torch */
#define BORG_VIEW   0x20    /* in line of sight */
#define BORG_TEMP   0x40    /* temporary flag */
#define BORG_XTRA   0x80    /* extra flag */


 /*
  * Maximum size of the "lite" array
  */
#define AUTO_LIGHT_MAX 1536

  /*
   * All of the various attacks or projections the borg can do.
   !FIX !TODO !AJG probably want to externalize this somehow.
   */
enum
{
    BORG_ATTACK_MISSILE,
    BORG_ATTACK_ARROW,
    BORG_ATTACK_MANA,
    BORG_ATTACK_METEOR,
    BORG_ATTACK_ACID,
    BORG_ATTACK_ELEC,
    BORG_ATTACK_FIRE,
    BORG_ATTACK_COLD,
    BORG_ATTACK_POIS,
    BORG_ATTACK_ICE,
    BORG_ATTACK_HOLY_ORB,
    BORG_ATTACK_DISP_UNDEAD,
    BORG_ATTACK_DISP_EVIL,
    BORG_ATTACK_DISP_SPIRITS,
    BORG_ATTACK_SLEEP_EVIL,
    BORG_ATTACK_HOLY_WORD,
    BORG_ATTACK_LIGHT_WEAK,
    BORG_ATTACK_OLD_DRAIN,
    BORG_ATTACK_KILL_WALL,
    BORG_ATTACK_NETHER,
    BORG_ATTACK_CHAOS,
    BORG_ATTACK_GRAVITY,
    BORG_ATTACK_SHARD,
    BORG_ATTACK_SOUND,
    BORG_ATTACK_PLASMA,
    BORG_ATTACK_CONFU,
    BORG_ATTACK_DISEN,
    BORG_ATTACK_NEXUS,
    BORG_ATTACK_FORCE,
    BORG_ATTACK_INERTIA,
    BORG_ATTACK_TIME,
    BORG_ATTACK_LIGHT,
    BORG_ATTACK_DARK,
    BORG_ATTACK_WATER,
    BORG_ATTACK_OLD_HEAL,
    BORG_ATTACK_OLD_CLONE,
    BORG_ATTACK_OLD_SPEED,
    BORG_ATTACK_DARK_WEAK,
    BORG_ATTACK_KILL_DOOR,
    BORG_ATTACK_KILL_TRAP,
    BORG_ATTACK_MAKE_WALL,
    BORG_ATTACK_MAKE_DOOR,
    BORG_ATTACK_MAKE_TRAP,
    BORG_ATTACK_AWAY_UNDEAD,
    BORG_ATTACK_TURN_EVIL,
    BORG_ATTACK_AWAY_ALL,
    BORG_ATTACK_AWAY_ALL_MORGOTH,
    BORG_ATTACK_DISP_ALL,
    BORG_ATTACK_OLD_CONF,
    BORG_ATTACK_TURN_ALL,
    BORG_ATTACK_OLD_SLOW,
    BORG_ATTACK_OLD_SLEEP,
    BORG_ATTACK_OLD_POLY,
    BORG_ATTACK_TURN_UNDEAD,
    BORG_ATTACK_AWAY_EVIL,
    BORG_ATTACK_TAP_UNLIFE,
    BORG_ATTACK_DRAIN_LIFE,
};

/*
 * Player race constants (hard-coded by save-files, arrays, etc)
 */
#define RACE_HUMAN      0
#define RACE_HALF_ELF   1
#define RACE_ELF        2
#define RACE_HOBBIT     3
#define RACE_GNOME      4
#define RACE_DWARF      5
#define RACE_HALF_ORC   6
#define RACE_HALF_TROLL 7
#define RACE_DUNADAN    8
#define RACE_HIGH_ELF   9
#define RACE_KOBOLD     10
#define MAX_RACES	    11


 /*
  * Maximum size of the "view" array
  */
  /*#define AUTO_VIEW_MAX 1536*/
#define AUTO_VIEW_MAX 9000


/*
 * Number of grids in the "temp" array
 */
#define AUTO_TEMP_MAX 9000


 /*
  * Number of grids in the "flow" array
  */
#define AUTO_FLOW_MAX 1536



/*
 * Enable the "borg_note()" usage of the Recall Window
 * Also specify the number of "rolling rows" to use
 */
#define BORG_NOTE_ROWS      12

/*
* Threshold where the borg will attempt to dig things
*/
#define BORG_DIG			13
#define BORG_DIG_HARD		40

/*
 * Size of Keypress buffer
 */
#define KEY_SIZE 8192




     /*
      * Object information
      */

typedef struct borg_take borg_take;

struct borg_take
{
    struct object_kind* kind;      /* Kind */
    bool    known;      /* Verified kind */
    bool    seen;       /* Assigned motion */
    bool    extra;      /* Unused */
    bool	orbed;		/* Orb of Draining cast on it */
    uint8_t x, y;       /* Location */
    int16_t when;       /* When last seen */
    int		value;		/* Estimated value of item */
    int 	tval;		/* Known tval */
};


/*
 * Monster information
 */

typedef struct borg_kill borg_kill;

struct borg_kill
{
    unsigned int r_idx;      /* Race index */

    bool    known;      /* Verified race */
    bool    awake;      /* Probably awake */

    bool    confused;   /* Probably confused */
    bool    afraid;     /* Probably afraid */
    bool    quiver;     /* Probably quivering */
    bool    stunned;
    bool    poisoned;   /* Probably poisoned */

    bool    seen;       /* Assigned motion */
    bool    used;       /* Assigned message */

    uint8_t x, y;       /* Location */

    uint8_t ox, oy;     /* Old location */

    uint8_t speed;      /* Estimated speed */
    uint8_t moves;      /* Estimates moves */
    uint8_t ranged_attack; /* qty of ranged attacks */
    uint8_t	spell[RSF_MAX];		/* spell flag for monster spells */
    int16_t power;      /* Estimated hit-points */
    int16_t	injury;		/* Percent wounded */
    int16_t other;      /* Estimated something */
    int16_t level;      /* Monsters Level */
    uint32_t spell_flags[RF_MAX]; /* Monster race spell flags preloaded */
    int16_t when;       /* When last seen */
    int16_t	m_idx;		/* Game's index */
};



/*
 * Forward declare
 */
typedef struct borg_grid borg_grid;

/*
 * A grid in the dungeon.  Several uint8_ts.
 *
 * There is a set of eight bit flags (see below) called "info".
 *
 * There is a terrain feature type, which may be incorrect.  It is
 * more or less based on the standard "feature" values, but some of
 * the legal values are never used, such as "secret door", and others
 * are used in bizarre ways, such as "invisible trap".
 *
 * There is an object index into the "object tracking" array.
 *
 * There is a monster index into the "monster tracking" array.
 *
 * There is a byte "hmmm" which is currently unused.
 *
 * There is a byte "xtra" which tracks how much "searching" has been done
 * in the grid or in any grid next to the grid.
 *
 * To perform "navigation" from one place to another, the "flow" routines
 * are used, which place "cost" information into the "cost" fields.  Then,
 * if the path is clear, the "cost" information is copied into the "flow"
 * fields, which are used for the actual navigation.  This allows multiple
 * routines to check for "possible" flowing, without hurting the current
 * flow, which may have taken a long time to construct.  We also assume
 * that the Borg never needs to follow a path longer than 250 grids long.
 * Note that the "cost" fields have been moved into external arrays.
 *
 * Hack -- note that the "char" zero will often crash the system!
 */
struct borg_grid
{
    uint8_t feat;      /* Grid type */
    uint8_t info;      /* Grid flags */
    bool    trap;
    bool    glyph;
    uint8_t store;

    uint8_t take;      /* Object index */
    uint8_t kill;      /* Monster index */

    uint8_t hmmm;      /* Extra field (unused) */

    uint8_t xtra;      /* Extra field (search count) */
};


/*
 * Forward declare
 */
typedef struct borg_data borg_data;

/*
 * Hack -- one byte of info per grid
 *
 * We use a structure to encapsulate the data into a "typed" form.
 */
struct borg_data
{
    uint8_t data[AUTO_MAX_Y][AUTO_MAX_X];
};




/*** Some macros ***/


/*
 * Determine "twice" the distance between two points
 * This results in "diagonals" being "correctly" ranged,
 * that is, a diagonal appears "furthur" than an adjacent.
 */
#define double_distance(Y1,X1,Y2,X2) \
    (borg_distance(((int)(Y1))<<1,((int)(X1))<<1,((int)(Y2))<<1,((int)(X2))<<1))



 /*** Some variables ***/


 /*
  * Some variables
  */
extern bool     borg_active;        /* Actually active */
extern bool     borg_resurrect;     /* Continous play mode */
extern bool     borg_cancel;        /* Being cancelled */
extern bool     borg_scumming_pots; /* Borg will quickly store pots in home */

extern char     genocide_target;    /* Identity of the poor unsuspecting soul */
extern int      borg_zap_slot;            /* to avoid a nasty game bug with amnesia */
extern bool     borg_casted_glyph;  /* we dont have a launch messages anymore */
extern bool     borg_dont_react;
extern int      successful_target;
extern int      sold_item_tval[10];
extern int      sold_item_sval[10];
extern int      sold_item_pval[10];
extern int      sold_item_store[10];
extern int      sold_item_num;
extern int      sold_item_nxt;
extern int      bought_item_tval[10];
extern int      bought_item_sval[10];
extern int      bought_item_pval[10];
extern int      bought_item_store[10];
extern int      bought_item_num;
extern int      bought_item_nxt;

extern const char * shop_menu_items;
extern uint8_t  borg_nasties_num;
extern uint8_t  borg_nasties_count[7];
extern char     borg_nasties[7];
extern uint8_t  borg_nasties_limit[7];

extern bool     borg_init_failure;
extern bool     borg_lunal_mode;
extern bool     borg_munchkin_mode;

extern int      borg_money_scum_who;
extern int      borg_money_scum_ware;


/* options from the borg.txt file */
/* IMPORTANT keep these in sync with borg_settings in borg9.c */
enum
{
    BORG_VERBOSE,
    BORG_MUNCHKIN_START,
    BORG_MUNCHKIN_LEVEL,
    BORG_MUNCHKIN_DEPTH,
    BORG_WORSHIPS_DAMAGE,
    BORG_WORSHIPS_SPEED,
    BORG_WORSHIPS_HP,
    BORG_WORSHIPS_MANA,
    BORG_WORSHIPS_AC,
    BORG_WORSHIPS_GOLD,
    BORG_PLAYS_RISKY,
    BORG_KILLS_UNIQUES,
    BORG_USES_SWAPS,
    BORG_USES_DYNAMIC_CALCS,
    BORG_SLOW_OPTIMIZEHOME,
    BORG_STOP_DLEVEL,
    BORG_STOP_CLEVEL,
    BORG_NO_DEEPER,
    BORG_STOP_KING,
    BORG_RESPAWN_WINNERS,
    BORG_RESPAWN_CLASS,
    BORG_RESPAWN_RACE,
    BORG_CHEST_FAIL_TOLERANCE,
    BORG_DELAY_FACTOR,
    BORG_MONEY_SCUM_AMOUNT,
    BORG_SELF_SCUM,
    BORG_LUNAL_MODE,
    BORG_SELF_LUNAL,
    BORG_ENCHANT_LIMIT,
    BORG_DUMP_LEVEL,
    BORG_SAVE_DEATH,
    BORG_MAX_SETTINGS
};

extern int *borg_cfg;

/* dynamic required items */
/* dynamic required items */
typedef struct req_item
{
    int depth;
    int item;
    int number;

} req_item;

extern req_item** borg_required_item;

extern int* n_req;
typedef struct power_item
{
    int     depth;
    int     cnd;
    int     item;
    int     from;
    int     to;
    int     power;
    bool    each;
} power_item;

extern power_item** borg_power_item;
extern int*     n_pwr;
extern int*     borg_has;
extern int*     borg_skill;
extern int      size_depth;
extern int      size_obj;

enum borg_item_pos
{
    BORG_INVEN = 1,
    BORG_EQUIP = 2,
    BORG_QUILL = 4
};

/* NOTE: This must exactly match the prefix_pref enums in BORG1.c */
enum
{
    BI_STR,
    BI_INT,
    BI_WIS,
    BI_DEX,
    BI_CON,
    BI_CSTR,
    BI_CINT,
    BI_CWIS,
    BI_CDEX,
    BI_CCON,
    BI_SSTR,
    BI_SINT,
    BI_SWIS,
    BI_SDEX,
    BI_SCON,
    BI_LIGHT,
    BI_CURHP,
    BI_MAXHP,
    BI_CURSP,
    BI_MAXSP,
    BI_SP_ADJ,
    BI_FAIL1,
    BI_FAIL2,
    BI_CLEVEL,
    BI_MAXCLEVEL,
    BI_ESP,
    BI_CURLITE,
    BI_RECALL,
    BI_FOOD,
    BI_SPEED,
    BI_MOD_MOVES,
    BI_DAM_RED,
    BI_SDIG,
    BI_FEATH,
    BI_REG,
    BI_SINV,
    BI_INFRA,
    BI_DISP,
    BI_DISM,
    BI_DEV,
    BI_SAV,
    BI_STL,
    BI_SRCH,
    BI_THN,
    BI_THB,
    BI_THT,
    BI_DIG,
    BI_IFIRE,
    BI_IACID,
    BI_ICOLD,
    BI_IELEC,
    BI_IPOIS,
    BI_TRFIRE,
    BI_TRCOLD,
    BI_TRACID,
    BI_TRPOIS,
    BI_TRELEC,
    BI_RFIRE,
    BI_RCOLD,
    BI_RELEC,
    BI_RACID,
    BI_RPOIS,
    BI_RFEAR,
    BI_RLITE,
    BI_RDARK,
    BI_RBLIND,
    BI_RCONF,
    BI_RSND,
    BI_RSHRD,
    BI_RNXUS,
    BI_RNTHR,
    BI_RKAOS,
    BI_RDIS,
    BI_HLIFE,
    BI_FRACT,
    BI_SRFIRE,
    BI_SRCOLD,
    BI_SRELEC,
    BI_SRACID,
    BI_SRPOIS,
    BI_SRFEAR,
    BI_SRLITE,
    BI_SRDARK,
    BI_SRBLIND,
    BI_SRCONF,
    BI_SRSND,
    BI_SRSHRD,
    BI_SRNXUS,
    BI_SRNTHR,
    BI_SRKAOS,
    BI_SRDIS,
    BI_SHLIFE,
    BI_SFRACT,

    BI_DEPTH,
    BI_CDEPTH,
    BI_MAXDEPTH,
    BI_KING,

    BI_ISWEAK,
    BI_ISHUNGRY,
    BI_ISFULL,
    BI_ISGORGED,
    BI_ISBLIND,
    BI_ISAFRAID,
    BI_ISCONFUSED,
    BI_ISPOISONED,
    BI_ISCUT,
    BI_ISSTUN,
    BI_ISHEAVYSTUN,
    BI_ISPARALYZED,
    BI_ISIMAGE,
    BI_ISFORGET,
    BI_ISENCUMB,
    BI_ISSTUDY,
    BI_ISFIXLEV,
    BI_ISFIXEXP,
    BI_ISFIXSTR,
    BI_ISFIXINT,
    BI_ISFIXWIS,
    BI_ISFIXDEX,
    BI_ISFIXCON,
    BI_ISFIXALL,

    BI_ARMOR,
    BI_TOHIT,
    BI_TODAM,
    BI_WTOHIT,
    BI_WTODAM,
    BI_BTOHIT,
    BI_BTODAM,
    BI_BLOWS,
    BI_SHOTS,
    BI_WMAXDAM,
    BI_WBASEDAM,
    BI_BMAXDAM,
    BI_HEAVYWEPON,
    BI_HEAVYBOW,
    BI_AMMO_COUNT,
    BI_AMMO_TVAL,
    BI_AMMO_SIDES,
    BI_AMMO_POWER,
    BI_AMISSILES,
    BI_AMISSILES_SPECIAL,
    BI_AMISSILES_CURSED,
    BI_QUIVER_SLOTS,
    BI_FIRST_CURSED,
    BI_WHERE_CURSED,

    BI_CRSENVELOPING,
    BI_CRSIRRITATION,
    BI_CRSTELE,
    BI_CRSPOIS,
    BI_CRSSIREN,
    BI_CRSHALU,
    BI_CRSPARA,
    BI_CRSSDEM,
    BI_CRSSDRA,
    BI_CRSSUND,
    BI_CRSSTONE,
    BI_CRSNOTEL,
    BI_CRSTWEP,
    BI_CRSAGRV,
    BI_CRSHPIMP,
    BI_CRSMPIMP,
    BI_CRSSTEELSKIN,
    BI_CRSAIRSWING,
    BI_CRSFEAR,
    BI_CRSDRAIN_XP,
    BI_CRSFVULN,
    BI_CRSEVULN,
    BI_CRSCVULN,
    BI_CRSAVULN,
    BI_CRSUNKNO,

    BI_WS_ANIMAL,
    BI_WS_EVIL,
    BI_WS_UNDEAD,
    BI_WS_DEMON,
    BI_WS_ORC,
    BI_WS_TROLL,
    BI_WS_GIANT,
    BI_WS_DRAGON,
    BI_WK_UNDEAD,
    BI_WK_DEMON,
    BI_WK_DRAGON,
    BI_W_IMPACT,
    BI_WB_ACID,
    BI_WB_ELEC,
    BI_WB_FIRE,
    BI_WB_COLD,
    BI_WB_POIS,
    BI_APHASE,
    BI_ATELEPORT,
    BI_AESCAPE,
    BI_AFUEL,
    BI_AHEAL,
    BI_AEZHEAL,
    BI_ALIFE,
    BI_AID,
    BI_ASPEED,
    BI_ASTFMAGI,
    BI_ASTFDEST,
    BI_ATPORTOTHER,
    BI_ACUREPOIS,
    BI_ADETTRAP,
    BI_ADETDOOR,
    BI_ADETEVIL,
    BI_AMAGICMAP,
    BI_ARECHARGE,
    BI_ALITE,
    BI_APFE,
    BI_AGLYPH,
    BI_ACCW,
    BI_ACSW,
    BI_ACLW,
    BI_ARESHEAT,
    BI_ARESCOLD,
    BI_ARESPOIS,
    BI_ATELEPORTLVL,  /* scroll of teleport level */
    BI_AHWORD,            /* Holy Word prayer */
    BI_AMASSBAN,	/* ?Mass Banishment */
    BI_ASHROOM,
    BI_AROD1,		/* Attack rods */
    BI_AROD2,		/* Attack rods */
    BI_DINV,	/* See Inv Spell Legal */
    BI_WEIGHT,  /* weight of all inventory and equipment */
    BI_EMPTY,   /* number of empty slots */

    BI_MAX
};

#define MAX_FORMULA_ELEMENTS 60
enum
{
    BFO_DONE, /* just to make sure we end fast if there is no formula */
    BFO_NUMBER,
    BFO_VARIABLE,
    BFO_EQ,
    BFO_NEQ,
    BFO_NOT,
    BFO_LT,
    BFO_LTE,
    BFO_GT,
    BFO_GTE,
    BFO_AND,
    BFO_OR,
    BFO_PLUS,
    BFO_MINUS,
    BFO_DIVIDE,
    BFO_MULT
};

extern int*     formula[1000];
extern const char* prefix_pref[];

/*
 * Hack -- optional cheating flags
 */

 /*
  * Various silly flags
  */

extern bool     borg_flag_save;     /* Save savefile at each level */

extern bool     borg_flag_dump;     /* Save savefile at each death */

extern bool     borg_save; /* do a save next time we get to press a key! */

extern bool     borg_borg_message;      /* List borg messages or not */
extern bool     borg_graphics;          /* List borg messages or not */
extern bool     borg_confirm_target;

extern char     borg_engine_date[];       /* last update */

/*
 * Use a simple internal random number generator
 */

extern bool     borg_rand_quick;        /* Save system setting */

extern uint32_t borg_rand_value;        /* Save system setting */

extern uint32_t borg_rand_local;        /* Save personal setting */


/*
 * Hack -- time variables
 */

extern int16_t  borg_t;        /* Current "time" */
extern int16_t  borg_t_morgoth;
extern unsigned int  borg_morgoth_id;
extern int16_t  need_see_inviso;        /* To tell me to cast it */
extern int16_t  borg_see_inv;
extern bool     need_shift_panel;        /* to spot offscreeners */
extern int16_t  when_shift_panel;
extern int16_t  time_this_panel;        /* Current "time" for current panel*/
extern bool     vault_on_level;     /* borg will search for a vault */
extern unsigned int unique_on_level;
extern bool     scaryguy_on_level;
extern bool     morgoth_on_level;
extern bool     borg_morgoth_position;
extern int      borg_t_antisummon;		/* Timestamp when in a AS spot */
extern bool     borg_as_position;		/* Sitting in an anti-summon corridor */
extern bool     borg_digging;			/* used in Anti-summon corridor */

extern bool     breeder_level;      /* Borg will shut doors */
extern int16_t  old_depth;
extern int16_t  borg_respawning;       /* to prevent certain crashes */
extern int16_t  borg_no_retreat;

/*
 * Hack -- Other time variables
 */

extern int16_t  when_call_LIGHT; /* When we last did call light */
extern int16_t  when_wizard_LIGHT;   /* When we last did wizard light */

extern int16_t  when_detect_traps;  /* When we last detected traps */
extern int16_t  when_detect_doors;  /* When we last detected doors */
extern int16_t  when_detect_walls;  /* When we last detected walls */
extern int16_t  when_detect_evil;
extern int16_t  when_detect_obj;
extern int16_t  when_last_kill_mult;   /* When a multiplier was last killed */

extern bool     my_need_alter;     /* incase of walls/doors */
extern bool     my_no_alter;     /* incase of walls/doors */
extern bool     my_need_redraw;     /* incase of walls/doors */
extern bool     borg_attempting_refresh_resist;  /* for the Resistance spell */

/*
 * Some information
 */

extern int16_t  goal;       /* Flowing (goal type) */

extern bool     goal_rising;    /* Currently returning to town */
extern bool     goal_leaving;   /* Currently leaving the level */
extern bool     goal_fleeing;   /* Currently fleeing the level */
extern bool     goal_fleeing_lunal;   /* Currently fleeing the level in lunal*/
extern bool     goal_fleeing_munchkin; /* Fleeing level while in munchkin Mode */
extern bool     goal_fleeing_to_town; /* Currently fleeing the level to return to town */
extern bool     goal_ignoring;  /* Currently ignoring monsters */
extern int      goal_recalling;  /* Currently waiting for recall, guessing turns left */
extern bool     goal_less;      /* return to, but dont use, the next up stairs */

extern int16_t  borg_times_twitch; /* how often twitchy on this level */
extern int16_t  borg_escapes; /* how often teleported on this level */

extern bool     stair_less;     /* Use the next "up" staircase */
extern bool     stair_more;     /* Use the next "down" staircase */

extern int32_t  borg_began;     /* When this level began */
extern int32_t  borg_time_town; /* how long it has been since I was in town */

extern int16_t  avoidance;      /* Current danger thresh-hold */

extern bool     borg_failure;   /* Notice failure */

extern bool     borg_simulate;  /* Simulation flag */
extern bool     borg_attacking; /* Are we attacking a monster? */
extern bool     borg_offsetting; /* Are we attacking a monster? with offsett balls*/

extern bool     borg_completed; /* Completed the level */
extern bool     borg_on_upstairs;      /* used when leaving a level */
extern bool     borg_on_dnstairs;      /* used when leaving a level */
extern bool     borg_needs_searching;  /* borg will search with each step */
extern int16_t  borg_oldchp;		/* hit points last game turn */
extern int16_t  borg_oldcsp;		/* mana points last game turn */

/* defence flags */
extern bool     borg_prot_from_evil;
extern bool     borg_speed;
extern bool     borg_bless;
extern bool     borg_hero;
extern bool     borg_berserk;
extern bool     borg_fastcast;
extern bool     borg_regen;
extern bool     borg_smite_evil;
extern bool     borg_venom;
extern int16_t  borg_game_ratio;
extern int16_t  borg_resistance;
extern int16_t  borg_no_rest_prep; /* borg wont rest for a few turns */
extern bool     borg_shield;
extern bool     borg_on_glyph; /* borg is standing on a glyph of warding */
extern bool     borg_create_door; /* borg is going to create doors */
extern bool     borg_sleep_spell;
extern bool     borg_sleep_spell_ii;
extern bool     borg_crush_spell;
extern bool     borg_slow_spell;
extern bool     borg_confuse_spell;
extern bool     borg_fear_mon_spell;


/*
 * Shop goals
 */
extern bool     borg_in_shop;
extern int16_t  goal_shop;      /* Next shop to visit */
extern int16_t  goal_ware;      /* Next item to buy there */
extern int16_t  goal_item;      /* Next item to sell there */
extern int      borg_food_onsale;      /* Are shops selling food? */
extern int      borg_fuel_onsale;      /* Are shops selling fuel? */
extern bool     borg_needs_quick_shopping; /* Needs to buy without browsing all shops */
extern int16_t  borg_best_fit_item;   /* Item to be worn, not sold */
extern int      borg_best_item;

/*
 * Other variables
 */

extern int      w_x;         /* Current panel offset (X) */
extern int      w_y;         /* Current panel offset (Y) */
extern int      morgy_panel_y;
extern int      morgy_panel_x;

extern int      borg_target_y;
extern int      borg_target_x;  /* Current targetted location */

extern int      c_x;         /* Current location (X) */
extern int      c_y;         /* Current location (Y) */

extern int      g_x;         /* Goal location (X) */
extern int      g_y;         /* Goal location (Y) */

/*
 * Some estimated state variables
 */

extern int16_t  my_stat_max[STAT_MAX]; /* Current "maximal" stat values    */
extern int16_t  my_stat_cur[STAT_MAX]; /* Current "natural" stat values    */
extern int16_t  my_stat_ind[STAT_MAX]; /* Current "additions" to stat values   */
extern bool     my_need_stat_check[STAT_MAX];  /* do I need to check my stats */

extern int16_t  my_stat_add[STAT_MAX];  /* aditions to stats */

extern int16_t  home_stat_add[STAT_MAX];

extern int      weapon_swap;   /* location of my swap weapon   */
extern int32_t  weapon_swap_value;   /* value of my swap weapon   */
extern int      armour_swap;   /* location of my swap weapon   */
extern int32_t  armour_swap_value;   /* value of my swap weapon   */

extern bool     decurse_weapon_swap;  /* my swap is great, except its cursed */
extern int      enchant_weapon_swap_to_h;  /* my swap is great, except its cursed */
extern int      enchant_weapon_swap_to_d;  /* my swap is great, except its cursed */
extern bool     decurse_armour_swap;  /* my swap is great, except its cursed */
extern int      enchant_armour_swap_to_a;  /* my swap is great, except its cursed */


extern int16_t  weapon_swap_digger;
extern uint8_t  weapon_swap_slay_animal;
extern uint8_t  weapon_swap_slay_evil;
extern uint8_t  weapon_swap_slay_undead;
extern uint8_t  weapon_swap_slay_demon;
extern uint8_t  weapon_swap_slay_orc;
extern uint8_t  weapon_swap_slay_troll;
extern uint8_t  weapon_swap_slay_giant;
extern uint8_t  weapon_swap_slay_dragon;
extern uint8_t  weapon_swap_impact;
extern uint8_t  weapon_swap_brand_acid;
extern uint8_t  weapon_swap_brand_elec;
extern uint8_t  weapon_swap_brand_fire;
extern uint8_t  weapon_swap_brand_cold;
extern uint8_t  weapon_swap_brand_pois;
extern uint8_t  weapon_swap_see_infra;
extern uint8_t  weapon_swap_slow_digest;
extern uint8_t  weapon_swap_aggravate;
extern uint8_t  weapon_swap_bad_curse;
extern uint8_t  weapon_swap_regenerate;
extern uint8_t  weapon_swap_telepathy;
extern uint8_t  weapon_swap_light;
extern uint8_t  weapon_swap_see_invis;
extern uint8_t  weapon_swap_ffall;
extern uint8_t  weapon_swap_free_act;
extern uint8_t  weapon_swap_hold_life;
extern uint8_t  weapon_swap_immune_fire;
extern uint8_t  weapon_swap_immune_acid;
extern uint8_t  weapon_swap_immune_cold;
extern uint8_t  weapon_swap_immune_elec;
extern uint8_t  weapon_swap_resist_acid;
extern uint8_t  weapon_swap_resist_elec;
extern uint8_t  weapon_swap_resist_fire;
extern uint8_t  weapon_swap_resist_cold;
extern uint8_t  weapon_swap_resist_pois;
extern uint8_t  weapon_swap_resist_conf;
extern uint8_t  weapon_swap_resist_sound;
extern uint8_t  weapon_swap_resist_light;
extern uint8_t  weapon_swap_resist_dark;
extern uint8_t  weapon_swap_resist_chaos;
extern uint8_t  weapon_swap_resist_disen;
extern uint8_t  weapon_swap_resist_shard;
extern uint8_t  weapon_swap_resist_nexus;
extern uint8_t  weapon_swap_resist_blind;
extern uint8_t  weapon_swap_resist_neth;
extern uint8_t  weapon_swap_resist_fear;
extern uint8_t  armour_swap_slay_animal;
extern uint8_t  armour_swap_slay_evil;
extern uint8_t  armour_swap_slay_undead;
extern uint8_t  armour_swap_slay_demon;
extern uint8_t  armour_swap_slay_orc;
extern uint8_t  armour_swap_slay_troll;
extern uint8_t  armour_swap_slay_giant;
extern uint8_t  armour_swap_slay_dragon;
extern uint8_t  armour_swap_impact;
extern uint8_t  armour_swap_brand_acid;
extern uint8_t  armour_swap_brand_elec;
extern uint8_t  armour_swap_brand_fire;
extern uint8_t  armour_swap_brand_cold;
extern uint8_t  armour_swap_brand_pois;
extern uint8_t  armour_swap_see_infra;
extern uint8_t  armour_swap_slow_digest;
extern uint8_t  armour_swap_aggravate;
extern uint8_t  armour_swap_bad_curse;
extern uint8_t  armour_swap_regenerate;
extern uint8_t  armour_swap_telepathy;
extern uint8_t  armour_swap_light;
extern uint8_t  armour_swap_see_invis;
extern uint8_t  armour_swap_ffall;
extern uint8_t  armour_swap_free_act;
extern uint8_t  armour_swap_hold_life;
extern uint8_t  armour_swap_immune_fire;
extern uint8_t  armour_swap_immune_acid;
extern uint8_t  armour_swap_immune_cold;
extern uint8_t  armour_swap_immune_elec;
extern uint8_t  armour_swap_resist_acid;
extern uint8_t  armour_swap_resist_elec;
extern uint8_t  armour_swap_resist_fire;
extern uint8_t  armour_swap_resist_cold;
extern uint8_t  armour_swap_resist_pois;
extern uint8_t  armour_swap_resist_conf;
extern uint8_t  armour_swap_resist_sound;
extern uint8_t  armour_swap_resist_LIGHT;
extern uint8_t  armour_swap_resist_dark;
extern uint8_t  armour_swap_resist_chaos;
extern uint8_t  armour_swap_resist_disen;
extern uint8_t  armour_swap_resist_shard;
extern uint8_t  armour_swap_resist_nexus;
extern uint8_t  armour_swap_resist_blind;
extern uint8_t  armour_swap_resist_neth;
extern uint8_t  armour_swap_resist_fear;

extern int16_t  my_need_enchant_to_a;   /* Need some enchantment */
extern int16_t  my_need_enchant_to_h;   /* Need some enchantment */
extern int16_t  my_need_enchant_to_d;   /* Need some enchantment */
extern int16_t  my_need_brand_weapon;  /* brand bolts */
extern int16_t  my_need_id;			/* need to buy ID for an inventory item */


/*
 * Hack -- basic "power"
 */

extern int32_t  my_power;


/*
 * Various "amounts" (for the player)
 */

extern int16_t  amt_food_lowcal;
extern int16_t  amt_food_hical;

extern int16_t  amt_slow_poison;
extern int16_t  amt_cure_confusion;
extern int16_t  amt_cure_blind;

extern int16_t  amt_cool_staff;  /* holiness-power staff */
extern int16_t  amt_cool_wand;	/* # of charges */
extern int16_t  amt_book[9];

extern int16_t  amt_add_stat[6];
extern int16_t  amt_inc_stat[6];

extern int16_t  amt_fix_exp;

extern int16_t  amt_enchant_to_a;
extern int16_t  amt_enchant_to_d;
extern int16_t  amt_enchant_to_h;
extern int16_t  amt_brand_weapon;  /* cubragol and bolts */
extern int16_t  amt_enchant_weapon;
extern int16_t  amt_enchant_armor;
extern int16_t  amt_digger;
extern int16_t  amt_ego;

/*
 * Various "amounts" (for the home)
 */

extern int16_t  num_food;
extern int16_t  num_fuel;
extern int16_t  num_mold;
extern int16_t  num_ident;
extern int16_t  num_recall;
extern int16_t  num_phase;
extern int16_t  num_escape;
extern int16_t  num_tele_staves;
extern int16_t  num_teleport;
extern int16_t  num_berserk;
extern int16_t  num_teleport_level;
extern int16_t  num_recharge;

extern int16_t  num_cure_critical;
extern int16_t  num_cure_serious;

extern int16_t  num_pot_rheat;
extern int16_t  num_pot_rcold;

extern int16_t  num_missile;

extern int16_t  num_book[9];

extern int16_t  num_fix_stat[7];

extern int16_t  num_fix_exp;
extern int16_t  num_mana;
extern int16_t  num_heal;
extern int16_t  num_heal_true;
extern int16_t  num_ezheal;
extern int16_t  num_ezheal_true;
extern int16_t  num_life;
extern int16_t  num_life_true;
extern int16_t  num_pfe;
extern int16_t  num_glyph;

extern int16_t  num_enchant_to_a;
extern int16_t  num_enchant_to_d;
extern int16_t  num_enchant_to_h;
extern int16_t  num_brand_weapon;  /*  crubragol and bolts */
extern int16_t  num_genocide;
extern int16_t  num_mass_genocide;

extern int16_t  num_artifact;
extern int16_t  num_ego;

extern int16_t  home_slot_free;
extern int16_t  home_un_id;
extern int16_t  home_damage;
extern int16_t  num_duplicate_items;
extern int16_t  num_slow_digest;
extern int16_t  num_regenerate;
extern int16_t  num_telepathy;
extern int16_t  num_LIGHT;
extern int16_t  num_see_inv;

extern int16_t  num_invisible; /**/

extern int16_t  num_ffall;
extern int16_t  num_free_act;
extern int16_t  num_hold_life;
extern int16_t  num_immune_acid;
extern int16_t  num_immune_elec;
extern int16_t  num_immune_fire;
extern int16_t  num_immune_cold;
extern int16_t  num_resist_acid;
extern int16_t  num_resist_elec;
extern int16_t  num_resist_fire;
extern int16_t  num_resist_cold;
extern int16_t  num_resist_pois;
extern int16_t  num_resist_conf;
extern int16_t  num_resist_sound;
extern int16_t  num_resist_LIGHT;
extern int16_t  num_resist_dark;
extern int16_t  num_resist_chaos;
extern int16_t  num_resist_disen;
extern int16_t  num_resist_shard;
extern int16_t  num_resist_nexus;
extern int16_t  num_resist_blind;
extern int16_t  num_resist_neth;
extern int16_t  num_sustain_str;
extern int16_t  num_sustain_int;
extern int16_t  num_sustain_wis;
extern int16_t  num_sustain_dex;
extern int16_t  num_sustain_con;
extern int16_t  num_sustain_all;

extern int16_t  num_speed;
extern int16_t  num_edged_weapon;
extern int16_t  num_bad_gloves;
extern int16_t  num_weapons;
extern int16_t  num_bow;
extern int16_t  num_rings;
extern int16_t  num_neck;
extern int16_t  num_armor;
extern int16_t  num_cloaks;
extern int16_t  num_shields;
extern int16_t  num_hats;
extern int16_t  num_gloves;
extern int16_t  num_boots;

/*
 * Deal with knowing which uniques are alive
 */
extern int      borg_numb_live_unique;
extern unsigned int borg_living_unique_index;
extern int      borg_unique_depth;

/*
 * Hack -- extra state variables
 */

extern int      borg_feeling_danger;   /* Current level "feeling" */
extern int      borg_feeling_stuff;    /* Current level "feeling" */

/*
 * Hack -- current shop index
 */

extern int16_t  shop_num;       /* Current shop index */



/*
 * State variables extracted from the screen
 */

extern int32_t  borg_exp;       /* Current experience */
extern int32_t  borg_gold;      /* Current gold */

extern int      borg_stat[6];    /* Current stats */
extern int      borg_book[9];    /* Current book slots */


/*
 * State variables extracted from the inventory/equipment
 */

extern int      borg_cur_wgt;    /* Current weight */


/*
 * Constant state variables
 */

extern int      borg_race;       /* Current race */
extern int      borg_class;      /* Current class */



/*
 * Constant state structures
 */


extern void mmove2(int* y, int* x, int y1, int x1, int y2, int x2);

/*
 * Number of turns to step for (zero means forever)
 */
extern uint16_t borg_step;      /* Step count (if any) */


/*
 * Status message search string
 */
extern char     borg_match[128];    /* Search string */


/*
 * Log file
 */
extern FILE*    borg_fff;      /* Log file */



/*
 * Hack -- the detection arrays
 */

extern bool     borg_detect_wall[6][18];
extern bool     borg_detect_trap[6][18];
extern bool     borg_detect_door[6][18];
extern bool     borg_detect_evil[6][18];
extern bool     borg_detect_obj[6][18];

/*
 * Locate the store doors
 */

extern int*     track_shop_x;
extern int*     track_shop_y;

/*
* track where some things are
*/
struct borg_track
{
    int16_t num;
    int16_t size;
    int*    x;
    int*    y;

};


/*
 * Track "stairs up"
 */
extern struct borg_track track_less;
/*
 * Track "stairs down"
 */
extern struct borg_track track_more;

/*
 * Track glyphs
 */
extern struct borg_track track_glyph;

extern bool     borg_needs_new_sea;

/*
 * Track the items worn to avoid loops
 */
extern int16_t  track_worn_num;
extern int16_t  track_worn_size;
extern int16_t  track_worn_time;
extern uint8_t* track_worn_name1;

extern const int16_t borg_ddx_ddd[24];
extern const int16_t borg_ddy_ddd[24];

/*
 * Track steps
 */
extern struct borg_track track_step;

/*
 * Track closed doors
 */
extern struct borg_track track_door;

/*
 * Track closed doors which started closed
 */
extern struct borg_track track_closed;

/*
 * Track the mineral veins with treasure
 *
 */
extern struct borg_track track_vein;

/*
 * The object list.  This list is used to "track" objects.
 */

extern int16_t  borg_takes_cnt;
extern int16_t  borg_takes_nxt;
extern borg_take* borg_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

extern int16_t  borg_kills_cnt;
extern int16_t  borg_kills_summoner;   /* index of a summoning guy */
extern int16_t  borg_kills_nxt;
extern borg_kill* borg_kills;


/*
 * Hack -- depth readiness
 */
extern int      borg_ready_morgoth;

/*
 * Hack -- extra fear per "region"
 */

extern uint16_t borg_fear_region[(AUTO_MAX_Y / 11) + 1][(AUTO_MAX_X / 11) + 1];
extern uint16_t borg_fear_monsters[AUTO_MAX_Y + 1][AUTO_MAX_X + 1];


/*
 * Hack -- count racial appearances per level
 */

extern int16_t* borg_race_count;


/*
 * Hack -- count racial kills (for uniques)
 */

extern int16_t* borg_race_death;


/*
 * Current "grid" list
 */

extern borg_grid* borg_grids[AUTO_MAX_Y];   /* Current "grid list" */

/*
 * Maintain a set of grids (liteable grids)
 */

extern int16_t  borg_LIGHT_n;
extern uint8_t  borg_LIGHT_y[AUTO_LIGHT_MAX];
extern uint8_t  borg_LIGHT_x[AUTO_LIGHT_MAX];

/*
 * Maintain a set of glow grids (liteable grids)
 */

extern int16_t  borg_glow_n;
extern uint8_t  borg_glow_y[AUTO_LIGHT_MAX];
extern uint8_t  borg_glow_x[AUTO_LIGHT_MAX];


/*
 * Maintain a set of grids (viewable grids)
 */

extern int16_t  borg_view_n;
extern uint8_t  borg_view_y[AUTO_VIEW_MAX];
extern uint8_t  borg_view_x[AUTO_VIEW_MAX];


/*
 * Maintain a set of grids (scanning arrays)
 */

extern int16_t  borg_temp_n;
extern uint8_t  borg_temp_y[AUTO_TEMP_MAX];
extern uint8_t  borg_temp_x[AUTO_TEMP_MAX];

/*
 * Maintain a temporary set of grids
 * Used to store lit grid info
 */
extern int16_t  borg_temp_lit_n;
extern uint8_t  borg_temp_lit_x[AUTO_TEMP_MAX];
extern uint8_t  borg_temp_lit_y[AUTO_TEMP_MAX];

/*
 * Maintain a set of special grids used for Teleport Other
 */
extern int16_t  borg_tp_other_n;
extern uint8_t  borg_tp_other_x[255];
extern uint8_t  borg_tp_other_y[255];
extern int      borg_tp_other_index[255];

extern uint8_t  offset_y;
extern uint8_t  offset_x;


/*
 * Maintain a set of grids (flow calculations)
 */

extern int16_t  borg_flow_n;
extern uint8_t  borg_flow_y[AUTO_FLOW_MAX];
extern uint8_t  borg_flow_x[AUTO_FLOW_MAX];


/*
 * Hack -- use "flow" array as a queue
 */

extern int      flow_head;
extern int      flow_tail;


/*
 * Some variables
 */

extern borg_data* borg_data_flow;   /* Current "flow" data */
extern borg_data* borg_data_cost;   /* Current "cost" data */
extern borg_data* borg_data_hard;   /* Constant "hard" data */
extern borg_data* borg_data_know;   /* Current "know" flags */
extern borg_data* borg_data_icky;   /* Current "icky" flags */


/*
 * Strategy flags -- recalculate things
 */

extern bool     borg_danger_wipe;       /* Recalculate danger */

extern bool     borg_do_update_view;    /* Recalculate view */
extern bool     borg_do_update_lite;    /* Recalculate lite */

/*
 * Strategy flags -- examine the world
 */

extern bool     borg_do_inven;      /* Acquire "inven" info */
extern bool     borg_do_equip;      /* Acquire "equip" info */
extern bool     borg_do_panel;      /* Acquire "panel" info */
extern bool     borg_do_frame;      /* Acquire "frame" info */
extern bool     borg_do_spell;      /* Acquire "spell" info */
extern uint8_t  borg_do_spell_aux;  /* Hack -- book for "borg_do_spell" */
extern bool     borg_do_browse;     /* Acquire "store" info */
extern uint8_t  borg_do_browse_what; /* Hack -- store for "borg_do_browse" */
extern uint8_t  borg_do_browse_more; /* Hack -- pages for "borg_do_browse" */


/*
 * Strategy flags -- run certain functions
 */

extern bool     borg_do_crush_junk;
extern bool     borg_do_crush_hole;
extern bool     borg_do_crush_slow;

/* am I fighting a unique */
extern int      borg_fighting_unique;
extern bool     borg_fighting_evil_unique;

/* am I fighting a summoner */
extern bool     borg_fighting_summoner;


extern const int borg_adj_mag_fail[STAT_RANGE];
extern const int borg_adj_mag_stat[STAT_RANGE];

extern struct player *borg_p;


/*** Some functions ***/

extern int  borg_lookup_kind(int tval, int sval);
extern bool borg_ego_has_random_power(struct ego_item* e_ptr);

/*
 * sort functions
 */
extern void borg_sort(void* u, void* v, int n);
extern bool borg_sort_comp_hook(void* u, void* v, int a, int b);
extern void borg_sort_swap_hook(void* u, void* v, int a, int b);
extern bool (*borg_sort_comp)(void* u, void* v, int a, int b);
extern void (*borg_sort_swap)(void* u, void* v, int a, int b);


/*
 * Queue a keypress
 */
extern errr borg_keypress(keycode_t k);

/*
 * Queue several keypresses
 */
extern errr borg_keypresses(const char* str);

/*
 * Dequeue a keypress
 */
extern keycode_t borg_inkey(bool take);

/*
 * Flush the keypresses
 */
extern void borg_flush(void);

/* save and retrieve direction when the */
/* command may be ambiguous */
extern void borg_queue_direction(keycode_t k);
extern keycode_t borg_get_queued_direction(void);

/*
 * Obtain some text from the screen (single character)
 */
extern errr borg_what_char(int x, int y, uint8_t* a, wchar_t* c);

/*
 * Obtain some text from the screen (multiple characters)
 */
extern errr borg_what_text(int x, int y, int n, uint8_t* a, char* s);


/*
 * Log a message to a file
 */
extern void borg_info(const char* what);

/*
 * Log a message, Search it, and Show/Memorize it in pieces
 */
extern void borg_note(const char* what);


/*
 * Abort the Borg, noting the reason
 */
extern void borg_oops(const char* what);


/*
 * Take a "memory note"
 */
extern bool borg_tell(char* what);

/*
 * Change the player name
 */
extern bool borg_change_name(char* str);

/*
 * Dump a character description
 */
extern bool borg_dump_character(char* str);

/*
 * Save the game (but do not quit)
 */
extern bool borg_save_game(void);


/*
 * Update the "frame" info from the screen
 */
extern void borg_update_frame(void);

/*
 * Calc a formula out in RPN
 */
extern int borg_calc_formula(int*);
/*
 * check out a formula in RPN
 */
extern int borg_check_formula(int*);
/*
 * return a string for the formula
 */
extern char* borg_prt_formula(int* formula);

/*
 * Print the string for an item
 */
extern const char* borg_prt_item(int item);

/*
 * Helper that converts old distance to new distance
 */
extern int borg_distance(int y, int x, int y2, int x2);
extern char* borg_massage_special_chars(char* name, char* memory);
extern bool borg_is_ammo(int tval);
/*
 * Initialize this file
 */
extern void borg_init_1(void);
extern void borg_clean_1(void);

#endif

#endif

