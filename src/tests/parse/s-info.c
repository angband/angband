/* parse/s-info */

#include "unit-test.h"

#include "init.h"
#include "types.h"

static int setup(void **state) {
	*state = init_parse_s();
	return !*state;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:1:Detect Monsters");
	struct spell *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	eq(s->sidx, 1);
	require(streq(s->name, "Detect Monsters"));
	ok;
}

static int test_i0(void *state) {
	enum parser_error r = parser_parse(state, "I:90:0:1");
	struct spell *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	eq(s->tval, 90);
	eq(s->sval, 0);
	eq(s->snum, 1);
	ok;
}

static int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:Teleports you randomly.");
	struct spell *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	require(streq(s->text, "Teleports you randomly."));
	ok;
}

static const char *suite_name = "parse/s-info";
static struct test tests[] = {
	{ "n0", test_n0 },
	{ "i0", test_i0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
