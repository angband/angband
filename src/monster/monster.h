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

/** Constants **/

/* Monster spell flags */
enum
{
    #define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m) RSF_##a,
    #include "list-mon-spells.h"
    #undef RSF
};

#define RSF_SIZE               FLAG_SIZE(RSF_MAX)

/* The codified monster messages */
enum {
	MON_MSG_NONE = 0,

	/* project_m */
	MON_MSG_DIE,
	MON_MSG_DESTROYED,
	MON_MSG_RESIST_A_LOT,
	MON_MSG_HIT_HARD,
	MON_MSG_RESIST,
	MON_MSG_IMMUNE,
	MON_MSG_RESIST_SOMEWHAT,
	MON_MSG_UNAFFECTED,
	MON_MSG_SPAWN,
	MON_MSG_HEALTHIER,
	MON_MSG_FALL_ASLEEP,
	MON_MSG_WAKES_UP,
	MON_MSG_CRINGE_LIGHT,
	MON_MSG_SHRIVEL_LIGHT,
	MON_MSG_LOSE_SKIN,
	MON_MSG_DISSOLVE,
	MON_MSG_CATCH_FIRE,
	MON_MSG_BADLY_FROZEN,
	MON_MSG_SHUDDER,
	MON_MSG_CHANGE,
	MON_MSG_DISAPPEAR,
	MON_MSG_MORE_DAZED,
	MON_MSG_DAZED,
	MON_MSG_NOT_DAZED,
	MON_MSG_MORE_CONFUSED,
	MON_MSG_CONFUSED,
	MON_MSG_NOT_CONFUSED,
	MON_MSG_MORE_SLOWED,
	MON_MSG_SLOWED,
	MON_MSG_NOT_SLOWED,
	MON_MSG_MORE_HASTED,
	MON_MSG_HASTED,
	MON_MSG_NOT_HASTED,
	MON_MSG_MORE_AFRAID,
	MON_MSG_FLEE_IN_TERROR,
	MON_MSG_NOT_AFRAID,
	MON_MSG_MORIA_DEATH,
	MON_MSG_DISENTEGRATES,
	MON_MSG_FREEZE_SHATTER,
	MON_MSG_MANA_DRAIN,
	MON_MSG_BRIEF_PUZZLE,
	MON_MSG_MAINTAIN_SHAPE,
	
	/* message_pain */
	MON_MSG_UNHARMED,
	MON_MSG_95,
	MON_MSG_75,
	MON_MSG_50,
	MON_MSG_35,
	MON_MSG_20,
	MON_MSG_10,
	MON_MSG_0,

	/* Always leave this at the end */
	MAX_MON_MSG
};


/* Maxinum number of stacked monster messages */
#define MAX_STORED_MON_MSG		200
#define MAX_STORED_MON_CODES	400


/* Flags for the monster timed functions */
#define MON_TMD_FLG_NOTIFY		0x01 /* Give notification */
#define MON_TMD_MON_SOURCE		0x02 /* Monster is causing the damage */
#define MON_TMD_FLG_NOMESSAGE	0x04 /* Never show a message */
#define MON_TMD_FLG_NOFAIL		0x08 /* Never fail */


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

	s16b hold_o_idx;	/* Object being held (if any) */

	byte attr;  		/* attr last used for drawing monster */

	u32b smart;			/* Field for "adult_ai_learn" */

	bitflag known_pflags[OF_SIZE]; /* Known player flags */
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


/*** Functions ***/

/* melee2.c */
extern bool check_hit(struct player *p, int power, int level);

/* monster1.c */
extern bool mon_inc_timed(int m_idx, int idx, int v, u16b flag);
extern bool mon_dec_timed(int m_idx, int idx, int v, u16b flag);
extern bool mon_clear_timed(int m_idx, int idx, u16b flag);
extern void cheat_monster_lore(int r_idx, monster_lore *l_ptr);
extern void wipe_monster_lore(int r_idx, monster_lore *l_ptr);
extern void describe_monster(int r_idx, bool spoilers);
extern void roff_top(int r_idx);
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);
extern int lookup_monster(const char *name);
extern int rval_find_idx(const char *name);
extern const char *rval_find_name(int rval);
monster_base *lookup_monster_base(const char *name);
bool match_monster_bases(monster_base *base, ...);

/* monster2.c */
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void wipe_mon_list(struct cave *c, struct player *p);
extern s16b mon_pop(void);
extern void get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void display_monlist(void);
extern void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode);
extern void lore_do_probe(int m_idx);
extern void update_mon(int m_idx, bool full);
extern void update_monsters(bool full);
extern s16b monster_carry(struct monster *m, object_type *j_ptr);
extern void monster_swap(int y1, int x1, int y2, int x2);
extern void player_place(struct cave *c, struct player *p, int y, int x);
extern s16b monster_place(int y, int x, monster_type *n_ptr, byte origin);
extern bool place_monster_aux(struct cave *, int y, int x, int r_idx, bool slp,
	bool grp, byte origin);
extern bool place_monster(struct cave *c, int y, int x, int depth, bool slp,
	bool grp, byte origin);
extern bool alloc_monster(struct cave *c, struct loc loc, int dis, bool slp, int depth);
extern bool summon_specific(int y1, int x1, int lev, int type, int delay);
extern bool multiply_monster(int m_idx);
extern void message_pain(int m_idx, int dam);
extern bool add_monster_message(const char *mon_name, int m_idx, int msg_code, bool delay);
extern void flush_all_monster_messages(void);
extern void update_smart_learn(struct monster *m, struct player *p, int what);
void monster_death(int m_idx, bool stats);
bool mon_take_hit(int m_idx, int dam, bool *fear, const char *note);
extern void monster_flags_known(const monster_race *r_ptr, const monster_lore *l_ptr, bitflag flags[RF_SIZE]);

extern void process_monsters(struct cave *c, byte min_energy);
int mon_hp(const struct monster_race *r_ptr, aspect hp_aspect);

#ifdef TEST
extern bool (*testfn_make_attack_normal)(struct monster *m, struct player *p);
#endif /* !TEST */

#endif /* !MONSTER_MONSTER_H */
