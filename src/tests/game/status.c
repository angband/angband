/* game/status.c */

#include "unit-test.h"
#include "test-utils.h"
#include <stdio.h>
#include <string.h>
#include "cave.h"
#include "game-event.h"
#include "init.h"
#include "player.h"
#include "player-birth.h"
#include "player-timed.h"
#include "trap.h"
#include "ui-display.h"
#include "ui-prefs.h"
#include "ui-term.h"

static term test_term;

int setup_tests(void **state) {
	set_file_paths();
	if (!init_angband())
		return 1;
#ifdef UNIX
	create_needed_dirs();
#endif
	if (!player_make_simple("Human", "Warrior", "Tester")) {
		cleanup_angband();
		return 1;
	}

	cave = t_build_arena(10, 10);
	player->cave = cave_new(10, 10);
	player->grid = loc(4, 4);
	player->depth = cave->depth = player->cave->depth = 1;
	player->upkeep->redraw = 0;
	term_init(&test_term, 160, 24, 256);
	Term_activate(&test_term);
	angband_term[0] = &test_term;
	textui_prefs_init();
	init_display();
	event_signal(EVENT_ENTER_WORLD);
	return 0;
}

int teardown_tests(void *state) {
	event_signal(EVENT_LEAVE_WORLD);
	textui_prefs_free();
	cleanup_angband();
	term_nuke(&test_term);
	angband_term[0] = NULL;
	return 0;
}

static void reset_square(void) {
	square_remove_all_traps(cave, player->grid);
	square_remove_all_traps(player->cave, player->grid);
	sqinfo_wipe(square(cave, player->grid)->info);
	square_set_feat(cave, player->grid, FEAT_MORE);
	square_forget(cave, player->grid);
	player->timed[TMD_BLIND] = 0;
	player->state.cur_light = 2;
}

static int check_status(const char *name, int expected_attr) {
	char expected[31];
	char line[161];
	wchar_t ch;
	char *start;
	int x, attr;

	my_strcpy(expected, name, sizeof(expected) - 1);
	my_strcap(expected);
	my_strcat(expected, " ", sizeof(expected));
	event_signal(EVENT_STATUS);
	for (x = 0; x < 160; ++x) {
		eq(Term_what(x, 23, &attr, &ch), 0);
		line[x] = ch ? (char)ch : ' ';
	}
	line[160] = '\0';
	if (verbose)
		printf("\nstatus: %s\n", line);
	start = strstr(line, expected);
	notnull(start);
	eq(Term_what(start - line, 23, &attr, &ch), 0);
	eq(attr, expected_attr);
	ok;
}

static int test_seen(void *state) {
	reset_square();
	update_view(cave, player);
	require(square_isseen(cave, player->grid));
	require(square_isknown(cave, player->grid));
	if (check_status(f_info[FEAT_MORE].name, f_info[FEAT_MORE].d_attr))
		return 1;

	/* Visible terrain changes still update the status line. */
	square_set_feat(cave, player->grid, FEAT_LAVA);
	square_note_spot(cave, player->grid);
	return check_status(f_info[FEAT_LAVA].name, f_info[FEAT_LAVA].d_attr);
}

static int test_blind(void *state) {
	reset_square();
	player->timed[TMD_BLIND] = 1;
	update_view(cave, player);
	require(!square_isseen(cave, player->grid));
	require(!square_isknown(cave, player->grid));
	return check_status(f_info[FEAT_NONE].name, f_info[FEAT_NONE].d_attr);
}

static int test_unlit(void *state) {
	reset_square();
	player->state.cur_light = 0;
	update_view(cave, player);
	require(!square_isseen(cave, player->grid));
	require(!square_isknown(cave, player->grid));
	return check_status(f_info[FEAT_NONE].name, f_info[FEAT_NONE].d_attr);
}

