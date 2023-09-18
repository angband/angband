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
#include "obj-chest.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"

/**
 * Stores cumulative probability distribution for objects at each level.  The
 * value at ilv * (z_info->k_max + 1) + itm is the probablity, out of
 * obj_alloc[ilv * (z_info->k_max + 1) + z_info->k_max], that an item whose
 * index is less than itm occurs at level, ilv.
 */
static uint32_t *obj_alloc;

/**
 * Has the same layout and interpretation as obj_alloc, but only items that
 * are good or better contribute to the cumulative probability distribution.
 */
static uint32_t *obj_alloc_great;

/**
 * Store the total allocation value for each tval by level.  The value at
 * ilv * TV_MAX + tval is the total for tval at the level, ilv.
 */
static uint32_t *obj_total_tval;

/**
 * Same layout and interpretation as obj_total_tval, but only items that are
 * good or better contribute.
 */
static uint32_t *obj_total_tval_great;

static int16_t alloc_ego_size = 0;
static alloc_entry *alloc_ego_table;

struct money {
	char *name;
	int type;
};

static struct money *money_type;
static int num_money_types;

/*
 * Initialize object allocation info
 */
static void alloc_init_objects(void) {
	int item, lev;
	int k_max = z_info->k_max;

	/* Allocate */
	obj_alloc = mem_alloc_alt((z_info->max_obj_depth + 1) * (k_max + 1) * sizeof(*obj_alloc));
	obj_alloc_great = mem_alloc_alt((z_info->max_obj_depth + 1) * (k_max + 1) * sizeof(*obj_alloc_great));
	obj_total_tval = mem_zalloc_alt((z_info->max_obj_depth + 1) * TV_MAX * sizeof(*obj_total_tval));
	obj_total_tval_great = mem_zalloc_alt((z_info->max_obj_depth + 1) * TV_MAX * sizeof(*obj_total_tval));

	/* The cumulative chance starts at zero for each level. */
	for (lev = 0; lev <= z_info->max_obj_depth; lev++) {
		obj_alloc[lev * (k_max + 1)] = 0;
		obj_alloc_great[lev * (k_max + 1)] = 0;
	}

	/* Fill the cumulative probability tables */
	for (item = 0; item < k_max; item++) {
		const struct object_kind *kind = &k_info[item];

		int min = kind->alloc_min;
		int max = kind->alloc_max;

		/* Go through all the dungeon levels */
		for (lev = 0; lev <= z_info->max_obj_depth; lev++) {
			int rarity = kind->alloc_prob;

			/* Add to the cumulative prob. in the standard table */
			if ((lev < min) || (lev > max)) rarity = 0;
			assert(rarity >= 0 && obj_alloc[(lev * (k_max + 1)) + item] <= (uint32_t)-1 - rarity);
			obj_alloc[(lev * (k_max + 1)) + item + 1] =
				obj_alloc[(lev * (k_max + 1)) + item] + rarity;

			obj_total_tval[lev * TV_MAX + kind->tval] += rarity;

			/* Add to the cumulative prob. in the "great" table */
			if (!kind_is_good(kind)) rarity = 0;
			obj_alloc_great[(lev * (k_max + 1)) + item + 1] =
				obj_alloc_great[(lev * (k_max + 1)) + item] + rarity;

			obj_total_tval_great[lev * TV_MAX + kind->tval] += rarity;
		}
	}
}

/*
 * Initialize ego-item allocation info
 *
 * The ego allocation probabilities table (alloc_ego_table) is sorted in
 * order of minimum depth.  Precisely why, I'm not sure!  But that is what
 * the code below is doing with the arrays 'num' and 'level_total'. -AS
 */
