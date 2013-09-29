/*
 * File: monster/constants.h
 * Purpose: constants for monsters
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2009 Chris Carr
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

#ifndef INCLUDED_MONSTER_CONSTANTS_H
#define INCLUDED_MONSTER_CONSTANTS_H

/*
 * Maximum flow depth when using "MONSTER_FLOW"
 */
#define MONSTER_FLOW_DEPTH 32


/*** Monster blow constants ***/
#define MONSTER_BLOW_MAX 4

/*
 * Monster blow methods
 */
enum
{
	#define RBM(x, y) RBM_##x,
	#include "list-blow-methods.h"
	#undef RBM
	RBM_MAX
};

/*
 * Monster blow effects
 */
enum
{
	#define RBE(x, y) RBE_##x,
	#include "list-blow-effects.h"
	#undef RBE
	RBE_MAX
};


/*
 * XXX Hack: player immunity to mana draining cannot be represented by 
 * m_ptr->known_pflags, so we need this.
 */
#define SM_IMM_MANA		0x00000800

/*** Monster flags ***/

/*
 * Special Monster Flags (all temporary)
 */
#define MFLAG_VIEW	0x01	/* Monster is in line of sight */
/* xxx */
#define MFLAG_NICE	0x20	/* Monster is still being nice */
#define MFLAG_SHOW	0x40	/* Monster is recently memorized */
#define MFLAG_MARK	0x80	/* Monster is currently memorized */


/*
 * Monster property and ability flags (race flags)
 */
enum
{
	#define RF(a,b) RF_##a,
	#include "list-mon-flags.h"
	#undef RF
	RF_MAX
};

#define RF_SIZE                FLAG_SIZE(RF_MAX)
#define RF_BYTES	  		   32 /* savefile bytes, i.e. 256 flags */

#define rf_has(f, flag)        flag_has_dbg(f, RF_SIZE, flag, #f, #flag)
#define rf_next(f, flag)       flag_next(f, RF_SIZE, flag)
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

/* Some flags are obvious */
#define RF_OBVIOUS_MASK \
	RF_UNIQUE, RF_QUESTOR, RF_MALE, RF_FEMALE, \
	RF_GROUP_AI

/* "race" flags */
#define RF_RACE_MASK \
	RF_ORC, RF_TROLL, RF_GIANT, RF_DRAGON, RF_DEMON, \
	RF_UNDEAD, RF_EVIL, RF_ANIMAL, RF_METAL, RF_NONLIVING

/* Drop flags to be revealed on first kill */
#define RF_DROP_MASK \
	RF_DROP_GOOD, \
	RF_DROP_GREAT, \
	RF_ONLY_ITEM, \
	RF_ONLY_GOLD, \
	RF_DROP_20, \
	RF_DROP_40, \
	RF_DROP_60, \
	RF_DROP_4, \
	RF_DROP_3, \
	RF_DROP_2, \
	RF_DROP_1

/*
 * Some monster types are different.
 */
#define monster_is_unusual(R) \
	flags_test((R)->flags, RF_SIZE, RF_DEMON, RF_UNDEAD, RF_STUPID, RF_NONLIVING, FLAG_END)

#define monster_is_nonliving(R) \
	flags_test((R)->flags, RF_SIZE, RF_DEMON, RF_UNDEAD, RF_NONLIVING, FLAG_END)

#endif /* INCLUDED_MONSTER_CONSTANTS_H */
