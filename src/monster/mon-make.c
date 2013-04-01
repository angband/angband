/*
 * File: mon-make.c
 * Purpose: Monster creation / placement code.
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
#include "history.h"
#include "init.h"
#include "target.h"
#include "monster/mon-lore.h"
#include "monster/mon-make.h"
#include "monster/mon-timed.h"
#include "monster/mon-util.h"
#include "object/tvalsval.h"

s16b num_repro;

static s16b alloc_race_size;
static struct alloc_entry *alloc_race_table;

void init_race_allocs(void) {
	int i;
	monster_race *r_ptr;
	alloc_entry *table;
	s16b num[MAX_DEPTH];
	s16b aux[MAX_DEPTH];

	/* Clear the "aux" array */
	(void)C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(num, MAX_DEPTH, s16b);

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Legal monsters */
		if (r_ptr->rarity)
		{
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[r_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town monsters!");


	/*** Initialize monster allocation info ***/

	/* Allocate the alloc_race_table */
	alloc_race_table = C_ZNEW(alloc_race_size, alloc_entry);

	/* Get the table entry */
	table = alloc_race_table;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Count valid pairs */
		if (r_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = r_ptr->level;

			/* Extract the base probability */
			p = (100 / r_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x-1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}

}

void cleanup_race_allocs(void) {
	FREE(alloc_race_table);
}

/**
 * Deletes a monster by index.
 *
 * When a monster is deleted, all of its objects are deleted.
 */
void delete_monster_idx(int m_idx)
{
	int x, y;
	s16b this_o_idx, next_o_idx = 0;
	monster_type *m_ptr;

	assert(m_idx > 0);

	m_ptr = cave_monster(cave, m_idx);

	/* Monster location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Hack -- Reduce the racial counter */
	m_ptr->race->cur_num--;

	/* Hack -- count the number of "reproducers" */
	if (rf_has(m_ptr->race->flags, RF_MULTIPLY)) num_repro--;

	/* Hack -- remove target monster */
	if (target_get_monster() == m_idx) target_set_monster(0);

	/* Hack -- remove tracked monster */
	if (p_ptr->health_who == m_ptr) health_track(p_ptr, NULL);

	/* Monster is gone */
	cave->m_idx[y][x] = 0;

	/* Delete objects */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Preserve unseen artifacts (we assume they were created as this
		 * monster's drop) - this will cause unintended behaviour in preserve
		 * off mode if monsters can pick up artifacts */
		if (o_ptr->artifact && !object_was_sensed(o_ptr))
			o_ptr->artifact->created = FALSE;

		/* Clear held_m_idx now to avoid wasting time in delete_object_idx */
		o_ptr->held_m_idx = 0;

		/* Delete the object */
		delete_object_idx(this_o_idx);
	}

	/* Delete mimicked objects */
	if (m_ptr->mimicked_o_idx > 0)
		delete_object_idx(m_ptr->mimicked_o_idx);

	/* Wipe the Monster */
	(void)WIPE(m_ptr, monster_type);

	/* Count monsters */
	cave->mon_cnt--;

	/* Visual update */
	cave_light_spot(cave, y, x);
}


/**
 * Deletes the monster, if any, at the given location.
 */
void delete_monster(int y, int x)
{
	assert(in_bounds(y, x));

	/* Delete the monster (if any) */
	if (cave->m_idx[y][x] > 0)
		delete_monster_idx(cave->m_idx[y][x]);
}


/**
 * Move a monster from index i1 to index i2 in the monster list.
 */
static void compact_monsters_aux(int i1, int i2)
{
	int y, x;

	monster_type *m_ptr;

	s16b this_o_idx, next_o_idx = 0;

	/* Do nothing */
	if (i1 == i2) return;

	/* Old monster */
	m_ptr = cave_monster(cave, i1);
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Update the cave */
	cave->m_idx[y][x] = i2;
	
	/* Update midx */
	m_ptr->midx = i2;

	/* Repair objects being carried by monster */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Reset monster pointer */
		o_ptr->held_m_idx = i2;
	}
	
	/* Move mimicked objects */
	if (m_ptr->mimicked_o_idx > 0) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(m_ptr->mimicked_o_idx);

		/* Reset monster pointer */
		o_ptr->mimicking_m_idx = i2;
	}

	/* Hack -- Update the target */
	if (target_get_monster() == i1)
		target_set_monster(i2);

	/* Hack -- Update the health bar */
	if (p_ptr->health_who == m_ptr)
		p_ptr->health_who = cave_monster(cave, i2);

	/* Hack -- move monster */
	COPY(cave_monster(cave, i2), cave_monster(cave, i1), struct monster);

	/* Hack -- wipe hole */
	(void)WIPE(cave_monster(cave, i1), monster_type);
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
			monster_type *m_ptr = cave_monster(cave, m_idx);

			/* Skip "dead" monsters */
			if (!m_ptr->race) continue;

			/* High level monsters start out "immune" */
			if (m_ptr->race->level > max_lev) continue;

			/* Ignore nearby monsters */
			if ((min_dis > 0) && (m_ptr->cdis < min_dis)) continue;

			/* Saving throw chance */
			chance = 90;

			/* Only compact "Quest" Monsters in emergencies */
			if (rf_has(m_ptr->race->flags, RF_QUESTOR) && (iter < 1000)) chance = 100;

			/* Try not to compact Unique Monsters */
			if (rf_has(m_ptr->race->flags, RF_UNIQUE)) chance = 99;

			/* All monsters get a saving throw */
			if (randint0(100) < chance) continue;

			/* Delete the monster */
			delete_monster(m_ptr->fy, m_ptr->fx);

			/* Count the monster */
			num_compacted++;
		}
	}


	/* Excise dead monsters (backwards!) */
	for (m_idx = cave_monster_max(cave) - 1; m_idx >= 1; m_idx--) {
		monster_type *m_ptr = cave_monster(cave, m_idx);

		/* Skip real monsters */
		if (m_ptr->race) continue;

		/* Move last monster into open hole */
		compact_monsters_aux(cave_monster_max(cave) - 1, m_idx);

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
 * Note that we do not delete the objects the monsters are carrying;
 * that must be taken care of separately via wipe_o_list().
 */
void wipe_mon_list(struct cave *c, struct player *p)
{
	int m_idx;

	/* Delete all the monsters */
	for (m_idx = cave_monster_max(cave) - 1; m_idx >= 1; m_idx--)
	{
		monster_type *m_ptr = cave_monster(cave, m_idx);

		/* Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Hack -- Reduce the racial counter */
		m_ptr->race->cur_num--;

		/* Monster is gone */
		c->m_idx[m_ptr->fy][m_ptr->fx] = 0;

		/* Wipe the Monster */
		(void)WIPE(m_ptr, monster_type);
	}

	/* Reset "cave->mon_max" */
	cave->mon_max = 1;

	/* Reset "mon_cnt" */
	cave->mon_cnt = 0;

	/* Hack -- reset "reproducer" count */
	num_repro = 0;

	/* Hack -- no more target */
	target_set_monster(0);

	/* Hack -- no more tracking */
	health_track(p, 0);
}

/**
 * Returns the index of a "free" monster, or 0 if no slot is available.
 *
 * This routine should almost never fail, but it *can* happen.
 * The calling code must check for and handle a 0 return.
 */
static s16b mon_pop(void)
{
	int m_idx;

	/* Normal allocation */
	if (cave_monster_max(cave) < z_info->m_max) {
		/* Get the next hole */
		m_idx = cave_monster_max(cave);

		/* Expand the array */
		cave->mon_max++;

		/* Count monsters */
		cave->mon_cnt++;

		return m_idx;
	}

	/* Recycle dead monsters if we've run out of room */
	for (m_idx = 1; m_idx < cave_monster_max(cave); m_idx++) {
		monster_type *m_ptr = cave_monster(cave, m_idx);

		/* Skip live monsters */
		if (m_ptr->race) continue;

		/* Count monsters */
		cave->mon_cnt++;

		/* Use this monster */
		return m_idx;
	}

	/* Warn the player if no index is available 
	 * (except during dungeon creation)
	 */
	if (character_dungeon)
		msg("Too many monsters!");

	/* Try not to crash */
	return 0;
}


/**
 * Apply a "monster restriction function" to the "monster allocation table".
 * This way, we can use get_mon_num() to get a level-appropriate monster that
 * satisfies certain conditions (such as belonging to a particular monster
 * family).
 */
void get_mon_num_prep(bool (*get_mon_num_hook)(monster_race *race))
{
	int i;

	/* Scan the allocation table */
	for (i = 0; i < alloc_race_size; i++) {
		alloc_entry *entry = &alloc_race_table[i];

		/* Accept monsters which pass the restriction, if any */
		if (!get_mon_num_hook || (*get_mon_num_hook)(&r_info[entry->index]))
			entry->prob2 = entry->prob1;

		/* Do not use this monster */
		else
			entry->prob2 = 0;
	}

	return;
}

/**
 * Helper function for get_mon_num(). Scans the prepared monster allocation
 * table and picks a random monster. Returns the index of a monster in
 * `table`.
 */
static monster_race *get_mon_race_aux(long total, const alloc_entry *table)
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
 * Chooses a monster race that seems "appropriate" to the given level
 *
 * This function uses the "prob2" field of the "monster allocation table",
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an "appropriate" monster, in
 * a relatively efficient manner.
 *
 * Note that "town" monsters will *only* be created in the town, and
 * "normal" monsters will *never* be created in the town, unless the
 * "level" is "modified", for example, by polymorph or summoning.
 *
 * There is a small chance (1/50) of "boosting" the given depth by
 * a small amount (up to four levels), except in the town.
 *
 * It is (slightly) more likely to acquire a monster of the given level
 * than one of a lower level.  This is done by choosing several monsters
 * appropriate to the given level and keeping the "hardest" one.
 *
 * Note that if no monsters are "appropriate", then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
monster_race *get_mon_num(int level)
{
	int i, p;

	long total;

	monster_race *race;

	alloc_entry *table = alloc_race_table;

	/* Occasionally produce a nastier monster in the dungeon */
	if (level > 0 && one_in_(NASTY_MON))
		level += MIN(level / 4 + 2, MON_OOD_MAX);

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

		/* Only one copy of a a unique must be around at the same time */
		if (rf_has(race->flags, RF_UNIQUE) && 
				race->cur_num >= race->max_num)
			continue;

		/* Some monsters never appear out of depth */
		if (rf_has(race->flags, RF_FORCE_DEPTH) && 
				race->level > p_ptr->depth)
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
		monster_race *old = race;

		/* Pick a new monster */
		race = get_mon_race_aux(total, table);

		/* Keep the deepest one */
		if (race->level < old->level) race = old;
	}

	/* Try for a "harder" monster twice (10%) */
	if (p < 10) {
		monster_race *old = race;

		/* Pick a monster */
		race = get_mon_race_aux(total, table);

		/* Keep the deepest one */
		if (race->level < old->level) race = old;
	}

	/* Result */
	return race;
}


