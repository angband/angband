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

#include "attack.h"
#include "cave.h"
#include "cmds.h"
#include "monster/monster.h"
#include "object/slays.h"
#include "object/tvalsval.h"
#include "spells.h"
#include "target.h"

/**
 *  Returns percent chance of an object breaking after throwing or shooting.
 */
int breakage_chance(const object_type *o_ptr) {
	/* Artifacts never break */
	if (artifact_p(o_ptr)) return 0;

	return o_ptr->kind->base->break_perc;
}


/**
 * Determine if the player "hits" a monster.
 */
bool test_hit(int chance, int ac, int vis) {
	int k = randint0(100);

	/* Penalize invisible targets */
	if (!vis) chance /= 2;

	/* There is an automatic 5% chance to hit and to miss */
	if (k < 10) return k < 5;

	/* If there is no chance, then miss */
	if (chance <= 0) return FALSE;

	/* Power competes against armor */
	return randint0(chance) >= (ac * 3) / 4;
}


/**
 * Critical hits (from objects thrown by player)
 *
 * Factor in item weight, total plusses, and player level.
 */
static int critical_shot(int weight, int plus, int dam, u32b *msg_type) {
	int chance = weight + (p_ptr->state.to_h + plus) * 4 + p_ptr->lev * 2;
	int power = weight + randint1(500);

	if (randint1(5000) > chance) {
		*msg_type = MSG_SHOOT_HIT;
		return dam;

	} else if (power < 500) {
		*msg_type = MSG_HIT_GOOD;
		return 2 * dam + 5;

	} else if (power < 1000) {
		*msg_type = MSG_HIT_GREAT;
		return 2 * dam + 10;

	} else {
		*msg_type = MSG_HIT_SUPERB;
		return 3 * dam + 15;
	}
}


/**
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
static int critical_norm(int weight, int plus, int dam, u32b *msg_type) {
	int chance = (weight + ((p_ptr->state.to_h + plus) * 5) + (p_ptr->lev * 3));
	int power = weight + randint1(650);

	if (randint1(5000) > chance) {
		*msg_type = MSG_HIT;
		return dam;

	} else if (power < 400) {
		*msg_type = MSG_HIT_GOOD;
		return 2 * dam + 5;

	} else if (power < 700) {
		*msg_type = MSG_HIT_GREAT;
		return 2 * dam + 10;

	} else if (power < 900) {
		*msg_type = MSG_HIT_SUPERB;
		return 3 * dam + 15;

	} else if (power < 1300) {
		*msg_type = MSG_HIT_HI_GREAT;
		return 3 * dam + 20;

	} else {
		*msg_type = MSG_HIT_HI_SUPERB;
		return 4 * dam + 20;
	}
}


/**
 * Attack the monster at the given location
 *
 * If no "weapon" is available, then "punch" the monster one time.
 *
 * We get blows until energy drops below that required for another blow, or
 * until the target monster dies. We use a wrapper to work out the number of
 * blows. We don't allow @ to spend more than 100 energy in one go, to avoid
 * slower monsters getting double moves.
 */
