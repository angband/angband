
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
extern int sv_food_ration;
extern int sv_food_slime_mold;
extern int sv_food_draught;
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
extern int sv_potion_berserk;
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
    BORG_ATTACK_ARROW_FLAME,
    BORG_ATTACK_ARROW_FROST,
    BORG_ATTACK_ARROW_ANIMAL,
    BORG_ATTACK_ARROW_UNDEAD,
    BORG_ATTACK_ARROW_DEMON,
    BORG_ATTACK_ARROW_ORC,
    BORG_ATTACK_ARROW_TROLL,
    BORG_ATTACK_ARROW_GIANT,
    BORG_ATTACK_ARROW_DRAGON,
    BORG_ATTACK_ARROW_EVIL,
    BORG_ATTACK_ARROW_WOUNDING,
    BORG_ATTACK_ARROW_POISON,
    BORG_ATTACK_ARROW_SEEKER,
    BORG_ATTACK_ARROW_SILVER,
    BORG_ATTACK_ARROW_HOLY,
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
    * Threshold where the borg will start to carry / use Digger items
    */
#define BORG_DIG			14

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
    byte    x, y;       /* Location */
    s16b    when;       /* When last seen */
    int		value;		/* Estimated value of item */
    int 	tval;		/* Known tval */
};


/*
 * Monster information
 */

typedef struct borg_kill borg_kill;

struct borg_kill
{
    s16b    r_idx;      /* Race index */

    bool    known;      /* Verified race */
    bool    awake;      /* Probably awake */

    bool    confused;   /* Probably confused */
    bool    afraid;     /* Probably afraid */
    bool    quiver;     /* Probably quivering */
    bool    stunned;
    bool    poisoned;   /* Probably poisoned */

    bool    seen;       /* Assigned motion */
    bool    used;       /* Assigned message */

    byte    x, y;       /* Location */

    byte    ox, oy;     /* Old location */

    byte    speed;      /* Estimated speed */
    byte    moves;      /* Estimates moves */
    byte    ranged_attack; /* qty of ranged attacks */
    byte	spell[RSF_MAX];		/* spell flag for monster spells */
    s16b    power;      /* Estimated hit-points */
    s16b	injury;		/* Percent wounded */
    s16b    other;      /* Estimated something */
    s16b    level;      /* Monsters Level */
    u32b	spell_flags[RF_MAX]; /* Monster race spell flags preloaded */
    s16b    when;       /* When last seen */
    s16b	m_idx;		/* Game's index */
};



/*
 * Forward declare
 */
typedef struct borg_grid borg_grid;

/*
 * A grid in the dungeon.  Several bytes.
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
    byte feat;      /* Grid type */
    byte info;      /* Grid flags */
    bool trap;
    bool glyph;
    byte store;

    byte take;      /* Object index */
    byte kill;      /* Monster index */

    byte hmmm;      /* Extra field (unused) */

    byte xtra;      /* Extra field (search count) */
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
    byte data[AUTO_MAX_Y][AUTO_MAX_X];
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
extern bool borg_active;        /* Actually active */
extern bool borg_resurrect;     /* Continous play mode */
extern bool borg_cancel;        /* Being cancelled */
extern bool borg_scumming_pots; /* Borg will quickly store pots in home */

extern char genocide_target;    /* Identity of the poor unsuspecting soul */
extern int zap_slot;            /* to avoid a nasty game bug with amnesia */
extern bool borg_casted_glyph;  /* we dont have a launch messages anymore */
extern int borg_stop_dlevel;
extern int borg_stop_clevel;
extern int borg_no_deeper;
extern bool borg_stop_king;
extern bool borg_dont_react;
extern int successful_target;
extern int sold_item_tval[10];
extern int sold_item_sval[10];
extern int sold_item_pval[10];
extern int sold_item_store[10];
extern int sold_item_num;
extern int sold_item_nxt;
extern int bought_item_tval[10];
extern int bought_item_sval[10];
extern int bought_item_pval[10];
extern int bought_item_store[10];
extern int bought_item_num;
extern int bought_item_nxt;

extern char * shop_menu_items;
extern byte borg_nasties_num;
extern byte borg_nasties_count[7];
extern char borg_nasties[7];
extern byte borg_nasties_limit[7];

