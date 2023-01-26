/* parse/e-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "effects.h"
#include "init.h"
#include "object.h"
#include "obj-curse.h"
#include "obj-init.h"
#include "obj-make.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "project.h"
#include "z-virt.h"


static char dummy_cloak_name[16] = "& Cloak~";
static char dummy_fur_cloak_name[16] = "& Fur Cloak~";
static char dummy_dagger_name[16] = "& Dagger~";
static char dummy_rapier_name[16] = "& Rapier~";
static char dummy_skullcap_name[16] = "& Skullcap~";
static char dummy_helm_name[16] = "& Steel Helm~";
static struct object_kind dummy_kinds[] = {
	{ .name = dummy_cloak_name, .kidx = 0, .tval = TV_CLOAK, .sval = 1 },
	{ .name = dummy_fur_cloak_name, .kidx = 1, .tval = TV_CLOAK, .sval = 2 },
	{ .name = dummy_dagger_name, .kidx = 2, .tval = TV_SWORD, .sval = 1 },
	{ .name = dummy_rapier_name, .kidx = 3, .tval = TV_SWORD, .sval = 2 },
	{ .name = dummy_skullcap_name, .kidx = 4, .tval = TV_HELM, .sval = 1 },
	{ .name = dummy_helm_name, .kidx = 5, .tval = TV_HELM, .sval = 2 },
};
static char dummy_cure_pois_act[16] = "CURE_POISON";
static char dummy_restore_mana_act[16] = "RESTORE_MANA";
static char dummy_illum_act[16] = "ILLUMINATION";
static struct activation dummy_activations[] = {
	{ .name = NULL, .index = 0 },
	{ .name = dummy_cure_pois_act, .index = 1 },
	{ .name = dummy_restore_mana_act, .index = 2 },
	{ .name = dummy_illum_act, .index = 3 },
};
static char dummy_orc_slay[16] = "ORC_3";
static char dummy_animal_slay[16] = "ANIMAL_2";
static struct slay dummy_slays[] = {
	{ .code = NULL },
	{ .code = dummy_orc_slay },
	{ .code = dummy_animal_slay },
};
static char dummy_cold_brand[16] = "COLD_2";
static char dummy_acid_brand[16] = "ACID_3";
static struct brand dummy_brands[] = {
	{ .code = NULL },
	{ .code = dummy_cold_brand },
	{ .code = dummy_acid_brand },
};
static char dummy_vuln_curse[16] = "vulnerability";
static char dummy_tele_curse[16] = "teleportation";
static struct curse dummy_curses[] = {
	{ .name = NULL },
	{ .name = dummy_vuln_curse },
	{ .name = dummy_tele_curse },
};

int setup_tests(void **state) {
	*state = ego_parser.init();
	/*
	 * Do minimal setup for adding of activations, slays, brands, and
	 * curses and for kind lookup.  z_info is also used by
	 * ego_parser.finish.
	 */
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->k_max = (uint16_t) N_ELEMENTS(dummy_kinds);
	k_info = dummy_kinds;
	z_info->act_max = (uint16_t) N_ELEMENTS(dummy_activations);
	activations = dummy_activations;
	if (z_info->act_max > 1) {
		size_t i;

		for (i = 0; i < (size_t) z_info->act_max - 1; ++i) {
			activations[i].next = activations + i + 1;
		}
	}
	if (z_info->act_max > 0) {
		activations[z_info->act_max - 1].next = NULL;
	}
	z_info->slay_max = (uint8_t) N_ELEMENTS(dummy_slays);
	slays = dummy_slays;
	z_info->brand_max = (uint8_t) N_ELEMENTS(dummy_brands);
	brands = dummy_brands;
	z_info->curse_max = (uint8_t) N_ELEMENTS(dummy_curses);
	curses = dummy_curses;
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (ego_parser.finish(p)) {
		r = 1;
	}
	ego_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ego_item *e = (struct ego_item*) parser_priv(p);
	enum parser_error r;

	null(e);
	r  = parser_parse(p, "info:1000:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "alloc:40:10 to 100");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "type:sword");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "item:helm:Skullcap");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "combat:d6:d6:d4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "min-combat:15:255:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "act:ILLUMINATION");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "time:30+1d30");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:FEATHER | IGNORE_FIRE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags-off:TAKES_FUEL | HATES_ACID");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "values:STEALTH[d2]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "min-values:STEALTH[0] | SPEED[0]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:They boost your to-hit and to-dam values.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "slay:ORC_3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "brand:COLD_2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "curse:vulnerability:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_order(void *state) {
	struct parser* p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "info:4");

	eq(r, PARSE_ERROR_MISSING_FIELD);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:of Resist Lightning");
	struct ego_item *e;
	int i;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	require(streq(e->name, "of Resist Lightning"));
	null(e->text);
	eq(e->cost, 0);
	require(of_is_empty(e->flags));
	require(of_is_empty(e->flags_off));
	require(kf_is_empty(e->kind_flags));
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		eq(e->modifiers[i].base, 0);
		eq(e->modifiers[i].dice, 0);
		eq(e->modifiers[i].sides, 0);
		eq(e->modifiers[i].m_bonus, 0);
	}
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		eq(e->min_modifiers[i], 0);
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(e->el_info[i].flags, 0);
		eq(e->el_info[i].res_level, 0);
	}
	null(e->brands);
	null(e->slays);
	null(e->curses);
	eq(e->rating, 0);
	eq(e->alloc_prob, 0);
	eq(e->alloc_min, 0);
	eq(e->alloc_max, 0);
	null(e->poss_items);
	eq(e->to_h.base, 0);
	eq(e->to_h.dice, 0);
	eq(e->to_h.sides, 0);
	eq(e->to_h.m_bonus, 0);
	eq(e->to_d.base, 0);
	eq(e->to_d.dice, 0);
	eq(e->to_d.sides, 0);
	eq(e->to_d.m_bonus, 0);
	eq(e->to_a.base, 0);
	eq(e->to_a.dice, 0);
	eq(e->to_a.sides, 0);
	eq(e->to_a.m_bonus, 0);
	eq(e->min_to_h, NO_MINIMUM);
	eq(e->min_to_d, NO_MINIMUM);
	eq(e->min_to_a, NO_MINIMUM);
	null(e->activation);
	eq(e->time.base, 0);
	eq(e->time.dice, 0);
	eq(e->time.sides, 0);
	eq(e->time.m_bonus, 0);
	eq(e->everseen, false);
	ok;
}

