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

struct power_calc *calculations;

/**
 * ------------------------------------------------------------------------
 * Helpers for power calculations
 * ------------------------------------------------------------------------ */
static ang_file *object_log;

/**
 * Log progress info to the object log
 */
void log_obj(char *message)
{
	file_putf(object_log, message);
}

static struct obj_property *obj_property_by_type_and_index(int type, int index)
{
	struct obj_property *prop;
	int i;

	/* Find the right property */
	for (i = 0; i < z_info->property_max; i++) {
		prop = &obj_properties[i];
		if ((prop->type == type) && (prop->index == index)) {
			return prop;
		}

		/* Special case - stats count as mods */
		if ((type == OBJ_PROPERTY_MOD) && (prop->type == OBJ_PROPERTY_STAT)
			&& (prop->index == index)) {
			return prop;
		}
	}

	return NULL;
}

/**
 * ------------------------------------------------------------------------
 * Individual object power calculations
 * ------------------------------------------------------------------------ */
static struct object *power_obj;
static int num_brands;
static int num_slays;
static int num_kills;
static int best_power;
static int iter;

static int object_power_calculation_TO_DAM(void)
{
	return power_obj->to_d;
}

static int object_power_calculation_DICE(void)
{
	if (tval_is_ammo(power_obj) || tval_is_melee_weapon(power_obj)) {
		return power_obj->dd * (power_obj->ds + 1);
	} else if (tval_is_launcher(power_obj)) {
		return 0;
	} else if (power_obj->brands || power_obj->slays ||
			   (power_obj->modifiers[OBJ_MOD_BLOWS] > 0) ||
			   (power_obj->modifiers[OBJ_MOD_SHOTS] > 0) ||
			   (power_obj->modifiers[OBJ_MOD_MIGHT] > 0)) {
		return 48;
	}

	return 0;
}

static int object_power_calculation_IS_EGO(void)
{
	return power_obj->ego ? 1 : 0;
}

static int object_power_calculation_EXTRA_BLOWS(void)
{
	return power_obj->modifiers[OBJ_MOD_BLOWS];
}

static int object_power_calculation_EXTRA_SHOTS(void)
{
	return power_obj->modifiers[OBJ_MOD_SHOTS];
}

static int object_power_calculation_EXTRA_MIGHT(void)
{
	return power_obj->modifiers[OBJ_MOD_MIGHT];
}

static int object_power_calculation_BOW_MULTIPLIER(void)
{
	return tval_is_launcher(power_obj) ? power_obj->pval : 1;
}

static int object_power_calculation_BEST_SLAY(void)
{
	return MAX(best_power, 100);
}

static int object_power_calculation_SLAY_SLAY(void)
{
	if (num_slays <= 1) return 0;
	return num_slays * num_slays;
}

static int object_power_calculation_BRAND_BRAND(void)
{
	if (num_brands <= 1) return 0;
	return num_brands * num_brands;
}

static int object_power_calculation_SLAY_BRAND(void)
{
	return num_slays * num_brands;
}

static int object_power_calculation_KILL_KILL(void)
{
	if (num_kills <= 1) return 0;
	return num_kills * num_kills;
}

static int object_power_calculation_ALL_SLAYS(void)
{
	int i, count = 0;
	for (i = 0; i < z_info->slay_max; i++) {
		struct slay *slay = &slays[i];
		if (slay->name && (slay->multiplier <= 3)) {
			count++;
		}
	}

	return num_slays == count ? 1 : 0;
}

static int object_power_calculation_ALL_BRANDS(void)
{
	int i, count = 0;
	for (i = 0; i < z_info->brand_max; i++) {
		struct brand *brand = &brands[i];
		if (brand->name) {
			count++;
		}
	}

	return num_brands == count ? 1 : 0;
}

static int object_power_calculation_ALL_KILLS(void)
{
	int i, count = 0;
	for (i = 0; i < z_info->slay_max; i++) {
		struct slay *slay = &slays[i];
		if (slay->name && (slay->multiplier > 3)) {
			count++;
		}
	}

	return num_kills == count ? 1 : 0;
}

static int object_power_calculation_TO_HIT(void)
{
	return power_obj->to_h;
}

