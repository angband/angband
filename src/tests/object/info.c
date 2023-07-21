/* object/info.c */
/* Exercise functions from obj-info.{h,c}. */

#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"
#include "init.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "obj-gear.h"
#include "obj-info.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "z-color.h"
#include "z-virt.h"

struct info_test_state {
	struct monster_base target_base;
	struct monster_race target_race;
	struct monster *target;
	int *damage_results;
	int *oi_brands;
	int *oi_slays;
};

/*
 * This is the number of hits to record to get a Monte Carlo calculation for
 * an average damage.
 */
#define NHITS (10000)

/*
 * This is the number of standard deviations of the mean an average from the
 * Monte Carlo simulation has to be from the object information result to
 * declare a test failure.
 */
#define STDFAIL (5.0)

/*
 * Set up a macro so details about the averages that are too far apart are
 * visible when run with the -v option.
 */
#define check_averages(desc, oi, mca, mcv) \
	{ \
		double dev = 0.1 * oi - mca; \
		if ((dev < -0.05 || dev > 0.05) \
				&& dev * dev > STDFAIL * STDFAIL * mc_var) { \
			if (verbose) { \
				showfail(); \
				(void)printf("    %s:%d: %s %d.%d object info too far from Monte Carlo result %.2f with %.4f variance\n", \
					suite_name, \
					__LINE__, \
					desc, \
					oi / 10, oi % 10, \
					mca, mcv); \
			} \
			return 1; \
		} \
	}

static void fill_in_monster_base(struct monster_base *base);
static void fill_in_monster_race(struct monster_race *race,
		struct monster_base *base);

int setup_tests(void **state)
{
	struct monster_group_info gi = { 0, 0 };
	struct info_test_state *ts;

	set_file_paths();
	init_angband();
#ifdef UNIX
	/* Necessary for creating the randart file. */
	create_needed_dirs();
#endif

	/* Set up for recording damage results. */
	ts = mem_alloc(sizeof(*ts));
	ts->damage_results = mem_alloc(NHITS * sizeof(*ts->damage_results));
	ts->oi_brands = mem_alloc(z_info->brand_max * sizeof(*ts->oi_brands));
	ts->oi_slays = mem_alloc(z_info->slay_max * sizeof(*ts->oi_slays));

	/* Set up the player. */
	if (!player_make_simple(NULL, NULL, "Tester")) {
		mem_free(ts->oi_slays);
		mem_free(ts->oi_brands);
		mem_free(ts->damage_results);
		mem_free(ts);
		cleanup_angband();
		return 1;
	}

	/* Give the player something to operate in. */
	cave = t_build_arena(10, 10);
	player_place(cave, player, loc(cave->width / 2, cave->height / 2));

	/* Give the player something to attack. */
	fill_in_monster_base(&ts->target_base);
	fill_in_monster_race(&ts->target_race, &ts->target_base);
	place_new_monster(cave, loc(player->grid.x + 1, player->grid.y),
		&ts->target_race, false, false, gi, ORIGIN_DROP_WIZARD);
	ts->target = square_monster(cave,
		loc(player->grid.x + 1, player->grid.y));

	*state = ts;
	return 0;
}

