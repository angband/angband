/*
 * File: identify.c
 * Purpose: Object identification and knowledge routines
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
#include "obj-identify.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-tvalsval.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "player-util.h"
#include "spells.h"
#include "squelch.h"

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
 * \returns whether an object should be treated as fully known (e.g. ID'd)
 */
bool object_is_known(const object_type *o_ptr)
{
	return (o_ptr->ident & IDENT_KNOWN) || easy_know(o_ptr) ||
			(o_ptr->ident & IDENT_STORE);
}

/**
 * \returns whether the object is known to be an artifact
 */
bool object_is_known_artifact(const object_type *o_ptr)
{
	return (o_ptr->ident & IDENT_INDESTRUCT) ||
			(o_ptr->artifact && object_was_sensed(o_ptr));
}

/**
 * \returns whether the object is known to be cursed
 */
bool object_is_known_cursed(const object_type *o_ptr)
{
	bitflag f[OF_SIZE], f2[OF_SIZE];

	object_flags_known(o_ptr, f);

	/* Gather whatever curse flags there are to know */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);

	return of_is_inter(f, f2);
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
	if (id_has(o_ptr->id_flags, ID_ARTIFACT) && !(o_ptr->artifact))
		return TRUE;

	return FALSE;
}

/**
 * \returns whether the object has been worn/wielded
 */
bool object_was_worn(const object_type *o_ptr)
{
	return o_ptr->ident & IDENT_WORN ? TRUE : FALSE;
}

/**
 * \returns whether the object has been fired/thrown
 */
bool object_was_fired(const object_type *o_ptr)
{
	return o_ptr->ident & IDENT_FIRED ? TRUE : FALSE;
}

/**
 * \returns whether the object has been sensed with pseudo-ID
 */
bool object_was_sensed(const object_type *o_ptr)
{
	return o_ptr->ident & IDENT_SENSE ? TRUE : FALSE;
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
	return (easy_know(o_ptr) || (o_ptr->ident & IDENT_EFFECT)
		|| (object_flavor_is_aware(o_ptr) && o_ptr->kind->effect)
		|| (o_ptr->ident & IDENT_STORE)) ? TRUE : FALSE;
}

/**
 * \returns whether any ego or artifact name is available to the player
 */
bool object_name_is_visible(const object_type *o_ptr)
{
	return o_ptr->ident & IDENT_NAME ? TRUE : FALSE;
}

/**
 * \returns whether both the object is both an ego and the player knows it is
 */
bool object_ego_is_visible(const object_type *o_ptr)
{
	if (!o_ptr->ego)
		return FALSE;

	if (tval_is_light(o_ptr))
		return TRUE;

	if ((o_ptr->ident & IDENT_NAME) || (o_ptr->ident & IDENT_STORE))
		return TRUE;
	else
		return FALSE;
}

/**
 * \returns whether the object's attack plusses are known
 */
bool object_attack_plusses_are_visible(const object_type *o_ptr)
{
	/* Bonuses have been revealed or for sale */
	if ((o_ptr->ident & IDENT_ATTACK) || (o_ptr->ident & IDENT_STORE))
		return TRUE;

	/* Aware jewelry with non-variable bonuses */
	if (tval_is_jewelry(o_ptr) && object_flavor_is_aware(o_ptr))
	{
		if (!randcalc_varies(o_ptr->kind->to_h) && !randcalc_varies(o_ptr->kind->to_d))
			return TRUE;
	}

	return FALSE;
}

/**
 * \returns whether the object's defence bonuses are known
 */
bool object_defence_plusses_are_visible(const object_type *o_ptr)
{
	/* Bonuses have been revealed or for sale */
	if ((o_ptr->ident & IDENT_DEFENCE) || (o_ptr->ident & IDENT_STORE))
		return TRUE;

	/* Aware jewelry with non-variable bonuses */
	if (tval_is_jewelry(o_ptr) && object_flavor_is_aware(o_ptr))
	{
		if (!randcalc_varies(o_ptr->kind->to_a))
			return TRUE;
	}

	return FALSE;
}


/*
 * \returns whether the player knows whether an object has a given flag
 */
