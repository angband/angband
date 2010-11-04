/*
 * File: cmd1.c
 * Purpose: Searching, movement, and pickup
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
#include "attack.h"
#include "cave.h"
#include "cmds.h"
#include "history.h"
#include "monster/monster.h"
#include "object/inventory.h"
#include "object/tvalsval.h"
#include "squelch.h"
#include "trap.h"

/*
 * Search for hidden things.  Returns true if a search was attempted, returns
 * false when the player has a 0% chance of finding anything.  Prints messages
 * for negative confirmation when verbose mode is requested.
 */
bool search(bool verbose)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x, chance;

	bool found = FALSE;

	object_type *o_ptr;


	/* Start with base search ability */
	chance = p_ptr->state.skills[SKILL_SEARCH];

	/* Penalize various conditions */
	if (p_ptr->timed[TMD_BLIND] || no_light()) chance = chance / 10;
	if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE]) chance = chance / 10;

	/* Prevent fruitless searches */
	if (chance <= 0)
	{
		if (verbose)
		{
			msg_print("You can't make out your surroundings well enough to search.");

			/* Cancel repeat */
			disturb(0, 0);
		}

		return FALSE;
	}

	/* Search the nearby grids, which are always in bounds */
	for (y = (py - 1); y <= (py + 1); y++)
	{
		for (x = (px - 1); x <= (px + 1); x++)
		{
			/* Sometimes, notice things */
			if (randint0(100) < chance)
			{
				/* Invisible trap */
				if (cave_feat[y][x] == FEAT_INVIS)
				{
					found = TRUE;

					/* Pick a trap */
					pick_trap(y, x);

					/* Message */
					msg_print("You have found a trap.");

					/* Disturb */
					disturb(0, 0);
				}

				/* Secret door */
				if (cave_feat[y][x] == FEAT_SECRET)
				{
					found = TRUE;

					/* Message */
					msg_print("You have found a secret door.");

					/* Pick a door */
					place_closed_door(y, x);

					/* Disturb */
					disturb(0, 0);
				}

				/* Scan all objects in the grid */
				for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
				{
					/* Skip non-chests */
					if (o_ptr->tval != TV_CHEST) continue;

					/* Skip disarmed chests */
					if (o_ptr->pval <= 0) continue;

					/* Skip non-trapped chests */
					if (!chest_traps[o_ptr->pval]) continue;

					/* Identify once */
					if (!object_is_known(o_ptr))
					{
						found = TRUE;

						/* Message */
						msg_print("You have discovered a trap on the chest!");

						/* Know the trap */
						object_notice_everything(o_ptr);

						/* Notice it */
						disturb(0, 0);
					}
				}
			}
		}
	}

	if (verbose && !found)
	{
		if (chance >= 100)
			msg_print("There are no secrets here.");
		else
			msg_print("You found nothing.");
	}

	return TRUE;
}


/*** Pickup ***/

/*
 * Pickup all gold at the player's current location.
 */
