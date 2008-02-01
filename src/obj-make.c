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

static u32b obj_total[MAX_DEPTH];
static byte *obj_alloc;

/* Don't worry about probabilities for anything past dlev100 */
#define MAX_O_DEPTH		100

/*
 * Free object allocation info.
 */
void free_obj_alloc(void)
{
	FREE(obj_alloc);
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

	/* Wipe the totals */
	C_WIPE(obj_total, MAX_O_DEPTH + 1, u32b);


	/* Init allocation data */
	for (item = 1; item < k_max; item++)
	{
		object_kind *k_ptr = &k_info[item];

		int min = k_ptr->alloc_min;
		int max = k_ptr->alloc_max;

		/* If an item doesn't have a rarity, move on */
		if (!k_ptr->alloc_prob) continue;

		/* Go through all the dungeon levels */
		for (lev = 0; lev <= MAX_O_DEPTH; lev++)
		{
			int rarity = k_ptr->alloc_prob;

			/* Give out-of-depth items a tiny chance at being made */
			if ((lev < min) || (lev > max)) rarity = 0;

			/* Save the probability */
			obj_total[lev] += rarity;
			obj_alloc[(lev * k_max) + item] = rarity;
		}
	}

	return TRUE;
}




/*
 * Choose an object kind given a dungeon level to choose it for.
 */
