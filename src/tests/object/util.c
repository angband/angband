/* object/util */

#include "unit-test.h"
#include "unit-test-data.h"

#include "object.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"

int setup_tests(void **state) {
	player = &test_player;
    player->body = test_player_body;
	player->body.slots = &test_slot_light;
	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->fuel_torch = 5000;
	z_info->fuel_lamp = 15000;
	z_info->default_lamp = 7500;

	quarks_init();
    return 0;
}

int teardown_tests(void *state) {
	quarks_free();
	mem_free(z_info);
	return 0;
}

/* Regression test for #1661 */
int test_obj_can_refill(void *state) {
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
int test_basic_check_for_inscrip_with_int(void *state) {
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

const char *suite_name = "object/util";
struct test tests[] = {
    { "obj_can_refill", test_obj_can_refill },
    { "basic_check_for_inscrip_with_uint", test_basic_check_for_inscrip_with_int },
    { NULL, NULL }
};
