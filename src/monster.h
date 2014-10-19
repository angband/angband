/**
 * \file monster.h
 * \brief structures and functions for monsters
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
#include "mon-constants.h"
#include "mon-timed.h"
#include "mon-blow-methods.h"
#include "mon-blow-effects.h"

/** Constants **/




/**
 * Monster spell flag indices
 */
enum
{
    #define RSF(a, b, c, d, e, f, g, h) RSF_##a,
    #include "list-mon-spells.h"
    #undef RSF
};

#define RSF_SIZE               FLAG_SIZE(RSF_MAX)


/** Structures **/

/**
 * Monster blows
 */
struct monster_blow {
	struct monster_blow *next;	/* Unused after parsing */

	int method;			/* Method (RBM_*) */
	int effect;			/* Effect (RBE_*) */
	random_value dice;	/* Damage dice */
	int times_seen;		/* Sightings of the blow (lore only) */
};

/**
 * Monster pain messages
 */
typedef struct monster_pain
{
	const char *messages[7];
	int pain_idx;
	
	struct monster_pain *next;
} monster_pain;


/**
 * Monster spell types
 */
struct monster_spell {
	struct monster_spell *next;

	u16b index;				/* Numerical index (RSF_FOO) */
	int hit;				/* To-hit level for the attack */
	struct effect *effect;	/* Effect(s) of the spell */
	random_value power;		/* Relative power of the spell */
};


/**
 * Base monster type
 */
typedef struct monster_base
{
	struct monster_base *next;

	char *name;						/* Name for recognition in code */
	char *text;						/* In-game name */

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */
	
	wchar_t d_char;					/* Default monster character */

	monster_pain *pain;				/* Pain messages */
} monster_base;


/**
 * Specified monster drops
 */
struct monster_drop {
	struct monster_drop *next;
	struct object_kind *kind;
	struct artifact *artifact;
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
typedef struct monster_race
{
	struct monster_race *next;

	unsigned int ridx;

	char *name;
	char *text;
	char *plural;			/* Optional pluralized name */

	struct monster_base *base;
	
	int avg_hp;				/* Average HP for this creature */

	int ac;					/* Armour Class */

	int sleep;				/* Inactive counter (base) */
	int aaf;				/* Area affect radius (1-100) */
	int speed;				/* Speed (normally 110) */

	int mexp;				/* Exp value for kill */

	long power;				/* Monster power */
	long scaled_power;		/* Monster power scaled by level */

	int freq_innate;		/* Innate spell frequency */
	int freq_spell;			/* Other spell frequency */

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */

	struct monster_blow *blow; /* Melee blows */

	int level;				/* Level of creature */
	int rarity;				/* Rarity of creature */

	byte d_attr;			/* Default monster attribute */
	wchar_t d_char;			/* Default monster character */

	byte x_attr;			/* Desired monster attribute */
	wchar_t x_char;			/* Desired monster character */

	byte max_num;			/* Maximum population allowed per level */
	int cur_num;			/* Monster population on current level */

	struct monster_drop *drops;
    
    struct monster_friends *friends;
	
    struct monster_friends_base *friends_base;
    
	struct monster_mimic *mimic_kinds;
} monster_race;


/**
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 *
 * The "hold_o_idx" field points to the first object of a stack
 * of objects (if any) being carried by the monster (see above).
 */
typedef struct monster
{
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

	s16b mimicked_o_idx; /* Object this monster is mimicking */

	s16b hold_o_idx;	/* Object being held (if any) */

	byte attr;  		/* attr last used for drawing monster */

	player_state known_pstate; /* Known player state */

    byte ty;		/**< Monster target */
    byte tx;

    byte min_range;	/**< What is the closest we want to be?  Not saved */
    byte best_range;	/**< How close do we want to be? Not saved */
} monster_type;

/** Variables **/
extern s16b num_repro;

extern monster_pain *pain_messages;
extern struct monster_spell *monster_spells;
extern monster_base *rb_info;
extern monster_race *r_info;
extern const monster_race *ref_race;

#endif /* !MONSTER_MONSTER_H */
