/**
   \file obj-power.c
   \brief calculation of object power and value
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
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-power.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "init.h"
#include "effects.h"
#include "mon-power.h"
#include "monster.h"

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
 */
static struct flag_set {
	int type;
	int factor;
	int bonus;
	int size;
	int count;
	const char *desc;
} flag_sets[] = {
	{ OFT_SUST, 1, 10, 5, 0, "sustains" },
	{ OFT_PROT, 3, 15, 4, 0, "protections" },
	{ OFT_MISC, 1, 25, 8, 0, "misc abilities" }
};


enum {
	T_LRES,
	T_HRES
};

/**
 * Similar data for elements
 */
static struct element_set {
	int type;
	int res_level;
	int factor;
	int bonus;
	int size;
	int count;
	const char *desc;
} element_sets[] = {
	{ T_LRES, 3, 6, INHIBIT_POWER, 4,    0,     "immunities" },
	{ T_LRES, 1, 1, 10,            4,    0,     "low resists" },
	{ T_HRES, 1, 2, 10,            9,    0,     "high resists" },
};

/**
 * Power data for elements
 */
static struct element_powers {
	const char *name;
	int type;
	int ignore_power;
	int vuln_power;
	int res_power;
	int im_power;
} el_powers[] = {
	{ "acid",			T_LRES,	3,	-6,	5,	38 },
	{ "electricity",	T_LRES,	1,	-6,	6,	35 },
	{ "fire",			T_LRES,	3,	-6,	6,	40 },
	{ "cold",			T_LRES,	1,	-6,	6,	37 },
	{ "poison",			T_HRES,	0,	0,	28,	0 },
	{ "light",			T_HRES,	0,	0,	6,	0 },
	{ "dark",			T_HRES,	0,	0,	16,	0 },
	{ "sound",			T_HRES,	0,	0,	14,	0 },
	{ "shards",			T_HRES,	0,	0,	8,	0 },
	{ "nexus",			T_HRES,	0,	0,	15,	0 },
	{ "nether",			T_HRES,	0,	0,	20,	0 },
	{ "chaos",			T_HRES,	0,	0,	20,	0 },
	{ "disenchantment",	T_HRES,	0,	0,	20,	0 }
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
void log_obj(char *message)
{
	file_putf(object_log, message);
}

/*
 * Calculate the multiplier we'll get with a given bow type.
 */
static int bow_multiplier(const object_type *o_ptr)
{
	int mult = 1;

	if (o_ptr->tval != TV_BOW)
		return mult;
	else
		mult = o_ptr->pval;

	log_obj(format("Base mult for this weapon is %d\n", mult));
	return mult;
}

/* To damage power */
static int to_damage_power(const object_type *o_ptr)
{
	int p;

	p = (o_ptr->to_d * DAMAGE_POWER / 2);
	if (p) log_obj(format("%d power from to_dam\n", p));

	/* Add second lot of damage power for non-weapons */
	if ((wield_slot(o_ptr) != slot_by_name(player, "bow")) &&
		!tval_is_melee_weapon(o_ptr) &&
		!tval_is_ammo(o_ptr)) {
		int q = (o_ptr->to_d * DAMAGE_POWER);
		p += q;
		if (q)
			log_obj(format("Add %d from non-weapon to_dam, total %d\n", q, p));
	}
	return p;
}

/* Damage dice power or equivalent */
static int damage_dice_power(const object_type *o_ptr)
{
	int dice = 0;

	/* Add damage from dice for any wieldable weapon or ammo */
	if (tval_is_melee_weapon(o_ptr) || tval_is_ammo(o_ptr)) {
		dice = (o_ptr->dd * (o_ptr->ds + 1) * DAMAGE_POWER / 4);
		log_obj(format("Add %d power for damage dice, ", dice));
	} else if (wield_slot(o_ptr) != slot_by_name(player, "bow")) {
		/* Add power boost for nonweapons with combat flags */
		if (o_ptr->brands || o_ptr->slays || 
			(o_ptr->modifiers[OBJ_MOD_BLOWS] > 0) ||
			(o_ptr->modifiers[OBJ_MOD_SHOTS] > 0) ||
			(o_ptr->modifiers[OBJ_MOD_MIGHT] > 0)) {
			dice = (WEAP_DAMAGE * DAMAGE_POWER);
			log_obj(format("Add %d power for non-weapon combat bonuses, ",
						   dice));
		}
	}
	return dice;
}

/* Add ammo damage for launchers, get multiplier and rescale */
static int ammo_damage_power(const object_type *o_ptr, int p)
{
	int q = 0;
	int launcher = -1;

	if (wield_slot(o_ptr) == slot_by_name(player, "bow")) {
		if (kf_has(o_ptr->kind->kind_flags, KF_SHOOTS_SHOTS))
			launcher = 0;
		else if (kf_has(o_ptr->kind->kind_flags, KF_SHOOTS_ARROWS))
			launcher = 1; 
		else if (kf_has(o_ptr->kind->kind_flags, KF_SHOOTS_BOLTS))
			launcher = 2;

		if (launcher != -1) {
			q = (archery[launcher].ammo_dam * DAMAGE_POWER / 2);
			log_obj(format("Adding %d power from ammo, total is %d\n", q, p + q));			
		}
	}
	return q;
}

/* Add launcher bonus for ego ammo, multiply for launcher and rescale */
static int launcher_ammo_damage_power(const object_type *o_ptr, int p)
{
	int ammo_type = 0;

	if (tval_is_ammo(o_ptr)) {
		if (o_ptr->tval == TV_ARROW) ammo_type = 1;
		if (o_ptr->tval == TV_BOLT) ammo_type = 2;
		if (o_ptr->ego)
			p += (archery[ammo_type].launch_dam * DAMAGE_POWER / 2);
		p = p * archery[ammo_type].launch_mult / (2 * MAX_BLOWS);
		log_obj(format("After multiplying ammo and rescaling, power is %d\n",
					   p));
	}
	return p;
}

/* Add power for extra blows */
static int extra_blows_power(const object_type *o_ptr, int p, bool known)
{
	int q = p;

	if (o_ptr->modifiers[OBJ_MOD_BLOWS] == 0)
		return p;

	if (known || object_this_mod_is_visible(o_ptr, OBJ_MOD_BLOWS)) {
		if (o_ptr->modifiers[OBJ_MOD_BLOWS] >= INHIBIT_BLOWS) {
			p += INHIBIT_POWER;
			log_obj("INHIBITING - too many extra blows - quitting\n");
			return p;
		} else {
			p = p * (MAX_BLOWS + o_ptr->modifiers[OBJ_MOD_BLOWS]) / MAX_BLOWS;
			/* Add boost for assumed off-weapon damage */
			p += (NONWEAP_DAMAGE * o_ptr->modifiers[OBJ_MOD_BLOWS] 
				  * DAMAGE_POWER / 2);
			log_obj(format("Add %d power for extra blows, total is %d\n", 
								p - q, p));
		}
	}
	return p;
}

/* Add power for extra shots - note that we cannot handle negative shots */
static int extra_shots_power(const object_type *o_ptr, int p, bool known)
{
	if (o_ptr->modifiers[OBJ_MOD_SHOTS] == 0)
		return p;

	if (known || object_this_mod_is_visible(o_ptr, OBJ_MOD_SHOTS)) {
		if (o_ptr->modifiers[OBJ_MOD_SHOTS] >= INHIBIT_SHOTS) {
			p += INHIBIT_POWER;
			log_obj("INHIBITING - too many extra shots - quitting\n");
			return p;
		} else if (o_ptr->modifiers[OBJ_MOD_SHOTS] > 0) {
			int q = o_ptr->modifiers[OBJ_MOD_SHOTS];
			p += q;
			log_obj(format("Add %d power for extra shots, total is %d\n",q, p));
		}
	}
	return p;
}


/* Add power for extra might */
static int extra_might_power(const object_type *o_ptr, int p, int mult,
							 bool known)
{
	if (o_ptr->modifiers[OBJ_MOD_MIGHT] == 0)
		return p;

	if (known || object_this_mod_is_visible(o_ptr, OBJ_MOD_MIGHT)) {
		if (o_ptr->modifiers[OBJ_MOD_MIGHT] >= INHIBIT_MIGHT) {
			p += INHIBIT_POWER;
			log_obj("INHIBITING - too much extra might - quitting\n");
			return p;
		} else {
			mult += o_ptr->modifiers[OBJ_MOD_MIGHT];
		}
		log_obj(format("Mult after extra might is %d\n", mult));
	}
	p *= mult;
	log_obj(format("After multiplying power for might, total is %d\n", p));
	return p;
}

/**
 * Calculate the rating for a given slay combination
 */
static s32b slay_power(const object_type *o_ptr, int p, int verbose, 
					   int dice_pwr, bool known)
{
	u32b sv = 0;
	int i, q, num_brands = 0, num_slays = 0, num_kills = 0;
	int mult;
	struct brand *brands = o_ptr->brands;
	struct slay *slays = o_ptr->slays;

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

	/* Look in the cache to see if we know this one yet */
	sv = check_slay_cache(o_ptr);

	/* If it's cached (or there are no slays), return the value */
	if (sv)	{
		log_obj("Slay cache hit\n");
	} else {

		/*
		 * Otherwise we need to calculate the expected average multiplier
		 * for this combination (multiplied by the total number of
		 * monsters, which we'll divide out later).
		 */
		for (i = 0; i < z_info->r_max; i++)	{
			monster_type *m_ptr;
			monster_type monster_type_body;
			const struct brand *b = NULL;
			const struct slay *s = NULL;
			char verb[20];
			//verb = mem_zalloc(20 * sizeof(char));

			mult = 1;
			m_ptr = &monster_type_body;
			m_ptr->race = &r_info[i];

			/* Find the best multiplier against this monster */
			improve_attack_modifier((object_type *)o_ptr, m_ptr, &b, &s, 
									verb, FALSE, !known);
			if (s)
				mult = s->multiplier;
			else if (b)
				mult = b->multiplier;

			/* Add the multiple to sv */
			sv += mult * m_ptr->race->scaled_power;
			//mem_free(verb);
		}

		/*
		 * To get the expected damage for this weapon, multiply the
		 * average damage from base dice by sv, and divide by the
		 * total number of monsters.
		 */
		if (verbose) {
			struct brand *brands = NULL;
			struct slay *slays = NULL;
			int num_slays;
			int num_brands;

			/* Write info about the slay combination and multiplier */
			log_obj("Slay multiplier for: ");

			brands = brand_collect(o_ptr, NULL, &num_brands, !known);
			slays = slay_collect(o_ptr, NULL, &num_slays, !known);

			for (i = 0; i < num_brands; i++) {
				log_obj(format("%sx%d ", brands[i].name,brands[i].multiplier)); 
			}
			for (i = 0; i < num_slays; i++) {
				log_obj(format("%sx%d ", slays[i].name, slays[i].multiplier)); 
			}
			log_obj(format("\nsv is: %d\n", sv));
			log_obj(format(" and t_m_p is: %d \n", tot_mon_power));
			log_obj(format("times 1000 is: %d\n", (1000 * sv) / tot_mon_power));
			if (brands) mem_free(brands);
			if (slays) mem_free(slays);
		}

		/* Add to the cache */
		if (fill_slay_cache(o_ptr, sv))
			log_obj("Added to slay cache\n");
	}

	q = (dice_pwr * (sv / 100)) / (tot_mon_power / 100);
	p += q;
	log_obj(format("Add %d for slay power, total is %d\n", q, p));

	/* Bonuses for multiple brands and slays */
	if (num_slays > 1) {
		q = (num_slays * num_slays * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_obj(format("Add %d power for multiple slays, total is %d\n", q, p));
	}
	if (num_brands > 1) {
		q = (2 * num_brands * num_brands * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_obj(format("Add %d power for multiple brands, total is %d\n",q, p));
	}
	if (num_kills > 1) {
		q = (3 * num_kills * num_kills * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_obj(format("Add %d power for multiple kills, total is %d\n", q, p));
	}
	if (num_slays == 8) {
		p += 10;
		log_obj(format("Add 10 power for full set of slays, total is %d\n", p));
	}
	if (num_brands == 5) {
		p += 20;
		log_obj(format("Add 20 power for full set of brands, total is %d\n",p));
	}
	if (num_kills == 3) {
		p += 20;
		log_obj(format("Add 20 power for full set of kills, total is %d\n", p));
	}

	return p;
}

/* Melee weapons assume MAX_BLOWS per turn, so we must divide by MAX_BLOWS
 * to get equal ratings for launchers. */
static int rescale_bow_power(const object_type *o_ptr, int p)
{
	if (wield_slot(o_ptr) == slot_by_name(player, "bow")) {
		p /= MAX_BLOWS;
		log_obj(format("Rescaling bow power, total is %d\n", p));
	}
	return p;
}

/* Add power for +to_hit */
static int to_hit_power(const object_type *o_ptr, int p)
{
	int q = (o_ptr->to_h * TO_HIT_POWER / 2);
	p += q;
	if (p) 
		log_obj(format("Add %d power for to hit, total is %d\n", q, p));
	return p;
}

/* Add power for base AC and adjust for weight */
static int ac_power(const object_type *o_ptr, int p)
{
	int q = 0;

	if (o_ptr->ac) {
		p += BASE_ARMOUR_POWER;
		q += (o_ptr->ac * BASE_AC_POWER / 2);
		log_obj(format("Adding %d power for base AC value\n", q));

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
		log_obj(format("Add %d power for AC per unit weight, now %d\n",	q, p));
	}
	return p;
}


/* Add power for +to_ac */
static int to_ac_power(const object_type *o_ptr, int p)
{
	int q;

	if (o_ptr->to_a == 0) return p;

	q = (o_ptr->to_a * TO_AC_POWER / 2);
	p += q;
	log_obj(format("Add %d power for to_ac of %d, total is %d\n", 
				   q, o_ptr->to_a, p));
	if (o_ptr->to_a > HIGH_TO_AC) {
		q = ((o_ptr->to_a - (HIGH_TO_AC - 1)) * TO_AC_POWER);
		p += q;
		log_obj(format("Add %d power for high to_ac, total is %d\n",
							q, p));
	}
	if (o_ptr->to_a > VERYHIGH_TO_AC) {
		q = ((o_ptr->to_a - (VERYHIGH_TO_AC -1)) * TO_AC_POWER * 2);
		p += q;
		log_obj(format("Add %d power for very high to_ac, total is %d\n",q, p));
	}
	if (o_ptr->to_a >= INHIBIT_AC) {
		p += INHIBIT_POWER;
		log_obj("INHIBITING: AC bonus too high\n");
	}
	return p;
}

/* Add base power for jewelry */
static int jewelry_power(const object_type *o_ptr, int p)
{
	if (tval_is_jewelry(o_ptr)) {
		p += BASE_JEWELRY_POWER;
		log_obj(format("Adding %d power for jewelry, total is %d\n", 
					   BASE_JEWELRY_POWER, p));
	}
	return p;
}

/* Add power for modifiers */
static int modifier_power(const object_type *o_ptr, int p, bool known)
{
	int i, k = 1, extra_stat_bonus = 0, q;

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (known || object_this_mod_is_visible(o_ptr, i)) {
			k = o_ptr->modifiers[i];
			extra_stat_bonus += (k * mod_mult(i));
		}
		else continue;

		if (mod_power(i)) {
			q = (k * mod_power(i) * mod_slot_mult(i, wield_slot(o_ptr)));
			p += q;
			if (q) log_obj(format("Add %d power for %d %s, total is %d\n", 
								  q, k, mod_name(i), p));
		}
	}

	/* Add extra power term if there are a lot of ability bonuses */
	if (extra_stat_bonus > 249) {
		log_obj(format("Inhibiting - Total ability bonus of %d is too high\n", 
					   extra_stat_bonus));
		p += INHIBIT_POWER;
	} else if (extra_stat_bonus > 0) {
		q = ability_power[extra_stat_bonus / 10];
		if (!q) return p;
		p += q;
		log_obj(format("Add %d power for modifier total of %d, total is %d\n", 
					   q, extra_stat_bonus, p));
	}
	return p;
}

/* Add power for non-derived flags (derived flags have flag_power 0) */
static int flags_power(const object_type *o_ptr, int p, int verbose, 
					   ang_file *log_file, bool known)
{
	size_t i, j;
	int q;
	bitflag flags[OF_SIZE];

	/* Extract the flags */
	if (known)
		object_flags(o_ptr, flags);
	else
		object_flags_known(o_ptr, flags);

	/* Log the flags in human-readable form */
	if (verbose)
		log_flags(flags, log_file);

	/* Zero the flag counts */
	for (i = 0; i < N_ELEMENTS(flag_sets); i++)
		flag_sets[i].count = 0;

	for (i = of_next(flags, FLAG_START); i != FLAG_END; 
		 i = of_next(flags, i + 1)) {
		if (flag_power(i)) {
			q = (flag_power(i) * flag_slot_mult(i, wield_slot(o_ptr)));
			p += q;
			log_obj(format("Add %d power for %s, total is %d\n", 
						   q, flag_name(i), p));
		}

		/* Track combinations of flag types */
		for (j = 0; j < N_ELEMENTS(flag_sets); j++)
			if (flag_sets[j].type == obj_flag_type(i))
				flag_sets[j].count++;
	}

	/* Add extra power for multiple flags of the same type */
	for (i = 0; i < N_ELEMENTS(flag_sets); i++) {
		if (flag_sets[i].count > 1) {
			q = (flag_sets[i].factor * flag_sets[i].count * flag_sets[i].count);
			p += q;
			log_obj(format("Add %d power for multiple %s, total is %d\n",
						   q, flag_sets[i].desc, p));
		}

		/* Add bonus if item has a full set of these flags */
		if (flag_sets[i].count == flag_sets[i].size) {
			q = flag_sets[i].bonus;
			p += q;
			log_obj(format("Add %d power for full set of %s, total is %d\n", 
						   q, flag_sets[i].desc, p));
		}
	}

	return p;
}

/* Add power for elemental properties */
static int element_power(const object_type *o_ptr, int p, bool known)
{
	size_t i, j;
	int q;

	/* Zero the set counts */
	for (i = 0; i < N_ELEMENTS(element_sets); i++)
		element_sets[i].count = 0;

	/* Analyse each element for ignore, vulnerability, resistance or immunity */
	for (i = 0; i < N_ELEMENTS(el_powers); i++) {
		if (!known && !(o_ptr->el_info[i].flags & EL_INFO_KNOWN)) continue;

		if (o_ptr->el_info[i].flags & EL_INFO_IGNORE) {
			if (el_powers[i].ignore_power != 0) {
				q = (el_powers[i].ignore_power);
				p += q;
				log_obj(format("Add %d power for ignoring %s, total is %d\n", q, el_powers[i].name, p));
			}
		}

		if (o_ptr->el_info[i].res_level == -1) {
			if (el_powers[i].vuln_power != 0) {
				q = (el_powers[i].vuln_power);
				p += q;
				log_obj(format("Add %d power for vulnerability to %s, total is %d\n", q, el_powers[i].name, p));
			}
		} else if (o_ptr->el_info[i].res_level == 1) {
			if (el_powers[i].res_power != 0) {
				q = (el_powers[i].res_power);
				p += q;
				log_obj(format("Add %d power for resistance to %s, total is %d\n", q, el_powers[i].name, p));
			}
		} else if (o_ptr->el_info[i].res_level == 3) {
			if (el_powers[i].im_power != 0) {
				q = (el_powers[i].im_power);
				p += q;
				log_obj(format("Add %d power for immunity to %s, total is %d\n",
							   q, el_powers[i].name, p));
			}
		}

		/* Track combinations of element properties */
		for (j = 0; j < N_ELEMENTS(element_sets); j++)
			if ((element_sets[j].type == el_powers[i].type) &&
				(element_sets[j].res_level == o_ptr->el_info[i].res_level))
				element_sets[j].count++;
	}

	/* Add extra power for multiple flags of the same type */
	for (i = 0; i < N_ELEMENTS(element_sets); i++) {
		if (element_sets[i].count > 1) {
			q = (element_sets[i].factor * element_sets[i].count * element_sets[i].count);
			p += q;
			log_obj(format("Add %d power for multiple %s, total is %d\n",
						   q, element_sets[i].desc, p));
		}

		/* Add bonus if item has a full set of these flags */
		if (element_sets[i].count == element_sets[i].size) {
			q = element_sets[i].bonus;
			p += q;
			log_obj(format("Add %d power for full set of %s, total is %d\n", 
						   q, element_sets[i].desc, p));
		}
	}

	return p;
}

/* add power for effect */
static int effects_power(const object_type *o_ptr, int p, bool known)
{
	int q = 0;

	if (known || object_effect_is_known(o_ptr))	{
		if (o_ptr->kind->power)
			q = o_ptr->kind->power;

		if (q) {
			p += q;
			log_obj(format("Add %d power for item activation, total is %d\n",
						   q, p));
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
	object_log = log_file;

	/* Known status */
	if (known)
		log_obj("Object is deemed known\n");
	else
		log_obj("Object may not be fully known\n");

	/* Get all the attack power */
	p = to_damage_power(o_ptr);
	dice_pwr = damage_dice_power(o_ptr);
	p += dice_pwr;
	if (dice_pwr) log_obj(format("total is %d\n", p));
	p += ammo_damage_power(o_ptr, p);
	mult = bow_multiplier(o_ptr);
	p = launcher_ammo_damage_power(o_ptr, p);
	p = extra_blows_power(o_ptr, p, known);
	if (p > INHIBIT_POWER) return p;
	p = extra_shots_power(o_ptr, p, known);
	if (p > INHIBIT_POWER) return p;
	p = extra_might_power(o_ptr, p, mult, known);
	if (p > INHIBIT_POWER) return p;
	p = slay_power(o_ptr, p, verbose, dice_pwr, known);
	p = rescale_bow_power(o_ptr, p);
	p = to_hit_power(o_ptr, p);

	/* Armour class power */
	p = ac_power(o_ptr, p);
	p = to_ac_power(o_ptr, p);

	/* Bonus for jewelry */
	p = jewelry_power(o_ptr, p);

	/* Other object properties */
	p = modifier_power(o_ptr, p, known);
	p = flags_power(o_ptr, p, verbose, object_log, known);
	p = element_power(o_ptr, p, known);
	p = effects_power(o_ptr, p, known);

	log_obj(format("FINAL POWER IS %d\n", p));

	return p;
}


/**
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b object_value_base(const object_type *o_ptr)
{
	/* Use template cost for aware objects */
	if (object_flavor_is_aware(o_ptr) || object_all_but_flavor_is_known(o_ptr))
		return o_ptr->kind->cost;

	/* Analyze the type */
	switch (o_ptr->tval)
	{
		case TV_FOOD:
		case TV_MUSHROOM:
			return 5;
		case TV_POTION:
		case TV_SCROLL:
			return 20;
		case TV_RING:
		case TV_AMULET:
			return 45;
		case TV_WAND:
			return 50;
		case TV_STAFF:
			return 70;
		case TV_ROD:
			return 90;
	}

	return 0;
}


/**
 * Return the real price of a known (or partly known) item.
 *
 * Wand and staffs get cost for each charge.
 *
 * Wearable items (weapons, launchers, jewelry, lights, armour) and ammo
 * are priced according to their power rating. All ammo, and normal (non-ego)
 * torches are scaled down by AMMO_RESCALER to reflect their impermanence.
 */
s32b object_value_real(const object_type *o_ptr, int qty, int verbose,
					   bool known)
{
	s32b value, total_value;

	s32b power;
	int a = 1;
	int b = 5;
	static file_mode pricing_mode = MODE_WRITE;

	/* Wearables and ammo have prices that vary by individual item properties */
	if (tval_has_variable_power(o_ptr))	{
		char buf[1024];
		ang_file *log_file = NULL;

		/* Logging */
		if (verbose) {
			path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "pricing.log");
			log_file = file_open(buf, pricing_mode, FTYPE_TEXT);
			if (!log_file) {
				msg("Error - can't open pricing.log for writing.");
				exit(1);
			}
			pricing_mode = MODE_APPEND;
		}

		file_putf(log_file, "object is %s\n", o_ptr->kind->name);

		/* Calculate power and value */
		power = object_power(o_ptr, verbose, log_file, known);
		value = SGN(power) * ((a * power * power) + (b * power));

		/* Rescale for expendables */
		if ((tval_is_light(o_ptr) && of_has(o_ptr->flags, OF_BURNS_OUT)
			 && !o_ptr->ego) || tval_is_ammo(o_ptr)) {
			value = value / AMMO_RESCALER;
			if (value < 1) value = 1;
		}

		/* More logging */
		file_putf(log_file, "a is %d and b is %d\n", a, b);
		file_putf(log_file, "value is %d\n", value);

		if (verbose) {
			if (!file_close(log_file)) {
				msg("Error - can't close pricing.log file.");
				exit(1);
			}
		}

		/* Get the total value */
		total_value = value * qty;
		if (total_value < 0) total_value = 0;
	} else {

		/* Worthless items */
		if (!o_ptr->kind->cost) return (0L);

		/* Base cost */
		value = o_ptr->kind->cost;

		/* Analyze the item type and quantity */
		if (tval_can_have_charges(o_ptr)) {
			int charges;

			total_value = value * qty;

			/* Calculate number of charges, rounded up */
			charges = o_ptr->pval * qty / o_ptr->number;
			if ((o_ptr->pval * qty) % o_ptr->number != 0)
				charges++;

			/* Pay extra for charges, depending on standard number of charges */
			total_value += value * charges / 20;
		} else
			total_value = value * qty;

		/* No negative value */
		if (total_value < 0) total_value = 0;
	}

	/* Return the value */
	return (total_value);
}


/**
 * Return the price of an item including plusses (and charges).
 *
 * This function returns the "value" of the given item (qty one).
 *
 * Never notice unknown bonuses or properties, including curses,
 * since that would give the player information they did not have.
 */
s32b object_value(const object_type *o_ptr, int qty, int verbose)
{
	s32b value;

	/* Known items use the actual value */
	if (object_is_known(o_ptr))	{
		if (cursed_p((bitflag *)o_ptr->flags)) return (0L);

		value = object_value_real(o_ptr, qty, verbose, TRUE);
	} else if (tval_has_variable_power(o_ptr)) {
		/* Variable power items are assessed by what is known about them */
		object_type object_type_body;
		object_type *j_ptr = &object_type_body;

		/* Hack -- Felt cursed items */
		if (object_was_sensed(o_ptr) && cursed_p((bitflag *)o_ptr->flags))
			return (0L);

		memcpy(j_ptr, o_ptr, sizeof(object_type));

		/* give j_ptr only the flags known to be in o_ptr */
		object_flags_known(o_ptr, j_ptr->flags);

		if (!object_attack_plusses_are_visible(o_ptr))
			j_ptr->to_h = j_ptr->to_d = 0;
		if (!object_defence_plusses_are_visible(o_ptr))
			j_ptr->to_a = 0;

		value = object_value_real(j_ptr, qty, verbose, FALSE);
	} else
		/* Unknown constant-price items just get a base value */
		value = object_value_base(o_ptr) * qty;


	/* Return the final value */
	return (value);
}
