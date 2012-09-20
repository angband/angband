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

  byte xtra1;			/* Something 			*/
  byte xtra2;			/* Something 			*/
  
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
  byte dead;			/* Monster is dead		*/

  byte cdis;			/* Current dis from player	*/

  bool ml;			/* Monster is "visible"		*/

  byte ty;			/* Y location of target		*/
  byte tx;			/* X location of target		*/

#ifdef WDT_TRACK_OPTIONS

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

  byte level;			/* Object level			*/
  byte number;			/* Always "one", for now	*/
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
 * Note: ix and iy are currently just "suggestions" for location,
 * which are verified before used (see delete_object()).
 * More properly, they would be saved and loaded from save files,
 * and set to zero when picked up, and set properly when dropped.
 * Even so, they already speed tpush() up a significant amount.
 *
 * The "cost" field represents the item's "base value"
 * The "scost" field represents how much a store owner would pay.
 * This may or may not interact badly with the black market...
 *
 * Also, "scost" is set negative until the store owner has "fixed"
 * his price at a certain value.  Currently, the "scost" field is
 * cleared when an object leaves the store, but it could be used to
 * track various conditions.
 *
 * Note that the "cost" of an item is changed when the item is sold
 * at a discounted rate, and that this change is permanent.  Also,
 * since objects with different costs never merge, the user can never
 * "cheat" the store.  For the same reason, the player can never "lose"
 * value on an object by buying a similar, discounted object.
 *
 * Making inscrip a pointer and mallocing space does not work, there are
 * too many places where inven_types are copied, which results in dangling
 * pointers, so we use a char array for them instead.  We could always
 * attempt to remove this dependency on copying structures... :-)
 */

#define INSCRIP_SIZE 12	 /* notice alignment, must be 4*x */

typedef struct _inven_type inven_type;

struct _inven_type {

  s16b k_idx;			/* Kind index (in k_list)	*/

  byte iy;			/* Y-position on map, or 0	*/
  byte ix;			/* X-position on map, or 0	*/

  byte tval;			/* Category number		*/
  byte sval;			/* Sub-category number		*/
  s16b pval;			/* Misc. use variable		*/

  s16b timeout;			/* Timeout counter		*/
  byte name1;			/* Artifact type, if any	*/
  byte name2;			/* Special type, if any		*/
  byte ident;			/* Identification info		*/
  byte number;			/* Number of items		*/
  s16b weight;			/* Weight			*/

  s16b tohit;			/* Plusses to hit		*/
  s16b todam;			/* Plusses to damage		*/
  s16b toac;			/* Plusses to AC		*/
  s16b ac;			/* Normal AC			*/
  byte dd, ds;			/* Damage dice/sides		*/

  byte unused;			/* Unused field			*/
  byte discount;		/* Discount (if any)		*/

  s32b cost;			/* Cost of item			*/
  s32b scost;			/* Store cost			*/

  u32b flags1;			/* Flags, set 1			*/
  u32b flags2;			/* Flags, set 2			*/
  u32b flags3;			/* Flags, set 3			*/

  char inscrip[INSCRIP_SIZE];	/* Object inscription		*/
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

  s16b k_idx;			/* Artifact base kind		*/

  byte tval;			/* Artifact type		*/
  byte sval;			/* Artifact sub type		*/
  s16b pval;			/* Artifact extra info		*/

  byte level;			/* Artifact level		*/
  byte rarity;			/* Artifact rarity		*/

  s16b tohit;			/* Plusses to hit		*/
  s16b todam;			/* Plusses to damage		*/
  s16b toac;			/* Plusses to AC		*/
  s16b ac;			/* Normal AC			*/
  byte dd, ds;			/* Damage when hits		*/
  s16b weight;			/* Weight			*/

  s32b cost;			/* Artifact "cost"		*/

  u32b flags1;		/* Artifact Flags, set 1	*/
  u32b flags2;		/* Artifact Flags, set 2	*/
  u32b flags3;		/* Artifact Flags, set 3	*/

  u16b padding;			/* Padding			*/

