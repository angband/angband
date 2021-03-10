/* player/calc-inventory.c */
/* Exercise calc_inventory(). */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "cmd-core.h"
#include "game-world.h"
#include "init.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-properties.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "z-quark.h"

/*
 * This is the maximum number of things (one of which will be a sentinel
 * element) to put in the gear for a test.
 */
#define TEST_SLOT_COUNT (40)

struct in_slot_desc { int tval, sval, num; bool known; bool equipped; };
struct out_slot_desc { int tval, sval, num; };
struct simple_test_case {
	struct in_slot_desc gear_in[TEST_SLOT_COUNT];
	struct out_slot_desc pack_out[TEST_SLOT_COUNT];
	struct out_slot_desc quiv_out[TEST_SLOT_COUNT];
};

int setup_tests(void **state) {
	set_file_paths();
	init_angband();

	/* Set up the player. */
	cmdq_push(CMD_BIRTH_INIT);
	cmdq_push(CMD_BIRTH_RESET);
	cmdq_push(CMD_CHOOSE_RACE);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);
	/* Use a mage so magic books are browseable. */
	cmdq_push(CMD_CHOOSE_CLASS);
	cmd_set_arg_choice(cmdq_peek(), "choice", 1);
	cmdq_push(CMD_ROLL_STATS);
	cmdq_push(CMD_NAME_CHOICE);
	cmd_set_arg_string(cmdq_peek(), "name", "Tester");
	cmdq_push(CMD_ACCEPT_CHARACTER);
	cmdq_execute(CTX_BIRTH);
	prepare_next_level(&cave, player);
	on_new_level();

	return 0;
}

int teardown_tests(void *state) {
	cleanup_angband();

	return 0;
}

