/* parse/h-info */

#include "unit-test.h"

#include "init.h"
#include "player/types.h"

static int setup(void **state) {
	*state = init_parse_h();
	return !*state;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:1:3:5:2");
	struct history *h;

	eq(r, PARSE_ERROR_NONE);
	h = parser_priv(state);
	require(h);
	eq(h->chart, 1);
	eq(h->next, 3);
	eq(h->roll, 5);
	eq(h->bonus, 2);
	ok;
}

static int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:hello there");
	struct history *h;

	eq(r, PARSE_ERROR_NONE);
	h = parser_priv(state);
	require(h);
	require(streq(h->text, "hello there"));
	ok;
}

static const char *suite_name = "parse/h-info";
static struct test tests[] = {
	{ "n0", test_n0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
