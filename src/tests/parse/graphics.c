/* parse/graphics */

#include <stdio.h>

#include "unit-test.h"
#include "test-utils.h"

#include "game-event.h"
#include "init.h" /* init_angband */
#include "message.h" /* msg */
#include "grafmode.h"
#include "ui-prefs.h"
#include "cmd-core.h"

int setup_tests(void **state) {
	set_file_paths();
	init_angband();
	init_graphics_modes("graphics.txt");
	textui_prefs_init();
	return 0;
}

int teardown_tests(void *state) {
	textui_prefs_free();
	return 0;
}

static void getmsg(game_event_type type, game_event_data *data, void *user) {
	bool *error = user;

	fprintf(stderr, "Message: %s\n", data->message.msg);
	*error = TRUE;
}

int test_prefs(void *state) {
	bool error = FALSE;
	graphics_mode *mode;

	/* This is a bit of a hack to ensure we have a player struct set up */
	/* Otherwise race/class dependent graphics will crash */
	cmdq_push(CMD_BIRTH_RESET);
	cmdq_execute(CMD_BIRTH);

	event_add_handler(EVENT_MESSAGE, getmsg, &error);

	for (mode = graphics_modes; mode; mode = mode->pNext) {
		/* Skip 'normal' */
		if (mode->grafID == 0) continue;

		printf("Testing mode '%s'.\n", mode->menuname);

		/* Load pref file */
		use_graphics = mode->grafID;
		reset_visuals(TRUE);
	}

	eq(error, FALSE);

	ok;
}

const char *suite_name = "parse/graphics";
struct test tests[] = {
	{ "prefs", test_prefs },
	{ NULL, NULL }
};
