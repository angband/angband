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
#include "files.h"
#include "game-cmd.h"
#include "game-event.h"
#include "generate.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "spells.h"
#include "squelch.h"
#include "trap.h"

/*
 * Go up one level
 */
void do_cmd_go_up(cmd_code code, cmd_arg args[])
{
	/* Verify stairs */
	if (cave->feat[p_ptr->py][p_ptr->px] != FEAT_LESS)
	{
		msg("I see no up staircase here.");
		return;
	}

	/* Ironman */
	if (OPT(birth_ironman))
	{
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
	/* Verify stairs */
	if (cave->feat[p_ptr->py][p_ptr->px] != FEAT_MORE)
	{
		msg("I see no down staircase here.");
		return;
	}

	/* Hack -- take a turn */
	p_ptr->energy_use = 100;

	/* Success */
	msgt(MSG_STAIRS_DOWN, "You enter a maze of down staircases.");

	/* Create a way back */
	p_ptr->create_up_stair = TRUE;
	p_ptr->create_down_stair = FALSE;

	/* Change level */
	dungeon_change_level(p_ptr->depth + 1);
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
 * Determine if a grid contains a chest
 */
static s16b chest_check(int y, int x)
{
	s16b this_o_idx, next_o_idx = 0;


	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Skip unknown chests XXX XXX */
		/* if (!o_ptr->marked) continue; */

		/* Check for chest */
		if (o_ptr->tval == TV_CHEST) return (this_o_idx);
	}

	/* No chest */
	return (0);
}


/*
 * Allocate objects upon opening a chest
 *
 * Disperse treasures from the given chest, centered at (x,y).
 *
 * Small chests often contain "gold", while Large chests always contain
 * items.  Wooden chests contain 2 items, Iron chests contain 4 items,
 * and Steel chests contain 6 items.  The "value" of the items in a
 * chest is based on the level on which the chest is generated.
 */
static void chest_death(int y, int x, s16b o_idx)
{
	int number, value;

	bool tiny;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;


	/* Get the chest */
	o_ptr = object_byid(o_idx);

	/* Small chests often hold "gold" */
	tiny = (o_ptr->sval < SV_CHEST_MIN_LARGE);

	/* Determine how much to drop (see above) */
	number = (o_ptr->sval % SV_CHEST_MIN_LARGE) * 2;

	/* Zero pval means empty chest */
	if (!o_ptr->pval[DEFAULT_PVAL]) number = 0;

	/* Determine the "value" of the items */
	value = o_ptr->origin_depth - 10 + 2 * o_ptr->sval;
	if (value < 1)
		value = 1;

	/* Drop some objects (non-chests) */
	for (; number > 0; --number)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Small chests often drop gold */
		if (tiny && (randint0(100) < 75))
			make_gold(i_ptr, value, SV_GOLD_ANY);

		/* Otherwise drop an item, as long as it isn't a chest */
		else {
			make_object(cave, i_ptr, value, FALSE, FALSE);
			if (i_ptr->tval == TV_CHEST || !i_ptr->kind)
				continue;
		}

		/* Record origin */
		i_ptr->origin = ORIGIN_CHEST;
		i_ptr->origin_depth = o_ptr->origin_depth;

		/* Drop it in the dungeon */
		drop_near(cave, i_ptr, 0, y, x, TRUE);
	}

	/* Empty */
	o_ptr->pval[DEFAULT_PVAL] = 0;

	/* Known */
	object_notice_everything(o_ptr);
}


/*
 * Chests have traps too.
 *
 * Exploding chest destroys contents (and traps).
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int y, int x, s16b o_idx)
{
	int i, trap;

	object_type *o_ptr = object_byid(o_idx);


	/* Ignore disarmed chests */
	if (o_ptr->pval[DEFAULT_PVAL] <= 0) return;

	/* Obtain the traps */
	trap = chest_traps[o_ptr->pval[DEFAULT_PVAL]];

	/* Lose strength */
	if (trap & (CHEST_LOSE_STR))
	{
		msg("A small needle has pricked you!");
		take_hit(p_ptr, damroll(1, 4), "a poison needle");
		(void)do_dec_stat(A_STR, FALSE);
	}

	/* Lose constitution */
	if (trap & (CHEST_LOSE_CON))
	{
		msg("A small needle has pricked you!");
		take_hit(p_ptr, damroll(1, 4), "a poison needle");
		(void)do_dec_stat(A_CON, FALSE);
	}

	/* Poison */
	if (trap & (CHEST_POISON))
	{
		msg("A puff of green gas surrounds you!");
		(void)player_inc_timed(p_ptr, TMD_POISONED, 10 + randint1(20), TRUE, TRUE);
	}

	/* Paralyze */
	if (trap & (CHEST_PARALYZE))
	{
		msg("A puff of yellow gas surrounds you!");
		(void)player_inc_timed(p_ptr, TMD_PARALYZED, 10 + randint1(20), TRUE, TRUE);
	}

	/* Summon monsters */
	if (trap & (CHEST_SUMMON))
	{
		int num = 2 + randint1(3);
		msg("You are enveloped in a cloud of smoke!");
		sound(MSG_SUM_MONSTER);
		for (i = 0; i < num; i++)
		{
			(void)summon_specific(y, x, p_ptr->depth, 0, 1);
		}
	}

	/* Explode */
	if (trap & (CHEST_EXPLODE))
	{
		msg("There is a sudden explosion!");
		msg("Everything inside the chest is destroyed!");
		o_ptr->pval[DEFAULT_PVAL] = 0;
		take_hit(p_ptr, damroll(5, 8), "an exploding chest");
	}
}


