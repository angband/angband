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
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
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
	{ EQUIP_MAX, false, false, NULL, NULL, NULL }
};

int slot_by_name(struct player *p, const char *name)
{
	int i;

	/* Look for the correctly named slot */
	for (i = 0; i < p->body.count; i++) {
		if (streq(name, p->body.slots[i].name)) {
			break;
		}
	}

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
	/* Assume default body if no player */
	struct player_body body = player ? player->body : bodies[0];

	return body.slots[slot].type == type ? true : false;
}

struct object *slot_object(struct player *p, int slot)
{
	/* Ensure a valid body */
	if (p->body.slots && p->body.slots[slot].obj) {
		return p->body.slots[slot].obj;
	}

	return NULL;
}

struct object *equipped_item_by_slot_name(struct player *p, const char *name)
{
	/* Ensure a valid body */
	if (p->body.slots) {
		return slot_object(p, slot_by_name(p, name));
	}

	return NULL;
}

int object_slot(struct player_body body, const struct object *obj)
{
	int i;

	for (i = 0; i < body.count; i++) {
		if (obj == body.slots[i].obj) {
			break;
		}
	}

	return i;
}

bool object_is_equipped(struct player_body body, const struct object *obj)
{
	return object_slot(body, obj) < body.count;
}

bool object_is_carried(struct player *p, const struct object *obj)
{
	return pile_contains(p->gear, obj);
}

/**
 * Check if an object is in the quiver
 */
static bool object_is_in_quiver(struct player *p, const struct object *obj)
{
	int i;

	for (i = 0; i < z_info->quiver_size; i++) {
		if (obj == p->upkeep->quiver[i]) {
			return true;
		}
	}

	return false;
}

/**
 * Calculate the number of pack slots used by the current gear.
 *
 * Note that this function does not check that there are adequate slots in the
 * quiver, just the total quantity of missiles.
 */
int pack_slots_used(struct player *p)
{
	struct object *obj;
	int quiver_slots = 0;
	int pack_slots = 0;
	int quiver_ammo = 0;

	for (obj = p->gear; obj; obj = obj->next) {
		/* Equipment doesn't count */
		if (!object_is_equipped(p->body, obj)) {
			/* Check if it could be in the quiver */
			if (tval_is_ammo(obj) && quiver_slots < z_info->quiver_size) {
				quiver_slots++;
				quiver_ammo += obj->number;
			} else {
				/* Count regular slots */
				pack_slots++;
			}
		}
	}

	/* Full slots */
	pack_slots += quiver_ammo / z_info->quiver_slot_size;

	/* Plus one for any remainder */
	if (quiver_ammo % z_info->quiver_slot_size) {
		pack_slots++;
	}

	return pack_slots;
}

/*
 * Return a string mentioning how a given item is carried
 */
