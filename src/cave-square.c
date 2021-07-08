 /**
 * \file cave-square.c
 * \brief functions for dealing with individual squares
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
#include "game-world.h"
#include "init.h"
#include "monster.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "trap.h"


/**
 * FEATURE PREDICATES
 *
 * These functions test a terrain feature index for the obviously described
 * type.  They are used in the square feature predicates below, and
 * occasionally on their own
 */

/**
 * True if the square is a magma wall.
 */
bool feat_is_magma(int feat)
{
	return tf_has(f_info[feat].flags, TF_MAGMA);
}

/**
 * True if the square is a quartz wall.
 */
bool feat_is_quartz(int feat)
{
	return tf_has(f_info[feat].flags, TF_QUARTZ);
}

/**
 * True if the square is a granite wall.
 */
bool feat_is_granite(int feat)
{
	return tf_has(f_info[feat].flags, TF_GRANITE);
}

/**
 * True if the square is a mineral wall with treasure (magma/quartz).
 */
bool feat_is_treasure(int feat)
{
	return (tf_has(f_info[feat].flags, TF_GOLD));
}

/**
 * True if the feature is a solid wall (not rubble).
 */
bool feat_is_wall(int feat)
{
	return tf_has(f_info[feat].flags, TF_WALL);
}

/**
 * True if the feature is a floor.
 */
bool feat_is_floor(int feat)
{
	return tf_has(f_info[feat].flags, TF_FLOOR);
}

/**
 * True if the feature can hold a trap.
 */
bool feat_is_trap_holding(int feat)
{
	return tf_has(f_info[feat].flags, TF_TRAP);
}

/**
 * True if the feature can hold an object.
 */
bool feat_is_object_holding(int feat)
{
	return tf_has(f_info[feat].flags, TF_OBJECT);
}

/**
 * True if a monster can walk through the feature.
 */
bool feat_is_monster_walkable(int feat)
{
	return tf_has(f_info[feat].flags, TF_PASSABLE);
}

/**
 * True if the feature is a shop entrance.
 */
bool feat_is_shop(int feat)
{
	return tf_has(f_info[feat].flags, TF_SHOP);
}

/**
 * True if the feature is passable by the player.
 */
bool feat_is_passable(int feat)
{
	return tf_has(f_info[feat].flags, TF_PASSABLE);
}

/**
 * True if any projectable can pass through the feature.
 */
bool feat_is_projectable(int feat)
{
	return tf_has(f_info[feat].flags, TF_PROJECT);
}

/**
 * True if the feature can be lit by light sources.
 */
bool feat_is_torch(int feat)
{
	return tf_has(f_info[feat].flags, TF_TORCH);
}

/**
 * True if the feature is internally lit.
 */
bool feat_is_bright(int feat)
{
	return tf_has(f_info[feat].flags, TF_BRIGHT);
}

/**
 * True if the feature is internally lit.
 */
bool feat_is_fiery(int feat)
{
	return tf_has(f_info[feat].flags, TF_FIERY);
}

/**
 * True if the feature doesn't carry monster flow information.
 */
bool feat_is_no_flow(int feat)
{
	return tf_has(f_info[feat].flags, TF_NO_FLOW);
}

/**
 * True if the feature doesn't carry player scent.
 */
bool feat_is_no_scent(int feat)
{
	return tf_has(f_info[feat].flags, TF_NO_SCENT);
}

/**
 * True if the feature should have smooth boundaries (for dungeon generation).
 */
bool feat_is_smooth(int feat)
{
	return tf_has(f_info[feat].flags, TF_SMOOTH);
}

/**
 * SQUARE FEATURE PREDICATES
 *
 * These functions are used to figure out what kind of square something is,
 * via c->squares[y][x].feat (preferably accessed via square(c, grid)).
 * All direct testing of square(c, grid).feat should be rewritten
 * in terms of these functions.
 *
 * It's often better to use square behavior predicates (written in terms of
 * these functions) instead of these functions directly. For instance,
 * square_isrock() will return false for a secret door, even though it will
 * behave like a rock wall until the player determines it's a door.
 *
 * Use functions like square_isdiggable, square_iswall, etc. in these cases.
 */

/**
 * True if the square is normal open floor.
 */
bool square_isfloor(struct chunk *c, struct loc grid)
{
	return feat_is_floor(square(c, grid).feat);
}

/**
 * True if the square can hold a trap.
 */
bool square_istrappable(struct chunk *c, struct loc grid)
{
	return feat_is_trap_holding(square(c, grid).feat);
}

/**
 * True if the square can hold an object.
 */
bool square_isobjectholding(struct chunk *c, struct loc grid)
{
	return feat_is_object_holding(square(c, grid).feat);
}

