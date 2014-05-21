/*
 * File: object2.c
 * Purpose: Object list maintenance and other object utilities
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
#include "obj-info.h"
#include "obj-make.h"
#include "obj-tval.h"
#include "obj-tvalsval.h"
#include "obj-ui.h"
#include "obj-util.h"
#include "player-util.h"
#include "prefs.h"
#include "randname.h"
#include "spells.h"
#include "squelch.h"
#include "z-queue.h"

spell_type *s_info;
object_base *kb_info;
object_kind *k_info;
artifact_type *a_info;
ego_item_type *e_info;
struct flavor *flavors;

/*
 * Hold the titles of scrolls, 6 to 14 characters each, plus quotes.
 */
static char scroll_adj[MAX_TITLES][18];

static void flavor_assign_fixed(void)
{
	int i;
	struct flavor *f;

	for (f = flavors; f; f = f->next) {
		if (f->sval == SV_UNKNOWN)
			continue;

		for (i = 0; i < z_info->k_max; i++) {
			struct object_kind *k = &k_info[i];
			if (k->tval == f->tval && k->sval == f->sval)
				k->flavor = f;
		}
	}
}


static void flavor_assign_random(byte tval)
{
	int i;
	int flavor_count = 0;
	int choice;
	struct flavor *f;

	/* Count the random flavors for the given tval */
	for (f = flavors; f; f = f->next)
		if (f->tval == tval && f->sval == SV_UNKNOWN)
			flavor_count++;

	for (i = 0; i < z_info->k_max; i++) {
		if (k_info[i].tval != tval || k_info[i].flavor)
			continue;

		if (!flavor_count)
			quit_fmt("Not enough flavors for tval %d.", tval);

		choice = randint0(flavor_count);
	
		for (f = flavors; f; f = f->next) {
			if (f->tval != tval || f->sval != SV_UNKNOWN)
				continue;

			if (choice == 0) {
				k_info[i].flavor = f;
				f->sval = k_info[i].sval;
				if (tval == TV_SCROLL)
					f->text = scroll_adj[k_info[i].sval];
				flavor_count--;
				break;
			}

			choice--;
		}
	}
}

/**
 * Reset svals on flavors, effectively removing any fixed flavors.
 *
 * Mainly useful for randarts so that fixed flavors for standards aren't predictable. The One Ring
 * is kept as fixed, since it lives through randarts.
 */
void flavor_reset_fixed(void)
{
	struct flavor *f;

	for (f = flavors; f; f = f->next) {
		if (f->tval == TV_RING && f->sval == SV_RING_POWER)
			continue;

		f->sval = SV_UNKNOWN;
	}
}

/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * The first 4 entries for potions are fixed (Water, Apple Juice,
 * Slime Mold Juice, Unused Potion).
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 2 to 8 letters long, and that no scroll is finished
 * until it attempts to grow beyond 15 letters.  The first time this
 * can happen is when the current title has 6 letters and the new word
 * has 8 letters, which would result in a 6 letter scroll title.
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 */
void flavor_init(void)
{
	int i, j;

	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;

	if (OPT(birth_randarts))
		flavor_reset_fixed();

	flavor_assign_fixed();

	flavor_assign_random(TV_RING);
	flavor_assign_random(TV_AMULET);
	flavor_assign_random(TV_STAFF);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_ROD);
	flavor_assign_random(TV_MUSHROOM);
	flavor_assign_random(TV_POTION);

	/* Scrolls (random titles, always white) */
	for (i = 0; i < MAX_TITLES; i++)
	{
		char buf[26];
		char *end = buf + 1;
		int titlelen = 0;
		int wordlen;
		bool okay = TRUE;

		strcpy(buf, "\"");
		wordlen = randname_make(RANDNAME_SCROLL, 2, 8, end, 24, name_sections);
		while (titlelen + wordlen < (int)(sizeof(scroll_adj[0]) - 3))
		{
			end[wordlen] = ' ';
			titlelen += wordlen + 1;
			end += wordlen + 1;
			wordlen = randname_make(RANDNAME_SCROLL, 2, 8, end, 24 - titlelen, name_sections);
		}
		buf[titlelen] = '"';
		buf[titlelen+1] = '\0';

		/* Check the scroll name hasn't already been generated */
		for (j = 0; j < i; j++)
		{
			if (streq(buf, scroll_adj[j]))
			{
				okay = FALSE;
				break;
			}
		}

		if (okay)
		{
			my_strcpy(scroll_adj[i], buf, sizeof(scroll_adj[0]));
		}
		else
		{
			/* Have another go at making a name */
			i--;
		}
	}
	flavor_assign_random(TV_SCROLL);

	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

	/* Analyze every object */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* No flavor yields aware */
		if (!k_ptr->flavor) k_ptr->aware = TRUE;
	}
}


/*
 * Obtain the flags for an item
 */
void object_flags(const object_type *o_ptr, bitflag flags[OF_SIZE])
{
	of_wipe(flags);

	if (!o_ptr->kind)
		return;

	of_copy(flags, o_ptr->flags);
}


/*
 * Obtain the flags for an item which are known to the player
 */
