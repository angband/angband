/* File: types.h */

/* Purpose: global type declarations */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


/*
 * This file should ONLY be included by "angband.h"
 */

/*
 * Note that "char" may or may not be signed, and that "signed char"
 * may or may not work on all machines.  So always use "s16b" or "s32b"
 * for signed values.  Also, note that unsigned values cause math problems
 * in many cases, so try to only use "u16b" and "u32b" for "bit flags"
 */
 



/*
 * Note that fixed length character fields allow pure "structure copy",
 * (which is technically bad anyway), but take up a LOT of space.  We
 * should really track down all the structure copies and remove them.
 * The only one left is "inscrip", and it would be nice to remove it.
 */



/*
 * Monster blow structure
 */
 
typedef struct _monster_blow monster_blow;

struct _monster_blow {
    byte method;
    byte effect;
    byte d_dice;
    byte d_side;
};



/*
 * An entry for the object kind allocator function
 */
typedef struct _kind_entry {
    u16b k_idx;		/* Object kind index */
    byte locale;	/* Base dungeon level */
    byte chance;	/* Rarity of occurance */
} kind_entry;


/*
 * An entry for the monster race allocator function
 */
typedef struct _race_entry {
    u16b r_idx;		/* Monster race index */
    byte locale;	/* Base dungeon level */
    byte chance;	/* Rarity of occurance */
} race_entry;


/*
 * Monster "variety" or "race"
 *
 * Note that "name" and "desc" are dynamically allocated.
 *
 * Note that "r_char" and "r_attr" are used for MORE than "visuals".
 */

typedef struct _monster_race monster_race;

struct _monster_race {

  cptr name;			/* Name				*/
  cptr desc;			/* Desc				*/

  byte r_attr;			/* Racial "color"		*/
  char r_char;			/* Racial "symbol"		*/

  byte level;			/* Level of creature		*/
  byte rarity;			/* Rarity of creature		*/

  byte hdice;			/* Creatures hit dice count	*/
  byte hside;			/* Creatures hit dice sides	*/

  s16b ac;			/* Armour Class			*/

  s16b sleep;			/* Inactive counter (base)	*/
  byte aaf;			/* Area affect radius (1-100)	*/
  byte speed;			/* Speed (normally 110)		*/

  s32b mexp;			/* Exp value for kill		*/

  s16b extra;			/* Something			*/
  
  byte freq_inate;		/* Inate spell frequency	*/
  byte freq_spell;		/* Other spell frequency	*/
  
  u32b rflags1;			/* Flags 1 (general)		*/
  u32b rflags2;			/* Flags 2 (abilities)		*/
  u32b rflags3;			/* Flags 3 (race/resist)	*/
  u32b rflags4;			/* Flags 4 (inate/breath)	*/
  u32b rflags5;			/* Flags 5 (normal spells)	*/
  u32b rflags6;			/* Flags 6 (special spells)	*/

  monster_blow blow[4];		/* Up to four blows per round	*/
};



/*
 * Monster memories (or "lore").
 *
 * Also, contains, for each monster race, the number of monsters
 * (of the given race) currently alive on the level, and the maximum
 * number of such monsters allowed at any one time on any level.
 * For uniques, we set "max_num" to "one", and then reset it to "zero"
 * once the unique has been killed.
 *
 * Note that there is really no reason for "monster_lore" not to
 * be part of "monster_race" except that it separates the things
 * that get written to the save-file from those that do not.
 *
 * Note that "max_num" is not retained across "lives".
 * Note that "cur_num" is not retained across "levels".
 *
 * Note that several of these fields, related to "recall", can be
 * scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc).
 */

typedef struct _monster_lore monster_lore;

struct _monster_lore {

    s16b sights;		/* Count sightings of this monster */
    s16b deaths;		/* Count deaths from this monster */

    s16b pkills;		/* Count monsters killed in this life */
    s16b tkills;		/* Count monsters killed in all lives */

    byte wake;			/* Number of times woken up (?) */
    byte ignore;		/* Number of times ignored (?) */