/* options from the borg.txt file */
extern int borg_respawn_race;
extern int borg_respawn_class;
extern int borg_respawn_str;
extern int borg_respawn_int;
extern int borg_respawn_wis;
extern int borg_respawn_dex;
extern int borg_respawn_con;
extern int borg_dump_level;
extern int borg_save_death;
extern bool borg_respawn_winners;
extern int borg_delay_factor;

extern bool borg_worships_damage;
extern bool borg_worships_speed;
extern bool borg_worships_hp;
extern bool borg_worships_mana;
extern bool borg_worships_ac;
extern bool borg_worships_gold;
extern bool borg_plays_risky;
extern bool borg_init_failure;
extern bool borg_uses_swaps;
extern bool borg_uses_calcs;
extern bool borg_slow_optimizehome;
extern bool borg_scums_uniques;
extern bool borg_kills_uniques;
extern int borg_chest_fail_tolerance;
extern s32b borg_money_scum_amount;
extern int borg_money_scum_who;
extern int borg_money_scum_ware;
extern bool borg_self_scum;
extern bool borg_lunal_mode;
extern bool borg_self_lunal;
extern bool borg_verbose;
extern bool borg_munchkin_start;
extern bool borg_munchkin_mode;
extern int borg_munchkin_level;
extern int borg_munchkin_depth;
extern int borg_enchant_limit; /* how high to enchant items */


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
    int depth;
    int cnd;
    int item;
    int from;
    int to;
    int power;
    bool each;
} power_item;

extern power_item** borg_power_item;
extern int* n_pwr;
extern int* borg_has;
extern int* borg_skill;
extern int size_depth;
extern int size_obj;

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
    BI_QUIVER_SLOTS,
    BI_FIRST_CURSED,

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
    BI_CRSUNKNO,
    BI_CRSAGRV,
    BI_CRSHPIMP,
    BI_CRSMPIMP,
    BI_CRSFEAR,
    BI_CRSDRAIN_XP,
    BI_CRSFVULN,
    BI_CRSEVULN,
    BI_CRSCVULN,
    BI_CRSAVULN,
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
    BI_WEIGHT,

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

extern int* formula[1000];
extern const char* prefix_pref[];

/*
 * Hack -- optional cheating flags
 */

 /*
  * Various silly flags
  */

extern bool borg_flag_save;     /* Save savefile at each level */

extern bool borg_flag_dump;     /* Save savefile at each death */

extern bool borg_save; /* do a save next time we get to press a key! */

extern bool borg_borg_message;      /* List borg messages or not */
extern bool borg_graphics;          /* List borg messages or not */
extern bool borg_confirm_target;

extern char borg_engine_date[];       /* last update */

/*
 * Use a simple internal random number generator
 */

extern bool borg_rand_quick;        /* Save system setting */

extern u32b borg_rand_value;        /* Save system setting */

extern u32b borg_rand_local;        /* Save personal setting */


/*
 * Hack -- time variables
 */

extern s16b borg_t;        /* Current "time" */
extern s16b borg_t_morgoth;
extern s16b need_see_inviso;        /* To tell me to cast it */
extern s16b borg_see_inv;
extern bool need_shift_panel;        /* to spot offscreeners */
extern s16b when_shift_panel;
extern s16b time_this_panel;        /* Current "time" for current panel*/
extern bool vault_on_level;     /* borg will search for a vault */
extern int unique_on_level;
extern bool scaryguy_on_level;
extern bool morgoth_on_level;
extern bool borg_morgoth_position;
extern int borg_t_antisummon;		/* Timestamp when in a AS spot */
extern bool borg_as_position;		/* Sitting in an anti-summon corridor */
extern bool borg_digging;			/* used in Anti-summon corridor */

extern bool breeder_level;      /* Borg will shut doors */
extern s16b old_depth;
extern s16b borg_respawning;       /* to prevent certain crashes */
extern s16b borg_no_retreat;

/*
 * Hack -- Other time variables
 */

extern s16b when_call_LIGHT; /* When we last did call light */
extern s16b when_wizard_LIGHT;   /* When we last did wizard light */

extern s16b when_detect_traps;  /* When we last detected traps */
extern s16b when_detect_doors;  /* When we last detected doors */
extern s16b when_detect_walls;  /* When we last detected walls */
extern s16b when_detect_evil;
extern s16b when_detect_obj;
extern s16b when_last_kill_mult;   /* When a multiplier was last killed */

extern bool my_need_alter;     /* incase of walls/doors */
extern bool my_no_alter;     /* incase of walls/doors */
extern bool my_need_redraw;     /* incase of walls/doors */
extern bool borg_attempting_refresh_resist;  /* for the Resistance spell */

