
/* File: borg1.h */
/* Purpose: Header file for "borg1.c" -BEN- */

#ifndef INCLUDED_BORG1_H
#define INCLUDED_BORG1_H

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"



#ifdef ALLOW_BORG

/* Allocate a wiped array of type T[N], assign to pointer P */
#define C_MAKE(P, N, T) \
	((P) = C_ZNEW(N, T))

/* Allocate a wiped thing of type T, assign to pointer P */
#define MAKE(P, T) \
	((P) = ZNEW(T))

/* Mega-Hack - indices of the player classes */

#define CLASS_WARRIOR            0
#define CLASS_MAGE               1
#define CLASS_PRIEST             2
#define CLASS_ROGUE              3
#define CLASS_RANGER             4
#define CLASS_PALADIN            5

#include "../effects.h"



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
#define AUTO_MAX_X  DUNGEON_WID
#define AUTO_MAX_Y  DUNGEON_HGT

/* The borg_has[] needs to know certain numbers */
#define SHROOM_STONESKIN	 22
#define POTION_HEAL			321 /* lookup_kind(TV_POTION, SV_POTION_HEAL) */
#define POTION_STAR_HEAL	322
#define POTION_LIFE			323
#define POTION_RES_MANA		325
/* #define POTION_INVULN		238 */
/* #define POTION_RESISTANCE	268 */
#define SCROLL_STAR_CURSE	276
#define SCROLL_CURSE		275
#define SCROLL_GENOCIDE		283
#define SCROLL_MGENOCIDE	284
#define SCROLL_TPORTLEVEL	262
#define WAND_ANNILATION		396
#define WAND_MM				370
#define WAND_SCLOUD			375
#define WAND_TPORTOTHER		389
#define STAFF_TPORT			420
#define ROD_RECALL			455
#define ROD_HEAL			447
#define RING_FLAMES			207
#define RING_ACID			208
#define RING_ICE			209
#define RING_LIGHTNING		210


/* The sval of some items are needed and the game does not supply some any if it has an effect (activation) */
#define SV_POTION_INC_STR			1
#define SV_POTION_INC_INT			2
#define SV_POTION_INC_WIS			3
#define SV_POTION_INC_DEX			4
#define SV_POTION_INC_CON			5
#define SV_POTION_INC_CHR			6
#define SV_POTION_INC_ALL			7
#define SV_POTION_INC_EXP			8
#define SV_POTION_CURE_LIGHT		9
#define SV_POTION_CURE_SERIOUS		10
#define SV_POTION_CURE_CRITICAL		11
#define SV_POTION_HEALING			12
#define SV_POTION_STAR_HEALING		13
#define SV_POTION_LIFE				14
#define SV_POTION_CURE_POISON		15
#define SV_POTION_RESTORE_MANA		16
#define SV_POTION_RES_STR			17
#define SV_POTION_RES_INT			18
#define SV_POTION_RES_WIS			19
#define SV_POTION_RES_DEX			20
#define SV_POTION_RES_CON			21
#define SV_POTION_RES_CHR			22
#define SV_POTION_RESTORE_EXP		23 /* Restore Level */
#define SV_POTION_INC_STR2			24 /* Brawn, gain Str lose 1 other */
#define SV_POTION_INC_INT2			25 /* Intellect, gain Str lose 1 other */
#define SV_POTION_INC_WIS2			26 /* Contemplate, gain Str lose 1 other */
#define SV_POTION_INC_DEX2			27 /* Nimble, gain Str lose 1 other */
#define SV_POTION_INC_CON2			28 /* Toughness, gain Str lose 1 other */
#define SV_POTION_ENLIGHTENMENT		30
#define SV_POTION_STAR_ENLIGHTNMENT	31
#define SV_POTION_SPEED				32
#define SV_POTION_HEROISM			33
#define SV_POTION_BERSERK_STRENGTH	34
#define SV_POTION_BOLDNESS			35
#define SV_POTION_RESIST_HEAT		36
#define SV_POTION_RESIST_COLD		37
#define SV_POTION_RESIST_POIS		38
#define SV_POTION_DETECT_INVIS		39
#define SV_POTION_INFRAVISION		49
#define SV_POTION_SLIME_MOLD		50


