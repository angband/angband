#ifndef INCLUDED_TYPES_H
#define INCLUDED_TYPES_H

/*
 * This file contains various defined types used by the game.
 *
 * TODO: Most of these should be elsewhere, in their own header files.
 * For example, the object structs should be in object.h.
 *
 * Be careful when creating data structures; most of these are designed
 * to be serialised to file, so be careful to use exact-size data types
 * (like u32b and s32b) and not just "int"s.
 */

/**** Available Types ****/

/** An array of 256 bytes */
typedef byte byte_256[256];

/** An array of DUNGEON_WID bytes */
typedef byte byte_wid[DUNGEON_WID];

/** An array of DUNGEON_WID s16b's */
typedef s16b s16b_wid[DUNGEON_WID];



/** Function hook types **/

/** Function prototype for the UI to provide to create native buttons */
typedef int (*button_add_f)(const char *, unsigned char);

/** Function prototype for the UI to provide to remove native buttons */
typedef int (*button_kill_f)(unsigned char);



/**** Available Structs ****/

typedef struct ego_item_type ego_item_type;
typedef struct monster_blow monster_blow;
typedef struct monster_race monster_race;
typedef struct monster_lore monster_lore;
typedef struct vault_type vault_type;
typedef struct object_type object_type;
typedef struct monster_type monster_type;
typedef struct alloc_entry alloc_entry;
typedef struct quest quest;
typedef struct owner_type owner_type;
typedef struct store_type store_type;
typedef struct magic_type magic_type;
typedef struct player_magic player_magic;
typedef struct spell_type spell_type;
typedef struct player_sex player_sex;
typedef struct player_race player_race;
typedef struct player_class player_class;
typedef struct hist_type hist_type;
typedef struct player_other player_other;
typedef struct player_type player_type;
typedef struct start_item start_item;
typedef struct autoinscription autoinscription;
typedef struct history_info history_info;


/**
 * Information about maximal indices of certain arrays.
 *
 * These are actually not the maxima, but the maxima plus one, because of
 * 0-based indexing issues.
 */
typedef struct
{
	u32b fake_text_size;  /**< Max size of all descriptions read in from lib/edit */
	u32b fake_name_size;  /**< Max size of all names read in from lib/edit */

	u16b f_max;       /**< Maximum number of terrain features */
	u16b k_max;       /**< Maximum number of object base kinds */
	u16b a_max;       /**< Maximum number of artifact kinds */
	u16b e_max;       /**< Maximum number of ego-item kinds */
	u16b r_max;       /**< Maximum number of monster races */
	u16b v_max;       /**< Maximum number of vault kinds */
	u16b p_max;       /**< Maximum number of player races */
	u16b h_max;       /**< Maximum number of chained player history entries */
	u16b b_max;       /**< Maximum number of shop owners per store kind */
	u16b c_max;       /**< Maximum number of player classes */
	u16b flavor_max;  /**< Maximum number of item flavour kinds */
	u16b s_max;       /**< Maximum number of magic spells */

	u16b o_max;       /**< Maximum number of objects on a given level */
	u16b m_max;       /**< Maximum number of monsters on a given level */
} maxima;


/**
 * Information about terrain features.
 *
 * At the moment this isn't very much, but eventually a primitive flag-based
 * information system will be used here.
 */
typedef struct
{
	u32b name;    /**< (const char *) feature_type::name + f_name = Name */
	u32b text;    /**< (const char *) feature_type::text + f_text = Description (unused) */

	byte mimic;   /**< Feature to mimic */

	byte extra;   /**< Unused */
	s16b unused;  /**< Unused */

	byte d_attr;  /**< Default feature attribute */
	char d_char;  /**< Default feature character */

	byte x_attr;  /**< Desired feature attribute (set by user/pref file) */
	char x_char;  /**< Desired feature character (set by user/pref file) */
} feature_type;


/**
 * Information about object kinds, including player knowledge.
 *
 * TODO: split out the user-changeable bits into a separate struct so this
 * one can be read-only.
 */
