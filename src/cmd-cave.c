/*
 * File: cmd2.c
 * Purpose: Chest and door opening/closing, disarming, running, resting, &c.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "dungeon.h"
#include "files.h"
#include "cmd-core.h"
#include "game-event.h"
#include "generate.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-chest.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-util.h"
#include "pathfind.h"
#include "player-timed.h"
#include "player-util.h"
#include "trap.h"
#include "tables.h"

/*
 * Go up one level
 */
void do_cmd_go_up(struct command *cmd)
{
	/* Verify stairs */
	if (!square_isupstairs(cave, player->py, player->px)) {
		msg("I see no up staircase here.");
		return;
	}

	/* Force descend */
	if (OPT(birth_force_descend)) {
		msg("Nothing happens!");
		return;
	}

	/* Hack -- take a turn */
	player->upkeep->energy_use = 100;

	/* Success */
	msgt(MSG_STAIRS_UP, "You enter a maze of up staircases.");

	/* Create a way back */
	player->upkeep->create_up_stair = FALSE;
	player->upkeep->create_down_stair = TRUE;

	/* Change level */
	dungeon_change_level(player->depth - 1);
}


/*
 * Go down one level
 */
void do_cmd_go_down(struct command *cmd)
{
	int descend_to = player->depth + 1;

	/* Verify stairs */
	if (!square_isdownstairs(cave, player->py, player->px)) {
		msg("I see no down staircase here.");
		return;
	}

	/* Paranoia, no descent from MAX_DEPTH-1 */
	if (player->depth == MAX_DEPTH-1) {
		msg("The dungeon does not appear to extend deeper");
		return;
	}

	/* Warn a force_descend player if they're going to a quest level */
	if (OPT(birth_force_descend)) {
		if (is_quest(player->max_depth + 1) && !get_check("Are you sure you want to descend?"))
			return;

		/* Don't overshoot */
		descend_to = MIN(player->max_depth + 1, MAX_DEPTH-1);
	}

	/* Hack -- take a turn */
	player->upkeep->energy_use = 100;

	/* Success */
	msgt(MSG_STAIRS_DOWN, "You enter a maze of down staircases.");

	/* Create a way back */
	player->upkeep->create_up_stair = TRUE;
	player->upkeep->create_down_stair = FALSE;

	/* Change level */
	dungeon_change_level(descend_to);
}




/*
 * Search for hidden things.  Returns true if a search was attempted, returns
 * false when the player has a 0% chance of finding anything.  Prints messages
 * for negative confirmation when verbose mode is requested.
 */
bool search(bool verbose)
{
	int py = player->py;
	int px = player->px;

	int y, x, chance;

	bool found = FALSE;

	object_type *o_ptr;


	/* Start with base search ability */
	chance = player->state.skills[SKILL_SEARCH];

	/* Penalize various conditions */
	if (player->timed[TMD_BLIND] || no_light()) chance = chance / 10;
	if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE]) chance = chance / 10;

	/* Prevent fruitless searches */
	if (chance <= 0)
	{
		if (verbose)
		{
			msg("You can't make out your surroundings well enough to search.");

			/* Cancel repeat */
			disturb(player, 0);
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
				if (square_invisible_trap(cave, y, x)) 
				{
					found = TRUE;

					/* Reveal trap, display a message */
					if (square_reveal_trap(cave, y, x, chance, TRUE))
					{
						/* Disturb */
						disturb(player, 0);
					}
				}

				/* Secret door */
				if (square_issecretdoor(cave, y, x))
				{
					found = TRUE;

					/* Message */
					msg("You have found a secret door.");

					/* Pick a door */
					place_closed_door(cave, y, x);

					/* Disturb */
					disturb(player, 0);
				}

				/* Scan all objects in the grid */
				for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
				{
					/* Skip if not a trapped chest */
					if (!is_trapped_chest(o_ptr)) continue;

					/* Identify once */
					if (!object_is_known(o_ptr))
					{
						found = TRUE;

						/* Message */
						msg("You have discovered a trap on the chest!");

						/* Know the trap */
						object_notice_everything(o_ptr);

						/* Notice it */
						disturb(player, 0);
					}
				}
			}
		}
	}

	if (verbose && !found)
	{
		if (chance >= 100)
			msg("There are no secrets here.");
		else
			msg("You found nothing.");
	}

	return TRUE;
}



