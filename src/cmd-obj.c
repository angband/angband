/**
 * \file cmd-obj.c
 * \brief Handle objects in various ways
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
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
#include "cmd-core.h"
#include "cmds.h"
#include "effects.h"
#include "game-input.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "target.h"
#include "trap.h"

/**
 * ------------------------------------------------------------------------
 * Utility bits and bobs
 * ------------------------------------------------------------------------
 */

/**
 * Check to see if the player can use a rod/wand/staff/activatable object.
 */
static int check_devices(struct object *obj)
{
	int fail;
	const char *action;
	const char *what = NULL;

	/* Get the right string */
	if (tval_is_rod(obj)) {
		action = "zap the rod";
	} else if (tval_is_wand(obj)) {
		action = "use the wand";
		what = "wand";
	} else if (tval_is_staff(obj)) {
		action = "use the staff";
		what = "staff";
	} else {
		action = "activate it";
	}

	/* Figure out how hard the item is to use */
	fail = get_use_device_chance(obj);

	/* Roll for usage */
	if (randint1(1000) < fail) {
		event_signal(EVENT_INPUT_FLUSH);
		msg("You failed to %s properly.", action);
		return FALSE;
	}

	/* Notice empty staffs */
	if (what && obj->pval <= 0) {
		event_signal(EVENT_INPUT_FLUSH);
		msg("The %s has no charges left.", what);
		return FALSE;
	}

	return TRUE;
}


/**
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


typedef enum {
	ART_TAG_NONE,
	ART_TAG_NAME,
	ART_TAG_KIND,
	ART_TAG_VERB,
	ART_TAG_VERB_IS
} art_tag_t;

static art_tag_t art_tag_lookup(const char *tag)
{
	if (strncmp(tag, "name", 4) == 0)
		return ART_TAG_NAME;
	else if (strncmp(tag, "kind", 4) == 0)
		return ART_TAG_KIND;
	else if (strncmp(tag, "s", 1) == 0)
		return ART_TAG_VERB;
	else if (strncmp(tag, "is", 2) == 0)
		return ART_TAG_VERB_IS;
	else
		return ART_TAG_NONE;
}

/**
 * Print an artifact activation message.
 *
 * In order to support randarts, with scrambled names, we re-write
 * the message to replace instances of {name} with the artifact name
 * and instances of {kind} with the type of object.
 *
 * This code deals with plural and singular forms of verbs correctly
 * when encountering {s}, though in fact both names and kinds are
 * always singular in the current code (gloves are "Set of" and boots
 * are "Pair of")
 */
static void activation_message(struct object *obj)
{
	char buf[1024] = "\0";
	const char *next;
	const char *s;
	const char *tag;
	const char *in_cursor;
	size_t end = 0;

	/* See if we have a message */
	if (!obj->activation) return;
	if (!obj->activation->message) return;
	in_cursor = obj->activation->message;

	next = strchr(in_cursor, '{');
	while (next) {
		/* Copy the text leading up to this { */
		strnfcat(buf, 1024, &end, "%.*s", next - in_cursor, in_cursor); 

		s = next + 1;
		while (*s && isalpha((unsigned char) *s)) s++;

		/* Valid tag */
		if (*s == '}') {
			/* Start the tag after the { */
			tag = next + 1;
			in_cursor = s + 1;

			switch(art_tag_lookup(tag)) {
			case ART_TAG_NAME:
				end += object_desc(buf, 1024, obj, ODESC_PREFIX | ODESC_BASE); 
				break;
			case ART_TAG_KIND:
				object_kind_name(&buf[end], 1024-end, obj->kind, TRUE);
				end += strlen(&buf[end]);
				break;
			case ART_TAG_VERB:
				strnfcat(buf, 1024, &end, "s");
				break;
			case ART_TAG_VERB_IS:
				if((end > 2) && (buf[end-2] == 's'))
					strnfcat(buf, 1024, &end, "are");
				else
					strnfcat(buf, 1024, &end, "is");
			default:
				break;
			}
		} else
			/* An invalid tag, skip it */
			in_cursor = next + 1;

		next = strchr(in_cursor, '{');
	}
	strnfcat(buf, 1024, &end, in_cursor);

	msg("%s", buf);
}


