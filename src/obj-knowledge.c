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
#include "init.h"
#include "object.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-properties.h"
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
 * Object knowledge data
 * This section covers initialisation, access and cleanup of rune data
 * ------------------------------------------------------------------------ */
/**
 * Describes a flag-name pair.
 */
struct flag_type {
	int flag;
	const char *name;
};

static size_t rune_max;
static struct rune *rune_list;
static char *c_rune[] = {
	"enchantment to armor",
	"enchantment to hit",
	"enchantment to damage"
};

static const struct flag_type f_rune[] =
{
	{ OF_NONE, "" },
	#define STAT(a, b, c, d, e, f, g, h, i) { OF_##c, i },
	#include "list-stats.h"
	#undef STAT
	#define OF(a, b, c, d, e, f) { OF_##a, e },
	#include "list-object-flags.h"
	#undef OF
};

static const char *e_rune[] =
{
	#define ELEM(a, b, c, d, e, f, g, h, i, col) b,
    #include "list-elements.h"
    #undef ELEM
};

static const char *m_rune[] =
{
	#define STAT(a, b, c, d, e, f, g, h, i) h,
	#include "list-stats.h"
	#undef STAT
	#define OBJ_MOD(a, b, c, d) d,
	#include "list-object-modifiers.h"
	#undef OBJ_MOD
};

/**
 * Initialise the rune module
 */
static void init_rune(void)
{
	int i, count, brands = 0, slays = 0;
	struct brand *b;
	struct slay *s;

	/* Count runes (combat runes are fixed) */
	count = COMBAT_RUNE_MAX;
	for (i = 0; i < OF_MAX; i++) {
		if (obj_flag_type(i) == OFT_NONE) continue;
		if (obj_flag_type(i) == OFT_LIGHT) continue;
		if (obj_flag_type(i) == OFT_CURSE) continue;
		count++;
	}
	for (i = 0; i < OBJ_MOD_MAX; i++)
		count++;
	for (i = 0; i <= ELEM_HIGH_MAX; i++)
		count++;
	for (b = game_brands; b; b = b->next)
		count++;
	for (s = game_slays; s; s = s->next)
		count++;

	/* Now allocate and fill the rune list */
	rune_max = count;
	rune_list = mem_zalloc(rune_max * sizeof(struct rune));
	count = 0;
	for (i = 0; i < COMBAT_RUNE_MAX; i++)
		rune_list[count++] = (struct rune) { RUNE_VAR_COMBAT, i, 0, c_rune[i] };
	for (i = 0; i < OBJ_MOD_MAX; i++)
		rune_list[count++] = (struct rune) { RUNE_VAR_MOD, i, 0, m_rune[i] };
	for (i = 0; i <= ELEM_HIGH_MAX; i++)
		rune_list[count++] = (struct rune) { RUNE_VAR_RESIST, i, 0, e_rune[i] };
	for (b = game_brands; b; b = b->next)
		rune_list[count++] =
			(struct rune) { RUNE_VAR_BRAND, brands++, 0, b->name};
	for (s = game_slays; s; s = s->next)
		rune_list[count++] =
			(struct rune) { RUNE_VAR_SLAY, slays++, 0, s->name };
	for (i = 0; i < OF_MAX; i++) {
		if (obj_flag_type(i) == OFT_NONE) continue;
		if (obj_flag_type(i) == OFT_LIGHT) continue;
		if (obj_flag_type(i) == OFT_CURSE) continue;

		rune_list[count++] = (struct rune)
			{ RUNE_VAR_FLAG, i, 0, f_rune[i].name };
	}
}

/**
 * Get a rune by variety and index
 */
static int rune_index(size_t variety, int index)
{
	size_t i;

	/* Look for the rune */
	for (i = 0; i < rune_max; i++)
		if ((rune_list[i].variety == variety) && (rune_list[i].index == index))
			return i;

	/* Can't find it */
	return -1;
}

