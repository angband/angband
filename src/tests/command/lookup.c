/* command/lookup
 *
 * Tests for command lookup
 *
 * Created by: myshkin
 *             1 May 2011
 */

#include "unit-test.h"
#include "object/obj-flag.h"
#include "object/object.h"
#include "cmds.h"
#include "ui-event.h"
#include "z-virt.h"

int setup_tests(void **state) {
	cmd_init();
	*state = 0;
	return 0;
}

int teardown_tests(void *state) {
	mem_free(state);
	return 0;
}

/* Regression test for #1330 */
int test_cmd_lookup(void *state) {
	require(cmd_lookup('Z') == CMD_NULL);
	require(cmd_lookup('{') == CMD_INSCRIBE);
	require(cmd_lookup('u') == CMD_USE_STAFF);
	require(cmd_lookup('T') == CMD_TUNNEL);
	require(cmd_lookup('g') == CMD_PICKUP);
	require(cmd_lookup('G') == CMD_STUDY_BOOK);
	require(cmd_lookup(KTRL('S')) == CMD_SAVE);
	require(cmd_lookup('+') == CMD_ALTER);
	
	ok;
}

const char *suite_name = "command/lookup";
struct test tests[] = {
	{ "cmd_lookup", test_cmd_lookup },
	{ NULL, NULL }
};
