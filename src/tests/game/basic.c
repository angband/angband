/* game/basic.c */

#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"

#include <stdio.h>
#include "cave.h"
#include "cmd-core.h"
#include "game-event.h"
#include "init.h"
#include "savefile.h"
#include "player.h"
#include "player-timed.h"
#include "z-util.h"

static void event_message(game_event_type type, game_event_data *data, void *user) {
	printf("Message: %s\n", data->message.msg);
}

static void println(const char *str) {
	printf("%s\n", str);
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

	return 0;
}

int teardown_tests(void **state) {
	file_delete("Test1");
	cleanup_angband();
	return 0;
}

int test_newgame(void *state) {

	/* Try making a new game */

	cmdq_push(CMD_BIRTH_INIT);
	cmdq_push(CMD_BIRTH_RESET);
	cmdq_push(CMD_CHOOSE_SEX);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);

	cmdq_push(CMD_CHOOSE_RACE);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);

	cmdq_push(CMD_CHOOSE_CLASS);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);

	cmdq_push(CMD_ROLL_STATS);
	cmdq_push(CMD_NAME_CHOICE);
	cmd_set_arg_string(cmdq_peek(), "name", "Tester");

	cmdq_push(CMD_ACCEPT_CHARACTER);
	cmdq_execute(CMD_BIRTH);

	eq(player->is_dead, FALSE);
	cave_generate(&cave, player);
	noteq(cave, NULL);
	eq(player->chp, player->mhp);
	eq(player->food, PY_FOOD_FULL - 1);

	/* Should be all set up to save properly now */
	eq(savefile_save("Test1"), TRUE);

	/* Make sure it saved properly */
	eq(file_exists("Test1"), TRUE);

	ok;
}

int test_loadgame(void *state) {

	/* Try loading the just-saved game */
	eq(savefile_load("Test1", FALSE), TRUE);

	eq(player->is_dead, FALSE);
	noteq(cave, NULL);
	eq(player->chp, player->mhp);
	eq(player->food, PY_FOOD_FULL - 1);

	ok;
}

const char *suite_name = "game/basic";
struct test tests[] = {
	{ "newgame", test_newgame },
	{ "loadgame", test_loadgame },
	{ NULL, NULL }
};
