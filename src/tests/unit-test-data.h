/* unit-test-data.h
 * Predefined data for tests
 */

#ifndef UNIT_TEST_DATA
#define UNIT_TEST_DATA

#ifndef TEST_DATA
  #ifdef __GNUC__
    #define TEST_DATA __attribute__ ((unused))
  #else
    #define TEST_DATA 
  #endif
#endif /* TEST_DATA */

#include "angband.h"
#include "object/tvalsval.h"
#include "player/types.h"

static struct player_sex TEST_DATA test_sex = {
	.title = "Test Sex",
	.winner = "Test Winner",
};

static struct object_base TEST_DATA sword_base = {
	.name = "Test Sword",
	.tval = TV_SWORD,
	.next = NULL,
	.break_perc = 50,
};

static struct artifact TEST_DATA test_artifact_sword = {
	.name = "Test Artifact",
	.text = "A test artifact.",
	.aidx = 0,
	.next = NULL,
	.tval = TV_SWORD,
	.sval = SV_LONG_SWORD,
	.to_a = 1,
	.to_h = 2,
	.to_d = 3,
	.ac = 5,
	.dd = 2,
	.ds = 5,
	.weight = 16,
	.cost = 40,
};

static struct object_kind TEST_DATA test_longsword = {
	.name = "Test Longsword",
	.text = "A test longsword [0].",
	.base = &sword_base,
	.kidx = 0,
	.tval = TV_SWORD,
	.sval = SV_LONG_SWORD,
	.pval = {
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
	},

	.to_h = {
			.base = 1,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},
	.to_d = {
			.base = 1,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},
	.to_a = {
			.base = 2,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},

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

static struct object_kind TEST_DATA test_torch = {
	.name = "Test Torch",
	.text = "A test torch [1].",
	.kidx = 1,
	.tval = TV_LIGHT,
	.sval = SV_LIGHT_TORCH,
	.pval = {
			{
				.base = 5000,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
	},

	.to_h = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},
	.to_d = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},
	.to_a = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},

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

static struct object_kind TEST_DATA test_gold = {
	.name = "Test Gold",
	.text = "Test gold [2].",
	.kidx = 2,
	.tval = TV_GOLD,
	.sval = 0,
	.pval = {
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
			{
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
			},
	},

	.to_h = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},
	.to_d = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},
	.to_a = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
	},

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

static struct player_race TEST_DATA test_race = {
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

	.history = NULL,
};

static struct start_item TEST_DATA start_torch = {
	.kind = &test_torch,
	.min = 3,
	.max = 5,
	.next = NULL,
};

static struct start_item TEST_DATA start_longsword = {
	.kind = &test_longsword,
	.min = 1,
	.max = 1,
	.next = &start_torch,
};

static struct player_class TEST_DATA test_class = {
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

	.start_items = &start_longsword,
};

static struct monster_base TEST_DATA test_rb_info = {
	.next = NULL,
	.name = "townsfolk",
	.text = "Townsfolk",
	.flags = "\0\0\0\0\0\0\0\0\0\0\0\0",
	.spell_flags = "\0\0\0\0\0\0\0\0\0\0\0\0",
	.d_char = 116,
	.pain = NULL,
	
};

#define _NOBLOW { .method = RBM_NONE, .effect = RBE_NONE, .d_dice = 0, .d_side = 0 }

static struct monster_race TEST_DATA test_r_human = {
	.next = NULL,
	.ridx = 0,
	.name = "Human",
	.text = "A random test human",

	.base = &test_rb_info,

	.avg_hp = 10,
	.ac = 12,
	.sleep = 0,
	.aaf = 20,
	.speed = 110,
	.mexp = 50,
	.power = 1,
	.scaled_power = 1,
	.highest_threat = 5,
	.freq_innate = 0,
	.freq_spell = 0,

	.blow = {
		{
			.method = RBM_HIT,
			.effect = RBE_HURT,
			.d_dice = 3,
			.d_side = 1,
		},
		_NOBLOW,
		_NOBLOW,
		_NOBLOW,
	},

	.level = 1,
	.rarity = 1,

	.d_attr = 0,
	.d_char = 't',

	.x_attr = 0,
	.x_char = 't',

	.max_num = 100,
	.cur_num = 0,

	.drops = NULL,
};

#undef _NOBLOW

static struct maxima TEST_DATA test_z_info = {
	.f_max   = 2,
	.k_max   = 2,
	.a_max   = 2,
	.e_max   = 2,
	.r_max   = 2,
	.mp_max  = 2,
	.s_max   = 2,
	.pit_max = 2,
	.o_max   = 2,
	.m_max   = 2,
};

static struct object TEST_DATA test_inven[ALL_INVEN_TOTAL];

static struct player TEST_DATA test_player = {
	.py = 1,
	.px = 1,
	.psex = 0,
	.sex = &test_sex,
	.race = &test_race,
	.class = &test_class,
	.hitdie = 10,
	.expfact = 100,
	.age = 12,
	.ht = 40,
	.wt = 80,
	.sc = 100,
	.au = 500,
	.max_depth = 10,
	.depth = 6,
	.max_lev = 3,
	.lev = 3,
	.max_exp = 100,
	.exp = 80,
	.mhp = 20,
	.chp = 14,
	.msp = 12,
	.csp = 11,
	.stat_max = {
		[A_STR] = 14,
		[A_DEX] = 12,
		[A_CON] = 14,
		[A_WIS] = 10,
		[A_INT] = 8,
		[A_CHR] = 12,
	},
	.stat_cur = {
		[A_STR] = 14,
		[A_DEX] = 11,
		[A_CON] = 14,
		[A_WIS] = 10,
		[A_INT] = 8,
		[A_CHR] = 8,
	},
	.word_recall = 0,
	.energy = 100,
	.food = 5000,
	.player_hp = {
		  5,  10,  15,  20,  25,  30,  35,  40,  45,  50,
		 55,  60,  65,  70,  75,  80,  85,  90,  95, 100,
		105, 110, 115, 120, 125, 130, 135, 140, 145, 150,
		155, 160, 165, 170, 175, 180, 185, 190, 195, 200,
		205, 210, 215, 220, 225, 230, 235, 240, 245, 250
	},
	.history = "no history",
	.is_dead = 0,
	.wizard = 0,
	.inventory = &test_inven[0],
};

#endif /* !UNIT_TEST_DATA */
