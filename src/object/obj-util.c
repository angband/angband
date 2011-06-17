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
#include "effects.h"
#include "game-cmd.h"
#include "generate.h"
#include "history.h"
#include "object/inventory.h"
#include "prefs.h"
#include "spells.h"
#include "squelch.h"
#include "randname.h"
#include "object/tvalsval.h"
#include "z-queue.h"

struct object *o_list;

/*
 * Hold the titles of scrolls, 6 to 14 characters each, plus quotes.
 */
char scroll_adj[MAX_TITLES][18];

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

		/* HACK - Ordinary food is "boring" */
		if ((tval == TV_FOOD) && (k_info[i].sval < SV_FOOD_MIN_SHROOM))
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

	flavor_assign_fixed();

	flavor_assign_random(TV_RING);
	flavor_assign_random(TV_AMULET);
	flavor_assign_random(TV_STAFF);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_ROD);
	flavor_assign_random(TV_FOOD);
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



#ifdef ALLOW_BORG_GRAPHICS
extern void init_translate_visuals(void);
#endif /* ALLOW_BORG_GRAPHICS */


/*
 * Reset the "visual" lists
 *
 * This involves resetting various things to their "default" state.
 *
 * If the "prefs" flag is TRUE, then we will also load the appropriate
 * "user pref file" based on the current setting of the "use_graphics"
 * flag.  This is useful for switching "graphics" on/off.
 */
/* XXX this does not belong here */
void reset_visuals(bool load_prefs)
{
	int i;
	struct flavor *f;

	/* Extract default attr/char code for features */
	for (i = 0; i < z_info->f_max; i++)
	{
		int j;
		feature_type *f_ptr = &f_info[i];

		/* Assume we will use the underlying values */
		for (j = 0; j < FEAT_LIGHTING_MAX; j++)
		{
			f_ptr->x_attr[j] = f_ptr->d_attr;
			f_ptr->x_char[j] = f_ptr->d_char;
		}
	}

	/* Extract default attr/char code for objects */
	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Default attr/char */
		k_ptr->x_attr = k_ptr->d_attr;
		k_ptr->x_char = k_ptr->d_char;
	}

	/* Extract default attr/char code for monsters */
	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Default attr/char */
		r_ptr->x_attr = r_ptr->d_attr;
		r_ptr->x_char = r_ptr->d_char;
	}

	/* Extract default attr/char code for flavors */
	for (f = flavors; f; f = f->next)
	{
		f->x_attr = f->d_attr;
		f->x_char = f->d_char;
	}

	/* Extract attr/chars for inventory objects (by tval) */
	for (i = 0; i < (int)N_ELEMENTS(tval_to_attr); i++)
	{
		/* Default to white */
		tval_to_attr[i] = TERM_WHITE;
	}

	if (!load_prefs)
		return;



	/* Graphic symbols */
	if (use_graphics)
		process_pref_file("graf.prf", FALSE, FALSE);

	/* Normal symbols */
	else
		process_pref_file("font.prf", FALSE, FALSE);

#ifdef ALLOW_BORG_GRAPHICS
	/* Initialize the translation table for the borg */
	init_translate_visuals();
#endif /* ALLOW_BORG_GRAPHICS */
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
 * Convert an inventory index into a one character label.
 *
 * Note that the label does NOT distinguish inven/equip.
 */
char index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!p_ptr->inventory[i].kind) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Convert a label into the index of a item in the "equip".
 *
 * Return "-1" if the label does not indicate a real item.
 */
s16b label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= ALL_INVEN_TOTAL)) return (-1);
	if (i == INVEN_TOTAL) return (-1);

	/* Empty slots can never be chosen */
	if (!p_ptr->inventory[i].kind) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
bool wearable_p(const object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_LIGHT:
		case TV_AMULET:
		case TV_RING: return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

int get_inscribed_ammo_slot(const object_type *o_ptr)
{
	char *s;
	if (!o_ptr->note) return 0;
	s = strchr(quark_str(o_ptr->note), 'f');
	if (!s || s[1] < '0' || s[1] > '9') return 0;

	return QUIVER_START + (s[1] - '0');
}

/**
 * Used by wield_slot() to find an appopriate slot for ammo. See wield_slot()
 * for information on what this returns.
 */
s16b wield_slot_ammo(const object_type *o_ptr)
{
	s16b i, open = 0;

	/* If the ammo is inscribed with a slot number, we'll try to put it in */
	/* that slot, if possible. */
	i = get_inscribed_ammo_slot(o_ptr);
	if (i && !p_ptr->inventory[i].kind) return i;

	for (i = QUIVER_START; i < QUIVER_END; i++)
	{
		if (!p_ptr->inventory[i].kind)
		{
			/* Save the open slot if we haven't found one already */
			if (!open) open = i;
			continue;
		}

		/* If ammo is cursed we can't stack it */
		if (cursed_p(p_ptr->inventory[i].flags)) continue;

		/* If they are stackable, we'll use this slot for sure */
		if (object_similar(&p_ptr->inventory[i], o_ptr,
			OSTACK_QUIVER)) return i;
	}

	/* If not absorbed, return an open slot (or QUIVER_START if no room) */
	return open ? open : QUIVER_START;
}

/**
 * Determine which equipment slot (if any) an item likes. The slot might (or
 * might not) be open, but it is a slot which the object could be equipped in.
 *
 * For items where multiple slots could work (e.g. ammo or rings), the function
 * will try to a return a stackable slot first (only for ammo), then an open
 * slot if possible, and finally a used (but valid) slot if necessary.
 */
s16b wield_slot(const object_type *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD: return (INVEN_WIELD);

		case TV_BOW: return (INVEN_BOW);

		case TV_RING:
			return p_ptr->inventory[INVEN_RIGHT].kind ? INVEN_LEFT : INVEN_RIGHT;

		case TV_AMULET: return (INVEN_NECK);

		case TV_LIGHT: return (INVEN_LIGHT);

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR: return (INVEN_BODY);

		case TV_CLOAK: return (INVEN_OUTER);

		case TV_SHIELD: return (INVEN_ARM);

		case TV_CROWN:
		case TV_HELM: return (INVEN_HEAD);

		case TV_GLOVES: return (INVEN_HANDS);

		case TV_BOOTS: return (INVEN_FEET);

		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT: return wield_slot_ammo(o_ptr);
	}

	/* No slot available */
	return (-1);
}


/*
 * \returns whether item o_ptr will fit in slot 'slot'
 */
bool slot_can_wield_item(int slot, const object_type *o_ptr)
{
	if (o_ptr->tval == TV_RING)
		return (slot == INVEN_LEFT || slot == INVEN_RIGHT) ? TRUE : FALSE;
	else if (obj_is_ammo(o_ptr))
		return (slot >= QUIVER_START && slot < QUIVER_END) ? TRUE : FALSE;
	else
		return (wield_slot(o_ptr) == slot) ? TRUE : FALSE;
}


/*
 * Return a string mentioning how a given item is carried
 */
const char *mention_use(int slot)
{
	switch (slot)
	{
		case INVEN_WIELD:
		{
			if (adj_str_hold[p_ptr->state.stat_ind[A_STR]] < p_ptr->inventory[slot].weight / 10)
				return "Just lifting";
			else
				return "Wielding";
		}

		case INVEN_BOW:
		{
			if (adj_str_hold[p_ptr->state.stat_ind[A_STR]] < p_ptr->inventory[slot].weight / 10)
				return "Just holding";
			else
				return "Shooting";
		}

		case INVEN_LEFT:  return "On left hand";
		case INVEN_RIGHT: return "On right hand";
		case INVEN_NECK:  return "Around neck";
		case INVEN_LIGHT: return "Light source";
		case INVEN_BODY:  return "On body";
		case INVEN_OUTER: return "About body";
		case INVEN_ARM:   return "On arm";
		case INVEN_HEAD:  return "On head";
		case INVEN_HANDS: return "On hands";
		case INVEN_FEET:  return "On feet";

		case QUIVER_START + 0: return "In quiver [f0]";
		case QUIVER_START + 1: return "In quiver [f1]";
		case QUIVER_START + 2: return "In quiver [f2]";
		case QUIVER_START + 3: return "In quiver [f3]";
		case QUIVER_START + 4: return "In quiver [f4]";
		case QUIVER_START + 5: return "In quiver [f5]";
		case QUIVER_START + 6: return "In quiver [f6]";
		case QUIVER_START + 7: return "In quiver [f7]";
		case QUIVER_START + 8: return "In quiver [f8]";
		case QUIVER_START + 9: return "In quiver [f9]";
	}

	/*if (slot >= QUIVER_START && slot < QUIVER_END)
		return "In quiver";*/

	return "In pack";
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
const char *describe_use(int i)
{
	const char *p;

	switch (i)
	{
		case INVEN_WIELD: p = "attacking monsters with"; break;
		case INVEN_BOW:   p = "shooting missiles with"; break;
		case INVEN_LEFT:  p = "wearing on your left hand"; break;
		case INVEN_RIGHT: p = "wearing on your right hand"; break;
		case INVEN_NECK:  p = "wearing around your neck"; break;
		case INVEN_LIGHT: p = "using to light the way"; break;
		case INVEN_BODY:  p = "wearing on your body"; break;
		case INVEN_OUTER: p = "wearing on your back"; break;
		case INVEN_ARM:   p = "wearing on your arm"; break;
		case INVEN_HEAD:  p = "wearing on your head"; break;
		case INVEN_HANDS: p = "wearing on your hands"; break;
		case INVEN_FEET:  p = "wearing on your feet"; break;
		default:          p = "carrying in your pack"; break;
	}

	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->state.stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->state.stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just holding";
		}
	}

	/* Return the result */
	return p;
}





