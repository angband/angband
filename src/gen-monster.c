/* 
 *  File gen-monster.c 
 *  Purpose: Dungeon monster generation
 *  Code for selecting appropriate monsters for levels when generated.  
 *
 * Copyright (c) 2013
 * Nick McConnell, Leon Marrick, Ben Harrison, James E. Wilson, 
 * Robert A. Koeneke
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
#include "math.h"
#include "files.h"
#include "game-event.h"
#include "generate.h"
#include "init.h"
#include "monster.h"
#include "mon-make.h"
#include "mon-spell.h"

/**
 * Restrictions on monsters, used in pits, vaults, and chambers.
 */
static bool allow_unique;
static char base_d_char[15];


/**************************************************************/
/*                                                            */
/*                 The monster-selection code                 */
/*                                                            */
/**************************************************************/


/**
 * Return the pit profile matching the given name.
 */
pit_profile *lookup_pit_profile(const char *name)
{
	pit_profile *profile;

	/* Look for it */
	for (profile = pit_info; profile; profile = profile->next) {
		if (streq(name, profile->name))
			return profile;
	}

	return NULL;
}

/**
 * This function selects monsters by monster base symbol 
 * (may be any of the characters allowed)
 *
 * Uniques may be forbidden, or allowed on rare occasions.
 */
static bool mon_select(monster_race *r_ptr)
{
    /* Require that the monster symbol be correct. */
    if (base_d_char[0] != '\0') {
		if (strchr(base_d_char, r_ptr->base->d_char) == 0)
			return (FALSE);
    }

	/* No invisible undead until deep. */
	if ((player->depth < 40) && (rf_has(r_ptr->flags, RF_UNDEAD))
		&& (rf_has(r_ptr->flags, RF_INVISIBLE)))
		return (FALSE);

    /* Usually decline unique monsters. */
    if (rf_has(r_ptr->flags, RF_UNIQUE)) {
		if (!allow_unique)
			return (FALSE);
		else if (randint0(5) != 0)
			return (FALSE);
    }

    /* Okay */
    return (TRUE);
}

/**
 * Accept characters representing a race or group of monsters and 
 * an (adjusted) depth, and use these to set values for required
 * monster base symbol.
 *
 * This code has been adapted from Oangband code to make use of monster bases.
 *
 * This function is called to set restrictions, point the monster 
 * allocation function to mon_select() or mon_pit_hook(), and remake monster 
 * allocation.  
 * It undoes all of this things when called with monster_type NULL.
 * If called with a pit profile name, it will get monsters from that profile.
 * If called with monster_type "random", it will get a random monster base and 
 * describe the monsters by its name (for use by cheat_room).
 */
bool mon_restrict(char *monster_type, byte depth, bool unique_ok)
{
    int i, j;

    /* Clear global monster restriction variables. */
    allow_unique = unique_ok;
    for (i = 0; i < 10; i++)
		base_d_char[i] = '\0';

    /* No symbol, no restrictions. */
    if (monster_type == NULL) {
		get_mon_num_prep(NULL);
		return TRUE;
	} else if (streq(monster_type, "random")) {
		/* Handle random */
		for (i = 0; i < 2500; i++) {
			/* Get a random monster. */
			j = randint1(z_info->r_max - 1);

			/* Must be a real monster */
			if (!r_info[j].rarity)
				continue;

			/* Try for close to depth, accept in-depth if necessary */
			if (i < 200) {
				if ((!rf_has(r_info[j].flags, RF_UNIQUE))
					&& (r_info[j].level != 0) && (r_info[j].level <= depth)
					&& (ABS(r_info[j].level - player->depth) <
						1 + (player->depth / 4)))
					break;
			} else {
				if ((!rf_has(r_info[j].flags, RF_UNIQUE))
					&& (r_info[j].level != 0) && (r_info[j].level <= depth))
					break;
			}
		}

		/* We've found a monster. */
		if (i < 2499) {
			/* Use that monster's base type for all monsters. */
			strcpy(monster_type, r_info[j].base->name);
			sprintf(base_d_char, "%c", r_info[j].base->d_char);

			/* Prepare allocation table */
			get_mon_num_prep(mon_select);
			return TRUE;
		} else
			/* Paranoia - area stays empty if no monster is found */
			return FALSE;
    } else {
		/* Use a pit profile */
		pit_profile *profile = lookup_pit_profile(monster_type);

		/* Accept the profile or leave area empty if none found */
		if (profile)
			dun->pit_type = profile;
		else
			return FALSE;

		/* Prepare allocation table */
		get_mon_num_prep(mon_pit_hook);
		return TRUE;
	}
}