const char *equip_mention(struct player *p, int slot)
{
	int type = p->body.slots[slot].type;

	/* Heavy */
	if ((type == EQUIP_WEAPON && p->state.heavy_wield) ||
			(type == EQUIP_WEAPON && p->state.heavy_shoot))
		return slot_table[type].heavy_describe;
	else if (slot_table[type].name_in_desc)
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
	int type = p->body.slots[slot].type;

	/* Heavy */
	if ((type == EQUIP_WEAPON && p->state.heavy_wield) ||
			(type == EQUIP_WEAPON && p->state.heavy_shoot))
		return slot_table[type].heavy_describe;
	else if (slot_table[type].name_in_desc)
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
int wield_slot(const struct object *obj)
{
	/* Slot for equipment */
	switch (obj->tval)
	{
		case TV_BOW: return slot_by_type(player, EQUIP_BOW, false);
		case TV_AMULET: return slot_by_type(player, EQUIP_AMULET, false);
		case TV_CLOAK: return slot_by_type(player, EQUIP_CLOAK, false);
		case TV_SHIELD: return slot_by_type(player, EQUIP_SHIELD, false);
		case TV_GLOVES: return slot_by_type(player, EQUIP_GLOVES, false);
		case TV_BOOTS: return slot_by_type(player, EQUIP_BOOTS, false);
	}

	if (tval_is_melee_weapon(obj))
		return slot_by_type(player, EQUIP_WEAPON, false);
	else if (tval_is_ring(obj))
		return slot_by_type(player, EQUIP_RING, false);
	else if (tval_is_light(obj))
		return slot_by_type(player, EQUIP_LIGHT, false);
	else if (tval_is_body_armor(obj))
		return slot_by_type(player, EQUIP_BODY_ARMOR, false);
	else if (tval_is_head_armor(obj))
		return slot_by_type(player, EQUIP_HAT, false);

	/* No slot available */
	return -1;
}


/**
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 * If any armor is damaged (or resists), the player takes less damage.
 */
bool minus_ac(struct player *p)
{
	int i, count = 0;
	struct object *obj = NULL;

	/* Avoid crash during monster power calculations */
	if (!p->gear) return false;

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

	/* If we can still damage the item */
	if (obj && (obj->ac + obj->to_a > 0)) {
		char o_name[80];
		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

		/* Object resists */
		if (obj->el_info[ELEM_ACID].flags & EL_INFO_IGNORE) {
			msg("Your %s is unaffected!", o_name);
		} else {
			msg("Your %s is damaged!", o_name);

			/* Damage the item */
			obj->to_a--;
			if (p->obj_k->to_a)
				obj->known->to_a = obj->to_a;

			p->upkeep->update |= (PU_BONUS);
			p->upkeep->redraw |= (PR_EQUIP);
		}

		/* There was an effect */
		return true;
	} else {
		/* No damage or effect */
		return false;
	}
}

/**
 * Convert a gear object into a one character label.
 */
char gear_to_label(struct object *obj)
{
	int i;

	/* Equipment is easy */
	if (object_is_equipped(player->body, obj)) {
		return I2A(equipped_item_slot(player->body, obj));
	}

	/* Check the quiver */
	for (i = 0; i < z_info->quiver_size; i++) {
		if (player->upkeep->quiver[i] == obj) {
			return I2D(i);
		}
	}

	/* Check the inventory */
	for (i = 0; i < z_info->pack_size; i++) {
		if (player->upkeep->inven[i] == obj) {
			return I2A(i);
		}
	}

	return '\0';
}

/**
 * Remove an object from the gear list, leaving it unattached
 * \param obj the object being tested
 * \return whether an object was removed
 */
static bool gear_excise_object(struct object *obj)
{
	int i;

	pile_excise(&player->gear_k, obj->known);
	pile_excise(&player->gear, obj);

	/* Change the weight */
	player->upkeep->total_weight -= (obj->number * obj->weight);

	/* Make sure it isn't still equipped */
	for (i = 0; i < player->body.count; i++) {
		if (slot_object(player, i) == obj) {
			player->body.slots[i].obj = NULL;
			player->upkeep->equip_cnt--;
		}
	}

	/* Update the gear */
	calc_inventory(player->upkeep, player->gear, player->body);

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS);
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	return true;
}

struct object *gear_last_item(void)
{
	return pile_last_item(player->gear);
}

void gear_insert_end(struct object *obj)
{
	pile_insert_end(&player->gear, obj);
	pile_insert_end(&player->gear_k, obj->known);
}

/**
 * Remove an amount of an object from the inventory or quiver, returning
 * a detached object which can be used.
 *
 * Optionally describe what remains.
 */
struct object *gear_object_for_use(struct object *obj, int num, bool message,
								   bool *none_left)
{
	struct object *usable;
	char name[80];
	char label = gear_to_label(obj);
	bool artifact = (obj->known->artifact != NULL);

	/* Bounds check */
	num = MIN(num, obj->number);

	/* Split off a usable object if necessary */
	if (obj->number > num) {
		usable = object_split(obj, num);

		/* Change the weight */
		player->upkeep->total_weight -= (num * obj->weight);

		if (message) {
			object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);
		}
	} else {
		if (message) {
			if (artifact) {
				object_desc(name, sizeof(name), obj, ODESC_FULL | ODESC_SINGULAR);
			} else {
				/* Describe zero amount */
				obj->number = 0;
				object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);
				obj->number = num;
			}
		}

		/* We're using the entire stack */
		usable = obj;
		gear_excise_object(usable);
		*none_left = true;

		/* Stop tracking item */
		if (tracked_object_is(player->upkeep, obj))
			track_object(player->upkeep, NULL);

		/* Inventory has changed, so disable repeat command */
		cmd_disable_repeat();
	}

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS);
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
 * Check how many missiles can be put in the quiver without increasing the
 * number of pack slots used.
 *
 * Returns the quantity from a given stack of missiles that can be added.
 */
