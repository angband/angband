/*
 * File: obj-make.c
 * Purpose: Object generation functions.
 *
 * Copyright (c) 1987-2007 Angband contributors
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
#include "tvalsval.h"


static bool kind_is_good(const object_kind *);

static u32b obj_total[MAX_DEPTH];
static byte *obj_alloc;

static u32b obj_total_great[MAX_DEPTH];
static byte *obj_alloc_great;

/* Don't worry about probabilities for anything past dlev100 */
#define MAX_O_DEPTH		100

/*
 * Free object allocation info.
 */
void free_obj_alloc(void)
{
	FREE(obj_alloc);
	FREE(obj_alloc_great);
}


/*
 * Using k_info[], init rarity data for the entire dungeon.
 */
bool init_obj_alloc(void)
{
	int k_max = z_info->k_max;
	int item, lev;


	/* Free obj_allocs if allocated */
	FREE(obj_alloc);

	/* Allocate and wipe */
	obj_alloc = C_ZNEW((MAX_O_DEPTH + 1) * k_max, byte);
	obj_alloc_great = C_ZNEW((MAX_O_DEPTH + 1) * k_max, byte);

	/* Wipe the totals */
	C_WIPE(obj_total, MAX_O_DEPTH + 1, u32b);
	C_WIPE(obj_total_great, MAX_O_DEPTH + 1, u32b);


	/* Init allocation data */
	for (item = 1; item < k_max; item++)
	{
		const object_kind *k_ptr = &k_info[item];

		int min = k_ptr->alloc_min;
		int max = k_ptr->alloc_max;

		/* If an item doesn't have a rarity, move on */
		if (!k_ptr->alloc_prob) continue;

		/* Go through all the dungeon levels */
		for (lev = 0; lev <= MAX_O_DEPTH; lev++)
		{
			int rarity = k_ptr->alloc_prob;

			/* Save the probability in the standard table */
			if ((lev < min) || (lev > max)) rarity = 0;
			obj_total[lev] += rarity;
			obj_alloc[(lev * k_max) + item] = rarity;

			/* Save the probability in the "great" table if relevant */
			if (!kind_is_good(k_ptr)) rarity = 0;
			obj_total_great[lev] += rarity;
			obj_alloc_great[(lev * k_max) + item] = rarity;
		}
	}

	return TRUE;
}




/*
 * Choose an object kind given a dungeon level to choose it for.
 */
s16b get_obj_num(int level, bool good)
{
	/* This is the base index into obj_alloc for this dlev */
	size_t ind, item;
	u32b value;

	/* Occasional level boost */
	if ((level > 0) && one_in_(GREAT_OBJ))
	{
		/* What a bizarre calculation */
		level = 1 + (level * MAX_O_DEPTH / randint1(MAX_O_DEPTH));
	}

	/* Paranoia */
	level = MIN(level, MAX_O_DEPTH);
	level = MAX(level, 0);

	/* Pick an object */
	ind = level * z_info->k_max;

	if (!good)
	{
		value = randint0(obj_total[level]);
		for (item = 1; item < z_info->k_max; item++)
		{
			/* Found it */
			if (value < obj_alloc[ind + item]) break;

			/* Decrement */
			value -= obj_alloc[ind + item];
		}
	}
	else
	{
		value = randint0(obj_total_great[level]);
		for (item = 1; item < z_info->k_max; item++)
		{
			/* Found it */
			if (value < obj_alloc_great[ind + item]) break;

			/* Decrement */
			value -= obj_alloc_great[ind + item];
		}
	}


	/* Return the item index */
	return item;
}




