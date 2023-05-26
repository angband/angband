/**
 * \file gen-util.c
 * \brief Dungeon generation utilities
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Erik Osheim, Nick McConnell
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
 *
 * This file contains various utility functions for dungeon generation - mostly
 * for finding appropriate grids for some purposes, or placing things. 
 */

#include "angband.h"
#include "cave.h"
#include "datafile.h"
#include "game-event.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-util.h"
#include "trap.h"
#include "z-queue.h"
#include "z-type.h"

/**
 * Accept values for y and x (considered as the endpoints of lines) between
 * 0 and 40, and return an angle in degrees (divided by two).  -LM-
 *
 * This table's input and output need some processing:
 *
 * Because this table gives degrees for a whole circle, up to radius 20, its
 * origin is at (x,y) = (20, 20).  Therefore, the input code needs to find
 * the origin grid (where the lines being compared come from), and then map
 * it to table grid 20,20.  Do not, however, actually try to compare the
 * angle of a line that begins and ends at the origin with any other line -
 * it is impossible mathematically, and the table will return the value "255".
 *
 * The output of this table also needs to be massaged, in order to avoid the
 * discontinuity at 0/180 degrees.  This can be done by:
 *   rotate = 90 - first value
 *   this rotates the first input to the 90 degree line)
 *   tmp = ABS(second value + rotate) % 180
 *   diff = ABS(90 - tmp) = the angular difference (divided by two) between
 *   the first and second values.
 *
 * Note that grids diagonal to the origin have unique angles.
 */
