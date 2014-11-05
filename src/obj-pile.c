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
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-ui.h"
#include "obj-util.h"
#include "player-spell.h"
#include "player-util.h"
#include "prefs.h"
#include "randname.h"
#include "z-queue.h"

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
int scan_floor(int *items, int max_size, int y, int x, int mode,
			   item_tester tester)
{
	int this_o_idx, next_o_idx;

	int num = 0;

	/* Sanity */
	if (!square_in_bounds(cave, y, x)) return 0;

	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* XXX Hack -- Enforce limit */
		if (num >= max_size) break;

		/* Get the object */
		o_ptr = cave_object(cave, this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Item tester */
		if ((mode & 0x01) && !object_test(tester, o_ptr)) continue;

		/* Marked */
		if ((mode & 0x02) && (!o_ptr->marked)) continue;

		/* Visible */
		if ((mode & 0x08) && !is_unknown(o_ptr) && ignore_item_ok(o_ptr))
			continue;

		/* Accept this item */
		items[num++] = this_o_idx;

		/* Only one */
		if (mode & 0x04) break;
	}

	return num;
}




/**
 * Excise a dungeon object from any stacks
 */
void excise_object_idx(int o_idx)
{
	object_type *j_ptr;
	s16b this_o_idx, next_o_idx = 0;
	s16b prev_o_idx = 0;

	/* Object */
	j_ptr = cave_object(cave, o_idx);

	/* Monster */
	if (j_ptr->held_m_idx) {
		monster_type *m_ptr = cave_monster(cave, j_ptr->held_m_idx);

		/* Scan all objects in the grid */
		for (this_o_idx = m_ptr->hold_o_idx; this_o_idx;
			 this_o_idx = next_o_idx) {
			object_type *o_ptr;

			/* Get the object */
			o_ptr = cave_object(cave, this_o_idx);

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Found it? */
			if (this_o_idx == o_idx) {
				/* No previous */
				if (prev_o_idx == 0)
					/* Remove from list */
					m_ptr->hold_o_idx = next_o_idx;

				/* Real previous */
				else {
					object_type *i_ptr;

					/* Previous object */
					i_ptr = cave_object(cave, prev_o_idx);

					/* Remove from list */
					i_ptr->next_o_idx = next_o_idx;
				}

				/* Forget next pointer */
				o_ptr->next_o_idx = 0;

				/* Done */
				break;
			}

			/* Save prev_o_idx */
			prev_o_idx = this_o_idx;
		}
	} else {
		/* Dungeon */
		int y = j_ptr->iy;
		int x = j_ptr->ix;

		/* Scan all objects in the grid */
		for (this_o_idx = cave->o_idx[y][x]; this_o_idx;
			 this_o_idx = next_o_idx) {
			object_type *o_ptr;

			/* Get the object */
			o_ptr = cave_object(cave, this_o_idx);

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Found it? */
			if (this_o_idx == o_idx) {
				/* No previous */
				if (prev_o_idx == 0)
					/* Remove from list */
					cave->o_idx[y][x] = next_o_idx;

				/* Real previous */
				else {
					object_type *i_ptr;

					/* Previous object */
					i_ptr = cave_object(cave, prev_o_idx);

					/* Remove from list */
					i_ptr->next_o_idx = next_o_idx;
				}

				/* Forget next pointer */
				o_ptr->next_o_idx = 0;

				/* Done */
				break;
			}
			/* Save prev_o_idx */
			prev_o_idx = this_o_idx;
		}
	}
}


/**
 * Delete a dungeon object
 *
 * Handle "stacks" of objects correctly.
 */
void delete_object_idx(int o_idx)
{
	object_type *j_ptr;

	/* Excise */
	excise_object_idx(o_idx);

	/* Object */
	j_ptr = cave_object(cave, o_idx);

	/* Dungeon floor */
	if (!(j_ptr->held_m_idx)) {
		int y, x;

		/* Location */
		y = j_ptr->iy;
		x = j_ptr->ix;

		square_light_spot(cave, y, x);
	}

	/* Delete the mimicking monster if necessary */
	if (j_ptr->mimicking_m_idx) {
		monster_type *m_ptr;

		m_ptr = cave_monster(cave, j_ptr->mimicking_m_idx);

		/* Clear the mimicry */
		m_ptr->mimicked_o_idx = 0;
		mflag_off(m_ptr->mflag, MFLAG_UNAWARE);
	}

	/* Wipe the object */
	object_wipe(j_ptr);

	/* Count objects */
	cave->obj_cnt--;

	/* Stop tracking deleted objects if necessary */
	if (tracked_object_is(player->upkeep, 0 - o_idx))
		track_object(player->upkeep, NO_OBJECT);
}