/*
 * Help determine an "enchantment bonus" for an object.
 *
 * To avoid floating point but still provide a smooth distribution of bonuses,
 * we simply round the results of division in such a way as to "average" the
 * correct floating point value.
 *
 * This function has been changed.  It uses "Rand_normal()" to choose values
 * from a normal distribution, whose mean moves from zero towards the max as
 * the level increases, and whose standard deviation is equal to 1/4 of the
 * max, and whose values are forced to lie between zero and the max, inclusive.
 *
 * Since the "level" rarely passes 100 before Morgoth is dead, it is very
 * rare to get the "full" enchantment on an object, even a deep levels.
 *
 * It is always possible (albeit unlikely) to get the "full" enchantment.
 *
 * A sample distribution of values from "m_bonus(10, N)" is shown below:
 *
 *   N       0     1     2     3     4     5     6     7     8     9    10
 * ---    ----  ----  ----  ----  ----  ----  ----  ----  ----  ----  ----
 *   0   66.37 13.01  9.73  5.47  2.89  1.31  0.72  0.26  0.12  0.09  0.03
 *   8   46.85 24.66 12.13  8.13  4.20  2.30  1.05  0.36  0.19  0.08  0.05
 *  16   30.12 27.62 18.52 10.52  6.34  3.52  1.95  0.90  0.31  0.15  0.05
 *  24   22.44 15.62 30.14 12.92  8.55  5.30  2.39  1.63  0.62  0.28  0.11
 *  32   16.23 11.43 23.01 22.31 11.19  7.18  4.46  2.13  1.20  0.45  0.41
 *  40   10.76  8.91 12.80 29.51 16.00  9.69  5.90  3.43  1.47  0.88  0.65
 *  48    7.28  6.81 10.51 18.27 27.57 11.76  7.85  4.99  2.80  1.22  0.94
 *  56    4.41  4.73  8.52 11.96 24.94 19.78 11.06  7.18  3.68  1.96  1.78
 *  64    2.81  3.07  5.65  9.17 13.01 31.57 13.70  9.30  6.04  3.04  2.64
 *  72    1.87  1.99  3.68  7.15 10.56 20.24 25.78 12.17  7.52  4.42  4.62
 *  80    1.02  1.23  2.78  4.75  8.37 12.04 27.61 18.07 10.28  6.52  7.33
 *  88    0.70  0.57  1.56  3.12  6.34 10.06 15.76 30.46 12.58  8.47 10.38
 *  96    0.27  0.60  1.25  2.28  4.30  7.60 10.77 22.52 22.51 11.37 16.53
 * 104    0.22  0.42  0.77  1.36  2.62  5.33  8.93 13.05 29.54 15.23 22.53
 * 112    0.15  0.20  0.56  0.87  2.00  3.83  6.86 10.06 17.89 27.31 30.27
 * 120    0.03  0.11  0.31  0.46  1.31  2.48  4.60  7.78 11.67 25.53 45.72
 * 128    0.02  0.01  0.13  0.33  0.83  1.41  3.24  6.17  9.57 14.22 64.07
 */
static s16b m_bonus(int max, int level)
{
	int bonus, stand, extra, value;


	/* Paranoia -- enforce maximal "level" */
	if (level > MAX_DEPTH - 1) level = MAX_DEPTH - 1;


	/* The "bonus" moves towards the max */
	bonus = ((max * level) / MAX_DEPTH);

	/* Hack -- determine fraction of error */
	extra = ((max * level) % MAX_DEPTH);

	/* Hack -- simulate floating point computations */
	if (randint0(MAX_DEPTH) < extra) bonus++;


	/* The "stand" is equal to one quarter of the max */
	stand = (max / 4);

	/* Hack -- determine fraction of error */
	extra = (max % 4);

	/* Hack -- simulate floating point computations */
	if (randint0(4) < extra) stand++;


	/* Choose an "interesting" value */
	value = Rand_normal(bonus, stand);

	/* Enforce the minimum value */
	if (value < 0) return (0);

	/* Enforce the maximum value */
	if (value > max) return (max);

	/* Result */
	return (value);
}




/*
 * Cheat -- describe a created object for the user
 */
static void object_mention(const object_type *o_ptr)
{
	char o_name[80];

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE | ODESC_SPOIL);

	/* Provide a silly message */
	if (artifact_p(o_ptr))
		msg_format("Artifact (%s)", o_name);
	else if (ego_item_p(o_ptr))
		msg_format("Ego-item (%s)", o_name);
	else
		msg_format("Object (%s)", o_name);
}


/*
 * Attempt to change an object into an ego-item -MWK-
 * Better only called by apply_magic().
 * The return value says if we picked a cursed item (if allowed) and is
 * passed on to a_m_aux1/2().
 * If no legal ego item is found, this routine returns 0, resulting in
 * an unenchanted item.
 */
static int make_ego_item(object_type *o_ptr, int level, bool force_uncursed)
{
	int i, j;

	int e_idx;

	long value, total;

	ego_item_type *e_ptr;

	alloc_entry *table = alloc_ego_table;


	/* Fail if object already is ego or artifact */
	if (o_ptr->name1) return (FALSE);
	if (o_ptr->name2) return (FALSE);

	/* Boost level (like with object base types) */
	if (level > 0)
	{
		/* Occasional "boost" */
		if (one_in_(GREAT_EGO))
		{
			/* The bizarre calculation again */
			level = 1 + (level * MAX_DEPTH / randint1(MAX_DEPTH));
		}
	}

	/* Reset total */
	total = 0L;

	/* Process probabilities */
	for (i = 0; i < alloc_ego_size; i++)
	{
		/* Default */
		table[i].prob3 = 0;

		/* Objects are sorted by depth */
		if (table[i].level > level) continue;

		/* Get the index */
		e_idx = table[i].index;

		/* Get the actual kind */
		e_ptr = &e_info[e_idx];

		/* Avoid cursed items if specified */
		if (force_uncursed && cursed_p(e_ptr)) continue;

		/* Test if this is a legal ego-item type for this object */
		for (j = 0; j < EGO_TVALS_MAX; j++)
		{
			/* Require identical base type */
			if (o_ptr->tval == e_ptr->tval[j])
			{
				/* Require sval in bounds, lower */
				if (o_ptr->sval >= e_ptr->min_sval[j])
				{
					/* Require sval in bounds, upper */
					if (o_ptr->sval <= e_ptr->max_sval[j])
					{
						/* Accept */
						table[i].prob3 = table[i].prob2;
					}
				}
			}
		}

		/* Total */
		total += table[i].prob3;
	}

	/* No legal ego-items -- create a normal unenchanted one */
	if (total == 0) return (0);


	/* Pick an ego-item */
	value = randint0(total);

	/* Find the object */
	for (i = 0; i < alloc_ego_size; i++)
	{
		/* Found the entry */
		if (value < table[i].prob3) break;

		/* Decrement */
		value = value - table[i].prob3;
	}

	/* We have one */
	e_idx = (byte)table[i].index;
	o_ptr->name2 = e_idx;

	return (e_info[e_idx].flags[2] & TR2_CURSE_MASK ? -2 : 2);
}


