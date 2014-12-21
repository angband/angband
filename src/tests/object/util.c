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
    return 0;
}

int teardown_tests(void **state) {
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
    eq(obj_can_refill(&obj_torch), FALSE);

    /* Lanterns can be refilled */    
    object_prep(&obj_lantern, &test_lantern, 1, AVERAGE);
	player->gear = &obj_lantern;
    player->body.slots->obj = &obj_lantern; 

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
