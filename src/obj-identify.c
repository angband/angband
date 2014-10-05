/**
   \file obj-identify.c
   \brief Object identification and knowledge routines
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
#include "dungeon.h"
#include "game-event.h"
#include "history.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "player-util.h"

/** Time last item was wielded */
s32b object_last_wield;

/*** Knowledge accessor functions ***/


/**
 * \returns whether an object counts as "known" due to EASY_KNOW status
 */
bool easy_know(const object_type *o_ptr)
{
	if (o_ptr->kind->aware && kf_has(o_ptr->kind->kind_flags, KF_EASY_KNOW))
		return TRUE;
	else
		return FALSE;
}

/**
 * Is the player aware of all of an object's flags?
 *
 * \param o_ptr is the object
 */
bool object_all_flags_are_known(const object_type *o_ptr)
{
	return of_is_full(o_ptr->known_flags) ? TRUE : FALSE;
}


/**
 * Is the player aware of all of an object's elemental properties?
 *
 * \param o_ptr is the object
 */
bool object_all_elements_are_known(const object_type *o_ptr)
{
	size_t i;

	for (i = 0; i < ELEM_MAX; i++)
		if (!(o_ptr->el_info[i].flags & EL_INFO_KNOWN)) return FALSE;

	return TRUE;
}


/**
 * Is the player aware of all of an object's brands and slays?
 *
 * \param o_ptr is the object
 */
bool object_all_brands_and_slays_are_known(const object_type *o_ptr)
{
	struct brand *b;
	struct slay *s;

	for (b = o_ptr->brands; b; b = b->next)
		if (!b->known) return FALSE;
	for (s = o_ptr->slays; s; s = s->next)
		if (!s->known) return FALSE;

	return TRUE;
}

/**
 * Is the player aware of all of an object's miscellaneous proerties?
 *
 * \param o_ptr is the object
 */
bool object_all_miscellaneous_are_known(const object_type *o_ptr)
{
	return id_is_full(o_ptr->id_flags) ? TRUE : FALSE;
}

/**
 * Is the player aware of all of an object's miscellaneous proerties?
 *
 * \param o_ptr is the object
 */
bool object_all_but_flavor_is_known(const object_type *o_ptr)
{
	if (!object_all_flags_are_known(o_ptr)) return FALSE;
	if (!object_all_elements_are_known(o_ptr)) return FALSE;
	if (!object_all_brands_and_slays_are_known(o_ptr)) return FALSE;
	if (!object_all_miscellaneous_are_known(o_ptr)) return FALSE;
	return TRUE;
}

/**
 * \returns whether an object should be treated as fully known (e.g. ID'd)
 */
bool object_is_known(const object_type *o_ptr)
{
	if (!object_flavor_is_aware(o_ptr)) return FALSE;
	return object_all_but_flavor_is_known(o_ptr) ? TRUE : FALSE;
}

/**
 * \returns whether the object is known to be an artifact
 */
bool object_is_known_artifact(const object_type *o_ptr)
{
	return (o_ptr->artifact && id_has(o_ptr->id_flags, ID_ARTIFACT));
}

/**
 * \returns whether the object is known to be blessed
 */
bool object_is_known_blessed(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];

	object_flags_known(o_ptr, f);

	return (of_has(f, OF_BLESSED)) ? TRUE : FALSE;
}

/**
 * \returns whether the object is known to not be an artifact
 */
bool object_is_known_not_artifact(const object_type *o_ptr)
{
	return (!(o_ptr->artifact) && id_has(o_ptr->id_flags, ID_ARTIFACT));
}

/**
 * \returns whether the object has been worn/wielded
 */
bool object_was_worn(const object_type *o_ptr)
{
	/* A hack, OK for now as ID_STR is only gained on wield or identify - NRM */
	return id_has(o_ptr->id_flags, ID_STR) ? TRUE : FALSE;
}

/**
 * \returns whether the player is aware of the object's flavour
 */
bool object_flavor_is_aware(const object_type *o_ptr)
{
	assert(o_ptr->kind);
	return o_ptr->kind->aware;
}

/**
 * \returns whether the player has tried to use other objects of the same kind
 */
bool object_flavor_was_tried(const object_type *o_ptr)
{
	assert(o_ptr->kind);
	return o_ptr->kind->tried;
}

/**
 * \returns whether the player is aware of the object's effect when used
 */
bool object_effect_is_known(const object_type *o_ptr)
{
	assert(o_ptr->kind);
	if (easy_know(o_ptr) || id_has(o_ptr->id_flags, ID_EFFECT))
		return TRUE;

	return FALSE;
}

/**
 * \returns whether any ego or artifact name is available to the player
 */
bool object_name_is_visible(const object_type *o_ptr)
{
	bool ego = o_ptr->ego && id_has(o_ptr->id_flags, ID_EGO_ITEM);
	bool art = o_ptr->artifact && id_has(o_ptr->id_flags, ID_ARTIFACT);
	return (ego || art) ? TRUE : FALSE;
}