/**
 * Copy artifact data to a normal object, and set various slightly hacky
 * globals.
 */
static void copy_artifact_data(object_type *o_ptr, const artifact_type *a_ptr)
{
	/* Extract the other fields */
	o_ptr->pval = a_ptr->pval;
	o_ptr->ac = a_ptr->ac;
	o_ptr->dd = a_ptr->dd;
	o_ptr->ds = a_ptr->ds;
	o_ptr->to_a = a_ptr->to_a;
	o_ptr->to_h = a_ptr->to_h;
	o_ptr->to_d = a_ptr->to_d;
	o_ptr->weight = a_ptr->weight;

	/* Hack -- extract the "cursed" flags */
	if (cursed_p(a_ptr))
		o_ptr->flags[2] |= (a_ptr->flags[2] & TR2_CURSE_MASK);

	/* Mega-Hack -- increase the rating */
	rating += 10;

	/* Mega-Hack -- increase the rating again */
	if (a_ptr->cost > 50000L) rating += 10;

	/* Set the good item flag */
	good_item_flag = TRUE;

	/* Cheat -- peek at the item */
	if (OPT(cheat_peek)) object_mention(o_ptr);
}


/*
 * Mega-Hack -- Attempt to create one of the "Special Objects".
 *
 * We are only called from "make_object()", and we assume that
 * "apply_magic()" is called immediately after we return.
 *
 * Note -- see "make_artifact()" and "apply_magic()".
 *
 * We *prefer* to create the special artifacts in order, but this is
 * normally outweighed by the "rarity" rolls for those artifacts.  The
 * only major effect of this logic is that the Phial (with rarity one)
 * is always the first special artifact created.
 */
static bool make_artifact_special(object_type *o_ptr, int level)
{
	int i;

	int k_idx;


	/* No artifacts, do nothing */
	if (OPT(adult_no_artifacts)) return (FALSE);

	/* No artifacts in the town */
	if (!p_ptr->depth) return (FALSE);

	/* Check the special artifacts */
	for (i = 0; i < ART_MIN_NORMAL; ++i)
	{
		artifact_type *a_ptr = &a_info[i];

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->created) continue;

		/* Enforce minimum "depth" (loosely) */
		if (a_ptr->level > p_ptr->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (a_ptr->level - p_ptr->depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Artifact "rarity roll" */
		if (randint0(a_ptr->rarity) != 0) continue;

		/* Find the base object */
		k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Enforce minimum "object" level (loosely) */
		if (k_info[k_idx].level > level)
		{
			/* Get the "out-of-depth factor" */
			int d = (k_info[k_idx].level - level) * 5;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Assign the template */
		object_prep(o_ptr, k_idx);

		/* Mark the item as an artifact */
		o_ptr->name1 = i;

		/* Copy across all the data from the artifact struct */
		copy_artifact_data(o_ptr, a_ptr);

		/* Mark the artifact as "created" */
		a_ptr->created = 1;

		/* Success */
		return TRUE;
	}

	/* Failure */
	return FALSE;
}


/*
 * Attempt to change an object into an artifact
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(object_type *o_ptr)
{
	int i;


	/* No artifacts, do nothing */
	if (OPT(adult_no_artifacts)) return (FALSE);

	/* No artifacts in the town */
	if (!p_ptr->depth) return (FALSE);

	/* Paranoia -- no "plural" artifacts */
	if (o_ptr->number != 1) return (FALSE);

	/* Check the artifact list (skip the "specials") */
	for (i = ART_MIN_NORMAL; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];

		/* Skip "empty" items */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->created) continue;

		/* Must have the correct fields */
		if (a_ptr->tval != o_ptr->tval) continue;
		if (a_ptr->sval != o_ptr->sval) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->level > p_ptr->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (a_ptr->level - p_ptr->depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* We must make the "rarity roll" */
		if (randint0(a_ptr->rarity) != 0) continue;

		/* Mark the item as an artifact */
		o_ptr->name1 = i;

		/* Copy across all the data from the artifact struct */
		copy_artifact_data(o_ptr, a_ptr);

		/* Hack -- Mark the artifact as "created" */
		a_ptr->created = 1;

		return TRUE;
	}

	return FALSE;
}




