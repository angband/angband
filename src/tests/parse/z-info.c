/* parse/z-info */
/* Exercise parsing used for constants.txt. */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "message.h"


int setup_tests(void **state) {
	*state = constants_parser.init();
	return !*state;
}

int teardown_tests(void *state) {
	struct angband_constants *z = parser_priv(state);
	z_info = z;
	constants_parser.cleanup();
	parser_destroy(state);
	return 0;
}

static int test_negative(void *state) {
	struct parser *p = (struct parser*) state;
	errr r = parser_parse(p, "level-max:monsters:-1");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "mon-gen:chance:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "mon-play:mult-rate:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "dun-gen:wall-max:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "world:dungeon-hgt:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "carry-cap:pack-size:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "store:shuffle:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "obj-make:great-obj:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "player:max-sight:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_baddirective(void *state) {
	struct parser *p = (struct parser*) state;
	errr r = parser_parse(p, "level-max:D:1");

	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "mon-gen:xyzzy:5");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "mon-play:xyzzy:10");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "dun-gen:xyzzy:3");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "world:xyzzy:170");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "carry-cap:xyzzy:40");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "store:xyzzy:20");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "obj-make:xyzzy:5000");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "player:xyzzy:300");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "melee-critical:xyzzy:300");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "ranged-critical:xyzzy:300");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "o-melee-critical:xyzzy:300");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	r = parser_parse(p, "o-ranged-critical:xyzzy:300");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

#define TEST_CONSTANT(l,u,section) \
	static int test_##l(void *s) { \
		struct angband_constants *m = parser_priv(s); \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "%s:%s:%d", section, u, __LINE__); \
		r = parser_parse(s, buf); \
		eq(m->l, __LINE__); \
		eq(r, 0); \
		ok; \
	}

TEST_CONSTANT(level_monster_max, "monsters", "level-max")

TEST_CONSTANT(alloc_monster_chance, "chance", "mon-gen")
TEST_CONSTANT(level_monster_min, "level-min", "mon-gen")
TEST_CONSTANT(town_monsters_day, "town-day", "mon-gen")
TEST_CONSTANT(town_monsters_night, "town-night", "mon-gen")
TEST_CONSTANT(repro_monster_max, "repro-max", "mon-gen")
TEST_CONSTANT(ood_monster_chance, "ood-chance", "mon-gen")
TEST_CONSTANT(ood_monster_amount, "ood-amount", "mon-gen")
TEST_CONSTANT(monster_group_max, "group-max", "mon-gen")
TEST_CONSTANT(monster_group_dist, "group-dist", "mon-gen")

TEST_CONSTANT(glyph_hardness, "break-glyph", "mon-play")
TEST_CONSTANT(repro_monster_rate, "mult-rate", "mon-play")
TEST_CONSTANT(life_drain_percent, "life-drain", "mon-play")
TEST_CONSTANT(flee_range, "flee-range", "mon-play")
TEST_CONSTANT(turn_range, "turn-range", "mon-play")

TEST_CONSTANT(level_room_max, "cent-max", "dun-gen")
TEST_CONSTANT(level_door_max, "door-max", "dun-gen")
TEST_CONSTANT(wall_pierce_max, "wall-max", "dun-gen")
TEST_CONSTANT(tunn_grid_max, "tunn-max", "dun-gen")
TEST_CONSTANT(room_item_av, "amt-room", "dun-gen")
TEST_CONSTANT(both_item_av, "amt-item", "dun-gen")
TEST_CONSTANT(both_gold_av, "amt-gold", "dun-gen")
TEST_CONSTANT(level_pit_max, "pit-max", "dun-gen")

TEST_CONSTANT(max_depth, "max-depth", "world")
TEST_CONSTANT(day_length, "day-length", "world")
TEST_CONSTANT(dungeon_hgt, "dungeon-hgt", "world")
TEST_CONSTANT(dungeon_wid, "dungeon-wid", "world")
TEST_CONSTANT(town_hgt, "town-hgt", "world")
TEST_CONSTANT(town_wid, "town-wid", "world")
TEST_CONSTANT(feeling_total, "feeling-total", "world")
TEST_CONSTANT(feeling_need, "feeling-need", "world")
TEST_CONSTANT(stair_skip, "stair-skip", "world")
TEST_CONSTANT(move_energy, "move-energy", "world")

