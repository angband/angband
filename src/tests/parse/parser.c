/* parse/parser */

#include "unit-test.h"

#include "parser.h"

static int setup(void **state) {
	struct parser *p = parser_new();
	if (!p)
		return 1;

	*state = p;
	return 0;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_blank(void *state) {
	requireeq(parser_parse(state, ""), PARSE_ERROR_NONE);
	ok;
}

static int test_spaces(void *state) {
	requireeq(parser_parse(state, "   "), PARSE_ERROR_NONE);
	ok;
}

static int test_comment0(void *state) {
	requireeq(parser_parse(state, "# foo"), PARSE_ERROR_NONE);
	ok;
}

static int test_comment1(void *state) {
	requireeq(parser_parse(state, "  # bar"), PARSE_ERROR_NONE);
	ok;
}

static const char *suite_name = "parse/parser";
static struct test tests[] = {
	{ "blank", test_blank },
	{ "spaces", test_spaces },
	{ "comment0", test_comment0 },
	{ "comment1", test_comment1 },
	{ NULL, NULL }
};
