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
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "player.h"
#include "store.h"


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
 * Transfer player object knowledge to an object
 */
void player_know_object(struct player *p, struct object *obj)
{
	int i;
	struct brand *b, *b_known;
	struct slay *s, *s_known;

	/* Unseen or only sensed objects don't get any ID */
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
		b = b->next;
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
		s = s->next;
	}

	/* Set combat details */
	obj->known->ac = p->obj_k->ac * obj->ac;
	obj->known->to_a = p->obj_k->to_a * obj->to_a;
	obj->known->to_h = p->obj_k->to_h * obj->to_h;
	obj->known->to_d = p->obj_k->to_d * obj->to_d;
	obj->known->dd = p->obj_k->dd * obj->dd;
	obj->known->ds = p->obj_k->ds * obj->ds;
}

/**
 * Propagate player knowledge of objects to all objects
 */
void update_player_object_knowledge(struct player *p)
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
}

/**
 * Learn a single flag
 */
bool player_learn_flag(struct player *p, int flag)
{
	/* If the flag was unknown, set it */
	if (of_on(p->obj_k->flags, flag))
		return true;

	return false;
}

/**
 * Learn a single modifier
 */
bool player_learn_mod(struct player *p, int mod)
{
	/* If the modifier was unknown, set it */
	if (p->obj_k->modifiers[mod] == 0) {
		p->obj_k->modifiers[mod] = 1;
		return true;
	}

	return false;
}

/**
 * Learn a single element
 */
bool player_learn_element(struct player *p, int element)
{
	/* If the element was unknown, set it */
	if (p->obj_k->el_info[element].res_level == 0) {
		p->obj_k->el_info[element].res_level = 1;
		return true;
	}

	return false;
}

/**
 * Learn a single elemental brand
 */
bool player_learn_brand(struct player *p, struct brand *b)
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

		return true;
	}

	return false;
}

/**
 * Learn a single slay
 */
bool player_learn_slay(struct player *p, struct slay *s)
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

		return true;
	}

	return false;
}

/**
 * Learn armour class
 */
bool player_learn_ac(struct player *p)
{
	if (p->obj_k->ac) return false;
	p->obj_k->ac = 1;
	return true;
}

/**
 * Learn to-armour bonus
 */
bool player_learn_to_a(struct player *p)
{
	if (p->obj_k->to_a) return false;
	p->obj_k->to_a = 1;
	return true;
}

/**
 * Learn to-hit bonus
 */
bool player_learn_to_h(struct player *p)
{
	if (p->obj_k->to_h) return false;
	p->obj_k->to_h = 1;
	return true;
}

/**
 * Learn to-damage bonus
 */
bool player_learn_to_d(struct player *p)
{
	if (p->obj_k->to_d) return false;
	p->obj_k->to_d = 1;
	return true;
}

/**
 * Learn damage dice
 */
bool player_learn_dice(struct player *p)
{
	if (p->obj_k->dd && p->obj_k->ds) return false;
	p->obj_k->dd = 1;
	p->obj_k->ds = 1;
	return true;
}