TEST_CONSTANT(pack_size, "pack-size", "carry-cap")
TEST_CONSTANT(quiver_size, "quiver-size", "carry-cap")
TEST_CONSTANT(quiver_slot_size, "quiver-slot-size", "carry-cap")
TEST_CONSTANT(thrown_quiver_mult, "thrown-quiver-mult", "carry-cap")
TEST_CONSTANT(floor_size, "floor-size", "carry-cap")

TEST_CONSTANT(store_inven_max, "inven-max", "store")
TEST_CONSTANT(store_turns, "turns", "store")
TEST_CONSTANT(store_shuffle, "shuffle", "store")
TEST_CONSTANT(store_magic_level, "magic-level", "store")

TEST_CONSTANT(max_obj_depth, "max-depth", "obj-make")
TEST_CONSTANT(great_obj, "great-obj", "obj-make")
TEST_CONSTANT(great_ego, "great-ego", "obj-make")
TEST_CONSTANT(fuel_torch, "fuel-torch", "obj-make")
TEST_CONSTANT(fuel_lamp, "fuel-lamp", "obj-make")
TEST_CONSTANT(default_lamp, "default-lamp", "obj-make")

TEST_CONSTANT(max_sight, "max-sight", "player")
TEST_CONSTANT(max_range, "max-range", "player")
TEST_CONSTANT(start_gold, "start-gold", "player")
TEST_CONSTANT(food_value, "food-value", "player")

TEST_CONSTANT(m_crit_debuff_toh, "debuff-toh", "melee-critical")
TEST_CONSTANT(m_crit_chance_weight_scl, "chance-weight-scale", "melee-critical")
TEST_CONSTANT(m_crit_chance_toh_scl, "chance-toh-scale", "melee-critical")
TEST_CONSTANT(m_crit_chance_level_scl, "chance-level-scale", "melee-critical")
TEST_CONSTANT(m_crit_chance_toh_skill_scl, "chance-toh-skill-scale", "melee-critical")
TEST_CONSTANT(m_crit_chance_offset, "chance-offset", "melee-critical")
TEST_CONSTANT(m_crit_chance_range, "chance-range", "melee-critical")
TEST_CONSTANT(m_crit_power_weight_scl, "power-weight-scale", "melee-critical")
TEST_CONSTANT(m_crit_power_random, "power-random", "melee-critical")

TEST_CONSTANT(r_crit_debuff_toh, "debuff-toh", "ranged-critical")
TEST_CONSTANT(r_crit_chance_weight_scl, "chance-weight-scale", "ranged-critical")
TEST_CONSTANT(r_crit_chance_toh_scl, "chance-toh-scale", "ranged-critical")
TEST_CONSTANT(r_crit_chance_level_scl, "chance-level-scale", "ranged-critical")
TEST_CONSTANT(r_crit_chance_launched_toh_skill_scl, "chance-launched-toh-skill-scale", "ranged-critical")
TEST_CONSTANT(r_crit_chance_thrown_toh_skill_scl, "chance-thrown-toh-skill-scale", "ranged-critical")
TEST_CONSTANT(r_crit_chance_offset, "chance-offset", "ranged-critical")
TEST_CONSTANT(r_crit_chance_range, "chance-range", "ranged-critical")
TEST_CONSTANT(r_crit_power_weight_scl, "power-weight-scale", "ranged-critical")
TEST_CONSTANT(r_crit_power_random, "power-random", "ranged-critical")

TEST_CONSTANT(o_m_crit_debuff_toh, "debuff-toh", "o-melee-critical")
TEST_CONSTANT(o_m_crit_power_toh_scl_num, "power-toh-scale-numerator", "o-melee-critical")
TEST_CONSTANT(o_m_crit_power_toh_scl_den, "power-toh-scale-denominator", "o-melee-critical")
TEST_CONSTANT(o_m_crit_chance_power_scl_num, "chance-power-scale-numerator", "o-melee-critical")
TEST_CONSTANT(o_m_crit_chance_power_scl_den, "chance-power-scale-denominator", "o-melee-critical")
TEST_CONSTANT(o_m_crit_chance_add_den, "chance-add-denominator", "o-melee-critical")

TEST_CONSTANT(o_r_crit_debuff_toh, "debuff-toh", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_power_launched_toh_scl_num, "power-launched-toh-scale-numerator", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_power_launched_toh_scl_den, "power-launched-toh-scale-denominator", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_power_thrown_toh_scl_num, "power-thrown-toh-scale-numerator", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_power_thrown_toh_scl_den, "power-thrown-toh-scale-denominator", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_chance_power_scl_num, "chance-power-scale-numerator", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_chance_power_scl_den, "chance-power-scale-denominator", "o-ranged-critical")
TEST_CONSTANT(o_r_crit_chance_add_den, "chance-add-denominator", "o-ranged-critical")

