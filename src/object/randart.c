
/*
 * File: randart.c
 * Purpose: Random artifact generation
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
 */
#include "angband.h"
#include "object/slays.h"
#include "object/tvalsval.h"
#include "object/pval.h"
#include "init.h"
#include "effects.h"
#include "randname.h"

/*
 * Original random artifact generator (randart) by Greg Wooledge.
 * Updated by Chris Carr / Chris Robertson 2001-2010.
 */

#define MAX_TRIES 200
#define BUFLEN 1024

#define MIN_NAME_LEN 5
#define MAX_NAME_LEN 9
#define S_WORD 26
#define E_WORD S_WORD

/*
 * Inhibiting factors for large bonus values
 * "HIGH" values use INHIBIT_WEAK
 * "VERYHIGH" values use INHIBIT_STRONG
 */
#define INHIBIT_STRONG  (one_in_(6))
#define INHIBIT_WEAK    (one_in_(2))

/*
 * Power rating below which uncursed randarts cannot aggravate
 * (so that aggravate is found only on endgame-quality items or
 * cursed items)
 */
#define AGGR_POWER 300

/*
 * Numerical index values for the different learned probabilities
 * These are to make the code more readable.
 * ToDo: turn these into an enum
 */

#define ART_IDX_BOW_SHOTS 0
#define ART_IDX_BOW_MIGHT 1
#define ART_IDX_BOW_BRAND 80
#define ART_IDX_BOW_SLAY 81
#define ART_IDX_WEAPON_HIT 2
#define ART_IDX_WEAPON_DAM 3
#define ART_IDX_NONWEAPON_HIT 4
#define ART_IDX_NONWEAPON_DAM 5
#define ART_IDX_NONWEAPON_HIT_DAM 6
#define ART_IDX_NONWEAPON_BRAND 78
#define ART_IDX_NONWEAPON_SLAY 79
#define ART_IDX_NONWEAPON_BLOWS 83
#define ART_IDX_NONWEAPON_SHOTS 84

#define ART_IDX_MELEE_BLESS 7
#define ART_IDX_MELEE_BRAND 8
#define ART_IDX_MELEE_SLAY 76
#define ART_IDX_MELEE_SINV 9
#define ART_IDX_MELEE_BLOWS 10
#define ART_IDX_MELEE_AC 11
#define ART_IDX_MELEE_DICE 12
#define ART_IDX_MELEE_WEIGHT 13
#define ART_IDX_MELEE_TUNN 14

#define ART_IDX_ALLARMOR_WEIGHT 15

#define ART_IDX_BOOT_AC 16
#define ART_IDX_BOOT_FEATHER 17
#define ART_IDX_BOOT_STEALTH 18
#define ART_IDX_BOOT_SPEED 19

#define ART_IDX_GLOVE_AC 20
#define ART_IDX_GLOVE_FA 21
#define ART_IDX_GLOVE_DEX 22

#define ART_IDX_HELM_AC 23
#define ART_IDX_HELM_RBLIND 24
#define ART_IDX_HELM_ESP 25
#define ART_IDX_HELM_SINV 26
#define ART_IDX_HELM_WIS 27
#define ART_IDX_HELM_INT 28

#define ART_IDX_SHIELD_AC 29
#define ART_IDX_SHIELD_LRES 30

#define ART_IDX_CLOAK_AC 31
#define ART_IDX_CLOAK_STEALTH 32

#define ART_IDX_ARMOR_AC 33
#define ART_IDX_ARMOR_STEALTH 34
#define ART_IDX_ARMOR_HLIFE 35
#define ART_IDX_ARMOR_CON 36
#define ART_IDX_ARMOR_LRES 37
#define ART_IDX_ARMOR_ALLRES 38
#define ART_IDX_ARMOR_HRES 39

#define ART_IDX_GEN_STAT 40
#define ART_IDX_GEN_SUST 41
#define ART_IDX_GEN_STEALTH 42
#define ART_IDX_GEN_SEARCH 43
#define ART_IDX_GEN_INFRA 44
#define ART_IDX_GEN_SPEED 45
#define ART_IDX_GEN_IMMUNE 46
#define ART_IDX_GEN_FA 47
#define ART_IDX_GEN_HLIFE 48
#define ART_IDX_GEN_FEATHER 49
#define ART_IDX_GEN_LIGHT 50
#define ART_IDX_GEN_SINV 51
#define ART_IDX_GEN_ESP 52
#define ART_IDX_GEN_SDIG 53
#define ART_IDX_GEN_REGEN 54
#define ART_IDX_GEN_LRES 55
#define ART_IDX_GEN_RPOIS 56
#define ART_IDX_GEN_RFEAR 57
#define ART_IDX_GEN_RLIGHT 58
#define ART_IDX_GEN_RDARK 59
#define ART_IDX_GEN_RBLIND 60
#define ART_IDX_GEN_RCONF 61
#define ART_IDX_GEN_RSOUND 62
#define ART_IDX_GEN_RSHARD 63
#define ART_IDX_GEN_RNEXUS 64
#define ART_IDX_GEN_RNETHER 65
#define ART_IDX_GEN_RCHAOS 66
#define ART_IDX_GEN_RDISEN 67
#define ART_IDX_GEN_AC 68
#define ART_IDX_GEN_TUNN 69
#define ART_IDX_GEN_ACTIV 82

/* Supercharged abilities - treated differently in algorithm */

#define ART_IDX_MELEE_DICE_SUPER 70
#define ART_IDX_BOW_SHOTS_SUPER 71
#define ART_IDX_BOW_MIGHT_SUPER 72
#define ART_IDX_GEN_SPEED_SUPER 73
#define ART_IDX_MELEE_BLOWS_SUPER 77
#define ART_IDX_GEN_AC_SUPER 85

/* Aggravation - weapon and nonweapon */
#define ART_IDX_WEAPON_AGGR 74
#define ART_IDX_NONWEAPON_AGGR 75

/* Total of abilities */
#define ART_IDX_TOTAL 86

/* Tallies of different ability types */
/* ToDo: use N_ELEMENTS for these */
#define ART_IDX_BOW_COUNT 4
#define ART_IDX_WEAPON_COUNT 3
#define ART_IDX_NONWEAPON_COUNT 8
#define ART_IDX_MELEE_COUNT 9
#define ART_IDX_ALLARMOR_COUNT 1
#define ART_IDX_BOOT_COUNT 4
#define ART_IDX_GLOVE_COUNT 3
#define ART_IDX_HELM_COUNT 6
#define ART_IDX_SHIELD_COUNT 2
#define ART_IDX_CLOAK_COUNT 2
#define ART_IDX_ARMOR_COUNT 7
#define ART_IDX_GEN_COUNT 31
#define ART_IDX_HIGH_RESIST_COUNT 12

/* Arrays of indices by item type, used in frequency generation */
static s16b art_idx_bow[] =
	{ART_IDX_BOW_SHOTS, ART_IDX_BOW_MIGHT, ART_IDX_BOW_BRAND, ART_IDX_BOW_SLAY};
static s16b art_idx_weapon[] =
	{ART_IDX_WEAPON_HIT, ART_IDX_WEAPON_DAM, ART_IDX_WEAPON_AGGR};
static s16b art_idx_nonweapon[] =
	{ART_IDX_NONWEAPON_HIT, ART_IDX_NONWEAPON_DAM, ART_IDX_NONWEAPON_HIT_DAM,
	ART_IDX_NONWEAPON_AGGR, ART_IDX_NONWEAPON_BRAND, ART_IDX_NONWEAPON_SLAY,
	ART_IDX_NONWEAPON_BLOWS, ART_IDX_NONWEAPON_SHOTS};
static s16b art_idx_melee[] =
	{ART_IDX_MELEE_BLESS, ART_IDX_MELEE_SINV, ART_IDX_MELEE_BRAND, ART_IDX_MELEE_SLAY,
	ART_IDX_MELEE_BLOWS, ART_IDX_MELEE_AC, ART_IDX_MELEE_DICE,
	ART_IDX_MELEE_WEIGHT, ART_IDX_MELEE_TUNN};
static s16b art_idx_allarmor[] =
	{ART_IDX_ALLARMOR_WEIGHT};
static s16b art_idx_boot[] =
	{ART_IDX_BOOT_AC, ART_IDX_BOOT_FEATHER, ART_IDX_BOOT_STEALTH, ART_IDX_BOOT_SPEED};
static s16b art_idx_glove[] =
	{ART_IDX_GLOVE_AC, ART_IDX_GLOVE_FA, ART_IDX_GLOVE_DEX};
static s16b art_idx_headgear[] =
	{ART_IDX_HELM_AC, ART_IDX_HELM_RBLIND, ART_IDX_HELM_ESP, ART_IDX_HELM_SINV,
	ART_IDX_HELM_WIS, ART_IDX_HELM_INT};
static s16b art_idx_shield[] =
	{ART_IDX_SHIELD_AC, ART_IDX_SHIELD_LRES};
static s16b art_idx_cloak[] =
	{ART_IDX_CLOAK_AC, ART_IDX_CLOAK_STEALTH};
static s16b art_idx_armor[] =
	{ART_IDX_ARMOR_AC, ART_IDX_ARMOR_STEALTH, ART_IDX_ARMOR_HLIFE, ART_IDX_ARMOR_CON,
	ART_IDX_ARMOR_LRES, ART_IDX_ARMOR_ALLRES, ART_IDX_ARMOR_HRES};
static s16b art_idx_gen[] =
	{ART_IDX_GEN_STAT, ART_IDX_GEN_SUST, ART_IDX_GEN_STEALTH,
	ART_IDX_GEN_SEARCH, ART_IDX_GEN_INFRA, ART_IDX_GEN_SPEED,
	ART_IDX_GEN_IMMUNE, ART_IDX_GEN_FA, ART_IDX_GEN_HLIFE,
	ART_IDX_GEN_FEATHER, ART_IDX_GEN_LIGHT, ART_IDX_GEN_SINV,
	ART_IDX_GEN_ESP, ART_IDX_GEN_SDIG, ART_IDX_GEN_REGEN,
	ART_IDX_GEN_LRES, ART_IDX_GEN_RPOIS, ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLIGHT, ART_IDX_GEN_RDARK, ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF, ART_IDX_GEN_RSOUND, ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS, ART_IDX_GEN_RNETHER, ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN, ART_IDX_GEN_AC, ART_IDX_GEN_TUNN,
	ART_IDX_GEN_ACTIV};
static s16b art_idx_high_resist[] =
	{ART_IDX_GEN_RPOIS, ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLIGHT, ART_IDX_GEN_RDARK, ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF, ART_IDX_GEN_RSOUND, ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS, ART_IDX_GEN_RNETHER, ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN};

/* Initialize the data structures for learned probabilities */
static s16b artprobs[ART_IDX_TOTAL];
s16b *baseprobs;
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
s16b *base_freq; 			/* base items */

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

/* Global just for convenience. */
static int verbose = 1;

char *artifact_gen_name(struct artifact *a, const char ***words) {
	char buf[BUFLEN];
	char word[MAX_NAME_LEN + 1];
	randname_make(RANDNAME_TOLKIEN, MIN_NAME_LEN, MAX_NAME_LEN, word, sizeof(word), words);
	word[0] = toupper((unsigned char)word[0]);
	if (one_in_(3))
		strnfmt(buf, sizeof(buf), "'%s'", word);
	else
		strnfmt(buf, sizeof(buf), "of %s", word);
	if (a->aidx == ART_POWER)
		strnfmt(buf, sizeof(buf), "of Power (The One Ring)");
	if (a->aidx == ART_GROND)
		strnfmt(buf, sizeof(buf), "'Grond'");
	if (a->aidx == ART_MORGOTH)
		strnfmt(buf, sizeof(buf), "of Morgoth");
	return string_make(buf);
}

/*
 * Use W. Sheldon Simms' random name generator.
 */
static errr init_names(void)
{
	int i;
	struct artifact *a;

	for (i = 0; i < z_info->a_max; i++)
	{
		char desc[128] = "Based on ";

		a = &a_info[i];
		if (!a->tval || !a->sval || !a->name) continue;

		if (prefix(a->name, "of Power"))
		{
			my_strcat(desc, a->name + 10, 
				strlen(a->name) - 1);
		}
		else if (prefix(a->name, "of "))
		{
			my_strcat(desc, a->name + 3, 
				strlen(a->name) + 7);
		}
		else
		{
			my_strcat(desc, a->name + 1, 
				strlen(a->name) + 8);
		}

		a->text = string_make(desc);
		a->name = artifact_gen_name(a, name_sections);
	}

	return 0;
}

/*
 * Return the artifact power, by generating a "fake" object based on the
 * artifact, and calling the common object_power function
 */