/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(struct command *cmd)
{
	/* Only take a turn if attempted */
	if (search(TRUE))
		player->upkeep->energy_use = 100;
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(struct command *cmd)
{
	/* Stop searching */
	if (player->searching)
	{
		/* Clear the searching flag */
		player->searching = FALSE;

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Redraw the state */
		player->upkeep->redraw |= (PR_STATE);
	}

	/* Start searching */
	else
	{
		/* Set the searching flag */
		player->searching = TRUE;

		/* Update stuff */
		player->upkeep->update |= (PU_BONUS);

		/* Redraw stuff */
		player->upkeep->redraw |= (PR_STATE | PR_SPEED);
	}
}


/*
 * Determine if a given grid may be "opened"
 */
static bool do_cmd_open_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_ismark(cave, y, x)) {
		msg("You see nothing there.");
		return FALSE;
	}

	if (!square_iscloseddoor(cave, y, x)) {
		msgt(MSG_NOTHING_TO_OPEN, "You see nothing there to open.");
		return FALSE;
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "open" command on doors
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_open_aux(int y, int x)
{
	int i, j;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_open_test(y, x)) return (FALSE);


	/* Locked door */
	if (square_islockeddoor(cave, y, x))
	{
		/* Disarm factor */
		i = player->state.skills[SKILL_DISARM];

		/* Penalize some conditions */
		if (player->timed[TMD_BLIND] || no_light()) i = i / 10;
		if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE]) i = i / 10;

		/* Extract the lock power */
		j = square_door_power(cave, y, x);

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success */
		if (randint0(100) < j)
		{
			/* Message */
			msgt(MSG_LOCKPICK, "You have picked the lock.");

			/* Open the door */
			square_open_door(cave, y, x);

			/* Update the visuals */
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Experience */
			/* Removed to avoid exploit by repeatedly locking and unlocking door */
			/* player_exp_gain(player, 1); */
		}

		/* Failure */
		else
		{
			flush();

			/* Message */
			msgt(MSG_LOCKPICK_FAIL, "You failed to pick the lock.");

			/* We may keep trying */
			more = TRUE;
		}
	}

	/* Closed door */
	else
	{
		/* Open the door */
		square_open_door(cave, y, x);

		/* Update the visuals */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Sound */
		sound(MSG_OPENDOOR);
	}

	/* Result */
	return (more);
}



/*
 * Open a closed/locked/jammed door or a closed/locked chest.
 *
 * Unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open(struct command *cmd)
{
	int y, x, dir;
	s16b o_idx;
	bool more = FALSE;
	int err;
	struct monster *m;

	/* Get arguments */
	err = cmd_get_arg_direction(cmd, "direction", &dir);
	if (err || dir == DIR_UNKNOWN) {
		int y, x;
		int n_closed_doors, n_locked_chests;

		n_closed_doors = count_feats(&y, &x, square_iscloseddoor, FALSE);
		n_locked_chests = count_chests(&y, &x, CHEST_OPENABLE);

		if (n_closed_doors + n_locked_chests == 1) {
			dir = coords_to_dir(y, x);
			cmd_set_arg_direction(cmd, "direction", dir);
		} else if (cmd_get_direction(cmd, "direction", &dir, FALSE)) {
			return;
		}
	}

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Check for chest */
	o_idx = chest_check(y, x, CHEST_OPENABLE);

	/* Check for door */
	if (!o_idx && !do_cmd_open_test(y, x)) {
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, FALSE)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];

		/* Check for chest */
		o_idx = chest_check(y, x, CHEST_OPENABLE);
	}

	/* Monster */
	m = square_monster(cave, y, x);
	if (m) {
		/* Mimics surprise the player */
		if (is_mimicking(m)) {
			become_aware(m);

			/* Mimic wakes up */
			mon_clear_timed(m, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);
		} else {
			/* Message */
			msg("There is a monster in the way!");

			/* Attack */
			py_attack(y, x);
		}
	}

	/* Chest */
	else if (o_idx)
		more = do_cmd_open_chest(y, x, o_idx);

	/* Door */
	else
		more = do_cmd_open_aux(y, x);

	/* Cancel repeat unless we may continue */
	if (!more) disturb(player, 0);
}


/*
 * Determine if a given grid may be "closed"
 */