/**
 * \returns whether both the object is both an ego and the player knows it is
 */
bool object_ego_is_visible(const object_type *o_ptr)
{
	if (o_ptr->ego && id_has(o_ptr->id_flags, ID_EGO_ITEM))
		return TRUE;

	return FALSE;
}

/**
 * \returns whether the object's attack plusses are known
 */
bool object_attack_plusses_are_visible(const object_type *o_ptr)
{
	/* Bonuses have been revealed or for sale */
	if (id_has(o_ptr->id_flags, ID_TO_H) && id_has(o_ptr->id_flags, ID_TO_D))
		return TRUE;

	return FALSE;
}

/**
 * \returns whether the object's defence bonuses are known
 */
bool object_defence_plusses_are_visible(const object_type *o_ptr)
{
	/* Bonuses have been revealed or for sale */
	if (id_has(o_ptr->id_flags, ID_TO_A))
		return TRUE;

	return FALSE;
}


/**
 * \returns whether the player knows whether an object has a given flag
 */
bool object_flag_is_known(const object_type *o_ptr, int flag)
{
	if (easy_know(o_ptr) || of_has(o_ptr->known_flags, flag))
		return TRUE;

	return FALSE;
}

/**
 * \returns whether the player knows the given element properties of an object
 */
bool object_element_is_known(const object_type *o_ptr, int element)
{
	if (element < 0 || element >= ELEM_MAX) return FALSE;

	if (easy_know(o_ptr) || (o_ptr->el_info[element].flags & EL_INFO_KNOWN))
		return TRUE;

	return FALSE;
}


/**
 * \returns whether a specific modifier is known to the player
 */
bool object_this_mod_is_visible(const object_type *o_ptr, int mod)
{
	assert(o_ptr->kind);

	if (id_has(o_ptr->id_flags, ID_MOD_MIN + mod))
		return TRUE;

	return FALSE;
}



/**
 * Sets an ID_ flag on an object.
 *
 * \param o_ptr is the object to check
 * \param flag is the id flag to be added
 *
 * \returns whether o_ptr->id_flag changed
 */
static bool object_add_id_flag(object_type *o_ptr, int flag)
{
	if (id_has(o_ptr->id_flags, flag))
		return FALSE;

	id_on(o_ptr->id_flags, flag);

	return TRUE;
}


/**
 * Checks for additional knowledge implied by what the player already knows.
 *
 * \param o_ptr is the object to check
 *
 * returns whether it calls object_notice_everyting
 */
bool object_check_for_ident(object_type *o_ptr)
{
	/* Check things which need to be learned */
	if (!object_all_flags_are_known(o_ptr)) return FALSE;
	if (!object_all_elements_are_known(o_ptr)) return FALSE;
	if (!object_all_brands_and_slays_are_known(o_ptr)) return FALSE;

	/* If we know attack bonuses, and defence bonuses, and effect, then
	 * we effectively know everything, so mark as such */
	if ((object_attack_plusses_are_visible(o_ptr) ||
		 (object_was_sensed(o_ptr) && o_ptr->to_h == 0 && o_ptr->to_d == 0)) &&
	    (object_defence_plusses_are_visible(o_ptr) ||
		 (object_was_sensed(o_ptr) && o_ptr->to_a == 0)) &&
	    (object_effect_is_known(o_ptr) || !object_effect(o_ptr)))
	{
		/* In addition to knowing the pval flags, it is necessary to know 
		 * the modifiers to know everything */
		int i;
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (!object_this_mod_is_visible(o_ptr, i))
				break;
		if (i == OBJ_MOD_MAX) {
			object_notice_everything(o_ptr);
			return TRUE;
		}
	}

	/* We still know all the flags, so we still know if it's an ego */
	if (o_ptr->ego)
		object_notice_ego(o_ptr);

	return FALSE;
}


/**
 * Mark an object's flavour as as one the player is aware of.
 *
 * \param o_ptr is the object whose flavour should be marked as aware
 */