/* Forget all known flavors. */
static void forget_flavors(void) {
	int i;

	for (i = 1; i < z_info->k_max; ++i) {
		struct object_kind *kind = &k_info[i];

		kind->aware = false;
	}
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
		curr = gear_object_for_use(curr, curr->number, false,
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
		obj->known = object_new();
		object_set_base_known(obj);
		object_touch(player, obj);
		if (slots->known && ! object_flavor_is_aware(obj)) {
			object_learn_on_use(player, obj);
		}
		gear_insert_end(obj);
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

/* Verify that the pack matches a given layout. */
static bool verify_pack(struct player *p, const struct out_slot_desc *slots,
		int slots_for_quiver) {
	int curr_slot = 0;
	int n_slots_used;

	if (!p->upkeep || !p->upkeep->inven) {
		return false;
	}
	n_slots_used = pack_slots_used(p);
	while (slots->tval > 0) {
		struct object_kind *kind =
			lookup_kind(slots->tval, slots->sval);

		if (curr_slot >= n_slots_used) {
			return false;
		}
		if (!p->upkeep->inven[curr_slot]) {
			return false;
		}
		if (p->upkeep->inven[curr_slot]->kind != kind) {
			return false;
		}
		if (p->upkeep->inven[curr_slot]->number != slots->num) {
			return false;
		}
		if (!object_is_carried(p, p->upkeep->inven[curr_slot])) {
			return false;
		}
		if (object_is_equipped(p->body, p->upkeep->inven[curr_slot])) {
			return false;
		}
		++curr_slot;
		++slots;
	}
	if (curr_slot + slots_for_quiver != n_slots_used) {
		return false;
	}
	return true;
}

/* Verify that the quiver matches a given layout. */
static bool verify_quiver(struct player *p, const struct out_slot_desc *slots) {
	int curr_slot = 0;
	int total = 0;

	if (!p->upkeep || !p->upkeep->quiver) {
		return false;
	}
	while (slots->tval >= 0) {
		if (slots->tval == 0) {
			if (p->upkeep->quiver[curr_slot]) {
				return false;
			}
		} else {
			struct object_kind *kind =
				lookup_kind(slots->tval, slots->sval);
			if (curr_slot >= z_info->quiver_size) {
				return false;
			}
			if (!p->upkeep->quiver[curr_slot]) {
				return false;
			}
			if (p->upkeep->quiver[curr_slot]->kind != kind) {
				return false;
			}
			if (p->upkeep->quiver[curr_slot]->number !=
					slots->num) {
				return false;
			}
			if (!object_is_carried(p,
					p->upkeep->quiver[curr_slot])) {
				return false;
			}
			if (object_is_equipped(p->body,
					p->upkeep->quiver[curr_slot])) {
				return false;
			}
			total += slots->num *
				(tval_is_ammo(p->upkeep->quiver[curr_slot]) ?
				1 : z_info->thrown_quiver_mult);
		}
		++curr_slot;
		++slots;
	}
	for (; curr_slot < z_info->quiver_size; ++curr_slot) {
		if (p->upkeep->quiver[curr_slot]) {
			return false;
		}
	}
	if (total != p->upkeep->quiver_cnt) {
		return false;
	}
	return true;
}

/*
 * Verify that another call to calc_inventory() with the gear unchanged gives
 * the same result.
 */
static bool verify_stability(struct player *p) {
	struct object **old_pack =
		mem_alloc(z_info->pack_size * sizeof(*old_pack));
	struct object **old_quiver =
		mem_alloc(z_info->quiver_size * sizeof(old_quiver));
	bool result = true;
	int i;

	for (i = 0; i < z_info->pack_size; ++i) {
		old_pack[i] = p->upkeep->inven[i];
	}
	for (i = 0; i < z_info->quiver_size; ++i) {
		old_quiver[i] = p->upkeep->quiver[i];
	}
	calc_inventory(p->upkeep, p->gear, p->body);
	for (i = 0; i < z_info->pack_size; ++i) {
		if (old_pack[i] != p->upkeep->inven[i]) {
			result = false;
		}
	}
	for (i = 0; i < z_info->quiver_size; ++i) {
		if (old_quiver[i] != p->upkeep->quiver[i]) {
			result = false;
		}
	}
	mem_free(old_quiver);
	mem_free(old_pack);
	return true;
}

static int test_calc_inventory_empty(void *state) {
	struct out_slot_desc empty = { -1, -1, -1 };

	require(flush_gear());
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, &empty, 0));
	require(verify_quiver(player, &empty));
	require(verify_stability(player));
	ok;
}

static int test_calc_inventory_only_equipped(void *state) {
	struct simple_test_case only_equipped_case = {
		{
			{ TV_SWORD, 1, 1, true, true },
			{ TV_BOW, 2, 1, true, true },
			{ TV_SHIELD, 1, 1, true, true },
			{ TV_CLOAK, 1, 1, true, true },
			{ TV_SOFT_ARMOR, 2, 1, true, true },
			{ -1, -1, -1, false, false, }
		},
		{ { -1, -1, -1 }, },
		{ { -1, -1, -1 }, }
	};

	require(flush_gear());
	require(populate_gear(only_equipped_case.gear_in));
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, only_equipped_case.pack_out, 0));
	require(verify_quiver(player, only_equipped_case.quiv_out));
	require(verify_stability(player));
	ok;
}