void object_flags_known(const object_type *o_ptr, bitflag flags[OF_SIZE])
{
	object_flags(o_ptr, flags);

	of_inter(flags, o_ptr->known_flags);

	if (object_flavor_is_aware(o_ptr))
		of_union(flags, o_ptr->kind->flags);

	if (o_ptr->ego && easy_know(o_ptr))
		of_union(flags, o_ptr->ego->flags);
}

/*
 * Apply a tester function, skipping all non-objects and gold
 */
bool object_test(item_tester tester, const struct object *obj)
{
	/* Require kind */
	if (!obj->kind) return FALSE;

	/* Ignore gold */
	if (tval_is_money(obj)) return FALSE;

	/* Pass without a tester, or tail-call the tester if it exists */
	return !tester || tester(obj);
}


/*
 * Verify the "okayness" of a given item.
 */
bool item_test(item_tester tester, int item)
{
	/* Verify the item */
	return object_test(tester, object_from_item_idx(item));
}



/** 
 * Return true if the item is unknown (has yet to be seen by the player).
 */
bool is_unknown(const object_type *o_ptr)
{
	grid_data gd = { 0 };
	map_info(o_ptr->iy, o_ptr->ix, &gd);
	return gd.unseen_object;
}	


/*
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
int scan_floor(int *items, int max_size, int y, int x, int mode, item_tester tester)
{
	int this_o_idx, next_o_idx;

	int num = 0;
	
	/* Sanity */
	if (!square_in_bounds(cave, y, x)) return 0;

	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
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
		if (mode & 0x02) {
			if (!o_ptr->marked) continue;
		}
		
		/* Visible */
		if (mode & 0x08) {
			if (!is_unknown(o_ptr) && squelch_item_ok(o_ptr)) continue;
		}

		/* Accept this item */
		items[num++] = this_o_idx;

		/* Only one */
		if (mode & 0x04) break;
	}

	return num;
}




