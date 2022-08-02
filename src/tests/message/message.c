/* message/message.c */

#include "unit-test.h"
#include "unit-test-data.h"

#include "message.h"
#include "game-event.h"
#include "z-color.h"
#include "z-util.h"
#include "z-virt.h"

struct test_message_event_state {
	char *lastmsg;
	char *lastbell;
	int lastmsg_type;
	int lastsound_type;
	int n_msg;
	int n_sound;
	int n_bell;
	int n_other;
	bool had_invalid_bell;
};

static void message_test_event_handler(game_event_type type,
	game_event_data *data, void *user);
static struct test_message_event_state *setup_event_handlers(void);
static void reset_event_counters(struct test_message_event_state *st);
static void cleanup_event_handlers(struct test_message_event_state *st);


int setup_tests(void **state) {
	player = &test_player;
	*state = setup_event_handlers();
	messages_init();
	return 0;
}

int teardown_tests(void *state) {
	struct test_message_event_state *st = state;

	messages_free();
	cleanup_event_handlers(st);
	return 0;
}

static int test_empty(void *state) {
	const char *txt;
	uint16_t n, mtype;
	uint8_t color;

	messages_free();
	messages_init();

	n = messages_num();
	eq(n, 0);
	txt = message_str(0);
	require(streq(txt, ""));
	txt = message_str(1);
	require(streq(txt, ""));
	n = message_count(0);
	eq(n, 0);
	n = message_count(2);
	eq(n, 0);
	mtype = message_type(0);
	eq(mtype, 0);
	mtype = message_type(3);
	eq(mtype, 0);
	color = message_color(0);
	eq(color, (COLOUR_WHITE));
	color = message_color(4);
	eq(color, (COLOUR_WHITE));

	ok;
}

static int test_add(void *state) {
	const char *m1 = "msg1";
	uint16_t t1 = MSG_GENERIC;
	const char *m2 = "msg2";
	uint16_t t2 = MSG_HIT;
	const char *txt;
	uint16_t n, mtype;

	messages_free();
	messages_init();

	message_add(m1, t1);
	n = messages_num();
	eq(n, 1);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, m1));
	mtype = message_type(0);
	eq(mtype, t1);
	n = message_count(1);
	eq(n, 0);

	message_add(m2, t2);
	n = messages_num();
	eq(n, 2);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, m2));
	mtype = message_type(0);
	eq(mtype, t2);
	n = message_count(1);
	eq(n, 1);
	txt = message_str(1);
	require(streq(txt, m1));
	mtype = message_type(1);
	eq(mtype, t1);
	n = message_count(2);
	eq(n, 0);

	/*
	 * Adding a duplicate message should only increase the count of the
	 * most recent one and have no other effect.
	 */
	message_add(m2, t2);
	n = messages_num();
	eq(n, 2);
	n = message_count(0);
	eq(n, 2);
	txt = message_str(0);
	require(streq(txt, m2));
	mtype = message_type(0);
	eq(mtype, t2);
	n = message_count(1);
	eq(n, 1);
	txt = message_str(1);
	require(streq(txt, m1));
	mtype = message_type(1);
	eq(mtype, t1);
	n = message_count(2);
	eq(n, 0);

	/*
	 * If the string is a duplicate but the type is different, it should
	 * not stack.
	 */
	message_add(m2, t1);
	n = messages_num();
	eq(n, 3);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, m2));
	mtype = message_type(0);
	eq(mtype, t1);
	n = message_count(1);
	eq(n, 2);
	txt = message_str(1);
	require(streq(txt, m2));
	mtype = message_type(1);
	eq(mtype, t2);
	n = message_count(2);
	eq(n, 1);
	txt = message_str(2);
	require(streq(txt, m1));
	mtype = message_type(2);
	eq(mtype, t1);
	n = message_count(3);
	eq(n, 0);

	/*
	 * If the type is the same but the string is different, it also should
	 * not stack.
	 */
	message_add(m1, t1);
	n = messages_num();
	eq(n, 4);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, m1));
	mtype = message_type(0);
	eq(mtype, t1);
	n = message_count(1);
	eq(n, 1);
	txt = message_str(1);
	require(streq(txt, m2));
	mtype = message_type(1);
	eq(mtype, t1);
	n = message_count(2);
	eq(n, 2);
	txt = message_str(2);
	require(streq(txt, m2));
	mtype = message_type(2);
	eq(mtype, t2);
	n = message_count(3);
	eq(n, 1);
	txt = message_str(3);
	require(streq(txt, m1));
	mtype = message_type(3);
	eq(mtype, t1);
	n = message_count(4);
	eq(n, 0);

	ok;
}

