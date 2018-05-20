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
 * True is the feature is a solid wall (not rubble).
 */
bool feat_is_wall(int feat)
{
	return tf_has(f_info[feat].flags, TF_WALL);
}

/**
 * True is the feature is a floor.
 */
bool feat_is_floor(int feat)
{
	return tf_has(f_info[feat].flags, TF_FLOOR);
}

/**
 * True is the feature can hold a trap.
 */
bool feat_is_trap_holding(int feat)
{
	return tf_has(f_info[feat].flags, TF_TRAP);
}

/**
 * True is the feature can hold an object.
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
 * via c->squares[y][x].feat. All direct testing of c->squares[y][x].feat should be rewritten
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
bool square_isfloor(struct chunk *c, int y, int x)
{
	return feat_is_floor(c->squares[y][x].feat);
}

/**
 * True if the square can hold a trap.
 */
bool square_istrappable(struct chunk *c, int y, int x)
{
	return feat_is_trap_holding(c->squares[y][x].feat);
}

/**
 * True if the square can hold an object.
 */
bool square_isobjectholding(struct chunk *c, int y, int x)
{
	return feat_is_object_holding(c->squares[y][x].feat);
}

/**
 * True if the square is a normal granite rock wall.
 */
bool square_isrock(struct chunk *c, int y, int x)
{
	return (tf_has(f_info[c->squares[y][x].feat].flags, TF_GRANITE) &&
			!tf_has(f_info[c->squares[y][x].feat].flags, TF_DOOR_ANY));
}

/**
 * True if the square is a permanent wall.
 */
bool square_isperm(struct chunk *c, int y, int x)
{
	return (tf_has(f_info[c->squares[y][x].feat].flags, TF_PERMANENT) &&
			tf_has(f_info[c->squares[y][x].feat].flags, TF_ROCK));
}

/**
 * True if the square is a magma wall.
 */
bool square_ismagma(struct chunk *c, int y, int x)
{
	return feat_is_magma(c->squares[y][x].feat);
}

/**
 * True if the square is a quartz wall.
 */
bool square_isquartz(struct chunk *c, int y, int x)
{
	return feat_is_quartz(c->squares[y][x].feat);
}

/**
 * True if the square is granite.
 */
bool square_isgranite(struct chunk *c, int y, int x)
{
	return feat_is_granite(c->squares[y][x].feat);
}

/**
 * True if the square is a mineral wall (magma/quartz).
 */
bool square_ismineral(struct chunk *c, int y, int x)
{
	return square_isrock(c, y, x) || square_ismagma(c, y, x) ||
		square_isquartz(c, y, x);
}

bool square_hasgoldvein(struct chunk *c, int y, int x)
{
	return tf_has(f_info[c->squares[y][x].feat].flags, TF_GOLD);
}

/**
 * True if the square is rubble.
 */
bool square_isrubble(struct chunk *c, int y, int x)
{
    return (!tf_has(f_info[c->squares[y][x].feat].flags, TF_WALL) &&
			tf_has(f_info[c->squares[y][x].feat].flags, TF_ROCK));
}

/**
 * True if the square is a hidden secret door.
 *
 * These squares appear as if they were granite--when detected a secret door
 * is replaced by a closed door.
 */
bool square_issecretdoor(struct chunk *c, int y, int x)
{
    return (tf_has(f_info[c->squares[y][x].feat].flags, TF_DOOR_ANY) &&
			tf_has(f_info[c->squares[y][x].feat].flags, TF_ROCK));
}

/**
 * True if the square is an open door.
 */
bool square_isopendoor(struct chunk *c, int y, int x)
{
    return (tf_has(f_info[c->squares[y][x].feat].flags, TF_CLOSABLE));
}

/**
 * True if the square is a closed door (possibly locked or jammed).
 */
bool square_iscloseddoor(struct chunk *c, int y, int x)
{
	int feat = c->squares[y][x].feat;
	return tf_has(f_info[feat].flags, TF_DOOR_CLOSED);
}

