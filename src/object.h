#ifndef INCLUDED_OBJECT_H
#define INCLUDED_OBJECT_H

#include "z-rand.h"
#include "z-quark.h"
#include "z-bitflag.h"
#include "z-dice.h"
#include "obj-flag.h"



/*** Game constants ***/

/*
 * Spell types used by project(), needed for object resistances.
 */
enum
{
    #define GF(a, b, c, d, e, obv, col, f, g, h, i, j, k, l, m, fh, oh, mh, ph) GF_COUNT_##a,
    #include "list-gf-types.h"
    #undef GF
};

/*
 * Refueling constants
 */
#define FUEL_TORCH                5000  /* Maximum amount of fuel in a torch */
#define FUEL_LAMP                15000  /* Maximum amount of fuel in a lantern */
#define DEFAULT_TORCH       FUEL_TORCH  /* Default amount of fuel in a torch */
#define DEFAULT_LAMP   (FUEL_LAMP / 2)  /* Default amount of fuel in a lantern */

/* A "stack" of items is limited to 40 items (hard-coded). */
#define MAX_STACK_SIZE 41

/* An item's pval (for charges, amount of gold, etc) is limited to s16b */
#define MAX_PVAL  32767

/*
 * Maximum number of objects allowed in a single dungeon grid.
 *
 * The main-screen has a minimum size of 24 rows, so we can always
 * display 23 objects + 1 header line.
 */
#define MAX_FLOOR_STACK			23


/*** API constants ***/

/* Object origin kinds */

enum {
    ORIGIN_NONE = 0,
    ORIGIN_FLOOR,			/* found on the dungeon floor */
    ORIGIN_DROP,			/* normal monster drops */
    ORIGIN_CHEST,
	ORIGIN_DROP_SPECIAL,	/* from monsters in special rooms */
	ORIGIN_DROP_PIT,		/* from monsters in pits/nests */
	ORIGIN_DROP_VAULT,		/* from monsters in vaults */
	ORIGIN_SPECIAL,			/* on the floor of a special room */
	ORIGIN_PIT,				/* on the floor of a pit/nest */
	ORIGIN_VAULT,			/* on the floor of a vault */
	ORIGIN_LABYRINTH,		/* on the floor of a labyrinth */
	ORIGIN_CAVERN,			/* on the floor of a cavern */
	ORIGIN_RUBBLE,			/* found under rubble */
    ORIGIN_MIXED,			/* stack with mixed origins */
	ORIGIN_STATS,			/* ^ only the above are considered by main-stats */
    ORIGIN_ACQUIRE,			/* called forth by scroll */
	ORIGIN_DROP_BREED,		/* from breeders */
	ORIGIN_DROP_SUMMON,		/* from combat summons */
    ORIGIN_STORE,			/* something you bought */
	ORIGIN_STOLEN,			/* stolen by monster (used only for gold) */
    ORIGIN_BIRTH,			/* objects created at character birth */
    ORIGIN_DROP_UNKNOWN,	/* drops from unseen foes */
    ORIGIN_CHEAT,			/* created by wizard mode */
	ORIGIN_DROP_POLY,		/* from polymorphees */
	ORIGIN_DROP_WIZARD,		/* from wizard mode summons */

	ORIGIN_MAX
};

#define ORIGIN_SIZE FLAG_SIZE(ORIGIN_MAX)
#define ORIGIN_BYTES 4 /* savefile bytes - room for 32 origin types */


/**
 * Maximum number of pvals on objects
 *
 * Note: all pvals other than DEFAULT_PVAL are assumed to be associated with
 * flags, and any non-flag uses of pval (e.g. chest quality, gold quantity)
 * are assumed to use DEFAULT_PVAL.
 */
#define MAX_PVALS 3
#define DEFAULT_PVAL 0