bool py_attack_real(int y, int x) {
	/* Information about the target of the attack */
	monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	char m_name[80];
	bool dead = FALSE;
	bool fear = FALSE;

	/* The weapon used */
	object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Information about the attack */
	int bonus = p_ptr->state.to_h + o_ptr->to_h;
	int chance = p_ptr->state.skills[SKILL_TO_HIT_MELEE] + bonus * BTH_PLUS_ADJ;
	bool do_quake = FALSE;
	bool success = FALSE;

	/* Default to punching for one damage */
	const char *hit_verb = "punch";
	int dmg = 1;
	u32b msg_type = MSG_HIT;

	/* Extract monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Auto-Recall if possible and visible */
	if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

	/* Track a new monster */
	if (m_ptr->ml) health_track(p_ptr, cave->m_idx[y][x]);

	/* Handle player fear (only for invisible monsters) */
	if (p_ptr->state.flags[OF_AFRAID]) {
		msgt(MSG_AFRAID, "You are too afraid to attack %s!", m_name);
		return FALSE;
	}

	/* Disturb the monster */
	mon_clear_timed(cave->m_idx[y][x], MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE);

	/* See if the player hit */
	success = test_hit(chance, r_ptr->ac, m_ptr->ml);

	/* If a miss, skip this hit */
	if (!success) {
		msgt(MSG_MISS, "You miss %s.", m_name);
		return FALSE;
	}

	/* Handle normal weapon */
	if (o_ptr->kind) {
		int i;
		const struct slay *best_s_ptr = NULL;

		hit_verb = "hit";

		/* Get the best attack from all slays or
		 * brands on all non-launcher equipment */
		for (i = INVEN_LEFT; i < INVEN_TOTAL; i++) {
			struct object *obj = &p_ptr->inventory[i];
			if (obj->kind)
				improve_attack_modifier(obj, m_ptr, &best_s_ptr, TRUE, FALSE);
		}

		improve_attack_modifier(o_ptr, m_ptr, &best_s_ptr, TRUE, FALSE);
		if (best_s_ptr != NULL)
			hit_verb = best_s_ptr->melee_verb;

		dmg = damroll(o_ptr->dd, o_ptr->ds);
		dmg *= (best_s_ptr == NULL) ? 1 : best_s_ptr->mult;

		dmg += o_ptr->to_d;
		dmg = critical_norm(o_ptr->weight, o_ptr->to_h, dmg, &msg_type);

		/* Learn by use for the weapon */
		object_notice_attack_plusses(o_ptr);

		if (p_ptr->state.flags[OF_IMPACT] && dmg > 50) {
			do_quake = TRUE;
			wieldeds_notice_flag(OF_IMPACT);
		}
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
	if (p_ptr->confusing) {
		p_ptr->confusing = FALSE;
		msg("Your hands stop glowing.");

		mon_inc_timed(cave->m_idx[y][x], MON_TMD_CONF,
				(10 + randint0(p_ptr->lev) / 10), MON_TMD_FLG_NOTIFY);
	}

	/* Damage, check for fear and death */
	dead = mon_take_hit(cave->m_idx[y][x], dmg, &fear, NULL);

	/* Hack -- delay fear messages */
	if (fear && m_ptr->ml)
		msgt(MSG_FLEE, "%^s flees in terror!", m_name);

	/* Apply earthquake brand */
	if (do_quake) earthquake(p_ptr->py, p_ptr->px, 10);

	return dead;
}


/**
 * TODO: write me!
 */
void py_attack(int y, int x) {
	int blow_energy = 10000 / p_ptr->state.num_blows;
	int blows = 0;

	/* disturb the player */
	disturb(0,0);

	/* Initialize the energy used */
	p_ptr->energy_use = 0;

	/* Attack until energy runs out or enemy dies. We limit energy use to 100
	 * to avoid giving monsters a possible double move. */
	while (p_ptr->energy >= blow_energy * (blows + 1)) {
		bool stop = py_attack_real(y, x);
		p_ptr->energy_use += blow_energy;
		if (stop || p_ptr->energy_use + blow_energy > 100) break;
		blows++;
	}
}


/**
 * This is a helper function used by do_cmd_throw and do_cmd_fire.
 *
 * It abstracts out the projectile path, display code, identify and clean up
 * logic, while using the 'attack' parameter to do work particular to each
 * kind of attack.
 */
void ranged_helper(int item, int dir, int range, int shots, ranged_attack attack) {
	/* Get the ammo */
	object_type *o_ptr = object_from_item_idx(item);

	int i, j;
	byte missile_attr = object_attr(o_ptr);
	char missile_char = object_char(o_ptr);

	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	char o_name[80];

	int path_n;
	u16b path_g[256];

	int msec = op_ptr->delay_factor;

	/* Start at the player */
	int x = p_ptr->px;
	int y = p_ptr->py;

	/* Predict the "target" location */
	s16b ty = y + 99 * ddy[dir];
	s16b tx = x + 99 * ddx[dir];

	bool hit_target = FALSE;

	/* Check for target validity */
	if ((dir == 5) && target_okay()) {
		target_get(&tx, &ty);
		if (distance(y, x, ty, tx) > range) {
			if (!get_check("Target out of range.  Fire anyway?")) return;
		}
	}

	/* Sound */
	sound(MSG_SHOOT);

	object_notice_on_firing(o_ptr);

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL | ODESC_SINGULAR);

	/* Actually "fire" the object -- Take a partial turn */
	p_ptr->energy_use = (100 / shots);

	/* Calculate the path */
	path_n = project_path(path_g, range, y, x, ty, tx, 0);

	/* Hack -- Handle stuff */
	handle_stuff();

	/* Start at the player */
	x = p_ptr->px;
	y = p_ptr->py;

	/* Project along the path */
	for (i = 0; i < path_n; ++i) {
		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		/* Hack -- Stop before hitting walls */
		if (!cave_floor_bold(ny, nx)) break;

		/* Advance */
		x = nx;
		y = ny;

		/* Only do visuals if the player can "see" the missile */
		if (player_can_see_bold(y, x)) {
			print_rel(missile_char, missile_attr, y, x);
			move_cursor_relative(y, x);

			Term_fresh();
			if (p_ptr->redraw) redraw_stuff();

			Term_xtra(TERM_XTRA_DELAY, msec);
			cave_light_spot(cave, y, x);

			Term_fresh();
			if (p_ptr->redraw) redraw_stuff();
		} else {
			/* Delay anyway for consistency */
			Term_xtra(TERM_XTRA_DELAY, msec);
		}

		/* Handle monster */
		if (cave->m_idx[y][x] > 0) break;
	}

	/* Try the attack on the monster at (x, y) if any */
	if (cave->m_idx[y][x]) {
		monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];
		int visible = m_ptr->ml;

		bool fear = FALSE;
		char m_name[80];
		const char *note_dies = monster_is_unusual(r_ptr) ? " is destroyed." : " dies.";

		struct attack_result result = attack(o_ptr, y, x);
		int dmg = result.dmg;
		u32b msg_type = result.msg_type;
		const char *hit_verb = result.hit_verb;

		if (result.success) {
			hit_target = TRUE;

			/* Get "the monster" or "it" */
			monster_desc(m_name, sizeof(m_name), m_ptr, 0);
		
			object_notice_attack_plusses(o_ptr);
		
			/* No negative damage; change verb if no damage done */
			if (dmg <= 0) {
				dmg = 0;
				hit_verb = "fail to harm";
			}
		
			if (!visible) {
				/* Invisible monster */
				msgt(MSG_SHOOT_HIT, "The %s finds a mark.", o_name);
			} else {
				/* Visible monster */
				if (msg_type == MSG_SHOOT_HIT)
					msgt(MSG_SHOOT_HIT, "The %s %s %s.", o_name, hit_verb, m_name);
				else if (msg_type == MSG_HIT_GOOD) {
					msgt(MSG_HIT_GOOD, "The %s %s %s. %s", o_name, hit_verb, m_name, "It was a good hit!");
				} else if (msg_type == MSG_HIT_GREAT) {
					msgt(MSG_HIT_GREAT, "The %s %s %s. %s", o_name, hit_verb, m_name,
						 "It was a great hit!");
				} else if (msg_type == MSG_HIT_SUPERB) {
					msgt(MSG_HIT_SUPERB, "The %s %s %s. %s", o_name, hit_verb, m_name,
						 "It was a superb hit!");
				}
		
				/* Track this monster */
				if (m_ptr->ml) monster_race_track(m_ptr->r_idx);
				if (m_ptr->ml) health_track(p_ptr, cave->m_idx[y][x]);
			}
		
			/* Complex message */
			if (p_ptr->wizard)
				msg("You do %d (out of %d) damage.", dmg, m_ptr->hp);
		
			/* Hit the monster, check for death */
			if (mon_take_hit(cave->m_idx[y][x], dmg, &fear, note_dies)) {
				/* Dead monster */
			} else {
				message_pain(cave->m_idx[y][x], dmg);
				if (fear && m_ptr->ml) msgt(MSG_FLEE, "%^s flees in terror!", m_name);
			}
		}
	}

	/* Obtain a local object */
	object_copy(i_ptr, o_ptr);

	/* Single object */
	i_ptr->number = 1;

	/* See if the ammunition broke or not */
	j = (hit_target ? breakage_chance(i_ptr) : 0);

	/* Drop (or break) near that location */
	drop_near(cave, i_ptr, j, y, x, TRUE);

	if (item >= 0) {
		/* The ammo is from the inventory */
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	} else {
		/* The ammo is from the floor */
		floor_item_increase(0 - item, -1);
		floor_item_optimize(0 - item);
	}
}


