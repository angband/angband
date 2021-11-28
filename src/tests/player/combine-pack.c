/* player/combine-pack.c */
/* Exercise combine_pack(). */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "z-quark.h"

/*
 * This is the maximum number of things (one of which will be a sentinel
 * element) to put in the gear for a test.
 */
#define TEST_SLOT_COUNT (40)

struct in_slot_desc {
	int tval, sval, num;
	const char* inscrip;
	byte origin, origin_depth;
	bool known, equipped;
};
struct out_slot_desc {
	int tval, sval, num;
	byte origin, origin_depth;
	bool equipped;
};
struct simple_test_case {
	struct in_slot_desc gear_in[TEST_SLOT_COUNT];
	struct out_slot_desc gear_out[TEST_SLOT_COUNT];
};

int setup_tests(void **state) {
	set_file_paths();
	init_angband();
#ifdef UNIX
	/* Necessary for creating the randart file. */
	create_needed_dirs();
#endif

	/* Set up the player. */
	if (!player_make_simple(NULL, NULL, "Tester")) {
		cleanup_angband();
		return 1;
	}

	prepare_next_level(player);
	on_new_level();

	return 0;
}

int teardown_tests(void *state) {
	wipe_mon_list(cave, player);
	cleanup_angband();

	return 0;
}

/* Remove all of the gear. */
static bool flush_gear(void) {
	struct object *curr = player->gear;

	while (curr != NULL) {
		struct object *next = curr->next;
		bool none_left = false;

		if (object_is_equipped(player->body, curr)) {
			inven_takeoff(curr);
		}
		curr = gear_object_for_use(player, curr, curr->number, false,
			&none_left);
		if (curr->known) {
			object_free(curr->known);
		}
		object_free(curr);
		curr = next;
		if (!none_left) {
			return false;
		}
	}
	return true;
}

/* Fill the gear with specified, simple, items. */
static bool populate_gear(const struct in_slot_desc *slots) {
	while (slots->tval > 0) {
		struct object_kind *kind =
			lookup_kind(slots->tval, slots->sval);
		struct object *obj;

		if (!kind) {
			return false;
		}
		obj = object_new();
		object_prep(obj, kind, 0, RANDOMISE);
		obj->number = slots->num;
		obj->origin = slots->origin;
		obj->origin_depth = slots->origin_depth;
		if (slots->inscrip && slots->inscrip[0] != '\0') {
			obj->note = quark_add(slots->inscrip);
		}
		obj->known = object_new();
		object_set_base_known(player, obj);
		object_touch(player, obj);
		if (slots->known && ! object_flavor_is_aware(obj)) {
			object_learn_on_use(player, obj);
		}
		gear_insert_end(player, obj);
		if (!object_is_carried(player, obj)) {
			return false;
		}
		if (slots->equipped) {
			inven_wield(obj, wield_slot(obj));
			if (!object_is_equipped(player->body, obj)) {
				return false;
			}
		}

		++slots;
	}

	return true;
}

/* Verify that the gear matches a given layout. */
static bool verify_gear(struct player *p, const struct out_slot_desc *slots) {
	struct object *obj = player->gear;
	bool result = true;

	while (1) {
		struct object_kind *kind;

		if (slots->tval <= 0) {
			if (obj) {
				result = false;
			}
			break;
		}
		if (!obj) {
			result = false;
			break;
		}

		kind = lookup_kind(slots->tval, slots->sval);
		if (obj->kind != kind || obj->number != slots->num
				|| obj->origin != slots->origin
				|| (obj->origin != ORIGIN_MIXED
				&& obj->origin_depth != slots->origin_depth)
				|| slots->equipped !=
				object_is_equipped(p->body, obj)) {
			result = false;
			break;
		}

		obj = obj->next;
		++slots;
	}
	return result;
}

/*
 * Verify that another call to combine_pack() with the gear unchanged gives
 * the same result.
 */
static bool verify_stability(struct player *p) {
	int allocated = 32;
	struct object **old_gear = mem_alloc(allocated * sizeof(*old_gear));
	bool result = true;
	int n = 0, i = 0;
	struct object *obj;

	for (obj = player->gear; obj; obj = obj->next) {
		if (n >= allocated) {
			allocated += allocated;
			old_gear = mem_realloc(old_gear,
				allocated * sizeof(*old_gear));
		}
		old_gear[n] = obj;
		++n;
	}

	combine_pack(p);

	for (obj = player->gear; obj && result; obj = obj->next) {
		if (i >= n || obj != old_gear[i]) {
			result = false;
		}
		++i;
	}

	mem_free(old_gear);
	return result;
}

static int test_combine_pack_empty(void *state) {
	struct out_slot_desc empty = { -1, -1, -1, ORIGIN_NONE, 0, false };

	require(flush_gear());
	combine_pack(player);
	require(verify_gear(player, &empty));
	require(verify_stability(player));
	ok;
}