/*
 * Apply magic to an item known to be a "weapon"
 *
 * Hack -- note special base damage dice boosting
 * Hack -- note special processing for weapon/digger
 * Hack -- note special rating boost for dragon scale mail
 */
static void a_m_aux_1(object_type *o_ptr, int level, int power)
{
	int tohit1 = randint1(5) + m_bonus(5, level);
	int tohit2 = m_bonus(10, level);

	int todam1 = randint1(5) + m_bonus(5, level);
	int todam2 = m_bonus(10, level);

	switch (power)
	{
		case -2:
			o_ptr->to_h -= tohit2;
			o_ptr->to_d -= todam2;

		case -1:
			o_ptr->to_h -= tohit1;
			o_ptr->to_d -= todam1;
			break;

		case 2:
			o_ptr->to_h += tohit2;
			o_ptr->to_d += todam2;

		case 1:
			o_ptr->to_h += tohit1;
			o_ptr->to_d += todam1;
			break;
	}


	/* Analyze type */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		{
			/* Very bad */
			if (power < -1)
			{
				/* Hack -- Horrible digging bonus */
				o_ptr->pval = 0 - (5 + randint1(5));
			}

			/* Bad */
			else if (power < 0)
			{
				/* Hack -- Reverse digging bonus */
				o_ptr->pval = -o_ptr->pval;
			}

			break;
		}


		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			/* Very Good */
			if (power > 1)
			{
				/* Hack -- Super-charge the damage dice */
				while ((o_ptr->dd * o_ptr->ds > 0) &&
				       one_in_(10L * o_ptr->dd * o_ptr->ds))
				{
					o_ptr->dd++;
				}

				/* Hack -- Lower the damage dice */
				if (o_ptr->dd > 9) o_ptr->dd = 9;
			}

			break;
		}


		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			/* Very good */
			if (power > 1)
			{
				/* Hack -- super-charge the damage dice */
				while ((o_ptr->dd * o_ptr->ds > 0) &&
				       one_in_(10L * o_ptr->dd * o_ptr->ds))
				{
					o_ptr->dd++;
				}

				/* Hack -- restrict the damage dice */
				if (o_ptr->dd > 9) o_ptr->dd = 9;
			}

			break;
		}
	}
}


/*
 * Apply magic to armour
 */
static void a_m_aux_2(object_type *o_ptr, int level, int power)
{
	int toac1 = randint1(5) + m_bonus(5, level);
	int toac2 = m_bonus(10, level);


	if (power == -2)
		o_ptr->to_a -= toac1 + toac2;
	else if (power == -1)
		o_ptr->to_a -= toac1;
	else if (power == 1)
		o_ptr->to_a += toac1;
	else if (power == 2)
		o_ptr->to_a += toac1 + toac2;


	/* Analyze type */
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		{
			/* Rating boost */
			rating += 30;

			/* Mention the item */
			if (OPT(cheat_peek)) object_mention(o_ptr);

			break;
		}
	}
}



/*
 * Apply magic to an item known to be a "ring" or "amulet"
 *
 * Hack -- note special rating boost for ring of speed
 * Hack -- note special rating boost for certain amulets
 * Hack -- note special "pval boost" code for ring of speed
 * Hack -- note that some items must be cursed (or blessed)
 */
