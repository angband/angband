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
#include "object/tvalsval.h"
#include "init.h"
#include "randname.h"

/*
 * Original random artifact generator (randart) by Greg Wooledge.
 * Updated by Chris Carr / Chris Robertson 2001-2009.
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
 * Numerical index values for the different learned probabilities
 * These are to make the code more readable.
 * ToDo: turn these into an enum
 */

#define ART_IDX_BOW_SHOTS 0
#define ART_IDX_BOW_MIGHT 1
#define ART_IDX_WEAPON_HIT 2
#define ART_IDX_WEAPON_DAM 3
#define ART_IDX_NONWEAPON_HIT 4
#define ART_IDX_NONWEAPON_DAM 5
#define ART_IDX_NONWEAPON_HIT_DAM 6

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
#define ART_IDX_GEN_LITE 50
#define ART_IDX_GEN_SINV 51
#define ART_IDX_GEN_ESP 52
#define ART_IDX_GEN_SDIG 53
#define ART_IDX_GEN_REGEN 54
#define ART_IDX_GEN_LRES 55
#define ART_IDX_GEN_RPOIS 56
#define ART_IDX_GEN_RFEAR 57
#define ART_IDX_GEN_RLITE 58
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

/* Supercharged abilities - treated differently in algorithm */

#define ART_IDX_MELEE_DICE_SUPER 70
#define ART_IDX_BOW_SHOTS_SUPER 71
#define ART_IDX_BOW_MIGHT_SUPER 72
#define ART_IDX_GEN_SPEED_SUPER 73
#define ART_IDX_MELEE_BLOWS_SUPER 77

/* Aggravation - weapon and nonweapon */
#define ART_IDX_WEAPON_AGGR 74
#define ART_IDX_NONWEAPON_AGGR 75

/* Total of abilities */
#define ART_IDX_TOTAL 78

/* Tallies of different ability types */
/* ToDo: use N_ELEMENTS for these */
#define ART_IDX_BOW_COUNT 2
#define ART_IDX_WEAPON_COUNT 3
#define ART_IDX_NONWEAPON_COUNT 4
#define ART_IDX_MELEE_COUNT 9
#define ART_IDX_ALLARMOR_COUNT 1
#define ART_IDX_BOOT_COUNT 4
#define ART_IDX_GLOVE_COUNT 3
#define ART_IDX_HELM_COUNT 6
#define ART_IDX_SHIELD_COUNT 2
#define ART_IDX_CLOAK_COUNT 2
#define ART_IDX_ARMOR_COUNT 7
#define ART_IDX_GEN_COUNT 30
#define ART_IDX_HIGH_RESIST_COUNT 12

/* Arrays of indices by item type, used in frequency generation */

static s16b art_idx_bow[] =
	{ART_IDX_BOW_SHOTS, ART_IDX_BOW_MIGHT};
static s16b art_idx_weapon[] =
	{ART_IDX_WEAPON_HIT, ART_IDX_WEAPON_DAM, ART_IDX_WEAPON_AGGR};
static s16b art_idx_nonweapon[] =
	{ART_IDX_NONWEAPON_HIT, ART_IDX_NONWEAPON_DAM, ART_IDX_NONWEAPON_HIT_DAM,
	ART_IDX_NONWEAPON_AGGR};
static s16b art_idx_melee[] =
	{ART_IDX_MELEE_BLESS, ART_IDX_MELEE_BRAND, ART_IDX_MELEE_SLAY, ART_IDX_MELEE_SINV,
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
	ART_IDX_GEN_FEATHER, ART_IDX_GEN_LITE, ART_IDX_GEN_SINV,
	ART_IDX_GEN_ESP, ART_IDX_GEN_SDIG, ART_IDX_GEN_REGEN,
	ART_IDX_GEN_LRES, ART_IDX_GEN_RPOIS, ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLITE, ART_IDX_GEN_RDARK, ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF, ART_IDX_GEN_RSOUND, ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS, ART_IDX_GEN_RNETHER, ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN, ART_IDX_GEN_AC, ART_IDX_GEN_TUNN};
static s16b art_idx_high_resist[] =
	{ART_IDX_GEN_RPOIS, ART_IDX_GEN_RFEAR,
	ART_IDX_GEN_RLITE, ART_IDX_GEN_RDARK, ART_IDX_GEN_RBLIND,
	ART_IDX_GEN_RCONF, ART_IDX_GEN_RSOUND, ART_IDX_GEN_RSHARD,
	ART_IDX_GEN_RNEXUS, ART_IDX_GEN_RNETHER, ART_IDX_GEN_RCHAOS,
	ART_IDX_GEN_RDISEN};

/* Initialize the data structures for learned probabilities */
static s16b artprobs[ART_IDX_TOTAL];
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
 * Working array for holding frequency values - global to avoid repeated
 * allocation of memory
 */
static s16b art_freq[ART_IDX_TOTAL];

/*
 * Mean start and increment values for to_hit, to_dam and AC.  Update these
 * if the algorithm changes.  They are used in frequency generation.
 */
static s16b mean_hit_increment = 3;
static s16b mean_dam_increment = 3;
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
static byte *base_art_rarity;

/* Global just for convenience. */
static int verbose = 1;

/*
 * Use W. Sheldon Simms' random name generator.
 */
static errr init_names(void)
{
	char buf[BUFLEN];
	size_t name_size;
	char *a_base;
	char *a_next;
	int i;

	/* Temporary space for names, while reading and randomizing them. */
	char **names;

	/* Allocate the "names" array */
	/* ToDo: Make sure the memory is freed correctly in case of errors */
	names = C_ZNEW(z_info->a_max, char *);

	for (i = 0; i < z_info->a_max; i++)
	{
		char word[MAX_NAME_LEN + 1];
		randname_make(RANDNAME_TOLKIEN, MIN_NAME_LEN, MAX_NAME_LEN, word, sizeof word);
		word[0] = toupper((unsigned char) word[0]);

		if (one_in_(3))
			strnfmt(buf, sizeof(buf), "'%s'", word);
		else
			strnfmt(buf, sizeof(buf), "of %s", word);

		names[i] = string_make(buf);
	}

	/* Special cases -- keep these three names separate. */
	string_free(names[ART_POWER - 1]);
	string_free(names[ART_GROND - 1]);
	string_free(names[ART_MORGOTH - 1]);
	names[ART_POWER - 1] = string_make("of Power (The One Ring)");
	names[ART_GROND - 1] = string_make("'Grond'");
	names[ART_MORGOTH - 1] = string_make("of Morgoth");

	/* Convert our names array into an a_name structure for later use. */
	name_size = 0;

	for (i = 1; i < z_info->a_max; i++)
	{
		name_size += strlen(names[i-1]) + 2;	/* skip first char */
	}

	a_base = C_ZNEW(name_size, char);

	a_next = a_base + 1;	/* skip first char */

	for (i = 1; i < z_info->a_max; i++)
	{
		my_strcpy(a_next, names[i-1], name_size - 1);
		if (a_info[i].tval > 0)		/* skip unused! */
			a_info[i].name = a_next - a_base;
		a_next += strlen(names[i-1]) + 1;
	}

	/* Free the old names */
	FREE(a_name);

	for (i = 0; i < z_info->a_max; i++)
	{
		string_free(names[i]);
	}

	/* Free the "names" array */
	FREE(names);

	/* Store the names */
	a_name = a_base;
	a_head.name_ptr = a_base;
	a_head.name_size = name_size;

	/* Success */
	return (0);
}

/*
 * Return the artifact power, by generating a "fake" object based on the
 * artifact, and calling the common object_power function
 */ 
static s32b artifact_power(int a_idx)
{
	object_type obj;
	
	LOG_PRINT("********** ENTERING EVAL POWER ********\n");
	LOG_PRINT1("Artifact index is %d\n", a_idx);
	
	if(!make_fake_artifact(&obj, a_idx))
	{
		return 0;
	}

	return object_power(&obj, verbose, log_file);
}


/*
 * Store the original artifact power ratings as a baseline
 */
static void store_base_power (void)
{
	int i;
	artifact_type *a_ptr;
	object_kind *k_ptr;
	s16b k_idx;

	for(i = 0; i < z_info->a_max; i++)
	{
		base_power[i] = artifact_power(i);
	}

	for(i = 0; i < z_info->a_max; i++)
	{
		/* Kinds array was populated in the above step */
		a_ptr = &a_info[i];
		k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
		k_ptr = &k_info[k_idx];
		base_item_level[i] = k_ptr->level;
		base_item_prob[i] = k_ptr->alloc_prob;
		base_art_rarity[i] = a_ptr->rarity;
	}

	/* Store the number of different types, for use later */
	/* ToDo: replace this with base item freq parsing */
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
		default:
			art_other_total++;
		}
	}
	art_total = art_melee_total + art_bow_total + art_armor_total +
	            art_shield_total + art_cloak_total + art_headgear_total +
	            art_glove_total + art_boot_total + art_other_total;
}

static struct item_choice {
	int threshold;
	int tval;
	char *report;
} item_choices[] = {
	{  7, TV_BOW,           "a missile weapon"},
	{ 10, TV_DIGGING,       "a digger"},
	{ 20, TV_HAFTED,        "a blunt weapon"},
	{ 32, TV_SWORD,         "an edged weapon"},
	{ 42, TV_POLEARM,       "a polearm"},
	{ 64, TV_SOFT_ARMOR,    "body armour"},
	{ 71, TV_BOOTS,         "footwear"},
	{ 78, TV_GLOVES,        "gloves"},
	{ 87, TV_HELM,          "a hat"},
	{ 94, TV_SHIELD,        "a shield"},
	{100, TV_CLOAK,         "a cloak"}
};

/*
 * Randomly select a base item type (tval,sval).  Assign the various fields
 * corresponding to that choice.
 *
 * The return value gives the index of the new item type.  The method is
 * passed a pointer to a rarity value in order to return the rarity of the
 * new item.
 *
 * ToDo: complete rewrite removing the hard-coded tables above and below
 */
