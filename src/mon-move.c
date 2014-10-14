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
#include "init.h"
#include "monster.h"
#include "mon-attack.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-ignore.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-util.h"
#include "project.h"
#include "tables.h"
#include "trap.h"


/**
 * Monsters will run up to 5 grids out of sight
 */
#define FLEE_RANGE      MAX_SIGHT + 5

/**
 * Terrified monsters will turn to fight if they are slower than the
 * character, and closer than this distance.
 */
#define TURN_RANGE      5

/**
 * Calculate minimum and desired combat ranges.  -BR-
 */
static void find_range(monster_type *m_ptr)
{
	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

	/* All "afraid" monsters will run away */
	if (m_ptr->m_timed[MON_TMD_FEAR])
		m_ptr->min_range = FLEE_RANGE;

	else {

		/* Minimum distance - stay at least this far if possible */
		m_ptr->min_range = 1;

		/* Examine player power (level) */
		p_lev = player->lev;

		/* Hack - increase p_lev based on specialty abilities */

		/* Examine monster power (level plus morale) */
		m_lev = m_ptr->race->level + (m_ptr->midx & 0x08) + 25;

		/* Simple cases first */
		if (m_lev + 3 < p_lev)
			m_ptr->min_range = FLEE_RANGE;
		else if (m_lev - 5 < p_lev) {

			/* Examine player health */
			p_chp = player->chp;
			p_mhp = player->mhp;

			/* Examine monster health */
			m_chp = m_ptr->hp;
			m_mhp = m_ptr->maxhp;

			/* Prepare to optimize the calculation */
			p_val = (p_lev * p_mhp) + (p_chp << 2);	/* div p_mhp */
			m_val = (m_lev * m_mhp) + (m_chp << 2);	/* div m_mhp */

			/* Strong players scare strong monsters */
			if (p_val * m_mhp > m_val * p_mhp)
				m_ptr->min_range = FLEE_RANGE;
		}
	}

	if (m_ptr->min_range < FLEE_RANGE) {
		/* Creatures that don't move never like to get too close */
		if (rf_has(m_ptr->race->flags, RF_NEVER_MOVE))
			m_ptr->min_range += 3;

		/* Spellcasters that don't strike never like to get too close */
		if (rf_has(m_ptr->race->flags, RF_NEVER_BLOW))
			m_ptr->min_range += 3;
	}

	/* Maximum range to flee to */
	if (!(m_ptr->min_range < FLEE_RANGE))
		m_ptr->min_range = FLEE_RANGE;

	/* Nearby monsters won't run away */
	else if (m_ptr->cdis < TURN_RANGE)
		m_ptr->min_range = 1;

	/* Now find preferred range */
	m_ptr->best_range = m_ptr->min_range;

	/* Archers are quite happy at a good distance */
	//if (rf_has(m_ptr->race->flags, RF_ARCHER))
	//	m_ptr->best_range += 3;

	if (m_ptr->race->freq_spell > 24) {
		/* Breathers like point blank range */
		if (flags_test(m_ptr->race->spell_flags, RSF_SIZE, RSF_BREATH_MASK,
					   FLAG_END)
			&& (m_ptr->best_range < 6) && (m_ptr->hp > m_ptr->maxhp / 2))
			m_ptr->best_range = 6;

		/* Other spell casters will sit back and cast */
		else
			m_ptr->best_range += 3;
	}
}


/**
 * Lets the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 *
 * Returns TRUE if the monster successfully reproduced.
 */
bool multiply_monster(const monster_type *m_ptr)
{
	int i, y, x;

	bool result = FALSE;

	/* Try up to 18 times */
	for (i = 0; i < 18; i++) {
		int d = 1;

		/* Pick a location */
		scatter(cave, &y, &x, m_ptr->fy, m_ptr->fx, d, TRUE);

		/* Require an "empty" floor grid */
		if (!square_isempty(cave, y, x)) continue;

		/* Create a new monster (awake, no groups) */
		result = place_new_monster(cave, y, x, m_ptr->race, FALSE, FALSE,
			ORIGIN_DROP_BREED);

		/* Done */
		break;
	}

	/* Result */
	return (result);
}


/* From Will Asher in DJA:
 * Find whether a monster is near a permanent wall
 * this decides whether PASS_WALL & KILL_WALL monsters 
 * use the monster flow code
 */
static bool near_permwall(const monster_type *m_ptr, struct chunk *c)
{
	int y, x;
	int my = m_ptr->fy;
	int mx = m_ptr->fx;
	
	/* if PC is in LOS, there's no need to go around walls */
    if (projectable(cave, my, mx, player->py, player->px, PROJECT_NONE)) 
		return FALSE;
    
    /* PASS_WALL & KILL_WALL monsters occasionally flow for a turn anyway */
    if (randint0(99) < 5) return TRUE;
    
	/* Search the nearby grids, which are always in bounds */
	for (y = (my - 2); y <= (my + 2); y++)
	{
		for (x = (mx - 2); x <= (mx + 2); x++)
		{
            if (!square_in_bounds_fully(c, y, x)) continue;
            if (square_isperm(c, y, x)) return TRUE;
		}
	}
	return FALSE;
}


/*
 * Choose the best direction for "flowing".
 *
 * Note that ghosts and rock-eaters generally don't flow because they can move
 * through obstacles.
 *
 * Monsters first try to use up-to-date distance information ('sound') as
 * saved in cave->cost.  Failing that, they'll try using scent ('when')
 * which is just old cost information.
 *
 * Tracking by 'scent' means that monsters end up near enough the player to
 * switch to 'sound' (cost), or they end up somewhere the player left via 
 * teleport.  Teleporting away from a location will cause the monsters who
 * were chasing the player to converge on that location as long as the player
 * is still near enough to "annoy" them without being close enough to chase
 * directly.
 */
