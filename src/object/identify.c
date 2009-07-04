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


/**
 * Whether an object counts as "known" due to EASY_KNOW status
 */
/* XXX Eddie should have a different name */
static bool easy_know(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* XXX Eddie should I really access k_ptr->aware directly? */
	if (k_ptr->aware && k_ptr->flags[2] & TR2_EASY_KNOW)
		return TRUE;
	else
		return FALSE;
}


/*
 * Recall if item has ever been worn.  Alternative to "tried".
 */
bool object_was_worn(const object_type *o_ptr)
{
	if (o_ptr->ident & IDENT_WORN)
		return TRUE;
	else
		return FALSE;
}


/*
 * Whether an object has known pseudo value.
 */
bool object_was_sensed(const object_type *o_ptr)
{
	/* XXX Eddie must consider whether ident sets IDENT_SENSE and whether to repair it, then first test goes away */
	if (object_known_p(o_ptr))
		return TRUE;

	if (o_ptr->ident & IDENT_SENSE)
		return TRUE;
	else
		return FALSE;
}


/*
 * Whether the flavor of an object was tried.  A new ring can be tried without having been worn.
 */
bool object_flavor_was_tried(const object_type *o_ptr)
{
	return k_info[o_ptr->k_idx].tried;
}


/*
 * Whether the player knows the value of an object flag,
 * is TRUE when the player knows a flag is not present as well as knowing it is present
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
 * Whether it is possible an object has a high resist, given the knowledge so far
 */
bool object_high_resist_is_possible(const object_type *o_ptr)
{
#if 0
	u32b flags[OBJ_FLAG_N];
	object_flags(o_ptr, flags);

	if (flags[1] & o_ptr->known_flags[1] & TR1_HIGH_RESIST_MASK)
		return TRUE;
	else if ((o_ptr->known_flags[1] & TR1_HIGH_RESIST_MASK) == TR1_HIGH_RESIST_MASK)
		return FALSE;
	else
		return TRUE;
#endif

	/* XXX for now */
	return TRUE;
}


/**
 * Mark as object as fully known, a.k.a identified. 
 *
 * \param o_ptr is the object to mark as identified
 */
void object_known(object_type *o_ptr)
{
	u32b flags[OBJ_FLAG_N];

	/* The object is not sensed, or "empty" */
	o_ptr->ident &= ~(IDENT_SENSE | IDENT_EMPTY);

	/* Mark as known */
	o_ptr->ident |= IDENT_KNOWN;

	/* Know all flags there are to be known */
	object_flags(o_ptr, flags);
	memcpy(o_ptr->known_flags, flags, sizeof(flags));
}

/**
 * Mark an object as "aware".
 *
 * \param o_ptr is the object to become aware of
 */
void object_aware(object_type *o_ptr)
{
	int i;

	/* Fully aware of the effects */
	k_info[o_ptr->k_idx].aware = TRUE;

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
 * Set the ID flag on an object if known and actual flags are the same.
 */
static void tweak_id(object_type *o_ptr)
{
	u32b f[OBJ_FLAG_N];

	object_flags(o_ptr, f);
	o_ptr->known_flags[2] |= (f[2] & TR2_EASY_KNOW);

	if (memcmp(f, o_ptr->known_flags, sizeof(f)) == 0)
	{
		object_aware(o_ptr);
		object_known(o_ptr);
	}
}



/**
 * Mark an object as "tried".
 *
 * \param o_ptr is the object to mark
 */
void object_tried(object_type *o_ptr)
{
	k_info[o_ptr->k_idx].tried = TRUE;
}


/**
 * Notice slays on wielded items, and additionally one kind of ammo.
 *
 * \param known_f0 is the list of flags to notice
 * \param inven_idx is the index of the inventory item to notice, or -1
 */
void object_notice_slays(u32b known_f0, int inven_idx)
{
	int i;

	/* XXX pay attention to inven_idx */

	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];

		o_ptr->known_flags[0] |= known_f0;
		tweak_id(o_ptr);
	}	

	return;
}


