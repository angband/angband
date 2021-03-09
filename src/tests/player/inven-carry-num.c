/* player/inven-carry-num.c */
/* Exercise inven_carry_num() and inven_carry_okay(). */

#include "test-utils.h"
#include "unit-test.h"
#include "cmd-core.h"
#include "init.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-calcs.h"

struct carry_num_state {
	struct player *p;
	/*
	 * Want something that is neither ammunition nor good for throwing
	 * (torch), ammunition but not good for throwing (arrow), ammunition
	 * and good for throwing (shot), and good for throwing but not
	 * ammunition (flask of oil) when testing how the quiver fills.
	 */
	struct object *torch;
	struct object *arrow;
	struct object *shot;
	struct object *flask;
	struct object *inscribed_flask;
};

int setup_tests(void **state) {
	struct carry_num_state *cns;
	int i;

	set_file_paths();
	init_angband();

	/*
	 * Use a smaller than normal pack and quiver so it is less tedious to
	 * fill them up.  The tests are structured to assume that pack_size is
	 * at least two larger than the quiver size.  Use a quiver size of
	 * three so it is possible to fill it up with one stack of arrows, one
	 * stack of shots, and one stack of flasks.
	 */
	z_info->pack_size = 5;
	z_info->quiver_size = 3;

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

	cns = mem_zalloc(sizeof *cns);
	cns->p = player;
	cns->torch = object_new();
	object_prep(cns->torch, lookup_kind(TV_LIGHT, 1), 0, RANDOMISE);
	cns->torch->known = object_new();
	object_set_base_known(cns->torch);
	object_touch(cns->p, cns->torch);
	cns->arrow = object_new();
	object_prep(cns->arrow, lookup_kind(TV_ARROW, 1), 0, RANDOMISE);
	cns->arrow->known = object_new();
	object_set_base_known(cns->arrow);
	object_touch(cns->p, cns->arrow);
	cns->shot = object_new();
	object_prep(cns->shot, lookup_kind(TV_SHOT, 1), 0, RANDOMISE);
	cns->shot->known = object_new();
	object_set_base_known(cns->shot);
	object_touch(cns->p, cns->shot);
	cns->flask = object_new();
	object_prep(cns->flask, lookup_kind(TV_FLASK, 1), 0, RANDOMISE);
	cns->flask->known = object_new();
	object_set_base_known(cns->flask);
	object_touch(cns->p, cns->flask);
	/* Make a version that is inscribed so it will go into the quiver. */
	cns->inscribed_flask = object_new();
	object_prep(cns->inscribed_flask, cns->flask->kind, 0, RANDOMISE);
	cns->inscribed_flask->note =
		quark_add(format("@v%d", z_info->quiver_size - 1));
	cns->inscribed_flask->known = object_new();
	object_set_base_known(cns->inscribed_flask);
	object_touch(cns->p, cns->inscribed_flask);
	*state = cns;

	return 0;
}

int teardown_tests(void *state) {
	struct carry_num_state *cns = state;
	int i;

	if (cns->torch->known) {
		object_free(cns->torch->known);
	}
	object_free(cns->torch);
	if (cns->arrow->known) {
		object_free(cns->arrow->known);
	}
	object_free(cns->arrow);
	if (cns->shot->known) {
		object_free(cns->shot->known);
	}
	object_free(cns->shot);
	if (cns->flask->known) {
		object_free(cns->flask->known);
	}
	object_free(cns->flask);
	if (cns->inscribed_flask->known) {
		object_free(cns->inscribed_flask->known);
	}
	object_free(cns->inscribed_flask);
	mem_free(state);

	cleanup_angband();

	return 0;
}

