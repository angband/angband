/* player/history */

#include "player.h"
#include "player-birth.h"
#include "unit-test.h"

NOTEARDOWN

static struct history_chart ca;
static struct history_chart cb;
static struct history_chart cc;

static struct history_entry ea0;
static char ea0_text[8] = "A0";
static struct history_entry ea1;
static char ea1_text[8] = "A1";
static struct history_entry eb0;
static char eb0_text[8] = "B0";
static struct history_entry eb1;
static char eb1_text[8] = "B1";
static struct history_entry ec0;
static char ec0_text[8] = "C0";
static struct history_entry ec1;
static char ec1_text[8] = "C1";

int setup_tests(void **state) {
	ca.entries = &ea0;
	cb.entries = &eb0;
	cc.entries = &ec0;

	ea0.next = &ea1;
	ea0.succ = &cb;
	ea0.roll = 50;
	ea0.text = ea0_text;

	ea1.next = NULL;
	ea1.succ = &cc;
	ea1.roll = 100;
	ea1.text = ea1_text;

	eb0.next = &eb1;
	eb0.succ = &cc;
	eb0.roll = 50;
	eb0.text = eb0_text;

	eb1.next = NULL;
	eb1.succ = NULL;
	eb1.roll = 100;
	eb1.text = eb1_text;

	ec0.next = &ec1;
	ec0.succ = NULL;
	ec0.roll = 50;
	ec0.text = ec0_text;

	ec1.next = NULL;
	ec1.succ = NULL;
	ec1.roll = 100;
	ec1.text = ec1_text;

	return 0;
}

static int test_0(void *state) {
	int i;
	for (i = 0; i < 100; i++) {
		char *h = get_history(&ca);
		assert(h);
		eq(h[0], 'A');
		require(isdigit(h[1]));
		require(h[2] == 'B' || h[2] == 'C');
		require(isdigit(h[3]));
		if (h[2] == 'B' && h[4]) {
			require(h[4] == 'C');
			require(isdigit(h[5]));
		}
		string_free(h);
	}

	ok;
}

const char *suite_name = "player/history";
struct test tests[] = {
	{ "0", test_0 },
	{ NULL, NULL },
};