static bool do_cmd_close_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_ismark(cave, y, x))
	{
		/* Message */
		msg("You see nothing there.");

		/* Nope */
		return (FALSE);
	}

 	/* Require open/broken door */
	if (!square_isopendoor(cave, y, x) && !square_isbrokendoor(cave, y, x))
	{
		/* Message */
		msg("You see nothing there to close.");

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "close" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_close_aux(int y, int x)
{
	bool more = FALSE;

	/* Verify legality */
	if (!do_cmd_close_test(y, x)) return (FALSE);

	/* Broken door */
	if (square_isbrokendoor(cave, y, x))
	{
		/* Message */
		msg("The door appears to be broken.");
	}

	/* Open door */
	else
	{
		/* Close the door */
		square_close_door(cave, y, x);

		/* Update the visuals */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Sound */
		sound(MSG_SHUTDOOR);
	}

	/* Result */
	return (more);
}


/*
 * Close an open door.
 */
void do_cmd_close(struct command *cmd)
{
	int y, x, dir;
	int err;

	bool more = FALSE;

	/* Get arguments */
	err = cmd_get_arg_direction(cmd, "direction", &dir);
	if (err || dir == DIR_UNKNOWN) {
		int y, x;

		/* Count open doors */
		if (count_feats(&y, &x, square_isopendoor, FALSE) == 1) {
			dir = coords_to_dir(y, x);
			cmd_set_arg_direction(cmd, "direction", dir);
		} else if (cmd_get_direction(cmd, "direction", &dir, FALSE)) {
			return;
		}
	}

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Verify legality */
	if (!do_cmd_close_test(y, x)) {
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, FALSE)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
	}

	/* Monster - alert, then attack */
	if (cave->m_idx[y][x] > 0) {
		msg("There is a monster in the way!");
		py_attack(y, x);
	}

	/* Door - close it */
	else
		more = do_cmd_close_aux(y, x);


	/* Cancel repeat unless told not to */
	if (!more) disturb(player, 0);
}


/*
 * Determine if a given grid may be "tunneled"
 */