uint8_t get_angle_to_grid[41][41] =
{
  {  68,  67,  66,  65,  64,  63,  62,  62,  60,  59,  58,  57,  56,  55,  53,  52,  51,  49,  48,  46,  45,  44,  42,  41,  39,  38,  37,  35,  34,  33,  32,  31,  30,  28,  28,  27,  26,  25,  24,  24,  23 },
  {  69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,  56,  55,  54,  52,  51,  49,  48,  47,  45,  43,  42,  41,  39,  38,  36,  35,  34,  32,  31,  30,  29,  28,  27,  26,  25,  24,  24,  23,  22 },
  {  69,  69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  58,  57,  56,  54,  53,  51,  50,  48,  47,  45,  43,  42,  40,  39,  37,  36,  34,  33,  32,  30,  29,  28,  27,  26,  25,  24,  24,  23,  22,  21 },
  {  70,  69,  69,  68,  67,  66,  65,  64,  63,  61,  60,  59,  58,  56,  55,  53,  52,  50,  48,  47,  45,  43,  42,  40,  38,  37,  35,  34,  32,  31,  30,  29,  27,  26,  25,  24,  24,  23,  22,  21,  20 },
  {  71,  70,  69,  69,  68,  67,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  49,  47,  45,  43,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  23,  22,  21,  20,  19 },
  {  72,  71,  70,  69,  69,  68,  67,  65,  64,  63,  62,  60,  59,  58,  56,  54,  52,  51,  49,  47,  45,  43,  41,  39,  38,  36,  34,  32,  31,  30,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18 },
  {  73,  72,  71,  70,  69,  69,  68,  66,  65,  64,  63,  61,  60,  58,  57,  55,  53,  51,  49,  47,  45,  43,  41,  39,  37,  35,  33,  32,  30,  29,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,  17 },
  {  73,  73,  72,  71,  70,  70,  69,  68,  66,  65,  64,  62,  61,  59,  57,  56,  54,  51,  49,  47,  45,  43,  41,  39,  36,  34,  33,  31,  29,  28,  26,  25,  24,  23,  21,  20,  20,  19,  18,  17,  17 },
  {  75,  74,  73,  72,  72,  71,  70,  69,  68,  66,  65,  63,  62,  60,  58,  56,  54,  52,  50,  47,  45,  43,  40,  38,  36,  34,  32,  30,  28,  27,  25,  24,  23,  21,  20,  19,  18,  18,  17,  16,  15 },
  {  76,  75,  74,  74,  73,  72,  71,  70,  69,  68,  66,  65,  63,  61,  59,  57,  55,  53,  50,  48,  45,  42,  40,  37,  35,  33,  31,  29,  27,  25,  24,  23,  21,  20,  19,  18,  17,  16,  16,  15,  14 },
  {  77,  76,  75,  75,  74,  73,  72,  71,  70,  69,  68,  66,  64,  62,  60,  58,  56,  53,  51,  48,  45,  42,  39,  37,  34,  32,  30,  28,  26,  24,  23,  21,  20,  19,  18,  17,  16,  15,  15,  14,  13 },
  {  78,  77,  77,  76,  75,  75,  74,  73,  72,  70,  69,  68,  66,  64,  62,  60,  57,  54,  51,  48,  45,  42,  39,  36,  33,  30,  28,  26,  24,  23,  21,  20,  18,  17,  16,  15,  15,  14,  13,  13,  12 },
  {  79,  79,  78,  77,  77,  76,  75,  74,  73,  72,  71,  69,  68,  66,  63,  61,  58,  55,  52,  49,  45,  41,  38,  35,  32,  29,  27,  24,  23,  21,  19,  18,  17,  16,  15,  14,  13,  13,  12,  11,  11 },
  {  80,  80,  79,  79,  78,  77,  77,  76,  75,  74,  73,  71,  69,  68,  65,  63,  60,  57,  53,  49,  45,  41,  37,  33,  30,  27,  25,  23,  21,  19,  17,  16,  15,  14,  13,  13,  12,  11,  11,  10,  10 },
  {  82,  81,  81,  80,  80,  79,  78,  78,  77,  76,  75,  73,  72,  70,  68,  65,  62,  58,  54,  50,  45,  40,  36,  32,  28,  25,  23,  20,  18,  17,  15,  14,  13,  12,  12,  11,  10,  10,   9,   9,   8 },
  {  83,  83,  82,  82,  81,  81,  80,  79,  79,  78,  77,  75,  74,  72,  70,  68,  64,  60,  56,  51,  45,  39,  34,  30,  26,  23,  20,  18,  16,  15,  13,  12,  11,  11,  10,   9,   9,   8,   8,   7,   7 },
  {  84,  84,  84,  83,  83,  83,  82,  81,  81,  80,  79,  78,  77,  75,  73,  71,  68,  63,  58,  52,  45,  38,  32,  27,  23,  19,  17,  15,  13,  12,  11,  10,   9,   9,   8,   7,   7,   7,   6,   6,   6 },
  {  86,  86,  85,  85,  85,  84,  84,  84,  83,  82,  82,  81,  80,  78,  77,  75,  72,  68,  62,  54,  45,  36,  28,  23,  18,  15,  13,  12,  10,   9,   8,   8,   7,   6,   6,   6,   5,   5,   5,   4,   4 },
  {  87,  87,  87,  87,  86,  86,  86,  86,  85,  85,  84,  84,  83,  82,  81,  79,  77,  73,  68,  58,  45,  32,  23,  17,  13,  11,   9,   8,   7,   6,   6,   5,   5,   4,   4,   4,   4,   3,   3,   3,   3 },
  {  89,  88,  88,  88,  88,  88,  88,  88,  88,  87,  87,  87,  86,  86,  85,  84,  83,  81,  77,  68,  45,  23,  13,   9,   7,   6,   5,   4,   4,   3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   1 },
  {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90, 255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
  {  91,  92,  92,  92,  92,  92,  92,  92,  92,  93,  93,  93,  94,  94,  95,  96,  97,  99, 103, 113, 135, 158, 167, 171, 173, 174, 175, 176, 176, 177, 177, 177, 178, 178, 178, 178, 178, 178, 178, 178, 179 },
  {  93,  93,  93,  93,  94,  94,  94,  94,  95,  95,  96,  96,  97,  98,  99, 101, 103, 107, 113, 122, 135, 148, 158, 163, 167, 169, 171, 172, 173, 174, 174, 175, 175, 176, 176, 176, 176, 177, 177, 177, 177 },
  {  94,  94,  95,  95,  95,  96,  96,  96,  97,  98,  98,  99, 100, 102, 103, 105, 108, 113, 118, 126, 135, 144, 152, 158, 162, 165, 167, 168, 170, 171, 172, 172, 173, 174, 174, 174, 175, 175, 175, 176, 176 },
  {  96,  96,  96,  97,  97,  97,  98,  99,  99, 100, 101, 102, 103, 105, 107, 109, 113, 117, 122, 128, 135, 142, 148, 153, 158, 161, 163, 165, 167, 168, 169, 170, 171, 171, 172, 173, 173, 173, 174, 174, 174 },
  {  97,  97,  98,  98,  99,  99, 100, 101, 101, 102, 103, 105, 106, 108, 110, 113, 116, 120, 124, 129, 135, 141, 146, 150, 154, 158, 160, 162, 164, 165, 167, 168, 169, 169, 170, 171, 171, 172, 172, 173, 173 },
  {  98,  99,  99, 100, 100, 101, 102, 102, 103, 104, 105, 107, 108, 110, 113, 115, 118, 122, 126, 130, 135, 140, 144, 148, 152, 155, 158, 160, 162, 163, 165, 166, 167, 168, 168, 169, 170, 170, 171, 171, 172 },
  { 100, 100, 101, 101, 102, 103, 103, 104, 105, 106, 107, 109, 111, 113, 115, 117, 120, 123, 127, 131, 135, 139, 143, 147, 150, 153, 155, 158, 159, 161, 163, 164, 165, 166, 167, 167, 168, 169, 169, 170, 170 },
  { 101, 101, 102, 103, 103, 104, 105, 106, 107, 108, 109, 111, 113, 114, 117, 119, 122, 125, 128, 131, 135, 139, 142, 145, 148, 151, 153, 156, 158, 159, 161, 162, 163, 164, 165, 166, 167, 167, 168, 169, 169 },
  { 102, 103, 103, 104, 105, 105, 106, 107, 108, 110, 111, 113, 114, 116, 118, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147, 150, 152, 154, 156, 158, 159, 160, 162, 163, 164, 165, 165, 166, 167, 167, 168 },
  { 103, 104, 105, 105, 106, 107, 108, 109, 110, 111, 113, 114, 116, 118, 120, 122, 124, 127, 129, 132, 135, 138, 141, 143, 146, 148, 150, 152, 154, 156, 158, 159, 160, 161, 162, 163, 164, 165, 165, 166, 167 },
  { 104, 105, 106, 106, 107, 108, 109, 110, 111, 113, 114, 115, 117, 119, 121, 123, 125, 127, 130, 132, 135, 138, 140, 143, 145, 147, 149, 151, 153, 155, 156, 158, 159, 160, 161, 162, 163, 164, 164, 165, 166 },
  { 105, 106, 107, 108, 108, 109, 110, 111, 113, 114, 115, 117, 118, 120, 122, 124, 126, 128, 130, 133, 135, 137, 140, 142, 144, 146, 148, 150, 152, 153, 155, 156, 158, 159, 160, 161, 162, 162, 163, 164, 165 },
  { 107, 107, 108, 109, 110, 110, 111, 113, 114, 115, 116, 118, 119, 121, 123, 124, 126, 129, 131, 133, 135, 137, 139, 141, 144, 146, 147, 149, 151, 152, 154, 155, 156, 158, 159, 160, 160, 161, 162, 163, 163 },
  { 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 119, 120, 122, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 148, 150, 151, 153, 154, 155, 156, 158, 159, 159, 160, 161, 162, 163 },
  { 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 120, 121, 122, 124, 126, 128, 129, 131, 133, 135, 137, 139, 141, 142, 144, 146, 148, 149, 150, 152, 153, 154, 155, 157, 158, 159, 159, 160, 161, 162 },
  { 109, 110, 111, 112, 113, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 159, 159, 160, 161 },
  { 110, 111, 112, 113, 114, 114, 115, 116, 117, 119, 120, 121, 122, 124, 125, 127, 128, 130, 132, 133, 135, 137, 138, 140, 142, 143, 145, 146, 148, 149, 150, 151, 153, 154, 155, 156, 157, 158, 159, 159, 160 },
  { 111, 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 122, 123, 124, 126, 127, 129, 130, 132, 133, 135, 137, 138, 140, 141, 143, 144, 146, 147, 148, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 159 },
  { 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 121, 122, 124, 125, 126, 128, 129, 131, 132, 133, 135, 137, 138, 139, 141, 142, 144, 145, 146, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159 },
  { 113, 114, 114, 115, 116, 117, 118, 118, 120, 121, 122, 123, 124, 125, 127, 128, 129, 131, 132, 134, 135, 136, 138, 139, 141, 142, 143, 145, 146, 147, 148, 149, 150, 152, 152, 153, 154, 155, 156, 157, 158 }
};


/**
 * Used to convert grid into an array index (i) in a chunk of width w.
 * \param grid location
 * \param w area width
 * \return index
 */
int grid_to_i(struct loc grid, int w)
{
	return grid.y * w + grid.x;
}

/**
 * Used to convert an array index (i) into grid in a chunk of width w.
 * \param i grid index
 * \param w area width
 * \param grid location
 */
void i_to_grid(int i, int w, struct loc *grid)
{
	grid->y = i / w;
	grid->x = i % w;
}

/**
 * Shuffle an array using Knuth's shuffle.
 * \param arr array
 * \param n number of shuffle moves
 */
void shuffle(int *arr, int n)
{
	int i, j, k;
	for (i = 0; i < n; i++) {
		j = randint0(n - i) + i;
		k = arr[j];
		arr[j] = arr[i];
		arr[i] = k;
	}
}


/**
 * Set up to locate a square in a rectangular region of a chunk.
 *
 * \param top_left is the upper left corner of the rectangle to be searched.
 * \param bottom_right is the lower right corner of the rectangle to be
 * searched.
 * \return the state for the search.  When no longer needed, the returned
 * value should be passed to mem_free().
 */
int *cave_find_init(struct loc top_left, struct loc bottom_right)
{
	struct loc diff = loc_diff(bottom_right, top_left);
	int n = (diff.y < 0 || diff.x < 0) ? 0 : (diff.x + 1) * (diff.y + 1);
	int *state = mem_alloc((5 + n) * sizeof(*state));
	int i;

	state[0] = n;
	state[1] = diff.x + 1;
	state[2] = top_left.x;
	state[3] = top_left.y;
	/* The next to search is the first one. */
	state[4] = 0;
	/*
	 * Set up for left to right, top to bottom, search; will randomize in
	 * cave_find_get_grid().
	 */
	for (i = 5; i < 5 + n; ++i) {
		state[i] = i - 5;
	}
	return state;
}


/*
 * Reset a search created by cave_find_init() to start again from fresh.
 *
 * \param state is the search state created by cave_find_init().
 */
void cave_find_reset(int *state)
{
	/* The next to search is the first one. */
	state[4] = 0;
}

/**
 * Get the next grid for a search created by cave_find_init().
 *
 * \param grid is dereferenced and set to the grid to check.
 * \param state is the search state created by cave_find_init().
 * \return true if grid was dereferenced and set to the next grid to be
 * searched; otherwise return false to indicate that there are no more grids
 * available.
 */
bool cave_find_get_grid(struct loc *grid, int *state)
{
	int j, k;

	assert(state[4] >= 0);
	if (state[4] >= state[0]) return false;

	/*
	 * Choose one of the remaining ones at random.  Swap it with the one
	 * that's next in order.
	 */
	j = randint0(state[0] - state[4]) + state[4];
	k = state[5 + j];
	state[5 + j] = state[5 + state[4]];
	state[5 + state[4]] = k;

	grid->y = (k / state[1]) + state[3];
	grid->x = (k % state[1]) + state[2];

	/*
	 * Increment so a future call to cave_find_get_grid() will get the
	 * next one.
	 */
	++state[4];
	return true;
}


/**
 * Locate a square in a rectangle which satisfies the given predicate.
 *
 * \param c current chunk
 * \param grid found grid
 * \param top_left top left grid of rectangle
 * \param bottom_right bottom right grid of rectangle
 * \param pred square_predicate specifying what we're looking for
 * \return success
 */
bool cave_find_in_range(struct chunk *c, struct loc *grid,
		struct loc top_left, struct loc bottom_right,
		square_predicate pred)
{
	int *state = cave_find_init(top_left, bottom_right);
	bool found = false;

	while (!found && cave_find_get_grid(grid, state)) {
		found = pred(c, *grid);
	}
	mem_free(state);
	return found;
}


/**
 * Locate a square in the dungeon which satisfies the given predicate.
 * \param c current chunk
 * \param grid found grid
 * \param pred square_predicate specifying what we're looking for
 * \return success
 */
bool cave_find(struct chunk *c, struct loc *grid, square_predicate pred)
{
	struct loc top_left = loc(0, 0);
	struct loc bottom_right = loc(c->width - 1, c->height - 1);
	return cave_find_in_range(c, grid, top_left, bottom_right, pred);
}


/**
 * Locate an empty square for 0 <= y < ymax, 0 <= x < xmax.
 * \param c current chunk
 * \param grid found grid
 * \return success
 */
bool find_empty(struct chunk *c, struct loc *grid)
{
	return cave_find(c, grid, square_isempty);
}


/**
 * Locate an empty square in a given rectangle.
 * \param c current chunk
 * \param grid found grid
 * \param top_left top left grid of rectangle
 * \param bottom_right bottom right grid of rectangle
 * \return success
 */
bool find_empty_range(struct chunk *c, struct loc *grid, struct loc top_left,
	struct loc bottom_right)
{
	return cave_find_in_range(c, grid, top_left, bottom_right,
		square_isempty);
}


/**
 * Locate a grid within +/- yd, xd of a centre.
 * \param c current chunk
 * \param grid found grid
 * \param centre starting grid
 * \param yd y-range
 * \param xd x-range
 * \return success
 */
bool find_nearby_grid(struct chunk *c, struct loc *grid, struct loc centre,
	int yd, int xd)
{
	struct loc top_left = loc(centre.x - xd, centre.y - yd);
	struct loc bottom_right = loc(centre.x + xd, centre.y + yd);
	return cave_find_in_range(c, grid, top_left, bottom_right,
		square_in_bounds_fully);
}


/**
 * Given two points, pick a valid cardinal direction from one to the other.
 * \param offset found offset direction from grid 1 to grid2
 * \param grid1 starting grid
 * \param grid2 target grid
 */
void correct_dir(struct loc *offset, struct loc grid1, struct loc grid2)
{
	/* Extract horizontal and vertical directions */
	offset->x = CMP(grid2.x, grid1.x);
	offset->y = CMP(grid2.y, grid1.y);

	/* If we only have one direction to go, then we're done */
	if (!offset->x || !offset->y) return;

	/* If we need to go diagonally, then choose a random direction */
	if (randint0(100) < 50)
		offset->y = 0;
	else
		offset->x = 0;
}


/**
 * Pick a random cardinal direction.
 * \param offset direction offset
 */
void rand_dir(struct loc *offset)
{
	/* Pick a random direction and extract the dy/dx components */
	int i = randint0(4);
	*offset = ddgrid_ddd[i];
}


/**
 * Determine whether the given coordinate is a valid starting location.
 * \param c current chunk
 * \param y co-ordinates
 * \param x co-ordinates
 * \return success
 */
static bool find_start(struct chunk *c, struct loc *grid)
{
	int *state = cave_find_init(loc(1, 1),
		loc(c->width - 2, c->height - 2));
	bool found = false;

	/* Find the best possible place */
	while (!found && cave_find_get_grid(grid, state)) {
		found = square_suits_stairs_well(c, *grid);
	}

	if (!found) {
		cave_find_reset(state);
		while (!found && cave_find_get_grid(grid, state)) {
			found = square_suits_stairs_ok(c, *grid);
		}
	}

	if (!found) {
		int walls = 6;

		/* Gradually reduce number of walls if having trouble */
		while (!found && walls >= 0) {
			cave_find_reset(state);
			while (!found && cave_find_get_grid(grid, state)) {
				int total_walls;

				if (!square_isempty(c, *grid)
						|| square_isvault(c, *grid)
						|| square_isno_stairs(c, *grid)) {
					continue;
				}
				total_walls =
					square_num_walls_adjacent(c, *grid) +
					square_num_walls_diagonal(c, *grid);
				found = (total_walls == walls);
			}
			walls--;
		}
	}

	mem_free(state);

	return found;
}


/**
 * Place the player at a random starting location.
 * \param c current chunk
 * \param p the player
 * \return true on success or false on failure
 */
bool new_player_spot(struct chunk *c, struct player *p)
{
	struct loc grid;

	/* Try to find a good place to put the player */
	if (OPT(p, birth_levels_persist) &&
			square_in_bounds_fully(c, p->grid) &&
			square_isstairs(c, p->grid)) {
		grid = p->grid;
	} else if (!find_start(c, &grid)) {
		msg("Failed to place player; please report.  Restarting generation.");
		dump_level_simple(NULL, "Player Placement Failure", c);
		return false;
	}

	/* Create stairs the player came down if allowed and necessary */
	if (!OPT(p, birth_connect_stairs))
		;
	else if (p->upkeep->create_down_stair)
		square_set_feat(c, grid, FEAT_MORE);
	else if (p->upkeep->create_up_stair)
		square_set_feat(c, grid, FEAT_LESS);

	player_place(c, p, grid);
	return true;
}


/**
 * Place rubble at a given location.
 * \param c current chunk
 * \param grid location
 */
static void place_rubble(struct chunk *c, struct loc grid)
{
	square_set_feat(c, grid, one_in_(2) ? FEAT_RUBBLE : FEAT_PASS_RUBBLE);
}


/**
 * Place stairs (of the requested type 'feat' if allowed) at a given location.
 *
 * \param c current chunk
 * \param grid location
 * \param quest is whether or not this is a quest level.
 * \param feat stair terrain type
 *
 * All stairs from town go down. All stairs on an unfinished quest level go up.
 */
static void place_stairs(struct chunk *c, struct loc grid, bool quest, int feat)
{
	if (!c->depth) {
		square_set_feat(c, grid, FEAT_MORE);
	} else if (quest || c->depth >= z_info->max_depth - 1) {
		square_set_feat(c, grid, FEAT_LESS);
	} else {
		square_set_feat(c, grid, feat);
	}
}


/**
 * Place random stairs at a given location.
 *
 * \param c current chunk
 * \param grid location
 * \param quest is whether or not this is a quest level.
 */
void place_random_stairs(struct chunk *c, struct loc grid, bool quest)
{
	int feat = randint0(100) < 50 ? FEAT_LESS : FEAT_MORE;
	if (square_canputitem(c, grid))
		place_stairs(c, grid, quest, feat);
}


/**
 * Place a random object at a given location.
 * \param c current chunk
 * \param grid location
 * \param level generation depth
 * \param good is it a good object?
 * \param great is it a great object?
 * \param origin item origin
 * \param tval specified tval, if any
 */
void place_object(struct chunk *c, struct loc grid, int level, bool good,
		bool great, uint8_t origin, int tval)
{
	int32_t rating = 0;
	struct object *new_obj;
	bool dummy = true;

	if (!square_in_bounds(c, grid)) return;
	if (!square_canputitem(c, grid)) return;

	/* Make an appropriate object */
	new_obj = make_object(c, level, good, great, false, &rating, tval);
	if (!new_obj) return;
	new_obj->origin = origin;
	new_obj->origin_depth = convert_depth_to_origin(c->depth);

	/* Give it to the floor */
	if (!floor_carry(c, grid, new_obj, &dummy)) {
		if (new_obj->artifact) {
			mark_artifact_created(new_obj->artifact, false);
		}
		object_delete(c, NULL, &new_obj);
		return;
	} else {
		uint32_t sqrating;

		list_object(c, new_obj);
		if (new_obj->artifact) {
			c->good_item = true;
		}
		/* Avoid overflows */
		if (rating > 2500000) {
			rating = 2500000;
		} else if (rating < -2500000) {
			rating = -2500000;
		}
		sqrating = (rating / 100) * (rating / 100);
		if (c->obj_rating < UINT32_MAX - sqrating) {
			c->obj_rating += sqrating;
		} else {
			c->obj_rating = UINT32_MAX;
		}
	}
}


/**
 * Place a random amount of gold at a given location.
 * \param c current chunk
 * \param grid location
 * \param level generation depth
 * \param origin item origin
 */
void place_gold(struct chunk *c, struct loc grid, int level, uint8_t origin)
{
	struct object *money = NULL;
	bool dummy = true;

	if (!square_in_bounds(c, grid)) return;
	if (!square_canputitem(c, grid)) return;

	money = make_gold(level, "any");
	money->origin = origin;
	money->origin_depth = convert_depth_to_origin(level);

	if (!floor_carry(c, grid, money, &dummy)) {
		object_delete(c, NULL, &money);
	} else {
		list_object(c, money);
	}
}


/**
 * Place a secret door at a given location.
 * \param c current chunk
 * \param grid location
 */
void place_secret_door(struct chunk *c, struct loc grid)
{
	square_set_feat(c, grid, FEAT_SECRET);
}


/**
 * Place a closed (and possibly locked) door at a given location.
 * \param c current chunk
 * \param grid location
 */
void place_closed_door(struct chunk *c, struct loc grid)
{
	square_set_feat(c, grid, FEAT_CLOSED);
	if (one_in_(4))
		square_set_door_lock(c, grid, randint1(7));
}


/**
 * Place a random door at a given location.
 * \param c current chunk
 * \param grid location
 *
 * The door generated could be closed (and possibly locked), open, or broken.
 */
void place_random_door(struct chunk *c, struct loc grid)
{
	int tmp = randint0(100);

	if (tmp < 30)
		square_set_feat(c, grid, FEAT_OPEN);
	else if (tmp < 40)
		square_set_feat(c, grid, FEAT_BROKEN);
	else
		place_closed_door(c, grid);
}

/**
 * Place some staircases near walls.
 * \param c the current chunk
 * \param feat the stair terrain type
 * \param num number of staircases to place
 * \param minsep If greater than zero, the stairs must be more than minsep
 * grids in x or y from other staircases.  sepany controls which types of
 * staircases are considered when enforcing the separation constraint.
 * \param sepany If true, the minimum separation contraint applies to any
 * type of staircase.  Otherwise, the minimum separation contraint only applies
 * to staircases of the same type.
 * \param avoid_list If not NULL and minsep is greater than zero, also avoid
 * the locations in avoid_list which have staircases of the opposite type.
 * \param quest is whether or not this is a quest level.
 */
void alloc_stairs(struct chunk *c, int feat, int num, int minsep, bool sepany,
		const struct connector *avoid_list, bool quest)
{
	int i, navalloc, nav, walls;
	struct loc *av;
	int *state;

	nav = 0;
	if (minsep > 0) {
		/*
		 * Get the locations of the stairs already there that'll have
		 * to be avoided.
		 */
		square_predicate tester = (sepany) ? square_isstairs :
			((feat == FEAT_MORE) ?
			square_isdownstairs : square_isupstairs);
		struct loc grid;
		const struct connector *avc;

		navalloc = 8;
		av = mem_alloc(navalloc * sizeof(*av));
		for (grid.y = 0; grid.y < c->height; ++grid.y) {
			for (grid.x = 0; grid.x < c->width; ++grid.x) {
				if ((*tester)(c, grid)) {
					assert(nav >= 0 && nav <= navalloc);
					if (nav == navalloc) {
						navalloc += navalloc;
						av = mem_realloc(av, navalloc *
							sizeof(*av));
					}
					av[nav++] = grid;
				}
			}
		}

		/* Also add the locations that were passed in. */
		for (avc = avoid_list; avc; avc = avc->next) {
			if (avc->feat != feat) {
				assert(nav >= 0 && nav <= navalloc);
				if (nav == navalloc) {
					navalloc += navalloc;
					av = mem_realloc(av, navalloc *
						sizeof(*av));
				}
				av[nav++] = avc->grid;
			}
		}
	} else {
		navalloc = 0;
		av = NULL;
	}

	/* Place "num" stairs */
	state = cave_find_init(loc(1, 1), loc(c->width - 2, c->height - 2));
	i = 0;
	walls = 3;
	while (i < num && walls >= 0) {
		struct loc grid;

		/* Try to find; then decrease "walls" */
		while (i < num && cave_find_get_grid(&grid, state)) {
			if (!square_isempty(c, grid)
					|| square_num_walls_adjacent(c, grid) != walls) {
				continue;
			}
			if (minsep > 0) {
				int k;

				/* Check against the stairs to be avoided. */
				for (k = 0; k < nav; ++k) {
					if (ABS(grid.y - av[k].y) <= minsep
							&& ABS(grid.x
							- av[k].x) <= minsep) {
						break;
					}
				}
				if (k < nav) {
					continue;
				}
				/* Add this to the avoidance list. */
				assert(nav >= 0 && nav <= navalloc);
				if (nav == navalloc) {
					navalloc += navalloc;
					av = mem_realloc(av, navalloc *
						sizeof(*av));
				}
				av[nav++] = grid;
			}

			place_stairs(c, grid, quest, feat);
			assert(square_isstairs(c, grid));
			++i;
		}

		/* Require fewer walls */
		if (i < num) {
			--walls;
			cave_find_reset(state);
		}
	}

	mem_free(state);
	mem_free(av);
}


/**
 * Allocates 'num' random entities in the dungeon.
 * \param c the current chunk
 * \param set where the entity is placed (corridor, room or either)
 * \param typ what is placed (rubble, trap, gold, item)
 * \param num number to place
 * \param depth generation depth
 * \param origin item origin (if appropriate)
 *
 * See alloc_object() for more information.
 */
void alloc_objects(struct chunk *c, int set, int typ, int num, int depth,
		uint8_t origin)
{
	int k, l = 0;
	for (k = 0; k < num; k++) {
		bool ok = alloc_object(c, set, typ, depth, origin);
		if (!ok) l++;
	}
}


/**
 * Allocates a single random object in the dungeon.
 * \param c the current chunk
 * \param set where the entity is placed (corridor, room or either)
 * \param typ what is placed (rubble, trap, gold, item)
 * \param depth generation depth
 * \param origin item origin (if appropriate)
 *
 * 'set' controls where the object is placed (corridor, room, either).
 * 'typ' conrols the kind of object (rubble, trap, gold, item).
 */
bool alloc_object(struct chunk *c, int set, int typ, int depth, uint8_t origin)
{
	bool placed = false;
	int *state = cave_find_init(loc(1, 1),
		loc(c->width - 2, c->height - 2));
	struct loc grid;

	while (!placed && cave_find_get_grid(&grid, state)) {
		/*
		 * If we're ok with a corridor and we're in one, we're done.
		 * If we are ok with a room and we're in one, we're done
		 */
		bool matched = ((set & SET_CORR) && !square_isroom(c, grid))
			|| ((set & SET_ROOM) && square_isroom(c, grid));
		if (square_isempty(c, grid) && matched) {
			/* Place something */
			switch (typ) {
			case TYP_RUBBLE:
				place_rubble(c, grid);
				break;
			case TYP_TRAP:
				place_trap(c, grid, -1, depth);
				break;
			case TYP_GOLD:
				place_gold(c, grid, depth, origin);
				break;
			case TYP_OBJECT:
				place_object(c, grid, depth, false, false, origin, 0);
				break;
			case TYP_GOOD:
				place_object(c, grid, depth, true, false, origin, 0);
				break;
			case TYP_GREAT:
				place_object(c, grid, depth, true, true, origin, 0);
				break;
			}
			placed = true;
		}
	}

	mem_free(state);
	return placed;
}

/**
 * Create up to 'num' objects near the given coordinates in a vault.
 * \param c the current chunk
 * \param grid location
 * \param depth generation depth
 * \param num number of objects
 */
void vault_objects(struct chunk *c, struct loc grid, int depth, int num)
{
	int i;

	/* Attempt to place 'num' objects */
	for (; num > 0; --num) {
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i) {
			struct loc near;

			/* Pick a random location */
			if (!find_nearby_grid(c, &near, grid, 2, 3)) assert(0);

			/* Require "clean" floor space */
			if (!square_canputitem(c, near)) continue;

			/* Place an item or gold */
			if (randint0(100) < 75)
				place_object(c, near, depth, false, false, ORIGIN_SPECIAL, 0);
			else
				place_gold(c, near, depth, ORIGIN_VAULT);

			/* Placement accomplished */
			break;
		}
	}
}

