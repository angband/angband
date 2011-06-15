/*
 * File: obj-power.c
 * Purpose: calculation of object power
 *
 * Copyright (c) 2001 Chris Carr, Chris Robertson
 * Revised in 2009-11 by Chris Carr, Peter Denison
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
#include "angband.h"
#include "object/slays.h"
#include "object/tvalsval.h"
#include "object/pval.h"
#include "init.h"
#include "effects.h"
#include "monster/mon-power.h"

/**
 * Constants for the power algorithm:
 * - fudge factor for extra damage from rings etc. (used if extra blows)
 * - assumed damage for off-weapon brands
 * - base power for jewelry
 * - base power for armour items (for halving acid damage)
 * - power per point of damage
 * - power per point of +to_hit
 * - power per point of base AC
 * - power per point of +to_ac
 * (these four are all halved in the algorithm)
 * - assumed max blows
 * - inhibiting values for +blows/might/shots/immunities (max is one less)
 */
#define NONWEAP_DAMAGE   		15 /* fudge to boost extra blows */
#define WEAP_DAMAGE				12 /* and for off-weapon combat flags */
#define BASE_JEWELRY_POWER		 4
#define BASE_ARMOUR_POWER		 1
#define BASE_LIGHT_POWER		 6 /* for rad-2; doubled for rad-3 */
#define DAMAGE_POWER             5 /* i.e. 2.5 */
#define TO_HIT_POWER             3 /* i.e. 1.5 */
#define BASE_AC_POWER            2 /* i.e. 1 */
#define TO_AC_POWER              2 /* i.e. 1 */
#define MAX_BLOWS                5
#define INHIBIT_BLOWS            3
#define INHIBIT_MIGHT            4
#define INHIBIT_SHOTS            3
#define INHIBIT_IMMUNITIES       4

/**
 * Define a set of constants for dealing with launchers and ammo:
 * - the assumed average damage of ammo (for rating launchers)
 * (the current values assume normal (non-seeker) ammo enchanted to +9)
 * - the assumed bonus on launchers (for rating ego ammo)
 * - twice the assumed multiplier (for rating any ammo)
 * N.B. Ammo tvals are assumed to be consecutive! We access this array using
 * (o_ptr->tval - TV_SHOT) for ammo, and 
 * (o_ptr->sval / 10) for launchers
 */
static struct archery {
	int ammo_tval;
	int ammo_dam;
	int launch_dam;
	int launch_mult;
} archery[] = {
	 {TV_SHOT, 10, 9, 4},
	{TV_ARROW, 12, 9, 5},
	 {TV_BOLT, 14, 9, 7}
};

/**
 * Set the weightings of flag types:
 * - factor for power increment for multiple flags
 * - additional power bonus for a "full set" of these flags
 * - number of these flags which constitute a "full set"
 * - whether value is damage-dependent
 */
static struct set {
	int type;
	int factor;
	int bonus;
	int size;
	bool dam_dep;
	int count;
	const char *desc;
} sets[] = {
	{ OFT_SUST, 1, 10, 5, FALSE, 0, "sustains" },
	{ OFT_SLAY, 1, 10, 8, TRUE,  0, "normal slays" },
	{ OFT_BRAND, 2, 20, 5, TRUE,  0, "brands" },
	{ OFT_KILL, 3, 20, 3, TRUE,  0, "x5 slays" },
	{ OFT_IMM,  6, INHIBIT_POWER, 4, FALSE, 0, "immunities" },
	{ OFT_LRES, 1, 10, 4, FALSE, 0, "low resists" },
	{ OFT_HRES, 2, 10, 9, FALSE, 0, "high resists" },
	{ OFT_PROT, 3, 15, 4, FALSE, 0, "protections" },
	{ OFT_MISC, 1, 25, 8, FALSE, 0, "misc abilities" }
};

/**
 * Boost ratings for combinations of ability bonuses
 * We go up to +24 here - anything higher is inhibited
 * N.B. Not all stats count equally towards this total
 */
static s16b ability_power[25] =
	{0, 0, 0, 0, 0, 0, 0, 2, 4, 6, 8,
	12, 16, 20, 24, 30, 36, 42, 48, 56, 64,
	74, 84, 96, 110};

/**
 * Calculate the rating for a given slay combination
 */
