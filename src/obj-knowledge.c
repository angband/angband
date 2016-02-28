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
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-calcs.h"
#include "player-history.h"
#include "store.h"

/**
 * Overview
 * ========
 * This file deals with the new "rune-based ID" system.  This system operates
 * as follows:
 * - struct player has an object struct attached to it (obj_k) which contains
 *   the player's knowledge of object properties (runes)
 * - whenever the player learns a rune, 
 *   - if it's an object flag, that flag is set in obj_k
 *   - if it's an integer value, that value in obj_k is set to 1
 *   - if it's element info, the res_level value is set to 1
 *   - if it's a brand, a brand is added to obj_k with the relevant element
 *   - if it's a slay, a slay is added to obj_k with the right race flag or name
 * - every object has a known version which is filled in with details as the
 *   player learns them
 * - whenever the player learns a rune, that knowledge is applied to the known
 *   version of every object that the player has picked up or walked over
 *   or seen in a shop
 */

/**
 * ------------------------------------------------------------------------
 * Object knowledge predicates
 * These functions tell how much the player knows about an object
 * ------------------------------------------------------------------------ */

/**
 * Check if a brand is known to the player
 *
 * \param p is the player
 * \param b is the brand
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
 *
 * \param p is the player
 * \param s is the slay
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
 *
 * \param p is the player
 * \param ego is the ego item type
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
 * Checks whether the player is aware of the object's effect when used
 *
 * \param obj is the object
 */
bool object_effect_is_known(const struct object *obj)
{
	if (obj->effect == obj->known->effect) return true;

	return false;
}

/**
 * Checks whether the object is known to be an artifact
 *
 * \param obj is the object
 */
bool object_is_known_artifact(const struct object *obj)
{
	if (!obj->known) return false;
	return obj->known->artifact ? true : false;
}

/**
 * Check if an object is fully known to the player
 *
 * \param obj is the object
 */
bool object_fully_known(const struct object *obj)
{
	int i;

	/* No known object */
	if (!obj->known) return false;

	/* Not all combat details known */
	if (obj->known->to_a != obj->to_a) return false;
	if (obj->known->to_h != obj->to_h) return false;
	if (obj->known->to_d != obj->to_d) return false;

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
	if (!object_effect_is_known(obj)) return false;

	return true;
}

/**
 * Checks whether the player knows whether an object has a given flag
 *
 * \param obj is the object
 */
bool object_flag_is_known(const struct object *obj, int flag)
{
	/* Object fully known means OK */
	if (object_fully_known(obj)) return true;

	/* Player knows the flag means OK */
	if (of_has(player->obj_k->flags, flag)) return true;

	return false;
}

/**
 * Checks whether the player knows the given element properties of an object
 *
 * \param obj is the object
 */
bool object_element_is_known(const struct object *obj, int element)
{
	if (element < 0 || element >= ELEM_MAX) return false;

	/* Object fully known means OK */
	if (object_fully_known(obj)) return true;

	/* Player knows the element means OK */
	if (player->obj_k->el_info[element].res_level) return true;

	return false;
}

/**
 * ------------------------------------------------------------------------
 * Object knowledge propagators
 * These functions transfer player knowledge to objects
 * ------------------------------------------------------------------------ */
/**
 * Sets the basic details on a known object
 */
void object_set_base_known(struct object *obj)
{
	assert(obj->known);
	obj->known->kind = obj->kind;
	obj->known->tval = obj->tval;
	obj->known->sval = obj->sval;
	obj->known->number = obj->number;

	/* Generic dice and ac */
	obj->known->dd = obj->kind->dd * player->obj_k->dd;
	obj->known->ds = obj->kind->ds * player->obj_k->ds;
	obj->known->ac = obj->kind->ac * player->obj_k->ac;

	/* Aware flavours get info now */
	if (obj->kind->flavor && obj->kind->aware) {
		obj->known->pval = obj->pval;
		obj->known->effect = obj->effect;
	}
}

/**
 * Transfer player object knowledge to an object
 *
 * \param p is the player
 * \param obj is the object
 */
