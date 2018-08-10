/**
 * \file player-attack.c
 * \brief Attacks (both throwing and melee) by the player
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
#include "effects.h"
#include "game-event.h"
#include "game-input.h"
#include "generate.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-msg.h"
#include "mon-predicate.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"

/**
 * Returns percent chance of an object breaking after throwing or shooting.
 *
 * Artifacts will never break.
 *
 * Beyond that, each item kind has a percent chance to break (0-100). When the
 * object hits its target this chance is used.
 *
 * When an object misses it also has a chance to break. This is determined by
 * squaring the normaly breakage probability. So an item that breaks 100% of
 * the time on hit will also break 100% of the time on a miss, whereas a 50%
 * hit-breakage chance gives a 25% miss-breakage chance, and a 10% hit breakage
 * chance gives a 1% miss-breakage chance.
 */
int breakage_chance(const struct object *obj, bool hit_target) {
	int perc = obj->kind->base->break_perc;

	if (obj->artifact) return 0;
	if (!hit_target) return (perc * perc) / 100;
	return perc;
}

static int chance_of_missile_hit(const struct player *p,
		const struct object *missile,
		const struct object *launcher,
		int y,
		int x)
{
	bool throw = (launcher ? false : true);
	int bonus = p->state.to_h + missile->to_h;
	int chance;

	if (throw) {
		chance = p->state.skills[SKILL_TO_HIT_THROW] + bonus * BTH_PLUS_ADJ;
	} else {
		bonus += launcher->to_h;
		chance = p->state.skills[SKILL_TO_HIT_BOW] + bonus * BTH_PLUS_ADJ;
	}

	return chance - distance(loc(p->px, p->py), loc(x, y));
}

/**
 * Determine if the player "hits" a monster.
 */
bool test_hit(int chance, int ac, int vis) {
	int k = randint0(100);

	/* There is an automatic 12% chance to hit,
	 * and 5% chance to miss.
	 */
	if (k < 17) return k < 12;

	/* Penalize invisible targets */
	if (!vis) chance /= 2;

	/* Starting a bit higher up on the scale */
	if (chance < 9) chance = 9;

	/* Power competes against armor */
	return randint0(chance) >= (ac * 2 / 3);
}


/**
 * Determine standard melee damage.
 *
 * Factor in damage dice, to-dam and any brand or slay.
 */
static int melee_damage(struct object *obj, int b, int s)
{
	int dmg = damroll(obj->dd, obj->ds);

	if (s) {
		dmg *= slays[s].multiplier;
	} else if (b) {
		dmg *= brands[b].multiplier;
	}

	dmg += obj->to_d;

	return dmg;
}

/**
 * Determine standard ranged damage.
 *
 * Factor in damage dice, to-dam, multiplier and any brand or slay.
 */
static int ranged_damage(struct object *missile, struct object *launcher, 
						 int b, int s, int mult)
{
	int dam;

	/* If we have a slay or brand , modify the multiplier appropriately */
	if (b) {
		mult += brands[b].multiplier;
	} else if (s) {
		mult += slays[s].multiplier;
	}

	/* Apply damage: multiplier, slays, bonuses */
	dam = damroll(missile->dd, missile->ds);
	dam += missile->to_d;
	if (launcher) {
		dam += launcher->to_d;
	}
	dam *= mult;

	return dam;
}

/**
 * Check if a monster is debuffed in such a way as to make a critical
 * hit more likely.
 */
static bool is_debuffed(const struct monster *monster)
{
	return monster->m_timed[MON_TMD_CONF] > 0 ||
			monster->m_timed[MON_TMD_HOLD] > 0 ||
			monster->m_timed[MON_TMD_STUN] > 0;
}

/**
 * Determine damage for critical hits from shooting.
 *
 * Factor in item weight, total plusses, and player level.
 */
