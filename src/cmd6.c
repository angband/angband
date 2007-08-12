/*
 * File: cmd6.c
 * Purpose: Eating/quaffing/reading/aiming/staving/zapping/activating
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007 Andrew Sidwell
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
#include "effects.h"


/* Types of item use */
typedef enum
{
	USE_TIMEOUT,
	USE_CHARGE,
	USE_SINGLE
} use_type;


/*
 * Hook to determine if an object is activatable
 */
static bool item_tester_hook_activate(const object_type *o_ptr)
{
	u32b f1, f2, f3;

	/* Not known */
	if (!object_known_p(o_ptr)) return (FALSE);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);
	
	/* Check the recharge */
	if (o_ptr->timeout) return (FALSE);

	/* Check activation flag */
	if (f3 & (TR3_ACTIVATE)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Hook to determine if an object is zappable
 */
static bool item_tester_hook_zap(const object_type *o_ptr)
{
	const object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Non-rods are out */
	if (o_ptr->tval != TV_ROD) return FALSE;

	/* All still charging? */
	if (o_ptr->number <= (o_ptr->timeout + (k_ptr->time_base - 1)) / k_ptr->time_base) return FALSE;

	/* Otherwise OK */
	return TRUE;
}


/*
 * Determine if the player can read scrolls.
 */
static bool can_read_scroll(void)
{
	if (p_ptr->timed[TMD_BLIND])
	{
		msg_print("You can't see anything.");
		return FALSE;
	}

	if (no_lite())
	{
		msg_print("You have no light to read by.");
		return FALSE;
	}

	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg_print("You are too confused to read!");
		return FALSE;
	}

	if (p_ptr->timed[TMD_AMNESIA])
	{
		msg_print("You can't remember how to read!");
		return FALSE;
	}

	return TRUE;
}


/*
 * Check to see if the player can use a rod/wand/staff/activatable object.
 */
static int check_devices(object_type *o_ptr)
{
	int lev, chance;
	const char *msg;
	const char *what = NULL;

	/* Get the right string */
	switch (o_ptr->tval)
	{
		case TV_ROD:   msg = "zap the rod";   break;
		case TV_WAND:  msg = "use the wand";  what = "wand";  break;
		case TV_STAFF: msg = "use the staff"; what = "staff"; break;
		default:       msg = "activatee it";  break;
	}

	/* Extract the item level */
	if (artifact_p(o_ptr))
		lev = a_info[o_ptr->name1].level;
	else
		lev = k_info[o_ptr->k_idx].level;

	/* Base chance of success */
	chance = p_ptr->skills[SKILL_DEVICE];

	/* Confusion hurts skill */
	if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_AMNESIA])
		chance = chance / 2;

	/* High level objects are harder */
	chance -= MIN(lev, 50);

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE))
	{
		if (flush_failure) flush();
		msg_format("You failed to %s properly.", msg);
		return FALSE;
	}

	/* Notice empty staffs */
	if (what && o_ptr->pval <= 0)
	{
		if (flush_failure) flush();
		msg_format("The %s has no charges left.", what);
		o_ptr->ident |= (IDENT_EMPTY);
		return FALSE;
	}

	return TRUE;
}

/*
 * Return the chance of an effect beaming, given a tval.
 */
static int beam_chance(int tval)
{
	switch (tval)
	{
		case TV_WAND: return 20;
		case TV_ROD:  return 10;
	}

	return 0;
}


/*
 * Use an object the right way.
 *
 * There may be a BIG problem with any "effect" that can cause "changes"
 * to the inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to "disappear", moving the inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" could cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * It seems as though a "rod of recharging" might in fact cause problems.
 * The basic problem is that the act of recharging (and destroying) an
 * item causes the inducer of that action to "move", causing "o_ptr" to
 * no longer point at the correct item, with horrifying results.
 */
