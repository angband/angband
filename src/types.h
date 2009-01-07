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

typedef struct vault_type vault_type;
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
	u32b name;     /**< (const char *) feature_type::name + f_name = Name */
	u32b text;     /**< (const char *) feature_type::text + f_text = Description (unused) */

	byte mimic;    /**< Feature to mimic */
	byte priority; /**< Display priority */

	byte locked;   /**< How locked is it? */
	byte jammed;   /**< How jammed is it? */
	byte shopnum;  /**< Which shop does it take you to? */
	byte dig;      /**< How hard is it to dig through? */

	u32b effect;   /**< Effect on entry to grid */
	u32b flags;    /**< Terrain flags */

	byte d_attr;   /**< Default feature attribute */
	char d_char;   /**< Default feature character */

	byte x_attr;   /**< Desired feature attribute (set by user/pref file) */
	char x_char;   /**< Desired feature character (set by user/pref file) */
} feature_type;



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
