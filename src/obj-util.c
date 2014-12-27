/**
 * \file obj-util.c
 * \brief Object utilities
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
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-spell.h"
#include "player-util.h"
#include "prefs.h"
#include "randname.h"
#include "z-queue.h"

object_base *kb_info;
object_kind *k_info;
artifact_type *a_info;
ego_item_type *e_info;
struct flavor *flavors;

/**
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
 * Mainly useful for randarts so that fixed flavors for standards aren't
 * predictable. The One Ring is kept as fixed, since it lives through randarts.
 */
void flavor_reset_fixed(void)
{
	struct flavor *f;

	for (f = flavors; f; f = f->next) {
		if (f->tval == TV_RING && strstr(f->text, "Plain Gold"))
			continue;

		f->sval = SV_UNKNOWN;
	}
}

/**
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Mushrooms, Potions, Scrolls.
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

		my_strcpy(buf, "\"", 2);
		wordlen = randname_make(RANDNAME_SCROLL, 2, 8, end, 24, name_sections);
		while (titlelen + wordlen < (int)(sizeof(scroll_adj[0]) - 3))
		{
			end[wordlen] = ' ';
			titlelen += wordlen + 1;
			end += wordlen + 1;
			wordlen = randname_make(RANDNAME_SCROLL, 2, 8, end, 24 - titlelen,
									name_sections);
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


/**
 * Obtain the flags for an item
 */
void object_flags(const struct object *obj, bitflag flags[OF_SIZE])
{
	of_wipe(flags);

	if (!obj)
		return;

	of_copy(flags, obj->flags);
}


/**
 * Obtain the flags for an item which are known to the player
 */
void object_flags_known(const struct object *o_ptr, bitflag flags[OF_SIZE])
{
	object_flags(o_ptr, flags);

	of_inter(flags, o_ptr->known_flags);

	if (object_flavor_is_aware(o_ptr))
		of_union(flags, o_ptr->kind->flags);

	if (o_ptr->ego && easy_know(o_ptr))
		of_union(flags, o_ptr->ego->flags);
}

/**
 * Apply a tester function, skipping all non-objects and gold
 */
bool object_test(item_tester tester, const struct object *obj)
{
	/* Require object */
	if (!obj) return FALSE;

	/* Ignore gold */
	if (tval_is_money(obj)) return FALSE;

	/* Pass without a tester, or tail-call the tester if it exists */
	return !tester || tester(obj);
}


/**
 * Return true if the item is unknown (has yet to be seen by the player).
 */
bool is_unknown(const struct object *o_ptr)
{
	grid_data gd = { 0 };
	map_info(o_ptr->iy, o_ptr->ix, &gd);
	return gd.unseen_object;
}	


/**
 * Looks if "inscrip" is present on the given object.
 */
unsigned check_for_inscrip(const struct object *o_ptr, const char *inscrip)
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
struct object_kind *lookup_kind(int tval, int sval)
{
	int k;

	/* Look for it */
	for (k = 0; k < z_info->k_max; k++) {
		object_kind *kind = &k_info[k];
		if (kind->tval == tval && kind->sval == sval)
			return kind;
	}

	/* Failure */
	msg("No object: %d:%d (%s)", tval, sval, tval_find_name(tval));
	return NULL;
}

struct object_kind *objkind_byid(int kidx) {
	if (kidx < 1 || kidx > z_info->k_max)
		return NULL;
	return &k_info[kidx];
}


/*** Textual<->numeric conversion ***/

/**
 * Return the a_idx of the artifact with the given name
 */
int lookup_artifact_name(const char *name)
{
	int i;
	int a_idx = -1;
	
	/* Look for it */
	for (i = 1; i < z_info->a_max; i++) {
		artifact_type *a_ptr = &a_info[i];

		/* Test for equality */
		if (a_ptr->name && streq(name, a_ptr->name))
			return i;
		
		/* Test for close matches */
		if (strlen(name) >= 3 && a_ptr->name && my_stristr(a_ptr->name, name)
			&& a_idx == -1)
			a_idx = i;
	}

	/* Return our best match */
	return a_idx;
}


