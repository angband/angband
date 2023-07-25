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
 * ------------------------------------------------------------------------
 * Hit and breakage calculations
 * ------------------------------------------------------------------------ */
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
	if (of_has(obj->flags, OF_THROWING) &&
		!of_has(obj->flags, OF_EXPLODE) &&
		!tval_is_ammo(obj)) {
		perc = 1;
	}
	if (!hit_target) return (perc * perc) / 100;
	return perc;
}

/**
 * Calculate the player's base melee to-hit value without regard to a specific
 * monster.
 * See also: chance_of_missile_hit_base
 *
 * \param p The player
 * \param weapon The player's weapon
 */
int chance_of_melee_hit_base(const struct player *p,
		const struct object *weapon)
{
	int bonus = p->state.to_h + (weapon ? weapon->to_h : 0);
	return p->state.skills[SKILL_TO_HIT_MELEE] + bonus * BTH_PLUS_ADJ;
}

/**
 * Calculate the player's melee to-hit value against a specific monster.
 * See also: chance_of_missile_hit
 *
 * \param p The player
 * \param weapon The player's weapon
 * \param mon The monster
 */
static int chance_of_melee_hit(const struct player *p,
		const struct object *weapon, const struct monster *mon)
{
	int chance = chance_of_melee_hit_base(p, weapon);
	/* Non-visible targets have a to-hit penalty of 50% */
	return monster_is_visible(mon) ? chance : chance / 2;
}

/**
 * Calculate the player's base missile to-hit value without regard to a specific
 * monster.
 * See also: chance_of_melee_hit_base
 *
 * \param p The player
 * \param missile The missile to launch
 * \param launcher The launcher to use (optional)
 */
int chance_of_missile_hit_base(const struct player *p,
								 const struct object *missile,
								 const struct object *launcher)
{
	int bonus = missile->to_h;
	int chance;

	if (!launcher) {
		/* Other thrown objects are easier to use, but only throwing weapons 
		 * take advantage of bonuses to Skill and Deadliness from other 
		 * equipped items. */
		if (of_has(missile->flags, OF_THROWING)) {
			bonus += p->state.to_h;
			chance = p->state.skills[SKILL_TO_HIT_THROW] + bonus * BTH_PLUS_ADJ;
		} else {
			chance = 3 * p->state.skills[SKILL_TO_HIT_THROW] / 2
				+ bonus * BTH_PLUS_ADJ;
		}
	} else {
		bonus += p->state.to_h + launcher->to_h;
		chance = p->state.skills[SKILL_TO_HIT_BOW] + bonus * BTH_PLUS_ADJ;
	}

	return chance;
}

/**
 * Calculate the player's missile to-hit value against a specific monster.
 * See also: chance_of_melee_hit
 *
 * \param p The player
 * \param missile The missile to launch
 * \param launcher Optional launcher to use (thrown weapons use no launcher)
 * \param mon The monster
 */
static int chance_of_missile_hit(const struct player *p,
	const struct object *missile, const struct object *launcher,
	const struct monster *mon)
{
	int chance = chance_of_missile_hit_base(p, missile, launcher);
	/* Penalize for distance */
	chance -= distance(p->grid, mon->grid);
	/* Non-visible targets have a to-hit penalty of 50% */
	return monster_is_obvious(mon) ? chance : chance / 2;
}

/**
 * Determine if a hit roll is successful against the target AC.
 * See also: hit_chance
 *
 * \param to_hit To total to-hit value to use
 * \param ac The AC to roll against
 */
bool test_hit(int to_hit, int ac)
{
	random_chance c;
	hit_chance(&c, to_hit, ac);
	return random_chance_check(c);
}

/**
 * Return a random_chance by reference, which represents the likelihood of a
 * hit roll succeeding for the given to_hit and ac values. The hit calculation
 * will:
 *
 * Always hit 12% of the time
 * Always miss 5% of the time
 * Put a floor of 9 on the to-hit value
 * Roll between 0 and the to-hit value
 * The outcome must be >= AC*2/3 to be considered a hit
 *
 * \param chance The random_chance to return-by-reference
 * \param to_hit The to-hit value to use
 * \param ac The AC to roll against
 */
