/** @file main-test.c
 *  @brief Pseudo-UI for end-to-end testing.
 *  @author Elly <elly+angband@leptoquark.net>
 */

#include "angband.h"

static int verbose = 0;

static void c_quit(char *rest) {
	quit(NULL);
}

static void c_verbose(char *rest) {
	if (!strcmp(rest, "0")) {
		printf("cmd-verbose: off\n");
		verbose = 0;
	} else {
		printf("cmd-verbose: on\n");
		verbose = 1;
	}
}

static void c_version(char *rest) {
	printf("cmd-version: %s %s\n", VERSION_NAME, VERSION_STRING);
}

typedef struct {
	const char *name;
	void (*func)(char *args);
} test_cmd;

static test_cmd cmds[] = {
	{ "quit", c_quit },
	{ "verbose", c_verbose },
	{ "version", c_version },
	{ NULL, NULL }
};

static errr test_docmd(void) {
	char buf[1024];
	char *cmd;
	char *rest;
	int i;

	printf("test> ");
	fflush(stdout);
	if (!fgets(buf, sizeof(buf), stdin)) {
		return -1;
	}
	if (strchr(buf, '\n')) {
		*strchr(buf, '\n') = '\0';
	}

	if (verbose) printf("test-docmd: %s\n", buf);
	cmd = strtok(buf, " ");
	rest = strtok(NULL, "");

	for (i = 0; cmds[i].name; i++) {
		if (!strcmp(cmds[i].name, cmd)) {
			cmds[i].func(rest);
			return 0;
		}
	}

	return 0;
}

typedef struct term_data term_data;
struct term_data {
	term t;
};

static term_data td;
typedef struct {
	int key;
	errr (*func)(int v);
} term_xtra_func;

static void term_init_test(term *t) {
	if (verbose) printf("term-init %s %s\n", VERSION_NAME, VERSION_STRING);
}

static void term_nuke_test(term *t) {
	if (verbose) printf("term-end\n");
}

static errr term_user_test(int n) {
	if (verbose) printf("term-user %d\n", n);
	return 0;
}

static errr term_xtra_clear(int v) {
	if (verbose) printf("term-xtra-clear %d\n", v);
	return 0;
}

static errr term_xtra_noise(int v) {
	if (verbose) printf("term-xtra-noise %d\n", v);
	return 0;
}

static errr term_xtra_fresh(int v) {
	if (verbose) printf("term-xtra-fresh %d\n", v);
	return 0;
}

static errr term_xtra_shape(int v) {
	if (verbose) printf("term-xtra-shape %d\n", v);
	return 0;
}

static errr term_xtra_alive(int v) {
	if (verbose) printf("term-xtra-alive %d\n", v);
	return 0;
}

static errr term_xtra_event(int v) {
	if (verbose) printf("term-xtra-event %d\n", v);
	return test_docmd();
}

static errr term_xtra_flush(int v) {
	if (verbose) printf("term-xtra-flush %d\n", v);
	return 0;
}

static errr term_xtra_delay(int v) {
	if (verbose) printf("term-xtra-delay %d\n", v);
	return 0;
}

static errr term_xtra_react(int v) {
	if (verbose) printf("term-xtra-react\n");
	return 0;
}

static term_xtra_func xtras[] = {
	{ TERM_XTRA_CLEAR, term_xtra_clear },
	{ TERM_XTRA_NOISE, term_xtra_noise },
	{ TERM_XTRA_FRESH, term_xtra_fresh },
	{ TERM_XTRA_SHAPE, term_xtra_shape },
	{ TERM_XTRA_ALIVE, term_xtra_alive },
	{ TERM_XTRA_EVENT, term_xtra_event },
	{ TERM_XTRA_FLUSH, term_xtra_flush },
	{ TERM_XTRA_DELAY, term_xtra_delay },
	{ TERM_XTRA_REACT, term_xtra_react },
	{ 0, NULL },
};

static errr term_xtra_test(int n, int v) {
	int i;
	for (i = 0; xtras[i].func; i++) {
		if (xtras[i].key == n) {
			return xtras[i].func(v);
		}
	}
	if (verbose) printf("term-xtra-unknown %d %d\n", n, v);
	return 0;
}

static errr term_curs_test(int x, int y) {
	if (verbose) printf("term-curs %d %d\n", x, y);
	return 0;
}

static errr term_wipe_test(int x, int y, int n) {
	if (verbose) printf("term-wipe %d %d %d\n", x, y, n);
	return 0;
}

static errr term_text_test(int x, int y, int n, byte a, const char *s) {
	if (verbose) printf("term-text %d %d %d %02x %s\n", x, y, n, a, s);
	return 0;
}

static void term_data_link(int i) {
	term *t = &td.t;

	term_init(t, 80, 24, 256);

	t->init_hook = term_init_test;
	t->nuke_hook = term_nuke_test;

	t->user_hook = term_user_test;
	t->xtra_hook = term_xtra_test;
	t->curs_hook = term_curs_test;
	t->wipe_hook = term_wipe_test;
	t->text_hook = term_text_test;

	t->data = &td;

	Term_activate(t);

	angband_term[i] = t;
}

const char help_test[] = "Pseudo-UI for end-to-end tests.";

errr init_test(int argc, char *argv[]) {
	term_data_link(0);
	return 0;
}
