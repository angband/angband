/**
 * \file obj-randart.c
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
#include "angband.h"
#include "datafile.h"
#include "effects.h"
#include "init.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-randart.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "project.h"
#include "randname.h"
#include "z-textblock.h"

/*
 * Pointer for logging file
 */
static ang_file *log_file = NULL;

/* Activation list */
struct activation *activations;

/**
 * ------------------------------------------------------------------------
 * Arrays of indices by item type, used in frequency generation
 * ------------------------------------------------------------------------ */
static s16b art_idx_bow[] = {
	ART_IDX_BOW_SHOTS,
	ART_IDX_BOW_MIGHT,
	ART_IDX_BOW_BRAND,
	ART_IDX_BOW_SLAY
};
static s16b art_idx_weapon[] = {
	ART_IDX_WEAPON_HIT,
	ART_IDX_WEAPON_DAM,
	ART_IDX_WEAPON_AGGR
};
static s16b art_idx_nonweapon[] = {
	ART_IDX_NONWEAPON_HIT,
	ART_IDX_NONWEAPON_DAM,
	ART_IDX_NONWEAPON_HIT_DAM,
	ART_IDX_NONWEAPON_AGGR,
	ART_IDX_NONWEAPON_BRAND,
	ART_IDX_NONWEAPON_SLAY,
	ART_IDX_NONWEAPON_BLOWS,
	ART_IDX_NONWEAPON_SHOTS
};
static s16b art_idx_melee[] = {
	ART_IDX_MELEE_BLESS,
	ART_IDX_MELEE_SINV,
	ART_IDX_MELEE_BRAND,
	ART_IDX_MELEE_SLAY,
	ART_IDX_MELEE_BLOWS,
	ART_IDX_MELEE_AC,
	ART_IDX_MELEE_DICE,
	ART_IDX_MELEE_WEIGHT,
	ART_IDX_MELEE_TUNN
};
static s16b art_idx_allarmor[] = {
	ART_IDX_ALLARMOR_WEIGHT
};
static s16b art_idx_boot[] = {
	ART_IDX_BOOT_AC,
	ART_IDX_BOOT_FEATHER,
	ART_IDX_BOOT_STEALTH,
	ART_IDX_BOOT_TRAP_IMM,
	ART_IDX_BOOT_SPEED
};
static s16b art_idx_glove[] = {
	ART_IDX_GLOVE_AC,
	ART_IDX_GLOVE_HIT_DAM,
	ART_IDX_GLOVE_FA,
	ART_IDX_GLOVE_DEX
};
static s16b art_idx_headgear[] = {
	ART_IDX_HELM_AC,
	ART_IDX_HELM_RBLIND,
	ART_IDX_HELM_ESP,
	ART_IDX_HELM_SINV,
	ART_IDX_HELM_WIS,
	ART_IDX_HELM_INT
};
static s16b art_idx_shield[] = {
	ART_IDX_SHIELD_AC,
	ART_IDX_SHIELD_LRES
};
static s16b art_idx_cloak[] = {
	ART_IDX_CLOAK_AC,
	ART_IDX_CLOAK_STEALTH
};
static s16b art_idx_armor[] = {
	ART_IDX_ARMOR_AC,
	ART_IDX_ARMOR_STEALTH,
	ART_IDX_ARMOR_HLIFE,
	ART_IDX_ARMOR_CON,
	ART_IDX_ARMOR_LRES,
	ART_IDX_ARMOR_ALLRES,
	ART_IDX_ARMOR_HRES};
static s16b art_idx_gen[] = {
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
	ART_IDX_GEN_DAM_RED,
	ART_IDX_GEN_MOVES,
	ART_IDX_GEN_TRAP_IMM
};
static s16b art_idx_high_resist[] =	{
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
	ART_IDX_GEN_PSTUN
};

/**
 * ------------------------------------------------------------------------
 * Calculation of the statistics of an artifact set
 * ------------------------------------------------------------------------ */
/**
 * Return the artifact power, by generating a "fake" object based on the
 * artifact, and calling the common object_power function
 */
static int artifact_power(int a_idx, char *reason, bool verbose)
{
	struct object *obj = object_new();
	struct object *known_obj = object_new();
	char buf[256];
	s32b power;

	file_putf(log_file, "********** Evaluating %s ********\n", reason);
	file_putf(log_file, "Artifact index is %d\n", a_idx);

	if (!make_fake_artifact(obj, &a_info[a_idx])) {
		object_delete(&known_obj);
		object_delete(&obj);
		return 0;
	}

	object_copy(known_obj, obj);
	obj->known = known_obj;
	object_desc(buf, 256 * sizeof(char), obj,
				ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);
	file_putf(log_file, "%s\n", buf);

	power = object_power(obj, verbose, log_file);

	object_delete(&known_obj);
	object_delete(&obj);
	return power;
}


/**
 * Store the original artifact power ratings as a baseline
 */
static void store_base_power(struct artifact_set_data *data)
{
	int i, num;
	struct artifact *art;
	struct object_kind *kind;
	int *fake_total_power;
	int **fake_tv_power;

	data->max_power = 0;
	data->min_power = INHIBIT_POWER + 1;
	data->var_power = 0;
	fake_total_power = mem_zalloc(z_info->a_max * sizeof(int));
	fake_tv_power = mem_zalloc(TV_MAX * sizeof(int*));
	for (i = 0; i < TV_MAX; i++) {
		fake_tv_power[i] = mem_zalloc(z_info->a_max * sizeof(int));
		data->min_tv_power[i] = INHIBIT_POWER + 1;
		data->max_tv_power[i] = 0;
	}
	num = 0;

	for (i = 0; i < z_info->a_max; i++, num++) {
		data->base_power[i] = artifact_power(i, "for original power", true);

		/* Capture power stats, ignoring cursed and uber arts */
		if (data->base_power[i] > data->max_power &&
			data->base_power[i] < INHIBIT_POWER)
			data->max_power = data->base_power[i];
		if (data->base_power[i] < data->min_power && data->base_power[i] > 0)
			data->min_power = data->base_power[i];
		if (data->base_power[i] > 0 && data->base_power[i] < INHIBIT_POWER) {
			int tval = a_info[i].tval;
			fake_total_power[num] = (int)data->base_power[i];
			fake_tv_power[tval][data->tv_num[tval]++] = data->base_power[i];
			if (data->base_power[i] < data->min_tv_power[tval]) {
				data->min_tv_power[tval] = data->base_power[i];
			}
			if (data->base_power[i] > data->max_tv_power[tval]) {
				data->max_tv_power[tval] = data->base_power[i];
			}
		} else {
			num--;
		}
		if (data->base_power[i] < 0) {
			data->neg_power_total++;
		}

		if (!data->base_power[i]) continue;
		art = &a_info[i];
		kind = lookup_kind(art->tval, art->sval);
		data->base_item_level[i] = kind->level;
		data->base_item_prob[i] = kind->alloc_prob;
		data->base_art_alloc[i] = art->alloc_prob;
	}

	data->avg_power = mean(fake_total_power, num);
	data->var_power = variance(fake_total_power, num);
	for (i = 0; i < TV_MAX; i++) {
		if (data->tv_num[i]) {
			data->avg_tv_power[i] = mean(fake_tv_power[i], data->tv_num[i]);
		}
	}

	file_putf(log_file, "Max power is %d, min is %d\n", data->max_power,
			  data->min_power);
	file_putf(log_file, "Mean is %d, variance is %d\n", data->avg_power,
			  data->var_power);
	for (i = 0; i < TV_MAX; i++) {
		if (data->avg_tv_power[i]) {
			file_putf(log_file, "Power for tval %s: min %d, max %d, avg %d\n",
					  tval_find_name(i), data->min_tv_power[i],
					  data->max_tv_power[i], data->avg_tv_power[i]);
		}
	}

	/* Store the number of different types, for use later */
	/* ToDo: replace this with full combination tracking */
	for (i = 0; i < z_info->a_max; i++) {
		switch (a_info[i].tval)
		{
		case TV_SWORD:
		case TV_POLEARM:
		case TV_HAFTED:
			data->melee_total++; break;
		case TV_BOW:
			data->bow_total++; break;
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			data->armor_total++; break;
		case TV_SHIELD:
			data->shield_total++; break;
		case TV_CLOAK:
			data->cloak_total++; break;
		case TV_HELM:
		case TV_CROWN:
			data->headgear_total++; break;
		case TV_GLOVES:
			data->glove_total++; break;
		case TV_BOOTS:
			data->boot_total++; break;
		case TV_NULL:
			break;
		default:
			data->other_total++;
		}
		data->total++;
	}

	for (i = 0; i < TV_MAX; i++) {
		mem_free(fake_tv_power[i]);
	}
	mem_free(fake_tv_power);
    mem_free(fake_total_power);
}

/**
 * Handle weapon combat abilities
 */
void count_weapon_abilities(const struct artifact *art,
							struct artifact_set_data *data)
{
	int bonus;
	struct object_kind *kind = lookup_kind(art->tval, art->sval);
	int min_to_h = randcalc(kind->to_h, 0, MINIMISE);
	int min_to_d = randcalc(kind->to_d, 0, MINIMISE);
	int min_to_a = randcalc(kind->to_a, 0, MINIMISE);

	/* To-hit and to-dam */
	bonus = (art->to_h - min_to_h - data->hit_startval) / data->hit_increment;
	if (bonus > 0)
		file_putf(log_file, "Adding %d instances of extra to-hit bonus for weapon\n", bonus);
	else if (bonus < 0)
		file_putf(log_file, "Subtracting %d instances of extra to-hit bonus for weapon\n", bonus);
	data->art_probs[ART_IDX_WEAPON_HIT] += bonus;

	bonus = (art->to_d - min_to_d - data->dam_startval) / data->dam_increment;
	if (bonus > 0)
		file_putf(log_file, "Adding %d instances of extra to-dam bonus for weapon\n", bonus);
	else
		file_putf(log_file, "Subtracting %d instances of extra to-dam bonus for weapon\n", bonus);
	data->art_probs[ART_IDX_WEAPON_DAM] += bonus;

	/* Does this weapon have an unusual bonus to AC? */
	bonus = (art->to_a - min_to_a) / data->ac_increment;
	if (art->to_a > 20) {
		file_putf(log_file, "Adding %d for supercharged AC\n", bonus);
		(data->art_probs[ART_IDX_MELEE_AC_SUPER])++;
	} else if (bonus > 0) {
		file_putf(log_file,
				  "Adding %d instances of extra AC bonus for weapon\n", bonus);
		(data->art_probs[ART_IDX_MELEE_AC]) += bonus;
	}

	/* Check damage dice - are they more than normal? */
	if (art->dd > kind->dd) {
		/* Difference of 3 or more? */
		if ((art->dd - kind->dd) > 2) {
			file_putf(log_file, "Adding 1 for super-charged damage dice!\n");
			(data->art_probs[ART_IDX_MELEE_DICE_SUPER])++;
		} else {
			file_putf(log_file, "Adding 1 for extra damage dice.\n");
			(data->art_probs[ART_IDX_MELEE_DICE])++;
		}
	}

	/* Check weight - is it different from normal? */
	if (art->weight != kind->weight) {
		file_putf(log_file, "Adding 1 for unusual weight.\n");
		(data->art_probs[ART_IDX_MELEE_WEIGHT])++;
	}

	/* Do we have 3 or more extra blows? (Unlikely) */
	if (art->modifiers[OBJ_MOD_BLOWS] > 2) {
		file_putf(log_file, "Adding 1 for supercharged blows (3 or more!)\n");
		(data->art_probs[ART_IDX_MELEE_BLOWS_SUPER])++;
	} else if (art->modifiers[OBJ_MOD_BLOWS] > 0) {
		file_putf(log_file, "Adding 1 for extra blows\n");
		(data->art_probs[ART_IDX_MELEE_BLOWS])++;
	}

	/* Aggravation */
	if (of_has(art->flags, OF_AGGRAVATE)) {
		file_putf(log_file, "Adding 1 for aggravation - weapon\n");
		data->art_probs[ART_IDX_WEAPON_AGGR]++;
	}

	/* Blessed weapon? */
	if (of_has(art->flags, OF_BLESSED)) {
		file_putf(log_file, "Adding 1 for blessed weapon\n");
		(data->art_probs[ART_IDX_MELEE_BLESS])++;
	}

	/* See invisible? */
	if (of_has(art->flags, OF_SEE_INVIS)) {
		file_putf(log_file, "Adding 1 for see invisible (weapon case)\n");
		(data->art_probs[ART_IDX_MELEE_SINV])++;
	}

	/* Check for tunnelling ability */
	if (art->modifiers[OBJ_MOD_TUNNEL] > 0) {
		file_putf(log_file, "Adding 1 for tunnelling bonus.\n");
		(data->art_probs[ART_IDX_MELEE_TUNN])++;
	}

	/* Count brands and slays */
	if (art->slays) {
		bonus = slay_count(art->slays);
		data->art_probs[ART_IDX_MELEE_SLAY] += bonus;
		file_putf(log_file, "Adding %d for slays\n", bonus);
	}
	if (art->brands) {
		bonus = brand_count(art->brands);
		data->art_probs[ART_IDX_MELEE_BRAND] += bonus;
		file_putf(log_file, "Adding %d for brands\n", bonus);
	}
}

