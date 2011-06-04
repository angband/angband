/* parse/parser */

#include "unit-test.h"

#include "parser.h"

int setup_tests(void **state) {
	struct parser *p = parser_new();
	if (!p)
		return 1;

	*state = p;
	return 0;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_blank(void *state) {
	eq(parser_parse(state, ""), PARSE_ERROR_NONE);
	ok;
}

int test_spaces(void *state) {
	eq(parser_parse(state, "   "), PARSE_ERROR_NONE);
	ok;
}

int test_comment0(void *state) {
	eq(parser_parse(state, "# foo"), PARSE_ERROR_NONE);
	ok;
}

int test_comment1(void *state) {
	eq(parser_parse(state, "  # bar"), PARSE_ERROR_NONE);
	ok;
}

int test_priv(void *state) {
	ptreq(parser_priv(state), 0);
	parser_setpriv(state, (void*)0x42);
	ptreq(parser_priv(state), (void*)0x42);
	ok;
}

int test_reg0(void *state) {
	errr r = parser_reg(state, "", ignored);
	eq(r, -EINVAL);
	ok;
}

int test_reg1(void *state) {
	errr r = parser_reg(state, " ", ignored);
	eq(r, -EINVAL);
	ok;
}

int test_reg2(void *state) {
	errr r = parser_reg(state, "abc int", ignored);
	eq(r, -EINVAL);
	ok;
}

int test_reg3(void *state) {
	errr r = parser_reg(state, "abc notype name", ignored);
	eq(r, -EINVAL);
	ok;
}

int test_reg4(void *state) {
	errr r = parser_reg(state, "abc int a ?int b int c", ignored);
	eq(r, -EINVAL);
	ok;
}

int test_reg5(void *state) {
	errr r = parser_reg(state, "abc str foo int bar", ignored);
	eq(r, -EINVAL);
	ok;
}

int test_reg_int(void *state) {
	errr r = parser_reg(state, "test-reg-int int foo", ignored);
	eq(r, 0);
	ok;
}

int test_reg_sym(void *state) {
	errr r = parser_reg(state, "test-reg-sym sym bar", ignored);
	eq(r, 0);
	ok;
}

int test_reg_str(void *state) {
	errr r = parser_reg(state, "test-reg-str str baz", ignored);
	eq(r, 0);
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

int test_sym0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-sym0 sym foo", helper_sym0);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-sym0:bar");
	eq(r, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_sym1(struct parser *p) {
	const char *s = parser_getsym(p, "foo");
	const char *t = parser_getsym(p, "baz");
	int *wasok = parser_priv(p);
	if (!s || !t || strcmp(s, "bar") || strcmp(t, "quxx"))
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_sym1(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-sym1 sym foo sym baz", helper_sym1);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-sym1:bar:quxx");
	eq(r, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_int0(struct parser *p) {
	int s = parser_getint(p, "i0");
	int t = parser_getint(p, "i1");
	int *wasok = parser_priv(p);
	*wasok = (s == 42 && t == 81);
	return PARSE_ERROR_NONE;
}

int test_int0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-int0 int i0 int i1", helper_int0);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-int0:42:81");
	eq(r, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_int1(struct parser *p) {
	int v = parser_getint(p, "i0");
	int *wasok = parser_priv(p);
	*wasok = (v == -3);
	return PARSE_ERROR_NONE;
}

int test_int1(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-int1 int i0", helper_int1);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-int1:-3");
	eq(r, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_str0(struct parser *p) {
	const char *s = parser_getstr(p, "s0");
	int *wasok = parser_priv(p);
	if (!s || strcmp(s, "foo:bar:baz quxx..."))
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_str0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-str0 str s0", helper_str0);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-str0:foo:bar:baz quxx...");
	eq(r, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

int test_syntax0(void *state) {
	struct parser_state s;
	int v;
	errr r = parser_reg(state, "test-syntax0 str s0", ignored);
	eq(r, 0);
	r = parser_parse(state, "test-syntax0");
	eq(r, PARSE_ERROR_MISSING_FIELD);
	v = parser_getstate(state, &s);
	require(v);
	eq(s.line, 5);
	eq(s.col, 2);
	ok;
}

int test_syntax1(void *state) {
	struct parser_state s;
	int v;
	errr r = parser_reg(state, "test-syntax1 int i0", ignored);
	eq(r, 0);
	r = parser_parse(state, "test-syntax1:a");
	eq(r, PARSE_ERROR_NOT_NUMBER);
	v = parser_getstate(state, &s);
	require(v);
	eq(s.line, 6);
	eq(s.col, 2);
	ok;
}

int test_syntax2(void *state) {
	struct parser_state s;
	int v;
	errr r = parser_reg(state, "test-syntax2 int i0 sym s1", ignored);
	eq(r, 0);
	r = parser_parse(state, "test-syntax2::test");
	eq(r, PARSE_ERROR_NOT_NUMBER);
	v = parser_getstate(state, &s);
	require(v);
	eq(s.line, 7);
	eq(s.col, 2);
	ok;
}

int test_baddir(void *state) {
	errr r = parser_parse(state, "test-baddir");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

static enum parser_error helper_rand0(struct parser *p) {
	struct random v = parser_getrand(p, "r0");
	int *wasok = parser_priv(p);
	if (v.dice != 2 || v.sides != 3)
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_rand0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-rand0 rand r0", helper_rand0);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-rand0:2d3");
	eq(r, 0);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_rand1(struct parser *p) {
	struct random v = parser_getrand(p, "r0");
	struct random u = parser_getrand(p, "r1");
	int *wasok = parser_priv(p);
	if (v.dice != 2 || v.sides != 3 || u.dice != 4 || u.sides != 5)
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_rand1(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-rand1 rand r0 rand r1", helper_rand1);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-rand1:2d3:4d5");
	eq(r, 0);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_opt0(struct parser *p) {
	const char *s0 = parser_getsym(p, "s0");
	const char *s1 = parser_hasval(p, "s1") ? parser_getsym(p, "s1") : NULL;
	int *wasok = parser_priv(p);
	if (!s0 || strcmp(s0, "foo"))
		return PARSE_ERROR_GENERIC;
	if (s1 && !strcmp(s1, "bar"))
		*wasok = 2;
	else
		*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_opt0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-opt0 sym s0 ?sym s1", helper_opt0);
	eq(r, 0);
	parser_setpriv(state, &wasok);
	r = parser_parse(state, "test-opt0:foo");
	eq(r, 0);
	eq(wasok, 1);
	require(parser_hasval(state, "s0"));
	require(!parser_hasval(state, "s1"));
	r = parser_parse(state, "test-opt0:foo:bar");
	eq(r, 0);
	eq(wasok, 2);
	require(parser_hasval(state, "s0"));
	require(parser_hasval(state, "s1"));
	ok;
}

static enum parser_error helper_uint0(struct parser *p) {
	unsigned int a = parser_getuint(p, "u0");
	int *wasok = parser_priv(p);
	if (a != 42)
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_uint0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-uint0 uint u0", helper_uint0);
	enum parser_error e;
	eq(r, 0);
	parser_setpriv(state, &wasok);
	e = parser_parse(state, "test-uint0:42");
	eq(e, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;

}

int test_uint1(void *state) {
	errr r = parser_reg(state, "test-uint1 uint u0", ignored);
	enum parser_error e = parser_parse(state, "test-uint1:-2");
	eq(r, 0);
	eq(e, PARSE_ERROR_NOT_NUMBER);
	ok;
}

static enum parser_error helper_char0(struct parser *p) {
	char c = parser_getchar(p, "c");
	int *wasok = parser_priv(p);

	if (c != 'C')
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_char0(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-char0 char c", helper_char0);
	enum parser_error e;
	eq(r, 0);
	parser_setpriv(state, &wasok);
	e = parser_parse(state, "test-char0:C");
	eq(e, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

static enum parser_error helper_char1(struct parser *p) {
	char c0 = parser_getchar(p, "c0");
	char c1 = parser_getchar(p, "c1");
	int i0 = parser_getint(p, "i0");
	const char *s = parser_getstr(p, "s");
	int *wasok = parser_priv(p);

	if (c0 != ':' || c1 != ':' || i0 != 34 || !streq(s, "lala"))
		return PARSE_ERROR_GENERIC;
	*wasok = 1;
	return PARSE_ERROR_NONE;
}

int test_char1(void *state) {
	int wasok = 0;
	errr r = parser_reg(state, "test-char1 char c0 int i0 char c1 str s", helper_char1);
	enum parser_error e;
	eq(r, 0);
	parser_setpriv(state, &wasok);
	e = parser_parse(state, "test-char1:::34:::lala");
	eq(e, PARSE_ERROR_NONE);
	eq(wasok, 1);
	ok;
}

const char *suite_name = "parse/parser";
struct test tests[] = {
	{ "priv", test_priv },
	{ "reg0", test_reg0 },
	{ "reg1", test_reg1 },
	{ "reg2", test_reg2 },
	{ "reg3", test_reg3 },
	{ "reg4", test_reg4 },
	{ "reg5", test_reg5 },
	{ "reg-int", test_reg_int },
	{ "reg-sym", test_reg_sym },
	{ "reg-str", test_reg_str },

	{ "blank", test_blank },
	{ "spaces", test_spaces },
	{ "comment0", test_comment0 },
	{ "comment1", test_comment1 },

	{ "syntax0", test_syntax0 },
	{ "syntax1", test_syntax1 },
	{ "syntax2", test_syntax2 },

	{ "sym0", test_sym0 },
	{ "sym1", test_sym1 },

	{ "int0", test_int0 },
	{ "int1", test_int1 },

	{ "str0", test_str0 },

	{ "rand0", test_rand0 },
	{ "rand1", test_rand1 },

	{ "opt0", test_opt0 },

	{ "uint0", test_uint0 },
	{ "uint1", test_uint1 },

	{ "char0", test_char0 },
	{ "char1", test_char1 },

	{ "baddir", test_baddir },

	{ NULL, NULL }
};