#define SV_POTION_DRAGON_BREATH		58
#define SV_POTION_DETONATIONS		59


#define SV_ROD_RECALL				26
#define SV_ROD_IDENTIFY				27
#define SV_ROD_DETECT_TRAP			3
#define SV_ROD_DETECT_DOOR			2
#define SV_ROD_DETECTION			4
#define SV_ROD_ILLUMINATION			25
#define SV_ROD_SPEED				28
#define SV_ROD_MAPPING				5
#define SV_ROD_HEALING				18
#define SV_ROD_TELEPORT_OTHER		22
#define SV_ROD_LIGHT				24
#define SV_ROD_SLOW_MONSTER			15
#define SV_ROD_SLEEP_MONSTER		16
#define SV_ROD_ELEC_BOLT			9
#define SV_ROD_COLD_BOLT			8
#define SV_ROD_FIRE_BOLT			7
#define SV_ROD_ACID_BOLT			10
#define SV_ROD_DRAIN_LIFE			21
#define SV_ROD_ELEC_BALL			13
#define SV_ROD_COLD_BALL			12
#define SV_ROD_FIRE_BALL			11
#define SV_ROD_ACID_BALL			14
#define SV_ROD_CURING				17


#define SV_WAND_TELEPORT_AWAY		20
#define SV_WAND_STINKING_CLOUD		6
#define SV_WAND_MAGIC_MISSILE		1
#define SV_WAND_ANNIHILATION		27
#define SV_WAND_LIGHT				15
#define SV_WAND_WONDER				22
#define SV_WAND_SLOW_MONSTER		11
#define SV_WAND_SLEEP_MONSTER		13
#define SV_WAND_FEAR_MONSTER		14
#define SV_WAND_CONFUSE_MONSTER		12
#define SV_WAND_ELEC_BOLT			2
#define SV_WAND_COLD_BOLT			3
#define SV_WAND_ACID_BOLT			5
#define SV_WAND_FIRE_BOLT			4
#define SV_WAND_ELEC_BALL			7
#define SV_WAND_COLD_BALL			8
#define SV_WAND_ACID_BALL			10
#define SV_WAND_FIRE_BALL			9
#define SV_WAND_DRAGON_FIRE			23
#define SV_WAND_DRAGON_COLD			24
#define SV_WAND_DRAIN_LIFE			26


#define SV_STAFF_IDENTIFY			25
#define SV_STAFF_SPEED				22
#define SV_STAFF_HEALING			14
#define SV_STAFF_THE_MAGI			24
#define SV_STAFF_POWER				17
#define SV_STAFF_HOLINESS			18
#define SV_STAFF_TELEPORTATION		21
#define SV_STAFF_DESTRUCTION		5
#define SV_STAFF_CURING				12
#define SV_STAFF_SLEEP_MONSTERS		8
#define SV_STAFF_SLOW_MONSTERS		7
#define SV_STAFF_DISPEL_EVIL		15
#define SV_STAFF_BANISHMENT			16
#define SV_STAFF_DETECT_INVIS		9
#define SV_STAFF_DETECT_EVIL		10
#define SV_STAFF_LIGHT				19
#define SV_STAFF_MAPPING			11
#define SV_STAFF_REMOVE_CURSE		23


#define SV_FOOD_SECOND_SIGHT		6 /* esp */
#define SV_FOOD_FAST_RECOVERY		7 /* cure stun, cut, pois, blind */
#define SV_FOOD_RESTORING			8 /* Restore All */
#define SV_FOOD_CURE_MIND			9 /* Cure confustion, Halluc, fear, tmp resist Conf */
#define SV_FOOD_EMERGENCY			10 /* Hallucinate, Oppose Fire, Oppose Cold, Heal 200 */
#define SV_FOOD_TERROR				11 /* Terror --give +5 speed boost */
#define SV_FOOD_STONESKIN			12 /* StoneSkin */
#define SV_FOOD_DEBILITY			14 /* Mana Restore, temp loss of a stat (str/con) */
#define SV_FOOD_SPRINTING			15 /* Sprinting (speed +10) */
#define SV_FOOD_PURGING				16 /* Purging --Makes hungry, restore Str/Con, Cure Pois */