/*
 * Check an item against the item tester info
 */
bool item_tester_okay(const object_type *o_ptr)
{
	/* Hack -- allow listing empty slots */
	if (item_tester_full) return (TRUE);

	/* Require an item */
	if (!o_ptr->kind) return (FALSE);

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_GOLD) return (FALSE);

	/* Check the tval */
	if (item_tester_tval)
	{
		if (item_tester_tval != o_ptr->tval) return (FALSE);
	}

	/* Check the hook */
	if (item_tester_hook)
	{
		if (!(*item_tester_hook)(o_ptr)) return (FALSE);
	}

	/* Assume okay */
	return (TRUE);
}



/*
 * Get the indexes of objects at a given floor location. -TNB-
 *
 * Return the number of object indexes acquired.
 *
 * Valid flags are any combination of the bits:
 *   0x01 -- Verify item tester
 *   0x02 -- Marked/visible items only
 *   0x04 -- Only the top item
 */
int scan_floor(int *items, int max_size, int y, int x, int mode)
{
	int this_o_idx, next_o_idx;

	int num = 0;

	/* Sanity */
	if (!in_bounds(y, x)) return 0;

	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* XXX Hack -- Enforce limit */
		if (num >= max_size) break;


		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Item tester */
		if ((mode & 0x01) && !item_tester_okay(o_ptr)) continue;

		/* Marked */
		if ((mode & 0x02) && (!o_ptr->marked || squelch_item_ok(o_ptr)))
			continue;

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
	j_ptr = object_byid(o_idx);

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
			o_ptr = object_byid(this_o_idx);

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
					i_ptr = object_byid(prev_o_idx);

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
			o_ptr = object_byid(this_o_idx);

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
					i_ptr = object_byid(prev_o_idx);

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
	j_ptr = object_byid(o_idx);

	/* Dungeon floor */
	if (!(j_ptr->held_m_idx))
	{
		int y, x;

		/* Location */
		y = j_ptr->iy;
		x = j_ptr->ix;

		cave_light_spot(cave, y, x);
	}

	/* Wipe the object */
	object_wipe(j_ptr);

	/* Count objects */
	o_cnt--;

	/* Stop tracking deleted objects if necessary */
	if (tracked_object_is(0 - o_idx))
	{
		track_object(NO_OBJECT);
	}
}


/*
 * Deletes all objects at given location
 */
void delete_object(int y, int x)
{
	s16b this_o_idx, next_o_idx = 0;

	/* Paranoia */
	if (!in_bounds(y, x)) return;

	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Preserve unseen artifacts */
		if (o_ptr->artifact && !object_was_sensed(o_ptr))
			o_ptr->artifact->created = FALSE;

		/* Wipe the object */
		object_wipe(o_ptr);

		/* Count objects */
		o_cnt--;
	}

	/* Objects are gone */
	cave->o_idx[y][x] = 0;

	/* Visual update */
	cave_light_spot(cave, y, x);
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
	for (i = 1; i < o_max; i++)
	{
		/* Get the object */
		o_ptr = object_byid(i);

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
	o_ptr = object_byid(i1);


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
	}


	/* Hack -- move object */
	COPY(object_byid(i2), object_byid(i1), object_type);

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
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, y, x, cnt;

	int cur_lev, cur_dis, chance;


	/* Reorder objects when not passed a size */
	if (!size)
	{
		/* Excise dead objects (backwards!) */
		for (i = o_max - 1; i >= 1; i--)
		{
			object_type *o_ptr = object_byid(i);
			if (o_ptr->kind) continue;

			/* Move last object into open hole */
			compact_objects_aux(o_max - 1, i);

			/* Compress "o_max" */
			o_max--;
		}

		return;
	}


	/* Message */
	msg("Compacting objects...");

	/*** Try destroying objects ***/

	/* First do gold */
	for (i = 1; (i < o_max) && (size); i++)
	{
		object_type *o_ptr = object_byid(i);

		/* Nuke gold or squelched items */
		if (o_ptr->tval == TV_GOLD || squelch_item_ok(o_ptr))
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
		for (i = 1; (i < o_max) && (size); i++)
		{
			object_type *o_ptr = object_byid(i);
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
void wipe_o_list(struct cave *c)
{
	int i;

	/* Delete the existing objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = object_byid(i);
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
			m_ptr = cave_monster(cave, o_ptr->held_m_idx);

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

	/* Reset "o_max" */
	o_max = 1;

	/* Reset "o_cnt" */
	o_cnt = 0;
}


/*
 * Get and return the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b o_pop(void)
{
	int i;


	/* Initial allocation */
	if (o_max < z_info->o_max)
	{
		/* Get next space */
		i = o_max;

		/* Expand object array */
		o_max++;

		/* Count objects */
		o_cnt++;

		/* Use this object */
		return (i);
	}


	/* Recycle dead objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = object_byid(i);
		if (o_ptr->kind) continue;

		/* Count objects */
		o_cnt++;

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
		return object_byid(o_idx);

	/* No object */
	return (NULL);
}


/*
 * Get the next object in a stack or NULL if there isn't one.
 */
object_type *get_next_object(const object_type *o_ptr)
{
	if (o_ptr->next_o_idx)
		return object_byid(o_ptr->next_o_idx);

	/* No more objects */
	return NULL;
}



/*
 * Determine if a weapon is 'blessed'
 */
bool is_blessed(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];

	/* Get the flags */
	object_flags(o_ptr, f);

	/* Is the object blessed? */
	return (of_has(f, OF_BLESSED) ? TRUE : FALSE);
}



