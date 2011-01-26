/*
 * File: monster2.c
 * Purpose: Low-level monster manipulation
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
#include "generate.h"
#include "history.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "object/object.h"
#include "target.h"

/*
 * Delete a monster by index.
 *
 * When a monster is deleted, all of its objects are deleted.
 */
void delete_monster_idx(int i)
{
	int x, y;

	monster_type *m_ptr = &mon_list[i];

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

		/* Hack -- efficiency */
		o_ptr->held_m_idx = 0;

		/* Delete the object */
		delete_object_idx(this_o_idx);
	}


	/* Wipe the Monster */
	(void)WIPE(m_ptr, monster_type);

	/* Count monsters */
	mon_cnt--;

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
	m_ptr = &mon_list[i1];

	/* Location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Update the cave */
	cave->m_idx[y][x] = i2;

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

	/* Hack -- Update the target */
	if (target_get_monster() == i1) target_set_monster(i2);

	/* Hack -- Update the health bar */
	if (p_ptr->health_who == i1) p_ptr->health_who = i2;

	/* Hack -- move monster */
	COPY(&mon_list[i2], &mon_list[i1], monster_type);

	/* Hack -- wipe hole */
	(void)WIPE(&mon_list[i1], monster_type);
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
		for (i = 1; i < mon_max; i++)
		{
			monster_type *m_ptr = &mon_list[i];

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
	for (i = mon_max - 1; i >= 1; i--)
	{
		/* Get the i'th monster */
		monster_type *m_ptr = &mon_list[i];

		/* Skip real monsters */
		if (m_ptr->r_idx) continue;

		/* Move last monster into open hole */
		compact_monsters_aux(mon_max - 1, i);

		/* Compress "mon_max" */
		mon_max--;
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
	for (i = mon_max - 1; i >= 1; i--)
	{
		monster_type *m_ptr = &mon_list[i];

		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Mega-Hack -- preserve Unique's XXX XXX XXX */

		/* Hack -- Reduce the racial counter */
		r_ptr->cur_num--;

		/* Monster is gone */
		c->m_idx[m_ptr->fy][m_ptr->fx] = 0;

		/* Wipe the Monster */
		(void)WIPE(m_ptr, monster_type);
	}

	/* Reset "mon_max" */
	mon_max = 1;

	/* Reset "mon_cnt" */
	mon_cnt = 0;

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
s16b mon_pop(void)
{
	int i;


	/* Normal allocation */
	if (mon_max < z_info->m_max)
	{
		/* Get the next hole */
		i = mon_max;

		/* Expand the array */
		mon_max++;

		/* Count monsters */
		mon_cnt++;

		/* Return the index */
		return (i);
	}


	/* Recycle dead monsters */
	for (i = 1; i < mon_max; i++)
	{
		monster_type *m_ptr;

		/* Get the monster */
		m_ptr = &mon_list[i];

		/* Skip live monsters */
		if (m_ptr->r_idx) continue;

		/* Count monsters */
		mon_cnt++;

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


/*
 * Display visible monsters in a window
 */
void display_monlist(void)
{
	size_t i, j, k;
	int max;
	int line = 1, x = 0;
	int cur_x;
	unsigned total_count = 0, disp_count = 0, type_count = 0, los_count = 0;

	byte attr;

	char *m_name;
	char buf[80];

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_race *r2_ptr;

	monster_vis *list;

	u16b *order;

	bool in_term = (Term != angband_term[0]);

	/* Hallucination is weird */
	if (p_ptr->timed[TMD_IMAGE])
	{
		if (in_term)
			clear_from(0);
		Term_gotoxy(0, 0);
		text_out_to_screen(TERM_ORANGE,
			"Your hallucinations are too wild to see things clearly.");

		return;
	}


	/* Clear the term if in a subwindow, set x otherwise */
	if (in_term)
	{
		clear_from(0);
		max = Term->hgt - 1;
	}
	else
	{
		x = 13;
		max = Term->hgt - 2;
	}

	/* Allocate the primary array */
	list = C_ZNEW(z_info->r_max, monster_vis);

	/* Scan the list of monsters on the level */
	for (i = 1; i < (size_t)mon_max; i++)
	{
		monster_vis *v;

		m_ptr = &mon_list[i];
		r_ptr = &r_info[m_ptr->r_idx];

		/* Only consider visible monsters */
		if (!m_ptr->ml) continue;

		/* Take a pointer to this monster visibility entry */
		v = &list[m_ptr->r_idx];

		/* Note each monster type and save its display attr (color) */
		if (!v->count) type_count++;
		if (!v->attr) v->attr = m_ptr->attr ? m_ptr->attr : r_ptr->x_attr;
		
		/* Check for LOS
		 * Hack - we should use (m_ptr->mflag & (MFLAG_VIEW)) here,
		 * but this does not catch monsters detected by ESP which are
		 * targetable, so we cheat and use projectable() instead 
		 */
		if (projectable(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx,
			PROJECT_NONE))
		{
			/* Increment the total number of in-LOS monsters */
			los_count++;

			/* Increment the LOS count for this monster type */
			v->los++;
			
			/* Check if asleep and increment accordingly */
			if (m_ptr->csleep) v->los_asleep++;
		}
		/* Not in LOS so increment if asleep */
		else if (m_ptr->csleep) v->asleep++;

		/* Bump the count for this race, and the total count */
		v->count++;
		total_count++;
	}

	/* Note no visible monsters at all */
	if (!total_count)
	{
		/* Clear display and print note */
		c_prt(TERM_SLATE, "You see no monsters.", 0, 0);
		if (!in_term)
		    Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

		/* Free up memory */
		FREE(list);

		/* Done */
		return;
	}

	/* Allocate the secondary array */
	order = C_ZNEW(type_count, u16b);

	/* Sort, because we cannot rely on monster.txt being ordered */

	/* Populate the ordered array, starting at 1 to ignore @ */
	for (i = 1; i < z_info->r_max; i++)
	{
		/* No monsters of this race are visible */
		if (!list[i].count) continue;

		/* Get the monster info */
		r_ptr = &r_info[i];

		/* Fit this monster into the sorted array */
		for (j = 0; j < type_count; j++)
		{
			/* If we get to the end of the list, put this one in */
			if (!order[j])
			{
				order[j] = i;
				break;
			}

			/* Get the monster info for comparison */
			r2_ptr = &r_info[order[j]];

			/* Monsters are sorted by depth */
			/* Monsters of same depth are sorted by power */
			if ((r_ptr->level > r2_ptr->level) ||
				((r_ptr->level == r2_ptr->level) &&
				(r_ptr->power > r2_ptr->power)))
			{
				/* Move weaker monsters down the array */
				for (k = type_count - 1; k > j; k--)
				{
					order[k] = order[k - 1];
				}

				/* Put current monster in the right place */
				order[j] = i;
				break;
			}
		}
	}

	/* Message for monsters in LOS - even if there are none */
	if (!los_count) prt(format("You can see no monsters."), 0, 0);
	else prt(format("You can see %d monster%s", los_count, (los_count == 1
		? ":" : "s:")), 0, 0);

	/* Print out in-LOS monsters in descending order */
	for (i = 0; (i < type_count) && (line < max); i++)
	{
		/* Skip if there are none of these in LOS */
		if (!list[order[i]].los) continue;

		/* Reset position */
		cur_x = x;

		/* Note that these have been displayed */
		disp_count += list[order[i]].los;

		/* Get monster race and name */
		r_ptr = &r_info[order[i]];
		m_name = r_ptr->name;

		/* Display uniques in a special colour */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
			attr = TERM_VIOLET;
		else if (r_ptr->level > p_ptr->depth)
			attr = TERM_RED;
		else
			attr = TERM_WHITE;

		/* Build the monster name */
		if (list[order[i]].los == 1)
			strnfmt(buf, sizeof(buf), (list[order[i]].los_asleep ==
			1 ? "%s (asleep) " : "%s "), m_name);
		else strnfmt(buf, sizeof(buf), (list[order[i]].los_asleep > 0 ?
			"%s (x%d, %d asleep) " : "%s (x%d)"), m_name, 
			list[order[i]].los, list[order[i]].los_asleep);

		/* Display the pict */
		if ((tile_width == 1) && (tile_height == 1))
		{
		        Term_putch(cur_x++, line, list[order[i]].attr, r_ptr->x_char);
			Term_putch(cur_x++, line, TERM_WHITE, ' ');
		}

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;

		/* Page wrap */
		if (!in_term && (line == max) && disp_count != total_count)
		{
			prt("-- more --", line, x);
			anykey();

			/* Clear the screen */
			for (line = 1; line <= max; line++)
				prt("", line, 0);

			/* Reprint Message */
			prt(format("You can see %d monster%s",
				los_count, (los_count > 0 ? (los_count == 1 ?
				":" : "s:") : "s.")), 0, 0);

			/* Reset */
			line = 1;
		}
	}

	/* Message for monsters outside LOS, if there are any */
	if (total_count > los_count)
	{
		/* Leave a blank line */
		line++;
		
		prt(format("You are aware of %d %smonster%s", 
		(total_count - los_count), (los_count > 0 ? "other " : ""), 
		((total_count - los_count) == 1 ? ":" : "s:")), line++, 0);
	}

	/* Print out non-LOS monsters in descending order */
	for (i = 0; (i < type_count) && (line < max); i++)
	{
		/* Skip if there are none of these out of LOS */
		if (list[order[i]].count == list[order[i]].los) continue;

		/* Reset position */
		cur_x = x;

		/* Note that these have been displayed */
		disp_count += (list[order[i]].count - list[order[i]].los);

		/* Get monster race and name */
		r_ptr = &r_info[order[i]];
		m_name = r_ptr->name;

		/* Display uniques in a special colour */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
			attr = TERM_VIOLET;
		else if (r_ptr->level > p_ptr->depth)
			attr = TERM_RED;
		else
			attr = TERM_WHITE;

		/* Build the monster name */
		if ((list[order[i]].count - list[order[i]].los) == 1)
			strnfmt(buf, sizeof(buf), (list[order[i]].asleep ==
			1 ? "%s (asleep) " : "%s "), m_name);
		else strnfmt(buf, sizeof(buf), (list[order[i]].asleep > 0 ? 
			"%s (x%d, %d asleep) " : "%s (x%d) "), m_name, 
			(list[order[i]].count - list[order[i]].los),
			list[order[i]].asleep);

		/* Display the pict */
		if ((tile_width == 1) && (tile_height == 1))
		{
		        Term_putch(cur_x++, line, list[order[i]].attr, r_ptr->x_char);
			Term_putch(cur_x++, line, TERM_WHITE, ' ');
		}

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;

		/* Page wrap */
		if (!in_term && (line == max) && disp_count != total_count)
		{
			prt("-- more --", line, x);
			anykey();

			/* Clear the screen */
			for (line = 1; line <= max; line++)
				prt("", line, 0);

			/* Reprint Message */
			prt(format("You are aware of %d %smonster%s",
				(total_count - los_count), (los_count > 0 ?
				"other " : ""), ((total_count - los_count) > 0
				? ((total_count - los_count) == 1 ? ":" : "s:")
				: "s.")), 0, 0);

			/* Reset */
			line = 1;
		}
	}


	/* Print "and others" message if we've run out of space */
	if (disp_count != total_count)
	{
		strnfmt(buf, sizeof buf, "  ...and %d others.", total_count - disp_count);
		c_prt(TERM_WHITE, buf, line, x);
	}

	/* Otherwise clear a line at the end, for main-term display */
	else
	{
		prt("", line, x);
	}

	if (!in_term)
		Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

	/* Free the arrays */
	FREE(list);
	FREE(order);
}


/*
 * Build a string describing a monster in some way.
 *
 * We can correctly describe monsters based on their visibility.
 * We can force all monsters to be treated as visible or invisible.
 * We can build nominatives, objectives, possessives, or reflexives.
 * We can selectively pronominalize hidden, visible, or all monsters.
 * We can use definite or indefinite descriptions for hidden monsters.
 * We can use definite or indefinite descriptions for visible monsters.
 *
 * Pronominalization involves the gender whenever possible and allowed,
 * so that by cleverly requesting pronominalization / visibility, you
 * can get messages like "You hit someone.  She screams in agony!".
 *
 * Reflexives are acquired by requesting Objective plus Possessive.
 *
 * I am assuming that no monster name is more than 65 characters long,
 * so that "char desc[80];" is sufficiently large for any result, even
 * when the "offscreen" notation is added.
 *
 * Note that the "possessive" for certain unique monsters will look
 * really silly, as in "Morgoth, King of Darkness's".  We should
 * perhaps add a flag to "remove" any "descriptives" in the name.
 *
 * Note that "offscreen" monsters will get a special "(offscreen)"
 * notation in their name if they are visible but offscreen.  This
 * may look silly with possessives, as in "the rat's (offscreen)".
 * Perhaps the "offscreen" descriptor should be abbreviated.
 *
 * Mode Flags:
 *   0x01 --> Objective (or Reflexive)
 *   0x02 --> Possessive (or Reflexive)
 *   0x04 --> Use indefinites for hidden monsters ("something")
 *   0x08 --> Use indefinites for visible monsters ("a kobold")
 *   0x10 --> Pronominalize hidden monsters
 *   0x20 --> Pronominalize visible monsters
 *   0x40 --> Assume the monster is hidden
 *   0x80 --> Assume the monster is visible
 *
 * Useful Modes:
 *   0x00 --> Full nominative name ("the kobold") or "it"
 *   0x04 --> Full nominative name ("the kobold") or "something"
 *   0x80 --> Banishment resistance name ("the kobold")
 *   0x88 --> Killing name ("a kobold")
 *   0x22 --> Possessive, genderized if visable ("his") or "its"
 *   0x23 --> Reflexive, genderized if visable ("himself") or "itself"
 */
void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode)
{
	const char * res;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	const char * name = r_ptr->name;

	bool seen, pron;


	/* Can we "see" it (forced, or not hidden + visible) */
	seen = ((mode & (0x80)) || (!(mode & (0x40)) && m_ptr->ml));

	/* Sexed Pronouns (seen and forced, or unseen and allowed) */
	pron = ((seen && (mode & (0x20))) || (!seen && (mode & (0x10))));


	/* First, try using pronouns, or describing hidden monsters */
	if (!seen || pron)
	{
		/* an encoding of the monster "sex" */
		int kind = 0x00;

		/* Extract the gender (if applicable) */
		if (rf_has(r_ptr->flags, RF_FEMALE)) kind = 0x20;
		else if (rf_has(r_ptr->flags, RF_MALE)) kind = 0x10;

		/* Ignore the gender (if desired) */
		if (!m_ptr || !pron) kind = 0x00;


		/* Assume simple result */
		res = "it";

		/* Brute force: split on the possibilities */
		switch (kind + (mode & 0x07))
		{
			/* Neuter, or unknown */
			case 0x00: res = "it"; break;
			case 0x01: res = "it"; break;
			case 0x02: res = "its"; break;
			case 0x03: res = "itself"; break;
			case 0x04: res = "something"; break;
			case 0x05: res = "something"; break;
			case 0x06: res = "something's"; break;
			case 0x07: res = "itself"; break;

			/* Male (assume human if vague) */
			case 0x10: res = "he"; break;
			case 0x11: res = "him"; break;
			case 0x12: res = "his"; break;
			case 0x13: res = "himself"; break;
			case 0x14: res = "someone"; break;
			case 0x15: res = "someone"; break;
			case 0x16: res = "someone's"; break;
			case 0x17: res = "himself"; break;

			/* Female (assume human if vague) */
			case 0x20: res = "she"; break;
			case 0x21: res = "her"; break;
			case 0x22: res = "her"; break;
			case 0x23: res = "herself"; break;
			case 0x24: res = "someone"; break;
			case 0x25: res = "someone"; break;
			case 0x26: res = "someone's"; break;
			case 0x27: res = "herself"; break;
		}

		/* Copy the result */
		my_strcpy(desc, res, max);
	}


	/* Handle visible monsters, "reflexive" request */
	else if ((mode & 0x02) && (mode & 0x01))
	{
		/* The monster is visible, so use its gender */
		if (rf_has(r_ptr->flags, RF_FEMALE)) my_strcpy(desc, "herself", max);
		else if (rf_has(r_ptr->flags, RF_MALE)) my_strcpy(desc, "himself", max);
		else my_strcpy(desc, "itself", max);
	}


	/* Handle all other visible monster requests */
	else
	{
		/* It could be a Unique */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			/* Start with the name (thus nominative and objective) */
			my_strcpy(desc, name, max);
		}

		/* It could be an indefinite monster */
		else if (mode & 0x08)
		{
			/* XXX Check plurality for "some" */

			/* Indefinite monsters need an indefinite article */
			my_strcpy(desc, is_a_vowel(name[0]) ? "an " : "a ", max);
			my_strcat(desc, name, max);
		}

		/* It could be a normal, definite, monster */
		else
		{
			/* Definite monsters need a definite article */
			my_strcpy(desc, "the ", max);
			my_strcat(desc, name, max);
		}

		/* Handle the Possessive as a special afterthought */
		if (mode & 0x02)
		{
			/* XXX Check for trailing "s" */

			/* Simply append "apostrophe" and "s" */
			my_strcat(desc, "'s", max);
		}

		/* Mention "offscreen" monsters XXX XXX */
		if (!panel_contains(m_ptr->fy, m_ptr->fx))
		{
			/* Append special notation */
			my_strcat(desc, " (offscreen)", max);
		}
	}
}




/*
 * Learn about a monster (by "probing" it)
 */
void lore_do_probe(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	unsigned i;

	/* Know various things */
	rsf_setall(l_ptr->flags);
	rsf_copy(l_ptr->spell_flags, r_ptr->spell_flags);
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
		l_ptr->blows[i] = MAX_UCHAR;

	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
		p_ptr->redraw |= (PR_MONSTER);
}


/*
 * Take note that the given monster just dropped some treasure
 *
 * Note that learning the "GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 *
 * This "indirect" method is used to prevent the player from learning
 * exactly how much treasure a monster can drop from observing only
 * a single example of a drop.  This method actually observes how much
 * gold and items are dropped, and remembers that information to be
 * described later by the monster recall code.
 */
void lore_treasure(int m_idx, int num_item, int num_gold)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];


	/* Note the number of things dropped */
	if (num_item > l_ptr->drop_item) l_ptr->drop_item = num_item;
	if (num_gold > l_ptr->drop_gold) l_ptr->drop_gold = num_gold;

	/* Learn about drop quality */
	rf_on(l_ptr->flags, RF_DROP_GOOD);
	rf_on(l_ptr->flags, RF_DROP_GREAT);

	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->redraw |= (PR_MONSTER);
	}
}



