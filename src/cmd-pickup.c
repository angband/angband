/*
 * File: cmd-pickup.c
 * Purpose: Pickup code
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
#include "generate.h"
#include "history.h"
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
#include "obj-ui.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-util.h"
#include "tables.h"
#include "trap.h"



/*
 * Pick up objects on the floor beneath you.  -LM-
 */
void do_cmd_pickup(struct command *cmd)
{
	int energy_cost;
	struct object *obj = square_object(cave, player->py, player->px);

	/* Autopickup first */
	energy_cost = do_autopickup() * 10;

	/* Pick up floor objects with a menu for multiple objects */
	energy_cost += py_pickup_item(1, obj) * 10;

	/* Limit */
	if (energy_cost > 100) energy_cost = 100;

	/* Charge this amount of energy. */
	player->upkeep->energy_use = energy_cost;
}

/*
 * Pick up objects on the floor beneath you.  -LM-
 */
void do_cmd_autopickup(struct command *cmd)
{
	player->upkeep->energy_use = do_autopickup() * 10;
}



/*
 * Pickup all gold at the player's current location.
 */
static void py_pickup_gold(void)
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
		object_delete(obj);
		obj = next;
	}

	/* Pick up the gold, if present */
	if (total_gold) {
		char buf[100];

		/* Build a message */
		(void)strnfmt(buf, sizeof(buf),
					  "You have found %ld gold pieces worth of ",
					  (long)total_gold);

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



/*
 * Determine if the object can be picked up automatically.
 */
static bool auto_pickup_okay(const object_type *o_ptr)
{
	if (!inven_carry_okay(o_ptr)) return FALSE;
	if (OPT(pickup_always) || check_for_inscrip(o_ptr, "=g")) return TRUE;
	if (OPT(pickup_inven) && inven_stack_okay(o_ptr)) return TRUE;

	return FALSE;
}


/*
 * Carry an object and delete it.
 */
static void py_pickup_aux(struct object *obj, bool domsg)
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
	inven_carry(player, obj, domsg);
}

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
	py_pickup_gold();

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
				py_pickup_aux(obj, TRUE);
				objs_picked_up++;
			}
		}
		obj = next;
	}

	return objs_picked_up;
}

/**
 * Pick up objects and treasure on the floor.  -LM-
 *
 * Called with pickup:
 * 0 to act according to the player's settings
 * 1 to quickly pickup single objects or present a menu for more
 * 2 to force a menu for any number of objects
 *
 * Scan the list of objects in that floor grid. Pick up gold automatically.
 * Pick up objects automatically until backpack space is full if
 * auto-pickup option is on, Otherwise, store objects on
 * floor in an array, and tally both how many there are and can be picked up.
 *
 * If not picking up anything, indicate objects on the floor.  Show more
 * details if the "OPT(pickup_detail)" option is set.  Do the same thing if we
 * don't have room for anything.
 *
 * [This paragraph is not true, intentional?]
 * If we are picking up objects automatically, and have room for at least
 * one, allow the "OPT(pickup_detail)" option to display information about
 * objects and prompt the player.  Otherwise, automatically pick up a single
 * object or use a menu for more than one.
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
 * \param item is the floor item index (must be negative) to pick up.
 */
byte py_pickup_item(int pickup, struct object *obj)
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
	py_pickup_gold();

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

	/* Use the item that we are given, if it is on the floor. */
	if (square_holds_object(cave, py, px, obj))
		current = obj;

	/* Use a menu interface for multiple objects, or pickup single objects */
	if (pickup == 1 && !current) {
		if (floor_num > 1)
			pickup = 2;
		else
			current = floor_list[0];
	}

	/* Display a list if requested. */
	if (pickup == 2 && !current) {
		const char *q, *s;
		struct object *obj1;

		/* Get an object or exit. */
		q = "Get which item?";
		s = "You see nothing there.";
		if (!get_item(&obj1, q, s, CMD_PICKUP, inven_carry_okay, USE_FLOOR)) {
			mem_free(floor_list);
			return (objs_picked_up);
		}

		current = obj1;
		call_function_again = TRUE;

		/* With a list, we do not need explicit pickup messages */
		domsg = FALSE;
	}

	/* Pick up object, if legal */
	if (current) {
		/* Pick up the object */
		py_pickup_aux(current, domsg);

		/* Indicate an object picked up. */
		objs_picked_up = 1;
	}

	/*
	 * If requested, call this function recursively.  Count objects picked
	 * up.  Force the display of a menu in all cases.
	 */
	if (call_function_again) objs_picked_up += py_pickup(2);

	mem_free(floor_list);

	/* Indicate how many objects have been picked up. */
	return (objs_picked_up);
}