/**
 * True if the square is a normal granite rock wall.
 */
bool square_isrock(struct chunk *c, struct loc grid)
{
	return (tf_has(f_info[square(c, grid).feat].flags, TF_GRANITE) &&
			!tf_has(f_info[square(c, grid).feat].flags, TF_DOOR_ANY));
}

/**
 * True if the square is granite.
 */
bool square_isgranite(struct chunk *c, struct loc grid)
{
	return feat_is_granite(square(c, grid).feat);
}

/**
 * True if the square is a permanent wall.
 */
bool square_isperm(struct chunk *c, struct loc grid)
{
	return (tf_has(f_info[square(c, grid).feat].flags, TF_PERMANENT) &&
			tf_has(f_info[square(c, grid).feat].flags, TF_ROCK));
}

/**
 * True if the square is a magma wall.
 */
bool square_ismagma(struct chunk *c, struct loc grid)
{
	return feat_is_magma(square(c, grid).feat);
}

/**
 * True if the square is a quartz wall.
 */
bool square_isquartz(struct chunk *c, struct loc grid)
{
	return feat_is_quartz(square(c, grid).feat);
}

/**
 * True if the square is a mineral wall (magma/quartz/granite).
 */
bool square_ismineral(struct chunk *c, struct loc grid)
{
	return square_isrock(c, grid) || square_ismagma(c, grid) ||
		square_isquartz(c, grid);
}

bool square_hasgoldvein(struct chunk *c, struct loc grid)
{
	return tf_has(f_info[square(c, grid).feat].flags, TF_GOLD);
}

/**
 * True if the square is rubble.
 */
bool square_isrubble(struct chunk *c, struct loc grid)
{
    return (!tf_has(f_info[square(c, grid).feat].flags, TF_WALL) &&
			tf_has(f_info[square(c, grid).feat].flags, TF_ROCK));
}

/**
 * True if the square is a hidden secret door.
 *
 * These squares appear as if they were granite--when detected a secret door
 * is replaced by a closed door.
 */
bool square_issecretdoor(struct chunk *c, struct loc grid)
{
    return (tf_has(f_info[square(c, grid).feat].flags, TF_DOOR_ANY) &&
			tf_has(f_info[square(c, grid).feat].flags, TF_ROCK));
}

/**
 * True if the square is an open door.
 */
bool square_isopendoor(struct chunk *c, struct loc grid)
{
    return (tf_has(f_info[square(c, grid).feat].flags, TF_CLOSABLE));
}

/**
 * True if the square is a closed door (possibly locked or jammed).
 */
bool square_iscloseddoor(struct chunk *c, struct loc grid)
{
	int feat = square(c, grid).feat;
	return tf_has(f_info[feat].flags, TF_DOOR_CLOSED);
}

bool square_isbrokendoor(struct chunk *c, struct loc grid)
{
	int feat = square(c, grid).feat;
    return (tf_has(f_info[feat].flags, TF_DOOR_ANY) &&
			tf_has(f_info[feat].flags, TF_PASSABLE) &&
			!tf_has(f_info[feat].flags, TF_CLOSABLE));
}

/**
 * True if the square is a door.
 *
 * This includes open, closed, and hidden doors.
 */
bool square_isdoor(struct chunk *c, struct loc grid)
{
	int feat = square(c, grid).feat;
	return tf_has(f_info[feat].flags, TF_DOOR_ANY);
}

/**
 * True if square is any stair
 */
bool square_isstairs(struct chunk *c, struct loc grid)
{
	int feat = square(c, grid).feat;
	return tf_has(f_info[feat].flags, TF_STAIR);
}

/**
 * True if square is an up stair.
 */
bool square_isupstairs(struct chunk*c, struct loc grid)
{
	int feat = square(c, grid).feat;
	return tf_has(f_info[feat].flags, TF_UPSTAIR);
}

/**
 * True if square is a down stair.
 */
bool square_isdownstairs(struct chunk *c, struct loc grid)
{
	int feat = square(c, grid).feat;
	return tf_has(f_info[feat].flags, TF_DOWNSTAIR);
}

/**
 * True if the square is a shop entrance.
 */
bool square_isshop(struct chunk *c, struct loc grid)
{
	return feat_is_shop(square(c, grid).feat);
}

/**
 * True if the square contains the player
 */
bool square_isplayer(struct chunk *c, struct loc grid) {
	return square(c, grid).mon < 0 ? true : false;
}

/**
 * True if the square contains the player or a monster
 */
bool square_isoccupied(struct chunk *c, struct loc grid) {
	return square(c, grid).mon != 0 ? true : false;
}

/**
 * True if the the player knows the terrain of the square
 */
