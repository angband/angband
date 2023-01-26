/* parse/r-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "init.h"
#include "monster.h"
#include "mon-spell.h"
#include "object.h"
#include "obj-util.h"
#include <locale.h>
#include <langinfo.h>

static char dummy_chest_1[24] = "& Small wooden chest~";
static char dummy_chest_2[24] = "& Small iron chest~";
static char dummy_torch[24] = "& Wooden Torch~";
static struct object_kind dummy_kinds[] = {
	{ .name = NULL, .kidx = 0, .tval = 0 },
	{ .name = dummy_chest_1, .kidx = 1, .tval = TV_CHEST, .sval = 1 },
	{ .name = dummy_chest_2, .kidx = 2, .tval = TV_CHEST, .sval = 2 },
	{ .name = dummy_torch, .kidx = 3, .tval = TV_LIGHT, .sval = 1, .next = NULL }
};

int setup_tests(void **state) {
	int i;

	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->max_sight = 20;
	/*
	 * Initialize just enough of the blow methods and effects so the tests
	 * will work.
	 */
	z_info->blow_methods_max = 3;
	blow_methods = mem_zalloc(z_info->blow_methods_max * sizeof(*blow_methods));
	blow_methods[1].name = string_make("CLAW");
	blow_methods[1].next = &blow_methods[2];
	blow_methods[2].name = string_make("BITE");
	blow_methods[2].next = NULL;
	z_info->blow_effects_max = 2;
	blow_effects = mem_zalloc(z_info->blow_effects_max * sizeof(*blow_effects));
	blow_effects[0].name = string_make("NONE");
	blow_effects[0].next = &blow_effects[1];
	blow_effects[1].name = string_make("FIRE");
	blow_effects[1].next = NULL;
	/* Set up so monster base lookups work. */
	rb_info = &test_rb_info;
	/* Set up just enough so object lookups work for the tests. */
	k_info = dummy_kinds;
	z_info->k_max = (uint16_t) N_ELEMENTS(dummy_kinds);
	z_info->ordinary_kind_max = z_info->k_max;
	for (i = (int) N_ELEMENTS(dummy_kinds) - 2; i >= 0; --i) {
		dummy_kinds[i].next = dummy_kinds + i + 1;
	}
	*state = init_parse_monster();
	return !*state;
}

int teardown_tests(void *state) {
	struct monster_race *mr = parser_priv(state);
	struct monster_blow *mb;
	struct monster_altmsg *ma;
	struct blow_method *meth;
	struct blow_effect *eff;
	struct monster_drop *md;
	struct monster_friends *mf;
	struct monster_friends_base *mfb;
	struct monster_mimic *mm;
	struct monster_shape *ms;

	string_free(mr->name);
	string_free(mr->text);
	string_free(mr->plural);
	mb = mr->blow;
	while (mb) {
		struct monster_blow *mbn = mb->next;

		mem_free(mb);
		mb = mbn;
	}
	ma = mr->spell_msgs;
	while (ma) {
		struct monster_altmsg *man = ma->next;

		string_free(ma->message);
		mem_free(ma);
		ma = man;
	}
	md = mr->drops;
	while (md) {
		struct monster_drop *mdn = md->next;

		mem_free(md);
		md = mdn;
	}
	mf = mr->friends;
	while (mf) {
		struct monster_friends *mfn = mf->next;

		string_free(mf->name);
		mem_free(mf);
		mf = mfn;
	}
	mfb = mr->friends_base;
	while (mfb) {
		struct monster_friends_base *mfbn = mfb->next;

		mem_free(mfb);
		mfb = mfbn;
	}
	mm = mr->mimic_kinds;
	while (mm) {
		struct monster_mimic *mmn = mm->next;

		mem_free(mm);
		mm = mmn;
	}
	ms = mr->shapes;
	while (ms) {
		struct monster_shape *msn = ms->next;

		string_free(ms->name);
		mem_free(ms);
		ms = msn;
	}
	mem_free(mr);
	parser_destroy(state);
	for (eff = blow_effects; eff; eff = eff->next) {
		string_free(eff->effect_type);
		string_free(eff->desc);
		string_free(eff->name);
	}
	mem_free(blow_effects);
	for (meth = &blow_methods[1]; meth; meth = meth->next) {
		struct blow_message *msg = meth->messages;
		string_free(meth->desc);
		while (msg) {
			struct blow_message *next = msg->next;
			string_free(msg->act_msg);
			mem_free(msg);
			msg = next;
		}
		string_free(meth->name);
	}
	mem_free(blow_methods);
	mem_free(z_info);
	return 0;
}