/**
 * Deletes all objects at given location
 */
void delete_object(int y, int x)
{
	object_type *obj;

	/* Paranoia */
	if (!square_in_bounds(cave, y, x)) return;

	/* Scan all objects in the grid */
	for (obj = get_first_object(y, x); obj; obj = get_next_object(obj)) {

		/* Preserve unseen artifacts */
		if (obj->artifact && !object_was_sensed(obj))
			obj->artifact->created = FALSE;

		/* Delete the mimicking monster if necessary */
		if (obj->mimicking_m_idx) {
			monster_type *m_ptr;

			m_ptr = cave_monster(cave, obj->mimicking_m_idx);

			/* Clear the mimicry */
			m_ptr->mimicked_o_idx = 0;

			delete_monster_idx(obj->mimicking_m_idx);
		}

		/* Wipe the object */
		object_wipe(obj);

		/* Count objects */
		cave->obj_cnt--;
	}

	/* Objects are gone */
	cave->o_idx[y][x] = 0;

	/* Visual update */
	square_light_spot(cave, y, x);
}



/*
 * Move an object from index i1 to index i2 in the object list
 */
static void compact_objects_aux(int i1, int i2)
{
	int i;
	object_type *o_ptr;

	/* Do nothing */
	if (i1 == i2) return;

	/* Repair objects */
	for (i = 1; i < cave_object_max(cave); i++) {
		/* Get the object */
		o_ptr = cave_object(cave, i);

		/* Skip "dead" objects */
		if (!o_ptr->kind) continue;

		/* Repair "next" pointers */
		if (o_ptr->next_o_idx == i1)
		{
			/* Repair */
			o_ptr->next_o_idx = i2;
		}
	}

	/* Get the object */
	o_ptr = cave_object(cave, i1);

	/* Monster */
	if (o_ptr->held_m_idx) {
		monster_type *m_ptr;

		/* Get the monster */
		m_ptr = cave_monster(cave, o_ptr->held_m_idx);

		/* Repair monster */
		if (m_ptr->hold_o_idx == i1)
			m_ptr->hold_o_idx = i2;
	} else {
		/* Dungeon */
		int y, x;

		/* Get location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Repair grid */
		if (cave->o_idx[y][x] == i1)
			cave->o_idx[y][x] = i2;

		/* Mimic */
		if (o_ptr->mimicking_m_idx) {
			monster_type *m_ptr;

			/* Get the monster */
			m_ptr = cave_monster(cave, o_ptr->mimicking_m_idx);

			/* Repair monster */
			if (m_ptr->mimicked_o_idx == i1)
				m_ptr->mimicked_o_idx = i2;
		}
	}
	/* Hack -- move object */
	COPY(cave_object(cave, i2), cave_object(cave, i1), object_type);

	/* Hack -- wipe hole */
	object_wipe(o_ptr);
}


/**
 * Compact and reorder the object list
 *
 * This function can be very dangerous, use with caution!
 *
 * When compacting objects, we first destroy gold, on the basis that by the
 * time item compaction becomes an issue, the player really won't care.
 * We also nuke items marked as ignore.
 *
 * When compacting other objects, we base the saving throw on a combination of
 * object level, distance from player, and current "desperation".
 *
 * After compacting, we "reorder" the objects into a more compact order, and we
 * reset the allocation info, and the "live" array.
 */