static int test_bad_m_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r;

	null(m->m_crit_level_head);
	r = parser_parse(p, "melee-critical-level:400:2:5:XYZZY");
	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	null(m->m_crit_level_head);
	ok;
}

static int test_m_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r = parser_parse(p,
		"melee-critical-level:400:2:5:HIT_GOOD");
	const struct critical_level *this_l, *last_l;

	eq(r, PARSE_ERROR_NONE);
	notnull(m->m_crit_level_head);
	for (this_l = m->m_crit_level_head;
			this_l->next;
			this_l = this_l->next) {}
	eq(this_l->cutoff, 400);
	eq(this_l->mult, 2);
	eq(this_l->add, 5);
	eq(this_l->msgt, MSG_HIT_GOOD);

	r = parser_parse(p, "melee-critical-level:900:3:15:HIT_SUPERB");
	eq(r, PARSE_ERROR_NONE);
	notnull(m->m_crit_level_head);
	for (last_l = NULL, this_l = m->m_crit_level_head;
			this_l->next;
			last_l = this_l, this_l = this_l->next) {}
	notnull(last_l);
	eq(last_l->cutoff, 400);
	eq(last_l->mult, 2);
	eq(last_l->add, 5);
	eq(last_l->msgt, MSG_HIT_GOOD);
	eq(this_l->cutoff, 900);
	eq(this_l->mult, 3);
	eq(this_l->add, 15);
	eq(this_l->msgt, MSG_HIT_SUPERB);

	ok;
}

static int test_bad_r_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r;

	null(m->r_crit_level_head);
	r = parser_parse(p, "ranged-critical-level:500:2:5:XYZZY");
	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	null(m->r_crit_level_head);
	ok;
}

static int test_r_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r = parser_parse(p,
		"ranged-critical-level:500:2:5:HIT_GOOD");
	const struct critical_level *this_l, *last_l;

	eq(r, PARSE_ERROR_NONE);
	notnull(m->r_crit_level_head);
	for (this_l = m->r_crit_level_head;
			this_l->next;
			this_l = this_l->next) {}
	eq(this_l->cutoff, 500);
	eq(this_l->mult, 2);
	eq(this_l->add, 5);
	eq(this_l->msgt, MSG_HIT_GOOD);

	r = parser_parse(p, "ranged-critical-level:-1:4:20:HIT_SUPERB");
	eq(r, PARSE_ERROR_NONE);
	notnull(m->r_crit_level_head);
	for (last_l = NULL, this_l = m->r_crit_level_head;
			this_l->next;
			last_l = this_l, this_l = this_l->next) {}
	notnull(last_l);
	eq(last_l->cutoff, 500);
	eq(last_l->mult, 2);
	eq(last_l->add, 5);
	eq(last_l->msgt, MSG_HIT_GOOD);
	eq(this_l->cutoff, -1);
	eq(this_l->mult, 4);
	eq(this_l->add, 20);
	eq(this_l->msgt, MSG_HIT_SUPERB);

	ok;
}

static int test_bad_o_m_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r;

	null(m->o_m_crit_level_head);
	/* Check invalid chance. */
	r = parser_parse(p, "o-melee-critical-level:0:5:HIT_HI_SUPERB");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	null(m->o_m_crit_level_head);
	/* Check invalid message. */
	r = parser_parse(p, "o-melee-critical-level:40:5:XYZZY");
	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	null(m->o_m_crit_level_head);
	/* Check invalid chance and invalid message. */
	r = parser_parse(p, "o-melee-critical-level:0:5:XYZZY");
	noteq(r, PARSE_ERROR_NONE);
	null(m->o_m_crit_level_head);
	ok;
}

static int test_o_m_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r = parser_parse(p,
		"o-melee-critical-level:40:5:HIT_HI_SUPERB");
	const struct o_critical_level *this_l, *last_l;

	eq(r, PARSE_ERROR_NONE);
	notnull(m->o_m_crit_level_head);
	for (this_l = m->o_m_crit_level_head;
			this_l->next;
			this_l = this_l->next) {}
	eq(this_l->chance, 40);
	eq(this_l->added_dice, 5);
	eq(this_l->msgt, MSG_HIT_HI_SUPERB);

	r = parser_parse(p, "o-melee-critical-level:12:4:HIT_HI_GREAT");
	eq(r, PARSE_ERROR_NONE);
	notnull(m->o_m_crit_level_head);
	for (last_l = NULL, this_l = m->o_m_crit_level_head;
			this_l->next;
			last_l = this_l, this_l = this_l->next) {}
	notnull(last_l);
	eq(last_l->chance, 40);
	eq(last_l->added_dice, 5);
	eq(last_l->msgt, MSG_HIT_HI_SUPERB);
	eq(this_l->chance, 12);
	eq(this_l->added_dice, 4);
	eq(this_l->msgt, MSG_HIT_HI_GREAT);

	ok;
}