static s16b choose_item(int a_idx)
{
	artifact_type *a_ptr = &a_info[a_idx];
	int tval;
	int sval = 0;
	object_kind *k_ptr;
	int r, i;
	s16b k_idx, r2;
	byte target_level;

	/*
	 * Look up the original artifact's base object kind to get level.
	 */
	k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
	k_ptr = &k_info[k_idx];
	target_level = base_item_level[a_idx];
	LOG_PRINT1("Base item level is: %d\n", target_level);

	/*
	 * If the artifact level is higher then we use that instead.
	 */

	if(a_ptr->level > target_level) target_level = a_ptr->level;
	LOG_PRINT1("Target level is: %d\n", target_level);

	/*
	 * Pick a category (tval) of weapon randomly.  Within each tval, roll
	 * an sval (specific item) based on the target level.  The number we
	 * roll should be a bell curve.  The mean and standard variation of the
	 * bell curve are based on the target level; the distribution of
	 * kinds versus the bell curve is hand-tweaked. :-(
	 */
	r = randint0(100);
	r2 = Rand_normal(target_level * 2, target_level);
	LOG_PRINT2("r is: %d, r2 is: %d\n", r, r2);

	i = 0;
	while (r >= item_choices[i].threshold)
	{
		i++;
	}

	tval = item_choices[i].tval;
	LOG_PRINT1("Creating %s\n", item_choices[i].report);

	switch (tval)
	{
	case TV_BOW:
		if (r2 <10) sval = SV_SLING;
		else if (r2 < 30) sval = SV_SHORT_BOW;
		else if (r2 < 60) sval = SV_LONG_BOW;
		else if (r2 < 90) sval = SV_LIGHT_XBOW;
		else sval = SV_HEAVY_XBOW;
		break;

	case TV_DIGGING:
		if (r2 < 40) sval = SV_SHOVEL;
		else if (r2 < 80) sval = SV_PICK;
		else sval = SV_MATTOCK;
		break;

	case TV_HAFTED:
		if (r2 < 5) sval = SV_WHIP;
		else if (r2 < 10) sval = SV_MACE;
		else if (r2 < 17) sval = SV_WAR_HAMMER;
		else if (r2 < 25) sval = SV_QUARTERSTAFF;
		else if (r2 < 35) sval = SV_MORNING_STAR;
		else if (r2 < 48) sval = SV_FLAIL;
		else if (r2 < 63) sval = SV_LEAD_FILLED_MACE;
		else if (r2 < 80) sval = SV_BALL_AND_CHAIN;
		else if (r2 < 100) sval = SV_GREAT_HAMMER;
		else if (r2 < 120) sval = SV_MAUL;
		else if (r2 < 135) sval = SV_TWO_HANDED_GREAT_FLAIL;
		else sval = SV_MACE_OF_DISRUPTION;
		break;

	case TV_SWORD:
		if (r2 < 5) sval = SV_DAGGER;
		else if (r2 < 9) sval = SV_MAIN_GAUCHE;
		else if (r2 < 14) sval = SV_RAPIER;
		else if (r2 < 20) sval = SV_SHORT_SWORD;
		else if (r2 < 27) sval = SV_CUTLASS;
		else if (r2 < 35) sval = SV_TULWAR;
		else if (r2 < 44) sval = SV_BROAD_SWORD;
		else if (r2 < 54) sval = SV_LONG_SWORD;
		else if (r2 < 65) sval = SV_SCIMITAR;
		else if (r2 < 80) sval = SV_BASTARD_SWORD;
		else if (r2 < 100) sval = SV_KATANA;
		else if (r2 < 120) sval = SV_ZWEIHANDER;
		else if (r2 < 135) sval = SV_EXECUTIONERS_SWORD;
		else sval = SV_BLADE_OF_CHAOS;
		break;

	case TV_POLEARM:
		if (r2 < 5) sval = SV_SPEAR;
		else if (r2 < 9) sval = SV_TRIDENT;
		else if (r2 < 14) sval = SV_LANCE;
		else if (r2 < 20) sval = SV_AWL_PIKE;
		else if (r2 < 27) sval = SV_PIKE;
		else if (r2 < 35) sval = SV_BEAKED_AXE;
		else if (r2 < 44) sval = SV_BROAD_AXE;
		else if (r2 < 54) sval = SV_BATTLE_AXE;
		else if (r2 < 65) sval = SV_GLAIVE;
		else if (r2 < 76) sval = SV_LUCERNE_HAMMER;
		else if (r2 < 88) sval = SV_HALBERD;
		else if (r2 < 102) sval = SV_GREAT_AXE;
		else if (r2 < 120) sval = SV_SCYTHE;
		else if (r2 < 135) sval = SV_LOCHABER_AXE;
		else sval = SV_SCYTHE_OF_SLICING;
		break;

	case TV_SOFT_ARMOR:
		/* Hack - multiply r2 by 3/2 (armor has deeper base levels than other types) */
		r2 = sign(r2) * ((ABS(r2) * 3) / 2);

		/* Adjust tval, as all armor is done together */
		if (r2 < 30) tval = TV_SOFT_ARMOR;
		else if (r2 < 116) tval = TV_HARD_ARMOR;
		else tval = TV_DRAG_ARMOR;

		/* Soft stuff. */
		if (r2 < 5) sval = SV_ROBE;
		else if (r2 < 11) sval = SV_SOFT_LEATHER_ARMOUR;
		else if (r2 < 18) sval = SV_STUDDED_LEATHER_ARMOUR;
		else if (r2 < 24) sval = SV_HARD_LEATHER_ARMOUR;
		else if (r2 < 30) sval = SV_LEATHER_SCALE_MAIL;

		/* Hard stuff. */
		else if (r2 < 38) sval = SV_METAL_SCALE_MAIL;
		else if (r2 < 45) sval = SV_CHAIN_MAIL;
		else if (r2 < 52) sval = SV_AUGMENTED_CHAIN_MAIL;
		else if (r2 < 59) sval = SV_BAR_CHAIN_MAIL;
		else if (r2 < 66) sval = SV_METAL_BRIGANDINE_ARMOUR;
		else if (r2 < 73) sval = SV_PARTIAL_PLATE_ARMOUR;
		else if (r2 < 80) sval = SV_METAL_LAMELLAR_ARMOUR;
		else if (r2 < 87) sval = SV_FULL_PLATE_ARMOUR;
		else if (r2 < 94) sval = SV_RIBBED_PLATE_ARMOUR;
		else if (r2 < 101) sval = SV_MITHRIL_CHAIN_MAIL;
		else if (r2 < 108) sval = SV_MITHRIL_PLATE_MAIL;
		else if (r2 < 116) sval = SV_ADAMANTITE_PLATE_MAIL;

		/* DSM - CC 18/8/01 */
		else if (r2 < 121) sval = SV_DRAGON_BLACK;
		else if (r2 < 126) sval = SV_DRAGON_BLUE;
		else if (r2 < 131) sval = SV_DRAGON_WHITE;
		else if (r2 < 136) sval = SV_DRAGON_RED;
		else if (r2 < 142) sval = SV_DRAGON_GREEN;
		else if (r2 < 148) sval = SV_DRAGON_MULTIHUED;
		else if (r2 < 154) sval = SV_DRAGON_SHINING;
		else if (r2 < 160) sval = SV_DRAGON_LAW;
		else if (r2 < 167) sval = SV_DRAGON_BRONZE;
		else if (r2 < 173) sval = SV_DRAGON_GOLD;
		else if (r2 < 178) sval = SV_DRAGON_CHAOS;
		else if (r2 < 183) sval = SV_DRAGON_BALANCE;
		else sval = SV_DRAGON_POWER;
		break;

	case TV_BOOTS:
		if (r2 < 15) sval = SV_PAIR_OF_LEATHER_SANDALS;
		else if (r2 < 30) sval = SV_PAIR_OF_LEATHER_BOOTS;
		else if (r2 < 50) sval = SV_PAIR_OF_IRON_SHOD_BOOTS;
		else if (r2 < 70) sval = SV_PAIR_OF_STEEL_SHOD_BOOTS; 
		else if (r2 < 90) sval = SV_PAIR_OF_MITHRIL_SHOD_BOOTS; 
		else sval = SV_PAIR_OF_ETHEREAL_SLIPPERS;
		break;

	case TV_GLOVES:
		if (r2 < 15) sval = SV_SET_OF_LEATHER_GLOVES;
		else if (r2 < 30) sval = SV_SET_OF_GAUNTLETS;
		else if (r2 < 60) sval = SV_SET_OF_MITHRIL_GAUNTLETS;
		else if (r2 < 90) sval = SV_SET_OF_ALCHEMISTS_GLOVES;
		else sval = SV_SET_OF_CAESTUS;
		break;

	case TV_HELM:
		/* Adjust tval to handle crowns and helms here */
		if (r2 < 50) tval = TV_HELM; else tval = TV_CROWN;

		if (r2 < 9) sval = SV_HARD_LEATHER_CAP;
		else if (r2 < 20) sval = SV_METAL_CAP;
		else if (r2 < 35) sval = SV_IRON_HELM;
		else if (r2 < 50) sval = SV_STEEL_HELM;

		else if (r2 < 70) sval = SV_IRON_CROWN;
		else if (r2 < 90) sval = SV_GOLDEN_CROWN;
		else sval = SV_JEWEL_ENCRUSTED_CROWN;
		break;

	case TV_SHIELD:
		if (r2 < 15) sval = SV_WICKER_SHIELD;
		else if (r2 < 40) sval = SV_SMALL_METAL_SHIELD;
		else if (r2 < 70) sval = SV_LEATHER_SHIELD;
		else if (r2 < 95) sval = SV_LARGE_METAL_SHIELD;
		else sval = SV_MITHRIL_SHIELD;
		break;

	case TV_CLOAK:
		if (r2 < 40) sval = SV_CLOAK;
		else if (r2 < 70) sval = SV_FUR_CLOAK;
		else if (r2 < 100) sval = SV_ELVEN_CLOAK;
		else sval = SV_ETHEREAL_CLOAK;
		break;
	}
/* CC debug hacks */
	LOG_PRINT2("tval is %d, sval is %d\n", tval, sval);
	k_idx = lookup_kind(tval, sval);
	k_ptr = &k_info[k_idx];
	LOG_PRINT2("k_idx is %d, k_ptr->alloc_prob is %d\n", k_idx, k_ptr->alloc_prob);
/* CC end - but need a flush here */
	a_ptr->tval = k_ptr->tval;
	a_ptr->sval = k_ptr->sval;
	a_ptr->pval = k_ptr->pval;
	a_ptr->to_h = k_ptr->to_h;
	a_ptr->to_d = k_ptr->to_d;
	a_ptr->to_a = k_ptr->to_a;
	a_ptr->ac = k_ptr->ac;
	a_ptr->dd = k_ptr->dd;
	a_ptr->ds = k_ptr->ds;
	a_ptr->weight = k_ptr->weight;
	a_ptr->flags1 = k_ptr->flags1;
	a_ptr->flags2 = k_ptr->flags2;
	a_ptr->flags3 = k_ptr->flags3;

	/*
	 * Dragon armor: remove activation flag.  This is because the current
	 * code doesn't support standard DSM activations for artifacts very
	 * well.  If it gets an activation from the base artifact it will be
	 * reset later.
	 * ToDo: proper random activations
	 */
	if (a_ptr->tval == TV_DRAG_ARMOR)
		a_ptr->flags3 &= ~TR3_ACTIVATE;

	/* Artifacts ignore everything */
	a_ptr->flags3 |= TR3_IGNORE_MASK;

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
			LOG_PRINT2("Assigned basic stats, to_hit: %d, to_dam: %d\n", a_ptr->to_h, a_ptr->to_d);
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
			LOG_PRINT1("Assigned basic stats, AC bonus: %d\n", a_ptr->to_a);
			break;
	}

	/* Done - return the index of the new object kind. */
	return k_idx;
}


/*
 * We've just added an ability which uses the pval bonus.  Make sure it's
 * not zero.  If it's currently negative, leave it negative (heh heh).
 */
