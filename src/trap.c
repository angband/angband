/**
 * \file trap.c
 * \brief The trap layer - player traps, runes and door locks
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
#include "effects.h"
#include "init.h"
#include "mon-util.h"
#include "obj-knowledge.h"
#include "player-attack.h"
#include "player-timed.h"
#include "player-util.h"
#include "trap.h"

struct trap_kind *trap_info;

/**
 * Find a trap kind based on its short description
 */
struct trap_kind *lookup_trap(const char *desc)
{
	int i;
	struct trap_kind *closest = NULL;

	/* Look for it */
	for (i = 1; i < z_info->trap_max; i++) {
		struct trap_kind *kind = &trap_info[i];
		if (!kind->name)
			continue;

		/* Test for equality */
		if (streq(desc, kind->desc))
			return kind;

		/* Test for close matches */
		if (!closest && my_stristr(kind->desc, desc))
			closest = kind;
	}

	/* Return our best match */
	return closest;
}

/**
 * Is there a specific kind of trap in this square?
 */
bool square_trap_specific(struct chunk *c, int y, int x, int t_idx)
{
    struct trap *trap = square_trap(c, y, x);
	
    /* First, check the trap marker */
    if (!square_istrap(c, y, x))
		return false;
	
    /* Scan the square trap list */
    while (trap) {
		/* We found a trap of the right kind */
		if (trap->t_idx == t_idx)
			return true;
		trap = trap->next;
	}

    /* Report failure */
    return false;
}

/**
 * Is there a trap with a given flag in this square?
 */
bool square_trap_flag(struct chunk *c, int y, int x, int flag)
{
    struct trap *trap = square_trap(c, y, x);

    /* First, check the trap marker */
    if (!square_istrap(c, y, x))
		return false;
	
    /* Scan the square trap list */
    while (trap) {
		/* We found a trap with the right flag */
		if (trf_has(trap->flags, flag))
			return true;
		trap = trap->next;
    }

    /* Report failure */
    return false;
}

/**
 * Determine if a trap actually exists in this square.
 *
 * Called with vis = 0 to accept any trap, = 1 to accept only visible
 * traps, and = -1 to accept only invisible traps.
 *
 * Clear the SQUARE_TRAP flag if none exist.
 */
static bool square_verify_trap(struct chunk *c, int y, int x, int vis)
{
    struct trap *trap = square_trap(c, y, x);
    bool trap_exists = false;

    /* Scan the square trap list */
    while (trap) {
		/* Accept any trap */
		if (!vis)
			return true;

		/* Accept traps that match visibility requirements */
		if ((vis == 1) && trf_has(trap->flags, TRF_VISIBLE)) 
			return true;

		if ((vis == -1)  && !trf_has(trap->flags, TRF_VISIBLE)) 
			return true;

		/* Note that a trap does exist */
		trap_exists = true;
    }

    /* No traps in this location. */
    if (!trap_exists) {
		/* No traps */
		sqinfo_off(c->squares[y][x].info, SQUARE_TRAP);

		/* Take note */
		square_note_spot(c, y, x);
    }

    /* Report failure */
    return false;
}

/**
 * Determine if a cave grid is allowed to have player traps in it.
 */
bool square_player_trap_allowed(struct chunk *c, int y, int x)
{
    /*
     * We currently forbid multiple traps in a grid under normal conditions.
     * If this changes, various bits of code elsewhere will have to change too.
     */
    if (square_istrap(c, y, x))
		return false;

    /* We currently forbid traps in a grid with objects. */
    if (square_object(c, y, x))
		return false;

    /* Check the feature trap flag */
    return (tf_has(square_feat(c, y, x)->flags, TF_TRAP));
}

/**
 * Instantiate a player trap
 */
static int pick_trap(int feat, int trap_level)
{
    int trap_index = 0;
    struct feature *f = &f_info[feat];
	
    struct trap_kind *kind;
    bool trap_is_okay = false;
	
    /* Paranoia */
    if (!tf_has(f->flags, TF_TRAP))
		return -1;
	
    /* Try to create a trap appropriate to the level.  Make certain that at
     * least one trap type can be made on any possible level. -LM- */
    while (!trap_is_okay) {
		/* Pick at random. */
		trap_index = randint0(z_info->trap_max);

		/* Get this trap */
		kind = &trap_info[trap_index];

		/* Ensure that this is a player trap */
		if (!kind->name) continue;
		if (!trf_has(kind->flags, TRF_TRAP)) continue;
	
		/* Require that trap_level not be too low */
		if (kind->min_depth > trap_level) continue;

		/* Assume legal until proven otherwise. */
		trap_is_okay = true;

		/* Floor? */
		if (tf_has(f->flags, TF_FLOOR) && !trf_has(kind->flags, TRF_FLOOR))
			trap_is_okay = false;

		/* Check legality of trapdoors. */
		if (trf_has(kind->flags, TRF_DOWN)) {
			/* No trap doors on quest levels */
			if (is_quest(player->depth)) trap_is_okay = false;

			/* No trap doors on the deepest level */
			if (player->depth >= z_info->max_depth - 1)
				trap_is_okay = false;
	    }

    }

    /* Return our chosen trap */
    return (trap_index);
}