/**
 * Unknown item hook for get_item()
 */
static bool item_tester_unknown(const struct object *obj)
{
	return object_is_known(obj) ? FALSE : TRUE;
}

/**
 * Return TRUE if there are any objects available to identify (whether on
 * floor or in gear)
 */
static bool spell_identify_unknown_available(void)
{
	int floor_max = z_info->floor_size;
	struct object **floor_list = mem_zalloc(floor_max * sizeof(*floor_list));
	int floor_num;
	struct object *obj;
	bool unidentified_gear = FALSE;

	floor_num = scan_floor(floor_list, floor_max, player->py, player->px, 0x0B,
						   item_tester_unknown);

	for (obj = player->gear; obj; obj = obj->next) {
		if (object_test(item_tester_unknown, obj)) {
			unidentified_gear = TRUE;
			break;
		}
	}

	return unidentified_gear || floor_num > 0;
}



/**
 * ------------------------------------------------------------------------
 * Inscriptions
 * ------------------------------------------------------------------------
 */

/**
 * Remove inscription
 */
void do_cmd_uninscribe(struct command *cmd)
{
	struct object *obj;

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Uninscribe which item?",
			/* Error  */ "You have nothing you can uninscribe.",
			/* Filter */ obj_has_inscrip,
			/* Choice */ USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR) != CMD_OK)
		return;

	obj->note = 0;
	msg("Inscription removed.");

	player->upkeep->notice |= (PN_COMBINE | PN_IGNORE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
}

/**
 * Add inscription
 */
void do_cmd_inscribe(struct command *cmd)
{
	struct object *obj;
	const char *str;

	char prompt[1024];
	char o_name[80];

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Inscribe which item?",
			/* Error  */ "You have nothing to inscribe.",
			/* Filter */ NULL,
			/* Choice */ USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR | IS_HARMLESS) != CMD_OK)
		return;

	/* Form prompt */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
	strnfmt(prompt, sizeof prompt, "Inscribing %s.", o_name);

	if (cmd_get_string(cmd, "inscription", &str,
			quark_str(obj->note) /* Default */,
			prompt, "Inscribe with what? ") != CMD_OK)
		return;

	obj->note = quark_add(str);

	player->upkeep->notice |= (PN_COMBINE | PN_IGNORE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
}


/**
 * Autoinscribe all appropriate objects
 */
void do_cmd_autoinscribe(struct command *cmd)
{
	int py = player->py;
	int px = player->px;
	struct object *obj;

	/* Autoinscribe each object on the floor beneath the player */
	for (obj = square_object(cave, py, px); obj; obj = obj->next)
		apply_autoinscription(obj);

	/* Autoinscribe each object in the inventory */
	for (obj = player->gear; obj; obj = obj->next)
		apply_autoinscription(obj);

	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
}


/**
 * ------------------------------------------------------------------------
 * Taking off/putting on
 * ------------------------------------------------------------------------
 */

/**
 * Take off an item
 */
void do_cmd_takeoff(struct command *cmd)
{
	struct object *obj;

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Take off or unwield which item?",
			/* Error  */ "You have nothing to take off or unwield.",
			/* Filter */ obj_can_takeoff,
			/* Choice */ USE_EQUIP) != CMD_OK)
		return;

	inven_takeoff(obj);
	pack_overflow(obj);
	player->upkeep->energy_use = z_info->move_energy / 2;
}


/**
 * Wield or wear a single item from the pack or floor
 */
