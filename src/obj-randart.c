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
#include "effects.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-randart.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "randname.h"

/* Arrays of indices by item type, used in frequency generation */
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
	ART_IDX_BOOT_SPEED
};
static s16b art_idx_glove[] = {
	ART_IDX_GLOVE_AC,
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
	ART_IDX_GEN_PSTUN
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

/* Initialize the data structures for learned probabilities */
static s16b artprobs[ART_IDX_TOTAL];
static s16b *baseprobs;
static s16b art_bow_total = 0;
static s16b art_melee_total = 0;
static s16b art_boot_total = 0;
static s16b art_glove_total = 0;
static s16b art_headgear_total = 0;
static s16b art_shield_total = 0;
static s16b art_cloak_total = 0;
static s16b art_armor_total = 0;
static s16b art_other_total = 0;
static s16b art_total = 0;

/*
 * Working arrays for holding frequency values - global to avoid repeated
 * allocation of memory
 */
static s16b art_freq[ART_IDX_TOTAL];  	/* artifact attributes */
static s16b *base_freq; 			/* base items */

/*
 * Mean start and increment values for to_hit, to_dam and AC.  Update these
 * if the algorithm changes.  They are used in frequency generation.
 */
static s16b mean_hit_increment = 4;
static s16b mean_dam_increment = 4;
static s16b mean_hit_startval = 10;
static s16b mean_dam_startval = 10;
static s16b mean_ac_startval = 15;
static s16b mean_ac_increment = 5;

/*
 * Pointer for logging file
 */
static ang_file *log_file = NULL;

/*
 * Store the original artifact power ratings
 */
static s32b *base_power;
static s16b max_power;
static s16b min_power;
static s16b avg_power;
static s16b var_power;

/*
 * Store the original base item levels
 */
static byte *base_item_level;

/*
 * Store the original base item rarities
 */
static byte *base_item_prob;

/*
 * Store the original artifact rarities
 */
static byte *base_art_alloc;

/* Activation list */
struct activation *activations;

/* Global just for convenience. */
static int verbose = 1;

/* Fake pvals array for maintaining current behaviour NRM */
int fake_pval[3] = {0, 0, 0};

/**
 * Include the elements and names
 */
static const struct element_type {
	int index;
	const char *name;
} elements[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) {ELEM_##a, b},
	#include "list-elements.h"
	#undef ELEM
};

void fake_pvals_to_mods(struct artifact *a)
{
	int i;

	/* Copy the fake_pvals in as modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (a->modifiers[i] != 0) {
			a->modifiers[i] = fake_pval[a->modifiers[i] - 1];
			file_putf(log_file, "Modifier %s is %d\n", mod_name(i), a->modifiers[i]);
		}
	}
}

void mods_to_fake_pvals(struct artifact *a)
{
	int i, j;

	/* Set the fake pvals to 0 */
	for (i = 0; i < 3; i++)
		fake_pval[i] = 0;

	/* Now set any non-zero mod as a fake_pval, and set the mod value to the 
	 * fake_pval position this mod is in  - lunacy, but hey NRM */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (a->modifiers[i] != 0) {
			for (j = 0; j < 3; j++) {
				/* If the mod value is already there, refer to it */
				if (a->modifiers[i] == fake_pval[j]) {
					a->modifiers[i] = j + 1;
					break;
				}
				/* If that failed, and we have a zero position, use that */
				if (fake_pval[j] == 0) {
					fake_pval[j] = a->modifiers[i];
					a->modifiers[i] = j + 1;
					break;
				}
				/* If those both failed, we lose a mod, but at this stage there
				 * are only a max of three distinct mods per object, and when 
				 * there aren't we should have removed this silliness */
			}
		}
	}
}

/**
 * Return the artifact power, by generating a "fake" object based on the
 * artifact, and calling the common object_power function
 */
static s32b artifact_power(int a_idx, bool translate)
{
	struct object obj, known_obj;
	char buf[256];
	bool fail = false;
	s32b power;

	file_putf(log_file, "********** ENTERING EVAL POWER ********\n");
	file_putf(log_file, "Artifact index is %d\n", a_idx);

	if (translate) fake_pvals_to_mods(&a_info[a_idx]);
	if (!make_fake_artifact(&obj, &a_info[a_idx]))
		fail = true;
	if (translate) mods_to_fake_pvals(&a_info[a_idx]);

	if (fail) return 0;

	object_copy(&known_obj, &obj);
	obj.known = &known_obj;
	object_desc(buf, 256 * sizeof(char), &obj,
				ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);
	file_putf(log_file, "%s\n", buf);

	power = object_power(&obj, verbose, log_file);

	object_wipe(&known_obj);
	object_wipe(&obj);
	return power;
}


/**
 * Store the original artifact power ratings as a baseline
 */
static void store_base_power(void)
{
	int i, j;
	struct artifact *art;
	struct object_kind *kind;
	int *fake_power;

	max_power = 0;
	min_power = 32767;
	var_power = 0;
	fake_power = mem_zalloc(z_info->a_max * sizeof(int));
	j = 0;

	for (i = 0; i < z_info->a_max; i++, j++) {
		base_power[i] = artifact_power(i, false);

		/* capture power stats, ignoring cursed and uber arts */
		if (base_power[i] > max_power && base_power[i] < INHIBIT_POWER)
			max_power = base_power[i];
		if (base_power[i] < min_power && base_power[i] > 0)
			min_power = base_power[i];
		if (base_power[i] > 0 && base_power[i] < INHIBIT_POWER)
			fake_power[j] = (int)base_power[i];
		else
			j--;

		if (!base_power[i]) continue;
		art = &a_info[i];
		kind = lookup_kind(art->tval, art->sval);
		base_item_level[i] = kind->level;
		base_item_prob[i] = kind->alloc_prob;
		base_art_alloc[i] = art->alloc_prob;
	}

	avg_power = mean(fake_power, j);
	var_power = variance(fake_power, j);

	file_putf(log_file, "Max power is %d, min is %d\n", max_power, min_power);
	file_putf(log_file, "Mean is %d, variance is %d\n", avg_power, var_power);

	/* Store the number of different types, for use later */
	/* ToDo: replace this with full combination tracking */
	for (i = 0; i < z_info->a_max; i++) {
		switch (a_info[i].tval)
		{
		case TV_SWORD:
		case TV_POLEARM:
		case TV_HAFTED:
			art_melee_total++; break;
		case TV_BOW:
			art_bow_total++; break;
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			art_armor_total++; break;
		case TV_SHIELD:
			art_shield_total++; break;
		case TV_CLOAK:
			art_cloak_total++; break;
		case TV_HELM:
		case TV_CROWN:
			art_headgear_total++; break;
		case TV_GLOVES:
			art_glove_total++; break;
		case TV_BOOTS:
			art_boot_total++; break;
		case TV_NULL:
			break;
		default:
			art_other_total++;
		}
	}
	art_total = art_melee_total + art_bow_total + art_armor_total +
	            art_shield_total + art_cloak_total + art_headgear_total +
	            art_glove_total + art_boot_total + art_other_total;

    mem_free(fake_power);
}


/**
 * Randomly select a base item type (tval,sval).  Assign the various fields
 * corresponding to that choice.
 *
 * The return value gives the index of the new item type.  The method is
 * passed a pointer to a rarity value in order to return the rarity of the
 * new item.
 */
static struct object_kind *choose_item(int a_idx)
{
	struct artifact *art = &a_info[a_idx];
	int tval = 0, sval = 0, i = 0;
	struct object_kind *kind;
	s16b r;

	/*
	 * Pick a base item from the cumulative frequency table.
	 *
	 * Although this looks hideous, it provides for easy addition of
	 * future artifact types, simply by removing the tvals from this
	 * loop.
	 *
	 * N.B. Could easily generate lights, rings and amulets this way if
	 * the whole special/flavour issue was sorted out (see ticket #1014).
	 */
	while (tval == 0 ||	k_info[i].alloc_prob == 0 ||
		   tval == TV_SHOT || tval == TV_ARROW || tval == TV_BOLT ||
		   tval == TV_STAFF || tval == TV_WAND || tval == TV_ROD ||
		   tval == TV_SCROLL || tval == TV_POTION || tval == TV_FLASK ||
		   tval == TV_FOOD || tval == TV_MUSHROOM || tval == TV_MAGIC_BOOK ||
		   tval == TV_PRAYER_BOOK || tval == TV_GOLD || tval == TV_LIGHT ||
		   tval == TV_AMULET || tval == TV_RING || tval == TV_CHEST ||
		   (tval == TV_HAFTED && sval == lookup_sval(tval, "Mighty Hammer")) ||
		   (tval == TV_CROWN && sval == lookup_sval(tval, "Massive Iron Crown"))) {
		r = randint1(base_freq[z_info->k_max - 1]);
		i = 0;
		while (r > base_freq[i])
			i++;
		tval = k_info[i].tval;
		sval = k_info[i].sval;
	}
	file_putf(log_file, "Creating tval %d sval %d\n", tval, sval);
	kind = lookup_kind(tval, sval);
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
	art->slays = NULL;
	copy_slay(&art->slays, kind->slays);
	art->brands = NULL;
	copy_brand(&art->brands, kind->brands);
	art->activation = NULL;
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		art->modifiers[i] = randcalc(kind->modifiers[i], 0, MINIMISE);
	}
	for (i = 0; i < ELEM_MAX; i++)
		art->el_info[i] = kind->el_info[i];

	/* Artifacts ignore everything */
	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++)
		art->el_info[i].flags |= EL_INFO_IGNORE;

	/* Assign basic stats to the artifact based on its artifact level. */
	/*
	 * CR, 2001-09-03: changed to a simpler version to match the hit-dam
	 * parsing algorithm. We use random ranges averaging mean_hit_startval
	 * and mean_dam_startval, but permitting variation of 50% to 150%.
	 * Level-dependent term has been removed for the moment.
	 */
	switch (art->tval) {
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_SWORD:
		case TV_POLEARM:
			art->to_h += (s16b)(mean_hit_startval / 2 +
			                      randint0(mean_hit_startval) );
			art->to_d += (s16b)(mean_dam_startval / 2 +
			                      randint0(mean_dam_startval) );
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
			/* CR: adjusted this to work with parsing logic */
			art->to_a += (s16b)(mean_ac_startval / 2 +
			                      randint0(mean_ac_startval) );
			file_putf(log_file, "Assigned basic stats, AC bonus: %d\n",
					  art->to_a);
			break;
	}

	/* Done - return the index of the new object kind. */
	return kind;
}


/**
 * We've just added an ability which uses the pval bonus.  Make sure it's
 * not zero.  If it's currently negative, leave it negative (heh heh).
 */
