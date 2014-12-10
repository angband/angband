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
#include "dungeon.h"
#include "effects.h"
#include "cmd-core.h"
#include "generate.h"
#include "grafmode.h"
#include "history.h"
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
#include "obj-ui.h"
#include "obj-util.h"
#include "player-spell.h"
#include "player-util.h"
#include "prefs.h"
#include "randname.h"
#include "z-queue.h"

struct object *pile_last_item(struct object *start)
{
	struct object *obj = start;

	/* No pile at all */
	if (!obj)
		return NULL;

	/* Run along the list, stopping just before the end */
	while (obj->next)
		obj = obj->next;

	return obj;
}

bool object_in_pile(struct object *top, struct object *obj)
{
	struct object *pile_obj = top;

	while (pile_obj) {
		if (obj == pile_obj)
			return TRUE;
		pile_obj = pile_obj->next;
	}

	return FALSE;
}

/**
 * Excise an object from a floor pile, leaving it orphaned.

 * Code using this function must then deal with the orphaned object in some
 * way - usually by deleting it, or adding it to a player, monster or store
 * inventory.
 */
bool pile_object_excise(struct chunk *c, int y, int x, struct object *obj)
{
	struct object *current = square_object(c, y, x);

	/* Special case - excise top object */
	if (current == obj) {
		c->squares[y][x].obj = obj->next;
		if (obj->next)
			(obj->next)->prev = NULL;
		obj->next = NULL;
		obj->prev = NULL;
		return TRUE;
	}

	/* Otherwise find the object... */
	while (current != obj) {
		current = current->next;

		/* Object isn't in the pile */
		if (!current)
			return FALSE;
	}

	/* ...and remove it */
	(obj->prev)->next = obj->next;
	if (obj->next)
		(obj->next)->prev = obj->prev;
	obj->next = NULL;
	obj->prev = NULL;
	return TRUE;
}

/**
 * Create a new object and return it
 */
struct object *object_new(void)
{
	return mem_zalloc(sizeof(struct object));
}

/**
 * Delete an object and free its memory
 */
void object_delete(struct object *obj)
{
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
}

/**
 * Free an entire object pile
 */