/*
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b object_value_base(const object_type *o_ptr)
{
	/* Use template cost for aware objects */
	if (object_flavor_is_aware(o_ptr) || o_ptr->ident & IDENT_STORE)
		return o_ptr->kind->cost;

	/* Analyze the type */
	switch (o_ptr->tval)
	{
		case TV_FOOD:
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
	int b = 1;
	static file_mode pricing_mode = MODE_WRITE;

	if (wearable_p(o_ptr))
	{
 		char buf[1024];
		ang_file *log_file = NULL;

		if (verbose)
		{
			path_build(buf, sizeof(buf), ANGBAND_DIR_USER, 					"pricing.log");
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

		if ( (o_ptr->tval == TV_SHOT) || (o_ptr->tval == TV_ARROW) ||
  			(o_ptr->tval == TV_BOLT) || ((o_ptr->tval == TV_LIGHT)
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
	switch (o_ptr->tval)
	{
		/* Wands/Staffs */
		case TV_WAND:
		case TV_STAFF:
		{
			int charges;

			total_value = value * qty;

			/* Calculate number of charges, rounded up */
			charges = o_ptr->pval[DEFAULT_PVAL]
				* qty / o_ptr->number;
			if ((o_ptr->pval[DEFAULT_PVAL] * qty) % o_ptr->number != 0)
				charges++;

			/* Pay extra for charges, depending on standard number of charges */
			total_value += value * charges / 20;

			/* Done */
			break;
		}

		default:
		{
			total_value = value * qty;
			break;
		}
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


/*
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
bool object_similar(const object_type *o_ptr, const object_type *j_ptr,
	object_stack_t mode)
{
	int i;
	int total = o_ptr->number + j_ptr->number;

	/* Check against stacking limit - except in stores which absorb anyway */
	if (!(mode & OSTACK_STORE) && (total >= MAX_STACK_SIZE)) return FALSE;

	/* Hack -- identical items cannot be stacked */
	if (o_ptr == j_ptr) return FALSE;

	/* Require identical object kinds */
	if (o_ptr->kind != j_ptr->kind) return FALSE;

	/* Different flags don't stack */
	if (!of_is_equal(o_ptr->flags, j_ptr->flags)) return FALSE;

	/* Artifacts never stack */
	if (o_ptr->artifact || j_ptr->artifact) return FALSE;

	/* Analyze the items */
	switch (o_ptr->tval)
	{
		/* Chests never stack */
		case TV_CHEST:
		{
			/* Never okay */
			return FALSE;
		}

		/* Food, potions, scrolls and rods all stack nicely */
		case TV_FOOD:
		case TV_POTION:
		case TV_SCROLL:
		case TV_ROD:
		{
			/* Since the kinds are identical, either both will be
			aware or both will be unaware */
			break;
		}

		/* Gold, staves and wands stack most of the time */
		case TV_STAFF:
		case TV_WAND:
		case TV_GOLD:
		{
			/* Too much gold or too many charges */
			if (o_ptr->pval[DEFAULT_PVAL] + j_ptr->pval[DEFAULT_PVAL] > MAX_PVAL)
				return FALSE;

			/* ... otherwise ok */
			else break;
		}

		/* Weapons, ammo, armour, jewelry, lights */
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_RING:
		case TV_AMULET:
		case TV_LIGHT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			/* Require identical values */
			if (o_ptr->ac != j_ptr->ac) return FALSE;
			if (o_ptr->dd != j_ptr->dd) return FALSE;
			if (o_ptr->ds != j_ptr->ds) return FALSE;

			/* Require identical bonuses */
			if (o_ptr->to_h != j_ptr->to_h) return FALSE;
			if (o_ptr->to_d != j_ptr->to_d) return FALSE;
			if (o_ptr->to_a != j_ptr->to_a) return FALSE;

			/* Require all identical pvals */
			for (i = 0; i < MAX_PVALS; i++)
				if (o_ptr->pval[i] != j_ptr->pval[i])
					return (FALSE);

			/* Require identical ego-item types */
			if (o_ptr->ego != j_ptr->ego) return (FALSE);

			/* Hack - Never stack recharging wearables ... */
			if ((o_ptr->timeout || j_ptr->timeout) &&
					o_ptr->tval != TV_LIGHT) return FALSE;

			/* ... and lights must have same amount of fuel */
			else if ((o_ptr->timeout != j_ptr->timeout) &&
					o_ptr->tval == TV_LIGHT) return FALSE;

			/* Prevent unIDd items stacking in the object list */
			if (mode & OSTACK_LIST &&
					!(o_ptr->ident & j_ptr->ident & IDENT_KNOWN)) return FALSE;

			/* Probably okay */
			break;
		}

		/* Anything else */
		default:
		{
			/* Probably okay */
			break;
		}
	}

	/* Require compatible inscriptions */
	if (o_ptr->note && j_ptr->note && (o_ptr->note != j_ptr->note))
		return FALSE;

	/* They must be similar enough */
	return (TRUE);
}


/*
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
void object_absorb(object_type *o_ptr, const object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Add together the item counts */
	o_ptr->number = ((total < MAX_STACK_SIZE) ? total : (MAX_STACK_SIZE - 1));

	/* Blend all knowledge */
	o_ptr->ident |= (j_ptr->ident & ~IDENT_EMPTY);
	of_union(o_ptr->known_flags, j_ptr->known_flags);

	/* Merge inscriptions */
	if (j_ptr->note)
		o_ptr->note = j_ptr->note;

	/* Combine timeouts for rod stacking */
	if (o_ptr->tval == TV_ROD)
		o_ptr->timeout += j_ptr->timeout;

	/* Combine pvals for wands and staves */
	if (o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF ||
			o_ptr->tval == TV_GOLD)
	{
		int total = o_ptr->pval[DEFAULT_PVAL] + j_ptr->pval[DEFAULT_PVAL];
		o_ptr->pval[DEFAULT_PVAL] = total >= MAX_PVAL ? MAX_PVAL : total;
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
	int charge_time = randcalc(src->kind->time, 0, AVERAGE), max_time;

	/* Get a copy of the object */
	object_copy(dst, src);

	/* Modify quantity */
	dst->number = amt;
	dst->note = src->note;

	/* 
	 * If the item has charges/timeouts, set them to the correct level 
	 * too. We split off the same amount as distribute_charges.
	 */
	if (src->tval == TV_WAND || src->tval == TV_STAFF)
	{
		dst->pval[DEFAULT_PVAL] =
			src->pval[DEFAULT_PVAL] * amt / src->number;
	}

	if (src->tval == TV_ROD)
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
		o_ptr = object_byid(this_o_idx);

		if (squelch_item_ok(o_ptr))
			squelch_idx = this_o_idx;
	}

	return squelch_idx;
}



/*
 * Let the floor carry an object, deleting old squelched items if necessary
 */
s16b floor_carry(struct cave *c, int y, int x, object_type *j_ptr)
{
	int n = 0;

	s16b o_idx;

	s16b this_o_idx, next_o_idx = 0;


	/* Scan objects in that grid for combination */
	for (this_o_idx = c->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr = object_byid(this_o_idx);

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
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(o_idx);

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

		cave_note_spot(c, y, x);
		cave_light_spot(c, y, x);
	}

	/* Result */
	return (o_idx);
}


/*
 * Let an object fall to the ground at or near a location.
 *
 * The initial location is assumed to be "in_bounds_fully()".
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
void drop_near(struct cave *c, object_type *j_ptr, int chance, int y, int x, bool verbose)
{
	int i, k, n, d, s;

	int bs, bn;
	int by, bx;
	int dy, dx;
	int ty, tx;

	object_type *o_ptr;

	char o_name[80];

	bool flag = FALSE;

	bool plural = FALSE;


	/* Extract plural */
	if (j_ptr->number != 1) plural = TRUE;

	/* Describe object */
	object_desc(o_name, sizeof(o_name), j_ptr, ODESC_BASE);


	/* Handle normal "breakage" */
	if (!j_ptr->artifact && (randint0(100) < chance))
	{
		/* Message */
		msg("The %s break%s.", o_name, PLURAL(plural));

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
			if (!in_bounds_fully(ty, tx)) continue;

			/* Require line of sight */
			if (!los(y, x, ty, tx)) continue;

			/* Require floor space */
			if (cave->feat[ty][tx] != FEAT_FLOOR) continue;

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
		msg("The %s disappear%s.", o_name, PLURAL(plural));

		/* Debug */
		if (p_ptr->wizard) msg("Breakage (no floor space).");

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
		if (cave->feat[ty][tx] != FEAT_FLOOR) continue;

		/* Bounce to that location */
		by = ty;
		bx = tx;

		/* Require floor space */
		if (!cave_clean_bold(by, bx)) continue;

		/* Okay */
		flag = TRUE;
	}


	/* Give it to the floor */
	if (!floor_carry(c, by, bx, j_ptr))
	{
		/* Message */
		msg("The %s disappear%s.", o_name, PLURAL(plural));

		/* Debug */
		if (p_ptr->wizard) msg("Breakage (too many objects).");

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
	int feat_old = cave->feat[y][x];

	object_type *o_ptr;
   
	struct queue *queue = q_new(MAX_FLOOR_STACK);

	/* Push all objects on the square into the queue */
	for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
		q_push_ptr(queue, o_ptr);

	/* Set feature to an open door */
	cave_set_feat(cave, y, x, FEAT_OPEN);
	
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
	cave_set_feat(cave, y, x, feat_old);
	
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
		make_object(cave, i_ptr, level, TRUE, great);
		if (!i_ptr->kind) continue;
		i_ptr->origin = ORIGIN_ACQUIRE;
		i_ptr->origin_depth = p_ptr->depth;

		/* Drop the object */
		drop_near(cave, i_ptr, 0, y1, x1, TRUE);
	}
}


/*
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Require staff/wand */
	if ((o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return;

	/* Require known item */
	if (!object_is_known(o_ptr)) return;

	/* Print a message */
	msg("You have %d charge%s remaining.", o_ptr->pval[DEFAULT_PVAL],
	    (o_ptr->pval[DEFAULT_PVAL] != 1) ? "s" : "");
}


/*
 * Describe an item in the inventory. Note: only called when an item is 
 * dropped, used, or otherwise deleted from the inventory
 */
void inven_item_describe(int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	char o_name[80];

	if (o_ptr->artifact && 
		(object_is_known(o_ptr) || object_name_is_visible(o_ptr)))
	{
		/* Get a description */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL | ODESC_SINGULAR);

		/* Print a message */
		msg("You no longer have the %s (%c).", o_name, index_to_label(item));
	}
	else
	{
		/* Get a description */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Print a message */
		msg("You have %s (%c).", o_name, index_to_label(item));
	}
}


/*
 * Increase the "number" of an item in the inventory
 */
void inven_item_increase(int item, int num)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number and weight */
	if (num)
	{
		/* Add the number */
		o_ptr->number += num;

		/* Add the weight */
		p_ptr->total_weight += (num * o_ptr->weight);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana XXX */
		p_ptr->update |= (PU_MANA);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
	}
}


/**
 * Save the size of the quiver.
 */
void save_quiver_size(struct player *p)
{
	int i, count = 0;
	for (i = QUIVER_START; i < QUIVER_END; i++)
		if (p->inventory[i].kind)
			count += p->inventory[i].number;

	p->quiver_size = count;
	p->quiver_slots = (count + 98) / 99;
	p->quiver_remainder = count % 99;
}


/**
 * Compare ammunition from slots (0-9); used for sorting.
 *
 * \returns -1 if slot1 should come first, 1 if slot2 should come first, or 0.
 */
int compare_ammo(int slot1, int slot2)
{
	/* Right now there is no sorting criteria */
	return 0;
}

/**
 * Swap ammunition between quiver slots (0-9).
 */
void swap_quiver_slots(int slot1, int slot2)
{
	int i = slot1 + QUIVER_START;
	int j = slot2 + QUIVER_START;
	object_type o;

	object_copy(&o, &p_ptr->inventory[i]);
	object_copy(&p_ptr->inventory[i], &p_ptr->inventory[j]);
	object_copy(&p_ptr->inventory[j], &o);

	/* Update object_idx if necessary */
	if (tracked_object_is(i))
	{
		track_object(j);
	}

	if (tracked_object_is(j))
	{
		track_object(i);
	}
}

/**
 * Sorts the quiver--ammunition inscribed with @fN prefers to end up in quiver
 * slot N.
 */
void sort_quiver(void)
{
	/* Ammo slots go from 0-9; these indices correspond to the range of
	 * (QUIVER_START) - (QUIVER_END-1) in inventory[].
	 */
	bool locked[QUIVER_SIZE] = {FALSE, FALSE, FALSE, FALSE, FALSE,
								FALSE, FALSE, FALSE, FALSE, FALSE};
	int desired[QUIVER_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int i, j, k;
	object_type *o_ptr;

	/* Here we figure out which slots have inscribed ammo, and whether that
	 * ammo is already in the slot it "wants" to be in or not.
	 */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		j = QUIVER_START + i;
		o_ptr = &p_ptr->inventory[j];

		/* Skip this slot if it doesn't have ammo */
		if (!o_ptr->kind) continue;

		/* Figure out which slot this ammo prefers, if any */
		k = get_inscribed_ammo_slot(o_ptr);
		if (!k) continue;

		k -= QUIVER_START;
		if (k == i) locked[i] = TRUE;
		if (desired[k] < 0) desired[k] = i;
	}

	/* For items which had a preference that was not fulfilled, we will swap
	 * them into the slot as long as it isn't already locked.
	 */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		if (locked[i] || desired[i] < 0) continue;

		/* item in slot 'desired[i]' desires to be in slot 'i' */
		swap_quiver_slots(desired[i], i);
		locked[i] = TRUE;
	}

	/* Now we need to compact ammo which isn't in a preferrred slot towards the
	 * "front" of the quiver */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		/* If the slot isn't empty, skip it */
		if (p_ptr->inventory[QUIVER_START + i].kind) continue;

		/* Start from the end and find an unlocked item to put here. */
		for (j=QUIVER_SIZE - 1; j > i; j--)
		{
			if (!p_ptr->inventory[QUIVER_START + j].kind || locked[j]) continue;
			swap_quiver_slots(i, j);
			break;
		}
	}

	/* Now we will sort all other ammo using a simple insertion sort */
	for (i=0; i < QUIVER_SIZE; i++)
	{
		k = i;
		if (!locked[k])
			for (j = i + 1; j < QUIVER_SIZE; j++)
				if (!locked[j] && compare_ammo(k, j) > 0)
					swap_quiver_slots(j, k);
	}
}

