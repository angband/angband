/* parse/c-info */

#include "unit-test.h"

#include "effects.h"
#include "init.h"
#include "obj-properties.h"
#include "object.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "option.h"
#include "player.h"
#include "project.h"
#include "z-color.h"
#include "z-form.h"
#include <locale.h>
#include <langinfo.h>

static char dummy_torch_name[16] = "& Wooden Torch~";
static char dummy_lantern_name[16] = "& Lantern~";
static char dummy_rapier_name[16] = "& Rapier~";
static struct object_kind dummy_kinds[] = {
	{ .name = NULL, .tval = 0, .sval = 0 },
	{ .name = dummy_torch_name, .tval = TV_LIGHT, .sval = 1 },
	{ .name = dummy_lantern_name, .tval = TV_LIGHT, .sval = 2 },
	{ .name = dummy_rapier_name, .tval = TV_SWORD, .sval = 1 },
};
static char dummy_realm_name[16] = "arcane";
static struct magic_realm dummy_realms[] = {
	{ .next = NULL, .name = dummy_realm_name },
};

int setup_tests(void **state) {
	int i;
	bool last;

	*state = class_parser.init();
	/*
	 * Set up a minimal set of kinds so tests for starting equipment and
	 * spell books work.  The kinds need to reallocatable so kinds of books
	 * can be added.
	 */
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->k_max = (uint16_t) N_ELEMENTS(dummy_kinds);
	z_info->ordinary_kind_max = z_info->k_max;
	kb_info = mem_zalloc(TV_MAX * sizeof(*kb_info));
	for (i = TV_MAX - 1, last = true; i >= 0; --i) {
		kb_info[i].tval = i;
		if (last) {
			last = false;
			kb_info[i].next = NULL;
		} else {
			kb_info[i].next = kb_info + i + 1;
		}
	}
	k_info = mem_zalloc(z_info->k_max * sizeof(*k_info));
	for (i = (int) N_ELEMENTS(dummy_kinds) - 1, last = true; i >= 0; --i) {
		k_info[i].name = string_make(dummy_kinds[i].name);
		k_info[i].base = kb_info + dummy_kinds[i].tval;
		k_info[i].kidx = i;
		k_info[i].tval = dummy_kinds[i].tval;
		k_info[i].sval = dummy_kinds[i].sval;
		if (k_info[i].base->num_svals < k_info[i].sval) {
			k_info[i].base->num_svals = k_info[i].sval;
		}
		if (last) {
			last = false;
			k_info[i].next = NULL;
		} else {
			k_info[i].next = dummy_kinds + i + 1;
		}
	}
	/* Set up a minimal set of realms so tests for books and spells work. */
	realms = dummy_realms;
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_class *c = (struct player_class*) parser_priv(p);
	int i;

	string_free((char *)c->name);
	for (i = 0; i < PY_MAX_LEVEL / 5; i++) {
		string_free((char *)c->title[i]);
	}
	while (c->start_items) {
		struct start_item *si = c->start_items;

		c->start_items = si->next;
		mem_free(si->eopts);
		mem_free(si);
	}
	for (i = 0; i < c->magic.num_books; ++i) {
		int j;

		for (j = 0; j < c->magic.books[i].num_spells; ++j) {
			string_free(c->magic.books[i].spells[j].name);
			string_free(c->magic.books[i].spells[j].text);
			free_effect(c->magic.books[i].spells[j].effect);
		}
		mem_free(c->magic.books[i].spells);
	}
	mem_free(c->magic.books);
	mem_free(c);
	parser_destroy(p);
	for (i = 0; i < z_info->k_max; ++i) {
		string_free(k_info[i].name);
	}
	mem_free(k_info);
	mem_free(kb_info);
	mem_free(z_info);
	return 0;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_class *c = (struct player_class*) parser_priv(p);
	enum parser_error r;

	null(c);
	r = parser_parse(p, "stats:0:1:-3:3:-1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-disarm-phys:45:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-disarm-magic:45:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-device:32:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-save:28:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-stealth:3:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-search:20:16");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-melee:35:45");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-shoot:66:30");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-throw:55:45");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-dig:5:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "hitdie:9");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "exp:30");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "max-attacks:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "min-weight:40");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "strength-multiplier:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "title:Novice");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "equip:magic book:2:2:5:none");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "obj-flags:FREE_ACT | FEATHER");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "player-flags:BLESS_WEAPON | ZERO_FAIL");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "magic:3:400:9");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "book:magic book:town:[First Spells]:2:arcane");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "book-graphics:?:R");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "book-properties:25:40:1 to 100");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell:Light Room:1:2:26:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:LIGHT_AREA");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:22:40");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:D:PLAYER_LEVEL:- 1 / 5 + 3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:shadow shifting");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:Detects all traps, doors, and stairs in "
		"the immediate area.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:Ranger");
	struct player_class *c;
	int i;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->name);
	require(streq(c->name, "Ranger"));
	for (i = 0; i < 10; ++i) {
		null(c->title[i]);
	}
	for (i = 0; i < STAT_MAX; ++i) {
		eq(c->c_adj[i], 0);
	}
	for (i = 0; i < SKILL_MAX; ++i) {
		eq(c->c_skills[i], 0);
	}
	for (i = 0; i < SKILL_MAX; ++i) {
		eq(c->x_skills[i], 0);
	}
	eq(c->c_mhp, 0);
	eq(c->c_exp, 0);
	require(of_is_empty(c->flags));
	require(pf_is_empty(c->pflags));
	eq(c->max_attacks, 0);
	eq(c->min_weight, 0);
	eq(c->att_multiply, 0);
	null(c->start_items);
	eq(c->magic.spell_first, 0);
	eq(c->magic.spell_weight, 0);
	eq(c->magic.num_books, 0);
	null(c->magic.books);
	eq(c->magic.total_spells, 0);
	ok;
}

