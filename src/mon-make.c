/**
 * \file mon-make.c
 * \brief Monster creation / placement code.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "alloc.h"
#include "game-world.h"
#include "init.h"
#include "mon-group.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-predicate.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "target.h"

/**
 * ------------------------------------------------------------------------
 * Monster race allocation
 *
 * Monster race allocation is done using an allocation table (see alloc.h).
 * This table is sorted by depth.  Each line of the table contains the
 * monster race index, the monster race level, and three probabilities:
 * - prob1 is the base probability of the race, calculated from monster.txt.
 * - prob2 is calculated by get_mon_num_prep(), which decides whether a
 *         monster is appropriate based on a secondary function; prob2 is
 *         always either prob1 or 0.
 * - prob3 is calculated by get_mon_num(), which checks whether universal
 *         restrictions apply (for example, unique monsters can only appear
 *         once on a given level); prob3 is always either prob2 or 0.
 * ------------------------------------------------------------------------ */
static s16b alloc_race_size;
static struct alloc_entry *alloc_race_table;

/**
 * Initialize monster allocation info
 */
static void init_race_allocs(void) {
	int i;
	struct monster_race *race;
	alloc_entry *table;
	s16b *num = mem_zalloc(z_info->max_depth * sizeof(s16b));
	s16b *already_counted = mem_zalloc(z_info->max_depth * sizeof(s16b));

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++) {
		/* Get the i'th race */
		race = &r_info[i];

		/* Legal monsters */
		if (race->rarity) {
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[race->level]++;
		}
	}

	/* Calculate the cumultive level totals */
	for (i = 1; i < z_info->max_depth; i++) {
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town monsters!");

	/* Allocate the alloc_race_table */
	alloc_race_table = mem_zalloc(alloc_race_size * sizeof(alloc_entry));

	/* Get the table entry */
	table = alloc_race_table;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++) {
		/* Get the i'th race */
		race = &r_info[i];

		/* Count valid races */
		if (race->rarity) {
			int p, lev, prev_lev_count, race_index;

			/* Extract this race's level */
			lev = race->level;

			/* Extract the base probability */
			p = (100 / race->rarity);

			/* Multiply by depth factor (experimental) */
			p *= (1 + lev / 10);

			/* Skip entries preceding this monster's level */
			prev_lev_count = (lev > 0) ? num[lev - 1] : 0;

			/* Skip entries already counted for this level */
			race_index = prev_lev_count + already_counted[lev];

			/* Load the entry */
			table[race_index].index = i;
			table[race_index].level = lev;
			table[race_index].prob1 = p;
			table[race_index].prob2 = p;
			table[race_index].prob3 = p;

			/* Another entry complete for this locale */
			already_counted[lev]++;
		}
	}
	mem_free(already_counted);
	mem_free(num);
}

static void cleanup_race_allocs(void) {
	mem_free(alloc_race_table);
}


/**
 * Apply a monster restriction function to the monster allocation table.
 * This way, we can use get_mon_num() to get a level-appropriate monster that
 * satisfies certain conditions (such as belonging to a particular monster
 * family).
 */
void get_mon_num_prep(bool (*get_mon_num_hook)(struct monster_race *race))
{
	int i;

	/* Scan the allocation table */
	for (i = 0; i < alloc_race_size; i++) {
		alloc_entry *entry = &alloc_race_table[i];

		/* Check the restriction, if any */
		if (!get_mon_num_hook || (*get_mon_num_hook)(&r_info[entry->index])) {
			/* Accept this monster */
			entry->prob2 = entry->prob1;

		} else {
			/* Do not use this monster */
			entry->prob2 = 0;
		}
	}
}

/**
 * Helper function for get_mon_num(). Scans the prepared monster allocation
 * table and picks a random monster. Returns the index of a monster in
 * `table`.
 */
static struct monster_race *get_mon_race_aux(long total,
											 const alloc_entry *table)
{
	int i;

	/* Pick a monster */
	long value = randint0(total);

	/* Find the monster */
	for (i = 0; i < alloc_race_size; i++) {
		/* Found the entry */
		if (value < table[i].prob3) break;

		/* Decrement */
		value -= table[i].prob3;
	}

	return &r_info[table[i].index];
}