typedef struct
{
	/** Constants **/

	u32b name;     /**< (const char *) object_kind::name + k_name = Name */
	u32b text;     /**< (const char *) object_kind::text + k_text = Description  */

	byte tval;     /**< General object type (see TV_ macros) */
	byte sval;     /**< Object sub-type (see SV_ macros) */
	s16b pval;     /**< Power for any flags which need it */

	s16b to_h;     /**< Bonus to-hit */
	s16b to_d;     /**< Bonus to damage */
	s16b to_a;     /**< Bonus to armor */
	s16b ac;       /**< Base armor */

	byte dd;       /**< Damage dice */
	byte ds;       /**< Damage sides */
	s16b weight;   /**< Weight, in 1/10lbs */

	s32b cost;     /**< Object base cost */

	u32b flags1;   /**< Flags, set 1 (see TR1_ macros) */
	u32b flags2;   /**< Flags, set 2 (see TR2_ macros) */
	u32b flags3;   /**< Flags, set 3 (see TR3_ macros) */

	byte d_attr;   /**< Default object attribute */
	char d_char;   /**< Default object character */

	byte alloc_prob;   /**< Allocation: commonness */
	byte alloc_min;    /**< Highest normal dungeon level */
	byte alloc_max;    /**< Lowest normal dungeon level */
	byte level;        /**< Level */

	u16b effect;       /**< Effect this item produces (effects.c) */
	u16b time_base;    /**< Recharge time (if appropriate) */
	byte time_dice;    /**< Randomised recharge time dice */
	byte time_sides;   /**< Randomised recharge time sides */

	byte charge_base;  /**< Non-random initial charge base */
	byte charge_dd;    /**< Randomised initial charge dice */
	byte charge_ds;    /**< Randomised initial charge sides */

	byte gen_mult_prob; /**< Probability of generating more than one */
	byte gen_dice;      /**< Number to generate dice */
	byte gen_side;      /**< Number to generate sides */

	u16b flavor;        /**< Special object flavor (or zero) */


	/** Game-dependent **/

	byte x_attr;   /**< Desired object attribute (set by user/pref file) */
	char x_char;   /**< Desired object character (set by user/pref file) */

	/** Also saved in savefile **/

	u16b note;     /**< Autoinscription quark number */

	bool aware;    /**< Set if player is aware of the kind's effects */
	bool tried;    /**< Set if kind has been tried */

	bool squelch;  /**< Set if kind should be squelched */
	bool everseen; /**< Set if kind has ever been seen (to despoilify squelch menus) */
} object_kind;



/**
 * Information about artifacts.
 *
 * Note that ::cur_num is written to the savefile.
 *
 * TODO: Fix this max_num/cur_num crap and just have a big boolean array of
 * which artifacts have been created and haven't, so this can become read-only.
 */
typedef struct
{
	u32b name;    /**< (const char *) artifact_type::name + a_name = Name */
	u32b text;    /**< (const char *) artifact_type::text + a_text = Description  */

	byte tval;    /**< General artifact type (see TV_ macros) */
	byte sval;    /**< Artifact sub-type (see SV_ macros) */
	s16b pval;    /**< Power for any flags which need it */

	s16b to_h;    /**< Bonus to hit */
	s16b to_d;    /**< Bonus to damage */
	s16b to_a;    /**< Bonus to armor */
	s16b ac;      /**< Base armor */

	byte dd;      /**< Base damage dice */
	byte ds;      /**< Base damage sides */

	s16b weight;  /**< Weight in 1/10lbs */

	s32b cost;    /**< Artifact (pseudo-)worth */

	u32b flags1;  /**< Flags, set 1 (see TR1_ macros) */
	u32b flags2;  /**< Flags, set 2 (see TR2_ macros) */
	u32b flags3;  /**< Flags, set 3 (see TR3_ macros) */

	byte level;   /**< Minimum depth artifact can appear at */
	byte rarity;  /**< Artifact rarity */

	byte max_num; /**< Unused (should be "1") */
	byte cur_num; /**< Number created (0 or 1) */

	u16b effect;     /**< Artifact activation (see effects.c) */
	u32b effect_msg; /**< (const char *) artifact_type::effect_msg + a_text = Effect message */
	u16b time_base;  /**< Recharge time (if appropriate) */
	byte time_dice;  /**< Randomised recharge time dice */
	byte time_sides; /**< Randomised recharge time sides */

} artifact_type;