bool object_flag_is_known(const object_type *o_ptr, int flag)
{
	if (easy_know(o_ptr) ||
	    (o_ptr->ident & IDENT_STORE) ||
	    of_has(o_ptr->known_flags, flag))
		return TRUE;

	return FALSE;
}

/*
 * \returns whether the player knows the given element properties of an object
 */
bool object_element_is_known(const object_type *o_ptr, int element)
{
	if (element < 0 || element >= ELEM_MAX) return FALSE;

	if (easy_know(o_ptr) ||
	    (o_ptr->ident & IDENT_STORE) ||
	    (o_ptr->el_info[element].flags & EL_INFO_KNOWN))
		return TRUE;

	return FALSE;
}


/**
 * \returns whether a specific modifier is known to the player
 */
bool object_this_mod_is_visible(const object_type *o_ptr, int mod)
{
	assert(o_ptr->kind);

	/* Store objects */
	if (o_ptr->ident & IDENT_STORE)
		return TRUE;

	/* Aware jewelry with a fixed modifier (usually light) */
	if (tval_is_jewelry(o_ptr) && object_flavor_is_aware(o_ptr)
		&& !randcalc_varies(o_ptr->kind->modifiers[mod]))
		return TRUE;

	/* Wearing shows all modifiers */
	if (object_was_worn(o_ptr))
		return TRUE;

	return FALSE;
}

/*
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



/*
 * Sets a some IDENT_ flags on an object.
 *
 * \param o_ptr is the object to check
 * \param flags are the ident flags to be added
 *
 * \returns whether o_ptr->ident changed
 */
static bool object_add_ident_flags(object_type *o_ptr, u32b flags)
{
	if ((o_ptr->ident & flags) != flags)
	{
		o_ptr->ident |= flags;
		return TRUE;
	}

	return FALSE;
}


/*
 * Checks for additional knowledge implied by what the player already knows.
 *
 * \param o_ptr is the object to check
 *
 * returns whether it calls object_notice_everyting
 */
bool object_check_for_ident(object_type *o_ptr)
{
	bitflag flags[OF_SIZE], known_flags[OF_SIZE];
	size_t i;
	
	object_flags(o_ptr, flags);
	object_flags_known(o_ptr, known_flags);

	if (!of_is_equal(flags, known_flags)) return FALSE;

	/* Check for unknown resists, immunities and vulnerabilities */
	for (i = 0; i < ELEM_MAX; i++) {
		if (o_ptr->el_info[i].flags & EL_INFO_KNOWN) continue;
		if (o_ptr->el_info[i].res_level != 0) return FALSE;
	}

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
	{
		/* require worn status so you don't learn launcher of accuracy or 
		 * gloves of slaying before wield */
		if (object_was_worn(o_ptr))
			object_notice_ego(o_ptr);
	}

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

	/* Fix squelch/autoinscribe */
	if (kind_is_squelched_unaware(o_ptr->kind)) {
		kind_squelch_when_aware(o_ptr->kind);
	}
	player->upkeep->notice |= PN_SQUELCH;
	apply_autoinscription(o_ptr);

	for (i = 1; i < cave_object_max(cave); i++)
	{
		const object_type *floor_o_ptr = cave_object(cave, i);

		/* Some objects change tile on awareness */
		/* So update display for all floor objects of this kind */
		if (!floor_o_ptr->held_m_idx &&
				floor_o_ptr->kind == o_ptr->kind)
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
 * Make the player aware of all of an object's flags.
 *
 * \param o_ptr is the object to mark
 */
void object_know_all_elements(object_type *o_ptr)
{
	size_t i;

	for (i = 0; i < ELEM_MAX; i++)
		o_ptr->el_info[i].flags |= EL_INFO_KNOWN;
}


void object_know_brands_and_slays(object_type *o_ptr)
{
	struct brand *b;
	struct slay *s;

	for (b = o_ptr->brands; b; b = b->next)
		b->known = TRUE;
	for (s = o_ptr->slays; s; s = s->next)
		s->known = TRUE;
}



#define IDENTS_SET_BY_IDENTIFY ( IDENT_KNOWN | IDENT_ATTACK | IDENT_DEFENCE | IDENT_SENSE | IDENT_EFFECT | IDENT_WORN | IDENT_FIRED | IDENT_NAME )

/**
 * Mark as object as fully known, a.k.a identified. 
 *
 * \param o_ptr is the object to mark as identified
 */
void object_notice_everything(object_type *o_ptr)
{
	/* The object is "empty" */
	o_ptr->ident &= ~(IDENT_EMPTY);

	/* Mark as known */
	object_flavor_aware(o_ptr);
	object_add_ident_flags(o_ptr, IDENTS_SET_BY_IDENTIFY);

	/* Artifact has now been seen */
	if (o_ptr->artifact && !(o_ptr->ident & IDENT_FAKE))
	{
		o_ptr->artifact->seen = o_ptr->artifact->everseen = TRUE;
		history_add_artifact(o_ptr->artifact, TRUE, TRUE);
	}

	/* Know all flags there are to be known */
	object_know_all_flags(o_ptr);

	/* Know all elemental properties */
	object_know_all_elements(o_ptr);

	/* Know all brands and slays */
	object_know_brands_and_slays(o_ptr);
}



/**
 * Notice that an object is indestructible.
 */
void object_notice_indestructible(object_type *o_ptr)
{
	if (object_add_ident_flags(o_ptr, IDENT_INDESTRUCT))
		object_check_for_ident(o_ptr);
}


/*
 * Notice the ego on an ego item.
 */
void object_notice_ego(object_type *o_ptr)
{
	bitflag learned_flags[OF_SIZE];
	bitflag xtra_flags[OF_SIZE];
	size_t i;

	if (!o_ptr->ego)
		return;


	/* XXX Eddie print a message on notice ego if not already noticed? */
	/* XXX Eddie should we do something about everseen of egos here? */

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
				(o_ptr->el_info[i].flags & EL_INFO_RANDOM))
				o_ptr->el_info[i].flags |= EL_INFO_KNOWN;
	}


	of_union(o_ptr->known_flags, learned_flags);

	if (object_add_ident_flags(o_ptr, IDENT_NAME))
	{
		/* if you know the ego, you know which it is of excellent or splendid */
		object_notice_sensing(o_ptr);

		object_check_for_ident(o_ptr);
	}
}