void wield_item(struct object *obj, int slot)
{
	struct object *wielded, *old = player->body.slots[slot].obj;

	const char *fmt;
	char o_name[80];

	/* Increase equipment counter if empty slot */
	if (old == NULL)
		player->upkeep->equip_cnt++;

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Split off a new object if necessary */
	if (obj->number > 1) {
		/* Split off a new single object */
		wielded = object_split(obj, 1);

		/* If it's a gear object, give the split item a list entry */
		if (pile_contains(player->gear, obj)) {
			wielded->next = obj->next;
			obj->next = wielded;
			wielded->prev = obj;
			if (wielded->next)
				(wielded->next)->prev = wielded;
		}
	} else
		wielded = obj;

	/* Carry floor items, don't allow combining */
	if (square_holds_object(cave, player->py, player->px, wielded)) {
		square_excise_object(cave, player->py, player->px, wielded);
		inven_carry(player, wielded, FALSE, FALSE);
	}

	/* Wear the new stuff */
	player->body.slots[slot].obj = wielded;

	/* Do any ID-on-wield */
	object_notice_on_wield(wielded);

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

	/* Cursed! */
	if (cursed_p(wielded->flags)) {
		/* Warn the player */
		msgt(MSG_CURSED, "Oops! It feels deathly cold!");

		/* Sense the object */
		object_notice_curses(wielded);
	}

	/* See if we have to overflow the pack */
	pack_overflow(old);

	/* Recalculate bonuses, torch, mana, gear */
	player->upkeep->notice |= (PN_IGNORE | PN_COMBINE);
	player->upkeep->update |= (PU_BONUS | PU_INVEN);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
}


/**
 * Wield or wear an item
 */
void do_cmd_wield(struct command *cmd)
{
	struct object *equip_obj;
	char o_name[80];

	unsigned n;

	int slot;
	struct object *obj;

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Wear or wield which item?",
			/* Error  */ "You have nothing to wear or wield.",
			/* Filter */ obj_can_wear,
			/* Choice */ USE_INVEN | USE_FLOOR) != CMD_OK)
		return;

	/* Get the slot the object wants to go in, and the item currently there */
	slot = wield_slot(obj);
	equip_obj = slot_object(player, slot);

	/* If the slot is open, wield and be done */
	if (!equip_obj) {
		wield_item(obj, slot);
		return;
	}

	/* Usually if the slot is taken we'll just replace the item in the slot,
	 * but for rings we need to ask the user which slot they actually
	 * want to replace */
	if (tval_is_ring(obj)) {
		if (cmd_get_item(cmd, "replace", &equip_obj,
						 /* Prompt */ "Replace which ring? ",
						 /* Error  */ "Error in do_cmd_wield(), please report.",
						 /* Filter */ tval_is_ring,
						 /* Choice */ USE_EQUIP) != CMD_OK)
			return;

		/* Change slot if necessary */
		slot = equipped_item_slot(player->body, equip_obj);
	}

	/* Prevent wielding into a cursed slot */
	if (cursed_p(equip_obj->flags)) {
		object_desc(o_name, sizeof(o_name), equip_obj, ODESC_BASE);
		msg("The %s you are %s appears to be cursed.", o_name,
			equip_describe(player, slot));
		return;
	}

	/* "!t" checks for taking off */
	n = check_for_inscrip(equip_obj, "!t");
	while (n--) {
		/* Prompt */
		object_desc(o_name, sizeof(o_name), equip_obj,
					ODESC_PREFIX | ODESC_FULL);
		
		/* Forget it */
		if (!get_check(format("Really take off %s? ", o_name))) return;
	}

	wield_item(obj, slot);
}

/**
 * Drop an item
 */
