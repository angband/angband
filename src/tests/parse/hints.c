/* parse/hints */
/* Exercise parsing used for hints.txt. */

#include "unit-test.h"
#include "init.h"
#include "hint.h"

NOSETUP
NOTEARDOWN

static int test_complete0(void *state) {
	const char *lines[] = {
		"H:+10 speed means you move twice as fast as normal.",
		"H:The stores change their stock every so often.",
		"H:Never be too afraid to run away."
	};
	struct parser *p = hints_parser.init();
	struct hint *h;
	int i, rtn;

	notnull(p);
	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	rtn = hints_parser.finish(p);
	eq(rtn, 0);

	h = hints;
	for (i = (int) N_ELEMENTS(lines) - 1; i >= 0; --i) {
		notnull(h);
		notnull(h->hint);
		require(streq(h->hint, lines[i] + 2));
		h = h->next;
	}

	hints_parser.cleanup();
	ok;
}

const char *suite_name = "parse/hints";
struct test tests[] = {
	{ "complete0", test_complete0 },
	{ NULL, NULL }
};