/*
 * Mark an object as sensed.
 */
void object_notice_sensing(object_type *o_ptr)
{
	if (object_was_sensed(o_ptr))
		return;

	if (o_ptr->artifact) {
		o_ptr->artifact->seen = o_ptr->artifact->everseen = TRUE;
		o_ptr->ident |= IDENT_NAME;
	}

	object_notice_curses(o_ptr);
	if (object_add_ident_flags(o_ptr, IDENT_SENSE))
		object_check_for_ident(o_ptr);
}


/*
 * Sense artifacts
 */
void object_sense_artifact(object_type *o_ptr)
{
	id_on(o_ptr->id_flags, ID_ARTIFACT);
	if (o_ptr->artifact)
		object_notice_sensing(o_ptr);
}


/**
 * Notice the "effect" from activating an object.
 *
 * \param o_ptr is the object to become aware of
 */
void object_notice_effect(object_type *o_ptr)
{
	if (object_add_ident_flags(o_ptr, IDENT_EFFECT))
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

	if (object_add_ident_flags(o_ptr, IDENT_DEFENCE))
		object_check_for_ident(o_ptr);

	if (o_ptr->ac || o_ptr->to_a)
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		msgt(MSG_PSEUDOID,
				"You know more about the %s you are wearing.",
				o_name);
	}

	p->upkeep->update |= (PU_BONUS);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


void object_notice_attack_plusses(object_type *o_ptr)
{
	assert(o_ptr && o_ptr->kind);

	if (object_attack_plusses_are_visible(o_ptr))
		return;

	if (object_add_ident_flags(o_ptr, IDENT_ATTACK))
		object_check_for_ident(o_ptr);


	if (wield_slot(o_ptr) == INVEN_WIELD)
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		msgt(MSG_PSEUDOID,
				"You know more about the %s you are using.",
				o_name);
	}
	else if ((o_ptr->to_d || o_ptr->to_h) &&
			!((o_ptr->tval == TV_HARD_ARMOR || o_ptr->tval == TV_SOFT_ARMOR) && (o_ptr->to_h < 0)))
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		msgt(MSG_PSEUDOID, "Your %s glow%s.",
				o_name, ((o_ptr->number > 1) ? "" : "s"));
	}

	player->upkeep->update |= (PU_BONUS);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/*
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


/*
 * Notice a single flag - returns TRUE if anything new was learned
 */
