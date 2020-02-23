/**
 * \file mon-move.c
 * \brief Monster movement
 *
 * Monster AI affecting movement and spells, process a monster 
 * (with spells and actions of all kinds, reproduction, effects of any 
 * terrain on monster movement, picking up and destroying objects), 
 * process all monsters.
 *
 * Copyright (c) 1997 Ben Harrison, David Reeve Sward, Keldon Jones.
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
#include "game-world.h"
#include "init.h"
#include "monster.h"
#include "mon-attack.h"
#include "mon-desc.h"
#include "mon-group.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "mon-timed.h"
#include "obj-desc.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "trap.h"


/**
 * ------------------------------------------------------------------------
 * Routines to enable decisions on monster behaviour
 * ------------------------------------------------------------------------ */
/**
 * From Will Asher in DJA:
 * Find whether a monster is near a permanent wall
 *
 * this decides whether PASS_WALL & KILL_WALL monsters use the monster flow code
 */
static bool monster_near_permwall(const struct monster *mon, struct chunk *c)
{
	int y, x;
	int my = mon->grid.y;
	int mx = mon->grid.x;

	/* If player is in LOS, there's no need to go around walls */
    if (projectable(c, mon->grid, player->grid, PROJECT_SHORT))
		return false;

    /* PASS_WALL & KILL_WALL monsters occasionally flow for a turn anyway */
    if (randint0(99) < 5) return true;

	/* Search the nearby grids, which are always in bounds */
	for (y = (my - 2); y <= (my + 2); y++) {
		for (x = (mx - 2); x <= (mx + 2); x++) {
			struct loc grid = loc(x, y);
           if (!square_in_bounds_fully(c, grid)) continue;
            if (square_isperm(c, grid)) return true;
		}
	}
	return false;
}

/**
 * Check if the monster can hear anything
 */
static bool monster_can_hear(struct chunk *c, struct monster *mon)
{
	int base_hearing = mon->race->hearing
		- player->state.skills[SKILL_STEALTH] / 3;
	if (c->noise.grids[mon->grid.y][mon->grid.x] == 0) {
		return false;
	}
	return base_hearing > c->noise.grids[mon->grid.y][mon->grid.x];
}

/**
 * Check if the monster can smell anything
 */
static bool monster_can_smell(struct chunk *c, struct monster *mon)
{
	if (c->scent.grids[mon->grid.y][mon->grid.x] == 0) {
		return false;
	}
	return mon->race->smell > c->scent.grids[mon->grid.y][mon->grid.x];
}

/**
 * Compare the "strength" of two monsters XXX XXX XXX
 */
static int compare_monsters(const struct monster *mon1,
							const struct monster *mon2)
{
	u32b mexp1 = mon1->race->mexp;
	u32b mexp2 = mon2->race->mexp;

	/* Compare */
	if (mexp1 < mexp2) return (-1);
	if (mexp1 > mexp2) return (1);

	/* Assume equal */
	return (0);
}

/**
 * Check if the monster can kill any monster on the relevant grid
 */
static bool monster_can_kill(struct chunk *c, struct monster *mon,
							 struct loc grid)
{
	struct monster *mon1 = square_monster(c, grid);

	/* No monster */
	if (!mon1) return true;

	if (rf_has(mon->race->flags, RF_KILL_BODY) &&
		compare_monsters(mon, mon1) > 0) {
		return true;
	}

	return false;
}

/**
 * Check if the monster can move any monster on the relevant grid
 */
static bool monster_can_move(struct chunk *c, struct monster *mon,
							 struct loc grid)
{
	struct monster *mon1 = square_monster(c, grid);

	/* No monster */
	if (!mon1) return true;

	if (rf_has(mon->race->flags, RF_MOVE_BODY) &&
		compare_monsters(mon, mon1) > 0) {
		return true;
	}

	return false;
}

/**
 * Check if the monster can occupy a grid safely
 */
static bool monster_hates_grid(struct chunk *c, struct monster *mon,
							   struct loc grid)
{
	/* Only some creatures can handle damaging terrain */
	if (square_isdamaging(c, grid) &&
		!rf_has(mon->race->flags, square_feat(c, grid)->resist_flag)) {
		return true;
	}
	return false;
}

/**
 * ------------------------------------------------------------------------
 * Monster movement routines
 * These routines, culminating in get_move(), choose if and where a monster
 * will move on its turn
 * ------------------------------------------------------------------------ */
/**
 * Calculate minimum and desired combat ranges.  -BR-
 *
 * Afraid monsters will set this to their maximum flight distance.
 * Currently this is recalculated every turn - if it becomes a significant
 * overhead it could be calculated only when something has changed (monster HP,
 * chance of escaping, etc)
 */
static void get_move_find_range(struct monster *mon)
{
	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

	/* Monsters will run up to z_info->flee_range grids out of sight */
	int flee_range = z_info->max_sight + z_info->flee_range;

	/* All "afraid" monsters will run away */
	if (mon->m_timed[MON_TMD_FEAR] || rf_has(mon->race->flags, RF_FRIGHTENED)) {
		mon->min_range = flee_range;
	} else if (mon->group_info[PRIMARY_GROUP].role == MON_GROUP_BODYGUARD) {
		/* Bodyguards don't flee */
		mon->min_range = 1;
	} else {
		/* Minimum distance - stay at least this far if possible */
		mon->min_range = 1;

		/* Taunted monsters just want to get in your face */
		if (player->timed[TMD_TAUNT]) return;

		/* Examine player power (level) */
		p_lev = player->lev;

		/* Hack - increase p_lev based on specialty abilities */

		/* Examine monster power (level plus morale) */
		m_lev = mon->race->level + (mon->midx & 0x08) + 25;

		/* Simple cases first */
		if (m_lev + 3 < p_lev) {
			mon->min_range = flee_range;
		} else if (m_lev - 5 < p_lev) {

			/* Examine player health */
			p_chp = player->chp;
			p_mhp = player->mhp;

			/* Examine monster health */
			m_chp = mon->hp;
			m_mhp = mon->maxhp;

			/* Prepare to optimize the calculation */
			p_val = (p_lev * p_mhp) + (p_chp << 2);	/* div p_mhp */
			m_val = (m_lev * m_mhp) + (m_chp << 2);	/* div m_mhp */

			/* Strong players scare strong monsters */
			if (p_val * m_mhp > m_val * p_mhp)
				mon->min_range = flee_range;
		}
	}

	if (mon->min_range < flee_range) {
		/* Creatures that don't move never like to get too close */
		if (rf_has(mon->race->flags, RF_NEVER_MOVE))
			mon->min_range += 3;

		/* Spellcasters that don't strike never like to get too close */
		if (rf_has(mon->race->flags, RF_NEVER_BLOW))
			mon->min_range += 3;
	}

	/* Maximum range to flee to */
	if (!(mon->min_range < flee_range)) {
		mon->min_range = flee_range;
	} else if (mon->cdis < z_info->turn_range) {
		/* Nearby monsters won't run away */
		mon->min_range = 1;
	}

	/* Now find preferred range */
	mon->best_range = mon->min_range;

	/* Archers are quite happy at a good distance */
	if (monster_loves_archery(mon)) {
		mon->best_range += 3;
	}

	if (mon->race->freq_spell > 24) {
		/* Breathers like point blank range */
		if (monster_breathes(mon) && (mon->hp > mon->maxhp / 2)) {
			mon->best_range = MAX(6, mon->best_range);
		} else {
			/* Other spell casters will sit back and cast */
			mon->best_range += 3;
		}
	}
}

