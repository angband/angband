/**
 * \file obj-knowledge.c
 * \brief Object knowledge
 *
 * Copyright (c) 2016 Nick McConnell
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
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-properties.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-util.h"
#include "project.h"
#include "store.h"

/**
 * Overview
 * ========
 * This file deals with the new "rune-based ID" system.  This system operates
 * as follows:
 * - struct player has an object struct attached to it (obj_k) which contains
 *   the player's knowledge of object properties (runes)
 * - whenever the player learns a rune, 
 *   - if it's an object flag, that flag is set in obj_k
 *   - if it's an integer value, that value in obj_k is set to 1
 *   - if it's element info, the res_level value is set to 1
 *   - if it's a brand, a brand is added to obj_k with the relevant element
 *   - if it's a slay, a slay is added to obj_k with the right race flag or name
 * - every object has a known version which is filled in with details as the
 *   player learns them
 * - whenever the player learns a rune, that knowledge is applied to the known
 *   version of every object that the player has picked up or walked over
 *   or seen in a shop
 */

/**
 * ------------------------------------------------------------------------
 * Object knowledge data
 * This section covers initialisation, access and cleanup of rune data
 * ------------------------------------------------------------------------ */
static size_t rune_max;
static struct rune *rune_list;
static char *c_rune[] = {
	"enchantment to armor",
	"enchantment to hit",
	"enchantment to damage"
};

/**
 * Initialise the rune module
 */
static void init_rune(void)
{
	int i, j, count;

	/* Count runes (combat runes are fixed) */
	count = COMBAT_RUNE_MAX;
	for (i = 1; i < OF_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
		if (prop->subtype == OFT_NONE) continue;
		if (prop->subtype == OFT_LIGHT) continue;
		if (prop->subtype == OFT_DIG) continue;
		if (prop->subtype == OFT_THROW) continue;
		count++;
	}
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		count++;
	}
	for (i = 0; i < ELEM_HIGH_MAX; i++) {
		count++;
	}
	/* Note brand runes cover all brands with the same name */
	for (i = 1; i < z_info->brand_max; i++) {
		bool counted = false;
		if (brands[i].name) {
			for (j = 1; j < i; j++) {
				if (streq(brands[i].name, brands[j].name)) {
					counted = true;
				}
			}
			if (!counted) {
				count++;
			}
		}
	}
	/* Note slay runes cover all slays with the same flag/base */
	for (i = 1; i < z_info->slay_max; i++) {
		bool counted = false;
		if (slays[i].name) {
			for (j = 1; j < i; j++) {
				if (same_monsters_slain(i, j)) {
					counted = true;
				}
			}
			if (!counted) {
				count++;
			}
		}
	}
	for (i = 1; i < z_info->curse_max; i++) {
		if (curses[i].name) {
			count++;
		}
	}

	/* Now allocate and fill the rune list */
	rune_max = count;
	rune_list = mem_zalloc(rune_max * sizeof(struct rune));
	count = 0;
	for (i = 0; i < COMBAT_RUNE_MAX; i++) {
		rune_list[count++] = (struct rune) { RUNE_VAR_COMBAT, i, 0, c_rune[i] };
	}
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_MOD, i);
		rune_list[count++] = (struct rune) { RUNE_VAR_MOD, i, 0, prop->name };
	}
	for (i = 0; i < ELEM_HIGH_MAX; i++) {
		rune_list[count++] = (struct rune) { RUNE_VAR_RESIST, i, 0, projections[i].name };
	}
	for (i = 1; i < z_info->brand_max; i++) {
		bool counted = false;
		if (brands[i].name) {
			for (j = 1; j < i; j++) {
				if (streq(brands[i].name, brands[j].name)) {
					counted = true;
				}
			}
			if (!counted) {
				rune_list[count++] =
					(struct rune) { RUNE_VAR_BRAND, i, 0, brands[i].name };
			}
		}
	}
	for (i = 1; i < z_info->slay_max; i++) {
		bool counted = false;
		if (slays[i].name) {
			for (j = 1; j < i; j++) {
				if (same_monsters_slain(i, j)) {
					counted = true;
				}
			}
			if (!counted) {
				rune_list[count++] =
					(struct rune) { RUNE_VAR_SLAY, i, 0, slays[i].name };
			}
		}
	}
	for (i = 1; i < z_info->curse_max; i++) {
		if (curses[i].name) {
			rune_list[count++] =
				(struct rune) { RUNE_VAR_CURSE, i, 0, curses[i].name };
		}
	}
	for (i = 1; i < OF_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
		if (prop->subtype == OFT_NONE) continue;
		if (prop->subtype == OFT_LIGHT) continue;
		if (prop->subtype == OFT_DIG) continue;
		if (prop->subtype == OFT_THROW) continue;

		rune_list[count++] = (struct rune)
			{ RUNE_VAR_FLAG, i, 0, prop->name };
	}
}

/**
 * Get a rune by variety and index
 */
static int rune_index(size_t variety, int index)
{
	size_t i;

	/* Look for the rune */
	for (i = 0; i < rune_max; i++)
		if ((rune_list[i].variety == variety) && (rune_list[i].index == index))
			return i;

	/* Can't find it */
	return -1;
}

/**
 * Cleanup the rune module
 */
static void cleanup_rune(void)
{
	mem_free(rune_list);
}

struct init_module rune_module = {
	.name = "rune",
	.init = init_rune,
	.cleanup = cleanup_rune
};

/**
 * ------------------------------------------------------------------------
 * Rune knowledge functions
 * These functions provide details about the rune list for use in 
 * player knowledge screens
 * ------------------------------------------------------------------------ */
/**
 * The number of runes
 */
int max_runes(void)
{
	return rune_max;
}

/**
 * The variety of a rune
 */
enum rune_variety rune_variety(size_t i)
{
	return rune_list[i].variety;
}

/**
 * Reports if the player knows a given rune
 *
 * \param p is the player
 * \param i is the rune's number in the rune list
 */
bool player_knows_rune(struct player *p, size_t i)
{
	struct rune *r = &rune_list[i];

	switch (r->variety) {
		/* Combat runes */
		case RUNE_VAR_COMBAT: {
			if (r->index == COMBAT_RUNE_TO_A) {
				if (p->obj_k->to_a)
					return true;
			} else if (r->index == COMBAT_RUNE_TO_H) {
				if (p->obj_k->to_h)
					return true;
			} else if (r->index == COMBAT_RUNE_TO_D) {
				if (p->obj_k->to_d)
					return true;
			}
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			if (p->obj_k->modifiers[r->index])
				return true;
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			if (p->obj_k->el_info[r->index].res_level)
				return true;
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			assert(r->index < z_info->brand_max);
			if (p->obj_k->brands[r->index]) {
				return true;
			}
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			assert(r->index < z_info->slay_max);
			if (p->obj_k->slays[r->index]) {
				return true;
			}
			break;
		}
		/* Curse runes */
		case RUNE_VAR_CURSE: {
			assert(r->index < z_info->curse_max);
			if (p->obj_k->curses[r->index].power) {
				return true;
			}
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			if (of_has(p->obj_k->flags, r->index))
				return true;
			break;
		}
		default: {
			break;
		}
	}

	return false;
}

/**
 * The name of a rune
 */