/**
 * Places the player at the given coordinates in the cave.
 */
void player_place(struct cave *c, struct player *p, int y, int x)
{
	assert(!c->m_idx[y][x]);

	/* Save player location */
	p->py = y;
	p->px = x;

	/* Mark cave grid */
	c->m_idx[y][x] = -1;
}


/**
 * Creates a specific monster's drop, including any drops specified
 * in the monster.txt file.
 *
 * Returns TRUE if anything is created, FALSE if nothing is.
 */
static bool mon_create_drop(struct monster *m_ptr, byte origin)
{
	struct monster_drop *drop;

	bool great, good, gold_ok, item_ok;
	bool any = FALSE;

	int number = 0, level, j;

	object_type *i_ptr;
	object_type object_type_body;
	
	assert(m_ptr);

	great = (rf_has(m_ptr->race->flags, RF_DROP_GREAT));
	good = great || (rf_has(m_ptr->race->flags, RF_DROP_GOOD));
	gold_ok = (!rf_has(m_ptr->race->flags, RF_ONLY_ITEM));
	item_ok = (!rf_has(m_ptr->race->flags, RF_ONLY_GOLD));

	/* Determine how much we can drop */
	if (rf_has(m_ptr->race->flags, RF_DROP_20) && randint0(100) < 20) number++;
	if (rf_has(m_ptr->race->flags, RF_DROP_40) && randint0(100) < 40) number++;
	if (rf_has(m_ptr->race->flags, RF_DROP_60) && randint0(100) < 60) number++;
	if (rf_has(m_ptr->race->flags, RF_DROP_4)) number += rand_range(2, 6);
	if (rf_has(m_ptr->race->flags, RF_DROP_3)) number += rand_range(2, 4);
	if (rf_has(m_ptr->race->flags, RF_DROP_2)) number += rand_range(1, 3);
	if (rf_has(m_ptr->race->flags, RF_DROP_1)) number++;

	/* Take the best of (average of monster level and current depth)
	   and (monster level) - to reward fighting OOD monsters */
	level = MAX((m_ptr->race->level + p_ptr->depth) / 2, m_ptr->race->level);

	/* Specified drops */
	for (drop = m_ptr->race->drops; drop; drop = drop->next) {
		if ((unsigned int)randint0(100) >= drop->percent_chance)
			continue;

		i_ptr = &object_type_body;
		if (drop->artifact) {
			object_prep(i_ptr, objkind_get(drop->artifact->tval,
				drop->artifact->sval), level, RANDOMISE);
			i_ptr->artifact = drop->artifact;
			copy_artifact_data(i_ptr, i_ptr->artifact);
			i_ptr->artifact->created = 1;
		} else {
			object_prep(i_ptr, drop->kind, level, RANDOMISE);
			apply_magic(i_ptr, level, TRUE, good, great);
		}

		i_ptr->origin = origin;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;
		i_ptr->number = randint0(drop->max - drop->min) + drop->min;
		if (monster_carry(m_ptr, i_ptr))
			any = TRUE;
	}

	/* Make some objects */
	for (j = 0; j < number; j++) {
		i_ptr = &object_type_body;
		object_wipe(i_ptr);

		if (gold_ok && (!item_ok || (randint0(100) < 50))) {
			make_gold(i_ptr, level, SV_GOLD_ANY);
		} else {
			if (!make_object(cave, i_ptr, level, good, great, NULL, 0)) continue;
		}

		i_ptr->origin = origin;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;
		if (monster_carry(m_ptr, i_ptr))
			any = TRUE;
	}

	return any;
}


