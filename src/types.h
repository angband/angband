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
 * (like uint32_t and int32_t) and not just "int"s.
 */

#include "z-term.h"

/**** Available Types ****/

/** An array of 256 uint8_ts */
typedef uint8_t uint8_t_256[256];

/** An array of DUNGEON_WID uint8_ts */
typedef uint8_t uint8_t_wid[DUNGEON_WID];

/** An array of DUNGEON_WID int16_t's */
typedef int16_t int16_t_wid[DUNGEON_WID];



/** Function hook types **/

/** Function prototype for the UI to provide to create native buttons */
typedef int (*button_add_f)(const char *, keycode_t);

/** Function prototype for the UI to provide to remove native buttons */
typedef int (*button_kill_f)(keycode_t);



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
	uint16_t f_max;       /**< Maximum number of terrain features */
	uint16_t k_max;       /**< Maximum number of object base kinds */
	uint16_t a_max;       /**< Maximum number of artifact kinds */
	uint16_t e_max;       /**< Maximum number of ego-item kinds */
	uint16_t r_max;       /**< Maximum number of monster races */
	uint16_t mp_max;	  /**< Maximum number of monster pain message sets */
	uint16_t s_max;       /**< Maximum number of magic spells */
	uint16_t pit_max;	  /**< Maximum number of monster pit types */

	uint16_t o_max;       /**< Maximum number of objects on a given level */
	uint16_t m_max;       /**< Maximum number of monsters on a given level */
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

	uint8_t mimic;    /**< Feature to mimic */
	uint8_t priority; /**< Display priority */

	uint8_t locked;   /**< How locked is it? */
	uint8_t jammed;   /**< How jammed is it? */
	uint8_t shopnum;  /**< Which shop does it take you to? */
	uint8_t dig;      /**< How hard is it to dig through? */

	uint32_t effect;   /**< Effect on entry to grid */
	bitflag flags[FF_SIZE];    /**< Terrain flags */

	uint8_t d_attr;   /**< Default feature attribute */
	wchar_t d_char;   /**< Default feature character */

	uint8_t x_attr[3];   /**< Desired feature attribute (set by user/pref file) */
	wchar_t x_char[3];   /**< Desired feature character (set by user/pref file) */
} feature_type;



/*
 * Information about "vault generation"
 */
typedef struct vault {
	struct vault *next;
	unsigned int vidx;
	char *name;
	char *text;

	uint8_t typ;			/* Vault type */

	uint8_t rat;			/* Vault rating */

	uint8_t hgt;			/* Vault height */
	uint8_t wid;			/* Vault width */
} vault_type;


/*
 * Information about "room generation"
 */
typedef struct room_template {
	struct room_template *next;
	unsigned int tidx;
	char *name;
	char *text;

	uint8_t typ;			/* Room type */

	uint8_t rat;			/* Room rating */

	uint8_t hgt;			/* Room height */
	uint8_t wid;			/* Room width */
	uint8_t dor;           /* Random door options */
	uint8_t tval;			/* tval for objects in this room */
} room_template_type;


/*
 * An entry for the object/monster allocation functions
 *
 * Pass 1 is determined from allocation information
 * Pass 2 is determined from allocation restriction
 * Pass 3 is determined from allocation calculation
 */
struct alloc_entry
{
	int index;		/* The actual index */

	int level;		/* Base dungeon level */
	int prob1;		/* Probability, pass 1 */
	int prob2;		/* Probability, pass 2 */
	int prob3;		/* Probability, pass 3 */
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
	uint8_t level;		/* Dungeon level */
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

	uint8_t realm;			/* 0 = mage; 1 = priest */
	uint8_t tval;			/* Item type for book this spell is in */
	uint8_t sval;			/* Item sub-type for book (= book number) */
	uint8_t snum;			/* Position of spell within book */

	uint8_t spell_index;	/* Index into player_magic array */
};


/**
 * Defines a (value, name) pairing.  Variable names used are historical.
 */
typedef struct
{
	uint8_t tval;
	const char *name;
} grouper;

struct history_info
{
	uint16_t type;			/* Kind of history item */
	int16_t dlev;			/* Dungeon level when this item was recorded */
	int16_t clev;			/* Character level when this item was recorded */
	uint8_t a_idx;			/* Artifact this item relates to */
	int32_t turn;			/* Turn this item was recorded on */
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
	uint32_t m_idx;		/* Monster index */
	uint32_t f_idx;		/* Feature index */
	struct object_kind *first_kind;	/* The "kind" of the first item on the grid */
	bool multiple_objects;	/* Is there more than one item there? */
	bool unseen_object;	/* Is there an unaware object there? */
	bool unseen_money; /* Is there some unaware money there? */

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
	uint8_t color_translate[MAX_ATTR];       /* Index for various in-game translations */
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
