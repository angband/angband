/**
 * \file obj-knowledge.c
 * \brief Object knowledge
 *
 * Copyright (c) 2016 Nick McConnell
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
#include "object.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-history.h"
#include "store.h"


/**
 * ------------------------------------------------------------------------
 * Functions used for applying knowledge to objects
 * ------------------------------------------------------------------------ */
/**
 * Check if a brand is known to the player
 */
bool player_knows_brand(struct player *p, struct brand *b)
{
	struct brand *b_check = p->obj_k->brands;
	while (b_check) {
		/* Same element is all we need */
		if (b_check->element == b->element) return true;
		b_check = b_check->next;
	}

	return false;
}

/**
 * Check if a slay is known to the player
 */
bool player_knows_slay(struct player *p, struct slay *s)
{
	struct slay *s_check = p->obj_k->slays;
	while (s_check) {
		/* Name and race flag need to be the same */
		if (streq(s_check->name, s->name) &&
			(s_check->race_flag == s->race_flag)) return true;
		s_check = s_check->next;
	}

	return false;
}

/**
 * Check if an ego item type is known to the player
 */
bool player_knows_ego(struct player *p, struct ego_item *ego)
{
	int i;
	struct brand *b;
	struct slay *s;

	if (!ego) return false;

	/* All flags known */
	if (!of_is_subset(p->obj_k->flags, ego->flags)) return false;

	/* All modifiers known */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (randcalc(ego->modifiers[i], MAX_RAND_DEPTH, MAXIMISE) &&
			!p->obj_k->modifiers[i])
			return false;

	/* All elements known */
	for (i = 0; i < ELEM_MAX; i++)
		if (ego->el_info[i].res_level && !p->obj_k->el_info[i].res_level)
			return false;

	/* All brands known */
	for (b = ego->brands; b; b = b->next)
		if (!player_knows_brand(p, b)) return false;

	/* All slays known */
	for (s = ego->slays; s; s = s->next)
		if (!player_knows_slay(p, s)) return false;

	return true;
}

/**
 * Check if an object is fully known to the player
 */
bool object_fully_known(const struct object *obj)
{
	int i;

	/* No known object */
	if (!obj->known) return false;

	/* Not all flags known */
	if (!of_is_equal(obj->flags, obj->known->flags)) return false;

	/* Not all modifiers known */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->modifiers[i] != obj->known->modifiers[i])
			return false;

	/* Not all elements known */
	for (i = 0; i < ELEM_MAX; i++)
		if (obj->el_info[i].res_level != obj->known->el_info[i].res_level)
			return false;

	/* Not all brands known */
	if (!brands_are_equal(obj->brands, obj->known->brands))
		return false;

	/* Not all slays known */
	if (!slays_are_equal(obj->slays, obj->known->slays))
		return false;

	/* Effect not known */
	if (obj->effect != obj->known->effect) return false;

	return true;
}

/**
 * Transfer player object knowledge to an object
 */
void player_know_object(struct player *p, struct object *obj)
{
	int i;
	struct brand *b, *b_known;
	struct slay *s, *s_known;

	/* Unseen or only sensed objects don't get any ID */
	if (!obj) return;
	if (!obj->known) return;
	if (obj->kind != obj->known->kind) return;

	/* Set object flags */
	(void) of_inter(obj->known->flags, p->obj_k->flags);

	/* Set modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (p->obj_k->modifiers[i])
			obj->known->modifiers[i] = obj->modifiers[i];

	/* Set elements */
	for (i = 0; i < ELEM_MAX; i++)
		if (p->obj_k->el_info[i].res_level == 1) {
			obj->known->el_info[i].res_level = obj->el_info[i].res_level;
			obj->known->el_info[i].flags = obj->el_info[i].flags;
		}

	/* Reset brands */
	free_brand(obj->known->brands);
	obj->known->brands = NULL;
	for (b = obj->brands; b; b = b->next) {
		if (player_knows_brand(p, b)) {
			/* Copy */
			struct brand *new_b = mem_zalloc(sizeof *new_b);
			new_b->name = string_make(b->name);
			new_b->element = b->element;
			new_b->multiplier = b->multiplier;

			/* Attach the new brand */
			if (obj->known->brands) {
				b_known->next = new_b;
				b_known = b_known->next;
			} else {
				obj->known->brands = new_b;
				b_known = obj->known->brands;
			}
		}
	}

	/* Reset slays */
	free_slay(obj->known->slays);
	obj->known->slays = NULL;
	for (s = obj->slays; s; s = s->next) {
		if (player_knows_slay(p, s)) {
			/* Copy */
			struct slay *new_s = mem_zalloc(sizeof *new_s);
			new_s->name = string_make(s->name);
			new_s->race_flag = s->race_flag;
			new_s->multiplier = s->multiplier;

			/* Attach the new slay */
			if (obj->known->slays) {
				s_known->next = new_s;
				s_known = s_known->next;
			} else {
				obj->known->slays = new_s;
				s_known = obj->known->slays;
			}
		}
	}

	/* Set combat details */
	obj->known->ac = p->obj_k->ac * obj->ac;
	obj->known->to_a = p->obj_k->to_a * obj->to_a;
	obj->known->to_h = p->obj_k->to_h * obj->to_h;
	obj->known->to_d = p->obj_k->to_d * obj->to_d;
	obj->known->dd = p->obj_k->dd * obj->dd;
	obj->known->ds = p->obj_k->ds * obj->ds;

	/* Set ego type, jewellery type if known */
	if (player_knows_ego(p, obj->ego))
		obj->known->ego = obj->ego;
	if (object_fully_known(obj) && tval_is_jewelry(obj))
		object_flavor_aware(obj);
}