static int test_calc_inventory_only_pack(void *state) {
	struct simple_test_case only_pack_case = {
		{
			{ TV_SCROLL, 5, 3, true, false },
			{ TV_WAND, 3, 1, true, false },
			{ TV_FOOD, 2, 4, true, false },
			{ TV_ROD, 2, 2, true, false },
			{ TV_POTION, 4, 5, true, false },
			{ TV_MAGIC_BOOK, 1, 1, true, false },
			{ TV_LIGHT, 1, 6, true, false },
			{ TV_DIGGING, 1, 1, true, false },
			{ TV_FLASK, 1, 1, true, false },
			{ TV_STAFF, 3, 1, true, false },
			{ -1, -1, -1, false, false }
		},
		/*
		 * Usable book is first; then appear in order of decreasing
		 * tval.
		 */
		{
			{ TV_MAGIC_BOOK, 1, 1 },
			{ TV_FOOD, 2, 4 },
			{ TV_FLASK, 1, 1 },
			{ TV_POTION, 4, 5 },
			{ TV_SCROLL, 5, 3 },
			{ TV_ROD, 2, 2 },
			{ TV_WAND, 3, 1 },
			{ TV_STAFF, 3, 1 },
			{ TV_LIGHT, 1, 6 },
			{ TV_DIGGING, 1, 1 },
			{ -1, -1, -1 }
		},
		{ { -1, -1, -1 } }
	};

	require(flush_gear());
	require(populate_gear(only_pack_case.gear_in));
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, only_pack_case.pack_out, 0));
	require(verify_quiver(player, only_pack_case.quiv_out));
	require(verify_stability(player));
	ok;
}

static int test_calc_inventory_only_quiver(void *state) {
	struct simple_test_case only_quiver_case = {
		{
			{ TV_BOLT, 1, 20, true, false },
			/* spear */
			{ TV_POLEARM, 1, 1, true, false },
			{ TV_SHOT, 1, 27, true, false },
			{ TV_ARROW, 1, 15, true, false },
		},
		{ { -1, -1, -1 } },
		/*
		 * There's no launcher equipped, so the ammunition will be
		 * ordered by decreasing tval.
		 */
		{
			{ TV_BOLT, 1, 20 },
			{ TV_POLEARM, 1, 1 },
			{ TV_ARROW, 1, 15 },
			{ TV_SHOT, 1, 27 },
			{ -1, -1, -1 }
		}
	};
	struct object *obj;
	int quiver_size;

	require(flush_gear());
	require(populate_gear(only_quiver_case.gear_in));
	/*
	 * Inscribe the spear so it goes to the quiver.  Also, compute how
	 * much space the quiver will take.
	 */
	obj = player->gear;
	quiver_size = 0;
	while (obj) {
		if (obj->tval == TV_POLEARM) {
			require(of_has(obj->flags, OF_THROWING));
			obj->note = quark_add("@v1");
			quiver_size += z_info->thrown_quiver_mult * obj->number;
		} else {
			quiver_size += obj->number;
		}
		obj = obj->next;
	}
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, only_quiver_case.pack_out,
		(quiver_size + z_info->quiver_slot_size - 1) /
		z_info->quiver_slot_size));
	require(verify_quiver(player, only_quiver_case.quiv_out));
	require(verify_stability(player));
	ok;
}

