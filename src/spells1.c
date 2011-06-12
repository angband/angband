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
#include "object/object.h"
#include "monster/mon-spell.h"
#include "squelch.h"
#include "trap.h"
#include "spells.h"

/**
 * Details of the different projectable attack types in the game.
 * See src/spells.h for structure
 */
const struct gf_type gf_table[] =
{
        #define GF(a, b, c, d, e, f, g, h, i, j, k, l, m) \
			{ GF_##a, b, c, d, e, f, g, h, i, j, k, l, m },
                #define RV(b, x, y, m) {b, x, y, m}
        #include "list-gf-types.h"
        #undef GF
                #undef RV
};


/**
 * Check for resistance to a GF_ attack type. Return codes:
 * -1 = vulnerability
 * 0 = no resistance (or resistance plus vulnerability)
 * 1 = single resistance or opposition (or double resist plus vulnerability)
 * 2 = double resistance (including opposition)
 * 3 = total immunity
 *
 * \param type is the attack type we are trying to resist
 * \param flags is the set of flags we're checking
 * \param real is whether this is a real attack
 */
int check_for_resist(struct player *p, int type, bitflag *flags, bool real)
{
	const struct gf_type *gf_ptr = &gf_table[type];
	int result = 0;

	if (gf_ptr->vuln && of_has(flags, gf_ptr->vuln))
		result--;

	/* If it's not a real attack, we don't check timed status explicitly */
	if (real && gf_ptr->opp && p->timed[gf_ptr->opp])
		result++;

	if (gf_ptr->resist && of_has(flags, gf_ptr->resist))
		result++;

	if (gf_ptr->immunity && of_has(flags, gf_ptr->immunity))
		result = 3;

	/* Notice flags, if it's a real attack */
	if (real && gf_ptr->immunity)
		wieldeds_notice_flag(p, gf_ptr->immunity);
	if (real && gf_ptr->resist)
		wieldeds_notice_flag(p, gf_ptr->resist);
	if (real && gf_ptr->vuln)
		wieldeds_notice_flag(p, gf_ptr->vuln);

	return result;
}


/**
 * Check whether the player is immune to side effects of a GF_ type.
 *
 * \param type is the GF_ type we are checking.
 */
bool check_side_immune(int type)
{
	const struct gf_type *gf_ptr = &gf_table[type];

	if (gf_ptr->immunity) {
		if (gf_ptr->side_immune && check_state(p_ptr, gf_ptr->immunity,
				p_ptr->state.flags))
			return TRUE;
	} else if ((gf_ptr->resist && of_has(p_ptr->state.flags, gf_ptr->resist)) ||
				(gf_ptr->opp && p_ptr->timed[gf_ptr->opp]))
		return TRUE;

	return FALSE;
}

/**
 * Update monster knowledge of player resists.
 *
 * \param m_idx is the monster who is learning
 * \param type is the GF_ type to which it's learning about the player's
 *    resistance (or lack of)
 */
void monster_learn_resists(struct monster *m, struct player *p, int type)
{
	const struct gf_type *gf_ptr = &gf_table[type];

	update_smart_learn(m, p, gf_ptr->resist);
	update_smart_learn(m, p, gf_ptr->immunity);
	update_smart_learn(m, p, gf_ptr->vuln);

	return;
}

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
void teleport_away(struct monster *m_ptr, int dis)
{
	int ny = 0, nx = 0, oy, ox, d, i, min;

	bool look = TRUE;


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
			if (cave->feat[ny][nx] == FEAT_GLYPH) continue;

			/* No teleporting into vaults and such */
			/* if (cave->info[ny][nx] & (CAVE_ICKY)) continue; */

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
			if (cave->info[y][x] & (CAVE_ICKY)) continue;

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
	handle_stuff(p_ptr);
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
	handle_stuff(p_ptr);
}



/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{

	if (is_quest(p_ptr->depth) || (p_ptr->depth >= MAX_DEPTH-1))
	{
		if (OPT(birth_ironman))
		{
			msg("Nothing happens.");
			return;
		}

		msgt(MSG_TPLEVEL, "You rise up through the ceiling.");

		/* New depth */
		p_ptr->depth--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else if ((!p_ptr->depth) || (OPT(birth_ironman)))
	{
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		/* New depth */
		p_ptr->depth++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else if (randint0(100) < 50)
	{
		msgt(MSG_TPLEVEL, "You rise up through the ceiling.");

		/* New depth */
		p_ptr->depth--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else
	{
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		/* New depth */
		p_ptr->depth++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
}


static const char *gf_name_list[] =
{
    #define GF(a, b, c, d, e, f, g, h, i, j, k, l, m) #a,
    #include "list-gf-types.h"
    #undef GF
    NULL
};

int gf_name_to_idx(const char *name)
{
    int i;
    for (i = 0; gf_name_list[i]; i++) {
        if (!my_stricmp(name, gf_name_list[i]))
            return i;
    }

    return -1;
}

const char *gf_idx_to_name(int type)
{
    assert(type >= 0);
    assert(type < GF_MAX);

    return gf_name_list[type];
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
		case GF_DISEN:		return (TERM_VIOLET);
		case GF_NEXUS:		return (TERM_L_RED);
		case GF_CONFU:		return (TERM_L_UMBER);
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
static void bolt_pict(int y, int x, int ny, int nx, int typ, byte *a, char *c)
{
	int motion;

	/* Convert co-ordinates into motion */
	if ((ny == y) && (nx == x))
		motion = BOLT_NO_MOTION;
	else if (nx == x)
		motion = BOLT_0;
	else if ((ny-y) == (x-nx))
		motion = BOLT_45;
	else if (ny == y)
		motion = BOLT_90;
	else if ((ny-y) == (nx-x))
		motion = BOLT_135;
	else
		motion = BOLT_NO_MOTION;

	/* Decide on output char */
	if (use_graphics == GRAPHICS_NONE || use_graphics == GRAPHICS_PSEUDO) {
		/* ASCII is simple */
		char chars[] = "*|/-\\";

		*c = chars[motion];
		*a = spell_color(typ);
	} else {
		*a = gf_to_attr[typ][motion];
		*c = gf_to_char[typ][motion];
	}
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
void take_hit(struct player *p, int dam, const char *kb_str)
{
	int old_chp = p->chp;

	int warning = (p->mhp * op_ptr->hitpoint_warn / 10);


	/* Paranoia */
	if (p->is_dead) return;


	/* Disturb */
	disturb(p, 1, 0);

	/* Mega-Hack -- Apply "invulnerability" */
	if (p->timed[TMD_INVULN] && (dam < 9000)) return;

	/* Hurt the player */
	p->chp -= dam;

	/* Display the hitpoints */
	p->redraw |= (PR_HP);

	/* Dead player */
	if (p->chp < 0)
	{
		/* Hack -- Note death */
		msgt(MSG_DEATH, "You die.");
		message_flush();

		/* Note cause of death */
		my_strcpy(p->died_from, kb_str, sizeof(p->died_from));

		/* No longer a winner */
		p->total_winner = FALSE;

		/* Note death */
		p->is_dead = TRUE;

		/* Leaving */
		p->leaving = TRUE;

		/* Dead */
		return;
	}

	/* Hitpoint warning */
	if (p->chp < warning)
	{
		/* Hack -- bell on first notice */
		if (old_chp > warning)
		{
			bell("Low hitpoint warning!");
		}

		/* Message */
		msgt(MSG_HITPOINT_WARN, "*** LOW HITPOINT WARNING! ***");
		message_flush();
	}
}


/*
 * Destroys a type of item on a given percent chance.
 * The chance 'cperc' is in hundredths of a percent (1-in-10000)
 * Note that missiles are no longer necessarily all destroyed
 *
 * Returns number of items destroyed.
 */
int inven_damage(struct player *p, int type, int cperc)
{
	const struct gf_type *gf_ptr = &gf_table[type];

	int i, j, k, amt;

	object_type *o_ptr;

	char o_name[80];
	
	bool damage;

	bitflag f[OF_SIZE];

	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < QUIVER_END; i++)
	{
		if (i >= INVEN_PACK && i < QUIVER_START) continue;

		o_ptr = &p->inventory[i];

		of_wipe(f);
		object_flags(o_ptr, f);

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Hack -- for now, skip artifacts */
		if (o_ptr->artifact) continue;

		/* Give this item slot a shot at death if it is vulnerable */
		if (of_has(f, gf_ptr->obj_hates) &&	!of_has(f, gf_ptr->obj_imm))
		{
			/* Chance to destroy this item */
			int chance = cperc;

			/* Track if it is damaged instead of destroyed */
			damage = FALSE;

			/** 
			 * Analyze the type to see if we just damage it
			 * - we also check for rods to reduce chance
			 */
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
					if (randint0(10000) < cperc)
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
					if (randint0(10000) < cperc)
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
				p->update |= (PU_BONUS);
				p->redraw |= (PR_EQUIP);

				/* Casualty count */
				amt = o_ptr->number;
			}

			/* ... or count the casualties */
			else for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (randint0(10000) < chance) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr,
					ODESC_BASE);

				/* Message */
				msgt(MSG_DESTROY, "%sour %s (%c) %s %s!",
				           ((o_ptr->number > 1) ?
				            ((amt == o_ptr->number) ? "All of y" :
				             (amt > 1 ? "Some of y" : "One of y")) : "Y"),
				           o_name, index_to_label(i),
				           ((amt > 1) ? "were" : "was"),
					   (damage ? "damaged" : "destroyed"));

				/* Damage already done? */
				if (damage) continue;

				/* Reduce charges if some devices are destroyed */
				reduce_charges(o_ptr, amt);

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
static int minus_ac(struct player *p)
{
	object_type *o_ptr = NULL;

	bitflag f[OF_SIZE];

	char o_name[80];

	/* Avoid crash during monster power calculations */
	if (!p->inventory) return FALSE;

	/* Pick a (possibly empty) inventory slot */
	switch (randint1(6))
	{
		case 1: o_ptr = &p->inventory[INVEN_BODY]; break;
		case 2: o_ptr = &p->inventory[INVEN_ARM]; break;
		case 3: o_ptr = &p->inventory[INVEN_OUTER]; break;
		case 4: o_ptr = &p->inventory[INVEN_HANDS]; break;
		case 5: o_ptr = &p->inventory[INVEN_HEAD]; break;
		case 6: o_ptr = &p->inventory[INVEN_FEET]; break;
		default: assert(0);
	}

	/* Nothing to damage */
	if (!o_ptr->kind) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Object resists */
	if (of_has(f, OF_IGNORE_ACID))
	{
		msg("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Message */
	msg("Your %s is damaged!", o_name);

	/* Damage the item */
	o_ptr->to_a--;

	p->update |= PU_BONUS;
	p->redraw |= (PR_EQUIP);

	/* Item was damaged */
	return (TRUE);
}

/**
 * Adjust damage according to resistance or vulnerability.
 *
 * \param type is the attack type we are checking.
 * \param dam is the unadjusted damage.
 * \param dam_aspect is the calc we want (min, avg, max, random).
 * \param resist is the degree of resistance (-1 = vuln, 3 = immune).
 */
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect, int resist)
{
	const struct gf_type *gf_ptr = &gf_table[type];
	int i, denom;

	if (resist == 3) /* immune */
		return 0;

	/* Hack - acid damage is halved by armour, holy orb is halved */
	if ((type == GF_ACID && minus_ac(p)) || type == GF_HOLY_ORB)
		dam = (dam + 1) / 2;

	if (resist == -1) /* vulnerable */
		return (dam * 4 / 3);

	/* Variable resists vary the denominator, so we need to invert the logic
	 * of dam_aspect. (m_bonus is unused) */
	switch (dam_aspect) {
		case MINIMISE:
			denom = randcalc(gf_ptr->denom, 0, MAXIMISE);
			break;
		case MAXIMISE:
			denom = randcalc(gf_ptr->denom, 0, MINIMISE);
			break;
		default:
			denom = randcalc(gf_ptr->denom, 0, dam_aspect);
	}

	for (i = resist; i > 0; i--)
		if (denom)
			dam = dam * gf_ptr->num / denom;

	return dam;
}

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
	if (!o_ptr->kind) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}


	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);


	/* Artifacts have 60% chance to resist */
	if (o_ptr->artifact && (randint0(100) < 60))
	{
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!",
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
	msg("Your %s (%c) %s disenchanted!",
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
			if (cave_issecretdoor(cave, y, x))
			{
				place_closed_door(cave, y, x);

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					obvious = TRUE;
				}
			}

			/* Destroy traps */
			if (cave_istrap(cave, y, x))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					msg("There is a bright flash of light!");
					obvious = TRUE;
				}

				/* Forget the trap */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the trap */
				cave_set_feat(cave, y, x, FEAT_FLOOR);
			}

			/* Locked doors are unlocked */
			else if ((cave->feat[y][x] >= FEAT_DOOR_HEAD + 0x01) &&
			          (cave->feat[y][x] <= FEAT_DOOR_HEAD + 0x07))
			{
				/* Unlock the door */
				cave_set_feat(cave, y, x, FEAT_DOOR_HEAD + 0x00);

				/* Check line of sound */
				if (player_has_los_bold(y, x))
				{
					msg("Click!");
					obvious = TRUE;
				}
			}

			break;
		}

		/* Destroy Doors (and traps) */
		case GF_KILL_DOOR:
		{
			/* Destroy all doors and traps */
			if (cave_istrap(cave, y, x) ||
					cave_isopendoor(cave, y, x) ||
					cave->feat[y][x] == FEAT_BROKEN ||
					cave_isdoor(cave, y, x))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg("There is a bright flash of light!");
					obvious = TRUE;

					/* Visibility change */
					if ((cave->feat[y][x] >= FEAT_DOOR_HEAD) &&
					    (cave->feat[y][x] <= FEAT_DOOR_TAIL))
					{
						/* Update the visuals */
						p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
					}
				}

				/* Forget the door */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(cave, y, x, FEAT_FLOOR);
			}

			break;
		}

		/* Destroy walls (and doors) */
		case GF_KILL_WALL:
		{
			/* Non-walls (etc) */
			if (cave_floor_bold(y, x)) break;

			/* Permanent walls */
			if (cave->feat[y][x] >= FEAT_PERM_EXTRA) break;

			/* Granite */
			if (cave->feat[y][x] >= FEAT_WALL_EXTRA)
			{
				/* Message */
				if (cave->info[y][x] & (CAVE_MARK))
				{
					msg("The wall turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(cave, y, x, FEAT_FLOOR);
			}

			/* Quartz / Magma with treasure */
			else if (cave->feat[y][x] >= FEAT_MAGMA_H)
			{
				/* Message */
				if (cave->info[y][x] & (CAVE_MARK))
				{
					msg("The vein turns into mud!");
					msg("You have found something!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(cave, y, x, FEAT_FLOOR);

				/* Place some gold */
				place_gold(cave, y, x, p_ptr->depth, ORIGIN_FLOOR);
			}

			/* Quartz / Magma */
			else if (cave->feat[y][x] >= FEAT_MAGMA)
			{
				/* Message */
				if (cave->info[y][x] & (CAVE_MARK))
				{
					msg("The vein turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(cave, y, x, FEAT_FLOOR);
			}

			/* Rubble */
			else if (cave->feat[y][x] == FEAT_RUBBLE)
			{
				/* Message */
				if (cave->info[y][x] & (CAVE_MARK))
				{
					msg("The rubble turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the rubble */
				cave_set_feat(cave, y, x, FEAT_FLOOR);

				/* Hack -- place an object */
				if (randint0(100) < 10){
					if (player_can_see_bold(y, x)) {
						msg("There was something buried in the rubble!");
						obvious = TRUE;
					}
					place_object(cave, y, x, p_ptr->depth, FALSE, FALSE,
						ORIGIN_RUBBLE);
				}
			}

			/* Destroy doors (and secret doors) */
			else /* if (cave->feat[y][x] >= FEAT_DOOR_HEAD) */
			{
				/* Hack -- special message */
				if (cave->info[y][x] & (CAVE_MARK))
				{
					msg("The door turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave->info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(cave, y, x, FEAT_FLOOR);
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
			/* Require a grid without monsters */
			if (cave->m_idx[y][x]) break;
			
			/* Require a floor grid */
			if (!(cave->feat[y][x] == FEAT_FLOOR)) break;
			
			/* Push objects off the grid */
			if (cave->o_idx[y][x]) push_object(y,x);

			/* Create closed door */
			cave_set_feat(cave, y, x, FEAT_DOOR_HEAD + 0x00);

			/* Observe */
			if (cave->info[y][x] & (CAVE_MARK)) obvious = TRUE;

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
			place_trap(cave, y, x);

			break;
		}

		/* Light up the grid */
		case GF_LIGHT_WEAK:
		case GF_LIGHT:
		{
			/* Turn on the light */
			cave->info[y][x] |= (CAVE_GLOW);

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
				cave->info[y][x] &= ~(CAVE_GLOW);

				/* Hack -- Forget "boring" grids */
				if (cave->feat[y][x] <= FEAT_INVIS)
					cave->info[y][x] &= ~(CAVE_MARK);
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
static bool project_o(int who, int r, int y, int x, int dam, int typ,
	bool obvious)
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
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		bool is_art = FALSE;
		bool ignore = FALSE;
		bool plural = FALSE;
		bool do_kill = FALSE;

		const char *note_kill = NULL;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Extract the flags */
		object_flags(o_ptr, f);

		/* Get the "plural"-ness */
		if (o_ptr->number > 1) plural = TRUE;

		/* Check for artifact */
		if (o_ptr->artifact) is_art = TRUE;

		/* Analyze the type */
		switch (typ)
		{
			/* Acid -- Lots of things */
			case GF_ACID:
			{
				if (of_has(f, OF_HATES_ACID))
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
				if (of_has(f, OF_HATES_ELEC))
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
				if (of_has(f, OF_HATES_FIRE))
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
				if (of_has(f, OF_HATES_COLD))
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
				if (of_has(f, OF_HATES_FIRE))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (of_has(f, OF_IGNORE_FIRE)) ignore = TRUE;
				}
				if (of_has(f, OF_HATES_ELEC))
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
				if (of_has(f, OF_HATES_FIRE))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (of_has(f, OF_IGNORE_FIRE)) ignore = TRUE;
				}
				if (of_has(f, OF_HATES_COLD))
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
				if (of_has(f, OF_HATES_COLD))
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
				if (cursed_p(o_ptr->flags))
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
					if (o_ptr->pval[DEFAULT_PVAL] > 0)
					{
						/* Disarm or Unlock */
						o_ptr->pval[DEFAULT_PVAL] = (0 - o_ptr->pval[DEFAULT_PVAL]);

						/* Identify */
						object_notice_everything(o_ptr);

						/* Notice */
						if (o_ptr->marked && !squelch_item_ok(o_ptr))
						{
							msg("Click!");
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
			if (o_ptr->marked && !squelch_item_ok(o_ptr))
			{
				obvious = TRUE;
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
			}

			/* Artifacts, and other objects, get to resist */
			if (is_art || ignore)
			{
				/* Observe the resist */
				if (o_ptr->marked && !squelch_item_ok(o_ptr))
				{
					msg("The %s %s unaffected!",
					           o_name, (plural ? "are" : "is"));
				}
			}

			/* Kill it */
			else
			{
				/* Describe if needed */
				if (o_ptr->marked && note_kill && !squelch_item_ok(o_ptr))
				{
					msgt(MSG_DESTROY, "The %s%s", o_name, note_kill);
				}

				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Redraw */
				cave_light_spot(cave, y, x);
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
static bool project_m(int who, int r, int y, int x, int dam, int typ,
	bool obvious)
{
	int tmp;
	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;
	u16b flag = 0;

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

	/* Slow setting (amount to haste) */
	int do_slow = 0;

	/* Haste setting (amount to haste) */
	int do_haste = 0;

	/* Sleep amount (amount to sleep) */
	bool do_sleep = FALSE;

	/* Fear amount (amount to fear) */
	int do_fear = 0;

	/* Hold the monster name */
	char m_name[80];
	char m_poss[80];

	int m_idx = cave->m_idx[y][x];

	/* Assume no note */
	int m_note = MON_MSG_NONE;

	/* Assume a default death */
	byte note_dies = MON_MSG_DIE;


	/* Walls protect monsters */
	if (!cave_floor_bold(y,x)) return (FALSE);


	/* No monster here */
	if (!(m_idx > 0)) return (FALSE);

	/* Never affect projector */
	if (m_idx == who) return (FALSE);


	/* Obtain monster info */
	m_ptr = cave_monster(cave, m_idx);
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];
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
		note_dies = MON_MSG_DESTROYED;
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
				m_note = MON_MSG_RESIST_A_LOT;
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
				m_note = MON_MSG_RESIST_A_LOT;
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
				m_note = MON_MSG_RESIST_A_LOT;
				dam /= 9;
			}
			else if (rf_has(r_ptr->flags, RF_HURT_FIRE))
			{
				m_note = MON_MSG_CATCH_FIRE;
				note_dies = MON_MSG_DISENTEGRATES;
				dam *= 2;
			}
			break;
		}

		/* Cold */
		case GF_COLD:
		/* Ice -- Cold + Stun */
		case GF_ICE:
		{
			if (seen)
			{
				obvious = TRUE;
				rf_on(l_ptr->flags, RF_IM_COLD);
				rf_on(l_ptr->flags, RF_HURT_COLD);
			}

			if (typ == GF_ICE) {
				if (who > 0) {
					do_stun = (randint1(15) + r) / (r + 1);
					flag |= MON_TMD_MON_SOURCE;
				} else
					do_stun = (randint1(15) + r + p_ptr->lev / 5) / (r + 1);
			}

			if (rf_has(r_ptr->flags, RF_IM_COLD))
			{
				m_note = MON_MSG_RESIST_A_LOT;
				dam /= 9;
			}
			else if (rf_has(r_ptr->flags, RF_HURT_COLD))
			{
				m_note = MON_MSG_BADLY_FROZEN;
				note_dies = MON_MSG_FREEZE_SHATTER;
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
				m_note = MON_MSG_RESIST_A_LOT;
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
				m_note = MON_MSG_HIT_HARD;
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
				m_note = MON_MSG_RESIST;
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
				m_note = MON_MSG_IMMUNE;
				dam = 0;
			}
			else if (rf_has(r_ptr->flags, RF_RES_NETH) ||
			         rsf_has(r_ptr->spell_flags, RSF_BR_NETH))
			{
				m_note = MON_MSG_RESIST;
				dam *= 3; dam /= (randint1(6)+6);
			}
			else if (rf_has(r_ptr->flags, RF_EVIL))
			{
				dam /= 2;
				m_note = MON_MSG_RESIST_SOMEWHAT;
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
				m_note = MON_MSG_IMMUNE;
				dam = 0;
			}
			break;
		}

		/* Chaos -- Chaos breathers resist */
		case GF_CHAOS:
		{
			if (seen) obvious = TRUE;

			do_poly = TRUE;

			if (who > 0) {
				do_conf = (5 + randint1(11) + r) / (r + 1);
				flag |= MON_TMD_MON_SOURCE;
			} else
				do_conf = (5 + randint1(11) + r + p_ptr->lev / 5) / (r + 1);

			if (rsf_has(r_ptr->spell_flags, RSF_BR_CHAO))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_CHAO);

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

				m_note = MON_MSG_RESIST;
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Sound -- Sound breathers resist */
		case GF_SOUND:
		{
			if (seen) obvious = TRUE;

			if (who > 0) {
				do_stun = (10 + randint1(15) + r) / (r + 1);
				flag |= MON_TMD_MON_SOURCE;
			} else
				do_stun = (10 + randint1(15) + r + p_ptr->lev / 5) / (r + 1);

			if (rsf_has(r_ptr->spell_flags, RSF_BR_SOUN))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_SOUN);

				m_note = MON_MSG_RESIST;
				dam *= 2; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Disenchantment */
		case GF_DISEN:
		{
			if (seen) obvious = TRUE;
			if (seen) rf_on(l_ptr->flags, RF_RES_DISE);
			if (rf_has(r_ptr->flags, RF_RES_DISE))
			{
				m_note = MON_MSG_RESIST;
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
				m_note = MON_MSG_RESIST;
				dam *= 3; dam /= (randint1(6)+6);
			}
			break;
		}

		/* Force */
		case GF_FORCE:
		{
			if (seen) obvious = TRUE;

			if (who > 0) {
				do_stun = (randint1(15) + r) / (r + 1);
				flag |= MON_TMD_MON_SOURCE;
			} else
				do_stun = (randint1(15) + r + p_ptr->lev / 5) / (r + 1);

			if (rsf_has(r_ptr->spell_flags, RSF_BR_WALL))
			{
				/* Learn about breathers through resistance */
				if (seen) rsf_on(l_ptr->spell_flags, RSF_BR_WALL);

				m_note = MON_MSG_RESIST;
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

				m_note = MON_MSG_RESIST;
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

				m_note = MON_MSG_RESIST;
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

				m_note = MON_MSG_RESIST;
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
			if (monster_is_nonliving(r_ptr))
			{
				m_note = MON_MSG_UNAFFECTED;
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
			if (multiply_monster(m_idx))
			{
				m_note = MON_MSG_SPAWN;
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
			mon_clear_timed(m_idx, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE);

			/* Heal */
			m_ptr->hp += dam;

			/* No overflow */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Message */
			else m_note = MON_MSG_HEALTHIER;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Speed Monster (Ignore "dam") */
		case GF_OLD_SPEED:
		{
			if (seen) obvious = TRUE;

			/* Speed up */
			do_haste = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Slow Monster (Use "dam" as "power") */
		case GF_OLD_SLOW:
		{
			if (seen) obvious = TRUE;

			do_slow = dam;

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
				m_note = MON_MSG_CRINGE_LIGHT;
				note_dies = MON_MSG_SHRIVEL_LIGHT;
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

				m_note = MON_MSG_RESIST;
				dam *= 2; dam /= (randint1(6)+6);
			}
			else if (rf_has(r_ptr->flags, RF_HURT_LIGHT))
			{
				m_note = MON_MSG_CRINGE_LIGHT;
				note_dies = MON_MSG_SHRIVEL_LIGHT;
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

				m_note = MON_MSG_RESIST;
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
				m_note = MON_MSG_LOSE_SKIN;
				note_dies = MON_MSG_DISSOLVE;
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
				m_note = MON_MSG_SHUDDER;
				note_dies = MON_MSG_DISSOLVE;
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
				m_note = MON_MSG_SHUDDER;
				note_dies = MON_MSG_DISSOLVE;
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
			m_note = MON_MSG_SHUDDER;
			note_dies = MON_MSG_DISSOLVE;

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
		m_note = note_dies;
	}

	/* Handle polymorph */
	else if (do_poly)
	{
		/* Default -- assume no polymorph */
		m_note = MON_MSG_UNAFFECTED;

		/* Uniques cannot be polymorphed */
		if (!rf_has(r_ptr->flags, RF_UNIQUE))
		{
			if (seen) obvious = TRUE;

			/* Saving throws are allowed */
			if (r_ptr->level > randint1(90) ||
			    (typ == GF_OLD_POLY && r_ptr->level > randint1(MAX(1, do_poly - 10)) + 10))
			{
				if (typ == GF_OLD_POLY) m_note = MON_MSG_MAINTAIN_SHAPE;
			}
			else
			{
				/* Pick a "new" monster race */
				tmp = poly_r_idx(m_ptr->r_idx);

				/* Handle polymorph */
				if (tmp != m_ptr->r_idx)
				{
					/* Monster polymorphs */
					m_note = MON_MSG_CHANGE;

					/* Add the message now before changing the monster race */
					add_monster_message(m_name, m_idx, m_note, FALSE);

					/* No more messages */
					m_note = MON_MSG_NONE;

					/* Turn off the damage */
					dam = 0;

					/* "Kill" the "old" monster */
					delete_monster_idx(m_idx);

					/* Create a new monster (no groups) */
					(void)place_monster_aux(cave, y, x, tmp, FALSE, FALSE,
						ORIGIN_DROP_POLY);

					/* Hack -- Assume success XXX XXX XXX */

					/* Hack -- Get new monster */
					m_ptr = cave_monster(cave, m_idx);

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
		m_note = MON_MSG_DISAPPEAR;

		/* Teleport */
		teleport_away(m_ptr, do_dist);

		/* Hack -- get new location */
		y = m_ptr->fy;
		x = m_ptr->fx;
	}

	/* Handle stunning, confusion, slowing, hasting and fear */
	else if (do_stun)
	{
		if (m_ptr->m_timed[MON_TMD_STUN])
			do_stun /= 2;

		obvious = mon_inc_timed(m_idx, MON_TMD_STUN, do_stun, flag | MON_TMD_FLG_NOTIFY);
	}

	else if (do_conf)
	{
		int tmp = damroll(3, (do_conf / 2)) + 1;

		obvious = mon_inc_timed(m_idx, MON_TMD_CONF, tmp, flag | MON_TMD_FLG_NOTIFY);
	}

	else if (do_slow)
		obvious = mon_inc_timed(m_idx, MON_TMD_SLOW, do_slow, flag | MON_TMD_FLG_NOTIFY);
	else if (do_haste)
		obvious = mon_inc_timed(m_idx, MON_TMD_FAST, do_haste, flag | MON_TMD_FLG_NOTIFY);

	if (do_fear)
		obvious = mon_inc_timed(m_idx, MON_TMD_FEAR, do_fear, flag | MON_TMD_FLG_NOTIFY);

	/* If another monster did the damage, hurt the monster by hand */
	if (who > 0)
	{
		/* Redraw (later) if needed */
		if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

		/* Wake the monster up */
		mon_clear_timed(m_idx, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE);

		/* Hurt the monster */
		m_ptr->hp -= dam;

		/* Dead monster */
		if (m_ptr->hp < 0)
		{
			/* Give detailed messages if destroyed */
			if (!seen) note_dies = MON_MSG_MORIA_DEATH;

			/* dump the note*/
			add_monster_message(m_name, m_idx, note_dies, FALSE);

			/* Generate treasure, etc */
			monster_death(m_idx, FALSE);

			/* Delete the monster */
			delete_monster_idx(m_idx);

			mon_died = TRUE;
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if ((m_note != MON_MSG_NONE) && seen)
			{
				add_monster_message(m_name, m_idx, m_note, FALSE);
			}

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(m_idx, dam);
		}
	}

	/* If the player did it, give them experience, check fear */
	else
	{
		bool fear = FALSE;

		/* The monster is going to be killed */
		if (dam > m_ptr->hp)
		{
			/* Adjust message for unseen monsters */
			if (!seen) note_dies = MON_MSG_MORIA_DEATH;

			/* Save the death notification for later */
			add_monster_message(m_name, m_idx, note_dies, FALSE);
		}

		if (do_sleep)
			obvious = mon_inc_timed(m_idx, MON_TMD_SLEEP, 500 + p_ptr->lev * 10,
				flag | MON_TMD_FLG_NOTIFY);
		else if (mon_take_hit(m_idx, dam, &fear, ""))
			mon_died = TRUE;
		else
		{
			/* Give detailed messages if visible or destroyed */
			if ((m_note != MON_MSG_NONE) && seen)
			{
				add_monster_message(m_name, m_idx, m_note, FALSE);
			}

			/* Hack -- Pain message */
			else if (dam > 0)
				message_pain(m_idx, dam);

			if (fear && m_ptr->ml)
				add_monster_message(m_name, m_idx, MON_MSG_FLEE_IN_TERROR, TRUE);
		}
	}

	/* Verify this code XXX XXX XXX */

	/* Update the monster */
	if (!mon_died) update_mon(m_idx, FALSE);

	/* Redraw the monster grid */
	cave_light_spot(cave, y, x);


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
	/* Get the damage type details */
	const struct gf_type *gf_ptr = &gf_table[typ];

	/* Source monster */
	monster_type *m_ptr = cave_monster(cave, who);

	/* Monster name (for attacks) */
	char m_name[80];

	/* Monster name (for damage) */
	char killer[80];

	/* Player blind-ness */
	bool blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	bool seen = (!blind && m_ptr->ml);

	/* No player here */
	if (!(cave->m_idx[y][x] < 0)) return (FALSE);

	/* Never affect projector */
	if (cave->m_idx[y][x] == who) return (FALSE);

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);

	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Get the monster's real name */
	monster_desc(killer, sizeof(killer), m_ptr, MDESC_SHOW | MDESC_IND2);

	/* Let player know what is going on */
	if (!seen)
		msg("You are hit by %s!", gf_ptr->desc);

	if (typ == GF_GRAVITY)
		msg("Gravity warps around you.");

	/* Adjust damage for resistance, immunity or vulnerability, and apply it */
	dam = adjust_dam(p_ptr, typ, dam, RANDOMISE, check_for_resist(p_ptr, typ,
		p_ptr->state.flags, TRUE));
	if (dam)
		take_hit(p_ptr, dam, killer);

	/* Disturb */
	disturb(p_ptr, 1, 0);

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
 * Note that we must call "handle_stuff(p_ptr)" after affecting terrain features
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

	int msec = op_ptr->delay_factor;

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
		x1 = cave_monster(cave, who)->fx;
		y1 = cave_monster(cave, who)->fy;
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
	handle_stuff(p_ptr);

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
				byte a;
				char c;

				/* Obtain the bolt pict */
				bolt_pict(oy, ox, y, x, typ, &a, &c);

				/* Visual effects */
				print_rel(c, a, y, x);
				move_cursor_relative(y, x);

				Term_fresh();
				if (p_ptr->redraw) redraw_stuff(p_ptr);

				Term_xtra(TERM_XTRA_DELAY, msec);

				cave_light_spot(cave, y, x);

				Term_fresh();
				if (p_ptr->redraw) redraw_stuff(p_ptr);

				/* Display "beam" grids */
				if (flg & (PROJECT_BEAM))
				{
					/* Obtain the explosion pict */
					bolt_pict(y, x, y, x, typ, &a, &c);

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
					byte a;
					char c;

					drawn = TRUE;

					/* Obtain the explosion pict */
					bolt_pict(y, x, y, x, typ, &a, &c);

					/* Visual effects -- Display */
					print_rel(c, a, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush each "radius" separately */
			Term_fresh();

			/* Flush */
			if (p_ptr->redraw) redraw_stuff(p_ptr);

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
					cave_light_spot(cave, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush the explosion */
			Term_fresh();

			/* Flush */
			if (p_ptr->redraw) redraw_stuff(p_ptr);
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
	if (p_ptr->update) update_stuff(p_ptr);


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
			if (cave->m_idx[y][x] > 0)
			{
				monster_type *m_ptr = cave_monster(cave, cave->m_idx[y][x]);

				/* Hack -- auto-recall */
				if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

				/* Hack - auto-track */
				if (m_ptr->ml) health_track(p_ptr, cave->m_idx[y][x]);
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