/**
 * Cleanup the rune module
 */
static void cleanup_rune(void)
{
	mem_free(rune_list);
}

struct init_module rune_module = {
	.name = "rune",
	.init = init_rune,
	.cleanup = cleanup_rune
};

/**
 * ------------------------------------------------------------------------
 * Rune knowledge functions
 * These functions provide details about the rune list for use in 
 * player knowledge screens
 * ------------------------------------------------------------------------ */
/**
 * The number of runes
 */
int max_runes(void)
{
	return rune_max;
}

/**
 * The variety of a rune
 */
enum rune_variety rune_variety(size_t i)
{
	return rune_list[i].variety;
}

/**
 * Reports if the player knows a given rune
 *
 * \param p is the player
 * \param i is the rune index
 */
bool player_knows_rune(struct player *p, size_t i)
{
	struct rune *r = &rune_list[i];

	switch (r->variety) {
		/* Combat runes */
		case RUNE_VAR_COMBAT: {
			if (r->index == COMBAT_RUNE_TO_A) {
				if (p->obj_k->to_a)
					return true;
			} else if (r->index == COMBAT_RUNE_TO_H) {
				if (p->obj_k->to_h)
					return true;
			} else if (r->index == COMBAT_RUNE_TO_D) {
				if (p->obj_k->to_d)
					return true;
			}
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			if (p->obj_k->modifiers[r->index])
				return true;
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			if (p->obj_k->el_info[r->index].res_level)
				return true;
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			int num;
			struct brand *b;
			for (b = game_brands, num = 0; b; b = b->next, num++)
				if (num == r->index) break;
			assert(b != NULL);

			if (player_knows_brand(p, b))
				return true;
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			int num;
			struct slay *s;
			for (s = game_slays, num = 0; s; s = s->next, num++)
				if (num == r->index) break;
			assert(s != NULL);

			if (player_knows_slay(p, s))
				return true;
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			if (of_has(p->obj_k->flags, r->index))
				return true;
			break;
		}
		default: {
			break;
		}
	}

	return false;
}

/**
 * The name of a rune
 */
char *rune_name(size_t i)
{
	struct rune *r = &rune_list[i];

	if (r->variety == RUNE_VAR_BRAND)
		return format("%s brand", r->name);
	else if (r->variety == RUNE_VAR_SLAY)
		return format("slay %s", r->name);
	else if (r->variety == RUNE_VAR_RESIST)
		return format("resist %s", r->name);
	else
		return format("%s", r->name);

	return NULL;
}

/**
 * The description of a rune
 */
char *rune_desc(size_t i)
{
	struct rune *r = &rune_list[i];

	switch (r->variety) {
		/* Combat runes */
		case RUNE_VAR_COMBAT: {
			if (r->index == COMBAT_RUNE_TO_A)
				return "Object magically increases the player's armor class";
			else if (r->index == COMBAT_RUNE_TO_H)
				return "Object magically increases the player's chance to hit";
			else if (r->index == COMBAT_RUNE_TO_D)
				return "Object magically increases the player's damage";
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			return format("Object gives the player a magical bonus to %s.",
						  r->name);
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			return format("Object affects the player's resistance to %s.",
						  r->name);
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			int num;
			struct brand *b;
			for (b = game_brands, num = 0; b; b = b->next, num++)
				if (num == r->index) break;
			assert(b != NULL);

			return format("Object brands the player's attacks with %s.",
						  r->name);
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			int num;
			struct slay *s;
			for (s = game_slays, num = 0; s; s = s->next, num++)
				if (num == r->index) break;
			assert(s != NULL);

			return format("Object makes the player's attacks against %s more powerful.", r->name);
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			return format("Object gives the player the property of %s.",
						  r->name);
			break;
		}
		default: {
			break;
		}
	}

	return NULL;
}

/**
 * The autoinscription index (if any) of a rune
 */
quark_t rune_note(size_t i)
{
	return rune_list[i].note;
}

