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
#include "object/tvalsval.h"


/** Time last item was wielded */
s32b object_last_wield;



/*** Knowledge accessor functions ***/


/**
 * \returns whether an object counts as "known" due to EASY_KNOW status
 */
static bool easy_know(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	if (k_ptr->aware && (k_ptr->flags[2] & TR2_EASY_KNOW))
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
			(artifact_p(o_ptr) && object_is_known(o_ptr));
}

/**
 * \returns whether the object has been worn/wielded
 */
bool object_was_worn(const object_type *o_ptr)
{
	return o_ptr->ident & IDENT_WORN ? TRUE : FALSE;
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
	return (easy_know(o_ptr) || o_ptr->ident & IDENT_EFFECT) ? TRUE : FALSE;
}

/**
 * \returns whether the object's pval is known to the player
 */
bool object_pval_is_visible(const object_type *o_ptr)
{
	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);
	
	if (o_ptr->ident & IDENT_STORE)
		return TRUE;
	
	if (f[0] & TR0_PVAL_MASK & o_ptr->known_flags[0])
		return TRUE;
	else
		return FALSE;
}

/**
 * \returns whether both the object is both an ego and the player knows it is
 */
bool object_ego_is_visible(const object_type *o_ptr)
{
	if ((o_ptr->tval == TV_LITE) && (ego_item_p(o_ptr)))
		return TRUE;
	if ((o_ptr->ident & IDENT_EGO) || 
	    ((o_ptr->ident & IDENT_KNOWN) && ego_item_p(o_ptr)) || /* XXX Eddie this should go, but necessary to use savefiles with IDENT_KNOWN before IDENT_EGO was added */
	    ((o_ptr->ident & IDENT_STORE) && ego_item_p(o_ptr)))
		return TRUE;
	else
		return FALSE;
}

/**
 * \returns whether the object's attack plusses are known
 */
bool object_attack_plusses_are_visible(const object_type *o_ptr)
{
	if ((o_ptr->ident & IDENT_ATTACK) || (o_ptr->ident & IDENT_STORE))
		return TRUE;
	else
		return FALSE;
}

/**
 * \returns whether the object's defence bonuses are known
 */
bool object_defence_plusses_are_visible(const object_type *o_ptr)
{
	if ((o_ptr->ident & IDENT_DEFENCE) || (o_ptr->ident & IDENT_STORE))
		return TRUE;
	else
		return FALSE;
}


/*
 * \returns whether the player knows whether an object has a given flag
 */
bool object_flag_is_known(const object_type *o_ptr, int idx, u32b flag)
{
	assert ((idx >= 0) && (idx < OBJ_FLAG_N));
	if (easy_know(o_ptr) || (o_ptr->known_flags[idx] & flag))
		return TRUE;
	else
		return FALSE;
}


/*
 * \returns whether it is possible an object has a high resist given the
 *          player's current knowledge
 */
bool object_high_resist_is_possible(const object_type *o_ptr)
{
	u32b flags[OBJ_FLAG_N];
	object_flags(o_ptr, flags);

	if (flags[1] & o_ptr->known_flags[1] & TR1_HIGH_RESIST_MASK)
		return TRUE;
	else if ((o_ptr->known_flags[1] & TR1_HIGH_RESIST_MASK)
						== TR1_HIGH_RESIST_MASK)
		return FALSE;
	else
		return TRUE;
}




/*
 * Checks for additional knowledge implied by what the player already knows.
 *
 * \param o_ptr is the object to check
 */
static void object_check_for_ident(object_type *o_ptr)
{
	u32b flags[OBJ_FLAG_N];
	object_flags(o_ptr, flags);
	
	int i;
	
	/* Some flags are irrelevant or never learned or too hard to learn */
	flags[2] &= ~(TR2_INSTA_ART | TR2_EASY_KNOW | TR2_HIDE_TYPE | TR2_SHOW_MODS | TR2_IGNORE_ACID | TR2_IGNORE_ELEC | TR2_IGNORE_FIRE | TR2_IGNORE_COLD);
	
	for (i = 0; i < OBJ_FLAG_N; i++)
	{
		if (flags[i] != (flags[i] & o_ptr->known_flags[i]))
			return;
	}
	
	/* If we know attack bonuses, and defence bonuses, and effect, then
	 * we effectively know everything, so mark as such */
	if ((o_ptr->ident & IDENT_ATTACK || (o_ptr->ident & IDENT_SENSE && o_ptr->to_h == 0 && o_ptr->to_d == 0)) &&
	    (o_ptr->ident & IDENT_DEFENCE || (o_ptr->ident & IDENT_SENSE && o_ptr->to_a == 0)) &&
	    (o_ptr->ident & IDENT_EFFECT || !object_effect(o_ptr)))
	{
		object_notice_everything(o_ptr);
	}
	
	/* We still know all the flags, so we still know if it's an ego */
	else if (ego_item_p(o_ptr))
	{
		o_ptr->ident |= IDENT_EGO;
	}
}


