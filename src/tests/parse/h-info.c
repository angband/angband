/* parse/h-info */

#include "unit-test.h"

#include "init.h"
#include "player.h"

int setup_tests(void **state) {
	*state = init_parse_history();
	return !*state;
}

int teardown_tests(void *state) {
	struct history_chart *h = parser_priv(state);
	string_free(h->entries->text);
	mem_free(h->entries);
	mem_free(h);
	parser_destroy(state);
	return 0;
}

static int test_chart0(void *state) {
	enum parser_error r = parser_parse(state, "chart:1:3:5");
	struct history_chart *c;
	struct history_entry *e;

	eq(r, PARSE_ERROR_NONE);
	c = parser_priv(state);
	require(c);
	e = c->entries;
	require(e);
	eq(c->idx, 1);
	null(e->next);
	ptreq(e->roll, 5);
	eq(e->isucc, 3);
	ok;
}

static int test_phrase0(void *state) {
	enum parser_error r = parser_parse(state, "phrase:hello there");
	struct history_chart *h;

	eq(r, PARSE_ERROR_NONE);
	h = parser_priv(state);
	require(h);
	require(streq(h->entries->text, "hello there"));
	ok;
}

const char *suite_name = "parse/h-info";
struct test tests[] = {
	{ "chart0", test_chart0 },
	{ "phrase0", test_phrase0 },
	{ NULL, NULL }
};