/*
 * Some information
 */

extern s16b goal;       /* Flowing (goal type) */

extern bool goal_rising;    /* Currently returning to town */

extern bool goal_leaving;   /* Currently leaving the level */

extern bool goal_fleeing;   /* Currently fleeing the level */

extern bool goal_fleeing_lunal;   /* Currently fleeing the level in lunal*/
extern bool goal_fleeing_munchkin; /* Fleeing level while in munchkin Mode */

extern bool borg_fleeing_town; /* Currently fleeing the level to return to town */

extern bool goal_ignoring;  /* Currently ignoring monsters */

extern int goal_recalling;  /* Currently waiting for recall, guessing turns left */
extern bool goal_less;      /* return to, but dont use, the next up stairs */

extern s16b borg_times_twitch; /* how often twitchy on this level */
extern s16b borg_escapes; /* how often teleported on this level */

extern bool stair_less;     /* Use the next "up" staircase */
extern bool stair_more;     /* Use the next "down" staircase */

extern s32b borg_began;     /* When this level began */
extern s32b borg_time_town; /* how long it has been since I was in town */

extern s16b avoidance;      /* Current danger thresh-hold */

extern bool borg_failure;   /* Notice failure */

extern bool borg_simulate;  /* Simulation flag */
extern bool borg_attacking; /* Are we attacking a monster? */
extern bool borg_offsetting; /* Are we attacking a monster? with offsett balls*/

extern bool borg_completed; /* Completed the level */
extern bool borg_on_upstairs;      /* used when leaving a level */
extern bool borg_on_dnstairs;      /* used when leaving a level */
extern bool borg_needs_searching;  /* borg will search with each step */
extern s16b borg_oldchp;		/* hit points last game turn */
extern s16b borg_oldcsp;		/* mana points last game turn */

/* defence flags */
extern bool borg_prot_from_evil;
extern bool borg_speed;
extern bool borg_bless;
extern bool borg_hero;
extern bool borg_berserk;
extern bool borg_fastcast;
extern bool borg_regen;
extern bool borg_smite_evil;
extern bool borg_venom;
extern s16b borg_game_ratio;
extern s16b borg_resistance;
extern s16b borg_no_rest_prep; /* borg wont rest for a few turns */
extern bool borg_shield;
extern bool borg_on_glyph; /* borg is standing on a glyph of warding */
extern bool borg_create_door; /* borg is going to create doors */
extern bool borg_sleep_spell;
extern bool borg_sleep_spell_ii;
extern bool borg_crush_spell;
extern bool borg_slow_spell;
extern bool borg_confuse_spell;
extern bool borg_fear_mon_spell;


/*
 * Shop goals
 */
extern bool borg_in_shop;
extern s16b goal_shop;      /* Next shop to visit */
extern s16b goal_ware;      /* Next item to buy there */
extern s16b goal_item;      /* Next item to sell there */
extern int borg_food_onsale;      /* Are shops selling food? */
extern int borg_fuel_onsale;      /* Are shops selling fuel? */
extern bool borg_needs_quick_shopping; /* Needs to buy without browsing all shops */
extern s16b borg_best_fit_item;   /* Item to be worn, not sold */
extern int borg_best_item;

/*
 * Other variables
 */

extern int w_x;         /* Current panel offset (X) */
extern int w_y;         /* Current panel offset (Y) */
extern int morgy_panel_y;
extern int morgy_panel_x;

extern int borg_target_y;
extern int borg_target_x;  /* Current targetted location */

extern int c_x;         /* Current location (X) */
extern int c_y;         /* Current location (Y) */

extern int g_x;         /* Goal location (X) */
extern int g_y;         /* Goal location (Y) */

/*
 * Some estimated state variables
 */

extern s16b my_stat_max[STAT_MAX]; /* Current "maximal" stat values    */
extern s16b my_stat_cur[STAT_MAX]; /* Current "natural" stat values    */
extern s16b my_stat_ind[STAT_MAX]; /* Current "additions" to stat values   */
extern bool my_need_stat_check[STAT_MAX];  /* do I need to check my stats */

extern s16b my_stat_add[STAT_MAX];  /* aditions to stats */

extern s16b home_stat_add[STAT_MAX];

