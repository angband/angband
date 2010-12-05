/* parse/z-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "types.h"

static int setup(void **state) {
	*state = init_parse_z();
	return !*state;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_negative(void *state) {
	errr r = parser_parse(state, "M:F:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_badmax(void *state) {
	errr r = parser_parse(state, "M:D:1");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

#define test_max(l,u) \
	static int test_##l(void *s) { \
		maxima *m = parser_priv(s); \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "M:%c:%d", u, __LINE__); \
		r = parser_parse(s, buf); \
		eq(m->l, __LINE__); \
		eq(r, 0); \
		ok; \
	}

test_max(f_max, 'F');
test_max(k_max, 'K');
test_max(a_max, 'A');
test_max(e_max, 'E');
test_max(r_max, 'R');
test_max(v_max, 'V');
test_max(p_max, 'P');
test_max(c_max, 'C');
test_max(h_max, 'H');
test_max(b_max, 'B');
test_max(s_max, 'S');
test_max(o_max, 'O');
test_max(m_max, 'M');

test_max(flavor_max, 'L');

static const char *suite_name = "parse/z-info";
static struct test tests[] = {
	{ "negative", test_negative },
	{ "fmax", test_f_max },
	{ "kmax", test_k_max },
	{ "amax", test_a_max },
	{ "emax", test_e_max },
	{ "rmax", test_r_max },
	{ "vmax", test_v_max },
	{ "pmax", test_p_max },
	{ "cmax", test_c_max },
	{ "hmax", test_h_max },
	{ "bmax", test_b_max },
	{ "smax", test_s_max },
	{ "omax", test_o_max },
	{ "mmax", test_m_max },
	{ "flavormax", test_flavor_max },
	{ NULL, NULL }
};