#define SV_SCROLL_TELEPORT			2
#define SV_SCROLL_TELEPORT_LEVEL	3
#define SV_SCROLL_MAPPING			4

#define SV_SCROLL_DETECT_TRAP		6
#define SV_SCROLL_DETECT_DOOR		7
#define SV_SCROLL_DETECT_INVIS		8

#define SV_SCROLL_ENCHANT_WEAPON_TO_HIT	11
#define SV_SCROLL_ENCHANT_WEAPON_TO_DAM	12
#define SV_SCROLL_ENCHANT_ARMOR		13
#define SV_SCROLL_STAR_ENCHANT_WEAPON	14
#define	SV_SCROLL_STAR_ENCHANT_ARMOR	15
#define	SV_SCROLL_REMOVE_CURSE		16
#define	SV_SCROLL_STAR_REMOVE_CURSE	17

#define SV_SCROLL_ACQUIREMENT		21
#define SV_SCROLL_STAR_ACQUIREMENT	22
#define SV_SCROLL_DISPEL_UNDEAD		23
#define SV_SCROLL_BANISHMENT		24
#define SV_SCROLL_MASS_BANISHMENT	25
#define SV_SCROLL_SATISFY_HUNGER	26
#define SV_SCROLL_IDENTIFY			27
#define SV_SCROLL_LIGHT				28

#define SV_SCROLL_RECHARGING		30
#define SV_SCROLL_TRAP_DOOR_DESTRUCTION	31
#define SV_SCROLL_BLESSING			33
#define SV_SCROLL_HOLY_CHANT		34
#define SV_SCROLL_HOLY_PRAYER		35
#define SV_SCROLL_PROTECTION_FROM_EVIL	36
#define SV_SCROLL_MONSTER_CONFUSION	37
#define SV_SCROLL_RUNE_PROTECTION	38


#define SV_RING_ACID				19
#define SV_RING_FIRE				18
#define SV_RING_COLD				20
#define SV_RING_ELEC				21
#define SV_PLANATIR					7
#define SV_PHIAL					4

#define EGO_AMAN					42 /* Where did not name2 go in 330? */
#define MAX_CLASSES					6 /* Max # of classes 0 = warrior, 5 = Paladin */
#define MAX_RACES					10
/*
 * Flags for the "info" field of grids
 *
 * Note that some of the flags below are not "perfect", in particular,
 * several of the flags should be treated as "best guesses", see below.
 *
 * The "BORG_MARK" flag means that the grid has been "observed", though
 * the terrain feature may or may not be memorized.  Note the use of the
 * "FEAT_NONE", "FEAT_FLOOR", and "FEAT_INVIS" feature codes below.
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
 * Some assistance with the borg_attack and magic arrows
 */
#define GF_ARROW_FLAME   70
#define GF_ARROW_FROST   71
#define GF_ARROW_ANIMAL  72
#define GF_ARROW_UNDEAD  73
#define GF_ARROW_DEMON   74
#define GF_ARROW_ORC     75
#define GF_ARROW_TROLL   76
#define GF_ARROW_GIANT   77
#define GF_ARROW_DRAGON  78
#define GF_ARROW_EVIL    79
#define GF_ARROW_WOUNDING 80
#define GF_ARROW_POISON  81
#define GF_ARROW_SEEKER  82
#define GF_ARROW_SILVER  83
#define GF_ARROW_HOLY	 84
#define GF_HOLY_WORD     85
#define GF_AWAY_ALL_MORGOTH 86

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
    struct object_kind *kind;      /* Kind */

    bool    known;      /* Verified kind */

    bool    seen;       /* Assigned motion */

    bool    extra;      /* Unused */

	bool	orbed;		/* Orb of Draining cast on it */

	byte    x, y;       /* Location */

    s16b    when;       /* When last seen */

	int		value;		/* Estimated value of item */

	int		tval;		/* Known tval */


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
    byte	spell[96];		/* spell flag for monster spells */
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
    (distance(((int)(Y1))<<1,((int)(X1))<<1,((int)(Y2))<<1,((int)(X2))<<1))



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