static bool has_alternate_message(const struct monster_race *r, uint16_t s_idx,
		enum monster_altmsg_type msg_type, const char *message)
{
	struct monster_altmsg *am = r->spell_msgs;

	while (1) {
		if (!am) return false;
		if (am->index == s_idx && am->msg_type == msg_type
				&& streq(am->message, message)) return true;
		am = am->next;
	}
}

static int test_missing_header_record0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_race *mr = (struct monster_race*) parser_priv(p);
	enum parser_error r;

	null(mr);
	r = parser_parse(p, "plural:red-hatted elves");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "color:r");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "speed:110");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "hit-points:3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "light:-2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "hearing:30");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "smell:50");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "armor-class:36");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "sleepiness:45");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "depth:15");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "rarity:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "experience:25");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:IM_POIS");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags-off:HURT_COLD");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:He looks squalid and thoroughly revolting.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "innate-freq:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-freq:12");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-power:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spells:WOUND | SCARE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-vis:WOUND:{name} dances a jig.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-invis:WOUND:Something curses.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-miss:WOUND:{name} coughs up a hairball.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "drop:chest:small wooden chest:20:1:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "drop-base:light:5:1:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "friends:20:1d2:blubbering idiot");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "friends-base:20:1d3:townsfolk");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "mimic:chest:small wooden chest");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "shape:townsfolk");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:Carcharoth, the Jaws of Thirst");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(streq(mr->name, "Carcharoth, the Jaws of Thirst"));
	ok;
}

static int test_plural0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that specifying no plural (i.e. use default) works. */
	enum parser_error r = parser_parse(p, "plural:");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	null(mr->plural);
	/* Check that supplying a plural works. */
	r = parser_parse(p, "plural:red-hatted elves");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->plural);
	require(streq(mr->plural, "red-hatted elves"));
	ok;
}

static int test_base0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "base:townsfolk");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(state);
	notnull(mr);
	notnull(mr->base);
	require(streq(mr->base->name, "townsfolk"));
	ok;
}

static int test_base_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized monster base. */
	enum parser_error r = parser_parse(p, "base:xyzzy");

	eq(r, PARSE_ERROR_INVALID_MONSTER_BASE);
	ok;
}

static int test_glyph0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "glyph:!");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	eq(mr->d_char, L'!');
	if (setlocale(LC_CTYPE, "") && streq(nl_langinfo(CODESET), "UTF-8")) {
		/*
		 * Check that a glyph outside of the ASCII range works.  Using
		 * the Yen sign, U+00A5 or C2 A5 as UTF-8.
		 */
		wchar_t wcs[3];
		int nc;

		r = parser_parse(p, "glyph:¥");
		eq(r, PARSE_ERROR_NONE);
		nc = text_mbstowcs(wcs, "¥", (int) N_ELEMENTS(wcs));
		eq(nc, 1);
		eq(mr->d_char, wcs[0]);
	}
	ok;
}

static int test_color0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "color:v");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(p);
	require(mr);
	eq(mr->d_attr, COLOUR_VIOLET);
	/* Check that color can be set by the full name. */
	r = parser_parse(p, "color:Light Green");
	eq(r, PARSE_ERROR_NONE);
	eq(mr->d_attr, COLOUR_L_GREEN);
	/* Check that full name matching is case insensitive. */
	r = parser_parse(p, "color:light red");
	eq(r, PARSE_ERROR_NONE);
	eq(mr->d_attr, COLOUR_L_RED);
	ok;
}

