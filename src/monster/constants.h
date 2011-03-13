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
 * Legal restrictions for "summon_specific()"
 */
enum
{
	SUMMON_ANIMAL = 11,
	SUMMON_SPIDER = 12,
	SUMMON_HOUND = 13,
	SUMMON_HYDRA = 14,
	SUMMON_ANGEL = 15,
	SUMMON_DEMON = 16,
	SUMMON_UNDEAD = 17,
	SUMMON_DRAGON = 18,
	SUMMON_HI_DEMON = 26,
	SUMMON_HI_UNDEAD = 27,
	SUMMON_HI_DRAGON = 28,
	SUMMON_WRAITH = 31,
	SUMMON_UNIQUE = 32,
	SUMMON_KIN = 33,
	SUMMON_MONSTER = 41,
	SUMMON_MONSTERS = 42,
};


/*
 * Some constants for the "learn" code
 *
 * Most of these come from the "SM_xxx" flags
 */
#define DRS_FREE		14
#define DRS_MANA		15
#define DRS_RES_ACID	16
#define DRS_RES_ELEC	17
#define DRS_RES_FIRE	18
#define DRS_RES_COLD	19
#define DRS_RES_POIS	20
#define DRS_RES_FEAR	21
#define DRS_RES_LIGHT	22
#define DRS_RES_DARK	23
#define DRS_RES_BLIND	24
#define DRS_RES_CONFU	25
#define DRS_RES_SOUND	26
#define DRS_RES_SHARD	27
#define DRS_RES_NEXUS	28
#define DRS_RES_NETHR	29
#define DRS_RES_CHAOS	30
#define DRS_RES_DISEN	31


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
 * Some bit-flags for the "smart" field of "monster_type".
 *
 * Most of these map to the "OF_xxx" flags.
 */
#define SM_OPP_ACID		0x00000001
#define SM_OPP_ELEC		0x00000002
#define SM_OPP_FIRE		0x00000004
#define SM_OPP_COLD		0x00000008
#define SM_OPP_POIS		0x00000010
#define SM_OPP_XXX1		0x00000020
#define SM_OPP_XXX2		0x00000040
#define SM_OPP_XXX3		0x00000080
#define SM_IMM_XXX5		0x00000100
#define SM_IMM_XXX6		0x00000200
#define SM_IMM_FREE		0x00000400
#define SM_IMM_MANA		0x00000800
#define SM_IMM_ACID		0x00001000
#define SM_IMM_ELEC		0x00002000
#define SM_IMM_FIRE		0x00004000
#define SM_IMM_COLD		0x00008000
#define SM_RES_ACID		0x00010000
#define SM_RES_ELEC		0x00020000
#define SM_RES_FIRE		0x00040000
#define SM_RES_COLD		0x00080000
#define SM_RES_POIS		0x00100000
#define SM_RES_FEAR		0x00200000
#define SM_RES_LIGHT	0x00400000
#define SM_RES_DARK		0x00800000
#define SM_RES_BLIND	0x01000000
#define SM_RES_CONFU	0x02000000
#define SM_RES_SOUND	0x04000000
#define SM_RES_SHARD	0x08000000
#define SM_RES_NEXUS	0x10000000
#define SM_RES_NETHR	0x20000000
#define SM_RES_CHAOS	0x40000000
#define SM_RES_DISEN	0x80000000


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
	RF_FRIEND, RF_FRIENDS, RF_ESCORT, RF_ESCORTS

/* "race" flags */
#define RF_RACE_MASK \
	RF_ORC, RF_TROLL, RF_GIANT, RF_DRAGON, RF_DEMON, \
	RF_UNDEAD, RF_EVIL, RF_ANIMAL, RF_METAL, RF_NONLIVING


/*
 * Monster spell flags
 */
enum
{
	#define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m) RSF_##a,
	#include "list-mon-spells.h"
	#undef RSF
	RSF_MAX
};

/*
 * Spell type bitflags
 */
typedef enum
{
	RST_BOLT	= 0x001,
	RST_BALL	= 0x002,
	RST_BREATH 	= 0x004,
	RST_ATTACK	= 0x008,	/* Direct (non-projectable) attacks */
	RST_ANNOY	= 0x010,	/* Irritant spells, usually non-fatal */
	RST_HASTE	= 0x020,	/* Relative speed advantage */
	RST_HEAL	= 0x040,
	RST_TACTIC	= 0x080,	/* Get a better position */
	RST_ESCAPE	= 0x100,
	RST_SUMMON	= 0x200
} mon_spell_type;