void object_flavor_aware(object_type *o_ptr)
{
	int i;

	if (o_ptr->kind->aware) return;
	o_ptr->kind->aware = TRUE;

	/* Charges or food value (pval) and effect now known */
	id_on(o_ptr->id_flags, ID_PVAL);
	id_on(o_ptr->id_flags, ID_EFFECT);

	/* Jewelry with fixed bonuses gets more info now */
	if (tval_is_jewelry(o_ptr)) {
		if (!randcalc_varies(o_ptr->kind->to_h)) 
			id_on(o_ptr->id_flags, ID_TO_H);
		if (!randcalc_varies(o_ptr->kind->to_d))
			id_on(o_ptr->id_flags, ID_TO_D);
		if (!randcalc_varies(o_ptr->kind->to_a))
			id_on(o_ptr->id_flags, ID_TO_A);
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (!randcalc_varies(o_ptr->kind->modifiers[i]))
				id_on(o_ptr->id_flags, ID_MOD_MIN + i);
	}

	/* Fix ignore/autoinscribe */
	if (kind_is_ignored_unaware(o_ptr->kind)) {
		kind_ignore_when_aware(o_ptr->kind);
	}
	player->upkeep->notice |= PN_IGNORE;
	apply_autoinscription(o_ptr);

	/* Quit if no dungeon yet */
	if (!cave) return;

	/* Some objects change tile on awareness */
	/* So update display for all floor objects of this kind */
	for (i = 1; i < cave_object_max(cave); i++)
	{
		const object_type *floor_o_ptr = cave_object(cave, i);

		if (!floor_o_ptr->held_m_idx &&	floor_o_ptr->kind == o_ptr->kind)
			square_light_spot(cave, floor_o_ptr->iy, floor_o_ptr->ix);
	}
}


/**
 * Mark an object's flavour as tried.
 *
 * \param o_ptr is the object whose flavour should be marked
 */
void object_flavor_tried(object_type *o_ptr)
{
	assert(o_ptr);
	assert(o_ptr->kind);

	o_ptr->kind->tried = TRUE;
}

/**
 * Make the player aware of all of an object's flags.
 *
 * \param o_ptr is the object to mark
 */
void object_know_all_flags(object_type *o_ptr)
{
	of_setall(o_ptr->known_flags);
}


/**
 * Make the player aware of all of an object's elemental properties.
 *
 * \param o_ptr is the object to mark
 */
void object_know_all_elements(object_type *o_ptr)
{
	size_t i;

	for (i = 0; i < ELEM_MAX; i++)
		o_ptr->el_info[i].flags |= EL_INFO_KNOWN;
}


/**
 * Make the player aware of all of an object's brands and slays.
 *
 * \param o_ptr is the object to mark
 */
void object_know_brands_and_slays(object_type *o_ptr)
{
	struct brand *b;
	struct slay *s;

	for (b = o_ptr->brands; b; b = b->next)
		b->known = TRUE;
	for (s = o_ptr->slays; s; s = s->next)
		s->known = TRUE;
}

/**
 * Make the player aware of all of an object's miscellaneous properties.
 *
 * \param o_ptr is the object to mark
 */
void object_know_all_miscellaneous(object_type *o_ptr)
{
	id_setall(o_ptr->id_flags);
}

/**
 * Make the player aware of all of an object' properties except flavor.
 *
 * \param o_ptr is the object to mark
 */
void object_know_all_but_flavor(object_type *o_ptr)
{
	/* Know all flags there are to be known */
	object_know_all_flags(o_ptr);

	/* Know all elemental properties */
	object_know_all_elements(o_ptr);

	/* Know all brands and slays */
	object_know_brands_and_slays(o_ptr);

	/* Know everything else */
	object_know_all_miscellaneous(o_ptr);
}



/**
 * Mark as object as fully known, a.k.a identified. 
 *
 * \param o_ptr is the object to mark as identified
 */
void object_notice_everything(object_type *o_ptr)
{
	/* Mark as known */
	object_flavor_aware(o_ptr);

	/* Artifact has now been seen */
	if (o_ptr->artifact) {
		o_ptr->artifact->seen = o_ptr->artifact->everseen = TRUE;
		history_add_artifact(o_ptr->artifact, TRUE, TRUE);
	}

	/* Know everything else */
	object_know_all_but_flavor(o_ptr);
}



/**
 * Notice the ego on an ego item.
 */
void object_notice_ego(object_type *o_ptr)
{
	bitflag learned_flags[OF_SIZE];
	bitflag xtra_flags[OF_SIZE];
	size_t i;

	if (!o_ptr->ego)
		return;

	/* Learn ego flags */
	of_union(o_ptr->known_flags, o_ptr->ego->flags);

	/* Learn ego element properties */
	for (i = 0; i < ELEM_MAX; i++)
		if (o_ptr->ego->el_info[i].res_level != 0)
			o_ptr->el_info[i].flags |= EL_INFO_KNOWN;

	/* Learn all flags except random abilities */
	of_setall(learned_flags);

	/* Random ego extras */
	if (kf_has(o_ptr->ego->kind_flags, KF_RAND_SUSTAIN)) {
		create_mask(xtra_flags, FALSE, OFT_SUST, OFT_MAX);
		of_diff(learned_flags, xtra_flags);
	} else if (kf_has(o_ptr->ego->kind_flags, KF_RAND_POWER)) {
		create_mask(xtra_flags, FALSE, OFT_MISC, OFT_PROT, OFT_MAX);
		of_diff(learned_flags, xtra_flags);
	} else if (kf_has(o_ptr->ego->kind_flags, KF_RAND_HI_RES)) {
		for (i = ELEM_HIGH_MIN; i <= ELEM_HIGH_MAX; i++)
			if ((o_ptr->ego->el_info[i].res_level == 1) &&
				!(o_ptr->el_info[i].flags & EL_INFO_RANDOM))
				o_ptr->el_info[i].flags |= EL_INFO_KNOWN;
	}

	of_union(o_ptr->known_flags, learned_flags);

	if (object_add_id_flag(o_ptr, ID_EGO_ITEM))
	{
		/* if you know the ego, you know which it is of excellent or splendid */
		object_notice_sensing(o_ptr);
		object_check_for_ident(o_ptr);
	}
}


