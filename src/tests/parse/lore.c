/* parse/lure.c */

#include "unit-test.h"
#include "test-utils.h"

#include <stdio.h>
#include "init.h"
#include "mon-init.h"

int setup_tests(void **state) {
	set_file_paths();
	init_angband();

	return 0;
}

int teardown_tests(void *state) {
	cleanup_angband();
	return 0;
}

static errr run_parse_monster(struct parser *p) {
	return parse_file(p, "monster");
}

static errr finish_parse_monster(struct parser *p) {
	return 0;
}

static int test_lore_parse_monster_text(void *state) {
	struct file_parser test_lore_parser = lore_parser;
	errr err;

	test_lore_parser.run = run_parse_monster;
	test_lore_parser.finish = finish_parse_monster;
	err = run_parser(&test_lore_parser);

	eq(err, PARSE_ERROR_NONE);

	ok;
}


const char *suite_name = "parse/lore";
struct test tests[] = {
	{ "lore_parse_monster_text", test_lore_parse_monster_text },
	{ NULL, NULL }
};