/*
 * Attempt to open the given chest at the given location
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_open_chest(int y, int x, s16b o_idx)
{
	int i, j;

	bool flag = TRUE;

	bool more = FALSE;

	object_type *o_ptr = object_byid(o_idx);


	/* Attempt to unlock it */
	if (o_ptr->pval[DEFAULT_PVAL] > 0)
	{
		/* Assume locked, and thus not open */
		flag = FALSE;

		/* Get the "disarm" factor */
		i = p_ptr->state.skills[SKILL_DISARM];

		/* Penalize some conditions */
		if (p_ptr->timed[TMD_BLIND] || no_light()) i = i / 10;
		if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE]) i = i / 10;

		/* Extract the difficulty */
		j = i - o_ptr->pval[DEFAULT_PVAL];

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success -- May still have traps */
		if (randint0(100) < j)
		{
			msgt(MSG_LOCKPICK, "You have picked the lock.");
			player_exp_gain(p_ptr, 1);
			flag = TRUE;
		}

		/* Failure -- Keep trying */
		else
		{
			/* We may continue repeating */
			more = TRUE;
			flush();
			msgt(MSG_LOCKPICK_FAIL, "You failed to pick the lock.");
		}
	}

	/* Allowed to open */
	if (flag)
	{
		/* Apply chest traps, if any */
		chest_trap(y, x, o_idx);

		/* Let the Chest drop items */
		chest_death(y, x, o_idx);

		/* Squelch chest if autosquelch calls for it */
		p_ptr->notice |= PN_SQUELCH;

		/* Redraw chest, to be on the safe side (it may have been squelched) */
		cave_light_spot(cave, y, x);
	}

	/* Result */
	return (more);
}


/*
 * Attempt to disarm the chest at the given location
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_disarm_chest(int y, int x, s16b o_idx)
{
	int i, j;

	bool more = FALSE;

	object_type *o_ptr = object_byid(o_idx);


	/* Get the "disarm" factor */
	i = p_ptr->state.skills[SKILL_DISARM];

	/* Penalize some conditions */
	if (p_ptr->timed[TMD_BLIND] || no_light()) i = i / 10;
	if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE]) i = i / 10;

	/* Extract the difficulty */
	j = i - o_ptr->pval[DEFAULT_PVAL];

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Must find the trap first. */
	if (!object_is_known(o_ptr))
	{
		msg("I don't see any traps.");
	}

	/* Already disarmed/unlocked */
	else if (o_ptr->pval[DEFAULT_PVAL] <= 0)
	{
		msg("The chest is not trapped.");
	}

	/* No traps to find. */
	else if (!chest_traps[o_ptr->pval[DEFAULT_PVAL]])
	{
		msg("The chest is not trapped.");
	}

	/* Success (get a lot of experience) */
	else if (randint0(100) < j)
	{
		msgt(MSG_DISARM, "You have disarmed the chest.");
		player_exp_gain(p_ptr, o_ptr->pval[DEFAULT_PVAL]);
		o_ptr->pval[DEFAULT_PVAL] = (0 - o_ptr->pval[DEFAULT_PVAL]);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint1(i) > 5))
	{
		/* We may keep trying */
		more = TRUE;
		flush();
		msg("You failed to disarm the chest.");
	}

	/* Failure -- Set off the trap */
	else
	{
		msg("You set off a trap!");
		chest_trap(y, x, o_idx);
	}

	/* Result */
	return (more);
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
		if (!in_bounds_fully(yy, xx)) continue;

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
 * Return the number of chests around (or under) the character.
 * If requested, count only trapped chests.
 */