static void do_mod(struct artifact *art)
{
	int factor = 1;

	/* Track whether we have blows, might or shots on this item */
	if (art->modifiers[OBJ_MOD_BLOWS] > 0) factor++;
	if (art->modifiers[OBJ_MOD_MIGHT] > 0) factor++;
	if (art->modifiers[OBJ_MOD_SHOTS] > 0) factor++;

	if (fake_pval[0] == 0) {
		/* Blows, might, shots handled separately */
		if (factor > 1) {
			fake_pval[0] = (s16b)randint1(2);
			/* Give it a shot at +3 */
			if (INHIBIT_STRONG) fake_pval[0] = 3;
		} else
			fake_pval[0] = (s16b)randint1(4);
		file_putf(log_file, "Assigned initial pval, value is: %d\n",
				  fake_pval[0]);
	} else if (fake_pval[0] < 0) {
		if (one_in_(2)) {
			fake_pval[0]--;
			file_putf(log_file, "Decreasing pval by 1, new value is: %d\n",
					  fake_pval[0]);
		}
	} else if (one_in_(fake_pval[0] * factor)) {
		/*
		 * CR: made this a bit rarer and diminishing with higher pval -
		 * also rarer if item has blows/might/shots already
		 */
		fake_pval[0]++;
		file_putf(log_file, "Increasing pval by 1, new value is: %d\n",
				  fake_pval[0]);
	}
}


/**
 * Clean up the artifact by removing illogical combinations of powers.
 */
static void remove_contradictory(struct artifact *art)
{
	if (of_has(art->flags, OF_AGGRAVATE))
		art->modifiers[OBJ_MOD_STEALTH] = 0;

	if (fake_pval[0] < 0) {
		if (art->modifiers[OBJ_MOD_STR] != 0)
			of_off(art->flags, OF_SUST_STR);
		if (art->modifiers[OBJ_MOD_INT] != 0)
			of_off(art->flags, OF_SUST_INT);
		if (art->modifiers[OBJ_MOD_WIS] != 0)
			of_off(art->flags, OF_SUST_WIS);
		if (art->modifiers[OBJ_MOD_DEX] != 0)
			of_off(art->flags, OF_SUST_DEX);
		if (art->modifiers[OBJ_MOD_CON] != 0)
			of_off(art->flags, OF_SUST_CON);
		art->modifiers[OBJ_MOD_BLOWS] = 0;
	}

	if (of_has(art->flags, OF_LIGHT_CURSE))
		of_off(art->flags, OF_BLESSED);
	if (of_has(art->flags, OF_DRAIN_EXP))
		of_off(art->flags, OF_HOLD_LIFE);
}

/**
 * Adjust the parsed frequencies for any peculiarities of the
 * algorithm.  For example, if stat bonuses and sustains are
 * being added in a correlated fashion, it will tend to push
 * the frequencies up for both of them.  In this method we
 * compensate for cases like this by applying corrective
 * scaling.
 */
static void adjust_freqs(void)
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
	if (artprobs[ART_IDX_GEN_RFEAR] < 5)
		artprobs[ART_IDX_GEN_RFEAR] = 5;
	if (artprobs[ART_IDX_MELEE_DICE_SUPER] < 5)
		artprobs[ART_IDX_MELEE_DICE_SUPER] = 5;
	if (artprobs[ART_IDX_BOW_SHOTS_SUPER] < 5)
		artprobs[ART_IDX_BOW_SHOTS_SUPER] = 5;
	if (artprobs[ART_IDX_BOW_MIGHT_SUPER] < 5)
		artprobs[ART_IDX_BOW_MIGHT_SUPER] = 5;
	if (artprobs[ART_IDX_MELEE_BLOWS_SUPER] < 5)
		artprobs[ART_IDX_MELEE_BLOWS_SUPER] = 5;
	if (artprobs[ART_IDX_GEN_SPEED_SUPER] < 5)
		artprobs[ART_IDX_GEN_SPEED_SUPER] = 5;
	if (artprobs[ART_IDX_GEN_AC] < 5)
		artprobs[ART_IDX_GEN_AC] = 5;
	if (artprobs[ART_IDX_GEN_TUNN] < 5)
		artprobs[ART_IDX_GEN_TUNN] = 5;
	if (artprobs[ART_IDX_NONWEAPON_BRAND] < 2)
		artprobs[ART_IDX_NONWEAPON_BRAND] = 2;
	if (artprobs[ART_IDX_NONWEAPON_SLAY] < 2)
		artprobs[ART_IDX_NONWEAPON_SLAY] = 2;
	if (artprobs[ART_IDX_BOW_BRAND] < 2)
		artprobs[ART_IDX_BOW_BRAND] = 2;
	if (artprobs[ART_IDX_BOW_SLAY] < 2)
		artprobs[ART_IDX_BOW_SLAY] = 2;
	if (artprobs[ART_IDX_NONWEAPON_BLOWS] < 2)
		artprobs[ART_IDX_NONWEAPON_BLOWS] = 2;
	if (artprobs[ART_IDX_NONWEAPON_SHOTS] < 2)
		artprobs[ART_IDX_NONWEAPON_SHOTS] = 2;
	if (artprobs[ART_IDX_GEN_AC_SUPER] < 5)
		artprobs[ART_IDX_GEN_AC_SUPER] = 5;
	if (artprobs[ART_IDX_MELEE_AC] < 5)
		artprobs[ART_IDX_MELEE_AC] = 5;
	if (artprobs[ART_IDX_GEN_PSTUN] < 3)
		artprobs[ART_IDX_GEN_PSTUN] = 3;

	/* Cut aggravation frequencies in half since they're used twice */
	artprobs[ART_IDX_NONWEAPON_AGGR] /= 2;
	artprobs[ART_IDX_WEAPON_AGGR] /= 2;
}

/**
 * Parse the list of artifacts and count up the frequencies of the various
 * abilities.  This is used to give dynamic generation probabilities.
 */