static s32b slay_power(const object_type *o_ptr, int verbose, ang_file*
	log_file, bool known)
{
	bitflag s_index[OF_SIZE], f[OF_SIZE], f2[OF_SIZE];
	u32b sv = 0;
	int i, j;
	int mult;
	const struct slay *best_s_ptr = NULL;
	monster_race *r_ptr;
	monster_type *m_ptr;
	monster_type monster_type_body;
	const char *desc[SL_MAX] = { 0 }, *brand[SL_MAX] = { 0 };
	int s_mult[SL_MAX] = { 0 };

	if (known)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	/* Combine the slay bytes into an index value, return if there are none */
	of_copy(s_index, f);
	create_mask(f2, FALSE, OFT_SLAY, OFT_KILL, OFT_BRAND, OFT_MAX);

	if (!of_is_inter(s_index, f2))
		return tot_mon_power;
	else
		of_inter(s_index, f2);

	/* Look in the cache to see if we know this one yet */
	sv = check_slay_cache(s_index);

	/* If it's cached (or there are no slays), return the value */
	if (sv)	{
		file_putf(log_file, "Slay cache hit\n");
		return sv;
	}

	/*
	 * Otherwise we need to calculate the expected average multiplier
	 * for this combination (multiplied by the total number of
	 * monsters, which we'll divide out later).
	 */
	for (i = 0; i < z_info->r_max; i++)	{
		best_s_ptr = NULL;
		mult = 1;
		r_ptr = &r_info[i];
		m_ptr = &monster_type_body;
		m_ptr->r_idx = i;

		/* Find the best multiplier against this monster */
		improve_attack_modifier((object_type *)o_ptr, m_ptr, &best_s_ptr,
				FALSE, !known);
		if (best_s_ptr)
			mult = best_s_ptr->mult;

		/* Add the multiple to sv */
		sv += mult * r_ptr->scaled_power;
	}

	/*
	 * To get the expected damage for this weapon, multiply the
	 * average damage from base dice by sv, and divide by the
	 * total number of monsters.
	 */
	if (verbose) {
		/* Write info about the slay combination and multiplier */
		file_putf(log_file, "Slay multiplier for: ");

		j = list_slays(s_index, s_index, desc, brand, s_mult, FALSE);

		for (i = 0; i < j; i++) {
			if (brand[i]) {
				file_putf(log_file, brand[i]);
			} else {
				file_putf(log_file, desc[i]);
			}
			file_putf(log_file, "x%d ", s_mult[i]); 
		}
		file_putf(log_file, "\nsv is: %d\n", sv);
		file_putf(log_file, " and t_m_p is: %d \n", tot_mon_power);
		file_putf(log_file, "times 1000 is: %d\n", (1000 * sv) / tot_mon_power);
	}

	/* Add to the cache */
	if (fill_slay_cache(s_index, sv))
		file_putf(log_file, "Added to slay cache\n");

	return sv;
}

/*
 * Calculate the multiplier we'll get with a given bow type.
 * Note that this relies on the multiplier being the 2nd digit of the bow's
 * sval. We assume that sval has already been checked for legitimacy before
 * we get here.
 */
static int bow_multiplier(int sval)
{
	int mult = 0;
	mult = sval - 10 * (sval / 10);
	return mult;
}

/*
 * Evaluate the object's overall power level.
 */
