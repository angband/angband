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

#include "z-term.h"

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

typedef struct alloc_entry alloc_entry;
typedef struct quest quest;
typedef struct spell spell_type;
typedef struct history_info history_info;
typedef struct color_type color_type;


/**
 * Information about maximal indices of certain arrays.
 *
 * These are actually not the maxima, but the maxima plus one, because of
 * 0-based indexing issues.
 */
typedef struct maxima
{
	u16b f_max;       /**< Maximum number of terrain features */
	u16b k_max;       /**< Maximum number of object base kinds */
	u16b a_max;       /**< Maximum number of artifact kinds */
	u16b e_max;       /**< Maximum number of ego-item kinds */
	u16b r_max;       /**< Maximum number of monster races */
	u16b mp_max;	  /**< Maximum number of monster pain message sets */
	u16b s_max;       /**< Maximum number of magic spells */
	u16b pit_max;	  /**< Maximum number of monster pit types */

	u16b o_max;       /**< Maximum number of objects on a given level */
	u16b m_max;       /**< Maximum number of monsters on a given level */
} maxima;


/**
 * Information about terrain features.
 *
 * At the moment this isn't very much, but eventually a primitive flag-based
 * information system will be used here.
 */
typedef struct feature
{
	char *name;
	int fidx;

	struct feature *next;

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

	byte x_attr[3];   /**< Desired feature attribute (set by user/pref file) */
	char x_char[3];   /**< Desired feature character (set by user/pref file) */
} feature_type;



/*
 * Information about "vault generation"
 */
typedef struct vault {
	struct vault *next;
	unsigned int vidx;
	char *name;
	char *text;

	byte typ;			/* Vault type */

	byte rat;			/* Vault rating */

	byte hgt;			/* Vault height */
	byte wid;			/* Vault width */
} vault_type;


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
 * And here's the structure for the "fixed" spell information
 */
struct spell {
	struct spell *next;
	unsigned int sidx;
	char *name;
	char *text;

	byte realm;			/* 0 = mage; 1 = priest */
	byte tval;			/* Item type for book this spell is in */
	byte sval;			/* Item sub-type for book (= book number) */
	byte snum;			/* Position of spell within book */

	byte spell_index;	/* Index into player_magic array */
};


/**
 * Defines a (value, name) pairing.  Variable names used are historical.
 */
typedef struct
{
	byte tval;
	const char *name;
} grouper;

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
	FEAT_LIGHTING_BRIGHT = 0,
	FEAT_LIGHTING_LIT,
	FEAT_LIGHTING_DARK,
	FEAT_LIGHTING_MAX
};

typedef struct
{
	u32b m_idx;		/* Monster index */
	u32b f_idx;		/* Feature index */
	struct object_kind *first_kind;	/* The "kind" of the first item on the grid */
	bool multiple_objects;	/* Is there more than one item there? */

	enum grid_light_level lighting; /* Light level */
	bool in_view; /* TRUE when the player can currently see the grid. */
	bool is_player;
	bool hallucinate;
	bool trapborder;
} grid_data;


/*
 * A game color.
 */
struct color_type
{
	char index_char;            /* Character index:  'r' = red, etc. */
	char name[32];              /* Color name */
	byte color_translate[MAX_ATTR];       /* Index for various in-game translations */
};

/*
 * A hint.
 */
struct hint
{
        char *hint;
        struct hint *next;
};


#endif /* !INCLUDED_TYPES_H */
