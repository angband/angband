/* cave/scatter */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "init.h"

static struct chunk *create_empty_cave(int height, int width) {
	struct chunk *c = cave_new(height, width);
	struct loc grid;

	grid.y = 0;
	for (grid.x = 0; grid.x < width; ++grid.x) {
		square_set_feat(c, grid, FEAT_PERM);
	}
	for (grid.y = 1; grid.y < height - 1; ++grid.y) {
		grid.x = 0;
		square_set_feat(c, grid, FEAT_PERM);
		for (grid.x = 1; grid.x < width - 1; ++grid.x) {
			square_set_feat(c, grid, FEAT_FLOOR);
		}
		grid.x = width - 1;
		square_set_feat(c, grid, FEAT_PERM);
	}
	grid.y = height - 1;
	for (grid.x = 0; grid.x < width; ++grid.x) {
		square_set_feat(c, grid, FEAT_PERM);
	}
	return c;
}

int setup_tests(void **state) {
	struct chunk *c;

	/* Need to initialize the terrain information. */
	set_file_paths();
	if (!init_angband()) {
		*state = NULL;
		return 1;
	}

	c  = create_empty_cave(7, 9);
	*state = c;

	return 0;
}

int teardown_tests(void *state) {
	cave_free(state);
	cleanup_angband();
	return 0;
}

static int test_scatter_0(void *state) {
	struct chunk *c = state;
	struct loc ctr = loc(c->width / 2, c->height / 2);
	struct loc grids[3] = { loc(-1, -1), loc(-1, -1), loc(-1, -1) };
	int n;

	/* For a point fully in bounds, should get the search origin back. */
	n = scatter_ext(c, grids + 1, 9, ctr, 0, false, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == ctr.x && grids[1].y == ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	n = scatter_ext(c, grids + 1, 9, ctr, 0, true, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == ctr.x && grids[1].y == ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 0, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == ctr.x && grids[1].y == ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 0, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == ctr.x && grids[1].y == ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	/* For a point not fully in bounds, should get nothing. */
	ctr = loc(c->width - 1, -1);
	grids[1] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 9, ctr, 0, false, NULL);
	eq(n, 0);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == -1 && grids[1].y == -1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(c->width - 1, -1);
	grids[1] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 9, ctr, 0, true, NULL);
	eq(n, 0);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == -1 && grids[1].y == -1);
	require(grids[2].x == -1 && grids[2].y == -1);

	scatter(c, grids + 1, ctr, 0, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == -1 && grids[1].y == -1);
	require(grids[2].x == -1 && grids[2].y == -1);

	scatter(c, grids + 1, ctr, 0, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == -1 && grids[1].y == -1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ok;
}