#define RSF_SIZE               FLAG_SIZE(RSF_MAX)

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

/* Minimum flag which can fail */
#define MIN_NONINNATE_SPELL    (FLAG_START + 32)

/* List of spell effects */
enum
{
	#define RSE(a, b, c, d, e, f, g, h, i, j) \
            RSE_##a,
	#include "list-spell-effects.h"
	#undef RSE
};

/* Flags for non-timed side effects */
enum spell_effect_flag {
    S_INV_DAM,
    S_TELEPORT,
    S_TELE_TO,
    S_TELE_LEV,
    S_DRAIN_LIFE,
    S_DRAIN_STAT,
    S_SWAP_STAT,
    S_DRAIN_ALL,
	S_DISEN,

    S_MAX
};


/*
 * Some monster types are different.
 */
#define monster_is_unusual(R) \
	flags_test((R)->flags, RF_SIZE, RF_DEMON, RF_UNDEAD, RF_STUPID, RF_NONLIVING, FLAG_END)

#define monster_is_nonliving(R) \
	flags_test((R)->flags, RF_SIZE, RF_DEMON, RF_UNDEAD, RF_NONLIVING, FLAG_END)
	

/*** Obsolete - these will be removed soon ***/

/*
 * These macros are used in three places:
 *
 * melee2.c  - to calculate the damage for monster spell and breath attacks
 * spells1.c - to apply resistances to reduce this damage
 * init1.c   - to evaluate the "power" of monsters
 *
 * The first two uses can involve randomness, but the third requires minimum
 * values (of resistance) and maxima (of damage). So we use z-rand.c's
 * damcalc() as a wrapper for damroll(), which allows us to obtain the values
 * we need in each case. All uses of randint1(x) have been replaced with
 * damroll(1, x) in order to pass cleanly through the wrapper.
 *
 * If you want to change any damage or resistance calculations, you only have
 * to edit this file, without touching init1.c or melee2.c or spells1.c
 *
 * This complexity does not apply to breath damage, because there is
 * no random element. But it also means that if you want to make breath
 * damage dependent on rlev (for example), you have more work to do.
 */
#define NOT_USED	/* to avoid confusion in spells1.c */

#define ARROW1_HIT				40
#define ARROW1_DMG(level, dam_aspect)		(damcalc(1, 6, (dam_aspect)))

#define ARROW2_HIT				40
#define ARROW2_DMG(level, dam_aspect)		(damcalc(3, 6, (dam_aspect)))

#define ARROW3_HIT				50
#define ARROW3_DMG(level, dam_aspect)		(damcalc(5, 6, (dam_aspect)))

#define ARROW4_HIT				50
#define ARROW4_DMG(level, dam_aspect)		(damcalc(7, 6, (dam_aspect)))

#define BR_ACID_MAX				1600
#define BR_ACID_DIVISOR				3
#define RES_ACID_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_ACID_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_ACID_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_ELEC_MAX				1600
#define BR_ELEC_DIVISOR				3
#define RES_ELEC_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_ELEC_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_ELEC_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_FIRE_MAX				1600
#define BR_FIRE_DIVISOR				3
#define RES_FIRE_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_FIRE_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_FIRE_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_COLD_MAX				1600
#define BR_COLD_DIVISOR				3
#define RES_COLD_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_COLD_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_COLD_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_POIS_MAX				800
#define BR_POIS_DIVISOR				3
#define RES_POIS_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_POIS_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)

#define BR_NETH_MAX				550
#define BR_NETH_DIVISOR				6
#define RES_NETH_ADJ(dam, dam_aspect)		(((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_LIGHT_MAX				400
#define BR_LIGHT_DIVISOR			6
#define RES_LIGHT_ADJ(dam, dam_aspect)		(((dam) * 4) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_DARK_MAX				400
#define BR_DARK_DIVISOR				6
#define RES_DARK_ADJ(dam, dam_aspect)           (((dam) * 4) / (damcalc(1, 6, dam_aspect) + 6))
/* Confusion no longer used as an element, post-3.2 
#define BR_CONF_MAX				400
#define BR_CONF_DIVISOR				6
#define RES_CONF_ADJ(dam, dam_aspect)           (((dam) * 5) / (damcalc(1, 6, dam_aspect) + 6))
 */
