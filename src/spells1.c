/*
 * File: spells1.c
 * Purpose: Some spell effects, and the project() function
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
#include "generate.h"
#include "object/tvalsval.h"
#include "monster/constants.h"
#include "monster/monster.h"
#include "squelch.h"
#include "trap.h"

/*
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
s16b poly_r_idx(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	int i, r, lev1, lev2;

	/* Paranoia -- Uniques never polymorph */
	if (rf_has(r_ptr->flags, RF_UNIQUE)) return (r_idx);

	/* Allowable range of "levels" for resulting monster */
	lev1 = r_ptr->level - ((randint1(20)/randint1(9))+1);
	lev2 = r_ptr->level + ((randint1(20)/randint1(9))+1);

	/* Pick a (possibly new) non-unique race */
	for (i = 0; i < 1000; i++)
	{
		/* Pick a new race, using a level calculation */
		r = get_mon_num((p_ptr->depth + r_ptr->level) / 2 + 5);

		/* Handle failure */
		if (!r) break;

		/* Obtain race */
		r_ptr = &r_info[r];

		/* Ignore unique monsters */
		if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

		/* Ignore monsters with incompatible levels */
		if ((r_ptr->level < lev1) || (r_ptr->level > lev2)) continue;

		/* Use that index */
		r_idx = r;

		/* Done */
		break;
	}

	/* Result */
	return (r_idx);
}


/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(int m_idx, int dis)
{
	int ny = 0, nx = 0, oy, ox, d, i, min;

	bool look = TRUE;

	monster_type *m_ptr = &mon_list[m_idx];


	/* Paranoia */
	if (!m_ptr->r_idx) return;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds_fully(ny, nx)) continue;

			/* Require "empty" floor space */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave_feat[ny][nx] == FEAT_GLYPH) continue;

			/* No teleporting into vaults and such */
			/* if (cave_info[ny][nx] & (CAVE_ICKY)) continue; */

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TPOTHER);

	/* Swap the monsters */
	monster_swap(oy, ox, ny, nx);
}


