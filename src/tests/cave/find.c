/* cave/find */

#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"
#include "cave.h"
#include "generate.h"
#include "z-rand.h"
#include "z-virt.h"

int setup_tests(void **state) {
	struct chunk *c;

	Rand_init();
	z_info = &test_z_info;
	c = cave_new(11, 9);
	*state = c;
	return 0;
}

int teardown_tests(void *state) {
	cave_free(state);
	return 0;
}

static void wipe_chunk_flags(struct chunk *c) {
	struct loc grid;

	for (grid.y = 0; grid.y < c->height; ++grid.y) {
		for (grid.x = 0; grid.x < c->width; ++grid.x) {
			sqinfo_wipe(square(c, grid)->info);
		}
	}
}

static int test_cave_find_0(void *state) {
	/* In this, a negative coordinate is from one end. */
	const struct loc targets[] = {
		{ 0, 0 }, { -1, 0 }, { 0, -1 }, { -1, -1 }
	};
	struct chunk *c = state;
	struct loc grid, target;
	int i;

	wipe_chunk_flags(c);

	require(!cave_find(c, &grid, square_isroom));
	for (i = 0; i < (int)N_ELEMENTS(targets); ++i) {
		target.x = targets[i].x + ((targets[i].x < 0) ? c->width : 0);
		target.y = targets[i].y + ((targets[i].y < 0) ? c->height : 0);
		sqinfo_on(square(c, target)->info, SQUARE_ROOM);
		require(cave_find(c, &grid, square_isroom));
		require(loc_eq(grid, target));
		sqinfo_off(square(c, target)->info, SQUARE_ROOM);
	}

	target.x = 1 + randint0(c->width - 2);
	target.y = 0;
	sqinfo_on(square(c, target)->info, SQUARE_ROOM);
	require(cave_find(c, &grid, square_isroom));
	require(loc_eq(grid, target));
	sqinfo_off(square(c, target)->info, SQUARE_ROOM);

	target.x = 1 + randint0(c->width - 2);
	target.y = c->height - 1;
	sqinfo_on(square(c, target)->info, SQUARE_ROOM);
	require(cave_find(c, &grid, square_isroom));
	require(loc_eq(grid, target));
	sqinfo_off(square(c, target)->info, SQUARE_ROOM);

	target.x = 1 + randint0(c->width - 2);
	target.y = 1 + randint0(c->height - 2);
	sqinfo_on(square(c, target)->info, SQUARE_ROOM);
	require(cave_find(c, &grid, square_isroom));
	require(loc_eq(grid, target));
	sqinfo_off(square(c, target)->info, SQUARE_ROOM);

	target.x = 0;
	target.y = 1 + randint0(c->height - 2);
	sqinfo_on(square(c, target)->info, SQUARE_ROOM);
	require(cave_find(c, &grid, square_isroom));
	require(loc_eq(grid, target));
	sqinfo_off(square(c, target)->info, SQUARE_ROOM);

	target.x = c->width - 1;
	target.y = 1 + randint0(c->height - 2);
	sqinfo_on(square(c, target)->info, SQUARE_ROOM);
	require(cave_find(c, &grid, square_isroom));
	require(loc_eq(grid, target));
	sqinfo_off(square(c, target)->info, SQUARE_ROOM);

	ok;
}

static int test_cave_find_in_range_0(void *state) {
	struct chunk *c = state;
	struct loc ul, br, grid;

	ul.x = -3;
	ul.y = -2;
	br.x = 0;
	br.y = 0;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds_fully));
	require(cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	require(loc_eq(grid, br));

	ul.x = c->width - 1;
	ul.y = -1;
	br.x = c->width + 5;
	br.y = 0;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds_fully));
	require(cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	require(loc_eq(grid, loc(c->width - 1, 0)));

	ul.x = -1;
	ul.y = c->height - 1;
	br.x = 0;
	br.y = c->height + 2;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds_fully));
	require(cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	require(loc_eq(grid, loc(0, c->height - 1)));

	ul.x = c->width - 1;
	ul.y = c->height - 1;
	br.x = c->width + 2;
	br.y = c->height + 3;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds_fully));
	require(cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	require(loc_eq(grid, ul));

	ul.x = 0;
	ul.y = 0;
	br.x = c->width - 1;
	br.y = c->height - 1;
	require(cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	require(grid.x >= 0 && grid.x < c->width && grid.y >= 0 &&
		grid.y < c->height);
	require(cave_find_in_range(c, &grid, ul, br, square_in_bounds_fully));
	require(grid.x >= 1 && grid.x < c->width - 1 && grid.y >= 1
		&& grid.y < c->height - 1);

	/* Check empty search ranges. */
	ul.x = c->width / 2;
	ul.y = c->height / 2;
	br.x = ul.x - 3;
	br.y = ul.y + 3;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	br.x = ul.x + 4;
	br.y = ul.y - 2;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds));
	br.x = ul.x - 2;
	br.y = ul.y - 4;
	require(!cave_find_in_range(c, &grid, ul, br, square_in_bounds));

	ok;
}