void hit_chance(random_chance *chance, int to_hit, int ac)
{
	/* Percentages scaled to 10,000 to avoid rounding error */
	const int HUNDRED_PCT = 10000;
	const int ALWAYS_HIT = 1200;
	const int ALWAYS_MISS = 500;

	/* Put a floor on the to_hit */
	to_hit = MAX(9, to_hit);

	/* Calculate the hit percentage */
	chance->numerator = MAX(0, to_hit - ac * 2 / 3);
	chance->denominator = to_hit;

	/* Convert the ratio to a scaled percentage */
	chance->numerator = HUNDRED_PCT * chance->numerator / chance->denominator;
	chance->denominator = HUNDRED_PCT;

	/* The calculated rate only applies when the guaranteed hit/miss don't */
	chance->numerator = chance->numerator *
			(HUNDRED_PCT - ALWAYS_MISS - ALWAYS_HIT) / HUNDRED_PCT;

	/* Add in the guaranteed hit */
	chance->numerator += ALWAYS_HIT;
}

/**
 * ------------------------------------------------------------------------
 * Damage calculations
 * ------------------------------------------------------------------------ */
/**
 * Conversion of plusses to Deadliness to a percentage added to damage.
 * Much of this table is not intended ever to be used, and is included
 * only to handle possible inflation elsewhere. -LM-
 */
uint8_t deadliness_conversion[151] =
  {
    0,
    5,  10,  14,  18,  22,  26,  30,  33,  36,  39,
    42,  45,  48,  51,  54,  57,  60,  63,  66,  69,
    72,  75,  78,  81,  84,  87,  90,  93,  96,  99,
    102, 104, 107, 109, 112, 114, 117, 119, 122, 124,
    127, 129, 132, 134, 137, 139, 142, 144, 147, 149,
    152, 154, 157, 159, 162, 164, 167, 169, 172, 174,
    176, 178, 180, 182, 184, 186, 188, 190, 192, 194,
    196, 198, 200, 202, 204, 206, 208, 210, 212, 214,
    216, 218, 220, 222, 224, 226, 228, 230, 232, 234,
    236, 238, 240, 242, 244, 246, 248, 250, 251, 253,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255
  };

/**
 * Deadliness multiplies the damage done by a percentage, which varies 
 * from 0% (no damage done at all) to at most 355% (damage is multiplied 
 * by more than three and a half times!).
 *
 * We use the table "deadliness_conversion" to translate internal plusses 
 * to deadliness to percentage values.
 *
 * This function multiplies damage by 100.
 */
void apply_deadliness(int *die_average, int deadliness)
{
	int i;

	/* Paranoia - ensure legal table access. */
	if (deadliness > 150)
		deadliness = 150;
	if (deadliness < -150)
		deadliness = -150;

	/* Deadliness is positive - damage is increased */
	if (deadliness >= 0) {
		i = deadliness_conversion[deadliness];

		*die_average *= (100 + i);
	}

	/* Deadliness is negative - damage is decreased */
	else {
		i = deadliness_conversion[ABS(deadliness)];

		if (i >= 100)
			*die_average = 0;
		else
			*die_average *= (100 - i);
	}
}

/**
 * Check if a monster is debuffed in such a way as to make a critical
 * hit more likely.
 */
static bool is_debuffed(const struct monster *monster)
{
	return monster->m_timed[MON_TMD_CONF] > 0 ||
			monster->m_timed[MON_TMD_HOLD] > 0 ||
			monster->m_timed[MON_TMD_FEAR] > 0 ||
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
		int dam, bool launched, uint32_t *msg_type)
{
	int to_h = p->state.to_h + plus;
	int chance, new_dam;

	if (is_debuffed(monster)) {
		to_h += z_info->r_crit_debuff_toh;
	}
	chance = z_info->r_crit_chance_weight_scl * weight
		+ z_info->r_crit_chance_toh_scl * to_h
		+ z_info->r_crit_chance_level_scl * p->lev
		+ z_info->r_crit_chance_offset;
	if (launched) {
		chance += z_info->r_crit_chance_launched_toh_skill_scl
			* p->state.skills[SKILL_TO_HIT_BOW];
	} else {
		chance += z_info->r_crit_chance_thrown_toh_skill_scl
			* p->state.skills[SKILL_TO_HIT_THROW];
	}

	if (randint1(z_info->r_crit_chance_range) > chance
			|| !z_info->r_crit_level_head) {
		*msg_type = MSG_SHOOT_HIT;
		new_dam = dam;
	} else {
		int power = z_info->r_crit_power_weight_scl * weight
			+ randint1(z_info->r_crit_power_random);
		const struct critical_level *this_l = z_info->r_crit_level_head;

		while (power >= this_l->cutoff && this_l->next) {
			this_l = this_l->next;
		}
		*msg_type = this_l->msgt;
		new_dam = this_l->add + this_l->mult * dam;
	}

	return new_dam;
}

/**
 * Determine O-combat damage for critical hits from shooting.
 */