/**
 * Propagate player knowledge of objects to all objects
 */
static void update_player_object_knowledge(struct player *p)
{
	int i;
	struct object *obj;

	/* Level objects */
	for (i = 0; i < cave->obj_max; i++)
		player_know_object(p, cave->objects[i]);

	/* Player objects */
	for (obj = player->gear; obj; obj = obj->next)
		player_know_object(p, obj);

	/* Store objects */
	for (i = 0; i < MAX_STORES; i++) {
		struct store *s = &stores[i];
		for (obj = s->stock; obj; obj = obj->next)
			player_know_object(p, obj);
	}
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/**
 * ------------------------------------------------------------------------
 * Functions for increasing player knowledge
 * ------------------------------------------------------------------------ */
/**
 * Learn a single flag
 */
void player_learn_flag(struct player *p, int flag)
{
	/* If the flag was unknown, set it */
	if (of_on(p->obj_k->flags, flag))
		update_player_object_knowledge(p);
}

/**
 * Learn a single modifier
 */
void player_learn_mod(struct player *p, int mod)
{
	/* If the modifier was unknown, set it */
	if (p->obj_k->modifiers[mod] == 0) {
		p->obj_k->modifiers[mod] = 1;
		//mod_message(mod);
		update_player_object_knowledge(p);
	}
}

/**
 * Learn a single element
 */
void player_learn_element(struct player *p, int element)
{
	/* If the element was unknown, set it */
	if (p->obj_k->el_info[element].res_level == 0) {
		p->obj_k->el_info[element].res_level = 1;
		update_player_object_knowledge(p);
	}
}

/**
 * Learn a single elemental brand
 */
void player_learn_brand(struct player *p, struct brand *b)
{
	/* If the brand was unknown, add it to known brands */
	if (!player_knows_brand(p, b)) {
		/* Copy the name and element */
		struct brand *new_b = mem_zalloc(sizeof *new_b);
		new_b->name = string_make(b->name);
		new_b->element = b->element;

		/* Attach the new brand */
		new_b->next = p->obj_k->brands;
		p->obj_k->brands = new_b;

		update_player_object_knowledge(p);
	}
}

/**
 * Learn a single slay
 */
void player_learn_slay(struct player *p, struct slay *s)
{
	/* If the slay was unknown, add it to known slays */
	if (!player_knows_slay(p, s)) {
		/* Copy the name and race flag */
		struct slay *new_s = mem_zalloc(sizeof *new_s);
		new_s->name = string_make(s->name);
		new_s->race_flag = s->race_flag;

		/* Attach the new brand */
		new_s->next = p->obj_k->slays;
		p->obj_k->slays = new_s;

		update_player_object_knowledge(p);
	}
}

/**
 * Learn armour class
 */
void player_learn_ac(struct player *p)
{
	if (p->obj_k->ac) return;
	p->obj_k->ac = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn to-armour bonus
 */
void player_learn_to_a(struct player *p)
{
	if (p->obj_k->to_a) return;
	p->obj_k->to_a = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn to-hit bonus
 */
void player_learn_to_h(struct player *p)
{
	if (p->obj_k->to_h) return;
	p->obj_k->to_h = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn to-damage bonus
 */
void player_learn_to_d(struct player *p)
{
	if (p->obj_k->to_d) return;
	p->obj_k->to_d = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn damage dice
 */
void player_learn_dice(struct player *p)
{
	if (p->obj_k->dd && p->obj_k->ds) return;
	p->obj_k->dd = 1;
	p->obj_k->ds = 1;
	update_player_object_knowledge(p);
}

/**
 * ------------------------------------------------------------------------
 * Functions for learning about equipment properties
 * ------------------------------------------------------------------------ */
/**
 * Learn things which happen on defending.
 *
 * \param p is the player
 */
void equip_learn_on_defend(struct player *p)
{
	int i;

	if (p->obj_k->ac && p->obj_k->to_a) return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (obj) {
			assert(obj->known);
			if (obj->ac) player_learn_ac(p);
			if (obj->to_a) player_learn_to_a(p);
			if (p->obj_k->ac && p->obj_k->to_a) return;
		}
	}
}

/**
 * Learn attack bonus on making a ranged attack.
 * Can be applied to the missile or the missile launcher
 *
 * \param p is the player
 */
void missile_learn_on_ranged_attack(struct player *p, struct object *obj)
{
	if (p->obj_k->to_h && p->obj_k->to_d && p->obj_k->dd && p->obj_k->ds)
		return;

	assert(obj->known);
	player_learn_dice(player);
	if (obj->to_h) player_learn_to_h(p);
	if (obj->to_d) player_learn_to_d(p);
}

/**
 * Learn to-hit bonus on making a ranged attack.
 * Does not apply to weapon or bow
 *
 * \param p is the player
 */
void equip_learn_on_ranged_attack(struct player *p)
{
	int i;

	if (p->obj_k->to_h) return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (i == slot_by_name(p, "weapon")) continue;
		if (i == slot_by_name(p, "shooting")) continue;
		if (obj) {
			assert(obj->known);
			player_learn_dice(player);
			if (obj->to_h) player_learn_to_h(p);
			if (p->obj_k->to_h) return;
		}
	}

	return;
}


/**
 * Learn things which happen on making a melee attack.
 * Does not apply to bow
 *
 * \param p is the player
 */
void equip_learn_on_melee_attack(struct player *p)
{
	int i;

	if (p->obj_k->to_h && p->obj_k->to_d && p->obj_k->dd && p->obj_k->ds)
		return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (i == slot_by_name(p, "shooting")) continue;
		if (obj) {
			assert(obj->known);
			player_learn_dice(player);
			if (obj->to_h) player_learn_to_h(p);
			if (obj->to_d) player_learn_to_d(p);
			if (p->obj_k->to_h && p->obj_k->to_d) return;
		}
	}

	return;
}


/**
 * Learn a given object flag on wielded items.
 *
 * \param p is the player
 * \param flag is the flag to notice
 */
void equip_learn_flag(struct player *p, int flag)
{
	int i;

	/* No flag or already known */
	if (!flag) return;
	if (of_has(p->obj_k->flags, flag)) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (!obj) continue;
		assert(obj->known);

		/* Does the object have the flag? */
		if (of_has(obj->flags, flag)) {
			char o_name[80];
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

			/* Learn the flag */
			player_learn_flag(p, flag);

			/* Message */
			flag_message(flag, o_name);
			return;
		}
	}
}

/**
 * Learn the elemental resistance properties on wielded items.
 *
 * \param p is the player
 * \param element is the element to notice
 */
void equip_learn_element(struct player *p, int element)
{
	int i;

	/* Invalid element or element already known */
	if (element < 0 || element >= ELEM_MAX) return;
	if (p->obj_k->el_info[element].res_level == 1) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (!obj) continue;
		assert(obj->known);

		/* Does the object affect the player's resistance to the element? */
		if (obj->el_info[element].res_level != 0) {
			char o_name[80];
			object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

			/* Learn the element properties */
			player_learn_element(p, element);

			/* Message */
			msg("Your %s glows.", o_name);
			return;
		}
	}
}