static int test_stats0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "stats:3:-3:2:-2:1");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_adj[STAT_STR], 3);
	eq(c->c_adj[STAT_INT], -3);
	eq(c->c_adj[STAT_WIS], 2);
	eq(c->c_adj[STAT_DEX], -2);
	eq(c->c_adj[STAT_CON], 1);
	ok;
}

static int test_skill_disarm_phys0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-disarm-phys:30:8");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_DISARM_PHYS], 30);
	eq(c->x_skills[SKILL_DISARM_PHYS], 8);
	ok;
}

static int test_skill_disarm_magic0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-disarm-magic:20:10");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_DISARM_MAGIC], 20);
	eq(c->x_skills[SKILL_DISARM_MAGIC], 10);
	ok;
}

static int test_skill_device0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-device:32:10");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_DEVICE], 32);
	eq(c->x_skills[SKILL_DEVICE], 10);
	ok;
}

static int test_skill_save0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-save:28:10");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_SAVE], 28);
	eq(c->x_skills[SKILL_SAVE], 10);
	ok;
}

static int test_skill_stealth0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-stealth:3:0");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_STEALTH], 3);
	eq(c->x_skills[SKILL_STEALTH], 0);
	ok;
}

static int test_skill_search0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-search:24:0");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_SEARCH], 24);
	eq(c->x_skills[SKILL_SEARCH], 0);
	ok;
}

static int test_skill_melee0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-melee:56:30");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_TO_HIT_MELEE], 56);
	eq(c->x_skills[SKILL_TO_HIT_MELEE], 30);
	ok;
}

static int test_skill_shoot0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-shoot:72:45");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_TO_HIT_BOW], 72);
	eq(c->x_skills[SKILL_TO_HIT_BOW], 45);
	ok;
}

static int test_skill_throw0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-throw:72:45");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_TO_HIT_THROW], 72);
	eq(c->x_skills[SKILL_TO_HIT_THROW], 45);
	ok;
}

static int test_skill_dig0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-dig:0:0");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_skills[SKILL_DIGGING], 0);
	eq(c->x_skills[SKILL_DIGGING], 0);
	ok;
}

static int test_hitdie0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "hitdie:4");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_mhp, 4);
	ok;
}

static int test_exp0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "exp:30");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->c_exp, 30);
	ok;
}

static int test_max_attacks0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "max-attacks:5");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->max_attacks, 5);
	ok;
}

static int test_min_weight0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "min-weight:35");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->min_weight, 35);
	ok;
}

static int test_strength_multiplier0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "strength-multiplier:4");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->att_multiply, 4);
	ok;
}