static bool get_moves_flow(struct chunk *c, struct monster *m_ptr, int *yp, int *xp)
{
	int i;

	int best_when = 0;
	int best_cost = 999;
	int best_direction = 0;

	int py = player->py, px = player->px;
	int my = m_ptr->fy, mx = m_ptr->fx;

	/* Only use this algorithm for passwall monsters if near permanent walls, to avoid getting snagged */
	if (flags_test(m_ptr->race->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL, FLAG_END) &&
			!near_permwall(m_ptr, c))
		return (FALSE);

	/* If the player has never been near this grid, abort */
	if (c->when[my][mx] == 0) return FALSE;

	/* Monster is too far away to notice the player */
	if (c->cost[my][mx] > z_info->max_flow_depth) return FALSE;
	if (c->cost[my][mx] > (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)) return FALSE;

	/* If the player can see monster, run towards them */
	if (player_has_los_bold(my, mx)) return FALSE;

	/* Check nearby grids, diagonals first */
	/* This gives preference to the cardinal directions */
	for (i = 7; i >= 0; i--)
	{
		/* Get the location */
		int y = my + ddy_ddd[i];
		int x = mx + ddx_ddd[i];

		/* Ignore unvisited/unpassable locations */
		if (c->when[y][x] == 0) continue;

		/* Ignore locations whose data is more stale */
		if (c->when[y][x] < best_when) continue;

		/* Ignore locations which are farther away */
		if (c->cost[y][x] > best_cost) continue;

		/* Save the cost and time */
		best_when = c->when[y][x];
		best_cost = c->cost[y][x];
		best_direction = i;
	}

	/* Save the location to flow toward */
 	/* We multiply by 16 to angle slightly toward the player's actual location */
	if (best_direction) {
		(*yp) = py + 16 * ddy_ddd[best_direction];
		(*xp) = px + 16 * ddx_ddd[best_direction];
		return TRUE;
	}

	return FALSE;
}

/*
 * Provide a location to flee to, but give the player a wide berth.
 *
 * A monster may wish to flee to a location that is behind the player,
 * but instead of heading directly for it, the monster should "swerve"
 * around the player so that it has a smaller chance of getting hit.
 */
static bool get_moves_fear(struct chunk *c, struct monster *m_ptr, int *yp, int *xp)
{
	int i;
	int gy = 0, gx = 0;
	int best_when = 0, best_score = -1;

	int py = player->py, px = player->px;
	int my = m_ptr->fy, mx = m_ptr->fx;

	/* Desired destination, relative to current position */
	int dy = my - (*yp);
	int dx = mx - (*xp);

	/* If the player is not currently near the monster, no reason to flow */
	if (c->when[my][mx] < c->when[py][px])
		return FALSE;

	/* Monster is too far away to use flow information */
	if (c->cost[my][mx] > z_info->max_flow_depth) return FALSE;
	if (c->cost[my][mx] > (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)) return FALSE;

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--)
	{
		int dis, score;

		/* Get the location */
		int y = my + ddy_ddd[i];
		int x = mx + ddx_ddd[i];

		/* Ignore illegal & older locations */
		if (c->when[y][x] == 0 || c->when[y][x] < best_when) continue;

		/* Calculate distance of this grid from our destination */
		dis = distance(y, x, dy, dx);

		/* Score this grid */
		/* First half of calculation is inversely proportional to distance */
		/* Second half is inversely proportional to grid's distance from player */
		score = 5000 / (dis + 3) - 500 / (c->cost[y][x] + 1);

		/* No negative scores */
		if (score < 0) score = 0;

		/* Ignore lower scores */
		if (score < best_score) continue;

		/* Save the score and time */
		best_when = c->when[y][x];
		best_score = score;

		/* Save the location */
		gy = y;
		gx = x;
	}

	/* No legal move (?) */
	if (!best_when) return FALSE;

	/* Find deltas */
	(*yp) = my - gy;
	(*xp) = mx - gx;

	/* Success */
	return TRUE;
}



/*
 * Hack -- Precompute a bunch of calls to distance() in find_safety() and
 * find_hiding().
 *
 * The pair of arrays dist_offsets_y[n] and dist_offsets_x[n] contain the
 * offsets of all the locations with a distance of n from a central point,
 * with an offset of (0,0) indicating no more offsets at this distance.
 *
 * This is, of course, fairly unreadable, but it eliminates multiple loops
 * from the previous version.
 *
 * It is probably better to replace these arrays with code to compute
 * the relevant arrays, even if the storage is pre-allocated in hard
 * coded sizes.  At the very least, code should be included which is
 * able to generate and dump these arrays (ala "los()").  XXX XXX XXX
 *
 * Also, the storage needs could be reduced by using char.  XXX XXX XXX
 */


static const int d_off_y_0[] =
{ 0 };

static const int d_off_x_0[] =
{ 0 };


static const int d_off_y_1[] =
{ -1, -1, -1, 0, 0, 1, 1, 1, 0 };

static const int d_off_x_1[] =
{ -1, 0, 1, -1, 1, -1, 0, 1, 0 };


static const int d_off_y_2[] =
{ -1, -1, -2, -2, -2, 0, 0, 1, 1, 2, 2, 2, 0 };