    byte xtra1;			/* Something */
    byte xtra2;			/* Something */

    byte drop_gold;		/* Max number of gold dropped at once */
    byte drop_item;		/* Max number of item dropped at once */

    byte cast_inate;		/* Max number of inate spells seen */
    byte cast_spell;		/* Max number of other spells seen */

    byte blows[4];		/* Number of times each blow type was seen */

    u32b flags1;		/* Observed racial flags */
    u32b flags2;		/* Observed racial flags */
    u32b flags3;		/* Observed racial flags */
    u32b flags4;		/* Observed racial flags */
    u32b flags5;		/* Observed racial flags */
    u32b flags6;		/* Observed racial flags */
    

    byte max_num;		/* Maximum population allowed per level */

    byte cur_num;		/* Monster population on current level */

    byte l_attr;		/* Desired monster attribute */
    char l_char;		/* Desired monster character */
};




/*
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 */

typedef struct _monster_type monster_type;

struct _monster_type {

  s16b r_idx;			/* Monster race index		*/

  byte fy;			/* Y location on map		*/
  byte fx;			/* X location on map		*/

  s16b hp;			/* Current Hit points		*/
  s16b maxhp;			/* Max Hit points		*/

  s16b csleep;			/* Inactive counter		*/

  byte mspeed;			/* Monster "speed"		*/
  byte energy;			/* Monster "energy"		*/

  byte stunned;			/* Monster is stunned		*/
  byte confused;		/* Monster is confused		*/
  byte monfear;			/* Monster is afraid		*/
  byte xtra;			/* Monster is ???		*/

  byte cdis;			/* Current dis from player	*/

  bool ml;			/* Monster is "visible"		*/

#ifdef WDT_TRACK_OPTIONS

  byte ty;			/* Y location of target		*/
  byte tx;			/* X location of target		*/

  s16b t_who;			/* Who are we tracking		*/
  byte t_dur;			/* How long are we tracking	*/
  byte t_bit;			/* Four bit flags		*/

#endif

#ifdef DRS_SMART_OPTIONS

  u32b smart;			/* Field for "smart_learn"	*/

#endif

};



/*
 * Structure for the "quests"
 *
 * Hack -- currently, only the "level" parameter is set, with the
 * semantics that "one (QUEST) monster of that level" must be killed,
 * and then the "level" is reset to zero, meaning "all done".  Later,
 * we should allow quests like "kill 100 fire hounds", and note that
 * the "quest level" is then the level past which progress is forbidden
 * until the quest is complete.  Note that the "QUESTOR" flag then could
 * become a more general "never out of depth" flag for monsters.
 */

typedef struct _quest quest;

struct _quest {

    int level;		/* Dungeon level */
    int r_idx;		/* Monster race */

    int cur_num;	/* Number killed (unused) */
    int max_num;	/* Number required (unused) */
};



/*
 * Variable information about a "kind" of object
 *
 * The "x_list" array is modified in various places
 *
 * Only "aware" and "tried" are saved in the savefile
 */

typedef struct _inven_xtra inven_xtra;

struct _inven_xtra {

    bool aware;		/* The player is "aware" of the item's effects */

    bool tried;		/* The player has "tried" one of the items */

    bool has_flavor;	/* This object has a flavor */

    bool easy_know;	/* This object is always known (if aware) */

    byte k_attr;	/* The underlying attr for this object */
    char k_char;	/* The underlying char for this object */

    byte x_attr;	/* The desired attr for this object */
    char x_char;	/* The desired char for this object */

#ifdef FAST_OBJDES
    cptr single;	/* Hack -- singular form */
    cptr plural;	/* Hack -- plural form */
#endif

};



/*
 * Constant information about a "kind" of object
 *
 * The "k_list" array is initialized in "arrays.c" and never changed
 *
 * Note that the "name" field is dynamically allocated.
 */

typedef struct _inven_kind inven_kind;

struct _inven_kind {

  cptr name;			/* Name				*/

  byte tval;			/* Object type			*/
  byte sval;			/* Object sub type		*/
  s16b pval;			/* Object extra info		*/

