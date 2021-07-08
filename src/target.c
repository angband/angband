/**
 * \file target.c
 * \brief Targetting code
 *
 * Copyright (c) 1997-2007 Angband contributors
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
#include "cmd-core.h"
#include "game-input.h"
#include "mon-desc.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-ignore.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "project.h"
#include "target.h"

/**
 * Is the target set?
 */
static bool target_set;

/**
 * Is the target fixed (for the duration of a spell)?
 */
static bool target_fixed;

/**
 * Player target
 */
static struct target target;

/**
 * Old player target
 */
static struct target old_target;

/**
 * Monster health description
 */
void look_mon_desc(char *buf, size_t max, int m_idx)
{
	struct monster *mon = cave_monster(cave, m_idx);

	bool living = true;

	if (!mon) return;

	/* Determine if the monster is "living" (vs "undead") */
	if (monster_is_destroyed(mon)) living = false;

	/* Assess health */
	if (mon->hp >= mon->maxhp) {
		/* No damage */
		my_strcpy(buf, (living ? "unhurt" : "undamaged"), max);
	} else {
		/* Calculate a health "percentage" */
		int perc = 100L * mon->hp / mon->maxhp;

		if (perc >= 60)
			my_strcpy(buf, (living ? "somewhat wounded" : "somewhat damaged"),
					  max);
		else if (perc >= 25)
			my_strcpy(buf, (living ? "wounded" : "damaged"), max);
		else if (perc >= 10)
			my_strcpy(buf, (living ? "badly wounded" : "badly damaged"), max);
		else
			my_strcpy(buf, (living ? "almost dead" : "almost destroyed"), max);
	}

	/* Effect status */
	if (mon->m_timed[MON_TMD_SLEEP]) my_strcat(buf, ", asleep", max);
	if (mon->m_timed[MON_TMD_HOLD]) my_strcat(buf, ", held", max);
	if (mon->m_timed[MON_TMD_DISEN]) my_strcat(buf, ", disenchanted", max);
	if (mon->m_timed[MON_TMD_CONF]) my_strcat(buf, ", confused", max);
	if (mon->m_timed[MON_TMD_FEAR]) my_strcat(buf, ", afraid", max);
	if (mon->m_timed[MON_TMD_STUN]) my_strcat(buf, ", stunned", max);
	if (mon->m_timed[MON_TMD_SLOW]) my_strcat(buf, ", slowed", max);
	if (mon->m_timed[MON_TMD_FAST]) my_strcat(buf, ", hasted", max);
}



/**
 * Determine if a monster makes a reasonable target
 *
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 */
bool target_able(struct monster *m)
{
	return m && m->race && monster_is_obvious(m) &&
		projectable(cave, player->grid, m->grid, PROJECT_NONE) &&
		!player->timed[TMD_IMAGE];
}



/**
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return true if the target is "okay" and false otherwise.
 */
bool target_okay(void)
{
	/* No target */
	if (!target_set) return false;

	/* Check "monster" targets */
	if (target.midx > 0) {
		struct monster *mon = cave_monster(cave, target.midx);
		if (target_able(mon)) {
			/* Get the monster location */
			target.grid = mon->grid;

			/* Good target */
			return true;
		}
	} else if (target.grid.x && target.grid.y) {
		/* Allow a direction without a monster */
		return true;
	}

	/* Assume no target */
	return false;
}


/**
 * Set the target to a monster (or nobody); if target is fixed, don't unset
 */
bool target_set_monster(struct monster *mon)
{
	/* Acceptable target */
	if (mon && target_able(mon)) {
		target_set = true;
		target.midx = mon->midx;
		target.grid = mon->grid;
		return true;
	} else if (target_fixed) {
		/* If a monster has died during a spell, this maintains its grid as
		 * the target in case further effects of the spell need it */
		target.midx = 0;
		return true;
	}

	/* Reset target info */
	target_set = false;
	target.midx = 0;
	target.grid.y = 0;
	target.grid.x = 0;

	return false;
}


/**
 * Set the target to a location
 */
void target_set_location(int y, int x)
{
	struct loc grid = loc(x, y);

	/* Legal target */
	if (square_in_bounds_fully(cave, grid)) {
		/* Save target info */
		target_set = true;
		target.midx = 0;
		target.grid = grid;
		return;
	}

	/* Reset target info */
	target_set = false;
	target.midx = 0;
	target.grid.y = 0;
	target.grid.x = 0;
}

/**
 * Tell the UI the target is set
 */
bool target_is_set(void)
{
	return target_set;
}

/**
 * Fix the target
 */
void target_fix(void)
{
	old_target = target;
	target_fixed = true;
}

/**
 * Release the target
 */
void target_release(void)
{
	target_fixed = false;

	/* If the old target is a now-dead monster, cancel it */
	if (old_target.midx != 0) {
		struct monster *mon = cave_monster(cave, old_target.midx);
		if (!mon || !mon->race || !monster_is_in_view(mon)) {
			target.grid.y = 0;
			target.grid.x = 0;
		}
	}
}

/**
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 */
int cmp_distance(const void *a, const void *b)
{
	int py = player->grid.y;
	int px = player->grid.x;

	const struct loc *pa = a;
	const struct loc *pb = b;

	int da, db, kx, ky;

	/* Absolute distance components */
	kx = pa->x; kx -= px; kx = ABS(kx);
	ky = pa->y; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Absolute distance components */
	kx = pb->x; kx -= px; kx = ABS(kx);
	ky = pb->y; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Compare the distances */
	if (da < db)
		return -1;
	if (da > db)
		return 1;
	return 0;
}