char *rune_name(size_t i)
{
	struct rune *r = &rune_list[i];

	if (r->variety == RUNE_VAR_BRAND)
		return format("%s brand", r->name);
	else if (r->variety == RUNE_VAR_SLAY)
		return format("slay %s", r->name);
	else if (r->variety == RUNE_VAR_CURSE)
		return format("%s curse", r->name);
	else if (r->variety == RUNE_VAR_RESIST)
		return format("resist %s", r->name);
	else
		return format("%s", r->name);

	return NULL;
}

/**
 * The description of a rune
 */
char *rune_desc(size_t i)
{
	struct rune *r = &rune_list[i];

	switch (r->variety) {
		/* Combat runes */
		case RUNE_VAR_COMBAT: {
			if (r->index == COMBAT_RUNE_TO_A)
				return "Object magically increases the player's armor class";
			else if (r->index == COMBAT_RUNE_TO_H)
				return "Object magically increases the player's chance to hit";
			else if (r->index == COMBAT_RUNE_TO_D)
				return "Object magically increases the player's damage";
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			return format("Object gives the player a magical bonus to %s.",
						  r->name);
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			return format("Object affects the player's resistance to %s.",
						  r->name);
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			return format("Object brands the player's attacks with %s.",
						  r->name);
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			return format("Object makes the player's attacks against %s more powerful.", r->name);
			break;
		}
		/* Curse runes */
		case RUNE_VAR_CURSE: {
			return format("Object %s.", curses[r->index].desc);
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			return format("Object gives the player the property of %s.",
						  r->name);
			break;
		}
		default: {
			break;
		}
	}

	return NULL;
}

/**
 * The autoinscription index (if any) of a rune
 */
quark_t rune_note(size_t i)
{
	return rune_list[i].note;
}

/**
 * Set an autoinscription on a rune
 */
void rune_set_note(size_t i, const char *inscription)
{
	struct rune *r = &rune_list[i];

	if (!inscription)
		r->note = 0;
	else
		r->note = quark_add(inscription);
}

/**
 * ------------------------------------------------------------------------
 * Object knowledge predicates
 * These functions tell how much the player knows about an object
 * ------------------------------------------------------------------------ */

/**
 * Check if a brand is known to the player
 *
 * \param p is the player
 * \param b is the brand
 */
bool player_knows_brand(struct player *p, int i)
{
	return p->obj_k->brands[i];
}

/**
 * Check if a slay is known to the player
 *
 * \param p is the player
 * \param s is the slay
 */
bool player_knows_slay(struct player *p, int i)
{
	return p->obj_k->slays[i];
}

/**
 * Check if a curse is known to the player
 *
 * \param p is the player
 * \param c is the curse
 */
bool player_knows_curse(struct player *p, int index)
{
	return p->obj_k->curses[index].power == 1;
}

/**
 * Check if an ego item type is known to the player
 *
 * \param p is the player
 * \param ego is the ego item type
 */
bool player_knows_ego(struct player *p, struct ego_item *ego)
{
	int i;

	if (!ego) return false;

	/* All flags known */
	if (!of_is_subset(p->obj_k->flags, ego->flags)) return false;

	/* All modifiers known */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (randcalc(ego->modifiers[i], MAX_RAND_DEPTH, MAXIMISE) &&
			!p->obj_k->modifiers[i])
			return false;

	/* All elements known */
	for (i = 0; i < ELEM_MAX; i++)
		if (ego->el_info[i].res_level && !p->obj_k->el_info[i].res_level)
			return false;

	/* All brands known */
	for (i = 1; i < z_info->brand_max; i++) {
		if (ego->brands && ego->brands[i] && !player_knows_brand(p, i)) {
			return false;
		}
	}

	/* All slays known */
	for (i = 1; i < z_info->slay_max; i++) {
		if (ego->slays && ego->slays[i] && !player_knows_slay(p, i)) {
			return false;
		}
	}

	/* All curses known */
	for (i = 1; i < z_info->curse_max; i++) {
		if (ego->curses && ego->curses[i] && !player_knows_curse(p, i)) {
			return false;
		}
	}

	return true;
}

/**
 * Checks whether the player is aware of the object's effect when used
 *
 * \param obj is the object
 */
bool object_effect_is_known(const struct object *obj)
{
	if (obj->effect == obj->known->effect) return true;

	return false;
}

/**
 * Checks whether the object is known to be an artifact
 *
 * \param obj is the object
 */
bool object_is_known_artifact(const struct object *obj)
{
	if (!obj->known) return false;
	return obj->known->artifact ? true : false;
}

/**
 * Checks whether the object is in a store (not the home)
 *
 * \param obj is the object
 */
bool object_is_in_store(const struct object *obj)
{
	int i;
	struct object *obj1;

	/* Check all the store objects */
	for (i = 0; i < MAX_STORES; i++) {
		struct store *s = &stores[i];
		if (s->sidx == STORE_HOME) continue;
		for (obj1 = s->stock; obj1; obj1 = obj1->next)
			if (obj1 == obj) return true;
	}

	return false;
}

/**
 * Checks whether the object has the usual to-hit value
 *
 * \param obj is the object
 */
bool object_has_standard_to_h(const struct object *obj)
{
	/* Hack for curse object structures */
	if (!obj->kind) {
		return true;
	}
	if (tval_is_body_armor(obj) && !randcalc_varies(obj->kind->to_h)) {
		return (obj->to_h == obj->kind->to_h.base);
	} else {
		return (obj->to_h == 0);
	}
}

/**
 * Check if an object has a rune
 *
 * \param obj is the object
 * \param rune_no is the rune's number in the rune list
 */
bool object_has_rune(const struct object *obj, int rune_no)
{
	struct rune *r = &rune_list[rune_no];

	switch (r->variety) {
		/* Combat runes - just check them all */
		case RUNE_VAR_COMBAT: {
			if ((r->index == COMBAT_RUNE_TO_A) && (obj->to_a))
				return true;
			else if ((r->index == COMBAT_RUNE_TO_H) &&
					 !object_has_standard_to_h(obj))
				return true;
			else if ((r->index == COMBAT_RUNE_TO_D) && (obj->to_d))
				return true;
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			if (obj->modifiers[r->index] != 0)
				return true;
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			if (obj->el_info[r->index].res_level != 0)
				return true;
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			if (obj->brands) {
				int i;
				for (i = 0; i < z_info->brand_max; i++) {
					if (obj->brands[i] && streq(brands[i].name, r->name)) {
						return true;
					}
				}
			}
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			if (obj->slays) {
				int i;
				for (i = 0; i < z_info->slay_max; i++) {
					if (obj->slays[i] && same_monsters_slain(r->index, i)) {
						return true;
					}
				}
			}
			break;
		}
		/* Curse runes */
		case RUNE_VAR_CURSE: {
			if (obj->curses && obj->curses[r->index].power)
				return true;
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			if (of_has(obj->flags, r->index))
				return true;
			break;
		}
		default: break;
	}

	return false;
}

/**
 * Check if all non-curse runes on an object are known to the player
 *
 * \param obj is the object
 */
