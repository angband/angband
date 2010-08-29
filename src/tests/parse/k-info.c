/* parse/k-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "types.h"

#define MAXOBJS 5

static int setup(void **state) {
	struct header *h = mem_alloc(sizeof *h);
	memset(h, 0, sizeof(*h));
	h->info_ptr = mem_alloc(MAXOBJS * sizeof(struct object_kind));
	h->info_num = MAXOBJS;
	*state = h;
	return 0;
}

static int teardown(void *state) {
	struct header *h = state;
	mem_free(h->info_ptr);
	mem_free(h);
	return 0;
}

#define test_error(n, s, g) \
	static int test_##n(void *state) { \
		errr r = parse_k_info(s, state); \
		requireeq(r, g); \
		ok; \
	}

test_error(syntax0, "", PARSE_ERROR_INTERNAL);

test_error(syntax_n0, "N.1:A", PARSE_ERROR_MISSING_COLON);
test_error(syntax_n1, "N:", PARSE_ERROR_MISSING_FIELD);
test_error(syntax_n2, "N:1.A", PARSE_ERROR_MISSING_COLON);
test_error(syntax_n3, "N:1:", PARSE_ERROR_MISSING_FIELD);
test_error(syntax_n4, "N:A:A", PARSE_ERROR_NOT_NUMBER);
test_error(syntax_n5, "N::A", PARSE_ERROR_MISSING_FIELD);
test_error(syntax_n6, "N:1A:A:", PARSE_ERROR_NOT_NUMBER);

test_error(badtype, "Z:haha", PARSE_ERROR_UNDEFINED_DIRECTIVE);

static int test_n0(void *state) {
	struct header *h = state;
	struct object_kind *k = h->info_ptr;
	errr r = parse_k_info("N:1:A Test Object", state);

	requireeq(r, 0);
	requireeq(k[1].kidx, 1);
	require(!strcmp(k[1].name, "A Test Object"));
	ok;
}

static const char *suite_name = "parse/k-info";
static struct test tests[] = {
	{ "syntax", test_syntax0 },

	{ "syntax-n0", test_syntax_n0 },
	{ "syntax-n1", test_syntax_n1 },
	{ "syntax-n2", test_syntax_n2 },
	{ "syntax-n3", test_syntax_n3 },
	{ "syntax-n4", test_syntax_n4 },
	{ "syntax-n5", test_syntax_n5 },
	{ "syntax-n6", test_syntax_n6 },

	{ "n0", test_n0 },

	{ "badtype", test_badtype },

	{ NULL, NULL }
};