/**
 * Attempts to place a copy of the given monster at the given position in
 * the dungeon.
 *
 * All of the monster placement routines eventually call this function. This
 * is what actually puts the monster in the dungeon (i.e., it notifies the cave
 * and sets the monsters position). The dungeon loading code also calls this
 * function directly.
 *
 * `origin` is the item origin to use for any monster drops (e.g. ORIGIN_DROP,
 * ORIGIN_DROP_PIT, etc.) The dungeon loading code calls this with origin = 0,
 * which prevents the monster's drops from being generated again.
 *
 * Returns the m_idx of the newly copied monster, or 0 if the placement fails.
 */
s16b place_monster(int y, int x, monster_type *mon, byte origin)
{
	s16b m_idx;
	monster_type *m_ptr;

	assert(in_bounds(y, x));
	assert(!cave_monster_at(cave, y, x));

	/* Get a new record */
	m_idx = mon_pop();
	if (!m_idx) return 0;

	/* Copy the monster */
	m_ptr = cave_monster(cave, m_idx);
	COPY(m_ptr, mon, monster_type);

	/* Set the ID */
	m_ptr->midx = m_idx;

	/* Set the location */
	cave->m_idx[y][x] = m_ptr->midx;
	m_ptr->fy = y;
	m_ptr->fx = x;
	assert(cave_monster_at(cave, y, x) == m_ptr);

	update_mon(m_ptr, TRUE);

	/* Hack -- Count the number of "reproducers" */
	if (rf_has(m_ptr->race->flags, RF_MULTIPLY)) num_repro++;

	/* Count racial occurrences */
	m_ptr->race->cur_num++;

	/* Create the monster's drop, if any */
	if (origin)
		(void)mon_create_drop(m_ptr, origin);

	/* Make mimics start mimicking */
	if (origin && m_ptr->race->mimic_kinds) {
		object_type *i_ptr;
		object_type object_type_body;
		object_kind *kind = m_ptr->race->mimic_kinds->kind;
		struct monster_mimic *mimic_kind;
		int i = 1;
		
		/* Pick a random object kind to mimic */
		for (mimic_kind = m_ptr->race->mimic_kinds; mimic_kind; 
				mimic_kind = mimic_kind->next, i++) {
			if (one_in_(i)) kind = mimic_kind->kind;
		}

		i_ptr = &object_type_body;

		if (kind->tval == TV_GOLD) {
			make_gold(i_ptr, p_ptr->depth, kind->sval);
		} else {
			object_prep(i_ptr, kind, m_ptr->race->level, RANDOMISE);
			apply_magic(i_ptr, m_ptr->race->level, TRUE, FALSE, FALSE);
			i_ptr->number = 1;
		}

		i_ptr->origin = origin;
		i_ptr->mimicking_m_idx = m_idx;
		m_ptr->mimicked_o_idx = floor_carry(cave, y, x, i_ptr);
	}

	/* Result */
	return m_idx;
}