  byte level;			/* Level			*/
  byte extra;			/* Something			*/

  byte k_attr;			/* Object "attribute"		*/
  char k_char;			/* Object "symbol"		*/

  s16b tohit;			/* Plusses to hit		*/
  s16b todam;			/* Plusses to damage		*/
  s16b toac;			/* Plusses to AC		*/
  s16b ac;			/* Normal AC			*/
  byte dd, ds;			/* Damage dice/sides		*/
  s16b weight;			/* Weight			*/

  s32b cost;			/* Object "base cost"		*/

  u32b flags1;			/* Flags, set 1			*/
  u32b flags2;			/* Flags, set 2			*/
  u32b flags3;			/* Flags, set 3			*/

  byte locale[4];		/* Allocation level(s)		*/
  byte chance[4];		/* Allocation chance(s)		*/
};


/*
 * Structure for an object.
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that a great deal of structure copying goes on with items, thus
 * it is *not* possible to simply use "string_make()" for inscriptions.
 */

typedef struct _inven_type inven_type;

struct _inven_type {

  s16b k_idx;			/* Kind index (zero if "dead")	*/

  byte iy;			/* Y-position on map, or zero	*/
  byte ix;			/* X-position on map, or zero	*/

  byte tval;			/* Item type (from kind)	*/
  byte sval;			/* Item sub-type (from kind)	*/
  s16b pval;			/* Item extra-parameter		*/

  byte discount;		/* Discount (if any)		*/
  byte number;			/* Number of items		*/
  s16b weight;			/* Weight			*/

  byte name1;			/* Artifact type, if any	*/
  byte name2;			/* Ego-Item type, if any	*/
  s16b timeout;			/* Activation Timeout Counter	*/

  s16b tohit;			/* Plusses to hit		*/
  s16b todam;			/* Plusses to damage		*/
  s16b toac;			/* Plusses to AC		*/
  s16b ac;			/* Normal AC			*/
  byte dd, ds;			/* Damage dice/sides		*/

  u16b ident;			/* General flags 		*/

  u32b flags1;			/* Wearable Flags, set 1	*/
  u32b flags2;			/* Wearable Flags, set 2	*/
  u32b flags3;			/* Wearable Flags, set 3	*/

  u16b note;			/* Inscription index		*/
};



/*
 * Special information about "artifacts" (permanent and variable).
 *
 * Note that the save-file only writes "cur_num" to the savefile.
 *
 * Note that most of these fields are initialized from a file.
 *
 * Note that "max_num" is always "1" (if that artifact "exists")
 */

typedef struct inven_very inven_very;

struct inven_very {

  cptr name;			/* Artifact Name		*/

  byte tval;			/* Artifact type		*/
  byte sval;			/* Artifact sub type		*/

  s16b pval;			/* Artifact extra info		*/

  s16b tohit;			/* Plusses to hit		*/
  s16b todam;			/* Plusses to damage		*/
  s16b toac;			/* Plusses to AC		*/

  s16b ac;			/* Normal AC			*/

  byte dd, ds;			/* Damage when hits		*/

  s16b weight;			/* Weight			*/

  s32b cost;			/* Artifact "cost"		*/

  u32b flags1;			/* Artifact Flags, set 1	*/
  u32b flags2;			/* Artifact Flags, set 2	*/
  u32b flags3;			/* Artifact Flags, set 3	*/

  byte level;			/* Artifact level		*/
  byte rarity;			/* Artifact rarity		*/

  byte cur_num;			/* Number created (0 or 1)	*/
  byte max_num;			/* Unused (should be "1")	*/
};




/*
 * A single "grid" in a Cave
 *
 * Note that several aspects of the code restrict the actual cave
 * to a max size of 256 by 256.  In partcular, locations are often
 * saved as bytes, limiting each coordinate to the 0-255 range.
 *
 * We use a full 16 bits for m_idx and i_idx, but only need about 10.
 *
 * Note the special fields for the simple "monster flow" code.
 */