static int test_title0(void *state) {
	const char *titles[] = {
		"Runner",
		"Strider",
		"Scout",
		"Courser",
		"Tracker",
		"Guide",
		"Explorer",
		"Pathfinder",
		"Ranger",
		"Ranger Lord"
	};
	struct parser *p = (struct parser*) state;
	char buffer[80];
	struct player_class *c;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(titles); ++i) {
		enum parser_error r;
		size_t np = strnfmt(buffer, sizeof(buffer), "title:%s",
			titles[i]);

		require(np < sizeof(buffer));
		r = parser_parse(p, buffer);
		eq(r, PARSE_ERROR_NONE);
	}
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	for (i = 0; i < (int) N_ELEMENTS(titles); ++i) {
		notnull(c->title[i]);
		require(streq(c->title[i], titles[i]));
	}
	ok;
}

static int test_title_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * test_title0() should have filled all the titles so this should
	 * trigger an error.
	 */
	enum parser_error r = parser_parse(p, "title:One Title Too Many");

	eq(r, PARSE_ERROR_TOO_MANY_ENTRIES);
	ok;
}

static int test_equip0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "equip:magic book:2:2:5:none");
	struct player_class *c;
	const char *oname;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->start_items);
	eq(c->start_items->tval, TV_MAGIC_BOOK)
	eq(c->start_items->sval, 2);
	eq(c->start_items->min, 2);
	eq(c->start_items->max, 5);
	null(c->start_items->eopts);
	/* Check restriction of starting equipment by birth options. */
	r = parser_parse(p, "equip:light:lantern:1:1:birth_no_recall");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->start_items);
	eq(c->start_items->tval, TV_LIGHT);
	eq(c->start_items->sval, 2);
	eq(c->start_items->min, 1);
	eq(c->start_items->max, 1);
	notnull(c->start_items->eopts);
	require(c->start_items->eopts[0] > 0
		&& c->start_items->eopts[0] < OPT_MAX);
	oname = option_name(c->start_items->eopts[0]);
	notnull(oname);
	require(streq(oname, "birth_no_recall"));
	eq(c->start_items->eopts[1], 0);
	r = parser_parse(p, "equip:light:wooden torch:1:2:"
		"NOT-birth_no_recall | birth_force_descend");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->start_items);
	eq(c->start_items->tval, TV_LIGHT);
	eq(c->start_items->sval, 1);
	eq(c->start_items->min, 1);
	eq(c->start_items->max, 2);
	notnull(c->start_items->eopts);
	require(c->start_items->eopts[0] < 0
		&& c->start_items->eopts[0] > -OPT_MAX);
	oname = option_name(-(c->start_items->eopts[0]));
	notnull(oname);
	require(streq(oname, "birth_no_recall"));
	require(c->start_items->eopts[1] > 0
		&& c->start_items->eopts[1] < OPT_MAX);
	oname = option_name(c->start_items->eopts[1]);
	require(streq(oname, "birth_force_descend"));
	eq(c->start_items->eopts[2], 0);
	ok;
}

static int test_equip_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized tval. */
	enum parser_error r = parser_parse(p, "equip:xyzzy:1:1:3:none");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	/* Try an unrecognized sval. */
	r = parser_parse(p, "equip:light:xyzzy:1:3:none");
	eq(r, PARSE_ERROR_UNRECOGNISED_SVAL);
	/* Try an invalid number (rejects anything more than 99. */
	r = parser_parse(p, "equip:light:wooden torch:1:100:none");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	r = parser_parse(p, "equip:light:wooden torch:104:3:none");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	r = parser_parse(p, "equip:light:wooden torch:100:105:none");
	eq(r, PARSE_ERROR_INVALID_ITEM_NUMBER);
	/* Try tying it to an option that's not a birth option. */
	r = parser_parse(p, "equip:light:wooden torch:1:2:rogue_like_commands");
	eq(r, PARSE_ERROR_INVALID_OPTION);
	r = parser_parse(p, "equip:light:wooden torch:1:2:"
		"NOT-rogue_like_commands");
	eq(r, PARSE_ERROR_INVALID_OPTION);
	/* Try tying it to an invalid option. */
	r = parser_parse(p, "equip:light:wooden torch:1:2:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OPTION);
	r = parser_parse(p, "equip:light:wooden torch:1:2:NOT-xyzzy");
	eq(r, PARSE_ERROR_INVALID_OPTION);
	ok;
}

