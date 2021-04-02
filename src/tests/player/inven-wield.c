/* player/inven-wield.c */
/* Exercise inven_wield(). */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "cmd-core.h"
#include "effects.h"
#include "game-world.h"
#include "init.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "z-quark.h"

static bool find_empty_spot(struct chunk *c, struct player *p)
{
	int ntry = 0;

	while (ntry < 100) {
		if (square_isobjectholding(c, p->grid) &&
				square_object(c, p->grid) == NULL) {
			return true;
		}

		effect_simple(EF_TELEPORT, source_player(),
			"10", 0, 0, 0, 0, 0, NULL);
		++ntry;
	}
	return false;
}

static bool empty_gear(struct player *p) {
	struct object *curr = p->gear;
	int pass = 0;

	/*
	 * In first pass, only clear pack and quiver.  Then there's slots
	 * available so the second pass can take off and delete equipment
	 * without overflowing the pack.
	 */
	while (1) {
		struct object *next;
		bool none_left;

		if (curr == NULL) {
			if (pass == 0) {
				curr = p->gear;
				if (curr == NULL) {
					break;
				}
				++pass;
			} else {
				break;
			}
		}

		next = curr->next;
		if (object_is_equipped(p->body, curr)) {
			if (pass == 0) {
				curr = next;
				continue;
			}
			inven_takeoff(curr);
		}

		none_left = false;
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
	if (pack_slots_used(p) != 0) return false;
	if (p->upkeep->equip_cnt != 0) return false;
	if (p->upkeep->total_weight != 0) return false;
	return true;
}

static bool empty_floor(struct chunk *c, struct loc grid) {
	while (1) {
		struct object *obj = square_object(c, grid);
		bool none_left;

		if (!obj) {
			return true;
		}

		none_left = false;
		obj = floor_object_for_use(obj, obj->number, false, &none_left);
		if (obj->known) {
			object_free(obj->known);
		}
		object_free(obj);
		if (!none_left) {
			return false;
		}
	}
}

static struct object *setup_object(int tval, int sval, int num) {
	struct object_kind *kind = lookup_kind(tval, sval);
	struct object *obj = NULL;

	if (kind) {
		obj = object_new();
		object_prep(obj, kind, 0, RANDOMISE);
		obj->number = num;
		obj->known = object_new();
		object_set_base_known(obj);
		object_touch(player, obj);
	}
	return obj;
}

static bool fill_pack(void) {
	int slots_used = pack_slots_used(player);

	while (slots_used < z_info->pack_size) {
		struct object *obj = setup_object(TV_POTION, 1, 1);

		if (!obj) return false;
		/* Inscribe it so it doesn't stack. */
		obj->note = quark_add(format("%d", slots_used));
		gear_insert_end(obj);
		if (!object_is_carried(player, obj)) return false;
		player->upkeep->total_weight += obj->weight;
		++slots_used;
	}
	return true;
}

/*
 * Perform a subset of the checks that object_similar() and object_stackable()
 * perform.
 */
static bool check_similar(const struct object* obj1, const struct object *obj2) {
	int i;

	if (obj1 == NULL) return obj2 == NULL;
	if (obj1->kind != obj2->kind) return false;
	if (!of_is_equal(obj1->flags, obj2->flags)) return false;
	for (i = 0; i < ELEM_MAX; i++) {
		if (obj1->el_info[i].res_level != obj2->el_info[i].res_level)
			return false;
		if ((obj1->el_info[i].flags & (EL_INFO_HATES | EL_INFO_IGNORE)) !=
				(obj2->el_info[i].flags & (EL_INFO_HATES | EL_INFO_IGNORE)))
			return false;
	}
	if (tval_is_weapon(obj1) || tval_is_armor(obj1) ||
			tval_is_jewelry(obj1) || tval_is_light(obj1)) {
		if (obj1->ac != obj2->ac) return false;
		if (obj1->dd != obj2->dd) return false;
		if (obj1->ds != obj2->ds) return false;

		if (obj1->to_h != obj2->to_h) return false;
		if (obj1->to_d != obj2->to_d) return false;
		if (obj1->to_a != obj2->to_a) return false;

		for (i = 0; i < OBJ_MOD_MAX; i++) {
			if (obj1->modifiers[i] != obj2->modifiers[i])
				return false;
		}

		if (obj1->ego != obj2->ego) return false;

		if (!curses_are_equal(obj1, obj2)) return false;
	}
	if (obj1->note && obj2->note && (obj1->note != obj2->note))
		return false;

	return true;
}

int setup_tests(void **state) {
	set_file_paths();
	init_angband();

	/* Set up the player. */
	cmdq_push(CMD_BIRTH_INIT);
	cmdq_push(CMD_BIRTH_RESET);
	cmdq_push(CMD_CHOOSE_RACE);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);
	cmdq_push(CMD_CHOOSE_CLASS);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);
	cmdq_push(CMD_ROLL_STATS);
	cmdq_push(CMD_NAME_CHOICE);
	cmd_set_arg_string(cmdq_peek(), "name", "Tester");
	cmdq_push(CMD_ACCEPT_CHARACTER);
	cmdq_execute(CTX_BIRTH);
	prepare_next_level(&cave, player);
	on_new_level();

	/* Shift to empty spot so pickup or drop is easier. */
	if (!find_empty_spot(cave, player)) return 1;

	return 0;
}