bool square_isbrokendoor(struct chunk *c, int y, int x)
{
	int feat = c->squares[y][x].feat;
    return (tf_has(f_info[feat].flags, TF_DOOR_ANY) &&
			tf_has(f_info[feat].flags, TF_PASSABLE) &&
			!tf_has(f_info[feat].flags, TF_CLOSABLE));
}

/**
 * True if the square is a door.
 *
 * This includes open, closed, and hidden doors.
 */
bool square_isdoor(struct chunk *c, int y, int x)
{
	int feat = c->squares[y][x].feat;
	return tf_has(f_info[feat].flags, TF_DOOR_ANY);
}

/**
 * True if square is any stair
 */
bool square_isstairs(struct chunk *c, int y, int x)
{
	int feat = c->squares[y][x].feat;
	return tf_has(f_info[feat].flags, TF_STAIR);
}

/**
 * True if square is an up stair.
 */
bool square_isupstairs(struct chunk*c, int y, int x)
{
	int feat = c->squares[y][x].feat;
	return tf_has(f_info[feat].flags, TF_UPSTAIR);
}

/**
 * True if square is a down stair.
 */
bool square_isdownstairs(struct chunk *c, int y, int x)
{
	int feat = c->squares[y][x].feat;
	return tf_has(f_info[feat].flags, TF_DOWNSTAIR);
}

/**
 * True if the square is a shop entrance.
 */
bool square_isshop(struct chunk *c, int y, int x)
{
	return feat_is_shop(c->squares[y][x].feat);
}

/**
 * True if the square contains the player
 */
bool square_isplayer(struct chunk *c, int y, int x) {
	return c->squares[y][x].mon < 0 ? true : false;
}

/**
 * True if the square contains the player or a monster
 */
bool square_isoccupied(struct chunk *c, int y, int x) {
	return c->squares[y][x].mon != 0 ? true : false;
}

/**
 * True if the the player knows the terrain of the square
 */
bool square_isknown(struct chunk *c, int y, int x) {
	if (c != cave) return false;
	if (player->cave == NULL) return false;
	return player->cave->squares[y][x].feat == FEAT_NONE ? false : true;
}

/**
 * True if the the player's knowledge of the terrain of the square is wrong
 * or missing
 */
bool square_isnotknown(struct chunk *c, int y, int x) {
	if (c != cave) return false;
	if (player->cave == NULL) return true;
	return (player->cave->squares[y][x].feat != c->squares[y][x].feat);
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
bool square_ismark(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_MARK);
}

/**
 * True if the square is lit
 */
bool square_isglow(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_GLOW);
}

/**
 * True if the square is part of a vault.
 *
 * This doesn't say what kind of square it is, just that it is part of a vault.
 */
bool square_isvault(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_VAULT);
}

/**
 * True if the square is part of a room.
 */
bool square_isroom(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_ROOM);
}

/**
 * True if the square has been seen by the player
 */
bool square_isseen(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_SEEN);
}

/**
 * True if the cave square is currently viewable by the player
 */
bool square_isview(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_VIEW);
}

/**
 * True if the cave square was seen before the current update
 */
bool square_wasseen(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_WASSEEN);
}

/**
 * True if cave square is a feeling trigger square 
 */
bool square_isfeel(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_FEEL);
}

/**
 * True if the square has a known trap
 */
bool square_istrap(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_TRAP);
}

/**
 * True if the square has an unknown trap
 */
bool square_isinvis(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_INVIS);
}

/**
 * True if cave square is an inner wall (generation)
 */
bool square_iswall_inner(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_WALL_INNER);
}

/**
 * True if cave square is an outer wall (generation)
 */
bool square_iswall_outer(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_WALL_OUTER);
}

/**
 * True if cave square is a solid wall (generation)
 */
bool square_iswall_solid(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_WALL_SOLID);
}

/**
 * True if cave square has monster restrictions (generation)
 */
bool square_ismon_restrict(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_MON_RESTRICT);
}

/**
 * True if cave square can't be teleported from by the player
 */
bool square_isno_teleport(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_NO_TELEPORT);
}

/**
 * True if cave square can't be magically mapped by the player
 */
bool square_isno_map(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_NO_MAP);
}