/**
 * Return the numeric sval of the object kind with the given `tval` and
 * name `name`.
 */
int lookup_sval(int tval, const char *name)
{
	int k;
	unsigned int r;

	if (sscanf(name, "%u", &r) == 1)
		return r;

	/* Look for it */
	for (k = 1; k < z_info->k_max; k++) {
		object_kind *k_ptr = &k_info[k];
		char cmp_name[1024];

		if (!k_ptr || !k_ptr->name) continue;

		obj_desc_name_format(cmp_name, sizeof cmp_name, 0, k_ptr->name, 0,
							 FALSE);

		/* Found a match */
		if (k_ptr->tval == tval && !my_stricmp(cmp_name, name))
			return k_ptr->sval;
	}

	return -1;
}

void object_short_name(char *buf, size_t max, const char *name)
{
	size_t j, k;
	/* Copy across the name, stripping modifiers & and ~) */
	size_t len = strlen(name);
	for (j = 0, k = 0; j < len && k < max; j++) {
		if (j == 0 && name[0] == '&' && name[1] == ' ')
			j += 2;
		if (name[j] == '~')
			continue;

		buf[k++] = name[j];
	}
	buf[k] = 0;
}

/**
 * Sort comparator for objects using only tval and sval.
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 */
static int compare_types(const struct object *o1, const struct object *o2)
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
int compare_items(const struct object *o1, const struct object *o2)
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
 * Determine if an object has charges
 */
bool obj_has_charges(const struct object *o_ptr)
{
	if (!tval_can_have_charges(o_ptr)) return FALSE;

	if (o_ptr->pval <= 0) return FALSE;

	return TRUE;
}

/**
 * Determine if an object is zappable
 */
bool obj_can_zap(const struct object *o_ptr)
{
	/* Any rods not charging? */
	if (tval_can_have_timeout(o_ptr) && number_charging(o_ptr) < o_ptr->number)
		return TRUE;

	return FALSE;
}

/**
 * Determine if an object is activatable
 */
bool obj_is_activatable(const struct object *o_ptr)
{
	return object_effect(o_ptr) ? TRUE : FALSE;
}

/**
 * Determine if an object can be activated now
 */
bool obj_can_activate(const struct object *o_ptr)
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
bool obj_can_refill(const struct object *obj)
{
	const struct object *light = equipped_item_by_slot_name(player, "light");

	/* Need fuel? */
	if (of_has(obj->flags, OF_NO_FUEL)) return FALSE;

	/* A lantern can be refueled from a flask or another lantern */
	if (of_has(light->flags, OF_TAKES_FUEL)) {
		if (tval_is_fuel(obj))
			return TRUE;
		else if (tval_is_light(obj) && of_has(obj->flags, OF_TAKES_FUEL) &&
				 obj->timeout > 0) 
			return TRUE;
	}

	return FALSE;
}

bool obj_can_browse(const struct object *o_ptr)
{
	int i;

	for (i = 0; i < player->class->magic.num_books; i++) {
		class_book book = player->class->magic.books[i];
		if (o_ptr->kind == lookup_kind(book.tval, book.sval))
			return TRUE;
	}

	return FALSE;
}

bool obj_can_cast_from(const struct object *o_ptr)
{
	return obj_can_browse(o_ptr) &&
			spell_book_count_spells(o_ptr, spell_okay_to_cast) > 0;
}

bool obj_can_study(const struct object *o_ptr)
{
	return obj_can_browse(o_ptr) &&
			spell_book_count_spells(o_ptr, spell_okay_to_study) > 0;
}


/* Can only take off non-cursed items */
bool obj_can_takeoff(const struct object *o_ptr)
{
	return !cursed_p((bitflag *)o_ptr->flags);
}

/* Can only put on wieldable items */
bool obj_can_wear(const struct object *o_ptr)
{
	return (wield_slot(o_ptr) >= 0);
}

/* Can only fire an item with the right tval */
bool obj_can_fire(const struct object *o_ptr)
{
	return o_ptr->tval == player->state.ammo_tval;
}