static int test_remembered(void *state) {
	reset_square();
	square_memorize(cave, player->grid);
	square_set_feat(cave, player->grid, FEAT_FLOOR);
	player->state.cur_light = 0;
	update_view(cave, player);
	require(!square_isseen(cave, player->grid));
	eq(square_feat(player->cave, player->grid), &f_info[FEAT_MORE]);
	eq(square_feat(cave, player->grid), &f_info[FEAT_FLOOR]);
	return check_status(f_info[FEAT_MORE].name, f_info[FEAT_MORE].d_attr);
}

static int test_mimic(void *state) {
	reset_square();
	square_set_feat(player->cave, player->grid, FEAT_SECRET);
	notnull(f_info[FEAT_SECRET].mimic);
	return check_status(f_info[FEAT_GRANITE].name, f_info[FEAT_GRANITE].d_attr);
}

static int test_hidden_trap(void *state) {
	struct trap_kind *kind = lookup_trap("pit");

	reset_square();
	square_set_feat(cave, player->grid, FEAT_FLOOR);
	square_memorize(cave, player->grid);
	player->timed[TMD_BLIND] = 1;
	player->state.cur_light = 0;
	update_view(cave, player);
	notnull(kind);
	place_trap(cave, player->grid, kind->tidx, 1);
	square_memorize_traps(cave, player->grid);
	notnull(square_trap(cave, player->grid));
	null(square_trap(player->cave, player->grid));
	return check_status(f_info[FEAT_FLOOR].name, f_info[FEAT_FLOOR].d_attr);
}

static int check_known_trap(struct trap_kind *kind) {
	struct grid_data data;

	map_info(player->grid, &data);
	require(!data.in_view);
	notnull(data.trap);
	eq(data.trap->kind, kind);
	return check_status(kind->name, kind->d_attr);
}

static int test_remembered_trap(void *state) {
	struct trap_kind *kind = lookup_trap("pit");

	reset_square();
	square_set_feat(cave, player->grid, FEAT_FLOOR);
	square_memorize(cave, player->grid);
	notnull(kind);
	place_trap(cave, player->grid, kind->tidx, 1);
	notnull(square_trap(cave, player->grid));
	trf_on(square_trap(cave, player->grid)->flags, TRF_VISIBLE);
	square_memorize_traps(cave, player->grid);
	notnull(square_trap(player->cave, player->grid));
	if (check_status(kind->name, kind->d_attr))
		return 1;

	/* The map also displays remembered traps while blind or without light. */
	player->timed[TMD_BLIND] = 1;
	update_view(cave, player);
	if (check_known_trap(kind))
		return 1;
	player->timed[TMD_BLIND] = 0;
	player->state.cur_light = 0;
	update_view(cave, player);
	if (check_known_trap(kind))
		return 1;

	/* Forgetting terrain also hides any leftover trap memory. */
	square_forget(cave, player->grid);
	notnull(square_trap(player->cave, player->grid));
	if (check_status(f_info[FEAT_NONE].name, f_info[FEAT_NONE].d_attr))
		return 1;
	square_memorize(cave, player->grid);

	/* Changes outside the player's view do not change remembered traps. */
	sqinfo_off(square(cave, player->grid)->info, SQUARE_SEEN);
	square_remove_all_traps(cave, player->grid);
	notnull(square_trap(player->cave, player->grid));
	if (check_status(kind->name, kind->d_attr))
		return 1;

	square_memorize_traps(cave, player->grid);
	return check_status(f_info[FEAT_FLOOR].name, f_info[FEAT_FLOOR].d_attr);
}

const char *suite_name = "game/status";
struct test tests[] = {
	{ "seen", test_seen },
	{ "blind", test_blind },
	{ "unlit", test_unlit },
	{ "remembered", test_remembered },
	{ "mimic", test_mimic },
	{ "hidden-trap", test_hidden_trap },
	{ "remembered-trap", test_remembered_trap },
	{ NULL, NULL }
};
