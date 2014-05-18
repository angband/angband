/**
   \file obj-gear.c
   \brief management of inventory, equipment and quiver
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2014 Nick McConnell
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
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-tval.h"
#include "obj-tvalsval.h"
#include "obj-util.h"
#include "player-util.h"
#include "spells.h"
#include "squelch.h"

static const struct slot_info {
	int index;
	bool acid_vuln;
	bool name_in_desc;
	const char *mention;
	const char *heavy_mention;
	const char *describe;
} slot_table[] = {
	#define EQUIP(a, b, c, d, e, f) { EQUIP_##a, b, c, d, e, f },
	#include "list-equip-slots.h"
	#undef EQUIP
	{ EQUIP_MAX, FALSE, FALSE, NULL, NULL, NULL }
};

/*
 * Convert an inventory index into a one character label.
 *
 * Note that the label does NOT distinguish inven/equip.
 */
char index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!player->inventory[i].kind) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Convert a label into the index of a item in the "equip".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= ALL_INVEN_TOTAL)) return (-1);
	if (i == INVEN_TOTAL) return (-1);

	/* Empty slots can never be chosen */
	if (!player->inventory[i].kind) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
bool wearable_p(const object_type *o_ptr)
{
	return tval_is_wearable(o_ptr);
}

static int get_inscribed_ammo_slot(const object_type *o_ptr)
{
	char *s;
	if (!o_ptr->note) return 0;
	s = strchr(quark_str(o_ptr->note), 'f');
	if (!s || s[1] < '0' || s[1] > '9') return 0;

	return QUIVER_START + (s[1] - '0');
}

/**
 * Used by wield_slot() to find an appopriate slot for ammo. See wield_slot()
 * for information on what this returns.
 */
static s16b wield_slot_ammo(const object_type *o_ptr)
{
	s16b i, open = 0;

	/* If the ammo is inscribed with a slot number, we'll try to put it in */
	/* that slot, if possible. */
	i = get_inscribed_ammo_slot(o_ptr);
	if (i && !player->inventory[i].kind) return i;

	for (i = QUIVER_START; i < QUIVER_END; i++)
	{
		if (!player->inventory[i].kind)
		{
			/* Save the open slot if we haven't found one already */
			if (!open) open = i;
			continue;
		}

		/* If ammo is cursed we can't stack it */
		if (cursed_p(player->inventory[i].flags)) continue;

		/* If they are stackable, we'll use this slot for sure */
		if (object_similar(&player->inventory[i], o_ptr,
			OSTACK_QUIVER)) return i;
	}

	/* If not absorbed, return an open slot (or QUIVER_START if no room) */
	return open ? open : QUIVER_START;
}

/**
 * Determine which equipment slot (if any) an item likes. The slot might (or
 * might not) be open, but it is a slot which the object could be equipped in.
 *
 * For items where multiple slots could work (e.g. ammo or rings), the function
 * will try to a return a stackable slot first (only for ammo), then an open
 * slot if possible, and finally a used (but valid) slot if necessary.
 */
s16b wield_slot(const object_type *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
		case TV_BOW: return (INVEN_BOW);
		case TV_AMULET: return (INVEN_NECK);
		case TV_CLOAK: return (INVEN_OUTER);
		case TV_SHIELD: return (INVEN_ARM);
		case TV_GLOVES: return (INVEN_HANDS);
		case TV_BOOTS: return (INVEN_FEET);
	}

	if (tval_is_melee_weapon(o_ptr))
		return INVEN_WIELD;
	else if (tval_is_ring(o_ptr))
		return player->inventory[INVEN_RIGHT].kind ? INVEN_LEFT : INVEN_RIGHT;
	else if (tval_is_light(o_ptr))
		return INVEN_LIGHT;
	else if (tval_is_body_armor(o_ptr))
		return INVEN_BODY;
	else if (tval_is_head_armor(o_ptr))
		return INVEN_HEAD;
	else if (tval_is_ammo(o_ptr))
		return wield_slot_ammo(o_ptr);

	/* No slot available */
	return (-1);
}


/*
 * Return a string mentioning how a given item is carried
 */