bool object_notice_flag(object_type *o_ptr, int flag)
{
	if (!of_has(o_ptr->known_flags, flag))
	{
		of_on(o_ptr->known_flags, flag);
		/* XXX Eddie don't want infinite recursion if object_check_for_ident sets more flags,
		 * but maybe this will interfere with savefile repair
		 */
		object_check_for_ident(o_ptr);
		event_signal(EVENT_INVENTORY);
		event_signal(EVENT_EQUIPMENT);

		return TRUE;
	}

	return FALSE;
}


/*
 * Notice a set of flags - returns TRUE if anything new was learned
 */
bool object_notice_flags(object_type *o_ptr, bitflag flags[OF_SIZE])
{
	if (!of_is_subset(o_ptr->known_flags, flags))
	{
		of_union(o_ptr->known_flags, flags);
		/* XXX Eddie don't want infinite recursion if object_check_for_ident sets more flags,
		 * but maybe this will interfere with savefile repair
		 */
		object_check_for_ident(o_ptr);
		event_signal(EVENT_INVENTORY);
		event_signal(EVENT_EQUIPMENT);

		return TRUE;
	}

	return FALSE;
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

	player->upkeep->notice |= PN_SQUELCH;

	return !of_is_empty(f);
}


/**
 * Notice things which happen on defending.
 */