int count_chests(int *y, int *x, bool trapped)
{
	int d, count, o_idx;

	object_type *o_ptr;

	/* Count how many matches */
	count = 0;

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++)
	{
		/* Extract adjacent (legal) location */
		int yy = p_ptr->py + ddy_ddd[d];
		int xx = p_ptr->px + ddx_ddd[d];

		/* No (visible) chest is there */
		if ((o_idx = chest_check(yy, xx)) == 0) continue;

		/* Grab the object */
		o_ptr = object_byid(o_idx);

		/* Already open */
		if (o_ptr->pval[DEFAULT_PVAL] == 0) continue;

		/* No (known) traps here */
		if (trapped &&
		    (!object_is_known(o_ptr) ||
		     (o_ptr->pval[DEFAULT_PVAL] < 0) ||
		     !chest_traps[o_ptr->pval[DEFAULT_PVAL]]))
		{
			continue;
		}

		/* Count it */
		++count;

		/* Remember the location of the last chest found */
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


	/* Jammed door */
	if (cave_isjammeddoor(cave, y, x))
	{
		msg("The door appears to be stuck.");
	}

	/* Locked door */
	else if (cave_islockeddoor(cave, y, x))
	{
		/* Disarm factor */
		i = p_ptr->state.skills[SKILL_DISARM];

		/* Penalize some conditions */
		if (p_ptr->timed[TMD_BLIND] || no_light()) i = i / 10;
		if (p_ptr->timed[TMD_CONFUSED] || p_ptr->timed[TMD_IMAGE]) i = i / 10;

		/* Extract the lock power */
		j = cave->feat[y][x] - FEAT_DOOR_HEAD;

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
			cave_set_feat(cave, y, x, FEAT_OPEN);

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
		cave_set_feat(cave, y, x, FEAT_OPEN);

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
	o_idx = chest_check(y, x);


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
		o_idx = chest_check(y, x);
	}


	/* Monster */
	if (cave->m_idx[y][x] > 0)
	{
		/* Message */
		msg("There is a monster in the way!");

		/* Attack */
		py_attack(y, x);
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
	if ((cave->feat[y][x] != FEAT_OPEN) &&
	    (cave->feat[y][x] != FEAT_BROKEN))
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
	if (cave->feat[y][x] == FEAT_BROKEN)
	{
		/* Message */
		msg("The door appears to be broken.");
	}

	/* Open door */
	else
	{
		/* Close the door */
		cave_set_feat(cave, y, x, FEAT_DOOR_HEAD);

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
	if (cave_floor_bold(y, x))
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
	if (cave_floor_bold(y, x)) return (FALSE);

	/* Sound */
	sound(MSG_DIG);

	/* Forget the wall */
	cave->info[y][x] &= ~(CAVE_MARK);

	/* Remove the feature */
	cave_set_feat(cave, y, x, FEAT_FLOOR);

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
	if (cave->feat[y][x] >= FEAT_PERM_EXTRA)
	{
		msg("This seems to be permanent rock.");
	}

	/* Granite */
	else if (cave->feat[y][x] >= FEAT_WALL_EXTRA)
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
	else if (cave->feat[y][x] >= FEAT_MAGMA)
	{
		bool okay = FALSE;
		bool gold = FALSE;
		bool hard = FALSE;

		/* Found gold */
		if (cave->feat[y][x] >= FEAT_MAGMA_H)
		{
			gold = TRUE;
		}

		/* Extract "quartz" flag XXX XXX XXX */
		if ((cave->feat[y][x] - FEAT_MAGMA) & 0x01)
		{
			hard = TRUE;
		}

		/* Quartz */
		if (hard)
		{
			okay = (p_ptr->state.skills[SKILL_DIGGING] > 20 + randint0(800));
		}

		/* Magma */
		else
		{
			okay = (p_ptr->state.skills[SKILL_DIGGING] > 10 + randint0(400));
		}

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
	else if (cave->feat[y][x] == FEAT_RUBBLE)
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
					ORIGIN_RUBBLE);

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
	else if (cave->feat[y][x] >= FEAT_SECRET)
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
	if (cave->feat[y][x] == FEAT_DOOR_HEAD)	return TRUE;

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
		cave_set_feat(cave, y, x, FEAT_DOOR_HEAD + power);
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
		cave_set_feat(cave, y, x, FEAT_FLOOR);
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
	o_idx = chest_check(y, x);


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
		o_idx = chest_check(y, x);
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
	else if (cave->feat[y][x] == FEAT_DOOR_HEAD)
		more = do_cmd_lock_door(y, x);

	/* Disarm trap */
	else
		more = do_cmd_disarm_aux(y, x);

	/* Cancel repeat unless told not to */
	if (!more) disturb(p_ptr, 0, 0);
}


/*
 * Determine if a given grid may be "bashed"
 */
static bool do_cmd_bash_test(int y, int x)
{
	/* Must have knowledge */
	if (!(cave->info[y][x] & (CAVE_MARK))) {
		msg("You see nothing there.");
		return (FALSE);
	}

	if (!cave_iscloseddoor(cave, y, x)) {
		msg("You see nothing there to bash.");
		return FALSE;
	}

	/* Okay */
	return (TRUE);
}


/*
 * Perform the basic "bash" command
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool do_cmd_bash_aux(int y, int x)
{
	int bash, temp;

	bool more = FALSE;


	/* Verify legality */
	if (!do_cmd_bash_test(y, x)) return (FALSE);


	/* Message */
	msg("You smash into the door!");

	/* Hack -- Bash power based on strength */
	/* (Ranges from 3 to 20 to 100 to 200) */
	bash = adj_str_blow[p_ptr->state.stat_ind[A_STR]];

	/* Extract door power */
	temp = ((cave->feat[y][x] - FEAT_DOOR_HEAD) & 0x07);

	/* Compare bash power to door power */
	temp = (bash - (temp * 10));

	/* Hack -- always have a chance */
	if (temp < 1) temp = 1;

	/* Hack -- attempt to bash down the door */
	if (randint0(100) < temp)
	{
		/* Break down the door */
		if (randint0(100) < 50)
		{
			cave_set_feat(cave, y, x, FEAT_BROKEN);
		}

		/* Open the door */
		else
		{
			cave_set_feat(cave, y, x, FEAT_OPEN);
		}

		msgt(MSG_OPENDOOR, "The door crashes open!");

		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}

	/* Saving throw against stun */
	else if (randint0(100) < adj_dex_safe[p_ptr->state.stat_ind[A_DEX]] +
	         p_ptr->lev) {
		msg("The door holds firm.");

		/* Allow repeated bashing */
		more = TRUE;
	}

	/* Low dexterity has bad consequences */
	else {
		msg("You are off-balance.");

		/* Lose balance ala stun */
		(void)player_inc_timed(p_ptr, TMD_STUN, 2 + randint0(2), TRUE, FALSE);
	}

	/* Result */
	return more;
}


/*
 * Bash open a door, success based on character strength
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open or bash doors, see elsewhere.
 */
void do_cmd_bash(cmd_code code, cmd_arg args[])
{
	int y, x, dir;
	bool more = FALSE;

	dir = args[0].direction;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_bash_test(y, x))
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
		/* Bash the door */
		more = do_cmd_bash_aux(y, x);
	}

	/* Cancel repeat unless we may continue */
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

	int feat;

	bool more = FALSE;

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];


	/* Original feature */
	feat = cave->feat[y][x];

	/* Must have knowledge to know feature XXX XXX */
	if (!(cave->info[y][x] & (CAVE_MARK))) feat = FEAT_NONE;


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
 * Find the index of some "spikes", if possible.
 *
 * XXX XXX XXX Let user choose a pile of spikes, perhaps?
 */
