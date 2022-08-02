/* monster/desc.c */
/* Exercise monster_desc(), get_mon_name(), and plural_aux(). */

#include "unit-test.h"
#include "game-input.h"
#include "mon-desc.h"
#include "z-virt.h"


struct my_test_data {
	struct monster mon;
	struct monster_race race;
};

#define MY_SZ (40)
#define MY_N_MONSTER (8)
struct my_monster {
	char name[32]; bool unique, male, female, comma;
};
struct my_desc_exp {
	int mode;
	size_t sz;
	const char *results[MY_N_MONSTER];
};


static struct my_monster g_monsters[MY_N_MONSTER] = {
	{ "Gilgamesh", true, true, false, false },
	{ "Inanna", true, false, true, false },
	{ "Watcher in the Water", true, false, false, false },
	{ "Wormtongue, Agent of Saruman", true, true, false, true },
	{ "satyr", false, true, false, false },
	{ "nymph", false, false, true, false },
	{ "alligator", false, false, false, false },
	{ "dog, man's best friend", false, false, false, true }
};


int setup_tests(void **state) {
	struct my_test_data *d = mem_zalloc(sizeof(*d));

	d->mon.race = &d->race;
	*state = d;
	return 0;
}


int teardown_tests(void *state) {
	mem_free(state);
	return 0;
}


static void fill_fluff(char *buf, const char *lead, size_t sz) {
	const char fluff[4] = { '\xde', '\xad', '\xbe', '\xef' };
	size_t i = 0;
	size_t ind;

	if (lead) {
		while (*lead && i < sz) {
			buf[i] = *lead;
			++lead;
			++i;
		}
		if (i < sz) {
			buf[i] = '\0';
			++i;
		}
	}

	for (ind = i % sizeof(fluff); i < sz; ++i) {
		buf[i] = fluff[ind];
		++ind;
		if (ind == sizeof(fluff)) {
			ind = 0;
		}
	}
}


static bool check_fluff(const char *buf, const char *lead, size_t sz) {
	const char fluff[4] = { '\xde', '\xad', '\xbe', '\xef' };
	size_t i = 0;
	size_t ind;

	if (lead) {
		while (*lead && i < sz - 1) {
			if (buf[i] != *lead) {
				return false;
			}
			++lead;
			++i;
		}
		if (i < sz) {
			if (buf[i] != '\0') {
				return false;
			}
			++i;
		}
	}
	ind = i % sizeof(fluff);
	while (1) {
		if (i >= sz) {
			return true;
		}
		if (buf[i] != fluff[ind]) {
			return false;
		}
		++i;
		++ind;
		if (ind == sizeof(fluff)) {
			ind = 0;
		}
	}
}


static bool fake_panel_always_contains(unsigned int y, unsigned int x) {
	return true;
}


static bool fake_panel_never_contains(unsigned int y, unsigned int x) {
	return false;
}

static int test_plural_aux_0(void *state) {
	char buf[10];

	fill_fluff(buf, "crow", sizeof(buf));
	plural_aux(buf, sizeof(buf));
	require(check_fluff(buf, "crows", sizeof(buf)));
	fill_fluff(buf, "crow", sizeof(buf));
	plural_aux(buf, 5);
	require(check_fluff(buf, "crow", sizeof(buf)));
	fill_fluff(buf, "ibis", sizeof(buf));
	plural_aux(buf, sizeof(buf));
	require(check_fluff(buf, "ibises", sizeof(buf)));
	fill_fluff(buf, "ibis", sizeof(buf));
	plural_aux(buf, 6);
	require(check_fluff(buf, "ibise", sizeof(buf)));
	ok;
}