static int test_info0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "info:6:8");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	eq(e->cost, 6);
	eq(e->rating, 8);
	ok;
}

static int test_alloc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "alloc:40:10 to 100");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	eq(e->alloc_prob, 40);
	eq(e->alloc_min, 10);
	eq(e->alloc_max, 100);
	ok;
}

static int test_alloc_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with a mismatching string as the second parameter. */
	enum parser_error r = parser_parse(p, "alloc:40:10 100");

	eq(r, PARSE_ERROR_INVALID_ALLOCATION);
	/* Try with allocation ranges that are out of bounds. */
	r = parser_parse(p, "alloc:40:-1 to 100");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	r = parser_parse(p, "alloc:40:0 to 290");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	r = parser_parse(p, "alloc:40:370 to 40");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	r = parser_parse(p, "alloc:40:30 to -7");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	r = parser_parse(p, "alloc:40:-70 to -3");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	r = parser_parse(p, "alloc:40:-10 to 371");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	r = parser_parse(p, "alloc:40:268 to 500");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	ok;
}

static int test_type0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type:sword");
	bool valid = true;
	struct ego_item *e;
	struct poss_item *pi;
	bool *marked;
	int i;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	/* Find all the swords that were included. */
	marked = mem_zalloc(z_info->k_max * sizeof(*marked));
	for (pi = e->poss_items; pi; pi = pi->next) {
		if (pi->kidx < z_info->k_max) {
			if (k_info[pi->kidx].tval == TV_SWORD) {
				marked[pi->kidx] = true;
			}
		} else {
			valid = false;
		}
	}
	/*
	 * Now check the the list of all kinds to see if all swords were
	 * included.
	 */
	for (i = 0; i < z_info->k_max; ++i) {
		if (k_info[i].tval == TV_SWORD && !marked[i]) {
			valid = false;
		}
	}
	mem_free(marked);
	eq(valid, true);
	ok;
}

static int test_type_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check for an unrecognized tval. */
	enum parser_error r = parser_parse(p, "type:xyzzy");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	/* Check for a tval that has no kinds. */
	r = parser_parse(p, "type:light");
	eq(r, PARSE_ERROR_NO_KIND_FOR_EGO_TYPE);
	ok;
}

static int test_item0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "item:helm:Skullcap");
	bool valid = true, found = false;
	struct ego_item *e;
	struct poss_item *pi;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	for (pi = e->poss_items; pi && !found; pi = pi->next) {
		if (pi->kidx < z_info->k_max) {
			if (k_info[pi->kidx].tval == TV_HELM
					&& my_stristr(k_info[pi->kidx].name,
					"Skullcap")) {
				found = true;
			}
		} else {
			valid = false;
		}
	}
	eq((valid && found), true);
	ok;
}

static int test_item_bad0(void* state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized tval. */
	enum parser_error r = parser_parse(p, "item:xyzzy:Dagger");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	/* Try a valid tval but with an sval that isn't in it. */
	r = parser_parse(p, "item:sword:Skullcap");
	eq(r, PARSE_ERROR_UNRECOGNISED_SVAL);
	ok;
}