static int object_power_calculation_AC(void)
{
	return power_obj->ac;
}

static int object_power_calculation_TO_ARMOR(void)
{
	return power_obj->to_a;
}

static int object_power_calculation_TOTAL_ARMOR(void)
{
	return power_obj->ac + power_obj->to_a;
}

static int object_power_calculation_WEIGHT(void)
{
	return MAX(20, power_obj->weight);
}

static int object_power_calculation_MODIFIER(void)
{
	return power_obj->modifiers[iter];
}

static int object_power_calculation_MOD_POWER(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_MOD, iter);
	assert(prop);
	return prop->power;
}

static int object_power_calculation_MOD_TYPE_MULT(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_MOD, iter);
	assert(prop);
	return prop->type_mult[power_obj->tval];
}

static int object_power_calculation_MOD_MULT(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_MOD, iter);
	assert(prop);
	return prop->mult;
}

static int object_power_calculation_FLAG_POWER(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_FLAG, iter);
	assert(prop);
	return of_has(power_obj->flags, iter + 1) ? prop->power : 0;
}

static int object_power_calculation_FLAG_TYPE_MULT(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_FLAG, iter);
	assert(prop);
	return prop->type_mult[power_obj->tval];
}

static int object_power_calculation_NUM_SUSTAINS(void)
{
	bitflag f[OF_SIZE];
	of_wipe(f);
	create_obj_flag_mask(f, false, OFT_SUST, OFT_MAX);
	of_inter(f, power_obj->flags);
	return of_count(f) > 1 ? of_count(f) : 0;
}

static int object_power_calculation_ALL_SUSTAINS(void)
{
	bitflag f[OF_SIZE];
	of_wipe(f);
	create_obj_flag_mask(f, false, OFT_SUST, OFT_MAX);
	return of_is_subset(power_obj->flags, f) ? 1 : 0;
}

static int object_power_calculation_NUM_PROTECTS(void)
{
	bitflag f[OF_SIZE];
	of_wipe(f);
	create_obj_flag_mask(f, false, OFT_PROT, OFT_MAX);
	of_inter(f, power_obj->flags);
	return of_count(f) > 1 ? of_count(f) : 0;
}

static int object_power_calculation_ALL_PROTECTS(void)
{
	bitflag f[OF_SIZE];
	of_wipe(f);
	create_obj_flag_mask(f, false, OFT_PROT, OFT_MAX);
	return of_is_subset(power_obj->flags, f) ? 1 : 0;
}

static int object_power_calculation_NUM_MISC(void)
{
	bitflag f[OF_SIZE];
	of_wipe(f);
	create_obj_flag_mask(f, false, OFT_MISC, OFT_MAX);
	of_inter(f, power_obj->flags);
	return of_count(f) > 1 ? of_count(f) : 0;
}

static int object_power_calculation_ALL_MISC(void)
{
	bitflag f[OF_SIZE];
	of_wipe(f);
	create_obj_flag_mask(f, false, OFT_MISC, OFT_MAX);
	return of_is_subset(power_obj->flags, f) ? 1 : 0;
}

static int object_power_calculation_IGNORE(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_IGNORE, iter);
	assert(prop);
	if (power_obj->el_info[iter].flags & EL_INFO_IGNORE) {
		return prop->power;
	}

	return 0;
}

static int object_power_calculation_VULN(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_VULN, iter);
	assert(prop);
	if (power_obj->el_info[iter].res_level == -1) {
		return prop->power;
	}

	return 0;
}

static int object_power_calculation_RESIST(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_RESIST, iter);
	assert(prop);
	if (power_obj->el_info[iter].res_level == 1) {
		return prop->power;
	}

	return 0;
}

static int object_power_calculation_IMM(void)
{
	struct obj_property *prop;
	prop = obj_property_by_type_and_index(OBJ_PROPERTY_IMM, iter);
	assert(prop);
	if (power_obj->el_info[iter].res_level == 3) {
		return prop->power;
	}

	return 0;
}

static int object_power_calculation_NUM_BASE_RES(void)
{
	int i, count = 0;
	for (i = ELEM_BASE_MIN; i < ELEM_BASE_MAX; i++) {
		if (power_obj->el_info[i].res_level >= 1) {
			count++;
		}
	}
	return count > 1 ? count : 0;
}

