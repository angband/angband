/*
 * File: src/monster/mon-spell.h
 * Purpose: structures and functions for monster power
 *
 * Copyright (c) 2011 Chris Carr
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

#ifndef MONSTER_SPELL_H
#define MONSTER_SPELL_H

#include "angband.h"

/** Variables **/
/* none so far */

/** Constants **/

/* List of spell effects */
enum
{
    #define RSE(a, b, c, d, e, f, g, h, i, j, k) \
            RSE_##a,
    #include "list-spell-effects.h"
    #undef RSE
};

/* Flags for non-timed spell effects
 * - include legal restrictions for "summon_specific()"
 * (see src/timed.h for timed effect flags) */
enum spell_effect_flag {
    S_INV_DAM,
    S_TELEPORT,
    S_TELE_TO,
    S_TELE_LEV,
	S_TELE_SELF,
    S_DRAIN_LIFE,
    S_DRAIN_STAT,
    S_SWAP_STAT,
    S_DRAIN_ALL,
    S_DISEN,
    S_ANIMAL = 11,
    S_SPIDER = 12,
    S_HOUND = 13,
    S_HYDRA = 14,
    S_ANGEL = 15,
    S_DEMON = 16,
    S_UNDEAD = 17,
    S_DRAGON = 18,
    S_HI_DEMON = 26,
    S_HI_UNDEAD = 27,
    S_HI_DRAGON = 28,
    S_WRAITH = 31,
    S_UNIQUE = 32,
    S_KIN = 33,
    S_MONSTER = 41,
    S_MONSTERS = 42,
	S_DRAIN_MANA,
	S_HEAL,
	S_BLINK,
	S_DARKEN,
	S_TRAPS,
	S_AGGRAVATE,

    S_MAX
};

/* Spell type bitflags */
enum mon_spell_type {
    RST_BOLT    = 0x001,
    RST_BALL    = 0x002,
    RST_BREATH  = 0x004,
    RST_ATTACK  = 0x008,    /* Direct (non-projectable) attacks */
    RST_ANNOY   = 0x010,    /* Irritant spells, usually non-fatal */
    RST_HASTE   = 0x020,    /* Relative speed advantage */
    RST_HEAL    = 0x040,
    RST_TACTIC  = 0x080,    /* Get a better position */
    RST_ESCAPE  = 0x100,
    RST_SUMMON  = 0x200
};

/* Minimum flag which can fail */
#define MIN_NONINNATE_SPELL    (FLAG_START + 32)

/** Macros **/
#define rsf_has(f, flag)       flag_has_dbg(f, RSF_SIZE, flag, #f, #flag)
#define rsf_next(f, flag)      flag_next(f, RSF_SIZE, flag)
#define rsf_is_empty(f)        flag_is_empty(f, RSF_SIZE)
#define rsf_is_full(f)         flag_is_full(f, RSF_SIZE)
#define rsf_is_inter(f1, f2)   flag_is_inter(f1, f2, RSF_SIZE)
#define rsf_is_subset(f1, f2)  flag_is_subset(f1, f2, RSF_SIZE)
#define rsf_is_equal(f1, f2)   flag_is_equal(f1, f2, RSF_SIZE)
#define rsf_on(f, flag)        flag_on_dbg(f, RSF_SIZE, flag, #f, #flag)
#define rsf_off(f, flag)       flag_off(f, RSF_SIZE, flag)
#define rsf_wipe(f)            flag_wipe(f, RSF_SIZE)
#define rsf_setall(f)          flag_setall(f, RSF_SIZE)
#define rsf_negate(f)          flag_negate(f, RSF_SIZE)
#define rsf_copy(f1, f2)       flag_copy(f1, f2, RSF_SIZE)
#define rsf_union(f1, f2)      flag_union(f1, f2, RSF_SIZE)
#define rsf_comp_union(f1, f2) flag_comp_union(f1, f2, RSF_SIZE)
#define rsf_inter(f1, f2)      flag_inter(f1, f2, RSF_SIZE)
#define rsf_diff(f1, f2)       flag_diff(f1, f2, RSF_SIZE)


/** Structures **/

/* Structure for monster spell types */
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

/* Structure for side effects of spell attacks */
struct spell_effect {
    u16b index;             /* Numerical index (RAE_#) */
    u16b method;            /* What RSF_ attack has this effect */
    int gf;                 /* What GF_ type has this effect */
    bool timed;             /* TRUE if timed, FALSE if permanent */
    int flag;               /* Effect flag */
    random_value base;      /* The base duration or impact */
    random_value dam;       /* Damage-dependent duration or impact */
    int chance;             /* Chance of this effect if >1 available */
    bool save;              /* Does this effect allow a saving throw? */
    int res_flag;           /* Resistance to this specific effect */
	random_value power;		/* Power rating of effect */
};


/** Functions **/
void do_mon_spell(int spell, int m_idx, bool seen);
bool test_spells(bitflag *f, enum mon_spell_type type);
void set_spells(bitflag *f, enum mon_spell_type type);
int best_spell_power(const monster_race *r_ptr, int resist);
void unset_spells(bitflag *spells, bitflag *flags, const monster_race *r_ptr);

#endif /* MONSTER_SPELL_H */
