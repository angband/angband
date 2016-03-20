/**
 * \file cmd-cave.c
 * \brief Chest and door opening/closing, disarming, running, resting, &c.
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
#include "cave.h"
#include "cmd-core.h"
#include "cmds.h"
#include "game-event.h"
#include "game-input.h"
#include "generate.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-chest.h"
#include "obj-ignore.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-path.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "trap.h"

/**
 * Go up one level
 */
void do_cmd_go_up(struct command *cmd)
{
	int ascend_to;

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
	
	ascend_to = dungeon_get_next_level(player->depth, -1);
	
	if (ascend_to == player->depth) {
		msg("You can't go up from here!");
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Success */
	msgt(MSG_STAIRS_UP, "You enter a maze of up staircases.");

	/* Create a way back */
	player->upkeep->create_up_stair = false;
	player->upkeep->create_down_stair = true;
	
	/* Change level */
	dungeon_change_level(ascend_to);
}


/**
 * Go down one level
 */
void do_cmd_go_down(struct command *cmd)
{
	int descend_to = dungeon_get_next_level(player->depth, 1);

	/* Verify stairs */
	if (!square_isdownstairs(cave, player->py, player->px)) {
		msg("I see no down staircase here.");
		return;
	}

	/* Paranoia, no descent from z_info->max_depth - 1 */
	if (player->depth == z_info->max_depth - 1) {
		msg("The dungeon does not appear to extend deeper");
		return;
	}

	/* Warn a force_descend player if they're going to a quest level */
	if (OPT(birth_force_descend)) {
		descend_to = dungeon_get_next_level(player->max_depth, 1);
		if (is_quest(descend_to) &&
			!get_check("Are you sure you want to descend?"))
			return;
	}

	/* Hack -- take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Success */
	msgt(MSG_STAIRS_DOWN, "You enter a maze of down staircases.");

	/* Create a way back */
	player->upkeep->create_up_stair = true;
	player->upkeep->create_down_stair = false;

	/* Change level */
	dungeon_change_level(descend_to);
}



/**
 * Search for hidden things.  Returns true if a search was attempted, returns
 * false when the player has a 0% chance of finding anything.  Prints messages
 * for negative confirmation when verbose mode is requested.
 */
bool search(bool verbose)
{
	int py = player->py;
	int px = player->px;
	int y, x, chance;
	bool found = false;
	struct object *obj;

	/* Start with base search ability */
	chance = player->state.skills[SKILL_SEARCH];

	/* Penalize various conditions */
	if (player->timed[TMD_BLIND] || no_light())
		chance = chance / 10;
	if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE])
		chance = chance / 10;

	/* Prevent fruitless searches */
	if (chance <= 0) {
		if (verbose) {
			msg("You can't make out your surroundings well enough to search.");

			/* Cancel repeat */
			disturb(player, 0);
		}

		return false;
	}

	/* Search the nearby grids, which are always in bounds */
	for (y = (py - 1); y <= (py + 1); y++) {
		for (x = (px - 1); x <= (px + 1); x++) {
			/* Sometimes, notice things */
			if (randint0(100) < chance) {
				if (square_issecrettrap(cave, y, x)) {
					found = true;

					/* Reveal trap, display a message */
					if (square_reveal_trap(cave, y, x, chance, true))
						/* Disturb */
						disturb(player, 0);
				}

				/* Secret door */
				if (square_issecretdoor(cave, y, x)) {
					found = true;

					/* Message */
					msg("You have found a secret door.");

					/* Pick a door */
					place_closed_door(cave, y, x);

					/* Disturb */
					disturb(player, 0);
				}

				/* Scan all objects in the grid */
				for (obj = square_object(cave, y, x); obj; obj = obj->next) {
					/* Skip if not a trapped chest */
					if (!is_trapped_chest(obj)) continue;

					/* Identify once */
					if (obj->known->pval != obj->pval) {
						found = true;

						/* Message */
						msg("You have discovered a trap on the chest!");

						/* Know the trap */
						obj->known->pval = obj->pval;

						/* Notice it */
						disturb(player, 0);
					}
				}
			}
		}
	}

	if (verbose && !found) {
		if (chance >= 100)
			msg("There are no secrets here.");
		else
			msg("You found nothing.");
	}

	return true;
}