/**
 * Notice the "effect" from activating an object.
 *
 * \param o_ptr is the object to become aware of
 */
void object_notice_effect(object_type *o_ptr)
{
	if (object_add_id_flag(o_ptr, ID_EFFECT))
		object_check_for_ident(o_ptr);

	/* noticing an effect gains awareness */
	if (!object_flavor_is_aware(o_ptr))
		object_flavor_aware(o_ptr);
}


static void object_notice_defence_plusses(struct player *p, object_type *o_ptr)
{
	assert(o_ptr && o_ptr->kind);

	if (object_defence_plusses_are_visible(o_ptr))
		return;

	if (object_add_id_flag(o_ptr, ID_TO_A)) 
		object_check_for_ident(o_ptr);

	if (o_ptr->ac || o_ptr->to_a) {
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		msgt(MSG_PSEUDOID, "You know more about the %s you are wearing.",
			 o_name);
	}

	p->upkeep->update |= (PU_BONUS);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


void object_notice_attack_plusses(object_type *o_ptr)
{
	bool to_hit = FALSE, to_dam = FALSE;
	char o_name[80];

	assert(o_ptr && o_ptr->kind);

	if (object_attack_plusses_are_visible(o_ptr)) return;

	/* This looks silly while these only ever appear together */
	to_hit = object_add_id_flag(o_ptr, ID_TO_H);
	to_dam = object_add_id_flag(o_ptr, ID_TO_D);
	if (object_add_id_flag(o_ptr, ID_DICE) || to_hit || to_dam)
		object_check_for_ident(o_ptr);

	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	if (equipped_item_by_slot_name(player, "weapon") == o_ptr)
		msgt(MSG_PSEUDOID,
			 "You know more about the %s you are using.", o_name);
	else if ((o_ptr->to_d || o_ptr->to_h) &&
			 !(tval_is_body_armor(o_ptr) && (o_ptr->to_h < 0)))
		msgt(MSG_PSEUDOID, "Your %s glow%s.", o_name,
			 ((o_ptr->number > 1) ? "" : "s"));

	player->upkeep->update |= (PU_BONUS);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/**
 * Notice elemental resistance properties for an element on an object
 */
bool object_notice_element(object_type *o_ptr, int element)
{
	if (element < 0 || element >= ELEM_MAX) return FALSE;

	/* Already known */
	if (o_ptr->el_info[element].flags & EL_INFO_KNOWN)
		return FALSE;

	/* Learn about this element */
	o_ptr->el_info[element].flags |= EL_INFO_KNOWN;

	object_check_for_ident(o_ptr);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);

	return TRUE;
}


/**
 * Notice a single flag - returns TRUE if anything new was learned
 */
bool object_notice_flag(object_type *o_ptr, int flag)
{
	if (of_has(o_ptr->known_flags, flag))
		return FALSE;

	of_on(o_ptr->known_flags, flag);
	object_check_for_ident(o_ptr);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);

	return TRUE;
}


/**
 * Notice a set of flags - returns TRUE if anything new was learned
 */
bool object_notice_flags(object_type *o_ptr, bitflag flags[OF_SIZE])
{
	if (of_is_subset(o_ptr->known_flags, flags))
		return FALSE;

	of_union(o_ptr->known_flags, flags);
	object_check_for_ident(o_ptr);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);

	return TRUE;
}


/**
 * Notice curses on an object.
 *
 * \param o_ptr is the object to notice curses on
 */
bool object_notice_curses(object_type *o_ptr)
{
	bitflag f[OF_SIZE], f2[OF_SIZE];

	object_flags(o_ptr, f);

	/* Gather whatever curse flags there are to know */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);

	/* Remove everything except the curse flags */
	of_inter(f, f2);

	/* give knowledge of which curses are present */
	object_notice_flags(o_ptr, f);

	object_check_for_ident(o_ptr);

	player->upkeep->notice |= PN_IGNORE;

	return !of_is_empty(f);
}


/**
 * Notice things which happen on defending.
 */
void object_notice_on_defend(struct player *p)
{
	int i;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = equipped_item_by_slot(p, i);
		if (obj && obj->kind)
			object_notice_defence_plusses(p, obj);
	}

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/**
 * Notice object properties that become obvious on wielding or wearing
 */