typedef struct _cave_type cave_type;

struct _cave_type {

  s16b m_idx;		/* Monster index (in m_list) or zero */
  s16b i_idx;		/* Item index (in i_list) or zero */

#ifdef MONSTER_FLOW

  byte cost;		/* Hack -- cost of flowing */
  byte when;		/* Hack -- when cost was computed */

#endif

#ifdef WDT_TRACK_OPTIONS

  s16b track;		/* Hack -- footprint counter */

#endif

  u16b info;		/* Flags -- use the "GRID_*" constants */
};


/*
 * A store owner
 */

typedef struct _owner_type owner_type;

struct _owner_type {
  cptr owner_name;
  s16b max_cost;
  byte max_inflate;
  byte min_inflate;
  byte haggle_per;
  byte owner_race;
  byte insult_max;
  byte unused;
};




/*
 * A store, including an array of items for sale.
 */

typedef struct _store_type store_type;

struct _store_type {

  s32b store_open;

  s16b insult_cur;
  byte owner;
  byte store_ctr;

  s16b good_buy;
  s16b bad_buy;

  inven_type store_item[STORE_INVEN_MAX];
};





/*
 * The "name" of spell 'N' is stored as spell_names[X][N],
 * where X is 0 for mage-spells and 1 for priest-spells.
 */

typedef struct _magic_type magic_type;

struct _magic_type {

  byte slevel;		/* Required level */
  byte smana;		/* Required mana */
  byte sfail;		/* Minimum chance of failure */
  byte sexp;		/* Experience (divided by slevel) */
};


/*
 * Information about the player's "magic"
 *
 * Note that a player with a "spell_book" of "zero" is illiterate.
 */

typedef struct _player_magic player_magic;

struct _player_magic {

  s16b spell_book;		/* Tval of spell books (if any)	*/
  s16b spell_xtra;		/* Something for later		*/
  
  s16b spell_stat;		/* Stat for spells (if any) 	*/
  s16b spell_type;		/* Spell type (mage/priest)	*/
  
  s16b spell_first;		/* Level of first spell		*/
  s16b spell_weight;		/* Weight that hurts spells	*/

  magic_type info[64];		/* The available spells		*/
};



/*
 * Player racial info
 */
 
typedef struct _player_race player_race;

struct _player_race {

  cptr trace;			/* Type of race */

  s16b radj[6];			/* Racial stat bonuses */

  byte b_age;			    /* Base age of character	 */
  byte m_age;			    /* Racial age modifier 	 */

  byte m_b_ht;			    /* base height for males	 */
  byte m_m_ht;			    /* mod height for males	 */
  byte m_b_wt;			    /* base weight for males	 */
  byte m_m_wt;			    /* mod weight for males	 */
  byte f_b_ht;			    /* base height females	 */
  byte f_m_ht;			    /* mod height for females	 */
  byte f_b_wt;			    /* base weight for female	 */
  byte f_m_wt;			    /* mod weight for females	 */

  s16b rdis;			    /* disarming		*/
  s16b rdev;			    /* magic devices		*/
  s16b rsav;			    /* saving throw		*/
  s16b rstl;			    /* stealth			*/
  s16b rsrh;			    /* search ability		*/
  s16b rfos;			    /* search frequency		*/
  s16b rthn;			    /* base "to hit" (normal)	*/
  s16b rthb;			    /* base "to hit" (bows)	*/

  byte bhitdie;			/* Base hit points for race	 */
  byte infra;			/* See infra-red		 */
  byte b_exp;			/* Base experience factor	 */
  byte rtclass;			/* Bit field for class types */
};


/*
 * Player class info
 */

typedef struct _player_class player_class;

struct _player_class {

  cptr title;			/* Type of class		*/

  s16b c_adj[6];		/* Class stat modifier		*/

  s16b c_dis;			/* class disarming		*/
  s16b c_dev;			/* class magic devices		*/
  s16b c_sav;			/* class saving throws		*/
  s16b c_stl;			/* class stealth		*/
  s16b c_srh;			/* class searching ability	*/
  s16b c_fos;			/* class searching frequency	*/
  s16b c_thn;			/* class to hit (normal)	*/
  s16b c_thb;			/* class to hit (bows)		*/