/**
 * Simple command to "search" for one turn
 */
void do_cmd_search(struct command *cmd)
{
	/* Only take a turn if attempted */
	if (search(true))
		player->upkeep->energy_use = z_info->move_energy;
}


/**
 * Toggle search mode
 */
void do_cmd_toggle_search(struct command *cmd)
{
	if (player->searching) {
		/* Stop searching */
		player->searching = false;
		player->upkeep->update |= (PU_BONUS);
		player->upkeep->redraw |= (PR_STATE);
	} else {
		/* Start searching */
		player->searching = true;
		player->upkeep->update |= (PU_BONUS);
		player->upkeep->redraw |= (PR_STATE | PR_SPEED);
	}
}


/**
 * Determine if a given grid may be "opened"
 */
static bool do_cmd_open_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_isknown(cave, y, x)) {
		msg("You see nothing there.");
		return false;
	}

	/* Must be a closed door */
	if (!square_iscloseddoor(cave, y, x)) {
		msgt(MSG_NOTHING_TO_OPEN, "You see nothing there to open.");
		return false;
	}

	return (true);
}


/**
 * Perform the basic "open" command on doors
 *
 * Assume there is no monster blocking the destination
 *
 * Returns true if repeated commands may continue
 */
static bool do_cmd_open_aux(int y, int x)
{
	int i, j;
	bool more = false;

	/* Verify legality */
	if (!do_cmd_open_test(y, x)) return (false);

	/* Locked door */
	if (square_islockeddoor(cave, y, x)) {
		/* Disarm factor */
		i = player->state.skills[SKILL_DISARM];

		/* Penalize some conditions */
		if (player->timed[TMD_BLIND] || no_light())
			i = i / 10;
		if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE])
			i = i / 10;

		/* Extract the lock power */
		j = square_door_power(cave, y, x);

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		if (randint0(100) < j) {
			/* Message */
			msgt(MSG_LOCKPICK, "You have picked the lock.");

			/* Open the door */
			square_open_door(cave, y, x);

			/* Update the visuals */
			square_memorize(cave, y, x);
			square_light_spot(cave, y, x);
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Experience */
			/* Removed to avoid exploit by repeatedly locking and unlocking */
			/* player_exp_gain(player, 1); */
		} else {
			event_signal(EVENT_INPUT_FLUSH);

			/* Message */
			msgt(MSG_LOCKPICK_FAIL, "You failed to pick the lock.");

			/* We may keep trying */
			more = true;
		}
	} else {
		/* Closed door */
		square_open_door(cave, y, x);
		square_memorize(cave, y, x);
		square_light_spot(cave, y, x);
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		sound(MSG_OPENDOOR);
	}

	/* Result */
	return (more);
}



/**
 * Open a closed/locked/jammed door or a closed/locked chest.
 *
 * Unlocking a locked chest is worth one experience point; since doors are
 * player lockable, there is no experience for unlocking doors.
 */
void do_cmd_open(struct command *cmd)
{
	int y, x, dir;
	struct object *obj;
	bool more = false;
	int err;
	struct monster *m;

	/* Get arguments */
	err = cmd_get_arg_direction(cmd, "direction", &dir);
	if (err || dir == DIR_UNKNOWN) {
		int y, x;
		int n_closed_doors, n_locked_chests;

		n_closed_doors = count_feats(&y, &x, square_iscloseddoor, false);
		n_locked_chests = count_chests(&y, &x, CHEST_OPENABLE);

		if (n_closed_doors + n_locked_chests == 1) {
			dir = coords_to_dir(y, x);
			cmd_set_arg_direction(cmd, "direction", dir);
		} else if (cmd_get_direction(cmd, "direction", &dir, false)) {
			return;
		}
	}

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Check for chest */
	obj = chest_check(y, x, CHEST_OPENABLE);

	/* Check for door */
	if (!obj && !do_cmd_open_test(y, x)) {
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, false)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];

		/* Check for chest */
		obj = chest_check(y, x, CHEST_OPENABLE);
	}

	/* Monster */
	m = square_monster(cave, y, x);
	if (m) {
		/* Mimics surprise the player */
		if (is_mimicking(m)) {
			become_aware(m);

			/* Mimic wakes up */
			mon_clear_timed(m, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, false);
		} else {
			/* Message */
			msg("There is a monster in the way!");

			/* Attack */
			py_attack(y, x);
		}
	} else if (obj) {
		/* Chest */
		more = do_cmd_open_chest(y, x, obj);
	} else {
		/* Door */
		more = do_cmd_open_aux(y, x);
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(player, 0);
}


