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
#include "mon-util.h"
#include "monster.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-knowledge.h"
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
#include "trap.h"
#include "z-queue.h"

/* #define LIST_DEBUG */

static struct object *fail_pile;
static struct object *fail_object;
static bool fail_prev;
static bool fail_next;
static char *fail_file;
static int fail_line;

void write_pile(ang_file *fff)
{
	file_putf(fff, "Pile integrity failure at %s:%d\n\n", fail_file, fail_line);
	file_putf(fff, "Guilty object\n=============\n");
	if (fail_object && fail_object->kind) {
		file_putf(fff, "Name: %s\n", fail_object->kind->name);
		if (fail_prev) {
			file_putf(fff, "Previous: ");
			if (fail_object->prev && fail_object->prev->kind) {
				file_putf(fff, "%s\n", fail_object->prev->kind->name);
			} else {
				file_putf(fff, "bad object\n");
			}
		}
		if (fail_next) {
			file_putf(fff, "Next: ");
			if (fail_object->next && fail_object->next->kind) {
				file_putf(fff, "%s\n", fail_object->next->kind->name);
			} else {
				file_putf(fff, "bad object\n");
			}
		}
		file_putf(fff, "\n");
	}
	if (fail_pile) {
		file_putf(fff, "Guilty pile\n=============\n");
		while (fail_pile) {
			if (fail_pile->kind) {
				file_putf(fff, "Name: %s\n", fail_pile->kind->name);
			} else {
				file_putf(fff, "bad object\n");
			}
			fail_pile = fail_pile->next;
		}
	}
}

/**
 * Quit on getting an object pile error, writing a diagnosis file
 */