extern char shop_orig[28];
extern char shop_rogue[28];
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
extern int borg_respawn_chr;
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

/* HACK... this should really be a parm into borg_prepared */
/*         I am just being lazy */
extern bool borg_slow_return;

/* dynamic required items */
/* dynamic required items */
typedef struct req_item
{
   int depth;
   int item;
   int number;

} req_item;

extern req_item **borg_required_item;

extern int *n_req;
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

extern power_item **borg_power_item;
extern int *n_pwr;
extern int *borg_has;
extern int *borg_has_on;
extern int *borg_artifact;
extern int *borg_skill;
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
    BI_CHR,
    BI_CSTR,
    BI_CINT,
    BI_CWIS,
    BI_CDEX,
    BI_CCON,
    BI_CCHR,
    BI_SSTR,
    BI_SINT,
    BI_SWIS,
    BI_SDEX,
    BI_SCON,
    BI_SCHR,
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
    BI_SDIG,
    BI_FEATH,
    BI_REG,
    BI_SINV,
    BI_INFRA,
    BI_DIS,
    BI_DEV,
    BI_SAV,
    BI_STL,
    BI_SRCH,
    BI_SRCHFREQ,
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
    BI_ISSEARCHING,
    BI_ISFIXLEV,
    BI_ISFIXEXP,
    BI_ISFIXSTR,
    BI_ISFIXINT,
    BI_ISFIXWIS,
    BI_ISFIXDEX,
    BI_ISFIXCON,
    BI_ISFIXCHR,
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
    BI_CRSTELE,
    BI_CRSAGRV,
	BI_CRSHPIMP,
	BI_CRSMPIMP,
	BI_CRSFEAR,
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
    BI_AMISSILES,
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
	BI_ADETONATE, /* POTIONS used as weapons */
	BI_AMASSBAN,	/* ?Mass Banishment */
	BI_ASHROOM,
	BI_AROD1,		/* Attack rods */
	BI_AROD2,		/* Attack rods */
	BI_DINV,	/* See Inv Spell Legal */

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

extern int *formula[1000];
extern char *prefix_pref[];

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
extern s16b borg_game_ratio;
extern s16b borg_resistance;
extern s16b borg_no_rest_prep; /* borg wont rest for a few turns */
extern bool borg_shield;
extern bool borg_on_glyph; /* borg is standing on a glyph of warding */
extern bool borg_create_door; /* borg is going to create doors */
extern bool borg_sleep_spell;
extern bool borg_sleep_spell_ii;
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

extern int bad_obj_x[50];   /* Dropped cursed artifact at location (X) */
extern int bad_obj_y[50];   /* Dropped cursed artifact at location (Y) */
extern int bad_obj_cnt;

/*
 * Some estimated state variables
 */

extern s16b my_stat_max[6]; /* Current "maximal" stat values    */
extern s16b my_stat_cur[6]; /* Current "natural" stat values    */
extern s16b my_stat_use[6]; /* Current "resulting" stat values  */
extern s16b my_stat_ind[6]; /* Current "additions" to stat values   */
extern bool my_need_stat_check[6];  /* do I need to check my stats */

extern s16b my_stat_add[6];  /* aditions to stats */

extern s16b home_stat_add[6];

extern int  weapon_swap;   /* location of my swap weapon   */
extern s32b weapon_swap_value;   /* value of my swap weapon   */
extern int  armour_swap;   /* location of my swap weapon   */
extern s32b armour_swap_value;   /* value of my swap weapon   */

