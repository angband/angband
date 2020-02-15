/**
 * \file obj-power.c
 * \brief calculation of object power and value
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
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-power.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "init.h"
#include "effects.h"
#include "monster.h"

/**
 * ------------------------------------------------------------------------
 * Object power data and assumptions
 * ------------------------------------------------------------------------ */

/**
 * Define a set of constants for dealing with launchers and ammo:
 * - the assumed average damage of ammo (for rating launchers)
 * (the current values assume normal (non-seeker) ammo enchanted to +9)
 * - the assumed bonus on launchers (for rating ego ammo)
 * - twice the assumed multiplier (for rating any ammo)
 * N.B. Ammo tvals are assumed to be consecutive! We access this array using
 * (obj->tval - TV_SHOT) for ammo, and
 * (obj->sval / 10) for launchers
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
static ang_file *object_log;

/**
 * Log progress info to the object log
 */
void log_obj(char *message)
{
	file_putf(object_log, message);
}

/**
 * ------------------------------------------------------------------------
 * Object power calculations
 * ------------------------------------------------------------------------ */

/**
 * Calculate the multiplier we'll get with a given bow type.
 */
static int bow_multiplier(const struct object *obj)
{
	int mult = 1;

	if (obj->tval != TV_BOW)
		return mult;
	else
		mult = obj->pval;

	log_obj(format("Base mult for this weapon is %d\n", mult));
	return mult;
}

/**
 * To damage power
 */
static int to_damage_power(const struct object *obj)
{
	int p;

	p = (obj->to_d * DAMAGE_POWER / 2);
	if (p) log_obj(format("%d power from to_dam\n", p));

	/* Add second lot of damage power for non-weapons */
	if ((wield_slot(obj) != slot_by_name(player, "shooting")) &&
		!tval_is_melee_weapon(obj) &&
		!tval_is_ammo(obj)) {
		int q = (obj->to_d * DAMAGE_POWER);
		p += q;
		if (q)
			log_obj(format("Add %d from non-weapon to_dam, total %d\n", q, p));
	}
	return p;
}

/**
 * Damage dice power or equivalent
 */
static int damage_dice_power(const struct object *obj)
{
	int dice = 0;

	/* Add damage from dice for any wieldable weapon or ammo */
	if (tval_is_melee_weapon(obj) || tval_is_ammo(obj)) {
		dice = (obj->dd * (obj->ds + 1) * DAMAGE_POWER / 4);
		log_obj(format("Add %d power for damage dice, ", dice));
	} else if (wield_slot(obj) != slot_by_name(player, "shooting")) {
		/* Add power boost for nonweapons with combat flags */
		if (obj->brands || obj->slays ||
			(obj->modifiers[OBJ_MOD_BLOWS] > 0) ||
			(obj->modifiers[OBJ_MOD_SHOTS] > 0) ||
			(obj->modifiers[OBJ_MOD_MIGHT] > 0)) {
			dice = (WEAP_DAMAGE * DAMAGE_POWER);
			log_obj(format("Add %d power for non-weapon combat bonuses, ",
						   dice));
		}
	}
	return dice;
}

/**
 * Add ammo damage for launchers, get multiplier and rescale
 */
static int ammo_damage_power(const struct object *obj, int p)
{
	int q = 0;
	int launcher = -1;

	if (wield_slot(obj) == slot_by_name(player, "shooting")) {
		if (kf_has(obj->kind->kind_flags, KF_SHOOTS_SHOTS))
			launcher = 0;
		else if (kf_has(obj->kind->kind_flags, KF_SHOOTS_ARROWS))
			launcher = 1; 
		else if (kf_has(obj->kind->kind_flags, KF_SHOOTS_BOLTS))
			launcher = 2;

		if (launcher != -1) {
			q = (archery[launcher].ammo_dam * DAMAGE_POWER / 2);
			log_obj(format("Adding %d power from ammo, total is %d\n", q,
						   p + q));
		}
	}
	return q;
}

/**
 * Add launcher bonus for ego ammo, multiply for launcher and rescale
 */