/**
 * Determine if a given grid may be "closed"
 */
static bool do_cmd_close_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_isknown(cave, y, x)) {
		/* Message */
		msg("You see nothing there.");

		/* Nope */
		return (false);
	}

 	/* Require open/broken door */
	if (!square_isopendoor(cave, y, x) && !square_isbrokendoor(cave, y, x)) {
		/* Message */
		msg("You see nothing there to close.");

		/* Nope */
		return (false);
	}

	/* Okay */
	return (true);
}


/**
 * Perform the basic "close" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns true if repeated commands may continue
 */
static bool do_cmd_close_aux(int y, int x)
{
	bool more = false;

	/* Verify legality */
	if (!do_cmd_close_test(y, x)) return (false);

	/* Broken door */
	if (square_isbrokendoor(cave, y, x))
	{
		msg("The door appears to be broken.");
	} else {
		/* Close door */
		square_close_door(cave, y, x);
		square_memorize(cave, y, x);
		square_light_spot(cave, y, x);
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		sound(MSG_SHUTDOOR);
	}

	/* Result */
	return (more);
}


/**
 * Close an open door.
 */
void do_cmd_close(struct command *cmd)
{
	int y, x, dir;
	int err;

	bool more = false;

	/* Get arguments */
	err = cmd_get_arg_direction(cmd, "direction", &dir);
	if (err || dir == DIR_UNKNOWN) {
		int y, x;

		/* Count open doors */
		if (count_feats(&y, &x, square_isopendoor, false) == 1) {
			dir = coords_to_dir(y, x);
			cmd_set_arg_direction(cmd, "direction", dir);
		} else if (cmd_get_direction(cmd, "direction", &dir, false)) {
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
	player->upkeep->energy_use = z_info->move_energy;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, false)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
	}

	/* Monster - alert, then attack */
	if (cave->squares[y][x].mon > 0) {
		msg("There is a monster in the way!");
		py_attack(y, x);
	} else
		/* Door - close it */
		more = do_cmd_close_aux(y, x);

	/* Cancel repeat unless told not to */
	if (!more) disturb(player, 0);
}


/**
 * Determine if a given grid may be "tunneled"
 */
static bool do_cmd_tunnel_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_isknown(cave, y, x)) {
		msg("You see nothing there.");
		return (false);
	}

	/* Titanium */
	if (square_isperm(cave, y, x)) {
		msg("This seems to be permanent rock.");
		return (false);
	}

	/* Must be a wall/door/etc */
	if (!(square_isdiggable(cave, y, x) || square_iscloseddoor(cave, y, x))) {
		msg("You see nothing there to tunnel.");
		return (false);
	}

	/* Okay */
	return (true);
}


/**
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
		return (false);

	/* Sound */
	sound(MSG_DIG);

	/* Forget the wall */
	square_forget(cave, y, x);

	/* Remove the feature */
	square_tunnel_wall(cave, y, x);

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Result */
	return (true);
}


/**
 * Perform the basic "tunnel" command
 *
 * Assumes that no monster is blocking the destination.
 * Uses twall() (above) to do all "terrain feature changing".
 * Returns true if repeated commands may continue.
 */
