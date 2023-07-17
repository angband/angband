/* object/slays.c */
/* Exercise functions from obj-slays.{h,c}. */

#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"
#include "init.h"
#include "mon-spell.h"
#include "obj-slays.h"
#include "player-birth.h"
#include "player-timed.h"
#include "z-color.h"
#include "z-util.h"
#include "z-virt.h"

struct slays_test_state {
	bool *slays;
	bool *brands;
};

int setup_tests(void **state)
{
	struct slays_test_state *ts;

	set_file_paths();
	init_angband();
#ifdef UNIX
	/* Necessary for creating the randart file. */
	create_needed_dirs();
#endif
	ts = mem_alloc(sizeof(*ts));
	ts->slays = mem_zalloc(z_info->slay_max * sizeof(*ts->slays));
	ts->brands = mem_zalloc(z_info->brand_max * sizeof(*ts->brands));
	/* Set up the player. */
	if (!player_make_simple(NULL, NULL, "Tester")) {
		mem_free(ts->brands);
		mem_free(ts->slays);
		mem_free(ts);
		cleanup_angband();
		return 1;
	}
	*state = ts;
	return 0;
}

int teardown_tests(void *state)
{
	struct slays_test_state *ts = state;

	mem_free(ts->brands);
	mem_free(ts->slays);
	mem_free(ts);
	cleanup_angband();
	return 0;
}

static void fill_in_monster_base(struct monster_base *base)
{
	static char name[20] = "blob";
	static char text[20] = "blob";

	base->next = NULL;
	base->name = name;
	base->text = text;
	rf_wipe(base->flags);
	base->d_char = L'b';
	base->pain = NULL;
}

static void fill_in_monster_race(struct monster_race *race,
		struct monster_base *base)
{
	static char name[20] = "white blob";
	static char text[20] = "white blob";

	race->next = NULL;
	race->ridx = 1;
	race->name = name;
	race->text = text;
	race->plural = NULL;
	race->base = base;
	race->avg_hp = 10;
	race->ac = 12;
	race->sleep = 0;
	race->hearing = 20;
	race->smell = 20;
	race->speed = 110;
	race->light = 0;
	race->mexp = 0;
	race->freq_innate = 0;
	race->freq_spell = 0;
	race->spell_power = 0;
	rf_wipe(race->flags);
	rsf_wipe(race->spell_flags);
	race->blow = &test_blow[0];
	race->level = 1;
	race->rarity = 1;
	race->d_attr = COLOUR_WHITE;
	race->d_char = base->d_char;
	race->max_num = 100;
	race->cur_num = 0;
	race->drops = NULL;
	race->friends = NULL;
	race->friends_base = NULL;
	race->mimic_kinds = NULL;
	race->shapes = NULL;
	race->num_shapes = 0;
}

static void fill_in_monster(struct monster *mon, struct monster_race *race)
{
	mon->race = race;
	mon->original_race = NULL;
	mon->midx = 1;
	mon->grid = loc(1, 1);
	mon->hp = race->avg_hp;
	mon->maxhp = race->avg_hp;
	memset(mon->m_timed, 0, MON_TMD_MAX * sizeof(mon->m_timed[0]));
	mon->mspeed = race->speed;
	mon->energy = 0;
	mon->cdis = 100;
	rf_wipe(mon->mflag);
	mon->mimicked_obj = NULL;
	mon->held_obj = NULL;
	mon->attr = race->d_attr;
	memset(&mon->known_pstate, 0, sizeof(mon->known_pstate));
	mon->target.grid = loc(0, 0);
	mon->target.midx = 0;
	memset(mon->group_info, 0, GROUP_MAX * sizeof(mon->group_info[0]));
	mon->heatmap.grids = NULL;
	mon->min_range = 0;
	mon->best_range = 0;
}

static void fill_in_object_base(struct object_base *base)
{
	static char name[20] = "weapon";

	base->name = name;
	base->tval = 1;
	base->next = NULL;
	base->attr = COLOUR_WHITE;
	of_wipe(base->flags);
	kf_wipe(base->kind_flags);
	memset(base->el_info, 0, ELEM_MAX * sizeof(base->el_info[0]));
	base->break_perc = 0;
	base->max_stack = 40;
	base->num_svals = 1;
}