/*
 * Shifts ammo at or above the item slot towards the end of the quiver, making
 * room for a new piece of ammo.
 */
void open_quiver_slot(int slot)
{
	int i, pref;
	int dest = QUIVER_END - 1;

	/* This should only be used on ammunition */
	if (slot < QUIVER_START) return;

	/* Quiver is full */
	if (p_ptr->inventory[QUIVER_END - 1].kind) return;

	/* Find the first open quiver slot */
	while (p_ptr->inventory[dest].kind) dest++;

	/* Swap things with the space one higher (essentially moving the open space
	 * towards our goal slot. */
	for (i = dest - 1; i >= slot; i--)
	{
		/* If we have an item with an inscribed location (and it's in */
		/* that location) then we won't move it. */
		pref = get_inscribed_ammo_slot(&p_ptr->inventory[i]);
		if (i != slot && pref && pref == i) continue;

		/* Update object_idx if necessary */
		if (tracked_object_is(i))
		{
			track_object(dest);
		}

		/* Copy the item up and wipe the old slot */
		COPY(&p_ptr->inventory[dest], &p_ptr->inventory[i], object_type);
		dest = i;
		object_wipe(&p_ptr->inventory[dest]);


	}
}


/*
 * Erase an inventory slot if it has no more items
 */
void inven_item_optimize(int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];
	int i, j, slot, limit;

	/* Save a possibly new quiver size */
	if (item >= QUIVER_START) save_quiver_size(p_ptr);

	/* Only optimize real items which are empty */
	if (!o_ptr->kind || o_ptr->number) return;

	/* Stop tracking erased item if necessary */
	if (tracked_object_is(item))
	{
		track_object(NO_OBJECT);
	}

	/* Items in the pack are treated differently from other items */
	if (item < INVEN_WIELD)
	{
		p_ptr->inven_cnt--;
		p_ptr->redraw |= PR_INVEN;
		limit = INVEN_MAX_PACK;
	}

	/* Items in the quiver and equipped items are (mostly) treated similarly */
	else
	{
		p_ptr->equip_cnt--;
		p_ptr->redraw |= PR_EQUIP;
		limit = item >= QUIVER_START ? QUIVER_END : 0;
	}

	/* If the item is equipped (but not in the quiver), there is no need to */
	/* slide other items. Bonuses and such will need to be recalculated */
	if (!limit)
	{
		/* Erase the empty slot */
		object_wipe(&p_ptr->inventory[item]);
		
		/* Recalculate stuff */
		p_ptr->update |= (PU_BONUS);
		p_ptr->update |= (PU_TORCH);
		p_ptr->update |= (PU_MANA);
		
		return;
	}

	/* Slide everything down */
	for (j = item, i = item + 1; i < limit; i++)
	{
		if (limit == QUIVER_END && p_ptr->inventory[i].kind)
		{
			/* If we have an item with an inscribed location (and it's in */
			/* that location) then we won't move it. */
			slot = get_inscribed_ammo_slot(&p_ptr->inventory[i]);
			if (slot && slot == i)
				continue;
		}
		COPY(&p_ptr->inventory[j], &p_ptr->inventory[i], object_type);

		/* Update object_idx if necessary */
		if (tracked_object_is(i))
		{
			track_object(j);
		}

		j = i;
	}

	/* Reorder the quiver if necessary */
	if (item >= QUIVER_START) sort_quiver();

	/* Wipe the left-over object on the end */
	object_wipe(&p_ptr->inventory[j]);

	/* Inventory has changed, so disable repeat command */ 
	cmd_disable_repeat();
}


/*
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(int item)
{
	object_type *o_ptr = object_byid(item);

	/* Require staff/wand */
	if ((o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return;

	/* Require known item */
	if (!object_is_known(o_ptr)) return;

	/* Print a message */
	msg("There %s %d charge%s remaining.",
	    (o_ptr->pval[DEFAULT_PVAL] != 1) ? "are" : "is",
	     o_ptr->pval[DEFAULT_PVAL],
	    (o_ptr->pval[DEFAULT_PVAL] != 1) ? "s" : "");
}



/*
 * Describe an item in the inventory.
 */
void floor_item_describe(int item)
{
	object_type *o_ptr = object_byid(item);

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
	object_type *o_ptr = object_byid(item);

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
	object_type *o_ptr = object_byid(item);

	/* Paranoia -- be sure it exists */
	if (!o_ptr->kind) return;

	/* Only optimize empty items */
	if (o_ptr->number) return;

	/* Delete the object */
	delete_object_idx(item);
}


/*
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(const object_type *o_ptr)
{
	/* Empty slot? */
	if (p_ptr->inven_cnt < INVEN_MAX_PACK) return TRUE;

	/* Check if it can stack */
	if (inven_stack_okay(o_ptr)) return TRUE;

	/* Nope */
	return FALSE;
}

/*
 * Check to see if an item is stackable in the inventory
 */
bool inven_stack_okay(const object_type *o_ptr)
{
	/* Similar slot? */
	int j;

	/* If our pack is full and we're adding too many missiles, there won't be
	 * enough room in the quiver, so don't check it. */
	int limit;

	if (!pack_is_full())
		/* The pack has more room */
		limit = ALL_INVEN_TOTAL;
	else if (p_ptr->quiver_remainder == 0)
		/* Quiver already maxed out */
		limit = INVEN_PACK;
	else if (p_ptr->quiver_remainder + o_ptr->number > 99)
		/* Too much new ammo */
		limit = INVEN_PACK;
	else
		limit = ALL_INVEN_TOTAL;

	for (j = 0; j < limit; j++)
	{
		object_type *j_ptr = &p_ptr->inventory[j];

		/* Skip equipped items and non-objects */
		if (j >= INVEN_PACK && j < QUIVER_START) continue;
		if (!j_ptr->kind) continue;

		/* Check if the two items can be combined */
		if (object_similar(j_ptr, o_ptr, OSTACK_PACK)) return (TRUE);
	}
	return (FALSE);
}