void do_cmd_drop(struct command *cmd)
{
	int amt;
	struct object *obj;

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Drop which item?",
			/* Error  */ "You have nothing to drop.",
			/* Filter */ NULL,
			/* Choice */ USE_EQUIP | USE_INVEN | USE_QUIVER) != CMD_OK)
		return;

	if (cmd_get_quantity(cmd, "quantity", &amt, obj->number) != CMD_OK)
		return;

	/* Hack -- Cannot remove cursed items */
	if (object_is_equipped(player->body, obj) && cursed_p(obj->flags)) {
		msg("Hmmm, it seems to be cursed.");
		return;
	}

	inven_drop(obj, amt);
	player->upkeep->energy_use = z_info->move_energy / 2;
}

/**
 * ------------------------------------------------------------------------
 * Using items the traditional way
 * ------------------------------------------------------------------------
 */

enum use {
	USE_TIMEOUT,
	USE_CHARGE,
	USE_SINGLE
};

/**
 * Use an object the right way.
 */
static void use_aux(struct command *cmd, struct object *obj, enum use use,
					int snd)
{
	struct effect *effect = object_effect(obj);
	bool ident = FALSE, used = FALSE;
	bool was_aware;
	int dir = 5;
	int px = player->px, py = player->py;
	enum use;
	struct trap_kind *rune = lookup_trap("glyph of warding");

	/* Get arguments */
	assert(cmd_get_arg_item(cmd, "item", &obj) == CMD_OK);

	if (obj_needs_aim(obj)) {
		if (cmd_get_target(cmd, "target", &dir) != CMD_OK)
			return;

		player_confuse_dir(player, &dir, FALSE);
	}

	was_aware = object_flavor_is_aware(obj);

	/* track the object used */
	track_object(player->upkeep, obj);

	/* Verify effect */
	assert(effect);

	/* Check for unknown objects to prevent wasted player turns. */
	if (effect->index == EF_IDENTIFY &&
		!spell_identify_unknown_available()) {
		msg("You have nothing to identify.");
		return;
	}

	/* Check for use if necessary, and execute the effect */
	if ((use != USE_CHARGE && use != USE_TIMEOUT) || check_devices(obj)) {
		int beam = beam_chance(obj->tval);
		int boost, level;

		/* Get the level */
		if (obj->artifact)
			level = obj->artifact->level;
		else if (obj->ego)
			level = obj->ego->level;
		else
			level = obj->kind->level;

		/* Sound and/or message */
		if (obj->activation) {
			msgt(snd, "You activate it.");
			activation_message(obj);
		} else if (obj->kind->effect_msg) {
			msgt(snd, obj->kind->effect_msg);
		} else {
			/* Make a noise! */
			sound(snd);
		}

		/* A bit of a hack to make ID work better.
			-- Check for "obvious" effects beforehand. */
		if (effect->index == EF_IDENTIFY) object_flavor_aware(obj);

		/* Boost damage effects if skill > difficulty */
		boost = MAX(player->state.skills[SKILL_DEVICE] - level, 0);

		/* Do effect */
		used = effect_do(effect, &ident, was_aware, dir, beam, boost);

		/* Quit if the item wasn't used and no knowledge was gained */
		if (!used && (was_aware || !ident)) return;
	}

	/* If the item is a null pointer or has been wiped, be done now */
	if (!obj) return;

	if (ident) object_notice_effect(obj);

	/* Use the turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Mark as tried and redisplay */
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP | PR_OBJECT);

	/*
	 * If the player becomes aware of the item's function, then mark it as
	 * aware and reward the player with some experience.  Otherwise, mark
	 * it as "tried".
	 */
	if (ident && !was_aware) {
		/* Object level */
		int lev = obj->kind->level;

		object_flavor_aware(obj);
		if (tval_is_rod(obj)) object_notice_everything(obj);
		player_exp_gain(player, (lev + (player->lev / 2)) / player->lev);
		player->upkeep->notice |= PN_IGNORE;
	} else if (used) {
		object_flavor_tried(obj);
	}

	/* If there are no more of the item left, then we're done. */
	if (!obj->number) return;

	/* Chargeables act differently to single-used items when not used up */
	if (used && use == USE_CHARGE) {
		/* Use a single charge */
		obj->pval--;

		/* Describe charges */
		if (object_is_carried(player, obj))
			inven_item_charges(obj);
		else
			floor_item_charges(obj);
	} else if (used && use == USE_TIMEOUT) {
		obj->timeout += randcalc(obj->time, 0, RANDOMISE);
	} else if (used && use == USE_SINGLE) {
		struct object *used_obj;

		/* Destroy an item in the pack */
		if (object_is_carried(player, obj))
			used_obj = gear_object_for_use(obj, 1, TRUE);
		else
			/* Destroy an item on the floor */
			used_obj = floor_object_for_use(obj, 1, TRUE);
		object_delete(used_obj);
	}

	/* Hack to make Glyph of Warding work properly */
	if (square_trap_specific(cave, py, px, rune->tidx)) {
		/* Push objects off the grid */
		if (square_object(cave, py, px))
			push_object(py, px);
	}
}