/**
 * Calculates hp for a monster. This function assumes that the Rand_normal
 * function has limits of +/- 4x std_dev. If that changes, this function
 * will become inaccurate.
 *
 * \param r_ptr is the race of the monster in question.
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
		default:
			return Rand_normal(race->avg_hp, std_dev);
	}
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
static bool place_new_monster_one(int y, int x, monster_race *race, 
		bool sleep, byte origin)
{
	int i;

	struct monster *mon;
	struct monster monster_body;

	assert(in_bounds(y, x));
	assert(race && race->name);

	/* Require empty space */
	if (!cave_empty_bold(y, x)) return (FALSE);

	/* No creation on glyph of warding */
	if (cave->feat[y][x] == FEAT_GLYPH) return (FALSE);

	/* "unique" monsters must be "unique" */
	if (rf_has(race->flags, RF_UNIQUE) && race->cur_num >= race->max_num)
		return (FALSE);

	/* Depth monsters may NOT be created out of depth */
	if (rf_has(race->flags, RF_FORCE_DEPTH) && p_ptr->depth < race->level)
		return (FALSE);

	/* Add to level feeling */
	cave->mon_rating += race->power / 20;

	/* Check out-of-depth-ness */
	if (race->level > p_ptr->depth) {
		if (rf_has(race->flags, RF_UNIQUE)) { /* OOD unique */
			if (OPT(cheat_hear))
				msg("Deep unique (%s).", race->name);
		} else { /* Normal monsters but OOD */
			if (OPT(cheat_hear))
				msg("Deep monster (%s).", race->name);
		}
		/* Boost rating by power per 10 levels OOD */
		cave->mon_rating += (race->level - p_ptr->depth) * race->power / 200;
	}
	/* Note uniques for cheaters */
	else if (rf_has(race->flags, RF_UNIQUE) && OPT(cheat_hear))
		msg("Unique (%s).", race->name);

	/* Get local monster */
	mon = &monster_body;

	/* Clean out the monster */
	(void)WIPE(mon, monster_type);

	/* Save the race */
	mon->race = race;
	mon->r_idx = race->ridx;

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
		i = extract_energy[race->speed] / 10;
		if (i) mon->mspeed += rand_spread(0, i);
	}

	/* Give a random starting energy */
	mon->energy = (byte)randint0(50);

	/* Force monster to wait for player */
	if (rf_has(race->flags, RF_FORCE_SLEEP))
		mon->mflag |= (MFLAG_NICE);

	/* Radiate light? */
	if (rf_has(race->flags, RF_HAS_LIGHT))
		p_ptr->update |= PU_UPDATE_VIEW;
	
	/* Is this obviously a monster? (Mimics etc. aren't) */
	if (rf_has(race->flags, RF_UNAWARE)) 
		mon->unaware = TRUE;
	else
		mon->unaware = FALSE;

	/* Set the color if necessary */
	if (rf_has(race->flags, RF_ATTR_RAND))
		mon->attr = randint1(BASIC_COLORS - 1);

	/* Place the monster in the dungeon */
	if (!place_monster(y, x, mon, origin))
		return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	25