static int o_critical_shot(const struct player *p,
		const struct monster *monster,
		const struct object *missile,
		const struct object *launcher,
		uint32_t *msg_type)
{
	int power = chance_of_missile_hit_base(p, missile, launcher);
	int chance_num, chance_den, add_dice;

	if (is_debuffed(monster)) {
		power += z_info->o_r_crit_debuff_toh;
	}
	/* Apply a rational scale factor. */
	if (launcher) {
		power = (power * z_info->o_r_crit_power_launched_toh_scl_num)
			/ z_info->o_r_crit_power_launched_toh_scl_den;
	} else {
		power = (power * z_info->o_r_crit_power_thrown_toh_scl_num)
			/ z_info->o_r_crit_power_thrown_toh_scl_den;
	}

	/* Test for critical hit:  chance is a * power / (b * power + c) */
	chance_num = power * z_info->o_r_crit_chance_power_scl_num;
	chance_den = power * z_info->o_r_crit_chance_power_scl_den
		+ z_info->o_r_crit_chance_add_den;
	if (randint1(chance_den) <= chance_num && z_info->o_r_crit_level_head) {
		/* Determine level of critical hit. */
		const struct o_critical_level *this_l =
			z_info->o_r_crit_level_head;

		while (this_l->next && !one_in_(this_l->chance)) {
			this_l = this_l->next;
		}
		add_dice = this_l->added_dice;
		*msg_type = this_l->msgt;
	} else {
		add_dice = 0;
		*msg_type = MSG_SHOOT_HIT;
	}

	return add_dice;
}

/**
 * Determine damage for critical hits from melee.
 *
 * Factor in weapon weight, total plusses, player level.
 */
static int critical_melee(const struct player *p,
		const struct monster *monster,
		int weight, int plus,
		int dam, uint32_t *msg_type)
{
	int to_h = p->state.to_h + plus;
	int chance, new_dam;

	if (is_debuffed(monster)) {
		to_h += z_info->m_crit_debuff_toh;
	}
	chance = z_info->m_crit_chance_weight_scl * weight
		+ z_info->m_crit_chance_toh_scl * to_h
		+ z_info->m_crit_chance_level_scl * p->lev
		+ z_info->m_crit_chance_toh_skill_scl
			* p->state.skills[SKILL_TO_HIT_MELEE]
		+ z_info->m_crit_chance_offset;

	if (randint1(z_info->m_crit_chance_range) > chance
			|| !z_info->m_crit_level_head) {
		*msg_type = MSG_HIT;
		new_dam = dam;
	} else {
		int power = z_info->m_crit_power_weight_scl * weight
			+ randint1(z_info->m_crit_power_random);
		const struct critical_level *this_l = z_info->m_crit_level_head;

		while (power >= this_l->cutoff && this_l->next) {
			this_l = this_l->next;
		}
		*msg_type = this_l->msgt;
		new_dam = this_l->add + this_l->mult * dam;
	}

	return new_dam;
}

/**
 * Determine O-combat damage for critical hits from melee.
 */
static int o_critical_melee(const struct player *p,
		const struct monster *monster,
		const struct object *obj, uint32_t *msg_type)
{
	int power = chance_of_melee_hit_base(p, obj);
	int chance_num, chance_den, add_dice;

	if (is_debuffed(monster)) {
		power += z_info->o_m_crit_debuff_toh;
	}
	/* Apply a rational scale factor. */
	power = (power * z_info->o_m_crit_power_toh_scl_num)
		/ z_info->o_m_crit_power_toh_scl_den;

	/* Test for critical hit:  chance is a * power / (b * power + c) */
	chance_num = power * z_info->o_m_crit_chance_power_scl_num;
	chance_den = power * z_info->o_m_crit_chance_power_scl_den
		+ z_info->o_m_crit_chance_add_den;
	if (randint1(chance_den) <= chance_num && z_info->o_m_crit_level_head) {
		/* Determine level of critical hit. */
		const struct o_critical_level *this_l =
			z_info->o_m_crit_level_head;

		while (this_l->next && !one_in_(this_l->chance)) {
			this_l = this_l->next;
		}
		add_dice = this_l->added_dice;
		*msg_type = this_l->msgt;
	} else {
		add_dice = 0;
		*msg_type = MSG_SHOOT_HIT;
	}

	return add_dice;
}

/**
 * Determine standard melee damage.
 *
 * Factor in damage dice, to-dam and any brand or slay.
 */
static int melee_damage(const struct monster *mon, struct object *obj, int b, int s)
{
	int dmg = (obj) ? damroll(obj->dd, obj->ds) : 1;

	if (s) {
		dmg *= slays[s].multiplier;
	} else if (b) {
		dmg *= get_monster_brand_multiplier(mon, &brands[b], false);
	}

	if (obj) dmg += obj->to_d;

	return dmg;
}