static int test_combat0(void *state) {
	struct parser* p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "combat:1d2:3d4:5d6");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	eq(e->to_h.dice, 1);
	eq(e->to_h.sides, 2);
	eq(e->to_d.dice, 3);
	eq(e->to_d.sides, 4);
	eq(e->to_a.dice, 5);
	eq(e->to_a.sides, 6);
	ok;
}

static int test_min0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "min-combat:10:13:4");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	eq(e->min_to_h, 10);
	eq(e->min_to_d, 13);
	eq(e->min_to_a, 4);
	ok;
}

static int test_act0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "act:ILLUMINATION");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	notnull(e->activation);
	notnull(e->activation->name);
	require(streq(e->activation->name, "ILLUMINATION"));
	ok;
}

static int test_act_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "act:XYZZY");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	null(e->activation);
	ok;
}

static int test_time0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "time:100+1d200");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	eq(e->time.base, 100);
	eq(e->time.dice, 1);
	eq(e->time.sides, 200);
	eq(e->time.m_bonus, 0);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ego_item *e = (struct ego_item*) parser_priv(p);
	enum parser_error r;
	int i;

	/* Wipe the slate. */
	notnull(e);
	of_wipe(e->flags);
	kf_wipe(e->kind_flags);
	for (i = 0; i < ELEM_MAX; ++i) {
		e->el_info[i].flags = 0;
	}
	/* Verify that an empty set of flags works. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	/* Try an object flag. */
	r = parser_parse(p, "flags:SEE_INVIS");
	eq(r, PARSE_ERROR_NONE);
	/* Try a kind flag and an element flag. */
	r = parser_parse(p, "flags:RAND_POWER | IGNORE_ACID");
	eq(r, PARSE_ERROR_NONE);
	/* Verify that the state is correct. */
	require(of_has(e->flags, OF_SEE_INVIS));
	of_off(e->flags, OF_SEE_INVIS);
	require(of_is_empty(e->flags));
	require(kf_has(e->kind_flags, KF_RAND_POWER));
	kf_off(e->kind_flags, KF_RAND_POWER);
	require(kf_is_empty(e->kind_flags));
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(e->el_info[i].flags,
			((i == ELEM_ACID) ? EL_INFO_IGNORE : 0));
	}
	ok;
}

static int test_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized flag. */
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	/* Try an unrecognized element. */
	r = parser_parse(p, "flags:HATES_XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "flags:IGNORE_XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_flags_off0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ego_item *e = (struct ego_item*) parser_priv(p);
	enum parser_error r;
	bitflag expected[OF_SIZE];

	notnull(e);
	/* Clear any prior settings. */
	of_wipe(e->flags_off);
	/*
	 * Setting flags-off with nothing should be successful but not do
	 * anything.
	 */
	r = parser_parse(p, "flags-off:");
	eq(r, PARSE_ERROR_NONE);
	require(of_is_empty(e->flags_off));
	/* Try with one flag. */
	r = parser_parse(p, "flags-off:FEATHER");
	eq(r, PARSE_ERROR_NONE);
	/* Try with two flags. */
	r = parser_parse(p, "flags-off:SEE_INVIS | PROT_FEAR");
	of_wipe(expected);
	of_on(expected, OF_FEATHER);
	of_on(expected, OF_SEE_INVIS);
	of_on(expected, OF_PROT_FEAR);
	require(of_is_equal(expected, e->flags_off));
	ok;
}

static int test_flags_off_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an unrecognized flag. */
	enum parser_error r = parser_parse(p, "flags-off:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_values0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ego_item *e = (struct ego_item*) parser_priv(p);
	enum parser_error r;
	int i;

	notnull(e);
	/* Clear any prior settings. */
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		e->modifiers[i].base = 0;
		e->modifiers[i].dice = 0;
		e->modifiers[i].sides = 0;
		e->modifiers[i].m_bonus = 0;
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		e->el_info[i].res_level = 0;
	}
	/* Try setting one object modifier. */
	r = parser_parse(p, "values:STEALTH[1+2d3]");
	eq(r, PARSE_ERROR_NONE);
	/* Try setting an object modifier and a resistance. */
	r = parser_parse(p, "values:INFRA[3] | RES_POIS[1]");
	eq(r, PARSE_ERROR_NONE);
	/* Check the state. */
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		if (i == OBJ_MOD_STEALTH) {
			eq(e->modifiers[i].base, 1);
			eq(e->modifiers[i].dice, 2);
			eq(e->modifiers[i].sides, 3);
			eq(e->modifiers[i].m_bonus, 0);
		} else if (i == OBJ_MOD_INFRA) {
			eq(e->modifiers[i].base, 3);
			eq(e->modifiers[i].dice, 0);
			eq(e->modifiers[i].sides, 0);
			eq(e->modifiers[i].m_bonus, 0);
		} else {
			eq(e->modifiers[i].base, 0);
			eq(e->modifiers[i].dice, 0);
			eq(e->modifiers[i].sides, 0);
			eq(e->modifiers[i].m_bonus, 0);
		}
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(e->el_info[i].res_level, ((i == ELEM_POIS) ? 1 : 0));
	}
	ok;
}