static void parse_frequencies(void)
{
	size_t i;
	int j;
	const struct artifact *art;
	struct object_kind *kind;
	s32b m, temp;

	file_putf(log_file, "\n****** BEGINNING GENERATION OF FREQUENCIES\n\n");

	/* Zero the frequencies for artifact attributes */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		artprobs[i] = 0;

	/*
	 * Initialise the frequencies for base items so that each item could
	 * be chosen - we check for illegal items during choose_item()
	 */
	for (i = 0; i < z_info->k_max; i++)
		baseprobs[i] = 1;

	/* Go through the list of all artifacts */
	for (i = 0; i < z_info->a_max; i++) {
		file_putf(log_file, "Current artifact index is %d\n", i);

		art = &a_info[i];

		/* Don't parse cursed or null items */
		if (base_power[i] < 0 || art->tval == 0) continue;

		/* Get a pointer to the base item for this artifact */
		kind = lookup_kind(art->tval, art->sval);

		/* Special cases -- don't parse these! */
		if (strstr(art->name, "The One Ring") ||
			kf_has(kind->kind_flags, KF_QUEST_ART))
			continue;

		/* Add the base item to the baseprobs array */
		baseprobs[kind->kidx]++;
		file_putf(log_file, "Base item is %d\n", kind->kidx);

		/* Count up the abilities for this artifact */
		if (art->tval == TV_BOW) {
			/* Do we have 3 or more extra shots? (Unlikely) */
			if (art->modifiers[OBJ_MOD_SHOTS] > 2) {
				file_putf(log_file,
						  "Adding 1 for supercharged shots (3 or more!)\n");
				(artprobs[ART_IDX_BOW_SHOTS_SUPER])++;
			} else if (art->modifiers[OBJ_MOD_SHOTS] > 0) {
				file_putf(log_file, "Adding 1 for extra shots\n");
				(artprobs[ART_IDX_BOW_SHOTS])++;
			}

			/* Do we have 3 or more extra might? (Unlikely) */
			if (art->modifiers[OBJ_MOD_MIGHT] > 2) {
				file_putf(log_file,
						  "Adding 1 for supercharged might (3 or more!)\n");
				(artprobs[ART_IDX_BOW_MIGHT_SUPER])++;
			} else if (art->modifiers[OBJ_MOD_MIGHT] > 0) {
				file_putf(log_file, "Adding 1 for extra might\n");
				(artprobs[ART_IDX_BOW_MIGHT])++;
			}

			/* Count brands and slays */
			if (art->slays) {
				temp = slay_count(art->slays);
				artprobs[ART_IDX_MELEE_SLAY] += temp;
				file_putf(log_file, "Adding %d for slays\n", temp);
			}
			if (art->brands) {
				temp = brand_count(art->brands);
				artprobs[ART_IDX_MELEE_BRAND] += temp;
				file_putf(log_file, "Adding %d for brands\n", temp);
			}
		}

		/* Handle hit / dam ratings - are they higher than normal? */
		/* Also handle other weapon/nonweapon abilities */
		if (art->tval == TV_BOW || art->tval == TV_DIGGING ||
			art->tval == TV_HAFTED || art->tval == TV_POLEARM ||
			art->tval == TV_SWORD) {
			m = randcalc(kind->to_h, 0, MINIMISE);
			temp = (art->to_h - m - mean_hit_startval) / mean_hit_increment;
			if (temp > 0)
				file_putf(log_file, "Adding %d instances of extra to-hit bonus for weapon\n", temp);
			else if (temp < 0)
				file_putf(log_file, "Subtracting %d instances of extra to-hit bonus for weapon\n", temp);
			
			artprobs[ART_IDX_WEAPON_HIT] += temp;

			m = randcalc(kind->to_d, 0, MINIMISE);
			temp = (art->to_d - m - mean_dam_startval) / mean_dam_increment;
			if (temp > 0)
				file_putf(log_file, "Adding %d instances of extra to-dam bonus for weapon\n", temp);
			else
				file_putf(log_file, "Subtracting %d instances of extra to-dam bonus for weapon\n", temp);

			artprobs[ART_IDX_WEAPON_DAM] += temp;

			/* Aggravation */
			if (of_has(art->flags, OF_AGGRAVATE))
			{
				file_putf(log_file, "Adding 1 for aggravation - weapon\n");
				artprobs[ART_IDX_WEAPON_AGGR]++;
			}

			/* End weapon stuff */
		} else {
			if ((art->to_h - randcalc(kind->to_h, 0, MINIMISE) > 0) &&
				(art->to_h - randcalc(kind->to_h, 0, MINIMISE) == art->to_d - randcalc(kind->to_d, 0, MINIMISE)) ) {
				/* Special case: both hit and dam bonuses present and equal */
				temp = (art->to_d - randcalc(kind->to_d, 0, MINIMISE)) / mean_dam_increment;
				if (temp > 0) {
					file_putf(log_file, "Adding %d instances of extra to-hit and to-dam bonus for non-weapon\n", temp);

					(artprobs[ART_IDX_NONWEAPON_HIT_DAM]) += temp;
				}
			} else {
				/* Uneven bonuses - handle separately */
				if (art->to_h - randcalc(kind->to_h, 0, MINIMISE) > 0) {
					temp = (art->to_d - randcalc(kind->to_d, 0, MINIMISE)) / mean_dam_increment;
					if (temp > 0) {
						file_putf(log_file, "Adding %d instances of extra to-hit bonus for non-weapon\n", temp);

						(artprobs[ART_IDX_NONWEAPON_HIT]) += temp;
					}
				}
				if (art->to_d - randcalc(kind->to_d, 0, MINIMISE) > 0) {
					temp = (art->to_d - randcalc(kind->to_d, 0, MINIMISE)) / mean_dam_increment;
					if (temp > 0) {
						file_putf(log_file, "Adding %d instances of extra to-dam bonus for non-weapon\n", temp);

						(artprobs[ART_IDX_NONWEAPON_DAM]) += temp;
					}
				}
			}

			/* Aggravation */
			if (of_has(art->flags, OF_AGGRAVATE)) {
				file_putf(log_file, "Adding 1 for aggravation - nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_AGGR])++;
			}

			/* Count brands and slays */
			if (art->slays) {
				temp = slay_count(art->slays);
				artprobs[ART_IDX_MELEE_SLAY] += temp;
				file_putf(log_file, "Adding %d for slays\n", temp);
			}
			if (art->brands) {
				temp = brand_count(art->brands);
				artprobs[ART_IDX_MELEE_BRAND] += temp;
				file_putf(log_file, "Adding %d for brands\n", temp);
			}

			if (art->modifiers[OBJ_MOD_BLOWS] > 0) {
				file_putf(log_file, "Adding 1 for extra blows on nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_BLOWS])++;
			}

			if (art->modifiers[OBJ_MOD_SHOTS] > 0) {
				file_putf(log_file, "Adding 1 for extra shots on nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_SHOTS])++;
			}
		}

		if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
			art->tval == TV_POLEARM || art->tval == TV_SWORD) {
			/* Blessed weapon? */
			if (of_has(art->flags, OF_BLESSED)) {
				file_putf(log_file, "Adding 1 for blessed weapon\n");
				(artprobs[ART_IDX_MELEE_BLESS])++;
			}

			/* See invisible? */
			if (of_has(art->flags, OF_SEE_INVIS)) {
				file_putf(log_file,
						  "Adding 1 for see invisible (weapon case)\n");
				(artprobs[ART_IDX_MELEE_SINV])++;
			}

			/* Do we have 3 or more extra blows? (Unlikely) */
			if (art->modifiers[OBJ_MOD_BLOWS] > 2) {
				file_putf(log_file,
						  "Adding 1 for supercharged blows (3 or more!)\n");
				(artprobs[ART_IDX_MELEE_BLOWS_SUPER])++;
			} else if (art->modifiers[OBJ_MOD_BLOWS] > 0) {
				file_putf(log_file, "Adding 1 for extra blows\n");
				(artprobs[ART_IDX_MELEE_BLOWS])++;
			}

			/* Does this weapon have an unusual bonus to AC? */
			if ((art->to_a - randcalc(kind->to_a, 0, MAXIMISE)) > 0) {
				temp = (art->to_a - randcalc(kind->to_a, 0, MAXIMISE)) / mean_ac_increment;
				if (temp > 0) {
					file_putf(log_file, "Adding %d instances of extra AC bonus for weapon\n", temp);
					(artprobs[ART_IDX_MELEE_AC]) += temp;
				}
			}

			/* Check damage dice - are they more than normal? */
			if (art->dd > kind->dd) {
				/* Difference of 3 or more? */
				if ( (art->dd - kind->dd) > 2) {
					file_putf(log_file,
							  "Adding 1 for super-charged damage dice!\n");
					(artprobs[ART_IDX_MELEE_DICE_SUPER])++;
				} else {
					file_putf(log_file, "Adding 1 for extra damage dice.\n");
					(artprobs[ART_IDX_MELEE_DICE])++;
				}
			}

			/* Check weight - is it different from normal? */
			if (art->weight != kind->weight) {
				file_putf(log_file, "Adding 1 for unusual weight.\n");
				(artprobs[ART_IDX_MELEE_WEIGHT])++;
			}

			/* Check for tunnelling ability */
			if (art->modifiers[OBJ_MOD_TUNNEL] > 0) {
				file_putf(log_file, "Adding 1 for tunnelling bonus.\n");
				(artprobs[ART_IDX_MELEE_TUNN])++;
			}

			/* Count brands and slays */
			if (art->slays) {
				temp = slay_count(art->slays);
				artprobs[ART_IDX_MELEE_SLAY] += temp;
				file_putf(log_file, "Adding %d for slays\n", temp);
			}
			if (art->brands) {
				temp = brand_count(art->brands);
				artprobs[ART_IDX_MELEE_BRAND] += temp;
				file_putf(log_file, "Adding %d for brands\n", temp);
			}

			/* End of weapon-specific stuff */
		} else {
			/* Check for tunnelling ability */
			if (art->modifiers[OBJ_MOD_TUNNEL] > 0) {
				file_putf(log_file,
						  "Adding 1 for tunnelling bonus - general.\n");
				(artprobs[ART_IDX_GEN_TUNN])++;
			}
		}

		/*
		 * Count up extra AC bonus values.
		 * Could also add logic to subtract for lower values here, but it's
		 * probably not worth the trouble since it's so rare.
		 */

		if ((art->to_a - randcalc(kind->to_a, 0, MINIMISE) - mean_ac_startval) > 0) {
			temp = (art->to_a - randcalc(kind->to_a, 0, MINIMISE) - mean_ac_startval) / mean_ac_increment;
			if (temp > 0) {
				if (art->to_a > 20) {
					file_putf(log_file, "Adding %d for supercharged AC\n",
							  temp);
					(artprobs[ART_IDX_GEN_AC_SUPER])++;
				} else if (art->tval == TV_BOOTS) {
					file_putf(log_file, "Adding %d for AC bonus - boots\n",
							  temp);
					(artprobs[ART_IDX_BOOT_AC]) += temp;
				} else if (art->tval == TV_GLOVES) {
					file_putf(log_file, "Adding %d for AC bonus - gloves\n",
							  temp);
					(artprobs[ART_IDX_GLOVE_AC]) += temp;
				} else if (art->tval == TV_HELM || art->tval == TV_CROWN) {
					file_putf(log_file, "Adding %d for AC bonus - headgear\n",
							  temp);
					(artprobs[ART_IDX_HELM_AC]) += temp;
				} else if (art->tval == TV_SHIELD) {
					file_putf(log_file, "Adding %d for AC bonus - shield\n",
							  temp);
					(artprobs[ART_IDX_SHIELD_AC]) += temp;
				} else if (art->tval == TV_CLOAK) {
					file_putf(log_file, "Adding %d for AC bonus - cloak\n",
							  temp);
					(artprobs[ART_IDX_CLOAK_AC]) += temp;
				} else if (art->tval == TV_SOFT_ARMOR ||
						   art->tval == TV_HARD_ARMOR ||
						   art->tval == TV_DRAG_ARMOR) {
					file_putf(log_file, "Adding %d for AC bonus - body armor\n",
							  temp);
					(artprobs[ART_IDX_ARMOR_AC]) += temp;
				} else {
					file_putf(log_file, "Adding %d for AC bonus - general\n",
							  temp);
					(artprobs[ART_IDX_GEN_AC]) += temp;
				}
			}
		}

		/* Generic armor abilities */
		if (art->tval == TV_BOOTS || art->tval == TV_GLOVES ||
			art->tval == TV_HELM || art->tval == TV_CROWN ||
			art->tval == TV_SHIELD || art->tval == TV_CLOAK ||
			art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
			art->tval == TV_DRAG_ARMOR) 		{
			/* Check weight - is it different from normal? */
			/* ToDo: count higher and lower separately */
			if (art->weight != kind->weight) {
				file_putf(log_file, "Adding 1 for unusual weight.\n");
				(artprobs[ART_IDX_ALLARMOR_WEIGHT])++;
			}

			/* Done with generic armor abilities */
		}

		/*
		 * General abilities.  This section requires a bit more work
		 * than the others, because we have to consider cases where
		 * a certain ability might be found in a particular item type.
		 * For example, ESP is commonly found on headgear, so when
		 * we count ESP we must add it to either the headgear or
		 * general tally, depending on the base item.  This permits
		 * us to have general abilities appear more commonly on a
		 * certain item type.
		 */

		/* Stat bonus case.  Add up the number of individual
		   bonuses */
		temp = 0;
		if (art->modifiers[OBJ_MOD_STR] > 0) temp++;
		if (art->modifiers[OBJ_MOD_INT] > 0) temp++;
		if (art->modifiers[OBJ_MOD_WIS] > 0) temp++;
		if (art->modifiers[OBJ_MOD_DEX] > 0) temp++;
		if (art->modifiers[OBJ_MOD_CON] > 0) temp++;

		/* Handle a few special cases separately. */
		if ((art->tval == TV_HELM || art->tval == TV_CROWN) &&
			(art->modifiers[OBJ_MOD_WIS] > 0 ||
			 art->modifiers[OBJ_MOD_INT] > 0)) {
			/* Handle WIS and INT on helms and crowns */
			if (art->modifiers[OBJ_MOD_WIS] > 0) {
				file_putf(log_file, "Adding 1 for WIS bonus on headgear.\n");
				(artprobs[ART_IDX_HELM_WIS])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}
			if (art->modifiers[OBJ_MOD_INT] > 0) {
				file_putf(log_file, "Adding 1 for INT bonus on headgear.\n");
				(artprobs[ART_IDX_HELM_INT])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}
		} else if ((art->tval == TV_SOFT_ARMOR ||
					art->tval == TV_HARD_ARMOR ||
					art->tval == TV_DRAG_ARMOR) && 
				   art->modifiers[OBJ_MOD_CON] > 0) {
			/* Handle CON bonus on armor */
			file_putf(log_file, "Adding 1 for CON bonus on body armor.\n");

			(artprobs[ART_IDX_ARMOR_CON])++;
			/* Counted this one separately so subtract it here */
			temp--;
		} else if (art->tval == TV_GLOVES && 
				   art->modifiers[OBJ_MOD_DEX] > 0) {
			/* Handle DEX bonus on gloves */
			file_putf(log_file, "Adding 1 for DEX bonus on gloves.\n");
			(artprobs[ART_IDX_GLOVE_DEX])++;
			/* Counted this one separately so subtract it here */
			temp--;
		}

		/* Now the general case */
		if (temp > 0) {
			/* There are some bonuses that weren't handled above */
			file_putf(log_file, "Adding %d for stat bonuses - general.\n",
					  temp);
			(artprobs[ART_IDX_GEN_STAT]) += temp;

			/* Done with stat bonuses */
		}

		if (flags_test(art->flags, OF_SIZE, OF_SUST_STR, OF_SUST_INT,
		                     OF_SUST_WIS, OF_SUST_DEX, OF_SUST_CON,
		                     FLAG_END)) {
			/* Now do sustains, in a similar manner */
			temp = 0;
			if (of_has(art->flags, OF_SUST_STR)) temp++;
			if (of_has(art->flags, OF_SUST_INT)) temp++;
			if (of_has(art->flags, OF_SUST_WIS)) temp++;
			if (of_has(art->flags, OF_SUST_DEX)) temp++;
			if (of_has(art->flags, OF_SUST_CON)) temp++;
			file_putf(log_file, "Adding %d for stat sustains.\n", temp);
			(artprobs[ART_IDX_GEN_SUST]) += temp;
		}

		if (art->modifiers[OBJ_MOD_STEALTH] > 0) {
			/* Handle stealth, including a couple of special cases */
			if (art->tval == TV_BOOTS) {
				file_putf(log_file, "Adding 1 for stealth bonus on boots.\n");
				(artprobs[ART_IDX_BOOT_STEALTH])++;
			} else if (art->tval == TV_CLOAK) {
				file_putf(log_file, "Adding 1 for stealth bonus on cloak.\n");
				(artprobs[ART_IDX_CLOAK_STEALTH])++;
			} else if (art->tval == TV_SOFT_ARMOR ||
					   art->tval == TV_HARD_ARMOR ||
					   art->tval == TV_DRAG_ARMOR) {
				file_putf(log_file, "Adding 1 for stealth bonus on armor.\n");

				(artprobs[ART_IDX_ARMOR_STEALTH])++;
			} else {
				/* General case */
				file_putf(log_file, "Adding 1 for stealth bonus - general.\n");
				(artprobs[ART_IDX_GEN_STEALTH])++;
			}
			/* Done with stealth */
		}

		if (art->modifiers[OBJ_MOD_SEARCH] > 0) {
			/* Handle searching bonus - fully generic this time */
			file_putf(log_file, "Adding 1 for search bonus - general.\n");
			(artprobs[ART_IDX_GEN_SEARCH])++;
		}

		if (art->modifiers[OBJ_MOD_INFRA] > 0) {
			/* Handle infravision bonus - fully generic */
			file_putf(log_file, "Adding 1 for infravision bonus - general.\n");
			(artprobs[ART_IDX_GEN_INFRA])++;
		}

		if (art->modifiers[OBJ_MOD_SPEED] > 0) {
			/*
			 * Speed - boots handled separately.
			 * This is something of a special case in that we use the same
			 * frequency for the supercharged value and the normal value.
			 * We get away with this by using a somewhat lower average value
			 * for the supercharged ability than in the basic set (around
			 * +7 or +8 - c.f. Ringil and the others at +10 and upwards).
			 * This then allows us to add an equal number of
			 * small bonuses around +3 or so without unbalancing things.
			 */

			if (art->modifiers[OBJ_MOD_SPEED] > 7) {
				/* Supercharge case */
				file_putf(log_file, "Adding 1 for supercharged speed bonus!\n");
				(artprobs[ART_IDX_GEN_SPEED_SUPER])++;
			} else if (art->tval == TV_BOOTS) {
				/* Handle boots separately */
				file_putf(log_file,
						  "Adding 1 for normal speed bonus on boots.\n");
				(artprobs[ART_IDX_BOOT_SPEED])++;
			} else {
				file_putf(log_file,
						  "Adding 1 for normal speed bonus - general.\n");
				(artprobs[ART_IDX_GEN_SPEED])++;
			}
			/* Done with speed */
		}

		if (of_has(art->flags, OF_FREE_ACT)) {
			/* Free action - handle gloves separately */
			if (art->tval == TV_GLOVES) {
				file_putf(log_file, "Adding 1 for free action on gloves.\n");
				(artprobs[ART_IDX_GLOVE_FA])++;
			} else {
				file_putf(log_file, "Adding 1 for free action - general.\n");
				(artprobs[ART_IDX_GEN_FA])++;
			}
		}

		if (of_has(art->flags, OF_HOLD_LIFE)) {
			/* Hold life - do body armor separately */
			if( (art->tval == TV_SOFT_ARMOR) ||
				(art->tval == TV_HARD_ARMOR) ||
				(art->tval == TV_DRAG_ARMOR)) {
				file_putf(log_file, "Adding 1 for hold life on armor.\n");
				(artprobs[ART_IDX_ARMOR_HLIFE])++;
			} else {
				file_putf(log_file, "Adding 1 for hold life - general.\n");
				(artprobs[ART_IDX_GEN_HLIFE])++;
			}
		}

		if (of_has(art->flags, OF_FEATHER)) {
			/* Feather fall - handle boots separately */
			if (art->tval == TV_BOOTS) {
				file_putf(log_file, "Adding 1 for feather fall on boots.\n");
				(artprobs[ART_IDX_BOOT_FEATHER])++;
			} else {
				file_putf(log_file, "Adding 1 for feather fall - general.\n");
				(artprobs[ART_IDX_GEN_FEATHER])++;
			}
		}

		if (art->modifiers[OBJ_MOD_LIGHT] > 0) {
			/* Handle permanent light */
			file_putf(log_file, "Adding 1 for light radius - general.\n");
			(artprobs[ART_IDX_GEN_LIGHT])++;
		}

		if (of_has(art->flags, OF_SEE_INVIS)) {
			/*
			 * Handle see invisible - do helms / crowns separately
			 * (Weapons were done already so exclude them)
			 */
			if(!(art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
				 art->tval == TV_POLEARM || art->tval == TV_SWORD)) {
				if (art->tval == TV_HELM || art->tval == TV_CROWN) {
					file_putf(log_file,
							  "Adding 1 for see invisible - headgear.\n");
					(artprobs[ART_IDX_HELM_SINV])++;
				} else {
					file_putf(log_file,
							  "Adding 1 for see invisible - general.\n");
					(artprobs[ART_IDX_GEN_SINV])++;
				}
			}
		}

		if (of_has(art->flags, OF_TELEPATHY)) {
			/* ESP case.  Handle helms/crowns separately. */
			if (art->tval == TV_HELM || art->tval == TV_CROWN) {
				file_putf(log_file, "Adding 1 for ESP on headgear.\n");
				(artprobs[ART_IDX_HELM_ESP])++;
			} else {
				file_putf(log_file, "Adding 1 for ESP - general.\n");
				(artprobs[ART_IDX_GEN_ESP])++;
			}
		}

		if (of_has(art->flags, OF_SLOW_DIGEST)) {
			/* Slow digestion case - generic. */
			file_putf(log_file, "Adding 1 for slow digestion - general.\n");
			(artprobs[ART_IDX_GEN_SDIG])++;
		}

		if (of_has(art->flags, OF_REGEN)) {
			/* Regeneration case - generic. */
			file_putf(log_file, "Adding 1 for regeneration - general.\n");
			(artprobs[ART_IDX_GEN_REGEN])++;
		}

		/* Count up immunities for this item, if any */
		temp = 0;
		if (art->el_info[ELEM_ACID].res_level == 3) temp++;
		if (art->el_info[ELEM_ELEC].res_level == 3) temp++;
		if (art->el_info[ELEM_FIRE].res_level == 3) temp++;
		if (art->el_info[ELEM_COLD].res_level == 3) temp++;
		file_putf(log_file, "Adding %d for immunities.\n", temp);

		(artprobs[ART_IDX_GEN_IMMUNE]) += temp;

		/* Count up low resists (not the type, just the number) */
		temp = 0;
		if (art->el_info[ELEM_ACID].res_level == 1) temp++;
		if (art->el_info[ELEM_ELEC].res_level == 1) temp++;
		if (art->el_info[ELEM_FIRE].res_level == 1) temp++;
		if (art->el_info[ELEM_COLD].res_level == 1) temp++;

		if (temp) {
			/* Shields treated separately */
			if (art->tval == TV_SHIELD) {
				file_putf(log_file, "Adding %d for low resists on shield.\n",
						  temp);
				(artprobs[ART_IDX_SHIELD_LRES]) += temp;
			}
			else if (art->tval == TV_SOFT_ARMOR ||
					 art->tval == TV_HARD_ARMOR ||
					 art->tval == TV_DRAG_ARMOR) {
				/* Armor also treated separately */
				if (temp == 4) {
					/* Special case: armor has all four low resists */
					file_putf(log_file,
							  "Adding 1 for ALL LOW RESISTS on body armor.\n");
					(artprobs[ART_IDX_ARMOR_ALLRES])++;
				} else {
					/* Just tally up the resists as usual */
					file_putf(log_file,
							  "Adding %d for low resists on body armor.\n",
							  temp);
					(artprobs[ART_IDX_ARMOR_LRES]) += temp;
				}
			} else {
				/* General case */
				file_putf(log_file, "Adding %d for low resists - general.\n",
						  temp);
				(artprobs[ART_IDX_GEN_LRES]) += temp;
			}
			/* Done with low resists */
		}

		/*
		 * If the item is body armor then count up all the high resists before
		 * going through them individually.  High resists are an important
		 * component of body armor so we track probability for them separately.
		 * The proportions of the high resists will be determined by the
		 * generic frequencies - this number just tracks the total.
		 */
		if (art->tval == TV_SOFT_ARMOR ||
			art->tval == TV_HARD_ARMOR ||
			art->tval == TV_DRAG_ARMOR) {
			temp = 0;
			if (art->el_info[ELEM_POIS].res_level == 1) temp++;
			if (of_has(art->flags, OF_PROT_FEAR)) temp++;
			if (art->el_info[ELEM_LIGHT].res_level == 1) temp++;
			if (art->el_info[ELEM_DARK].res_level == 1) temp++;
			if (of_has(art->flags, OF_PROT_BLIND)) temp++;
			if (of_has(art->flags, OF_PROT_CONF)) temp++;
			if (art->el_info[ELEM_SOUND].res_level == 1) temp++;
			if (art->el_info[ELEM_SHARD].res_level == 1) temp++;
			if (art->el_info[ELEM_NEXUS].res_level == 1) temp++;
			if (art->el_info[ELEM_NETHER].res_level == 1) temp++;
			if (art->el_info[ELEM_CHAOS].res_level == 1) temp++;
			if (art->el_info[ELEM_DISEN].res_level == 1) temp++;
			if (of_has(art->flags, OF_PROT_STUN)) temp++;
			file_putf(log_file, "Adding %d for high resists on body armor.\n",
					  temp);
			(artprobs[ART_IDX_ARMOR_HRES]) += temp;
		}

		/* Now do the high resists individually */
		if (art->el_info[ELEM_POIS].res_level == 1) {
			/* Resist poison ability */
			file_putf(log_file, "Adding 1 for resist poison - general.\n");

			(artprobs[ART_IDX_GEN_RPOIS])++;
		}

		if (of_has(art->flags, OF_PROT_FEAR)) {
			/* Resist fear ability */
			file_putf(log_file, "Adding 1 for resist fear - general.\n");
			(artprobs[ART_IDX_GEN_RFEAR])++;
		}

		if (art->el_info[ELEM_LIGHT].res_level == 1) {
			/* Resist light ability */
			file_putf(log_file, "Adding 1 for resist light - general.\n");
			(artprobs[ART_IDX_GEN_RLIGHT])++;
		}

		if (art->el_info[ELEM_DARK].res_level == 1) {
			/* Resist dark ability */
			file_putf(log_file, "Adding 1 for resist dark - general.\n");
			(artprobs[ART_IDX_GEN_RDARK])++;
		}

		if (of_has(art->flags, OF_PROT_BLIND)) {
			/* Resist blind ability - helms/crowns are separate */
			if (art->tval == TV_HELM || art->tval == TV_CROWN) {
				file_putf(log_file,
						  "Adding 1 for resist blindness - headgear.\n");
				(artprobs[ART_IDX_HELM_RBLIND])++;
			} else {
				/* General case */
				file_putf(log_file,
						  "Adding 1 for resist blindness - general.\n");
				(artprobs[ART_IDX_GEN_RBLIND])++;
			}
		}

		if (of_has(art->flags, OF_PROT_CONF)) {
			/* Resist confusion ability */
			file_putf(log_file, "Adding 1 for resist confusion - general.\n");
			(artprobs[ART_IDX_GEN_RCONF])++;
		}

		if (art->el_info[ELEM_SOUND].res_level == 1) {
			/* Resist sound ability */
			file_putf(log_file, "Adding 1 for resist sound - general.\n");
			(artprobs[ART_IDX_GEN_RSOUND])++;
		}

		if (art->el_info[ELEM_SHARD].res_level == 1) {
			/* Resist shards ability */
			file_putf(log_file, "Adding 1 for resist shards - general.\n");
			(artprobs[ART_IDX_GEN_RSHARD])++;
		}

		if (art->el_info[ELEM_NEXUS].res_level == 1) {
			/* Resist nexus ability */
			file_putf(log_file, "Adding 1 for resist nexus - general.\n");
			(artprobs[ART_IDX_GEN_RNEXUS])++;
		}

		if (art->el_info[ELEM_NETHER].res_level == 1) {
			/* Resist nether ability */
			file_putf(log_file, "Adding 1 for resist nether - general.\n");
			(artprobs[ART_IDX_GEN_RNETHER])++;
		}

		if (art->el_info[ELEM_CHAOS].res_level == 1) {
			/* Resist chaos ability */
			file_putf(log_file, "Adding 1 for resist chaos - general.\n");
			(artprobs[ART_IDX_GEN_RCHAOS])++;
		}

		if (art->el_info[ELEM_DISEN].res_level == 1) {
			/* Resist disenchantment ability */
			file_putf(log_file,
					  "Adding 1 for resist disenchantment - general.\n");
			(artprobs[ART_IDX_GEN_RDISEN])++;
		}

		if (of_has(art->flags, OF_PROT_STUN)) {
			/* Resist stunning ability */
			file_putf(log_file, "Adding 1 for res_stun - general.\n");
			(artprobs[ART_IDX_GEN_PSTUN])++;
		}

		if (art->activation) {
			/* Activation */
			file_putf(log_file, "Adding 1 for activation.\n");
			(artprobs[ART_IDX_GEN_ACTIV])++;
		}
		/* Done with parsing of frequencies for this item */
	}
	/* End for loop */

	if (verbose) {
	/* Print out some of the abilities, to make sure that everything's fine */
		for (i = 0; i < ART_IDX_TOTAL; i++)
			file_putf(log_file, "Frequency of ability %d: %d\n", i,
					  artprobs[i]);

		for (i = 0; i < z_info->k_max; i++)
			file_putf(log_file, "Frequency of item %d: %d\n", i, baseprobs[i]);
	}

	/*
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

	/* Bow-only abilities */
	for (i = 0; i < N_ELEMENTS(art_idx_bow); i++)
		artprobs[art_idx_bow[i]] = (artprobs[art_idx_bow[i]] * art_total)
			/ art_bow_total;

	/* All weapon abilities */
	for (i = 0; i < N_ELEMENTS(art_idx_weapon); i++)
		artprobs[art_idx_weapon[i]] = (artprobs[art_idx_weapon[i]] *
			art_total) / (art_bow_total + art_melee_total);

	/* Corresponding non-weapon abilities */
	temp = art_total - art_melee_total - art_bow_total;
	for (i = 0; i < N_ELEMENTS(art_idx_nonweapon); i++)
		artprobs[art_idx_nonweapon[i]] = (artprobs[art_idx_nonweapon[i]] *
			art_total) / temp;

	/* All melee weapon abilities */
	for (i = 0; i < N_ELEMENTS(art_idx_melee); i++)
		artprobs[art_idx_melee[i]] = (artprobs[art_idx_melee[i]] *
			art_total) / art_melee_total;

	/* All general armor abilities */
	temp = art_armor_total + art_boot_total + art_shield_total +
		art_headgear_total + art_cloak_total + art_glove_total;
	for (i = 0; i < N_ELEMENTS(art_idx_allarmor); i++)
		artprobs[art_idx_allarmor[i]] = (artprobs[art_idx_allarmor[i]] *
			art_total) / temp;

	/* Boots */
	for (i = 0; i < N_ELEMENTS(art_idx_boot); i++)
		artprobs[art_idx_boot[i]] = (artprobs[art_idx_boot[i]] *
			art_total) / art_boot_total;

	/* Gloves */
	for (i = 0; i < N_ELEMENTS(art_idx_glove); i++)
		artprobs[art_idx_glove[i]] = (artprobs[art_idx_glove[i]] *
			art_total) / art_glove_total;

	/* Headgear */
	for (i = 0; i < N_ELEMENTS(art_idx_headgear); i++)
		artprobs[art_idx_headgear[i]] = (artprobs[art_idx_headgear[i]] *
			art_total) / art_headgear_total;

	/* Shields */
	for (i = 0; i < N_ELEMENTS(art_idx_shield); i++)
		artprobs[art_idx_shield[i]] = (artprobs[art_idx_shield[i]] *
			art_total) / art_shield_total;

	/* Cloaks */
	for (i = 0; i < N_ELEMENTS(art_idx_cloak); i++)
		artprobs[art_idx_cloak[i]] = (artprobs[art_idx_cloak[i]] *
			art_total) / art_cloak_total;

	/* Body armor */
	for (i = 0; i < N_ELEMENTS(art_idx_armor); i++)
		artprobs[art_idx_armor[i]] = (artprobs[art_idx_armor[i]] *
			art_total) / art_armor_total;

	/*
	 * All others are general case and don't need to be rescaled,
	 * unless the algorithm is getting too clever about separating
	 * out individual cases (in which case some logic should be
	 * added for them in the following method call).
	 */

	/* Perform any additional rescaling and adjustment, if required. */
	adjust_freqs();

	/* Log the final frequencies to check that everything's correct */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		file_putf(log_file,  "Rescaled frequency of ability %d: %d\n", i,
				  artprobs[i]);

	/* Build a cumulative frequency table for the base items */
	for (i = 0; i < z_info->k_max; i++)
		for (j = i; j < z_info->k_max; j++)
			base_freq[j] += baseprobs[i];

	/* Print out the frequency table, for verification */
	for (i = 0; i < z_info->k_max; i++)
		file_putf(log_file, "Cumulative frequency of item %d is: %d\n", i,
				  base_freq[i]);
}

/**
 * Adds a flag to an artifact. Returns true when changes were made.
 */
static bool add_flag(struct artifact *art, int flag)
{
	if (of_has(art->flags, flag))
		return false;

	of_on(art->flags, flag);
	file_putf(log_file, "Adding ability: %s\n", flag_name(flag));

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
	file_putf(log_file, "Adding resistance to %s\n", elements[element].name);

	return true;
}

/**
 * Adds an immunity to an artifact. Returns true when changes were made.
 */
static void add_immunity(struct artifact *art)
{
	int r = randint0(4);
	art->el_info[r].res_level = 3;
	file_putf(log_file, "Adding immunity to %s\n", elements[r].name);
}

/**
 * Adds a flag and pval to an artifact. Always attempts
 * to increase the pval.
 */
static void add_pval_mod(struct artifact *art, int mod)
{
	art->modifiers[mod] = 1;
	do_mod(art);
	file_putf(log_file, "Adding ability: %s (now %+d)\n", mod_name(mod), 
			  fake_pval[0]);
}

/**
 * Adds a flag and a pval to an artifact, but won't increase
 * the pval if the flag is present. Returns true when changes were made.
 */
static bool add_fixed_pval_mod(struct artifact *art, int mod)
{
	if (art->modifiers[mod])
		return false;

	art->modifiers[mod] = 1;
	do_mod(art);
	file_putf(log_file, "Adding ability: %s (now %+d)\n", mod_name(mod), 
			  fake_pval[0]);

	return true;
}

/**
 * Adds a flag and an initial pval to an artifact.  Returns true
 * when the flag was not present.
 */
static bool add_first_pval_mod(struct artifact *art, int mod)
{
	art->modifiers[mod] = 1;

	if (fake_pval[0] == 0) {
		fake_pval[0] = (s16b)randint1(4);
		file_putf(log_file, "Adding ability: %s (first time) (now %+d)\n", 
				  mod_name(mod), fake_pval[0]);

		return true;
	}

	do_mod(art);
	file_putf(log_file, "Adding ability: %s (now %+d)\n", mod_name(mod), 
			  fake_pval[0]);

	return false;
}

/**
 * Adds a stat modifier, if possible
 */
static void add_stat(struct artifact *art)
{
	int r;
	bool success = false;

	/* Break out if all stats are raised to avoid an infinite loop */
	if (art->modifiers[OBJ_MOD_STR] && 
		art->modifiers[OBJ_MOD_INT] && 
		art->modifiers[OBJ_MOD_WIS] && 
		art->modifiers[OBJ_MOD_DEX] && 
		art->modifiers[OBJ_MOD_CON])
			return;

	/* Make sure we add one that hasn't been added yet */
	while (!success) {
		r = randint0(5);
		if (r == 0) success = add_fixed_pval_mod(art, OBJ_MOD_STR);
		else if (r == 1) success = add_fixed_pval_mod(art, OBJ_MOD_INT);
		else if (r == 2) success = add_fixed_pval_mod(art, OBJ_MOD_WIS);
		else if (r == 3) success = add_fixed_pval_mod(art, OBJ_MOD_DEX);
		else if (r == 4) success = add_fixed_pval_mod(art, OBJ_MOD_CON);
	}
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
static void add_high_resist(struct artifact *art)
{
	/* Add a high resist, according to the generated frequency distribution. */
	size_t i;
	int r, temp;
	int count = 0;
	bool success = false;

	temp = 0;
	for (i = 0; i < N_ELEMENTS(art_idx_high_resist); i++)
		temp += artprobs[art_idx_high_resist[i]];

	/* The following will fail (cleanly) if all high resists already added */
	while (!success && (count < MAX_TRIES)) {
		/* Randomize from 1 to this total amount */
		r = randint1(temp);

		/* Determine which (weighted) resist this number corresponds to */

		temp = artprobs[art_idx_high_resist[0]];
		i = 0;
		while (r > temp && i < N_ELEMENTS(art_idx_high_resist))	{
			temp += artprobs[art_idx_high_resist[i]];
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
	char *name;

	for (count = 0; count < MAX_TRIES; count++) {
		if (!append_random_brand(&art->brands, &name)) continue;
		file_putf(log_file, "Adding brand: %s\n", name);
		return;
	}
}

/**
 * Adds a slay, if possible
 */
static void add_slay(struct artifact *art)
{
	int count;
	char *name;

	for (count = 0; count < MAX_TRIES; count++) {
		if (!append_random_slay(&art->slays, &name)) continue;
		file_putf(log_file, "Adding slay: %s\n", name);
		return;
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
static void add_activation(struct artifact *art, s32b target_power)
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

		/*
		 * Check that activation is useful but not exploitable,
		 * and roughly proportionate to the overall power
		 */
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
static void build_freq_table(struct artifact *art, s16b *freq)
{
	int i;
	size_t j;
	s16b f_temp[ART_IDX_TOTAL];

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
			f_temp[art_idx_bow[j]] = artprobs[art_idx_bow[j]];
		}

	/* General weapon abilities */
	if (art->tval == TV_BOW || art->tval == TV_DIGGING ||
		art->tval == TV_HAFTED || art->tval == TV_POLEARM ||
		art->tval == TV_SWORD) {
		size_t n = N_ELEMENTS(art_idx_weapon);
		for (j = 0; j < n; j++)
			f_temp[art_idx_weapon[j]] = artprobs[art_idx_weapon[j]];
	}
	/* General non-weapon abilities */
	else {
		size_t n = N_ELEMENTS(art_idx_nonweapon);
		for (j = 0; j < n; j++)
			f_temp[art_idx_nonweapon[j]] = artprobs[art_idx_nonweapon[j]];
	}

	/* General melee abilities */
	if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
		art->tval == TV_POLEARM || art->tval == TV_SWORD) {
		size_t n = N_ELEMENTS(art_idx_melee);
		for (j = 0; j < n; j++)
			f_temp[art_idx_melee[j]] = artprobs[art_idx_melee[j]];
	}

	/* General armor abilities */
	if (art->tval == TV_BOOTS || art->tval == TV_GLOVES ||
		art->tval == TV_HELM || art->tval == TV_CROWN ||
		art->tval == TV_SHIELD || art->tval == TV_CLOAK ||
		art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
		art->tval == TV_DRAG_ARMOR) {
		size_t n = N_ELEMENTS(art_idx_allarmor);
		for (j = 0; j < n; j++)
			f_temp[art_idx_allarmor[j]] = artprobs[art_idx_allarmor[j]];
	}

	/* Boot abilities */
	if (art->tval == TV_BOOTS) {
		size_t n = N_ELEMENTS(art_idx_boot);
		for (j = 0; j < n; j++)
			f_temp[art_idx_boot[j]] = artprobs[art_idx_boot[j]];
	}

	/* Glove abilities */
	if (art->tval == TV_GLOVES) {
		size_t n = N_ELEMENTS(art_idx_glove);
		for (j = 0; j < n; j++)
			f_temp[art_idx_glove[j]] = artprobs[art_idx_glove[j]];
	}

	/* Headgear abilities */
	if (art->tval == TV_HELM || art->tval == TV_CROWN) {
		size_t n = N_ELEMENTS(art_idx_headgear);
		for (j = 0; j < n; j++)
			f_temp[art_idx_headgear[j]] = artprobs[art_idx_headgear[j]];
	}

	/* Shield abilities */
	if (art->tval == TV_SHIELD) {
		size_t n = N_ELEMENTS(art_idx_shield);
		for (j = 0; j < n; j++)
			f_temp[art_idx_shield[j]] = artprobs[art_idx_shield[j]];
	}

	/* Cloak abilities */
	if (art->tval == TV_CLOAK) {
		size_t n = N_ELEMENTS(art_idx_cloak);
		for (j = 0; j < n; j++)
			f_temp[art_idx_cloak[j]] = artprobs[art_idx_cloak[j]];
	}

	/* Armor abilities */
	if (art->tval == TV_SOFT_ARMOR || art->tval == TV_HARD_ARMOR ||
		art->tval == TV_DRAG_ARMOR) {
		size_t n = N_ELEMENTS(art_idx_armor);
		for (j = 0; j < n; j++)
			f_temp[art_idx_armor[j]] = artprobs[art_idx_armor[j]];
	}

	/* General abilities - no constraint */
	for (j = 0; j < N_ELEMENTS(art_idx_gen); j++)
		f_temp[art_idx_gen[j]] = artprobs[art_idx_gen[j]];

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
 * Choose a random ability using weights based on the given cumulative
 * frequency table.  A pointer to the frequency array (which must be of size
 * ART_IDX_TOTAL) is passed as a parameter.  The function returns a number
 * representing the index of the ability chosen.
 */

static int choose_ability (s16b *freq_table)
{
	int r, ability;

	/* Generate a random number between 1 and the last value in the table */
	r = randint1(freq_table[ART_IDX_TOTAL-1]);

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

static void add_ability_aux(struct artifact *art, int r, s32b target_power)
{
	switch(r)
	{
		case ART_IDX_BOW_SHOTS:
		case ART_IDX_NONWEAPON_SHOTS:
			add_pval_mod(art, OBJ_MOD_SHOTS);
			break;

		case ART_IDX_BOW_MIGHT:
			add_pval_mod(art, OBJ_MOD_MIGHT);
			break;

		case ART_IDX_WEAPON_HIT:
		case ART_IDX_NONWEAPON_HIT:
			add_to_hit(art, 1, 2 * mean_hit_increment);
			break;

		case ART_IDX_WEAPON_DAM:
		case ART_IDX_NONWEAPON_DAM:
			add_to_dam(art, 1, 2 * mean_dam_increment);
			break;

		case ART_IDX_NONWEAPON_HIT_DAM:
			add_to_hit(art, 1, 2 * mean_hit_increment);
			add_to_dam(art, 1, 2 * mean_dam_increment);
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
			add_pval_mod(art, OBJ_MOD_BLOWS);
			break;

		case ART_IDX_MELEE_AC:
		case ART_IDX_BOOT_AC:
		case ART_IDX_GLOVE_AC:
		case ART_IDX_HELM_AC:
		case ART_IDX_SHIELD_AC:
		case ART_IDX_CLOAK_AC:
		case ART_IDX_ARMOR_AC:
		case ART_IDX_GEN_AC:
			add_to_AC(art, 1, 2 * mean_ac_increment);
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
			add_pval_mod(art, OBJ_MOD_TUNNEL);
			break;

		case ART_IDX_BOOT_FEATHER:
		case ART_IDX_GEN_FEATHER:
			add_flag(art, OF_FEATHER);
			break;

		case ART_IDX_BOOT_STEALTH:
		case ART_IDX_CLOAK_STEALTH:
		case ART_IDX_ARMOR_STEALTH:
		case ART_IDX_GEN_STEALTH:
			add_pval_mod(art, OBJ_MOD_STEALTH);
			break;

		case ART_IDX_BOOT_SPEED:
		case ART_IDX_GEN_SPEED:
			add_first_pval_mod(art, OBJ_MOD_SPEED);
			break;

		case ART_IDX_GLOVE_FA:
		case ART_IDX_GEN_FA:
			add_flag(art, OF_FREE_ACT);
			break;

		case ART_IDX_GLOVE_DEX:
			add_fixed_pval_mod(art, OBJ_MOD_DEX);
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
			add_fixed_pval_mod(art, OBJ_MOD_WIS);
			break;

		case ART_IDX_HELM_INT:
			add_fixed_pval_mod(art, OBJ_MOD_INT);
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
			add_fixed_pval_mod(art, OBJ_MOD_CON);
			break;

		case ART_IDX_ARMOR_ALLRES:
			add_resist(art, ELEM_ACID);
			add_resist(art, ELEM_ELEC);
			add_resist(art, ELEM_FIRE);
			add_resist(art, ELEM_COLD);
			break;

		case ART_IDX_ARMOR_HRES:
			add_high_resist(art);
			break;

		case ART_IDX_GEN_STAT:
			add_stat(art);
			break;

		case ART_IDX_GEN_SUST:
			add_sustain(art);
			break;

		case ART_IDX_GEN_SEARCH:
			add_pval_mod(art, OBJ_MOD_SEARCH);
			break;

		case ART_IDX_GEN_INFRA:
			add_pval_mod(art, OBJ_MOD_INFRA);
			break;

		case ART_IDX_GEN_IMMUNE:
			add_immunity(art);
			break;

		case ART_IDX_GEN_LIGHT: {
				if (art->tval != TV_LIGHT && (fake_pval[0] != 0)) {
					art->modifiers[OBJ_MOD_LIGHT] = 2;
					fake_pval[1] = 1;
				} else
				break;
		}
			break;

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

		case ART_IDX_GEN_ACTIV:
			if (!art->activation) add_activation(art, target_power);
			break;
	}
}

/**
 * Randomly select an extra ability to be added to the artifact in question.
 */
static void add_ability(struct artifact *art, s32b target_power)
{
	int r;

	/* Choose a random ability using the frequency table previously defined*/
	r = choose_ability(art_freq);

	/* Add the appropriate ability */
	add_ability_aux(art, r, target_power);

	/* Now remove contradictory or redundant powers. */
	remove_contradictory(art);

	/* Adding WIS to sharp weapons always blesses them */
	if (art->modifiers[OBJ_MOD_WIS] &&
		(art->tval == TV_SWORD || art->tval == TV_POLEARM))
		add_flag(art, OF_BLESSED);
}


/**
 * Try to supercharge this item by running through the list of the supercharge
 * abilities and attempting to add each in turn.  An artifact only gets one
 * chance at each of these up front (if applicable).
 */
static void try_supercharge(struct artifact *art, s32b target_power)
{
	/* Huge damage dice or max blows - melee weapon only */
	if (art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
		art->tval == TV_POLEARM || art->tval == TV_SWORD) {
		/* Damage dice */
		if (randint0(z_info->a_max) < artprobs[ART_IDX_MELEE_DICE_SUPER]) {
			art->dd += 3 + randint0(4);
			file_putf(log_file, "Supercharging damage dice!  (Now %d dice)\n",
					  art->dd);
		} else if (randint0(z_info->a_max) < artprobs[ART_IDX_MELEE_BLOWS_SUPER]) {
			/* Blows */
			art->modifiers[OBJ_MOD_BLOWS] = 1;
			fake_pval[0] = INHIBIT_BLOWS - 1;
			file_putf(log_file, "Supercharging melee blows! (+2 blows)\n");
		}
	}

	/* Bows - max might or shots */
	if (art->tval == TV_BOW) {
		if (randint0(z_info->a_max) < artprobs[ART_IDX_BOW_SHOTS_SUPER]) {
			art->modifiers[OBJ_MOD_SHOTS] = 1;
			fake_pval[0] = INHIBIT_SHOTS - 1;
			file_putf(log_file, "Supercharging shots for bow!  (2 extra shots)\n");
		} else if (randint0(z_info->a_max) < artprobs[ART_IDX_BOW_MIGHT_SUPER]){
			art->modifiers[OBJ_MOD_MIGHT] = 1;
			fake_pval[0] = INHIBIT_MIGHT - 1;
			file_putf(log_file, "Supercharging might for bow!  (3 extra might)\n");
		}
	}

	/* Big speed bonus - any item (potentially) but more likely on boots */
	if (randint0(z_info->a_max) < artprobs[ART_IDX_GEN_SPEED_SUPER] ||
		(art->tval == TV_BOOTS && randint0(z_info->a_max) <
		artprobs[ART_IDX_BOOT_SPEED])) {
		art->modifiers[OBJ_MOD_SPEED] = 1;
		fake_pval[0] = 5 + randint0(6);
		if (INHIBIT_WEAK) fake_pval[0] += randint1(3);
		if (INHIBIT_STRONG) fake_pval[0] += 1 + randint1(6);
		file_putf(log_file, "Supercharging speed for this item!  (New speed bonus is %d)\n", fake_pval[0]);
	}

	/* Big AC bonus */
	if (randint0(z_info->a_max) < artprobs[ART_IDX_GEN_AC_SUPER]) {
		art->to_a += 19 + randint1(11);
		if (INHIBIT_WEAK) art->to_a += randint1(10);
		if (INHIBIT_STRONG) art->to_a += randint1(20);
		file_putf(log_file, "Supercharging AC! New AC bonus is %d\n",
				  art->to_a);
	}

	/* Aggravation */
	if (art->tval == TV_BOW || art->tval == TV_DIGGING ||
		art->tval == TV_HAFTED || art->tval == TV_POLEARM ||
		art->tval == TV_SWORD) {
		if ((randint0(z_info->a_max) < artprobs[ART_IDX_WEAPON_AGGR]) &&
		    (target_power > AGGR_POWER)) {
			of_on(art->flags, OF_AGGRAVATE);
			file_putf(log_file, "Adding aggravation\n");
		}
	} else {
		if ((randint0(z_info->a_max) < artprobs[ART_IDX_NONWEAPON_AGGR]) &&
		    (target_power > AGGR_POWER)) {
			of_on(art->flags, OF_AGGRAVATE);
			file_putf(log_file, "Adding aggravation\n");
		}
	}
}

/**
 * Make it bad, or if it's already bad, make it worse!
 */
static void do_curse(struct artifact *art)
{
	if (one_in_(7))
		of_on(art->flags, OF_AGGRAVATE);
	if (one_in_(4))
		of_on(art->flags, OF_DRAIN_EXP);
	if (one_in_(7))
		of_on(art->flags, OF_TELEPORT);

	if ((fake_pval[0] > 0) && one_in_(2))
		fake_pval[0] = -fake_pval[0];
	if ((art->to_a > 0) && one_in_(2))
		art->to_a = -art->to_a;
	if ((art->to_h > 0) && one_in_(2))
		art->to_h = -art->to_h;
	if ((art->to_d > 0) && one_in_(4))
		art->to_d = -art->to_d;

	if (of_has(art->flags, OF_LIGHT_CURSE)) {
		if (one_in_(2)) of_on(art->flags, OF_HEAVY_CURSE);
		return;
	}

	of_on(art->flags, OF_LIGHT_CURSE);

	if (one_in_(4))
		of_on(art->flags, OF_HEAVY_CURSE);
}

/**
 * Copy artifact fields from a_src to a_dst, and fake pvals from
 * fake_pval_src to fake_pval_dst
 */

static void copy_artifact(struct artifact *a_src, 
	struct artifact *a_dst, int *fake_pval_src, int *fake_pval_dst)
{
	int i;

	if (a_dst->slays) {
		free_slay(a_dst->slays);
	}
	if (a_dst->brands) {
		free_brand(a_dst->brands);
	}
	/* Copy the structure */
	memcpy(a_dst, a_src, sizeof(struct artifact));

	a_dst->next = NULL;
	a_dst->slays = NULL;
	a_dst->brands = NULL;
	if (a_dst->tval != TV_LIGHT)
		a_dst->activation = NULL;
	a_dst->alt_msg = NULL;

	if (a_src->slays) {
		copy_slay(&a_dst->slays, a_src->slays);
	}
	if (a_src->brands) {
		copy_brand(&a_dst->brands, a_src->brands);
	}

	/* Save contents of fake_pval */
	for (i = 0; i < 3; i++) {
		fake_pval_dst[i] = fake_pval_src[i];
	}
}

/**
 * Scramble an artifact.  Regular artifacts have their base type changed at
 * random, whereas special artifacts keep theirs.
 *
 * Note the three special cases (One Ring, Grond, Morgoth).
 *
 * This code is full of intricacies developed over years of tweaking.
 */
static void scramble_artifact(int a_idx)
{
	struct artifact *art = &a_info[a_idx];
	struct object_kind *kind = lookup_kind(art->tval, art->sval);
	struct artifact *a_old = mem_zalloc(sizeof *a_old);
	s32b power;
	int tries = 0;
	byte alloc_old, base_alloc_old, alloc_new;
	s32b ap = 0;
	bool curse_me = false;
	bool success = false;
	int i;
	int fake_pval_save[3] = {0, 0, 0};

	bool special_artifact = kf_has(kind->kind_flags, KF_INSTA_ART);

	/* Skip unused artifacts */
	if (art->tval == 0) return;

	/* Special cases -- don't randomize these! */
	if (strstr(art->name, "The One Ring") ||
		kf_has(kind->kind_flags, KF_QUEST_ART))
		return;

	/* Evaluate the original artifact to determine the power level. */
	power = base_power[a_idx];

	/* If it has a restricted ability then don't randomize it. */
	if (power > INHIBIT_POWER) {
		file_putf(log_file, "Skipping artifact number %d - too powerful to randomize!", a_idx);
		return;
	}

	mods_to_fake_pvals(art);

	if (power < 0) curse_me = true;

	file_putf(log_file, "+++++++++++++++++ CREATING NEW ARTIFACT ++++++++++++++++++\n");
	file_putf(log_file, "Artifact %d: power = %d\n", a_idx, power);

	/*
	 * Flip the sign on power if it's negative, since it's only used for base
	 * item choice
	 */
	if (power < 0) power = -power;

	if (!special_artifact) {
		/*
		 * Normal artifact - choose a random base item type.  Not too
		 * powerful, so we'll have to add something to it.  Not too
		 * weak, for the opposite reason.
		 *
		 * CR 7/15/2001 - lowered the upper limit so that we get at
		 * least a few powers (from 8/10 to 6/10) but permit anything
		 * more than 20 below the target power
		 */
		int count = 0;
		s32b ap2;

		/* Capture the rarity of the original base item and artifact */
		alloc_old = base_art_alloc[a_idx];
		base_alloc_old = base_item_prob[a_idx];
		do {
			/* Get the new item kind */
			kind = choose_item(a_idx);

			/*
			 * Hack: if power is positive but very low, and if we're not having
			 * any luck finding a base item, curse it once.  This helps ensure
			 * that we get a base item for borderline cases like Wormtongue.
			 */

			if (power > 0 && power < 10 && count > MAX_TRIES / 2) {
				file_putf(log_file, "Cursing base item to help get a match.\n");
				do_curse(art);
			}
			ap2 = artifact_power(a_idx, true);
			count++;
			/*
			 * Calculate the proper rarity based on the new type.  We attempt
			 * to preserve the 'effective rarity' which is equal to the
			 * artifact rarity multiplied by the base item rarity.
			 */

			alloc_new = alloc_old * base_alloc_old / kind->alloc_prob;

			if (alloc_new > 99) alloc_new = 99;
			if (alloc_new < 1) alloc_new = 1;

			file_putf(log_file, "Old allocs are base %d, art %d\n",
					  base_alloc_old, alloc_old);
			file_putf(log_file, "New allocs are base %d, art %d\n",
					  kind->alloc_prob, alloc_new);

		} while ((count < MAX_TRIES) &&
				 (((ap2 > (power * 6) / 10 + 1) && (power-ap2 < 20)) ||
		          (ap2 < (power / 10))));

		/* Got an item - set the new rarity */
		art->alloc_prob = alloc_new;

		if (count >= MAX_TRIES)
			file_putf(log_file, "Warning! Couldn't get appropriate power level on base item.\n");
	} else {
		/* Special artifact (light source, ring, or amulet) */

		/* Keep the item kind */
		kind = lookup_kind(art->tval, art->sval);

		/* Clear the following fields; leave the rest alone */
		art->to_h = art->to_d = art->to_a = 0;
		of_wipe(art->flags);
		for (i = 0; i < ELEM_MAX; i++) {
			art->el_info[i].res_level = 0;
			art->el_info[i].flags = 0;
		}
		for (i = 0; i < OBJ_MOD_MAX; i++)
			art->modifiers[i] = 0;
		wipe_brands(art->brands);
		art->brands = NULL;
		wipe_slays(art->slays);
		art->slays = NULL;

		/* Clear the activations for rings and amulets but not lights */
		if ((art->tval != TV_LIGHT) && art->activation)
			art->activation = NULL;

		/* Restore lights */
		else {
			of_on(art->flags, OF_NO_FUEL);
			art->modifiers[OBJ_MOD_LIGHT] = 1;
			fake_pval[0] = 3;
		}
		/* Artifacts ignore everything */
		for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++)
			art->el_info[i].flags |= EL_INFO_IGNORE;

		file_putf(log_file, "Alloc prob is %d\n", art->alloc_prob);
	}

	/* Generate the cumulative frequency table for this base item type */
	build_freq_table(art, art_freq);

	/* Copy artifact info temporarily. */
	copy_artifact(art, a_old, fake_pval, fake_pval_save);

	/* Give this artifact a shot at being supercharged */
	try_supercharge(art, power);
	ap = artifact_power(a_idx, true);
	if (ap > (power * 23) / 20 + 1)	{
		/* too powerful -- put it back */
		copy_artifact(a_old, art, fake_pval_save, fake_pval);
		file_putf(log_file, "--- Supercharge is too powerful! Rolling back.\n");
	}

	/* First draft: add two abilities, then curse it three times. */
	if (curse_me) {
		/* Copy artifact info temporarily. */
		copy_artifact(art, a_old, fake_pval, fake_pval_save);
		do {
			add_ability(art, power);
			add_ability(art, power);
			do_curse(art);
			do_curse(art);
			do_curse(art);
			remove_contradictory(art);
			ap = artifact_power(a_idx, true);
			/* Accept if it doesn't have any inhibited abilities */
			if (ap < INHIBIT_POWER)
				success = true;
			/* Otherwise go back and try again */
			else {
				file_putf(log_file, "Inhibited ability added - rolling back\n");
				copy_artifact(a_old, art, fake_pval_save, fake_pval);
			}
		} while (!success);

		/* Cursed items never have any resale value */
		art->cost = 0;
	} else {
		/*
		 * Select a random set of abilities which roughly matches the
		 * original's in terms of overall power/usefulness.
		 */
		for (tries = 0; tries < MAX_TRIES; tries++) {
			/* Copy artifact info temporarily. */
			copy_artifact(art, a_old, fake_pval, fake_pval_save);

			add_ability(art, power);
			ap = artifact_power(a_idx, true);

			/* CR 11/14/01 - pushed both limits up by about 5% */
			if (ap > (power * 23) / 20 + 1) {
				/* too powerful -- put it back */
				copy_artifact(a_old, art, fake_pval_save, fake_pval);
				file_putf(log_file, "--- Too powerful!  Rolling back.\n");
				continue;
			} else if (ap >= (power * 19) / 20) {	/* just right */
				/* CC 11/02/09 - add rescue for crappy weapons */
				if ((art->tval == TV_DIGGING || art->tval == TV_HAFTED ||
					art->tval == TV_POLEARM || art->tval == TV_SWORD
					|| art->tval == TV_BOW) && (art->to_d < 10)) {
					art->to_d += randint0(10);
					file_putf(log_file, "Redeeming crappy weapon: +dam now %d\n", art->to_d);
				}
				break;
			}
		}		/* end of power selection */

		if (verbose && tries >= MAX_TRIES)
			/*
			 * We couldn't generate an artifact within the number of permitted
			 * iterations.  Show a warning message.
			 */
			file_putf(log_file, "Warning!  Couldn't get appropriate power level on artifact.\n");
	}

	/* Cleanup a_old */
	if (a_old->slays) {
		free_slay(a_old->slays);
	}
	if (a_old->brands) {
		free_brand(a_old->brands);
	}
	mem_free(a_old);

	/* Set depth and rarity info according to power */
	/* This is currently very tricky for special artifacts */
	file_putf(log_file, "Old depths are min %d, max %d\n", art->alloc_min,
			  art->alloc_max);
	file_putf(log_file, "Alloc prob is %d\n", art->alloc_prob);

	/* Flip cursed items to avoid overflows */
	if (ap < 0) ap = -ap;

	if (special_artifact) {
		art->alloc_max = 127;
		if (ap > avg_power) {
			art->alloc_prob = 1;
			art->alloc_min = MAX(50, ((ap + 150) * 100 /
				max_power));
		} else if (ap > 30) {
			art->alloc_prob = MAX(2, (avg_power - ap) / 20);
			art->alloc_min = MAX(25, ((ap + 200) * 100 /
				max_power));
		} else {/* Just the Phial */
			art->alloc_prob = 50 - ap;
			art->alloc_min = 5;
		}
	} else {
		file_putf(log_file, "kind->alloc_prob is %d\n", kind->alloc_prob);
		art->alloc_max = MIN(127, (ap * 4) / 5);
		art->alloc_min = MIN(100, ((ap + 100) * 100 / max_power));

		/* Leave alloc_prob consistent with base art total rarity */
	}

	/* Sanity check */
	if (art->alloc_prob > 99) art->alloc_prob = 99;
	if (art->alloc_prob < 1) art->alloc_prob = 1;

	/* Write the mods back in */
	fake_pvals_to_mods(art);

	/* Ensure diggers keep a basic digging bonus */
	if (art->modifiers[OBJ_MOD_TUNNEL] < kind->modifiers[OBJ_MOD_TUNNEL].base)
		art->modifiers[OBJ_MOD_TUNNEL] = kind->modifiers[OBJ_MOD_TUNNEL].base;

	file_putf(log_file, "New depths are min %d, max %d\n", art->alloc_min,
			  art->alloc_max);
	file_putf(log_file, "Power-based alloc_prob is %d\n", art->alloc_prob);

	/* Success */
	file_putf(log_file, ">>>>>>>>>>>>>>>>>>>>>>>>>> ARTIFACT COMPLETED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	file_putf(log_file, "Number of tries for artifact %d was: %d\n", a_idx, tries);
}

/**
 * Return true if the whole set of random artifacts meets certain
 * criteria.  Return false if we fail to meet those criteria (which will
 * restart the whole process).
 */
static bool artifacts_acceptable(void)
{
	int swords = 5, polearms = 5, blunts = 5, bows = 4;
	int bodies = 5, shields = 4, cloaks = 4, hats = 4;
	int gloves = 4, boots = 4;
	int i;

	for (i = 0; i < z_info->a_max; i++)
	{
		switch (a_info[i].tval)
		{
			case TV_SWORD:
				swords--; break;
			case TV_POLEARM:
				polearms--; break;
			case TV_HAFTED:
				blunts--; break;
			case TV_BOW:
				bows--; break;
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				bodies--; break;
			case TV_SHIELD:
				shields--; break;
			case TV_CLOAK:
				cloaks--; break;
			case TV_HELM:
			case TV_CROWN:
				hats--; break;
			case TV_GLOVES:
				gloves--; break;
			case TV_BOOTS:
				boots--; break;
		}
	}

	file_putf(log_file, "Deficit amount for swords is %d\n", swords);
	file_putf(log_file, "Deficit amount for polearms is %d\n", polearms);
	file_putf(log_file, "Deficit amount for blunts is %d\n", blunts);
	file_putf(log_file, "Deficit amount for bows is %d\n", bows);
	file_putf(log_file, "Deficit amount for bodies is %d\n", bodies);
	file_putf(log_file, "Deficit amount for shields is %d\n", shields);
	file_putf(log_file, "Deficit amount for cloaks is %d\n", cloaks);
	file_putf(log_file, "Deficit amount for hats is %d\n", hats);
	file_putf(log_file, "Deficit amount for gloves is %d\n", gloves);
	file_putf(log_file, "Deficit amount for boots is %d\n", boots);

	if (swords > 0 || polearms > 0 || blunts > 0 || bows > 0 ||
	    bodies > 0 || shields > 0 || cloaks > 0 || hats > 0 ||
	    gloves > 0 || boots > 0) {
		if (verbose) {
			char types[256];
			strnfmt(types, sizeof(types), "%s%s%s%s%s%s%s%s%s%s",
					swords > 0 ? " swords" : "",
					polearms > 0 ? " polearms" : "",
					blunts > 0 ? " blunts" : "",
					bows > 0 ? " bows" : "",
					bodies > 0 ? " body-armors" : "",
					shields > 0 ? " shields" : "",
					cloaks > 0 ? " cloaks" : "",
					hats > 0 ? " hats" : "",
					gloves > 0 ? " gloves" : "",
					boots > 0 ? " boots" : "");

			file_putf(log_file, "Restarting generation process: not enough%s",
					  types);
		}
		return false;
	} else
		return true;
}

/**
 * Scramble each artifact
 */
static errr scramble(void)
{
	/* If our artifact set fails to meet certain criteria, we start over. */
	do {
		int a_idx;

		/* Generate all the artifacts. */
		for (a_idx = 1; a_idx < z_info->a_max; a_idx++)
			scramble_artifact(a_idx);
	} while (!artifacts_acceptable());

	/* Success */
	return (0);
}

/**
 * Use W. Sheldon Simms' random name generator.
 */
char *artifact_gen_name(struct artifact *a, const char ***words) {
	char buf[BUFLEN];
	char word[MAX_NAME_LEN + 1];
	struct object_kind *kind = lookup_kind(a->tval, a->sval);

	randname_make(RANDNAME_TOLKIEN, MIN_NAME_LEN, MAX_NAME_LEN, word,
				  sizeof(word), words);
	my_strcap(word);

	if (strstr(a->name, "The One Ring"))
		strnfmt(buf, sizeof(buf), "(The One Ring)");
	else if (kf_has(kind->kind_flags, KF_QUEST_ART))
		strnfmt(buf, sizeof(buf), a->name);
	else if (strstr(kind->name, "Ring of") || one_in_(3))
		/* Hack - the activation message for rings of power assumes this */
		strnfmt(buf, sizeof(buf), "'%s'", word);
	else
		strnfmt(buf, sizeof(buf), "of %s", word);
	return string_make(buf);
}

/**
 * Initialise all the artifact names.
 */
static errr init_names(void)
{
	int i;
	struct artifact *a;

	for (i = 0; i < z_info->a_max; i++) {
		char desc[128] = "Based on ";

		a = &a_info[i];
		if (!a->tval || !a->sval || !a->name) continue;

		if (prefix(a->name, "of Power"))
			my_strcat(desc, a->name + 10, strlen(a->name) - 1);
		else if (prefix(a->name, "of "))
			my_strcat(desc, a->name + 3, strlen(a->name) + 7);
		else
			my_strcat(desc, a->name + 1, strlen(a->name) + 8);

		a->text = string_make(desc);
		a->name = artifact_gen_name(a, name_sections);
	}

	return 0;
}

/**
 * Call the name allocation and artifact scrambling routines
 */
static errr do_randart_aux(bool full)
{
	errr result;

	/* Generate random names */
	if ((result = init_names()) != 0) return (result);

	/* Randomize the artifacts */
	if (full)
		if ((result = scramble()) != 0) return (result);

	/* Success */
	return (0);
}


/**
 * Randomize the artifacts
 *
 * The full flag toggles between just randomizing the names and
 * complete randomization of the artifacts.
 */
errr do_randart(u32b randart_seed, bool full)
{
	errr err;

	/* Prepare to use the Angband "simple" RNG. */
	Rand_value = randart_seed;
	Rand_quick = true;

	/* Only do all the following if full randomization requested */
	if (full) {
		/* Allocate the various "original powers" arrays */
		base_power = mem_zalloc(z_info->a_max * sizeof(s32b));
		base_item_level = mem_zalloc(z_info->a_max * sizeof(byte));
		base_item_prob = mem_zalloc(z_info->a_max * sizeof(byte));
		base_art_alloc = mem_zalloc(z_info->a_max * sizeof(byte));
		baseprobs = mem_zalloc(z_info->k_max * sizeof(s16b));
		base_freq = mem_zalloc(z_info->k_max * sizeof(s16b));

		/* Open the log file for writing */
		if (verbose) {
			char buf[1024];
			path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "randart.log");
			log_file = file_open(buf, MODE_WRITE, FTYPE_TEXT);
			if (!log_file) {
				msg("Error - can't open randart.log for writing.");
				exit(1);
			}
		}

		/* Store the original power ratings */
		store_base_power();

		/* Determine the generation probabilities */
		parse_frequencies();
	}

	/* Generate the random artifact (names) */
	err = do_randart_aux(full);

	/* Only do all the following if full randomization requested */
	if (full) {
		/* Just for fun, look at the frequencies on the finished items */
		/* Remove this prior to release */
		store_base_power();
		parse_frequencies();

		/* Close the log file */
		if (verbose) {
			if (!file_close(log_file))
			{
				msg("Error - can't close randart.log file.");
				exit(1);
			}
		}

		/* Free the "original powers" arrays */
		mem_free(base_power);
		mem_free(base_item_level);
		mem_free(base_item_prob);
		mem_free(base_art_alloc);
		mem_free(baseprobs);
		mem_free(base_freq);
	}

	/* When done, resume use of the Angband "complex" RNG. */
	Rand_quick = false;

	return (err);
}