bool square_isknown(struct chunk *c, struct loc grid) {
	if (c != cave) return false;
	if (player->cave == NULL) return false;
	return square(player->cave, grid).feat == FEAT_NONE ? false : true;
}

/**
 * True if the the player's knowledge of the terrain of the square is wrong
 * or missing
 */
bool square_isnotknown(struct chunk *c, struct loc grid) {
	if (c != cave) return false;
	if (player->cave == NULL) return true;
	return square(player->cave, grid).feat != square(c, grid).feat;
}

/**
 * SQUARE INFO PREDICATES
 *
 * These functions tell whether a square is marked with one of the SQUARE_*
 * flags.  These flags are mostly used to mark a square with some information
 * about its location or status.
 */

/**
 * True if the square is marked
 */
bool square_ismark(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_MARK);
}

/**
 * True if the square is lit
 */
bool square_isglow(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_GLOW);
}

/**
 * True if the square is part of a vault.
 *
 * This doesn't say what kind of square it is, just that it is part of a vault.
 */
bool square_isvault(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_VAULT);
}

/**
 * True if the square is part of a room.
 */
bool square_isroom(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_ROOM);
}

/**
 * True if the square has been seen by the player
 */
bool square_isseen(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_SEEN);
}

/**
 * True if the cave square is currently viewable by the player
 */
bool square_isview(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_VIEW);
}

/**
 * True if the cave square was seen before the current update
 */
bool square_wasseen(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_WASSEEN);
}

/**
 * True if cave square is a feeling trigger square 
 */
bool square_isfeel(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_FEEL);
}

/**
 * True if the square has a known trap
 */
bool square_istrap(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_TRAP);
}

/**
 * True if the square has an unknown trap
 */
bool square_isinvis(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_INVIS);
}

/**
 * True if cave square is an inner wall (generation)
 */
bool square_iswall_inner(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_WALL_INNER);
}

/**
 * True if cave square is an outer wall (generation)
 */
bool square_iswall_outer(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_WALL_OUTER);
}

/**
 * True if cave square is a solid wall (generation)
 */
bool square_iswall_solid(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_WALL_SOLID);
}

/**
 * True if cave square has monster restrictions (generation)
 */
bool square_ismon_restrict(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_MON_RESTRICT);
}

/**
 * True if cave square can't be teleported from by the player
 */
bool square_isno_teleport(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_NO_TELEPORT);
}

/**
 * True if cave square can't be magically mapped by the player
 */
bool square_isno_map(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_NO_MAP);
}

/**
 * True if cave square can't be detected by player ESP
 */
bool square_isno_esp(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_NO_ESP);
}

/**
 * True if cave square is marked for projection processing
 */
bool square_isproject(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_PROJECT);
}

/**
 * True if cave square has been detected for traps
 */
bool square_isdtrap(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_DTRAP);
}

/**
 * True if cave square is inappropriate to place stairs
 */
bool square_isno_stairs(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return sqinfo_has(square(c, grid).info, SQUARE_NO_STAIRS);
}


/**
 * SQUARE BEHAVIOR PREDICATES
 *
 * These functions define how a given square behaves, e.g. whether it is
 * passable by the player, whether it is diggable, contains items, etc.
 *
 * These functions use the SQUARE FEATURE PREDICATES (among other info) to
 * make the determination.
 */

/**
 * True if the square is open (a floor square not occupied by a monster).
 */
bool square_isopen(struct chunk *c, struct loc grid) {
	return square_isfloor(c, grid) && !square(c, grid).mon;
}

/**
 * True if the square is empty (an open square without any items).
 */
bool square_isempty(struct chunk *c, struct loc grid) {
	if (square_isplayertrap(c, grid)) return false;
	if (square_iswebbed(c, grid)) return false;
	return square_isopen(c, grid) && !square_object(c, grid);
}

/**
 * True if the square is empty (an open square without any items).
 */
bool square_isarrivable(struct chunk *c, struct loc grid) {
	if (square(c, grid).mon) return false;
	if (square_isplayertrap(c, grid)) return false;
	if (square_iswebbed(c, grid)) return false;
	if (square_isfloor(c, grid)) return true;
	if (square_isstairs(c, grid)) return true;
	// maybe allow open doors or suchlike?
	return false;
}

/**
 * True if the square is an untrapped floor square without items.
 */
bool square_canputitem(struct chunk *c, struct loc grid) {
	if (!square_isobjectholding(c, grid)) return false;
	if (square_istrap(c, grid)) return false;
	return !square_object(c, grid);
}

/**
 * True if the square can be dug: this includes rubble and non-permanent walls.
 */