static void py_pickup_gold(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	s32b total_gold = 0L;
	byte *treasure;

	s16b this_o_idx, next_o_idx = 0;

	object_type *o_ptr;

	int sound_msg;
	bool verbal = FALSE;

	/* Allocate an array of ordinary gold objects */
	treasure = C_ZNEW(SV_GOLD_MAX, byte);


	/* Pick up all the ordinary gold objects */
	for (this_o_idx = cave_o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Ignore if not legal treasure */
		if ((o_ptr->tval != TV_GOLD) ||
		    (o_ptr->sval >= SV_GOLD_MAX)) continue;

		/* Note that we have this kind of treasure */
		treasure[o_ptr->sval]++;

		/* Remember whether feedback message is in order */
		if (!squelch_item_ok(o_ptr))
			verbal = TRUE;

		/* Increment total value */
		total_gold += (s32b)o_ptr->pval;

		/* Delete the gold */
		delete_object_idx(this_o_idx);
	}

	/* Pick up the gold, if present */
	if (total_gold)
	{
		char buf[1024];
		char tmp[80];
		int i, count, total, k_idx;

		/* Build a message */
		(void)strnfmt(buf, sizeof(buf), "You have found %ld gold pieces worth of ", (long)total_gold);

		/* Count the types of treasure present */
		for (total = 0, i = 0; i < SV_GOLD_MAX; i++)
		{
			if (treasure[i]) total++;
		}

		/* List the treasure types */
		for (count = 0, i = 0; i < SV_GOLD_MAX; i++)
		{
			/* Skip if no treasure of this type */
			if (!treasure[i]) continue;

			/* Get this object index */
			k_idx = lookup_kind(TV_GOLD, i);

			/* Skip past errors  XXX */
			if (k_idx <= 0) continue;

			/* Get the object name */
			object_kind_name(tmp, sizeof tmp, k_idx, TRUE);

			/* Build up the pickup string */
			my_strcat(buf, tmp, sizeof(buf));

			/* Added another kind of treasure */
			count++;

			/* Add a comma if necessary */
			if ((total > 2) && (count < total)) my_strcat(buf, ",", sizeof(buf));

			/* Add an "and" if necessary */
			if ((total >= 2) && (count == total-1)) my_strcat(buf, " and", sizeof(buf));

			/* Add a space or period if necessary */
			if (count < total) my_strcat(buf, " ", sizeof(buf));
			else               my_strcat(buf, ".", sizeof(buf));
		}

		/* Determine which sound to play */
		if      (total_gold < 200) sound_msg = MSG_MONEY1;
		else if (total_gold < 600) sound_msg = MSG_MONEY2;
		else                       sound_msg = MSG_MONEY3;

		/* Display the message */
		if (verbal)
			message(sound_msg, 0, buf);

		/* Add gold to purse */
		p_ptr->au += total_gold;

		/* Redraw gold */
		p_ptr->redraw |= (PR_GOLD);
	}

	/* Free the gold array */
	FREE(treasure);
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
static void py_pickup_aux(int o_idx, bool msg)
{
	int slot, quiver_slot = 0;

	char o_name[80];
	object_type *o_ptr = &o_list[o_idx];

	/* Carry the object */
	slot = inven_carry(p_ptr, o_ptr);

	/* Handle errors (paranoia) */
	if (slot < 0) return;

	/* If we have picked up ammo which matches something in the quiver, note
	 * that it so that we can wield it later (and suppress pick up message) */
	if (obj_is_ammo(o_ptr)) 
	{
		int i;
		for (i = QUIVER_START; i < QUIVER_END; i++) 
		{
			if (!p_ptr->inventory[i].k_idx) continue;
			if (!object_similar(&p_ptr->inventory[i], o_ptr)) continue;
			quiver_slot = i;
			break;
		}
	}

	/* Get the new object */
	o_ptr = &p_ptr->inventory[slot];

	/* Set squelch status */
	p_ptr->notice |= PN_SQUELCH;

	/* Automatically sense artifacts */
	object_sense_artifact(o_ptr);

	/* Log artifacts if found */
	if (artifact_p(o_ptr))
		history_add_artifact(o_ptr->name1, object_is_known(o_ptr), TRUE);

	/* Optionally, display a message */
	if (msg && !quiver_slot)
	{
		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Message */
		msg_format("You have %s (%c).", o_name, index_to_label(slot));
	}


	/* Delete the object */
	delete_object_idx(o_idx);

	/* If we have a quiver slot that this ammo matches, use it */
	if (quiver_slot) wield_item(o_ptr, slot, quiver_slot);
}


/*
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
 * one, allow the "OPT(pickup_detail)" option to display information about objects
 * and prompt the player.  Otherwise, automatically pick up a single object
 * or use a menu for more than one.
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
 */
byte py_pickup(int pickup)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	char o_name[80];

	s16b this_o_idx, next_o_idx = 0;

	object_type *o_ptr;

	/* Objects picked up.  Used to determine time cost of command. */
	byte objs_picked_up = 0;

	size_t floor_num = 0;
	int floor_list[MAX_FLOOR_STACK + 1], floor_o_idx = 0;

	int can_pickup = 0;
	bool call_function_again = FALSE;

	bool blind = ((p_ptr->timed[TMD_BLIND]) || (no_light()));
	bool msg = TRUE;


	/* Nothing to pick up -- return */
	if (!cave_o_idx[py][px]) return (0);


	/* Always pickup gold, effortlessly */
	py_pickup_gold();


	/* Scan the remaining objects */
	for (this_o_idx = cave_o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the object and the next object */
		o_ptr = &o_list[this_o_idx];
		next_o_idx = o_ptr->next_o_idx;

		/* Ignore all hidden objects and non-objects */
		if (squelch_hide_item(o_ptr) || !o_ptr->k_idx) continue;

		/* XXX Hack -- Enforce limit */
		if (floor_num >= N_ELEMENTS(floor_list)) break;


		/* Hack -- disturb */
		disturb(0, 0);


		/* Automatically pick up items into the backpack */
		if (auto_pickup_okay(o_ptr))
		{
			/* Pick up the object with message */
			py_pickup_aux(this_o_idx, TRUE);
			objs_picked_up++;

			continue;
		}


		/* Tally objects and store them in an array. */

		/* Remember this object index */
		floor_list[floor_num] = this_o_idx;

		/* Count non-gold objects that remain on the floor. */
		floor_num++;

		/* Tally objects that can be picked up.*/
		if (inven_carry_okay(o_ptr))
			can_pickup++;
	}

	/* There are no objects left */
	if (!floor_num)
		return objs_picked_up;


	/* Get hold of the last floor index */
	floor_o_idx = floor_list[floor_num - 1];



	/* Mention the objects if player is not picking them up. */
	if (pickup == 0 || !can_pickup)
	{
		const char *p = "see";

		/* One object */
		if (floor_num == 1)
		{
			if (!can_pickup)	p = "have no room for";
			else if (blind)     p = "feel";

			/* Get the object */
			o_ptr = &o_list[floor_o_idx];

			/* Describe the object.  Less detail if blind. */
			if (blind)
				object_desc(o_name, sizeof(o_name), o_ptr,
							ODESC_PREFIX | ODESC_BASE);
			else
				object_desc(o_name, sizeof(o_name), o_ptr,
							ODESC_PREFIX | ODESC_FULL);

			/* Message */
			message_flush();
			msg_format("You %s %s.", p, o_name);
		}
		else
		{
			/* Optionally, display more information about floor items */
			if (OPT(pickup_detail))
			{
				if (!can_pickup)	p = "have no room for the following objects";
				else if (blind)     p = "feel something on the floor";

				/* Scan all marked objects in the grid */
				floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), py, px, 0x03);

				/* Save screen */
				screen_save();

				/* Display objects on the floor */
				show_floor(floor_list, floor_num, (OLIST_WEIGHT));

				/* Display prompt */
				prt(format("You %s: ", p), 0, 0);

				/* Move cursor back to character, if needed */
				if (OPT(highlight_player)) move_cursor_relative(p_ptr->py, p_ptr->px);

				/* Wait for it.  Use key as next command. */
				p_ptr->command_new = inkey();

				/* Restore screen */
				screen_load();
			}

			/* Show less detail */
			else
			{
				message_flush();

				if (!can_pickup)
					msg_print("You have no room for any of the items on the floor.");
				else
					msg_format("You %s a pile of %d items.", (blind ? "feel" : "see"), floor_num);
			}
		}

		/* Done */
		return (objs_picked_up);
	}


	/* We can pick up objects.  Menus are not requested (yet). */
	if (pickup == 1)
	{
		/* Scan floor (again) */
		floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), py, px, 0x03);

		/* Use a menu interface for multiple objects, or pickup single objects */
		if (floor_num > 1)
			pickup = 2;
		else
			this_o_idx = floor_o_idx;
	}


	/* Display a list if requested. */
	if (pickup == 2)
	{
		cptr q, s;
		int item;

		/* Restrict the choices */
		item_tester_hook = inven_carry_okay;

		/* Get an object or exit. */
		q = "Get which item?";
		s = "You see nothing there.";
		if (!get_item(&item, q, s, USE_FLOOR))
			return (objs_picked_up);

		this_o_idx = 0 - item;
		call_function_again = TRUE;

		/* With a list, we do not need explicit pickup messages */
		msg = FALSE;
	}

	/* Pick up object, if legal */
	if (this_o_idx)
	{
		/* Pick up the object */
		py_pickup_aux(this_o_idx, msg);

		/* Indicate an object picked up. */
		objs_picked_up = 1;
	}

	/*
	 * If requested, call this function recursively.  Count objects picked
	 * up.  Force the display of a menu in all cases.
	 */
	if (call_function_again) objs_picked_up += py_pickup(2);

	/* Indicate how many objects have been picked up. */
	return (objs_picked_up);
}



