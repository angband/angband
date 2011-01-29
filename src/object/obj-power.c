/*
 * File: obj-power.c
 * Purpose: calculation of object power
 *
 * Copyright (c) 2001 Chris Carr, Chris Robertson
 * Revised in 2009-10 by Chris Carr, Peter Denison
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
#include "init.h"
#include "effects.h"

/*
 * Constants for the power algorithm:
 * - ammo damage for calculating launcher power
 * (the current values assume normal (non-seeker) ammo enchanted to +9)
 * - launcher multipliers for calculating ammo power
 * (these are halved in the algorithm)
 * - fudge factor for extra damage from rings etc. (used if extra blows)
 * - assumed damage for ring brands
 * - base power for light sources (additional power for NFUEL is added later)
 * - base power for jewelry
 * - base power for armour items (for halving acid damage)
 * - power per point of damage
 * - power per point of +to_hit
 * - power per point of base AC
 * - power per point of +to_ac
 * (these four are all halved in the algorithm)
 * - assumed max blows
 * - fudge factor for rescaling missile power
 * (shots are currently set to be 1.25x as powerful as blows)
 * - inhibiting values for +blows/might/shots/immunities (max is one less)
 * - power per unit pval for each pval ability (except speed)
 * (there is an extra term for multiple pval bonuses)
 * - additional power for full rbase set (on top of arithmetic progression)
 * - additional power for full sustain set (ditto, excludes susCHR)
 */
#define AVG_SLING_AMMO_DAMAGE  10
#define AVG_BOW_AMMO_DAMAGE    12
#define AVG_XBOW_AMMO_DAMAGE   14
#define AVG_SLING_MULT          4 /* i.e. 2 */
#define AVG_BOW_MULT            5 /* i.e. 2.5 */
#define AVG_XBOW_MULT           7 /* i.e. 3.5 */
#define AVG_LAUNCHER_DMG		9
#define MELEE_DAMAGE_BOOST     30 /* fudge to boost extra blows */
#define RING_BRAND_DMG	       60 /* fudge to boost off-weapon brand power */
#define BASE_LIGHT_POWER        6
#define BASE_JEWELRY_POWER		4
#define BASE_ARMOUR_POWER		1
#define DAMAGE_POWER            5 /* i.e. 2.5 */
#define TO_HIT_POWER            3 /* i.e. 1.5 */
#define BASE_AC_POWER           2 /* i.e. 1 */
#define TO_AC_POWER             2 /* i.e. 1 */
#define MAX_BLOWS               5
#define BOW_RESCALER           15 /* i.e. 1.5 */
#define INHIBIT_BLOWS           4
#define INHIBIT_MIGHT           4
#define INHIBIT_SHOTS           4
#define IMMUNITY_POWER         25 /* for each immunity after the first */
#define INHIBIT_IMMUNITIES      4
#define STR_POWER	        	9
#define INT_POWER	        	5
#define WIS_POWER	        	5
#define DEX_POWER				6
#define CON_POWER	    	   12
#define CHR_POWER				2
#define STEALTH_POWER			8
#define SEARCH_POWER			2
#define INFRA_POWER				4
#define TUNN_POWER				2
#define RBASE_POWER				5
#define SUST_POWER				5

/*
 * Table giving speed power ratings
 * We go up to +20 here, but in practice it will never get there
 */
static s16b speed_power[21] =
/*	{0, 10, 21, 33, 46, 60, 75, 91, 108, 126, 145,
	163, 180, 196, 211, 225, 238, 250, 261, 271, 280}; */
	{0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200,
	220, 240, 260, 280, 300, 320, 340, 360, 380, 400};

/*
 * Boost ratings for combinations of ability bonuses
 * We go up to +24 here - anything higher is inhibited
 * N.B. Not all stats count equally towards this total
 */
static s16b ability_power[25] =
	{0, 0, 0, 0, 0, 0, 0, 2, 4, 6, 8,
	12, 16, 20, 24, 30, 36, 42, 48, 56, 64,
	74, 84, 96, 110};