static int test_speed0(void *state) {
	enum parser_error r = parser_parse(state, "speed:7");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->speed, 7);
	ok;
}

static int test_hp0(void *state) {
	enum parser_error r = parser_parse(state, "hit-points:500");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->avg_hp, 500);
	ok;
}

static int test_hearing0(void *state) {
	enum parser_error r = parser_parse(state, "hearing:80");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->hearing, 80);
	ok;
}

static int test_smell0(void *state) {
	enum parser_error r = parser_parse(state, "smell:30");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->smell, 30);
	ok;
}

static int test_ac0(void *state) {
	enum parser_error r = parser_parse(state, "armor-class:22");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->ac, 22);
	ok;
}

static int test_sleep0(void *state) {
	enum parser_error r = parser_parse(state, "sleepiness:3");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->sleep, 3);
	ok;
}

static int test_depth0(void *state) {
	enum parser_error r = parser_parse(state, "depth:42");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->level, 42);
	ok;
}

static int test_rarity0(void *state) {
	enum parser_error r = parser_parse(state, "rarity:11");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->rarity, 11);
	ok;
}

static int test_mexp0(void *state) {
	enum parser_error r = parser_parse(state, "experience:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->mexp, 4);
	ok;
}

static int test_blow0(void *state) {
	enum parser_error r = parser_parse(state, "blow:CLAW:FIRE:9d12");
	struct monster_race *mr;
	struct monster_blow *mb;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->blow);
	mb = mr->blow;
	while (mb->next) {
		mb = mb->next;
	}
	require(mb->method && streq(mb->method->name, "CLAW"));
	require(mb->effect && streq(mb->effect->name, "FIRE"));
	eq(mb->dice.dice, 9);
	eq(mb->dice.sides, 12);
	ok;
}

static int test_blow1(void *state) {
	enum parser_error r = parser_parse(state, "blow:BITE:FIRE:6d8:0");
	struct monster_race *mr;
	struct monster_blow *mb;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->blow);
	mb = mr->blow;
	while (mb->next) {
		mb = mb->next;
	}
	require(mb->method && streq(mb->method->name, "BITE"));
	require(mb->effect && streq(mb->effect->name, "FIRE"));
	eq(mb->dice.dice, 6);
	eq(mb->dice.sides, 8);
	ok;
}

static int test_blow_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized type of blow. */
	enum parser_error r = parser_parse(p, "blow:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_BLOW);
	/* Try an unrecognized effect. */
	r = parser_parse(p, "blow:BITE:XYZZY");
	eq(r, PARSE_ERROR_INVALID_EFFECT);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_race *mr = (struct monster_race*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[RF_SIZE];

	notnull(mr);
	rf_wipe(mr->flags);
	/* Check that using an empty set of flags works. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(rf_is_empty(mr->flags));
	/* Check that supplying a single flag works. */
	r = parser_parse(p, "flags:UNAWARE");
	eq(r, PARSE_ERROR_NONE);
	/* Check that supplying multiple flags works. */
	r = parser_parse(p, "flags:UNIQUE | MALE");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	rf_wipe(eflags);
	rf_on(eflags, RF_UNAWARE);
	rf_on(eflags, RF_UNIQUE);
	rf_on(eflags, RF_MALE);
	require(rf_is_equal(mr->flags, eflags));
	ok;
}

static int test_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an unknown flag generates an appropriate error. */
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_flags_off0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_race *mr = (struct monster_race*) parser_priv(p);
	enum parser_error r;

	notnull(mr);
	rf_wipe(mr->flags);
	rf_on(mr->flags, RF_UNIQUE);
	rf_on(mr->flags, RF_MALE);
	rf_on(mr->flags, RF_UNAWARE);
	/* Check that using an empty set of flags works. */
	r = parser_parse(p, "flags-off:");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(rf_has(mr->flags, RF_UNIQUE));
	require(rf_has(mr->flags, RF_MALE));
	require(rf_has(mr->flags, RF_UNAWARE));
	/* Check that supplying a single flag works. */
	r = parser_parse(p, "flags-off:UNIQUE");
	eq(r, PARSE_ERROR_NONE);
	/* Check that supplying multiple flags works. */
	r = parser_parse(p, "flags-off:MALE | UNAWARE");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(!rf_has(mr->flags, RF_UNIQUE));
	require(!rf_has(mr->flags, RF_MALE));
	require(!rf_has(mr->flags, RF_UNAWARE));
	ok;
}

