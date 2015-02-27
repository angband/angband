/**
 * \file obj-gear.c
 * \brief management of inventory, equipment and quiver
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
#include "cmd-core.h"
#include "game-event.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-util.h"

static const struct slot_info {
	int index;
	bool acid_vuln;
	bool name_in_desc;
	const char *mention;
	const char *heavy_describe;
	const char *describe;
} slot_table[] = {
	#define EQUIP(a, b, c, d, e, f) { EQUIP_##a, b, c, d, e, f },
	#include "list-equip-slots.h"
	#undef EQUIP
	{ EQUIP_MAX, FALSE, FALSE, NULL, NULL, NULL }
};

int slot_by_name(struct player *p, const char *name)
{
	int i;

	/* Look for the correctly named slot */
	for (i = 0; i < p->body.count; i++)
		if (streq(name, p->body.slots[i].name)) break;

	/* Index for that slot */
	return i;
}

/**
 * Gets a slot of the given type, preferentially empty unless full is true
 */
int slot_by_type(struct player *p, int type, bool full)
{
	int i, fallback = p->body.count;

	/* Look for a correct slot type */
	for (i = 0; i < p->body.count; i++) {
		if (type == p->body.slots[i].type) {
			if (full) {
				/* Found a full slot */
				if (p->body.slots[i].obj != NULL) break;
			} else {
				/* Found an empty slot */
				if (p->body.slots[i].obj == NULL) break;
			}
			/* Not right for full/empty, but still the right type */
			if (fallback == p->body.count)
				fallback = i;
		}
	}

	/* Index for the best slot we found, or p->body.count if none found  */
	return (i != p->body.count) ? i : fallback;
}

bool slot_type_is(int slot, int type)
{
	return player->body.slots[slot].type == type ? TRUE : FALSE;
}

struct object *slot_object(struct player *p, int slot)
{
	/* Ensure a valid body */
	if (!p->body.slots)
		return NULL;

	/* Returns NULL if no object in that slot */
	return p->body.slots[slot].obj;
}

struct object *equipped_item_by_slot_name(struct player *p, const char *name)
{
	/* Ensure a valid body */
	if (!p->body.slots)
		return NULL;

	return slot_object(p, slot_by_name(p, name));
}

bool object_is_equipped(struct player_body body, const struct object *obj)
{
	int i;

	for (i = 0; i < body.count; i++)
		if (obj == body.slots[i].obj) break;

	return (i < body.count) ? TRUE : FALSE;
}

bool object_is_carried(struct player *p, const struct object *obj)
{
	return pile_contains(p->gear, obj);
}

