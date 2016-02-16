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
 * Checks for additional knowledge implied by what the player already knows.
 *
 * \param obj is the object to check
 *
 * returns whether it calls object_notice_everyting
 */
bool object_check_for_ident(struct object *obj)
{
	assert(obj->known);
	/* Check things which need to be learned */
	if (!object_all_flags_are_known(obj)) return false;
	if (!object_all_elements_are_known(obj)) return false;
	if (!object_all_brands_and_slays_are_known(obj)) return false;

	/* If we know attack bonuses, and defence bonuses, and effect, then
	 * we effectively know everything, so mark as such */
	if ((object_attack_plusses_are_visible(obj) ||
		 (object_was_sensed(obj) && obj->to_h == 0 && obj->to_d == 0)) &&
	    (object_defence_plusses_are_visible(obj) ||
		 (object_was_sensed(obj) && obj->to_a == 0)) &&
	    (object_effect_is_known(obj) || !object_effect(obj)))
	{
		/* In addition to knowing the pval flags, it is necessary to know 
		 * the modifiers to know everything */
		int i;
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (!object_this_mod_is_visible(obj, i))
				break;
		if (i == OBJ_MOD_MAX) {
			object_notice_everything(obj);
			return true;
		}
	}

	return false;
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
 * Make the player aware of all of an object's flags.
 *
 * \param obj is the object to mark
 */
void object_know_all_flags(struct object *obj)
{
	assert(obj->known);
	of_setall(obj->known->flags);
}


/**
 * Make the player aware of all of an object's elemental properties.
 *
 * \param obj is the object to mark
 */
void object_know_all_elements(struct object *obj)
{
	size_t i;

	assert(obj->known);
	for (i = 0; i < ELEM_MAX; i++)
		obj->known->el_info[i].res_level = 1;
}


/**
 * Make the player aware of all of an object's miscellaneous properties.
 *
 * \param obj is the object to mark
 */
void object_know_all_miscellaneous(struct object *obj)
{
	assert(obj->known);
	obj->known->artifact = (struct artifact *)1;
	obj->known->ego = (struct ego_item *)1;
	obj->known->pval = 1;
	obj->known->dd = 1;
	obj->known->ds = 1;
	obj->known->ac = 1;
	obj->known->to_a = 1;
	obj->known->to_h = 1;
	obj->known->to_d = 1;
	obj->known->effect = (struct effect *)1;
}

/**
 * Make the player aware of all of an object' properties except flavor.
 *
 * \param obj is the object to mark
 */
void object_know_all_but_flavor(struct object *obj)
{
	int i;

	assert(obj->known);
	/* Make sure the tval, sval and kind are known */
	obj->known->tval = obj->tval;
	obj->known->sval = obj->sval;
	obj->known->kind = obj->kind;

	/* Know all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		obj->known->modifiers[i] = 1;

	/* Know all flags there are to be known */
	object_know_all_flags(obj);

	/* Know all elemental properties */
	object_know_all_elements(obj);

	/* Know everything else */
	object_know_all_miscellaneous(obj);
}



/**
 * Mark as object as fully known, a.k.a identified. 
 *
 * \param obj is the object to mark as identified
 */
void object_notice_everything(struct object *obj)
{
	assert(obj->known);
	/* Make sure the tval, sval and kind are known */
	obj->known->tval = obj->tval;
	obj->known->sval = obj->sval;
	obj->known->kind = obj->kind;

	/* Mark as known */
	object_flavor_aware(obj);

	/* Artifact has now been seen */
	if (obj->artifact && !(obj->known->notice & OBJ_NOTICE_IMAGINED)) {
		obj->artifact->seen = obj->artifact->everseen = true;
		history_add_artifact(obj->artifact, true, true);
	}

	/* Know everything else */
	obj->known->notice |= OBJ_NOTICE_SENSED;
	object_know_all_but_flavor(obj);
	if (!player->is_dead)
		apply_autoinscription(obj);
}



/**
 * Notice a set of flags - returns true if anything new was learned
 */
bool object_notice_flags(struct object *obj, bitflag flags[OF_SIZE])
{
	assert(obj->known);
	if (of_is_subset(obj->known->flags, flags))
		return false;

	of_union(obj->known->flags, flags);
	object_check_for_ident(obj);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);

	return true;
}