static int test_flags_off_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an unknown flag generates an appropriate error. */
	enum parser_error r = parser_parse(p, "flags-off:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_desc0(void *state) {
	enum parser_error r = parser_parse(state, "desc:foo bar ");
	enum parser_error s = parser_parse(state, "desc: baz");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	eq(s, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(streq(mr->text, "foo bar  baz"));
	ok;
}

static int test_innate_freq0(void *state) {
	enum parser_error r = parser_parse(state, "innate-freq:10");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->freq_innate, 10);
	ok;
}

static int test_innate_freq_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that values outside of 1 to 100 are rejected. */
	enum parser_error r = parser_parse(p, "innate-freq:0");

	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	r = parser_parse(p, "innate-freq:-2");
	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	r = parser_parse(p, "innate-freq:101");
	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	ok;
}

static int test_spell_freq0(void *state) {
	enum parser_error r = parser_parse(state, "spell-freq:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->freq_spell, 25);
	ok;
}

static int test_spell_freq_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that values outside of 1 to 100 are rejected. */
	enum parser_error r = parser_parse(p, "spell-freq:0");

	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	r = parser_parse(p, "spell-freq:-5");
	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	r = parser_parse(p, "spell-freq:101");
	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	ok;
}

static int test_spell_power0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "spell-power:15");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	eq(mr->spell_power, 15);
	ok;
}

static int test_spells0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_race *mr = (struct monster_race*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[RSF_SIZE];

	notnull(mr);
	rsf_wipe(mr->spell_flags);
	/* Check that one spell works. */
	r = parser_parse(p, "spells:SCARE");
	eq(r, PARSE_ERROR_NONE);
	/* Check that setting multiple spells works. */
	r = parser_parse(p, "spells:BR_DARK | S_HOUND");
	eq(r, PARSE_ERROR_NONE);
	rsf_wipe(eflags);
	rsf_on(eflags, RSF_SCARE);
	rsf_on(eflags, RSF_BR_DARK);
	rsf_on(eflags, RSF_S_HOUND);
	require(rsf_is_equal(mr->spell_flags, eflags));
	/*
	 * Check that innate frequency is implictly set if a innate spell is
	 * enabled and the frequency was not set before.
	 */
	rsf_wipe(mr->spell_flags);
	mr->freq_innate = 0;
	r = parser_parse(p, "spells:SHRIEK");
	eq(r, PARSE_ERROR_NONE);
	require(mr->freq_innate != 0);
	rsf_wipe(eflags);
	rsf_on(eflags, RSF_SHRIEK);
	require(rsf_is_equal(mr->spell_flags, eflags));
	/*
	 * Check that non-innate frequency is implicitly set if a non-innate
	 * spell is enabled and the frequency was not set before.
	 */
	rsf_wipe(mr->spell_flags);
	mr->freq_spell = 0;
	r = parser_parse(p, "spells:BA_ACID");
	eq(r, PARSE_ERROR_NONE);
	require(mr->freq_spell != 0);
	rsf_wipe(eflags);
	rsf_on(eflags, RSF_BA_ACID);
	require(rsf_is_equal(mr->spell_flags, eflags));
	ok;
}

