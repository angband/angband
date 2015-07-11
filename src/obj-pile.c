/**
 * \file obj-pile.c
 * \brief Deal with piles of objects
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "effects.h"
#include "cmd-core.h"
#include "game-input.h"
#include "generate.h"
#include "grafmode.h"
#include "init.h"
#include "mon-make.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-spell.h"
#include "player-util.h"
#include "randname.h"
#include "z-queue.h"

/* #define LIST_DEBUG */

/**
 * Check the integrity of a linked - make sure it's not circular and that each
 * entry in the chain has consistent next and prev pointers.
 */
void pile_check_integrity(const char *op, struct object *pile, struct object *hilight)
{
	struct object *obj = pile;
	struct object *prev = NULL;

#ifdef LIST_DEBUG
	int i = 0;
	fprintf(stderr, "\n%s  pile %08x\n", op, (int)pile);
#endif

	/* Check prev<->next chain */
	while (obj) {
#ifdef LIST_DEBUG
		fprintf(stderr, "[%2d] this = %08x  prev = %08x  next = %08x  %s\n",
			i, (int)obj, (int)obj->prev, (int)obj->next,
			(obj == hilight) ? "*" : "");
		i++;
#endif

		assert(obj->prev == prev);
		prev = obj;
		obj = obj->next;
	};

	/* Check for circularity */
	for (obj = pile; obj; obj = obj->next) {
		struct object *check;
		for (check = obj->next; check; check = check->next) {
			assert(check->next != obj);
		}
	}
}

/**
 * Insert 'obj' into the pile 'pile'.
 *
 * 'obj' must not already be in any other lists.
 */
void pile_insert(struct object **pile, struct object *obj)
{
	assert(obj->prev == NULL);
	assert(obj->next == NULL);

	if (*pile) {
		obj->next = *pile;
		(*pile)->prev = obj;
	}

	*pile = obj;

	pile_check_integrity("insert", *pile, obj);
}

/**
 * Insert 'obj' at the end of pile 'pile'.
 *
 * Unlike pile_insert(), obj can be the beginning of a new list of objects.
 */
void pile_insert_end(struct object **pile, struct object *obj)
{
	assert(obj->prev == NULL);

	if (*pile) {
		struct object *end = pile_last_item(*pile);

		end->next = obj;
		obj->prev = end;
	} else {
		*pile = obj;
	}

	pile_check_integrity("insert_end", *pile, obj);
}

/**
 * Remove object 'obj' from pile 'pile'.
 */
void pile_excise(struct object **pile, struct object *obj)
{
	struct object *prev = obj->prev;
	struct object *next = obj->next;

	assert(pile_contains(*pile, obj));
	pile_check_integrity("excise [pre]", *pile, obj);

	/* Special case: unlink top object */
	if (*pile == obj) {
		assert(prev == NULL);	/* Invariant - if it's the top of the pile */

		*pile = next;
	} else {
		assert(obj->prev != NULL);	/* Should definitely have a previous one set */

		/* Otherwise unlink from the previous */
		prev->next = next;
		obj->prev = NULL;
	}

	/* And then unlink from the next */
	if (next) {
		next->prev = prev;
		obj->next = NULL;
	}

	pile_check_integrity("excise [post]", *pile, NULL);
}

/**
 * Return the last item in pile 'pile'.
 */
struct object *pile_last_item(struct object *const pile)
{
	struct object *obj = pile;

	pile_check_integrity("last_item", pile, NULL);

	/* No pile at all */
	if (!pile)
		return NULL;

	/* Run along the list, stopping just before the end */
	while (obj->next)
		obj = obj->next;

	return obj;
}

/**
 * Check if pile 'pile' contains object 'obj'.
 */
bool pile_contains(const struct object *top, const struct object *obj)
{
	const struct object *pile_obj = top;

	while (pile_obj) {
		if (obj == pile_obj)
			return TRUE;
		pile_obj = pile_obj->next;
	}

	return FALSE;
}

/**
 * Create a new object and return it
 */
struct object *object_new(void)
{
	return mem_zalloc(sizeof(struct object));
}

