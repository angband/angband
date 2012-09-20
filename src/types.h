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
 * in many cases, so try to only use "u16b" and "u32b" for "bit flags",
 * unless you really need the extra bit of information, or you really
 * need to restrict yourself to a single byte for storage reasons.
 *
 * Also, if possible, attempt to restrict yourself to sub-fields of
 * known size (use "s16b" or "s32b" instead of "int", and "byte" instead
 * of "bool"), and attempt to align all fields along four-byte words, to
 * optimize storage issues on 32-bit machines.  Also, avoid "bit flags"
 * since these increase the code size and slow down execution.  When
 * you need to store bit flags, use one byte per flag, or, where space
 * is an issue, use a "byte" or "u16b" or "u32b", and add special code
 * to access the various bit flags.
 *
 * Many of these structures were developed to reduce the number of global
 * variables, facilitate structured program design, allow the use of ascii
 * template files, simplify access to indexed data, or facilitate efficient
 * clearing of many variables at once.
 *
 * Certain data is saved in multiple places for efficient access, currently,
 * this includes the tval/sval/weight fields in "inven_type", various fields
 * in "header_type", and the "m_idx" and "i_idx" fields in "cave_type".  All
 * of these could be removed, but this would, in general, slow down the game
 * and increase the complexity of the code.
 */
 




/*
 * Template file header information (see "init.c").  16 bytes.
 *
 * Note that the sizes of many of the "arrays" are between 32768 and
 * 65535, and so we must use "unsigned" values to hold the "sizes" of
 * these arrays below.  Normally, I try to avoid using unsigned values,
 * since they can cause all sorts of bizarre problems, but I have no
 * choice here, at least, until the "race" array is split into "normal"
 * and "unique" monsters, which may or may not actually help.
 *
 * Note that, on some machines, for example, the Macintosh, the standard
 * "read()" and "write()" functions cannot handle more than 32767 bytes
 * at one time, so we need replacement functions, see "util.c" for details.
 *
 * Note that, on some machines, for example, the Macintosh, the standard
 * "malloc()" function cannot handle more than 32767 bytes at one time,
 * but we may assume that the "ralloc()" function can handle up to 65535
 * butes at one time.  We should not, however, assume that the "ralloc()"
 * function can handle more than 65536 bytes at a time, since this might
 * result in segmentation problems on certain older machines, and in fact,
 * we should not assume that it can handle exactly 65536 bytes at a time,
 * since the internal functions may use an unsigned short to specify size.
 *
 * In general, these problems occur only on machines (such as most personal
 * computers) which use 2 byte "int" values, and which use "int" for the
 * arguments to the relevent functions.
 */

typedef struct header header;

struct header {

    byte	v_major;		/* Version -- major */
    byte	v_minor;		/* Version -- minor */
    byte	v_patch;		/* Version -- patch */
    byte	v_extra;		/* Version -- extra */


    u16b	info_num;		/* Number of "info" records */

    u16b	info_len;		/* Size of each "info" record */


    u16b	head_size;		/* Size of the "header" in bytes */

    u16b	info_size;		/* Size of the "info" array in bytes */

    u16b	name_size;		/* Size of the "name" array in bytes */

    u16b	text_size;		/* Size of the "text" array in bytes */
};



/*
 * Information about terrain "features"
 */

typedef struct feature_type feature_type;

struct feature_type {

    u16b name;			/* Name (offset)		*/
    u16b text;			/* Text (offset)		*/

    byte f_attr;		/* Object "attribute"		*/
    char f_char;		/* Object "symbol"		*/

    byte z_attr;	/* The desired attr for this feature	*/
    char z_char;	/* The desired char for this feature	*/
};


/*
 * Information about object "kinds", including player knowledge.
 *
 * Only "aware" and "tried" are saved in the savefile
 */

typedef struct inven_kind inven_kind;

struct inven_kind {

    u16b name;			/* Name (offset)		*/
    u16b text;			/* Text (offset)		*/

    byte tval;			/* Object type			*/
    byte sval;			/* Object sub type		*/

    s16b pval;			/* Object extra info		*/