/**
 * Set an autoinscription on a rune
 */
void rune_set_note(size_t i, const char *inscription)
{
	struct rune *r = &rune_list[i];

	if (!inscription)
		r->note = 0;
	else
		r->note = quark_add(inscription);
}

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
 * Checks whether the object is known to be an artifact
 *
 * \param obj is the object
 */
bool object_is_in_store(const struct object *obj)
{
	int i;
	struct object *obj1;

	/* Check all the store objects */
	for (i = 0; i < MAX_STORES; i++) {
		struct store *s = &stores[i];
		for (obj1 = s->stock; obj1; obj1 = obj1->next)
			if (obj1 == obj) return true;
	}

	return false;
}

/**
 * Check if all the runes on an object are known to the player
 *
 * \param obj is the object
 */
bool object_has_rune(const struct object *obj, int rune_no)
{
	struct rune *r = &rune_list[rune_no];

	switch (r->variety) {
		/* Combat runes - just check them all */
		case RUNE_VAR_COMBAT: {
			if ((r->index == COMBAT_RUNE_TO_A) && (obj->to_a))
				return true;
			else if ((r->index == COMBAT_RUNE_TO_H) && (obj->to_h))
				return true;
			else if ((r->index == COMBAT_RUNE_TO_D) && (obj->to_d))
				return true;
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			if (obj->modifiers[r->index] != 0)
				return true;
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			if (obj->el_info[r->index].res_level != 0)
				return true;
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			struct brand *b;
			for (b = obj->brands; b; b = b->next)
				if (streq(b->name, r->name))
					return true;
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			struct slay *s;
			for (s = obj->slays; s; s = s->next)
				if (streq(s->name, r->name))
					return true;
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			if (of_has(obj->flags, r->index))
				return true;
			break;
		}
		default: break;
	}

	return false;
}

/**
 * Check if all the runes on an object are known to the player
 *
 * \param obj is the object
 */
bool object_runes_known(const struct object *obj)
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

	return true;
}

/**
 * Check if an object is fully known to the player
 *
 * \param obj is the object
 */