void player_know_object(struct player *p, struct object *obj)
{
	int i, flag;
	struct brand *b, *b_known;
	struct slay *s, *s_known;

	/* Unseen or only sensed objects don't get any ID */
	if (!obj) return;
	if (!obj->known) return;
	if (obj->kind != obj->known->kind) return;

	/* Set object flags */
	for (flag = of_next(p->obj_k->flags, FLAG_START); flag != FLAG_END;
		 flag = of_next(p->obj_k->flags, flag + 1)) {
		if (of_has(obj->flags, flag))
			of_on(obj->known->flags, flag);
	}

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
 *
 * \param p is the player
 */
static void update_player_object_knowledge(struct player *p)
{
	int i;
	struct object *obj;

	/* Level objects */
	if (cave)
		for (i = 0; i < cave->obj_max; i++)
			player_know_object(p, cave->objects[i]);

	/* Player objects */
	for (obj = p->gear; obj; obj = obj->next)
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
 * Object knowledge learners
 * These functions are for increasing player knowledge of object properties
 * ------------------------------------------------------------------------ */
/**
 * Learn a single flag
 *
 * \param p is the player
 * \param flag is the object flag 
 */
void player_learn_flag(struct player *p, int flag)
{
	/* If the flag was unknown, set it */
	if (of_on(p->obj_k->flags, flag))
		update_player_object_knowledge(p);
}

/**
 * Learn a single modifier
 *
 * \param p is the player
 * \param mod is the modifier
 */
void player_learn_mod(struct player *p, int mod)
{
	/* If the modifier was unknown, set it */
	if (p->obj_k->modifiers[mod] == 0) {
		p->obj_k->modifiers[mod] = 1;
		update_player_object_knowledge(p);
	}
}

/**
 * Learn a single element
 *
 * \param p is the player
 * \param element is the element about which all info is being learnt
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
 *
 * \param p is the player
 * \param b is the brand being learnt (known for any multiplier)
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
 *
 * \param p is the player
 * \param s is the slay being learnt (known for any multiplier)
 */
void player_learn_slay(struct player *p, struct slay *s)
{
	/* If the slay was unknown, add it to known slays */
	if (!player_knows_slay(p, s)) {
		/* Copy the name and race flag */
		struct slay *new_s = mem_zalloc(sizeof *new_s);
		new_s->name = string_make(s->name);
		new_s->race_flag = s->race_flag;

		/* Attach the new slay */
		new_s->next = p->obj_k->slays;
		p->obj_k->slays = new_s;

		update_player_object_knowledge(p);
	}
}

/**
 * Learn armour class
 *
 * \param p is the player
 */
void player_learn_ac(struct player *p)
{
	if (p->obj_k->ac) return;
	p->obj_k->ac = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn to-armour bonus
 *
 * \param p is the player
 */
void player_learn_to_a(struct player *p)
{
	if (p->obj_k->to_a) return;
	p->obj_k->to_a = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn to-hit bonus
 *
 * \param p is the player
 */
void player_learn_to_h(struct player *p)
{
	if (p->obj_k->to_h) return;
	p->obj_k->to_h = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn to-damage bonus
 *
 * \param p is the player
 */
void player_learn_to_d(struct player *p)
{
	if (p->obj_k->to_d) return;
	p->obj_k->to_d = 1;
	update_player_object_knowledge(p);
}

/**
 * Learn damage dice
 *
 * \param p is the player
 */
void player_learn_dice(struct player *p)
{
	if (p->obj_k->dd && p->obj_k->ds) return;
	p->obj_k->dd = 1;
	p->obj_k->ds = 1;
	update_player_object_knowledge(p);
}

static const char *brand_names[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) b,
	#include "list-elements.h"
	#undef ELEM
	NULL
};

static const char *slay_names[] = {
	#define RF(a, b, c) b,
	#include "list-mon-race-flags.h"
	#undef RF
	NULL
};

/**
 * Learn absolutely everything
 *
 * \param p is the player
 */
void player_learn_everything(struct player *p)
{
	int i;
	struct monster_base *base;

	player_learn_ac(p);
	player_learn_to_a(p);
	player_learn_to_h(p);
	player_learn_to_d(p);
	player_learn_dice(p);
	for (i = 1; i < OF_MAX; i++)
		player_learn_flag(p, i);
	for (i = 0; i < OBJ_MOD_MAX; i++)
		player_learn_mod(p, i);
	for (i = 0; i < ELEM_MAX; i++)
		player_learn_element(p, i);

	/* Cover more brands and slays than exist */
	free_brand(p->obj_k->brands);
	p->obj_k->brands = NULL;
	for (i = 0; i < ELEM_MAX; i++) {
		struct brand *new_b = mem_zalloc(sizeof *new_b);
		new_b->name = string_make(brand_names[i]);
		new_b->element = i;
		new_b->next = p->obj_k->brands;
		p->obj_k->brands = new_b;
	}
	free_slay(p->obj_k->slays);
	p->obj_k->slays = NULL;
	for (i = 0; i < RF_MAX; i++) {
		struct slay *new_s = mem_zalloc(sizeof *new_s);
		new_s->name = string_make(slay_names[i]);
		new_s->race_flag = i;
		new_s->next = p->obj_k->slays;
		p->obj_k->slays = new_s;
	}
	for (base = rb_info; base; base = base->next) {
		struct slay *new_s = mem_zalloc(sizeof *new_s);
		new_s->name = string_make(base->name);
		new_s->next = p->obj_k->slays;
		p->obj_k->slays = new_s;
	}
	update_player_object_knowledge(p);
}

/**
 * ------------------------------------------------------------------------
 * Functions for learning about equipment properties
 * These functions are for gaining object knowledge from the behaviour of 
 * the player's equipment
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
 *
 * \param p is the player
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
 * \param obj is the object 
 * \param mod is the modifier being noticed
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
 *
 * \param p is the player
 * \param obj is the wielded object
 */
void object_learn_on_wield(struct player *p, struct object *obj)
{
	bitflag f[OF_SIZE], f2[OF_SIZE], obvious_mask[OF_SIZE];
	int i, flag;

	assert(obj->known);

	/* Check the worn flag */
	if (obj->known->notice & OBJ_NOTICE_WORN)
		return;
	else
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
		if (p->upkeep->playing) flag_message(flag, o_name);
	}

	/* Learn all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->modifiers[i] && !p->obj_k->modifiers[i]) {
			/* Learn the mod */
			player_learn_mod(p, i);

			/* Message */
			if (p->upkeep->playing) mod_message(obj, i);
		}
}

/**
 * Learn object properties that become obvious on use, mark it as
 * aware and reward the player with some experience.
 *
 * \param p is the player
 * \param obj is the used object
 */
void object_learn_on_use(struct player *p, struct object *obj)
{
	/* Object level */
	int lev = obj->kind->level;

	object_flavor_aware(obj);
	obj->known->effect = obj->effect;
	player_exp_gain(p, (lev + (p->lev / 2)) / p->lev);

	p->upkeep->notice |= PN_IGNORE;
}

/**
 * Learn attack bonus on making a ranged attack.
 * Can be applied to the missile or the missile launcher
 *
 * \param p is the player
 * \param obj is the missile or launcher
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
 * ------------------------------------------------------------------------
 * Object kind functions
 * These deal with knowledge of an object's kind
 * ------------------------------------------------------------------------ */

/**
 * Checks whether an object counts as "known" due to EASY_KNOW status
 *
 * \param obj is the object
 */
bool easy_know(const struct object *obj)
{
	assert(obj->kind);
	if (obj->kind->aware && kf_has(obj->kind->kind_flags, KF_EASY_KNOW))
		return true;
	else
		return false;
}

/**
 * Checks whether the player is aware of the object's flavour
 *
 * \param obj is the object
 */
bool object_flavor_is_aware(const struct object *obj)
{
	assert(obj->kind);
	return obj->kind->aware;
}

/**
 * Checks whether the player has tried to use other objects of the same kind
 *
 * \param obj is the object
 */
bool object_flavor_was_tried(const struct object *obj)
{
	assert(obj->kind);
	return obj->kind->tried;
}

/**
 * Mark an object's flavour as as one the player is aware of.
 *
 * \param obj is the object whose flavour should be marked as aware
 */
void object_flavor_aware(struct object *obj)
{
	int y, x;

	assert(obj->known);
	if (obj->kind->aware) return;
	obj->kind->aware = true;
	obj->known->effect = obj->effect;

	/* Fix ignore/autoinscribe */
	if (kind_is_ignored_unaware(obj->kind))
		kind_ignore_when_aware(obj->kind);
	player->upkeep->notice |= PN_IGNORE;

	/* Quit if no dungeon yet */
	if (!cave) return;

	/* Some objects change tile on awareness, so update display for all
	 * floor objects of this kind */
	for (y = 1; y < cave->height; y++) {
		for (x = 1; x < cave->width; x++) {
			bool light = false;
			const struct object *floor_obj;

			for (floor_obj = square_object(cave, y, x); floor_obj;
				 floor_obj = floor_obj->next)
				if (floor_obj->kind == obj->kind) {
					light = true;
					break;
				}

			if (light) square_light_spot(cave, y, x);
		}
	}
}


/**
 * Mark an object's flavour as tried.
 *
 * \param obj is the object whose flavour should be marked
 */
void object_flavor_tried(struct object *obj)
{
	assert(obj);
	assert(obj->kind);
	obj->kind->tried = true;
}