/**
 * Delete an object and free its memory, and set its pointer to NULL
 */
void object_delete(struct object **obj_address)
{
	struct object *obj = *obj_address;
	struct object *prev = obj->prev;
	struct object *next = obj->next;

	/* Free slays and brands */
	if (obj->slays)
		free_slay(obj->slays);
	if (obj->brands)
		free_brand(obj->brands);

	/* Check any next and previous objects */
	if (next) {
		if (prev) {
			prev->next = next;
			next->prev = prev;
		} else {
			next->prev = NULL;
		}
	} else if (prev) {
		prev->next = NULL;
	}

	/* If we're tracking the object, stop */
	if (player && player->upkeep && obj == player->upkeep->object)
		player->upkeep->object = NULL;

	mem_free(obj);
	*obj_address = NULL;
}

/**
 * Free an entire object pile
 */
void object_pile_free(struct object *obj)
{
	struct object *current = obj, *next;

	while (current) {
		next = current->next;
		object_delete(&current);
		current = next;
	}
}


/**
 * Determine if an item can "absorb" a second item
 *
 * See "object_absorb()" for the actual "absorption" code.
 *
 * If permitted, we allow weapons/armor to stack, if "known".
 *
 * Missiles will combine if both stacks have the same "known" status.
 * This is done to make unidentified stacks of missiles useful.
 *
 * Food, potions, scrolls, and "easy know" items always stack.
 *
 * Chests, and activatable items, except rods, never stack (for various
 * reasons).
 */
bool object_stackable(const struct object *obj1, const struct object *obj2,
					  object_stack_t mode)
{
	int i;

	/* Equipment items don't stack */
	if (object_is_equipped(player->body, obj1))
		return FALSE;
	if (object_is_equipped(player->body, obj2))
		return FALSE;

	/* If either item is unknown, do not stack */
	if (mode & OSTACK_LIST && obj1->marked == MARK_AWARE) return FALSE;
	if (mode & OSTACK_LIST && obj2->marked == MARK_AWARE) return FALSE;

	/* Hack -- identical items cannot be stacked */
	if (obj1 == obj2) return FALSE;

	/* Require identical object kinds */
	if (obj1->kind != obj2->kind) return FALSE;

	/* Different flags don't stack */
	if (!of_is_equal(obj1->flags, obj2->flags)) return FALSE;

	/* Different elements don't stack */
	for (i = 0; i < ELEM_MAX; i++) {
		if (obj1->el_info[i].res_level != obj2->el_info[i].res_level)
			return FALSE;
		if ((obj1->el_info[i].flags & (EL_INFO_HATES | EL_INFO_IGNORE)) !=
			(obj2->el_info[i].flags & (EL_INFO_HATES | EL_INFO_IGNORE)))
			return FALSE;
	}

	/* Artifacts never stack */
	if (obj1->artifact || obj2->artifact) return FALSE;

	/* Analyze the items */
	if (tval_is_chest(obj1)) {
		/* Chests never stack */
		return FALSE;
	}
	else if (tval_is_edible(obj1) || tval_is_potion(obj1) ||
		tval_is_scroll(obj1) || tval_is_rod(obj1)) {
		/* Food, potions, scrolls and rods all stack nicely,
		   since the kinds are identical, either both will be
		   aware or both will be unaware */
	} else if (tval_can_have_charges(obj1) || tval_is_money(obj1)) {
		/* Gold, staves and wands stack most of the time */
		/* Too much gold or too many charges */
		if (obj1->pval + obj2->pval > MAX_PVAL)
			return FALSE;

		/* ... otherwise ok */
	} else if (tval_is_weapon(obj1) || tval_is_armor(obj1) ||
		tval_is_jewelry(obj1) || tval_is_light(obj1)) {
		bool obj1_is_known = object_is_known(obj1);
		bool obj2_is_known = object_is_known(obj2);

		/* Require identical values */
		if (obj1->ac != obj2->ac) return FALSE;
		if (obj1->dd != obj2->dd) return FALSE;
		if (obj1->ds != obj2->ds) return FALSE;

		/* Require identical bonuses */
		if (obj1->to_h != obj2->to_h) return FALSE;
		if (obj1->to_d != obj2->to_d) return FALSE;
		if (obj1->to_a != obj2->to_a) return FALSE;

		/* Require all identical modifiers */
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (obj1->modifiers[i] != obj2->modifiers[i])
				return (FALSE);

		/* Require identical ego-item types */
		if (obj1->ego != obj2->ego) return FALSE;

		/* Hack - Never stack recharging wearables ... */
		if ((obj1->timeout || obj2->timeout) &&
			!tval_is_light(obj1)) return FALSE;

		/* ... and lights must have same amount of fuel */
		else if ((obj1->timeout != obj2->timeout) &&
				 tval_is_light(obj1)) return FALSE;

		/* Prevent unIDd items stacking with IDd items in the object list */
		if (mode & OSTACK_LIST && (obj1_is_known != obj2_is_known))
			return FALSE;
	} else {
		/* Anything else probably okay */
	}

	/* Require compatible inscriptions */
	if (obj1->note && obj2->note && (obj1->note != obj2->note))
		return FALSE;

	/* They must be similar enough */
	return TRUE;
}

