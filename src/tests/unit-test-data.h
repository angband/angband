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
#include "init.h"
#include "mon-lore.h"
#include "monster.h"
#include "obj-tval.h"
#include "player.h"
#include "player-calcs.h"
#include "project.h"

/* 30 = TMD_MAX */
static s16b TEST_DATA test_timed[30] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};

static struct object_base TEST_DATA sword_base = {
	.name = "Test Sword",
	.tval = TV_SWORD,
	.next = NULL,
	.break_perc = 50,
};

static struct object_base TEST_DATA light_base = {
	.name = "Test Light~",
	.tval = TV_LIGHT,
	.next = NULL,
	.break_perc = 50,
};

static struct object_base TEST_DATA flask_base = {
	.name = "Test Flask~",
	.tval = TV_FLASK,
	.next = NULL,
	.break_perc = 100,
};

static struct object_base TEST_DATA rod_base = {
	.name = "Test Rod~",
	.tval = TV_ROD,
	.next = NULL,
};

static struct artifact TEST_DATA test_artifact_sword = {
	.name = "Test Artifact",
	.text = "A test artifact.",
	.aidx = 0,
	.next = NULL,
	.tval = TV_SWORD,
	.sval = 8, //Hack - depends on edit file order - Long Sword (NRM)
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
	.sval = 8, //Hack - depends on edit file order - Long Sword (NRM)
	.pval = {
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
	},
	.modifiers = { 
		[OBJ_MOD_STR] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_WIS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_DEX] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_CON] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_STEALTH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SEARCH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INFRA] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_TUNNEL] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SPEED] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_BLOWS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SHOTS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_MIGHT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_LIGHT] = { 0, 0, 0, 0 }, 
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
	.d_char = L'|',

	.alloc_prob = 20,
	.alloc_min = 1,
	.alloc_max = 10,
	.level = 0,

	.effect = NULL,

	.gen_mult_prob = 0,
	.flavor = NULL,
};

static struct object_kind TEST_DATA test_torch = {
	.name = "Test Torch",
	.text = "A test torch [1].",
	.base = &light_base,
	.next = NULL,
	.kidx = 2,
	.tval = TV_LIGHT,
	.sval = 1, //Hack - depends on edit file order - Wooden Torch (NRM)
	.pval = {
				.base = 5000,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
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
	.ac = 0,

	.dd = 1,
	.ds = 1,
	.weight = 22,

	.cost = 1,

	.flags = { 0, 0, 4, 0 },
	.kind_flags = { 32, 0 },

	.modifiers = { 
		[OBJ_MOD_STR] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_WIS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_DEX] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_CON] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_STEALTH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SEARCH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INFRA] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_TUNNEL] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SPEED] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_BLOWS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SHOTS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_MIGHT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_LIGHT] = { 1, 0, 0, 0 }, 
	},
	.el_info = {
		[ELEM_ACID] = { 0, 0 },
		[ELEM_ELEC] = { 0, 0 },
		[ELEM_FIRE] = { 0, 0 },
		[ELEM_COLD] = { 0, 0 },
		[ELEM_POIS] = { 0, 0 },
		[ELEM_LIGHT] = { 0, 0 },
		[ELEM_DARK] = { 0, 0 },
		[ELEM_SOUND] = { 0, 0 },
		[ELEM_SHARD] = { 0, 0 },
		[ELEM_NEXUS] = { 0, 0 },
		[ELEM_NETHER] = { 0, 0 },
		[ELEM_CHAOS] = { 0, 0 },
		[ELEM_DISEN] = { 0, 0 },
		[ELEM_WATER] = { 0, 0 },
		[ELEM_ICE] = { 0, 0 },
		[ELEM_GRAVITY] = { 0, 0 },
		[ELEM_INERTIA] = { 0, 0 },
		[ELEM_FORCE] = { 0, 0 },
		[ELEM_TIME] = { 0, 0 },
		[ELEM_PLASMA] = { 0, 0 },
		[ELEM_METEOR] = { 0, 0 },
		[ELEM_MISSILE] = { 0, 0 },
		[ELEM_MANA] = { 0, 0 },
		[ELEM_HOLY_ORB] = { 0, 0 },
		[ELEM_ARROW] = { 0, 0 },
	},

	.brands = NULL,
	.slays = NULL,

	.d_attr = 7,
	.d_char = L'~',

	.alloc_prob = 70,
	.alloc_min = 1,
	.alloc_max = 40,
	.level = 1,

	.effect = NULL,
	.power = 0,
	.effect_msg = NULL,
	.time = {
		.base = 0,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
	},
	.charge = {
		.base = 0,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
	},

	.gen_mult_prob = 0,
	.stack_size = {
		.base = 0,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
	},
	.flavor = NULL,
};