/**
 * Choose the best direction for a bodyguard.
 *
 * The idea is to stay close to the group leader, but attack the player if the
 * chance arises
 */
static bool get_move_bodyguard(struct chunk *c, struct monster *mon)
{
	int i;
	struct monster *leader = monster_group_leader(c, mon);
	int dist;
	struct loc best;
	bool found = false;

	if (!leader) return false;

	/* Get distance */
	dist = distance(mon->grid, leader->grid);

	/* If currently adjacent to the leader, we can afford a move */
	if (dist <= 1) return false;

	/* If the leader's too out of sight and far away, save yourself */
	if (!los(cave, mon->grid, leader->grid) && (dist > 10)) return false;

	/* Check nearby adjacent grids and assess */
	for (i = 0; i < 8; i++) {
		/* Get the location */
		struct loc grid = loc_sum(mon->grid, ddgrid_ddd[i]);
		int new_dist = distance(grid, leader->grid);
		int char_dist = distance(grid, player->grid);

		/* Bounds check */
		if (!square_in_bounds(c, grid)) {
			continue;
		}

		/* There's a monster blocking that we can't deal with */
		if (!monster_can_kill(c, mon, grid) && !monster_can_move(c, mon, grid)){
			continue;
		}

		/* There's damaging terrain */
		if (monster_hates_grid(c, mon, grid)) {
			continue;
		}

		/* Closer to the leader is always better */
		if (new_dist < dist) {
			best = grid;
			found = true;
			/* If there's a grid that's also closer to the player, that wins */
			if (char_dist < mon->cdis) {
				break;
			}
		}
	}

	/* If we found one, set the target */
	if (found) {
		mon->target.grid = best;
		return true;
	}

	return false;
}


/**
 * Choose the best direction to advance toward the player, using sound or scent.
 *
 * Ghosts and rock-eaters generally just head straight for the player. Other
 * monsters try sight, then current sound as saved in c->noise.grids[y][x],
 * then current scent as saved in c->scent.grids[y][x].
 *
 * This function assumes the monster is moving to an adjacent grid, and so the
 * noise can be louder by at most 1.  The monster target grid set by sound or
 * scent tracking in this function will be a grid they can step to in one turn,
 * so is the preferred option for get_move() unless there's some reason
 * not to use it.
 *
 * Tracking by 'scent' means that monsters end up near enough the player to
 * switch to 'sound' (noise), or they end up somewhere the player left via 
 * teleport.  Teleporting away from a location will cause the monsters who
 * were chasing the player to converge on that location as long as the player
 * is still near enough to "annoy" them without being close enough to chase
 * directly.
 */
static bool get_move_advance(struct chunk *c, struct monster *mon, bool *track)
{
	int i;
	struct loc decoy = cave_find_decoy(c);
	struct loc target = loc_is_zero(decoy) ? player->grid : decoy;

	int base_hearing = mon->race->hearing
		- player->state.skills[SKILL_STEALTH] / 3;
	int current_noise = base_hearing - c->noise.grids[mon->grid.y][mon->grid.x];
	int best_scent = 0;

	struct loc best_grid;
	struct loc backup_grid;
	bool found = false;
	bool found_backup = false;

	/* Bodyguards are special */
	if (mon->group_info[PRIMARY_GROUP].role == MON_GROUP_BODYGUARD) {
		if (get_move_bodyguard(c, mon)) {
			return true;
		}
	}

	/* If the monster can pass through nearby walls, do that */
	if (monster_passes_walls(mon) && !monster_near_permwall(mon, c)) {
		mon->target.grid = target;
		return true;
	}

	/* If the player can see monster, set target and run towards them */
	if (square_isview(c, mon->grid)) {
		mon->target.grid = target;
		return true;
	}

	/* Check nearby sound, giving preference to the cardinal directions */
	for (i = 0; i < 8; i++) {
		/* Get the location */
		struct loc grid = loc_sum(mon->grid, ddgrid_ddd[i]);
		int heard_noise = base_hearing - c->noise.grids[grid.y][grid.x];

		/* Bounds check */
		if (!square_in_bounds(c, grid)) {
			continue;
		}

		/* Must be some noise */
		if (c->noise.grids[grid.y][grid.x] == 0) {
			continue;
		}

		/* There's a monster blocking that we can't deal with */
		if (!monster_can_kill(c, mon, grid) && !monster_can_move(c, mon, grid)){
			continue;
		}

		/* There's damaging terrain */
		if (monster_hates_grid(c, mon, grid)) {
			continue;
		}

		/* If it's better than the current noise, choose this direction */
		if (heard_noise > current_noise) {
			best_grid = grid;
			found = true;
			break;
		} else if (heard_noise == current_noise) {
			/* Possible move if we can't actually get closer */
			backup_grid = grid;
			found_backup = true;
			continue;
		}
	}

	/* If no good sound, use scent */
	if (!found) {
		for (i = 0; i < 8; i++) {
			/* Get the location */
			struct loc grid = loc_sum(mon->grid, ddgrid_ddd[i]);
			int smelled_scent;

			/* If no good sound yet, use scent */
			smelled_scent = mon->race->smell - c->scent.grids[grid.y][grid.x];
			if ((smelled_scent > best_scent) &&
				(c->scent.grids[grid.y][grid.x] != 0)) {
				best_scent = smelled_scent;
				best_grid = grid;
				found = true;
			}
		}
	}

	/* Set the target */
	if (found) {
		mon->target.grid = best_grid;
		*track = true;
		return true;
	} else if (found_backup) {
		/* Move around to try and improve position */
		mon->target.grid = backup_grid;
		*track = true;
		return true;
	}

	/* No reason to advance */
	return false;
}

