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
typedef struct
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


#endif /* INCLUDED_OBJECT_TYPES_H */