/* ID flags */
#define IDENT_SENSE     0x0001  /* Has been "sensed" */
#define IDENT_WORN      0x0002  /* Has been tried on */
#define IDENT_EMPTY     0x0004  /* Is known to be empty */
#define IDENT_KNOWN     0x0008  /* Fully known */
#define IDENT_STORE     0x0010  /* Item is in the inventory of a store */
#define IDENT_ATTACK    0x0020  /* Know combat dice/ac/bonuses */
#define IDENT_DEFENCE   0x0040  /* Know AC/etc bonuses */
#define IDENT_EFFECT    0x0080  /* Know item activation/effect */
/* xxx */
#define IDENT_INDESTRUCT 0x0200 /* Tried to destroy it and failed */
#define IDENT_NAME      0x0400  /* Know the name of ego or artifact if there is one */
#define IDENT_FIRED     0x0800  /* Has been used as a missile */
#define IDENT_NOTART    0x1000  /* Item is known not to be an artifact */
#define IDENT_FAKE      0x2000  /* Item is a fake, for displaying knowledge */
#define IDENT_SENSED_THIS_TURN 0x4000 /* Item has had a chance to be sensed on this turn (see sense_inventory()) */

/* Whether to learn egos and flavors with less than complete information */
#define EASY_LEARN 1

/* Maximum number of scroll titles generated */
#define MAX_TITLES     50

/**
 * Modes for object_desc().
 */
enum {
	ODESC_BASE   = 0x00,   /*!< Only describe the base name */
	ODESC_COMBAT = 0x01,   /*!< Also show combat bonuses */
	ODESC_EXTRA  = 0x02,   /*!< Show charges/inscriptions/pvals */

	ODESC_FULL   = ODESC_COMBAT | ODESC_EXTRA,
	                       /*!< Show entire description */

	ODESC_STORE  = 0x04,   /*!< This is an in-store description */
	ODESC_PLURAL = 0x08,   /*!< Always pluralise */
	ODESC_SINGULAR    = 0x10,    /*!< Always singular */
	ODESC_SPOIL  = 0x20,    /*!< Display regardless of player knowledge */
	ODESC_PREFIX = 0x40,   /* */

	ODESC_CAPITAL = 0x80,	/*!< Capitalise object name */
	ODESC_TERSE = 0x100  	/*!< Make terse names */
};


/**
 * Modes for item lists in "show_inven()"  "show_equip()" and "show_floor()"
 */
typedef enum {
	OLIST_NONE   = 0x00,   /* No options */
   	OLIST_WINDOW = 0x01,   /* Display list in a sub-term (left-align) */
   	OLIST_QUIVER = 0x02,   /* Display quiver lines */
   	OLIST_GOLD   = 0x04,   /* Include gold in the list */
	OLIST_WEIGHT = 0x08,   /* Show item weight */
	OLIST_PRICE  = 0x10,   /* Show item price */
	OLIST_FAIL   = 0x20,    /* Show device failure */
	OLIST_SEMPTY = 0x40
} olist_detail_t;


/**
 * Modes for object_info()
 */
typedef enum {
	OINFO_NONE   = 0x00, /* No options */
	OINFO_TERSE  = 0x01, /* Keep descriptions brief, e.g. for dumps */
	OINFO_SUBJ   = 0x02, /* Describe object from the character's POV */
} oinfo_detail_t;


/**
 * Modes for stacking by object_similar()
 */
typedef enum
{
	OSTACK_NONE    = 0x00, /* No options (this does NOT mean no stacking) */
	OSTACK_STORE   = 0x01, /* Store stacking */
	OSTACK_PACK    = 0x02, /* Inventory and home */
	OSTACK_LIST    = 0x04, /* Object list */
	OSTACK_MONSTER = 0x08, /* Monster carrying objects */
	OSTACK_FLOOR   = 0x10, /* Floor stacking */
	OSTACK_QUIVER  = 0x20  /* Quiver */
} object_stack_t;


/**
 * Pseudo-ID markers.
 */
typedef enum
{
	INSCRIP_NULL = 0,            /*!< No pseudo-ID status */
	INSCRIP_STRANGE = 1,         /*!< Item that has mixed combat bonuses */
	INSCRIP_AVERAGE = 2,         /*!< Item with no interesting features */
	INSCRIP_MAGICAL = 3,         /*!< Item with combat bonuses */
	INSCRIP_SPLENDID = 4,        /*!< Obviously good item */
	INSCRIP_EXCELLENT = 5,       /*!< Ego-item */
	INSCRIP_SPECIAL = 6,         /*!< Artifact */
	INSCRIP_UNKNOWN = 7,

	INSCRIP_MAX                  /*!< Maximum number of pseudo-ID markers */
} obj_pseudo_t;