static int launcher_ammo_damage_power(const struct object *obj, int p)
{
	int ammo_type = 0;

	if (tval_is_ammo(obj)) {
		if (obj->tval == TV_ARROW) ammo_type = 1;
		if (obj->tval == TV_BOLT) ammo_type = 2;
		if (obj->ego)
			p += (archery[ammo_type].launch_dam * DAMAGE_POWER / 2);
		p = p * archery[ammo_type].launch_mult / (2 * MAX_BLOWS);
		log_obj(format("After multiplying ammo and rescaling, power is %d\n",
					   p));
	}
	return p;
}

/**
 * Add power for extra blows
 */
static int extra_blows_power(const struct object *obj, int p)
{
	int q = p;

	if (obj->modifiers[OBJ_MOD_BLOWS] == 0)
		return p;

	if (obj->modifiers[OBJ_MOD_BLOWS] >= INHIBIT_BLOWS) {
		p += INHIBIT_POWER;
		log_obj("INHIBITING - too many extra blows - quitting\n");
		return p;
	} else {
		p = p * (MAX_BLOWS + obj->modifiers[OBJ_MOD_BLOWS]) / MAX_BLOWS;
		/* Add boost for assumed off-weapon damage */
		p += (NONWEAP_DAMAGE * obj->modifiers[OBJ_MOD_BLOWS]
			  * DAMAGE_POWER / 2);
		log_obj(format("Add %d power for extra blows, total is %d\n",
					   p - q, p));
	}
	return p;
}

/**
 * Add power for extra shots - note that we cannot handle negative shots
 */
static int extra_shots_power(const struct object *obj, int p)
{
	if (obj->modifiers[OBJ_MOD_SHOTS] == 0)
		return p;

	if (obj->modifiers[OBJ_MOD_SHOTS] >= INHIBIT_SHOTS) {
		p += INHIBIT_POWER;
		log_obj("INHIBITING - too many extra shots - quitting\n");
		return p;
	} else if (obj->modifiers[OBJ_MOD_SHOTS] > 0) {
		/* Multiply by effective number of shots */
		int q = obj->modifiers[OBJ_MOD_SHOTS];
		p *= (10 + q);
		p /= 10;
		log_obj(format("Adding %d%% power for extra shots, total is %d\n",
					   10 * q, p));
	}
	return p;
}


/**
 * Add power for extra might
 */
static int extra_might_power(const struct object *obj, int p, int mult)
{
	if (obj->modifiers[OBJ_MOD_MIGHT] >= INHIBIT_MIGHT) {
		p += INHIBIT_POWER;
		log_obj("INHIBITING - too much extra might - quitting\n");
		return p;
	} else {
		mult += obj->modifiers[OBJ_MOD_MIGHT];
	}
	log_obj(format("Mult after extra might is %d\n", mult));
	p *= mult;
	log_obj(format("After multiplying power for might, total is %d\n", p));
	return p;
}

/**
 * Calculate the rating for a given slay combination
 */