static void a_m_aux_3(object_type *o_ptr, int level, int power)
{
	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
		case TV_RING:
		{
			/* Analyze */
			switch (o_ptr->sval)
			{
				/* Strength, Constitution, Dexterity, Intelligence */
				case SV_RING_STRENGTH:
				case SV_RING_CONSTITUTION:
				case SV_RING_DEXTERITY:
				case SV_RING_INTELLIGENCE:
				{
					/* Stat bonus */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
						o_ptr->pval = -o_ptr->pval;

					break;
				}

				/* Ring of Speed! */
				case SV_RING_SPEED:
				{
					/* Base speed (1 to 10) */
					o_ptr->pval = randint1(5) + m_bonus(5, level);

					/* Super-charge the ring */
					while (randint0(100) < 50) o_ptr->pval++;

					/* Cursed Ring */
					if (power < 0)
					{
						/* Reverse pval */
						o_ptr->pval = -o_ptr->pval;
					}
					else
					{
						/* Rating boost */
						rating += 25;

						/* Mention the item */
						if (OPT(cheat_peek)) object_mention(o_ptr);
					}

					break;
				}

				/* Searching */
				case SV_RING_SEARCHING:
				{
					/* Bonus to searching */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
						o_ptr->pval = -o_ptr->pval;

					break;
				}

				/* Light, Dark */
				case SV_RING_LIGHT:
				case SV_RING_DARK:
				{
					/* Searching bonus */
					o_ptr->pval = 1 + m_bonus(5, level);

					break;
				}

				/* Flames, Acid, Ice, Lightning */
				case SV_RING_FLAMES:
				case SV_RING_ACID:
				case SV_RING_ICE:
				case SV_RING_LIGHTNING:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);

					break;
				}

				/* Ring of damage */
				case SV_RING_DAMAGE:
				{
					/* Bonus to damage */
					o_ptr->to_d = 5 + randint1(3) + m_bonus(7, level);

					/* Cursed */
					if (power < 0)
						o_ptr->to_d = -o_ptr->to_d;

					break;
				}

				/* Ring of Accuracy */
				case SV_RING_ACCURACY:
				{
					/* Bonus to hit */
					o_ptr->to_h = 5 + randint1(3) + m_bonus(7, level);

					/* Cursed */
					if (power < 0)
						o_ptr->to_h = -o_ptr->to_h;

					break;
				}

				/* Ring of Protection */
				case SV_RING_PROTECTION:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
						o_ptr->to_a = -o_ptr->to_a;

					break;
				}

				/* Ring of Slaying */
				case SV_RING_SLAYING:
				{
					/* Bonus to damage and to hit */
					o_ptr->to_d = randint1(5) + m_bonus(5, level);
					o_ptr->to_h = randint1(5) + m_bonus(5, level);

					/* Cursed -- reverse bonuses */
					if (power < 0)
					{
						o_ptr->to_h = -o_ptr->to_h;
						o_ptr->to_d = -o_ptr->to_d;
					}

					break;
				}

				case SV_RING_RECKLESS_ATTACKS:
				{
					int amt = rand_range(3, 5);

					o_ptr->to_d = o_ptr->to_h = amt;
					o_ptr->to_a = -4 * amt;

					break;
				}

				case SV_RING_OF_THE_MOUSE:
				{
					int amt = randint1(4);

					/* Dex bonus, dam penalty */
					o_ptr->pval = amt;
					o_ptr->to_d = -3 * amt;

					break;
				}

				case SV_RING_DELVING:
				{
					/* Digging bonus */
					o_ptr->pval = rand_range(3, 5);

					break;
				}
			}

			break;
		}

		case TV_AMULET:
		{
			/* Analyze */
			switch (o_ptr->sval)
			{
				/* Amulet of wisdom/charisma/infravision */
				case SV_AMULET_WISDOM:
				case SV_AMULET_CHARISMA:
				case SV_AMULET_INFRAVISION:
				{
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
						o_ptr->pval = -o_ptr->pval;

					break;
				}

				/* Amulet of searching */
				case SV_AMULET_SEARCHING:
				{
					o_ptr->pval = randint1(5) + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
						o_ptr->pval = -o_ptr->pval;

					break;
				}

				/* Amulet of ESP -- never cursed */
				case SV_AMULET_ESP:
				{
					o_ptr->pval = randint1(5) + m_bonus(5, level);

					break;
				}

				/* Amulet of the Magi -- never cursed */
				case SV_AMULET_THE_MAGI:
				{
					o_ptr->pval = 1 + m_bonus(3, level);
					o_ptr->to_a = randint1(5) + m_bonus(5, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (OPT(cheat_peek)) object_mention(o_ptr);

					break;
				}

				/* Amulet of Devotion -- never cursed */
				case SV_AMULET_DEVOTION:
				{
					o_ptr->pval = 1 + m_bonus(3, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (OPT(cheat_peek)) object_mention(o_ptr);

					break;
				}

				/* Amulet of Weaponmastery -- never cursed */
				case SV_AMULET_WEAPONMASTERY:
				{
					o_ptr->to_h = 1 + m_bonus(4, level);
					o_ptr->to_d = 1 + m_bonus(4, level);
					o_ptr->pval = 1 + m_bonus(2, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (OPT(cheat_peek)) object_mention(o_ptr);

					break;
				}

				/* Amulet of Trickery -- never cursed */
				case SV_AMULET_TRICKERY:
				{
					o_ptr->pval = randint1(1) + m_bonus(3, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (OPT(cheat_peek)) object_mention(o_ptr);

					break;
				}
			}

			break;
		}
	}

	/* Just for now */
	if (o_ptr->pval < 0)
		o_ptr->flags[2] |= TR2_LIGHT_CURSE;
}


/*
 * Apply magic to an item known to be "boring"
 *
 * Hack -- note the special code for various items
 */
static void a_m_aux_4(object_type *o_ptr, int level, int power)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Unused parameters */
	(void)level;
	(void)power;

	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
		case TV_LITE:
		{
			/* Torches and lanterns get half-fuel */
			if (o_ptr->sval == SV_LITE_TORCH)
				o_ptr->timeout = FUEL_TORCH;
			else if (o_ptr->sval == SV_LITE_LANTERN)
				o_ptr->timeout = FUEL_LAMP / 2;

			break;
		}

		case TV_WAND:
		case TV_STAFF:
		{
			/* Charge staves and wands */
			o_ptr->pval = k_ptr->charge_base;

			if (k_ptr->charge_dd && k_ptr->charge_ds)
				o_ptr->pval += damroll(k_ptr->charge_dd, k_ptr->charge_ds);

			break;
		}

		case TV_ROD:
		{
			object_kind *k_ptr = &k_info[o_ptr->k_idx];

			/* Transfer the pval. */
			o_ptr->pval = k_ptr->pval;

			break;
		}

		case TV_CHEST:
		{
			/* Hack -- skip ruined chests */
			if (k_info[o_ptr->k_idx].level <= 0) break;

			/* Hack -- pick a "difficulty" */
			o_ptr->pval = randint1(k_info[o_ptr->k_idx].level);

			/* Never exceed "difficulty" of 55 to 59 */
			if (o_ptr->pval > 55) o_ptr->pval = (s16b)(55 + randint0(5));

			break;
		}
	}
}

