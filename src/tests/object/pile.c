/* object/util */

#include "unit-test.h"
#include "unit-test-data.h"

#include "object.h"
#include "obj-pile.h"

int setup_tests(void **state) {
	return 0;
}

NOTEARDOWN

/* Testing the linked list functions in obj-pile.c */
int test_obj_piles(void *state) {
	struct object *pile = NULL;

	struct object *o1 = object_new();
	struct object *o2 = object_new();
	struct object *o3 = object_new();

	pile_insert(&pile, o1);
	eq(pile_contains(pile, o1), TRUE);
	eq(pile_contains(pile, o2), FALSE);
	ptreq(pile, o1);
	ptreq(pile_last_item(pile), o1);

	pile_insert_end(&pile, o2);
	eq(pile_contains(pile, o1), TRUE);
	eq(pile_contains(pile, o2), TRUE);
	eq(pile_contains(pile, o3), FALSE);
	ptreq(pile, o1);
	ptreq(pile_last_item(pile), o2);

	pile_insert_end(&pile, o3);
	eq(pile_contains(pile, o1), TRUE);
	eq(pile_contains(pile, o2), TRUE);
	eq(pile_contains(pile, o3), TRUE);
	ptreq(pile, o1);
	ptreq(pile_last_item(pile)->prev, o2);
	ptreq(pile_last_item(pile), o3);

	/* Now let's try excision */

	/* From the top */
	pile_excise(&pile, o1);
	ptreq(pile, o2);
	eq(pile_contains(pile, o1), FALSE);

	/* Now put it back */
	pile_insert(&pile, o1);

	/* From the end */
	pile_excise(&pile, o3);
	ptreq(pile, o1);
	eq(pile_contains(pile, o3), FALSE);
	ptreq(pile_last_item(pile), o2);
	ptreq(pile_last_item(pile)->prev, o1);
	object_delete(o3);

	object_pile_free(pile);

	ok;
}

const char *suite_name = "object/pile";
struct test tests[] = {
	{ "pile checking", test_obj_piles },
	{ NULL, NULL }
};