static void do_pval(artifact_type *a_ptr)
{
	int factor = 1;
	/* Track whether we have blows, might or shots on this item */
	if (a_ptr->flags1 & TR1_BLOWS) factor++;
	if (a_ptr->flags1 & TR1_MIGHT) factor++;
	if (a_ptr->flags1 & TR1_SHOTS) factor++;

	if (a_ptr->pval == 0)
	{
		/* Blows, might, shots handled separately */
		if (factor > 1)
		{
			a_ptr->pval = (s16b)randint1(2);
			/* Give it a shot at +3 */
			if (INHIBIT_STRONG) a_ptr->pval = 3;
		}
		else a_ptr->pval = (s16b)randint1(4);
		LOG_PRINT1("Assigned initial pval, value is: %d\n", a_ptr->pval);
	}
	else if (a_ptr->pval < 0)
	{
		if (one_in_(2))
		{
			a_ptr->pval--;
			LOG_PRINT1("Decreasing pval by 1, new value is: %d\n", a_ptr->pval);
		}
	}
	else if (one_in_(a_ptr->pval * factor))
	{
		/*
		 * CR: made this a bit rarer and diminishing with higher pval -
		 * also rarer if item has blows/might/shots already
		 */
		a_ptr->pval++;
		LOG_PRINT1("Increasing pval by 1, new value is: %d\n", a_ptr->pval);
	}
}


static void remove_contradictory(artifact_type *a_ptr)
{
	if (a_ptr->flags3 & TR3_AGGRAVATE) a_ptr->flags1 &= ~(TR1_STEALTH);
	if (a_ptr->flags2 & TR2_IM_ACID) a_ptr->flags2 &= ~(TR2_RES_ACID);
	if (a_ptr->flags2 & TR2_IM_ELEC) a_ptr->flags2 &= ~(TR2_RES_ELEC);
	if (a_ptr->flags2 & TR2_IM_FIRE) a_ptr->flags2 &= ~(TR2_RES_FIRE);
	if (a_ptr->flags2 & TR2_IM_COLD) a_ptr->flags2 &= ~(TR2_RES_COLD);

	if (a_ptr->pval < 0)
	{
		if (a_ptr->flags1 & TR1_STR) a_ptr->flags2 &= ~(TR2_SUST_STR);
		if (a_ptr->flags1 & TR1_INT) a_ptr->flags2 &= ~(TR2_SUST_INT);
		if (a_ptr->flags1 & TR1_WIS) a_ptr->flags2 &= ~(TR2_SUST_WIS);
		if (a_ptr->flags1 & TR1_DEX) a_ptr->flags2 &= ~(TR2_SUST_DEX);
		if (a_ptr->flags1 & TR1_CON) a_ptr->flags2 &= ~(TR2_SUST_CON);
		if (a_ptr->flags1 & TR1_CHR) a_ptr->flags2 &= ~(TR2_SUST_CHR);
		a_ptr->flags1 &= ~(TR1_BLOWS);
	}

	if (a_ptr->flags3 & TR3_LIGHT_CURSE) a_ptr->flags3 &= ~(TR3_BLESSED);
	if (a_ptr->flags1 & TR1_KILL_DRAGON) a_ptr->flags1 &= ~(TR1_SLAY_DRAGON);
	if (a_ptr->flags1 & TR1_KILL_DEMON) a_ptr->flags1 &= ~(TR1_SLAY_DEMON);
	if (a_ptr->flags1 & TR1_KILL_UNDEAD) a_ptr->flags1 &= ~(TR1_SLAY_UNDEAD);
	if (a_ptr->flags3 & TR3_DRAIN_EXP) a_ptr->flags3 &= ~(TR3_HOLD_LIFE);
}

/*
 * Adjust the parsed frequencies for any peculiarities of the
 * algorithm.  For example, if stat bonuses and sustains are
 * being added in a correlated fashion, it will tend to push
 * the frequencies up for both of them.  In this method we
 * compensate for cases like this by applying corrective
 * scaling.
 */