bool object_fully_known(const struct object *obj)
{
	/* Not all runes known */
	if (!object_runes_known(obj)) return false;

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

	/* Generic dice and ac, and launcher multipliers */
	obj->known->dd = obj->kind->dd * player->obj_k->dd;
	obj->known->ds = obj->kind->ds * player->obj_k->ds;
	obj->known->ac = obj->kind->ac * player->obj_k->ac;
	if (tval_is_launcher(obj))
		obj->known->pval = obj->pval;

	/* Aware flavours and unflavored non-wearables get info now */
	if ((obj->kind->aware && obj->kind->flavor) ||
		(!tval_is_wearable(obj) && !obj->kind->flavor)) {
		obj->known->pval = obj->pval;
		obj->known->effect = obj->effect;
	}

	/* Non-jewelry wearables have known activations */
	if (tval_is_wearable(obj) && !tval_is_jewelry(obj) && obj->kind->effect)
		obj->known->effect = obj->effect;
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
	struct brand *b;
	struct slay *s;

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
			new_b->next = obj->known->brands;
			obj->known->brands = new_b;
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
			new_s->next = obj->known->slays;
			obj->known->slays = new_s;
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
void update_player_object_knowledge(struct player *p)
{
	int i;
	struct object *obj;

	/* Hack - REMOVE THIS AFTER COMP 186 - NRM */
	p->obj_k->dd = 1;
	p->obj_k->ds = 1;
	p->obj_k->ac = 1;

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

	/* Update */
	if (cave)
		autoinscribe_ground();
	autoinscribe_pack();
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/**
 * ------------------------------------------------------------------------
 * Object knowledge learners
 * These functions are for increasing player knowledge of object properties
 * ------------------------------------------------------------------------ */
/**
 * Learn a given rune
 *
 * \param p is the player
 * \param i is the rune index
 * \param message is whether or not to print a message
 */
static void player_learn_rune(struct player *p, size_t i, bool message)
{
	struct rune *r = &rune_list[i];
	bool learned = false;

	switch (r->variety) {
		/* Combat runes */
		case RUNE_VAR_COMBAT: {
			if (r->index == COMBAT_RUNE_TO_A) {
				if (!p->obj_k->to_a) {
					p->obj_k->to_a = 1;
					learned = true;
				}
			} else if (r->index == COMBAT_RUNE_TO_H) {
				if (!p->obj_k->to_h) {
					p->obj_k->to_h = 1;
					learned = true;
				}
			} else if (r->index == COMBAT_RUNE_TO_D) {
				if (!p->obj_k->to_d) {
					p->obj_k->to_d = 1;
					learned = true;
				}
			}
			break;
		}
		/* Mod runes */
		case RUNE_VAR_MOD: {
			if (!p->obj_k->modifiers[r->index]) {
				p->obj_k->modifiers[r->index] = 1;
				learned = true;
			}
			break;
		}
		/* Element runes */
		case RUNE_VAR_RESIST: {
			if (!p->obj_k->el_info[r->index].res_level) {
				p->obj_k->el_info[r->index].res_level = 1;
				learned = true;
			}
			break;
		}
		/* Brand runes */
		case RUNE_VAR_BRAND: {
			int num;
			struct brand *b;
			for (b = game_brands, num = 0; b; b = b->next, num++)
				if (num == r->index) break;
			assert(b != NULL);

			/* If the brand was unknown, add it to known brands */
			if (!player_knows_brand(p, b)) {
				/* Copy the name and element */
				struct brand *new_b = mem_zalloc(sizeof *new_b);
				new_b->name = string_make(b->name);
				new_b->element = b->element;

				/* Attach the new brand */
				new_b->next = p->obj_k->brands;
				p->obj_k->brands = new_b;
				learned = true;
			}
			break;
		}
		/* Slay runes */
		case RUNE_VAR_SLAY: {
			int num;
			struct slay *s;
			for (s = game_slays, num = 0; s; s = s->next, num++)
				if (num == r->index) break;
			assert(s != NULL);

			/* If the slay was unknown, add it to known slays */
			if (!player_knows_slay(p, s)) {
				/* Copy the name and race flag */
				struct slay *new_s = mem_zalloc(sizeof *new_s);
				new_s->name = string_make(s->name);
				new_s->race_flag = s->race_flag;

				/* Attach the new slay */
				new_s->next = p->obj_k->slays;
				p->obj_k->slays = new_s;
				learned = true;
			}
			break;
		}
		/* Flag runes */
		case RUNE_VAR_FLAG: {
			if (of_on(p->obj_k->flags, r->index))
				learned = true;
			break;
		}
		default: {
			learned = false;
			break;
		}
	}

	/* Nothing learned */
	if (!learned) return;

	/* Give a message */
	if (message)
		msg("You have learned the rune of %s.", rune_name(i));

	/* Update knowledge */
	update_player_object_knowledge(p);
}

/**
 * Learn absolutely everything
 *
 * \param p is the player
 */
void player_learn_everything(struct player *p)
{
	size_t i;

	for (i = 0; i < rune_max; i++)
		player_learn_rune(p, i, false);
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

	if (p->obj_k->to_a) return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (obj) {
			assert(obj->known);
			if (obj->to_a) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_A);
				player_learn_rune(p, index, true);
			}
			if (p->obj_k->to_a) return;
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
			if (obj->to_h) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
				player_learn_rune(p, index, true);
			}
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

	if (p->obj_k->to_h && p->obj_k->to_d)
		return;

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = slot_object(p, i);
		if (i == slot_by_name(p, "shooting")) continue;
		if (obj) {
			assert(obj->known);
			if (obj->to_h) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
				player_learn_rune(p, index, true);
			}
			if (obj->to_d) {
				int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_D);
				player_learn_rune(p, index, true);
			}
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

			/* Message */
			flag_message(flag, o_name);

			/* Learn the flag */
			player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
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

			/* Message */
			msg("Your %s glows.", o_name);

			/* Learn the element properties */
			player_learn_rune(p, rune_index(RUNE_VAR_RESIST, element), true);
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
	int i, flag;
	bitflag f[OF_SIZE], timed_mask[OF_SIZE];

	/* Get the timed flags */
	create_mask(timed_mask, true, OFID_TIMED, OFT_MAX);

	/* Get the unknown timed flags, and return if there are none */
	object_flags(p->obj_k, f);
	of_negate(f);
	of_inter(timed_mask, f);
	if (of_is_empty(timed_mask)) return;

	/* All wielded items eligible */
	for (i = 0; i < p->body.count; i++) {
		char o_name[80];
		struct object *obj = slot_object(p, i);
		if (!obj) continue;
		assert(obj->known);

		/* Get the unknown timed flags for this object */
		object_flags(obj, f);
		of_inter(f, timed_mask);

		/* Attempt to learn every flag */
		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
		for (flag = of_next(f, FLAG_START); flag != FLAG_END;
			 flag = of_next(f, flag + 1)) {
			if (!of_has(p->obj_k->flags, flag))
				flag_message(flag, o_name);
			player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);
		}
	}
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
 * Get a random unknown rune from an object
 *
 * \param p is the player
 * \param obj is the object
 * \return the index into the rune list, or -1 for no unknown runes
 */