/**
 * Mark an object's flavour as as one the player is aware of.
 *
 * \param o_ptr is the object whose flavour should be marked as aware
 */
void object_flavor_aware(object_type *o_ptr)
{
	int i;

	if (k_info[o_ptr->k_idx].aware) return;
	k_info[o_ptr->k_idx].aware = TRUE;

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
			lite_spot(floor_o_ptr->iy, floor_o_ptr->ix);
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


/**
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
	o_ptr->ident |= (IDENT_KNOWN | IDENT_ATTACK | IDENT_DEFENCE |
					 IDENT_SENSE | IDENT_EFFECT | IDENT_WORN);

	/* Artifact has now been seen */
	if (a_ptr)
		a_ptr->seen = a_ptr->everseen = TRUE;

	/* Mark ego as known */
	if (ego_item_p(o_ptr))
		o_ptr->ident |= IDENT_EGO;

	/* Know all flags there are to be known */
	object_know_all_flags(o_ptr);
}


/**
 * Notice that an object is indestructible.
 */
void object_notice_indestructible(object_type *o_ptr)
{
	o_ptr->ident |= IDENT_INDESTRUCT;
}


/*
 * Notice the ego on an ego item.
 */
void object_notice_ego(object_type *o_ptr)
{
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		/* XXX Eddie print a message on notice ego if not already noticed? */
		/* XXX Eddie should we do something about everseen of egos here? */

		/* learn all flags except random abilities */
		u32b learned_flags[OBJ_FLAG_N];
		int i;

		for (i = 0; i < OBJ_FLAG_N; i++)
			learned_flags[i] = (u32b) -1;

		switch (e_ptr->xtra)
		{
			case OBJECT_XTRA_TYPE_NONE:
				break;
			case OBJECT_XTRA_TYPE_SUSTAIN:
				learned_flags[ego_xtra_sustain_idx()] &= ~(ego_xtra_sustain_list());
				break;
			case OBJECT_XTRA_TYPE_RESIST:
				learned_flags[ego_xtra_resist_idx()] &= ~(ego_xtra_resist_list());
				break;
			case OBJECT_XTRA_TYPE_POWER:
				learned_flags[ego_xtra_power_idx()] &= ~(ego_xtra_power_list());
				break;
			default:
				assert(0);
		}

		for (i = 0; i < OBJ_FLAG_N; i++)
			o_ptr->known_flags[i] |=
					(learned_flags[i] | e_ptr->flags[i]);

		/* XXX Eddie should check for ident be allowed if repairing?  For now, only repair is things currently IDENT_KNOWN in savefile, but in future if something changes might need to repair based upon arbitrary player knowledge */
		if (!(o_ptr->ident & IDENT_EGO))
		{
			o_ptr->ident |= IDENT_EGO;
			object_check_for_ident(o_ptr);
		}
	}
}


/*
 * Mark an object as sensed.
 */
void object_notice_sensing(object_type *o_ptr)
{
	/* XXX Eddie can be called to repair knowledge, should print messages only if IDENT_SENSE prev not set */
	if (!object_was_sensed(o_ptr))
	{
		artifact_type *a_ptr = artifact_of(o_ptr);
		if (a_ptr) a_ptr->seen = a_ptr->everseen = TRUE;

		o_ptr->ident |= IDENT_SENSE;
		object_check_for_ident(o_ptr);
	}

	/* for repair purposes, notice curses even on prev sensed object */
	object_notice_curses(o_ptr);
}



/**
 * Notice the "effect" from activating an object.
 *
 * \param o_ptr is the object to become aware of
 */