/**
 * Choose a "safe" location near a monster for it to run toward.
 *
 * A location is "safe" if it can be reached quickly and the player
 * is not able to fire into it (it isn't a "clean shot").  So, this will
 * cause monsters to "duck" behind walls.  Hopefully, monsters will also
 * try to run towards corridor openings if they are in a room.
 *
 * This function may take lots of CPU time if lots of monsters are fleeing.
 *
 * Return true if a safe location is available.
 */
static bool get_move_find_safety(struct chunk *c, struct monster *mon)
{
	int i, dy, dx, d, dis, gdis = 0;

	const int *y_offsets;
	const int *x_offsets;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++) {
		struct loc best = loc(0, 0);

		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i]) {
			struct loc grid = loc_sum(mon->grid, loc(dx, dy));

			/* Skip illegal locations */
			if (!square_in_bounds_fully(c, grid)) continue;

			/* Skip locations in a wall */
			if (!square_ispassable(c, grid)) continue;

			/* Ignore too-distant grids */
			if (c->noise.grids[grid.y][grid.x] >
				c->noise.grids[mon->grid.y][mon->grid.x] + 2 * d)
				continue;

			/* Ignore damaging terrain if they can't handle it */
			if (square_isdamaging(c, grid) &&
				!rf_has(mon->race->flags, square_feat(c, grid)->resist_flag))
				continue;

			/* Check for absence of shot (more or less) */
			if (!square_isview(c, grid)) {
				/* Calculate distance from player */
				dis = distance(grid, player->grid);

				/* Remember if further than previous */
				if (dis > gdis) {
					best = grid;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis > 0) {
			/* Good location */
			mon->target.grid = best;
			return (true);
		}
	}

	/* No safe place */
	return (false);
}

/**
 * Choose a good hiding place near a monster for it to run toward.
 *
 * Pack monsters will use this to "ambush" the player and lure him out
 * of corridors into open space so they can swarm him.
 *
 * Return true if a good location is available.
 */
static bool get_move_find_hiding(struct chunk *c, struct monster *mon)
{
	int i, dy, dx, d, dis, gdis = 999, min;

	const int *y_offsets, *x_offsets;

	/* Closest distance to get */
	min = distance(player->grid, mon->grid) * 3 / 4 + 2;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++) {
		struct loc best = loc(0, 0);

		/* Get the lists of points with a distance d from monster */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i]) {
			struct loc grid = loc_sum(mon->grid, loc(dx, dy));

			/* Skip illegal locations */
			if (!square_in_bounds_fully(c, grid)) continue;

			/* Skip occupied locations */
			if (!square_isempty(c, grid)) continue;

			/* Check for hidden, available grid */
			if (!square_isview(c, grid) &&
				projectable(c, mon->grid, grid, PROJECT_STOP)) {
				/* Calculate distance from player */
				dis = distance(grid, player->grid);

				/* Remember if closer than previous */
				if (dis < gdis && dis >= min) {
					best = grid;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis < 999) {
			/* Good location */
			mon->target.grid = best;
			return (true);
		}
	}

	/* No good place */
	return (false);
}

/**
 * Provide a location to flee to, but give the player a wide berth.
 *
 * A monster may wish to flee to a location that is behind the player,
 * but instead of heading directly for it, the monster should "swerve"
 * around the player so that it has a smaller chance of getting hit.
 */
static bool get_move_flee(struct chunk *c, struct monster *mon)
{
	int i;
	struct loc best = loc(0, 0);
	int best_score = -1;

	/* Taking damage from terrain makes moving vital */
	if (!monster_taking_terrain_damage(mon)) {
		/* If the player is not currently near the monster, no reason to flow */
		if (mon->cdis >= mon->best_range) {
			return false;
		}

		/* Monster is too far away to use sound or scent */
		if (!monster_can_hear(c, mon) && !monster_can_smell(c, mon)) {
			return false;
		}
	}

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--) {
		int dis, score;

		/* Get the location */
		struct loc grid = loc_sum(mon->grid, ddgrid_ddd[i]);

		/* Bounds check */
		if (!square_in_bounds(c, grid)) continue;

		/* Calculate distance of this grid from our target */
		dis = distance(grid, mon->target.grid);

		/* Score this grid
		 * First half of calculation is inversely proportional to distance
		 * Second half is inversely proportional to grid's distance from player
		 */
		score = 5000 / (dis + 3) - 500 / (c->noise.grids[grid.y][grid.x] + 1);

		/* No negative scores */
		if (score < 0) score = 0;

		/* Ignore lower scores */
		if (score < best_score) continue;

		/* Save the score */
		best_score = score;

		/* Save the location */
		best = grid;
	}

	/* Set the immediate target */
	mon->target.grid = best;

	/* Success */
	return true;
}

/**
 * Choose the basic direction of movement, and whether to bias left or right
 * if the main direction is blocked.
 *
 * Note that the input is an offset to the monster's current position, and
 * the output direction is intended as an index into the side_dirs array.
 */
static int get_move_choose_direction(struct loc offset)
{
	int dir = 0;
	int dx = offset.x, dy = offset.y;

	/* Extract the "absolute distances" */
	int ay = ABS(dy);
	int ax = ABS(dx);

	/* We mostly want to move vertically */
	if (ay > (ax * 2)) {
		/* Choose between directions '8' and '2' */
		if (dy > 0) {
			/* We're heading down */
			dir = 2;
			if ((dx > 0) || (dx == 0 && turn % 2 == 0))
				dir += 10;
		} else {
			/* We're heading up */
			dir = 8;
			if ((dx < 0) || (dx == 0 && turn % 2 == 0))
				dir += 10;
		}
	}

	/* We mostly want to move horizontally */
	else if (ax > (ay * 2)) {
		/* Choose between directions '4' and '6' */
		if (dx > 0) {
			/* We're heading right */
			dir = 6;
			if ((dy < 0) || (dy == 0 && turn % 2 == 0))
				dir += 10;
		} else {
			/* We're heading left */
			dir = 4;
			if ((dy > 0) || (dy == 0 && turn % 2 == 0))
				dir += 10;
		}
	}

	/* We want to move down and sideways */
	else if (dy > 0) {
		/* Choose between directions '1' and '3' */
		if (dx > 0) {
			/* We're heading down and right */
			dir = 3;
			if ((ay < ax) || (ay == ax && turn % 2 == 0))
				dir += 10;
		} else {
			/* We're heading down and left */
			dir = 1;
			if ((ay > ax) || (ay == ax && turn % 2 == 0))
				dir += 10;
		}
	}

	/* We want to move up and sideways */
	else {
		/* Choose between directions '7' and '9' */
		if (dx > 0) {
			/* We're heading up and right */
			dir = 9;
			if ((ay > ax) || (ay == ax && turn % 2 == 0))
				dir += 10;
		} else {
			/* We're heading up and left */
			dir = 7;
			if ((ay < ax) || (ay == ax && turn % 2 == 0))
				dir += 10;
		}
	}

	return dir;
}

