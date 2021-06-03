/* object/attack */

#include "unit-test.h"
#include "unit-test-data.h"

#include "object.h"
#include "obj-make.h"
#include "player-attack.h"

NOSETUP
NOTEARDOWN

static int test_breakage_chance(void *state) {
	struct object obj;
	int c;

	object_prep(&obj, &test_longsword, 1, AVERAGE);
	c = breakage_chance(&obj, true);
	eq(c, 50);
	c = breakage_chance(&obj, false);
	eq(c, 25);
	obj.artifact = &test_artifact_sword;
	c = breakage_chance(&obj, true);
	eq(c, 0);
	c = breakage_chance(&obj, false);
	eq(c, 0);
	ok;
}

const char *suite_name = "object/attack";
struct test tests[] = {
	{ "breakage-chance", test_breakage_chance },
	{ NULL, NULL }
};
