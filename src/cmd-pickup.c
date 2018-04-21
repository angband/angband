/**
 * \file cmd-pickup.c
 * \brief Pickup code
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke,
 * Copyright (c) 2007 Leon Marrick
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "cmds.h"
#include "game-event.h"
#include "game-input.h"
#include "generate.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-util.h"
#include "trap.h"

/**
 * Pick up all gold at the player's current location.
 */
static void player_pickup_gold(void)
{
	s32b total_gold = 0L;
	char name[30] = "";

	struct object *obj = square_object(cave, player->py, player->px), *next;

	int sound_msg;
	bool verbal = FALSE;
	bool at_most_one = TRUE;

	/* Pick up all the ordinary gold objects */
	while (obj) {
		struct object_kind *kind = NULL;

		/* Get next object */
		next = obj->next;

		/* Ignore if not legal treasure */
		kind = lookup_kind(obj->tval, obj->sval);
		if (!tval_is_money(obj) || !kind) {
			obj = next;
			continue;
		}

		/* Multiple types if we have a second name, otherwise record the name */
		if (total_gold && !streq(kind->name, name))
			at_most_one = FALSE;
		else
			my_strcpy(name, kind->name, sizeof(name));

		/* Remember whether feedback message is in order */
		if (!ignore_item_ok(obj))
			verbal = TRUE;

		/* Increment total value */
		total_gold += (s32b)obj->pval;

		/* Delete the gold */
		square_excise_object(cave, player->py, player->px, obj);
		object_delete(&obj);
		obj = next;
	}

	/* Pick up the gold, if present */
	if (total_gold) {
		char buf[100];

		/* Build a message */
		(void)strnfmt(buf, sizeof(buf),
					  "You have found %d gold pieces worth of ", total_gold);

		/* One treasure type.. */
		if (at_most_one)
			my_strcat(buf, name, sizeof(buf));
		/* ... or more */
		else
			my_strcat(buf, "treasures", sizeof(buf));
		my_strcat(buf, ".", sizeof(buf));

		/* Determine which sound to play */
		if      (total_gold < 200) sound_msg = MSG_MONEY1;
		else if (total_gold < 600) sound_msg = MSG_MONEY2;
		else                       sound_msg = MSG_MONEY3;

		/* Display the message */
		if (verbal)
			msgt(sound_msg, "%s", buf);

		/* Add gold to purse */
		player->au += total_gold;

		/* Redraw gold */
		player->upkeep->redraw |= (PR_GOLD);
	}
}



/**
 * Determine if an object can be picked up automatically.
 */
static bool auto_pickup_okay(const struct object *obj)
{
	if (!inven_carry_okay(obj)) return FALSE;
	if (OPT(pickup_always) || check_for_inscrip(obj, "=g")) return TRUE;
	if (OPT(pickup_inven) && inven_stack_okay(obj)) return TRUE;

	return FALSE;
}


/**
 * Move an object from a floor pile to the player's gear
 */
static void player_pickup_aux(struct object *obj, bool domsg)
{
	/* Confirm the object can be picked up*/
	if (!inven_carry_okay(obj))
		quit_fmt("Failed pickup of %s", obj->kind->name);

	/* Set ignore status */
	player->upkeep->notice |= PN_IGNORE;

	/* Automatically sense artifacts */
	object_sense_artifact(obj);

	/* Log artifacts if found */
	if (obj->artifact)
		history_add_artifact(obj->artifact, object_is_known(obj), TRUE);

	/* Carry the object */
	square_excise_object(cave, player->py, player->px, obj);
	inven_carry(player, obj, TRUE, domsg);
}

/**
 * Pick up objects and treasure on the floor.  -LM-
 *
 * Scan the list of objects in that floor grid. Pick up gold automatically.
 * Pick up objects automatically until backpack space is full if
 * auto-pickup option is on, otherwise, store objects on
 * floor in an array, and tally both how many there are and can be picked up.
 *
 * If not picking up anything, indicate objects on the floor.  Do the same
 * thing if we don't have room for anything.
 *
 * Pick up multiple objects using Tim Baker's menu system.   Recursively
 * call this function (forcing menus for any number of objects) until
 * objects are gone, backpack is full, or player is satisfied.
 *
 * We keep track of number of objects picked up to calculate time spent.
 * This tally is incremented even for automatic pickup, so we are careful
 * (in "dungeon.c" and elsewhere) to handle pickup as either a separate
 * automated move or a no-cost part of the stay still or 'g'et command.
 *
 * Note the lack of chance for the character to be disturbed by unmarked
 * objects.  They are truly "unknown".
 *
 * \param obj is the object to pick up.
 * \param menu is whether to present a menu to the player
 */