static int test_fill(void *state) {
	int i = 0;
	const char *txt;
	char buf[16];
	uint16_t n, n2, j;

	messages_free();
	messages_init();

	/*
	 * Keep adding messages till it fills up.  Verify that the oldest
	 * message is lost.
	 */
	while (1) {
		require(i < 1 << 16);
		(void) sprintf(buf, "%d", i);
		message_add(buf, MSG_GENERIC);
		++i;
		n = messages_num();
		if (i != n) {
			eq(i, (int)n + 1);
			for (j = 0; j < n; ++j) {
				n2 = message_count(j);
				eq(n2, 1);
				txt = message_str(j);
				(void) sprintf(buf, "%d", i - 1 - (int) j);
				require(streq(txt, buf));
			}

			/* Test adding one more. */
			message_add("msg", MSG_GENERIC);
			n2 = messages_num();
			eq(n, n2);
			n2 = message_count(0);
			eq(n2, 1);
			txt = message_str(0);
			require(streq(txt, "msg"));
			n2 = message_count(n - 1);
			eq(n2, 1);
			txt = message_str(n - 1);
			require(streq(txt, "2"));
			break;
		}
	}

	ok;
}

static int test_many_repeat(void *state)
{
	int i = 0;
	const char *txt;
	uint16_t n, mtype;

	messages_free();
	messages_init();

	/*
	 * Keep repeating the same message until it can't stack them any more.
	 */
	while (1) {
		require(i < 1 << 16);
		message_add("msg", MSG_GENERIC);
		++i;
		n = messages_num();
		if (n != 1) {
			eq(n, 2);
			n = message_count(0);
			eq(n, 1);
			txt = message_str(0);
			require(streq(txt, "msg"));
			mtype = message_type(0);
			eq(mtype, MSG_GENERIC);
			n = message_count(1);
			eq(n, i - 1);
			txt = message_str(1);
			require(streq(txt, "msg"));
			mtype = message_type(1);
			eq(mtype, MSG_GENERIC);
			break;
		}
		n = message_count(0);
		eq(i, n);
		txt = message_str(0);
		require(streq(txt, "msg"));
		mtype = message_type(0);
		eq(mtype, MSG_GENERIC);
	}

	ok;
}

static int test_color(void *state) {
	uint8_t color;

	messages_free();
	messages_init();

	color = message_type_color(MSG_HIT);
	eq(color, (COLOUR_WHITE));

	message_color_define(MSG_HIT, (COLOUR_RED));
	color = message_type_color(MSG_HIT);
	eq(color, (COLOUR_RED));

	message_color_define(MSG_MISS, (COLOUR_GREEN));
	color = message_type_color(MSG_MISS);
	eq(color, (COLOUR_GREEN));

	message_add("msg0", MSG_MISS);
	color = message_color(0);
	eq(color, (COLOUR_GREEN));

	message_add("msg1", MSG_HIT);
	color = message_color(0);
	eq(color, (COLOUR_RED));
	color = message_color(1);
	eq(color, (COLOUR_GREEN));

	message_add("msg2", MSG_GENERIC);
	color = message_color(0);
	eq(color, (COLOUR_WHITE));
	color = message_color(1);
	eq(color, (COLOUR_RED));
	color = message_color(2);
	eq(color, (COLOUR_GREEN));

	message_color_define(MSG_HIT, (COLOUR_L_BLUE));
	color = message_color(0);
	eq(color, (COLOUR_WHITE));
	color = message_color(1);
	eq(color, (COLOUR_L_BLUE));
	color = message_color(2);
	eq(color, (COLOUR_GREEN));

	message_color_define(MSG_MISS, (COLOUR_VIOLET));
	color = message_color(0);
	eq(color, (COLOUR_WHITE));
	color = message_color(1);
	eq(color, (COLOUR_L_BLUE));
	color = message_color(2);
	eq(color, (COLOUR_VIOLET));


	ok;
}