/**
 * Chooses a monster race that seems appropriate to the given level
 *
 * This function uses the "prob2" field of the monster allocation table,
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an appropriate monster, in
 * a relatively efficient manner.
 *
 * Note that town monsters will *only* be created in the town, and
 * "normal" monsters will *never* be created in the town, unless the
 * level is modified, for example, by polymorph or summoning.
 *
 * There is a small chance (1/25) of boosting the given depth by
 * a small amount (up to four levels), except in the town.
 *
 * It is (slightly) more likely to acquire a monster of the given level
 * than one of a lower level.  This is done by choosing several monsters
 * appropriate to the given level and keeping the deepest one.
 *
 * Note that if no monsters are appropriate, then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
struct monster_race *get_mon_num(int level)
{
	int i, p;
	long total;
	struct monster_race *race;
	alloc_entry *table = alloc_race_table;
	time_t cur_time = time(NULL);
	struct tm *date = localtime(&cur_time);

	/* Occasionally produce a nastier monster in the dungeon */
	if (level > 0 && one_in_(z_info->ood_monster_chance))
		level += MIN(level / 4 + 2, z_info->ood_monster_amount);

	total = 0L;

	/* Process probabilities */
	for (i = 0; i < alloc_race_size; i++) {
		/* Monsters are sorted by depth */
		if (table[i].level > level) break;

		/* Default */
		table[i].prob3 = 0;

		/* No town monsters in dungeon */
		if ((level > 0) && (table[i].level <= 0)) continue;

		/* Get the chosen monster */
		race = &r_info[table[i].index];

		/* No seasonal monsters outside of Christmas */
		if (rf_has(race->flags, RF_SEASONAL) &&
			!(date->tm_mon == 11 && date->tm_mday >= 24 && date->tm_mday <= 26))
			continue;

		/* Only one copy of a unique must be around at the same time */
		if (rf_has(race->flags, RF_UNIQUE) && (race->cur_num >= race->max_num))
			continue;

		/* Some monsters never appear out of depth */
		if (rf_has(race->flags, RF_FORCE_DEPTH) && race->level > player->depth)
			continue;

		/* Accept */
		table[i].prob3 = table[i].prob2;

		/* Total */
		total += table[i].prob3;
	}

	/* No legal monsters */
	if (total <= 0) return NULL;

	/* Pick a monster */
	race = get_mon_race_aux(total, table);

	/* Try for a "harder" monster once (50%) or twice (10%) */
	p = randint0(100);

	if (p < 60) {
		struct monster_race *old = race;

		/* Pick a new monster */
		race = get_mon_race_aux(total, table);

		/* Keep the deepest one */
		if (race->level < old->level) race = old;
	}

	/* Try for a "harder" monster twice (10%) */
	if (p < 10) {
		struct monster_race *old = race;

		/* Pick a monster */
		race = get_mon_race_aux(total, table);

		/* Keep the deepest one */
		if (race->level < old->level) race = old;
	}

	/* Result */
	return race;
}

/**
 * ------------------------------------------------------------------------
 * Deleting of monsters and monster list handling
 * ------------------------------------------------------------------------ */
/**
 * Deletes a monster by index.
 *
 * When a monster is deleted, all of its objects are deleted.
 */
void delete_monster_idx(int m_idx)
{
	struct monster *mon = cave_monster(cave, m_idx);
	struct loc grid;

	assert(m_idx > 0);
	assert(square_in_bounds(cave, mon->grid));
	grid = mon->grid;

	/* Hack -- Reduce the racial counter */
	if (mon->original_race) mon->original_race->cur_num--;
	else mon->race->cur_num--;

	/* Count the number of "reproducers" */
	if (rf_has(mon->race->flags, RF_MULTIPLY)) {
		cave->num_repro--;
	}

	/* Affect light? */
	if (mon->race->light != 0)
		player->upkeep->update |= PU_UPDATE_VIEW | PU_MONSTERS;

	/* Hack -- remove target monster */
	if (target_get_monster() == mon)
		target_set_monster(NULL);

	/* Hack -- remove tracked monster */
	if (player->upkeep->health_who == mon)
		health_track(player->upkeep, NULL);

	/* Hack -- remove any command status */
	if (mon->m_timed[MON_TMD_COMMAND]) {
		(void) player_clear_timed(player, TMD_COMMAND, true);
	}

	/* Monster is gone from square and group */
	square_set_mon(cave, grid, 0);
	monster_remove_from_groups(cave, mon);

	/* Delete objects */
	struct object *obj = mon->held_obj;
	while (obj) {
		struct object *next = obj->next;

		/* Preserve unseen artifacts (we assume they were created as this
		 * monster's drop) - this will cause unintended behaviour in preserve
		 * off mode if monsters can pick up artifacts */
		if (obj->artifact && !obj_is_known_artifact(obj)) {
			obj->artifact->created = false;
		}

		/* Delete the object.  Since it's in the cave's list do
		 * some additional bookkeeping. */
		if (obj->known) {
			/* It's not in a floor pile so remove it completely.
			 * Once compatibility with old savefiles isn't needed
			 * can skip the test and simply delist and delete
			 * since any obj->known from a monster's inventory
			 * will not be in a floor pile. */
			if (loc_is_zero(obj->known->grid)) {
				delist_object(player->cave, obj->known);
				object_delete(&obj->known);
			}
		}
		delist_object(cave, obj);
		object_delete(&obj);
		obj = next;
	}

	/* Delete mimicked objects */
	if (mon->mimicked_obj) {
		square_delete_object(cave, mon->grid, mon->mimicked_obj, true, false);
	}

	/* Wipe the Monster */
	memset(mon, 0, sizeof(struct monster));

	/* Count monsters */
	cave->mon_cnt--;

	/* Visual update */
	square_light_spot(cave, grid);
}


/**
 * Deletes the monster, if any, at the given location.
 */
void delete_monster(struct loc grid)
{
	assert(square_in_bounds(cave, grid));

	/* Delete the monster (if any) */
	if (square(cave, grid)->mon > 0)
		delete_monster_idx(square(cave, grid)->mon);
}


/**
 * Move a monster from index i1 to index i2 in the monster list.
 *
 * This should only be called when there is an actual monster at i1
 */