void object_notice_on_defend(struct player *p)
{
	int i;

	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		if (p->inventory[i].kind)
			object_notice_defence_plusses(p, &p->inventory[i]);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/*
 * Notice stuff when firing or throwing objects.
 *
 */
/* XXX Eddie perhaps some stuff from do_cmd_fire and do_cmd_throw should be moved here */
void object_notice_on_firing(object_type *o_ptr)
{
	if (object_add_ident_flags(o_ptr, IDENT_FIRED))
		object_check_for_ident(o_ptr);
}



/*
 * Determine whether a weapon or missile weapon is obviously {excellent} when
 * worn.
 *
 * XXX Eddie should messages be adhoc all over the place?  perhaps the main
 * loop should check for change in inventory/wieldeds and all messages be
 * printed from one place
 */
void object_notice_on_wield(object_type *o_ptr)
{
	bitflag f[OF_SIZE], f2[OF_SIZE], obvious_mask[OF_SIZE];
	bool obvious = FALSE;
	int i;

	create_mask(obvious_mask, TRUE, OFID_WIELD, OFT_MAX);

	/* Save time of wield for later */
	object_last_wield = turn;

	/* Only deal with un-ID'd items */
	if (object_is_known(o_ptr)) return;

	/* Wear it */
	object_flavor_tried(o_ptr);
	if (object_add_ident_flags(o_ptr, IDENT_WORN))
		object_check_for_ident(o_ptr);

	/* CC: may wish to be more subtle about this once we have ego lights
	 * with multiple pvals */
	if (tval_is_light(o_ptr) && o_ptr->ego)
		object_notice_ego(o_ptr);

	if (object_flavor_is_aware(o_ptr) && easy_know(o_ptr))
	{
		object_notice_everything(o_ptr);
		return;
	}

	/* Automatically sense artifacts upon wield */
	object_sense_artifact(o_ptr);

	/* Note artifacts when found */
	if (o_ptr->artifact)
		history_add_artifact(o_ptr->artifact, object_is_known(o_ptr), TRUE);

	/* special case FA, needed at least for mages wielding gloves */
	if (object_FA_would_be_obvious(o_ptr))
		of_on(obvious_mask, OF_FREE_ACT);

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Find obvious things (disregarding curses) - why do we remove the curses?? */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);
	of_diff(obvious_mask, f2);
	if (of_is_inter(f, obvious_mask)) obvious = TRUE;
	create_mask(obvious_mask, TRUE, OFID_WIELD, OFT_MAX);
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (o_ptr->modifiers[i]) obvious = TRUE;

	/* Notice any brands */
	object_notice_brands(o_ptr, NULL);

	/* Learn about obvious flags */
	of_union(o_ptr->known_flags, obvious_mask);

	/* XXX Eddie should these next NOT call object_check_for_ident due to worries about repairing? */

	/* XXX Eddie this is a small hack, but jewelry with anything noticeable really is obvious */
	/* XXX Eddie learn =soulkeeping vs =bodykeeping when notice sustain_str */
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

	/* XXX Eddie need to add stealth here, also need to assert/double-check everything is covered */
	/* CC: also need to add FA! */
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

	/* WARNING -- masking f by obvious mask -- this should be at the end of this function */
	/* CC: I think this can safely go, but just in case ... */
/*	flags_mask(f, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END); */

	/* Remember the flags */
	object_notice_sensing(o_ptr);

	/* XXX Eddie should we check_for_ident here? */
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
	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		o_ptr = &player->inventory[i];

		if (!o_ptr->kind || object_is_known(o_ptr)) continue;

		/* Check for timed notice flags */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		object_flags(o_ptr, f);
		of_inter(f, timed_mask);

		for (flag = of_next(f, FLAG_START); flag != FLAG_END; flag = of_next(f, flag + 1))
		{
			if (!of_has(o_ptr->known_flags, flag))
			{
				/* Message */
				flag_message(flag, o_name);

				/* Notice the flag */
				object_notice_flag(o_ptr, flag);

				if (tval_is_jewelry(o_ptr) &&
					 (!object_effect(o_ptr) || object_effect_is_known(o_ptr)))
				{
					/* XXX this is a small hack, but jewelry with anything noticeable really is obvious */
					/* XXX except, wait until learn activation if that is only clue */
					object_flavor_aware(o_ptr);
					object_check_for_ident(o_ptr);
				}
			}
			else
			{
				/* Notice the flag is absent */
				object_notice_flag(o_ptr, flag);
			}
		}

		/* XXX Is this necessary? */
		object_check_for_ident(o_ptr);
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

	/* XXX Eddie need different naming conventions for starting wieldeds at INVEN_WIELD vs INVEN_WIELD+2 */
	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p->inventory[i];
		bitflag f[OF_SIZE];

		if (!o_ptr->kind) continue;

		object_flags(o_ptr, f);

		if (of_has(f, flag) && !of_has(o_ptr->known_flags, flag))
		{
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

			/* Notice the flag */
			object_notice_flag(o_ptr, flag);

			/* XXX Eddie should this go before noticing the flag to avoid learning twice? */
			if (tval_is_jewelry(o_ptr))
			{
				object_flavor_aware(o_ptr);
				object_check_for_ident(o_ptr);
			}

			/* Message */
			flag_message(flag, o_name);
		}
		else
		{
			/* Notice that flag is absent */
			object_notice_flag(o_ptr, flag);
		}

		/* XXX Eddie should not need this, should be done in noticing, but will remove later */
		object_check_for_ident(o_ptr);

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

	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p->inventory[i];

		if (!o_ptr->kind) continue;

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

		/* I hope this will be done elsewhere - NRM */
		if (tval_is_jewelry(o_ptr))
		{
			object_flavor_aware(o_ptr);
			object_check_for_ident(o_ptr);
		}
	}
}

/**
 * Notice to-hit bonus on attacking.
 */
void wieldeds_notice_to_hit_on_attack(void)
/* Used e.g. for ranged attacks where the item's to_d is not involved. */
/* Does not apply to weapon or bow which should be done separately */
{
	int i;

	for (i = INVEN_WIELD + 2; i < INVEN_TOTAL; i++)
		if (player->inventory[i].kind &&
		    player->inventory[i].to_h)
			object_notice_attack_plusses(&player->inventory[i]);

	return;
}


/**
 * Notice things which happen on attacking.
 */
void wieldeds_notice_on_attack(void)
/* Does not apply to weapon or bow which should be done separately */
{
	int i;

	for (i = INVEN_WIELD + 2; i < INVEN_TOTAL; i++)
		if (player->inventory[i].kind)
			object_notice_attack_plusses(&player->inventory[i]);

	/* XXX Eddie print message? */
	/* XXX Eddie do we need to do more about ammo? */

	return;
}