int pack_slots_used(struct player *p)
{
	struct object *obj;
	int quiver_slots = 0, pack_slots = 0, quiver_ammo = 0;
	int maxsize = z_info->stack_size;

	for (obj = p->gear; obj; obj = obj->next) {
		/* Equipment doesn't count */
		if (object_is_equipped(p->body, obj)) continue;

		/* Check if it could be in the quiver */
		if (tval_is_ammo(obj))
			if (quiver_slots < z_info->quiver_size) {
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
 */
const char *equip_mention(struct player *p, int slot)
{
	int type;

	type = p->body.slots[slot].type;

	/* Heavy */
	if ((type == EQUIP_WEAPON && p->state.heavy_wield) ||
		(type == EQUIP_WEAPON && p->state.heavy_shoot))
		return slot_table[type].heavy_describe;

	if (slot_table[type].name_in_desc)
		return format(slot_table[type].mention, p->body.slots[slot].name);
	else
		return slot_table[type].mention;
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
const char *equip_describe(struct player *p, int slot)
{
	int type;

	type = p->body.slots[slot].type;

	/* Heavy */
	if ((type == EQUIP_WEAPON && p->state.heavy_wield) ||
		(type == EQUIP_WEAPON && p->state.heavy_shoot))
		return slot_table[type].heavy_describe;

	if (slot_table[type].name_in_desc)
		return format(slot_table[type].describe, p->body.slots[slot].name);
	else
		return slot_table[type].describe;
}

/**
 * Determine which equipment slot (if any) an item likes. The slot might (or
 * might not) be open, but it is a slot which the object could be equipped in.
 *
 * For items where multiple slots could work (e.g. rings), the function
 * will try to return an open slot if possible.
 */
int wield_slot(const struct object *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
		case TV_BOW: return slot_by_type(player, EQUIP_BOW, FALSE);
		case TV_AMULET: return slot_by_type(player, EQUIP_AMULET, FALSE);
		case TV_CLOAK: return slot_by_type(player, EQUIP_CLOAK, FALSE);
		case TV_SHIELD: return slot_by_type(player, EQUIP_SHIELD, FALSE);
		case TV_GLOVES: return slot_by_type(player, EQUIP_GLOVES, FALSE);
		case TV_BOOTS: return slot_by_type(player, EQUIP_BOOTS, FALSE);
	}

	if (tval_is_melee_weapon(o_ptr))
		return slot_by_type(player, EQUIP_WEAPON, FALSE);
	else if (tval_is_ring(o_ptr))
		return slot_by_type(player, EQUIP_RING, FALSE);
	else if (tval_is_light(o_ptr))
		return slot_by_type(player, EQUIP_LIGHT, FALSE);
	else if (tval_is_body_armor(o_ptr))
		return slot_by_type(player, EQUIP_BODY_ARMOR, FALSE);
	else if (tval_is_head_armor(o_ptr))
		return slot_by_type(player, EQUIP_HAT, FALSE);

	/* No slot available */
	return (-1);
}


/**
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 * If any armor is damaged (or resists), the player takes less damage.
 */
int minus_ac(struct player *p)
{
	int i, count = 0;
	object_type *obj = NULL;

	char o_name[80];

	/* Avoid crash during monster power calculations */
	if (!p->gear) return FALSE;

	/* Count the armor slots */
	for (i = 0; i < player->body.count; i++) {
		/* Ignore non-armor */
		if (slot_type_is(i, EQUIP_WEAPON)) continue;
		if (slot_type_is(i, EQUIP_BOW)) continue;
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

		/* Add */
		count++;
	}

	/* Pick one at random */
	for (i = player->body.count - 1; i >= 0; i--) {
		/* Ignore non-armor */
		if (slot_type_is(i, EQUIP_WEAPON)) continue;
		if (slot_type_is(i, EQUIP_BOW)) continue;
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

		if (one_in_(count--)) break;
	}

	/* Get the item */
	obj = slot_object(player, i);

	/* Nothing to damage */
	if (!obj) return (FALSE);

	/* No damage left to be done */
	if (obj->ac + obj->to_a <= 0) return (FALSE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

	/* Object resists */
	if (obj->el_info[ELEM_ACID].flags & EL_INFO_IGNORE) {
		msg("Your %s is unaffected!", o_name);
		return (TRUE);
	}

	/* Message */
	msg("Your %s is damaged!", o_name);

	/* Damage the item */
	obj->to_a--;

	p->upkeep->update |= (PU_BONUS);
	p->upkeep->redraw |= (PR_EQUIP);

	/* Item was damaged */
	return (TRUE);
}

/**
 * Convert a gear object into a one character label.
 */
char gear_to_label(struct object *obj)
{
	int i;

	/* Equipment is easy */
	if (object_is_equipped(player->body, obj))
		return I2A(equipped_item_slot(player->body, obj));

	/* Check the quiver */
	for (i = 0; i < z_info->quiver_size; i++)
		if (player->upkeep->quiver[i] == obj) return I2A(i);

	/* Check the inventory */
	for (i = 0; i < z_info->pack_size; i++)
		if (player->upkeep->inven[i] == obj) return I2A(i);

	return '\0';
}

/**
 * Remove an object from the gear list, leaving it unattached
 * \param obj the object being tested
 * \return whether an object was removed
 */
bool gear_excise_object(struct object *obj)
{
	int i;

	pile_excise(&player->gear, obj);

	/* Make sure it isn't still equipped */
	for (i = 0; i < player->body.count; i++) {
		if (slot_object(player, i) == obj)
			player->body.slots[i].obj = NULL;
	}

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS | PU_MANA | PU_INVEN);
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	return TRUE;
}

struct object *gear_last_item(void)
{
	return pile_last_item(player->gear);
}

void gear_insert_end(struct object *obj)
{
	pile_insert_end(&player->gear, obj);
}

/**
 * Remove an amount of an object from the inventory or quiver, returning
 * a detached object which can be used.
 *
 * Optionally describe what remains.
 */
struct object *gear_object_for_use(struct object *obj, int num, bool message)
{
	struct object *usable;
	char name[80];
	char label = gear_to_label(obj);
	bool artifact = obj->artifact &&
		(object_is_known(obj) || object_name_is_visible(obj));

	/* Bounds check */
	num = MIN(num, obj->number);

	/* Prepare a name if necessary */
	if (message) {
		/* Artifacts */
		if (artifact)
			object_desc(name, sizeof(name), obj, ODESC_FULL | ODESC_SINGULAR);
		else {
			/* Describe as if it's already reduced */
			obj->number -= num;
			object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);
			obj->number += num;
		}
	}

	/* Split off a usable object if necessary */
	if (obj->number > num) {
		usable = object_split(obj, num);
	} else {
		/* We're using the entire stack */
		usable = obj;
		gear_excise_object(usable);

		/* Stop tracking item */
		if (tracked_object_is(player->upkeep, obj))
			track_object(player->upkeep, NULL);

		/* Inventory has changed, so disable repeat command */ 
		cmd_disable_repeat();
	}

	/* Change the weight */
	player->upkeep->total_weight -= (num * obj->weight);

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS | PU_MANA | PU_INVEN | PU_TORCH);
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Print a message if desired */
	if (message) {
		if (artifact)
			msg("You no longer have the %s (%c).", name, label);
		else
			msg("You have %s (%c).", name, label);
	}

	return usable;
}

/**
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(const object_type *obj)
{
	/* Empty slot? */
	if (pack_slots_used(player) < z_info->pack_size) return TRUE;

	/* Check if it can stack */
	if (inven_stack_okay(obj)) return TRUE;

	/* Nope */
	return FALSE;
}