void monster_index_move(int i1, int i2)
{
	struct monster *mon;
	struct object *obj;

	/* Do nothing */
	if (i1 == i2) return;

	/* Old monster */
	mon = cave_monster(cave, i1);
	if (!mon) return;

	/* Update the cave */
	square_set_mon(cave, mon->grid, i2);

	/* Update midx */
	mon->midx = i2;

	/* Update group */
	if (!monster_group_change_index(cave, i2, i1)) {
		quit("Bad monster group info!") ;
		monster_groups_verify(cave);
	}

	/* Repair objects being carried by monster */
	for (obj = mon->held_obj; obj; obj = obj->next)
		obj->held_m_idx = i2;

	/* Move mimicked objects (heh) */
	if (mon->mimicked_obj)
		mon->mimicked_obj->mimicking_m_idx = i2;

	/* Update the target */
	if (target_get_monster() == mon)
		target_set_monster(cave_monster(cave, i2));

	/* Update the health bar */
	if (player->upkeep->health_who == mon)
		player->upkeep->health_who = cave_monster(cave, i2);

	/* Move monster */
	memcpy(cave_monster(cave, i2),
			cave_monster(cave, i1),
			sizeof(struct monster));

	/* Wipe hole */
	memset(cave_monster(cave, i1), 0, sizeof(struct monster));
}


/**
 * Compacts and reorders the monster list.
 *
 * This function can be very dangerous, use with caution!
 *
 * When `num_to_compact` is 0, we just reorder the monsters into a more compact
 * order, eliminating any "holes" left by dead monsters. If `num_to_compact` is
 * positive, then we delete at least that many monsters and then reorder.
 * We try not to delete monsters that are high level or close to the player.
 * Each time we make a full pass through the monster list, if we haven't
 * deleted enough monsters, we relax our bounds a little to accept
 * monsters of a slightly higher level, and monsters slightly closer to
 * the player.
 */
void compact_monsters(int num_to_compact)
{
	int m_idx, num_compacted, iter;

	int max_lev, min_dis, chance;


	/* Message (only if compacting) */
	if (num_to_compact)
		msg("Compacting monsters...");


	/* Compact at least 'num_to_compact' objects */
	for (num_compacted = 0, iter = 1; num_compacted < num_to_compact; iter++) {
		/* Get more vicious each iteration */
		max_lev = 5 * iter;

		/* Get closer each iteration */
		min_dis = 5 * (20 - iter);

		/* Check all the monsters */
		for (m_idx = 1; m_idx < cave_monster_max(cave); m_idx++) {
			struct monster *mon = cave_monster(cave, m_idx);

			/* Skip "dead" monsters */
			if (!mon->race) continue;

			/* High level monsters start out "immune" */
			if (mon->race->level > max_lev) continue;

			/* Ignore nearby monsters */
			if ((min_dis > 0) && (mon->cdis < min_dis)) continue;

			/* Saving throw chance */
			chance = 90;

			/* Only compact "Quest" Monsters in emergencies */
			if (rf_has(mon->race->flags, RF_QUESTOR) && (iter < 1000))
				chance = 100;

			/* Try not to compact Unique Monsters */
			if (rf_has(mon->race->flags, RF_UNIQUE)) chance = 99;

			/* All monsters get a saving throw */
			if (randint0(100) < chance) continue;

			/* Delete the monster */
			delete_monster(mon->grid);

			/* Count the monster */
			num_compacted++;
		}
	}


	/* Excise dead monsters (backwards!) */
	for (m_idx = cave_monster_max(cave) - 1; m_idx >= 1; m_idx--) {
		struct monster *mon = cave_monster(cave, m_idx);

		/* Skip real monsters */
		if (mon->race) continue;

		/* Move last monster into open hole */
		monster_index_move(cave_monster_max(cave) - 1, m_idx);

		/* Compress "cave->mon_max" */
		cave->mon_max--;
	}
}


/**
 * Deletes all the monsters when the player leaves the level.
 *
 * This is an efficient method of simulating multiple calls to the
 * "delete_monster()" function, with no visual effects.
 *
 * Note that we must delete the objects the monsters are carrying, but we
 * do nothing with mimicked objects.
 */
void wipe_mon_list(struct chunk *c, struct player *p)
{
	int m_idx, i;

	/* Delete all the monsters */
	for (m_idx = cave_monster_max(c) - 1; m_idx >= 1; m_idx--) {
		struct monster *mon = cave_monster(c, m_idx);
		struct object *held_obj = mon ? mon->held_obj : NULL;

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Delete all the objects */
		if (held_obj) {
			/* Go through all held objects and check for artifacts */
			struct object *obj = held_obj;
			while (obj) {
				if (obj->artifact && !(obj->known && obj->known->artifact))
					obj->artifact->created = false;
				/*
				 * Also, remove from the cave's object list.
				 * That way, the scan for orphaned objects
				 * in cave_free() doesn't attempt to
				 * access freed memory or free memory
				 * twice.
				 */
				if (obj->oidx) {
					c->objects[obj->oidx] = NULL;
				}
				obj = obj->next;
			}
			object_pile_free(held_obj);
		}

		/* Reduce the racial counter */
		if (mon->original_race) mon->original_race->cur_num--;
		else mon->race->cur_num--;

		/* Monster is gone from square */
		square_set_mon(c, mon->grid, 0);

		/* Wipe the Monster */
		memset(mon, 0, sizeof(struct monster));
	}

	/* Delete all the monster groups */
	for (i = 1; i < z_info->level_monster_max; i++) {
		if (c->monster_groups[i]) {
			monster_group_free(c, c->monster_groups[i]);
		}
	}

	/* Reset "cave->mon_max" */
	c->mon_max = 1;

	/* Reset "mon_cnt" */
	c->mon_cnt = 0;

	/* Reset "reproducer" count */
	c->num_repro = 0;

	/* Hack -- no more target */
	target_set_monster(0);

	/* Hack -- no more tracking */
	health_track(p->upkeep, 0);
}

