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
#include "cave.h"
#include "tvalsval.h"


/*
 * There is a 1/20 (5%) chance of inflating the requested object level
 * during the creation of an object (see "get_obj_num()" in "object.c").
 * Lower values yield better objects more often.
 */
#define GREAT_OBJ   20
  
/*
 * There is a 1/20 (5%) chance that ego-items with an inflated base-level are
 * generated when an object is turned into an ego-item (see make_ego_item()
 * in object2.c). As above, lower values yield better ego-items more often.
 */
#define GREAT_EGO   20


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
		const object_kind *kind = &k_info[item];

		int min = kind->alloc_min;
		int max = kind->alloc_max;

		/* If an item doesn't have a rarity, move on */
		if (!kind->alloc_prob) continue;

		/* Go through all the dungeon levels */
		for (lev = 0; lev <= MAX_O_DEPTH; lev++)
		{
			int rarity = kind->alloc_prob;

			/* Save the probability in the standard table */
			if ((lev < min) || (lev > max)) rarity = 0;
			obj_total[lev] += rarity;
			obj_alloc[(lev * k_max) + item] = rarity;

			/* Save the probability in the "great" table if relevant */
			if (!kind_is_good(kind)) rarity = 0;
			obj_total_great[lev] += rarity;
			obj_alloc_great[(lev * k_max) + item] = rarity;
		}
	}

	return TRUE;
}




/*
 * Choose an object kind given a dungeon level to choose it for.
 */
object_kind *get_obj_num(int level, bool good)
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
	return objkind_byid(item);
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
		msg("Artifact (%s)", o_name);
	else if (ego_item_p(o_ptr))
		msg("Ego-item (%s)", o_name);
	else
		msg("Object (%s)", o_name);
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

	return (flags_test(e_info[e_idx].flags, OF_SIZE, OF_CURSE_MASK, FLAG_END) ? -2 : 2);
}


/**
 * Copy artifact data to a normal object, and set various slightly hacky
 * globals.
 */