static const u32b ego_sustains[] =
{
	TR1_SUST_STR,
	TR1_SUST_INT,
	TR1_SUST_WIS,
	TR1_SUST_DEX,
	TR1_SUST_CON,
	TR1_SUST_CHR,
};

/*
 * Which TR? flags have the sustains
 */
unsigned ego_xtra_sustain_idx(void)
{
	return 1;
}

u32b ego_xtra_sustain_list(void)
{
	u32b ret = 0;
	size_t i;

	for (i = 0; i < N_ELEMENTS(ego_sustains); i++)
		ret |= ego_sustains[i];

	return ret;
}

static const u32b ego_resists[] =
{
	TR1_RES_POIS,
	TR1_RES_FEAR,
	TR1_RES_LITE,
	TR1_RES_DARK,
	TR1_RES_BLIND,
	TR1_RES_CONFU,
	TR1_RES_SOUND,
	TR1_RES_SHARD,
	TR1_RES_NEXUS,
	TR1_RES_NETHR,
	TR1_RES_CHAOS,
	TR1_RES_DISEN,
};

/*
 * Which TR? flags have the random resists
 */
unsigned ego_xtra_resist_idx(void)
{
	return 1;
}

u32b ego_xtra_resist_list(void)
{
	u32b ret = 0;
	size_t i;

	for (i = 0; i < N_ELEMENTS(ego_resists); i++)
		ret |= ego_resists[i];

	return ret;
}

static const u32b ego_powers[] =
{
	TR2_SLOW_DIGEST,
	TR2_FEATHER,
	TR2_LITE,
	TR2_REGEN,
	TR2_TELEPATHY,
	TR2_SEE_INVIS,
	TR2_FREE_ACT,
	TR2_HOLD_LIFE,
};

/*
 * Which TR? flags have the random powers
 */
unsigned ego_xtra_power_idx(void)
{
	return 2;
}

u32b ego_xtra_power_list(void)
{
	u32b ret = 0;
	size_t i;

	for (i = 0; i < N_ELEMENTS(ego_powers); i++)
		ret |= ego_powers[i];

	return ret;
}

/**
 * This is a safe way to choose a random new flag to add to an object.
 * It takes the existing flags, an array of new attrs, and the size of
 * the array, and returns an entry from attrs, or 0 if there are no
 * new attrs.
 */
u32b get_new_attr(u32b flags, const u32b attrs[], size_t size)
{
	size_t i;
	int options = 0;
	u32b flag = 0;
	for (i = 0; i < size; i++)
	{
		/* skip this one if the flag is already present */
		if (flags & attrs[i]) continue;

		/* each time we find a new possible option, we have a 1-in-N chance of
		 * choosing it and an (N-1)-in-N chance of keeping a previous one */
		if (one_in_(++options)) flag = attrs[i];
	}
	return flag;
}


/**
 * Complete object creation by applying magic to it.
 *
 * Magic includes rolling for random bonuses, applying flags to ego-items,
 * charging charged items, fuelling lights, and trapping chests.
 *
 * The `good` argument forces the item to be at least `good`, and the `great`
 * argument does likewise.  Setting `allow_artifacts` to TRUE allows artifacts
 * to be created here.
 *
 * If `good` or `great` are not set, then the `lev` argument controls the
 * quality of item.  See the function itself for the specifics of the
 * calculations involved.
 */