static int test_spells_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an unknown spell generates an appropriate error. */
	enum parser_error r = parser_parse(p, "spells:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_messagevis0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an empty message works. */
	enum parser_error r = parser_parse(p, "message-vis:TRAPS");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(has_alternate_message(mr, RSF_TRAPS, MON_ALTMSG_SEEN, ""));
	/* Check with a non-empty message. */
	r = parser_parse(p, "message-vis:WOUND:{name} curses malevolently.");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(has_alternate_message(mr, RSF_WOUND, MON_ALTMSG_SEEN,
		"{name} curses malevolently."));
	ok;
}

static int test_messagevis_bad0(void *state) {
	enum parser_error r = parser_parse(state,
		"message-vis:XYZZY:{name} waves its tentacles menacingly.");

	eq(r, PARSE_ERROR_INVALID_SPELL_NAME);
	ok;
}

static int test_messageinvis0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an empty message works. */
	enum parser_error r = parser_parse(p, "message-invis:BLINK");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(has_alternate_message(mr, RSF_BLINK, MON_ALTMSG_UNSEEN, ""));
	/* Check with a non-empty message. */
	r = parser_parse(p, "message-invis:SHRIEK:Something shouts.");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(has_alternate_message(mr, RSF_SHRIEK, MON_ALTMSG_UNSEEN,
		"Something shouts."));
	ok;
}

static int test_messageinvis_bad0(void *state) {
	enum parser_error r = parser_parse(state,
		"message-invis:XYZZY:Something whispers.");

	eq(r, PARSE_ERROR_INVALID_SPELL_NAME);
	ok;
}

static int test_messagemiss0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an empty message works. */
	enum parser_error r = parser_parse(p, "message-miss:SPIT");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(has_alternate_message(mr, RSF_SPIT, MON_ALTMSG_MISS, ""));
	/* Check with a non-empty message. */
	r = parser_parse(p,
		"message-miss:BOULDER:{name} throws a boulder and misses.");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	require(has_alternate_message(mr, RSF_BOULDER, MON_ALTMSG_MISS,
		"{name} throws a boulder and misses."));
	ok;
}

static int test_messagemiss_bad0(void *state) {
	enum parser_error r = parser_parse(state,
		"message-miss:XYZZY:{name} bobbles the ball and drops it.");

	eq(r, PARSE_ERROR_INVALID_SPELL_NAME);
	ok;
}

static int test_drop0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "drop:light:wooden torch:10:1:2");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->drops);
	notnull(mr->drops->kind);
	eq(mr->drops->kind->tval, TV_LIGHT);
	eq(mr->drops->kind->sval, lookup_sval(TV_LIGHT, "wooden torch"));
	eq(mr->drops->percent_chance, 10);
	eq(mr->drops->min, 1);
	eq(mr->drops->max, 2);
	ok;
}

static int test_drop_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized tval. */
	enum parser_error r = parser_parse(p, "drop:xyzzy:small wooden chest:5:1:1");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	/* Try an unrecognized object. */
	r = parser_parse(p, "drop:light:xyzzy:10:1:3");
	eq(r, PARSE_ERROR_UNRECOGNISED_SVAL);
	/* Try invalid numbers (rejects anything more than 99). */
	r = parser_parse(p, "drop:light:wooden torch:8:1:100");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	r = parser_parse(p, "drop:light:wooden torch:8:100:3");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	r = parser_parse(p, "drop:light:wooden torch:8:100:105");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	ok;
}

static int test_drop_base0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "drop-base:light:10:1:2");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->drops);
	null(mr->drops->kind);
	eq(mr->drops->tval, TV_LIGHT);
	eq(mr->drops->percent_chance, 10);
	eq(mr->drops->min, 1);
	eq(mr->drops->max, 2);
	ok;
}

static int test_drop_base_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized tval. */
	enum parser_error r = parser_parse(p, "drop-base:xyzzy:5:1:4");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	/* Try invalid numbers (rejects anything more than 99). */
	r = parser_parse(p, "drop-base:light:10:1:100");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	r = parser_parse(p, "drop-base:light:20:101:3");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	r = parser_parse(p, "drop-base:light:14:100:108");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	ok;
}

