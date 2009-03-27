/*
 * File: obj-power.c
 * Purpose: calculation of object power
 *
 * Copyright (c) 2001 Chris Carr, Chris Robertson
 * Revised in 2009 by Chris Carr, Peter Denison
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

/* Total number of different slay types used
 * ToDo: reduce this to cache only the slays found in ego-item.txt
 */
#define SLAY_MAX 0x00010000L

/*
 * Constants for the power algorithm:
 * - ammo damage for calculating launcher power
 * (the current values assume normal (non-seeker) ammo enchanted to +9)
 * - launcher multipliers for calculating ammo power
 * (these are halved in the algorithm)
 * - fudge factor for extra damage from rings etc. (used if extra blows)
 * - base power for light sources (additional power for NFUEL is added later)
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
 */
#define AVG_SLING_AMMO_DAMAGE  11
#define AVG_BOW_AMMO_DAMAGE    12
#define AVG_XBOW_AMMO_DAMAGE   13
#define AVG_SLING_MULT          4 /* i.e. 2 */
#define AVG_BOW_MULT            5 /* i.e. 2.5 */
#define AVG_XBOW_MULT           7 /* i.e. 3.5 */
#define MELEE_DAMAGE_BOOST      5
#define BASE_LITE_POWER         6
#define DAMAGE_POWER            4 /* i.e. 2 */
#define TO_HIT_POWER            2 /* i.e. 1 */
#define BASE_AC_POWER           3 /* i.e. 1.5 */
#define TO_AC_POWER             2 /* i.e. 1 */
#define MAX_BLOWS               5
#define BOW_RESCALER            4
#define INHIBIT_BLOWS           4
#define INHIBIT_MIGHT           4
#define INHIBIT_SHOTS           4
#define IMMUNITY_POWER         25 /* for each immunity after the first */
#define INHIBIT_IMMUNITIES      4
#define STR_POWER	        6
#define INT_POWER	        4
#define WIS_POWER	        4
#define DEX_POWER		6
#define CON_POWER		8
#define CHR_POWER		1
#define STEALTH_POWER		4
#define SEARCH_POWER		2
#define INFRA_POWER		2
#define TUNN_POWER		2

/*
 * Table giving speed power ratings
 * We go up to +20 here, but in practice it will never get there
 */
static s16b speed_power[21] =
	{0, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132,
	152, 170, 186, 200, 212, 224, 236, 248, 260, 272};

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
 * Cache the results of slay_value(), which is expensive and would
 * otherwise be called much too often.
 */
static s32b *slays;

/*
 * Free object power info.
 */
void free_obj_power(void)
{
	FREE(slays);
}

/*
 * Initialise the arrays used in working out object (ego and artifact)
 * power.
 */
bool init_obj_power(void)
{
	/* Allocate the "slay values" array */
	slays = C_ZNEW(SLAY_MAX, s32b);
	
	return TRUE;
}

/*
 * Calculate the rating for a given slay combination
 * ToDo: rewrite to use an external structure for slays
 */