/*
 * Chest check types
 */
enum chest_query {
	CHEST_ANY,
	CHEST_OPENABLE,
	CHEST_TRAPPED
};

/*
 * Bit flags for get_item() function
 */
#define USE_EQUIP     0x0001	/* Allow equip items */
#define USE_INVEN     0x0002	/* Allow inven items */
#define USE_FLOOR     0x0004	/* Allow floor items */
#define IS_HARMLESS   0x0008	/* Ignore generic warning inscriptions */
#define SHOW_PRICES   0x0010	/* Show item prices in item lists */
#define SHOW_FAIL     0x0020 	/* Show device failure in item lists */
#define SHOW_QUIVER   0x0040	/* Show quiver summary when looking at inventory */
#define SHOW_EMPTY    0x0080	/* Show empty slots in equipment display */
#define QUIVER_TAGS   0x0100	/* 0-9 are quiver slots when selecting */


/*
 * Some constants used in randart generation and power calculation
 * - thresholds for limiting to_hit, to_dam and to_ac
 * - fudge factor for rescaling ammo cost
 * (a stack of this many equals a weapon of the same damage output)
 */
#define INHIBIT_POWER       20000
#define INHIBIT_BLOWS           3
#define INHIBIT_MIGHT           4
#define INHIBIT_SHOTS           3
#define HIGH_TO_AC             26
#define VERYHIGH_TO_AC         36
#define INHIBIT_AC             56
#define HIGH_TO_HIT            16
#define VERYHIGH_TO_HIT        26
#define HIGH_TO_DAM            16
#define VERYHIGH_TO_DAM        26
#define AMMO_RESCALER          20 /* this value is also used for torches */

#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/* Values for struct object->marked */
enum {
	MARK_UNAWARE = 0,
	MARK_AWARE = 1,
	MARK_SEEN = 2
};



/*** Macros ***/

/*** Structures ***/

/*
 * And here's the structure for the "fixed" spell information
 */
typedef struct spell {
	struct spell *next;
	unsigned int sidx;
	char *name;
	char *text;

	byte realm;			/* 0 = mage; 1 = priest */
	byte tval;			/* Item type for book this spell is in */
	byte sval;			/* Item sub-type for book (= book number) */
	byte snum;			/* Position of spell within book */

	byte spell_index;	/* Index into player_magic array */
	dice_t *dice;		/* Value information from spell file */
	int params[3];		/* Extra parameters to be passed to the handler */
} spell_type;

/*
 * The spell arrays
 */
extern spell_type *s_info;


enum spell_param_project_type_e {
	SPELL_PROJECT_NONE = 0,
	SPELL_PROJECT_BOLT,
	SPELL_PROJECT_BEAM,
	SPELL_PROJECT_BOLT_OR_BEAM,
	SPELL_PROJECT_BALL,
};


/* Brand or slay type */
typedef struct brand_or_slay
{
	char *name;
	int multiplier;
	struct brand_or_slay *next;
} brand_or_slay;

/**
 * Information about object types, like rods, wands, etc.
 */
typedef struct object_base
{
	char *name;

	int tval;
	struct object_base *next;

	int attr;

	bitflag flags[OF_SIZE];
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */

	int break_perc;
} object_base;

extern object_base *kb_info;

/**
 * Information about object kinds, including player knowledge.
 *
 * TODO: split out the user-changeable bits into a separate struct so this
 * one can be read-only.
 */
