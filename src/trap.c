/*
 * File: trap.c
 * Purpose: Trap triggering, selection, and placement
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
#include "dungeon.h"
#include "effects.h"
#include "init.h"
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
	for (i = 1; i < z_info->trap_max; i++)
	{
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
    int i;
	
    /* First, check the trap marker */
    if (!square_istrap(c, y, x)) return (FALSE);
	
    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);
		
		/* Find a trap in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* We found a trap of the right kind */
			if (trap->t_idx == t_idx) return (TRUE);
		}
    }

    /* Report failure */
    return (FALSE);
}

/**
 * Is there a trap with a given flag in this square?
 */
bool square_trap_flag(struct chunk *c, int y, int x, int flag)
{
    int i;

    /* First, check the trap marker */
    if (!square_istrap(c, y, x)) return (FALSE);
	
    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);
		
		/* Find a trap in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* We found a trap with the right flag */
			if (trf_has(trap->flags, flag)) return (TRUE);
		}
    }

    /* Report failure */
    return (FALSE);
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
    int i;
    bool trap_exists = FALSE;
    
    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);
	
		/* Find a trap in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* Accept any trap */
			if (!vis) return (TRUE);

			/* Accept traps that match visibility requirements */
			if (vis == 1)
			{
				if (trf_has(trap->flags, TRF_VISIBLE)) 
					return (TRUE);
			}
	    
			if (vis == -1)
			{
				if (!trf_has(trap->flags, TRF_VISIBLE)) 
					return (TRUE);
			}
	    
			/* Note that a trap does exist */
			trap_exists = TRUE;
		}
    }
    
    /* No traps in this location. */
    if (!trap_exists)
    {
		/* No traps */
		sqinfo_off(c->info[y][x], SQUARE_TRAP);
	
		/* No reason to mark this square, ... */
		sqinfo_off(c->info[y][x], SQUARE_MARK);

		/* ... unless certain conditions apply */
		square_note_spot(c, y, x);
    }
    
    /* Report failure */
    return (FALSE);
}

/**
 * Is there a visible trap in this square?
 */
bool square_visible_trap(struct chunk *c, int y, int x)
{
    /* Look for a visible trap */
    return (square_trap_flag(c, y, x, TRF_VISIBLE));
}

/**
 * Is there an invisible trap in this square?
 */
bool square_invisible_trap(struct chunk *c, int y, int x)
{
    /* First, check the trap marker */
    if (!square_istrap(c, y, x)) return (FALSE);

    /* Verify trap, require that it be invisible */
    return (square_verify_trap(c, y, x, -1));
}

/**
 * Is there a player trap in this square?
 */
bool square_player_trap(struct chunk *c, int y, int x)
{
    /* Look for a player trap */
    return (square_trap_flag(c, y, x, TRF_TRAP));
}

/**
 * Return the index of any visible trap 
 */
int square_visible_trap_idx(struct chunk *c, int y, int x)
{
    int i;

    if (!square_visible_trap(c, y, x)) 
		return -1;
    
    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);
		
		/* Find a visible trap in this position */
		if ((trap->fy == y) && (trap->fx == x) && 
			trf_has(trap->flags, TRF_VISIBLE))
			return (i);
    }
    
    /* Paranoia */
    return -1;
}

/**
 * Get the graphics of a listed trap.
 *
 * We should probably have better handling of stacked traps, but that can
 * wait until we do, in fact, have stacked traps under normal conditions.
 *
 */
bool get_trap_graphics(struct chunk *c, int t_idx, int *a, wchar_t *ch, bool require_visible)
{
    struct trap *trap = cave_trap(c, t_idx);
    
    /* Trap is visible, or we don't care */
    if (!require_visible || trf_has(trap->flags, TRF_VISIBLE))
    {
		/* Get the graphics */
		*a = trap->kind->x_attr;
		*ch = trap->kind->x_char;
	
		/* We found a trap */
		return (TRUE);
    }
    
    /* No traps found with the requirement */
    return (FALSE);
}

/**
 * Reveal some of the traps in a square
 */