/**
 * Place monsters, up to the number asked for, in a rectangle centered on 
 * y0, x0.  Accept values for monster depth, symbol, and maximum vertical 
 * and horizontal displacement.  Call monster restriction functions if 
 * needed.
 *
 * Return prematurely if the code starts looping too much (this may happen 
 * if y0 or x0 are out of bounds, or the area is already occupied).
 */
void spread_monsters(struct cave *c, char *type, int depth, int num, int y0, 
					 int x0, int dy, int dx, byte origin)
{
    int i, j;			/* Limits on loops */
    int count;
    int y = y0, x = x0;
    int start_mon_num = c->mon_max;

    /* Restrict monsters.  Allow uniques. Leave area empty if none found. */
    if (!mon_restrict(type, (byte) depth, TRUE))
		return;

    /* Build the monster probability table. */
    if (!get_mon_num(depth))
		return;


    /* Try to summon monsters within our rectangle of effect. */
    for (count = 0, i = 0; ((count < num) && (i < 50)); i++) {
		/* Get a location */
		if ((dy == 0) && (dx == 0)) {
			y = y0;
			x = x0;
			if (!square_in_bounds(c, y, x))
				return;
		} else {
			for (j = 0; j < 10; j++) {
				y = rand_spread(y0, dy);
				x = rand_spread(x0, dx);
				if (!square_in_bounds(c, y, x)) {
					if (j < 9)
						continue;
					else
						return;
				}
				break;
			}
		}

		/* Require "empty" floor grids */
		if (!square_isempty(c, y, x)) continue;

		/* Place the monster (sleeping, allow groups) */
		pick_and_place_monster(c, y, x, depth, TRUE, TRUE, origin);

		/* Rein in monster groups and escorts a little. */
		if (c->mon_max - start_mon_num > num * 2)
			break;

		/* Count the monster(s), reset the loop count */
		count++;
		i = 0;
    }

    /* Remove monster restrictions. */
    (void) mon_restrict(NULL, (byte) depth, TRUE);
}


/**
 * To avoid rebuilding the monster list too often (which can quickly 
 * get expensive), we handle monsters of a specified race separately.
 */
void get_vault_monsters(struct cave *c, char racial_symbol[], byte vault_type, const char *data, int y1, int y2, int x1, int x2)
{
    int i, y, x, depth;
    const char *t;

    for (i = 0; racial_symbol[i] != '\0'; i++) {
		/* Require correct race, allow uniques. */
		allow_unique = TRUE;
		sprintf(base_d_char, "%c", racial_symbol[i]);

		/* Determine level of monster */
		if (vault_type == 6)
			depth = player->depth + 2;
		else if (vault_type == 7)
			depth = player->depth + 4;
		else if (vault_type == 8)
			depth = player->depth + 6;
		else
			depth = player->depth;

		/* Prepare allocation table */
		get_mon_num_prep(mon_select);

		/* Build the monster probability table. */
		if (!get_mon_num(depth))
			continue;


		/* Place the monsters */
		for (t = data, y = y1; y <= y2; y++) {
			for (x = x1; x <= x2; x++, t++) {
				if (*t == racial_symbol[i]) {
					/* Place a monster */
					pick_and_place_monster(c, y, x, depth, FALSE,
										   FALSE, ORIGIN_DROP_SPECIAL);
				}
			}
		}
    }

    /* Clear any current monster restrictions. */
	get_mon_num_prep(NULL);
}