/*
 * Information about "ego-items".
 */
struct ego_item_type
{
	u32b name;			/* Name (offset) */
	u32b text;			/* Text (offset) */

	s32b cost;			/* Ego-item "cost" */

	u32b flags1;		/* Ego-Item Flags, set 1 */
	u32b flags2;		/* Ego-Item Flags, set 2 */
	u32b flags3;		/* Ego-Item Flags, set 3 */

	byte level;			/* Minimum level */
	byte rarity;		/* Object rarity */
	byte rating;		/* Level rating boost */

	byte tval[EGO_TVALS_MAX]; /* Legal tval */
	byte min_sval[EGO_TVALS_MAX];	/* Minimum legal sval */
	byte max_sval[EGO_TVALS_MAX];	/* Maximum legal sval */

	byte max_to_h;		/* Maximum to-hit bonus */
	byte max_to_d;		/* Maximum to-dam bonus */
	byte max_to_a;		/* Maximum to-ac bonus */
	byte max_pval;		/* Maximum pval */

	byte xtra;			/* Extra sustain/resist/power */

	bool everseen;		/* Do not spoil squelch menus */
};




/*
 * Monster blow structure
 *
 *	- Method (RBM_*)
 *	- Effect (RBE_*)
 *	- Damage Dice
 *	- Damage Sides
 */
struct monster_blow
{
	byte method;
	byte effect;
	byte d_dice;
	byte d_side;
};



/*
 * Monster "race" information, including racial memories
 *
 * Note that "d_attr" and "d_char" are used for MORE than "visual" stuff.
 *
 * Note that "x_attr" and "x_char" are used ONLY for "visual" stuff.
 *
 * Note that "cur_num" (and "max_num") represent the number of monsters
 * of the given race currently on (and allowed on) the current level.
 * This information yields the "dead" flag for Unique monsters.
 *
 * Note that "max_num" is reset when a new player is created.
 * Note that "cur_num" is reset when a new level is created.
 *
 * Maybe "x_attr", "x_char", "cur_num", and "max_num" should
 * be moved out of this array since they are not read from
 * "monster.txt".
 */
struct monster_race
{
	u32b name;				/* Name (offset) */
	u32b text;				/* Text (offset) */

	u16b avg_hp;				/* Average HP for this creature */

	s16b ac;				/* Armour Class */

	s16b sleep;				/* Inactive counter (base) */
	byte aaf;				/* Area affect radius (1-100) */
	byte speed;				/* Speed (normally 110) */

	s32b mexp;				/* Exp value for kill */

	s16b power;				/* Monster power */

#ifdef ALLOW_TEMPLATES_PROCESS

	s16b highest_threat;	/* Monster highest threat */

#endif /* ALLOW_TEMPLATES_PROCESS */

	byte freq_innate;		/* Innate spell frequency */
	byte freq_spell;		/* Other spell frequency */

	u32b flags[RACE_FLAG_STRICT_UB];	/* Flags */
		/* Flags 0 (general) */
		/* Flags 1 (abilities) */
		/* Flags 2 (race/resist) */
	u32b spell_flags[RACE_FLAG_SPELL_STRICT_UB];	/* Spell flags */
		/* Flags 3 (innate/breath) */
		/* Flags 4 (normal spells) */
		/* Flags 5 (special spells) */

	monster_blow blow[MONSTER_BLOW_MAX]; /* Up to four blows per round */

	byte level;				/* Level of creature */
	byte rarity;			/* Rarity of creature */

	byte d_attr;			/* Default monster attribute */
	char d_char;			/* Default monster character */

