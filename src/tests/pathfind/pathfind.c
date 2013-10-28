/* pathfind/pathfind */

#include "unit-test.h"
#include "game-cmd.h"
#include "pathfind.h"

NOSETUP
NOTEARDOWN

int test_dir_to(void *state) {
	eq(pathfind_direction_to(loc(0,0), loc(0,1)), DIR_N);
	eq(pathfind_direction_to(loc(0,0), loc(1,0)), DIR_E);
	eq(pathfind_direction_to(loc(0,0), loc(1,1)), DIR_NE);
	eq(pathfind_direction_to(loc(0,0), loc(0,-1)), DIR_S);
	eq(pathfind_direction_to(loc(0,0), loc(-1,0)), DIR_W);
	eq(pathfind_direction_to(loc(0,0), loc(-1,-1)), DIR_SW);
	eq(pathfind_direction_to(loc(0,0), loc(-1,1)), DIR_NW);
	eq(pathfind_direction_to(loc(0,0), loc(1,-1)), DIR_SE);
	eq(pathfind_direction_to(loc(0,0), loc(0,0)), DIR_NONE);

	eq(pathfind_direction_to(loc(0,0), loc(1,10)), DIR_N);
	eq(pathfind_direction_to(loc(0,0), loc(8,10)), DIR_NE);
	eq(pathfind_direction_to(loc(0,0), loc(12,4)), DIR_E);
	ok;
}

const char *suite_name = "pathfind/pathfind";
struct test tests[] = {
	{ "dir-to", test_dir_to },
	{ NULL, NULL },
};