static bool object_non_curse_runes_known(const struct object *obj)
{
	int i;

	/* No known object */
	if (!obj->known) return false;

	/* Not all combat details known */
	if (obj->known->to_a != obj->to_a) return false;
	if (obj->known->to_h != obj->to_h) return false;
	if (obj->known->to_d != obj->to_d) return false;

	/* Not all modifiers known */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->modifiers[i] != obj->known->modifiers[i])
			return false;

	/* Not all elements known */
	for (i = 0; i < ELEM_MAX; i++)
		if ((obj->el_info[i].res_level != 0) &&
			(obj->known->el_info[i].res_level == 0))
			return false;

	/* Not all brands known */
	if (obj->brands) {
		if (!obj->known->brands)
			return false;
		for (i = 0; i < z_info->brand_max; i++) {
			if (obj->brands[i] && !obj->known->brands[i]) {
				return false;
			}
		}
	}

	/* Not all slays known */
	if (obj->slays) {
		if (!obj->known->slays)
			return false;
		for (i = 0; i < z_info->slay_max; i++) {
			if (obj->slays[i] && !obj->known->slays[i]) {
				return false;
			}
		}
	}

	/* Not all flags known */
	if (!of_is_subset(obj->known->flags, obj->flags)) return false;

	return true;
}

/**
 * Check if all the runes on an object are known to the player
 *
 * \param obj is the object
 */
bool object_runes_known(const struct object *obj)
{
	/* No known object */
	if (!obj->known) return false;

	/* Not all curses known */
	if (!curses_are_equal(obj, obj->known)) {
		return false;
	}

	/* Answer is now the same as for non-curse runes */
	return object_non_curse_runes_known(obj);
}


/**
 * Check if an object is fully known to the player
 *
 * \param obj is the object
 */
bool object_fully_known(const struct object *obj)
{
	/* Not all runes known */
	if (!object_runes_known(obj)) return false;

	/* Effect not known */
	if (!object_effect_is_known(obj)) return false;

	return true;
}


/**
 * Checks whether the player knows whether an object has a given flag
 *
 * \param obj is the object
 * \param flag is the flag
 */
bool object_flag_is_known(const struct object *obj, int flag)
{
	/* Object fully known means OK */
	if (object_fully_known(obj)) return true;

	/* Player knows the flag means OK */
	if (of_has(player->obj_k->flags, flag)) return true;

	/* Object has had a chance to display the flag means OK */
	if (of_has(obj->known->flags, flag)) return true;

	return false;
}

/**
 * Checks whether the player knows the given element properties of an object
 *
 * \param obj is the object
 * \param element is the element
 */
bool object_element_is_known(const struct object *obj, int element)
{
	if (element < 0 || element >= ELEM_MAX) return false;

	/* Object fully known means OK */
	if (object_fully_known(obj)) return true;

	/* Player knows the element means OK */
	if (player->obj_k->el_info[element].res_level) return true;

	/* Object has been exposed to the element means OK */
	if (obj->known->el_info[element].res_level) return true;

	return false;
}

/**
 * ------------------------------------------------------------------------
 * Object knowledge propagators
 * These functions transfer player knowledge to objects
 * ------------------------------------------------------------------------ */
/**
 * Sets the basic details on a known object
 */
void object_set_base_known(struct object *obj)
{
	assert(obj->known);
	obj->known->kind = obj->kind;
	obj->known->tval = obj->tval;
	obj->known->sval = obj->sval;
	obj->known->weight = obj->weight;
	obj->known->number = obj->number;

	/* Generic dice and ac, to_h for armor, and launcher multipliers */
	if (!obj->known->dd) {
		obj->known->dd = obj->kind->dd * player->obj_k->dd;
	}
	if (!obj->known->ds) {
		obj->known->ds = obj->kind->ds * player->obj_k->ds;
	}
	if (!obj->known->ac) {
		obj->known->ac = obj->kind->ac * player->obj_k->ac;
	}
	if (object_has_standard_to_h(obj)) {
		obj->known->to_h = obj->kind->to_h.base;
	}
	if (tval_is_launcher(obj)) {
		obj->known->pval = obj->pval;
	}

	/* Aware flavours and unflavored non-wearables get info now */
	if ((obj->kind->aware && obj->kind->flavor) ||
		(!tval_is_wearable(obj) && !obj->kind->flavor)) {
		obj->known->pval = obj->pval;
		obj->known->effect = obj->effect;
	}

	/* Know standard activations for wearables */
	if (tval_is_wearable(obj) && obj->kind->effect && obj->kind->aware) {
		obj->known->effect = obj->effect;
	}
}

/**
 * Gain knowledge based on sensing an object on the floor
 */
void object_sense(struct player *p, struct object *obj)
{
	struct object *known_obj = p->cave->objects[obj->oidx];
	struct loc grid = obj->grid;
	int none = tval_find_idx("none");

	/* Make new sensed objects where necessary or move them */
	if (known_obj == NULL ||
	    !square_holds_object(p->cave, grid, known_obj)) {
		struct object *new_obj;

		/* Check whether we need to make a new one */
		if (obj->known) {
			assert(known_obj == obj->known);
			new_obj = obj->known;
		} else {
			new_obj = object_new();
			obj->known = new_obj;
			p->cave->objects[obj->oidx] = new_obj;
			new_obj->oidx = obj->oidx;
		}

		/* Give it a fake kind and number. */
		new_obj->number = 1;
		if (tval_is_money(obj)) {
			new_obj->kind = unknown_gold_kind;
			new_obj->sval = lookup_sval(none, "<unknown treasure>");
		} else {
			new_obj->kind = unknown_item_kind;
			new_obj->sval = lookup_sval(none, "<unknown item>");
		}

		/* Attach it to the current floor pile */
		new_obj->grid = grid;
		pile_insert_end(&p->cave->squares[grid.y][grid.x].obj, new_obj);
	}
}

/**
 * Gain knowledge based on seeing an object on the floor
 */
void object_see(struct player *p, struct object *obj)
{
	struct object *known_obj = p->cave->objects[obj->oidx];
	struct loc grid = obj->grid;

	/* Make new known objects, fully know sensed ones, relocate old ones */
	if (known_obj == NULL) {
		/* Make a new one */
		struct object *new_obj;

		assert(! obj->known);
		new_obj = object_new();
		obj->known = new_obj;
		object_set_base_known(obj);

		/* List the known object */
		p->cave->objects[obj->oidx] = new_obj;
		new_obj->oidx = obj->oidx;

		/* If monster held, we're done */
		if (obj->held_m_idx) return;

		/* Attach it to the current floor pile */
		new_obj->grid = grid;
		pile_insert_end(&p->cave->squares[grid.y][grid.x].obj, new_obj);
	} else {
		struct loc old = known_obj->grid;

		/* Make sure knowledge is correct */
		assert(known_obj == obj->known);

		if (known_obj->kind != obj->kind) {
			/* Copy over actual details */
			object_set_base_known(obj);
		} else {
			known_obj->number = obj->number;
		}

		/* If monster held, we're done */
		if (obj->held_m_idx) return;

		/* Attach it to the current floor pile if necessary */
		if (! square_holds_object(p->cave, grid, known_obj)) {
			/* Detach from any old pile */
			if (!loc_is_zero(old) && square_holds_object(p->cave, old, known_obj)) {
				square_excise_object(p->cave, old, known_obj);
			}

			known_obj->grid = grid;
			pile_insert_end(&p->cave->squares[grid.y][grid.x].obj, known_obj);
		}
	}
}