/*
 * Calculate the rating for a given slay combination
 */
static s32b slay_power(const object_type *o_ptr, int verbose, ang_file*
	log_file, bool known)
{
	bitflag s_index[OF_SIZE], f[OF_SIZE];
	s32b sv = 0;
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

	/* Combine the slay bytes into an index value */
	of_copy(s_index, f);
	flags_mask(s_index, OF_SIZE, OF_ALL_SLAY_MASK, FLAG_END);

	/* Look in the cache to see if we know this one yet */
	sv = check_slay_cache(s_index);

	/* If it's cached (or there are no slays), return the value */
	if (sv)
	{
		LOG_PRINT("Slay cache hit\n");
		return sv;
	}

	/*
	 * Otherwise we need to calculate the expected average multiplier
	 * for this combination (multiplied by the total number of
	 * monsters, which we'll divide out later).
	 */
	for (i = 0; i < z_info->r_max; i++)
	{
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
		sv += mult * r_ptr->power;
	}

	/*
	 * To get the expected damage for this weapon, multiply the
	 * average damage from base dice by sv, and divide by the
	 * total number of monsters.
	 */
	if (verbose) {
		/* Write info about the slay combination and multiplier */
		LOG_PRINT("Slay multiplier for: ");

		j = list_slays(s_index, s_index, desc, brand, s_mult, FALSE);

		for (i = 0; i < j; i++) {
			if (brand[i]) {
				LOG_PRINT(brand[i]);
			}
			else {
				LOG_PRINT(desc[i]);
				LOG_PRINT1("x%d", s_mult[i]); 
			}

			LOG_PRINT(" ");
		}

		LOG_PRINT1("\nsv is: %d\n", sv);
		LOG_PRINT1(" and t_m_p is: %d \n", tot_mon_power);

		LOG_PRINT1("times 1000 is: %d\n", (1000 * sv) / tot_mon_power);
	}

	/* Add to the cache */
	if (fill_slay_cache(s_index, sv))
		LOG_PRINT("Added to slay cache\n");

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
	s32b p = 0;
	int immunities = 0;
	int misc = 0;
	int lowres = 0;
	int highres = 0;
	int sustains = 0;
	int extra_stat_bonus = 0;
	int i, j;
	bitflag flags[OF_SIZE], mask[OF_SIZE], pval_flags[MAX_PVALS][OF_SIZE];

	/* Extract the flags */
	if (known) {
		LOG_PRINT("Object is known\n");
		object_flags(o_ptr, flags);
	} else {
		LOG_PRINT("Object is not fully known\n");
		object_flags_known(o_ptr, flags);
	}

	if (verbose) {
		LOG_PRINT("Object flags =");
		for (i = 0; i < (int)OF_SIZE; i++)
			LOG_PRINT1(" %02x", flags[i]);
		LOG_PRINT("\n");
		if (o_ptr->num_pvals) {
			object_pval_flags(o_ptr, pval_flags);
			for (j = 0; j < o_ptr->num_pvals; j++) {
				LOG_PRINT1("PVAL %d flags =", j);
				for (i = 0; i < (int)OF_SIZE; i++)
					LOG_PRINT1(" %02x", pval_flags[j][i]);
				LOG_PRINT("\n");
			}
		}
	}

	/* Evaluate certain abilities based on type of object. */
	switch (o_ptr->tval)
	{
		case TV_BOW:
		{
			int mult;

			/*
			 * Damage multiplier for bows should be weighted less than that
			 * for melee weapons, since players typically get fewer shots
			 * than hits (note, however, that the multipliers are applied
			 * afterwards in the bow calculation, not before as for melee
			 * weapons, which tends to bring these numbers back into line).
			 * ToDo: rework evaluation of negative pvals
			 */

			p += (o_ptr->to_d * DAMAGE_POWER / 2);
			LOG_PRINT1("Adding power from to_dam, total is %d\n", p);

			/*
			 * Add the average damage of fully enchanted (good) ammo for this
			 * weapon.  Could make this dynamic based on k_info if desired.
			 * ToDo: precisely that.
			 */

			if (o_ptr->sval == SV_SLING)
			{
				p += (AVG_SLING_AMMO_DAMAGE * DAMAGE_POWER / 2);
			}
			else if (o_ptr->sval == SV_SHORT_BOW ||
				o_ptr->sval == SV_LONG_BOW)
			{
				p += (AVG_BOW_AMMO_DAMAGE * DAMAGE_POWER / 2);
			}
			else if (o_ptr->sval == SV_LIGHT_XBOW ||
				o_ptr->sval == SV_HEAVY_XBOW)
			{
				p += (AVG_XBOW_AMMO_DAMAGE * DAMAGE_POWER / 2);
			}

			LOG_PRINT1("Adding power from ammo, total is %d\n", p);

			mult = bow_multiplier(o_ptr->sval);

			LOG_PRINT1("Base multiplier for this weapon is %d\n", mult);

			if (of_has(flags, OF_MIGHT) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_MIGHT || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					mult = 1;	/* don't overflow */
					LOG_PRINT("INHIBITING - too much extra might\n");
				}
				else
				{
					mult += o_ptr->pval[DEFAULT_PVAL];
				}
				LOG_PRINT1("Extra might multiple is %d\n", mult);
			}
			p *= mult;
			LOG_PRINT2("Multiplying power by %d, total is %d\n", mult, p);

			if (of_has(flags, OF_SHOTS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra shots: %d\n", o_ptr->pval[DEFAULT_PVAL]);

				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_SHOTS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING - too many extra shots\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p = (p * (1 + o_ptr->pval[DEFAULT_PVAL]));
					LOG_PRINT2("Multiplying power by 1 + %d, total is %d\n",
						o_ptr->pval[DEFAULT_PVAL], p);
				}
			}

			/* Apply the correct slay multiplier */
			p = (p * slay_power(o_ptr, verbose, log_file, known)) / tot_mon_power;
			LOG_PRINT1("Adjusted for slay power, total is %d\n", p);

			if (o_ptr->weight < o_ptr->kind->weight)
			{
				p++;
				LOG_PRINT("Incrementing power by one for low weight\n");
			}

			/*
			 * Correction to match ratings to melee damage ratings.
			 * We multiply all missile weapons by 1.5 in order to compare damage.
			 * (CR 11/20/01 - changed this to 1.25).
			 * (CC 25/07/10 - changed this back to 1.5).
			 * Melee weapons assume MAX_BLOWS per turn, so we must
			 * also divide by MAX_BLOWS to get equal ratings.
			 */
			p = sign(p) * (ABS(p) * BOW_RESCALER / (10 * MAX_BLOWS));
			LOG_PRINT1("Rescaling bow power, total is %d\n", p);

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER / 2);
			LOG_PRINT1("Adding power from to_hit, total is %d\n", p);

			break;
		}
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			p += (o_ptr->dd * (o_ptr->ds + 1) * DAMAGE_POWER / 4);
			LOG_PRINT1("Adding power for dam dice, total is %d\n", p);

			/* Apply the correct slay multiplier */
			p = (p * slay_power(o_ptr, verbose, log_file, known)) / tot_mon_power;
			LOG_PRINT1("Adjusted for slay power, total is %d\n", p);

			p += (o_ptr->to_d * DAMAGE_POWER / 2);
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			if (of_has(flags, OF_BLOWS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra blows: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_BLOWS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING, too many extra blows or a negative number\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p = sign(p) * ((ABS(p) * (MAX_BLOWS + o_ptr->pval[DEFAULT_PVAL])) 
						/ MAX_BLOWS);
					/* Add an extra amount per extra blow to account for damage/branding rings */
					p += ((MELEE_DAMAGE_BOOST + RING_BRAND_DMG) * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER / (2 * MAX_BLOWS));
					LOG_PRINT1("Adding power for blows, total is %d\n", p);
				}
			}

			/*
			 * add extra power for multiple slays/brands, as these
			 * add diminishing amounts to average damage
			 */
			flags_init(mask, OF_SIZE, OF_ALL_SLAY_MASK, FLAG_END);
			i = list_slays(flags, mask, NULL, NULL, NULL, FALSE);
			if (i > 1)
				p += (i * 3);
			LOG_PRINT1("Adding power for multiple slays/brands, total is %d\n", p);

			/* add launcher bonus for ego ammo, and multiply */
			if (o_ptr->tval == TV_SHOT)
			{
				if (o_ptr->name2) p += (AVG_LAUNCHER_DMG * DAMAGE_POWER / 2);
				p = p * AVG_SLING_MULT * BOW_RESCALER / (20 * MAX_BLOWS);
			}
			if (o_ptr->tval == TV_ARROW)
			{
				if (o_ptr->name2) p += (AVG_LAUNCHER_DMG * DAMAGE_POWER / 2);
				p = p * AVG_BOW_MULT * BOW_RESCALER / (20 * MAX_BLOWS);
			}
			if (o_ptr->tval == TV_BOLT)
			{
				if (o_ptr->name2) p += (AVG_LAUNCHER_DMG * DAMAGE_POWER / 2);
				p = p * AVG_XBOW_MULT * BOW_RESCALER / (20 * MAX_BLOWS);
			}
			LOG_PRINT1("After multiplying ammo and rescaling, power is %d\n", p);

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER / 2);
			LOG_PRINT1("Adding power for to hit, total is %d\n", p);

			/* Remember, weight is in 0.1 lb. units. */
			if (o_ptr->weight < o_ptr->kind->weight)
			{
				p += (o_ptr->kind->weight - o_ptr->weight) / 20;
				LOG_PRINT1("Adding power for low weight, total is %d\n", p);
			}

			break;
		}
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			p += BASE_ARMOUR_POWER;
			LOG_PRINT1("Armour item, base power is %d\n", p);

			p += sign(o_ptr->ac) * ((ABS(o_ptr->ac) * BASE_AC_POWER) / 2);
			LOG_PRINT1("Adding power for base AC value, total is %d\n", p);

			/* Add power for AC per unit weight */
			if (o_ptr->weight > 0)
			{
				i = 1000 * (o_ptr->ac + o_ptr->to_a) /
					o_ptr->weight;

				/* Stop overpricing Elven Cloaks */
				if (i > 400) i = 400;

				/* Adjust power */
				p *= i;
				p /= 100;
			}
			/* Weightless (ethereal) items get fixed boost */
			else p *= 5;

			/* Add power for +hit and +dam */
			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += o_ptr->to_d * DAMAGE_POWER;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			/* Apply the correct brand/slay multiplier */
			p += (((2 * (o_ptr->to_d + RING_BRAND_DMG)
				* slay_power(o_ptr, verbose, log_file, known))
				/ tot_mon_power) - (2 * (o_ptr->to_d + RING_BRAND_DMG)));
			LOG_PRINT1("Adjusted for brand/slay power, total is %d\n", p);

			/* Add power for extra blows */
			if (of_has(flags, OF_BLOWS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra blows: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_BLOWS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING, too many extra blows or a negative number\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p += ((MELEE_DAMAGE_BOOST + RING_BRAND_DMG + o_ptr->to_d) * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER / MAX_BLOWS);
					LOG_PRINT1("Adding power for extra blows, total is %d\n", p);
				}
			}

			/* Add power for extra shots */
			if (of_has(flags, OF_SHOTS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra shots: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_SHOTS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING - too many extra shots\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p += ((AVG_XBOW_AMMO_DAMAGE + AVG_LAUNCHER_DMG) * AVG_XBOW_MULT * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER * BOW_RESCALER / (20 * MAX_BLOWS));
					LOG_PRINT1("Adding power for extra shots - total is %d\n", p);
				}
			}

			break;
		}
		case TV_LIGHT:
		{
			p += BASE_LIGHT_POWER;
			LOG_PRINT("Light source, adding base power\n");

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += o_ptr->to_d * DAMAGE_POWER;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			/* Apply the correct brand/slay multiplier */
			p += (((2 * (o_ptr->to_d + RING_BRAND_DMG)
				* slay_power(o_ptr, verbose, log_file, known))
				/ tot_mon_power) - (2 * (o_ptr->to_d + RING_BRAND_DMG)));
			LOG_PRINT1("Adjusted for brand/slay power, total is %d\n", p);

			/* Add power for extra blows */
			if (of_has(flags, OF_BLOWS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra blows: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_BLOWS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING, too many extra blows or a negative number\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p += ((MELEE_DAMAGE_BOOST + RING_BRAND_DMG + o_ptr->to_d) * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER / MAX_BLOWS);
					LOG_PRINT1("Adding power for extra blows, total is %d\n", p);
				}
			}

			/* Add power for extra shots */
			if (of_has(flags, OF_SHOTS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra shots: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_SHOTS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING - too many extra shots\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p += ((AVG_XBOW_AMMO_DAMAGE + AVG_LAUNCHER_DMG) * AVG_XBOW_MULT * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER * BOW_RESCALER / (20 * MAX_BLOWS));
					LOG_PRINT1("Adding power for extra shots - total is %d\n", p);
				}
			}

			/*
			 * Big boost for extra light radius
			 * n.b. Another few points are added below
			 */
			if (of_has(flags, OF_LIGHT)) p += 30;

			break;
		}
		case TV_RING:
		case TV_AMULET:
		{
			p += BASE_JEWELRY_POWER;
			LOG_PRINT("Jewellery - adding base power\n");

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += o_ptr->to_d * DAMAGE_POWER;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			/* Apply the correct brand/slay multiplier */
			p += (((2 * (o_ptr->to_d + RING_BRAND_DMG)
				* slay_power(o_ptr, verbose, log_file, known))
				/ tot_mon_power) - (2 * (o_ptr->to_d + RING_BRAND_DMG)));
			LOG_PRINT1("Adjusted for brand/slay power, total is %d\n", p);

			/* Add power for extra blows */
			if (of_has(flags, OF_BLOWS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra blows: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_BLOWS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING, too many extra blows or a negative number\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p += ((MELEE_DAMAGE_BOOST + RING_BRAND_DMG + o_ptr->to_d) * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER / MAX_BLOWS);
					LOG_PRINT1("Adding power for extra blows, total is %d\n", p);
				}
			}

			/* Add power for extra shots */
			if (of_has(flags, OF_SHOTS) &&
			    (known || object_pval_is_visible(o_ptr)))
			{
				LOG_PRINT1("Extra shots: %d\n", o_ptr->pval[DEFAULT_PVAL]);
				if (o_ptr->pval[DEFAULT_PVAL] >= INHIBIT_SHOTS || o_ptr->pval[DEFAULT_PVAL] < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING - too many extra shots\n");
				}
				else if (o_ptr->pval[DEFAULT_PVAL] > 0)
				{
					p += ((AVG_XBOW_AMMO_DAMAGE + AVG_LAUNCHER_DMG) * AVG_XBOW_MULT * o_ptr->pval[DEFAULT_PVAL] * DAMAGE_POWER * BOW_RESCALER / (20 * MAX_BLOWS));
					LOG_PRINT1("Adding power for extra shots - total is %d\n", p);
				}
			}

			break;
		}
	}

	/* Other abilities are evaluated independent of the object type. */
	p += sign(o_ptr->to_a) * (ABS(o_ptr->to_a) * TO_AC_POWER / 2);
	LOG_PRINT2("Adding power for to_ac of %d, total is %d\n", o_ptr->to_a, p);

	if (o_ptr->to_a > HIGH_TO_AC)
	{
		p += ((o_ptr->to_a - (HIGH_TO_AC - 1)) * TO_AC_POWER / 2);
		LOG_PRINT1("Adding power for high to_ac value, total is %d\n", p);
	}
	if (o_ptr->to_a > VERYHIGH_TO_AC)
	{
		p += ((o_ptr->to_a - (VERYHIGH_TO_AC -1)) * TO_AC_POWER / 2);
		LOG_PRINT1("Adding power for very high to_ac value, total is %d\n", p);
	}
	if (o_ptr->to_a >= INHIBIT_AC)
	{
		p += INHIBIT_POWER;	/* inhibit */
		LOG_PRINT("INHIBITING: AC bonus too high\n");
	}

	if ((o_ptr->pval[DEFAULT_PVAL] > 0) && (known || object_pval_is_visible(o_ptr)))
	{
		if (of_has(flags, OF_STR))
		{
			p += STR_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for STR bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_INT))
		{
			p += INT_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for INT bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_WIS))
		{
			p += WIS_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for WIS bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_DEX))
		{
			p += DEX_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for DEX bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_CON))
		{
			p += CON_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for CON bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_STEALTH))
		{
			p += STEALTH_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for Stealth bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_SEARCH))
		{
			p += SEARCH_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for searching bonus %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL] , p);
		}
		/* Add extra power term if there are a lot of ability bonuses */
		if (o_ptr->pval[DEFAULT_PVAL] > 0)
		{
			extra_stat_bonus += (of_has(flags, OF_STR) ? 1 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_INT) ? 1 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_WIS) ? 1 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_DEX) ? 1 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_CON) ? 1 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_CHR) ? 0 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_STEALTH) ? 1 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_INFRA) ? 0 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_TUNNEL) ? 0 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_SEARCH) ? 0 * o_ptr->pval[DEFAULT_PVAL] : 0);
			extra_stat_bonus += (of_has(flags, OF_SPEED) ? 0 * o_ptr->pval[DEFAULT_PVAL] : 0);

			if (o_ptr->tval == TV_BOW)
			{
				extra_stat_bonus += (of_has(flags, OF_MIGHT) ? 5 * o_ptr->pval[DEFAULT_PVAL] / 2 : 0);
				extra_stat_bonus += (of_has(flags, OF_SHOTS) ? 3 * o_ptr->pval[DEFAULT_PVAL] : 0);
			}
			else if ( (o_ptr->tval == TV_DIGGING) || (o_ptr->tval == TV_HAFTED) ||
			          (o_ptr->tval == TV_POLEARM) || (o_ptr->tval == TV_SWORD) )
			{
				extra_stat_bonus += (of_has(flags, OF_BLOWS) ? 3 * o_ptr->pval[DEFAULT_PVAL] : 0);
			}

			if (extra_stat_bonus > 24)
			{
				/* Inhibit */
				LOG_PRINT1("Inhibiting!  (Total ability bonus of %d is too high)\n", extra_stat_bonus);
				p += INHIBIT_POWER;
			}
			else
			{
				p += ability_power[extra_stat_bonus];
				LOG_PRINT2("Adding power for combination of %d, total is %d\n", ability_power[extra_stat_bonus], p);
			}
		}

	}
	else if ((o_ptr->pval[DEFAULT_PVAL] < 0) && (known || object_pval_is_visible(o_ptr)))
	{
		if (of_has(flags, OF_STR)) p += 4 * o_ptr->pval[DEFAULT_PVAL];
		if (of_has(flags, OF_INT)) p += 2 * o_ptr->pval[DEFAULT_PVAL];
		if (of_has(flags, OF_WIS)) p += 2 * o_ptr->pval[DEFAULT_PVAL];
		if (of_has(flags, OF_DEX)) p += 3 * o_ptr->pval[DEFAULT_PVAL];
		if (of_has(flags, OF_CON)) p += 4 * o_ptr->pval[DEFAULT_PVAL];
		if (of_has(flags, OF_STEALTH)) p += o_ptr->pval[DEFAULT_PVAL];
		LOG_PRINT1("Subtracting power for negative ability values, total is %d\n", p);
	}

	if (known || object_pval_is_visible(o_ptr))
	{
		if (of_has(flags, OF_CHR))
		{
			p += CHR_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for CHR bonus/penalty %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_INFRA))
		{
			p += INFRA_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for infra bonus/penalty %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_TUNNEL))
		{
			p += TUNN_POWER * o_ptr->pval[DEFAULT_PVAL];
			LOG_PRINT2("Adding power for tunnelling bonus/penalty %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
		if (of_has(flags, OF_SPEED))
		{
			p += sign(o_ptr->pval[DEFAULT_PVAL]) * speed_power[ABS(o_ptr->pval[DEFAULT_PVAL])];
			LOG_PRINT2("Adding power for speed bonus/penalty %d, total is %d\n", o_ptr->pval[DEFAULT_PVAL], p);
		}
	}

#define ADD_POWER1(string, val, flag) \
	if (of_has(flags, flag)) { \
		p += (val); \
		LOG_PRINT1("Adding power for " string ", total is %d\n", p); \
	}

#define ADD_POWER2(string, val, flag, extra) \
	if (of_has(flags, flag)) { \
		p += (val); \
		extra; \
		LOG_PRINT1("Adding power for " string ", total is %d\n", p); \
	}

	ADD_POWER2("sustain STR",         9, OF_SUST_STR, sustains++);
	ADD_POWER2("sustain INT",         4, OF_SUST_INT, sustains++);
	ADD_POWER2("sustain WIS",         4, OF_SUST_WIS, sustains++);
	ADD_POWER2("sustain DEX",         7, OF_SUST_DEX, sustains++);
	ADD_POWER2("sustain CON",         8, OF_SUST_CON, sustains++);
	ADD_POWER1("sustain CHR",         1, OF_SUST_CHR);

	for (i = 2; i <= sustains; i++)
	{
		p += i;
		LOG_PRINT1("Adding power for multiple sustains, total is %d\n", p);
		if (i == 5)
		{
			p += SUST_POWER;
			LOG_PRINT1("Adding power for full set of sustains, total is %d\n", p);
		}
	}

	ADD_POWER2("acid immunity",      38, OF_IM_ACID, immunities++);
	ADD_POWER2("elec immunity",      35, OF_IM_ELEC, immunities++);
	ADD_POWER2("fire immunity",      40, OF_IM_FIRE, immunities++);
	ADD_POWER2("cold immunity",      37, OF_IM_COLD, immunities++);

	for (i = 2; i <= immunities; i++)
	{
		p += IMMUNITY_POWER;
		LOG_PRINT1("Adding power for multiple immunities, total is %d\n", p);
		if (i >= INHIBIT_IMMUNITIES)
		{
			p += INHIBIT_POWER;             /* inhibit */
			LOG_PRINT("INHIBITING: Too many immunities\n");
		}
	}

	ADD_POWER2("free action",           14, OF_FREE_ACT,    misc++);
	ADD_POWER2("hold life",             12, OF_HOLD_LIFE,   misc++);
	ADD_POWER1("feather fall",           1, OF_FEATHER);
	ADD_POWER2("permanent light",        3, OF_LIGHT,       misc++);
	ADD_POWER2("see invisible",         10, OF_SEE_INVIS,   misc++);
	ADD_POWER2("telepathy",             70, OF_TELEPATHY,   misc++);
	ADD_POWER2("slow digestion",         2, OF_SLOW_DIGEST, misc++);
	ADD_POWER2("resist acid",            5, OF_RES_ACID,    lowres++);
	ADD_POWER2("resist elec",            6, OF_RES_ELEC,    lowres++);
	ADD_POWER2("resist fire",            6, OF_RES_FIRE,    lowres++);
	ADD_POWER2("resist cold",            6, OF_RES_COLD,    lowres++);
	ADD_POWER2("resist poison",         28, OF_RES_POIS,    highres++);
	ADD_POWER2("resist fear",            6, OF_RES_FEAR,    highres++);
	ADD_POWER2("resist light",           6, OF_RES_LIGHT,   highres++);
	ADD_POWER2("resist dark",           16, OF_RES_DARK,    highres++);
	ADD_POWER2("resist blindness",      16, OF_RES_BLIND,   highres++);
	ADD_POWER2("resist confusion",      24, OF_RES_CONFU,   highres++);
	ADD_POWER2("resist sound",          14, OF_RES_SOUND,   highres++);
	ADD_POWER2("resist shards",          8, OF_RES_SHARD,   highres++);
	ADD_POWER2("resist nexus",          15, OF_RES_NEXUS,   highres++);
	ADD_POWER2("resist nether",         20, OF_RES_NETHR,   highres++);
	ADD_POWER2("resist chaos",          20, OF_RES_CHAOS,   highres++);
	ADD_POWER2("resist disenchantment", 20, OF_RES_DISEN,   highres++);
	ADD_POWER2("regeneration",           9, OF_REGEN,       misc++);
	ADD_POWER1("blessed",                1, OF_BLESSED);
	ADD_POWER1("no fuel",                5, OF_NO_FUEL);

	for (i = 2; i <= misc; i++)
	{
		p += i;
		LOG_PRINT1("Adding power for multiple misc abilities, total is %d\n", p);
	}

	for (i = 2; i <= lowres; i++)
	{
		p += i;
		LOG_PRINT1("Adding power for multiple low resists, total is %d\n", p);
		if (i == 4)
		{
			p += RBASE_POWER;
			LOG_PRINT1("Adding power for full rbase set, total is %d\n", p);
		}
	}

	for (i = 2; i <= highres; i++)
	{
		p += (i * 2);
		LOG_PRINT1("Adding power for multiple high resists, total is %d\n", p);
	}

	/* Note: the following code is irrelevant until curses are reworked */
	if (of_has(flags, OF_TELEPORT))
	{
		p -= 1;
		LOG_PRINT1("Subtracting power for teleportation, total is %d\n", p);
	}
	if (of_has(flags, OF_DRAIN_EXP))
	{
		p -= 1;
		LOG_PRINT1("Subtracting power for drain experience, total is %d\n", p);
	}
	if (of_has(flags, OF_AGGRAVATE))
	{
		p -= 1;
		LOG_PRINT1("Subtracting power for aggravation, total is %d\n", p);
	}
	if (of_has(flags, OF_LIGHT_CURSE))
	{
		p -= 1;
		LOG_PRINT1("Subtracting power for light curse, total is %d\n", p);
	}
	if (of_has(flags, OF_HEAVY_CURSE))
	{
		p -= 1;
		LOG_PRINT1("Subtracting power for heavy curse, total is %d\n", p);
	}

	/*	if (of_has(flags, OF_PERMA_CURSE)) p -= 40; */

	/* add power for effect */
	if (known || object_effect_is_known(o_ptr))
	{
		if (o_ptr->name1 && a_info[o_ptr->name1].effect)
		{
			p += effect_power(a_info[o_ptr->name1].effect);
			LOG_PRINT1("Adding power for artifact activation, total is %d\n", p);
		}
		else
		{
			p += effect_power(o_ptr->kind->effect);
			LOG_PRINT1("Adding power for item activation, total is %d\n", p);
		}
	}

	/* add tiny amounts for ignore flags */
	if (of_has(flags, OF_IGNORE_ACID)) p++;
	if (of_has(flags, OF_IGNORE_FIRE)) p++;
	if (of_has(flags, OF_IGNORE_COLD)) p++;
	if (of_has(flags, OF_IGNORE_ELEC)) p++;

	LOG_PRINT1("After ignore flags, FINAL POWER IS %d\n", p);

	return (p);
}
