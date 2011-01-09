/*
 * File: attack.c
 * Purpose: Attacking (both throwing and melee) code
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
#include "cmds.h"
#include "monster/monster.h"
#include "object/slays.h"
#include "object/tvalsval.h"
#include "spells.h"
#include "target.h"

/* Returns percent chance of an object breaking after throwing or shooting. */
int breakage_chance(const object_type *o_ptr)
{
	/* Artifacts never break */
	if (artifact_p(o_ptr)) return 0;

	switch (o_ptr->tval)
	{
		case TV_FLASK:
		case TV_POTION:
		case TV_BOTTLE:
		case TV_FOOD:
		case TV_JUNK:
			return 100;

		case TV_LIGHT:
		case TV_SCROLL:
		case TV_SKELETON:
			return 50;

		case TV_ARROW:
			return 35;

		case TV_WAND:
		case TV_SHOT:
		case TV_BOLT:
		case TV_SPIKE:
			return 25;

		default:
			return 10;
	}
}


/*
 * Determine if the player "hits" a monster.
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit(int chance, int ac, int vis)
{
	int k;

	/* Percentile dice */
	k = randint0(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Penalize invisible targets */
	if (!vis) chance = chance / 2;

	/* Power competes against armor */
	if ((chance > 0) && (randint0(chance) >= (ac * 3 / 4))) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
static int critical_shot(int weight, int plus, int dam, u32b *msg_type)
{
	int i, k;

	/* Extract "shot" power */
	i = (weight + ((p_ptr->state.to_h + plus) * 4) + (p_ptr->lev * 2));

	/* Critical hit */
	if (randint1(5000) <= i)
	{
		k = weight + randint1(500);

		if (k < 500)
		{
			*msg_type = MSG_HIT_GOOD;
			dam = 2 * dam + 5;
		}
		else if (k < 1000)
		{
			*msg_type = MSG_HIT_GREAT;
			dam = 2 * dam + 10;
		}
		else
		{
			*msg_type = MSG_HIT_SUPERB;
			dam = 3 * dam + 15;
		}
	}
	else
		*msg_type = MSG_SHOOT_HIT;

	return (dam);
}



/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
static int critical_norm(int weight, int plus, int dam, u32b *msg_type)
{
	int i, k;

	/* Extract "blow" power */
	i = (weight + ((p_ptr->state.to_h + plus) * 5) + (p_ptr->lev * 3));

	/* Chance */
	if (randint1(5000) <= i)
	{
		k = weight + randint1(650);

		if (k < 400)
		{
			*msg_type = MSG_HIT_GOOD;
			dam = 2 * dam + 5;
		}
		else if (k < 700)
		{
			*msg_type = MSG_HIT_GREAT;
			dam = 2 * dam + 10;
		}
		else if (k < 900)
		{
			*msg_type = MSG_HIT_SUPERB;
			dam = 3 * dam + 15;
		}
		else if (k < 1300)
		{
			*msg_type = MSG_HIT_HI_GREAT;
			dam = 3 * dam + 20;
		}
		else
		{
			*msg_type = MSG_HIT_HI_SUPERB;
			dam = ((7 * dam) / 2) + 25;
		}
	}
	else
		*msg_type = MSG_HIT;

	return dam;
}


/*
 * Attack the monster at the given location
 *
 * If no "weapon" is available, then "punch" the monster one time.
 *
 * We get blows until energy drops below that required for another blow, or
 * until the target monster dies. We use a wrapper to work out the number of
 * blows. We don't allow @ to spend more than 100 energy in one go, to avoid
 * slower monsters getting double moves.
 */
bool py_attack_real(int y, int x)
{
	int bonus, chance;

	monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	object_type *o_ptr;

	char m_name[80];

	bool fear = FALSE;

	bool do_quake = FALSE;
	bool dead = FALSE;

	u32b msg_type = 0;
	bool success = FALSE;

	/* Default to punching for one damage */
	const char *hit_verb = "punch";
	int dmg = 1;
	msg_type = MSG_HIT;

	/* Extract monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Auto-Recall if possible and visible */
	if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

	/* Track a new monster */
	if (m_ptr->ml) health_track(p_ptr, cave->m_idx[y][x]);

	/* Handle player fear (only for invisible monsters) */
	if (p_ptr->state.afraid)
	{
		msgt(MSG_AFRAID, "You are too afraid to attack %s!", m_name);
		return (FALSE);
	}

	/* Disturb the monster */
	wake_monster(m_ptr);

	/* Get the weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Calculate the "attack quality" */
	bonus = p_ptr->state.to_h + o_ptr->to_h;
	chance = (p_ptr->state.skills[SKILL_TO_HIT_MELEE] + (bonus * BTH_PLUS_ADJ));

	/* See if the player hit */
	success = test_hit(chance, r_ptr->ac, m_ptr->ml);

	/* If a miss, skip this hit */
	if (!success)
	{
		msgt(MSG_MISS, "You miss %s.", m_name);
		return (FALSE);
	}

	/* Handle normal weapon */
	if (o_ptr->k_idx)
	{
		int i;
		const struct slay *best_s_ptr = NULL;

		hit_verb = "hit";

		/* Get the best attack from all slays or
		 * brands on all non-launcher equipment */
		for (i = INVEN_LEFT; i < INVEN_TOTAL; i++)
			improve_attack_modifier(&p_ptr->inventory[i], m_ptr, &best_s_ptr,
					TRUE, FALSE);

		improve_attack_modifier(o_ptr, m_ptr, &best_s_ptr, TRUE, FALSE);
		if (best_s_ptr != NULL)
			hit_verb = best_s_ptr->melee_verb;

		dmg = damroll(o_ptr->dd, o_ptr->ds);
		dmg *= (best_s_ptr == NULL) ? 1 : best_s_ptr->mult;

		if (p_ptr->state.impact && (dmg > 50))
			do_quake = TRUE;

		dmg += o_ptr->to_d;
		dmg = critical_norm(o_ptr->weight, o_ptr->to_h, dmg, &msg_type);

		/* Learn by use for the weapon */
		object_notice_attack_plusses(o_ptr);

		if (do_quake)
			wieldeds_notice_flag(OF_IMPACT);
	}

	/* Learn by use for other equipped items */
	wieldeds_notice_on_attack();

	/* Apply the player damage bonuses */
	dmg += p_ptr->state.to_d;

	/* No negative damage */
	if (dmg <= 0) dmg = 0;

	/* Tell the player what happened */
	if (dmg <= 0)
		msgt(MSG_MISS, "You fail to harm %s.", m_name);
	else if (msg_type == MSG_HIT)
		msgt(MSG_HIT, "You %s %s.", hit_verb, m_name);
	else if (msg_type == MSG_HIT_GOOD)
		msgt(MSG_HIT_GOOD, "You %s %s. %s", hit_verb, m_name, "It was a good hit!");
	else if (msg_type == MSG_HIT_GREAT)
		msgt(MSG_HIT_GREAT, "You %s %s. %s", hit_verb, m_name, "It was a great hit!");
	else if (msg_type == MSG_HIT_SUPERB)
		msgt(MSG_HIT_SUPERB, "You %s %s. %s", hit_verb, m_name, "It was a superb hit!");
	else if (msg_type == MSG_HIT_HI_GREAT)
		msgt(MSG_HIT_HI_GREAT, "You %s %s. %s", hit_verb, m_name, "It was a *GREAT* hit!");
	else if (msg_type == MSG_HIT_HI_SUPERB)
		msgt(MSG_HIT_HI_SUPERB, "You %s %s. %s", hit_verb, m_name, "It was a *SUPERB* hit!");

	/* Complex message */
	if (p_ptr->wizard)
		msg("You do %d (out of %d) damage.", dmg, m_ptr->hp);

	/* Confusion attack */
	if (p_ptr->confusing)
	{
		/* Cancel glowing hands */
		p_ptr->confusing = FALSE;

		/* Message */
		msg("Your hands stop glowing.");

		/* Update the lore */
		if (m_ptr->ml)
			rf_on(l_ptr->flags, RF_NO_CONF);

		/* Confuse the monster */
		if (rf_has(r_ptr->flags, RF_NO_CONF))
			msg("%^s is unaffected.", m_name);
		else if (randint0(100) < r_ptr->level)
			msg("%^s is unaffected.", m_name);
		else
		{
			msg("%^s appears confused.", m_name);
			m_ptr->confused += 10 + randint0(p_ptr->lev) / 5;
		}
	}

	/* Damage, check for fear and death */
	dead = mon_take_hit(cave->m_idx[y][x], dmg, &fear, NULL);

	/* Hack -- delay fear messages */
	if (fear && m_ptr->ml)
		msgt(MSG_FLEE, "%^s flees in terror!",
		m_name);

	/* Mega-Hack -- apply earthquake brand */
	if (do_quake) earthquake(p_ptr->py, p_ptr->px, 10);

	return (dead);
}

void py_attack(int y, int x)
{
	int blow_energy;
	bool stop = FALSE;
	int blows = 0;

	/* disturb the player */
	disturb(0,0);

	/* calculate energy per blow */
	blow_energy = 10000 / p_ptr->state.num_blow;

	/*
	 * set energy use to zero, overriding whatever was set before we
	 * got here
	 */
	p_ptr->energy_use = 0;

	/*
	 * take blows until energy runs out or monster dies -
	 * note that we limit energy use to 100, even if an extra blow would
	 * be possible, to avoid monster double moves
	 */
	while ((p_ptr->energy >= (blow_energy * (blows + 1))) && !stop)
	{
		stop = py_attack_real(y, x);
		p_ptr->energy_use += blow_energy;
		if ((p_ptr->energy_use + blow_energy) > 100) stop = TRUE;
		blows++;
	}
}


/*
 * Fire an object from the pack or floor.
 *
 * You may only fire items that "match" your missile launcher.
 *
 * See "calc_bonuses()" for more calculations and such.
 *
 * Note that "firing" a missile is MUCH better than "throwing" it.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that when firing missiles, the launcher multiplier is applied
 * after all the bonuses are added in, making multipliers very useful.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 */
void do_cmd_fire(cmd_code code, cmd_arg args[])
{
	int dir, item;
	int i, j, y, x;
	s16b ty, tx;
	int tdam, tdis, thits;
	int bonus, chance;

	object_type *o_ptr;
	object_type *j_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	bool hit_body = FALSE;

	byte missile_attr;
	char missile_char;

	char o_name[80];

	u32b msg_type = 0;

	int path_n;
	u16b path_g[256];

	int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	/* Get the "bow" */
	j_ptr = &p_ptr->inventory[INVEN_BOW];

	/* Require a usable launcher */
	if (!j_ptr->tval || !p_ptr->state.ammo_tval)
	{
		msg("You have nothing to fire with.");
		return;
	}

	/* Get item to fire and direction to fire in. */
	item = args[0].item;
	dir = args[1].direction;

	/* Check the item being fired is usable by the player. */
	if (!item_is_available(item, NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		msg("That item is not within your reach.");
		return;
	}

	/* Get the object for the ammo */
	o_ptr = object_from_item_idx(item);

	/* Check the ammo can be used with the launcher */
	if (o_ptr->tval != p_ptr->state.ammo_tval)
	{
		msg("That ammo cannot be fired by your current weapon.");
		return;
	}

	/* Base range XXX XXX */
	tdis = 6 + 2 * p_ptr->state.ammo_mult;

	/* Start at the player */
	x = p_ptr->px;
	y = p_ptr->py;

	/* Predict the "target" location */
	ty = y + 99 * ddy[dir];
	tx = x + 99 * ddx[dir];

	/* Check for target validity */
	if ((dir == 5) && target_okay())
	{
		target_get(&tx, &ty);
		if (distance(y, x, ty, tx) > tdis)
		{
			if (!get_check("Target out of range.  Fire anyway? "))
				return;
		}
	}

	/* Sound */
	sound(MSG_SHOOT);

	object_notice_on_firing(o_ptr);

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL | ODESC_SINGULAR);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(o_ptr);
	missile_char = object_char(o_ptr);

	/* Use the proper number of shots */
	thits = p_ptr->state.num_fire;

	/* Actually "fire" the object */
	bonus = (p_ptr->state.to_h + o_ptr->to_h + j_ptr->to_h);
	chance = p_ptr->state.skills[SKILL_TO_HIT_BOW] +
			(bonus * BTH_PLUS_ADJ);

	/* Take a (partial) turn */
	p_ptr->energy_use = (100 / thits);

	/* Calculate the path */
	path_n = project_path(path_g, tdis, y, x, ty, tx, 0);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Project along the path */
	for (i = 0; i < path_n; ++i)
	{
		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		/* Hack -- Stop before hitting walls */
		if (!cave_floor_bold(ny, nx)) break;

		/* Advance */
		x = nx;
		y = ny;

		/* Only do visuals if the player can "see" the missile */
		if (player_can_see_bold(y, x))
		{
			/* Visual effects */
			print_rel(missile_char, missile_attr, y, x);
			move_cursor_relative(y, x);

			Term_fresh();
			if (p_ptr->redraw) redraw_stuff();

			Term_xtra(TERM_XTRA_DELAY, msec);
			cave_light_spot(cave, y, x);

			Term_fresh();
			if (p_ptr->redraw) redraw_stuff();
		}

		/* Delay anyway for consistency */
		else
		{
			/* Pause anyway, for consistancy */
			Term_xtra(TERM_XTRA_DELAY, msec);
		}

		/* Handle monster */
		if (cave->m_idx[y][x] > 0)
		{
			monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			int chance2 = chance - distance(p_ptr->py, p_ptr->px, y, x);
			int visible = m_ptr->ml;
			int multiplier = 1;

			const char *hit_verb = "hits";
			const struct slay *best_s_ptr = NULL;

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize distance travelled) */
			if (test_hit(chance2, r_ptr->ac, m_ptr->ml))
			{
				bool fear = FALSE;

				/* Assume a default death */
				cptr note_dies = " dies.";

				improve_attack_modifier(o_ptr, m_ptr, &best_s_ptr, TRUE,
						FALSE);
				improve_attack_modifier(j_ptr, m_ptr, &best_s_ptr, TRUE,
						FALSE);
				if (best_s_ptr != NULL)
					hit_verb = best_s_ptr->range_verb;

				/* Some monsters get "destroyed" */
				if (monster_is_unusual(r_ptr))
				{
					/* Special note at death */
					note_dies = " is destroyed.";
				}

				/* Calculate multiplier */
				multiplier = p_ptr->state.ammo_mult;
				if (best_s_ptr != NULL) multiplier += best_s_ptr->mult;

				/* Apply damage: multiplier, slays, criticals, bonuses */
				tdam = damroll(o_ptr->dd, o_ptr->ds);
				tdam += o_ptr->to_d + j_ptr->to_d;
				tdam *= multiplier;
				tdam = critical_shot(o_ptr->weight, o_ptr->to_h, tdam, &msg_type);

				object_notice_attack_plusses(o_ptr);
				object_notice_attack_plusses(&p_ptr->inventory[INVEN_BOW]);

				/* No negative damage; change verb if no damage done */
				if (tdam <= 0)
				{
					tdam = 0;
					hit_verb = "fail to harm";
				}

				/* Handle unseen monster */
				if (!visible)
				{
					/* Invisible monster */
					msgt(MSG_SHOOT_HIT, "The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else
				{
					char m_name[80];

					/* Get "the monster" or "it" */
					monster_desc(m_name, sizeof(m_name), m_ptr, 0);

					/* Tell the player what happened */
					if (msg_type == MSG_SHOOT_HIT)
						msgt(MSG_SHOOT_HIT, "The %s %s %s.", o_name, hit_verb, m_name);
					else
					{
						if (msg_type == MSG_HIT_GOOD)
						{
							msgt(MSG_HIT_GOOD, "The %s %s %s. %s", o_name, hit_verb, m_name,
										   "It was a good hit!");
						}
						else if (msg_type == MSG_HIT_GREAT)
						{
							msgt(MSG_HIT_GREAT, "The %s %s %s. %s", o_name, hit_verb, m_name,
										   "It was a great hit!");
						}
						else if (msg_type == MSG_HIT_SUPERB)
						{
							msgt(MSG_HIT_SUPERB, "The %s %s %s. %s", o_name, hit_verb, m_name,
										   "It was a superb hit!");
						}
					}
					/* Hack -- Track this monster race */
					if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

					/* Hack -- Track this monster */
					if (m_ptr->ml) health_track(p_ptr, cave->m_idx[y][x]);

				}

				/* Complex message */
				if (p_ptr->wizard)
				{
					msg("You do %d (out of %d) damage.",
					           tdam, m_ptr->hp);
				}

				/* Hit the monster, check for death */
				if (mon_take_hit(cave->m_idx[y][x], tdam, &fear, note_dies))
				{
					/* Dead monster */
				}

				/* No death */
				else
				{
					/* Message */
					message_pain(cave->m_idx[y][x], tdam);

					/* Take note */
					if (fear && m_ptr->ml)
					{
						char m_name[80];

						/* Get the monster name (or "it") */
						monster_desc(m_name, sizeof(m_name), m_ptr, 0);

						/* Message */
						msgt(MSG_FLEE, "%^s flees in terror!", m_name);
					}
				}
			}

			/* Stop looking */
			break;
		}
	}



	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain a local object */
	object_copy(i_ptr, o_ptr);

	/* Single object */
	i_ptr->number = 1;

	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Reduce and describe floor item */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_optimize(0 - item);
	}


	/* Chance of breakage (during attacks) */
	j = (hit_body ? breakage_chance(i_ptr) : 0);

	/* Drop (or break) near that location */
	drop_near(cave, i_ptr, j, y, x, TRUE);
}


void textui_cmd_fire_at_nearest(void)
{
	/* the direction '5' means 'use the target' */
	int i, dir = 5, item = -1;

	/* Require a usable launcher */
	if (!p_ptr->inventory[INVEN_BOW].tval || !p_ptr->state.ammo_tval)
	{
		msg("You have nothing to fire with.");
		return;
	}

	/* Find first eligible ammo in the quiver */
	for (i = QUIVER_START; i < QUIVER_END; i++)
	{
		if (p_ptr->inventory[i].tval != p_ptr->state.ammo_tval) continue;
		item = i;
		break;
	}

	/* Require usable ammo */
	if (item < 0)
	{
		msg("You have no ammunition in the quiver to fire");
		return;
	}

	/* Require foe */
	if (!target_set_closest(TARGET_KILL | TARGET_QUIET))
		return;

	/* Check for confusion */
	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg("You are confused.");
		dir = ddd[randint0(8)];
	}

	/* Fire! */
	cmd_insert(CMD_FIRE);
	cmd_set_arg_item(cmd_get_top(), 0, item);
	cmd_set_arg_target(cmd_get_top(), 1, dir);
}