static s32b slay_power(const struct object *obj, int p, int verbose,
					   int dice_pwr)
{
	int i, q, num_brands = 0, num_slays = 0, num_kills = 0;
	int best_power = 1;

	/* Count the brands and slays */
	if (obj->brands) {
		for (i = 1; i < z_info->brand_max; i++) {
			if (obj->brands[i]) {
				num_brands++;
				if (brands[i].power > best_power)
					best_power = brands[i].power;
			}
		}
	}
	if (obj->slays) {
		for (i = 1; i < z_info->slay_max; i++) {
			if (obj->slays[i]) {
				if (slays[i].multiplier <= 3) {
					num_slays++;
				} else {
					num_kills++;
				}
				if (slays[i].power > best_power)
					best_power = slays[i].power;
			}
		}
	}

	/* If there are no slays or brands return */
	if ((num_slays + num_brands + num_kills) == 0)
		return p;

	/* Write the best power */
	if (verbose) {
		/* Write info about the slay combination and multiplier */
		log_obj("Slay and brands: ");

		if (obj->brands) {
			for (i = 1; i < z_info->brand_max; i++) {
				if (obj->brands[i]) {
					struct brand *b = &brands[i];
					log_obj(format("%sx%d ", b->name, b->multiplier));
				}
			}
		}
		if (obj->slays) {
			for (i = 1; i < z_info->slay_max; i++) {
				if (obj->slays[i]) {
					struct slay *s = &slays[i];
					log_obj(format("%sx%d ", s->name, s->multiplier));
				}
			}
		}
		log_obj(format("\nbest power is : %d\n", best_power));
	}

	q = (dice_pwr * (best_power - 100)) / 100;
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
	if (num_slays && num_brands) {
		q = (num_slays * num_brands * dice_pwr) / (DAMAGE_POWER * 5);
		p += q;
		log_obj(format("Add %d power for slay and brand, total is %d\n", q, p));
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

/**
 * Melee weapons assume MAX_BLOWS per turn, so we must divide by MAX_BLOWS
 * to get equal ratings for launchers.
 */
static int rescale_bow_power(const struct object *obj, int p)
{
	if (wield_slot(obj) == slot_by_name(player, "shooting")) {
		p /= MAX_BLOWS;
		log_obj(format("Rescaling bow power, total is %d\n", p));
	}
	return p;
}

/**
 * Add power for +to_hit
 */
static int to_hit_power(const struct object *obj, int p)
{
	int q = (obj->to_h * TO_HIT_POWER / 2);
	p += q;
	if (p) 
		log_obj(format("Add %d power for to hit, total is %d\n", q, p));
	return p;
}

/**
 * Add power for base AC and adjust for weight
 */
static int ac_power(const struct object *obj, int p)
{
	int q = 0;

	if (obj->ac) {
		p += BASE_ARMOUR_POWER;
		q += (obj->ac * BASE_AC_POWER / 2);
		log_obj(format("Adding %d power for base AC value\n", q));

		/* Add power for AC per unit weight */
		if (obj->weight > 0) {
			int i = 750 * (obj->ac + obj->to_a) / obj->weight;

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


/**
 * Add power for +to_ac
 */
static int to_ac_power(const struct object *obj, int p)
{
	int q;

	if (obj->to_a == 0) return p;

	q = (obj->to_a * TO_AC_POWER / 2);
	p += q;
	log_obj(format("Add %d power for to_ac of %d, total is %d\n", 
				   q, obj->to_a, p));
	if (obj->to_a > HIGH_TO_AC) {
		q = ((obj->to_a - (HIGH_TO_AC - 1)) * TO_AC_POWER);
		p += q;
		log_obj(format("Add %d power for high to_ac, total is %d\n",
							q, p));
	}
	if (obj->to_a > VERYHIGH_TO_AC) {
		q = ((obj->to_a - (VERYHIGH_TO_AC -1)) * TO_AC_POWER * 2);
		p += q;
		log_obj(format("Add %d power for very high to_ac, total is %d\n",q, p));
	}
	if (obj->to_a >= INHIBIT_AC) {
		p += INHIBIT_POWER;
		log_obj("INHIBITING: AC bonus too high\n");
	}
	return p;
}

/**
 * Add base power for jewelry
 */
static int jewelry_power(const struct object *obj, int p)
{
	if (tval_is_jewelry(obj)) {
		p += BASE_JEWELRY_POWER;
		log_obj(format("Adding %d power for jewelry, total is %d\n", 
					   BASE_JEWELRY_POWER, p));
	}
	return p;
}

/**
 * Add power for modifiers
 */
static int modifier_power(const struct object *obj, int p)
{
	int i, k, extra_stat_bonus = 0, q;

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		/* Get the modifier details */
		struct obj_property *mod = lookup_obj_property(OBJ_PROPERTY_MOD, i);
		assert(mod);

		k = obj->modifiers[i];
		extra_stat_bonus += (k * mod->mult);

		if (mod->power) {
			q = (k * mod->power * mod->type_mult[obj->tval]);
			p += q;
			if (q) log_obj(format("Add %d power for %d %s, total is %d\n", 
								  q, k, mod->name, p));
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

/**
 * Add power for non-derived flags (derived flags have flag_power 0)
 */
static int flags_power(const struct object *obj, int p, int verbose,
					   ang_file *log_file)
{
	size_t i, j;
	int q;
	bitflag flags[OF_SIZE];

	/* Extract the flags */
	object_flags(obj, flags);

	/* Zero the flag counts */
	for (i = 0; i < N_ELEMENTS(flag_sets); i++)
		flag_sets[i].count = 0;

	for (i = of_next(flags, FLAG_START); i != FLAG_END; 
		 i = of_next(flags, i + 1)) {
		/* Get the flag details */
		struct obj_property *flag = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
		assert(flag);

		if (flag->power) {
			q = (flag->power * flag->type_mult[obj->tval]);
			p += q;
			log_obj(format("Add %d power for %s, total is %d\n", 
						   q, flag->name, p));
		}

		/* Track combinations of flag types */
		for (j = 0; j < N_ELEMENTS(flag_sets); j++)
			if (flag_sets[j].type == flag->subtype)
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

/**
 * Add power for elemental properties
 */
static int element_power(const struct object *obj, int p)
{
	size_t i, j;
	int q;

	/* Zero the set counts */
	for (i = 0; i < N_ELEMENTS(element_sets); i++)
		element_sets[i].count = 0;

	/* Analyse each element for ignore, vulnerability, resistance or immunity */
	for (i = 0; i < N_ELEMENTS(el_powers); i++) {
		if (obj->el_info[i].flags & EL_INFO_IGNORE) {
			if (el_powers[i].ignore_power != 0) {
				q = (el_powers[i].ignore_power);
				p += q;
				log_obj(format("Add %d power for ignoring %s, total is %d\n",
							   q, el_powers[i].name, p));
			}
		}

		if (obj->el_info[i].res_level == -1) {
			if (el_powers[i].vuln_power != 0) {
				q = (el_powers[i].vuln_power);
				p += q;
				log_obj(format("Add %d power for vulnerability to %s, total is %d\n", q, el_powers[i].name, p));
			}
		} else if (obj->el_info[i].res_level == 1) {
			if (el_powers[i].res_power != 0) {
				q = (el_powers[i].res_power);
				p += q;
				log_obj(format("Add %d power for resistance to %s, total is %d\n", q, el_powers[i].name, p));
			}
		} else if (obj->el_info[i].res_level == 3) {
			if (el_powers[i].im_power != 0) {
				q = (el_powers[i].im_power + el_powers[i].res_power);
				p += q;
				log_obj(format("Add %d power for immunity to %s, total is %d\n",
							   q, el_powers[i].name, p));
			}
		}

		/* Track combinations of element properties */
		for (j = 0; j < N_ELEMENTS(element_sets); j++)
			if ((element_sets[j].type == el_powers[i].type) &&
				(element_sets[j].res_level <= obj->el_info[i].res_level))
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

/**
 * Add power for effect
 */
static int effects_power(const struct object *obj, int p)
{
	int q = 0;

	if (obj->activation) {
		q = obj->activation->power;
	} else if (obj->kind->power) {
		q = obj->kind->power;
	}

	if (q) {
		p += q;
		log_obj(format("Add %d power for item activation, total is %d\n",
					   q, p));
	}
	return p;
}

/**
 * Add power for curses
 */
static int curse_power(const struct object *obj, int p, int verbose,
					   ang_file *log_file)
{
	int i, q = 0;

	if (obj->curses) {
		/* Get the curse object power */
		for (i = 1; i < z_info->curse_max; i++) {
			if (obj->curses[i].power) {
				int curse_power;
				log_obj(format("Calculating %s curse power...\n",
							   curses[i].name));
				curse_power = object_power(curses[i].obj, verbose, log_file);
				curse_power -= obj->curses[i].power / 10;
				log_obj(format("Adjust for strength of curse, %d for %s curse power\n", curse_power, curses[i].name));
				q += curse_power;
			}
		}
	}

	if (q != 0) {
		p += q;
		log_obj(format("Total of %d power added for curses, total is %d\n",
					   q, p));
	}
	return p;
}


/**
 * Evaluate the object's overall power level.
 */
s32b object_power(const struct object* obj, bool verbose, ang_file *log_file)
{
	s32b p = 0, dice_pwr = 0;
	int mult;

	/* Set the log file */
	object_log = log_file;

	/* Get all the attack power */
	p = to_damage_power(obj);
	dice_pwr = damage_dice_power(obj);
	p += dice_pwr;
	if (dice_pwr) log_obj(format("total is %d\n", p));
	p += ammo_damage_power(obj, p);
	mult = bow_multiplier(obj);
	p = launcher_ammo_damage_power(obj, p);
	p = extra_blows_power(obj, p);
	if (p > INHIBIT_POWER) return p;
	p = extra_shots_power(obj, p);
	if (p > INHIBIT_POWER) return p;
	p = extra_might_power(obj, p, mult);
	if (p > INHIBIT_POWER) return p;
	p = slay_power(obj, p, verbose, dice_pwr);
	p = rescale_bow_power(obj, p);
	p = to_hit_power(obj, p);

	/* Armour class power */
	p = ac_power(obj, p);
	p = to_ac_power(obj, p);

	/* Bonus for jewelry */
	p = jewelry_power(obj, p);

	/* Other object properties */
	p = modifier_power(obj, p);
	p = flags_power(obj, p, verbose, object_log);
	p = element_power(obj, p);
	p = effects_power(obj, p);
	p = curse_power(obj, p, verbose, object_log);

	log_obj(format("FINAL POWER IS %d\n", p));

	return p;
}


/**
 * ------------------------------------------------------------------------
 * Object pricing
 * ------------------------------------------------------------------------ */
/**
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static int object_value_base(const struct object *obj)
{
	/* Use template cost for aware objects */
	if (object_flavor_is_aware(obj))
		return obj->kind->cost;

	/* Analyze the type */
	switch (obj->tval)
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
int object_value_real(const struct object *obj, int qty)
{
	int value, total_value;

	int power;
	int a = 1;
	int b = 5;

	/* Wearables and ammo have prices that vary by individual item properties */
	if (tval_has_variable_power(obj)) {
#ifdef PRICE_DEBUG
		char buf[1024];
		ang_file *log_file = NULL;
		static file_mode pricing_mode = MODE_WRITE;

		/* Logging */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "pricing.log");
		log_file = file_open(buf, pricing_mode, FTYPE_TEXT);
		if (!log_file) {
			msg("Error - can't open pricing.log for writing.");
			exit(1);
		}
		pricing_mode = MODE_APPEND;

		file_putf(log_file, "object is %s\n", obj->kind->name);

		power = object_power(obj, true, log_file);
#else /* PRICE_DEBUG */
		power = object_power(obj, false, NULL);
#endif /* PRICE_DEBUG */
		value = SGN(power) * ((a * power * power) + (b * power));

		/* Rescale for expendables */
		if ((tval_is_light(obj) && of_has(obj->flags, OF_BURNS_OUT)
			 && !obj->ego) || tval_is_ammo(obj)) {
			value = value / AMMO_RESCALER;
		}

		/* Round up to make sure things like cloaks are not worthless */
		if (value == 0) {
			value = 1;
		}

#ifdef PRICE_DEBUG
		/* More logging */
		file_putf(log_file, "a is %d and b is %d\n", a, b);
		file_putf(log_file, "value is %d\n", value);

		if (!file_close(log_file)) {
			msg("Error - can't close pricing.log file.");
			exit(1);
		}
#endif /* PRICE_DEBUG */

		/* Get the total value */
		total_value = value * qty;
		if (total_value < 0) total_value = 0;
	} else {

		/* Worthless items */
		if (!obj->kind->cost) return (0L);

		/* Base cost */
		value = obj->kind->cost;

		/* Analyze the item type and quantity */
		if (tval_can_have_charges(obj)) {
			int charges;

			total_value = value * qty;

			/* Calculate number of charges, rounded up */
			charges = obj->pval * qty / obj->number;
			if ((obj->pval * qty) % obj->number != 0)
				charges++;

			/* Pay extra for charges, depending on standard number of charges */
			total_value += value * charges / 20;
		} else {
			total_value = value * qty;
		}

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
int object_value(const struct object *obj, int qty)
{
	int value;

	/* Variable power items are assessed by what is known about them */
	if (tval_has_variable_power(obj) && obj->known) {
		value = object_value_real(obj->known, qty);
	} else if (tval_can_have_flavor_k(obj->kind) &&
			   object_flavor_is_aware(obj)) {
		value = object_value_real(obj, qty);
	} else {
		/* Unknown constant-price items just get a base value */
		value = object_value_base(obj) * qty;
	}

	/* Return the final value */
	return (value);
}