/**
 * Read a scroll
 */
void do_cmd_read_scroll(struct command *cmd)
{
	struct object *obj;

	/* Check player can use scroll */
	if (!player_can_read(player, TRUE))
		return;

	/* Get the scroll */
	if (cmd_get_item(cmd, "item", &obj,
			"Read which scroll? ",
			"You have no scrolls to read.",
			tval_is_scroll,
			USE_INVEN | USE_FLOOR) != CMD_OK) return;

	use_aux(cmd, obj, USE_SINGLE, MSG_GENERIC);
}

/**
 * Use a staff 
 */
void do_cmd_use_staff(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Use which staff? ",
			"You have no staves to use.",
			tval_is_staff,
			USE_INVEN | USE_FLOOR | SHOW_FAIL) != CMD_OK) return;

	if (!obj_has_charges(obj)) {
		msg("That staff has no charges.");
		return;
	}

	use_aux(cmd, obj, USE_CHARGE, MSG_USE_STAFF);
}

/**
 * Aim a wand 
 */
void do_cmd_aim_wand(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Aim which wand? ",
			"You have no wands to aim.",
			tval_is_wand,
			USE_INVEN | USE_FLOOR | SHOW_FAIL) != CMD_OK) return;

	if (!obj_has_charges(obj)) {
		msg("That wand has no charges.");
		return;
	}

	use_aux(cmd, obj, USE_CHARGE, MSG_ZAP_ROD);
}

/**
 * Zap a rod 
 */
void do_cmd_zap_rod(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Zap which rod? ",
			"You have no rods to zap.",
			tval_is_rod,
			USE_INVEN | USE_FLOOR | SHOW_FAIL) != CMD_OK) return;

	if (!obj_can_zap(obj)) {
		msg("That rod is still charging.");
		return;
	}

	use_aux(cmd, obj, USE_TIMEOUT, MSG_ZAP_ROD);
}

/**
 * Activate an object 
 */
void do_cmd_activate(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Activate which item? ",
			"You have no items to activate.",
			obj_is_activatable,
			USE_EQUIP | SHOW_FAIL) != CMD_OK) return;

	if (!obj_can_activate(obj)) {
		msg("That item is still charging.");
		return;
	}

	use_aux(cmd, obj, USE_TIMEOUT, MSG_ACT_ARTIFACT);
}

/**
 * Eat some food 
 */
void do_cmd_eat_food(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Eat which food? ",
			"You have no food to eat.",
			tval_is_edible,
			USE_INVEN | USE_FLOOR) != CMD_OK) return;

	use_aux(cmd, obj, USE_SINGLE, MSG_EAT);
}

/**
 * Quaff a potion 
 */
void do_cmd_quaff_potion(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Quaff which potion? ",
			"You have no potions from which to quaff.",
			tval_is_potion,
			USE_INVEN | USE_FLOOR) != CMD_OK) return;

	use_aux(cmd, obj, USE_SINGLE, MSG_QUAFF);
}