/* Can has inscrip pls */
bool obj_has_inscrip(const struct object *o_ptr)
{
	return (o_ptr->note ? TRUE : FALSE);
}

bool obj_is_useable(const struct object *o_ptr)
{
	if (tval_is_useable(o_ptr))
		return TRUE;

	if (object_effect(o_ptr))
		return TRUE;

	if (tval_is_ammo(o_ptr))
		return o_ptr->tval == player->state.ammo_tval;

	return FALSE;
}

/*** Generic utility functions ***/

/**
 * Return an object's effect.
 */
u16b object_effect(const struct object *o_ptr)
{
	if (!o_ptr->effect) return 0;
	return o_ptr->effect->index;
}

/**
 * Does the given object need to be aimed?
 */ 
bool obj_needs_aim(struct object *o_ptr)
{
	struct effect *effect = o_ptr->effect;

	/* If the effect needs aiming, or if the object type needs
	   aiming, this object needs aiming. */
	return effect_aim(effect) || tval_is_ammo(o_ptr) ||
			tval_is_wand(o_ptr) ||
			(tval_is_rod(o_ptr) && !object_flavor_is_aware(o_ptr));
}

/**
 * Can the object fail if used?
 */
bool obj_can_fail(const struct object *o)
{
	if (tval_can_have_failure(o))
		return TRUE;

	return wield_slot(o) == -1 ? FALSE : TRUE;
}


/**
 * Returns the number of times in 1000 that @ will FAIL
 * - thanks to Ed Graham for the formula
 */
int get_use_device_chance(const struct object *o_ptr)
{
	int lev, fail, numerator, denominator;

	int skill = player->state.skills[SKILL_DEVICE];

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


/**
 * Distribute charges of rods, staves, or wands.
 *
 * \param o_ptr is the source item
 * \param q_ptr is the target item, must be of the same type as o_ptr
 * \param amt is the number of items that are transfered
 */
void distribute_charges(struct object *o_ptr, struct object *q_ptr, int amt)
{
	int charge_time = randcalc(o_ptr->time, 0, AVERAGE), max_time;

	/*
	 * Hack -- If rods, staves, or wands are dropped, the total maximum
	 * timeout or charges need to be allocated between the two stacks.
	 * If all the items are being dropped, it makes for a neater message
	 * to leave the original stack's pval alone. -LM-
	 */
	if (tval_can_have_charges(o_ptr)) {
		q_ptr->pval = o_ptr->pval * amt / o_ptr->number;

		if (amt < o_ptr->number)
			o_ptr->pval -= q_ptr->pval;
	}

	/*
	 * Hack -- Rods also need to have their timeouts distributed.
	 *
	 * The dropped stack will accept all time remaining to charge up to
	 * its maximum.
	 */
	if (tval_can_have_timeout(o_ptr)) {
		max_time = charge_time * amt;

		if (o_ptr->timeout > max_time)
			q_ptr->timeout = max_time;
		else
			q_ptr->timeout = o_ptr->timeout;

		if (amt < o_ptr->number)
			o_ptr->timeout -= q_ptr->timeout;
	}
}


/**
 * If rods or wand are destroyed, the total maximum timeout or charges of the
 * stack needs to be reduced, unless all the items are being destroyed. -LM-
 */
void reduce_charges(struct object *o_ptr, int amt)
{
	if (tval_can_have_charges(o_ptr) && amt < o_ptr->number)
		o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;

	if (tval_can_have_timeout(o_ptr) && amt < o_ptr->number)
		o_ptr->timeout -= o_ptr->timeout * amt / o_ptr->number;
}

/**
 * Number of items (usually rods) charging
 */
int number_charging(const struct object *o_ptr)
{
	int charge_time, num_charging;

	charge_time = randcalc(o_ptr->time, 0, AVERAGE);

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

/**
 * Allow a stack of charging objects to charge by one unit per charging object
 * Return TRUE if something recharged
 */
bool recharge_timeout(struct object *o_ptr)
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

