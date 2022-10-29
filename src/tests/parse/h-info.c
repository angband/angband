/* parse/h-info */
/* Exercise parsing used for history.txt. */

#include "unit-test.h"

#include "init.h"
#include "player.h"

int setup_tests(void **state) {
	*state = history_parser.init();
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	struct history_chart *h = parser_priv(p);

	string_free(h->entries->text);
	mem_free(h->entries);
	mem_free(h);
	parser_destroy(p);
	return 0;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct history_chart *c = (struct history_chart*) parser_priv(p);
	enum parser_error r;

	null(c);
	r = parser_parse(p, "phrase:hello there");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_chart0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "chart:1:3:5");
	struct history_chart *c;
	struct history_entry *e;

	eq(r, PARSE_ERROR_NONE);
	c = (struct history_chart*) parser_priv(p);
	notnull(c);
	e = c->entries;
	notnull(e);
	eq(c->idx, 1);
	null(e->next);
	eq(e->isucc, 3);
	eq(e->roll, 5);
	ok;
}

static int test_phrase0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "phrase:hello ");
	struct history_chart *h;

	eq(r, PARSE_ERROR_NONE);
	/*
	 * Check that multiple phrase directives within the same record are
	 * appended.
	 */
	r = parser_parse(p, "phrase:there");
	eq(r, PARSE_ERROR_NONE);
	h = (struct history_chart*) parser_priv(p);
	notnull(h);
	notnull(h->entries);
	notnull(h->entries->text);
	require(streq(h->entries->text, "hello there"));
	ok;
}

const char *suite_name = "parse/h-info";
/*
 * test_missing_record_header0() has to be before test_chart0().  test_phrase0()
 * has to be after test_char0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "chart0", test_chart0 },
	{ "phrase0", test_phrase0 },
	{ NULL, NULL }
};