bool square_isdiggable(struct chunk *c, struct loc grid) {
	return (square_ismineral(c, grid) ||
			square_issecretdoor(c, grid) || 
			square_isrubble(c, grid));
}

/**
 * True if the square is a floor with no traps.
 */
bool square_iswebbable(struct chunk *c, struct loc grid) {
	if (square_trap(c, grid)) return false;
	return square_isfloor(c, grid);
}

/**
 * True if a monster can walk through the tile.
 *
 * This is needed for polymorphing. A monster may be on a feature that isn't
 * an empty space, causing problems when it is replaced with a new monster.
 */
bool square_is_monster_walkable(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds(c, grid));
	return feat_is_monster_walkable(square(c, grid).feat);
}

/**
 * True if the square is passable by the player.
 */
bool square_ispassable(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return feat_is_passable(square(c, grid).feat);
}

/**
 * True if any projectable can pass through the square.
 *
 * This function is the logical negation of square_iswall().
 */
bool square_isprojectable(struct chunk *c, struct loc grid) {
	if (!square_in_bounds(c, grid)) return false;
	return feat_is_projectable(square(c, grid).feat);
}

/**
 * True if the square is a wall square (impedes the player).
 *
 * This function is the logical negation of square_isprojectable().
 */
bool square_iswall(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return !square_isprojectable(c, grid);
}

/**
 * True if the square is a permanent wall or one of the "stronger" walls.
 *
 * The stronger walls are granite, magma and quartz. This excludes things like
 * secret doors and rubble.
 */
bool square_isstrongwall(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return square_ismineral(c, grid) || square_isperm(c, grid);
}

/**
 * True if the cave square is internally lit.
 */
bool square_isbright(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return feat_is_bright(square(c, grid).feat);
}

/**
 * True if the cave square is fire-based.
 */
bool square_isfiery(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return feat_is_fiery(square(c, grid).feat);
}

/**
 * True if the cave square is lit.
 */
bool square_islit(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return square_light(c, grid) > 0 ? true : false;
}

/**
 * True if a lit wall should appear lit.
 */
bool square_islitwall(struct chunk *c, struct loc grid) {
	int d;
	assert(square_in_bounds(c, grid));
	for (d = 0; d < 8; d++) {
		struct loc adj = loc_sum(grid, ddgrid_ddd[d]);
		if (!square_in_bounds(c, adj)) continue;
		if (!square_isfloor(c, adj)) continue;
		if (!square_isroom(c, adj)) continue;
		if (!square_isglow(c, adj)) continue;
		if (!square_isseen(c, adj)) continue;
		return true;
	}

	return false;
}

/**
 * True if the cave square can damage the inhabitant - only lava so far
 */
bool square_isdamaging(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return feat_is_fiery(square(c, grid).feat);
}

/**
 * True if the cave square doesn't allow monster flow information.
 */
bool square_isnoflow(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return feat_is_no_flow(square(c, grid).feat);
}

/**
 * True if the cave square doesn't carry player scent.
 */
bool square_isnoscent(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	return feat_is_no_scent(square(c, grid).feat);
}

bool square_iswarded(struct chunk *c, struct loc grid)
{
	struct trap_kind *rune = lookup_trap("glyph of warding");
	return square_trap_specific(c, grid, rune->tidx);
}

bool square_isdecoyed(struct chunk *c, struct loc grid)
{
	struct trap_kind *glyph = lookup_trap("decoy");
	return square_trap_specific(c, grid, glyph->tidx);
}

bool square_iswebbed(struct chunk *c, struct loc grid)
{
	struct trap_kind *web = lookup_trap("web");
	return square_trap_specific(c, grid, web->tidx);
}

bool square_seemslikewall(struct chunk *c, struct loc grid)
{
	return tf_has(f_info[square(c, grid).feat].flags, TF_ROCK);
}

bool square_isinteresting(struct chunk *c, struct loc grid)
{
	int f = square(c, grid).feat;
	return tf_has(f_info[f].flags, TF_INTERESTING);
}

/**
 * True if the square is a closed, locked door.
 */
bool square_islockeddoor(struct chunk *c, struct loc grid)
{
	return square_door_power(c, grid) > 0;
}

/**
 * True if there is a player trap (known or unknown) in this square.
 */
bool square_isplayertrap(struct chunk *c, struct loc grid)
{
    return square_trap_flag(c, grid, TRF_TRAP);
}

/**
 * True if there is a visible trap in this square.
 */
bool square_isvisibletrap(struct chunk *c, struct loc grid)
{
    /* Look for a visible trap */
    return square_trap_flag(c, grid, TRF_VISIBLE);
}
/**
 * True if the square is an unknown player trap (it will appear as a floor tile)
 */