static int test_player_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_class *c = (struct player_class*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[PF_SIZE];

	notnull(c);
	pf_wipe(c->pflags);
	/* Check that supplying no flags works. */
	r = parser_parse(p, "player-flags:");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	require(pf_is_empty(c->pflags));
	/* Check that supplying one flag works. */
	r = parser_parse(p, "player-flags:ZERO_FAIL");
	eq(r, PARSE_ERROR_NONE);
	/* Check that supplying more than one flag at a time works. */
	r = parser_parse(p, "player-flags:BLESS_WEAPON | CHOOSE_SPELLS");
	eq(r, PARSE_ERROR_NONE);
	pf_wipe(eflags);
	pf_on(eflags, PF_ZERO_FAIL);
	pf_on(eflags, PF_BLESS_WEAPON);
	pf_on(eflags, PF_CHOOSE_SPELLS);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	require(pf_is_equal(c->pflags, eflags));
	ok;
}

static int test_player_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized player flag. */
	enum parser_error r = parser_parse(p, "player-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_obj_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_class *c = (struct player_class*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[OF_SIZE];

	notnull(c);
	of_wipe(c->flags);
	/* Check that specifying no flags works. */
	r = parser_parse(p, "obj-flags:");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	require(of_is_empty(c->flags));
	/*  Check that specifying one flag works. */
	r = parser_parse(p, "obj-flags:FEATHER");
	eq(r, PARSE_ERROR_NONE);
	/* Check that specifying more than one flag at a time works. */
	r = parser_parse(p, "obj-flags:SEE_INVIS | IMPAIR_HP");
	eq(r, PARSE_ERROR_NONE);
	of_wipe(eflags);
	of_on(eflags, OF_FEATHER);
	of_on(eflags, OF_SEE_INVIS);
	of_on(eflags, OF_IMPAIR_HP);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	require(pf_is_equal(c->flags, eflags));
	ok;
}

static int test_obj_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized player flag. */
	enum parser_error r = parser_parse(p, "obj-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_missing_magic0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * Specify anything about a book, spell, or spell's effect when there
	 * hasn't been a magic directive for the class should signal an error.
	 */
	enum parser_error r = parser_parse(p,
		"book:magic book:town:[First Spells]:7:arcane");

	eq(r, PARSE_ERROR_TOO_MANY_ENTRIES);
	r = parser_parse(p, "book-graphics:?:R");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "book-properties:25:40:1 to 100");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell:Light Room:1:2:26:4");
	eq(r, PARSE_ERROR_TOO_MANY_ENTRIES);
	r = parser_parse(p, "effect:LIGHT_AREA");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:22:40");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:D:PLAYER_LEVEL:- 1 / 5 + 3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:shadow shifting");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:Detects all traps, doors, and stairs in "
		"the immediate area.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_magic0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "magic:4:400:3");
	struct player_class *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->magic.spell_first, 4);
	eq(c->magic.spell_weight, 400);
	notnull(c->magic.books);
	ok;
}

static int test_missing_book0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * Specifying the book graphics, the book properties, a spell, or
	 * anything about a spell's effect without a prior book directive for
	 * the class should signal an error.
	 */
	enum parser_error r = parser_parse(p, "book-graphics:?:R");

	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "book-properties:25:40:1 to 100");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell:Light Room:1:2:26:4");
	eq(r, PARSE_ERROR_TOO_MANY_ENTRIES);
	r = parser_parse(p, "effect:LIGHT_AREA");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:22:40");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:$Dd4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:D:PLAYER_LEVEL:- 1 / 5 + 3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:shadow shifting");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p,
		"desc:Teleports you randomly up to 10 squares away.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_book0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"book:magic book:town:[First Spells]:2:arcane");
	struct player_class *c;
	struct object_kind *bk;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	eq(c->magic.books[0].tval, TV_MAGIC_BOOK);
	eq(c->magic.books[0].sval, 1);
	eq(c->magic.books[0].dungeon, false);
	eq(c->magic.books[0].num_spells, 0);
	notnull(c->magic.books[0].realm);
	ptreq(c->magic.books[0].realm, &dummy_realms[0]);
	notnull(c->magic.books[0].spells);
	/* Check that added book kind is okay. */
	bk = lookup_kind(TV_MAGIC_BOOK, 1);
	notnull(bk);
	eq(bk->tval, TV_MAGIC_BOOK);
	eq(bk->sval, 1);
	notnull(bk->name);
	require(streq(bk->name, "[First Spells]"));
	/*
	 * Since there's no special artifacts in these tests, these should
	 * be equal.
	 */
	eq(z_info->k_max, z_info->ordinary_kind_max);
	ok;
}