    s16b to_h;			/* Bonus to hit			*/
    s16b to_d;			/* Bonus to damage		*/
    s16b to_a;			/* Bonus to armor		*/

    s16b ac;			/* Base armor			*/

    byte dd, ds;		/* Damage dice/sides		*/

    s16b weight;		/* Weight			*/

    s32b cost;			/* Object "base cost"		*/

    u32b flags1;		/* Flags, set 1			*/
    u32b flags2;		/* Flags, set 2			*/
    u32b flags3;		/* Flags, set 3			*/

    byte locale[4];		/* Allocation level(s)		*/
    byte chance[4];		/* Allocation chance(s)		*/

    byte level;			/* Level			*/
    byte extra;			/* Something			*/


    byte k_attr;		/* Object "attribute"		*/
    char k_char;		/* Object "symbol"		*/


    byte i_attr;	/* The underlying attr for this object	*/
    char i_char;	/* The underlying char for this object	*/

    byte x_attr;	/* The desired attr for this object	*/
    char x_char;	/* The desired char for this object	*/


    bool has_flavor;	/* This object has a flavor			*/

    bool easy_know;	/* This object is always known (if aware)	*/


    bool aware;		/* The player is "aware" of the item's effects	*/

    bool tried;		/* The player has "tried" one of the items	*/
};



/*
 * Information about "artifacts".
 *
 * Note that the save-file only writes "cur_num" to the savefile.
 *
 * Note that "max_num" is always "1" (if that artifact "exists")
 */

typedef struct artifact_type artifact_type;

struct artifact_type {

    u16b name;			/* Name (offset)		*/
    u16b text;			/* Text (offset)		*/

    byte tval;			/* Artifact type		*/
    byte sval;			/* Artifact sub type		*/

    s16b pval;			/* Artifact extra info		*/

    s16b to_h;			/* Bonus to hit			*/
    s16b to_d;			/* Bonus to damage		*/
    s16b to_a;			/* Bonus to armor		*/

    s16b ac;			/* Base armor			*/

    byte dd, ds;		/* Damage when hits		*/

    s16b weight;		/* Weight			*/

    s32b cost;			/* Artifact "cost"		*/

    u32b flags1;		/* Artifact Flags, set 1	*/
    u32b flags2;		/* Artifact Flags, set 2	*/
    u32b flags3;		/* Artifact Flags, set 3	*/

    byte level;			/* Artifact level		*/
    byte rarity;		/* Artifact rarity		*/

    byte cur_num;		/* Number created (0 or 1)	*/
    byte max_num;		/* Unused (should be "1")	*/
};


/*
 * Information about "ego-items".
 */

typedef struct ego_item_type ego_item_type;

struct ego_item_type {

    u16b name;			/* Name (offset)		*/
    u16b text;			/* Text (offset)		*/

    byte slot;			/* Standard slot value		*/
    byte rating;		/* Rating boost			*/

    byte level;			/* Minimum level		*/
    byte rarity;		/* Object rarity		*/

    byte max_to_h;		/* Maximum to-hit bonus		*/
    byte max_to_d;		/* Maximum to-dam bonus		*/
    byte max_to_a;		/* Maximum to-ac bonus		*/

    byte max_pval;		/* Maximum pval			*/

    s32b cost;			/* Ego-item "cost"		*/

    u32b flags1;		/* Ego-Item Flags, set 1	*/
    u32b flags2;		/* Ego-Item Flags, set 2	*/
    u32b flags3;		/* Ego-Item Flags, set 3	*/
};




/*
 * Monster blow structure
 *
 *	- Method (RBM_*)
 *	- Effect (RBE_*)
 *	- Damage Dice
 *	- Damage Sides
 */
 
typedef struct monster_blow monster_blow;

struct monster_blow {
    byte method;
    byte effect;
    byte d_dice;
    byte d_side;
};