static bool do_cmd_tunnel_aux(int y, int x)
{
	bool more = false;
	int digging_chances[DIGGING_MAX];
	bool okay = false;
	bool gold = square_hasgoldvein(cave, y, x);
	bool rubble = square_isrubble(cave, y, x);

	/* Verify legality */
	if (!do_cmd_tunnel_test(y, x)) return (false);

	calc_digging_chances(&player->state, digging_chances);

	/* Do we succeed? */
	okay = (digging_chances[square_digging(cave, y, x) - 1] > randint0(1600));

	/* Success */
	if (okay && twall(y, x)) {
		/* Rubble is a special case - could be handled more generally NRM */
		if (rubble) {
			/* Message */
			msg("You have removed the rubble.");

			/* Place an object (except in town) */
			if ((randint0(100) < 10) && player->depth) {
				/* Create a simple object */
				place_object(cave, y, x, player->depth, false, false,
							 ORIGIN_RUBBLE, 0);

				/* Observe the new object */
				if (!ignore_item_ok(square_object(cave, y, x)) &&
					square_isseen(cave, y, x))
					msg("You have found something!");
			} 
		} else if (gold) {
			/* Found treasure */
			place_gold(cave, y, x, player->depth, ORIGIN_FLOOR);
			msg("You have found something!");
		} else {
			msg("You have finished the tunnel.");
		}
	} else {
		/* Failure, continue digging */
		if (rubble)
			msg("You dig in the rubble.");
		else
			msg("You tunnel into the %s.",
				square_apparent_name(cave, player, y, x));
		more = true;
		if (square_issecretdoor(cave, y, x))
			/* Occasional Search XXX XXX */
			if (randint0(100) < 25) search(false);
	}

	/* Result */
	return (more);
}


/**
 * Tunnel through "walls" (including rubble and doors, secret or otherwise)
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel(struct command *cmd)
{
	int y, x, dir;
	bool more = false;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, false))
		return;

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Oops */
	if (!do_cmd_tunnel_test(y, x)) {
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, false)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
	}

	/* Attack any monster we run into */
	if (cave->squares[y][x].mon > 0) {
		msg("There is a monster in the way!");
		py_attack(y, x);
	} else {
		/* Tunnel through walls */
		more = do_cmd_tunnel_aux(y, x);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(player, 0);
}

/**
 * Determine if a given grid may be "disarmed"
 */
static bool do_cmd_disarm_test(int y, int x)
{
	/* Must have knowledge */
	if (!square_isknown(cave, y, x)) {
		msg("You see nothing there.");
		return false;
	}

	/* Look for a closed, unlocked door to lock */
	if (square_iscloseddoor(cave, y, x) && !square_islockeddoor(cave, y, x))
		return true;

	/* Look for a trap */
	if (!square_isknowntrap(cave, y, x)) {
		msg("You see nothing there to disarm.");
		return false;
	}

	/* Okay */
	return true;
}


/**
 * Perform the command "lock door"
 *
 * Assume there is no monster blocking the destination
 *
 * Returns true if repeated commands may continue
 */
static bool do_cmd_lock_door(int y, int x)
{
	int i, j, power;
	bool more = false;

	/* Verify legality */
	if (!do_cmd_disarm_test(y, x)) return false;

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
		square_set_door_lock(cave, y, x, power);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint1(i) > 5)) {
		event_signal(EVENT_INPUT_FLUSH);
		msg("You failed to lock the door.");

		/* We may keep trying */
		more = true;
	}
	/* Failure */
	else
		msg("You failed to lock the door.");

	/* Result */
	return more;
}


/**
 * Perform the basic "disarm" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns true if repeated commands may continue
 */
static bool do_cmd_disarm_aux(int y, int x)
{
	int i, j, power;
    struct trap *trap = cave->squares[y][x].trap;
	bool more = false;


	/* Verify legality */
	if (!do_cmd_disarm_test(y, x)) return (false);


    /* Choose first player trap */
	while (trap) {
		if (trf_has(trap->flags, TRF_TRAP))
			break;
		trap = trap->next;
	}
	if (!trap)
		return false;

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
	if (randint0(100) < j) {
		/* Message */
		msgt(MSG_DISARM, "You have disarmed the %s.", trap->kind->name);

		/* Reward */
		player_exp_gain(player, power);

		/* Forget the trap */
		square_forget(cave, y, x);

		/* Remove the trap */
		square_destroy_trap(cave, y, x);
	} else if ((i > 5) && (randint1(i) > 5)) {
		/* Failure -- Keep trying */
		event_signal(EVENT_INPUT_FLUSH);
		msg("You failed to disarm the %s.", trap->kind->name);
		more = true;
	} else {
		/* Failure -- Set off the trap */
		msg("You set off the %s!", trap->kind->name);
		hit_trap(y, x);
	}

	/* Result */
	return (more);
}


