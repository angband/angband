/**
 * \file monster.h
 * \brief Flags, structures and variables for monsters
 *
 * Copyright (c) 2007 Andi Sidwell
 * Copyright (c) 2010 Chris Carr
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#ifndef MONSTER_MONSTER_H
#define MONSTER_MONSTER_H

#include "h-basic.h"
#include "z-bitflag.h"
#include "z-rand.h"
#include "cave.h"
#include "mon-timed.h"
#include "mon-blows.h"

/*** Monster flags ***/

/**
 * Special Monster Flags (all temporary)
 */
enum
{
	#define MFLAG(a, b) MFLAG_##a,
	#include "list-mon-temp-flags.h"
	#undef MFLAG
	MFLAG_MAX
};


/**
 * The monster flag types
 */
enum monster_flag_type {
	RFT_NONE = 0,	/* placeholder flag */
	RFT_OBV,		/* an obvious property */
	RFT_DISP,		/* for display purposes */
	RFT_GEN,		/* related to generation */
	RFT_NOTE,		/* especially noteworthy for lore */
	RFT_BEHAV,		/* behaviour-related */
	RFT_DROP,		/* drop details */
	RFT_DET,		/* detection properties */
	RFT_ALTER,		/* environment shaping */
	RFT_RACE_N,		/* types of monster (noun) */
	RFT_RACE_A,		/* types of monster (adjective) */
	RFT_VULN,		/* vulnerabilities with no corresponding resistance */
	RFT_VULN_I,		/* vulnerabilities with a corresponding resistance */
	RFT_RES,		/* elemental resistances */
	RFT_PROT,		/* immunity from status effects */

	RFT_MAX
};

#define MFLAG_SIZE                FLAG_SIZE(MFLAG_MAX)

#define mflag_has(f, flag)        flag_has_dbg(f, MFLAG_SIZE, flag, #f, #flag)
#define mflag_next(f, flag)       flag_next(f, MFLAG_SIZE, flag)
#define mflag_is_empty(f)         flag_is_empty(f, MFLAG_SIZE)
#define mflag_is_full(f)          flag_is_full(f, MFLAG_SIZE)
#define mflag_is_inter(f1, f2)    flag_is_inter(f1, f2, MFLAG_SIZE)
#define mflag_is_subset(f1, f2)   flag_is_subset(f1, f2, MFLAG_SIZE)
#define mflag_is_equal(f1, f2)    flag_is_equal(f1, f2, MFLAG_SIZE)
#define mflag_on(f, flag)         flag_on_dbg(f, MFLAG_SIZE, flag, #f, #flag)
#define mflag_off(f, flag)        flag_off(f, MFLAG_SIZE, flag)
#define mflag_wipe(f)             flag_wipe(f, MFLAG_SIZE)
#define mflag_setall(f)           flag_setall(f, MFLAG_SIZE)
#define mflag_negate(f)           flag_negate(f, MFLAG_SIZE)
#define mflag_copy(f1, f2)        flag_copy(f1, f2, MFLAG_SIZE)
#define mflag_union(f1, f2)       flag_union(f1, f2, MFLAG_SIZE)
#define mflag_inter(f1, f2)       flag_inter(f1, f2, MFLAG_SIZE)
#define mflag_diff(f1, f2)        flag_diff(f1, f2, MFLAG_SIZE)

/**
 * Monster property and ability flags (race flags)
 */
enum
{
	#define RF(a, b, c) RF_##a,
	#include "list-mon-race-flags.h"
	#undef RF
	RF_MAX
};

#define RF_SIZE                FLAG_SIZE(RF_MAX)

#define rf_has(f, flag)        flag_has_dbg(f, RF_SIZE, flag, #f, #flag)
#define rf_next(f, flag)       flag_next(f, RF_SIZE, flag)
#define rf_count(f)            flag_count(f, RF_SIZE)
#define rf_is_empty(f)         flag_is_empty(f, RF_SIZE)
#define rf_is_full(f)          flag_is_full(f, RF_SIZE)
#define rf_is_inter(f1, f2)    flag_is_inter(f1, f2, RF_SIZE)
#define rf_is_subset(f1, f2)   flag_is_subset(f1, f2, RF_SIZE)
#define rf_is_equal(f1, f2)    flag_is_equal(f1, f2, RF_SIZE)
#define rf_on(f, flag)         flag_on_dbg(f, RF_SIZE, flag, #f, #flag)
#define rf_off(f, flag)        flag_off(f, RF_SIZE, flag)
#define rf_wipe(f)             flag_wipe(f, RF_SIZE)
#define rf_setall(f)           flag_setall(f, RF_SIZE)
#define rf_negate(f)           flag_negate(f, RF_SIZE)
#define rf_copy(f1, f2)        flag_copy(f1, f2, RF_SIZE)
#define rf_union(f1, f2)       flag_union(f1, f2, RF_SIZE)
#define rf_comp_union(f1, f2)  flag_comp_union(f1, f2, RF_SIZE)
#define rf_inter(f1, f2)       flag_inter(f1, f2, RF_SIZE)
#define rf_diff(f1, f2)        flag_diff(f1, f2, RF_SIZE)


/**
 * Monster spell flag indices
 */
enum
{
    #define RSF(a, b) RSF_##a,
    #include "list-mon-spells.h"
    #undef RSF
};

#define RSF_SIZE               FLAG_SIZE(RSF_MAX)


/** Structures **/

/**
 * The monster flag structure
 */
struct monster_flag {
	u16b index;				/* the RF_ index */
	u16b type;				/* RFT_ category */
	const char *desc;		/* lore description */
};

/**
 * Monster blows
 */
struct monster_blow {
	struct monster_blow *next;	/* Unused after parsing */