static struct object_kind TEST_DATA test_lantern = {
	.name = "Test Lantern",
	.text = "A test lantern.",
	.base = &light_base,
	.next = NULL,
 	.kidx = 3,
	.tval = TV_LIGHT,
	.sval = 2, //Hack - depends on edit file order -  Lantern (NRM)
	.pval = {
		.base = 5000,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
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
	.ac = 0,

	.dd = 1,
	.ds = 1,
	.weight = 50,

	.cost = 1,

	.flags = { 0, 0, 8, 0 }, /* OF_TAKES_FUEL */
	.kind_flags = { 32, 0 },

	.modifiers = { 
		[OBJ_MOD_STR] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_WIS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_DEX] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_CON] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_STEALTH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SEARCH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INFRA] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_TUNNEL] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SPEED] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_BLOWS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SHOTS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_MIGHT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_LIGHT] = { 2, 0, 0, 0 }, 
	},
	.el_info = {
		[ELEM_ACID] = { 0, 0 },
		[ELEM_ELEC] = { 0, 0 },
		[ELEM_FIRE] = { 0, 4 },
		[ELEM_COLD] = { 0, 0 },
		[ELEM_POIS] = { 0, 0 },
		[ELEM_LIGHT] = { 0, 0 },
		[ELEM_DARK] = { 0, 0 },
		[ELEM_SOUND] = { 0, 0 },
		[ELEM_SHARD] = { 0, 0 },
		[ELEM_NEXUS] = { 0, 0 },
		[ELEM_NETHER] = { 0, 0 },
		[ELEM_CHAOS] = { 0, 0 },
		[ELEM_DISEN] = { 0, 0 },
		[ELEM_WATER] = { 0, 0 },
		[ELEM_ICE] = { 0, 0 },
		[ELEM_GRAVITY] = { 0, 0 },
		[ELEM_INERTIA] = { 0, 0 },
		[ELEM_FORCE] = { 0, 0 },
		[ELEM_TIME] = { 0, 0 },
		[ELEM_PLASMA] = { 0, 0 },
		[ELEM_METEOR] = { 0, 0 },
		[ELEM_MISSILE] = { 0, 0 },
		[ELEM_MANA] = { 0, 0 },
		[ELEM_HOLY_ORB] = { 0, 0 },
		[ELEM_ARROW] = { 0, 0 },
	},

	.brands = NULL,
	.slays = NULL,

	.d_attr = 0,
	.d_char = L'~',

	.alloc_prob = 10,
	.alloc_min = 1,
	.alloc_max = 10,
	.level = 0,

	.effect = NULL,
	.power = 0,
	.effect_msg = NULL,
	.time = {
		.base = 0,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
	},
	.charge = {
		.base = 0,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
	},

	.gen_mult_prob = 0,
	.stack_size = {
		.base = 0,
		.dice = 0,
		.sides = 0,
		.m_bonus = 0,
	},
	.flavor = NULL,
};

static struct object_kind TEST_DATA test_flask = {
	.name = "Test Flask",
	.text = "A test flask.",
	.base = &flask_base,
	.kidx = 1,
	.tval = TV_FLASK,
	.sval = 0,
	.pval = {
				.base = 7500,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
	},

	.modifiers = { 
		[OBJ_MOD_STR] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_WIS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_DEX] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_CON] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_STEALTH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SEARCH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INFRA] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_TUNNEL] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SPEED] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_BLOWS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SHOTS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_MIGHT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_LIGHT] = { 0, 0, 0, 0 }, 
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
	.ds = 4,
	.weight = 20,

	.cost = 3,

	.d_attr = 11,
	.d_char = L'!',

	.alloc_prob = 50,
	.alloc_min = 1,
	.alloc_max = 100,
	.level = 1,

	.effect = NULL,

	.gen_mult_prob = 0,
	.flavor = NULL,
};

static struct object_kind TEST_DATA test_rod_treasure_location = {
	.name = "Test Rod of Treasure Location",
	.text = "A test rod of treasure location.",
	.base = &rod_base,
	.kidx = 1,
	.tval = TV_ROD,
	.sval = 1,
	.pval = {
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
	},

