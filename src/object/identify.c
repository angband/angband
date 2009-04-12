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
 * Mark an object as "tried".
 *
 * \param o_ptr is the object to mark
 */
void object_tried(object_type *o_ptr)
{
	k_info[o_ptr->k_idx].tried = TRUE;
	o_ptr->ident |= IDENT_TRIED;
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
	}	

	return;
}


/**
 * Notice a given special flag on wielded items.
 *
 * \param flagset is the set the flag is in
 * \param flag is teh flag to notice
 */
void object_notice_flag(int flagset, u32b flag)
{
	return;
}


/**
 * Notice curses on an object.
 *
 * \param o_ptr is the object to notice curses on
 */
bool object_notice_curses(object_type *o_ptr)
{
	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);

	u32b curses = (f[2] & TR2_CURSE_MASK);

	/* Know whatever curse flags there are to know */
	o_ptr->known_flags[2] |= curses;

	p_ptr->notice |= PN_SQUELCH;

	return (curses ? TRUE : FALSE);
}


/**
 * Notice things about an object that would be noticed in time.
 */
static void object_notice_after_time(void)
{
	/* Notice: */
	/* SLOW_DIGEST, REGEN, IMPAIR_HP, IMPAIR_SP, AGGRAVATE, STEALTH */
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

	/* Strange messages for strange properties (this way, we don't have
	 * to give them when the item is identified).
	 *
	 * Perhaps these messages should be in a new edit file?
	 */

	if (f[0] & TR0_STR)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "strong" : "weak");
	if (f[0] & TR0_INT)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "smart" : "stupid");
	if (f[0] & TR0_WIS)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "wise" : "naive");
	if (f[0] & TR0_DEX)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "dextrous" : "clumsy");
	if (f[0] & TR0_CON)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "healthy" : "sickly");
	if (f[0] & TR0_CHR)
		msg_format ("You feel strangely %s!", o_ptr->pval > 0 ? "cute" : "ugly");
	if (f[0] & TR0_STEALTH)
		msg_format ("You feel strangely %s.", o_ptr->pval > 0 ? "stealthy" : "noisy");
	if (f[0] & TR0_SPEED)
		msg_format ("You feel strangely %s.", o_ptr->pval > 0 ? "quick" : "sluggish");
	if (f[0] & (TR0_BLOWS | TR0_SHOTS))
		msg_format ("Your hands strangely %s!", o_ptr->pval > 0 ? "tingle!" : "ache.");
	if (f[2] & TR2_LITE)
		msg_print("It shines strangely!");
	if (f[2] & TR2_TELEPATHY)
		msg_print("Your mind feels strangely sharper!");

	/* Remember the flags */
	o_ptr->known_flags[0] |= (f[0] & TR0_OBVIOUS_MASK);
	o_ptr->known_flags[2] |= (f[2] & TR2_OBVIOUS_MASK);
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
	else if (!(o_ptr->ident & IDENT_SENSE))
		return INSCRIP_NULL;
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