/**
 * Choose "logical" directions for monster movement
 *
 * This function is responsible for deciding where the monster wants to move,
 * and so is the core of monster "AI".
 *
 * First, we work out how best to advance toward the player:
 * - Try to head toward the player directly if we can pass through walls or
 *   if we can see them
 * - Failing that follow the player by sound, or failing that by scent
 * - If none of that works, just head in the general direction
 * Then we look at possible reasons not to just advance:
 * - If we're part of a pack, try to lure the player into the open
 * - If we're afraid, try to find a safe place to run to, and if no safe place
 *   just run in the opposite direction to the advance move
 * - If we can see the player and we're part of a group, try and surround them
 *
 * The function then returns false if we're already where we want to be, and
 * otherwise sets the chosen direction to step and returns true.
 */
static bool get_move(struct chunk *c, struct monster *mon, int *dir, bool *good)
{
	struct loc decoy = cave_find_decoy(c);
	struct loc target = loc_is_zero(decoy) ? player->grid : decoy;
	bool group_ai = rf_has(mon->race->flags, RF_GROUP_AI);

	/* Offset to current position to move toward */
	struct loc grid;

	/* Monsters will run up to z_info->flee_range grids out of sight */
	int flee_range = z_info->max_sight + z_info->flee_range;

	bool done = false;

	/* Calculate range */
	get_move_find_range(mon);

	/* Assume we're heading towards the player */
	if (get_move_advance(c, mon, good)) {
		/* We have a good move, use it */
		grid = loc_diff(mon->target.grid, mon->grid);
		mflag_on(mon->mflag, MFLAG_TRACKING);
	} else {
		/* Try to follow someone who knows where they're going */
		struct monster *tracker = group_monster_tracking(c, mon);
		if (tracker && los(c, mon->grid, tracker->grid)) { /* Need los? */
			grid = loc_diff(tracker->grid, mon->grid);
		} else {
			/* Head blindly straight for the "player" if no better idea */
			grid = loc_diff(target, mon->grid);
		}

		/* No longer tracking */
		mflag_off(mon->mflag, MFLAG_TRACKING);
	}

	/* Monster is taking damage from terrain */
	if (monster_taking_terrain_damage(mon)) {
		/* Try to find safe place */
		if (get_move_find_safety(c, mon)) {
			/* Set a course for the safe place */
			get_move_flee(c, mon);
			grid = loc_diff(mon->target.grid, mon->grid);
			done = true;
		}
	}

	/* Normal animal packs try to get the player out of corridors. */
	if (!done && group_ai && !monster_passes_walls(mon)) {
		int i, open = 0;

		/* Count empty grids next to player */
		for (i = 0; i < 8; i++) {
			/* Check grid around the player for room interior (room walls count)
			 * or other empty space */
			struct loc test = loc_sum(target, ddgrid_ddd[i]);
			if (square_ispassable(c, test) || square_isroom(c, test)) {
				/* One more open grid */
				open++;
			}
		}

		/* Not in an empty space and strong player */
		if ((open < 5) && (player->chp > player->mhp / 2)) {
			/* Find hiding place for an ambush */
			if (get_move_find_hiding(c, mon)) {
				done = true;
				grid = loc_diff(mon->target.grid, mon->grid);

				/* No longer tracking */
				mflag_off(mon->mflag, MFLAG_TRACKING);
			}
		}
	}

	/* Not hiding and monster is afraid */
	if (!done && (mon->min_range == flee_range)) {
		/* Try to find safe place */
		if (get_move_find_safety(c, mon)) {
			/* Set a course for the safe place */
			get_move_flee(c, mon);
			grid = loc_diff(mon->target.grid, mon->grid);
		} else {
			/* Just leg it away from the player */
			grid = loc_diff(loc(0, 0), grid);
		}

		/* No longer tracking */
		mflag_off(mon->mflag, MFLAG_TRACKING);
		done = true;
	}

	/* Monster groups try to surround the player if they're in sight */
	if (!done && group_ai && square_isview(c, mon->grid)) {
		int i;
		struct loc grid1 = mon->target.grid;

		/* If we are not already adjacent */
		if (mon->cdis > 1) {
			/* Find an empty square near the player to fill */
			int tmp = randint0(8);
			for (i = 0; i < 8; i++) {
				/* Pick squares near player (pseudo-randomly) */
				grid1 = loc_sum(target, ddgrid_ddd[(tmp + i) % 8]);

				/* Ignore filled grids */
				if (!square_isempty(c, grid1)) continue;

				/* Try to fill this hole */
				break;
			}
		}

		/* Head in the direction of the chosen grid */
		grid = loc_diff(grid1, mon->grid);
	}

	/* Check if the monster has already reached its target */
	if (loc_is_zero(grid)) return (false);

	/* Pick the correct direction */
	*dir = get_move_choose_direction(grid);

	/* Want to move */
	return (true);
}


/**
 * ------------------------------------------------------------------------
 * Monster turn routines
 * These routines, culminating in monster_turn(), decide how a monster uses
 * its turn
 * ------------------------------------------------------------------------ */
/**
 * Lets the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 *
 * Returns true if the monster successfully reproduced.
 */
bool multiply_monster(struct chunk *c, const struct monster *mon)
{
	struct loc grid;
	int i;
	bool result = false;
	struct monster_group_info info = { 0, 0 };

	/* Try up to 18 times */
	for (i = 0; i < 18; i++) {
		int d = 1;

		/* Pick a location */
		scatter(c, &grid, mon->grid, d, true);

		/* Require an "empty" floor grid */
		if (!square_isempty(c, grid)) continue;

		/* Create a new monster (awake, no groups) */
		result = place_new_monster(c, grid, mon->race, false, false, info,
								   ORIGIN_DROP_BREED);

		/* Done */
		break;
	}

	/* Result */
	return (result);
}

/**
 * Attempt to reproduce, if possible.  All monsters are checked here for
 * lore purposes, the unfit fail.
 */