static int critical_shot(const struct player *p,
		const struct monster *monster,
		int weight, int plus,
		int dam, u32b *msg_type)
{
	int debuff_to_hit = is_debuffed(monster) ? DEBUFF_CRITICAL_HIT : 0;
	int chance = weight + (p->state.to_h + plus + debuff_to_hit) * 4 + p->lev * 2;
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
 * Determine damage for critical hits from melee.
 *
 * Factor in weapon weight, total plusses, player level.
 */
static int critical_norm(const struct player *p,
		const struct monster *monster,
		int weight, int plus,
		int dam, u32b *msg_type)
{
	int debuff_to_hit = is_debuffed(monster) ? DEBUFF_CRITICAL_HIT : 0;
	int power = weight + randint1(650);
	int chance = weight + (p->state.to_h + plus + debuff_to_hit) * 5 + p->lev * 3;

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
 * Apply the player damage bonuses
 */
static int player_damage_bonus(struct player_state *state)
{
	return state->to_d;
}

/**
 * Apply blow side effects
 */
static void blow_side_effects(struct player *p, struct monster *mon)
{
	/* Confusion attack */
	if (p->timed[TMD_ATT_CONF]) {
		player_clear_timed(p, TMD_ATT_CONF, true);

		mon_inc_timed(mon, MON_TMD_CONF, (10 + randint0(p->lev) / 10),
					  MON_TMD_FLG_NOTIFY, false);
	}
}

/**
 * Apply knock back from powerful blows
 */
static bool blow_knock_back(struct player *p, struct monster *mon, int dmg,
							bool *fear)
{
	int y = mon->fy;
	int x = mon->fx;
	int dy = y - p->py;
	int dx = x - p->px;
	int power = (p->state.num_blows - 100) / 100;

	/* Not enough power left */
	if (!power) return false;

	/* Forced backwards until power runs out */
	while (power > 0) {
		/* Move back a square */
		y += dy;
		x += dx;

		/* React differently depending on the terrain behind the monster */
		if (square_ispassable(cave, y, x)) {
			/* Monster there - current monster takes all the damage
			 * Note we could have more fun here by pushing it back... */
			if (square_monster(cave, y, x)) {
				if (mon_take_hit(mon, dmg * power, fear, NULL)) return true;
				break;
			} else {
				/* Push back a square */
				monster_swap(mon->fy, mon->fx, y, x);
				power--;
			}
		} else {
			bool moved = false;

			/* Deal with impassable terrain */
			if (square_isdoor(cave, y, x)) {
				if (power >= 1) {
					square_open_door(cave, y, x);
					monster_swap(mon->fy, mon->fx, y, x);
					if (mon_take_hit(mon, dmg, fear, NULL)) return true;
					power--;
					moved = true;
				}
			} else if (square_isrubble(cave, y, x)) {
				if (power >= 1) {
					square_destroy_wall(cave, y, x);
					monster_swap(mon->fy, mon->fx, y, x);
					if (mon_take_hit(mon, dmg, fear, NULL)) return true;
					power--;
					moved = true;
				}
			} else if (square_ismagma(cave, y, x)) {
				if (power >= 1) {
					square_destroy_wall(cave, y, x);
					monster_swap(mon->fy, mon->fx, y, x);
					if (square_hasgoldvein(cave, y, x)) {
						place_gold(cave, y, x, p->depth, ORIGIN_FLOOR);
					}
					if (randint0(20) < power) {
						effect_simple(EF_EARTHQUAKE,
									  source_monster(mon->midx), "0",
									  0, 3, 0, 0, 0, NULL);
					}
					if (mon_take_hit(mon, dmg, fear, NULL)) return true;
					power--;
					moved = true;
				}
			} else if (square_isquartz(cave, y, x)) {
				if (power >= 2) {
					square_destroy_wall(cave, y, x);
					monster_swap(mon->fy, mon->fx, y, x);
					if (square_hasgoldvein(cave, y, x)) {
						place_gold(cave, y, x, p->depth, ORIGIN_FLOOR);
					}
					if (randint0(20) < power) {
						effect_simple(EF_EARTHQUAKE,
									  source_monster(mon->midx), "0",
									  0, 3, 0, 0, 0, NULL);
					}
					if (mon_take_hit(mon, dmg * 2, fear, NULL)) return true;
					power -= 2;
					moved = true;
				}
			} else if (square_isgranite(cave, y, x)) {
				if (power >= 3) {
					square_destroy_wall(cave, y, x);
					monster_swap(mon->fy, mon->fx, y, x);
					if (randint0(20) < power) {
						effect_simple(EF_EARTHQUAKE,
									  source_monster(mon->midx), "0",
									  0, 3, 0, 0, 0, NULL);
					}
					if (mon_take_hit(mon, dmg * 3, fear, NULL)) return true;
					power -= 3;
					moved = true;
				}
			}
			if (!moved) {
				/* Monster can't move, give it the remaining damage */
				if (mon_take_hit(mon, dmg * power, fear, NULL)) return true;
				break;
			}
		}
	}
	/* Player needs to stop hitting if the monster has moved */
	return (ABS(mon->fy - p->py) > 1) || (ABS(mon->fx - p->px) > 1);
}

/**
 * Apply blow after effects
 */
static bool blow_after_effects(int y, int x, bool quake)
{
	/* Apply earthquake brand */
	if (quake) {
		effect_simple(EF_EARTHQUAKE, source_player(), "0", 0, 10, 0, 0, 0, NULL);

		/* Monster may be dead or moved */
		if (!square_monster(cave, y, x))
			return true;
	}

	return false;
}

/* Melee and throwing hit types */
static const struct hit_types melee_hit_types[] = {
	{ MSG_MISS, NULL },
	{ MSG_HIT, NULL },
	{ MSG_HIT_GOOD, "It was a good hit!" },
	{ MSG_HIT_GREAT, "It was a great hit!" },
	{ MSG_HIT_SUPERB, "It was a superb hit!" },
	{ MSG_HIT_HI_GREAT, "It was a *GREAT* hit!" },
	{ MSG_HIT_HI_SUPERB, "It was a *SUPERB* hit!" },
};

/**
 * Return the player's chance to hit with a particular weapon.
 */
int py_attack_hit_chance(const struct player *p, const struct object *weapon)
{
	int chance, bonus = p->state.to_h;

	if (weapon)
		bonus += weapon->to_h;
	chance = p->state.skills[SKILL_TO_HIT_MELEE] + bonus * BTH_PLUS_ADJ;
	return chance;
}

/**
 * Attack the monster at the given location with a single blow.
 */
static bool py_attack_real(struct player *p, int y, int x, bool *fear)
{
	size_t i;

	/* Information about the target of the attack */
	struct monster *mon = square_monster(cave, y, x);
	char m_name[80];
	bool stop = false;

	/* The weapon used */
	struct object *obj = equipped_item_by_slot_name(p, "weapon");

	/* Information about the attack */
	int chance = py_attack_hit_chance(p, obj);
	int drain = 0;
	bool do_quake = false;
	bool success = false;

	/* Default to punching for one damage */
	char verb[20];
	int dmg = 1;
	u32b msg_type = MSG_HIT;

	/* Default to punching for one damage */
	my_strcpy(verb, "punch", sizeof(verb));

	/* Extract monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_TARG);

	/* Auto-Recall and track if possible and visible */
	if (monster_is_visible(mon)) {
		monster_race_track(p->upkeep, mon->race);
		health_track(p->upkeep, mon);
	}

	/* Handle player fear (only for invisible monsters) */
	if (player_of_has(p, OF_AFRAID)) {
		equip_learn_flag(p, OF_AFRAID);
		msgt(MSG_AFRAID, "You are too afraid to attack %s!", m_name);
		return false;
	}

	/* Disturb the monster */
	mon_clear_timed(mon, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, false);
	mon_clear_timed(mon, MON_TMD_HOLD, MON_TMD_FLG_NOTIFY, false);

	/* See if the player hit */
	success = test_hit(chance, mon->race->ac, monster_is_visible(mon));

	/* If a miss, skip this hit */
	if (!success) {
		msgt(MSG_MISS, "You miss %s.", m_name);

		/* Small chance of bloodlust side-effects */
		if (p->timed[TMD_BLOODLUST] && one_in_(50)) {
			msg("You feel strange...");
			player_over_exert(p, PY_EXERT_SCRAMBLE, 20, 20);
		}

		return false;
	}

	/* Handle normal weapon */
	if (obj) {
		int j;
		int b = 0, s = 0;
		int weight = obj->weight * (p->timed[TMD_POWERBLOW] ? 2 : 1);

		my_strcpy(verb, "hit", sizeof(verb));

		/* Best attack from all slays or brands on all non-launcher equipment */
		for (j = 2; j < p->body.count; j++) {
			struct object *obj_local = slot_object(p, j);
			if (obj_local)
				improve_attack_modifier(obj_local, mon, &b, &s, verb, false);
		}

		/* Get the best attack from all slays or brands - weapon or temporary */
		improve_attack_modifier(obj, mon, &b, &s, verb, false);
		improve_attack_modifier(NULL, mon, &b, &s, verb, false);

		dmg = melee_damage(obj, b, s);
		dmg = critical_norm(p, mon, weight, obj->to_h, dmg, &msg_type);

		if (player_of_has(p, OF_IMPACT) && dmg > 50) {
			do_quake = true;
			equip_learn_flag(p, OF_IMPACT);
		}
	}

	/* Learn by use */
	equip_learn_on_melee_attack(p);

	/* Apply the player damage bonuses */
	dmg += player_damage_bonus(&p->state);

	/* Substitute shape-specific blows for shapechanged players */
	if (player_is_shapechanged(p)) {
		int choice = randint0(p->shape->num_blows);
		struct player_blow *blow = p->shape->blows;
		while (choice--) {
			blow = blow->next;
		}
		my_strcpy(verb, blow->name, sizeof(verb));
	}

	/* No negative damage; change verb if no damage done */
	if (dmg <= 0) {
		dmg = 0;
		msg_type = MSG_MISS;
		my_strcpy(verb, "fail to harm", sizeof(verb));
	}

	for (i = 0; i < N_ELEMENTS(melee_hit_types); i++) {
		const char *dmg_text = "";

		if (msg_type != melee_hit_types[i].msg_type)
			continue;

		if (OPT(p, show_damage))
			dmg_text = format(" (%d)", dmg);

		if (melee_hit_types[i].text)
			msgt(msg_type, "You %s %s%s. %s", verb, m_name, dmg_text,
					melee_hit_types[i].text);
		else
			msgt(msg_type, "You %s %s%s.", verb, m_name, dmg_text);
	}

	/* Pre-damage side effects */
	blow_side_effects(p, mon);

	/* Damage, check for hp drain, knockback, fear and death */
	drain = MIN(mon->hp, dmg);
	stop = mon_take_hit(mon, dmg, fear, NULL);

	/* Small chance of bloodlust side-effects */
	if (p->timed[TMD_BLOODLUST] && one_in_(50)) {
		msg("You feel something give way!");
		player_over_exert(p, PY_EXERT_CON, 20, 0);
	}

	if (!stop) {
		if (p->timed[TMD_ATT_VAMP] && monster_is_living(mon)) {
			effect_simple(EF_HEAL_HP, source_player(), format("%d", drain),
						  0, 0, 0, 0, 0, NULL);
		} else if (p->timed[TMD_POWERBLOW]) {
			stop = blow_knock_back(p, mon, dmg, fear);

			/* Small chance of side-effects */
			if (one_in_(50)) {
				msg("You swing around wildly!");
				player_over_exert(p, PY_EXERT_CONF, 40, 10);
			}

			player_clear_timed(p, TMD_POWERBLOW, true);
		}
	}

	if (stop)
		(*fear) = false;

	/* Post-damage effects */
	if (blow_after_effects(y, x, do_quake))
		stop = true;

	return stop;
}


/**
 * Attempt a shield bash; return true if the monster dies
 */
bool attempt_shield_bash(struct player *p, struct monster *mon, bool *fear,
						 int *blows)
{
	struct object *weapon = slot_object(p, slot_by_name(p, "weapon"));
	struct object *shield = slot_object(p, slot_by_name(p, "arm"));
	int bash_quality, bash_dam;

	/* Bashing chance depends on melee skill, DEX, and a level bonus. */
	int bash_chance = p->state.skills[SKILL_TO_HIT_MELEE] / 4 +
		adj_dex_th[p->state.stat_ind[STAT_DEX]];

	/* No shield, no bash */
	if (!shield) return false;

	/* Players bash more often when they see a real need: */
	if (!equipped_item_by_slot_name(p, "weapon")) {
		/* Unarmed... */
		bash_chance *= 4;
	} else if (weapon->dd * weapon->ds * (*blows) <
			   shield->dd * shield->ds * 3) {
		/* ... or armed with a puny weapon */
		bash_chance *= 2;
	}

	/* Try to get in a shield bash. */
	if (bash_chance > randint0(200 + mon->race->level)) {
		msgt(MSG_HIT, "You get in a shield bash!");

		/* Calculate attack quality, a mix of momentum and accuracy. */
		bash_quality = p->state.skills[SKILL_TO_HIT_MELEE] / 4 + p->wt / 8 +
			p->upkeep->total_weight / 80 + shield->weight / 2;

		/* Calculate damage.  Big shields are deadly. */
		bash_dam = damroll(shield->dd, shield->ds);

		/* Multiply by quality and experience factors */
		bash_dam *= bash_quality / 40 + p->lev / 14;

		/* Strength bonus. */
		bash_dam += adj_str_td[p->state.stat_ind[STAT_STR]];

		/* Paranoia. */
		bash_dam = MIN(bash_dam, 125);

		/* Encourage the player to keep wearing that heavy shield. */
		if (randint1(bash_dam) > 30 + randint1(bash_dam / 2)) {
			msgt(MSG_HIT_HI_SUPERB, "WHAMM!");
		}

		/* Damage, check for fear and death. */
		if (mon_take_hit(mon, bash_dam, fear, NULL)) return true;

		/* Stunning. */
		if (bash_quality + p->lev > randint1(200 + mon->race->level * 8)) {
			mon_inc_timed(mon, MON_TMD_STUN, randint0(p->lev / 5) + 4, 0,
						  false);
		}

		/* Confusion. */
		if (bash_quality + p->lev > randint1(300 + mon->race->level * 12)) {
			mon_inc_timed(mon, MON_TMD_CONF, randint0(p->lev / 5) + 4, 0,
						  false);
		}

		/* The player will sometimes stumble. */
		if (35 + adj_dex_th[p->state.stat_ind[STAT_DEX]] < randint1(60)) {
			*blows += randint1(p->state.num_blows);
			msgt(MSG_GENERIC, "You stumble!");
		}
	}
	return false;
}

/**
 * Attack the monster at the given location
 *
 * We get blows until energy drops below that required for another blow, or
 * until the target monster dies. Each blow is handled by py_attack_real().
 * We don't allow @ to spend more than 100 energy in one go, to avoid slower
 * monsters getting double moves.
 */
void py_attack(struct player *p, int y, int x)
{
	int blow_energy = 100 * z_info->move_energy / p->state.num_blows;
	int blows = 0;
	bool fear = false;
	struct monster *mon = square_monster(cave, y, x);

	/* Disturb the player */
	disturb(p, 0);

	/* Initialize the energy used */
	p->upkeep->energy_use = 0;

	/* Player attempts a shield bash if they can, and if monster is visible
	 * and not too pathetic */
	if (player_has(p, PF_SHIELD_BASH) && monster_is_visible(mon) &&
		(mon->race->level > p->lev / 2)) {
		if (attempt_shield_bash(p, mon, &fear, &blows)) return;
	}

	/* Attack until energy runs out or enemy dies. We limit energy use to 100
	 * to avoid giving monsters a possible double move. */
	while (p->energy >= blow_energy * (blows + 1)) {
		bool stop = py_attack_real(player, y, x, &fear);
		p->upkeep->energy_use += blow_energy;
		if (p->upkeep->energy_use + blow_energy > z_info->move_energy ||
			stop) break;
		blows++;
	}

	/* Hack - delay fear messages */
	if (fear && monster_is_visible(mon)) {
		add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
	}
}

/* Shooting hit types */
static const struct hit_types ranged_hit_types[] = {
	{ MSG_MISS, NULL },
	{ MSG_SHOOT_HIT, NULL },
	{ MSG_HIT_GOOD, "It was a good hit!" },
	{ MSG_HIT_GREAT, "It was a great hit!" },
	{ MSG_HIT_SUPERB, "It was a superb hit!" }
};

/**
 * This is a helper function used by do_cmd_throw and do_cmd_fire.
 *
 * It abstracts out the projectile path, display code, identify and clean up
 * logic, while using the 'attack' parameter to do work particular to each
 * kind of attack.
 */
static void ranged_helper(struct player *p,	struct object *obj, int dir,
						  int range, int shots, ranged_attack attack,
						  const struct hit_types *hit_types, int num_types)
{
	int i, j;

	char o_name[80];

	int path_n;
	struct loc path_g[256];

	/* Start at the player */
	int x = p->px;
	int y = p->py;

	/* Predict the "target" location */
	int ty = y + 99 * ddy[dir];
	int tx = x + 99 * ddx[dir];

	bool hit_target = false;
	bool none_left = false;

	struct object *missile;
	int pierce = 1;

	/* Check for target validity */
	if ((dir == DIR_TARGET) && target_okay()) {
		int taim;
		target_get(&tx, &ty);
		taim = distance(loc(x, y), loc(tx, ty));
		if (taim > range) {
			char msg[80];
			strnfmt(msg, sizeof(msg),
					"Target out of range by %d squares. Fire anyway? ",
				taim - range);
			if (!get_check(msg)) return;
		}
	}

	/* Sound */
	sound(MSG_SHOOT);

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL | ODESC_SINGULAR);

	/* Actually "fire" the object -- Take a partial turn */
	p->upkeep->energy_use = (z_info->move_energy * 10 / shots);

	/* Calculate the path */
	path_n = project_path(path_g, range, y, x, ty, tx, 0);

	/* Calculate potenital piercing */
	if (player->timed[TMD_POWERSHOT] && tval_is_sharp_missile(obj)) {
		pierce = player->state.ammo_mult;
	}

	/* Hack -- Handle stuff */
	handle_stuff(p);

	/* Start at the player */
	x = p->px;
	y = p->py;

	/* Project along the path */
	for (i = 0; i < path_n; ++i) {
		struct monster *mon = NULL;
		int ny = path_g[i].y;
		int nx = path_g[i].x;
		bool see = square_isseen(cave, ny, nx);

		/* Stop before hitting walls */
		if (!(square_ispassable(cave, ny, nx)) &&
			!(square_isprojectable(cave, ny, nx)))
			break;

		/* Advance */
		x = nx;
		y = ny;

		/* Tell the UI to display the missile */
		event_signal_missile(EVENT_MISSILE, obj, see, y, x);

		/* Try the attack on the monster at (x, y) if any */
		mon = square_monster(cave, y, x);
		if (mon) {
			int visible = monster_is_visible(mon);

			bool fear = false;
			const char *note_dies = monster_is_destroyed(mon) ? 
				" is destroyed." : " dies.";

			struct attack_result result = attack(p, obj, y, x);
			int dmg = result.dmg;
			u32b msg_type = result.msg_type;
			char hit_verb[20];
			my_strcpy(hit_verb, result.hit_verb, sizeof(hit_verb));
			mem_free(result.hit_verb);

			if (result.success) {
				hit_target = true;

				missile_learn_on_ranged_attack(p, obj);

				/* Learn by use for other equipped items */
				equip_learn_on_ranged_attack(p);

				/* No negative damage; change verb if no damage done */
				if (dmg <= 0) {
					dmg = 0;
					msg_type = MSG_MISS;
					my_strcpy(hit_verb, "fails to harm", sizeof(hit_verb));
				}

				if (!visible) {
					/* Invisible monster */
					msgt(MSG_SHOOT_HIT, "The %s finds a mark.", o_name);
				} else {
					for (j = 0; j < num_types; j++) {
						char m_name[80];
						const char *dmg_text = "";

						if (msg_type != hit_types[j].msg_type) {
							continue;
						}

						if (OPT(p, show_damage)) {
							dmg_text = format(" (%d)", dmg);
						}

						monster_desc(m_name, sizeof(m_name), mon, MDESC_OBJE);

						if (hit_types[j].text) {
							msgt(msg_type, "Your %s %s %s%s. %s", o_name, 
								 hit_verb, m_name, dmg_text, hit_types[j].text);
						} else {
							msgt(msg_type, "Your %s %s %s%s.", o_name, hit_verb,
								 m_name, dmg_text);
						}
					}

					/* Track this monster */
					if (monster_is_visible(mon)) {
						monster_race_track(p->upkeep, mon->race);
						health_track(p->upkeep, mon);
					}
				}

				/* Hit the monster, check for death */
				if (!mon_take_hit(mon, dmg, &fear, note_dies)) {
					message_pain(mon, dmg);
					if (fear && monster_is_visible(mon)) {
						add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
					}
				}
			}
			/* Stop the missile, or reduce its piercing effect */
			pierce--;
			if (pierce) continue;
			else break;
		}

		/* Stop if non-projectable but passable */
		if (!(square_isprojectable(cave, ny, nx))) 
			break;
	}

	/* Get the missile */
	if (object_is_carried(p, obj))
		missile = gear_object_for_use(obj, 1, true, &none_left);
	else
		missile = floor_object_for_use(obj, 1, true, &none_left);

	/* Terminate piercing */
	if (p->timed[TMD_POWERSHOT]) {
		player_clear_timed(p, TMD_POWERSHOT, true);
	}

	/* Drop (or break) near that location */
	drop_near(cave, &missile, breakage_chance(missile, hit_target), y, x, true);
}


/**
 * Helper function used with ranged_helper by do_cmd_fire.
 */
static struct attack_result make_ranged_shot(struct player *p,
		struct object *ammo, int y, int x)
{
	char *hit_verb = mem_alloc(20 * sizeof(char));
	struct attack_result result = {false, 0, 0, hit_verb};
	struct object *bow = equipped_item_by_slot_name(p, "shooting");
	struct monster *mon = square_monster(cave, y, x);
	int chance = chance_of_missile_hit(p, ammo, bow, y, x);
	int multiplier = p->state.ammo_mult;
	int b = 0, s = 0;

	my_strcpy(hit_verb, "hits", 20);

	/* Did we hit it (penalize distance travelled) */
	if (!test_hit(chance, mon->race->ac, monster_is_visible(mon)))
		return result;

	result.success = true;

	improve_attack_modifier(ammo, mon, &b, &s, result.hit_verb, true);
	improve_attack_modifier(bow, mon, &b, &s, result.hit_verb, true);

	result.dmg = ranged_damage(ammo, bow, b, s, multiplier);
	result.dmg = critical_shot(player, mon, ammo->weight, ammo->to_h, result.dmg,
							   &result.msg_type);

	missile_learn_on_ranged_attack(p, bow);

	return result;
}


/**
 * Helper function used with ranged_helper by do_cmd_throw.
 */
static struct attack_result make_ranged_throw(struct player *p,
	struct object *obj, int y, int x)
{
	char *hit_verb = mem_alloc(20*sizeof(char));
	struct attack_result result = {false, 0, 0, hit_verb};
	struct monster *mon = square_monster(cave, y, x);
	int chance = chance_of_missile_hit(p, obj, NULL, y, x);
	int multiplier = 1;
	int b = 0, s = 0;

	my_strcpy(hit_verb, "hits", 20);

	/* If we missed then we're done */
	if (!test_hit(chance, mon->race->ac, monster_is_visible(mon)))
		return result;

	result.success = true;

	improve_attack_modifier(obj, mon, &b, &s, result.hit_verb, true);

	result.dmg = ranged_damage(obj, NULL, b, s, multiplier);
	result.dmg = critical_norm(player, mon, obj->weight, obj->to_h, result.dmg,
							   &result.msg_type);

	/* Direct adjustment for exploding things (flasks of oil) */
	if (of_has(obj->flags, OF_EXPLODE))
		result.dmg *= 3;

	return result;
}


/**
 * Fire an object from the quiver, pack or floor at a target.
 */
void do_cmd_fire(struct command *cmd) {
	int dir;
	int range = MIN(6 + 2 * player->state.ammo_mult, z_info->max_range);
	int shots = player->state.num_shots;

	ranged_attack attack = make_ranged_shot;

	struct object *bow = equipped_item_by_slot_name(player, "shooting");
	struct object *obj;

	if (player_is_shapechanged(player)) {
		msg("You cannot do this while in %s form.",	player->shape->name);
		if (get_check("Do you want to change back? " )) {
			player_resume_normal_shape(player);
		} else {
			return;
		}
	}

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Fire which ammunition?",
			/* Error  */ "You have no ammunition to fire.",
			/* Filter */ obj_can_fire,
			/* Choice */ USE_INVEN | USE_QUIVER | USE_FLOOR | QUIVER_TAGS)
		!= CMD_OK)
		return;

	if (cmd_get_target(cmd, "target", &dir) == CMD_OK)
		player_confuse_dir(player, &dir, false);
	else
		return;

	/* Require a usable launcher */
	if (!bow || !player->state.ammo_tval) {
		msg("You have nothing to fire with.");
		return;
	}

	/* Check the item being fired is usable by the player. */
	if (!item_is_available(obj)) {
		msg("That item is not within your reach.");
		return;
	}

	/* Check the ammo can be used with the launcher */
	if (obj->tval != player->state.ammo_tval) {
		msg("That ammo cannot be fired by your current weapon.");
		return;
	}

	ranged_helper(player, obj, dir, range, shots, attack, ranged_hit_types,
				  (int) N_ELEMENTS(ranged_hit_types));
}