static int object_power_calculation_ALL_BASE_RES(void)
{
	int i;
	for (i = ELEM_BASE_MIN; i < ELEM_BASE_MAX; i++) {
		if (power_obj->el_info[i].res_level < 1) {
			return 0;
		}
	}
	return 1;
}

static int object_power_calculation_NUM_HIGH_RES(void)
{
	int i, count = 0;
	for (i = ELEM_HIGH_MIN; i < ELEM_HIGH_MAX; i++) {
		if (power_obj->el_info[i].res_level == 1) {
			count++;
		}
	}
	return count > 1 ? count : 0;
}

static int object_power_calculation_ALL_HIGH_RES(void)
{
	int i;
	for (i = ELEM_HIGH_MIN; i < ELEM_HIGH_MAX; i++) {
		if (power_obj->el_info[i].res_level != 1) {
			return 0;
		}
	}
	return 1;
}

static int object_power_calculation_NUM_IMM(void)
{
	int i, count = 0;
	for (i = ELEM_BASE_MIN; i < ELEM_BASE_MAX; i++) {
		if (power_obj->el_info[i].res_level == 3) {
			count++;
		}
	}
	return count > 1 ? count : 0;
}

static int object_power_calculation_ALL_IMM(void)
{
	int i;
	for (i = ELEM_BASE_MIN; i < ELEM_BASE_MAX; i++) {
		if (power_obj->el_info[i].res_level != 3) {
			return 0;
		}
	}
	return 1;
}

static int object_power_calculation_EFFECT_POWER(void)
{
	if (power_obj->artifact && power_obj->artifact->activation) {
		return power_obj->artifact->activation->power;
	}
	return power_obj->kind->power;
}

expression_base_value_f power_calculation_by_name(const char *name)
{
	static const struct power_calc_s {
		const char *name;
		expression_base_value_f function;
	} power_calcs[] = {
		{ "OBJ_POWER_TO_DAM", object_power_calculation_TO_DAM },
		{ "OBJ_POWER_DICE", object_power_calculation_DICE },
		{ "OBJ_POWER_IS_EGO", object_power_calculation_IS_EGO },
		{ "OBJ_POWER_EXTRA_BLOWS", object_power_calculation_EXTRA_BLOWS },
		{ "OBJ_POWER_EXTRA_SHOTS", object_power_calculation_EXTRA_SHOTS },
		{ "OBJ_POWER_EXTRA_MIGHT", object_power_calculation_EXTRA_MIGHT },
		{ "OBJ_POWER_BOW_MULTIPLIER", object_power_calculation_BOW_MULTIPLIER },
		{ "OBJ_POWER_BEST_SLAY", object_power_calculation_BEST_SLAY },
		{ "OBJ_POWER_SLAY_SLAY", object_power_calculation_SLAY_SLAY },
		{ "OBJ_POWER_BRAND_BRAND", object_power_calculation_BRAND_BRAND },
		{ "OBJ_POWER_SLAY_BRAND", object_power_calculation_SLAY_BRAND },
		{ "OBJ_POWER_KILL_KILL", object_power_calculation_KILL_KILL },
		{ "OBJ_POWER_ALL_SLAYS", object_power_calculation_ALL_SLAYS },
		{ "OBJ_POWER_ALL_BRANDS", object_power_calculation_ALL_BRANDS },
		{ "OBJ_POWER_ALL_KILLS", object_power_calculation_ALL_KILLS },
		{ "OBJ_POWER_TO_HIT", object_power_calculation_TO_HIT },
		{ "OBJ_POWER_AC", object_power_calculation_AC },
		{ "OBJ_POWER_TO_ARMOR", object_power_calculation_TO_ARMOR },
		{ "OBJ_POWER_TOTAL_ARMOR", object_power_calculation_TOTAL_ARMOR },
		{ "OBJ_POWER_WEIGHT", object_power_calculation_WEIGHT },
		{ "OBJ_POWER_MODIFIER", object_power_calculation_MODIFIER },
		{ "OBJ_POWER_MOD_POWER", object_power_calculation_MOD_POWER },
		{ "OBJ_POWER_MOD_TYPE_MULT", object_power_calculation_MOD_TYPE_MULT },
		{ "OBJ_POWER_MOD_MULT", object_power_calculation_MOD_MULT },
		{ "OBJ_POWER_FLAG_POWER", object_power_calculation_FLAG_POWER },
		{ "OBJ_POWER_FLAG_TYPE_MULT", object_power_calculation_FLAG_TYPE_MULT },
		{ "OBJ_POWER_NUM_SUSTAINS", object_power_calculation_NUM_SUSTAINS },
		{ "OBJ_POWER_ALL_SUSTAINS", object_power_calculation_ALL_SUSTAINS },
		{ "OBJ_POWER_NUM_PROTECTS", object_power_calculation_NUM_PROTECTS },
		{ "OBJ_POWER_ALL_PROTECTS", object_power_calculation_ALL_PROTECTS },
		{ "OBJ_POWER_NUM_MISC", object_power_calculation_NUM_MISC },
		{ "OBJ_POWER_ALL_MISC", object_power_calculation_ALL_MISC },
		{ "OBJ_POWER_IGNORE", object_power_calculation_IGNORE },
		{ "OBJ_POWER_VULN", object_power_calculation_VULN },
		{ "OBJ_POWER_RESIST", object_power_calculation_RESIST },
		{ "OBJ_POWER_IMM", object_power_calculation_IMM },
		{ "OBJ_POWER_NUM_BASE_RES", object_power_calculation_NUM_BASE_RES },
		{ "OBJ_POWER_ALL_BASE_RES", object_power_calculation_ALL_BASE_RES },
		{ "OBJ_POWER_NUM_HIGH_RES", object_power_calculation_NUM_HIGH_RES },
		{ "OBJ_POWER_ALL_HIGH_RES", object_power_calculation_ALL_HIGH_RES },
		{ "OBJ_POWER_NUM_IMM", object_power_calculation_NUM_IMM },
		{ "OBJ_POWER_ALL_IMM", object_power_calculation_ALL_IMM },
		{ "OBJ_POWER_EFFECT_POWER", object_power_calculation_EFFECT_POWER },
		{ NULL, NULL },
	};
	const struct power_calc_s *current = power_calcs;

	while (current->name != NULL && current->function != NULL) {
		if (my_stricmp(name, current->name) == 0)
			return current->function;

		current++;
	}

	return NULL;
}

