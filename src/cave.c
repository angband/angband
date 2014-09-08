/*
 * File: cave.c
 * Purpose: Lighting and update functions
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
#include "cmds.h"
#include "dungeon.h"
#include "cmd-core.h"
#include "game-event.h"
#include "init.h"
#include "monster.h"
#include "obj-ignore.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "tables.h"
#include "trap.h"

feature_type *f_info;

/*
 * Determine if a given location may be "destroyed"
 *
 * Used by destruction spells, and for placing stairs, etc.
 */
bool square_valid_bold(int y, int x)
{
	object_type *o_ptr;

	/* Forbid perma-grids */
	if (square_isperm(cave, y, x) || square_isshop(cave, y, x) || 
		square_isstairs(cave, y, x)) return (FALSE);

	/* Check objects */
	for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
	{
		/* Forbid artifact grids */
		if (o_ptr->artifact) return (FALSE);
	}

	/* Accept */
	return (TRUE);
}


/* 
 * Checks if a square is at the (inner) edge of a trap detect area 
 */ 
bool dtrap_edge(int y, int x) 
{ 
	/* Check if the square is a dtrap in the first place */ 
	if (!square_isdtrap(cave, y, x)) return FALSE;

 	/* Check for non-dtrap adjacent grids */ 
	if (square_in_bounds_fully(cave, y + 1, x) &&
		(!square_isdtrap(cave, y + 1, x)))
		return TRUE;
	if (square_in_bounds_fully(cave, y, x + 1) &&
		(!square_isdtrap(cave, y, x + 1)))
		return TRUE;
	if (square_in_bounds_fully(cave, y - 1, x) &&
		(!square_isdtrap(cave, y - 1, x)))
		return TRUE;
	if (square_in_bounds_fully(cave, y, x - 1) &&
		(!square_isdtrap(cave, y, x - 1)))
		return TRUE;

	return FALSE; 
}



struct feature *square_feat(struct chunk *c, int y, int x)
{
	assert(c);
	assert(y >= 0 && y < c->height);
	assert(x >= 0 && x < c->width);

	return &f_info[c->feat[y][x]];
}

void square_set_feat(struct chunk *c, int y, int x, int feat)
{
	int current_feat = c->feat[y][x];

	assert(c);
	assert(y >= 0 && y < c->height);
	assert(x >= 0 && x < c->width);

	/* Track changes */
	if (current_feat) c->feat_count[current_feat]--;
	if (feat) c->feat_count[feat]++;

	/* Make the change */
	c->feat[y][x] = feat;

	/* Make the new terrain feel at home */
	if (character_dungeon) {
		square_note_spot(c, y, x);
		square_light_spot(c, y, x);
	} else {
		/* Make sure no incorrect wall flags set for dungeon generation */
		   sqinfo_off(c->info[y][x], SQUARE_WALL_INNER);
		   sqinfo_off(c->info[y][x], SQUARE_WALL_OUTER);
		   sqinfo_off(c->info[y][x], SQUARE_WALL_SOLID);
	}
}

bool square_in_bounds(struct chunk *c, int y, int x)
{
	return x >= 0 && x < c->width && y >= 0 && y < c->height;
}

bool square_in_bounds_fully(struct chunk *c, int y, int x)
{
	return x > 0 && x < c->width - 1 && y > 0 && y < c->height - 1;
}


/*
 * Standard "find me a location" function
 *
 * Obtains a legal location within the given distance of the initial
 * location, and with "los()" from the source to destination location.
 *
 * This function is often called from inside a loop which searches for
 * locations while increasing the "d" distance.
 *
 * need_los determines whether line of sight is needed
 */
void scatter(struct chunk *c, int *yp, int *xp, int y, int x, int d, bool need_los)
{
	int nx, ny;


	/* Pick a location */
	while (TRUE)
	{
		/* Pick a new location */
		ny = rand_spread(y, d);
		nx = rand_spread(x, d);

		/* Ignore annoying locations */
		if (!square_in_bounds_fully(c, ny, nx)) continue;

		/* Ignore "excessively distant" locations */
		if ((d > 1) && (distance(y, x, ny, nx) > d)) continue;
		
		/* Don't need los */
		if (!need_los) break;

		/* Require "line of sight" if set */
		if (need_los && (los(c, y, x, ny, nx))) break;
	}

	/* Save the location */
	(*yp) = ny;
	(*xp) = nx;
}