static void do_cmd_use(const char *q, const char *s, int flag, int snd, use_type use)
{
	int item;
	object_type *o_ptr;
	int effect;
	bool ident = FALSE, used;
	int dir = 5;

	/* Get the item */
	if (!get_item(&item, q, s, flag))
		return;

	/* Get the item (in the pack/on the floor) */
	if (item >= 0) o_ptr = &inventory[item];
	else o_ptr = &o_list[0 - item];

	/* Figure out effect to use */
	if (o_ptr->name1)
		effect = a_info[o_ptr->name1].effect;
	else
		effect = k_info[o_ptr->k_idx].effect;

	/* If the item requires a direction, get one (allow cancelling) */
	if (effect_aim(effect) ||
	    (o_ptr->tval == TV_WAND) ||
	    (o_ptr->tval == TV_ROD && (o_ptr->sval >= SV_ROD_MIN_DIRECTION || !object_aware_p(o_ptr))))
	{
		/* Get a direction, allow cancel */
		if (!get_aim_dir(&dir))
			return;
	}

	/* Use energy regardless of failure */	
	p_ptr->energy_use = 100;

	/* Check for use */
	if (use == USE_CHARGE || use == USE_TIMEOUT)
	{
		if (!check_devices(o_ptr))
			return;
	}

	/* Special message for artifacts */
	if (artifact_p(o_ptr))
	{
		message(snd, 0, "You activate it.");
		msg_print(a_text + a_info[o_ptr->name1].effect_msg);
	}
	else
	{
		/* Make a noise! */
		sound(snd);
	}

	/* Do effect */
	used = do_effect(effect, &ident, dir, beam_chance(o_ptr->tval));

	/* Food feeds the player */
	if (o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION)
		(void)set_food(p_ptr->food + o_ptr->pval);

	if (!used && !ident) return;

	/* Mark as tried and redisplay */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	p_ptr->window |= (PW_INVEN | PW_EQUIP);


	/*
	 * If the player becomes aware of the item's function, then mark it as
	 * aware and reward the player with some experience.  Otherwise, mark
	 * it as "tried".
	 */
	if (ident && !object_aware_p(o_ptr))
	{
		/* Object level */
		int lev = k_info[o_ptr->k_idx].level;

		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}
	else
	{
		object_tried(o_ptr);
	}

	
	/* Some uses are "free" */
	if (!used) return;

	/* Chargeables act differently to single-used items when not used up */
	if (use == USE_CHARGE)
	{
		/* Use a single charge */
		o_ptr->pval--;

		/* Describe charges in the pack */
		if (item >= 0)
			inven_item_charges(item);

		/* Describe charges on the floor */
		else
			floor_item_charges(0 - item);
	}
	else if (use == USE_TIMEOUT)
	{
		/* Artifacts use their own special field */
		if (o_ptr->name1)
		{
			const artifact_type *a_ptr = &a_info[o_ptr->name1];
			o_ptr->timeout = a_ptr->time_base + damroll(a_ptr->time_dice, a_ptr->time_sides);
		}
		else
		{
			const object_kind *k_ptr = &k_info[o_ptr->k_idx];
			o_ptr->timeout += k_ptr->time_base + damroll(k_ptr->time_dice, k_ptr->time_sides);
		}
	}
	else if (use == USE_SINGLE)
	{
		/* Destroy a potion in the pack */
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_describe(item);
			inven_item_optimize(item);
		}

		/* Destroy a potion on the floor */
		else
		{
			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
	}
}


/*
 * Eat some food (from the pack or floor)
 */
void do_cmd_eat_food(void)
{
	/* Restrict choices to food */
	item_tester_tval = TV_FOOD;

	/* Use the object */
	do_cmd_use("Eat which item? ", "You have nothing to eat.", (USE_INVEN | USE_FLOOR), MSG_EAT, USE_SINGLE);
}


/*
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_quaff_potion(void)
{
	/* Restrict choices to potions */
	item_tester_tval = TV_POTION;

	/* Use the object */
	do_cmd_use("Quaff which potion? ", "You have no potions to quaff.", (USE_INVEN | USE_FLOOR), MSG_QUAFF, USE_SINGLE);
}


/*
 * Read a scroll (from the pack or floor).
 */
void do_cmd_read_scroll(void)
{
	/* Check some conditions */
	if (!can_read_scroll()) return;

	/* Restrict choices to scrolls */
	item_tester_tval = TV_SCROLL;

	/* Use the object */
	do_cmd_use("Read which scroll? ", "You have no scrolls to read.", (USE_INVEN | USE_FLOOR), MSG_GENERIC, USE_SINGLE);
}


/*
 * Use a staff
 */
void do_cmd_use_staff(void)
{
	/* Restrict choices to staves */
	item_tester_tval = TV_STAFF;

	/* Use the object */
	do_cmd_use("Use which staff? ", "You have no staff to use.", (USE_INVEN | USE_FLOOR), MSG_USE_STAFF, USE_CHARGE);
}


/*
 * Aim a wand
 */
void do_cmd_aim_wand(void)
{
	/* Restrict choices to wands */
	item_tester_tval = TV_WAND;

	/* Use the object */
	do_cmd_use("Aim which wand? ", "You have no wand to aim.", (USE_INVEN | USE_FLOOR), MSG_ZAP_ROD, USE_CHARGE);
}


/*
 * Zap a Rod
 */
void do_cmd_zap_rod(void)
{
	/* Restrict choices to rods */
	item_tester_hook = item_tester_hook_zap;

	/* Use the object */
	do_cmd_use("Zap which rod? ", "You have no charged rods to zap.", (USE_INVEN | USE_FLOOR), MSG_ZAP_ROD, USE_TIMEOUT);
}


/*
 * Activate a wielded object.
 */
void do_cmd_activate(void)
{
	/* Prepare the hook */
	item_tester_hook = item_tester_hook_activate;

	/* Use the object */
	do_cmd_use("Activate which item? ", "You have nothing to activate.", USE_EQUIP, MSG_ACT_ARTIFACT, USE_TIMEOUT);
}