/**
 * Disarms a trap, or a chest
 *
 * Traps must be visible, chests must be known trapped
 */
void do_cmd_disarm(struct command *cmd)
{
	int y, x, dir;
	int err;

	struct object *obj;
	bool more = false;

	/* Get arguments */
	err = cmd_get_arg_direction(cmd, "direction", &dir);
	if (err || dir == DIR_UNKNOWN) {
		int y, x;
		int n_traps, n_chests;

		n_traps = count_feats(&y, &x, square_isknowntrap, true);
		n_chests = count_chests(&y, &x, CHEST_TRAPPED);

		if (n_traps + n_chests == 1) {
			dir = coords_to_dir(y, x);
			cmd_set_arg_direction(cmd, "direction", dir);
		} else if (cmd_get_direction(cmd, "direction", &dir, n_chests > 0)) {
			/* If there are chests to disarm, 5 is allowed as a direction */
			return;
		}
	}

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Check for chests */
	obj = chest_check(y, x, CHEST_TRAPPED);

	/* Verify legality */
	if (!obj && !do_cmd_disarm_test(y, x)) {
		/* Cancel repeat */
		disturb(player, 0);
		return;
	}

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, false)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];

		/* Check for chests */
		obj = chest_check(y, x, CHEST_TRAPPED);
	}


	/* Monster */
	if (cave->squares[y][x].mon > 0) {
		msg("There is a monster in the way!");
		py_attack(y, x);
	} else if (obj)
		/* Chest */
		more = do_cmd_disarm_chest(y, x, obj);
	else if (square_iscloseddoor(cave, y, x) &&
			 !square_islockeddoor(cave, y, x))
		/* Door to lock */
		more = do_cmd_lock_door(y, x);
	else
		/* Disarm trap */
		more = do_cmd_disarm_aux(y, x);

	/* Cancel repeat unless told not to */
	if (!more) disturb(player, 0);
}

/**
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
	bool more = false;

	/* Get location */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Apply confusion */
	if (player_confuse_dir(player, &dir, false)) {
		/* Get location */
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
	}

	/* Action depends on what's there */
	if (cave->squares[y][x].mon > 0)
		/* Attack monsters */
		py_attack(y, x);
	else if (square_isdiggable(cave, y, x))
		/* Tunnel through walls and rubble */
		more = do_cmd_tunnel_aux(y, x);
	else if (square_iscloseddoor(cave, y, x))
		/* Open closed doors */
		more = do_cmd_open_aux(y, x);
	else if (square_isknowntrap(cave, y, x))
		/* Disarm traps */
		more = do_cmd_disarm_aux(y, x);
	else
		/* Oops */
		msg("You spin around.");

	/* Cancel repetition unless we can continue */
	if (!more) disturb(player, 0);
}

void do_cmd_alter(struct command *cmd)
{
	int dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, false) != CMD_OK)
		return;

	do_cmd_alter_aux(dir);
}