struct chunk *cave = NULL;

struct chunk *cave_new(int height, int width) {
	int y, x;

	struct chunk *c = mem_zalloc(sizeof *c);
	c->height = height;
	c->width = width;
	c->feat_count = mem_zalloc((z_info->f_max + 1) * sizeof(int));
	c->info = mem_zalloc(c->height * sizeof(bitflag**));
	c->feat = mem_zalloc(c->height * sizeof(byte*));
	c->cost = mem_zalloc(c->height * sizeof(byte*));
	c->when = mem_zalloc(c->height * sizeof(byte*));
	c->m_idx = mem_zalloc(c->height * sizeof(s16b*));
	c->o_idx = mem_zalloc(c->height * sizeof(s16b*));
	for (y = 0; y < c->height; y++){
		c->info[y] = mem_zalloc(c->width * sizeof(bitflag*));
		for (x = 0; x < c->width; x++)
			c->info[y][x] = mem_zalloc(SQUARE_SIZE * sizeof(bitflag));
		c->feat[y] = mem_zalloc(c->width * sizeof(byte));
		c->cost[y] = mem_zalloc(c->width * sizeof(byte));
		c->when[y] = mem_zalloc(c->width * sizeof(byte));
		c->m_idx[y] = mem_zalloc(c->width * sizeof(s16b));
		c->o_idx[y] = mem_zalloc(c->width * sizeof(s16b));
	}

	c->monsters = mem_zalloc(z_info->m_max * sizeof(struct monster));
	c->mon_max = 1;
	c->mon_current = -1;

	c->objects = mem_zalloc(z_info->o_max * sizeof(struct object));
	c->obj_max = 1;

	c->traps = mem_zalloc(z_info->l_max * sizeof(struct trap));
	c->trap_max = 1;

	c->created_at = turn;
	return c;
}

void cave_free(struct chunk *c) {
	int y, x;
	for (y = 0; y < c->height; y++){
		for (x = 0; x < c->width; x++)
			mem_free(c->info[y][x]);
		mem_free(c->info[y]);
		mem_free(c->feat[y]);
		mem_free(c->cost[y]);
		mem_free(c->when[y]);
		mem_free(c->m_idx[y]);
		mem_free(c->o_idx[y]);
	}
	mem_free(c->feat_count);
	mem_free(c->info);
	mem_free(c->feat);
	mem_free(c->cost);
	mem_free(c->when);
	mem_free(c->m_idx);
	mem_free(c->o_idx);
	mem_free(c->monsters);
	mem_free(c->objects);
	mem_free(c->traps);
	mem_free(c);
}

/**
 * FEATURE PREDICATES
 *
 * These functions are used to figure out what kind of square something is,
 * via c->feat[y][x]. All direct testing of c->feat[y][x] should be rewritten
 * in terms of these functions.
 *
 * It's often better to use feature behavior predicates (written in terms of
 * these functions) instead of these functions directly. For instance,
 * square_isrock() will return false for a secret door, even though it will
 * behave like a rock wall until the player determines it's a door.
 *
 * Use functions like square_isdiggable, square_iswall, etc. in these cases.
 */

/**
 * True if the square is normal open floor.
 */
bool square_isfloor(struct chunk *c, int y, int x) {
	return tf_has(f_info[c->feat[y][x]].flags, TF_FLOOR);
}

/**
 * True if the square is a normal granite rock wall.
 */
bool square_isrock(struct chunk *c, int y, int x) {
	return (tf_has(f_info[c->feat[y][x]].flags, TF_GRANITE) &&
			!tf_has(f_info[c->feat[y][x]].flags, TF_DOOR_ANY));
}

/**
 * True if the square is a permanent wall.
 */
bool square_isperm(struct chunk *c, int y, int x) {
	return (tf_has(f_info[c->feat[y][x]].flags, TF_PERMANENT) &&
			tf_has(f_info[c->feat[y][x]].flags, TF_ROCK));
}

/**
 * True if the square is a magma wall.
 */