extern int  weapon_swap;   /* location of my swap weapon   */
extern s32b weapon_swap_value;   /* value of my swap weapon   */
extern int  armour_swap;   /* location of my swap weapon   */
extern s32b armour_swap_value;   /* value of my swap weapon   */

extern bool decurse_weapon_swap;  /* my swap is great, except its cursed */
extern int enchant_weapon_swap_to_h;  /* my swap is great, except its cursed */
extern int enchant_weapon_swap_to_d;  /* my swap is great, except its cursed */
extern bool decurse_armour_swap;  /* my swap is great, except its cursed */
extern int enchant_armour_swap_to_a;  /* my swap is great, except its cursed */


extern s16b weapon_swap_digger;
extern byte  weapon_swap_slay_animal;
extern byte  weapon_swap_slay_evil;
extern byte  weapon_swap_slay_undead;
extern byte  weapon_swap_slay_demon;
extern byte  weapon_swap_slay_orc;
extern byte  weapon_swap_slay_troll;
extern byte  weapon_swap_slay_giant;
extern byte  weapon_swap_slay_dragon;
extern byte  weapon_swap_impact;
extern byte  weapon_swap_brand_acid;
extern byte  weapon_swap_brand_elec;
extern byte  weapon_swap_brand_fire;
extern byte  weapon_swap_brand_cold;
extern byte  weapon_swap_brand_pois;
extern byte  weapon_swap_see_infra;
extern byte  weapon_swap_slow_digest;
extern byte  weapon_swap_aggravate;
extern byte  weapon_swap_bad_curse;
extern byte  weapon_swap_regenerate;
extern byte  weapon_swap_telepathy;
extern byte  weapon_swap_light;
extern byte  weapon_swap_see_invis;
extern byte  weapon_swap_ffall;
extern byte  weapon_swap_free_act;
extern byte  weapon_swap_hold_life;
extern byte  weapon_swap_immune_fire;
extern byte  weapon_swap_immune_acid;
extern byte  weapon_swap_immune_cold;
extern byte  weapon_swap_immune_elec;
extern byte  weapon_swap_resist_acid;
extern byte  weapon_swap_resist_elec;
extern byte  weapon_swap_resist_fire;
extern byte  weapon_swap_resist_cold;
extern byte  weapon_swap_resist_pois;
extern byte  weapon_swap_resist_conf;
extern byte  weapon_swap_resist_sound;
extern byte  weapon_swap_resist_light;
extern byte  weapon_swap_resist_dark;
extern byte  weapon_swap_resist_chaos;
extern byte  weapon_swap_resist_disen;
extern byte  weapon_swap_resist_shard;
extern byte  weapon_swap_resist_nexus;
extern byte  weapon_swap_resist_blind;
extern byte  weapon_swap_resist_neth;
extern byte  weapon_swap_resist_fear;
extern byte  armour_swap_slay_animal;
extern byte  armour_swap_slay_evil;
extern byte  armour_swap_slay_undead;
extern byte  armour_swap_slay_demon;
extern byte  armour_swap_slay_orc;
extern byte  armour_swap_slay_troll;
extern byte  armour_swap_slay_giant;
extern byte  armour_swap_slay_dragon;
extern byte  armour_swap_impact;
extern byte  armour_swap_brand_acid;
extern byte  armour_swap_brand_elec;
extern byte  armour_swap_brand_fire;
extern byte  armour_swap_brand_cold;
extern byte  armour_swap_brand_pois;
extern byte  armour_swap_see_infra;
extern byte  armour_swap_slow_digest;
extern byte  armour_swap_aggravate;
extern byte  armour_swap_bad_curse;
extern byte  armour_swap_regenerate;
extern byte  armour_swap_telepathy;
extern byte  armour_swap_light;
extern byte  armour_swap_see_invis;
extern byte  armour_swap_ffall;
extern byte  armour_swap_free_act;
extern byte  armour_swap_hold_life;
extern byte  armour_swap_immune_fire;
extern byte  armour_swap_immune_acid;
extern byte  armour_swap_immune_cold;
extern byte  armour_swap_immune_elec;
extern byte  armour_swap_resist_acid;
extern byte  armour_swap_resist_elec;
extern byte  armour_swap_resist_fire;
extern byte  armour_swap_resist_cold;
extern byte  armour_swap_resist_pois;
extern byte  armour_swap_resist_conf;
extern byte  armour_swap_resist_sound;
extern byte  armour_swap_resist_LIGHT;
extern byte  armour_swap_resist_dark;
extern byte  armour_swap_resist_chaos;
extern byte  armour_swap_resist_disen;
extern byte  armour_swap_resist_shard;
extern byte  armour_swap_resist_nexus;
extern byte  armour_swap_resist_blind;
extern byte  armour_swap_resist_neth;
extern byte  armour_swap_resist_fear;