void object_notice_effect(object_type *o_ptr)
{
	o_ptr->ident |= IDENT_EFFECT;

	/* noticing an effect gains awareness */
	if (!object_flavor_is_aware(o_ptr))
		object_flavor_aware(o_ptr);
}


/*
 * Notice slays on a particular object.
 *
 * \param known_f0 is the list of flags to notice
 */
void object_notice_slays(object_type *o_ptr, u32b known_f0)
{
	object_notice_flags(o_ptr, 0, known_f0);

	u32b flags[OBJ_FLAG_N];
	object_flags(o_ptr, flags);

	/* if you learn a slay, learn the ego */
	if (EASY_LEARN && (flags[0] & known_f0))
		object_notice_ego(o_ptr);
	object_check_for_ident(o_ptr);
}


static void object_notice_defence_plusses(object_type *o_ptr)
{
	if (!o_ptr->k_idx) return;
	if (o_ptr->ident & IDENT_DEFENCE)
		return;

	o_ptr->ident |= IDENT_DEFENCE;
	object_check_for_ident(o_ptr);

	if (o_ptr->ac || o_ptr->to_a)
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);
		message_format(MSG_PSEUDOID, 0,
				"You feel your better know the %s you are wearing.",
				o_name);
	}
}


void object_notice_attack_plusses(object_type *o_ptr)
{
	if (!o_ptr->k_idx) return;
	if (o_ptr->ident & IDENT_ATTACK)
		return;

	o_ptr->ident |= IDENT_ATTACK;
	object_check_for_ident(o_ptr);

	if (wield_slot(o_ptr) == INVEN_WIELD)
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);
		message_format(MSG_PSEUDOID, 0,
				"You feel your better know the %s you are attacking with.",
				o_name);
	}
	else if ((o_ptr->to_d || o_ptr->to_h) &&
			!((o_ptr->tval == TV_HARD_ARMOR || o_ptr->tval == TV_SOFT_ARMOR) && (o_ptr->to_h < 0)))
	{
		char o_name[80];

		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);
		message_format(MSG_PSEUDOID, 0, "Your %s glows.", o_name);
	}
}


typedef struct
{
	int flagset;
	u32b flag;
	const char *msg;
} flag_message_t;

static const flag_message_t msgs[] =
{
	{ 0, TR0_SEARCH,	"Your %s glows." },
	{ 1, 0xffffffff,	"Your %s glows." },
	{ 2, TR2_FREE_ACT,	"Your %s glows." },
	{ 2, TR2_HOLD_LIFE,	"Your %s glows." },
	{ 2, TR2_DRAIN_EXP,	"You feel your %s drain your life." },
	{ 2, TR2_FEATHER,	"Your %s slows your fall." },
	{ 2, TR2_IMPACT,	"Your %s causes an earthquake!" },
	{ 2, TR2_TELEPORT,	"Your %s teleports you." },
};


/*
 * Notice a set of flags
 *
 * this is non-standard -- everything else notices an individual flag
 */
void object_notice_flags(object_type *o_ptr, int flagset, u32b flags)
{
	if (flags & (~o_ptr->known_flags[flagset]))
	{
		o_ptr->known_flags[flagset] |= flags;
		/* XXX Eddie don't want infinite recursion if object_check_for_ident sets more flags, but maybe this will interfere with savefile repair */
		object_check_for_ident(o_ptr);
	}
}


/**
 * Notice curses on an object.
 *
 * \param o_ptr is the object to notice curses on
 */
bool object_notice_curses(object_type *o_ptr)
{
	u32b curses;
	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);

	/* Know whatever curse flags there are to know */
	curses = (f[2] & TR2_CURSE_MASK);

	/* give knowledge of which curses are present */
	object_notice_flags(o_ptr, 2, TR2_CURSE_MASK);

	object_check_for_ident(o_ptr);
	p_ptr->notice |= PN_SQUELCH;

	return (curses ? TRUE : FALSE);
}


/**
 * Notice things which happen on defending.
 */
void object_notice_on_defend(void)
{
	int i;
	
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		object_notice_defence_plusses(&inventory[i]);
	
	/* XXX Eddie print message? */
	
	return;
}