bool feat_is_magma(int feat)
{
	return tf_has(f_info[feat].flags, TF_MAGMA);
}

/**
 * True if the square is a magma wall.
 */
bool square_ismagma(struct chunk *c, int y, int x) {
	return feat_is_magma(c->feat[y][x]);
}

/**
 * True if the square is a quartz wall.
 */
bool feat_is_quartz(int feat)
{
	return tf_has(f_info[feat].flags, TF_QUARTZ);
}

/**
 * True if the square is a quartz wall.
 */
bool square_isquartz(struct chunk *c, int y, int x) {
	return feat_is_quartz(c->feat[y][x]);
}

/**
 * True if the square is a mineral wall (magma/quartz).
 */
bool square_ismineral(struct chunk *c, int y, int x) {
	return square_isrock(c, y, x) || square_ismagma(c, y, x) || square_isquartz(c, y, x);
}

/**
 * True if the square is a mineral wall with treasure (magma/quartz).
 */
bool feat_is_treasure(int feat) {
	return (tf_has(f_info[feat].flags, TF_GOLD) &&
			tf_has(f_info[feat].flags, TF_INTERESTING));
}

/**
 * True if the square is rubble.
 */
bool square_isrubble(struct chunk *c, int y, int x) {
    return (!tf_has(f_info[c->feat[y][x]].flags, TF_WALL) &&
			tf_has(f_info[c->feat[y][x]].flags, TF_ROCK));
}

/**
 * True if the square is a hidden secret door.
 *
 * These squares appear as if they were granite--when detected a secret door
 * is replaced by a closed door.
 */
bool square_issecretdoor(struct chunk *c, int y, int x) {
    return (tf_has(f_info[c->feat[y][x]].flags, TF_DOOR_ANY) &&
			tf_has(f_info[c->feat[y][x]].flags, TF_ROCK));
}

/**
 * True if the square is an open door.
 */
bool square_isopendoor(struct chunk *c, int y, int x) {
    return (tf_has(f_info[c->feat[y][x]].flags, TF_CLOSABLE));
}

/**
 * True if the square is a closed door (possibly locked or jammed).
 */
bool square_iscloseddoor(struct chunk *c, int y, int x) {
	int feat = c->feat[y][x];
	return tf_has(f_info[feat].flags, TF_DOOR_CLOSED);
}

/**
 * True if the square is a closed, locked door.
 */
bool square_islockeddoor(struct chunk *c, int y, int x) {
	int feat = c->feat[y][x];
	return (tf_has(f_info[feat].flags, TF_DOOR_LOCKED) ||
			tf_has(f_info[feat].flags, TF_DOOR_JAMMED));
}

/**
 * True if the square is a door.
 *
 * This includes open, closed, and hidden doors.
 */
bool square_isdoor(struct chunk *c, int y, int x) {
	int feat = c->feat[y][x];
	return tf_has(f_info[feat].flags, TF_DOOR_ANY);
}

/**
 * True if the square is an unknown trap (it will appear as a floor tile).
 */
bool square_issecrettrap(struct chunk *c, int y, int x) {
    return square_invisible_trap(c, y, x) && square_player_trap(c, y, x);
}

/**
 * True is the feature is a solid wall (not rubble).
 */
bool feat_is_wall(int feat) {
	return tf_has(f_info[feat].flags, TF_WALL);
}

/**
 * True if the square is a known player trap.
 */
bool square_isknowntrap(struct chunk *c, int y, int x) {
	return square_visible_trap(c, y, x) && square_player_trap(c, y, x);
}

/**
 * True if the feature is a shop entrance.
 */
bool feature_isshop(int feat) {
	return tf_has(f_info[feat].flags, TF_SHOP);
}

/**
 * True if square is any stair
 */
bool square_isstairs(struct chunk*c, int y, int x) {
	int feat = c->feat[y][x];
	return tf_has(f_info[feat].flags, TF_STAIR);
}

/**
 * True if square is an up stair.
 */
bool square_isupstairs(struct chunk*c, int y, int x) {
	int feat = c->feat[y][x];
	return tf_has(f_info[feat].flags, TF_UPSTAIR);
}

/**
 * True if square is a down stair.
 */