static void adjust_freqs()
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
	if (artprobs[ART_IDX_GEN_SPEED_SUPER] < 3)
		artprobs[ART_IDX_GEN_SPEED_SUPER] = 3;
	if (artprobs[ART_IDX_GEN_AC] < 5)
		artprobs[ART_IDX_GEN_AC] = 5;
	if (artprobs[ART_IDX_GEN_TUNN] < 5)
		artprobs[ART_IDX_GEN_TUNN] = 5;

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
	int i;
	const artifact_type *a_ptr;
	object_kind *k_ptr;
	s32b temp, temp2;
	s16b k_idx;


	LOG_PRINT("\n****** BEGINNING GENERATION OF FREQUENCIES\n\n");

	/* Zero the frequencies */

	for(i = 0; i < ART_IDX_TOTAL; i++)
	{
		artprobs[i] = 0;
	}

	/* Go through the list of all artifacts */

	for(i = 0; i < z_info->a_max; i++)
	{
		LOG_PRINT1("Current artifact index is %d\n", i);

		a_ptr = &a_info[i];

		/* Special cases -- don't parse these! */
		if ((i == ART_POWER) ||
			(i == ART_GROND) ||
			(i == ART_MORGOTH))
			continue;

		/* Also don't parse cursed items */
		if (base_power[i] < 0) continue;

		/* Get a pointer to the base item for this artifact */
		k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
		k_ptr = &k_info[k_idx];

		/* Count up the abilities for this artifact */

		if (a_ptr->tval == TV_BOW)
		{
			if(a_ptr->flags1 & TR1_SHOTS)
			{
				/* Do we have 3 or more extra shots? (Unlikely) */
				if(a_ptr->pval > 2)
				{
					LOG_PRINT("Adding 1 for supercharged shots (3 or more!)\n");

					(artprobs[ART_IDX_BOW_SHOTS_SUPER])++;
				}
				else {
					LOG_PRINT("Adding 1 for extra shots\n");

					(artprobs[ART_IDX_BOW_SHOTS])++;
				}
			}
			if(a_ptr->flags1 & TR1_MIGHT)
			{
				/* Do we have 3 or more extra might? (Unlikely) */
				if(a_ptr->pval > 2)
				{
					LOG_PRINT("Adding 1 for supercharged might (3 or more!)\n");

					(artprobs[ART_IDX_BOW_MIGHT_SUPER])++;
				}
				else {
					LOG_PRINT("Adding 1 for extra might\n");

					(artprobs[ART_IDX_BOW_MIGHT])++;
				}
			}
		}

		/* Handle hit / dam ratings - are they higher than normal? */
		/* Also handle other weapon/nonweapon abilities */
		if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
			a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
			a_ptr->tval == TV_SWORD)
		{
			if (a_ptr->to_h - k_ptr->to_h - mean_hit_startval > 0)
			{
				temp = (a_ptr->to_d - k_ptr->to_d - mean_dam_startval) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-hit bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_HIT]) += temp;
				}
			}
			else if (a_ptr->to_h - k_ptr->to_h - mean_hit_startval < 0)
			{
				temp = ( -(a_ptr->to_d - k_ptr->to_d - mean_dam_startval) ) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Subtracting %d instances of extra to-hit bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_HIT]) -= temp;
				}
			}
			if (a_ptr->to_d - k_ptr->to_d - mean_dam_startval > 0)
			{
				temp = (a_ptr->to_d - k_ptr->to_d - mean_dam_startval) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-dam bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_DAM]) += temp;
				}
			}
			else if (a_ptr->to_d - k_ptr->to_d - mean_dam_startval < 0)
			{
				temp = ( -(a_ptr->to_d - k_ptr->to_d - mean_dam_startval)) /
					mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Subtracting %d instances of extra to-dam bonus for weapon\n", temp);

					(artprobs[ART_IDX_WEAPON_DAM]) -= temp;
				}
			}

			/* Aggravation */
			if (a_ptr->flags3 & TR3_AGGRAVATE)
			{
				LOG_PRINT("Adding 1 for aggravation - weapon\n");
				(artprobs[ART_IDX_WEAPON_AGGR])++;
			}

			/* End weapon stuff */
		}
		else
		{
			if ( (a_ptr->to_h - k_ptr->to_h > 0) &&
				(a_ptr->to_h - k_ptr->to_h == a_ptr->to_d - k_ptr->to_d) )
			{
				/* Special case: both hit and dam bonuses present and equal */
				temp = (a_ptr->to_d - k_ptr->to_d) / mean_dam_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra to-hit and to-dam bonus for non-weapon\n", temp);

					(artprobs[ART_IDX_NONWEAPON_HIT_DAM]) += temp;
				}
			}
			else
			{
				/* Uneven bonuses - handle separately */
				if (a_ptr->to_h - k_ptr->to_h > 0)
				{
					temp = (a_ptr->to_d - k_ptr->to_d) / mean_dam_increment;
					if (temp > 0)
					{
						LOG_PRINT1("Adding %d instances of extra to-hit bonus for non-weapon\n", temp);

						(artprobs[ART_IDX_NONWEAPON_HIT]) += temp;
					}
				}
				if (a_ptr->to_d - k_ptr->to_d > 0)
				{
					temp = (a_ptr->to_d - k_ptr->to_d) / mean_dam_increment;
					if (temp > 0)
					{
						LOG_PRINT1("Adding %d instances of extra to-dam bonus for non-weapon\n", temp);

						(artprobs[ART_IDX_NONWEAPON_DAM]) += temp;
					}
				}
			}

			/* Aggravation */
			if (a_ptr->flags3 & TR3_AGGRAVATE)
			{
				LOG_PRINT("Adding 1 for aggravation - nonweapon\n");
				(artprobs[ART_IDX_NONWEAPON_AGGR])++;
			}

		}

		if (a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
			a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD)
		{
			/* Blessed weapon Y/N */

			if(a_ptr->flags3 & TR3_BLESSED)
			{
				LOG_PRINT("Adding 1 for blessed weapon\n");

				(artprobs[ART_IDX_MELEE_BLESS])++;
			}

			/*
			 * Brands or slays - count all together
			 * We will need to add something here unless the weapon has
			 * nothing at all
			 */

			if (a_ptr->flags1 & (TR1_SLAY_MASK | TR1_BRAND_MASK | TR1_KILL_MASK))
			{
				/* We have some brands or slays - count them */
				temp = 0;
				temp2 = 0;
				const slay_t *s_ptr;

				for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++ )
				{
					if (a_ptr->flags1 & s_ptr->slay_flag)
					{
						if (s_ptr->slay_flag & (TR1_SLAY_MASK | TR1_KILL_MASK)) temp++;
						if (s_ptr->slay_flag & TR1_BRAND_MASK) temp2++;
					}
				}
				LOG_PRINT1("Adding %d for slays\n", temp);
				LOG_PRINT1("Adding %d for brands\n", temp2);

				/* Add these to the frequency count */
				artprobs[ART_IDX_MELEE_BRAND] += temp2;
				artprobs[ART_IDX_MELEE_SLAY] += temp;
			}

			/* See invisible? */
			if(a_ptr->flags3 & TR3_SEE_INVIS)
			{
				LOG_PRINT("Adding 1 for see invisible (weapon case)\n");

				(artprobs[ART_IDX_MELEE_SINV])++;
			}

			/* Does this weapon have extra blows? */
			if (a_ptr->flags1 & TR1_BLOWS)
			{
				/* Do we have 3 or more extra blows? (Unlikely) */
				if(a_ptr->pval > 2)
				{
					LOG_PRINT("Adding 1 for supercharged blows (3 or more!)\n");
					(artprobs[ART_IDX_MELEE_BLOWS_SUPER])++;
				}
				else {
					LOG_PRINT("Adding 1 for extra blows\n");
					(artprobs[ART_IDX_MELEE_BLOWS])++;
				}
			}
			
			/* Does this weapon have an unusual bonus to AC? */
			if ( (a_ptr->to_a - k_ptr->to_a) > 0)
			{
				temp = (a_ptr->to_a - k_ptr->to_a) / mean_ac_increment;
				if (temp > 0)
				{
					LOG_PRINT1("Adding %d instances of extra AC bonus for weapon\n", temp);

					(artprobs[ART_IDX_MELEE_AC]) += temp;
				}
			}

			/* Check damage dice - are they more than normal? */
			if (a_ptr->dd > k_ptr->dd)
			{
				/* Difference of 3 or more? */
				if ( (a_ptr->dd - k_ptr->dd) > 2)
				{
					LOG_PRINT("Adding 1 for super-charged damage dice!\n");

					(artprobs[ART_IDX_MELEE_DICE_SUPER])++;
				}
				else
				{
					LOG_PRINT("Adding 1 for extra damage dice.\n");

					(artprobs[ART_IDX_MELEE_DICE])++;
				}
			}

			/* Check weight - is it different from normal? */
			if (a_ptr->weight != k_ptr->weight)
			{
				LOG_PRINT("Adding 1 for unusual weight.\n");

				(artprobs[ART_IDX_MELEE_WEIGHT])++;
			}

			/* Check for tunnelling ability */
			if (a_ptr->flags1 & TR1_TUNNEL)
			{
				LOG_PRINT("Adding 1 for tunnelling bonus.\n");

				(artprobs[ART_IDX_MELEE_TUNN])++;
			}

			/* End of weapon-specific stuff */
		}
		else
		{
			/* Check for tunnelling ability */
			if (a_ptr->flags1 & TR1_TUNNEL)
			{
				LOG_PRINT("Adding 1 for tunnelling bonus - general.\n");

				(artprobs[ART_IDX_GEN_TUNN])++;
			}
		}
		/*
		 * Count up extra AC bonus values.
		 * Could also add logic to subtract for lower values here, but it's
		 * probably not worth the trouble since it's so rare.
		 */

		if ( (a_ptr->to_a - k_ptr->to_a - mean_ac_startval) > 0)
		{
			temp = (a_ptr->to_a - k_ptr->to_a - mean_ac_startval) /
				mean_ac_increment;
			if (temp > 0)
			{
				if (a_ptr->tval == TV_BOOTS)
				{
					LOG_PRINT1("Adding %d for AC bonus - boots\n", temp);
					(artprobs[ART_IDX_BOOT_AC]) += temp;
				}
				else if (a_ptr->tval == TV_GLOVES)
				{
					LOG_PRINT1("Adding %d for AC bonus - gloves\n", temp);
					(artprobs[ART_IDX_GLOVE_AC]) += temp;
				}
				else if (a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
				{
					LOG_PRINT1("Adding %d for AC bonus - headgear\n", temp);
					(artprobs[ART_IDX_HELM_AC]) += temp;
				}
				else if (a_ptr->tval == TV_SHIELD)
				{
					LOG_PRINT1("Adding %d for AC bonus - shield\n", temp);
					(artprobs[ART_IDX_SHIELD_AC]) += temp;
				}
				else if (a_ptr->tval == TV_CLOAK)
				{
					LOG_PRINT1("Adding %d for AC bonus - cloak\n", temp);
					(artprobs[ART_IDX_CLOAK_AC]) += temp;
				}
				else if (a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
					a_ptr->tval == TV_DRAG_ARMOR)
				{
					LOG_PRINT1("Adding %d for AC bonus - body armor\n", temp);
					(artprobs[ART_IDX_ARMOR_AC]) += temp;
				}
				else
				{
					LOG_PRINT1("Adding %d for AC bonus - general\n", temp);
					(artprobs[ART_IDX_GEN_AC]) += temp;
				}
			}
		}

		/* Generic armor abilities */
		if ( a_ptr->tval == TV_BOOTS || a_ptr->tval == TV_GLOVES ||
			a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN ||
			a_ptr->tval == TV_SHIELD || a_ptr->tval == TV_CLOAK ||
			a_ptr->tval == TV_SOFT_ARMOR || a_ptr->tval == TV_HARD_ARMOR ||
			a_ptr->tval == TV_DRAG_ARMOR)
		{
			/* Check weight - is it different from normal? */
			/* ToDo: count higher and lower separately */
			if (a_ptr->weight != k_ptr->weight)
			{
				LOG_PRINT("Adding 1 for unusual weight.\n");

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

		if ( (a_ptr->flags1 & TR1_STR) || (a_ptr->flags1 & TR1_INT) ||
			(a_ptr->flags1 & TR1_WIS) || (a_ptr->flags1 & TR1_DEX) ||
			(a_ptr->flags1 & TR1_CON) || (a_ptr->flags1 & TR1_CHR) )
		{
			/* Stat bonus case.  Add up the number of individual bonuses */

			temp = 0;
			if (a_ptr->flags1 & TR1_STR) temp++;
			if (a_ptr->flags1 & TR1_INT) temp++;
			if (a_ptr->flags1 & TR1_WIS) temp++;
			if (a_ptr->flags1 & TR1_DEX) temp++;
			if (a_ptr->flags1 & TR1_CON) temp++;
			if (a_ptr->flags1 & TR1_CHR) temp++;

			/* Handle a few special cases separately. */

			if((a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN) &&
				((a_ptr->flags1 & TR1_WIS) || (a_ptr->flags1 & TR1_INT)))
			{
				/* Handle WIS and INT on helms and crowns */
				if(a_ptr->flags1 & TR1_WIS)
				{
					LOG_PRINT("Adding 1 for WIS bonus on headgear.\n");

					(artprobs[ART_IDX_HELM_WIS])++;
					/* Counted this one separately so subtract it here */
					temp--;
				}
				if(a_ptr->flags1 & TR1_INT)
				{
					LOG_PRINT("Adding 1 for INT bonus on headgear.\n");

					(artprobs[ART_IDX_HELM_INT])++;
					/* Counted this one separately so subtract it here */
					temp--;
				}
			}
			else if ((a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR ||
				a_ptr->tval == TV_DRAG_ARMOR) && a_ptr->flags1 & TR1_CON)
			{
				/* Handle CON bonus on armor */
				LOG_PRINT("Adding 1 for CON bonus on body armor.\n");

				(artprobs[ART_IDX_ARMOR_CON])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}
			else if ((a_ptr->tval == TV_GLOVES) && (a_ptr->flags1 & TR1_DEX))
			{
				/* Handle DEX bonus on gloves */
				LOG_PRINT("Adding 1 for DEX bonus on gloves.\n");

				(artprobs[ART_IDX_GLOVE_DEX])++;
				/* Counted this one separately so subtract it here */
				temp--;
			}

			/* Now the general case */

			if (temp > 0)
			{
				/* There are some bonuses that weren't handled above */
				LOG_PRINT1("Adding %d for stat bonuses - general.\n", temp);

				(artprobs[ART_IDX_GEN_STAT]) += temp;

			/* Done with stat bonuses */
			}
		}

		if ( (a_ptr->flags2 & TR2_SUST_STR) || (a_ptr->flags2 & TR2_SUST_INT) ||
			(a_ptr->flags2 & TR2_SUST_WIS) || (a_ptr->flags2 & TR2_SUST_DEX) ||
			(a_ptr->flags2 & TR2_SUST_CON) || (a_ptr->flags2 & TR2_SUST_CHR) )
		{
			/* Now do sustains, in a similar manner */
			temp = 0;
			if (a_ptr->flags2 & TR2_SUST_STR) temp++;
			if (a_ptr->flags2 & TR2_SUST_INT) temp++;
			if (a_ptr->flags2 & TR2_SUST_WIS) temp++;
			if (a_ptr->flags2 & TR2_SUST_DEX) temp++;
			if (a_ptr->flags2 & TR2_SUST_CON) temp++;
			if (a_ptr->flags2 & TR2_SUST_CHR) temp++;
			LOG_PRINT1("Adding %d for stat sustains.\n", temp);

			(artprobs[ART_IDX_GEN_SUST]) += temp;
		}

		if (a_ptr->flags1 & TR1_STEALTH)
		{
			/* Handle stealth, including a couple of special cases */
			if(a_ptr->tval == TV_BOOTS)
			{
				LOG_PRINT("Adding 1 for stealth bonus on boots.\n");

				(artprobs[ART_IDX_BOOT_STEALTH])++;
			}
			else if (a_ptr->tval == TV_CLOAK)
			{
				LOG_PRINT("Adding 1 for stealth bonus on cloak.\n");

				(artprobs[ART_IDX_CLOAK_STEALTH])++;
			}
			else if (a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
			{
				LOG_PRINT("Adding 1 for stealth bonus on armor.\n");

				(artprobs[ART_IDX_ARMOR_STEALTH])++;
			}
			else
			{
				/* General case */
				LOG_PRINT("Adding 1 for stealth bonus - general.\n");

				(artprobs[ART_IDX_GEN_STEALTH])++;
			}
			/* Done with stealth */
		}

		if (a_ptr->flags1 & TR1_SEARCH)
		{
			/* Handle searching bonus - fully generic this time */
			LOG_PRINT("Adding 1 for search bonus - general.\n");

			(artprobs[ART_IDX_GEN_SEARCH])++;
		}

		if (a_ptr->flags1 & TR1_INFRA)
		{
			/* Handle infravision bonus - fully generic */
			LOG_PRINT("Adding 1 for infravision bonus - general.\n");

			(artprobs[ART_IDX_GEN_INFRA])++;
		}

		if (a_ptr->flags1 & TR1_SPEED)
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

			if (a_ptr->pval > 7)
			{
				/* Supercharge case */
				LOG_PRINT("Adding 1 for supercharged speed bonus!\n");

				(artprobs[ART_IDX_GEN_SPEED_SUPER])++;
			}
			else if(a_ptr->tval == TV_BOOTS)
			{
				/* Handle boots separately */
				LOG_PRINT("Adding 1 for normal speed bonus on boots.\n");

				(artprobs[ART_IDX_BOOT_SPEED])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for normal speed bonus - general.\n");

				(artprobs[ART_IDX_GEN_SPEED])++;
			}
			/* Done with speed */
		}

		if ( (a_ptr->flags2 & TR2_IM_ACID) || (a_ptr->flags2 & TR2_IM_ELEC) ||
			(a_ptr->flags2 & TR2_IM_FIRE) || (a_ptr->flags2 & TR2_IM_COLD) )
		{
			/* Count up immunities for this item, if any */
			temp = 0;
			if (a_ptr->flags2 & TR2_IM_ACID) temp++;
			if (a_ptr->flags2 & TR2_IM_ELEC) temp++;
			if (a_ptr->flags2 & TR2_IM_FIRE) temp++;
			if (a_ptr->flags2 & TR2_IM_COLD) temp++;
			LOG_PRINT1("Adding %d for immunities.\n", temp);

			(artprobs[ART_IDX_GEN_IMMUNE]) += temp;
		}

		if (a_ptr->flags3 & TR3_FREE_ACT)
		{
			/* Free action - handle gloves separately */
			if(a_ptr->tval == TV_GLOVES)
			{
				LOG_PRINT("Adding 1 for free action on gloves.\n");

				(artprobs[ART_IDX_GLOVE_FA])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for free action - general.\n");

				(artprobs[ART_IDX_GEN_FA])++;
			}
		}

		if (a_ptr->flags3 & TR3_HOLD_LIFE)
		{
			/* Hold life - do body armor separately */
			if( (a_ptr->tval == TV_SOFT_ARMOR) || (a_ptr->tval == TV_HARD_ARMOR) ||
				(a_ptr->tval == TV_DRAG_ARMOR))
			{
				LOG_PRINT("Adding 1 for hold life on armor.\n");

				(artprobs[ART_IDX_ARMOR_HLIFE])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for hold life - general.\n");

				(artprobs[ART_IDX_GEN_HLIFE])++;
			}
		}

		if (a_ptr->flags3 & TR3_FEATHER)
		{
			/* Feather fall - handle boots separately */
			if(a_ptr->tval == TV_BOOTS)
			{
				LOG_PRINT("Adding 1 for feather fall on boots.\n");

				(artprobs[ART_IDX_BOOT_FEATHER])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for feather fall - general.\n");

				(artprobs[ART_IDX_GEN_FEATHER])++;
			}
		}

		if (a_ptr->flags3 & TR3_LITE)
		{
			/* Handle permanent light */
			LOG_PRINT("Adding 1 for permanent light - general.\n");

			(artprobs[ART_IDX_GEN_LITE])++;
		}

		if (a_ptr->flags3 & TR3_SEE_INVIS)
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
					LOG_PRINT("Adding 1 for see invisible - headgear.\n");

					(artprobs[ART_IDX_HELM_SINV])++;
				}
				else
				{
					LOG_PRINT("Adding 1 for see invisible - general.\n");

					(artprobs[ART_IDX_GEN_SINV])++;
				}
			}
		}

		if (a_ptr->flags3 & TR3_TELEPATHY)
		{
			/* ESP case.  Handle helms/crowns separately. */
			if(a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
			{
				LOG_PRINT("Adding 1 for ESP on headgear.\n");

				(artprobs[ART_IDX_HELM_ESP])++;
			}
			else
			{
				LOG_PRINT("Adding 1 for ESP - general.\n");

				(artprobs[ART_IDX_GEN_ESP])++;
			}
		}

		if (a_ptr->flags3 & TR3_SLOW_DIGEST)
		{
			/* Slow digestion case - generic. */
			LOG_PRINT("Adding 1 for slow digestion - general.\n");

			(artprobs[ART_IDX_GEN_SDIG])++;
		}

		if (a_ptr->flags3 & TR3_REGEN)
		{
			/* Regeneration case - generic. */
			LOG_PRINT("Adding 1 for regeneration - general.\n");

			(artprobs[ART_IDX_GEN_REGEN])++;
		}

		if ( (a_ptr->flags2 & TR2_RES_ACID) || (a_ptr->flags2 & TR2_RES_ELEC) ||
			(a_ptr->flags2 & TR2_RES_FIRE) || (a_ptr->flags2 & TR2_RES_COLD) )
		{
			/* Count up low resists (not the type, just the number) */
			temp = 0;
			if (a_ptr->flags2 & TR2_RES_ACID) temp++;
			if (a_ptr->flags2 & TR2_RES_ELEC) temp++;
			if (a_ptr->flags2 & TR2_RES_FIRE) temp++;
			if (a_ptr->flags2 & TR2_RES_COLD) temp++;

			/* Shields treated separately */
			if (a_ptr->tval == TV_SHIELD)
			{
				LOG_PRINT1("Adding %d for low resists on shield.\n", temp);

				(artprobs[ART_IDX_SHIELD_LRES]) += temp;
			}
			else if (a_ptr->tval == TV_SOFT_ARMOR ||
				a_ptr->tval == TV_HARD_ARMOR || a_ptr->tval == TV_DRAG_ARMOR)
			{
				/* Armor also treated separately */
				if (temp == 4)
				{
					/* Special case: armor has all four low resists */
					LOG_PRINT("Adding 1 for ALL LOW RESISTS on body armor.\n");

					(artprobs[ART_IDX_ARMOR_ALLRES])++;
				}
				else
				{
					/* Just tally up the resists as usual */
					LOG_PRINT1("Adding %d for low resists on body armor.\n", temp);

					(artprobs[ART_IDX_ARMOR_LRES]) += temp;
				}
			}
			else
			{
				/* General case */
				LOG_PRINT1("Adding %d for low resists - general.\n", temp);

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
			if (a_ptr->flags2 & TR2_RES_POIS) temp++;
			if (a_ptr->flags2 & TR2_RES_FEAR) temp++;
			if (a_ptr->flags2 & TR2_RES_LITE) temp++;
			if (a_ptr->flags2 & TR2_RES_DARK) temp++;
			if (a_ptr->flags2 & TR2_RES_BLIND) temp++;
			if (a_ptr->flags2 & TR2_RES_CONFU) temp++;
			if (a_ptr->flags2 & TR2_RES_SOUND) temp++;
			if (a_ptr->flags2 & TR2_RES_SHARD) temp++;
			if (a_ptr->flags2 & TR2_RES_NEXUS) temp++;
			if (a_ptr->flags2 & TR2_RES_NETHR) temp++;
			if (a_ptr->flags2 & TR2_RES_CHAOS) temp++;
			if (a_ptr->flags2 & TR2_RES_DISEN) temp++;
			LOG_PRINT1("Adding %d for high resists on body armor.\n", temp);

			(artprobs[ART_IDX_ARMOR_HRES]) += temp;
		}

		/* Now do the high resists individually */
		if (a_ptr->flags2 & TR2_RES_POIS)
		{
			/* Resist poison ability */
			LOG_PRINT("Adding 1 for resist poison - general.\n");

			(artprobs[ART_IDX_GEN_RPOIS])++;
		}

		if (a_ptr->flags2 & TR2_RES_FEAR)
		{
			/* Resist fear ability */
			LOG_PRINT("Adding 1 for resist fear - general.\n");

			(artprobs[ART_IDX_GEN_RFEAR])++;
		}

		if (a_ptr->flags2 & TR2_RES_LITE)
		{
			/* Resist light ability */
			LOG_PRINT("Adding 1 for resist light - general.\n");

			(artprobs[ART_IDX_GEN_RLITE])++;
		}

		if (a_ptr->flags2 & TR2_RES_DARK)
		{
			/* Resist dark ability */
			LOG_PRINT("Adding 1 for resist dark - general.\n");

			(artprobs[ART_IDX_GEN_RDARK])++;
		}

		if (a_ptr->flags2 & TR2_RES_BLIND)
		{
			/* Resist blind ability - helms/crowns are separate */
			if(a_ptr->tval == TV_HELM || a_ptr->tval == TV_CROWN)
			{
				LOG_PRINT("Adding 1 for resist blindness - headgear.\n");

				(artprobs[ART_IDX_HELM_RBLIND])++;
			}
			else
			{
				/* General case */
				LOG_PRINT("Adding 1 for resist blindness - general.\n");

				(artprobs[ART_IDX_GEN_RBLIND])++;
			}
		}

		if (a_ptr->flags2 & TR2_RES_CONFU)
		{
			/* Resist confusion ability */
			LOG_PRINT("Adding 1 for resist confusion - general.\n");

			(artprobs[ART_IDX_GEN_RCONF])++;
		}

		if (a_ptr->flags2 & TR2_RES_SOUND)
		{
			/* Resist sound ability */
			LOG_PRINT("Adding 1 for resist sound - general.\n");

			(artprobs[ART_IDX_GEN_RSOUND])++;
		}

		if (a_ptr->flags2 & TR2_RES_SHARD)
		{
			/* Resist shards ability */
			LOG_PRINT("Adding 1 for resist shards - general.\n");

			(artprobs[ART_IDX_GEN_RSHARD])++;
		}

		if (a_ptr->flags2 & TR2_RES_NEXUS)
		{
			/* Resist nexus ability */
			LOG_PRINT("Adding 1 for resist nexus - general.\n");

			(artprobs[ART_IDX_GEN_RNEXUS])++;
		}

		if (a_ptr->flags2 & TR2_RES_NETHR)
		{
			/* Resist nether ability */
			LOG_PRINT("Adding 1 for resist nether - general.\n");

			(artprobs[ART_IDX_GEN_RNETHER])++;
		}

		if (a_ptr->flags2 & TR2_RES_CHAOS)
		{
			/* Resist chaos ability */
			LOG_PRINT("Adding 1 for resist chaos - general.\n");

			(artprobs[ART_IDX_GEN_RCHAOS])++;
		}

		if (a_ptr->flags2 & TR2_RES_DISEN)
		{
			/* Resist disenchantment ability */
			LOG_PRINT("Adding 1 for resist disenchantment - general.\n");

			(artprobs[ART_IDX_GEN_RDISEN])++;
		}
		/* Done with parsing of frequencies for this item */
	}
	/* End for loop */

	if(verbose)
	{
	/* Print out some of the abilities, to make sure that everything's fine */
		for(i=0; i<ART_IDX_TOTAL; i++)
		{
			file_putf(log_file, "Frequency of ability %d: %d\n", i, artprobs[i]);
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
	for(i=0; i<ART_IDX_TOTAL; i++)
	{
		LOG_PRINT2( "Rescaled frequency of ability %d: %d\n", i, artprobs[i]);
	}

}

static bool add_str(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_STR) return FALSE;
	a_ptr->flags1 |= TR1_STR;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: STR (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_int(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_INT) return FALSE;
	a_ptr->flags1 |= TR1_INT;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: INT (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_wis(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_WIS) return FALSE;
	a_ptr->flags1 |= TR1_WIS;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: WIS (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_dex(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_DEX) return FALSE;
	a_ptr->flags1 |= TR1_DEX;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: DEX (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_con(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_CON) return FALSE;
	a_ptr->flags1 |= TR1_CON;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: CON (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static bool add_chr(artifact_type *a_ptr)
{
	if(a_ptr->flags1 & TR1_CHR) return FALSE;
	a_ptr->flags1 |= TR1_CHR;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: CHR (now %+d)\n", a_ptr->pval);
	return TRUE;
}

static void add_stat(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack: break out if all stats are raised to avoid an infinite loop */
	if ((a_ptr->flags1 & TR1_STR) && (a_ptr->flags1 & TR1_INT) &&
		(a_ptr->flags1 & TR1_WIS) && (a_ptr->flags1 & TR1_DEX) &&
		(a_ptr->flags1 & TR1_CON) && (a_ptr->flags1 & TR1_CHR))
			return;

	/* Make sure we add one that hasn't been added yet */
	while (!success)
	{
		r = randint0(6);
		if (r == 0) success = add_str(a_ptr);
		else if (r == 1) success = add_int(a_ptr);
		else if (r == 2) success = add_wis(a_ptr);
		else if (r == 3) success = add_dex(a_ptr);
		else if (r == 4) success = add_con(a_ptr);
		else if (r == 5) success = add_chr(a_ptr);
	}
}

static bool add_sus_str(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_STR) return FALSE;
	a_ptr->flags2 |= TR2_SUST_STR;
	LOG_PRINT("Adding ability: sustain STR\n");
	return TRUE;
}

static bool add_sus_int(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_INT) return FALSE;
	a_ptr->flags2 |= TR2_SUST_INT;
	LOG_PRINT("Adding ability: sustain INT\n");
	return TRUE;
}

static bool add_sus_wis(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_WIS) return FALSE;
	a_ptr->flags2 |= TR2_SUST_WIS;
	LOG_PRINT("Adding ability: sustain WIS\n");
	return TRUE;
}

static bool add_sus_dex(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_DEX) return FALSE;
	a_ptr->flags2 |= TR2_SUST_DEX;
	LOG_PRINT("Adding ability: sustain DEX\n");
	return TRUE;
}

static bool add_sus_con(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_CON) return FALSE;
	a_ptr->flags2 |= TR2_SUST_CON;
	LOG_PRINT("Adding ability: sustain CON\n");
	return TRUE;
}

static bool add_sus_chr(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_SUST_CHR) return FALSE;
	a_ptr->flags2 |= TR2_SUST_CHR;
	LOG_PRINT("Adding ability: sustain CHR\n");
	return TRUE;
}

static void add_sustain(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack: break out if all stats are sustained to avoid an infinite loop */
	if ((a_ptr->flags2 & TR2_SUST_STR) && (a_ptr->flags2 & TR2_SUST_INT) &&
		(a_ptr->flags2 & TR2_SUST_WIS) && (a_ptr->flags2 & TR2_SUST_DEX) &&
		(a_ptr->flags2 & TR2_SUST_CON) && (a_ptr->flags2 & TR2_SUST_CHR))
			return;

	while (!success)
	{
		r = randint0(6);
		if (r == 0) success = add_sus_str(a_ptr);
		else if (r == 1) success = add_sus_int(a_ptr);
		else if (r == 2) success = add_sus_wis(a_ptr);
		else if (r == 3) success = add_sus_dex(a_ptr);
		else if (r == 4) success = add_sus_con(a_ptr);
		else if (r == 5) success = add_sus_chr(a_ptr);
	}
}

static void add_stealth(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_STEALTH;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: stealth (now %+d)\n", a_ptr->pval);
}

static void add_search(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SEARCH;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: searching (now %+d)\n", a_ptr->pval);
}

static void add_infravision(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_INFRA;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: infravision (now %+d)\n", a_ptr->pval);
}

static void add_tunnelling(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_TUNNEL;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: tunnelling (new bonus is %+d)\n", a_ptr->pval);
}

static void add_speed(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SPEED;
	if (a_ptr->pval == 0)
	{
		a_ptr->pval = (s16b)randint1(4);
		LOG_PRINT1("Adding ability: speed (first time) (now %+d)\n", a_ptr->pval);
	}
	else
	{
		do_pval(a_ptr);
		LOG_PRINT1("Adding ability: speed (now %+d)\n", a_ptr->pval);
	}
}

static void add_shots(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_SHOTS;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: extra shots (now %+d)\n", a_ptr->pval);
}

static void add_blows(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_BLOWS;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: extra blows (%d additional blows)\n", a_ptr->pval);
}

static void add_might(artifact_type *a_ptr)
{
	a_ptr->flags1 |= TR1_MIGHT;
	do_pval(a_ptr);
	LOG_PRINT1("Adding ability: extra might (now %+d)\n", a_ptr->pval);
}

static bool add_resist_acid(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_ACID) return FALSE;
	a_ptr->flags2 |= TR2_RES_ACID;
	LOG_PRINT("Adding ability: resist acid\n");
	return TRUE;
}