/**
 * Determine O-combat melee damage.
 *
 * Deadliness and any brand or slay add extra sides to the damage dice,
 * criticals add extra dice.
 */
static int o_melee_damage(struct player *p, const struct monster *mon,
		struct object *obj, int b, int s, uint32_t *msg_type)
{
	int dice = (obj) ? obj->dd : 1;
	int sides, dmg, add = 0;
	bool extra;

	/* Get the average value of a single damage die. (x10) */
	int die_average = (10 * (((obj) ? obj->ds : 1) + 1)) / 2;

	/* Adjust the average for slays and brands. (10x inflation) */
	if (s) {
		die_average *= slays[s].o_multiplier;
		add = slays[s].o_multiplier - 10;
	} else if (b) {
		int bmult = get_monster_brand_multiplier(mon, &brands[b], true);

		die_average *= bmult;
		add = bmult - 10;
	} else {
		die_average *= 10;
	}

	/* Apply deadliness to average. (100x inflation) */
	apply_deadliness(&die_average,
		MIN(((obj) ? obj->to_d : 0) + p->state.to_d, 150));

	/* Calculate the actual number of sides to each die. */
	sides = (2 * die_average) - 10000;
	extra = randint0(10000) < (sides % 10000);
	sides /= 10000;
	sides += (extra ? 1 : 0);

	/*
	 * Get number of critical dice; for now, excluding criticals for
	 * unarmed combat
	 */
	if (obj) dice += o_critical_melee(p, mon, obj, msg_type);

	/* Roll out the damage. */
	dmg = damroll(dice, sides);

	/* Apply any special additions to damage. */
	dmg += add;

	return dmg;
}

/**
 * Determine standard ranged damage.
 *
 * Factor in damage dice, to-dam, multiplier and any brand or slay.
 */
static int ranged_damage(struct player *p, const struct monster *mon,
						 struct object *missile, struct object *launcher,
						 int b, int s)
{
	int dmg;
	int mult = (launcher ? p->state.ammo_mult : 1);

	/* If we have a slay or brand, modify the multiplier appropriately */
	if (b) {
		mult += get_monster_brand_multiplier(mon, &brands[b], false);
	} else if (s) {
		mult += slays[s].multiplier;
	}

	/* Apply damage: multiplier, slays, bonuses */
	dmg = damroll(missile->dd, missile->ds);
	dmg += missile->to_d;
	if (launcher) {
		dmg += launcher->to_d;
	} else if (of_has(missile->flags, OF_THROWING)) {
		/* Adjust damage for throwing weapons.
		 * This is not the prettiest equation, but it does at least try to
		 * keep throwing weapons competitive. */
		dmg *= 2 + missile->weight / 12;
	}
	dmg *= mult;

	return dmg;
}

/**
 * Determine O-combat ranged damage.
 *
 * Deadliness, launcher multiplier and any brand or slay add extra sides to the
 * damage dice, criticals add extra dice.
 */
static int o_ranged_damage(struct player *p, const struct monster *mon,
		struct object *missile, struct object *launcher,
		int b, int s, uint32_t *msg_type)
{
	int mult = (launcher ? p->state.ammo_mult : 1);
	int dice = missile->dd;
	int sides, deadliness, dmg, add = 0;
	bool extra;

	/* Get the average value of a single damage die. (x10) */
	int die_average = (10 * (missile->ds + 1)) / 2;

	/* Apply the launcher multiplier. */
	die_average *= mult;

	/* Adjust the average for slays and brands. (10x inflation) */
	if (b) {
		int bmult = get_monster_brand_multiplier(mon, &brands[b], true);

		die_average *= bmult;
		add = bmult - 10;
	} else if (s) {
		die_average *= slays[s].o_multiplier;
		add = slays[s].o_multiplier - 10;
	} else {
		die_average *= 10;
	}

	/* Apply deadliness to average. (100x inflation) */
	if (launcher) {
		deadliness = missile->to_d + launcher->to_d + p->state.to_d;
	} else if (of_has(missile->flags, OF_THROWING)) {
		deadliness = missile->to_d + p->state.to_d;
	} else {
		deadliness = missile->to_d;
	}
	apply_deadliness(&die_average, MIN(deadliness, 150));

	/* Calculate the actual number of sides to each die. */
	sides = (2 * die_average) - 10000;
	extra = randint0(10000) < (sides % 10000);
	sides /= 10000;
	sides += (extra ? 1 : 0);

	/* Get number of critical dice - only for suitable objects */
	if (launcher) {
		dice += o_critical_shot(p, mon, missile, launcher, msg_type);
	} else if (of_has(missile->flags, OF_THROWING)) {
		dice += o_critical_shot(p, mon, missile, NULL, msg_type);

		/* Multiply the number of damage dice by the throwing weapon
		 * multiplier.  This is not the prettiest equation,
		 * but it does at least try to keep throwing weapons competitive. */
		dice *= 2 + missile->weight / 12;
	}

	/* Roll out the damage. */
	dmg = damroll(dice, sides);

	/* Apply any special additions to damage. */
	dmg += add;

	return dmg;
}

