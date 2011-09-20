/* z-quark/quark.c */

#include "unit-test.h"
#include "z-textblock.h"
#include "z-term.h"

int setup_tests(void **state) {
	ok;
}

int teardown_tests(void *state) {
	ok;
}

int test_alloc(void *state) {
	textblock *tb = textblock_new();

	require(tb);

	textblock_free(tb);

	ok;
}

int test_append(void *state) {
	textblock *tb = textblock_new();

	require(!wcscmp(textblock_text(tb), L""));

	textblock_append(tb, "Hello");
	require(!wcscmp(textblock_text(tb), L"Hello"));

	textblock_append(tb, "%d", 20);
	require(!wcscmp(textblock_text(tb), L"Hello20"));

	ok;
}

int test_colour(void *state) {
	textblock *tb = textblock_new();

	const char text[] = "two";
	const byte attrs[] = { TERM_L_GREEN, TERM_L_GREEN, TERM_L_GREEN };	

	textblock_append_c(tb, TERM_L_GREEN, text);

	require(!memcmp(textblock_attrs(tb), attrs, 3));

	ok;
}

int test_length(void *state) {
	textblock *tb = textblock_new();

	const char text[] = "1234567";
	const wchar_t test_text[] = L"1234567";
	int i;

	const wchar_t *tb_text;

	/* Add it 32 times to make sure that appending definitely works */
	for (i = 0; i < 32; i++) {
		textblock_append(tb, text);
	}

	/* Now make sure it's all right */
	tb_text = textblock_text(tb);
	for (i = 0; i < 32; i++) {
		int n = N_ELEMENTS(text) - 1;
		int offset = i * n;

	 	require(!wmemcmp(tb_text + offset, test_text, n));
	}

	ok;
}

const char *suite_name = "z-textblock/textblock";
struct test tests[] = {
	{ "alloc", test_alloc },
	{ "append", test_append },
	{ "colour", test_colour },
	{ "length", test_length },
	{ NULL, NULL }
};