extern s16b my_need_enchant_to_a;   /* Need some enchantment */
extern s16b my_need_enchant_to_h;   /* Need some enchantment */
extern s16b my_need_enchant_to_d;   /* Need some enchantment */
extern s16b my_need_brand_weapon;  /* brand bolts */
extern s16b my_need_id;			/* need to buy ID for an inventory item */


/*
 * Hack -- basic "power"
 */

extern s32b my_power;


/*
 * Various "amounts" (for the player)
 */

extern s16b amt_food_lowcal;
extern s16b amt_food_hical;

extern s16b amt_slow_poison;
extern s16b amt_cure_confusion;
extern s16b amt_cure_blind;

extern s16b amt_cool_staff;  /* holiness-power staff */
extern s16b amt_cool_wand;	/* # of charges */
extern s16b amt_book[9];

extern s16b amt_add_stat[6];
extern s16b amt_inc_stat[6];

extern s16b amt_fix_exp;

extern s16b amt_enchant_to_a;
extern s16b amt_enchant_to_d;
extern s16b amt_enchant_to_h;
extern s16b amt_brand_weapon;  /* cubragol and bolts */
extern s16b amt_enchant_weapon;
extern s16b amt_enchant_armor;
extern s16b amt_digger;
extern s16b amt_ego;

/*
 * Various "amounts" (for the home)
 */

extern s16b num_food;
extern s16b num_fuel;
extern s16b num_mold;
extern s16b num_ident;
extern s16b num_recall;
extern s16b num_phase;
extern s16b num_escape;
extern s16b num_tele_staves;
extern s16b num_teleport;
extern s16b num_berserk;
extern s16b num_teleport_level;
extern s16b num_recharge;

extern s16b num_cure_critical;
extern s16b num_cure_serious;

extern s16b num_pot_rheat;
extern s16b num_pot_rcold;

extern s16b num_missile;

extern s16b num_book[9];

extern s16b num_fix_stat[7];

extern s16b num_fix_exp;
extern s16b num_mana;
extern s16b num_heal;
extern s16b num_heal_true;
extern s16b num_ezheal;
extern s16b num_ezheal_true;
extern s16b num_life;
extern s16b num_life_true;
extern s16b num_pfe;
extern s16b num_glyph;
extern s16b num_detonate;
extern s16b num_mush_second_sight;		/* esp */
extern s16b num_mush_fast_recovery;		/* cure stun, cut, pois, blind */
extern s16b num_mush_restoring;			/* Restore All */
extern s16b num_mush_cure_mind;			/* Cure confustion, Halluc, fear, tmp resist Conf */
extern s16b num_mush_emergency;			/* Hallucinate, Oppose Fire, Oppose Cold, Heal 200 */
extern s16b num_mush_terror;			/* Terror --give +5 speed boost */
extern s16b num_mush_stoneskin;			/* StoneSkin */
extern s16b num_mush_debility;			/* Mana Restore, temp loss of a stat (str/con) */
extern s16b num_mush_sprinting;			/* Sprinting (speed +10) */
extern s16b num_mush_purging;			/* Purging --Makes hungry, restore Str/Con, Cure Pois */

extern s16b num_enchant_to_a;
extern s16b num_enchant_to_d;
extern s16b num_enchant_to_h;
extern s16b num_brand_weapon;  /*  crubragol and bolts */
extern s16b num_genocide;
extern s16b num_mass_genocide;

extern s16b num_artifact;
extern s16b num_ego;

extern s16b home_slot_free;
extern s16b home_damage;
extern s16b num_duplicate_items;
extern s16b num_slow_digest;
extern s16b num_regenerate;
extern s16b num_telepathy;
extern s16b num_LIGHT;
extern s16b num_see_inv;

extern s16b num_invisible; /**/