/**
 * Place a trap near (x, y), with a given displacement.
 * \param c the current chunk
 * \param grid location to place the trap near
 * \param yd how far afield to look for a place
 * \param xd how far afield to look for a place
 */
static void vault_trap_aux(struct chunk *c, struct loc grid, int yd, int xd)
{
	int tries;

	/* Find a nearby empty grid and place a trap */
	for (tries = 0; tries <= 5; tries++) {
		struct loc near;
		if (!find_nearby_grid(c, &near, grid, yd, xd)) assert(0);
		if (!square_isempty(c, near)) continue;

		square_add_trap(c, near);
		break;
	}
}


/**
 * Place 'num' traps near a location, with a given displacement.
 * \param c the current chunk
 * \param grid location to place the trap near
 * \param yd how far afield to look for a place
 * \param xd how far afield to look for a place
 * \param num number of traps to place
 */
void vault_traps(struct chunk *c, struct loc grid, int yd, int xd, int num)
{
	int i;
	for (i = 0; i < num; i++)
		vault_trap_aux(c, grid, yd, xd);
}


/**
 * Place 'num' sleeping monsters near the location.
 * \param c the current chunk
 * \param grid location to place the monsters near
 * \param depth generation depth
 * \param num number of monsters to place
 */
void vault_monsters(struct chunk *c, struct loc grid, int depth, int num)
{
	int k, i;

	/* If the starting location is illegal, don't even start */
	if (!square_in_bounds(c, grid)) return;

	/* Try to summon "num" monsters "near" the given location */
	for (k = 0; k < num; k++) {
		/* Try nine locations */
		for (i = 0; i < 9; i++) {
			struct loc near;

			/* Pick a nearby empty location. */
			if (scatter_ext(c, &near, 1, grid, 1, true,
					square_isempty) == 0) continue;

			/* Place the monster (allow groups) */
			pick_and_place_monster(c, near, depth, true, true,
				ORIGIN_DROP_SPECIAL);

			break;
		}
	}
}