const char *mention_use(int slot)
{
	switch (slot)
	{
		case INVEN_WIELD:
		{
			if (adj_str_hold[player->state.stat_ind[A_STR]] < player->inventory[slot].weight / 10)
				return "Just lifting";
			else
				return "Wielding";
		}

		case INVEN_BOW:
		{
			if (adj_str_hold[player->state.stat_ind[A_STR]] < player->inventory[slot].weight / 10)
				return "Just holding";
			else
				return "Shooting";
		}

		case INVEN_LEFT:  return "On left hand";
		case INVEN_RIGHT: return "On right hand";
		case INVEN_NECK:  return "Around neck";
		case INVEN_LIGHT: return "Light source";
		case INVEN_BODY:  return "On body";
		case INVEN_OUTER: return "About body";
		case INVEN_ARM:   return "On arm";
		case INVEN_HEAD:  return "On head";
		case INVEN_HANDS: return "On hands";
		case INVEN_FEET:  return "On feet";

		case QUIVER_START + 0: return "In quiver [f0]";
		case QUIVER_START + 1: return "In quiver [f1]";
		case QUIVER_START + 2: return "In quiver [f2]";
		case QUIVER_START + 3: return "In quiver [f3]";
		case QUIVER_START + 4: return "In quiver [f4]";
		case QUIVER_START + 5: return "In quiver [f5]";
		case QUIVER_START + 6: return "In quiver [f6]";
		case QUIVER_START + 7: return "In quiver [f7]";
		case QUIVER_START + 8: return "In quiver [f8]";
		case QUIVER_START + 9: return "In quiver [f9]";
	}

	/*if (slot >= QUIVER_START && slot < QUIVER_END)
		return "In quiver";*/

	return "In pack";
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
const char *describe_use(int i)
{
	const char *p;

	switch (i)
	{
		case INVEN_WIELD: p = "attacking monsters with"; break;
		case INVEN_BOW:   p = "shooting missiles with"; break;
		case INVEN_LEFT:  p = "wearing on your left hand"; break;
		case INVEN_RIGHT: p = "wearing on your right hand"; break;
		case INVEN_NECK:  p = "wearing around your neck"; break;
		case INVEN_LIGHT: p = "using to light the way"; break;
		case INVEN_BODY:  p = "wearing on your body"; break;
		case INVEN_OUTER: p = "wearing on your back"; break;
		case INVEN_ARM:   p = "wearing on your arm"; break;
		case INVEN_HEAD:  p = "wearing on your head"; break;
		case INVEN_HANDS: p = "wearing on your hands"; break;
		case INVEN_FEET:  p = "wearing on your feet"; break;
		default:          p = "carrying in your pack"; break;
	}

	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &player->inventory[i];
		if (adj_str_hold[player->state.stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &player->inventory[i];
		if (adj_str_hold[player->state.stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just holding";
		}
	}

	/* Return the result */
	return p;
}


/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
int minus_ac(struct player *p)
{
	object_type *o_ptr = NULL;

	char o_name[80];

	/* Avoid crash during monster power calculations */
	if (!p->inventory) return FALSE;

	/* Pick a (possibly empty) inventory slot */
	switch (randint1(6))
	{
		case 1: o_ptr = &p->inventory[INVEN_BODY]; break;
		case 2: o_ptr = &p->inventory[INVEN_ARM]; break;
		case 3: o_ptr = &p->inventory[INVEN_OUTER]; break;
		case 4: o_ptr = &p->inventory[INVEN_HANDS]; break;
		case 5: o_ptr = &p->inventory[INVEN_HEAD]; break;
		case 6: o_ptr = &p->inventory[INVEN_FEET]; break;
		default: assert(0);
	}

	/* Nothing to damage */
	if (!o_ptr->kind) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Object resists */
	if (o_ptr->el_info[ELEM_ACID].flags & EL_INFO_IGNORE)
	{
		msg("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Message */
	msg("Your %s is damaged!", o_name);

	/* Damage the item */
	o_ptr->to_a--;

	p->upkeep->update |= PU_BONUS;
	p->upkeep->redraw |= (PR_EQUIP);

	/* Item was damaged */
	return (TRUE);
}

/*
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(int item)
{
	object_type *o_ptr = &player->inventory[item];

	/* Require staff/wand */
	if (!tval_can_have_charges(o_ptr)) return;

	/* Require known item */
	if (!object_is_known(o_ptr)) return;

	/* Print a message */
	msg("You have %d charge%s remaining.", o_ptr->pval,
	    (o_ptr->pval != 1) ? "s" : "");
}


/*
 * Describe an item in the inventory. Note: only called when an item is 
 * dropped, used, or otherwise deleted from the inventory
 */
void inven_item_describe(int item)
{
	object_type *o_ptr = &player->inventory[item];

	char o_name[80];

	if (o_ptr->artifact && 
		(object_is_known(o_ptr) || object_name_is_visible(o_ptr)))
	{
		/* Get a description */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL | ODESC_SINGULAR);

		/* Print a message */
		msg("You no longer have the %s (%c).", o_name, index_to_label(item));
	}
	else
	{
		/* Get a description */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Print a message */
		msg("You have %s (%c).", o_name, index_to_label(item));
	}
}


/*
 * Increase the "number" of an item in the inventory
 */
void inven_item_increase(int item, int num)
{
	object_type *o_ptr = &player->inventory[item];

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number and weight */
	if (num)
	{
		/* Add the number */
		o_ptr->number += num;

		/* Add the weight */
		player->upkeep->total_weight += (num * o_ptr->weight);

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana XXX */
		player->upkeep->update |= (PU_MANA);

		/* Combine the pack */
		player->upkeep->notice |= (PN_COMBINE);

		/* Redraw stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}
}


/**
 * Save the size of the quiver.
 */
void save_quiver_size(struct player *p)
{
	int i, count = 0;
	int maxsize = MAX_STACK_SIZE - 1;

	for (i = QUIVER_START; i < QUIVER_END; i++)
		if (p->inventory[i].kind)
			count += p->inventory[i].number;

	p->upkeep->quiver_size = count;
	p->upkeep->quiver_slots = (count + maxsize - 1) / maxsize;
	p->upkeep->quiver_remainder = count % maxsize;
}


/**
 * Compare ammunition from slots (0-9); used for sorting.
 *
 * \returns -1 if slot1 should come first, 1 if slot2 should come first, or 0.
 */
static int compare_ammo(int slot1, int slot2)
{
	/* Right now there is no sorting criteria */
	return 0;
}

/**
 * Swap ammunition between quiver slots (0-9).
 */
static void swap_quiver_slots(int slot1, int slot2)
{
	int i = slot1 + QUIVER_START;
	int j = slot2 + QUIVER_START;
	object_type o;

	object_copy(&o, &player->inventory[i]);
	object_copy(&player->inventory[i], &player->inventory[j]);
	object_copy(&player->inventory[j], &o);

	/* Update object_idx if necessary */
	if (tracked_object_is(player->upkeep, i))
	{
		track_object(player->upkeep, j);
	}

	if (tracked_object_is(player->upkeep, j))
	{
		track_object(player->upkeep, i);
	}
}

/**
 * Sorts the quiver--ammunition inscribed with @fN prefers to end up in quiver
 * slot N.
 */
void sort_quiver(void)
{
	/* Ammo slots go from 0-9; these indices correspond to the range of
	 * (QUIVER_START) - (QUIVER_END-1) in inventory[].
	 */
	bool locked[QUIVER_SIZE] = {FALSE, FALSE, FALSE, FALSE, FALSE,
								FALSE, FALSE, FALSE, FALSE, FALSE};
	int desired[QUIVER_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int i, j, k;
	object_type *o_ptr;

	/* Here we figure out which slots have inscribed ammo, and whether that
	 * ammo is already in the slot it "wants" to be in or not.
	 */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		j = QUIVER_START + i;
		o_ptr = &player->inventory[j];

		/* Skip this slot if it doesn't have ammo */
		if (!o_ptr->kind) continue;

		/* Figure out which slot this ammo prefers, if any */
		k = get_inscribed_ammo_slot(o_ptr);
		if (!k) continue;

		k -= QUIVER_START;
		if (k == i) locked[i] = TRUE;
		if (desired[k] < 0) desired[k] = i;
	}

	/* For items which had a preference that was not fulfilled, we will swap
	 * them into the slot as long as it isn't already locked.
	 */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		if (locked[i] || desired[i] < 0) continue;

		/* item in slot 'desired[i]' desires to be in slot 'i' */
		swap_quiver_slots(desired[i], i);
		locked[i] = TRUE;
	}

	/* Now we need to compact ammo which isn't in a preferrred slot towards the
	 * "front" of the quiver */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		/* If the slot isn't empty, skip it */
		if (player->inventory[QUIVER_START + i].kind) continue;

		/* Start from the end and find an unlocked item to put here. */
		for (j=QUIVER_SIZE - 1; j > i; j--)
		{
			if (!player->inventory[QUIVER_START + j].kind || locked[j]) continue;
			swap_quiver_slots(i, j);
			break;
		}
	}

	/* Now we will sort all other ammo using a simple insertion sort */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		k = i;
		if (!locked[k])
			for (j = i + 1; j < QUIVER_SIZE; j++)
				if (!locked[j] && compare_ammo(k, j) > 0)
					swap_quiver_slots(j, k);
	}
}

/*
 * Shifts ammo at or above the item slot towards the end of the quiver, making
 * room for a new piece of ammo.
 */
void open_quiver_slot(int slot)
{
	int i, pref;
	int dest = QUIVER_END - 1;

	/* This should only be used on ammunition */
	if (slot < QUIVER_START) return;

	/* Quiver is full */
	if (player->inventory[QUIVER_END - 1].kind) return;

	/* Find the first open quiver slot */
	while (player->inventory[dest].kind) dest++;

	/* Swap things with the space one higher (essentially moving the open space
	 * towards our goal slot. */
	for (i = dest - 1; i >= slot; i--)
	{
		/* If we have an item with an inscribed location (and it's in */
		/* that location) then we won't move it. */
		pref = get_inscribed_ammo_slot(&player->inventory[i]);
		if (i != slot && pref && pref == i) continue;

		/* Update object_idx if necessary */
		if (tracked_object_is(player->upkeep, i))
		{
			track_object(player->upkeep, dest);
		}

		/* Copy the item up and wipe the old slot */
		COPY(&player->inventory[dest], &player->inventory[i], object_type);
		dest = i;
		object_wipe(&player->inventory[dest]);


	}
}


/*
 * Erase an inventory slot if it has no more items
 */
void inven_item_optimize(int item)
{
	object_type *o_ptr = &player->inventory[item];
	int i, j, slot, limit;

	/* Save a possibly new quiver size */
	if (item >= QUIVER_START) save_quiver_size(player);

	/* Only optimize real items which are empty */
	if (!o_ptr->kind || o_ptr->number) return;

	/* Stop tracking erased item if necessary */
	if (tracked_object_is(player->upkeep, item))
	{
		track_object(player->upkeep, NO_OBJECT);
	}

	/* Items in the pack are treated differently from other items */
	if (item < INVEN_WIELD)
	{
		player->upkeep->inven_cnt--;
		player->upkeep->redraw |= PR_INVEN;
		limit = INVEN_MAX_PACK;
	}

	/* Items in the quiver and equipped items are (mostly) treated similarly */
	else
	{
		player->upkeep->equip_cnt--;
		player->upkeep->redraw |= PR_EQUIP;
		limit = item >= QUIVER_START ? QUIVER_END : 0;
	}

	/* If the item is equipped (but not in the quiver), there is no need to */
	/* slide other items. Bonuses and such will need to be recalculated */
	if (!limit)
	{
		/* Erase the empty slot */
		object_wipe(&player->inventory[item]);
		
		/* Recalculate stuff */
		player->upkeep->update |= (PU_BONUS);
		player->upkeep->update |= (PU_TORCH);
		player->upkeep->update |= (PU_MANA);
		
		return;
	}

	/* Slide everything down */
	for (j = item, i = item + 1; i < limit; i++)
	{
		if (limit == QUIVER_END && player->inventory[i].kind)
		{
			/* If we have an item with an inscribed location (and it's in */
			/* that location) then we won't move it. */
			slot = get_inscribed_ammo_slot(&player->inventory[i]);
			if (slot && slot == i)
				continue;
		}
		COPY(&player->inventory[j], &player->inventory[i], object_type);

		/* Update object_idx if necessary */
		if (tracked_object_is(player->upkeep, i))
		{
			track_object(player->upkeep, j);
		}

		j = i;
	}

	/* Reorder the quiver if necessary */
	if (item >= QUIVER_START) sort_quiver();

	/* Wipe the left-over object on the end */
	object_wipe(&player->inventory[j]);

	/* Inventory has changed, so disable repeat command */ 
	cmd_disable_repeat();
}


/*
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(const object_type *o_ptr)
{
	/* Empty slot? */
	if (player->upkeep->inven_cnt < INVEN_MAX_PACK) return TRUE;

	/* Check if it can stack */
	if (inven_stack_okay(o_ptr)) return TRUE;

	/* Nope */
	return FALSE;
}

/*
 * Check to see if an item is stackable in the inventory
 */
bool inven_stack_okay(const object_type *o_ptr)
{
	/* Similar slot? */
	int j;

	/* If our pack is full and we're adding too many missiles, there won't be
	 * enough room in the quiver, so don't check it. */
	int limit;

	if (!pack_is_full())
		/* The pack has more room */
		limit = ALL_INVEN_TOTAL;
	else if (player->upkeep->quiver_remainder == 0)
		/* Quiver already maxed out */
		limit = INVEN_PACK;
	else if (player->upkeep->quiver_remainder + o_ptr->number >= MAX_STACK_SIZE)
		/* Too much new ammo */
		limit = INVEN_PACK;
	else
		limit = ALL_INVEN_TOTAL;

	for (j = 0; j < limit; j++)
	{
		object_type *j_ptr = &player->inventory[j];

		/* Skip equipped items and non-objects */
		if (j >= INVEN_PACK && j < QUIVER_START) continue;
		if (!j_ptr->kind) continue;

		/* Check if the two items can be combined */
		if (object_similar(j_ptr, o_ptr, OSTACK_PACK)) return (TRUE);
	}
	return (FALSE);
}

/**
 * Return the preferred inventory slot for the given object.
 *
 * This function defines the sort order for the inventory.
 *
 * \param o_ptr is the object that needs a slot.
 * \param max_slot is the maximum slot we will allow for this object.
 * \return the inventory slot index for the object.
 */
static int inventory_slot_for_object(const struct object *o_ptr, size_t max_slot)
{
	/* Get the "value" of the item */
	s32b o_value = o_ptr->kind->cost;
	s32b j_value;
	struct object *j_ptr;
	size_t j;

	/* Scan every occupied slot */
	for (j = 0; j < max_slot; j++)
	{
		/* Get the item already there */
		j_ptr = &player->inventory[j];

		/* Use empty slots */
		if (!j_ptr->kind) break;
		
		/* Hack -- readable books always come first */
		if ((o_ptr->tval == player->class->spell_book) &&
			(j_ptr->tval != player->class->spell_book)) break;
		if ((j_ptr->tval == player->class->spell_book) &&
			(o_ptr->tval != player->class->spell_book)) continue;
		
		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;
		
		/* Non-aware (flavored) items always come last */
		if (!object_flavor_is_aware(o_ptr)) continue;
		if (!object_flavor_is_aware(j_ptr)) break;
		
		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;
		
		/* Unidentified objects always come last */
		if (!object_is_known(o_ptr)) continue;
		if (!object_is_known(j_ptr)) break;
		
		/* Lights sort by decreasing fuel */
		if (tval_is_light(o_ptr))
		{
			if (o_ptr->pval > j_ptr->pval) break;
			if (o_ptr->pval < j_ptr->pval) continue;
		}
		
		/* Determine the "value" of the pack item */
		j_value = j_ptr->kind->cost;
		
		/* Objects sort by decreasing value */
		if (o_value > j_value) break;
		if (o_value < j_value) continue;
	}

	return j;
}

/*
 * Add an item to the players inventory, and return the slot used.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so, using "object_similar()" and "object_absorb()", else,
 * the item will be placed into the "proper" location in the inventory.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined.  This may be tricky.  See "dungeon.c" for info.
 *
 * Note that this code must remove any location/stack information
 * from the object once it is placed into the inventory.
 */
extern s16b inven_carry(struct player *p, struct object *o)
{
	int i, j, k;
	int n = -1;

	object_type *j_ptr;

	/* Apply an autoinscription */
	apply_autoinscription(o);

	/* Check for combining */
	for (j = 0; j < INVEN_PACK; j++)
	{
		j_ptr = &p->inventory[j];
		if (!j_ptr->kind) continue;

		/* Hack -- track last item */
		n = j;

		/* Check if the two items can be combined */
		if (object_similar(j_ptr, o, OSTACK_PACK))
		{
			/* Combine the items */
			object_absorb(j_ptr, o);

			/* Increase the weight */
			p->upkeep->total_weight += (o->number * o->weight);

			/* Recalculate bonuses */
			p->upkeep->update |= (PU_BONUS);

			/* Redraw stuff */
			p->upkeep->redraw |= (PR_INVEN);

			/* Save quiver size */
			save_quiver_size(p);

			/* Success */
			return (j);
		}
	}


	/* Paranoia */
	if (p->upkeep->inven_cnt > INVEN_MAX_PACK) return (-1);


	/* Find an empty slot */
	for (j = 0; j <= INVEN_MAX_PACK; j++)
	{
		j_ptr = &p->inventory[j];
		if (!j_ptr->kind) break;
	}

	/* Use that slot */
	i = j;

	/* Reorder the pack */
	if (i < INVEN_MAX_PACK)
	{
		j = inventory_slot_for_object(o, INVEN_MAX_PACK);

		/* Use that slot */
		i = j;

		/* Slide objects */
		for (k = n; k >= i; k--)
		{
			/* Hack -- Slide the item */
			object_copy(&p->inventory[k+1], &p->inventory[k]);

			/* Update object_idx if necessary */
			if (tracked_object_is(player->upkeep, k))
			{
				track_object(player->upkeep, k+1);
			}
		}

		/* Wipe the empty slot */
		object_wipe(&p->inventory[i]);
	}

	object_copy(&p->inventory[i], o);

	j_ptr = &p->inventory[i];
	j_ptr->next_o_idx = 0;
	j_ptr->held_m_idx = 0;
	j_ptr->iy = j_ptr->ix = 0;
	j_ptr->marked = FALSE;

	p->upkeep->total_weight += (j_ptr->number * j_ptr->weight);
	p->upkeep->inven_cnt++;
	p->upkeep->update |= (PU_BONUS);
	p->upkeep->notice |= (PN_COMBINE | PN_REORDER);
	p->upkeep->redraw |= (PR_INVEN);

	/* Hobbits ID mushrooms on pickup, gnomes ID wands and staffs on pickup */
	if (!object_is_known(j_ptr))
	{
		if (player_has(PF_KNOW_MUSHROOM) && tval_is_mushroom(j_ptr))
		{
			do_ident_item(j_ptr);
			msg("Mushrooms for breakfast!");
		}
		else if (player_has(PF_KNOW_ZAPPER) && tval_is_zapper(j_ptr))
		{
			do_ident_item(j_ptr);
		}
	}

	/* Save quiver size */
	save_quiver_size(p);

	/* Return the slot */
	return (i);
}


/*
 * Take off (some of) a non-cursed equipment item
 *
 * Note that only one item at a time can be wielded per slot.
 *
 * Note that taking off an item when "full" may cause that item
 * to fall to the ground.
 *
 * Return the inventory slot into which the item is placed.
 */
s16b inven_takeoff(int item, int amt)
{
	int slot;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	const char *act;

	char o_name[80];

	bool track_removed_item = FALSE;


	/* Get the item to take off */
	o_ptr = &player->inventory[item];

	/* Paranoia */
	if (amt <= 0) return (-1);

	/* Verify */
	if (amt > o_ptr->number) amt = o_ptr->number;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain a local object */
	object_copy(i_ptr, o_ptr);

	/* Modify quantity */
	i_ptr->number = amt;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Took off weapon */
	if (item == INVEN_WIELD)
	{
		act = "You were wielding";
	}

	/* Took off bow */
	else if (item == INVEN_BOW)
	{
		act = "You were holding";
	}

	/* Took off light */
	else if (item == INVEN_LIGHT)
	{
		act = "You were holding";
	}

	/* Took off something */
	else
	{
		act = "You were wearing";
	}

	/* Update object_idx if necessary, after optimization */
	if (tracked_object_is(player->upkeep, item))
	{
		track_removed_item = TRUE;
	}

	/* Modify, Optimize */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);

	/* Carry the object */
	slot = inven_carry(player, i_ptr);

	/* Track removed item if necessary */
	if (track_removed_item)
	{
		track_object(player->upkeep, slot);
	}

	/* Message */
	msgt(MSG_WIELD, "%s %s (%c).", act, o_name, index_to_label(slot));

	player->upkeep->notice |= PN_SQUELCH;

	/* Return slot */
	return (slot);
}


