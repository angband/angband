/* parse/graphics */

#include <stdio.h>

#include "unit-test.h"
#include "test-utils.h"

#include "game-event.h"
#include "init.h" /* init_angband */
#include "message.h" /* msg */
#include "grafmode.h"
#include "player.h"
#include "player-birth.h"
#include "ui-prefs.h"

#include "mon-util.h" /* lookup_monster_base */
#include "obj-tval.h" /* tval_find_idx */

int setup_tests(void **state) {
	set_file_paths();
	init_angband();
	init_graphics_modes();
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
	*error = true;
}

int test_prefs(void *state) {
	bool error = false;
	graphics_mode *mode;

	/* This is a bit of a hack to ensure we have a player struct set up */
	/* Otherwise race/class dependent graphics will crash */
	eq(player_make_simple(NULL, NULL, NULL), true);

	event_add_handler(EVENT_MESSAGE, getmsg, &error);

	for (mode = graphics_modes; mode; mode = mode->pNext) {
		/* Skip 'normal' */
		if (mode->grafID == 0) continue;

		printf("Testing mode '%s'.\n", mode->menuname);

		/* Load pref file */
		use_graphics = mode->grafID;
		reset_visuals(true);
	}

	eq(error, false);

	ok;
}

int test_defaults(void *state) {
	size_t i;
	struct monster_base *mb = lookup_monster_base("giant");
	int tval = tval_find_idx("sword");

	/* Monster bases */
	eq(process_pref_file_command("monster-base:giant:3:3"), 0);

	for (i = 0; i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];

		if (race->base != mb) continue;

		eq(monster_x_attr[race->ridx], 3);
		eq(monster_x_char[race->ridx], 3);
	}

	/* Object tvals */
	eq(process_pref_file_command("object:sword:*:3:3"), 0);

	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		if (kind->tval != tval)
			continue;

		eq(kind_x_attr[kind->kidx], 3);
		eq(kind_x_char[kind->kidx], 3);
	}

	/* Traps */
	eq(process_pref_file_command("trap:*:*:3:3"), 0);

	for (i = 0; i < z_info->trap_max; i++) {
		int light_idx;

		for (light_idx = 0; light_idx < LIGHTING_MAX; light_idx++) {
			eq(trap_x_attr[light_idx][i], 3);
			eq(trap_x_attr[light_idx][i], 3);
		}
	}

	ok;
}

const char *suite_name = "parse/graphics";
struct test tests[] = {
	{ "prefs", test_prefs },
	{ "defaults", test_defaults },
	{ NULL, NULL }
};