static int test_values_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized object modifier. */
	enum parser_error r = parser_parse(p, "values:XYZZY[2]");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	/* Try an unrecognized element. */
	r = parser_parse(p, "values:RES_XYZZY[3]");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_min_values0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ego_item *e = (struct ego_item*) parser_priv(p);
	enum parser_error r;
	int i;

	notnull(e);
	/* Clear any prior state. */
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		e->min_modifiers[i] = 0;
	}
	/* Try one object modifier. */
	r = parser_parse(p, "min-values:SPEED[1]");
	eq(r, PARSE_ERROR_NONE);
	/* Try try two object modifiers in the same directive. */
	r = parser_parse(p, "min-values:STEALTH[2] | INFRA[4]");
	eq(r, PARSE_ERROR_NONE);
	/* Check state. */
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		if (i == OBJ_MOD_SPEED) {
			eq(e->min_modifiers[i], 1);
		} else if (i == OBJ_MOD_STEALTH){
			eq(e->min_modifiers[i], 2);
		} else {
			eq(e->min_modifiers[i], ((i == OBJ_MOD_INFRA) ?
				4 : 0));
		}
	}
	ok;
}

static int test_min_values_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized object modifier. */
	enum parser_error r = parser_parse(p, "min-values:XYZZY[3]");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:foo");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "desc: bar");
	eq(r, PARSE_ERROR_NONE);

	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	require(streq(e->text, "foo bar"));
	ok;
}

static int test_slay0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "slay:ANIMAL_2");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	notnull(e->slays);
	eq(e->slays[0], false);
	eq(e->slays[1], false);
	eq(e->slays[2], true);
	ok;
}

static int test_slay_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "slay:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_SLAY);
	ok;
}

static int test_brand0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "brand:COLD_2");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	notnull(e->brands);
	eq(e->brands[0], false);
	eq(e->brands[1], true);
	eq(e->brands[2], false);
	ok;
}

static int test_brand_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "brand:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_BRAND);
	ok;
}

static int test_curse0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * Check that adding a curse with a non-positive power has no effect
	 * and does not flag an error.
	 */
	enum parser_error r = parser_parse(p, "curse:teleportation:0");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct ego_item*) parser_priv(p);
	notnull(e);
	null(e->curses);
	r = parser_parse(p, "curse:teleportation:-8");
	eq(r, PARSE_ERROR_NONE);
	null(e->curses);
	/* Check that adding with a positive power does have an effect. */
	r = parser_parse(p, "curse:vulnerability:12");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->curses);
	eq(e->curses[0], 0);
	eq(e->curses[1], 12);
	eq(e->curses[2], 0);
	ok;
}

static int test_curse_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "curse:xyzzy:5");

	eq(r, PARSE_ERROR_UNRECOGNISED_CURSE);
	ok;
}

const char *suite_name = "parse/e-info";
/*
 * test_missing_record_header0() has to be before test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "order", test_order },
	{ "name0", test_name0 },
	{ "info0", test_info0 },
	{ "alloc0", test_alloc0 },
	{ "alloc_bad0", test_alloc_bad0 },
	{ "type0", test_type0 },
	{ "type_bad0", test_type_bad0 },
	{ "item0", test_item0 },
	{ "item_bad0", test_item_bad0 },
	{ "combat0", test_combat0 },
	{ "min_combat0", test_min0 },
	{ "act0", test_act0 },
	{ "act_bad0", test_act_bad0 },
	{ "time0", test_time0 },
	{ "flags0", test_flags0 },
	{ "flags_bad0", test_flags_bad0 },
	{ "flags_off0", test_flags_off0 },
	{ "flags_off_bad0", test_flags_off_bad0 },
	{ "values0", test_values0 },
	{ "values_bad0", test_values_bad0 },
	{ "min_values0", test_min_values0 },
	{ "min_values_bad0", test_min_values_bad0 },
	{ "desc0", test_desc0 },
	{ "slay0", test_slay0 },
	{ "slay_bad0", test_slay_bad0 },
	{ "brand0", test_brand0 },
	{ "brand_bad0", test_brand_bad0 },
	{ "curse0", test_curse0 },
	{ "curse_bad0", test_curse_bad0 },
	{ NULL, NULL }
};