static bool fill_pack_quiver(struct carry_num_state *cns, int n_pack,
		int n_arrow, int n_shot, int n_flask) {
	struct object *curr = cns->p->gear;
	int qslot = 0;
	int i;

	/* Empty out the pack and quiver. */
	while (curr != NULL) {
		if (! object_is_equipped(cns->p->body, curr)) {
			struct object *next = curr->next;
			bool none_left = false;

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
		} else {
			curr = curr->next;
		}
	}

	/* Add to pack. */
	for (i = 0; i < n_pack; ++i) {
		if (pack_is_full()) {
			return false;
		}
		curr = object_new();
		object_copy(curr, cns->torch);
		/* Vary inscriptions so they won't stack. */
		curr->note = quark_add(format("dummy%d", i));
		if (cns->torch->known) {
			curr->known = object_new();
			object_copy(curr->known, cns->torch->known);
		}
		inven_carry(cns->p, curr, false, false);
		calc_inventory(cns->p->upkeep, cns->p->gear, cns->p->body);
		if (! object_is_carried(cns->p, curr) ||
				object_is_equipped(cns->p->body, curr)) {
			return false;
		}
	}

	/* Add arrows. */
	i = 0;
	while (i < n_arrow) {
		int n = n_arrow - i;

		if (n > z_info->quiver_slot_size) {
			n = z_info->quiver_slot_size;
		}
		if (pack_is_full()) {
			return false;
		}
		curr = object_new();
		object_copy(curr, cns->arrow);
		curr->number = n;
		if (cns->arrow->known) {
			curr->known = object_new();
			object_copy(curr->known, cns->arrow->known);
			curr->known->number = n;
		}
		inven_carry(cns->p, curr, false, false);
		calc_inventory(cns->p->upkeep, cns->p->gear, cns->p->body);
		if (! object_is_carried(cns->p, curr) ||
				object_is_equipped(cns->p->body, curr)) {
			return false;
		}
		i += n;
		++qslot;
	}

	/* Add shots. */
	i = 0;
	while (i < n_shot) {
		int n = n_shot - i;

		if (n > z_info->quiver_slot_size) {
			n = z_info->quiver_slot_size;
		}
		if (pack_is_full()) {
			return false;
		}
		curr = object_new();
		object_copy(curr, cns->shot);
		curr->number = n;
		if (cns->shot->known) {
			curr->known = object_new();
			object_copy(curr->known, cns->shot->known);
			curr->known->number = n;
		}
		inven_carry(cns->p, curr, false, false);
		calc_inventory(cns->p->upkeep, cns->p->gear, cns->p->body);
		if (! object_is_carried(cns->p, curr) ||
				object_is_equipped(cns->p->body, curr)) {
			return false;
		}
		i += n;
		++qslot;
	}

	/* Add flasks. */
	i = 0;
	while (i < n_flask) {
		int n = n_flask - i;

		if (n * z_info->thrown_quiver_mult > z_info->quiver_slot_size) {
			n = z_info->quiver_slot_size /
				z_info->thrown_quiver_mult;
		}
		if (pack_is_full()) {
			return false;
		}
		curr = object_new();
		object_copy(curr, cns->flask);
		curr->number = n;
		/* Inscribe so it goes into the quiver. */
		if (qslot >= 0 && qslot < z_info->quiver_size) {
			curr->note = quark_add(format("@v%d", qslot));
		} else {
			if (curr->known) {
				object_free(curr->known);
			}
			object_free(curr);
			return false;
		}
		if (cns->flask->known) {
			curr->known = object_new();
			object_copy(curr->known, cns->flask->known);
			curr->known->number = n;
			curr->known->note = curr->note;
		}
		inven_carry(cns->p, curr, false, false);
		calc_inventory(cns->p->upkeep, cns->p->gear, cns->p->body);
		if (! object_is_carried(cns->p, curr) ||
				object_is_equipped(cns->p->body, curr)) {
			return false;
		}
		i += n;
		++qslot;
	}

	return true;
}

/* Try inven_carry() and inven_carry_okay() for one specific object. */
static bool perform_one_test(struct carry_num_state *cns, struct object *obj,
		int n_try, int n_expected) {
	int n_old = obj->number;
	bool success = true;

	obj->number = n_try;
	if (inven_carry_num(obj) != n_expected) {
		success = false;
	}
	if (inven_carry_okay(obj)) {
		if (n_expected == 0) {
			success = false;
		}
	} else {
		if (n_expected > 0) {
			success = false;
		}
	}
	obj->number = n_old;
	return success;
}

static int test_carry_num_empty_pack_empty_quiver(void *state) {
	struct carry_num_state *cns = state;
	require(fill_pack_quiver(cns, 0, 0, 0, 0));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	ok;
}

static int test_carry_num_partial_pack_empty_quiver(void *state) {
	struct carry_num_state *cns = state;
	require(fill_pack_quiver(cns, z_info->pack_size - 1, 0, 0, 0));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/* Since it is not inscribed, it goes into the remaining pack slot. */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Since it is inscribed, it should go into the quiver, taking up one
	 * slot and expanding the quiver to take the remaining pack slot.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));
	ok;
}