/**
 * Picks a monster group size. Used for monsters with the FRIENDS
 * flag and monsters with the ESCORT/ESCORTS flags.
 */
static int group_size_1(const monster_race *race)
{
	int total;

	assert(race && race->name);
	
	/* Pick a group size */
	total = randint1(13);

	/* Hard monsters, small groups */
	if (race->level > p_ptr->depth)
		total += 0 - randint1(race->level - p_ptr->depth);

	/* Easy monsters, large groups */
	else if (race->level < p_ptr->depth)
		total += randint1(p_ptr->depth - race->level);

	if (total < 1) total = 1;
	if (total > GROUP_MAX) total = GROUP_MAX;

	return total;
}
		
/**
 * Picks a monster group size. Used for monsters with the FRIEND
 * flag.
 */
static int group_size_2(const monster_race *race)
{
	/* Start small */
	int total = 1;

	assert(race && race->name);

	/* Easy monsters, large groups */
	if (race->level < p_ptr->depth)
		total += randint1(2 * (p_ptr->depth - race->level));

	if (total > GROUP_MAX) total = GROUP_MAX;

	return total;
}

		
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
static bool place_new_monster_group(struct cave *c, int y, int x, 
		monster_race *r_ptr, bool sleep, int total, byte origin)
{
	int n, i;

	int hack_n;

	/* x and y coordinates of the placed monsters */
	byte hack_y[GROUP_MAX];
	byte hack_x[GROUP_MAX];

	assert(r_ptr);
	
	/* Start on the monster */
	hack_n = 1;
	hack_x[0] = x;
	hack_y[0] = y;

	/* Puddle monsters, breadth first, up to total */
	for (n = 0; (n < hack_n) && (hack_n < total); n++) {
		/* Grab the location */
		int hx = hack_x[n];
		int hy = hack_y[n];

		/* Check each direction, up to total */
		for (i = 0; (i < 8) && (hack_n < total); i++) {
			int mx = hx + ddx_ddd[i];
			int my = hy + ddy_ddd[i];

			/* Walls and Monsters block flow */
			if (!cave_empty_bold(my, mx)) continue;

			/* Attempt to place another monster */
			if (place_new_monster_one(my, mx, r_ptr, sleep, origin)) {
				/* Add it to the "hack" set */
				hack_y[hack_n] = my;
				hack_x[hack_n] = mx;
				hack_n++;
			}
		}
	}

	/* Success */
	return (TRUE);
}


/*
 * Hack -- help pick an escort type
 */
static monster_race *place_monster_race = NULL;

/**
 * Hack -- helps pick an escort type. Requires place_monster_race to be set.
 * Returns TRUE if monster race `r_idx` is appropriate as an escort for
 * the monster of race place_monster_race.
 */