int object_find_unknown_rune(struct player *p, struct object *obj)
{
	size_t i, num = 0;
	int *poss_runes = mem_zalloc(rune_max * sizeof(int));

	if (object_runes_known(obj)) return -1;

	for (i = 0; i < rune_max; i++) {
		struct rune *r = &rune_list[i];

		switch (r->variety) {
			/* Combat runes - just check them all */
			case RUNE_VAR_COMBAT: {
				if ((r->index == COMBAT_RUNE_TO_A) &&
					(obj->known->to_a != obj->to_a))
					poss_runes[num++] = i;
				else if ((r->index == COMBAT_RUNE_TO_H)
						 && (obj->known->to_h != obj->to_h))
					poss_runes[num++] = i;
				else if ((r->index == COMBAT_RUNE_TO_D)
						 && (obj->known->to_d != obj->to_d))
					poss_runes[num++] = i;
				break;
			}
			/* Mod runes */
			case RUNE_VAR_MOD: {
				if (obj->modifiers[r->index] != obj->known->modifiers[r->index])
					poss_runes[num++] = i;
				break;
			}
			/* Element runes */
			case RUNE_VAR_RESIST: {
				if (obj->el_info[r->index].res_level !=
					obj->known->el_info[r->index].res_level)
					poss_runes[num++] = i;
				break;
			}
			/* Brand runes */
			case RUNE_VAR_BRAND: {
				struct brand *b;
				for (b = obj->brands; b; b = b->next)
					if (streq(b->name, r->name)) break;

				/* Brand not on the object, or known */
				if (!b) break;
				if (player_knows_brand(p, b)) break;

				/* If we're here we have an unknown brand */
				poss_runes[num++] = i;
				break;
			}
			/* Slay runes */
			case RUNE_VAR_SLAY: {
				struct slay *s;
				for (s = obj->slays; s; s = s->next)
					if (streq(s->name, r->name)) break;

				/* Slay not on the object, or known */
				if (!s) break;
				if (player_knows_slay(p, s)) break;

				/* If we're here we have an unknown slay */
				poss_runes[num++] = i;
				break;
			}
			/* Flag runes */
			case RUNE_VAR_FLAG: {
				if (of_has(obj->flags, r->index) &&
					!of_has(obj->known->flags, r->index))
					poss_runes[num++] = i;
				break;
			}
			default: break;
		}
	}

	/* Grab a random rune from among the unknowns  */
	if (num)
		return poss_runes[randint0(num)];

	return -1;
}