/**
 * Return whether each stack of objects can be merged into one stack.
 */
bool object_similar(const struct object *obj1, const struct object *obj2,
					object_stack_t mode)
{
	int total = obj1->number + obj2->number;

	/* Check against stacking limit - except in stores which absorb anyway */
	if (!(mode & OSTACK_STORE) && (total > z_info->stack_size))
		return FALSE;

	return object_stackable(obj1, obj2, mode);
}

/**
 * Allow one item to "absorb" another, assuming they are similar.
 *
 * The blending of the "note" field assumes that either (1) one has an
 * inscription and the other does not, or (2) neither has an inscription.
 * In both these cases, we can simply use the existing note, unless the
 * blending object has a note, in which case we use that note.
 *
* These assumptions are enforced by the "object_similar()" code.
 */
static void object_absorb_merge(struct object *obj1, const struct object *obj2)
{
	int total;

	/* Blend all knowledge */
	of_union(obj1->known_flags, obj2->known_flags);
	of_union(obj1->id_flags, obj2->id_flags);

	/* Merge inscriptions */
	if (obj2->note)
		obj1->note = obj2->note;

	/* Combine timeouts for rod stacking */
	if (tval_can_have_timeout(obj1))
		obj1->timeout += obj2->timeout;

	/* Combine pvals for wands and staves */
	if (tval_can_have_charges(obj1) || tval_is_money(obj1)) {
		total = obj1->pval + obj2->pval;
		obj1->pval = total >= MAX_PVAL ? MAX_PVAL : total;
	}

	/* Combine origin data as best we can */
	if (obj1->origin != obj2->origin ||
		obj1->origin_depth != obj2->origin_depth ||
		obj1->origin_xtra != obj2->origin_xtra) {
		int act = 2;

		if (obj1->origin_xtra && obj2->origin_xtra) {
			struct monster_race *race1 = &r_info[obj1->origin_xtra];
			struct monster_race *race2 = &r_info[obj2->origin_xtra];

			bool r1_uniq = rf_has(race1->flags, RF_UNIQUE) ? TRUE : FALSE;
			bool r2_uniq = rf_has(race2->flags, RF_UNIQUE) ? TRUE : FALSE;

			if (r1_uniq && !r2_uniq) act = 0;
			else if (r2_uniq && !r1_uniq) act = 1;
			else act = 2;
		}

		switch (act)
		{
				/* Overwrite with obj2 */
			case 1:
			{
				obj1->origin = obj2->origin;
				obj1->origin_depth = obj2->origin_depth;
				obj1->origin_xtra = obj2->origin_xtra;
			}

				/* Set as "mixed" */
			case 2:
			{
				obj1->origin = ORIGIN_MIXED;
			}
		}
	}
}

/**
 * Merge a smaller stack into a larger stack, leaving two uneven stacks.
 */
void object_absorb_partial(struct object *obj1, struct object *obj2)
{
	int smallest = MIN(obj1->number, obj2->number);
	int largest = MAX(obj1->number, obj2->number);
	int difference = (z_info->stack_size) - largest;
	obj1->number = largest + difference;
	obj2->number = smallest - difference;

	object_absorb_merge(obj1, obj2);
}