void object_notice_on_wield(object_type *o_ptr)
{
	bitflag f[OF_SIZE], f2[OF_SIZE], obvious_mask[OF_SIZE];
	bool obvious = FALSE;
	int i;

	/* Only deal with un-ID'd items */
	if (object_is_known(o_ptr)) return;

	/* EASY_KNOW jewelry is now known */
	if (object_flavor_is_aware(o_ptr) && easy_know(o_ptr))
	{
		object_notice_everything(o_ptr);
		return;
	}

	/* Worn means tried (for flavored wearables) */
	object_flavor_tried(o_ptr);

	/* Save time of wield for later */
	object_last_wield = turn;

	/* Get the obvious object flags */
	create_mask(obvious_mask, TRUE, OFID_WIELD, OFT_MAX);

	/* special case FA, needed for mages wielding gloves */
	if (player_has(PF_CUMBER_GLOVE) && o_ptr->tval == TV_GLOVES &&
		(o_ptr->modifiers[OBJ_MOD_DEX] <= 0) && 
		!kf_has(o_ptr->kind->kind_flags, KF_SPELLS_OK))
		of_on(obvious_mask, OF_FREE_ACT);

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Find obvious flags - curses left for special message later */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);
	of_diff(obvious_mask, f2);

	/* Learn about obvious flags */
	if (of_is_inter(f, obvious_mask)) obvious = TRUE;
	of_union(o_ptr->known_flags, obvious_mask);

	/* Notice all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (o_ptr->modifiers[i]) obvious = TRUE;
		id_on(o_ptr->id_flags, i + ID_MOD_MIN);
	}

	/* Notice any brands */
	object_notice_brands(o_ptr, NULL);

	/* Automatically sense artifacts upon wield */
	object_sense_artifact(o_ptr);

	/* Note artifacts when found */
	if (o_ptr->artifact)
		history_add_artifact(o_ptr->artifact, object_is_known(o_ptr), TRUE);

	/* Special cases for jewellery (because the flavor isn't the whole story) */
	if (tval_is_jewelry(o_ptr))
	{
		/* Learn the flavor of jewelry with obvious flags */
		if (obvious)
			object_flavor_aware(o_ptr);

		/* Learn all flags and elements on any aware non-artifact jewelry */
		if (object_flavor_is_aware(o_ptr) && !o_ptr->artifact) {
			object_know_all_flags(o_ptr);
			object_know_all_elements(o_ptr);
		}
	}

	object_check_for_ident(o_ptr);

	if (!obvious) return;

	/* Special messages for individual properties */
	if (o_ptr->modifiers[OBJ_MOD_STR] > 0)
		msg("You feel stronger!");
	else if (o_ptr->modifiers[OBJ_MOD_STR] < 0)
		msg("You feel weaker!");
	if (o_ptr->modifiers[OBJ_MOD_INT] > 0)
		msg("You feel smarter!");
	else if (o_ptr->modifiers[OBJ_MOD_INT] < 0)
		msg("You feel more stupid!");
	if (o_ptr->modifiers[OBJ_MOD_WIS] > 0)
		msg("You feel wiser!");
	else if (o_ptr->modifiers[OBJ_MOD_WIS] < 0)
		msg("You feel more naive!");
	if (o_ptr->modifiers[OBJ_MOD_DEX] > 0)
		msg("You feel more dextrous!");
	else if (o_ptr->modifiers[OBJ_MOD_DEX] < 0)
		msg("You feel clumsier!");
	if (o_ptr->modifiers[OBJ_MOD_CON] > 0)
		msg("You feel healthier!");
	else if (o_ptr->modifiers[OBJ_MOD_CON] < 0)
		msg("You feel sicklier!");
	if (o_ptr->modifiers[OBJ_MOD_STEALTH] > 0)
		msg("You feel stealthier.");
	else if (o_ptr->modifiers[OBJ_MOD_SPEED] < 0)
		msg("You feel noisier.");
	if (o_ptr->modifiers[OBJ_MOD_SPEED] > 0)
		msg("You feel strangely quick.");
	else if (o_ptr->modifiers[OBJ_MOD_SPEED] < 0)
		msg("You feel strangely sluggish.");
	if (o_ptr->modifiers[OBJ_MOD_BLOWS] > 0)
		msg("Your weapon tingles in your hands.");
	else if (o_ptr->modifiers[OBJ_MOD_BLOWS] < 0)
		msg("Your weapon aches in your hands.");
	if (o_ptr->modifiers[OBJ_MOD_SHOTS] > 0)
		msg("Your bow tingles in your hands.");
	else if (o_ptr->modifiers[OBJ_MOD_SHOTS] < 0)
		msg("Your bow aches in your hands.");
	if (o_ptr->modifiers[OBJ_MOD_INFRA])
		msg("Your eyes tingle.");
	if (o_ptr->modifiers[OBJ_MOD_LIGHT])
		msg("It glows!");
	if (of_has(f, OF_TELEPATHY))
		msg("Your mind feels strangely sharper!");
	if (of_has(f, OF_FREE_ACT))
		msg("You feel mobile!");

	/* Remember the flags */
	object_notice_sensing(o_ptr);
}


