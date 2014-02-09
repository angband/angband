/*
 * File: quest.c
 * Purpose: All quest-related code
 *
 * Copyright (c) 2013 Angband developers
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
#include "quest.h"

/*
 * Array[MAX_Q_IDX] of quests
 */
quest q_list[MAX_Q_IDX];

/*
 * Check if the given level is a quest level.
 */
bool is_quest(int level)
{
	size_t i;

	/* Town is never a quest */
	if (!level) return FALSE;

	for (i = 0; i < N_ELEMENTS(q_list); i++)
		if (q_list[i].level == level)
			return TRUE;

	return FALSE;
}

/*
 * Wipe all quests, add back in Sauron and Morgoth
 */
void quest_reset(void) {
	size_t i;

	for (i = 0; i < N_ELEMENTS(q_list); i++)
		q_list[i].level = 0;

	q_list[0].level = 99;
	q_list[1].level = 100;
}

/*
 * Creates magical stairs after finishing a quest monster.
 */
static void build_quest_stairs(int y, int x)
{
	int ny, nx;

	/* Stagger around */
	while (!cave_valid_bold(y, x) && !cave_iswall(cave, y, x) && !cave_isdoor(cave, y, x)) {
		/* Pick a location */
		scatter(&ny, &nx, y, x, 1, FALSE);

		/* Stagger */
		y = ny; x = nx;
	}

	/* Push any objects */
	push_object(y, x);

	/* Explain the staircase */
	msg("A magical staircase appears...");

	/* Create stairs down */
	/* XXX: fake depth = 0 to always produce downstairs */
	cave_add_stairs(cave, y, x, 0);

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
}

/*
 * Check if this (now dead) monster is a quest monster, and act appropriately
 */
bool quest_check(const struct monster *m) {
	size_t i;
	int total = 0;

	/* Don't bother with non-questors */
	if (!rf_has(m->race->flags, RF_QUESTOR)) return FALSE;

	/* Mark quests as complete */
	for (i = 0; i < N_ELEMENTS(q_list); i++) {
		/* Note completed quests */
		if (q_list[i].level == m->race->level) q_list[i].level = 0;

		/* Count incomplete quests */
		if (q_list[i].level) total++;
	}

	/* Build magical stairs */
	build_quest_stairs(m->fy, m->fx);

	/* Nothing left, game over... */
	if (total == 0) {
		p_ptr->total_winner = TRUE;
		p_ptr->redraw |= (PR_TITLE);
		msg("*** CONGRATULATIONS ***");
		msg("You have won the game!");
		msg("You may retire (commit suicide) when you are ready.");
	}

	return TRUE;
}

/*
 * Initialise/free the quest list.
 *
 * This used to dynamically allocate an array of length 4, but
 * now it just makes sure the existing one is clear.
 */
void quest_init(void) {
	memset(q_list, 0, sizeof q_list);
	return;
}

void quest_free(void) {
	return;
}
