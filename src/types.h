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



typedef byte	int8u;

typedef s16b	int16;
typedef u16b	int16u;

typedef s32b	int32;
typedef u32b	int32u;


/*
 * some machines will not accept 'signed char' as a type, and some accept it
 * but still treat it like an unsigned character, let's just avoid it,
 * any variable which can ever hold a negative value must be 16 or 32 bits 
 */


/*
 * Some simple buffers (overkill)
 */

#define VTYPESIZ    160
#define BIGVTYPESIZ 300

typedef char vtype[VTYPESIZ];
typedef char bigvtype[BIGVTYPESIZ];



/*
 * Note that fixed length character fields allow pure "structure copy",
 * (which is technically bad anyway), but take up a LOT of space.  The
 * only one left is "inscrip", and it would be nice to remove it.
 *
 * all fields are given the smallest possible type, and all fields are
 * aligned within the structure to their natural size boundary, so that
 * the structures contain no padding and are minimum size.  In theory...
 *
 * Note that bit fields are extremely inefficient, and are thus used
 * only for "cave_type", where they provide a major savings in space.
 * Actually, this is not really true, perhaps we should eliminate them.
 */


/*
 * Monster attack and damage types
 * Field order fixed by use in tables.c
 */
typedef struct _monster_attack monster_attack;

struct _monster_attack {
    int8u attack_type;		    
    int8u attack_desc;
    int8u attack_dice;
    int8u attack_sides;
};


/*
 * Monster "variety" or "race"
 *
 * A major advantage of loading the "m_list" and "k_list" arrays from
 * a text file is that the structures below are no longer hard-coded.
 *
 * However, see the "speed-load" code in "arrays.c", and be sure to
 * wipe the "data" directory if the structures are changed.
 *
 * Note that "name" and "desc" are dynamically allocated, and must
 * be at the end for the "speed load" code.
 *
 * Note that the "gender" info is now encoded into "cflags1", as is
 * information on "multihued-ness" and "transparency".
 *
 * Note that "r_char" and "r_attr" are used for MORE than "visuals".
 *
 * Should attempt to "move" the monster attack info into the race.
 * This would allow the attacks to be much more "varied"...
 */

typedef struct _monster_race monster_race;

struct _monster_race {

  int8u level;			/* Level of creature		*/
  int8u rarity;			/* Rarity of creature		*/
  byte r_attr;			/* Racial "attribute"		*/
  char r_char;			/* Racial "symbol"		*/

  int8u hd[2];			/* Creatures hit die		*/
  int16u ac;			/* Armour Class			*/

  int16u sleep;			/* Inactive counter/10		*/
  int8u aaf;			/* Area affect radius		*/
  int8u speed;			/* Movement speed+10		*/

  int32u mexp;			/* Exp value for kill		*/

  int16u damage[4];		/* Type attack and damage	*/

  int32u cflags1;		/* Flags 1 (movement)		*/
  int32u cflags2;		/* Flags 2 (defense)		*/
  
  int32u spells1;		/* Spell flags 1		*/
  int32u spells2;		/* Spell flags 2		*/
  int32u spells3;		/* Spell flags 3		*/

  cptr name;			/* Name (must be at end)	*/
  cptr desc;			/* Desc (must be at end)	*/
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
 */

typedef struct _monster_lore monster_lore;

struct _monster_lore {

    int32u r_cflags1;
    int32u r_cflags2;
    int32u r_spells1;
    int32u r_spells2;
    int32u r_spells3;

    int16u r_kills;		/* Count player killing monster */
    int16u r_deaths;		/* Count monster killing player */

    int8u r_attacks[4];

    int8u r_wake;		/* Number of times woken up (?) */
    int8u r_ignore;		/* Number of times ignored (?) */

    int8u r_cast;		/* Total number of spells cast */
    int8u r_drop;		/* Max number of things dropped at once */
    
    int8u max_num;		/* Maximum population allowed per level */

    int8u cur_num;		/* Monster population on this level */
    
    char l_char;		/* Graphic character for display */
    char l_attr;		/* Graphic symbol for display */
};




/*
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 */

typedef struct _monster_type monster_type;

struct _monster_type {

  int16u r_idx;			/* Monster race index		*/

