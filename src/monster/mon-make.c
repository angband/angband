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
#include "target.h"
#include "monster/mon-lore.h"
#include "monster/mon-make.h"
#include "monster/mon-util.h"
#include "object/tvalsval.h"

/*
 * Delete a monster by index.
 *
 * When a monster is deleted, all of its objects are deleted.
 */
void delete_monster_idx(int i)
{
	int x, y;

	monster_type *m_ptr = cave_monster(cave, i);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	s16b this_o_idx, next_o_idx = 0;

	/* Get location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Hack -- Reduce the racial counter */
	r_ptr->cur_num--;

	/* Hack -- count the number of "reproducers" */
	if (rf_has(r_ptr->flags, RF_MULTIPLY)) num_repro--;

	/* Hack -- remove target monster */
	if (target_get_monster() == i) target_set_monster(0);

	/* Hack -- remove tracked monster */
	if (p_ptr->health_who == i) health_track(p_ptr, 0);

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


/*
 * Delete the monster, if any, at a given location
 */
void delete_monster(int y, int x)
{
	/* Paranoia */
	if (!in_bounds(y, x)) return;

	/* Delete the monster (if any) */
	if (cave->m_idx[y][x] > 0) delete_monster_idx(cave->m_idx[y][x]);
}


/*
 * Move an object from index i1 to index i2 in the object list
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

	/* Location */
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
	if (target_get_monster() == i1) target_set_monster(i2);

	/* Hack -- Update the health bar */
	if (p_ptr->health_who == i1) p_ptr->health_who = i2;

	/* Hack -- move monster */
	COPY(cave_monster(cave, i2), cave_monster(cave, i1), struct monster);

	/* Hack -- wipe hole */
	(void)WIPE(cave_monster(cave, i1), monster_type);
}


/*
 * Compact and Reorder the monster list
 *
 * This function can be very dangerous, use with caution!
 *
 * When actually "compacting" monsters, we base the saving throw
 * on a combination of monster level, distance from player, and
 * current "desperation".
 *
 * After "compacting" (if needed), we "reorder" the monsters into a more
 * compact order, and we reset the allocation info, and the "live" array.
 */
void compact_monsters(int size)
{
	int i, num, cnt;

	int cur_lev, cur_dis, chance;


	/* Message (only if compacting) */
	if (size) msg("Compacting monsters...");


	/* Compact at least 'size' objects */
	for (num = 0, cnt = 1; num < size; cnt++)
	{
		/* Get more vicious each iteration */
		cur_lev = 5 * cnt;

		/* Get closer each iteration */
		cur_dis = 5 * (20 - cnt);

		/* Check all the monsters */
		for (i = 1; i < cave_monster_max(cave); i++)
		{
			monster_type *m_ptr = cave_monster(cave, i);

			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Paranoia -- skip "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Hack -- High level monsters start out "immune" */
			if (r_ptr->level > cur_lev) continue;

			/* Ignore nearby monsters */
			if ((cur_dis > 0) && (m_ptr->cdis < cur_dis)) continue;

			/* Saving throw chance */
			chance = 90;

			/* Only compact "Quest" Monsters in emergencies */
			if (rf_has(r_ptr->flags, RF_QUESTOR) && (cnt < 1000)) chance = 100;

			/* Try not to compact Unique Monsters */
			if (rf_has(r_ptr->flags, RF_UNIQUE)) chance = 99;

			/* All monsters get a saving throw */
			if (randint0(100) < chance) continue;

			/* Delete the monster */
			delete_monster_idx(i);

			/* Count the monster */
			num++;
		}
	}


	/* Excise dead monsters (backwards!) */
	for (i = cave_monster_max(cave) - 1; i >= 1; i--)
	{
		/* Get the i'th monster */
		monster_type *m_ptr = cave_monster(cave, i);

		/* Skip real monsters */
		if (m_ptr->r_idx) continue;

		/* Move last monster into open hole */
		compact_monsters_aux(cave_monster_max(cave) - 1, i);

		/* Compress "cave->mon_max" */
		cave->mon_max--;
	}
}