int teardown_tests(void *state) {
	cleanup_angband();

	return 0;
}

static int test_inven_wield_pack_single_empty(void *state) {
	struct object *obj;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj = setup_object(TV_CLOAK, 1, 1);
	require(obj != NULL);
	gear_insert_end(obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += obj->weight;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	require(object_is_equipped(player->body, obj));
	require(object_is_carried(player, obj));
	require(obj->number == 1);
	require(obj == slot_object(player, slot));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots - 1);
	ok;
}

static int test_inven_wield_pack_stack_empty(void *state) {
	struct object *obj;
	struct object *split;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj = setup_object(TV_LIGHT, 1, 3);
	require(obj != NULL);
	gear_insert_end(obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += obj->weight * obj->number;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	/* The wielded one was split off. */
	require(!object_is_equipped(player->body, obj));
	require(object_is_carried(player, obj));
	require(obj->number == 2);
	split = slot_object(player, slot);
	require(object_is_equipped(player->body, split));
	require(object_is_carried(player, split));
	require(check_similar(obj, split));
	require(split->number == 1);
	require(player->upkeep->equip_cnt == 1);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	ok;
}

static int test_inven_wield_pack_single_filled(void *state) {
	struct object *obj1;
	struct object *obj2;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj1 = setup_object(TV_SHIELD, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	obj2 = setup_object(TV_SHIELD, 2, 1);
	require(obj2 != NULL);
	gear_insert_end(obj2);
	require(object_is_carried(player, obj2));
	player->upkeep->total_weight += obj2->weight;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	inven_wield(obj2, slot);
	require(object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	require(obj2 == slot_object(player, slot));
	require(!object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	ok;
}

static int test_inven_wield_pack_stack_filled(void *state) {
	struct object *obj1;
	struct object *obj2;
	struct object *split;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj1 = setup_object(TV_GLOVES, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	obj2 = setup_object(TV_GLOVES, 2, 2);
	require(obj2 != NULL);
	gear_insert_end(obj2);
	require(object_is_carried(player, obj2));
	player->upkeep->total_weight += obj2->weight * obj2->number;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	inven_wield(obj2, slot);
	/* The wielded one was split off. */
	require(!object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	split = slot_object(player, slot);
	require(object_is_equipped(player->body, split));
	require(object_is_carried(player, split));
	require(check_similar(obj2, split));
	require(split->number == 1);
	require(player->upkeep->equip_cnt == 1);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots + 1);
	ok;
}

static int test_inven_wield_floor_single_empty(void *state) {
	struct object *obj;
	bool note;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj = setup_object(TV_BOOTS, 1, 1);
	require(obj != NULL);
	note = false;
	require(floor_carry(cave, player->grid, obj, &note));
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	require(object_is_equipped(player->body, obj));
	require(object_is_carried(player, obj));
	require(obj == slot_object(player, slot));
	require(obj->number == 1);
	require(player->upkeep->equip_cnt == 1);
	require(old_weight + obj->weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	require(square_object(cave, player->grid) == NULL);
	ok;
}

static int test_inven_wield_floor_stack_empty(void *state) {
	struct object *obj;
	struct object *split;
	bool note;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj = setup_object(TV_CROWN, 1, 4);
	require(obj != NULL);
	note = false;
	require(floor_carry(cave, player->grid, obj, &note));
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	require(!object_is_equipped(player->body, obj));
	require(!object_is_carried(player, obj));
	require(obj->number == 3);
	split = slot_object(player, slot);
	require(object_is_equipped(player->body, split));
	require(object_is_carried(player, split));
	require(check_similar(obj, split));
	require(split->number == 1);
	require(player->upkeep->equip_cnt == 1);
	require(old_weight + split->weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	require(square_object(cave, player->grid) == obj);
	ok;
}

static int test_inven_wield_floor_single_filled(void *state) {
	struct object *obj1;
	struct object *obj2;
	bool note;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj1 = setup_object(TV_AMULET, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	obj2 = setup_object(TV_AMULET, 2, 1);
	require(obj2 != NULL);
	note = false;
	require(floor_carry(cave, player->grid, obj2, &note));
	inven_wield(obj2, slot);
	require(object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	require(obj2 == slot_object(player, slot));
	require(!object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight + obj2->weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots + 1);
	require(square_object(cave, player->grid) == NULL);
	ok;
}

static int test_inven_wield_floor_stack_filled(void *state) {
	struct object *obj1;
	struct object *obj2;
	struct object *split;
	bool note;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj1 = setup_object(TV_DIGGING, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	obj2 = setup_object(TV_HAFTED, 1, 3);
	require(obj2 != NULL);
	note = false;
	require(floor_carry(cave, player->grid, obj2, &note));
	require(wield_slot(obj2) == slot);
	inven_wield(obj2, slot);
	/* The wielded one is split off. */
	require(!object_is_equipped(player->body, obj2));
	require(!object_is_carried(player, obj2));
	require(obj2->number == 2);
	split = slot_object(player, slot);
	require(object_is_equipped(player->body, split));
	require(object_is_carried(player, split));
	require(check_similar(obj2, split));
	require(split->number == 1);
	require(!object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight + obj2->weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots + 1);
	require(square_object(cave, player->grid) == obj2);
	ok;
}

static int test_inven_wield_pack_full_no_overflow(void *state) {
	struct object *obj1;
	struct object *obj2;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj1 = setup_object(TV_SOFT_ARMOR, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	obj2 = setup_object(TV_HARD_ARMOR, 1, 1);
	require(obj2 != NULL);
	gear_insert_end(obj2);
	require(object_is_carried(player, obj2));
	player->upkeep->total_weight += obj2->weight;
	require(fill_pack());
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	require(wield_slot(obj2) == slot);
	inven_wield(obj2, slot);
	require(object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	require(obj2 == slot_object(player, slot));
	require(!object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	require(square_object(cave, player->grid) == NULL);
	ok;
}

static int test_inven_wield_pack_full_overflow(void *state) {
	struct object *obj1;
	struct object *obj2;
	struct object *split;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj1 = setup_object(TV_HELM, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	obj2 = setup_object(TV_HELM, 2, 3);
	require(obj2 != NULL);
	gear_insert_end(obj2);
	require(object_is_carried(player, obj2));
	player->upkeep->total_weight += obj2->weight * obj2->number;
	require(fill_pack());
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	require(wield_slot(obj2) == slot);
	inven_wield(obj2, slot);
	/* Wielded one is split off. */
	require(!object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 2);
	split = slot_object(player, slot);
	require(object_is_equipped(player->body, split));
	require(object_is_carried(player, split));
	require(check_similar(obj2, split));
	require(split->number == 1);
	require(!object_is_equipped(player->body, obj1));
	require(!object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight - obj1->weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	require(square_object(cave, player->grid) == obj1);
	ok;
}

static int test_inven_wield_floor_full_overflow(void *state) {
	struct object *obj1;
	struct object *obj2;
	bool note;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	require(empty_floor(cave, player->grid));
	obj1 = setup_object(TV_BOW, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot = wield_slot(obj1);
	inven_wield(obj1, slot);
	require(fill_pack());
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	obj2 = setup_object(TV_BOW, 2, 1);
	require(obj2 != NULL);
	note = false;
	require(floor_carry(cave, player->grid, obj2, &note));
	require(wield_slot(obj2) == slot);
	inven_wield(obj2, slot);
	require(object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	require(slot_object(player, slot) == obj2);
	require(!object_is_equipped(player->body, obj1));
	require(!object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 1);
	require(old_weight - obj1->weight + obj2->weight ==
		player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	require(square_object(cave, player->grid) == obj1);
	ok;
}

static int test_inven_wield_ring_none(void *state) {
	struct object *obj;
	int slot;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj = setup_object(TV_RING, 1, 1);
	require(obj != NULL);
	gear_insert_end(obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += obj->weight;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	require(object_is_equipped(player->body, obj));
	require(object_is_carried(player, obj));
	require(obj->number == 1);
	require(slot_object(player, slot) == obj);
	require(player->upkeep->equip_cnt == 1);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots - 1);
	ok;
}

static int test_inven_wield_ring_one(void *state) {
	struct object *obj1;
	struct object *obj2;
	int slot1, slot2;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj1 = setup_object(TV_RING, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot1 = wield_slot(obj1);
	/* Wear first ring. */
	inven_wield(obj1, slot1);
	obj2 = setup_object(TV_RING, 2, 1);
	require(obj2 != NULL);
	gear_insert_end(obj2);
	require(object_is_carried(player, obj2));
	player->upkeep->total_weight += obj2->weight;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	/* Verify that wearing the second works as expected. */
	slot2 = wield_slot(obj2);
	require(slot2 != slot1);
	inven_wield(obj2, slot2);
	require(object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	require(slot_object(player, slot2) == obj2);
	require(object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(obj1->number == 1);
	require(slot_object(player, slot1) == obj1);
	require(player->upkeep->equip_cnt == 2);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots - 1);
	ok;
}

static int test_inven_wield_ring_two(void *state) {
	struct object *obj1;
	struct object *obj2;
	struct object *obj3;
	int slot1, slot2;
	int old_weight;
	int old_slots;

	require(empty_gear(player));
	obj1 = setup_object(TV_RING, 1, 1);
	require(obj1 != NULL);
	gear_insert_end(obj1);
	require(object_is_carried(player, obj1));
	player->upkeep->total_weight += obj1->weight;
	slot1 = wield_slot(obj1);
	/* Wear the first ring. */
	inven_wield(obj1, slot1);
	obj2 = setup_object(TV_RING, 2, 1);
	require(obj2 != NULL);
	gear_insert_end(obj2);
	require(object_is_carried(player, obj2));
	player->upkeep->total_weight += obj2->weight;
	slot2 = wield_slot(obj2);
	require(slot2 != slot1);
	/* Wear the second ring. */
	inven_wield(obj2, slot2);
	obj3 = setup_object(TV_RING, 3, 1);
	require(obj3 != NULL);
	gear_insert_end(obj3);
	require(object_is_carried(player, obj3));
	player->upkeep->total_weight += obj3->weight;
	old_weight = player->upkeep->total_weight;
	old_slots = pack_slots_used(player);
	/* Verify that wearing the third ring in place of the first works. */
	inven_wield(obj3, slot1);
	require(object_is_equipped(player->body, obj3));
	require(object_is_carried(player, obj3));
	require(obj3->number == 1);
	require(slot_object(player, slot1) == obj3);
	require(object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(obj2->number == 1);
	require(slot_object(player, slot2) == obj2);
	require(!object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(player->upkeep->equip_cnt == 2);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	/* Verify that wearing the first ring in place of the second works. */
	inven_wield(obj1, slot2);
	require(object_is_equipped(player->body, obj1));
	require(object_is_carried(player, obj1));
	require(obj1->number == 1);
	require(slot_object(player, slot2) == obj1);
	require(object_is_equipped(player->body, obj3));
	require(object_is_carried(player, obj3));
	require(obj3->number == 1);
	require(slot_object(player, slot1) == obj3);
	require(!object_is_equipped(player->body, obj2));
	require(object_is_carried(player, obj2));
	require(player->upkeep->equip_cnt == 2);
	require(old_weight == player->upkeep->total_weight);
	require(pack_slots_used(player) == old_slots);
	ok;
}

const char *suite_name = "player/inven-wield";
struct test tests[] = {
	{ "inven_wield pack/single/empty slot", test_inven_wield_pack_single_empty },
	{ "inven_wield pack/stack/empty slot", test_inven_wield_pack_stack_empty },
	{ "inven_wield pack/single/filled slot", test_inven_wield_pack_single_filled },
	{ "inven_wield pack/stack/filled slot", test_inven_wield_pack_stack_filled },
	{ "inven_wield floor/single/empty slot", test_inven_wield_floor_single_empty },
	{ "inven_wield floor/stack/empty slot", test_inven_wield_floor_stack_empty },
	{ "inven_wield floor/single/filled slot", test_inven_wield_floor_single_filled },
	{ "inven_wield floor/stack/filled slot", test_inven_wield_floor_stack_filled },
	{ "inven_wield pack/full pack/no overflow", test_inven_wield_pack_full_no_overflow },
	{ "inven_wield pack/full pack/overflow", test_inven_wield_pack_full_overflow },
	{ "inven_wield floor/full pack/overflow", test_inven_wield_floor_full_overflow },
	{ "inven_wield ring none carried", test_inven_wield_ring_none },
	{ "inven_wield ring one carried", test_inven_wield_ring_one },
	{ "inven_wield ring two carried", test_inven_wield_ring_two },
	{ NULL, NULL }
};
