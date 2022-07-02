/* z-quark/quark.c */

#include "unit-test.h"
#include "z-quark.h"

int setup_tests(void **state) {
	return 0;
}

int teardown_tests(void *state) {
	return 0;
}

static int test_alloc(void *state) {
	char buffer[64];

	/* Check it functions at all */
	strcpy(buffer, "1234567890");
	utf8_clipto(buffer, 5);
	require(strcmp(buffer, "12345") == 0);

	/* Extremely low input */
	strcpy(buffer, "Test");
	utf8_clipto(buffer, 0);
	require(buffer[0] == 0);

	/* Overly high input */
	strcpy(buffer, "Test");
	utf8_clipto(buffer, 10);
	require(strcmp(buffer, "Test") == 0);

	/* Non-ASCII clipping */
	strcpy(buffer, "Lómin");
	utf8_clipto(buffer, 2);
	require(strcmp(buffer, "Ló") == 0);

	strcpy(buffer, "åéïø");
	utf8_clipto(buffer, 3);
	require(strcmp(buffer, "åéï") == 0);

	ok;
}

static int test_utf8_fskip(void *state) {
	/*
	 * dollar sign (U+0024; 1 byte as UTF-8), cent sign (U+00A2; 2 bytes
	 * as UTF-8)), euro sign (U+20AC; 3 bytes as UTF-8), gothic letter
	 * hwair (U+10348; 4 bytes as UTF-8), and Shavian letter hung
	 * (U+10459; 4 bytes as UTF-8).
	 */
	char buffer[] = "$¢€𐍈𐑙";
	int offsets[] = { 0, 1, 3, 6, 10, 14 };

	require(utf8_fskip(buffer, 1, NULL) == buffer + offsets[1]);
	require(utf8_fskip(buffer, 3, NULL) == buffer + offsets[3]);
	/* Check case when advancing lands at terminating null. */
	require(utf8_fskip(buffer, 5, NULL) == buffer + offsets[5]);
	/* Check case where advancing would go past terminating null. */
	require(utf8_fskip(buffer, 6, NULL) == NULL);
	/*
	 * Advancing zero should do nothing if already at the start of
	 * a character.
	 */
	require(utf8_fskip(buffer + offsets[3], 0, NULL) ==
		buffer + offsets[3]);
	/*
	 * Advancing zero should go to the next character if in the middle
	 * of a character.
	 */
	require(utf8_fskip(buffer + offsets[3] + 1, 0, NULL) ==
		buffer + offsets[4]);
	/*
	 * Check cases where limit is imposed on advance:  limit has no
	 * effect, advance ends on the limit, and advance tries to go
	 * past limit.
	 */
	require(utf8_fskip(buffer, 1, buffer + offsets[3]) ==
		buffer + offsets[1]);
	require(utf8_fskip(buffer, 3, buffer + offsets[3]) ==
		buffer + offsets[3]);
	require(utf8_fskip(buffer, 4, buffer + offsets[3]) == NULL);

	ok;
}

static int test_utf8_rskip(void *state) {
	/*
	 * dollar sign (U+0024; 1 byte as UTF-8), cent sign (U+00A2; 2 bytes
	 * as UTF-8)), euro sign (U+20AC; 3 bytes as UTF-8), gothic letter
	 * hwair (U+10348; 4 bytes as UTF-8), and Shavian letter hung
	 * (U+10459; 4 bytes as UTF-8).
	 */
	char buffer[] = "$¢€𐍈𐑙";
	int offsets[] = { 0, 1, 3, 6, 10, 14 };

	require(utf8_rskip(buffer + offsets[4], 1, buffer) ==
		buffer + offsets[3]);
	require(utf8_rskip(buffer + offsets[4], 3, buffer) ==
		buffer + offsets[1]);
	/* Check case where backtracking ends at the limit. */
	require(utf8_rskip(buffer + offsets[4], 4, buffer) == buffer);
	/* Check case where backtracking would go past the limit. */
	require(utf8_rskip(buffer + offsets[4], 5, buffer) == NULL);
	/*
	 * Backtracking zero should do nothing if already at the start of
	 * a character.
	 */
	require(utf8_rskip(buffer + offsets[4], 0, buffer) ==
		buffer + offsets[4]);
	/*
	 * Backtracking zero should get to the start of the character if
	 * in the middle of that character.
	 */
	require(utf8_rskip(buffer + offsets[4] + 2, 0, buffer) ==
		buffer + offsets[4]);

	ok;
}