/**
 * Count combat abilities on bows
 */
void count_bow_abilities(const struct artifact *art,
						 struct artifact_set_data *data)
{
	int bonus;
	struct object_kind *kind = lookup_kind(art->tval, art->sval);
	int min_to_h = randcalc(kind->to_h, 0, MINIMISE);
	int min_to_d = randcalc(kind->to_d, 0, MINIMISE);
	int min_to_a = randcalc(kind->to_a, 0, MINIMISE);

	/* To-hit */
	bonus = (art->to_h - min_to_h - data->hit_startval) / data->hit_increment;
	if (bonus > 0)
		file_putf(log_file, "Adding %d instances of extra to-hit bonus for weapon\n", bonus);
	else if (bonus < 0)
		file_putf(log_file, "Subtracting %d instances of extra to-hit bonus for weapon\n", bonus);
	data->art_probs[ART_IDX_WEAPON_HIT] += bonus;

	/* To-dam */
	bonus = (art->to_d - min_to_d - data->dam_startval) / data->dam_increment;
	if (bonus > 0)
		file_putf(log_file, "Adding %d instances of extra to-dam bonus for weapon\n", bonus);
	else
		file_putf(log_file, "Subtracting %d instances of extra to-dam bonus for weapon\n", bonus);
	data->art_probs[ART_IDX_WEAPON_DAM] += bonus;

	/* Armor class */
	bonus = (art->to_a - min_to_a - data->ac_startval) / data->ac_increment;
	if (bonus > 0) {
		file_putf(log_file, "Adding %d for AC bonus - general\n", bonus);
		(data->art_probs[ART_IDX_GEN_AC]) += bonus;
	}

	/* Aggravation */
	if (of_has(art->flags, OF_AGGRAVATE)) {
		file_putf(log_file, "Adding 1 for aggravation - weapon\n");
		data->art_probs[ART_IDX_WEAPON_AGGR]++;
	}

	/* Do we have 3 or more extra shots? (Unlikely) */
	if (art->modifiers[OBJ_MOD_SHOTS] > 2) {
		file_putf(log_file, "Adding 1 for supercharged shots (3 or more!)\n");
		(data->art_probs[ART_IDX_BOW_SHOTS_SUPER])++;
	} else if (art->modifiers[OBJ_MOD_SHOTS] > 0) {
		file_putf(log_file, "Adding 1 for extra shots\n");
		(data->art_probs[ART_IDX_BOW_SHOTS])++;
	}

	/* Do we have 3 or more extra might? (Unlikely) */
	if (art->modifiers[OBJ_MOD_MIGHT] > 2) {
		file_putf(log_file, "Adding 1 for supercharged might (3 or more!)\n");
		(data->art_probs[ART_IDX_BOW_MIGHT_SUPER])++;
	} else if (art->modifiers[OBJ_MOD_MIGHT] > 0) {
		file_putf(log_file, "Adding 1 for extra might\n");
		(data->art_probs[ART_IDX_BOW_MIGHT])++;
	}

	/* Count brands and slays */
	if (art->slays) {
		int bonus = slay_count(art->slays);
		data->art_probs[ART_IDX_BOW_SLAY] += bonus;
		file_putf(log_file, "Adding %d for slays\n", bonus);
	}
	if (art->brands) {
		int bonus = brand_count(art->brands);
		data->art_probs[ART_IDX_BOW_BRAND] += bonus;
		file_putf(log_file, "Adding %d for brands\n", bonus);
	}
}

/**
 * Handle nonweapon combat abilities
 */
void count_nonweapon_abilities(const struct artifact *art,
							   struct artifact_set_data *data)
{
	struct object_kind *kind = lookup_kind(art->tval, art->sval);
	int to_hit = art->to_h - randcalc(kind->to_h, 0, MINIMISE);
	int to_dam = art->to_d - randcalc(kind->to_d, 0, MINIMISE);
	int to_a = art->to_a - randcalc(kind->to_a, 0, MINIMISE)
		- data->ac_startval;
	int bonus = to_a / data->ac_increment;

	/* Armor class */
	if (bonus > 0) {
		if (art->to_a > 20) {
			file_putf(log_file, "Adding %d for supercharged AC\n", bonus);
			(data->art_probs[ART_IDX_GEN_AC_SUPER])++;
		} else if (art->tval == TV_BOOTS) {
			file_putf(log_file, "Adding %d for AC bonus - boots\n", bonus);
			(data->art_probs[ART_IDX_BOOT_AC]) += bonus;
		} else if (art->tval == TV_GLOVES) {
			file_putf(log_file, "Adding %d for AC bonus - gloves\n", bonus);
			(data->art_probs[ART_IDX_GLOVE_AC]) += bonus;
		} else if (art->tval == TV_HELM || art->tval == TV_CROWN) {
			file_putf(log_file, "Adding %d for AC bonus - hat\n", bonus);
			(data->art_probs[ART_IDX_HELM_AC]) += bonus;
		} else if (art->tval == TV_SHIELD) {
			file_putf(log_file, "Adding %d for AC bonus - shield\n", bonus);
			(data->art_probs[ART_IDX_SHIELD_AC]) += bonus;
		} else if (art->tval == TV_CLOAK) {
			file_putf(log_file, "Adding %d for AC bonus - cloak\n", bonus);
			(data->art_probs[ART_IDX_CLOAK_AC]) += bonus;
		} else if (art->tval == TV_SOFT_ARMOR ||
				   art->tval == TV_HARD_ARMOR ||
				   art->tval == TV_DRAG_ARMOR) {
			file_putf(log_file, "Adding %d for AC bonus - body armor\n", bonus);
			(data->art_probs[ART_IDX_ARMOR_AC]) += bonus;
		} else {
			file_putf(log_file, "Adding %d for AC bonus - general\n", bonus);
			(data->art_probs[ART_IDX_GEN_AC]) += bonus;
		}
	}

	/* To hit and dam to bonuses */
	if ((to_hit > 0) && (to_dam > 0)) {
		bonus = (to_hit + to_dam) / (data->hit_increment + data->dam_increment);
		if (bonus > 0) {
			if (art->tval == TV_GLOVES) {
				file_putf(log_file, "Adding %d instances of extra to-hit and to-dam bonus for gloves\n", bonus);
				(data->art_probs[ART_IDX_GLOVE_HIT_DAM]) += bonus;
			} else {
				file_putf(log_file, "Adding %d instances of extra to-hit and to-dam bonus for non-weapon\n", bonus);
				(data->art_probs[ART_IDX_NONWEAPON_HIT_DAM]) += bonus;
			}
		}
	} else if (to_hit > 0) {
		bonus = to_hit / data->hit_increment;
		if (bonus > 0) {
			file_putf(log_file, "Adding %d instances of extra to-hit bonus for non-weapon\n", bonus);
			(data->art_probs[ART_IDX_NONWEAPON_HIT]) += bonus;
		}
	} else if (to_dam > 0) {
		bonus = to_dam / data->dam_increment;
		if (bonus > 0) {
			file_putf(log_file, "Adding %d instances of extra to-dam bonus for non-weapon\n", bonus);
			(data->art_probs[ART_IDX_NONWEAPON_DAM]) += bonus;
		}
	}

	/* Check weight - is it different from normal? */
	if (art->weight != kind->weight) {
		file_putf(log_file, "Adding 1 for unusual weight.\n");
		(data->art_probs[ART_IDX_ALLARMOR_WEIGHT])++;
	}

	/* Aggravation */
	if (of_has(art->flags, OF_AGGRAVATE)) {
		file_putf(log_file, "Adding 1 for aggravation - nonweapon\n");
		(data->art_probs[ART_IDX_NONWEAPON_AGGR])++;
	}

	/* Count brands and slays */
	if (art->slays) {
		bonus = slay_count(art->slays);
		data->art_probs[ART_IDX_NONWEAPON_SLAY] += bonus;
		file_putf(log_file, "Adding %d for slays\n", bonus);
	}
	if (art->brands) {
		bonus = brand_count(art->brands);
		data->art_probs[ART_IDX_NONWEAPON_BRAND] += bonus;
		file_putf(log_file, "Adding %d for brands\n", bonus);
	}

	/* Blows */
	if (art->modifiers[OBJ_MOD_BLOWS] > 0) {
		file_putf(log_file, "Adding 1 for extra blows on nonweapon\n");
		(data->art_probs[ART_IDX_NONWEAPON_BLOWS])++;
	}

	/* Shots */
	if (art->modifiers[OBJ_MOD_SHOTS] > 0) {
		file_putf(log_file, "Adding 1 for extra shots on nonweapon\n");
		(data->art_probs[ART_IDX_NONWEAPON_SHOTS])++;
	}

	/* Check for tunnelling ability */
	if (art->modifiers[OBJ_MOD_TUNNEL] > 0) {
		file_putf(log_file, "Adding 1 for tunnelling bonus - general.\n");
		(data->art_probs[ART_IDX_GEN_TUNN])++;
	}
}


/**
 * Count modifiers
 */
void count_modifiers(const struct artifact *art, struct artifact_set_data *data)
{
	int num = 0;

	/* Stat bonuses.  Add up the number of individual bonuses */
	if (art->modifiers[OBJ_MOD_STR] > 0) num++;
	if (art->modifiers[OBJ_MOD_INT] > 0) num++;
	if (art->modifiers[OBJ_MOD_WIS] > 0) num++;
	if (art->modifiers[OBJ_MOD_DEX] > 0) num++;
	if (art->modifiers[OBJ_MOD_CON] > 0) num++;

	/* Handle a few special cases separately. */
	if ((art->tval == TV_HELM || art->tval == TV_CROWN) &&
		(art->modifiers[OBJ_MOD_WIS] > 0 || art->modifiers[OBJ_MOD_INT] > 0)) {
		/* Handle WIS and INT on helms and crowns */
		if (art->modifiers[OBJ_MOD_WIS] > 0) {
			file_putf(log_file, "Adding 1 for WIS bonus on headgear.\n");
			(data->art_probs[ART_IDX_HELM_WIS])++;
			/* Counted this one separately so subtract it here */
			num--;
		}
		if (art->modifiers[OBJ_MOD_INT] > 0) {
			file_putf(log_file, "Adding 1 for INT bonus on headgear.\n");
			(data->art_probs[ART_IDX_HELM_INT])++;
			/* Counted this one separately so subtract it here */
			num--;
		}
	} else if ((art->tval == TV_SOFT_ARMOR ||
				art->tval == TV_HARD_ARMOR ||
				art->tval == TV_DRAG_ARMOR) &&
			   art->modifiers[OBJ_MOD_CON] > 0) {
		/* Handle CON bonus on armor */
		file_putf(log_file, "Adding 1 for CON bonus on body armor.\n");

		(data->art_probs[ART_IDX_ARMOR_CON])++;
		/* Counted this one separately so subtract it here */
		num--;
	} else if (art->tval == TV_GLOVES && art->modifiers[OBJ_MOD_DEX] > 0) {
		/* Handle DEX bonus on gloves */
		file_putf(log_file, "Adding 1 for DEX bonus on gloves.\n");
		(data->art_probs[ART_IDX_GLOVE_DEX])++;
		/* Counted this one separately so subtract it here */
		num--;
	}

	/* Now the general case */
	if (num > 0) {
		/* There are some bonuses that weren't handled above */
		file_putf(log_file, "Adding %d for stat bonuses - general.\n", num);
			(data->art_probs[ART_IDX_GEN_STAT]) += num;
	}

	/* Handle stealth, including a couple of special cases */
	if (art->modifiers[OBJ_MOD_STEALTH] > 0) {
		if (art->tval == TV_BOOTS) {
			file_putf(log_file, "Adding 1 for stealth bonus on boots.\n");
			(data->art_probs[ART_IDX_BOOT_STEALTH])++;
		} else if (art->tval == TV_CLOAK) {
			file_putf(log_file, "Adding 1 for stealth bonus on cloak.\n");
			(data->art_probs[ART_IDX_CLOAK_STEALTH])++;
		} else if (art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
				   art->tval == TV_DRAG_ARMOR) {
			file_putf(log_file, "Adding 1 for stealth bonus on armor.\n");
			(data->art_probs[ART_IDX_ARMOR_STEALTH])++;
		} else {
			/* General case */
			file_putf(log_file, "Adding 1 for stealth bonus - general.\n");
			(data->art_probs[ART_IDX_GEN_STEALTH])++;
		}
	}

	/* Handle searching bonus - fully generic this time */
	if (art->modifiers[OBJ_MOD_SEARCH] > 0) {
		file_putf(log_file, "Adding 1 for search bonus - general.\n");
		(data->art_probs[ART_IDX_GEN_SEARCH])++;
	}

	/* Handle infravision bonus - fully generic */
	if (art->modifiers[OBJ_MOD_INFRA] > 0) {
		file_putf(log_file, "Adding 1 for infravision bonus - general.\n");
		(data->art_probs[ART_IDX_GEN_INFRA])++;
	}

	/* Handle damage reduction bonus - fully generic */
	if (art->modifiers[OBJ_MOD_DAM_RED] > 0) {
		file_putf(log_file, "Adding 1 for damage reduction bonus - general.\n");
		(data->art_probs[ART_IDX_GEN_DAM_RED])++;
	}

	/* Handle moves bonus - fully generic */
	if (art->modifiers[OBJ_MOD_MOVES] > 0) {
		file_putf(log_file, "Adding 1 for moves bonus - general.\n");
		(data->art_probs[ART_IDX_GEN_MOVES])++;
	}

	/* Speed - boots handled separately.
	 * This is something of a special case in that we use the same
	 * frequency for the supercharged value and the normal value.
	 * We get away with this by using a somewhat lower average value
	 * for the supercharged ability than in the basic set (around
	 * +7 or +8 - c.f. Ringil and the others at +10 and upwards).
	 * This then allows us to add an equal number of
	 * small bonuses around +3 or so without unbalancing things.
	 */
	if (art->modifiers[OBJ_MOD_SPEED] > 0) {
		if (art->modifiers[OBJ_MOD_SPEED] > 7) {
			/* Supercharge case */
			file_putf(log_file, "Adding 1 for supercharged speed bonus!\n");
			(data->art_probs[ART_IDX_GEN_SPEED_SUPER])++;
		} else if (art->tval == TV_BOOTS) {
			/* Handle boots separately */
			file_putf(log_file, "Adding 1 for normal speed bonus on boots.\n");
			(data->art_probs[ART_IDX_BOOT_SPEED])++;
		} else {
			file_putf(log_file, "Adding 1 for normal speed bonus - general.\n");
			(data->art_probs[ART_IDX_GEN_SPEED])++;
		}
	}

	/* Handle permanent light */
	if (art->modifiers[OBJ_MOD_LIGHT] > 0) {
		file_putf(log_file, "Adding 1 for light radius - general.\n");
		(data->art_probs[ART_IDX_GEN_LIGHT])++;
	}
}