/**
 * Apply the player damage bonuses
 */
static int player_damage_bonus(struct player_state *state)
{
	return state->to_d;
}

/**
 * ------------------------------------------------------------------------
 * Non-damage melee blow effects
 * ------------------------------------------------------------------------ */
/**
 * Apply blow side effects
 */
static void blow_side_effects(struct player *p, struct monster *mon)
{
	/* Confusion attack */
	if (p->timed[TMD_ATT_CONF]) {
		player_clear_timed(p, TMD_ATT_CONF, true, false);

		mon_inc_timed(mon, MON_TMD_CONF, (10 + randint0(p->lev) / 10),
					  MON_TMD_FLG_NOTIFY);
	}
}

/**
 * Apply blow after effects
 */
static bool blow_after_effects(struct loc grid, int dmg, int splash,
							   bool *fear, bool quake)
{
	/* Apply earthquake brand */
	if (quake) {
		effect_simple(EF_EARTHQUAKE, source_player(), "0", 0, 10, 0, 0, 0,
					  NULL);

		/* Monster may be dead or moved */
		if (!square_monster(cave, grid))
			return true;
	}

	return false;
}

/**
 * ------------------------------------------------------------------------
 * Melee attack
 * ------------------------------------------------------------------------ */
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
 * Attack the monster at the given location with a single blow.
 */
bool py_attack_real(struct player *p, struct loc grid, bool *fear)
{
	size_t i;

	/* Information about the target of the attack */
	struct monster *mon = square_monster(cave, grid);
	char m_name[80];
	bool stop = false;

	/* The weapon used */
	struct object *obj = equipped_item_by_slot_name(p, "weapon");

	/* Information about the attack */
	int drain = 0;
	int splash = 0;
	bool do_quake = false;
	bool success = false;

	char verb[20];
	uint32_t msg_type = MSG_HIT;
	int j, b, s, weight, dmg;

	/* Default to punching */
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
	monster_wake(mon, false, 100);
	mon_clear_timed(mon, MON_TMD_HOLD, MON_TMD_FLG_NOTIFY);

	/* See if the player hit */
	success = test_hit(chance_of_melee_hit(p, obj, mon), mon->race->ac);

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

	if (obj) {
		/* Handle normal weapon */
		weight = obj->weight;
		my_strcpy(verb, "hit", sizeof(verb));
	} else {
		weight = 0;
	}

	/* Best attack from all slays or brands on all non-launcher equipment */
	b = 0;
	s = 0;
	for (j = 2; j < p->body.count; j++) {
		struct object *obj_local = slot_object(p, j);
		if (obj_local)
			improve_attack_modifier(p, obj_local, mon, &b, &s,
				verb, false);
	}

	/* Get the best attack from all slays or brands - weapon or temporary */
	if (obj) {
		improve_attack_modifier(p, obj, mon, &b, &s, verb, false);
	}
	improve_attack_modifier(p, NULL, mon, &b, &s, verb, false);

	/* Get the damage */
	if (!OPT(p, birth_percent_damage)) {
		dmg = melee_damage(mon, obj, b, s);
		/* For now, exclude criticals on unarmed combat */
		if (obj) dmg = critical_melee(p, mon, weight, obj->to_h,
			dmg, &msg_type);
	} else {
		dmg = o_melee_damage(p, mon, obj, b, s, &msg_type);
	}

	/* Splash damage and earthquakes */
	splash = (weight * dmg) / 100;
	if (player_of_has(p, OF_IMPACT) && dmg > 50) {
		do_quake = true;
		equip_learn_flag(p, OF_IMPACT);
	}

	/* Learn by use */
	equip_learn_on_melee_attack(p);
	learn_brand_slay_from_melee(p, obj, mon);

	/* Apply the player damage bonuses */
	if (!OPT(p, birth_percent_damage)) {
		dmg += player_damage_bonus(&p->state);
	}

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

	/* Damage, check for hp drain, fear and death */
	drain = MIN(mon->hp, dmg);
	stop = mon_take_hit(mon, p, dmg, fear, NULL);

	/* Small chance of bloodlust side-effects */
	if (p->timed[TMD_BLOODLUST] && one_in_(50)) {
		msg("You feel something give way!");
		player_over_exert(p, PY_EXERT_CON, 20, 0);
	}

	if (!stop) {
		if (p->timed[TMD_ATT_VAMP] && monster_is_living(mon)) {
			effect_simple(EF_HEAL_HP, source_player(), format("%d", drain),
						  0, 0, 0, 0, 0, NULL);
		}
	}

	if (stop)
		(*fear) = false;

	/* Post-damage effects */
	if (blow_after_effects(grid, dmg, splash, fear, do_quake))
		stop = true;

	return stop;
}


