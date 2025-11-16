/* player/digging.c */
/* Exercise aspects of players trying to tunnel. */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "cmd-core.h"
#include "game-world.h"
#include "init.h"
#include "mon-make.h"
#include "object.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player.h"
#include "player-birth.h"
#include "player-path.h"
#include "player-util.h"


static struct object *setup_object(int tval, int sval, int num) {
	struct object_kind *kind = lookup_kind(tval, sval);
	struct object *obj = NULL;

	if (kind) {
		obj = object_new();
		object_prep(obj, kind, 0, MINIMISE);
		obj->number = num;
		obj->known = object_new();
		object_set_base_known(player, obj);
		object_touch(player, obj);
	}
	return obj;
}


int setup_tests(void **state) {
	struct player_class *caster_class = NULL, *cursor_class;
	struct object *weapon, *digger;

	set_file_paths();
	init_angband();
#ifdef UNIX
	/* Necessary for creating the randart file. */
	create_needed_dirs();
#endif

	/*
	 * Set up the player with a class that allows spells at the start.
	 * Do not use a class with the COMBAT_REGEN player flag.
	 */
	for (cursor_class = classes; cursor_class;
			cursor_class = cursor_class->next) {
		if (!pf_has(cursor_class->pflags, PF_COMBAT_REGEN)
				&& cursor_class->magic.spell_first == 1
				&& cursor_class->magic.books) {
			caster_class = cursor_class;
			break;
		}
	}
	if (!player_make_simple(NULL,
			(caster_class) ? caster_class->name : NULL, "Tester")) {
		cleanup_angband();
		return 1;
	}

	/*
	 * Set up a weapon that will positively affect hit points and mana.
	 * Then set up a weapon that will be better for digging than that
	 * weapon but will not affect hit points or mana.
	 */
	weapon = setup_object(TV_SWORD, 1, 1);
	if (!weapon) {
		cleanup_angband();
		return 1;
	}
	if (weapon->to_h < 8) {
		weapon->to_h = 8;
	}
	if (weapon->to_d < 8) {
		weapon->to_d = 8;
	}
	weapon->modifiers[OBJ_MOD_STR] = 0;
	weapon->modifiers[OBJ_MOD_INT] = 8;
	weapon->modifiers[OBJ_MOD_WIS] = 8;
	weapon->modifiers[OBJ_MOD_CON] = 8;
	weapon->modifiers[OBJ_MOD_TUNNEL] = 0;

	digger = setup_object(TV_DIGGING, 1, 1);
	if (!digger) {
		if (weapon->known) {
			object_free(weapon->known);
		}
		object_free(weapon);
		cleanup_angband();
		return 1;
	}
	digger->to_h = 0;
	digger->to_d = 0;
	digger->modifiers[OBJ_MOD_STR] = 4;
	digger->modifiers[OBJ_MOD_INT] = 0;
	digger->modifiers[OBJ_MOD_WIS] = 0;
	digger->modifiers[OBJ_MOD_CON] = 0;
	digger->modifiers[OBJ_MOD_TUNNEL] = 8;

	/*
	 * Equip the player with a weapon that will affect hit points and mana.
	 * Put the digger in the pack.
	 */
	if (inven_carry_okay(weapon)) {
		int slot = wield_slot(weapon);

		if (!slot_object(player, slot)) {
			++player->upkeep->equip_cnt;
		}

		player->body.slots[slot].obj = weapon;
		object_learn_on_wield(player, weapon);
		pile_insert_end(&player->gear, weapon);
		pile_insert_end(&player->gear_k, weapon->known);
	} else {
		if (digger->known) {
			object_free(digger->known);
		}
		object_free(digger);
		if (weapon->known) {
			object_free(weapon->known);
		}
		object_free(weapon);
		cleanup_angband();
		return 1;
	}
	if (inven_carry_okay(digger)) {
		inven_carry(player, digger, true, false);
	} else {
		object_free(digger);
		if (weapon->known) {
			object_free(weapon->known);
		}
		cleanup_angband();
		return 1;
	}

	return 0;
}


int teardown_tests(void *state) {
	cleanup_angband();
	return 0;
}


/*
 * Check that autowapping for a digger in the pack and swapping back does not
 * have extra side effects on the player's state (namely the amount of current
 * hit and spell points).
 */
static int test_autoswap_side_effects(void *state) {
	int16_t old_chp, old_csp;

	if (player->cave) {
		cave_free(player->cave);
	}
	if (cave) {
		wipe_mon_list(cave, player);
		cave_free(cave);
	}
	cave = t_build_arena(9, 9);

	/* Give the player something difficult to dig. */
	square_set_feat(cave, loc(4, 4), FEAT_GRANITE);

	/* Put the player in the cave. */
	player_place(cave, player, loc(5, 4));
	player->cave = cave_new(cave->height, cave->width);
	player->cave->depth = cave->depth;
	player->cave->objects = mem_zalloc((cave->obj_max + 1)
		* sizeof(struct object*));
	player->cave->obj_max = cave->obj_max;
	cave_illuminate(cave, true);
	character_dungeon = true;
	on_new_level();

	/* Rest until hit points and mana are fully recovered. */
	player->chp = player->mhp;
	player->chp_frac = 0;
	player->csp = player->msp;
	player->csp_frac = 0;
	old_chp = player->chp;
	old_csp = player->csp;

	/*
	 * Dig out the granite.  That should autoswap to what is in the pack,
	 * and then swap back. There should be no effect on hit points or mana.
	 */
	cmdq_push(CMD_TUNNEL);
	cmd_set_arg_direction(cmdq_peek(), "direction", 4);
	cmdq_execute(CTX_GAME);
	eq(player->chp, old_chp);
	eq(player->chp_frac, 0);
	eq(player->csp, old_csp);
	eq(player->csp_frac, 0);

	/*
	 * Try pathfinding which will autoswap to determine the time cost
	 * to deal with rubble and then swap back.  There should be no effect
	 * on hit points or mana.
	 */
	(void)find_path(player, player->grid, loc(7, 7), NULL);
	eq(player->chp, old_chp);
	eq(player->chp_frac, 0);
	eq(player->csp, old_csp);
	eq(player->csp_frac, 0);

	if (player->cave) {
		cave_free(player->cave);
		player->cave = NULL;
	}
	wipe_mon_list(cave, player);
	cave_free(cave);
	cave = NULL;
	character_dungeon = false;

	ok;
}


const char *suite_name = "player/digging";
struct test tests[] = {
	{ "autoswap side effects", test_autoswap_side_effects },
	{ NULL, NULL }
};