/*
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player (if requested),
 * and then checking for visibility (natural, infravision, see-invis,
 * telepathy), updating the monster visibility flag, redrawing (or
 * erasing) the monster when its visibility changes, and taking note
 * of any interesting monster flags (cold-blooded, invisible, etc).
 *
 * Note the new "mflag" field which encodes several monster state flags,
 * including "view" for when the monster is currently in line of sight,
 * and "mark" for when the monster is currently visible via detection.
 *
 * The only monster fields that are changed here are "cdis" (the
 * distance from the player), "ml" (visible to the player), and
 * "mflag" (to maintain the "MFLAG_VIEW" flag).
 *
 * Note the special "update_monsters()" function which can be used to
 * call this function once for every monster.
 *
 * Note the "full" flag which requests that the "cdis" field be updated,
 * this is only needed when the monster (or the player) has moved.
 *
 * Every time a monster moves, we must call this function for that
 * monster, and update the distance, and the visibility.  Every time
 * the player moves, we must call this function for every monster, and
 * update the distance, and the visibility.  Whenever the player "state"
 * changes in certain ways ("blindness", "infravision", "telepathy",
 * and "see invisible"), we must call this function for every monster,
 * and update the visibility.
 *
 * Routines that change the "illumination" of a grid must also call this
 * function for any monster in that grid, since the "visibility" of some
 * monsters may be based on the illumination of their grid.
 *
 * Note that this function is called once per monster every time the
 * player moves.  When the player is running, this function is one
 * of the primary bottlenecks, along with "update_view()" and the
 * "process_monsters()" code, so efficiency is important.
 *
 * Note the optimized "inline" version of the "distance()" function.
 *
 * A monster is "visible" to the player if (1) it has been detected
 * by the player, (2) it is close to the player and the player has
 * telepathy, or (3) it is close to the player, and in line of sight
 * of the player, and it is "illuminated" by some combination of
 * infravision, torch light, or permanent light (invisible monsters
 * are only affected by "light" if the player can see invisible).
 *
 * Monsters which are not on the current panel may be "visible" to
 * the player, and their descriptions will include an "offscreen"
 * reference.  Currently, offscreen monsters cannot be targeted
 * or viewed directly, but old targets will remain set.  XXX XXX
 *
 * The player can choose to be disturbed by several things, including
 * "OPT(disturb_move)" (monster which is viewable moves in some way), and
 * "OPT(disturb_near)" (monster which is "easily" viewable moves in some
 * way).  Note that "moves" includes "appears" and "disappears".
 */