/*
 * Drop (some of) a non-cursed inventory/equipment item
 *
 * The object will be dropped "near" the current location
 */
void inven_drop(int item, int amt)
{
	int py = player->py;
	int px = player->px;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	char o_name[80];


	/* Get the original object */
	o_ptr = &player->inventory[item];

	/* Error check */
	if (amt <= 0) return;

	/* Not too many */
	if (amt > o_ptr->number) amt = o_ptr->number;


	/* Take off equipment */
	if (item >= INVEN_WIELD)
	{
		/* Take off first */
		item = inven_takeoff(item, amt);

		/* Get the original object */
		o_ptr = &player->inventory[item];
	}

	/* Stop tracking items no longer in the inventory */
	if (tracked_object_is(player->upkeep, item) && amt == o_ptr->number)
	{
		track_object(player->upkeep, NO_OBJECT);
	}

	i_ptr = &object_type_body;

	object_copy(i_ptr, o_ptr);
	object_split(i_ptr, o_ptr, amt);

	/* Describe local object */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", o_name, index_to_label(item));

	/* Drop it near the player */
	drop_near(cave, i_ptr, 0, py, px, FALSE);

	/* Modify, Describe, Optimize */
	inven_item_increase(item, -amt);
	inven_item_describe(item);
	inven_item_optimize(item);
}