s32b object_power(const object_type* o_ptr, int verbose, ang_file *log_file,
	bool known)
{
	s32b p = 0, q = 0, slay_pwr = 0, dice_pwr = 0;
	unsigned int i, j;
	int extra_stat_bonus = 0, mult = 1, num_slays = 0, k = 1;
	bitflag flags[OF_SIZE], mask[OF_SIZE];

	/* Zero the flag counts */
	for (i = 0; i < N_ELEMENTS(sets); i++)
		sets[i].count = 0;

	/* Extract the flags */
	if (known) {
		file_putf(log_file, "Object is deemed known\n");
		object_flags(o_ptr, flags);
	} else {
		file_putf(log_file, "Object may not be fully known\n");
		object_flags_known(o_ptr, flags);
	}

	/* Log the flags in human-readable form */
	if (verbose)
		log_flags(flags, log_file);

	/* Get the slay power and number of slay/brand types */
	create_mask(mask, FALSE, OFT_SLAY, OFT_KILL, OFT_BRAND, OFT_MAX);
	num_slays = list_slays(flags, mask, NULL, NULL, NULL, TRUE);
	if (num_slays)
		slay_pwr = slay_power(o_ptr, verbose, log_file, known);

	/* Start with any damage boost from the item itself */
	p += (o_ptr->to_d * DAMAGE_POWER / 2);
	file_putf(log_file, "Adding power from to_dam, total is %d\n", p);

	/* Add damage from dice for any wieldable weapon or ammo */
	if (wield_slot(o_ptr) == INVEN_WIELD || obj_is_ammo(o_ptr)) {
		dice_pwr = (o_ptr->dd * (o_ptr->ds + 1) * DAMAGE_POWER / 4);
		file_putf(log_file, "Adding %d power for dam dice\n", dice_pwr);
	/* Add 2nd lot of damage power for nonweapons */
	} else if (wield_slot(o_ptr) != INVEN_BOW) {
		p += (o_ptr->to_d * DAMAGE_POWER);
		file_putf(log_file, "Adding power from nonweap to_dam, total is %d\n", p);
		/* Add power boost for nonweapons with combat flags */
		if (num_slays || of_has(flags, OF_BLOWS) || of_has(flags, OF_SHOTS) ||
				of_has(flags, OF_MIGHT)) {
			dice_pwr = (WEAP_DAMAGE * DAMAGE_POWER);
			file_putf(log_file, "Adding %d power for nonweap combat flags\n", dice_pwr);
		}
	}
	p += dice_pwr;

	/* Add ammo damage for launchers, get multiplier and rescale */
	if (wield_slot(o_ptr) == INVEN_BOW) {
		p += (archery[o_ptr->sval / 10].ammo_dam * DAMAGE_POWER / 2);
		file_putf(log_file, "Adding power from ammo, total is %d\n", p);

		mult = bow_multiplier(o_ptr->sval);
		file_putf(log_file, "Base mult for this weapon is %d\n", mult);
	}

	/* Add launcher bonus for ego ammo, multiply for launcher and rescale */
	if (obj_is_ammo(o_ptr)) {
		if (o_ptr->ego)
			p += (archery[o_ptr->tval - TV_SHOT].launch_dam * DAMAGE_POWER / 2);
		p = p * archery[o_ptr->tval - TV_SHOT].launch_mult / (2 * MAX_BLOWS);
		file_putf(log_file, "After multiplying ammo and rescaling, power is %d\n", p);
	}

	/* Add power for extra blows */
	if (of_has(flags, OF_BLOWS)) {
		j = which_pval(o_ptr, OF_BLOWS);
		if (known || object_this_pval_is_visible(o_ptr, j)) {
			if (o_ptr->pval[j] >= INHIBIT_BLOWS) {
				p += INHIBIT_POWER;
				file_putf(log_file, "INHIBITING - too many extra blows - quitting\n");
				return p;
			} else {
				p = p * (MAX_BLOWS + o_ptr->pval[j]) / MAX_BLOWS;
				/* Add boost for assumed off-weapon damage */
				p += (NONWEAP_DAMAGE * o_ptr->pval[j] * DAMAGE_POWER / 2);
				file_putf(log_file, "Adding power for extra blows, total is %d\n", p);
			}
		}
	}

	/* Add power for extra shots - note that we cannot handle negative shots */
	if (of_has(flags, OF_SHOTS)) {
		j = which_pval(o_ptr, OF_SHOTS);
		if (known || object_this_pval_is_visible(o_ptr, j)) {
			if (o_ptr->pval[j] >= INHIBIT_SHOTS) {
				p += INHIBIT_POWER;
				file_putf(log_file, "INHIBITING - too many extra shots - quitting\n");
				return p;
			} else if (o_ptr->pval[j] > 0) {
				p = (p * (1 + o_ptr->pval[j]));
				file_putf(log_file, "Extra shots: multiplying power by 1 + %d, total is %d\n", o_ptr->pval[j], p);
			}
		}
	}

	/* Add power for extra might */
	if (of_has(flags, OF_MIGHT)) {
		j = which_pval(o_ptr, OF_MIGHT);
		if (known || object_this_pval_is_visible(o_ptr, j)) {
			if (o_ptr->pval[j] >= INHIBIT_MIGHT) {
				p += INHIBIT_POWER;
				mult = 1;	/* don't overflow */
				file_putf(log_file, "INHIBITING - too much extra might - quitting\n");
				return p;
			} else
				mult += o_ptr->pval[j];
			file_putf(log_file, "Mult after extra might is %d\n", mult);
		}
	}
	p *= mult;
	file_putf(log_file, "After multiplying power for might, total is %d\n", p);

	/* Apply the correct slay multiplier */
	if (slay_pwr) {
		p += (dice_pwr * (slay_pwr / 100)) / (tot_mon_power / 100);
		file_putf(log_file, "Adjusted for slay power, total is %d\n", p);
	}

	/* Melee weapons assume MAX_BLOWS per turn, so we must divide by MAX_BLOWS
     * to get equal ratings for launchers. */
	if (wield_slot(o_ptr) == INVEN_BOW) {
		p /= MAX_BLOWS;
		file_putf(log_file, "Rescaling bow power, total is %d\n", p);
	}

	/* Add power for +to_hit */
	p += (o_ptr->to_h * TO_HIT_POWER / 2);
	file_putf(log_file, "Adding power for to hit, total is %d\n", p);

	/* Add power for base AC and adjust for weight */
	if (o_ptr->ac) {
		p += BASE_ARMOUR_POWER;
		q += (o_ptr->ac * BASE_AC_POWER / 2);
		file_putf(log_file, "Adding %d power for base AC value\n", q);

		/* Add power for AC per unit weight */
		if (o_ptr->weight > 0) {
			i = 750 * (o_ptr->ac + o_ptr->to_a) / o_ptr->weight;

			/* Avoid overpricing Elven Cloaks */
			if (i > 450) i = 450;

			q *= i;
			q /= 100;

		/* Weightless (ethereal) armour items get fixed boost */
		} else
			q *= 5;
		p += q;
		file_putf(log_file, "Adding power for AC per unit weight, now %d\n", p);
	}
	/* Add power for +to_ac */
	p += (o_ptr->to_a * TO_AC_POWER / 2);
	file_putf(log_file, "Adding power for to_ac of %d, total is %d\n", o_ptr->to_a, p);
	if (o_ptr->to_a > HIGH_TO_AC) {
		p += ((o_ptr->to_a - (HIGH_TO_AC - 1)) * TO_AC_POWER);
		file_putf(log_file, "Adding power for high to_ac value, total is %d\n", p);
	}
	if (o_ptr->to_a > VERYHIGH_TO_AC) {
		p += ((o_ptr->to_a - (VERYHIGH_TO_AC -1)) * TO_AC_POWER * 2);
		file_putf(log_file, "Adding power for very high to_ac value, total is %d\n", p);
	}
	if (o_ptr->to_a >= INHIBIT_AC) {
		p += INHIBIT_POWER;
		file_putf(log_file, "INHIBITING: AC bonus too high\n");
	}

	/* Add power for light sources by radius XXX Hack - rewrite calc_torch! */
	if (wield_slot(o_ptr) == INVEN_LIGHT) {
		p += BASE_LIGHT_POWER;

		/* Artifact lights have larger radius so add more */
		if (o_ptr->artifact)
			p += BASE_LIGHT_POWER;

		file_putf(log_file, "Adding power for light radius, total is %d\n", p);
	}

	/* Add base power for jewelry */
	if (object_is_jewelry(o_ptr)) {
		p += BASE_JEWELRY_POWER;
		file_putf(log_file, "Adding power for jewelry, total is %d\n", p);
	}

	/* Add power for non-derived flags (derived flags have flag_power 0) */
	for (i = of_next(flags, FLAG_START); i != FLAG_END; i = of_next(flags, i + 1)) {
		if (flag_uses_pval(i)) {
			j = which_pval(o_ptr, i);
			if (known || object_this_pval_is_visible(o_ptr, j)) {
				k = o_ptr->pval[j];
				extra_stat_bonus += (k * pval_mult(i));
			}
		} else
			k = 1;

		if (flag_power(i)) {
			p += (k * flag_power(i) * slot_mult(i, wield_slot(o_ptr)));
			file_putf(log_file, "Adding power for %s, total is %d\n", flag_name(i), p);
		}

		/* Track combinations of flag types - note we ignore SUST_CHR */
		for (j = 0; j < N_ELEMENTS(sets); j++)
			if ((sets[j].type == obj_flag_type(i)) && (i != OF_SUST_CHR))
				sets[j].count++;
	}

	/* Add extra power term if there are a lot of ability bonuses */
	if (extra_stat_bonus > 249) {
		file_putf(log_file, "Inhibiting!  (Total ability bonus of %d is too high)\n", extra_stat_bonus);
		p += INHIBIT_POWER;
	} else {
		p += ability_power[extra_stat_bonus / 10];
		file_putf(log_file, "Adding power for pval total of %d, total is %d\n", extra_stat_bonus, p);
	}

	/* Add extra power for multiple flags of the same type */
	for (i = 0; i < N_ELEMENTS(sets); i++) {
		if (sets[i].count > 1) {
			q = (sets[i].factor * sets[i].count * sets[i].count);
			/* Scale damage-dependent set bonuses by damage dice power */
			if (sets[i].dam_dep)
				q = q * dice_pwr / (DAMAGE_POWER * 5);
			p += q;
			file_putf(log_file, "Adding power for multiple %s, total is %d\n", sets[i].desc, p);
		}

		/* Add bonus if item has a full set of these flags */
		if (sets[i].count == sets[i].size) {
			p += sets[i].bonus;
			file_putf(log_file, "Adding power for full set of %s, total is %d\n", sets[i].desc, p);
		}
	}

	/* add power for effect */
	if (known || object_effect_is_known(o_ptr))	{
		if (o_ptr->artifact && o_ptr->artifact->effect) {
			p += effect_power(o_ptr->artifact->effect);
			file_putf(log_file, "Adding power for artifact activation, total is %d\n", p);
		} else {
			p += effect_power(o_ptr->kind->effect);
			file_putf(log_file, "Adding power for item activation, total is %d\n", p);
		}
	}

	file_putf(log_file, "FINAL POWER IS %d\n", p);

	return p;
}