#define BR_SOUN_MAX				500
#define BR_SOUN_DIVISOR				6
#define RES_SOUN_ADJ(dam, dam_aspect)           (((dam) * 5) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_CHAO_MAX				500
#define BR_CHAO_DIVISOR				6
#define RES_CHAO_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_DISE_MAX				500
#define BR_DISE_DIVISOR				6
#define RES_DISE_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_NEXU_MAX				400
#define BR_NEXU_DIVISOR				6
#define RES_NEXU_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_TIME_MAX				150
#define BR_TIME_DIVISOR				3
/* no resist */

#define BR_INER_MAX				200
#define BR_INER_DIVISOR				6
/* no resist */

#define BR_GRAV_MAX				200
#define BR_GRAV_DIVISOR				3
/* no resist */

#define BR_SHAR_MAX				500
#define BR_SHAR_DIVISOR				6
#define RES_SHAR_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_PLAS_MAX				150
#define BR_PLAS_DIVISOR				6
/* no resist */

#define BR_FORC_MAX				200
#define BR_FORC_DIVISOR				6
/* no resist */

/*
#define BR_MANA_MAX	1600
#define BR_MANA_DIVISOR 3
*/

#define BOULDER_HIT			60
#define BOULDER_DMG(level, dam_aspect)	(damcalc(1 + ((level) / 7), 12, (dam_aspect)))

#define BA_ACID_DMG(level, dam_aspect)	(damcalc(1, ((level) * 3), (dam_aspect)) + 15)
#define BA_ELEC_DMG(level, dam_aspect)	(damcalc(1, ((level) * 3 / 2), (dam_aspect)) + 8)
#define BA_FIRE_DMG(level, dam_aspect)	(damcalc(1, ((level) * 7 / 2), (dam_aspect)) + 10)
#define BA_COLD_DMG(level, dam_aspect)	(damcalc(1, ((level) * 3 / 2), (dam_aspect)) + 10)
#define BA_POIS_DMG(level, dam_aspect)	(damcalc(12, 2, (dam_aspect)))
#define BA_NETH_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + (level) + 50)
#define BA_WATE_DMG(level, dam_aspect)	(damcalc(1, ((level) * 5 / 2), (dam_aspect)) + 50)
#define BA_MANA_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + ((level) * 5))  /* manastorm */
#define BA_DARK_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + ((level) * 5))  /* darkness storm */

#define MIND_BLAST_DMG(level, dam_aspect)	(damcalc(8, 8, (dam_aspect)))
#define BRAIN_SMASH_DMG(level, dam_aspect)	(damcalc(12, 15, (dam_aspect)))

#define CAUSE1_DMG(level, dam_aspect)	(damcalc(3, 8, (dam_aspect)))
#define CAUSE2_DMG(level, dam_aspect)	(damcalc(8, 8, (dam_aspect)))
#define CAUSE3_DMG(level, dam_aspect)	(damcalc(10, 15, (dam_aspect)))
#define CAUSE4_DMG(level, dam_aspect)	(damcalc(15, 15, (dam_aspect)))
#define CAUSE4_CUT			damroll(10, 10)

#define BO_ACID_DMG(level, dam_aspect)	(damcalc(7, 8, (dam_aspect)) + ((level) / 3))
#define BO_ELEC_DMG(level, dam_aspect)	(damcalc(4, 8, (dam_aspect)) + ((level) / 3))
#define BO_FIRE_DMG(level, dam_aspect) 	(damcalc(9, 8, (dam_aspect)) + ((level) / 3))
#define BO_COLD_DMG(level, dam_aspect) 	(damcalc(6, 8, (dam_aspect)) + ((level) / 3))
/* #define BO_POIS_DMG(level, dam_aspect) 	(damcalc(9, 8, (dam_aspect)) + ((level) / 3)) */
#define BO_NETH_DMG(level, dam_aspect) 	(damcalc(5, 5, (dam_aspect)) + ((level) * 3 / 2) + 30)
#define BO_WATE_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + (level))
#define BO_MANA_DMG(level, dam_aspect)	(damcalc(1, ((level) * 7 / 2), (dam_aspect)) + 50)
#define BO_PLAS_DMG(level, dam_aspect)	(damcalc(8, 7, (dam_aspect)) + (level) + 10)
#define BO_ICEE_DMG(level, dam_aspect)	(damcalc(6, 6, (dam_aspect)) + (level))
#define MISSILE_DMG(level, dam_aspect)	(damcalc(2, 6, (dam_aspect)) + ((level) / 3))

#endif /* INCLUDED_MONSTER_CONSTANTS_H */