static int test_calc_inventory_equipped_pack_quiver(void *state) {
	struct simple_test_case this_test_case = {
		{
			{ TV_BOLT, 1, 10, true, false },
			{ TV_SCROLL, 3, 4, true, false },
			{ TV_SOFT_ARMOR, 2, 1, false, true },
			/* dagger */
			{ TV_SWORD, 1, 1, false, false },
			{ TV_POTION, 2, 3, false, false },
			{ TV_ARROW, 2, 7, true, false },
			{ TV_SHOT, 1, 13, true, false },
			{ TV_ARROW, 1, 15, true, false },
			{ TV_NATURE_BOOK, 1, 1, false, false },
			{ TV_SCROLL, 1, 3, false, false },
			{ TV_POTION, 3, 1, true, false },
			{ TV_PRAYER_BOOK, 1, 1, false, false },
			{ TV_BOLT, 2, 5, true, false },
			{ TV_SHOT, 2, 3, true, false },
			{ TV_MAGIC_BOOK, 1, 1, false, false },
			/* sling */
			{ TV_BOW, 1, 1, false, true },
			{ TV_POTION, 5, 2, true, false },
		},
		{
			{ TV_MAGIC_BOOK, 1, 1 },
			{ TV_NATURE_BOOK, 1, 1 },
			{ TV_PRAYER_BOOK, 1, 1 },
			{ TV_POTION, 3, 1 },
			{ TV_POTION, 5, 2 },
			{ TV_POTION, 2, 3 },
			{ TV_SCROLL, 3, 4 },
			{ TV_SCROLL, 1, 3 },
			{ -1, -1, -1 }
		},
		/*
		 * A sling is equipped, so the shots should appear before the
		 * other ammunition. The rest appear in order of decreasing
		 * tval.
		 */
		{
			{ TV_SHOT, 1, 13 },
			{ TV_SHOT, 2, 3 },
			{ TV_SWORD, 1, 1 },
			{ TV_BOLT, 1, 10 },
			{ TV_BOLT, 2, 5 },
			{ TV_ARROW, 1, 15 },
			{ TV_ARROW, 2, 7 },
			{ -1, -1, -1 }
		}
	};
	struct object *obj;
	int quiver_size;

	/*
	 * Forget all flavors so the order of the known/unknown flavors set
	 * locally won't depend on previously run tests.
	 */
	forget_flavors();
	require(flush_gear());
	require(populate_gear(this_test_case.gear_in));
	/*
	 * Inscribe the spear so it goes to the quiver.  Also, compute how
	 * much space the quiver will take.
	 */
	obj = player->gear;
	quiver_size = 0;
	while (obj) {
		if (obj->tval == TV_SWORD) {
			require(of_has(obj->flags, OF_THROWING));
			obj->note = quark_add("@v2");
			quiver_size += z_info->thrown_quiver_mult * obj->number;
		} else if (tval_is_ammo(obj)) {
			quiver_size += obj->number;
		}
		obj = obj->next;
	}
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, this_test_case.pack_out,
		(quiver_size + z_info->quiver_slot_size - 1) /
		z_info->quiver_slot_size));
	require(verify_quiver(player, this_test_case.quiv_out));
	require(verify_stability(player));
	ok;
}

static int test_calc_inventory_oversubscribed_quiver(void *state) {
	struct simple_test_case this_test_case = {
		{
			{ TV_SHOT, 2, 40, true, false },
			{ TV_ARROW, 1, 40, true, false },
			{ TV_BOLT, 1, 40, true, false },
			{ TV_ARROW, 1, 40, true, false },
			{ TV_SHOT, 2, 40, true, false },
			{ TV_ARROW, 2, 10, true, false },
			/* short bow */
			{ TV_BOW, 2, 1, true, true },
			{ TV_BOLT, 3, 7, true, false },
			{ TV_ARROW, 3, 15, true, false },
			{ TV_BOLT, 1, 40, true, false },
			{ TV_SHOT, 1, 25, true, false },
			{ TV_BOLT, 2, 12, true, false },
			{ TV_SHOT, 3, 17, true, false },
			{ -1, -1, -1 }
		},
		{
			{ TV_SHOT, 2, 40 },
			{ TV_SHOT, 3, 17 },
			{ -1, -1, -1 }
		},
		{
			{ TV_ARROW, 1, 40 },
			{ TV_ARROW, 1, 40 },
			{ TV_ARROW, 2, 10 },
			{ TV_ARROW, 3, 15 },
			{ TV_BOLT, 1, 40 },
			{ TV_BOLT, 1, 40 },
			{ TV_BOLT, 2, 12 },
			{ TV_BOLT, 3, 7 },
			{ TV_SHOT, 1, 25 },
			{ TV_SHOT, 2, 40 },
			{ -1, -1, -1 }
		}
	};
	struct object *obj;
	int quiver_size;

	require(flush_gear());
	require(populate_gear(this_test_case.gear_in));
	/* Compute how much space the quiver will take. */
	obj = player->gear;
	quiver_size = 0;
	while (obj) {
		if (tval_is_ammo(obj)) {
			quiver_size += obj->number;
		}
		obj = obj->next;
	}
	/* Adjust for the ones that will end up in the pack. */
	quiver_size -= 57;
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, this_test_case.pack_out,
		(quiver_size + z_info->quiver_slot_size - 1) /
		z_info->quiver_slot_size));
	require(verify_quiver(player, this_test_case.quiv_out));
	require(verify_stability(player));
	ok;
}