/**
 * Gain knowledge based on being an the same square as an object
 */
void object_touch(struct player *p, struct object *obj)
{
	/* Automatically notice artifacts, mark as assessed */
	obj->known->artifact = obj->artifact;
	obj->known->notice |= OBJ_NOTICE_ASSESSED;

	/* Apply known properties to the object */
	player_know_object(p, obj);

	/* Log artifacts if found */
	if (obj->artifact)
		history_find_artifact(p, obj->artifact);
}


/**
 * Gain knowledge based on grabbing an object from a monster
 */
void object_grab(struct player *p, struct object *obj)
{
	struct object *known_obj = p->cave->objects[obj->oidx];

	/* Make new known objects, fully know sensed ones, relocate old ones */
	if (known_obj == NULL) {
		/* Make a new one */
		struct object *new_obj;

		assert(! obj->known);
		new_obj = object_new();
		obj->known = new_obj;
		object_set_base_known(obj);
		p->cave->objects[obj->oidx] = new_obj;
		new_obj->oidx = obj->oidx;
	} else {
		struct loc old = known_obj->grid;

		/* Make sure knowledge is correct */
		assert(known_obj == obj->known);

		/* Detach from any old (incorrect) floor pile
		 * This will be dead code once compatibility with old savefiles
		 * isn't needed.  It (and the declaration of old above) can be
		 * removed in 4.3.0. */
		if (!loc_is_zero(old) && square_holds_object(p->cave, old, known_obj)) {
			square_excise_object(p->cave, old, known_obj);
		}

		/* Copy over actual details */
		object_set_base_known(obj);
	}

	/* Touch the object */
	object_touch(p, obj);
}


/**
 * Transfer player object knowledge to an object
 *
 * \param p is the player
 * \param obj is the object
 */
void player_know_object(struct player *p, struct object *obj)
{
	int i, flag;
	bool seen = true;

	/* Unseen or only sensed objects don't get any ID */
	if (!obj) return;
	if (!obj->known) return;
	if (obj->kind != obj->known->kind) return;

	/* Distant objects just get base properties */
	if (obj->kind && !(obj->known->notice & OBJ_NOTICE_ASSESSED)) {
		object_set_base_known(obj);
		return;
	}

	/* Get the dice, and the pval for anything but chests */
	obj->known->dd = obj->dd * p->obj_k->dd;
	obj->known->ds = obj->ds * p->obj_k->ds;
	obj->known->ac = obj->ac * p->obj_k->ac;
	if (!tval_is_chest(obj))
		obj->known->pval = obj->pval;

	/* Set combat details */
	obj->known->to_a = p->obj_k->to_a * obj->to_a;
	if (!object_has_standard_to_h(obj))
		obj->known->to_h = p->obj_k->to_h * obj->to_h;
	obj->known->to_d = p->obj_k->to_d * obj->to_d;

	/* Set modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (p->obj_k->modifiers[i])
			obj->known->modifiers[i] = obj->modifiers[i];

	/* Set elements */
	for (i = 0; i < ELEM_MAX; i++)
		if (p->obj_k->el_info[i].res_level == 1) {
			obj->known->el_info[i].res_level = obj->el_info[i].res_level;
			obj->known->el_info[i].flags = obj->el_info[i].flags;
		}

	/* Set object flags */
	for (flag = of_next(p->obj_k->flags, FLAG_START); flag != FLAG_END;
		 flag = of_next(p->obj_k->flags, flag + 1)) {
		if (of_has(obj->flags, flag))
			of_on(obj->known->flags, flag);
	}

	/* Curse object structures are finished now */
	if (!obj->kind) {
		return;
	}

	/* Set brands */
	if (obj->brands) {
		for (i = 1; i < z_info->brand_max; i++) {
			if (player_knows_brand(p, i) && obj->brands[i]) {
				if (!obj->known->brands) {
					obj->known->brands = mem_zalloc(z_info->brand_max *
													sizeof(bool));
				}
				obj->known->brands[i] = true;
			}
		}
	}

	/* Set slays */
	if (obj->slays) {
		for (i = 1; i < z_info->slay_max; i++) {
			if (player_knows_slay(p, i) && obj->slays[i]) {
				if (!obj->known->slays) {
					obj->known->slays = mem_zalloc(z_info->slay_max *
												   sizeof(bool));
				}
				obj->known->slays[i] = true;
			}
		}
	}

	/* Set curses - be very careful to keep knowledge aligned */
	if (obj->curses) {
		bool known_cursed = false;
		for (i = 1; i < z_info->curse_max; i++) {
			if (p->obj_k->curses[i].power && obj->curses[i].power) {
				if (!obj->known->curses) {
					obj->known->curses = mem_zalloc(z_info->curse_max *
													sizeof(struct curse_data));
				}
				obj->known->curses[i].power = obj->curses[i].power;
				known_cursed = true;
			} else if (obj->known->curses) {
				obj->known->curses[i].power = 0;
			}
		}
		if (!known_cursed) {
			mem_free(obj->known->curses);
			obj->known->curses = NULL;
		}
	} else if (obj->known->curses) {
		mem_free(obj->known->curses);
		obj->known->curses = NULL;
	}

	/* Set ego type, jewellery type if known */
	if (player_knows_ego(p, obj->ego)) {
		seen = obj->ego->everseen;
		obj->known->ego = obj->ego;
	}

	if (object_non_curse_runes_known(obj) && tval_is_jewelry(obj)) {
		seen = obj->kind->everseen;
		object_flavor_aware(obj);
	}

	/* Report on new stuff */
	if (!seen) {
		char o_name[80];

		/* Describe the object if it's available */
		if (object_is_carried(p, obj)) {
			object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
			msg("You have %s (%c).", o_name, gear_to_label(obj));
		} else if (cave && square_holds_object(cave, p->grid, obj)) {
			object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
			msg("On the ground: %s.", o_name);
		}
	}

	/* Fully known objects have their known element and flag info set to 
	 * match the actual info, rather than showing what elements and flags
	 * the would be displaying if they had them */
	if (object_fully_known(obj)) {
		for (i = 0; i < ELEM_MAX; i++) {
			obj->known->el_info[i].res_level = obj->el_info[i].res_level;
			obj->known->el_info[i].flags = obj->el_info[i].flags;
		}
		of_wipe(obj->known->flags);
		of_copy(obj->known->flags, obj->flags);
	}
}

/**
 * Propagate player knowledge of objects to all objects
 *
 * \param p is the player
 */