/*
 * Determine whether a weapon or missile weapon is obviously {excellent} when worn.
 *
 * When repairing knowledge, do not print messages.
 */
/* XXX Eddie should messages be adhoc all over the place?  perhaps the main loop should check for change in inventory/wieldeds and all messages be printed from one place */
void object_notice_on_wield(object_type *o_ptr)
{
	u32b f[OBJ_FLAG_N];
	bool obvious = FALSE;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];


	/* Save time of wield for later */
	object_last_wield = turn;

	/* Only deal with un-ID'd items */
	if (object_is_known(o_ptr)) return;

	/* Wear it */
	o_ptr->ident |= IDENT_WORN;
	object_flavor_tried(o_ptr);
	
	if (obj_is_lite(o_ptr) && ego_item_p(o_ptr))
		object_notice_ego(o_ptr);

	if (object_flavor_is_aware(o_ptr))
	{
		if (easy_know(o_ptr))
		{
			object_notice_everything(o_ptr);
			return;
		}

		/* We currently always know all flags on aware jewelry */
		else if (object_is_jewelry(o_ptr))
		{
			object_know_all_flags(o_ptr);
		}
	}

	/* notice all artifacts upon wield */
	if (artifact_p(o_ptr))
		object_notice_sensing(o_ptr);
	
	/* Extract the flags */
	object_flags(o_ptr, f);
	
	/* Find obvious things */
	if (f[0] & TR0_OBVIOUS_MASK) obvious = TRUE;
	if (f[2] & TR2_OBVIOUS_MASK & ~TR2_CURSE_MASK) obvious = TRUE;
	
	/* XXX Eddie this next block should go when learning cascades with flags */
	bool obvious_without_activate = FALSE;
	if (f[0] & TR0_OBVIOUS_MASK) obvious_without_activate = TRUE;
	if (f[2] & TR2_OBVIOUS_MASK & ~TR2_CURSE_MASK)
		obvious_without_activate = TRUE;
	
	bool to_sense = FALSE;
	if (f[0] & TR0_OBVIOUS_MASK & ~k_ptr->flags[0]) to_sense = TRUE;
	if (f[2] & TR2_OBVIOUS_MASK & ~k_ptr->flags[2]) to_sense = TRUE;
	
	/* XXX Eddie should these next NOT call object_check_for_ident due to worries about repairing? */
	o_ptr->known_flags[0] |= TR0_OBVIOUS_MASK;
	o_ptr->known_flags[2] |= TR2_OBVIOUS_MASK;
	
	object_check_for_ident(o_ptr);
	
	if (!obvious) return;
	
	/* something obvious should be immediately sensed */
	if (to_sense)
		object_notice_sensing(o_ptr);
	/* XXX Eddie is above necessary here?  done again at end of function */
	
	if (EASY_LEARN && object_is_jewelry(o_ptr) && obvious_without_activate)
	{
		/* XXX Eddie this is a small hack, but jewelry with anything noticeable really is obvious */
		/* XXX Eddie learn =soulkeeping vs =bodykeeping when notice sustain_str */
		object_flavor_aware(o_ptr);
		object_check_for_ident(o_ptr);
	}
	
	/* Messages */
	if (wield_slot(o_ptr) == INVEN_WIELD)
	{
		if (f[0] & TR0_BRAND_POIS)
			msg_print("It seethes with poison!");
		if (f[0] & TR0_BRAND_ELEC)
			msg_print("It crackles with electricity!");
		if (f[0] & TR0_BRAND_FIRE)
			msg_print("It flares with fire!");
		if (f[0] & TR0_BRAND_COLD)
			msg_print("It coats itself in ice!");
		if (f[0] & TR0_BRAND_ACID)
			msg_print("It starts spitting acid!");
	}
	
	/* XXX Eddie need to add stealth here, also need to assert/double-check everything is covered */
	
	if (f[0] & TR0_STR)
		msg_format("You feel %s!", o_ptr->pval > 0 ? "stronger" : "weaker");
	if (f[0] & TR0_INT)
		msg_format("You feel %s!", o_ptr->pval > 0 ? "smarter" : "more stupid");
	if (f[0] & TR0_WIS)
		msg_format("You feel %s!", o_ptr->pval > 0 ? "wiser" : "more naive");
	if (f[0] & TR0_DEX)
		msg_format("You feel %s!", o_ptr->pval > 0 ? "more dextrous" : "clumsier");
	if (f[0] & TR0_CON)
		msg_format("You feel %s!", o_ptr->pval > 0 ? "healthier" : "sicklier");
	if (f[0] & TR0_CHR)
		msg_format("You feel %s!", o_ptr->pval > 0 ? "cuter" : "uglier");
	if (f[0] & TR0_SPEED)
		msg_format("You feel strangely %s.", o_ptr->pval > 0 ? "quick" : "sluggish");
	if (f[0] & (TR0_BLOWS | TR0_SHOTS))
		msg_format("Your hands %s", o_ptr->pval > 0 ? "tingle!" : "ache.");
	if (f[0] & TR0_INFRA)
		msg_format("Your eyes tingle.");
	
	if (f[2] & TR2_LITE)
		msg_print("It glows!");
	if (f[2] & TR2_TELEPATHY)
		msg_print("Your mind feels strangely sharper!");
	
	/* learn the ego on any brand or slay */
	if (EASY_LEARN && f[0] & TR0_OBVIOUS_MASK & TR0_ALL_SLAYS)
		if (ego_item_p(o_ptr))
		/* XXX Eddie somewhat inconsistent, style is to notice even when property is not present */
			object_notice_ego(o_ptr);
	
	/* Remember the flags */
	object_notice_sensing(o_ptr);
	
	/* XXX Eddie should we check_for_ident here? */
}


