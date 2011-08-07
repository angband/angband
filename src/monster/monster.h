/*
 * File: monster.h
 * Purpose: structures and functions for monsters
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

#include "defines.h"
#include "h-basic.h"
#include "z-bitflag.h"
#include "z-rand.h"
#include "cave.h"
#include "player/types.h"
#include "monster/mon-timed.h"

/** Constants **/

/* Monster spell flags */
enum
{
    #define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m) RSF_##a,
    #include "list-mon-spells.h"
    #undef RSF
};

#define RSF_SIZE               FLAG_SIZE(RSF_MAX)


/** Structures **/

/*
 * Monster blow structure
 *
 *	- Method (RBM_*)
 *	- Effect (RBE_*)
 *	- Damage Dice
 *	- Damage Sides
 */
struct monster_blow {
	byte method;
	byte effect;
	byte d_dice;
	byte d_side;
};

/*
 * Monster pain messages.
 */
typedef struct monster_pain
{
	const char *messages[7];
	int pain_idx;
	
	struct monster_pain *next;
} monster_pain;
 
/*
 * Information about "base" monster type.
 */
typedef struct monster_base
{
	struct monster_base *next;

	char *name;
	char *text;

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */
	
	char d_char;			/* Default monster character */

	monster_pain *pain;		/* Pain messages */
} monster_base;

/* Information about specified monster drops */ 
struct monster_drop {
	struct monster_drop *next;
	struct object_kind *kind;
	struct artifact *artifact;
	unsigned int percent_chance;
	unsigned int min;
	unsigned int max;
};

struct monster_mimic {
	struct monster_mimic *next;
	struct object_kind *kind;
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
typedef struct monster_race
{
	struct monster_race *next;

	unsigned int ridx;

	char *name;
	char *text;

	struct monster_base *base;
	
	u16b avg_hp;				/* Average HP for this creature */

	s16b ac;				/* Armour Class */

	s16b sleep;				/* Inactive counter (base) */
	byte aaf;				/* Area affect radius (1-100) */
	byte speed;				/* Speed (normally 110) */

	s32b mexp;				/* Exp value for kill */

	long power;				/* Monster power */
	long scaled_power;		/* Monster power scaled by level */

	s16b highest_threat;	/* Monster highest threat */
	
	/*AMF:DEBUG*/			/**/
	long melee_dam;			/**/
	long spell_dam;			/**/
	long hp;				/**/
	/*END AMF:DEBUG*/		/**/

	byte freq_innate;		/* Innate spell frequency */
	byte freq_spell;		/* Other spell frequency */

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */

	struct monster_blow blow[MONSTER_BLOW_MAX]; /* Up to four blows per round */

	byte level;				/* Level of creature */
	byte rarity;			/* Rarity of creature */

	byte d_attr;			/* Default monster attribute */
	char d_char;			/* Default monster character */

	byte x_attr;			/* Desired monster attribute */
	char x_char;			/* Desired monster character */

	byte max_num;			/* Maximum population allowed per level */
	byte cur_num;			/* Monster population on current level */

	struct monster_drop *drops;
	
	struct monster_mimic *mimic_kinds;
} monster_race;


/*
 * Monster "lore" information
 *
 * Note that these fields are related to the "monster recall" and can
 * be scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc). XXX XXX XXX
 */
typedef struct
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

	bitflag flags[RF_SIZE]; /* Observed racial flags - a 1 indicates
	                         * the flag (or lack thereof) is known to
	                         * the player */
	bitflag spell_flags[RSF_SIZE];  /* Observed racial spell flags */
} monster_lore;



/*
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
	s16b r_idx;			/* Monster race index */
	int midx;

	byte fy;			/* Y location on map */
	byte fx;			/* X location on map */

	s16b hp;			/* Current Hit points */
	s16b maxhp;			/* Max Hit points */

	s16b m_timed[MON_TMD_MAX]; /* Timed monster status effects */

	byte mspeed;		/* Monster "speed" */
	byte energy;		/* Monster "energy" */

	byte cdis;			/* Current dis from player */

	byte mflag;			/* Extra monster flags */

	bool ml;			/* Monster is "visible" */
	bool unaware;		/* Player doesn't know this is a monster */
	
	s16b mimicked_o_idx; /* Object this monster is mimicking */

	s16b hold_o_idx;	/* Object being held (if any) */

	byte attr;  		/* attr last used for drawing monster */

	u32b smart;			/* Field for "adult_ai_learn" */

	bitflag known_pflags[OF_SIZE]; /* Known player flags */
} monster_type;

/*** Functions ***/

/* melee2.c */
extern bool check_hit(struct player *p, int power, int level);
extern void process_monsters(struct cave *c, byte min_energy);
int mon_hp(const struct monster_race *r_ptr, aspect hp_aspect);

#ifdef TEST
extern bool (*testfn_make_attack_normal)(struct monster *m, struct player *p);
#endif /* !TEST */

#endif /* !MONSTER_MONSTER_H */