void compact_objects(int size)
{
	int py = player->py;
	int px = player->px;

	int i, y, x, cnt;

	int cur_lev, cur_dis, chance;

	/* Reorder objects when not passed a size */
	if (!size) {
		/* Excise dead objects (backwards!) */
		for (i = cave_object_max(cave) - 1; i >= 1; i--) {
			object_type *o_ptr = cave_object(cave, i);
			if (o_ptr->kind) continue;

			/* Move last object into open hole */
			compact_objects_aux(cave_object_max(cave) - 1, i);

			/* Compress cave->obj_max */
			cave->obj_max--;
		}
		return;
	}

	/* Message */
	msg("Compacting objects...");

	/*** Try destroying objects ***/

	/* First do gold */
	for (i = 1; (i < cave_object_max(cave)) && (size); i++) {
		object_type *o_ptr = cave_object(cave, i);

		/* Nuke gold or ignored items */
		if (tval_is_money(o_ptr) || ignore_item_ok(o_ptr)) {
			delete_object_idx(i);
			size--;
		}
	}

	/* Compact at least 'size' objects */
	for (cnt = 1; size; cnt++) {
		/* Get more vicious each iteration */
		cur_lev = 5 * cnt;

		/* Get closer each iteration */
		cur_dis = 5 * (20 - cnt);

		/* Examine the objects */
		for (i = 1; (i < cave_object_max(cave)) && (size); i++) {
			object_type *o_ptr = cave_object(cave, i);
			if (!o_ptr->kind) continue;

			/* Hack -- High level objects start out "immune" */
			if (o_ptr->kind->level > cur_lev && !o_ptr->kind->ignore)
				continue;

			/* Monster */
			if (o_ptr->held_m_idx) {
				monster_type *m_ptr;

				/* Get the monster */
				m_ptr = cave_monster(cave, o_ptr->held_m_idx);

				/* Get the location */
				y = m_ptr->fy;
				x = m_ptr->fx;

				/* Monsters protect their objects */
				if ((randint0(100) < 90) && !o_ptr->kind->ignore)
					continue;
			} else if (o_ptr->mimicking_m_idx) {
				/* Mimicked items */

				/* Get the location */
				y = o_ptr->iy;
				x = o_ptr->ix;

				/* Mimicked items try hard not to be compacted */
				if (randint0(100) < 90)
					continue;
			} else {
				/* Dungeon */

				/* Get the location */
				y = o_ptr->iy;
				x = o_ptr->ix;
			}

			/* Nearby objects start out "immune" */
			if ((cur_dis > 0) && (distance(py, px, y, x) < cur_dis) &&
				!o_ptr->kind->ignore)
				continue;

			/* Saving throw */
			chance = 90;

			/* Hack -- only compact artifacts in emergencies */
			if (o_ptr->artifact && (cnt < 1000)) chance = 100;

			/* Apply the saving throw */
			if (randint0(100) < chance) continue;

			/* Delete the object */
			delete_object_idx(i);
			size--;
		}
	}

	/* Reorder objects */
	compact_objects(0);
}

/**
 * Free an object
 */
void object_free(struct object *obj)
{
	/* Free slays and brands */
	free_slay(obj->slays);
	free_brand(obj->brands);

	/* Free the object structure */
	object_wipe(obj);
}


/**
 * Delete all the items when player leaves the level
 *
 * Note -- we do NOT visually reflect these (irrelevant) changes
 *
 * Hack -- we clear the "cave->o_idx[y][x]" field for every grid,
 * and the "m_ptr->next_o_idx" field for every monster, since
 * we know we are clearing every object.  Technically, we only
 * clear those fields for grids/monsters containing objects,
 * and we clear it once for every such object.
 */
void wipe_o_list(struct chunk *c)
{
	int i;

	/* Delete the existing objects */
	for (i = 1; i < cave_object_max(c); i++)
	{
		object_type *o_ptr = cave_object(c, i);
		if (!o_ptr->kind) continue;

		/* Preserve artifacts or mark them as lost in the history */
		if (o_ptr->artifact) {
			/* Preserve if dungeon creation failed, or preserve mode, or items
			 * carried by monsters, and only artifacts not seen */
			if ((!character_dungeon || !OPT(birth_no_preserve) ||
					o_ptr->held_m_idx) && !object_was_sensed(o_ptr))
				o_ptr->artifact->created = FALSE;
			else
				history_lose_artifact(o_ptr->artifact);
		}

		/* Monster */
		if (o_ptr->held_m_idx) {
			monster_type *m_ptr;

			/* Monster */
			m_ptr = cave_monster(c, o_ptr->held_m_idx);

			/* Hack -- see above */
			m_ptr->hold_o_idx = 0;
		} else {
			/* Dungeon */
			/* Get the location */
			int y = o_ptr->iy;
			int x = o_ptr->ix;

			/* Hack -- see above */
			c->o_idx[y][x] = 0;
		}

		/* Wipe the object */
		object_free(o_ptr);
	}

	/* Reset obj_max */
	c->obj_max = 1;

	/* Reset obj_cnt */
	c->obj_cnt = 0;
}