/**
 * Notice things about an object that would be noticed in time.
 */
static void object_notice_after_time(void)
{
	int i;
	int flag;

	object_type *o_ptr;
	char o_name[80];

	bitflag f[OF_SIZE], timed_mask[OF_SIZE];

	create_mask(timed_mask, TRUE, OFID_TIMED, OFT_MAX);

	/* Check every item the player is wearing */
	for (i = 0; i < player->body.count; i++)
	{
		o_ptr = equipped_item_by_slot(player, i);
		if (!o_ptr->kind) continue;

		if (object_is_known(o_ptr)) continue;

		/* Check for timed notice flags */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		object_flags(o_ptr, f);
		of_inter(f, timed_mask);

		for (flag = of_next(f, FLAG_START); flag != FLAG_END;
			 flag = of_next(f, flag + 1)) {
			if (!of_has(o_ptr->known_flags, flag)) {
				/* Message */
				flag_message(flag, o_name);

				/* Notice the flag */
				object_notice_flag(o_ptr, flag);

				if (tval_is_jewelry(o_ptr) &&
					 (!object_effect(o_ptr) || object_effect_is_known(o_ptr))) {
					/* Jewelry with a noticeable flag is obvious */
					object_flavor_aware(o_ptr);
					object_check_for_ident(o_ptr);
				}
			} else {
				/* Notice the flag is absent */
				object_notice_flag(o_ptr, flag);
			}
		}
	}
}


/**
 * Notice a given special flag on wielded items.
 *
 * \param flag is the flag to notice
 */
void wieldeds_notice_flag(struct player *p, int flag)
{
	int i;

	/* Sanity check */
	if (!flag) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		object_type *o_ptr = equipped_item_by_slot(p, i);

		if (of_has(o_ptr->flags, flag) && !of_has(o_ptr->known_flags, flag)) {
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

			/* Notice the flag */
			object_notice_flag(o_ptr, flag);

			/* Jewelry with a noticeable flag is obvious */
			if (tval_is_jewelry(o_ptr)) {
				object_flavor_aware(o_ptr);
				object_check_for_ident(o_ptr);
			}

			/* Message */
			flag_message(flag, o_name);
		} else {
			/* Notice that flag is absent */
			object_notice_flag(o_ptr, flag);
		}
	}

	return;
}

/**
 * Notice the elemental resistance properties on wielded items.
 *
 * \param element is the element to notice
 */
void wieldeds_notice_element(struct player *p, int element)
{
	int i;

	if (element < 0 || element >= ELEM_MAX) return;

	for (i = 0; i < p->body.count; i++) {
		object_type *o_ptr = equipped_item_by_slot(p, i);

		/* Already known */
		if (o_ptr->el_info[element].flags & EL_INFO_KNOWN) continue;

		/* Notice the element properties */
		object_notice_element(o_ptr, element);

		/* Comment if it actually does something */
		if (o_ptr->el_info[element].res_level != 0) {
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

			msg("Your %s glows", o_name);
		}

		/* Jewelry with a noticeable element is obvious */
		if (tval_is_jewelry(o_ptr)) {
			object_flavor_aware(o_ptr);
			object_check_for_ident(o_ptr);
		}
	}
}

/**
 * Notice to-hit bonus on attacking.
 *
 * Used e.g. for ranged attacks where the item's to_d is not involved.
 * Does not apply to weapon or bow which should be done separately
 */
void wieldeds_notice_to_hit_on_attack(void)
{
	int i;

	for (i = 0; i < player->body.count; i++) {
		struct object *obj = equipped_item_by_slot(player, i);
		if (i == slot_by_name(player, "weapon")) continue;
		if (i == slot_by_name(player, "bow")) continue;
		if (obj && obj->to_h)
			object_notice_attack_plusses(obj);
	}

	return;
}


/**
 * Notice things which happen on attacking.
 * Does not apply to weapon or bow which should be done separately
 */
void wieldeds_notice_on_attack(void)
{
	int i;

	for (i = 0; i < player->body.count; i++) {
		struct object *obj = equipped_item_by_slot(player, i);
		if (i == slot_by_name(player, "weapon")) continue;
		if (i == slot_by_name(player, "bow")) continue;
		if (obj->kind)
			object_notice_attack_plusses(obj);
	}

	return;
}


/* Ostracism line */

/**
 * \returns whether it is possible an object has a high resist given the
 *          player's current knowledge
 */