s16b get_obj_num(int level)
{
	/* This is the base index into obj_alloc for this dlev */
	size_t ind, item;
	u32b value;

	/* Occasional level boost */
	if ((level > 0) && one_in_(GREAT_OBJ))
	{
		/* What a bizarre calculation */
		level = 1 + (level * MAX_O_DEPTH / randint(MAX_O_DEPTH));
	}

	/* Paranoia */
	level = MIN(level, MAX_O_DEPTH);
	level = MAX(level, 0);

	/* Pick an object */
	ind = level * z_info->k_max;
	value = rand_int(obj_total[level]);
	for (item = 1; item < z_info->k_max; item++)
	{
		/* Found it */
		if (value < obj_alloc[ind + item]) break;

		/* Decrement */
		value -= obj_alloc[ind + item];
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
	if (rand_int(MAX_DEPTH) < extra) bonus++;


	/* The "stand" is equal to one quarter of the max */
	stand = (max / 4);

	/* Hack -- determine fraction of error */
	extra = (max % 4);

	/* Hack -- simulate floating point computations */
	if (rand_int(4) < extra) stand++;


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
	object_desc_spoil(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);

	/* Artifact */
	if (artifact_p(o_ptr))
	{
		/* Silly message */
		msg_format("Artifact (%s)", o_name);
	}

	/* Ego-item */
	else if (ego_item_p(o_ptr))
	{
		/* Silly message */
		msg_format("Ego-item (%s)", o_name);
	}

	/* Normal item */
	else
	{
		/* Silly message */
		msg_format("Object (%s)", o_name);
	}
}


/*
 * Attempt to change an object into an ego-item -MWK-
 * Better only called by apply_magic().
 * The return value says if we picked a cursed item (if allowed) and is
 * passed on to a_m_aux1/2().
 * If no legal ego item is found, this routine returns 0, resulting in
 * an unenchanted item.
 */
static int make_ego_item(object_type *o_ptr, int level, bool only_good)
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
			level = 1 + (level * MAX_DEPTH / randint(MAX_DEPTH));
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

		/* If we force good/great, don't create cursed */
		if (only_good && (e_ptr->flags3 & TR3_LIGHT_CURSE)) continue;

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
	value = rand_int(total);

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

	return ((e_info[e_idx].flags3 & TR3_LIGHT_CURSE) ? -2 : 2);
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
	if (adult_no_artifacts) return (FALSE);

	/* No artifacts in the town */
	if (!p_ptr->depth) return (FALSE);

	/* Check the special artifacts */
	for (i = 0; i < ART_MIN_NORMAL; ++i)
	{
		artifact_type *a_ptr = &a_info[i];

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->cur_num) continue;

		/* Enforce minimum "depth" (loosely) */
		if (a_ptr->level > p_ptr->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (a_ptr->level - p_ptr->depth) * 2;

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0) continue;
		}

		/* Artifact "rarity roll" */
		if (rand_int(a_ptr->rarity) != 0) continue;

		/* Find the base object */
		k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Enforce minimum "object" level (loosely) */
		if (k_info[k_idx].level > level)
		{
			/* Get the "out-of-depth factor" */
			int d = (k_info[k_idx].level - level) * 5;

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0) continue;
		}

		/* Assign the template */
		object_prep(o_ptr, k_idx);

		/* Mark the item as an artifact */
		o_ptr->name1 = i;

		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
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
	if (adult_no_artifacts) return (FALSE);

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
		if (a_ptr->cur_num) continue;

		/* Must have the correct fields */
		if (a_ptr->tval != o_ptr->tval) continue;
		if (a_ptr->sval != o_ptr->sval) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->level > p_ptr->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (a_ptr->level - p_ptr->depth) * 2;

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0) continue;
		}

		/* We must make the "rarity roll" */
		if (rand_int(a_ptr->rarity) != 0) continue;

		/* Mark the item as an artifact */
		o_ptr->name1 = i;

		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
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
	int tohit1 = randint(5) + m_bonus(5, level);
	int todam1 = randint(5) + m_bonus(5, level);

	int tohit2 = m_bonus(10, level);
	int todam2 = m_bonus(10, level);


	/* Good */
	if (power > 0)
	{
		/* Enchant */
		o_ptr->to_h += tohit1;
		o_ptr->to_d += todam1;

		/* Very good */
		if (power > 1)
		{
			/* Enchant again */
			o_ptr->to_h += tohit2;
			o_ptr->to_d += todam2;
		}
	}

	/* Cursed */
	else if (power < 0)
	{
		/* Penalize */
		o_ptr->to_h -= tohit1;
		o_ptr->to_d -= todam1;

		/* Very cursed */
		if (power < -1)
		{
			/* Penalize again */
			o_ptr->to_h -= tohit2;
			o_ptr->to_d -= todam2;
		}

		/* Cursed (if "bad") */
		if (o_ptr->to_h < 0 || o_ptr->to_d < 0)
		    o_ptr->flags3 |= TR3_LIGHT_CURSE;
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
				o_ptr->pval = 0 - (5 + randint(5));
			}

			/* Bad */
			else if (power < 0)
			{
				/* Hack -- Reverse digging bonus */
				o_ptr->pval = 0 - (o_ptr->pval);
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
 * Apply magic to an item known to be "armor"
 *
 * Hack -- note special processing for crown/helm
 * Hack -- note special processing for robe of permanence
 */
static void a_m_aux_2(object_type *o_ptr, int level, int power)
{
	int toac1 = randint(5) + m_bonus(5, level);

	int toac2 = m_bonus(10, level);


	/* Good */
	if (power > 0)
	{
		/* Enchant */
		o_ptr->to_a += toac1;

		/* Very good */
		if (power > 1)
		{
			/* Enchant again */
			o_ptr->to_a += toac2;
		}
	}

	/* Cursed */
	else if (power < 0)
	{
		/* Penalize */
		o_ptr->to_a -= toac1;

		/* Very cursed */
		if (power < -1)
		{
			/* Penalize again */
			o_ptr->to_a -= toac2;
		}

		/* Cursed (if "bad") */
		if (o_ptr->to_a < 0)
		    o_ptr->flags3 |= TR3_LIGHT_CURSE;
	}


	/* Analyze type */
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		{
			/* Rating boost */
			rating += 30;

			/* Mention the item */
			if (cheat_peek) object_mention(o_ptr);

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
				case SV_RING_STR:
				case SV_RING_CON:
				case SV_RING_DEX:
				case SV_RING_INT:
				{
					/* Stat bonus */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Ring of Speed! */
				case SV_RING_SPEED:
				{
					/* Base speed (1 to 10) */
					o_ptr->pval = randint(5) + m_bonus(5, level);

					/* Super-charge the ring */
					while (rand_int(100) < 50) o_ptr->pval++;

					/* Cursed Ring */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);

						break;
					}
					else
					{
						/* Rating boost */
						rating += 25;
					}

					/* Mention the item */
					if (cheat_peek) object_mention(o_ptr);

					break;
				}

				/* Searching */
				case SV_RING_SEARCHING:
				{
					/* Bonus to searching */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;
						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Flames, Acid, Ice, Lightning */
				case SV_RING_FLAMES:
				case SV_RING_ACID:
				case SV_RING_ICE:
				case SV_RING_LIGHTNING:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint(5) + m_bonus(10, level);
					break;
				}

				/* Weakness, Stupidity */
				case SV_RING_WEAKNESS:
				case SV_RING_STUPIDITY:
				{
					/* Broken */
					o_ptr->ident |= (IDENT_BROKEN);

					/* Cursed */
					o_ptr->flags3 |= TR3_LIGHT_CURSE;

					/* Penalize */
					o_ptr->pval = 0 - (1 + m_bonus(5, level));

					break;
				}

				/* WOE, Stupidity */
				case SV_RING_WOE:
				{
					/* Broken */
					o_ptr->ident |= (IDENT_BROKEN);

					/* Cursed */
					o_ptr->flags3 |= TR3_LIGHT_CURSE;

					/* Penalize */
					o_ptr->to_a = 0 - (5 + m_bonus(10, level));
					o_ptr->pval = 0 - (1 + m_bonus(5, level));

					break;
				}

				/* Ring of damage */
				case SV_RING_DAMAGE:
				{
					/* Bonus to damage */
					o_ptr->to_d = 5 + randint(3) + m_bonus(7, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse bonus */
						o_ptr->to_d = 0 - (o_ptr->to_d);
					}

					break;
				}

				/* Ring of Accuracy */
				case SV_RING_ACCURACY:
				{
					/* Bonus to hit */
					o_ptr->to_h = 5 + randint(3) + m_bonus(7, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse tohit */
						o_ptr->to_h = 0 - (o_ptr->to_h);
					}

					break;
				}

				/* Ring of Protection */
				case SV_RING_PROTECTION:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint(5) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse toac */
						o_ptr->to_a = 0 - (o_ptr->to_a);
					}

					break;
				}

				/* Ring of Slaying */
				case SV_RING_SLAYING:
				{
					/* Bonus to damage and to hit */
					o_ptr->to_d = randint(5) + m_bonus(5, level);
					o_ptr->to_h = randint(5) + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse bonuses */
						o_ptr->to_h = 0 - (o_ptr->to_h);
						o_ptr->to_d = 0 - (o_ptr->to_d);
					}

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
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Amulet of searching */
				case SV_AMULET_SEARCHING:
				{
					o_ptr->pval = randint(5) + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Broken */
						o_ptr->ident |= (IDENT_BROKEN);

						/* Cursed */
						o_ptr->flags3 |= TR3_LIGHT_CURSE;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Amulet of ESP -- never cursed */
				case SV_AMULET_ESP:
				{
					o_ptr->pval = randint(5) + m_bonus(5, level);

					break;
				}

				/* Amulet of the Magi -- never cursed */
				case SV_AMULET_THE_MAGI:
				{
					o_ptr->pval = 1 + m_bonus(3, level);
					o_ptr->to_a = randint(5) + m_bonus(5, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (cheat_peek) object_mention(o_ptr);

					break;
				}

				/* Amulet of Devotion -- never cursed */
				case SV_AMULET_DEVOTION:
				{
					o_ptr->pval = 1 + m_bonus(3, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (cheat_peek) object_mention(o_ptr);

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
					if (cheat_peek) object_mention(o_ptr);

					break;
				}

				/* Amulet of Trickery -- never cursed */
				case SV_AMULET_TRICKERY:
				{
					o_ptr->pval = randint(1) + m_bonus(3, level);

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (cheat_peek) object_mention(o_ptr);

					break;
				}

				/* Amulet of Doom -- always cursed */
				case SV_AMULET_DOOM:
				{
					/* Broken */
					o_ptr->ident |= (IDENT_BROKEN);

					/* Cursed */
					o_ptr->flags3 |= TR3_LIGHT_CURSE;

					/* Penalize */
					o_ptr->pval = 0 - (randint(5) + m_bonus(5, level));
					o_ptr->to_a = 0 - (randint(5) + m_bonus(5, level));

					break;
				}
			}

			break;
		}
	}
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
			/* Hack -- Torches & lanterns -- random fuel at 1/2 max */
			if (o_ptr->sval == SV_LITE_TORCH)
			{
				o_ptr->timeout = randint(FUEL_TORCH/2);
			}

			else if (o_ptr->sval == SV_LITE_LANTERN)
			{
				o_ptr->timeout = randint(FUEL_LAMP/2);
			}

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
			o_ptr->pval = randint(k_info[o_ptr->k_idx].level);

			/* Never exceed "difficulty" of 55 to 59 */
			if (o_ptr->pval > 55) o_ptr->pval = (s16b)(55 + rand_int(5));

			break;
		}
	}
}