void update_player_object_knowledge(struct player *p)
{
	int i;
	struct object *obj;

	/* Level objects */
	if (cave)
		for (i = 0; i < cave->obj_max; i++)
			player_know_object(p, cave->objects[i]);

	/* Player objects */
	for (obj = p->gear; obj; obj = obj->next)
		player_know_object(p, obj);

	/* Store objects */
	for (i = 0; i < MAX_STORES; i++) {
		struct store *s = &stores[i];
		for (obj = s->stock; obj; obj = obj->next)
			player_know_object(p, obj);
	}

	/* Curse objects */
	for (i = 1; i < z_info->curse_max; i++) {
		player_know_object(player, curses[i].obj);
	}

	/* Update */
	if (cave)
		autoinscribe_ground();
	autoinscribe_pack();
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/**
 * ------------------------------------------------------------------------
 * Object knowledge learners
 * These functions are for increasing player knowledge of object properties
 * ------------------------------------------------------------------------ */
/**
 * Learn a given rune
 *
 * \param p is the player
 * \param i is the rune index
 * \param message is whether or not to print a message
 */
static void player_learn_rune(struct player *p, size_t i, bool message)
{
	struct rune *r = &rune_list[i];
	bool learned = false;

	switch (r->variety) {
		/* Combat runes */
		case RUNE_VAR_COMBAT: {
			if (r->index == COMBAT_RUNE_TO_A) {
				if (!p->obj_k->to_a) {
					p->obj_k->to_a = 1;
					learned = true;
				}
			} else if (r->index == COMBAT_RUNE_TO_H) {
				if (!p->obj_k->to_h) {
					p->obj_k->to_h = 1;
					learned = true;
				}
			} else if (r->index == COMBAT_RUNE_TO_D) {
				if (!p->obj_k->to_d) {
					p->obj_k->to_d = 1;
					learned = true;
				}
			}
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			if (!p->obj_k->modifiers[r->index]) {
				p->obj_k->modifiers[r->index] = 1;
				learned = true;
			}
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			if (!p->obj_k->el_info[r->index].res_level) {
				p->obj_k->el_info[r->index].res_level = 1;
				learned = true;
			}
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			assert(r->index < z_info->brand_max);

			/* If the brand was unknown, add it to known brands */
			if (!player_knows_brand(p, r->index)) {
				int i;
				for (i = 1; i < z_info->brand_max; i++) {
					/* Check base and race flag */
					if (streq(brands[r->index].name, brands[i].name)) {
						p->obj_k->brands[i] = true;
						learned = true;
					}
				}
			}
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			assert(r->index < z_info->slay_max);

			/* If the slay was unknown, add it to known slays */
			if (!player_knows_slay(p, r->index)) {
				int i;
				for (i = 1; i < z_info->slay_max; i++) {
					/* Check base and race flag */
					if (same_monsters_slain(r->index, i)) {
						p->obj_k->slays[i] = true;
						learned = true;
					}
				}
			}
			break;
		}

		/* Curse runes */
		case RUNE_VAR_CURSE: {
			int i = r->index;
			assert(i < z_info->curse_max);

			/* If the curse was unknown, add it to known curses */
			if (!player_knows_curse(p, i)) {
				p->obj_k->curses[i].power = 1;
				learned = true;
			}
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			if (of_on(p->obj_k->flags, r->index))
				learned = true;
			break;
		}
		default: {
			learned = false;
			break;
		}
	}

	/* Nothing learned */
	if (!learned) return;

	/* Give a message */
	if (message)
		msgt(MSG_RUNE, "You have learned the rune of %s.", rune_name(i));

	/* Update knowledge */
	update_player_object_knowledge(p);
}

/**
 * Learn a flag
 */
void player_learn_flag(struct player *p, int flag)
{
	player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
	update_player_object_knowledge(p);
}

/**
 * Learn a curse
 */
void player_learn_curse(struct player *p, struct curse *curse)
{
	int index = rune_index(RUNE_VAR_CURSE, lookup_curse(curse->name));
	if (index >= 0) {
		player_learn_rune(p, index, true);
	}
	update_player_object_knowledge(p);
}

/**
 * Learn all innate runes
 *
 * \param p is the player
 */
void player_learn_innate(struct player *p)
{
	int element, flag;

	/* Elements */
	for (element = 0; element < ELEM_MAX; element++) {
		if (p->race->el_info[element].res_level != 0) {
			player_learn_rune(p, rune_index(RUNE_VAR_RESIST, element), false);
		}
	}

	/* Flags */
	for (flag = of_next(p->race->flags, FLAG_START); flag != FLAG_END;
		 flag = of_next(p->race->flags, flag + 1)) {
		player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), false);
	}

	update_player_object_knowledge(p);
}

/**
 * Learn absolutely everything
 *
 * \param p is the player
 */
void player_learn_all_runes(struct player *p)
{
	size_t i;

	for (i = 0; i < rune_max; i++)
		player_learn_rune(p, i, false);
}

/**
 * ------------------------------------------------------------------------
 * Functions for learning from the behaviour of indvidual objects or shapes
 * ------------------------------------------------------------------------ */
/**
 * Print a message when an object modifier is identified by use.
 *
 * \param obj is the object 
 * \param mod is the modifier being noticed
 */
void mod_message(struct object *obj, int mod)
{
	/* Special messages for individual properties */
	switch (mod) {
		case OBJ_MOD_STR:
			if (obj->modifiers[OBJ_MOD_STR] > 0)
				msg("You feel stronger!");
			else if (obj->modifiers[OBJ_MOD_STR] < 0)
				msg("You feel weaker!");
			break;
		case OBJ_MOD_INT:
			if (obj->modifiers[OBJ_MOD_INT] > 0)
				msg("You feel smarter!");
			else if (obj->modifiers[OBJ_MOD_INT] < 0)
				msg("You feel more stupid!");
			break;
		case OBJ_MOD_WIS:
			if (obj->modifiers[OBJ_MOD_WIS] > 0)
				msg("You feel wiser!");
			else if (obj->modifiers[OBJ_MOD_WIS] < 0)
				msg("You feel more naive!");
			break;
		case OBJ_MOD_DEX:
			if (obj->modifiers[OBJ_MOD_DEX] > 0)
				msg("You feel more dextrous!");
			else if (obj->modifiers[OBJ_MOD_DEX] < 0)
				msg("You feel clumsier!");
			break;
		case OBJ_MOD_CON:
			if (obj->modifiers[OBJ_MOD_CON] > 0)
				msg("You feel healthier!");
			else if (obj->modifiers[OBJ_MOD_CON] < 0)
				msg("You feel sicklier!");
			break;
		case OBJ_MOD_STEALTH:
			if (obj->modifiers[OBJ_MOD_STEALTH] > 0)
				msg("You feel stealthier.");
			else if (obj->modifiers[OBJ_MOD_STEALTH] < 0)
				msg("You feel noisier.");
			break;
		case OBJ_MOD_SPEED:
			if (obj->modifiers[OBJ_MOD_SPEED] > 0)
				msg("You feel strangely quick.");
			else if (obj->modifiers[OBJ_MOD_SPEED] < 0)
				msg("You feel strangely sluggish.");
			break;
		case OBJ_MOD_BLOWS:
			if (obj->modifiers[OBJ_MOD_BLOWS] > 0)
				msg("Your weapon tingles in your hands.");
			else if (obj->modifiers[OBJ_MOD_BLOWS] < 0)
				msg("Your weapon aches in your hands.");
			break;
		case OBJ_MOD_SHOTS:
			if (obj->modifiers[OBJ_MOD_SHOTS] > 0)
				msg("Your bow tingles in your hands.");
			else if (obj->modifiers[OBJ_MOD_SHOTS] < 0)
				msg("Your bow aches in your hands.");
			break;
		case OBJ_MOD_INFRA:
			msg("Your eyes tingle.");
			break;
		case OBJ_MOD_LIGHT:
			msg("It glows!");
			break;
		default:
			break;
	}
}