/**
 * Check to see if an item is stackable in the inventory
 */
bool inven_stack_okay(const object_type *o_ptr)
{
	struct object *gear_obj;
	int new_number;
	bool extra_slot;

	/* Check for similarity */
	for (gear_obj = player->gear; gear_obj; gear_obj = gear_obj->next) {
		/* Skip equipped items and non-objects */
		if (object_is_equipped(player->body, gear_obj))
			continue;
		if (!gear_obj)
			continue;

		/* Check if the two items can be combined */
		if (object_similar(gear_obj, o_ptr, OSTACK_PACK))
			break;
	}

	/* Definite no */
	if (!gear_obj) return FALSE;

	/* Add it and see what happens */
	gear_obj->number += o_ptr->number;
	extra_slot = (gear_obj->number > z_info->stack_size);
	new_number = pack_slots_used(player);
	gear_obj->number -= o_ptr->number;

	/* Analyse the results */
	if (new_number + (extra_slot ? 1 : 0) > z_info->pack_size)
		return FALSE;

	return TRUE;
}

/**
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(struct object *obj)
{
	/* Require staff/wand */
	if (!tval_can_have_charges(obj)) return;

	/* Require known item */
	if (!object_is_known(obj)) return;

	/* Print a message */
	msg("You have %d charge%s remaining.", obj->pval,
	    (obj->pval != 1) ? "s" : "");
}


/**
 * Add an item to the players inventory.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so, using object_similar() and object_absorb(), else,
 * the item will be placed into the first available gear array index.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined.  This may be tricky.  See "dungeon.c" for info.
 *
 * Note that this code removes any location information from the object once
 * it is placed into the inventory, but takes no responsibility for removing
 * the object from any other pile it was in.
 */