static int quiver_absorb_num(const struct object *obj)
{
	/* Must be ammo */
	if (tval_is_ammo(obj)) {
		int i, quiver_count = 0, space_free = 0;

		/* Count the current space this object could go into */
		for (i = 0; i < z_info->quiver_size; i++) {
			struct object *quiver_obj = player->upkeep->quiver[i];
			if (quiver_obj) {
				quiver_count += quiver_obj->number;
				if (object_stackable(quiver_obj, obj, OSTACK_PACK))
					space_free += z_info->quiver_slot_size - quiver_obj->number;
			} else {
				space_free += z_info->quiver_slot_size;
			}
		}

		if (space_free) {
			/* Check we won't need another pack slot */
			quiver_count += z_info->quiver_slot_size;
			while (quiver_count > z_info->quiver_slot_size)
				quiver_count -= z_info->quiver_slot_size;

			/* Return the number, or the number that will fit */
			space_free = MIN(space_free, z_info->quiver_slot_size - quiver_count);
			return MIN(obj->number, space_free);
		}
	}

	/* No ammo or no space */
	return 0;
}

/**
 * Calculate how much of an item is can be carried in the inventory or quiver.
 *
 * Optionally only return a positive value if there is already a similar object.
 */
int inven_carry_num(const struct object *obj, bool stack)
{
	/* Check for similarity */
	if (stack) {
		struct object *gear_obj;

		for (gear_obj = player->gear; gear_obj; gear_obj = gear_obj->next) {
			if (!object_is_equipped(player->body, gear_obj) &&
					object_stackable(gear_obj, obj, OSTACK_PACK)) {
				break;
			}
		}

		/* No similar object, so no stacking */
		if (!gear_obj) {
			return 0;
		}
	}

	/* Free inventory slots, so there is definitely room */
	if (pack_slots_used(player) < z_info->pack_size) {
		return obj->number;
	} else {
		int i;

		/* Absorb as many as we can in the quiver */
		int num_left = obj->number - quiver_absorb_num(obj);

		/* See if we can add to a part full inventory slot */
		for (i = 0; i < z_info->pack_size; i++) {
			struct object *inven_obj = player->upkeep->inven[i];
			if (inven_obj && object_stackable(inven_obj, obj, OSTACK_PACK)) {
				num_left -= inven_obj->kind->base->max_stack - inven_obj->number;
			}
		}

		/* Return the number we can absorb */
		return obj->number - MAX(num_left, 0);
	}
}

/**
 * Check if we have space for some of an item in the pack, optionally requiring
 * stacking
 */
bool inven_carry_okay(const struct object *obj)
{
	return inven_carry_num(obj, false) > 0;
}

/**
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(struct object *obj)
{
	/* Require staff/wand */
	if (tval_can_have_charges(obj) && object_flavor_is_aware(obj)) {
		msg("You have %d charge%s remaining.",
				obj->pval,
				PLURAL(obj->pval));
	}
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
void inven_carry(struct player *p, struct object *obj, bool absorb,
				 bool message)
{
	bool combining = false;

	/* Check for combining, if appropriate */
	if (absorb) {
		struct object *combine_item = NULL;

		struct object *gear_obj = p->gear;
		while ((combine_item == NULL) && (gear_obj != NULL)) {
			if (!object_is_equipped(p->body, gear_obj) &&
					object_similar(gear_obj, obj, OSTACK_PACK)) {
				combine_item = gear_obj;
			}

			gear_obj = gear_obj->next;
		}

		if (combine_item) {
			/* Increase the weight */
			p->upkeep->total_weight += (obj->number * obj->weight);

			/* Combine the items, and their known versions */
			object_absorb(combine_item->known, obj->known);
			obj->known = NULL;
			object_absorb(combine_item, obj);

			/* Ensure numbers are aligned (should not be necessary, but safe) */
			combine_item->known->number = combine_item->number;

			obj = combine_item;
			combining = true;
		}
	}

	/* We didn't manage the find an object to combine with */
	if (!combining) {
		/* Paranoia */
		assert(pack_slots_used(p) <= z_info->pack_size);

		gear_insert_end(obj);
		apply_autoinscription(obj);

		/* Remove cave object details */
		obj->held_m_idx = 0;
		obj->grid = loc(0, 0);
		obj->known->grid = loc(0, 0);

		/* Update the inventory */
		p->upkeep->total_weight += (obj->number * obj->weight);
		p->upkeep->notice |= (PN_COMBINE);

		/* Hobbits ID mushrooms on pickup, gnomes ID wands and staffs on pickup */
		if (!object_flavor_is_aware(obj)) {
			if (player_has(player, PF_KNOW_MUSHROOM) && tval_is_mushroom(obj)) {
				object_flavor_aware(obj);
				msg("Mushrooms for breakfast!");
			} else if (player_has(player, PF_KNOW_ZAPPER) && tval_is_zapper(obj))
				object_flavor_aware(obj);
		}
	}

	p->upkeep->update |= (PU_BONUS | PU_INVEN);
	p->upkeep->redraw |= (PR_INVEN);
	update_stuff(player);

	if (message) {
		char o_name[80];
		object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
		msg("You have %s (%c).", o_name, gear_to_label(obj));
	}

	if (object_is_in_quiver(p, obj))
		sound(MSG_QUIVER);
}