static bool place_monster_okay(monster_race *race)
{
	assert(place_monster_race);	
	assert(race);

	/* Require identical monster template */
	if (race->base != place_monster_race->base) return (FALSE);

	/* Skip more advanced monsters */
	if (race->level > place_monster_race->level) return (FALSE);

	/* Skip unique monsters */
	if (rf_has(race->flags, RF_UNIQUE)) return (FALSE);

	/* Paranoia -- Skip identical monsters */
	if (place_monster_race == race) return (FALSE);

	/* Okay */
	return (TRUE);
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
 *
 * Note the "bizarre" use of non-recursion to prevent annoying output
 * when running a code profiler.
 *
 * Note the use of the "monster allocation table" to restrict
 * the "get_mon_num()" function to "legal" escort types.
 */
bool place_new_monster(struct cave *c, int y, int x, monster_race *race, bool sleep,
	bool group_okay, byte origin)
{
	int i;

	assert(c);
	assert(race);
	
	/* Place one monster, or fail */
	if (!place_new_monster_one(y, x, race, sleep, origin)) return (FALSE);

	/* We're done unless the group flag is set */
	if (!group_okay) return (TRUE);

	/* Friends for certain monsters */
	if (rf_has(race->flags, RF_FRIEND)) {
		int total = group_size_2(race);
		(void)place_new_monster_group(c, y, x, race, sleep, total, origin);
	}

	/* Friends for certain monsters */
	if (rf_has(race->flags, RF_FRIENDS)) {
		int total = group_size_1(race);
		(void)place_new_monster_group(c, y, x, race, sleep, total, origin);
	}

	/* Escorts for certain monsters */
	if (rf_has(race->flags, RF_ESCORT)) {
		/* Try to place several "escorts" */
		for (i = 0; i < 50; i++) {
			int nx, ny, d = 3;
			monster_race *race2;

			/* Pick a location */
			scatter(&ny, &nx, y, x, d, 0);

			/* Require empty grids */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Set the escort index */
			place_monster_race = race;

			/* Prepare allocation table */
			get_mon_num_prep(place_monster_okay);

			/* Pick a random race */
			race2 = get_mon_num(race->level);

			/* Prepare allocation table */
			get_mon_num_prep(NULL);

			/* Handle failure */
			if (!race2) break;

			/* Place a single escort */
			(void)place_new_monster_one(ny, nx, race2, sleep, origin);

			/* Place a "group" of escorts if needed */
			if (rf_has(race2->flags, RF_FRIEND)) {
				int total = group_size_2(race2);
				(void)place_new_monster_group(c, ny, nx, race2, sleep, total, origin);
			}
			
			if (rf_has(race2->flags, RF_FRIENDS) || rf_has(race->flags, RF_ESCORTS)) {
				int total = group_size_1(race2);
				(void)place_new_monster_group(c, ny, nx, race2, sleep, total, origin);
			}
		}
	}

	/* Success */
	return (TRUE);
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
 * Returns TRUE if we successfully place a monster.
 */
bool pick_and_place_monster(struct cave *c, int y, int x, int depth, bool sleep,
	bool group_okay, byte origin)
{
	/* Pick a monster race */
	monster_race *race = get_mon_num(depth);
	if (!race) return (FALSE);

	/* Attempt to place the monster */
	return (place_new_monster(c, y, x, race, sleep, group_okay, origin));
}


/**
 * Picks a monster race, makes a new monster of that race, then attempts to 
 * place it in the dungeon at least `dis` away from the player. The monster 
 * race chosen will be appropriate for dungeon level equal to `depth`.
 *
 * If `sleep` is true, the monster is placed with its default sleep value,
 * which is given in monster.txt.
 *
 * Returns TRUE if we successfully place a monster.
 */
bool pick_and_place_distant_monster(struct cave *c, struct loc loc, int dis,
		bool sleep, int depth)
{
	int py = loc.y;
	int px = loc.x;

	int y = 0, x = 0;
	int	attempts_left = 10000;

	assert(c);

	/* Find a legal, distant, unoccupied, space */
	while (--attempts_left) {
		/* Pick a location */
		y = randint0(c->height);
		x = randint0(c->width);

		/* Require "naked" floor grid */
		if (!cave_isempty(c, y, x)) continue;

		/* Accept far away grids */
		if (distance(y, x, py, px) > dis) break;
	}

	if (!attempts_left) {
		if (OPT(cheat_xtra) || OPT(cheat_hear))
			msg("Warning! Could not allocate a new monster.");

		return FALSE;
	}

	/* Attempt to place the monster, allow groups */
	if (pick_and_place_monster(c, y, x, depth, sleep, TRUE, ORIGIN_DROP))
		return (TRUE);

	/* Nope */
	return (FALSE);
}

/**
 * Creates magical stairs after finishing a quest monster.
 */
static void build_quest_stairs(int y, int x)
{
	int ny, nx;

	/* Stagger around */
	while (!cave_valid_bold(y, x)) {
		int d = 1;

		/* Pick a location */
		scatter(&ny, &nx, y, x, d, 0);

		/* Stagger */
		y = ny; x = nx;
	}

	/* Destroy any objects */
	delete_object(y, x);

	/* Explain the staircase */
	msg("A magical staircase appears...");

	/* Create stairs down */
	cave_set_feat(cave, y, x, FEAT_MORE);

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
}


/**
 * Handles the "death" of a monster.
 *
 * Disperses treasures carried by the monster centered at the monster location.
 * Note that objects dropped may disappear in crowded rooms.
 *
 * Checks for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * If `stats` is true, then we skip updating the monster memory. This is
 * used by stats-generation code, for efficiency.
 */
void monster_death(struct monster *m_ptr, bool stats)
{
	int i;
	int dump_item = 0;
	int dump_gold = 0;
	int total = 0;
	s16b this_o_idx, next_o_idx = 0;

	object_type *i_ptr;
	object_type object_type_body;

	bool visible = (m_ptr->ml || rf_has(m_ptr->race->flags, RF_UNIQUE));

	int y = m_ptr->fy;
	int x = m_ptr->fx;

	/* Delete any mimicked objects */
	if (m_ptr->mimicked_o_idx > 0)
		delete_object_idx(m_ptr->mimicked_o_idx);

	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Line up the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object, copy it and delete the original */
		i_ptr = &object_type_body;
		object_copy(i_ptr, o_ptr);
		delete_object_idx(this_o_idx);

		/* Count it and drop it - refactor once origin is a bitflag */
		if (!stats) {
			if ((i_ptr->tval == TV_GOLD) && (i_ptr->origin != ORIGIN_STOLEN))
				dump_gold++;
			else if ((i_ptr->tval != TV_GOLD) && ((i_ptr->origin == ORIGIN_DROP)
					|| (i_ptr->origin == ORIGIN_DROP_PIT)
					|| (i_ptr->origin == ORIGIN_DROP_VAULT)
					|| (i_ptr->origin == ORIGIN_DROP_SUMMON)
					|| (i_ptr->origin == ORIGIN_DROP_SPECIAL)
					|| (i_ptr->origin == ORIGIN_DROP_BREED)
					|| (i_ptr->origin == ORIGIN_DROP_POLY)
					|| (i_ptr->origin == ORIGIN_DROP_WIZARD)))
				dump_item++;
		}

		/* Change origin if monster is invisible, unless we're in stats mode */
		if (!visible && !stats)
			i_ptr->origin = ORIGIN_DROP_UNKNOWN;

		drop_near(cave, i_ptr, 0, y, x, TRUE);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;

	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
		lore_treasure(m_ptr, dump_item, dump_gold);

	/* Update monster list window */
	p_ptr->redraw |= PR_MONLIST;

	/* Nothing else to do for non-"Quest Monsters" */
	if (!rf_has(m_ptr->race->flags, RF_QUESTOR)) return;

	/* Mark quests as complete */
	for (i = 0; i < MAX_Q_IDX; i++)	{
		/* Note completed quests */
		if (q_list[i].level == m_ptr->race->level) q_list[i].level = 0;

		/* Count incomplete quests */
		if (q_list[i].level) total++;
	}

	/* Build magical stairs */
	build_quest_stairs(y, x);

	/* Nothing left, game over... */
	if (total == 0) {
		p_ptr->total_winner = TRUE;
		p_ptr->redraw |= (PR_TITLE);
		msg("*** CONGRATULATIONS ***");
		msg("You have won the game!");
		msg("You may retire (commit suicide) when you are ready.");
	}
}


/**
 * Decreases a monster's hit points by `dam` and handle monster death.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * We announce monster death (using an optional "death message" (`note`)
 * if given, and a otherwise a generic killed/destroyed message).
 * 
 * Returns TRUE if the monster has been killed (and deleted).
 *
 * TODO: Consider decreasing monster experience over time, say, by using
 * "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))" instead
 * of simply "(m_exp * m_lev) / (p_lev)", to make the first monster
 * worth more than subsequent monsters.  This would also need to
 * induce changes in the monster recall code.  XXX XXX XXX
 **/
bool mon_take_hit(struct monster *m_ptr, int dam, bool *fear, const char *note)
{
	s32b div, new_exp, new_exp_frac;
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];


	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_ptr) p_ptr->redraw |= (PR_HEALTH);

	/* Wake it up */
	mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);

	/* Become aware of its presence */
	if (m_ptr->unaware)
		become_aware(m_ptr);

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0) {
		char m_name[80];
		char buf[80];

		/* Assume normal death sound */
		int soundfx = MSG_KILL;

		/* Play a special sound if the monster was unique */
		if (rf_has(m_ptr->race->flags, RF_UNIQUE)) {
			if (m_ptr->race->base == lookup_monster_base("Morgoth"))
				soundfx = MSG_KILL_KING;
			else
				soundfx = MSG_KILL_UNIQUE;
		}

		/* Extract monster name */
		monster_desc(m_name, sizeof(m_name), m_ptr, 0);

		/* Death by Missile/Spell attack */
		if (note) {
			/* Hack -- allow message suppression */
			if (strlen(note) <= 1) {
				/* Be silent */
			} else {
				char *str = format("%s%s", m_name, note);
				my_strcap(str);
				msgt(soundfx, "%s", str);
			}
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
			msgt(soundfx, "You have killed %s.", m_name);

		/* Death by Physical attack -- non-living monster */
		else if (monster_is_unusual(m_ptr->race))
			msgt(soundfx, "You have destroyed %s.", m_name);

		/* Death by Physical attack -- living monster */
		else
			msgt(soundfx, "You have slain %s.", m_name);

		/* Player level */
		div = p_ptr->lev;

		/* Give some experience for the kill */
		new_exp = ((long)m_ptr->race->mexp * m_ptr->race->level) / div;

		/* Handle fractional experience */
		new_exp_frac = ((((long)m_ptr->race->mexp * m_ptr->race->level) % div)
		                * 0x10000L / div) + p_ptr->exp_frac;

		/* Keep track of experience */
		if (new_exp_frac >= 0x10000L) {
			new_exp++;
			p_ptr->exp_frac = (u16b)(new_exp_frac - 0x10000L);
		}
		else
			p_ptr->exp_frac = (u16b)new_exp_frac;

		/* When the player kills a Unique, it stays dead */
		if (rf_has(m_ptr->race->flags, RF_UNIQUE)) {
			char unique_name[80];
			m_ptr->race->max_num = 0;

			/* 
			 * This gets the correct name if we slay an invisible 
			 * unique and don't have See Invisible.
			 */
			monster_desc(unique_name, sizeof(unique_name), m_ptr, 
					MDESC_SHOW | MDESC_IND2);

			/* Log the slaying of a unique */
			strnfmt(buf, sizeof(buf), "Killed %s", unique_name);
			history_add(buf, HISTORY_SLAY_UNIQUE, 0);
		}

		/* Gain experience */
		player_exp_gain(p_ptr, new_exp);

		/* Generate treasure */
		monster_death(m_ptr, FALSE);

		/* Recall even invisible uniques or winners */
		if (m_ptr->ml || rf_has(m_ptr->race->flags, RF_UNIQUE)) {
			/* Count kills this life */
			if (l_ptr->pkills < MAX_SHORT) l_ptr->pkills++;

			/* Count kills in all lives */
			if (l_ptr->tkills < MAX_SHORT) l_ptr->tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(m_ptr->r_idx);
		}

		/* Delete the monster */
		delete_monster_idx(m_ptr->midx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}


	/* Mega-Hack -- Pain cancels fear */
	if (!(*fear) && m_ptr->m_timed[MON_TMD_FEAR] && (dam > 0)) {
		int tmp = randint1(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->m_timed[MON_TMD_FEAR]) {
			/* Reduce fear */
			mon_dec_timed(m_ptr, MON_TMD_FEAR, tmp, MON_TMD_FLG_NOMESSAGE,
				FALSE);
		}

		/* Cure all the fear */
		else {
			/* Cure fear */
			mon_clear_timed(m_ptr, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE, FALSE);

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->m_timed[MON_TMD_FEAR] && !rf_has(m_ptr->race->flags, RF_NO_FEAR) &&
		dam > 0) {
		int percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		 * Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points
		 */
		if ((randint1(10) >= percentage) ||
		    ((dam >= m_ptr->hp) && (randint0(100) < 80)))
		{
			int timer = randint1(10) + (((dam >= m_ptr->hp) && (percentage > 7)) ?
	                   20 : ((11 - percentage) * 5));

			/* Hack -- note fear */
			(*fear) = TRUE;

			mon_inc_timed(m_ptr, MON_TMD_FEAR, timer,
					MON_TMD_FLG_NOMESSAGE | MON_TMD_FLG_NOFAIL, FALSE);
		}
	}


	/* Not dead yet */
	return (FALSE);
}

struct init_module mon_make_module = {
	.name = "monster/mon-make",
	.init = init_race_allocs,
	.cleanup = cleanup_race_allocs
};
