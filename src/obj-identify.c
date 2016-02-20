/**
 * \file obj-identify.c
 * \brief Object identification and knowledge routines
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2009 Brian Bull
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
#include "game-event.h"
#include "game-world.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-timed.h"
#include "player-util.h"

/**
 * Time last item was wielded
 */
s32b object_last_wield;

/**
 * ------------------------------------------------------------------------
 * Object knowledge predicates
 * These answer questions about an object's ID status
 * ------------------------------------------------------------------------ */


/**
 * \returns whether an object counts as "known" due to EASY_KNOW status
 */
bool easy_know(const struct object *obj)
{
	if (obj->kind->aware && kf_has(obj->kind->kind_flags, KF_EASY_KNOW))
		return true;
	else
		return false;
}

/**
 * Is the player aware of all of an object's flags?
 *
 * \param obj is the object
 */
bool object_all_flags_are_known(const struct object *obj)
{
	if (!obj->known) return false;
	return easy_know(obj) || of_is_subset(obj->known->flags, obj->flags)
		? true : false;
}


/**
 * Is the player aware of all of an object's elemental properties?
 *
 * \param obj is the object
 */
bool object_all_elements_are_known(const struct object *obj)
{
	size_t i;

	for (i = 0; i < ELEM_MAX; i++)
		/* Only check if the flags are set if there's something to look at */
		if ((obj->el_info[i].res_level != 0) &&
			!object_element_is_known(obj, i))
			return false;

	return true;
}


/**
 * Is the player aware of all of an object's brands and slays?
 *
 * \param obj is the object
 */
bool object_all_brands_and_slays_are_known(const struct object *obj)
{
	if (!obj->known) return false;
	if (!brands_are_equal(obj->brands, obj->known->brands))
		return false;
	if (!slays_are_equal(obj->slays, obj->known->slays))
		return false;

	return true;
}

/**
 * Is the player aware of all of an object's miscellaneous proerties?
 *
 * \param obj is the object
 */
bool object_all_miscellaneous_are_known(const struct object *obj)
{
	if (!obj->known) return false;
	if (easy_know(obj)) return true;
	if (obj->known->artifact != obj->artifact) return false;
	if (obj->known->ego != obj->ego) return false;
	if (obj->known->pval != obj->pval) return false;
	if (obj->known->dd != obj->dd) return false;
	if (obj->known->ds != obj->ds) return false;
	if (obj->known->ac != obj->ac) return false;
	if (obj->known->to_a != obj->to_a) return false;
	if (obj->known->to_h != obj->to_h) return false;
	if (obj->known->to_d != obj->to_d) return false;
	if (obj->known->effect != obj->effect) return false;
	return true;
}

/**
 * Is the player aware of all of an object's miscellaneous properties?
 *
 * \param obj is the object
 */
bool object_all_but_flavor_is_known(const struct object *obj)
{
	if (!obj->known) return false;
	if (!object_all_flags_are_known(obj)) return false;
	if (!object_all_elements_are_known(obj)) return false;
	if (!object_all_brands_and_slays_are_known(obj)) return false;
	if (!object_all_miscellaneous_are_known(obj)) return false;

	return true;
}

/**
 * \returns whether an object should be treated as fully known
 */
bool object_is_known(const struct object *obj)
{
	if (!object_flavor_is_aware(obj)) return false;
	return object_all_but_flavor_is_known(obj) ? true : false;
}

/**
 * \returns whether the object is known to be an artifact
 */
bool object_is_known_artifact(const struct object *obj)
{
	if (!obj->known) return false;
	return (obj->artifact && obj->known->artifact);
}

/**
 * \returns whether the object is known to not be an artifact
 */
bool object_is_known_not_artifact(const struct object *obj)
{
	if (!obj->known) return false;
	return (!obj->artifact && obj->known->artifact);
}

/**
 * \returns whether the object has been worn/wielded
 */
bool object_was_worn(const struct object *obj)
{
	if (!obj->known) return false;
	return obj->known->notice & OBJ_NOTICE_WORN ? true : false;
}

/**
 * \returns whether the player is aware of the object's flavour
 */
bool object_flavor_is_aware(const struct object *obj)
{
	assert(obj->kind);
	return obj->kind->aware;
}

/**
 * \returns whether the player has tried to use other objects of the same kind
 */