/**
 * Wield or wear a single item from the pack or floor
 */
void inven_wield(struct object *obj, int slot)
{
	struct object *wielded, *old = player->body.slots[slot].obj;

	const char *fmt;
	char o_name[80];
	bool dummy = false;

	/* Increase equipment counter if empty slot */
	if (old == NULL)
		player->upkeep->equip_cnt++;

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* It's either a gear object or a floor object */
	if (object_is_carried(player, obj)) {
		/* Split off a new object if necessary */
		if (obj->number > 1) {
			wielded = gear_object_for_use(obj, 1, false, &dummy);

			/* The new item needs new gear and known gear entries */
			wielded->next = obj->next;
			obj->next = wielded;
			wielded->prev = obj;
			if (wielded->next)
				(wielded->next)->prev = wielded;
			wielded->known->next = obj->known->next;
			obj->known->next = wielded->known;
			wielded->known->prev = obj->known;
			if (wielded->known->next)
				(wielded->known->next)->prev = wielded->known;
		} else {
			/* Just use the object directly */
			wielded = obj;
		}
	} else {
		/* Get a floor item and carry it */
		wielded = floor_object_for_use(obj, 1, false, &dummy);
		inven_carry(player, wielded, false, false);
	}

	/* Wear the new stuff */
	player->body.slots[slot].obj = wielded;

	/* Do any ID-on-wield */
	object_learn_on_wield(player, wielded);

	/* Where is the item now */
	if (tval_is_melee_weapon(wielded))
		fmt = "You are wielding %s (%c).";
	else if (wielded->tval == TV_BOW)
		fmt = "You are shooting with %s (%c).";
	else if (tval_is_light(wielded))
		fmt = "Your light source is %s (%c).";
	else
		fmt = "You are wearing %s (%c).";

	/* Describe the result */
	object_desc(o_name, sizeof(o_name), wielded, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msgt(MSG_WIELD, fmt, o_name, I2A(slot));

	/* Sticky flag geats a special mention */
	if (of_has(wielded->flags, OF_STICKY)) {
		/* Warn the player */
		msgt(MSG_CURSED, "Oops! It feels deathly cold!");
	}

	/* See if we have to overflow the pack */
	combine_pack();
	pack_overflow(old);

	/* Recalculate bonuses, torch, mana, gear */
	player->upkeep->notice |= (PN_IGNORE);
	player->upkeep->update |= (PU_BONUS | PU_INVEN | PU_UPDATE_VIEW);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP | PR_ARMOR);
	player->upkeep->redraw |= (PR_STATS | PR_HP | PR_MANA | PR_SPEED);
	update_stuff(player);

	/* Disable repeats */
	cmd_disable_repeat();
}


/**
 * Take off a non-cursed equipment item
 *
 * Note that taking off an item when "full" may cause that item
 * to fall to the ground.
 *
 * Note also that this function does not try to combine the taken off item
 * with other inventory items - that must be done by the calling function.
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
	player->upkeep->equip_cnt--;

	player->upkeep->update |= (PU_BONUS | PU_INVEN | PU_UPDATE_VIEW);
	player->upkeep->notice |= (PN_IGNORE);
	update_stuff(player);

	/* Message */
	msgt(MSG_WIELD, "%s %s (%c).", act, o_name, gear_to_label(obj));

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
	struct object *dropped;
	bool none_left = false;
	bool quiver = false;

	char name[80];
	char label;

	/* Error check */
	if (amt <= 0)
		return;

	/* Check it is still held, in case there were two drop commands queued
	 * for this item.  This is in theory not ideal, but in practice should
	 * be safe. */
	if (!object_is_carried(player, obj))
		return;

	/* Get where the object is now */
	label = gear_to_label(obj);

	/* Is it in the quiver? */
	if (object_is_in_quiver(player, obj))
		quiver = true;

	/* Not too many */
	if (amt > obj->number) amt = obj->number;

	/* Take off equipment, don't combine */
	if (object_is_equipped(player->body, obj))
		inven_takeoff(obj);

	/* Get the object */
	dropped = gear_object_for_use(obj, amt, false, &none_left);

	/* Describe the dropped object */
	object_desc(name, sizeof(name), dropped, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", name, label);

	/* Describe what's left */
	if (dropped->artifact) {
		object_desc(name, sizeof(name), dropped,
					ODESC_FULL | ODESC_SINGULAR);
		msg("You no longer have the %s (%c).", name, label);
	} else if (none_left) {
		/* Play silly games to get the right description */
		int number = dropped->number;
		dropped->number = 0;
		object_desc(name, sizeof(name), dropped, ODESC_PREFIX | ODESC_FULL);
		msg("You have %s (%c).", name, label);
		dropped->number = number;
	} else {
		object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);
		msg("You have %s (%c).", name, label);
	}

	/* Drop it near the player */
	drop_near(cave, &dropped, 0, player->grid, false);

	/* Sound for quiver objects */
	if (quiver)
		sound(MSG_QUIVER);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/**
 * Return whether each stack of objects can be merged into two uneven stacks.
 */