/*
 * Monster "race" information, including racial memories
 *
 * Note that "r_attr" and "r_char" are used for MORE than "visual" stuff.
 *
 * Note that "l_attr" and "l_char" are used ONLY for "visual" stuff.
 *
 * Note that "cur_num" (and "max_num") represent the number of monsters
 * of the given race currently on (and allowed on) the current level.
 * This information yields the "dead" flag for Unique monsters.
 *
 * Note that "max_num" is reset when a new player is created.
 * Note that "cur_num" is reset when a new level is created.
 *
 * Note that several of these fields, related to "recall", can be
 * scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc).  All of the "recall"
 * fields have a special prefix to aid in searching for them.
 */


typedef struct monster_race monster_race;

struct monster_race {

    u16b name;			/* Name (offset)		*/
    u16b text;			/* Text (offset)		*/

    byte hdice;			/* Creatures hit dice count	*/
    byte hside;			/* Creatures hit dice sides	*/

    s16b ac;			/* Armour Class			*/

    s16b sleep;			/* Inactive counter (base)	*/
    byte aaf;			/* Area affect radius (1-100)	*/
    byte speed;			/* Speed (normally 110)		*/

    s32b mexp;			/* Exp value for kill		*/

    s16b extra;			/* Unused (for now)		*/
  
    byte freq_inate;		/* Inate spell frequency	*/
    byte freq_spell;		/* Other spell frequency	*/
  
    u32b flags1;		/* Flags 1 (general)		*/
    u32b flags2;		/* Flags 2 (abilities)		*/
    u32b flags3;		/* Flags 3 (race/resist)	*/
    u32b flags4;		/* Flags 4 (inate/breath)	*/
    u32b flags5;		/* Flags 5 (normal spells)	*/
    u32b flags6;		/* Flags 6 (special spells)	*/

    monster_blow blow[4];	/* Up to four blows per round	*/


    byte level;			/* Level of creature		*/
    byte rarity;		/* Rarity of creature		*/


    byte r_attr;		/* Racial "color"		*/
    char r_char;		/* Racial "symbol"		*/


    byte l_attr;		/* Desired monster attribute	*/
    char l_char;		/* Desired monster character	*/


    byte max_num;		/* Maximum population allowed per level */

    byte cur_num;		/* Monster population on current level */


    s16b r_sights;		/* Count sightings of this monster	*/
    s16b r_deaths;		/* Count deaths from this monster	*/

    s16b r_pkills;		/* Count monsters killed in this life	*/
    s16b r_tkills;		/* Count monsters killed in all lives	*/

    byte r_wake;		/* Number of times woken up (?)		*/
    byte r_ignore;		/* Number of times ignored (?)		*/

    byte r_xtra1;		/* Something (unused)			*/
    byte r_xtra2;		/* Something (unused)			*/

    byte r_drop_gold;		/* Max number of gold dropped at once	*/
    byte r_drop_item;		/* Max number of item dropped at once	*/

    byte r_cast_inate;		/* Max number of inate spells seen	*/
    byte r_cast_spell;		/* Max number of other spells seen	*/

    byte r_blows[4];		/* Number of times each blow type was seen */

    u32b r_flags1;		/* Observed racial flags */
    u32b r_flags2;		/* Observed racial flags */
    u32b r_flags3;		/* Observed racial flags */
    u32b r_flags4;		/* Observed racial flags */
    u32b r_flags5;		/* Observed racial flags */
    u32b r_flags6;		/* Observed racial flags */
};



/*
 * Information about "vault generation"
 */

typedef struct vault_type vault_type;

struct vault_type {

    u16b name;			/* Name (offset)		*/
    u16b text;			/* Text (offset)		*/

    byte typ;			/* Vault type			*/

    byte rat;			/* Vault rating			*/

    byte hgt;			/* Vault height			*/
    byte wid;			/* Vault width			*/
};





/*
 * A single "grid" in a Cave
 *
 * Note that several aspects of the code restrict the actual cave
 * to a max size of 256 by 256.  In partcular, locations are often
 * saved as bytes, limiting each coordinate to the 0-255 range.
 *
 * The "m_idx" and "i_idx" fields are very interesting.  There are
 * many places in the code where we need quick access to the actual
 * monster or object(s) in a given cave grid.  The easiest way to
 * do this is to simply keep the index of the monster and object
 * (if any) with the grid, but takes a lot of memory.  Several other
 * methods come to mind, but they all seem rather complicated.
 *
 * Note the special fields for the simple "monster flow" code,
 * and for the "tracking" code.
 */

