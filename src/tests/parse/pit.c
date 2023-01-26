/* parse/pit */
/* Exercise parsing used for pit.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "generate.h"
#include "init.h"
#include "monster.h"
#include "mon-init.h"
#include "mon-spell.h"
#include "z-color.h"
#include "z-virt.h"

static char dummy_dragon[16] = "ancient dragon";
static char dummy_ainu[16] = "ainu";
static char dummy_ant[16] = "ant";
static struct monster_base dummy_bases[] = {
	{ .name = dummy_dragon },
	{ .name = dummy_ainu },
	{ .name = dummy_ant, .next = NULL }
};

static char dummy_cutpurse[16] = "cutpurse";
static char dummy_gremlin_1[16] = "wimpy gremlin";
static char dummy_gremlin_2[16] = "ubergremlin";
static struct monster_race dummy_races[] = {
	{ .name = NULL, .ridx = 0 },
	{ .name = dummy_cutpurse, .ridx = 1 },
	{ .name = dummy_gremlin_1, .ridx = 2 },
	{ .name = dummy_gremlin_2, .ridx = 3, .next = NULL }
};

int setup_tests(void **state) {
	int i;

	*state = pit_parser.init();
	/* pit_parser.finish needs z_info.  As does monster lookup. */
	z_info = mem_zalloc(sizeof(*z_info));
	/* Do the minimum so monster base lookups work. */
	for (i = 0; i < (int) N_ELEMENTS(dummy_bases) - 1; ++i) {
		dummy_bases[i].next = dummy_bases + i + 1;
	}
	rb_info = dummy_bases;
	/* Do the minimum so monster lookups work. */
	for (i = 0; i < (int) N_ELEMENTS(dummy_races) - 1; ++i) {
		dummy_races[i].next = dummy_races + i + 1;
	}
	r_info = dummy_races;
	z_info->r_max = (uint16_t) N_ELEMENTS(dummy_races);
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (pit_parser.finish(p)) {
		r = 1;
	}
	pit_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct pit_profile *pit = (struct pit_profile*) parser_priv(p);
	enum parser_error r;

	null(pit);
	r = parser_parse(p, "room:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "alloc:1:25");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "obj-rarity:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "mon-base:ancient dragon");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "mon-ban:cutpurse");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "color:r");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags-req:FEMALE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags-ban:MALE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "innate-freq:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-req:BR_FIRE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-ban:BR_COLD");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:Orc");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	require(streq(pit->name, "Orc"));
	eq(pit->room_type, 0);
	eq(pit->ave, 0);
	eq(pit->rarity, 0);
	eq(pit->obj_rarity, 0);
	rf_is_empty(pit->flags);
	rf_is_empty(pit->forbidden_flags);
	eq(pit->freq_innate, 0);
	rsf_is_empty(pit->spell_flags);
	rsf_is_empty(pit->forbidden_spell_flags);
	null(pit->bases);
	null(pit->colors);
	null(pit->forbidden_monsters);
	ok;
}

static int test_room0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "room:1");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	eq(pit->room_type, 1);
	ok;
}

static int test_alloc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "alloc:1:25");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	eq(pit->ave, 25);
	eq(pit->rarity, 1);
	ok;
}

static int test_obj_rarity0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "obj-rarity:5");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	eq(pit->obj_rarity, 5);
	ok;
}

static int test_mon_base0(void* state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "mon-base:ancient dragon");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	/* Check that a second directive accumulates with those prior to it. */
	r = parser_parse(p, "mon-base:ant");
	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	notnull(pit->bases);
	notnull(pit->bases->base);
	notnull(pit->bases->base->name);
	require(streq(pit->bases->base->name, "ant"));
	notnull(pit->bases->next);
	notnull(pit->bases->next->base);
	notnull(pit->bases->next->base->name);
	require(streq(pit->bases->next->base->name, "ancient dragon"));
	ok;
}

static int test_mon_base_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized base. */
	enum parser_error r = parser_parse(p, "mon-base:xyzzy");

	require(r != PARSE_ERROR_NONE);
	ok;
}