/**
 * Get and return the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b o_pop(struct chunk *c)
{
	int i;

	/* Initial allocation */
	if (cave_object_max(c) < z_info->level_object_max) {
		/* Get next space */
		i = cave_object_max(c);

		/* Expand object array */
		c->obj_max++;

		/* Count objects */
		c->obj_cnt++;

		/* Use this object */
		return (i);
	}

	/* Recycle dead objects */
	for (i = 1; i < cave_object_max(c); i++) {
		object_type *obj = cave_object(c, i);
		if (obj->kind) continue;

		/* Count objects */
		c->obj_cnt++;

		/* Use this object */
		return (i);
	}

	/* Warn the player (except during dungeon creation) */
	if (character_dungeon) msg("Too many objects!");

	/* Oops */
	return (0);
}


/**
 * Get the first object at a dungeon location
 * or NULL if there isn't one.
 */
object_type *get_first_object(int y, int x)
{
	s16b o_idx = cave->o_idx[y][x];

	if (o_idx)
		return cave_object(cave, o_idx);

	/* No object */
	return (NULL);
}


/**
 * Get the next object in a stack or NULL if there isn't one.
 */
object_type *get_next_object(const object_type *o_ptr)
{
	if (o_ptr->next_o_idx)
		return cave_object(cave, o_ptr->next_o_idx);

	/* No more objects */
	return NULL;
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
bool object_stackable(const object_type *o_ptr, const object_type *j_ptr,
					  object_stack_t mode)
{
	int i;

	/* Equipment items don't stack */
	if (item_is_equipped(player, object_gear_index(player, o_ptr)))
		return FALSE;
	if (item_is_equipped(player, object_gear_index(player, j_ptr)))
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
bool object_similar(const object_type *o_ptr, const object_type *j_ptr,
					object_stack_t mode)
{
	int total = o_ptr->number + j_ptr->number;

	/* Check against stacking limit - except in stores which absorb anyway */
	if (!(mode & OSTACK_STORE) && (total >= MAX_STACK_SIZE)) return FALSE;

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
static void object_absorb_merge(object_type *o_ptr, const object_type *j_ptr)
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
void object_absorb_partial(object_type *o_ptr, object_type *j_ptr)
{
	int smallest = MIN(o_ptr->number, j_ptr->number);
	int largest = MAX(o_ptr->number, j_ptr->number);
	int difference = (MAX_STACK_SIZE - 1) - largest;
	o_ptr->number = largest + difference;
	j_ptr->number = smallest - difference;

	object_absorb_merge(o_ptr, j_ptr);
}

/**
 * Merge two stacks into one stack.
 */
void object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Add together the item counts */
	o_ptr->number = ((total < MAX_STACK_SIZE) ? total : (MAX_STACK_SIZE - 1));

	object_absorb_merge(o_ptr, j_ptr);
}

/**
 * Wipe an object clean.
 */
void object_wipe(object_type *obj)
{
	/* Wipe the structure */
	memset(obj, 0, sizeof(object_type));
}


/**
 * Prepare an object based on an existing object
 */
void object_copy(object_type *o_ptr, const object_type *j_ptr)
{
	/* Copy the structure */
	COPY(o_ptr, j_ptr, object_type);
}

/**
 * Prepare an object `dst` representing `amt` objects,  based on an existing
 * object `src` representing at least `amt` objects.
 *
 * Takes care of the charge redistribution concerns of stacked items.
 */
void object_copy_amt(object_type *dst, object_type *src, int amt)
{
	int charge_time = randcalc(src->time, 0, AVERAGE), max_time;

	/* Get a copy of the object */
	object_copy(dst, src);

	/* Modify quantity */
	dst->number = amt;
	dst->note = src->note;

	/*
	 * If the item has charges/timeouts, set them to the correct level
	 * too. We split off the same amount as distribute_charges.
	 */
	if (tval_can_have_charges(src))
		dst->pval = src->pval * amt / src->number;

	if (tval_can_have_timeout(src)) {
		max_time = charge_time * amt;

		if (src->timeout > max_time)
			dst->timeout = max_time;
		else
			dst->timeout = src->timeout;
	}
}

/**
 * Split off 'amt' items from 'src' into 'dest'.
 */
void object_split(struct object *dest, struct object *src, int amt)
{
	/* Distribute charges of wands, staves, or rods */
	distribute_charges(src, dest, amt);

	/* Modify quantity */
	dest->number = amt;
	if (src->note)
		dest->note = src->note;
}

/**
 * Find and return the index to the oldest object on the given grid marked as
 * "ignore".
 */
static s16b floor_get_idx_oldest_ignored(int y, int x)
{
	s16b ignore_idx = 0;
	s16b this_o_idx;

	object_type *o_ptr = NULL;

	for (this_o_idx = cave->o_idx[y][x]; this_o_idx;
		 this_o_idx = o_ptr->next_o_idx)
	{
		o_ptr = cave_object(cave, this_o_idx);

		if (ignore_item_ok(o_ptr))
			ignore_idx = this_o_idx;
	}

	return ignore_idx;
}



/**
 * Let the floor carry an object, deleting old ignored items if necessary
 */
s16b floor_carry(struct chunk *c, int y, int x, object_type *j_ptr)
{
	int n = 0;
	s16b o_idx;
	s16b this_o_idx, next_o_idx = 0;


	/* Scan objects in that grid for combination */
	for (this_o_idx = c->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr = cave_object(c, this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr, OSTACK_FLOOR)) {
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Result */
			return (this_o_idx);
		}

		/* Count objects */
		n++;
	}

	/* Option -- disallow stacking */
	if (OPT(birth_no_stacking) && n) return (0);

	/* The stack is already too large */
	if (n >= z_info->floor_size) {
		/* Ignore the oldest ignored object */
		s16b ignore_idx = floor_get_idx_oldest_ignored(y, x);

		if (ignore_idx)
			delete_object_idx(ignore_idx);
		else
			return 0;
	}

	/* Make an object */
	o_idx = o_pop(c);

	/* Success */
	if (o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = cave_object(c, o_idx);

		/* Structure Copy */
		object_copy(o_ptr, j_ptr);

		/* Location */
		o_ptr->iy = y;
		o_ptr->ix = x;

		/* Forget monster */
		o_ptr->held_m_idx = 0;

		/* Link the object to the pile */
		o_ptr->next_o_idx = c->o_idx[y][x];

		/* Link the floor to the object */
		c->o_idx[y][x] = o_idx;

		square_note_spot(c, y, x);
		square_light_spot(c, y, x);
	}

	/* Result */
	return (o_idx);
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
 */
void drop_near(struct chunk *c, object_type *j_ptr, int chance, int y, int x,
			   bool verbose)
{
	int i, k, n, d, s;

	int bs, bn;
	int by, bx;
	int dy, dx;
	int ty, tx;

	object_type *o_ptr;

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
			for (o_ptr = get_first_object(ty, tx); o_ptr;
					o_ptr = get_next_object(o_ptr)) {
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
				!floor_get_idx_oldest_ignored(ty, tx)) continue;

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
	if (!floor_carry(c, by, bx, j_ptr)) {
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

	object_type *o_ptr;

	struct queue *queue = q_new(z_info->floor_size);

	/* Push all objects on the square into the queue */
	for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
		q_push_ptr(queue, o_ptr);

	/* Set feature to an open door */
	square_force_floor(cave, y, x);
	square_add_door(cave, y, x, FALSE);

	/* Drop objects back onto the floor */
	while (q_len(queue) > 0) {
		/* Take object from the queue */
		o_ptr = q_pop_ptr(queue);

		/* Drop the object */
		drop_near(cave, o_ptr, 0, y, x, FALSE);
	}

	/* Delete original objects */
	delete_object(y, x);

	/* Reset cave feature */
	square_set_feat(cave, y, x, feat_old->fidx);

	q_free(queue);
}

/**
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(int item)
{
	object_type *o_ptr = cave_object(cave, item);

	/* Require staff/wand */
	if (!tval_can_have_charges(o_ptr)) return;

	/* Require known item */
	if (!object_is_known(o_ptr)) return;

	/* Print a message */
	msg("There %s %d charge%s remaining.",
	    (o_ptr->pval != 1) ? "are" : "is",
	     o_ptr->pval,
	    (o_ptr->pval != 1) ? "s" : "");
}



/**
 * Describe an item on the floor.
 */
void floor_item_describe(int item)
{
	object_type *o_ptr = cave_object(cave, item);

	char o_name[80];

	/* Get a description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Print a message */
	msg("You see %s.", o_name);
}


/**
 * Increase the "number" of an item on the floor
 */
void floor_item_increase(int item, int num)
{
	object_type *o_ptr = cave_object(cave, item);

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number */
	o_ptr->number += num;
}


/**
 * Optimize an item on the floor (destroy "empty" items)
 */
void floor_item_optimize(int item)
{
	object_type *o_ptr = cave_object(cave, item);

	/* Paranoia -- be sure it exists */
	if (!o_ptr->kind) return;

	/* Only optimize empty items */
	if (o_ptr->number) return;

	/* Delete the object */
	delete_object_idx(item);
}


/**
 * Get a list of "valid" item indexes.
 *
 * Fills item_list[] with items that are "okay" as defined by the
 * provided tester function, etc.  mode determines what combination of
 * inventory, equipment and player's floor location should be used
 * when drawing up the list.
 *
 * Returns the number of items placed into the list.
 *
 * Maximum space that can be used is
 * z_info->pack_size + z_info->quiver_size + player->body.count +
 * z_info->floor_size,
 * though practically speaking much smaller numbers are likely.
 */
int scan_items(int *item_list, size_t item_list_max, int mode,
			   item_tester tester)
{
	bool use_inven = ((mode & USE_INVEN) ? TRUE : FALSE);
	bool use_equip = ((mode & USE_EQUIP) ? TRUE : FALSE);
	bool use_quiver = ((mode & USE_QUIVER) ? TRUE : FALSE);
	bool use_floor = ((mode & USE_FLOOR) ? TRUE : FALSE);

	int floor_max = z_info->floor_size;
	int *floor_list = mem_zalloc(floor_max * sizeof(int));
	int floor_num;

	int i;
	size_t item_list_num = 0;

	if (use_inven)
		for (i = 0; i < z_info->pack_size && item_list_num < item_list_max; i++)
		{
			if (item_test(tester, player->upkeep->inven[i]))
				item_list[item_list_num++] = player->upkeep->inven[i];
		}

	if (use_equip)
		for (i = 0; i < player->body.count && item_list_num < item_list_max;
			 i++) {
			if (item_test(tester, slot_index(player, i)))
				item_list[item_list_num++] = slot_index(player, i);
		}

	if (use_quiver)
		for (i = 0; i < z_info->quiver_size && item_list_num < item_list_max; i++) {
			if (item_test(tester, player->upkeep->quiver[i]))
				item_list[item_list_num++] = player->upkeep->quiver[i];
		}

	/* Scan all non-gold objects in the grid */
	if (use_floor) {
		floor_num = scan_floor(floor_list, floor_max, player->py, player->px,
							   0x0B, tester);

		for (i = 0; i < floor_num && item_list_num < item_list_max; i++)
			item_list[item_list_num++] = -floor_list[i];
	}

	mem_free(floor_list);
	return item_list_num;
}


/**
 * Check if the given item is available for the player to use.
 *
 * 'mode' defines which areas we should look at, a la scan_items().
 */
bool item_is_available(int item, bool (*tester)(const object_type *), int mode)
{
	int item_max = z_info->pack_size + z_info->quiver_size +
		player->body.count + z_info->floor_size;
	int *item_list = mem_zalloc(item_max * sizeof(int));
	int item_num;
	int i;

	item_num = scan_items(item_list, item_max, mode, tester);

	for (i = 0; i < item_num; i++)
		if (item_list[i] == item) {
			mem_free(item_list);
			return TRUE;
		}

	mem_free(item_list);
	return FALSE;
}

