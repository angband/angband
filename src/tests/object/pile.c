/* object/pile */

#include <stdio.h>

#include "unit-test.h"
#include "unit-test-data.h"

#include "object.h"
#include "obj-pile.h"

int setup_tests(void **state) {
	return 0;
}

NOTEARDOWN

/* Testing the linked list functions in obj-pile.c */
static int test_obj_piles(void *state) {
	struct object *pile = NULL;

	struct object *o1 = object_new();
	struct object *o2 = object_new();
	struct object *o3 = object_new();
	struct object *o4 = object_new();

	pile_insert(&pile, o1);
	eq(pile_contains(pile, o1), true);
	eq(pile_contains(pile, o2), false);
	ptreq(pile, o1);
	ptreq(pile_last_item(pile), o1);

	pile_insert_end(&pile, o2);
	eq(pile_contains(pile, o1), true);
	eq(pile_contains(pile, o2), true);
	eq(pile_contains(pile, o3), false);
	ptreq(pile, o1);
	ptreq(pile_last_item(pile), o2);

	pile_insert_end(&pile, o3);
	eq(pile_contains(pile, o1), true);
	eq(pile_contains(pile, o2), true);
	eq(pile_contains(pile, o3), true);
	ptreq(pile, o1);
	ptreq(pile_last_item(pile)->prev, o2);
	ptreq(pile_last_item(pile), o3);

	/* Now let's try excision */

	/* From the top */
	pile_excise(&pile, o1);
	ptreq(pile, o2);
	eq(pile_contains(pile, o1), false);

	/* Now put it back */
	pile_insert(&pile, o1);

	/* From the end */
	pile_excise(&pile, o3);
	ptreq(pile, o1);
	eq(pile_contains(pile, o3), false);
	ptreq(pile_last_item(pile), o2);
	ptreq(pile_last_item(pile)->prev, o1);
	object_delete(NULL, NULL, &o3);

	/* Now put it back, and add another */
	o3 = object_new();
	pile_insert_end(&pile, o3);
	pile_insert_end(&pile, o4);

	/* Try removing from the middle */
	pile_excise(&pile, o3);
	ptreq(pile, o1);

	/* Now the list should look like o1 <-> o2 <-> o4, so check that */
	null(o1->prev);
	ptreq(o1->next, o2);

	ptreq(o2->prev, o1);
	ptreq(o2->next, o4);

	null(o3->prev);
	null(o3->next);

	ptreq(o4->prev, o2);
	null(o4->next);

	/* Free up */
	object_pile_free(NULL, NULL, pile);
	object_free(o3);

	ok;
}

const char *suite_name = "object/pile";
struct test tests[] = {
	{ "pile checking", test_obj_piles },
	{ NULL, NULL }
};