/**
 * Helper function used with ranged_helper by do_cmd_fire.
 */
struct attack_result make_ranged_shot(object_type *o_ptr, int y, int x) {
	struct attack_result result = {FALSE, 0, 0, "hit"};

	object_type *j_ptr = &p_ptr->inventory[INVEN_BOW];

	monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int bonus = p_ptr->state.to_h + o_ptr->to_h + j_ptr->to_h;
	int chance = p_ptr->state.skills[SKILL_TO_HIT_BOW] + bonus * BTH_PLUS_ADJ;
	int chance2 = chance - distance(p_ptr->py, p_ptr->px, y, x);

	int multiplier = p_ptr->state.ammo_mult;
	const struct slay *best_s_ptr = NULL;

	/* Did we hit it (penalize distance travelled) */
	if (!test_hit(chance2, r_ptr->ac, m_ptr->ml)) return result;

	result.success = TRUE;

	improve_attack_modifier(o_ptr, m_ptr, &best_s_ptr, TRUE, FALSE);
	improve_attack_modifier(j_ptr, m_ptr, &best_s_ptr, TRUE, FALSE);

	/* If we have a slay, modify the multiplier appropriately */
	if (best_s_ptr != NULL) {
		result.hit_verb = best_s_ptr->range_verb;
		multiplier += best_s_ptr->mult;
	}