static const int d_off_x_2[] =
{ -2, 2, -1, 0, 1, -2, 2, -2, 2, -1, 0, 1, 0 };


static const int d_off_y_3[] =
{ -1, -1, -2, -2, -3, -3, -3, 0, 0, 1, 1, 2, 2,
  3, 3, 3, 0 };

static const int d_off_x_3[] =
{ -3, 3, -2, 2, -1, 0, 1, -3, 3, -3, 3, -2, 2,
  -1, 0, 1, 0 };


static const int d_off_y_4[] =
{ -1, -1, -2, -2, -3, -3, -3, -3, -4, -4, -4, 0,
  0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 0 };

static const int d_off_x_4[] =
{ -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, -4, 4,
  -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, 0 };


static const int d_off_y_5[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -4, -4, -5, -5,
  -5, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5,
  5, 0 };

static const int d_off_x_5[] =
{ -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1, 0, 1,
  -5, 5, -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1,
  0, 1, 0 };


static const int d_off_y_6[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
  5, 5, 6, 6, 6, 0 };

static const int d_off_x_6[] =
{ -6, 6, -5, 5, -5, 5, -4, 4, -2, -3, 2, 3, -1,
  0, 1, -6, 6, -6, 6, -5, 5, -5, 5, -4, 4, -2,
  -3, 2, 3, -1, 0, 1, 0 };


static const int d_off_y_7[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, -6, -7, -7, -7, 0, 0, 1, 1, 2, 2, 3,
  3, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 0 };

static const int d_off_x_7[] =
{ -7, 7, -6, 6, -6, 6, -5, 5, -4, -5, 4, 5, -2,
  -3, 2, 3, -1, 0, 1, -7, 7, -7, 7, -6, 6, -6,
  6, -5, 5, -4, -5, 4, 5, -2, -3, 2, 3, -1, 0,
  1, 0 };


static const int d_off_y_8[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -6, -6, -7, -7, -7, -7, -8, -8, -8, 0, 0, 1, 1,
  2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
  8, 8, 8, 0 };

static const int d_off_x_8[] =
{ -8, 8, -7, 7, -7, 7, -6, 6, -6, 6, -4, -5, 4,
  5, -2, -3, 2, 3, -1, 0, 1, -8, 8, -8, 8, -7,
  7, -7, 7, -6, 6, -6, 6, -4, -5, 4, 5, -2, -3,
  2, 3, -1, 0, 1, 0 };


static const int d_off_y_9[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -7, -7, -7, -7, -8, -8, -8, -8, -9, -9, -9, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7,
  7, 8, 8, 8, 8, 9, 9, 9, 0 };

static const int d_off_x_9[] =
{ -9, 9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4,
  -5, 4, 5, -2, -3, 2, 3, -1, 0, 1, -9, 9, -9,
  9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4, -5,
  4, 5, -2, -3, 2, 3, -1, 0, 1, 0 };


static const int *dist_offsets_y[10] =
{
	d_off_y_0, d_off_y_1, d_off_y_2, d_off_y_3, d_off_y_4,
	d_off_y_5, d_off_y_6, d_off_y_7, d_off_y_8, d_off_y_9
};

static const int *dist_offsets_x[10] =
{
	d_off_x_0, d_off_x_1, d_off_x_2, d_off_x_3, d_off_x_4,
	d_off_x_5, d_off_x_6, d_off_x_7, d_off_x_8, d_off_x_9
};


/*
 * Choose a "safe" location near a monster for it to run toward.
 *
 * A location is "safe" if it can be reached quickly and the player
 * is not able to fire into it (it isn't a "clean shot").  So, this will
 * cause monsters to "duck" behind walls.  Hopefully, monsters will also
 * try to run towards corridor openings if they are in a room.
 *
 * This function may take lots of CPU time if lots of monsters are fleeing.
 *
 * Return TRUE if a safe location is available.
 */
static bool find_safety(struct chunk *c, struct monster *m_ptr, int *yp, int *xp)
{
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int py = player->py;
	int px = player->px;

	int i, y, x, dy, dx, d, dis;
	int gy = 0, gx = 0, gdis = 0;

	const int *y_offsets;
	const int *x_offsets;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++)
	{
		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i])
		{
			y = fy + dy;
			x = fx + dx;

			/* Skip illegal locations */
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Skip locations in a wall */
			if (!square_ispassable(cave, y, x)) continue;

			/* Ignore grids very far from the player */
			if (c->when[y][x] < c->when[py][px]) continue;

			/* Ignore too-distant grids */
			if (c->cost[y][x] > c->cost[fy][fx] + 2 * d) continue;

			/* Check for absence of shot (more or less) */
			if (!player_has_los_bold(y,x))
			{
				/* Calculate distance from player */
				dis = distance(y, x, py, px);

				/* Remember if further than previous */
				if (dis > gdis)
				{
					gy = y;
					gx = x;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis > 0)
		{
			/* Good location */
			(*yp) = fy - gy;
			(*xp) = fx - gx;

			/* Found safe place */
			return (TRUE);
		}
	}

	/* No safe place */
	return (FALSE);
}




/*
 * Choose a good hiding place near a monster for it to run toward.
 *
 * Pack monsters will use this to "ambush" the player and lure him out
 * of corridors into open space so they can swarm him.
 *
 * Return TRUE if a good location is available.
 */