static bool monster_turn_multiply(struct chunk *c, struct monster *mon)
{
	int k = 0, y, x;

	struct monster_lore *lore = get_lore(mon->race);

	/* Too many breeders on the level already */
	if (c->num_repro >= z_info->repro_monster_max) return false;

	/* Count the adjacent monsters */
	for (y = mon->grid.y - 1; y <= mon->grid.y + 1; y++)
		for (x = mon->grid.x - 1; x <= mon->grid.x + 1; x++)
			if (square(c, loc(x, y)).mon > 0) k++;

	/* Multiply slower in crowded areas */
	if ((k < 4) && (k == 0 || one_in_(k * z_info->repro_monster_rate))) {
		/* Successful breeding attempt, learn about that now */
		if (monster_is_visible(mon))
			rf_on(lore->flags, RF_MULTIPLY);

		/* Leave now if not a breeder */
		if (!rf_has(mon->race->flags, RF_MULTIPLY))
			return false;

		/* Try to multiply */
		if (multiply_monster(c, mon)) {
			/* Make a sound */
			if (monster_is_visible(mon))
				sound(MSG_MULTIPLY);

			/* Multiplying takes energy */
			return true;
		}
	}

	return false;
}

/**
 * Check if a monster should stagger (that is, step at random) or not.
 * Always stagger when confused, but also deal with random movement for
 * RAND_25 and RAND_50 monsters.
 */
static bool monster_turn_should_stagger(struct monster *mon)
{
	struct monster_lore *lore = get_lore(mon->race);
	int chance = 0;

	/* Increase chance of being erratic for every level of confusion */
	int conf_level = monster_effect_level(mon, MON_TMD_CONF);
	while (conf_level) {
		int accuracy = 100 - chance;
		accuracy *= (100 - CONF_ERRATIC_CHANCE);
		accuracy /= 100;
		chance = 100 - accuracy;
		conf_level--;
	}

	/* RAND_25 and RAND_50 are cumulative */
	if (rf_has(mon->race->flags, RF_RAND_25)) {
		chance += 25;
		if (monster_is_visible(mon))
			rf_on(lore->flags, RF_RAND_25);
	}

	if (rf_has(mon->race->flags, RF_RAND_50)) {
		chance += 50;
		if (monster_is_visible(mon))
			rf_on(lore->flags, RF_RAND_50);
	}

	return randint0(100) < chance;
}


/**
 * Work out if a monster can move through the grid, if necessary bashing 
 * down doors in the way.
 *
 * Returns true if the monster is able to move through the grid.
 */
static bool monster_turn_can_move(struct chunk *c, struct monster *mon,
		const char *m_name, struct loc new, bool *did_something)
{
	struct monster_lore *lore = get_lore(mon->race);

	/* Dangerous terrain in the way */
	if (monster_hates_grid(c, mon, new)) {
		return false;
	}

	/* Floor is open? */
	if (square_ispassable(c, new)) {
		return true;
	}

	/* Permanent wall in the way */
	if (square_iswall(c, new) && square_isperm(c, new)) {
		return false;
	}

	/* Normal wall, door, or secret door in the way */

	/* There's some kind of feature in the way, so learn about
	 * kill-wall and pass-wall now */
	if (monster_is_visible(mon)) {
		rf_on(lore->flags, RF_PASS_WALL);
		rf_on(lore->flags, RF_KILL_WALL);
		rf_on(lore->flags, RF_SMASH_WALL);
	}

	/* Monster may be able to deal with walls and doors */
	if (rf_has(mon->race->flags, RF_PASS_WALL)) {
		return true;
	} else if (rf_has(mon->race->flags, RF_SMASH_WALL)) {
		/* Remove the wall and much of what's nearby */
		square_smash_wall(c, new);

		/* Note changes to viewable region */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		return true;
	} else if (rf_has(mon->race->flags, RF_KILL_WALL)) {
		/* Remove the wall */
		square_destroy_wall(c, new);

		/* Note changes to viewable region */
		if (square_isview(c, new))
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		return true;
	} else if (square_iscloseddoor(c, new) || square_issecretdoor(c, new)) {
		bool can_open = rf_has(mon->race->flags, RF_OPEN_DOOR);
		bool can_bash = rf_has(mon->race->flags, RF_BASH_DOOR);
		bool will_bash = false;

		/* Take a turn */
		*did_something = true;

		/* Learn about door abilities */
		if (monster_is_visible(mon)) {
			rf_on(lore->flags, RF_OPEN_DOOR);
			rf_on(lore->flags, RF_BASH_DOOR);
		}

		/* If creature can open or bash doors, make a choice */
		if (can_open) {
			/* Sometimes bash anyway (impatient) */
			if (can_bash) {
				will_bash = one_in_(2) ? true : false;
			}
		} else if (can_bash) {
			/* Only choice */
			will_bash = true;
		} else {
			/* Door is an insurmountable obstacle */
			return false;
		}

		/* Now outcome depends on type of door */
		if (square_islockeddoor(c, new)) {
			/* Locked door -- test monster strength against door strength */
			int k = square_door_power(c, new);
			if (randint0(mon->hp / 10) > k) {
				if (will_bash) {
					msg("%s slams against the door.", m_name);
				} else {
					msg("%s fiddles with the lock.", m_name);
				}

				/* Reduce the power of the door by one */
				square_set_door_lock(c, new, k - 1);
			}
		} else {
			/* Closed or secret door -- always open or bash */
			if (square_isview(c, new))
				player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			if (will_bash) {
				square_smash_door(c, new);

				msg("You hear a door burst open!");
				disturb(player, 0);

				/* Fall into doorway */
				return true;
			} else {
				square_open_door(c, new);
			}
		}
	}

	return false;
}

/**
 * Try to break a glyph.
 */
static bool monster_turn_attack_glyph(struct chunk *c, struct monster *mon,
									  struct loc new)
{
	assert(square_iswarded(c, new));

	/* Break the ward */
	if (randint1(z_info->glyph_hardness) < mon->race->level) {
		/* Describe observable breakage */
		if (square_isseen(c, new)) {
			msg("The rune of protection is broken!");

			/* Forget the rune */
			square_forget(c, new);
		}

		/* Break the rune */
		square_destroy_trap(c, new);

		return true;
	}

	/* Unbroken ward - can't move */
	return false;
}

/**
 * Try to push past / kill another monster.  Returns true on success.
 */
static bool monster_turn_try_push(struct chunk *c, struct monster *mon,
									 const char *m_name, struct loc new)
{
	struct monster *mon1 = square_monster(c, new);
	struct monster_lore *lore = get_lore(mon->race);

	/* Kill weaker monsters */
	int kill_ok = monster_can_kill(c, mon, new);

	/* Move weaker monsters if they can swap places */
	/* (not in a wall) */
	int move_ok = (monster_can_move(c, mon, new) &&
				   square_ispassable(c, mon->grid));

	if (kill_ok || move_ok) {
		/* Get the names of the monsters involved */
		char n_name[80];
		monster_desc(n_name, sizeof(n_name), mon1, MDESC_IND_HID);

		/* Learn about pushing and shoving */
		if (monster_is_visible(mon)) {
			rf_on(lore->flags, RF_KILL_BODY);
			rf_on(lore->flags, RF_MOVE_BODY);
		}

		/* Reveal mimics */
		if (monster_is_mimicking(mon1))
			become_aware(mon1);

		/* Note if visible */
		if (monster_is_visible(mon) && monster_is_in_view(mon))
			msg("%s %s %s.", m_name, kill_ok ? "tramples over" : "pushes past",
				n_name);

		/* Monster ate another monster */
		if (kill_ok)
			delete_monster(new);

		monster_swap(mon->grid, new);
		return true;
	}

	return false;
}