/**
 * Count low resists and immunities.
 */
void count_low_resists(const struct artifact *art,
					   struct artifact_set_data *data)
{
	int num = 0;

	/* Count up immunities for this item, if any */
	if (art->el_info[ELEM_ACID].res_level == 3) num++;
	if (art->el_info[ELEM_ELEC].res_level == 3) num++;
	if (art->el_info[ELEM_FIRE].res_level == 3) num++;
	if (art->el_info[ELEM_COLD].res_level == 3) num++;
	file_putf(log_file, "Adding %d for immunities.\n", num);

	(data->art_probs[ART_IDX_GEN_IMMUNE]) += num;

	/* Count up low resists (not the type, just the number) */
	num = 0;
	if (art->el_info[ELEM_ACID].res_level == 1) num++;
	if (art->el_info[ELEM_ELEC].res_level == 1) num++;
	if (art->el_info[ELEM_FIRE].res_level == 1) num++;
	if (art->el_info[ELEM_COLD].res_level == 1) num++;

	if (num) {
		/* Shields treated separately */
		if (art->tval == TV_SHIELD) {
			file_putf(log_file, "Adding %d for low resists on shield.\n",
					  num);
			(data->art_probs[ART_IDX_SHIELD_LRES]) += num;
		} else if (art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
				   art->tval == TV_DRAG_ARMOR) {
			/* Armor also treated separately */
			if (num == 4) {
				/* Special case: armor has all four low resists */
				file_putf(log_file,
						  "Adding 1 for ALL LOW RESISTS on body armor.\n");
				(data->art_probs[ART_IDX_ARMOR_ALLRES])++;
			} else {
				/* Just tally up the resists as usual */
				file_putf(log_file,
						  "Adding %d for low resists on body armor.\n", num);
				(data->art_probs[ART_IDX_ARMOR_LRES]) += num;
			}
		} else {
			/* General case */
			file_putf(log_file, "Adding %d for low resists - general.\n", num);
			(data->art_probs[ART_IDX_GEN_LRES]) += num;
		}
	}
}

/**
 * Count high resists and protections.
 */
void count_high_resists(const struct artifact *art,
						struct artifact_set_data *data)
{
	int num = 0;

	/* If the item is body armor then count up all the high resists before
	 * going through them individually.  High resists are an important
	 * component of body armor so we track probability for them separately.
	 * The proportions of the high resists will be determined by the
	 * generic frequencies - this number just tracks the total. */
	if (art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
		art->tval == TV_DRAG_ARMOR) {
		if (art->el_info[ELEM_POIS].res_level == 1) num++;
		if (of_has(art->flags, OF_PROT_FEAR)) num++;
		if (art->el_info[ELEM_LIGHT].res_level == 1) num++;
		if (art->el_info[ELEM_DARK].res_level == 1) num++;
		if (of_has(art->flags, OF_PROT_BLIND)) num++;
		if (of_has(art->flags, OF_PROT_CONF)) num++;
		if (art->el_info[ELEM_SOUND].res_level == 1) num++;
		if (art->el_info[ELEM_SHARD].res_level == 1) num++;
		if (art->el_info[ELEM_NEXUS].res_level == 1) num++;
		if (art->el_info[ELEM_NETHER].res_level == 1) num++;
		if (art->el_info[ELEM_CHAOS].res_level == 1) num++;
		if (art->el_info[ELEM_DISEN].res_level == 1) num++;
		if (of_has(art->flags, OF_PROT_STUN)) num++;
		file_putf(log_file, "Adding %d for high resists on body armor.\n",
				  num);
		(data->art_probs[ART_IDX_ARMOR_HRES]) += num;
	}

	/* Now do the high resists individually */
	if (art->el_info[ELEM_POIS].res_level == 1) {
		/* Resist poison ability */
		file_putf(log_file, "Adding 1 for resist poison - general.\n");

		(data->art_probs[ART_IDX_GEN_RPOIS])++;
	}

	if (of_has(art->flags, OF_PROT_FEAR)) {
		/* Resist fear ability */
		file_putf(log_file, "Adding 1 for resist fear - general.\n");
		(data->art_probs[ART_IDX_GEN_RFEAR])++;
	}

	if (art->el_info[ELEM_LIGHT].res_level == 1) {
		/* Resist light ability */
		file_putf(log_file, "Adding 1 for resist light - general.\n");
		(data->art_probs[ART_IDX_GEN_RLIGHT])++;
	}

	if (art->el_info[ELEM_DARK].res_level == 1) {
		/* Resist dark ability */
		file_putf(log_file, "Adding 1 for resist dark - general.\n");
		(data->art_probs[ART_IDX_GEN_RDARK])++;
	}

	if (of_has(art->flags, OF_PROT_BLIND)) {
		/* Resist blind ability - helms/crowns are separate */
		if (art->tval == TV_HELM || art->tval == TV_CROWN) {
			file_putf(log_file, "Adding 1 for resist blindness - headgear.\n");
			(data->art_probs[ART_IDX_HELM_RBLIND])++;
		} else {
			/* General case */
			file_putf(log_file, "Adding 1 for resist blindness - general.\n");
			(data->art_probs[ART_IDX_GEN_RBLIND])++;
		}
	}

	if (of_has(art->flags, OF_PROT_CONF)) {
		/* Resist confusion ability */
		file_putf(log_file, "Adding 1 for resist confusion - general.\n");
		(data->art_probs[ART_IDX_GEN_RCONF])++;
	}

	if (art->el_info[ELEM_SOUND].res_level == 1) {
		/* Resist sound ability */
		file_putf(log_file, "Adding 1 for resist sound - general.\n");
		(data->art_probs[ART_IDX_GEN_RSOUND])++;
	}

	if (art->el_info[ELEM_SHARD].res_level == 1) {
		/* Resist shards ability */
		file_putf(log_file, "Adding 1 for resist shards - general.\n");
		(data->art_probs[ART_IDX_GEN_RSHARD])++;
	}

	if (art->el_info[ELEM_NEXUS].res_level == 1) {
		/* Resist nexus ability */
		file_putf(log_file, "Adding 1 for resist nexus - general.\n");
		(data->art_probs[ART_IDX_GEN_RNEXUS])++;
	}

	if (art->el_info[ELEM_NETHER].res_level == 1) {
		/* Resist nether ability */
		file_putf(log_file, "Adding 1 for resist nether - general.\n");
		(data->art_probs[ART_IDX_GEN_RNETHER])++;
	}

	if (art->el_info[ELEM_CHAOS].res_level == 1) {
		/* Resist chaos ability */
		file_putf(log_file, "Adding 1 for resist chaos - general.\n");
		(data->art_probs[ART_IDX_GEN_RCHAOS])++;
	}

	if (art->el_info[ELEM_DISEN].res_level == 1) {
		/* Resist disenchantment ability */
		file_putf(log_file, "Adding 1 for resist disenchantment - general.\n");
		(data->art_probs[ART_IDX_GEN_RDISEN])++;
	}

	if (of_has(art->flags, OF_PROT_STUN)) {
		/* Resist stunning ability */
		file_putf(log_file, "Adding 1 for res_stun - general.\n");
		(data->art_probs[ART_IDX_GEN_PSTUN])++;
	}
}

/**
 * General abilities.  This section requires a bit more work
 * than the others, because we have to consider cases where
 * a certain ability might be found in a particular item type.
 * For example, ESP is commonly found on headgear, so when
 * we count ESP we must add it to either the headgear or
 * general tally, depending on the base item.  This permits
 * us to have general abilities appear more commonly on a
 * certain item type.
 */
void count_abilities(const struct artifact *art, struct artifact_set_data *data)
{
	int num = 0;
	struct object_kind *kind = lookup_kind(art->tval, art->sval);

	if (flags_test(art->flags, OF_SIZE, OF_SUST_STR, OF_SUST_INT, OF_SUST_WIS,
				   OF_SUST_DEX, OF_SUST_CON, FLAG_END)) {
		/* Now do sustains, in a similar manner */
		num = 0;
		if (of_has(art->flags, OF_SUST_STR)) num++;
		if (of_has(art->flags, OF_SUST_INT)) num++;
		if (of_has(art->flags, OF_SUST_WIS)) num++;
		if (of_has(art->flags, OF_SUST_DEX)) num++;
		if (of_has(art->flags, OF_SUST_CON)) num++;
		file_putf(log_file, "Adding %d for stat sustains.\n", num);
		(data->art_probs[ART_IDX_GEN_SUST]) += num;
	}

	if (of_has(art->flags, OF_FREE_ACT)) {
		/* Free action - handle gloves separately */
		if (art->tval == TV_GLOVES) {
			file_putf(log_file, "Adding 1 for free action on gloves.\n");
			(data->art_probs[ART_IDX_GLOVE_FA])++;
		} else {
			file_putf(log_file, "Adding 1 for free action - general.\n");
			(data->art_probs[ART_IDX_GEN_FA])++;
		}
	}

	if (of_has(art->flags, OF_HOLD_LIFE)) {
		/* Hold life - do body armor separately */
		if( (art->tval == TV_SOFT_ARMOR) ||
			(art->tval == TV_HARD_ARMOR) ||
			(art->tval == TV_DRAG_ARMOR)) {
			file_putf(log_file, "Adding 1 for hold life on armor.\n");
			(data->art_probs[ART_IDX_ARMOR_HLIFE])++;
		} else {
			file_putf(log_file, "Adding 1 for hold life - general.\n");
			(data->art_probs[ART_IDX_GEN_HLIFE])++;
		}
	}

	if (of_has(art->flags, OF_FEATHER)) {
		/* Feather fall - handle boots separately */
		if (art->tval == TV_BOOTS) {
			file_putf(log_file, "Adding 1 for feather fall on boots.\n");
			(data->art_probs[ART_IDX_BOOT_FEATHER])++;
		} else {
			file_putf(log_file, "Adding 1 for feather fall - general.\n");
			(data->art_probs[ART_IDX_GEN_FEATHER])++;
		}
	}

	if (of_has(art->flags, OF_SEE_INVIS)) {
		/*
		 * Handle see invisible - do helms / crowns separately
		 * (Weapons were done already so exclude them)
		 */
		if(!(art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
			 art->tval == TV_POLEARM || art->tval == TV_SWORD)) {
			if (art->tval == TV_HELM || art->tval == TV_CROWN) {
				file_putf(log_file, "Adding 1 for see invisible - headgear.\n");
				(data->art_probs[ART_IDX_HELM_SINV])++;
			} else {
				file_putf(log_file, "Adding 1 for see invisible - general.\n");
				(data->art_probs[ART_IDX_GEN_SINV])++;
			}
		}
	}

	if (of_has(art->flags, OF_TELEPATHY)) {
		/* ESP case.  Handle helms/crowns separately. */
		if (art->tval == TV_HELM || art->tval == TV_CROWN) {
			file_putf(log_file, "Adding 1 for ESP on headgear.\n");
			(data->art_probs[ART_IDX_HELM_ESP])++;
		} else {
			file_putf(log_file, "Adding 1 for ESP - general.\n");
			(data->art_probs[ART_IDX_GEN_ESP])++;
		}
	}

	if (of_has(art->flags, OF_SLOW_DIGEST)) {
		/* Slow digestion case - generic. */
		file_putf(log_file, "Adding 1 for slow digestion - general.\n");
		(data->art_probs[ART_IDX_GEN_SDIG])++;
	}

	if (of_has(art->flags, OF_REGEN)) {
		/* Regeneration case - generic. */
		file_putf(log_file, "Adding 1 for regeneration - general.\n");
		(data->art_probs[ART_IDX_GEN_REGEN])++;
	}

	if (of_has(art->flags, OF_TRAP_IMMUNE)) {
		/* Trap immunity - handle boots separately */
		if (art->tval == TV_BOOTS) {
			file_putf(log_file, "Adding 1 for trap immunity on boots.\n");
			(data->art_probs[ART_IDX_BOOT_TRAP_IMM])++;
		} else {
			file_putf(log_file, "Adding 1 for trap immunity - general.\n");
			(data->art_probs[ART_IDX_GEN_TRAP_IMM])++;
		}
	}


	if (art->activation || kind->activation) {
		/* Activation */
		file_putf(log_file, "Adding 1 for activation.\n");
		(data->art_probs[ART_IDX_GEN_ACTIV])++;
	}
}

