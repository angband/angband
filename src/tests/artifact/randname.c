/* artifact/randname */

#include "unit-test.h"
#include "object/object.h"

nosetup;
noteardown;

#define NAMES_TRIES	100

const char *names[] = {
	"aaaaaa",
	"bbbbbb",
	"cccccc",
	"dddddd",
	"eeeeee",
	"ffffff",
	"gggggg",
	"hhhhhh",
	"iiiiii",
	"jjjjjj",
	NULL
};

const char **p[] = { names, names };

static int test_names(void *state) {
	struct artifact a;
	char *n;
	int i;

	a.aidx = 1;
	for (i = 0; i < NAMES_TRIES; i++) {
		n = artifact_gen_name(&a, p);
		if (strchr(n, '\''))
			require(strchr(n, '\'') != strrchr(n, '\''));
		else
			require(strstr(n, "of "));
		mem_free(n);
	}

	ok;
}

static const char *suite_name = "artifact/randname";
static struct test tests[] = {
	{ "names", test_names },
	{ NULL, NULL }
};