bool square_reveal_trap(struct chunk *c, int y, int x, int chance, bool domsg)
{
    int i;
    int found_trap = 0;
    
    /* Check the trap marker */
    if (!square_istrap(c, y, x)) return (FALSE);

    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);

		/* Find a trap in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* Trap is invisible */
			if (!trf_has(trap->flags, TRF_VISIBLE))
			{
				/* See the trap */
				trf_on(trap->flags, TRF_VISIBLE);
				sqinfo_on(c->info[y][x], SQUARE_MARK);

				/* We found a trap */
				found_trap++;

				/* If chance is < 100, sometimes stop */
				if ((chance < 100) && (randint1(100) > chance)) break;
			}
		}
    }

    /* We found at least one trap */
    if (found_trap)
    {
		/* We want to talk about it */
		if (domsg)
		{
			if (found_trap == 1) msg("You have found a trap.");
			else msg("You have found %d traps.", found_trap);
		}

		/* Memorize */
		sqinfo_on(c->info[y][x], SQUARE_MARK);

		/* Redraw */
		square_light_spot(c, y, x);
    }
    
    /* Return TRUE if we found any traps */
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
    int i, num;
    
    /* Scan the current trap list */
    for (num = 0, i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);
	
		/* Find all traps in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* Require that trap be capable of affecting the character */
			if (!trf_has(trap->kind->flags, TRF_TRAP)) continue;
	    
			/* Require correct visibility */
			if (vis >= 1)
			{
				if (trf_has(trap->flags, TRF_VISIBLE)) num++;
			}
			else if (vis <= -1)
			{
				if (!trf_has(trap->flags, TRF_VISIBLE)) num++;
			}
			else
			{
				num++;
			}
		}
    }
	
    /* Return the number of traps */
    return (num);
}

/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
bool trap_check_hit(int power)
{
	return test_hit(power, player->state.ac + player->state.to_a, TRUE);
}


/**
 * Determine if a cave grid is allowed to have traps in it.
 */
bool square_trap_allowed(struct chunk *c, int y, int x)
{
    /*
     * We currently forbid multiple traps in a grid under normal conditions.
     * If this changes, various bits of code elsewhere will have to change too.
     */
    if (square_istrap(c, y, x)) return (FALSE);

    /* We currently forbid traps in a grid with objects. */
    if (c->o_idx[y][x]) return (FALSE);

    /* Check the feature trap flag */
    return (tf_has(f_info[c->feat[y][x]].flags, TF_TRAP));
}

/**
 * Instantiate a trap
 */
static int pick_trap(int feat, int trap_level)
{
    int trap_index = 0;
    feature_type *f_ptr = &f_info[feat];
	
    struct trap_kind *kind;
    bool trap_is_okay = FALSE;
	
    /* Paranoia */
    if (!tf_has(f_ptr->flags, TF_TRAP))
		return -1;
	
    /* Try to create a trap appropriate to the level.  Make certain that at
     * least one trap type can be made on any possible level. -LM- */
    while (!trap_is_okay) 
    {
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
		trap_is_okay = TRUE;

		/* Floor? */
		if (tf_has(f_ptr->flags, TF_FLOOR) && !trf_has(kind->flags, TRF_FLOOR))
			trap_is_okay = FALSE;

		/* Check legality of trapdoors. */
		if (trf_has(kind->flags, TRF_DOWN))
	    {
			/* No trap doors on quest levels */
			if (is_quest(player->depth)) trap_is_okay = FALSE;

			/* No trap doors on the deepest level */
			if (player->depth >= MAX_DEPTH - 1) trap_is_okay = FALSE;
	    }

    }

    /* Return our chosen trap */
    return (trap_index);
}

/**
 * Make a new trap of the given type.  Return TRUE if successful.
 *
 * We choose a trap at random if the index is not legal.
 *
 * This should be the only function that places traps in the dungeon
 * except the savefile loading code.
 */
void place_trap(struct chunk *c, int y, int x, int t_idx, int trap_level)
{
    int i;

    /* Require the correct terrain */
    if (!square_trap_allowed(c, y, x)) return;

    /* Hack -- don't use up all the trap slots during dungeon generation */
    if (!character_dungeon)
    {
		if (cave_trap_max(c) > z_info->l_max - 50) return;
    }

    /* We've been called with an illegal index; choose a random trap */
    if ((t_idx <= 0) || (t_idx >= z_info->trap_max))
    {
		t_idx = pick_trap(c->feat[y][x], trap_level);
    }

    /* Failure */
    if (t_idx < 0) return;


    /* Scan the entire trap list */
    for (i = 1; i < z_info->l_max; i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);

		/* This space is available */
		if (!trap->t_idx)
		{
			/* Fill in the trap index */
			trap->t_idx = t_idx;
			trap->kind = &trap_info[t_idx];

			/* Fill in the trap details */
			trap->fy = y;
			trap->fx = x;

			trf_copy(trap->flags, trap_info[trap->t_idx].flags);

			/* Adjust trap count if necessary */
			if (i + 1 > cave_trap_max(c)) c->trap_max = i + 1;

			/* Toggle on the trap marker */
			sqinfo_on(c->info[y][x], SQUARE_TRAP);

			/* Redraw the grid */
			square_light_spot(c, y, x);
	    
	    /* Success */
	    return;
		}
    }
}