bool square_isdownstairs(struct chunk *c, int y, int x) {
	int feat = c->feat[y][x];
	return tf_has(f_info[feat].flags, TF_DOWNSTAIR);
}

/**
 * True if the square is a shop entrance.
 */
bool square_isshop(struct chunk *c, int y, int x) {
	return feature_isshop(c->feat[y][x]);
}

int square_shopnum(struct chunk *c, int y, int x) {
	if (square_isshop(c, y, x))
		return c->feat[y][x] - FEAT_SHOP_HEAD;
	return -1;
}

/**
 * True if the square contains the player
 */
bool square_isplayer(struct chunk *c, int y, int x) {
	return c->m_idx[y][x] < 0 ? TRUE : FALSE;
}

/**
 * SQUARE BEHAVIOR PREDICATES
 *
 * These functions define how a given square behaves, e.g. whether it is
 * passable by the player, whether it is diggable, contains items, etc.
 *
 * These functions use the FEATURE PREDICATES (as well as c->info) to make
 * the determination.
 */

/**
 * True if the square is open (a floor square not occupied by a monster).
 */
bool square_isopen(struct chunk *c, int y, int x) {
	return square_isfloor(c, y, x) && !c->m_idx[y][x];
}

/**
 * True if the square is empty (an open square without any items).
 */
bool square_isempty(struct chunk *c, int y, int x) {
	return square_isopen(c, y, x) && !c->o_idx[y][x];
}

/**
 * True if the square is a floor square without items.
 */
bool square_canputitem(struct chunk *c, int y, int x) {
	return square_isfloor(c, y, x) && !c->o_idx[y][x];
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
 * True if a monster can walk through the feature.
 */
bool feat_is_monster_walkable(feature_type *feature)
{
	return tf_has(feature->flags, TF_PASSABLE);
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
	return feat_is_monster_walkable(&f_info[c->feat[y][x]]);
}

/**
 * True if the feature is passable by the player.
 */
bool feat_ispassable(feature_type *f_ptr) {
	return tf_has(f_ptr->flags, TF_PASSABLE);
}

/**
 * True if the square is passable by the player.
 */
bool square_ispassable(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_ispassable(&f_info[c->feat[y][x]]);
}

/**
 * True if any projectable can pass through the feature.
 */
bool feat_isprojectable(feature_type *f_ptr) {
	return tf_has(f_ptr->flags, TF_PROJECT);
}

/**
 * True if any projectable can pass through the square.
 *
 * This function is the logical negation of square_iswall().
 */
bool square_isprojectable(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_isprojectable(&f_info[c->feat[y][x]]);
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
 * True if a square's terrain is memorized by the player
 */
bool square_ismark(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_MARK);
}

/**
 * True if the square is lit
 */
bool square_isglow(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_GLOW);
}

/**
 * True if the square is part of a vault.
 *
 * This doesn't say what kind of square it is, just that it is part of a vault.
 */
bool square_isvault(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_VAULT);
}

/**
 * True if the square is part of a room.
 */
bool square_isroom(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_ROOM);
}

/**
 * True if the square has been seen by the player
 */
bool square_isseen(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_SEEN);
}

/**
 * True if the cave square is currently viewable by the player
 */
bool square_isview(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_VIEW);
}

/**
 * True if the cave square was seen before the current update
 */
bool square_wasseen(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_WASSEEN);
}

/**
 * True if the square has been detected for traps
 */
bool square_isdtrap(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_DTRAP);
}

/**
 * True if cave square is a feeling trigger square 
 */
bool square_isfeel(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_FEEL);
}

/**
 * True if the square is on the trap detection edge
 */
bool square_isdedge(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_DEDGE);
}

/**
 * True if the square has a known trap
 */
bool square_istrap(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_TRAP);
}

/**
 * True if the square has an unknown trap
 */
bool square_isinvis(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_INVIS);
}

/**
 * True if cave square is an inner wall (generation)
 */
bool square_iswall_inner(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_WALL_INNER);
}

/**
 * True if cave square is an outer wall (generation)
 */
bool square_iswall_outer(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_WALL_OUTER);
}

/**
 * True if cave square is a solid wall (generation)
 */