/**
 * Parse the standard artifacts and count up the frequencies of the various
 * abilities.
 */
static void collect_artifact_data(struct artifact_set_data *data)
{
	size_t i;

	/* Go through the list of all artifacts */
	for (i = 0; i < z_info->a_max; i++) {
		struct object_kind *kind;
		const struct artifact *art = &a_info[i];

		file_putf(log_file, "Current artifact index is %d\n", i);

		/* Don't parse cursed or null items */
		if (data->base_power[i] < 0 || art->tval == 0) continue;

		/* Get a pointer to the base item for this artifact */
		kind = lookup_kind(art->tval, art->sval);

		/* Special cases -- don't parse these! */
		if (strstr(art->name, "The One Ring") ||
			kf_has(kind->kind_flags, KF_QUEST_ART))
			continue;

		/* Add the base item tval to the tv_probs array */
		data->tv_probs[kind->tval]++;
		file_putf(log_file, "Base item is %d\n", kind->kidx);

		/* Count combat abilities broken up by type */
		if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
			art->tval == TV_POLEARM || art->tval == TV_SWORD) {
			count_weapon_abilities(art, data);
		} else if (art->tval == TV_BOW) {
			count_bow_abilities(art, data);
		} else {
			count_nonweapon_abilities(art, data);
		}

		/* Count other properties  */
		count_modifiers(art, data);
		count_low_resists(art, data);
		count_high_resists(art, data);
		count_abilities(art, data);
	}
}

/**
 * Rescale the abilities so that dependent / independent abilities are
 * comparable.  We do this by rescaling the frequencies for item-dependent
 * abilities as though the entire set was made up of that item type.  For
 * example, if one bow out of three has extra might, and there are 120
 * artifacts in the full set, we rescale the frequency for extra might to
 * 40 (if we had 120 randart bows, about 40 would have extra might).
 *
 * This will allow us to compare the frequencies of all ability types,
 * no matter what the dependency.  We assume that generic abilities (like
 * resist fear in the current version) don't need rescaling.  This
 * introduces some inaccuracy in cases where specific instances of an
 * ability (like INT bonus on helms) have been counted separately -
 * ideally we should adjust for this in the general case.  However, as
 * long as this doesn't occur too often, it shouldn't be a big issue.
 *
 * The following loops look complicated, but they are simply equivalent
 * to going through each of the relevant ability types one by one.
 */
static void rescale_freqs(struct artifact_set_data *data)
{
	size_t i;
	s32b temp;

	/* Bow-only abilities */
	for (i = 0; i < N_ELEMENTS(art_idx_bow); i++)
		data->art_probs[art_idx_bow[i]] =
			(data->art_probs[art_idx_bow[i]] * data->total)
			/ data->bow_total;

	/* All weapon abilities */
	for (i = 0; i < N_ELEMENTS(art_idx_weapon); i++)
		data->art_probs[art_idx_weapon[i]] =
			(data->art_probs[art_idx_weapon[i]] * data->total)
			/ (data->bow_total + data->melee_total);

	/* Corresponding non-weapon abilities */
	temp = data->total - data->melee_total - data->bow_total;
	for (i = 0; i < N_ELEMENTS(art_idx_nonweapon); i++)
		data->art_probs[art_idx_nonweapon[i]] =
			(data->art_probs[art_idx_nonweapon[i]] * data->total)
			/ temp;

	/* All melee weapon abilities */
	for (i = 0; i < N_ELEMENTS(art_idx_melee); i++)
		data->art_probs[art_idx_melee[i]] =
			(data->art_probs[art_idx_melee[i]] * data->total)
			/ data->melee_total;

	/* All general armor abilities */
	temp = data->armor_total + data->boot_total + data->shield_total +
		data->headgear_total + data->cloak_total + data->glove_total;
	for (i = 0; i < N_ELEMENTS(art_idx_allarmor); i++)
		data->art_probs[art_idx_allarmor[i]] =
			(data->art_probs[art_idx_allarmor[i]] *	data->total)
			/ temp;

	/* Boots */
	for (i = 0; i < N_ELEMENTS(art_idx_boot); i++)
		data->art_probs[art_idx_boot[i]] =
			(data->art_probs[art_idx_boot[i]] *	data->total)
			/ data->boot_total;

	/* Gloves */
	for (i = 0; i < N_ELEMENTS(art_idx_glove); i++)
		data->art_probs[art_idx_glove[i]] =
			(data->art_probs[art_idx_glove[i]] * data->total)
			/ data->glove_total;

	/* Headgear */
	for (i = 0; i < N_ELEMENTS(art_idx_headgear); i++)
		data->art_probs[art_idx_headgear[i]] =
			(data->art_probs[art_idx_headgear[i]] *	data->total)
			/ data->headgear_total;

	/* Shields */
	for (i = 0; i < N_ELEMENTS(art_idx_shield); i++)
		data->art_probs[art_idx_shield[i]] =
			(data->art_probs[art_idx_shield[i]] *data->total)
			/ data->shield_total;

	/* Cloaks */
	for (i = 0; i < N_ELEMENTS(art_idx_cloak); i++)
		data->art_probs[art_idx_cloak[i]] =
			(data->art_probs[art_idx_cloak[i]] * data->total)
			/ data->cloak_total;

	/* Body armor */
	for (i = 0; i < N_ELEMENTS(art_idx_armor); i++)
		data->art_probs[art_idx_armor[i]] =
			(data->art_probs[art_idx_armor[i]] * data->total)
			/ data->armor_total;

	/*
	 * All others are general case and don't need to be rescaled,
	 * unless the algorithm is getting too clever about separating
	 * out individual cases (in which case some logic should be
	 * added for them in rescale_freqs()).
	 */
}

/**
 * Adjust the parsed frequencies for any peculiarities of the
 * algorithm.  For example, if stat bonuses and sustains are
 * being added in a correlated fashion, it will tend to push
 * the frequencies up for both of them.  In this method we
 * compensate for cases like this by applying corrective
 * scaling.
 */
static void adjust_freqs(struct artifact_set_data *data)
{
	/*
	 * Enforce minimum values for any frequencies that might potentially
	 * be missing in the standard set, especially supercharged ones.
	 * Numbers here represent the average number of times this ability
	 * would appear if the entire randart set was eligible to receive
	 * it (so in the case of a bow ability: if the set was all bows).
	 *
	 * Note that low numbers here for very specialized abilities could
	 * mean that there's a good chance this ability will not appear in
	 * a given randart set.  If this is a problem, raise the number.
	 */
	if (data->art_probs[ART_IDX_GEN_RFEAR] < 5)
		data->art_probs[ART_IDX_GEN_RFEAR] = 5;
	if (data->art_probs[ART_IDX_MELEE_DICE_SUPER] < 5)
		data->art_probs[ART_IDX_MELEE_DICE_SUPER] = 5;
	if (data->art_probs[ART_IDX_BOW_SHOTS_SUPER] < 5)
		data->art_probs[ART_IDX_BOW_SHOTS_SUPER] = 5;
	if (data->art_probs[ART_IDX_BOW_MIGHT_SUPER] < 5)
		data->art_probs[ART_IDX_BOW_MIGHT_SUPER] = 5;
	if (data->art_probs[ART_IDX_MELEE_BLOWS_SUPER] < 5)
		data->art_probs[ART_IDX_MELEE_BLOWS_SUPER] = 5;
	if (data->art_probs[ART_IDX_GEN_SPEED_SUPER] < 5)
		data->art_probs[ART_IDX_GEN_SPEED_SUPER] = 5;
	if (data->art_probs[ART_IDX_GEN_AC] < 5)
		data->art_probs[ART_IDX_GEN_AC] = 5;
	if (data->art_probs[ART_IDX_GEN_TUNN] < 5)
		data->art_probs[ART_IDX_GEN_TUNN] = 5;
	if (data->art_probs[ART_IDX_NONWEAPON_BRAND] < 2)
		data->art_probs[ART_IDX_NONWEAPON_BRAND] = 2;
	if (data->art_probs[ART_IDX_NONWEAPON_SLAY] < 2)
		data->art_probs[ART_IDX_NONWEAPON_SLAY] = 2;
	if (data->art_probs[ART_IDX_BOW_BRAND] < 2)
		data->art_probs[ART_IDX_BOW_BRAND] = 2;
	if (data->art_probs[ART_IDX_BOW_SLAY] < 2)
		data->art_probs[ART_IDX_BOW_SLAY] = 2;
	if (data->art_probs[ART_IDX_NONWEAPON_BLOWS] < 2)
		data->art_probs[ART_IDX_NONWEAPON_BLOWS] = 2;
	if (data->art_probs[ART_IDX_NONWEAPON_SHOTS] < 2)
		data->art_probs[ART_IDX_NONWEAPON_SHOTS] = 2;
	if (data->art_probs[ART_IDX_GEN_AC_SUPER] < 5)
		data->art_probs[ART_IDX_GEN_AC_SUPER] = 5;
	if (data->art_probs[ART_IDX_MELEE_AC] < 5)
		data->art_probs[ART_IDX_MELEE_AC] = 5;
	if (data->art_probs[ART_IDX_GEN_PSTUN] < 3)
		data->art_probs[ART_IDX_GEN_PSTUN] = 3;

	/* Cut aggravation frequencies in half since they're used twice */
	data->art_probs[ART_IDX_NONWEAPON_AGGR] /= 2;
	data->art_probs[ART_IDX_WEAPON_AGGR] /= 2;
}

/**
 * Parse the artifacts and write frequencies of their abilities and
 * base object kinds. 
 *
 * This is used to give dynamic generation probabilities.
 */
static void parse_frequencies(struct artifact_set_data *data)
{
	size_t i;
	int j;

	file_putf(log_file, "\n****** BEGINNING GENERATION OF FREQUENCIES\n\n");

	/* Zero the frequencies for artifact attributes */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		data->art_probs[i] = 0;

	collect_artifact_data(data);

	/* Big hack, reduce frequencies of sharp weapons */
	for (i = 0; i < TV_MAX; i++) {
		if ((i == TV_SWORD) || (i == TV_POLEARM)) {
			data->tv_probs[i] *= 2;
			data->tv_probs[i] /= 3;
		}
	}

	/* Print out some of the abilities, to make sure that everything's fine */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		file_putf(log_file, "Frequency of ability %d: %d\n", i,
				  data->art_probs[i]);

	for (i = 0; i < TV_MAX; i++)
		file_putf(log_file, "Frequency of %s: %d\n", tval_find_name(i),
				  data->tv_probs[i]);

	/* Rescale frequencies */
	rescale_freqs(data);

	/* Perform any additional rescaling and adjustment, if required. */
	adjust_freqs(data);

	/* Log the final frequencies to check that everything's correct */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		file_putf(log_file,  "Rescaled frequency of ability %d: %d\n", i,
				  data->art_probs[i]);

	/* Build a cumulative frequency table for tvals */
	for (i = 0; i < TV_MAX; i++)
		for (j = i; j < TV_MAX; j++)
			data->tv_freq[j] += data->tv_probs[i];

	/* Print out the frequency table, for verification */
	for (i = 0; i < TV_MAX; i++)
		file_putf(log_file, "Cumulative frequency of %s is: %d\n",
				  tval_find_name(i), data->tv_freq[i]);
}

/**
 * ------------------------------------------------------------------------
 * Generation of a random artifact
 * ------------------------------------------------------------------------ */
/**
 * Pick a random base item tval
 */
static int get_base_item_tval(struct artifact_set_data *data)
{
	int tval = 0;
	int r = randint1(data->tv_freq[TV_MAX - 1]);

	/* Get a tval based on original artifact tval frequencies */
	while (r > data->tv_freq[tval]) {
		tval++;
	}

	return tval;
}

/**
 * Pick a random base item from artifact data and a tval
 */
static struct object_kind *get_base_item(struct artifact_set_data *data,
										 int tval)
{
	struct object_kind *kind = NULL;
	char name[120] = "";
	int start = 1;

	/* Restrict to appropriate kinds if jewellery */
	if ((tval == TV_RING) || (tval == TV_AMULET)) {
		struct object_kind *test_kind = lookup_kind(tval, start);
		while (test_kind->kidx < z_info->ordinary_kind_max) {
			start++;
			test_kind = lookup_kind(tval, start);
		}
	}

	/* Pick an sval for that tval at random */
	while (!kind) {
		int r = start + randint0(kb_info[tval].num_svals - start + 1);
		kind = lookup_kind(tval, r);

		/* No items based on quest artifacts or elven rings */
		if (strstr(kind->name, "Ring of") ||
			kf_has(kind->kind_flags, KF_QUEST_ART))
				kind = NULL;
	}

	object_short_name(name, sizeof name, kind->name);
	file_putf(log_file, "Creating %s\n", name);
	return kind;
}

