/* parse/h-info */

#include "unit-test.h"

#include "init.h"
#include "player/types.h"

int setup_tests(void **state) {
	*state = init_parse_h();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:1:3:5:2");
	struct history_chart *c;
	struct history_entry *e;

	eq(r, PARSE_ERROR_NONE);
	c = parser_priv(state);
	require(c);
	e = c->entries;
	require(e);
	eq(c->idx, 1);
	ptreq(e->next, NULL);
	ptreq(e->roll, 5);
	ptreq(e->bonus, 2);
	eq(e->isucc, 3);
	ok;
}

int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:hello there");
	struct history_chart *h;

	eq(r, PARSE_ERROR_NONE);
	h = parser_priv(state);
	require(h);
	require(streq(h->entries->text, "hello there"));
	ok;
}

const char *suite_name = "parse/h-info";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