/**
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
	struct monster *mon = cave_monster(cave, m_idx);
	bool alterable = (square_isknowntrap(cave, y, x) ||
					  square_iscloseddoor(cave, y, x));

	/* Attack monsters, alter traps/doors on movement, hit obstacles or move */
	if (m_idx > 0) {
		/* Mimics surprise the player */
		if (is_mimicking(mon)) {
			become_aware(mon);

			/* Mimic wakes up */
			mon_clear_timed(mon, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, false);

		} else {
			py_attack(y, x);
		}
	} else if (disarm && square_isknown(cave, y, x) && alterable) {
		/* Auto-repeat if not already repeating */
		if (cmd_get_nrepeats() == 0)
			cmd_set_repeat(99);

		do_cmd_alter_aux(dir);
	} else if (player->upkeep->running && square_isknowntrap(cave, y, x)) {
		/* Stop running before known traps */
		disturb(player, 0);
	} else if (!square_ispassable(cave, y, x)) {
		/* Disturb the player */
		disturb(player, 0);

		/* Notice unknown obstacles, mention known obstacles */
		if (!square_isknown(cave, y, x)) {
			if (square_isrubble(cave, y, x)) {
				msgt(MSG_HITWALL,
					 "You feel a pile of rubble blocking your way.");
				square_memorize(cave, y, x);
				square_light_spot(cave, y, x);
			} else if (square_iscloseddoor(cave, y, x)) {
				msgt(MSG_HITWALL, "You feel a door blocking your way.");
				square_memorize(cave, y, x);
				square_light_spot(cave, y, x);
			} else {
				msgt(MSG_HITWALL, "You feel a wall blocking your way.");
				square_memorize(cave, y, x);
				square_light_spot(cave, y, x);
			}
		} else {
			if (square_isrubble(cave, y, x))
				msgt(MSG_HITWALL,
					 "There is a pile of rubble blocking your way.");
			else if (square_iscloseddoor(cave, y, x))
				msgt(MSG_HITWALL, "There is a door blocking your way.");
			else
				msgt(MSG_HITWALL, "There is a wall blocking your way.");
		}
	} else {
		/* See if trap detection status will change */
		bool old_dtrap = square_isdtrap(cave, py, px);
		bool new_dtrap = square_isdtrap(cave, y, x);

		/* Note the change in the detect status */
		if (old_dtrap != new_dtrap)
			player->upkeep->redraw |= (PR_DTRAP);

		/* Disturb player if the player is about to leave the area */
		if (player->upkeep->running && !player->upkeep->running_firststep && 
			old_dtrap && !new_dtrap) {
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
			search(false);

		/* Handle store doors, or notice objects */
		if (square_isshop(cave, player->py, player->px)) {
			/* Disturb */
			disturb(player, 0);
			event_signal(EVENT_ENTER_STORE);
			event_remove_handler_type(EVENT_ENTER_STORE);
			event_signal(EVENT_USE_STORE);
			event_remove_handler_type(EVENT_USE_STORE);
			event_signal(EVENT_LEAVE_STORE);
			event_remove_handler_type(EVENT_LEAVE_STORE);
		} else {
			/* Know objects, queue autopickup */
			floor_pile_know(cave, player->py, player->px);
			cmdq_push(CMD_AUTOPICKUP);
		}


		/* Discover invisible traps, set off visible ones */
		if (square_issecrettrap(cave, y, x)) {
			/* Disturb */
			disturb(player, 0);

			/* Hit the trap. */
			hit_trap(y, x);
		} else if (square_isknowntrap(cave, y, x)) {
			/* Disturb */
			disturb(player, 0);

			/* Hit the trap */
			hit_trap(y, x);
		}
	}

	player->upkeep->running_firststep = false;
}

/**
 * Determine if a given grid may be "walked"
 */
static bool do_cmd_walk_test(int y, int x)
{
	int m_idx = cave->squares[y][x].mon;
	struct monster *mon = cave_monster(cave, m_idx);

	/* Allow attack on visible monsters if unafraid */
	if (m_idx > 0 && mflag_has(mon->mflag, MFLAG_VISIBLE) &&
		!is_mimicking(mon)) {
		/* Handle player fear */
		if (player_of_has(player, OF_AFRAID)) {
			/* Extract monster name (or "it") */
			char m_name[80];
			monster_desc(m_name, sizeof(m_name), mon, MDESC_DEFAULT);

			/* Message */
			msgt(MSG_AFRAID, "You are too afraid to attack %s!", m_name);

			/* Nope */
			return (false);
		}

		return (true);
	}

	/* If we don't know the grid, allow attempts to walk into it */
	if (!square_isknown(cave, y, x))
		return true;

	/* Require open space */
	if (!square_ispassable(cave, y, x)) {
		if (square_isrubble(cave, y, x))
			/* Rubble */
			msgt(MSG_HITWALL, "There is a pile of rubble in the way!");
		else if (square_iscloseddoor(cave, y, x))
			/* Door */
			return true;
		else if (square_isbright(cave, y, x))
			/* Lava */
			msgt(MSG_HITWALL, "The heat of the lava turns you away!");
		else
			/* Wall */
			msgt(MSG_HITWALL, "There is a wall in the way!");

		/* Cancel repeat */
		disturb(player, 0);

		/* Nope */
		return (false);
	}

	/* Okay */
	return (true);
}


