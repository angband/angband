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

static enum parser_error ignored(struct parser *p) {
	(void)p;
	return PARSE_ERROR_NONE;
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

static int test_priv(void *state) {
	requireeq(parser_priv(state), NULL);
	parser_setpriv(state, (void*)0x42);
	requireeq(parser_priv(state), (void*)0x42);
	ok;
}

static int test_reg0(void *state) {
	errr r = parser_reg(state, "", ignored);
	requireeq(r, -EINVAL);
	ok;
}

static int test_reg1(void *state) {
	errr r = parser_reg(state, " ", ignored);
	requireeq(r, -EINVAL);
	ok;
}

static int test_reg2(void *state) {
	errr r = parser_reg(state, "abc int", ignored);
	requireeq(r, -EINVAL);
	ok;
}

static int test_reg3(void *state) {
	errr r = parser_reg(state, "abc notype name", ignored);
	requireeq(r, -EINVAL);
	ok;
}

static int test_reg_int(void *state) {
	errr r = parser_reg(state, "test-reg-int int foo", ignored);
	requireeq(r, 0);
	ok;
}

static int test_reg_sym(void *state) {
	errr r = parser_reg(state, "test-reg-sym sym bar", ignored);
	requireeq(r, 0);
	ok;
}

static int test_reg_str(void *state) {
	errr r = parser_reg(state, "test-reg-str str baz", ignored);
	requireeq(r, 0);
	ok;
}

static enum parser_error helper_sym0(struct parser *p) {
	const char *s = parser_getsym(p, "foo");
	int *wasok = parser_priv(p);
	if (!s || strcmp(s, "bar"))
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

static int test_sym0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-sym0 sym foo", ignored);
	requireeq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-sym0:bar");
	requireeq(wasok, 1);
	requireeq(r, PARSE_ERROR_NONE);
	ok;
}

static const char *suite_name = "parse/parser";
static struct test tests[] = {
	{ "priv", test_priv },
	{ "reg0", test_reg0 },
	{ "reg1", test_reg1 },
	{ "reg2", test_reg2 },
	{ "reg3", test_reg3 },
	{ "reg-int", test_reg_int },
	{ "reg-sym", test_reg_sym },
	{ "reg-str", test_reg_str },

	{ "blank", test_blank },
	{ "spaces", test_spaces },
	{ "comment0", test_comment0 },
	{ "comment1", test_comment1 },

	{ "sym0", test_sym0 },

	{ NULL, NULL }
};