/**
 * Attempt a shield bash; return true if the monster dies
 */
static bool attempt_shield_bash(struct player *p, struct monster *mon, bool *fear)
{
	struct object *weapon = slot_object(p, slot_by_name(p, "weapon"));
	struct object *shield = slot_object(p, slot_by_name(p, "arm"));
	int nblows = p->state.num_blows / 100;
	int bash_quality, bash_dam, energy_lost;

	/* Bashing chance depends on melee skill, DEX, and a level bonus. */
	int bash_chance = p->state.skills[SKILL_TO_HIT_MELEE] / 8 +
		adj_dex_th[p->state.stat_ind[STAT_DEX]] / 2;

	/* No shield, no bash */
	if (!shield) return false;

	/* Monster is too pathetic, don't bother */
	if (mon->race->level < p->lev / 2) return false;

	/* Players bash more often when they see a real need: */
	if (!equipped_item_by_slot_name(p, "weapon")) {
		/* Unarmed... */
		bash_chance *= 4;
	} else if (weapon->dd * weapon->ds * nblows < shield->dd * shield->ds * 3) {
		/* ... or armed with a puny weapon */
		bash_chance *= 2;
	}

	/* Try to get in a shield bash. */
	if (bash_chance <= randint0(200 + mon->race->level)) {
		return false;
	}

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

	if (OPT(p, show_damage)) {
		msgt(MSG_HIT, "You get in a shield bash! (%d)", bash_dam);
	} else {
		msgt(MSG_HIT, "You get in a shield bash!");
	}

	/* Encourage the player to keep wearing that heavy shield. */
	if (randint1(bash_dam) > 30 + randint1(bash_dam / 2)) {
		msgt(MSG_HIT_HI_SUPERB, "WHAMM!");
	}

	/* Damage, check for fear and death. */
	if (mon_take_hit(mon, p, bash_dam, fear, NULL)) return true;

	/* Stunning. */
	if (bash_quality + p->lev > randint1(200 + mon->race->level * 8)) {
		mon_inc_timed(mon, MON_TMD_STUN, randint0(p->lev / 5) + 4, 0);
	}

	/* Confusion. */
	if (bash_quality + p->lev > randint1(300 + mon->race->level * 12)) {
		mon_inc_timed(mon, MON_TMD_CONF, randint0(p->lev / 5) + 4, 0);
	}

	/* The player will sometimes stumble. */
	if (35 + adj_dex_th[p->state.stat_ind[STAT_DEX]] < randint1(60)) {
		energy_lost = randint1(50) + 25;
		/* Lose 26-75% of a turn due to stumbling after shield bash. */
		msgt(MSG_GENERIC, "You stumble!");
		p->upkeep->energy_use += energy_lost * z_info->move_energy / 100;
	}

	return false;
}

/**
 * Attack the monster at the given location
 *
 * We get blows until energy drops below that required for another blow, or
 * until the target monster dies. Each blow is handled by py_attack_real().
 * We don't allow @ to spend more than 1 turn's worth of energy,
 * to avoid slower monsters getting double moves.
 */
void py_attack(struct player *p, struct loc grid)
{
	int avail_energy = MIN(p->energy, z_info->move_energy);
	int blow_energy = 100 * z_info->move_energy / p->state.num_blows;
	bool slain = false, fear = false;
	struct monster *mon = square_monster(cave, grid);

	/* Disturb the player */
	disturb(p);

	/* Initialize the energy used */
	p->upkeep->energy_use = 0;

	/* Reward BGs with 5% of max SPs, min 1/2 point */
	if (player_has(p, PF_COMBAT_REGEN)) {
		int32_t sp_gain = (((int32_t)MAX(p->msp, 10)) * 16384) / 5;
		player_adjust_mana_precise(p, sp_gain);
	}

	/* Player attempts a shield bash if they can, and if monster is visible
	 * and not too pathetic */
	if (player_has(p, PF_SHIELD_BASH) && monster_is_visible(mon)) {
		/* Monster may die */
		if (attempt_shield_bash(p, mon, &fear)) return;
	}

	/* Attack until the next attack would exceed energy available or
	 * a full turn or until the enemy dies. We limit energy use
	 * to avoid giving monsters a possible double move. */
	while (avail_energy - p->upkeep->energy_use >= blow_energy && !slain) {
		slain = py_attack_real(p, grid, &fear);
		p->upkeep->energy_use += blow_energy;
	}

	/* Hack - delay fear messages */
	if (fear && monster_is_visible(mon)) {
		add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
	}
}