typedef struct object_kind
{
	char *name;
	char *text;

	object_base *base;

	struct object_kind *next;
	u32b kidx;

	byte tval;         /**< General object type (see TV_ macros) */
	byte sval;         /**< Object sub-type (see SV_ macros) */
	random_value pval[MAX_PVALS]; /**< Power for any flags which need it */
	byte num_pvals;	   /**< Number of pvals in use on this item */

	random_value to_h; /**< Bonus to hit */
	random_value to_d; /**< Bonus to damage */
	random_value to_a; /**< Bonus to armor */
	s16b ac;           /**< Base armor */

	byte dd;           /**< Damage dice */
	byte ds;           /**< Damage sides */
	s16b weight;       /**< Weight, in 1/10lbs */

	s32b cost;         /**< Object base cost */

	bitflag flags[OF_SIZE];			/**< Flags */
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */

	random_value modifiers[OBJ_MOD_MAX];
	s16b resists[GF_COUNT_MAX];

	brand_or_slay brands;
	brand_or_slay slays;

	byte d_attr;       /**< Default object attribute */
	wchar_t d_char;       /**< Default object character */

	int alloc_prob;   /**< Allocation: commonness */
	byte alloc_min;    /**< Highest normal dungeon level */
	byte alloc_max;    /**< Lowest normal dungeon level */
	byte level;        /**< Level (difficulty of activation) */

	u16b effect;         /**< Effect this item produces (effects.c) */
	random_value time;   /**< Recharge time (rods/activation) */
	random_value charge; /**< Number of charges (staves/wands) */

	byte gen_mult_prob;      /**< Probability of generating more than one */
	random_value stack_size; /**< Number to generate */

	struct flavor *flavor;         /**< Special object flavor (or zero) */


	/** Game-dependent **/

	byte x_attr;   /**< Desired object attribute (set by user/pref file) */
	wchar_t x_char;   /**< Desired object character (set by user/pref file) */

	/** Also saved in savefile **/

	quark_t note; /**< Autoinscription quark number */

	bool aware;    /**< Set if player is aware of the kind's effects */
	bool tried;    /**< Set if kind has been tried */

	byte squelch;  /**< Squelch settings */
	bool everseen; /**< Set if kind has ever been seen (to despoilify squelch menus) */

	struct spell *spells;
} object_kind;

extern object_kind *k_info;

/*** Important artifact indexes (see "lib/edit/artifact.txt") ***/

#define ART_POWER			13
#define ART_MORGOTH			34
#define ART_GROND			111

/*
 * Hack -- first "normal" artifact in the artifact list.  All of
 * the artifacts with indexes from 1 to 15 are "special" (lights,
 * rings, amulets), and the ones from 16 to 127 are "normal".
 */
#define ART_MIN_NORMAL		16

/**
 * Information about artifacts.
 *
 * Note that ::cur_num is written to the savefile.
 *
 * TODO: Fix this max_num/cur_num crap and just have a big boolean array of
 * which artifacts have been created and haven't, so this can become read-only.
 */
typedef struct artifact
{
	char *name;
	char *text;

	u32b aidx;

	struct artifact *next;

	byte tval;    /**< General artifact type (see TV_ macros) */
	byte sval;    /**< Artifact sub-type (see SV_ macros) */
	s16b pval[MAX_PVALS];    /**< Power for any flags which need it */
	byte num_pvals;/**< Number of pvals in use on this item */

	s16b to_h;    /**< Bonus to hit */
	s16b to_d;    /**< Bonus to damage */
	s16b to_a;    /**< Bonus to armor */
	s16b ac;      /**< Base armor */

	byte dd;      /**< Base damage dice */
	byte ds;      /**< Base damage sides */

	s16b weight;  /**< Weight in 1/10lbs */

	s32b cost;    /**< Artifact (pseudo-)worth */

	bitflag flags[OF_SIZE];		/**< Flags */
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */

	s16b modifiers[OBJ_MOD_MAX];
	s16b resists[GF_COUNT_MAX];

	brand_or_slay brands;
	brand_or_slay slays;

	byte level;   /** Difficulty level for activation */

	int alloc_prob; /** Chance of being generated (i.e. rarity) */
	byte alloc_min;  /** Minimum depth (can appear earlier) */
	byte alloc_max;  /** Maximum depth (will NEVER appear deeper) */

	bool created;	/**< Whether this artifact has been created */
	bool seen;	/**< Whether this artifact has been seen this game */
	bool everseen;	/**< Whether this artifact has ever been seen  */

	u16b effect;     /**< Artifact activation (see effects.c) */
	char *effect_msg;

	random_value time;  /**< Recharge time (if appropriate) */
} artifact_type;

/*
 * The artifact arrays
 */
extern artifact_type *a_info;


/*
 * Number of tval/min-sval/max-sval slots per ego item
 */
#define EGO_TVALS_MAX 3

/*
 * Information about "ego-items".
 */