/**
 * ------------------------------------------------------------------------
 * Monster creation utilities:
 *  Getting a new monster index
 *  Creating objects for monsters to carry or mimic
 *  Calculating hitpoints
 * ------------------------------------------------------------------------ */
/**
 * Returns the index of a "free" monster, or 0 if no slot is available.
 *
 * This routine should almost never fail, but it *can* happen.
 * The calling code must check for and handle a 0 return.
 */
s16b mon_pop(struct chunk *c)
{
	int m_idx;

	/* Normal allocation */
	if (cave_monster_max(c) < z_info->level_monster_max) {
		/* Get the next hole */
		m_idx = cave_monster_max(c);

		/* Expand the array */
		c->mon_max++;

		/* Count monsters */
		c->mon_cnt++;

		return m_idx;
	}

	/* Recycle dead monsters if we've run out of room */
	for (m_idx = 1; m_idx < cave_monster_max(c); m_idx++) {
		struct monster *mon = cave_monster(c, m_idx);

		/* Skip live monsters */
		if (!mon->race) {
			/* Count monsters */
			c->mon_cnt++;

			/* Use this monster */
			return m_idx;
		}
	}

	/* Warn the player if no index is available */
	if (character_dungeon)
		msg("Too many monsters!");

	/* Try not to crash */
	return 0;
}


/**
 * Return the number of things dropped by a monster.
 *
 * \param race is the monster race.
 * \param maximize should be set to false for a random number, true to find
 * out the maximum count.
 * \param specific if true, specific drops will be included in the total
 * returned; otherwise they are excluded from that value.
 * \param specific_count if not NULL, *specific_count will be set to the
 * number of specific objects (either a random value or the maximum if maximize
 * is true).
 */
int mon_create_drop_count(const struct monster_race *race, bool maximize,
	bool specific, int *specific_count)
{
	int number = 0;
	int specnum = 0;
	static const int drop_4_max = 6;
	static const int drop_3_max = 4;
	static const int drop_2_max = 3;
	struct monster_drop *drop;

	if (maximize) {
		if (rf_has(race->flags, RF_DROP_20)) number++;
		if (rf_has(race->flags, RF_DROP_40)) number++;
		if (rf_has(race->flags, RF_DROP_60)) number++;
		if (rf_has(race->flags, RF_DROP_4)) number += drop_4_max;
		if (rf_has(race->flags, RF_DROP_3)) number += drop_3_max;
		if (rf_has(race->flags, RF_DROP_2)) number += drop_2_max;
		if (rf_has(race->flags, RF_DROP_1)) number++;
		for (drop = race->drops; drop; drop = drop->next) {
			specnum += drop->max;
		}
	} else {
		if (rf_has(race->flags, RF_DROP_20) && randint0(100) < 20) number++;
		if (rf_has(race->flags, RF_DROP_40) && randint0(100) < 40) number++;
		if (rf_has(race->flags, RF_DROP_60) && randint0(100) < 60) number++;
		if (rf_has(race->flags, RF_DROP_4)) number += rand_range(2, drop_4_max);
		if (rf_has(race->flags, RF_DROP_3)) number += rand_range(2, drop_3_max);
		if (rf_has(race->flags, RF_DROP_2)) number += rand_range(1, drop_2_max);
		if (rf_has(race->flags, RF_DROP_1)) number++;
		for (drop = race->drops; drop; drop = drop->next) {
			if ((unsigned int)randint0(100) <
					drop->percent_chance) {
				specnum += randint0(drop->max - drop->min) +
					drop->min;
			}
		}
	}
	if (specific) {
		number += specnum;
	}
	if (specific_count) {
		*specific_count = specnum;
	}
	return number;
}

/**
 * Creates a specific monster's drop, including any drops specified
 * in the monster.txt file.
 *
 * Returns true if anything is created, false if nothing is.
 */