void update_mon(int m_idx, bool full)
{
	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	int d;

	/* Current location */
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	/* Seen at all */
	bool flag = FALSE;

	/* Seen by vision */
	bool easy = FALSE;


	/* Compute distance */
	if (full)
	{
		int py = p_ptr->py;
		int px = p_ptr->px;

		/* Distance components */
		int dy = (py > fy) ? (py - fy) : (fy - py);
		int dx = (px > fx) ? (px - fx) : (fx - px);

		/* Approximate distance */
		d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

		/* Restrict distance */
		if (d > 255) d = 255;

		/* Save the distance */
		m_ptr->cdis = d;
	}

	/* Extract distance */
	else
	{
		/* Extract the distance */
		d = m_ptr->cdis;
	}


	/* Detected */
	if (m_ptr->mflag & (MFLAG_MARK)) flag = TRUE;


	/* Nearby */
	if (d <= MAX_SIGHT)
	{
		/* Basic telepathy */
		if (p_ptr->state.telepathy)
		{
			/* Empty mind, no telepathy */
			if (rf_has(r_ptr->flags, RF_EMPTY_MIND))
			{
				/* Nothing! */
			}

			/* Weird mind, occasional telepathy */
			else if (rf_has(r_ptr->flags, RF_WEIRD_MIND))
			{
				/* One in ten individuals are detectable */
				if ((m_idx % 10) == 5)
				{
					/* Detectable */
					flag = TRUE;

					/* Check for LOS so that MFLAG_VIEW is set later */
					if (player_has_los_bold(fy, fx)) easy = TRUE;
				}
			}

			/* Normal mind, allow telepathy */
			else
			{
				/* Detectable */
				flag = TRUE;

				/* Check for LOS to that MFLAG_VIEW is set later */
				if (player_has_los_bold(fy, fx)) easy = TRUE;
			}
		}

		/* Normal line of sight, and not blind */
		if (player_has_los_bold(fy, fx) && !p_ptr->timed[TMD_BLIND])
		{
			/* Use "infravision" */
			if (d <= p_ptr->state.see_infra)
			{
				/* Learn about warm/cold blood */
				rf_on(l_ptr->flags, RF_COLD_BLOOD);
				
				/* Handle "warm blooded" monsters */
				if (!rf_has(r_ptr->flags, RF_COLD_BLOOD))
				{
					/* Easy to see */
					easy = flag = TRUE;
				}
			}

			/* See if the monster is emitting lite */
			/*if (rf_has(r_ptr->flags, RF_HAS_LITE)) easy = flag = TRUE;*/

			/* Use "illumination" */
			if (player_can_see_bold(fy, fx))
			{
				/* Learn it emits light */
				rf_on(l_ptr->flags, RF_HAS_LITE);

				/* Learn about invisibility */
				rf_on(l_ptr->flags, RF_INVISIBLE);
				
				/* Handle "invisible" monsters */
				if (rf_has(r_ptr->flags, RF_INVISIBLE))
				{
					/* See invisible */
					if (p_ptr->state.see_inv)
					{
						/* Easy to see */
						easy = flag = TRUE;
					}
				}

				/* Handle "normal" monsters */
				else
				{
					/* Easy to see */
					easy = flag = TRUE;
				}
			}
		}
	}


	/* The monster is now visible */
	if (flag)
	{
		/* Learn about the monster's mind */
		if (p_ptr->state.telepathy)
		{
			flags_set(l_ptr->flags, RF_SIZE, RF_EMPTY_MIND, RF_WEIRD_MIND, RF_SMART, RF_STUPID, FLAG_END);
		}

		/* It was previously unseen */
		if (!m_ptr->ml)
		{
			/* Mark as visible */
			m_ptr->ml = TRUE;

			/* Draw the monster */
			cave_light_spot(cave, fy, fx);

			/* Update health bar as needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Hack -- Count "fresh" sightings */
			if (l_ptr->sights < MAX_SHORT) l_ptr->sights++;

			/* Disturb on appearance */
			if (OPT(disturb_move)) disturb(1, 0);

			/* Window stuff */
			p_ptr->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not visible */
	else
	{
		/* It was previously seen */
		if (m_ptr->ml)
		{
			/* Mark as not visible */
			m_ptr->ml = FALSE;

			/* Erase the monster */
			cave_light_spot(cave, fy, fx);

			/* Update health bar as needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Disturb on disappearance */
			if (OPT(disturb_move)) disturb(1, 0);

			/* Window stuff */
			p_ptr->redraw |= PR_MONLIST;
		}
	}


	/* The monster is now easily visible */
	if (easy)
	{
		/* Change */
		if (!(m_ptr->mflag & (MFLAG_VIEW)))
		{
			/* Mark as easily visible */
			m_ptr->mflag |= (MFLAG_VIEW);

			/* Disturb on appearance */
			if (OPT(disturb_near)) disturb(1, 0);

			/* Re-draw monster window */
			p_ptr->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not easily visible */
	else
	{
		/* Change */
		if (m_ptr->mflag & (MFLAG_VIEW))
		{
			/* Mark as not easily visible */
			m_ptr->mflag &= ~(MFLAG_VIEW);

			/* Disturb on disappearance */
			if (OPT(disturb_near)) disturb(1, 0);

			/* Re-draw monster list window */
			p_ptr->redraw |= PR_MONLIST;
		}
	}
}




/*
 * This function simply updates all the (non-dead) monsters (see above).
 */
void update_monsters(bool full)
{
	int i;

	/* Update each (live) monster */
	for (i = 1; i < mon_max; i++)
	{
		monster_type *m_ptr = &mon_list[i];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Update the monster */
		update_mon(i, full);
	}
}




/*
 * Make a monster carry an object
 */
s16b monster_carry(int m_idx, object_type *j_ptr)
{
	s16b o_idx;

	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = &mon_list[m_idx];


	/* Scan objects already being held for combination */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr, OSTACK_MONSTER))
		{
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Result */
			return (this_o_idx);
		}
	}


	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		object_type *o_ptr;

		/* Get new object */
		o_ptr = object_byid(o_idx);

		/* Copy object */
		object_copy(o_ptr, j_ptr);

		/* Forget mark */
		o_ptr->marked = FALSE;

		/* Forget location */
		o_ptr->iy = o_ptr->ix = 0;

		/* Link the object to the monster */
		o_ptr->held_m_idx = m_idx;

		/* Link the object to the pile */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Link the monster to the object */
		m_ptr->hold_o_idx = o_idx;
	}

	/* Result */
	return (o_idx);
}


