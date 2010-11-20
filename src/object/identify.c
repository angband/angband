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
#include "game-event.h"
#include "history.h"
#include "object/tvalsval.h"
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
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	if (k_ptr->aware && of_has(k_ptr->flags, OF_EASY_KNOW))
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
			(artifact_p(o_ptr) && object_was_sensed(o_ptr));
}

/**
 * \returns whether the object is known to be cursed
 */
bool object_is_known_cursed(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];

	object_flags_known(o_ptr, f);

	/* Know whatever curse flags there are to know */
	flags_mask(f, OF_SIZE, OF_CURSE_MASK, FLAG_END);

	return !of_is_empty(f);
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
	if (o_ptr->ident & IDENT_NOTART)
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
	return k_info[o_ptr->k_idx].aware;
}

/**
 * \returns whether the player has tried to use other objects of the same kind
 */
bool object_flavor_was_tried(const object_type *o_ptr)
{
	return k_info[o_ptr->k_idx].tried;
}

/**
 * \returns whether the player is aware of the object's effect when used
 */
bool object_effect_is_known(const object_type *o_ptr)
{
	return (easy_know(o_ptr) || (o_ptr->ident & IDENT_EFFECT)
		|| (object_flavor_is_aware(o_ptr) && k_info[o_ptr->k_idx].effect)
		|| (o_ptr->ident & IDENT_STORE)) ? TRUE : FALSE;
}

/**
 * \returns whether the object's pval is known to the player
 */