static bool add_resist_lightning(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_ELEC) return FALSE;
	a_ptr->flags2 |= TR2_RES_ELEC;
	LOG_PRINT("Adding ability: resist lightning\n");
	return TRUE;
}

static bool add_resist_fire(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_FIRE) return FALSE;
	a_ptr->flags2 |= TR2_RES_FIRE;
	LOG_PRINT("Adding ability: resist fire\n");
	return TRUE;
}

static bool add_resist_cold(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_COLD) return FALSE;
	a_ptr->flags2 |= TR2_RES_COLD;
	LOG_PRINT("Adding ability: resist cold\n");
	return TRUE;
}

static void add_low_resist(artifact_type *a_ptr)
{
	int r;
	bool success = FALSE;

	/* Hack - if all low resists added already, exit to avoid infinite loop */
	if( (a_ptr->flags2 & TR2_RES_ACID) && (a_ptr->flags2 & TR2_RES_ELEC) &&
		(a_ptr->flags2 & TR2_RES_FIRE) && (a_ptr->flags2 & TR2_RES_COLD) )
			return;

	while (!success)
	{
		r = randint0(4);
		if (r == 0) success = add_resist_acid(a_ptr);
		else if (r == 1) success = add_resist_lightning(a_ptr);
		else if (r == 2) success = add_resist_fire(a_ptr);
		else if (r == 3) success = add_resist_cold(a_ptr);
	}
}