/*
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
	if (j_ptr->held_m_idx)
	{
		monster_type *m_ptr;

		/* Monster */
		m_ptr = cave_monster(cave, j_ptr->held_m_idx);

		/* Scan all objects in the grid */
		for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Get the object */
			o_ptr = cave_object(cave, this_o_idx);

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Done */
			if (this_o_idx == o_idx)
			{
				/* No previous */
				if (prev_o_idx == 0)
				{
					/* Remove from list */
					m_ptr->hold_o_idx = next_o_idx;
				}

				/* Real previous */
				else
				{
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

	/* Dungeon */
	else
	{
		int y = j_ptr->iy;
		int x = j_ptr->ix;

		/* Scan all objects in the grid */
		for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Get the object */
			o_ptr = cave_object(cave, this_o_idx);

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Done */
			if (this_o_idx == o_idx)
			{
				/* No previous */
				if (prev_o_idx == 0)
				{
					/* Remove from list */
					cave->o_idx[y][x] = next_o_idx;
				}

				/* Real previous */
				else
				{
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


/*
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
	if (!(j_ptr->held_m_idx))
	{
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
		m_ptr->unaware = FALSE;

#if 0 /* Hack - just make the mimic obviously a mimic instead of deleting it */
		delete_monster_idx(j_ptr->mimicking_m_idx);
#endif
	}

	/* Wipe the object */
	object_wipe(j_ptr);

	/* Count objects */
	cave->obj_cnt--;

	/* Stop tracking deleted objects if necessary */
	if (tracked_object_is(player->upkeep, 0 - o_idx))
	{
		track_object(player->upkeep, NO_OBJECT);
	}
}


/*
 * Deletes all objects at given location
 */
void delete_object(int y, int x)
{
	s16b this_o_idx, next_o_idx = 0;

	/* Paranoia */
	if (!square_in_bounds(cave, y, x)) return;

	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = cave_object(cave, this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Preserve unseen artifacts */
		if (o_ptr->artifact && !object_was_sensed(o_ptr))
			o_ptr->artifact->created = FALSE;

		/* Delete the mimicking monster if necessary */
		if (o_ptr->mimicking_m_idx) {
			monster_type *m_ptr;
			
			m_ptr = cave_monster(cave, o_ptr->mimicking_m_idx);
			
			/* Clear the mimicry */
			m_ptr->mimicked_o_idx = 0;
			
			delete_monster_idx(o_ptr->mimicking_m_idx);
		}

		/* Wipe the object */
		object_wipe(o_ptr);

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
	for (i = 1; i < cave_object_max(cave); i++)
	{
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
	if (o_ptr->held_m_idx)
	{
		monster_type *m_ptr;

		/* Get the monster */
		m_ptr = cave_monster(cave, o_ptr->held_m_idx);

		/* Repair monster */
		if (m_ptr->hold_o_idx == i1)
		{
			/* Repair */
			m_ptr->hold_o_idx = i2;
		}
	}

	/* Dungeon */
	else
	{
		int y, x;

		/* Get location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Repair grid */
		if (cave->o_idx[y][x] == i1)
		{
			/* Repair */
			cave->o_idx[y][x] = i2;
		}

		/* Mimic */
		if (o_ptr->mimicking_m_idx)
		{
			monster_type *m_ptr;

			/* Get the monster */
			m_ptr = cave_monster(cave, o_ptr->mimicking_m_idx);

			/* Repair monster */
			if (m_ptr->mimicked_o_idx == i1)
			{
				/* Repair */
				m_ptr->mimicked_o_idx = i2;
			}
		}
	}


	/* Hack -- move object */
	COPY(cave_object(cave, i2), cave_object(cave, i1), object_type);

	/* Hack -- wipe hole */
	object_wipe(o_ptr);
}


/*
 * Compact and reorder the object list
 *
 * This function can be very dangerous, use with caution!
 *
 * When compacting objects, we first destroy gold, on the basis that by the
 * time item compaction becomes an issue, the player really won't care.
 * We also nuke items marked as squelch.
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
	if (!size)
	{
		/* Excise dead objects (backwards!) */
		for (i = cave_object_max(cave) - 1; i >= 1; i--)
		{
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
	for (i = 1; (i < cave_object_max(cave)) && (size); i++)
	{
		object_type *o_ptr = cave_object(cave, i);

		/* Nuke gold or squelched items */
		if (tval_is_money(o_ptr) || squelch_item_ok(o_ptr))
		{
			delete_object_idx(i);
			size--;
		}
	}


	/* Compact at least 'size' objects */
	for (cnt = 1; size; cnt++)
	{
		/* Get more vicious each iteration */
		cur_lev = 5 * cnt;

		/* Get closer each iteration */
		cur_dis = 5 * (20 - cnt);

		/* Examine the objects */
		for (i = 1; (i < cave_object_max(cave)) && (size); i++)
		{
			object_type *o_ptr = cave_object(cave, i);
			if (!o_ptr->kind) continue;

			/* Hack -- High level objects start out "immune" */
			if (o_ptr->kind->level > cur_lev && !o_ptr->kind->squelch)
				continue;

			/* Monster */
			if (o_ptr->held_m_idx)
			{
				monster_type *m_ptr;

				/* Get the monster */
				m_ptr = cave_monster(cave, o_ptr->held_m_idx);

				/* Get the location */
				y = m_ptr->fy;
				x = m_ptr->fx;

				/* Monsters protect their objects */
				if ((randint0(100) < 90) && !o_ptr->kind->squelch)
					continue;
			}

			/* Mimicked items */
			else if (o_ptr->mimicking_m_idx)
			{
				/* Get the location */
				y = o_ptr->iy;
				x = o_ptr->ix;

				/* Mimicked items try hard not to be compacted */
				if (randint0(100) < 90)
					continue;
			}
			
			/* Dungeon */
			else
			{
				/* Get the location */
				y = o_ptr->iy;
				x = o_ptr->ix;
			}

			/* Nearby objects start out "immune" */
			if ((cur_dis > 0) && (distance(py, px, y, x) < cur_dis) && !o_ptr->kind->squelch)
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


/*
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
	for (i = 1; i < cave_object_max(cave); i++)
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
		if (o_ptr->held_m_idx)
		{
			monster_type *m_ptr;

			/* Monster */
			m_ptr = cave_monster(c, o_ptr->held_m_idx);

			/* Hack -- see above */
			m_ptr->hold_o_idx = 0;
		}

		/* Dungeon */
		else
		{
			/* Get the location */
			int y = o_ptr->iy;
			int x = o_ptr->ix;

			/* Hack -- see above */
			c->o_idx[y][x] = 0;
		}

		/* Wipe the object */
		(void)WIPE(o_ptr, object_type);
	}

	/* Reset obj_max */
	cave->obj_max = 1;

	/* Reset obj_cnt */
	cave->obj_cnt = 0;
}


/*
 * Get and return the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b o_pop(struct chunk *c)
{
	int i;


	/* Initial allocation */
	if (cave_object_max(c) < z_info->o_max)
	{
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
	for (i = 1; i < cave_object_max(c); i++)
	{
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


/*
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


/*
 * Get the next object in a stack or NULL if there isn't one.
 */
object_type *get_next_object(const object_type *o_ptr)
{
	if (o_ptr->next_o_idx)
		return cave_object(cave, o_ptr->next_o_idx);

	/* No more objects */
	return NULL;
}



/*
 * Determine if a weapon is 'blessed'
 */
bool is_blessed(const object_type *o_ptr)
{
	/* Is the object blessed? */
	return of_has(o_ptr->flags, OF_BLESSED) ? TRUE : FALSE;
}



/*
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b object_value_base(const object_type *o_ptr)
{
	/* Use template cost for aware objects */
	if (object_flavor_is_aware(o_ptr) || object_all_but_flavor_is_known(o_ptr))
		return o_ptr->kind->cost;

	/* Analyze the type */
	switch (o_ptr->tval)
	{
		case TV_FOOD:
		case TV_MUSHROOM:
			return 5;
		case TV_POTION:
		case TV_SCROLL:
			return 20;
		case TV_RING:
		case TV_AMULET:
			return 45;
		case TV_WAND:
			return 50;
		case TV_STAFF:
			return 70;
		case TV_ROD:
			return 90;
	}

	return 0;
}


/*
 * Return the "real" price of a "known" item, not including discounts.
 *
 * Wand and staffs get cost for each charge.
 *
 * Wearable items (weapons, launchers, jewelry, lights, armour) and ammo
 * are priced according to their power rating. All ammo, and normal (non-ego)
 * torches are scaled down by AMMO_RESCALER to reflect their impermanence.
 */
s32b object_value_real(const object_type *o_ptr, int qty, int verbose,
	bool known)
{
	s32b value, total_value;

	s32b power;
	int a = 1;
	int b = 5;
	static file_mode pricing_mode = MODE_WRITE;

	if (wearable_p(o_ptr))
	{
 		char buf[1024];
		ang_file *log_file = NULL;

		if (verbose)
		{
			path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "pricing.log");
                	log_file = file_open(buf, pricing_mode, FTYPE_TEXT);
                	if (!log_file)
                	{
                		msg("Error - can't open pricing.log for writing.");
                	        exit(1);
                	}
			pricing_mode = MODE_APPEND;
		}

		file_putf(log_file, "object is %s\n", o_ptr->kind->name);
		power = object_power(o_ptr, verbose, log_file, known);
		value = sign(power) * ((a * power * power) + (b * power));

		if (tval_is_ammo(o_ptr) || (tval_is_light(o_ptr)
			&& (o_ptr->sval == SV_LIGHT_TORCH) && !o_ptr->ego) )
		{
			value = value / AMMO_RESCALER;
			if (value < 1) value = 1;
		}

		file_putf(log_file, "a is %d and b is %d\n", a, b);
		file_putf(log_file, "value is %d\n", value);
		total_value = value * qty;

		if (verbose)
		{
			if (!file_close(log_file))
			{
				msg("Error - can't close pricing.log file.");
				exit(1);
			}
		}
		if (total_value < 0) total_value = 0;

		return (total_value);
	}

	/* Hack -- "worthless" items */
	if (!o_ptr->kind->cost) return (0L);

	/* Base cost */
	value = o_ptr->kind->cost;

	/* Analyze the item type and quantity*/
	if (tval_can_have_charges(o_ptr)) {
		int charges;

		total_value = value * qty;

		/* Calculate number of charges, rounded up */
		charges = o_ptr->pval
		* qty / o_ptr->number;
		if ((o_ptr->pval * qty) % o_ptr->number != 0)
			charges++;

		/* Pay extra for charges, depending on standard number of charges */
		total_value += value * charges / 20;
	}
	else {
		total_value = value * qty;
	}

	/* No negative value */
	if (total_value < 0) total_value = 0;

	/* Return the value */
	return (total_value);
}


/*
 * Return the price of an item including plusses (and charges).
 *
 * This function returns the "value" of the given item (qty one).
 *
 * Never notice "unknown" bonuses or properties, including "curses",
 * since that would give the player information he did not have.
 *
 * Note that discounted items stay discounted forever.
 */
s32b object_value(const object_type *o_ptr, int qty, int verbose)
{
	s32b value;


	if (object_is_known(o_ptr))
	{
		if (cursed_p((bitflag *)o_ptr->flags)) return (0L);

		value = object_value_real(o_ptr, qty, verbose, TRUE);
	}
	else if (wearable_p(o_ptr))
	{
		object_type object_type_body;
		object_type *j_ptr = &object_type_body;

		/* Hack -- Felt cursed items */
		if (object_was_sensed(o_ptr) && cursed_p((bitflag *)o_ptr->flags))
			return (0L);

		memcpy(j_ptr, o_ptr, sizeof(object_type));

		/* give j_ptr only the flags known to be in o_ptr */
		object_flags_known(o_ptr, j_ptr->flags);

		if (!object_attack_plusses_are_visible(o_ptr))
			j_ptr->to_h = j_ptr->to_d = 0;
		if (!object_defence_plusses_are_visible(o_ptr))
			j_ptr->to_a = 0;

		value = object_value_real(j_ptr, qty, verbose, FALSE);
	}
	else value = object_value_base(o_ptr) * qty;


	/* Return the final value */
	return (value);
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
	}
	else if (tval_can_have_charges(o_ptr) || tval_is_money(o_ptr)) {
		/* Gold, staves and wands stack most of the time */
		/* Too much gold or too many charges */
		if (o_ptr->pval + j_ptr->pval > MAX_PVAL)
			return FALSE;

		/* ... otherwise ok */
	}
	else if (tval_is_weapon(o_ptr) || tval_is_armor(o_ptr) ||
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
	}
	else {
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
 * The blending of the "discount" field assumes that either (1) one is a
 * special inscription and one is nothing, or (2) one is a discount and
 * one is a smaller discount, or (3) one is a discount and one is nothing,
 * or (4) both are nothing.  In all of these cases, we can simply use the
 * "maximum" of the two "discount" fields.
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
	if (tval_can_have_charges(o_ptr) || tval_is_money(o_ptr))
	{
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
void object_absorb(object_type *o_ptr, const object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Add together the item counts */
	o_ptr->number = ((total < MAX_STACK_SIZE) ? total : (MAX_STACK_SIZE - 1));

	object_absorb_merge(o_ptr, j_ptr);
}

/*
 * Wipe an object clean.
 */
void object_wipe(object_type *o_ptr)
{
	/* Wipe the structure */
	(void)WIPE(o_ptr, object_type);
}


/*
 * Prepare an object based on an existing object
 */
void object_copy(object_type *o_ptr, const object_type *j_ptr)
{
	/* Copy the structure */
	COPY(o_ptr, j_ptr, object_type);
}

/*
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
	{
		dst->pval =
			src->pval * amt / src->number;
	}

	if (tval_can_have_timeout(src))
	{
		max_time = charge_time * amt;

		if (src->timeout > max_time)
			dst->timeout = max_time;
		else
			dst->timeout = src->timeout;
	}
}

/**
 * Split off 'at' items from 'src' into 'dest'.
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
 * "squelch".
 */
static s16b floor_get_idx_oldest_squelched(int y, int x)
{
	s16b squelch_idx = 0;
	s16b this_o_idx;

	object_type *o_ptr = NULL;

	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = o_ptr->next_o_idx)
	{
		o_ptr = cave_object(cave, this_o_idx);

		if (squelch_item_ok(o_ptr))
			squelch_idx = this_o_idx;
	}

	return squelch_idx;
}



/*
 * Let the floor carry an object, deleting old squelched items if necessary
 */
s16b floor_carry(struct chunk *c, int y, int x, object_type *j_ptr)
{
	int n = 0;

	s16b o_idx;

	s16b this_o_idx, next_o_idx = 0;


	/* Scan objects in that grid for combination */
	for (this_o_idx = c->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr = cave_object(c, this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr, OSTACK_FLOOR))
		{
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
	if (n >= MAX_FLOOR_STACK)
	{
		/* Squelch the oldest squelched object */
		s16b squelch_idx = floor_get_idx_oldest_squelched(y, x);

		if (squelch_idx)
			delete_object_idx(squelch_idx);
		else
			return 0;
	}


	/* Make an object */
	o_idx = o_pop(c);

	/* Success */
	if (o_idx)
	{
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


/*
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
void drop_near(struct chunk *c, object_type *j_ptr, int chance, int y, int x, bool verbose)
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
	if (!j_ptr->artifact && (randint0(100) < chance))
	{
		/* Message */
		msg("The %s %s.", o_name, VERB_AGREEMENT(j_ptr->number, "breaks", "break"));

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
	for (dy = -3; dy <= 3; dy++)
	{
		/* Scan local grids */
		for (dx = -3; dx <= 3; dx++)
		{
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
					o_ptr = get_next_object(o_ptr))
			{
				/* Check for possible combination */
				if (object_similar(o_ptr, j_ptr, OSTACK_FLOOR))
					comb = TRUE;

				/* Count objects */
				if (!squelch_item_ok(o_ptr))
					k++;
				else
					n++;
			}

			/* Add new object */
			if (!comb) k++;

			/* Option -- disallow stacking */
			if (OPT(birth_no_stacking) && (k > 1)) continue;
			
			/* Paranoia? */
			if ((k + n) > MAX_FLOOR_STACK &&
					!floor_get_idx_oldest_squelched(ty, tx)) continue;

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
	if (!flag && !j_ptr->artifact)
	{
		/* Message */
		msg("The %s %s.", o_name, VERB_AGREEMENT(j_ptr->number, "disappears", "disappear"));

		/* Debug */
		if (player->wizard) msg("Breakage (no floor space).");

		/* Failure */
		return;
	}


	/* Find a grid */
	for (i = 0; !flag; i++)
	{
		/* Bounce around */
		if (i < 1000)
		{
			ty = rand_spread(by, 1);
			tx = rand_spread(bx, 1);
		}

		/* Random locations */
		else
		{
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
	if (!floor_carry(c, by, bx, j_ptr))
	{
		/* Message */
		msg("The %s %s.", o_name, VERB_AGREEMENT(j_ptr->number, "disappears", "disappear"));

		/* Debug */
		if (player->wizard) msg("Breakage (too many objects).");

		if (j_ptr->artifact) j_ptr->artifact->created = FALSE;

		/* Failure */
		return;
	}


	/* Sound */
	sound(MSG_DROP);

	/* Message when an object falls under the player */
	if (verbose && (cave->m_idx[by][bx] < 0) && !squelch_item_ok(j_ptr))
	{
		msg("You feel something roll beneath your feet.");
	}
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
   
	struct queue *queue = q_new(MAX_FLOOR_STACK);

	/* Push all objects on the square into the queue */
	for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
		q_push_ptr(queue, o_ptr);

	/* Set feature to an open door */
	square_force_floor(cave, y, x);
	square_add_door(cave, y, x, FALSE);
	
	/* Drop objects back onto the floor */
	while (q_len(queue) > 0)
	{
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

/*
 * Scatter some "great" objects near the player
 */
void acquirement(int y1, int x1, int level, int num, bool great)
{
	object_type *i_ptr;
	object_type object_type_body;

	/* Acquirement */
	while (num--)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Make a good (or great) object (if possible) */
		if (!make_object(cave, i_ptr, level, TRUE, great, TRUE, NULL, 0)) continue;

		i_ptr->origin = ORIGIN_ACQUIRE;
		i_ptr->origin_depth = player->depth;

		/* Drop the object */
		drop_near(cave, i_ptr, 0, y1, x1, TRUE);
	}
}


/*
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



/*
 * Describe an item in the inventory.
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


/*
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


/*
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


/*
 * Looks if "inscrip" is present on the given object.
 */
unsigned check_for_inscrip(const object_type *o_ptr, const char *inscrip)
{
	unsigned i = 0;
	const char *s;

	if (!o_ptr->note) return 0;

	s = quark_str(o_ptr->note);

	do {
		s = strstr(s, inscrip);
		if (!s) break;

		i++;
		s++;
	} while (s);

	return i;
}

/*** Object kind lookup functions ***/

/**
 * Return the object kind with the given `tval` and `sval`, or NULL.
 */
object_kind *lookup_kind(int tval, int sval)
{
	int k;

	/* Look for it */
	for (k = 0; k < z_info->k_max; k++)
	{
		object_kind *kind = &k_info[k];
		if (kind->tval == tval && kind->sval == sval)
			return kind;
	}

	/* Failure */
	msg("No object: %d:%d (%s)", tval, sval, tval_find_name(tval));
	return NULL;
}

struct object_kind *objkind_get(int tval, int sval) {
	return lookup_kind(tval, sval);
}

struct object_kind *objkind_byid(int kidx) {
	if (kidx < 1 || kidx > z_info->k_max)
		return NULL;
	return &k_info[kidx];
}


/*** Textual<->numeric conversion ***/

/**
 * Return the k_idx of the object kind with the given `tval` and name `name`.
 */
int lookup_name(int tval, const char *name)
{
	int k;

	/* Look for it */
	for (k = 1; k < z_info->k_max; k++)
	{
		object_kind *k_ptr = &k_info[k];
		char cmp_name[1024];

		if (!k_ptr || !k_ptr->name) continue;

		obj_desc_name_format(cmp_name, sizeof cmp_name, 0, k_ptr->name, 0, FALSE);

		/* Found a match */
		if (k_ptr->tval == tval && !my_stricmp(cmp_name, name))
			return k;
	}

	msg("No object (\"%s\",\"%s\")", tval_find_name(tval), name);
	return -1;
}

/**
 * Return the a_idx of the artifact with the given name
 */
int lookup_artifact_name(const char *name)
{
	int i;
	int a_idx = -1;
	
	/* Look for it */
	for (i = 1; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];

		/* Test for equality */
		if (a_ptr->name && streq(name, a_ptr->name))
			return i;
		
		/* Test for close matches */
		if (strlen(name) >= 3 && a_ptr->name && my_stristr(a_ptr->name, name) && a_idx == -1)
			a_idx = i;
	} 

	/* Return our best match */
	return a_idx;
}


/**
 * Return the numeric sval of the object kind with the given `tval` and name `name`.
 */
int lookup_sval(int tval, const char *name)
{
	int k;
	unsigned int r;

	if (sscanf(name, "%u", &r) == 1)
		return r;

	/* Look for it */
	for (k = 1; k < z_info->k_max; k++)
	{
		object_kind *k_ptr = &k_info[k];
		char cmp_name[1024];

		if (!k_ptr || !k_ptr->name) continue;

		obj_desc_name_format(cmp_name, sizeof cmp_name, 0, k_ptr->name, 0, FALSE);

		/* Found a match */
		if (k_ptr->tval == tval && !my_stricmp(cmp_name, name))
			return k_ptr->sval;
	}

	return -1;
}

/**
 * Sort comparator for objects using only tval and sval.
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 */
static int compare_types(const object_type *o1, const object_type *o2)
{
	if (o1->tval == o2->tval)
		return CMP(o1->sval, o2->sval);
	else
		return CMP(o1->tval, o2->tval);
}	
	

/**
 * Sort comparator for objects
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 *
 * The sort order is designed with the "list items" command in mind.
 */
int compare_items(const object_type *o1, const object_type *o2)
{

	/* unknown objects go at the end, order doesn't matter */
	if (is_unknown(o1) || is_unknown(o2)) {
		if (!is_unknown(o1)) return -1;
		return 1;
	}

	/* known artifacts will sort first */
	if (object_is_known_artifact(o1) && object_is_known_artifact(o2))
		return compare_types(o1, o2);
	if (object_is_known_artifact(o1)) return -1;
	if (object_is_known_artifact(o2)) return 1;

	/* unknown objects will sort next */
	if (!object_flavor_is_aware(o1) && !object_flavor_is_aware(o2))
		return compare_types(o1, o2);
	if (!object_flavor_is_aware(o1)) return -1;
	if (!object_flavor_is_aware(o2)) return 1;

	/* if only one of them is worthless, the other comes first */
	if (o1->kind->cost == 0 && o2->kind->cost != 0) return 1;
	if (o1->kind->cost != 0 && o2->kind->cost == 0) return -1;

	/* otherwise, just compare tvals and svals */
	/* NOTE: arguably there could be a better order than this */
	return compare_types(o1, o2);
}


/**
 * Helper to draw the Object Recall subwindow; this actually does the work.
 */
static void display_object_recall(object_type *o_ptr)
{
	char header[120];

	textblock *tb = object_info(o_ptr, OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

	clear_from(0);
	textui_textblock_place(tb, SCREEN_REGION, header);
	textblock_free(tb);
}


/**
 * This draws the Object Recall subwindow when displaying a particular object
 * (e.g. a helmet in the backpack, or a scroll on the ground)
 */
void display_object_idx_recall(s16b item)
{
	object_type *o_ptr = object_from_item_idx(item);
	display_object_recall(o_ptr);
}


/**
 * This draws the Object Recall subwindow when displaying a recalled item kind
 * (e.g. a generic ring of acid or a generic blade of chaos)
 */
void display_object_kind_recall(struct object_kind *kind)
{
	object_type object = { 0 };
	object_prep(&object, kind, 0, EXTREMIFY);
	if (kind->aware)
		object_notice_everything(&object);

	display_object_recall(&object);
}

/**
 * Display object recall modally and wait for a keypress.
 *
 * This is set up for use in look mode (see target_set_interactive_aux()).
 *
 * \param o_ptr is the object to be described.
 */
void display_object_recall_interactive(object_type *o_ptr)
{
	char header[120];
	textblock *tb;

	message_flush();

	tb = object_info(o_ptr, OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);
	textui_textblock_show(tb, SCREEN_REGION, header);
	textblock_free(tb);
}

/* Determine if an object has charges */
bool obj_has_charges(const object_type *o_ptr)
{
	if (!tval_can_have_charges(o_ptr)) return FALSE;

	if (o_ptr->pval <= 0) return FALSE;

	return TRUE;
}

/* Determine if an object is zappable */
bool obj_can_zap(const object_type *o_ptr)
{
	/* Any rods not charging? */
	if (tval_can_have_timeout(o_ptr) && number_charging(o_ptr) < o_ptr->number)
		return TRUE;

	return FALSE;
}

/* Determine if an object is activatable */
bool obj_is_activatable(const object_type *o_ptr)
{
	return object_effect(o_ptr) ? TRUE : FALSE;
}

/* Determine if an object can be activated now */
bool obj_can_activate(const object_type *o_ptr)
{
	if (obj_is_activatable(o_ptr))
	{
		/* Check the recharge */
		if (!o_ptr->timeout) return TRUE;
	}

	return FALSE;
}

/**
 * Check if an object can be used to refuel other objects.
 */
bool obj_can_refill(const object_type *obj)
{
	const object_type *light = equipped_item_by_slot_name(player, "light");
	bool no_fuel;

	/* Need fuel? */
	no_fuel = of_has(obj->flags, OF_NO_FUEL) ? TRUE : FALSE;

	/* A lantern can be refueled from a flask or another lantern */
	if (light->sval == SV_LIGHT_LANTERN) {
		if (tval_is_fuel(obj))
			return TRUE;
		else if (tval_is_light(obj) &&
			obj->sval == SV_LIGHT_LANTERN &&
			obj->timeout > 0 &&
			!no_fuel) 
			return TRUE;
	}

	return FALSE;
}


bool obj_can_browse(const object_type *o_ptr)
{
	return o_ptr->tval == player->class->spell_book;
}

bool obj_can_cast_from(const object_type *o_ptr)
{
	return obj_can_browse(o_ptr) &&
			spell_book_count_spells(o_ptr, spell_okay_to_cast) > 0;
}

bool obj_can_study(const object_type *o_ptr)
{
	return obj_can_browse(o_ptr) &&
			spell_book_count_spells(o_ptr, spell_okay_to_study) > 0;
}


/* Can only take off non-cursed items */
bool obj_can_takeoff(const object_type *o_ptr)
{
	return !cursed_p((bitflag *)o_ptr->flags);
}

/* Can only put on wieldable items */
bool obj_can_wear(const object_type *o_ptr)
{
	return (wield_slot(o_ptr) >= 0);
}

/* Can only fire an item with the right tval */
bool obj_can_fire(const object_type *o_ptr)
{
	return o_ptr->tval == player->state.ammo_tval;
}

/* Can has inscrip pls */
bool obj_has_inscrip(const object_type *o_ptr)
{
	return (o_ptr->note ? TRUE : FALSE);
}

bool obj_is_useable(const object_type *o_ptr)
{
	if (tval_is_useable(o_ptr))
		return TRUE;

	if (object_effect(o_ptr))
		return TRUE;

	if (tval_is_ammo(o_ptr))
		return o_ptr->tval == player->state.ammo_tval;

	return FALSE;
}

bool obj_is_used_aimed(const object_type *o_ptr)
{
	//return obj_needs_aim(o_ptr);
	int effect;
	if (tval_is_wand(o_ptr))
		return TRUE;

	if (tval_is_rod(o_ptr) && !object_flavor_is_aware(o_ptr))
		return TRUE;

	if (tval_is_ammo(o_ptr))
		return o_ptr->tval == player->state.ammo_tval;

	effect = object_effect(o_ptr);
	if (effect && effect_aim(effect))
		return TRUE;

	return FALSE;
}
bool obj_is_used_unaimed(const object_type *o_ptr)
{
	int effect;

	if (tval_is_staff(o_ptr) || tval_is_scroll(o_ptr) ||
		tval_is_potion(o_ptr) || tval_is_food(o_ptr))
		return TRUE;

	if (tval_is_rod(o_ptr) && !(!object_flavor_is_aware(o_ptr)))
		return TRUE;

	if (tval_is_ammo(o_ptr))
		return FALSE;

	effect = object_effect(o_ptr);
	if (!effect || !effect_aim(effect))
		return TRUE;

	return FALSE;
}

/*** Generic utility functions ***/

/*
 * Return an object's effect.
 */
u16b object_effect(const object_type *o_ptr)
{
	return o_ptr->effect;
}

/* Get an o_ptr from an item number */
object_type *object_from_item_idx(int item)
{
	if (item >= 0)
		return &player->gear[item];
	else
		return cave_object(cave, 0 - item);
}

/**
 * Return TRUE if the two objects are the same. Equality can be either be by value or by
 * identity (memory address). Value comparison is strict; all values must be equal.
 */
bool object_equals_object(const object_type *a, const object_type *b)
{
	int i;

	if (a == b)
		return TRUE;

#define MUST_EQUAL(p) if (a->p != b->p) return FALSE;
	MUST_EQUAL(kind);
	MUST_EQUAL(ego);
	MUST_EQUAL(artifact);

	MUST_EQUAL(iy);
	MUST_EQUAL(ix);

	MUST_EQUAL(tval);
	MUST_EQUAL(sval);

	MUST_EQUAL(weight);

	MUST_EQUAL(ac);
	MUST_EQUAL(to_a);
	MUST_EQUAL(to_h);
	MUST_EQUAL(to_d);

	MUST_EQUAL(dd);
	MUST_EQUAL(ds);

	MUST_EQUAL(timeout);
	MUST_EQUAL(effect);

	MUST_EQUAL(number);
	MUST_EQUAL(marked);
	MUST_EQUAL(ignore);

	MUST_EQUAL(next_o_idx);
	MUST_EQUAL(held_m_idx);
	MUST_EQUAL(mimicking_m_idx);

	MUST_EQUAL(origin);
	MUST_EQUAL(origin_depth);
	MUST_EQUAL(origin_xtra);

	MUST_EQUAL(note);
#undef MUST_EQUAL

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (a->modifiers[i] != b->modifiers[i])
			return FALSE;
	}

	if (!of_is_equal((const bitflag *)&a->flags, (const bitflag *)&b->flags))
		return FALSE;

	if (!of_is_equal((const bitflag *)&a->known_flags, (const bitflag *)&b->known_flags))
		return FALSE;

	return TRUE;
}

/**
 * Return the gear index of an object that matches the given object.
 *
 * \returns A valid gear index or -1 if the object cannot be found.
 */
int gear_index_matching_object(const object_type *o_ptr)
{
	int i;

	for (i = 0; i < MAX_GEAR; i++) {
		if (object_equals_object(o_ptr, &player->gear[i]))
			return i;
	}

	return -1;
}

/*
 * Does the given object need to be aimed?
 */ 
bool obj_needs_aim(object_type *o_ptr)
{
	int effect = object_effect(o_ptr);

	/* If the effect needs aiming, or if the object type needs
	   aiming, this object needs aiming. */
	return effect_aim(effect) || tval_is_ammo(o_ptr) ||
			tval_is_wand(o_ptr) ||
			(tval_is_rod(o_ptr) && !object_flavor_is_aware(o_ptr));
}

/*
 * Can the object fail if used?
 */
bool obj_can_fail(const struct object *o)
{
	if (tval_can_have_failure(o))
		return TRUE;

	return wield_slot(o) == -1 ? FALSE : TRUE;
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
 * [INVEN_PACK + QUIVER_SIZE + EQUIP_MAX_SLOTS + MAX_FLOOR_STACK],
 * though practically speaking much smaller numbers are likely.
 */
int scan_items(int *item_list, size_t item_list_max, int mode, item_tester tester)
{
	bool use_inven = ((mode & USE_INVEN) ? TRUE : FALSE);
	bool use_equip = ((mode & USE_EQUIP) ? TRUE : FALSE);
	bool use_floor = ((mode & USE_FLOOR) ? TRUE : FALSE);

	int floor_list[MAX_FLOOR_STACK];
	int floor_num;

	int i;
	size_t item_list_num = 0;

	if (use_inven)
	{
		for (i = 0; i < INVEN_PACK && item_list_num < item_list_max; i++)
		{
			if (item_test(tester, player->upkeep->inven[i]))
				item_list[item_list_num++] = player->upkeep->inven[i];
		}
	}

	if (use_equip)
	{
		for (i = 0; i < player->body.count && item_list_num < item_list_max;i++)
		{
			if (item_test(tester, slot_index(player, i)))
				item_list[item_list_num++] = slot_index(player, i);
		}

		for (i = 0; i < QUIVER_SIZE && item_list_num < item_list_max; i++)
		{
			if (item_test(tester, player->upkeep->quiver[i]))
				item_list[item_list_num++] = player->upkeep->quiver[i];
		}
	}

	/* Scan all non-gold objects in the grid */
	if (use_floor)
	{
		floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), player->py, player->px, 0x0B, tester);

		for (i = 0; i < floor_num && item_list_num < item_list_max; i++)
			item_list[item_list_num++] = -floor_list[i];
	}

	return item_list_num;
}


/* 
 * Check if the given item is available for the player to use. 
 *
 * 'mode' defines which areas we should look at, a la scan_items().
 */
bool item_is_available(int item, bool (*tester)(const object_type *), int mode)
{
	int item_list[INVEN_PACK + QUIVER_SIZE + EQUIP_MAX_SLOTS + MAX_FLOOR_STACK];
	int item_num;
	int i;

	item_num = scan_items(item_list, N_ELEMENTS(item_list), mode, tester);

	for (i = 0; i < item_num; i++)
	{
		if (item_list[i] == item)
			return TRUE;
	}

	return FALSE;
}