static int test_book_graphics0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with one letter code for the color. */
	enum parser_error r = parser_parse(p, "book-graphics:?:y");
	struct player_class *c;
	struct object_kind *bk;

	r = parser_parse(p, "book-graphics:?:y");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	bk = lookup_kind(c->magic.books[c->magic.num_books - 1].tval,
		c->magic.books[c->magic.num_books - 1].sval);
	notnull(bk);
	eq(bk->d_char, L'?');
	eq(bk->d_attr, COLOUR_YELLOW);
	/* Try with a full name for the color. */
	r = parser_parse(p, "book-graphics:_:Light Green");
	eq(r, PARSE_ERROR_NONE);
	eq(bk->d_char, L'_');
	eq(bk->d_attr, COLOUR_L_GREEN);
	/* Check that full name matching for the color is case insensitive. */
	r = parser_parse(p, "book-graphics:?:light red");
	eq(r, PARSE_ERROR_NONE);
	eq(bk->d_char, L'?');
	eq(bk->d_attr, COLOUR_L_RED);
	if (setlocale(LC_CTYPE, "") && streq(nl_langinfo(CODESET), "UTF-8")) {
		/*
		 * Check for glyph that is outside of the ASCII range.  Use
		 * the pound sign, Unicode U+00A3 or CA A3 as UTF-8.
		 */
		wchar_t wcs[3];
		size_t nc;

		r = parser_parse(p, "book-graphics:£:r");
		eq(r, PARSE_ERROR_NONE);
		nc = text_mbstowcs(wcs, "£", (int) N_ELEMENTS(wcs));
		eq(nc, 1);
		eq(bk->d_char, wcs[0]);
		eq(bk->d_attr, COLOUR_RED);
	}
	ok;
}

static int test_book_properties0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "book-properties:25:40:1 to 100");
	struct player_class *c;
	struct object_kind *bk;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	bk = lookup_kind(c->magic.books[c->magic.num_books - 1].tval,
		c->magic.books[c->magic.num_books - 1].sval);
	notnull(bk);
	eq(bk->cost, 25);
	eq(bk->alloc_prob, 40);
	eq(bk->level, 1);
	eq(bk->alloc_min, 1);
	eq(bk->alloc_max, 100);
	ok;
}

static int test_book_properties_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an invalid allocation range. */
	enum parser_error r = parser_parse(p, "book-properties:25:40:1 100");

	eq(r, PARSE_ERROR_INVALID_ALLOCATION);
	ok;
}

static int test_missing_spell0(void * state) {
	struct parser *p = (struct parser*) state;
	/*
	 * Specifying anything about a spell's effect without a spell directive
	 * for the current book should signal an error.
	 */
	enum parser_error r = parser_parse(p, "effect:LIGHT_AREA");

	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:22:40");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:$Dd4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:D:PLAYER_LEVEL:- 1 / 5 + 3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:shadow shifting");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p,
		"desc:Teleports you randomly up to 10 squares away.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_spell0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "spell:Light Room:1:2:26:4");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	notnull(s->name);
	require(streq(s->name, "Light Room"));
	null(s->text);
	null(s->effect);
	ptreq(s->realm, &dummy_realms[0]);
	eq(s->sidx, (c->magic.total_spells - 1))
	eq(s->bidx, (c->magic.num_books - 1));
	eq(s->slevel, 1);
	eq(s->smana, 2);
	eq(s->sfail, 26);
	eq(s->sexp, 4);
	ok;
}

static int test_missing_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_class *c = (struct player_class*) parser_priv(p);
	struct class_book *b;
	struct class_spell *s;
	enum parser_error r;

	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	null(s->effect);
	/*
	 * Supplying effect-yx, dice, expr, or effect-msg directives when the
	 * current spell doesn't yet have an effect set should not signal
	 * an error and not modify the spell.
	 */
	r = parser_parse(p, "effect-yx:11:22");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:10+8d4");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:S:PLAYER_LEVEL:* 2");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-msg:self sacrifice");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	null(s->effect);
	ok;
}

