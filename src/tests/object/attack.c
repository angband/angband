/* object/attack */

#include "unit-test.h"
#include "unit-test-data.h"

#include "attack.h"
#include "object/object.h"

NOSETUP
NOTEARDOWN

int test_breakage_chance(void *state) {
	struct object obj;
	int c;

	object_prep(&obj, &test_longsword, 1, AVERAGE);
	c = breakage_chance(&obj, TRUE);
	eq(c, 50);
	c = breakage_chance(&obj, FALSE);
	eq(c, 25);
	obj.artifact = &test_artifact_sword;
	c = breakage_chance(&obj, TRUE);
	eq(c, 0);
	c = breakage_chance(&obj, FALSE);
	eq(c, 0);
	ok;
}

const char *suite_name = "object/attack";
struct test tests[] = {
	{ "breakage-chance", test_breakage_chance },
	{ NULL, NULL }
};