static int test_combine_pack_only_equipped(void *state) {
	struct simple_test_case only_equipped_case = {
		{
			{ TV_SWORD, 1, 1, "", ORIGIN_BIRTH, 0, true, true },
			{ TV_BOW, 2, 1, "", ORIGIN_FLOOR, 5, true, true },
			{ TV_SHIELD, 1, 1, "", ORIGIN_STORE, 0, true, true },
			{ TV_CLOAK, 1, 1, "", ORIGIN_FLOOR, 1, true, true },
			{ TV_SOFT_ARMOR, 2, 1, "", ORIGIN_BIRTH, 0, true, true },
			{ -1, -1, -1, NULL, ORIGIN_NONE, 0, false, false, }
		},
		{
			{ TV_SWORD, 1, 1, ORIGIN_BIRTH, 0, true },
			{ TV_BOW, 2, 1, ORIGIN_FLOOR, 5, true },
			{ TV_SHIELD, 1, 1, ORIGIN_STORE, 0, true },
			{ TV_CLOAK, 1, 1, ORIGIN_FLOOR, 1, true },
			{ TV_SOFT_ARMOR, 2, 1, ORIGIN_BIRTH, 0, true },
			{ -1, -1, -1, ORIGIN_NONE, 0, false }
		}
	};

	require(flush_gear());
	require(populate_gear(only_equipped_case.gear_in));
	combine_pack(player);
	require(verify_gear(player, only_equipped_case.gear_out));
	require(verify_stability(player));
	ok;
}

static int test_combine_pack_mixed(void *state) {
	struct simple_test_case mixed_case = {
		{
			{ TV_MAGIC_BOOK, 1, 1, "=g3", ORIGIN_BIRTH, 0, true, false },
			{ TV_SCROLL, 5, 3, "", ORIGIN_FLOOR, 3, true, false },
			{ TV_WAND, 3, 1, "", ORIGIN_CHEST, 4, true, false },
			{ TV_SWORD, 1, 1, "", ORIGIN_BIRTH, 0, true, true },
			{ TV_ARROW, 1, 38, "", ORIGIN_BIRTH, 0, true, false },
			{ TV_FOOD, 2, 4, "", ORIGIN_STORE, 0, true, false },
			{ TV_SCROLL, 5, 4, "", ORIGIN_STORE, 0, true, false },
			{ TV_FOOD, 2, 1, "", ORIGIN_STORE, 0, true, false },
			{ TV_MAGIC_BOOK, 1, 1, "@m1", ORIGIN_STORE, 0, true, false },
			{ TV_ARROW, 1, 6, "", ORIGIN_STORE, 0, true, false },
			{ TV_SWORD, 1, 1, "", ORIGIN_FLOOR, 1, true, false },
			{ -1, -1, -1, NULL, ORIGIN_NONE, 0, false, false }
		},
		{
			{ TV_MAGIC_BOOK, 1, 1, ORIGIN_BIRTH, 0, false },
			{ TV_SCROLL, 5, 7, ORIGIN_MIXED, 0, false },
			{ TV_WAND, 3, 1, ORIGIN_CHEST, 4, false },
			{ TV_SWORD, 1, 1, ORIGIN_BIRTH, 0, true },
			{ TV_ARROW, 1, 40, ORIGIN_MIXED, 0, false },
			{ TV_FOOD, 2, 5, ORIGIN_STORE, 0, false },
			{ TV_MAGIC_BOOK, 1, 1, ORIGIN_STORE, 0, false },
			{ TV_ARROW, 1, 4, ORIGIN_STORE, 0, false },
			{ TV_SWORD, 1, 1, ORIGIN_FLOOR, 1, false },
			{ -1, -1, -1, ORIGIN_NONE, 0, false }
		}
	};

	require(flush_gear());
	require(populate_gear(mixed_case.gear_in));
	combine_pack(player);
	require(verify_gear(player, mixed_case.gear_out));
	require(verify_stability(player));
	ok;
}

/*
 * Test case that triggered assertion failure in 4.2.3; see PowerWyrm's
 * report here, http://angband.oook.cz/forum/showpost.php?p=155986&postcount=1 .
 */
static int test_combine_pack_4_2_3_assertion(void *state) {
	struct simple_test_case assertion_case = {
		{
			/*
			 * Nine stacks of arrows to fill up all but one slot
			 * in the quiver
			 */
			{ TV_ARROW, 1, 40, "", ORIGIN_STORE, 0, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 1, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 2, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 3, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 4, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 5, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 6, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 7, true, false },
			{ TV_ARROW, 1, 40, "", ORIGIN_FLOOR, 8, true, false },
			/* Daggers; first inscribed to go to quiver */
			{ TV_SWORD, 1, 8, "@v9", ORIGIN_FLOOR, 9, true, false },
			{ TV_SWORD, 1, 40, "", ORIGIN_FLOOR, 10, true, false },
			{ -1, -1, -1, NULL, ORIGIN_NONE, 0, false, false }
		},
		{
			{ TV_ARROW, 1, 40, ORIGIN_STORE, 0, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 1, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 2, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 3, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 4, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 5, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 6, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 7, false },
			{ TV_ARROW, 1, 40, ORIGIN_FLOOR, 8, false },
			{ TV_SWORD, 1, 8, ORIGIN_FLOOR, 9, false },
			{ TV_SWORD, 1, 40, ORIGIN_FLOOR, 10, false },
			{ -1, -1, -1, ORIGIN_NONE, 0, false }
		}
	};

	require(flush_gear());
	require(populate_gear(assertion_case.gear_in));
	calc_inventory(player);
	combine_pack(player);
	require(verify_gear(player, assertion_case.gear_out));
	require(verify_stability(player));
	ok;
}

const char *suite_name = "player/combine-pack";
struct test tests[] = {
	{ "combine_pack empty", test_combine_pack_empty },
	{ "combine_pack only equipped", test_combine_pack_only_equipped },
	{ "combine_pack mixed", test_combine_pack_mixed },
	{ "combine_pack 4.2.3 assertion", test_combine_pack_4_2_3_assertion },
	{ NULL, NULL }
};