/**
 * Notice curses on an object.
 *
 * \param obj is the object to notice curses on
 */
bool object_notice_curses(struct object *obj)
{
	bitflag f[OF_SIZE], f2[OF_SIZE];

	assert(obj->known);
	object_flags(obj, f);

	/* Gather whatever curse flags there are to know */
	create_mask(f2, false, OFT_CURSE, OFT_MAX);

	/* Remove everything except the curse flags */
	of_inter(f, f2);

	/* Give knowledge of which curses are present */
	object_notice_flags(obj, f);

	object_check_for_ident(obj);

	player->upkeep->notice |= PN_IGNORE;

	return !of_is_empty(f);
}


/**
 * Notice object properties that become obvious on wielding or wearing
 */
void object_notice_on_wield(struct object *obj)
{
	bitflag f[OF_SIZE], f2[OF_SIZE], obvious_mask[OF_SIZE];
	bool obvious = false;
	int i;

	assert(obj->known);
	/* Always set the worn flag, and know arnour class */
	obj->known->notice |= OBJ_NOTICE_WORN;
	obj->known->ac = 1;

	/* EASY_KNOW jewelry is now known */
	if (object_flavor_is_aware(obj) && easy_know(obj)) {
		object_notice_everything(obj);
		return;
	}

	/* Only deal with un-ID'd items */
	if (object_is_known(obj)) return;

	/* Worn means tried (for flavored wearables) */
	object_flavor_tried(obj);

	/* Save time of wield for later */
	object_last_wield = turn;

	/* Get the obvious object flags */
	create_mask(obvious_mask, true, OFID_WIELD, OFT_MAX);

	/* special case FA, needed for mages wielding gloves */
	if (player_has(player, PF_CUMBER_GLOVE) && obj->tval == TV_GLOVES &&
		(obj->modifiers[OBJ_MOD_DEX] <= 0) && 
		!kf_has(obj->kind->kind_flags, KF_SPELLS_OK))
		of_on(obvious_mask, OF_FREE_ACT);

	/* Extract the flags */
	object_flags(obj, f);

	/* Find obvious flags - curses left for special message later */
	create_mask(f2, false, OFT_CURSE, OFT_MAX);
	of_diff(obvious_mask, f2);

	/* Learn about obvious flags */
	if (of_is_inter(f, obvious_mask)) obvious = true;
	of_union(obj->known->flags, obvious_mask);

	/* Notice all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (obj->modifiers[i]) obvious = true;
		obj->known->modifiers[i] = 1;
	}

	/* Automatically sense artifacts upon wield */
	object_sense_artifact(obj);

	/* Note artifacts when found */
	if (obj->artifact)
		history_add_artifact(obj->artifact, object_is_known(obj), true);

	/* Special cases for jewellery (because the flavor isn't the whole story) */
	if (tval_is_jewelry(obj)) {
		/* Learn the flavor of jewelry with obvious flags */
		if (obvious) {
			object_flavor_aware(obj);
			apply_autoinscription(obj);
		}

		/* Learn all flags and elements on any aware non-artifact jewelry */
		if (object_flavor_is_aware(obj) && !obj->artifact) {
			object_know_all_flags(obj);
			object_know_all_elements(obj);
		}
	}

	object_check_for_ident(obj);

	if (!obvious) return;

	/* Special messages for individual properties */
	if (obj->modifiers[OBJ_MOD_STR] > 0)
		msg("You feel stronger!");
	else if (obj->modifiers[OBJ_MOD_STR] < 0)
		msg("You feel weaker!");
	if (obj->modifiers[OBJ_MOD_INT] > 0)
		msg("You feel smarter!");
	else if (obj->modifiers[OBJ_MOD_INT] < 0)
		msg("You feel more stupid!");
	if (obj->modifiers[OBJ_MOD_WIS] > 0)
		msg("You feel wiser!");
	else if (obj->modifiers[OBJ_MOD_WIS] < 0)
		msg("You feel more naive!");
	if (obj->modifiers[OBJ_MOD_DEX] > 0)
		msg("You feel more dextrous!");
	else if (obj->modifiers[OBJ_MOD_DEX] < 0)
		msg("You feel clumsier!");
	if (obj->modifiers[OBJ_MOD_CON] > 0)
		msg("You feel healthier!");
	else if (obj->modifiers[OBJ_MOD_CON] < 0)
		msg("You feel sicklier!");
	if (obj->modifiers[OBJ_MOD_STEALTH] > 0)
		msg("You feel stealthier.");
	else if (obj->modifiers[OBJ_MOD_STEALTH] < 0)
		msg("You feel noisier.");
	if (obj->modifiers[OBJ_MOD_SPEED] > 0)
		msg("You feel strangely quick.");
	else if (obj->modifiers[OBJ_MOD_SPEED] < 0)
		msg("You feel strangely sluggish.");
	if (obj->modifiers[OBJ_MOD_BLOWS] > 0)
		msg("Your weapon tingles in your hands.");
	else if (obj->modifiers[OBJ_MOD_BLOWS] < 0)
		msg("Your weapon aches in your hands.");
	if (obj->modifiers[OBJ_MOD_SHOTS] > 0)
		msg("Your bow tingles in your hands.");
	else if (obj->modifiers[OBJ_MOD_SHOTS] < 0)
		msg("Your bow aches in your hands.");
	if (obj->modifiers[OBJ_MOD_INFRA] > 0)
		msg("Your eyes tingle.");
	else if (obj->modifiers[OBJ_MOD_INFRA] < 0)
		msg("Your eyes tingle.");
	if (obj->modifiers[OBJ_MOD_LIGHT])
		msg("It glows!");
	if (of_has(f, OF_TELEPATHY))
		msg("Your mind feels strangely sharper!");
	if (of_has(f, OF_FREE_ACT) && of_has(obvious_mask, OF_FREE_ACT))
		msg("You feel mobile!");

	/* Remember the flags */
	object_notice_sensing(obj);
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
	if (tval_is_rod(obj))
		object_notice_everything(obj);
	player_exp_gain(player, (lev + (player->lev / 2)) / player->lev);

	player->upkeep->notice |= PN_IGNORE;
}