static bool get_spike(int *ip)
{
	int i;

	/* Check every item in the pack */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Check the "tval" code */
		if (o_ptr->tval == TV_SPIKE)
		{
			/* Save the spike index */
			(*ip) = i;

			/* Success */
			return (TRUE);
		}
	}

	/* Oops */
	return (FALSE);
}


/*
 * Determine if a given grid may be "spiked"
 */
static bool do_cmd_spike_test(int y, int x)
{
	/* Must have knowledge */
	if (!(cave->info[y][x] & (CAVE_MARK))) {
		msg("You see nothing there.");
		return FALSE;
	}

	/* Check if door is closed */
	if (!cave_iscloseddoor(cave, y, x)) {
		msg("You see nothing there to spike.");
		return FALSE;
	}

	/* Check that the door is not fully spiked */
	if (!(cave->feat[y][x] < FEAT_DOOR_TAIL)) {
		msg("You can't use more spikes on this door.");
		return FALSE;
	}

	/* Okay */
	return TRUE;
}


/*
 * Jam a closed door with a spike
 *
 * This command may NOT be repeated
 */
void do_cmd_spike(cmd_code code, cmd_arg args[])
{
	int y, x, dir, item = 0;

	dir = args[0].direction;

	/* Get a spike */
	if (!get_spike(&item))
	{
		/* Message */
		msg("You have no spikes!");

		/* Done */
		return;
	}

	/* Get location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];


	/* Verify legality */
	if (!do_cmd_spike_test(y, x)) return;


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

	/* Go for it */
	else
	{
		/* Verify legality */
		if (!do_cmd_spike_test(y, x)) return;

		/* Successful jamming */
		msg("You jam the door with a spike.");

		/* Convert "locked" to "stuck" XXX XXX XXX */
		if (cave->feat[y][x] < FEAT_DOOR_HEAD + 0x08)
			cave->feat[y][x] += 0x08;

		/* Add one spike to the door */
		if (cave->feat[y][x] < FEAT_DOOR_TAIL)
			cave->feat[y][x] += 0x01;

		/* Use up, and describe, a single spike, from the bottom */
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}
}


