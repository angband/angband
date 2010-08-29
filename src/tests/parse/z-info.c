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
	require(r == PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

int test_syntax1(void *state) {
	errr r = parse_z_info("D:F:0", state);
	require(r == PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

int test_syntax2(void *state) {
	errr r = parse_z_info("M.F:0", state);
	require(r == PARSE_ERROR_MISSING_COLON);
	ok;
}

int test_syntax3(void *state) {
	errr r = parse_z_info("M:", state);
	require(r == PARSE_ERROR_MISSING_FIELD);
	ok;
}

int test_syntax4(void *state) {
	errr r = parse_z_info("M:F.0", state);
	require(r == PARSE_ERROR_MISSING_COLON);
	ok;
}

int test_syntax5(void *state) {
	errr r = parse_z_info("M:F:", state);
	require(r == PARSE_ERROR_MISSING_FIELD);
	ok;
}

int test_syntax6(void *state) {
	errr r = parse_z_info("M:F:a", state);
	require(r == PARSE_ERROR_NOT_NUMBER);
	ok;
}

static int test_negative(void *state) {
	errr r = parse_z_info("M:F:-1", state);
	require(r == PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_null0(void *state) {
	errr r = parse_z_info(NULL, state);
	requireeq(r, PARSE_ERROR_INTERNAL);
	ok;
}

static int test_null1(void *state) {
	errr r = parse_z_info("M:F:1", NULL);
	requireeq(r, PARSE_ERROR_INTERNAL);
	ok;
}

#define test_max(l,u) \
	static int test_##l##max(void *s) { \
		struct header *h = s; \
		maxima *m = h->info_ptr; \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "M:%c:%d", u, __LINE__); \
		r = parse_z_info(buf, s); \
		requireeq(m->l ## _max, __LINE__); \
		requireeq(r, 0); \
		ok; \
	}

test_max(f, 'F');
test_max(k, 'K');
test_max(a, 'A');
test_max(e, 'E');
test_max(r, 'R');
test_max(v, 'V');
test_max(p, 'P');
test_max(c, 'C');
test_max(h, 'H');
test_max(b, 'B');
test_max(s, 'S');
test_max(o, 'O');
test_max(m, 'M');

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
	{ "fmax", test_fmax },
	{ "kmax", test_kmax },
	{ "amax", test_amax },
	{ "emax", test_emax },
	{ "rmax", test_rmax },
	{ "vmax", test_vmax },
	{ "pmax", test_pmax },
	{ "cmax", test_cmax },
	{ "hmax", test_hmax },
	{ "bmax", test_bmax },
	{ "smax", test_smax },
	{ "omax", test_omax },
	{ "mmax", test_mmax },
	{ NULL, NULL }
};