  s16b x_dis;			/* extra disarming		*/
  s16b x_dev;			/* extra magic devices		*/
  s16b x_sav;			/* extra saving throws		*/
  s16b x_stl;			/* extra stealth		*/
  s16b x_srh;			/* extra searching ability	*/
  s16b x_fos;			/* extra searching frequency	*/
  s16b x_thn;			/* extra to hit (normal)	*/
  s16b x_thb;			/* extra to hit (bows)		*/

  s16b c_mhp;			/* Adjust hit points		*/
  s16b c_exp;			/* Class experience factor	*/
};




/*
 * Most of the "player" information goes here.
 *
 * Basically, this stucture gives us a large collection of global
 * variables, which can all be wiped to zero at creation time.
 *
 * Most of these variables are recalculated on a regular basis,
 * and only a few actually represent "state" information.  Very
 * few represent "permanent state information".
 */

typedef struct _player_type player_type;

struct _player_type {

  byte maximize;		/* Maximize stats	*/
  byte preserve;		/* Preserve artifacts	*/

  byte prace;			/* Race index		*/
  byte pclass;			/* Class index		*/
  byte male;			/* Sex of character	*/
  byte new_spells;		/* Spells available	*/
  byte hitdie;			/* Hit dice (sides)	*/
  byte expfact;			/* Experience factor	*/

  s16b age;			/* Characters age	*/
  s16b ht;			/* Height		*/
  s16b wt;			/* Weight		*/
  s16b sc;			/* Social Class		*/

  s16b use_stat[6];		/* Current "resulting" stat values	*/
  s16b mod_stat[6];		/* Current "modifiers" to stat values	*/

  s16b max_stat[6];		/* Current "maximal" stat values	*/
  s16b cur_stat[6];		/* Current "natural" stat values	*/

  s32b au;			/* Current Gold			*/

  s32b max_exp;			/* Max experience		*/
  s32b exp;			/* Cur experience		*/
  u16b exp_frac;		/* Cur exp frac (times 2^16)	*/

  s16b lev;			/* Level			*/

  s16b mhp;			/* Max hit pts			*/
  s16b chp;			/* Cur hit pts			*/
  u16b chp_frac;		/* Cur hit frac (times 2^16)	*/

  s16b msp;			/* Max mana pts			*/
  s16b csp;			/* Cur mana pts			*/
  u16b csp_frac;		/* Cur mana frac (times 2^16)	*/

  s16b max_plv;			/* Max Player Level		*/
  s16b max_dlv;			/* Max level explored		*/

  s16b pac;			/* Total AC		*/
  s16b ptoac;			/* Magical AC		*/
  s16b ptohit;			/* Plusses to hit	*/
  s16b ptodam;			/* Plusses to dam	*/

  s16b dis_th;			/* Displayed +ToHit	*/
  s16b dis_td;			/* Displayed +ToDam	*/
  s16b dis_ta;			/* Displayed +ToTAC	*/
  s16b dis_ac;			/* Displayed AC		*/

  u32b notice;			/* Noticed Things	*/

  u32b update;			/* Pending Updates	*/
  u32b redraw;			/* Desired Redraws	*/

  s16b pspeed;			/* Current speed	*/
  s16b energy;			/* Current energy	*/

  s16b food;			/* Current nutrition	*/
  s16b food_digested;		/* Food per round	*/

  byte confusing;		/* Glowing hands	*/
  byte searching;		/* Currently searching	*/

  s16b rest;			/* Rest counter			*/

  s16b word_recall;		/* Word of recall counter	*/

