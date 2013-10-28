/* player/history */

#include "birth.h"
#include "player/types.h"
#include "unit-test.h"

NOTEARDOWN

static struct history_chart ca;
static struct history_chart cb;
static struct history_chart cc;

static struct history_entry ea0;
static struct history_entry ea1;
static struct history_entry eb0;
static struct history_entry eb1;
static struct history_entry ec0;
static struct history_entry ec1;

int setup_tests(void **state) {
	ca.entries = &ea0;
	cb.entries = &eb0;
	cc.entries = &ec0;

	ea0.next = &ea1;
	ea0.succ = &cb;
	ea0.roll = 50;
	ea0.bonus = 0;
	ea0.text = "A0";

	ea1.next = NULL;
	ea1.succ = &cc;
	ea1.roll = 100;
	ea1.bonus = 0;
	ea1.text = "A1";

	eb0.next = &eb1;
	eb0.succ = &cc;
	eb0.roll = 50;
	eb0.bonus = 0;
	eb0.text = "B0";

	eb1.next = NULL;
	eb1.succ = NULL;
	eb1.roll = 100;
	eb1.bonus = 0;
	eb1.text = "B1";

	ec0.next = &ec1;
	ec0.succ = NULL;
	ec0.roll = 50;
	ec0.bonus = 0;
	ec0.text = "C0";

	ec1.next = NULL;
	ec1.succ = NULL;
	ec1.roll = 100;
	ec1.bonus = 0;
	ec1.text = "C1";

	return 0;
}

int test_0(void *state) {
	int i;
	for (i = 0; i < 100; i++) {
		char *h = get_history(&ca, NULL);
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
};