static int test_msg(void *state) {
	struct test_message_event_state *st = state;
	const char expected1[] = "%   abcde   1  +2  3 4  ";
	const char expected2[] = "ab      -7";
	const char *txt;
	uint16_t n, mtype;

	reset_event_counters(st);
	messages_free();
	messages_init();

	msg("%%%c%3s%.4s %3d %+3d % d %-3d", ' ', "a", "bcdef", 1, 2, 3, 4);
	n = messages_num();
	eq(n, 1);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, expected1));
	mtype = message_type(0);
	eq(mtype, MSG_GENERIC);
	require(st->lastmsg && streq(st->lastmsg, expected1));
	eq(st->lastmsg_type, MSG_GENERIC);
	eq(st->n_msg, 1);
	eq(st->n_sound, 0);
	eq(st->n_bell, 0);
	eq(st->n_other, 0);

	msg("%-4s %*d", "ab", 5, -7);
	n = messages_num();
	eq(n, 2);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, expected2));
	mtype = message_type(0);
	eq(mtype, MSG_GENERIC);
	require(st->lastmsg && streq(st->lastmsg, expected2));
	eq(st->lastmsg_type, MSG_GENERIC);
	eq(st->n_msg, 2);
	eq(st->n_sound, 0);
	eq(st->n_bell, 0);
	eq(st->n_other, 0);

	ok;
}

static int test_sound(void *state) {
	struct test_message_event_state *st = state;

	reset_event_counters(st);
	messages_free();
	messages_init();

	player->opts.opt[OPT_use_sound] = false;
	sound(MSG_HIT);
	eq(st->n_msg, 0);
	eq(st->n_sound, 0);
	eq(st->n_bell, 0);
	eq(st->n_other, 0);

	player->opts.opt[OPT_use_sound] = true;
	sound(MSG_MISS);
	eq(st->n_msg, 0);
	eq(st->n_sound, 1);
	eq(st->n_bell, 0);
	eq(st->n_other, 0);

	ok;
}

static int test_bell(void *state) {
	struct test_message_event_state *st = state;
	const char *txt;
	uint16_t n;

	reset_event_counters(st);
	messages_free();
	messages_init();

	player->opts.opt[OPT_use_sound] = false;
	bell();
	n = messages_num();
	eq(n, 0);
	n = message_count(0);
	eq(n, 0);
	txt = message_str(0);
	require(streq(txt, ""));
	require(st->lastbell == NULL);
	eq(st->n_msg, 0);
	eq(st->n_sound, 0);
	eq(st->n_bell, 1);
	eq(st->n_other, 0);
	require(!st->had_invalid_bell);

	ok;
}

static int test_msgt(void *state)
{
	struct test_message_event_state *st = state;
	const char expected1[] = "msg1";
	const char expected2[] = "msg2";
	const char *txt;
	uint16_t n, mtype;

	reset_event_counters(st);
	messages_free();
	messages_init();

	player->opts.opt[OPT_use_sound] = false;
	msgt(MSG_HIT, "%s", expected1);
	n = messages_num();
	eq(n, 1);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, expected1));
	mtype = message_type(0);
	eq(mtype, MSG_HIT);
	require(st->lastmsg && streq(st->lastmsg, expected1));
	eq(st->lastmsg_type, MSG_HIT);
	eq(st->n_msg, 1);
	eq(st->n_sound, 0);
	eq(st->n_bell, 0);
	eq(st->n_other, 0);

	player->opts.opt[OPT_use_sound] = true;
	msgt(MSG_WALK, "%s", expected2);
	n = messages_num();
	eq(n, 2);
	n = message_count(0);
	eq(n, 1);
	txt = message_str(0);
	require(streq(txt, expected2));
	mtype = message_type(0);
	eq(mtype, MSG_WALK);
	require(st->lastmsg && streq(st->lastmsg, expected2));
	eq(st->lastmsg_type, MSG_WALK);
	eq(st->lastsound_type, MSG_WALK);
	eq(st->n_msg, 2);
	eq(st->n_sound, 1);
	eq(st->n_bell, 0);
	eq(st->n_other, 0);

	ok;
}

