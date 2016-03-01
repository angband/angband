/**
 * \file object.h
 * \brief basic object structs and enums
 */
#ifndef INCLUDED_OBJECT_H
#define INCLUDED_OBJECT_H

#include "z-rand.h"
#include "z-quark.h"
#include "z-bitflag.h"
#include "z-dice.h"
#include "obj-properties.h"


/*** Game constants ***/

/**
 * Elements
 */
enum
{
	#define ELEM(a, b, c, d, e, f, g, h, i, col) ELEM_##a,
	#include "list-elements.h"
	#undef ELEM

	ELEM_MAX
};

#define ELEM_BASE_MIN  ELEM_ACID
#define ELEM_HIGH_MIN  ELEM_POIS
#define ELEM_HIGH_MAX  ELEM_DISEN

/**
 * Object origin kinds
 */

enum {
	#define ORIGIN(a, b, c) ORIGIN_##a,
	#include "list-origins.h"
	#undef ORIGIN

	ORIGIN_MAX
};


/*** Structures ***/

/* Effect */
struct effect {
	struct effect *next;
	u16b index;		/**< The effect index */
	dice_t *dice;	/**< Dice expression used in the effect */
	int params[3];	/**< Extra parameters to be passed to the handler */
};

/* Brand type */
struct brand {
	char *name;
	int element;
	int multiplier;
	int damage; /* Storage for damage during description */
	struct brand *next;
};

extern struct brand *game_brands;

/* Slay type */
struct slay {
	char *name;
	int race_flag;
	int multiplier;
	int damage; /* Storage for damage during description */
	struct slay *next;
};

extern struct slay *game_slays;

enum {
	EL_INFO_HATES = 0x01,
	EL_INFO_IGNORE = 0x02,
	EL_INFO_RANDOM = 0x04,
};

/* Element info type */
struct element_info {
	s16b res_level;
	bitflag flags;
};

/**
 * Activation structure
 */
struct activation {
	struct activation *next;
	char *name;
	int index;
	bool aim;
	int power;
	struct effect *effect;
	char *message;
	char *desc;
};

extern struct activation *activations;

/**
 * Information about object types, like rods, wands, etc.
 */
struct object_base {
	char *name;

	int tval;
	struct object_base *next;

	int attr;

	bitflag flags[OF_SIZE];
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */
	struct element_info el_info[ELEM_MAX];

	int break_perc;
	int num_svals;
};

extern struct object_base *kb_info;

/**
 * Information about object kinds, including player knowledge.
 *
 * TODO: split out the user-changeable bits into a separate struct so this
 * one can be read-only.
 */
struct object_kind {
	char *name;
	char *text;

	struct object_base *base;

	struct object_kind *next;
	u32b kidx;

	int tval;					/**< General object type (see TV_ macros) */
	int sval;					/**< Object sub-type  */

	random_value pval;			/* Item extra-parameter */

	random_value to_h;			/**< Bonus to hit */
	random_value to_d;			/**< Bonus to damage */
	random_value to_a;			/**< Bonus to armor */
	int ac;					/**< Base armor */

	int dd;					/**< Damage dice */
	int ds;					/**< Damage sides */
	int weight;				/**< Weight, in 1/10lbs */

	int cost;					/**< Object base cost */

	bitflag flags[OF_SIZE];					/**< Flags */
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */

	random_value modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	byte d_attr;			/**< Default object attribute */
	wchar_t d_char;			/**< Default object character */

	int alloc_prob;			/**< Allocation: commonness */
	int alloc_min;			/**< Highest normal dungeon level */
	int alloc_max;			/**< Lowest normal dungeon level */
	int level;				/**< Level (difficulty of activation) */

	struct effect *effect;	/**< Effect this item produces (effects.c) */
	int power;				/**< Power of the item's effect */
	char *effect_msg;
	random_value time;		/**< Recharge time (rods/activation) */
	random_value charge;	/**< Number of charges (staves/wands) */

	int gen_mult_prob;		/**< Probability of generating more than one */
	random_value stack_size;/**< Number to generate */

	struct flavor *flavor;	/**< Special object flavor (or zero) */

	/** Also saved in savefile **/

	quark_t note; 	/**< Autoinscription quark number */

	bool aware;		/**< Set if player is aware of the kind's effects */
	bool tried;		/**< Set if kind has been tried */

	byte ignore;  	/**< Ignore settings */
	bool everseen; 	/**< Kind has been seen (to despoilify ignore menus) */
};

extern struct object_kind *k_info;
extern struct object_kind *unknown_item_kind;
extern struct object_kind *unknown_gold_kind;
extern struct object_kind *pile_kind;

/**
 * Information about artifacts.
 *
 * Note that ::cur_num is written to the savefile.
 *
 * TODO: Fix this max_num/cur_num crap and just have a big boolean array of
 * which artifacts have been created and haven't, so this can become read-only.
 */
struct artifact {
	char *name;
	char *text;

	u32b aidx;

	struct artifact *next;

	int tval;		/**< General artifact type (see TV_ macros) */
	int sval;		/**< Artifact sub-type  */

	int to_h;		/**< Bonus to hit */
	int to_d;		/**< Bonus to damage */
	int to_a;		/**< Bonus to armor */
	int ac;		/**< Base armor */

	int dd;		/**< Base damage dice */
	int ds;		/**< Base damage sides */

	int weight;	/**< Weight in 1/10lbs */

	int cost;		/**< Artifact (pseudo-)worth */

	bitflag flags[OF_SIZE];			/**< Flags */