/**
 * ------------------------------------------------------------------------
 * Other functions that either need a rethink, or will change in the move
 * to "rune-based" ID
 * ------------------------------------------------------------------------ */

/**
 * \returns whether it is possible an object has a high resist given the
 *          player's current knowledge
 */
bool object_high_resist_is_possible(const struct object *obj)
{
	size_t i;

	assert(obj->known);
	/* Look at all the high resists */
	for (i = ELEM_HIGH_MIN; i <= ELEM_HIGH_MAX; i++) {
		/* Object has the resist */
		if (obj->el_info[i].res_level > 0) return true;

		/* Element properties unknown */
		if (!object_element_is_known(obj, i)) return true;
	}

	/* No possibilities */
	return false;
}


/**
 * \returns whether the object has been sensed with pseudo-ID
 */
bool object_was_sensed(const struct object *obj)
{
	if (!obj->known) return false;
	return obj->known->notice & OBJ_NOTICE_SENSED ? true : false;
}

/**
 * Mark an object as sensed (kind of).
 */
void object_notice_sensing(struct object *obj)
{
	assert(obj->known);
	if (object_was_sensed(obj))
		return;

	if (obj->artifact) {
		obj->artifact->seen = obj->artifact->everseen = true;
		history_add_artifact(obj->artifact, object_is_known(obj), true);
	}

	object_notice_curses(obj);
	obj->known->notice |= OBJ_NOTICE_SENSED;
	object_check_for_ident(obj);

	/* For unflavoured objects we can rule out some things */
	if (!obj->artifact && !obj->ego && !obj->kind->flavor) {
		object_know_all_flags(obj);
		object_know_all_elements(obj);
	}
}


/**
 * Sense artifacts
 */
void object_sense_artifact(struct object *obj)
{
	assert(obj->known);
	obj->known->artifact = (struct artifact *)1;
	if (obj->artifact)
		object_notice_sensing(obj);
}