/* a 3 state boolean */
/*-1 = not cursed, no help needed for it */
/* 0 = light curse, needs light remove curse spell */
/* 1 = heavy curse, needs heavy remove curse spell */
extern int decurse_weapon_swap;  /* my swap is great, except its cursed */
extern int enchant_weapon_swap_to_h;  /* my swap is great, except its cursed */
extern int enchant_weapon_swap_to_d;  /* my swap is great, except its cursed */
extern int decurse_armour_swap;  /* my swap is great, except its cursed */
extern int enchant_armour_swap_to_a;  /* my swap is great, except its cursed */
extern bool borg_wearing_cursed;

extern s16b weapon_swap_digger;
extern byte  weapon_swap_slay_animal;
extern byte  weapon_swap_slay_evil;
extern byte  weapon_swap_slay_undead;
extern byte  weapon_swap_slay_demon;
extern byte  weapon_swap_slay_orc;
extern byte  weapon_swap_slay_troll;
extern byte  weapon_swap_slay_giant;
extern byte  weapon_swap_slay_dragon;
extern byte  weapon_swap_kill_undead;
extern byte  weapon_swap_kill_demon;
extern byte  weapon_swap_kill_dragon;
extern byte  weapon_swap_impact;
extern byte  weapon_swap_brand_acid;
extern byte  weapon_swap_brand_elec;
extern byte  weapon_swap_brand_fire;
extern byte  weapon_swap_brand_cold;
extern byte  weapon_swap_brand_pois;
extern byte  weapon_swap_see_infra;
extern byte  weapon_swap_slow_digest;
extern byte  weapon_swap_aggravate;
extern byte  weapon_swap_teleport;
extern byte  weapon_swap_regenerate;
extern byte  weapon_swap_telepathy;
extern byte  weapon_swap_LIGHT;
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
extern byte  weapon_swap_resist_LIGHT;
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
extern byte  armour_swap_kill_undead;
extern byte  armour_swap_kill_demon;
extern byte  armour_swap_kill_dragon;
extern byte  armour_swap_impact;
extern byte  armour_swap_brand_acid;
extern byte  armour_swap_brand_elec;
extern byte  armour_swap_brand_fire;
extern byte  armour_swap_brand_cold;
extern byte  armour_swap_brand_pois;
extern byte  armour_swap_see_infra;
extern byte  armour_swap_slow_digest;
extern byte  armour_swap_aggravate;
extern byte  armour_swap_teleport;
extern byte  armour_swap_regenerate;
extern byte  armour_swap_telepathy;
extern byte  armour_swap_LIGHT;
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

extern byte my_ammo_tval;   /* Ammo -- "tval"   */
extern byte my_ammo_sides;  /* Ammo -- "sides"  */
extern s16b my_ammo_power;  /* Shooting multipler   */

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
extern s16b amt_fix_stat[7];

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
extern s16b num_speed;
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

extern int borg_feeling;    /* Current level "feeling" */

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

extern player_magic *mb_ptr;    /* Player magic info */


extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);

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
extern FILE *borg_fff;      /* Log file */



/*
 * Hack -- the detection arrays
 */

extern bool borg_detect_wall[6][18];

extern bool borg_detect_trap[6][18];

extern bool borg_detect_door[6][18];

extern bool borg_detect_evil[6][18];

/*
 * Locate the store doors
 */

extern int *track_shop_x;
extern int *track_shop_y;


/*
 * Track "stairs up"
 */

extern s16b track_less_num;
extern s16b track_less_size;
extern int *track_less_x;
extern int *track_less_y;


/*
 * Track "stairs down"
 */

extern s16b track_more_num;
extern s16b track_more_size;
extern int *track_more_x;
extern int *track_more_y;

/*
 * Track glyphs
 */
extern s16b track_glyph_num;
extern s16b track_glyph_size;
extern int *track_glyph_x;
extern int *track_glyph_y;