int teardown_tests(void *state)
{
	struct info_test_state *ts = (struct info_test_state*)state;

	wipe_mon_list(cave, player);
	cleanup_angband();
	mem_free(ts->oi_slays);
	mem_free(ts->oi_brands);
	mem_free(ts->damage_results);
	mem_free(ts);
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
	race->avg_hp = 5000;
	race->ac = 1;
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
	/*
	 * Allow for at least two slays and two brands to be effective but do
	 * give immunity to at least two brands and two slays.  Do not give an
	 * enhanced vulnerability to a brand since the object information
	 * calculations never account for those.
	 */
	rf_on(race->flags, RF_UNDEAD);
	rf_on(race->flags, RF_EVIL);
	rf_on(race->flags, RF_IM_COLD);
	rf_on(race->flags, RF_IM_POIS);
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

static void wipe_brands_slays(struct object *weapon)
{
	if (weapon->brands) {
		mem_free(weapon->brands);
	}
	weapon->brands = NULL;
	if (weapon->slays) {
		mem_free(weapon->slays);
	}
	weapon->slays = NULL;
	if (weapon->known) {
		if (weapon->known->brands) {
			mem_free(weapon->known->brands);
			weapon->known->brands = NULL;
		}
		if (weapon->known->slays) {
			mem_free(weapon->known->slays);
			weapon->known->slays = NULL;
		}
	}
}

static bool is_similar_brand(int i, int j)
{
	return i == j || streq(brands[i].name, brands[j].name);
}

static bool object_has_similar_brand(const struct object *o, int i)
{
	int j;

	/* It has no brands at all - easy. */
	if (!o->brands) {
		return false;
	}
	/* It already has that brand - easy. */
	if (o->brands[i]) {
		return true;
	}
	/* Look for a similar brand. */
	j = 1;
	while (1) {
		if (j >= z_info->brand_max) {
			return false;
		}
		if (o->brands[j] && is_similar_brand(j, i)) {
			return true;
		}
		++j;
	}
}

static bool object_has_similar_slay(const struct object *o, int i)
{
	int j;

	/* It has no slays at all - easy. */
	if (!o->slays) {
		return false;
	}
	/* It already has that slay - easy. */
	if (o->slays[i]) {
		return true;
	}
	/* Look for a similar slay. */
	j = 1;
	while (1) {
		if (j >= z_info->slay_max) {
			return false;
		}
		if (o->slays[j] && same_monsters_slain(j, i)) {
			return true;
		}
		++j;
	}
}

static int add_random_effective_brand(struct player *p, struct object *o,
		struct monster *m)
{
	int i_add = 0, n = 0, i;

	for (i = 1; i < z_info->brand_max; ++i) {
		if (!rf_has(m->race->flags, brands[i].resist_flag)
				&& !object_has_similar_brand(o, i)) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add == 0) {
		return 0;
	}
	if (!o->brands) {
		o->brands = mem_zalloc(z_info->brand_max * sizeof(*o->brands));
	}
	o->brands[i_add] = true;
	object_learn_brand(p, o, i_add);
	player_know_object(p, o);
	return i_add;
}

static int add_random_ineffective_brand(struct player *p, struct object *o,
		struct monster *m)
{
	int i_add = 0, n = 0, i;

	for (i = 1; i < z_info->brand_max; ++i) {
		if (rf_has(m->race->flags, brands[i].resist_flag)
				&& !object_has_similar_brand(o, i)) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add == 0) {
		return 0;
	}
	if (!o->brands) {
		o->brands = mem_zalloc(z_info->brand_max * sizeof(*o->brands));
	}
	o->brands[i_add] = true;
	object_learn_brand(p, o, i_add);
	player_know_object(p, o);
	return i_add;
}

static int add_random_effective_slay(struct player *p, struct object *o,
		struct monster *m)
{
	int i_add = 0, n = 0, i;

	for (i = 1; i < z_info->slay_max; ++i) {
		if ((rf_has(m->race->flags, slays[i].race_flag) ||
				(slays[i].base && streq(slays[i].base,
				m->race->base->name)))
				&& !object_has_similar_slay(o, i)) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add == 0) {
		return 0;
	}
	if (!o->slays) {
		o->slays = mem_zalloc(z_info->slay_max * sizeof(*o->slays));
	}
	o->slays[i_add] = true;
	object_learn_slay(p, o, i_add);
	player_know_object(p, o);
	return i_add;
}

static int add_random_ineffective_slay(struct player *p, struct object *o,
		struct monster *m)
{
	int i_add = 0, n = 0, i;

	for (i = 1; i < z_info->slay_max; ++i) {
		if ((!rf_has(m->race->flags, slays[i].race_flag) &&
				(!slays[i].base || !streq(slays[i].base,
				m->race->base->name)))
				&& !object_has_similar_slay(o, i)) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add == 0) {
		return 0;
	}
	if (!o->slays) {
		o->slays = mem_zalloc(z_info->slay_max * sizeof(*o->slays));
	}
	o->slays[i_add] = true;
	object_learn_slay(p, o, i_add);
	player_know_object(p, o);
	return i_add;
}

static int add_random_effective_temporary_brand(struct player *p,
		struct monster *m)
{
	int i_add = -1, n = 0, i;

	for (i = 0; i < TMD_MAX; ++i) {
		if (timed_effects[i].temp_brand > 0
				&& timed_effects[i].temp_brand < z_info->brand_max
				&& !rf_has(m->race->flags, brands[timed_effects[i].temp_brand].resist_flag)) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add != -1) {
		(void)player_set_timed(p, i_add, 10, false, false);
	}
	return i_add;
}

static int add_random_ineffective_temporary_brand(struct player *p,
		struct monster *m)
{
	int i_add = -1, n = 0, i;

	for (i = 0; i < TMD_MAX; ++i) {
		if (timed_effects[i].temp_brand > 0
				&& timed_effects[i].temp_brand < z_info->brand_max
				&& rf_has(m->race->flags, brands[timed_effects[i].temp_brand].resist_flag)) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add != -1) {
		(void)player_set_timed(p, i_add, 10, false, false);
	}
	return i_add;
}

static int add_random_effective_temporary_slay(struct player *p,
		struct monster *m)
{
	int i_add = -1, n = 0, i;

	for (i = 0; i < TMD_MAX; ++i) {
		if (timed_effects[i].temp_slay > 0
				&& timed_effects[i].temp_slay < z_info->slay_max
				&& (rf_has(m->race->flags, slays[timed_effects[i].temp_slay].race_flag)
				|| (slays[timed_effects[i].temp_slay].base
				&& streq(slays[timed_effects[i].temp_slay].base,
				m->race->base->name)))) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add != -1) {
		(void)player_set_timed(p, i_add, 10, false, false);
	}
	return i_add;
}

static int add_random_ineffective_temporary_slay(struct player *p,
		struct monster *m)
{
	int i_add = -1, n = 0, i;

	for (i = 0; i < TMD_MAX; ++i) {
		if (timed_effects[i].temp_slay > 0
				&& timed_effects[i].temp_slay < z_info->slay_max
				&& (!rf_has(m->race->flags, slays[timed_effects[i].temp_slay].race_flag)
				&& (!slays[timed_effects[i].temp_slay].base
				|| !streq(slays[timed_effects[i].temp_slay].base,
				m->race->base->name)))) {
			++n;
			if (one_in_(n)) {
				i_add = i;
			}
		}
	}
	if (i_add != -1) {
		(void)player_set_timed(p, i_add, 10, false, false);
	}
	return i_add;
}

static void remove_all_temporary_brands_and_slays(struct player *p)
{
	int i;

	for (i = 0; i < TMD_MAX; ++i) {
		if ((timed_effects[i].temp_brand > 0
				&& timed_effects[i].temp_brand < z_info->brand_max)
				|| (timed_effects[i].temp_slay > 0
				&& timed_effects[i].temp_slay < z_info->slay_max)) {
			(void)player_set_timed(p, i, 0, false, false);
		}
	}
}

static void collect_damage_results(double *avg, double *avg_var, int *work,
		struct player *p, struct monster *m, struct object *weapon,
		struct object *launcher, bool throw)
{
	int weapon_slot = -1;
	int launcher_slot = -1;
	struct object *old_weapon = NULL;
	struct object *old_launcher = NULL;
	int i = 0;

	if (launcher) {
		launcher_slot = wield_slot(launcher);
		old_launcher = p->body.slots[launcher_slot].obj;
		p->body.slots[launcher_slot].obj = launcher;
		if (!old_launcher) {
			++p->upkeep->equip_cnt;
		}
		p->upkeep->total_weight += launcher->weight
			- ((old_launcher) ? old_launcher->weight : 0);
		p->upkeep->update |= (PU_BONUS);
	} else if (!throw) {
		weapon_slot = wield_slot(weapon);
		old_weapon = p->body.slots[weapon_slot].obj;
		p->body.slots[weapon_slot].obj = weapon;
		if (!old_weapon && weapon) {
			++p->upkeep->equip_cnt;
		} else if (old_weapon && !weapon) {
			assert(p->upkeep->equip_cnt > 0);
			--p->upkeep->equip_cnt;
		}
		p->upkeep->total_weight +=
			((weapon) ? weapon->weight : 0)
			- ((old_weapon) ? old_weapon->weight : 0);
		p->upkeep->update |= (PU_BONUS);
	}
	update_stuff(p);

	*avg = 0.0;
	while (i < NHITS) {
		if (launcher || throw) {
			struct attack_result ar;

			/* Need a missile of some sort. */
			assert(weapon);
			if (launcher) {
				ar = make_ranged_shot(p, weapon, m->grid);
			} else {
				ar = make_ranged_throw(p, weapon, m->grid);
			}
			mem_free(ar.hit_verb);
			if (ar.success) {
				work[i] = ar.dmg;
				*avg += ar.dmg;
				++i;
			}
		} else {
			int16_t old_hp = m->hp;
			bool afraid = false;

			(void)py_attack_real(p, m->grid, &afraid);
			/*
			 * Will not work if hits can have non-positive damage.
			 * Could test for the message string (but that'll break
			 * if the strings change).  Testing the message type is
			 * no better than testing the monster's HP since
			 * py_attack_real() currently uses MSG_MISS for hits
			 * that do not do positive damage.
			 */
			if (m->hp < old_hp) {
				int dam = old_hp - m->hp;

				work[i] = dam;
				*avg += dam;
				++i;
				/*
				 * Heal the monster so it won't die while
				 * collecting statistics.
				 */
				m->hp = old_hp;
			}
		}
	}
	*avg /= NHITS;

	*avg_var = 0.0;
	for (i = 0; i < NHITS; ++i) {
		double dev = work[i] - *avg;

		*avg_var += dev * dev;
	}
	*avg_var /= (double)(NHITS - 1) * (double)NHITS;

	/* Scale results to be per turn rather than per attack. */
	if (launcher) {
		double shots_per_turn = 0.1 * p->state.num_shots;

		*avg *= shots_per_turn;
		*avg_var *= shots_per_turn * shots_per_turn;
	} else if (!throw) {
		double blows_per_turn = 0.01 * p->state.num_blows;

		*avg *= blows_per_turn;
		*avg_var *= blows_per_turn * blows_per_turn;
	}

	/* Restore the player's equipment. */
	if (launcher_slot != -1) {
		assert(launcher && p->body.slots[launcher_slot].obj == launcher);
		p->body.slots[launcher_slot].obj = old_launcher;
		if (!old_launcher) {
			assert(p->upkeep->equip_cnt > 0);
			--p->upkeep->equip_cnt;
		}
		p->upkeep->total_weight +=
			((old_launcher) ? old_launcher->weight : 0)
			- launcher->weight;
		p->upkeep->update |= (PU_BONUS);
	}
	if (weapon_slot != -1) {
		assert(p->body.slots[weapon_slot].obj == weapon);
		p->body.slots[weapon_slot].obj = old_weapon;
		if (old_weapon && !weapon) {
			++p->upkeep->equip_cnt;
		} else if (!old_weapon && weapon) {
			assert(p->upkeep->equip_cnt > 0);
			--p->upkeep->equip_cnt;
		}
		p->upkeep->total_weight +=
			((old_weapon) ? old_weapon->weight : 0)
			- ((weapon) ? weapon->weight : 0);
		p->upkeep->update |= (PU_BONUS);
	}
	update_stuff(p);
}

static int test_melee_weapon_damage_info(void *state)
{
	struct info_test_state *ts = (struct info_test_state*)state;
	struct object_kind *weapon_kind, *gloves_kind;
	struct object *weapon, *gloves, *old_gloves;
	double mc_avg, mc_var;
	int gloves_slot, oi_avg, iadd0, iadd1;
	bool has_brand_slay, has_nonweap;

	remove_all_temporary_brands_and_slays(player);
	weapon_kind = get_obj_num(1, false, TV_SWORD);
	if (!weapon_kind) {
		/*
		 * The depth-dependent lookup did not work so try using the
		 * first kind.
		 */
		weapon_kind = lookup_kind(TV_SWORD, 1);
	}
	notnull(weapon_kind);
	weapon = object_new();
	object_prep(weapon, weapon_kind, 1, MINIMISE);
	/* Give it a damage bonus and multiple dice of damage. */
	weapon->to_d = 2;
	weapon->dd = 3;
	weapon->ds = 8;
	/* Make sure the player knows its properties. */
	weapon->known = object_new();
	wipe_brands_slays(weapon);
	object_set_base_known(player, weapon);
	object_touch(player, weapon);
	object_learn_on_wield(player, weapon);

	gloves_kind = get_obj_num(1, false, TV_GLOVES);
	if (!gloves_kind) {
		/*
		 * The depth-dependent lookup did not work so try using the
		 * first kind.
		 */
		gloves_kind = lookup_kind(TV_GLOVES, 1);
	}
	notnull(gloves_kind);
	gloves = object_new();
	object_prep(gloves, gloves_kind, 1, MINIMISE);
	/* Make sure the player knows its properties. */
	gloves->known = object_new();
	wipe_brands_slays(gloves);
	object_set_base_known(player, gloves);
	object_touch(player, gloves);
	object_learn_on_wield(player, gloves);
	/* Wield them. */
	gloves_slot = wield_slot(gloves);
	old_gloves = player->body.slots[gloves_slot].obj;
	player->body.slots[gloves_slot].obj = gloves;
	if (!old_gloves) {
		++player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight += gloves->weight
		- ((old_gloves) ? old_gloves->weight : 0);
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	/* Check armed combat without a slay or brand. */
	OPT(player, birth_percent_damage) = false;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("melee; no brand or slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("melee; O-combat; no brand or slay;", oi_avg, mc_avg,
		mc_var);

	/* Check armed combat with a brand on the weapon. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; one useful brand;", ts->oi_brands[iadd0], mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; O-combat; one useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; one useless brand;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; O-combat; one useless brand;", oi_avg, mc_avg,
		mc_var);

	/* Check armed combat with a slay on the weapon. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; one useful slay;", ts->oi_slays[iadd0], mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; O-combat; one useful slay;", ts->oi_slays[iadd0],
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; one useless slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; O-combat; one useless slay;", oi_avg, mc_avg,
		mc_var);

	/* Check armed combat with more than one slay or brand on the weapon. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_effective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_brands[iadd1];
	check_averages("melee; two useful brands;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_brands[iadd1];
	check_averages("melee; O-combat; two useful brands;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_slays[iadd1];
	check_averages("melee; one useful brand; one useful slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_slays[iadd1];
	check_averages("melee; O-combat; one useful brand; one useful slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_slay(player, weapon, ts->target);
	iadd1 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_slays[iadd1];
	check_averages("melee; two useful slays;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_slays[iadd1];
	check_averages("melee; O-combat; two useful slays;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("melee; one useful brand; one useless brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("melee; O-combat; one useful brand; one useless brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; one useful brand; one useless slay;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; O-combat; one useful brand; one useless slay;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	iadd1 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; one useless brand; one useful slay;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; O-combat; one useless brand; one useful slay;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_slay(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; one useful slay; one useless slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; O-combat; one useful slay; one useless slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("melee; two useless brands;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("melee; O-combat; two useless brands;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; one useless brand; one useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; O-combat; one useless brand; one useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_slay(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; two useless slays;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("melee; O-combat; two useless slays;", oi_avg, mc_avg,
		mc_var);

	/* Check armed combat with an off-weapon brand. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_brand(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	wipe_brands_slays(weapon);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; useful brand on gloves;", ts->oi_brands[iadd0],
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; O-combat; useful brand on gloves;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(gloves);
	iadd0 = add_random_ineffective_brand(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; useless brand on gloves;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("melee; O-combat; useless brand on gloves;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_temporary_brand(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	wipe_brands_slays(gloves);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[timed_effects[iadd0].temp_brand] > oi_avg);
	oi_avg = ts->oi_brands[timed_effects[iadd0].temp_brand];
	check_averages("melee; useful temporary brand;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[timed_effects[iadd0].temp_brand] > oi_avg);
	oi_avg = ts->oi_brands[timed_effects[iadd0].temp_brand];
	check_averages("melee; O-combat; useful temporary brand;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	remove_all_temporary_brands_and_slays(player);
	iadd0 = add_random_ineffective_temporary_brand(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[timed_effects[iadd0].temp_brand] > oi_avg);
	check_averages("melee; useless temporary brand;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_brands[timed_effects[iadd0].temp_brand] > oi_avg);
	check_averages("melee; O-combat; useless temporary brand;", oi_avg,
		mc_avg, mc_var);

	/* Check armed combat with an off-weapon slay. */
	OPT(player, birth_percent_damage) = false;
	remove_all_temporary_brands_and_slays(player);
	iadd0 = add_random_effective_slay(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; useful slay on gloves;", ts->oi_slays[iadd0],
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; O-combat; useful slay on gloves;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(gloves);
	iadd0 = add_random_ineffective_slay(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; useless slay on gloves;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("melee; O-combat; useless slay on gloves;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(gloves);
	iadd0 = add_random_effective_temporary_slay(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[timed_effects[iadd0].temp_slay] > oi_avg);
	oi_avg = ts->oi_slays[timed_effects[iadd0].temp_slay];
	check_averages("melee; useful temporary slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[timed_effects[iadd0].temp_slay] > oi_avg);
	oi_avg = ts->oi_slays[timed_effects[iadd0].temp_slay];
	check_averages("melee; O-combat; useful temporary slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	remove_all_temporary_brands_and_slays(player);
	iadd0 = add_random_ineffective_temporary_slay(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[timed_effects[iadd0].temp_slay] > oi_avg);
	check_averages("melee; useless temporary slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, false);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, true);
	require(ts->oi_slays[timed_effects[iadd0].temp_slay] > oi_avg);
	check_averages("melee; O-combat; useless temporary slay;", oi_avg,
		mc_avg, mc_var);

	/* Take off the gloves. */
	player->body.slots[gloves_slot].obj = old_gloves;
	if (!old_gloves) {
		assert(player->upkeep->equip_cnt > 0);
		--player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight +=
		((old_gloves) ? old_gloves->weight : 0) - gloves->weight;
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	if (gloves->known) {
		object_free(gloves->known);
		gloves->known = NULL;
	}
	object_free(gloves);
	if (weapon->known) {
		object_free(weapon->known);
		weapon->known = NULL;
	}
	object_free(weapon);
	remove_all_temporary_brands_and_slays(player);

	ok;
}

static int test_launched_weapon_damage_info(void *state)
{
	struct info_test_state *ts = (struct info_test_state*)state;
	struct object_kind *launcher_kind, *ammo_kind, *gloves_kind;
	struct object *launcher, *old_launcher, *ammo, *gloves, *old_gloves;
	double mc_avg, mc_var;
	int launcher_slot, gloves_slot, ammo_tval, oi_avg, iadd0, iadd1;
	bool has_brand_slay, has_nonweap;

	remove_all_temporary_brands_and_slays(player);

	/* Set up the launcher. */
	launcher_kind = get_obj_num(1, false, TV_BOW);
	if (!launcher_kind) {
		/*
		 * The depth-dependent lookup did not work so try using the
		 * first kind.
		 */
		launcher_kind = lookup_kind(TV_BOW, 1);
	}
	notnull(launcher_kind);
	launcher = object_new();
	object_prep(launcher, launcher_kind, 1, MINIMISE);
	/* Give it a damage bonus and extra shots. */
	launcher->to_d = 2;
	launcher->modifiers[OBJ_MOD_SHOTS] = 4;
	/* Make sure the player knows its properties. */
	launcher->known = object_new();
	wipe_brands_slays(launcher);
	object_set_base_known(player, launcher);
	object_touch(player, launcher);
	object_learn_on_wield(player, launcher);
	/* Wield it. */
	launcher_slot = wield_slot(launcher);
	old_launcher = player->body.slots[launcher_slot].obj;
	player->body.slots[launcher_slot].obj = launcher;
	if (!old_launcher) {
		++player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight += launcher->weight
		- ((old_launcher) ? old_launcher->weight : 0);
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	/* Set up a piece of ammo. */
	if (kf_has(launcher->kind->kind_flags, KF_SHOOTS_SHOTS)) {
		ammo_tval = TV_SHOT;
	} else if (kf_has(launcher->kind->kind_flags, KF_SHOOTS_ARROWS)) {
		ammo_tval = TV_ARROW;
	} else if (kf_has(launcher->kind->kind_flags, KF_SHOOTS_BOLTS)) {
		ammo_tval = TV_BOLT;
	} else {
		ammo_tval = 0;
	}
	require(ammo_tval > 0);
	ammo_kind = get_obj_num(1, false, ammo_tval);
	if (!ammo_kind) {
		/*
		 * The depth-dependent lookup did not work so try using the
		 * first kind.
		 */
		ammo_kind = lookup_kind(ammo_tval, 1);
	}
	notnull(ammo_kind);
	ammo = object_new();
	object_prep(ammo, ammo_kind, 1, MINIMISE);
	/* Give it a damage bonus and multiple dice of damage. */
	ammo->to_d = 3;
	ammo->dd = 2;
	ammo->ds = 4;
	/* Make sure the player knows its properties. */
	ammo->known = object_new();
	wipe_brands_slays(ammo);
	object_set_base_known(player, ammo);
	object_touch(player, ammo);
	object_learn_on_wield(player, ammo);

	gloves_kind = get_obj_num(1, false, TV_GLOVES);
	if (!gloves_kind) {
		/*
		 * The depth-dependent lookup did not work so try using the
		 * first kind.
		 */
		gloves_kind = lookup_kind(TV_GLOVES, 1);
	}
	notnull(gloves_kind);
	gloves = object_new();
	object_prep(gloves, gloves_kind, 1, MINIMISE);
	/* Make sure the player knows its properties. */
	gloves->known = object_new();
	wipe_brands_slays(gloves);
	object_set_base_known(player, gloves);
	object_touch(player, gloves);
	object_learn_on_wield(player, gloves);
	/* Wield them. */
	gloves_slot = wield_slot(gloves);
	old_gloves = player->body.slots[gloves_slot].obj;
	player->body.slots[gloves_slot].obj = gloves;
	if (!old_gloves) {
		++player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight += gloves->weight
		- ((old_gloves) ? old_gloves->weight : 0);
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	/*
	 * Check with a missile and launcher that do not have any slays or
	 * brands.
	 */
	OPT(player, birth_percent_damage) = false;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; no brand or slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; O-combat; no brand or slay;", oi_avg, mc_avg,
		mc_var);

	/* Check with a missile that has a brand and nothing on the launcher. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; ammo has one useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has one useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	iadd0 = add_random_ineffective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; ammo has one useless brand;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has one useless brand;",
		oi_avg, mc_avg, mc_var);

	/* Check with a launcher that has a brand and nothing on the missile. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	iadd0 = add_random_effective_brand(player, launcher, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; launcher has one useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; O-combat; launcher has one useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_brand(player, launcher, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; launcher has one useless brand;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; O-combat; launcher has one useless brand;",
		oi_avg, mc_avg, mc_var);

	/* Check with a missile that has a slay and nothing on the launcher. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; ammo has one useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has one useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	iadd0 = add_random_ineffective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; ammo has one useless slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has one useless slay;",
		oi_avg, mc_avg, mc_var);

	/* Check with a launcher that has a slay and nothing on the missile. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	iadd0 = add_random_effective_slay(player, launcher, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	check_averages("launched; launcher has one useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; O-combat; launcher has one useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_slay(player, launcher, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; launcher has one useless slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; O-combat; launcher has one useless slay;",
		oi_avg, mc_avg, mc_var);

	/* Check with a missile and launcher that have slays or brands. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_brand(player, launcher, ts->target);
	iadd1 = add_random_effective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (is_similar_brand(iadd0, iadd1)) {
		if (brands[iadd0].multiplier > brands[iadd1].multiplier) {
			require(ts->oi_brands[iadd0] > oi_avg);
		} else if (brands[iadd0].multiplier
				< brands[iadd1].multiplier) {
			require(ts->oi_brands[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1
				&& ts->oi_brands[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_brands[iadd0] > oi_avg
			&& ts->oi_brands[iadd1] > oi_avg);
	}
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_brands[iadd1];
	check_averages("launched; ammo and launcher have useful brands;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (is_similar_brand(iadd0, iadd1)) {
		if (brands[iadd0].o_multiplier > brands[iadd1].o_multiplier) {
			require(ts->oi_brands[iadd0] > oi_avg);
		} else if (brands[iadd0].o_multiplier
				< brands[iadd1].o_multiplier) {
			require(ts->oi_brands[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1
				&& ts->oi_brands[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_brands[iadd0] > oi_avg
			&& ts->oi_brands[iadd1] > oi_avg);
	}
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_brands[iadd1];
	check_averages("launched; O-combat; ammo and launcher have useful brand",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_brand(player, launcher, ts->target);
	iadd1 = add_random_effective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_slays[iadd1];
	check_averages("launched; ammo has useful slay; launcher has useful brand;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_slays[iadd1];
	check_averages("launched; O-combat; ammo has useful slay; launcher has useful brand;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_slay(player, launcher, ts->target);
	iadd1 = add_random_effective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_brands[iadd1];
	check_averages("launched; ammo has useful brand; launcher has useful slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_brands[iadd1];
	check_averages("launched; O-combat; ammo has useful brand; launcher has useful slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_slay(player, launcher, ts->target);
	iadd1 = add_random_effective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (same_monsters_slain(iadd0, iadd1)) {
		if (slays[iadd0].multiplier > slays[iadd1].multiplier) {
			require(ts->oi_slays[iadd0] > oi_avg);
		} else if (slays[iadd0].multiplier
				< slays[iadd1].multiplier) {
			require(ts->oi_slays[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1 && ts->oi_slays[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_slays[iadd0] > oi_avg
			&& ts->oi_slays[iadd1] > oi_avg);
	}
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_slays[iadd1];
	check_averages("launched; ammo and launcher have useful slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (same_monsters_slain(iadd0, iadd1)) {
		if (slays[iadd0].o_multiplier > slays[iadd1].o_multiplier) {
			require(ts->oi_slays[iadd0] > oi_avg);
		} else if (slays[iadd0].o_multiplier
				< slays[iadd1].o_multiplier) {
			require(ts->oi_slays[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1 && ts->oi_slays[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_slays[iadd0] > oi_avg
			&& ts->oi_slays[iadd1] > oi_avg);
	}
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_slays[iadd1];
	check_averages("launched; O-combat; ammo and launcher have useful slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_brand(player, launcher, ts->target);
	iadd1 = add_random_ineffective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; ammo has useless brand; launcher has useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has useless brand; launcher has useful brand",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_brand(player, launcher, ts->target);
	iadd1 = add_random_effective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd1] > oi_avg);
	check_averages("launched; ammo has useful brand; launcher has useless brand;",
		ts->oi_brands[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd1] > oi_avg);
	check_averages("launched; O-combat; ammo has useful brand; launcher has useless brand",
		ts->oi_brands[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_brand(player, launcher, ts->target);
	iadd1 = add_random_ineffective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; ammo has useless slay; launcher has useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has useless slay; launcher has useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_brand(player, launcher, ts->target);
	iadd1 = add_random_effective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd1] > oi_avg);
	check_averages("launched; ammo has useful slay; launcher has useless brand;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd1] > oi_avg);
	check_averages("launched; O-combat; ammo has useful slay; launcher has useless brand;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_slay(player, launcher, ts->target);
	iadd1 = add_random_effective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd1] > oi_avg);
	check_averages("launched; ammo has useful brand; launcher has useless slay;",
		ts->oi_brands[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd1] > oi_avg);
	check_averages("launched; O-combat; ammo has useful brand; launcher has useless slay;",
		ts->oi_brands[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_slay(player, launcher, ts->target);
	iadd1 = add_random_ineffective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; ammo has useless brand; launcher has useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has useless brand; launcher has useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_slay(player, launcher, ts->target);
	iadd1 = add_random_effective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd1] > oi_avg);
	check_averages("launched; ammo has useful slay; launcher has useless slay;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd1] > oi_avg);
	check_averages("launched; O-combat; ammo has useful slay; launcher has useless slay;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_effective_slay(player, launcher, ts->target);
	iadd1 = add_random_ineffective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; ammo has useless slay; launcher has useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("launched; O-combat; ammo has useless slay; launcher has ful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(launcher);
	wipe_brands_slays(ammo);
	iadd0 = add_random_ineffective_brand(player, launcher, ts->target);
	iadd1 = add_random_ineffective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (is_similar_brand(iadd0, iadd1)) {
		if (brands[iadd0].multiplier > brands[iadd1].multiplier) {
			require(ts->oi_brands[iadd0] > oi_avg);
		} else if (brands[iadd0].multiplier
				< brands[iadd1].multiplier) {
			require(ts->oi_brands[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1
				&& ts->oi_brands[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_brands[iadd0] > oi_avg
			&& ts->oi_brands[iadd1] > oi_avg);
	}
	check_averages("launched; ammo and launcher have useless brands;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (is_similar_brand(iadd0, iadd1)) {
		if (brands[iadd0].o_multiplier > brands[iadd1].o_multiplier) {
			require(ts->oi_brands[iadd0] > oi_avg);
		} else if (brands[iadd0].o_multiplier
				< brands[iadd1].o_multiplier) {
			require(ts->oi_brands[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1
				&& ts->oi_brands[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_brands[iadd0] > oi_avg
			&& ts->oi_brands[iadd1] > oi_avg);
	}
	check_averages("launched; O-combat; ammo and launcher have useless brand",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_brand(player, launcher, ts->target);
	iadd1 = add_random_ineffective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("launched; ammo has useless slay; launcher has useless brand;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("launched; O-combat; ammo has useless slay; launcher has useless brand;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_slay(player, launcher, ts->target);
	iadd1 = add_random_ineffective_brand(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("launched; ammo has useless brand; launcher has useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("launched; O-combat; ammo has useless brand; launcher has useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	iadd0 = add_random_ineffective_slay(player, launcher, ts->target);
	iadd1 = add_random_ineffective_slay(player, ammo, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (same_monsters_slain(iadd0, iadd1)) {
		if (slays[iadd0].multiplier > slays[iadd1].multiplier) {
			require(ts->oi_slays[iadd0] > oi_avg);
		} else if (slays[iadd0].multiplier
				< slays[iadd1].multiplier) {
			require(ts->oi_slays[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1 && ts->oi_slays[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_slays[iadd0] > oi_avg
			&& ts->oi_slays[iadd1] > oi_avg);
	}
	check_averages("launched; ammo and launcher have useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	if (same_monsters_slain(iadd0, iadd1)) {
		if (slays[iadd0].o_multiplier > slays[iadd1].o_multiplier) {
			require(ts->oi_slays[iadd0] > oi_avg);
		} else if (slays[iadd0].o_multiplier
				< slays[iadd1].o_multiplier) {
			require(ts->oi_slays[iadd1] > oi_avg);
		} else {
			require(iadd0 == iadd1 && ts->oi_slays[iadd0] > oi_avg);
		}
	} else {
		require(ts->oi_slays[iadd0] > oi_avg
			&& ts->oi_slays[iadd1] > oi_avg);
	}
	check_averages("launched; O-combat; ammo and launcher have useless slay;",
		oi_avg, mc_avg, mc_var);

	/* Check with an off-weapon slay or brand.  Should have no effect. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_brand(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	wipe_brands_slays(ammo);
	wipe_brands_slays(launcher);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; useful brand on gloves;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; O-combat; useful brand on gloves;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_temporary_brand(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	wipe_brands_slays(gloves);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; temporary brand;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; O-combat; temporary brand;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	remove_all_temporary_brands_and_slays(player);
	iadd0 = add_random_effective_slay(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; useful slay on gloves;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; O-combat; useful slay on gloves;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(gloves);
	iadd0 = add_random_effective_temporary_slay(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; temporary slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, ammo, launcher, false);
	has_brand_slay = o_obj_known_damage(ammo, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, false);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("launched; O-combat; temporary slay;", oi_avg, mc_avg,
		mc_var);

	/* Take off the gloves. */
	player->body.slots[gloves_slot].obj = old_gloves;
	if (!old_gloves) {
		assert(player->upkeep->equip_cnt > 0);
		--player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight +=
		((old_gloves) ? old_gloves->weight : 0) - gloves->weight;
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	/* Take off the launcher. */
	player->body.slots[launcher_slot].obj = old_launcher;
	if (!old_launcher) {
		assert(player->upkeep->equip_cnt > 0);
		--player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight +=
		((old_launcher) ? old_launcher->weight : 0) - launcher->weight;
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	if (gloves->known) {
		object_free(gloves->known);
		gloves->known = NULL;
	}
	object_free(gloves);
	if (ammo->known) {
		object_free(ammo->known);
	}
	object_free(ammo);
	if (launcher->known) {
		object_free(launcher->known);
		launcher->known = NULL;
	}
	object_free(launcher);
	remove_all_temporary_brands_and_slays(player);

	ok;
}

static int test_thrown_weapon_damage_info(void *state)
{
	struct info_test_state *ts = (struct info_test_state*)state;
	struct object_kind *weapon_kind, *gloves_kind;
	struct object *weapon, *gloves, *old_gloves;
	double mc_avg, mc_var;
	int gloves_slot, oi_avg, iadd0, iadd1;
	bool has_brand_slay, has_nonweap;

	remove_all_temporary_brands_and_slays(player);
	weapon_kind = lookup_kind(TV_SWORD, lookup_sval(TV_SWORD, "dagger"));
	notnull(weapon_kind);
	weapon = object_new();
	object_prep(weapon, weapon_kind, 1, MINIMISE);
	/* Make sure the player knows its properties. */
	weapon->known = object_new();
	wipe_brands_slays(weapon);
	object_set_base_known(player, weapon);
	object_touch(player, weapon);
	object_learn_on_wield(player, weapon);

	gloves_kind = get_obj_num(1, false, TV_GLOVES);
	if (!gloves_kind) {
		/*
		 * The depth-dependent lookup did not work so try using the
		 * first kind.
		 */
		gloves_kind = lookup_kind(TV_GLOVES, 1);
	}
	notnull(gloves_kind);
	gloves = object_new();
	object_prep(gloves, gloves_kind, 1, MINIMISE);
	/* Make sure the player knows its properties. */
	gloves->known = object_new();
	wipe_brands_slays(gloves);
	object_set_base_known(player, gloves);
	object_touch(player, gloves);
	object_learn_on_wield(player, gloves);
	/* Wield them. */
	gloves_slot = wield_slot(gloves);
	old_gloves = player->body.slots[gloves_slot].obj;
	player->body.slots[gloves_slot].obj = gloves;
	if (!old_gloves) {
		++player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight += gloves->weight
		- ((old_gloves) ? old_gloves->weight : 0);
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	/* Check with no brand or slay on the thrown weapon. */
	OPT(player, birth_percent_damage) = false;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; no brand or slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; O-combat; no brand or slay;", oi_avg, mc_avg,
		mc_var);

	/* Check with a brand on the thrown weapon. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("thrown; one useful brand;", ts->oi_brands[iadd0],
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("thrown; O-combat; one useful brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("thrown; one useless brand;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg);
	check_averages("thrown; O-combat; one useless brand;", oi_avg, mc_avg,
		mc_var);

	/* Check with a slay on the thrown weapon. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("thrown; one useful slay;", ts->oi_slays[iadd0],
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("thrown; O-combat; one useful slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("thrown; one useless slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg);
	check_averages("thrown; O-combat; one useless slay;", oi_avg, mc_avg,
		mc_var);

	/* Check with more than one brand or slay on the weapon. */
	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_effective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_brands[iadd1];
	check_averages("thrown; two useful brands;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_brands[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_brands[iadd1];
	check_averages("thrown; O-combat; two useful brands;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_slays[iadd1];
	check_averages("thrown; one useful brand; one useful slay;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	oi_avg = (ts->oi_brands[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_brands[iadd0] : ts->oi_slays[iadd1];
	check_averages("thrown; O-combat; one useful brand; one useful slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_slay(player, weapon, ts->target);
	iadd1 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	/* Whichever is better should take effect. */
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_slays[iadd1];
	check_averages("thrown; two useful slays;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	oi_avg = (ts->oi_slays[iadd0] > ts->oi_slays[iadd1])
		? ts->oi_slays[iadd0] : ts->oi_slays[iadd1];
	check_averages("thrown; O-combat; two useful slays;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("thrown; one useful brand; one useless brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("thrown; O-combat; one useful brand; one useless brand;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; one useful brand; one useless slay;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; O-combat; one useful brand; one useless slay;",
		ts->oi_brands[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	iadd1 = add_random_effective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	check_averages("thrown; one useless brand; one useful slay;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; O-combat; one useless brand; one useful slay;",
		ts->oi_slays[iadd1], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_effective_slay(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; one useful slay; one useless slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; O-combat; one useful slay; one useless slay;",
		ts->oi_slays[iadd0], mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_brand(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("thrown; two useless brands;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_brands[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_brands[iadd1] > oi_avg);
	check_averages("thrown; O-combat; two useless brands;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_brand(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; one useless brand; one useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_brands[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; O-combat; one useless brand; one useless slay;",
		oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	wipe_brands_slays(weapon);
	iadd0 = add_random_ineffective_slay(player, weapon, ts->target);
	iadd1 = add_random_ineffective_slay(player, weapon, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	require(iadd1 > 0 && iadd1 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; two useless slays;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	ts->oi_slays[iadd1] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, true);
	eq(has_nonweap, false);
	require(ts->oi_slays[iadd0] > oi_avg && ts->oi_slays[iadd1] > oi_avg);
	check_averages("thrown; O-combat; one useless slays;",
		oi_avg, mc_avg, mc_var);

	/* Check with an off-weapon slay or brand.  Should have no effect. */
	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_brand(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->brand_max);
	ts->oi_brands[iadd0] = 0;
	wipe_brands_slays(weapon);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; useful brand on gloves;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; O-combat; useful brand on gloves;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	iadd0 = add_random_effective_temporary_brand(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	wipe_brands_slays(gloves);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; temporary brand;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_brands[timed_effects[iadd0].temp_brand] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; O-combat; temporary brand;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = false;
	remove_all_temporary_brands_and_slays(player);
	iadd0 = add_random_effective_slay(player, gloves, ts->target);
	require(iadd0 > 0 && iadd0 < z_info->slay_max);
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; useful slay on gloves;", oi_avg, mc_avg,
		mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[iadd0] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; O-combat; useful slay on gloves;", oi_avg,
		mc_avg, mc_var);

	OPT(player, birth_percent_damage) = false;
	remove_all_temporary_brands_and_slays(player);
	iadd0 = add_random_effective_temporary_slay(player, ts->target);
	require(iadd0 >= 0 && iadd0 < TMD_MAX);
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	wipe_brands_slays(gloves);
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; temporary slay;", oi_avg, mc_avg, mc_var);

	OPT(player, birth_percent_damage) = true;
	ts->oi_slays[timed_effects[iadd0].temp_slay] = 0;
	collect_damage_results(&mc_avg, &mc_var, ts->damage_results, player,
		ts->target, weapon, NULL, true);
	has_brand_slay = o_obj_known_damage(weapon, &oi_avg, ts->oi_brands,
		ts->oi_slays, &has_nonweap, true);
	eq(has_brand_slay, false);
	eq(has_nonweap, false);
	check_averages("thrown; O-combat; temporary slay;", oi_avg, mc_avg,
		mc_var);

	/* Take off the gloves. */
	player->body.slots[gloves_slot].obj = old_gloves;
	if (!old_gloves) {
		assert(player->upkeep->equip_cnt > 0);
		--player->upkeep->equip_cnt;
	}
	player->upkeep->total_weight +=
		((old_gloves) ? old_gloves->weight : 0) - gloves->weight;
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player);

	if (gloves->known) {
		object_free(gloves->known);
		gloves->known = NULL;
	}
	object_free(gloves);
	if (weapon->known) {
		object_free(weapon->known);
		weapon->known = NULL;
	}
	object_free(weapon);
	remove_all_temporary_brands_and_slays(player);

	ok;
}

const char *suite_name = "object/info";
struct test tests[] = {
	{ "melee weapon damage info", test_melee_weapon_damage_info },
	{ "launched weapon damage info", test_launched_weapon_damage_info },
	{ "thrown weapon damage info", test_thrown_weapon_damage_info },
	{ NULL, NULL }
};