static int test_bad_o_r_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r;

	null(m->o_r_crit_level_head);
	/* Check invalid chance. */
	r = parser_parse(p, "o-ranged-critical-level:0:3:HIT_SUPERB");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	null(m->o_r_crit_level_head);
	/* Check invalid message. */
	r = parser_parse(p, "o-ranged-critical-level:50:3:XYZZY");
	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	null(m->o_m_crit_level_head);
	/* Check invalid chance and invalid message. */
	r = parser_parse(p, "o-ranged-critical-level:0:3:XYZZY");
	noteq(r, PARSE_ERROR_NONE);
	null(m->o_r_crit_level_head);
	ok;
}

static int test_o_r_crit_level(void *s) {
	struct parser *p = (struct parser*)s;
	struct angband_constants *m = parser_priv(p);
	enum parser_error r = parser_parse(p,
		"o-ranged-critical-level:50:3:HIT_SUPERB");
	const struct o_critical_level *this_l, *last_l;

	eq(r, PARSE_ERROR_NONE);
	notnull(m->o_r_crit_level_head);
	for (this_l = m->o_r_crit_level_head;
			this_l->next;
			this_l = this_l->next) {}
	eq(this_l->chance, 50);
	eq(this_l->added_dice, 3);
	eq(this_l->msgt, MSG_HIT_SUPERB);

	r = parser_parse(p, "o-ranged-critical-level:10:2:HIT_GREAT");
	eq(r, PARSE_ERROR_NONE);
	notnull(m->o_r_crit_level_head);
	for (last_l = NULL, this_l = m->o_r_crit_level_head;
			this_l->next;
			last_l = this_l, this_l = this_l->next) {}
	notnull(last_l);
	eq(last_l->chance, 50);
	eq(last_l->added_dice, 3);
	eq(last_l->msgt, MSG_HIT_SUPERB);
	eq(this_l->chance, 10);
	eq(this_l->added_dice, 2);
	eq(this_l->msgt, MSG_HIT_GREAT);

	ok;
}

/*
 * test_bad_m_crit_level(), test_bad_r_crit_level(), test_bad_o_m_crit_level(),
 * test_bad_o_r_crit_level() have to be before test_m_crit_level(),
 * test_r_crit_level(), test_o_m_crit_level(), and test_o_r_crit_level().
 */