/**
 * ------------------------------------------------------------------------
 * Overall object power calculations
 * ------------------------------------------------------------------------ */
/**
 * Run an individual power calculation
 *
 * Dice are used in power calculations sometimes as an easy way of encoding
 * multiplication, so the MAXIMISE aspect is always used in their evaluation.
 */
static int run_power_calculation(struct power_calc *calc)
{
	random_value rv = {0, 0, 0, 0};
	struct poss_item *poss = calc->poss_items;

	/* Ignore null calculations */
	if (!calc->dice) return 0;

	/* Check whether this calculation applies to this item */
	if (poss) {
		while (poss) {
			if (power_obj->kind->kidx == poss->kidx) break;
			poss = poss->next;
		}
		if (!poss) return 0;
	}

	return dice_evaluate(calc->dice, 1, MAXIMISE, &rv);
}

static void apply_op(int operation, int *current, int new)
{
	switch (operation) {
		case POWER_CALC_NONE: {
			break;
		}
		case POWER_CALC_ADD: {
			*current += new;
			break;
		}
		case POWER_CALC_ADD_IF_POSITIVE: {
			if (new > 0) {
				*current += new;
			}
			break;
		}
		case POWER_CALC_SQUARE_ADD_IF_POSITIVE: {
			if (new > 0) {
				new *= new;
				*current += new;
			}
			break;
		}
		case POWER_CALC_MULTIPLY: {
			*current *= new;
			break;
		}
		case POWER_CALC_DIVIDE: {
			*current /= new;
			break;
		}
		default: {
			break;
		}
	}
}

/**
 * Calculate stats on slays and brands up
 */