	byte x_attr;			/* Desired monster attribute */
	char x_char;			/* Desired monster character */

	byte max_num;			/* Maximum population allowed per level */
	byte cur_num;			/* Monster population on current level */
};


/*
 * Monster "lore" information
 *
 * Note that these fields are related to the "monster recall" and can
 * be scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc). XXX XXX XXX
 */
struct monster_lore
{
	s16b sights;			/* Count sightings of this monster */
	s16b deaths;			/* Count deaths from this monster */

	s16b pkills;			/* Count monsters killed in this life */
	s16b tkills;			/* Count monsters killed in all lives */

	byte wake;				/* Number of times woken up (?) */
	byte ignore;			/* Number of times ignored (?) */

	byte drop_gold;			/* Max number of gold dropped at once */
	byte drop_item;			/* Max number of item dropped at once */

	byte cast_innate;		/* Max number of innate spells seen */
	byte cast_spell;		/* Max number of other spells seen */

	byte blows[MONSTER_BLOW_MAX]; /* Number of times each blow type was seen */

	u32b flags[RACE_FLAG_STRICT_UB];	/* Observed racial flags */
	u32b spell_flags[RACE_FLAG_SPELL_STRICT_UB];	/* Observed racial spell flags */
};



/*
 * Information about "vault generation"
 */
struct vault_type
{
	u32b name;			/* Name (offset) */
	u32b text;			/* Text (offset) */

	byte typ;			/* Vault type */

	byte rat;			/* Vault rating */

	byte hgt;			/* Vault height */
	byte wid;			/* Vault width */
};



/*
 * Object information, for a specific object.
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that "object" records are "copied" on a fairly regular basis,
 * and care must be taken when handling such objects.
 *
 * Note that "object flags" must now be derived from the object kind,
 * the artifact and ego-item indexes, and the two "xtra" fields.
 *
 * Each cave grid points to one (or zero) objects via the "o_idx"
 * field (above).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a "stack" of objects in the same grid.
 *
 * Each monster points to one (or zero) objects via the "hold_o_idx"
 * field (below).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a pile of objects held by the monster.
 *
 * The "held_m_idx" field is used to indicate which monster, if any,
 * is holding the object.  Objects being held have "ix=0" and "iy=0".
 */
struct object_type
{
	s16b k_idx;			/* Kind index (zero if "dead") */

	byte iy;			/* Y-position on map, or zero */
	byte ix;			/* X-position on map, or zero */

	byte tval;			/* Item type (from kind) */
	byte sval;			/* Item sub-type (from kind) */

	s16b pval;			/* Item extra-parameter */

	s16b weight;		/* Item weight */

	byte name1;			/* Artifact type, if any */
	byte name2;			/* Ego-Item type, if any */

	u32b flags1;		/* Flags, set 1 */
	u32b flags2;		/* Flags, set 2 */
	u32b flags3;		/* Flags, set 3 */

	s16b ac;			/* Normal AC */
	s16b to_a;			/* Plusses to AC */
	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */

	byte dd, ds;		/* Damage dice/sides */

	s16b timeout;		/* Timeout Counter */

	byte number;		/* Number of items */

	byte pseudo;		/* Pseudo-ID marker */
	byte ident;			/* Special flags */
	byte marked;		/* Object is marked */

	s16b next_o_idx;	/* Next object in stack (if any) */
	s16b held_m_idx;	/* Monster holding us (if any) */

	byte origin;        /* How this item was found */
	byte origin_depth;  /* What depth the item was found at */
	u16b origin_xtra;   /* Extra information about origin */

	quark_t note;			/* Inscription index */
};



/*
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 *
 * The "hold_o_idx" field points to the first object of a stack
 * of objects (if any) being carried by the monster (see above).
 */
struct monster_type
{
	s16b r_idx;			/* Monster race index */

	byte fy;			/* Y location on map */
	byte fx;			/* X location on map */

	s16b hp;			/* Current Hit points */
	s16b maxhp;			/* Max Hit points */