static int test_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an effect without subtype, radius, or other parameters. */
	enum parser_error r = parser_parse(p, "effect:LIGHT_AREA");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_LIGHT_AREA);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Try an effect with a subtype but no radius or other parameters. */
	r = parser_parse(p, "effect:BOLT_OR_BEAM:COLD");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_BOLT_OR_BEAM);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_COLD);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Try an effect with a subtype and radius but no other parameter. */
	r = parser_parse(p, "effect:BALL:FIRE:2");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_BALL);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_FIRE);
	eq(e->radius, 2);
	eq(e->other, 0);
	null(e->msg);
	/* Try an effect with subtype, radius, and other parameters. */
	r = parser_parse(p, "effect:SHORT_BEAM:ELEC:0:1");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_SHORT_BEAM);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_ELEC);
	eq(e->radius, 0);
	eq(e->other, 1);
	null(e->msg);
	ok;
}

static int test_effect_yx0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "effect-yx:22:40");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->y, 22);
	eq(e->x, 40);
	ok;
}

static int test_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "dice:8+1d10m5");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->dice);
	require(dice_test_values(e->dice, 8, 1, 10, 5));
	/* Check that repeated dice directive doesn't trigger a memory leak. */
	r = parser_parse(p, "dice:6+2d4");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->dice);
	require(dice_test_values(e->dice, 6, 2, 4, 0));
	ok;
}

static int test_dice_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect without dice. */
	enum parser_error r = parser_parse(p, "effect:BOLT:FIRE");

	/* Try an invalid dice expression. */
	r = parser_parse(p, "dice:1d4 + 1d8");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_missing_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect without dice. */
	enum parser_error r = parser_parse(p, "effect:BOLT:FIRE");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying an expression when there's not dice should not signal
	 * an error and leave the spell unmodified.
	 */
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:* 3 - 2");
	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	null(e->dice);
	ok;
}

static int test_expr0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect with dice. */
	enum parser_error r = parser_parse(p, "effect:BOLT:FIRE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$Dd8");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:D:PLAYER_LEVEL:/ 5 + 1");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_expr_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect with dice. */
	enum parser_error r = parser_parse(p, "effect:BOLT:FIRE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$Dd8");
	eq(r, PARSE_ERROR_NONE);
	/* Try to bind an expression with invalid operations. */
	r = parser_parse(p, "expr:D:PLAYER_LEVEL:^ 2");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/*
	 * Try to bind an expression to a variable that isn't in the
	 * dice string.
	 */
	r = parser_parse(p, "expr:E:PLAYER_LEVEL:* 4 - 3");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_effect_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "effect-msg:shadow shifting");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->msg);
	require(streq(e->msg, "shadow shifting"));
	/*
	 * Check that another directive for the same effect appends to what's
	 * there.
	 */
	r = parser_parse(p, "effect-msg: went wrong");
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->msg);
	require(streq(e->msg, "shadow shifting went wrong"));
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"desc:Shoots a bolt of frost that always hits its target.");
	struct player_class *c;
	struct class_book *b;
	struct class_spell *s;

	eq(r, PARSE_ERROR_NONE);
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	notnull(s->text);
	require(streq(s->text,
		"Shoots a bolt of frost that always hits its target."));
	/*
	 * Check that another directive for the same spell appends to what's
	 * there.
	 */
	r = parser_parse(p, "desc:  Sometimes a beam is fired instead.");
	c = (struct player_class*) parser_priv(p);
	notnull(c);
	notnull(c->magic.books);
	require(c->magic.num_books > 0);
	b = &c->magic.books[c->magic.num_books - 1];
	require(b->num_spells > 0);
	s = &b->spells[b->num_spells - 1];
	notnull(s->text);
	require(streq(s->text, "Shoots a bolt of frost that always hits "
		"its target.  Sometimes a beam is fired instead."));
	ok;
}

static int test_spell_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * test_book0() specified a maximum of two spells.  One was used in
	 * test_spell0().  Use up the other here.  The one after that should
	 * trigger an error.
	 */
	enum parser_error r = parser_parse(p, "spell:Test Spell 1:2:2:22:4");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "spell:Bad Spell:3:2:24:4");
	eq(r, PARSE_ERROR_TOO_MANY_ENTRIES);
	ok;
}