bool object_pval_is_visible(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];

	if (o_ptr->ident & IDENT_STORE)
		return TRUE;

	/* Aware jewelry with non-variable pvals */
	if (object_is_jewelry(o_ptr) && object_flavor_is_aware(o_ptr))
	{
		const object_kind *k_ptr = &k_info[o_ptr->k_idx];

		if (!randcalc_varies(k_ptr->pval))
			return TRUE;
	}

	if (object_was_worn(o_ptr))
	{
		object_flags_known(o_ptr, f);

		if (flags_test(f, OF_SIZE, OF_PVAL_MASK, FLAG_END))
			return TRUE;
	}

	return FALSE;
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
	if (!ego_item_p(o_ptr))
		return FALSE;

	if (o_ptr->tval == TV_LIGHT)
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
	if (object_is_jewelry(o_ptr) && object_flavor_is_aware(o_ptr))
	{
		const object_kind *k_ptr = &k_info[o_ptr->k_idx];

		if (!randcalc_varies(k_ptr->to_h) && !randcalc_varies(k_ptr->to_d))
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
	if (object_is_jewelry(o_ptr) && object_flavor_is_aware(o_ptr))
	{
		const object_kind *k_ptr = &k_info[o_ptr->k_idx];

		if (!randcalc_varies(k_ptr->to_a))
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
 * \returns whether it is possible an object has a high resist given the
 *          player's current knowledge
 */
bool object_high_resist_is_possible(const object_type *o_ptr)
{
	bitflag flags[OF_SIZE];

	/* Actual object flags */
	object_flags(o_ptr, flags);

	/* Add player's uncertainty */
	of_comp_union(flags, o_ptr->known_flags);

	/* Check for possible high resist */
	if (flags_test(flags, OF_SIZE, OF_HIGH_RESIST_MASK, FLAG_END))
		return TRUE;
	else
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
	
	object_flags(o_ptr, flags);
	object_flags_known(o_ptr, known_flags);

	/* Some flags are irrelevant or never learned or too hard to learn */
	flags_clear(flags, OF_SIZE, OF_OBJ_ONLY_MASK, FLAG_END);
	flags_clear(known_flags, OF_SIZE, OF_OBJ_ONLY_MASK, FLAG_END);

	if (!of_is_equal(flags, known_flags)) return FALSE;

	/* If we know attack bonuses, and defence bonuses, and effect, then
	 * we effectively know everything, so mark as such */
	if ((object_attack_plusses_are_visible(o_ptr) || (object_was_sensed(o_ptr) && o_ptr->to_h == 0 && o_ptr->to_d == 0)) &&
	    (object_defence_plusses_are_visible(o_ptr) || (object_was_sensed(o_ptr) && o_ptr->to_a == 0)) &&
	    (object_effect_is_known(o_ptr) || !object_effect(o_ptr)))
	{
		object_notice_everything(o_ptr);
		return TRUE;
	}

	/* We still know all the flags, so we still know if it's an ego */
	else if (ego_item_p(o_ptr))
	{
		/* require worn status so you don't learn launcher of accuracy or gloves of slaying before wield */
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
	p_ptr->notice |= PN_SQUELCH;
	apply_autoinscription(o_ptr);

	for (i = 1; i < o_max; i++)
	{
		const object_type *floor_o_ptr = &o_list[i];

		/* Some objects change tile on awareness */
		/* So update display for all floor objects of this kind */
		if (!floor_o_ptr->held_m_idx &&
				floor_o_ptr->k_idx == o_ptr->k_idx)
			light_spot(floor_o_ptr->iy, floor_o_ptr->ix);
	}
}


/**
 * Mark an object's flavour as tried.
 *
 * \param o_ptr is the object whose flavour should be marked
 */
void object_flavor_tried(object_type *o_ptr)
{
	assert(o_ptr != NULL);
	assert(o_ptr->k_idx > 0);
	assert(o_ptr->k_idx < z_info->k_max);

	k_info[o_ptr->k_idx].tried = TRUE;
}

/**
 * Make the player aware of all of an object's flags.
 *
 * \param o_ptr is the object to mark
 */
void object_know_all_flags(object_type *o_ptr)
{
	memset(o_ptr->known_flags, 0xff, sizeof(o_ptr->known_flags));
}


#define IDENTS_SET_BY_IDENTIFY ( IDENT_KNOWN | IDENT_ATTACK | IDENT_DEFENCE | IDENT_SENSE | IDENT_EFFECT | IDENT_WORN | IDENT_FIRED | IDENT_NAME )

/**
 * Check whether an object has IDENT_KNOWN but should not
 */
bool object_is_not_known_consistently(const object_type *o_ptr)
{
	if (easy_know(o_ptr))
		return FALSE;
	if (!(o_ptr->ident & IDENT_KNOWN))
		return TRUE;
	if ((o_ptr->ident & IDENTS_SET_BY_IDENTIFY) != IDENTS_SET_BY_IDENTIFY)
		return TRUE;
	if (o_ptr->ident & IDENT_EMPTY)
		return TRUE;
	else if (o_ptr->name1)
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];
		if (!(a_ptr->seen || a_ptr->everseen))
			return TRUE;
	}

	if (!of_is_full(o_ptr->known_flags))
		return TRUE;

	return FALSE;
}



/**
 * Mark as object as fully known, a.k.a identified. 
 * Mark as object as fully known, a.k.a identified.
 *
 * \param o_ptr is the object to mark as identified
 */
void object_notice_everything(object_type *o_ptr)
{
	artifact_type *a_ptr = artifact_of(o_ptr);

	/* The object is "empty" */
	o_ptr->ident &= ~(IDENT_EMPTY);

	/* Mark as known */
	object_flavor_aware(o_ptr);
	object_add_ident_flags(o_ptr, IDENTS_SET_BY_IDENTIFY);

	/* Artifact has now been seen */
	if (a_ptr)
	{
		a_ptr->seen = a_ptr->everseen = TRUE;
		history_add_artifact(o_ptr->name1, TRUE, TRUE);
	}

	/* Know all flags there are to be known */
	object_know_all_flags(o_ptr);
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
	ego_item_type *e_ptr;
	bitflag learned_flags[OF_SIZE];
	bitflag xtra_flags[OF_SIZE];

	if (!o_ptr->name2)
		return;

	e_ptr = &e_info[o_ptr->name2];


	/* XXX Eddie print a message on notice ego if not already noticed? */
	/* XXX Eddie should we do something about everseen of egos here? */

	/* Learn ego flags */
	of_union(o_ptr->known_flags, e_ptr->flags);

	/* Learn all flags except random abilities */
	of_setall(learned_flags);

	switch (e_ptr->xtra)
	{
		case OBJECT_XTRA_TYPE_NONE:
			break;
		case OBJECT_XTRA_TYPE_SUSTAIN:
			set_ego_xtra_sustain(xtra_flags);
			of_diff(learned_flags, xtra_flags);
			break;
		case OBJECT_XTRA_TYPE_RESIST:
			set_ego_xtra_resist(xtra_flags);
			of_diff(learned_flags, xtra_flags);
			break;
		case OBJECT_XTRA_TYPE_POWER:
			set_ego_xtra_power(xtra_flags);
			of_diff(learned_flags, xtra_flags);
			break;
		default:
			assert(0);
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
	artifact_type *a_ptr = artifact_of(o_ptr);

	if (object_was_sensed(o_ptr))
		return;


	if (a_ptr)
	{
		a_ptr->seen = a_ptr->everseen = TRUE;
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
	if (artifact_p(o_ptr))
		object_notice_sensing(o_ptr);
	else
		o_ptr->ident |= IDENT_NOTART;
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


/*
 * Notice slays on a particular object.
 *
 * \param known_f0 is the list of flags to notice
 */
void object_notice_slay(object_type *o_ptr, int flag)
{
	const slay_t *s_ptr;
	bool learned = object_notice_flag(o_ptr, flag);

	/* if you learn a slay, learn the ego and print a message */
	if (EASY_LEARN && learned)
	{
		object_notice_ego(o_ptr);

		for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++)
		{
			if (s_ptr->slay_flag == flag)
			{
				char o_name[40];
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
				msg_format("Your %s %s!", o_name, s_ptr->active_verb);
			}
		}
	}

	object_check_for_ident(o_ptr);
}


static void object_notice_defence_plusses(object_type *o_ptr)
{
	if (!o_ptr->k_idx) return;
	if (object_defence_plusses_are_visible(o_ptr))
		return;

	if (object_add_ident_flags(o_ptr, IDENT_DEFENCE))
		object_check_for_ident(o_ptr);

	if (o_ptr->ac || o_ptr->to_a)
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		message_format(MSG_PSEUDOID, 0,
				"You know more about the %s you are wearing.",
				o_name);
	}

	p_ptr->update |= (PU_BONUS);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


void object_notice_attack_plusses(object_type *o_ptr)
{
	if (!o_ptr->k_idx) return;
	if (object_attack_plusses_are_visible(o_ptr))
		return;

	if (object_add_ident_flags(o_ptr, IDENT_ATTACK))
		object_check_for_ident(o_ptr);


	if (wield_slot(o_ptr) == INVEN_WIELD)
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		message_format(MSG_PSEUDOID, 0,
				"You know more about the %s you are using.",
				o_name);
	}
	else if ((o_ptr->to_d || o_ptr->to_h) &&
			!((o_ptr->tval == TV_HARD_ARMOR || o_ptr->tval == TV_SOFT_ARMOR) && (o_ptr->to_h < 0)))
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		message_format(MSG_PSEUDOID, 0, "Your %s glows.", o_name);
	}

	p_ptr->update |= (PU_BONUS);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


static const char *msgs[] =
{
	#define OF(a,b) b,
	#include "list-object-flags.h"
	#undef OF
};


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
	bitflag f[OF_SIZE];

	object_flags(o_ptr, f);

	/* Know whatever curse flags there are to know */
	flags_mask(f, OF_SIZE, OF_CURSE_MASK, FLAG_END);

	/* give knowledge of which curses are present */
	object_notice_flags(o_ptr, f);

	object_check_for_ident(o_ptr);

	p_ptr->notice |= PN_SQUELCH;

	return !of_is_empty(f);
}


/**
 * Notice things which happen on defending.
 */
void object_notice_on_defend(void)
{
	int i;

	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		object_notice_defence_plusses(&p_ptr->inventory[i]);

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
 * Determine whether a weapon or missile weapon is obviously {excellent} when worn.
 */
/* XXX Eddie should messages be adhoc all over the place?  perhaps the main
 * loop should check for change in inventory/wieldeds and all messages be
 * printed from one place
 */
void object_notice_on_wield(object_type *o_ptr)
{
	bitflag f[OF_SIZE], obvious_mask[OF_SIZE];
	bool obvious = FALSE;
	const slay_t *s_ptr;

	flags_init(obvious_mask, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END);

	/* Save time of wield for later */
	object_last_wield = turn;

	/* Only deal with un-ID'd items */
	if (object_is_known(o_ptr)) return;

	/* Wear it */
	object_flavor_tried(o_ptr);
	if (object_add_ident_flags(o_ptr, IDENT_WORN))
		object_check_for_ident(o_ptr);

	if (obj_is_light(o_ptr) && ego_item_p(o_ptr))
		object_notice_ego(o_ptr);

	if (object_flavor_is_aware(o_ptr) && easy_know(o_ptr))
	{
		object_notice_everything(o_ptr);
		return;
	}

	/* Automatically sense artifacts upon wield */
	object_sense_artifact(o_ptr);

	/* Note artifacts when found */
	if (artifact_p(o_ptr))
		history_add_artifact(o_ptr->name1, object_is_known(o_ptr), TRUE);

	/* special case FA, needed at least for mages wielding gloves */
	if (object_FA_would_be_obvious(o_ptr))
		of_on(obvious_mask, OF_FREE_ACT);

	/* Learn about obvious flags */
	of_union(o_ptr->known_flags, obvious_mask);

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Find obvious things (disregarding curses) */
	flags_clear(obvious_mask, OF_SIZE, OF_CURSE_MASK, FLAG_END);
	if (of_is_inter(f, obvious_mask)) obvious = TRUE;
	flags_init(obvious_mask, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END);

	/* XXX Eddie should these next NOT call object_check_for_ident due to worries about repairing? */


	/* XXX Eddie this is a small hack, but jewelry with anything noticeable really is obvious */
	/* XXX Eddie learn =soulkeeping vs =bodykeeping when notice sustain_str */
	if (object_is_jewelry(o_ptr))
	{
		/* Learn the flavor of jewelry with obvious flags */
		if (EASY_LEARN && obvious)
			object_flavor_aware(o_ptr);

		/* Learn all flags on any aware non-artifact jewelry */
		if (object_flavor_is_aware(o_ptr) && !artifact_p(o_ptr))
			object_know_all_flags(o_ptr);
	}

	object_check_for_ident(o_ptr);

	if (!obvious) return;

	/* Messages */
	for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++)
	{
		if (of_has(f, s_ptr->slay_flag) && s_ptr->brand)
		{
			char o_name[40];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
			msg_format("Your %s %s!", o_name, s_ptr->active_verb);
		}
	}

	/* XXX Eddie need to add stealth here, also need to assert/double-check everything is covered */
	if (of_has(f, OF_STR))
		msg_format("You feel %s!", o_ptr->pval > 0 ? "stronger" : "weaker");
	if (of_has(f, OF_INT))
		msg_format("You feel %s!", o_ptr->pval > 0 ? "smarter" : "more stupid");
	if (of_has(f, OF_WIS))
		msg_format("You feel %s!", o_ptr->pval > 0 ? "wiser" : "more naive");
	if (of_has(f, OF_DEX))
		msg_format("You feel %s!", o_ptr->pval > 0 ? "more dextrous" : "clumsier");
	if (of_has(f, OF_CON))
		msg_format("You feel %s!", o_ptr->pval > 0 ? "healthier" : "sicklier");
	if (of_has(f, OF_CHR))
		msg_format("You feel %s!", o_ptr->pval > 0 ? "cuter" : "uglier");
	if (of_has(f, OF_SPEED))
		msg_format("You feel strangely %s.", o_ptr->pval > 0 ? "quick" : "sluggish");
	if (flags_test(f, OF_SIZE, OF_BLOWS, OF_SHOTS, FLAG_END))
		msg_format("Your hands %s", o_ptr->pval > 0 ? "tingle!" : "ache.");
	if (of_has(f, OF_INFRA))
		msg_format("Your eyes tingle.");
	if (of_has(f, OF_LIGHT))
		msg_print("It glows!");
	if (of_has(f, OF_TELEPATHY))
		msg_print("Your mind feels strangely sharper!");

	/* WARNING -- masking f by obvious mask -- this should be at the end of this function */
	flags_mask(f, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END);

	/* learn the ego on any obvious brand or slay */
	if (EASY_LEARN && ego_item_p(o_ptr) && obvious &&
	    flags_test(f, OF_SIZE, OF_ALL_SLAY_MASK, FLAG_END))
		object_notice_ego(o_ptr);

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

	flags_init(timed_mask, OF_SIZE, OF_NOTICE_TIMED_MASK, FLAG_END);

	/* Check every item the player is wearing */
	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->k_idx || object_is_known(o_ptr)) continue;

		/* Check for timed notice flags */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
		object_flags(o_ptr, f);
		of_inter(f, timed_mask);

		for (flag = of_next(f, FLAG_START); flag != FLAG_END; flag = of_next(f, flag + 1))
		{
			if (!of_has(o_ptr->known_flags, flag))
			{
				/* Message */
				if (!streq(msgs[flag], ""))
					msg_format(msgs[flag], o_name);

				/* Notice the flag */
				object_notice_flag(o_ptr, flag);

				if (object_is_jewelry(o_ptr) &&
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
void wieldeds_notice_flag(int flag)
{
	int i;

	/* XXX Eddie need different naming conventions for starting wieldeds at INVEN_WIELD vs INVEN_WIELD+2 */
	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];
		bitflag f[OF_SIZE];

		if (!o_ptr->k_idx) continue;

		object_flags(o_ptr, f);

		if (of_has(f, flag) && !of_has(o_ptr->known_flags, flag))
		{
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

			/* Notice the flag */
			object_notice_flag(o_ptr, flag);

			/* XXX Eddie should this go before noticing the flag to avoid learning twice? */
			if (EASY_LEARN && object_is_jewelry(o_ptr))
			{
				/* XXX Eddie EASY_LEARN Possible concern: gets =teleportation just from +2 speed */
				object_flavor_aware(o_ptr);
				object_check_for_ident(o_ptr);
			}

			/* Message */
			if (!streq(msgs[flag], ""))
				msg_format(msgs[flag], o_name);
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
 * Notice things which happen on attacking.
 */
void wieldeds_notice_on_attack(void)
/* Does not apply to weapon or bow which should be done separately */
{
	int i;

	for (i = INVEN_WIELD + 2; i < INVEN_TOTAL; i++)
		object_notice_attack_plusses(&p_ptr->inventory[i]);

	/* XXX Eddie print message? */
	/* XXX Eddie do we need to do more about ammo? */

	return;
}


bool object_FA_would_be_obvious(const object_type *o_ptr)
{
	bitflag flags[OF_SIZE];
	
	if (!player_has(PF_CUMBER_GLOVE))
		return FALSE;

	if ((wield_slot(o_ptr) != INVEN_HANDS) || (o_ptr->sval == SV_SET_OF_ALCHEMISTS_GLOVES))
		return FALSE;

	object_flags(o_ptr, flags);
	if (of_has(flags, OF_DEX))
		return FALSE;

	return TRUE;
}

/*
 * Given an object, return a short identifier which gives some idea of what
 * the item is.
 */
obj_pseudo_t object_pseudo(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	bitflag flags[OF_SIZE];

	/* Get the known and obvious flags on the object,
	 * not including curses or properties of the kind
	 */
	object_flags_known(o_ptr, flags);

	/* MEGA-hack : there needs to be a table of what is obvious in each slot perhaps for each class */
	/* FA on gloves is obvious to mage casters */
	if (object_FA_would_be_obvious(o_ptr))
		flags_mask(flags, OF_SIZE, OF_OBVIOUS_MASK, OF_FREE_ACT, FLAG_END);
	else
		flags_mask(flags, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END);

	flags_clear(flags, OF_SIZE, OF_CURSE_MASK, FLAG_END);
	of_diff(flags, k_ptr->flags);

	if (o_ptr->ident & IDENT_INDESTRUCT)
		return INSCRIP_SPECIAL;
	if ((object_was_sensed(o_ptr) || object_was_worn(o_ptr)) && artifact_p(o_ptr))
		return INSCRIP_SPECIAL;

	/* jewelry does not pseudo */
	if (object_is_jewelry(o_ptr))
		return INSCRIP_NULL;

	/* XXX Eddie should also check for flags with pvals where the pval exceeds
	 * the base pval for things like picks of digging, though for now acid brand gets those
	 */
	if (!of_is_empty(flags))
		return INSCRIP_SPLENDID;

	if (!object_is_known(o_ptr) && !object_was_sensed(o_ptr))
		return INSCRIP_NULL;

	if (ego_item_p(o_ptr))
	{
		/* uncursed bad egos are not excellent */
		if (flags_test(e_info[o_ptr->name2].flags, OF_SIZE, OF_CURSE_MASK, FLAG_END))
			return INSCRIP_STRANGE; /* XXX Eddie need something worse */
		else
			return INSCRIP_EXCELLENT;
	}

	if (o_ptr->to_a == randcalc(k_ptr->to_a, 0, MINIMISE) &&
	    o_ptr->to_h == randcalc(k_ptr->to_h, 0, MINIMISE) &&
		 o_ptr->to_d == randcalc(k_ptr->to_d, 0, MINIMISE))
		return INSCRIP_AVERAGE;

	if (o_ptr->to_a >= randcalc(k_ptr->to_a, 0, MINIMISE) &&
	    o_ptr->to_h >= randcalc(k_ptr->to_h, 0, MINIMISE) &&
	    o_ptr->to_d >= randcalc(k_ptr->to_d, 0, MINIMISE))
		return INSCRIP_MAGICAL;

	if (o_ptr->to_a <= randcalc(k_ptr->to_a, 0, MINIMISE) &&
	    o_ptr->to_h <= randcalc(k_ptr->to_h, 0, MINIMISE) &&
	    o_ptr->to_d <= randcalc(k_ptr->to_d, 0, MINIMISE))
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
	
	
	/* No ID when confused in a bad state */
	if (p_ptr->timed[TMD_CONFUSED]) return;


	/* Notice some things after a while */
	if (turn >= (object_last_wield + 3000))
	{
		object_notice_after_time();
		object_last_wield = 0;
	}


	/* Get improvement rate */
	if (player_has(PF_PSEUDO_ID_IMPROV))
		rate = cp_ptr->sense_base / (p_ptr->lev * p_ptr->lev + cp_ptr->sense_div);
	else
		rate = cp_ptr->sense_base / (p_ptr->lev + cp_ptr->sense_div);

	if (!one_in_(rate)) return;


	/* Check everything */
	for (i = 0; i < ALL_INVEN_TOTAL; i++)
	{
		const char *text = NULL;

		object_type *o_ptr = &p_ptr->inventory[i];
		obj_pseudo_t feel;
		bool cursed;

		bool okay = FALSE;

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Valid "tval" codes */
		switch (o_ptr->tval)
		{
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
			{
				okay = TRUE;
				break;
			}
		}
		
		/* Skip non-sense machines */
		if (!okay) continue;
		
		/* It is known, no information needed */
		if (object_is_known(o_ptr)) continue;
		
		
		/* It has already been sensed, do not sense it again */
		if (object_was_sensed(o_ptr))
		{
			/* Small chance of wielded, sensed items getting complete ID */
			if (!o_ptr->name1 && (i >= INVEN_WIELD) && one_in_(1000))
				do_ident_item(i, o_ptr);

			continue;
		}

		/* Occasional failure on inventory items */
		if ((i < INVEN_WIELD) && one_in_(5)) continue;


		/* Sense the object */
		object_notice_sensing(o_ptr);
		cursed = object_notice_curses(o_ptr);

		/* Get the feeling */
		feel = object_pseudo(o_ptr);

		/* Stop everything */
		disturb(0, 0);

		if (cursed)
			text = "cursed";
		else
			text = inscrip_text[feel];

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

		/* Average pseudo-ID means full ID */
		if (feel == INSCRIP_AVERAGE)
		{
			object_notice_everything(o_ptr);

			message_format(MSG_PSEUDOID, 0,
					"You feel the %s (%c) %s %s average...",
					o_name, index_to_label(i),((i >=
					INVEN_WIELD) ? "you are using" : "in your pack"),
					((o_ptr->number == 1) ? "is" : "are"));
		}
		else
		{
			if (i >= INVEN_WIELD)
			{
				message_format(MSG_PSEUDOID, 0, "You feel the %s (%c) you are %s %s %s...",
							   o_name, index_to_label(i), describe_use(i),
							   ((o_ptr->number == 1) ? "is" : "are"),
				                           text);
			}
			else
			{
				message_format(MSG_PSEUDOID, 0, "You feel the %s (%c) in your pack %s %s...",
							   o_name, index_to_label(i),
							   ((o_ptr->number == 1) ? "is" : "are"),
				                           text);
			}
		}


		/* Set squelch flag as appropriate */
		if (i < INVEN_WIELD)
			p_ptr->notice |= PN_SQUELCH;
		
		
		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);
		
		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
	}
}