void object_curses_find_to_a(struct player *p, struct object *obj)
{
	int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_A);
	if (obj->curses) {
		int i;

		for (i = 1; i < z_info->curse_max; i++) {
			if (!obj->curses[i].power || !curses[i].obj)
				continue;

			if (curses[i].obj->to_a != 0) {
				player_learn_rune(p, index, true);

				/* Learn the curse */
				index = rune_index(RUNE_VAR_CURSE, i);
				if (index >= 0) {
					player_learn_rune(p, index, true);
					curses[i].obj->known->to_a = curses[i].obj->to_a;
				}
			}
		}
	}
}

void object_curses_find_to_h(struct player *p, struct object *obj)
{
	int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
	if (obj->curses) {
		int i;

		for (i = 1; i < z_info->curse_max; i++) {
			if (!obj->curses[i].power || !curses[i].obj)
				continue;

			if (curses[i].obj->to_h != 0) {
				player_learn_rune(p, index, true);

				/* Learn the curse */
				index = rune_index(RUNE_VAR_CURSE, i);
				if (index >= 0) {
					player_learn_rune(p, index, true);
					curses[i].obj->known->to_h = curses[i].obj->to_h;
				}
			}
		}
	}
}

void object_curses_find_to_d(struct player *p, struct object *obj)
{
	int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_D);
	if (obj->curses) {
		int i;

		for (i = 1; i < z_info->curse_max; i++) {
			if (!obj->curses[i].power || !curses[i].obj)
				continue;

			if (curses[i].obj->to_d != 0) {
				player_learn_rune(p, index, true);

				/* Learn the curse */
				index = rune_index(RUNE_VAR_CURSE, i);
				if (index >= 0) {
					player_learn_rune(p, index, true);
					curses[i].obj->known->to_d = curses[i].obj->to_d;
				}
			}
		}
	}
}

/**
 * Find flags caused by curses
 *
 * \param p is the player
 * \param obj is the object
 * \param test_flags is the set of flags to check for
 * \return whether a flag was found
 */
bool object_curses_find_flags(struct player *p, struct object *obj,
							  bitflag *test_flags)
{
	char o_name[80];
	bool new = false;

	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
	if (obj->curses) {
		int i;
		int index;
		bitflag f[OF_SIZE];
		int flag;

		for (i = 1; i < z_info->curse_max; i++) {
			if (!obj->curses[i].power || !curses[i].obj)
				continue;

			/* Get all the relevant flags */
			object_flags(curses[i].obj, f);
			of_inter(f, test_flags);
			for (flag = of_next(f, FLAG_START); flag != FLAG_END;
				 flag = of_next(f, flag + 1)) {
				/* Learn any new flags */
				if (!of_has(p->obj_k->flags, flag)) {
					new = true;
					player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
					if (p->upkeep->playing) {
						flag_message(flag, o_name);
					}
				}

				/* Learn the curse */
				index = rune_index(RUNE_VAR_CURSE, i);
				if (index >= 0) {
					player_learn_rune(p, index, true);
					of_on(curses[i].obj->known->flags, flag);
				}
			}
		}
	}

	return new;
}

/**
 * Find a modifiers caused by curses
 *
 * \param p is the player
 * \param obj is the object
 */
void object_curses_find_modifiers(struct player *p, struct object *obj)
{
	int i;

	if (obj->curses) {
		for (i = 1; i < z_info->curse_max; i++) {
			int index = rune_index(RUNE_VAR_CURSE, i);
			int j;

			if (!obj->curses[i].power || !curses[i].obj)
				continue;

			/* Learn all modifiers */
			for (j = 0; j < OBJ_MOD_MAX; j++) {
				if (curses[i].obj->modifiers[j]) {
					if (!p->obj_k->modifiers[j]) {
						player_learn_rune(p, rune_index(RUNE_VAR_MOD, j), true);
						if (p->upkeep->playing) {
							mod_message(obj, j);
						}
					}

					/* Learn the curse */
					if (index >= 0) {
						player_learn_rune(p, index, true);
						curses[i].obj->modifiers[j] = 1;
					}
				}
			}
		}
	}
}

/**
 * Find an elemental property caused by curses
 *
 * \param p is the player
 * \param obj is the object
 * \param elem the element
 * \return whether the element appeared in a curse
 */
bool object_curses_find_element(struct player *p, struct object *obj, int elem)
{
	char o_name[80];
	bool new = false;

	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
	if (obj->curses) {
		int i;

		for (i = 1; i < z_info->curse_max; i++) {
			int index = rune_index(RUNE_VAR_CURSE, i);

			if (!obj->curses[i].power || !curses[i].obj)
				continue;

			/* Does the object affect the player's resistance to the element? */
			if (curses[i].obj->el_info[elem].res_level != 0) {
				/* Learn the element properties if we don't know yet */
				if (!p->obj_k->el_info[elem].res_level) {
					msg("Your %s glows.", o_name);

					player_learn_rune(p, rune_index(RUNE_VAR_RESIST, elem),
									  true);
				}

				/* Learn the curse */
				if (index >= 0) {
					player_learn_rune(p, index, true);
					curses[i].obj->known->el_info[elem].res_level
						= curses[i].obj->el_info[elem].res_level;
				}
				new = true;
			}
		}
	}
	return new;
}

/**
 * Get a random unknown rune from an object
 *
 * \param p is the player
 * \param obj is the object
 * \return the index into the rune list, or -1 for no unknown runes
 */
int object_find_unknown_rune(struct player *p, struct object *obj)
{
	size_t i, num = 0;
	int *poss_runes = mem_zalloc(rune_max * sizeof(int));
	int chosen = -1;

	if (object_runes_known(obj)) return -1;

	for (i = 0; i < rune_max; i++)
		if (object_has_rune(obj, i) && !player_knows_rune(p, i))
			poss_runes[num++] = i;

	/* Grab a random rune from among the unknowns  */
	if (num) {
		chosen = poss_runes[randint0(num)];
	}

	mem_free(poss_runes);
	return chosen;
}

/**
 * Learn a random unknown rune from an object
 *
 * \param p is the player
 * \param obj is the object
 */
void object_learn_unknown_rune(struct player *p, struct object *obj)
{
	/* Get a random unknown rune from the object */
	int i = object_find_unknown_rune(p, obj);

	/* No unknown runes */
	if (i < 0) return;

	/* Learn the rune */
	player_learn_rune(p, i, true);
}

/**
 * Learn object properties that become obvious on wielding or wearing
 *
 * \param p is the player
 * \param obj is the wielded object
 */