/*
 * Delete/Remove all the monsters when the player leaves the level
 *
 * This is an efficient method of simulating multiple calls to the
 * "delete_monster()" function, with no visual effects.
 */
void wipe_mon_list(struct cave *c, struct player *p)
{
	int i;

	/* Delete all the monsters */
	for (i = cave_monster_max(cave) - 1; i >= 1; i--)
	{
		monster_type *m_ptr = cave_monster(cave, i);

		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Reduce the racial counter */
		r_ptr->cur_num--;

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

/*
 * Get and return the index of a "free" monster.
 *
 * This routine should almost never fail, but it *can* happen.
 */
static s16b mon_pop(void)
{
	int i;


	/* Normal allocation */
	if (cave_monster_max(cave) < z_info->m_max)
	{
		/* Get the next hole */
		i = cave_monster_max(cave);

		/* Expand the array */
		cave->mon_max++;

		/* Count monsters */
		cave->mon_cnt++;

		/* Return the index */
		return (i);
	}


	/* Recycle dead monsters */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		monster_type *m_ptr;

		/* Get the monster */
		m_ptr = cave_monster(cave, i);

		/* Skip live monsters */
		if (m_ptr->r_idx) continue;

		/* Count monsters */
		cave->mon_cnt++;

		/* Use this monster */
		return (i);
	}


	/* Warn the player (except during dungeon creation) */
	if (character_dungeon) msg("Too many monsters!");

	/* Try not to crash */
	return (0);
}


/*
 * Apply a "monster restriction function" to the "monster allocation table"
 */
void get_mon_num_prep(void)
{
	int i;

	/* Scan the allocation table */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Get the entry */
		alloc_entry *entry = &alloc_race_table[i];

		/* Accept monsters which pass the restriction, if any */
		if (!get_mon_num_hook || (*get_mon_num_hook)(entry->index))
		{
			/* Accept this monster */
			entry->prob2 = entry->prob1;
		}

		/* Do not use this monster */
		else
		{
			/* Decline this monster */
			entry->prob2 = 0;
		}
	}

	/* Success */
	return;
}