/**
 * Mark artifacts in a failed chunk as not created
 */
void uncreate_artifacts(struct chunk *c)
{
	int y, x;

	/* Also mark created artifacts as not created ... */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			struct loc grid = loc(x, y);
			struct object *obj = square_object(c, grid);
			while (obj) {
				if (obj->artifact) {
					mark_artifact_created(obj->artifact, false);
				}
				obj = obj->next;
			}
		}
	}
}

/**
 * Dump the given level for post-mortem analysis; handle all I/O.
 * \param basefilename Is the base name (no directory or extension) for the
 * file to use.  If NULL, "dumpedlevel" will be used.
 * \param title Is the label to use within the file.  If NULL, "Dumped Level"
 * will be used.
 * \param c Is the chunk to dump.
 */
void dump_level_simple(const char *basefilename, const char *title,
	struct chunk *c)
{
	char path[1024];
	ang_file *fo;

	path_build(path, sizeof(path), ANGBAND_DIR_USER, (basefilename) ?
		format("%s.html", basefilename) : "dumpedlevel.html");
	fo = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (fo) {
		dump_level(fo, (title) ? title : "Dumped Level", c, NULL);
		if (file_close(fo)) {
			msg("Level dumped to %s.html",
				(basefilename) ? basefilename : "dumpedlevel");
		}
	}
}