/**
 * Grab all objects from the grid.
 */
void monster_turn_grab_objects(struct chunk *c, struct monster *mon,
							   const char *m_name, struct loc new)
{
	struct monster_lore *lore = get_lore(mon->race);
	struct object *obj;
	bool visible = monster_is_visible(mon);

	/* Learn about item pickup behavior */
	for (obj = square_object(c, new); obj; obj = obj->next) {
		if (!tval_is_money(obj) && visible) {
			rf_on(lore->flags, RF_TAKE_ITEM);
			rf_on(lore->flags, RF_KILL_ITEM);
			break;
		}
	}

	/* Abort if can't pickup/kill */
	if (!rf_has(mon->race->flags, RF_TAKE_ITEM) &&
		!rf_has(mon->race->flags, RF_KILL_ITEM)) {
		return;
	}

	/* Take or kill objects on the floor */
	obj = square_object(c, new);
	while (obj) {
		char o_name[80];
		bool safe = obj->artifact ? true : false;
		struct object *next = obj->next;

		/* Skip gold */
		if (tval_is_money(obj)) {
			obj = next;
			continue;
		}

		/* Skip mimicked objects */
		if (obj->mimicking_m_idx) {
			obj = next;
			continue;
		}

		/* Get the object name */
		object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

		/* React to objects that hurt the monster */
		if (react_to_slay(obj, mon))
			safe = true;

		/* Try to pick up, or crush */
		if (safe) {
			/* Only give a message for "take_item" */
			if (rf_has(mon->race->flags, RF_TAKE_ITEM) && visible &&
				square_isview(c, new) && !ignore_item_ok(obj)) {
				/* Dump a message */
				msg("%s tries to pick up %s, but fails.", m_name, o_name);
			}
		} else if (rf_has(mon->race->flags, RF_TAKE_ITEM)) {
			/* Describe observable situations */
			if (square_isseen(c, new) && !ignore_item_ok(obj))
				msg("%s picks up %s.", m_name, o_name);

			/* Carry the object */
			square_excise_object(c, new, obj);
			monster_carry(c, mon, obj);
			square_note_spot(c, new);
			square_light_spot(c, new);
		} else {
			/* Describe observable situations */
			if (square_isseen(c, new) && !ignore_item_ok(obj))
				msgt(MSG_DESTROY, "%s crushes %s.", m_name, o_name);

			/* Delete the object */
			square_excise_object(c, new, obj);
			delist_object(c, obj);
			object_delete(&obj);
			square_note_spot(c, new);
			square_light_spot(c, new);
		}

		/* Next object */
		obj = next;
	}
}


/**
 * Process a monster's turn
 *
 * In several cases, we directly update the monster lore
 *
 * Note that a monster is only allowed to "reproduce" if there
 * are a limited number of "reproducing" monsters on the current
 * level.  This should prevent the level from being "swamped" by
 * reproducing monsters.  It also allows a large mass of mice to
 * prevent a louse from multiplying, but this is a small price to
 * pay for a simple multiplication method.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door
 *
 * In addition, monsters which *cannot* open or bash down a door
 * will still stand there trying to open it...  XXX XXX XXX
 *
 * Technically, need to check for monster in the way combined
 * with that monster being in a wall (or door?) XXX
 */
static void monster_turn(struct chunk *c, struct monster *mon)
{
	struct monster_lore *lore = get_lore(mon->race);

	bool did_something = false;

	int i;
	int dir = 0;
	bool stagger = false;
	bool tracking = false;
	char m_name[80];

	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_CAPITAL | MDESC_IND_HID);

	/* If we're in a web, deal with that */
	if (square_iswebbed(c, mon->grid)) {
		/* Learn web behaviour */
		if (monster_is_visible(mon)) {
			rf_on(lore->flags, RF_CLEAR_WEB);
			rf_on(lore->flags, RF_PASS_WEB);
		}

		/* If we can pass, no need to clear */
		if (!rf_has(mon->race->flags, RF_PASS_WEB)) {
			/* Learn wall behaviour */
			if (monster_is_visible(mon)) {
				rf_on(lore->flags, RF_PASS_WALL);
				rf_on(lore->flags, RF_KILL_WALL);
			}

			/* Now several possibilities */
			if (rf_has(mon->race->flags, RF_PASS_WALL)) {
				/* Insubstantial monsters go right through */
			} else if (monster_passes_walls(mon)) {
				/* If you can destroy a wall, you can destroy a web */
				square_destroy_trap(c, mon->grid);
			} else if (rf_has(mon->race->flags, RF_CLEAR_WEB)) {
				/* Clearing costs a turn (assume there are no other "traps") */
				square_destroy_trap(c, mon->grid);
				return;
			} else {
				/* Stuck */
				return;
			}
		}
	}

	/* Let other group monsters know about the player */
	monster_group_rouse(c, mon);

	/* Try to multiply - this can use up a turn */
	if (monster_turn_multiply(c, mon))
		return;

	/* Attempt a ranged attack */
	if (make_ranged_attack(mon)) return;

	/* Work out what kind of movement to use - random movement or AI */
	if (monster_turn_should_stagger(mon)) {
		stagger = true;
	} else {
		/* If there's no sensible move, we're done */
		if (!get_move(c, mon, &dir, &tracking)) return;
	}

	/* Try to move first in the chosen direction, or next either side of the
	 * chosen direction, or next at right angles to the chosen direction.
	 * Monsters which are tracking by sound or scent will not move if they
	 * can't move in their chosen direction. */
	for (i = 0; i < 5 && !did_something; i++) {
		/* Get the direction (or stagger) */
		int d = (stagger ? ddd[randint0(8)] : side_dirs[dir][i]);

		/* Get the grid to step to or attack */
		struct loc new = loc_sum(mon->grid, ddgrid[d]);

		/* Tracking monsters have their best direction, don't change */
		if ((i > 0) && !stagger && !square_isview(c, mon->grid) && tracking) {
			break;
		}

		/* Check if we can move */
		if (!monster_turn_can_move(c, mon, m_name, new, &did_something))
			continue;

		/* Try to break the glyph if there is one.  This can happen multiple
		 * times per turn because failure does not break the loop */
		if (square_iswarded(c, new) && !monster_turn_attack_glyph(c, mon, new))
			continue;

		/* Break a decoy if there is one */
		if (square_isdecoyed(c, new)) {
			/* Learn about if the monster attacks */
			if (monster_is_visible(mon))
				rf_on(lore->flags, RF_NEVER_BLOW);

			/* Some monsters never attack */
			if (rf_has(mon->race->flags, RF_NEVER_BLOW))
				continue;

			/* Wait a minute... */
			square_destroy_decoy(c, new);
			did_something = true;
			break;
		}

		/* The player is in the way. */
		if (square_isplayer(c, new)) {
			/* Learn about if the monster attacks */
			if (monster_is_visible(mon))
				rf_on(lore->flags, RF_NEVER_BLOW);

			/* Some monsters never attack */
			if (rf_has(mon->race->flags, RF_NEVER_BLOW))
				continue;

			/* Otherwise, attack the player */
			make_attack_normal(mon, player);

			did_something = true;
			break;
		} else {
			/* Some monsters never move */
			if (rf_has(mon->race->flags, RF_NEVER_MOVE)) {
				/* Learn about lack of movement */
				if (monster_is_visible(mon))
					rf_on(lore->flags, RF_NEVER_MOVE);

				return;
			}
		}

		/* A monster is in the way, try to push past/kill */
		if (square_monster(c, new)) {
			did_something = monster_turn_try_push(c, mon, m_name, new);
		} else {
			/* Otherwise we can just move */
			monster_swap(mon->grid, new);
			did_something = true;
		}

		/* Scan all objects in the grid, if we reached it */
		if (mon == square_monster(c, new)) {
			monster_desc(m_name, sizeof(m_name), mon,
						 MDESC_CAPITAL | MDESC_IND_HID);
			monster_turn_grab_objects(c, mon, m_name, new);
		}
	}

	if (did_something) {
		/* Learn about no lack of movement */
		if (monster_is_visible(mon))
			rf_on(lore->flags, RF_NEVER_MOVE);

		/* Possible disturb */
		if (monster_is_visible(mon) && monster_is_in_view(mon) && 
			OPT(player, disturb_near))
			disturb(player, 0);		
	}

	/* Hack -- get "bold" if out of options */
	if (!did_something && mon->m_timed[MON_TMD_FEAR])
		mon_clear_timed(mon, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY);

	/* If we see an unaware monster do something, become aware of it */
	if (did_something && monster_is_camouflaged(mon))
		become_aware(mon);
}


