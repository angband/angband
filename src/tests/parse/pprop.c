/* parse/pprop */
/* Exercise parsing used for player_property.txt. */

#include "unit-test.h"
#include "init.h"
#include "object.h"
#include "player.h"
#include "project.h"
#include "z-form.h"
#include "z-virt.h"
#include <ctype.h>

static const char *elem_names[] = {
	#define ELEM(a) #a,
	#include "list-elements.h"
	#undef ELEM
};

int setup_tests(void **state) {
	int i;

	*state = player_property_parser.init();
	/*
	 * Set up enough of the projections array to satisfy how it is used for
	 * elemental properties.
	 */
	projections = mem_zalloc(ELEM_MAX * sizeof(*projections));
	for (i = 0; i < ELEM_MAX; ++i) {
		char *cursor;

		projections[i].name = string_make(elem_names[i]);
		cursor = projections[i].name;
		while (*cursor) {
			*cursor = tolower(*cursor);
			++cursor;
		}
	}
	return !*state;
}

int teardown_tests(void *state) {
	struct {
		const char *src, *ptype, *name, *desc, *idx_str; int idx, val; bool repeated;
	} expected[] = {
		{
			"test_complete_player0",
			"player",
			"Full Spellcaster",
			"You may obtain a perfect success rate with magic.",
			"PF_ZERO_FAIL",
			PF_ZERO_FAIL,
			0,
			false
		},
		{
			"test_complete_object0",
			"object",
			"Sustain Strength",
			"Your strength is sustained",
			"OF_SUST_STR",
			OF_SUST_STR,
			0,
			false,
		},
		{
			"test_complete_element0",
			"element",
			"Vulnerability",
			"You are vulnerable to",
			"ELEM_",
			0,
			-1,
			true
		}
	};
	struct parser *p = (struct parser*) state;
	int r = 0;
	char buffer[320];
	const struct player_ability *a;
	int i;

	if (player_property_parser.finish(p)) {
		r = 1;
	}
	a = player_abilities;
	for (i = 0; i < (int) N_ELEMENTS(expected); ++i) {
		int jn = (expected[i].repeated) ? ELEM_MAX : 1;
		int j;

		for (j = 0; j < jn;  ++j) {
			if (a) {
				if (a->index != expected[i].idx + j) {
					r = 1;
					if (verbose) {
						printf("    %s:%d: code from %s() did not make it through\n", suite_name, __LINE__ - 3, expected[i].src);
						printf("      got %d\n", a->index);
						if (expected[i].repeated) {
							printf("      expected %d (%s%s)\n", expected[i].idx + j, expected[i].idx_str, elem_names[j]);
						} else {
							printf("      expected %d (%s)\n", expected[i].idx + j, expected[i].idx_str);
						}
					}
				}
				if (!streq(a->type, expected[i].ptype)) {
					r = 1;
					if (verbose) {
						printf("    %s:%d: type from %s() did not make it through\n", suite_name, __LINE__ - 3, expected[i].src);
						printf("      got \"%s\"\n", a->type);
						printf("      expected \"%s\"\n", expected[i].ptype);
					}
				}
				if (expected[i].repeated) {
					char *cap_name = string_make(
						projections[j].name);
					size_t nc;

					my_strcap(cap_name);
					nc = strnfmt(buffer,
						sizeof(buffer), "%s %s",
						cap_name, expected[i].name);
					if (nc >= sizeof(buffer) - 1) {
						r = 1;
						if (verbose) {
							printf("    %s:%d: due to an insufficient buffer, could not check name from %s() for ability (%d of %d)\n", suite_name, __LINE__ - 3, expected[i].src, j + 1, ELEM_MAX);
						}
					} else if (!streq(a->name, buffer)) {
						r = 1;
						if (verbose) {
							printf("    %s:%d: name from %s() did not make it through to ability (%d of %d)\n", suite_name, __LINE__ - 3, expected[i].src, j + 1, ELEM_MAX);
							printf("      got \"%s\"\n", a->name);
							printf("      expected \"%s\"\n", buffer);
						}
					}
					string_free(cap_name);
					nc = strnfmt(buffer,
						sizeof(buffer), "%s %s.",
						expected[i].desc,
						projections[j].name);
					if (nc >= sizeof(buffer) - 1) {
						r = 1;
						if (verbose) {
							printf("    %s:%d: due to an insufficient buffer, could not check desc from %s() for ability (%d of %d)\n", suite_name, __LINE__ - 3, expected[i].src, j + 1, ELEM_MAX);
						}
					} else if (!streq(a->desc, buffer)) {
						r = 1;
						if (verbose) {
							printf("    %s:%d: desc from %s() did not make it through to ability (%d of %d)\n", suite_name, __LINE__ - 3, expected[i].src, j + 1, ELEM_MAX);
							printf("      got \"%s\"\n", a->desc);
							printf("      expected \"%s\"\n", buffer);
						}
					}
				} else {
					if (!streq(a->name, expected[i].name)) {
						r = 1;
						if (verbose) {
							printf("    %s:%d: name from %s() did not make it through\n", suite_name, __LINE__ - 3, expected[i].src);
							printf("      got \"%s\"\n", a->name);
							printf("      expected \"%s\"\n", expected[i].name);
						}
					}
					if (!streq(a->desc, expected[i].desc)) {
						r = 1;
						if (verbose) {
							printf("    %s:%d: desc from %s() did not make it through\n", suite_name, __LINE__ - 3, expected[i].src);
							printf("      got \"%s\"\n", a->desc);
							printf("      expected \"%s\"\n", expected[i].desc);
						}
					}
				}
				if (a->value != expected[i].val) {
					r = 1;
					if (verbose) {
						printf("    %s:%d: value from %s() did not get through\n", suite_name, __LINE__ - 3, expected[i].src);
						printf("      got %d\n", a->value);
						printf("      expected %d\n", expected[i].val);
					}
				}
				a = a->next;
			} else {
				r = 1;
				if (verbose) {
					if (expected[i].repeated) {
						printf("    %s:%d: missing ability (%d of %d) generated from %s()\n", suite_name, __LINE__ - 3, j + 1, ELEM_MAX, expected[i].src);
					} else {
						printf("    %s:%d: missing ability from %s()\n", suite_name, __LINE__ - 5, expected[i].src);
					}
				}
			}
		}
	}
	player_property_parser.cleanup();
	for (i = 0; i < ELEM_MAX; ++i) {
		string_free(projections[i].name);
	}
	mem_free(projections);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	null(parser_priv(p));
	/* Without a preceding type directive, all these should fail. */
	r = parser_parse(p, "code:ZERO_FAIL");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:You are made of rock.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "name:Rock");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "value:3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "bindui:test_ui:0:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_complete_player0(void *state) {
	const char *lines[] = {
		"type:player",
		"code:ZERO_FAIL",
		"name:Full Spellcaster",
		"desc:You may obtain a perfect ",
		"desc:success rate with magic."
	};
	struct parser *p = (struct parser*) state;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	ok;
}