typedef struct cave_type cave_type;

struct cave_type {

  s16b m_idx;		/* Monster index (in m_list) or zero	*/
  
  s16b i_idx;		/* Item index (in i_list) or zero	*/

#ifdef MONSTER_FLOW

  byte cost;		/* Hack -- cost of flowing		*/
  byte when;		/* Hack -- when cost was computed	*/

#endif

  u16b feat;		/* Flags -- use the "CAVE_*" constants	*/
};



/*
 * Structure for an object. (32 bytes)
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that "object" records are "copied" on a fairly regular basis.
 *
 * Note that "object flags" must now be derived from the object kind,
 * the artifact and ego-item indexes, and the two "xtra" fields.
 */

typedef struct inven_type inven_type;

struct inven_type {

  s16b k_idx;			/* Kind index (zero if "dead")	*/

  byte iy;			/* Y-position on map, or zero	*/
  byte ix;			/* X-position on map, or zero	*/

  byte tval;			/* Item type (from kind)	*/
  byte sval;			/* Item sub-type (from kind)	*/

  s16b pval;			/* Item extra-parameter		*/

  byte discount;		/* Discount (if any)		*/

  byte number;			/* Number of items		*/

  s16b weight;			/* Item weight			*/

  byte name1;			/* Artifact type, if any	*/
  byte name2;			/* Ego-Item type, if any	*/
  
  byte xtra1;			/* Extra info type		*/
  byte xtra2;			/* Extra info index		*/

  s16b to_h;			/* Plusses to hit		*/
  s16b to_d;			/* Plusses to damage		*/
  s16b to_a;			/* Plusses to AC		*/

  s16b ac;			/* Normal AC			*/

  byte dd, ds;			/* Damage dice/sides		*/

  s16b timeout;			/* Timeout Counter		*/

  u16b ident;			/* General flags 		*/

  u16b note;			/* Inscription index		*/
};



/*
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 */

typedef struct monster_type monster_type;

struct monster_type {

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

  byte t_dur;			/* How long are we tracking	*/

  byte t_bit;			/* Up to eight bit flags	*/

#endif

#ifdef DRS_SMART_OPTIONS

  u32b smart;			/* Field for "smart_learn"	*/

#endif

};




/*
 * An entry for the object kind allocator function
 */

typedef struct kind_entry kind_entry;

struct kind_entry {
    u16b k_idx;		/* Object kind index */
    byte locale;	/* Base dungeon level */
    byte chance;	/* Rarity of occurance */
};


/*
 * An entry for the monster race allocator function
 */

typedef struct race_entry race_entry;

struct race_entry {
    u16b r_idx;		/* Monster race index */
    byte locale;	/* Base dungeon level */
    byte chance;	/* Rarity of occurance */
};



/*
 * Available "options"
 *
 *	- Address of actual option variable (or NULL)
 *
 *	- Normal Value (TRUE or FALSE)
 *
 *	- Option Page Number (or zero)
 *
 *	- Savefile Set (or zero)
 *	- Savefile Bit in that set
 *
 *	- Textual name (or NULL)
 *	- Textual description
 */

typedef struct option_type option_type;

struct option_type {

    bool	*o_var;

    byte	o_norm;
    
    byte	o_page;

    byte	o_set;
    byte	o_bit;
    
    cptr	o_text;
    cptr	o_desc;
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
 *
 * Actually, in Angband 2.8.0 it will probably prove easier to restrict
 * the concept of quest monsters to specific unique monsters, and to
 * actually scan the dead unique list to see what quests are left.
 */

typedef struct quest quest;

struct quest {

    int level;		/* Dungeon level */
    int r_idx;		/* Monster race */

    int cur_num;	/* Number killed (unused) */
    int max_num;	/* Number required (unused) */
};




/*
 * A store owner
 */

typedef struct owner_type owner_type;

struct owner_type {

  cptr owner_name;	/* Name */

  s16b max_cost;	/* Purse limit */