static int test_calc_inventory_oversubscribed_quiver_slot(void *state) {
	struct simple_test_case this_test_case = {
		{
			{ TV_BOLT, 1, 10, true, false },
			/* dagger */
			{ TV_SWORD, 1, 1, true, false },
			{ TV_ARROW, 2, 7, true, false },
			{ TV_SHOT, 1, 13, true, false },
			/* spear */
			{ TV_POLEARM, 1, 1, true, false },
			{ TV_ARROW, 1, 15, true, false },
			{ TV_BOLT, 2, 5, true, false },
			{ TV_SHOT, 2, 3, true, false },
		},
		{
			{ TV_SWORD, 1, 1 },
			{ -1, -1, -1 }
		},
		{
			{ TV_BOLT, 1, 10 },
			{ TV_ARROW, 2, 7 },
			{ TV_POLEARM, 1, 1 },
			{ TV_BOLT, 2, 5 },
			{ TV_ARROW, 1, 15 },
			{ TV_SHOT, 1, 13 },
			{ TV_SHOT, 2, 3 },
			{ -1, -1, -1 },
		}
	};
	struct object *obj;
	int i, quiver_size;

	require(flush_gear());
	require(populate_gear(this_test_case.gear_in));
	/*
	 * Inscribe everything going to the quiver with more than one targeting
	 * each slot.  Also, compute the total size for the things in the
	 * quiver;
	 */
	obj = player->gear;
	i = 0;
	quiver_size = 0;
	while (obj) {
		if (tval_is_ammo(obj)) {
			obj->note = quark_add(format("@f%d", i / 2));
			quiver_size += obj->number;
		} else {
			obj->note = quark_add(format("@v%d", i / 2));
			if (i % 2 == 0) {
				quiver_size += z_info->thrown_quiver_mult *
					obj->number;
			}
		}
		++i;
		obj = obj->next;
	}
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, this_test_case.pack_out,
		(quiver_size + z_info->quiver_slot_size - 1) /
		z_info->quiver_slot_size));
	require(verify_quiver(player, this_test_case.quiv_out));
	require(verify_stability(player));
	ok;
}

static test_calc_inventory_quiver_split_pile(void *state) {
	struct simple_test_case this_test_case = {
		{
			{ TV_FLASK, 1, 10 },
			{ -1, -1, -1 }
		},
		{
			{ TV_FLASK, 1, 2 },
			{ -1, -1, -1 }
		},
		{
			{ 0, 0, 0 },
			{ TV_FLASK, 1, 8 },
			{ -1, -1, -1 }
		}
	};

	require(flush_gear());
	require(populate_gear(this_test_case.gear_in));
	/* Inscribe the flasks so they want to go to the quiver. */
	player->gear->note = quark_add("@v1");
	calc_inventory(player->upkeep, player->gear, player->body);
	require(verify_pack(player, this_test_case.pack_out, 1));
	require(verify_quiver(player, this_test_case.quiv_out));
	require(verify_stability(player));
	ok;
}

const char *suite_name = "player/calc-inventory";
struct test tests[] = {
	{ "calc_inventory empty", test_calc_inventory_empty },
	{ "calc_inventory only equipped", test_calc_inventory_only_equipped },
	{ "calc_inventory only pack", test_calc_inventory_only_pack },
	{ "calc_inventory only quiver", test_calc_inventory_only_quiver },
	{ "calc_inventory equipped/pack/quiver", test_calc_inventory_equipped_pack_quiver },
	{ "calc_inventory oversubscribed quiver", test_calc_inventory_oversubscribed_quiver },
	{ "calc_inventory oversubscribed quiver slot", test_calc_inventory_oversubscribed_quiver_slot },
	{ "calc_inventory split pile for quiver", test_calc_inventory_quiver_split_pile },
	{ NULL, NULL }
};