void pile_integrity_fail(struct object *pile, struct object *obj, char *file,
						 int line)
{
	char path[1024];

	/* Set the pile info to write out */
	fail_pile = pile;
	fail_object = obj;
	fail_prev = (obj->prev != NULL);
	fail_next = (obj->next != NULL);
	fail_file = file;
	fail_line = line;

	/* Write to the user directory */
	path_build(path, sizeof(path), ANGBAND_DIR_USER, "pile_error.txt");

	if (text_lines_to_file(path, write_pile)) {
		quit_fmt("Failed to create file %s.new", path);
	}
	quit_fmt("Pile integrity failure, details written to %s", path);
}

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

		if (obj->prev != prev) {
			pile_integrity_fail(pile, obj, __FILE__, __LINE__);
		}
		prev = obj;
		obj = obj->next;
	};

	/* Check for circularity */
	for (obj = pile; obj; obj = obj->next) {
		struct object *check;
		for (check = obj->next; check; check = check->next) {
			if (check->next == obj) {
				pile_integrity_fail(pile, check, __FILE__, __LINE__);
			}
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
	if (obj->prev || obj->next) {
		pile_integrity_fail(NULL, obj, __FILE__, __LINE__);
	}

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
	if (obj->prev) {
		pile_integrity_fail(NULL, obj, __FILE__, __LINE__);
	}

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

	if (!pile_contains(*pile, obj)) {
		pile_integrity_fail(*pile, obj, __FILE__, __LINE__);
	}
	pile_check_integrity("excise [pre]", *pile, obj);

	/* Special case: unlink top object */
	if (*pile == obj) {
		if (prev) {
			pile_integrity_fail(*pile, obj, __FILE__, __LINE__);
		}

		*pile = next;
	} else {
		if (obj->prev == NULL) {
			pile_integrity_fail(*pile, obj, __FILE__, __LINE__);
		}

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
			return true;
		pile_obj = pile_obj->next;
	}

	return false;
}

/**
 * Create a new object and return it
 */
struct object *object_new(void)
{
	return mem_zalloc(sizeof(struct object));
}

/**
 * Free up an object
 *
 * This doesn't affect any game state outside of the object itself
 */
void object_free(struct object *obj)
{
	mem_free(obj->slays);
	mem_free(obj->brands);
	mem_free(obj->curses);
	mem_free(obj);
}

/**
 * Delete an object and free its memory, and set its pointer to NULL
 */
void object_delete(struct object **obj_address)
{
	struct object *obj = *obj_address;
	struct object *prev = obj->prev;
	struct object *next = obj->next;

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

	/* Orphan rather than actually delete if we still have a known object */
	if (cave && player && player->cave && obj->oidx &&
		(obj == cave->objects[obj->oidx]) &&
		player->cave->objects[obj->oidx]) {
		obj->grid = loc(0, 0);
		obj->held_m_idx = 0;
		obj->mimicking_m_idx = 0;

		/* Object is now purely imaginary to the player */
		obj->known->notice |= OBJ_NOTICE_IMAGINED;

		return;
	}

	/* Remove from any lists */
	if (player && player->cave && player->cave->objects && obj->oidx
		&& (obj == player->cave->objects[obj->oidx]))
		player->cave->objects[obj->oidx] = NULL;

	if (cave && cave->objects && obj->oidx
		&& (obj == cave->objects[obj->oidx]))
		cave->objects[obj->oidx] = NULL;

	object_free(obj);
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
		return false;
	if (object_is_equipped(player->body, obj2))
		return false;

	/* If either item is unknown, do not stack */
	if (mode & OSTACK_LIST && obj1->kind != obj1->known->kind) return false;
	if (mode & OSTACK_LIST && obj2->kind != obj2->known->kind) return false;

	/* Hack -- identical items cannot be stacked */
	if (obj1 == obj2) return false;

	/* Require identical object kinds */
	if (obj1->kind != obj2->kind) return false;

	/* Different flags don't stack */
	if (!of_is_equal(obj1->flags, obj2->flags)) return false;

	/* Different elements don't stack */
	for (i = 0; i < ELEM_MAX; i++) {
		if (obj1->el_info[i].res_level != obj2->el_info[i].res_level)
			return false;
		if ((obj1->el_info[i].flags & (EL_INFO_HATES | EL_INFO_IGNORE)) !=
			(obj2->el_info[i].flags & (EL_INFO_HATES | EL_INFO_IGNORE)))
			return false;
	}

	/* Artifacts never stack */
	if (obj1->artifact || obj2->artifact) return false;

	/* Analyze the items */
	if (tval_is_chest(obj1)) {
		/* Chests never stack */
		return false;
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
			return false;

		/* ... otherwise ok */
	} else if (tval_is_weapon(obj1) || tval_is_armor(obj1) ||
		tval_is_jewelry(obj1) || tval_is_light(obj1)) {
		bool obj1_is_known = object_fully_known((struct object *)obj1);
		bool obj2_is_known = object_fully_known((struct object *)obj2);

		/* Require identical values */
		if (obj1->ac != obj2->ac) return false;
		if (obj1->dd != obj2->dd) return false;
		if (obj1->ds != obj2->ds) return false;

		/* Require identical bonuses */
		if (obj1->to_h != obj2->to_h) return false;
		if (obj1->to_d != obj2->to_d) return false;
		if (obj1->to_a != obj2->to_a) return false;

		/* Require all identical modifiers */
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (obj1->modifiers[i] != obj2->modifiers[i])
				return (false);

		/* Require identical ego-item types */
		if (obj1->ego != obj2->ego) return false;

		/* Require identical curses */
		if (!curses_are_equal(obj1, obj2)) return false;

		/* Hack - Never stack recharging wearables ... */
		if ((obj1->timeout || obj2->timeout) &&
			!tval_is_light(obj1)) return false;

		/* ... and lights must have same amount of fuel */
		else if ((obj1->timeout != obj2->timeout) &&
				 tval_is_light(obj1)) return false;

		/* Prevent unIDd items stacking with IDd items in the object list */
		if (mode & OSTACK_LIST && (obj1_is_known != obj2_is_known))
			return false;
	} else {
		/* Anything else probably okay */
	}

	/* Require compatible inscriptions */
	if (obj1->note && obj2->note && (obj1->note != obj2->note))
		return false;

	/* They must be similar enough */
	return true;
}

/**
 * Return whether each stack of objects can be merged into one stack.
 */
bool object_similar(const struct object *obj1, const struct object *obj2,
					object_stack_t mode)
{
	int total = obj1->number + obj2->number;

	/* Check against stacking limit - except in stores which absorb anyway */
	if (!(mode & OSTACK_STORE) && (total > obj1->kind->base->max_stack))
		return false;

	return object_stackable(obj1, obj2, mode);
}

/**
 * Combine the origins of two objects
 */
void object_origin_combine(struct object *obj1, const struct object *obj2)
{
	int act = 2;

	if (obj1->origin_race && obj2->origin_race) {
		bool uniq1 = false;
		bool uniq2 = false;

		if (obj1->origin_race) {
			uniq1 = rf_has(obj1->origin_race->flags, RF_UNIQUE) ? true : false;
		}
		if (obj2->origin_race) {
			uniq2 = rf_has(obj2->origin_race->flags, RF_UNIQUE) ? true : false;
		}

		if (uniq1 && !uniq2) act = 0;
		else if (uniq2 && !uniq1) act = 1;
		else act = 2;
	}

	switch (act)
	{
		/* Overwrite with obj2 */
		case 1:
		{
			obj1->origin = obj2->origin;
			obj1->origin_depth = obj2->origin_depth;
			obj1->origin_race = obj2->origin_race;
			break;
		}

		/* Set as "mixed" */
		case 2:
		{
			obj1->origin = ORIGIN_MIXED;
			break;
		}
		default: break;
	}
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

	/* First object gains any extra knowledge from second */
	if (obj1->known && obj2->known) {
		if (obj2->known->effect)
			obj1->known->effect = obj1->effect;
		player_know_object(player, obj1);
	}

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
	object_origin_combine(obj1, obj2);
}

/**
 * Merge a smaller stack into a larger stack, leaving two uneven stacks.
 */
void object_absorb_partial(struct object *obj1, struct object *obj2)
{
	int smallest = MIN(obj1->number, obj2->number);
	int largest = MAX(obj1->number, obj2->number);
	int difference = obj1->kind->base->max_stack - largest;
	obj1->number = largest + difference;
	obj2->number = smallest - difference;

	object_absorb_merge(obj1, obj2);
}

/**
 * Merge two stacks into one stack.
 */
void object_absorb(struct object *obj1, struct object *obj2)
{
	struct object *known = obj2->known;
	int total = obj1->number + obj2->number;

	/* Add together the item counts */
	obj1->number = MIN(total, obj1->kind->base->max_stack);

	object_absorb_merge(obj1, obj2);
	if (known) {
		if (!loc_is_zero(known->grid)) {
			square_excise_object(player->cave, known->grid, known);
		}
		delist_object(player->cave, known);
		object_delete(&known);
	}
	object_delete(&obj2);
}

/**
 * Wipe an object clean.
 */
void object_wipe(struct object *obj)
{
	/* Free slays and brands */
	mem_free(obj->slays);
	mem_free(obj->brands);
	mem_free(obj->curses);

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

	if (src->slays) {
		dest->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
		memcpy(dest->slays, src->slays, z_info->slay_max * sizeof(bool));
	}
	if (src->brands) {
		dest->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
		memcpy(dest->brands, src->brands, z_info->brand_max * sizeof(bool));
	}
	if (src->curses) {
		size_t array_size = z_info->curse_max * sizeof(struct curse_data);
		dest->curses = mem_zalloc(array_size);
		memcpy(dest->curses, src->curses, array_size);
	}

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
	struct object *dest = object_new(), *dest_known;

	/* Get a copy of the object */
	object_copy(dest, src);

	/* Do we need a new known object? */
	if (src->known) {
		/* Ensure numbers are aligned (should not be necessary, but safer) */
		src->known->number = src->number;

		/* Make the new object */
		dest_known = object_new();
		object_copy(dest_known, src->known);
		dest->known = dest_known;
	}

	/* Check legality */
	assert(src->number > amt);

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(src, dest, amt);
	if (src->known)
		distribute_charges(src->known, dest->known, amt);

	/* Modify quantity */
	dest->number = amt;
	src->number -= amt;
	if (src->note)
		dest->note = src->note;
	if (src->known) {
		dest->known->number = dest->number;
		src->known->number = src->number;
		dest->known->note = src->known->note;
	}

	/* Remove any index */
	if (dest->known)
		dest->known->oidx = 0;
	dest->oidx = 0;

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
		square_excise_object(player->cave, usable->grid, usable->known);
		delist_object(player->cave, usable->known);
		square_excise_object(cave, usable->grid, usable);
		delist_object(cave, usable);
		*none_left = true;

		/* Stop tracking item */
		if (tracked_object_is(player->upkeep, obj))
			track_object(player->upkeep, NULL);

		/* Inventory has changed, so disable repeat command */ 
		cmd_disable_repeat();
	}

	/* Object no longer has a location */
	usable->known->grid = loc(0, 0);
	usable->grid = loc(0, 0);

	/* Housekeeping */
	player->upkeep->update |= (PU_BONUS | PU_INVEN);
	player->upkeep->notice |= (PN_COMBINE);
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Print a message if requested and there is anything left */
	if (message) {
		if (usable == obj)
			obj->number = 0;

		/* Get a description */
		object_desc(name, sizeof(name), obj, ODESC_PREFIX | ODESC_FULL);

		if (usable == obj)
			obj->number = num;

		/* Print a message */
		msg("You see %s.", name);
	}

	return usable;
}