  byte max_inflate;	/* Inflation (max) */
  byte min_inflate;	/* Inflation (min) */

  byte haggle_per;	/* Haggle unit */

  byte insult_max;	/* Insult limit */

  byte owner_race;	/* Owner race */

  byte unused;		/* Unused */
};




/*
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */

typedef struct store_type store_type;

struct store_type {

  byte owner;			/* Owner index			*/
  byte extra;			/* Unused for now		*/
  
  s16b insult_cur;		/* Insult counter		*/

  s16b good_buy;		/* Number of "good" buys	*/
  s16b bad_buy;			/* Number of "bad" buys		*/

  s32b store_open;		/* Closed until this turn	*/

  s32b store_wrap;		/* Unused for now		*/
  
  s16b table_num;		/* Table -- Number of entries	*/
  s16b table_size;		/* Table -- Total Size of Array	*/
  s16b *table;			/* Table -- Legal item kinds	*/
  
  s16b stock_num;		/* Stock -- Number of entries	*/
  s16b stock_size;		/* Stock -- Total Size of Array	*/
  inven_type *stock;		/* Stock -- Actual stock items	*/
};





/*
 * The "name" of spell 'N' is stored as spell_names[X][N],
 * where X is 0 for mage-spells and 1 for priest-spells.
 */

typedef struct magic_type magic_type;

struct magic_type {

  byte slevel;			/* Required level (to learn)	*/
  byte smana;			/* Required mana (to cast)	*/
  byte sfail;			/* Minimum chance of failure	*/
  byte sexp;			/* Encoded experience bonus	*/
};


/*
 * Information about the player's "magic"
 *
 * Note that a player with a "spell_book" of "zero" is illiterate.
 */

typedef struct player_magic player_magic;

struct player_magic {

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
 
typedef struct player_race player_race;

struct player_race {

  cptr title;			/* Type of race			*/

  s16b r_adj[6];		/* Racial stat bonuses		*/

  s16b r_dis;			/* disarming			*/
  s16b r_dev;			/* magic devices		*/
  s16b r_sav;			/* saving throw			*/
  s16b r_stl;			/* stealth			*/
  s16b r_srh;			/* search ability		*/
  s16b r_fos;			/* search frequency		*/
  s16b r_thn;			/* combat (normal)		*/
  s16b r_thb;			/* combat (shooting)		*/

  byte r_mhp;			/* Race hit-dice modifier	*/
  byte r_exp;			/* Race experience factor	*/

  byte b_age;			/* base age			*/
  byte m_age;			/* mod age			*/

  byte m_b_ht;			/* base height (males)		*/
  byte m_m_ht;			/* mod height (males)		*/
  byte m_b_wt;			/* base weight (males)		*/
  byte m_m_wt;			/* mod weight (males)		*/
  
  byte f_b_ht;			/* base height (females)	*/
  byte f_m_ht;			/* mod height (females)	 	*/
  byte f_b_wt;			/* base weight (females)	*/
  byte f_m_wt;			/* mod weight (females)		*/

  byte infra;			/* Infra-vision	range		*/

  byte choice;			/* Legal class choices		*/
};


/*
 * Player class info
 */

typedef struct player_class player_class;

struct player_class {

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

  s16b c_mhp;			/* Class hit-dice adjustment	*/
  s16b c_exp;			/* Class experience factor	*/
};




/*
 * Most of the "player" information goes here.
 *
 * This stucture gives us a large collection of player variables.
 *
 * This structure contains several "blocks" of information.
 *   (1) the "permanent" info
 *   (2) the "variable" info
 *   (3) the "transient" info
 *
 * All of the "permanent" info, and most of the "variable" info,
 * is saved in the savefile.  The "transient" info is recomputed
 * whenever anything important changes.
 */

typedef struct player_type player_type;

struct player_type {

  byte prace;			/* Race index		*/
  byte pclass;			/* Class index		*/
  byte male;			/* Sex of character	*/
  byte oops;			/* Unused		*/

  byte hitdie;			/* Hit dice (sides)	*/
  byte expfact;			/* Experience factor	*/