/**
 * Return whether each stack of objects can be merged into two uneven stacks.
 */
static bool inventory_can_stack_partial(const object_type *o_ptr,
										const object_type *j_ptr,
										object_stack_t mode)
{
	if (!(mode & OSTACK_STORE)) {
		int total = o_ptr->number + j_ptr->number;
		int remainder = total - (MAX_STACK_SIZE - 1);

		if (remainder >= MAX_STACK_SIZE)
			return FALSE;
	}

	return object_stackable(o_ptr, j_ptr, mode);
}

/*
 * Combine items in the pack
 * Also "pick up" any gold in the inventory by accident
 *
 * Note special handling of the "overflow" slot
 */
void combine_pack(void)
{
	int i, j, k;
	object_type *o_ptr;
	object_type *j_ptr;
	bool display_message = FALSE;
	bool redraw = FALSE;

	/* Combine the pack (backwards) */
	for (i = INVEN_PACK; i > 0; i--)
	{
		bool slide = FALSE;

		/* Get the item */
		o_ptr = &player->inventory[i];

		/* Skip empty items */
		if (!o_ptr->kind) continue;

		/* Absorb gold */
		if (tval_is_money(o_ptr))
		{
			/* Count the gold */
			slide = TRUE;
			player->au += o_ptr->pval;
		}

		/* Scan the items above that item */
		else for (j = 0; j < i; j++)
		{
			/* Get the item */
			j_ptr = &player->inventory[j];

			/* Skip empty items */
			if (!j_ptr->kind) continue;

			/* Can we drop "o_ptr" onto "j_ptr"? */
			if (object_similar(j_ptr, o_ptr, OSTACK_PACK)) {
				display_message = TRUE;
				slide = TRUE;
				redraw = TRUE;
				object_absorb(j_ptr, o_ptr);
				break;
			}
			else if (inventory_can_stack_partial(j_ptr, o_ptr, OSTACK_PACK)) {
				display_message = FALSE; /* Setting this to TRUE spams the combine message. */
				slide = FALSE;
				redraw = TRUE;
				object_absorb_partial(j_ptr, o_ptr);
				break;
			}
		}

		/* Compact the inventory */
		if (slide)
		{
			/* One object is gone */
			player->upkeep->inven_cnt--;

			/* Slide everything down */
			for (k = i; k < INVEN_PACK; k++)
			{
				/* Hack -- slide object */
				COPY(&player->inventory[k], &player->inventory[k+1], object_type);

				/* Update object_idx if necessary */
				if (tracked_object_is(player->upkeep, k+1))
				{
					track_object(player->upkeep, k);
				}
			}

			/* Hack -- wipe hole */
			object_wipe(&player->inventory[k]);

			redraw = TRUE;
		}
	}

	/* Redraw stuff */
	if (redraw)
		player->upkeep->redraw |= (PR_INVEN);

	/* Message */
	if (display_message)
	{
		msg("You combine some items in your pack.");

		/* Stop "repeat last command" from working. */
		cmd_disable_repeat();
	}
}