/**
 * Add basic data to an artifact of a given object kind
 */
void artifact_prep(struct artifact *art, const struct object_kind *kind,
				   struct artifact_set_data *data)
{
	int i;

	art->tval = kind->tval;
	art->sval = kind->sval;
	art->to_h = randcalc(kind->to_h, 0, MINIMISE);
	art->to_d = randcalc(kind->to_d, 0, MINIMISE);
	art->to_a = randcalc(kind->to_a, 0, MINIMISE);
	art->ac = kind->ac;
	art->dd = kind->dd;
	art->ds = kind->ds;
	art->weight = kind->weight;
	of_copy(art->flags, kind->flags);
	mem_free(art->slays);
	art->slays = NULL;
	copy_slays(&art->slays, kind->slays);
	mem_free(art->brands);
	art->brands = NULL;
	copy_brands(&art->brands, kind->brands);
	art->activation = NULL;
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		art->modifiers[i] = randcalc(kind->modifiers[i], 0, MINIMISE);
	}
	for (i = 0; i < ELEM_MAX; i++)
		art->el_info[i] = kind->el_info[i];

	/* Artifacts ignore everything */
	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++)
		art->el_info[i].flags |= EL_INFO_IGNORE;

	/* Assign basic stats to the artifact based on its artifact level */
	switch (kind->tval) {
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_SWORD:
		case TV_POLEARM:
			art->to_h += (s16b)(data->hit_startval / 2 +
								randint0(data->hit_startval));
			art->to_d += (s16b)(data->dam_startval / 2 +
								randint0(data->dam_startval));
			file_putf(log_file,
					  "Assigned basic stats, to_hit: %d, to_dam: %d\n",
					  art->to_h, art->to_d);
			break;
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			art->to_a += (s16b)(data->ac_startval / 2 +
								randint0(data->ac_startval));
			file_putf(log_file, "Assigned basic stats, AC bonus: %d\n",
					  art->to_a);
			break;
		case TV_LIGHT:
			of_off(art->flags, OF_TAKES_FUEL);
			of_off(art->flags, OF_BURNS_OUT);
			of_on(art->flags, OF_NO_FUEL);
			if (kind->kidx >= z_info->ordinary_kind_max) {
				art->modifiers[OBJ_MOD_LIGHT] = 3;
			}
			break;
		case TV_RING:
		case TV_AMULET:
		default:
			break;
	}
}


/**
 * Build a suitable frequency table for this item, based on the generated
 * frequencies.  The frequencies for any abilities that don't apply for
 * this item type will be set to zero.  First parameter is the artifact
 * for which to generate the frequency table.
 *
 * The second input parameter is a pointer to an array that the function
 * will use to store the frequency table.  The array must have size
 * ART_IDX_TOTAL.
 *
 * The resulting frequency table is cumulative for ease of use in the
 * weighted randomization algorithm.
 */
static void build_freq_table(struct artifact *art, int *freq,
							 struct artifact_set_data *data)
{
	int i;
	size_t j;
	int f_temp[ART_IDX_TOTAL];

	/* First, set everything to zero */
	for (i = 0; i < ART_IDX_TOTAL; i++) {
		f_temp[i] = 0;
		freq[i] = 0;
	}

	/* Now copy over appropriate frequencies for applicable abilities */
	/* Bow abilities */
	if (art->tval == TV_BOW) {
		size_t n = N_ELEMENTS(art_idx_bow);
		for (j = 0; j < n; j++)
			f_temp[art_idx_bow[j]] = data->art_probs[art_idx_bow[j]];
		}

	/* General weapon abilities */
	if (art->tval == TV_BOW || art->tval == TV_DIGGING ||
		art->tval == TV_HAFTED || art->tval == TV_POLEARM ||
		art->tval == TV_SWORD) {
		size_t n = N_ELEMENTS(art_idx_weapon);
		for (j = 0; j < n; j++)
			f_temp[art_idx_weapon[j]] = data->art_probs[art_idx_weapon[j]];
	}
	/* General non-weapon abilities */
	else {
		size_t n = N_ELEMENTS(art_idx_nonweapon);
		for (j = 0; j < n; j++)
			f_temp[art_idx_nonweapon[j]] =
				data->art_probs[art_idx_nonweapon[j]];
	}

	/* General melee abilities */
	if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
		art->tval == TV_POLEARM || art->tval == TV_SWORD) {
		size_t n = N_ELEMENTS(art_idx_melee);
		for (j = 0; j < n; j++)
			f_temp[art_idx_melee[j]] = data->art_probs[art_idx_melee[j]];
	}

	/* General armor abilities */
	if (art->tval == TV_BOOTS || art->tval == TV_GLOVES ||
		art->tval == TV_HELM || art->tval == TV_CROWN ||
		art->tval == TV_SHIELD || art->tval == TV_CLOAK ||
		art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
		art->tval == TV_DRAG_ARMOR) {
		size_t n = N_ELEMENTS(art_idx_allarmor);
		for (j = 0; j < n; j++)
			f_temp[art_idx_allarmor[j]] = data->art_probs[art_idx_allarmor[j]];
	}

	/* Boot abilities */
	if (art->tval == TV_BOOTS) {
		size_t n = N_ELEMENTS(art_idx_boot);
		for (j = 0; j < n; j++)
			f_temp[art_idx_boot[j]] = data->art_probs[art_idx_boot[j]];
	}

	/* Glove abilities */
	if (art->tval == TV_GLOVES) {
		size_t n = N_ELEMENTS(art_idx_glove);
		for (j = 0; j < n; j++)
			f_temp[art_idx_glove[j]] = data->art_probs[art_idx_glove[j]];
	}

	/* Headgear abilities */
	if (art->tval == TV_HELM || art->tval == TV_CROWN) {
		size_t n = N_ELEMENTS(art_idx_headgear);
		for (j = 0; j < n; j++)
			f_temp[art_idx_headgear[j]] = data->art_probs[art_idx_headgear[j]];
	}

	/* Shield abilities */
	if (art->tval == TV_SHIELD) {
		size_t n = N_ELEMENTS(art_idx_shield);
		for (j = 0; j < n; j++)
			f_temp[art_idx_shield[j]] = data->art_probs[art_idx_shield[j]];
	}

	/* Cloak abilities */
	if (art->tval == TV_CLOAK) {
		size_t n = N_ELEMENTS(art_idx_cloak);
		for (j = 0; j < n; j++)
			f_temp[art_idx_cloak[j]] = data->art_probs[art_idx_cloak[j]];
	}

	/* Armor abilities */
	if (art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
		art->tval == TV_DRAG_ARMOR) {
		size_t n = N_ELEMENTS(art_idx_armor);
		for (j = 0; j < n; j++)
			f_temp[art_idx_armor[j]] = data->art_probs[art_idx_armor[j]];
	}

	/* General abilities - no constraint */
	for (j = 0; j < N_ELEMENTS(art_idx_gen); j++)
		f_temp[art_idx_gen[j]] = data->art_probs[art_idx_gen[j]];

	/*
	 * Now we have the correct individual frequencies, we build a cumulative
	 * frequency table for them.
	 */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		for (j = i; j < ART_IDX_TOTAL; j++)
			freq[j] += f_temp[i];

	/* Print out the frequency table, for verification */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		file_putf(log_file, "Cumulative frequency of ability %d is: %d\n", i,
				  freq[i]);
}

/**
 * Try to supercharge this item by running through the list of the supercharge
 * abilities and attempting to add each in turn.  An artifact only gets one
 * chance at each of these up front (if applicable).
 */
static void try_supercharge(struct artifact *art, s32b target_power,
							struct artifact_set_data *data)
{
	/* Huge damage dice or max blows - melee weapon only */
	if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
		art->tval == TV_POLEARM || art->tval == TV_SWORD) {
		/* Damage dice */
		if (randint0(z_info->a_max) <
			data->art_probs[ART_IDX_MELEE_DICE_SUPER]) {
			art->dd += 3 + randint0(4);
			file_putf(log_file, "Supercharging damage dice!  (Now %d dice)\n",
					  art->dd);
		} else if (randint0(z_info->a_max) <
				   data->art_probs[ART_IDX_MELEE_BLOWS_SUPER]) {
			/* Blows */
			art->modifiers[OBJ_MOD_BLOWS] = INHIBIT_BLOWS - 1;
			file_putf(log_file, "Supercharging melee blows! (%+d blows)\n",
				INHIBIT_BLOWS - 1);
		}
	}

	/* Bows - max might or shots */
	if (art->tval == TV_BOW) {
		if (randint0(z_info->a_max)
			< data->art_probs[ART_IDX_BOW_SHOTS_SUPER]) {
			art->modifiers[OBJ_MOD_SHOTS] = INHIBIT_SHOTS - 1;
			file_putf(log_file, "Supercharging shots! (%+d extra shots)\n",
				INHIBIT_SHOTS - 1);
		} else if (randint0(z_info->a_max) <
				   data->art_probs[ART_IDX_BOW_MIGHT_SUPER]) {
			art->modifiers[OBJ_MOD_MIGHT] = INHIBIT_MIGHT - 1;
			file_putf(log_file, "Supercharging might! (%+d extra might)\n",
					  INHIBIT_MIGHT - 1);
		}
	}

	/* Big speed bonus - any item (potentially) but more likely on boots */
	if (randint0(z_info->a_max) < data->art_probs[ART_IDX_GEN_SPEED_SUPER] ||
		(art->tval == TV_BOOTS && randint0(z_info->a_max) <
		data->art_probs[ART_IDX_BOOT_SPEED])) {
		art->modifiers[OBJ_MOD_SPEED] = 5 + randint0(6);
		if (INHIBIT_WEAK)
			art->modifiers[OBJ_MOD_SPEED] += randint1(3);
		if (INHIBIT_STRONG)
			art->modifiers[OBJ_MOD_SPEED] += 1 + randint1(6);
		file_putf(log_file, "Supercharging speed for this item!  (New speed bonus is %d)\n", art->modifiers[OBJ_MOD_SPEED]);
	}

	/* Big AC bonus */
	if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
		art->tval == TV_POLEARM || art->tval == TV_SWORD) {
		if (randint0(z_info->a_max) < data->art_probs[ART_IDX_MELEE_AC_SUPER]) {
			art->to_a += 19 + randint1(11);
			if (INHIBIT_WEAK)
				art->to_a += randint1(10);
			if (INHIBIT_STRONG)
				art->to_a += randint1(20);
			file_putf(log_file, "Supercharging AC! New AC bonus is %d\n",
					  art->to_a);
		}
	} else if ((art->tval != TV_BOW) &&
			   (randint0(z_info->a_max) <
				data->art_probs[ART_IDX_GEN_AC_SUPER])) {
		art->to_a += 19 + randint1(11);
		if (INHIBIT_WEAK)
			art->to_a += randint1(10);
		if (INHIBIT_STRONG)
			art->to_a += randint1(20);
		file_putf(log_file, "Supercharging AC! New AC bonus is %d\n",
				  art->to_a);
	}

	/* Aggravation */
	if (art->tval == TV_BOW || art->tval == TV_DIGGING ||
		art->tval == TV_HAFTED || art->tval == TV_POLEARM ||
		art->tval == TV_SWORD) {
		if ((randint0(z_info->a_max) < data->art_probs[ART_IDX_WEAPON_AGGR]) &&
		    (target_power > AGGR_POWER)) {
			of_on(art->flags, OF_AGGRAVATE);
			file_putf(log_file, "Adding aggravation\n");
		}
	} else {
		if ((randint0(z_info->a_max)
			 < data->art_probs[ART_IDX_NONWEAPON_AGGR]) &&
			(target_power > AGGR_POWER)) {
			of_on(art->flags, OF_AGGRAVATE);
			file_putf(log_file, "Adding aggravation\n");
		}
	}
}

/**
 * Adds a flag to an artifact. Returns true when changes were made.
 */
static bool add_flag(struct artifact *art, int flag)
{
	struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, flag);
	if (of_has(art->flags, flag))
		return false;

	of_on(art->flags, flag);
	file_putf(log_file, "Adding ability: %s\n", prop->name);

	return true;
}

/**
 * Adds a resist to an artifact. Returns true when changes were made.
 */
static bool add_resist(struct artifact *art, int element)
{
	if (art->el_info[element].res_level > 0)
		return false;

	art->el_info[element].res_level = 1;
	file_putf(log_file, "Adding resistance to %s\n", projections[element].name);

	return true;
}

/**
 * Adds an immunity to an artifact. Returns true when changes were made.
 */
static void add_immunity(struct artifact *art)
{
	int r = randint0(4);
	art->el_info[r].res_level = 3;
	file_putf(log_file, "Adding immunity to %s\n", projections[r].name);
}

/**
 * Adds, or increases the positive value of, or decreases the negative value
 * of, a modifier to an artifact.
 */