bool object_flavor_was_tried(const struct object *obj)
{
	assert(obj->kind);
	return obj->kind->tried;
}

/**
 * \returns whether the player is aware of the object's effect when used
 */
bool object_effect_is_known(const struct object *obj)
{
	assert(obj->kind);
	if (obj->effect == obj->known->effect)
		return true;

	return false;
}

/**
 * \returns whether any ego or artifact name is available to the player
 */
bool object_name_is_visible(const struct object *obj)
{
	bool ego = obj->ego && obj->known && obj->known->ego;
	bool art = obj->artifact && obj->known && obj->known->artifact;
	return (ego || art) ? true : false;
}

/**
 * \returns whether both the object is both an ego and the player knows it is
 */
bool object_ego_is_visible(const struct object *obj)
{
	if (obj->ego && obj->known && obj->known->ego)
		return true;

	return false;
}

/**
 * \returns whether the object's attack plusses are known
 */
bool object_attack_plusses_are_visible(const struct object *obj)
{
	if (!obj->known) return false;
	/* Bonuses have been revealed or for sale */
	if (obj->known->to_h && obj->known->to_d)
		return true;

	return false;
}

/**
 * \returns whether the object's defence bonuses are known
 */
bool object_defence_plusses_are_visible(const struct object *obj)
{
	if (!obj->known) return false;
	/* Bonuses have been revealed or for sale */
	if (obj->known->to_a)
		return true;

	return false;
}


/**
 * \returns whether the player knows whether an object has a given flag
 */
bool object_flag_is_known(const struct object *obj, int flag)
{
	if (obj->known && of_has(obj->known->flags, flag))
		return true;

	return false;
}

/**
 * \returns whether the player knows the given element properties of an object
 */
bool object_element_is_known(const struct object *obj, int element)
{
	if (element < 0 || element >= ELEM_MAX) return false;

	if (obj->known && obj->known->el_info[element].res_level)
		return true;

	return false;
}


/**
 * \returns whether a specific modifier is known to the player
 */
bool object_this_mod_is_visible(const struct object *obj, int mod)
{
	assert(obj->kind);

	if (!obj->known) return false;
	if (obj->known->modifiers[mod])
		return true;

	return false;
}

/**
 * ------------------------------------------------------------------------
 * Object knowledge improvers
 * These add to the player's knowledge of an object, where necessary
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
	obj->known->number = obj->number;

	/* Generic dice and ac */
	obj->known->dd = obj->kind->dd * player->obj_k->dd;
	obj->known->ds = obj->kind->ds * player->obj_k->ds;
	obj->known->ac = obj->kind->ac * player->obj_k->ac;

	/* Aware flavours get info now */
	if (obj->kind->flavor && obj->kind->aware) {
		obj->known->pval = obj->pval;
		obj->known->effect = obj->effect;
	}
}

/**
 * Mark an object's flavour as as one the player is aware of.
 *
 * \param obj is the object whose flavour should be marked as aware
 */
void object_flavor_aware(struct object *obj)
{
	int y, x;

	assert(obj->known);
	if (obj->kind->aware) return;
	obj->kind->aware = true;
	obj->known->effect = obj->effect;

	/* Fix ignore/autoinscribe */
	if (kind_is_ignored_unaware(obj->kind))
		kind_ignore_when_aware(obj->kind);
	player->upkeep->notice |= PN_IGNORE;

	/* Quit if no dungeon yet */
	if (!cave) return;

	/* Some objects change tile on awareness, so update display for all
	 * floor objects of this kind */
	for (y = 1; y < cave->height; y++) {
		for (x = 1; x < cave->width; x++) {
			bool light = false;
			const struct object *floor_obj;

			for (floor_obj = square_object(cave, y, x); floor_obj;
				 floor_obj = floor_obj->next)
				if (floor_obj->kind == obj->kind) {
					light = true;
					break;
				}

			if (light) square_light_spot(cave, y, x);
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
	assert(obj->known);

	obj->kind->tried = true;
}

/**
 * Notice object properties that become obvious on use, mark it as
 * aware and reward the player with some experience.
 */
void object_notice_on_use(struct object *obj)
{
	/* Object level */
	int lev = obj->kind->level;

	object_flavor_aware(obj);
	obj->known->effect = obj->effect;
	player_exp_gain(player, (lev + (player->lev / 2)) / player->lev);

	player->upkeep->notice |= PN_IGNORE;
}