static bool add_resist_poison(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_POIS) return FALSE;
	a_ptr->flags2 |= TR2_RES_POIS;
	LOG_PRINT("Adding ability: resist poison\n");
	return TRUE;
}

static bool add_resist_fear(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_FEAR) return FALSE;
	a_ptr->flags2 |= TR2_RES_FEAR;
	LOG_PRINT("Adding ability: resist fear\n");
	return TRUE;
}

static bool add_resist_light(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_LITE) return FALSE;
	a_ptr->flags2 |= TR2_RES_LITE;
	LOG_PRINT("Adding ability: resist light\n");
	return TRUE;
}

static bool add_resist_dark(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_DARK) return FALSE;
	a_ptr->flags2 |= TR2_RES_DARK;
	LOG_PRINT("Adding ability: resist dark\n");
	return TRUE;
}

static bool add_resist_blindness(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_BLIND) return FALSE;
	a_ptr->flags2 |= TR2_RES_BLIND;
	LOG_PRINT("Adding ability: resist blindness\n");
	return TRUE;
}

static bool add_resist_confusion(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_CONFU) return FALSE;
	a_ptr->flags2 |= TR2_RES_CONFU;
	LOG_PRINT("Adding ability: resist confusion\n");
	return TRUE;
}

static bool add_resist_sound(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_SOUND) return FALSE;
	a_ptr->flags2 |= TR2_RES_SOUND;
	LOG_PRINT("Adding ability: resist sound\n");
	return TRUE;
}

static bool add_resist_shards(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_SHARD) return FALSE;
	a_ptr->flags2 |= TR2_RES_SHARD;
	LOG_PRINT("Adding ability: resist shards\n");
	return TRUE;
}

static bool add_resist_nexus(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_NEXUS) return FALSE;
	a_ptr->flags2 |= TR2_RES_NEXUS;
	LOG_PRINT("Adding ability: resist nexus\n");
	return TRUE;
}

static bool add_resist_nether(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_NETHR) return FALSE;
	a_ptr->flags2 |= TR2_RES_NETHR;
	LOG_PRINT("Adding ability: resist nether\n");
	return TRUE;
}

static bool add_resist_chaos(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_CHAOS) return FALSE;
	a_ptr->flags2 |= TR2_RES_CHAOS;
	LOG_PRINT("Adding ability: resist chaos\n");
	return TRUE;
}

static bool add_resist_disenchantment(artifact_type *a_ptr)
{
	if (a_ptr->flags2 & TR2_RES_DISEN) return FALSE;
	a_ptr->flags2 |= TR2_RES_DISEN;
	LOG_PRINT("Adding ability: resist disenchantment\n");
	return TRUE;
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
		if (i == 0) success = add_resist_poison(a_ptr);
		else if (i == 1) success = add_resist_fear(a_ptr);
		else if (i == 2) success = add_resist_light(a_ptr);
		else if (i == 3) success = add_resist_dark(a_ptr);
		else if (i == 4) success = add_resist_blindness(a_ptr);
		else if (i == 5) success = add_resist_confusion(a_ptr);
		else if (i == 6) success = add_resist_sound(a_ptr);
		else if (i == 7) success = add_resist_shards(a_ptr);
		else if (i == 8) success = add_resist_nexus(a_ptr);
		else if (i == 9) success = add_resist_nether(a_ptr);
		else if (i == 10) success = add_resist_chaos(a_ptr);
		else if (i == 11) success = add_resist_disenchantment(a_ptr);

		count++;
	}
}

static void add_slow_digestion(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_SLOW_DIGEST;
	LOG_PRINT("Adding ability: slow digestion\n");
}

static void add_feather_falling(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_FEATHER;
	LOG_PRINT("Adding ability: feather fall\n");
}

static void add_permanent_light(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_LITE;
	LOG_PRINT("Adding ability: permanent light\n");
}

static void add_regeneration(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_REGEN;
	LOG_PRINT("Adding ability: regeneration\n");
}

static void add_telepathy(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_TELEPATHY;
	LOG_PRINT("Adding ability: telepathy\n");
}

static void add_see_invisible(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_SEE_INVIS;
	LOG_PRINT("Adding ability: see invisible\n");
}

static void add_free_action(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_FREE_ACT;
	LOG_PRINT("Adding ability: free action\n");
}

static void add_hold_life(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_HOLD_LIFE;
	LOG_PRINT("Adding ability: hold life\n");
}

static bool add_slay_animal(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_ANIMAL) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_ANIMAL;
	LOG_PRINT("Adding ability: slay animal\n");
	return TRUE;
}

static bool add_slay_evil(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_EVIL) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_EVIL;
	LOG_PRINT("Adding ability: slay evil\n");
	return TRUE;
}

static bool add_slay_orc(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_ORC) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_ORC;
	LOG_PRINT("Adding ability: slay orc\n");
	return TRUE;
}

static bool add_slay_troll(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_TROLL) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_TROLL;
	LOG_PRINT("Adding ability: slay troll \n");
	return TRUE;
}

static bool add_slay_giant(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_GIANT) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_GIANT;
	LOG_PRINT("Adding ability: slay giant\n");
	return TRUE;
}

static bool add_slay_demon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_DEMON) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_DEMON;
	LOG_PRINT("Adding ability: slay demon\n");
	return TRUE;
}

static bool add_slay_undead(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_UNDEAD) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_UNDEAD;
	LOG_PRINT("Adding ability: slay undead\n");
	return TRUE;
}

static bool add_slay_dragon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_SLAY_DRAGON) return FALSE;
	a_ptr->flags1 |= TR1_SLAY_DRAGON;
	LOG_PRINT("Adding ability: slay dragon\n");
	return TRUE;
}

static bool add_kill_demon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_KILL_DEMON) return FALSE;
	a_ptr->flags1 |= TR1_KILL_DEMON;
	LOG_PRINT("Adding ability: kill demon\n");
	return TRUE;
}

static bool add_kill_undead(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_KILL_UNDEAD) return FALSE;
	a_ptr->flags1 |= TR1_KILL_UNDEAD;
	LOG_PRINT("Adding ability: kill undead\n");
	return TRUE;
}

static bool add_kill_dragon(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_KILL_DRAGON) return FALSE;
	a_ptr->flags1 |= TR1_KILL_DRAGON;
	LOG_PRINT("Adding ability: kill dragon\n");
	return TRUE;
}

static bool add_acid_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_ACID) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_ACID;
	LOG_PRINT("Adding ability: acid brand\n");
	return TRUE;
}

