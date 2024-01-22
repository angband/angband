/* object/util */

#include "unit-test.h"
#include "unit-test-data.h"

#include "object.h"
#include "obj-curse.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"

static struct curse_data *obj_curse_data = NULL;

int setup_tests(void **state) {
	player = &test_player;
	player->body = test_player_body;
	player->body.slots = &test_slot_light;
	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->fuel_torch = 5000;
	z_info->fuel_lamp = 15000;
	z_info->default_lamp = 7500;
	/*
	 * One which has no effect on weight, two which have additive effects
	 * and two which have multiplicative effects.
	 */
	z_info->curse_max = 5;
	curses = mem_zalloc(z_info->curse_max * sizeof(*curses));
	curses[0].obj = mem_zalloc(sizeof(*curses[0].obj));
	curses[1].obj = mem_zalloc(sizeof(*curses[1].obj));
	curses[1].obj->weight = -1;
	curses[2].obj = mem_zalloc(sizeof(*curses[2].obj));
	curses[2].obj->weight = 3;
	curses[3].obj = mem_zalloc(sizeof(*curses[2].obj));
	curses[3].obj->weight = 90;
	of_on(curses[3].obj->flags, OF_MULTIPLY_WEIGHT);
	curses[4].obj = mem_zalloc(sizeof(*curses[2].obj));
	curses[4].obj->weight = 120;
	of_on(curses[4].obj->flags, OF_MULTIPLY_WEIGHT);
	obj_curse_data =
		mem_zalloc(z_info->curse_max * sizeof(*obj_curse_data));
	quarks_init();
	return 0;
}

int teardown_tests(void *state) {
	int i;

	quarks_free();
	mem_free(obj_curse_data);
	for (i = 0; i < z_info->curse_max; ++i) {
		mem_free(curses[i].obj);
	}
	mem_free(curses);
	mem_free(z_info);
	return 0;
}

/* Regression test for #1661 */
static int test_obj_can_refill(void *state) {
    struct object obj_torch, obj_lantern, obj_candidate;

    /* Torches cannot be refilled */
    object_prep(&obj_torch, &test_torch, 1, AVERAGE);
	player->gear = &obj_torch;
    player->body.slots->obj = &obj_torch; 
    eq(obj_can_refill(&obj_torch), false);

    /* Lanterns can be refilled */    
    object_prep(&obj_lantern, &test_lantern, 1, AVERAGE);
	player->gear = &obj_lantern;
    player->body.slots->obj = &obj_lantern; 

    /* Not by torches */
    eq(obj_can_refill(&obj_torch), false);

    /* Lanterns can be refilled by other lanterns */
    eq(obj_can_refill(&obj_lantern), true);

    /* ...but not by empty lanterns */
    obj_lantern.timeout = 0;
    eq(obj_can_refill(&obj_lantern), false);

    /* Lanterns can be refilled by flasks of oil */
    object_prep(&obj_candidate, &test_flask, 1, AVERAGE);
    eq(obj_can_refill(&obj_candidate), true);

    /* Lanterns cannot be refilled by charging rods of treasure detection */
    object_prep(&obj_candidate, &test_rod_treasure_location, 1, AVERAGE);
    obj_candidate.timeout = 50;
    eq(obj_can_refill(&obj_candidate), false);
    ok;
}

/* Test basic functionality of check_for_inscrip_with_int(). */
static int test_basic_check_for_inscrip_with_int(void *state) {
    struct object obj;
    int dummy = 8974;
    int inarg;

    /* No inscription should return zero and leave inarg unchanged. */
    memset(&obj, 0, sizeof(obj));
    inarg = dummy;
    eq(check_for_inscrip_with_int(&obj, "=g", &inarg), 0);
    eq(inarg, dummy);

    /* An inscription not containing the search string should return zero and
     * leave inarg unchanged. */
    obj.note = quark_add("@m1@b1@G1");
    inarg = dummy;
    eq(check_for_inscrip_with_int(&obj, "=g", &inarg), 0);
    eq(inarg, dummy);

    /* An inscription containing the search string but not followed by an
     * integer should return zero and leave inarg unchanged. */
    obj.note = quark_add("=g@m1@b1@G1");
    inarg = dummy;
    eq(check_for_inscrip_with_int(&obj, "=g", &inarg), 0);
    eq(inarg, dummy);

    /* An inscription containing one instance of the search string followed
     * by a nonnegative integer should return one and set inarg to the value
     * of the integer. */
    obj.note = quark_add("=g5@m1@b1@G1");
    inarg = dummy;
    eq(check_for_inscrip_with_int(&obj, "=g", &inarg), 1);
    eq(inarg, 5);

    /* An inscription containing two instances of the search string followed
     * by a nonnegative integer should return two and set inarg to the value
     * of the integer following the first instance. */
    obj.note = quark_add("@m1@b1=g8@G1=g5");
    inarg = dummy;
    eq(check_for_inscrip_with_int(&obj, "=g", &inarg), 2);
    eq(inarg, 8);

    ok;
}