/*
 * Reorder items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void reorder_pack(void)
{
	int i, j, k;
	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;
	bool flag = FALSE;

	/* Re-order the pack (forwards) */
	for (i = 0; i < INVEN_PACK; i++)
	{
		/* Get the item */
		o_ptr = &player->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->kind) continue;

		j = inventory_slot_for_object(o_ptr, INVEN_PACK);

		/* Never move down */
		if (j >= i) continue;

		/* Take note */
		flag = TRUE;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Save a copy of the moving item */
		object_copy(i_ptr, &player->inventory[i]);

		/* Slide the objects */
		for (k = i; k > j; k--)
		{
			/* Slide the item */
			object_copy(&player->inventory[k], &player->inventory[k - 1]);

			/* Update object_idx if necessary */
			if (tracked_object_is(player->upkeep, k - 1))
			{
				track_object(player->upkeep, k);
			}
		}

		/* Insert the moving item */
		object_copy(&player->inventory[j], i_ptr);

		/* Update object_idx if necessary */
		if (tracked_object_is(player->upkeep, i))
		{
			track_object(player->upkeep, j);
		}

		/* Redraw stuff */
		player->upkeep->redraw |= (PR_INVEN);
	}

	if (flag) 
	{
		msg("You reorder some items in your pack.");

		/* Stop "repeat last command" from working. */
		cmd_disable_repeat();
	}
}


