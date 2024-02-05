/* game/basic.c */

#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"

#include <stdio.h>
#include "cave.h"
#include "cmd-core.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "savefile.h"
#include "player.h"
#include "player-birth.h"
#include "player-timed.h"
#include "z-util.h"

static void event_message(game_event_type type, game_event_data *data, void *user) {
	printf("Message: %s\n", data->message.msg);
}

static void println(const char *str) {
	printf("%s\n", str);
}

static int choose_direction(struct chunk *c, struct player *p) {
	int dir = 0;

	while (1) {
		struct loc grid;

		if (dir >= 9) {
			return -1;
		}
		grid.x = p->grid.x + ddx_ddd[dir];
		grid.y = p->grid.y + ddy_ddd[dir];
		if (square_isempty(c, grid)) {
			return ddd[dir];
		}
		++dir;
	}
}

static int reverse_direction(int dir) {
	int rdir[10] = { 5, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	return (dir >= 0 && dir <= 9) ? rdir[dir] : -1;
}

static void reset_before_load(void) {
	play_again = true;
	wipe_mon_list(cave, player);
	cleanup_angband();
	chunk_list_max = 0;
	init_angband();
	play_again = false;
}

int setup_tests(void **state) {
	/* Register a basic error handler */
	plog_aux = println;

	/* Register some display functions */
	event_add_handler(EVENT_MESSAGE, event_message, NULL);
	event_add_handler(EVENT_INITSTATUS, event_message, NULL);

	/* Init the game */
	set_file_paths();
	init_angband();
#ifdef UNIX
	/* Necessary for creating the randart file. */
	create_needed_dirs();
#endif

	return 0;
}

int teardown_tests(void *state) {
	file_delete("Test1");
	wipe_mon_list(cave, player);
	cleanup_angband();
	return 0;
}

static int test_newgame(void *state) {

	/* Try making a new game */
	eq(player_make_simple(NULL, NULL, "Tester"), true);

	eq(player->is_dead, false);
	prepare_next_level(player);
	on_new_level();
	notnull(cave);
	eq(player->chp, player->mhp);
	eq(player->timed[TMD_FOOD], PY_FOOD_FULL - 1);

	/* Should be all set up to save properly now */
	eq(savefile_save("Test1"), true);

	/* Make sure it saved properly */
	eq(file_exists("Test1"), true);

	ok;
}

static int test_loadgame(void *state) {
	reset_before_load();

	/* Try loading the just-saved game */
	eq(savefile_load("Test1", false), true);

	eq(player->is_dead, false);
	notnull(cave);
	eq(player->chp, player->mhp);
	eq(player->timed[TMD_FOOD], PY_FOOD_FULL - 1);

	ok;
}

static int test_stairs1(void *state) {
	reset_before_load();

	/* Load the saved game */
	eq(savefile_load("Test1", false), true);

	/* Perform normal set up after loading. */
	require(character_dungeon);
	on_new_level();

	cmdq_push(CMD_GO_DOWN);
	run_game_loop();
	eq(player->depth, 1);

	ok;
}

static int test_stairs2(void *state) {
	bool at_stairs = true;
	int dir;

	reset_before_load();

	/* Load the saved game */
	eq(savefile_load("Test1", false), true);

	/* Perform normal set up after loading. */
	require(character_dungeon);
	on_new_level();

	dir = choose_direction(cave, player);
	if (dir >= 0) {
		cmdq_push(CMD_WALK);
		cmd_set_arg_direction(cmdq_peek(), "direction", dir);
		run_game_loop();
		if (!square_monster(cave, loc_sum(player->grid,
				ddgrid[reverse_direction(dir)]))) {
			cmdq_push(CMD_WALK);
			cmd_set_arg_direction(cmdq_peek(), "direction",
				reverse_direction(dir));
			run_game_loop();
		} else {
			/*
			 * A monster got in the way.  Skip testing if can go
			 * down the stairs and report the test was successful.
			 */
			at_stairs = false;
		}
	}
	if (at_stairs) {
		cmdq_push(CMD_GO_DOWN);
		run_game_loop();
		eq(player->depth, 1);
	}

	ok;
}

static int test_drop_pickup(void *state) {
	int dir;

	reset_before_load();

	/* Load the saved game */
	eq(savefile_load("Test1", false), true);

	/* Perform normal set up after loading. */
	require(character_dungeon);
	on_new_level();

	dir = choose_direction(cave, player);
	if (dir >= 0) {
		cmdq_push(CMD_WALK);
		cmd_set_arg_direction(cmdq_peek(), "direction", dir);
		run_game_loop();
		if (player->upkeep->inven[0]->number > 1) {
			cmdq_push(CMD_DROP);
			cmd_set_arg_item(cmdq_peek(), "item",
				player->upkeep->inven[0]);
			cmd_set_arg_number(cmdq_peek(), "quantity", 1);
			run_game_loop();
			eq(square_object(cave, player->grid)->number, 1);
			cmdq_push(CMD_AUTOPICKUP);
			run_game_loop();
		}
		null(square_object(cave, player->grid));
	}

	ok;
}

static int test_drop_eat(void *state) {
	int num = 0;
	int dir;

	reset_before_load();

	/* Load the saved game */
	eq(savefile_load("Test1", false), true);
	num = player->upkeep->inven[0]->number;

	/* Perform normal set up after loading. */
	require(character_dungeon);
	on_new_level();

	dir = choose_direction(cave, player);
	if (dir >= 0) {
		cmdq_push(CMD_WALK);
		cmd_set_arg_direction(cmdq_peek(), "direction", dir);
		run_game_loop();
		cmdq_push(CMD_DROP);
		cmd_set_arg_item(cmdq_peek(), "item", player->upkeep->inven[0]);
		cmd_set_arg_number(cmdq_peek(), "quantity",
					   player->upkeep->inven[0]->number);
		run_game_loop();
		eq(square_object(cave, player->grid)->number, num);
		cmdq_push(CMD_EAT);
		cmd_set_arg_item(cmdq_peek(), "item",
					 square_object(cave, player->grid));
		run_game_loop();
		if (num > 1) {
			eq(square_object(cave, player->grid)->number, num - 1);
		} else {
			null(square_object(cave, player->grid));
		}
	}

	ok;
}

const char *suite_name = "game/basic";
struct test tests[] = {
	{ "newgame", test_newgame },
	{ "loadgame", test_loadgame },
	{ "stairs1", test_stairs1 },
	{ "stairs2", test_stairs2 },
	{ "droppickup", test_drop_pickup },
	{ "dropeat", test_drop_eat },
	{ NULL, NULL }
};