void apply_magic(object_type *o_ptr, int lev, bool allow_artifacts, bool good, bool great)
{
	int power = 0;
	/*u32b xtra = 0;*/
	/*bool new = FALSE;*/

	/* Chance of being `good` and `great` */
	int good_chance = (lev+2) * 3;
	int great_chance = MIN(lev/4 + lev, 50);


	/* Limit depth */
	if (lev > MAX_DEPTH - 1) lev = MAX_DEPTH - 1;

	/* Roll for "good" */
	if (good || (randint0(100) < good_chance))
	{
		/* Assume "good" */
		power = 1;

		/* Roll for "great" */
		if (great || (randint0(100) < great_chance)) power = 2;
	}

	/* Roll for "cursed" */
	else if (randint0(100) < good_chance)
	{
		/* Assume "cursed" */
		power = -1;

		/* Roll for "broken" */
		if (randint0(100) < great_chance) power = -2;
	}


	/* Roll for artifact creation */
	if (allow_artifacts && !o_ptr->name1)
	{
		int i;
		int rolls = 0;

		/* Get one roll if excellent */
		if (power >= 2) rolls = 1;

		/* Get four rolls if forced great */
		if (great) rolls = 4;

		/* Roll for artifacts if allowed */
		for (i = 0; i < rolls; i++)
		{
			if (make_artifact(o_ptr))
				return;
		}
	}


	/* Apply magic */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOW:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			if (power == 2 || power == -2)
			{
				int ego_power;

				ego_power = make_ego_item(o_ptr, lev, (bool)(power > 0));

				if (ego_power) power = ego_power;
			}

			if (power) a_m_aux_1(o_ptr, lev, power);

			break;
		}

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_HELM:
		case TV_CROWN:
		case TV_CLOAK:
		case TV_GLOVES:
		case TV_BOOTS:
		{
			if (power == 2 || power == -2)
			{
				int ego_power;

				ego_power = make_ego_item(o_ptr, lev, (bool)(power > 0));

				if (ego_power) power = ego_power;
			}

			if (power) a_m_aux_2(o_ptr, lev, power);

			break;
		}

		case TV_RING:
		case TV_AMULET:
		{
			if (!power && (randint0(100) < 50)) power = -1;
			a_m_aux_3(o_ptr, lev, power);
			break;
		}

		case TV_LITE:
		{
			if (power == 2 || power == -2)
				make_ego_item(o_ptr, lev, (bool)(power > 0));

			/* Fuel it */
			a_m_aux_4(o_ptr, lev, power);
			break;
		}

		default:
		{
			a_m_aux_4(o_ptr, lev, power);
			break;
		}
	}


	/* Hack -- analyze ego-items */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];
		u32b flags[OBJ_FLAG_N];
		object_flags(o_ptr, flags);

		/* Extra powers */
		if (e_ptr->xtra == OBJECT_XTRA_TYPE_SUSTAIN)
			o_ptr->flags[1] |= get_new_attr(flags[1], ego_sustains,
											N_ELEMENTS(ego_sustains));
		else if (e_ptr->xtra == OBJECT_XTRA_TYPE_RESIST)
			o_ptr->flags[1] |= get_new_attr(flags[1], ego_resists,
											N_ELEMENTS(ego_resists));
		else if (e_ptr->xtra == OBJECT_XTRA_TYPE_POWER)
			o_ptr->flags[2] |= get_new_attr(flags[2], ego_powers,
											N_ELEMENTS(ego_powers));

		/* Hack -- acquire "cursed" flags */
		if (cursed_p(e_ptr))
			o_ptr->flags[2] |= (e_ptr->flags[2] & TR2_CURSE_MASK);

		/* Hack -- apply extra penalties if needed */
		if (cursed_p(o_ptr))
		{
			/* Hack -- obtain bonuses */
			if (e_ptr->max_to_h > 0) o_ptr->to_h -= randint1(e_ptr->max_to_h);
			if (e_ptr->max_to_d > 0) o_ptr->to_d -= randint1(e_ptr->max_to_d);
			if (e_ptr->max_to_a > 0) o_ptr->to_a -= randint1(e_ptr->max_to_a);

			/* Hack -- obtain pval */
			if (e_ptr->max_pval > 0) o_ptr->pval -= randint1(e_ptr->max_pval);

			/* Apply minimums */
			if (o_ptr->to_h > -1 * e_ptr->min_to_h) o_ptr->to_h = -1 * e_ptr->min_to_h;
			if (o_ptr->to_d > -1 * e_ptr->min_to_d) o_ptr->to_d = -1 * e_ptr->min_to_d;
			if (o_ptr->to_a > -1 * e_ptr->min_to_a) o_ptr->to_a = -1 * e_ptr->min_to_a;
			if (o_ptr->pval > -1 * e_ptr->min_pval) o_ptr->pval = -1 * e_ptr->min_pval;
		}

		/* Hack -- apply extra bonuses if needed */
		else
		{
			/* Hack -- obtain bonuses */
			if (e_ptr->max_to_h > 0) o_ptr->to_h += randint1(e_ptr->max_to_h);
			if (e_ptr->max_to_d > 0) o_ptr->to_d += randint1(e_ptr->max_to_d);
			if (e_ptr->max_to_a > 0) o_ptr->to_a += randint1(e_ptr->max_to_a);

			/* Hack -- obtain pval */
			if (e_ptr->max_pval > 0) o_ptr->pval += randint1(e_ptr->max_pval);

			/* Apply minimums */
			if (o_ptr->to_h < e_ptr->min_to_h) o_ptr->to_h = e_ptr->min_to_h;
			if (o_ptr->to_d < e_ptr->min_to_d) o_ptr->to_d = e_ptr->min_to_d;
			if (o_ptr->to_a < e_ptr->min_to_a) o_ptr->to_a = e_ptr->min_to_a;
			if (o_ptr->pval < e_ptr->min_pval) o_ptr->pval = e_ptr->min_pval;
		}

		/* Hack -- apply rating bonus */
		rating += e_ptr->rating;

		/* Cheat -- describe the item */
		if (OPT(cheat_peek)) object_mention(o_ptr);

		/* Done */
		return;
	}


	/* Examine real objects */
	if (o_ptr->k_idx)
	{
		object_kind *k_ptr = &k_info[o_ptr->k_idx];

		/* Hack -- acquire "cursed" flag */
		if (cursed_p(k_ptr))
			o_ptr->flags[2] |= (k_ptr->flags[2] & TR2_CURSE_MASK);
	}
}