static bool find_hiding(struct monster *m_ptr, int *yp, int *xp)
{
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int py = player->py;
	int px = player->px;

	int i, y, x, dy, dx, d, dis;
	int gy = 0, gx = 0, gdis = 999, min;

	const int *y_offsets, *x_offsets;

	/* Closest distance to get */
	min = distance(py, px, fy, fx) * 3 / 4 + 2;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++)
	{
		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i])
		{
			y = fy + dy;
			x = fx + dx;

			/* Skip illegal locations */
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Skip occupied locations */
			if (!square_isempty(cave, y, x)) continue;

			/* Check for hidden, available grid */
			if (!player_has_los_bold(y, x) &&
				projectable(cave, fy, fx, y, x, PROJECT_STOP))
			{
				/* Calculate distance from player */
				dis = distance(y, x, py, px);

				/* Remember if closer than previous */
				if (dis < gdis && dis >= min)
				{
					gy = y;
					gx = x;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis < 999)
		{
			/* Good location */
			(*yp) = fy - gy;
			(*xp) = fx - gx;

			/* Found good place */
			return (TRUE);
		}
	}

	/* No good place */
	return (FALSE);
}


/*
 * Choose "logical" directions for monster movement
 *
 * We store the directions in a special "mm" array
 */
static bool get_moves(struct chunk *c, struct monster *m_ptr, int mm[5])
{
	int py = player->py;
	int px = player->px;

	int y, ay, x, ax;

	int move_val = 0;

	int y2 = py;
	int x2 = px;

	bool done = FALSE;

	/* Calculate range */
	find_range(m_ptr);

	/* Flow towards the player */
	get_moves_flow(c, m_ptr, &y2, &x2);

	/* Extract the "pseudo-direction" */
	y = m_ptr->fy - y2;
	x = m_ptr->fx - x2;



	/* Normal animal packs try to get the player out of corridors. */
	if (rf_has(m_ptr->race->flags, RF_GROUP_AI) &&
	    !flags_test(m_ptr->race->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL, FLAG_END))
	{
		int i, open = 0;

		/* Count empty grids next to player */
		for (i = 0; i < 8; i++)
		{
			int ry = py + ddy_ddd[i];
			int rx = px + ddx_ddd[i];
			/* Check grid around the player for room interior (room walls count)
			   or other empty space */
			if (square_ispassable(cave, ry, rx) || square_isroom(cave, ry, rx))
			{
				/* One more open grid */
				open++;
			}
		}

		/* Not in an empty space and strong player */
		if ((open < 7) && (player->chp > player->mhp / 2))
		{
			/* Find hiding place */
			if (find_hiding(m_ptr, &y, &x)) done = TRUE;
		}
	}


	/* Apply fear */
	if (!done && (m_ptr->min_range == FLEE_RANGE))
	{
		/* Try to find safe place */
		if (!find_safety(c, m_ptr, &y, &x))
		{
			/* This is not a very "smart" method XXX XXX */
			y = (-y);
			x = (-x);
		}

		else
		{
			/* Adjust movement */
			get_moves_fear(c, m_ptr, &y, &x);
		}

		done = TRUE;
	}


	/* Monster groups try to surround the player */
	if (!done && rf_has(m_ptr->race->flags, RF_GROUP_AI))
	{
		int i;

		/* If we are not already adjacent */
		if (m_ptr->cdis > 1)
		{
			/* Find an empty square near the player to fill */
			int tmp = randint0(8);
			for (i = 0; i < 8; i++)
			{
				/* Pick squares near player (pseudo-randomly) */
				y2 = py + ddy_ddd[(tmp + i) & 7];
				x2 = px + ddx_ddd[(tmp + i) & 7];
				
				/* Ignore filled grids */
				if (!square_isempty(cave, y2, x2)) continue;
				
				/* Try to fill this hole */
				break;
			}
		}
		/* Extract the new "pseudo-direction" */
		y = m_ptr->fy - y2;
		x = m_ptr->fx - x2;
	}


	/* Check for no move */
	if (!x && !y) return (FALSE);

	/* Extract the "absolute distances" */
	ax = ABS(x);
	ay = ABS(y);

	/* Do something weird */
	if (y < 0) move_val += 8;
	if (x > 0) move_val += 4;

	/* Prevent the diamond maneuvre */
	if (ay > (ax << 1))
	{
		move_val++;
		move_val++;
	}
	else if (ax > (ay << 1))
	{
		move_val++;
	}

	/* Analyze */
	switch (move_val)
	{
		case 0:
		{
			mm[0] = 9;
			if (ay > ax)
			{
				mm[1] = 8;
				mm[2] = 6;
				mm[3] = 7;
				mm[4] = 3;
			}
			else
			{
				mm[1] = 6;
				mm[2] = 8;
				mm[3] = 3;
				mm[4] = 7;
			}
			break;
		}

		case 1:
		case 9:
		{
			mm[0] = 6;
			if (y < 0)
			{
				mm[1] = 3;
				mm[2] = 9;
				mm[3] = 2;
				mm[4] = 8;
			}
			else
			{
				mm[1] = 9;
				mm[2] = 3;
				mm[3] = 8;
				mm[4] = 2;
			}
			break;
		}

		case 2:
		case 6:
		{
			mm[0] = 8;
			if (x < 0)
			{
				mm[1] = 9;
				mm[2] = 7;
				mm[3] = 6;
				mm[4] = 4;
			}
			else
			{
				mm[1] = 7;
				mm[2] = 9;
				mm[3] = 4;
				mm[4] = 6;
			}
			break;
		}

		case 4:
		{
			mm[0] = 7;
			if (ay > ax)
			{
				mm[1] = 8;
				mm[2] = 4;
				mm[3] = 9;
				mm[4] = 1;
			}
			else
			{
				mm[1] = 4;
				mm[2] = 8;
				mm[3] = 1;
				mm[4] = 9;
			}
			break;
		}

		case 5:
		case 13:
		{
			mm[0] = 4;
			if (y < 0)
			{
				mm[1] = 1;
				mm[2] = 7;
				mm[3] = 2;
				mm[4] = 8;
			}
			else
			{
				mm[1] = 7;
				mm[2] = 1;
				mm[3] = 8;
				mm[4] = 2;
			}
			break;
		}

		case 8:
		{
			mm[0] = 3;
			if (ay > ax)
			{
				mm[1] = 2;
				mm[2] = 6;
				mm[3] = 1;
				mm[4] = 9;
			}
			else
			{
				mm[1] = 6;
				mm[2] = 2;
				mm[3] = 9;
				mm[4] = 1;
			}
			break;
		}

		case 10:
		case 14:
		{
			mm[0] = 2;
			if (x < 0)
			{
				mm[1] = 3;
				mm[2] = 1;
				mm[3] = 6;
				mm[4] = 4;
			}
			else
			{
				mm[1] = 1;
				mm[2] = 3;
				mm[3] = 4;
				mm[4] = 6;
			}
			break;
		}

		default: /* case 12: */
		{
			mm[0] = 1;
			if (ay > ax)
			{
				mm[1] = 2;
				mm[2] = 4;
				mm[3] = 3;
				mm[4] = 7;
			}
			else
			{
				mm[1] = 4;
				mm[2] = 2;
				mm[3] = 7;
				mm[4] = 3;
			}
			break;
		}
	}

	/* Want to move */
	return (TRUE);
}