static void collect_slay_brand_stats(const struct object *obj)
{
	int i;

	num_brands = 0;
	num_slays = 0;
	num_kills = 0;
	best_power = 1;
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

	/* Write the best power */
	if (num_slays + num_brands + num_kills) {
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
}

/**
 * Run all the power calculations on an object to find its power
 */
int object_power(const struct object* obj, bool verbose, ang_file *log_file)
{
	int i;
	int **current_value;
	int power = 0;

	/* Set the log file */
	object_log = log_file;

	/* Set the power evaluation object and collect slay and brand stats */
	power_obj = (struct object *) obj;
	collect_slay_brand_stats(obj);

	/* Set up arrays for each power calculation (most of them length 1) */
	current_value = mem_zalloc(z_info->calculation_max * sizeof(int*));
	for (i = 0; i < z_info->calculation_max; i++) {
		struct power_calc *calc = &calculations[i];

		current_value[i] = mem_zalloc(calc->iterate.max * sizeof(int));
	}

	/* Preprocess the power calculations for intermediate results */
	for (i = 0; i < z_info->calculation_max; i++) {
		struct power_calc *calc = &calculations[i];
		int j;

		/* Run the calculation... */
		for (iter = 0; iter < calc->iterate.max; iter++) {
			current_value[i][iter] = run_power_calculation(calc);
		}

		/* ...and apply to an earlier one if needed */
		if (calc->apply_to) {
			for (j = 0; j < i; j++) {
				if (!calculations[j].name) continue;
				if (streq(calculations[j].name, calc->apply_to)) {
					break;
				}
			}

			/* Ignore this calculation if no name found, otherwise apply it */
			if (i == j) {
				log_obj(format("No target %s for %s to apply to\n",
							   calc->apply_to, calc->name));
			} else {
				if ((calculations[j].iterate.max == 1) &&
					(calc->iterate.max > 1)) {
					/* Many values applying to one */
					for (iter = 0; iter < calc->iterate.max; iter++) {
						apply_op(calc->operation, &current_value[j][0],
								 current_value[i][iter]);
					}
				} else if (calculations[j].iterate.max == calc->iterate.max) {
					/* Both the same size */
					for (iter = 0; iter < calc->iterate.max; iter++) {
						apply_op(calc->operation, &current_value[j][iter],
								 current_value[i][iter]);
					}
				} else {
					log_obj(format("Size mismatch applying %s to %s\n",
								   calc->name, calculations[j].name));
				}
			}
		}
	}

	/* Put all the power calculations together */
	for (i = 0; i < z_info->calculation_max; i++) {
		struct power_calc *calc = &calculations[i];
		struct poss_item *poss = calc->poss_items;

		/* Check whether this calculation applies to this item */
		if (poss) {
			while (poss) {
				if (power_obj->kind->kidx == poss->kidx) break;
				poss = poss->next;
			}
			if (!poss) continue;
		}

		if (calc->apply_to == NULL) {
			int old_power = power;
			for (iter = 0; iter < calc->iterate.max; iter++) {
				apply_op(calc->operation, &power, current_value[i][iter]);
			}

			/* Report result if there's a change in power */
			if (power != old_power) {
				if (calc->iterate.max == 1) {
					log_obj(format("%s is %d, power is %d\n", calc->name,
								   power - old_power, power));
				} else {
					for (iter = 0; iter < calc->iterate.max; iter++) {
						struct obj_property *prop;
						int type = calc->iterate.property_type;
						prop = obj_property_by_type_and_index(type, iter);
						if (current_value[i][iter] != 0) {
							old_power += current_value[i][iter];
							log_obj(format("%d for %s, power is %d\n",
										   current_value[i][iter], prop->name,
										   old_power));
						}
					}
				}
			}
		}
	}

	/* Free the current value arrays */
	for (i = 0; i < z_info->calculation_max; i++) {
		mem_free(current_value[i]);
	}
	mem_free(current_value);

	log_obj(format("FINAL POWER IS %d\n", power));
	return power;
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
int object_value(const struct object *obj, int qty)
{
	int value;

	/* Variable power items are assessed by what is known about them */
	if (tval_has_variable_power(obj) && obj->known)
		value = object_value_real(obj->known, qty);
	else
		/* Unknown constant-price items just get a base value */
		value = object_value_base(obj) * qty;

	/* Return the final value */
	return (value);
}
