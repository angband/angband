/* parse/z-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "types.h"

static int setup(void **state) {
	struct header *h = mem_alloc(sizeof *h);
	h->info_ptr = mem_alloc(sizeof(maxima));
	memset(h->info_ptr, 0, sizeof(maxima));
	*state = h;
	return 0;
}

static int teardown(void *state) {
	struct header *h = state;
	mem_free(h->info_ptr);
	mem_free(h);
	return 0;
}

int test_syntax0(void *state) {
	errr r = parse_z_info("", state);
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

int test_syntax1(void *state) {
	errr r = parse_z_info("D:F:0", state);
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

int test_syntax2(void *state) {
	errr r = parse_z_info("M.F:0", state);
	eq(r, PARSE_ERROR_MISSING_COLON);
	ok;
}

int test_syntax3(void *state) {
	errr r = parse_z_info("M:", state);
	eq(r, PARSE_ERROR_MISSING_FIELD);
	ok;
}

int test_syntax4(void *state) {
	errr r = parse_z_info("M:F.0", state);
	eq(r, PARSE_ERROR_MISSING_COLON);
	ok;
}

int test_syntax5(void *state) {
	errr r = parse_z_info("M:F:", state);
	eq(r, PARSE_ERROR_MISSING_FIELD);
	ok;
}

int test_syntax6(void *state) {
	errr r = parse_z_info("M:F:a", state);
	eq(r, PARSE_ERROR_NOT_NUMBER);
	ok;
}

static int test_negative(void *state) {
	errr r = parse_z_info("M:F:-1", state);
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_null0(void *state) {
	errr r = parse_z_info(NULL, state);
	eq(r, PARSE_ERROR_INTERNAL);
	ok;
}

static int test_null1(void *state) {
	errr r = parse_z_info("M:F:1", NULL);
	eq(r, PARSE_ERROR_INTERNAL);
	ok;
}

static int test_badmax(void *state) {
	errr r = parse_z_info("M:D:1", state);
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

#define test_max(l,u) \
	static int test_##l(void *s) { \
		struct header *h = s; \
		maxima *m = h->info_ptr; \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "M:%c:%d", u, __LINE__); \
		r = parse_z_info(buf, s); \
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
test_max(fake_name_size, 'N');
test_max(fake_text_size, 'T');

static const char *suite_name = "parse/z-info";
static struct test tests[] = {
	{ "syntax0", test_syntax0 },
	{ "syntax1", test_syntax1 },
	{ "syntax2", test_syntax2 },
	{ "syntax3", test_syntax3 },
	{ "syntax4", test_syntax4 },
	{ "syntax5", test_syntax5 },
	{ "syntax6", test_syntax6 },
	{ "negative", test_negative },
	{ "null0", test_null0 },
	{ "null1", test_null1 },
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
	{ "fake-name-size", test_fake_name_size },
	{ "fake-text-size", test_fake_text_size },
	{ NULL, NULL }
};