/**
 * Learn things that would be noticed in time.
 */
void equip_learn_after_time(struct player *p)
{
	int flag;
	bitflag f[OF_SIZE], timed_mask[OF_SIZE];

	/* Get the timed flags */
	create_mask(timed_mask, true, OFID_TIMED, OFT_MAX);

	/* Get the unknown timed flags, and return if there are none */
	object_flags(p->obj_k, f);
	of_negate(f);
	of_inter(f, timed_mask);
	if (of_is_empty(f)) return;

	/* Attempt to learn every flag */
	for (flag = of_next(f, FLAG_START); flag != FLAG_END;
		 flag = of_next(f, flag + 1))
		player_learn_flag(p, flag);
}

/**
 * ------------------------------------------------------------------------
 * Functions for learning from the behaviour of indvidual objects
 * ------------------------------------------------------------------------ */
/**
 * Print a message when an object modifier is identified by use.
 *
 * \param mod is the modifier being noticed
 * \param name is the object name 
 */
void mod_message(struct object *obj, int mod)
{
	/* Special messages for individual properties */
	switch (mod) {
	case OBJ_MOD_STR:
		if (obj->modifiers[OBJ_MOD_STR] > 0)
			msg("You feel stronger!");
		else if (obj->modifiers[OBJ_MOD_STR] < 0)
			msg("You feel weaker!");
		break;
	case OBJ_MOD_INT:
		if (obj->modifiers[OBJ_MOD_INT] > 0)
			msg("You feel smarter!");
		else if (obj->modifiers[OBJ_MOD_INT] < 0)
			msg("You feel more stupid!");
		break;
	case OBJ_MOD_WIS:
		if (obj->modifiers[OBJ_MOD_WIS] > 0)
			msg("You feel wiser!");
		else if (obj->modifiers[OBJ_MOD_WIS] < 0)
			msg("You feel more naive!");
		break;
	case OBJ_MOD_DEX:
		if (obj->modifiers[OBJ_MOD_DEX] > 0)
			msg("You feel more dextrous!");
		else if (obj->modifiers[OBJ_MOD_DEX] < 0)
			msg("You feel clumsier!");
		break;
	case OBJ_MOD_CON:
		if (obj->modifiers[OBJ_MOD_CON] > 0)
			msg("You feel healthier!");
		else if (obj->modifiers[OBJ_MOD_CON] < 0)
			msg("You feel sicklier!");
		break;
	case OBJ_MOD_STEALTH:
		if (obj->modifiers[OBJ_MOD_STEALTH] > 0)
			msg("You feel stealthier.");
		else if (obj->modifiers[OBJ_MOD_STEALTH] < 0)
			msg("You feel noisier.");
		break;
	case OBJ_MOD_SPEED:
		if (obj->modifiers[OBJ_MOD_SPEED] > 0)
			msg("You feel strangely quick.");
		else if (obj->modifiers[OBJ_MOD_SPEED] < 0)
			msg("You feel strangely sluggish.");
		break;
	case OBJ_MOD_BLOWS:
		if (obj->modifiers[OBJ_MOD_BLOWS] > 0)
			msg("Your weapon tingles in your hands.");
		else if (obj->modifiers[OBJ_MOD_BLOWS] < 0)
			msg("Your weapon aches in your hands.");
		break;
	case OBJ_MOD_SHOTS:
		if (obj->modifiers[OBJ_MOD_SHOTS] > 0)
			msg("Your bow tingles in your hands.");
		else if (obj->modifiers[OBJ_MOD_SHOTS] < 0)
			msg("Your bow aches in your hands.");
		break;
	case OBJ_MOD_INFRA:
		msg("Your eyes tingle.");
		break;
	case OBJ_MOD_LIGHT:
		msg("It glows!");
		break;
	default:
		break;
	}
}