	.modifiers = { 
		[OBJ_MOD_STR] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_WIS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_DEX] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_CON] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_STEALTH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SEARCH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INFRA] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_TUNNEL] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SPEED] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_BLOWS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SHOTS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_MIGHT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_LIGHT] = { 0, 0, 0, 0 }, 
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

	.dd = 0,
	.ds = 0,
	.weight = 15,

	.cost = 1000,

	.d_attr = 0,
	.d_char = L'-',

	.alloc_prob = 30,
	.alloc_min = 8,
	.alloc_max = 75,
	.level = 5,

	.effect = NULL,

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
				.base = 0,
				.dice = 0,
				.sides = 0,
				.m_bonus = 0,
	},

	.modifiers = { 
		[OBJ_MOD_STR] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_WIS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_DEX] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_CON] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_STEALTH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SEARCH] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_INFRA] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_TUNNEL] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SPEED] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_BLOWS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_SHOTS] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_MIGHT] = { 0, 0, 0, 0 }, 
		[OBJ_MOD_LIGHT] = { 0, 0, 0, 0 }, 
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
	.d_char = L'$',

	.alloc_prob = 0,
	.alloc_min = 0,
	.alloc_max = 0,
	.level = 0,

	.effect = NULL,

	.gen_mult_prob = 0,
	.flavor = NULL,
};

static struct player_race TEST_DATA test_race = {
	.name = "TestRace",
	.r_adj = {
		[STAT_STR] = +2,
		[STAT_DEX] = +1,
		[STAT_CON] = +3,
		[STAT_INT] = -1,
		[STAT_WIS] = -2,
	},
	.r_skills = {
		[SKILL_DISARM_PHYS] = 0,
		[SKILL_DISARM_MAGIC] = 0,
		[SKILL_DEVICE] = 5,
		[SKILL_SAVE] = 10,
		[SKILL_STEALTH] = -5,
		[SKILL_SEARCH] = -10,
		[SKILL_TO_HIT_MELEE] = 0,
		[SKILL_TO_HIT_BOW] = 0,
		[SKILL_TO_HIT_THROW] = 0,
		[SKILL_DIGGING] = 0,
	},

	.r_mhp = 10,
	.r_exp = 110,

	.b_age = 14,
	.m_age = 6,

	.base_hgt = 72,
	.mod_hgt = 6,
	.base_wgt = 150,
	.mod_wgt = 20,

	.infra = 40,

	.history = NULL,
};

static struct start_item TEST_DATA start_torch = {
	.tval = TV_LIGHT,
	.sval = 1, //Hack - depends on edit file order - Wooden Torch (NRM)
	.min = 3,
	.max = 5,
	.next = NULL,
};

static struct start_item TEST_DATA start_longsword = {
	.tval = TV_SWORD,
	.sval = 8, //Hack - depends on edit file order - Long Sword (NRM)
	.min = 1,
	.max = 1,
	.next = &start_torch,
};

static struct magic_realm TEST_DATA test_realm = {
	.next = NULL,
	.name = "realm",
	.stat = 1,
	.verb = "spell_verb",
	.spell_noun = "spell_noun",
	.book_noun = "book_noun",
};

static struct class_book TEST_DATA test_book = {
	.tval = 10,
	.sval = 4,
	.realm = &test_realm,
	.num_spells = 8,
	.spells = NULL,
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
		[STAT_STR] = +1,
		[STAT_DEX] = +2,
		[STAT_CON] = -1,
		[STAT_INT] = -2,
		[STAT_WIS] = +3,
	},

	.c_skills = {
		[SKILL_DISARM_PHYS] = 25,
		[SKILL_DISARM_MAGIC] = 25,
		[SKILL_DEVICE] = 18,
		[SKILL_SAVE] = 18,
		[SKILL_STEALTH] = 1,
		[SKILL_SEARCH] = 14,
		[SKILL_TO_HIT_MELEE] = 70,
		[SKILL_TO_HIT_BOW] = 55,
		[SKILL_TO_HIT_THROW] = 55,
		[SKILL_DIGGING] = 0,
	},

	.x_skills = {
		[SKILL_DISARM_PHYS] = 10,
		[SKILL_DISARM_MAGIC] = 10,
		[SKILL_DEVICE] = 7,
		[SKILL_SAVE] = 10,
		[SKILL_STEALTH] = 0,
		[SKILL_SEARCH] = 0,
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

	.start_items = &start_longsword,
	.magic =  {
		.spell_first = 1,
		.spell_weight = 300,
		.num_books = 1,
		.books = &test_book,
		.total_spells = 8,
	},
};

static struct monster_base TEST_DATA test_rb_info = {
	.next = NULL,
	.name = "townsfolk",
	.text = "Townsfolk",
	.flags = "\0\0\0\0\0\0\0\0\0\0",
	.spell_flags = "\0\0\0\0\0\0\0\0\0\0\0",
	.d_char = 116,
	.pain = NULL,
	
};