  int8u fy;			/* Y location on map		*/
  int8u fx;			/* X location on map		*/

  int16 hp;			/* Current Hit points		*/
  int16 maxhp;			/* Max Hit points		*/

  int16 csleep;			/* Inactive counter		*/
  int16 cspeed;			/* Movement speed		*/

  int8u stunned;		/* Monster is stunned		*/
  int8u confused;		/* Monster is confused		*/
  int8u monfear;		/* Monster is afraid		*/
  int8u unused;			/* Unused, but saved		*/

  int16u cdis;			/* Current dis from player	*/

  bool los;			/* Player to Monster l.o.s.	*/
  bool ml;			/* Monster is "visible"		*/
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
 * Extra Item "memory" (not very important)
 */

typedef struct _inven_xtra inven_xtra;

struct _inven_xtra {

    bool aware;		/* The player is "aware" of the item's effects */
    bool tried;		/* The player has "tried" one of the items */
    
    char x_char;	/* Graphic symbol for display */
    byte x_attr;	/* Graphic attribute for display */
};



/*
 * The "kind" of an object (or treasure).
 *
 * Note that "name" is dynamically allocated.
 * Note that "name" must be at the end.
 */

typedef struct _inven_kind inven_kind;

struct _inven_kind {

  int8u tval;			/* Object type			*/
  int8u sval;			/* Object sub type		*/
  int16 pval;			/* Object extra info		*/

  int8u level;			/* Object level			*/
  int8u number;			/* Always "one", for now	*/
  byte i_attr;			/* Object "attribute"		*/
  char i_char;			/* Object "symbol"		*/
  
  int16 tohit;			/* Plusses to hit		*/
  int16 todam;			/* Plusses to damage		*/
  int16 toac;			/* Plusses to AC		*/
  int16 ac;			/* Normal AC			*/
  int8u damage[2];		/* Damage when hits		*/
  int16 weight;			/* Weight			*/

  int32 cost;			/* Object "base cost"		*/

  int32u flags1;		/* Flags, set 1			*/
  int32u flags2;		/* Flags, set 2			*/
  int32u flags3;		/* Flags, set 3			*/

  int8u locale[4];		/* Allocation level(s)		*/
  int8u rarity[4];		/* Allocation chance(s)		*/

  cptr name;			/* Name (must be at end)	*/
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

  int16u k_idx;			/* Kind index (in k_list)	*/

  int8u iy;			/* Y-position on map, or 0	*/
  int8u ix;			/* X-position on map, or 0	*/

  int8u tval;			/* Category number		*/
  int8u sval;			/* Sub-category number		*/
  int16 pval;			/* Misc. use variable		*/

  int16 timeout;		/* Timeout counter		*/
  int8u name1;			/* Artifact type, if any	*/
  int8u name2;			/* Special type, if any		*/
  int8u ident;			/* Identification info		*/
  int8u number;			/* Number of items		*/
  int16 weight;			/* Weight			*/

  int16 tohit;			/* Plusses to hit		*/
  int16 todam;			/* Plusses to damage		*/
  int16 toac;			/* Plusses to AC		*/
  int16 ac;			/* Normal AC			*/
  int8u damage[2];		/* Damage when hits		*/
  int16u unused;		/* Padding */

  int32 cost;			/* Cost of item			*/
  int32 scost;			/* Store cost			*/

  int32u flags1;		/* Flags, set 1			*/
  int32u flags2;		/* Flags, set 2			*/
  int32u flags3;		/* Flags, set 3			*/

  char inscrip[INSCRIP_SIZE];	/* Object inscription		*/
};



/*
 * Special information about "artifacts" (permanent and variable).
 *
 * Note that the save-file only writes "cur_num" to the savefile.
 *
 * Note that "name" and "tval" and "sval" are initialized globally.
 *
 * Note that "max_num" is always "1" except for "GROND" and "MORGOTH".
 * Note that "max_num" is currently "unused".  For now, I am still
 * using the old "permit_grond" and "permit_morgoth" flags.
 */
 
typedef struct inven_very inven_very;

struct inven_very {

  cptr name;			/* Artifact "name"			*/
  