  byte maximize;		/* Maximize stats	*/
  byte preserve;		/* Preserve artifacts	*/

  s16b age;			/* Characters age	*/
  s16b ht;			/* Height		*/
  s16b wt;			/* Weight		*/
  s16b sc;			/* Social Class		*/


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

  s16b stat_max[6];		/* Current "maximal" stat values	*/
  s16b stat_cur[6];		/* Current "natural" stat values	*/

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

  s16b word_recall;		/* Word of recall counter	*/

  s16b energy;			/* Current energy		*/

  s16b food;			/* Current nutrition		*/

  byte confusing;		/* Glowing hands		*/
  byte searching;		/* Currently searching		*/

  s16b new_spells;		/* Number of spells available	*/

  s16b old_spells;

  bool old_cumber_armor;
  bool old_cumber_glove;
  bool old_heavy_wield;
  bool old_heavy_shoot;
  bool old_icky_wield;

  s16b old_lite;		/* Old radius of lite (if any)	*/
  s16b old_view;		/* Old radius of view (if any)	*/

  s16b old_cut;			/* Old value of "cut"		*/
  s16b old_stun;		/* Old value of "stun"		*/
  
  s16b old_food_aux;		/* Old value of food		*/


  bool cumber_armor;		/* Mana draining armor		*/
  bool cumber_glove;		/* Mana draining gloves		*/
  bool heavy_wield;		/* Heavy weapon			*/
  bool heavy_shoot;		/* Heavy shooter		*/
  bool icky_wield;		/* Icky weapon			*/
  
  s16b cur_lite;		/* Radius of lite (if any)	*/
  s16b cur_view;		/* Radius of view (if any)	*/


  u32b notice;			/* Noticed Things (bit flags)	*/

  u32b update;			/* Pending Updates (bit flags)	*/
  u32b redraw;			/* Desired Redraws (bit flags)	*/


  s16b stat_use[6];		/* Resulting stat values	*/

  s16b stat_ind[6];		/* Indexes into stat tables	*/

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

  byte exp_drain;		/* Experience draining	*/

  byte ffall;			/* No damage falling	*/
  byte lite;			/* Permanent light	*/
  byte free_act;		/* Never paralyzed	*/
  byte see_inv;			/* Can see invisible	*/
  byte regenerate;		/* Regenerate hit pts	*/
  byte hold_life;		/* Resist life draining	*/
  byte telepathy;		/* Telepathy		*/
  byte slow_digest;		/* Slower digestion	*/
  byte bless_blade;		/* Blessed blade	*/
  byte xtra_might;		/* Extra might bow	*/

  s16b dis_to_h;		/* Known bonus to hit	*/
  s16b dis_to_d;		/* Known bonus to dam	*/
  s16b dis_to_a;		/* Known bonus to ac	*/

  s16b dis_ac;			/* Known base ac	*/

  s16b to_h;			/* Bonus to hit		*/
  s16b to_d;			/* Bonus to dam		*/
  s16b to_a;			/* Bonus to ac		*/

  s16b ac;			/* Base ac		*/

  s16b see_infra;		/* Infravision range	*/

  s16b skill_dis;		/* Skill: Disarming		*/
  s16b skill_dev;		/* Skill: Magic Devices		*/
  s16b skill_sav;		/* Skill: Saving throw		*/
  s16b skill_stl;		/* Skill: Stealth factor	*/
  s16b skill_srh;		/* Skill: Searching ability	*/
  s16b skill_fos;		/* Skill: Searching frequency	*/
  s16b skill_thn;		/* Skill: To hit (normal)	*/
  s16b skill_thb;		/* Skill: To hit (shooting)	*/
  s16b skill_tht;		/* Skill: To hit (throwing)	*/
  s16b skill_dig;		/* Skill: Digging		*/

  s16b num_blow;		/* Number of blows	*/
  s16b num_fire;		/* Number of shots	*/

  byte tval_xtra;		/* Correct xtra tval	*/

  byte tval_ammo;		/* Correct ammo tval	*/

  s16b pspeed;			/* Current speed	*/

  s16b food_digested;		/* Food per round	*/
};