/**
 * Hit a trap. 
 */
extern void hit_trap(int y, int x)
{
	bool ident;
    int i;
	struct effect *effect;

    /* Count the hidden traps here */
    int num = num_traps(cave, y, x, -1);

    /* Oops.  We've walked right into trouble. */
    if      (num == 1) msg("You stumble upon a trap!");
    else if (num >  1) msg("You stumble upon some traps!");


    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(cave); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(cave, i);

		/* Find all traps in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* Disturb the player */
			disturb(player, 0);

			/* Fire off the trap */
			effect = trap->kind->effect;
			effect_do(effect, &ident, FALSE, 0, 0, 0);

			/* Trap becomes visible (always XXX) */
			trf_on(trap->flags, TRF_VISIBLE);
			sqinfo_on(cave->info[y][x], SQUARE_MARK);
		}
    }

    /* Verify traps (remove marker if appropriate) */
    (void)square_verify_trap(cave, y, x, 0);
}

/**
 * Delete/Remove all the traps when the player leaves the level
 */
void wipe_trap_list(struct chunk *c)
{
	int i;

	/* Delete all the traps */
	for (i = cave_trap_max(c) - 1; i >= 0; i--)
	{
		struct trap *trap = cave_trap(c, i);

		/* Wipe the trap */
		WIPE(trap, struct trap);
	}

	/* Reset "trap_max" */
	c->trap_max = 0;
}



/**
 * Remove a trap
 */
static void remove_trap_aux(struct chunk *c, struct trap *trap, int y, int x, bool domsg)
{
    /* We are deleting a rune */
    if (trf_has(trap->flags, TRF_RUNE))
    {
		if (domsg)
			msg("You have removed the %s.", trap->kind->name);
    }
    /* We are disarming a trap */
    else if (domsg)
		msgt(MSG_DISARM, "You have disarmed the %s.", trap->kind->name);
    
    /* Wipe the trap */
    sqinfo_off(c->info[y][x], SQUARE_TRAP);
    (void)WIPE(trap, struct trap);
}

/**
 * Remove traps.
 *
 * If called with t_idx < 0, will remove all traps in the location given.
 * Otherwise, will remove the trap with the given index.
 *
 * Return TRUE if no traps now exist in this grid.
 */
bool square_remove_trap(struct chunk *c, int y, int x, bool domsg, int t_idx)
{
    int i;
    bool trap_exists;

    /* Called with a specific index */
    if (t_idx >= 0)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, t_idx);
	
		/* Remove it */
		remove_trap_aux(c, trap, y, x, domsg);
	
		/* Note when trap list actually gets shorter */
		if (t_idx == cave_trap_max(c) - 1) c->trap_max--;
    }

    /* No specific index -- remove all traps here */
    else
    {
		/* Scan the current trap list (backwards) */
		for (i = cave_trap_max(c) - 1; i >= 0; i--)
		{
			/* Point to this trap */
			struct trap *trap = cave_trap(c, i);
	    
			/* Find all traps in this position */
			if ((trap->fy == y) && (trap->fx == x))
			{
				/* Remove it */
				remove_trap_aux(c, trap, y, x, domsg);
		
				/* Note when trap list actually gets shorter */
				if (i == cave_trap_max(c) - 1) c->trap_max--;
			}
		}
    }

    /* Refresh grids that the character can see */
    if (player_can_see_bold(y, x)) square_light_spot(c, y, x);
    
    /* Verify traps (remove marker if appropriate) */
    trap_exists = square_verify_trap(c, y, x, 0);
    
    /* Report whether any traps exist in this grid */
    return (trap_exists);
}

/**
 * Remove all traps of a specific kind from a location.
 */
void square_remove_trap_kind(struct chunk *c, int y, int x, bool domsg, int t_idx)
{
    int i;

    /* Scan the current trap list */
    for (i = 0; i < cave_trap_max(c); i++)
    {
		/* Point to this trap */
		struct trap *trap = cave_trap(c, i);

		/* Find a trap in this position */
		if ((trap->fy == y) && (trap->fx == x))
		{
			/* Require that it be of this type */
			if (trap->t_idx == t_idx) 
				(void)square_remove_trap(c, y, x, domsg, i);
		}
    }
}