/**
 * Make a new trap of the given type.  Return true if successful.
 *
 * We choose a player trap at random if the index is not legal. This means that
 * things which are not player traps must be picked by passing a valid index.
 *
 * This should be the only function that places traps in the dungeon
 * except the savefile loading code.
 */
void place_trap(struct chunk *c, int y, int x, int t_idx, int trap_level)
{
	struct trap *new_trap;

    /* We've been called with an illegal index; choose a random trap */
    if ((t_idx <= 0) || (t_idx >= z_info->trap_max)) {
		/* Require the correct terrain */
		if (!square_player_trap_allowed(c, y, x)) return;

		t_idx = pick_trap(c->squares[y][x].feat, trap_level);
    }

    /* Failure */
    if (t_idx < 0) return;

	/* Allocate a new trap for this grid (at the front of the list) */
	new_trap = mem_zalloc(sizeof(*new_trap));
	new_trap->next = square_trap(c, y, x);
	c->squares[y][x].trap = new_trap;

	/* Set the details */
	new_trap->t_idx = t_idx;
	new_trap->kind = &trap_info[t_idx];
	new_trap->fy = y;
	new_trap->fx = x;
	trf_copy(new_trap->flags, trap_info[t_idx].flags);

	/* Toggle on the trap marker */
	sqinfo_on(c->squares[y][x].info, SQUARE_TRAP);

	/* Redraw the grid */
	square_light_spot(c, y, x);
}

/**
 * Free memory for all traps on a grid
 */
void square_free_trap(struct chunk *c, int y, int x)
{
	struct trap *next, *trap = square_trap(c, y, x);

	while (trap) {
		next = trap->next;
		mem_free(trap);
		trap = next;
	}
}

/**
 * Reveal some of the player traps in a square
 */
bool square_reveal_trap(struct chunk *c, int y, int x, bool domsg)
{
    int found_trap = 0;
	struct trap *trap = square_trap(c, y, x);
    
    /* Check there is a player trap */
    if (!square_isplayertrap(c, y, x))
		return false;

	/* Scan the grid */
	while (trap) {
		/* Skip non-player traps */
		if (!trf_has(trap->flags, TRF_TRAP)) {
			trap = trap->next;
			continue;
		}
		
		/* Trap is invisible */
		if (!trf_has(trap->flags, TRF_VISIBLE)) {
			/* See the trap */
			trf_on(trap->flags, TRF_VISIBLE);
			square_memorize(c, y, x);

			/* We found a trap */
			found_trap++;
		}
		trap = trap->next;
	}

    /* We found at least one trap */
    if (found_trap) {
		/* We want to talk about it */
		if (domsg) {
			if (found_trap == 1)
				msg("You have found a trap.");
			else
				msg("You have found %d traps.", found_trap);
		}

		/* Memorize */
		square_memorize(c, y, x);

		/* Redraw */
		square_light_spot(c, y, x);
    }

    /* Return true if we found any traps */
    return (found_trap != 0);
}

/**
 * Count the number of player traps in this location.
 *
 * Called with vis = 0 to accept any trap, = 1 to accept only visible
 * traps, and = -1 to accept only invisible traps.
 */
int num_traps(struct chunk *c, int y, int x, int vis)
{
    int num = 0;
	struct trap *trap;

	/* Look at the traps in this grid */
	for (trap = square_trap(c, y, x); trap; trap = trap->next) {
		/* Require that trap be capable of affecting the character */
		if (!trf_has(trap->kind->flags, TRF_TRAP)) continue;
	    
		/* Require correct visibility */
		if (vis >= 1) {
			if (trf_has(trap->flags, TRF_VISIBLE)) num++;
		} else if (vis <= -1) {
			if (!trf_has(trap->flags, TRF_VISIBLE)) num++;
		} else {
			num++;
		}
	}

    /* Return the number of traps */
    return (num);
}