bool inven_carry(struct player *p, struct object *obj, bool message)
{
	struct object *gear_obj;
	char o_name[80];

	/* Apply an autoinscription */
	apply_autoinscription(obj);

	/* Check for combining */
	for (gear_obj = p->gear; gear_obj; gear_obj = gear_obj->next) {
		/* Can't stack equipment */
		if (object_is_equipped(p->body, gear_obj))
			continue;

		/* Check if the two items can be combined */
		if (object_similar(gear_obj, obj, OSTACK_PACK)) {
			/* Increase the weight */
			p->upkeep->total_weight += (obj->number * obj->weight);

			/* Combine the items */
			object_absorb(gear_obj, obj);

			/* Describe the combined object */
			object_desc(o_name, sizeof(o_name), gear_obj,
						ODESC_PREFIX | ODESC_FULL);

			/* Recalculate bonuses */
			p->upkeep->update |= (PU_BONUS | PU_INVEN);

			/* Redraw stuff */
			p->upkeep->redraw |= (PR_INVEN);

			/* Inventory will need updating */
			update_stuff(player->upkeep);

			/* Optionally, display a message */
			if (message)
				msg("You have %s (%c).", o_name, gear_to_label(gear_obj));

			/* Success */
			return TRUE;
		}
	}

	/* Paranoia */
	if (pack_slots_used(p) > z_info->pack_size)
		return FALSE;

	/* Add to the end of the list */
	gear_insert_end(obj);

	/* Remove cave object details */
	obj->held_m_idx = 0;
	obj->iy = obj->ix = 0;
	obj->marked = FALSE;

	/* Update the inventory */
	p->upkeep->total_weight += (obj->number * obj->weight);
	p->upkeep->update |= (PU_BONUS | PU_INVEN);
	p->upkeep->notice |= (PN_COMBINE);
	p->upkeep->redraw |= (PR_INVEN);

	/* Inventory will need updating */
	update_stuff(player->upkeep);

	/* Hobbits ID mushrooms on pickup, gnomes ID wands and staffs on pickup */
	if (!object_is_known(obj)) {
		if (player_has(PF_KNOW_MUSHROOM) && tval_is_mushroom(obj)) {
			do_ident_item(obj);
			msg("Mushrooms for breakfast!");
		} else if (player_has(PF_KNOW_ZAPPER) && tval_is_zapper(obj))
			do_ident_item(obj);
	}

	/* Optionally, display a message */
	if (message) {
		/* Describe the object */
		object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

		/* Message */
		msg("You have %s (%c).", o_name, gear_to_label(obj));
	}

	return TRUE;
}


/**
 * Take off a non-cursed equipment item
 *
 * Note that only one item at a time can be wielded per slot.
 *
 * Note that taking off an item when "full" may cause that item
 * to fall to the ground.
 */
void inven_takeoff(struct object *obj)
{
	int slot = equipped_item_slot(player->body, obj);
	const char *act;
	char o_name[80];

	/* Paranoia */
	if (slot == player->body.count) return;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

	/* Describe removal by slot */
	if (slot_type_is(slot, EQUIP_WEAPON))
		act = "You were wielding";
	else if (slot_type_is(slot, EQUIP_BOW))
		act = "You were holding";
	else if (slot_type_is(slot, EQUIP_LIGHT))
		act = "You were holding";
	else
		act = "You were wearing";

	/* De-equip the object */
	player->body.slots[slot].obj = NULL;

	/* Message */
	msgt(MSG_WIELD, "%s %s (%c).", act, o_name, I2A(slot));

	player->upkeep->update |= (PU_BONUS | PU_INVEN);
	player->upkeep->notice |= (PN_IGNORE | PN_COMBINE);

	return;
}


/**
 * Drop (some of) a non-cursed inventory/equipment item "near" the current
 * location
 *
 * There are two cases here - a single object or entire stack is being dropped,
 * or part of a stack is being split off and dropped
 */
void inven_drop(struct object *obj, int amt)
{
	int py = player->py;
	int px = player->px;
	int num = 0;
	struct object *dropped;

	char o_name[80];

	/* Error check */
	if (amt <= 0)
		return;

	/* Not too many */
	if (amt > obj->number) amt = obj->number;

	/* Take off equipment */
	if (object_is_equipped(player->body, obj))
		inven_takeoff(obj);

	/* Describe object */
	num = obj->number;
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
	obj->number = num;

	/* Message */
	msg("You drop %s (%c).", o_name, gear_to_label(obj));

	/* Get the object */
	dropped = gear_object_for_use(obj, amt, TRUE);

	/* Drop it near the player */
	drop_near(cave, dropped, 0, py, px, FALSE);

	event_signal(EVENT_INVENTORY);
}