static bool do_cmd_tunnel_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_ismark(cave, y, x))
	{
		/* Message */
		msg("You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Must be a wall/door/etc */
	if (!(square_isdiggable(cave, y, x) || square_iscloseddoor(cave, y, x)))
	{
		/* Message */
		msg("You see nothing there to tunnel.");

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Tunnel through wall.  Assumes valid location.
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 *
 * Attempting to do so will produce floor grids which are not part
 * of the room, and whose "illumination" status do not change with
 * the rest of the room.
 */
static bool twall(int y, int x)
{
	/* Paranoia -- Require a wall or door or some such */
	if (!(square_isdiggable(cave, y, x) || square_iscloseddoor(cave, y, x)))
		return (FALSE);

	/* Sound */
	sound(MSG_DIG);

	/* Forget the wall */
	sqinfo_off(cave->info[y][x], SQUARE_MARK);

	/* Remove the feature */
	square_tunnel_wall(cave, y, x);

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Result */
	return (TRUE);
}


/*
 * Perform the basic "tunnel" command
 *
 * Assumes that no monster is blocking the destination
 *
 * Uses "twall" (above) to do all "terrain feature changing".
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_tunnel_aux(int y, int x)
{
	bool more = FALSE;
	int digging_chances[DIGGING_MAX];

	/* Verify legality */
	if (!do_cmd_tunnel_test(y, x)) return (FALSE);

	calc_digging_chances(&player->state, digging_chances);

	/* Sound XXX XXX XXX */
	/* sound(MSG_DIG); */

	/* Titanium */
	if (square_isperm(cave, y, x))
	{
		msg("This seems to be permanent rock.");
	}

	/* Granite */
	else if (square_isrock(cave, y, x))
	{
		/* Tunnel */
		if (digging_chances[DIGGING_GRANITE] > randint0(1600) && twall(y, x))
		{
			msg("You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg("You tunnel into the granite wall.");
			more = TRUE;
		}
	}

	/* Quartz / Magma */
	else if (square_ismagma(cave, y, x) || square_isquartz(cave, y, x))
	{
		bool okay = FALSE;
		bool gold = FALSE;
		bool hard = FALSE;

		/* Found gold */
		if (square_hasgoldvein(cave, y, x))
			gold = TRUE;

		/* Extract "quartz" flag XXX XXX XXX */
		if (square_isquartz(cave, y, x))
			hard = TRUE;

		/* Quartz */
		if (hard)
			okay = (digging_chances[DIGGING_QUARTZ] > randint0(1600));
		/* Magma */
		else
			okay = (digging_chances[DIGGING_MAGMA] > randint0(1600));

		/* Success */
		if (okay && twall(y, x))
		{
			/* Found treasure */
			if (gold)
			{
				/* Place some gold */
				place_gold(cave, y, x, player->depth, ORIGIN_FLOOR);

				/* Message */
				msg("You have found something!");
			}

			/* Found nothing */
			else
			{
				/* Message */
				msg("You have finished the tunnel.");
			}
		}

		/* Failure (quartz) */
		else if (hard)
		{
			/* Message, continue digging */
			msg("You tunnel into the quartz vein.");
			more = TRUE;
		}

		/* Failure (magma) */
		else
		{
			/* Message, continue digging */
			msg("You tunnel into the magma vein.");
			more = TRUE;
		}
	}

	/* Rubble */
	else if (square_isrubble(cave, y, x))
	{
		/* Remove the rubble */
		if ((digging_chances[DIGGING_RUBBLE] > randint0(1600)) && twall(y, x))
		{
			/* Message */
			msg("You have removed the rubble.");

			/* Hack -- place an object */
			if (randint0(100) < 10)	{
				/* Create a simple object */
				place_object(cave, y, x, player->depth, FALSE, FALSE,
					ORIGIN_RUBBLE, 0);

				/* Observe the new object */
				if (!ignore_item_ok(square_object(cave, y, x)) &&
					    player_can_see_bold(y, x))
					msg("You have found something!");
			}
		}

		else
		{
			/* Message, keep digging */
			msg("You dig in the rubble.");
			more = TRUE;
		}
	}

	/* Secret doors */
	else if (square_issecretdoor(cave, y, x))
	{
		/* Tunnel */
		if ((digging_chances[DIGGING_DOORS] > randint0(1600)) && twall(y, x))
		{
			msg("You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg("You tunnel into the granite wall.");
			more = TRUE;

			/* Occasional Search XXX XXX */
			if (randint0(100) < 25) search(FALSE);
		}
	}

	/* Doors */
	else
	{
		/* Tunnel */
		if ((digging_chances[DIGGING_DOORS] > randint0(1600)) && twall(y, x))
		{
			msg("You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg("You tunnel into the door.");
			more = TRUE;
		}
	}

	/* Result */
	return (more);
}


/*
 * Tunnel through "walls" (including rubble and secret doors)
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel(struct command *cmd)
{
	int y, x, dir;
	bool more = FALSE;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, FALSE))
		return;

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];


	/* Oops */
	if (!do_cmd_tunnel_test(y, x))
	{
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, FALSE))
	{
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
	}


	/* Monster */
	if (cave->m_idx[y][x] > 0)
	{
		/* Message */
		msg("There is a monster in the way!");

		/* Attack */
		py_attack(y, x);
	}

	/* Walls */
	else
	{
		/* Tunnel through walls */
		more = do_cmd_tunnel_aux(y, x);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(player, 0);
}

/*
 * Determine if a given grid may be "disarmed"
 */
static bool do_cmd_disarm_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_ismark(cave, y, x)) {
		msg("You see nothing there.");
		return FALSE;
	}

	/* Look for a closed, unlocked door to lock */
	if (square_iscloseddoor(cave, y, x) && !square_islockeddoor(cave, y, x))
		return TRUE;

	/* Look for a trap */
	if (!square_isknowntrap(cave, y, x)) {
		msg("You see nothing there to disarm.");
		return FALSE;
	}

	/* Okay */
	return TRUE;
}


/*
 * Perform the command "lock door"
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_lock_door(int y, int x)
{
	int i, j, power;
	bool more = FALSE;

	/* Verify legality */
	if (!do_cmd_disarm_test(y, x)) return FALSE;

	/* Get the "disarm" factor */
	i = player->state.skills[SKILL_DISARM];

	/* Penalize some conditions */
	if (player->timed[TMD_BLIND] || no_light())
		i = i / 10;
	if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE])
		i = i / 10;

	/* Calculate lock "power" */
	power = m_bonus(7, player->depth);

	/* Extract the difficulty */
	j = i - power;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Success */
	if (randint0(100) < j) {
		msg("You lock the door.");
		square_lock_door(cave, y, x, power);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint1(i) > 5)) {
		flush();
		msg("You failed to lock the door.");

		/* We may keep trying */
		more = TRUE;
	}
	/* Failure */
	else
		msg("You failed to lock the door.");

	/* Result */
	return more;
}