static struct blow_method TEST_DATA test_blow_method = {
	.name = "HIT",
	.cut = true,
	.stun = true,
	.miss = false,
	.phys = false,
	.msgt = 34,
	.act_msg = "hits you",
	.desc = "hit",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_hurt = {
	.name = "HURT",
	.power = 40,
	.eval = 0,
	.desc = "attack",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_poison = {
	.name = "POISON",
	.power = 20,
	.eval = 10,
	.desc = "poison",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_acid = {
	.name = "ACID",
	.power = 20,
	.eval = 20,
	.desc = "shoot acid",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_elec = {
	.name = "ELEC",
	.power = 40,
	.eval = 10,
	.desc = "electrify",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_fire = {
	.name = "FIRE",
	.power = 40,
	.eval = 10,
	.desc = "burn",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_cold = {
	.name = "COLD",
	.power = 40,
	.eval = 10,
	.desc = "freeze",
	.next = NULL
};

static struct blow_effect TEST_DATA test_blow_effect_blind = {
	.name = "BLIND",
	.power = 0,
	.eval = 20,
	.desc = "blind",
	.next = NULL
};

static struct monster_blow TEST_DATA test_blow[4] = {
	{
		.method = &test_blow_method,
		.effect = &test_blow_effect_hurt,
		.dice = {
			.base = 0,
			.dice = 3,
			.sides = 1,
			.m_bonus = 0,
		},
		.times_seen = 1,
	},
	{
		.method = NULL,
		.effect = NULL,
		.dice = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
		},
		.times_seen = 0,
	},
	{
		.method = NULL,
		.effect = NULL,
		.dice = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
		},
		.times_seen = 0,
	},
	{
		.method = NULL,
		.effect = NULL,
		.dice = {
			.base = 0,
			.dice = 0,
			.sides = 0,
			.m_bonus = 0,
		},
		.times_seen = 0,
	}
};

static bool TEST_DATA test_blows_known[4] = {
	true,
	false,
	false,
	false,
};

static struct monster_race TEST_DATA test_r_human = {
	.next = NULL,
	.ridx = 0,
	.name = "Human",
	.text = "A random test human",

	.base = &test_rb_info,

	.avg_hp = 10,
	.ac = 12,
	.sleep = 0,
	.hearing = 20,
	.smell = 20,
	.speed = 110,
	.mexp = 50,
	.freq_innate = 0,
	.freq_spell = 0,

	.blow = &test_blow[0],

	.level = 1,
	.rarity = 1,

	.d_attr = 0,
	.d_char = 't',

	.max_num = 100,
	.cur_num = 0,

	.drops = NULL,
};

static monster_lore TEST_DATA test_lore = {
	.ridx = 0,
	.sights = 1,
	.deaths = 0,
	.pkills = 0,
	.tkills = 5,
	.wake = 1,
	.ignore = 4,
	.drop_gold = 0,
	.drop_item = 0,
	.cast_innate = 0,
	.cast_spell = 0,

	.blows = &test_blow[0],

	.flags = "\0\0\0\0\0\0\0\0\0\0",
	.spell_flags = "\0\0\0\0\0\0\0\0\0\0\0",
	.drops = NULL,
	.friends = NULL,
	.friends_base = NULL,
	.mimic_kinds = NULL,
	.all_known = false,
	.blow_known = &test_blows_known[0],
	.armour_known = false,
	.drop_known = false,
	.sleep_known = false,
	.spell_freq_known = false
};

static struct angband_constants TEST_DATA test_z_info = {
	.f_max    = 2,
	.trap_max = 2,
	.k_max    = 2,
	.a_max    = 2,
	.e_max    = 2,
	.r_max    = 2,
	.mp_max   = 2,
	.s_max    = 2,
	.pit_max  = 2,
	.act_max  = 2,
	.level_monster_max = 2,
};

static struct equip_slot TEST_DATA test_slot_light = {
	.type = 5,
	.name = "light",
	.obj = NULL,
};

static struct quest TEST_DATA test_quest = {
	.next = NULL,
	.index = 0,
	.name = "Test",
	.level = 1,
	.race = &test_r_human,
	.cur_num = 0,
	.max_num = 4,
};

static struct player_body TEST_DATA test_player_body = {
	.next    = NULL,
	.name    = "Humanoid",
	.count   = 12,
};

static struct player_upkeep TEST_DATA test_player_upkeep = {
	.playing = 1,
	.autosave = 0,
	.generate_level = 0,
	.energy_use = 0,
	.new_spells = 0,

	.health_who = NULL,
	.monster_race = NULL,
	.object = NULL,
	.object_kind = NULL,

	.notice = 0,
	.update = 0,
	.redraw = 0,

	.command_wrk = 0,

	.create_up_stair = 0,
	.create_down_stair = 0,

	.running = 0,
	.running_withpathfind = 0,
	.running_firststep = 0,

	.quiver = NULL,
	.inven = NULL,

	.total_weight = 0,
	.inven_cnt = 0,
	.equip_cnt = 0,
	.quiver_cnt = 0,
};

static struct object TEST_DATA test_player_knowledge = {
	.kind = NULL,
	.ego = NULL,
	.artifact = NULL,
	.prev = NULL,
	.next = NULL,
	.known = NULL,
	.oidx = 0,
	.iy = 0,
	.ix = 0,
	.tval = 0,
	.sval = 0,
	.pval = 0,
	.weight = 0,

	.modifiers = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	.el_info = {
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 }
	},
	.brands = NULL,
	.slays = NULL,

	.ac = 0,
	.to_h = 0,
	.to_d = 0,
	.to_a = 0,

	.dd = 0,
	.ds = 0,

	.effect = NULL,
	.effect_msg = NULL,
	.activation = NULL,
	.time = { 0, 0, 0, 0, },
	.timeout = 0,

	.number = 0,
	.notice = 0,

	.held_m_idx = 0,
	.mimicking_m_idx = 0,
	.origin = 0,
	.origin_depth = 0,
	.origin_race = NULL,
	.note = 0,
};


static struct player TEST_DATA test_player = {
	.py = 1,
	.px = 1,
	.race = &test_race,
	.class = &test_class,
	.hitdie = 10,
	.expfact = 100,
	.age = 12,
	.ht = 40,
	.wt = 80,
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
		[STAT_STR] = 14,
		[STAT_DEX] = 12,
		[STAT_CON] = 14,
		[STAT_WIS] = 10,
		[STAT_INT] = 8,
	},
	.stat_cur = {
		[STAT_STR] = 14,
		[STAT_DEX] = 11,
		[STAT_CON] = 14,
		[STAT_WIS] = 10,
		[STAT_INT] = 8,
	},
	.timed = test_timed,
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
	.upkeep = &test_player_upkeep,
	.gear = NULL,
	.gear_k = NULL,
	.obj_k = &test_player_knowledge,
};

static struct chunk TEST_DATA test_cave = {
	.name = "Test",
	.turn = 1,
	.depth = 1,

	.feeling = 0,
	.obj_rating = 0,
	.mon_rating = 0,
	.good_item = false,

	.height = 2,
	.width = 2,

	.feeling_squares = 0,
	.feat_count = NULL,

	.squares = NULL,

	.monsters = NULL,
	.mon_max = 1,
	.mon_cnt = 0,
	.mon_current = -1,
};

static struct projection TEST_DATA test_projections[4] = {
	{
		.index = 0,
		.name = "acid",
		.type = "element",
		.desc = "acid",
		.player_desc = "acid",
		.blind_desc = "acid",
		.numerator = 1,
		.denominator = {3, 0, 0, 0},
		.divisor = 3,
		.damage_cap = 1600,
		.msgt = 0,
		.obvious = true,
		.color = 2,
		.next = NULL
	},
	{
		.index = 1,
		.name = "electricity",
		.type = "element",
		.desc = "electricity",
		.player_desc = "electricity",
		.blind_desc = "electricity",
		.numerator = 1,
		.denominator = {3, 0, 0, 0},
		.divisor = 3,
		.damage_cap = 1600,
		.msgt = 0,
		.obvious = true,
		.color = 6,
		.next = NULL
	},
	{
		.index = 2,
		.name = "fire",
		.type = "element",
		.desc = "fire",
		.player_desc = "fire",
		.blind_desc = "fire",
		.numerator = 1,
		.denominator = {3, 0, 0, 0},
		.divisor = 3,
		.damage_cap = 1600,
		.msgt = 0,
		.obvious = true,
		.color = 4,
		.next = NULL
	},
	{
		.index = 3,
		.name = "cold",
		.type = "element",
		.desc = "cold",
		.player_desc = "cold",
		.blind_desc = "cold",
		.numerator = 1,
		.denominator = {3, 0, 0, 0},
		.divisor = 3,
		.damage_cap = 1600,
		.msgt = 0,
		.obvious = true,
		.color = 1,
		.next = NULL
	}
};

#endif /* !UNIT_TEST_DATA */