static void fill_in_object_kind(struct object_kind *kind,
		struct object_base *base)
{
	static char name[20] = "weapon";
	static char text[20] = "weapon";

	kind->name = name;
	kind->text = text;
	kind->base = base;
	kind->next = NULL;
	kind->kidx = 1;
	kind->tval = base->tval;
	kind->sval = 1;
	kind->pval.base = 0;
	kind->pval.dice = 0;
	kind->pval.sides = 0;
	kind->pval.m_bonus = 0;
	kind->to_h.base = 0;
	kind->to_h.dice = 0;
	kind->to_h.sides = 0;
	kind->to_h.m_bonus = 0;
	kind->to_d.base = 0;
	kind->to_d.dice = 0;
	kind->to_d.sides = 0;
	kind->to_d.m_bonus = 0;
	kind->to_a.base = 0;
	kind->to_a.dice = 0;
	kind->to_a.sides = 0;
	kind->to_a.m_bonus = 0;
	kind->ac = 0;
	kind->dd = 1;
	kind->ds = 4;
	kind->weight = 10;
	kind->cost = 0;
	of_wipe(kind->flags);
	kf_wipe(kind->kind_flags);
	kind->brands = NULL;
	kind->slays = NULL;
	kind->curses = NULL;
	kind->d_attr = COLOUR_WHITE;
	kind->d_char = L'/';
	kind->alloc_prob = 20;
	kind->alloc_min = 0;
	kind->alloc_max = 100;
	kind->level = 1;
	kind->activation = NULL;
	kind->effect = NULL;
	kind->power = 0;
	kind->effect_msg = NULL;
	kind->vis_msg = NULL;
	kind->time.base = 0;
	kind->time.dice = 0;
	kind->time.sides = 0;
	kind->time.m_bonus = 0;
	kind->charge.base = 0;
	kind->charge.dice = 0;
	kind->charge.sides = 0;
	kind->charge.m_bonus = 0;
	kind->gen_mult_prob = 0;
	kind->stack_size.base = 1;
	kind->stack_size.dice = 0;
	kind->stack_size.sides = 0;
	kind->stack_size.m_bonus = 0;
	kind->flavor = NULL;
	kind->note_aware = 0;
	kind->note_unaware = 0;
	kind->aware = true;
	kind->tried = true;
	kind->ignore = false;
	kind->everseen = true;
}

static void fill_in_object(struct object *obj, struct object_kind *kind)
{
	obj->kind = kind;
	obj->ego = NULL;
	obj->artifact = NULL;
	obj->prev = NULL;
	obj->next = NULL;
	obj->known = NULL;
	obj->oidx = 1;
	obj->grid = loc(1, 1);
	obj->tval = kind->tval;
	obj->sval = kind->sval;
	obj->pval = randcalc(kind->pval, 0, MINIMISE);
	obj->weight = kind->weight;
	obj->dd = kind->dd;
	obj->ds = kind->ds;
	obj->ac = kind->ac;
	obj->to_a = randcalc(kind->to_a, 0, MINIMISE);
	obj->to_h = randcalc(kind->to_h, 0, MINIMISE);
	obj->to_d = randcalc(kind->to_d, 0, MINIMISE);
	of_wipe(obj->flags);
	memset(obj->modifiers, 0, OBJ_MOD_MAX * sizeof(obj->modifiers[0]));
	memset(obj->el_info, 0, ELEM_MAX * sizeof(obj->el_info[0]));
	obj->brands = kind->brands;
	obj->slays = kind->slays;
	obj->curses = NULL;
	obj->effect = kind->effect;
	obj->effect_msg = kind->effect_msg;
	obj->activation = kind->activation;
	obj->time.base = 0;
	obj->time.dice = 0;
	obj->time.sides = 0;
	obj->time.m_bonus = 0;
	obj->timeout = 0;
	obj->number = 1;
	obj->notice = 0;
	obj->held_m_idx = 0;
	obj->mimicking_m_idx = 0;
	obj->origin = ORIGIN_DROP_WIZARD;
	obj->origin_depth = 1;
	obj->origin_race = NULL;
	obj->note = 0;
}