static int test_get_mon_name_nonunique_0(void *state) {
	char name1[] = "crow";
	char name2[] = "ibis";
	char name3[] = "dog of war";
	char plural3[] = "dogs of war";
	struct my_test_data *d = (struct my_test_data*) state;
	struct monster_race *dummy_race = &d->race;
	char buf[30];

	dummy_race->name = name1;
	dummy_race->plural = NULL;
	rf_wipe(dummy_race->flags);
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 1);
	require(check_fluff(buf, "  1 crow", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 5);
	require(check_fluff(buf, "  5 crows", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, 7, dummy_race, 5);
	require(check_fluff(buf, "  5 cr", sizeof(buf)));
	dummy_race->name = name2;
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 1);
	require(check_fluff(buf, "  1 ibis", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 8);
	require(check_fluff(buf, "  8 ibises", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, 3, dummy_race, 8);
	require(check_fluff(buf, "  ", sizeof(buf)));
	dummy_race->name = name3;
	dummy_race->plural = plural3;
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 1);
	require(check_fluff(buf, "  1 dog of war", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 2);
	require(check_fluff(buf, "  2 dogs of war", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, 10, dummy_race, 11);
	require(check_fluff(buf, " 11 dogs ", sizeof(buf)));
	ok;
}


static int test_get_mon_name_unique_0(void *state) {
	char name[] = "Gilgamesh";
	struct my_test_data *d = (struct my_test_data*) state;
	struct monster_race *dummy_race = &d->race;
	char buf[30];

	dummy_race->name = name;
	dummy_race->plural = NULL;
	rf_wipe(dummy_race->flags);
	rf_on(dummy_race->flags, RF_UNIQUE);
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 1);
	require(check_fluff(buf, "[U] Gilgamesh", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, sizeof(buf), dummy_race, 3);
	require(check_fluff(buf, "[U] Gilgamesh", sizeof(buf)));
	fill_fluff(buf, NULL, sizeof(buf));
	get_mon_name(buf, 8, dummy_race, 1);
	require(check_fluff(buf, "[U] Gil", sizeof(buf)));
	ok;
}


static int test_monster_desc_hidden_def_0(void *state) {
	struct my_desc_exp expected0[] = {
		{ 0, MY_SZ, { "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ 0, 2, { "i", "i", "i", "i", "i", "i", "i", "i" } },
		{ MDESC_OBJE, MY_SZ,
			{ "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ MDESC_OBJE, 3,
			{ "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ MDESC_POSS, MY_SZ,
			{ "its", "its", "its", "its", "its", "its", "its",
			"its" } },
		{ MDESC_POSS, 3,
			{ "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ MDESC_OBJE | MDESC_POSS, MY_SZ,
			{ "itself", "itself", "itself", "itself", "itself",
			"itself", "itself", "itself" } },
		{ MDESC_OBJE | MDESC_POSS, 5,
			{ "itse", "itse", "itse", "itse", "itse", "itse",
			"itse", "itse" } },
		{ MDESC_COMMA, MY_SZ,
			{ "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ MDESC_COMMA, 2, { "i", "i", "i", "i", "i", "i", "i", "i" } },
		{ MDESC_PRO_HID, MY_SZ,
			{ "he", "she", "it", "he", "he", "she", "it", "it" } },
		{ MDESC_PRO_HID, 3,
			{ "he", "sh", "it", "he", "he", "sh", "it", "it" } },
		{ MDESC_PRO_HID | MDESC_OBJE, MY_SZ,
			{ "him", "her", "it", "him", "him", "her", "it",
			"it" } },
		{ MDESC_PRO_HID | MDESC_OBJE, 2,
			{ "h", "h", "i", "h", "h", "h", "i", "i" } },
		{ MDESC_PRO_HID | MDESC_POSS, MY_SZ,
			{ "his", "her", "its", "his", "his", "her", "its",
			"its" } },
		{ MDESC_PRO_HID | MDESC_POSS, 3,
			{ "hi", "he", "it", "hi", "hi", "he", "it", "it" } },
		{ MDESC_PRO_HID | MDESC_OBJE | MDESC_POSS, MY_SZ,
			{ "himself", "herself", "itself", "himself", "himself",
			"herself", "itself", "itself" } },
		{ MDESC_PRO_HID | MDESC_OBJE | MDESC_POSS, 6,
			{ "himse", "herse", "itsel", "himse", "himse", "herse",
			"itsel", "itsel" } },
		{ MDESC_PRO_HID | MDESC_COMMA, MY_SZ,
			{ "he", "she", "it", "he", "he", "she", "it", "it" } },
		{ MDESC_PRO_HID | MDESC_COMMA, 3,
			{ "he", "sh", "it", "he", "he", "sh", "it", "it" } },
	};
	struct my_desc_exp expected1[] = {
		{ MDESC_HIDE, MY_SZ,
			{ "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ MDESC_HIDE, 2, { "i", "i", "i", "i", "i", "i", "i", "i" } },
		{ MDESC_HIDE | MDESC_PRO_HID, MY_SZ,
			{ "he", "she", "it", "he", "he", "she", "it", "it" } },
		{ MDESC_HIDE | MDESC_PRO_HID, 2,
			{ "h", "s", "i", "h", "h", "s", "i", "i" } },
	};
	struct my_test_data *d = (struct my_test_data*) state;
	struct monster *dummy_mon = &d->mon;
	char buf[MY_SZ];
	int i, j;

	for (i = 0; i < MY_N_MONSTER; ++i) {
		dummy_mon->race->name = g_monsters[i].name;
		dummy_mon->race->plural = NULL;
		rf_wipe(dummy_mon->race->flags);
		if (g_monsters[i].unique) {
			rf_on(dummy_mon->race->flags, RF_UNIQUE);
		}
		if (g_monsters[i].male) {
			rf_on(dummy_mon->race->flags, RF_MALE);
		}
		if (g_monsters[i].female) {
			rf_on(dummy_mon->race->flags, RF_FEMALE);
		}
		if (g_monsters[i].comma) {
			rf_on(dummy_mon->race->flags, RF_NAME_COMMA);
		}
		mflag_wipe(dummy_mon->mflag);
		panel_contains_hook = fake_panel_always_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected0); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected0[j].sz <= sizeof(buf));
			monster_desc(buf, expected0[j].sz, dummy_mon,
				expected0[j].mode);
			require(check_fluff(buf, expected0[j].results[i],
				sizeof(buf)));
		}

		mflag_on(dummy_mon->mflag, MFLAG_VISIBLE);
		panel_contains_hook = fake_panel_never_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected1); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected1[j].sz <= sizeof(buf));
			monster_desc(buf, expected1[j].sz, dummy_mon,
				expected1[j].mode);
			require(check_fluff(buf, expected1[j].results[i],
				sizeof(buf)));
		}
	}

	ok;
}


static int test_monster_desc_hidden_indef_0(void *state) {
	struct my_desc_exp expected0[] = {
		{ MDESC_IND_HID, MY_SZ,
			{ "something", "something", "something", "something",
			"something", "something", "something", "something" } },
		{ MDESC_IND_HID, 6,
			{ "somet", "somet", "somet", "somet", "somet", "somet",
			"somet", "somet" } },
		{ MDESC_IND_HID | MDESC_OBJE, MY_SZ,
			{ "something", "something", "something", "something",
			"something", "something", "something", "something" } },
		{ MDESC_IND_HID | MDESC_OBJE, 4,
			{ "som", "som", "som", "som", "som", "som", "som",
			"som" } },
		{ MDESC_IND_HID | MDESC_POSS, MY_SZ,
			{ "something's", "something's", "something's",
			"something's", "something's", "something's",
			"something's", "something's" } },
		{ MDESC_IND_HID | MDESC_POSS, 7,
			{ "someth", "someth", "someth", "someth", "someth",
			"someth", "someth", "someth" } },
		{ MDESC_IND_HID | MDESC_OBJE | MDESC_POSS, MY_SZ,
			{ "itself", "itself", "itself", "itself", "itself",
			"itself", "itself", "itself" } },
		{ MDESC_IND_HID | MDESC_OBJE | MDESC_POSS, 3,
			{ "it", "it", "it", "it", "it", "it", "it", "it" } },
		{ MDESC_IND_HID | MDESC_COMMA, MY_SZ,
			{ "something", "something", "something", "something",
			"something", "something", "something", "something" } },
		{ MDESC_IND_HID | MDESC_COMMA, 8,
			{ "somethi", "somethi", "somethi", "somethi", "somethi",
			"somethi", "somethi", "somethi" } },
		{ MDESC_IND_HID | MDESC_OBJE | MDESC_COMMA, MY_SZ,
			{ "something", "something", "something", "something",
			"something", "something", "something", "something" } },
		{ MDESC_IND_HID | MDESC_OBJE | MDESC_COMMA, 6,
			{ "somet", "somet", "somet", "somet", "somet", "somet",
			"somet", "somet" } },
		{ MDESC_IND_HID | MDESC_PRO_HID, MY_SZ,
			{ "someone", "someone", "something", "someone",
			"someone", "someone", "something", "something" } },
		{ MDESC_IND_HID | MDESC_PRO_HID, 7,
			{ "someon", "someon", "someth", "someon", "someon",
			"someon", "someth", "someth" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_OBJE, MY_SZ,
			{ "someone", "someone", "something", "someone",
			"someone", "someone", "something", "something" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_OBJE, 4,
			{ "som", "som", "som", "som", "som", "som", "som",
			"som" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_POSS, MY_SZ,
			{ "someone's", "someone's", "something's", "someone's",
			"someone's", "someone's", "something's",
			"something's" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_POSS, 9,
			{ "someone'", "someone'", "somethin", "someone'",
			"someone'", "someone'", "somethin", "somethin" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_OBJE | MDESC_POSS, MY_SZ,
			{ "himself", "herself", "itself", "himself", "himself",
			"herself", "itself", "itself" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_OBJE | MDESC_POSS, 5,
			{ "hims", "hers", "itse", "hims", "hims", "hers",
			"itse", "itse" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_COMMA, MY_SZ,
			{ "someone", "someone", "something", "someone",
			"someone", "someone", "something", "something" } },
		{ MDESC_IND_HID | MDESC_PRO_HID | MDESC_COMMA, 6,
			{ "someo", "someo", "somet", "someo", "someo", "someo",
			"somet", "somet" } },
		{ MDESC_STANDARD, MY_SZ,
			{ "Someone", "Someone", "Something", "Someone",
			"Someone", "Someone", "Something", "Something" } },
		{ MDESC_STANDARD, 4,
			{ "Som", "Som", "Som", "Som", "Som", "Som",  "Som",
			"Som" } },
		{ MDESC_TARG, MY_SZ,
			{ "someone", "someone", "something", "someone",
			"someone", "someone", "something", "something" } },
		{ MDESC_TARG, 6,
			{ "someo", "someo", "somet", "someo", "someo", "someo",
			"somet", "somet" } },
	};
	struct my_desc_exp expected1[] = {
		{ MDESC_HIDE | MDESC_IND_HID, MY_SZ,
			{ "something", "something", "something", "something",
			"something", "something", "something", "something" } },
		{ MDESC_HIDE | MDESC_IND_HID, 7,
			{ "someth", "someth", "someth", "someth", "someth",
			"someth", "someth", "someth" } },
		{ MDESC_HIDE | MDESC_IND_HID | MDESC_PRO_HID, MY_SZ,
			{ "someone", "someone", "something", "someone",
			"someone", "someone", "something", "something" } },
		{ MDESC_HIDE | MDESC_IND_HID | MDESC_PRO_HID, 8,
			{ "someone", "someone", "somethi", "someone",
			"someone", "someone", "somethi", "somethi" } },
	};
	struct my_test_data *d = (struct my_test_data*) state;
	struct monster *dummy_mon = &d->mon;
	char buf[MY_SZ];
	int i, j;

	for (i = 0; i < MY_N_MONSTER; ++i) {
		dummy_mon->race->name = g_monsters[i].name;
		dummy_mon->race->plural = NULL;
		rf_wipe(dummy_mon->race->flags);
		if (g_monsters[i].unique) {
			rf_on(dummy_mon->race->flags, RF_UNIQUE);
		}
		if (g_monsters[i].male) {
			rf_on(dummy_mon->race->flags, RF_MALE);
		}
		if (g_monsters[i].female) {
			rf_on(dummy_mon->race->flags, RF_FEMALE);
		}
		if (g_monsters[i].comma) {
			rf_on(dummy_mon->race->flags, RF_NAME_COMMA);
		}
		mflag_wipe(dummy_mon->mflag);
		panel_contains_hook = fake_panel_always_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected0); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected0[j].sz <= sizeof(buf));
			monster_desc(buf, expected0[j].sz, dummy_mon,
				expected0[j].mode);
		}

		mflag_on(dummy_mon->mflag, MFLAG_VISIBLE);
		panel_contains_hook = fake_panel_never_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected1); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected1[j].sz <= sizeof(buf));
			monster_desc(buf, expected1[j].sz, dummy_mon,
				expected1[j].mode);
		}
	}

	ok;
}


static int test_monster_desc_seen_def_0(void *state) {
	struct my_desc_exp expected0[] = {
		{ 0, MY_SZ, { "Gilgamesh", "Inanna", "Watcher in the Water",
			"Wormtongue, Agent of Saruman", "the satyr",
			"the nymph", "the alligator",
			"the dog, man's best friend" } },
		{ 0, 2, { "G", "I", "W", "W", "t", "t", "t", "t" } },
		{ MDESC_OBJE, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman",
			"the satyr", "the nymph", "the alligator",
			"the dog, man's best friend" } },
		{ MDESC_OBJE, 3, { "Gi", "In", "Wa", "Wo", "th", "th", "th",
			"th" } },
		{ MDESC_POSS, MY_SZ, { "Gilgamesh's", "Inanna's",
			"Watcher in the Water's", "Wormtongue's", "the satyr's",
			"the nymph's", "the alligator's", "the dog's" } },
		{ MDESC_POSS, 4, { "Gil", "Ina", "Wat", "Wor", "the", "the",
			"the", "the" } },
		{ MDESC_OBJE | MDESC_POSS, MY_SZ, { "himself", "herself",
			"itself", "himself", "himself", "herself", "itself",
			"itself" } },
		{ MDESC_OBJE | MDESC_POSS, 5, { "hims", "hers", "itse", "hims",
			"hims", "hers", "itse", "itse" } },
		{ MDESC_COMMA, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman,",
			"the satyr", "the nymph", "the alligator",
			"the dog, man's best friend," } },
		{ MDESC_COMMA, 2, { "G", "I", "W", "W", "t", "t", "t", "t" } },
		{ MDESC_STANDARD, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman,",
			"The satyr", "The nymph", "The alligator",
			"The dog, man's best friend," } },
		{ MDESC_STANDARD, 10, { "Gilgamesh", "Inanna",
			"Watcher i", "Wormtongu", "The satyr", "The nymph",
			"The allig", "The dog, " } },
		{ MDESC_TARG, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman",
			"the satyr", "the nymph", "the alligator",
			"the dog, man's best friend" } },
		{ MDESC_TARG, 8, { "Gilgame", "Inanna", "Watcher", "Wormton",
			"the sat", "the nym", "the all", "the dog" } },
		{ MDESC_PRO_VIS, MY_SZ, { "he", "she", "it", "he", "he", "she",
			"it", "it" } },
		{ MDESC_PRO_VIS, 3, { "he", "sh", "it", "he", "he", "sh",
			"it", "it" } },
		{ MDESC_PRO_VIS | MDESC_OBJE, MY_SZ, { "him", "her", "it",
			 "him", "him", "her", "it", "it" } },
		{ MDESC_PRO_VIS | MDESC_OBJE, 2, { "h", "h", "i", "h", "h",
			"h", "i", "i" } },
		{ MDESC_PRO_VIS | MDESC_POSS, MY_SZ, { "his", "her", "its",
			"his", "his", "her", "its", "its" } },
		{ MDESC_PRO_VIS | MDESC_POSS, 3, { "hi", "he", "it", "hi", "hi",
			"he", "it", "it" } },
		{ MDESC_PRO_VIS | MDESC_OBJE | MDESC_POSS, MY_SZ, { "himself",
			"herself", "itself", "himself", "himself", "herself",
			"itself", "itself" } },
		{ MDESC_PRO_VIS | MDESC_OBJE | MDESC_POSS, 6, { "himse",
			"herse", "itsel", "himse", "himse", "herse",
			"itsel", "itsel" } },
		{ MDESC_PRO_VIS | MDESC_COMMA, MY_SZ, { "he", "she", "it",
			"he", "he", "she", "it", "it" } },
		{ MDESC_PRO_VIS | MDESC_COMMA, 3, { "he", "sh", "it", "he",
			"he", "sh", "it", "it" } },
	};
	struct my_desc_exp expected1[] = {
		{ MDESC_SHOW, MY_SZ, { "Gilgamesh (offscreen)",
			"Inanna (offscreen)",
			"Watcher in the Water (offscreen)",
			"Wormtongue, Agent of Saruman (offscreen)",
			"the satyr (offscreen)", "the nymph (offscreen)",
			"the alligator (offscreen)",
			"the dog, man's best friend (offscreen)" } },
		{ MDESC_SHOW, 7, { "Gilgam", "Inanna", "Watche", "Wormto",
			"the sa", "the ny", "the al", "the do" } },
		{ MDESC_SHOW | MDESC_PRO_VIS, MY_SZ, { "he", "she", "it", "he",
			"he", "she", "it", "it" } },
		{ MDESC_SHOW | MDESC_PRO_VIS, 2, { "h", "s", "i", "h", "h",
			"s", "i", "i" } },
	};
	struct my_test_data *d = (struct my_test_data*) state;
	struct monster *dummy_mon = &d->mon;
	char buf[MY_SZ];
	int i, j;

	for (i = 0; i < MY_N_MONSTER; ++i) {
		dummy_mon->race->name = g_monsters[i].name;
		dummy_mon->race->plural = NULL;
		rf_wipe(dummy_mon->race->flags);
		if (g_monsters[i].unique) {
			rf_on(dummy_mon->race->flags, RF_UNIQUE);
		}
		if (g_monsters[i].male) {
			rf_on(dummy_mon->race->flags, RF_MALE);
		}
		if (g_monsters[i].female) {
			rf_on(dummy_mon->race->flags, RF_FEMALE);
		}
		if (g_monsters[i].comma) {
			rf_on(dummy_mon->race->flags, RF_NAME_COMMA);
		}
		mflag_wipe(dummy_mon->mflag);
		mflag_on(dummy_mon->mflag, MFLAG_VISIBLE);
		panel_contains_hook = fake_panel_always_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected0); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected0[j].sz <= sizeof(buf));
			monster_desc(buf, expected0[j].sz, dummy_mon,
				expected0[j].mode);
			require(check_fluff(buf, expected0[j].results[i],
				sizeof(buf)));
		}

		mflag_off(dummy_mon->mflag, MFLAG_VISIBLE);
		panel_contains_hook = fake_panel_never_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected1); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected1[j].sz <= sizeof(buf));
			monster_desc(buf, expected1[j].sz, dummy_mon,
				expected1[j].mode);
			require(check_fluff(buf, expected1[j].results[i],
				sizeof(buf)));
		}
	}

	ok;
}