bool object_high_resist_is_possible(const object_type *o_ptr)
{
	size_t i;

	/* Look at all the high resists */
	for (i = ELEM_HIGH_MIN; i <= ELEM_HIGH_MAX; i++) {
		/* Object doesn't have it - not interesting */
		if (o_ptr->el_info[i].res_level <= 0) continue;

		/* Element properties known */
		if (o_ptr->el_info[i].flags & EL_INFO_KNOWN) continue;

		/* Has a resist, or doubt remains */
		return TRUE;
	}

	/* No doubt left */
	return FALSE;
}


/**
 * \returns whether the object has been sensed with pseudo-ID
 */
bool object_was_sensed(const object_type *o_ptr)
{
	/* Hackish - NRM */
	return id_has(o_ptr->id_flags, ID_AC) ? TRUE : FALSE;
}

/**
 * Mark an object as sensed (kind of).
 */
void object_notice_sensing(object_type *o_ptr)
{
	if (object_was_sensed(o_ptr))
		return;

	if (o_ptr->artifact) {
		o_ptr->artifact->seen = o_ptr->artifact->everseen = TRUE;
		id_on(o_ptr->id_flags, ID_ARTIFACT);
	}

	object_notice_curses(o_ptr);
	/* Hackish - NRM */
	if (object_add_id_flag(o_ptr, ID_AC))
		object_check_for_ident(o_ptr);
}


/**
 * Sense artifacts
 */
void object_sense_artifact(object_type *o_ptr)
{
	id_on(o_ptr->id_flags, ID_ARTIFACT);
	if (o_ptr->artifact)
		object_notice_sensing(o_ptr);
}


/**
 * Given an object, return a short identifier which gives some idea of what
 * the item is.
 */
obj_pseudo_t object_pseudo(const object_type *o_ptr)
{
	bitflag flags[OF_SIZE], f2[OF_SIZE];

	/* Get the known and obvious flags on the object,
	 * not including curses or properties of the kind.
	 */
	object_flags_known(o_ptr, flags);
	create_mask(f2, TRUE, OFID_WIELD, OFT_MAX);

	/* FA on gloves is obvious to mage casters */
	if (player_has(PF_CUMBER_GLOVE) && o_ptr->tval == TV_GLOVES &&
		(o_ptr->modifiers[OBJ_MOD_DEX] <= 0) && 
		!kf_has(o_ptr->kind->kind_flags, KF_SPELLS_OK))
		of_on(f2, OF_FREE_ACT);

	/* Now we remove the non-obvious known flags */
	of_inter(flags, f2);

	/* Now we remove the cursed flags and the kind flags */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);
	of_diff(flags, f2);
	of_diff(flags, o_ptr->kind->flags);

	if ((object_was_sensed(o_ptr) || object_was_worn(o_ptr)) && o_ptr->artifact)
		return INSCRIP_SPECIAL;

	/* jewelry does not pseudo */
	if (tval_is_jewelry(o_ptr))
		return INSCRIP_NULL;

	/* XXX Eddie should also check for flags with pvals where the pval exceeds
	 * the base pval for things like picks of digging, though for now acid brand gets those
	 */
	if (!of_is_empty(flags))
		return INSCRIP_SPLENDID;

	if (!object_is_known(o_ptr) && !object_was_sensed(o_ptr))
		return INSCRIP_NULL;

	if (o_ptr->ego)
	{
		/* uncursed bad egos are not excellent */
		if (of_is_inter(o_ptr->ego->flags, f2))
			return INSCRIP_STRANGE; /* XXX Eddie need something worse */
		else
			return INSCRIP_EXCELLENT;
	}

	if (o_ptr->to_a == randcalc(o_ptr->kind->to_a, 0, MINIMISE) &&
	    o_ptr->to_h == randcalc(o_ptr->kind->to_h, 0, MINIMISE) &&
		 o_ptr->to_d == randcalc(o_ptr->kind->to_d, 0, MINIMISE))
		return INSCRIP_AVERAGE;

	if (o_ptr->to_a >= randcalc(o_ptr->kind->to_a, 0, MINIMISE) &&
	    o_ptr->to_h >= randcalc(o_ptr->kind->to_h, 0, MINIMISE) &&
	    o_ptr->to_d >= randcalc(o_ptr->kind->to_d, 0, MINIMISE))
		return INSCRIP_MAGICAL;

	if (o_ptr->to_a <= randcalc(o_ptr->kind->to_a, 0, MINIMISE) &&
	    o_ptr->to_h <= randcalc(o_ptr->kind->to_h, 0, MINIMISE) &&
	    o_ptr->to_d <= randcalc(o_ptr->kind->to_d, 0, MINIMISE))
		return INSCRIP_MAGICAL;

	return INSCRIP_STRANGE;
}


/**
 * Identify an item.
 */