void object_learn_on_wield(struct player *p, struct object *obj)
{
	bitflag f[OF_SIZE], obvious_mask[OF_SIZE];
	int i, flag;
	char o_name[80];

	assert(obj->known);
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

	/* Check the worn flag */
	if (obj->known->notice & OBJ_NOTICE_WORN) {
		return;
	} else {
		obj->known->notice |= OBJ_NOTICE_WORN;
	}

	/* Worn means tried (for flavored wearables) */
	object_flavor_tried(obj);

	/* Get the obvious object flags */
	create_obj_flag_mask(obvious_mask, true, OFID_WIELD, OFT_MAX);

	/* Make sustains obvious for items with that stat bonus */
	for (i = 0; i < STAT_MAX; i++) {
		int sust = sustain_flag(i);
		if (obj->modifiers[i]) {
			of_on(obvious_mask, sust);
		}
	}

	/* Learn about obvious, previously unknown flags */
	object_flags(obj, f);
	of_inter(f, obvious_mask);
	for (flag = of_next(f, FLAG_START); flag != FLAG_END;
		 flag = of_next(f, flag + 1)) {
		if (!of_has(p->obj_k->flags, flag)) {
			player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
			if (p->upkeep->playing) {
				flag_message(flag, o_name);
			}
		}
	}

	/* Learn all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (obj->modifiers[i] && !p->obj_k->modifiers[i]) {
			player_learn_rune(p, rune_index(RUNE_VAR_MOD, i), true);
			if (p->upkeep->playing) {
				mod_message(obj, i);
			}
		}
	}

	/* Learn curses */
	object_curses_find_to_a(p, obj);
	object_curses_find_to_h(p, obj);
	object_curses_find_to_d(p, obj);
	object_curses_find_flags(p, obj, obvious_mask);
	object_curses_find_modifiers(p, obj);
	for (i = 0; i < ELEM_MAX; i++) {
		if (p->obj_k->el_info[i].res_level) {
			(void) object_curses_find_element(p, obj, i);
		}
	}
}

/**
 * Learn object properties that become obvious on making a shapechange
 *
 * \param p is the player
 * \param name is the name of the assumed shape
 */
void shape_learn_on_assume(struct player *p, const char *name)
{
	bitflag f[OF_SIZE], obvious_mask[OF_SIZE];
	int i, flag;
	struct player_shape *shape = lookup_player_shape(name);

	/* Get the shape's obvious flags */
	create_obj_flag_mask(obvious_mask, true, OFID_WIELD, OFT_MAX);
	of_copy(f, shape->flags);
	of_inter(f, obvious_mask);

	/* Learn flags */
	for (flag = of_next(f, FLAG_START); flag != FLAG_END;
		 flag = of_next(f, flag + 1)) {
		player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
	}

	/* Learn all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (shape->modifiers[i]) {
			player_learn_rune(p, rune_index(RUNE_VAR_MOD, i), true);
		}
	}
}

/**
 * Learn object properties that become obvious on use, mark it as
 * aware and reward the player with some experience.
 *
 * \param p is the player
 * \param obj is the used object
 */
void object_learn_on_use(struct player *p, struct object *obj)
{
	/* Object level */
	int lev = obj->kind->level;

	object_flavor_aware(obj);
	obj->known->effect = obj->effect;
	update_player_object_knowledge(p);
	player_exp_gain(p, (lev + (p->lev / 2)) / p->lev);

	p->upkeep->notice |= PN_IGNORE;
}

/**
 * Notice any slays on a particular object which affect a particular monster.
 *
 * \param obj is the object on which we are noticing slays
 * \param mon the monster we are trying to slay
 */
void object_learn_slay(struct player *p, struct object *obj, int index)
{
	/* Learn about the slay */
	if (!player_knows_slay(p, index)) {
		int i;

		/* Find the rune index */
		for (i = 1; i < z_info->slay_max; i++) {
			if (same_monsters_slain(i, index)) {
				break;
			}
		}
		assert(i < z_info->slay_max);

		/* Learn the rune */
		player_learn_rune(p, rune_index(RUNE_VAR_SLAY, i), true);
		update_player_object_knowledge(p);
	}
}

/**
 * Notice any brands on a particular object which affect a particular monster.
 *
 * \param obj is the object on which we are noticing brands
 * \param mon the monster we are trying to brand
 */
void object_learn_brand(struct player *p, struct object *obj, int index)
{
	/* Learn about the brand */
	if (!player_knows_brand(p, index)) {
		int i;

		/* Find the rune index */
		for (i = 1; i < z_info->brand_max; i++) {
			if (streq(brands[i].name, brands[index].name)) {
				break;
			}
		}
		assert(i < z_info->brand_max);

		/* Learn the rune */
		player_learn_rune(p, rune_index(RUNE_VAR_BRAND, i), true);
		update_player_object_knowledge(p);
	}
}


/**
 * Learn attack bonus on making a ranged attack.
 * Can be applied to the missile or the missile launcher
 *
 * \param p is the player
 * \param obj is the missile or launcher
 */
void missile_learn_on_ranged_attack(struct player *p, struct object *obj)
{
	if (p->obj_k->to_h && p->obj_k->to_d)
		return;

	assert(obj->known);
	if (!object_has_standard_to_h(obj)) {
		int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
		player_learn_rune(p, index, true);
	}
	if (obj->to_d) {
		int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_D);
		player_learn_rune(p, index, true);
	}
	object_curses_find_to_h(p, obj);
	object_curses_find_to_d(p, obj);
}

/**
 * ------------------------------------------------------------------------
 * Functions for learning about equipment properties
 * These functions are for gaining object knowledge from the behaviour of
 * the player's equipment or shape
 * ------------------------------------------------------------------------ */
/**
 * Learn things which happen on defending.
 *
 * \param p is the player
 */
void equip_learn_on_defend(struct player *p)
{
	int i;

	if (p->obj_k->to_a) return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (obj) {
			assert(obj->known);
			if (obj->to_a) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_A);
				player_learn_rune(p, index, true);
			}
			object_curses_find_to_a(p, obj);
			if (p->obj_k->to_a) return;
		}
	}
	if (p->shape) {
		struct player_shape *shape = lookup_player_shape(p->shape->name);
		if (shape->to_a != 0) {
			int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_A);
			player_learn_rune(p, index, true);
		}
	}
}

/**
 * Learn to-hit bonus on making a ranged attack.
 * Does not apply to weapon or bow
 *
 * \param p is the player
 */
void equip_learn_on_ranged_attack(struct player *p)
{
	int i;

	if (p->obj_k->to_h) return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (i == slot_by_name(p, "weapon")) continue;
		if (i == slot_by_name(p, "shooting")) continue;
		if (obj) {
			assert(obj->known);
			if (!object_has_standard_to_h(obj)) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
				player_learn_rune(p, index, true);
			}
			object_curses_find_to_h(p, obj);
			if (p->obj_k->to_h) return;
		}
	}
	if (p->shape) {
		struct player_shape *shape = lookup_player_shape(p->shape->name);
		if (shape->to_h != 0) {
			int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
			player_learn_rune(p, index, true);
		}
	}
}


/**
 * Learn things which happen on making a melee attack.
 * Does not apply to bow
 *
 * \param p is the player
 */
void equip_learn_on_melee_attack(struct player *p)
{
	int i;

	if (p->obj_k->to_h && p->obj_k->to_d)
		return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (i == slot_by_name(p, "shooting")) continue;
		if (obj) {
			assert(obj->known);
			if (!object_has_standard_to_h(obj)) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
				player_learn_rune(p, index, true);
			}
			if (obj->to_d) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_D);
				player_learn_rune(p, index, true);
			}
			object_curses_find_to_h(p, obj);
			object_curses_find_to_d(p, obj);
			if (p->obj_k->to_h && p->obj_k->to_d) return;
		}
	}
	if (p->shape) {
		struct player_shape *shape = lookup_player_shape(p->shape->name);
		if (shape->to_h != 0) {
			int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
			player_learn_rune(p, index, true);
		}
		if (shape->to_d != 0) {
			int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_D);
			player_learn_rune(p, index, true);
		}
	}
}