bool square_issecrettrap(struct chunk *c, struct loc grid)
{
    return !square_isvisibletrap(c, grid) && square_isplayertrap(c, grid);
}

/**
 * True if the square is a known, disabled player trap.
 */
bool square_isdisabledtrap(struct chunk *c, struct loc grid)
{
	return square_isvisibletrap(c, grid) &&
		(square_trap_timeout(c, grid, -1) > 0);
}

/**
 * True if the square is a known, disarmable player trap.
 */
bool square_isdisarmabletrap(struct chunk *c, struct loc grid)
{
	if (square_isdisabledtrap(c, grid)) return false;
	return square_isvisibletrap(c, grid) && square_isplayertrap(c, grid);
}

/**
 * Checks if a square is at the (inner) edge of a trap detect area
 */
bool square_dtrap_edge(struct chunk *c, struct loc grid)
{
	/* Check if the square is a dtrap in the first place */
	if (!square_isdtrap(c, grid)) return false;

	/* Check for non-dtrap adjacent grids */
	if (square_in_bounds_fully(c, next_grid(grid, DIR_S)) &&
		(!square_isdtrap(c, next_grid(grid, DIR_S))))
		return true;
	if (square_in_bounds_fully(c, next_grid(grid, DIR_E)) &&
		(!square_isdtrap(c, next_grid(grid, DIR_E))))
		return true;
	if (square_in_bounds_fully(c, next_grid(grid, DIR_N)) &&
		(!square_isdtrap(c, next_grid(grid, DIR_N))))
		return true;
	if (square_in_bounds_fully(c, next_grid(grid, DIR_W)) &&
		(!square_isdtrap(c, next_grid(grid, DIR_W))))
		return true;

	return false;
}

/**
 * Determine if a given location may be "destroyed"
 *
 * Used by destruction spells, and for placing stairs, etc.
 */
bool square_changeable(struct chunk *c, struct loc grid)
{
	struct object *obj;

	/* Forbid perma-grids */
	if (square_isperm(c, grid) || square_isshop(c, grid) ||
		square_isstairs(c, grid))
		return (false);

	/* Check objects */
	for (obj = square_object(c, grid); obj; obj = obj->next)
		/* Forbid artifact grids */
		if (obj->artifact) return (false);

	/* Accept */
	return (true);
}


bool square_in_bounds(struct chunk *c, struct loc grid)
{
	assert(c);
	return grid.x >= 0 && grid.x < c->width &&
		grid.y >= 0 && grid.y < c->height;
}

bool square_in_bounds_fully(struct chunk *c, struct loc grid)
{
	assert(c);
	return grid.x > 0 && grid.x < c->width - 1 &&
		grid.y > 0 && grid.y < c->height - 1;
}

/**
 * Checks if a square is thought by the player to block projections
 */
bool square_isbelievedwall(struct chunk *c, struct loc grid)
{
	// the edge of the world is definitely gonna block things
	if (!square_in_bounds_fully(c, grid)) return true;
	// if we dont know assume its projectable
	if (!square_isknown(c, grid)) return false;
	// report what we think (we may be wrong)
	return !square_isprojectable(player->cave, grid);
}

/**
 * Checks if a square is in a cul-de-sac
 */
bool square_suits_stairs_well(struct chunk *c, struct loc grid)
{
	if (square_isvault(c, grid) || square_isno_stairs(c, grid)) return false;
	return (square_num_walls_adjacent(c, grid) == 3) &&
		(square_num_walls_diagonal(c, grid) == 4) && square_isempty(c, grid);
}

/**
 * Checks if a square is in a corridor
 */
bool square_suits_stairs_ok(struct chunk *c, struct loc grid)
{
	if (square_isvault(c, grid) || square_isno_stairs(c, grid)) return false;
	return (square_num_walls_adjacent(c, grid) == 2) &&
		(square_num_walls_diagonal(c, grid) == 4) && square_isempty(c, grid);
}



/**
 * OTHER SQUARE FUNCTIONS
 *
 * Below are various square-specific functions which are not predicates
 */

struct square square(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds(c, grid));
	return c->squares[grid.y][grid.x];
}

struct feature *square_feat(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds(c, grid));
	return &f_info[square(c, grid).feat];
}

int square_light(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds(c, grid));
	return square(c, grid).light;
}

/**
 * Get a monster on the current level by its position.
 */
struct monster *square_monster(struct chunk *c, struct loc grid)
{
	if (!square_in_bounds(c, grid)) return NULL;
	if (square(c, grid).mon > 0) {
		struct monster *mon = cave_monster(c, square(c, grid).mon);
		return mon && mon->race ? mon : NULL;
	}

	return NULL;
}

/**
 * Get the top object of a pile on the current level by its position.
 */