const char *suite_name = "parse/z-info";
struct test tests[] = {
	{ "negative", test_negative },
	{ "baddirective", test_baddirective },
	{ "bad_m_crit_level", test_bad_m_crit_level },
	{ "bad_r_crit_level", test_bad_r_crit_level },
	{ "bad_o_m_crit_level", test_bad_o_m_crit_level },
	{ "bad_o_r_crit_level", test_bad_o_r_crit_level },
	{ "monsters_max", test_level_monster_max },
	{ "mon_chance", test_alloc_monster_chance },
	{ "monsters_min", test_level_monster_min },
	{ "town_day", test_town_monsters_day },
	{ "town_night", test_town_monsters_night },
	{ "repro_max", test_repro_monster_max },
	{ "ood_chance", test_ood_monster_chance },
	{ "ood_amount", test_ood_monster_amount },
	{ "group_max", test_monster_group_max },
	{ "group_dist", test_monster_group_dist },
	{ "break_glyph", test_glyph_hardness },
	{ "mult_rate", test_repro_monster_rate },
	{ "life_drain", test_life_drain_percent },
	{ "flee_range", test_flee_range },
	{ "turn_range", test_turn_range },
	{ "cent_max", test_level_room_max },
	{ "door_max", test_level_door_max },
	{ "wall_max", test_wall_pierce_max },
	{ "tunn_max", test_tunn_grid_max },
	{ "amt_room", test_room_item_av },
	{ "amt_item", test_both_item_av },
	{ "amt_gold", test_both_gold_av },
	{ "pit_max", test_level_pit_max },
	{ "max_depth", test_max_depth },
	{ "day_length", test_day_length },
	{ "dungeon_hgt", test_dungeon_hgt },
	{ "dungeon_wid", test_dungeon_wid },
	{ "town_hgt", test_town_hgt },
	{ "town_wid", test_town_wid },
	{ "feeling_total", test_feeling_total },
	{ "feeling_need", test_feeling_need },
	{ "stair_skip", test_stair_skip },
	{ "move_energy", test_move_energy },
	{ "pack_size", test_pack_size },
	{ "quiver_size", test_quiver_size },
	{ "quiver_slot_size", test_quiver_slot_size },
	{ "thrown_quiver_mult", test_thrown_quiver_mult },
	{ "floor_size", test_floor_size },
	{ "inven_max", test_store_inven_max },
	{ "turns", test_store_turns },
	{ "shuffle", test_store_shuffle },
	{ "magic-level", test_store_magic_level },
	{ "max_obj_depth", test_max_obj_depth },
	{ "great_obj", test_great_obj },
	{ "great_ego", test_great_ego },
	{ "fuel_torch", test_fuel_torch },
	{ "fuel_lamp", test_fuel_lamp },
	{ "default_lamp", test_default_lamp },
	{ "max_sight", test_max_sight },
	{ "max_range", test_max_range },
	{ "start_gold", test_start_gold },
	{ "food_value", test_food_value },
	{ "m_crit_debuff_toh", test_m_crit_debuff_toh },
	{ "m_crit_chance_weight_scl", test_m_crit_chance_weight_scl },
	{ "m_crit_chance_toh_scl", test_m_crit_chance_toh_scl },
	{ "m_crit_chance_level_scl", test_m_crit_chance_level_scl },
	{ "m_crit_chance_toh_skill_scl", test_m_crit_chance_toh_skill_scl },
	{ "m_crit_chance_offset", test_m_crit_chance_offset },
	{ "m_crit_chance_range", test_m_crit_chance_range },
	{ "m_crit_power_weight_scl", test_m_crit_power_weight_scl },
	{ "m_crit_power_random", test_m_crit_power_random },
	{ "m_crit_level", test_m_crit_level },
	{ "r_crit_debuff_toh", test_r_crit_debuff_toh },
	{ "r_crit_chance_weight_scl", test_r_crit_chance_weight_scl },
	{ "r_crit_chance_toh_scl", test_r_crit_chance_toh_scl },
	{ "r_crit_chance_level_scl", test_r_crit_chance_level_scl },
	{ "r_crit_chance_launched_toh_skill_scl", test_r_crit_chance_launched_toh_skill_scl },
	{ "r_crit_chance_thrown_toh_skill_scl", test_r_crit_chance_thrown_toh_skill_scl },
	{ "r_crit_chance_offset", test_r_crit_chance_offset },
	{ "r_crit_chance_range", test_r_crit_chance_range },
	{ "r_crit_power_weight_scl", test_r_crit_power_weight_scl },
	{ "r_crit_power_random", test_r_crit_power_random },
	{ "r_crit_level", test_r_crit_level },
	{ "o_m_crit_debuff_toh", test_o_m_crit_debuff_toh },
	{ "o_m_crit_power_toh_scl_num", test_o_m_crit_power_toh_scl_num },
	{ "o_m_crit_power_toh_scl_den", test_o_m_crit_power_toh_scl_den },
	{ "o_m_crit_chance_power_scl_den", test_o_m_crit_chance_power_scl_num },
	{ "o_m_crit_chance_power_scl_num", test_o_m_crit_chance_power_scl_den },
	{ "o_m_crit_chance_add_den", test_o_m_crit_chance_add_den },
	{ "o_m_crit_level", test_o_m_crit_level },
	{ "o_r_crit_debuff_toh", test_o_r_crit_debuff_toh },
	{ "o_r_crit_power_launched_toh_scl_num", test_o_r_crit_power_launched_toh_scl_num },
	{ "o_r_crit_power_launched_toh_scl_den", test_o_r_crit_power_launched_toh_scl_den },
	{ "o_r_crit_power_thrown_toh_scl_num", test_o_r_crit_power_thrown_toh_scl_num },
	{ "o_r_crit_power_thrown_toh_scl_den", test_o_r_crit_power_thrown_toh_scl_den },
	{ "o_r_crit_chance_power_scl_den", test_o_r_crit_chance_power_scl_num },
	{ "o_r_crit_chance_power_scl_num", test_o_r_crit_chance_power_scl_den },
	{ "o_r_crit_chance_add_den", test_o_r_crit_chance_add_den },
	{ "o_r_crit_level", test_o_r_crit_level },
	{ NULL, NULL }
};