/**
 * Process a monster's timed effects, e.g. decrease them.
 *
 * Returns TRUE if the monster is skipping its turn.
 */
static bool process_monster_timed(struct chunk *c, struct monster *m_ptr)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Handle "sleep" */
	if (m_ptr->m_timed[MON_TMD_SLEEP]) {
		bool woke_up = FALSE;

		/* Anti-stealth */
		int notice = randint0(1024);

		/* Aggravation */
		if (player_of_has(player, OF_AGGRAVATE)) {
			/* Wake the monster and notify player */
			mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOTIFY, FALSE);
			woke_up = TRUE;

		/* Hack See if monster "notices" player */
		} else if ((notice * notice * notice) <= player->state.noise) {
			int d = 1;

			/* Wake up faster near the player */
			if (m_ptr->cdis < 50) d = (100 / m_ptr->cdis);

			/* Note a complete wakeup */
			if (m_ptr->m_timed[MON_TMD_SLEEP] <= d) woke_up = TRUE;

			/* Monster wakes up a bit */
			mon_dec_timed(m_ptr, MON_TMD_SLEEP, d, MON_TMD_FLG_NOTIFY, FALSE);

			/* Update knowledge */
			if (m_ptr->ml && !m_ptr->unaware) {
				if (!woke_up && l_ptr->ignore < MAX_UCHAR)
					l_ptr->ignore++;
				else if (woke_up && l_ptr->wake < MAX_UCHAR)
					l_ptr->wake++;
				lore_update(m_ptr->race, l_ptr);
			}
		}

		/* Update the health bar */
		if (woke_up && m_ptr->ml && !m_ptr->unaware &&
				player->upkeep->health_who == m_ptr)
			player->upkeep->redraw |= (PR_HEALTH);

		/* Sleeping monsters don't recover in any other ways */
		/* If the monster just woke up, then it doesn't act */
		return TRUE;
	}

	if (m_ptr->m_timed[MON_TMD_FAST])
		mon_dec_timed(m_ptr, MON_TMD_FAST, 1, 0, FALSE);

	if (m_ptr->m_timed[MON_TMD_SLOW])
		mon_dec_timed(m_ptr, MON_TMD_SLOW, 1, 0, FALSE);

	if (m_ptr->m_timed[MON_TMD_STUN]) {
		int d = 1;

		/* Make a "saving throw" against stun */
		if (randint0(5000) <= m_ptr->race->level * m_ptr->race->level)
			/* Recover fully */
			d = m_ptr->m_timed[MON_TMD_STUN];

		/* Hack -- Recover from stun */
		mon_dec_timed(m_ptr, MON_TMD_STUN, d, MON_TMD_FLG_NOTIFY, FALSE);
	}

	if (m_ptr->m_timed[MON_TMD_CONF]) {
		int d = randint1(m_ptr->race->level / 10 + 1);
		mon_dec_timed(m_ptr, MON_TMD_CONF, d, MON_TMD_FLG_NOTIFY, FALSE);
	}

	if (m_ptr->m_timed[MON_TMD_FEAR]) {
		int d = randint1(m_ptr->race->level / 10 + 1);
		mon_dec_timed(m_ptr, MON_TMD_FEAR, d, MON_TMD_FLG_NOTIFY, FALSE);
	}

	/* Don't do anything if stunned */
	return m_ptr->m_timed[MON_TMD_STUN] ? TRUE : FALSE;
}


/**
 * Attempt to reproduce, if possible.  All monsters are checked here for
 * lore purposes, the unfit fail.
 */