/**
 * Dump the given level to a file for post-mortem analysis.
 * \param fo Is the file handle to use.  Must be capable of sequential writes
 * in text format.  The level is dumped starting at the current offset in the
 * file.
 * \param title Is the title to use for the contents.
 * \param c Is the chunk to dump.
 * \param dist If not NULL, must act like a two dimensional C array with the
 * first dimension being at least c->height elements and the second being at
 * least c->width elements.  For a location (x,y) in the level, if dist[y][x]
 * is negative, the contents will be rendered differently.
 *
 * The current output format is HTML since a typical browser will happily
 * display the content in a scrollable area without wrapping lines.  This
 * function is a convenience to replace a set of calls to dump_level_header(),
 * dump_level_body(), and dump_level_footer().
 */
void dump_level(ang_file *fo, const char *title, struct chunk *c, int **dist)
{
	dump_level_header(fo, title);
	dump_level_body(fo, title, c, dist);
	dump_level_footer(fo);
}


/**
 * Helper function to write a string while escaping any special characters.
 * \param fo Is the file handle to use.
 * \param s Is the string to write.
 */
static void dump_level_escaped_string(ang_file *fo, const char *s)
{
	while (*s) {
		switch (*s) {
		case '&':
			file_put(fo, "&amp;");
			break;

		case '<':
			file_put(fo, "&lt;");
			break;

		case '>':
			file_put(fo, "&gt;");
			break;

		case '\"':
			file_put(fo, "&quot;");
			break;

		default:
			file_putf(fo, "%c", *s);
			break;
		}
		++s;
	}
}


