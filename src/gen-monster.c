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
#include "monster/monster.h"
#include "monster/mon-make.h"
#include "monster/mon-spell.h"
#include "object/tvalsval.h"

/**
 * Restrictions on monsters, used in pits, vaults, and chambers.
 */
static bool allow_unique;
static char d_char_req[15];
static byte d_attr_req[8];
static u32b racial_flag_mask;
static bitflag breath_flag_mask[RSF_SIZE];


/**************************************************************/
/*                                                            */
/*                 The monster-selection code                 */
/*                                                            */
/**************************************************************/


/**
 * Use various selection criteria (set elsewhere) to restrict monster 
 * generation.
 *
 * This function is capable of selecting monsters by:
 *   - racial symbol (may be any of the characters allowed)
 *   - symbol color (may be any of up to four colors).
 *   - racial flag(s) (monster may have any allowed flag)
 *   - breath flag(s) (monster must have exactly the flags specified)
 *
 * Uniques may be forbidden, or allowed on rare occasions.
 *
 */
static bool mon_select(monster_race *r_ptr)
{
    bool ok = FALSE;
    bitflag mon_breath[RSF_SIZE];


    /* Get breath attack */
    rsf_copy(mon_breath, r_ptr->spell_flags);
    flags_mask(mon_breath, RSF_SIZE, RSF_BREATH_MASK, FLAG_END);

    /* Require that the monster symbol be correct. */
    if (d_char_req[0] != '\0') {
		if (strchr(d_char_req, r_ptr->base->d_char) == 0)
			return (FALSE);
    }

    /* Require correct racial type. */
    if (racial_flag_mask) {
		if (!(rf_has(r_ptr->flags, racial_flag_mask)))
			return (FALSE);

		/* Hack -- no invisible undead until deep. */
		if ((p_ptr->depth < 40) && (rf_has(r_ptr->flags, RF_UNDEAD))
			&& (rf_has(r_ptr->flags, RF_INVISIBLE)))
			return (FALSE);
    }

    /* Require that monster breaths be exactly those specified. */
    if (!rsf_is_empty(breath_flag_mask)) {
		if (!rsf_is_equal(mon_breath, breath_flag_mask))
			return (FALSE);
    }

    /* Require that the monster color be correct. */
    if (d_attr_req[0]) {
		/* Check all allowed colors, if given. */
		if ((d_attr_req[0]) && (r_ptr->d_attr == d_attr_req[0]))
			ok = TRUE;
		if ((d_attr_req[1]) && (r_ptr->d_attr == d_attr_req[1]))
			ok = TRUE;
		if ((d_attr_req[2]) && (r_ptr->d_attr == d_attr_req[2]))
			ok = TRUE;
		if ((d_attr_req[3]) && (r_ptr->d_attr == d_attr_req[3]))
			ok = TRUE;

		/* Doesn't match any of the given colors? Not good. */
		if (!ok)
			return (FALSE);
    }

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
 * an (adjusted) depth, and use these to set values for required racial 
 * type, monster base symbol, monster symbol color, and breath type.
 *
 * This code was originally from Oangband, and will need tweaking before
 * actual use - in particular better use could be made of monster bases,
 * and the symbol colors may not be relevant.
 *
 * This function is called to set restrictions, point the monster 
 * allocation function to mon_select(), and remake monster allocation.  
 * It undoes all of this things when called with the symbol '\0'.
 * 
 * Describe the monsters (for use by cheat_room) and determine if they 
 * should be neatly ordered or randomly placed (used in monster pits).
 */
char *mon_restrict(char symbol, byte depth, bool * ordered,
						  bool unique_ok)
{
    int i, j;

    /* Assume no definite name */
    char name[80] = "misc";

    /* Clear global monster restriction variables. */
    allow_unique = unique_ok;
    for (i = 0; i < 10; i++)
		d_char_req[i] = '\0';
    for (i = 0; i < 4; i++)
		d_attr_req[i] = 0;
    racial_flag_mask = 0;
    rsf_wipe(breath_flag_mask);


    /* No symbol, no restrictions. */
    if (symbol == '\0') {
		get_mon_num_prep(NULL);
		return ("misc");
    }

    /* Handle the "wild card" symbol '*' */
    if (symbol == '*') {
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
					&& (ABS(r_info[j].level - p_ptr->depth) <
						1 + (p_ptr->depth / 4)))
					break;
			} else {
				if ((!rf_has(r_info[j].flags, RF_UNIQUE))
					&& (r_info[j].level != 0) && (r_info[j].level <= depth))
					break;
			}
		}

		/* We've found a monster. */
		if (i < 2499) {
			/* ...use that monster's symbol for all monsters. */
			symbol = r_info[j].d_char;
			strcpy(name, r_info[j].base->name);
		} else {
			/* Paranoia - pit stays empty if no monster is found */
			return (NULL);
		}
    }

    /* Apply monster restrictions according to symbol. */
    switch (symbol) {
		/* All animals */
	/* Insects */
    case '1':
	{
	    strcpy(name, "insect");
	    strcpy(d_char_req, "aclFIK");
	    *ordered = FALSE;
	    break;
	}

	/* Reptiles */
    case '2':
	{
	    strcpy(name, "reptile");
	    strcpy(d_char_req, "nJR");
	    *ordered = FALSE;
	    break;
	}

	/* Jellies, etc. */
    case '3':
	{
	    strcpy(name, "jelly");
	    strcpy(d_char_req, "ijm,");
	    *ordered = FALSE;
	    break;
	}

    case '4':
	{
	    strcpy(name, "animal");
	    racial_flag_mask = RF_ANIMAL;
	    *ordered = FALSE;
	    break;
	}

	/* Humans and humaniods */
    case 'p':
    case 'h':
	{
	    /* 'p's and 'h's can coexist. */
	    if (randint0(3) == 0) {
			strcpy(d_char_req, "ph");

			/* If so, they will usually all be of similar classes. */
			if (randint0(4) != 0) {
				/* Randomizer. */
				i = randint0(5);

				/* Magicians and necromancers */
				if (i == 0) {
					d_attr_req[0] = TERM_RED;
					d_attr_req[1] = TERM_L_PURPLE;
					d_attr_req[2] = TERM_L_RED;
					d_attr_req[3] = TERM_PURPLE;
					d_attr_req[4] = TERM_VIOLET;
					d_attr_req[5] = TERM_MAGENTA;
					d_attr_req[6] = TERM_L_VIOLET;
					d_attr_req[7] = TERM_L_PINK;
					strcpy(name, "school of sorcery");
				}
				/* Priests and paladins */
				else if (i == 1) {
					d_attr_req[0] = TERM_WHITE;
					d_attr_req[1] = TERM_GREEN;
					d_attr_req[2] = TERM_L_GREEN;
					strcpy(name, "temple of piety");
				}
				/* Mystics and ninjas */
				else if (i == 2) {
					d_attr_req[0] = TERM_ORANGE;
					d_attr_req[1] = TERM_YELLOW;
					d_attr_req[2] = TERM_L_YELLOW;
					strcpy(name, "gathering of mystery");
				}
				/* Thieves and assassins */
				else if (i == 3) {
					d_attr_req[0] = TERM_BLUE;
					d_attr_req[1] = TERM_L_BLUE;
					d_attr_req[2] = TERM_TEAL;
					d_attr_req[3] = TERM_L_TEAL;
					d_attr_req[4] = TERM_BLUE_SLATE;
					d_attr_req[5] = TERM_DEEP_L_BLUE;
					strcpy(name, "den of thieves");
				}
				/* Warriors and rangers */
				else {
					d_attr_req[0] = TERM_SLATE;
					d_attr_req[1] = TERM_UMBER;
					d_attr_req[2] = TERM_L_DARK;
					d_attr_req[3] = TERM_L_WHITE;
					d_attr_req[4] = TERM_L_UMBER;
					d_attr_req[5] = TERM_MUD;
					d_attr_req[6] = TERM_MUSTARD;
					strcpy(name, "fighter's hall");
				}
			} else {
				strcpy(name, "humans and humanoids");
			}
	    }

	    /* Usually, just accept the symbol. */
	    else {
			d_char_req[0] = symbol;

			if (symbol == 'p')
				strcpy(name, "human");
			else if (symbol == 'h')
				strcpy(name, "humanoid");
	    }

	    *ordered = FALSE;
	    break;
	}

	/* Orcs */
    case 'o':
	{
	    strcpy(name, "orc");
	    strcpy(d_char_req, "o");
	    *ordered = TRUE;
	    break;
	}

	/* Trolls */
    case 'T':
	{
	    strcpy(name, "troll");
	    strcpy(d_char_req, "T");
	    *ordered = TRUE;
	    break;
	}

	/* Giants (sometimes ogres at low levels) */
    case 'P':
	{
	    strcpy(name, "giant");
	    if ((p_ptr->depth < 30) && (randint0(3) == 0))
			strcpy(d_char_req, "O");
	    else
			strcpy(d_char_req, "P");
	    *ordered = TRUE;
	    break;
	}

	/* Orcs, ogres, trolls, or giants */
    case '%':
	{
	    strcpy(name, "moria");
	    strcpy(d_char_req, "oOPT");
	    *ordered = FALSE;
	    break;
	}

	/* Monsters found in caves */
    case '0':
	{
	    strcpy(name, "dungeon monsters");
	    strcpy(d_char_req, "ykoOT");
	    *ordered = FALSE;
	    break;
	}

	/* Undead */
    case 'N':
	{
	    /* Sometimes, restrict by symbol. */
	    if ((depth > 40) && (randint0(3) == 0)) {
			for (i = 0; i < 500; i++) {
				/* Find a suitable monster near depth. */
				j = randint1(z_info->r_max - 1);

				/* Require a non-unique undead. */
				if (rf_has(r_info[j].flags, RF_UNDEAD)
					&& (!rf_has(r_info[j].flags, RF_UNIQUE))
					&& (strchr("GLWV", r_info[j].d_char))
					&& (ABS(r_info[j].level - p_ptr->depth) <
						1 + (p_ptr->depth / 4))) {
					break;
				}
			}

			/* If we find a monster, */
			if (i < 499) {
				/* Use that monster's symbol for all monsters */
				d_char_req[0] = r_info[j].d_char;

				/* No pit name (yet) */

				/* In this case, we do order the monsters */
				*ordered = TRUE;
			} else {
				/* Accept any undead. */
				strcpy(name, "undead");
				racial_flag_mask = RF_UNDEAD;
				*ordered = FALSE;
			}
	    } else {
			/* No restrictions on symbol. */
			strcpy(name, "undead");
			racial_flag_mask = RF_UNDEAD;
			*ordered = FALSE;
	    }
	    break;
	}

	/* Demons */
    case 'u':
    case 'U':
	{
	    strcpy(name, "demon");
	    if (depth > 55)
			strcpy(d_char_req, "U");
	    else if (depth < 40)
			strcpy(d_char_req, "u");
	    else
			strcpy(d_char_req, "uU");
	    *ordered = TRUE;
	    break;
	}

	/* Dragons */
    case 'd':
    case 'D':
	{
	    strcpy(d_char_req, "dD");

	    /* Dragons usually associate with others of their kind. */
	    if (randint0(6) != 0) {
			/* Dragons of a single kind are ordered. */
			*ordered = TRUE;

			/* Some dragon types are not found everywhere */
			if (depth > 70)
				i = randint0(35);
			else if (depth > 45)
				i = randint0(32);
			else if (depth > 32)
				i = randint0(30);
			else if (depth > 23)
				i = randint0(28);
			else
				i = randint0(24);

			if (i < 4) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_ACID,
						   FLAG_END);
				strcpy(name, "dragon - acid");
			} else if (i < 8) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_ELEC,
						   FLAG_END);
				strcpy(name, "dragon - electricity");
			} else if (i < 12) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_FIRE,
						   FLAG_END);
				strcpy(name, "dragon - fire");
			} else if (i < 16) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_COLD,
						   FLAG_END);
				strcpy(name, "dragon - cold");
			} else if (i < 20) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_POIS,
						   FLAG_END);
				strcpy(name, "dragon - poison");
			} else if (i < 24) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_ACID,
						   RSF_BR_ELEC, RSF_BR_FIRE, RSF_BR_COLD,
						   RSF_BR_POIS, FLAG_END);
				strcpy(name, "dragon - multihued");
			} else if (i < 28) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_SOUN,
						   FLAG_END);
				strcpy(name, "dragon - sound");
			} else if (i < 30) {
				flags_init(breath_flag_mask, RSF_SIZE, RSF_BR_LIGHT,
						   RSF_BR_DARK, FLAG_END);
				strcpy(name, "dragon - ethereal");
			}

			/* Chaos, Law, Balance, Power, etc.) */
			else {
				d_attr_req[0] = TERM_VIOLET;
				d_attr_req[1] = TERM_L_BLUE;
				d_attr_req[2] = TERM_L_GREEN;
				strcpy(name, "dragon - arcane");
			}
	    } else {
			strcpy(name, "dragon - mixed");

			/* Dragons of all kinds are not ordered. */
			*ordered = FALSE;
	    }
	    break;
	}

	/* Vortexes and elementals */
    case 'v':
    case 'E':
	{
	    /* Usually, just have any kind of 'v' or 'E' */
	    if (randint0(3) != 0) {
			d_char_req[0] = symbol;

			if (symbol == 'v')
				strcpy(name, "vortex");
			if (symbol == 'E')
				strcpy(name, "elemental");
	    }

	    /* Sometimes, choose both 'v' and 'E's of one element */
	    else {
			strcpy(d_char_req, "vE");

			i = randint0(4);

			/* Fire */
			if (i == 0) {
				d_attr_req[0] = TERM_RED;
				strcpy(name, "fire");
			}
			/* Frost */
			if (i == 1) {
				d_attr_req[0] = TERM_L_WHITE;
				d_attr_req[1] = TERM_WHITE;
				strcpy(name, "frost");
			}
			/* Air/electricity */
			if (i == 2) {
				d_attr_req[0] = TERM_L_BLUE;
				d_attr_req[1] = TERM_BLUE;
				strcpy(name, "air");
			}
			/* Acid/water/earth */
			if (i == 3) {
				d_attr_req[0] = TERM_GREEN;
				d_attr_req[1] = TERM_L_UMBER;
				d_attr_req[2] = TERM_UMBER;
				d_attr_req[3] = TERM_SLATE;
				strcpy(name, "earth & water");
			}
	    }

	    *ordered = FALSE;
	    break;
	}

	/* Special case: mimics and treasure */
    case '!':
    case '?':
    case '=':
    case '~':
    case '|':
    case '.':
    case '$':
	{
	    if (symbol == '$') {
			strcpy(name, "treasure");

			/* Nothing but loot! */
			if (randint0(3) == 0)
				strcpy(d_char_req, "$");

			/* Guard the money well. */
			else
				strcpy(d_char_req, "$!?=~|.");
	    } else {
			/* No treasure. */
			strcpy(d_char_req, "!?=~|.");
			strcpy(name, "mimic");
	    }

	    *ordered = FALSE;
	    break;
	}

	/* Special case: creatures of earth. */
    case 'X':
    case '#':
	{
	    strcpy(d_char_req, "X#");
	    strcpy(name, "creatures of earth");
	    *ordered = FALSE;
	    break;
	}

	/* Space for more monster types here. */


	/* Any symbol not handled elsewhere. */
    default:
	{
		monster_base *race;

	    /* Accept the character. */
	    d_char_req[0] = symbol;

		/* Get name from monster template */
		for (race = rb_info; race; race = race->next){
			if (char_matches_key(race->d_char, symbol)) {
				strnfmt(name, sizeof(name), "%s", race->text);
			}
		}

	    /* Some monsters should logically be ordered. */
	    if (strchr("knosuyzGLMOPTUVW", symbol))
			*ordered = TRUE;

	    /* Most should not */
	    else
			*ordered = FALSE;

	    break;
	}
    }

    /* Prepare allocation table */
    get_mon_num_prep(mon_select);

    /* Return the name. */
    return (format("%s", name));
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
void spread_monsters(struct cave *c, char symbol, int depth, int num, int y0, int x0, int dy, int dx, byte origin)
{
    int i, j;			/* Limits on loops */
    int count;
    int y = y0, x = x0;
    int start_mon_num = c->mon_max;
    bool dummy;

    /* Restrict monsters.  Allow uniques. */
    (void) mon_restrict(symbol, (byte) depth, &dummy, TRUE);

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
    (void) mon_restrict('\0', (byte) depth, &dummy, TRUE);
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
		int attr;

		/* Require correct race, allow uniques. */
		allow_unique = TRUE;
		sprintf(d_char_req, "%c", racial_symbol[i]);
		for (attr = 0; attr < 8; attr++)
			d_attr_req[attr] = 0;
		racial_flag_mask = 0;
		rsf_wipe(breath_flag_mask);

		/* Determine level of monster */
		if (vault_type == 6)
			depth = p_ptr->depth + 2;
		else if (vault_type == 7)
			depth = p_ptr->depth + 4;
		else if (vault_type == 8)
			depth = p_ptr->depth + 6;
		else
			depth = p_ptr->depth;

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