static int test_book_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try a book with an unrecognized tval. */
	enum parser_error r = parser_parse(p,
		"book:xyzzy:town:[Nothing]:6:arcane");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);

	/*
	 * test_magic0() specified a maximum of three books.  One was used up
	 * by test_book0(); the above bad tval shouldn't have used up one.  Use
	 * up the other two here.  The next book after that should trigger an
	 * error.
	 */
	r = parser_parse(p, "book:magic book:dungeon:[Test Book 1]:5:arcane");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "book:magic book:dungeon:[Test Book 2]:6:arcane");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "book:magic book:dungeon:[Bad Book 1]:4:arcane");
	eq(r, PARSE_ERROR_TOO_MANY_ENTRIES);
	ok;
}

static int test_magic_repeated0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * Having more than one magic directive for the same class should signal
	 * an error.
	 */
	enum parser_error r = parser_parse(p, "magic:1:350:5");

	eq(r, PARSE_ERROR_REPEATED_DIRECTIVE);
	ok;
}

const char *suite_name = "parse/c-info";
/*
 * test_missing_record_header0() has to be before test_name0().  All others,
 * except test_name0(), have to be after test_name0().  test_title_bad0() has
 * to be after test_title0().  test_missing_magic0() has to be before
 * test_magic0().  test_missing_book0() has to be after test_magic0() and
 * before test_book0().  test_book0() has to be after test_magic0().
 * test_book_graphics0(), test_book_properties0(), test_book_properties_bad0(),
 * and test_spell0() have to be after test_book0().  test_missing_spell0()
 * has to be after test_book0() and before test_spell0().
 * test_missing_effect0() has to be after test_spell0() and before
 * test_effect0(), test_bad_dice0(), test_missing_dice0(), test_expr0(), and
 * test_expr_bad0().  test_effect0(), test_desc0(), test_bad_dice0(),
 * test_missing_dice0(), test_expr0(), and test_expr_bad0() have to after
 * test_spell0().  test_effect_yx0(), test_dice0(), and test_effect_msg0()
 * have to be after test_effect0().  Run test_spell_bad0(), test_book_bad0(),
 * test_magic_repeated0() last in case they would interfere with the other
 * book and spell tests.
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "stats0", test_stats0 },
	{ "skill_disarm_phys0", test_skill_disarm_phys0 },
	{ "skill_disarm_magic0", test_skill_disarm_magic0 },
	{ "skill_device0", test_skill_device0 },
	{ "skill_save0", test_skill_save0 },
	{ "skill_stealth0", test_skill_stealth0 },
	{ "skill_search0", test_skill_search0 },
	{ "skill_melee0", test_skill_melee0 },
	{ "skill_shoot0", test_skill_shoot0 },
	{ "skill_throw0", test_skill_throw0 },
	{ "skill_dig0", test_skill_dig0 },
	{ "hitdie0", test_hitdie0 },
	{ "exp0", test_exp0 },
	{ "max_attacks0", test_max_attacks0 },
	{ "min_weight0", test_min_weight0 },
	{ "strength_multiplier0", test_strength_multiplier0 },
	{ "title0", test_title0 },
	{ "title_bad0", test_title_bad0 },
	{ "equip0", test_equip0 },
	{ "equip_bad0", test_equip_bad0 },
	{ "player_flags0", test_player_flags0 },
	{ "player_flags_bad0", test_player_flags_bad0 },
	{ "obj_flags0", test_obj_flags0 },
	{ "obj_flags_bad0", test_obj_flags_bad0 },
	{ "missing_magic0", test_missing_magic0 },
	{ "magic0", test_magic0 },
	{ "missing_book0", test_missing_book0 },
	{ "book0", test_book0 },
	{ "book_graphics0", test_book_graphics0 },
	{ "book_properties0", test_book_properties0 },
	{ "book_properties_bad0", test_book_properties_bad0 },
	{ "missing_spell0", test_missing_spell0 },
	{ "spell0", test_spell0 },
	{ "missing_effect0", test_missing_effect0 },
	{ "effect0", test_effect0 },
	{ "effect_yx0", test_effect_yx0 },
	{ "dice0", test_dice0 },
	{ "dice_bad0", test_dice_bad0 },
	{ "missing_dice0", test_missing_dice0 },
	{ "expr0", test_expr0 },
	{ "expr_bad0", test_expr_bad0 },
	{ "effect_msg0", test_effect_msg0 },
	{ "desc0", test_desc0 },
	{ "spell_bad0", test_spell_bad0 },
	{ "book_bad0", test_book_bad0 },
	{ "magic_repeated0", test_magic_repeated0 },
	{ NULL, NULL }
};