static int test_friends0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that specifying without a role works. */
	enum parser_error r = parser_parse(p,
		"friends:15:1d2:blubbering idiot");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->friends);
	notnull(mr->friends->name);
	require(streq(mr->friends->name, "blubbering idiot"));
	eq(mr->friends->number_dice, 1);
	eq(mr->friends->number_side, 2);
	eq(mr->friends->percent_chance, 15);
	eq(mr->friends->role, MON_GROUP_MEMBER);
	/* Check that specifying a servant role works. */
	r = parser_parse(p, "friends:25:2d1:agent of the black market:servant");
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->friends);
	notnull(mr->friends->name);
	require(streq(mr->friends->name, "agent of the black market"));
	eq(mr->friends->number_dice, 2);
	eq(mr->friends->number_side, 1);
	eq(mr->friends->percent_chance, 25);
	eq(mr->friends->role, MON_GROUP_SERVANT);
	/* Check that specifying a bodyguard role works. */
	r = parser_parse(p, "friends:75:1d3:mean-looking mercenary:bodyguard");
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->friends);
	notnull(mr->friends->name);
	require(streq(mr->friends->name, "mean-looking mercenary"));
	eq(mr->friends->number_dice, 1);
	eq(mr->friends->number_side, 3);
	eq(mr->friends->percent_chance, 75);
	eq(mr->friends->role, MON_GROUP_BODYGUARD);
	ok;
}

static int test_friends_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized role. */
	enum parser_error r = parser_parse(p,
		"friends:5:1d2:blubbering idiot:xyzzy");

	eq(r, PARSE_ERROR_INVALID_MONSTER_ROLE);
	ok;
}

static int test_friends_base0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that specifying without a role works. */
	enum parser_error r = parser_parse(p, "friends-base:20:1d3:townsfolk");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->friends_base);
	notnull(mr->friends_base->base);
	notnull(mr->friends_base->base->name);
	require(streq(mr->friends_base->base->name, "townsfolk"));
	eq(mr->friends_base->number_dice, 1);
	eq(mr->friends_base->number_side, 3);
	eq(mr->friends_base->percent_chance, 20);
	eq(mr->friends_base->role, MON_GROUP_MEMBER);
	/* Check that specifying a servant role works. */
	r = parser_parse(p, "friends-base:5:1d6:townsfolk:servant");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->friends_base);
	notnull(mr->friends_base->base);
	notnull(mr->friends_base->base->name);
	require(streq(mr->friends_base->base->name, "townsfolk"));
	eq(mr->friends_base->number_dice, 1);
	eq(mr->friends_base->number_side, 6);
	eq(mr->friends_base->percent_chance, 5);
	eq(mr->friends_base->role, MON_GROUP_SERVANT);
	/* Check that specifying a bodyguard role works. */
	r = parser_parse(p, "friends-base:10:1d2:townsfolk:bodyguard");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->friends_base);
	notnull(mr->friends_base->base);
	notnull(mr->friends_base->base->name);
	require(streq(mr->friends_base->base->name, "townsfolk"));
	eq(mr->friends_base->number_dice, 1);
	eq(mr->friends_base->number_side, 2);
	eq(mr->friends_base->percent_chance, 10);
	eq(mr->friends_base->role, MON_GROUP_BODYGUARD);
	ok;
}

static int test_friends_base_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized base. */
	enum parser_error r = parser_parse(p, "friends-base:15:1d2:xyzzy");

	require(r != PARSE_ERROR_NONE);
	/* Try an unrecognized role. */
	r = parser_parse(p, "friends-base:20:1d3:townsfolk:xyzzy");
	eq(r, PARSE_ERROR_INVALID_MONSTER_ROLE);
	ok;
}

static int test_mimic0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "mimic:chest:small wooden chest");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->mimic_kinds);
	notnull(mr->mimic_kinds->kind);
	eq(mr->mimic_kinds->kind->tval, TV_CHEST);
	eq(mr->mimic_kinds->kind->sval, lookup_sval(TV_CHEST,
		"small wooden chest"));
	ok;
}