/*
 * Perform the basic "disarm" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_disarm_aux(int y, int x)
{
	int i, j, power;

	int trap;
    trap_type *t_ptr;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_disarm_test(y, x)) return (FALSE);


    /* Choose trap */
    trap = square_visible_trap_idx(cave, y, x);
    if (trap < 0) return (FALSE);
    t_ptr = cave_trap(cave, trap);

	/* Get the "disarm" factor */
	i = player->state.skills[SKILL_DISARM];

	/* Penalize some conditions */
	if (player->timed[TMD_BLIND] || no_light()) i = i / 10;
	if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE]) i = i / 10;

	/* XXX XXX XXX Variable power? */

	/* Extract trap "power" */
	power = 5;

	/* Extract the difficulty */
	j = i - power;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Success */
	if (randint0(100) < j)
	{
		/* Message */
		msgt(MSG_DISARM, "You have disarmed the %s.", t_ptr->kind->name);

		/* Reward */
		player_exp_gain(player, power);

		/* Forget the trap */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Remove the trap */
		square_destroy_trap(cave, y, x);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint1(i) > 5))
	{
		flush();

		/* Message */
		msg("You failed to disarm the %s.", t_ptr->kind->name);

		/* We may keep trying */
		more = TRUE;
	}

	/* Failure -- Set off the trap */
	else
	{
		/* Message */
		msg("You set off the %s!", t_ptr->kind->name);

		/* Hit the trap */
		hit_trap(y, x);
	}

	/* Result */
	return (more);
}


/*
 * Disarms a trap, or a chest
 */
void do_cmd_disarm(struct command *cmd)
{
	int y, x, dir;
	int err;

	s16b o_idx;
	bool more = FALSE;

	/* Get arguments */
	err = cmd_get_arg_direction(cmd, "direction", &dir);
	if (err || dir == DIR_UNKNOWN) {
		int y, x;
		int n_visible_traps, n_trapped_chests;

		n_visible_traps = count_feats(&y, &x, square_isknowntrap, TRUE);
		n_trapped_chests = count_chests(&y, &x, CHEST_TRAPPED);

		if (n_visible_traps + n_trapped_chests == 1) {
			dir = coords_to_dir(y, x);
			cmd_set_arg_direction(cmd, "direction", dir);
		}

		/* If there are chests to disarm, allow 5 as a direction */
		else if (cmd_get_direction(cmd, "direction", &dir, n_trapped_chests > 0)) {
			return;
		}
	}

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Check for chests */
	o_idx = chest_check(y, x, CHEST_TRAPPED);

	/* Verify legality */
	if (!o_idx && !do_cmd_disarm_test(y, x))
	{
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, FALSE))
	{
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];

		/* Check for chests */
		o_idx = chest_check(y, x, CHEST_TRAPPED);
	}


	/* Monster */
	if (cave->m_idx[y][x] > 0) {
		msg("There is a monster in the way!");
		py_attack(y, x);
	}

	/* Chest */
	else if (o_idx)
		more = do_cmd_disarm_chest(y, x, o_idx);

	/* Door to lock */
	else if (    square_iscloseddoor(cave, y, x)
	         && !square_islockeddoor(cave, y, x))
		more = do_cmd_lock_door(y, x);

	/* Disarm trap */
	else
		more = do_cmd_disarm_aux(y, x);

	/* Cancel repeat unless told not to */
	if (!more) disturb(player, 0);
}

/*
 * Manipulate an adjacent grid in some way
 *
 * Attack monsters, tunnel through walls, disarm traps, open doors.
 *
 * This command must always take energy, to prevent free detection
 * of invisible monsters.
 *
 * The "semantics" of this command must be chosen before the player
 * is confused, and it must be verified against the new grid.
 */
void do_cmd_alter_aux(int dir)
{
	int y, x;
	bool more = FALSE;

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Take a turn */
	player->upkeep->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, FALSE)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
	}

	/* Attack monsters */
	if (cave->m_idx[y][x] > 0)
		py_attack(y, x);

	/* Tunnel through walls and rubble */
	else if (square_isdiggable(cave, y, x))
		more = do_cmd_tunnel_aux(y, x);

	/* Open closed doors */
	else if (square_iscloseddoor(cave, y, x))
		more = do_cmd_open_aux(y, x);

	/* Disarm traps */
	else if (square_isknowntrap(cave, y, x))
		more = do_cmd_disarm_aux(y, x);

	/* Oops */
	else
		msg("You spin around.");

	/* Cancel repetition unless we can continue */
	if (!more) disturb(player, 0);
}