static bool mon_create_drop(struct chunk *c, struct monster *mon, byte origin)
{
	struct monster_drop *drop;
	struct monster_lore *lore = get_lore(mon->race);

	bool great, good, gold_ok, item_ok;
	bool extra_roll = false;
	bool any = false;

	int number = 0, level, j, monlevel;

	struct object *obj;

	assert(mon);

	great = (rf_has(mon->race->flags, RF_DROP_GREAT));
	good = great || (rf_has(mon->race->flags, RF_DROP_GOOD));
	gold_ok = (!rf_has(mon->race->flags, RF_ONLY_ITEM));
	item_ok = (!rf_has(mon->race->flags, RF_ONLY_GOLD));

	/* Determine how much we can drop */
	number = mon_create_drop_count(mon->race, false, false, NULL);

	/* Uniques that have been stolen from get their quantity reduced */
	if (rf_has(mon->race->flags, RF_UNIQUE)) {
		number = MAX(0, number - lore->thefts);
	}

	/* Give added bonus for unique monsters */
	monlevel = mon->race->level;
	if (rf_has(mon->race->flags, RF_UNIQUE)) {
		monlevel = MIN(monlevel + 15, monlevel * 2);
		extra_roll = true;
	}

	/* Take the best of (average of monster level and current depth)
	   and (monster level) - to reward fighting OOD monsters */
	level = MAX((monlevel + player->depth) / 2, monlevel);
	level = MIN(level, 100);

	/* Morgoth currently drops all artifacts with the QUEST_ART flag */
	if (rf_has(mon->race->flags, RF_QUESTOR) && (mon->race->level == 100)) {
		/* Search all the artifacts */
		for (j = 1; j < z_info->a_max; j++) {
			struct artifact *art = &a_info[j];
			struct object_kind *kind = lookup_kind(art->tval, art->sval);
			if (!kf_has(kind->kind_flags, KF_QUEST_ART)) {
				continue;
			}

			/* Allocate by hand, prep, apply magic */
			obj = mem_zalloc(sizeof(*obj));
			object_prep(obj, kind, 100, RANDOMISE);
			obj->artifact = art;
			copy_artifact_data(obj, obj->artifact);
			obj->artifact->created = true;

			/* Set origin details */
			obj->origin = origin;
			obj->origin_depth = player->depth;
			obj->origin_race = mon->race;
			obj->number = 1;

			/* Try to carry */
			if (monster_carry(c, mon, obj)) {
				any = true;
			} else {
				obj->artifact->created = false;
				object_wipe(obj);
				mem_free(obj);
			}
		}
	}

	/* Specified drops */
	for (drop = mon->race->drops; drop; drop = drop->next) {
		if ((unsigned int)randint0(100) >= drop->percent_chance)
			continue;

		/* Specified by tval or by kind */
		if (drop->kind) {
			/* Allocate by hand, prep, apply magic */
			obj = mem_zalloc(sizeof(*obj));
			object_prep(obj, drop->kind, level, RANDOMISE);
			apply_magic(obj, level, true, good, great, extra_roll);
		} else {
			/* Choose by set tval */
			assert(drop->tval);
			obj = make_object(c, level, good, great, extra_roll, NULL,
							  drop->tval);
		}

		/* Skip if the object couldn't be created. */
		if (!obj) continue;

		/* Set origin details */
		obj->origin = origin;
		obj->origin_depth = player->depth;
		obj->origin_race = mon->race;
		obj->number = (obj->artifact) ?
			1 : randint0(drop->max - drop->min) + drop->min;

		/* Try to carry */
		if (monster_carry(c, mon, obj)) {
			any = true;
		} else {
			object_wipe(obj);
			mem_free(obj);
		}
	}

	/* Make some objects */
	for (j = 0; j < number; j++) {
		if (gold_ok && (!item_ok || (randint0(100) < 50))) {
			obj = make_gold(level, "any");
		} else {
			obj = make_object(c, level, good, great, extra_roll, NULL, 0);
			if (!obj) continue;
		}

		/* Set origin details */
		obj->origin = origin;
		obj->origin_depth = player->depth;
		obj->origin_race = mon->race;

		/* Try to carry */
		if (monster_carry(c, mon, obj)) {
			any = true;
		} else {
			if (obj->artifact) {
				obj->artifact->created = false;
			}
			object_wipe(obj);
			mem_free(obj);
		}
	}

	return any;
}


/**
 * Creates the onbject a mimic is imitating.
 */
void mon_create_mimicked_object(struct chunk *c, struct monster *mon, int index)
{
	struct object *obj;
	struct object_kind *kind = mon->race->mimic_kinds->kind;
	struct monster_mimic *mimic_kind;
	int i = 1;
	bool dummy = true;

	/* Pick a random object kind to mimic */
	for (mimic_kind = mon->race->mimic_kinds;
		 mimic_kind;
		 mimic_kind = mimic_kind->next, i++) {
		if (one_in_(i)) {
			kind = mimic_kind->kind;
		}
	}

	if (tval_is_money_k(kind)) {
		obj = make_gold(player->depth, kind->name);
	} else {
		obj = object_new();
		object_prep(obj, kind, mon->race->level, RANDOMISE);
		apply_magic(obj, mon->race->level, true, false, false, false);
		obj->number = 1;
		obj->origin = ORIGIN_DROP_MIMIC;
		obj->origin_depth = player->depth;
	}

	obj->mimicking_m_idx = index;
	mon->mimicked_obj = obj;

	/* Put the object on the floor if it goes, otherwise no mimicry */
	if (floor_carry(c, mon->grid, obj, &dummy)) {
		list_object(c, obj);
	} else {
		/* Clear the mimicry */
		obj->mimicking_m_idx = 0;
		mon->mimicked_obj = NULL;

		/* Give the object to the monster if appropriate */
		if (rf_has(mon->race->flags, RF_MIMIC_INV)) {
			monster_carry(c, mon, obj);
		} else {
			/* Otherwise delete the mimicked object */
			object_delete(&obj);
		}
	}
}

/**
 * Calculates hp for a monster. This function assumes that the Rand_normal
 * function has limits of +/- 4x std_dev. If that changes, this function
 * will become inaccurate.
 *
 * \param race is the race of the monster in question.
 * \param hp_aspect is the hp calc we want (min, max, avg, random).
 */
int mon_hp(const struct monster_race *race, aspect hp_aspect)
{
	int std_dev = (((race->avg_hp * 10) / 8) + 5) / 10;

	if (race->avg_hp > 1) std_dev++;

	switch (hp_aspect) {
		case MINIMISE:
			return (race->avg_hp - (4 * std_dev));
		case MAXIMISE:
		case EXTREMIFY:
			return (race->avg_hp + (4 * std_dev));
		case AVERAGE:
			return race->avg_hp;
		case RANDOMISE:
			return Rand_normal(race->avg_hp, std_dev);
	}

	assert(0 && "Should never reach here");
	return 0;
}


/**
 * ------------------------------------------------------------------------
 * Placement of a single monster
 * These are the functions that actually put the monster into the world
 * ------------------------------------------------------------------------ */