/**
 * ------------------------------------------------------------------------
 * Ranged attacks
 * ------------------------------------------------------------------------ */
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

	int path_n;
	struct loc path_g[256];

	/* Start at the player */
	struct loc grid = p->grid;

	/* Predict the "target" location */
	struct loc target = loc_sum(grid, loc(99 * ddx[dir], 99 * ddy[dir]));

	bool hit_target = false;
	bool none_left = false;

	struct object *missile;
	int pierce = 1;

	/* Check for target validity */
	if ((dir == DIR_TARGET) && target_okay()) {
		int taim;
		target_get(&target);
		taim = distance(grid, target);
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

	/* Actually "fire" the object -- Take a partial turn */
	p->upkeep->energy_use = (z_info->move_energy * 10 / shots);

	/* Calculate the path */
	path_n = project_path(cave, path_g, range, grid, target, 0);

	/* Calculate potenital piercing */
	if (p->timed[TMD_POWERSHOT] && tval_is_sharp_missile(obj)) {
		pierce = p->state.ammo_mult;
	}

	/* Hack -- Handle stuff */
	handle_stuff(p);

	/* Project along the path */
	for (i = 0; i < path_n; ++i) {
		struct monster *mon = NULL;
		bool see = square_isseen(cave, path_g[i]);

		/* Stop before hitting walls */
		if (!(square_ispassable(cave, path_g[i])) &&
			!(square_isprojectable(cave, path_g[i])))
			break;

		/* Advance */
		grid = path_g[i];

		/* Tell the UI to display the missile */
		event_signal_missile(EVENT_MISSILE, obj, see, grid.y, grid.x);

		/* Try the attack on the monster at (x, y) if any */
		mon = square_monster(cave, path_g[i]);
		if (mon) {
			int visible = monster_is_obvious(mon);

			bool fear = false;
			const char *note_dies = monster_is_destroyed(mon) ? 
				" is destroyed." : " dies.";

			struct attack_result result = attack(p, obj, grid);
			int dmg = result.dmg;
			uint32_t msg_type = result.msg_type;
			char hit_verb[20];
			my_strcpy(hit_verb, result.hit_verb, sizeof(hit_verb));
			mem_free(result.hit_verb);

			if (result.success) {
				char o_name[80];

				hit_target = true;

				missile_learn_on_ranged_attack(p, obj);

				/* Learn by use for other equipped items */
				equip_learn_on_ranged_attack(p);

				/*
				 * Describe the object (have most up-to-date
				 * knowledge now).
				 */
				object_desc(o_name, sizeof(o_name), obj,
					ODESC_FULL | ODESC_SINGULAR, p);

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
					if (monster_is_obvious(mon)) {
						monster_race_track(p->upkeep, mon->race);
						health_track(p->upkeep, mon);
					}
				}

				/* Hit the monster, check for death */
				if (!mon_take_hit(mon, p, dmg, &fear, note_dies)) {
					message_pain(mon, dmg);
					if (fear && monster_is_obvious(mon)) {
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
		if (!(square_isprojectable(cave, path_g[i]))) 
			break;
	}

	/* Get the missile */
	if (object_is_carried(p, obj)) {
		missile = gear_object_for_use(p, obj, 1, true, &none_left);
	} else {
		missile = floor_object_for_use(p, obj, 1, true, &none_left);
	}

	/* Terminate piercing */
	if (p->timed[TMD_POWERSHOT]) {
		player_clear_timed(p, TMD_POWERSHOT, true, false);
	}

	/* Drop (or break) near that location */
	drop_near(cave, &missile, breakage_chance(missile, hit_target), grid, true, false);
}


/**
 * Helper function used with ranged_helper by do_cmd_fire.
 */
struct attack_result make_ranged_shot(struct player *p,
		struct object *ammo, struct loc grid)
{
	char *hit_verb = mem_alloc(20 * sizeof(char));
	struct attack_result result = {false, 0, 0, hit_verb};
	struct object *bow = equipped_item_by_slot_name(p, "shooting");
	struct monster *mon = square_monster(cave, grid);
	int b = 0, s = 0;

	my_strcpy(hit_verb, "hits", 20);

	/* Did we hit it */
	if (!test_hit(chance_of_missile_hit(p, ammo, bow, mon), mon->race->ac))
		return result;

	result.success = true;

	improve_attack_modifier(p, ammo, mon, &b, &s, result.hit_verb, true);
	improve_attack_modifier(p, bow, mon, &b, &s, result.hit_verb, true);

	if (!OPT(p, birth_percent_damage)) {
		result.dmg = ranged_damage(p, mon, ammo, bow, b, s);
		result.dmg = critical_shot(p, mon, ammo->weight, ammo->to_h,
			result.dmg, true, &result.msg_type);
	} else {
		result.dmg = o_ranged_damage(p, mon, ammo, bow, b, s, &result.msg_type);
	}

	missile_learn_on_ranged_attack(p, bow);
	learn_brand_slay_from_launch(p, ammo, bow, mon);

	return result;
}


/**
 * Helper function used with ranged_helper by do_cmd_throw.
 */
struct attack_result make_ranged_throw(struct player *p,
	struct object *obj, struct loc grid)
{
	char *hit_verb = mem_alloc(20 * sizeof(char));
	struct attack_result result = {false, 0, 0, hit_verb};
	struct monster *mon = square_monster(cave, grid);
	int b = 0, s = 0;

	my_strcpy(hit_verb, "hits", 20);

	/* If we missed then we're done */
	if (!test_hit(chance_of_missile_hit(p, obj, NULL, mon), mon->race->ac))
		return result;

	result.success = true;

	improve_attack_modifier(p, obj, mon, &b, &s, result.hit_verb, true);

	if (!OPT(p, birth_percent_damage)) {
		result.dmg = ranged_damage(p, mon, obj, NULL, b, s);
		result.dmg = critical_shot(p, mon, obj->weight, obj->to_h,
			result.dmg, false, &result.msg_type);
	} else {
		result.dmg = o_ranged_damage(p, mon, obj, NULL, b, s, &result.msg_type);
	}

	/* Direct adjustment for exploding things (flasks of oil) */
	if (of_has(obj->flags, OF_EXPLODE))
		result.dmg *= 3;

	learn_brand_slay_from_throw(p, obj, mon);

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

	if (!player_get_resume_normal_shape(player, cmd)) {
		return;
	}

	/* Get arguments */
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Fire which ammunition?",
			/* Error  */ "You have no suitable ammunition to fire.",
			/* Filter */ obj_can_fire,
			/* Choice */ USE_INVEN | USE_QUIVER | USE_FLOOR | QUIVER_TAGS)
		!= CMD_OK)
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

	if (cmd_get_target(cmd, "target", &dir) == CMD_OK)
		player_confuse_dir(player, &dir, false);
	else
		return;

	ranged_helper(player, obj, dir, range, shots, attack, ranged_hit_types,
				  (int) N_ELEMENTS(ranged_hit_types));
}


/**
 * Throw an object from the quiver, pack, floor, or, in limited circumstances,
 * the equipment.
 */
void do_cmd_throw(struct command *cmd) {
	int dir;
	int shots = 10;
	int str = adj_str_blow[player->state.stat_ind[STAT_STR]];
	ranged_attack attack = make_ranged_throw;

	int weight;
	int range;
	struct object *obj;

	if (!player_get_resume_normal_shape(player, cmd)) {
		return;
	}

	/*
	 * Get arguments.  Never default to showing the equipment as the first
	 * list (since throwing the equipped weapon leaves that slot empty will
	 * have to choose another source anyways).
	 */
	if (player->upkeep->command_wrk == USE_EQUIP)
		player->upkeep->command_wrk = USE_INVEN;
	if (cmd_get_item(cmd, "item", &obj,
			/* Prompt */ "Throw which item?",
			/* Error  */ "You have nothing to throw.",
			/* Filter */ obj_can_throw,
			/* Choice */ USE_EQUIP | USE_QUIVER | USE_INVEN | USE_FLOOR | SHOW_THROWING)
		!= CMD_OK)
		return;

	if (cmd_get_target(cmd, "target", &dir) == CMD_OK)
		player_confuse_dir(player, &dir, false);
	else
		return;

	if (object_is_equipped(player->body, obj)) {
		assert(obj_can_takeoff(obj) && tval_is_melee_weapon(obj));
		inven_takeoff(obj);
	}

	weight = MAX(obj->weight, 10);
	range = MIN(((str + 20) * 10) / weight, 10);

	ranged_helper(player, obj, dir, range, shots, attack, ranged_hit_types,
				  (int) N_ELEMENTS(ranged_hit_types));
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