/**
 * Find and return the oldest object on the given grid marked as "ignore".
 */
static struct object *floor_get_oldest_ignored(struct chunk *c, struct loc grid)
{
	struct object *obj, *ignore = NULL;

	for (obj = square_object(c, grid); obj; obj = obj->next)
		if (ignore_item_ok(obj))
			ignore = obj;

	return ignore;
}


/**
 * Let the floor carry an object, deleting old ignored items if necessary.
 * The calling function must deal with the dropped object on failure.
 *
 * Optionally put the object at the top or bottom of the pile
 */
bool floor_carry(struct chunk *c, struct loc grid, struct object *drop,
				 bool *note)
{
	int n = 0;
	struct object *obj, *ignore = floor_get_oldest_ignored(c, grid);

	/* Fail if the square can't hold objects */
	if (!square_isobjectholding(c, grid))
		return false;

	/* Scan objects in that grid for combination */
	for (obj = square_object(c, grid); obj; obj = obj->next) {
		/* Check for combination */
		if (object_similar(obj, drop, OSTACK_FLOOR)) {
			/* Combine the items */
			object_absorb(obj, drop);

			/* Note the pile */
			if (square_isview(c, grid)) {
				square_note_spot(c, grid);
			}

			/* Don't mention if ignored */
			if (ignore_item_ok(obj)) {
				*note = false;
			}

			/* Result */
			return true;
		}

		/* Count objects */
		n++;
	}

	/* The stack is already too large */
	if (n >= z_info->floor_size || (!OPT(player, birth_stacking) && n)) {
		/* Delete the oldest ignored object */
		if (ignore) {
			square_excise_object(c, grid, ignore);
			delist_object(c, ignore);
			object_delete(&ignore);
		} else {
			return false;
		}
	}

	/* Location */
	drop->grid = grid;

	/* Forget monster */
	drop->held_m_idx = 0;

	/* Link to the first object in the pile */
	pile_insert(&c->squares[grid.y][grid.x].obj, drop);

	/* Record in the level list */
	list_object(c, drop);

	/* If there's a known version, put it in the player's view of the
	 * cave but at an unknown location.  square_note_spot() will move
	 * it to the correct place if seen. */
	if (drop->known) {
		drop->known->oidx = drop->oidx;
		drop->known->held_m_idx = 0;
		drop->known->grid = loc(0, 0);
		player->cave->objects[drop->oidx] = drop->known;
	}

	/* Redraw */
	square_note_spot(c, grid);
	square_light_spot(c, grid);

	/* Don't mention if ignored */
	if (ignore_item_ok(drop)) {
		*note = false;
	}

	/* Result */
	return true;
}