/*
 * Add an item to the players inventory, and return the slot used.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so, using "object_similar()" and "object_absorb()", else,
 * the item will be placed into the "proper" location in the inventory.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined.  This may be tricky.  See "dungeon.c" for info.
 *
 * Note that this code must remove any location/stack information
 * from the object once it is placed into the inventory.
 */
extern s16b inven_carry(struct player *p, struct object *o)
{
	int i, j, k;
	int n = -1;

	object_type *j_ptr;

	/* Apply an autoinscription */
	apply_autoinscription(o);

	/* Check for combining */
	for (j = 0; j < INVEN_PACK; j++)
	{
		j_ptr = &p->inventory[j];
		if (!j_ptr->kind) continue;

		/* Hack -- track last item */
		n = j;

		/* Check if the two items can be combined */
		if (object_similar(j_ptr, o, OSTACK_PACK))
		{
			/* Combine the items */
			object_absorb(j_ptr, o);

			/* Increase the weight */
			p->total_weight += (o->number * o->weight);

			/* Recalculate bonuses */
			p->update |= (PU_BONUS);

			/* Redraw stuff */
			p->redraw |= (PR_INVEN);

			/* Save quiver size */
			save_quiver_size(p);

			/* Success */
			return (j);
		}
	}


	/* Paranoia */
	if (p->inven_cnt > INVEN_MAX_PACK) return (-1);


	/* Find an empty slot */
	for (j = 0; j <= INVEN_MAX_PACK; j++)
	{
		j_ptr = &p->inventory[j];
		if (!j_ptr->kind) break;
	}

	/* Use that slot */
	i = j;

	/* Reorder the pack */
	if (i < INVEN_MAX_PACK)
	{
		s32b o_value, j_value;

		/* Get the "value" of the item */
		o_value = o->kind->cost;

		/* Scan every occupied slot */
		for (j = 0; j < INVEN_MAX_PACK; j++)
		{
			j_ptr = &p->inventory[j];
			if (!j_ptr->kind) break;

			/* Hack -- readable books always come first */
			if ((o->tval == p_ptr->class->spell_book) &&
			    (j_ptr->tval != p_ptr->class->spell_book)) break;
			if ((j_ptr->tval == p_ptr->class->spell_book) &&
			    (o->tval != p_ptr->class->spell_book)) continue;

			/* Objects sort by decreasing type */
			if (o->tval > j_ptr->tval) break;
			if (o->tval < j_ptr->tval) continue;

			/* Non-aware (flavored) items always come last */
			if (!object_flavor_is_aware(o)) continue;
			if (!object_flavor_is_aware(j_ptr)) break;

			/* Objects sort by increasing sval */
			if (o->sval < j_ptr->sval) break;
			if (o->sval > j_ptr->sval) continue;

			/* Unidentified objects always come last */
			if (!object_is_known(o)) continue;
			if (!object_is_known(j_ptr)) break;

			/* Lights sort by decreasing fuel */
			if (o->tval == TV_LIGHT)
			{
				if (o->pval[DEFAULT_PVAL] > j_ptr->pval[DEFAULT_PVAL]) break;
				if (o->pval[DEFAULT_PVAL] < j_ptr->pval[DEFAULT_PVAL]) continue;
			}

			/* Determine the "value" of the pack item */
			j_value = j_ptr->kind->cost;

			/* Objects sort by decreasing value */
			if (o_value > j_value) break;
			if (o_value < j_value) continue;
		}

		/* Use that slot */
		i = j;

		/* Slide objects */
		for (k = n; k >= i; k--)
		{
			/* Hack -- Slide the item */
			object_copy(&p->inventory[k+1], &p->inventory[k]);

			/* Update object_idx if necessary */
			if (tracked_object_is(k))
			{
				track_object(k+1);
			}
		}

		/* Wipe the empty slot */
		object_wipe(&p->inventory[i]);
	}

	object_copy(&p->inventory[i], o);

	j_ptr = &p->inventory[i];
	j_ptr->next_o_idx = 0;
	j_ptr->held_m_idx = 0;
	j_ptr->iy = j_ptr->ix = 0;
	j_ptr->marked = FALSE;

	p->total_weight += (j_ptr->number * j_ptr->weight);
	p->inven_cnt++;
	p->update |= (PU_BONUS);
	p->notice |= (PN_COMBINE | PN_REORDER);
	p->redraw |= (PR_INVEN);

	/* Hobbits ID mushrooms on pickup, gnomes ID wands and staffs on pickup */
	if (!object_is_known(j_ptr))
	{
		if (player_has(PF_KNOW_MUSHROOM) && j_ptr->tval == TV_FOOD)
		{
			do_ident_item(i, j_ptr);
			msg("Mushrooms for breakfast!");
		}

		if (player_has(PF_KNOW_ZAPPER) &&
			(j_ptr->tval == TV_WAND || j_ptr->tval == TV_STAFF))
		{
			do_ident_item(i, j_ptr);
		}
	}

	/* Save quiver size */
	save_quiver_size(p);

	/* Return the slot */
	return (i);
}


/*
 * Take off (some of) a non-cursed equipment item
 *
 * Note that only one item at a time can be wielded per slot.
 *
 * Note that taking off an item when "full" may cause that item
 * to fall to the ground.
 *
 * Return the inventory slot into which the item is placed.
 */
s16b inven_takeoff(int item, int amt)
{
	int slot;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	const char *act;

	char o_name[80];

	bool track_removed_item = FALSE;


	/* Get the item to take off */
	o_ptr = &p_ptr->inventory[item];

	/* Paranoia */
	if (amt <= 0) return (-1);

	/* Verify */
	if (amt > o_ptr->number) amt = o_ptr->number;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain a local object */
	object_copy(i_ptr, o_ptr);

	/* Modify quantity */
	i_ptr->number = amt;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Took off weapon */
	if (item == INVEN_WIELD)
	{
		act = "You were wielding";
	}

	/* Took off bow */
	else if (item == INVEN_BOW)
	{
		act = "You were holding";
	}

	/* Took off light */
	else if (item == INVEN_LIGHT)
	{
		act = "You were holding";
	}

	/* Took off something */
	else
	{
		act = "You were wearing";
	}

	/* Update object_idx if necessary, after optimization */
	if (tracked_object_is(item))
	{
		track_removed_item = TRUE;
	}

	/* Modify, Optimize */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);

	/* Carry the object */
	slot = inven_carry(p_ptr, i_ptr);

	/* Track removed item if necessary */
	if (track_removed_item)
	{
		track_object(slot);
	}

	/* Message */
	msgt(MSG_WIELD, "%s %s (%c).", act, o_name, index_to_label(slot));

	p_ptr->notice |= PN_SQUELCH;

	/* Return slot */
	return (slot);
}


/*
 * Drop (some of) a non-cursed inventory/equipment item
 *
 * The object will be dropped "near" the current location
 */
void inven_drop(int item, int amt)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	char o_name[80];


	/* Get the original object */
	o_ptr = &p_ptr->inventory[item];

	/* Error check */
	if (amt <= 0) return;

	/* Not too many */
	if (amt > o_ptr->number) amt = o_ptr->number;


	/* Take off equipment */
	if (item >= INVEN_WIELD)
	{
		/* Take off first */
		item = inven_takeoff(item, amt);

		/* Get the original object */
		o_ptr = &p_ptr->inventory[item];
	}

	/* Stop tracking items no longer in the inventory */
	if (tracked_object_is(item) && amt == o_ptr->number)
	{
		track_object(NO_OBJECT);
	}

	i_ptr = &object_type_body;

	object_copy(i_ptr, o_ptr);
	object_split(i_ptr, o_ptr, amt);

	/* Describe local object */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", o_name, index_to_label(item));

	/* Drop it near the player */
	drop_near(cave, i_ptr, 0, py, px, FALSE);

	/* Modify, Describe, Optimize */
	inven_item_increase(item, -amt);
	inven_item_describe(item);
	inven_item_optimize(item);
}



/*
 * Combine items in the pack
 * Also "pick up" any gold in the inventory by accident
 *
 * Note special handling of the "overflow" slot
 */
void combine_pack(void)
{
	int i, j, k;

	object_type *o_ptr;
	object_type *j_ptr;

	bool flag = FALSE;


	/* Combine the pack (backwards) */
	for (i = INVEN_PACK; i > 0; i--)
	{
		bool slide = FALSE;

		/* Get the item */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty items */
		if (!o_ptr->kind) continue;

		/* Absorb gold */
		if (o_ptr->tval == TV_GOLD)
		{
			/* Count the gold */
			slide = TRUE;
			p_ptr->au += o_ptr->pval[DEFAULT_PVAL];
		}

		/* Scan the items above that item */
		else for (j = 0; j < i; j++)
		{
			/* Get the item */
			j_ptr = &p_ptr->inventory[j];

			/* Skip empty items */
			if (!j_ptr->kind) continue;

			/* Can we drop "o_ptr" onto "j_ptr"? */
			if (object_similar(j_ptr, o_ptr, OSTACK_PACK))
			{
				/* Take note */
				flag = slide = TRUE;

				/* Add together the item counts */
				object_absorb(j_ptr, o_ptr);

				break;
			}
		}


		/* Compact the inventory */
		if (slide)
		{
			/* One object is gone */
			p_ptr->inven_cnt--;

			/* Slide everything down */
			for (k = i; k < INVEN_PACK; k++)
			{
				/* Hack -- slide object */
				COPY(&p_ptr->inventory[k], &p_ptr->inventory[k+1], object_type);

				/* Update object_idx if necessary */
				if (tracked_object_is(k+1))
				{
					track_object(k);
				}
			}

			/* Hack -- wipe hole */
			object_wipe(&p_ptr->inventory[k]);

			/* Redraw stuff */
			p_ptr->redraw |= (PR_INVEN);
		}
	}

	/* Message */
	if (flag)
	{
		msg("You combine some items in your pack.");

		/* Stop "repeat last command" from working. */
		cmd_disable_repeat();
	}
}