/**
 * Learn a given object flag on wielded items.
 *
 * \param p is the player
 * \param flag is the flag to notice
 */
void equip_learn_flag(struct player *p, int flag)
{
	int i;
	bitflag f[OF_SIZE];
	of_wipe(f);
	of_on(f, flag);

	/* No flag */
	if (!flag) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (!obj) continue;
		assert(obj->known);

		/* Does the object have the flag? */
		if (of_has(obj->flags, flag)) {
			if (!of_has(p->obj_k->flags, flag)) {
				char o_name[80];
				object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
				flag_message(flag, o_name);
				player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
			}
		} else if (!object_fully_known(obj)) {
			/* Objects not fully known yet get marked as having had a chance
			 * to display the flag */
			of_on(obj->known->flags, flag);
		}

		/* Flag may be on a curse */
		object_curses_find_flags(p, obj, f);
	}
	if (p->shape) {
		struct player_shape *shape = lookup_player_shape(p->shape->name);
		if (of_has(shape->flags, flag) && !of_has(p->obj_k->flags, flag)) {
			msg("You understand your %s shape better.", p->shape->name);
			player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
		}
	}
}

/**
 * Learn the elemental resistance properties on wielded items.
 *
 * \param p is the player
 * \param element is the element to notice
 */
void equip_learn_element(struct player *p, int element)
{
	int i;

	/* Invalid element or element already known */
	if (element < 0 || element >= ELEM_MAX) return;
	if (p->obj_k->el_info[element].res_level == 1) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (!obj) continue;
		assert(obj->known);

		/* Does the object affect the player's resistance to the element? */
		if (obj->el_info[element].res_level != 0) {
			char o_name[80];
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

			/* Message */
			msg("Your %s glows.", o_name);

			/* Learn the element properties */
			player_learn_rune(p, rune_index(RUNE_VAR_RESIST, element), true);
		} else if (!object_fully_known(obj)) {
			/* Objects not fully known yet get marked as having had a chance
			 * to display the element */
			obj->known->el_info[element].res_level = 1;
			obj->known->el_info[element].flags = obj->el_info[element].flags;
		}

		/* Element may be on a curse */
		object_curses_find_element(p, obj, element);
	}
	if (p->shape) {
		struct player_shape *shape = lookup_player_shape(p->shape->name);
		if (shape->el_info[element].res_level &&
			!p->obj_k->el_info[element].res_level) {
			msg("You understand your %s shape better.", p->shape->name);
			player_learn_rune(p, rune_index(RUNE_VAR_RESIST, element), true);
		}
	}
}

/**
 * Learn things that would be noticed in time.
 *
 * \param p is the player
 */
void equip_learn_after_time(struct player *p)
{
	int i, flag;
	bitflag f[OF_SIZE], timed_mask[OF_SIZE];
	bool messaged = false;

	/* Get the timed flags */
	create_obj_flag_mask(timed_mask, true, OFID_TIMED, OFT_MAX);

	/* Get the unknown timed flags, and return if there are none */
	object_flags(p->obj_k, f);
	of_negate(f);
	of_inter(timed_mask, f);
	if (of_is_empty(timed_mask)) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		char o_name[80];
		struct object *obj = slot_object(p, i);

		if (!obj) continue;
		assert(obj->known);
		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

		/* Get the unknown timed flags for this object */
		object_flags(obj, f);
		of_inter(f, timed_mask);

		/* Attempt to learn every flag */
		for (flag = of_next(f, FLAG_START); flag != FLAG_END;
			 flag = of_next(f, flag + 1)) {
			if (!of_has(p->obj_k->flags, flag)) {
				flag_message(flag, o_name);
			}
			player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
		}

		/* Learn curses */
		object_curses_find_flags(p, obj, timed_mask);

		if (!object_fully_known(obj)) {
			/* Objects not fully known yet get marked as having had a chance
			 * to display all the timed flags */
			of_union(obj->known->flags, timed_mask);
		}
	}
	if (p->shape) {
		struct player_shape *shape = lookup_player_shape(p->shape->name);
		for (flag = of_next(timed_mask, FLAG_START); flag != FLAG_END;
			 flag = of_next(timed_mask, flag + 1)) {
			if (of_has(shape->flags, flag) && !of_has(p->obj_k->flags, flag)) {
				if (!messaged) {
					msg("You understand your %s shape better.", p->shape->name);
					messaged = true;
				}
				player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
			}
		}
	}
}

/**
 * ------------------------------------------------------------------------
 * Object kind functions
 * These deal with knowledge of an object's kind
 * ------------------------------------------------------------------------ */

/**
 * Checks whether an object counts as "known" due to EASY_KNOW status
 *
 * \param obj is the object
 */
bool easy_know(const struct object *obj)
{
	assert(obj->kind);
	if (obj->kind->aware && kf_has(obj->kind->kind_flags, KF_EASY_KNOW))
		return true;
	else
		return false;
}

/**
 * Checks whether the player is aware of the object's flavour
 *
 * \param obj is the object
 */
bool object_flavor_is_aware(const struct object *obj)
{
	assert(obj->kind);
	return obj->kind->aware;
}

/**
 * Checks whether the player has tried to use other objects of the same kind
 *
 * \param obj is the object
 */
bool object_flavor_was_tried(const struct object *obj)
{
	assert(obj->kind);
	return obj->kind->tried;
}

/**
 * Mark an object's flavour as as one the player is aware of.
 *
 * \param obj is the object whose flavour should be marked as aware
 */
void object_flavor_aware(struct object *obj)
{
	int y, x, i;
	struct object *obj1;

	assert(obj->known);
	if (obj->kind->aware) return;
	obj->kind->aware = true;
	obj->known->effect = obj->effect;

	/* Fix ignore/autoinscribe */
	if (kind_is_ignored_unaware(obj->kind))
		kind_ignore_when_aware(obj->kind);
	player->upkeep->notice |= PN_IGNORE;

	/* Update player objects */
	for (obj1 = player->gear; obj1; obj1 = obj1->next)
		object_set_base_known(obj1);

	/* Store objects */
	for (i = 0; i < MAX_STORES; i++) {
		struct store *s = &stores[i];
		for (obj1 = s->stock; obj1; obj1 = obj1->next)
			object_set_base_known(obj1);
	}

	/* Quit if no dungeon yet */
	if (!cave) return;

	/* Some objects change tile on awareness, so update display for all
	 * floor objects of this kind */
	for (y = 1; y < cave->height; y++) {
		for (x = 1; x < cave->width; x++) {
			bool light = false;
			const struct object *floor_obj;
			struct loc grid = loc(x, y);

			for (floor_obj = square_object(cave, grid); floor_obj;
				 floor_obj = floor_obj->next)
				if (floor_obj->kind == obj->kind) {
					light = true;
					break;
				}

			if (light) square_light_spot(cave, grid);
		}
	}
}

/**
 * Mark an object's flavour as tried.
 *
 * \param obj is the object whose flavour should be marked
 */
void object_flavor_tried(struct object *obj)
{
	assert(obj);
	assert(obj->kind);
	obj->kind->tried = true;
}