static int test_lookup(void *state)
{
	char buffer[16];
	int i, j;

	messages_free();
	messages_init();

	/* Test by name. */
	i = message_lookup_by_name("GENERIC");
	eq(i, MSG_GENERIC);
	i = message_lookup_by_name("DEATH");
	eq(i, MSG_DEATH);
	i = message_lookup_by_name("MAX");
	eq(i, MSG_MAX);

	/* Test by printed number. */
	for (i = MSG_GENERIC; i < MSG_MAX; ++i) {
		(void) sprintf(buffer, "%d", i);
		j = message_lookup_by_name(buffer);
		eq(i, j);
	}

	/* Test failed lookups. */
	i = message_lookup_by_name("");
	eq(i, -1);
	i = message_lookup_by_name("kskl8bktk2b");
	eq(i, -1);
	i = message_lookup_by_name("-3");
	eq(i, -1);
	(void) sprintf(buffer, "%d", MSG_MAX);
	i = message_lookup_by_name(buffer);
	eq(i, -1);
	(void) sprintf(buffer, "%d", MSG_MAX + 1);
	i = message_lookup_by_name(buffer);
	eq(i, -1);

	ok;
}

static int test_sound_lookup(void *state)
{
	int i;

	messages_free();
	messages_init();

	for (i = MSG_GENERIC; i < MSG_MAX; ++i) {
		const char *name = message_sound_name(i);
		int j;

		notnull(name);
		j = message_lookup_by_sound_name(name);
		if (j != i) {
			/*
			 * It's not guaranteed to be a one-to-one
			 * relationship, but the following should be
			 * guaranteed.
			 */
			const char *name2 = message_sound_name(j);

			require(streq(name, name2));
		}
	}

	/* Check for lookups that should fail. */
	i = message_lookup_by_sound_name("ahvkaugowhbnsk");
	eq(i, MSG_GENERIC);
	null(message_sound_name((MSG_GENERIC) - 1));
	null(message_sound_name((MSG_MAX) + 1));

	ok;
}

static void message_test_event_handler(game_event_type type,
	game_event_data *data, void *user) {
	struct test_message_event_state *st = user;

	switch (type) {
	case EVENT_MESSAGE:
		++st->n_msg;
		if (st->lastmsg) {
			string_free(st->lastmsg);
		}
		st->lastmsg = (data->message.msg) ?
			string_make(data->message.msg) : NULL;
		st->lastmsg_type = data->message.type;
		break;

	case EVENT_BELL:
		++st->n_bell;
		if (st->lastbell) {
			string_free(st->lastbell);
		}
		st->lastbell = (data->message.msg) ?
			string_make(data->message.msg) : NULL;
		if (data->message.type != MSG_BELL) {
			st->had_invalid_bell = true;
		}
		break;

	case EVENT_SOUND:
		++st->n_sound;
		st->lastsound_type = data->message.type;
		break;

	default:
		++st->n_other;
		break;
	}
}

static struct test_message_event_state *setup_event_handlers(void) {
	struct test_message_event_state *st = mem_alloc(sizeof(*st));
	int i;

	st->lastmsg = NULL;
	st->lastbell = NULL;
	reset_event_counters(st);
	for (i = 0; i < EVENT_END; ++i) {
		event_add_handler(i, message_test_event_handler, st);
	}
	return st;
}

static void reset_event_counters(struct test_message_event_state *st)
{
	string_free(st->lastmsg);
	st->lastmsg = NULL;
	string_free(st->lastbell);
	st->lastbell = NULL;
	st->lastmsg_type = -1;
	st->lastsound_type = -1;
	st->n_msg = 0;
	st->n_sound = 0;
	st->n_bell = 0;
	st->n_other = 0;
	st->had_invalid_bell = false;
}

static void cleanup_event_handlers(struct test_message_event_state *st) {
	int i;

	for (i = 0; i < EVENT_END; ++i) {
		event_remove_handler(i, message_test_event_handler, st);
	}
	if (st) {
		string_free(st->lastmsg);
		string_free(st->lastbell);
		mem_free(st);
	}
}

const char *suite_name = "message/message";
struct test tests[] = {
	{ "empty", test_empty },
	{ "add", test_add },
	{ "fill", test_fill },
	{ "many_repeat", test_many_repeat },
	{ "color", test_color },
	{ "format", test_msg },
	{ "sound", test_sound },
	{ "bell", test_bell },
	{ "msgt", test_msgt },
	{ "lookup", test_lookup },
	{ "sound_lookup", test_sound_lookup },
	{ NULL, NULL },
};