/**
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
bool trap_check_hit(int power)
{
	return test_hit(power, player->state.ac + player->state.to_a, true);
}


/**
 * Hit a trap. 
 */
extern void hit_trap(int y, int x)
{
	bool ident;
	struct trap *trap;
	struct effect *effect;

    /* Count the hidden traps here */
    int num = num_traps(cave, y, x, -1);

	/* The player is safe from all traps */
	if (player->timed[TMD_TRAPSAFE]) return;

    /* Oops.  We've walked right into trouble. */
    if      (num == 1) msg("You stumble upon a trap!");
    else if (num >  1) msg("You stumble upon some traps!");


	/* Look at the traps in this grid */
	for (trap = square_trap(cave, y, x); trap; trap = trap->next) {
		int flag;
		bool saved = false;

		/* Require that trap be capable of affecting the character */
		if (!trf_has(trap->kind->flags, TRF_TRAP)) continue;
		if (trap->timeout) continue;

		/* Set this to be the current trap */
		cave->trap_current = trap;
	    
		/* Disturb the player */
		disturb(player, 0);

		/* Give a message */
		if (trap->kind->msg)
			msg(trap->kind->msg);

		/* Test for save due to flag */
		for (flag = of_next(trap->kind->save_flags, FLAG_START);
			 flag != FLAG_END;
			 flag = of_next(trap->kind->save_flags, flag + 1))
			if (player_of_has(player, flag)) {
				saved = true;
				equip_learn_flag(player, flag);
			}

		/* Test for save due to armor */
		if (trf_has(trap->kind->flags, TRF_SAVE_ARMOR) && !trap_check_hit(125))
			saved = true;

		/* Test for save due to saving throw */
		if (trf_has(trap->kind->flags, TRF_SAVE_THROW) &&
			(randint0(100) < player->state.skills[SKILL_SAVE]))
			saved = true;

		/* Save, or fire off the trap */
		if (saved) {
			if (trap->kind->msg_good)
				msg(trap->kind->msg_good);
		} else {
			if (trap->kind->msg_bad)
				msg(trap->kind->msg_bad);
			effect = trap->kind->effect;
			effect_do(effect, NULL, &ident, false, 0, 0, 0);

			/* Do any extra effects */
			if (trap->kind->effect_xtra && one_in_(2)) {
				if (trap->kind->msg_xtra)
					msg(trap->kind->msg_xtra);
				effect = trap->kind->effect_xtra;
				effect_do(effect, NULL, &ident, false, 0, 0, 0);
			}
		}

		/* Some traps drop you a dungeon level */
		if (trf_has(trap->kind->flags, TRF_DOWN))
			dungeon_change_level(dungeon_get_next_level(player->depth, 1));

		/* Some traps drop you onto them */
		if (trf_has(trap->kind->flags, TRF_PIT))
			monster_swap(player->py, player->px, trap->fy, trap->fx);

		/* Some traps disappear after activating */
		if (trf_has(trap->kind->flags, TRF_ONETIME)) {
			square_destroy_trap(cave, y, x);
			square_forget(cave, y, x);
		}

		/* Trap may have gone */
		if (!square_trap(cave, y, x)) break;

		/* Trap becomes visible (always XXX) */
		trf_on(trap->flags, TRF_VISIBLE);
		square_memorize(cave, y, x);
	}

	/* Clear the current trap */
	cave->trap_current = NULL;

    /* Verify traps (remove marker if appropriate) */
    (void)square_verify_trap(cave, y, x, 0);
}

/**
 * Remove a trap
 */
static void remove_trap_aux(struct chunk *c, struct trap *trap, int y, int x,
							bool domsg)
{
	/* Message if needed */
	if (domsg) {
		/* We are deleting a rune */
		if (trf_has(trap->flags, TRF_RUNE)) {
			if (c->mon_current < 0) {
				/* Removed by player */
				msg("You have removed the %s.", trap->kind->name);
			}
		} else {
			/* We are disarming a trap */
			msgt(MSG_DISARM, "You have disarmed the %s.", trap->kind->name);
		}
	}

    /* Wipe the trap */
	mem_free(trap);
}

/**
 * Remove traps.
 *
 * If called with t_idx < 0, will remove all traps in the location given.
 * Otherwise, will remove all traps with the given kind.
 *
 * Return true if no traps now exist in this grid.
 */