static const flag_message_t notice_msgs[] =
{
	{ 0, TR0_STEALTH,	"Your %s glows." },
	{ 2, TR2_SLOW_DIGEST,	"You feel your %s slow your metabolism." },
	{ 2, TR2_REGEN,		"You feel your %s speed up your recovery." },
	{ 2, TR2_AGGRAVATE,	"You feel your %s aggravate things around you." },
	{ 2, TR2_IMPAIR_HP,	"You feel your %s slow your recovery." },
	{ 2, TR2_IMPAIR_MANA,	"You feel your %s slow your mana recovery." },
};


/**
 * Notice things about an object that would be noticed in time.
 */
static void object_notice_after_time(void)
{
	int i;
	size_t j;

	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];
		char o_name[80];
		u32b f[OBJ_FLAG_N];

		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);
		object_flags(o_ptr, f);

		for (j = 0; j < N_ELEMENTS(notice_msgs); j++)
		{
			int set = notice_msgs[j].flagset;
			u32b flag = notice_msgs[j].flag;

			if ((f[set] & flag) &&
					!(o_ptr->known_flags[set] & flag))
			{
				/* Notice the flag */
				object_notice_flags(o_ptr, set, flag);

				/* Message */
				msg_format(notice_msgs[j].msg, o_name);

				if (object_is_jewelry(o_ptr) &&
						(!object_effect(o_ptr) || o_ptr->ident & IDENT_EFFECT))
				{
					/* XXX this is a small hack, but jewelry with anything noticeable really is obvious */
					/* XXX except, wait until learn activation if that is only clue */
					object_flavor_aware(o_ptr);
					object_check_for_ident(o_ptr);
				}
			}
			else
			{
				/* Notice that the flag is not present */
				object_notice_flags(o_ptr, set, flag);
			}
		}
		/* XXX Eddie the object_notice_flags should presumably come out of the if/else and next check not necessary, fix later */
		object_check_for_ident(o_ptr);
	}	
}


/**
 * Notice a given special flag on wielded items.
 *
 * \param flagset is the set the flag is in
 * \param flag is the flag to notice
 */