/*
 * Reorder items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void reorder_pack(void)
{
	int i, j, k;

	s32b o_value;
	s32b j_value;

	object_type *o_ptr;
	object_type *j_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	bool flag = FALSE;


	/* Re-order the pack (forwards) */
	for (i = 0; i < INVEN_PACK; i++)
	{
		/* Get the item */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->kind) continue;

		/* Get the "value" of the item */
		o_value = o_ptr->kind->cost;

		/* Scan every occupied slot */
		for (j = 0; j < INVEN_PACK; j++)
		{
			/* Get the item already there */
			j_ptr = &p_ptr->inventory[j];

			/* Use empty slots */
			if (!j_ptr->kind) break;

			/* Hack -- readable books always come first */
			if ((o_ptr->tval == p_ptr->class->spell_book) &&
			    (j_ptr->tval != p_ptr->class->spell_book)) break;
			if ((j_ptr->tval == p_ptr->class->spell_book) &&
			    (o_ptr->tval != p_ptr->class->spell_book)) continue;

			/* Objects sort by decreasing type */
			if (o_ptr->tval > j_ptr->tval) break;
			if (o_ptr->tval < j_ptr->tval) continue;

			/* Non-aware (flavored) items always come last */
			if (!object_flavor_is_aware(o_ptr)) continue;
			if (!object_flavor_is_aware(j_ptr)) break;

			/* Objects sort by increasing sval */
			if (o_ptr->sval < j_ptr->sval) break;
			if (o_ptr->sval > j_ptr->sval) continue;

			/* Unidentified objects always come last */
			if (!object_is_known(o_ptr)) continue;
			if (!object_is_known(j_ptr)) break;

			/* Lights sort by decreasing fuel */
			if (o_ptr->tval == TV_LIGHT)
			{
				if (o_ptr->pval[DEFAULT_PVAL] > j_ptr->pval[DEFAULT_PVAL]) break;
				if (o_ptr->pval[DEFAULT_PVAL] < j_ptr->pval[DEFAULT_PVAL]) continue;
			}

			/* Determine the "value" of the pack item */
			j_value = j_ptr->kind->cost;

			/* Objects sort by decreasing value */
			if (o_value > j_value) break;
			if (o_value < j_value) continue;
		}

		/* Never move down */
		if (j >= i) continue;

		/* Take note */
		flag = TRUE;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Save a copy of the moving item */
		object_copy(i_ptr, &p_ptr->inventory[i]);

		/* Slide the objects */
		for (k = i; k > j; k--)
		{
			/* Slide the item */
			object_copy(&p_ptr->inventory[k], &p_ptr->inventory[k-1]);

			/* Update object_idx if necessary */
			if (tracked_object_is(k-1))
			{
				track_object(k);
			}
		}

		/* Insert the moving item */
		object_copy(&p_ptr->inventory[j], i_ptr);

		/* Update object_idx if necessary */
		if (tracked_object_is(i))
		{
			track_object(j);
		}

		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN);
	}

	if (flag) 
	{
		msg("You reorder some items in your pack.");

		/* Stop "repeat last command" from working. */
		cmd_disable_repeat();
	}
}


/*
 *Returns the number of times in 1000 that @ will FAIL
 * - thanks to Ed Graham for the formula
 */
int get_use_device_chance(const object_type *o_ptr)
{
	int lev, fail, numerator, denominator;

	int skill = p_ptr->state.skills[SKILL_DEVICE];

	int skill_min = 10;
	int skill_max = 141;
	int diff_min  = 1;
	int diff_max  = 100;

	/* Extract the item level, which is the difficulty rating */
	if (o_ptr->artifact)
		lev = o_ptr->artifact->level;
	else
		lev = o_ptr->kind->level;

	/* TODO: maybe use something a little less convoluted? */
	numerator   = (skill - lev) - (skill_max - diff_min);
	denominator = (lev - skill) - (diff_max - skill_min);

	/* Make sure that we don't divide by zero */
	if (denominator == 0) denominator = numerator > 0 ? 1 : -1;

	fail = (100 * numerator) / denominator;

	/* Ensure failure rate is between 1% and 75% */
	if (fail > 750) fail = 750;
	if (fail < 10) fail = 10;

	return fail;
}


/*
 * Distribute charges of rods, staves, or wands.
 *
 * o_ptr = source item
 * q_ptr = target item, must be of the same type as o_ptr
 * amt   = number of items that are transfered
 */
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt)
{
	int charge_time = randcalc(o_ptr->kind->time, 0, AVERAGE), max_time;

	/*
	 * Hack -- If rods, staves, or wands are dropped, the total maximum
	 * timeout or charges need to be allocated between the two stacks.
	 * If all the items are being dropped, it makes for a neater message
	 * to leave the original stack's pval alone. -LM-
	 */
	if ((o_ptr->tval == TV_WAND) ||
	    (o_ptr->tval == TV_STAFF))
	{
		q_ptr->pval[DEFAULT_PVAL] = o_ptr->pval[DEFAULT_PVAL] * amt / o_ptr->number;

		if (amt < o_ptr->number)
			o_ptr->pval[DEFAULT_PVAL] -= q_ptr->pval[DEFAULT_PVAL];
	}

	/*
	 * Hack -- Rods also need to have their timeouts distributed.
	 *
	 * The dropped stack will accept all time remaining to charge up to
	 * its maximum.
	 */
	if (o_ptr->tval == TV_ROD)
	{
		max_time = charge_time * amt;

		if (o_ptr->timeout > max_time)
			q_ptr->timeout = max_time;
		else
			q_ptr->timeout = o_ptr->timeout;

		if (amt < o_ptr->number)
			o_ptr->timeout -= q_ptr->timeout;
	}
}


void reduce_charges(object_type *o_ptr, int amt)
{
	/*
	 * Hack -- If rods or wand are destroyed, the total maximum timeout or
	 * charges of the stack needs to be reduced, unless all the items are
	 * being destroyed. -LM-
	 */
	if (((o_ptr->tval == TV_WAND) ||
	     (o_ptr->tval == TV_STAFF)) &&
	    (amt < o_ptr->number))
	{
		o_ptr->pval[DEFAULT_PVAL] -= o_ptr->pval[DEFAULT_PVAL] * amt / o_ptr->number;
	}

	if ((o_ptr->tval == TV_ROD) &&
	    (amt < o_ptr->number))
	{
		o_ptr->timeout -= o_ptr->timeout * amt / o_ptr->number;
	}
}


int number_charging(const object_type *o_ptr)
{
	int charge_time, num_charging;
	random_value timeout;

	/* Artifacts have a special timeout */	
	if (o_ptr->artifact)
		timeout = o_ptr->artifact->time;
	else
		timeout = o_ptr->kind->time;

	charge_time = randcalc(timeout, 0, AVERAGE);

	/* Item has no timeout */
	if (charge_time <= 0) return 0;

	/* No items are charging */
	if (o_ptr->timeout <= 0) return 0;

	/* Calculate number charging based on timeout */
	num_charging = (o_ptr->timeout + charge_time - 1) / charge_time;

	/* Number charging cannot exceed stack size */
	if (num_charging > o_ptr->number) num_charging = o_ptr->number;

	return num_charging;
}