extern s16b num_ffall;
extern s16b num_free_act;
extern s16b num_hold_life;
extern s16b num_immune_acid;
extern s16b num_immune_elec;
extern s16b num_immune_fire;
extern s16b num_immune_cold;
extern s16b num_resist_acid;
extern s16b num_resist_elec;
extern s16b num_resist_fire;
extern s16b num_resist_cold;
extern s16b num_resist_pois;
extern s16b num_resist_conf;
extern s16b num_resist_sound;
extern s16b num_resist_LIGHT;
extern s16b num_resist_dark;
extern s16b num_resist_chaos;
extern s16b num_resist_disen;
extern s16b num_resist_shard;
extern s16b num_resist_nexus;
extern s16b num_resist_blind;
extern s16b num_resist_neth;
extern s16b num_sustain_str;
extern s16b num_sustain_int;
extern s16b num_sustain_wis;
extern s16b num_sustain_dex;
extern s16b num_sustain_con;
extern s16b num_sustain_all;

extern s16b num_speed;
extern s16b num_edged_weapon;
extern s16b num_bad_gloves;
extern s16b num_weapons;
extern s16b num_bow;
extern s16b num_rings;
extern s16b num_neck;
extern s16b num_armor;
extern s16b num_cloaks;
extern s16b num_shields;
extern s16b num_hats;
extern s16b num_gloves;
extern s16b num_boots;

/*
 * Deal with knowing which uniques are alive
 */
extern int borg_numb_live_unique;
extern int borg_living_unique_index;
extern int borg_unique_depth;

/*
 * Hack -- extra state variables
 */

extern int borg_feeling_danger;   /* Current level "feeling" */
extern int borg_feeling_stuff;    /* Current level "feeling" */

/*
 * Hack -- current shop index
 */

extern s16b shop_num;       /* Current shop index */



/*
 * State variables extracted from the screen
 */

extern s32b borg_exp;       /* Current experience */

extern s32b borg_gold;      /* Current gold */

extern int borg_stat[6];    /* Current stats */

extern int borg_book[9];    /* Current book slots */


/*
 * State variables extracted from the inventory/equipment
 */

extern int borg_cur_wgt;    /* Current weight */


/*
 * Constant state variables
 */

extern int borg_race;       /* Current race */
extern int borg_class;      /* Current class */



/*
 * Constant state structures
 */


extern void mmove2(int* y, int* x, int y1, int x1, int y2, int x2);

/*
 * Number of turns to step for (zero means forever)
 */
extern u16b borg_step;      /* Step count (if any) */


/*
 * Status message search string
 */
extern char borg_match[128];    /* Search string */


/*
 * Log file
 */
extern FILE* borg_fff;      /* Log file */



/*
 * Hack -- the detection arrays
 */

extern bool borg_detect_wall[6][18];

extern bool borg_detect_trap[6][18];

extern bool borg_detect_door[6][18];

extern bool borg_detect_evil[6][18];

extern bool borg_detect_obj[6][18];

/*
 * Locate the store doors
 */

extern int* track_shop_x;
extern int* track_shop_y;


/*
 * Track "stairs up"
 */

extern s16b track_less_num;
extern s16b track_less_size;
extern int* track_less_x;
extern int* track_less_y;


/*
 * Track "stairs down"
 */

extern s16b track_more_num;
extern s16b track_more_size;
extern int* track_more_x;
extern int* track_more_y;

/*
 * Track glyphs
 */
extern s16b track_glyph_num;
extern s16b track_glyph_size;
extern int* track_glyph_x;
extern int* track_glyph_y;

extern bool borg_needs_new_sea;

/*
 * Track the items worn to avoid loops
 */
extern s16b track_worn_num;
extern s16b track_worn_size;
extern s16b track_worn_time;
extern byte* track_worn_name1;

extern const s16b borg_ddx_ddd[24];
extern const s16b borg_ddy_ddd[24];

/*
 * Track steps
 */
extern s16b track_step_num;
extern s16b track_step_size;
extern int* track_step_x;
extern int* track_step_y;

/*
 * Track closed doors
 */
extern s16b track_door_num;
extern s16b track_door_size;
extern int* track_door_x;
extern int* track_door_y;

/*
 * Track closed doors which started closed
 */
extern s16b track_closed_num;
extern s16b track_closed_size;
extern int* track_closed_x;
extern int* track_closed_y;

/*
 * Track the mineral veins with treasure
 *
 */
extern s16b track_vein_num;
extern s16b track_vein_size;
extern int* track_vein_x;
extern int* track_vein_y;

/*
 * The object list.  This list is used to "track" objects.
 */

extern s16b borg_takes_cnt;

extern s16b borg_takes_nxt;

extern borg_take* borg_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