/**
 * True if cave square can't be detected by player ESP
 */
bool square_isno_esp(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_NO_ESP);
}

/**
 * True if cave square is marked for projection processing
 */
bool square_isproject(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_PROJECT);
}

/**
 * True if cave square has been detected for traps
 */
bool square_isdtrap(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_DTRAP);
}

/**
 * True if cave square is inappropriate to place stairs
 */
bool square_isno_stairs(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->squares[y][x].info, SQUARE_NO_STAIRS);
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
bool square_isopen(struct chunk *c, int y, int x) {
	return square_isfloor(c, y, x) && !c->squares[y][x].mon;
}

/**
 * True if the square is empty (an open square without any items).
 */
bool square_isempty(struct chunk *c, int y, int x) {
	if (square_isplayertrap(c, y, x)) return false;
	return square_isopen(c, y, x) && !square_object(c, y, x);
}

/**
 * True if the square is empty (an open square without any items).
 */
bool square_isarrivable(struct chunk *c, int y, int x) {
	if (c->squares[y][x].mon) return false;
	if (square_isplayertrap(c, y, x)) return false;
	if (square_isfloor(c, y, x)) return true;
	if (square_isstairs(c, y, x)) return true;
	// maybe allow open doors or suchlike?
	return false;
}

/**
 * True if the square is an untrapped floor square without items.
 */
bool square_canputitem(struct chunk *c, int y, int x) {
	if (!square_isfloor(c, y, x))
		return false;
	if (square_iswarded(c, y, x) || square_isplayertrap(c, y, x))
		return false;
	return !square_object(c, y, x);
}

/**
 * True if the square can be dug: this includes rubble and non-permanent walls.
 */
bool square_isdiggable(struct chunk *c, int y, int x) {
	return (square_ismineral(c, y, x) ||
			square_issecretdoor(c, y, x) || 
			square_isrubble(c, y, x));
}

/**
 * True if a monster can walk through the tile.
 *
 * This is needed for polymorphing. A monster may be on a feature that isn't
 * an empty space, causing problems when it is replaced with a new monster.
 */
bool square_is_monster_walkable(struct chunk *c, int y, int x)
{
	assert(square_in_bounds(c, y, x));
	return feat_is_monster_walkable(c->squares[y][x].feat);
}

/**
 * True if the square is passable by the player.
 */
bool square_ispassable(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_is_passable(c->squares[y][x].feat);
}

/**
 * True if any projectable can pass through the square.
 *
 * This function is the logical negation of square_iswall().
 */
bool square_isprojectable(struct chunk *c, int y, int x) {
	if (!square_in_bounds(c, y, x)) return false;
	return feat_is_projectable(c->squares[y][x].feat);
}

/**
 * True if the square is a wall square (impedes the player).
 *
 * This function is the logical negation of square_isprojectable().
 */
bool square_iswall(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return !square_isprojectable(c, y, x);
}

/**
 * True if the square is a permanent wall or one of the "stronger" walls.
 *
 * The stronger walls are granite, magma and quartz. This excludes things like
 * secret doors and rubble.
 */
bool square_isstrongwall(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return square_ismineral(c, y, x) || square_isperm(c, y, x);
}

/**
 * True if the cave square is internally lit.
 */
bool square_isbright(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_is_bright(c->squares[y][x].feat);
}

/**
 * True if the cave square is fire-based.
 */
bool square_isfiery(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_is_fiery(c->squares[y][x].feat);
}

/**
 * True if the cave square is lit.
 */
bool square_islit(struct chunk *c, int y, int x) {
	int ny, nx;
	assert(square_in_bounds(c, y, x));

	/* Lit by itself or the player */
	if (square_isglow(c, y, x)) return true;
	if (player->state.cur_light > 0) return true;

	/* Lit by adjacent monster or terrain */
	for (ny = y - 1; ny <= y + 1; ny++) {
		for (nx = x - 1; nx <= x + 1; nx++) {
			struct monster *mon = square_monster(c, ny, nx);
			if (mon && rf_has(mon->race->flags, RF_HAS_LIGHT)) return true;
			if (square_isbright(c, ny, nx)) return true;
		}
	}

	/* Unlit */
	return false;
}

