/**
 * \file project.c
 * \brief The project() function and helpers
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
#include "game-event.h"
#include "game-input.h"
#include "generate.h"
#include "init.h"
#include "mon-predicate.h"
#include "mon-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "project.h"
#include "source.h"
#include "trap.h"

struct projection *projections;

/*
 * Specify attr/char pairs for visual special effects for project()
 * Ideally move these and PROJ-type colors to the UI - NRM
 */
byte proj_to_attr[PROJ_MAX][BOLT_MAX];
wchar_t proj_to_char[PROJ_MAX][BOLT_MAX];

/**
 * ------------------------------------------------------------------------
 * PROJ type info needed for projections
 *
 * Note that elements come first, so PROJ_ACID == ELEM_ACID, etc
 * ------------------------------------------------------------------------ */
static const char *proj_name_list[] =
{
	#define ELEM(a) #a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ(a) #a,
	#include "list-projections.h"
	#undef PROJ
	"MAX",
    NULL
};

int proj_name_to_idx(const char *name)
{
    int i;
    for (i = 0; proj_name_list[i]; i++) {
        if (!my_stricmp(name, proj_name_list[i]))
            return i;
    }

    return -1;
}

const char *proj_idx_to_name(int type)
{
    assert(type >= 0);
    assert(type < PROJ_MAX);

    return proj_name_list[type];
}

/**
 * ------------------------------------------------------------------------
 * Projection paths
 * ------------------------------------------------------------------------ */
/**
 * Determine the path taken by a projection.
 *
 * The projection will always start from the grid1, and will travel
 * towards grid2, touching one grid per unit of distance along
 * the major axis, and stopping when it enters the finish grid or a
 * wall grid, or has travelled the maximum legal distance of "range".
 *
 * Note that "distance" in this function (as in the "update_view()" code)
 * is defined as "MAX(dy,dx) + MIN(dy,dx)/2", which means that the player
 * actually has an "octagon of projection" not a "circle of projection".
 *
 * The path grids are saved into the grid array pointed to by "gp", and
 * there should be room for at least "range" grids in "gp".  Note that
 * due to the way in which distance is calculated, this function normally
 * uses fewer than "range" grids for the projection path, so the result
 * of this function should never be compared directly to "range".  Note
 * that the initial grid grid1 is never saved into the grid array, not
 * even if the initial grid is also the final grid.  XXX XXX XXX
 *
 * The "flg" flags can be used to modify the behavior of this function.
 *
 * In particular, the "PROJECT_STOP" and "PROJECT_THRU" flags have the same
 * semantics as they do for the "project" function, namely, that the path
 * will stop as soon as it hits a monster, or that the path will continue
 * through the finish grid, respectively.
 *
 * The "PROJECT_JUMP" flag, which for the "project()" function means to
 * start at a special grid (which makes no sense in this function), means
 * that the path should be "angled" slightly if needed to avoid any wall
 * grids, allowing the player to "target" any grid which is in "view".
 * This flag is non-trivial and has not yet been implemented, but could
 * perhaps make use of the "vinfo" array (above).  XXX XXX XXX
 *
 * This function returns the number of grids (if any) in the path.  This
 * function will return zero if and only if grid1 and grid2 are equal.
 *
 * This algorithm is similar to, but slightly different from, the one used
 * by "update_view_los()", and very different from the one used by "los()".
 */