/**
 * Return whether each stack of objects can be merged into two uneven stacks.
 */
static bool inven_can_stack_partial(const object_type *o_ptr,
									const object_type *j_ptr,
									object_stack_t mode)
{
	if (!(mode & OSTACK_STORE)) {
		int total = o_ptr->number + j_ptr->number;
		int remainder = total - (z_info->stack_size);

		if (remainder > z_info->stack_size)
			return FALSE;
	}

	return object_stackable(o_ptr, j_ptr, mode);
}

/**
 * Combine items in the pack, confirming no blank objects or gold
 */
void combine_pack(void)
{
	struct object *obj1, *obj2;
	bool display_message = FALSE;
	bool redraw = FALSE;

	/* Combine the pack (backwards) */
	for (obj1 = gear_last_item(); obj1; obj1 = obj1->prev) {
		assert(obj1->kind);
		assert(!tval_is_money(obj1));

		/* Scan the items above that item */
		for (obj2 = player->gear; obj2 && obj2 != obj1; obj2 = obj2->next) {
			assert(obj2->kind);

			/* Can we drop "obj1" onto "obj2"? */
			if (object_similar(obj2, obj1, OSTACK_PACK)) {
				display_message = TRUE;
				redraw = TRUE;
				object_absorb(obj2, obj1);
			} else if (inven_can_stack_partial(obj2, obj1, OSTACK_PACK)) {
				/* Setting this to TRUE spams the combine message. */
				display_message = FALSE;
				redraw = TRUE;
				object_absorb_partial(obj2, obj1);
				break;
			}
		}
	}

	/* Redraw stuff */
	if (redraw) {
		player->upkeep->redraw |= (PR_INVEN);
		player->upkeep->update |= (PU_INVEN);
	}

	/* Message */
	if (display_message) {
		msg("You combine some items in your pack.");

		/* Stop "repeat last command" from working. */
		cmd_disable_repeat();
	}
}

/**
 * Returns whether the pack is holding the maximum number of items.
 */
bool pack_is_full(void)
{
	return pack_slots_used(player) == z_info->pack_size ? TRUE : FALSE;
}

/**
 * Returns whether the pack is holding the more than the maximum number of
 * items. If this is true, calling pack_overflow() will trigger a pack overflow.
 */
bool pack_is_overfull(void)
{
	return pack_slots_used(player) > z_info->pack_size ? TRUE : FALSE;
}

/**
 * Overflow an item from the pack, if it is overfull.
 */
void pack_overflow(void)
{
	int i;
	struct object *obj = NULL;
	char o_name[80];

	if (!pack_is_overfull()) return;

	/* Disturbing */
	disturb(player, 0);

	/* Warning */
	msg("Your pack overflows!");

	/* Find the last inventory item */
	for (i = 1; i <= z_info->pack_size; i++)
		if (!player->upkeep->inven[i])
			break;

	/* Last object was the previous index */
	obj = player->upkeep->inven[i - 1];

	/* Rule out weirdness (like pack full, but inventory empty) */
	assert(obj != NULL);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", o_name, I2A(i - 1));

	/* Excise the object and drop it (carefully) near the player */
	gear_excise_object(obj);
	drop_near(cave, obj, 0, player->py, player->px, FALSE);

	/* Describe */
	if (obj->artifact)
		msg("You no longer have the %s (%c).", o_name, I2A(i - 1));
	else
		msg("You no longer have %s (%c).", o_name, I2A(i - 1));

	/* Notice stuff (if needed) */
	if (player->upkeep->notice) notice_stuff(player->upkeep);

	/* Update stuff (if needed) */
	if (player->upkeep->update) update_stuff(player->upkeep);

	/* Redraw stuff (if needed) */
	if (player->upkeep->redraw) redraw_stuff(player->upkeep);
}