static int test_complete_object0(void *state) {
	const char *lines[] = {
		"type:object",
		"code:SUST_STR",
		"bindui:test_ui:1:special",
		"name:Sustain Strength",
		"desc:Your strength is sustained",
	};
	struct parser *p = (struct parser*) state;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	ok;
}

static int test_complete_element0(void *state) {
	const char *lines[] = {
		"type:element",
		"bindui:test_ui:0:-1",
		"name:Vulnerability",
		"desc:You are vulnerable to",
		"value:-1",
	};
	struct parser *p = (struct parser*) state;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	ok;
}

static int test_name_memleak0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a property */
	enum parser_error r = parser_parse(p, "type:player");

	eq(r, PARSE_ERROR_NONE);
	/* Try setting the name twice to see if memory is leaked. */
	r = parser_parse(p, "name:First Name");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "name:Second Name");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_code_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* First try a property of type player with an invalid code. */
	enum parser_error r = parser_parse(p, "type:player");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:XYZZY");
	eq(r, PARSE_ERROR_INVALID_PLAY_PROP_CODE);
	/* Try a property of type object with an invalid code. */
	r = parser_parse(p, "type:object");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:XYZZY");
	eq(r, PARSE_ERROR_INVALID_PLAY_PROP_CODE);
	/*
	 * Then try with a property of a type that's not recognized; that'll
	 * be detected when trying to set the code.
	 */
	r = parser_parse(p, "type:xyzzy");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:SUST_STR");
	eq(r, PARSE_ERROR_INVALID_PLAY_PROP_CODE);
	ok;
}

static int test_bindui_bad0(void *state)
{
	struct parser *p = (struct parser*) state;
	/* Set up an empty property. */
	enum parser_error r = parser_parse(p, "type:player");

	eq(r, PARSE_ERROR_NONE);
	/*
	 * Try binding with values that are not "special" nor an integer
	 * that is in bounds.
	 */
	r = parser_parse(p, "bindui:test_ui:0:xyzzy");
	eq(r, PARSE_ERROR_NOT_NUMBER);
	r = parser_parse(p, "bindui:test_ui:1:18446744073709551615");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "bindui:test_ui:1:-18446744073709551615");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

const char *suite_name = "parse/pprop";
/*
 * test_missing_record_header0() has to be before test_complete_player0(),
 * test_complete_object0(), and test_complete_element0().  The checks in
 * teardown_tests() assume test_complete_player0(), test_complete_object0(),
 * and test_complete_element0() are run in that order and are before
 * test_name_memleak0(), test_code_bad0(), and test_bindui_bad0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "complete_player0", test_complete_player0 },
	{ "complete_object0", test_complete_object0 },
	{ "complete_element0", test_complete_element0 },
	{ "name_memleak0", test_name_memleak0 },
	{ "code_bad0", test_code_bad0 },
	{ "bindui_bad0", test_bindui_bad0 },
	{ NULL, NULL }
};