  byte cur_num;		/* Number created (0 or 1)	*/
  byte max_num;		/* Unused (should be "1")	*/
};



/*
 * The "name" of spell 'N' is stored as spell_names[X][N],
 * where X is 0 for mage-spells and 1 for priest-spells.
 *
 * XXX We should probably change the whole "sexp" method, so that
 * the "spell experience" is taken from slevel, smana, and sfail.
 */

typedef struct _spell_type spell_type;

struct _spell_type {
  byte slevel;		/* Required level */
  byte smana;		/* Required mana */
  byte sfail;		/* Minimum chance of failure */
  byte sxtra;		/* Padding, for now */
  u16b sexp;		/* 1/4 of exp gained for learning spell */
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

  s16b b_dis;			    /* base chance to disarm	 */
  s16b srh;			    /* base chance for search	 */
  s16b stl;			    /* Stealth of character	 */
  s16b fos;			    /* frequency of auto search	 */
  s16b bth;			    /* adj base chance to hit	 */
  s16b bthb;			    /* adj base to hit with bows */
  s16b bsav;			    /* Race base for saving throw*/

  byte bhitdie;			/* Base hit points for race	 */
  byte infra;			/* See infra-red		 */
  byte b_exp;			/* Base experience factor	 */
  byte rtclass;			/* Bit field for class types */
};


/*
 * Player class info
 *
 * Hack -- note that we use A_STR/A_INT/A_WIS as the actual values
 * of "spell_stat" which lets us use the fact that A_STR is zero...
 * Thus if "p_ptr->spell_stat" is "TRUE" then the player has spells.
 */

typedef struct _player_class player_class;

struct _player_class {

  cptr title;			/* Type of class		*/

  s16b cadj[6];			/* Class stat modifier		*/

  byte adj_hd;			/* Adjust hit points		*/
  byte m_exp;			/* Class experience factor	*/

  byte mdis;			/* mod disarming traps		*/
  byte msrh;			/* modifier to searching	*/
  byte mfos;			/* modifier to freq-of-search	*/
  byte mstl;			/* modifier to stealth		*/
  byte mbth;			/* modifier to base to hit	*/
  byte mbthb;			/* modifier to base to hit with bows	*/
  byte msav;			/* modifier to saving throws	*/

  byte spell_stat;		/* Stat for spells (if any) 	*/
  byte spell_first;		/* First level spells usable	*/
};


/*
 * Player background information
 */

typedef struct _player_background player_background;

struct _player_background {

  cptr info;			    /* History information	    */

  byte roll;			    /* Die roll needed for history  */
  byte chart;			    /* Table number		    */
  byte next;			    /* Pointer to next table	    */
  byte bonus;			    /* Bonus to the Social Class+50 */
};




/*
 * Most of the "player" information goes here.
 *
 * Basically, this stucture gives us a large collection of global
 * variables, which can all be wiped to zero at creation time.
 *
 * Note that "speed" is now calculated in "calc_bonuses()".
 * Note that "normal" speed is "110", and "fast" is "120".
 */

typedef struct _player_type player_type;

struct _player_type {

  byte maximize;		/* Maximal stats affected by race/class */
  byte preserve;		/* Preserve artifacts, reduce "feelings" */

  byte prace;			/* # of race	*/
  byte pclass;			/* # of class	*/
  byte male;			/* Sex of character */
  byte new_spells;		/* Number of spells can learn. */
  byte hitdie;			/* Char hit die	*/
  byte expfact;			/* Experience factor	*/

  s16b age;			/* Characters age	*/
  s16b ht;			/* Height		*/
  s16b wt;			/* Weight		*/

  s16b use_stat[6];		/* Current "resulting" stat values */
  s16b max_stat[6];		/* Current "maximal" stat values */
  s16b cur_stat[6];		/* Current "natural" stat values */
  s16b mod_stat[6];		/* Current "stat modifiers" */

  s32b au;			/* Gold		*/

  s32b max_exp;			/* Max experience	*/
  s32b exp;			/* Cur experience	*/
  u16b exp_frac;		/* Cur exp fraction * 2^16	*/