/*
 * Swap the players/monsters (if any) at two locations XXX XXX XXX
 */
void monster_swap(int y1, int x1, int y2, int x2)
{
	int m1, m2;

	monster_type *m_ptr;

	monster_race *r_ptr;

	/* Monsters */
	m1 = cave->m_idx[y1][x1];
	m2 = cave->m_idx[y2][x2];


	/* Update grids */
	cave->m_idx[y1][x1] = m2;
	cave->m_idx[y2][x2] = m1;


	/* Monster 1 */
	if (m1 > 0)
	{
		m_ptr = &mon_list[m1];

		/* Move monster */
		m_ptr->fy = y2;
		m_ptr->fx = x2;

		/* Update monster */
		update_mon(m1, TRUE);

		/* Radiate light? */
		r_ptr = &r_info[m_ptr->r_idx];
		if (rf_has(r_ptr->flags, RF_HAS_LITE)) p_ptr->redraw |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Player 1 */
	else if (m1 < 0)
	{
		/* Move player */
		p_ptr->py = y2;
		p_ptr->px = x2;

		/* Update the trap detection status */
		p_ptr->redraw |= (PR_DTRAP);

		/* Update the panel */
		p_ptr->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_UPDATE_FLOW);

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Monster 2 */
	if (m2 > 0)
	{
		m_ptr = &mon_list[m2];

		/* Move monster */
		m_ptr->fy = y1;
		m_ptr->fx = x1;

		/* Update monster */
		update_mon(m2, TRUE);

		/* Radiate light? */
		r_ptr = &r_info[m_ptr->r_idx];
		if (rf_has(r_ptr->flags, RF_HAS_LITE)) p_ptr->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Player 2 */
	else if (m2 < 0)
	{
		/* Move player */
		p_ptr->py = y1;
		p_ptr->px = x1;

		/* Update the trap detection status */
		p_ptr->redraw |= (PR_DTRAP);

		/* Update the panel */
		p_ptr->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_UPDATE_FLOW);

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}


	/* Redraw */
	cave_light_spot(cave, y1, x1);
	cave_light_spot(cave, y2, x2);
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


/*
 * Place a copy of a monster in the dungeon XXX XXX
 */
s16b monster_place(int y, int x, monster_type *n_ptr)
{
	s16b m_idx;

	monster_type *m_ptr;
	monster_race *r_ptr;


	/* Paranoia XXX XXX */
	if (cave->m_idx[y][x] != 0) return (0);


	/* Get a new record */
	m_idx = mon_pop();

	/* Oops */
	if (m_idx)
	{
		/* Make a new monster */
		cave->m_idx[y][x] = m_idx;

		/* Get the new monster */
		m_ptr = &mon_list[m_idx];

		/* Copy the monster XXX */
		COPY(m_ptr, n_ptr, monster_type);

		/* Location */
		m_ptr->fy = y;
		m_ptr->fx = x;

		/* Update the monster */
		update_mon(m_idx, TRUE);

		/* Get the new race */
		r_ptr = &r_info[m_ptr->r_idx];

		/* Hack -- Notice new multi-hued monsters */
		if (rf_has(r_ptr->flags, RF_ATTR_MULTI)) shimmer_monsters = TRUE;

		/* Hack -- Count the number of "reproducers" */
		if (rf_has(r_ptr->flags, RF_MULTIPLY)) num_repro++;

		/* Count racial occurances */
		r_ptr->cur_num++;
	}

	/* Result */
	return (m_idx);
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
static bool place_monster_one(int y, int x, int r_idx, bool slp)
{
	int i;

	monster_race *r_ptr;

	monster_type *n_ptr;
	monster_type monster_type_body;

	const char * name;


	/* Paranoia */
	if (!in_bounds(y, x)) return (FALSE);

	/* Require empty space */
	if (!cave_empty_bold(y, x)) return (FALSE);

	/* Hack -- no creation on glyph of warding */
	if (cave->feat[y][x] == FEAT_GLYPH) return (FALSE);


	/* Paranoia */
	if (!r_idx) return (FALSE);

	/* Race */
	r_ptr = &r_info[r_idx];

	/* Paranoia */
	if (!r_ptr->name) return (FALSE);

	/* Name */
	name = r_ptr->name;


	/* Hack -- "unique" monsters must be "unique" */
	if (rf_has(r_ptr->flags, RF_UNIQUE) && r_ptr->cur_num >= r_ptr->max_num)
	{
		/* Cannot create */
		return (FALSE);
	}


	/* Depth monsters may NOT be created out of depth */
	if (rf_has(r_ptr->flags, RF_FORCE_DEPTH) && p_ptr->depth < r_ptr->level)
	{
		/* Cannot create */
		return (FALSE);
	}


	/* Powerful monster */
	if (r_ptr->level > p_ptr->depth)
	{
		/* Unique monsters */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			/* Message for cheaters */
			if (OPT(cheat_hear)) msg("Deep Unique (%s).", name);

			/* Boost rating by twice delta-depth */
			cave->rating += (r_ptr->level - p_ptr->depth) * 2;
		}

		/* Normal monsters */
		else
		{
			/* Message for cheaters */
			if (OPT(cheat_hear)) msg("Deep Monster (%s).", name);

			/* Boost rating by delta-depth */
			cave->rating += (r_ptr->level - p_ptr->depth);
		}
	}

	/* Note the monster */
	else if (rf_has(r_ptr->flags, RF_UNIQUE))
	{
		/* Unique monsters induce message */
		if (OPT(cheat_hear)) msg("Unique (%s).", name);
	}


	/* Get local monster */
	n_ptr = &monster_type_body;

	/* Clean out the monster */
	(void)WIPE(n_ptr, monster_type);


	/* Save the race */
	n_ptr->r_idx = r_idx;


	/* Enforce sleeping if needed */
	if (slp && r_ptr->sleep)
	{
		int val = r_ptr->sleep;
		n_ptr->csleep = ((val * 2) + randint1(val * 10));
	}


	/* Uniques get a fixed amount of HP */
	if (rf_has(r_ptr->flags, RF_UNIQUE))
	{
		n_ptr->maxhp = r_ptr->avg_hp;
	}
	else
	{
		int std_dev = (((r_ptr->avg_hp * 10) / 8) + 5) / 10;
		if (r_ptr->avg_hp > 1) std_dev++;

		n_ptr->maxhp = Rand_normal(r_ptr->avg_hp, std_dev);
		n_ptr->maxhp = MAX(n_ptr->maxhp, 1);
	}

	/* And start out fully healthy */
	n_ptr->hp = n_ptr->maxhp;


	/* Extract the monster base speed */
	n_ptr->mspeed = r_ptr->speed;

	/* Hack -- small racial variety */
	if (!rf_has(r_ptr->flags, RF_UNIQUE))
	{
		/* Allow some small variation per monster */
		i = extract_energy[r_ptr->speed] / 10;
		if (i) n_ptr->mspeed += rand_spread(0, i);
	}


	/* Give a random starting energy */
	n_ptr->energy = (byte)randint0(50);

	/* Force monster to wait for player */
	if (rf_has(r_ptr->flags, RF_FORCE_SLEEP))
	{
		/* Monster is still being nice */
		n_ptr->mflag |= (MFLAG_NICE);

		/* Optimize -- Repair flags */
		repair_mflag_nice = TRUE;
	}

	/* Radiate light? */
	if (rf_has(r_ptr->flags, RF_HAS_LITE)) p_ptr->update |= PU_UPDATE_VIEW;

	/* Place the monster in the dungeon */
	if (!monster_place(y, x, n_ptr)) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	32


/*
 * Attempt to place a "group" of monsters around the given location
 */
static bool place_monster_group(struct cave *c, int y, int x, int r_idx, bool slp)
{
	monster_race *r_ptr = &r_info[r_idx];

	int old, n, i;
	int total, extra = 0;

	int hack_n;

	byte hack_y[GROUP_MAX];
	byte hack_x[GROUP_MAX];


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

	/* Hack -- limit group reduction */
	if (extra > 12) extra = 12;

	/* Modify the group size */
	total += extra;

	/* Minimum size */
	if (total < 1) total = 1;

	/* Maximum size */
	if (total > GROUP_MAX) total = GROUP_MAX;


	/* Save the rating */
	old = c->rating;

	/* Start on the monster */
	hack_n = 1;
	hack_x[0] = x;
	hack_y[0] = y;

	/* Puddle monsters, breadth first, up to total */
	for (n = 0; (n < hack_n) && (hack_n < total); n++)
	{
		/* Grab the location */
		int hx = hack_x[n];
		int hy = hack_y[n];

		/* Check each direction, up to total */
		for (i = 0; (i < 8) && (hack_n < total); i++)
		{
			int mx = hx + ddx_ddd[i];
			int my = hy + ddy_ddd[i];

			/* Walls and Monsters block flow */
			if (!cave_empty_bold(my, mx)) continue;

			/* Attempt to place another monster */
			if (place_monster_one(my, mx, r_idx, slp))
			{
				/* Add it to the "hack" set */
				hack_y[hack_n] = my;
				hack_x[hack_n] = mx;
				hack_n++;
			}
		}
	}

	/* Hack -- restore the rating */
	c->rating = old;


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
bool place_monster_aux(struct cave *c, int y, int x, int r_idx, bool slp, bool grp)
{
	int i;

	monster_race *r_ptr = &r_info[r_idx];

	assert(c);

	/* Place one monster, or fail */
	if (!place_monster_one(y, x, r_idx, slp)) return (FALSE);


	/* Require the "group" flag */
	if (!grp) return (TRUE);


	/* Friends for certain monsters */
	if (rf_has(r_ptr->flags, RF_FRIENDS))
	{
		/* Attempt to place a group */
		(void)place_monster_group(c, y, x, r_idx, slp);
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
			(void)place_monster_one(ny, nx, z, slp);

			/* Place a "group" of escorts if needed */
			if (rf_has(r_info[z].flags, RF_FRIENDS) ||
			    rf_has(r_ptr->flags, RF_ESCORTS))
			{
				/* Place a group of monsters */
				(void)place_monster_group(c, ny, nx, z, slp);
			}
		}
	}


	/* Success */
	return (TRUE);
}


/*
 * Hack -- attempt to place a monster at the given location
 *
 * Attempt to find a monster appropriate to the given depth
 */
bool place_monster(struct cave *c, int y, int x, int depth, bool slp, bool grp)
{
	int r_idx;

	assert(c);

	/* Pick a monster */
	r_idx = get_mon_num(depth);

	/* Handle failure */
	if (!r_idx) return (FALSE);

	/* Attempt to place the monster */
	if (place_monster_aux(c, y, x, r_idx, slp, grp)) return (TRUE);

	/* Oops */
	return (FALSE);
}




/*
 * XXX XXX XXX Player Ghosts are such a hack, they have been completely
 * removed.
 *
 * An idea for reintroducing them is to create a small number of
 * "unique" monsters which will serve as the "player ghosts".
 * Each will have a place holder for the "name" of a deceased player,
 * which will be extracted from a "bone" file, or replaced with a
 * "default" name if a real name is not available.  Each ghost will
 * appear exactly once and will not induce a special feeling.
 *
 * Possible methods:
 *   (s) 1 Skeleton
 *   (z) 1 Zombie
 *   (M) 1 Mummy
 *   (G) 1 Polterguiest, 1 Spirit, 1 Ghost, 1 Shadow, 1 Phantom
 *   (W) 1 Wraith
 *   (V) 1 Vampire, 1 Vampire Lord
 *   (L) 1 Lich
 *
 * Possible change: Lose 1 ghost, Add "Master Lich"
 *
 * Possible change: Lose 2 ghosts, Add "Wraith", Add "Master Lich"
 *
 * Possible change: Lose 4 ghosts, lose 1 vampire lord
 *
 * Note that ghosts should never sleep, should be very attentive, should
 * have maximal hitpoints, drop only good (or great) items, should be
 * cold blooded, evil, undead, immune to poison, sleep, confusion, fear.
 *
 * Base monsters:
 *   Skeleton
 *   Zombie
 *   Mummy
 *   Poltergeist
 *   Spirit
 *   Ghost
 *   Vampire
 *   Wraith
 *   Vampire Lord
 *   Shadow
 *   Phantom
 *   Lich
 *
 * This routine will simply extract ghost names from files, and
 * attempt to allocate a player ghost somewhere in the dungeon,
 * note that normal allocation may also attempt to place ghosts,
 * so we must work with some form of default names.
 *
 * XXX XXX XXX
 */

/*
 * Attempt to allocate a random monster in the dungeon.
 *
 * Place the monster at least "dis" distance from the player.
 *
 * Use "slp" to choose the initial "sleep" status
 *
 * Use "depth" for the monster level
 */
bool alloc_monster(struct cave *c, struct loc loc, int dis, bool slp, int depth)
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
	if (place_monster(c, y, x, depth, slp, TRUE)) return (TRUE);

	/* Nope */
	return (FALSE);
}




/*
 * Hack -- the "type" of the current "summon specific"
 */
static int summon_specific_type = 0;


/*
 * Hack -- help decide if a monster race is "okay" to summon
 */
static bool summon_specific_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	bool okay = FALSE;

	/* Hack -- no specific type specified */
	if (!summon_specific_type) return (TRUE);

	/* Check our requirements */
	switch (summon_specific_type)
	{
		case SUMMON_ANIMAL:
		{
			okay = (rf_has(r_ptr->flags, RF_ANIMAL) &&
			        !rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}

		case SUMMON_SPIDER:
		{
			okay = (r_ptr->d_char == 'S' &&
			        !rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}

		case SUMMON_HOUND:
		{
			okay = ((r_ptr->d_char == 'C' || r_ptr->d_char == 'Z') &&
			        !rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}

		case SUMMON_HYDRA:
		{
			okay = (r_ptr->d_char == 'M' &&
			        !rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}

		case SUMMON_ANGEL:
		{
			okay = (r_ptr->d_char == 'A' &&
			        !flags_test_all(r_ptr->flags, RF_SIZE, RF_UNIQUE,
			                              RF_FRIEND, RF_FRIENDS, RF_ESCORT,
			                              RF_ESCORTS, FLAG_END));
			break;
		}

		case SUMMON_DEMON:
		{
			okay = (rf_has(r_ptr->flags, RF_DEMON) &&
			        !flags_test_all(r_ptr->flags, RF_SIZE, RF_UNIQUE,
			                              RF_FRIEND, RF_FRIENDS, RF_ESCORT,
			                              RF_ESCORTS, FLAG_END));
			break;
		}

		case SUMMON_UNDEAD:
		{
			okay = (rf_has(r_ptr->flags, RF_UNDEAD) &&
			        !flags_test_all(r_ptr->flags, RF_SIZE, RF_UNIQUE,
			                              RF_FRIEND, RF_FRIENDS, RF_ESCORT,
			                              RF_ESCORTS, FLAG_END));
			break;
		}

		case SUMMON_DRAGON:
		{
			okay = (rf_has(r_ptr->flags, RF_DRAGON) &&
			        !flags_test_all(r_ptr->flags, RF_SIZE, RF_UNIQUE,
			                              RF_FRIEND, RF_FRIENDS, RF_ESCORT,
			                              RF_ESCORTS, FLAG_END));
			break;
		}

		case SUMMON_KIN:
		{
			okay = (r_ptr->d_char == summon_kin_type &&
			        !rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}

		case SUMMON_HI_UNDEAD:
		{
			okay = (r_ptr->d_char == 'L' ||
			        r_ptr->d_char == 'V' ||
			        r_ptr->d_char == 'W');
			break;
		}

		case SUMMON_HI_DRAGON:
		{
			okay = (r_ptr->d_char == 'D');
			break;
		}

		case SUMMON_HI_DEMON:
		{
			okay = (r_ptr->d_char == 'U');
			break;
		}

		case SUMMON_WRAITH:
		{
			okay = (r_ptr->d_char == 'W' &&
			        rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}

		case SUMMON_UNIQUE:
		{
			okay = (rf_has(r_ptr->flags, RF_UNIQUE)) ? TRUE : FALSE;
			break;
		}

		case SUMMON_MONSTER:
		{
			okay = (!flags_test_all(r_ptr->flags, RF_SIZE, RF_UNIQUE,
			                              RF_FRIEND, RF_FRIENDS, RF_ESCORT,
			                              RF_ESCORTS, FLAG_END));
			break;
		}

		case SUMMON_MONSTERS:
		{
			okay = (!rf_has(r_ptr->flags, RF_UNIQUE));
			break;
		}
	}

	/* Result */
	return (okay);
}


/*
 * Place a monster (of the specified "type") near the given
 * location.  Return TRUE iff a monster was actually summoned.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: SUMMON_UNIQUE and SUMMON_WRAITH (XXX) will summon Uniques
 * Note: SUMMON_HI_UNDEAD and SUMMON_HI_DRAGON may summon Uniques
 * Note: None of the other summon codes will ever summon Uniques.
 *
 * This function has been changed.  We now take the "monster level"
 * of the summoning monster as a parameter, and use that, along with
 * the current dungeon level, to help determine the level of the
 * desired monster.  Note that this is an upper bound, and also
 * tends to "prefer" monsters of that level.  Currently, we use
 * the average of the dungeon and monster levels, and then add
 * five to allow slight increases in monster power.
 *
 * Note that we use the new "monster allocation table" creation code
 * to restrict the "get_mon_num()" function to the set of "legal"
 * monsters, making this function much faster and more reliable.
 *
 * Note that this function may not succeed, though this is very rare.
 */
bool summon_specific(int y1, int x1, int lev, int type, int delay)
{
	int i, x = 0, y = 0, r_idx;


	/* Look for a location */
	for (i = 0; i < 20; ++i)
	{
		/* Pick a distance */
		int d = (i / 15) + 1;

		/* Pick a location */
		scatter(&y, &x, y1, x1, d, 0);

		/* Require "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Hack -- no summon on glyph of warding */
		if (cave->feat[y][x] == FEAT_GLYPH) continue;

		/* Okay */
		break;
	}

	/* Failure */
	if (i == 20) return (FALSE);


	/* Save the "summon" type */
	summon_specific_type = type;


	/* Require "okay" monsters */
	get_mon_num_hook = summon_specific_okay;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick a monster, using the level calculation */
	r_idx = get_mon_num((p_ptr->depth + lev) / 2 + 5);


	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Handle failure */
	if (!r_idx) return (FALSE);

	/* Attempt to place the monster (awake, allow groups) */
	if (!place_monster_aux(cave, y, x, r_idx, FALSE, TRUE)) return (FALSE);

	/* If delay, try to let the player act before the summoned monsters. */
	/* NOTE: should really be -100, but energy is currently 0-255. */
	if (delay)
		mon_list[cave->m_idx[y][x]].energy = 0;

	/* Success */
	return (TRUE);
}





/*
 * Let the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 */
bool multiply_monster(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];

	int i, y, x;

	bool result = FALSE;

	/* Try up to 18 times */
	for (i = 0; i < 18; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, m_ptr->fy, m_ptr->fx, d, 0);

		/* Require an "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Create a new monster (awake, no groups) */
		result = place_monster_aux(cave, y, x, m_ptr->r_idx, FALSE, FALSE);

		/* Done */
		break;
	}

	/* Result */
	return (result);
}





/*
 * Dump a message describing a monster's reaction to damage
 *
 * Technically should attempt to treat "Beholder"'s as jelly's
 */
void message_pain(int m_idx, int dam)
{
	long oldhp, newhp, tmp;
	int percentage;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	char m_name[80];


	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Notice non-damage */
	if (dam == 0)
	{
		msg("%^s is unharmed.", m_name);
		return;
	}

	/* Note -- subtle fix -CFT */
	newhp = (long)(m_ptr->hp);
	oldhp = newhp + (long)(dam);
	tmp = (newhp * 100L) / oldhp;
	percentage = (int)(tmp);


	/* Jelly's, Mold's, Vortex's, Quthl's */
	if (strchr("jmvQ", r_ptr->d_char))
	{
		if (percentage > 95)
			msg("%^s barely notices.", m_name);
		else if (percentage > 75)
			msg("%^s flinches.", m_name);
		else if (percentage > 50)
			msg("%^s squelches.", m_name);
		else if (percentage > 35)
			msg("%^s quivers in pain.", m_name);
		else if (percentage > 20)
			msg("%^s writhes about.", m_name);
		else if (percentage > 10)
			msg("%^s writhes in agony.", m_name);
		else
			msg("%^s jerks limply.", m_name);
	}

	/* Dogs and Hounds */
	else if (strchr("CZ", r_ptr->d_char))
	{
		if (percentage > 95)
			msg("%^s shrugs off the attack.", m_name);
		else if (percentage > 75)
			msg("%^s snarls with pain.", m_name);
		else if (percentage > 50)
			msg("%^s yelps in pain.", m_name);
		else if (percentage > 35)
			msg("%^s howls in pain.", m_name);
		else if (percentage > 20)
			msg("%^s howls in agony.", m_name);
		else if (percentage > 10)
			msg("%^s writhes in agony.", m_name);
		else
			msg("%^s yelps feebly.", m_name);
	}

	/* One type of monsters (ignore,squeal,shriek) */
	else if (strchr("FIKMRSXabclqrst", r_ptr->d_char))
	{
		if (percentage > 95)
			msg("%^s ignores the attack.", m_name);
		else if (percentage > 75)
			msg("%^s grunts with pain.", m_name);
		else if (percentage > 50)
			msg("%^s squeals in pain.", m_name);
		else if (percentage > 35)
			msg("%^s shrieks in pain.", m_name);
		else if (percentage > 20)
			msg("%^s shrieks in agony.", m_name);
		else if (percentage > 10)
			msg("%^s writhes in agony.", m_name);
		else
			msg("%^s cries out feebly.", m_name);
	}

	/* Another type of monsters (shrug,cry,scream) */
	else
	{
		if (percentage > 95)
			msg("%^s shrugs off the attack.", m_name);
		else if (percentage > 75)
			msg("%^s grunts with pain.", m_name);
		else if (percentage > 50)
			msg("%^s cries out in pain.", m_name);
		else if (percentage > 35)
			msg("%^s screams in pain.", m_name);
		else if (percentage > 20)
			msg("%^s screams in agony.", m_name);
		else if (percentage > 10)
			msg("%^s writhes in agony.", m_name);
		else
			msg("%^s cries out feebly.", m_name);
	}
}


/* XXX Eddie This is ghastly.  The monster should have known_flags similar to in the object_type structure. */
typedef struct {
	int idx;
	int flag;
} learn_attack_struct;

static learn_attack_struct attack_table[] = {
	/* first 14 unused */
	{ 0, FLAG_END },
	{ 1, FLAG_END },
	{ 2, FLAG_END },
	{ 3, FLAG_END },
	{ 4, FLAG_END },
	{ 5, FLAG_END },
	{ 6, FLAG_END },
	{ 7, FLAG_END },
	{ 8, FLAG_END },
	{ 9, FLAG_END },
	{ 10, FLAG_END },
	{ 11, FLAG_END },
	{ 12, FLAG_END },
	{ 13, FLAG_END },
	{ DRS_FREE, OF_FREE_ACT },
	{ DRS_MANA, FLAG_END },
	{ DRS_RES_ACID, OF_RES_ACID },
	{ DRS_RES_ELEC, OF_RES_ELEC },
	{ DRS_RES_FIRE, OF_RES_FIRE },
	{ DRS_RES_COLD, OF_RES_COLD },
	{ DRS_RES_POIS, OF_RES_POIS },
	{ DRS_RES_FEAR, OF_RES_FEAR },
	{ DRS_RES_LIGHT, OF_RES_LIGHT },
	{ DRS_RES_DARK, OF_RES_DARK },
	{ DRS_RES_BLIND, OF_RES_BLIND },
	{ DRS_RES_CONFU, OF_RES_CONFU },
	{ DRS_RES_SOUND, OF_RES_SOUND },
	{ DRS_RES_SHARD, OF_RES_SHARD },
	{ DRS_RES_NEXUS, OF_RES_NEXUS },
	{ DRS_RES_NETHR, OF_RES_NETHR },
	{ DRS_RES_CHAOS, OF_RES_CHAOS },
	{ DRS_RES_DISEN, OF_RES_DISEN },
};

/* XXX Eddie this ought to be as simple as testing visibility and/or intelligence, then or-ing a flag into m_ptr->known_flags */
/*
 * Learn about an "observed" resistance.
 */
void update_smart_learn(int m_idx, int what)
{
	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];


	/* anything a monster might learn, the player should learn */
	assert(what >= 0);
	assert(what < (int)N_ELEMENTS(attack_table));
	assert (attack_table[what].idx == what);
	if (attack_table[what].flag >= FLAG_START)
		wieldeds_notice_flag(attack_table[what].flag);

	/* Not allowed to learn */
	if (!OPT(birth_ai_learn)) return;

	/* Too stupid to learn anything */
	if (rf_has(r_ptr->flags, RF_STUPID)) return;

	/* Not intelligent, only learn sometimes */
	if (!rf_has(r_ptr->flags, RF_SMART) && (randint0(100) < 50)) return;


	/* XXX XXX XXX */

	/* Analyze the knowledge */
	switch (what)
	{
		case DRS_FREE:
		{
			if (p_ptr->state.free_act) m_ptr->smart |= (SM_IMM_FREE);
			break;
		}

		case DRS_MANA:
		{
			if (!p_ptr->msp) m_ptr->smart |= (SM_IMM_MANA);
			break;
		}

		case DRS_RES_ACID:
		{
			if (p_ptr->state.resist_acid) m_ptr->smart |= (SM_RES_ACID);
			if (p_ptr->timed[TMD_OPP_ACID]) m_ptr->smart |= (SM_OPP_ACID);
			if (p_ptr->state.immune_acid) m_ptr->smart |= (SM_IMM_ACID);
			break;
		}

		case DRS_RES_ELEC:
		{
			if (p_ptr->state.resist_elec) m_ptr->smart |= (SM_RES_ELEC);
			if (p_ptr->timed[TMD_OPP_ELEC]) m_ptr->smart |= (SM_OPP_ELEC);
			if (p_ptr->state.immune_elec) m_ptr->smart |= (SM_IMM_ELEC);
			break;
		}

		case DRS_RES_FIRE:
		{
			if (p_ptr->state.resist_fire) m_ptr->smart |= (SM_RES_FIRE);
			if (p_ptr->timed[TMD_OPP_FIRE]) m_ptr->smart |= (SM_OPP_FIRE);
			if (p_ptr->state.immune_fire) m_ptr->smart |= (SM_IMM_FIRE);
			break;
		}

		case DRS_RES_COLD:
		{
			if (p_ptr->state.resist_cold) m_ptr->smart |= (SM_RES_COLD);
			if (p_ptr->timed[TMD_OPP_COLD]) m_ptr->smart |= (SM_OPP_COLD);
			if (p_ptr->state.immune_cold) m_ptr->smart |= (SM_IMM_COLD);
			break;
		}

		case DRS_RES_POIS:
		{
			if (p_ptr->state.resist_pois) m_ptr->smart |= (SM_RES_POIS);
			if (p_ptr->timed[TMD_OPP_POIS]) m_ptr->smart |= (SM_OPP_POIS);
			break;
		}

		case DRS_RES_FEAR:
		{
			if (p_ptr->state.resist_fear) m_ptr->smart |= (SM_RES_FEAR);
			break;
		}

		case DRS_RES_LIGHT:
		{
			if (p_ptr->state.resist_light) m_ptr->smart |= (SM_RES_LIGHT);
			break;
		}

		case DRS_RES_DARK:
		{
			if (p_ptr->state.resist_dark) m_ptr->smart |= (SM_RES_DARK);
			break;
		}

		case DRS_RES_BLIND:
		{
			if (p_ptr->state.resist_blind) m_ptr->smart |= (SM_RES_BLIND);
			break;
		}

		case DRS_RES_CONFU:
		{
			if (p_ptr->state.resist_confu) m_ptr->smart |= (SM_RES_CONFU);
			break;
		}

		case DRS_RES_SOUND:
		{
			if (p_ptr->state.resist_sound) m_ptr->smart |= (SM_RES_SOUND);
			break;
		}

		case DRS_RES_SHARD:
		{
			if (p_ptr->state.resist_shard) m_ptr->smart |= (SM_RES_SHARD);
			break;
		}

		case DRS_RES_NEXUS:
		{
			if (p_ptr->state.resist_nexus) m_ptr->smart |= (SM_RES_NEXUS);
			break;
		}

		case DRS_RES_NETHR:
		{
			if (p_ptr->state.resist_nethr) m_ptr->smart |= (SM_RES_NETHR);
			break;
		}

		case DRS_RES_CHAOS:
		{
			if (p_ptr->state.resist_chaos) m_ptr->smart |= (SM_RES_CHAOS);
			break;
		}

		case DRS_RES_DISEN:
		{
			if (p_ptr->state.resist_disen) m_ptr->smart |= (SM_RES_DISEN);
			break;
		}
	}
}



/*
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



/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that monsters can now carry objects, and when a monster dies,
 * it drops all of its objects, which may disappear in crowded rooms.
 */
void monster_death(int m_idx)
{
	int i, j, y, x, level;

	int dump_item = 0;
	int dump_gold = 0;

	int number = 0;
	int total = 0;

	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool visible = (m_ptr->ml || rf_has(r_ptr->flags, RF_UNIQUE));

	bool great = (rf_has(r_ptr->flags, RF_DROP_GREAT)) ? TRUE : FALSE;
	bool good = (rf_has(r_ptr->flags, RF_DROP_GOOD) ? TRUE : FALSE) || great;

	bool gold_ok = (!rf_has(r_ptr->flags, RF_ONLY_ITEM));
	bool item_ok = (!rf_has(r_ptr->flags, RF_ONLY_GOLD));

	int force_coin = get_coin_type(r_ptr);

	object_type *i_ptr;
	object_type object_type_body;


	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;


	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Copy the object */
		object_copy(i_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

		/* Drop it */
		drop_near(cave, i_ptr, 0, y, x, TRUE);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;


	/* Mega-Hack -- drop "winner" treasures */
	if (rf_has(r_ptr->flags, RF_DROP_CHOSEN))
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Mega-Hack -- Make "Grond" */
		object_prep(i_ptr, objkind_get(TV_HAFTED, SV_GROND), 0, MAXIMISE);
		i_ptr->name1 = ART_GROND;
		apply_magic(i_ptr, 0, TRUE, TRUE, TRUE);

		i_ptr->origin = ORIGIN_DROP;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(cave, i_ptr, 0, y, x, TRUE);


		/* Get local object */
		i_ptr = &object_type_body;

		/* Mega-Hack -- Make "Morgoth" */
		object_prep(i_ptr, objkind_get(TV_CROWN, SV_MORGOTH), 0, MAXIMISE);
		i_ptr->name1 = ART_MORGOTH;
		apply_magic(i_ptr, 0, TRUE, TRUE, TRUE);

		i_ptr->origin = ORIGIN_DROP;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(cave, i_ptr, 0, y, x, TRUE);
	}


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

	/* Drop some objects */
	for (j = 0; j < number; j++)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Make Gold */
		if (gold_ok && (!item_ok || (randint0(100) < 50)))
		{
			/* Make some gold */
			make_gold(i_ptr, level, force_coin);
			dump_gold++;
		}

		/* Make Object */
		else
		{
			/* Make an object */
			if (!make_object(cave, i_ptr, level, good, great)) continue;
			dump_item++;
		}

		/* Set origin */
		i_ptr->origin = visible ? ORIGIN_DROP : ORIGIN_DROP_UNKNOWN;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(cave, i_ptr, 0, y, x, TRUE);
	}

	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
	{
		/* Take notes on treasure */
		lore_treasure(m_idx, dump_item, dump_gold);
	}

	/* Update monster list window */
	p_ptr->redraw |= PR_MONLIST;

	/* Only process "Quest Monsters" */
	if (!rf_has(r_ptr->flags, RF_QUESTOR)) return;

	/* Hack -- Mark quests as complete */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		/* Hack -- note completed quests */
		if (q_list[i].level == r_ptr->level) q_list[i].level = 0;

		/* Count incomplete quests */
		if (q_list[i].level) total++;
	}

	/* Build magical stairs */
	build_quest_stairs(y, x);

	/* Nothing left, game over... */
	if (total == 0)
	{
		/* Total winner */
		p_ptr->total_winner = TRUE;

		/* Redraw the "title" */
		p_ptr->redraw |= (PR_TITLE);

		/* Congratulations */
		msg("*** CONGRATULATIONS ***");
		msg("You have won the game!");
		msg("You may retire (commit suicide) when you are ready.");
	}
}


/*
 * If the monster is asleep, then wake it up. Otherwise, do nothing.
 * Returns TRUE if the monster just woke up, or FALSE if it was already awake.
 */
bool wake_monster(monster_type *m_ptr)
{
	if (m_ptr->csleep <= 0)
		return FALSE;

	m_ptr->csleep = 0;

	/* If it just woke up, update the monster list */
	p_ptr->redraw |= PR_MONLIST;
	
	return TRUE;
}


/*
 * Decrease a monster's hit points, handle monster death.
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
 */
bool mon_take_hit(int m_idx, int dam, bool *fear, const char * note)
{
	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	s32b div, new_exp, new_exp_frac;


	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

	/* Wake it up */
	wake_monster(m_ptr);

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
			/* Mega-Hack -- Morgoth -- see monster_death() */
			if (rf_has(r_ptr->flags, RF_DROP_CHOSEN))
				soundfx = MSG_KILL_KING;
			else
				soundfx = MSG_KILL_UNIQUE;
		}

		/* Extract monster name */
		monster_desc(m_name, sizeof(m_name), m_ptr, 0);

		/* Death by Missile/Spell attack */
		if (note)
		{
			msgt(soundfx, "%^s%s", m_name, note);
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
		gain_exp(new_exp);

		/* Generate treasure */
		monster_death(m_idx);

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
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint1(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->monfear && !rf_has(r_ptr->flags, RF_NO_FEAR) && dam > 0)
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
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* Hack -- Add some timed fear */
			m_ptr->monfear = (randint1(10) +
			                  (((dam >= m_ptr->hp) && (percentage > 7)) ?
			                   20 : ((11 - percentage) * 5)));
		}
	}


	/* Not dead yet */
	return (FALSE);
}

/*
 * Obtain the "flags" for a monster race which are known to the monster
 * lore struct.  Known flags will be 1 for present, or 0 for not present.
 * Unknown flags will always be 0.
 */
void monster_flags_known(const monster_race *r_ptr, const monster_lore *l_ptr, bitflag flags[RF_SIZE])
{
	rf_copy(flags, r_ptr->flags);
	rf_inter(flags, l_ptr->flags);
}