/*
 * Hack -- determine if a template is "good".
 *
 * Note that this test only applies to the object *kind*, so it is
 * possible to choose a kind which is "good", and then later cause
 * the actual object to be cursed.  We do explicitly forbid objects
 * which are known to be boring or which start out somewhat damaged.
 */
static bool kind_is_good(const object_kind *k_ptr)
{
	/* Analyze the item type */
	switch (k_ptr->tval)
	{
		/* Armor -- Good unless damaged */
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		{
			if (k_ptr->to_a < 0) return (FALSE);
			return (TRUE);
		}

		/* Weapons -- Good unless damaged */
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		{
			if (k_ptr->to_h < 0) return (FALSE);
			if (k_ptr->to_d < 0) return (FALSE);
			return (TRUE);
		}

		/* Ammo -- Arrows/Bolts are good */
		case TV_BOLT:
		case TV_ARROW:
		{
			return (TRUE);
		}

		/* Books -- High level books are good */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			if (k_ptr->sval >= SV_BOOK_MIN_GOOD) return (TRUE);
			return (FALSE);
		}

		/* Rings -- Rings of Speed are good */
		case TV_RING:
		{
			if (k_ptr->sval == SV_RING_SPEED) return (TRUE);
			return (FALSE);
		}

		/* Amulets -- Amulets of the Magi are good */
		case TV_AMULET:
		{
			if (k_ptr->sval == SV_AMULET_THE_MAGI) return (TRUE);
			if (k_ptr->sval == SV_AMULET_DEVOTION) return (TRUE);
			if (k_ptr->sval == SV_AMULET_WEAPONMASTERY) return (TRUE);
			if (k_ptr->sval == SV_AMULET_TRICKERY) return (TRUE);
			return (FALSE);
		}
	}

	/* Assume not good */
	return (FALSE);
}



/*
 * Attempt to make an object (normal or good/great)
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * We assume that the given object has been "wiped".
 */
bool make_object(object_type *j_ptr, int lev, bool good, bool great)
{
	int k_idx, base;
	object_kind *k_ptr;

	/* Try to make a special artifact */
	if (one_in_(good ? 10 : 1000))
	{
		if (make_artifact_special(j_ptr, lev)) return TRUE;
		/* If we failed to make an artifact, the player gets a great item */
		good = great = TRUE;
	}

	/* Base level for the object */
	base = (good ? (lev + 10) : lev);

	/* Get the object */
	k_idx = get_obj_num(base, good || great);
	if (!k_idx) return FALSE;

	/* Prepare the object */
	object_prep(j_ptr, k_idx);

	/* Apply magic (allow artifacts) */
	apply_magic(j_ptr, lev, TRUE, good, great);


	/* Generate multiple items */
	k_ptr = &k_info[j_ptr->k_idx];

	if (k_ptr->gen_mult_prob >= 100 ||
	    k_ptr->gen_mult_prob >= randint1(100))
	{
		j_ptr->number = damroll(k_ptr->gen_dice, k_ptr->gen_side);
	}


	/* Notice "okay" out-of-depth objects */
	if (!cursed_p(j_ptr) && (k_info[j_ptr->k_idx].level > p_ptr->depth))
	{
		/* Rating increase */
		rating += (k_info[j_ptr->k_idx].level - p_ptr->depth);

		/* Cheat -- peek at items */
		if (OPT(cheat_peek)) object_mention(j_ptr);
	}

	return TRUE;
}



/* The largest possible average gold drop at max depth with biggest spread */
#define MAX_GOLD_DROP     (3*MAX_DEPTH + 30)


/*
 * Make a money object
 */
void make_gold(object_type *j_ptr, int lev, int coin_type)
{
	int sval;
	int k_idx;

	/* This average is 20 at dlev0, 105 at dlev40, 220 at dlev100. */
	/* Follows the formula: y=2x+20 */
	s32b avg = 2*lev + 20;
	s32b spread = lev + 10;
	s32b value = rand_spread(avg, spread);

	/* Pick a treasure variety scaled by level, or force a type */
	if (coin_type != SV_GOLD_ANY)
		sval = coin_type;
	else
		sval = (((value * 100) / MAX_GOLD_DROP) * SV_GOLD_MAX) / 100;

	/* Do not create illegal treasure types */
	if (sval > SV_GOLD_MAX) sval = SV_GOLD_MAX;

	/* Prepare a gold object */
	k_idx = lookup_kind(TV_GOLD, sval);
	object_prep(j_ptr, k_idx);
	j_ptr->pval = value;
}