	s16b csleep;		/* Inactive counter */

	byte mspeed;		/* Monster "speed" */
	byte energy;		/* Monster "energy" */

	byte stunned;		/* Monster is stunned */
	byte confused;		/* Monster is confused */
	byte monfear;		/* Monster is afraid */

	byte cdis;			/* Current dis from player */

	byte mflag;			/* Extra monster flags */

	bool ml;			/* Monster is "visible" */

	s16b hold_o_idx;	/* Object being held (if any) */

	u32b smart;			/* Field for "adult_ai_learn" */
};




/*
 * An entry for the object/monster allocation functions
 *
 * Pass 1 is determined from allocation information
 * Pass 2 is determined from allocation restriction
 * Pass 3 is determined from allocation calculation
 */
struct alloc_entry
{
	s16b index;		/* The actual index */

	byte level;		/* Base dungeon level */
	byte prob1;		/* Probability, pass 1 */
	byte prob2;		/* Probability, pass 2 */
	byte prob3;		/* Probability, pass 3 */

	u16b total;		/* Unused for now */
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
struct quest
{
	byte level;		/* Dungeon level */
	int r_idx;		/* Monster race */

	int cur_num;	/* Number killed (unused) */
	int max_num;	/* Number required (unused) */
};




/*
 * A store owner
 */
struct owner_type
{
	u32b owner_name;	/* Name (offset) */
	s32b max_cost;		/* Purse limit */
};




/*
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
struct store_type
{
	byte owner;				/* Owner index */

	byte stock_num;			/* Stock -- Number of entries */
	s16b stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */

	s16b table_num;     /* Table -- Number of entries */
	s16b table_size;    /* Table -- Total Size of Array */
	s16b *table;        /* Table -- Legal item kinds */
};


/*
 * A structure to hold class-dependent information on spells.
 */
struct magic_type
{
	byte slevel;		/* Required level (to learn) */
	byte smana;			/* Required mana (to cast) */
	byte sfail;			/* Minimum chance of failure */
	byte sexp;			/* Encoded experience bonus */
};


/*
 * Information about the player's "magic"
 *
 * Note that a player with a "spell_book" of "zero" is illiterate.
 */
struct player_magic
{
	magic_type info[PY_MAX_SPELLS];	/* The available spells */
};


/*
 * And here's the structure for the "fixed" spell information
 */
struct spell_type
{
	u32b name;			/* Name (offset) */
	u32b text;			/* Text (offset) */

	byte realm;			/* 0 = mage; 1 = priest */
	byte tval;			/* Item type for book this spell is in */
	byte sval;			/* Item sub-type for book (= book number) */
	byte snum;			/* Position of spell within book */

	byte spell_index;	/* Index into player_magic array */
};


/*
 * Player sex info
 */
struct player_sex
{
	cptr title;			/* Type of sex */

	cptr winner;		/* Name of winner */
};


/*
 * Player racial info
 */
struct player_race
{
	u32b name;			/* Name (offset) */
	u32b text;			/* Text (offset) */

	s16b r_adj[A_MAX];	/* Racial stat bonuses */

	s16b r_skills[SKILL_MAX_NO_RACE_CLASS];	/* racial skills */

	byte r_mhp;			/* Race hit-dice modifier */
	byte r_exp;			/* Race experience factor */

	byte b_age;			/* base age */
	byte m_age;			/* mod age */

	byte m_b_ht;		/* base height (males) */
	byte m_m_ht;		/* mod height (males) */
	byte m_b_wt;		/* base weight (males) */
	byte m_m_wt;		/* mod weight (males) */

	byte f_b_ht;		/* base height (females) */
	byte f_m_ht;		/* mod height (females) */
	byte f_b_wt;		/* base weight (females) */
	byte f_m_wt;		/* mod weight (females) */

	byte infra;			/* Infra-vision	range */

	byte choice;		/* Legal class choices */

	s16b hist;			/* Starting history index */