/*
 *Returns the number of times in 1000 that @ will FAIL
 * - thanks to Ed Graham for the formula
 */
int get_use_device_chance(const object_type *o_ptr)
{
	int lev, fail, numerator, denominator;

	int skill = player->state.skills[SKILL_DEVICE];

	int skill_min = 10;
	int skill_max = 141;
	int diff_min  = 1;
	int diff_max  = 100;

	/* Extract the item level, which is the difficulty rating */
	if (o_ptr->artifact)
		lev = o_ptr->artifact->level;
	else
		lev = o_ptr->kind->level;

	/* TODO: maybe use something a little less convoluted? */
	numerator   = (skill - lev) - (skill_max - diff_min);
	denominator = (lev - skill) - (diff_max - skill_min);

	/* Make sure that we don't divide by zero */
	if (denominator == 0) denominator = numerator > 0 ? 1 : -1;

	fail = (100 * numerator) / denominator;

	/* Ensure failure rate is between 1% and 75% */
	if (fail > 750) fail = 750;
	if (fail < 10) fail = 10;

	return fail;
}


/*
 * Distribute charges of rods, staves, or wands.
 *
 * o_ptr = source item
 * q_ptr = target item, must be of the same type as o_ptr
 * amt   = number of items that are transfered
 */
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt)
{
	int charge_time = randcalc(o_ptr->time, 0, AVERAGE), max_time;

	/*
	 * Hack -- If rods, staves, or wands are dropped, the total maximum
	 * timeout or charges need to be allocated between the two stacks.
	 * If all the items are being dropped, it makes for a neater message
	 * to leave the original stack's pval alone. -LM-
	 */
	if (tval_can_have_charges(o_ptr))
	{
		q_ptr->pval = o_ptr->pval * amt / o_ptr->number;

		if (amt < o_ptr->number)
			o_ptr->pval -= q_ptr->pval;
	}

	/*
	 * Hack -- Rods also need to have their timeouts distributed.
	 *
	 * The dropped stack will accept all time remaining to charge up to
	 * its maximum.
	 */
	if (tval_can_have_timeout(o_ptr))
	{
		max_time = charge_time * amt;

		if (o_ptr->timeout > max_time)
			q_ptr->timeout = max_time;
		else
			q_ptr->timeout = o_ptr->timeout;

		if (amt < o_ptr->number)
			o_ptr->timeout -= q_ptr->timeout;
	}
}