static bool add_lightning_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_ELEC) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_ELEC;
	LOG_PRINT("Adding ability: lightning brand\n");
	return TRUE;
}

static bool add_fire_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_FIRE) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_FIRE;
	LOG_PRINT("Adding ability: fire brand\n");
	return TRUE;
}

static bool add_frost_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_COLD) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_COLD;
	LOG_PRINT("Adding ability: frost brand\n");
	return TRUE;
}

static bool add_poison_brand(artifact_type *a_ptr)
{
	if (a_ptr->flags1 & TR1_BRAND_POIS) return FALSE;
	a_ptr->flags1 |= TR1_BRAND_POIS;
	LOG_PRINT("Adding ability: poison brand\n");
	return TRUE;
}

static void add_brand(artifact_type *a_ptr)
{
	/* Pick a brand at random */

	int r;
	int count = 0;
	bool success = FALSE;

	while ( (!success) & (count < MAX_TRIES) )
	{
		r = randint0(5);
		if (r == 0) success = add_fire_brand(a_ptr);
		else if (r == 1) success = add_frost_brand(a_ptr);
		else if (r == 2) success = add_poison_brand(a_ptr);
		else if (r == 3) success = add_acid_brand(a_ptr);
		else if (r == 4) success = add_lightning_brand(a_ptr);

		count++;
	}
}

static void add_slay(artifact_type *a_ptr)
{
	/* Pick a slay at random */

	int r;
	int count = 0;
	bool success = FALSE;

	while ( (!success) & (count < MAX_TRIES) )
	{
		r = randint0(11);
		if (r == 0) success = add_slay_evil(a_ptr);
		else if (r == 1) success = add_kill_dragon(a_ptr);
		else if (r == 2) success = add_slay_animal(a_ptr);
		else if (r == 3) success = add_slay_undead(a_ptr);
		else if (r == 4) success = add_slay_dragon(a_ptr);
		else if (r == 5) success = add_slay_demon(a_ptr);
		else if (r == 6) success = add_slay_troll(a_ptr);
		else if (r == 7) success = add_slay_orc(a_ptr);
		else if (r == 8) success = add_slay_giant(a_ptr);
		else if (r == 9) success = add_kill_demon(a_ptr);
		else if (r == 10) success = add_kill_undead(a_ptr);

		count++;
	}
}

static void add_bless_weapon(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_BLESSED;
	LOG_PRINT("Adding ability: blessed weapon\n");
}

static void add_damage_dice(artifact_type *a_ptr)
{
	/* CR 2001-09-02: changed this to increments 1 or 2 only */
	a_ptr->dd += (byte)randint1(2);
	if (a_ptr->dd > 9)
		a_ptr->dd = 9;
	LOG_PRINT1("Adding ability: extra damage dice (now %d dice)\n", a_ptr->dd);
}

static void add_to_hit(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_h > VERYHIGH_TO_HIT)
	{
		if (!INHIBIT_STRONG)
		{
			LOG_PRINT1("Failed to add to-hit, value of %d is too high\n", a_ptr->to_h);
			return;
		}
	}
	else if (a_ptr->to_h > HIGH_TO_HIT)
	{
		if (!INHIBIT_WEAK)
		{
			LOG_PRINT1("Failed to add to-hit, value of %d is too high\n", a_ptr->to_h);
			return;
		}
	}
	a_ptr->to_h += (s16b)(fixed + randint0(random));
	if (a_ptr->to_h > 0) a_ptr->flags3 |= TR3_SHOW_MODS;
	LOG_PRINT1("Adding ability: extra to_h (now %+d)\n", a_ptr->to_h);
}

static void add_to_dam(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_d > VERYHIGH_TO_DAM)
	{
		if (!INHIBIT_STRONG)
		{
			LOG_PRINT1("Failed to add to-dam, value of %d is too high\n", a_ptr->to_d);
			return;
		}
	}
	else if (a_ptr->to_h > HIGH_TO_DAM)
	{
		if (!INHIBIT_WEAK)
		{
			LOG_PRINT1("Failed to add to-dam, value of %d is too high\n", a_ptr->to_d);
			return;
		}
	}
	a_ptr->to_d += (s16b)(fixed + randint0(random));
	if (a_ptr->to_d > 0) a_ptr->flags3 |= TR3_SHOW_MODS;
	LOG_PRINT1("Adding ability: extra to_dam (now %+d)\n", a_ptr->to_d);
}

static void add_aggravation(artifact_type *a_ptr)
{
	a_ptr->flags3 |= TR3_AGGRAVATE;
	LOG_PRINT("Adding aggravation\n");
}

static void add_to_AC(artifact_type *a_ptr, int fixed, int random)
{
	/* Inhibit above certain threshholds */
	if (a_ptr->to_a > VERYHIGH_TO_AC)
	{
		if (!INHIBIT_STRONG)
		{
			LOG_PRINT1("Failed to add to-AC, value of %d is too high\n", a_ptr->to_a);
			return;
		}
	}
	else if (a_ptr->to_h > HIGH_TO_AC)
	{
		if (!INHIBIT_WEAK)
		{
			LOG_PRINT1("Failed to add to-AC, value of %d is too high\n", a_ptr->to_a);
			return;
		}
	}
	a_ptr->to_a += (s16b)(fixed + randint0(random));
	LOG_PRINT1("Adding ability: AC bonus (new bonus is %+d)\n", a_ptr->to_a);
}

static void add_weight_mod(artifact_type *a_ptr)
{
	a_ptr->weight = (a_ptr->weight * 9) / 10;
	LOG_PRINT1("Adding ability: lower weight (new weight is %d)\n", a_ptr->weight);
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
			a_ptr->flags2 |= TR2_IM_ACID;
			LOG_PRINT("Adding ability: immunity to acid\n");
			break;
		}
		case 1:
		{
			a_ptr->flags2 |= TR2_IM_ELEC;
			LOG_PRINT("Adding ability: immunity to lightning\n");
			break;
		}
		case 2:
		{
			a_ptr->flags2 |= TR2_IM_FIRE;
			LOG_PRINT("Adding ability: immunity to fire\n");
			break;
		}
		case 3:
		{
			a_ptr->flags2 |= TR2_IM_COLD;
			LOG_PRINT("Adding ability: immunity to cold\n");
			break;
		}
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
		LOG_PRINT2("Cumulative frequency of ability %d is: %d\n", i, freq[i]);
}

/*
 * Choose a random ability using weights based on the given cumulative frequency
 * table.  A pointer to the frequency array (which must be of size ART_IDX_TOTAL)
 * is passed as a parameter.  The function returns a number representing the
 * index of the ability chosen.
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

	LOG_PRINT1("Ability chosen was number: %d\n", ability);
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
			add_shots(a_ptr);
			break;

		case ART_IDX_BOW_MIGHT:
			add_might(a_ptr);
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
			if (target_power > 90)
			{
				add_aggravation(a_ptr);
			}
			break;

		case ART_IDX_MELEE_BLESS:
			add_bless_weapon(a_ptr);
			break;

		case ART_IDX_MELEE_BRAND:
			add_brand(a_ptr);
			break;

		case ART_IDX_MELEE_SLAY:
			add_slay(a_ptr);
			break;

		case ART_IDX_MELEE_SINV:
		case ART_IDX_HELM_SINV:
		case ART_IDX_GEN_SINV:
			add_see_invisible(a_ptr);
			break;

		case ART_IDX_MELEE_BLOWS:
			add_blows(a_ptr);
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
			add_tunnelling(a_ptr);
			break;

		case ART_IDX_BOOT_FEATHER:
		case ART_IDX_GEN_FEATHER:
			add_feather_falling(a_ptr);
			break;

		case ART_IDX_BOOT_STEALTH:
		case ART_IDX_CLOAK_STEALTH:
		case ART_IDX_ARMOR_STEALTH:
		case ART_IDX_GEN_STEALTH:
			add_stealth(a_ptr);
			break;

		case ART_IDX_BOOT_SPEED:
		case ART_IDX_GEN_SPEED:
			add_speed(a_ptr);
			break;

		case ART_IDX_GLOVE_FA:
		case ART_IDX_GEN_FA:
			add_free_action(a_ptr);
			break;

		case ART_IDX_GLOVE_DEX:
			add_dex(a_ptr);
			break;

		case ART_IDX_HELM_RBLIND:
		case ART_IDX_GEN_RBLIND:
			add_resist_blindness(a_ptr);
			break;

		case ART_IDX_HELM_ESP:
		case ART_IDX_GEN_ESP:
			add_telepathy(a_ptr);
			break;

		case ART_IDX_HELM_WIS:
			add_wis(a_ptr);
			break;

		case ART_IDX_HELM_INT:
			add_int(a_ptr);
			break;

		case ART_IDX_SHIELD_LRES:
		case ART_IDX_ARMOR_LRES:
		case ART_IDX_GEN_LRES:
			add_low_resist(a_ptr);
			break;

		case ART_IDX_ARMOR_HLIFE:
		case ART_IDX_GEN_HLIFE:
			add_hold_life(a_ptr);
			break;

		case ART_IDX_ARMOR_CON:
			add_con(a_ptr);
			break;

		case ART_IDX_ARMOR_ALLRES:
			add_resist_acid(a_ptr);
			add_resist_lightning(a_ptr);
			add_resist_fire(a_ptr);
			add_resist_cold(a_ptr);
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
			add_search(a_ptr);
			break;

		case ART_IDX_GEN_INFRA:
			add_infravision(a_ptr);
			break;

		case ART_IDX_GEN_IMMUNE:
			add_immunity(a_ptr);
			break;

		case ART_IDX_GEN_LITE:
			add_permanent_light(a_ptr);
			break;

		case ART_IDX_GEN_SDIG:
			add_slow_digestion(a_ptr);
			break;

		case ART_IDX_GEN_REGEN:
			add_regeneration(a_ptr);
			break;

		case ART_IDX_GEN_RPOIS:
			add_resist_poison(a_ptr);
			break;

		case ART_IDX_GEN_RFEAR:
			add_resist_fear(a_ptr);
			break;

		case ART_IDX_GEN_RLITE:
			add_resist_light(a_ptr);
			break;

		case ART_IDX_GEN_RDARK:
			add_resist_dark(a_ptr);
			break;

		case ART_IDX_GEN_RCONF:
			add_resist_confusion(a_ptr);
			break;

		case ART_IDX_GEN_RSOUND:
			add_resist_sound(a_ptr);
			break;

		case ART_IDX_GEN_RSHARD:
			add_resist_shards(a_ptr);
			break;

		case ART_IDX_GEN_RNEXUS:
			add_resist_nexus(a_ptr);
			break;

		case ART_IDX_GEN_RNETHER:
			add_resist_nether(a_ptr);
			break;

		case ART_IDX_GEN_RCHAOS:
			add_resist_chaos(a_ptr);
			break;

		case ART_IDX_GEN_RDISEN:
			add_resist_disenchantment(a_ptr);
			break;
	}
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
	if ((a_ptr->flags1 & TR1_WIS) && (a_ptr->tval == TV_SWORD || a_ptr->tval == TV_POLEARM))
	{
		add_bless_weapon(a_ptr);
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
			if (a_ptr->dd > 9) a_ptr->dd = 9;
			LOG_PRINT1("Supercharging damage dice!  (Now %d dice)\n", a_ptr->dd);
		}
		else if (randint0(z_info->a_max) < artprobs[ART_IDX_MELEE_BLOWS_SUPER])
		{
			a_ptr->flags1 |= TR1_BLOWS;
			a_ptr->pval = 3;
			LOG_PRINT("Supercharging melee blows! (+3 blows)\n");
		}
	}

	/* Bows - +3 might or +3 shots */
	if (a_ptr->tval == TV_BOW)
	{
		if (randint0(z_info->a_max) < artprobs[ART_IDX_BOW_SHOTS_SUPER])
		{
			a_ptr->flags1 |= TR1_SHOTS;
			a_ptr->pval = 3;
			LOG_PRINT("Supercharging shots for bow!  (3 extra shots)\n");
		}
		else if (randint0(z_info->a_max) < artprobs[ART_IDX_BOW_MIGHT_SUPER])
		{
			a_ptr->flags1 |= TR1_MIGHT;
			a_ptr->pval = 3;
			LOG_PRINT("Supercharging might for bow!  (3 extra might)\n");
		}
	}

	/* Big speed bonus - any item (potentially) */
	if (randint0(z_info->a_max) < artprobs[ART_IDX_GEN_SPEED_SUPER])
	{
		a_ptr->flags1 |= TR1_SPEED;
		a_ptr->pval = 7 + randint0(6); 
		if (one_in_(4)) a_ptr->pval += randint1(4);
		LOG_PRINT1("Supercharging speed for this item!  (New speed bonus is %d)\n", a_ptr->pval);
	}
	/* Aggravation */
	if (a_ptr->tval == TV_BOW || a_ptr->tval == TV_DIGGING ||
		a_ptr->tval == TV_HAFTED || a_ptr->tval == TV_POLEARM ||
		a_ptr->tval == TV_SWORD)
	{
		if ((randint0(z_info->a_max) < artprobs[ART_IDX_WEAPON_AGGR]) &&
		    (target_power > 90))
		{
			a_ptr->flags3 |= TR3_AGGRAVATE;
			LOG_PRINT("Adding aggravation\n");
		}
	}
	else
	{
		if ((randint0(z_info->a_max) < artprobs[ART_IDX_NONWEAPON_AGGR]) &&
		    (target_power > 90))
		{
			a_ptr->flags3 |= TR3_AGGRAVATE;
			LOG_PRINT("Adding aggravation\n");
		}
	}
}