/**
 * Attempts to place a copy of the given monster at the given position in
 * the dungeon.
 *
 * All of the monster placement routines eventually call this function. This
 * is what actually puts the monster in the dungeon (i.e., it notifies the cave
 * and sets the monster's position). The dungeon loading code also calls this
 * function directly.
 *
 * `origin` is the item origin to use for any monster drops (e.g. ORIGIN_DROP,
 * ORIGIN_DROP_PIT, etc.) The dungeon loading code calls this with origin = 0,
 * which prevents the monster's drops from being generated again.
 *
 * Returns the m_idx of the newly copied monster, or 0 if the placement fails.
 */
s16b place_monster(struct chunk *c, struct loc grid, struct monster *mon,
				   byte origin)
{
	s16b m_idx;
	struct monster *new_mon;
	struct monster_group_info *info = mon->group_info;
	bool loading = mon->midx > 0;

	assert(square_in_bounds(c, grid));
	assert(!square_monster(c, grid));

	/* Get a new record, or recycle the old one */
	if (loading) {
		m_idx = mon->midx;
		c->mon_max++;
		c->mon_cnt++;
	} else {
		m_idx = mon_pop(c);
		if (!m_idx) return 0;
	}

	/* Copy the monster */
	new_mon = cave_monster(c, m_idx);
	memcpy(new_mon, mon, sizeof(struct monster));

	/* Set the ID */
	new_mon->midx = m_idx;

	/* Set the location */
	square_set_mon(c, grid, new_mon->midx);
	new_mon->grid = grid;
	assert(square_monster(c, grid) == new_mon);

	/* Assign monster to its monster group */
	monster_group_assign(c, new_mon, info, loading);

	update_mon(new_mon, c, true);

	/* Count the number of "reproducers" */
	if (rf_has(new_mon->race->flags, RF_MULTIPLY)) c->num_repro++;

	/* Count racial occurrences */
	if (new_mon->original_race) new_mon->original_race->cur_num++;
	else new_mon->race->cur_num++;

	/* Create the monster's drop, if any */
	if (origin)
		(void)mon_create_drop(c, new_mon, origin);

	/* Make mimics start mimicking */
	if (origin && new_mon->race->mimic_kinds) {
		mon_create_mimicked_object(c, new_mon, m_idx);
	}

	/* Result */
	return m_idx;
}

/**
 * Attempts to place a monster of the given race at the given location.
 *
 * If `sleep` is true, the monster is placed with its default sleep value,
 * which is given in monster.txt.
 *
 * `origin` is the item origin to use for any monster drops (e.g. ORIGIN_DROP,
 * ORIGIN_DROP_PIT, etc.)
 *
 * To give the player a sporting chance, some especially dangerous
 * monsters are marked as "FORCE_SLEEP" in monster.txt, which will
 * cause them to be placed with low energy. This helps ensure that
 * if such a monster suddenly appears in line-of-sight (due to a
 * summon, for instance), the player gets a chance to move before
 * they do.
 *
 * This routine refuses to place out-of-depth "FORCE_DEPTH" monsters.
 *
 * This is the only function which may place a monster in the dungeon,
 * except for the savefile loading code, which calls place_monster()
 * directly.
 */
static bool place_new_monster_one(struct chunk *c, struct loc grid,
								  struct monster_race *race, bool sleep,
								  struct monster_group_info group_info,
								  byte origin)
{
	int i;
	struct monster *mon;
	struct monster monster_body;

	assert(square_in_bounds(c, grid));
	assert(race && race->name);

	/* Not where monsters already are */
	if (square_monster(c, grid)) return false;

	/* Not where the player already is */
	if (loc_eq(player->grid, grid)) return false;

	/* Prevent monsters from being placed where they cannot walk, but allow
	 * other feature types */
	if (!square_is_monster_walkable(c, grid)) return false;

	/* No creation on glyphs */
	if (square_iswarded(c, grid) || square_isdecoyed(c, grid)) return false;

	/* "unique" monsters must be "unique" */
	if (rf_has(race->flags, RF_UNIQUE) && (race->cur_num >= race->max_num))
		return false;

	/* Depth monsters may NOT be created out of depth */
	if (rf_has(race->flags, RF_FORCE_DEPTH) && player->depth < race->level)
		return false;

	/* Add to level feeling, note uniques for cheaters */
	c->mon_rating += race->level * race->level;

	/* Check out-of-depth-ness */
	if (race->level > c->depth) {
		if (rf_has(race->flags, RF_UNIQUE)) { /* OOD unique */
			if (OPT(player, cheat_hear))
				msg("Deep unique (%s).", race->name);
		} else { /* Normal monsters but OOD */
			if (OPT(player, cheat_hear))
				msg("Deep monster (%s).", race->name);
		}
		/* Boost rating by power per 10 levels OOD */
		c->mon_rating += (race->level - c->depth) * race->level * race->level;
	} else if (rf_has(race->flags, RF_UNIQUE) && OPT(player, cheat_hear)) {
		msg("Unique (%s).", race->name);
	}

	/* Get local monster */
	mon = &monster_body;

	/* Clean out the monster */
	memset(mon, 0, sizeof(struct monster));

	/* Save the race */
	mon->race = race;

	/* Enforce sleeping if needed */
	if (sleep && race->sleep) {
		int val = race->sleep;
		mon->m_timed[MON_TMD_SLEEP] = ((val * 2) + randint1(val * 10));
	}

	/* Uniques get a fixed amount of HP */
	if (rf_has(race->flags, RF_UNIQUE))
		mon->maxhp = race->avg_hp;
	else {
		mon->maxhp = mon_hp(race, RANDOMISE);
		mon->maxhp = MAX(mon->maxhp, 1);
	}

	/* And start out fully healthy */
	mon->hp = mon->maxhp;

	/* Extract the monster base speed */
	mon->mspeed = race->speed;

	/* Hack -- small racial variety */
	if (!rf_has(race->flags, RF_UNIQUE)) {
		/* Allow some small variation per monster */
		i = turn_energy(race->speed) / 10;
		if (i) mon->mspeed += rand_spread(0, i);
	}

	/* Give a random starting energy */
	mon->energy = (byte)randint0(50);

	/* Force monster to wait for player */
	if (rf_has(race->flags, RF_FORCE_SLEEP))
		mflag_on(mon->mflag, MFLAG_NICE);

	/* Affect light? */
	if (mon->race->light != 0)
		player->upkeep->update |= PU_UPDATE_VIEW | PU_MONSTERS;

	/* Is this obviously a monster? (Mimics etc. aren't) */
	if (rf_has(race->flags, RF_UNAWARE))
		mflag_on(mon->mflag, MFLAG_CAMOUFLAGE);
	else
		mflag_off(mon->mflag, MFLAG_CAMOUFLAGE);

	/* Set the color if necessary */
	if (rf_has(race->flags, RF_ATTR_RAND))
		mon->attr = randint1(BASIC_COLORS - 1);

	/* Set the group info */
	mon->group_info[PRIMARY_GROUP].index = group_info.index;
	mon->group_info[PRIMARY_GROUP].role = group_info.role;

	/* Place the monster in the dungeon */
	if (!place_monster(c, grid, mon, origin))
		return (false);

	/* Success */
	return (true);
}