bool recharge_timeout(object_type *o_ptr)
{
	int charging_before, charging_after;

	/* Find the number of charging items */
	charging_before = number_charging(o_ptr);

	/* Nothing to charge */	
	if (charging_before == 0)
		return FALSE;

	/* Decrease the timeout */
	o_ptr->timeout -= MIN(charging_before, o_ptr->timeout);

	/* Find the new number of charging items */
	charging_after = number_charging(o_ptr);

	/* Return true if at least 1 item obtained a charge */
	if (charging_after < charging_before)
		return TRUE;
	else
		return FALSE;
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
 * List of { tval, name } pairs.
 */
static const grouper tval_names[] =
{
	{ TV_SKELETON,    "skeleton" },
	{ TV_BOTTLE,      "bottle" },
	{ TV_JUNK,        "junk" },
	{ TV_SPIKE,       "spike" },
	{ TV_CHEST,       "chest" },
	{ TV_SHOT,        "shot" },
	{ TV_ARROW,       "arrow" },
	{ TV_BOLT,        "bolt" },
	{ TV_BOW,         "bow" },
	{ TV_DIGGING,     "digger" },
	{ TV_HAFTED,      "hafted" },
	{ TV_POLEARM,     "polearm" },
	{ TV_SWORD,       "sword" },
	{ TV_BOOTS,       "boots" },
	{ TV_GLOVES,      "gloves" },
	{ TV_HELM,        "helm" },
	{ TV_CROWN,       "crown" },
	{ TV_SHIELD,      "shield" },
	{ TV_CLOAK,       "cloak" },
	{ TV_SOFT_ARMOR,  "soft armor" },
	{ TV_SOFT_ARMOR,  "soft armour" },
	{ TV_HARD_ARMOR,  "hard armor" },
	{ TV_HARD_ARMOR,  "hard armour" },
	{ TV_DRAG_ARMOR,  "dragon armor" },
	{ TV_DRAG_ARMOR,  "dragon armour" },
	{ TV_LIGHT,       "light" },
	{ TV_AMULET,      "amulet" },
	{ TV_RING,        "ring" },
	{ TV_STAFF,       "staff" },
	{ TV_WAND,        "wand" },
	{ TV_ROD,         "rod" },
	{ TV_SCROLL,      "scroll" },
	{ TV_POTION,      "potion" },
	{ TV_FLASK,       "flask" },
	{ TV_FOOD,        "food" },
	{ TV_MAGIC_BOOK,  "magic book" },
	{ TV_PRAYER_BOOK, "prayer book" },
	{ TV_GOLD,        "gold" },
};

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
		const char *nm = k_ptr->name;

		if (!nm) continue;

		if (*nm == '&' && *(nm+1))
			nm += 2;

		/* Found a match */
		if (k_ptr->tval == tval && !strcmp(name, nm))
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
		if (a_ptr->name && my_stristr(a_ptr->name, name) && a_idx == -1)
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
		const char *nm = k_ptr->name;

		if (!nm) continue;

		if (*nm == '&' && *(nm+1))
			nm += 2;

		/* Found a match */
		if (k_ptr->tval == tval && !strcmp(name, nm))
			return k_ptr->sval;
	}

	return -1;
}

/**
 * Returns the numeric equivalent tval of the textual tval `name`.
 */
int tval_find_idx(const char *name)
{
	size_t i = 0;
	unsigned int r;

	if (sscanf(name, "%u", &r) == 1)
		return r;

	for (i = 0; i < N_ELEMENTS(tval_names); i++)
	{
		if (!my_stricmp(name, tval_names[i].name))
			return tval_names[i].tval;
	}

	return -1;
}

/**
 * Returns the textual equivalent tval of the numeric tval `name`.
 */
const char *tval_find_name(int tval)
{
	size_t i = 0;

	for (i = 0; i < N_ELEMENTS(tval_names); i++)
	{
		if (tval == tval_names[i].tval)
			return tval_names[i].name;
	}

	return "unknown";
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
static int compare_items(const object_type *o1, const object_type *o2)
{
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
void display_object_recall(object_type *o_ptr)
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
void display_object_kind_recall(s16b k_idx)
{
	object_type object = { 0 };
	object_prep(&object, &k_info[k_idx], 0, EXTREMIFY);
	if (k_info[k_idx].aware)
		object.ident |= IDENT_STORE;

	display_object_recall(&object);
}

/*
 * Display visible items, similar to display_monlist
 */
void display_itemlist(void)
{
	int max;
	int mx, my;
	unsigned num;
	int line = 1, x = 0;
	int cur_x;
	unsigned i;
	unsigned disp_count = 0;
	byte a;
	char c;

	object_type *types[MAX_ITEMLIST];
	int counts[MAX_ITEMLIST];
	int dx[MAX_ITEMLIST], dy[MAX_ITEMLIST];
	unsigned counter = 0;

	int dungeon_hgt = p_ptr->depth == 0 ? TOWN_HGT : DUNGEON_HGT;
	int dungeon_wid = p_ptr->depth == 0 ? TOWN_WID : DUNGEON_WID;

	byte attr;
	char buf[80];

	int floor_list[MAX_FLOOR_STACK];

	/* Clear the term if in a subwindow, set x otherwise */
	if (Term != angband_term[0])
	{
		clear_from(0);
		max = Term->hgt - 1;
	}
	else
	{
		x = 13;
		max = Term->hgt - 2;
	}

	/* Look at each square of the dungeon for items */
	for (my = 0; my < dungeon_hgt; my++)
	{
		for (mx = 0; mx < dungeon_wid; mx++)
		{
			num = scan_floor(floor_list, MAX_FLOOR_STACK, my, mx, 0x02);

			/* Iterate over all the items found on this square */
			for (i = 0; i < num; i++)
			{
				object_type *o_ptr = object_byid(floor_list[i]);
				unsigned j;

				/* Skip gold/squelched */
				if (o_ptr->tval == TV_GOLD || squelch_item_ok(o_ptr))
					continue;

				/* See if we've already seen a similar item; if so, just add */
				/* to its count */
				for (j = 0; j < counter; j++)
				{
					if (object_similar(o_ptr, types[j],
						OSTACK_LIST))
					{
						counts[j] += o_ptr->number;
						if ((my - p_ptr->py) * (my - p_ptr->py) + (mx - p_ptr->px) * (mx - p_ptr->px) < dy[j] * dy[j] + dx[j] * dx[j])
						{
							dy[j] = my - p_ptr->py;
							dx[j] = mx - p_ptr->px;
						}
						break;
					}
				}

				/* We saw a new item. So insert it at the end of the list and */
				/* then sort it forward using compare_items(). The types list */
				/* is always kept sorted. */
				if (j == counter)
				{
					types[counter] = o_ptr;
					counts[counter] = o_ptr->number;
					dy[counter] = my - p_ptr->py;
					dx[counter] = mx - p_ptr->px;

					while (j > 0 && compare_items(types[j - 1], types[j]) > 0)
					{
						object_type *tmp_o = types[j - 1];
						int tmpcount;
						int tmpdx = dx[j-1];
						int tmpdy = dy[j-1];

						types[j - 1] = types[j];
						types[j] = tmp_o;
						dx[j-1] = dx[j];
						dx[j] = tmpdx;
						dy[j-1] = dy[j];
						dy[j] = tmpdy;
						tmpcount = counts[j - 1];
						counts[j - 1] = counts[j];
						counts[j] = tmpcount;
						j--;
					}
					counter++;
				}
			}
		}
	}

	/* Note no visible items */
	if (!counter)
	{
		/* Clear display and print note */
		c_prt(TERM_SLATE, "You see no items.", 0, 0);
		if (Term == angband_term[0])
			Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

		/* Done */
		return;
	}
	else
	{
		/* Reprint Message */
		prt(format("You can see %d item%s:",
				   counter, (counter > 1 ? "s" : "")), 0, 0);
	}

	for (i = 0; i < counter; i++)
	{
		/* o_name will hold the object_desc() name for the object. */
		/* o_desc will also need to put a (x4) behind it. */
		/* can there be more than 999 stackable items on a level? */
		char o_name[80];
		char o_desc[86];

		object_type *o_ptr = types[i];

		/* We shouldn't list coins or squelched items */
		if (o_ptr->tval == TV_GOLD || squelch_item_ok(o_ptr))
			continue;

		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);
		if (counts[i] > 1)
			strnfmt(o_desc, sizeof(o_desc), "%s (x%d) %d %c, %d %c", o_name, counts[i],
				(dy[i] > 0) ? dy[i] : -dy[i], (dy[i] > 0) ? 'S' : 'N',
				(dx[i] > 0) ? dx[i] : -dx[i], (dx[i] > 0) ? 'E' : 'W');
		else
			strnfmt(o_desc, sizeof(o_desc), "%s  %d %c %d %c", o_name,
				(dy[i] > 0) ? dy[i] : -dy[i], (dy[i] > 0) ? 'S' : 'N',
				(dx[i] > 0) ? dx[i] : -dx[i], (dx[i] > 0) ? 'E' : 'W');

		/* Reset position */
		cur_x = x;

		/* See if we need to scroll or not */
		if (Term == angband_term[0] && (line == max) && disp_count != counter)
		{
			prt("-- more --", line, x);
			anykey();

			/* Clear the screen */
			for (line = 1; line <= max; line++)
				prt("", line, x);

			/* Reprint Message */
			prt(format("You can see %d item%s:",
					   counter, (counter > 1 ? "s" : "")), 0, 0);

			/* Reset */
			line = 1;
		}
		else if (line == max)
		{
			continue;
		}

		/* Note that the number of items actually displayed */
		disp_count++;

		if (o_ptr->artifact && object_is_known(o_ptr))
			/* known artifact */
			attr = TERM_VIOLET;
		else if (!object_flavor_is_aware(o_ptr))
			/* unaware of kind */
			attr = TERM_RED;
		else if (o_ptr->kind->cost == 0)
			/* worthless */
			attr = TERM_SLATE;
		else
			/* default */
			attr = TERM_WHITE;

		a = object_kind_attr(o_ptr->kind);
		c = object_kind_char(o_ptr->kind);

		/* Display the pict */
		if ((tile_width == 1) && (tile_height == 1))
		{
		        Term_putch(cur_x++, line, a, c);
			Term_putch(cur_x++, line, TERM_WHITE, ' ');
		}

		/* Print and bump line counter */
		c_prt(attr, o_desc, line, cur_x);
		line++;
	}

	if (disp_count != counter)
	{
		/* Print "and others" message if we've run out of space */
		strnfmt(buf, sizeof buf, "  ...and %d others.", counter - disp_count);
		c_prt(TERM_WHITE, buf, line, x);
	}
	else
	{
		/* Otherwise clear a line at the end, for main-term display */
		prt("", line, x);
	}

	if (Term == angband_term[0])
		Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");
}


/* Basic tval testers */
bool obj_is_staff(const object_type *o_ptr)  { return o_ptr->tval == TV_STAFF; }
bool obj_is_wand(const object_type *o_ptr)   { return o_ptr->tval == TV_WAND; }
bool obj_is_rod(const object_type *o_ptr)    { return o_ptr->tval == TV_ROD; }
bool obj_is_potion(const object_type *o_ptr) { return o_ptr->tval == TV_POTION; }
bool obj_is_scroll(const object_type *o_ptr) { return o_ptr->tval == TV_SCROLL; }
bool obj_is_food(const object_type *o_ptr)   { return o_ptr->tval == TV_FOOD; }
bool obj_is_light(const object_type *o_ptr)   { return o_ptr->tval == TV_LIGHT; }
bool obj_is_ring(const object_type *o_ptr)   { return o_ptr->tval == TV_RING; }


/**
 * Determine whether an object is ammo
 *
 * \param o_ptr is the object to check
 */
bool obj_is_ammo(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
			return TRUE;
		default:
			return FALSE;
	}
}