/**
 * Given an object, return a short identifier which gives some idea of what
 * the item is.
 */
obj_pseudo_t object_pseudo(const struct object *obj)
{
	int i;
	bitflag flags[OF_SIZE], f2[OF_SIZE];

	assert(obj->known);
	/* Get the known and obvious flags on the object,
	 * not including curses or properties of the kind.
	 */
	object_flags_known(obj, flags);
	create_mask(f2, true, OFID_WIELD, OFT_MAX);

	/* FA on gloves is obvious to mage casters */
	if (player_has(player, PF_CUMBER_GLOVE) && obj->tval == TV_GLOVES &&
		(obj->modifiers[OBJ_MOD_DEX] <= 0) && 
		!kf_has(obj->kind->kind_flags, KF_SPELLS_OK))
		of_on(f2, OF_FREE_ACT);

	/* Now we remove the non-obvious known flags */
	of_inter(flags, f2);

	/* Now we remove the cursed flags and the kind flags */
	create_mask(f2, false, OFT_CURSE, OFT_MAX);
	of_diff(flags, f2);
	of_diff(flags, obj->kind->flags);

	if ((object_was_sensed(obj) || object_was_worn(obj)) && obj->artifact)
		return INSCRIP_SPECIAL;

	/* jewelry does not pseudo */
	if (tval_is_jewelry(obj))
		return INSCRIP_NULL;

	/* Check modifiers for splendid - anything different from kind base
	 * modifier is splendid (in fact, kind base modifiers are currently
	 * constant for all relevant objects) */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if ((obj->modifiers[i] != obj->kind->modifiers[i].base) &&
			object_this_mod_is_visible(obj, i))
			return INSCRIP_SPLENDID;

	/* Any remaining obvious-on-wield flags also mean splendid */
	if (!of_is_empty(flags))
		return INSCRIP_SPLENDID;

	/* Known brands are also splendid */
	if (obj->known->brands)
		return INSCRIP_SPLENDID;

	if (!object_is_known(obj) && !object_was_sensed(obj))
		return INSCRIP_NULL;

	if (obj->ego) {
		/* Uncursed bad egos are not excellent */
		if (of_is_inter(obj->ego->flags, f2))
			return INSCRIP_STRANGE; /* Need something worse - post 4.0 NRM*/
		else
			return INSCRIP_EXCELLENT;
	}

	if (obj->to_a == randcalc(obj->kind->to_a, 0, MINIMISE) &&
	    obj->to_h == randcalc(obj->kind->to_h, 0, MINIMISE) &&
		 obj->to_d == randcalc(obj->kind->to_d, 0, MINIMISE))
		return INSCRIP_AVERAGE;

	if (obj->to_a >= randcalc(obj->kind->to_a, 0, MINIMISE) &&
	    obj->to_h >= randcalc(obj->kind->to_h, 0, MINIMISE) &&
	    obj->to_d >= randcalc(obj->kind->to_d, 0, MINIMISE))
		return INSCRIP_MAGICAL;

	if (obj->to_a <= randcalc(obj->kind->to_a, 0, MINIMISE) &&
	    obj->to_h <= randcalc(obj->kind->to_h, 0, MINIMISE) &&
	    obj->to_d <= randcalc(obj->kind->to_d, 0, MINIMISE))
		return INSCRIP_MAGICAL;

	return INSCRIP_STRANGE;
}


/**
 * Identify an item.
 */