/**
 * Learn a random unknown rune from an object
 *
 * \param p is the player
 * \param obj is the object
 */
void object_learn_unknown_rune(struct player *p, struct object *obj)
{
	/* Get a random unknown rune from the object */
	int i = object_find_unknown_rune(p, obj);

	/* No unknown runes */
	if (i < 0) return;

	/* Learn the rune */
	player_learn_rune(p, i, true);
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

	/* Make sustains obvious for items with that stat bonus */
	for (i = 0; i < STAT_MAX; i++)
		/* Sustains are the first flags, stats are the first modifiers */
		if ((obj_flag_type(i + 1) == OFT_SUST) && obj->modifiers[i])
			of_on(obvious_mask, i + 1);

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
		player_learn_rune(p, rune_index(RUNE_VAR_FLAG, flag), true);

		/* Message */
		if (p->upkeep->playing) flag_message(flag, o_name);
	}

	/* Learn all modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->modifiers[i] && !p->obj_k->modifiers[i]) {
			/* Message */
			if (p->upkeep->playing) mod_message(obj, i);

			/* Learn the mod */
			player_learn_rune(p, rune_index(RUNE_VAR_MOD, i), true);
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
 * Notice a brand on a particular object which affects a particular monster.
 *
 * \param obj is the object on which we are noticing brands
 * \param mon the monster we are hitting
 * \param b is the brand we are learning
 */
void object_learn_brand(struct player *p, struct object *obj, struct brand *b)
{
	/* Learn about the brand */
	if (!player_knows_brand(player, b)) {
		int num = 0;
		struct brand *b_check = game_brands;

		/* Find the rune index */
		while (b_check && !streq(b_check->name, b->name)) {
			num++;
			b_check = b_check->next;
		}
		assert(b_check);

		/* Learn the rune */
		player_learn_rune(p, rune_index(RUNE_VAR_BRAND, num), true);
		update_player_object_knowledge(p);
	}
}

/**
 * Notice any slays on a particular object which affect a particular monster.
 *
 * \param obj is the object on which we are noticing slays
 * \param mon the monster we are trying to slay
 */
void object_learn_slay(struct player *p, struct object *obj, struct slay *s)
{
	/* Learn about the slay */
	if (!player_knows_slay(player, s)) {
		int num = 0;
		struct slay *s_check = game_slays;

		/* Find the rune index */
		while (s_check && !streq(s_check->name, s->name)) {
			num++;
			s_check = s_check->next;
		}
		assert(s_check);

		/* Learn the rune */
		player_learn_rune(p, rune_index(RUNE_VAR_SLAY, num), true);
		update_player_object_knowledge(p);
	}
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
	if (p->obj_k->to_h && p->obj_k->to_d)
		return;

	assert(obj->known);
	if (obj->to_h) {
		int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_H);
		player_learn_rune(p, index, true);
	}
	if (obj->to_d) {
		int index = rune_index(RUNE_VAR_COMBAT, COMBAT_RUNE_TO_D);
		player_learn_rune(p, index, true);
	}
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
	int y, x, i;
	struct object *obj1;

	assert(obj->known);
	if (obj->kind->aware) return;
	obj->kind->aware = true;
	obj->known->effect = obj->effect;

	/* Fix ignore/autoinscribe */
	if (kind_is_ignored_unaware(obj->kind))
		kind_ignore_when_aware(obj->kind);
	player->upkeep->notice |= PN_IGNORE;

	/* Update player objects */
	for (obj1 = player->gear; obj1; obj1 = obj1->next)
		object_set_base_known(obj1);

	/* Store objects */
	for (i = 0; i < MAX_STORES; i++) {
		struct store *s = &stores[i];
		for (obj1 = s->stock; obj1; obj1 = obj1->next)
			object_set_base_known(obj1);
	}

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