static void alloc_init_egos(void) {
	int *num = mem_zalloc((z_info->max_obj_depth + 1) * sizeof(int));
	int *level_total = mem_zalloc((z_info->max_obj_depth + 1) * sizeof(int));

	int i;

	for (i = 0; i < z_info->e_max; i++) {
		struct ego_item *ego = &e_info[i];

		if (ego->alloc_prob) {
			/* Count the entries */
			alloc_ego_size++;

			/* Group by level */
			num[ego->alloc_min]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < z_info->max_obj_depth; i++)
		num[i] += num[i - 1];

	/* Allocate the alloc_ego_table */
	alloc_ego_table = mem_zalloc(alloc_ego_size * sizeof(alloc_entry));

	/* Scan the ego-items */
	for (i = 0; i < z_info->e_max; i++) {
		struct ego_item *ego = &e_info[i];

		/* Count valid pairs */
		if (ego->alloc_prob) {
			int min_level = ego->alloc_min;

			/* Skip entries preceding our locale */
			int y = (min_level > 0) ? num[min_level - 1] : 0;

			/* Skip previous entries at this locale */
			int z = y + level_total[min_level];

			/* Load the entry */
			alloc_ego_table[z].index = i;
			alloc_ego_table[z].level = min_level;			/* Unused */
			alloc_ego_table[z].prob1 = ego->alloc_prob;
			alloc_ego_table[z].prob2 = ego->alloc_prob;
			alloc_ego_table[z].prob3 = ego->alloc_prob;

			/* Another entry complete for this locale */
			level_total[min_level]++;
		}
	}

	mem_free(level_total);
	mem_free(num);
}

/*
 * Initialize money info
 */
static void init_money_svals(void)
{
	int *money_svals;
	int i;

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

static void init_obj_make(void) {
	alloc_init_objects();
	alloc_init_egos();
	init_money_svals();
}

static void cleanup_obj_make(void) {
	int i;
	for (i = 0; i < num_money_types; i++) {
		string_free(money_type[i].name);
	}
	mem_free(money_type);
	mem_free(alloc_ego_table);
	mem_free_alt(obj_total_tval_great);
	mem_free_alt(obj_total_tval);
	mem_free_alt(obj_alloc_great);
	mem_free_alt(obj_alloc);
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
	for (i = ELEM_HIGH_MIN; i < ELEM_HIGH_MAX; i++)
		if (obj->el_info[i].res_level == 0) count++;

	if (count == 0) return false;

	/* Pick one */
	r = randint0(count);

	/* Find the one we picked */
	for (i = ELEM_HIGH_MIN; i < ELEM_HIGH_MAX; i++) {
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
 * Return the index, i, into the cumulative probability table, tbl, such that
 * tbl[i] <= p < tbl[i + 1].  p must be less than tbl[n - 1] and tbl[0] must be
 * zero.
 */
static int binary_search_probtable(const uint32_t *tbl, int n, uint32_t p)
{
	int ilow = 0, ihigh = n;

	assert(tbl[0] == 0 && tbl[n - 1] > p);
	while (1) {
		int imid;

		if (ilow == ihigh - 1) {
			assert(tbl[ilow] <= p && tbl[ihigh] > p);
			return ilow;
		}
		imid = (ilow + ihigh) / 2;
		if (tbl[imid] <= p) {
			ilow = imid;
		} else {
			ihigh = imid;
		}
	}
}


/**
 * Select an ego-item that fits the object's tval and sval.
 */
static struct ego_item *ego_find_random(struct object *obj, int level)
{
	int i;
	long total = 0L;

	alloc_entry *table = alloc_ego_table;

	/* Go through all possible ego items and find ones which fit this item */
	for (i = 0; i < alloc_ego_size; i++) {
		struct ego_item *ego = &e_info[table[i].index];

		/* Reset any previous probability of this type being picked */
		table[i].prob3 = 0;

		if (level <= ego->alloc_max) {
			int ood_chance = MAX(2, (ego->alloc_min - level) / 3);
			if (level >= ego->alloc_min || one_in_(ood_chance)) {
				struct poss_item *poss;

				for (poss = ego->poss_items; poss; poss = poss->next)
					if (poss->kidx == obj->kind->kidx) {
						table[i].prob3 = table[i].prob2;
						break;
					}

				/* Total */
				total += table[i].prob3;
			}
		}
	}

	if (total) {
		long value = randint0(total);
		for (i = 0; i < alloc_ego_size; i++) {
			/* Found the entry */
			if (value < table[i].prob3) {
				return &e_info[table[i].index];
			} else {
				/* Decrement */
				value = value - table[i].prob3;
			}
		}
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
		create_obj_flag_mask(newf, false, OFT_SUST, OFT_MAX);
		of_on(obj->flags, get_new_attr(obj->flags, newf));
	} else if (kf_has(obj->ego->kind_flags, KF_RAND_POWER) || (pick == 1)) {
		create_obj_flag_mask(newf, false, OFT_PROT, OFT_MISC, OFT_MAX);
		of_on(obj->flags, get_new_attr(obj->flags, newf));
	} else if (kf_has(obj->ego->kind_flags, KF_RAND_BASE_RES) || (pick > 1)) {
		/* Get a base resist if available, mark it as random */
		if (random_base_resist(obj, &resist)) {
			obj->el_info[resist].res_level = 1;
			obj->el_info[resist].flags |= EL_INFO_RANDOM | EL_INFO_IGNORE;
		}
	} else if (kf_has(obj->ego->kind_flags, KF_RAND_HI_RES)) {
		/* Get a high resist if available, mark it as random */
		if (random_high_resist(obj, &resist)) {
			obj->el_info[resist].res_level = 1;
			obj->el_info[resist].flags |= EL_INFO_RANDOM | EL_INFO_IGNORE;
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

	/* Add slays, brands and curses */
	copy_slays(&obj->slays, obj->ego->slays);
	copy_brands(&obj->brands, obj->ego->brands);
	copy_curses(obj, obj->ego->curses);

	/* Add resists */
	for (i = 0; i < ELEM_MAX; i++) {
		/* Take the larger of ego and base object resist levels */
		obj->el_info[i].res_level =
			MAX(obj->ego->el_info[i].res_level, obj->el_info[i].res_level);

		/* Union of flags so as to know when ignoring is notable */
		obj->el_info[i].flags |= obj->ego->el_info[i].flags;
	}

	/* Add activation (ego's activation will trump object's, if any). */
	if (obj->ego->activation) {
		obj->activation = obj->ego->activation;
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
 * Copy artifact data to a normal object.
 */
void copy_artifact_data(struct object *obj, const struct artifact *art)
{
	int i;
	struct object_kind *kind = lookup_kind(art->tval, art->sval);

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

	/* Activations can come from the artifact or the kind */
	if (art->activation) {
		obj->activation = art->activation;
		obj->time = art->time;
	} else if (kind->activation) {
		obj->activation = kind->activation;
		obj->time = kind->time;
	}

	/* Fix for artifact lights */
	of_off(obj->flags, OF_TAKES_FUEL);
	of_off(obj->flags, OF_BURNS_OUT);

	/* Timeouts are always 0 */
	obj->timeout = 0;

	of_union(obj->flags, art->flags);
	copy_slays(&obj->slays, art->slays);
	copy_brands(&obj->brands, art->brands);
	copy_curses(obj, art->curses);
	for (i = 0; i < ELEM_MAX; i++) {
		/* Use any non-zero artifact resist level */
		if (art->el_info[i].res_level != 0) {
			obj->el_info[i].res_level = art->el_info[i].res_level;
		}

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
static struct object *make_artifact_special(int level, int tval)
{
	int i;
	struct object *new_obj;

	/* No artifacts, do nothing */
	if (OPT(player, birth_no_artifacts)) return NULL;

	/* No artifacts in the town */
	if (!player->depth) return NULL;

	/* Check the special artifacts */
	for (i = 0; i < z_info->a_max; ++i) {
		const struct artifact *art = &a_info[i];
		struct object_kind *kind = lookup_kind(art->tval, art->sval);

		/* Skip "empty" artifacts */
		if (!art->name) continue;

		/* Make sure the kind was found */
		if (!kind) continue;

		/* Make sure it's the right tval (if given) */
		if (tval && (tval != art->tval)) continue;

		/* Skip non-special artifacts */
		if (!kf_has(kind->kind_flags, KF_INSTA_ART)) continue;

		/* Cannot make an artifact twice */
		if (is_artifact_created(art)) continue;

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

		/* Assign the template */
		new_obj = object_new();
		object_prep(new_obj, kind, art->alloc_min, RANDOMISE);

		/* Mark the item as an artifact */
		new_obj->artifact = art;

		/* Copy across all the data from the artifact struct */
		copy_artifact_data(new_obj, art);

		/* Mark the artifact as "created" */
		mark_artifact_created(art, true);

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

	/* Make sure birth no artifacts isn't set */
	if (OPT(player, birth_no_artifacts)) return false;

	/* No artifacts in the town */
	if (!player->depth) return false;

	/* Paranoia -- no "plural" artifacts */
	if (obj->number != 1) return false;

	/* Check the artifact list (skip the "specials") */
	for (i = 0; !obj->artifact && i < z_info->a_max; i++) {
		const struct artifact *art = &a_info[i];
		struct object_kind *kind = lookup_kind(art->tval, art->sval);

		/* Skip "empty" items */
		if (!art->name) continue;

		/* Make sure the kind was found */
		if (!kind) continue;

		/* Skip special artifacts */
		if (kf_has(kind->kind_flags, KF_INSTA_ART)) continue;

		/* Cannot make an artifact twice */
		if (is_artifact_created(art)) continue;

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
		mark_artifact_created(obj->artifact, true);
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
bool make_fake_artifact(struct object *obj, const struct artifact *artifact)
{
	struct object_kind *kind;

	/* Don't bother with empty artifacts */
	if (!artifact->tval) return false;

	/* Get the "kind" index */
	kind = lookup_kind(artifact->tval, artifact->sval);
	if (!kind) return false;

	/* Create the artifact */
	object_prep(obj, kind, 0, MAXIMISE);
	obj->artifact = artifact;
	copy_artifact_data(obj, artifact);

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

		if (tval_is_melee_weapon(obj)) {
			/* Super-charge the damage dice */
			while ((obj->dd * obj->ds > 0) && one_in_(4 * obj->dd * obj->ds)) {
				/* More dice or sides means more likely to get still more */
				if (randint0(obj->dd + obj->ds) < obj->dd) {
					int newdice = randint1(2 + obj->dd/obj->ds);
					while (((obj->dd + 1) * obj->ds <= 40) && newdice) {
						if (!one_in_(3)) {
							obj->dd++;
						}
						newdice--;
					}
				} else {
					int newsides = randint1(2 + obj->ds/obj->dd);
					while ((obj->dd * (obj->ds + 1) <= 40) && newsides) {
						if (!one_in_(3)) {
							obj->ds++;
						}
						newsides--;
					}
				}
			}
		} else if (tval_is_ammo(obj)) {
			/* Up to two chances to enhance damage dice. */
			if (one_in_(6) == 1) {
				obj->ds++;
				if (one_in_(10) == 1) {
					obj->ds++;
				}
			}
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

	/* Default slays, brands and curses */
	copy_slays(&obj->slays, k->slays);
	copy_brands(&obj->brands, k->brands);
	copy_curses(obj, k->curses);

	/* Default resists */
	for (i = 0; i < ELEM_MAX; i++) {
		obj->el_info[i].res_level = k->el_info[i].res_level;
		obj->el_info[i].flags = k->el_info[i].flags;
		obj->el_info[i].flags |= k->base->el_info[i].flags;
	}
}

/**
 * Attempt to apply curses to an object, with a corresponding increase in
 * generation level of the object
 */
static int apply_curse(struct object *obj, int lev)
{
	int pick, max_curses = randint1(4);
	int power = randint1(9) + 10 * m_bonus(9, lev);
	int new_lev = lev;

	if (of_has(obj->flags, OF_BLESSED)) return lev;

	while (max_curses--) {
		/* Try to curse it */
		int tries = 3;
		while (tries--) {
			pick = randint1(z_info->curse_max - 1);
			if (curses[pick].poss[obj->tval]) {
				if (append_object_curse(obj, pick, power)) {
					new_lev += randint1(1 + power / 10);
				}
				break;
			}
		}
	}

	return new_lev;
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
	int16_t power = 0;

	/* Chance of being `good` and `great` */
	/* This has changed over the years:
	 * 3.0.0:   good = MIN(75, lev + 10);      great = MIN(20, lev / 2); 
	 * 3.3.0:   good = (lev + 2) * 3;          great = MIN(lev / 4 + lev, 50);
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

	/* Give it a chance to be cursed */
	if (one_in_(20) && tval_is_wearable(obj)) {
		lev = apply_curse(obj, lev);
	}

	/* Apply magic */
	if (tval_is_weapon(obj)) {
		apply_magic_weapon(obj, lev, power);
	} else if (tval_is_armor(obj)) {
		apply_magic_armour(obj, lev, power);
	} else if (tval_is_ring(obj)) {
		if (obj->sval == lookup_sval(obj->tval, "Speed")) {
			/* Super-charge the ring */
			while (one_in_(2))
				obj->modifiers[OBJ_MOD_SPEED]++;
		}
	} else if (tval_is_chest(obj)) {
		/* Get a random, level-dependent set of chest traps */
		obj->pval = pick_chest_traps(obj);
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
	const uint32_t *objects;
	uint32_t total, value;
	int item;

	assert(level >= 0 && level <= z_info->max_obj_depth);
	assert(tval >= 0 && tval < TV_MAX);
	if (good) {
		objects = obj_alloc_great + level * (z_info->k_max + 1);
		total = obj_total_tval_great[level * TV_MAX + tval];
	} else {
		objects = obj_alloc + level * (z_info->k_max + 1);
		total = obj_total_tval[level * TV_MAX + tval];
	}

	/* No appropriate items of that tval */
	if (!total) return NULL;

	/* Pick an object */
	value = randint0(total);

	/*
	 * Find it.  Having a loop to calculate the cumulative probability
	 * here with only the tval and applying a binary search was slower
	 * for a test of getting a TV_SWORD from 4.2's available objects.
	 * So continue to use the O(N) search.
	 */
	for (item = 0; item < z_info->k_max; item++) {
		if (objkind_byid(item)->tval == tval) {
			uint32_t prob = objects[item + 1] - objects[item];

			if (value < prob) break;
			value -= prob;
		}
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
	const uint32_t *objects;
	uint32_t value;
	int item;

	/* Occasional level boost */
	if ((level > 0) && one_in_(z_info->great_obj))
		/* What a bizarre calculation */
		level = 1 + (level * z_info->max_obj_depth / randint1(z_info->max_obj_depth));

	/* Paranoia */
	level = MIN(level, z_info->max_obj_depth);
	level = MAX(level, 0);

	if (tval)
		return get_obj_num_by_kind(level, good, tval);

	objects = (good ? obj_alloc_great : obj_alloc) +
		level * (z_info->k_max + 1);

	/* Pick an object. */
	if (! objects[z_info->k_max]) {
		return NULL;
	}
	value = randint0(objects[z_info->k_max]);

	/* Find it with a binary search. */
	item = binary_search_probtable(objects, z_info->k_max + 1, value);

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
		bool extra_roll, int32_t *value, int tval)
{
	int base, tries = 3;
	struct object_kind *kind = NULL;
	struct object *new_obj;

	/* Try to make a special artifact */
	if (one_in_(good ? 10 : 1000)) {
		new_obj = make_artifact_special(lev, tval);
		if (new_obj) {
			if (value) *value = object_value_real(new_obj, 1);
			return new_obj;
		}

		/* If we failed to make an artifact, the player gets a good item */
		good = true;
	}

	/* Base level for the object */
	base = (good ? (lev + 10) : lev);

	/* Try to choose an object kind; reject most books the player can't read */
	while (tries) {
		kind = get_obj_num(base, good || great, tval);
		if (kind && tval_is_book_k(kind) && !obj_kind_can_browse(kind)) {
			if (one_in_(5)) break;
			kind = NULL;
			tries--;
			continue;
		} else {
			break;
		}
	}
	if (!kind)
		return NULL;

	/* Make the object, prep it and apply magic */
	new_obj = object_new();
	object_prep(new_obj, kind, lev, RANDOMISE);
	apply_magic(new_obj, lev, true, good, great, extra_roll);

	/* Generate multiple items */
	if (!new_obj->artifact && kind->gen_mult_prob >= randint1(100))
		new_obj->number = randcalc(kind->stack_size, lev, RANDOMISE);

	if (new_obj->number > new_obj->kind->base->max_stack)
		new_obj->number = new_obj->kind->base->max_stack;

	/* Get the value */
	if (value)
		*value = object_value_real(new_obj, new_obj->number);

	/* Boost of 20% per level OOD for uncursed objects */
	if ((!new_obj->curses) && (kind->alloc_min > c->depth) && value) {
		int32_t ood = kind->alloc_min - c->depth;
		int32_t frac = MAX(*value, 0) / 5;
		int32_t adj;

		if (frac <= INT32_MAX / ood) {
			adj = ood * frac;
		} else {
			adj = INT32_MAX;
		}
		if (*value <= INT32_MAX - adj) {
			*value += adj;
		} else {
			*value = INT32_MAX;
		}
	}

	return new_obj;
}


/**
 * Scatter some objects near the player
 */
void acquirement(struct loc grid, int level, int num, bool great)
{
	struct object *nice_obj;

	/* Acquirement */
	while (num--) {
		/* Make a good (or great) object (if possible) */
		nice_obj = make_object(cave, level, true, great, true, NULL, 0);
		if (!nice_obj) continue;

		nice_obj->origin = ORIGIN_ACQUIRE;
		nice_obj->origin_depth = convert_depth_to_origin(player->depth);

		/* Drop the object */
		drop_near(cave, &nice_obj, 0, grid, true, false);
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
struct object *make_gold(int lev, const char *coin_type)
{
	/* This average is 16 at dlev0, 80 at dlev40, 176 at dlev100. */
	int avg = (16 * lev)/10 + 16;
	int spread = lev + 10;
	int value = rand_spread(avg, spread);
	struct object *new_gold = mem_zalloc(sizeof(*new_gold)); 

	/* Increase the range to infinite, moving the average to 110% */
	while (one_in_(100) && value * 10 <= SHRT_MAX)
		value *= 10;

	/* Prepare a gold object */
	object_prep(new_gold, money_kind(coin_type, value), lev, RANDOMISE);

	/* If we're playing with no_selling, increase the value */
	if (OPT(player, birth_no_selling) && player->depth)	{
		value *= 5;
	}

	/* Cap gold at max short (or alternatively make pvals int32_t) */
	if (value >= SHRT_MAX) {
		value = SHRT_MAX - randint0(200);
	}

	new_gold->pval = value;

	return new_gold;
}

struct init_module obj_make_module = {
	.name = "object/obj-make",
	.init = init_obj_make,
	.cleanup = cleanup_obj_make
};