	u32b flags1;		/* Racial Flags, set 1 */
	u32b flags2;		/* Racial Flags, set 2 */
	u32b flags3;		/* Racial Flags, set 3 */
};


/*
 * Starting equipment entry
 */
struct start_item
{
	byte tval;	/* Item's tval */
	byte sval;	/* Item's sval */
	byte min;	/* Minimum starting amount */
	byte max;	/* Maximum starting amount */
};


/*
 * Player class info
 */
struct player_class
{
	u32b name;			/* Name (offset) */

	u32b title[10];		/* Titles - offset */

	s16b c_adj[A_MAX];	/* Class stat modifier */

	s16b c_skills[SKILL_MAX_NO_RACE_CLASS];	/* class skills */
	s16b x_skills[SKILL_MAX_NO_RACE_CLASS];	/* extra skills */

	s16b c_mhp;			/* Class hit-dice adjustment */
	s16b c_exp;			/* Class experience factor */

	u32b flags;			/* Class Flags */

	u16b max_attacks;	/* Maximum possible attacks */
	u16b min_weight;	/* Minimum weapon weight for calculations */
	u16b att_multiply;	/* Multiplier for attack calculations */

	byte spell_book;	/* Tval of spell books (if any) */
	u16b spell_stat;	/* Stat for spells (if any) */
	u16b spell_first;	/* Level of first spell */
	u16b spell_weight;	/* Weight that hurts spells */

	u32b sense_base;	/* Base pseudo-id value */
	u16b sense_div;		/* Pseudo-id divisor */

	start_item start_items[MAX_START_ITEMS];/**< The starting inventory */

	player_magic spells; /* Magic spells */
};


/*
 * Player background information
 */
struct hist_type
{
	u32b text;			    /* Text (offset) */

	byte roll;			    /* Frequency of this entry */
	byte chart;			    /* Chart index */
	byte next;			    /* Next chart index */
	byte bonus;			    /* Social Class Bonus + 50 */
};



/*
 * Some more player information
 *
 * This information is retained across player lives
 */
struct player_other
{
	char full_name[32];		/* Full name */
	char base_name[32];		/* Base name */

	bool opt[OPT_MAX];		/* Options */

	u32b window_flag[ANGBAND_TERM_MAX];	/* Window flags */

	byte hitpoint_warn;		/* Hitpoint warning (0 to 9) */

	byte delay_factor;		/* Delay factor (0 to 9) */
};


/*
 * Most of the "player" information goes here.
 *
 * This stucture gives us a large collection of player variables.
 *
 * This entire structure is wiped when a new character is born.
 *
 * This structure is more or less laid out so that the information
 * which must be saved in the savefile precedes all the information
 * which can be recomputed as needed.
 */
struct player_type
{
	s16b py;			/* Player location */
	s16b px;			/* Player location */

	byte psex;			/* Sex index */
	byte prace;			/* Race index */
	byte pclass;		/* Class index */
	byte oops;			/* Unused */

	byte hitdie;		/* Hit dice (sides) */
	byte expfact;		/* Experience factor */

	s16b age;			/* Characters age */
	s16b ht;			/* Height */
	s16b wt;			/* Weight */
	s16b sc;			/* Social Class */

	s32b au;			/* Current Gold */

	s16b max_depth;		/* Max depth */
	s16b depth;			/* Cur depth */

	s16b max_lev;		/* Max level */
	s16b lev;			/* Cur level */

	s32b max_exp;		/* Max experience */
	s32b exp;			/* Cur experience */
	u16b exp_frac;		/* Cur exp frac (times 2^16) */

	s16b mhp;			/* Max hit pts */
	s16b chp;			/* Cur hit pts */
	u16b chp_frac;		/* Cur hit frac (times 2^16) */

	s16b msp;			/* Max mana pts */
	s16b csp;			/* Cur mana pts */
	u16b csp_frac;		/* Cur mana frac (times 2^16) */

	s16b stat_max[A_MAX];	/* Current "maximal" stat values */
	s16b stat_cur[A_MAX];	/* Current "natural" stat values */