static int test_find_nearby_grid_0(void *state) {
	struct chunk *c = state;
	struct loc grid;

	require(!find_nearby_grid(c, &grid, loc(-4, -3), 2, 3));
	require(!find_nearby_grid(c, &grid, loc(c->width + 2, 1), 3, 1));
	require(!find_nearby_grid(c, &grid, loc(-3, c->height + 4), 4, 2));
	require(!find_nearby_grid(c, &grid, loc(c->width + 2, c->height + 1),
		1, 2));

	require(find_nearby_grid(c, &grid, loc(c->width / 2, -1), 2, 1));
	require(grid.x >= c->width / 2 - 1 && grid.x <= c->width / 2 + 1
		&& grid.y == 1);

	require(find_nearby_grid(c, &grid, loc(c->width / 2, c->height + 1),
		3, 1));
	require(grid.x >= c->width / 2 - 1 && grid.x <= c->width / 2 + 1
		&& grid.y == c->height - 2);

	require(find_nearby_grid(c, &grid, loc(-1, c->height / 2),
		1, 2));
	require(grid.x == 1 && grid.y >= c->height / 2 - 1
		&& grid.y <= c->height / 2 + 1);

	require(find_nearby_grid(c, &grid, loc(c->width + 2, c->height / 2),
		1, 4));
	require(grid.x == c->width - 2 && grid.y >= c->height / 2 - 1
		&& grid.y <= c->height / 2 + 1);

	ok;
}

static int test_unbundled_find_0(void *state) {
	struct chunk *c = state;
	bool invalid = false;
	int *find_state;
	struct loc grid;

	wipe_chunk_flags(c);

	find_state = cave_find_init(loc(1, 1),
		loc(c->width - 2, c->height - 2));
	while (cave_find_get_grid(&grid, find_state)) {
		if (square_in_bounds_fully(c, grid) && !square_isroom(c, grid)) {
			sqinfo_on(square(c, grid)->info, SQUARE_ROOM);
		} else {
			invalid = true;
		}
	}

	/* Verify that it visited all the grids. */
	for (grid.y = 1; grid.y < c->height - 1; ++grid.y) {
		for (grid.x = 1; grid.x < c->width - 1; ++grid.x) {
			if (!square_isroom(c, grid)) {
				invalid = true;
			}
		}
	}

	cave_find_reset(find_state);
	while (cave_find_get_grid(&grid, find_state)) {
		if (square_in_bounds_fully(c, grid) && square_isroom(c, grid)) {
			sqinfo_off(square(c, grid)->info, SQUARE_ROOM);
		} else {
			invalid = true;
		}
	}

	/* Verify that the second pass also visited all the grids. */
	for (grid.y = 1; grid.y < c->height - 1; ++grid.y) {
		for (grid.x = 1; grid.x < c->width - 1; ++grid.x) {
			if (square_isroom(c, grid)) {
				invalid = true;
			}
		}
	}

	mem_free(find_state);
	require(!invalid);
	ok;
}

const char *suite_name = "cave/find";
struct test tests[] = {
	{ "cave_find 0", test_cave_find_0 },
	{ "cave_find_in_range 0", test_cave_find_in_range_0 },
	{ "find_nearby_grid 0", test_find_nearby_grid_0 },
	{ "unbundled find 0", test_unbundled_find_0 },
	{ NULL, NULL }
};