/**
 * ------------------------------------------------------------------------
 * More complex monster placement routines
 * ------------------------------------------------------------------------ */
/**
 * Attempts to place a group of monsters of race `r_idx` around
 * the given location. The number of monsters to place is `total`.
 *
 * If `sleep` is true, the monster is placed with its default sleep value,
 * which is given in monster.txt.
 *
 * `origin` is the item origin to use for any monster drops (e.g. ORIGIN_DROP,
 * ORIGIN_DROP_PIT, etc.)
 */
static bool place_new_monster_group(struct chunk *c, struct loc grid,
									struct monster_race *race, bool sleep,
									struct monster_group_info group_info,
									int total, byte origin)
{
	int n, i;

	int loc_num;

	/* Locations of the placed monsters */
	struct loc *loc_list = mem_zalloc(sizeof(struct loc) *
									  z_info->monster_group_max);

	/* Sanity and bounds check */
	assert(race);
	total = MIN(total, z_info->monster_group_max);

	/* Start on the monster */
	loc_num = 1;
	loc_list[0] = grid;

	/* Puddle monsters, breadth first, up to total */
	for (n = 0; (n < loc_num) && (loc_num < total); n++) {
		/* Check each direction, up to total */
		for (i = 0; (i < 8) && (loc_num < total); i++) {
			struct loc try = loc_sum(loc_list[n], ddgrid_ddd[i]);

			/* Walls and Monsters block flow */
			if (!square_isempty(c, try)) continue;

			/* Attempt to place another monster */
			if (place_new_monster_one(c, try, race, sleep, group_info, origin)){
				/* Add it to the "hack" set */
				loc_list[loc_num] = try;
				loc_num++;
			}
		}
	}

	/* Success */
	mem_free(loc_list);
	return (true);
}

static struct monster_base *place_monster_base = NULL;

/**
 * Predicate function for get_mon_num_prep()
 * Check to see if the monster race has the same base as
 * place_monter_base.
 */
static bool place_monster_base_okay(struct monster_race *race)
{
	assert(place_monster_base);
	assert(race);

	/* Check if it matches */
	if (race->base != place_monster_base) return false;

	/* No uniques */
	if (rf_has(race->flags, RF_UNIQUE)) return false;

	return true;
}

/**
 * Helper function to place monsters that appear as friends or escorts
 */
static bool place_friends(struct chunk *c, struct loc grid, struct monster_race *race,
					struct monster_race *friends_race, int total, bool sleep,
					struct monster_group_info group_info, byte origin)
{
	int extra_chance;

	/* Find the difference between current dungeon depth and monster level */
	int level_difference = player->depth - friends_race->level + 5;

	/* Handle unique monsters */
	bool is_unique = rf_has(friends_race->flags, RF_UNIQUE);

	/* Make sure the unique hasn't been killed already */
	if (is_unique && (friends_race->cur_num >= friends_race->max_num)) {
		return false;
	}

	/* More than 4 levels OoD, no groups allowed */
	if (level_difference <= 0 && !is_unique) {
		return false;
	}

	/* Reduce group size within 5 levels of natural depth*/
	if (level_difference < 10 && !is_unique) {
		extra_chance = (total * level_difference) % 10;
		total = total * level_difference / 10;

		/* Instead of flooring the group value, we use the decimal place
		   as a chance of an extra monster */
		if (randint0(10) > extra_chance) {
			total += 1;
		}
	}

	/* No monsters in this group */
	if (total > 0) {
		/* Handle friends same as original monster */
		if (race->ridx == friends_race->ridx) {
			return place_new_monster_group(c, grid, race, sleep, group_info,
										   total, origin);
		} else {
			int j;
			struct loc new;

			/* Find a nearby place to put the other groups */
			for (j = 0; j < 50; j++) {
				scatter(c, &new, grid, z_info->monster_group_dist, false);
				if (square_isopen(c, new)) {
					break;
				}
			}

			/* Place the monsters */
			bool success = place_new_monster_one(c, new, friends_race, sleep,
												 group_info, origin);
			if (total > 1)
				success = place_new_monster_group(c, new, friends_race, sleep,
												  group_info, total, origin);

			return success;
		}
	}