/* XXX Eddie like above, should specify whether \param flag is allowed to be a set of |ed flags */
void wieldeds_notice_flag(int flagset, u32b flag)
{
	int i;
	size_t j;
	
	/* XXX Eddie need different naming conventions for starting wieldeds at INVEN_WIELD vs INVEN_WIELD+2 */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];
		u32b f[OBJ_FLAG_N];
		
		object_flags(o_ptr, f);
		if ((f[flagset] & flag) &&
			!(o_ptr->known_flags[flagset] & flag))
		{
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, FALSE,
						ODESC_BASE);
			
			/* Notice flags */
			object_notice_flags(o_ptr, flagset, flag);
			
			/* XXX Eddie should this go before noticing the flag to avoid learning twice? */
			if (EASY_LEARN && object_is_jewelry(o_ptr))
			{
				/* XXX Eddie EASY_LEARN Possible concern: gets =teleportation just from +2 speed */
				object_flavor_aware(o_ptr);
				object_check_for_ident(o_ptr);
			}
			
			for (j = 0; j < N_ELEMENTS(msgs); j++)
			{
				if (msgs[j].flagset == flagset &&
					(msgs[j].flag & flag))
					msg_format(msgs[j].msg, o_name);
			}
		}
		else
		{
			/* Notice that flag is absent */
			object_notice_flags(o_ptr, flagset, flag);
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
		object_notice_attack_plusses(&inventory[i]);

	/* XXX Eddie print message? */
	/* XXX Eddie do we need to do more about ammo? */

	return;
}


/*
 * Notice slays on wielded items other than melee and bow slots.
 *
 * \param known_f0 is the list of flags to notice
 */
void wieldeds_notice_slays(u32b known_f0)
{
	int i;
	
	for (i = INVEN_WIELD+2; i < INVEN_TOTAL; i++)
		object_notice_slays(&inventory[i], known_f0);
}



/*
 * Given an object, return a short identifier which gives some idea of what
 * the item is.
 */
obj_pseudo_t object_pseudo(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	u32b flags[OBJ_FLAG_N];
	object_flags(o_ptr, flags);


	if (o_ptr->ident & IDENT_INDESTRUCT)
		return INSCRIP_SPECIAL;
	if ((object_was_sensed(o_ptr) || object_was_worn(o_ptr)) && artifact_p(o_ptr))
		return INSCRIP_SPECIAL;

	/* jewelry does not pseudo */
	if (object_is_jewelry(o_ptr))
		return INSCRIP_NULL;

	/* XXX Eddie should also check for flags with pvals where the pval exceeds the base pval for things like picks of digging, though for now acid brand gets those */
	if ((o_ptr->known_flags[0] & flags[0] & ~k_ptr->flags[0] & TR0_OBVIOUS_MASK) ||
	    (o_ptr->known_flags[2] & flags[2] & ~k_ptr->flags[2] & TR2_OBVIOUS_MASK & ~TR2_CURSE_MASK))
		return INSCRIP_SPLENDID;

	if (!object_is_known(o_ptr) && !object_was_sensed(o_ptr))
		return INSCRIP_NULL;

	if (ego_item_p(o_ptr))
	{
		/* uncursed bad egos are not excellent */
		if (e_info[o_ptr->name2].flags[2] & TR2_CURSE_MASK)
			return INSCRIP_STRANGE; /* XXX Eddie need something worse */
		else
			return INSCRIP_EXCELLENT;
	}

	if (o_ptr->to_a == k_ptr->to_a && o_ptr->to_h == k_ptr->to_h &&
			o_ptr->to_d == k_ptr->to_d)
		return INSCRIP_AVERAGE;

	if (o_ptr->to_a >= k_ptr->to_a && o_ptr->to_h >= k_ptr->to_h &&
			o_ptr->to_d >= k_ptr->to_d)
		return INSCRIP_MAGICAL;

	if (o_ptr->to_a <= k_ptr->to_a && o_ptr->to_h <= k_ptr->to_h &&
			o_ptr->to_d <= k_ptr->to_d)
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
	if (cp_ptr->flags & CF_PSEUDO_ID_IMPROV)
		rate = cp_ptr->sense_base / (p_ptr->lev * p_ptr->lev + cp_ptr->sense_div);
	else
		rate = cp_ptr->sense_base / (p_ptr->lev + cp_ptr->sense_div);
	
	if (!one_in_(rate)) return;
	
	
	/* Check everything */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		const char *text = NULL;

		object_type *o_ptr = &inventory[i];
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
		if (o_ptr->ident & IDENT_SENSE)
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

		object_desc(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);

		/* Average pseudo-ID means full ID */
		if (feel == INSCRIP_AVERAGE)
		{
			object_notice_everything(o_ptr);

			message_format(MSG_PSEUDOID, 0,
					"You feel the %s (%c) in your pack %s average...",
					o_name, index_to_label(i),
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
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);
		
		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
	}
}