static int brand_index_to_timed_index(int idx)
{
	int i = 0;

	while (1) {
		if (i >= TMD_MAX) return -1;
		if (timed_effects[i].temp_brand == idx) return i;
		++i;
	}
}

static bool set_temporary_brand(struct player *p, int idx)
{
	int tmd_idx = brand_index_to_timed_index(idx);

	if (tmd_idx < 0) return false;
	(void) player_set_timed(player, tmd_idx, 100, false, false);
	return true;
}

static bool clear_temporary_brand(struct player *p, int idx)
{
	int tmd_idx = brand_index_to_timed_index(idx);

	if (tmd_idx < 0) return false;
	(void) player_clear_timed(player, tmd_idx, false, false);
	return true;
}

static int slay_index_to_timed_index(int idx)
{
	int i = 0;

	while (1) {
		if (i >= TMD_MAX) return -1;
		if (timed_effects[i].temp_slay == idx) return i;
		++i;
	}
}

static bool set_temporary_slay(struct player *p, int idx)
{
	int tmd_idx = slay_index_to_timed_index(idx);

	if (tmd_idx < 0) return false;
	(void) player_set_timed(player, tmd_idx, 100, false, false);
	return true;
}

static bool clear_temporary_slay(struct player *p, int idx)
{
	int tmd_idx = slay_index_to_timed_index(idx);

	if (tmd_idx < 0) return false;
	(void) player_clear_timed(player, tmd_idx, false, false);
	return true;
}

static int test_same_monsters_slain(void *state)
{
	int i1, i2;

	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		for (i2 = i1; i2 < z_info->slay_max; ++i2) {
			bool s1 = same_monsters_slain(i1, i2);
			bool s2 = same_monsters_slain(i2, i1);

			eq(s1, s2);
			if (i1 == i2) {
				require(s1);
			} else if (s1) {
				require(slays[i1].race_flag
					== slays[i2].race_flag
					&& ((!slays[i1].base
					&& !slays[i2].base)
					|| (slays[i1].base && slays[i2].base
					&& streq(slays[i1].base,
					slays[i2].base))));
			} else {
				require(slays[i1].race_flag
					!= slays[i2].race_flag
					|| ((!slays[i1].base && slays[i2].base)
					|| (slays[i1].base && !slays[i2].base)
					|| (slays[i1].base && slays[i2].base
					&& !streq(slays[i1].base,
					slays[i2].base))));
			}
		}
	}
	ok;
}

static int test_player_has_temporary_brand(void *state)
{
	int i1;

	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		(void) clear_temporary_brand(player, i1);
	}
	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		require(!player_has_temporary_brand(player, i1));
	}
	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		int i2;

		if (!set_temporary_brand(player, i1)) continue;
		for (i2 = 1; i2 < z_info->brand_max; ++i2) {
			if (i2 == i1) {
				require(player_has_temporary_brand(player, i2));
			} else {
				require(!player_has_temporary_brand(player, i2));
			}
		}
		require(clear_temporary_brand(player, i1));
	}
	ok;
}

static int test_player_has_temporary_slay(void *state)
{
	int i1;

	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		(void) clear_temporary_slay(player, i1);
	}
	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		require(!player_has_temporary_slay(player, i1));
	}
	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		int i2;

		if (!set_temporary_slay(player, i1)) continue;
		for (i2 = 1; i2 < z_info->slay_max; ++i2) {
			if (i2 == i1) {
				require(player_has_temporary_slay(player, i2));
			} else {
				require(!player_has_temporary_slay(player, i2));
			}
		}
		require(clear_temporary_slay(player, i1));
	}
	ok;
}

static int test_get_monster_brand_multiplier(void *state)
{
	struct monster_base dummy_base;
	struct monster_race dummy_race;
	struct monster dummy;
	int i1;

	fill_in_monster_base(&dummy_base);
	fill_in_monster_race(&dummy_race, &dummy_base);
	fill_in_monster(&dummy, &dummy_race);

	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		int mult;

		mult = get_monster_brand_multiplier(&dummy, &brands[i1], false);
		eq(mult, brands[i1].multiplier);
		mult = get_monster_brand_multiplier(&dummy, &brands[i1], true);
		eq(mult, brands[i1].o_multiplier);

		if (brands[i1].vuln_flag) {
			rf_on(dummy_race.flags, brands[i1].vuln_flag);
			mult = get_monster_brand_multiplier(&dummy,
				&brands[i1], false);
			eq(mult, 2 * brands[i1].multiplier);
			mult = get_monster_brand_multiplier(&dummy,
				&brands[i1], true);
			eq(mult, 2 * (brands[i1].o_multiplier - 10) + 10);
			rf_off(dummy_race.flags, brands[i1].vuln_flag);
		}
	}
	ok;
}