int project_path(struct loc *gp, int range, struct loc grid1, struct loc grid2,
				 int flg)
{
	int y, x;

	int n = 0;
	int k = 0;

	/* Absolute */
	int ay, ax;

	/* Offsets */
	int sy, sx;

	/* Fractions */
	int frac;

	/* Scale factors */
	int full, half;

	/* Slope */
	int m;

	/* Possible decoy */
	struct loc decoy = cave_find_decoy(cave);

	/* No path necessary (or allowed) */
	if (loc_eq(grid1, grid2)) return (0);


	/* Analyze "dy" */
	if (grid2.y < grid1.y) {
		ay = (grid1.y - grid2.y);
		sy = -1;
	} else {
		ay = (grid2.y - grid1.y);
		sy = 1;
	}

	/* Analyze "dx" */
	if (grid2.x < grid1.x) {
		ax = (grid1.x - grid2.x);
		sx = -1;
	} else {
		ax = (grid2.x - grid1.x);
		sx = 1;
	}


	/* Number of "units" in one "half" grid */
	half = (ay * ax);

	/* Number of "units" in one "full" grid */
	full = half << 1;


	/* Vertical */
	if (ay > ax) {
		/* Start at tile edge */
		frac = ax * ax;

		/* Let m = ((dx/dy) * full) = (dx * dx * 2) = (frac * 2) */
		m = frac << 1;

		/* Start */
		y = grid1.y + sy;
		x = grid1.x;

		/* Create the projection path */
		while (1) {
			/* Save grid */
			gp[n++] = loc(x, y);

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at finish grid */
			if (!(flg & (PROJECT_THRU)))
				if (loc_eq(loc(x, y), grid2)) break;

			/* Don't stop if making paths through rock for generation */
			if (!(flg & (PROJECT_ROCK))) {
				/* Stop at non-initial wall grids, except where that would
				 * leak info during targetting */
				if (!(flg & (PROJECT_INFO))) {
					if ((n > 0) && !square_isprojectable(cave, loc(x, y)))
						break;
				} else if ((n > 0) && square_isbelievedwall(cave, loc(x, y))) {
					break;
				}
			}

			/* Sometimes stop at non-initial monsters/players, decoys */
			if (flg & (PROJECT_STOP)) {
				if ((n > 0) && (square(cave, loc(x, y))->mon != 0)) break;
				if (loc_eq(loc(x, y), decoy)) break;
			}

			/* Slant */
			if (m) {
				/* Advance (X) part 1 */
				frac += m;

				/* Horizontal change */
				if (frac >= half) {
					/* Advance (X) part 2 */
					x += sx;

					/* Advance (X) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (Y) */
			y += sy;
		}
	}

	/* Horizontal */
	else if (ax > ay) {
		/* Start at tile edge */
		frac = ay * ay;

		/* Let m = ((dy/dx) * full) = (dy * dy * 2) = (frac * 2) */
		m = frac << 1;

		/* Start */
		y = grid1.y;
		x = grid1.x + sx;

		/* Create the projection path */
		while (1) {
			/* Save grid */
			gp[n++] = loc(x, y);

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at finish grid */
			if (!(flg & (PROJECT_THRU)))
				if (loc_eq(loc(x, y), grid2)) break;

			/* Don't stop if making paths through rock for generation */
			if (!(flg & (PROJECT_ROCK))) {
				/* Stop at non-initial wall grids, except where that would
				 * leak info during targetting */
				if (!(flg & (PROJECT_INFO))) {
					if ((n > 0) && !square_isprojectable(cave, loc(x, y)))
						break;
				} else if ((n > 0) && square_isbelievedwall(cave, loc(x, y))) {
					break;
				}
			}

			/* Sometimes stop at non-initial monsters/players, decoys */
			if (flg & (PROJECT_STOP)) {
				if ((n > 0) && (square(cave, loc(x, y))->mon != 0)) break;
				if (loc_eq(loc(x, y), decoy)) break;
			}

			/* Slant */
			if (m) {
				/* Advance (Y) part 1 */
				frac += m;

				/* Vertical change */
				if (frac >= half) {
					/* Advance (Y) part 2 */
					y += sy;

					/* Advance (Y) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (X) */
			x += sx;
		}
	}

	/* Diagonal */
	else {
		/* Start */
		y = grid1.y + sy;
		x = grid1.x + sx;

		/* Create the projection path */
		while (1) {
			/* Save grid */
			gp[n++] = loc(x, y);

			/* Hack -- Check maximum range */
			if ((n + (n >> 1)) >= range) break;

			/* Sometimes stop at finish grid */
			if (!(flg & (PROJECT_THRU)))
				if (loc_eq(loc(x, y), grid2)) break;

			/* Don't stop if making paths through rock for generation */
			if (!(flg & (PROJECT_ROCK))) {
				/* Stop at non-initial wall grids, except where that would
				 * leak info during targetting */
				if (!(flg & (PROJECT_INFO))) {
					if ((n > 0) && !square_isprojectable(cave, loc(x, y)))
						break;
				} else if ((n > 0) && square_isbelievedwall(cave, loc(x, y))) {
					break;
				}
			}

			/* Sometimes stop at non-initial monsters/players, decoys */
			if (flg & (PROJECT_STOP)) {
				if ((n > 0) && (square(cave, loc(x, y))->mon != 0)) break;
				if (loc_eq(loc(x, y), decoy)) break;
			}

			/* Advance */
			y += sy;
			x += sx;
		}
	}

	/* Length */
	return (n);
}


/**
 * Determine if a bolt spell cast from grid1 to grid2 will arrive
 * at the final destination, assuming that no monster gets in the way,
 * using the project_path() function to check the projection path.
 *
 * Note that no grid is ever projectable() from itself.
 *
 * This function is used to determine if the player can (easily) target
 * a given grid, and if a monster can target the player.
 */
bool projectable(struct chunk *c, struct loc grid1, struct loc grid2, int flg)
{
	struct loc grid_g[512];
	int grid_n = 0;
	int max_range = z_info->max_range;

	/* Check for shortened projection range */
	if ((flg & PROJECT_SHORT) && player->timed[TMD_COVERTRACKS]) {
		max_range /= 4;
	}

	/* Check the projection path */
	grid_n = project_path(grid_g, max_range, grid1, grid2, flg);

	/* No grid is ever projectable from itself */
	if (!grid_n) return false;

	/* May not end in a wall grid */
	if (!square_ispassable(c, grid_g[grid_n - 1])) return false;

	/* May not end in an unrequested grid */
	if (!loc_eq(grid_g[grid_n - 1], grid2)) return false;

	/* Assume okay */
	return (true);
}




/**
 * ------------------------------------------------------------------------
 * The main project() function and its helpers
 * ------------------------------------------------------------------------ */

/**
 * Given an origin, find its coordinates and return them
 *
 * If there is no origin, return (-1, -1)
 */
struct loc origin_get_loc(struct source origin)
{
	switch (origin.what) {
		case SRC_MONSTER: {
			struct monster *who = cave_monster(cave, origin.which.monster);
			return who ? who->grid : loc(-1, -1);
		}

		case SRC_TRAP: {
			struct trap *trap = origin.which.trap;
			return trap->grid;
		}

		case SRC_PLAYER:
		case SRC_OBJECT:	/* Currently only worn cursed objects use this */
		case SRC_CHEST_TRAP:
			return player->grid;

		case SRC_NONE:
			return loc(-1, -1);
	}

	return loc(-1, -1);
}

/**
 * Generic "beam"/"bolt"/"ball" projection routine.
 *   -BEN-, some changes by -LM-
 *
 *   \param origin Origin of the projection
 *   \param rad Radius of explosion (0 = beam/bolt, 1 to 20 = ball), or maximum
 *	  length of arc from the source.
 *   \param y Target location (or location to travel towards)
 *   \param x Target location (or location to travel towards)
 *   \param dam Base damage to apply to monsters, terrain, objects, or player
 *   \param typ Type of projection (fire, frost, dispel demons etc.)
 *   \param flg Extra bit flags that control projection behavior
 *   \param degrees_of_arc How wide an arc spell is (in degrees).
 *   \param diameter_of_source how wide the source diameter is.
 *   \param obj An object that the projection ignores
 *
 *   \return true if any effects of the projection were observed, else false
 *
 *
 * At present, there are five major types of projections:
 *
 * Point-effect projection:  (no PROJECT_BEAM flag, radius of zero, and either 
 *   jumps directly to target or has a single source and target grid)
 * A point-effect projection has no line of projection, and only affects one 
 *   grid.  It is used for most area-effect spells (like dispel evil) and 
 *   pinpoint strikes like the monster Holding prayer.
 * 
 * Bolt:  (no PROJECT_BEAM flag, radius of zero, has to travel from source to 
 *   target)
 * A bolt travels from source to target and affects only the final grid in its 
 *   projection path.  If given the PROJECT_STOP flag, it is stopped by any 
 *   monster or character in its path (at present, all bolts use this flag).
 *
 * Beam:  (PROJECT_BEAM)
 * A beam travels from source to target, affecting all grids passed through 
 *   with full damage.  It is never stopped by monsters in its path.  Beams 
 *   may never be combined with any other projection type.
 *
 * Ball:  (positive radius, unless the PROJECT_ARC flag is set)
 * A ball travels from source towards the target, and always explodes.  Unless 
 *   specified, it does not affect wall grids, but otherwise affects any grids 
 *   in LOS from the center of the explosion.
 * If used with a direction, a ball will explode on the first occupied grid in 
 *   its path.  If given a target, it will explode on that target.  If a 
 *   wall is in the way, it will explode against the wall.  If a ball reaches 
 *   z_info->max_range without hitting anything or reaching its target, it will 
 *   explode at that point.
 *
 * Arc:  (positive radius, with the PROJECT_ARC flag set)
 * An arc is a portion of a source-centered ball that explodes outwards 
 *   towards the target grid.  Like a ball, it affects all non-wall grids in 
 *   LOS of the source in the explosion area.  The width of arc spells is con-
 *   trolled by degrees_of_arc.
 * An arc is created by rejecting all grids that form the endpoints of lines 
 *   whose angular difference (in degrees) from the centerline of the arc is 
 *   greater than one-half the input "degrees_of_arc".  See the table "get_
 *   angle_to_grid" in "util.c" for more information.
 * Note:  An arc with a value for degrees_of_arc of zero is actually a beam of
 *   defined length.
 *
 * Projections that effect all monsters in LOS are handled through the use 
 *   of the PROJECT_LOS effect, which applies a single-grid projection to
 *   individual monsters.  Projections that light up rooms or affect all
 *   monsters on the level are more efficiently handled through special
 *   functions.
 *
 *
 * Variations:
 *
 * PROJECT_STOP forces a path of projection to stop at the first occupied grid 
 *   it hits.  This is used with bolts, and also by ball spells travelling in 
 *   a specific direction rather than towards a target.
 *
 * PROJECT_THRU allows a path of projection towards a target to continue 
 *   past that target.  It also allows a spell to affect wall grids adjacent 
 *   to a grid in LOS of the center of the explosion.
 * 
 * PROJECT_JUMP allows a projection to immediately set the source of the pro-
 *   jection to the target.  This is used for all area effect spells (like 
 *   dispel evil), and can also be used for bombardments.
 * 
 * PROJECT_HIDE erases all graphical effects, making the projection invisible.
 *
 * PROJECT_GRID allows projections to affect terrain features.
 *
 * PROJECT_ITEM allows projections to affect objects on the ground.
 *
 * PROJECT_KILL allows projections to affect monsters.
 *
 * PROJECT_PLAY allows projections to affect the player.
 *
 * degrees_of_arc controls the width of arc spells.  With a value for 
 *   degrees_of_arc of zero, arcs act like beams of defined length.
 *
 * diameter_of_source controls how quickly explosions lose strength with dis-
 *   tance from the target.  Most ball spells have a source diameter of 10,
 *   which means that they do 1/2 damage at range 1, 1/3 damage at range 2,
 *   and so on.   Caster-centered balls usually have a source diameter of 20,
 *   which allows them to do full damage to all adjacent grids.   Arcs have
 *   source diameters ranging up from 20, which allows the spell designer to
 *   fine-tune how quickly a breath loses strength outwards from the breather.
 *
 *
 * Implementation notes:
 *
 * If the source grid is not the same as the target, we project along the path 
 *   between them.  Bolts stop if they hit anything, beams stop if they hit a 
 *   wall, and balls and arcs may exhibit either bahavior.  When they reach 
 *   the final grid in the path, balls and arcs explode.  We do not allow 
 *   beams to be combined with explosions.
 * Balls affect all floor grids in LOS (optionally, also wall grids adjacent 
 *   to a grid in LOS) within their radius.  Arcs do the same, but only within 
 *   their cone of projection.
 * Because affected grids are only scanned once, and it is really helpful to 
 *   have explosions that travel outwards from the source, they are sorted by 
 *   distance.  For each distance, an adjusted damage is calculated.
 * In successive passes, the code then displays explosion graphics, erases 
 *   these graphics, marks terrain for possible later changes, affects 
 *   objects, monsters, the character, and finally changes features and 
 *   teleports monsters and characters in marked grids.
 * 
 *
 * Usage and graphics notes:
 *
 * Only 256 grids can be affected per projection, limiting the effective 
 * radius of standard ball attacks to nine units (diameter nineteen).  Arcs 
 * can have larger radii; an arc capable of going out to range 20 should not 
 * be wider than 70 degrees.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters on 
 * both sides of a wall. 
 *
 * Note that for consistency, we pretend that the bolt actually takes time
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the illumination of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool project(struct source origin, int rad, struct loc finish,
			 int dam, int typ, int flg,
			 int degrees_of_arc, byte diameter_of_source,
			 const struct object *obj)
{
	int i, j, k, dist_from_centre;

	u32b dam_temp;

	struct loc centre;
	struct loc start;

	int n1y = 0;
	int n1x = 0;

	/* Assume the player sees nothing */
	bool notice = false;

	/* Notify the UI if it can draw this projection */
	bool drawing = false;

	/* Is the player blind? */
	bool blind = (player->timed[TMD_BLIND] ? true : false);

	/* Number of grids in the "path" */
	int num_path_grids = 0;

	/* Actual grids in the "path" */
	struct loc path_grid[512];

	/* Number of grids in the "blast area" (including the "beam" path) */
	int num_grids = 0;

	/* Coordinates of the affected grids */
	struct loc blast_grid[256];

	/* Distance to each of the affected grids. */
	int distance_to_grid[256];

	/* Player visibility of each of the affected grids. */
	bool player_sees_grid[256];

	/* Precalculated damage values for each distance. */
	int *dam_at_dist = malloc((z_info->max_range + 1) * sizeof(*dam_at_dist));

	/* Flush any pending output */
	handle_stuff(player);

	/* No projection path - jump to target */
	if (flg & PROJECT_JUMP) {
		start = finish;

		/* Clear the flag */
		flg &= ~(PROJECT_JUMP);
	} else {
		start = origin_get_loc(origin);

		/* Default to finish grid */
		if (start.y == -1 && start.x == -1) {
			start = finish;
		}
	}

	/* Default center of explosion (if any) */
	centre = start;

	/*
	 * An arc spell with no width and a non-zero radius is actually a
	 * beam of defined length.  Mark it as such.
	 */
	if ((flg & (PROJECT_ARC)) && (degrees_of_arc == 0) && (rad != 0)) {
		/* No longer an arc */
		flg &= ~(PROJECT_ARC);

		/* Now considered a beam */
		flg |= (PROJECT_BEAM);
		flg |= (PROJECT_THRU);
	}

	/*
	 * If a single grid is both start and finish (for example
	 * if PROJECT_JUMP is set), store it; otherwise, travel along the
	 * projection path.
	 */
	if (loc_eq(start, finish)) {
		blast_grid[num_grids] =  finish;
		centre = finish;
		distance_to_grid[num_grids] = 0;
		sqinfo_on(square(cave, finish)->info, SQUARE_PROJECT);
		num_grids++;
	} else {
		/* Start from caster */
		int y = start.y;
		int x = start.x;

		/* Calculate the projection path */
		num_path_grids = project_path(path_grid, z_info->max_range, start,
									  finish, flg);

		/* Some beams have limited length. */
		if (flg & (PROJECT_BEAM)) {
			/* Use length limit, if any is given. */
			if ((rad > 0) && (rad < num_path_grids)) {
				num_path_grids = rad;
			}
		}


		/* Project along the path (except for arcs) */
		if (!(flg & (PROJECT_ARC))) {
			for (i = 0; i < num_path_grids; ++i) {
				int oy = y;
				int ox = x;

				int ny = path_grid[i].y;
				int nx = path_grid[i].x;

				/* Hack -- Balls explode before reaching walls. */
				if (!square_ispassable(cave, path_grid[i]) && (rad > 0) &&
					!(flg & (PROJECT_BEAM)))
					break;

				/* Advance */
				y = ny;
				x = nx;

				/* Beams collect all grids in the path, all other methods
				 * collect only the final grid in the path. */
				if (flg & (PROJECT_BEAM)) {
					blast_grid[num_grids].y = y;
					blast_grid[num_grids].x = x;
					distance_to_grid[num_grids] = 0;
					sqinfo_on(square(cave, loc(x, y))->info, SQUARE_PROJECT);
					num_grids++;
				} else if (i == num_path_grids - 1) {
					blast_grid[num_grids].y = y;
					blast_grid[num_grids].x = x;
					distance_to_grid[num_grids] = 0;
					sqinfo_on(square(cave, loc(x, y))->info, SQUARE_PROJECT);
					num_grids++;
				}

				/* Only do visuals if requested and within range limit. */
				if (!blind && !(flg & (PROJECT_HIDE))) {
					bool seen = square_isview(cave, loc(x, y));
					bool beam = flg & (PROJECT_BEAM);

					/* Tell the UI to display the bolt */
					event_signal_bolt(EVENT_BOLT, typ, drawing, seen, beam, oy,
									  ox, y, x);
				}
			}
		}

		/* Save the "blast epicenter" */
		centre.y = y;
		centre.x = x;
	}

	/* Now check for explosions.  Beams have already stored all the grids they
	 * will affect; all non-beam projections with positive radius explode in
	 * some way */
	if ((rad > 0) && (!(flg & (PROJECT_BEAM)))) {
		int y, x;

		/* Pre-calculate some things for arcs. */
		if ((flg & (PROJECT_ARC)) && (num_path_grids != 0)) {
			/* Explosion centers on the caster. */
			centre = start;

			/* The radius of arcs cannot be more than 20 */
			if (rad > 20)
				rad = 20;

			/* Ensure legal access into get_angle_to_grid table */
			if (num_path_grids < 21)
				i = num_path_grids - 1;
			else
				i = 20;

			/* Reorient the grid forming the end of the arc's centerline. */
			n1y = path_grid[i].y - centre.y + 20;
			n1x = path_grid[i].x - centre.x + 20;
		}

		/* If the explosion centre hasn't been saved already, save it now. */
		if (num_grids == 0) {
			blast_grid[num_grids] = centre;
			distance_to_grid[num_grids] = 0;
			sqinfo_on(square(cave, centre)->info, SQUARE_PROJECT);
			num_grids++;
		}

		/* Scan every grid that might possibly be in the blast radius. */
		for (y = centre.y - rad; y <= centre.y + rad; y++) {
			for (x = centre.x - rad; x <= centre.x + rad; x++) {
				struct loc grid = loc(x, y);
				bool on_path = false;

				/* Center grid has already been stored. */
				if (loc_eq(grid, centre))
					continue;

				/* Precaution: Stay within area limit. */
				if (num_grids >= 255)
					break;

				/* Ignore "illegal" locations */
				if (!square_in_bounds(cave, grid))
					continue;

				/* Most explosions are immediately stopped by walls. If
				 * PROJECT_THRU is set, walls can be affected if adjacent to
				 * a grid visible from the explosion centre - note that as of
				 * Angband 3.5.0 there are no such explosions - NRM.
				 * All explosions can affect one layer of terrain which is
				 * passable but not projectable */
				if ((flg & (PROJECT_THRU)) || square_ispassable(cave, grid)) {
					/* If this is a wall grid, ... */
					if (!square_isprojectable(cave, grid)) {
						bool can_see_one = false;
						/* Check neighbors */
						for (i = 0; i < 8; i++) {
							struct loc adj_grid = loc_sum(grid, ddgrid_ddd[i]);
							if (los(cave, centre, adj_grid)) {
								can_see_one = true;
								break;
							}
						}

						/* Require at least one adjacent grid in LOS. */
						if (!can_see_one)
							continue;
					}
				} else if (!square_isprojectable(cave, grid))
					continue;

				/* Must be within maximum distance. */
				dist_from_centre  = (distance(centre, grid));
				if (dist_from_centre > rad)
					continue;

				/* Mark grids which are on the projection path */
				for (i = 0; i < num_path_grids; i++) {
					if (loc_eq(grid, path_grid[i])) {
						on_path = true;
					}
				}

				/* Do we need to consider a restricted angle? */
				if (flg & (PROJECT_ARC)) {
					/* Use angle comparison to delineate an arc. */
					int n2y, n2x, tmp, rotate, diff;

					/* Reorient current grid for table access. */
					n2y = y - start.y + 20;
					n2x = x - start.x + 20;

					/* Find the angular difference (/2) between the lines to
					 * the end of the arc's center-line and to the current grid.
					 */
					rotate = 90 - get_angle_to_grid[n1y][n1x];
					tmp = ABS(get_angle_to_grid[n2y][n2x] + rotate) % 180;
					diff = ABS(90 - tmp);

					/* If difference is greater then that allowed, skip it,
					 * unless it's on the target path */
					if ((diff >= (degrees_of_arc + 6) / 4) && !on_path)
						continue;
				}

				/* Accept remaining grids if in LOS or on the projection path */
				if (los(cave, centre, grid) || on_path) {
					blast_grid[num_grids].y = y;
					blast_grid[num_grids].x = x;
					distance_to_grid[num_grids] = dist_from_centre;
					sqinfo_on(square(cave, grid)->info, SQUARE_PROJECT);
					num_grids++;
				}
			}
		}
	}

	/* Calculate and store the actual damage at each distance. */
	for (i = 0; i <= z_info->max_range; i++) {
		if (i > rad) {
			/* No damage outside the radius. */
			dam_temp = 0;
		} else if ((!diameter_of_source) || (i == 0)) {
			/* Standard damage calc. for 10' source diameters, or at origin. */
			dam_temp = (dam + i) / (i + 1);
		} else {
			/* If a particular diameter for the source of the explosion's
			 * energy is given, it is full strength to that diameter and
			 * then reduces */
			dam_temp = (diameter_of_source * dam) / (i + 1);
			if (dam_temp > (u32b) dam) {
				dam_temp = dam;
			}
		}

		/* Store it. */
		dam_at_dist[i] = dam_temp;
	}


	/* Sort the blast grids by distance from the centre. */
	for (i = 0, k = 0; i <= rad; i++) {
		/* Collect all the grids of a given distance together. */
		for (j = k; j < num_grids; j++) {
			if (distance_to_grid[j] == i) {
				struct loc tmp;
				int tmp_d = distance_to_grid[k];
				tmp = blast_grid[k];

				blast_grid[k] = blast_grid[j];
				distance_to_grid[k] = distance_to_grid[j];

				blast_grid[j] = tmp;
				distance_to_grid[j] = tmp_d;

				/* Write to next slot */
				k++;
			}
		}
	}

	/* Establish which grids are visible - no blast visuals with PROJECT_HIDE */
	for (i = 0; i < num_grids; i++) {
		if (panel_contains(blast_grid[i].y, blast_grid[i].x) &&
			square_isview(cave, blast_grid[i]) &&
			!blind && !(flg & (PROJECT_HIDE))) {
			player_sees_grid[i] = true;
		} else {
			player_sees_grid[i] = false;
		}
	}

	/* Tell the UI to display the blast */
	event_signal_blast(EVENT_EXPLOSION, typ, num_grids, distance_to_grid,
					   drawing, player_sees_grid, blast_grid, centre);

	/* Affect objects on every relevant grid */
	if (flg & (PROJECT_ITEM)) {
		for (i = 0; i < num_grids; i++) {
			if (project_o(origin, distance_to_grid[i], blast_grid[i],
						  dam_at_dist[distance_to_grid[i]], typ, obj)) {
				notice = true;
			}
		}
	}

	/* Check monsters */
	if (flg & (PROJECT_KILL)) {
		bool was_obvious = false;
		bool did_hit = false;
		int num_hit = 0;
		struct loc last_hit_grid = loc(0, 0);

		/* Scan for monsters */
		for (i = 0; i < num_grids; i++) {
			struct monster *mon = NULL;

			/* Check this monster hasn't been processed already */
			if (!square_isproject(cave, blast_grid[i]))
				continue;

			/* Check there is actually a monster here */
			mon = square_monster(cave, blast_grid[i]);
			if (mon == NULL)
				continue;

			/* Affect the monster in the grid */
			project_m(origin, distance_to_grid[i], blast_grid[i],
			          dam_at_dist[distance_to_grid[i]], typ, flg,
			          &did_hit, &was_obvious);
			if (was_obvious) {
				notice = true;
			}
			if (did_hit) {
				num_hit++;

				/* Monster location may have been updated by project_m() */
				last_hit_grid = mon->grid;
			}
		}

		/* Player affected one monster (without "jumping") */
		if (origin.what == SRC_PLAYER &&
				num_hit == 1 &&
				!(flg & PROJECT_JUMP)) {
			/* Location */
			int x = last_hit_grid.x;
			int y = last_hit_grid.y;

			/* Track if possible */
			if (square(cave, loc(x, y))->mon > 0) {
				struct monster *mon = square_monster(cave, loc(x, y));

				/* Recall and track */
				if (monster_is_visible(mon)) {
					monster_race_track(player->upkeep, mon->race);
					health_track(player->upkeep, mon);
				}
			}
		}
	}

	/* Look for the player, affect them when found */
	if (flg & (PROJECT_PLAY)) {
		/* Set power */
		int power = 0;
		if (origin.what == SRC_MONSTER) {
			struct monster *mon = cave_monster(cave, origin.which.monster);
			power = mon->race->spell_power;

			/* Breaths from powerful monsters get power effects as well */
			if (monster_is_powerful(mon))
				power = MAX(power, 80);
		}
		for (i = 0; i < num_grids; i++) {
			if (project_p(origin, distance_to_grid[i], blast_grid[i],
						  dam_at_dist[distance_to_grid[i]], typ, power,
						  flg & PROJECT_SELF)) {
				notice = true;
				if (player->is_dead) {
					free(dam_at_dist);
					return notice;
				}
				break;
			}
		}
	}

	/* Affect features in every relevant grid */
	if (flg & (PROJECT_GRID)) {
		for (i = 0; i < num_grids; i++) {
			if (project_f(origin, distance_to_grid[i], blast_grid[i],
						  dam_at_dist[distance_to_grid[i]], typ)) {
				notice = true;
			}
		}
	}

	/* Clear all the processing marks. */
	for (i = 0; i < num_grids; i++) {
		/* Clear the mark */
		sqinfo_off(square(cave, blast_grid[i])->info, SQUARE_PROJECT);
	}

	/* Update stuff if needed */
	if (player->upkeep->update) update_stuff(player);

	free(dam_at_dist);

	/* Return "something was noticed" */
	return (notice);
}
