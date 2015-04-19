/**
 * \file obj-randart.h
 * \brief Random artifact generation
 *
 * Copyright (c) 1998 Greg Wooledge, Ben Harrison, Robert Ruhlmann
 * Copyright (c) 2001 Chris Carr, Chris Robertson
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
 *
 * Original random artifact generator (randart) by Greg Wooledge.
 * Updated by Chris Carr / Chris Robertson 2001-2010.
 */

#ifndef OBJECT_RANDART_H
#define OBJECT_RANDART_H

#include "object.h"

#define MAX_TRIES 200
#define BUFLEN 1024

#define MIN_NAME_LEN 5
#define MAX_NAME_LEN 9

/**
 * Inhibiting factors for large bonus values
 * "HIGH" values use INHIBIT_WEAK
 * "VERYHIGH" values use INHIBIT_STRONG
 */
#define INHIBIT_STRONG  (one_in_(6))
#define INHIBIT_WEAK    (one_in_(2))

/**
 * Power rating below which uncursed randarts cannot aggravate
 * (so that aggravate is found only on endgame-quality items or
 * cursed items)
 */
#define AGGR_POWER 300

/**
 * Numerical index values for the different learned probabilities
 * These are to make the code more readable.
 */

enum {
	ART_IDX_BOW_SHOTS,
	ART_IDX_BOW_MIGHT,
	ART_IDX_BOW_BRAND,
	ART_IDX_BOW_SLAY,

	ART_IDX_WEAPON_HIT,
	ART_IDX_WEAPON_DAM,

	ART_IDX_NONWEAPON_HIT,
	ART_IDX_NONWEAPON_DAM,
	ART_IDX_NONWEAPON_HIT_DAM,
	ART_IDX_NONWEAPON_BRAND,
	ART_IDX_NONWEAPON_SLAY,
	ART_IDX_NONWEAPON_BLOWS,
	ART_IDX_NONWEAPON_SHOTS,

	ART_IDX_MELEE_BLESS,
	ART_IDX_MELEE_BRAND,
	ART_IDX_MELEE_SLAY,
	ART_IDX_MELEE_SINV,
	ART_IDX_MELEE_BLOWS,
	ART_IDX_MELEE_AC,
	ART_IDX_MELEE_DICE,
	ART_IDX_MELEE_WEIGHT,
	ART_IDX_MELEE_TUNN,

	ART_IDX_ALLARMOR_WEIGHT,

	ART_IDX_BOOT_AC,
	ART_IDX_BOOT_FEATHER,
	ART_IDX_BOOT_STEALTH,
	ART_IDX_BOOT_SPEED,

	ART_IDX_GLOVE_AC,
	ART_IDX_GLOVE_FA,
	ART_IDX_GLOVE_DEX,

	ART_IDX_HELM_AC,
	ART_IDX_HELM_RBLIND,
	ART_IDX_HELM_ESP,
	ART_IDX_HELM_SINV,
	ART_IDX_HELM_WIS,
	ART_IDX_HELM_INT,

	ART_IDX_SHIELD_AC,
	ART_IDX_SHIELD_LRES,

	ART_IDX_CLOAK_AC,
	ART_IDX_CLOAK_STEALTH,

	ART_IDX_ARMOR_AC,
	ART_IDX_ARMOR_STEALTH,
	ART_IDX_ARMOR_HLIFE,
	ART_IDX_ARMOR_CON,
	ART_IDX_ARMOR_LRES,
	ART_IDX_ARMOR_ALLRES,
	ART_IDX_ARMOR_HRES,

	ART_IDX_GEN_STAT,
	ART_IDX_GEN_SUST,
	ART_IDX_GEN_STEALTH,
	ART_IDX_GEN_SEARCH,
	ART_IDX_GEN_INFRA,
	ART_IDX_GEN_SPEED,
	ART_IDX_GEN_IMMUNE,
	ART_IDX_GEN_FA,
	ART_IDX_GEN_HLIFE,
	ART_IDX_GEN_FEATHER,
	ART_IDX_GEN_LIGHT,
	ART_IDX_GEN_SINV,
	ART_IDX_GEN_ESP,
	ART_IDX_GEN_SDIG,
	ART_IDX_GEN_REGEN,
	ART_IDX_GEN_LRES,
	ART_IDX_GEN_RPOIS,
	ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLIGHT,
	ART_IDX_GEN_RDARK,
	ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF,
	ART_IDX_GEN_RSOUND,
	ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS,
	ART_IDX_GEN_RNETHER,
	ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN,
	ART_IDX_GEN_AC,
	ART_IDX_GEN_TUNN,
	ART_IDX_GEN_ACTIV,
	ART_IDX_GEN_PSTUN,

/* Supercharged abilities - treated differently in algorithm */

	ART_IDX_MELEE_DICE_SUPER,
	ART_IDX_BOW_SHOTS_SUPER,
	ART_IDX_BOW_MIGHT_SUPER,
	ART_IDX_GEN_SPEED_SUPER,
	ART_IDX_MELEE_BLOWS_SUPER,
	ART_IDX_GEN_AC_SUPER,

/* Aggravation - weapon and nonweapon */
	ART_IDX_WEAPON_AGGR,
	ART_IDX_NONWEAPON_AGGR,

/* Total of abilities */
	ART_IDX_TOTAL
};

char *artifact_gen_name(struct artifact *a, const char ***wordlist);
errr do_randart(u32b randart_seed, bool full);

#endif /* OBJECT_RANDART_H */