	/* Apply damage: multiplier, slays, criticals, bonuses */
	result.dmg = damroll(o_ptr->dd, o_ptr->ds);
	result.dmg += o_ptr->to_d + j_ptr->to_d;
	result.dmg *= multiplier;
	result.dmg = critical_shot(o_ptr->weight, o_ptr->to_h, result.dmg, &result.msg_type);

	object_notice_attack_plusses(&p_ptr->inventory[INVEN_BOW]);

	return result;
}


/**
 * Helper function used with ranged_helper by do_cmd_throw.
 */
struct attack_result make_ranged_throw(object_type *o_ptr, int y, int x) {
	struct attack_result result = {FALSE, 0, 0, "hit"};

	monster_type *m_ptr = &mon_list[cave->m_idx[y][x]];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int bonus = p_ptr->state.to_h + o_ptr->to_h;
	int chance = p_ptr->state.skills[SKILL_TO_HIT_THROW] + bonus * BTH_PLUS_ADJ;
	int chance2 = chance - distance(p_ptr->py, p_ptr->px, y, x);

	int multiplier = 1;
	const struct slay *best_s_ptr = NULL;

	/* If we missed then we're done */
	if (!test_hit(chance2, r_ptr->ac, m_ptr->ml)) return result;

	result.success = TRUE;

	improve_attack_modifier(o_ptr, m_ptr, &best_s_ptr, TRUE, FALSE);

	/* If we have a slay, modify the multiplier appropriately */
	if (best_s_ptr != NULL) {
		result.hit_verb = best_s_ptr->range_verb;
		multiplier += best_s_ptr->mult;
	}

	/* Apply damage: multiplier, slays, criticals, bonuses */
	result.dmg = damroll(o_ptr->dd, o_ptr->ds);
	result.dmg += o_ptr->to_d;
	result.dmg *= multiplier;
	result.dmg = critical_norm(o_ptr->weight, o_ptr->to_h, result.dmg, &result.msg_type);

	return result;
}