void do_cmd_alter(struct command *cmd)
{
	int dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, FALSE) != CMD_OK)
		return;

	do_cmd_alter_aux(dir);
}

/*
 * Determine if a given grid may be "walked"
 */
static bool do_cmd_walk_test(int y, int x)
{
	int m_idx = cave->m_idx[y][x];
	struct monster *m_ptr = cave_monster(cave, m_idx);

	/* Allow attack on visible monsters if unafraid */
	if (m_idx > 0 && m_ptr->ml && !is_mimicking(m_ptr))
	{
		/* Handle player fear */
		if (player_of_has(player, OF_AFRAID))
		{
			/* Extract monster name (or "it") */
			char m_name[80];
			monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_DEFAULT);

			/* Message */
			msgt(MSG_AFRAID, "You are too afraid to attack %s!", m_name);

			/* Nope */
			return (FALSE);
		}

		return (TRUE);
	}

	/* If we don't know the grid, allow attempts to walk into it */
	if (!square_ismark(cave, y, x))
		return TRUE;

	/* Require open space */
	if (!square_ispassable(cave, y, x))
	{
		/* Rubble */
		if (square_isrubble(cave, y, x))
			msgt(MSG_HITWALL, "There is a pile of rubble in the way!");

		/* Door */
		else if (square_iscloseddoor(cave, y, x))
			return TRUE;

		/* Wall */
		else
			msgt(MSG_HITWALL, "There is a wall in the way!");

		/* Cancel repeat */
		disturb(player, 0);

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Walk in the given direction.
 */
void do_cmd_walk(struct command *cmd)
{
	int x, y, dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, FALSE) != CMD_OK)
		return;

	/* Apply confusion if necessary */
	/* Confused movements use energy no matter what */
	if (player_confuse_dir(player, &dir, FALSE))
		player->upkeep->energy_use = 100;
	
	/* Verify walkability */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	player->upkeep->energy_use = 100;

	move_player(dir, TRUE);
}


/*
 * Walk into a trap.
 */
void do_cmd_jump(struct command *cmd)
{
	int x, y, dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, FALSE) != CMD_OK)
		return;

	/* Apply confusion if necessary */
	if (player_confuse_dir(player, &dir, FALSE))
		player->upkeep->energy_use = 100;

	/* Verify walkability */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	player->upkeep->energy_use = 100;

	move_player(dir, FALSE);
}


/*
 * Start running.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_run(struct command *cmd)
{
	int x, y, dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "dirction", &dir, FALSE) != CMD_OK)
		return;

	if (player_confuse_dir(player, &dir, TRUE))
		return;

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	/* Start run */
	run_step(dir);
}


/*
 * Start running with pathfinder.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_pathfind(struct command *cmd)
{
	int x, y;

	/* XXX-AS Add better arg checking */
	cmd_get_arg_point(cmd, "point", &x, &y);

	if (player->timed[TMD_CONFUSED])
		return;

	if (findpath(x, y)) {
		player->upkeep->running = 1000;
		/* Calculate torch radius */
		player->upkeep->update |= (PU_TORCH);
		player->upkeep->running_withpathfind = TRUE;
		run_step(0);
	}
}



/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_hold(struct command *cmd)
{
	/* Take a turn */
	player->upkeep->energy_use = 100;

	/* Spontaneous Searching */
	if ((player->state.skills[SKILL_SEARCH_FREQUENCY] >= 50) ||
	    one_in_(50 - player->state.skills[SKILL_SEARCH_FREQUENCY]))
	{
		search(FALSE);
	}

	/* Continuous Searching */
	if (player->searching)
	{
		search(FALSE);
	}

	/* Pick things up, not using extra energy */
	do_autopickup();

	/* Hack -- enter a store if we are on one */
	if (square_isshop(cave, player->py, player->px)) {
		/* Disturb */
		disturb(player, 0);

		cmdq_push(CMD_ENTER_STORE);

		/* Free turn XXX XXX XXX */
		player->upkeep->energy_use = 0;
	}
	else
	{
	    event_signal(EVENT_SEEFLOOR);
	}
}


/*
 * Rest (restores hit points and mana and such)
 */