	s16b timed[TMD_MAX];	/* Timed effects */

	s16b word_recall;	/* Word of recall counter */

	s16b energy;		/* Current energy */

	s16b food;			/* Current nutrition */

	byte confusing;		/* Glowing hands */
	byte searching;		/* Currently searching */

	byte spell_flags[PY_MAX_SPELLS]; /* Spell flags */

	byte spell_order[PY_MAX_SPELLS];	/* Spell order */

	s16b player_hp[PY_MAX_LEVEL];	/* HP Array */

	char died_from[80];		/* Cause of death */
	char history[250];	/* Initial history */

	u16b total_winner;		/* Total winner */
	u16b panic_save;		/* Panic save */

	u16b noscore;			/* Cheating flags */

	bool is_dead;			/* Player is dead */

	bool wizard;			/* Player is in wizard mode */


	/*** Temporary fields ***/

	bool playing;			/* True if player is playing */

	bool leaving;			/* True if player is leaving */

	bool create_up_stair;	/* Create up stair on next level */
	bool create_down_stair;	/* Create down stair on next level */

	s32b total_weight;		/* Total weight being carried */

	s16b inven_cnt;			/* Number of items in inventory */
	s16b equip_cnt;			/* Number of items in equipment */

	s16b health_who;		/* Health bar trackee */

	s16b monster_race_idx;	/* Monster race trackee */

	s16b object_kind_idx;	/* Object kind trackee */

	s16b energy_use;		/* Energy use this turn */

	s16b resting;			/* Resting counter */
	s16b running;			/* Running counter */
	bool running_withpathfind;      /* Are we using the pathfinder ? */

	s16b run_cur_dir;		/* Direction we are running */
	s16b run_old_dir;		/* Direction we came from */
	bool run_unused;		/* Unused (padding field) */
	bool run_open_area;		/* Looking for an open area */
	bool run_break_right;	/* Looking for a break (right) */
	bool run_break_left;	/* Looking for a break (left) */

	s16b command_cmd;		/* Gives identity of current command */
	s16b command_arg;		/* Gives argument of current command */
	s16b command_rep;		/* Gives repetition of current command */
	s16b command_dir;		/* Gives direction of current command */
	int  command_inv;		/* Gives item of current command */
	ui_event_data command_cmd_ex; /* Gives additional information of current command */

	s16b command_wrk;		/* See "cmd1.c" */
	s16b command_new;		/* Hack -- command chaining XXX XXX */

	s16b new_spells;		/* Number of spells available */

	bool cumber_armor;	/* Mana draining armor */
	bool cumber_glove;	/* Mana draining gloves */
	bool heavy_wield;	/* Heavy weapon */
	bool heavy_shoot;	/* Heavy shooter */
	bool icky_wield;	/* Icky weapon */

	s16b cur_lite;		/* Radius of lite (if any) */

	u32b notice;		/* Special Updates (bit flags) */
	u32b update;		/* Pending Updates (bit flags) */
	u32b redraw;		/* Normal Redraws (bit flags) */

	s16b stat_use[A_MAX];	/* Current modified stats */
	s16b stat_top[A_MAX];	/* Maximal modified stats */

	/*** Extracted fields ***/

	s16b stat_add[A_MAX];	/* Equipment stat bonuses */
	s16b stat_ind[A_MAX];	/* Indexes into stat tables */

	bool immune_acid;	/* Immunity to acid */
	bool immune_elec;	/* Immunity to lightning */
	bool immune_fire;	/* Immunity to fire */
	bool immune_cold;	/* Immunity to cold */

	bool resist_acid;	/* Resist acid */
	bool resist_elec;	/* Resist lightning */
	bool resist_fire;	/* Resist fire */
	bool resist_cold;	/* Resist cold */
	bool resist_pois;	/* Resist poison */