/**
 * Use any usable item
 */
void do_cmd_use(struct command *cmd)
{
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Use which item? ",
			"You have no items to use.",
			obj_is_useable,
			USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR | SHOW_FAIL | QUIVER_TAGS | SHOW_FAIL) != CMD_OK)
		return;

	if (tval_is_ammo(obj))				do_cmd_fire(cmd);
	else if (tval_is_potion(obj))		do_cmd_quaff_potion(cmd);
	else if (tval_is_food(obj))			do_cmd_eat_food(cmd);
	else if (tval_is_rod(obj))			do_cmd_zap_rod(cmd);
	else if (tval_is_wand(obj))			do_cmd_aim_wand(cmd);
	else if (tval_is_staff(obj))		do_cmd_use_staff(cmd);
	else if (tval_is_scroll(obj))		do_cmd_read_scroll(cmd);
	else if (obj_can_refill(obj))		do_cmd_refill(cmd);
	else if (obj_is_activatable(obj))	do_cmd_activate(cmd);
	else
		msg("The item cannot be used at the moment");
}


/**
 * ------------------------------------------------------------------------
 * Refuelling
 * ------------------------------------------------------------------------
 */

static void refill_lamp(struct object *lamp, struct object *obj)
{
	/* Refuel */
	lamp->timeout += obj->timeout ? obj->timeout : obj->pval;

	/* Message */
	msg("You fuel your lamp.");

	/* Comment */
	if (lamp->timeout >= z_info->fuel_lamp) {
		lamp->timeout = z_info->fuel_lamp;
		msg("Your lamp is full.");
	}

	/* Refilled from a lantern */
	if (of_has(obj->flags, OF_TAKES_FUEL)) {
		/* Unstack if necessary */
		if (obj->number > 1) {
			/* Obtain a local object, split */
			struct object *used = object_split(obj, 1);

			/* Remove fuel */
			used->timeout = 0;

			/* Carry or drop */
			if (object_is_carried(player, obj))
				inven_carry(player, used, TRUE, TRUE);
			else
				drop_near(cave, used, 0, player->py, player->px, FALSE);
		} else
			/* Empty a single lantern */
			obj->timeout = 0;

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Redraw stuff */
		player->upkeep->redraw |= (PR_INVEN);
	} else { /* Refilled from a flask */
		struct object *used;

		/* Decrease the item from the pack or the floor */
		if (object_is_carried(player, obj))
			used = gear_object_for_use(obj, 1, TRUE);
		else
			used = floor_object_for_use(obj, 1, TRUE);
		object_delete(used);
	}

	/* Recalculate torch */
	player->upkeep->update |= (PU_TORCH);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_EQUIP);
}


void do_cmd_refill(struct command *cmd)
{
	struct object *light = equipped_item_by_slot_name(player, "light");
	struct object *obj;

	/* Get an item */
	if (cmd_get_item(cmd, "item", &obj,
			"Refuel with with fuel source? ",
			"You have nothing you can refuel with.",
			obj_can_refill,
			USE_INVEN | USE_FLOOR) != CMD_OK) return;

	/* Check what we're wielding. */
	if (!tval_is_light(light)) {
		msg("You are not wielding a light.");
		return;
	} else if (of_has(light->flags, OF_NO_FUEL)) {
		msg("Your light cannot be refilled.");
		return;
	} else if (of_has(light->flags, OF_TAKES_FUEL)) {
		refill_lamp(light, obj);
	} else {
		msg("Your light cannot be refilled.");
		return;
	}

	player->upkeep->energy_use = z_info->move_energy / 2;
}



/**
 * ------------------------------------------------------------------------
 * Spell casting
 * ------------------------------------------------------------------------
 */

/**
 * Cast a spell from a book
 */