/*
 * Determine if a given grid may be "walked"
 */
static bool do_cmd_walk_test(int y, int x)
{
	/* Allow attack on visible monsters if unafraid */
	if ((cave->m_idx[y][x] > 0) && (cave_monster(cave, cave->m_idx[y][x])->ml))
	{
		/* Handle player fear */
		if(check_state(p_ptr, OF_AFRAID, p_ptr->state.flags))
		{
			/* Extract monster name (or "it") */
			char m_name[80];
			monster_type *m_ptr;

			m_ptr = cave_monster(cave, cave->m_idx[y][x]);
			monster_desc(m_name, sizeof(m_name), m_ptr, 0);

			/* Message */
			msgt(MSG_AFRAID,
				"You are too afraid to attack %s!", m_name);

			/* Nope */
			return (FALSE);
		}
		
		return (TRUE);
	}

	/* If we don't know the grid, allow attempts to walk into it */
	if (!(cave->info[y][x] & CAVE_MARK))
		return TRUE;

	/* Require open space */
	if (!cave_floor_bold(y, x))
	{
		/* Rubble */
		if (cave->feat[y][x] == FEAT_RUBBLE)
			msgt(MSG_HITWALL, "There is a pile of rubble in the way!");

		/* Door */
		else if (cave->feat[y][x] < FEAT_SECRET)
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

	if (findpath(args[0].point.y, args[0].point.x))
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
	if ((cave->feat[p_ptr->py][p_ptr->px] >= FEAT_SHOP_HEAD) &&
	    (cave->feat[p_ptr->py][p_ptr->px] <= FEAT_SHOP_TAIL))
	{
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
 * Pick up objects on the floor beneath you.  -LM-
 */
void do_cmd_pickup(cmd_code code, cmd_arg args[])
{
	int energy_cost;

	/* Pick up floor objects, forcing a menu for multiple objects. */
	energy_cost = py_pickup(1) * 10;

	/* Charge this amount of energy. */
	p_ptr->energy_use = energy_cost;
}

/*
 * Pick up objects on the floor beneath you.  -LM-
 */
void do_cmd_autopickup(cmd_code code, cmd_arg args[])
{
	p_ptr->energy_use = do_autopickup() * 10;
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
	if ((args[0].choice < 0) &&
		((args[0].choice != REST_COMPLETE) &&
		 (args[0].choice != REST_ALL_POINTS) &&
		 (args[0].choice != REST_SOME_POINTS))) 
	{
		return;
	}

	/* Save the rest code */
	p_ptr->resting = args[0].choice;
	
	/* Truncate overlarge values */
	if (p_ptr->resting > 9999) p_ptr->resting = 9999;
	
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
  	/* Prompt for time if needed */
	if (p_ptr->command_arg <= 0)
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
}


/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(cmd_code code, cmd_arg args[])
{
	/* Commit suicide */
	p_ptr->is_dead = TRUE;

	/* Stop playing */
	p_ptr->playing = FALSE;

	/* Leaving */
	p_ptr->leaving = TRUE;

	/* Cause of death */
	my_strcpy(p_ptr->died_from, "Quitting", sizeof(p_ptr->died_from));
}


void textui_cmd_suicide(void)
{
	/* Flush input */
	flush();

	/* Verify Retirement */
	if (p_ptr->total_winner)
	{
		/* Verify */
		if (!get_check("Do you want to retire? ")) return;
	}

	/* Verify Suicide */
	else
	{
		struct keypress ch;

		/* Verify */
		if (!get_check("Do you really want to commit suicide? ")) return;

		/* Special Verification for suicide */
		prt("Please verify SUICIDE by typing the '@' sign: ", 0, 0);
		flush();
		ch = inkey();
		prt("", 0, 0);
		if (ch.code != '@') return;
	}

	cmd_insert(CMD_SUICIDE);
}

void do_cmd_save_game(cmd_code code, cmd_arg args[])
{
	save_game();
}

