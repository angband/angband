#ifndef INCLUDED_MONSTER_TYPES_H
#define INCLUDED_MONSTER_TYPES_H

#include "defines.h"
#include "h-basic.h"
#include "z-bitflag.h"
#include "z-rand.h"

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

	s16b power;				/* Monster power */

	s16b highest_threat;	/* Monster highest threat */

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
typedef struct
{
	s16b r_idx;			/* Monster race index */

	byte fy;			/* Y location on map */
	byte fx;			/* X location on map */

	s16b hp;			/* Current Hit points */
	s16b maxhp;			/* Max Hit points */

	s16b m_timed[MON_TMD_MAX];

	byte mspeed;		/* Monster "speed" */
	byte energy;		/* Monster "energy" */

	byte cdis;			/* Current dis from player */

	byte mflag;			/* Extra monster flags */

	bool ml;			/* Monster is "visible" */
	bool unaware;		/* Player doesn't know this is a monster */

	s16b hold_o_idx;	/* Object being held (if any) */

	byte attr;  /* attr last used for drawing monster */

	u32b smart;			/* Field for "adult_ai_learn" */
} monster_type;

/* 
 * Monster data for the visible monster list 
 */
typedef struct
{
	u16b count;		/* total number of this type visible */
	u16b asleep;		/* number asleep (not in LOS) */
	u16b los;		/* number in LOS */
	u16b los_asleep;	/* number asleep and in LOS */
	byte attr; /* attr to use for drawing */
} monster_vis; 

/*
 * A stacked monster message entry
 */
typedef struct monster_race_message
{
	s16b mon_race;		/* The race of the monster */
	byte mon_flags;		/* Flags: 0x01 means hidden monster, 0x02 means offscreen monster */
 	int  msg_code;		/* The coded message */
	byte mon_count;		/* How many monsters triggered this message */
	bool delay;			/* Should this message be put off to the end */
} monster_race_message;

typedef struct monster_message_history
{
	int monster_idx;	/* The monster */
	int message_code;		/* The coded message */
} monster_message_history;

/**
 * Structure for monster spell types
 */
struct mon_spell {
    u16b index;             /* Numerical index (RSF_FOO) */
    int type;               /* Type bitflag */
    const char *desc;       /* Verbal description */
    int cap;                /* Damage cap */
    int div;                /* Damage divisor (monhp / this) */
    int gf;                 /* Flag for projection type (GF_FOO) */
    int msgt;               /* Flag for message colouring */
    bool save;              /* Does this attack allow a saving throw? */
    int hit;                /* To-hit level for the attack */
    const char *verb;       /* Description of the attack */
    random_value base_dam;  /* Base damage for the attack */
    random_value rlev_dam;  /* Monster-level-dependent damage */
    const char *blind_verb; /* Description of the attack if unseen */
};

/**
 * Structure for side effects of spell attacks
 */
struct spell_effect {
    u16b index;             /* Numerical index (RAE_#) */
    u16b method;            /* What attack has this effect (RSF_ or GF_) */
    bool timed;             /* TRUE if timed, FALSE if permanent */
    int flag;               /* Effect flag */
    random_value base;      /* The base duration or impact */
    random_value dam;       /* Damage-dependent duration or impact */
    int chance;             /* Chance of this effect if >1 available */
    bool save;              /* Does this effect allow a saving throw? */
    int res_flag;           /* Resistance to this specific effect */
};

#endif /* INCLUDED_MONSTER_TYPES_H */