bool object_activation_is_visible(const object_type *o_ptr)
{
	if (!obj_can_wear(o_ptr))
	{
		/* XXX Eddie need function _if_charges ? or maybe the activation code should use something else */
		switch(o_ptr->tval)
		{
			case TV_WAND:
			case TV_STAFF:
			case TV_ROD:
				break;
			default:
				return FALSE;
		}

		if (object_aware_p(o_ptr))
			return TRUE;
		/* This would work now, but could fail if we add artifact rods
		else
			return FALSE;
		*/
	}

	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);

	if ((f[2] & TR2_ACTIVATE) && (easy_know(o_ptr) || o_ptr->known_flags[2] & TR2_ACTIVATE))
		return TRUE;
	else
		return FALSE;
}

bool object_effect_is_known(const object_type *o_ptr)
{
	if (easy_know(o_ptr) || o_ptr->ident & IDENT_EFFECT)
		return TRUE;
	else
		return FALSE;
}

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

bool object_ego_is_visible(const object_type *o_ptr)
{
	if ((o_ptr->tval == TV_LITE) && (ego_item_p(o_ptr)))
		return TRUE;
	if (((o_ptr->ident & IDENT_KNOWN) && ego_item_p(o_ptr)) || /* XXX Eddie this should go, but necessary to use savefiles with IDENT_KNOWN before IDENT_EGO was added */
			((o_ptr->ident & IDENT_STORE) && ego_item_p(o_ptr)))
		return TRUE;
	else
		return FALSE;
}

bool object_attack_plusses_are_visible(const object_type *o_ptr)
{
	/* XXX Eddie next ref to IDENT_STORE not clearly correct -- perhaps should guarantee store items have IDENT_ATTACK set */
	if (object_known_p(o_ptr) ||
			(o_ptr->ident & IDENT_ATTACK) || (o_ptr->ident & IDENT_STORE))
		return TRUE;
	else
		return FALSE;
}


bool object_defence_plusses_are_visible(const object_type *o_ptr)
{
	/* XXX Eddie next IDENT_STORE not clear -- perhaps should guarantee store items have IDENT_DEFENCE set */
	if (object_known_p(o_ptr) ||
			(o_ptr->ident & IDENT_DEFENCE) || (o_ptr->ident & IDENT_STORE))
		return TRUE;
	else
		return FALSE;
}


typedef struct
{
	int flagset;
	u32b flag;
	const char *msg;
} flag_message_t;

static const flag_message_t msgs[] =
{
	{ 0, TR0_SEARCH,	"Your %s assists your searching." },
	{ 1, 0xffffffff,	"Your %s glows." },
	{ 2, TR2_FREE_ACT,	"Your %s glows." },
	{ 2, TR2_HOLD_LIFE,	"Your %s glows." },
	{ 2, TR2_DRAIN_EXP,	"You feel your %s drain your life." },
	{ 2, TR2_FEATHER,	"Your %s slows your fall." },
	{ 2, TR2_IMPACT,	"Your %s causes an earthquake!" },
	{ 2, TR2_TELEPORT,	"Your %s teleports you." },
};



/**
 * Notice a given special flag on wielded items.
 *
 * \param flagset is the set the flag is in
 * \param flag is teh flag to notice
 */
void wieldeds_notice_flag(int flagset, u32b flag)
{
	int i;
	size_t j;

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
			o_ptr->known_flags[flagset] |= flag;
			tweak_id(o_ptr);

			for (j = 0; j < N_ELEMENTS(msgs); j++)
			{
				if (msgs[j].flagset == flagset &&
						(msgs[j].flag & flag))
					msg_format(msgs[j].msg, o_name);
			}
		}
	}	

	return;
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

	curses = (f[2] & TR2_CURSE_MASK);

	/* Know whatever curse flags there are to know */
	o_ptr->known_flags[2] |= curses;
	tweak_id(o_ptr);

	p_ptr->notice |= PN_SQUELCH;

	return (curses ? TRUE : FALSE);
}