static void copy_artifact_data(object_type *o_ptr, const artifact_type *a_ptr)
{
	int i;

	/* Extract the other fields */
	for (i = 0; i < a_ptr->num_pvals; i++)
		if (a_ptr->pval[i])
			o_ptr->pval[i] = a_ptr->pval[i];
	o_ptr->num_pvals = a_ptr->num_pvals;
	o_ptr->ac = a_ptr->ac;
	o_ptr->dd = a_ptr->dd;
	o_ptr->ds = a_ptr->ds;
	o_ptr->to_a = a_ptr->to_a;
	o_ptr->to_h = a_ptr->to_h;
	o_ptr->to_d = a_ptr->to_d;
	o_ptr->weight = a_ptr->weight;

	/* Hack -- extract the "cursed" flags */
	if (cursed_p(a_ptr))
	{
		bitflag curse_flags[OF_SIZE];

		of_copy(curse_flags, a_ptr->flags);
		flags_mask(curse_flags, OF_SIZE, OF_CURSE_MASK, FLAG_END);
		of_union(o_ptr->flags, curse_flags);
	}

	/* Mega-Hack -- increase the level rating
	 * - a sizeable increase for any artifact (c.f. up to 30 for ego items)
	 * - a bigger increase for more powerful artifacts
	 */
	cave->rating += 30;
	cave->rating += object_power(o_ptr, FALSE, NULL, TRUE) / 25;

	/* Set the good item flag */
	cave->good_item = TRUE;

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
 * normally outweighed by the "rarity" rolls for those artifacts.
 */
static bool make_artifact_special(object_type *o_ptr, int level)
{
	int i;

	object_kind *kind;


	/* No artifacts, do nothing */
	if (OPT(birth_no_artifacts)) return (FALSE);

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
		if (a_ptr->alloc_min > p_ptr->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (a_ptr->alloc_min - p_ptr->depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Enforce maximum depth (strictly) */
		if (a_ptr->alloc_max < p_ptr->depth) continue;

		/* Artifact "rarity roll" */
		if (randint1(100) > a_ptr->alloc_prob) continue;

		/* Find the base object */
		kind = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Enforce minimum "object" level (loosely) */
		if (kind->level > level)
		{
			/* Get the "out-of-depth factor" */
			int d = (kind->level - level) * 5;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Assign the template */
		object_prep(o_ptr, kind, a_ptr->alloc_min, RANDOMISE);

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
 * Attempt to change an object into an artifact.  If the object's name1
 * is already set, use that artifact.  Otherwise, look for a suitable
 * artifact and attempt to use it.
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(object_type *o_ptr)
{
	artifact_type *a_ptr;
	int i;


	/* No artifacts, do nothing */
	if (OPT(birth_no_artifacts) &&
	    o_ptr->name1 != ART_GROND &&
	    o_ptr->name1 != ART_MORGOTH)
		return (FALSE);

	/* No artifacts in the town */
	if (!p_ptr->depth) return (FALSE);

	/* Paranoia -- no "plural" artifacts */
	if (o_ptr->number != 1) return (FALSE);

	/* Check the artifact list (skip the "specials") */
	for (i = ART_MIN_NORMAL; !o_ptr->name1 && i < z_info->a_max; i++)
	{
		a_ptr = &a_info[i];

		/* Skip "empty" items */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->created) continue;

		/* Must have the correct fields */
		if (a_ptr->tval != o_ptr->tval) continue;
		if (a_ptr->sval != o_ptr->sval) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->alloc_min > p_ptr->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (a_ptr->alloc_min - p_ptr->depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Enforce maximum depth (strictly) */
		if (a_ptr->alloc_max < p_ptr->depth) continue;

		/* We must make the "rarity roll" */
		if (randint1(100) > a_ptr->alloc_prob) continue;

		/* Mark the item as an artifact */
		o_ptr->name1 = i;
	}

	if (o_ptr->name1)
	{
		a_ptr = &a_info[o_ptr->name1];

		/* Copy across all the data from the artifact struct */
		copy_artifact_data(o_ptr, a_ptr);

		/* Mark the artifact as "created" */
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
				o_ptr->pval[which_pval(o_ptr, OF_TUNNEL)]
					= 0 - (5 + randint1(5));
			}

			/* Bad */
			else if (power < 0)
			{
				/* Hack -- Reverse digging bonus */
				o_ptr->pval[which_pval(o_ptr, OF_TUNNEL)]
					= -o_ptr->pval[which_pval(o_ptr, OF_TUNNEL)];
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
			cave->rating += object_power(o_ptr, FALSE, NULL, TRUE) / 15;

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
	/* Apply curses */
	if (power < 0)
	{
		/* Rings */
		if (o_ptr->tval == TV_RING)
		{
			switch (o_ptr->sval)
			{
				case SV_RING_STRENGTH:
				case SV_RING_CONSTITUTION:
				case SV_RING_DEXTERITY:
				case SV_RING_INTELLIGENCE:
				case SV_RING_SEARCHING:
				case SV_RING_DAMAGE:
				case SV_RING_ACCURACY:
				case SV_RING_PROTECTION:
				case SV_RING_SLAYING:
				{
					/* CC: multiple pvals deliberately not 
					 * affected pending new curses */
					o_ptr->pval[DEFAULT_PVAL] = -o_ptr->pval[DEFAULT_PVAL];
					o_ptr->to_h = -o_ptr->to_h;
					o_ptr->to_d = -o_ptr->to_d;
					o_ptr->to_a = -o_ptr->to_a;
					of_on(o_ptr->flags, OF_LIGHT_CURSE);

					break;
				}
			}
		}

		/* Amulets */
		else if (o_ptr->tval == TV_AMULET)
		{
			switch (o_ptr->sval)
			{
				case SV_AMULET_WISDOM:
				case SV_AMULET_CHARISMA:
				case SV_AMULET_INFRAVISION:
				case SV_AMULET_SEARCHING:
				{
					/* CC: multiple pvals deliberately not 
					 * affected pending new curses */
					o_ptr->pval[DEFAULT_PVAL] = -o_ptr->pval[DEFAULT_PVAL];
					o_ptr->to_h = -o_ptr->to_h;
					o_ptr->to_d = -o_ptr->to_d;
					o_ptr->to_a = -o_ptr->to_a;
					of_on(o_ptr->flags, OF_LIGHT_CURSE);

					break;
				}
			}
		}
	}


	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
		case TV_RING:
		{
			/* Analyze */
			
			switch (o_ptr->sval)
			{
				case SV_RING_SPEED:
				{
					/* Super-charge the ring */
					while (randint0(100) < 50) o_ptr->pval[which_pval(o_ptr, OF_SPEED)]++;

					if (power >= 0)
					{
						/* Rating boost */
						cave->rating += 25;

						/* Mention the item */
						if (OPT(cheat_peek)) object_mention(o_ptr);
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
				case SV_AMULET_THE_MAGI:
				case SV_AMULET_DEVOTION:
				case SV_AMULET_WEAPONMASTERY:
				case SV_AMULET_TRICKERY:
				{
					/* Boost the rating */
					cave->rating += 25;

					/* Mention the item */
					if (OPT(cheat_peek)) object_mention(o_ptr);

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
	/* Unused parameters */
	(void)level;
	(void)power;

	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
		case TV_LIGHT:
		{
			/* Default fuel levels */
			if (o_ptr->sval == SV_LIGHT_TORCH)
				o_ptr->timeout = DEFAULT_TORCH;
			else if (o_ptr->sval == SV_LIGHT_LANTERN)
				o_ptr->timeout = DEFAULT_LAMP;

			break;
		}

		case TV_CHEST:
		{
			/* Hack -- skip ruined chests */
			if (o_ptr->kind->level <= 0) break;

			/* Hack -- pick a "difficulty" */
			o_ptr->pval[DEFAULT_PVAL] = randint1(o_ptr->kind->level);

			/* Never exceed "difficulty" of 55 to 59 */
			if (o_ptr->pval[DEFAULT_PVAL] > 55) o_ptr->pval[DEFAULT_PVAL] = (s16b)(55 + randint0(5));

			break;
		}
	}
}

static const int ego_sustains[] =
{
	OF_SUST_STR,
	OF_SUST_INT,
	OF_SUST_WIS,
	OF_SUST_DEX,
	OF_SUST_CON,
	OF_SUST_CHR,
};

void set_ego_xtra_sustain(bitflag flags[OF_SIZE])
{
	size_t i;

	of_wipe(flags);

	for (i = 0; i < N_ELEMENTS(ego_sustains); i++)
		of_on(flags, ego_sustains[i]);
}

static const int ego_resists[] =
{
	OF_RES_POIS,
	OF_RES_LIGHT,
	OF_RES_DARK,
	OF_RES_SOUND,
	OF_RES_SHARD,
	OF_RES_NEXUS,
	OF_RES_NETHR,
	OF_RES_CHAOS,
	OF_RES_DISEN,
};

void set_ego_xtra_resist(bitflag flags[OF_SIZE])
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(ego_resists); i++)
		of_on(flags, ego_resists[i]);
}

static const int ego_powers[] =
{
	OF_SLOW_DIGEST,
	OF_FEATHER,
	OF_LIGHT,
	OF_REGEN,
	OF_TELEPATHY,
	OF_SEE_INVIS,
	OF_FREE_ACT,
	OF_HOLD_LIFE,
	OF_RES_BLIND,
	OF_RES_CONFU,
	OF_RES_FEAR,
};

void set_ego_xtra_power(bitflag flags[OF_SIZE])
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(ego_powers); i++)
		of_on(flags, ego_powers[i]);
}

/**
 * This is a safe way to choose a random new flag to add to an object.
 * It takes the existing flags, an array of new attrs, and the size of
 * the array, and returns an entry from attrs, or 0 if there are no
 * new attrs.
 */
static int get_new_attr(bitflag flags[OF_SIZE], const int attrs[], size_t size)
{
	size_t i;
	int options = 0;
	int flag = 0;

	for (i = 0; i < size; i++)
	{
		/* skip this one if the flag is already present */
		if (of_has(flags, attrs[i])) continue;

		/* each time we find a new possible option, we have a 1-in-N chance of
		 * choosing it and an (N-1)-in-N chance of keeping a previous one */
		if (one_in_(++options)) flag = attrs[i];
	}

	return flag;
}


/*
 * Prepare an object based on an object kind.
 * Use the specified randomization aspect
 */
void object_prep(object_type *o_ptr, struct object_kind *k, int lev, aspect rand_aspect)
{
	int i;

	/* Clear the record */
	(void)WIPE(o_ptr, object_type);

	o_ptr->kind = k;

	/* Efficiency -- tval/sval */
	o_ptr->tval = k->tval;
	o_ptr->sval = k->sval;

	/* Default number */
	o_ptr->number = 1;

	/* Default "pvals" */
	for (i = 0; i < k->num_pvals; i++)
		o_ptr->pval[i] = randcalc(k->pval[i], lev, rand_aspect);
	o_ptr->num_pvals = k->num_pvals;

	/* Default weight */
	o_ptr->weight = k->weight;
	
	/* Assign charges (wands/staves only) */
	if (o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF)
		o_ptr->pval[DEFAULT_PVAL] = randcalc(k->charge, lev, rand_aspect);

	/* Default magic */
	o_ptr->to_h = randcalc(k->to_h, lev, rand_aspect);
	o_ptr->to_d = randcalc(k->to_d, lev, rand_aspect);
	o_ptr->to_a = randcalc(k->to_a, lev, rand_aspect);

	/* Default power */
	o_ptr->ac = k->ac;
	o_ptr->dd = k->dd;
	o_ptr->ds = k->ds;

	/* Hack -- cursed items are always "cursed" */
	if (of_has(k->flags, OF_LIGHT_CURSE))
	    of_on(o_ptr->flags, OF_LIGHT_CURSE);
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
	int i;
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
	if (allow_artifacts)
	{
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

		case TV_LIGHT:
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
		bitflag flags[OF_SIZE];

		object_flags(o_ptr, flags);

		/* Extra powers */
		if (e_ptr->xtra == OBJECT_XTRA_TYPE_SUSTAIN)
			of_on(o_ptr->flags, get_new_attr(flags, ego_sustains,
											N_ELEMENTS(ego_sustains)));
		else if (e_ptr->xtra == OBJECT_XTRA_TYPE_RESIST)
			of_on(o_ptr->flags, get_new_attr(flags, ego_resists,
											N_ELEMENTS(ego_resists)));
		else if (e_ptr->xtra == OBJECT_XTRA_TYPE_POWER)
			of_on(o_ptr->flags, get_new_attr(flags, ego_powers,
											N_ELEMENTS(ego_powers)));

		/* Hack -- acquire "cursed" flags */
		if (cursed_p(e_ptr))
		{
			bitflag curse_flags[OF_SIZE];

			of_copy(curse_flags, e_ptr->flags);
			flags_mask(curse_flags, OF_SIZE, OF_CURSE_MASK, FLAG_END);
			of_union(o_ptr->flags, curse_flags);
		}

		/* Hack -- apply extra penalties if needed */
		if (cursed_p(o_ptr))
		{
			/* Apply extra ego bonuses */
			o_ptr->to_h -= randcalc(e_ptr->to_h, lev, RANDOMISE);
			o_ptr->to_d -= randcalc(e_ptr->to_d, lev, RANDOMISE);
			o_ptr->to_a -= randcalc(e_ptr->to_a, lev, RANDOMISE);

			/* CC: multiple pvals left untouched pending new curses */
			o_ptr->pval[DEFAULT_PVAL] -= randcalc(e_ptr->pval[DEFAULT_PVAL], lev, RANDOMISE);

			/* Apply minima */
			if (o_ptr->to_h > -1 * e_ptr->min_to_h)
				o_ptr->to_h = -1 * e_ptr->min_to_h;
			if (o_ptr->to_d > -1 * e_ptr->min_to_d)
				o_ptr->to_d = -1 * e_ptr->min_to_d;
			if (o_ptr->to_a > -1 * e_ptr->min_to_a)
				o_ptr->to_a = -1 * e_ptr->min_to_a;

			if (o_ptr->pval[DEFAULT_PVAL]
				> -1 * e_ptr->min_pval[DEFAULT_PVAL])
				o_ptr->pval[DEFAULT_PVAL]
					= -1 * e_ptr->min_pval[DEFAULT_PVAL];
		}

		/* Hack -- apply extra bonuses if needed */
		else
		{
			/* Apply extra ego bonuses */
			o_ptr->to_h += randcalc(e_ptr->to_h, lev, RANDOMISE);
			o_ptr->to_d += randcalc(e_ptr->to_d, lev, RANDOMISE);
			o_ptr->to_a += randcalc(e_ptr->to_a, lev, RANDOMISE);

			/* Apply ego pvals */
			for (i = 0; i < e_ptr->num_pvals; i++) {
				if (!o_ptr->pval[i]) o_ptr->num_pvals++;
				o_ptr->pval[i] += randcalc(e_ptr->pval[i], lev, RANDOMISE);
			}

			/* Apply minimums */
			if (o_ptr->to_h < e_ptr->min_to_h) o_ptr->to_h = e_ptr->min_to_h;
			if (o_ptr->to_d < e_ptr->min_to_d) o_ptr->to_d = e_ptr->min_to_d;
			if (o_ptr->to_a < e_ptr->min_to_a) o_ptr->to_a = e_ptr->min_to_a;

			for (i = 0; i < e_ptr->num_pvals; i++)
			{
				if (o_ptr->pval[i] < e_ptr->min_pval[i])
					o_ptr->pval[i] = e_ptr->min_pval[i];
			}
		}

		/* Hack -- apply rating bonus */
		cave->rating += e_ptr->rating;

		/* Cheat -- describe the item */
		if (OPT(cheat_peek)) object_mention(o_ptr);

		/* Done */
		return;
	}


	/* Hack -- acquire "cursed" flag */
	if (o_ptr->kind && cursed_p(o_ptr->kind))
	{
		bitflag curse_flags[OF_SIZE];

		of_copy(curse_flags, o_ptr->kind->flags);
		flags_mask(curse_flags, OF_SIZE, OF_CURSE_MASK, FLAG_END);
		of_union(o_ptr->flags, curse_flags);
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
static bool kind_is_good(const object_kind *kind)
{
	/* Analyze the item type */
	switch (kind->tval)
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
			if (randcalc(kind->to_a, 0, MINIMISE) < 0) return (FALSE);
			return (TRUE);
		}

		/* Weapons -- Good unless damaged */
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		{
			if (randcalc(kind->to_h, 0, MINIMISE) < 0) return (FALSE);
			if (randcalc(kind->to_d, 0, MINIMISE) < 0) return (FALSE);
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
			if (kind->sval >= SV_BOOK_MIN_GOOD) return (TRUE);
			return (FALSE);
		}

		/* Rings -- Rings of Speed are good */
		case TV_RING:
		{
			if (kind->sval == SV_RING_SPEED) return (TRUE);
			return (FALSE);
		}

		/* Amulets -- Amulets of the Magi are good */
		case TV_AMULET:
		{
			if (kind->sval == SV_AMULET_THE_MAGI) return (TRUE);
			if (kind->sval == SV_AMULET_DEVOTION) return (TRUE);
			if (kind->sval == SV_AMULET_WEAPONMASTERY) return (TRUE);
			if (kind->sval == SV_AMULET_TRICKERY) return (TRUE);
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
bool make_object(struct cave *c, object_type *j_ptr, int lev, bool good, bool great)
{
	int base;
	object_kind *kind;

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
	kind = get_obj_num(base, good || great);
	if (!kind) return FALSE;

	/* Prepare the object */
	object_prep(j_ptr, kind, lev, RANDOMISE);

	/* Apply magic (allow artifacts) */
	apply_magic(j_ptr, lev, TRUE, good, great);

	/* Generate multiple items */
	if (kind->gen_mult_prob >= 100 ||
	    kind->gen_mult_prob >= randint1(100))
	{
		j_ptr->number = randcalc(kind->stack_size, lev, RANDOMISE);
	}


	/* Notice "okay" out-of-depth objects */
	if (!cursed_p(j_ptr) && (kind->level > c->depth))
	{
		/* Rating increase */
		c->rating += (kind->alloc_min - c->depth);

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

	/* This average is 20 at dlev0, 105 at dlev40, 220 at dlev100. */
	/* Follows the formula: y=2x+20 */
	s32b avg = 2*lev + 20;
	s32b spread = lev + 10;
	s32b value = rand_spread(avg, spread);

	/* Increase the range to infinite, moving the average to 110% */
	while (one_in_(100) && value * 10 <= MAX_SHORT)
		value *= 10;

	/* Pick a treasure variety scaled by level, or force a type */
	if (coin_type != SV_GOLD_ANY)
		sval = coin_type;
	else
		sval = (((value * 100) / MAX_GOLD_DROP) * SV_GOLD_MAX) / 100;

	/* Do not create illegal treasure types */
	if (sval >= SV_GOLD_MAX) sval = SV_GOLD_MAX - 1;

	/* Prepare a gold object */
	object_prep(j_ptr, lookup_kind(TV_GOLD, sval), lev, RANDOMISE);

	/* If we're playing with no_selling, increase the value */
	if (OPT(birth_no_selling)) value = 5 * value;

	j_ptr->pval[DEFAULT_PVAL] = value;
}