/**
 * Walk in the given direction.
 */
void do_cmd_walk(struct command *cmd)
{
	int x, y, dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, false) != CMD_OK)
		return;

	/* Apply confusion if necessary */
	/* Confused movements use energy no matter what */
	if (player_confuse_dir(player, &dir, false))
		player->upkeep->energy_use = z_info->move_energy;
	
	/* Verify walkability */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	player->upkeep->energy_use = z_info->move_energy;

	move_player(dir, true);
}


/**
 * Walk into a trap.
 */
void do_cmd_jump(struct command *cmd)
{
	int x, y, dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, false) != CMD_OK)
		return;

	/* Apply confusion if necessary */
	if (player_confuse_dir(player, &dir, false))
		player->upkeep->energy_use = z_info->move_energy;

	/* Verify walkability */
	y = player->py + ddy[dir];
	x = player->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	player->upkeep->energy_use = z_info->move_energy;

	move_player(dir, false);
}


/**
 * Start running.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_run(struct command *cmd)
{
	int x, y, dir;

	/* Get arguments */
	if (cmd_get_direction(cmd, "direction", &dir, false) != CMD_OK)
		return;

	if (player_confuse_dir(player, &dir, true))
		return;

	/* Get location */
	if (dir) {
		y = player->py + ddy[dir];
		x = player->px + ddx[dir];
		if (!do_cmd_walk_test(y, x))
			return;
	}

	/* Start run */
	run_step(dir);
}


/**
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
		player->upkeep->running_withpathfind = true;
		run_step(0);
	}
}



/**
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_hold(struct command *cmd)
{
	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Spontaneous Searching */
	if ((player->state.skills[SKILL_SEARCH_FREQUENCY] >= 50) ||
	    one_in_(50 - player->state.skills[SKILL_SEARCH_FREQUENCY]))
		search(false);

	/* Continuous Searching */
	if (player->searching)
		search(false);

	/* Pick things up, not using extra energy */
	do_autopickup();

	/* Enter a store if we are on one, otherwise look at the floor */
	if (square_isshop(cave, player->py, player->px)) {
		disturb(player, 0);
		event_signal(EVENT_ENTER_STORE);
		event_remove_handler_type(EVENT_ENTER_STORE);
		event_signal(EVENT_USE_STORE);
		event_remove_handler_type(EVENT_USE_STORE);
		event_signal(EVENT_LEAVE_STORE);
		event_remove_handler_type(EVENT_LEAVE_STORE);

		/* Turn will be taken exiting the shop */
		player->upkeep->energy_use = 0;
	} else {
	    event_signal(EVENT_SEEFLOOR);
		floor_pile_know(cave, player->py, player->px);
	}
}


/**
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

	/* Do some upkeep on the first turn of rest */
	if (!player_is_resting(player)) {
		player->searching = false;
		player->upkeep->update |= (PU_BONUS);

		/* If a number of turns was entered, remember it */
		if (n > 1)
			player_set_resting_repeat_count(player, n);
		else if (n == 1)
			/* If we're repeating the command, use the same count */
			n = player_get_resting_repeat_count(player);
	}

	/* Set the counter, and stop if told to */
	player_resting_set_count(player, n);
	if (!player_is_resting(player))
		return;

	/* Take a turn */
	player_resting_step_turn(player);

	/* Redraw the state if requested */
	handle_stuff(player);

	/* Prepare to continue, or cancel and clean up */
	if (player_resting_count(player) > 0) {
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", n - 1);
	} else if (player_resting_is_special(n)) {
		cmdq_push(CMD_REST);
		cmd_set_arg_choice(cmdq_peek(), "choice", n);
		player_set_resting_repeat_count(player, 0);
	} else {
		player_resting_cancel(player, false);
	}

}


/**
 * Spend a turn doing nothing
 */
void do_cmd_sleep(struct command *cmd)
{
	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;
}


/**
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

/**
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

/**
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
	if (obj_only) {
		disturb(player, 0);
		msg("You feel that %s", obj_feeling_text[obj_feeling]);
		return;
	}

	/* Players automatically get a monster feeling. */
	if (cave->feeling_squares < z_info->feeling_need) {
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
	display_feeling(false);
}