void reduce_charges(object_type *o_ptr, int amt)
{
	/*
	 * Hack -- If rods or wand are destroyed, the total maximum timeout or
	 * charges of the stack needs to be reduced, unless all the items are
	 * being destroyed. -LM-
	 */
	if (tval_can_have_charges(o_ptr) && amt < o_ptr->number)
		o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;

	if (tval_can_have_timeout(o_ptr) && amt < o_ptr->number)
		o_ptr->timeout -= o_ptr->timeout * amt / o_ptr->number;
}


int number_charging(const object_type *o_ptr)
{
	int charge_time, num_charging;

	charge_time = randcalc(o_ptr->time, 0, AVERAGE);

	/* Item has no timeout */
	if (charge_time <= 0) return 0;

	/* No items are charging */
	if (o_ptr->timeout <= 0) return 0;

	/* Calculate number charging based on timeout */
	num_charging = (o_ptr->timeout + charge_time - 1) / charge_time;

	/* Number charging cannot exceed stack size */
	if (num_charging > o_ptr->number) num_charging = o_ptr->number;

	return num_charging;
}


bool recharge_timeout(object_type *o_ptr)
{
	int charging_before, charging_after;

	/* Find the number of charging items */
	charging_before = number_charging(o_ptr);

	/* Nothing to charge */	
	if (charging_before == 0)
		return FALSE;

	/* Decrease the timeout */
	o_ptr->timeout -= MIN(charging_before, o_ptr->timeout);

	/* Find the new number of charging items */
	charging_after = number_charging(o_ptr);

	/* Return true if at least 1 item obtained a charge */
	if (charging_after < charging_before)
		return TRUE;
	else
		return FALSE;
}

/*
 * Returns whether the pack is holding the maximum number of items. The max
 * size is INVEN_MAX_PACK, which is a macro since quiver size affects slots
 * available.
 */
bool pack_is_full(void)
{
	return player->inventory[INVEN_MAX_PACK - 1].kind ? TRUE : FALSE;
}

/*
 * Returns whether the pack is holding the more than the maximum number of
 * items. The max size is INVEN_MAX_PACK, which is a macro since quiver size
 * affects slots available. If this is true, calling pack_overflow() will
 * trigger a pack overflow.
 */