	int modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	int level;			/** Difficulty level for activation */

	int alloc_prob;		/** Chance of being generated (i.e. rarity) */
	int alloc_min;		/** Minimum depth (can appear earlier) */
	int alloc_max;		/** Maximum depth (will NEVER appear deeper) */

	bool created;		/**< Whether this artifact has been created */
	bool seen;			/**< Whether this artifact has been seen this game */
	bool everseen;		/**< Whether this artifact has ever been seen  */

	struct activation *activation;	/**< Artifact activation */
	char *alt_msg;

	random_value time;	/**< Recharge time (if appropriate) */
};

/**
 * The artifact arrays
 */
extern struct artifact *a_info;


/**
 * Structure for possible object kinds for an ego item
 */
struct ego_poss_item {
	u32b kidx;
	struct ego_poss_item *next;
};

/**
 * Information about ego-items.
 */
struct ego_item {
	struct ego_item *next;

	char *name;
	char *text;

	u32b eidx;

	int cost;						/* Ego-item "cost" */

	bitflag flags[OF_SIZE];			/**< Flags */
	bitflag flags_off[OF_SIZE];		/**< Flags to remove */
	bitflag kind_flags[KF_SIZE];	/**< Kind flags */

	random_value modifiers[OBJ_MOD_MAX];
	int min_modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	int level;				/* Minimum level */
	int rarity;			/* Object rarity */
	int rating;			/* Level rating boost */
	int alloc_prob; 		/** Chance of being generated (i.e. rarity) */
	int alloc_min;			/** Minimum depth (can appear earlier) */
	int alloc_max;			/** Maximum depth (will NEVER appear deeper) */

	struct ego_poss_item *poss_items;

	random_value to_h;		/* Extra to-hit bonus */
	random_value to_d;		/* Extra to-dam bonus */
	random_value to_a;		/* Extra to-ac bonus */

	int min_to_h;			/* Minimum to-hit value */
	int min_to_d;			/* Minimum to-dam value */
	int min_to_a;			/* Minimum to-ac value */

	struct effect *effect;	/**< Effect this item produces (effects.c) */
	char *effect_msg;
	random_value time;		/**< Recharge time (rods/activation) */

	bool everseen;			/* Do not spoil ignore menus */
};

/*
 * The ego-item arrays
 */
extern struct ego_item *e_info;

/**
 * Flags for the obj->notice field
 */
enum {
	OBJ_NOTICE_WORN = 0x01,
	OBJ_NOTICE_ASSESSED = 0x02,
	OBJ_NOTICE_IGNORE = 0x04,
	OBJ_NOTICE_IMAGINED = 0x08,
};

/*
 * Object information, for a specific object.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Each cave grid points to one (or zero) objects via the "obj" field in
 * its "squares" struct.  Each object then points to one (or zero) objects
 * via the "next" field, and (aside from the first) back via its "prev"
 * field, forming a doubly linked list, which in game terms represents a
 * stack of objects in the same grid.
 *
 * Each monster points to one (or zero) objects via the "held_obj"
 * field (see monster.h).  Each object then points to one (or zero) objects
 * and back to previous objects by its own "next" and "prev" fields,
 * forming a doubly linked list, which in game terms represents the
 * monster's inventory.
 *
 * The "held_m_idx" field is used to indicate which monster, if any,
 * is holding the object.  Objects being held have "ix = 0" and "iy = 0".
 *
 * Note that object records are not now copied, but allocated on object
 * creation and freed on object destruction.  These records are handed
 * around between player and monster inventories and the floor on a fairly
 * regular basis, and care must be taken when handling such objects.
 */
struct object {
	struct object_kind *kind;
	struct ego_item *ego;
	struct artifact *artifact;

	struct object *prev;	/* Previous object in a pile */
	struct object *next;	/* Next object in a pile */
	struct object *known;	/* Known version of this object */

	u16b oidx;			/* Item list index, if any */

	byte iy;			/* Y-position on map, or zero */
	byte ix;			/* X-position on map, or zero */

	byte tval;			/* Item type (from kind) */
	byte sval;			/* Item sub-type (from kind) */

	s16b pval;			/* Item extra-parameter */

	s16b weight;		/* Item weight */

	bitflag flags[OF_SIZE];			/**< Flags */

	s16b modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	s16b ac;			/* Normal AC */
	s16b to_a;			/* Plusses to AC */
	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */

	byte dd, ds;		/* Damage dice/sides */

	struct effect *effect;	/**< Effect this item produces (effects.c) */
	char *effect_msg;
	struct activation *activation;	/**< Artifact activation, if applicable */
	random_value time;	/**< Recharge time (rods/activation) */
	s16b timeout;		/* Timeout Counter */

	byte number;		/* Number of items */
	bitflag notice;		/* Attention paid to the object */

	s16b held_m_idx;	/* Monster holding us (if any) */
	s16b mimicking_m_idx; /* Monster mimicking us (if any) */

	byte origin;		/* How this item was found */
	byte origin_depth;  /* What depth the item was found at */
	u16b origin_xtra;   /* Extra information about origin */

	quark_t note; 		/* Inscription index */
};

struct flavor
{
	char *text;
	struct flavor *next;
	unsigned int fidx;

	byte tval;	  /* Associated object type */
	byte sval;	  /* Associated object sub-type */

	byte d_attr;	/* Default flavor attribute */
	wchar_t d_char;	/* Default flavor character */
};

extern struct flavor *flavors;


typedef bool (*item_tester)(const struct object *);


#endif /* !INCLUDED_OBJECT_H */