static int test_mon_ban0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "mon-ban:wimpy gremlin");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	/* Check that a second directive accumulates with those prior to it. */
	r = parser_parse(p, "mon-ban:ubergremlin");
	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	notnull(pit->forbidden_monsters);
	notnull(pit->forbidden_monsters->race);
	notnull(pit->forbidden_monsters->race->name);
	require(streq(pit->forbidden_monsters->race->name, "ubergremlin"));
	notnull(pit->forbidden_monsters->next);
	notnull(pit->forbidden_monsters->next->race);
	notnull(pit->forbidden_monsters->next->race->name);
	require(streq(pit->forbidden_monsters->next->race->name,
		"wimpy gremlin"));
	ok;
}

static int test_mon_ban_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized monster race. */
	enum parser_error r = parser_parse(p, "mon-ban:xyzzy");
	struct pit_profile *pit;

	/*
	 * The unrecognized race doesn't trigger an error, but does give
	 * a NULL race pointer in the list of forbidden monsters.
	 */
	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	notnull(pit->forbidden_monsters);
	null(pit->forbidden_monsters->race);
	ok;
}

static int test_color0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "color:Light Green");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	notnull(pit->colors);
	eq(pit->colors->color, COLOUR_L_GREEN);
	/* Try a full color name with a different capitalization. */
	r = parser_parse(p, "color:light red");
	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	notnull(pit->colors);
	eq(pit->colors->color, COLOUR_L_RED);
	/* Try a single letter color code. */
	r = parser_parse(p, "color:y");
	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(pit);
	notnull(pit->colors);
	eq(pit->colors->color, COLOUR_YELLOW);
	ok;
}