static int test_scatter_1(void *state) {
	struct chunk *c = state;
	struct loc ctr = loc(c->width / 2, c->height / 2);
	struct loc grids[11];
	int n, i, j;

	/* Asking for 9 locations should give nine distinct locations. */
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		false, NULL);
	eq(n, 9);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x - 1 && grids[i].x <= ctr.x + 1);
		require(grids[i].y >= ctr.y - 1 && grids[i].y <= ctr.y + 1);
		for (j = 1; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		true, NULL);
	eq(n, 9);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x - 1 && grids[i].x <= ctr.x + 1);
		require(grids[i].y >= ctr.y - 1 && grids[i].y <= ctr.y + 1);
		for (j = 1; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x - 1 && grids[1].x <= ctr.x + 1);
	require(grids[1].y >= ctr.y - 1 && grids[1].y <= ctr.y + 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x - 1 && grids[1].x <= ctr.x + 1);
	require(grids[1].y >= ctr.y - 1 && grids[1].y <= ctr.y + 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	/*
	 * Put the search origin diagonally in from the corners to test for
	 * fully in bounds.
	 */
	ctr = loc(1, 1);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		false, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x && grids[i].x <= ctr.x + 1);
		require(grids[i].y >= ctr.y && grids[i].y <= ctr.y + 1);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		true, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x && grids[i].x <= ctr.x + 1);
		require(grids[i].y >= ctr.y && grids[i].y <= ctr.y + 1);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x && grids[1].x <= ctr.x + 1);
	require(grids[1].y >= ctr.y && grids[1].y <= ctr.y + 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x && grids[1].x <= ctr.x + 1);
	require(grids[1].y >= ctr.y && grids[1].y <= ctr.y + 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(c->width - 2, 1);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		false, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x - 1 && grids[i].x <= ctr.x);
		require(grids[i].y >= ctr.y && grids[i].y <= ctr.y + 1);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		true, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x - 1 && grids[i].x <= ctr.x);
		require(grids[i].y >= ctr.y && grids[i].y <= ctr.y + 1);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x - 1 && grids[1].x <= ctr.x);
	require(grids[1].y >= ctr.y && grids[1].y <= ctr.y + 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x - 1 && grids[1].x <= ctr.x);
	require(grids[1].y >= ctr.y && grids[1].y <= ctr.y + 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(1, c->height - 2);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		false, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x && grids[i].x <= ctr.x + 1);
		require(grids[i].y >= ctr.y - 1 && grids[i].y <= ctr.y);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		true, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x && grids[i].x <= ctr.x + 1);
		require(grids[i].y >= ctr.y - 1 && grids[i].y <= ctr.y);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x && grids[1].x <= ctr.x + 1);
	require(grids[1].y >= ctr.y - 1 && grids[1].y <= ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x && grids[1].x <= ctr.x + 1);
	require(grids[1].y >= ctr.y - 1 && grids[1].y <= ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(c->width - 2, c->height - 2);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		false, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x - 1 && grids[i].x <= ctr.x);
		require(grids[i].y >= ctr.y - 1 && grids[i].y <= ctr.y);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 1,
		true, NULL);
	eq(n, 4);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(grids[i].x >= ctr.x - 1 && grids[i].x <= ctr.x);
		require(grids[i].y >= ctr.y - 1 && grids[i].y <= ctr.y);
		for (j = 0; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	require(grids[n + 1].x == -1 && grids[n + 1].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x - 1 && grids[1].x <= ctr.x);
	require(grids[1].y >= ctr.y - 1 && grids[1].y <= ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x >= ctr.x - 1 && grids[1].x <= ctr.x);
	require(grids[1].y >= ctr.y - 1 && grids[1].y <= ctr.y);
	require(grids[2].x == -1 && grids[2].y == -1);

	/* Check the four corners. */
	ctr = loc(0, 0);
	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, false, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, true, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(c->width - 1, 0);
	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, false, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, true, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(0, c->height - 1);
	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, false, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, true, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == 1 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	ctr = loc(c->width - 1, c->height - 1);
	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, false, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	n = scatter_ext(c, grids + 1, 2, ctr, 1, true, NULL);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == c->width - 2 && grids[1].y == c->height - 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	/* If it's far enough outside, there should be no matches. */
	ctr = loc(-1, c->height);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, 9, ctr, 1, false, NULL);
	eq(n, 0);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		require(grids[i].x == -1 && grids[i].y == -1);
	}

	n = scatter_ext(c, grids + 1, 9, ctr, 1, true, NULL);
	eq(n, 0);
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		require(grids[i].x == -1 && grids[i].y == -1);
	}

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == -1 && grids[1].y == -1);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 1, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(grids[1].x == -1 && grids[1].y == -1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ok;
}

static int test_scatter_2(void *state) {
	struct chunk *c = state;
	struct loc ctr = loc(c->width / 2, c->height / 2);
	struct loc grids[27];
	int n, i, j;

	/*
	 * Should get 21 locations with the way distance is currently
	 * approximated.
	 */
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 2,
		false, NULL);
	eq(n, 21);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(distance(ctr, grids[i]) <= 2);
		for (j = 1; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	for (i = n + 1; i < (int) N_ELEMENTS(grids); ++i) {
		require(grids[i].x == -1 && grids[i].y == -1);
	}

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 2,
		true, NULL);
	eq(n, 21);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(distance(ctr, grids[i]) <= 2);
		for (j = 1; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	for (i = n + 1; i < (int) N_ELEMENTS(grids); ++i) {
		require(grids[i].x == -1 && grids[i].y == -1);
	}

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 2, false);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(distance(ctr, grids[1]) <= 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 2, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(distance(ctr, grids[1]) <= 2);
	require(grids[2].x == -1 && grids[2].y == -1);

	ok;
}