  int8u tval;			/* Category number			*/
  int8u sval;			/* Sub-category number			*/

  int8u cur_num;		/* Number created (0 or 1)		*/
  int8u max_num;		/* Number allowed (0 or 1)		*/
};



/*
 * spell name is stored in spell_names[] array
 * at index i if "mage", Or at index i + ??? if "priest"
 *
 * We should probably change the whole "sexp" method, so that
 * the "spell experience" is taken from slevel, smana, and sfail.
 */

typedef struct _spell_type spell_type;

struct _spell_type {
  int8u slevel;		/* Required level */
  int8u smana;		/* Required mana */
  int8u sfail;		/* Minimum chance of failure */
  int8u sxtra;		/* Padding, for now */
  int16u sexp;		/* 1/4 of exp gained for learning spell */
};



/*
 * A single "grid" in a Cave
 *
 * Note that several aspects of the code restrict the actual cave
 * to a max size of 256 by 256.  In partcular, locations are often
 * saved as bytes, limiting each coordinate to the 0-255 range.
 *
 * Note that the monster/item indexes must be larger than bytes.
 *
 * Note that the "info" field replaces the old (inefficient) bit fields:
 *   lr: room should be lit with perm light, walls with
 *        this set should be perm lit after tunneled out
 *   fm: field mark, used for traps/doors/stairs, object is
 *        hidden if fm is FALSE
 *   pl: permanent light, used for walls and lighted rooms
 *   tl: temporary light, used for player's lamp light,etc.
 *
 * To access the old bit fields, do bit operations on "info" using the
 * constants "CAVE_LR", "CAVE_FM", "CAVE_PL", and "CAVE_TL".
 *
 * The new fields in "info" are: "CAVE_INIT" to allow lit rooms to be noticed
 * when first entered, "CAVE_SEEN" for future use, "CAVE_VIEW" to mark a grid
 * as "viewable", and "CAVE_XTRA" for the "update_view()" routine.
 *
 * Note the preparation for a "mental state" of the map, so that, for
 * example, doors and walls do not suddenly "disappear" halfway accross
 * the dungeon.  These are currently usused.
 */

typedef struct _cave_type cave_type;

struct _cave_type {

  int16 m_idx;		/* Monster index (in m_list) */
  int16 i_idx;		/* Item index (in i_list) */

  byte fval;		/* Grid type (0-15) */

  byte info;		/* Holds the "CAVE_*" flags */

#ifdef NEW_MAPS

  char mchar;		/* Hack -- "map memory" */
  byte mattr;		/* Hack -- "map memory" */
  
#endif

};


/*
 * A store owner
 */

typedef struct _owner_type owner_type;

struct _owner_type {
  cptr owner_name;
  int16 max_cost;
  int8u max_inflate;
  int8u min_inflate;
  int8u haggle_per;
  int8u owner_race;
  int8u insult_max;
  int8u unused;
};




/*
 * A store.  Now holds some items, which themselves hold their store cost.
 */

typedef struct _store_type store_type;

struct _store_type {

  int32 store_open;

  int16 insult_cur;
  int8u owner;
  int8u store_ctr;

  int16u good_buy;
  int16u bad_buy;

  inven_type store_item[STORE_INVEN_MAX];
};



typedef struct _player_race player_race;

struct _player_race {

  cptr trace;			    /* Type of race	    */

  int16 str_adj;
  int16 int_adj;		    
  int16 wis_adj;
  int16 dex_adj;
  int16 con_adj;
  int16 chr_adj;

  int8u b_age;			    /* Base age of character	 */
  int8u m_age;			    /* Maximum age of character	 */
  int8u m_b_ht;			    /* base height for males	 */
  int8u m_m_ht;			    /* mod height for males	 */
  int8u m_b_wt;			    /* base weight for males	 */
  int8u m_m_wt;			    /* mod weight for males	 */
  int8u f_b_ht;			    /* base height females	 */
  int8u f_m_ht;			    /* mod height for females	 */
  int8u f_b_wt;			    /* base weight for female	 */
  int8u f_m_wt;			    /* mod weight for females	 */