/**
 * Help select a location.  This function picks the closest from a set in 
 *(roughly) a given direction.
 */
s16b target_pick(int y1, int x1, int dy, int dx, struct point_set *targets)
{
	int i, v;

	int x2, y2, x3, y3, x4, y4;

	int b_i = -1, b_v = 9999;


	/* Scan the locations */
	for (i = 0; i < point_set_size(targets); i++) {
		/* Point 2 */
		x2 = targets->pts[i].x;
		y2 = targets->pts[i].y;

		/* Directed distance */
		x3 = (x2 - x1);
		y3 = (y2 - y1);

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		x4 = ABS(x3);
		y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i; b_v = v;
	}

	/* Result */
	return (b_i);
}


/**
 * Determine if a given location is "interesting"
 */
bool target_accept(int y, int x)
{
	struct loc grid = loc(x, y);
	struct object *obj;

	/* Player grids are always interesting */
	if (square(cave, grid).mon < 0) return true;

	/* Handle hallucination */
	if (player->timed[TMD_IMAGE]) return false;

	/* Obvious monsters */
	if (square(cave, grid).mon > 0) {
		struct monster *mon = square_monster(cave, grid);
		if (monster_is_obvious(mon)) {
			return true;
		}
	}

	/* Traps */
	if (square_isvisibletrap(cave, grid)) return true;

	/* Scan all objects in the grid */
	for (obj = square_object(player->cave, grid); obj; obj = obj->next) {
		/* Memorized object */
		if ((obj->kind == unknown_item_kind) || !ignore_known_item_ok(obj)) {
			return true;
		}
	}

	/* Interesting memorized features */
	if (square_isknown(cave, grid) && square_isinteresting(cave, grid)) {
		return true;
	}

	/* Nope */
	return false;
}

/**
 * Describe a location relative to the player position.
 * e.g. "12 S 35 W" or "0 N, 33 E" or "0 N, 0 E"
 */
void coords_desc(char *buf, int size, int y, int x)
{
	const char *east_or_west;
	const char *north_or_south;

	int py = player->grid.y;
	int px = player->grid.x;

	if (y > py)
		north_or_south = "S";
	else
		north_or_south = "N";

	if (x < px)
		east_or_west = "W";
	else
		east_or_west = "E";

	strnfmt(buf, size, "%d %s, %d %s",
		ABS(y - py), north_or_south, ABS(x-px), east_or_west);
}

/**
 * Obtains the location the player currently targets.
 */
void target_get(struct loc *grid)
{
	assert(grid);
	*grid = target.grid;
}


/**
 * Returns the currently targeted monster index.
 */
struct monster *target_get_monster(void)
{
	return cave_monster(cave, target.midx);
}


/**
 * True if the player's current target is in LOS.
 */
bool target_sighted(void)
{
	return target_okay() &&
			panel_contains(target.grid.y, target.grid.x) &&
			 /* either the target is a grid and is visible, or it is a monster
			  * that is visible */
		((!target.midx && square_isseen(cave, target.grid)) ||
		 (target.midx && monster_is_visible(cave_monster(cave, target.midx))));
}


#define TS_INITIAL_SIZE	20

/**
 * Return a target set of target_able monsters.
 */
struct point_set *target_get_monsters(int mode, monster_predicate pred)
{
	int y, x;
	int min_y, min_x, max_y, max_x;
	struct point_set *targets = point_set_new(TS_INITIAL_SIZE);

	/* Get the current panel */
	get_panel(&min_y, &min_x, &max_y, &max_x);

	/* Scan for targets */
	for (y = min_y; y < max_y; y++) {
		for (x = min_x; x < max_x; x++) {
			struct loc grid = loc(x, y);

			/* Check bounds */
			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Require "interesting" contents */
			if (!target_accept(y, x)) continue;

			/* Special mode */
			if (mode & (TARGET_KILL)) {
				struct monster *mon = square_monster(cave, grid);

				/* Must contain a monster */
				if (mon == NULL) continue;

				/* Must be a targettable monster */
				if (!target_able(mon)) continue;

				/* Must be the right sort of monster */
				if (pred && !pred(mon)) continue;
			}

			/* Save the location */
			add_to_point_set(targets, grid);
		}
	}

	sort(targets->pts, point_set_size(targets), sizeof(*(targets->pts)),
		 cmp_distance);
	return targets;
}


/**
 * Set target to closest monster.
 */
bool target_set_closest(int mode, monster_predicate pred)
{
	struct monster *mon;
	char m_name[80];
	struct point_set *targets;

	/* Cancel old target */
	target_set_monster(NULL);

	/* Get ready to do targetting */
	targets = target_get_monsters(mode, pred);

	/* If nothing was prepared, then return */
	if (point_set_size(targets) < 1) {
		msg("No Available Target.");
		point_set_dispose(targets);
		return false;
	}

	/* Find the first monster in the queue */
	mon = square_monster(cave, targets->pts[0]);
	
	/* Target the monster, if possible */
	if (!target_able(mon)) {
		msg("No Available Target.");
		point_set_dispose(targets);
		return false;
	}

	/* Target the monster */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_CAPITAL);
	if (!(mode & TARGET_QUIET))
		msg("%s is targeted.", m_name);

	/* Set up target information */
	monster_race_track(player->upkeep, mon->race);
	health_track(player->upkeep, mon);
	target_set_monster(mon);

	point_set_dispose(targets);
	return true;
}