  s16b fast;			/* Timed -- Fast		*/
  s16b slow;			/* Timed -- Slow		*/
  s16b blind;			/* Timed -- Blindness		*/
  s16b paralysis;		/* Timed -- Paralysis		*/
  s16b confused;		/* Timed -- Confusion		*/
  s16b fear;			/* Timed -- Fear		*/
  s16b image;			/* Timed -- Hallucination	*/
  s16b poisoned;		/* Timed -- Poisoned		*/
  s16b cut;			/* Timed -- Cut			*/
  s16b stun;			/* Timed -- Stun		*/

  s16b protevil;		/* Timed -- Protection		*/
  s16b invuln;			/* Timed -- Invulnerable	*/
  s16b hero;			/* Timed -- Heroism		*/
  s16b shero;			/* Timed -- Super Heroism	*/
  s16b shield;			/* Timed -- Shield Spell	*/
  s16b blessed;			/* Timed -- Blessed		*/
  s16b tim_invis;		/* Timed -- See Invisible	*/
  s16b tim_infra;		/* Timed -- Infra Vision	*/

  s16b oppose_acid;		/* Timed -- oppose acid		*/
  s16b oppose_elec;		/* Timed -- oppose lightning	*/
  s16b oppose_fire;		/* Timed -- oppose heat		*/
  s16b oppose_cold;		/* Timed -- oppose cold		*/
  s16b oppose_pois;		/* Timed -- oppose poison	*/

  byte immune_acid;		/* Immunity to acid		*/
  byte immune_elec;		/* Immunity to lightning	*/
  byte immune_fire;		/* Immunity to fire		*/
  byte immune_cold;		/* Immunity to cold		*/
  byte immune_pois;		/* Immunity to poison		*/

  byte resist_acid;		/* Resist acid		*/
  byte resist_elec;		/* Resist lightning	*/
  byte resist_fire;		/* Resist fire		*/
  byte resist_cold;		/* Resist cold		*/
  byte resist_pois;		/* Resist poison	*/

  byte resist_conf;		/* Resist confusion	*/
  byte resist_sound;		/* Resist sound		*/
  byte resist_lite;		/* Resist light		*/
  byte resist_dark;		/* Resist darkness	*/
  byte resist_chaos;		/* Resist chaos		*/
  byte resist_disen;		/* Resist disenchant	*/
  byte resist_shard;		/* Resist shards	*/
  byte resist_nexus;		/* Resist nexus		*/
  byte resist_blind;		/* Resist blindness	*/
  byte resist_neth;		/* Resist nether	*/
  byte resist_fear;		/* Resist fear		*/

  byte sustain_str;		/* Keep strength	*/
  byte sustain_int;		/* Keep intelligence	*/
  byte sustain_wis;		/* Keep wisdom		*/
  byte sustain_dex;		/* Keep dexterity	*/
  byte sustain_con;		/* Keep constitution	*/
  byte sustain_chr;		/* Keep charisma	*/

  byte aggravate;		/* Aggravate monsters	*/
  byte teleport;		/* Random teleporting	*/

  byte ffall;			/* No damage falling	*/
  byte lite;			/* Permanent light	*/
  byte free_act;		/* Never paralyzed	*/
  byte see_inv;			/* Can see invisible	*/
  byte regenerate;		/* Regenerate hit pts	*/
  byte hold_life;		/* Resist life draining	*/
  byte telepathy;		/* Telepathy		*/
  byte slow_digest;		/* Slower digestion	*/
    
  s16b see_infra;		/* Infravision range	*/

  s16b skill_dis;		/* Skill: Disarming		*/
  s16b skill_dev;		/* Skill: Magic Devices		*/
  s16b skill_sav;		/* Skill: Saving throw		*/
  s16b skill_stl;		/* Skill: Stealth factor	*/
  s16b skill_srh;		/* Skill: Searching ability	*/
  s16b skill_fos;		/* Skill: Searching frequency	*/
  s16b skill_thn;		/* Skill: To hit (normal)	*/
  s16b skill_thb;		/* Skill: To hit (bows)		*/

  s16b num_blow;		/* Number of blows	*/
  s16b num_fire;		/* Number of shots	*/

  byte tval_xtra;		/* Correct xtra tval	*/

  byte tval_ammo;		/* Correct ammo tval	*/
};