  int16 b_dis;			    /* base chance to disarm	 */
  int16 srh;			    /* base chance for search	 */
  int16 stl;			    /* Stealth of character	 */
  int16 fos;			    /* frequency of auto search	 */
  int16 bth;			    /* adj base chance to hit	 */
  int16 bthb;			    /* adj base to hit with bows */
  int16 bsav;			    /* Race base for saving throw*/
  int8u bhitdie;		    /* Base hit points for race	 */
  int8u infra;			    /* See infra-red		 */
  int8u b_exp;			    /* Base experience factor	 */
  int8u rtclass;		    /* Bit field for class types */
};

typedef struct _player_class player_class;

struct _player_class {

  cptr title;			    /* type of class		       */

  int8u adj_hd;			    /* Adjust hit points	       */
  int8u mdis;			    /* mod disarming traps	       */
  int8u msrh;			    /* modifier to searching	       */
  int8u mstl;			    /* modifier to stealth	       */
  int8u mfos;			    /* modifier to freq-of-search      */
  int8u mbth;			    /* modifier to base to hit	       */
  int8u mbthb;			    /* modifier to base to hit - bows  */
  int8u msav;			    /* Class modifier to save	       */

  int16 madj_str;		    /* Class modifier for strength     */
  int16 madj_int;		    /* Class modifier for intelligence */
  int16 madj_wis;		    /* Class modifier for wisdom       */
  int16 madj_dex;		    /* Class modifier for dexterity    */
  int16 madj_con;		    /* Class modifier for constitution */
  int16 madj_chr;		    /* Class modifier for charisma     */

  int8u spell;			    /* class use mage spells	       */
  int8u m_exp;			    /* Class experience factor	       */
  int8u first_spell_lev;	    /* First level spells usable */
  int8u age_adj;		    /* age percentage (warrior = 100) */
};


/*
 * Player background information
 */

typedef struct _player_background player_background;

struct _player_background {

  cptr info;			    /* History information	    */

  int8u roll;			    /* Die roll needed for history  */
  int8u chart;			    /* Table number		    */
  int8u next;			    /* Pointer to next table	    */
  int8u bonus;			    /* Bonus to the Social Class+50 */
};




/*
 * Most of the "player" information goes here.
 *
 * Basically, this stucture gives us a large collection of global
 * variables, which can all be wiped to zero at creation time.
 */

typedef struct _player_type player_type;

struct _player_type {

  int8u prace;			/* # of race	*/
  int8u pclass;			/* # of class	*/
  int8u male;			/* Sex of character */
  int8u new_spells;		/* Number of spells can learn. */
  int8u hitdie;			/* Char hit die	*/
  int8u expfact;		/* Experience factor	*/

  int16u age;			/* Characters age	*/
  int16u ht;			/* Height		*/
  int16u wt;			/* Weight		*/

  int16 use_stat[6];	    /* Current "resulting" stat values */
  int16 max_stat[6];	    /* Current "maximal" stat values */
  int16 cur_stat[6];	    /* Current "natural" stat values */
  int16 mod_stat[6];	    /* Current "stat modifiers" */

  int32 au;			/* Gold		*/

  int32 max_exp;		/* Max experience	*/
  int32 exp;			/* Cur experience	*/
  int16u exp_frac;		/* Cur exp fraction * 2^16	*/
  int16u lev;			/* Level		*/

  int16 mana;			/* Mana points	*/
  int16 cmana;			/* Cur mana pts		*/
  int16u cmana_frac;		/* Cur mana fraction * 2^16 */

  int16 mhp;			/* Max hit pts	*/
  int16 chp;			/* Cur hit pts		*/
  int16u chp_frac;		/* Cur hit fraction * 2^16	*/

  int16u max_plv;		/* Max Player Level */
  int16u max_dlv;		/* Max level explored	*/

  int16 srh;			/* Chance in search */
  int16 fos;			/* Frenq of search	*/
  int16 disarm;			/* % to Disarm	*/
  int16 save;			/* Saving throw	*/
  int16 sc;			/* Social Class	*/
  int16 stl;			/* Stealth factor	*/

  int16 bth;			/* Base to hit	*/
  int16 bthb;			/* BTH with bows	*/
  int16 ptohit;			/* Plusses to hit	*/
  int16 ptodam;			/* Plusses to dam	*/
  int16 pac;			/* Total AC		*/
  int16 ptoac;			/* Magical AC	*/

