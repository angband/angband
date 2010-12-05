/* unit-test-data.h
 * Predefined data for tests
 */

#ifndef UNIT_TEST_DATA
#define UNIT_TEST_DATA

#include "angband.h"
#include "object/tvalsval.h"

static struct player_sex test_sex = {
	.title = "Test Sex",
	.winner = "Test Winner",
};

static struct object_kind test_longsword = {
	.name = "Test Longsword",
	.text = "A test longsword [0].",
	.kidx = 0,
	.tval = TV_SWORD,
	.sval = SV_LONG_SWORD,
	.pval = 0,

	.to_h = 1,
	.to_d = 1,
	.to_a = 2,

	.dd = 4,
	.ds = 6,
	.weight = 16,

	.cost = 20,

	.d_attr = 0,
	.d_char = '|',

	.alloc_prob = 20,
	.alloc_min = 1,
	.alloc_max = 10,
	.level = 0,

	.effect = 0,
	.gen_mult_prob = 0,
	.flavor = NULL,
};

static struct object_kind test_torch = {
	.name = "Test Torch",
	.text = "A test torch [1].",
	.kidx = 1,
	.tval = TV_LIGHT,
	.sval = SV_LIGHT_TORCH,
	.pval = 5000,

	.to_h = 0,
	.to_d = 0,
	.to_a = 0,

	.dd = 1,
	.ds = 1,
	.weight = 10,

	.cost = 1,

	.d_attr = 0,
	.d_char = '~',

	.alloc_prob = 10,
	.alloc_min = 1,
	.alloc_max = 10,
	.level = 0,

	.effect = 0,
	.gen_mult_prob = 0,
	.flavor = NULL,
};

static struct object_kind test_gold = {
	.name = "Test Gold",
	.text = "Test gold [2].",
	.kidx = 2,
	.tval = TV_GOLD,
	.sval = 0,
	.pval = 0,

	.to_h = 0,
	.to_d = 0,
	.to_a = 0,

	.dd = 1,
	.ds = 1,
	.weight = 1,

	.cost = 0,

	.d_attr = 0,
	.d_char = '$',

	.alloc_prob = 0,
	.alloc_min = 0,
	.alloc_max = 0,
	.level = 0,

	.effect = 0,
	.gen_mult_prob = 0,
	.flavor = NULL,
};

static struct player_race test_race = {
	.name = "TestRace",
	.r_adj = {
		[A_STR] = +2,
		[A_DEX] = +1,
		[A_CON] = +3,
		[A_INT] = -1,
		[A_WIS] = -2,
		[A_CHR] = +0,
	},
	.r_skills = {
		[SKILL_DISARM] = 0,
		[SKILL_DEVICE] = 5,
		[SKILL_SAVE] = 10,
		[SKILL_STEALTH] = -5,
		[SKILL_SEARCH] = -10,
		[SKILL_SEARCH_FREQUENCY] = 10,
		[SKILL_TO_HIT_MELEE] = 0,
		[SKILL_TO_HIT_BOW] = 0,
		[SKILL_TO_HIT_THROW] = 0,
		[SKILL_DIGGING] = 0,
	},

	.r_mhp = 10,
	.r_exp = 110,

	.b_age = 14,
	.m_age = 6,

	.m_b_ht = 72,
	.m_m_ht = 6,
	.f_b_ht = 66,
	.f_m_ht = 4,

	.m_b_wt = 180,
	.m_m_wt = 25,
	.f_b_wt = 150,
	.f_m_wt = 20,

	.infra = 40,

	.choice = 0xFF,

	.hist = 0,
};

static struct player_class test_class = {
	.name = "TestClass",
	.title = {
		"TestTitle0",
		"TestTitle1",
		"TestTitle2",
		"TestTitle3",
		"TestTitle4",
		"TestTitle5",
		"TestTitle6",
		"TestTitle7",
		"TestTitle8",
		"TestTitle9",
	},

	.c_adj = {
		[A_STR] = +1,
		[A_DEX] = +2,
		[A_CON] = -1,
		[A_INT] = -2,
		[A_WIS] = +3,
		[A_CHR] = +0,
	},

	.c_skills = {
		[SKILL_DISARM] = 25,
		[SKILL_DEVICE] = 18,
		[SKILL_SAVE] = 18,
		[SKILL_STEALTH] = 1,
		[SKILL_SEARCH] = 14,
		[SKILL_SEARCH_FREQUENCY] = 2,
		[SKILL_TO_HIT_MELEE] = 70,
		[SKILL_TO_HIT_BOW] = 55,
		[SKILL_TO_HIT_THROW] = 55,
		[SKILL_DIGGING] = 0,
	},

	.x_skills = {
		[SKILL_DISARM] = 10,
		[SKILL_DEVICE] = 7,
		[SKILL_SAVE] = 10,
		[SKILL_STEALTH] = 0,
		[SKILL_SEARCH] = 0,
		[SKILL_SEARCH_FREQUENCY] = 0,
		[SKILL_TO_HIT_MELEE] = 45,
		[SKILL_TO_HIT_BOW] = 45,
		[SKILL_TO_HIT_THROW] = 45,
		[SKILL_DIGGING] = 0,
	},

	.c_mhp = 9,
	.c_exp = 0,

	.max_attacks = 6,
	.min_weight = 30,
	.att_multiply = 5,

	.spell_book = 0,
	.spell_stat = 0,
	.spell_first = 0,
	.spell_weight = 0,

	.sense_base = 7000,
	.sense_div = 40,

	.start_items = {
		{ &test_longsword, 1, 1 },
		{ &test_torch, 3, 5 },
	},
};

#endif /* !UNIT_TEST_DATA */