static int test_improve_attack_modifier(void *state)
{
	struct slays_test_state *ts = state;
	bool pctdam = OPT(player, birth_percent_damage);
	struct object_base weapon_base;
	struct object_kind weapon_kind;
	struct object weapon;
	struct monster_base dummy_base;
	struct monster_race dummy_race;
	struct monster dummy;
	char *old_base;
	int b, s, i1;
	char verb[20];

	fill_in_object_base(&weapon_base);
	fill_in_object_kind(&weapon_kind, &weapon_base);
	fill_in_object(&weapon, &weapon_kind);
	fill_in_monster_base(&dummy_base);
	fill_in_monster_race(&dummy_race, &dummy_base);
	fill_in_monster(&dummy, &dummy_race);
	old_base = dummy_race.base->name;

	/* Has no slays or brands that would be effective. */
	b = 0;
	s = 0;
	my_strcpy(verb, "hit", sizeof(verb));
	improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb, false);
	require(b == 0 && s == 0 && streq(verb, "hit"));
	b = 0;
	s = 0;
	my_strcpy(verb, "hit", sizeof(verb));
	improve_attack_modifier(player, NULL, &dummy, &b, &s, verb, false);
	require(b == 0 && s == 0 && streq(verb, "hit"));
	b = 0;
	s = 0;
	my_strcpy(verb, "hit", sizeof(verb));
	improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb, true);
	require(b == 0 && s == 0 && streq(verb, "hit"));
	b = 0;
	s = 0;
	my_strcpy(verb, "hit", sizeof(verb));
	improve_attack_modifier(player, NULL, &dummy, &b, &s, verb, true);
	require(b == 0 && s == 0 && streq(verb, "hit"));

	/*
	 * Has no slays or brands that would be effective; check that preset
	 * value of b or s is preserved.
	 */
	i1 = rand_range(1, z_info->brand_max - 1);
	b = i1;
	s = 0;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb, false);
	require(b == i1 && s == 0 && streq(verb, "punch"));
	b = i1;
	s = 0;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb, true);
	require(b == i1 && s == 0 && streq(verb, "punch"));
	b = i1;
	s = 0;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, NULL, &dummy, &b, &s, verb, false);
	require(b == i1 && s == 0 && streq(verb, "punch"));
	b = i1;
	s = 0;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, NULL, &dummy, &b, &s, verb, true);
	require(b == i1 && s == 0 && streq(verb, "punch"));
	i1 = rand_range(1, z_info->slay_max - 1);
	b = 0;
	s = i1;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb, false);
	require(b == 0 && s == i1 && streq(verb, "punch"));
	b = 0;
	s = i1;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb, true);
	require(b == 0 && s == i1 && streq(verb, "punch"));
	b = 0;
	s = i1;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, NULL, &dummy, &b, &s, verb, false);
	require(b == 0 && s == i1 && streq(verb, "punch"));
	b = 0;
	s = i1;
	my_strcpy(verb, "punch", sizeof(verb));
	improve_attack_modifier(player, NULL, &dummy, &b, &s, verb, true);
	require(b == 0 && s == i1 && streq(verb, "punch"));

	/* Check temporary slay or brand. */
	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		if (!set_temporary_brand(player, i1)) continue;
		b = 0;
		s = 0;
		my_strcpy(verb, "punch", sizeof(verb));
		improve_attack_modifier(player, NULL, &dummy, &b, &s,
			verb, false);
		require(b == i1 && s == 0 && streq(verb, brands[i1].verb));
		require(clear_temporary_brand(player, i1));
	}
	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		if (!slays[i1].base|| !slays[i1].race_flag) continue;
		if (!set_temporary_slay(player, i1)) continue;
		rf_on(dummy_race.flags, slays[i1].race_flag);
		if (slays[i1].base) {
			dummy_race.base->name = slays[i1].base;
		}
		b = 0;
		s = 0;
		my_strcpy(verb, "punch", sizeof(verb));
		improve_attack_modifier(player, NULL, &dummy, &b, &s,
			verb, false);
		require(b == 0 && s == i1 && streq(verb, slays[i1].melee_verb));
		require(clear_temporary_slay(player, i1));
		rf_off(dummy_race.flags, slays[i1].race_flag);
		if (slays[i1].base) {
			dummy_race.base->name = old_base;
		}
	}

	memset(ts->slays, 0, z_info->slay_max * sizeof(*ts->slays));
	memset(ts->brands, 0, z_info->brand_max * sizeof(*ts->brands));

	/* Test with one brand on the weapon. */
	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		bool *old_brands = weapon.brands;

		weapon.brands = ts->brands;
		weapon.brands[i1] = true;

		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			false);
		require(b == i1 && s == 0 && streq(verb, brands[i1].verb));
		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			true);
		require(b == i1 && s == 0 && prefix(verb, brands[i1].verb)
			&& suffix(verb, "s"));

		rf_on(dummy.race->flags, brands[i1].resist_flag);
		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			false);
		require(b == 0 && s == 0 && streq(verb, "hit"));
		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			true);
		require(b == 0 && s == 0 && streq(verb, "hit"));
		rf_off(dummy.race->flags, brands[i1].resist_flag);

		weapon.brands[i1] = false;
		weapon.brands = old_brands;
	}

	/* Test with one slay on the weapon. */
	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		bool *old_slays;

		if (!slays[i1].base|| !slays[i1].race_flag) continue;

		old_slays = weapon.slays;
		weapon.slays = ts->slays;
		weapon.slays[i1] = true;

		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			false);
		require(b == 0 && s == 0 && streq(verb, "hit"));
		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			true);
		require(b == 0 && s == 0 && streq(verb, "hit"));

		rf_on(dummy_race.flags, slays[i1].race_flag);
		if (slays[i1].base) {
			dummy_race.base->name = slays[i1].base;
		}
		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			false);
		require(b == 0 && s == i1 && streq(verb, slays[i1].melee_verb));
		b = 0;
		s = 0;
		my_strcpy(verb, "hit", sizeof(verb));
		improve_attack_modifier(player, &weapon, &dummy, &b, &s, verb,
			true);
		require(b == 0 && s == i1 && streq(verb, slays[i1].range_verb));
		rf_off(dummy_race.flags, slays[i1].race_flag);
		if (slays[i1].base) {
			dummy_race.base->name = old_base;
		}

		weapon.slays[i1] = false;
		weapon.slays = old_slays;
	}

	/*
	 * Test with a combination of two (both brands, one slay and one brand,
	 * or both slays).
	 */
	for (i1 = 1; i1 < z_info->brand_max; ++i1) {
		int i2;

		for (i2 = i1 + 1; i2 < z_info->brand_max; ++i2) {
			bool *old_brands;
			int expected;

			if (brands[i1].resist_flag == brands[i2].resist_flag)
				continue;

			old_brands = weapon.brands;
			weapon.brands = ts->brands;
			weapon.brands[i1] = true;
			weapon.brands[i2] = true;

			/* Susceptible to both */
			if (pctdam) {
				if (brands[i1].o_multiplier
						>= brands[i2].o_multiplier) {
					expected = i1;
				} else {
					expected = i2;
				}
			} else {
				if (brands[i1].multiplier
						>= brands[i2].multiplier) {
					expected = i1;
				} else {
					expected = i2;
				}
			}
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == expected && s == 0
				&& streq(verb, brands[expected].verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == expected && s == 0
				&& prefix(verb, brands[expected].verb)
				&& suffix(verb, "s"));

			/* Only susceptible to the second */
			rf_on(dummy.race->flags, brands[i1].resist_flag);
			expected = i2;
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == expected && s == 0
				&& streq(verb, brands[expected].verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == expected && s == 0
				&& prefix(verb, brands[expected].verb)
				&& suffix(verb, "s"));
			rf_off(dummy.race->flags, brands[i1].resist_flag);

			/* Only susceptible to the first */
			rf_on(dummy.race->flags, brands[i2].resist_flag);
			expected = i1;
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == expected && s == 0
				&& streq(verb, brands[expected].verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == expected && s == 0
				&& prefix(verb, brands[expected].verb)
				&& suffix(verb, "s"));
			rf_off(dummy.race->flags, brands[i2].resist_flag);

			if (brands[i1].vuln_flag) {
				/* Especially vulnerable to the first */
				rf_on(dummy.race->flags, brands[i1].vuln_flag);
				if (pctdam) {
					if (2 * (brands[i1].o_multiplier - 10)
							+ 10 >= brands[i2].o_multiplier) {
						expected = i1;
					} else {
						expected = i2;
					}
				} else {
					if (2 * brands[i1].multiplier
							>= brands[i2].multiplier) {
						expected = i1;
					} else {
						expected = i2;
					}
				}
				b = 0;
				s = 0;
				my_strcpy(verb, "hit", sizeof(verb));
				improve_attack_modifier(player, &weapon, &dummy,
					&b, &s, verb, false);
				require(b == expected && s == 0
					&& streq(verb, brands[expected].verb));
				b = 0;
				s = 0;
				my_strcpy(verb, "hit", sizeof(verb));
				improve_attack_modifier(player, &weapon, &dummy,
					&b, &s, verb, true);
				require(b == expected && s == 0
					&& prefix(verb, brands[expected].verb)
					&& suffix(verb, "s"));
				rf_off(dummy.race->flags, brands[i1].vuln_flag);
			}

			if (brands[i2].vuln_flag) {
				/* Especially vulnerable to the second */
				rf_on(dummy.race->flags, brands[i2].vuln_flag);
				if (pctdam) {
					if (2 * (brands[i2].o_multiplier - 10)
							+ 10 > brands[i1].o_multiplier) {
						expected = i2;
					} else {
						expected = i1;
					}
				} else {
					if (2 * brands[i2].multiplier
							> brands[i1].multiplier) {
						expected = i2;
					} else {
						expected = i1;
					}
				}
				b = 0;
				s = 0;
				my_strcpy(verb, "hit", sizeof(verb));
				improve_attack_modifier(player, &weapon, &dummy,
					&b, &s, verb, false);
				require(b == expected && s == 0
					&& streq(verb, brands[expected].verb));
				b = 0;
				s = 0;
				my_strcpy(verb, "hit", sizeof(verb));
				improve_attack_modifier(player, &weapon, &dummy,
					&b, &s, verb, true);
				require(b == expected && s == 0
					&& prefix(verb, brands[expected].verb)
					&& suffix(verb, "s"));
				rf_off(dummy.race->flags, brands[i2].vuln_flag);
			}

			weapon.brands[i1] = false;
			weapon.brands[i2] = false;
			weapon.brands = old_brands;
		}

		for (i2 = 1; i2 < z_info->slay_max; ++i2) {
			bool *old_brands;
			bool *old_slays;
			int es, eb;
			char em_verb[20], er_verb[20];
			bool brand_better;

			if (!slays[i2].base || !slays[i2].race_flag) continue;

			old_brands = weapon.brands;
			weapon.brands = ts->brands;
			weapon.brands[i1] = true;
			old_slays = weapon.slays;
			weapon.slays = ts->slays;
			weapon.slays[i2] = true;

			/* Susceptible to both */
			rf_on(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = slays[i2].base;
			}
			if (pctdam) {
				brand_better = (brands[i1].o_multiplier
					>= slays[i2].o_multiplier);
			} else {
				brand_better = (brands[i1].multiplier
					>= slays[i2].multiplier);
			}
			if (brand_better) {
				eb = i1;
				es = 0;
				my_strcpy(em_verb, brands[i1].verb,
					sizeof(em_verb));
				my_strcpy(er_verb, brands[i1].verb,
					sizeof(er_verb));
				my_strcat(er_verb, "s", sizeof(er_verb));
			} else {
				eb = 0;
				es = i2;
				my_strcpy(em_verb, slays[i2].melee_verb,
					sizeof(em_verb));
				my_strcpy(er_verb, slays[i2].range_verb,
					sizeof(er_verb));
			}
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == eb && s == es && streq(verb, em_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == eb && s == es && streq(verb, er_verb));
			rf_off(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = old_base;
			}

			/*
			 * Susceptible to both; especially vulnerable to the
			 * brand
			 */
			if (brands[i1].vuln_flag) {
				rf_on(dummy_race.flags, brands[i1].vuln_flag);
				rf_on(dummy_race.flags, slays[i2].race_flag);
				if (slays[i2].base) {
					dummy_race.base->name = slays[i2].base;
				}
				if (pctdam) {
					brand_better = (2
						* (brands[i1].o_multiplier
						- 10) + 10
						>= slays[i2].o_multiplier);
				} else {
					brand_better =
						(2 * brands[i1].multiplier
						>= slays[i2].multiplier);
				}
				if (brand_better) {
					eb = i1;
					es = 0;
					my_strcpy(em_verb, brands[i1].verb,
						sizeof(em_verb));
					my_strcpy(er_verb, brands[i1].verb,
						sizeof(er_verb));
					my_strcat(er_verb, "s",
						sizeof(er_verb));
				} else {
					eb = 0;
					es = i2;
					my_strcpy(em_verb, slays[i2].melee_verb,
						sizeof(em_verb));
					my_strcpy(er_verb, slays[i2].range_verb,
						sizeof(er_verb));
				}
				b = 0;
				s = 0;
				my_strcpy(verb, "hit", sizeof(verb));
				improve_attack_modifier(player, &weapon, &dummy,
					&b, &s, verb, false);
				require(b == eb && s == es
					&& streq(verb, em_verb));
				b = 0;
				s = 0;
				my_strcpy(verb, "hit", sizeof(verb));
				improve_attack_modifier(player, &weapon, &dummy,
					&b, &s, verb, true);
				require(b == eb && s == es
					&& streq(verb, er_verb));
				rf_off(dummy_race.flags, brands[i1].vuln_flag);
				rf_off(dummy_race.flags, slays[i2].race_flag);
				if (slays[i2].base) {
					dummy_race.base->name = old_base;
				}
			}

			/* Only susceptible to the brand */
			eb = i1;
			es = 0;
			my_strcpy(em_verb, brands[i1].verb, sizeof(em_verb));
			my_strcpy(er_verb, brands[i1].verb, sizeof(er_verb));
			my_strcat(er_verb, "s", sizeof(er_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == eb && s == es && streq(verb, em_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == eb && s == es && streq(verb, er_verb));

			/* Only susceptible to the slay */
			rf_on(dummy_race.flags, brands[i1].resist_flag);
			rf_on(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = slays[i2].base;
			}
			eb = 0;
			es = i2;
			my_strcpy(em_verb, slays[i2].melee_verb,
				sizeof(em_verb));
			my_strcpy(er_verb, slays[i2].range_verb,
				sizeof(er_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == eb && s == es && streq(verb, em_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == eb && s == es && streq(verb, er_verb));
			rf_off(dummy_race.flags, brands[i1].resist_flag);
			rf_off(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = old_base;
			}

			weapon.brands[i1] = false;
			weapon.brands = old_brands;
			weapon.slays[i2] = false;
			weapon.slays = old_slays;
		}
	}

	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		int i2;

		if (!slays[i1].base || !slays[i1].race_flag) continue;

		for (i2 = i1 + 1; i2 < z_info->slay_max; ++i2) {
			bool *old_slays;
			int expected;

			if (!slays[i2].base || !slays[i2].race_flag) continue;
			if (slays[i1].base && slays[i2].base
					&& !streq(slays[i1].base,
					slays[i2].base))
				continue;
			if (slays[i1].race_flag == slays[i2].race_flag
					&& ((slays[i1].base && slays[i2].base)
					|| (!slays[i1].base && !slays[i2].base)))
				continue;

			old_slays = weapon.slays;
			weapon.slays = ts->slays;
			weapon.slays[i1] = true;
			weapon.slays[i2] = true;

			/* Susceptible to both */
			rf_on(dummy_race.flags, slays[i1].race_flag);
			if (slays[i1].base) {
				dummy_race.base->name = slays[i1].base;
			}
			rf_on(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = slays[i2].base;
			}
			if (pctdam) {
				if (slays[i1].o_multiplier
						>= slays[i2].multiplier) {
					expected = i1;
				} else {
					expected = i2;
				}
			} else {
				if (slays[i1].multiplier
						>= slays[i2].multiplier) {
					expected = i1;
				} else {
					expected = i2;
				}
			}
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == 0 && s == expected
				&& streq(verb, slays[expected].melee_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == 0 && s == expected
				&& streq(verb, slays[expected].range_verb));
			rf_off(dummy_race.flags, slays[i1].race_flag);
			if (slays[i1].base) {
				dummy_race.base->name = old_base;
			}
			rf_off(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = old_base;
			}

			/* Only susceptible to the first */
			rf_on(dummy_race.flags, slays[i1].race_flag);
			if (slays[i1].base) {
				dummy_race.base->name = slays[i1].base;
			}
			expected = i1;
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == 0 && s == expected
				&& streq(verb, slays[expected].melee_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == 0 && s == expected
				&& streq(verb, slays[expected].range_verb));
			rf_off(dummy_race.flags, slays[i1].race_flag);
			if (slays[i1].base) {
				dummy_race.base->name = old_base;
			}

			/* Only susceptible to the second */
			rf_on(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = slays[i2].base;
			}
			expected = i2;
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, false);
			require(b == 0 && s == expected
				&& streq(verb, slays[expected].melee_verb));
			b = 0;
			s = 0;
			my_strcpy(verb, "hit", sizeof(verb));
			improve_attack_modifier(player, &weapon, &dummy,
				&b, &s, verb, true);
			require(b == 0 && s == expected
				&& streq(verb, slays[expected].range_verb));
			rf_off(dummy_race.flags, slays[i2].race_flag);
			if (slays[i2].base) {
				dummy_race.base->name = old_base;
			}

			weapon.slays[i1] = false;
			weapon.slays[i2] = false;
			weapon.slays = old_slays;
		}
	}

	ok;
}

static int test_react_to_slay(void *state)
{
	struct slays_test_state *ts = state;
	struct object_base weapon_base;
	struct object_kind weapon_kind;
	struct object weapon;
	struct monster_base dummy_base;
	struct monster_race dummy_race;
	struct monster dummy;
	int i1;

	fill_in_object_base(&weapon_base);
	fill_in_object_kind(&weapon_kind, &weapon_base);
	fill_in_object(&weapon, &weapon_kind);
	fill_in_monster_base(&dummy_base);
	fill_in_monster_race(&dummy_race, &dummy_base);
	fill_in_monster(&dummy, &dummy_race);

	/* Has no slays that would be effective. */
	eq(react_to_slay(&weapon, &dummy), false);

	memset(ts->slays, 0, z_info->slay_max * sizeof(*ts->slays));
	for (i1 = 1; i1 < z_info->slay_max; ++i1) {
		char *old_base;
		bool *old_slays;

		if (!slays[i1].base || !slays[i1].race_flag) continue;

		rf_on(dummy_race.flags, slays[i1].race_flag);
		if (slays[i1].base) {
			old_base = dummy_race.base->name;
			dummy_race.base->name = slays[i1].base;
		}
		/* Monster is vulnerable, but weapon doesn't have the slay. */
		eq(react_to_slay(&weapon, &dummy), false);
		old_slays = weapon.slays;
		weapon.slays = ts->slays;
		weapon.slays[i1] = true;
		/* Monster is vulnerable, and weapon has the slay. */
		eq(react_to_slay(&weapon, &dummy), true);
		weapon.slays[i1] = false;
		weapon.slays = old_slays;
		rf_off(dummy_race.flags, slays[i1].race_flag);
		if (slays[i1].base) {
			dummy_race.base->name = old_base;
		}
	}

	ok;
}

const char *suite_name = "object/slays";
struct test tests[] = {
	{ "same_monsters_slain", test_same_monsters_slain },
	{ "player_has_temporary_brand", test_player_has_temporary_brand },
	{ "player_has_temporary_slay", test_player_has_temporary_slay },
	{ "get_monster_brand_multiplier", test_get_monster_brand_multiplier },
	{ "improve_attack_modifier", test_improve_attack_modifier },
	{ "react_to_slay", test_react_to_slay },
	{ NULL, NULL }
};