struct object *square_object(struct chunk *c, struct loc grid) {
	if (!square_in_bounds(c, grid)) return NULL;
	return square(c, grid).obj;
}

/**
 * Get the first (and currently only) trap in a position on the current level.
 */
struct trap *square_trap(struct chunk *c, struct loc grid)
{
	if (!square_in_bounds(c, grid)) return NULL;
    return square(c, grid).trap;
}

/**
 * Return true if the given object is on the floor at this grid
 */
bool square_holds_object(struct chunk *c, struct loc grid, struct object *obj) {
	assert(square_in_bounds(c, grid));
	return pile_contains(square_object(c, grid), obj);
}

/**
 * Excise an object from a floor pile, leaving it orphaned.
 */
void square_excise_object(struct chunk *c, struct loc grid, struct object *obj){
	assert(square_in_bounds(c, grid));
	pile_excise(&c->squares[grid.y][grid.x].obj, obj);
}

/**
 * Excise an entire floor pile.
 */
void square_excise_pile(struct chunk *c, struct loc grid) {
	assert(square_in_bounds(c, grid));
	object_pile_free(square_object(c, grid));
	square_set_obj(c, grid, NULL);
}

/**
 * Excise an object from a floor pile and delete it while doing the other
 * necessary bookkeeping.  Normally, this is only called for the chunk
 * representing the true nature of the environment and not the one
 * representing the player's view of it.  If do_note is true, call
 * square_note_spot().  If do_light is true, call square_light_spot().
 * Unless calling this on the player's view, those both would be true
 * except as an optimization/simplification when the caller would call
 * square_note_spot()/square_light_spot() anyways or knows that those aren't
 * necessary.
 */
void square_delete_object(struct chunk *c, struct loc grid, struct object *obj, bool do_note, bool do_light)
{
	square_excise_object(c, grid, obj);
	delist_object(c, obj);
	object_delete(&obj);
	if (do_note) {
		square_note_spot(c, grid);
	}
	if (do_light) {
		square_light_spot(c, grid);
	}
}

/**
 * Sense the existence of objects on a grid in the current level
 */
void square_sense_pile(struct chunk *c, struct loc grid)
{
	struct object *obj;

	if (c != cave) return;

	/* Sense every item on this grid */
	for (obj = square_object(c, grid); obj; obj = obj->next) {
		object_sense(player, obj);
	}
}

/**
 * Update the player's knowledge of the objects on a grid in the current level
 */
void square_know_pile(struct chunk *c, struct loc grid)
{
	struct object *obj;

	if (c != cave) return;

	object_lists_check_integrity(c, player->cave);

	/* Know every item on this grid, greater knowledge for the player grid */
	for (obj = square_object(c, grid); obj; obj = obj->next) {
		object_see(player, obj);
		if (loc_eq(grid, player->grid)) {
			object_touch(player, obj);
		}
	}

	/* Remove known location of anything not on this grid */
	obj = square_object(player->cave, grid);
	while (obj) {
		struct object *next = obj->next;
		assert(c->objects[obj->oidx]);
		if (!square_holds_object(c, grid, c->objects[obj->oidx])) {
			struct object *original = c->objects[obj->oidx];
			square_excise_object(player->cave, grid, obj);
			obj->grid = loc(0, 0);

			/* Delete objects which no longer exist anywhere */
			if (obj->notice & OBJ_NOTICE_IMAGINED) {
				delist_object(player->cave, obj);
				object_delete(&obj);
				original->known = NULL;
				delist_object(c, original);
				object_delete(&original);
			}
		}
		obj = next;
	}
}


/**
 * Return how many cardinal directions around (x, y) contain walls.
 * \param c current chunk
 * \param y co-ordinates
 * \param x co-ordinates
 * \return the number of walls
 */
int square_num_walls_adjacent(struct chunk *c, struct loc grid)
{
    int k = 0;
    assert(square_in_bounds(c, grid));

    if (feat_is_wall(square(c, next_grid(grid, DIR_S)).feat)) k++;
	if (feat_is_wall(square(c, next_grid(grid, DIR_N)).feat)) k++;
    if (feat_is_wall(square(c, next_grid(grid, DIR_E)).feat)) k++;
    if (feat_is_wall(square(c, next_grid(grid, DIR_W)).feat)) k++;

    return k;
}

/**
 * Return how many diagonal directions around (x, y) contain walls.
 * \param c current chunk
 * \param y co-ordinates
 * \param x co-ordinates
 * \return the number of walls
 */
