/**
 * \file main-test.c
 * \brief Pseudo-UI for end-to-end testing.
 *
 * Copyright (c) 2011 Elly <elly+angband@leptoquark.net>
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "buildid.h"
#include "main.h"
#include "player.h"
#include "player-birth.h"

#ifdef USE_TEST

static int prompt = 0;
static int verbose = 0;
static int nextkey = 0;

static void c_key(char *rest) {
	if (streq(rest, "left")) {
		nextkey = ARROW_LEFT;
	} else if (streq(rest, "right")) {
		nextkey = ARROW_RIGHT;
	} else if (streq(rest, "up")) {
		nextkey = ARROW_UP;
	} else if (streq(rest, "down")) {
		nextkey = ARROW_DOWN;
	} else if (streq(rest, "space")) {
		nextkey = ' ';
	} else if (streq(rest, "enter")) {
		nextkey = '\n';
	} else if (rest[0] == 'C' && rest[1] == '-') {
		nextkey = KTRL(rest[2]);
	} else {
		nextkey = rest[0];
	}
}

static void c_noop(char *rest) {

}

static void c_quit(char *rest) {
	quit(NULL);
}

static void c_verbose(char *rest) {
	if (rest && streq(rest, "0")) {
		printf("cmd-verbose: off\n");
		verbose = 0;
	} else {
		printf("cmd-verbose: on\n");
		verbose = 1;
	}
}

static void c_version(char *rest) {
	printf("cmd-version: %s\n", buildid);
}

/**
 * Player commands
 */
static void c_player_birth(char *rest) {
	const char *race = strtok(rest, " ");
	const char *class = strtok(NULL, " ");
	struct player_class *c;
	struct player_race *r;

	if (!race) race = "Human";
	if (!class) class = "Warrior";

	for (r = races; r; r = r->next)
		if (streq(race, r->name))
			break;
	if (!r) {
		printf("player-birth: bad race '%s'\n", race);
		return;
	}

	for (c = classes; c; c = c->next)
		if (streq(class, c->name))
			break;

	if (!c) {
		printf("player-birth: bad class '%s'\n", class);
		return;
	}

	player_generate(player, r, c, false);
}

static void c_player_class(char *rest) {
	printf("player-class: %s\n", player->class->name);
}

static void c_player_race(char *rest) {
	printf("player-race: %s\n", player->race->name);
}

typedef struct {
	const char *name;
	void (*func)(char *args);
} test_cmd;

static test_cmd cmds[] = {
	{ "#", c_noop },
	{ "key", c_key },
	{ "noop", c_noop },
	{ "quit", c_quit },
	{ "verbose", c_verbose },
	{ "version?", c_version },

	{ "player-birth", c_player_birth },
	{ "player-class?", c_player_class },
	{ "player-race?", c_player_race },

	{ NULL, NULL }
};

static errr test_docmd(void) {
	char buf[1024];
	char *cmd;
	char *rest;
	int i;

	memset(buf, 0, sizeof(buf));

	if (prompt) {
		printf("test> ");
		fflush(stdout);
	}
	if (!fgets(buf, sizeof(buf), stdin)) {
		return -1;
	}
	if (strchr(buf, '\n')) {
		*strchr(buf, '\n') = '\0';
	}

	if (verbose) printf("test-docmd: %s\n", buf);
	cmd = strtok(buf, " ");
	if (!cmd) return 0;
	rest = strtok(NULL, "");

	for (i = 0; cmds[i].name; i++) {
		if (streq(cmds[i].name, cmd)) {
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
	if (verbose) printf("term-init %s\n", buildid);
}

static void term_nuke_test(term *t) {
	if (verbose) printf("term-end\n");
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
	if (nextkey) {
		Term_keypress(nextkey, 0);
		nextkey = 0;
	}
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

static errr term_text_test(int x, int y, int n, int a, const wchar_t *s) {
	if (verbose) {
		char str[256];
		wcstombs(str, s, 256);
		printf("term-text %d %d %d %02x %s\n", x, y, n, a, str);
	}
	return 0;
}

static void term_data_link(int i) {
	term *t = &td.t;

	term_init(t, 80, 24, 256);

	t->init_hook = term_init_test;
	t->nuke_hook = term_nuke_test;

	t->xtra_hook = term_xtra_test;
	t->curs_hook = term_curs_test;
	t->wipe_hook = term_wipe_test;
	t->text_hook = term_text_test;

	t->data = &td;

	Term_activate(t);

	angband_term[i] = t;
}

const char help_test[] = "Test mode, subopts -p(rompt)";

errr init_test(int argc, char *argv[]) {
	int i;

	/* Skip over argv[0] */
	for (i = 1; i < argc; i++) {
		if (streq(argv[i], "-p")) {
			prompt = 1;
			continue;
		}
		printf("init-test: bad argument '%s'\n", argv[i]);
	}

	term_data_link(0);
	return 0;
}
#endif