bool pack_is_overfull(void)
{
	return player->inventory[INVEN_MAX_PACK].kind ? TRUE : FALSE;
}

/*
 * Overflow an item from the pack, if it is overfull.
 */
void pack_overflow(void)
{
	int item = INVEN_MAX_PACK;
	char o_name[80];
	object_type *o_ptr;

	if (!pack_is_overfull()) return;

	/* Get the slot to be dropped */
	o_ptr = &player->inventory[item];

	/* Disturbing */
	disturb(player, 0);

	/* Warning */
	msg("Your pack overflows!");

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", o_name, index_to_label(item));

	/* Drop it (carefully) near the player */
	drop_near(cave, o_ptr, 0, player->py, player->px, FALSE);

	/* Modify, Describe, Optimize */
	inven_item_increase(item, -255);
	inven_item_describe(item);
	inven_item_optimize(item);

	/* Notice stuff (if needed) */
	if (player->upkeep->notice) notice_stuff(player->upkeep);

	/* Update stuff (if needed) */
	if (player->upkeep->update) update_stuff(player->upkeep);

	/* Redraw stuff (if needed) */
	if (player->upkeep->redraw) redraw_stuff(player->upkeep);
}

void embody_player(struct player *p, int body)
{
	int i;

	/* Copy the body */
	p->body = bodies[body];

	/* Set it to be carrying nothing */
	for (i = 0; i < p->body.count; i++)
		p->body.slots[i].index = MAX_GEAR;
}

int slot_by_name(const char *name)
{
	int i;

	/* Look for the correctly named slot */
	for (i = 0; i < player->body.count; i++)
		if (streq(name, player->body.slots[i].name)) break;

	/* Index for that slot */
	return i;
}

int slot_by_type(int type)
{
	int i;

	/* Look for a correct slot type */
	for (i = 0; i < player->body.count; i++)
		if (type == player->body.slots[i].type) break;

	/* Index for that slot */
	return i;
}

struct object *equipped_item_by_slot(int slot)
{
	int gear_index;

	/* Check for valid slot */
	if (slot < 0 || slot >= player->body.count) return NULL;

	/* Get the index into the gear array */
	gear_index = player->body.slots[slot].index;

	/* Index is set to MAX_GEAR (NO_OBJECT - NRM) if no object in that slot */
	return (gear_index < MAX_GEAR) ? &player->gear[gear_index] : NULL;
}

struct object *equipped_item_by_slot_name(const char *name)
{
	return equipped_item_by_slot(slot_by_name(name));
}

struct object *equipped_item_by_slot_type(int type)
{
	return equipped_item_by_slot(slot_by_type(type));
}

int equipped_item_slot(int item)
{
	int i;

	/* Look for an equipment slot with this item */
	for (i = 0; i < player->body.count; i++)
		if (item == player->body.slots[i].index) break;

	/* Correct slot, or player->body.count if not equipped */
	return i;
}

bool item_is_equipped(int item)
{
	return (equipped_item_slot(item) < player->body.count) ? TRUE : FALSE;
}

int pack_slots_used(struct player *p)
{
	int i, quiver_slots = 0, pack_slots = 0, quiver_ammo = 0;
	int maxsize = MAX_STACK_SIZE - 1;

	for (i = 0; i < MAX_GEAR; i++) {
		struct object *obj = &p->gear[i];

		/* No actual object */
		if (!obj->kind) continue;

		/* Equipment doesn't count */
		if (item_is_equipped(i)) continue;

		/* Check if it could be in the quiver */
		if (tval_is_ammo(obj))
			if (quiver_slots < QUIVER_SIZE) {
				quiver_slots++;
				quiver_ammo += obj->number;
				continue;
			}

		/* Count regular slots */
		pack_slots++;
	}

	/* Full slots */
	pack_slots += quiver_ammo / maxsize;

	/* Plus one for any remainder */
	if (quiver_ammo % maxsize) pack_slots++;

	return pack_slots;
}

/*
 * Return a string mentioning how a given item is carried
 *
 * Need to deal with heavy weapon/bow - NRM
 */
const char *equip_mention(int slot)
{
	int type;

	//if (!item_is_equipped(item)) return "In pack";

	type = player->body.slots[slot].type;

	if (slot_table[type].name_in_desc)
		return format(slot_table[type].mention, player->body.slots[slot].name);
	else
		return slot_table[type].mention;
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 *
 * Need to deal with heavy weapon/bow - NRM
 */
const char *equip_describe(int slot)
{
	int type;

	//if (!item_is_equipped(item)) return NULL;

	type = player->body.slots[slot].type;

	if (slot_table[type].name_in_desc)
		return format(slot_table[type].describe, player->body.slots[slot].name);
	else
		return slot_table[type].describe;
}


/**
 *
 */
struct object *inven_item(int item)
{
	return &player->gear[player->upkeep->inven[item]];
}

/**
 *
 */
struct object *equip_item(int item)
{
	return equipped_item_by_slot(item - INVEN_WIELD);
}

/**
 *
 */
struct object *quiver_item(int item)
{
	return &player->gear[player->upkeep->quiver[item - QUIVER_START]];
}