void do_ident_item(struct object *obj)
{
	char o_name[80];

	u32b msg_type = 0;
	int i, slot;
	bool bad = true;

	assert(obj->known);
    /* Identify and apply autoinscriptions. */
	object_flavor_aware(obj);
	object_notice_everything(obj);
	apply_autoinscription(obj);

	/* Update the gear */
	calc_inventory(player->upkeep, player->gear, player->body);

	/* Set ignore flag */
	player->upkeep->notice |= PN_IGNORE;

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Description */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

	/* Determine the message type. */
	/* CC: we need to think more carefully about how we define "bad" with
	 * multiple modifiers - currently using "all nonzero modifiers < 0" */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->modifiers[i] > 0)
			bad = false;

	if (bad)
		msg_type = MSG_IDENT_BAD;
	else if (obj->artifact)
		msg_type = MSG_IDENT_ART;
	else if (obj->ego)
		msg_type = MSG_IDENT_EGO;
	else
		msg_type = MSG_GENERIC;

	/* Log artifacts to the history list. */
	if (obj->artifact)
		history_add_artifact(obj->artifact, true, true);

	/* Describe */
	slot = equipped_item_slot(player->body, obj);
	if (object_is_equipped(player->body, obj)) {
		/* Format and capitalise */
		char *msg = format("%s: %s (%c).", equip_describe(player, slot),
						   o_name, I2A(slot));
		my_strcap(msg);

		msgt(msg_type, msg);
	} else if (object_is_carried(player, obj))
		msgt(msg_type, "In your pack: %s (%c).", o_name, gear_to_label(obj));
	else
		msgt(msg_type, "On the ground: %s.", o_name);
}

/**
 * Sense the inventory
 */
void sense_inventory(void)
{
	struct object *obj;
	char o_name[80];
	unsigned int rate;
	
	/* No ID when confused in a bad state */
	if (player->timed[TMD_CONFUSED]) return;

	/* Notice some things after a while */
	if (turn >= (object_last_wield + 3000)) {
		equip_learn_after_time(player);
		object_last_wield = 0;
	}

	/* Get improvement rate */
	if (player_has(player, PF_PSEUDO_ID_IMPROV))
		rate = player->class->sense_base /
			(player->lev * player->lev + player->class->sense_div);
	else
		rate = player->class->sense_base /
			(player->lev + player->class->sense_div);

	/* Check if player may sense anything this time */
	if (player->lev < 20 && !one_in_(rate)) return;

	/* Give each object one opportunity to have a chance at being sensed. */
	for (obj = player->gear; obj; obj = obj->next) {
		const char *text = NULL;
		obj_pseudo_t feel;
		bool cursed;
		bool equipped = object_is_equipped(player->body, obj);

		assert(obj->known);
		/* Valid tval codes only */
		if (!tval_is_weapon(obj) && !tval_is_armor(obj)) continue;
		
		/* It is known, no information needed */
		if (object_is_known(obj)) continue;

		/* It has already been sensed, do not sense it again */
		if (object_was_sensed(obj)) {
			/* Small chance of wielded, sensed items getting complete ID */
			if (!obj->artifact && equipped && one_in_(1000))
				do_ident_item(obj);

			continue;
		}

		/* Occasional failure on inventory items */
		if (!equipped && one_in_(5)) continue;

		/* Sense the object */
		object_notice_sensing(obj);
		cursed = object_notice_curses(obj);

		/* Get the feeling */
		feel = object_pseudo(obj);

		/* Stop everything */
		disturb(player, 0);

		if (cursed)
			text = "cursed";
		else
			text = inscrip_text[feel];

		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

		/* Average pseudo-ID means full ID */
		if (feel == INSCRIP_AVERAGE) {
			object_notice_everything(obj);

			msgt(MSG_PSEUDOID, "You feel the %s (%c) %s %s average...",
				 o_name, gear_to_label(obj),
				 equipped ? "you are using" : "in your pack",
				 VERB_AGREEMENT(obj->number, "is", "are"));
		} else {
			if (equipped) {
				msgt(MSG_PSEUDOID, "You feel the %s (%c) you are %s %s %s...",
					 o_name, gear_to_label(obj),
					 equip_describe(player,
									equipped_item_slot(player->body, obj)),
					 VERB_AGREEMENT(obj->number, "is", "are"), text);
			} else {
				msgt(MSG_PSEUDOID, "You feel the %s (%c) in your pack %s %s...",
					 o_name, gear_to_label(obj),
					 VERB_AGREEMENT(obj->number, "is", "are"), text);
			}
		}

		/* Set ignore flag as appropriate */
		if (!equipped)
			player->upkeep->notice |= PN_IGNORE;
		
		/* Update the gear */
		player->upkeep->update |= (PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);
		
		/* Redraw stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}
}