static s32b artifact_power(int a_idx)
{
	object_type obj;
	char buf[256];

	file_putf(log_file, "********** ENTERING EVAL POWER ********\n");
	file_putf(log_file, "Artifact index is %d\n", a_idx);

	if (!make_fake_artifact(&obj, &a_info[a_idx]))
		return 0;

	object_desc(buf, 256*sizeof(char), &obj, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);
	file_putf(log_file, "%s\n", buf);

	return object_power(&obj, verbose, log_file, TRUE);
}


/*
 * Store the original artifact power ratings as a baseline
 */
static void store_base_power (void)
{
	int i, j;
	artifact_type *a_ptr;
	object_kind *k_ptr;
	int *fake_power;

	max_power = 0;
	min_power = 32767;
	var_power = 0;
	fake_power = C_ZNEW(z_info->a_max, int);
	j = 0;

	for(i = 0; i < z_info->a_max; i++, j++)
	{
		base_power[i] = artifact_power(i);

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
		a_ptr = &a_info[i];
		k_ptr = lookup_kind(a_ptr->tval, a_ptr->sval);
		base_item_level[i] = k_ptr->level;
		base_item_prob[i] = k_ptr->alloc_prob;
		base_art_alloc[i] = a_ptr->alloc_prob;
	}

	avg_power = mean(fake_power, j);
	var_power = variance(fake_power, j);

	file_putf(log_file, "Max power is %d, min is %d\n", max_power, min_power);
	file_putf(log_file, "Mean is %d, variance is %d\n", avg_power, var_power);

	/* Store the number of different types, for use later */
	/* ToDo: replace this with full combination tracking */
	for (i = 0; i < z_info->a_max; i++)
	{
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
}


/*
 * Randomly select a base item type (tval,sval).  Assign the various fields
 * corresponding to that choice.
 *
 * The return value gives the index of the new item type.  The method is
 * passed a pointer to a rarity value in order to return the rarity of the
 * new item.
 */
static object_kind *choose_item(int a_idx)
{
	artifact_type *a_ptr = &a_info[a_idx];
	int tval = 0, sval = 0, i = 0;
	object_kind *k_ptr;
	s16b r;
	bitflag f[OF_SIZE];

	/*
	 * Pick a base item from the cumulative frequency table.
	 *
	 * Although this looks hideous, it provides for easy addition of
	 * future artifact types, simply by removing the tvals from this
	 * loop.
	 *
	 * N.B. Could easily generate lights, rings and amulets this way if
	 * the whole special/flavour issue was sorted out (see ticket #1014)
	 * Note that Carlammas and Barahir have the same sval as Grond/Morgoth
	 */
	while (tval == 0 || tval == TV_SKELETON || tval == TV_BOTTLE ||
		tval == TV_JUNK || tval == TV_SPIKE || tval == TV_CHEST ||
		tval == TV_SHOT || tval == TV_ARROW || tval == TV_BOLT ||
		tval == TV_STAFF || tval == TV_WAND || tval == TV_ROD ||
		tval == TV_SCROLL || tval == TV_POTION || tval == TV_FLASK ||
		tval == TV_FOOD || tval == TV_MAGIC_BOOK || tval ==
		TV_PRAYER_BOOK || tval == TV_GOLD || tval == TV_LIGHT ||
		tval == TV_AMULET || tval == TV_RING || sval == SV_GROND ||
		sval == SV_MORGOTH || k_info[i].alloc_prob == 0)
	{
		r = randint1(base_freq[z_info->k_max - 1]);
		i = 0;
		while (r > base_freq[i])
		{
			i++;
		}
		tval = k_info[i].tval;
		sval = k_info[i].sval;
	}
	file_putf(log_file, "Creating tval %d sval %d\n", tval, sval);
	k_ptr = lookup_kind(tval, sval);
	a_ptr->tval = k_ptr->tval;
	a_ptr->sval = k_ptr->sval;
	a_ptr->pval[DEFAULT_PVAL] = randcalc(k_ptr->pval[DEFAULT_PVAL], 0, MINIMISE);
	a_ptr->to_h = randcalc(k_ptr->to_h, 0, MINIMISE);
	a_ptr->to_d = randcalc(k_ptr->to_d, 0, MINIMISE);
	a_ptr->to_a = randcalc(k_ptr->to_a, 0, MINIMISE);
	a_ptr->ac = k_ptr->ac;
	a_ptr->dd = k_ptr->dd;
	a_ptr->ds = k_ptr->ds;
	a_ptr->weight = k_ptr->weight;
	of_copy(a_ptr->flags, k_ptr->flags);
	for (i = 0; i < MAX_PVALS; i++)
		of_copy(a_ptr->pval_flags[i], k_ptr->pval_flags[i]);
	a_ptr->num_pvals = k_ptr->num_pvals;
	a_ptr->effect = 0;

	/* Artifacts ignore everything */
	create_mask(f, FALSE, OFT_IGNORE, OFT_MAX);
	of_union(a_ptr->flags, f);

	/* Assign basic stats to the artifact based on its artifact level. */
	/*
	 * CR, 2001-09-03: changed to a simpler version to match the hit-dam
	 * parsing algorithm. We use random ranges averaging mean_hit_startval
	 * and mean_dam_startval, but permitting variation of 50% to 150%.
	 * Level-dependent term has been removed for the moment.
	 */
	switch (a_ptr->tval)
	{
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_SWORD:
		case TV_POLEARM:
			a_ptr->to_h += (s16b)(mean_hit_startval / 2 +
			                      randint0(mean_hit_startval) );
			a_ptr->to_d += (s16b)(mean_dam_startval / 2 +
			                      randint0(mean_dam_startval) );
			file_putf(log_file, "Assigned basic stats, to_hit: %d, to_dam: %d\n", a_ptr->to_h, a_ptr->to_d);
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
			a_ptr->to_a += (s16b)(mean_ac_startval / 2 +
			                      randint0(mean_ac_startval) );
			file_putf(log_file, "Assigned basic stats, AC bonus: %d\n", a_ptr->to_a);
			break;
	}

	/* Done - return the index of the new object kind. */
	return k_ptr;
}


/*
 * We've just added an ability which uses the pval bonus.  Make sure it's
 * not zero.  If it's currently negative, leave it negative (heh heh).
 */
static void do_pval(artifact_type *a_ptr)
{
	int factor = 1;

	/* Track whether we have blows, might or shots on this item */
	if (of_has(a_ptr->flags, OF_BLOWS)) factor++;
	if (of_has(a_ptr->flags, OF_MIGHT)) factor++;
	if (of_has(a_ptr->flags, OF_SHOTS)) factor++;

	if (a_ptr->pval[DEFAULT_PVAL] == 0)
	{
		/* Blows, might, shots handled separately */
		if (factor > 1)
		{
			a_ptr->pval[DEFAULT_PVAL] = (s16b)randint1(2);
			/* Give it a shot at +3 */
			if (INHIBIT_STRONG) a_ptr->pval[DEFAULT_PVAL] = 3;
		}
		else a_ptr->pval[DEFAULT_PVAL] = (s16b)randint1(4);
		file_putf(log_file, "Assigned initial pval, value is: %d\n", a_ptr->pval[DEFAULT_PVAL]);
	}
	else if (a_ptr->pval[DEFAULT_PVAL] < 0)
	{
		if (one_in_(2))
		{
			a_ptr->pval[DEFAULT_PVAL]--;
			file_putf(log_file, "Decreasing pval by 1, new value is: %d\n", a_ptr->pval[DEFAULT_PVAL]);
		}
	}
	else if (one_in_(a_ptr->pval[DEFAULT_PVAL] * factor))
	{
		/*
		 * CR: made this a bit rarer and diminishing with higher pval -
		 * also rarer if item has blows/might/shots already
		 */
		a_ptr->pval[DEFAULT_PVAL]++;
		file_putf(log_file, "Increasing pval by 1, new value is: %d\n", a_ptr->pval[DEFAULT_PVAL]);
	}
}


static void remove_contradictory(artifact_type *a_ptr)
{
	if (of_has(a_ptr->flags, OF_AGGRAVATE)) of_off(a_ptr->flags, OF_STEALTH);
	if (of_has(a_ptr->flags, OF_IM_ACID)) of_off(a_ptr->flags, OF_RES_ACID);
	if (of_has(a_ptr->flags, OF_IM_ELEC)) of_off(a_ptr->flags, OF_RES_ELEC);
	if (of_has(a_ptr->flags, OF_IM_FIRE)) of_off(a_ptr->flags, OF_RES_FIRE);
	if (of_has(a_ptr->flags, OF_IM_COLD)) of_off(a_ptr->flags, OF_RES_COLD);

	if (a_ptr->pval[DEFAULT_PVAL] < 0)
	{
		if (of_has(a_ptr->flags, OF_STR)) of_off(a_ptr->flags, OF_SUST_STR);
		if (of_has(a_ptr->flags, OF_INT)) of_off(a_ptr->flags, OF_SUST_INT);
		if (of_has(a_ptr->flags, OF_WIS)) of_off(a_ptr->flags, OF_SUST_WIS);
		if (of_has(a_ptr->flags, OF_DEX)) of_off(a_ptr->flags, OF_SUST_DEX);
		if (of_has(a_ptr->flags, OF_CON)) of_off(a_ptr->flags, OF_SUST_CON);
		if (of_has(a_ptr->flags, OF_CHR)) of_off(a_ptr->flags, OF_SUST_CHR);
		of_off(a_ptr->flags, OF_BLOWS);
	}

	if (of_has(a_ptr->flags, OF_LIGHT_CURSE)) of_off(a_ptr->flags, OF_BLESSED);
	if (of_has(a_ptr->flags, OF_KILL_DRAGON)) of_off(a_ptr->flags, OF_SLAY_DRAGON);
	if (of_has(a_ptr->flags, OF_KILL_DEMON)) of_off(a_ptr->flags, OF_SLAY_DEMON);
	if (of_has(a_ptr->flags, OF_KILL_UNDEAD)) of_off(a_ptr->flags, OF_SLAY_UNDEAD);
	if (of_has(a_ptr->flags, OF_DRAIN_EXP)) of_off(a_ptr->flags, OF_HOLD_LIFE);
}

/*
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

	/* Cut aggravation frequencies in half since they're used twice */
	artprobs[ART_IDX_NONWEAPON_AGGR] /= 2;
	artprobs[ART_IDX_WEAPON_AGGR] /= 2;
}

/*
 * Parse the list of artifacts and count up the frequencies of the various
 * abilities.  This is used to give dynamic generation probabilities.
 */
