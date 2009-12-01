#ifndef INCLUDED_OBJECT_TYPES_H
#define INCLUDED_OBJECT_TYPES_H



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

	u32b flags[OBJ_FLAG_N];		/**< Flags */

	byte d_attr;   /**< Default object attribute */
	char d_char;   /**< Default object character */

	byte alloc_prob;   /**< Allocation: commonness */
	byte alloc_min;    /**< Highest normal dungeon level */
	byte alloc_max;    /**< Lowest normal dungeon level */
	byte level;        /**< Level */

	u16b effect;       /**< Effect this item produces (effects.c) */
	u16b time_base;    /**< Recharge time (if appropriate) */
	u16b time_dice;    /**< Randomised recharge time dice */
	u16b time_sides;   /**< Randomised recharge time sides */

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

	byte squelch;  /**< Squelch settings */
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

	u32b flags[OBJ_FLAG_N];		/**< Flags */

	byte level;   /** Unused */
	byte rarity;  /** Unused */
	byte alloc_prob; /** Chance of being generated (i.e. rarity) */
	byte alloc_min;  /** Minimum depth (can appear earlier) */
	byte alloc_max;  /** Maximum depth (will NEVER appear deeper) */

	bool created;	/**< Whether this artifact has been created */
	bool seen;	/**< Whether this artifact has been seen as an artifact */
	bool everseen;	/**< Whether this artifact has ever been seen (this game or previous) */

	u16b effect;     /**< Artifact activation (see effects.c) */
	u32b effect_msg; /**< (const char *) artifact_type::effect_msg + a_text = Effect message */

	u16b time_base;  /**< Recharge time (if appropriate) */
	u16b time_dice;  /**< Randomised recharge time dice */
	u16b time_sides; /**< Randomised recharge time sides */

} artifact_type;


/*
 * Information about "ego-items".
 */
typedef struct
{
	u32b name;			/* Name (offset) */
	u32b text;			/* Text (offset) */

	s32b cost;			/* Ego-item "cost" */

	u32b flags[OBJ_FLAG_N];		/**< Flags */

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

	byte min_to_h;		/* Minimum to-hit value */
	byte min_to_d;		/* Minimum to-dam value */
	byte min_to_a;		/* Minimum to-ac value */
	byte min_pval;		/* Minimum pval */

	byte xtra;			/* Extra sustain/resist/power */

	bool everseen;		/* Do not spoil squelch menus */
} ego_item_type;



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
typedef struct
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

	u32b flags[OBJ_FLAG_N];		/**< Flags */
	u32b known_flags[OBJ_FLAG_N];	/**< Player-known flags */
	u16b ident;			/* Special flags */

	s16b ac;			/* Normal AC */
	s16b to_a;			/* Plusses to AC */
	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */

	byte dd, ds;		/* Damage dice/sides */

	s16b timeout;		/* Timeout Counter */

	byte number;		/* Number of items */
	byte marked;		/* Object is marked */

	s16b next_o_idx;	/* Next object in stack (if any) */
	s16b held_m_idx;	/* Monster holding us (if any) */

	byte origin;        /* How this item was found */
	byte origin_depth;  /* What depth the item was found at */
	u16b origin_xtra;   /* Extra information about origin */

	quark_t note;			/* Inscription index */
} object_type;


/**
 * Flavour type. XXX
 */
typedef struct
{
	u32b text;      /* Text (offset) */

	byte tval;      /* Associated object type */
	byte sval;      /* Associated object sub-type */

	byte d_attr;    /* Default flavor attribute */
	char d_char;    /* Default flavor character */

	byte x_attr;    /* Desired flavor attribute */
	char x_char;    /* Desired flavor character */
} flavor_type;

/*
 * Slay type.  Used for the global table of brands/slays and their effects.
 */
typedef struct
{
	u32b slay_flag;		/* Object flag for the slay */
	u32b monster_flag;	/* Which monster flag(s) make it vulnerable */
	u32b resist_flag;	/* Which monster flag(s) make it resist */
	int mult;		/* Slay multiplier */
	const char *range_verb;	/* attack verb for ranged hits */
	const char *melee_verb; /* attack verb for melee hits */
	const char *active_verb; /* verb for when the object is active */
	const char *desc;	/* description of vulnerable creatures */
	const char *brand;	/* name of brand */
} slay_t;

/*
 * Slay cache. Used for looking up slay values in obj-power.c
 */
typedef struct
{
	u32b flags;		/* Combination of slays and brands */
	s32b value;		/* Value of this combination */
} flag_cache;

#endif /* INCLUDED_OBJECT_TYPES_H */