/**
 * True if the cave square can damage the inhabitant - only lava so far
 */
bool square_isdamaging(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_is_fiery(c->squares[y][x].feat);
}

/**
 * True if the cave square doesn't allow monster flow information.
 */
bool square_isnoflow(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_is_no_flow(c->squares[y][x].feat);
}

/**
 * True if the cave square doesn't carry player scent.
 */
bool square_isnoscent(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_is_no_scent(c->squares[y][x].feat);
}

bool square_iswarded(struct chunk *c, int y, int x)
{
	struct trap_kind *rune = lookup_trap("glyph of warding");
	return square_trap_specific(c, y, x, rune->tidx);
}

bool square_canward(struct chunk *c, int y, int x)
{
	return square_isfloor(c, y, x);
}

bool square_seemslikewall(struct chunk *c, int y, int x)
{
	return tf_has(f_info[c->squares[y][x].feat].flags, TF_ROCK);
}

bool square_isinteresting(struct chunk *c, int y, int x)
{
	int f = c->squares[y][x].feat;
	return tf_has(f_info[f].flags, TF_INTERESTING);
}

/**
 * True if the square is a closed, locked door.
 */
bool square_islockeddoor(struct chunk *c, int y, int x)
{
	return square_door_power(c, y, x) > 0;
}

/**
 * True if there is a player trap (known or unknown) in this square.
 */
bool square_isplayertrap(struct chunk *c, int y, int x)
{
    return square_trap_flag(c, y, x, TRF_TRAP);
}

/**
 * True if there is a visible trap in this square.
 */
bool square_isvisibletrap(struct chunk *c, int y, int x)
{
    /* Look for a visible trap */
    return square_trap_flag(c, y, x, TRF_VISIBLE);
}
/**
 * True if the square is an unknown player trap (it will appear as a floor tile)
 */
bool square_issecrettrap(struct chunk *c, int y, int x)
{
    return !square_isvisibletrap(c, y, x) && square_isplayertrap(c, y, x);
}

/**
 * True if the square is a known, disabled player trap.
 */
bool square_isdisabledtrap(struct chunk *c, int y, int x)
{
	return square_isvisibletrap(c, y, x) &&
		(square_trap_timeout(c, y, x, -1) > 0);
}

/**
 * True if the square is a known, disarmable player trap.
 */
bool square_isdisarmabletrap(struct chunk *c, int y, int x)
{
	if (square_isdisabledtrap(c, y, x)) return false;
	return square_isvisibletrap(c, y, x) && square_isplayertrap(c, y, x);
}

/**
 * Checks if a square is at the (inner) edge of a trap detect area
 */
bool square_dtrap_edge(struct chunk *c, int y, int x)
{
	/* Check if the square is a dtrap in the first place */
	if (!square_isdtrap(c, y, x)) return false;

	/* Check for non-dtrap adjacent grids */
	if (square_in_bounds_fully(c, y + 1, x) && (!square_isdtrap(c, y + 1, x)))
		return true;
	if (square_in_bounds_fully(c, y, x + 1) && (!square_isdtrap(c, y, x + 1)))
		return true;
	if (square_in_bounds_fully(c, y - 1, x) && (!square_isdtrap(c, y - 1, x)))
		return true;
	if (square_in_bounds_fully(c, y, x - 1) && (!square_isdtrap(c, y, x - 1)))
		return true;

	return false;
}

/**
 * Determine if a given location may be "destroyed"
 *
 * Used by destruction spells, and for placing stairs, etc.
 */
bool square_changeable(struct chunk *c, int y, int x)
{
	struct object *obj;

	/* Forbid perma-grids */
	if (square_isperm(c, y, x) || square_isshop(c, y, x) ||
		square_isstairs(c, y, x))
		return (false);

	/* Check objects */
	for (obj = square_object(c, y, x); obj; obj = obj->next)
		/* Forbid artifact grids */
		if (obj->artifact) return (false);

	/* Accept */
	return (true);
}