/**
 * Merge two stacks into one stack.
 */
void object_absorb(struct object *obj1, struct object *obj2)
{
	int total = obj1->number + obj2->number;

	/* Add together the item counts */
	obj1->number = (total < z_info->stack_size ? total : z_info->stack_size);

	object_absorb_merge(obj1, obj2);
	object_delete(&obj2);
}

/**
 * Wipe an object clean.
 */
void object_wipe(struct object *obj)
{
	/* Wipe the structure */
	memset(obj, 0, sizeof(*obj));
}


/**
 * Prepare an object based on an existing object
 */
void object_copy(struct object *dest, const struct object *src)
{
	/* Copy the structure */
	memcpy(dest, src, sizeof(struct object));

	dest->slays = NULL;
	dest->brands = NULL;

	if (src->slays)
		copy_slay(&dest->slays, src->slays);
	if (src->brands)
		copy_brand(&dest->brands, src->brands);

	/* Detach from any pile */
	dest->prev = NULL;
	dest->next = NULL;
}

/**
 * Prepare an object `dst` representing `amt` objects,  based on an existing
 * object `src` representing at least `amt` objects.
 *
 * Takes care of the charge redistribution concerns of stacked items.
 */
void object_copy_amt(struct object *dest, struct object *src, int amt)
{
	int charge_time = randcalc(src->time, 0, AVERAGE), max_time;

	/* Get a copy of the object */
	object_copy(dest, src);

	/* Modify quantity */
	dest->number = amt;
	dest->note = src->note;

	/*
	 * If the item has charges/timeouts, set them to the correct level
	 * too. We split off the same amount as distribute_charges.
	 */
	if (tval_can_have_charges(src))
		dest->pval = src->pval * amt / src->number;

	if (tval_can_have_timeout(src)) {
		max_time = charge_time * amt;

		if (src->timeout > max_time)
			dest->timeout = max_time;
		else
			dest->timeout = src->timeout;
	}
}

/**
 * Split off 'amt' items from 'src' and return.
 *
 * Where object_copy_amt() makes `amt` new objects, this function leaves the
 * total number unchanged; otherwise the two functions are similar.
 *
 * This function should only be used when amt < src->number
 */
struct object *object_split(struct object *src, int amt)
{
	struct object *dest = object_new();

	/* Get a copy of the object */
	object_copy(dest, src);

	/* Check legality */
	assert(src->number > amt);

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(src, dest, amt);

	/* Modify quantity */
	dest->number = amt;
	src->number -= amt;
	if (src->note)
		dest->note = src->note;

	return dest;
}

/**
 * Remove an amount of an object from the floor, returning a detached object
 * which can be used - it is assumed that the object is on the player grid.
 *
 * Optionally describe what remains.
 */
struct object *floor_object_for_use(struct object *obj, int num, bool message,
									bool *none_left)
{
	struct object *usable;
	char name[80];

	/* Bounds check */
	num = MIN(num, obj->number);

	/* Split off a usable object if necessary */
	if (obj->number > num) {
		usable = object_split(obj, num);
	} else {
		usable = obj;
		square_excise_object(cave, usable->iy, usable->ix, usable);
		*none_left = TRUE;

		/* Stop tracking item */
		if (tracked_object_is(player->upkeep, obj))
			track_object(player->upkeep, NULL);

		/* Inventory has changed, so disable repeat command */ 
		cmd_disable_repeat();
	}

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS | PU_INVEN);
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Print a message if requested and there is anything left */
	if (message) {
		if (usable == obj)
			obj->number--;

		/* Get a description */
		object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);

		if (usable == obj)
			obj->number++;

		/* Print a message */
		msg("You see %s.", name);
	}

	return usable;
}


/**
 * Find and return the oldest object on the given grid marked as "ignore".
 */
static struct object *floor_get_oldest_ignored(int y, int x)
{
	struct object *obj, *ignore = NULL;

	for (obj = square_object(cave, y, x); obj; obj = obj->next)
		if (ignore_item_ok(obj))
			ignore = obj;