static int test_flags_req0(void *state) {
	struct parser *p = (struct parser*) state;
	struct pit_profile *pit = (struct pit_profile*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[RF_SIZE];

	notnull(pit);
	rf_wipe(pit->flags);
	/* Check that giving no flags works. */
	r = parser_parse(p, "flags-req:");
	eq(r, PARSE_ERROR_NONE);
	require(rf_is_empty(pit->flags));
	/* Check that giving a single flag works. */
	r = parser_parse(p, "flags-req:UNDEAD");
	eq(r, PARSE_ERROR_NONE);
	/* Check that giving multiple flags works. */
	r = parser_parse(p, "flags-req:SMART | NO_CONF");
	eq(r, PARSE_ERROR_NONE);
	rf_wipe(eflags);
	rf_on(eflags, RF_UNDEAD);
	rf_on(eflags, RF_SMART);
	rf_on(eflags, RF_NO_CONF);
	require(rf_is_equal(pit->flags, eflags));
	ok;
}

static int test_flags_req_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an invalid flag. */
	enum parser_error r = parser_parse(p, "flags-req:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_flags_ban0(void *state) {
	struct parser *p = (struct parser*) state;
	struct pit_profile *pit = (struct pit_profile*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[RF_SIZE];

	notnull(pit);
	rf_wipe(pit->forbidden_flags);
	/* Check that giving no flags works. */
	r = parser_parse(p, "flags-ban:");
	eq(r, PARSE_ERROR_NONE);
	require(rf_is_empty(pit->forbidden_flags));
	/* Check that giving a single flag works. */
	r = parser_parse(p, "flags-ban:KILL_BODY");
	eq(r, PARSE_ERROR_NONE);
	/* Check that giving multiple flags works. */
	r = parser_parse(p, "flags-ban:PASS_WALL| KILL_WALL");
	eq(r, PARSE_ERROR_NONE);
	rf_wipe(eflags);
	rf_on(eflags, RF_KILL_BODY);
	rf_on(eflags, RF_PASS_WALL);
	rf_on(eflags, RF_KILL_WALL);
	require(rf_is_equal(pit->forbidden_flags, eflags));
	ok;
}

static int test_flags_ban_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an invalid flag. */
	enum parser_error r = parser_parse(p, "flags-ban:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_innate_freq0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "innate-freq:4");
	struct pit_profile *pit;

	eq(r, PARSE_ERROR_NONE);
	pit = (struct pit_profile*) parser_priv(p);
	notnull(p);
	/* Input value, v, is stored as 100 / v. */
	eq(pit->freq_innate, 25);
	ok;
}

static int test_innate_freq_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try values outside the accepted range to 1 to 100. */
	enum parser_error r = parser_parse(p, "innate-freq:0");

	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	r = parser_parse(p, "innate-freq:-1");
	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	r = parser_parse(p, "innate-freq:101");
	eq(r, PARSE_ERROR_INVALID_SPELL_FREQ);
	ok;
}

static int test_spell_req0(void *state) {
	struct parser *p = (struct parser*) state;
	struct pit_profile *pit = (struct pit_profile*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[RSF_SIZE];

	notnull(pit);
	rsf_wipe(pit->spell_flags);
	/* Check that giving no spell works. */
	r = parser_parse(p, "spell-req:");
	eq(r, PARSE_ERROR_NONE);
	require(rsf_is_empty(pit->spell_flags));
	/* Check that giving a single spell works. */
	r = parser_parse(p, "spell-req:BR_FIRE");
	eq(r, PARSE_ERROR_NONE);
	/* Check that giving multiple spells works. */
	r = parser_parse(p, "spell-req:SCARE | CONF");
	eq(r, PARSE_ERROR_NONE);
	rsf_wipe(eflags);
	rsf_on(eflags, RSF_BR_FIRE);
	rsf_on(eflags, RSF_SCARE);
	rsf_on(eflags, RSF_CONF);
	require(rsf_is_equal(pit->spell_flags, eflags));
	ok;
}

static int test_spell_req_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an invalid spell. */
	enum parser_error r = parser_parse(p, "spell-req:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_spell_ban0(void *state) {
	struct parser *p = (struct parser*) state;
	struct pit_profile *pit = (struct pit_profile*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[RSF_SIZE];

	notnull(pit);
	rsf_wipe(pit->forbidden_spell_flags);
	/* Check that giving no spell works. */
	r = parser_parse(p, "spell-ban:");
	eq(r, PARSE_ERROR_NONE);
	require(rsf_is_empty(pit->forbidden_spell_flags));
	/* Check that giving a single spell works. */
	r = parser_parse(p, "spell-ban:BR_COLD");
	eq(r, PARSE_ERROR_NONE);
	/* Check that giving multiple spells works. */
	r = parser_parse(p, "spell-ban:SCARE | CONF");
	eq(r, PARSE_ERROR_NONE);
	rsf_wipe(eflags);
	rsf_on(eflags, RSF_BR_COLD);
	rsf_on(eflags, RSF_SCARE);
	rsf_on(eflags, RSF_CONF);
	require(rsf_is_equal(pit->forbidden_spell_flags, eflags));
	ok;
}

static int test_spell_ban_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an invalid spell. */
	enum parser_error r = parser_parse(p, "spell-ban:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

const char *suite_name = "parse/pit";
/*
 * test_missing_record_header0() has to be before test_name0().
 * test_room0(), test_alloc0(), test_obj_rarity0(), test_mon_base0(),
 * test_mon_base_bad0(), test_mon_ban0(), test_mon_ban_bad0(), test_color0(),
 * test_flags_req0(), test_flags_req_bad0(), test_flags_ban0(),
 * test_flags_ban_bad0(), test_innate_freq0(), test_innate_freq_bad0(),
 * test_spell_req0(), test_spell_req_bad0(), test_spell_ban0(), and
 * test_spell_ban_bad0() have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "room0", test_room0 },
	{ "alloc0", test_alloc0 },
	{ "obj_rarity0", test_obj_rarity0 },
	{ "mon_base0", test_mon_base0 },
	{ "mon_base_bad0", test_mon_base_bad0 },
	{ "mon_ban0", test_mon_ban0 },
	{ "mon_ban_bad0", test_mon_ban_bad0 },
	{ "color0", test_color0 },
	{ "flags_req0", test_flags_req0 },
	{ "flags_req_bad0", test_flags_req_bad0 },
	{ "flags_ban0", test_flags_ban0 },
	{ "flags_ban_bad0",  test_flags_ban_bad0 },
	{ "innate_freq0", test_innate_freq0 },
	{ "innate_freq_bad0", test_innate_freq_bad0 },
	{ "spell_req0", test_spell_req0 },
	{ "spell_req_bad0", test_spell_req_bad0 },
	{ "spell_ban0", test_spell_ban0 },
	{ "spell_ban_bad0", test_spell_ban_bad0 },
	{ NULL, NULL }
};