static bool process_monster_multiply(struct chunk *c, struct monster *m_ptr)
{
	int oy = m_ptr->fy;
	int ox = m_ptr->fx;

	int k = 0, y, x;

	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Too many breeders on the level already */
	if (num_repro >= z_info->repro_monster_max) return FALSE;

	/* Count the adjacent monsters */
	for (y = oy - 1; y <= m_ptr->fy + 1; y++)
		for (x = ox - 1; x <= m_ptr->fx + 1; x++)
			if (c->m_idx[y][x] > 0) k++;

	/* Multiply slower in crowded areas */
	if ((k < 4) && (k == 0 || one_in_(k * z_info->repro_monster_rate))) {
		/* Successful breeding attempt, learn about that now */
		if (m_ptr->ml)
			rf_on(l_ptr->flags, RF_MULTIPLY);

		/* Leave now if not a breeder */
		if (!rf_has(m_ptr->race->flags, RF_MULTIPLY))
			return FALSE;

		/* Try to multiply */
		if (multiply_monster(m_ptr)) {
			/* Make a sound */
			if (m_ptr->ml)
				sound(MSG_MULTIPLY);

			/* Multiplying takes energy */
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Check if a monster should stagger or not.  Always stagger when confused,
 * but also deal with random movement for RAND_25 and _50 monsters.
 */
static bool process_monster_should_stagger(struct monster *m_ptr)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	int chance = 0;

	/* Confused */
	if (m_ptr->m_timed[MON_TMD_CONF])
		return TRUE;

	/* RAND_25 and RAND_50 are cumulative */
	if (rf_has(m_ptr->race->flags, RF_RAND_25)) {
		chance += 25;
		if (m_ptr->ml) rf_on(l_ptr->flags, RF_RAND_25);
	}

	if (rf_has(m_ptr->race->flags, RF_RAND_50)) {
		chance += 50;
		if (m_ptr->ml) rf_on(l_ptr->flags, RF_RAND_50);
	}

	return randint0(100) < chance;
}


/**
 * Work out if a monster can move through the grid, if necessary bashing 
 * down doors in the way.
 *
 * Returns TRUE if the monster is able to move through the grid.
 */
static bool process_monster_can_move(struct chunk *c, struct monster *m_ptr,
		const char *m_name, int nx, int ny, bool *did_something)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Floor is open? */
	if (square_ispassable(c, ny, nx))
		return TRUE;

	/* Permanent wall in the way */
	if (square_iswall(c, ny, nx) && square_isperm(c, ny, nx))
		return FALSE;

	/* Normal wall, door, or secret door in the way */

	/* There's some kind of feature in the way, so learn about
	 * kill-wall and pass-wall now */
	if (m_ptr->ml) {
		rf_on(l_ptr->flags, RF_PASS_WALL);
		rf_on(l_ptr->flags, RF_KILL_WALL);
	}

	/* Monster moves through walls (and doors) */
	if (rf_has(m_ptr->race->flags, RF_PASS_WALL)) 
		return TRUE;

	/* Monster destroys walls (and doors) */
	else if (rf_has(m_ptr->race->flags, RF_KILL_WALL)) {
		/* Forget the wall */
		sqinfo_off(c->info[ny][nx], SQUARE_MARK);

		/* Notice */
		square_destroy_wall(c, ny, nx);

		/* Note changes to viewable region */
		if (player_has_los_bold(ny, nx))
			player->upkeep->update |= PU_UPDATE_VIEW;

		/* Update the flow, since walls affect flow */
		player->upkeep->update |= PU_UPDATE_FLOW;

		return TRUE;
	}

	/* Handle doors and secret doors */
	else if (square_iscloseddoor(c, ny, nx) || square_issecretdoor(c, ny, nx)) {
		bool may_bash = rf_has(m_ptr->race->flags, RF_BASH_DOOR) && one_in_(2);

		/* Take a turn */
		*did_something = TRUE;

		/* Learn about door abilities */
		if (m_ptr->ml) {
			rf_on(l_ptr->flags, RF_OPEN_DOOR);
			rf_on(l_ptr->flags, RF_BASH_DOOR);
		}

		/* Creature can open or bash doors */
		if (!rf_has(m_ptr->race->flags, RF_OPEN_DOOR) && !rf_has(m_ptr->race->flags, RF_BASH_DOOR))
			return FALSE;

		/* Stuck door -- try to unlock it */
		if (square_islockeddoor(c, ny, nx)) {
			int k = square_door_power(c, ny, nx);

			if (randint0(m_ptr->hp / 10) > k) {
				if (may_bash)
					msg("%s slams against the door.", m_name);
				else
					msg("%s fiddles with the lock.", m_name);

				/* Reduce the power of the door by one */
				square_set_door_lock(c, ny, nx, k - 1);
			}
		} else {
			/* Handle viewable doors */
			if (player_has_los_bold(ny, nx))
				player->upkeep->update |= PU_UPDATE_VIEW;

			/* Closed or secret door -- open or bash if allowed */
			if (may_bash) {
				square_smash_door(c, ny, nx);

				msg("You hear a door burst open!");
				disturb(player, 0);

				/* Fall into doorway */
				return TRUE;
			} else if (rf_has(m_ptr->race->flags, RF_OPEN_DOOR)) {
				square_open_door(c, ny, nx);
			}
		}
	}

	return FALSE;
}

/**
 * Try to break a glyph.
 */
static bool process_monster_glyph(struct chunk *c, struct monster *m_ptr,
								  int nx, int ny)
{
	assert(square_iswarded(c, ny, nx));

	/* Break the ward */
	if (randint1(z_info->glyph_hardness) < m_ptr->race->level) {
		/* Describe observable breakage */
		if (square_ismark(c, ny, nx))
			msg("The rune of protection is broken!");

		/* Forget the rune */
		sqinfo_off(c->info[ny][nx], SQUARE_MARK);

		/* Break the rune */
		square_remove_ward(c, ny, nx);

		return TRUE;
	}

	/* Unbroken ward - can't move */
	return FALSE;
}

/*
 * Hack -- compare the "strength" of two monsters XXX XXX XXX
 */
static int compare_monsters(const struct monster *m_ptr,
							const struct monster *n_ptr)
{
	u32b mexp1 = m_ptr->race->mexp;
	u32b mexp2 = n_ptr->race->mexp;

	/* Compare */
	if (mexp1 < mexp2) return (-1);
	if (mexp1 > mexp2) return (1);

	/* Assume equal */
	return (0);
}

/**
 * Try to push past / kill another monster.  Returns TRUE on success.
 */
static bool process_monster_try_push(struct chunk *c, struct monster *m_ptr, const char *m_name, int nx, int ny)
{
	monster_type *n_ptr = square_monster(c, ny, nx);
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Kill weaker monsters */
	int kill_ok = rf_has(m_ptr->race->flags, RF_KILL_BODY);

	/* Move weaker monsters if they can swap places */
	/* (not in a wall) */
	int move_ok = (rf_has(m_ptr->race->flags, RF_MOVE_BODY) &&
				   square_ispassable(c, m_ptr->fy, m_ptr->fx));

	if (compare_monsters(m_ptr, n_ptr) > 0) {
		/* Learn about pushing and shoving */
		if (m_ptr->ml) {
			rf_on(l_ptr->flags, RF_KILL_BODY);
			rf_on(l_ptr->flags, RF_MOVE_BODY);
		}

		if (kill_ok || move_ok) {
			/* Get the names of the monsters involved */
			char n_name[80];
			monster_desc(n_name, sizeof(n_name), n_ptr, MDESC_IND_HID);

			/* Reveal mimics */
			if (is_mimicking(n_ptr))
				become_aware(n_ptr);

			/* Note if visible */
			if (m_ptr->ml && (m_ptr->mflag & MFLAG_VIEW))
				msg("%s %s %s.", kill_ok ? "tramples over" : "pushes past", m_name, n_name);

			/* Monster ate another monster */
			if (kill_ok)
				delete_monster(ny, nx);

			monster_swap(m_ptr->fy, m_ptr->fx, ny, nx);
			return TRUE;
		} 
	}

	return FALSE;
}

/**
 * Grab all objects from the grid.
 */
void process_monster_grab_objects(struct chunk *c, struct monster *m_ptr, 
		const char *m_name, int nx, int ny)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);
	s16b this_o_idx, next_o_idx = 0;

	bool is_item = square_object(c, ny, nx);
	if (is_item && m_ptr->ml) {
		rf_on(l_ptr->flags, RF_TAKE_ITEM);
		rf_on(l_ptr->flags, RF_KILL_ITEM);
	}

	/* Abort if can't pickup/kill */
	if (!rf_has(m_ptr->race->flags, RF_TAKE_ITEM) &&
			!rf_has(m_ptr->race->flags, RF_KILL_ITEM)) {
		return;
	}

	/* Take or kill objects on the floor */
	for (this_o_idx = c->o_idx[ny][nx]; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr = cave_object(c, this_o_idx);
		char o_name[80];
		bool safe = o_ptr->artifact ? TRUE : FALSE;

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Skip gold */
		if (tval_is_money(o_ptr)) continue;

		/* Get the object name */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* React to objects that hurt the monster */
		if (react_to_slay(o_ptr, m_ptr))
			safe = TRUE;

		/* The object cannot be picked up by the monster */
		if (safe) {
			/* Only give a message for "take_item" */
			if (rf_has(m_ptr->race->flags, RF_TAKE_ITEM) &&
						m_ptr->ml && player_has_los_bold(ny, nx) &&
						!ignore_item_ok(o_ptr)) {
				/* Dump a message */
				msg("%s tries to pick up %s, but fails.", m_name, o_name);
			}

		/* Pick up the item */
		} else if (rf_has(m_ptr->race->flags, RF_TAKE_ITEM)) {
			object_type *i_ptr;
			object_type object_type_body;

			/* Describe observable situations */
			if (player_has_los_bold(ny, nx) && !ignore_item_ok(o_ptr))
				msg("%s picks up %s.", m_name, o_name);

			/* Get local object */
			i_ptr = &object_type_body;

			/* Obtain local object */
			object_copy(i_ptr, o_ptr);

			/* Delete the object */
			delete_object_idx(this_o_idx);

			/* Carry the object */
			monster_carry(c, m_ptr, i_ptr);

		/* Destroy the item */
		} else {
			/* Describe observable situations */
			if (player_has_los_bold(ny, nx) && !ignore_item_ok(o_ptr))
				msgt(MSG_DESTROY, "%s crushes %s.", m_name, o_name);

			/* Delete the object */
			delete_object_idx(this_o_idx);
		}
	}
}