	return ignore;
}


/**
 * Let the floor carry an object, deleting old ignored items if necessary
 *
 * Optionally put the object at the top or bottom of the pile
 */
bool floor_carry(struct chunk *c, int y, int x, struct object *drop, bool last)
{
	int n = 0;
	struct object *obj;

	/* Scan objects in that grid for combination */
	for (obj = square_object(c, y, x); obj; obj = obj->next) {
		/* Check for combination */
		if (object_similar(obj, drop, OSTACK_FLOOR)) {
			/* Combine the items */
			object_absorb(obj, drop);

			/* Result */
			return TRUE;
		}

		/* Count objects */
		n++;
	}

	/* The stack is already too large */
	if (n >= z_info->floor_size || (OPT(birth_no_stacking) && n)) {
		/* Delete the oldest ignored object */
		struct object *ignore = floor_get_oldest_ignored(y, x);

		if (ignore) {
			square_excise_object(c, y, x, ignore);
			object_delete(&ignore);
		} else
			return FALSE;
	}

	/* Location */
	drop->iy = y;
	drop->ix = x;

	/* Forget monster */
	drop->held_m_idx = 0;

	/* Link to the first or last object in the pile */
	if (last)
		pile_insert_end(&c->squares[y][x].obj, drop);
	else
		pile_insert(&c->squares[y][x].obj, drop);

	/* Redraw */
	square_note_spot(c, y, x);
	square_light_spot(c, y, x);

	/* Result */
	return TRUE;
}


/**
 * Let an object fall to the ground at or near a location.
 *
 * The initial location is assumed to be "square_in_bounds_fully(cave, )".
 *
 * This function takes a parameter "chance".  This is the percentage
 * chance that the item will "disappear" instead of drop.  If the object
 * has been thrown, then this is the chance of disappearance on contact.
 *
 * This function will produce a description of a drop event under the player
 * when "verbose" is true.
 *
 * We check several locations to see if we can find a location at which
 * the object can combine, stack, or be placed.  Artifacts will try very
 * hard to be placed, including "teleporting" to a useful grid if needed.
 *
 * Objects which fail to be carried by the floor are deleted.
 */