void do_cmd_cast(struct command *cmd)
{
	int spell, dir;

	const char *verb = player->class->magic.spell_realm->verb;
	const char *noun = player->class->magic.spell_realm->spell_noun;
	const class_spell *s_ptr;

	/* Check the player can cast spells at all */
	if (!player_can_cast(player, TRUE))
		return;

	/* Get arguments */
	if (cmd_get_spell(cmd, "spell", &spell,
			/* Verb */   "cast",
			/* Book */   obj_can_cast_from,
			/* Error */  "There are no spells you can cast.",
			/* Filter */ spell_okay_to_cast) != CMD_OK)
		return;

	if (spell_needs_aim(spell)) {
		if (cmd_get_target(cmd, "target", &dir) == CMD_OK)
			player_confuse_dir(player, &dir, FALSE);
		else
			return;
	}

	/* Get the spell */
	s_ptr = spell_by_index(spell);

	/* Check for unknown objects to prevent wasted player turns. */
	if (spell_is_identify(spell) && !spell_identify_unknown_available()) {
		msg("You have nothing to identify.");
		return;
	}

	/* Verify "dangerous" spells */
	if (s_ptr->smana > player->csp) {
		/* Warning */
		msg("You do not have enough mana to %s this %s.", verb, noun);

		/* Flush input */
		event_signal(EVENT_INPUT_FLUSH);

		/* Verify */
		if (!get_check("Attempt it anyway? ")) return;
	}

	/* Cast a spell */
	if (spell_cast(spell, dir))
		player->upkeep->energy_use = z_info->move_energy;
}


/**
 * Gain a specific spell, specified by spell number (for mages).
 */
void do_cmd_study_spell(struct command *cmd)
{
	int spell;

	/* Check the player can study at all atm */
	if (!player_can_study(player, TRUE))
		return;

	if (cmd_get_spell(cmd, "spell", &spell,
			/* Verb */   "study",
			/* Book */   obj_can_study,
			/* Error  */ "You cannot learn any new spells from the books you have.",
			/* Filter */ spell_okay_to_study) != CMD_OK)
		return;

	spell_learn(spell);
	player->upkeep->energy_use = z_info->move_energy;
}

/**
 * Gain a random spell from the given book (for priests)
 */
void do_cmd_study_book(struct command *cmd)
{
	struct object *book_obj;
	const class_book *book;
	int spell = -1;
	class_spell *sp;
	int i, k = 0;

	const char *p = player->class->magic.spell_realm->spell_noun;

	if (cmd_get_item(cmd, "item", &book_obj,
			/* Prompt */ "Study which book? ",
			/* Error  */ "You cannot learn any new spells from the books you have.",
			/* Filter */ obj_can_study,
			/* Choice */ USE_INVEN | USE_FLOOR) != CMD_OK)
		return;

	book = object_to_book(book_obj);
	track_object(player->upkeep, book_obj);
	handle_stuff(player->upkeep);

	/* Check the player can study at all atm */
	if (!player_can_study(player, TRUE))
		return;

	for (i = 0; i < book->num_spells; i++) {
		sp = &book->spells[i];
		if (!spell_okay_to_study(sp->sidx))
			continue;
		if ((++k > 1) && (randint0(k) != 0))
			continue;
		spell = sp->sidx;
	}

	if (spell < 0)
		msg("You cannot learn any %ss in that book.", p);
	else {
		spell_learn(spell);
		player->upkeep->energy_use = z_info->move_energy;
	}
}

/**
 * Choose the way to study.  Choose life.  Choose a career.  Choose family.
 * Choose a fucking big monster, choose orc shamans, kobolds, dark elven
 * druids, and Mim, Betrayer of Turin.
 */
void do_cmd_study(struct command *cmd)
{
	if (player_has(PF_CHOOSE_SPELLS))
		do_cmd_study_spell(cmd);
	else
		do_cmd_study_book(cmd);
}