bool square_iswall_solid(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_WALL_SOLID);
}

/**
 * True if cave square has monster restrictions (generation)
 */
bool square_ismon_restrict(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_MON_RESTRICT);
}

/**
 * True if cave square can't be teleported from by the player
 */
bool square_isno_teleport(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_NO_TELEPORT);
}

/**
 * True if cave square can't be magically mapped by the player
 */
bool square_isno_map(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_NO_MAP);
}

/**
 * True if cave square can't be detected by player ESP
 */
bool square_isno_esp(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return sqinfo_has(c->info[y][x], SQUARE_NO_ESP);
}

/**
 * True if the feature is "boring".
 */
bool feat_isboring(feature_type *f_ptr) {
	return !tf_has(f_ptr->flags, TF_INTERESTING);
}

/**
 * True if the cave square is "boring".
 */
bool square_isboring(struct chunk *c, int y, int x) {
	assert(square_in_bounds(c, y, x));
	return feat_isboring(&f_info[c->feat[y][x]]);
}

/**
 * Get a monster on the current level by its index.
 */
struct monster *cave_monster(struct chunk *c, int idx) {
	if (idx <= 0) return NULL;
	return &c->monsters[idx];
}

/**
 * Get a monster on the current level by its position.
 */
struct monster *square_monster(struct chunk *c, int y, int x) {
	if (c->m_idx[y][x] > 0) {
		struct monster *mon = cave_monster(c, c->m_idx[y][x]);
		return mon->race ? mon : NULL;
	}

	return NULL;
}

/**
 * The maximum number of monsters allowed in the level.
 */
int cave_monster_max(struct chunk *c) {
	return c->mon_max;
}

/**
 * The current number of monsters present on the level.
 */
int cave_monster_count(struct chunk *c) {
	return c->mon_cnt;
}

/**
 * Get an object on the current level by its index.
 */
struct object *cave_object(struct chunk *c, int idx) {
	assert(idx > 0);
	assert(idx <= z_info->o_max);
	return &c->objects[idx];
}

/**
 * Get the top object of a pile on the current level by its position.
 */
struct object *square_object(struct chunk *c, int y, int x) {
	if (c->o_idx[y][x] > 0) {
	struct object *obj = cave_object(c, c->o_idx[y][x]);
	return obj->kind ? obj : NULL;
	}

	return NULL;
}

/**
 * The maximum number of objects allowed in the level.
 */
int cave_object_max(struct chunk *c) {
	return c->obj_max;
}

/**
 * The current number of objects present on the level.
 */
int cave_object_count(struct chunk *c) {
	return c->obj_cnt;
}

/**
 * Get a trap on the current level by its index.
 */
struct trap *cave_trap(struct chunk *c, int idx) {
	return &c->traps[idx];
}

/**
 * The maximum number of traps allowed in the level.
 */
int cave_trap_max(struct chunk *c) {
	return c->trap_max;
}

/**
 * Add visible treasure to a mineral square.
 */
void upgrade_mineral(struct chunk *c, int y, int x) {
	switch (c->feat[y][x]) {
	case FEAT_MAGMA: square_set_feat(c, y, x, FEAT_MAGMA_K); break;
	case FEAT_QUARTZ: square_set_feat(c, y, x, FEAT_QUARTZ_K); break;
	}
}

int square_door_power(struct chunk *c, int y, int x) {
	return (c->feat[y][x] & 0x07);
}

void square_open_door(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_OPEN);
}

void square_smash_door(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_BROKEN);
}

void square_destroy_trap(struct chunk *c, int y, int x) {
	square_remove_trap(c, y, x, FALSE, -1);
}

void square_lock_door(struct chunk *c, int y, int x, int power) {
	square_set_feat(c, y, x, FEAT_DOOR_HEAD + power);
}

bool square_hasgoldvein(struct chunk *c, int y, int x) {
	return tf_has(f_info[c->feat[y][x]].flags, TF_GOLD);
}