void drop_near(struct chunk *c, struct object *dropped, int chance, int y,
			   int x, bool verbose)
{
	int i, k, n, d, s;

	int bs, bn;
	int by, bx;
	int dy, dx;
	int ty, tx;

	struct object *obj;

	char o_name[80];

	bool flag = FALSE;
	bool ignorable = ignore_item_ok(dropped);

	/* Describe object */
	object_desc(o_name, sizeof(o_name), dropped, ODESC_BASE);

	/* Handle normal "breakage" */
	if (!dropped->artifact && (randint0(100) < chance)) {
		/* Message */
		msg("The %s %s.", o_name,
			VERB_AGREEMENT(dropped->number, "breaks", "break"));

		/* Failure */
		return;
	}

	/* Score */
	bs = -1;

	/* Picker */
	bn = 0;

	/* Default */
	by = y;
	bx = x;

	/* Scan local grids */
	for (dy = -3; dy <= 3; dy++) {
		for (dx = -3; dx <= 3; dx++) {
			bool comb = FALSE;

			/* Calculate actual distance */
			d = (dy * dy) + (dx * dx);

			/* Ignore distant grids */
			if (d > 10) continue;

			/* Location */
			ty = y + dy;
			tx = x + dx;

			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, ty, tx)) continue;

			/* Require line of sight */
			if (!los(cave, y, x, ty, tx)) continue;

			/* Require floor space */
			if (!square_isfloor(cave, ty, tx)) continue;

			/* Require no trap or rune */
			if (square_isplayertrap(cave, ty, tx) ||
				square_iswarded(cave, ty, tx))
				continue;

			/* No objects */
			k = 0;
			n = 0;

			/* Scan objects in that grid */
			for (obj = square_object(c, ty, tx); obj; obj = obj->next) {
				/* Check for possible combination */
				if (object_similar(obj, dropped, OSTACK_FLOOR))
					comb = TRUE;

				/* Count objects */
				if (!ignore_item_ok(obj))
					k++;
				else
					n++;
			}

			/* Add new object */
			if (!comb) k++;

			/* Option -- disallow stacking */
			if (OPT(birth_no_stacking) && (k > 1)) continue;

			/* Paranoia? */
			if ((k + n) > z_info->floor_size &&
				!floor_get_oldest_ignored(ty, tx)) continue;

			/* Calculate score */
			s = 1000 - (d + k * 5);

			/* Skip bad values */
			if (s < bs) continue;

			/* New best value */
			if (s > bs) bn = 0;

			/* Apply the randomizer to equivalent values */
			if ((++bn >= 2) && (randint0(bn) != 0)) continue;

			/* Keep score */
			bs = s;

			/* Track it */
			by = ty;
			bx = tx;

			/* Okay */
			flag = TRUE;
		}
	}

	/* Handle lack of space */
	if (!flag && !dropped->artifact) {
		/* Message */
		msg("The %s %s.", o_name,
			VERB_AGREEMENT(dropped->number, "disappears", "disappear"));

		/* Debug */
		if (player->wizard) msg("Breakage (no floor space).");

		/* Failure */
		return;
	}

	/* Find a grid */
	for (i = 0; !flag; i++) {
		/* Bounce around */
		if (i < 1000) {
			ty = rand_spread(by, 1);
			tx = rand_spread(bx, 1);
		} else {
			/* Random locations */
			ty = randint0(c->height);
			tx = randint0(c->width);
		}

		/* Require floor space */
		if (!square_canputitem(cave, ty, tx)) continue;

		/* Bounce to that location */
		by = ty;
		bx = tx;

		/* Okay */
		flag = TRUE;
	}

	/* Give it to the floor */
	if (!floor_carry(c, by, bx, dropped, FALSE)) {
		/* Message */
		msg("The %s %s.", o_name,
			VERB_AGREEMENT(dropped->number, "disappears", "disappear"));

		/* Debug */
		if (player->wizard) msg("Breakage (too many objects).");

		if (dropped->artifact) dropped->artifact->created = FALSE;

		/* Failure */
		return;
	}

	/* Sound */
	sound(MSG_DROP);

	/* Message when an object falls under the player */
	if (verbose && (cave->squares[by][bx].mon < 0) && !ignorable)
		msg("You feel something roll beneath your feet.");
}

/**
 * This will push objects off a square.
 *
 * The methodology is to load all objects on the square into a queue. Replace
 * the previous square with a type that does not allow for objects. Drop the
 * objects. Last, put the square back to its original type.
 */
void push_object(int y, int x)
{
	/* Save the original terrain feature */
	struct feature *feat_old = square_feat(cave, y, x);

	struct object *obj = square_object(cave, y, x);

	struct queue *queue = q_new(z_info->floor_size);

	bool glyph = square_iswarded(cave, y, x);

	/* Push all objects on the square, stripped of pile info, into the queue */
	while (obj) {
		struct object *next = obj->next;
		q_push_ptr(queue, obj);

		/* Orphan the object */
		obj->next = NULL;
		obj->prev = NULL;

		/* Next object */
		obj = next;
	}

	/* Disassociate the objects from the square */
	cave->squares[y][x].obj = NULL;

	/* Set feature to an open door */
	square_force_floor(cave, y, x);
	square_add_door(cave, y, x, FALSE);

	/* Drop objects back onto the floor */
	while (q_len(queue) > 0) {
		/* Take object from the queue */
		obj = q_pop_ptr(queue);

		/* Drop the object */
		drop_near(cave, obj, 0, y, x, FALSE);
	}

	/* Reset cave feature and rune if needed */
	square_set_feat(cave, y, x, feat_old->fidx);
	if (glyph)
		square_add_ward(cave, y, x);

	q_free(queue);
}

/**
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(struct object *obj)
{
	/* Require staff/wand */
	if (!tval_can_have_charges(obj)) return;

	/* Require known item */
	if (!object_is_known(obj)) return;

	/* Print a message */
	msg("There %s %d charge%s remaining.", (obj->pval != 1) ? "are" : "is",
	     obj->pval, (obj->pval != 1) ? "s" : "");
}