/*
 * Process a monster
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
static void process_monster(struct chunk *c, struct monster *m_ptr)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	bool did_something = FALSE;

	int i;
	int mm[5];
	bool stagger = FALSE;
	char m_name[80];

	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_CAPITAL | MDESC_IND_HID);

	/* Try to multiply - this can use up a turn */
	if (process_monster_multiply(c, m_ptr))
		return;

	/* Attempt to cast a spell */
	if (make_attack_spell(m_ptr)) return;

	/* Work out what kind of movement to use - AI or staggered movement */
	if (!process_monster_should_stagger(m_ptr)) {
		if (!get_moves(c, m_ptr, mm)) return;
	} else {
		stagger = TRUE;
	}

	/* Process moves */
	for (i = 0; i < 5 && !did_something; i++) {
		int oy = m_ptr->fy;
		int ox = m_ptr->fx;

		/* Get the direction (or stagger) */
		int d = (stagger ? ddd[randint0(8)] : mm[i]);

		/* Get the destination */
		int ny = oy + ddy[d];
		int nx = ox + ddx[d];

		/* Check if we can move */
		if (!process_monster_can_move(c, m_ptr, m_name, nx, ny, &did_something))
			continue;

		/* Try to break the glyph if there is one */
		/* This can happen multiple times per turn because failure does not break the loop */
		if (square_iswarded(c, ny, nx) && !process_monster_glyph(c, m_ptr, nx, ny))
			continue;

		/* The player is in the way. */
		if (square_isplayer(c, ny, nx)) {
			/* Learn about if the monster attacks */
			if (m_ptr->ml)
				rf_on(l_ptr->flags, RF_NEVER_BLOW);

			/* Some monsters never attack */
			if (rf_has(m_ptr->race->flags, RF_NEVER_BLOW))
				continue;

			/* Otherwise, attack the player */
			make_attack_normal(m_ptr, player);

			did_something = TRUE;
			break;
		} else {
			/* Some monsters never move */
			if (rf_has(m_ptr->race->flags, RF_NEVER_MOVE)) {
				/* Learn about lack of movement */
				if (m_ptr->ml)
					rf_on(l_ptr->flags, RF_NEVER_MOVE);

				return;
			}
		}

		/* A monster is in the way, try to push past/kill */
		if (square_monster(c, ny, nx)) {
			did_something = process_monster_try_push(c, m_ptr, m_name, nx, ny);
		} else {
			/* Otherwise we can just move */
			monster_swap(oy, ox, ny, nx);
			did_something = TRUE;
		}

		/* Scan all objects in the grid */
		process_monster_grab_objects(c, m_ptr, m_name, nx, ny);
	}

	if (did_something) {
		/* Learn about no lack of movement */
		if (m_ptr->ml) rf_on(l_ptr->flags, RF_NEVER_MOVE);

		/* Possible disturb */
		if (m_ptr->ml && (m_ptr->mflag & MFLAG_VIEW) && OPT(disturb_near))
			disturb(player, 0);		
	}

	/* Hack -- get "bold" if out of options */
	if (!did_something && m_ptr->m_timed[MON_TMD_FEAR])
		mon_clear_timed(m_ptr, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY, FALSE);

	/* If we see an unaware monster do something, become aware of it */
	if (did_something && m_ptr->unaware)
		become_aware(m_ptr);
}