static s32b slay_power(const object_type *o_ptr, int verbose, ang_file* log_file)
{
	s32b s_index = 0;
	s32b sv;
	int i;
	int mult;
	monster_race *r_ptr;
	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Combine the slay bytes into an index value
	 * For now we do not support the two undefined slays (XXX),
	 * but this could be added
	 */

	if (f1 & TR1_SLAY_ANIMAL) s_index |= 0x0001;
	if (f1 & TR1_SLAY_EVIL) s_index |= 0x0002;
	if (f1 & TR1_SLAY_UNDEAD) s_index |= 0x0004;
	if (f1 & TR1_SLAY_DEMON) s_index |= 0x0008;
	if (f1 & TR1_SLAY_ORC) s_index |= 0x0010;
	if (f1 & TR1_SLAY_TROLL) s_index |= 0x0020;
	if (f1 & TR1_SLAY_GIANT) s_index |= 0x0040;
	if (f1 & TR1_SLAY_DRAGON) s_index |= 0x0080;
	if (f1 & TR1_KILL_DRAGON) s_index |= 0x0100;
	if (f1 & TR1_KILL_DEMON) s_index |= 0x0200;
	if (f1 & TR1_KILL_UNDEAD) s_index |= 0x0400;

	if (f1 & TR1_BRAND_POIS) s_index |= 0x0800;
	if (f1 & TR1_BRAND_ACID) s_index |= 0x1000;
	if (f1 & TR1_BRAND_ELEC) s_index |= 0x2000;
	if (f1 & TR1_BRAND_FIRE) s_index |= 0x4000;
	if (f1 & TR1_BRAND_COLD) s_index |= 0x8000;

	/* Look in the cache to see if we know this one yet */

	sv = slays[s_index];

	/* If it's cached, return its value */

	if(sv) return slays[s_index];

	/* Otherwise we need to calculate the expected average multiplier
	 * for this combination (multiplied by the total number of
	 * monsters, which we'll divide out later).
	 */

	sv = 0;

	for(i = 0; i < z_info->r_max; i++) {
		mult = 1;

		r_ptr = &r_info[i];

		/*
		 * Do the following in ascending order so that the best
		 * multiple is retained
		 */
		if ( (r_ptr->flags[2] & RF2_ANIMAL)
		     && (f1 & TR1_SLAY_ANIMAL) )
			mult = 2;
		if ( (r_ptr->flags[2] & RF2_EVIL)
			 && (f1 & TR1_SLAY_EVIL) )
			mult = 2;
		if ( (r_ptr->flags[2] & RF2_UNDEAD)
			 && (f1 & TR1_SLAY_UNDEAD) )
			mult = 3;
		if ( (r_ptr->flags[2] & RF2_DEMON)
			 && (f1 & TR1_SLAY_DEMON) )
			mult = 3;
		if ( (r_ptr->flags[2] & RF2_ORC)
			 && (f1 & TR1_SLAY_ORC) )
			mult = 3;
		if ( (r_ptr->flags[2] & RF2_TROLL)
			 && (f1 & TR1_SLAY_TROLL) )
			mult = 3;
		if ( (r_ptr->flags[2] & RF2_GIANT)
			 && (f1 & TR1_SLAY_GIANT) )
			mult = 3;
		if ( (r_ptr->flags[2] & RF2_DRAGON)
			 && (f1 & TR1_SLAY_DRAGON) )
			mult = 3;

		/* Brands get the multiple if monster is NOT resistant */
		if ( !(r_ptr->flags[2] & RF2_IM_ACID)
		     && (f1 & TR1_BRAND_ACID) )
			mult = 3;
		if ( !(r_ptr->flags[2] & RF2_IM_FIRE)
		     && (f1 & TR1_BRAND_FIRE) )
			mult = 3;
		if ( !(r_ptr->flags[2] & RF2_IM_COLD)
			 && (f1 & TR1_BRAND_COLD) )
			mult = 3;
		if ( !(r_ptr->flags[2] & RF2_IM_ELEC)
			 && (f1 & TR1_BRAND_ELEC) )
			mult = 3;
		if ( !(r_ptr->flags[2] & RF2_IM_POIS)
		     && (f1 & TR1_BRAND_POIS) )
			mult = 3;

		/* Do kill flags last since they have the highest multiplier */
		if ( (r_ptr->flags[2] & RF2_DRAGON)
		      && (f1 & TR1_KILL_DRAGON) )
			mult = 5;
		if ( (r_ptr->flags[2] & RF2_DEMON)
		      && (f1 & TR1_KILL_DEMON) )
			mult = 5;
		if ( (r_ptr->flags[2] & RF2_UNDEAD)
		      && (f1 & TR1_KILL_UNDEAD) )
			mult = 5;

		/* Add the multiple to sv */
		sv += mult * r_ptr->power;

		/* End loop */
	}

	/*
	 * To get the expected damage for this weapon, multiply the
	 * average damage from base dice by sv, and divide by the
	 * total number of monsters.
	 */

	if (verbose)
	{
		/* Write info about the slay combination and multiplier */
		file_putf(log_file,"Slay multiplier for:");

		if (f1 & TR1_SLAY_EVIL) file_putf(log_file,"Evl ");
		if (f1 & TR1_KILL_DRAGON) file_putf(log_file,"XDr ");
		if (f1 & TR1_KILL_DEMON) file_putf(log_file,"XDm ");
		if (f1 & TR1_KILL_UNDEAD) file_putf(log_file,"XUn ");
		if (f1 & TR1_SLAY_ANIMAL) file_putf(log_file,"Ani ");
		if (f1 & TR1_SLAY_UNDEAD) file_putf(log_file,"Und ");
		if (f1 & TR1_SLAY_DRAGON) file_putf(log_file,"Drg ");
		if (f1 & TR1_SLAY_DEMON) file_putf(log_file,"Dmn ");
		if (f1 & TR1_SLAY_TROLL) file_putf(log_file,"Tro ");
		if (f1 & TR1_SLAY_ORC) file_putf(log_file,"Orc ");
		if (f1 & TR1_SLAY_GIANT) file_putf(log_file,"Gia ");

		if (f1 & TR1_BRAND_ACID) file_putf(log_file,"Acd ");
		if (f1 & TR1_BRAND_ELEC) file_putf(log_file,"Elc ");
		if (f1 & TR1_BRAND_FIRE) file_putf(log_file,"Fir ");
		if (f1 & TR1_BRAND_COLD) file_putf(log_file,"Cld ");
		if (f1 & TR1_BRAND_POIS) file_putf(log_file,"Poi ");

		file_putf(log_file,"sv is: %d\n", sv);
		file_putf(log_file," and t_m_p is: %d \n", tot_mon_power);

		file_putf(log_file,"times 1000 is: %d\n", (1000 * sv) / tot_mon_power);
/*		fflush(log_file); - need to convert from ang_file type */
	}

	/* Add to the cache */
	slays[s_index] = sv;

	return sv;
	/* End method */
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
s32b object_power(const object_type* o_ptr, int verbose, ang_file *log_file)
{
	s32b p = 0;
	object_kind *k_ptr;
	int immunities = 0;
	int extra_stat_bonus = 0;
	int i;
	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	k_ptr = &k_info[o_ptr->k_idx];

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

			if (f1 & TR1_MIGHT)
			{
				if (o_ptr->pval >= INHIBIT_MIGHT || o_ptr->pval < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					mult = 1;	/* don't overflow */
					LOG_PRINT("INHIBITING - too much extra might\n");
				}
				else
				{
					mult += o_ptr->pval;
				}
				LOG_PRINT1("Extra might multiple is %d\n", mult);
			}
			p *= mult;
			LOG_PRINT2("Multiplying power by %d, total is %d\n", mult, p);

			if (f1 & TR1_SHOTS)
			{
				LOG_PRINT1("Extra shots: %d\n", o_ptr->pval);

				if (o_ptr->pval >= INHIBIT_SHOTS || o_ptr->pval < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING - too many extra shots\n");
				}
				else if (o_ptr->pval > 0)
				{
					p = (p * (1 + o_ptr->pval));
					LOG_PRINT2("Multiplying power by 1 + %d, total is %d\n", o_ptr->pval, p);
				}
			}

			if (o_ptr->weight < k_ptr->weight)
			{
				p++;
				LOG_PRINT("Incrementing power by one for low weight\n");
			}

			/*
			 * Correction to match ratings to melee damage ratings.
			 * We multiply all missile weapons by 1.5 in order to compare damage.
			 * (CR 11/20/01 - changed this to 1.25).
			 * Melee weapons assume 5 attacks per turn, so we must also divide
			 * by 5 to get equal ratings. 1.25 / 5 = 0.25
			 */
			p = sign(p) * (ABS(p) / BOW_RESCALER);
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

			p = (p * slay_power(o_ptr, verbose, log_file)) / tot_mon_power;
			LOG_PRINT1("Adjusted for slay power, total is %d\n", p);

			p += (o_ptr->to_d * DAMAGE_POWER / 2);
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			if (f1 & TR1_BLOWS)
			{
				LOG_PRINT1("Extra blows: %d\n", o_ptr->pval);
				if (o_ptr->pval >= INHIBIT_BLOWS || o_ptr->pval < 0)
				{
					p += INHIBIT_POWER;	/* inhibit */
					LOG_PRINT("INHIBITING, too many extra blows or a negative number\n");
				}
				else if (o_ptr->pval > 0)
				{
					p = sign(p) * ((ABS(p) * (MAX_BLOWS + o_ptr->pval)) 
						/ MAX_BLOWS);
					/* Add an extra amount per blow to account for damage 						rings */
					p += (MELEE_DAMAGE_BOOST * o_ptr->pval * DAMAGE_POWER / 2);
					LOG_PRINT1("Adding power for blows, total is %d\n", p);
				}
			}

			if (o_ptr->tval == TV_SHOT)  p = p * AVG_SLING_MULT / (2 * BOW_RESCALER);
			if (o_ptr->tval == TV_ARROW) p = p * AVG_BOW_MULT / (2 * BOW_RESCALER);
			if (o_ptr->tval == TV_BOLT)  p = p * AVG_XBOW_MULT / (2 * BOW_RESCALER);
			LOG_PRINT1("After multiplying ammo and rescaling, power is %d\n", p);
			
			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER / 2);
			LOG_PRINT1("Adding power for to hit, total is %d\n", p);

			/* Remember, weight is in 0.1 lb. units. */
			if (o_ptr->weight < k_ptr->weight)
			{
				p += (k_ptr->weight - o_ptr->weight) / 20; 
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
			p += sign(o_ptr->ac) * ((ABS(o_ptr->ac) * BASE_AC_POWER) / 2);
			LOG_PRINT1("Adding power for base AC value, total is %d\n", p);

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += o_ptr->to_d * DAMAGE_POWER;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			if (o_ptr->weight < k_ptr->weight)
			{
				p += (k_ptr->weight - o_ptr->weight) / 10;
				LOG_PRINT1("Adding power for low weight, total is %d\n", p);
			}
			break;
		}
		case TV_LITE:
		{
			p += BASE_LITE_POWER;
			LOG_PRINT("Artifact light source, adding base power\n");

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += o_ptr->to_d * DAMAGE_POWER;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

			break;
		}
		case TV_RING:
		case TV_AMULET:
		{
			LOG_PRINT("Jewellery - adding 0 as base\n");

			p += sign(o_ptr->to_h) * (ABS(o_ptr->to_h) * TO_HIT_POWER);
			LOG_PRINT1("Adding power for to_hit, total is %d\n", p);

			p += o_ptr->to_d * DAMAGE_POWER;
			LOG_PRINT1("Adding power for to_dam, total is %d\n", p);

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

	if (o_ptr->pval > 0)
	{
		if (f1 & TR1_STR)
		{
			p += STR_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for STR bonus %d, total is %d\n", o_ptr->pval, p);
		}
		if (f1 & TR1_INT)
		{
			p += INT_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for INT bonus %d, total is %d\n", o_ptr->pval, p);
		}
		if (f1 & TR1_WIS)
		{
			p += WIS_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for WIS bonus %d, total is %d\n", o_ptr->pval, p);
		}
		if (f1 & TR1_DEX)
		{
			p += DEX_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for DEX bonus %d, total is %d\n", o_ptr->pval, p);
		}
		if (f1 & TR1_CON)
		{
			p += CON_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for CON bonus %d, total is %d\n", o_ptr->pval, p);
		}
		if (f1 & TR1_STEALTH)
		{
			p += STEALTH_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for Stealth bonus %d, total is %d\n", o_ptr->pval, p);
		}
		if (f1 & TR1_SEARCH)
		{
			p += SEARCH_POWER * o_ptr->pval;
			LOG_PRINT2("Adding power for searching bonus %d, total is %d\n", o_ptr->pval , p);
		}
		/* Add extra power term if there are a lot of ability bonuses */
		if (o_ptr->pval > 0)
		{
			extra_stat_bonus += ( (f1 & TR1_STR) ? 1 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_INT) ? 3 * o_ptr->pval / 4: 0);
			extra_stat_bonus += ( (f1 & TR1_WIS) ? 3 * o_ptr->pval / 4: 0);
			extra_stat_bonus += ( (f1 & TR1_DEX) ? 1 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_CON) ? 1 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_CHR) ? 0 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_STEALTH) ? 3 * o_ptr->pval / 4: 0);
			extra_stat_bonus += ( (f1 & TR1_INFRA) ? 0 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_TUNNEL) ? 0 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_SEARCH) ? 0 * o_ptr->pval: 0);
			extra_stat_bonus += ( (f1 & TR1_SPEED) ? 0 * o_ptr->pval: 0);

			if (o_ptr->tval == TV_BOW)
			{
				extra_stat_bonus += ( (f1 & TR1_MIGHT) ? 5 * o_ptr->pval / 2: 0);
				extra_stat_bonus += ( (f1 & TR1_SHOTS) ? 3 * o_ptr->pval: 0);
			}
			else if ( (o_ptr->tval == TV_DIGGING) || (o_ptr->tval == TV_HAFTED) ||
			          (o_ptr->tval == TV_POLEARM) || (o_ptr->tval == TV_SWORD) )
			{
				extra_stat_bonus += ( (f1 & TR1_BLOWS) ? 3 * o_ptr->pval: 0);
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
	else if (o_ptr->pval < 0)	/* hack: don't give large negatives */
	{
		if (f1 & TR1_STR) p += 4 * o_ptr->pval;
		if (f1 & TR1_INT) p += 2 * o_ptr->pval;
		if (f1 & TR1_WIS) p += 2 * o_ptr->pval;
		if (f1 & TR1_DEX) p += 3 * o_ptr->pval;
		if (f1 & TR1_CON) p += 4 * o_ptr->pval;
		if (f1 & TR1_STEALTH) p += o_ptr->pval;
		LOG_PRINT1("Subtracting power for negative ability values, total is %d\n", p);
	}
	if (f1 & TR1_CHR)
	{
		p += CHR_POWER * o_ptr->pval;
		LOG_PRINT2("Adding power for CHR bonus/penalty %d, total is %d\n", o_ptr->pval, p);
	}
	if (f1 & TR1_INFRA)
	{
		p += INFRA_POWER * o_ptr->pval;
		LOG_PRINT2("Adding power for infra bonus/penalty %d, total is %d\n", o_ptr->pval, p);
	}
	if (f1 & TR1_TUNNEL)
	{
		p += TUNN_POWER * o_ptr->pval;
		LOG_PRINT2("Adding power for tunnelling bonus/penalty %d, total is %d\n", o_ptr->pval, p);
	}
	if (f1 & TR1_SPEED)
	{
		p += sign(o_ptr->pval) * speed_power[ABS(o_ptr->pval)];
		LOG_PRINT2("Adding power for speed bonus/penalty %d, total is %d\n", o_ptr->pval, p);
	}

#define ADD_POWER(string, val, flag, flgnum, extra) \
	if (f##flgnum & flag) { \
		p += (val); \
		extra; \
		LOG_PRINT1("Adding power for " string ", total is %d\n", p); \
	}

	ADD_POWER("sustain STR",        10, TR2_SUST_STR, 2,);
	ADD_POWER("sustain INT",         5, TR2_SUST_INT, 2,);
	ADD_POWER("sustain WIS",         5, TR2_SUST_WIS, 2,);
	ADD_POWER("sustain DEX",         8, TR2_SUST_DEX, 2,);
	ADD_POWER("sustain CON",         8, TR2_SUST_CON, 2,);
	ADD_POWER("sustain CHR",         1, TR2_SUST_CHR, 2,);

	ADD_POWER("acid immunity",      38, TR2_IM_ACID,  2, immunities++);
	ADD_POWER("elec immunity",      35, TR2_IM_ELEC,  2, immunities++);
	ADD_POWER("fire immunity",      40, TR2_IM_FIRE,  2, immunities++);
	ADD_POWER("cold immunity",      37, TR2_IM_COLD,  2, immunities++);

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

	ADD_POWER("free action",		14, TR3_FREE_ACT,    3,);
	ADD_POWER("hold life",			12, TR3_HOLD_LIFE,   3,);
	ADD_POWER("feather fall",		 1, TR3_FEATHER,     3,);
	ADD_POWER("permanent light",		 2, TR3_LITE,	     3,);
	ADD_POWER("see invisible",		10, TR3_SEE_INVIS,   3,);
	ADD_POWER("telepathy",			40, TR3_TELEPATHY,   3,);
	ADD_POWER("slow digestion",		 2, TR3_SLOW_DIGEST, 3,);
	ADD_POWER("resist acid",		 5, TR2_RES_ACID,    2,);
	ADD_POWER("resist elec",		 6, TR2_RES_ELEC,    2,);
	ADD_POWER("resist fire",		 6, TR2_RES_FIRE,    2,);
	ADD_POWER("resist cold",		 6, TR2_RES_COLD,    2,);
	ADD_POWER("resist poison",		28, TR2_RES_POIS,    2,);
	ADD_POWER("resist fear",		 6, TR2_RES_FEAR,    2,);
	ADD_POWER("resist light",		 6, TR2_RES_LITE,    2,);
	ADD_POWER("resist dark",		16, TR2_RES_DARK,    2,);
	ADD_POWER("resist blindness",		16, TR2_RES_BLIND,   2,);
	ADD_POWER("resist confusion",		24, TR2_RES_CONFU,   2,);
	ADD_POWER("resist sound",		14, TR2_RES_SOUND,   2,);
	ADD_POWER("resist shards",		 8, TR2_RES_SHARD,   2,);
	ADD_POWER("resist nexus",		10, TR2_RES_NEXUS,   2,);
	ADD_POWER("resist nether",		20, TR2_RES_NETHR,   2,);
	ADD_POWER("resist chaos",		20, TR2_RES_CHAOS,   2,);
	ADD_POWER("resist disenchantment",	20, TR2_RES_DISEN,   2,);
	ADD_POWER("regeneration",		 9, TR3_REGEN,	     3,);
	ADD_POWER("blessed",			 1, TR3_BLESSED,     3,);
	ADD_POWER("no fuel",			 5, TR3_NO_FUEL,     3,);

	if (f3 & TR3_TELEPORT)
	{
		p -= 80;
		LOG_PRINT1("Subtracting power for teleportation, total is %d\n", p);
	}
	if (f3 & TR3_DRAIN_EXP)
	{
		p -= 30;
		LOG_PRINT1("Subtracting power for drain experience, total is %d\n", p);
	}
	if (f3 & TR3_AGGRAVATE)
	{
		p -= 60;
		LOG_PRINT1("Subtracting power for aggravation, total is %d\n", p);
	}
	if (f3 & TR3_LIGHT_CURSE)
	{
		p -= 6;
		LOG_PRINT1("Subtracting power for light curse, total is %d\n", p);
	}
	if (f3 & TR3_HEAVY_CURSE)
	{
		p -= 20;
		LOG_PRINT1("Subtracting power for heavy curse, total is %d\n", p);
	}

	/*	if (f3 & TR3_PERMA_CURSE) p -= 40; */

	LOG_PRINT1("FINAL POWER IS %d\n", p);

	return (p);
}