static int test_scatter_los(void *state) {
	struct chunk *c = state;
	struct loc ctr = loc(c->width / 2, c->height / 2);
	struct loc grids[27];
	int n, i, j;

	for (i = 1; i < c->height - 1; ++i) {
		square_set_feat(c, loc(ctr.x - 1, i), FEAT_GRANITE);
	}

	/*
	 * Should get 18 locations (the wall is in the line of sight) with
	 * the way distance is currently approximated.
	 */
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 2,
		true, NULL);
	eq(n, 18);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(distance(ctr, grids[i]) <= 2 &&
			grids[i].x >= ctr.x - 1);
		for (j = 1; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	for (i = n + 1; i < (int) N_ELEMENTS(grids); ++i) {
		require(grids[i].x == -1 && grids[i].y == -1);
	}

	grids[1] = loc(-1, -1);
	grids[2] = loc(-1, -1);
	scatter(c, grids + 1, ctr, 2, true);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(distance(ctr, grids[1]) <= 2 && grids[1].x >= ctr.x - 1);
	require(grids[2].x == -1 && grids[2].y == -1);

	ok;
}

static int test_scatter_pred(void *state) {
	struct chunk *c = state;
	struct loc ctr = loc(c->width / 2, c->height / 2);
	struct loc grids[27], g;
	int n, i, j;

	for (g.y = ctr.y - 2; g.y <= ctr.y + 2; ++g.y) {
		for (g.x = ctr.x - 2; g.x <= ctr.x + 2; ++g.x) {
			square_set_feat(c, g, FEAT_FLOOR);
		}
	}
	g.x = rand_range(ctr.x - 2, ctr.x + 2);
	switch (ABS(g.x - ctr.x)) {
		case 0: g.y = rand_range(ctr.y - 2, ctr.y + 2); break;
		case 1: g.y = rand_range(ctr.y - 1, ctr.y + 1); break;
		default: g.y = ctr.y; break;
	}
	square_set_feat(c, g, FEAT_LESS);

	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 2,
		true, square_isstairs);
	eq(n, 1);
	require(grids[0].x == -1 && grids[0].y == -1);
	require(loc_eq(g, grids[1]));
	require(grids[2].x == -1 && grids[2].y == -1);

	/*
	 * Should get 20 locations with the current approximation for distance.
	 */
	for (i = 0; i < (int) N_ELEMENTS(grids); ++i) {
		grids[i] = loc(-1, -1);
	}
	n = scatter_ext(c, grids + 1, (int) N_ELEMENTS(grids) - 1, ctr, 2,
		true, square_isfloor);
	eq(n, 20);
	require(grids[0].x == -1 && grids[0].y == -1);
	for (i = 1; i < n + 1; ++i) {
		require(distance(ctr, grids[i]) <= 2 && !loc_eq(g, grids[i]));
		for (j = 1; j < i; ++j) {
			require(grids[i].x != grids[j].x
				|| grids[i].y != grids[j].y);
		}
	}
	for (i = n + 1; i < (int) N_ELEMENTS(grids); ++i) {
		require(grids[i].x == -1 && grids[i].y == -1);
	}

	ok;
}

/*
 * There's a non-deterministic aspect to this test.  It could fail (about .01%
 * of the time) even if random selection is working correctly).
 */
static int test_scatter_distribution(void *state) {
	struct chunk *c = state;
	struct loc ctr = loc(c->width / 2, c->height / 2);
	int counts[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int n = 9000, i;
	int nexp, dmax;

	for (i = 0; i < n; ++i) {
		struct loc grid = loc(-1, -1);
		int dx, dy;

		scatter(c, &grid, ctr, 1, false);
		dx = grid.x - ctr.x;
		dy = grid.y - ctr.y;
		require(ABS(dx) <= 1 && ABS(dy) <= 1);
		++counts[dx + 1 + 3 * (dy + 1)];
	}

	/* Locate the one that's furthest from the expected value. */
	nexp = n / 9;
	dmax = ABS(counts[0] - nexp);
	for (i = 1; i < 9; ++i) {
		int d = ABS(counts[i] - nexp);

		if (dmax < d) {
			dmax = d;
		}
	}

	/*
	 * By my calculation of the Chernoff upper bound for the cumulative
	 * distribution function of the binomial distribution,
	 * https://en.wikipedia.org/wiki/Binomial_distribution#Tail_bounds
	 * , with n = 9000 and p = 1/9, 874 is the closest point to where the
	 * upper bound crosses a probability of .0001.
	 */
	require(nexp - dmax >= 874);

	ok;
}

const char *suite_name = "cave/scatter";
struct test tests[] = {
	{ "scatter dist=0", test_scatter_0 },
	{ "scatter dist=1", test_scatter_1 },
	{ "scatter dist=2", test_scatter_2 },
	{ "scatter los", test_scatter_los },
	{ "scatter pred", test_scatter_pred },
	{ "scatter distribution", test_scatter_distribution },
	{ NULL, NULL }
};