byte py_pickup(int pickup)
{
	return py_pickup_item(pickup, NULL);
}

/*
 * Move player in the given direction.
 *
 * This routine should only be called when energy has been expended.
 *
 * Note that this routine handles monsters in the destination grid,
 * and also handles attempting to move into walls/doors/rubble/etc.
 */
void move_player(int dir, bool disarm)
{
	int py = player->py;
	int px = player->px;

	int y = py + ddy[dir];
	int x = px + ddx[dir];

	int m_idx = cave->squares[y][x].mon;
	struct monster *m_ptr = cave_monster(cave, m_idx);

	/* Attack monsters */
	if (m_idx > 0) {
		/* Mimics surprise the player */
		if (is_mimicking(m_ptr)) {
			become_aware(m_ptr);

			/* Mimic wakes up */
			mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);

		} else {
			py_attack(y, x);
		}
	}

	/* Optionally alter traps/doors on movement */
	else if (disarm && square_ismark(cave, y, x) &&
			(square_isknowntrap(cave, y, x) || square_iscloseddoor(cave, y, x)))
	{
		/* Auto-repeat if not already repeating */
		if (cmd_get_nrepeats() == 0)
			cmd_set_repeat(99);

		do_cmd_alter_aux(dir);
	}

	/* Cannot walk through walls */
	else if (!square_ispassable(cave, y, x))
	{
		/* Disturb the player */
		disturb(player, 0);

		/* Notice unknown obstacles */
		if (!square_ismark(cave, y, x))
		{
			/* Rubble */
			if (square_isrubble(cave, y, x))
			{
				msgt(MSG_HITWALL, "You feel a pile of rubble blocking your way.");
				sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);
				square_light_spot(cave, y, x);
			}

			/* Closed door */
			else if (square_iscloseddoor(cave, y, x))
			{
				msgt(MSG_HITWALL, "You feel a door blocking your way.");
				sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);
				square_light_spot(cave, y, x);
			}

			/* Wall (or secret door) */
			else
			{
				msgt(MSG_HITWALL, "You feel a wall blocking your way.");
				sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);
				square_light_spot(cave, y, x);
			}
		}

		/* Mention known obstacles */
		else
		{
			if (square_isrubble(cave, y, x))
				msgt(MSG_HITWALL, "There is a pile of rubble blocking your way.");
			else if (square_iscloseddoor(cave, y, x))
				msgt(MSG_HITWALL, "There is a door blocking your way.");
			else
				msgt(MSG_HITWALL, "There is a wall blocking your way.");
		}
	}

	/* Normal movement */
	else
	{
		/* See if trap detection status will change */
		bool old_dtrap = square_isdtrap(cave, py, px);
		bool new_dtrap = square_isdtrap(cave, y, x);

		/* Note the change in the detect status */
		if (old_dtrap != new_dtrap)
			player->upkeep->redraw |= (PR_DTRAP);

		/* Disturb player if the player is about to leave the area */
		if (player->upkeep->running && !player->upkeep->running_firststep && 
			old_dtrap && !new_dtrap)
		{
			disturb(player, 0);
			return;
		}

		/* Move player */
		monster_swap(py, px, y, x);
  
		/* New location */
		y = py = player->py;
		x = px = player->px;

		/* Searching */
		if (player->searching ||
				(player->state.skills[SKILL_SEARCH_FREQUENCY] >= 50) ||
				one_in_(50 - player->state.skills[SKILL_SEARCH_FREQUENCY]))
			search(FALSE);

		/* Handle "store doors" */
		if (square_isshop(cave, player->py, player->px)) {
			/* Disturb */
			disturb(player, 0);
			cmdq_push(CMD_ENTER_STORE);
		}

		/* All other grids (including traps) */
		else
		{
			/* Handle objects (later) */
			player->upkeep->notice |= (PN_PICKUP);
		}


		/* Discover invisible traps */
		if (square_issecrettrap(cave, y, x))
		{
			/* Disturb */
			disturb(player, 0);
			
			/* Hit the trap. */
			hit_trap(y, x);
		}

		/* Set off a visible trap */
		else if (square_isknowntrap(cave, y, x))
		{
			/* Disturb */
			disturb(player, 0);

			/* Hit the trap */
			hit_trap(y, x);
		}
	}

	player->upkeep->running_firststep = FALSE;
}