static int test_carry_num_full_pack_empty_quiver(void *state) {
	struct carry_num_state *cns = state;
	require(fill_pack_quiver(cns, z_info->pack_size, 0, 0, 0));
	/* Will fit because it will stack with torches already there. */
	require(perform_one_test(cns, cns->torch, 3, 3));
	/*
	 * No pack slots are available so the quiver can not expand and
	 * nothing can be added to it.
	 */
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size, 0));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size, 0));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size, 0));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, 0));
	ok;
}

static int test_carry_num_empty_pack_partial_quiver(void *state) {
	const int n_arrow_miss = 8;
	const int n_shot_miss = 9;
	const int n_flask_miss = 2;
	struct carry_num_state *cns = state;
	/* First do tests with one quiver slot empty. */
	require(fill_pack_quiver(cns, 0,
		z_info->quiver_slot_size - n_arrow_miss,
		z_info->quiver_slot_size - n_shot_miss, 0));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/* It is not inscribed, so it should go into the pack. */
	require(perform_one_test(cns, cns->flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	/*
         * It is inscribed and should go into the one quiver slot with the
	 * remainder going to the other available pack slot.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));

	require(fill_pack_quiver(cns, 0,
		z_info->quiver_slot_size - n_arrow_miss, 0,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * It is not inscribed; some can combine with what's in the quiver; the
	 * rest should go to the pack.
	 */
	require(perform_one_test(cns, cns->flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	/*
	 * It is inscribed (but at a different pack slot than what's there) so
	 * some will go into the remaining quiver slot and the rest will go
	 * to the pack.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));

	require(fill_pack_quiver(cns, 0, 0,
		z_info->quiver_slot_size - n_shot_miss,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * It is not inscribed; some can combine with what's in the quiver; the
	 * rest should go to the pack.
	 */
	require(perform_one_test(cns, cns->flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	/*
	 * It is inscribed (but at a different pack slot than what's there) so
	 * some will go into the remaining quiver slot and the rest will go to
	 * the pack.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));

	/* Then do tests with all slots filled but with room in each stack. */
	require(fill_pack_quiver(cns, 0,
		z_info->quiver_slot_size - n_arrow_miss,
		z_info->quiver_slot_size - n_shot_miss,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	/*
	 * n_arrow_miss should go to the quiver; the remainder should go to
	 * the pack.
	 */
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * n_shot_miss should go to the quiver; the remainder should go to
	 * the pack.
	 */
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * It is not inscribed; some can combine with what's in the quiver; the
	 * rest should go to the pack.
	 */
	require(perform_one_test(cns, cns->flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	/*
	 * It is inscribed for the same slot as what's there.  Some will go
	 * to the quiver.  The rest will go to the pack.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));

	ok;
}

static int test_carry_num_partial_pack_partial_quiver(void *state) {
	const int n_arrow_miss = 9;
	const int n_shot_miss = 8;
	const int n_flask_miss = 1;
	struct carry_num_state *cns = state;
	/*
	 * First do tests with one quiver slot empty.  Always leave one pack
	 * slot available.
	 */
	require(fill_pack_quiver(cns, z_info->pack_size - 3,
		z_info->quiver_slot_size - n_arrow_miss,
		z_info->quiver_slot_size - n_shot_miss, 0));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/* Not inscribed, so it goes into the remaining pack slot. */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Goes into the remaining quiver slot, but that leaves no pack slots
	 * for the remainder.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));

	require(fill_pack_quiver(cns, z_info->pack_size - 3,
		z_info->quiver_slot_size - n_arrow_miss, 0,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Combines with those in the quiver; remainder goes to empty pack slot.
	 */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Goes into the remaining quiver slot, but that leaves no pack slots
	 * for the remainder.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));

	require(fill_pack_quiver(cns, z_info->pack_size - 3, 0,
		z_info->quiver_slot_size - n_shot_miss,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Combines with those in the quiver; remainder goes to empty pack slot.
	 */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Goes into the remaining quiver slot, but that leaves no pack slots
	 * for the remainder.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));

	/* Then do tests with all slots filled but with room in each stack. */
	require(fill_pack_quiver(cns, z_info->pack_size - 4,
		z_info->quiver_slot_size - n_arrow_miss,
		z_info->quiver_slot_size - n_shot_miss,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Combines with those in the quiver; remainder goes to empty pack slot.
	 */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	/*
	 * Inscription now matches what's in the quiver so it can combine.
	 * Remainder go to the empty pack slot.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));

	ok;
}

static int test_carry_num_full_pack_partial_quiver(void *state) {
	const int n_arrow_miss = 5;
	const int n_shot_miss = 4;
	const int n_flask_miss = 3;
	struct carry_num_state *cns = state;
	/* First do tests with one quiver slot empty. */
	require(fill_pack_quiver(cns, z_info->pack_size - 2,
		z_info->quiver_slot_size - n_arrow_miss,
		z_info->quiver_slot_size - n_shot_miss, 0));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		n_arrow_miss + n_shot_miss));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		n_arrow_miss + n_shot_miss));
	/*
	 * It's not inscribed so it only goes to the quiver if it stacks with
	 * something already there.
	 */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size, 0));
	/*
	 * Goes to the empty slot in the quiver, only add enough so that the
	 * quiver does not need more pack slots.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		(n_arrow_miss + n_shot_miss) / z_info->thrown_quiver_mult));

	require(fill_pack_quiver(cns, z_info->pack_size - 2,
		z_info->quiver_slot_size - n_arrow_miss, 0,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		n_arrow_miss + z_info->thrown_quiver_mult * n_flask_miss));
	/* Goes to the empty quiver slot. */
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		n_arrow_miss + z_info->thrown_quiver_mult * n_flask_miss));
	/* Only stacks with what's there and won't go to the empty slot. */
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		n_flask_miss));
	/*
	 * Inscribed differently than what's in the quiver, so it can't stack.
	 * Some go into the empty slot targeted by the inscription.
	 */
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		n_arrow_miss / z_info->thrown_quiver_mult + n_flask_miss));

	require(fill_pack_quiver(cns, z_info->pack_size - 2, 0,
		z_info->quiver_slot_size - n_shot_miss,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	/* Goes to the empty quiver slot. */
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		n_shot_miss + n_flask_miss * z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		n_shot_miss + n_flask_miss * z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		n_flask_miss));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size,
		n_shot_miss / z_info->thrown_quiver_mult + n_flask_miss));

	/* Then do tests with all slots filled but with room in each stack. */
	require(fill_pack_quiver(cns, z_info->pack_size - 3,
		z_info->quiver_slot_size - n_arrow_miss,
		z_info->quiver_slot_size - n_shot_miss,
		(z_info->quiver_slot_size - z_info->thrown_quiver_mult *
		n_flask_miss) / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		n_arrow_miss));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		n_shot_miss));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		n_flask_miss));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, n_flask_miss));

	ok;
}

