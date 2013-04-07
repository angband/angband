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
#include "monster/mon-timed.h"
#include "object/obj-flag.h"

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
	uint8_t method;
	uint8_t effect;
	uint8_t d_dice;
	uint8_t d_side;
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
	
	wchar_t d_char;			/* Default monster character */

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
	
	uint16_t avg_hp;				/* Average HP for this creature */

	int16_t ac;				/* Armour Class */

	int16_t sleep;				/* Inactive counter (base) */
	uint8_t aaf;				/* Area affect radius (1-100) */
	uint8_t speed;				/* Speed (normally 110) */

	int32_t mexp;				/* Exp value for kill */

	long power;				/* Monster power */
	long scaled_power;		/* Monster power scaled by level */

	int16_t highest_threat;	/* Monster highest threat */
	
	/*AMF:DEBUG*/			/**/
	long melee_dam;			/**/
	long spell_dam;			/**/
	long hp;				/**/
	/*END AMF:DEBUG*/		/**/

	uint8_t freq_innate;		/* Innate spell frequency */
	uint8_t freq_spell;		/* Other spell frequency */

	bitflag flags[RF_SIZE];         /* Flags */
	bitflag spell_flags[RSF_SIZE];  /* Spell flags */

	struct monster_blow blow[MONSTER_BLOW_MAX]; /* Up to four blows per round */

	uint8_t level;				/* Level of creature */
	int rarity;			/* Rarity of creature */

	uint8_t d_attr;			/* Default monster attribute */
	wchar_t d_char;			/* Default monster character */

	uint8_t x_attr;			/* Desired monster attribute */
	wchar_t x_char;			/* Desired monster character */

	uint8_t max_num;			/* Maximum population allowed per level */
	uint8_t cur_num;			/* Monster population on current level */

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
	int16_t sights;			/* Count sightings of this monster */
	int16_t deaths;			/* Count deaths from this monster */

	int16_t pkills;			/* Count monsters killed in this life */
	int16_t tkills;			/* Count monsters killed in all lives */

	uint8_t wake;				/* Number of times woken up (?) */
	uint8_t ignore;			/* Number of times ignored (?) */

	uint8_t drop_gold;			/* Max number of gold dropped at once */
	uint8_t drop_item;			/* Max number of item dropped at once */

	uint8_t cast_innate;		/* Max number of innate spells seen */
	uint8_t cast_spell;		/* Max number of other spells seen */

	uint8_t blows[MONSTER_BLOW_MAX]; /* Number of times each blow type was seen */

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
	int midx;

	uint8_t fy;			/* Y location on map */
	uint8_t fx;			/* X location on map */

	int16_t hp;			/* Current Hit points */
	int16_t maxhp;			/* Max Hit points */

	int16_t m_timed[MON_TMD_MAX]; /* Timed monster status effects */

	uint8_t mspeed;		/* Monster "speed" */
	uint8_t energy;		/* Monster "energy" */

	uint8_t cdis;			/* Current dis from player */

	uint8_t mflag;			/* Extra monster flags */

	bool ml;			/* Monster is "visible" */
	bool unaware;		/* Player doesn't know this is a monster */
	
	int16_t mimicked_o_idx; /* Object this monster is mimicking */

	int16_t hold_o_idx;	/* Object being held (if any) */

	uint8_t attr;  		/* attr last used for drawing monster */

	uint32_t smart;			/* Field for "adult_ai_learn" */

	bitflag known_pflags[OF_SIZE]; /* Known player flags */
} monster_type;

/*** Functions ***/

/* melee2.c */
extern bool check_hit(struct player *p, int power, int level);
extern void process_monsters(struct cave *c, uint8_t min_energy);
int mon_hp(const struct monster_race *r_ptr, aspect hp_aspect);
extern bool make_attack_spell(struct monster *m);


extern int16_t num_repro;

extern bool (*testfn_make_attack_normal)(struct monster *m, struct player *p);

#endif /* !MONSTER_MONSTER_H */