void do_cmd_rest(struct command *cmd)
{
	int n;

	/* XXX-AS need to insert UI here */
	if (cmd_get_arg_choice(cmd, "choice", &n) != CMD_OK)
		return;

	/* 
	 * A little sanity checking on the input - only the specified negative 
	 * values are valid. 
	 */
    if (n < 0 && !player_resting_is_special(n))
        return;

	player_resting_set_count(player, n);

	/* Take a turn XXX XXX XXX (?) */
	player->upkeep->energy_use = 100;

	/* Cancel searching */
	player->searching = FALSE;

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Redraw the state */
	player->upkeep->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Refresh XXX XXX XXX */
	Term_fresh();
}


void textui_cmd_rest(void)
{
	const char *p = "Rest (0-9999, '!' for HP or SP, '*' for HP and SP, '&' as needed): ";

	char out_val[5] = "& ";

	/* Ask for duration */
	if (!get_string(p, out_val, sizeof(out_val))) return;

	/* Rest until done */
	if (out_val[0] == '&')
	{
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", REST_COMPLETE);
	}

	/* Rest a lot */
	else if (out_val[0] == '*')
	{
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", REST_ALL_POINTS);
	}

	/* Rest until HP or SP filled */
	else if (out_val[0] == '!')
	{
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", REST_SOME_POINTS);
	}

	/* Rest some */
	else
	{
		int turns = atoi(out_val);
		if (turns <= 0) return;
		if (turns > 9999) turns = 9999;
		
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", turns);
	}
}


/*
 * Array of feeling strings for object feelings.
 * Keep strings at 36 or less characters to keep the
 * combined feeling on one row.
 */
static const char *obj_feeling_text[] =
{
	"Looks like any other level.",
	"you sense an item of wondrous power!",
	"there are superb treasures here.",
	"there are excellent treasures here.",
	"there are very good treasures here.",
	"there are good treasures here.",
	"there may be something worthwhile here.",
	"there may not be much interesting here.",
	"there aren't many treasures here.",
	"there are only scraps of junk here.",
	"there are naught but cobwebs here."
};

/*
 * Array of feeling strings for monster feelings.
 * Keep strings at 36 or less characters to keep the
 * combined feeling on one row.
 */
static const char *mon_feeling_text[] =
{
	/* first string is just a place holder to 
	 * maintain symmetry with obj_feeling.
	 */
	"You are still uncertain about this place",
	"Omens of death haunt this place",
	"This place seems murderous",
	"This place seems terribly dangerous",
	"You feel anxious about this place",
	"You feel nervous about this place",
	"This place does not seem too risky",
	"This place seems reasonably safe",
	"This seems a tame, sheltered place",
	"This seems a quiet, peaceful place"
};

/*
 * Display the feeling.  Players always get a monster feeling.
 * Object feelings are delayed until the player has explored some
 * of the level.
 */

void display_feeling(bool obj_only)
{
	u16b obj_feeling = cave->feeling / 10;
	u16b mon_feeling = cave->feeling - (10 * obj_feeling);
	const char *join;

	/* Don't show feelings for cold-hearted characters */
	if (OPT(birth_no_feelings)) return;

	/* No useful feeling in town */
	if (!player->depth) {
		msg("Looks like a typical town.");
		return;
	}
	
	/* Display only the object feeling when it's first discovered. */
	if (obj_only){
		msg("You feel that %s", obj_feeling_text[obj_feeling]);
		return;
	}
	
	/* Players automatically get a monster feeling. */
	if (cave->feeling_squares < FEELING1){
		msg("%s.", mon_feeling_text[mon_feeling]);
		return;
	}
	
	/* Verify the feelings */
	if (obj_feeling >= N_ELEMENTS(obj_feeling_text))
		obj_feeling = N_ELEMENTS(obj_feeling_text) - 1;

	if (mon_feeling >= N_ELEMENTS(mon_feeling_text))
		mon_feeling = N_ELEMENTS(mon_feeling_text) - 1;

	/* Decide the conjunction */
	if ((mon_feeling <= 5 && obj_feeling > 6) ||
			(mon_feeling > 5 && obj_feeling <= 6))
		join = ", yet";
	else
		join = ", and";

	/* Display the feeling */
	msg("%s%s %s", mon_feeling_text[mon_feeling], join,
		obj_feeling_text[obj_feeling]);
}


void do_cmd_feeling(void)
{
	display_feeling(FALSE);
}