bool object_FA_would_be_obvious(const object_type *o_ptr)
{
	if (player_has(PF_CUMBER_GLOVE) && wield_slot(o_ptr) == INVEN_HANDS) {

		if ((o_ptr->modifiers[OBJ_MOD_DEX] <= 0) && 
			!kf_has(o_ptr->kind->kind_flags, KF_SPELLS_OK))
			return TRUE;
	}

	return FALSE;
}

/*
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
	if (object_FA_would_be_obvious(o_ptr))
		of_on(f2, OF_FREE_ACT);

	/* Now we remove the non-obvious known flags */
	of_inter(flags, f2);

	/* Now we remove the cursed flags and the kind flags */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);
	of_diff(flags, f2);
	of_diff(flags, o_ptr->kind->flags);

	if (o_ptr->ident & IDENT_INDESTRUCT)
		return INSCRIP_SPECIAL;
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


/*
 * Sense the inventory
 */
void sense_inventory(void)
{
	int i;
	
	char o_name[80];
	
	unsigned int rate;
	bool sensed_this_turn[ALL_INVEN_TOTAL];
	
	/* Nothing sensed yet */
	for (i = 0; i < ALL_INVEN_TOTAL; i++)
		sensed_this_turn[i] = FALSE;

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

	/*
	 * Give each object one opportunity to have a chance at being sensed.
	 * Because the inventory can be reordered in do_ident_item(),
	 * we want to prevent objects from having more than one opportunity each
	 * turn. This state is stored in the sensed_this_turn array. If the pack
	 * is reordered, we start the loop over and skip objects that have had
	 * their opportunity.
	 *
	 * Also, i is incremented at the top of the loop so that the conditions
	 * can properly use "continue"; hence why we start at -1.
	 */
	i = -1;
	while (i < ALL_INVEN_TOTAL - 1)
	{
		const char *text = NULL;
		object_type *o_ptr;
		obj_pseudo_t feel;
		bool cursed;
		bool okay = FALSE;

		i++;
		o_ptr = &player->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->kind) continue;

		/* Valid "tval" codes */
		if (tval_is_weapon(o_ptr) || tval_is_armor(o_ptr))
			okay = TRUE;

		/* Skip non-sense machines */
		if (!okay) continue;
		
		/* It is known, no information needed */
		if (object_is_known(o_ptr)) continue;

		/* Do not allow an object to have more than one opportunity to have
		 * a chance of being fully ID'd or sensed again. */
		if (sensed_this_turn[i])
			continue;

		/* It has already been sensed, do not sense it again */
		if (object_was_sensed(o_ptr))
		{
			/* Small chance of wielded, sensed items getting complete ID */
			if (!o_ptr->artifact && (i >= INVEN_WIELD) && one_in_(1000)) {
				/* Reset the loop to finish sensing. This item is now known,
				 * and will be skipped on the next pass. */
				do_ident_item(o_ptr);
				i = -1;
			}

			continue;
		}

		/* Prevent objects, which pass or fail the sense check for this turn,
		 * from getting another opportunity. */
		sensed_this_turn[i] = TRUE;

		/* Occasional failure on inventory items */
		if ((i < INVEN_WIELD) && one_in_(5)) continue;


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
				 index_to_label(i),
				 ((i >= INVEN_WIELD) ? "you are using" : "in your pack"),
				 VERB_AGREEMENT(o_ptr->number, "is", "are"));
		}
		else
		{
			if (i >= INVEN_WIELD)
			{
				msgt(MSG_PSEUDOID, "You feel the %s (%c) you are %s %s %s...",
					 o_name,
					 index_to_label(i),
					 describe_use(i),
					 VERB_AGREEMENT(o_ptr->number, "is", "are"),
					 text);
			}
			else
			{
				msgt(MSG_PSEUDOID, "You feel the %s (%c) in your pack %s %s...",
					 o_name,
					 index_to_label(i),
					 VERB_AGREEMENT(o_ptr->number, "is", "are"),
					 text);
			}
		}


		/* Set squelch flag as appropriate */
		if (i < INVEN_WIELD)
			player->upkeep->notice |= PN_SQUELCH;
		
		
		/* Combine / Reorder the pack (later) */
		player->upkeep->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);
		
		/* Redraw stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}
}