static const u32b ego_sustains[] =
{
	TR2_SUST_STR,
	TR2_SUST_INT,
	TR2_SUST_WIS,
	TR2_SUST_DEX,
	TR2_SUST_CON,
	TR2_SUST_CHR,
};

static const u32b ego_resists[] =
{
	TR2_RES_ACID,
	TR2_RES_ELEC,
	TR2_RES_FIRE,
	TR2_RES_COLD,
	TR2_RES_POIS,
	TR2_RES_FEAR,
	TR2_RES_LITE,
	TR2_RES_DARK,
	TR2_RES_BLIND,
	TR2_RES_CONFU,
	TR2_RES_SOUND,
	TR2_RES_SHARD,
};

static const u32b ego_powers[] =
{
	TR3_SLOW_DIGEST,
	TR3_FEATHER,
	TR3_LITE,
	TR3_REGEN,
	TR3_TELEPATHY,
	TR3_SEE_INVIS,
	TR3_FREE_ACT,
	TR3_HOLD_LIFE,
};



/*
 * Complete the "creation" of an object by applying "magic" to the item
 *
 * This includes not only rolling for random bonuses, but also putting the
 * finishing touches on ego-items and artifacts, giving charges to wands and
 * staffs, giving fuel to lites, and placing traps on chests.
 *
 * In particular, note that "Instant Artifacts", if "created" by an external
 * routine, must pass through this function to complete the actual creation.
 *
 * The base "chance" of the item being "good" increases with the "level"
 * parameter, which is usually derived from the dungeon level, being equal
 * to the level plus 10, up to a maximum of 75.  If "good" is true, then
 * the object is guaranteed to be "good".  If an object is "good", then
 * the chance that the object will be "great" (ego-item or artifact), also
 * increases with the "level", being equal to half the level, plus 5, up to
 * a maximum of 20.  If "great" is true, then the object is guaranteed to be
 * "great".  At dungeon level 65 and below, 15/100 objects are "great".
 *
 * If the object is not "good", there is a chance it will be "cursed", and
 * if it is "cursed", there is a chance it will be "broken".  These chances
 * are related to the "good" / "great" chances above.
 *
 * Otherwise "normal" rings and amulets will be "good" half the time and
 * "cursed" half the time, unless the ring/amulet is always good or cursed.
 *
 * If "okay" is true, and the object is going to be "great", then there is
 * a chance that an artifact will be created.  This is true even if both the
 * "good" and "great" arguments are false.  Objects which are forced "great"
 * get three extra "attempts" to become an artifact.
 */