int square_num_walls_diagonal(struct chunk *c, struct loc grid)
{
    int k = 0;
    assert(square_in_bounds(c, grid));

    if (feat_is_wall(square(c, next_grid(grid, DIR_SE)).feat)) k++;
    if (feat_is_wall(square(c, next_grid(grid, DIR_NW)).feat)) k++;
    if (feat_is_wall(square(c, next_grid(grid, DIR_NE)).feat)) k++;
    if (feat_is_wall(square(c, next_grid(grid, DIR_SW)).feat)) k++;

    return k;
}


/**
 * Set the terrain type for a square.
 *
 * This should be the only function that sets terrain, apart from the savefile
 * loading code.
 */
void square_set_feat(struct chunk *c, struct loc grid, int feat)
{
	int current_feat;

	assert(square_in_bounds(c, grid));
	current_feat = square(c, grid).feat;

	/* Track changes */
	if (current_feat) c->feat_count[current_feat]--;
	if (feat) c->feat_count[feat]++;

	/* Make the change */
	c->squares[grid.y][grid.x].feat = feat;

	/* Light bright terrain */
	if (feat_is_bright(feat)) {
		sqinfo_on(square(c, grid).info, SQUARE_GLOW);
	}

	/* Make the new terrain feel at home */
	if (character_dungeon) {
		/* Remove traps if necessary */
		if (!square_player_trap_allowed(c, grid))
			square_destroy_trap(c, grid);

		square_note_spot(c, grid);
		square_light_spot(c, grid);
	} else {
		/* Make sure no incorrect wall flags set for dungeon generation */
		sqinfo_off(square(c, grid).info, SQUARE_WALL_INNER);
		sqinfo_off(square(c, grid).info, SQUARE_WALL_OUTER);
		sqinfo_off(square(c, grid).info, SQUARE_WALL_SOLID);
	}
}

/**
 * Set the player-"known" terrain type for a square.
 */
static void square_set_known_feat(struct chunk *c, struct loc grid, int feat)
{
	if (c != cave) return;
	player->cave->squares[grid.y][grid.x].feat = feat;
}

/**
 * Set the occupying monster for a square.
 */
void square_set_mon(struct chunk *c, struct loc grid, int midx)
{
	c->squares[grid.y][grid.x].mon = midx;
}

/**
 * Set the (first) object for a square.
 */
void square_set_obj(struct chunk *c, struct loc grid, struct object *obj)
{
	c->squares[grid.y][grid.x].obj = obj;
}

/**
 * Set the (first) trap for a square.
 */
void square_set_trap(struct chunk *c, struct loc grid, struct trap *trap)
{
	c->squares[grid.y][grid.x].trap = trap;
}

void square_add_trap(struct chunk *c, struct loc grid)
{
	assert(square_in_bounds_fully(c, grid));
	place_trap(c, grid, -1, c->depth);
}

void square_add_glyph(struct chunk *c, struct loc grid, int type)
{
	struct trap_kind *glyph = NULL;
	switch (type) {
		case GLYPH_WARDING: {
			glyph = lookup_trap("glyph of warding");
			break;
		}
		case GLYPH_DECOY: {
			glyph = lookup_trap("decoy");
			c->decoy = grid;
			break;
		}
		default: {
			msg("Non-existent glyph requested. Please report this bug.");
			return;
		}
	}
	place_trap(c, grid, glyph->tidx, 0);
}

void square_add_web(struct chunk *c, struct loc grid)
{
	struct trap_kind *web = lookup_trap("web");
	place_trap(c, grid, web->tidx, 0);
}

void square_add_stairs(struct chunk *c, struct loc grid, int depth) {
	int down = randint0(100) < 50;
	if (depth == 0)
		down = 1;
	else if (is_quest(depth) || depth >= z_info->max_depth - 1)
		down = 0;

	square_set_feat(c, grid, down ? FEAT_MORE : FEAT_LESS);
}

void square_add_door(struct chunk *c, struct loc grid, bool closed) {
	square_set_feat(c, grid, closed ? FEAT_CLOSED : FEAT_OPEN);
}

void square_open_door(struct chunk *c, struct loc grid)
{
	square_remove_all_traps(c, grid);
	square_set_feat(c, grid, FEAT_OPEN);
}

void square_close_door(struct chunk *c, struct loc grid)
{
	square_set_feat(c, grid, FEAT_CLOSED);
}

void square_smash_door(struct chunk *c, struct loc grid)
{
	square_remove_all_traps(c, grid);
	square_set_feat(c, grid, FEAT_BROKEN);
}

void square_unlock_door(struct chunk *c, struct loc grid) {
	assert(square_islockeddoor(c, grid));
	square_set_door_lock(c, grid, 0);
}

void square_destroy_door(struct chunk *c, struct loc grid) {
	assert(square_isdoor(c, grid));
	square_remove_all_traps(c, grid);
	square_set_feat(c, grid, FEAT_FLOOR);
}