static bool inven_can_stack_partial(const struct object *obj1,
									const struct object *obj2,
									object_stack_t mode)
{
	if (!(mode & OSTACK_STORE)) {
		int total = obj1->number + obj2->number;
		int remainder = total - obj1->kind->base->max_stack;

		if (remainder > obj1->kind->base->max_stack)
			return false;
	}

	return object_stackable(obj1, obj2, mode);
}


/**
 * Combine items in the pack, confirming no blank objects or gold
 */
void combine_pack(void)
{
	struct object *obj1, *obj2, *prev;
	bool display_message = false;

	/* Combine the pack (backwards) */
	obj1 = gear_last_item();
	while (obj1) {
		assert(obj1->kind);
		assert(!tval_is_money(obj1));
		prev = obj1->prev;

		/* Scan the items above that item */
		for (obj2 = player->gear; obj2 && obj2 != obj1; obj2 = obj2->next) {
			assert(obj2->kind);

			/* Can we drop "obj1" onto "obj2"? */
			if (object_similar(obj2, obj1, OSTACK_PACK)) {
				display_message = true;
				object_absorb(obj2->known, obj1->known);
				obj1->known = NULL;
				object_absorb(obj2, obj1);

				/* Ensure numbers align (should not be necessary, but safer) */
				obj2->known->number = obj2->number;

				break;
			} else if (inven_can_stack_partial(obj2, obj1, OSTACK_PACK)) {
				/* Setting this to true spams the combine message. */
				display_message = false;
				object_absorb_partial(obj2->known, obj1->known);
				object_absorb_partial(obj2, obj1);

				/* Ensure numbers align (should not be necessary, but safer) */
				obj2->known->number = obj2->number;
				obj1->known->number = obj1->number;

				break;
			}
		}
		obj1 = prev;
	}

	calc_inventory(player->upkeep, player->gear, player->body);

	/* Redraw gear */
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);

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
	return pack_slots_used(player) == z_info->pack_size;
}

/**
 * Returns whether the pack is holding the more than the maximum number of
 * items. If this is true, calling pack_overflow() will trigger a pack overflow.
 */
bool pack_is_overfull(void)
{
	return pack_slots_used(player) > z_info->pack_size;
}

/**
 * Overflow an item from the pack, if it is overfull.
 */
void pack_overflow(struct object *obj)
{
	int i;
	char o_name[80];
	bool artifact = false;

	if (!pack_is_overfull()) return;

	/* Disturbing */
	disturb(player, 0);

	/* Warning */
	msg("Your pack overflows!");

	/* Get the last proper item */
	for (i = 1; i <= z_info->pack_size; i++)
		if (!player->upkeep->inven[i])
			break;

	/* Drop the last inventory item unless requested otherwise */
	if (!obj) {
		obj = player->upkeep->inven[i - 1];
	}

	/* Rule out weirdness (like pack full, but inventory empty) */
	assert(obj != NULL);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
	if (obj->artifact) {
		artifact = true;
	}

	/* Message */
	msg("You drop %s.", o_name);

	/* Excise the object and drop it (carefully) near the player */
	gear_excise_object(obj);
	drop_near(cave, &obj, 0, player->grid, false);

	/* Describe */
	if (artifact)
		msg("You no longer have the %s.", o_name);
	else
		msg("You no longer have %s.", o_name);

	/* Notice, update, redraw */
	if (player->upkeep->notice) notice_stuff(player);
	if (player->upkeep->update) update_stuff(player);
	if (player->upkeep->redraw) redraw_stuff(player);
}