  int16 dis_th;			/* Display +ToHit	*/
  int16 dis_td;			/* Display +ToDam	*/
  int16 dis_ac;			/* Display +ToAC	*/
  int16 dis_tac;		/* Display +ToTAC	*/


  int32u status;		/* Status of player	   */

  int16 rest;			/* Rest counter	   */
  int16 food;			/* Food counter	   */
  int16 food_digested;		/* Food per round	   */
  int16 speed;			/* Cur speed adjust	   */

  int16 blind;			/* Blindness counter   */
  int16 paralysis;		/* Paralysis counter   */
  int16 confused;		/* Confusion counter   */
  int16 protection;		/* Protection fr. evil */
  int16 fast;			/* Temp speed change   */
  int16 slow;			/* Temp speed change   */
  int16 afraid;			/* Fear		   */
  int16 cut;			/* Wounds		   */
  int16 stun;			/* Stunned player	   */
  int16 poisoned;		/* Poisoned		   */
  int16 image;			/* Hallucinate	   */

  int16 protevil;		/* Protect VS evil	   */
  int16 invuln;			/* Increases AC	   */
  int16 hero;			/* Heroism		   */
  int16 shero;			/* Super Heroism	   */
  int16 shield;			/* Shield Spell	   */
  int16 blessed;		/* Blessed		   */
  int16 detect_inv;		/* Timed see invisible */
  int16 word_recall;		/* Timed teleport level*/
  int16 see_infra;		/* See warm creatures  */
  int16 tim_infra;		/* Timed infra vision  */

  int16 oppose_acid;		/* Timed acid resist   */
  int16 oppose_elec;		/* Timed lightning resist  */
  int16 oppose_fire;		/* Timed heat resist   */
  int16 oppose_cold;		/* Timed cold resist   */
  int16 oppose_pois;		/* Timed poison resist */

  int8u immune_acid;		/* Immune to acid	   */
  int8u immune_elec;		/* Immune to lightning     */
  int8u immune_fire;		/* Immune to fire	   */
  int8u immune_cold;		/* Immune to cold	   */
  int8u immune_pois;		/* Immune to poison	   */

  int8u resist_acid;		/* Resistance to acid  */
  int8u resist_elec;		/* Resistance to lightning */
  int8u resist_fire;		/* Resistance to fire  */
  int8u resist_cold;		/* Resistance to cold  */
  int8u resist_pois;		/* Resistance to poison	   */

  int8u resist_conf;		/* Resist confusion	*/
  int8u resist_sound;		/* Resist sound		*/
  int8u resist_lite;		/* Resist light		*/
  int8u resist_dark;		/* Resist darkness	*/
  int8u resist_chaos;		/* Resist chaos		*/
  int8u resist_disen;		/* Resist disenchant	*/
  int8u resist_shards;		/* Resist shards	*/
  int8u resist_nexus;		/* Resist nexus		*/
  int8u resist_blind;		/* Resist blindness	*/
  int8u resist_nether;		/* Resist nether	*/
  int8u resist_fear;		/* Resist fear		*/

  int8u sustain_str;		/* Keep strength	*/
  int8u sustain_int;		/* Keep intelligence	*/
  int8u sustain_wis;		/* Keep wisdom		*/
  int8u sustain_dex;		/* Keep dexterity	*/
  int8u sustain_con;		/* Keep constitution	*/
  int8u sustain_chr;		/* Keep charisma	*/

  int8u aggravate;		/* Aggravate monsters	*/
  int8u teleport;		/* Random teleporting	*/

  int8u ffall;			/* No damage falling	*/
  int8u lite;			/* Permanent light	*/
  int8u free_act;		/* Never paralyzed	*/
  int8u see_inv;		/* Can see invisible	*/
  int8u regenerate;		/* Regenerate hit pts	*/
  int8u hold_life;		/* Immune to drain-life	*/
  int8u telepathy;		/* Has telepathy	*/
  int8u slow_digest;		/* Lower food needs	*/
  int8u confusing;		/* Glowing hands.	*/
  int8u searching;		/* Currently searching	*/
};