	bool resist_fear;	/* Resist fear */
	bool resist_lite;	/* Resist light */
	bool resist_dark;	/* Resist darkness */
	bool resist_blind;	/* Resist blindness */
	bool resist_confu;	/* Resist confusion */
	bool resist_sound;	/* Resist sound */
	bool resist_shard;	/* Resist shards */
	bool resist_nexus;	/* Resist nexus */
	bool resist_nethr;	/* Resist nether */
	bool resist_chaos;	/* Resist chaos */
	bool resist_disen;	/* Resist disenchant */

	bool sustain_str;	/* Keep strength */
	bool sustain_int;	/* Keep intelligence */
	bool sustain_wis;	/* Keep wisdom */
	bool sustain_dex;	/* Keep dexterity */
	bool sustain_con;	/* Keep constitution */
	bool sustain_chr;	/* Keep charisma */

	bool slow_digest;	/* Slower digestion */
	bool ffall;			/* Feather falling */
	bool regenerate;	/* Regeneration */
	bool telepathy;		/* Telepathy */
	bool see_inv;		/* See invisible */
	bool free_act;		/* Free action */
	bool hold_life;		/* Hold life */

	bool impact;		/* Earthquake blows */
	bool aggravate;		/* Aggravate monsters */
	bool teleport;		/* Random teleporting */
	bool exp_drain;		/* Experience draining */

	bool bless_blade;	/* Blessed blade */

	s16b dis_to_h;		/* Known bonus to hit */
	s16b dis_to_d;		/* Known bonus to dam */
	s16b dis_to_a;		/* Known bonus to ac */

	s16b dis_ac;		/* Known base ac */

	s16b to_h;			/* Bonus to hit */
	s16b to_d;			/* Bonus to dam */
	s16b to_a;			/* Bonus to ac */

	s16b ac;			/* Base ac */

	s16b see_infra;		/* Infravision range */

	s16b skills[SKILL_MAX];	/* Skills */

	u32b noise;			/* Derived from stealth */

	s16b num_blow;		/* Number of blows */
	s16b num_fire;		/* Number of shots */

	byte ammo_mult;		/* Ammo multiplier */

	byte ammo_tval;		/* Ammo variety */

	s16b pspeed;		/* Current speed */

	/* Generation fields (for quick start) */
	s32b au_birth;          /* Birth gold */
	s16b stat_birth[A_MAX]; /* Birth "natural" stat values */
	s16b ht_birth;          /* Birth Height */
	s16b wt_birth;          /* Birth Weight */
};


typedef struct flavor_type flavor_type;

struct flavor_type
{
	u32b text;      /* Text (offset) */

	byte tval;      /* Associated object type */
	byte sval;      /* Associated object sub-type */

	byte d_attr;    /* Default flavor attribute */
	char d_char;    /* Default flavor character */

	byte x_attr;    /* Desired flavor attribute */
	char x_char;    /* Desired flavor character */
};


/**
 * Defines a (value, name) pairing.  Variable names used are historical.
 */
typedef struct
{
	byte tval;
	const char *name;
} grouper;


/* Information for object auto-inscribe */
struct autoinscription
{
	s16b kind_idx;
	s16b inscription_idx;
};


struct history_info
{
	u16b type;			/* Kind of history item */
	s16b dlev;			/* Dungeon level when this item was recorded */
	s16b clev;			/* Character level when this item was recorded */
	byte a_idx;			/* Artifact this item relates to */
	s32b turn;			/* Turn this item was recorded on */
	char event[80];	/* The text of the item */
};


enum grid_light_level
{
	LIGHT_TORCH,
	LIGHT_GLOW,
	LIGHT_DARK
};

typedef struct
{
	u32b m_idx;		/* Monster index */
	u32b f_idx;		/* Feature index */
	u32b first_k_idx;	/* The "Kind" of the first item on the grid */
	bool multiple_objects;	/* Is there more than one item there? */

	enum grid_light_level lighting; /* Light level */
	bool in_view; /* TRUE when the player can currently see the grid. */
	bool is_player;
	bool hallucinate;
	bool trapborder;
} grid_data;


#endif /* !INCLUDED_TYPES_H */