static int test_carry_num_empty_pack_full_quiver(void *state) {
	struct carry_num_state *cns = state;
	require(fill_pack_quiver(cns, 0, z_info->quiver_slot_size,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	ok;
}

static int test_carry_num_partial_pack_full_quiver(void *state) {
	struct carry_num_state *cns = state;
	require(fill_pack_quiver(cns, z_info->pack_size - 4,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size,
		z_info->quiver_slot_size));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, z_info->quiver_slot_size));
	ok;
}

static int test_carry_num_full_pack_full_quiver(void *state) {
	struct carry_num_state *cns = state;
	require(fill_pack_quiver(cns, z_info->pack_size - 3,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size,
		z_info->quiver_slot_size / z_info->thrown_quiver_mult));
	require(perform_one_test(cns, cns->torch, 3, 3));
	require(perform_one_test(cns, cns->arrow, z_info->quiver_slot_size, 0));
	require(perform_one_test(cns, cns->shot, z_info->quiver_slot_size, 0));
	require(perform_one_test(cns, cns->flask, z_info->quiver_slot_size, 0));
	require(perform_one_test(cns, cns->inscribed_flask,
		z_info->quiver_slot_size, 0));
	ok;
}

const char *suite_name = "player/inven-carry-num";
struct test tests[] = {
	{ "carry num empty/empty", test_carry_num_empty_pack_empty_quiver },
	{ "carry num partial/empty", test_carry_num_partial_pack_empty_quiver },
	{ "carry num full/empty", test_carry_num_full_pack_empty_quiver },
	{ "carry num empty/partial", test_carry_num_empty_pack_partial_quiver },
	{ "carry num partial/partial", test_carry_num_partial_pack_partial_quiver },
	{ "carry num full/partial", test_carry_num_full_pack_partial_quiver },
	{ "carry num empty/full", test_carry_num_empty_pack_full_quiver },
	{ "carry num partial/full", test_carry_num_partial_pack_full_quiver },
	{ "carry num full/full", test_carry_num_full_pack_full_quiver },
	{ NULL, NULL }
};