/**
 * Throw an object from the quiver, pack or floor.
 */
void do_cmd_throw(struct command *cmd) {
	int dir;
	int shots = 10;
	int str = adj_str_blow[player->state.stat_ind[STAT_STR]];
	ranged_attack attack = make_ranged_throw;

	int weight;
	int range;
	struct object *obj;

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Throw which item?",
			/* Error  */ "You have nothing to throw.",
			/* Filter */ NULL,
			/* Choice */ USE_QUIVER | USE_INVEN | USE_FLOOR | QUIVER_TAGS)
		!= CMD_OK)
		return;

	if (cmd_get_target(cmd, "target", &dir) == CMD_OK)
		player_confuse_dir(player, &dir, false);
	else
		return;


	weight = MAX(obj->weight, 10);
	range = MIN(((str + 20) * 10) / weight, 10);

	/* Make sure the player isn't throwing wielded items */
	if (object_is_equipped(player->body, obj)) {
		msg("You cannot throw wielded items.");
		return;
	}

	ranged_helper(player, obj, dir, range, shots, attack, melee_hit_types,
				  (int) N_ELEMENTS(melee_hit_types));
}

/**
 * Front-end command which fires at the nearest target with default ammo.
 */
void do_cmd_fire_at_nearest(void) {
	int i, dir = DIR_TARGET;
	struct object *ammo = NULL;
	struct object *bow = equipped_item_by_slot_name(player, "shooting");

	/* Require a usable launcher */
	if (!bow || !player->state.ammo_tval) {
		msg("You have nothing to fire with.");
		return;
	}

	/* Find first eligible ammo in the quiver */
	for (i = 0; i < z_info->quiver_size; i++) {
		if (!player->upkeep->quiver[i])
			continue;
		if (player->upkeep->quiver[i]->tval != player->state.ammo_tval)
			continue;
		ammo = player->upkeep->quiver[i];
		break;
	}

	/* Require usable ammo */
	if (!ammo) {
		msg("You have no ammunition in the quiver to fire.");
		return;
	}

	/* Require foe */
	if (!target_set_closest((TARGET_KILL | TARGET_QUIET), NULL)) return;

	/* Fire! */
	cmdq_push(CMD_FIRE);
	cmd_set_arg_item(cmdq_peek(), "item", ammo);
	cmd_set_arg_target(cmdq_peek(), "target", dir);
}