  s16b lev;			/* Level		*/

  s16b mana;			/* Mana points	*/
  s16b cmana;			/* Cur mana pts		*/
  u16b cmana_frac;		/* Cur mana fraction * 2^16 */

  s16b mhp;			/* Max hit pts	*/
  s16b chp;			/* Cur hit pts		*/
  u16b chp_frac;		/* Cur hit fraction * 2^16	*/

  s16b max_plv;			/* Max Player Level */
  s16b max_dlv;			/* Max level explored	*/

  s16b sc;			/* Social Class	*/
  s16b stl;			/* Stealth factor	*/
  s16b srh;			/* Chance in search */
  s16b fos;			/* Frenq of search	*/
  s16b disarm;			/* % to Disarm	*/
  s16b save;			/* Saving throw	*/

  s16b bth;			/* Base to hit	*/
  s16b bthb;			/* BTH with bows	*/

  s16b pac;			/* Total AC		*/
  s16b ptoac;			/* Magical AC	*/
  s16b ptohit;			/* Plusses to hit	*/
  s16b ptodam;			/* Plusses to dam	*/

  s16b dis_th;			/* Display +ToHit	*/
  s16b dis_td;			/* Display +ToDam	*/
  s16b dis_ac;			/* Display +ToAC	*/
  s16b dis_tac;			/* Display +ToTAC	*/

  u32b update;			/* Pending Updates	*/
  u32b notice;			/* Noticed Things	*/
  u32b redraw;			/* Desired Redraws	*/

  s16b pspeed;			/* Current speed	*/
  s16b energy;			/* Current energy	*/

  s16b food;			/* Current nutrition	*/
  s16b food_digested;		/* Food per round	*/

  s16b rest;			/* Rest counter		*/
  s16b blind;			/* Blindness counter	*/
  s16b paralysis;		/* Paralysis counter	*/
  s16b confused;		/* Confusion counter	*/
  s16b protection;		/* Protection fr. evil	*/
  s16b fast;			/* Temp speed change	*/
  s16b slow;			/* Temp speed change	*/
  s16b afraid;			/* Fear			*/
  s16b cut;			/* Wounds		*/
  s16b stun;			/* Stunned player	*/
  s16b poisoned;		/* Poisoned		*/
  s16b image;			/* Hallucinate		*/

  s16b protevil;		/* Protect VS evil	   */
  s16b invuln;			/* Increases AC	   */
  s16b hero;			/* Heroism		   */
  s16b shero;			/* Super Heroism	   */
  s16b shield;			/* Shield Spell	   */
  s16b blessed;		/* Blessed		   */
  s16b detect_inv;		/* Timed see invisible */
  s16b word_recall;		/* Timed teleport level*/
  s16b see_infra;		/* See warm creatures  */
  s16b tim_infra;		/* Timed infra vision  */

  s16b oppose_acid;		/* Timed acid resist   */
  s16b oppose_elec;		/* Timed lightning resist  */
  s16b oppose_fire;		/* Timed heat resist   */
  s16b oppose_cold;		/* Timed cold resist   */
  s16b oppose_pois;		/* Timed poison resist */

  byte immune_acid;		/* Immune to acid	   */
  byte immune_elec;		/* Immune to lightning     */
  byte immune_fire;		/* Immune to fire	   */
  byte immune_cold;		/* Immune to cold	   */
  byte immune_pois;		/* Immune to poison	   */

  byte resist_acid;		/* Resistance to acid  */
  byte resist_elec;		/* Resistance to lightning */
  byte resist_fire;		/* Resistance to fire  */
  byte resist_cold;		/* Resistance to cold  */
  byte resist_pois;		/* Resistance to poison	   */

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
  byte hold_life;		/* Immune to drain-life	*/
  byte telepathy;		/* Has telepathy	*/
  byte slow_digest;		/* Lower food needs	*/
  byte confusing;		/* Glowing hands.	*/
  byte searching;		/* Currently searching	*/
};