bool square_in_bounds(struct chunk *c, int y, int x)
{
	assert(c);
	return x >= 0 && x < c->width && y >= 0 && y < c->height;
}

bool square_in_bounds_fully(struct chunk *c, int y, int x)
{
	assert(c);
	return x > 0 && x < c->width - 1 && y > 0 && y < c->height - 1;
}

/**
 * Checks if a square is thought by the player to block projections
 */
bool square_isbelievedwall(struct chunk *c, int y, int x)
{
	// the edge of the world is definitely gonna block things
	if (!square_in_bounds_fully(c, y, x)) return true;
	// if we dont know assume its projectable
	if (!square_isknown(c, y, x)) return false;
	// report what we think (we may be wrong)
	return !square_isprojectable(player->cave, y, x);
}

/**
 * Checks if a square is in a cul-de-sac
 */
bool square_suits_stairs_well(struct chunk *c, int y, int x)
{
	if (square_isvault(c, y, x) || square_isno_stairs(c, y, x)) return false;
	return (square_num_walls_adjacent(c, y, x) == 3) &&
		(square_num_walls_diagonal(c, y, x) == 4) && square_isempty(c, y, x);
}

/**
 * Checks if a square is in a corridor
 */
bool square_suits_stairs_ok(struct chunk *c, int y, int x)
{
	if (square_isvault(c, y, x) || square_isno_stairs(c, y, x)) return false;
	return (square_num_walls_adjacent(c, y, x) == 2) &&
		(square_num_walls_diagonal(c, y, x) == 4) && square_isempty(c, y, x);
}



/**
 * OTHER SQUARE FUNCTIONS
 *
 * Below are various square-specific functions which are not predicates
 */

struct feature *square_feat(struct chunk *c, int y, int x)
{
	assert(square_in_bounds(c, y, x));
	return &f_info[c->squares[y][x].feat];
}

/**
 * Get a monster on the current level by its position.
 */
struct monster *square_monster(struct chunk *c, int y, int x)
{
	if (!square_in_bounds(c, y, x)) return NULL;
	if (c->squares[y][x].mon > 0) {
		struct monster *mon = cave_monster(c, c->squares[y][x].mon);
		return mon->race ? mon : NULL;
	}

	return NULL;
}

/**
 * Get the top object of a pile on the current level by its position.
 */
struct object *square_object(struct chunk *c, int y, int x) {
	if (!square_in_bounds(c, y, x)) return NULL;
	return c->squares[y][x].obj;
}

/**
 * Get the first (and currently only) trap in a position on the current level.
 */
struct trap *square_trap(struct chunk *c, int y, int x)
{
	if (!square_in_bounds(c, y, x)) return NULL;
    return c->squares[y][x].trap;
}

/**
 * Return true if the given object is on the floor at this grid
 */
bool square_holds_object(struct chunk *c, int y, int x, struct object *obj) {
	assert(square_in_bounds(c, y, x));
	return pile_contains(square_object(c, y, x), obj);
}

/**
 * Excise an object from a floor pile, leaving it orphaned.
 */
void square_excise_object(struct chunk *c, int y, int x, struct object *obj) {
	assert(square_in_bounds(c, y, x));
	pile_excise(&c->squares[y][x].obj, obj);
}

/**
 * Excise an entire floor pile.
 */
void square_excise_pile(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	object_pile_free(square_object(c, y, x));
	c->squares[y][x].obj = NULL;
}

/**
 * Sense the existence of objects on a grid in the current level
 */
void square_sense_pile(struct chunk *c, int y, int x)
{
	struct object *obj;

	if (c != cave) return;

	/* Sense every item on this grid */
	for (obj = square_object(c, y, x); obj; obj = obj->next) {
		object_sense(player, obj);
	}
}

/**
 * Update the player's knowledge of the objects on a grid in the current level
 */