extern s16b borg_kills_cnt;
extern s16b borg_kills_summoner;   /* index of a summoning guy */
extern s16b borg_kills_nxt;

extern borg_kill* borg_kills;


/*
 * Hack -- depth readiness
 */
extern int borg_ready_morgoth;

/*
 * Hack -- extra fear per "region"
 */

extern u16b borg_fear_region[6][18];
extern u16b borg_fear_monsters[AUTO_MAX_Y + 1][AUTO_MAX_X + 1];


/*
 * Hack -- count racial appearances per level
 */

extern s16b* borg_race_count;


/*
 * Hack -- count racial kills (for uniques)
 */

extern s16b* borg_race_death;


/*
 * Current "grid" list
 */

extern borg_grid* borg_grids[AUTO_MAX_Y];   /* Current "grid list" */

/*
 * Maintain a set of grids (liteable grids)
 */

extern s16b borg_LIGHT_n;
extern byte borg_LIGHT_y[AUTO_LIGHT_MAX];
extern byte borg_LIGHT_x[AUTO_LIGHT_MAX];

/*
 * Maintain a set of glow grids (liteable grids)
 */

extern s16b borg_glow_n;
extern byte borg_glow_y[AUTO_LIGHT_MAX];
extern byte borg_glow_x[AUTO_LIGHT_MAX];


/*
 * Maintain a set of grids (viewable grids)
 */

extern s16b borg_view_n;
extern byte borg_view_y[AUTO_VIEW_MAX];
extern byte borg_view_x[AUTO_VIEW_MAX];


/*
 * Maintain a set of grids (scanning arrays)
 */

extern s16b borg_temp_n;
extern byte borg_temp_y[AUTO_TEMP_MAX];
extern byte borg_temp_x[AUTO_TEMP_MAX];

/*
 * Maintain a temporary set of grids
 * Used to store lit grid info
 */
extern s16b borg_temp_lit_n;
extern byte borg_temp_lit_x[AUTO_TEMP_MAX];
extern byte borg_temp_lit_y[AUTO_TEMP_MAX];

/*
 * Maintain a set of special grids used for Teleport Other
 */
extern s16b borg_tp_other_n;
extern byte borg_tp_other_x[255];
extern byte borg_tp_other_y[255];
extern int borg_tp_other_index[255];

extern byte offset_y;
extern byte offset_x;


/*
 * Maintain a set of grids (flow calculations)
 */

extern s16b borg_flow_n;
extern byte borg_flow_y[AUTO_FLOW_MAX];
extern byte borg_flow_x[AUTO_FLOW_MAX];


/*
 * Hack -- use "flow" array as a queue
 */

extern int flow_head;
extern int flow_tail;


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

extern bool borg_danger_wipe;       /* Recalculate danger */

extern bool borg_do_update_view;       /* Recalculate view */

extern bool borg_do_update_LIGHT;       /* Recalculate lite */

/*
 * Strategy flags -- examine the world
 */

extern bool borg_do_inven;      /* Acquire "inven" info */

extern bool borg_do_equip;      /* Acquire "equip" info */

extern bool borg_do_panel;      /* Acquire "panel" info */

extern bool borg_do_frame;      /* Acquire "frame" info */

extern bool borg_do_spell;      /* Acquire "spell" info */

extern byte borg_do_spell_aux;      /* Hack -- book for "borg_do_spell" */

extern bool borg_do_browse;     /* Acquire "store" info */

extern byte borg_do_browse_what;    /* Hack -- store for "borg_do_browse" */

extern byte borg_do_browse_more;    /* Hack -- pages for "borg_do_browse" */


/*
 * Strategy flags -- run certain functions
 */

extern bool borg_do_crush_junk;

extern bool borg_do_crush_hole;

extern bool borg_do_crush_slow;

/* am I fighting a unique */
extern int borg_fighting_unique;
extern bool borg_fighting_evil_unique;

/* am I fighting a summoner */
extern bool borg_fighting_summoner;

extern const int borg_adj_mag_fail[STAT_RANGE];
extern const int borg_adj_mag_stat[STAT_RANGE];

/*** Some functions ***/

extern int borg_lookup_kind(int tval, int sval);
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
extern errr borg_what_char(int x, int y, byte* a, wchar_t* c);

/*
 * Obtain some text from the screen (multiple characters)
 */
extern errr borg_what_text(int x, int y, int n, byte* a, char* s);


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

#endif

#endif