void square_tunnel_wall(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_destroy_wall(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_close_door(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_DOOR_HEAD);
}

bool square_isbrokendoor(struct chunk *c, int y, int x) {
	int feat = c->feat[y][x];
    return (tf_has(f_info[feat].flags, TF_DOOR_ANY) &&
			tf_has(f_info[feat].flags, TF_PASSABLE) &&
			!tf_has(f_info[feat].flags, TF_CLOSABLE));
}

void square_add_trap(struct chunk *c, int y, int x) {
	place_trap(c, y, x, -1, c->depth);
}

bool square_iswarded(struct chunk *c, int y, int x) {
	struct trap_kind *rune = lookup_trap("glyph of warding");
	return square_trap_specific(c, y, x, rune->tidx);
}

void square_add_ward(struct chunk *c, int y, int x) {
	struct trap_kind *rune = lookup_trap("glyph of warding");
	place_trap(c, y, x, rune->tidx, 0);
}

void square_remove_ward(struct chunk *c, int y, int x) {
	struct trap_kind *rune = lookup_trap("glyph of warding");
	assert(square_iswarded(c, y, x));
	square_remove_trap_kind(c, y, x, TRUE, rune->tidx);
}

bool square_canward(struct chunk *c, int y, int x) {
	return square_isfloor(c, y, x);
}

bool square_seemslikewall(struct chunk *c, int y, int x) {
	return tf_has(f_info[c->feat[y][x]].flags, TF_ROCK);
}

bool square_isinteresting(struct chunk *c, int y, int x) {
	int f = c->feat[y][x];
	return tf_has(f_info[f].flags, TF_INTERESTING);
}

void square_add_stairs(struct chunk *c, int y, int x, int depth) {
	int down = randint0(100) < 50;
	if (depth == 0)
		down = 1;
	else if (is_quest(depth) || depth >= MAX_DEPTH - 1)
		down = 0;
	square_set_feat(c, y, x, down ? FEAT_MORE : FEAT_LESS);
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

bool square_hassecretvein(struct chunk *c, int y, int x) {
	return (tf_has(f_info[c->feat[y][x]].flags, TF_GOLD) &&
			!tf_has(f_info[c->feat[y][x]].flags, TF_INTERESTING));
}

bool square_noticeable(struct chunk *c, int y, int x) {
	return tf_has(f_info[c->feat[y][x]].flags, TF_INTERESTING);
}

const char *square_apparent_name(struct chunk *c, struct player *p, int y, int x) {
	int f = f_info[c->feat[y][x]].mimic;

	if (!square_ismark(c, y, x) && !player_can_see_bold(y, x))
		f = FEAT_NONE;

	if (f == FEAT_NONE)
		return "unknown_grid";

	return f_info[f].name;
}

void square_unlock_door(struct chunk *c, int y, int x) {
	assert(square_islockeddoor(c, y, x));
	square_set_feat(c, y, x, FEAT_DOOR_HEAD);
}

void square_destroy_door(struct chunk *c, int y, int x) {
	assert(square_isdoor(c, y, x));
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_destroy_rubble(struct chunk *c, int y, int x) {
	assert(square_isrubble(c, y, x));
	square_set_feat(c, y, x, FEAT_FLOOR);
}

void square_add_door(struct chunk *c, int y, int x, bool closed) {
	square_set_feat(c, y, x, closed ? FEAT_DOOR_HEAD : FEAT_OPEN);
}

void square_force_floor(struct chunk *c, int y, int x) {
	square_set_feat(c, y, x, FEAT_FLOOR);
}

/*
 * Return the number of doors/traps around (or under) the character.
 */
int count_feats(int *y, int *x, bool (*test)(struct chunk *c, int y, int x), bool under)
{
	int d;
	int xx, yy;
	int count = 0; /* Count how many matches */

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++)
	{
		/* if not searching under player continue */
		if ((d == 8) && !under) continue;

		/* Extract adjacent (legal) location */
		yy = player->py + ddy_ddd[d];
		xx = player->px + ddx_ddd[d];

		/* Paranoia */
		if (!square_in_bounds_fully(cave, yy, xx)) continue;

		/* Must have knowledge */
		if (!square_ismark(cave, yy, xx)) continue;

		/* Not looking for this feature */
		if (!((*test)(cave, yy, xx))) continue;

		/* Count it */
		++count;

		/* Remember the location of the last door found */
		*y = yy;
		*x = xx;
	}

	/* All done */
	return count;
}