static void parse_frequencies(void)
{
	int i, j;
	const artifact_type *a_ptr;
	object_kind *k_ptr;
	s32b m, temp, temp2;
	bitflag mask[OF_SIZE];

	file_putf(log_file, "\n****** BEGINNING GENERATION OF FREQUENCIES\n\n");

	/* Zero the frequencies for artifact attributes */
	for (i = 0; i < ART_IDX_TOTAL; i++)
	{
		artprobs[i] = 0;
	}

	/*
	 * Initialise the frequencies for base items so that each item could
	 * be chosen - we check for illegal items during choose_item()
	 */
	for (i = 0; i < z_info->k_max; i++)
	{
		baseprobs[i] = 1;
	}

	/* Go through the list of all artifacts */
	for (i = 0; i < z_info->a_max; i++)
	{
		file_putf(log_file, "Current artifact index is %d\n", i);

		a_ptr = &a_info[i];

		/* Special cases -- don't parse these! */
		if ((i == ART_POWER) ||
			(i == ART_GROND) ||
			(i == ART_MORGOTH))
			continue;

		/* Also don't parse cursed or null items */
		if (base_power[i] < 0 || a_ptr->tval == 0) continue;

		/* Get a pointer to the base item for this artifact */
		k_ptr = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Add the base item to the baseprobs array */
		baseprobs[k_ptr->kidx]++;
		file_putf(log_file, "Base item is %d\n", k_ptr->kidx);

		/* Count up the abilities for this artifact */
		if (a_ptr->tval == TV_BOW)
		{
			if(of_has(a_ptr->flags, OF_SHOTS))
			{
				/* Do we have 3 or more extra shots? (Unlikely) */
				if(a_ptr->pval[DEFAULT_PVAL] > 2)
				{
					file_putf(log_file, "Adding 1 for supercharged shots (3 or more!)\n");

					(artprobs[ART_IDX_BOW_SHOTS_SUPER])++;
				}
				else {
					file_putf(log_file, "Adding 1 for extra shots\n");

					(artprobs[ART_IDX_BOW_SHOTS])++;
				}
			}
			if(of_has(a_ptr->flags, OF_MIGHT))
			{
				/* Do we have 3 or more extra might? (Unlikely) */
				if(a_ptr->pval[DEFAULT_PVAL] > 2)
				{
					file_putf(log_file, "Adding 1 for supercharged might (3 or more!)\n");

					(artprobs[ART_IDX_BOW_MIGHT_SUPER])++;
				}
				else {
					file_putf(log_file, "Adding 1 for extra might\n");

					(artprobs[ART_IDX_BOW_MIGHT])++;
				}
			}

			/* Brands or slays - count all together */
			create_mask(mask, FALSE, OFT_SLAY, OFT_BRAND, OFT_KILL, OFT_MAX);
			if (of_is_inter(a_ptr->flags, mask))
			{
				/* We have some brands or slays - count them */
				temp = list_slays(a_ptr->flags, mask, NULL, NULL, NULL,	FALSE);
				create_mask(mask, FALSE, OFT_BRAND, OFT_MAX);
				temp2 = list_slays(a_ptr->flags, mask, NULL, NULL, NULL, FALSE);

				file_putf(log_file, "Adding %d for slays\n", temp - temp2);
				file_putf(log_file, "Adding %d for brands\n", temp2);

				/* Add these to the frequency count */
				artprobs[ART_IDX_BOW_SLAY] += temp;
				artprobs[ART_IDX_BOW_BRAND] += temp2;
			}
		}

		/* Handle hit / dam ratings - are they higher than normal? */
		/* Also handle other weapon/nonweapon abilities */
		if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
			a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
			a_ptr->tval == TV_SWORD)
		{

			m = randcalc(k_ptr->to_h, 0, MINIMISE);
			temp = (a_ptr->to_h - m - mean_hit_startval) / mean_hit_increment;
			if (temp > 0)
				file_putf(log_file, "Adding %d instances of extra to-hit bonus for weapon\n", temp);
			else if (temp < 0)
				file_putf(log_file, "Subtracting %d instances of extra to-hit bonus for weapon\n", temp);
			
			artprobs[ART_IDX_WEAPON_HIT] += temp;

			m = randcalc(k_ptr->to_d, 0, MINIMISE);
			temp = (a_ptr->to_d - m - mean_dam_startval) / mean_dam_increment;
			if (temp > 0)
				file_putf(log_file, "Adding %d instances of extra to-dam bonus for weapon\n", temp);
			else
				file_putf(log_file, "Subtracting %d instances of extra to-dam bonus for weapon\n", temp);

			artprobs[ART_IDX_WEAPON_DAM] += temp;

			/* Aggravation */
			if (of_has(a_ptr->flags, OF_AGGRAVATE))
			{
				file_putf(log_file, "Adding 1 for aggravation - weapon\n");
				artprobs[ART_IDX_WEAPON_AGGR]++;
			}

			/* End weapon stuff */
		}
		else
		{
			if ( (a_ptr->to_h - randcalc(k_ptr->to_h, 0, MINIMISE) > 0) &&
				(a_ptr->to_h - randcalc(k_ptr->to_h, 0, MINIMISE) == a_ptr->to_d - randcalc(k_ptr->to_d, 0, MINIMISE)) )
			{
				/* Special case: both hit and dam bonuses present and equal */
				temp = (a_ptr->to_d - randcalc(k_ptr->to_d, 0, MINIMISE)) / mean_dam_increment;
				if (temp > 0)
				{
					file_putf(log_file, "Adding %d instances of extra to-hit and to-dam bonus for non-weapon\n", temp);

					(artprobs[ART_IDX_NONWEAPON_HIT_DAM]) += temp;
				}
			}
			else
			{
				/* Uneven bonuses - handle separately */
				if (a_ptr->to_h - randcalc(k_ptr->to_h, 0, MINIMISE) > 0)
				{
					temp = (a_ptr->to_d - randcalc(k_ptr->to_d, 0, MINIMISE)) / mean_dam_increment;
					if (temp > 0)
					{
						file_putf(log_file, "Adding %d instances of extra to-hit bonus for non-weapon\n", temp);

						(artprobs[ART_IDX_NONWEAPON_HIT]) += temp;
					}
				}
				if (a_ptr->to_d - randcalc(k_ptr->to_d, 0, MINIMISE) > 0)
				{
					temp = (a_ptr->to_d - randcalc(k_ptr->to_d, 0, MINIMISE)) / mean_dam_increment;
					if (temp > 0)
					{
						file_putf(log_file, "Adding %d instances of extra to-dam bonus for non-weapon\n", temp);

						(artprobs[ART_IDX_NONWEAPON_DAM]) += temp;
					}
				}
			}

			/* Aggravation */
			if (of_has(a_ptr->flags, OF_AGGRAVATE))
			{
				file_putf(log_file, "Adding 1 for aggravation - nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_AGGR])++;
			}

			/* Brands or slays - count all together */
			create_mask(mask, FALSE, OFT_SLAY, OFT_BRAND, OFT_KILL, OFT_MAX);
			if (of_is_inter(a_ptr->flags, mask))
			{
				/* We have some brands or slays - count them */
				temp = list_slays(a_ptr->flags, mask, NULL, NULL, NULL,	FALSE);
				create_mask(mask, FALSE, OFT_BRAND, OFT_MAX);
				temp2 = list_slays(a_ptr->flags, mask, NULL, NULL, NULL, FALSE);

				file_putf(log_file, "Adding %d for slays\n", temp - temp2);
				file_putf(log_file, "Adding %d for brands\n", temp2);

				/* Add these to the frequency count */
				artprobs[ART_IDX_NONWEAPON_SLAY] += temp;
				artprobs[ART_IDX_NONWEAPON_BRAND] += temp2;
			}

			if (of_has(a_ptr->flags, OF_BLOWS))
			{
				file_putf(log_file, "Adding 1 for extra blows on nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_BLOWS])++;
			}

			if (of_has(a_ptr->flags, OF_SHOTS))
			{
				file_putf(log_file, "Adding 1 for extra shots on nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_SHOTS])++;
			}
		}

		if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
			a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
		{
			/* Blessed weapon? */
			if (of_has(a_ptr->flags, OF_BLESSED))
			{
				file_putf(log_file, "Adding 1 for blessed weapon\n");

				(artprobs[ART_IDX_MELEE_BLESS])++;
			}

			/* See invisible? */
			if (of_has(a_ptr->flags, OF_SEE_INVIS))
			{
				file_putf(log_file, "Adding 1 for see invisible (weapon case)\n");

				(artprobs[ART_IDX_MELEE_SINV])++;
			}

			/* Does this weapon have extra blows? */
			if (of_has(a_ptr->flags, OF_BLOWS))
			{
				/* Do we have 3 or more extra blows? (Unlikely) */
				if(a_ptr->pval[DEFAULT_PVAL] > 2)
				{
					file_putf(log_file, "Adding 1 for supercharged blows (3 or more!)\n");
					(artprobs[ART_IDX_MELEE_BLOWS_SUPER])++;
				}
				else {
					file_putf(log_file, "Adding 1 for extra blows\n");
					(artprobs[ART_IDX_MELEE_BLOWS])++;
				}
			}

			/* Does this weapon have an unusual bonus to AC? */
			if ( (a_ptr->to_a - randcalc(k_ptr->to_a, 0, MAXIMISE)) > 0)
			{
				temp = (a_ptr->to_a - randcalc(k_ptr->to_a, 0, MAXIMISE)) / mean_ac_increment;
				if (temp > 0)
				{
					file_putf(log_file, "Adding %d instances of extra AC bonus for weapon\n", temp);

					(artprobs[ART_IDX_MELEE_AC]) += temp;
				}
			}

			/* Check damage dice - are they more than normal? */
			if (a_ptr->dd > k_ptr->dd)
			{
				/* Difference of 3 or more? */
				if ( (a_ptr->dd - k_ptr->dd) > 2)
				{
					file_putf(log_file, "Adding 1 for super-charged damage dice!\n");

					(artprobs[ART_IDX_MELEE_DICE_SUPER])++;
				}
				else
				{
					file_putf(log_file, "Adding 1 for extra damage dice.\n");

					(artprobs[ART_IDX_MELEE_DICE])++;
				}
			}

			/* Check weight - is it different from normal? */
			if (a_ptr->weight != k_ptr->weight)
			{
				file_putf(log_file, "Adding 1 for unusual weight.\n");

				(artprobs[ART_IDX_MELEE_WEIGHT])++;
			}

			/* Check for tunnelling ability */
			if (of_has(a_ptr->flags, OF_TUNNEL))
			{
				file_putf(log_file, "Adding 1 for tunnelling bonus.\n");

				(artprobs[ART_IDX_MELEE_TUNN])++;
			}

			/* Brands or slays - count all together */
			create_mask(mask, FALSE, OFT_SLAY, OFT_BRAND, OFT_KILL, OFT_MAX);
			if (of_is_inter(a_ptr->flags, mask))
			{
				/* We have some brands or slays - count them */
				temp = list_slays(a_ptr->flags, mask, NULL, NULL, NULL,	FALSE);
				create_mask(mask, FALSE, OFT_BRAND, OFT_MAX);
				temp2 = list_slays(a_ptr->flags, mask, NULL, NULL, NULL, FALSE);

				file_putf(log_file, "Adding %d for slays\n", temp - temp2);
				file_putf(log_file, "Adding %d for brands\n", temp2);

				/* Add these to the frequency count */
				artprobs[ART_IDX_MELEE_SLAY] += temp;
				artprobs[ART_IDX_MELEE_BRAND] += temp2;
			}

			/* End of weapon-specific stuff */
		}
		else
		{
			/* Check for tunnelling ability */
			if (of_has(a_ptr->flags, OF_TUNNEL))
			{
				file_putf(log_file, "Adding 1 for tunnelling bonus - general.\n");

				(artprobs[ART_IDX_GEN_TUNN])++;
			}
		}

		/*
		 * Count up extra AC bonus values.
		 * Could also add logic to subtract for lower values here, but it's
		 * probably not worth the trouble since it's so rare.
		 */

		if ( (a_ptr->to_a - randcalc(k_ptr->to_a, 0, MINIMISE) - mean_ac_startval) > 0)
		{
			temp = (a_ptr->to_a - randcalc(k_ptr->to_a, 0, MINIMISE) - mean_ac_startval) /
				mean_ac_increment;
			if (temp > 0)
			{
				if (a_ptr->to_a > 20)
				{
					file_putf(log_file, "Adding %d for supercharged AC\n", temp);
					(artprobs[ART_IDX_GEN_AC_SUPER])++;
				}
				else if (a_ptr->tval == TV_BOOTS)
				{
					file_putf(log_file, "Adding %d for AC bonus - boots\n", temp);
					(artprobs[ART_IDX_BOOT_AC]) += temp;
				}
				else if (a_ptr->tval == TV_GLOVES)
				{
					file_putf(log_file, "Adding %d for AC bonus - gloves\n", temp);
					(artprobs[ART_IDX_GLOVE_AC]) += temp;
				}
				else if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
				{
					file_putf(log_file, "Adding %d for AC bonus - headgear\n", temp);
					(artprobs[ART_IDX_HELM_AC]) += temp;
				}
				else if (a_ptr->tval == TV_SHIELD)
				{
					file_putf(log_file, "Adding %d for AC bonus - shield\n", temp);
					(artprobs[ART_IDX_SHIELD_AC]) += temp;
				}
				else if (a_ptr->tval == TV_CLOAK)
				{
					file_putf(log_file, "Adding %d for AC bonus - cloak\n", temp);
					(artprobs[ART_IDX_CLOAK_AC]) += temp;
				}
				else if (a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
					a_ptr->tval == TV_DRAG_ARMOR)
				{
					file_putf(log_file, "Adding %d for AC bonus - body armor\n", temp);
					(artprobs[ART_IDX_ARMOR_AC]) += temp;
				}
				else
				{
					file_putf(log_file, "Adding %d for AC bonus - general\n", temp);
					(artprobs[ART_IDX_GEN_AC]) += temp;
				}
			}
		}

		/* Generic armor abilities */
		if (a_ptr->tval == TV_BOOTS || a_ptr->tval == TV_GLOVES ||
			a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN ||
			a_ptr->tval == TV_SHIELD || a_ptr->tval == TV_CLOAK ||
			a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
			a_ptr->tval == TV_DRAG_ARMOR)
		{
			/* Check weight - is it different from normal? */
			/* ToDo: count higher and lower separately */
			if (a_ptr->weight != k_ptr->weight)
			{
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

		if (flags_test(a_ptr->flags, OF_SIZE, OF_STR, OF_INT, OF_WIS,
		                     OF_DEX, OF_CON, OF_CHR, FLAG_END))
		{
			/* Stat bonus case.  Add up the number of individual
			   bonuses */
			temp = 0;
			if (of_has(a_ptr->flags, OF_STR)) temp++;
			if (of_has(a_ptr->flags, OF_INT)) temp++;
			if (of_has(a_ptr->flags, OF_WIS)) temp++;
			if (of_has(a_ptr->flags, OF_DEX)) temp++;
			if (of_has(a_ptr->flags, OF_CON)) temp++;
			if (of_has(a_ptr->flags, OF_CHR)) temp++;

			/* Handle a few special cases separately. */
			if((a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN) &&
				(of_has(a_ptr->flags, OF_WIS) || of_has(a_ptr->flags, OF_INT)))
			{
				/* Handle WIS and INT on helms and crowns */
				if(of_has(a_ptr->flags, OF_WIS))
				{
					file_putf(log_file, "Adding 1 for WIS bonus on headgear.\n");

					(artprobs[ART_IDX_HELM_WIS])++;
					/* Counted this one separately so subtract it here */
					temp--;
				}
				if(of_has(a_ptr->flags, OF_INT))
				{
					file_putf(log_file, "Adding 1 for INT bonus on headgear.\n");

					(artprobs[ART_IDX_HELM_INT])++;
					/* Counted this one separately so subtract it here */
					temp--;
				}
			}
			else if ((a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR ||
				a_ptr->tval == TV_DRAG_ARMOR) && of_has(a_ptr->flags, OF_CON))
			{
				/* Handle CON bonus on armor */
				file_putf(log_file, "Adding 1 for CON bonus on body armor.\n");

				(artprobs[ART_IDX_ARMOR_CON])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}
			else if (a_ptr->tval == TV_GLOVES && of_has(a_ptr->flags, OF_DEX))
			{
				/* Handle DEX bonus on gloves */
				file_putf(log_file, "Adding 1 for DEX bonus on gloves.\n");

				(artprobs[ART_IDX_GLOVE_DEX])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}

			/* Now the general case */
			if (temp > 0)
			{
				/* There are some bonuses that weren't handled above */
				file_putf(log_file, "Adding %d for stat bonuses - general.\n", temp);

				(artprobs[ART_IDX_GEN_STAT]) += temp;

			/* Done with stat bonuses */
			}
		}

		if (flags_test(a_ptr->flags, OF_SIZE, OF_SUST_STR, OF_SUST_INT,
		                     OF_SUST_WIS, OF_SUST_DEX, OF_SUST_CON,
		                     OF_SUST_CHR, FLAG_END))
		{
			/* Now do sustains, in a similar manner */
			temp = 0;
			if (of_has(a_ptr->flags, OF_SUST_STR)) temp++;
			if (of_has(a_ptr->flags, OF_SUST_INT)) temp++;
			if (of_has(a_ptr->flags, OF_SUST_WIS)) temp++;
			if (of_has(a_ptr->flags, OF_SUST_DEX)) temp++;
			if (of_has(a_ptr->flags, OF_SUST_CON)) temp++;
			if (of_has(a_ptr->flags, OF_SUST_CHR)) temp++;
			file_putf(log_file, "Adding %d for stat sustains.\n", temp);

			(artprobs[ART_IDX_GEN_SUST]) += temp;
		}

		if (of_has(a_ptr->flags, OF_STEALTH))
		{
			/* Handle stealth, including a couple of special cases */
			if(a_ptr->tval == TV_BOOTS)
			{
				file_putf(log_file, "Adding 1 for stealth bonus on boots.\n");

				(artprobs[ART_IDX_BOOT_STEALTH])++;
			}
			else if (a_ptr->tval == TV_CLOAK)
			{
				file_putf(log_file, "Adding 1 for stealth bonus on cloak.\n");

				(artprobs[ART_IDX_CLOAK_STEALTH])++;
			}
			else if (a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
			{
				file_putf(log_file, "Adding 1 for stealth bonus on armor.\n");

				(artprobs[ART_IDX_ARMOR_STEALTH])++;
			}
			else
			{
				/* General case */
				file_putf(log_file, "Adding 1 for stealth bonus - general.\n");

				(artprobs[ART_IDX_GEN_STEALTH])++;
			}
			/* Done with stealth */
		}

		if (of_has(a_ptr->flags, OF_SEARCH))
		{
			/* Handle searching bonus - fully generic this time */
			file_putf(log_file, "Adding 1 for search bonus - general.\n");

			(artprobs[ART_IDX_GEN_SEARCH])++;
		}

		if (of_has(a_ptr->flags, OF_INFRA))
		{
			/* Handle infravision bonus - fully generic */
			file_putf(log_file, "Adding 1 for infravision bonus - general.\n");

			(artprobs[ART_IDX_GEN_INFRA])++;
		}

		if (of_has(a_ptr->flags, OF_SPEED))
		{
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

			if (a_ptr->pval[DEFAULT_PVAL] > 7)
			{
				/* Supercharge case */
				file_putf(log_file, "Adding 1 for supercharged speed bonus!\n");

				(artprobs[ART_IDX_GEN_SPEED_SUPER])++;
			}
			else if(a_ptr->tval == TV_BOOTS)
			{
				/* Handle boots separately */
				file_putf(log_file, "Adding 1 for normal speed bonus on boots.\n");

				(artprobs[ART_IDX_BOOT_SPEED])++;
			}
			else
			{
				file_putf(log_file, "Adding 1 for normal speed bonus - general.\n");

				(artprobs[ART_IDX_GEN_SPEED])++;
			}
			/* Done with speed */
		}

		if (flags_test(a_ptr->flags, OF_SIZE, OF_IM_ACID, OF_IM_ELEC,
		                     OF_IM_FIRE, OF_IM_COLD, FLAG_END))
		{
			/* Count up immunities for this item, if any */
			temp = 0;
			if (of_has(a_ptr->flags, OF_IM_ACID)) temp++;
			if (of_has(a_ptr->flags, OF_IM_ELEC)) temp++;
			if (of_has(a_ptr->flags, OF_IM_FIRE)) temp++;
			if (of_has(a_ptr->flags, OF_IM_COLD)) temp++;
			file_putf(log_file, "Adding %d for immunities.\n", temp);

			(artprobs[ART_IDX_GEN_IMMUNE]) += temp;
		}

		if (of_has(a_ptr->flags, OF_FREE_ACT))
		{
			/* Free action - handle gloves separately */
			if(a_ptr->tval == TV_GLOVES)
			{
				file_putf(log_file, "Adding 1 for free action on gloves.\n");

				(artprobs[ART_IDX_GLOVE_FA])++;
			}
			else
			{
				file_putf(log_file, "Adding 1 for free action - general.\n");

				(artprobs[ART_IDX_GEN_FA])++;
			}
		}

		if (of_has(a_ptr->flags, OF_HOLD_LIFE))
		{
			/* Hold life - do body armor separately */
			if( (a_ptr->tval == TV_SOFT_ARMOR) || (a_ptr->tval == TV_HARD_ARMOR) ||
				(a_ptr->tval == TV_DRAG_ARMOR))
			{
				file_putf(log_file, "Adding 1 for hold life on armor.\n");

				(artprobs[ART_IDX_ARMOR_HLIFE])++;
			}
			else
			{
				file_putf(log_file, "Adding 1 for hold life - general.\n");

				(artprobs[ART_IDX_GEN_HLIFE])++;
			}
		}

		if (of_has(a_ptr->flags, OF_FEATHER))
		{
			/* Feather fall - handle boots separately */
			if(a_ptr->tval == TV_BOOTS)
			{
				file_putf(log_file, "Adding 1 for feather fall on boots.\n");

				(artprobs[ART_IDX_BOOT_FEATHER])++;
			}
			else
			{
				file_putf(log_file, "Adding 1 for feather fall - general.\n");

				(artprobs[ART_IDX_GEN_FEATHER])++;
			}
		}

		if (of_has(a_ptr->flags, OF_LIGHT))
		{
			/* Handle permanent light */
			file_putf(log_file, "Adding 1 for permanent light - general.\n");

			(artprobs[ART_IDX_GEN_LIGHT])++;
		}

		if (of_has(a_ptr->flags, OF_SEE_INVIS))
		{
			/*
			 * Handle see invisible - do helms / crowns separately
			 * (Weapons were done already so exclude them)
			 */
			if( !(a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
			a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD))
			{
				if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
				{
					file_putf(log_file, "Adding 1 for see invisible - headgear.\n");

					(artprobs[ART_IDX_HELM_SINV])++;
				}
				else
				{
					file_putf(log_file, "Adding 1 for see invisible - general.\n");

					(artprobs[ART_IDX_GEN_SINV])++;
				}
			}
		}

		if (of_has(a_ptr->flags, OF_TELEPATHY))
		{
			/* ESP case.  Handle helms/crowns separately. */
			if(a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
			{
				file_putf(log_file, "Adding 1 for ESP on headgear.\n");

				(artprobs[ART_IDX_HELM_ESP])++;
			}
			else
			{
				file_putf(log_file, "Adding 1 for ESP - general.\n");

				(artprobs[ART_IDX_GEN_ESP])++;
			}
		}

		if (of_has(a_ptr->flags, OF_SLOW_DIGEST))
		{
			/* Slow digestion case - generic. */
			file_putf(log_file, "Adding 1 for slow digestion - general.\n");

			(artprobs[ART_IDX_GEN_SDIG])++;
		}

		if (of_has(a_ptr->flags, OF_REGEN))
		{
			/* Regeneration case - generic. */
			file_putf(log_file, "Adding 1 for regeneration - general.\n");

			(artprobs[ART_IDX_GEN_REGEN])++;
		}

		if (flags_test(a_ptr->flags, OF_SIZE, OF_RES_ACID, OF_RES_ELEC,
		                     OF_RES_FIRE, OF_RES_COLD, FLAG_END))
		{
			/* Count up low resists (not the type, just the number) */
			temp = 0;
			if (of_has(a_ptr->flags, OF_RES_ACID)) temp++;
			if (of_has(a_ptr->flags, OF_RES_ELEC)) temp++;
			if (of_has(a_ptr->flags, OF_RES_FIRE)) temp++;
			if (of_has(a_ptr->flags, OF_RES_COLD)) temp++;

			/* Shields treated separately */
			if (a_ptr->tval == TV_SHIELD)
			{
				file_putf(log_file, "Adding %d for low resists on shield.\n", temp);

				(artprobs[ART_IDX_SHIELD_LRES]) += temp;
			}
			else if (a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
			{
				/* Armor also treated separately */
				if (temp == 4)
				{
					/* Special case: armor has all four low resists */
					file_putf(log_file, "Adding 1 for ALL LOW RESISTS on body armor.\n");

					(artprobs[ART_IDX_ARMOR_ALLRES])++;
				}
				else
				{
					/* Just tally up the resists as usual */
					file_putf(log_file, "Adding %d for low resists on body armor.\n", temp);

					(artprobs[ART_IDX_ARMOR_LRES]) += temp;
				}
			}
			else
			{
				/* General case */
				file_putf(log_file, "Adding %d for low resists - general.\n", temp);

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
		if (a_ptr->tval == TV_SOFT_ARMOR ||
			a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
		{
			temp = 0;
			if (of_has(a_ptr->flags, OF_RES_POIS)) temp++;
			if (of_has(a_ptr->flags, OF_RES_FEAR)) temp++;
			if (of_has(a_ptr->flags, OF_RES_LIGHT)) temp++;
			if (of_has(a_ptr->flags, OF_RES_DARK)) temp++;
			if (of_has(a_ptr->flags, OF_RES_BLIND)) temp++;
			if (of_has(a_ptr->flags, OF_RES_CONFU)) temp++;
			if (of_has(a_ptr->flags, OF_RES_SOUND)) temp++;
			if (of_has(a_ptr->flags, OF_RES_SHARD)) temp++;
			if (of_has(a_ptr->flags, OF_RES_NEXUS)) temp++;
			if (of_has(a_ptr->flags, OF_RES_NETHR)) temp++;
			if (of_has(a_ptr->flags, OF_RES_CHAOS)) temp++;
			if (of_has(a_ptr->flags, OF_RES_DISEN)) temp++;
			file_putf(log_file, "Adding %d for high resists on body armor.\n", temp);

			(artprobs[ART_IDX_ARMOR_HRES]) += temp;
		}

		/* Now do the high resists individually */
		if (of_has(a_ptr->flags, OF_RES_POIS))
		{
			/* Resist poison ability */
			file_putf(log_file, "Adding 1 for resist poison - general.\n");

			(artprobs[ART_IDX_GEN_RPOIS])++;
		}

		if (of_has(a_ptr->flags, OF_RES_FEAR))
		{
			/* Resist fear ability */
			file_putf(log_file, "Adding 1 for resist fear - general.\n");

			(artprobs[ART_IDX_GEN_RFEAR])++;
		}

		if (of_has(a_ptr->flags, OF_RES_LIGHT))
		{
			/* Resist light ability */
			file_putf(log_file, "Adding 1 for resist light - general.\n");

			(artprobs[ART_IDX_GEN_RLIGHT])++;
		}

		if (of_has(a_ptr->flags, OF_RES_DARK))
		{
			/* Resist dark ability */
			file_putf(log_file, "Adding 1 for resist dark - general.\n");

			(artprobs[ART_IDX_GEN_RDARK])++;
		}

		if (of_has(a_ptr->flags, OF_RES_BLIND))
		{
			/* Resist blind ability - helms/crowns are separate */
			if(a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
			{
				file_putf(log_file, "Adding 1 for resist blindness - headgear.\n");

				(artprobs[ART_IDX_HELM_RBLIND])++;
			}
			else
			{
				/* General case */
				file_putf(log_file, "Adding 1 for resist blindness - general.\n");

				(artprobs[ART_IDX_GEN_RBLIND])++;
			}
		}

		if (of_has(a_ptr->flags, OF_RES_CONFU))
		{
			/* Resist confusion ability */
			file_putf(log_file, "Adding 1 for resist confusion - general.\n");

			(artprobs[ART_IDX_GEN_RCONF])++;
		}

		if (of_has(a_ptr->flags, OF_RES_SOUND))
		{
			/* Resist sound ability */
			file_putf(log_file, "Adding 1 for resist sound - general.\n");

			(artprobs[ART_IDX_GEN_RSOUND])++;
		}

		if (of_has(a_ptr->flags, OF_RES_SHARD))
		{
			/* Resist shards ability */
			file_putf(log_file, "Adding 1 for resist shards - general.\n");

			(artprobs[ART_IDX_GEN_RSHARD])++;
		}

		if (of_has(a_ptr->flags, OF_RES_NEXUS))
		{
			/* Resist nexus ability */
			file_putf(log_file, "Adding 1 for resist nexus - general.\n");

			(artprobs[ART_IDX_GEN_RNEXUS])++;
		}

		if (of_has(a_ptr->flags, OF_RES_NETHR))
		{
			/* Resist nether ability */
			file_putf(log_file, "Adding 1 for resist nether - general.\n");

			(artprobs[ART_IDX_GEN_RNETHER])++;
		}

		if (of_has(a_ptr->flags, OF_RES_CHAOS))
		{
			/* Resist chaos ability */
			file_putf(log_file, "Adding 1 for resist chaos - general.\n");

			(artprobs[ART_IDX_GEN_RCHAOS])++;
		}

		if (of_has(a_ptr->flags, OF_RES_DISEN))
		{
			/* Resist disenchantment ability */
			file_putf(log_file, "Adding 1 for resist disenchantment - general.\n");

			(artprobs[ART_IDX_GEN_RDISEN])++;
		}

		if (a_ptr->effect)
		{
			/* Activation */
			file_putf(log_file, "Adding 1 for activation.\n");
			(artprobs[ART_IDX_GEN_ACTIV])++;
		}
		/* Done with parsing of frequencies for this item */
	}
	/* End for loop */

	if (verbose)
	{
	/* Print out some of the abilities, to make sure that everything's fine */
		for (i = 0; i < ART_IDX_TOTAL; i++)
		{
			file_putf(log_file, "Frequency of ability %d: %d\n", i, artprobs[i]);
		}

		for (i = 0; i < z_info->k_max; i++)
		{
			file_putf(log_file, "Frequency of item %d: %d\n", i, baseprobs[i]);
		}
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
	for (i = 0; i < ART_IDX_BOW_COUNT; i++)
	{
		artprobs[art_idx_bow[i]] = (artprobs[art_idx_bow[i]] * art_total)
			/ art_bow_total;
	}

	/* All weapon abilities */
	for (i = 0; i < ART_IDX_WEAPON_COUNT; i++)
	{
		artprobs[art_idx_weapon[i]] = (artprobs[art_idx_weapon[i]] *
			art_total) / (art_bow_total + art_melee_total);
	}

	/* Corresponding non-weapon abilities */
	temp = art_total - art_melee_total - art_bow_total;
	for (i = 0; i < ART_IDX_NONWEAPON_COUNT; i++)
	{
		artprobs[art_idx_nonweapon[i]] = (artprobs[art_idx_nonweapon[i]] *
			art_total) / temp;
	}

	/* All melee weapon abilities */
	for (i = 0; i < ART_IDX_MELEE_COUNT; i++)
	{
		artprobs[art_idx_melee[i]] = (artprobs[art_idx_melee[i]] *
			art_total) / art_melee_total;
	}

	/* All general armor abilities */
	temp = art_armor_total + art_boot_total + art_shield_total +
		art_headgear_total + art_cloak_total + art_glove_total;
	for (i = 0; i < ART_IDX_ALLARMOR_COUNT; i++)
	{
		artprobs[art_idx_allarmor[i]] = (artprobs[art_idx_allarmor[i]] *
			art_total) / temp;
	}

	/* Boots */
	for (i = 0; i < ART_IDX_BOOT_COUNT; i++)
	{
		artprobs[art_idx_boot[i]] = (artprobs[art_idx_boot[i]] *
			art_total) / art_boot_total;
	}

	/* Gloves */
	for (i = 0; i < ART_IDX_GLOVE_COUNT; i++)
	{
		artprobs[art_idx_glove[i]] = (artprobs[art_idx_glove[i]] *
			art_total) / art_glove_total;
	}

	/* Headgear */
	for (i = 0; i < ART_IDX_HELM_COUNT; i++)
	{
		artprobs[art_idx_headgear[i]] = (artprobs[art_idx_headgear[i]] *
			art_total) / art_headgear_total;
	}

	/* Shields */
	for (i = 0; i < ART_IDX_SHIELD_COUNT; i++)
	{
		artprobs[art_idx_shield[i]] = (artprobs[art_idx_shield[i]] *
			art_total) / art_shield_total;
	}

	/* Cloaks */
	for (i = 0; i < ART_IDX_CLOAK_COUNT; i++)
	{
		artprobs[art_idx_cloak[i]] = (artprobs[art_idx_cloak[i]] *
			art_total) / art_cloak_total;
	}

	/* Body armor */
	for (i = 0; i < ART_IDX_ARMOR_COUNT; i++)
	{
		artprobs[art_idx_armor[i]] = (artprobs[art_idx_armor[i]] *
			art_total) / art_armor_total;
	}

	/*
	 * All others are general case and don't need to be rescaled,
	 * unless the algorithm is getting too clever about separating
	 * out individual cases (in which case some logic should be
	 * added for them in the following method call).
	 */

	/* Perform any additional rescaling and adjustment, if required. */
	adjust_freqs();

	/* Log the final frequencies to check that everything's correct */
	for(i = 0; i < ART_IDX_TOTAL; i++)
	{
		file_putf(log_file,  "Rescaled frequency of ability %d: %d\n", i, artprobs[i]);
	}

	/* Build a cumulative frequency table for the base items */
	for (i = 0; i < z_info->k_max; i++)
	{
		for (j = i; j < z_info->k_max; j++)
		{
			base_freq[j] += baseprobs[i];
		}
	}

	/* Print out the frequency table, for verification */
	for (i = 0; i < z_info->k_max; i++)
	{
		file_putf(log_file, "Cumulative frequency of item %d is: %d\n", i, base_freq[i]);
	}
}

/*
 * Adds a flag to an artifact. Returns true when canges were made.
 */
static bool add_flag(artifact_type *a_ptr, int flag)
{
	if (of_has(a_ptr->flags, flag))
		return FALSE;

	of_on(a_ptr->flags, flag);
	file_putf(log_file, "Adding ability: %s\n", flag_name(flag));

	return TRUE;
}

/*
 * Adds a flag and pval to an artifact. Always attempts
 * to increase the pval.
 */
static void add_pval_flag(artifact_type *a_ptr, int flag)
{
	of_on(a_ptr->flags, flag);
	of_on(a_ptr->pval_flags[DEFAULT_PVAL], flag);
	do_pval(a_ptr);
	file_putf(log_file, "Adding ability: %s (now %+d)\n", flag_name(flag), a_ptr->pval[DEFAULT_PVAL]);
}

/*
 * Adds a flag and a pval to an artifact, but won't increase
 * the pval if the flag is present. Returns true when changes were made.
 */
static bool add_fixed_pval_flag(artifact_type *a_ptr, int flag)
{
	if (of_has(a_ptr->flags, flag))
		return FALSE;

	of_on(a_ptr->flags, flag);
	of_on(a_ptr->pval_flags[DEFAULT_PVAL], flag);
	do_pval(a_ptr);
	file_putf(log_file, "Adding ability: %s (now %+d)\n", flag_name(flag), a_ptr->pval[DEFAULT_PVAL]);

	return TRUE;
}

/*
 * Adds a flag and an initial pval to an artifact.  Returns true
 * when the flag was not present.
 */
static bool add_first_pval_flag(artifact_type *a_ptr, int flag)
{
	of_on(a_ptr->flags, flag);
	of_on(a_ptr->pval_flags[DEFAULT_PVAL], flag);

	if (a_ptr->pval[DEFAULT_PVAL] == 0)
	{
		a_ptr->pval[DEFAULT_PVAL] = (s16b)randint1(4);
		file_putf(log_file, "Adding ability: %s (first time) (now %+d)\n", flag_name(flag), a_ptr->pval[DEFAULT_PVAL]);

		return TRUE;
	}

	do_pval(a_ptr);
	file_putf(log_file, "Adding ability: %s (now %+d)\n", flag_name(flag), a_ptr->pval[DEFAULT_PVAL]);

	return FALSE;
}

/* Count pvals and set num_pvals accordingly*/
static void recalc_num_pvals(artifact_type *a_ptr)
{
	int i;

	a_ptr->num_pvals = 0;
	for (i = 0; i < MAX_PVALS; i++)
		if (a_ptr->pval[i] != 0) a_ptr->num_pvals++;
	file_putf(log_file, "a_ptr->num_pvals is now %d.\n", a_ptr->num_pvals);
}

static void add_stat(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack: break out if all stats are raised to avoid an infinite loop */
	if (flags_test_all(a_ptr->flags, OF_SIZE, OF_STR, OF_INT, OF_WIS,
	                         OF_DEX, OF_CON, OF_CHR, FLAG_END))
			return;

	/* Make sure we add one that hasn't been added yet */
	while (!success)
	{
		r = randint0(6);
		if (r == 0) success = add_fixed_pval_flag(a_ptr, OF_STR);
		else if (r == 1) success = add_fixed_pval_flag(a_ptr, OF_INT);
		else if (r == 2) success = add_fixed_pval_flag(a_ptr, OF_WIS);
		else if (r == 3) success = add_fixed_pval_flag(a_ptr, OF_DEX);
		else if (r == 4) success = add_fixed_pval_flag(a_ptr, OF_CON);
		else if (r == 5) success = add_fixed_pval_flag(a_ptr, OF_CHR);
	}
}

static void add_sustain(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack: break out if all stats are sustained to avoid an infinite loop */
	if (flags_test_all(a_ptr->flags, OF_SIZE, OF_SUST_STR, OF_SUST_INT,
	    OF_SUST_WIS, OF_SUST_DEX, OF_SUST_CON, OF_SUST_CHR, FLAG_END))
			return;

	while (!success)
	{
		r = randint0(6);
		if (r == 0) success = add_flag(a_ptr, OF_SUST_STR);
		else if (r == 1) success = add_flag(a_ptr, OF_SUST_INT);
		else if (r == 2) success = add_flag(a_ptr, OF_SUST_WIS);
		else if (r == 3) success = add_flag(a_ptr, OF_SUST_DEX);
		else if (r == 4) success = add_flag(a_ptr, OF_SUST_CON);
		else if (r == 5) success = add_flag(a_ptr, OF_SUST_CHR);
	}
}

static void add_low_resist(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack - if all low resists added already, exit to avoid infinite loop */
	if(flags_test_all(a_ptr->flags, OF_SIZE, OF_RES_ACID, OF_RES_ELEC,
	                        OF_RES_FIRE, OF_RES_COLD, FLAG_END))
			return;

	while (!success)
	{
		r = randint0(4);
		if (r == 0) success = add_flag(a_ptr, OF_RES_ACID);
		else if (r == 1) success = add_flag(a_ptr, OF_RES_ELEC);
		else if (r == 2) success = add_flag(a_ptr, OF_RES_FIRE);
		else if (r == 3) success = add_flag(a_ptr, OF_RES_COLD);
	}
}

static void add_high_resist(artifact_type *a_ptr)
{
	/* Add a high resist, according to the generated frequency distribution. */
	int r, i, temp;
	int count = 0;
	bool success = FALSE;

	temp = 0;
	for (i = 0; i < ART_IDX_HIGH_RESIST_COUNT; i++)
	{
		temp += artprobs[art_idx_high_resist[i]];
	}

	/* The following will fail (cleanly) if all high resists already added */
	while ( (!success) && (count < MAX_TRIES) )
	{
		/* Randomize from 1 to this total amount */
		r = randint1(temp);

		/* Determine which (weighted) resist this number corresponds to */

		temp = artprobs[art_idx_high_resist[0]];
		i = 0;
		while (r > temp && i < ART_IDX_HIGH_RESIST_COUNT)
		{
			temp += artprobs[art_idx_high_resist[i]];
			i++;
		}

		/* Now i should give us the index of the correct high resist */
		if (i == 0) success = add_flag(a_ptr, OF_RES_POIS);
		else if (i == 1) success = add_flag(a_ptr, OF_RES_FEAR);
		else if (i == 2) success = add_flag(a_ptr, OF_RES_LIGHT);
		else if (i == 3) success = add_flag(a_ptr, OF_RES_DARK);
		else if (i == 4) success = add_flag(a_ptr, OF_RES_BLIND);
		else if (i == 5) success = add_flag(a_ptr, OF_RES_CONFU);
		else if (i == 6) success = add_flag(a_ptr, OF_RES_SOUND);
		else if (i == 7) success = add_flag(a_ptr, OF_RES_SHARD);
		else if (i == 8) success = add_flag(a_ptr, OF_RES_NEXUS);
		else if (i == 9) success = add_flag(a_ptr, OF_RES_NETHR);
		else if (i == 10) success = add_flag(a_ptr, OF_RES_CHAOS);
		else if (i == 11) success = add_flag(a_ptr, OF_RES_DISEN);

		count++;
	}
}

static void add_slay(artifact_type *a_ptr, bool brand)
{
	int count = 0;
	const struct slay *s_ptr;
	bitflag mask[OF_SIZE];

	if (brand)
		create_mask(mask, FALSE, OFT_BRAND, OFT_MAX);
	else
		create_mask(mask, FALSE, OFT_SLAY, OFT_KILL, OFT_MAX);

	for(count = 0; count < MAX_TRIES; count++) {
		s_ptr = random_slay(mask);

		if (!of_has(a_ptr->flags, s_ptr->object_flag)) {
			of_on(a_ptr->flags, s_ptr->object_flag);

			file_putf(log_file, "Adding %s: %s\n", s_ptr->brand ? "brand" : "slay", s_ptr->brand ? s_ptr->brand : s_ptr->desc);
			return;
		}
	}
}

static void add_damage_dice(artifact_type *a_ptr)
{
	/* CR 2001-09-02: changed this to increments 1 or 2 only */
	a_ptr->dd += (byte)randint1(2);
/*	if (a_ptr->dd > 9)
		a_ptr->dd = 9; */
	file_putf(log_file, "Adding ability: extra damage dice (now %d dice)\n", a_ptr->dd);
}

static void add_to_hit(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_h > VERYHIGH_TO_HIT)
	{
		if (!INHIBIT_STRONG)
		{
			file_putf(log_file, "Failed to add to-hit, value of %d is too high\n", a_ptr->to_h);
			return;
		}
	}
	else if (a_ptr->to_h > HIGH_TO_HIT)
	{
		if (!INHIBIT_WEAK)
		{
			file_putf(log_file, "Failed to add to-hit, value of %d is too high\n", a_ptr->to_h);
			return;
		}
	}
	a_ptr->to_h += (s16b)(fixed + randint0(random));
	if (a_ptr->to_h > 0) of_on(a_ptr->flags, OF_SHOW_MODS);
	file_putf(log_file, "Adding ability: extra to_h (now %+d)\n", a_ptr->to_h);
}

static void add_to_dam(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_d > VERYHIGH_TO_DAM)
	{
		if (!INHIBIT_STRONG)
		{
			file_putf(log_file, "Failed to add to-dam, value of %d is too high\n", a_ptr->to_d);
			return;
		}
	}
	else if (a_ptr->to_h > HIGH_TO_DAM)
	{
		if (!INHIBIT_WEAK)
		{
			file_putf(log_file, "Failed to add to-dam, value of %d is too high\n", a_ptr->to_d);
			return;
		}
	}
	a_ptr->to_d += (s16b)(fixed + randint0(random));
	if (a_ptr->to_d > 0) of_on(a_ptr->flags, OF_SHOW_MODS);
	file_putf(log_file, "Adding ability: extra to_dam (now %+d)\n", a_ptr->to_d);
}

static void add_to_AC(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_a > VERYHIGH_TO_AC)
	{
		if (!INHIBIT_STRONG)
		{
			file_putf(log_file, "Failed to add to-AC, value of %d is too high\n", a_ptr->to_a);
			return;
		}
	}
	else if (a_ptr->to_h > HIGH_TO_AC)
	{
		if (!INHIBIT_WEAK)
		{
			file_putf(log_file, "Failed to add to-AC, value of %d is too high\n", a_ptr->to_a);
			return;
		}
	}
	a_ptr->to_a += (s16b)(fixed + randint0(random));
	file_putf(log_file, "Adding ability: AC bonus (new bonus is %+d)\n", a_ptr->to_a);
}

static void add_weight_mod(artifact_type *a_ptr)
{
	a_ptr->weight = (a_ptr->weight * 9) / 10;
	file_putf(log_file, "Adding ability: lower weight (new weight is %d)\n", a_ptr->weight);
}

/*
 * Add a random immunity to this artifact
 * ASSUMPTION: All immunities are equally likely.
 * ToDo: replace with lookup once immunities are abstracted
 */
static void add_immunity(artifact_type *a_ptr)
{
	int imm_type = randint0(4);

	switch(imm_type)
	{
		case 0:
		{
			of_on(a_ptr->flags, OF_IM_ACID);
			file_putf(log_file, "Adding ability: immunity to acid\n");
			break;
		}
		case 1:
		{
			of_on(a_ptr->flags, OF_IM_ELEC);
			file_putf(log_file, "Adding ability: immunity to lightning\n");
			break;
		}
		case 2:
		{
			of_on(a_ptr->flags, OF_IM_FIRE);
			file_putf(log_file, "Adding ability: immunity to fire\n");
			break;
		}
		case 3:
		{
			of_on(a_ptr->flags, OF_IM_COLD);
			file_putf(log_file, "Adding ability: immunity to cold\n");
			break;
		}
	}
}

/* Add an activation (called only if artifact does not yet have one) */
static void add_activation(artifact_type *a_ptr, s32b target_power)
{
	int i, x, p, max_effect = 0;
	int count = 0;

	/* Work out the maximum allowed effect power */
	for (i = 0; i < EF_MAX; i++)
	{
		if (effect_power(i) > max_effect && effect_power(i) <
			INHIBIT_POWER)
			max_effect = effect_power(i);
	}

	/* Select an effect at random */
	while (count < MAX_TRIES)
	{
		x = randint0(EF_MAX);
		p = effect_power(x);

		/*
		 * Check that activation is useful but not exploitable,
		 * and roughly proportionate to the overall power
		 */
		if (p < INHIBIT_POWER && 100 * p / max_effect > 50 *
			target_power / max_power && 100 * p / max_effect < 200
			* target_power / max_power)
		{
			file_putf(log_file, "Adding activation effect %d\n", x);
			a_ptr->effect = x;
			a_ptr->time.base = (p * 8);
			a_ptr->time.dice = (p > 5 ? p / 5 : 1);
			a_ptr->time.sides = p;
			return;
		}
		count++;
	}
}


/*
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
static void build_freq_table(artifact_type *a_ptr, s16b *freq)
{
	int i,j;
	s16b f_temp[ART_IDX_TOTAL];

	/* First, set everything to zero */
	for (i = 0; i < ART_IDX_TOTAL; i++)
	{
		f_temp[i] = 0;
		freq[i] = 0;
	}

	/* Now copy over appropriate frequencies for applicable abilities */
	/* Bow abilities */
	if (a_ptr->tval == TV_BOW)
	{
		for (j = 0; j < ART_IDX_BOW_COUNT; j++)
		{
			f_temp[art_idx_bow[j]] = artprobs[art_idx_bow[j]];
		}
	}
	/* General weapon abilities */
	if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
		a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
		a_ptr->tval == TV_SWORD)
	{
		for (j = 0; j < ART_IDX_WEAPON_COUNT; j++)
		{
			f_temp[art_idx_weapon[j]] = artprobs[art_idx_weapon[j]];
		}
	}
	/* General non-weapon abilities */
	else
	{
		for (j = 0; j < ART_IDX_NONWEAPON_COUNT; j++)
		{
			f_temp[art_idx_nonweapon[j]] = artprobs[art_idx_nonweapon[j]];
		}
	}
	/* General melee abilities */
	if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
		a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
	{
		for (j = 0; j < ART_IDX_MELEE_COUNT; j++)
		{
			f_temp[art_idx_melee[j]] = artprobs[art_idx_melee[j]];
		}
	}
	/* General armor abilities */
	if ( a_ptr->tval == TV_BOOTS || a_ptr->tval == TV_GLOVES ||
		a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN ||
		a_ptr->tval == TV_SHIELD || a_ptr->tval == TV_CLOAK ||
		a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
		a_ptr->tval == TV_DRAG_ARMOR)
		{
		for (j = 0; j < ART_IDX_ALLARMOR_COUNT; j++)
		{
			f_temp[art_idx_allarmor[j]] = artprobs[art_idx_allarmor[j]];
		}
	}
	/* Boot abilities */
	if (a_ptr->tval == TV_BOOTS)
	{
		for (j = 0; j < ART_IDX_BOOT_COUNT; j++)
		{
			f_temp[art_idx_boot[j]] = artprobs[art_idx_boot[j]];
		}
	}
	/* Glove abilities */
	if (a_ptr->tval == TV_GLOVES)
	{
		for (j = 0; j < ART_IDX_GLOVE_COUNT; j++)
		{
			f_temp[art_idx_glove[j]] = artprobs[art_idx_glove[j]];
		}
	}
	/* Headgear abilities */
	if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
	{
		for (j = 0; j < ART_IDX_HELM_COUNT; j++)
		{
			f_temp[art_idx_headgear[j]] = artprobs[art_idx_headgear[j]];
		}
	}
	/* Shield abilities */
	if (a_ptr->tval == TV_SHIELD)
	{
		for (j = 0; j < ART_IDX_SHIELD_COUNT; j++)
		{
			f_temp[art_idx_shield[j]] = artprobs[art_idx_shield[j]];
		}
	}
	/* Cloak abilities */
	if (a_ptr->tval == TV_CLOAK)
	{
		for (j = 0; j < ART_IDX_CLOAK_COUNT; j++)
		{
			f_temp[art_idx_cloak[j]] = artprobs[art_idx_cloak[j]];
		}
	}
	/* Armor abilities */
	if (a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
		a_ptr->tval == TV_DRAG_ARMOR)
	{
		for (j = 0; j < ART_IDX_ARMOR_COUNT; j++)
		{
			f_temp[art_idx_armor[j]] = artprobs[art_idx_armor[j]];
		}
	}
	/* General abilities - no constraint */
	for (j = 0; j < ART_IDX_GEN_COUNT; j++)
	{
		f_temp[art_idx_gen[j]] = artprobs[art_idx_gen[j]];
	}

	/*
	 * Now we have the correct individual frequencies, we build a cumulative
	 * frequency table for them.
	 */
	for (i = 0; i < ART_IDX_TOTAL; i++)
	{
		for (j = i; j < ART_IDX_TOTAL; j++)
		{
			freq[j] += f_temp[i];
		}
	}
	/* Done - the freq array holds the desired frequencies. */

	/* Print out the frequency table, for verification */
	for (i = 0; i < ART_IDX_TOTAL; i++)
		file_putf(log_file, "Cumulative frequency of ability %d is: %d\n", i, freq[i]);
}

/*
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

/*
 * Add an ability given by the index r.  This is mostly just a long case
 * statement.
 *
 * Note that this method is totally general and imposes no restrictions on
 * appropriate item type for a given ability.  This is assumed to have
 * been done already.
 */

static void add_ability_aux(artifact_type *a_ptr, int r, s32b target_power)
{
	switch(r)
	{
		case ART_IDX_BOW_SHOTS:
		case ART_IDX_NONWEAPON_SHOTS:
			add_pval_flag(a_ptr, OF_SHOTS);
			break;

		case ART_IDX_BOW_MIGHT:
			add_pval_flag(a_ptr, OF_MIGHT);
			break;

		case ART_IDX_WEAPON_HIT:
		case ART_IDX_NONWEAPON_HIT:
			add_to_hit(a_ptr, 1, 2 * mean_hit_increment);
			break;

		case ART_IDX_WEAPON_DAM:
		case ART_IDX_NONWEAPON_DAM:
			add_to_dam(a_ptr, 1, 2 * mean_dam_increment);
			break;

		case ART_IDX_NONWEAPON_HIT_DAM:
			add_to_hit(a_ptr, 1, 2 * mean_hit_increment);
			add_to_dam(a_ptr, 1, 2 * mean_dam_increment);
			break;

		case ART_IDX_WEAPON_AGGR:
		case ART_IDX_NONWEAPON_AGGR:
			if (target_power > AGGR_POWER)
			{
				add_flag(a_ptr, OF_AGGRAVATE);
			}
			break;

		case ART_IDX_MELEE_BLESS:
			add_flag(a_ptr, OF_BLESSED);
			break;

		case ART_IDX_BOW_BRAND:
		case ART_IDX_MELEE_BRAND:
		case ART_IDX_NONWEAPON_BRAND:
			add_slay(a_ptr, TRUE);
			break;

		case ART_IDX_BOW_SLAY:
		case ART_IDX_MELEE_SLAY:
		case ART_IDX_NONWEAPON_SLAY:
			add_slay(a_ptr, FALSE);
			break;

		case ART_IDX_MELEE_SINV:
		case ART_IDX_HELM_SINV:
		case ART_IDX_GEN_SINV:
			add_flag(a_ptr, OF_SEE_INVIS);
			break;

		case ART_IDX_MELEE_BLOWS:
		case ART_IDX_NONWEAPON_BLOWS:
			add_pval_flag(a_ptr, OF_BLOWS);
			break;

		case ART_IDX_MELEE_AC:
		case ART_IDX_BOOT_AC:
		case ART_IDX_GLOVE_AC:
		case ART_IDX_HELM_AC:
		case ART_IDX_SHIELD_AC:
		case ART_IDX_CLOAK_AC:
		case ART_IDX_ARMOR_AC:
		case ART_IDX_GEN_AC:
			add_to_AC(a_ptr, 1, 2 * mean_ac_increment);
			break;

		case ART_IDX_MELEE_DICE:
			add_damage_dice(a_ptr);
			break;

		case ART_IDX_MELEE_WEIGHT:
		case ART_IDX_ALLARMOR_WEIGHT:
			add_weight_mod(a_ptr);
			break;

		case ART_IDX_MELEE_TUNN:
		case ART_IDX_GEN_TUNN:
			add_pval_flag(a_ptr, OF_TUNNEL);
			break;

		case ART_IDX_BOOT_FEATHER:
		case ART_IDX_GEN_FEATHER:
			add_flag(a_ptr, OF_FEATHER);
			break;

		case ART_IDX_BOOT_STEALTH:
		case ART_IDX_CLOAK_STEALTH:
		case ART_IDX_ARMOR_STEALTH:
		case ART_IDX_GEN_STEALTH:
			add_pval_flag(a_ptr, OF_STEALTH);
			break;

		case ART_IDX_BOOT_SPEED:
		case ART_IDX_GEN_SPEED:
			add_first_pval_flag(a_ptr, OF_SPEED);
			break;

		case ART_IDX_GLOVE_FA:
		case ART_IDX_GEN_FA:
			add_flag(a_ptr, OF_FREE_ACT);
			break;

		case ART_IDX_GLOVE_DEX:
			add_fixed_pval_flag(a_ptr, OF_DEX);
			break;

		case ART_IDX_HELM_RBLIND:
		case ART_IDX_GEN_RBLIND:
			add_flag(a_ptr, OF_RES_BLIND);
			break;

		case ART_IDX_HELM_ESP:
		case ART_IDX_GEN_ESP:
			add_flag(a_ptr, OF_TELEPATHY);
			break;

		case ART_IDX_HELM_WIS:
			add_fixed_pval_flag(a_ptr, OF_WIS);
			break;

		case ART_IDX_HELM_INT:
			add_fixed_pval_flag(a_ptr, OF_INT);
			break;

		case ART_IDX_SHIELD_LRES:
		case ART_IDX_ARMOR_LRES:
		case ART_IDX_GEN_LRES:
			add_low_resist(a_ptr);
			break;

		case ART_IDX_ARMOR_HLIFE:
		case ART_IDX_GEN_HLIFE:
			add_flag(a_ptr, OF_HOLD_LIFE);
			break;

		case ART_IDX_ARMOR_CON:
			add_fixed_pval_flag(a_ptr, OF_CON);
			break;

		case ART_IDX_ARMOR_ALLRES:
			add_flag(a_ptr, OF_RES_ACID);
			add_flag(a_ptr, OF_RES_ELEC);
			add_flag(a_ptr, OF_RES_FIRE);
			add_flag(a_ptr, OF_RES_COLD);
			break;

		case ART_IDX_ARMOR_HRES:
			add_high_resist(a_ptr);
			break;

		case ART_IDX_GEN_STAT:
			add_stat(a_ptr);
			break;

		case ART_IDX_GEN_SUST:
			add_sustain(a_ptr);
			break;

		case ART_IDX_GEN_SEARCH:
			add_pval_flag(a_ptr, OF_SEARCH);
			break;

		case ART_IDX_GEN_INFRA:
			add_pval_flag(a_ptr, OF_INFRA);
			break;

		case ART_IDX_GEN_IMMUNE:
			add_immunity(a_ptr);
			break;

		case ART_IDX_GEN_LIGHT:
			add_flag(a_ptr, OF_LIGHT);
			break;

		case ART_IDX_GEN_SDIG:
			add_flag(a_ptr, OF_SLOW_DIGEST);
			break;

		case ART_IDX_GEN_REGEN:
			add_flag(a_ptr, OF_REGEN);
			break;

		case ART_IDX_GEN_RPOIS:
			add_flag(a_ptr, OF_RES_POIS);
			break;

		case ART_IDX_GEN_RFEAR:
			add_flag(a_ptr, OF_RES_FEAR);
			break;

		case ART_IDX_GEN_RLIGHT:
			add_flag(a_ptr, OF_RES_LIGHT);
			break;

		case ART_IDX_GEN_RDARK:
			add_flag(a_ptr, OF_RES_DARK);
			break;

		case ART_IDX_GEN_RCONF:
			add_flag(a_ptr, OF_RES_CONFU);
			break;

		case ART_IDX_GEN_RSOUND:
			add_flag(a_ptr, OF_RES_SOUND);
			break;

		case ART_IDX_GEN_RSHARD:
			add_flag(a_ptr, OF_RES_SHARD);
			break;

		case ART_IDX_GEN_RNEXUS:
			add_flag(a_ptr, OF_RES_NEXUS);
			break;

		case ART_IDX_GEN_RNETHER:
			add_flag(a_ptr, OF_RES_NETHR);
			break;

		case ART_IDX_GEN_RCHAOS:
			add_flag(a_ptr, OF_RES_CHAOS);
			break;

		case ART_IDX_GEN_RDISEN:
			add_flag(a_ptr, OF_RES_DISEN);
			break;

		case ART_IDX_GEN_ACTIV:
			if (!a_ptr->effect) add_activation(a_ptr, target_power);
			break;
	}

	recalc_num_pvals(a_ptr);
}

/*
 * Randomly select an extra ability to be added to the artifact in question.
 */
static void add_ability(artifact_type *a_ptr, s32b target_power)
{
	int r;

	/* Choose a random ability using the frequency table previously defined*/
	r = choose_ability(art_freq);

	/* Add the appropriate ability */
	add_ability_aux(a_ptr, r, target_power);

	/* Now remove contradictory or redundant powers. */
	remove_contradictory(a_ptr);

	/* Adding WIS to sharp weapons always blesses them */
	if (of_has(a_ptr->flags, OF_WIS) && (a_ptr->tval == TV_SWORD || a_ptr->tval == TV_POLEARM))
	{
		add_flag(a_ptr, OF_BLESSED);
	}
}


/*
 * Try to supercharge this item by running through the list of the supercharge
 * abilities and attempting to add each in turn.  An artifact only gets one
 * chance at each of these up front (if applicable).
 */
static void try_supercharge(artifact_type *a_ptr, s32b target_power)
{
	/* Huge damage dice or +3 blows - melee weapon only */
	if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
		a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
	{
		if (randint0(z_info->a_max) < artprobs[ART_IDX_MELEE_DICE_SUPER])
		{
			a_ptr->dd += 3 + randint0(4);
/*			if (a_ptr->dd > 9) a_ptr->dd = 9; */
			file_putf(log_file, "Supercharging damage dice!  (Now %d dice)\n", a_ptr->dd);
		}
		else if (randint0(z_info->a_max) < artprobs[ART_IDX_MELEE_BLOWS_SUPER])
		{
			of_on(a_ptr->flags, OF_BLOWS);
			of_on(a_ptr->pval_flags[DEFAULT_PVAL], OF_BLOWS);
			a_ptr->pval[DEFAULT_PVAL] = 3;
			file_putf(log_file, "Supercharging melee blows! (+3 blows)\n");
		}
	}

	/* Bows - +3 might or +3 shots */
	if (a_ptr->tval == TV_BOW)
	{
		if (randint0(z_info->a_max) < artprobs[ART_IDX_BOW_SHOTS_SUPER])
		{
			of_on(a_ptr->flags, OF_SHOTS);
			of_on(a_ptr->pval_flags[DEFAULT_PVAL], OF_SHOTS);
			a_ptr->pval[DEFAULT_PVAL] = 3;
			file_putf(log_file, "Supercharging shots for bow!  (3 extra shots)\n");
		}
		else if (randint0(z_info->a_max) < artprobs[ART_IDX_BOW_MIGHT_SUPER])
		{
			of_on(a_ptr->flags, OF_MIGHT);
			of_on(a_ptr->pval_flags[DEFAULT_PVAL], OF_MIGHT);
			a_ptr->pval[DEFAULT_PVAL] = 3;
			file_putf(log_file, "Supercharging might for bow!  (3 extra might)\n");
		}
	}

	/* Big speed bonus - any item (potentially) but more likely on boots */
	if (randint0(z_info->a_max) < artprobs[ART_IDX_GEN_SPEED_SUPER] ||
		(a_ptr->tval == TV_BOOTS && randint0(z_info->a_max) <
		artprobs[ART_IDX_BOOT_SPEED]))
	{
		of_on(a_ptr->flags, OF_SPEED);
		of_on(a_ptr->pval_flags[DEFAULT_PVAL], OF_SPEED);
		a_ptr->pval[DEFAULT_PVAL] = 5 + randint0(6);
		if (INHIBIT_WEAK) a_ptr->pval[DEFAULT_PVAL] += randint1(3);
		if (INHIBIT_STRONG) a_ptr->pval[DEFAULT_PVAL] += 1 + randint1(6);
		file_putf(log_file, "Supercharging speed for this item!  (New speed bonus is %d)\n", a_ptr->pval[DEFAULT_PVAL]);
	}

	/* Big AC bonus */
	if (randint0(z_info->a_max) < artprobs[ART_IDX_GEN_AC_SUPER])
	{
		a_ptr->to_a += 19 + randint1(11);
		if (INHIBIT_WEAK) a_ptr->to_a += randint1(10);
		if (INHIBIT_STRONG) a_ptr->to_a += randint1(20);
		file_putf(log_file, "Supercharging AC! New AC bonus is %d\n", a_ptr->to_a);
	}

	/* Aggravation */
	if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
		a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
		a_ptr->tval == TV_SWORD)
	{
		if ((randint0(z_info->a_max) < artprobs[ART_IDX_WEAPON_AGGR]) &&
		    (target_power > AGGR_POWER))
		{
			of_on(a_ptr->flags, OF_AGGRAVATE);
			file_putf(log_file, "Adding aggravation\n");
		}
	}
	else
	{
		if ((randint0(z_info->a_max) < artprobs[ART_IDX_NONWEAPON_AGGR]) &&
		    (target_power > AGGR_POWER))
		{
			of_on(a_ptr->flags, OF_AGGRAVATE);
			file_putf(log_file, "Adding aggravation\n");
		}
	}

	recalc_num_pvals(a_ptr);
}

/*
 * Make it bad, or if it's already bad, make it worse!
 */
static void do_curse(artifact_type *a_ptr)
{
	if (one_in_(7))
		of_on(a_ptr->flags, OF_AGGRAVATE);
	if (one_in_(4))
		of_on(a_ptr->flags, OF_DRAIN_EXP);
	if (one_in_(7))
		of_on(a_ptr->flags, OF_TELEPORT);

	if ((a_ptr->pval[DEFAULT_PVAL] > 0) && one_in_(2))
		a_ptr->pval[DEFAULT_PVAL] = -a_ptr->pval[DEFAULT_PVAL];
	if ((a_ptr->to_a > 0) && one_in_(2))
		a_ptr->to_a = -a_ptr->to_a;
	if ((a_ptr->to_h > 0) && one_in_(2))
		a_ptr->to_h = -a_ptr->to_h;
	if ((a_ptr->to_d > 0) && one_in_(4))
		a_ptr->to_d = -a_ptr->to_d;

	if (of_has(a_ptr->flags, OF_LIGHT_CURSE))
	{
		if (one_in_(2)) of_on(a_ptr->flags, OF_HEAVY_CURSE);
		return;
	}

	of_on(a_ptr->flags, OF_LIGHT_CURSE);

	if (one_in_(4))
		of_on(a_ptr->flags, OF_HEAVY_CURSE);
}

/*
 * Note the three special cases (One Ring, Grond, Morgoth).
 */
static void scramble_artifact(int a_idx)
{
	artifact_type *a_ptr = &a_info[a_idx];
	artifact_type a_old;
	object_kind *k_ptr;
	s32b power;
	int tries = 0;
	byte alloc_old, base_alloc_old, alloc_new;
	s32b ap = 0;
	bool curse_me = FALSE;
	bool success = FALSE;
	int i;
	bitflag f[OF_SIZE];

	/* Special cases -- don't randomize these! */
	if ((a_idx == ART_POWER) ||
	    (a_idx == ART_GROND) ||
	    (a_idx == ART_MORGOTH))
		return;

	/* Skip unused artifacts, too! */
	if (a_ptr->tval == 0) return;

	/* Evaluate the original artifact to determine the power level. */
	power = base_power[a_idx];

	/* If it has a restricted ability then don't randomize it. */
	if (power > INHIBIT_POWER)
	{
		file_putf(log_file, "Skipping artifact number %d - too powerful to randomize!", a_idx);
		return;
	}

	if (power < 0) curse_me = TRUE;

	file_putf(log_file, "+++++++++++++++++ CREATING NEW ARTIFACT ++++++++++++++++++\n");
	file_putf(log_file, "Artifact %d: power = %d\n", a_idx, power);

	/*
	 * Flip the sign on power if it's negative, since it's only used for base
	 * item choice
	 */
	if (power < 0) power = -power;

	if (a_idx >= ART_MIN_NORMAL)
	{
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
		do
		{
			/* Get the new item kind */
			k_ptr = choose_item(a_idx);

			/*
			 * Hack: if power is positive but very low, and if we're not having
			 * any luck finding a base item, curse it once.  This helps ensure
			 * that we get a base item for borderline cases like Wormtongue.
			 */

			if (power > 0 && power < 10 && count > MAX_TRIES / 2)
			{
				file_putf(log_file, "Cursing base item to help get a match.\n");
				do_curse(a_ptr);
			}
			ap2 = artifact_power(a_idx);
			count++;
			/*
			 * Calculate the proper rarity based on the new type.  We attempt
			 * to preserve the 'effective rarity' which is equal to the
			 * artifact rarity multiplied by the base item rarity.
			 */

			alloc_new = alloc_old * base_alloc_old
				/ k_ptr->alloc_prob;

			if (alloc_new > 99) alloc_new = 99;
			if (alloc_new < 1) alloc_new = 1;

			file_putf(log_file, "Old allocs are base %d, art %d\n", base_alloc_old, alloc_old);
			file_putf(log_file, "New allocs are base %d, art %d\n", k_ptr->alloc_prob, alloc_new);

		} while ( (count < MAX_TRIES) &&
		          (((ap2 > (power * 6) / 10 + 1) && (power-ap2 < 20)) ||
		          (ap2 < (power / 10))) );

		/* Got an item - set the new rarity */
		a_ptr->alloc_prob = alloc_new;

		if (count >= MAX_TRIES)
		{
			msg("Warning! Couldn't get appropriate power level on base item.");
			file_putf(log_file, "Warning! Couldn't get appropriate power level on base item.\n");
		}
	}
	else
	{
		/* Special artifact (light source, ring, or amulet) */

		/* Keep the item kind */
		k_ptr = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Clear the following fields; leave the rest alone */
		a_ptr->to_h = a_ptr->to_d = a_ptr->to_a = 0;
		a_ptr->num_pvals = 0;
		of_wipe(a_ptr->flags);
		for (i = 0; i < MAX_PVALS; i++)
		{
			a_ptr->pval[i] = 0;
			of_wipe(a_ptr->pval_flags[i]);
		}

		/* Clear the activations for rings and amulets but not lights */
		if (a_ptr->tval != TV_LIGHT) a_ptr->effect = 0;

		/* Artifacts ignore everything */
		create_mask(f, FALSE, OFT_IGNORE, OFT_MAX);
		of_union(a_ptr->flags, f);

		file_putf(log_file, "Alloc prob is %d\n", a_ptr->alloc_prob);
	}

	/* Got a base item. */

	/* Generate the cumulative frequency table for this item type */
	build_freq_table(a_ptr, art_freq);

	/* Copy artifact info temporarily. */
	a_old = *a_ptr;

	/* Give this artifact a shot at being supercharged */
	try_supercharge(a_ptr, power);
	ap = artifact_power(a_idx);
	if (ap > (power * 23) / 20 + 1)
	{
		/* too powerful -- put it back */
		*a_ptr = a_old;
		file_putf(log_file, "--- Supercharge is too powerful!  Rolling back.\n");
	}

	/* First draft: add two abilities, then curse it three times. */
	if (curse_me)
	{
		/* Copy artifact info temporarily. */
		a_old = *a_ptr;
		do
		{
			add_ability(a_ptr, power);
			add_ability(a_ptr, power);
			do_curse(a_ptr);
			do_curse(a_ptr);
			do_curse(a_ptr);
			remove_contradictory(a_ptr);
			ap = artifact_power(a_idx);
			/* Accept if it doesn't have any inhibited abilities */
			if (ap < INHIBIT_POWER) success = TRUE;
			/* Otherwise go back and try again */
			else
			{
				file_putf(log_file, "Inhibited ability added - rolling back.\n");
				*a_ptr = a_old;
			}
		}
		while (!success);

		/* Cursed items never have any resale value */
		a_ptr->cost = 0;
	}
	else
	{
		/*
		 * Select a random set of abilities which roughly matches the
		 * original's in terms of overall power/usefulness.
		 */
		for (tries = 0; tries < MAX_TRIES; tries++)
		{
			/* Copy artifact info temporarily. */
			a_old = *a_ptr;

			add_ability(a_ptr, power);
			ap = artifact_power(a_idx);

			/* CR 11/14/01 - pushed both limits up by about 5% */
			if (ap > (power * 23) / 20 + 1)
			{
				/* too powerful -- put it back */
				*a_ptr = a_old;
				file_putf(log_file, "--- Too powerful!  Rolling back.\n");
				continue;
			}
			else if (ap >= (power * 19) / 20)	/* just right */
			{
				/* CC 11/02/09 - add rescue for crappy weapons */
				if ((a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
					a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD
					|| a_ptr->tval == TV_BOW) && (a_ptr->to_d < 10))
				{
					a_ptr->to_d += randint0(10);
					file_putf(log_file, "Redeeming crappy weapon: +dam now %d\n", a_ptr->to_d);
				}

				break;
			}

			/* Stop if we're going negative, so we don't overload
			   the artifact with great powers to compensate. */
			/* Removed CR 11/10/01 */
			/*
			else if ((ap < 0) && (ap < (-(power * 1)) / 10))
			{
				break;
			}
			*/
		}		/* end of power selection */

		if (verbose && tries >= MAX_TRIES)
		{
			/*
			 * We couldn't generate an artifact within the number of permitted
			 * iterations.  Show a warning message.
			 */
			msg("Warning!  Couldn't get appropriate power level on artifact.");
			file_putf(log_file, "Warning!  Couldn't get appropriate power level on artifact.\n");
			message_flush();
		}
	}

	/* Set depth and rarity info according to power */
	/* This is currently very tricky for special artifacts */
	file_putf(log_file, "Old depths are min %d, max %d\n", a_ptr->alloc_min, a_ptr->alloc_max);
	file_putf(log_file, "Alloc prob is %d\n", a_ptr->alloc_prob);

	/* flip cursed items to avoid overflows */
	if (ap < 0) ap = -ap;

	if (a_idx < ART_MIN_NORMAL)
	{
		a_ptr->alloc_max = 127;
		if (ap > avg_power)
		{
			a_ptr->alloc_prob = 1;
			a_ptr->alloc_min = MAX(50, ((ap + 150) * 100 /
				max_power));
		}
		else if (ap > 30)
		{
			a_ptr->alloc_prob = MAX(2, (avg_power - ap) / 20);
			a_ptr->alloc_min = MAX(25, ((ap + 200) * 100 /
				max_power));
		}
		else /* Just the Phial */
		{
			a_ptr->alloc_prob = 50 - ap;
			a_ptr->alloc_min = 5;
		}
	}
	else
	{
		file_putf(log_file, "k_ptr->alloc_prob is %d\n", k_ptr->alloc_prob);
		a_ptr->alloc_max = MIN(127, (ap * 4) / 5);
		a_ptr->alloc_min = MIN(100, ((ap + 100) * 100 / max_power));

		/* Leave alloc_prob consistent with base art total rarity */
	}

	/* sanity check */
	if (a_ptr->alloc_prob > 99) a_ptr->alloc_prob = 99;
	if (a_ptr->alloc_prob < 1) a_ptr->alloc_prob = 1;

	file_putf(log_file, "New depths are min %d, max %d\n", a_ptr->alloc_min, a_ptr->alloc_max);
	file_putf(log_file, "Power-based alloc_prob is %d\n", a_ptr->alloc_prob);

	/* Restore some flags */
	if (a_ptr->tval == TV_LIGHT) of_on(a_ptr->flags, OF_NO_FUEL);
	if (a_idx < ART_MIN_NORMAL) of_on(a_ptr->flags, OF_INSTA_ART);

	/*
	 * Add OF_HIDE_TYPE to all artifacts with nonzero pval because we're
	 * too lazy to find out which ones need it and which ones don't.
	 */
	if (a_ptr->pval[DEFAULT_PVAL])
		of_on(a_ptr->flags, OF_HIDE_TYPE);

	/* Success */
	file_putf(log_file, ">>>>>>>>>>>>>>>>>>>>>>>>>> ARTIFACT COMPLETED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	file_putf(log_file, "Number of tries for artifact %d was: %d\n", a_idx, tries);
}

/*
 * Return TRUE if the whole set of random artifacts meets certain
 * criteria.  Return FALSE if we fail to meet those criteria (which will
 * restart the whole process).
 */
static bool artifacts_acceptable(void)
{
	int swords = 5, polearms = 5, blunts = 5, bows = 4;
	int bodies = 5, shields = 4, cloaks = 4, hats = 4;
	int gloves = 4, boots = 4;
	int i;

	for (i = ART_MIN_NORMAL; i < z_info->a_max; i++)
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
	    gloves > 0 || boots > 0)
	{
		if (verbose)
		{
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

			msg("Restarting generation process: not enough%s", types);
			file_putf(log_file, "Restarting generation process: not enough%s", types);
		}
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


static errr scramble(void)
{
	/* If our artifact set fails to meet certain criteria, we start over. */
	do
	{
		int a_idx;

		/* Generate all the artifacts. */
		for (a_idx = 1; a_idx < z_info->a_max; a_idx++)
		{
			scramble_artifact(a_idx);
		}
	} while (!artifacts_acceptable());	/* end of all artifacts */

	/* Success */
	return (0);
}


static errr do_randart_aux(bool full)
{
	errr result;

	/* Generate random names */
	if ((result = init_names()) != 0) return (result);

	if (full)
	{
		/* Randomize the artifacts */
		if ((result = scramble()) != 0) return (result);
	}

	/* Success */
	return (0);
}


/*
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
	Rand_quick = TRUE;

	/* Only do all the following if full randomization requested */
	if (full)
	{
		/* Allocate the various "original powers" arrays */
		base_power = C_ZNEW(z_info->a_max, s32b);
		base_item_level = C_ZNEW(z_info->a_max, byte);
		base_item_prob = C_ZNEW(z_info->a_max, byte);
		base_art_alloc = C_ZNEW(z_info->a_max, byte);
		baseprobs = C_ZNEW(z_info->k_max, s16b);
		base_freq = C_ZNEW(z_info->k_max, s16b);

		/* Open the log file for writing */
		if (verbose)
		{
			char buf[1024];
			path_build(buf, sizeof(buf), ANGBAND_DIR_USER,
				"randart.log");
			log_file = file_open(buf, MODE_WRITE, FTYPE_TEXT);
			if (!log_file)
			{
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
	if (full)
	{
		/* Just for fun, look at the frequencies on the finished items */
		/* Remove this prior to release */
		store_base_power();
		parse_frequencies();

		/* Close the log file */
		if (verbose)
		{
			if (!file_close(log_file))
			{
				msg("Error - can't close randart.log file.");
				exit(1);
			}
		}

		/* Free the "original powers" arrays */
		FREE(base_power);
		FREE(base_item_level);
		FREE(base_item_prob);
		FREE(base_art_alloc);
		FREE(baseprobs);
		FREE(base_freq);
	}

	/* When done, resume use of the Angband "complex" RNG. */
	Rand_quick = FALSE;

	return (err);
}