/**
 * Delete an object when the floor fails to carry it, and attempt to remove
 * it from the object list
 */
static void floor_carry_fail(struct object *drop, bool broke)
{
	struct object *known = drop->known;

	/* Delete completely */
	if (known) {
		char o_name[80];
		char *verb = broke ? VERB_AGREEMENT(drop->number, "breaks", "break")
			: VERB_AGREEMENT(drop->number, "disappears", "disappear");
		object_desc(o_name, sizeof(o_name), drop, ODESC_BASE);
		msg("The %s %s.", o_name, verb);
		if (!loc_is_zero(known->grid))
			square_excise_object(player->cave, known->grid, known);
		delist_object(player->cave, known);
		object_delete(&known);
	}
	delist_object(cave, drop);
	object_delete(&drop);
}

/**
 * Find a grid near the given one for an object to fall on
 *
 * We check several locations to see if we can find a location at which
 * the object can combine, stack, or be placed.  Artifacts will try very
 * hard to be placed, including "teleporting" to a useful grid if needed.
 *
 * If no appropriate grid is found, the given grid is unchanged
 */
static void drop_find_grid(struct object *drop, struct loc *grid)
{
	int best_score = -1;
	struct loc start = *grid;
	struct loc best = start;
	int i, dy, dx;
	struct object *obj;

	/* Scan local grids */
	for (dy = -3; dy <= 3; dy++) {
		for (dx = -3; dx <= 3; dx++) {
			bool combine = false;
			int dist = (dy * dy) + (dx * dx);
			struct loc try = loc_sum(start, loc(dx, dy));
			int num_shown = 0;
			int num_ignored = 0;
			int score;

			/* Lots of reasons to say no */
			if ((dist > 10) ||
				!square_in_bounds_fully(cave, try) ||
				!los(cave, start, try) ||
				!square_isfloor(cave, try) ||
				square_istrap(cave, try))
				continue;

			/* Analyse the grid for carrying the new object */
			for (obj = square_object(cave, try); obj; obj = obj->next){
				/* Check for possible combination */
				if (object_similar(obj, drop, OSTACK_FLOOR))
					combine = true;

				/* Count objects */
				if (!ignore_item_ok(obj))
					num_shown++;
				else
					num_ignored++;
			}
			if (!combine)
				num_shown++;

			/* Disallow if the stack size is too big */
			if ((!OPT(player, birth_stacking) && (num_shown > 1)) ||
				((num_shown + num_ignored) > z_info->floor_size &&
				 !floor_get_oldest_ignored(cave, try)))
				continue;

			/* Score the location based on how close and how full the grid is */
			score = 1000 - (dist + num_shown * 5);

			if ((score < best_score) || ((score == best_score) && one_in_(2)))
				continue;

			best_score = score;
			best = try;
		}
	}

	/* Return if we have a score, otherwise fail or try harder for artifacts */
	if (best_score >= 0) {
		*grid = best;
		return;
	} else if (!drop->artifact) {
		return;
	}
	for (i = 0; i < 2000; i++) {
		/* Start bouncing from grid to grid, stopping if we find an empty one */
		if (i < 1000) {
			best = rand_loc(best, 1, 1);
		} else {
			/* Now go to purely random locations */
			best = loc(randint0(cave->width), randint0(cave->height));
		}
		if (square_canputitem(cave, best)) {
			*grid = best;
			return;
		}
	}
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
 * The calling function needs to deal with the consequences of the dropped
 * object being destroyed or absorbed into an existing pile.
 */
void drop_near(struct chunk *c, struct object **dropped, int chance,
			   struct loc grid, bool verbose)
{
	char o_name[80];
	struct loc best = grid;
	bool dont_ignore = verbose && !ignore_item_ok(*dropped);

	/* Only called in the current level */
	assert(c == cave);

	/* Describe object */
	object_desc(o_name, sizeof(o_name), *dropped, ODESC_BASE);

	/* Handle normal breakage */
	if (!((*dropped)->artifact) && (randint0(100) < chance)) {
		floor_carry_fail(*dropped, true);
		return;
	}

	/* Find the best grid and drop the item, destroying if there's no space */
	drop_find_grid(*dropped, &best);
	if (floor_carry(c, best, *dropped, &dont_ignore)) {
		sound(MSG_DROP);
		if (dont_ignore && (square(c, best).mon < 0)) {
			msg("You feel something roll beneath your feet.");
		}
	} else {
		floor_carry_fail(*dropped, false);
	}
}

/**
 * This will push objects off a square.
 *
 * The methodology is to load all objects on the square into a queue. Replace
 * the previous square with a type that does not allow for objects. Drop the
 * objects. Last, put the square back to its original type.
 */
void push_object(struct loc grid)
{
	/* Save the original terrain feature */
	struct feature *feat_old = square_feat(cave, grid);
	struct object *obj = square_object(cave, grid);
	struct queue *queue = q_new(z_info->floor_size);
	struct trap *trap = square_trap(cave, grid);

	/* Push all objects on the square, stripped of pile info, into the queue */
	while (obj) {
		struct object *next = obj->next;
		/* In case the object is known, make a copy to work with
		 * and try to delete the original which will orphan it to
		 * serve as a placeholder for the known version. */
		struct object *newobj = object_new();

		object_copy(newobj, obj);
		newobj->oidx = 0;
		newobj->grid = loc(0, 0);
		if (newobj->known) {
			newobj->known = object_new();
			object_copy(newobj->known, obj->known);
			newobj->known->oidx = 0;
			newobj->known->grid = loc(0, 0);
		}
		q_push_ptr(queue, newobj);

		delist_object(cave, obj);
		object_delete(&obj);

		/* Next object */
		obj = next;
	}

	/* Disassociate the objects from the square */
	square_set_obj(cave, grid, NULL);

	/* Set feature to an open door */
	square_force_floor(cave, grid);
	square_add_door(cave, grid, false);

	/* Drop objects back onto the floor */
	while (q_len(queue) > 0) {
		/* Take object from the queue */
		obj = q_pop_ptr(queue);

		/* Drop the object */
		drop_near(cave, &obj, 0, grid, false);
	}

	/* Reset cave feature, remove trap if needed */
	square_set_feat(cave, grid, feat_old->fidx);
	if (trap && !square_istrappable(cave, grid)) {
		square_destroy_trap(cave, grid);
	}

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
	if (!object_flavor_is_aware(obj)) return;

	/* Print a message */
	msg("There %s %d charge%s remaining.", (obj->pval != 1) ? "are" : "is",
	     obj->pval, (obj->pval != 1) ? "s" : "");
}



/**
 * Get a list of the objects at the player's location.
 *
 * Return the number of objects acquired.
 */
int scan_floor(struct object **items, int max_size, object_floor_t mode,
			   item_tester tester)
{
	struct object *obj;
	int num = 0;

	/* Sanity */
	if (!square_in_bounds(cave, player->grid)) return 0;

	/* Scan all objects in the grid */
	for (obj = square_object(cave, player->grid); obj; obj = obj->next) {
		/* Enforce limit */
		if (num >= max_size) break;

		/* Item tester */
		if ((mode & OFLOOR_TEST) && !object_test(tester, obj)) continue;

		/* Sensed or known */
		if ((mode & OFLOOR_SENSE) && (!obj->known)) continue;

		/* Visible */
		if ((mode & OFLOOR_VISIBLE) && !is_unknown(obj) && ignore_item_ok(obj))
			continue;

		/* Accept this item */
		items[num++] = obj;

		/* Only one */
		if (mode & OFLOOR_TOP) break;
	}

	return num;
}

/**
 * Get a list of the known objects at the given location.
 *
 * Return the number of objects acquired.
 */
int scan_distant_floor(struct object **items, int max_size, struct loc grid)
{
	struct object *obj;
	int num = 0;

	/* Sanity */
	if (!square_in_bounds(player->cave, grid)) return 0;

	/* Scan all objects in the grid */
	for (obj = square_object(player->cave, grid); obj; obj = obj->next) {
		/* Enforce limit */
		if (num >= max_size) break;

		/* Known */
		if (obj->kind == unknown_item_kind) continue;

		/* Visible */
		if (ignore_known_item_ok(obj)) continue;

		/* Accept this item's base object */
		items[num++] = cave->objects[obj->oidx];
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
	bool use_inven = ((mode & USE_INVEN) ? true : false);
	bool use_equip = ((mode & USE_EQUIP) ? true : false);
	bool use_quiver = ((mode & USE_QUIVER) ? true : false);
	bool use_floor = ((mode & USE_FLOOR) ? true : false);

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
		floor_num = scan_floor(floor_list, floor_max,
							   OFLOOR_TEST | OFLOOR_SENSE | OFLOOR_VISIBLE,
							   tester);

		for (i = 0; i < floor_num && item_num < item_max; i++)
			item_list[item_num++] = floor_list[i];
	}

	mem_free(floor_list);
	return item_num;
}


/**
 * Check if the given item is available for the player to use.
 */
bool item_is_available(struct object *obj)
{
	if (object_is_carried(player, obj)) return true;
	if (cave && square_holds_object(cave, player->grid, obj))
		return true;
	return false;
}