	return false;
}

/**
 * Attempts to place a monster of the given race at the given location.
 *
 * Note that certain monsters are placed with a large group of
 * identical or similar monsters. However, if `group_okay` is false,
 * then such monsters are placed by themselves.
 *
 * If `sleep` is true, the monster is placed with its default sleep value,
 * which is given in monster.txt.
 *
 * `origin` is the item origin to use for any monster drops (e.g. ORIGIN_DROP,
 * ORIGIN_DROP_PIT, etc.)
 */
bool place_new_monster(struct chunk *c, struct loc grid,
					   struct monster_race *race, bool sleep, bool group_ok,
					   struct monster_group_info group_info, byte origin)
{
	struct monster_friends *friends;
	struct monster_friends_base *friends_base;
	int total;

	assert(c);
	assert(race);

	/* If we don't have a group index already, make one; our first monster
	 * will be the leader */
	if (!group_info.index) {
		group_info.index = monster_group_index_new(c);
	}

	/* Place one monster, or fail */
	if (!place_new_monster_one(c, grid, race, sleep, group_info, origin)) {
		return (false);
	}

	/* We're done unless the group flag is set */
	if (!group_ok) return (true);

	/* Go through friends flags */
	for (friends = race->friends; friends; friends = friends->next) {
		if ((unsigned int)randint0(100) >= friends->percent_chance)
			continue;

		/* Calculate the base number of monsters to place */
		total = damroll(friends->number_dice, friends->number_side);

		/* Set group role */
		group_info.role = friends->role;

		/* Place them */
		place_friends(c, grid, race, friends->race, total, sleep, group_info,
					  origin);

	}

	/* Go through the friends_base flags */
	for (friends_base = race->friends_base; friends_base;
			friends_base = friends_base->next){
		struct monster_race *friends_race;

		/* Check if we pass chance for the monster appearing */
		if ((unsigned int)randint0(100) >= friends_base->percent_chance)
			continue;

		total = damroll(friends_base->number_dice, friends_base->number_side);

		/* Set the escort index base*/
		place_monster_base = friends_base->base;

		/* Prepare allocation table */
		get_mon_num_prep(place_monster_base_okay);

		/* Pick a random race */
		friends_race = get_mon_num(race->level);

		/* Reset allocation table */
		get_mon_num_prep(NULL);

		/* Handle failure */
		if (!friends_race) break;

		/* Set group role */
		group_info.role = friends_base->role;

		/* Place them */
		place_friends(c, grid, race, friends_race, total, sleep, group_info,
					  origin);
	}

	/* Success */
	return (true);
}


/**
 * Picks a monster race, makes a new monster of that race, then attempts to
 * place it in the dungeon. The monster race chosen will be appropriate for
 * dungeon level equal to `depth`.
 *
 * If `sleep` is true, the monster is placed with its default sleep value,
 * which is given in monster.txt.
 *
 * If `group_okay` is true, we allow the placing of a group, if the chosen
 * monster appears with friends or an escort.
 *
 * `origin` is the item origin to use for any monster drops (e.g. ORIGIN_DROP,
 * ORIGIN_DROP_PIT, etc.)
 *
 * Returns true if we successfully place a monster.
 */
bool pick_and_place_monster(struct chunk *c, struct loc grid, int depth,
							bool sleep, bool group_okay, byte origin)
{
	/* Pick a monster race, no specified group */
	struct monster_race *race = get_mon_num(depth);
	struct monster_group_info info = { 0, 0 };

	if (race) {
		return place_new_monster(c, grid, race, sleep, group_okay, info,
								 origin);
	} else {
		return false;
	}
}


/**
 * Picks a monster race, makes a new monster of that race, then attempts to
 * place it in the dungeon at least `dis` away from the player. The monster
 * race chosen will be appropriate for dungeon level equal to `depth`.
 *
 * If `sleep` is true, the monster is placed with its default sleep value,
 * which is given in monster.txt.
 *
 * Returns true if we successfully place a monster.
 */
bool pick_and_place_distant_monster(struct chunk *c, struct player *p, int dis,
		bool sleep, int depth)
{
	struct loc grid;
	int	attempts_left = 10000;

	assert(c);

	/* Find a legal, distant, unoccupied, space */
	while (--attempts_left) {
		/* Pick a location */
		grid = loc(randint0(c->width), randint0(c->height));

		/* Require "naked" floor grid */
		if (!square_isempty(c, grid)) continue;

		/* Do not put random monsters in marked rooms. */
		if ((!character_dungeon) && square_ismon_restrict(c, grid))
			continue;

		/* Accept far away grids */
		if (distance(grid, p->grid) > dis) break;
	}

	if (!attempts_left) {
		if (OPT(p, cheat_xtra) || OPT(p, cheat_hear))
			msg("Warning! Could not allocate a new monster.");

		return false;
	}

	/* Attempt to place the monster, allow groups */
	if (pick_and_place_monster(c, grid, depth, sleep, true, ORIGIN_DROP))
		return (true);

	/* Nope */
	return (false);
}

struct init_module mon_make_module = {
	.name = "monster/mon-make",
	.init = init_race_allocs,
	.cleanup = cleanup_race_allocs
};