/**
 * Learn object properties that become obvious on wielding or wearing
 */
void object_learn_on_wield(struct player *p, struct object *obj)
{
	bitflag f[OF_SIZE], f2[OF_SIZE], obvious_mask[OF_SIZE];
	int i, flag;

	assert(obj->known);
	/* Always set the worn flag */
	obj->known->notice |= OBJ_NOTICE_WORN;

	/* Worn means tried (for flavored wearables) */
	object_flavor_tried(obj);

	/* Get the obvious object flags */
	create_mask(obvious_mask, true, OFID_WIELD, OFT_MAX);

	/* Special case FA, needed for mages wielding gloves */
	if (player_has(p, PF_CUMBER_GLOVE) && obj->tval == TV_GLOVES &&
		(obj->modifiers[OBJ_MOD_DEX] <= 0) && 
		!kf_has(obj->kind->kind_flags, KF_SPELLS_OK))
		of_on(obvious_mask, OF_FREE_ACT);

	/* Extract the flags */
	object_flags(obj, f);

	/* Find obvious flags - curses left for special message later */
	create_mask(f2, false, OFT_CURSE, OFT_MAX);
	of_diff(obvious_mask, f2);

	/* Learn about obvious, previously unknown flags */
	of_inter(f, obvious_mask);
	for (flag = of_next(f, FLAG_START); flag != FLAG_END;
		 flag = of_next(f, flag + 1)) {
		char o_name[80];
		if (of_has(p->obj_k->flags, flag)) continue;
		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

		/* Learn the flag */
		player_learn_flag(p, flag);

		/* Message */
		flag_message(flag, o_name);
	}

	/* Learn all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->modifiers[i] && !p->obj_k->modifiers[i]) {
			/* Learn the mod */
			player_learn_mod(p, i);

			/* Message */
			mod_message(obj, i);
		}

	/* Automatically sense artifacts upon wield, mark as assessed */
	obj->known->artifact = obj->artifact;
	obj->known->notice |= OBJ_NOTICE_ASSESSED;

	/* Note artifacts when found */
	if (obj->artifact)
		history_add_artifact(obj->artifact, true, true);
}

