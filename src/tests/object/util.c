/* object/util */

#include "unit-test.h"
#include "unit-test-data.h"

#include "object/object.h"

int setup_tests(void **state) {
    p_ptr->inventory = &test_inven[0];
    return 0;
}

NOTEARDOWN

/* Regression test for #1661 */
int test_obj_can_refill(void *state) {
    struct object obj_torch, obj_lantern, obj_candidate;
    object_type *light_ptr = &p_ptr->inventory[INVEN_LIGHT];

    /* Torches cannot be refilled */
    object_prep(&obj_torch, &test_torch, 1, AVERAGE);
    object_copy(light_ptr, &obj_torch);
    eq(obj_can_refill(&obj_torch), FALSE);

    /* Lanterns can be refilled */    
    object_prep(&obj_lantern, &test_lantern, 1, AVERAGE);
    object_copy(light_ptr, &obj_lantern);

    /* Not by torches */
    eq(obj_can_refill(&obj_torch), FALSE);

    /* Lanterns can be refilled by other lanterns */
    eq(obj_can_refill(&obj_lantern), TRUE);

    /* ...but not by empty lanterns */
    obj_lantern.timeout = 0;
    eq(obj_can_refill(&obj_lantern), FALSE);

    /* Lanterns can be refilled by flasks of oil */
    object_prep(&obj_candidate, &test_flask, 1, AVERAGE);
    eq(obj_can_refill(&obj_candidate), TRUE);

    /* Lanterns cannot be refilled by charging rods of treasure detection */
    object_prep(&obj_candidate, &test_rod_treasure_location, 1, AVERAGE);
    obj_candidate.timeout = 50;
    eq(obj_can_refill(&obj_candidate), FALSE);
    ok;
}

const char *suite_name = "object/util";
struct test tests[] = {
    { "obj_can_refill", test_obj_can_refill },
    { NULL, NULL }
};