/*
 * Throw an object from the pack or floor.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 */
void do_cmd_throw(cmd_code code, cmd_arg args[])
{
	int dir, item;
	int i, j, y, x;
	s16b ty, tx;
	int chance, tdam, tdis;
	int weight;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	bool hit_body = FALSE;

	byte missile_attr;
	char missile_char;

	char o_name[80];

	u32b msg_type = 0;

	int path_n;
	u16b path_g[256];

	int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	/* Get item to throw and direction in which to throw it. */
	item = args[0].item;
	dir = args[1].direction;

	/* Make sure the player isn't throwing wielded items */
	if (item >= INVEN_WIELD && item < QUIVER_START)
	{
		msg("You have cannot throw wielded items.");
		return;
	}

	/* Check the item being thrown is usable by the player. */
	if (!item_is_available(item, NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		msg("That item is not within your reach.");
		return;
	}

	/* Get the object */
	o_ptr = object_from_item_idx(item);
	object_notice_on_firing(o_ptr);

	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain a local object */
	object_copy(i_ptr, o_ptr);

	/* Distribute the charges of rods/wands/staves between the stacks */
	distribute_charges(o_ptr, i_ptr, 1);

	/* Single object */
	i_ptr->number = 1;

	/* Reduce and describe inventory */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Reduce and describe floor item */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_optimize(0 - item);
	}


	/* Description */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_FULL);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(i_ptr);
	missile_char = object_char(i_ptr);


	/* Enforce a minimum "weight" of one pound */
	weight = ((i_ptr->weight > 10) ? i_ptr->weight : 10);

	/* Hack -- Distance -- Reward strength, penalize weight */
	tdis = (adj_str_blow[p_ptr->state.stat_ind[A_STR]] + 20) * 10 / weight;

	/* Max distance of 10 */
	if (tdis > 10) tdis = 10;

	/* Hack -- Base damage from thrown object */
	tdam = damroll(i_ptr->dd, i_ptr->ds);
	if (!tdam) tdam = 1;
	tdam += i_ptr->to_d;

	/* Chance of hitting */
	chance = (p_ptr->state.skills[SKILL_TO_HIT_THROW] +
		(p_ptr->state.to_h * BTH_PLUS_ADJ));


	/* Take a turn */
	p_ptr->energy_use = 100;


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	ty = p_ptr->py + 99 * ddy[dir];
	tx = p_ptr->px + 99 * ddx[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_okay())
	{
		target_get(&tx, &ty);
	}

	/* Calculate the path */
	path_n = project_path(path_g, tdis, p_ptr->py, p_ptr->px, ty, tx, 0);


	/* Hack -- Handle stuff */
	handle_stuff();

	/* Project along the path */
	for (i = 0; i < path_n; ++i)
	{
		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		/* Hack -- Stop before hitting walls */
		if (!cave_floor_bold(ny, nx)) break;

		/* Advance */
		x = nx;
		y = ny;

		/* Only do visuals if the player can "see" the missile */
		if (player_can_see_bold(y, x))
		{
			/* Visual effects */
			print_rel(missile_char, missile_attr, y, x);
			move_cursor_relative(y, x);

			Term_fresh();
			if (p_ptr->redraw) redraw_stuff();

			Term_xtra(TERM_XTRA_DELAY, msec);
			cave_light_spot(cave, y, x);

			Term_fresh();
			if (p_ptr->redraw) redraw_stuff();
		}

		/* Delay anyway for consistency */
		else
		{
			/* Pause anyway, for consistancy */
			Term_xtra(TERM_XTRA_DELAY, msec);
		}

		/* Handle monster */
		if (cave->m_idx[y][x] > 0)
		{
			monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			int chance2 = chance - distance(p_ptr->py, p_ptr->px, y, x);

			int visible = m_ptr->ml;

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit(chance2, r_ptr->ac, m_ptr->ml))
			{
				const char *hit_verb = "hits";
				bool fear = FALSE;
				const struct slay *best_s_ptr = NULL;

				/* Assume a default death */
				cptr note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if (monster_is_unusual(r_ptr))
				{
					/* Special note at death */
					note_dies = " is destroyed.";
				}

				/* Apply special damage  - brought forward to fill in hit_verb XXX XXX XXX */
				improve_attack_modifier(i_ptr, m_ptr, &best_s_ptr, TRUE,
						FALSE);
				if (best_s_ptr != NULL)
				{
					tdam *= best_s_ptr->mult;
					hit_verb = best_s_ptr->range_verb;
				}
				/* Apply special damage XXX XXX XXX */
				tdam = critical_shot(i_ptr->weight, i_ptr->to_h, tdam, &msg_type);

				/* No negative damage; change verb if no damage done */
				if (tdam <= 0)
				{
					tdam = 0;
					hit_verb = "fail to harm";
				}

				/* Handle unseen monster */
				if (!visible)
				{
					/* Invisible monster */
					msg("The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else
				{
					char m_name[80];

					/* Get "the monster" or "it" */
					monster_desc(m_name, sizeof(m_name), m_ptr, 0);

					/* Tell the player what happened */
					if (msg_type == MSG_SHOOT_HIT)
						msgt(MSG_SHOOT_HIT, "The %s %s %s.", o_name, hit_verb, m_name);
					else
					{
						if (msg_type == MSG_HIT_GOOD)
						{
							msgt(MSG_HIT_GOOD, "The %s %s %s. %s", o_name, hit_verb, m_name,
										   "It was a good hit!");
						}
						else if (msg_type == MSG_HIT_GREAT)
						{
							msgt(MSG_HIT_GREAT, "The %s %s %s. %s", o_name, hit_verb, m_name,
										   "It was a great hit!");
						}
						else if (msg_type == MSG_HIT_SUPERB)
						{
							msgt(MSG_HIT_SUPERB, "The %s %s %s. %s", o_name, hit_verb, m_name,
										   "It was a superb hit!");
						}
					}
					/* Hack -- Track this monster race */
					if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

					/* Hack -- Track this monster */
					if (m_ptr->ml) health_track(p_ptr, cave->m_idx[y][x]);
				}

				/* Learn the bonuses */
				/* XXX Eddie This is messed up, better done for firing, */
				/* should use that method [split last] instead */
				/* check if inven_optimize removed what o_ptr referenced */
				if (object_similar(i_ptr, o_ptr, OSTACK_PACK))
					object_notice_attack_plusses(o_ptr);
				object_notice_attack_plusses(i_ptr);

				/* Complex message */
				if (p_ptr->wizard)
					msg("You do %d (out of %d) damage.",
							   tdam, m_ptr->hp);

				/* Hit the monster, check for death */
				if (mon_take_hit(cave->m_idx[y][x], tdam, &fear, note_dies))
				{
					/* Dead monster */
				}

				/* No death */
				else
				{
					/* Message */
					message_pain(cave->m_idx[y][x], tdam);

					/* Take note */
					if (fear && m_ptr->ml)
					{
						char m_name[80];

						/* Get the monster name (or "it") */
						monster_desc(m_name, sizeof(m_name), m_ptr, 0);

						/* Message */
						msgt(MSG_FLEE, "%^s flees in terror!", m_name);
					}
				}
			}

			/* Stop looking */
			break;
		}
	}

	/* Chance of breakage (during attacks) */
	j = (hit_body ? breakage_chance(i_ptr) : 0);

	/* Drop (or break) near that location */
	drop_near(cave, i_ptr, j, y, x, TRUE);
}

void textui_cmd_throw(void)
{
	int item, dir;
	cptr q, s;

	/* Get an item */
	q = "Throw which item? ";
	s = "You have nothing to throw.";
	if (!get_item(&item, q, s, CMD_THROW, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	if (item >= INVEN_WIELD && item < QUIVER_START)
	{
		msg("You cannot throw wielded items.");
		return;
	}

	/* Get a direction (or cancel) */
	if (!get_aim_dir(&dir)) return;

	cmd_insert(CMD_THROW);
	cmd_set_arg_item(cmd_get_top(), 0, item);
	cmd_set_arg_target(cmd_get_top(), 1, dir);
}