static int test_monster_desc_seen_indef_0(void *state) {
	struct my_desc_exp expected0[] = {
		{ MDESC_IND_VIS, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman",
			"a satyr", "a nymph", "an alligator",
			"a dog, man's best friend" } },
		{ MDESC_IND_VIS, 6, { "Gilga", "Inann", "Watch", "Wormt",
			"a sat", "a nym", "an al", "a dog" } },
		{ MDESC_IND_VIS | MDESC_OBJE, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman",
			"a satyr", "a nymph", "an alligator",
			"a dog, man's best friend" } },
		{ MDESC_IND_VIS | MDESC_OBJE, 4, { "Gil", "Ina", "Wat", "Wor",
			"a s", "a n", "an ", "a d" } },
		{ MDESC_IND_VIS | MDESC_POSS, MY_SZ, { "Gilgamesh's",
			"Inanna's", "Watcher in the Water's", "Wormtongue's",
			"a satyr's", "a nymph's", "an alligator's",
			"a dog's" } },
		{ MDESC_IND_VIS | MDESC_POSS, 7, { "Gilgam", "Inanna",
			"Watche", "Wormto", "a saty", "a nymp", "an all",
			"a dog'" } },
		{ MDESC_IND_VIS | MDESC_OBJE | MDESC_POSS, MY_SZ, { "himself",
			"herself", "itself", "himself", "himself", "herself",
			"itself", "itself" } },
		{ MDESC_IND_VIS | MDESC_OBJE | MDESC_POSS, 3, { "hi", "he",
			"it", "hi", "hi", "he", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_COMMA, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman,",
			"a satyr", "a nymph", "an alligator",
			"a dog, man's best friend," } },
		{ MDESC_IND_VIS | MDESC_COMMA, 8, { "Gilgame", "Inanna",
			"Watcher", "Wormton", "a satyr", "a nymph",
			"an alli", "a dog, " } },
		{ MDESC_IND_VIS | MDESC_OBJE | MDESC_COMMA, MY_SZ,
			{ "Gilgamesh", "Inanna", "Watcher in the Water",
			"Wormtongue, Agent of Saruman,", "a satyr", "a nymph",
			"an alligator", "a dog, man's best friend," } },
		{ MDESC_IND_VIS | MDESC_OBJE | MDESC_COMMA, 6, { "Gilga",
			"Inann", "Watch", "Wormt", "a sat", "a nym",
			"an al", "a dog" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS, MY_SZ, { "he", "she", "it",
			"he", "he", "she", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS, 3, { "he", "sh", "it", "he",
			"he", "sh", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_OBJE, MY_SZ, { "him",
			"her", "it", "him", "him", "her", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_OBJE, 4, { "him",
			"her", "it", "him", "him", "her", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_POSS, MY_SZ, { "his",
			"her", "its", "his", "his", "her", "its", "its" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_POSS, 3, { "hi", "he",
			"it", "hi", "hi", "he", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_OBJE | MDESC_POSS,
			MY_SZ, { "himself", "herself", "itself", "himself",
			"himself", "herself", "itself", "itself" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_OBJE | MDESC_POSS, 5,
			{ "hims", "hers", "itse", "hims", "hims", "hers",
			"itse", "itse" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_COMMA, MY_SZ, { "he",
			"she", "it", "he", "he", "she", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_PRO_VIS | MDESC_COMMA, 3, { "he", "sh",
			"it", "he", "he", "sh", "it", "it" } },
		{ MDESC_IND_VIS | MDESC_STANDARD, MY_SZ, { "Gilgamesh",
			"Inanna", "Watcher in the Water",
			"Wormtongue, Agent of Saruman,", "A satyr", "A nymph",
			"An alligator", "A dog, man's best friend," } },
		{ MDESC_IND_VIS | MDESC_STANDARD, 4, { "Gil", "Ina", "Wat",
			"Wor", "A s", "A n", "An ", "A d" } },
		{ MDESC_IND_VIS | MDESC_TARG, MY_SZ, { "Gilgamesh", "Inanna",
			"Watcher in the Water", "Wormtongue, Agent of Saruman",
			"a satyr", "a nymph", "an alligator",
			"a dog, man's best friend" } },
		{ MDESC_IND_VIS | MDESC_TARG, 3, { "Gi", "In", "Wa", "Wo",
			"a ", "a ", "an", "a " } },
	};
	struct my_desc_exp expected1[] = {
		{ MDESC_SHOW | MDESC_IND_VIS, MY_SZ, { "Gilgamesh (offscreen)",
			"Inanna (offscreen)",
			"Watcher in the Water (offscreen)",
			"Wormtongue, Agent of Saruman (offscreen)",
			"a satyr (offscreen)", "a nymph (offscreen)",
			"an alligator (offscreen)",
			"a dog, man's best friend (offscreen)" } },
		{ MDESC_SHOW | MDESC_IND_VIS, 9, { "Gilgames", "Inanna (",
			"Watcher ", "Wormtong", "a satyr ", "a nymph ",
			"an allig", "a dog, m" } },
		{ MDESC_DIED_FROM, MY_SZ, { "Gilgamesh (offscreen)",
			"Inanna (offscreen)",
			"Watcher in the Water (offscreen)",
			"Wormtongue, Agent of Saruman (offscreen)",
			"a satyr (offscreen)", "a nymph (offscreen)",
			"an alligator (offscreen)",
			"a dog, man's best friend (offscreen)" } },
		{ MDESC_DIED_FROM, 11, { "Gilgamesh ", "Inanna (of",
			"Watcher in", "Wormtongue", "a satyr (o", "a nymph (o",
			"an alligat", "a dog, man" } },
		{ MDESC_SHOW | MDESC_IND_VIS | MDESC_PRO_VIS, MY_SZ, { "he",
			"she", "it", "he", "he", "she", "it", "it" } },
		{ MDESC_SHOW | MDESC_IND_VIS | MDESC_PRO_VIS, 2, { "h", "s",
			"i", "h", "h", "s", "i", "i" } },
	};
	struct my_test_data *d = (struct my_test_data*) state;
	struct monster *dummy_mon = &d->mon;
	char buf[MY_SZ];
	int i, j;

	for (i = 0; i < MY_N_MONSTER; ++i) {
		dummy_mon->race->name = g_monsters[i].name;
		dummy_mon->race->plural = NULL;
		rf_wipe(dummy_mon->race->flags);
		if (g_monsters[i].unique) {
			rf_on(dummy_mon->race->flags, RF_UNIQUE);
		}
		if (g_monsters[i].male) {
			rf_on(dummy_mon->race->flags, RF_MALE);
		}
		if (g_monsters[i].female) {
			rf_on(dummy_mon->race->flags, RF_FEMALE);
		}
		if (g_monsters[i].comma) {
			rf_on(dummy_mon->race->flags, RF_NAME_COMMA);
		}
		mflag_wipe(dummy_mon->mflag);
		mflag_on(dummy_mon->mflag, MFLAG_VISIBLE);
		panel_contains_hook = fake_panel_always_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected0); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected0[j].sz <= sizeof(buf));
			monster_desc(buf, expected0[j].sz, dummy_mon,
				expected0[j].mode);
			require(check_fluff(buf, expected0[j].results[i],
				sizeof(buf)));
		}

		mflag_off(dummy_mon->mflag, MFLAG_VISIBLE);
		panel_contains_hook = fake_panel_never_contains;

		for (j = 0; j < (int)N_ELEMENTS(expected1); ++j) {
			fill_fluff(buf, NULL, sizeof(buf));
			require(expected1[j].sz <= sizeof(buf));
			monster_desc(buf, expected1[j].sz, dummy_mon,
				expected1[j].mode);
			require(check_fluff(buf, expected1[j].results[i],
				sizeof(buf)));
		}
	}

	ok;
}


const char *suite_name = "monster/desc";
struct test tests[] = {
	{ "plural_aux 0", test_plural_aux_0 },
	{ "get_mon_name nonunique_0", test_get_mon_name_nonunique_0 },
	{ "get_mon_name unique_0", test_get_mon_name_unique_0 },
	{ "monster_desc hidden_def_0", test_monster_desc_hidden_def_0 },
	{ "monster_desc hidden_indef_0", test_monster_desc_hidden_indef_0 },
	{ "monster_desc seen_def_0", test_monster_desc_seen_def_0 },
	{ "monster_desc seen_indef_0", test_monster_desc_seen_indef_0 },
	{ NULL, NULL }
};
