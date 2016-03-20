/**
 * \file obj-make.c
 * \brief Object generation functions.
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
#include "alloc.h"
#include "cave.h"
#include "effects.h"
#include "init.h"
#include "obj-gear.h"
#include "obj-pile.h"
#include "obj-make.h"
#include "obj-power.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"

/** Arrays holding an index of objects to generate for a given level */
static u32b *obj_total;
static byte *obj_alloc;

static u32b *obj_total_great;
static byte *obj_alloc_great;

static s16b alloc_ego_size = 0;
static alloc_entry *alloc_ego_table;

struct money {
	char *name;
	int type;
};

static struct money *money_type;
static int num_money_types;

static void init_obj_make(void) {
	int i, item, lev;
	int k_max = z_info->k_max;
	struct alloc_entry *table;
	struct ego_item *ego;
	s16b *num;
	s16b *aux;
	int *money_svals;

	/*** Initialize object allocation info ***/

	/* Allocate and wipe */
	obj_alloc = mem_zalloc((z_info->max_obj_depth + 1) * k_max * sizeof(byte));
	obj_alloc_great = mem_zalloc((z_info->max_obj_depth + 1) * k_max * sizeof(byte));
	obj_total = mem_zalloc((z_info->max_obj_depth + 1) * sizeof(u32b));
	obj_total_great = mem_zalloc((z_info->max_obj_depth + 1) * sizeof(u32b));

	/* Init allocation data */
	for (item = 1; item < k_max; item++) {
		const struct object_kind *kind = &k_info[item];

		int min = kind->alloc_min;
		int max = kind->alloc_max;

		/* If an item doesn't have a rarity, move on */
		if (!kind->alloc_prob) continue;

		/* Go through all the dungeon levels */
		for (lev = 0; lev <= z_info->max_obj_depth; lev++) {
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

	/*** Initialize ego-item allocation info ***/

	num = mem_zalloc((z_info->max_obj_depth + 1) * sizeof(s16b));
	aux = mem_zalloc((z_info->max_obj_depth + 1) * sizeof(s16b));

	/* Scan the ego items */
	for (i = 1; i < z_info->e_max; i++) {
		/* Get the i'th ego item */
		ego = &e_info[i];

		/* Legal items */
		if (ego->rarity) {
			/* Count the entries */
			alloc_ego_size++;

			/* Group by level */
			num[ego->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < z_info->max_obj_depth; i++)
		num[i] += num[i - 1];

	/* Allocate the alloc_ego_table */
	alloc_ego_table = mem_zalloc(alloc_ego_size * sizeof(alloc_entry));

	/* Get the table entry */
	table = alloc_ego_table;

	/* Scan the ego-items */
	for (i = 1; i < z_info->e_max; i++) {
		/* Get the i'th ego item */
		ego = &e_info[i];

		/* Count valid pairs */
		if (ego->rarity) {
			int p, x, y, z;

			/* Extract the base level */
			x = ego->level;

			/* Extract the base probability */
			p = (100 / ego->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x - 1] : 0;

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
	mem_free(aux);
	mem_free(num);

	/*** Initialize money info ***/

	/* Count the money types and make a list */
	num_money_types = tval_sval_count("gold");
	money_type = mem_zalloc(num_money_types * sizeof(struct money));
	money_svals = mem_zalloc(num_money_types * sizeof(struct money));
	tval_sval_list("gold", money_svals, num_money_types);

	/* List the money types */
	for (i = 0; i < num_money_types; i++) {
		struct object_kind *kind = lookup_kind(TV_GOLD, money_svals[i]);
		money_type[i].name = string_make(kind->name);
		money_type[i].type = money_svals[i];
	}
	mem_free(money_svals);
}

static void cleanup_obj_make(void) {
	int i;
	for (i = 0; i < num_money_types; i++) {
		string_free(money_type[i].name);
	}
	mem_free(money_type);
	mem_free(alloc_ego_table);
	mem_free(obj_total_great);
	mem_free(obj_total);
	mem_free(obj_alloc_great);
	mem_free(obj_alloc);
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
 * Get a random new base resist on an item
 */
static int random_base_resist(struct object *obj, int *resist)
{
	int i, r, count = 0;

	/* Count the available base resists */
	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++)
		if (obj->el_info[i].res_level == 0) count++;

	if (count == 0) return false;

	/* Pick one */
	r = randint0(count);

	/* Find the one we picked */
	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++) {
		if (obj->el_info[i].res_level != 0) continue;
		if (r == 0) {
			*resist = i;
			return true;
		}
		r--;
	}

	return false;
}

/**
 * Get a random new high resist on an item
 */
static int random_high_resist(struct object *obj, int *resist)
{
	int i, r, count = 0;

	/* Count the available high resists */
	for (i = ELEM_HIGH_MIN; i <= ELEM_HIGH_MAX; i++)
		if (obj->el_info[i].res_level == 0) count++;

	if (count == 0) return false;

	/* Pick one */
	r = randint0(count);

	/* Find the one we picked */
	for (i = ELEM_HIGH_MIN; i <= ELEM_HIGH_MAX; i++) {
		if (obj->el_info[i].res_level != 0) continue;
		if (r == 0) {
			*resist = i;
			return true;
		}
		r--;
	}

	return false;
}


/**
 * Select an ego-item that fits the object's tval and sval.
 */
static struct ego_item *ego_find_random(struct object *obj, int level)
{
	int i, ood_chance;
	long total = 0L;

	alloc_entry *table = alloc_ego_table;
	struct ego_item *ego;
	struct ego_poss_item *poss;

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

		for (poss = ego->poss_items; poss; poss = poss->next)
			if (poss->kidx == obj->kind->kidx) {
				table[i].prob3 = table[i].prob2;
				break;
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
void ego_apply_magic(struct object *obj, int level)
{
	int i, x, resist = 0, pick = 0;
	bitflag newf[OF_SIZE];

	/* Resist or power? */
	if (kf_has(obj->ego->kind_flags, KF_RAND_RES_POWER))
		pick = randint1(3);

	/* Extra powers */
	if (kf_has(obj->ego->kind_flags, KF_RAND_SUSTAIN)) {
		create_mask(newf, false, OFT_SUST, OFT_MAX);
		of_on(obj->flags, get_new_attr(obj->flags, newf));
	} else if (kf_has(obj->ego->kind_flags, KF_RAND_POWER) || (pick == 1)) {
		create_mask(newf, false, OFT_PROT, OFT_MISC, OFT_MAX);
		of_on(obj->flags, get_new_attr(obj->flags, newf));
	} else if (kf_has(obj->ego->kind_flags, KF_RAND_BASE_RES) || (pick > 1)) {
		/* Get a base resist if available, mark it as random */
		if (random_base_resist(obj, &resist)) {
			obj->el_info[resist].res_level = 1;
			obj->el_info[resist].flags |= EL_INFO_RANDOM;
		}
	} else if (kf_has(obj->ego->kind_flags, KF_RAND_HI_RES)) {
		/* Get a high resist if available, mark it as random */
		if (random_high_resist(obj, &resist)) {
			obj->el_info[resist].res_level = 1;
			obj->el_info[resist].flags |= EL_INFO_RANDOM;
		}
	}

	/* Apply extra obj->ego bonuses */
	obj->to_h += randcalc(obj->ego->to_h, level, RANDOMISE);
	obj->to_d += randcalc(obj->ego->to_d, level, RANDOMISE);
	obj->to_a += randcalc(obj->ego->to_a, level, RANDOMISE);

	/* Apply modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		x = randcalc(obj->ego->modifiers[i], level, RANDOMISE);
		obj->modifiers[i] += x;
	}

	/* Apply flags */
	of_union(obj->flags, obj->ego->flags);
	of_diff(obj->flags, obj->ego->flags_off);

	/* Add slays and brands */
	copy_slay(&obj->slays, obj->ego->slays);
	copy_brand(&obj->brands, obj->ego->brands);

	/* Add resists */
	for (i = 0; i < ELEM_MAX; i++) {
		/* Take the larger of ego and base object resist levels */
		obj->el_info[i].res_level =
			MAX(obj->ego->el_info[i].res_level, obj->el_info[i].res_level);

		/* Union of flags so as to know when ignoring is notable */
		obj->el_info[i].flags |= obj->ego->el_info[i].flags;
	}

	/* Add effect (ego effect will trump object effect, when there are any) */
	if (obj->ego->effect) {
		obj->effect = obj->ego->effect;
		obj->time = obj->ego->time;
	}

	return;
}

/**
 * Apply minimum standards for ego-items.
 */
static void ego_apply_minima(struct object *obj)
{
	int i;

	if (!obj->ego) return;

	if (obj->ego->min_to_h != NO_MINIMUM &&
			obj->to_h < obj->ego->min_to_h)
		obj->to_h = obj->ego->min_to_h;
	if (obj->ego->min_to_d != NO_MINIMUM &&
			obj->to_d < obj->ego->min_to_d)
		obj->to_d = obj->ego->min_to_d;
	if (obj->ego->min_to_a != NO_MINIMUM &&
			obj->to_a < obj->ego->min_to_a)
		obj->to_a = obj->ego->min_to_a;

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (obj->modifiers[i] < obj->ego->min_modifiers[i])
			obj->modifiers[i] = obj->ego->min_modifiers[i];
	}
}


/**
 * Try to find an ego-item for an object, setting obj->ego if successful and
 * applying various bonuses.
 */
static void make_ego_item(struct object *obj, int level)
{
	/* Cannot further improve artifacts or ego items */
	if (obj->artifact || obj->ego) return;

	/* Occasionally boost the generation level of an item */
	if (level > 0 && one_in_(z_info->great_ego)) {
		level = 1 + (level * z_info->max_depth / randint1(z_info->max_depth));

		/* Ensure valid allocation level */
		if (level >= z_info->max_depth)
			level = z_info->max_depth - 1;
	}

	/* Try to get a legal ego type for this item */
	obj->ego = ego_find_random(obj, level);

	/* Actually apply the ego template to the item */
	if (obj->ego)
		ego_apply_magic(obj, level);

	return;
}


/*** Make an artifact ***/

/**
 * Copy artifact data to a normal object, and set various slightly hacky
 * globals.
 */
void copy_artifact_data(struct object *obj, const struct artifact *art)
{
	int i;

	/* Extract the data */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		obj->modifiers[i] = art->modifiers[i];
	obj->ac = art->ac;
	obj->dd = art->dd;
	obj->ds = art->ds;
	obj->to_a = art->to_a;
	obj->to_h = art->to_h;
	obj->to_d = art->to_d;
	obj->weight = art->weight;
	obj->activation = art->activation;
	if (art->time.base != 0)
		obj->time = art->time;
	of_union(obj->flags, art->flags);
	copy_slay(&obj->slays, art->slays);
	copy_brand(&obj->brands, art->brands);
	for (i = 0; i < ELEM_MAX; i++) {
		/* Take the larger of artifact and base object resist levels */
		obj->el_info[i].res_level =
			MAX(art->el_info[i].res_level, obj->el_info[i].res_level);

		/* Union of flags so as to know when ignoring is notable */
		obj->el_info[i].flags |= art->el_info[i].flags;
	}
}


/**
 * Mega-Hack -- Attempt to create one of the "Special Objects".
 *
 * We are only called from "make_object()"
 *
 * Note -- see "make_artifact()" and "apply_magic()".
 *
 * We *prefer* to create the special artifacts in order, but this is
 * normally outweighed by the "rarity" rolls for those artifacts.
 */
static struct object *make_artifact_special(int level)
{
	int i;
	struct object *new_obj;

	/* No artifacts, do nothing */
	if (OPT(birth_no_artifacts))
		return NULL;

	/* No artifacts in the town */
	if (!player->depth)
		return NULL;

	/* Check the special artifacts */
	for (i = 0; i < z_info->a_max; ++i) {
		struct artifact *art = &a_info[i];
		struct object_kind *kind = lookup_kind(art->tval, art->sval);

		/* Skip "empty" artifacts */
		if (!art->name) continue;

		/* Make sure the kind was found */
		if (!kind) continue;

		/* Skip non-special artifacts */
		if (!kf_has(kind->kind_flags, KF_INSTA_ART)) continue;

		/* Cannot make an artifact twice */
		if (art->created) continue;

		/* Enforce minimum "depth" (loosely) */
		if (art->alloc_min > player->depth) {
			/* Get the "out-of-depth factor" */
			int d = (art->alloc_min - player->depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Enforce maximum depth (strictly) */
		if (art->alloc_max < player->depth) continue;

		/* Artifact "rarity roll" */
		if (randint1(100) > art->alloc_prob) continue;

		/* Enforce minimum "object" level (loosely) */
		if (kind->level > level) {
			/* Get the "out-of-depth factor" */
			int d = (kind->level - level) * 5;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Assign the template */
		new_obj = object_new();
		object_prep(new_obj, kind, art->alloc_min, RANDOMISE);

		/* Mark the item as an artifact */
		new_obj->artifact = art;

		/* Copy across all the data from the artifact struct */
		copy_artifact_data(new_obj, art);

		/* Mark the artifact as "created" */
		art->created = true;

		/* Success */
		return new_obj;
	}

	/* Failure */
	return NULL;
}


/**
 * Attempt to change an object into an artifact.  If the object is already
 * set to be an artifact, use that, or otherwise use a suitable randomly-
 * selected artifact.
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(struct object *obj)
{
	int i;
	bool art_ok = true;

	/* Make sure birth no artifacts isn't set */
	if (OPT(birth_no_artifacts)) art_ok = false;

	/* Special handling of quest artifacts */
	if (kf_has(obj->kind->kind_flags, KF_QUEST_ART))
		art_ok = true;

	if (!art_ok) return (false);

	/* No artifacts in the town */
	if (!player->depth) return (false);

	/* Paranoia -- no "plural" artifacts */
	if (obj->number != 1) return (false);

	/* Check the artifact list (skip the "specials") */
	for (i = 0; !obj->artifact && i < z_info->a_max; i++) {
		struct artifact *art = &a_info[i];
		struct object_kind *kind = lookup_kind(art->tval, art->sval);

		/* Skip "empty" items */
		if (!art->name) continue;

		/* Make sure the kind was found */
		if (!kind) continue;

		/* Skip special artifacts */
		if (kf_has(kind->kind_flags, KF_INSTA_ART)) continue;

		/* Cannot make an artifact twice */
		if (art->created) continue;

		/* Must have the correct fields */
		if (art->tval != obj->tval) continue;
		if (art->sval != obj->sval) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (art->alloc_min > player->depth)
		{
			/* Get the "out-of-depth factor" */
			int d = (art->alloc_min - player->depth) * 2;

			/* Roll for out-of-depth creation */
			if (randint0(d) != 0) continue;
		}

		/* Enforce maximum depth (strictly) */
		if (art->alloc_max < player->depth) continue;

		/* We must make the "rarity roll" */
		if (randint1(100) > art->alloc_prob) continue;

		/* Mark the item as an artifact */
		obj->artifact = art;
	}

	if (obj->artifact) {
		copy_artifact_data(obj, obj->artifact);
		obj->artifact->created = true;
		return true;
	}

	return false;
}


/**
 * Create a fake artifact directly from a blank object
 *
 * This function is used for describing artifacts, and for creating them for
 * debugging.
 *
 * Since this is now in no way marked as fake, we must make sure this function
 * is never used to create an actual game object
 */
bool make_fake_artifact(struct object *obj, struct artifact *artifact)
{
	struct object_kind *kind;

	/* Don't bother with empty artifacts */
	if (!artifact->tval) return false;

	/* Get the "kind" index */
	kind = lookup_kind(artifact->tval, artifact->sval);
	if (!kind) return false;

	/* Create the artifact */
	object_prep(obj, kind, 0, MAXIMISE);

	/* Save the name */
	obj->artifact = artifact;

	/* Extract the fields */
	copy_artifact_data(obj, artifact);

	/* Success */
	return (true);
}


/*** Apply magic to an item ***/

/**
 * Apply magic to a weapon.
 */
static void apply_magic_weapon(struct object *obj, int level, int power)
{
	if (power <= 0)
		return;

	obj->to_h += randint1(5) + m_bonus(5, level);
	obj->to_d += randint1(5) + m_bonus(5, level);

	if (power > 1) {
		obj->to_h += m_bonus(10, level);
		obj->to_d += m_bonus(10, level);

		if (tval_is_melee_weapon(obj) || tval_is_ammo(obj)) {
			/* Super-charge the damage dice */
			while ((obj->dd * obj->ds > 0) &&
					one_in_(10L * obj->dd * obj->ds))
				obj->dd++;

			/* But not too high */
			if (obj->dd > 9) obj->dd = 9;
		}
	}
}


/**
 * Apply magic to armour
 */
static void apply_magic_armour(struct object *obj, int level, int power)
{
	if (power <= 0)
		return;

	obj->to_a += randint1(5) + m_bonus(5, level);
	if (power > 1)
		obj->to_a += m_bonus(10, level);
}


/**
 * Wipe an object clean and make it a standard object of the specified kind.
 */
void object_prep(struct object *obj, struct object_kind *k, int lev,
				 aspect rand_aspect)
{
	int i;

	/* Clean slate */
	memset(obj, 0, sizeof(*obj));

	/* Assign the kind and copy across data */
	obj->kind = k;
	obj->tval = k->tval;
	obj->sval = k->sval;
	obj->ac = k->ac;
	obj->dd = k->dd;
	obj->ds = k->ds;
	obj->weight = k->weight;
	obj->effect = k->effect;
	obj->time = k->time;

	/* Default number */
	obj->number = 1;

	/* Copy flags */
	of_copy(obj->flags, k->base->flags);
	of_copy(obj->flags, k->flags);

	/* Assign modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		obj->modifiers[i] = randcalc(k->modifiers[i], lev, rand_aspect);

	/* Assign charges (wands/staves only) */
	if (tval_can_have_charges(obj))
		obj->pval = randcalc(k->charge, lev, rand_aspect);

	/* Assign pval for food, oil and launchers */
	if (tval_is_edible(obj) || tval_is_potion(obj) || tval_is_fuel(obj) ||
		tval_is_launcher(obj))
		obj->pval
			= randcalc(k->pval, lev, rand_aspect);

	/* Default fuel */
	if (tval_is_light(obj)) {
		if (of_has(obj->flags, OF_BURNS_OUT))
			obj->timeout = z_info->fuel_torch;
		else if (of_has(obj->flags, OF_TAKES_FUEL))
			obj->timeout = z_info->default_lamp;
	}

	/* Default magic */
	obj->to_h = randcalc(k->to_h, lev, rand_aspect);
	obj->to_d = randcalc(k->to_d, lev, rand_aspect);
	obj->to_a = randcalc(k->to_a, lev, rand_aspect);

	/* Default slays and brands */
	copy_slay(&obj->slays, k->slays);
	copy_brand(&obj->brands, k->brands);

	/* Default resists */
	for (i = 0; i < ELEM_MAX; i++) {
		obj->el_info[i].res_level = k->el_info[i].res_level;
		obj->el_info[i].flags = k->el_info[i].flags;
		obj->el_info[i].flags |= k->base->el_info[i].flags;
	}
}


/**
 * Applying magic to an object, which includes creating ego-items, and applying
 * random bonuses,
 *
 * The `good` argument forces the item to be at least `good`, and the `great`
 * argument does likewise.  Setting `allow_artifacts` to true allows artifacts
 * to be created here.
 *
 * If `good` or `great` are not set, then the `lev` argument controls the
 * quality of item.
 *
 * Returns 0 if a normal object, 1 if a good object, 2 if an ego item, 3 if an
 * artifact.
 */
int apply_magic(struct object *obj, int lev, bool allow_artifacts, bool good,
				bool great, bool extra_roll)
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
			if (make_artifact(obj)) return 3;
	}

	/* Try to make an ego item */
	if (power == 2)
		make_ego_item(obj, lev);

	/* Apply magic */
	if (tval_is_weapon(obj)) {
		apply_magic_weapon(obj, lev, power);
	}
	else if (tval_is_armor(obj)) {
		apply_magic_armour(obj, lev, power);
	}
	else if (tval_is_ring(obj)) {
		if (obj->sval == lookup_sval(obj->tval, "Speed")) {
			/* Super-charge the ring */
			while (one_in_(2))
				obj->modifiers[OBJ_MOD_SPEED]++;
		}
	}
	else if (tval_is_chest(obj)) {
		/* Hack -- skip ruined chests */
		if (obj->kind->level > 0) {
			/* Hack -- pick a "difficulty" */
			obj->pval = randint1(obj->kind->level);

			/* Never exceed "difficulty" of 55 to 59 */
			if (obj->pval > 55)
				obj->pval = (s16b)(55 + randint0(5));
		}
	}

	/* Apply minima from ego items if necessary */
	ego_apply_minima(obj);

	return power;
}


/*** Generate a random object ***/

/**
 * Hack -- determine if a template is "good".
 *
 * Note that this test only applies to the object *kind*, so it is
 * possible to choose a kind which is "good", and then later cause
 * the actual object to be cursed.  We do explicitly forbid objects
 * which are known to be boring or which start out somewhat damaged.
 */
bool kind_is_good(const struct object_kind *kind)
{
	/* Some item types are (almost) always good */
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
			if (randcalc(kind->to_a, 0, MINIMISE) < 0) return (false);
			return true;
		}

		/* Weapons -- Good unless damaged */
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		{
			if (randcalc(kind->to_h, 0, MINIMISE) < 0) return (false);
			if (randcalc(kind->to_d, 0, MINIMISE) < 0) return (false);
			return true;
		}

		/* Ammo -- Arrows/Bolts are good */
		case TV_BOLT:
		case TV_ARROW:
		{
			return true;
		}
	}

	/* Anything with the GOOD flag */
	if (kf_has(kind->kind_flags, KF_GOOD))
		return true;

	/* Assume not good */
	return (false);
}


/**
 * Choose an object kind of a given tval given a dungeon level.
 */
static struct object_kind *get_obj_num_by_kind(int level, bool good, int tval)
{
	/* This is the base index into obj_alloc for this dlev */
	size_t ind, item;
	u32b value;
	int total = 0;
	byte *objects = good ? obj_alloc_great : obj_alloc;

	/* Pick an object */
	ind = level * z_info->k_max;

	/* Get new total */
	for (item = 1; item < z_info->k_max; item++)
		if (objkind_byid(item)->tval == tval)
			total += objects[ind + item];

	/* No appropriate items of that tval */
	if (!total) return NULL;
	
	value = randint0(total);
	
	for (item = 1; item < z_info->k_max; item++)
		if (objkind_byid(item)->tval == tval) {
			if (value < objects[ind + item]) break;

			value -= objects[ind + item];
		}

	/* Return the item index */
	return objkind_byid(item);
}

/**
 * Choose an object kind given a dungeon level to choose it for.
 * If tval = 0, we can choose an object of any type.
 * Otherwise we can only choose one of the given tval.
 */
struct object_kind *get_obj_num(int level, bool good, int tval)
{
	/* This is the base index into obj_alloc for this dlev */
	size_t ind, item;
	u32b value;

	/* Occasional level boost */
	if ((level > 0) && one_in_(z_info->great_obj))
		/* What a bizarre calculation */
		level = 1 + (level * z_info->max_obj_depth / randint1(z_info->max_obj_depth));

	/* Paranoia */
	level = MIN(level, z_info->max_obj_depth);
	level = MAX(level, 0);

	/* Pick an object */
	ind = level * z_info->k_max;
	
	if (tval)
		return get_obj_num_by_kind(level, good, tval);
	
	if (!good) {
		value = randint0(obj_total[level]);
		for (item = 1; item < z_info->k_max; item++) {
			/* Found it */
			if (value < obj_alloc[ind + item]) break;

			/* Decrement */
			value -= obj_alloc[ind + item];
		}
	} else {
		value = randint0(obj_total_great[level]);
		for (item = 1; item < z_info->k_max; item++) {
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
 * \param lev is the creation level of the object (not necessarily == depth).
 * \param good is whether the object is to be good
 * \param great is whether the object is to be great
 * \param extra_roll is whether we get an extra roll in apply_magic()
 * \param value is the value to be returned to the calling function
 * \param tval is the desired tval, or 0 if we allow any tval
 *
 * \return a pointer to the newly allocated object, or NULL on failure.
 */
struct object *make_object(struct chunk *c, int lev, bool good, bool great,
						   bool extra_roll, s32b *value, int tval)
{
	int base;
	struct object_kind *kind;
	struct object *new_obj;

	/* Try to make a special artifact */
	if (one_in_(good ? 10 : 1000)) {
		new_obj = make_artifact_special(lev);
		if (new_obj) {
			if (value) *value = object_value_real(new_obj, 1, false);
			return new_obj;
		}

		/* If we failed to make an artifact, the player gets a good item */
		good = true;
	}

	/* Base level for the object */
	base = (good ? (lev + 10) : lev);

	/* Try to choose an object kind */
	kind = get_obj_num(base, good || great, tval);
	if (!kind)
		return NULL;

	/* Make the object, prep it and apply magic */
	new_obj = object_new();
	object_prep(new_obj, kind, lev, RANDOMISE);
	apply_magic(new_obj, lev, true, good, great, extra_roll);

	/* Generate multiple items */
	if (kind->gen_mult_prob >= randint1(100))
		new_obj->number = randcalc(kind->stack_size, lev, RANDOMISE);

	if (new_obj->number > z_info->stack_size)
		new_obj->number = z_info->stack_size;

	/* Get the value */
	if (value)
		*value = object_value_real(new_obj, new_obj->number, false);

	/* Boost of 20% per level OOD for uncursed objects */
	if (!cursed_p(new_obj->flags) && (kind->alloc_min > c->depth)) {
		if (value) *value += (kind->alloc_min - c->depth) * (*value / 5);
	}

	return new_obj;
}


/**
 * Scatter some objects near the player
 */
void acquirement(int y1, int x1, int level, int num, bool great)
{
	struct object *nice_obj;

	/* Acquirement */
	while (num--) {
		/* Make a good (or great) object (if possible) */
		nice_obj = make_object(cave, level, true, great, true, NULL, 0);
		if (!nice_obj) continue;

		nice_obj->origin = ORIGIN_ACQUIRE;
		nice_obj->origin_depth = player->depth;

		/* Drop the object */
		drop_near(cave, nice_obj, 0, y1, x1, true);
	}
}


/*** Make a gold item ***/

/**
 * Get a money kind by name, or level-appropriate 
 */
struct object_kind *money_kind(const char *name, int value)
{
	int rank;
	/* (Roughly) the largest possible gold drop at max depth - the precise
	 * value is derivable from the calculations in make_gold(), but this is
	 * near enough */
	int max_gold_drop = 3 * z_info->max_depth + 30;

	/* Check for specified treasure variety */
	for (rank = 0; rank < num_money_types; rank++)
		if (streq(name, money_type[rank].name))
			break;

	/* Pick a treasure variety scaled by level */
	if (rank == num_money_types)
		rank = (((value * 100) / max_gold_drop) * num_money_types) / 100;

	/* Do not create illegal treasure types */
	if (rank >= num_money_types) rank = num_money_types - 1;

	return lookup_kind(TV_GOLD, money_type[rank].type);
}

/**
 * Make a money object
 *
 * \param lev the dungeon level
 * \param coin_type the name of the type of money object to make
 * \return a pointer to the newly minted cash (cannot fail)
 */
struct object *make_gold(int lev, char *coin_type)
{
	/* This average is 20 at dlev0, 100 at dlev40, 220 at dlev100. */
	s32b avg = (18 * lev)/10 + 18;
	s32b spread = lev + 10;
	s32b value = rand_spread(avg, spread);
	struct object *new_gold = mem_zalloc(sizeof(*new_gold)); 

	/* Increase the range to infinite, moving the average to 110% */
	while (one_in_(100) && value * 10 <= SHRT_MAX)
		value *= 10;

	/* Prepare a gold object */
	object_prep(new_gold, money_kind(coin_type, value), lev, RANDOMISE);

	/* If we're playing with no_selling, increase the value */
	if (OPT(birth_no_selling) && player->depth)
		value = value * MIN(5, player->depth);

	/* Cap gold at max short (or alternatively make pvals s32b) */
	if (value > SHRT_MAX)
		value = SHRT_MAX;

	new_gold->pval = value;

	return new_gold;
}

struct init_module obj_make_module = {
	.name = "object/obj-make",
	.init = init_obj_make,
	.cleanup = cleanup_obj_make
};