static bool add_mod(struct artifact *art, int mod)
{
	struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_MOD, mod);

	/* Blows, might, shots need special treatment */
	bool powerful = ((mod == OBJ_MOD_BLOWS) || (mod == OBJ_MOD_MIGHT) ||
					 (mod == OBJ_MOD_SHOTS));
	bool success = false;

	/* This code aims to favour a few larger bonuses over many small ones */
	if (art->modifiers[mod] < 0) {
		/* Negative mods just get a bit worse */
		if (one_in_(2)) {
			art->modifiers[mod]--;
			file_putf(log_file, "Decreasing %s by 1, new value is: %d\n",
					  prop->name, art->modifiers[mod]);
			success = true;
		}
	} else if (powerful) {
		/* Powerful mods need to be applied sparingly */
		if (art->modifiers[mod] == 0) {
			art->modifiers[mod] = randint1(2);
			file_putf(log_file, "Adding ability: %s (%+d)\n", prop->name,
					  art->modifiers[mod]);
			success = true;
		} else if (one_in_(2 * art->modifiers[mod])) {
			art->modifiers[mod]++;
			file_putf(log_file, "Increasing %s by 1, new value is: %d\n",
					  prop->name, art->modifiers[mod]);
			success = true;
		}
	} else {
		/* Hard cap of 6 on non-speed mods */
		if ((mod != OBJ_MOD_SPEED) && (art->modifiers[mod] >= 6)) {
			return false;
		}

		/* New mods average 3, old ones are incremented by 1 or 2 */
		if (art->modifiers[mod] == 0) {
			art->modifiers[mod] = randint0(3) + randint1(3);
			file_putf(log_file, "Adding ability: %s (%+d)\n", prop->name,
					  art->modifiers[mod]);
			success = true;
		} else {
			art->modifiers[mod] += randint1(2);
			file_putf(log_file, "Increasing %s by 2, new value is: %d\n",
					  prop->name, art->modifiers[mod]);
			success = true;
		}

		/* Enforce cap */
		if ((mod != OBJ_MOD_SPEED) && (art->modifiers[mod] >= 6)) {
			art->modifiers[mod] = 6;
		}
	}

	return success;
}

/**
 * Adds, or increases a stat modifier (probably)
 */
static void add_stat(struct artifact *art)
{
	add_mod(art, OBJ_MOD_MIN_STAT + randint0(STAT_MAX));
}

/**
 * Adds a sustain, if possible
 */
static void add_sustain(struct artifact *art)
{
	int r;
	bool success = false;

	/* Break out if all stats are sustained to avoid an infinite loop */
	if (flags_test_all(art->flags, OF_SIZE, OF_SUST_STR, OF_SUST_INT,
	    OF_SUST_WIS, OF_SUST_DEX, OF_SUST_CON, FLAG_END))
			return;

	while (!success) {
		r = randint0(5);
		if (r == 0) success = add_flag(art, OF_SUST_STR);
		else if (r == 1) success = add_flag(art, OF_SUST_INT);
		else if (r == 2) success = add_flag(art, OF_SUST_WIS);
		else if (r == 3) success = add_flag(art, OF_SUST_DEX);
		else if (r == 4) success = add_flag(art, OF_SUST_CON);
	}
}

/**
 * Adds a low resist, if possible
 */
static void add_low_resist(struct artifact *art)
{
	size_t r, i, count = 0;

	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++)
		if (art->el_info[i].res_level <= 0)
			count++;

	if (!count) return;

	r = randint0(count);
	count = 0;

	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++) {
		if (art->el_info[i].res_level > 0) continue;
		if (r == count++) {
			add_resist(art, i);
			return;
		}
	}
}

/**
 * Adds a high resist, if possible
 */
static void add_high_resist(struct artifact *art,
							struct artifact_set_data *data)
{
	/* Add a high resist, according to the generated frequency distribution. */
	size_t i;
	int r, temp;
	int count = 0;
	bool success = false;

	temp = 0;
	for (i = 0; i < N_ELEMENTS(art_idx_high_resist); i++)
		temp += data->art_probs[art_idx_high_resist[i]];

	/* The following will fail (cleanly) if all high resists already added */
	while (!success && (count < MAX_TRIES)) {
		/* Randomize from 1 to this total amount */
		r = randint1(temp);

		/* Determine which (weighted) resist this number corresponds to */

		temp = data->art_probs[art_idx_high_resist[0]];
		i = 0;
		while (r > temp && i < N_ELEMENTS(art_idx_high_resist))	{
			temp += data->art_probs[art_idx_high_resist[i]];
			i++;
		}

		/* Now i should give us the index of the correct high resist */
		if (i == 0) success = add_resist(art, ELEM_POIS);
		else if (i == 1) success = add_flag(art, OF_PROT_FEAR);
		else if (i == 2) success = add_resist(art, ELEM_LIGHT);
		else if (i == 3) success = add_resist(art, ELEM_DARK);
		else if (i == 4) success = add_flag(art, OF_PROT_BLIND);
		else if (i == 5) success = add_flag(art, OF_PROT_CONF);
		else if (i == 6) success = add_resist(art, ELEM_SOUND);
		else if (i == 7) success = add_resist(art, ELEM_SHARD);
		else if (i == 8) success = add_resist(art, ELEM_NEXUS);
		else if (i == 9) success = add_resist(art, ELEM_NETHER);
		else if (i == 10) success = add_resist(art, ELEM_CHAOS);
		else if (i == 11) success = add_resist(art, ELEM_DISEN);
		else if (i == 12) success = add_flag(art, OF_PROT_STUN);

		count++;
	}
}

/**
 * Adds a brand, if possible
 */
static void add_brand(struct artifact *art)
{
	int count;
	struct brand *brand;

	/* Mostly only one brand */
	if (art->brands && randint0(4)) return;

	/* Get a random brand */
	for (count = 0; count < MAX_TRIES; count++) {
		if (!append_random_brand(&art->brands, &brand)) continue;
		file_putf(log_file, "Adding brand: %sx%d\n", brand->name,
				  brand->multiplier);
		break;
	}

	/* Frequently add the corresponding resist */
	if (randint0(4)) {
		size_t i;
		for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++) {
			if (streq(brand->name, projections[i].name) &&
				(art->el_info[i].res_level <= 0)) {
				add_resist(art, i);
			}
		}
	}
}

/**
 * Adds a slay, if possible
 */
static void add_slay(struct artifact *art)
{
	int count;
	struct slay *slay;

	for (count = 0; count < MAX_TRIES; count++) {
		if (!append_random_slay(&art->slays, &slay)) continue;
		file_putf(log_file, "Adding slay: %sx%d\n", slay->name,
				  slay->multiplier);
		break;
	}

	/* Frequently add more slays if the first choice is weak */
	if (randint0(4) && (slay->power < 105)) {
		add_slay(art);
	}
}

/**
 * Adds one or two damage dice
 */
static void add_damage_dice(struct artifact *art)
{
	/* CR 2001-09-02: changed this to increments 1 or 2 only */
	art->dd += (byte)randint1(2);
	file_putf(log_file, "Adding ability: extra damage dice (now %d dice)\n",
			  art->dd);
}

/**
 * Adds to_h, if not too high already
 */
static void add_to_hit(struct artifact *art, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (art->to_h > VERYHIGH_TO_HIT) {
		if (!INHIBIT_STRONG) {
			file_putf(log_file, "Failed to add to-hit, value %d is too high\n",
					  art->to_h);
			return;
		}
	} else if (art->to_h > HIGH_TO_HIT) {
		if (!INHIBIT_WEAK) {
			file_putf(log_file, "Failed to add to-hit, value %d is too high\n",
					  art->to_h);
			return;
		}
	}
	art->to_h += (s16b)(fixed + randint0(random));
	file_putf(log_file, "Adding ability: extra to_h (now %+d)\n", art->to_h);
}

/**
 * Adds to_d, if not too high already
 */
static void add_to_dam(struct artifact *art, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (art->to_d > VERYHIGH_TO_DAM) {
		if (!INHIBIT_STRONG) {
			file_putf(log_file, "Failed to add to-dam, value %d is too high\n",
					  art->to_d);
			return;
		}
	} else if (art->to_h > HIGH_TO_DAM) {
		if (!INHIBIT_WEAK) {
			file_putf(log_file, "Failed to add to-dam, value %d is too high\n",
					  art->to_d);
			return;
		}
	}
	art->to_d += (s16b)(fixed + randint0(random));
	file_putf(log_file, "Adding ability: extra to_dam (now %+d)\n",
			  art->to_d);
}

/**
 * Adds to_a, if not too high already
 */
static void add_to_AC(struct artifact *art, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (art->to_a > VERYHIGH_TO_AC) {
		if (!INHIBIT_STRONG) {
			file_putf(log_file, "Failed to add to-AC, value %d is too high\n",
					  art->to_a);
			return;
		}
	} else if (art->to_h > HIGH_TO_AC) {
		if (!INHIBIT_WEAK) {
			file_putf(log_file, "Failed to add to-AC, value %d is too high\n",
					  art->to_a);
			return;
		}
	}
	art->to_a += (s16b)(fixed + randint0(random));
	file_putf(log_file, "Adding ability: AC bonus (new bonus is %+d)\n",
			  art->to_a);
}

/**
 * Lowers weight
 */
static void add_weight_mod(struct artifact *art)
{
	art->weight = (art->weight * 9) / 10;
	file_putf(log_file, "Adding ability: lower weight (new weight is %d)\n",
			  art->weight);
}

/**
 * Add an activation (called only if artifact does not yet have one)
 */
static void add_activation(struct artifact *art, int target_power,
						   int max_power)
{
	int i, x, p, max_effect = 0;
	int count = 0;

	/* Work out the maximum allowed activation power */
	for (i = 0; i < z_info->act_max; i++) {
		struct activation *act = &activations[i];
		if ((act->power > max_effect) && (act->power < INHIBIT_POWER))
			max_effect = act->power;
	}

	/* Select an activation at random */
	while (count < MAX_TRIES) {
		x = randint0(z_info->act_max);
		p = activations[x].power;

		/* Check that activation is useful but not exploitable, and roughly
		 * proportionate to the overall power */
		if (p < INHIBIT_POWER &&
			100 * p / max_effect > 50 * target_power / max_power &&
			100 * p / max_effect < 200 * target_power / max_power) {
			file_putf(log_file, "Adding activation effect %d\n", x);
			art->activation = &activations[x];
			art->time.base = (p * 8);
			art->time.dice = (p > 5 ? p / 5 : 1);
			art->time.sides = p;
			return;
		}
		count++;
	}
}


/**
 * Choose a random ability using weights based on the given cumulative
 * frequency table.  A pointer to the frequency array (which must be of size
 * ART_IDX_TOTAL) is passed as a parameter.  The function returns a number
 * representing the index of the ability chosen.
 */

static int choose_ability (int *freq_table)
{
	int r, ability;

	/* Generate a random number between 1 and the last value in the table */
	r = randint1(freq_table[ART_IDX_TOTAL - 1]);

	/* Find the entry in the table that this number represents. */
	ability = 0;
	while (r > freq_table[ability])
		ability++;

	file_putf(log_file, "Ability chosen was number: %d\n", ability);
	/*
	 * The ability variable is now the index of the first value in the table
	 * greater than or equal to r, which is what we want.
	 */
	return ability;
}

/**
 * Add an ability given by the index r.  This is mostly just a long case
 * statement.
 *
 * Note that this method is totally general and imposes no restrictions on
 * appropriate item type for a given ability.  This is assumed to have
 * been done already.
 */

static void add_ability_aux(struct artifact *art, int r, s32b target_power,
							struct artifact_set_data *data)
{
	struct object_kind *kind = lookup_kind(art->tval, art->sval);