void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great)
{
	int i, rolls, f1, f2, power;


	/* Maximum "level" for various things */
	if (lev > MAX_DEPTH - 1) lev = MAX_DEPTH - 1;


	/* Base chance of being "good" */
	f1 = lev + 10;

	/* Maximal chance of being "good" */
	if (f1 > 75) f1 = 75;

	/* Base chance of being "great" */
	f2 = f1 / 2;

	/* Maximal chance of being "great" */
	if (f2 > 20) f2 = 20;


	/* Assume normal */
	power = 0;

	/* Roll for "good" */
	if (good || (rand_int(100) < f1))
	{
		/* Assume "good" */
		power = 1;

		/* Roll for "great" */
		if (great || (rand_int(100) < f2)) power = 2;
	}

	/* Roll for "cursed" */
	else if (rand_int(100) < f1)
	{
		/* Assume "cursed" */
		power = -1;

		/* Roll for "broken" */
		if (rand_int(100) < f2) power = -2;
	}

	/* Assume no rolls */
	rolls = 0;

	/* Get one roll if excellent */
	if (power >= 2) rolls = 1;

	/* Get four rolls if forced great */
	if (great) rolls = 4;

	/* Get no rolls if not allowed */
	if (!okay || o_ptr->name1) rolls = 0;

	/* Roll for artifacts if allowed */
	for (i = 0; i < rolls; i++)
	{
		/* Roll for an artifact */
		if (make_artifact(o_ptr)) break;
	}


	/* Hack -- analyze artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];

		/* Hack -- Mark the artifact as "created" */
		a_ptr->cur_num = 1;

		/* Extract the other fields */
		o_ptr->pval = a_ptr->pval;
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;
		o_ptr->to_a = a_ptr->to_a;
		o_ptr->to_h = a_ptr->to_h;
		o_ptr->to_d = a_ptr->to_d;
		o_ptr->weight = a_ptr->weight;

		/* Hack -- extract the "broken" flag */
		if (!a_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Hack -- extract the "cursed" flag */
		if (a_ptr->flags3 & TR3_LIGHT_CURSE)
			o_ptr->flags3 |= TR3_LIGHT_CURSE;

		/* Mega-Hack -- increase the rating */
		rating += 10;

		/* Mega-Hack -- increase the rating again */
		if (a_ptr->cost > 50000L) rating += 10;

		/* Set the good item flag */
		good_item_flag = TRUE;

		/* Cheat -- peek at the item */
		if (cheat_peek) object_mention(o_ptr);

		/* Done */
		return;
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
			if ((power > 1) || (power < -1))
			{
				int ego_power;

				ego_power = make_ego_item(o_ptr, lev, (bool)(good || great));

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
			if ((power > 1) || (power < -1))
			{
				int ego_power;

				ego_power = make_ego_item(o_ptr, lev, (bool)(good || great));

				if (ego_power) power = ego_power;
			}

			if (power) a_m_aux_2(o_ptr, lev, power);

			break;
		}

		case TV_RING:
		case TV_AMULET:
		{
			if (!power && (rand_int(100) < 50)) power = -1;
			a_m_aux_3(o_ptr, lev, power);
			break;
		}

		case TV_LITE:
		{
			if ((power > 1) || (power < -1))
			{
				make_ego_item(o_ptr, lev, (bool)(good || great));
			}

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

		/* Extra powers */
		switch (e_ptr->xtra)
		{
			case OBJECT_XTRA_TYPE_SUSTAIN:
			{
				o_ptr->flags2 |= ego_sustains[rand_int(N_ELEMENTS(ego_sustains))];
				break;
			}

			case OBJECT_XTRA_TYPE_RESIST:
			{
				o_ptr->flags2 |= ego_resists[rand_int(N_ELEMENTS(ego_resists))];
				break;
			}

			case OBJECT_XTRA_TYPE_POWER:
			{
				o_ptr->flags3 |= ego_powers[rand_int(N_ELEMENTS(ego_powers))];
				break;
			}
		}

		/* Hack -- acquire "broken" flag */
		if (!e_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Hack -- acquire "cursed" flag */
		if (e_ptr->flags3 & (TR3_LIGHT_CURSE))
		    o_ptr->flags3 |= TR3_LIGHT_CURSE;

		/* Hack -- apply extra penalties if needed */
		if (cursed_p(o_ptr) || broken_p(o_ptr))
		{
			/* Hack -- obtain bonuses */
			if (e_ptr->max_to_h > 0) o_ptr->to_h -= randint(e_ptr->max_to_h);
			if (e_ptr->max_to_d > 0) o_ptr->to_d -= randint(e_ptr->max_to_d);
			if (e_ptr->max_to_a > 0) o_ptr->to_a -= randint(e_ptr->max_to_a);

			/* Hack -- obtain pval */
			if (e_ptr->max_pval > 0) o_ptr->pval -= randint(e_ptr->max_pval);
		}

		/* Hack -- apply extra bonuses if needed */
		else
		{
			/* Hack -- obtain bonuses */
			if (e_ptr->max_to_h > 0) o_ptr->to_h += randint(e_ptr->max_to_h);
			if (e_ptr->max_to_d > 0) o_ptr->to_d += randint(e_ptr->max_to_d);
			if (e_ptr->max_to_a > 0) o_ptr->to_a += randint(e_ptr->max_to_a);

			/* Hack -- obtain pval */
			if (e_ptr->max_pval > 0) o_ptr->pval += randint(e_ptr->max_pval);
		}

		/* Hack -- apply rating bonus */
		rating += e_ptr->rating;

		/* Cheat -- describe the item */
		if (cheat_peek) object_mention(o_ptr);

		/* Done */
		return;
	}


	/* Examine real objects */
	if (o_ptr->k_idx)
	{
		object_kind *k_ptr = &k_info[o_ptr->k_idx];

		/* Hack -- acquire "broken" flag */
		if (!k_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Hack -- acquire "cursed" flag */
		if (k_ptr->flags3 & (TR3_LIGHT_CURSE))
		    o_ptr->flags3 |= TR3_LIGHT_CURSE;
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
static bool kind_is_good(int k_idx)
{
	object_kind *k_ptr = &k_info[k_idx];

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
		
		/* Potions -- Potions of life, healing, *healing* are good,
		 * restore mana for spell casters are good,
		 * as are stat potions, when the stat is not maximised,
		 * as is augmentation (acts as a potion of 'restoration' if all
		 * stats are maximised).
		 * 
		 * XXX If we make too many useful items 'good' we may want to
		 * consider limiting the total number of good drops to uniques
		 * and truely nasty monsters.
		 */
		case TV_POTION:
		{
			if (k_ptr->sval == SV_POTION_HEALING) return (TRUE);
			if (k_ptr->sval == SV_POTION_STAR_HEALING) return (TRUE);
			if (k_ptr->sval == SV_POTION_LIFE) return (TRUE);
			if ((k_ptr->sval == SV_POTION_RESTORE_MANA) && (p_ptr->msp > 0)) return (TRUE);
			if ((k_ptr->sval >= SV_POTION_INC_STR) && (k_ptr->sval <= SV_POTION_INC_CHR))
			{
				if (p_ptr->stat_cur[k_ptr->sval - SV_POTION_INC_STR] < 18+100) return (TRUE);
			}
			if (k_ptr->sval == SV_POTION_AUGMENTATION) return (TRUE);
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
	int prob, base;
	object_kind *k_ptr;


	/* Chance of "special object" */
	prob = (good ? 10 : 1000);

	/* Base level for the object */
	base = (good ? (lev + 10) : lev);


	/* Generate a special artifact, or a normal object */
	if ((rand_int(prob) != 0) || !make_artifact_special(j_ptr, lev))
	{
		int k_idx;

#if 0
		/* Good objects */
		if (good)
		{
			/* Activate restriction */
			get_obj_num_hook = kind_is_good;

			/* Prepare allocation table */
			get_obj_num_prep();
		}
#endif

		/* Pick a random object */
		k_idx = get_obj_num(base);

#if 0
		/* Good objects */
		if (good)
		{
			/* Clear restriction */
			get_obj_num_hook = NULL;

			/* Prepare allocation table */
			get_obj_num_prep();
		}
#endif

		/* Handle failure */
		if (!k_idx) return (FALSE);

		/* Prepare the object */
		object_prep(j_ptr, k_idx);
	}

	/* Apply magic (allow artifacts) */
	apply_magic(j_ptr, lev, TRUE, good, great);


	/* Generate multiple items */
	/* Imported from Steamband and Sangband */
	/* XXX Will probably not work so well for stacks of potions (yet) */
	k_ptr = &k_info[j_ptr->k_idx];

	if (k_ptr->gen_mult_prob >= 100 ||
	    k_ptr->gen_mult_prob >= randint(100))
	{
		j_ptr->number = damroll(k_ptr->gen_dice, k_ptr->gen_side);
	}


	/* Notice "okay" out-of-depth objects */
	if (!cursed_p(j_ptr) && !broken_p(j_ptr) &&
	    (k_info[j_ptr->k_idx].level > p_ptr->depth))
	{
		/* Rating increase */
		rating += (k_info[j_ptr->k_idx].level - p_ptr->depth);

		/* Cheat -- peek at items */
		if (cheat_peek) object_mention(j_ptr);
	}

	/* Success */
	return (TRUE);
}



/*
 * XXX XXX XXX Do not use these hard-coded values.
 */
#define MAX_GOLD	18	/* Number of "gold" entries */

/*
 * Make a treasure object
 *
 * The location must be a legal, clean, floor grid.
 */
bool make_gold(object_type *j_ptr, int lev)
{
	int sval;
	int k_idx;
	s32b base;


	/* Hack -- Pick a Treasure variety */
	sval = ((randint(lev + 2) + 2) / 2);

	/* Apply "extra" magic */
	if (one_in_(GREAT_OBJ))
	{
		sval += randint(lev + 1);
	}

	/* Hack -- Creeping Coins only generate "themselves" */
	if (coin_type) sval = coin_type;

	/* Do not create "illegal" Treasure Types */
	if (sval > MAX_GOLD) sval = MAX_GOLD;

	k_idx = lookup_kind(TV_GOLD, sval);

	/* Prepare a gold object */
	object_prep(j_ptr, k_idx);

	/* Hack -- Base coin cost */
	base = k_info[k_idx].cost;

	/* Determine how much the treasure is "worth" */
	j_ptr->pval = (base + (8L * randint(base)) + randint(8));

	/* Success */
	return (TRUE);
}

/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
int object_pseudo_heavy(const object_type *o_ptr)
{
	if (artifact_p(o_ptr))
	{
		if (cursed_p(o_ptr) || broken_p(o_ptr))
			return INSCRIP_TERRIBLE;
		else
			return INSCRIP_SPECIAL;
	}

	if (ego_item_p(o_ptr))
	{
		if (cursed_p(o_ptr) || broken_p(o_ptr))
			return INSCRIP_WORTHLESS;
		else
			return INSCRIP_EXCELLENT;
	}

	if (cursed_p(o_ptr))
		return INSCRIP_CURSED;
	else if (broken_p(o_ptr))
		return INSCRIP_BROKEN;
	else if (o_ptr->to_a > 0 || (o_ptr->to_h + o_ptr->to_d > 0))
		return INSCRIP_GOOD;

	/* Default to "average" */
	return (INSCRIP_AVERAGE);
}



/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
int object_pseudo_light(const object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return (INSCRIP_CURSED);

	/* Broken items (all of them) */
	if (broken_p(o_ptr)) return (INSCRIP_BROKEN);

	/* Artifacts -- except cursed/broken ones */
	if (artifact_p(o_ptr)) return (INSCRIP_GOOD);

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return (INSCRIP_GOOD);

	/* Good armor bonus */
	if (o_ptr->to_a > 0) return (INSCRIP_GOOD);

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 0) return (INSCRIP_GOOD);

	/* No feeling */
	return (0);
}