static byte player_pickup_item(bool menu)
{
	int py = player->py;
	int px = player->px;

	struct object *current = NULL;

	int floor_max = z_info->floor_size + 1;
	struct object **floor_list = mem_zalloc(floor_max * sizeof(*floor_list));
	int floor_num = 0;

	int i;
	int can_pickup = 0;
	bool call_function_again = FALSE;

	bool domsg = TRUE;

	/* Objects picked up.  Used to determine time cost of command. */
	byte objs_picked_up = 0;

	/* Always pickup gold, effortlessly */
	player_pickup_gold();

	/* Nothing else to pick up -- return */
	if (!square_object(cave, py, px)) {
		mem_free(floor_list);
		return objs_picked_up;
	}

	/* Tally objects that can be picked up.*/
	floor_num = scan_floor(floor_list, floor_max, py, px, 0x08, NULL);
	for (i = 0; i < floor_num; i++)
	    if (inven_carry_okay(floor_list[i]))
			can_pickup++;
	
	if (!can_pickup) {
	    /* Can't pick up, but probably want to know what's there. */
	    event_signal(EVENT_SEEFLOOR);
		mem_free(floor_list);
	    return objs_picked_up;
	}

	/* Use a menu interface for multiple objects, or pickup single objects */
	if (!menu && !current) {
		if (floor_num > 1)
			menu = TRUE;
		else
			current = floor_list[0];
	}

	/* Display a list if requested. */
	if (menu && !current) {
		const char *q, *s;
		struct object *obj = NULL;

		/* Get an object or exit. */
		q = "Get which item?";
		s = "You see nothing there.";
		if (!get_item(&obj, q, s, CMD_PICKUP, inven_carry_okay, USE_FLOOR)) {
			mem_free(floor_list);
			return (objs_picked_up);
		}

		current = obj;
		call_function_again = TRUE;

		/* With a list, we do not need explicit pickup messages */
		domsg = TRUE;
	}

	/* Pick up object, if legal */
	if (current) {
		/* Pick up the object */
		player_pickup_aux(current, domsg);

		/* Indicate an object picked up. */
		objs_picked_up = 1;
	}

	/*
	 * If requested, call this function recursively.  Count objects picked
	 * up.  Force the display of a menu in all cases.
	 */
	if (call_function_again)
		objs_picked_up += player_pickup_item(TRUE);

	mem_free(floor_list);

	/* Indicate how many objects have been picked up. */
	return (objs_picked_up);
}

/**
 * Pick up everything on the floor that requires no player action
 */
int do_autopickup(void)
{
	int py = player->py;
	int px = player->px;

	struct object *obj, *next;

	/* Objects picked up.  Used to determine time cost of command. */
	byte objs_picked_up = 0;

	/* Nothing to pick up -- return */
	if (!square_object(cave, py, px))
		return 0;

	/* Always pickup gold, effortlessly */
	player_pickup_gold();

	/* Scan the remaining objects */
	obj = square_object(cave, py, px);
	while (obj) {
		next = obj->next;

		/* Ignore all hidden objects and non-objects */
		if (!ignore_item_ok(obj)) {
			/* Hack -- disturb */
			disturb(player, 0);

			/* Automatically pick up items into the backpack */
			if (auto_pickup_okay(obj)) {
				/* Pick up the object with message */
				player_pickup_aux(obj, TRUE);
				objs_picked_up++;
			}
		}
		obj = next;
	}

	return objs_picked_up;
}

/**
 * Pick up objects at the player's request
 */
void do_cmd_pickup(struct command *cmd)
{
	int energy_cost = 0;

	/* Pick up floor objects with a menu for multiple objects */
	energy_cost += player_pickup_item(FALSE) * z_info->move_energy / 10;

	/* Limit */
	if (energy_cost > z_info->move_energy) energy_cost = z_info->move_energy;

	/* Charge this amount of energy. */
	player->upkeep->energy_use = energy_cost;
}

/**
 * Pick up or look at objects on a square when the player steps onto it
 */
void do_cmd_autopickup(struct command *cmd)
{
	/* Get the obvious things */
	player->upkeep->energy_use = do_autopickup() * z_info->move_energy / 10;
	if (player->upkeep->energy_use > z_info->move_energy)
		player->upkeep->energy_use = z_info->move_energy;

	/* Look at what's left */
	event_signal(EVENT_SEEFLOOR);
}