void square_know_pile(struct chunk *c, int y, int x)
{
	struct object *obj;

	if (c != cave) return;

	object_lists_check_integrity(c, player->cave);

	/* Know every item on this grid, greater knowledge for the player grid */
	for (obj = square_object(c, y, x); obj; obj = obj->next) {
		object_see(player, obj);
		if ((y == player->py) && (x == player->px)) {
			object_touch(player, obj);
		}
	}

	/* Remove known location of anything not on this grid */
	obj = square_object(player->cave, y, x);
	while (obj) {
		struct object *next = obj->next;
		assert(c->objects[obj->oidx]);
		if (!square_holds_object(c, y, x, c->objects[obj->oidx])) {
			struct object *original = c->objects[obj->oidx];
			square_excise_object(player->cave, y, x, obj);
			obj->iy = 0;
			obj->ix = 0;

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
int square_num_walls_adjacent(struct chunk *c, int y, int x)
{
    int k = 0;
    assert(square_in_bounds(c, y, x));

    if (feat_is_wall(c->squares[y + 1][x].feat)) k++;
    if (feat_is_wall(c->squares[y - 1][x].feat)) k++;
    if (feat_is_wall(c->squares[y][x + 1].feat)) k++;
    if (feat_is_wall(c->squares[y][x - 1].feat)) k++;

    return k;
}

/**
 * Return how many diagonal directions around (x, y) contain walls.
 * \param c current chunk
 * \param y co-ordinates
 * \param x co-ordinates
 * \return the number of walls
 */
int square_num_walls_diagonal(struct chunk *c, int y, int x)
{
    int k = 0;
    assert(square_in_bounds(c, y, x));

    if (feat_is_wall(c->squares[y + 1][x + 1].feat)) k++;
    if (feat_is_wall(c->squares[y - 1][x - 1].feat)) k++;
    if (feat_is_wall(c->squares[y - 1][x + 1].feat)) k++;
    if (feat_is_wall(c->squares[y + 1][x - 1].feat)) k++;

    return k;
}


/**
 * Set the terrain type for a square.
 *
 * This should be the only function that sets terrain, apart from the savefile
 * loading code.
 */
void square_set_feat(struct chunk *c, int y, int x, int feat)
{
	int current_feat;

	assert(square_in_bounds(c, y, x));
	current_feat = c->squares[y][x].feat;

	/* Track changes */
	if (current_feat) c->feat_count[current_feat]--;
	if (feat) c->feat_count[feat]++;

	/* Make the change */
	c->squares[y][x].feat = feat;

	/* Light bright terrain */
	if (feat_is_bright(feat)) {
		sqinfo_on(c->squares[y][x].info, SQUARE_GLOW);
	}

	/* Make the new terrain feel at home */
	if (character_dungeon) {
		/* Remove traps if necessary */
		if (!square_player_trap_allowed(c, y, x))
			square_destroy_trap(c, y, x);

		square_note_spot(c, y, x);
		square_light_spot(c, y, x);
	} else {
		/* Make sure no incorrect wall flags set for dungeon generation */
		sqinfo_off(c->squares[y][x].info, SQUARE_WALL_INNER);
		sqinfo_off(c->squares[y][x].info, SQUARE_WALL_OUTER);
		sqinfo_off(c->squares[y][x].info, SQUARE_WALL_SOLID);
	}
}

void square_add_trap(struct chunk *c, int y, int x)
{
	assert(square_in_bounds_fully(c, y, x));
	place_trap(c, y, x, -1, c->depth);
}

void square_add_ward(struct chunk *c, int y, int x)
{
	struct trap_kind *rune = lookup_trap("glyph of warding");
	place_trap(c, y, x, rune->tidx, 0);
}

void square_add_stairs(struct chunk *c, int y, int x, int depth) {
	int down = randint0(100) < 50;
	if (depth == 0)
		down = 1;
	else if (is_quest(depth) || depth >= z_info->max_depth - 1)
		down = 0;

	square_set_feat(c, y, x, down ? FEAT_MORE : FEAT_LESS);
}

void square_add_door(struct chunk *c, int y, int x, bool closed) {
	square_set_feat(c, y, x, closed ? FEAT_CLOSED : FEAT_OPEN);
}

void square_open_door(struct chunk *c, int y, int x)
{
	square_remove_all_traps(c, y, x);
	square_set_feat(c, y, x, FEAT_OPEN);
}

void square_close_door(struct chunk *c, int y, int x)
{
	square_set_feat(c, y, x, FEAT_CLOSED);
}

void square_smash_door(struct chunk *c, int y, int x)
{
	square_remove_all_traps(c, y, x);
	square_set_feat(c, y, x, FEAT_BROKEN);
}

void square_unlock_door(struct chunk *c, int y, int x) {
	assert(square_islockeddoor(c, y, x));
	square_set_door_lock(c, y, x, 0);
}

void square_destroy_door(struct chunk *c, int y, int x) {
	assert(square_isdoor(c, y, x));
	square_remove_all_traps(c, y, x);
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_destroy_trap(struct chunk *c, int y, int x)
{
	square_remove_all_traps(c, y, x);
}

void square_disable_trap(struct chunk *c, int y, int x)
{
	if (!square_isplayertrap(c, y, x)) return;
	square_set_trap_timeout(c, y, x, false, -1, 10);
}

void square_tunnel_wall(struct chunk *c, int y, int x)
{
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_destroy_wall(struct chunk *c, int y, int x)
{
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_destroy(struct chunk *c, int y, int x) {
	int feat = FEAT_FLOOR;
	int r = randint0(200);

	if (r < 20)
		feat = FEAT_GRANITE;
	else if (r < 70)
		feat = FEAT_QUARTZ;
	else if (r < 100)
		feat = FEAT_MAGMA;

	square_set_feat(c, y, x, feat);
}

void square_earthquake(struct chunk *c, int y, int x) {
	int t = randint0(100);
	int f;

	if (!square_ispassable(c, y, x)) {
		square_set_feat(c, y, x, FEAT_FLOOR);
		return;
	}

	if (t < 20)
		f = FEAT_GRANITE;
	else if (t < 70)
		f = FEAT_QUARTZ;
	else
		f = FEAT_MAGMA;
	square_set_feat(c, y, x, f);
}

void square_remove_ward(struct chunk *c, int y, int x)
{
	assert(square_iswarded(c, y, x));

	struct trap_kind *rune = lookup_trap("glyph of warding");
	square_remove_trap(c, y, x, rune->tidx);
}

/**
 * Add visible treasure to a mineral square.
 */
void square_upgrade_mineral(struct chunk *c, int y, int x)
{
	if (c->squares[y][x].feat == FEAT_MAGMA)
		square_set_feat(c, y, x, FEAT_MAGMA_K);
	if (c->squares[y][x].feat == FEAT_QUARTZ)
		square_set_feat(c, y, x, FEAT_QUARTZ_K);
}

void square_destroy_rubble(struct chunk *c, int y, int x) {
	assert(square_isrubble(c, y, x));
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_force_floor(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_FLOOR);
}

/* Note that this returns the STORE_ index, which is one less than shopnum */
int square_shopnum(struct chunk *c, int y, int x) {
	if (square_isshop(c, y, x))
		return f_info[c->squares[y][x].feat].shopnum - 1;
	return -1;
}

int square_digging(struct chunk *c, int y, int x) {
	if (square_isdiggable(c, y, x))
		return f_info[c->squares[y][x].feat].dig;
	return 0;
}

const char *square_apparent_name(struct chunk *c, struct player *p, int y, int x) {
	int actual = player->cave->squares[y][x].feat;
	char *mimic_name = f_info[actual].mimic;
	int f = mimic_name ? lookup_feat(mimic_name) : actual;
	return f_info[f].name;
}

void square_memorize(struct chunk *c, int y, int x) {
	if (c != cave) return;
	player->cave->squares[y][x].feat = c->squares[y][x].feat;
}

void square_forget(struct chunk *c, int y, int x) {
	if (c != cave) return;
	player->cave->squares[y][x].feat = FEAT_NONE;
}

void square_mark(struct chunk *c, int y, int x) {
	sqinfo_on(c->squares[y][x].info, SQUARE_MARK);
}

void square_unmark(struct chunk *c, int y, int x) {
	sqinfo_off(c->squares[y][x].info, SQUARE_MARK);
}
