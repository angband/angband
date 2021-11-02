/* z-quark/quark.c */

#include "unit-test.h"
#include "z-color.h"
#include "z-textblock.h"

int setup_tests(void **state) {
	ok;
}

int teardown_tests(void *state) {
	ok;
}

static int test_alloc(void *state) {
	textblock *tb = textblock_new();

	require(tb);

	textblock_free(tb);

	ok;
}

static int test_append(void *state) {
	textblock *tb = textblock_new();

	require(!wcscmp(textblock_text(tb), L""));

	textblock_append(tb, "Hello");
	require(!wcscmp(textblock_text(tb), L"Hello"));

	textblock_append(tb, "%d", 20);
	require(!wcscmp(textblock_text(tb), L"Hello20"));

	textblock_free(tb);

	ok;
}

static int test_colour(void *state) {
	textblock *tb = textblock_new();

	const char text[] = "two";
	const uint8_t attrs[] = { COLOUR_L_GREEN, COLOUR_L_GREEN, COLOUR_L_GREEN };

	textblock_append_c(tb, COLOUR_L_GREEN, text);

	require(!memcmp(textblock_attrs(tb), attrs, 3));

	textblock_free(tb);

	ok;
}

static int test_length(void *state) {
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

	textblock_free(tb);

	ok;
}

static int test_append_textblock(void *state) {
	const uint8_t attrs[] = { COLOUR_L_BLUE, COLOUR_L_BLUE, COLOUR_L_BLUE,
		COLOUR_L_GREEN, COLOUR_L_GREEN, COLOUR_L_GREEN, COLOUR_L_GREEN };
	textblock *tb1 = textblock_new();
	textblock *tb2 = textblock_new();

	textblock_append_c(tb1, COLOUR_L_BLUE, "Hey");
	textblock_append_c(tb2, COLOUR_L_GREEN, " you");
	textblock_append_textblock(tb1, tb2);
	require(!wcscmp(textblock_text(tb1), L"Hey you"));
	require(!memcmp(textblock_attrs(tb1), attrs, sizeof(attrs)));
	require(!wcscmp(textblock_text(tb2), L" you"));
	require(!memcmp(textblock_attrs(tb2), attrs + 3,
		sizeof(attrs) - 3 * sizeof(*attrs)));

	textblock_free(tb2);
	textblock_free(tb1);

	ok;
}

const char *suite_name = "z-textblock/textblock";
struct test tests[] = {
	{ "alloc", test_alloc },
	{ "append", test_append },
	{ "colour", test_colour },
	{ "length", test_length },
	{ "append_textblock", test_append_textblock },
	{ NULL, NULL }
};