/*
 * Move player in the given direction.
 *
 * This routine should only be called when energy has been expended.
 *
 * Note that this routine handles monsters in the destination grid,
 * and also handles attempting to move into walls/doors/rubble/etc.
 */
void move_player(int dir)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;


	bool old_dtrap, new_dtrap;


	/* Find the result of moving */
	y = py + ddy[dir];
	x = px + ddx[dir];


	/* Attack monsters */
	if (cave_m_idx[y][x] > 0)
		py_attack(y, x);

	/* Optionally alter known traps/doors on movement */
	else if (OPT(easy_alter) && (cave_info[y][x] & CAVE_MARK) &&
			(cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
			(cave_feat[y][x] <= FEAT_DOOR_TAIL))
	{
		/* Auto-repeat if not already repeating */
		if (cmd_get_nrepeats() == 0)
			cmd_set_repeat(99);

		do_cmd_alter_aux(dir);
	}

	/* Cannot walk through walls */
	else if (!cave_floor_bold(y, x))
	{
		/* Disturb the player */
		disturb(0, 0);

		/* Notice unknown obstacles */
		if (!(cave_info[y][x] & CAVE_MARK))
		{
			/* Rubble */
			if (cave_feat[y][x] == FEAT_RUBBLE)
			{
				message(MSG_HITWALL, 0, "You feel a pile of rubble blocking your way.");
				cave_info[y][x] |= (CAVE_MARK);
				light_spot(y, x);
			}

			/* Closed door */
			else if (cave_feat[y][x] < FEAT_SECRET)
			{
				message(MSG_HITWALL, 0, "You feel a door blocking your way.");
				cave_info[y][x] |= (CAVE_MARK);
				light_spot(y, x);
			}

			/* Wall (or secret door) */
			else
			{
				message(MSG_HITWALL, 0, "You feel a wall blocking your way.");
				cave_info[y][x] |= (CAVE_MARK);
				light_spot(y, x);
			}
		}

		/* Mention known obstacles */
		else
		{
			if (cave_feat[y][x] == FEAT_RUBBLE)
				message(MSG_HITWALL, 0, "There is a pile of rubble blocking your way.");
			else if (cave_feat[y][x] < FEAT_SECRET)
				message(MSG_HITWALL, 0, "There is a door blocking your way.");
			else
				message(MSG_HITWALL, 0, "There is a wall blocking your way.");
		}
	}

	/* Normal movement */
	else
	{
		/* Sound XXX XXX XXX */
		/* sound(MSG_WALK); */

		/* See if trap detection status will change */
		old_dtrap = ((cave_info2[py][px] & (CAVE2_DTRAP)) != 0);
		new_dtrap = ((cave_info2[y][x] & (CAVE2_DTRAP)) != 0);

		/* Note the change in the detect status */
		if (old_dtrap != new_dtrap) p_ptr->redraw |= (PR_DTRAP);

		/* Disturb player if the player is about to leave the area */
		if (OPT(disturb_detect) &&
				p_ptr->running && old_dtrap && !new_dtrap)
		{
			disturb(0, 0);
			return;
		}

  		/* Move player */
  		monster_swap(py, px, y, x);
  


		/* New location */
		y = py = p_ptr->py;
		x = px = p_ptr->px;


		/* Spontaneous Searching */
		if ((p_ptr->state.skills[SKILL_SEARCH_FREQUENCY] >= 50) ||
		    one_in_(50 - p_ptr->state.skills[SKILL_SEARCH_FREQUENCY]))
		{
			search(FALSE);
		}

		/* Continuous Searching */
		if (p_ptr->searching)
		{
			search(FALSE);
		}


		/* Handle "store doors" */
		if ((cave_feat[p_ptr->py][p_ptr->px] >= FEAT_SHOP_HEAD) &&
			(cave_feat[p_ptr->py][p_ptr->px] <= FEAT_SHOP_TAIL))
		{
			/* Disturb */
			disturb(0, 0);
			cmd_insert(CMD_ENTER_STORE);
		}

		/* All other grids (including traps) */
		else
		{
			/* Handle objects (later) */
			p_ptr->notice |= (PN_PICKUP);
		}


		/* Discover invisible traps */
		if (cave_feat[y][x] == FEAT_INVIS)
		{
			/* Disturb */
			disturb(0, 0);

			/* Message */
			msg_print("You found a trap!");

			/* Pick a trap */
			pick_trap(y, x);

			/* Hit the trap */
			hit_trap(y, x);
		}

		/* Set off an visible trap */
		else if ((cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
		         (cave_feat[y][x] <= FEAT_TRAP_TAIL))
		{
			/* Disturb */
			disturb(0, 0);

			/* Hit the trap */
			hit_trap(y, x);
		}
	}
}