	struct blow_method *method;	/* Method */
	struct blow_effect *effect;	/* Effect */
	random_value dice;			/* Damage dice */
	int times_seen;				/* Sightings of the blow (lore only) */
};

/**
 * Monster pain messages
 */
struct monster_pain {
	const char *messages[7];
	int pain_idx;
	
	struct monster_pain *next;
};


/**
 * Monster spell types
 */
struct monster_spell {
	struct monster_spell *next;

	u16b index;				/* Numerical index (RSF_FOO) */
	int msgt;				/* Flag for message colouring */
	char *message;			/* Description of the attack */
	char *blind_message;	/* Description of the attack if unseen */
	char *message_strong;   /* Description of the attack (strong version) */
	char *blind_message_strong;/* Attack description if unseen (strong)*/
	char *miss_message;		/* Description of a missed attack */
	char *save_message;		/* Message on passing saving throw, if any */
	char *lore_desc;		/* Description of the attack used in lore text */
	char *lore_desc_strong; /* Attack description used in lore text (strong) */
	byte lore_attr;			/* Color of the attack used in lore text */
	byte lore_attr_resist;	/* Color used in lore text when resisted */
	byte lore_attr_immune;	/* Color used in lore text when resisted strongly */
	int hit;				/* To-hit level for the attack */
	struct effect *effect;	/* Effect(s) of the spell */
	random_value power;		/* Relative power of the spell */
};


/**
 * Base monster type
 */
struct monster_base {
	struct monster_base *next;

	char *name;						/* Name for recognition in code */
	char *text;						/* In-game name */

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */
	
	wchar_t d_char;					/* Default monster character */

	struct monster_pain *pain;				/* Pain messages */
};


/**
 * Specified monster drops
 */
struct monster_drop {
	struct monster_drop *next;
	struct object_kind *kind;
	unsigned int tval;
	unsigned int percent_chance;
	unsigned int min;
	unsigned int max;
};

/**
 * Monster friends (specific monster)
 */
struct monster_friends {
	struct monster_friends *next;
	char *name;
	struct monster_race *race;
	unsigned int percent_chance;
	unsigned int number_dice;
	unsigned int number_side;
};

/**
 * Monster friends (general type)
 */
struct monster_friends_base {
	struct monster_friends_base *next;
	struct monster_base *base;
	unsigned int percent_chance;
	unsigned int number_dice;
	unsigned int number_side;
};

/**
 * How monsters mimic
 */
struct monster_mimic {
	struct monster_mimic *next;
	struct object_kind *kind;
};

/**
 * Monster "race" information, including racial memories
 *
 * Note that "d_attr" and "d_char" are used for MORE than "visual" stuff.
 *
 * Note that "cur_num" (and "max_num") represent the number of monsters
 * of the given race currently on (and allowed on) the current level.
 * This information yields the "dead" flag for Unique monsters.
 *
 * Note that "max_num" is reset when a new player is created.
 * Note that "cur_num" is reset when a new level is created.
 *
 * Maybe "cur_num", and "max_num" should be moved out of this array since
 * they are not read from "monster.txt".
 */
struct monster_race {
	struct monster_race *next;

	unsigned int ridx;

	char *name;
	char *text;
	char *plural;			/* Optional pluralized name */

	struct monster_base *base;
	
	int avg_hp;				/* Average HP for this creature */

	int ac;					/* Armour Class */

	int sleep;				/* Inactive counter (base) */
	int hearing;			/* Monster sense of hearing (1-100, standard 20) */
	int smell;				/* Monster sense of smell (0-50, standard 20) */
	int speed;				/* Speed (normally 110) */

	int mexp;				/* Exp value for kill */

	int freq_innate;		/* Innate spell frequency */
	int freq_spell;			/* Other spell frequency */
	int spell_power;		/* Power of spells */

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */

	struct monster_blow *blow; /* Melee blows */

	int level;				/* Level of creature */
	int rarity;				/* Rarity of creature */

	byte d_attr;			/* Default monster attribute */
	wchar_t d_char;			/* Default monster character */

	byte max_num;			/* Maximum population allowed per level */
	int cur_num;			/* Monster population on current level */

	struct monster_drop *drops;
    
    struct monster_friends *friends;
	
    struct monster_friends_base *friends_base;
    
	struct monster_mimic *mimic_kinds;
};


/**
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 *
 * The "held_obj" field points to the first object of a stack
 * of objects (if any) being carried by the monster (see above).
 */
struct monster {
	struct monster_race *race;
	int midx;

	byte fy;			/* Y location on map */
	byte fx;			/* X location on map */

	s16b hp;			/* Current Hit points */
	s16b maxhp;			/* Max Hit points */

	s16b m_timed[MON_TMD_MAX]; /* Timed monster status effects */

	byte mspeed;		/* Monster "speed" */
	byte energy;		/* Monster "energy" */

	byte cdis;			/* Current dis from player */

	bitflag mflag[MFLAG_SIZE];	/* Temporary monster flags */

	struct object *mimicked_obj; /* Object this monster is mimicking */
	struct object *held_obj;	/* Object being held (if any) */

	byte attr;  		/* attr last used for drawing monster */

	struct player_state known_pstate; /* Known player state */

    byte ty;		/**< Monster target */
    byte tx;

    byte min_range;	/**< What is the closest we want to be?  Not saved */
    byte best_range;	/**< How close do we want to be? Not saved */
};

/** Variables **/

extern s16b num_repro;

extern struct monster_pain *pain_messages;
extern struct monster_spell *monster_spells;
extern struct monster_base *rb_info;
extern struct monster_race *r_info;
extern const struct monster_race *ref_race;

#endif /* !MONSTER_MONSTER_H */