/*
 * Choose a monster race that seems "appropriate" to the given level
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
s16b get_mon_num(int level)
{
	int i, j, p;

	int r_idx;

	long value, total;

	monster_race *r_ptr;

	alloc_entry *table = alloc_race_table;

	/* Occasionally produce a nastier monster in the dungeon */
	if (level > 0 && one_in_(NASTY_MON))
		level += MIN(level / 4 + 2, MON_OOD_MAX);

	/* Reset total */
	total = 0L;

	/* Process probabilities */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Monsters are sorted by depth */
		if (table[i].level > level) break;

		/* Default */
		table[i].prob3 = 0;

		/* Hack -- No town monsters in dungeon */
		if ((level > 0) && (table[i].level <= 0)) continue;

		/* Get the "r_idx" of the chosen monster */
		r_idx = table[i].index;

		/* Get the actual race */
		r_ptr = &r_info[r_idx];

		/* Hack -- "unique" monsters must be "unique" */
		if (rf_has(r_ptr->flags, RF_UNIQUE) &&
		    r_ptr->cur_num >= r_ptr->max_num)
		{
			continue;
		}

		/* Depth Monsters never appear out of depth */
		if (rf_has(r_ptr->flags, RF_FORCE_DEPTH) && r_ptr->level > p_ptr->depth)
		{
			continue;
		}

		/* Accept */
		table[i].prob3 = table[i].prob2;

		/* Total */
		total += table[i].prob3;
	}

	/* No legal monsters */
	if (total <= 0) return (0);


	/* Pick a monster */
	value = randint0(total);

	/* Find the monster */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Found the entry */
		if (value < table[i].prob3) break;

		/* Decrement */
		value = value - table[i].prob3;
	}


	/* Power boost */
	p = randint0(100);

	/* Try for a "harder" monster once (50%) or twice (10%) */
	if (p < 60)
	{
		/* Save old */
		j = i;

		/* Pick a monster */
		value = randint0(total);

		/* Find the monster */
		for (i = 0; i < alloc_race_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}

	/* Try for a "harder" monster twice (10%) */
	if (p < 10)
	{
		/* Save old */
		j = i;

		/* Pick a monster */
		value = randint0(total);

		/* Find the monster */
		for (i = 0; i < alloc_race_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}


	/* Result */
	return (table[i].index);
}


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
 * Return the coin type of a monster race, based on the monster being
 * killed.
 */
static int get_coin_type(const monster_race *r_ptr)
{
	const char *name = r_ptr->name;

	if (!rf_has(r_ptr->flags, RF_METAL)) return SV_GOLD_ANY;

	/* Look for textual clues */
	if (my_stristr(name, "copper "))	return SV_COPPER;
	if (my_stristr(name, "silver "))	return SV_SILVER;
	if (my_stristr(name, "gold "))		return SV_GOLD;
	if (my_stristr(name, "mithril "))	return SV_MITHRIL;
	if (my_stristr(name, "adamantite "))	return SV_ADAMANTITE;

	/* Assume nothing */
	return SV_GOLD_ANY;
}

/**
 * Create a specific monster's drop, including any specified drops.
 *
 * Returns TRUE if anything is created, FALSE if nothing is.
 */
static bool mon_create_drop(int m_idx, byte origin)
{
	struct monster_drop *drop;

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool great = (rf_has(r_ptr->flags, RF_DROP_GREAT)) ? TRUE : FALSE;
	bool good = (rf_has(r_ptr->flags, RF_DROP_GOOD) ? TRUE : FALSE) || great;
	bool any = FALSE;
	bool gold_ok = (!rf_has(r_ptr->flags, RF_ONLY_ITEM));
	bool item_ok = (!rf_has(r_ptr->flags, RF_ONLY_GOLD));

	int number = 0, level, j;
	int force_coin = get_coin_type(r_ptr);

	object_type *i_ptr;
	object_type object_type_body;

	/* Determine how much we can drop */
	if (rf_has(r_ptr->flags, RF_DROP_20) && randint0(100) < 20) number++;
	if (rf_has(r_ptr->flags, RF_DROP_40) && randint0(100) < 40) number++;
	if (rf_has(r_ptr->flags, RF_DROP_60) && randint0(100) < 60) number++;
	if (rf_has(r_ptr->flags, RF_DROP_4)) number += rand_range(2, 6);
	if (rf_has(r_ptr->flags, RF_DROP_3)) number += rand_range(2, 4);
	if (rf_has(r_ptr->flags, RF_DROP_2)) number += rand_range(1, 3);
	if (rf_has(r_ptr->flags, RF_DROP_1)) number++;

	/* Take the best of average of monster level and current depth,
	   and monster level - to reward fighting OOD monsters */
	level = MAX((r_ptr->level + p_ptr->depth) / 2, r_ptr->level);

	/* Specified drops */
	for (drop = r_ptr->drops; drop; drop = drop->next) {
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
			make_gold(i_ptr, level, force_coin);
		} else {
			if (!make_object(cave, i_ptr, level, good, great, NULL)) continue;
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
 * Place a copy of a monster in the dungeon XXX XXX
 */
s16b place_monster(int y, int x, monster_type *n_ptr, byte origin)
{
	s16b m_idx;

	monster_type *m_ptr;
	monster_race *r_ptr;

	/* Paranoia XXX XXX */
	if (cave->m_idx[y][x] != 0) return 0;

	/* Get a new record */
	m_idx = mon_pop();

	if (!m_idx) return 0;
	n_ptr->midx = m_idx;

	/* Make a new monster */
	cave->m_idx[y][x] = m_idx;

	/* Get the new monster */
	m_ptr = cave_monster(cave, m_idx);

	/* Copy the monster XXX */
	COPY(m_ptr, n_ptr, monster_type);

	/* Location */
	m_ptr->fy = y;
	m_ptr->fx = x;

	/* Update the monster */
	update_mon(m_idx, TRUE);

	/* Get the new race */
	r_ptr = &r_info[m_ptr->r_idx];

	/* Hack -- Count the number of "reproducers" */
	if (rf_has(r_ptr->flags, RF_MULTIPLY)) num_repro++;

	/* Count racial occurances */
	r_ptr->cur_num++;

	/* Create the monster's drop, if any */
	if (origin)
		(void)mon_create_drop(m_idx, origin);

	/* Make mimics start mimicking */
	if (origin && r_ptr->mimic_kinds) {
		object_type *i_ptr;
		object_type object_type_body;
		object_kind *kind = r_ptr->mimic_kinds->kind;
		struct monster_mimic *mimic_kind;
		int i = 1;
		
		/* Pick a random object kind to mimic */
		for (mimic_kind = r_ptr->mimic_kinds; mimic_kind; mimic_kind = mimic_kind->next, i++) {
			if (one_in_(i)) kind = mimic_kind->kind;
		}

		i_ptr = &object_type_body;

		if (kind->tval == TV_GOLD) {
			make_gold(i_ptr, p_ptr->depth, kind->sval);
		} else {
			object_prep(i_ptr, kind, r_ptr->level, RANDOMISE);
			apply_magic(i_ptr, r_ptr->level, TRUE, FALSE, FALSE);
			i_ptr->number = 1;
		}

		i_ptr->mimicking_m_idx = m_idx;
		m_ptr->mimicked_o_idx = floor_carry(cave, y, x, i_ptr);
	}

	/* Result */
	return m_idx;
}

/**
 * Calculate hp for a monster. This function assumes that the Rand_normal
 * function has limits of +/- 4x std_dev. If that changes, this function
 * will become inaccurate.
 *
 * \param r_ptr is the race of the monster in question.
 * \param hp_aspect is the hp calc we want (min, max, avg, random).
 */
int mon_hp(const struct monster_race *r_ptr, aspect hp_aspect)
{
		int std_dev = (((r_ptr->avg_hp * 10) / 8) + 5) / 10;

		if (r_ptr->avg_hp > 1) std_dev++;

		switch (hp_aspect) {
			case MINIMISE:
				return (r_ptr->avg_hp - (4 * std_dev));
			case MAXIMISE:
			case EXTREMIFY:
				return (r_ptr->avg_hp + (4 * std_dev));
			case AVERAGE:
				return r_ptr->avg_hp;
			default:
				return Rand_normal(r_ptr->avg_hp, std_dev);
		}
}


/*
 * Attempt to place a monster of the given race at the given location.
 *
 * To give the player a sporting chance, any monster that appears in
 * line-of-sight and is extremely dangerous can be marked as
 * "FORCE_SLEEP", which will cause them to be placed with low energy,
 * which often (but not always) lets the player move before they do.
 *
 * This routine refuses to place out-of-depth "FORCE_DEPTH" monsters.
 *
 * XXX XXX XXX Use special "here" and "dead" flags for unique monsters,
 * remove old "cur_num" and "max_num" fields.
 *
 * XXX XXX XXX Actually, do something similar for artifacts, to simplify
 * the "preserve" mode, and to make the "what artifacts" flag more useful.
 *
 * This is the only function which may place a monster in the dungeon,
 * except for the savefile loading code.
 */
static bool place_new_monster_one(int y, int x, int r_idx, bool slp, byte origin)
{
	int i;

	monster_race *r_ptr;
	monster_type *n_ptr;
	monster_type monster_type_body;

	const char *name;

	/* Paranoia */
	if (!in_bounds(y, x)) return (FALSE);

	/* Require empty space */
	if (!cave_empty_bold(y, x)) return (FALSE);

	/* No creation on glyph of warding */
	if (cave->feat[y][x] == FEAT_GLYPH) return (FALSE);

	/* Paranoia */
	if (!r_idx) return (FALSE);

	/* Race */
	r_ptr = &r_info[r_idx];

	/* Paranoia */
	if (!r_ptr->name) return (FALSE);

	name = r_ptr->name;

	/* "unique" monsters must be "unique" */
	if (rf_has(r_ptr->flags, RF_UNIQUE) && r_ptr->cur_num >= r_ptr->max_num)
		return (FALSE);

	/* Depth monsters may NOT be created out of depth */
	if (rf_has(r_ptr->flags, RF_FORCE_DEPTH) && p_ptr->depth < r_ptr->level)
		return (FALSE);

	/* Add to level feeling */
	cave->mon_rating += r_ptr->power / 20;

	/* Check out-of-depth-ness */
	if (r_ptr->level > p_ptr->depth) {
		if (rf_has(r_ptr->flags, RF_UNIQUE)) { /* OOD unique */
			if (OPT(cheat_hear))
				msg("Deep Unique (%s).", name);
		} else { /* Normal monsters but OOD */
			if (OPT(cheat_hear))
				msg("Deep Monster (%s).", name);
		}
		/* Boost rating by power per 10 levels OOD */
		cave->mon_rating += (r_ptr->level - p_ptr->depth) * r_ptr->power / 200;
	}
	/* Note uniques for cheaters */
	else if (rf_has(r_ptr->flags, RF_UNIQUE) && OPT(cheat_hear))
		msg("Unique (%s).", name);

	/* Get local monster */
	n_ptr = &monster_type_body;

	/* Clean out the monster */
	(void)WIPE(n_ptr, monster_type);

	/* Save the race */
	n_ptr->r_idx = r_idx;

	/* Enforce sleeping if needed */
	if (slp && r_ptr->sleep) {
		int val = r_ptr->sleep;
		n_ptr->m_timed[MON_TMD_SLEEP] = ((val * 2) + randint1(val * 10));
	}

	/* Uniques get a fixed amount of HP */
	if (rf_has(r_ptr->flags, RF_UNIQUE))
		n_ptr->maxhp = r_ptr->avg_hp;
	else {
		n_ptr->maxhp = mon_hp(r_ptr, RANDOMISE);
		n_ptr->maxhp = MAX(n_ptr->maxhp, 1);
	}

	/* And start out fully healthy */
	n_ptr->hp = n_ptr->maxhp;

	/* Extract the monster base speed */
	n_ptr->mspeed = r_ptr->speed;

	/* Hack -- small racial variety */
	if (!rf_has(r_ptr->flags, RF_UNIQUE)) {
		/* Allow some small variation per monster */
		i = extract_energy[r_ptr->speed] / 10;
		if (i) n_ptr->mspeed += rand_spread(0, i);
	}

	/* Give a random starting energy */
	n_ptr->energy = (byte)randint0(50);

	/* Force monster to wait for player */
	if (rf_has(r_ptr->flags, RF_FORCE_SLEEP))
		n_ptr->mflag |= (MFLAG_NICE);

	/* Radiate light? */
	if (rf_has(r_ptr->flags, RF_HAS_LIGHT))
		p_ptr->update |= PU_UPDATE_VIEW;
	
	/* Is this obviously a monster? (Mimics etc. aren't) */
	if (rf_has(r_ptr->flags, RF_UNAWARE)) 
		n_ptr->unaware = TRUE;
	else
		n_ptr->unaware = FALSE;

	/* Set the color if necessary */
	if (rf_has(r_ptr->flags, RF_ATTR_RAND))
		n_ptr->attr = randint1(BASIC_COLORS - 1);

	/* Place the monster in the dungeon */
	if (!place_monster(y, x, n_ptr, origin))
		return (FALSE);

	/* Success */
	return (TRUE);
	
	
}


/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	25

/*
 * Pick a monster group size. Used for monsters with the FRIENDS
 * flag and monsters with the ESCORT/ESCORTS flags.
 */
static int group_size_1(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	int total, extra = 0;

	/* Pick a group size */
	total = randint1(13);

	/* Hard monsters, small groups */
	if (r_ptr->level > p_ptr->depth)
	{
		extra = r_ptr->level - p_ptr->depth;
		extra = 0 - randint1(extra);
	}

	/* Easy monsters, large groups */
	else if (r_ptr->level < p_ptr->depth)
	{
		extra = p_ptr->depth - r_ptr->level;
		extra = randint1(extra);
	}

	/* Modify the group size */
	total += extra;

	/* Minimum size */
	if (total < 1) total = 1;

	/* Maximum size */
	if (total > GROUP_MAX) total = GROUP_MAX;

	return total;
}
		
/*
 * Pick a monster group size. Used for monsters with the FRIEND
 * flag.
 */
static int group_size_2(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	int total, extra = 0;

	/* Start small */
	total = 1;

	/* Easy monsters, large groups */
	if (r_ptr->level < p_ptr->depth)
	{
		extra = 2 * (p_ptr->depth - r_ptr->level);
		extra = randint1(extra);
	}

	/* Modify the group size */
	total += extra;

	/* Maximum size */
	if (total > GROUP_MAX) total = GROUP_MAX;

	return total;
}

		
/*
 * Attempt to place a "group" of monsters around the given location
 */
static bool place_new_monster_group(struct cave *c, int y, int x, int r_idx,
	bool slp, int total, byte origin)
{
	int n, i;

	int hack_n;

	byte hack_y[GROUP_MAX];
	byte hack_x[GROUP_MAX];

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
			if (place_new_monster_one(my, mx, r_idx, slp, origin)) {
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
static int place_monster_idx = 0;

/*
 * Hack -- help pick an escort type
 */
static bool place_monster_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[place_monster_idx];

	monster_race *z_ptr = &r_info[r_idx];

	/* Require similar "race" */
	if (z_ptr->d_char != r_ptr->d_char) return (FALSE);

	/* Skip more advanced monsters */
	if (z_ptr->level > r_ptr->level) return (FALSE);

	/* Skip unique monsters */
	if (rf_has(z_ptr->flags, RF_UNIQUE)) return (FALSE);

	/* Paranoia -- Skip identical monsters */
	if (place_monster_idx == r_idx) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Attempt to place a monster of the given race at the given location
 *
 * Note that certain monsters are now marked as requiring "friends".
 * These monsters, if successfully placed, and if the "grp" parameter
 * is TRUE, will be surrounded by a "group" of identical monsters.
 *
 * Note that certain monsters are now marked as requiring an "escort",
 * which is a collection of monsters with similar "race" but lower level.
 *
 * Some monsters induce a fake "group" flag on their escorts.
 *
 * Note the "bizarre" use of non-recursion to prevent annoying output
 * when running a code profiler.
 *
 * Note the use of the new "monster allocation table" code to restrict
 * the "get_mon_num()" function to "legal" escort types.
 */
bool place_new_monster(struct cave *c, int y, int x, int r_idx, bool slp,
	bool grp, byte origin)
{
	int i;

	monster_race *r_ptr = &r_info[r_idx];

	assert(c);

	/* Place one monster, or fail */
	if (!place_new_monster_one(y, x, r_idx, slp, origin)) return (FALSE);

	/* Require the "group" flag */
	if (!grp) return (TRUE);

	/* Friends for certain monsters */
	if (rf_has(r_ptr->flags, RF_FRIEND)) {
		int total = group_size_2(r_idx);
		
		/* Attempt to place a group */
		(void)place_new_monster_group(c, y, x, r_idx, slp, total, origin);
	}

	/* Friends for certain monsters */
	if (rf_has(r_ptr->flags, RF_FRIENDS)) {
		int total = group_size_1(r_idx);
		
		/* Attempt to place a group */
		(void)place_new_monster_group(c, y, x, r_idx, slp, total, origin);
	}

	/* Escorts for certain monsters */
	if (rf_has(r_ptr->flags, RF_ESCORT))
	{
		/* Try to place several "escorts" */
		for (i = 0; i < 50; i++)
		{
			int nx, ny, z, d = 3;

			/* Pick a location */
			scatter(&ny, &nx, y, x, d, 0);

			/* Require empty grids */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Set the escort index */
			place_monster_idx = r_idx;

			/* Set the escort hook */
			get_mon_num_hook = place_monster_okay;

			/* Prepare allocation table */
			get_mon_num_prep();

			/* Pick a random race */
			z = get_mon_num(r_ptr->level);

			/* Remove restriction */
			get_mon_num_hook = NULL;

			/* Prepare allocation table */
			get_mon_num_prep();

			/* Handle failure */
			if (!z) break;

			/* Place a single escort */
			(void)place_new_monster_one(ny, nx, z, slp, origin);

			/* Place a "group" of escorts if needed */
			if (rf_has(r_info[z].flags, RF_FRIEND)) {
				int total = group_size_2(r_idx);
				
				/* Attempt to place a group */
				(void)place_new_monster_group(c, y, x, r_idx, slp, total, origin);
			}
			
			if (rf_has(r_info[z].flags, RF_FRIENDS) || rf_has(r_ptr->flags, RF_ESCORTS)) {
				int total = group_size_1(r_idx);
				
				/* Place a group of monsters */
				(void)place_new_monster_group(c, ny, nx, z, slp, total, origin);
			}
		}
	}

	/* Success */
	return (TRUE);
}

/*
 * Pick a monster race, make a new monster of that race, then place
 * it in the dungeon.
 */
bool pick_and_place_monster(struct cave *c, int y, int x, int depth, bool slp,
	bool grp, byte origin)
{
	int r_idx;

	/* Pick a monster race */
	r_idx = get_mon_num(depth);

	/* Handle failure */
	if (!r_idx) return (FALSE);

	/* Attempt to place the monster */
	return (place_new_monster(c, y, x, r_idx, slp, grp, origin));
}


/*
 * Attempt to allocate a random monster in the dungeon.
 *
 * Place the monster at least "dis" distance from the player.
 *
 * Use "slp" to choose the initial "sleep" status
 *
 * Use "depth" for the monster level
 */
bool pick_and_place_distant_monster(struct cave *c, struct loc loc, int dis, bool slp, int depth)
{
	int py = loc.y;
	int px = loc.x;

	int y = 0, x = 0;
	int	attempts_left = 10000;

	assert(c);

	/* Find a legal, distant, unoccupied, space */
	while (--attempts_left)
	{
		/* Pick a location */
		y = randint0(c->height);
		x = randint0(c->width);

		/* Require "naked" floor grid */
		if (!cave_isempty(c, y, x)) continue;

		/* Accept far away grids */
		if (distance(y, x, py, px) > dis) break;
	}

	if (!attempts_left)
	{
		if (OPT(cheat_xtra) || OPT(cheat_hear))
		{
			msg("Warning! Could not allocate a new monster.");
		}

		return FALSE;
	}

	/* Attempt to place the monster, allow groups */
	if (pick_and_place_monster(c, y, x, depth, slp, TRUE, ORIGIN_DROP)) return (TRUE);

	/* Nope */
	return (FALSE);
}

/*
 * Create magical stairs after finishing a quest monster.
 */
static void build_quest_stairs(int y, int x)
{
	int ny, nx;


	/* Stagger around */
	while (!cave_valid_bold(y, x))
	{
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
 * Handle the "death" of a monster.
 *
 * Disperse treasures carried by the monster centered at the monster location.
 * Note that objects dropped may disappear in crowded rooms.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 */
void monster_death(int m_idx, bool stats)
{
	int i, y, x;
	int dump_item = 0;
	int dump_gold = 0;
	int total = 0;
	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool visible = (m_ptr->ml || rf_has(r_ptr->flags, RF_UNIQUE));

	object_type *i_ptr;
	object_type object_type_body;

	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;
	
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
		lore_treasure(m_idx, dump_item, dump_gold);

	/* Update monster list window */
	p_ptr->redraw |= PR_MONLIST;

	/* Only process "Quest Monsters" */
	if (!rf_has(r_ptr->flags, RF_QUESTOR)) return;

	/* Mark quests as complete */
	for (i = 0; i < MAX_Q_IDX; i++)	{
		/* Note completed quests */
		if (q_list[i].level == r_ptr->level) q_list[i].level = 0;

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
 * Decrease a monster's hit points and handle monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Invisible monsters induce a special "You have killed it." message.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * Consider decreasing monster experience over time, say, by using
 * "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))" instead
 * of simply "(m_exp * m_lev) / (p_lev)", to make the first monster
 * worth more than subsequent monsters.  This would also need to
 * induce changes in the monster recall code.  XXX XXX XXX
 **/
bool mon_take_hit(int m_idx, int dam, bool *fear, const char *note)
{
	monster_type *m_ptr = cave_monster(cave, m_idx);

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	s32b div, new_exp, new_exp_frac;


	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

	/* Wake it up */
	mon_clear_timed(m_idx, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE);

	/* Become aware of its presence */
	if (m_ptr->unaware)
		become_aware(m_idx);

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];
		char buf[80];

		/* Assume normal death sound */
		int soundfx = MSG_KILL;

		/* Play a special sound if the monster was unique */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			if (r_ptr->base == lookup_monster_base("Morgoth"))
				soundfx = MSG_KILL_KING;
			else
				soundfx = MSG_KILL_UNIQUE;
		}

		/* Extract monster name */
		monster_desc(m_name, sizeof(m_name), m_ptr, 0);

		/* Death by Missile/Spell attack */
		if (note)
		{
			/* Hack -- allow message suppression */
			if (strlen(note) <= 1)
			{
				/* Be silent */
			}

			else msgt(soundfx, "%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
		{
			msgt(soundfx, "You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if (monster_is_unusual(r_ptr))
		{
			msgt(soundfx, "You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			msgt(soundfx, "You have slain %s.", m_name);
		}

		/* Player level */
		div = p_ptr->lev;

		/* Give some experience for the kill */
		new_exp = ((long)r_ptr->mexp * r_ptr->level) / div;

		/* Handle fractional experience */
		new_exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % div)
		                * 0x10000L / div) + p_ptr->exp_frac;

		/* Keep track of experience */
		if (new_exp_frac >= 0x10000L)
		{
			new_exp++;
			p_ptr->exp_frac = (u16b)(new_exp_frac - 0x10000L);
		}
		else
		{
			p_ptr->exp_frac = (u16b)new_exp_frac;
		}

		/* When the player kills a Unique, it stays dead */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			char unique_name[80];
			r_ptr->max_num = 0;

			/* This gets the correct name if we slay an invisible unique and don't have See Invisible. */
			monster_desc(unique_name, sizeof(unique_name), m_ptr, MDESC_SHOW | MDESC_IND2);

			/* Log the slaying of a unique */
			strnfmt(buf, sizeof(buf), "Killed %s", unique_name);
			history_add(buf, HISTORY_SLAY_UNIQUE, 0);
		}

		/* Gain experience */
		player_exp_gain(p_ptr, new_exp);

		/* Generate treasure */
		monster_death(m_idx, FALSE);

		/* Recall even invisible uniques or winners */
		if (m_ptr->ml || rf_has(r_ptr->flags, RF_UNIQUE))
		{
			/* Count kills this life */
			if (l_ptr->pkills < MAX_SHORT) l_ptr->pkills++;

			/* Count kills in all lives */
			if (l_ptr->tkills < MAX_SHORT) l_ptr->tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(m_ptr->r_idx);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}


	/* Mega-Hack -- Pain cancels fear */
	if (!(*fear) && m_ptr->m_timed[MON_TMD_FEAR] && (dam > 0))
	{
		int tmp = randint1(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->m_timed[MON_TMD_FEAR])
		{
			/* Reduce fear */
			mon_dec_timed(m_idx, MON_TMD_FEAR, tmp , MON_TMD_FLG_NOMESSAGE);
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			mon_clear_timed(m_idx, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE);

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->m_timed[MON_TMD_FEAR] && !rf_has(r_ptr->flags, RF_NO_FEAR) && dam > 0)
	{
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

			mon_inc_timed(m_idx, MON_TMD_FEAR, timer, MON_TMD_FLG_NOMESSAGE | MON_TMD_FLG_NOFAIL);
		}
	}


	/* Not dead yet */
	return (FALSE);
}