/**
 * ------------------------------------------------------------------------
 * Processing routines that happen to a monster regardless of whether it
 * gets a turn, and/or to decide whether it gets a turn
 * ------------------------------------------------------------------------ */
/**
 * Determine whether a monster is active or passive
 */
static bool monster_check_active(struct chunk *c, struct monster *mon)
{
	if ((mon->cdis <= mon->race->hearing) && monster_passes_walls(mon)) {
		/* Character is inside scanning range, monster can go straight there */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (mon->hp < mon->maxhp) {
		/* Monster is hurt */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (square_isview(c, mon->grid)) {
		/* Monster can "see" the player (checked backwards) */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (monster_can_hear(c, mon)) {
		/* Monster can hear the player */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (monster_can_smell(c, mon)) {
		/* Monster can smell the player */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (monster_taking_terrain_damage(mon)) {
		/* Monster is taking damage from the terrain */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else {
		/* Otherwise go passive */
		mflag_off(mon->mflag, MFLAG_ACTIVE);
	}

	return mflag_has(mon->mflag, MFLAG_ACTIVE) ? true : false;
}

/**
 * Wake a monster or reduce its depth of sleep
 *
 * Chance of waking up is dependent only on the player's stealth, but the
 * amount of sleep reduction takes into account the monster's distance from
 * the player.  Currently straight line distance is used; possibly this
 * should take into account dungeon structure.
 */
static void monster_reduce_sleep(struct chunk *c, struct monster *mon)
{
	int stealth = player->state.skills[SKILL_STEALTH];
	int player_noise = 1 << (30 - stealth);
	int notice = randint0(1024);
	struct monster_lore *lore = get_lore(mon->race);

	/* Aggravation */
	if (player_of_has(player, OF_AGGRAVATE)) {
		char m_name[80];

		/* Wake the monster, make it aware */
		monster_wake(mon, false, 100);

		/* Get the monster name */
		monster_desc(m_name, sizeof(m_name), mon,
					 MDESC_CAPITAL | MDESC_IND_HID);

		/* Notify the player if aware */
		if (monster_is_obvious(mon)) {
			msg("%s wakes up.", m_name);
			equip_learn_flag(player, OF_AGGRAVATE);
		}
	} else if ((notice * notice * notice) <= player_noise) {
		int sleep_reduction = 1;
		int local_noise = c->noise.grids[mon->grid.y][mon->grid.x];
		bool woke_up = false;

		/* Test - wake up faster in hearing distance of the player 
		 * Note no dependence on stealth for now */
		if ((local_noise > 0) && (local_noise < 50)) {
			sleep_reduction = (100 / local_noise);
		}

		/* Note a complete wakeup */
		if (mon->m_timed[MON_TMD_SLEEP] <= sleep_reduction) {
			woke_up = true;
		}

		/* Monster wakes up a bit */
		mon_dec_timed(mon, MON_TMD_SLEEP, sleep_reduction, MON_TMD_FLG_NOTIFY);

		/* Update knowledge */
		if (monster_is_obvious(mon)) {
			if (!woke_up && lore->ignore < UCHAR_MAX)
				lore->ignore++;
			else if (woke_up && lore->wake < UCHAR_MAX)
				lore->wake++;
			lore_update(mon->race, lore);
		}
	}
}

/**
 * Process a monster's timed effects, e.g. decrease them.
 *
 * Returns true if the monster is skipping its turn.
 */
static bool process_monster_timed(struct chunk *c, struct monster *mon)
{
	/* If the monster is asleep or just woke up, then it doesn't act */
	if (mon->m_timed[MON_TMD_SLEEP]) {
		monster_reduce_sleep(c, mon);
		return true;
	} else {
		/* Awake, active monsters may become aware */
		if (one_in_(10) && mflag_has(mon->mflag, MFLAG_ACTIVE)) {
			mflag_on(mon->mflag, MFLAG_AWARE);
		}
	}

	if (mon->m_timed[MON_TMD_FAST])
		mon_dec_timed(mon, MON_TMD_FAST, 1, 0);

	if (mon->m_timed[MON_TMD_SLOW])
		mon_dec_timed(mon, MON_TMD_SLOW, 1, 0);

	if (mon->m_timed[MON_TMD_HOLD])
		mon_dec_timed(mon, MON_TMD_HOLD, 1, 0);

	if (mon->m_timed[MON_TMD_DISEN])
		mon_dec_timed(mon, MON_TMD_DISEN, 1, 0);

	if (mon->m_timed[MON_TMD_STUN])
		mon_dec_timed(mon, MON_TMD_STUN, 1, MON_TMD_FLG_NOTIFY);

	if (mon->m_timed[MON_TMD_CONF]) {
		mon_dec_timed(mon, MON_TMD_CONF, 1, MON_TMD_FLG_NOTIFY);
	}

	if (mon->m_timed[MON_TMD_CHANGED]) {
		mon_dec_timed(mon, MON_TMD_CHANGED, 1, MON_TMD_FLG_NOTIFY);
	}

	if (mon->m_timed[MON_TMD_FEAR]) {
		int d = randint1(mon->race->level / 10 + 1);
		mon_dec_timed(mon, MON_TMD_FEAR, d, MON_TMD_FLG_NOTIFY);
	}

	/* Always miss turn if held or commanded, one in STUN_MISS_CHANCE chance
	 * of missing if stunned,  */
	if (mon->m_timed[MON_TMD_HOLD] || mon->m_timed[MON_TMD_COMMAND]) {
		return true;
	} else if (mon->m_timed[MON_TMD_STUN]) {
		return one_in_(STUN_MISS_CHANCE);
	} else {
		return false;
	}
}

/**
 * Monster regeneration of HPs.
 */
static void regen_monster(struct monster *mon, int num)
{
	/* Regenerate (if needed) */
	if (mon->hp < mon->maxhp) {
		/* Base regeneration */
		int frac = mon->maxhp / 100;

		/* Minimal regeneration rate */
		if (!frac) frac = 1;

		/* Some monsters regenerate quickly */
		if (rf_has(mon->race->flags, RF_REGENERATE)) frac *= 2;

		/* Multiply by number of regenerations */
		frac *= num;

		/* Regenerate */
		mon->hp += frac;

		/* Do not over-regenerate */
		if (mon->hp > mon->maxhp) mon->hp = mon->maxhp;

		/* Redraw (later) if needed */
		if (player->upkeep->health_who == mon)
			player->upkeep->redraw |= (PR_HEALTH);
	}
}


/**
 * ------------------------------------------------------------------------
 * Monster processing routines to be called by the main game loop
 * ------------------------------------------------------------------------ */
/**
 * Process all the "live" monsters, once per game turn.
 *
 * During each game turn, we scan through the list of all the "live" monsters,
 * (backwards, so we can excise any "freshly dead" monsters), energizing each
 * monster, and allowing fully energized monsters to move, attack, pass, etc.
 *
 * This function and its children are responsible for a considerable fraction
 * of the processor time in normal situations, greater if the character is
 * resting.
 */
void process_monsters(struct chunk *c, int minimum_energy)
{
	int i;
	int mspeed;

	/* Only process some things every so often */
	bool regen = false;

	/* Regenerate hitpoints and mana every 100 game turns */
	if (turn % 100 == 0)
		regen = true;

	/* Process the monsters (backwards) */
	for (i = cave_monster_max(c) - 1; i >= 1; i--) {
		struct monster *mon;
		bool moving;

		/* Handle "leaving" */
		if (player->is_dead || player->upkeep->generate_level) break;

		/* Get a 'live' monster */
		mon = cave_monster(c, i);
		if (!mon->race) continue;

		/* Ignore monsters that have already been handled */
		if (mflag_has(mon->mflag, MFLAG_HANDLED))
			continue;

		/* Not enough energy to move yet */
		if (mon->energy < minimum_energy) continue;

		/* Does this monster have enough energy to move? */
		moving = mon->energy >= z_info->move_energy ? true : false;

		/* Prevent reprocessing */
		mflag_on(mon->mflag, MFLAG_HANDLED);

		/* Handle monster regeneration if requested */
		if (regen)
			regen_monster(mon, 1);

		/* Calculate the net speed */
		mspeed = mon->mspeed;
		if (mon->m_timed[MON_TMD_FAST])
			mspeed += 10;
		if (mon->m_timed[MON_TMD_SLOW]) {
			int slow_level = monster_effect_level(mon, MON_TMD_SLOW);
			mspeed -= (2 * slow_level);
		}

		/* Give this monster some energy */
		mon->energy += turn_energy(mspeed);

		/* End the turn of monsters without enough energy to move */
		if (!moving)
			continue;

		/* Use up "some" energy */
		mon->energy -= z_info->move_energy;

		/* Mimics lie in wait */
		if (monster_is_mimicking(mon)) continue;

		/* Check if the monster is active */
		if (monster_check_active(c, mon)) {
			/* Process timed effects - skip turn if necessary */
			if (process_monster_timed(c, mon))
				continue;

			/* Set this monster to be the current actor */
			c->mon_current = i;

			/* The monster takes its turn */
			monster_turn(c, mon);

			/* Monster is no longer current */
			c->mon_current = -1;
		}
	}

	/* Update monster visibility after this */
	/* XXX This may not be necessary */
	player->upkeep->update |= PU_MONSTERS;
}

/**
 * Clear 'moved' status from all monsters.
 *
 * Clear noise if appropriate.
 */
void reset_monsters(void)
{
	int i;
	struct monster *mon;

	/* Process the monsters (backwards) */
	for (i = cave_monster_max(cave) - 1; i >= 1; i--) {
		/* Access the monster */
		mon = cave_monster(cave, i);

		/* Dungeon hurts monsters */
		monster_take_terrain_damage(mon);

		/* Monster is ready to go again */
		mflag_off(mon->mflag, MFLAG_HANDLED);
	}
}

/**
 * Allow monsters on a frozen persistent level to recover
 */
void restore_monsters(void)
{
	int i;
	struct monster *mon;

	/* Get the number of turns that have passed */
	int num_turns = turn - cave->turn;

	/* Process the monsters (backwards) */
	for (i = cave_monster_max(cave) - 1; i >= 1; i--) {
		int status, status_red;

		/* Access the monster */
		mon = cave_monster(cave, i);

		/* Regenerate */
		regen_monster(mon, num_turns / 100);

		/* Handle timed effects */
		status_red = num_turns * turn_energy(mon->mspeed) / z_info->move_energy;
		if (status_red > 0) {
			for (status = 0; status < MON_TMD_MAX; status++) {
				if (mon->m_timed[status]) {
					mon_dec_timed(mon, status, status_red, 0);
				}
			}
		}
	}
}