static int test_utf32_to_utf8(void *state) {
	/*
	 * dollar sign (U+0024; 1 byte as UTF-8), cent sign (U+00A2; 2 bytes
	 * as UTF-8)), euro sign (U+20AC; 3 bytes as UTF-8), gothic letter
	 * hwair (U+10348; 4 bytes as UTF-8), and Shavian letter hung
	 * (U+10459; 4 bytes as UTF-8).
	 */
	char expected[] = "$¢€𐍈𐑙";
	uint32_t in[] = { 0x0024, 0x00a2, 0x20ac, 0x10348, 0x10459 };
	uint32_t bad_in[5];
	char out[32];
	size_t n_in = sizeof(in) / sizeof(in[0]);
	size_t n_out = sizeof(out) / sizeof(out[0]);
	size_t n_written, n_cnvt;

	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, in, n_in, &n_cnvt);
	require(n_written == strlen(expected) && out[n_written] == 0 &&
		strcmp(out, expected) == 0 && n_cnvt == n_in);
	if (n_written < n_out - 1) {
		require(out[n_written + 1] == 'a');
	}

	/* Check handling when the number of conversions isn't requested. */
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, in, n_in, NULL);
	require(n_written == strlen(expected) && out[n_written] == 0 &&
		strcmp(out, expected) == 0);
	if (n_written < n_out - 1) {
		require(out[n_written + 1] == 'a');
	}

	/* Check that output limit prevents overflow. */
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, 4, in, n_in, &n_cnvt);
	require(n_written == 3 && out[3] == 0 &&
		expected[0] == out[0] && expected[1] == out[1] &&
		expected[2] == out[2] && out[4] == 'a' && n_cnvt == 2);

	/*
	 * Check degenerate cases where the output buffer has zero or one
	 * bytes.
	 */
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, 1, in, n_in, &n_cnvt);
	require(n_written == 0 && out[0] == 0 && out[1] == 'a' && n_cnvt == 0);
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, 0, in, n_in, &n_cnvt);
	require(n_written == 0 && n_cnvt == 0 && out[0] == 'a');

	/* Check handling of invalid code points. */
	bad_in[0] = 0xd800;
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, bad_in, 1, &n_cnvt);
	require(n_written == 0 && out[0] == 0 && out[1] == 'a' && n_cnvt == 0);

	bad_in[0] = 0xda12;
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, bad_in, 1, &n_cnvt);
	require(n_written == 0 && out[0] == 0 && out[1] == 'a' && n_cnvt == 0);

	bad_in[0] = 0xdfff;
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, bad_in, 1, &n_cnvt);
	require(n_written == 0 && out[0] == 0 && out[1] == 'a' && n_cnvt == 0);

	bad_in[0] = 0x110000;
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, bad_in, 1, &n_cnvt);
	require(n_written == 0 && out[0] == 0 && out[1] == 'a' && n_cnvt == 0);

	/*
	 * This is a combined test for the case or prematurely terminated input
	 * and of code points just before or after the invalid ones.
	 */
	bad_in[0] = 0xd799;
	bad_in[1] = 0xe000;
	bad_in[2] = 0x10ffff;
	bad_in[3] = 0;
	bad_in[4] = in[0];
	(void) memset(out, 'a', n_out);
	n_cnvt = 99;
	n_written = utf32_to_utf8(out, n_out, bad_in, 5, &n_cnvt);
	require(n_written == 10 && out[10] == 0 && out[11] == 'a' &&
		n_cnvt == 3);

	ok;
}

static int test_hex_str_to_int(void *state) {
	require(hex_str_to_int("1Ba0") == 0x1ba0);
	require(hex_str_to_int("5z2") == -1);
	ok;
}

static int test_strunescape(void *state) {
	char x_empty[] = "\\x";
	strunescape(x_empty);
	require(!strcmp(x_empty, "\\x"));
	char x_single[] = "\\xa";
	strunescape(x_single);
	require(!strcmp(x_single, "\\xa"));
	char x_test[] = "\\xaD";
	strunescape(x_test);
	require(!strcmp(x_test, "\xaD"));
	char full_test[] = "\\\\test\\z\\xa0\\n4\\xFd";
	strunescape(full_test);
	require(!strcmp(full_test, "\\test\\z\xa0\n4\xfd"));
	ok;
}

const char *suite_name = "z-util/util";
struct test tests[] = {
	{ "utf8_clipto", test_alloc },
	{ "utf8_fskip", test_utf8_fskip },
	{ "utf8_rskip", test_utf8_rskip },
	{ "utf32_to_utf8", test_utf32_to_utf8 },
	{ "hex_str_to_int", test_hex_str_to_int },
	{ "strunescape", test_strunescape },
	{ NULL, NULL }
};