	switch(r)
	{
		case ART_IDX_BOW_SHOTS:
		case ART_IDX_NONWEAPON_SHOTS:
			add_mod(art, OBJ_MOD_SHOTS);
			break;

		case ART_IDX_BOW_MIGHT:
			add_mod(art, OBJ_MOD_MIGHT);
			break;

		case ART_IDX_WEAPON_HIT:
		case ART_IDX_NONWEAPON_HIT:
			add_to_hit(art, 1, 2 * data->hit_increment);
			break;

		case ART_IDX_WEAPON_DAM:
		case ART_IDX_NONWEAPON_DAM:
			add_to_dam(art, 1, 2 * data->dam_increment);
			break;

		case ART_IDX_NONWEAPON_HIT_DAM:
		case ART_IDX_GLOVE_HIT_DAM:
			add_to_hit(art, 1, 2 * data->hit_increment);
			add_to_dam(art, 1, 2 * data->dam_increment);
			break;

		case ART_IDX_WEAPON_AGGR:
		case ART_IDX_NONWEAPON_AGGR:
			if (target_power > AGGR_POWER)
			{
				add_flag(art, OF_AGGRAVATE);
			}
			break;

		case ART_IDX_MELEE_BLESS:
			add_flag(art, OF_BLESSED);
			break;

		case ART_IDX_BOW_BRAND:
		case ART_IDX_MELEE_BRAND:
		case ART_IDX_NONWEAPON_BRAND:
			add_brand(art);
			break;

		case ART_IDX_BOW_SLAY:
		case ART_IDX_MELEE_SLAY:
		case ART_IDX_NONWEAPON_SLAY:
			add_slay(art);
			break;

		case ART_IDX_MELEE_SINV:
		case ART_IDX_HELM_SINV:
		case ART_IDX_GEN_SINV:
			add_flag(art, OF_SEE_INVIS);
			break;

		case ART_IDX_MELEE_BLOWS:
		case ART_IDX_NONWEAPON_BLOWS:
			add_mod(art, OBJ_MOD_BLOWS);
			break;

		case ART_IDX_MELEE_AC:
		case ART_IDX_BOOT_AC:
		case ART_IDX_GLOVE_AC:
		case ART_IDX_HELM_AC:
		case ART_IDX_SHIELD_AC:
		case ART_IDX_CLOAK_AC:
		case ART_IDX_ARMOR_AC:
		case ART_IDX_GEN_AC:
			add_to_AC(art, 1, 2 * data->ac_increment);
			break;

		case ART_IDX_MELEE_DICE:
			add_damage_dice(art);
			break;

		case ART_IDX_MELEE_WEIGHT:
		case ART_IDX_ALLARMOR_WEIGHT:
			add_weight_mod(art);
			break;

		case ART_IDX_MELEE_TUNN:
		case ART_IDX_GEN_TUNN:
			add_mod(art, OBJ_MOD_TUNNEL);
			break;

		case ART_IDX_BOOT_FEATHER:
		case ART_IDX_GEN_FEATHER:
			add_flag(art, OF_FEATHER);
			break;

		case ART_IDX_BOOT_STEALTH:
		case ART_IDX_CLOAK_STEALTH:
		case ART_IDX_ARMOR_STEALTH:
		case ART_IDX_GEN_STEALTH:
			add_mod(art, OBJ_MOD_STEALTH);
			break;

		case ART_IDX_BOOT_SPEED:
		case ART_IDX_GEN_SPEED:
			add_mod(art, OBJ_MOD_SPEED);
			break;

		case ART_IDX_GLOVE_FA:
		case ART_IDX_GEN_FA:
			add_flag(art, OF_FREE_ACT);
			break;

		case ART_IDX_GLOVE_DEX:
			add_mod(art, OBJ_MOD_DEX);
			break;

		case ART_IDX_HELM_RBLIND:
		case ART_IDX_GEN_RBLIND:
			add_flag(art, OF_PROT_BLIND);
			break;

		case ART_IDX_HELM_ESP:
		case ART_IDX_GEN_ESP:
			add_flag(art, OF_TELEPATHY);
			break;

		case ART_IDX_HELM_WIS:
			add_mod(art, OBJ_MOD_WIS);
			break;

		case ART_IDX_HELM_INT:
			add_mod(art, OBJ_MOD_INT);
			break;

		case ART_IDX_SHIELD_LRES:
		case ART_IDX_ARMOR_LRES:
		case ART_IDX_GEN_LRES:
			add_low_resist(art);
			break;

		case ART_IDX_ARMOR_HLIFE:
		case ART_IDX_GEN_HLIFE:
			add_flag(art, OF_HOLD_LIFE);
			break;

		case ART_IDX_ARMOR_CON:
			add_mod(art, OBJ_MOD_CON);
			break;

		case ART_IDX_ARMOR_ALLRES:
			add_resist(art, ELEM_ACID);
			add_resist(art, ELEM_ELEC);
			add_resist(art, ELEM_FIRE);
			add_resist(art, ELEM_COLD);
			break;

		case ART_IDX_ARMOR_HRES:
			add_high_resist(art, data);
			break;

		case ART_IDX_GEN_STAT:
			add_stat(art);
			break;

		case ART_IDX_GEN_SUST:
			add_sustain(art);
			break;

		case ART_IDX_GEN_SEARCH:
			add_mod(art, OBJ_MOD_SEARCH);
			break;

		case ART_IDX_GEN_INFRA:
			add_mod(art, OBJ_MOD_INFRA);
			break;

		case ART_IDX_GEN_IMMUNE:
			add_immunity(art);
			break;

		case ART_IDX_GEN_LIGHT: {
				if (art->tval != TV_LIGHT) {
					art->modifiers[OBJ_MOD_LIGHT] = 1;
				}
				break;
		}
		case ART_IDX_GEN_SDIG:
			add_flag(art, OF_SLOW_DIGEST);
			break;

		case ART_IDX_GEN_REGEN:
			add_flag(art, OF_REGEN);
			break;

		case ART_IDX_GEN_RPOIS:
			add_resist(art, ELEM_POIS);
			break;

		case ART_IDX_GEN_RFEAR:
			add_flag(art, OF_PROT_FEAR);
			break;

		case ART_IDX_GEN_RLIGHT:
			add_resist(art, ELEM_LIGHT);
			break;

		case ART_IDX_GEN_RDARK:
			add_resist(art, ELEM_DARK);
			break;

		case ART_IDX_GEN_RCONF:
			add_flag(art, OF_PROT_CONF);
			break;

		case ART_IDX_GEN_RSOUND:
			add_resist(art, ELEM_SOUND);
			break;

		case ART_IDX_GEN_RSHARD:
			add_resist(art, ELEM_SHARD);
			break;

		case ART_IDX_GEN_RNEXUS:
			add_resist(art, ELEM_NEXUS);
			break;

		case ART_IDX_GEN_RNETHER:
			add_resist(art, ELEM_NETHER);
			break;

		case ART_IDX_GEN_RCHAOS:
			add_resist(art, ELEM_CHAOS);
			break;

		case ART_IDX_GEN_RDISEN:
			add_resist(art, ELEM_DISEN);
			break;

		case ART_IDX_GEN_PSTUN:
			add_flag(art, OF_PROT_STUN);
			break;

		case ART_IDX_BOOT_TRAP_IMM:
		case ART_IDX_GEN_TRAP_IMM:
			add_flag(art, OF_TRAP_IMMUNE);
			break;

		case ART_IDX_GEN_DAM_RED:
			add_mod(art, OBJ_MOD_DAM_RED);
			break;

		case ART_IDX_GEN_MOVES:
			add_mod(art, OBJ_MOD_MOVES);
			break;

		case ART_IDX_GEN_ACTIV:
			if (!art->activation && !kind->activation)
				add_activation(art, target_power, data->max_power);
			break;
	}
}

/**
 * Clean up the artifact by removing illogical combinations of powers.
 */
static void remove_contradictory(struct artifact *art)
{
	if (of_has(art->flags, OF_AGGRAVATE))
		art->modifiers[OBJ_MOD_STEALTH] = 0;

	if (art->modifiers[OBJ_MOD_STR] < 0)
		of_off(art->flags, OF_SUST_STR);
	if (art->modifiers[OBJ_MOD_INT] < 0)
		of_off(art->flags, OF_SUST_INT);
	if (art->modifiers[OBJ_MOD_WIS] < 0)
		of_off(art->flags, OF_SUST_WIS);
	if (art->modifiers[OBJ_MOD_DEX] < 0)
		of_off(art->flags, OF_SUST_DEX);
	if (art->modifiers[OBJ_MOD_CON] < 0)
		of_off(art->flags, OF_SUST_CON);

	if (of_has(art->flags, OF_DRAIN_EXP))
		of_off(art->flags, OF_HOLD_LIFE);

	/* Remove any conflicting curses */
	if (art->curses) {
		int i;
		for (i = 1; i < z_info->curse_max; i++) {
			if (artifact_curse_conflicts(art, i)) {
				art->curses[i] = 0;
				check_artifact_curses(art);
			}
			if (!art->curses) break;
		}
	}
}

/**
 * Randomly select an extra ability to be added to the artifact in question.
 */
static void add_ability(struct artifact *art, s32b target_power, int *freq,
						struct artifact_set_data *data)
{
	int r;

	/* Choose a random ability using the frequency table previously defined */
	r = choose_ability(freq);

	/* Add the appropriate ability */
	add_ability_aux(art, r, target_power, data);

	/* Now remove contradictory or redundant powers. */
	remove_contradictory(art);

	/* Adding WIS to sharp weapons always blesses them */
	if (art->modifiers[OBJ_MOD_WIS] &&
		(art->tval == TV_SWORD || art->tval == TV_POLEARM))
		add_flag(art, OF_BLESSED);
}


/**
 * Randomly select a curse and added it to the artifact in question.
 */
static void add_curse(struct artifact *art, int level)
{
	int max_tries = 5;

	if (of_has(art->flags, OF_BLESSED)) return;

	while (max_tries) {
		int pick = randint1(z_info->curse_max - 1);
		int power = randint1(9) + 10 * m_bonus(9, level);
		if (!curses[pick].poss[art->tval]) {
			max_tries--;
			continue;
		}
		append_artifact_curse(art, pick, power);
		return;
	}
}


/**
 * Make it bad, or if it's already bad, make it worse!
 */
static void make_bad(struct artifact *art, int level)
{
	int i;
	int num = randint1(2);

	if (one_in_(7))
		of_on(art->flags, OF_AGGRAVATE);
	if (one_in_(4))
		of_on(art->flags, OF_DRAIN_EXP);
	if (one_in_(7))
		of_on(art->flags, OF_NO_TELEPORT);

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if ((art->modifiers[i] > 0) && one_in_(2) && (i != OBJ_MOD_MIGHT)) {
			art->modifiers[i] = -art->modifiers[i];
		}
	}
	if ((art->to_a > 0) && one_in_(2))
		art->to_a = -art->to_a;
	if ((art->to_h > 0) && one_in_(2))
		art->to_h = -art->to_h;
	if ((art->to_d > 0) && one_in_(4))
		art->to_d = -art->to_d;

	while (num) {
		add_curse(art, level);
		num--;
	}
}

/**
 * Copy artifact fields from a_src to a_dst
 */

static void copy_artifact(struct artifact *a_src, struct artifact *a_dst)
{
	mem_free(a_dst->slays);
	mem_free(a_dst->brands);
	mem_free(a_dst->curses);

	/* Copy the structure */
	memcpy(a_dst, a_src, sizeof(struct artifact));

	a_dst->next = NULL;
	a_dst->slays = NULL;
	a_dst->brands = NULL;
	a_dst->curses = NULL;
	a_dst->activation = NULL;
	a_dst->alt_msg = NULL;

	if (a_src->slays) {
		a_dst->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
		memcpy(a_dst->slays, a_src->slays, z_info->slay_max * sizeof(bool));
	}
	if (a_src->brands) {
		a_dst->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
		memcpy(a_dst->brands, a_src->brands, z_info->brand_max * sizeof(bool));
	}
	if (a_src->curses) {
		a_dst->curses = mem_zalloc(z_info->curse_max * sizeof(int));
		memcpy(a_dst->curses, a_src->curses, z_info->curse_max * sizeof(int));
	}
}

/**
 * ------------------------------------------------------------------------
 * Generation of a set of random artifacts
 * ------------------------------------------------------------------------ */
/**
 * Use W. Sheldon Simms' random name generator.
 */
char *artifact_gen_name(struct artifact *a, const char ***words) {
	char buf[BUFLEN];
	char word[MAX_NAME_LEN + 1];

	randname_make(RANDNAME_TOLKIEN, MIN_NAME_LEN, MAX_NAME_LEN, word,
				  sizeof(word), words);
	my_strcap(word);

	if (one_in_(3))
		strnfmt(buf, sizeof(buf), "'%s'", word);
	else
		strnfmt(buf, sizeof(buf), "of %s", word);
	return string_make(buf);
}

/**
 * Give an artifact a (boring) description
 */
static void describe_artifact(int aidx, int power)
{
	struct artifact *art = &a_info[aidx];
	char desc[128] = "Random ";
	my_strcat(desc, tval_find_name(art->tval), sizeof(desc));
	my_strcat(desc, format(" of power %d", power), sizeof(desc));
	string_free(art->text);
	art->text = string_make(desc);
}


/**
 * Design a random artifact given a tval
 *
 * The artifact is assigned a power based on the range of powers for that tval
 * in the original artifact set.  It is then given a base item type which is
 * suitable for that power, and than has properties added to it until it falls
 * close enough to the target power - currently this means between 19/20 and
 * 23/20 of the target power.
 */