/**
 * Get the indexes of objects at a given floor location. -TNB-
 *
 * Return the number of object indexes acquired.
 *
 * Valid flags are any combination of the bits:
 *   0x01 -- Verify item tester
 *   0x02 -- Marked items only
 *   0x04 -- Only the top item
 *   0x08 -- Visible items only
 */
int scan_floor(struct object **items, int max_size, int y, int x, int mode,
			   item_tester tester)
{
	struct object *obj;

	int num = 0;

	/* Sanity */
	if (!square_in_bounds(cave, y, x)) return 0;

	/* Scan all objects in the grid */
	for (obj = square_object(cave, y, x); obj; obj = obj->next) {
		/* Enforce limit */
		if (num >= max_size) break;

		/* Item tester */
		if ((mode & 0x01) && !object_test(tester, obj)) continue;

		/* Marked */
		if ((mode & 0x02) && (!obj->marked)) continue;

		/* Visible */
		if ((mode & 0x08) && !is_unknown(obj) && ignore_item_ok(obj))
			continue;

		/* Accept this item */
		items[num++] = obj;

		/* Only one */
		if (mode & 0x04) break;
	}

	return num;
}

/**
 * Get a list of "valid" objects.
 *
 * Fills item_list[] with items that are "okay" as defined by the
 * provided tester function, etc.  mode determines what combination of
 * inventory, equipment, quiver and player's floor location should be used
 * when drawing up the list.
 *
 * Returns the number of items placed into the list.
 *
 * Maximum space that can be used is
 * z_info->pack_size + z_info->quiver_size + player->body.count +
 * z_info->floor_size,
 * though practically speaking much smaller numbers are likely.
 */
int scan_items(struct object **item_list, size_t item_max, int mode,
			   item_tester tester)
{
	bool use_inven = ((mode & USE_INVEN) ? TRUE : FALSE);
	bool use_equip = ((mode & USE_EQUIP) ? TRUE : FALSE);
	bool use_quiver = ((mode & USE_QUIVER) ? TRUE : FALSE);
	bool use_floor = ((mode & USE_FLOOR) ? TRUE : FALSE);

	int floor_max = z_info->floor_size;
	struct object **floor_list = mem_zalloc(floor_max * sizeof(struct object *));
	int floor_num;

	int i;
	size_t item_num = 0;

	if (use_inven)
		for (i = 0; i < z_info->pack_size && item_num < item_max; i++) {
			if (object_test(tester, player->upkeep->inven[i]))
				item_list[item_num++] = player->upkeep->inven[i];
		}

	if (use_equip)
		for (i = 0; i < player->body.count && item_num < item_max; i++) {
			if (object_test(tester, slot_object(player, i)))
				item_list[item_num++] = slot_object(player, i);
		}

	if (use_quiver)
		for (i = 0; i < z_info->quiver_size && item_num < item_max; i++) {
			if (object_test(tester, player->upkeep->quiver[i]))
				item_list[item_num++] = player->upkeep->quiver[i];
		}

	/* Scan all non-gold objects in the grid */
	if (use_floor) {
		floor_num = scan_floor(floor_list, floor_max, player->py, player->px,
							   0x0B, tester);

		for (i = 0; i < floor_num && item_num < item_max; i++)
			item_list[item_num++] = floor_list[i];
	}

	mem_free(floor_list);
	return item_num;
}


/**
 * Check if the given item is available for the player to use.
 *
 * 'mode' defines which areas we should look at, a la scan_items().
 */
bool item_is_available(struct object *obj, bool (*tester)(const struct object *), int mode)
{
	int item_max = z_info->pack_size + z_info->quiver_size +
		player->body.count + z_info->floor_size;
	struct object **item_list = mem_zalloc(item_max * sizeof(struct object *));
	int item_num;
	int i;

	item_num = scan_items(item_list, item_max, mode, tester);

	for (i = 0; i < item_num; i++)
		if (item_list[i] == obj) {
			mem_free(item_list);
			return TRUE;
		}

	mem_free(item_list);
	return FALSE;
}