bool square_remove_trap(struct chunk *c, int y, int x, bool domsg, int t_idx)
{
    bool trap_exists;
	struct trap **trap_slot = NULL;
	struct trap *next_trap;

	/* Bounds check */
	assert(square_in_bounds(c, y, x));

	/* Look at the traps in this grid */
	trap_slot = &c->squares[y][x].trap;
	while (*trap_slot) {
		/* Get the next trap (may be NULL) */
		next_trap = (*trap_slot)->next;

		/* If called with a specific index, skip others */
		if ((t_idx >= 0) && (t_idx != (*trap_slot)->t_idx)) {
			if (!next_trap) break;
			trap_slot = &next_trap;
			continue;
		}

		/* Remove it */
		remove_trap_aux(c, *trap_slot, y, x, domsg);

		/* Replace with the next trap */
		*trap_slot = next_trap;
    }

    /* Refresh grids that the character can see */
    if (square_isseen(c, y, x))
		square_light_spot(c, y, x);
    
    /* Verify traps (remove marker if appropriate) */
    trap_exists = square_verify_trap(c, y, x, 0);

    /* Report whether any traps exist in this grid */
    return (!trap_exists);
}

/**
 * Remove traps.
 *
 * If called with t_idx < 0, will remove all traps in the location given.
 * Otherwise, will remove all traps with the given kind.
 *
 * Return true if no traps now exist in this grid.
 */
bool square_set_trap_timeout(struct chunk *c, int y, int x, bool domsg,
							 int t_idx, int time)
{
    bool trap_exists;
	struct trap *current_trap = NULL;

	/* Bounds check */
	assert(square_in_bounds(c, y, x));

	/* Look at the traps in this grid */
	current_trap = c->squares[y][x].trap;
	while (current_trap) {
		/* Get the next trap (may be NULL) */
		struct trap *next_trap = current_trap->next;

		/* If called with a specific index, skip others */
		if ((t_idx >= 0) && (t_idx != current_trap->t_idx)) {
			if (!next_trap) break;
			current_trap = next_trap;
			continue;
		}

		/* Set the timer */
		current_trap->timeout = time;

		/* Message if requested */
		msg("You have disabled the %s.", current_trap->kind->name);

		/* Replace with the next trap */
		current_trap = next_trap;
    }

    /* Refresh grids that the character can see */
    if (square_isseen(c, y, x))
		square_light_spot(c, y, x);

    /* Verify traps (remove marker if appropriate) */
    trap_exists = square_verify_trap(c, y, x, 0);

    /* Report whether any traps exist in this grid */
    return (!trap_exists);
}

/**
 * Give the remaining time for a trap to be disabled; note it chooses the first
 * appropriate trap on the grid
 */
int square_trap_timeout(struct chunk *c, int y, int x, int t_idx)
{
	struct trap *current_trap = c->squares[y][x].trap;
	while (current_trap) {
		/* Get the next trap (may be NULL) */
		struct trap *next_trap = current_trap->next;

		/* If called with a specific index, skip others */
		if ((t_idx >= 0) && (t_idx != current_trap->t_idx)) {
			if (!next_trap) break;
			current_trap = next_trap;
			continue;
		}

		/* If the timer is set, return the value */
		if (current_trap->timeout)
			return current_trap->timeout;

		/* Replace with the next trap */
		current_trap = next_trap;
    }

	return 0;
}

/**
 * Lock a closed door to a given power
 */
void square_set_door_lock(struct chunk *c, int y, int x, int power)
{
	struct trap_kind *lock = lookup_trap("door lock");
	struct trap *trap;

	/* Verify it's a closed door */
	if (!square_iscloseddoor(c, y, x))
		return;

	/* If there's no lock there, add one */
	if (!square_trap_specific(c, y, x, lock->tidx))
		place_trap(c, y, x, lock->tidx, 0);

	/* Set the power (of all locks - there should be only one) */
	trap = square_trap(c, y, x);
	while (trap) {
		if (trap->kind == lock)
			trap->power = power;
		trap = trap->next;
	}
}

/**
 * Return the power of the lock on a door
 */
int square_door_power(struct chunk *c, int y, int x)
{
	struct trap_kind *lock = lookup_trap("door lock");
	struct trap *trap;

	/* Verify it's a closed door */
	if (!square_iscloseddoor(c, y, x))
		return 0;

	/* Is there a lock there? */
	if (!square_trap_specific(c, y, x, lock->tidx))
		return 0;

	/* Get the power and return it */
	trap = square_trap(c, y, x);
	while (trap) {
		if (trap->kind == lock)
			return trap->power;
		trap = trap->next;
	}

	return 0;
}