static void design_artifact(struct artifact_set_data *data, int tv, int *aidx)
{
	struct artifact *art = &a_info[*aidx];
	struct object_kind *kind = lookup_kind(art->tval, art->sval);
	int art_freq[ART_IDX_TOTAL];
	int art_level = art->level;
	int tries;
	int alloc_new;
	int ap = 0;
	bool hurt_me = false;

	/* Set tval if necessary */
	int tval = (tv == TV_NULL) ? get_base_item_tval(data) : tv;

	/* Structure to hold the old artifact */
	struct artifact *a_old = mem_zalloc(sizeof *a_old);

	/* Choose a power for the artifact */
	int power = Rand_sample(data->avg_tv_power[tval],
							data->max_tv_power[tval],
							data->min_tv_power[tval],
							20, 20);

	/* Choose a name */
	char *new_name = artifact_gen_name(art, name_sections);

	/* Skip fixed artifacts */
	while (strstr(art->name, "The One Ring") ||
		kf_has(kind->kind_flags, KF_QUEST_ART)) {
		(*aidx)++;
		if ((*aidx) >= z_info->a_max) {
			return;
		}
		art = &a_info[*aidx];
		art_level = art->level;
	}

	/* Apply the new name */
	string_free(art->name);
	art->name = new_name;

	file_putf(log_file, ">>>>>>>>>>>>>>>>>>>>>>>>>> CREATING NEW ARTIFACT\n");
	file_putf(log_file, "Artifact %d: power = %d\n", *aidx, power);

	/* Flip the sign on power if it's negative (unlikely) and damage */
	if (power < 0) {
		hurt_me = true;
		power = -power;
	}

	/* Choose a base item typen not too powerful, so we'll have to add to it. */
	for (tries = 0; tries < MAX_TRIES; tries++) {
		int base_power = 0;

		/* Get the new item kind and do basic prep on it */
		if (tval == TV_NULL) {
			tval = get_base_item_tval(data);
		}
		kind = get_base_item(data, tval);
		artifact_prep(art, kind, data);

		/* Get the kind again in case it's changed */
		kind = lookup_kind(art->tval, art->sval);

		base_power = artifact_power(*aidx, "for base item power", true);
		file_putf(log_file, "Base item power %d\n", base_power);

		/* New base item power too close to target artifact power */
		if ((base_power > (power * 6) / 10 + 1) && (power - base_power < 20)) {
			file_putf(log_file, "Power too high!\n");
			continue;
		}

		/* Acceptable */
		break;
	};

	/* Failed to get a good base item */
	if (tries >= MAX_TRIES)
		file_putf(log_file, "Warning! Couldn't get appropriate power level on base item.\n");

	/* Generate the cumulative frequency table for this base item type */
	build_freq_table(art, art_freq, data);

	/* Copy artifact info temporarily. */
	copy_artifact(art, a_old);

	/* Give this artifact a shot at being supercharged */
	try_supercharge(art, power, data);
	ap = artifact_power(*aidx, "result of supercharge", true);
	if (ap > (power * 23) / 20 + 1)	{
		/* Too powerful -- put it back */
		copy_artifact(a_old, art);
		file_putf(log_file, "--- Supercharge is too powerful! Rolling back.\n");
	}

	/* Give this artifact a chance to be cursed - note it retains its power */
	if (one_in_(z_info->a_max / MAX(2, data->neg_power_total))) {
		hurt_me = true;
	}

	/* Do the actual artifact design */
	for (tries = 0; tries < MAX_TRIES; tries++) {
		/* Copy artifact info temporarily. */
		copy_artifact(art, a_old);

		/* Add an ability */
		add_ability(art, power, art_freq, data);
		remove_contradictory(art);

		/* Check the power, handle negative power */
		ap = artifact_power(*aidx, "artifact attempt", true);
		if (ap < 0) {
			ap = -ap;
			break;
		}

		/* Curse the designated artifacts */
		if (hurt_me) {
			make_bad(art, art_level);
			if (one_in_(3)) {
				hurt_me = false;
			}
		}

		/* Check power */
		if (ap > (power * 23) / 20 + 1) {
			/* Too powerful -- put it back */
			copy_artifact(a_old, art);
			file_putf(log_file, "--- Too powerful!  Rolling back.\n");
			continue;
		} else if (ap >= (power * 19) / 20) {
			/* Just right */
			break;
		}
	}

	/* Couldn't generate an artifact with the number of permitted iterations */
	if (tries >= MAX_TRIES)
		file_putf(log_file, "Warning!  Couldn't get appropriate power level on artifact.\n");

	/* Cleanup a_old */
	mem_free(a_old->slays);
	mem_free(a_old->brands);
	mem_free(a_old->curses);
	mem_free(a_old);

	/* Set rarity based on power */
	alloc_new = 4000000 / (ap * ap);
	alloc_new /= (kind->alloc_prob ? kind->alloc_prob : 20);
	if (alloc_new > 99) alloc_new = 99;
	if (alloc_new < 1) alloc_new = 1;
	art->alloc_prob = alloc_new;


	/* Set depth according to power */
	art->alloc_max = MIN(127, (ap * 3) / 5);
	art->alloc_min = MIN(100, ((ap + 100) * 100 / data->max_power));

	/* Have a chance to be less rare or deep, more likely the less power */
	if (one_in_(500 / power)) {
		art->alloc_prob += randint1(20);
	} else if (one_in_(500 / power)) {
		art->alloc_min /= 2;
	}

	/* Sanity check */
	art->alloc_max = MAX(art->alloc_max, MIN(art->alloc_min * 2, 127));

	file_putf(log_file, "New depths are min %d, max %d\n", art->alloc_min,
			  art->alloc_max);
	file_putf(log_file, "Power-based alloc_prob is %d\n", art->alloc_prob);

	/* Success */
	file_putf(log_file, "<<<<<<<<<<<<<<<<<<<<<<<<<< ARTIFACT COMPLETED\n");
	file_putf(log_file, "Number of tries for artifact %d was: %d\n", *aidx,
			  tries);

	/* Describe it */
	describe_artifact(*aidx, ap);
}

/**
 * Create a random artifact set
 *
 * The resulting set will have at least 80% the number of artifacts from any
 * given tval as the original artifact set.  This means that tvals with less
 * than 5 artifacts in the original set will always have equal or increased
 * numbers on the new set.
 */
void create_artifact_set(struct artifact_set_data *data)
{
	int i, aidx = 1;
	int *tval_total = mem_zalloc(TV_MAX * sizeof(int));
	bool not_done = true;

	/* Get min tval frequencies for the new artifacts */
	for (i = 0; i < TV_MAX; i++) {
		/* At least 80% as many for each tval */
		tval_total[i] = (4 * (data->tv_num[i] + 1)) / 5;
	}

	/* Allocate a minimal set of artifacts to the tvals */
	while (not_done) {
		not_done = false;

		/* Multiple passes through tvals until all have enough artifacts */ 
		for (i = 0; i < TV_MAX; i++) {
			if (tval_total[i] > 0) {
				design_artifact(data, i, &aidx);
				tval_total[i]--;
				aidx++;
				not_done = true;
			}
		}
	}

	/* Allocate remaining artifacts at random */
	while (aidx < z_info->a_max - 1) {
		design_artifact(data, TV_NULL, &aidx);
		aidx++;
	}

	mem_free(tval_total);
}

/**
 * Allocate a new artifact set data structure
 */
static struct artifact_set_data *artifact_set_data_new(void)
{
	struct artifact_set_data *data = mem_zalloc(sizeof(*data));

	data->base_power = mem_zalloc(z_info->a_max * sizeof(int));
	data->avg_tv_power = mem_zalloc(TV_MAX * sizeof(int));
	data->min_tv_power = mem_zalloc(TV_MAX * sizeof(int));
	data->max_tv_power = mem_zalloc(TV_MAX * sizeof(int));
	data->base_item_level = mem_zalloc(z_info->a_max * sizeof(int));
	data->base_item_prob = mem_zalloc(z_info->a_max * sizeof(int));
	data->base_art_alloc = mem_zalloc(z_info->a_max * sizeof(int));
	data->tv_probs = mem_zalloc(TV_MAX * sizeof(int));
	data->tv_num = mem_zalloc(TV_MAX * sizeof(int));
	data->art_probs = mem_zalloc(ART_IDX_TOTAL * sizeof(int));
	data->tv_freq = mem_zalloc(TV_MAX * sizeof(int));

	/* Mean start and increment values for to_hit, to_dam and AC.  Update these
	 * if the algorithm changes.  They are used in frequency generation. */
	data->hit_increment = 4;
	data->dam_increment = 4;
	data->hit_startval = 10;
	data->dam_startval = 10;
	data->ac_startval = 15;
	data->ac_increment = 5;

	return data;
}

/**
 * Allocate a new artifact set data structure
 */
static void artifact_set_data_free(struct artifact_set_data *data)
{
	mem_free(data->base_power);
	mem_free(data->avg_tv_power);
	mem_free(data->min_tv_power);
	mem_free(data->max_tv_power);
	mem_free(data->base_item_level);
	mem_free(data->base_item_prob);
	mem_free(data->base_art_alloc);
	mem_free(data->tv_probs);
	mem_free(data->tv_num);
	mem_free(data->art_probs);
	mem_free(data->tv_freq);
	mem_free(data);
}

/**
 * Write an artifact data file
 */
void write_randart_entry(ang_file *fff, struct artifact *art)
{
	char name[120] = "";
	struct object_kind *kind = lookup_kind(art->tval, art->sval);
	int j;

	static const char *obj_flags[] = {
		"NONE",
		#define OF(a) #a,
		#include "list-object-flags.h"
		#undef OF
		NULL
	};

	/* Ignore non-existent artifacts */
	if (!art->name) return;

	/* Output description */
	file_putf(fff, "# %s\n", art->text);

	/* Output name */
	file_putf(fff, "name:%s\n", art->name);

	/* Output tval and sval */
	object_short_name(name, sizeof name, kind->name);
	file_putf(fff, "base-object:%s:%s\n", tval_find_name(art->tval), name);

	/* Output graphics if necessary */
	if (kind->kidx >= z_info->ordinary_kind_max) {
		const char *attr = attr_to_text(kind->d_attr);
		file_putf(fff, "graphics:%c:%s\n", kind->d_char, attr);
	}

	/* Output level, weight and cost */
	file_putf(fff, "level:%d\n", art->level);
	file_putf(fff, "weight:%d\n", art->weight);
	file_putf(fff, "cost:%d\n", art->cost);

	/* Output allocation info */
	file_putf(fff, "alloc:%d:%d to %d\n", art->alloc_prob, art->alloc_min,
			  art->alloc_max);

	/* Output combat power */
	file_putf(fff, "attack:%dd%d:%d:%d\n", art->dd, art->ds, art->to_h,
			  art->to_d);
	file_putf(fff, "armor:%d:%d\n", art->ac, art->to_a);

	/* Output flags */
	write_flags(fff, "flags:", art->flags, OF_SIZE, obj_flags);

	/* Output modifiers */
	write_mods(fff, art->modifiers);

	/* Output resists, immunities and vulnerabilities */
	write_elements(fff, art->el_info);

	/* Output slays */
	if (art->slays) {
		for (j = 1; j < z_info->slay_max; j++) {
			if (art->slays[j]) {
				file_putf(fff, "slay:%s\n", slays[j].code);
			}
		}
	}

	/* Output brands */
	if (art->brands) {
		for (j = 1; j < z_info->brand_max; j++) {
			if (art->brands[j]) {
				file_putf(fff, "brand:%s\n", brands[j].code);
			}
		}
	}

	/* Output curses */
	if (art->curses) {
		for (j = 1; j < z_info->curse_max; j++) {
			if (art->curses[j] != 0) {
				file_putf(fff, "curse:%s:%d\n", curses[j].name,
						  art->curses[j]);
			}
		}
	}

	/* Output activation details */
	if (art->activation) {
		file_putf(fff, "act:%s\n", art->activation->name);
		file_putf(fff, "time:%d+%dd%d\n", art->time.base, art->time.dice,
				  art->time.sides);
	} else if (kind->activation) {
		file_putf(fff, "act:%s\n", kind->activation->name);
		file_putf(fff, "time:%d+%dd%d\n", kind->time.base, kind->time.dice,
				  kind->time.sides);
	}

	/* Output description again */
	file_putf(fff, "desc:%s\n", art->text);

	file_putf(fff, "\n");
}

/**
 * Randomize the artifacts
 */
void do_randart(u32b randart_seed, bool create_file)
{
	char fname[1024];
	struct artifact_set_data *standarts = artifact_set_data_new();
	struct artifact_set_data *randarts;

	/* Prepare to use the Angband "simple" RNG. */
	Rand_value = randart_seed;
	Rand_quick = true;

	/* Open the log file for writing */
	path_build(fname, sizeof(fname), ANGBAND_DIR_USER, "randart.log");
	log_file = file_open(fname, MODE_WRITE, FTYPE_TEXT);
	if (!log_file) {
		msg("Error - can't open randart.log for writing.");
		artifact_set_data_free(standarts);
		exit(1);
	}

	/* Store the original power ratings */
	store_base_power(standarts);

	/* Determine the generation probabilities */
	parse_frequencies(standarts);

	/* Generate the random artifacts */
	create_artifact_set(standarts);
	artifact_set_data_free(standarts);

	/* Look at the frequencies on the finished items */
	randarts = artifact_set_data_new();
	store_base_power(randarts);
	parse_frequencies(randarts);
	artifact_set_data_free(randarts);

	/* Close the log file */
	if (!file_close(log_file)) {
		msg("Error - can't close randart.log file.");
		exit(1);
	}

	/* Write a data file if required */
	if (create_file) {
		int i;
		/* Open the file, write a header */
		path_build(fname, sizeof(fname), ANGBAND_DIR_USER, "randart.txt");
		log_file = file_open(fname, MODE_WRITE, FTYPE_TEXT);
		file_putf(log_file,
				  "# Artifact file for random artifacts with seed %08x\n\n\n",
				  randart_seed);

		/* Write individual entries */
		for (i = 1; i < z_info->a_max; i++) {
			struct artifact *art = &a_info[i];
			write_randart_entry(log_file, art);
		}

		/* Close the file */
		if (!file_close(log_file)) {
			quit_fmt("Error - can't close %s.", fname);
		}
	}

	/* When done, resume use of the Angband "complex" RNG. */
	Rand_quick = false;
}