static const flag_message_t notice_msgs[] =
{
	{ 0, TR0_STEALTH,	"You feel your %s affect your stealth." },
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
	int i, j;

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
				o_ptr->known_flags[set] |= flag;
				tweak_id(o_ptr);

				/* Message */
				msg_format(notice_msgs[j].msg, o_name);
			}
		}
	}	
}


/**
 * Notice things which happen on attacking.
 */
void object_notice_on_attack(void)
{
	int i;

	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &inventory[i];

		o_ptr->ident |= IDENT_ATTACK;
		tweak_id(o_ptr);
	}

	/* XXX print message? */
	/* XXX do we need to do more about ammo? */

	return;
}


/*
 * Determine whether a weapon or missile weapon is obviously {excellent} when worn.
 */
void object_notice_on_wield(object_type *o_ptr)
{
	u32b f[OBJ_FLAG_N];
	bool obvious = FALSE;

	/* Save time of wield for later */
	object_last_wield = turn;

	/* Notice: */
	/* Damage dice and bonuses for warrior-types */



	/* Only deal with un-ID'd items */
	if (object_known_p(o_ptr)) return;

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Find obvious things */
	if (f[0] & TR0_OBVIOUS_MASK) obvious = TRUE;
	if (f[2] & TR2_OBVIOUS_MASK) obvious = TRUE;
	if (!obvious) return;

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

	/* Remember the flags */
	o_ptr->ident |= IDENT_SENSE;
	o_ptr->known_flags[0] |= (f[0] & TR0_OBVIOUS_MASK);
	o_ptr->known_flags[2] |= (f[2] & TR2_OBVIOUS_MASK);
	tweak_id(o_ptr);
}


/*
 * Given an object, return a short identifier which gives some idea of what
 * the item is.
 */
obj_pseudo_t object_pseudo(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	if ((o_ptr->known_flags[0] & TR0_OBVIOUS_MASK) ||
			(o_ptr->known_flags[2] & TR2_OBVIOUS_MASK))
		return INSCRIP_SPLENDID;
	else if (o_ptr->ident & IDENT_INDESTRUCT)
		return INSCRIP_SPECIAL;
	else if (!(o_ptr->ident & IDENT_SENSE) && !object_known_p(o_ptr))
		return INSCRIP_UNKNOWN;
	else if (artifact_p(o_ptr))
		return INSCRIP_SPECIAL;
	else if (ego_item_p(o_ptr))
		return INSCRIP_EXCELLENT;
	else if (o_ptr->to_a == k_ptr->to_a && o_ptr->to_h == k_ptr->to_h &&
			o_ptr->to_d == k_ptr->to_d)
		return INSCRIP_AVERAGE;
	else if (o_ptr->to_a >= k_ptr->to_a && o_ptr->to_h >= k_ptr->to_h &&
			o_ptr->to_d >= k_ptr->to_d)
		return INSCRIP_MAGICAL;
	else if (o_ptr->to_a <= k_ptr->to_a && o_ptr->to_h <= k_ptr->to_h &&
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
		if (object_known_p(o_ptr)) continue;
		
		
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
		o_ptr->ident |= IDENT_SENSE;
		cursed = object_notice_curses(o_ptr);

		/* Get the feeling */
		feel = object_pseudo(o_ptr);

		/* Stop everything */
		disturb(0, 0);

		if (cursed)
			text = "cursed";
		else
			text = inscrip_text[feel];

		/* Average pseudo-ID means full ID */
		if (feel == INSCRIP_AVERAGE)
		{
			do_ident_item(i, o_ptr);
		}
		else
		{
			object_desc(o_name, sizeof(o_name), o_ptr, FALSE, ODESC_BASE);

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