static int test_mimic_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized tval. */
	enum parser_error r = parser_parse(p, "mimic:xyzzy:wooden torch");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	/* Try an unrecognized kind. */
	r = parser_parse(p, "mimic:light:xyzzy");
	eq(r, PARSE_ERROR_UNRECOGNISED_SVAL);
	ok;
}

static int test_shape0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_race *mr = (struct monster_race*) parser_priv(p);
	enum parser_error r;
	int old_count;

	notnull(mr);
	old_count = mr->num_shapes;
	/* Try a monster base. */
	r = parser_parse(p, "shape:townsfolk");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->shapes);
	notnull(mr->shapes->name);
	require(streq(mr->shapes->name, "townsfolk"));
	notnull(mr->shapes->base);
	notnull(mr->shapes->base->name);
	require(streq(mr->shapes->base->name, "townsfolk"));
	eq(mr->num_shapes, old_count + 1);
	/* Try a monster name. */
	r = parser_parse(p, "shape:blubbering idiot");
	eq(r, PARSE_ERROR_NONE);
	mr = (struct monster_race*) parser_priv(p);
	notnull(mr);
	notnull(mr->shapes);
	notnull(mr->shapes->name);
	require(streq(mr->shapes->name, "blubbering idiot"));
	null(mr->shapes->base);
	eq(mr->num_shapes, old_count + 2);
	ok;
}

const char *suite_name = "parse/r-info";
/*
 * test_missing_header_record0() has to be before test_name0().
 * All others, except test_name0(), have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_header_record0", test_missing_header_record0 },
	{ "name0", test_name0 },
	{ "plural0", test_plural0 },
	{ "base0", test_base0 },
	{ "base_bad0", test_base_bad0 },
	{ "color0", test_color0 },
	{ "glyph0", test_glyph0 },
	{ "speed0", test_speed0 },
	{ "hp0", test_hp0 },
	{ "hearing0", test_hearing0 },
	{ "smell0", test_smell0 },
	{ "ac0", test_ac0 },
	{ "sleep0", test_sleep0 },
	{ "depth0", test_depth0 },
	{ "rarity0", test_rarity0 },
	{ "mexp0", test_mexp0 },
	{ "blow0", test_blow0 },
	{ "blow1", test_blow1 },
	{ "blow_bad0", test_blow_bad0 },
	{ "flags0", test_flags0 },
	{ "flags_bad0", test_flags_bad0 },
	{ "flags_off0", test_flags_off0 },
	{ "flags_off_bad0", test_flags_off_bad0 },
	{ "desc0", test_desc0 },
	{ "innate-freq0", test_innate_freq0 },
	{ "innate-freq_bad0", test_innate_freq_bad0 },
	{ "spell-freq0", test_spell_freq0 },
	{ "spell-freq_bad0", test_spell_freq_bad0 },
	{ "spell_power0", test_spell_power0 },
	{ "spells0", test_spells0 },
	{ "spells_bad0", test_spells_bad0 },
	{ "message-vis0", test_messagevis0 },
	{ "message-vis-bad0", test_messagevis_bad0 },
	{ "message-invis0", test_messageinvis0 },
	{ "message-invis-bad0", test_messageinvis_bad0 },
	{ "message-miss0", test_messagemiss0 },
	{ "message-miss-bad0", test_messagemiss_bad0 },
	{ "drop0", test_drop0 },
	{ "drop_bad0", test_drop_bad0 },
	{ "drop-base0", test_drop_base0 },
	{ "drop-base_bad0", test_drop_base_bad0 },
	{ "friends0", test_friends0 },
	{ "friends_bad0", test_friends_bad0 },
	{ "friends-base0", test_friends_base0 },
	{ "friends-base_bad0", test_friends_base_bad0 },
	{ "mimic0", test_mimic0 },
	{ "mimic_bad0", test_mimic_bad0 },
	{ "shape0", test_shape0 },
	{ NULL, NULL }
};