/**
 * Fire an object from the quiver, pack or floor at a target.
 */
void do_cmd_fire(cmd_code code, cmd_arg args[]) {
	int item = args[0].item;
	int dir = args[1].direction;
	int range = 6 + 2 * p_ptr->state.ammo_mult;
	int shots = p_ptr->state.num_shots;

	ranged_attack attack = make_ranged_shot;

	object_type *j_ptr = &p_ptr->inventory[INVEN_BOW];
	object_type *o_ptr = object_from_item_idx(item);

	/* Require a usable launcher */
	if (!j_ptr->tval || !p_ptr->state.ammo_tval) {
		msg("You have nothing to fire with.");
		return;
	}

	/* Check the item being fired is usable by the player. */
	if (!item_is_available(item, NULL, USE_EQUIP | USE_INVEN | USE_FLOOR)) {
		msg("That item is not within your reach.");
		return;
	}

	/* Check the ammo can be used with the launcher */
	if (o_ptr->tval != p_ptr->state.ammo_tval) {
		msg("That ammo cannot be fired by your current weapon.");
		return;
	}

	ranged_helper(item, dir, range, shots, attack);
}


/**
 * Throw an object from the quiver, pack or floor.
 */
void do_cmd_throw(cmd_code code, cmd_arg args[]) {
	int item = args[0].item;
	int dir = args[1].direction;
	int shots = 1;

	object_type *o_ptr = object_from_item_idx(item);
	int weight = MAX(o_ptr->weight, 10);
	int str = adj_str_blow[p_ptr->state.stat_ind[A_STR]];
	int range = MIN(((str + 20) * 10) / weight, 10);

	ranged_attack attack = make_ranged_throw;

	/* Make sure the player isn't throwing wielded items */
	if (item >= INVEN_WIELD && item < QUIVER_START) {
		msg("You have cannot throw wielded items.");
		return;
	}

	/* Check the item being thrown is usable by the player. */
	if (!item_is_available(item, NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR))) {
		msg("That item is not within your reach.");
		return;
	}

	ranged_helper(item, dir, range, shots, attack);
}


/**
 * Front-end 'throw' command.
 */
void textui_cmd_throw(void) {
	int item, dir;
	const char *q, *s;

	/* Get an item */
	q = "Throw which item? ";
	s = "You have nothing to throw.";
	if (!get_item(&item, q, s, CMD_THROW, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	if (item >= INVEN_WIELD && item < QUIVER_START) {
		msg("You cannot throw wielded items.");
		return;
	}

	/* Get a direction (or cancel) */
	if (!get_aim_dir(&dir)) return;

	cmd_insert(CMD_THROW);
	cmd_set_arg_item(cmd_get_top(), 0, item);
	cmd_set_arg_target(cmd_get_top(), 1, dir);
}


/**
 * Front-end command which fires at the nearest target with default ammo.
 */
void textui_cmd_fire_at_nearest(void) {
	/* the direction '5' means 'use the target' */
	int i, dir = 5, item = -1;

	/* Require a usable launcher */
	if (!p_ptr->inventory[INVEN_BOW].tval || !p_ptr->state.ammo_tval) {
		msg("You have nothing to fire with.");
		return;
	}

	/* Find first eligible ammo in the quiver */
	for (i = QUIVER_START; i < QUIVER_END; i++) {
		if (p_ptr->inventory[i].tval != p_ptr->state.ammo_tval) continue;
		item = i;
		break;
	}

	/* Require usable ammo */
	if (item < 0) {
		msg("You have no ammunition in the quiver to fire");
		return;
	}

	/* Require foe */
	if (!target_set_closest(TARGET_KILL | TARGET_QUIET)) return;

	/* Check for confusion */
	if (p_ptr->timed[TMD_CONFUSED]) {
		msg("You are confused.");
		dir = ddd[randint0(8)];
	}

	/* Fire! */
	cmd_insert(CMD_FIRE);
	cmd_set_arg_item(cmd_get_top(), 0, item);
	cmd_set_arg_target(cmd_get_top(), 1, dir);
}
