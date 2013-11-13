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
#include "game-cmd.h"
#include "game-event.h"
#include "generate.h"
#include "monster/mon-timed.h"
#include "monster/mon-util.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "pathfind.h"
#include "spells.h"
#include "squelch.h"
#include "trap.h"

/*
 * Go up one level
 */
void do_cmd_go_up(cmd_code code, cmd_arg args[])
{
	/* Verify stairs */
	if (!cave_isupstairs(cave, p_ptr->py, p_ptr->px)) {
		msg("I see no up staircase here.");
		return;
	}

	/* Force descend */
	if (OPT(birth_force_descend)) {
		msg("Nothing happens!");
		return;
	}

	/* Hack -- take a turn */
	p_ptr->energy_use = 100;

	/* Success */
	msgt(MSG_STAIRS_UP, "You enter a maze of up staircases.");

	/* Create a way back */
	p_ptr->create_up_stair = FALSE;
	p_ptr->create_down_stair = TRUE;

	/* Change level */
	dungeon_change_level(p_ptr->depth - 1);
}


/*
 * Go down one level
 */
void do_cmd_go_down(cmd_code code, cmd_arg args[])
{
	int descend_to = p_ptr->depth + 1;

	/* Verify stairs */
	if (!cave_isdownstairs(cave, p_ptr->py, p_ptr->px)) {
		msg("I see no down staircase here.");
		return;
	}

	/* Paranoia, no descent from MAX_DEPTH-1 */
	if (p_ptr->depth == MAX_DEPTH-1) {
		msg("The dungeon does not appear to extend deeper");
		return;
	}

	/* Warn a force_descend player if they're going to a quest level */
	if (OPT(birth_force_descend)) {
		if (is_quest(p_ptr->max_depth + 1) && !get_check("Are you sure you want to descend?"))
			return;

		/* Don't overshoot */
		descend_to = MIN(p_ptr->max_depth + 1, MAX_DEPTH-1);
	}

	/* Hack -- take a turn */
	p_ptr->energy_use = 100;

	/* Success */
	msgt(MSG_STAIRS_DOWN, "You enter a maze of down staircases.");

	/* Create a way back */
	p_ptr->create_up_stair = TRUE;
	p_ptr->create_down_stair = FALSE;

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
			msg("You can't make out your surroundings well enough to search.");

			/* Cancel repeat */
			disturb(p_ptr, 0, 0);
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
				if (cave_issecrettrap(cave, y, x))
				{
					found = TRUE;

					/* Pick a trap */
					pick_trap(y, x);

					/* Message */
					msg("You have found a trap.");

					/* Disturb */
					disturb(p_ptr, 0, 0);
				}

				/* Secret door */
				if (cave_issecretdoor(cave, y, x))
				{
					found = TRUE;

					/* Message */
					msg("You have found a secret door.");

					/* Pick a door */
					place_closed_door(cave, y, x);

					/* Disturb */
					disturb(p_ptr, 0, 0);
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
						disturb(p_ptr, 0, 0);
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
void do_cmd_search(cmd_code code, cmd_arg args[])
{
	/* Only take a turn if attempted */
	if (search(TRUE))
		p_ptr->energy_use = 100;
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(cmd_code code, cmd_arg args[])
{
	/* Stop searching */
	if (p_ptr->searching)
	{
		/* Clear the searching flag */
		p_ptr->searching = FALSE;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Start searching */
	else
	{
		/* Set the searching flag */
		p_ptr->searching = TRUE;

		/* Update stuff */
		p_ptr->update |= (PU_BONUS);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_STATE | PR_SPEED);
	}
}


/*
 * Return the number of doors/traps around (or under) the character.
 */
int count_feats(int *y, int *x, bool (*test)(struct cave *cave, int y, int x), bool under)
{
	int d;
	int xx, yy;
	int count = 0; /* Count how many matches */

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++)
	{
		/* if not searching under player continue */
		if ((d == 8) && !under) continue;

		/* Extract adjacent (legal) location */
		yy = p_ptr->py + ddy_ddd[d];
		xx = p_ptr->px + ddx_ddd[d];

		/* Paranoia */
		if (!cave_in_bounds_fully(cave, yy, xx)) continue;

		/* Must have knowledge */
		if (!(cave->info[yy][xx] & (CAVE_MARK))) continue;

		/* Not looking for this feature */
		if (!((*test)(cave, yy, xx))) continue;

		/* Count it */
		++count;

		/* Remember the location of the last door found */
		*y = yy;
		*x = xx;
	}

	/* All done */
	return count;
}


/*
 * Extract a "direction" which will move one step from the player location
 * towards the given "target" location (or "5" if no motion necessary).
 */
int coords_to_dir(int y, int x)
{
	return (motion_dir(p_ptr->py, p_ptr->px, y, x));
}


/*
 * Determine if a given grid may be "opened"
 */
static bool do_cmd_open_test(int y, int x)
{
	/* Must have knowledge */
	if (!(cave->info[y][x] & (CAVE_MARK))) {
		msg("You see nothing there.");
		return FALSE;
	}

	if (!cave_iscloseddoor(cave, y, x)) {
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
	if (cave_islockeddoor(cave, y, x))
	{
		/* Disarm factor */
		i = p_ptr->state.skills[SKILL_DISARM];

		/* Penalize some conditions */
		if (p_ptr->timed[TMD_BLIND] || no_light()) i = i / 10;
		if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE]) i = i / 10;

		/* Extract the lock power */
		j = cave_door_power(cave, y, x);

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
			cave_open_door(cave, y, x);

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Experience */
			/* Removed to avoid exploit by repeatedly locking and unlocking door */
			/* player_exp_gain(p_ptr, 1); */
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
		cave_open_door(cave, y, x);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

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
void do_cmd_open(cmd_code code, cmd_arg args[])
{
	int y, x, dir;

	s16b o_idx;

	bool more = FALSE;

	dir = args[0].direction;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Check for chests */
	o_idx = chest_check(y, x, CHEST_OPENABLE);


	/* Verify legality */
	if (!o_idx && !do_cmd_open_test(y, x))
	{
		/* Cancel repeat */
		disturb(p_ptr, 0, 0);
		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(p_ptr, &dir, FALSE))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Check for chest */
		o_idx = chest_check(y, x, CHEST_OPENABLE);
	}


	/* Monster */
	if (cave->m_idx[y][x] > 0)
	{
		int m_idx = cave->m_idx[y][x];
		struct monster *m_ptr = cave_monster(cave, m_idx);

		/* Mimics surprise the player */
		if (is_mimicking(m_ptr)) {
			become_aware(m_ptr);

			/* Mimic wakes up */
			mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);
		} else {
			/* Message */
			msg("There is a monster in the way!");

			/* Attack */
			py_attack(y, x);
		}
	}

	/* Chest */
	else if (o_idx)
	{
		/* Open the chest */
		more = do_cmd_open_chest(y, x, o_idx);
	}

	/* Door */
	else
	{
		/* Open the door */
		more = do_cmd_open_aux(y, x);
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(p_ptr, 0, 0);
}


/*
 * Determine if a given grid may be "closed"
 */
static bool do_cmd_close_test(int y, int x)
{
	/* Must have knowledge */
	if (!(cave->info[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg("You see nothing there.");

		/* Nope */
		return (FALSE);
	}

 	/* Require open/broken door */
	if (!cave_isopendoor(cave, y, x) && !cave_isbrokendoor(cave, y, x))
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
	if (cave_isbrokendoor(cave, y, x))
	{
		/* Message */
		msg("The door appears to be broken.");
	}

	/* Open door */
	else
	{
		/* Close the door */
		cave_close_door(cave, y, x);

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Sound */
		sound(MSG_SHUTDOOR);
	}

	/* Result */
	return (more);
}


/*
 * Close an open door.
 */
void do_cmd_close(cmd_code code, cmd_arg args[])
{
	int y, x, dir;

	bool more = FALSE;

	dir = args[0].direction;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Verify legality */
	if (!do_cmd_close_test(y, x))
	{
		/* Cancel repeat */
		disturb(p_ptr, 0, 0);
		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(p_ptr, &dir, FALSE))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
	}


	/* Monster */
	if (cave->m_idx[y][x] > 0)
	{
		/* Message */
		msg("There is a monster in the way!");

		/* Attack */
		py_attack(y, x);
	}

	/* Door */
	else
	{
		/* Close door */
		more = do_cmd_close_aux(y, x);
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(p_ptr, 0, 0);
}


/*
 * Determine if a given grid may be "tunneled"
 */
static bool do_cmd_tunnel_test(int y, int x)
{
	/* Must have knowledge */
	if (!(cave->info[y][x] & (CAVE_MARK)))
	{
		/* Message */
		msg("You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Must be a wall/door/etc */
	if (cave_ispassable(cave, y, x))
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
	if (cave_ispassable(cave, y, x)) return (FALSE);

	/* Sound */
	sound(MSG_DIG);

	/* Forget the wall */
	cave->info[y][x] &= ~(CAVE_MARK);

	/* Remove the feature */
	cave_tunnel_wall(cave, y, x);

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

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


	/* Verify legality */
	if (!do_cmd_tunnel_test(y, x)) return (FALSE);


	/* Sound XXX XXX XXX */
	/* sound(MSG_DIG); */

	/* Titanium */
	if (cave_isperm(cave, y, x))
	{
		msg("This seems to be permanent rock.");
	}

	/* Granite */
	else if (cave_isrock(cave, y, x))
	{
		/* Tunnel */
		if ((p_ptr->state.skills[SKILL_DIGGING] > 40 + randint0(1600)) && twall(y, x))
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
	else if (cave_ismagma(cave, y, x) || cave_isquartz(cave, y, x))
	{
		bool okay = FALSE;
		bool gold = FALSE;
		bool hard = FALSE;

		/* Found gold */
		if (cave_hasgoldvein(cave, y, x))
			gold = TRUE;

		/* Extract "quartz" flag XXX XXX XXX */
		if (cave_isquartz(cave, y, x))
			hard = TRUE;

		/* Quartz */
		if (hard)
			okay = (p_ptr->state.skills[SKILL_DIGGING] > 20 + randint0(800));
		/* Magma */
		else
			okay = (p_ptr->state.skills[SKILL_DIGGING] > 10 + randint0(400));

		/* Success */
		if (okay && twall(y, x))
		{
			/* Found treasure */
			if (gold)
			{
				/* Place some gold */
				place_gold(cave, y, x, p_ptr->depth, ORIGIN_FLOOR);

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
	else if (cave_isrubble(cave, y, x))
	{
		/* Remove the rubble */
		if ((p_ptr->state.skills[SKILL_DIGGING] > randint0(200)) && twall(y, x))
		{
			/* Message */
			msg("You have removed the rubble.");

			/* Hack -- place an object */
			if (randint0(100) < 10)	{
				/* Create a simple object */
				place_object(cave, y, x, p_ptr->depth, FALSE, FALSE,
					ORIGIN_RUBBLE, 0);

				/* Observe the new object */
				if (!squelch_item_ok(object_byid(cave->o_idx[y][x])) &&
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
	else if (cave_issecretdoor(cave, y, x))
	{
		/* Tunnel */
		if ((p_ptr->state.skills[SKILL_DIGGING] > 30 + randint0(1200)) && twall(y, x))
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
		if ((p_ptr->state.skills[SKILL_DIGGING] > 30 + randint0(1200)) && twall(y, x))
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
void do_cmd_tunnel(cmd_code code, cmd_arg args[])
{
	int y, x, dir;
	bool more = FALSE;

	dir = args[0].direction;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];


	/* Oops */
	if (!do_cmd_tunnel_test(y, x))
	{
		/* Cancel repeat */
		disturb(p_ptr, 0, 0);
		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(p_ptr, &dir, FALSE))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
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
	if (!more) disturb(p_ptr, 0, 0);
}

/*
 * Determine if a given grid may be "disarmed"
 */
static bool do_cmd_disarm_test(int y, int x)
{
	/* Must have knowledge */
	if (!(cave->info[y][x] & (CAVE_MARK))) {
		msg("You see nothing there.");
		return FALSE;
	}

	/* Look for a closed, unlocked door to lock */
	if (cave_iscloseddoor(cave, y, x) && !cave_islockeddoor(cave, y, x))
		return TRUE;

	/* Look for a trap */
	if (!cave_isknowntrap(cave, y, x)) {
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
	i = p_ptr->state.skills[SKILL_DISARM];

	/* Penalize some conditions */
	if (p_ptr->timed[TMD_BLIND] || no_light())
		i = i / 10;
	if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE])
		i = i / 10;

	/* Calculate lock "power" */
	power = m_bonus(7, p_ptr->depth);

	/* Extract the difficulty */
	j = i - power;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Success */
	if (randint0(100) < j) {
		msg("You lock the door.");
		cave_lock_door(cave, y, x, power);
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

	const char *name;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_disarm_test(y, x)) return (FALSE);


	/* Get the trap name */
	name = f_info[cave->feat[y][x]].name;

	/* Get the "disarm" factor */
	i = p_ptr->state.skills[SKILL_DISARM];

	/* Penalize some conditions */
	if (p_ptr->timed[TMD_BLIND] || no_light()) i = i / 10;
	if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE]) i = i / 10;

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
		msgt(MSG_DISARM, "You have disarmed the %s.", name);

		/* Reward */
		player_exp_gain(p_ptr, power);

		/* Forget the trap */
		cave->info[y][x] &= ~(CAVE_MARK);

		/* Remove the trap */
		cave_destroy_trap(cave, y, x);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint1(i) > 5))
	{
		flush();

		/* Message */
		msg("You failed to disarm the %s.", name);

		/* We may keep trying */
		more = TRUE;
	}

	/* Failure -- Set off the trap */
	else
	{
		/* Message */
		msg("You set off the %s!", name);

		/* Hit the trap */
		hit_trap(y, x);
	}

	/* Result */
	return (more);
}


/*
 * Disarms a trap, or a chest
 */
void do_cmd_disarm(cmd_code code, cmd_arg args[])
{
	int y, x, dir;

	s16b o_idx;

	bool more = FALSE;

	dir = args[0].direction;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Check for chests */
	o_idx = chest_check(y, x, CHEST_TRAPPED);


	/* Verify legality */
	if (!o_idx && !do_cmd_disarm_test(y, x))
	{
		/* Cancel repeat */
		disturb(p_ptr, 0, 0);
		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(p_ptr, &dir, FALSE))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

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
	else if (    cave_iscloseddoor(cave, y, x)
	         && !cave_islockeddoor(cave, y, x))
		more = do_cmd_lock_door(y, x);

	/* Disarm trap */
	else
		more = do_cmd_disarm_aux(y, x);

	/* Cancel repeat unless told not to */
	if (!more) disturb(p_ptr, 0, 0);
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
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Apply confusion */
	if (player_confuse_dir(p_ptr, &dir, FALSE)) {
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
	}

	/* Attack monsters */
	if (cave->m_idx[y][x] > 0)
		py_attack(y, x);

	/* Tunnel through walls and rubble */
	else if (cave_isdiggable(cave, y, x))
		more = do_cmd_tunnel_aux(y, x);

	/* Open closed doors */
	else if (cave_iscloseddoor(cave, y, x))
		more = do_cmd_open_aux(y, x);

	/* Disarm traps */
	else if (cave_isknowntrap(cave, y, x))
		more = do_cmd_disarm_aux(y, x);

	/* Oops */
	else
		msg("You spin around.");

	/* Cancel repetition unless we can continue */
	if (!more) disturb(p_ptr, 0, 0);
}

void do_cmd_alter(cmd_code code, cmd_arg args[])
{
	do_cmd_alter_aux(args[0].direction);
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
		if (check_state(p_ptr, OF_AFRAID, p_ptr->state.flags))
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
	if (!(cave->info[y][x] & CAVE_MARK))
		return TRUE;

	/* Require open space */
	if (!cave_ispassable(cave, y, x))
	{
		/* Rubble */
		if (cave_isrubble(cave, y, x))
			msgt(MSG_HITWALL, "There is a pile of rubble in the way!");

		/* Door */
		else if (cave_iscloseddoor(cave, y, x))
			return TRUE;

		/* Wall */
		else
			msgt(MSG_HITWALL, "There is a wall in the way!");

		/* Cancel repeat */
		disturb(p_ptr, 0, 0);

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Walk in the given direction.
 */
void do_cmd_walk(cmd_code code, cmd_arg args[])
{
	int x, y;
	int dir = args[0].direction;

	/* Apply confusion if necessary */
	player_confuse_dir(p_ptr, &dir, FALSE);

	/* Confused movements use energy no matter what */
	if (dir != args[0].direction)	
		p_ptr->energy_use = 100;
	
	/* Verify walkability */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	p_ptr->energy_use = 100;

	move_player(dir, TRUE);
}


/*
 * Walk into a trap.
 */
void do_cmd_jump(cmd_code code, cmd_arg args[])
{
	int x, y;
	int dir = args[0].direction;

	/* Apply confusion if necessary */
	player_confuse_dir(p_ptr, &dir, FALSE);

	/* Verify walkability */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];
	if (!do_cmd_walk_test(y, x))
		return;

	p_ptr->energy_use = 100;

	move_player(dir, FALSE);
}


/*
 * Start running.
 *
 * Note that running while confused is not allowed.
 */
void do_cmd_run(cmd_code code, cmd_arg args[])
{
	int x, y;
	int dir = args[0].direction;

	if (player_confuse_dir(p_ptr, &dir, TRUE))
	{
		return;
	}

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];
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
void do_cmd_pathfind(cmd_code code, cmd_arg args[])
{
	/* Hack XXX XXX XXX */
	int dir = 5;
	if (player_confuse_dir(p_ptr, &dir, TRUE))
	{
		return;
	}

	if (findpath(args[0].point.x, args[0].point.y))
	{
		p_ptr->running = 1000;
		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);
		p_ptr->running_withpathfind = TRUE;
		run_step(0);
	}
}



/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_hold(cmd_code code, cmd_arg args[])
{
	/* Take a turn */
	p_ptr->energy_use = 100;

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

	/* Pick things up, not using extra energy */
	do_autopickup();

	/* Hack -- enter a store if we are on one */
	if (cave_isshop(cave, p_ptr->py, p_ptr->px)) {
		/* Disturb */
		disturb(p_ptr, 0, 0);

		cmd_insert(CMD_ENTER_STORE);

		/* Free turn XXX XXX XXX */
		p_ptr->energy_use = 0;
	}
	else
	{
	    event_signal(EVENT_SEEFLOOR);
	}
}


/*
 * Rest (restores hit points and mana and such)
 */
void do_cmd_rest(cmd_code code, cmd_arg args[])
{
	/* 
	 * A little sanity checking on the input - only the specified negative 
	 * values are valid. 
	 */
    if (args[0].choice < 0 && !player_resting_is_special(args[0].choice))
        return;

	player_resting_set_count(p_ptr, args[0].choice);

	/* Take a turn XXX XXX XXX (?) */
	p_ptr->energy_use = 100;

	/* Cancel searching */
	p_ptr->searching = FALSE;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff(p_ptr);

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
		cmd_insert(CMD_REST);
		cmd_set_arg_choice(cmd_get_top(), 0, REST_COMPLETE);
	}

	/* Rest a lot */
	else if (out_val[0] == '*')
	{
		cmd_insert(CMD_REST);
		cmd_set_arg_choice(cmd_get_top(), 0, REST_ALL_POINTS);
	}

	/* Rest until HP or SP filled */
	else if (out_val[0] == '!')
	{
		cmd_insert(CMD_REST);
		cmd_set_arg_choice(cmd_get_top(), 0, REST_SOME_POINTS);
	}
	
	/* Rest some */
	else
	{
		int turns = atoi(out_val);
		if (turns <= 0) return;
		if (turns > 9999) turns = 9999;
		
		cmd_insert(CMD_REST);
		cmd_set_arg_choice(cmd_get_top(), 0, turns);
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
	if (!p_ptr->depth) {
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

