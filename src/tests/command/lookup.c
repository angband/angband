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
#include "keymap.h"
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
int test_cmd_lookup_orig(void *state) {
	require(cmd_lookup('Z', KEYMAP_MODE_ORIG) == CMD_NULL);
	require(cmd_lookup('{', KEYMAP_MODE_ORIG) == CMD_INSCRIBE);
	require(cmd_lookup('u', KEYMAP_MODE_ORIG) == CMD_USE_STAFF);
	require(cmd_lookup('T', KEYMAP_MODE_ORIG) == CMD_TUNNEL);
	require(cmd_lookup('g', KEYMAP_MODE_ORIG) == CMD_PICKUP);
	require(cmd_lookup('G', KEYMAP_MODE_ORIG) == CMD_STUDY_BOOK);
	require(cmd_lookup(KTRL('S'), KEYMAP_MODE_ORIG) == CMD_SAVE);
	require(cmd_lookup('+', KEYMAP_MODE_ORIG) == CMD_ALTER);
	
	ok;
}

/* Introduced after commit 8871070 added modes to cmd_lookup() calls */
int test_cmd_lookup_rogue(void *state) {
	require(cmd_lookup('{', KEYMAP_MODE_ROGUE) == CMD_INSCRIBE);
	require(cmd_lookup('Z', KEYMAP_MODE_ROGUE) == CMD_USE_STAFF);
	require(cmd_lookup(KTRL('T'), KEYMAP_MODE_ROGUE) == CMD_TUNNEL);
	require(cmd_lookup('g', KEYMAP_MODE_ROGUE) == CMD_PICKUP);
	require(cmd_lookup('G', KEYMAP_MODE_ROGUE) == CMD_STUDY_BOOK);
	require(cmd_lookup(KTRL('S'), KEYMAP_MODE_ROGUE) == CMD_SAVE);
	require(cmd_lookup('+', KEYMAP_MODE_ROGUE) == CMD_ALTER);
	
	ok;
}

const char *suite_name = "command/lookup";
struct test tests[] = {
	{ "cmd_lookup_orig",  test_cmd_lookup_orig },
	{ "cmd_lookup_rogue", test_cmd_lookup_rogue },
	{ NULL, NULL }
};