typedef struct ego_item
{
	struct ego_item *next;

	char *name;
	char *text;

	u32b eidx;

	s32b cost;			/* Ego-item "cost" */

	bitflag flags[OF_SIZE];		/**< Flags */
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */

	random_value modifiers[OBJ_MOD_MAX];
	byte min_modifiers[OBJ_MOD_MAX];
	s16b resists[GF_COUNT_MAX];

	brand_or_slay brands;
	brand_or_slay slays;

	byte level;		/* Minimum level */
	byte rarity;		/* Object rarity */
	byte rating;		/* Level rating boost */
	int alloc_prob; 	/** Chance of being generated (i.e. rarity) */
	byte alloc_min;  	/** Minimum depth (can appear earlier) */
	byte alloc_max;  	/** Maximum depth (will NEVER appear deeper) */

	byte tval[EGO_TVALS_MAX]; 	/* Legal tval */
	byte min_sval[EGO_TVALS_MAX];	/* Minimum legal sval */
	byte max_sval[EGO_TVALS_MAX];	/* Maximum legal sval */

	random_value to_h;     		/* Extra to-hit bonus */
	random_value to_d; 		/* Extra to-dam bonus */
	random_value to_a; 		/* Extra to-ac bonus */
	random_value pval[MAX_PVALS]; 	/* Extra pval bonus */
	byte num_pvals;			/* Number of pvals used */

	byte min_to_h;			/* Minimum to-hit value */
	byte min_to_d;			/* Minimum to-dam value */
	byte min_to_a;			/* Minimum to-ac value */
	byte min_pval[MAX_PVALS];	/* Minimum pval */

	bool everseen;			/* Do not spoil squelch menus */
} ego_item_type;

/*
 * The ego-item arrays
 */
extern ego_item_type *e_info;


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
typedef struct object
{
	struct object_kind *kind;
	struct ego_item *ego;
	struct artifact *artifact;

	byte iy;			/* Y-position on map, or zero */
	byte ix;			/* X-position on map, or zero */

	byte tval;			/* Item type (from kind) */
	byte sval;			/* Item sub-type (from kind) */

	s16b pval[MAX_PVALS];		/* Item extra-parameter */
	byte num_pvals;			/* Number of pvals in use */

	s16b weight;			/* Item weight */

	bitflag flags[OF_SIZE];		/**< Flags */
	bitflag known_flags[OF_SIZE];	/**< Player-known flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */
	u16b ident;			/* Special flags */

	s16b modifiers[OBJ_MOD_MAX];
	s16b resists[GF_COUNT_MAX];

	brand_or_slay brands;
	brand_or_slay slays;

	s16b ac;			/* Normal AC */
	s16b to_a;			/* Plusses to AC */
	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */

	byte dd, ds;		/* Damage dice/sides */

	s16b timeout;		/* Timeout Counter */

	byte number;		/* Number of items */
	byte marked;		/* Object is marked */
	byte ignore;		/* Object is ignored */

	s16b next_o_idx;	/* Next object in stack (if any) */
	s16b held_m_idx;	/* Monster holding us (if any) */
	s16b mimicking_m_idx; /* Monster mimicking us (if any) */

	byte origin;        /* How this item was found */
	byte origin_depth;  /* What depth the item was found at */
	u16b origin_xtra;   /* Extra information about origin */

	quark_t note; /* Inscription index */
} object_type;

struct flavor
{
	char *text;
	struct flavor *next;
	unsigned int fidx;

	byte tval;      /* Associated object type */
	byte sval;      /* Associated object sub-type */

	byte d_attr;    /* Default flavor attribute */
	wchar_t d_char;    /* Default flavor character */

	byte x_attr;    /* Desired flavor attribute */
	wchar_t x_char;    /* Desired flavor character */
};

extern struct flavor *flavors;


typedef bool (*item_tester)(const struct object *);


/*** Functions ***/

/* obj-power.c and randart.c */
s32b object_power(const object_type *o_ptr, int verbose, ang_file *log_file, bool known);
char *artifact_gen_name(struct artifact *a, const char ***wordlist);
errr do_randart(u32b randart_seed, bool full);

#endif /* !INCLUDED_OBJECT_H */