/* Determine if an object has charges */
bool obj_has_charges(const object_type *o_ptr)
{
	if (o_ptr->tval != TV_WAND && o_ptr->tval != TV_STAFF) return FALSE;

	if (o_ptr->pval[DEFAULT_PVAL] <= 0) return FALSE;

	return TRUE;
}

/* Determine if an object is zappable */
bool obj_can_zap(const object_type *o_ptr)
{
	/* Any rods not charging? */
	if (o_ptr->tval == TV_ROD && number_charging(o_ptr) < o_ptr->number)
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

bool obj_can_refill(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];
	const object_type *j_ptr = &p_ptr->inventory[INVEN_LIGHT];

	/* Get flags */
	object_flags(o_ptr, f);

	if (j_ptr->sval == SV_LIGHT_LANTERN)
	{
		/* Flasks of oil are okay */
		if (o_ptr->tval == TV_FLASK) return (TRUE);
	}

	/* Non-empty, non-everburning sources are okay */
	if ((o_ptr->tval == TV_LIGHT) &&
	    (o_ptr->sval == j_ptr->sval) &&
	    (o_ptr->timeout > 0) &&
	    !of_has(f, OF_NO_FUEL))
	{
		return (TRUE);
	}

	/* Assume not okay */
	return (FALSE);
}


bool obj_can_browse(const object_type *o_ptr)
{
	return o_ptr->tval == p_ptr->class->spell_book;
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
	return (wield_slot(o_ptr) >= INVEN_WIELD);
}

/* Can only fire an item with the right tval */
bool obj_can_fire(const object_type *o_ptr)
{
	return o_ptr->tval == p_ptr->state.ammo_tval;
}

/* Can has inscrip pls */
bool obj_has_inscrip(const object_type *o_ptr)
{
	return (o_ptr->note ? TRUE : FALSE);
}

/*** Generic utility functions ***/

/*
 * Return an object's effect.
 */
u16b object_effect(const object_type *o_ptr)
{
	if (o_ptr->artifact)
		return o_ptr->artifact->effect;
	else
		return o_ptr->kind->effect;
}

/* Get an o_ptr from an item number */
object_type *object_from_item_idx(int item)
{
	if (item >= 0)
		return &p_ptr->inventory[item];
	else
		return object_byid(0 - item);
}


/*
 * Does the given object need to be aimed?
 */ 
bool obj_needs_aim(object_type *o_ptr)
{
	int effect = object_effect(o_ptr);

	/* If the effect needs aiming, or if the object type needs
	   aiming, this object needs aiming. */
	return effect_aim(effect) || o_ptr->tval == TV_BOLT ||
			o_ptr->tval == TV_SHOT || o_ptr->tval == TV_ARROW ||
			o_ptr->tval == TV_WAND ||
			(o_ptr->tval == TV_ROD && !object_flavor_is_aware(o_ptr));
}


/*
 * Verify the "okayness" of a given item.
 *
 * The item can be negative to mean "item on floor".
 */
bool get_item_okay(int item)
{
	/* Verify the item */
	return (item_tester_okay(object_from_item_idx(item)));
}


/*
 * Get a list of "valid" item indexes.
 *
 * Fills item_list[] with items that are "okay" as defined by the
 * current item_tester_hook, etc.  mode determines what combination of
 * inventory, equipment and player's floor location should be used
 * when drawing up the list.
 *
 * Returns the number of items placed into the list.
 *
 * Maximum space that can be used is [INVEN_TOTAL + MAX_FLOOR_STACK],
 * though practically speaking much smaller numbers are likely.
 */
int scan_items(int *item_list, size_t item_list_max, int mode)
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
			if (get_item_okay(i))
				item_list[item_list_num++] = i;
		}
	}

	if (use_equip)
	{
		for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL && item_list_num < item_list_max; i++)
		{
			if (get_item_okay(i))
				item_list[item_list_num++] = i;
		}
	}

	/* Scan all non-gold objects in the grid */
	if (use_floor)
	{
		floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), p_ptr->py, p_ptr->px, 0x03);

		for (i = 0; i < floor_num && item_list_num < item_list_max; i++)
		{
			if (get_item_okay(-floor_list[i]))
				item_list[item_list_num++] = -floor_list[i];
		}
	}

	/* Forget the item_tester_tval and item_tester_hook  restrictions */
	item_tester_tval = 0;
	item_tester_hook = NULL;

	return item_list_num;
}


/* 
 * Check if the given item is available for the player to use. 
 *
 * 'mode' defines which areas we should look at, a la scan_items().
 */
bool item_is_available(int item, bool (*tester)(const object_type *), int mode)
{
	int item_list[ALL_INVEN_TOTAL + MAX_FLOOR_STACK];
	int item_num;
	int i;

	item_tester_hook = tester;
	item_tester_tval = 0;
	item_num = scan_items(item_list, N_ELEMENTS(item_list), mode);

	for (i = 0; i < item_num; i++)
	{
		if (item_list[i] == item)
			return TRUE;
	}

	return FALSE;
}

/*
 * Returns whether the pack is holding the maximum number of items. The max
 * size is INVEN_MAX_PACK, which is a macro since quiver size affects slots
 * available.
 */
bool pack_is_full(void)
{
	return p_ptr->inventory[INVEN_MAX_PACK - 1].kind ? TRUE : FALSE;
}

/*
 * Returns whether the pack is holding the more than the maximum number of
 * items. The max size is INVEN_MAX_PACK, which is a macro since quiver size
 * affects slots available. If this is true, calling pack_overflow() will
 * trigger a pack overflow.
 */
bool pack_is_overfull(void)
{
	return p_ptr->inventory[INVEN_MAX_PACK].kind ? TRUE : FALSE;
}

/*
 * Overflow an item from the pack, if it is overfull.
 */
void pack_overflow(void)
{
	int item = INVEN_MAX_PACK;
	char o_name[80];
	object_type *o_ptr;

	if (!pack_is_overfull()) return;

	/* Get the slot to be dropped */
	o_ptr = &p_ptr->inventory[item];

	/* Disturbing */
	disturb(p_ptr, 0, 0);

	/* Warning */
	msg("Your pack overflows!");

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", o_name, index_to_label(item));

	/* Drop it (carefully) near the player */
	drop_near(cave, o_ptr, 0, p_ptr->py, p_ptr->px, FALSE);

	/* Modify, Describe, Optimize */
	inven_item_increase(item, -255);
	inven_item_describe(item);
	inven_item_optimize(item);

	/* Notice stuff (if needed) */
	if (p_ptr->notice) notice_stuff(p_ptr);

	/* Update stuff (if needed) */
	if (p_ptr->update) update_stuff(p_ptr);

	/* Redraw stuff (if needed) */
	if (p_ptr->redraw) redraw_stuff(p_ptr);
}

struct object *object_byid(s16b oidx)
{
	assert(oidx >= 0);
	assert(oidx <= z_info->o_max);
	return &o_list[oidx];
}

void objects_init(void)
{
	o_list = C_ZNEW(z_info->o_max, struct object);
}

void objects_destroy(void)
{
	mem_free(o_list);
}