static int test_object_weight_one(void *state) {
	struct object obj;
	int16_t weight;
	int i;

	object_prep(&obj, &test_torch, 1, AVERAGE);

	/*
	 * With no curses, the weight should be the same as the base weight
	 * as long as that is non-negative and should have no dependence
	 * on the number in the stack.
	 */
	obj.number = 1;
	obj.weight = 100;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight);
	obj.number = 10;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight);
	obj.weight = -5;
	weight = object_weight_one(&obj);
	eq(weight, 0);

	/*
	 * With a curse that has no effect on weight, the result again should
	 * be the same as the base weight.
	 */
	obj_curse_data[0].power = 1;
	for (i = 1; i < z_info->curse_max; ++i) {
		obj_curse_data[0].power = 0;
	}
	obj.curses = obj_curse_data;
	obj.number = 1;
	obj.weight = 100;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight);
	obj.number = 10;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight);
	obj.weight = -5;
	weight = object_weight_one(&obj);
	eq(weight, 0);

	/*
	 * With a curse that has a negative additive effect on weight, should
	 * see a change.
	 */
	obj_curse_data[0].power = 0;
	obj_curse_data[1].power = 1;
	obj.number = 1;
	obj.weight = 100;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight + curses[1].obj->weight);
	obj.number = 10;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight + curses[1].obj->weight);
	/*
	 * Start at less than 0.  That is coerced to zero before applying
	 * adjustments, and all intermediate negative results are coerced to 0.
	 */
	obj.weight = -5;
	weight = object_weight_one(&obj);
	eq(weight, 0);
	/* Start at 0.  All intermediate negative results are coerced to 0. */
	obj.weight = 0;
	weight = object_weight_one(&obj);
	eq(weight, 0);
	obj.weight = 1;
	weight = object_weight_one(&obj);
	eq(weight, 0);

	/*
	 * With a curse that has a positive additive effect on weight, should
	 * see a change.
	 */
	obj_curse_data[1].power = 0;
	obj_curse_data[2].power = 1;
	obj.number = 1;
	obj.weight = 100;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight + curses[2].obj->weight);
	obj.number = 10;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight + curses[2].obj->weight);
	/*
	 * Start at less than 0.  That is coerced to zero before applying
	 * adjustments, and all intermediate negative results are coerced to 0.
	 */
	obj.weight = -5;
	weight = object_weight_one(&obj);
	eq(weight, curses[2].obj->weight);
	obj.weight = 0;
	weight = object_weight_one(&obj);
	eq(weight, curses[2].obj->weight);
	/* Verify that it does not overflow. */
	obj.weight = 32767;
	weight = object_weight_one(&obj);
	eq(weight, 32767);

	/*
	 * Verify that two additive curses applied to the same object work
	 * as expected.
	 */
	obj_curse_data[1].power = 1;
	obj_curse_data[2].power = 1;
	obj.number = 1;
	obj.weight = 100;
	weight = object_weight_one(&obj);
	eq(weight, obj.weight + curses[1].obj->weight + curses[2].obj->weight);

	/*
	 * With a curse that has a multiplicative reducing effect on weight,
	 * should see a change.
	 */
	obj_curse_data[1].power = 0;
	obj_curse_data[2].power = 0;
	obj_curse_data[3].power = 1;
	obj.number = 1;
	obj.weight = 80;
	weight = object_weight_one(&obj);
	eq(weight, (obj.weight * (int32_t)curses[3].obj->weight + 50) / 100);
	obj.number = 10;
	weight = object_weight_one(&obj);
	eq(weight, (obj.weight * (int32_t)curses[3].obj->weight + 50) / 100);
	/*
	 * Start at less than 0.  That is coerced to zero (because the
	 * multiplicative factor is less than or equal to one) before applying
	 * adjustments.
	 */
	obj.weight = -5;
	weight = object_weight_one(&obj);
	eq(weight, 0);
	/*
	 * Zero is also coerced to zero when the multiplicative factor is less
	 * than or equal to one.
	 */
	obj.weight = 0;
	weight = object_weight_one(&obj);
	eq(weight, 0);

	/*
	 * With a curse that has a multiplicative amplifying effect on weight,
	 * should see a change.
	 */
	obj_curse_data[3].power = 0;
	obj_curse_data[4].power = 1;
	obj.number = 1;
	obj.weight = 80;
	weight = object_weight_one(&obj);
	eq(weight, (obj.weight * (int32_t)curses[4].obj->weight + 50) / 100);
	obj.number = 10;
	weight = object_weight_one(&obj);
	eq(weight, (obj.weight * (int32_t)curses[4].obj->weight + 50) / 100);
	/*
	 * Start at less than 0.  That is coerced to one (because the
	 * multiplicative factor is greater than one) before applying
	 * adjustments.
	 */
	obj.weight = -5;
	weight = object_weight_one(&obj);
	eq(weight, 1);
	/*
	 * Zero is also coerced to one when the multiplicative factor is greater
	 * than one.
	 */
	obj.weight = 0;
	weight = object_weight_one(&obj);
	eq(weight, 1);
	/* Verify that it does not overflow. */
	obj.weight = 32767;
	weight = object_weight_one(&obj);
	eq(weight, 32767);

	/*
	 * Verify that two multiplicative curses applied to the same object work
	 * as expected.
	 */
	obj_curse_data[3].power = 1;
	obj_curse_data[4].power = 1;
	obj.number = 1;
	obj.weight = 80;
	weight = object_weight_one(&obj);
	eq(weight, (((obj.weight * (int32_t)curses[3].obj->weight + 50)
		/ 100) * curses[4].obj->weight + 50) / 100);

	ok;
}

const char *suite_name = "object/util";
struct test tests[] = {
	{ "obj_can_refill", test_obj_can_refill },
	{ "basic_check_for_inscrip_with_uint", test_basic_check_for_inscrip_with_int },
	{ "object_weight_one", test_object_weight_one },
	{ NULL, NULL }
};