void do_ident_item(object_type *o_ptr)
{
	char o_name[80];

	u32b msg_type = 0;
	int i, index, slot;
	bool bad = TRUE;

    /* Identify and apply autoinscriptions. */
	object_flavor_aware(o_ptr);
	object_notice_everything(o_ptr);
	apply_autoinscription(o_ptr);

	/* Set ignore flag */
	player->upkeep->notice |= PN_IGNORE;

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Determine the message type. */
	/* CC: we need to think more carefully about how we define "bad" with
	 * multiple modifiers - currently using "all nonzero modifiers < 0" */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (o_ptr->modifiers[i] > 0)
			bad = FALSE;

	if (bad)
		msg_type = MSG_IDENT_BAD;
	else if (o_ptr->artifact)
		msg_type = MSG_IDENT_ART;
	else if (o_ptr->ego)
		msg_type = MSG_IDENT_EGO;
	else
		msg_type = MSG_GENERIC;

	/* Log artifacts to the history list. */
	if (o_ptr->artifact)
		history_add_artifact(o_ptr->artifact, TRUE, TRUE);

	/* Describe */
	index = object_gear_index(player, o_ptr);
	slot = equipped_item_slot(player->body, index);
	if (item_is_equipped(player, index)) {
		/* Format and capitalise */
		char *msg = format("%s: %s (%c).", equip_describe(player, slot),
						   o_name, equip_to_label(slot));
		my_strcap(msg);

		msgt(msg_type, msg);
	} else if (index != NO_OBJECT)
		msgt(msg_type, "In your pack: %s (%c).", o_name, gear_to_label(index));
	else
		msgt(msg_type, "On the ground: %s.", o_name);
}

/**
 * Sense the inventory
 */
void sense_inventory(void)
{
	int i;
	
	char o_name[80];
	
	unsigned int rate;
	
	/* No ID when confused in a bad state */
	if (player->timed[TMD_CONFUSED]) return;

	/* Notice some things after a while */
	if (turn >= (object_last_wield + 3000))
	{
		object_notice_after_time();
		object_last_wield = 0;
	}

	/* Get improvement rate */
	if (player_has(PF_PSEUDO_ID_IMPROV))
		rate = player->class->sense_base / (player->lev * player->lev + player->class->sense_div);
	else
		rate = player->class->sense_base / (player->lev + player->class->sense_div);

	/* Check if player may sense anything this time */
	if (player->lev < 20 && !one_in_(rate)) return;

	/* Give each object one opportunity to have a chance at being sensed. */
	for (i = 0; i < player->max_gear; i++)
	{
		const char *text = NULL;
		object_type *o_ptr;
		obj_pseudo_t feel;
		bool cursed;
		bool equipped = item_is_equipped(player, i);

		o_ptr = &player->gear[i];

		/* Skip empty slots */
		if (!o_ptr->kind) continue;

		/* Valid tval codes only */
		if (!tval_is_weapon(o_ptr) && !tval_is_armor(o_ptr)) continue;
		
		/* It is known, no information needed */
		if (object_is_known(o_ptr)) continue;

		/* It has already been sensed, do not sense it again */
		if (object_was_sensed(o_ptr))
		{
			/* Small chance of wielded, sensed items getting complete ID */
			if (!o_ptr->artifact && equipped && one_in_(1000))
				do_ident_item(o_ptr);

			continue;
		}

		/* Occasional failure on inventory items */
		if (!equipped && one_in_(5)) continue;

		/* Sense the object */
		object_notice_sensing(o_ptr);
		cursed = object_notice_curses(o_ptr);

		/* Get the feeling */
		feel = object_pseudo(o_ptr);

		/* Stop everything */
		disturb(player, 0);

		if (cursed)
			text = "cursed";
		else
			text = inscrip_text[feel];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

		/* Average pseudo-ID means full ID */
		if (feel == INSCRIP_AVERAGE)
		{
			object_notice_everything(o_ptr);

			msgt(MSG_PSEUDOID, "You feel the %s (%c) %s %s average...",
				 o_name,
				 gear_to_label(i),
				 equipped ? "you are using" : "in your pack",
				 VERB_AGREEMENT(o_ptr->number, "is", "are"));
		}
		else
		{
			if (equipped)
			{
				msgt(MSG_PSEUDOID, "You feel the %s (%c) you are %s %s %s...",
					 o_name,
					 gear_to_label(i),
					 equip_describe(player, equipped_item_slot(player->body, i)),
					 VERB_AGREEMENT(o_ptr->number, "is", "are"),
					 text);
			}
			else
			{
				msgt(MSG_PSEUDOID, "You feel the %s (%c) in your pack %s %s...",
					 o_name,
					 gear_to_label(i),
					 VERB_AGREEMENT(o_ptr->number, "is", "are"),
					 text);
			}
		}


		/* Set ignore flag as appropriate */
		if (!item_is_equipped(player, i))
			player->upkeep->notice |= PN_IGNORE;
		
		/* Update the gear */
		player->upkeep->update |= (PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);
		
		/* Redraw stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}
}