void square_destroy_trap(struct chunk *c, struct loc grid)
{
	square_remove_all_traps(c, grid);
}

void square_disable_trap(struct chunk *c, struct loc grid)
{
	if (!square_isplayertrap(c, grid)) return;
	square_set_trap_timeout(c, grid, false, -1, 10);
}

void square_destroy_decoy(struct chunk *c, struct loc grid)
{
	square_remove_all_traps(c, grid);
	c->decoy = loc(0, 0);
	if (los(c, player->grid, grid) && !player->timed[TMD_BLIND]){
		msg("The decoy is destroyed!");
	}
}

void square_tunnel_wall(struct chunk *c, struct loc grid)
{
	square_set_feat(c, grid, FEAT_FLOOR);
}

void square_destroy_wall(struct chunk *c, struct loc grid)
{
	square_set_feat(c, grid, FEAT_FLOOR);
}

void square_smash_wall(struct chunk *c, struct loc grid)
{
	int i;
	square_set_feat(c, grid, FEAT_FLOOR);

	for (i = 0; i < 8; i++) {
		/* Extract adjacent location */
		struct loc adj_grid = loc_sum(grid, ddgrid_ddd[i]);

		/* Check legality */
		if (!square_in_bounds_fully(c, adj_grid)) continue;

		/* Ignore permanent grids */
		if (square_isperm(c, adj_grid)) continue;

		/* Give this grid a chance to survive */
		if ((square_isgranite(c, adj_grid) && one_in_(4)) ||
			(square_isquartz(c, adj_grid) && one_in_(10)) ||
			(square_ismagma(c, adj_grid) && one_in_(20))) {
			continue;
		}

		/* Remove it */
		square_set_feat(c, adj_grid, FEAT_FLOOR);
	}
}

void square_destroy(struct chunk *c, struct loc grid) {
	int feat = FEAT_FLOOR;
	int r = randint0(200);

	if (r < 20)
		feat = FEAT_GRANITE;
	else if (r < 70)
		feat = FEAT_QUARTZ;
	else if (r < 100)
		feat = FEAT_MAGMA;

	square_set_feat(c, grid, feat);
}

void square_earthquake(struct chunk *c, struct loc grid) {
	int t = randint0(100);
	int f;

	if (!square_ispassable(c, grid)) {
		square_set_feat(c, grid, FEAT_FLOOR);
		return;
	}

	if (t < 20)
		f = FEAT_GRANITE;
	else if (t < 70)
		f = FEAT_QUARTZ;
	else
		f = FEAT_MAGMA;
	square_set_feat(c, grid, f);
}

/**
 * Add visible treasure to a mineral square.
 */
void square_upgrade_mineral(struct chunk *c, struct loc grid)
{
	if (square(c, grid).feat == FEAT_MAGMA)
		square_set_feat(c, grid, FEAT_MAGMA_K);
	if (square(c, grid).feat == FEAT_QUARTZ)
		square_set_feat(c, grid, FEAT_QUARTZ_K);
}

void square_destroy_rubble(struct chunk *c, struct loc grid) {
	assert(square_isrubble(c, grid));
	square_set_feat(c, grid, FEAT_FLOOR);
}

void square_force_floor(struct chunk *c, struct loc grid) {
	square_set_feat(c, grid, FEAT_FLOOR);
}

/* Note that this returns the STORE_ index, which is one less than shopnum */
int square_shopnum(struct chunk *c, struct loc grid) {
	if (square_isshop(c, grid))
		return f_info[square(c, grid).feat].shopnum - 1;
	return -1;
}

int square_digging(struct chunk *c, struct loc grid) {
	if (square_isdiggable(c, grid))
		return f_info[square(c, grid).feat].dig;
	return 0;
}

const char *square_apparent_name(struct chunk *c, struct player *p, struct loc grid) {
	int actual = square(player->cave, grid).feat;
	char *mimic_name = f_info[actual].mimic;
	int f = mimic_name ? lookup_feat(mimic_name) : actual;
	return f_info[f].name;
}

/* Memorize the terrain */
void square_memorize(struct chunk *c, struct loc grid) {
	if (c != cave) return;
	square_set_known_feat(c, grid, square(c, grid).feat);
}

/* Forget the terrain */
void square_forget(struct chunk *c, struct loc grid) {
	if (c != cave) return;
	square_set_known_feat(c, grid, FEAT_NONE);
}

void square_mark(struct chunk *c, struct loc grid) {
	sqinfo_on(square(c, grid).info, SQUARE_MARK);
}

void square_unmark(struct chunk *c, struct loc grid) {
	sqinfo_off(square(c, grid).info, SQUARE_MARK);
}
