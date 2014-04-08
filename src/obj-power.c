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
#include "obj-identify.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-tvalsval.h"
#include "obj-util.h"
#include "init.h"
#include "effects.h"
#include "mon-power.h"
#include "monster.h"

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
#define DAMAGE_POWER             5 /* i.e. 2.5 */
#define TO_HIT_POWER             3 /* i.e. 1.5 */
#define BASE_AC_POWER            2 /* i.e. 1 */
#define TO_AC_POWER              2 /* i.e. 1 */
#define MAX_BLOWS                5

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
	int count;
	const char *desc;
} sets[] = {
	{ OFT_SUST, 1, 10, 5, 0, "sustains" },
	{ OFT_IMM,  6, INHIBIT_POWER, 4, 0, "immunities" },
	{ OFT_LRES, 1, 10, 4, 0, "low resists" },
	{ OFT_HRES, 2, 10, 9, 0, "high resists" },
	{ OFT_PROT, 3, 15, 4, 0, "protections" },
	{ OFT_MISC, 1, 25, 8, 0, "misc abilities" }
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

/* Log file declared here for simplicity */
ang_file *object_log;

/* Log progress info to the object log */
void log_progress(char *message, int p)
{
	if (p) file_putf(object_log, message, p);
}

/*
 * Calculate the multiplier we'll get with a given bow type.
 */
static int bow_multiplier(const object_type *o_ptr, ang_file *log_file)
{
	int mult = 1;

	if (o_ptr->tval != TV_BOW) return mult;

	switch (o_ptr->sval) {
	case SV_SLING:
	case SV_SHORT_BOW:  mult = 2; break;
	case SV_LONG_BOW:
	case SV_LIGHT_XBOW: mult = 3; break;
	case SV_HEAVY_XBOW: mult = 4; break;
	}
	log_progress("Base mult for this weapon is %d\n", mult);
	return mult;
}

/* To damage power */
static int to_damage_power(const object_type *o_ptr, ang_file *log_file)
{
	int p;

	p = (o_ptr->to_d * DAMAGE_POWER / 2);
	log_progress("Adding power from to_dam, total is %d\n", p);

	/* Add second lot of damage power for non-weapons */
	if ((wield_slot(o_ptr) != INVEN_BOW) &&
		(wield_slot(o_ptr) != INVEN_WIELD) &&
		!tval_is_ammo(o_ptr)) {
		p += (o_ptr->to_d * DAMAGE_POWER);
		log_progress("Adding power from non-weapon to_dam, total is %d\n", p);
	}
	return p;
}

/* Damage dice power or equivalent */
static int damage_dice_power(const object_type *o_ptr, ang_file *log_file)
{
	int dice = 0;

	/* Add damage from dice for any wieldable weapon or ammo */
	if (wield_slot(o_ptr) == INVEN_WIELD || tval_is_ammo(o_ptr)) {
		dice = (o_ptr->dd * (o_ptr->ds + 1) * DAMAGE_POWER / 4);
		log_progress("Adding %d power for damage dice\n", dice);
	} else if (wield_slot(o_ptr) != INVEN_BOW) {
		/* Add power boost for nonweapons with combat flags */
		if (o_ptr->brands || o_ptr->slays || 
			(o_ptr->modifiers[OBJ_MOD_BLOWS] > 0) ||
			(o_ptr->modifiers[OBJ_MOD_SHOTS] > 0) ||
			(o_ptr->modifiers[OBJ_MOD_MIGHT] > 0)) {
			dice = (WEAP_DAMAGE * DAMAGE_POWER);
			log_progress("Adding %d power for non-weapon combat flags\n", dice);
		}
	}
	return dice;
}

/* Add ammo damage for launchers, get multiplier and rescale */
static int ammo_damage_power(const object_type *o_ptr, int p, 
							 ang_file *log_file)
{
	int q = 0;
	int launcher;

	if (wield_slot(o_ptr) == INVEN_BOW) {
		if (o_ptr->sval == SV_SLING) launcher = 0;
		else if ((o_ptr->sval == SV_SHORT_BOW) ||
				 (o_ptr->sval == SV_LONG_BOW)) launcher = 1; 
		else if ((o_ptr->sval == SV_LIGHT_XBOW) ||
				 (o_ptr->sval == SV_HEAVY_XBOW)) launcher = 2;

		q = (archery[launcher].ammo_dam * DAMAGE_POWER / 2);
		log_progress("Adding power from ammo, total is %d\n", p + q);
	}
	return q;
}

/* Add launcher bonus for ego ammo, multiply for launcher and rescale */
static int launcher_ammo_damage_power(const object_type *o_ptr, int p, 
									  ang_file *log_file)
{
	int ammo_type = 0;

	if (tval_is_ammo(o_ptr)) {
		if (o_ptr->tval == TV_ARROW) ammo_type = 1;
		if (o_ptr->tval == TV_BOLT) ammo_type = 2;
		if (o_ptr->ego)
			p += (archery[ammo_type].launch_dam * DAMAGE_POWER / 2);
		p = p * archery[ammo_type].launch_mult / (2 * MAX_BLOWS);
		log_progress("After multiplying ammo and rescaling, power is %d\n", p);
	}
	return p;
}

/* Add power for extra blows */
static int extra_blows_power(const object_type *o_ptr, int p, 
							 ang_file *log_file, bool known)
{
	if (known || object_this_mod_is_visible(o_ptr, OBJ_MOD_BLOWS)) {
		if (o_ptr->modifiers[OBJ_MOD_BLOWS] >= INHIBIT_BLOWS) {
			p += INHIBIT_POWER;
			file_putf(log_file, 
					  "INHIBITING - too many extra blows - quitting\n");
			return p;
		} else {
			p = p * (MAX_BLOWS + o_ptr->modifiers[OBJ_MOD_BLOWS]) / MAX_BLOWS;
			/* Add boost for assumed off-weapon damage */
			p += (NONWEAP_DAMAGE * o_ptr->modifiers[OBJ_MOD_BLOWS] 
				  * DAMAGE_POWER / 2);
			log_progress( "Adding power for extra blows, total is %d\n", p);
		}
	}
	return p;
}

/* Add power for extra shots - note that we cannot handle negative shots */
static int extra_shots_power(const object_type *o_ptr, int p, 
							 ang_file *log_file, bool known)
{
	if (known || object_this_mod_is_visible(o_ptr, OBJ_MOD_SHOTS)) {
		if (o_ptr->modifiers[OBJ_MOD_SHOTS] >= INHIBIT_SHOTS) {
			p += INHIBIT_POWER;
			file_putf(log_file, "INHIBITING - too many extra shots - quitting\n");
			return p;
		} else if (o_ptr->modifiers[OBJ_MOD_SHOTS] > 0) {
			p = (p * (1 + o_ptr->modifiers[OBJ_MOD_SHOTS]));
			file_putf(log_file, 
					  "Extra shots: multiplying power by 1 + %d, total is %d\n",
					  o_ptr->modifiers[OBJ_MOD_SHOTS], p);
		}
	}
	return p;
}


/* Add power for extra might */
static int extra_might_power(const object_type *o_ptr, int p, 
							 ang_file *log_file, int mult, bool known)
{
	if (known || object_this_mod_is_visible(o_ptr, OBJ_MOD_MIGHT)) {
		if (o_ptr->modifiers[OBJ_MOD_MIGHT] >= INHIBIT_MIGHT) {
			p += INHIBIT_POWER;
			file_putf(log_file, 
					  "INHIBITING - too much extra might - quitting\n");
			return p;
		} else {
			mult += o_ptr->modifiers[OBJ_MOD_MIGHT];
		}
		log_progress("Mult after extra might is %d\n", mult);
	}
	p *= mult;
	log_progress("After multiplying power for might, total is %d\n", p);
	return p;
}

/**
 * Calculate the rating for a given slay combination
 */
static s32b slay_power(const object_type *o_ptr, int p, int verbose, 
					   int dice_pwr, ang_file* log_file, bool known)
{
	bitflag s_index[OF_SIZE], f[OF_SIZE], f2[OF_SIZE];
	u32b sv = 0;
	int i, j, num_brands = 0, num_slays = 0, num_kills = 0;
	int mult;
	monster_type *m_ptr;
	monster_type monster_type_body;
	const char *desc[SL_MAX] = { 0 }, *brand[SL_MAX] = { 0 };
	int s_mult[SL_MAX] = { 0 }, slay_list[SL_MAX] = { 0 };
	struct brand *brands = o_ptr->brands;
	struct new_slay *slays = o_ptr->slays;

	/* Count the known brands and slays */
	while (brands) {
		if (known || brands->known)
			num_brands++;
		brands = brands->next;
	}
	while (slays) {
		if (known || slays->known) {
			if (slays->multiplier <= 3)
				num_slays++;
			else
				num_kills++;
		}
		slays = slays->next;
	}

	if (known)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	/* Combine the slay bytes into an index value, return if there are none */
	of_copy(s_index, f);
	create_mask(f2, FALSE, OFT_SLAY, OFT_KILL, OFT_BRAND, OFT_MAX);

	if (!of_is_inter(s_index, f2))
		return p;
	else
		of_inter(s_index, f2);

	/* Look in the cache to see if we know this one yet */
	sv = check_slay_cache(s_index);

	/* If it's cached (or there are no slays), return the value */
	if (sv)	{
		file_putf(log_file, "Slay cache hit\n");
	} else {

		/*
		 * Otherwise we need to calculate the expected average multiplier
		 * for this combination (multiplied by the total number of
		 * monsters, which we'll divide out later).
		 */
		for (i = 0; i < z_info->r_max; i++)	{
			const struct brand *b = NULL;
			const struct new_slay *s = NULL;
			char **verb;
			verb = mem_zalloc(20 * sizeof(char));

			mult = 1;
			m_ptr = &monster_type_body;
			m_ptr->race = &r_info[i];

			/* Find the best multiplier against this monster */
			improve_attack_modifier((object_type *)o_ptr, m_ptr, &b, &s, 
									(char **) &verb, FALSE, !known);
			if (s)
				mult = s->multiplier;
			else if (b)
				mult = b->multiplier;

			/* Add the multiple to sv */
			sv += mult * m_ptr->race->scaled_power;
		}

		/*
		 * To get the expected damage for this weapon, multiply the
		 * average damage from base dice by sv, and divide by the
		 * total number of monsters.
		 */
		if (verbose) {
			/* Write info about the slay combination and multiplier */
			file_putf(log_file, "Slay multiplier for: ");

			j = list_slays(s_index, s_index, slay_list, FALSE);
			slay_info_collect(slay_list, desc, brand, s_mult, j);

			for (i = 0; i < j; i++) {
				if (brand[i]) {
					file_putf(log_file, brand[i]);
				} else {
					file_putf(log_file, desc[i]);
				}
				log_progress("x%d ", s_mult[i]); 
			}
			log_progress("\nsv is: %d\n", sv);
			log_progress(" and t_m_p is: %d \n", tot_mon_power);
			log_progress("times 1000 is: %d\n", 
					  (1000 * sv) / tot_mon_power);
		}

		/* Add to the cache */
		if (fill_slay_cache(s_index, sv))
			file_putf(log_file, "Added to slay cache\n");
	}

	p += (dice_pwr * (sv / 100)) / (tot_mon_power / 100);
	log_progress("Adjusted for slay power, total is %d\n", p);

	/* Bonuses for multiple brands and slays */
	if (num_slays > 1) {
		int q = (num_slays * num_slays * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_progress("Adding power for multiple slays, total is %d\n", p);
	}
	if (num_brands > 1) {
		int q = (2 * num_brands * num_brands * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_progress("Adding power for multiple brands, total is %d\n", p);
	}
	if (num_kills > 1) {
		int q = (3 * num_kills * num_kills * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_progress("Adding power for multiple kills, total is %d\n", p);
	}
	if (num_slays == 8) {
		p += 10;
		log_progress("Adding power for full set of slays, total is %d\n", p);
	}
	if (num_brands == 5) {
		p += 20;
		log_progress("Adding power for full set of brands, total is %d\n", p);
	}
	if (num_kills == 3) {
		p += 20;
		log_progress("Adding power for full set of kills, total is %d\n", p);
	}

	return p;
}

/* Melee weapons assume MAX_BLOWS per turn, so we must divide by MAX_BLOWS
 * to get equal ratings for launchers. */
static int rescale_bow_power(const object_type *o_ptr, int p, 
							 ang_file *log_file)
{
	if (wield_slot(o_ptr) == INVEN_BOW) {
		p /= MAX_BLOWS;
		log_progress("Rescaling bow power, total is %d\n", p);
	}
	return p;
}

/* Add power for +to_hit */
static int to_hit_power(const object_type *o_ptr, int p, ang_file *log_file)
{
	p += (o_ptr->to_h * TO_HIT_POWER / 2);
	log_progress("Adding power for to hit, total is %d\n", p);
	return p;
}

/* Add power for base AC and adjust for weight */
static int ac_power(const object_type *o_ptr, int p, ang_file *log_file)
{
	int q = 0;

	if (o_ptr->ac) {
		p += BASE_ARMOUR_POWER;
		q += (o_ptr->ac * BASE_AC_POWER / 2);
		log_progress("Adding %d power for base AC value\n", q);

		/* Add power for AC per unit weight */
		if (o_ptr->weight > 0) {
			int i = 750 * (o_ptr->ac + o_ptr->to_a) / o_ptr->weight;

			/* Avoid overpricing Elven Cloaks */
			if (i > 450) i = 450;

			q *= i;
			q /= 100;

			/* Weightless (ethereal) armour items get fixed boost */
		} else
			q *= 5;
		p += q;
		log_progress("Adding power for AC per unit weight, now %d\n", p);
	}
	return p;
}


/* Add power for +to_ac */
static int to_ac_power(const object_type *o_ptr, int p, ang_file *log_file)
{
	p += (o_ptr->to_a * TO_AC_POWER / 2);
	file_putf(log_file, 
			  "Adding power for to_ac of %d, total is %d\n", o_ptr->to_a, p);
	if (o_ptr->to_a > HIGH_TO_AC) {
		p += ((o_ptr->to_a - (HIGH_TO_AC - 1)) * TO_AC_POWER);
		log_progress("Adding power for high to_ac value, total is %d\n", p);
	}
	if (o_ptr->to_a > VERYHIGH_TO_AC) {
		p += ((o_ptr->to_a - (VERYHIGH_TO_AC -1)) * TO_AC_POWER * 2);
		log_progress("Adding power for very high to_ac value, total is %d\n", 
					 p);
	}
	if (o_ptr->to_a >= INHIBIT_AC) {
		p += INHIBIT_POWER;
		file_putf(log_file, "INHIBITING: AC bonus too high\n");
	}
	return p;
}

/* Add base power for jewelry */
static int jewelry_power(const object_type *o_ptr, int p, ang_file *log_file)
{
	if (tval_is_jewelry(o_ptr)) {
		p += BASE_JEWELRY_POWER;
		log_progress("Adding power for jewelry, total is %d\n", p);
	}
	return p;
}

/* Add power for modifiers */
static int modifier_power(const object_type *o_ptr, int p, ang_file *log_file,
						  bool known)
{
	int i, k = 1, extra_stat_bonus = 0;

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (known || object_this_mod_is_visible(o_ptr, i)) {
			k = o_ptr->modifiers[i];
			extra_stat_bonus += (k * mod_mult(i));
		}

		if (mod_power(i)) {
			p += (k * mod_power(i) * mod_slot_mult(i, wield_slot(o_ptr)));
			file_putf(log_file, "Adding power for %s, total is %d\n", 
					  mod_name(i), p);
		}
	}

	/* Add extra power term if there are a lot of ability bonuses */
	if (extra_stat_bonus > 249) {
		log_progress("Inhibiting!  (Total ability bonus of %d is too high)\n", 
					 extra_stat_bonus);
		p += INHIBIT_POWER;
	} else if (extra_stat_bonus > 0) {
		p += ability_power[extra_stat_bonus / 10];
		file_putf(log_file, 
				  "Adding power for modifier total of %d, total is %d\n", 
				  extra_stat_bonus, p);
	}
	return p;
}

/* Add power for non-derived flags (derived flags have flag_power 0) */
static int flags_power(const object_type *o_ptr, int p, ang_file *log_file,
					   int verbose,  bool known)
{
	size_t i, j;
	int q;
	bitflag flags[OF_SIZE];

	/* Extract the flags */
	if (known) {
		object_flags(o_ptr, flags);
	} else {
		object_flags_known(o_ptr, flags);
	}

	/* Log the flags in human-readable form */
	if (verbose)
		log_flags(flags, log_file);

	/* Zero the flag counts */
	for (i = 0; i < N_ELEMENTS(sets); i++)
		sets[i].count = 0;

	for (i = of_next(flags, FLAG_START); i != FLAG_END; 
		 i = of_next(flags, i + 1)) {
		if (flag_power(i)) {
			p += (flag_power(i) * flag_slot_mult(i, wield_slot(o_ptr)));
			file_putf(log_file, "Adding power for %s, total is %d\n", 
					  flag_name(i), p);
		}

		/* Track combinations of flag types */
		for (j = 0; j < N_ELEMENTS(sets); j++)
			if (sets[j].type == obj_flag_type(i))
				sets[j].count++;
	}

	/* Add extra power for multiple flags of the same type */
	for (i = 0; i < N_ELEMENTS(sets); i++) {
		if (sets[i].count > 1) {
			q = (sets[i].factor * sets[i].count * sets[i].count);
			p += q;
			file_putf(log_file, "Adding power for multiple %s, total is %d\n",
					  sets[i].desc, p);
		}

		/* Add bonus if item has a full set of these flags */
		if (sets[i].count == sets[i].size) {
			p += sets[i].bonus;
			file_putf(log_file, 
					  "Adding power for full set of %s, total is %d\n", 
					  sets[i].desc, p);
		}
	}

	return p;
}

/* add power for effect */
static int effects_power(const object_type *o_ptr, int p, ang_file *log_file,
					   bool known)
{
	if (known || object_effect_is_known(o_ptr))	{
		if (o_ptr->artifact && o_ptr->artifact->effect) {
			p += effect_power(o_ptr->artifact->effect);
			log_progress(
					  "Adding power for artifact activation, total is %d\n", p);
		} else {
			p += effect_power(o_ptr->kind->effect);
			log_progress("Adding power for item activation, total is %d\n", p);
		}
	}
	return p;
}

/*
 * Evaluate the object's overall power level.
 */
s32b object_power(const object_type* o_ptr, int verbose, ang_file *log_file,
				  bool known)
{
	s32b p = 0, dice_pwr = 0;
	int mult = 1;

	/* Set the log file */
	object_log = object_log;

	/* Known status */
	if (known)
		file_putf(object_log, "Object is deemed known\n");
	else
		file_putf(object_log, "Object may not be fully known\n");

	/* Get all the attack power */
	p = to_damage_power(o_ptr, object_log);

	dice_pwr = damage_dice_power(o_ptr, object_log);
	p += dice_pwr;

	p += ammo_damage_power(o_ptr, p, object_log);

	mult = bow_multiplier(o_ptr, object_log);

	p = launcher_ammo_damage_power(o_ptr, p, object_log);

	p = extra_blows_power(o_ptr, p, object_log, known);
	if (p > INHIBIT_POWER) return p;

	p = extra_shots_power(o_ptr, p, object_log, known);
	if (p > INHIBIT_POWER) return p;

	p = extra_might_power(o_ptr, p, object_log, mult, known);
	if (p > INHIBIT_POWER) return p;

	p = slay_power(o_ptr, p, verbose, dice_pwr, object_log, known);

	p = rescale_bow_power(o_ptr, p, object_log);

	p = to_hit_power(o_ptr, p, object_log);

	/* Armour class power */
	p = ac_power(o_ptr, p, object_log);

	p = to_ac_power(o_ptr, p, object_log);

	/* Bonus for jewelry */
	p = jewelry_power(o_ptr, p, object_log);

	/* Other object properties */
	p = modifier_power(o_ptr, p, object_log, known);

	p = flags_power(o_ptr, p, object_log, verbose, known);

	p = effects_power(o_ptr, p, object_log, known);

	file_putf(object_log, "FINAL POWER IS %d\n", p);

	return p;
}