static bool monster_can_flow(struct chunk *c, struct monster *m_ptr)
{
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	assert(c);

	/* Check the flow (normal aaf is about 20) */
	if ((c->when[fy][fx] == c->when[player->py][player->px]) &&
	    (c->cost[fy][fx] < z_info->max_flow_depth) &&
	    (c->cost[fy][fx] < (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)))
		return TRUE;
	return FALSE;
}

/*
 * Process all the "live" monsters, once per game turn.
 *
 * During each game turn, we scan through the list of all the "live" monsters,
 * (backwards, so we can excise any "freshly dead" monsters), energizing each
 * monster, and allowing fully energized monsters to move, attack, pass, etc.
 *
 * Note that monsters can never move in the monster array (except when the
 * "compact_monsters()" function is called by "dungeon()" or "save_player()").
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.
 *
 * When the player is resting, virtually 90% of the processor time is spent
 * in this function, and its children, "process_monster()" and "make_move()".
 *
 * Most of the rest of the time is spent in "update_view()" and "light_spot()",
 * especially when the player is running.
 *
 * Note the special "MFLAG_NICE" flag, which prevents "nasty" monsters from
 * using any of their spell attacks until the player gets a turn.
 */
void process_monsters(struct chunk *c, byte minimum_energy)
{
	int i;

	/* Process the monsters (backwards) */
	for (i = cave_monster_max(c) - 1; i >= 1; i--)
	{
		monster_type *m_ptr;

		/* Handle "leaving" */
		if (player->upkeep->leaving) break;

		/* Get a 'live' monster */
		m_ptr = cave_monster(cave, i);
		if (!m_ptr->race) continue;

		/* Not enough energy to move */
		if (m_ptr->energy < minimum_energy) continue;

		/* Use up "some" energy */
		m_ptr->energy -= 100;

		/* Mimics lie in wait */
		if (is_mimicking(m_ptr)) continue;

		/*
		 * Process the monster if the monster either:
		 * - can "sense" the player
		 * - is hurt
		 * - can "see" the player (checked backwards)
		 * - can "smell" the player from far away (flow)
		 */
		if ((m_ptr->cdis <= (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)) ||
				(m_ptr->hp < m_ptr->maxhp) ||
				player_has_los_bold(m_ptr->fy, m_ptr->fx) ||
				monster_can_flow(c, m_ptr)) {
			/* Process timed effects - skip turn if necessary */
			if (process_monster_timed(c, m_ptr))
				continue;

			/* Set this monster to be the current actor */
			c->mon_current = i;

			/* Process the monster */
			process_monster(c, m_ptr);

			/* Monster is no longer current */
			c->mon_current = -1;
		}
	}

	/* Update monster visibility after this */
	/* XXX This may not be necessary */
	player->upkeep->update |= PU_MONSTERS;
}
