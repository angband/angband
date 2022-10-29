/* parse/world */
/* Exercise parsing used for world.txt. */

#include "unit-test.h"
#include "game-world.h"
#include "init.h"

NOSETUP
NOTEARDOWN

static int test_complete0(void *state)
{
	const char *lines[] = {
		"level:0:Town:None:Test 1",
		"level:1:Test 1:Town:Test 2",
		"level:2:Test 2:Test 1:None"
	};
	struct parser *p = world_parser.init();
	struct level *lv;
	int i, rtn;

	notnull(p);
	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	rtn = world_parser.finish(p);
	eq(rtn, 0);

	lv = world;
	notnull(lv);
	eq(lv->depth, 0);
	notnull(lv->name);
	require(streq(lv->name, "Town"));
	null(lv->up);
	notnull(lv->down);
	require(streq(lv->down, "Test 1"));
	lv = lv->next;
	notnull(lv);
	eq(lv->depth, 1);
	notnull(lv->name);
	require(streq(lv->name, "Test 1"));
	notnull(lv->up);
	require(streq(lv->up, "Town"));
	notnull(lv->down);
	require(streq(lv->down, "Test 2"));
	lv = lv->next;
	notnull(lv);
	eq(lv->depth, 2);
	notnull(lv->name);
	require(streq(lv->name, "Test 2"));
	notnull(lv->up);
	require(streq(lv->up, "Test 1"));
	null(lv->down);
	lv = lv->next;
	null(lv);

	world_parser.cleanup();
	ok;
}

const char *suite_name = "parse/world";
struct test tests[] = {
	{ "complete0", test_complete0 },
	{ NULL, NULL }
};