/**
 * Write the introductory material for the dump of one or move levels.
 * \param fo Is the file handle to use.  Must be capable of sequential writes
 * in text format.  Writes start at the current offset in the file.
 * \param title Is the title to use for the contents of the file.
 *
 * The current format uses HTML.  This should be called once per dump (or
 * take other measures to overwrite a previous call).
 */
void dump_level_header(ang_file *fo, const char *title)
{
	file_put(fo,
		"<!DOCTYPE html>\n"
		"<html lang=\"en\" xml:lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"  <head>\n"
		"    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
		"    <title>");
	dump_level_escaped_string(fo, title);
	file_put(fo, "</title>\n  </head>\n  <body>\n");
}


/**
 * Dump the given level to a file.
 * \param fo Is the file handle to use.  Must be capable of sequential writes
 * in text format.  The level is dumped starting at the current offset in the
 * file.
 * \param title Is the title to use for the level.
 * \param c Is the chunk to dump.
 * \param dist If not NULL, must act like a two dimensional C array with the
 * first dimension being at least c->height elements and the second being at
 * least c->width elements.  For a location (x,y) in the level, if dist[y][x]
 * is negative, the contents will be rendered differently.
 *
 * The current output format is HTML.  You can dump more than one level to
 * the same file by calling dump_level_header() once for the file, followed
 * by calling dump_level_body() for each level of interest, then calling
 * dump_level_footer() once to finish things off before you close the file
 * with file_close().
 */