/*
 * Make it bad, or if it's already bad, make it worse!
 */
static void do_curse(artifact_type *a_ptr)
{
	if (one_in_(7))
		a_ptr->flags3 |= TR3_AGGRAVATE;
	if (one_in_(4))
		a_ptr->flags3 |= TR3_DRAIN_EXP;
	if (one_in_(7))
		a_ptr->flags3 |= TR3_TELEPORT;

	if ((a_ptr->pval > 0) && one_in_(2))
		a_ptr->pval = -a_ptr->pval;
	if ((a_ptr->to_a > 0) && one_in_(2))
		a_ptr->to_a = -a_ptr->to_a;
	if ((a_ptr->to_h > 0) && one_in_(2))
		a_ptr->to_h = -a_ptr->to_h;
	if ((a_ptr->to_d > 0) && one_in_(4))
		a_ptr->to_d = -a_ptr->to_d;

	if (a_ptr->flags3 & TR3_LIGHT_CURSE)
	{
		if (one_in_(2)) a_ptr->flags3 |= TR3_HEAVY_CURSE;
		return;
	}

	a_ptr->flags3 |= TR3_LIGHT_CURSE;

	if (one_in_(4))
		a_ptr->flags3 |= TR3_HEAVY_CURSE;
}

/*
 * Note the three special cases (One Ring, Grond, Morgoth).
 */
static void scramble_artifact(int a_idx)
{
	artifact_type *a_ptr = &a_info[a_idx];
	artifact_type a_old;
	object_kind *k_ptr;
	u32b activates = a_ptr->flags3 & TR3_ACTIVATE;
	s32b power;
	int tries = 0;
	s16b k_idx;
	byte rarity_old, base_rarity_old;
	s16b rarity_new;
	s32b ap = 0;
	bool curse_me = FALSE;
	bool success = FALSE;

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
		LOG_PRINT1("Skipping artifact number %d - too powerful to randomize!", a_idx);
		return;
	}

	if (power < 0) curse_me = TRUE;

	LOG_PRINT("+++++++++++++++++ CREATING NEW ARTIFACT ++++++++++++++++++\n");
	LOG_PRINT2("Artifact %d: power = %d\n", a_idx, power);

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
		 * weak, for the opposite reason.  We also require a new
		 * rarity rating of at least 2.
		 *
		 * CR 7/15/2001 - lowered the upper limit so that we get at
		 * least a few powers (from 8/10 to 6/10) but permit anything
		 * more than 20 below the target power
		 */
		int count = 0;
		int smeg = 0;
		s32b ap2;

		/* Capture the rarity of the original base item and artifact */
		base_rarity_old = 100 / base_item_prob[a_idx];
		if (base_rarity_old < 1) base_rarity_old = 1; 
		rarity_old = base_art_rarity[a_idx];
		do
		{
			k_idx = choose_item(a_idx);

			/*
			 * Hack: if power is positive but very low, and if we're not having
			 * any luck finding a base item, curse it once.  This helps ensure
			 * that we get a base item for borderline cases like Wormtongue.
			 */

			if (power > 0 && power < 10 && count > MAX_TRIES / 2)
			{
				LOG_PRINT("Cursing base item to help get a match.\n");
				do_curse(a_ptr);
			}
			ap2 = artifact_power(a_idx);
			count++;
			/*
			 * Calculate the proper rarity based on the new type.  We attempt
			 * to preserve the 'effective rarity' which is equal to the
			 * artifact rarity multiplied by the base item rarity.
			 */

/* CC bugfix hacking */
			LOG_PRINT2("rarity old is %d, base is %d\n", rarity_old, base_rarity_old);
			k_ptr = &k_info[k_idx];
			smeg = 100 / k_ptr->alloc_prob;
			if (smeg < 1) smeg = 1;
			LOG_PRINT1("k_ptr->alloc_prob is %d\n", k_ptr->alloc_prob);
/* end CC */
			rarity_new = ( (s16b) rarity_old * (s16b) base_rarity_old ) /
			             smeg;

			if (rarity_new > 255) rarity_new = 255;
			if (rarity_new < 1) rarity_new = 1;

		} while ( (count < MAX_TRIES) &&
		          (((ap2 > (power * 6) / 10 + 1) && (power-ap2 < 20)) ||
		          (ap2 < (power / 10)) || rarity_new == 1) );

		/* Got an item - set the new rarity */
		a_ptr->rarity = (byte) rarity_new;
	}
	else
	{
		/*
		 * Special artifact (light source, ring, or amulet).
		 * Clear the following fields; leave the rest alone.
		 */
		a_ptr->pval = 0;
		a_ptr->to_h = a_ptr->to_d = a_ptr->to_a = 0;
		a_ptr->flags1 = a_ptr->flags2 = 0;

		/* Artifacts ignore everything */
		a_ptr->flags3 = (TR3_IGNORE_MASK);
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
		LOG_PRINT("--- Supercharge is too powerful!  Rolling back.\n");
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
				LOG_PRINT("Inhibited ability added - rolling back.\n");
				*a_ptr = a_old;
			}
		} while (!success);
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
				LOG_PRINT("--- Too powerful!  Rolling back.\n");
				continue;
			}
			else if (ap >= (power * 19) / 20)	/* just right */
			{
			/* CC 11/02/09 - add rescue for crappy weapons */
				if ((a_ptr->tval == TV_DIGGING || a_ptr->tval == TV_HAFTED ||
					a_ptr->tval == TV_POLEARM || a_ptr->tval == TV_SWORD
					|| a_ptr->tval == TV_BOW) && (a_ptr->to_d < 10))
				{
					a_ptr->to_d += randint0(6);
					LOG_PRINT1("Redeeming crappy weapon: +dam now %d\n", a_ptr->to_d);
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
			msg_format("Warning!  Couldn't get appropriate power level.");
			LOG_PRINT("Warning!  Couldn't get appropriate power level.\n");
			message_flush();
		}
	}

	if (a_ptr->cost < 0) a_ptr->cost = 0;

	/* Restore some flags */
	if (activates) a_ptr->flags3 |= TR3_ACTIVATE;
	if (a_ptr->tval == TV_LITE) a_ptr->flags3 |= TR3_NO_FUEL;
	if (a_idx < ART_MIN_NORMAL) a_ptr->flags3 |= TR3_INSTA_ART;

	/*
	 * Add TR3_HIDE_TYPE to all artifacts with nonzero pval because we're
	 * too lazy to find out which ones need it and which ones don't.
	 */
	if (a_ptr->pval)
		a_ptr->flags3 |= TR3_HIDE_TYPE;

	/* Success */

	LOG_PRINT(">>>>>>>>>>>>>>>>>>>>>>>>>> ARTIFACT COMPLETED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	LOG_PRINT2("Number of tries for artifact %d was: %d\n", a_idx, tries);
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

	LOG_PRINT1("Deficit amount for swords is %d\n", swords);
	LOG_PRINT1("Deficit amount for polearms is %d\n", polearms);
	LOG_PRINT1("Deficit amount for blunts is %d\n", blunts);
	LOG_PRINT1("Deficit amount for bows is %d\n", bows);
	LOG_PRINT1("Deficit amount for bodies is %d\n", bodies);
	LOG_PRINT1("Deficit amount for shields is %d\n", shields);
	LOG_PRINT1("Deficit amount for cloaks is %d\n", cloaks);
	LOG_PRINT1("Deficit amount for hats is %d\n", hats);
	LOG_PRINT1("Deficit amount for gloves is %d\n", gloves);
	LOG_PRINT1("Deficit amount for boots is %d\n", boots);

	if (swords > 0 || polearms > 0 || blunts > 0 || bows > 0 ||
	    bodies > 0 || shields > 0 || cloaks > 0 || hats > 0 ||
	    gloves > 0 || boots > 0)
	{
		if (verbose)
		{
			char types[256];
			sprintf(types, "%s%s%s%s%s%s%s%s%s%s",
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

			msg_format("Restarting generation process: not enough%s", types);
			LOG_PRINT1("Restarting generation process: not enough%s", types);
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
		base_art_rarity = C_ZNEW(z_info->a_max, byte);

		/* Open the log file for writing */
		if (verbose)
		{
			char buf[1024];
			path_build(buf, sizeof(buf), ANGBAND_DIR_USER, 
				"randart.log");
			log_file = file_open(buf, MODE_WRITE, FTYPE_TEXT);
			if (!log_file)
			{
				msg_print("Error - can't open randart.log for writing.");
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
				msg_print("Error - can't close randart.log file.");
				exit(1);
			}
		}

		/* Free the "original powers" arrays */
		FREE(base_power);
		FREE(base_item_level);
		FREE(base_item_prob);
		FREE(base_art_rarity);
	}

	/* When done, resume use of the Angband "complex" RNG. */
	Rand_quick = FALSE;

	return (err);
}