void object_pile_free(struct object *obj)
{
	struct object *current = obj, *next;

	while (current) {
		next = current->next;
		object_delete(current);
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
bool object_stackable(const struct object *o_ptr, const struct object *j_ptr,
					  object_stack_t mode)
{
	int i;

	/* Equipment items don't stack */
	if (object_is_equipped(player->body, o_ptr))
		return FALSE;
	if (object_is_equipped(player->body, j_ptr))
		return FALSE;

	/* If either item is unknown, do not stack */
	if (mode & OSTACK_LIST && o_ptr->marked == MARK_AWARE) return FALSE;
	if (mode & OSTACK_LIST && j_ptr->marked == MARK_AWARE) return FALSE;

	/* Hack -- identical items cannot be stacked */
	if (o_ptr == j_ptr) return FALSE;

	/* Require identical object kinds */
	if (o_ptr->kind != j_ptr->kind) return FALSE;

	/* Different flags don't stack */
	if (!of_is_equal(o_ptr->flags, j_ptr->flags)) return FALSE;

	/* Artifacts never stack */
	if (o_ptr->artifact || j_ptr->artifact) return FALSE;

	/* Analyze the items */
	if (tval_is_chest(o_ptr)) {
		/* Chests never stack */
		return FALSE;
	}
	else if (tval_is_food(o_ptr) || tval_is_potion(o_ptr) ||
		tval_is_scroll(o_ptr) || tval_is_rod(o_ptr)) {
		/* Food, potions, scrolls and rods all stack nicely,
		   since the kinds are identical, either both will be
		   aware or both will be unaware */
	} else if (tval_can_have_charges(o_ptr) || tval_is_money(o_ptr)) {
		/* Gold, staves and wands stack most of the time */
		/* Too much gold or too many charges */
		if (o_ptr->pval + j_ptr->pval > MAX_PVAL)
			return FALSE;

		/* ... otherwise ok */
	} else if (tval_is_weapon(o_ptr) || tval_is_armor(o_ptr) ||
		tval_is_jewelry(o_ptr) || tval_is_light(o_ptr)) {
		bool o_is_known = object_is_known(o_ptr);
		bool j_is_known = object_is_known(j_ptr);

		/* Require identical values */
		if (o_ptr->ac != j_ptr->ac) return FALSE;
		if (o_ptr->dd != j_ptr->dd) return FALSE;
		if (o_ptr->ds != j_ptr->ds) return FALSE;

		/* Require identical bonuses */
		if (o_ptr->to_h != j_ptr->to_h) return FALSE;
		if (o_ptr->to_d != j_ptr->to_d) return FALSE;
		if (o_ptr->to_a != j_ptr->to_a) return FALSE;

		/* Require all identical modifiers */
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (o_ptr->modifiers[i] != j_ptr->modifiers[i])
				return (FALSE);

		/* Require identical ego-item types */
		if (o_ptr->ego != j_ptr->ego) return FALSE;

		/* Hack - Never stack recharging wearables ... */
		if ((o_ptr->timeout || j_ptr->timeout) &&
			!tval_is_light(o_ptr)) return FALSE;

		/* ... and lights must have same amount of fuel */
		else if ((o_ptr->timeout != j_ptr->timeout) &&
				 tval_is_light(o_ptr)) return FALSE;

		/* Prevent unIDd items stacking with IDd items in the object list */
		if (mode & OSTACK_LIST && (o_is_known != j_is_known)) return FALSE;
	} else {
		/* Anything else probably okay */
	}

	/* Require compatible inscriptions */
	if (o_ptr->note && j_ptr->note && (o_ptr->note != j_ptr->note))
		return FALSE;

	/* They must be similar enough */
	return TRUE;
}

/**
 * Return whether each stack of objects can be merged into one stack.
 */
bool object_similar(const struct object *o_ptr, const struct object *j_ptr,
					object_stack_t mode)
{
	int total = o_ptr->number + j_ptr->number;

	/* Check against stacking limit - except in stores which absorb anyway */
	if (!(mode & OSTACK_STORE) && (total > z_info->stack_size))
		return FALSE;

	return object_stackable(o_ptr, j_ptr, mode);
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
static void object_absorb_merge(struct object *o_ptr, const struct object *j_ptr)
{
	int total;

	/* Blend all knowledge */
	of_union(o_ptr->known_flags, j_ptr->known_flags);

	/* Merge inscriptions */
	if (j_ptr->note)
		o_ptr->note = j_ptr->note;

	/* Combine timeouts for rod stacking */
	if (tval_can_have_timeout(o_ptr))
		o_ptr->timeout += j_ptr->timeout;

	/* Combine pvals for wands and staves */
	if (tval_can_have_charges(o_ptr) || tval_is_money(o_ptr)) {
		total = o_ptr->pval + j_ptr->pval;
		o_ptr->pval = total >= MAX_PVAL ? MAX_PVAL : total;
	}

	/* Combine origin data as best we can */
	if (o_ptr->origin != j_ptr->origin ||
		o_ptr->origin_depth != j_ptr->origin_depth ||
		o_ptr->origin_xtra != j_ptr->origin_xtra) {
		int act = 2;

		if (o_ptr->origin_xtra && j_ptr->origin_xtra) {
			monster_race *r_ptr = &r_info[o_ptr->origin_xtra];
			monster_race *s_ptr = &r_info[j_ptr->origin_xtra];

			bool r_uniq = rf_has(r_ptr->flags, RF_UNIQUE) ? TRUE : FALSE;
			bool s_uniq = rf_has(s_ptr->flags, RF_UNIQUE) ? TRUE : FALSE;

			if (r_uniq && !s_uniq) act = 0;
			else if (s_uniq && !r_uniq) act = 1;
			else act = 2;
		}

		switch (act)
		{
				/* Overwrite with j_ptr */
			case 1:
			{
				o_ptr->origin = j_ptr->origin;
				o_ptr->origin_depth = j_ptr->origin_depth;
				o_ptr->origin_xtra = j_ptr->origin_xtra;
			}

				/* Set as "mixed" */
			case 2:
			{
				o_ptr->origin = ORIGIN_MIXED;
			}
		}
	}
}

/**
 * Merge a smaller stack into a larger stack, leaving two uneven stacks.
 */
void object_absorb_partial(struct object *o_ptr, struct object *j_ptr)
{
	int smallest = MIN(o_ptr->number, j_ptr->number);
	int largest = MAX(o_ptr->number, j_ptr->number);
	int difference = (z_info->stack_size) - largest;
	o_ptr->number = largest + difference;
	j_ptr->number = smallest - difference;

	object_absorb_merge(o_ptr, j_ptr);
}

/**
 * Merge two stacks into one stack.
 */
void object_absorb(struct object *o_ptr, struct object *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Add together the item counts */
	o_ptr->number = (total < z_info->stack_size ? total : z_info->stack_size);

	object_absorb_merge(o_ptr, j_ptr);
	object_delete(j_ptr);
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
 * Split off 'amt' items from 'src' into 'dest'.
 *
 * Where object_copy_amt() makes `amt` new objects, this function leaves the
 * total number unchanged; otherwise the two functions are similar.
 */
void object_split(struct object *dest, struct object *src, int amt)
{
	/* Get a copy of the object */
	object_copy(dest, src);

	/* Check legality */
	if (src->number < amt)
		amt = src->number;

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(src, dest, amt);

	/* Modify quantity */
	dest->number = amt;
	src->number -= amt;
	if (src->note)
		dest->note = src->note;
}

/**
 * Remove an amount of an object from the floor, returning a detached object
 * which can be used - it is assumed that the object is on the player grid.
 *
 * Optionally describe what remains.
 */
struct object *floor_object_for_use(struct object *obj, int num, bool message)
{
	struct object *usable;
	char name[80];

	/* Bounds check */
	num = MAX(num, obj->number);

	/* Split off a usable object if necessary */
	if (obj->number > num) {
		usable = mem_zalloc(sizeof(*usable));
		object_split(usable, obj, num);
	} else {
		usable = obj;
		pile_object_excise(cave, player->py, player->px, usable);
	}

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS | PU_MANA | PU_INVEN);
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Print a message if requested and there is anything left */
	if (message && (usable != obj)) {
		/* Get a description */
		object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);

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

	/* Option -- disallow stacking */
	if (OPT(birth_no_stacking) && n) return (0);

	/* The stack is already too large */
	if (n >= z_info->floor_size) {
		/* Delete the oldest ignored object */
		struct object *ignore = floor_get_oldest_ignored(y, x);

		if (ignore) {
			pile_object_excise(c, y, x, ignore);
			object_delete(ignore);
		} else
			return FALSE;
	}

	/* Location */
	drop->iy = y;
	drop->ix = x;

	/* Forget monster */
	drop->held_m_idx = 0;

	/* Link to the first or last object in the pile */
	if (last) {
		obj = pile_last_item(square_object(c, y, x));
		drop->next = NULL;
		drop->prev = obj;
		if (obj)
			obj->next = drop;
		else
			c->squares[y][x].obj = drop;
	} else {
		drop->next = square_object(c, y, x);
		c->squares[y][x].obj = drop;
	}

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
void drop_near(struct chunk *c, struct object *j_ptr, int chance, int y, int x,
			   bool verbose)
{
	int i, k, n, d, s;

	int bs, bn;
	int by, bx;
	int dy, dx;
	int ty, tx;

	struct object *o_ptr;

	char o_name[80];

	bool flag = FALSE;

	/* Describe object */
	object_desc(o_name, sizeof(o_name), j_ptr, ODESC_BASE);

	/* Handle normal "breakage" */
	if (!j_ptr->artifact && (randint0(100) < chance)) {
		/* Message */
		msg("The %s %s.", o_name,
			VERB_AGREEMENT(j_ptr->number, "breaks", "break"));

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

			/* No objects */
			k = 0;
			n = 0;

			/* Scan objects in that grid */
			for (o_ptr = square_object(c, ty, tx); o_ptr; o_ptr = o_ptr->next) {
				/* Check for possible combination */
				if (object_similar(o_ptr, j_ptr, OSTACK_FLOOR))
					comb = TRUE;

				/* Count objects */
				if (!ignore_item_ok(o_ptr))
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
	if (!flag && !j_ptr->artifact) {
		/* Message */
		msg("The %s %s.", o_name,
			VERB_AGREEMENT(j_ptr->number, "disappears", "disappear"));

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
	if (!floor_carry(c, by, bx, j_ptr, FALSE)) {
		/* Message */
		msg("The %s %s.", o_name,
			VERB_AGREEMENT(j_ptr->number, "disappears", "disappear"));

		/* Debug */
		if (player->wizard) msg("Breakage (too many objects).");

		if (j_ptr->artifact) j_ptr->artifact->created = FALSE;

		/* Failure */
		return;
	}

	/* Sound */
	sound(MSG_DROP);

	/* Message when an object falls under the player */
	if (verbose && (cave->squares[by][bx].mon < 0) && !ignore_item_ok(j_ptr))
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

	struct object *obj;

	struct queue *queue = q_new(z_info->floor_size);

	/* Push all objects on the square into the queue */
	for (obj = square_object(cave, y, x); obj; obj = obj->next)
		q_push_ptr(queue, obj);

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

	/* Reset cave feature */
	square_set_feat(cave, y, x, feat_old->fidx);

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