void dump_level_body(ang_file *fo, const char *title, struct chunk *c,
	int **dist)
{
	int y;

	file_put(fo, "    <p>");
	dump_level_escaped_string(fo, title);
	if (dist != NULL) {
		file_put(fo, "\n    <p>A location where the distance array was negative is marked with *.");
	}
	file_put(fo, "\n    <pre>\n");
	for (y = 0; y < c->height; ++y) {
		int x;

		for (x = 0; x < c->width; ++x) {
			struct loc grid = loc(x, y);
			const char *s = "#";

			if (square_in_bounds_fully(c, grid)) {
				if (square_isplayer(c, grid)) {
					s = "@";
				} else if (square_isoccupied(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"M" : "*";
				} else if (square_isdoor(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"+" : "*";
				} else if (square_isrubble(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						":" : "*";
				} else if (square_isdownstairs(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"&gt;" : "*";
				} else if (square_isupstairs(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"&lt;" : "*";
				} else if (square_istrap(c, grid) ||
					square_isplayertrap(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"^" : "*";
				} else if (square_iswebbed(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"w" : "*";
				} else if (square_object(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"$" : "*";
				} else if (square_isempty(c, grid) &&
						(square_isvault(c, grid) ||
						square_isno_stairs(c, grid))) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						" " : "*";
				} else if (square_ispassable(c, grid)) {
					s = (dist == NULL || dist[y][x] >= 0) ?
						"." : "*";
				}
			}
			file_put(fo, s);
		}
		file_put(fo, "\n");
	}
	file_put(fo, "    </pre>\n");
}


/**
 * Write the concluding material for the dump of one or more levels.
 * \param fo Is the file handle to use.  Must be capable of sequential writes
 * in text format.  Writes start at the current offset in the file.
 */
void dump_level_footer(ang_file *fo)
{
	file_put(fo, "  </body>\n</html>\n");
}
