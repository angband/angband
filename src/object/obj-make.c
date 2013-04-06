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
#include "init.h"
#include "object/tvalsval.h"
#include "object/pval.h"
#include "object/slays.h"

/*
 * The chance of inflating the requested object level (1/x).
 * Lower values yield better objects more often.
 */
#define GREAT_OBJ   20

/*
 * There is a 1/20 (5%) chance that ego-items with an inflated base-level are
 * generated when an object is turned into an ego-item (see make_ego_item()
 * in object2.c). As above, lower values yield better ego-items more often.
 */
#define GREAT_EGO   20

/* Define a value for minima which will be ignored. */
#define NO_MINIMUM 	255

static s16b alloc_ego_size;
static alloc_entry *alloc_ego_table;

static void init_ego_allocs(void) {
	struct alloc_entry *table;
	int i;
	ego_item_type *e_ptr;
	s16b num[MAX_DEPTH];
	s16b aux[MAX_DEPTH];

	/* Clear the "aux" array */
	(void)C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(num, MAX_DEPTH, s16b);

	/* Size of "alloc_ego_table" */
	alloc_ego_size = 0;

	/* Scan the ego items */
	for (i = 1; i < z_info->e_max; i++)
	{
		/* Get the i'th ego item */
		e_ptr = &e_info[i];

		/* Legal items */
		if (e_ptr->rarity)
		{
			/* Count the entries */
			alloc_ego_size++;

			/* Group by level */
			num[e_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/*** Initialize ego-item allocation info ***/

	/* Allocate the alloc_ego_table */
	alloc_ego_table = C_ZNEW(alloc_ego_size, alloc_entry);

	/* Get the table entry */
	table = alloc_ego_table;

	/* Scan the ego-items */
	for (i = 1; i < z_info->e_max; i++)
	{
		/* Get the i'th ego item */
		e_ptr = &e_info[i];

		/* Count valid pairs */
		if (e_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = e_ptr->level;

			/* Extract the base probability */
			p = (100 / e_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x-1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}

}

static void cleanup_ego_allocs(void) {
	FREE(alloc_ego_table);
}

/*** Make an ego item ***/

/**
 * This is a safe way to choose a random new flag to add to an object.
 * It takes the existing flags and an array of new flags,
 * and returns an entry from newf, or 0 if there are no
 * new flags available.
 */
static int get_new_attr(bitflag flags[OF_SIZE], bitflag newf[OF_SIZE])
{
	size_t i;
	int options = 0, flag = 0;

	for (i = of_next(newf, FLAG_START); i != FLAG_END; i = of_next(newf, i + 1))
	{
		/* skip this one if the flag is already present */
		if (of_has(flags, i)) continue;

		/* each time we find a new possible option, we have a 1-in-N chance of
		 * choosing it and an (N-1)-in-N chance of keeping a previous one */
		if (one_in_(++options)) flag = i;
	}

	return flag;
}


/**
 * Select an ego-item that fits the object's tval and sval.
 */
static struct ego_item *ego_find_random(object_type *o_ptr, int level)
{
	int i, j, ood_chance;
	long total = 0L;

	/* XXX alloc_ego_table &c should be static to this file */
	alloc_entry *table = alloc_ego_table;
	ego_item_type *ego;

	/* Go through all possible ego items and find ones which fit this item */
	for (i = 0; i < alloc_ego_size; i++) {
		/* Reset any previous probability of this type being picked */
		table[i].prob3 = 0;

		if (level < table[i].level)
			continue;

		/* Access the ego item */
		ego = &e_info[table[i].index];
        
        /* enforce maximum */
        if (level > ego->alloc_max) continue;
        
        /* roll for Out of Depth (ood) */
        if (level < ego->alloc_min){
            ood_chance = MAX(2, (ego->alloc_min - level) / 3);
            if (!one_in_(ood_chance)) continue;
        }

		/* XXX Ignore cursed items for now */
		if (cursed_p(ego->flags)) continue;

		/* Test if this is a legal ego-item type for this object */
		for (j = 0; j < EGO_TVALS_MAX; j++) {
			/* Require identical base type */
			if (o_ptr->tval == ego->tval[j] &&
					o_ptr->sval >= ego->min_sval[j] &&
					o_ptr->sval <= ego->max_sval[j]) {
				table[i].prob3 = table[i].prob2;
				break;
			}
		}

		/* Total */
		total += table[i].prob3;
	}

	if (total) {
		long value = randint0(total);
		for (i = 0; i < alloc_ego_size; i++) {
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		return &e_info[table[i].index];
	}

	return NULL;
}


/**
 * Apply generation magic to an ego-item.
 */
void ego_apply_magic(object_type *o_ptr, int level)
{
	int i, flag, x;

	bitflag flags[OF_SIZE], newf[OF_SIZE], f2[OF_SIZE];
	object_flags(o_ptr, flags);

	/* Extra powers */
	if (o_ptr->ego->xtra == OBJECT_XTRA_TYPE_SUSTAIN)
		create_mask(newf, FALSE, OFT_SUST, OFT_MAX);
	else if (o_ptr->ego->xtra == OBJECT_XTRA_TYPE_RESIST)
		create_mask(newf, FALSE, OFT_HRES, OFT_MAX);
	else if (o_ptr->ego->xtra == OBJECT_XTRA_TYPE_POWER)
		create_mask(newf, FALSE, OFT_PROT, OFT_MISC, OFT_MAX);

	if (o_ptr->ego->xtra)
		of_on(o_ptr->flags, get_new_attr(flags, newf));

	/* Apply extra o_ptr->ego bonuses */
	o_ptr->to_h += randcalc(o_ptr->ego->to_h, level, RANDOMISE);
	o_ptr->to_d += randcalc(o_ptr->ego->to_d, level, RANDOMISE);
	o_ptr->to_a += randcalc(o_ptr->ego->to_a, level, RANDOMISE);

	/* Apply pvals */
	of_copy(f2, o_ptr->ego->flags);
	for (i = 0; i < o_ptr->ego->num_pvals; i++) {
		of_copy(flags, o_ptr->ego->pval_flags[i]);
		x = randcalc(o_ptr->ego->pval[i], level, RANDOMISE);
		for (flag = of_next(flags, FLAG_START); flag != FLAG_END;
				flag = of_next(flags, flag + 1))
			/* Prevent phantom flags */
			if (x)
				object_add_pval(o_ptr, x, flag);
			else
				of_off(f2, flag);
	}

	/* Apply remaining flags */
	of_union(o_ptr->flags, f2);

	return;
}

/**
 * Apply minimum pvals to an ego item.
 */
void ego_min_pvals(object_type *o_ptr)
{
	int i, j, flag;

	if (!o_ptr->ego) return;

	for (i = 0; i < o_ptr->num_pvals; i++)
		for (j = 0; j < o_ptr->ego->num_pvals; j++)
			for (flag = of_next(o_ptr->ego->pval_flags[j], FLAG_START);
					flag != FLAG_END;
					flag = of_next(o_ptr->ego->pval_flags[j], flag + 1))
				if ((!of_has(o_ptr->flags, flag) &&
						o_ptr->ego->min_pval[j] > 0) ||
						(o_ptr->ego->min_pval[j] != NO_MINIMUM
						&& of_has(o_ptr->pval_flags[i], flag) &&
						o_ptr->pval[i] < o_ptr->ego->min_pval[j]))
					object_add_pval(o_ptr, o_ptr->ego->min_pval[j] -
						o_ptr->pval[i], flag);
}

/**
 * Apply minimum standards for ego-items.
 */
static void ego_apply_minima(object_type *o_ptr)
{
	if (!o_ptr->ego) return;

	if (o_ptr->ego->min_to_h != NO_MINIMUM &&
			o_ptr->to_h < o_ptr->ego->min_to_h)
		o_ptr->to_h = o_ptr->ego->min_to_h;
	if (o_ptr->ego->min_to_d != NO_MINIMUM &&
			o_ptr->to_d < o_ptr->ego->min_to_d)
		o_ptr->to_d = o_ptr->ego->min_to_d;
	if (o_ptr->ego->min_to_a != NO_MINIMUM &&
			o_ptr->to_a < o_ptr->ego->min_to_a)
		o_ptr->to_a = o_ptr->ego->min_to_a;

	ego_min_pvals(o_ptr);
}


/**
 * Try to find an ego-item for an object, setting o_ptr->ego if successful and
 * applying various bonuses.
 */
static void make_ego_item(object_type *o_ptr, int level)
{
	/* Cannot further improve artifacts or ego items */
	if (o_ptr->artifact || o_ptr->ego) return;

	/* Occasionally boost the generation level of an item */
	if (level > 0 && one_in_(GREAT_EGO))
		level = 1 + (level * MAX_DEPTH / randint1(MAX_DEPTH));

	/* Try to get a legal ego type for this item */
	o_ptr->ego = ego_find_random(o_ptr, level);

	/* Actually apply the ego template to the item */
	if (o_ptr->ego)
		ego_apply_magic(o_ptr, level);

	return;
}


/*** Make an artifact ***/

/**
 * Copy artifact data to a normal object, and set various slightly hacky
 * globals.
 */
void copy_artifact_data(object_type *o_ptr, const artifact_type *a_ptr)
{
	int i;

	/* Extract the data */
	for (i = 0; i < a_ptr->num_pvals; i++)
		if (a_ptr->pval[i]) {
			o_ptr->pval[i] = a_ptr->pval[i];
			of_copy(o_ptr->pval_flags[i], a_ptr->pval_flags[i]);
		}
	o_ptr->num_pvals = a_ptr->num_pvals;
	o_ptr->ac = a_ptr->ac;
	o_ptr->dd = a_ptr->dd;
	o_ptr->ds = a_ptr->ds;
	o_ptr->to_a = a_ptr->to_a;
	o_ptr->to_h = a_ptr->to_h;
	o_ptr->to_d = a_ptr->to_d;
	o_ptr->weight = a_ptr->weight;
	of_union(o_ptr->flags, a_ptr->flags);
}


/*
 * Mega-Hack -- Attempt to create one of the "Special Objects".
 *
 * We are only called from "make_object()"
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
	if (OPT(birth_no_artifacts)) return FALSE;

	/* No artifacts in the town */
	if (!p_ptr->depth) return FALSE;

	/* Check the special artifacts */
	for (i = 0; i < ART_MIN_NORMAL; ++i) {
		artifact_type *a_ptr = &a_info[i];

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->created) continue;

		/* Enforce minimum "depth" (loosely) */
		if (a_ptr->alloc_min > p_ptr->depth) {
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

		/* Make sure the kind was found */
		if (!kind) continue;

		/* Enforce minimum "object" level (loosely) */
		if (kind->level > level) {
			/* Get the "out-of-depth factor" */
			int d = (kind->level - level) * 5;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Assign the template */
		object_prep(o_ptr, kind, a_ptr->alloc_min, RANDOMISE);

		/* Mark the item as an artifact */
		o_ptr->artifact = a_ptr;

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
 * Attempt to change an object into an artifact.  If the object is already
 * set to be an artifact, use that, or otherwise use a suitable randomly-
 * selected artifact.
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(object_type *o_ptr)
{
	artifact_type *a_ptr;
	int i;
	bool art_ok = TRUE;

	/* Make sure birth no artifacts isn't set */
	if (OPT(birth_no_artifacts)) art_ok = FALSE;

	/* Special handling of Grond/Morgoth */
	if (o_ptr->artifact)
	{
		switch (o_ptr->artifact->aidx)
		{
			case ART_GROND:
			case ART_MORGOTH:
				art_ok = TRUE;
		}
	}

	if (!art_ok) return (FALSE);

	/* No artifacts in the town */
	if (!p_ptr->depth) return (FALSE);

	/* Paranoia -- no "plural" artifacts */
	if (o_ptr->number != 1) return (FALSE);

	/* Check the artifact list (skip the "specials") */
	for (i = ART_MIN_NORMAL; !o_ptr->artifact && i < z_info->a_max; i++) {
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
		o_ptr->artifact = a_ptr;
	}

	if (o_ptr->artifact) {
		copy_artifact_data(o_ptr, o_ptr->artifact);
		o_ptr->artifact->created = 1;
		return TRUE;
	}

	return FALSE;
}


/*** Apply magic to an item ***/

/*
 * Apply magic to a weapon.
 */
static void apply_magic_weapon(object_type *o_ptr, int level, int power)
{
	if (power <= 0)
		return;

	o_ptr->to_h += randint1(5) + m_bonus(5, level);
	o_ptr->to_d += randint1(5) + m_bonus(5, level);

	if (power > 1) {
		o_ptr->to_h += m_bonus(10, level);
		o_ptr->to_d += m_bonus(10, level);

		if (wield_slot(o_ptr) == INVEN_WIELD || obj_is_ammo(o_ptr)) {
			/* Super-charge the damage dice */
			while ((o_ptr->dd * o_ptr->ds > 0) &&
					one_in_(10L * o_ptr->dd * o_ptr->ds))
				o_ptr->dd++;

			/* But not too high */
			if (o_ptr->dd > 9) o_ptr->dd = 9;
		}
	}
}


/*
 * Apply magic to armour
 */
static void apply_magic_armour(object_type *o_ptr, int level, int power)
{
	if (power <= 0)
		return;

	o_ptr->to_a += randint1(5) + m_bonus(5, level);
	if (power > 1)
		o_ptr->to_a += m_bonus(10, level);
}


/**
 * Wipe an object clean and make it a standard object of the specified kind.
 */
void object_prep(object_type *o_ptr, struct object_kind *k, int lev,
		aspect rand_aspect)
{
	int i, flag, pval;
	bitflag flags[OF_SIZE], f2[OF_SIZE];

	/* Clean slate */
	WIPE(o_ptr, object_type);

	/* Assign the kind and copy across data */
	o_ptr->kind = k;
	o_ptr->tval = k->tval;
	o_ptr->sval = k->sval;
	o_ptr->ac = k->ac;
	o_ptr->dd = k->dd;
	o_ptr->ds = k->ds;
	o_ptr->weight = k->weight;

	/* Default number */
	o_ptr->number = 1;

	/* Apply pvals and then copy flags */
	of_copy(f2, k->flags);
    for (i = 0; i < k->num_pvals; i++) {
        of_copy(flags, k->pval_flags[i]);
        pval = randcalc(k->pval[i], lev, rand_aspect);
        for (flag = of_next(flags, FLAG_START); flag != FLAG_END;
                flag = of_next(flags, flag + 1))
			/* Prevent phantom flags */
			if (pval)
				object_add_pval(o_ptr, pval, flag);
			else
				of_off(f2, flag);
    }
	of_copy(o_ptr->flags, k->base->flags);
	of_union(o_ptr->flags, f2);

	/* Assign charges (wands/staves only) */
	if (o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF)
		o_ptr->pval[DEFAULT_PVAL] = randcalc(k->charge, lev, rand_aspect);

	/* Assign flagless pval for food or oil */
	if (o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION ||
			o_ptr->tval == TV_FLASK)
		o_ptr->pval[DEFAULT_PVAL]
			= randcalc(k->pval[DEFAULT_PVAL], lev, rand_aspect);

	/* Default fuel for lamps */
	if (o_ptr->tval == TV_LIGHT) {
		if (o_ptr->sval == SV_LIGHT_TORCH)
			o_ptr->timeout = DEFAULT_TORCH;
		else if (o_ptr->sval == SV_LIGHT_LANTERN)
			o_ptr->timeout = DEFAULT_LAMP;
	}

	/* Default magic */
	o_ptr->to_h = randcalc(k->to_h, lev, rand_aspect);
	o_ptr->to_d = randcalc(k->to_d, lev, rand_aspect);
	o_ptr->to_a = randcalc(k->to_a, lev, rand_aspect);
}


/**
 * Applying magic to an object, which includes creating ego-items, and applying
 * random bonuses,
 *
 * The `good` argument forces the item to be at least `good`, and the `great`
 * argument does likewise.  Setting `allow_artifacts` to TRUE allows artifacts
 * to be created here.
 *
 * If `good` or `great` are not set, then the `lev` argument controls the
 * quality of item.
 *
 * Returns 0 if a normal object, 1 if a good object, 2 if an ego item, 3 if an
 * artifact.
 */
s16b apply_magic(object_type *o_ptr, int lev, bool allow_artifacts,
		bool good, bool great, bool extra_roll)
{
	int i;
	s16b power = 0;

	/* Chance of being `good` and `great` */
	/* This has changed over the years:
	 * 3.0.0:   good = MIN(75, lev + 10);      great = MIN(20, lev / 2); 
	 * 3.3.0:	good = (lev + 2) * 3;          great = MIN(lev / 4 + lev, 50);
     * 3.4.0:   good = (2 * lev) + 5
     * 3.4 was in between 3.0 and 3.3, 3.5 attempts to keep the same
     * area under the curve as 3.4, but make the generation chances
     * flatter.  This depresses good items overall since more items
     * are created deeper. 
     * This change is meant to go in conjunction with the changes
     * to ego item allocation levels. (-fizzix)
	 */
	int good_chance = (33 + lev);
	int great_chance = 30;

	/* Roll for "good" */
	if (good || (randint0(100) < good_chance)) {
		power = 1;

		/* Roll for "great" */
		if (great || (randint0(100) < great_chance))
			power = 2;
	}

	/* Roll for artifact creation */
	if (allow_artifacts) {
		int rolls = 0;

		/* Get one roll if excellent */
		if (power >= 2) rolls = 1;

		/* Get two rolls if forced great */
		if (great) rolls = 2;
        
        /* Give some extra rolls for uniques and acq scrolls */
        if (extra_roll) rolls += 2;

		/* Roll for artifacts if allowed */
		for (i = 0; i < rolls; i++)
			if (make_artifact(o_ptr)) return 3;
	}

	/* Try to make an ego item */
	if (power == 2)
		make_ego_item(o_ptr, lev);

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
			apply_magic_weapon(o_ptr, lev, power);
			break;

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_HELM:
		case TV_CROWN:
		case TV_CLOAK:
		case TV_GLOVES:
		case TV_BOOTS:
			apply_magic_armour(o_ptr, lev, power);
			break;

		case TV_RING:
			if (o_ptr->sval == SV_RING_SPEED) {
				/* Super-charge the ring */
				while (one_in_(2))
					o_ptr->pval[which_pval(o_ptr, OF_SPEED)]++;
			}
			break;

		case TV_CHEST:
			/* Hack -- skip ruined chests */
			if (o_ptr->kind->level <= 0) break;

			/* Hack -- pick a "difficulty" */
			o_ptr->pval[DEFAULT_PVAL] = randint1(o_ptr->kind->level);

			/* Never exceed "difficulty" of 55 to 59 */
			if (o_ptr->pval[DEFAULT_PVAL] > 55)
				o_ptr->pval[DEFAULT_PVAL] = (s16b)(55 + randint0(5));

			break;
	}

	/* Apply minima from ego items if necessary */
	ego_apply_minima(o_ptr);

	return power;
}


/*** Generate a random object ***/

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


/** Arrays holding an index of objects to generate for a given level */
static u32b obj_total[MAX_DEPTH];
static byte *obj_alloc;

static u32b obj_total_great[MAX_DEPTH];
static byte *obj_alloc_great;

/* Don't worry about probabilities for anything past dlev100 */
#define MAX_O_DEPTH		100

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
 * Free object allocation info.
 */
void free_obj_alloc(void)
{
	FREE(obj_alloc);
	FREE(obj_alloc_great);
}


/*
 * Choose an object kind of a given tval given a dungeon level.
 */
static object_kind *get_obj_num_by_kind(int level, bool good, int tval)
{
	/* This is the base index into obj_alloc for this dlev */
	size_t ind, item;
	u32b value;
	int total = 0;
	byte *objects = good ? obj_alloc_great : obj_alloc;

	/* Pick an object */
	ind = level * z_info->k_max;

	/* Get new total */
	for (item = 1; item < z_info->k_max; item++) {
		if (objkind_byid(item)->tval == tval) {
			total += objects[ind + item];
		}
	}

	/* No appropriate items of that tval */
	if (!total) return NULL;
	
	value = randint0(total);
	
	for (item = 1; item < z_info->k_max; item++) {
		if (objkind_byid(item)->tval == tval) {
			if (value < objects[ind + item]) break;

			value -= objects[ind + item];
		}
	}

	/* Return the item index */
	return objkind_byid(item);
}

/*
 * Choose an object kind given a dungeon level to choose it for.
 * If tval = 0, we can choose an object of any type.
 * Otherwise we can only choose one of the given tval.
 */
object_kind *get_obj_num(int level, bool good, int tval)
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
	
	if(tval)
		return get_obj_num_by_kind(level, good, tval);
	

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


/**
 * Attempt to make an object
 *
 * \param c is the current dungeon level.
 * \param j_ptr is the object struct to be populated.
 * \param lev is the creation level of the object (not necessarily == depth).
 * \param good is whether the object is to be good
 * \param great is whether the object is to be great
 * \param value is the value to be returned to the calling function
 * \param tval is the desired tval, or 0 if we allow any tval
 *
 * Returns the whether or not creation worked.
 */
bool make_object(struct cave *c, object_type *j_ptr, int lev, bool good,
	bool great, bool extra_roll, s32b *value, int tval)
{
	int base;
	object_kind *kind;

	/* Try to make a special artifact */
	if (one_in_(good ? 10 : 1000)) {
		if (make_artifact_special(j_ptr, lev)) {
			if (value) *value = object_value_real(j_ptr, 1, FALSE, TRUE);
			return TRUE;
		}

		/* If we failed to make an artifact, the player gets a good item */
		good = TRUE;
	}

	/* Base level for the object */
	base = (good ? (lev + 10) : lev);

	/* Get the object, prep it and apply magic */
	kind = get_obj_num(base, good || great, tval);
	if (!kind) return FALSE;
	object_prep(j_ptr, kind, lev, RANDOMISE);
	apply_magic(j_ptr, lev, TRUE, good, great, extra_roll);

	/* Generate multiple items */
	if (kind->gen_mult_prob >= randint1(100))
		j_ptr->number = randcalc(kind->stack_size, lev, RANDOMISE);

	if (j_ptr->number >= MAX_STACK_SIZE)
		j_ptr->number = MAX_STACK_SIZE - 1;

	/* Return value, increased for uncursed out-of-depth objects */
	if (value)
		*value = object_value_real(j_ptr, j_ptr->number, FALSE, TRUE);

	if (!cursed_p(j_ptr->flags) && (kind->alloc_min > c->depth)) {
		if (value) *value = (kind->alloc_min - c->depth) * (*value / 5);
	}

	return TRUE;
}


/*** Make a gold item ***/

/* The largest possible average gold drop at max depth with biggest spread */
#define MAX_GOLD_DROP     (3 * MAX_DEPTH + 30)

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
	if (OPT(birth_no_selling) && p_ptr->depth)
		value = value * MIN(5, p_ptr->depth);

	/* Cap gold at max short (or alternatively make pvals s32b) */
	if (value > MAX_SHORT)
		value = MAX_SHORT;

	j_ptr->pval[DEFAULT_PVAL] = value;
}

struct init_module obj_make_module = {
	.name = "object/obj-make",
	.init = init_ego_allocs,
	.cleanup = cleanup_ego_allocs
};
