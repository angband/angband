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
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "mon-timed.h"
#include "obj-desc.h"
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
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
	int my = mon->fy;
	int mx = mon->fx;

	/* If player is in LOS, there's no need to go around walls */
    if (projectable(c, my, mx, player->py, player->px, PROJECT_NONE))
		return false;

    /* PASS_WALL & KILL_WALL monsters occasionally flow for a turn anyway */
    if (randint0(99) < 5) return true;

	/* Search the nearby grids, which are always in bounds */
	for (y = (my - 2); y <= (my + 2); y++) {
		for (x = (mx - 2); x <= (mx + 2); x++) {
            if (!square_in_bounds_fully(c, y, x)) continue;
            if (square_isperm(c, y, x)) return true;
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
	if (c->noise.grids[mon->fy][mon->fx] == 0) {
		return false;
	}
	return base_hearing > c->noise.grids[mon->fy][mon->fx];
}

/**
 * Check if the monster can smell anything
 */
static bool monster_can_smell(struct chunk *c, struct monster *mon)
{
	if (c->scent.grids[mon->fy][mon->fx] == 0) {
		return false;
	}
	return mon->race->smell > c->scent.grids[mon->fy][mon->fx];
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
static bool monster_can_kill(struct chunk *c, struct monster *mon, int y, int x)
{
	struct monster *mon1 = square_monster(c, y, x);

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
static bool monster_can_move(struct chunk *c, struct monster *mon, int y, int x)
{
	struct monster *mon1 = square_monster(c, y, x);

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
static bool monster_hates_grid(struct chunk *c, struct monster *mon, int y,
							   int x)
{
	/* Only some creatures can handle damaging terrain */
	if (square_isdamaging(c, y, x) &&
		!rf_has(mon->race->flags, square_feat(c, y, x)->resist_flag)) {
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
 */
static void get_move_find_range(struct monster *mon)
{
	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

	/* Monsters will run up to z_info->flee_range grids out of sight */
	int flee_range = z_info->max_sight + z_info->flee_range;

	bool breathes = flags_test(mon->race->spell_flags, RSF_SIZE,
							   RSF_BREATH_MASK, FLAG_END);

	/* All "afraid" monsters will run away */
	if (mon->m_timed[MON_TMD_FEAR]) {
		mon->min_range = flee_range;
	} else {
		/* Minimum distance - stay at least this far if possible */
		mon->min_range = 1;

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
	//if (rf_has(mon->race->flags, RF_ARCHER))
	//	mon->best_range += 3;

	if (mon->race->freq_spell > 24) {
		/* Breathers like point blank range */
		if (breathes && (mon->best_range < 6) && (mon->hp > mon->maxhp / 2)) {
			mon->best_range = 6;
		} else {
			/* Other spell casters will sit back and cast */
			mon->best_range += 3;
		}
	}
}

/**
 * Choose the best direction to advance toward the player, using sound or scent.
 *
 * Note that ghosts and rock-eaters generally just head straight for the player.
 *
 * Monsters first try to use current sound information as saved in
 * c->noise.grids[y][x].  Failing that, they'll try using scent, saved in 
 * c->scent.grids[y][x].
 *
 * Note that this function assumes the monster is moving to an adjacent grid,
 * and so the noise can be louder by at most 1.
 *
 * Tracking by 'scent' means that monsters end up near enough the player to
 * switch to 'sound' (noise), or they end up somewhere the player left via 
 * teleport.  Teleporting away from a location will cause the monsters who
 * were chasing the player to converge on that location as long as the player
 * is still near enough to "annoy" them without being close enough to chase
 * directly.
 */
static bool get_move_advance(struct chunk *c, struct monster *mon)
{
	int i;
	int best_scent = 0;
	bool found_direction = false;
	int my = mon->fy, mx = mon->fx;
	int base_hearing = mon->race->hearing
		- player->state.skills[SKILL_STEALTH] / 3;
	int current_noise = base_hearing - c->noise.grids[my][mx];
	int best_direction = 8;
	int backup_direction = -1;

	/* If the monster can pass through nearby walls, do that */
	if (monster_passes_walls(mon) && !monster_near_permwall(mon, c)) {
		mon->ty = player->py;
		mon->tx = player->px;
		return true;
	}

	/* If the player can see monster, set target and run towards them */
	if (square_isview(c, my, mx)) {
		mon->ty = player->py;
		mon->tx = player->px;
		return true;
	}

	/* Check nearby sound, giving preference to the cardinal directions */
	for (i = 0; i < 8; i++) {
		/* Get the location */
		int y = my + ddy_ddd[i];
		int x = mx + ddx_ddd[i];
		int heard_noise = base_hearing - c->noise.grids[y][x];

		/* Bounds check */
		if (!square_in_bounds(c, y, x)) {
			continue;
		}

		/* Must be some noise */
		if (c->noise.grids[y][x] == 0) {
			continue;
		}

		/* There's a monster blocking that we can't deal with */
		if (!monster_can_kill(c, mon, y, x) && !monster_can_move(c, mon, y, x)){
			continue;
		}

		/* There's damaging terrain */
		if (monster_hates_grid(c, mon, y, x)) {
			continue;
		}

		/* If it's better than the current noise, choose this direction */
		if (heard_noise > current_noise) {
			best_direction = i;
			found_direction = true;
			break;
		} else if (heard_noise == current_noise) {
			/* Possible move if we can't actually get closer */
			backup_direction = i;
			continue;
		}
	}

	/* If no good sound, use scent */
	if (!found_direction) {
		for (i = 0; i < 8; i++) {
			/* Get the location */
			int y = my + ddy_ddd[i];
			int x = mx + ddx_ddd[i];
			int smelled_scent;

			/* If no good sound yet, use scent */
			smelled_scent = mon->race->smell - c->scent.grids[y][x];
			if ((smelled_scent > best_scent) && (c->scent.grids[y][x] != 0)) {
				best_scent = smelled_scent;
				best_direction = i;
				found_direction = true;
			}
		}
	}

	/* Set the target */
	if (found_direction) {
		mon->ty = my + ddy_ddd[best_direction];
		mon->tx = mx + ddx_ddd[best_direction];
		return true;
	} else if (backup_direction >= 0) {
		/* Move around to try and improve position */
		mon->ty = my + ddy_ddd[backup_direction];
		mon->tx = mx + ddx_ddd[backup_direction];
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
	int fy = mon->fy;
	int fx = mon->fx;

	int py = player->py;
	int px = player->px;

	int i, y, x, dy, dx, d, dis;
	int gy = 0, gx = 0, gdis = 0;

	const int *y_offsets;
	const int *x_offsets;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++) {
		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i]) {
			y = fy + dy;
			x = fx + dx;

			/* Skip illegal locations */
			if (!square_in_bounds_fully(c, y, x)) continue;

			/* Skip locations in a wall */
			if (!square_ispassable(c, y, x)) continue;

			/* Ignore too-distant grids */
			if (c->noise.grids[y][x] > c->noise.grids[fy][fx] + 2 * d)
				continue;

			/* Ignore damaging terrain if they can't handle it */
			if (square_isdamaging(c, y, x) &&
				!rf_has(mon->race->flags, square_feat(c, y, x)->resist_flag))
				continue;

			/* Check for absence of shot (more or less) */
			if (!square_isview(c, y, x)) {
				/* Calculate distance from player */
				dis = distance(y, x, py, px);

				/* Remember if further than previous */
				if (dis > gdis) {
					gy = y;
					gx = x;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis > 0) {
			/* Good location */
			mon->ty = gy;
			mon->tx = gx;

			/* Found safe place */
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
	int fy = mon->fy;
	int fx = mon->fx;

	int py = player->py;
	int px = player->px;

	int i, y, x, dy, dx, d, dis;
	int gy = 0, gx = 0, gdis = 999, min;

	const int *y_offsets, *x_offsets;

	/* Closest distance to get */
	min = distance(py, px, fy, fx) * 3 / 4 + 2;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++) {
		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i]) {
			y = fy + dy;
			x = fx + dx;

			/* Skip illegal locations */
			if (!square_in_bounds_fully(c, y, x)) continue;

			/* Skip occupied locations */
			if (!square_isempty(c, y, x)) continue;

			/* Check for hidden, available grid */
			if (!square_isview(c, y, x) &&
				projectable(c, fy, fx, y, x, PROJECT_STOP)) {
				/* Calculate distance from player */
				dis = distance(y, x, py, px);

				/* Remember if closer than previous */
				if (dis < gdis && dis >= min) {
					gy = y;
					gx = x;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis < 999) {
			/* Good location */
			mon->ty = gy;
			mon->tx = gx;

			/* Found good place */
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
	int gy = 0, gx = 0;
	int best_score = -1;
	int my = mon->fy, mx = mon->fx;

	/* If the player is not currently near the monster, no reason to flow */
	if (mon->cdis >= mon->best_range) {
		return false;
	}

	/* Monster is too far away to use sound or scent */
	if (!monster_can_hear(c, mon) && !monster_can_smell(c, mon)) {
		return false;
	}

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--) {
		int dis, score;

		/* Get the location */
		int y = my + ddy_ddd[i];
		int x = mx + ddx_ddd[i];

		/* Bounds check */
		if (!square_in_bounds(c, y, x)) continue;

		/* Calculate distance of this grid from our target */
		dis = distance(y, x, mon->ty, mon->tx);

		/* Score this grid
		 * First half of calculation is inversely proportional to distance
		 * Second half is inversely proportional to grid's distance from player
		 */
		score = 5000 / (dis + 3) - 500 / (c->noise.grids[y][x] + 1);

		/* No negative scores */
		if (score < 0) score = 0;

		/* Ignore lower scores */
		if (score < best_score) continue;

		/* Save the score */
		best_score = score;

		/* Save the location */
		gy = y;
		gx = x;
	}

	/* Set the immediate target */
	mon->ty = gy;
	mon->tx = gx;

	/* Success */
	return true;
}

/**
 * Choose the basic direction of movement, and whether to bias left or right
 * if the main direction is blocked.
 *
 * Note that this direction is intended as an index into the side_dirs array.
 */
static int get_move_choose_direction(int dy, int dx)
{
	int dir = 0;

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
 */
static bool get_move(struct chunk *c, struct monster *mon, int *dir, bool *good)
{
	int py = player->py;
	int px = player->px;

	int y, x;

	/* Monsters will run up to z_info->flee_range grids out of sight */
	int flee_range = z_info->max_sight + z_info->flee_range;

	bool done = false;

	/* Calculate range */
	get_move_find_range(mon);

	/* Assume we're heading towards the player */
	if (get_move_advance(c, mon)) {
		/* Extract the "pseudo-direction" */
		y = mon->ty - mon->fy;
		x = mon->tx - mon->fx;
		*good = true;
	} else {
		/* Head blindly straight for the player if there's no better idea */
		y = player->py - mon->fy;
		x = player->px - mon->fx;
	}

	/* Normal animal packs try to get the player out of corridors. */
	if (rf_has(mon->race->flags, RF_GROUP_AI) &&
	    !flags_test(mon->race->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL,
					FLAG_END)) {
		int i, open = 0;

		/* Count empty grids next to player */
		for (i = 0; i < 8; i++) {
			int ry = py + ddy_ddd[i];
			int rx = px + ddx_ddd[i];
			/* Check grid around the player for room interior (room walls count)
			 * or other empty space */
			if (square_ispassable(c, ry, rx) || square_isroom(c, ry, rx)) {
				/* One more open grid */
				open++;
			}
		}

		/* Not in an empty space and strong player */
		if ((open < 5) && (player->chp > player->mhp / 2)) {
			/* Find hiding place */
			if (get_move_find_hiding(c, mon)) {
				done = true;
				y = mon->ty - mon->fy;
				x = mon->tx - mon->fx;
			}
		}
	}

	/* Apply fear */
	if (!done && (mon->min_range == flee_range)) {
		/* Try to find safe place */
		if (!get_move_find_safety(c, mon)) {
			/* Just leg it away from the player */
			y = (-y);
			x = (-x);
		} else {
			/* Set a course for the safe place */
			get_move_flee(c, mon);
			y = mon->ty - mon->fy;
			x = mon->tx - mon->fx;
		}

		done = true;
	}

	/* Monster groups try to surround the player */
	if (!done && rf_has(mon->race->flags, RF_GROUP_AI) &&
		square_isview(c, mon->fy, mon->fx)) {
		int i, yy = mon->ty, xx = mon->tx;

		/* If we are not already adjacent */
		if (mon->cdis > 1) {
			/* Find an empty square near the player to fill */
			int tmp = randint0(8);
			for (i = 0; i < 8; i++) {
				/* Pick squares near player (pseudo-randomly) */
				yy = py + ddy_ddd[(tmp + i) & 7];
				xx = px + ddx_ddd[(tmp + i) & 7];

				/* Ignore filled grids */
				if (!square_isempty(c, yy, xx)) continue;

				/* Try to fill this hole */
				break;
			}
		}

		/* Extract the new "pseudo-direction" */
		y = yy - mon->fy;
		x = xx - mon->fx;
	}

	/* Check if the monster has already reached its target */
	if (!x && !y) return (false);

	/* Pick the correct direction */
	*dir = get_move_choose_direction(y, x);

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
	int i, y, x;

	bool result = false;

	/* Try up to 18 times */
	for (i = 0; i < 18; i++) {
		int d = 1;

		/* Pick a location */
		scatter(c, &y, &x, mon->fy, mon->fx, d, true);

		/* Require an "empty" floor grid */
		if (!square_isempty(c, y, x)) continue;

		/* Create a new monster (awake, no groups) */
		result = place_new_monster(c, y, x, mon->race, false, false,
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
	if (num_repro >= z_info->repro_monster_max) return false;

	/* Count the adjacent monsters */
	for (y = mon->fy - 1; y <= mon->fy + 1; y++)
		for (x = mon->fx - 1; x <= mon->fx + 1; x++)
			if (c->squares[y][x].mon > 0) k++;

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
 * Check if a monster should stagger or not.  Always stagger when confused,
 * but also deal with random movement for RAND_25 and _50 monsters.
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
		const char *m_name, int nx, int ny, bool *did_something)
{
	struct monster_lore *lore = get_lore(mon->race);

	/* Dangerous terrain in the way */
	if (monster_hates_grid(c, mon, ny, nx)) {
		return false;
	}

	/* Floor is open? */
	if (square_ispassable(c, ny, nx)) {
		return true;
	}

	/* Permanent wall in the way */
	if (square_iswall(c, ny, nx) && square_isperm(c, ny, nx)) {
		return false;
	}

	/* Normal wall, door, or secret door in the way */

	/* There's some kind of feature in the way, so learn about
	 * kill-wall and pass-wall now */
	if (monster_is_visible(mon)) {
		rf_on(lore->flags, RF_PASS_WALL);
		rf_on(lore->flags, RF_KILL_WALL);
	}

	/* Monster may be able to deal with walls and doors */
	if (rf_has(mon->race->flags, RF_PASS_WALL)) {
		return true;
	} else if (rf_has(mon->race->flags, RF_KILL_WALL)) {
		/* Remove the wall */
		square_destroy_wall(c, ny, nx);

		/* Note changes to viewable region */
		if (square_isview(c, ny, nx))
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		return true;
	} else if (square_iscloseddoor(c, ny, nx) ||
			   square_issecretdoor(c, ny, nx)) {
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
		if (square_islockeddoor(c, ny, nx)) {
			/* Locked door -- test monster strength against door strength */
			int k = square_door_power(c, ny, nx);
			if (randint0(mon->hp / 10) > k) {
				if (will_bash) {
					msg("%s slams against the door.", m_name);
				} else {
					msg("%s fiddles with the lock.", m_name);
				}

				/* Reduce the power of the door by one */
				square_set_door_lock(c, ny, nx, k - 1);
			}
		} else {
			/* Closed or secret door -- always open or bash */
			if (square_isview(c, ny, nx))
				player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			if (will_bash) {
				square_smash_door(c, ny, nx);

				msg("You hear a door burst open!");
				disturb(player, 0);

				/* Fall into doorway */
				return true;
			} else {
				square_open_door(c, ny, nx);
			}
		}
	}

	return false;
}

/**
 * Try to break a glyph.
 */
static bool monster_turn_glyph(struct chunk *c, struct monster *mon,
								  int nx, int ny)
{
	assert(square_iswarded(c, ny, nx));

	/* Break the ward */
	if (randint1(z_info->glyph_hardness) < mon->race->level) {
		/* Describe observable breakage */
		if (square_isseen(c, ny, nx)) {
			msg("The rune of protection is broken!");

			/* Forget the rune */
			square_forget(c, ny, nx);
		}

		/* Break the rune */
		square_remove_ward(c, ny, nx);

		return true;
	}

	/* Unbroken ward - can't move */
	return false;
}

/**
 * Try to push past / kill another monster.  Returns true on success.
 */
static bool monster_turn_try_push(struct chunk *c, struct monster *mon,
									 const char *m_name, int nx, int ny)
{
	struct monster *mon1 = square_monster(c, ny, nx);
	struct monster_lore *lore = get_lore(mon->race);

	/* Kill weaker monsters */
	int kill_ok = rf_has(mon->race->flags, RF_KILL_BODY);

	/* Move weaker monsters if they can swap places */
	/* (not in a wall) */
	int move_ok = (rf_has(mon->race->flags, RF_MOVE_BODY) &&
				   square_ispassable(c, mon->fy, mon->fx));

	if (compare_monsters(mon, mon1) > 0) {
		/* Learn about pushing and shoving */
		if (monster_is_visible(mon)) {
			rf_on(lore->flags, RF_KILL_BODY);
			rf_on(lore->flags, RF_MOVE_BODY);
		}

		if (kill_ok || move_ok) {
			/* Get the names of the monsters involved */
			char n_name[80];
			monster_desc(n_name, sizeof(n_name), mon1, MDESC_IND_HID);

			/* Reveal mimics */
			if (monster_is_mimicking(mon1))
				become_aware(mon1);

			/* Note if visible */
			if (monster_is_visible(mon) && monster_is_in_view(mon))
				msg("%s %s %s.", m_name,
					kill_ok ? "tramples over" : "pushes past", n_name);

			/* Monster ate another monster */
			if (kill_ok)
				delete_monster(ny, nx);

			monster_swap(mon->fy, mon->fx, ny, nx);
			return true;
		} 
	}

	return false;
}

/**
 * Grab all objects from the grid.
 */
void monster_turn_grab_objects(struct chunk *c, struct monster *mon,
		const char *m_name, int nx, int ny)
{
	struct monster_lore *lore = get_lore(mon->race);
	struct object *obj;
	bool visible = monster_is_visible(mon);

	/* Learn about item pickup behavior */
	for (obj = square_object(c, ny, nx); obj; obj = obj->next) {
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
	obj = square_object(c, ny, nx);
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
				square_isview(c, ny, nx) && !ignore_item_ok(obj)) {
				/* Dump a message */
				msg("%s tries to pick up %s, but fails.", m_name, o_name);
			}
		} else if (rf_has(mon->race->flags, RF_TAKE_ITEM)) {
			/* Describe observable situations */
			if (square_isseen(c, ny, nx) && !ignore_item_ok(obj))
				msg("%s picks up %s.", m_name, o_name);

			/* Carry the object */
			square_excise_object(c, ny, nx, obj);
			monster_carry(c, mon, obj);
			square_note_spot(c, ny, nx);
			square_light_spot(c, ny, nx);
		} else {
			/* Describe observable situations */
			if (square_isseen(c, ny, nx) && !ignore_item_ok(obj))
				msgt(MSG_DESTROY, "%s crushes %s.", m_name, o_name);

			/* Delete the object */
			square_excise_object(c, ny, nx, obj);
			delist_object(c, obj);
			object_delete(&obj);
			square_note_spot(c, ny, nx);
			square_light_spot(c, ny, nx);
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

	/* Try to multiply - this can use up a turn */
	if (monster_turn_multiply(c, mon))
		return;

	/* Attempt to cast a spell */
	if (make_attack_spell(mon)) return;

	/* Work out what kind of movement to use - AI or staggered movement */
	if (!monster_turn_should_stagger(mon)) {
		if (!get_move(c, mon, &dir, &tracking)) return;
	} else {
		stagger = true;
	}

	/* Process moves */
	for (i = 0; i < 5 && !did_something; i++) {
		int oy = mon->fy;
		int ox = mon->fx;

		/* Get the direction (or stagger) */
		int d = (stagger ? ddd[randint0(8)] : side_dirs[dir][i]);

		/* Get the destination */
		int ny = oy + ddy[d];
		int nx = ox + ddx[d];

		/* Tracking monsters have their best direction, don't change */
		if ((i > 0) && !stagger && !square_isview(c, oy, ox) && tracking) {
			break;
		}

		/* Check if we can move */
		if (!monster_turn_can_move(c, mon, m_name, nx, ny, &did_something))
			continue;

		/* Try to break the glyph if there is one.  This can happen multiple
		 * times per turn because failure does not break the loop */
		if (square_iswarded(c, ny, nx) &&
			!monster_turn_glyph(c, mon, nx, ny))
			continue;

		/* The player is in the way. */
		if (square_isplayer(c, ny, nx)) {
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
		if (square_monster(c, ny, nx)) {
			did_something = monster_turn_try_push(c, mon, m_name, nx, ny);
		} else {
			/* Otherwise we can just move */
			monster_swap(oy, ox, ny, nx);
			did_something = true;
		}

		/* Scan all objects in the grid, if we reached it */
		if (mon == square_monster(c, ny, nx)) {
			monster_desc(m_name, sizeof(m_name), mon,
						 MDESC_CAPITAL | MDESC_IND_HID);
			monster_turn_grab_objects(c, mon, m_name, nx, ny);
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
		mon_clear_timed(mon, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY, false);

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
	} else if (square_isview(c, mon->fy, mon->fx)) {
		/* Monster can "see" the player (checked backwards) */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (monster_can_hear(c, mon)) {
		/* Monster can hear the player */
		mflag_on(mon->mflag, MFLAG_ACTIVE);
	} else if (monster_can_smell(c, mon)) {
		/* Monster can smell the player */
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
	bool woke_up = false;
	int stealth = player->state.skills[SKILL_STEALTH];
	int player_noise = 1 << (30 - stealth);
	int notice = randint0(1024);
	struct monster_lore *lore = get_lore(mon->race);

	/* Aggravation */
	if (player_of_has(player, OF_AGGRAVATE)) {
		char m_name[80];

		/* Wake the monster */
		mon_clear_timed(mon, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, false);

		/* Get the monster name */
		monster_desc(m_name, sizeof(m_name), mon,
					 MDESC_CAPITAL | MDESC_IND_HID);

		/* Notify the player if aware */
		if (monster_is_obvious(mon))
			msg("%s wakes up.", m_name);

		woke_up = true;

	} else if ((notice * notice * notice) <= player_noise) {
		int sleep_reduction = 1;
		int local_noise = c->noise.grids[mon->fy][mon->fx];

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
		mon_dec_timed(mon, MON_TMD_SLEEP, sleep_reduction, MON_TMD_FLG_NOTIFY,
					  false);

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
	}

	if (mon->m_timed[MON_TMD_FAST])
		mon_dec_timed(mon, MON_TMD_FAST, 1, 0, false);

	if (mon->m_timed[MON_TMD_SLOW])
		mon_dec_timed(mon, MON_TMD_SLOW, 1, 0, false);

	if (mon->m_timed[MON_TMD_HOLD])
		mon_dec_timed(mon, MON_TMD_HOLD, 1, 0, false);

	if (mon->m_timed[MON_TMD_STUN])
		mon_dec_timed(mon, MON_TMD_STUN, 1, MON_TMD_FLG_NOTIFY, false);

	if (mon->m_timed[MON_TMD_CONF]) {
		mon_dec_timed(mon, MON_TMD_CONF, 1, MON_TMD_FLG_NOTIFY, false);
	}

	if (mon->m_timed[MON_TMD_FEAR]) {
		int d = randint1(mon->race->level / 10 + 1);
		mon_dec_timed(mon, MON_TMD_FEAR, d, MON_TMD_FLG_NOTIFY, false);
	}

	/* One in __ chance of missing turn if stunned; always miss if held */
	if (mon->m_timed[MON_TMD_STUN]) {
		return randint0(STUN_MISS_CHANCE) == 1;
	} else if (mon->m_timed[MON_TMD_HOLD]) {
		return true;
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
					mon_dec_timed(mon, status, status_red, 0, false);
				}
			}
		}
	}
}