/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
void teleport_player(int dis)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int d, i, min, y, x;

	bool look = TRUE;


	/* Initialize */
	y = py;
	x = px;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(py, dis);
				x = rand_spread(px, dis);
				d = distance(py, px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds_fully(y, x)) continue;

			/* Require "naked" floor space */
			if (!cave_naked_bold(y, x)) continue;

			/* No teleporting into vaults and such */
			if (cave_info[y][x] & (CAVE_ICKY)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;

	int dis = 0, ctr = 0;

	/* Initialize */
	y = py;
	x = px;

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (in_bounds_fully(y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (cave_naked_bold(y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{

	if (is_quest(p_ptr->depth) || (p_ptr->depth >= MAX_DEPTH-1))
	{
		if (OPT(adult_ironman))
		{
			msg_print("Nothing happens.");
			return;
		}

		message(MSG_TPLEVEL, 0, "You rise up through the ceiling.");

		/* New depth */
		p_ptr->depth--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else if ((!p_ptr->depth) || (OPT(adult_ironman)))
	{
		message(MSG_TPLEVEL, 0, "You sink through the floor.");

		/* New depth */
		p_ptr->depth++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else if (randint0(100) < 50)
	{
		message(MSG_TPLEVEL, 0, "You rise up through the ceiling.");

		/* New depth */
		p_ptr->depth--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else
	{
		message(MSG_TPLEVEL, 0, "You sink through the floor.");

		/* New depth */
		p_ptr->depth++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
}






/*
 * Return a color to use for the bolt/ball spells
 */
static byte spell_color(int type)
{
	/* Analyze */
	switch (type)
	{
		case GF_MISSILE:	return (TERM_VIOLET);
		case GF_ACID:		return (TERM_SLATE);
		case GF_ELEC:		return (TERM_BLUE);
		case GF_FIRE:		return (TERM_RED);
		case GF_COLD:		return (TERM_WHITE);
		case GF_POIS:		return (TERM_GREEN);
		case GF_HOLY_ORB:	return (TERM_L_DARK);
		case GF_MANA:		return (TERM_L_DARK);
		case GF_ARROW:		return (TERM_WHITE);
		case GF_WATER:		return (TERM_SLATE);
		case GF_NETHER:		return (TERM_L_GREEN);
		case GF_CHAOS:		return (TERM_VIOLET);
		case GF_DISENCHANT:	return (TERM_VIOLET);
		case GF_NEXUS:		return (TERM_L_RED);
		case GF_CONFUSION:	return (TERM_L_UMBER);
		case GF_SOUND:		return (TERM_YELLOW);
		case GF_SHARD:		return (TERM_UMBER);
		case GF_FORCE:		return (TERM_UMBER);
		case GF_INERTIA:	return (TERM_L_WHITE);
		case GF_GRAVITY:	return (TERM_L_WHITE);
		case GF_TIME:		return (TERM_L_BLUE);
		case GF_LIGHT_WEAK:	return (TERM_ORANGE);
		case GF_LIGHT:		return (TERM_ORANGE);
		case GF_DARK_WEAK:	return (TERM_L_DARK);
		case GF_DARK:		return (TERM_L_DARK);
		case GF_PLASMA:		return (TERM_RED);
		case GF_METEOR:		return (TERM_RED);
		case GF_ICE:		return (TERM_WHITE);
	}

	/* Standard "color" */
	return (TERM_WHITE);
}



/*
 * Find the attr/char pair to use for a spell effect
 *
 * It is moving (or has moved) from (x,y) to (nx,ny).
 *
 * If the distance is not "one", we (may) return "*".
 */
static u16b bolt_pict(int y, int x, int ny, int nx, int typ)
{
	int base;

	byte k;

	byte a;
	char c;

	if (!(use_graphics && (arg_graphics == GRAPHICS_DAVID_GERVAIS)))
	{
		/* No motion (*) */
		if ((ny == y) && (nx == x)) base = 0x30;

		/* Vertical (|) */
		else if (nx == x) base = 0x40;

		/* Horizontal (-) */
		else if (ny == y) base = 0x50;

		/* Diagonal (/) */
		else if ((ny-y) == (x-nx)) base = 0x60;

		/* Diagonal (\) */
		else if ((ny-y) == (nx-x)) base = 0x70;

		/* Weird (*) */
		else base = 0x30;

		/* Basic spell color */
		k = spell_color(typ);

		/* Reduce to allowed colour range for spell bolts/balls - see e.g. font-xxx.prf */
		k = get_color(k, ATTR_MISC, 1);

		/* Obtain attr/char */
		a = misc_to_attr[base+k];
		c = misc_to_char[base+k];
	}
	else
	{
		int add;

		/* No motion (*) */
		if ((ny == y) && (nx == x)) {base = 0x00; add = 0;}

		/* Vertical (|) */
		else if (nx == x) {base = 0x40; add = 0;}

		/* Horizontal (-) */
		else if (ny == y) {base = 0x40; add = 1;}

		/* Diagonal (/) */
		else if ((ny-y) == (x-nx)) {base = 0x40; add = 2;}

		/* Diagonal (\) */
		else if ((ny-y) == (nx-x)) {base = 0x40; add = 3;}

		/* Weird (*) */
		else {base = 0x00; add = 0;}

		if (typ >= 0x40) k = 0;
		else k = typ;

		/* Obtain attr/char */
		a = misc_to_attr[base+k];
		c = misc_to_char[base+k] + add;
	}

	/* Create pict */
	return (PICT(a,c));
}




/*
 * Decreases players hit points and sets death flag if necessary
 *
 * Invulnerability needs to be changed into a "shield" XXX XXX XXX
 *
 * Hack -- this function allows the user to save (or quit) the game
 * when he dies, since the "You die." message is shown before setting
 * the player to "dead".
 */
void take_hit(int dam, cptr kb_str)
{
	int old_chp = p_ptr->chp;

	int warning = (p_ptr->mhp * op_ptr->hitpoint_warn / 10);


	/* Paranoia */
	if (p_ptr->is_dead) return;


	/* Disturb */
	disturb(1, 0);

	/* Mega-Hack -- Apply "invulnerability" */
	if (p_ptr->timed[TMD_INVULN] && (dam < 9000)) return;

	/* Hurt the player */
	p_ptr->chp -= dam;

	/* Display the hitpoints */
	p_ptr->redraw |= (PR_HP);

	/* Dead player */
	if (p_ptr->chp < 0)
	{
		/* Hack -- Note death */
		message(MSG_DEATH, 0, "You die.");
		message_flush();

		/* Note cause of death */
		my_strcpy(p_ptr->died_from, kb_str, sizeof(p_ptr->died_from));

		/* No longer a winner */
		p_ptr->total_winner = FALSE;

		/* Note death */
		p_ptr->is_dead = TRUE;

		/* Leaving */
		p_ptr->leaving = TRUE;

		/* Dead */
		return;
	}

	/* Hitpoint warning */
	if (p_ptr->chp < warning)
	{
		/* Hack -- bell on first notice */
		if (old_chp > warning)
		{
			bell("Low hitpoint warning!");
		}

		/* Message */
		message(MSG_HITPOINT_WARN, 0, "*** LOW HITPOINT WARNING! ***");
		message_flush();
	}
}





/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
static bool hates_acid(const object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable items */
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls are wood/paper */
		case TV_STAFF:
		case TV_SCROLL:
		{
			return (TRUE);
		}

		/* Ouch */
		case TV_CHEST:
		{
			return (TRUE);
		}

		/* Junk is useless */
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate electricity?
 */
static bool hates_elec(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_RING:
		case TV_WAND:
		case TV_ROD:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
static bool hates_fire(const object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable */
		case TV_LIGHT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		{
			return (TRUE);
		}

		/* Books */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			return (TRUE);
		}

		/* Chests */
		case TV_CHEST:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls burn */
		case TV_STAFF:
		case TV_SCROLL:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
static bool hates_cold(const object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_POTION:
		case TV_FLASK:
		case TV_BOTTLE:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}









/*
 * Melt something
 */
static int set_acid_destroy(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];
	if (!hates_acid(o_ptr)) return (FALSE);
	object_flags(o_ptr, f);
	if (of_has(f, OF_IGNORE_ACID)) return (FALSE);
	return (TRUE);
}


/*
 * Electrical damage
 */
static int set_elec_destroy(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];
	if (!hates_elec(o_ptr)) return (FALSE);
	object_flags(o_ptr, f);
	if (of_has(f, OF_IGNORE_ELEC)) return (FALSE);
	return (TRUE);
}


/*
 * Burn something
 */
static int set_fire_destroy(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];
	if (!hates_fire(o_ptr)) return (FALSE);
	object_flags(o_ptr, f);
	if (of_has(f, OF_IGNORE_FIRE)) return (FALSE);
	return (TRUE);
}


/*
 * Freeze things
 */
static int set_cold_destroy(const object_type *o_ptr)
{
	bitflag f[OF_SIZE];
	if (!hates_cold(o_ptr)) return (FALSE);
	object_flags(o_ptr, f);
	if (of_has(f, OF_IGNORE_COLD)) return (FALSE);
	return (TRUE);
}




/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(const object_type *);

/*
 * Destroys a type of item on a given percent chance.
 * The chance 'cperc' is in hundredths of a percent (1-in-10000)
 * Note that missiles are no longer necessarily all destroyed
 *
 * Returns number of items destroyed.
 */
static int inven_damage(inven_func typ, int cperc)
{
	int i, j, k, amt;

	object_type *o_ptr;

	char o_name[80];
	
	bool damage;

	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < QUIVER_END; i++)
	{
		if (i >= INVEN_PACK && i < QUIVER_START) continue;

		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Hack -- for now, skip artifacts */
		if (artifact_p(o_ptr)) continue;

		/* Give this item slot a shot at death */
		if ((*typ)(o_ptr))
		{
			/* Chance to destroy this item */
			int chance = cperc;

			/* Track if it is damaged instead of destroyed */
			damage = FALSE;

			/* Analyze the type to see if we just damage it */
			switch (o_ptr->tval)
			{
				/* Weapons */
				case TV_BOW:
				case TV_SWORD:
				case TV_HAFTED:
				case TV_POLEARM:
				case TV_DIGGING:
				{
					/* Chance to damage it */
					if (randint0(10000 * 100) < cperc)
					{
						/* Damage the item */
						o_ptr->to_h--;
						o_ptr->to_d--;

						/* Damaged! */
						damage = TRUE;
					}
					else continue;

					break;
				}

				/* Wearable items */
				case TV_HELM:
				case TV_CROWN:
				case TV_SHIELD:
				case TV_BOOTS:
				case TV_GLOVES:
				case TV_CLOAK:
				case TV_SOFT_ARMOR:
				case TV_HARD_ARMOR:
				case TV_DRAG_ARMOR:
				{
					/* Chance to damage it */
					if (randint0(10000 * 100) < cperc)
					{
						/* Damage the item */
						o_ptr->to_a--;

						/* Damaged! */
						damage = TRUE;
					}
					else continue;

					break;
				}
				
				/* Rods are tough */
				case TV_ROD:
				{
					chance = (chance / 4);
					
					break;
				}
			}

			/* Damage instead of destroy */
			if (damage)
			{
				/* Calculate bonuses */
				p_ptr->update |= (PU_BONUS);

				/* Window stuff */
				p_ptr->redraw |= (PR_EQUIP);

				/* Casualty count */
				amt = o_ptr->number;
			}

			/* Count the casualties */
			else for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (randint0(10000) < chance) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

				/* Message */
				message_format(MSG_DESTROY, 0, "%sour %s (%c) %s %s!",
				           ((o_ptr->number > 1) ?
				            ((amt == o_ptr->number) ? "All of y" :
				             (amt > 1 ? "Some of y" : "One of y")) : "Y"),
				           o_name, index_to_label(i),
				           ((amt > 1) ? "were" : "was"),
					   (damage ? "damaged" : "destroyed"));

				/* Damage already done? */
				if (damage) continue;

				/* Hack -- If rods, wands, or staves are destroyed, the total
				 * maximum timeout or charges of the stack needs to be reduced,
				 * unless all the items are being destroyed. -LM-
				 */
				if (((o_ptr->tval == TV_WAND) ||
				     (o_ptr->tval == TV_STAFF) ||
				     (o_ptr->tval == TV_ROD)) &&
				    (amt < o_ptr->number))
				{
					o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;
				}

				/* Destroy "amt" items */
				inven_item_increase(i, -amt);
				inven_item_optimize(i);

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}




/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(void)
{
	object_type *o_ptr = NULL;

	bitflag f[OF_SIZE];

	char o_name[80];


	/* Pick a (possibly empty) inventory slot */
	switch (randint1(6))
	{
		case 1: o_ptr = &p_ptr->inventory[INVEN_BODY]; break;
		case 2: o_ptr = &p_ptr->inventory[INVEN_ARM]; break;
		case 3: o_ptr = &p_ptr->inventory[INVEN_OUTER]; break;
		case 4: o_ptr = &p_ptr->inventory[INVEN_HANDS]; break;
		case 5: o_ptr = &p_ptr->inventory[INVEN_HEAD]; break;
		case 6: o_ptr = &p_ptr->inventory[INVEN_FEET]; break;
	}

	/* Nothing to damage */
	if (!o_ptr->k_idx) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);


	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Object resists */
	if (of_has(f, OF_IGNORE_ACID))
	{
		msg_format("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Message */
	msg_format("Your %s is damaged!", o_name);

	/* Damage the item */
	o_ptr->to_a--;

	/* Calculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->redraw |= (PR_EQUIP);

	/* Item was damaged */
	return (TRUE);
}


/*
 * Hurt the player with Acid
 */
void acid_dam(int dam, cptr kb_str)
{
	int n;
	int inv;

	if (dam <= 0) return;

	/* Resist the damage */
	if (p_ptr->state.immune_acid) n = 3;
	else if (p_ptr->state.resist_acid) n = 1;
	else n = 0;

	if (p_ptr->state.vuln_acid) n--;
	if (p_ptr->timed[TMD_OPP_ACID]) n++;

	/* Notice flags */
	wieldeds_notice_flag(OF_IM_ACID);
	wieldeds_notice_flag(OF_RES_ACID);
	wieldeds_notice_flag(OF_VULN_ACID);

	/* Change damage */
	if (n >= 3) return;
	else if (n >= 2)  dam = DBLRES_ACID_ADJ(dam, NOT_USED);
	else if (n == 1)  dam = RES_ACID_ADJ(dam, NOT_USED);
	else if (n == -1) dam = VULN_ACID_ADJ(dam, NOT_USED);

	/* If any armor gets hit, defend the player */
	if (minus_ac()) dam = (dam + 1) / 2;

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inv = (MIN(dam, 60) * 100) / 20;
	inven_damage(set_acid_destroy, inv);
}




/*
 * Hurt the player with electricity
 */
void elec_dam(int dam, cptr kb_str)
{
	int n;
	int inv;

	if (dam <= 0) return;

	/* Resist the damage */
	if (p_ptr->state.immune_elec) n = 3;
	else if (p_ptr->state.resist_elec) n = 1;
	else n = 0;

	if (p_ptr->state.vuln_elec) n--;
	if (p_ptr->timed[TMD_OPP_ELEC]) n++;

	/* Notice flags */
	wieldeds_notice_flag(OF_IM_ELEC);
	wieldeds_notice_flag(OF_RES_ELEC);
	wieldeds_notice_flag(OF_VULN_ELEC);

	/* Change damage */
	if (n >= 3) return;
	else if (n >= 2) dam = DBLRES_ELEC_ADJ(dam, NOT_USED);
	else if (n == 1) dam = RES_ELEC_ADJ(dam, NOT_USED);
	else if (n == -1) dam = VULN_ELEC_ADJ(dam, NOT_USED);

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inv = (MIN(dam, 60) * 100) / 20;
	inven_damage(set_elec_destroy, inv);
}




/*
 * Hurt the player with Fire
 */
void fire_dam(int dam, cptr kb_str)
{
	int n;
	int inv;

	if (dam <= 0) return;

	/* Resist the damage */
	if (p_ptr->state.immune_fire) n = 3;
	else if (p_ptr->state.resist_fire) n = 1;
	else n = 0;

	if (p_ptr->state.vuln_fire) n--;
	if (p_ptr->timed[TMD_OPP_FIRE]) n++;

	/* Notice flags */
	wieldeds_notice_flag(OF_IM_FIRE);
	wieldeds_notice_flag(OF_RES_FIRE);
	wieldeds_notice_flag(OF_VULN_FIRE);

	/* Change damage */
	if (n >= 3) return;
	else if (n >= 2) dam = DBLRES_FIRE_ADJ(dam, NOT_USED);
	else if (n == 1) dam = RES_FIRE_ADJ(dam, NOT_USED);
	else if (n == -1) dam = VULN_FIRE_ADJ(dam, NOT_USED);

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inv = (MIN(dam, 60) * 100) / 20;
	inven_damage(set_fire_destroy, inv);
}




/*
 * Hurt the player with Cold
 */
void cold_dam(int dam, cptr kb_str)
{
	int n;
	int inv;

	if (dam <= 0) return;

	/* Resist the damage */
	if (p_ptr->state.immune_cold) n = 3;
	else if (p_ptr->state.resist_cold) n = 1;
	else n = 0;

	if (p_ptr->state.vuln_cold) n--;
	if (p_ptr->timed[TMD_OPP_COLD]) n++;

	/* Notice flags */
	wieldeds_notice_flag(OF_IM_COLD);
	wieldeds_notice_flag(OF_RES_COLD);
	wieldeds_notice_flag(OF_VULN_COLD);

	/* Change damage */
	if (n >= 3) return;
	else if (n >= 2) dam = DBLRES_COLD_ADJ(dam, NOT_USED);
	else if (n == 1) dam = RES_COLD_ADJ(dam, NOT_USED);
	else if (n == -1) dam = VULN_COLD_ADJ(dam, NOT_USED);

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inv = (MIN(dam, 60) * 100) / 20;
	inven_damage(set_cold_destroy, inv);
}

/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
	/* Restore if needed */
	if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
	{
		/* Restore */
		p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return (TRUE);
	}

	/* Nothing to restore */
	return (FALSE);
}




/*
 * Apply disenchantment to the player's stuff
 *
 * This function is also called from the "melee" code.
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything.
 */
bool apply_disenchant(int mode)
{
	int t = 0;

	object_type *o_ptr;

	char o_name[80];


	/* Unused parameter */
	(void)mode;

	/* Pick a random slot */
	switch (randint1(8))
	{
		case 1: t = INVEN_WIELD; break;
		case 2: t = INVEN_BOW; break;
		case 3: t = INVEN_BODY; break;
		case 4: t = INVEN_OUTER; break;
		case 5: t = INVEN_ARM; break;
		case 6: t = INVEN_HEAD; break;
		case 7: t = INVEN_HANDS; break;
		case 8: t = INVEN_FEET; break;
	}

	/* Get the item */
	o_ptr = &p_ptr->inventory[t];

	/* No item, nothing happens */
	if (!o_ptr->k_idx) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}


	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);


	/* Artifacts have 60% chance to resist */
	if (artifact_p(o_ptr) && (randint0(100) < 60))
	{
		/* Message */
		msg_format("Your %s (%c) resist%s disenchantment!",
		           o_name, index_to_label(t),
		           ((o_ptr->number != 1) ? "" : "s"));

		/* Notice */
		return (TRUE);
	}


	/* Disenchant tohit */
	if (o_ptr->to_h > 0) o_ptr->to_h--;
	if ((o_ptr->to_h > 5) && (randint0(100) < 20)) o_ptr->to_h--;

	/* Disenchant todam */
	if (o_ptr->to_d > 0) o_ptr->to_d--;
	if ((o_ptr->to_d > 5) && (randint0(100) < 20)) o_ptr->to_d--;

	/* Disenchant toac */
	if (o_ptr->to_a > 0) o_ptr->to_a--;
	if ((o_ptr->to_a > 5) && (randint0(100) < 20)) o_ptr->to_a--;

	/* Message */
	msg_format("Your %s (%c) %s disenchanted!",
	           o_name, index_to_label(t),
	           ((o_ptr->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->redraw |= (PR_EQUIP);

	/* Notice */
	return (TRUE);
}


/*
 * Apply Nexus
 */
static void apply_nexus(const monster_type *m_ptr)
{
	int max1, cur1, max2, cur2, ii, jj;

	switch (randint1(7))
	{
		case 1: case 2: case 3:
		{
			teleport_player(200);
			break;
		}

		case 4: case 5:
		{
			teleport_player_to(m_ptr->fy, m_ptr->fx);
			break;
		}

		case 6:
		{
			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
				break;
			}

			/* Teleport Level */
			teleport_player_level();
			break;
		}

		case 7:
		{
			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
				break;
			}

			msg_print("Your body starts to scramble...");

			/* Pick a pair of stats */
			ii = randint0(A_MAX);
			for (jj = ii; jj == ii; jj = randint0(A_MAX)) /* loop */;

			max1 = p_ptr->stat_max[ii];
			cur1 = p_ptr->stat_cur[ii];
			max2 = p_ptr->stat_max[jj];
			cur2 = p_ptr->stat_cur[jj];

			p_ptr->stat_max[ii] = max2;
			p_ptr->stat_cur[ii] = cur2;
			p_ptr->stat_max[jj] = max1;
			p_ptr->stat_cur[jj] = cur1;

			p_ptr->update |= (PU_BONUS);

			break;
		}
	}
}





/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;



/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * Hack -- We also "see" grids which are "memorized".
 *
 * Perhaps we should affect doors and/or walls.
 */
static bool project_f(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	/* Unused parameters */
	(void)who;
	(void)r;
	(void)dam;

#if 0 /* unused */
	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);
#endif /* 0 */

	/* Analyze the type */
	switch (typ)
	{
		/* Ignore most effects */
		case GF_ACID:
		case GF_ELEC:
		case GF_FIRE:
		case GF_COLD:
		case GF_PLASMA:
		case GF_METEOR:
		case GF_ICE:
		case GF_SHARD:
		case GF_FORCE:
		case GF_SOUND:
		case GF_MANA:
		case GF_HOLY_ORB:
		{
			break;
		}

		/* Destroy Traps (and Locks) */
		case GF_KILL_TRAP:
		{
			/* Reveal secret doors */
			if (cave_feat[y][x] == FEAT_SECRET)
			{
				place_closed_door(y, x);

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					obvious = TRUE;
				}
			}

			/* Destroy traps */
			if ((cave_feat[y][x] == FEAT_INVIS) ||
			    ((cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
			     (cave_feat[y][x] <= FEAT_TRAP_TAIL)))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					msg_print("There is a bright flash of light!");
					obvious = TRUE;
				}

				/* Forget the trap */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the trap */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Locked doors are unlocked */
			else if ((cave_feat[y][x] >= FEAT_DOOR_HEAD + 0x01) &&
			          (cave_feat[y][x] <= FEAT_DOOR_HEAD + 0x07))
			{
				/* Unlock the door */
				cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);

				/* Check line of sound */
				if (player_has_los_bold(y, x))
				{
					msg_print("Click!");
					obvious = TRUE;
				}
			}

			break;
		}

		/* Destroy Doors (and traps) */
		case GF_KILL_DOOR:
		{
			/* Destroy all doors and traps */
			if ((cave_feat[y][x] == FEAT_OPEN) ||
			    (cave_feat[y][x] == FEAT_BROKEN) ||
			    (cave_feat[y][x] == FEAT_INVIS) ||
			    ((cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
			     (cave_feat[y][x] <= FEAT_TRAP_TAIL)) ||
			    ((cave_feat[y][x] >= FEAT_DOOR_HEAD) &&
			     (cave_feat[y][x] <= FEAT_DOOR_TAIL)))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("There is a bright flash of light!");
					obvious = TRUE;

					/* Visibility change */
					if ((cave_feat[y][x] >= FEAT_DOOR_HEAD) &&
					    (cave_feat[y][x] <= FEAT_DOOR_TAIL))
					{
						/* Update the visuals */
						p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
					}
				}

				/* Forget the door */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			break;
		}

		/* Destroy walls (and doors) */
		case GF_KILL_WALL:
		{
			/* Non-walls (etc) */
			if (cave_floor_bold(y, x)) break;

			/* Permanent walls */
			if (cave_feat[y][x] >= FEAT_PERM_EXTRA) break;

			/* Granite */
			if (cave_feat[y][x] >= FEAT_WALL_EXTRA)
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The wall turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Quartz / Magma with treasure */
			else if (cave_feat[y][x] >= FEAT_MAGMA_H)
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The vein turns into mud!");
					msg_print("You have found something!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);

				/* Place some gold */
				place_gold(y, x, p_ptr->depth);
			}

			/* Quartz / Magma */
			else if (cave_feat[y][x] >= FEAT_MAGMA)
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The vein turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Rubble */
			else if (cave_feat[y][x] == FEAT_RUBBLE)
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The rubble turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the rubble */
				cave_set_feat(y, x, FEAT_FLOOR);

				/* Hack -- place an object */
				if (randint0(100) < 10)
				{
					/* Found something */
					if (player_can_see_bold(y, x))
					{
						msg_print("There was something buried in the rubble!");
						obvious = TRUE;
					}

					/* Place gold */
					place_object(y, x, p_ptr->depth, FALSE, FALSE);
				}
			}

			/* Destroy doors (and secret doors) */
			else /* if (cave_feat[y][x] >= FEAT_DOOR_HEAD) */
			{
				/* Hack -- special message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The door turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Fully update the flow */
			p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

			break;
		}

		/* Make doors */
		case GF_MAKE_DOOR:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Create closed door */
			cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);

			/* Observe */
			if (cave_info[y][x] & (CAVE_MARK)) obvious = TRUE;

			/* Update the visuals */
			p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			break;
		}

		/* Make traps */
		case GF_MAKE_TRAP:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Place a trap */
			place_trap(y, x);

			break;
		}

		/* Light up the grid */
		case GF_LIGHT_WEAK:
		case GF_LIGHT:
		{
			/* Turn on the light */
			cave_info[y][x] |= (CAVE_GLOW);

			/* Grid is in line of sight */
			if (player_has_los_bold(y, x))
			{
				if (!p_ptr->timed[TMD_BLIND])
				{
					/* Observe */
					obvious = TRUE;
				}

				/* Fully update the visuals */
				p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
			}

			break;
		}

		/* Darken the grid */
		case GF_DARK_WEAK:
		case GF_DARK:
		{
			if (p_ptr->depth != 0 || !is_daytime())
			{
				/* Turn off the light */
				cave_info[y][x] &= ~(CAVE_GLOW);

				/* Hack -- Forget "boring" grids */
				if (cave_feat[y][x] <= FEAT_INVIS)
					cave_info[y][x] &= ~(CAVE_MARK);
			}

			/* Grid is in line of sight */
			if (player_has_los_bold(y, x))
			{
				/* Observe */
				obvious = TRUE;

				/* Fully update the visuals */
				p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
			}

			/* All done */
			break;
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}



/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- We also "see" objects which are "memorized".
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
static bool project_o(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	s16b this_o_idx, next_o_idx = 0;

	bitflag f[OF_SIZE];

	char o_name[80];


	/* Unused parameters */
	(void)who;
	(void)r;
	(void)dam;

#if 0 /* unused */
	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);
#endif /* 0 */


	/* Scan all objects in the grid */
	for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		bool is_art = FALSE;
		bool ignore = FALSE;
		bool plural = FALSE;
		bool do_kill = FALSE;

		cptr note_kill = NULL;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Extract the flags */
		object_flags(o_ptr, f);

		/* Get the "plural"-ness */
		if (o_ptr->number > 1) plural = TRUE;

		/* Check for artifact */
		if (artifact_p(o_ptr)) is_art = TRUE;

		/* Analyze the type */
		switch (typ)
		{
			/* Acid -- Lots of things */
			case GF_ACID:
			{
				if (hates_acid(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " melt!" : " melts!");
					if (of_has(f, OF_IGNORE_ACID)) ignore = TRUE;
				}
				break;
			}

			/* Elec -- Rings and Wands */
			case GF_ELEC:
			{
				if (hates_elec(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
					if (of_has(f, OF_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire -- Flammable objects */
			case GF_FIRE:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (of_has(f, OF_IGNORE_FIRE)) ignore = TRUE;
				}
				break;
			}

			/* Cold -- potions and flasks */
			case GF_COLD:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
					if (of_has(f, OF_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Elec */
			case GF_PLASMA:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (of_has(f, OF_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_elec(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
					if (of_has(f, OF_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Cold */
			case GF_METEOR:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (of_has(f, OF_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_cold(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " shatter!" : " shatters!");
					if (of_has(f, OF_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Hack -- break potions and such */
			case GF_ICE:
			case GF_SHARD:
			case GF_FORCE:
			case GF_SOUND:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
				}
				break;
			}

			/* Mana -- destroys everything */
			case GF_MANA:
			{
				do_kill = TRUE;
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
				break;
			}

			/* Holy Orb -- destroys cursed non-artifacts */
			case GF_HOLY_ORB:
			{
				if (cursed_p(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
				}
				break;
			}

			/* Unlock chests */
			case GF_KILL_TRAP:
			case GF_KILL_DOOR:
			{
				/* Chests are noticed only if trapped or locked */
				if (o_ptr->tval == TV_CHEST)
				{
					/* Disarm/Unlock traps */
					if (o_ptr->pval > 0)
					{
						/* Disarm or Unlock */
						o_ptr->pval = (0 - o_ptr->pval);

						/* Identify */
						object_notice_everything(o_ptr);

						/* Notice */
						if (o_ptr->marked && !squelch_hide_item(o_ptr))
						{
							msg_print("Click!");
							obvious = TRUE;
						}
					}
				}

				break;
			}
		}


		/* Attempt to destroy the object */
		if (do_kill)
		{
			/* Effect "observed" */
			if (o_ptr->marked && !squelch_hide_item(o_ptr))
			{
				obvious = TRUE;
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
			}

			/* Artifacts, and other objects, get to resist */
			if (is_art || ignore)
			{
				/* Observe the resist */
				if (o_ptr->marked && !squelch_hide_item(o_ptr))
				{
					msg_format("The %s %s unaffected!",
					           o_name, (plural ? "are" : "is"));
				}
			}

			/* Kill it */
			else
			{
				/* Describe if needed */
				if (o_ptr->marked && note_kill && !squelch_hide_item(o_ptr))
				{
					message_format(MSG_DESTROY, 0, "The %s%s", o_name, note_kill);
				}

				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Redraw */
				light_spot(y, x);
			}
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}



/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint1(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint1(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint1(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint1(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint1(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "obvious" happen.
 */
static bool project_m(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	int tmp;

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;

	cptr name;

	/* Is the monster "seen"? */
	bool seen = FALSE;

	/* Were the effects "irrelevant"? */
	bool skipped = FALSE;

	/* Did the monster die? */
	bool mon_died = FALSE;

	/* Polymorph setting (true or false) */
	int do_poly = 0;

	/* Teleport setting (max distance) */
	int do_dist = 0;

	/* Confusion setting (amount to confuse) */
	int do_conf = 0;

	/* Stunning setting (amount to stun) */
	int do_stun = 0;

	/* Sleep amount (amount to sleep) */
	int do_sleep = 0;

	/* Fear amount (amount to fear) */
	int do_fear = 0;


	/* Hold the monster name */
	char m_name[80];
	char m_poss[80];

	/* Assume no note */
	cptr note = NULL;

	/* Assume a default death */
	cptr note_dies = " dies.";


	/* Walls protect monsters */
	if (!cave_floor_bold(y,x)) return (FALSE);


	/* No monster here */
	if (!(cave_m_idx[y][x] > 0)) return (FALSE);

	/* Never affect projector */
	if (cave_m_idx[y][x] == who) return (FALSE);


	/* Obtain monster info */
	m_ptr = &mon_list[cave_m_idx[y][x]];
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];
	name = r_ptr->name;
	if (m_ptr->ml) seen = TRUE;


	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Get the monster name (BEFORE polymorphing) */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO2 | MDESC_POSS);


	/* Some monsters get "destroyed" */
	if (monster_is_unusual(r_ptr))
	{
		/* Special note at death */
		note_dies = " is destroyed.";
	}


	/* Analyze the damage type */
	switch (typ)
	{
		/* Magic Missile -- pure damage */
		case GF_MISSILE:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Acid */
		case GF_ACID:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_IM_ACID);
			if (rf_has(r_ptr->flags, RF_IM_ACID))
			{
				note = " resists a lot.";
				dam /= 9;
			}
			break;
		}

		/* Electricity */
		case GF_ELEC:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_IM_ELEC);
			if (rf_has(r_ptr->flags, RF_IM_ELEC))
			{
				note = " resists a lot.";
				dam /= 9;
			}
			break;
		}

		/* Fire damage */
		case GF_FIRE:
		{
			if (seen) obvious = TRUE;
			if (seen)
			{
				rf_on(l_ptr->flags, RF_IM_FIRE);
				rf_on(l_ptr->flags, RF_HURT_FIRE);
			}
			if (rf_has(r_ptr->flags, RF_IM_FIRE))
			{
				note = " resists a lot.";
				dam /= 9;
			}
			else if (rf_has(r_ptr->flags, RF_HURT_FIRE))
			{
				note = " catches on fire!";
				note_dies = " disintegrates!";
				dam *= 2;
			}
			break;
		}

		/* Cold */
		case GF_COLD:
		/* Ice -- Cold + Stun */
		case GF_ICE:
		{
			if (seen) obvious = TRUE;
			if (seen)
			{
				rf_on(l_ptr->flags, RF_IM_COLD);
				rf_on(l_ptr->flags, RF_HURT_COLD);
			}
			
			if(typ == GF_ICE)
			{
				do_stun = (randint1(15) + 1) / (r + 1);
			}
			
			if (rf_has(r_ptr->flags, RF_IM_COLD))
			{
				note = " resists a lot.";
				dam /= 9;
			}
			else if (rf_has(r_ptr->flags, RF_HURT_COLD))
			{
				note = " is badly frozen!";
				note_dies = " freezes and shatters!";
				dam *= 2;
			}
			break;
		}

		/* Poison */
		case GF_POIS:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_IM_POIS);
			if (rf_has(r_ptr->flags, RF_IM_POIS))
			{
				note = " resists a lot.";
				dam /= 9;
			}
			break;
		}

		/* Holy Orb -- hurts Evil */
		case GF_HOLY_ORB:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_EVIL);
			if (rf_has(r_ptr->flags, RF_EVIL))
			{
				dam *= 2;
				note = " is hit hard.";
			}
			break;
		}

		/* Arrow -- no defense XXX */
		case GF_ARROW:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Plasma */
		case GF_PLASMA:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_RES_PLAS);
			if (rf_has(r_ptr->flags, RF_RES_PLAS))
			{
				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Nether -- see above */
		case GF_NETHER:
		{
			if (seen) obvious = TRUE;

			/* Update the lore */
			if (seen)
			{
				/* Acquire knowledge of undead type and nether resistance */
				rf_on(l_ptr->flags, RF_UNDEAD);
				rf_on(l_ptr->flags, RF_RES_NETH);
				
				/* If it isn't undead, acquire extra knowledge */
				if (!rf_has(r_ptr->flags, RF_UNDEAD))
				{
					/* Learn this creature breathes nether if true */
					if (rsf_has(r_ptr->spell_flags, RSF_BR_NETH))
					{
						rsf_on(l_ptr->spell_flags, RSF_BR_NETH);
					}

					/* Otherwise learn about evil type */
					else
					{
						rf_on(l_ptr->flags, RF_EVIL);
					}
				}
			}

			if (rf_has(r_ptr->flags, RF_UNDEAD))
			{
				note = " is immune.";
				dam = 0;
			}
			else if (rf_has(r_ptr->flags, RF_RES_NETH) ||
			         rsf_has(r_ptr->spell_flags, RSF_BR_NETH))
			{
				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			else if (rf_has(r_ptr->flags, RF_EVIL))
			{
				dam /= 2;
				note = " resists somewhat.";
			}
			break;
		}

		/* Water damage */
		case GF_WATER:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_IM_WATER);
			if (rf_has(r_ptr->flags, RF_IM_WATER))
			{
				note = " is immune.";
				dam = 0;
			}
			break;
		}

		/* Chaos -- Chaos breathers resist */
		case GF_CHAOS:
		{
			if (seen) obvious = TRUE;

			do_poly = TRUE;
			do_conf = (5 + randint1(11) + r) / (r + 1);
			if (rsf_has(r_ptr->spell_flags, RSF_BR_CHAO))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_CHAO);

				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
				do_poly = FALSE;
			}
			break;
		}

		/* Shards -- Shard breathers resist */
		case GF_SHARD:
		{
			if (seen) obvious = TRUE;
			if (rsf_has(r_ptr->spell_flags, RSF_BR_SHAR))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_SHAR);

				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Sound -- Sound breathers resist */
		case GF_SOUND:
		{
			if (seen) obvious = TRUE;
			
			do_stun = (10 + randint1(15) + r) / (r + 1);
			if (rsf_has(r_ptr->spell_flags, RSF_BR_SOUN))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_SOUN);

				note = " resists.";
				dam *= 2; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Confusion */
		case GF_CONFUSION:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_NO_CONF);

			do_conf = (10 + randint1(15) + r) / (r + 1);
			if (rsf_has(r_ptr->spell_flags, RSF_BR_CONF))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_CONF);

				note = " resists.";
				dam *= 2; dam /= (randint1(6)+6);
			}
			else if (rf_has(r_ptr->flags, RF_NO_CONF))
			{
				note = " resists somewhat.";
				dam /= 2;
			}
			break;
		}

		/* Disenchantment */
		case GF_DISENCHANT:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_RES_DISE);
			if (rf_has(r_ptr->flags, RF_RES_DISE))
			{
				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Nexus */
		case GF_NEXUS:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_RES_NEXUS);
			if (rf_has(r_ptr->flags, RF_RES_NEXUS))
			{
				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Force */
		case GF_FORCE:
		{
			if (seen) obvious = TRUE;
			
			do_stun = (randint1(15) + r) / (r + 1);
			if (rsf_has(r_ptr->spell_flags, RSF_BR_WALL))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_WALL);

				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Inertia -- breathers resist */
		case GF_INERTIA:
		{
			if (seen) obvious = TRUE;
			if (rsf_has(r_ptr->spell_flags, RSF_BR_INER))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_INER);

				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Time -- breathers resist */
		case GF_TIME:
		{
			if (seen) obvious = TRUE;
			if (rsf_has(r_ptr->spell_flags, RSF_BR_TIME))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_TIME);

				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Gravity -- breathers resist */
		case GF_GRAVITY:
		{
			if (seen) obvious = TRUE;

			/* Higher level monsters can resist the teleportation better */
			if (randint1(127) > r_ptr->level)
				do_dist = 10;

			if (rsf_has(r_ptr->spell_flags, RSF_BR_GRAV))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_GRAV);

				note = " resists.";
				dam *= 3; dam /= (randint1(6)+6);
				do_dist = 0;
			}
			break;
		}

		/* Pure damage */
		case GF_MANA:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Meteor -- powerful magic missile */
		case GF_METEOR:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Drain Life */
		case GF_OLD_DRAIN:
		{
			if (seen) obvious = TRUE;
			if (seen)
			{
				rf_on(l_ptr->flags, RF_UNDEAD);
				rf_on(l_ptr->flags, RF_DEMON);
			}
			if (monster_is_unusual(r_ptr))
			{
				note = " is unaffected!";
				obvious = FALSE;
				dam = 0;
			}

			break;
		}

		/* Polymorph monster (Use "dam" as "power") */
		case GF_OLD_POLY:
		{
			/* Polymorph later */
			do_poly = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Clone monsters (Ignore "dam") */
		case GF_OLD_CLONE:
		{
			if (seen) obvious = TRUE;

			/* Heal fully */
			m_ptr->hp = m_ptr->maxhp;

			/* Speed up */
			if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;

			/* Attempt to clone. */
			if (multiply_monster(cave_m_idx[y][x]))
			{
				note = " spawns!";
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Heal Monster (use "dam" as amount of healing) */
		case GF_OLD_HEAL:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			wake_monster(m_ptr);

			/* Heal */
			m_ptr->hp += dam;

			/* No overflow */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (p_ptr->health_who == cave_m_idx[y][x]) p_ptr->redraw |= (PR_HEALTH);

			/* Message */
			note = " looks healthier.";

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Speed Monster (Ignore "dam") */
		case GF_OLD_SPEED:
		{
			if (seen) obvious = TRUE;

			/* Speed up */
			if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;
			note = " starts moving faster.";

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Slow Monster (Use "dam" as "power") */
		case GF_OLD_SLOW:
		{
			if (seen) obvious = TRUE;

			/* Powerful monsters can resist */
			if (rf_has(r_ptr->flags, RF_UNIQUE) ||
			    (r_ptr->level > randint1((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* Normal monsters slow down */
			else
			{
				if (m_ptr->mspeed > 60) m_ptr->mspeed -= 10;
				note = " starts moving slower.";
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Sleep (Use "dam" as "power") */
		case GF_OLD_SLEEP:
		{
			/* Go to sleep later */
			do_sleep = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Confusion (Use "dam" as "power") */
		case GF_OLD_CONF:
		{
			/* Get confused later */
			do_conf = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Light, but only hurts susceptible creatures */
		case GF_LIGHT_WEAK:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_HURT_LIGHT);

			/* Hurt by light */
			if (rf_has(r_ptr->flags, RF_HURT_LIGHT))
			{
				/* Special effect */
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
			}

			/* Normally no damage */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Light -- opposite of Dark */
		case GF_LIGHT:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_HURT_LIGHT);

			if (rsf_has(r_ptr->spell_flags, RSF_BR_LIGHT))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_LIGHT);

				note = " resists.";
				dam *= 2; dam /= (randint1(6)+6);
			}
			else if (rf_has(r_ptr->flags, RF_HURT_LIGHT))
			{
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
				dam *= 2;
			}
			break;
		}


		/* Dark -- opposite of Light */
		case GF_DARK:
		{
			if (seen) obvious = TRUE;
			if (rsf_has(r_ptr->spell_flags, RSF_BR_DARK))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_DARK);

				note = " resists.";
				dam *= 2; dam /= (randint1(6)+6);
			}
			break;
		}


		/* Stone to Mud */
		case GF_KILL_WALL:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_HURT_ROCK);

			/* Hurt by rock remover */
			if (rf_has(r_ptr->flags, RF_HURT_ROCK))
			{
				/* Cute little message */
				note = " loses some skin!";
				note_dies = " dissolves!";
			}

			/* Usually, ignore the effects */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Teleport undead (Use "dam" as "power") */
		case GF_AWAY_UNDEAD:
		{
			if (seen) rf_on(l_ptr->flags, RF_UNDEAD);

			/* Only affect undead */
			if (rf_has(r_ptr->flags, RF_UNDEAD))
			{
				if (seen) obvious = TRUE;
				do_dist = dam;
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport evil (Use "dam" as "power") */
		case GF_AWAY_EVIL:
		{
			if (seen) rf_on(l_ptr->flags, RF_EVIL);

			/* Only affect evil */
			if (rf_has(r_ptr->flags, RF_EVIL))
			{
				if (seen) obvious = TRUE;
				do_dist = dam;
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport monster (Use "dam" as "power") */
		case GF_AWAY_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Prepare to teleport */
			do_dist = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn undead (Use "dam" as "power") */
		case GF_TURN_UNDEAD:
		{
			/* Only affect undead */
			if (rf_has(r_ptr->flags, RF_UNDEAD))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = dam;
			}
			else
			{
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn evil (Use "dam" as "power") */
		case GF_TURN_EVIL:
		{
			/* Only affect evil */
			if (rf_has(r_ptr->flags, RF_EVIL))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = dam;
			}
			else
			{
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn monster (Use "dam" as "power") */
		case GF_TURN_ALL:
		{
			/* Get frightened later */
			do_fear = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Dispel undead */
		case GF_DISP_UNDEAD:
		{
			if (seen) rf_on(l_ptr->flags, RF_UNDEAD);

			/* Only affect undead */
			if (rf_has(r_ptr->flags, RF_UNDEAD))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel evil */
		case GF_DISP_EVIL:
		{
			if (seen) rf_on(l_ptr->flags, RF_EVIL);

			/* Only affect evil */
			if (rf_has(r_ptr->flags, RF_EVIL))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel monster */
		case GF_DISP_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Message */
			note = " shudders.";
			note_dies = " dissolves!";

			break;
		}


		/* Default */
		default:
		{
			/* Irrelevant */
			skipped = TRUE;

			/* No damage */
			dam = 0;

			break;
		}
	}


	/* Absolutely no effect */
	if (skipped) return (FALSE);


	/* "Unique" monsters can only be "killed" by the player */
	if (rf_has(r_ptr->flags, RF_UNIQUE))
	{
		/* Uniques may only be killed by the player */
		if ((who > 0) && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}


	/* Check for death */
	if (dam > m_ptr->hp)
	{
		/* Extract method of death */
		note = note_dies;
	}

	/* Handle polymorph */
	else if (do_poly)
	{
		/* Uniques cannot be polymorphed */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			/* Don't print a message for chaos side effects */
			if (obvious && typ == GF_OLD_POLY) note = " is unaffected!";
		}
		
		else
		{
			if (seen) obvious = TRUE;
			
			/* Saving throws are allowed */
			if (r_ptr->level > randint1(90) ||
			    (typ == GF_OLD_POLY && r_ptr->level > randint1(MAX(1, do_poly - 10)) + 10))
			{
				if (typ == GF_OLD_POLY) note = format(" maintains %s shape.", m_poss);
			}

			/* Failed to save */
			else
			{
				/* Pick a "new" monster race */
				tmp = poly_r_idx(m_ptr->r_idx);

				/* Handle polymorph */
				if (tmp != m_ptr->r_idx)
				{
					/* Monster polymorphs */
					note = " changes!";

					/* Turn off the damage */
					dam = 0;

					/* "Kill" the "old" monster */
					delete_monster_idx(cave_m_idx[y][x]);

					/* Create a new monster (no groups) */
					(void)place_monster_aux(y, x, tmp, FALSE, FALSE);

					/* Hack -- Assume success XXX XXX XXX */

					/* Hack -- Get new monster */
					m_ptr = &mon_list[cave_m_idx[y][x]];

					/* Hack -- Get new race */
					r_ptr = &r_info[m_ptr->r_idx];
				}
			}
		}
	}

	/* Handle "teleport" */
	else if (do_dist)
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Message */
		note = " disappears!";

		/* Teleport */
		teleport_away(cave_m_idx[y][x], do_dist);

		/* Hack -- get new location */
		y = m_ptr->fy;
		x = m_ptr->fx;
	}

	/* Handle stunning */
	else if (do_stun)
	{
		/* Monsters can be immune, as are sound and force breathers */
		if (rf_has(r_ptr->flags, RF_NO_STUN) ||
	       flags_test(r_ptr->spell_flags, RSF_SIZE, RSF_BR_SOUN, RSF_BR_WALL, FLAG_END))
	   {
			/* Immune to stunning */
			if (seen && obvious) rf_on(l_ptr->flags, RF_NO_STUN);
		}
	   else
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_NO_STUN);

			tmp = do_stun;

			/* Get stunned */
			if (m_ptr->stunned)
			{
				note = " is more dazed.";
				tmp /= 2;
			}
			else
			{
				note = " is dazed.";
			}

			m_ptr->stunned += tmp;

			if (m_ptr->stunned > 200) m_ptr->stunned = 200;
		}
	}

	/* Handle confusion */
	else if (do_conf)
	{
		/* Uniques, confusion breathers, and chaos breathers are also immune */
		if (rf_has(r_ptr->flags, RF_NO_CONF) ||
			 rf_has(r_ptr->flags, RF_UNIQUE) ||
		    flags_test(r_ptr->spell_flags, RSF_SIZE, RSF_BR_CONF, RSF_BR_CHAO, FLAG_END))
		{
			/* Immune. Learn about confusion and get a message only if it's already obvious */
			if (seen && obvious)
			{
				rf_on(l_ptr->flags, RF_NO_CONF);

				/* Don't print a message for chaos and confusion damage side effects */
				if (typ == GF_OLD_CONF) note = " is unaffected!";
			}
		}
		else
		{
			/* Monster is not immune to confusion */
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_NO_CONF);

			/* Normal confusion has a saving throw. */
			if (typ == GF_OLD_CONF && r_ptr->level > randint1(MAX(1, do_conf - 10)) + 10)
			{
				note = " looks briefly puzzled.";
			}

			/* Failed to save, or cannot save (vs chaos/confusion damage) */
			else
			{
				tmp = damroll(3, (do_conf / 2)) + 1;

				if (m_ptr->confused)
				{
					note = " looks more confused.";
					tmp /= 2;
				}
				else
				{
					note = " looks confused.";
				}

				m_ptr->confused += tmp;

				if (m_ptr->confused > 200) m_ptr->confused = 200;
			}
		}
	}


	/* Handle fear */
	if (do_fear)
	{
		/* Normal fear - monsters can be immune, uniques are immune */
		if (typ == GF_TURN_ALL &&
		    (rf_has(r_ptr->flags, RF_NO_FEAR) ||
		     rf_has(r_ptr->flags, RF_UNIQUE)))
		{
			/* Immune. Learn about fear and get a message only if it's already obvious */
			if (seen && obvious)
			{
				rf_on(l_ptr->flags, RF_NO_FEAR);
				note = " is unaffected!";
			}
		}
		else
		{
			/* Monster is not immune */
			if (seen) obvious = TRUE;

			/* Learn about fear (NO_FEAR doesn't apply to turn evil/undead) */
			if (seen && typ == GF_TURN_ALL) rf_on(l_ptr->flags, RF_NO_FEAR);

			/* All fear has a saving throw. */
			if (r_ptr->level > randint1(MAX(1, do_fear - 10)) + 10)
			{
				note = format(" stands %s ground.", m_poss);
			}

			/* Failed to save */
			else
			{
				m_ptr->monfear += (damroll(3, (do_fear / 2)) + 1);

				if (m_ptr->monfear > 200) m_ptr->monfear = 200;
			}
		}
	}


	/* If another monster did the damage, hurt the monster by hand */
	if (who > 0)
	{
		/* Redraw (later) if needed */
		if (p_ptr->health_who == cave_m_idx[y][x]) p_ptr->redraw |= (PR_HEALTH);

		/* Wake the monster up */
		wake_monster(m_ptr);

		/* Hurt the monster */
		m_ptr->hp -= dam;

		/* Dead monster */
		if (m_ptr->hp < 0)
		{
			/* Generate treasure, etc */
			monster_death(cave_m_idx[y][x]);

			/* Delete the monster */
			delete_monster_idx(cave_m_idx[y][x]);

			/* Give detailed messages if destroyed */
			if (note) msg_format("%^s%s", m_name, note);

			mon_died = TRUE;
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(cave_m_idx[y][x], dam);
		}
	}

	/* If the player did it, give him experience, check fear */
	else
	{
		bool fear = FALSE;

		/* Hurt the monster, check for fear and death */
		if (mon_take_hit(cave_m_idx[y][x], dam, &fear, note_dies))
		{
			/* Dead monster */
			mon_died = TRUE;
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(cave_m_idx[y][x], dam);

			/* Take note */
			if (seen && (fear || m_ptr->monfear))
			{
				/* Message */
				message_format(MSG_FLEE, m_ptr->r_idx,
				               "%^s flees in terror!", m_name);
			}
		}
	}
	
	/* Handle sleep */
	if(!mon_died && do_sleep)
	{
		/* Monsters can be immune, and so are uniques */
		if (rf_has(r_ptr->flags, RF_NO_SLEEP) ||
		    rf_has(r_ptr->flags, RF_UNIQUE))
		{
			/* Immune.  Learn about sleep and get a message only if it's already obvious */
			if (seen && obvious)
			{
				rf_on(l_ptr->flags, RF_NO_SLEEP);
				note = " is unaffected!";
			}
		}
		else
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_NO_SLEEP);

			/* Monsters may still make a saving throw */
			if (r_ptr->level > randint1(MAX(1, do_sleep - 10)) + 10)
			{
				note = " looks drowsy for a moment.";
			}
			
			/* Failed to save */
			else
			{
				note = " falls asleep!";
				m_ptr->csleep = 500;
			}
		}

		/* Print the message */
		if (seen && note) msg_format("%^s%s", m_name, note);
	}


	/* Verify this code XXX XXX XXX */

	/* Update the monster */
	update_mon(cave_m_idx[y][x], FALSE);

	/* Redraw the monster grid */
	light_spot(y, x);


	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->redraw |= (PR_MONSTER);
	}


	/* Track it */
	project_m_n++;
	project_m_x = x;
	project_m_y = y;


	/* Return "Anything seen?" */
	return (obvious);
}






/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * We return "TRUE" if any "obvious" effects were observed.
 *
 * Actually, for historical reasons, we just assume that the effects were
 * obvious.  XXX XXX XXX
 */
static bool project_p(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	int k = 0;

	/* Player blind-ness */
	bool blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Source monster */
	monster_type *m_ptr;

	/* Monster name (for attacks) */
	char m_name[80];

	/* Monster name (for damage) */
	char killer[80];

	/* Hack -- messages */
	cptr act = NULL;


	/* No player here */
	if (!(cave_m_idx[y][x] < 0)) return (FALSE);

	/* Never affect projector */
	if (cave_m_idx[y][x] == who) return (FALSE);


	/* Limit maximum damage XXX XXX XXX */
	if (dam > 1600) dam = 1600;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Get the source monster */
	m_ptr = &mon_list[who];

	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Get the monster's real name */
	monster_desc(killer, sizeof(killer), m_ptr, MDESC_SHOW | MDESC_IND2);


	/* Analyze the damage */
	switch (typ)
	{
		/* Standard damage -- hurts inventory too */
		case GF_ACID:
		{
			if (blind) msg_print("You are hit by acid!");
			acid_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_FIRE:
		{
			if (blind) msg_print("You are hit by fire!");
			fire_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_COLD:
		{
			if (blind) msg_print("You are hit by cold!");
			cold_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_ELEC:
		{
			if (blind) msg_print("You are hit by lightning!");
			elec_dam(dam, killer);
			break;
		}

		/* Standard damage -- also poisons player */
		case GF_POIS:
		{
			if (blind) msg_print("You are hit by poison!");
			if (p_ptr->state.resist_pois)
			{
				dam = RES_POIS_ADJ(dam, NOT_USED);
				wieldeds_notice_flag(OF_RES_POIS);
			}

			if (p_ptr->timed[TMD_OPP_POIS])
				dam = RES_POIS_ADJ(dam, NOT_USED);

			take_hit(dam, killer);
			if (!(p_ptr->state.resist_pois || p_ptr->timed[TMD_OPP_POIS]))
			{
				(void)inc_timed(TMD_POISONED, randint0(dam) + 10, TRUE);
			}
			break;
		}

		/* Standard damage */
		case GF_MISSILE:
		{
			if (blind) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Holy Orb -- Player only takes partial damage */
		case GF_HOLY_ORB:
		{
			if (blind) msg_print("You are hit by something!");
			dam /= 2;
			take_hit(dam, killer);
			break;
		}

		/* Arrow -- no dodging XXX */
		case GF_ARROW:
		{
			if (blind) msg_print("You are hit by something sharp!");
			take_hit(dam, killer);
			break;
		}

		/* Plasma -- No resist XXX */
		case GF_PLASMA:
		{
			if (blind) msg_print("You are hit by something!");
			take_hit(dam, killer);
			if (!p_ptr->state.resist_sound)
			{
				int k = (randint1((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
				(void)inc_timed(TMD_STUN, k, TRUE);
			}
			else
			{
				wieldeds_notice_flag(OF_RES_SOUND);
			}
			break;
		}

		/* Nether -- drain experience */
		case GF_NETHER:
		{
			if (blind) msg_print("You are hit by something strange!");
			if (p_ptr->state.resist_nethr)
			{
				dam = RES_NETH_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_NETHR);
			}
			else
			{
				if (p_ptr->state.hold_life && (randint0(100) < 75))
				{
					msg_print("You keep hold of your life force!");
					wieldeds_notice_flag(OF_HOLD_LIFE);
				}
				else
				{
					s32b d = 200 + (p_ptr->exp / 100) * MON_DRAIN_LIFE;

					if (p_ptr->state.hold_life)
					{
						msg_print("You feel your life slipping away!");
						lose_exp(d / 10);
						wieldeds_notice_flag(OF_HOLD_LIFE);
					}
					else
					{
						msg_print("You feel your life draining away!");
						lose_exp(d);
					}
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Water -- stun/confuse */
		case GF_WATER:
		{
			if (blind) msg_print("You are hit by something!");
			if (!p_ptr->state.resist_sound)
				(void)inc_timed(TMD_STUN, randint1(40), TRUE);
			else
				wieldeds_notice_flag(OF_RES_SOUND);

			if (!p_ptr->state.resist_confu)
				(void)inc_timed(TMD_CONFUSED, randint1(5) + 5, TRUE);
			else
				wieldeds_notice_flag(OF_RES_CONFU);

			take_hit(dam, killer);
			break;
		}

		/* Chaos -- many effects */
		case GF_CHAOS:
		{
			if (blind) msg_print("You are hit by something strange!");
			if (p_ptr->state.resist_chaos)
			{
				dam = RES_CHAO_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_CHAOS);
			}
			if (!p_ptr->state.resist_confu && !p_ptr->state.resist_chaos)
			{
				(void)inc_timed(TMD_CONFUSED, randint0(20) + 10, TRUE);
			}

			if (!p_ptr->state.resist_chaos)
				(void)inc_timed(TMD_IMAGE, randint1(10), TRUE);
			else
				wieldeds_notice_flag(OF_RES_CHAOS);

			if (!p_ptr->state.resist_nethr && !p_ptr->state.resist_chaos)
			{
				if (p_ptr->state.hold_life && (randint0(100) < 75))
				{
					msg_print("You keep hold of your life force!");
					wieldeds_notice_flag(OF_HOLD_LIFE);
				}
				else
				{
					s32b d = 5000 + (p_ptr->exp / 100) * MON_DRAIN_LIFE;

					if (p_ptr->state.hold_life)
					{
						msg_print("You feel your life slipping away!");
						lose_exp(d / 10);
						wieldeds_notice_flag(OF_HOLD_LIFE);
					}
					else
					{
						msg_print("You feel your life draining away!");
						lose_exp(d);
					}
				}
			}
			else
			{
				wieldeds_notice_flag(OF_RES_NETHR);
				wieldeds_notice_flag(OF_RES_CHAOS);
			}

			take_hit(dam, killer);
			break;
		}

		/* Shards -- mostly cutting */
		case GF_SHARD:
		{
			if (blind) msg_print("You are hit by something sharp!");
			if (p_ptr->state.resist_shard)
			{
				dam = RES_SHAR_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_SHARD);
			}
			else
			{
				(void)inc_timed(TMD_CUT, dam, TRUE);
			}
			take_hit(dam, killer);
			break;
		}

		/* Sound -- mostly stunning */
		case GF_SOUND:
		{
			if (blind) msg_print("You are hit by something!");
			if (p_ptr->state.resist_sound)
			{
				dam = RES_SOUN_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_SOUND);
			}
			else
			{
				int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)inc_timed(TMD_STUN, k, TRUE);
			}
			take_hit(dam, killer);
			break;
		}

		/* Pure confusion */
		case GF_CONFUSION:
		{
			if (blind) msg_print("You are hit by something!");
			if (p_ptr->state.resist_confu)
			{
				dam = RES_CONF_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_CONFU);
			}
			else
			{
				(void)inc_timed(TMD_CONFUSED, randint1(20) + 10, TRUE);
			}
			take_hit(dam, killer);
			break;
		}

		/* Disenchantment -- see above */
		case GF_DISENCHANT:
		{
			if (blind) msg_print("You are hit by something strange!");
			if (p_ptr->state.resist_disen)
			{
				dam = RES_DISE_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_DISEN);
			}
			else
			{
				(void)apply_disenchant(0);
			}
			take_hit(dam, killer);
			break;
		}

		/* Nexus -- see above */
		case GF_NEXUS:
		{
			if (blind) msg_print("You are hit by something strange!");
			if (p_ptr->state.resist_nexus)
			{
				dam = RES_NEXU_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_NEXUS);
			}
			else
			{
				apply_nexus(m_ptr);
			}
			take_hit(dam, killer);
			break;
		}

		/* Force -- mostly stun */
		case GF_FORCE:
		{
			if (blind) msg_print("You are hit by something!");
			if (!p_ptr->state.resist_sound)
				(void)inc_timed(TMD_STUN, randint1(20), TRUE);
			else
				wieldeds_notice_flag(OF_RES_SOUND);

			take_hit(dam, killer);
			break;
		}

		/* Inertia -- slowness */
		case GF_INERTIA:
		{
			if (blind) msg_print("You are hit by something strange!");
			(void)inc_timed(TMD_SLOW, randint0(4) + 4, TRUE);
			take_hit(dam, killer);
			break;
		}

		/* Light -- blinding */
		case GF_LIGHT:
		{
			if (blind) msg_print("You are hit by something!");
			if (p_ptr->state.resist_light)
			{
				dam = RES_LIGHT_ADJ(dam, RANDOMISE);
				wieldeds_notice_flag(OF_RES_LIGHT);
			}
			else if (!blind && !p_ptr->state.resist_blind)
			{
				(void)inc_timed(TMD_BLIND, randint1(5) + 2, TRUE);
			}
			else if (p_ptr->state.resist_blind)
			{
				wieldeds_notice_flag(OF_RES_BLIND);
			}
			take_hit(dam, killer);
			break;
		}

		/* Dark -- blinding */
		case GF_DARK:
		{
			if (blind) msg_print("You are hit by something!");
			if (p_ptr->state.resist_dark)
			{
				dam = RES_DARK_ADJ(dam, RANDOMISE);
			}
			else if (!blind && !p_ptr->state.resist_blind)
			{
				(void)inc_timed(TMD_BLIND, randint1(5) + 2, TRUE);
			}
			wieldeds_notice_flag(OF_RES_DARK);
			wieldeds_notice_flag(OF_RES_BLIND);

			take_hit(dam, killer);
			break;
		}

		/* Time -- bolt fewer effects XXX */
		case GF_TIME:
		{
			if (blind) msg_print("You are hit by something strange!");

			switch (randint1(10))
			{
				case 1: case 2: case 3: case 4: case 5:
				{
					msg_print("You feel life has clocked back.");
					lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
					break;
				}

				case 6: case 7: case 8: case 9:
				{
					switch (randint1(6))
					{
						case 1: k = A_STR; act = "strong"; break;
						case 2: k = A_INT; act = "bright"; break;
						case 3: k = A_WIS; act = "wise"; break;
						case 4: k = A_DEX; act = "agile"; break;
						case 5: k = A_CON; act = "hale"; break;
						case 6: k = A_CHR; act = "beautiful"; break;
					}

					msg_format("You're not as %s as you used to be...", act);

					p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
					if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
					p_ptr->update |= (PU_BONUS);
					break;
				}

				case 10:
				{
					msg_print("You're not as powerful as you used to be...");

					for (k = 0; k < A_MAX; k++)
					{
						p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
						if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
					}
					p_ptr->update |= (PU_BONUS);
					break;
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Gravity -- stun plus slowness plus teleport */
		case GF_GRAVITY:
		{
			if (blind) msg_print("You are hit by something strange!");
			msg_print("Gravity warps around you.");

			/* Higher level players can resist the teleportation better */
			if (randint1(127) > p_ptr->lev)
				teleport_player(5);

			(void)inc_timed(TMD_SLOW, randint0(4) + 4, TRUE);
			if (!p_ptr->state.resist_sound)
			{
				int k = (randint1((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)inc_timed(TMD_STUN, k, TRUE);
			}
			else
			{
				wieldeds_notice_flag(OF_RES_SOUND);
			}
			take_hit(dam, killer);
			break;
		}

		/* Pure damage */
		case GF_MANA:
		{
			if (blind) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Pure damage */
		case GF_METEOR:
		{
			if (blind) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Ice -- cold plus stun plus cuts */
		case GF_ICE:
		{
			if (blind) msg_print("You are hit by something sharp!");
			cold_dam(dam, killer);

			if (!p_ptr->state.resist_shard)
				(void)inc_timed(TMD_CUT, damroll(5, 8), TRUE);
			else
				wieldeds_notice_flag(OF_RES_SHARD);

			if (!p_ptr->state.resist_sound)
				(void)inc_timed(TMD_STUN, randint1(15), TRUE);
			else
				wieldeds_notice_flag(OF_RES_SOUND);

			break;
		}


		/* Default */
		default:
		{
			/* No damage */
			dam = 0;

			break;
		}
	}


	/* Disturb */
	disturb(1, 0);


	/* Return "Anything seen?" */
	return (obvious);
}





/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (negative for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away.  The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced.  This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that all projections now "explode" at their final destination, even
 * if they were being projected at a more distant destination.  This means
 * that "ball" spells will *always* explode.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the "illumination" of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool project(int who, int rad, int y, int x, int dam, int typ, int flg)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, t, dist;

	int y1, x1;
	int y2, x2;

	int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	/* Assume the player sees nothing */
	bool notice = FALSE;

	/* Assume the player has seen nothing */
	bool visual = FALSE;

	/* Assume the player has seen no blast grids */
	bool drawn = FALSE;

	/* Is the player blind? */
	bool blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Number of grids in the "path" */
	int path_n = 0;

	/* Actual grids in the "path" */
	u16b path_g[512];

	/* Number of grids in the "blast area" (including the "beam" path) */
	int grids = 0;

	/* Coordinates of the affected grids */
	byte gx[256], gy[256];

	/* Encoded "radius" info (see above) */
	byte gm[16];


	/* Hack -- Jump to target */
	if (flg & (PROJECT_JUMP))
	{
		x1 = x;
		y1 = y;

		/* Clear the flag */
		flg &= ~(PROJECT_JUMP);
	}

	/* Start at player */
	else if (who < 0)
	{
		x1 = px;
		y1 = py;
	}

	/* Start at monster */
	else if (who > 0)
	{
		x1 = mon_list[who].fx;
		y1 = mon_list[who].fy;
	}

	/* Oops */
	else
	{
		x1 = x;
		y1 = y;
	}


	/* Default "destination" */
	y2 = y;
	x2 = x;


	/* Hack -- verify stuff */
	if (flg & (PROJECT_THRU))
	{
		if ((x1 == x2) && (y1 == y2))
		{
			flg &= ~(PROJECT_THRU);
		}
	}


	/* Hack -- Assume there will be no blast (max radius 16) */
	for (dist = 0; dist < 16; dist++) gm[dist] = 0;


	/* Initial grid */
	y = y1;
	x = x1;

	/* Collect beam grids */
	if (flg & (PROJECT_BEAM))
	{
		gy[grids] = y;
		gx[grids] = x;
		grids++;
	}


	/* Calculate the projection path */
	path_n = project_path(path_g, MAX_RANGE, y1, x1, y2, x2, flg);


	/* Hack -- Handle stuff */
	handle_stuff();

	/* Project along the path */
	for (i = 0; i < path_n; ++i)
	{
		int oy = y;
		int ox = x;

		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		/* Hack -- Balls explode before reaching walls */
		if (!cave_floor_bold(ny, nx) && (rad > 0)) break;

		/* Advance */
		y = ny;
		x = nx;

		/* Collect beam grids */
		if (flg & (PROJECT_BEAM))
		{
			gy[grids] = y;
			gx[grids] = x;
			grids++;
		}

		/* Only do visuals if requested */
		if (!blind && !(flg & (PROJECT_HIDE)))
		{
			/* Only do visuals if the player can "see" the bolt */
			if (player_has_los_bold(y, x))
			{
				u16b p;

				byte a;
				char c;

				/* Obtain the bolt pict */
				p = bolt_pict(oy, ox, y, x, typ);

				/* Extract attr/char */
				a = PICT_A(p);
				c = PICT_C(p);

				/* Visual effects */
				print_rel(c, a, y, x);
				move_cursor_relative(y, x);

				Term_fresh();
				if (p_ptr->redraw) redraw_stuff();

				Term_xtra(TERM_XTRA_DELAY, msec);

				light_spot(y, x);

				Term_fresh();
				if (p_ptr->redraw) redraw_stuff();

				/* Display "beam" grids */
				if (flg & (PROJECT_BEAM))
				{
					/* Obtain the explosion pict */
					p = bolt_pict(y, x, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects */
					print_rel(c, a, y, x);
				}

				/* Hack -- Activate delay */
				visual = TRUE;
			}

			/* Hack -- delay anyway for consistency */
			else if (visual)
			{
				/* Delay for consistency */
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}
	}


	/* Save the "blast epicenter" */
	y2 = y;
	x2 = x;

	/* Start the "explosion" */
	gm[0] = 0;

	/* Hack -- make sure beams get to "explode" */
	gm[1] = grids;

	/* Explode */
	/* Hack -- remove final beam grid */
	if (flg & (PROJECT_BEAM))
	{
		grids--;
	}

	/* Determine the blast area, work from the inside out */
	for (dist = 0; dist <= rad; dist++)
	{
		/* Scan the maximal blast area of radius "dist" */
		for (y = y2 - dist; y <= y2 + dist; y++)
		{
			for (x = x2 - dist; x <= x2 + dist; x++)
			{
				/* Ignore "illegal" locations */
				if (!in_bounds(y, x)) continue;

				/* Enforce a "circular" explosion */
				if (distance(y2, x2, y, x) != dist) continue;

				/* Ball explosions are stopped by walls */
				if (!los(y2, x2, y, x)) continue;

				/* Save this grid */
				gy[grids] = y;
				gx[grids] = x;
				grids++;
			}
		}

		/* Encode some more "radius" info */
		gm[dist+1] = grids;
	}


	/* Speed -- ignore "non-explosions" */
	if (!grids) return (FALSE);


	/* Display the "blast area" if requested */
	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		/* Then do the "blast", from inside out */
		for (t = 0; t <= rad; t++)
		{
			/* Dump everything with this radius */
			for (i = gm[t]; i < gm[t+1]; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Only do visuals if the player can "see" the blast */
				if (player_has_los_bold(y, x))
				{
					u16b p;

					byte a;
					char c;

					drawn = TRUE;

					/* Obtain the explosion pict */
					p = bolt_pict(y, x, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects -- Display */
					print_rel(c, a, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush each "radius" separately */
			Term_fresh();

			/* Flush */
			if (p_ptr->redraw) redraw_stuff();

			/* Delay (efficiently) */
			if (visual || drawn)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		/* Flush the erasing */
		if (drawn)
		{
			/* Erase the explosion drawn above */
			for (i = 0; i < grids; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Hack -- Erase if needed */
				if (player_has_los_bold(y, x))
				{
					light_spot(y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush the explosion */
			Term_fresh();

			/* Flush */
			if (p_ptr->redraw) redraw_stuff();
		}
	}


	/* Check features */
	if (flg & (PROJECT_GRID))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for features */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the feature in that grid */
			if (project_f(who, dist, y, x, dam, typ, FALSE)) notice = TRUE;
		}
	}


	/* Update stuff if needed */
	if (p_ptr->update) update_stuff();


	/* Check objects */
	if (flg & (PROJECT_ITEM))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for objects */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the object in the grid */
			if (project_o(who, dist, y, x, dam, typ, FALSE)) notice = TRUE;
		}
	}


	/* Check monsters */
	if (flg & (PROJECT_KILL))
	{
		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for monsters */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the monster in the grid */
			if (project_m(who, dist, y, x, dam, typ, (flg & (PROJECT_AWARE) ? TRUE : FALSE))) notice = TRUE;
		}

		/* Player affected one monster (without "jumping") */
		if ((who < 0) && (project_m_n == 1) && !(flg & (PROJECT_JUMP)))
		{
			/* Location */
			x = project_m_x;
			y = project_m_y;

			/* Track if possible */
			if (cave_m_idx[y][x] > 0)
			{
				monster_type *m_ptr = &mon_list[cave_m_idx[y][x]];

				/* Hack -- auto-recall */
				if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

				/* Hack - auto-track */
				if (m_ptr->ml) health_track(cave_m_idx[y][x]);
			}
		}
	}


	/* Check player */
	if (flg & (PROJECT_KILL))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for player */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the player (assume obvious) */
			if (project_p(who, dist, y, x, dam, typ, TRUE))
			{
				notice = TRUE;

				/* Only affect the player once */
				break;
			}
		}
	}


	/* Return "something was noticed" */
	return (notice);
}