extern bool borg_needs_new_sea;

/*
 * Track the items worn to avoid loops
 */
extern s16b track_worn_num;
extern s16b track_worn_size;
extern s16b track_worn_time;
extern byte *track_worn_name1;

extern const s16b borg_ddx_ddd[24];
extern const s16b borg_ddy_ddd[24];

/*
 * Track steps
 */
extern s16b track_step_num;
extern s16b track_step_size;
extern int *track_step_x;
extern int *track_step_y;

/*
 * Track closed doors
 */
extern s16b track_door_num;
extern s16b track_door_size;
extern int *track_door_x;
extern int *track_door_y;

/*
 * Track closed doors which started closed
 */
extern s16b track_closed_num;
extern s16b track_closed_size;
extern int *track_closed_x;
extern int *track_closed_y;

/*
 * Track the mineral veins with treasure
 *
 */
extern s16b track_vein_num;
extern s16b track_vein_size;
extern int *track_vein_x;
extern int *track_vein_y;

/*
 * The object list.  This list is used to "track" objects.
 */

extern s16b borg_takes_cnt;

extern s16b borg_takes_nxt;

extern borg_take *borg_takes;


/*
 * The monster list.  This list is used to "track" monsters.
 */

extern s16b borg_kills_cnt;
extern s16b borg_kills_summoner;   /* index of a summoning guy */
extern s16b borg_kills_nxt;

extern borg_kill *borg_kills;


/*
 * Hack -- depth readiness
 */
extern int borg_ready_morgoth;

/*
 * Hack -- extra fear per "region"
 */

extern u16b borg_fear_region[6][18];
extern u16b borg_fear_monsters[AUTO_MAX_Y][AUTO_MAX_X];


/*
 * Hack -- count racial appearances per level
 */

extern s16b *borg_race_count;


/*
 * Hack -- count racial kills (for uniques)
 */

extern s16b *borg_race_death;


/*
 * Current "grid" list
 */

extern borg_grid *borg_grids[AUTO_MAX_Y];   /* Current "grid list" */

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

extern borg_data *borg_data_flow;   /* Current "flow" data */

extern borg_data *borg_data_cost;   /* Current "cost" data */

extern borg_data *borg_data_hard;   /* Constant "hard" data */

extern borg_data *borg_data_know;   /* Current "know" flags */

extern borg_data *borg_data_icky;   /* Current "icky" flags */


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



/*** Some functions ***/

extern int borg_lookup_kind(int tval, int sval);

/*
 * Queue a keypress
 */
extern errr borg_keypress(keycode_t k);

/*
 * Queue several keypresses
 */
extern errr borg_keypresses(char *str);

/*
 * Dequeue a keypress
 */
extern keycode_t borg_inkey(bool take);

/*
 * Flush the keypresses
 */
extern void borg_flush(void);


/*
 * Obtain some text from the screen (single character)
 */
extern errr borg_what_char(int x, int y, byte *a, wchar_t *c);

/*
 * Obtain some text from the screen (multiple characters)
 */
extern errr borg_what_text(int x, int y, int n, byte *a, char *s);


/*
 * Log a message to a file
 */
extern void borg_info(char *what);

/*
 * Log a message, Search it, and Show/Memorize it in pieces
 */
extern void borg_note(char *what);


/*
 * Abort the Borg, noting the reason
 */
extern void borg_oops(char *what);


/*
 * Take a "memory note"
 */
extern bool borg_tell(char *what);

/*
 * Change the player name
 */
extern bool borg_change_name(char *str);

/*
 * Dump a character description
 */
extern bool borg_dump_character(char *str);

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
extern int borg_calc_formula(int *);
/*
 * check out a formula in RPN
 */
extern int borg_check_formula(int *);
/*
 * return a string for the formula
 */
extern char *borg_prt_formula(int *formula);

/*
 * Print the string for an item
 */
extern char *borg_prt_item(int item);

/*
 * Initialize this file
 */
extern void borg_init_1(void);

#endif

#endif

