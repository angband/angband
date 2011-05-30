/*
 * File: mon-spell.c
 * Purpose: functions to deal with spell attacks and effects
 *
 * Copyright (c) 2010-11 Chris Carr
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
#include "spells.h"
#include "effects.h"
#include "monster/mon-spell.h"

/**
 * Details of the different monster spells in the game.
 * See src/monster/monster.h for structure
 */
const struct mon_spell mon_spell_table[] =
{
    #define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m) \
			{ RSF_##a, b, c, d, e, f, g, h, i, j, k, l, m },
		#define RV(b, x, y, m) {b, x, y, m}
    #include "list-mon-spells.h"
    #undef RSF
		#undef RV
};


/**
 * Details of the different side effects of spells.
 * See src/monster/monster.h for structure
 */
const struct spell_effect spell_effect_table[] =
{
    #define RSE(a, b, c, d, e, f, g, h, i, j, k) \
			{ RSE_##a, b, c, d, e, f, g, h, i, j, k },
		#define RV(b, x, y, m) {b, x, y, m}
    #include "list-spell-effects.h"
    #undef RSE
		#undef RV
};


/**
 * Determine the damage of a spell attack which ignores monster hp
 * (i.e. bolts and balls, including arrows/boulders/storms/etc.)
 *
 * \param spell is the attack type
 * \param rlev is the monster level of the attacker
 * \param aspect is the damage calc required (min, avg, max, random)
 */
static int nonhp_dam(int spell, int rlev, aspect dam_aspect)
{
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];
	int dam;

	/* base damage is X + YdZ (m_bonus is not used) */
	dam = randcalc(rs_ptr->base_dam, 0, dam_aspect);

	/* rlev-dependent damage (m_bonus is used as a switch) */
	dam += (rlev * rs_ptr->rlev_dam.base / 100);

	if (rs_ptr->rlev_dam.m_bonus == 1) /* then rlev affects dice */
		dam += damcalc(MIN(1, rs_ptr->rlev_dam.dice * rlev / 100), 
				rs_ptr->rlev_dam.sides, dam_aspect);
	else /* rlev affects sides */
		dam += damcalc(rs_ptr->rlev_dam.dice, rs_ptr->rlev_dam.sides *
				rlev / 100, dam_aspect);

	return dam;
}

/**
 * Determine the damage of a monster attack which depends on its hp
 *
 * \param spell is the attack type
 * \param hp is the monster's hp
 */
static int hp_dam(int spell, int hp)
{
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];
	int dam;

	/* Damage is based on monster's current hp */
	dam = hp / rs_ptr->div;

	/* Check for maximum damage */
	if (dam > rs_ptr->cap)
		dam = rs_ptr->cap;

	return dam;
}

/**
 * Drain stats at random
 *
 * \param num is the number of points to drain
 * \param sustain is whether sustains will prevent draining
 * \param perma is whether the drains are permanent
 */
static void drain_stats(int num, bool sustain, bool perma)
{
	int i, k = 0;
	const char *act = NULL;

	for (i = 0; i < num; i++) {
		switch (randint1(6)) {
			case 1: k = A_STR; act = "strong"; break;
			case 2: k = A_INT; act = "bright"; break;
			case 3: k = A_WIS; act = "wise"; break;
			case 4: k = A_DEX; act = "agile"; break;
			case 5: k = A_CON; act = "hale"; break;
			case 6: k = A_CHR; act = "beautiful"; break;
		}

		if (sustain)
			do_dec_stat(k, perma);
		else {
			msg("You're not as %s as you used to be...", act);
			player_stat_dec(p_ptr, k, perma);
		}
	}

	return;
}

/**
 * Swap a random pair of stats
 */
static void swap_stats(void)
{
    int max1, cur1, max2, cur2, ii, jj;

    msg("Your body starts to scramble...");

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

    return;
}

/**
 * Drain mana from the player, healing the caster.
 *
 * \param m_idx is the monster casting
 * \param rlev is its level
 * \param seen is whether @ can see it
 */
static void drain_mana(int m_idx, int rlev, bool seen)
{
	monster_type *m_ptr = cave_monster(cave, m_idx);
	int r1;
	char m_name[80];

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0x00);

	if (!p_ptr->csp) {
		msg("The draining fails.");
		if (OPT(birth_ai_learn)) {
			msg("%^s notes that you have no mana!", m_name);
			m_ptr->smart |= SM_IMM_MANA;
		}
		return;
	}

	/* Attack power */
	r1 = (randint1(rlev) / 2) + 1;

	/* Full drain */
	if (r1 >= p_ptr->csp) {
		r1 = p_ptr->csp;
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;
	}
	/* Partial drain */
	else
		p_ptr->csp -= r1;

	/* Redraw mana */
	p_ptr->redraw |= PR_MANA;

	/* Heal the monster */
	if (m_ptr->hp < m_ptr->maxhp) {
		m_ptr->hp += (6 * r1);
		if (m_ptr->hp > m_ptr->maxhp)
			m_ptr->hp = m_ptr->maxhp;

		/* Redraw (later) if needed */
		if (p_ptr->health_who == m_idx)
			p_ptr->redraw |= (PR_HEALTH);

		/* Special message */
		if (seen)
			msg("%^s appears healthier.", m_name);
	}
}

/**
 * Monster self-healing.
 *
 * \param m_idx is the monster casting
 * \param rlev is its level
 * \param seen is whether @ can see it
 */
static void heal_self(int m_idx, int rlev, bool seen)
{
	monster_type *m_ptr = cave_monster(cave, m_idx);
	char m_name[80], m_poss[80];

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0x00);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO2 | MDESC_POSS);

	/* Heal some */
	m_ptr->hp += (rlev * 6);

	/* Fully healed */
	if (m_ptr->hp >= m_ptr->maxhp) {
		m_ptr->hp = m_ptr->maxhp;

		if (seen)
			msg("%^s looks REALLY healthy!", m_name);
		else
			msg("%^s sounds REALLY healthy!", m_name);
	} else if (seen) /* Partially healed */
		msg("%^s looks healthier.", m_name);
	else
		msg("%^s sounds healthier.", m_name);

	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

	/* Cancel fear */
	if (m_ptr->m_timed[MON_TMD_FEAR]) {
		mon_clear_timed(m_idx, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE);
		msg("%^s recovers %s courage.", m_name, m_poss);
	}
}

/**
 * Apply side effects from a spell attack to the player
 *
 * \param spell is the attack type
 * \param dam is the amount of damage caused by the attack
 * \param m_idx is the attacking monster
 * \param rlev is its level
 * \param seen is whether @ can see it
 */
static void do_side_effects(int spell, int dam, int m_idx, bool seen)
{
	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	const struct spell_effect *re_ptr;
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];

	int i, choice[99], dur = 0, j = 0, count = 0;
	s32b d = 0;

	bool sustain = FALSE, perma = FALSE, chosen[RSE_MAX] = { 0 };

	/* Extract the monster level */
	int rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* First we note all the effects we'll be doing. */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++) {
		if ((re_ptr->method && (re_ptr->method == rs_ptr->index)) ||
				(re_ptr->gf && (re_ptr->gf == rs_ptr->gf))) {

			/* If we have a choice of effects, we create a cum freq table */
			if (re_ptr->chance) {
				for (i = j; i < (j + re_ptr->chance); i++)
					choice[i] = re_ptr->index;
				j = i;
			}
			else
				chosen[re_ptr->index] = TRUE;
		}
	}

	/* If we have built a cum freq table, choose an effect from it */
	if (j)
		chosen[choice[randint0(j)]] = TRUE;

	/* Now we cycle through again to activate the chosen effects */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++) {
		if (chosen[re_ptr->index]) {

			/*
			 * Check for resistance - there are three possibilities:
			 * 1. Immunity to the attack type if side_immune is TRUE
			 * 2. Resistance to the attack type if it affords no immunity
			 * 3. Resistance to the specific side-effect
			 *
			 * TODO - add interesting messages to the RSE_ and GF_ tables
			 * to replace the generic ones below. (See #1376)
			 */
			if (re_ptr->res_flag)
				update_smart_learn(m_ptr, p_ptr, re_ptr->res_flag);

			if ((rs_ptr->gf && check_side_immune(rs_ptr->gf)) ||
					check_state(p_ptr, re_ptr->res_flag, p_ptr->state.flags)) {
				msg("You resist the effect!");
				continue;
			}

			/* Allow saving throw if available */
			if (re_ptr->save &&
					randint0(100) < p_ptr->state.skills[SKILL_SAVE]) {
				msg("You avoid the effect!");
				continue;
			}

			/* Implement the effect */
			if (re_ptr->timed) {

				/* Calculate base duration (m_bonus is not used) */
				dur = randcalc(re_ptr->base, 0, RANDOMISE);

				/* Calculate the damage-dependent duration (m_bonus is
				 * used as a cap) */
				dur += damcalc(re_ptr->dam.dice, re_ptr->dam.sides *
						dam / 100, RANDOMISE);

				if (re_ptr->dam.m_bonus && (dur > re_ptr->dam.m_bonus))
					dur = re_ptr->dam.m_bonus;

				/* Apply the effect - we have already checked for resistance */
				(void)player_inc_timed(p_ptr, re_ptr->flag, dur, TRUE, FALSE);

			} else {
				switch (re_ptr->flag) {
					case S_INV_DAM:
						if (dam > 0)
							inven_damage(p_ptr, re_ptr->gf, MIN(dam *
								randcalc(re_ptr->dam, 0, RANDOMISE), 300));
						break;

					case S_TELEPORT: /* m_bonus is used as a clev filter */
						if (!re_ptr->dam.m_bonus || 
								randint1(re_ptr->dam.m_bonus) > p_ptr->lev)
							teleport_player(randcalc(re_ptr->base, 0,
								RANDOMISE));
						break;

					case S_TELE_TO:
						teleport_player_to(m_ptr->fy, m_ptr->fx);
						break;

					case S_TELE_LEV:
						teleport_player_level();
						break;

					case S_TELE_SELF:
						teleport_away(m_ptr, randcalc(re_ptr->base, 0,
							RANDOMISE));
						break;

					case S_DRAIN_LIFE:
						d = re_ptr->base.base + (p_ptr->exp *
							re_ptr->base.sides / 100) * MON_DRAIN_LIFE;

						msg("You feel your life force draining away!");
						player_exp_lose(p_ptr, d, FALSE);
						break;

					case S_DRAIN_STAT: /* m_bonus is used as a flag */
						if (re_ptr->dam.m_bonus > 0)
							sustain = TRUE;

						if (abs(re_ptr->dam.m_bonus) > 1)
							perma = TRUE;

						drain_stats(randcalc(re_ptr->base, 0, RANDOMISE),
							sustain, perma);
						break;

					case S_SWAP_STAT:
						swap_stats();
						break;

					case S_DRAIN_ALL:
						msg("You're not as powerful as you used to be...");

						for (i = 0; i < A_MAX; i++)
							player_stat_dec(p_ptr, i, FALSE);
						break;

					case S_DISEN:
						(void)apply_disenchant(0);
						break;

					case S_DRAIN_MANA:
						drain_mana(m_idx, rlev, seen);
						break;

					case S_HEAL:
						heal_self(m_idx, rlev, seen);
						break;

					case S_DARKEN:
						(void)unlight_area(0, 3);
						break;

					case S_TRAPS:
						(void)trap_creation();
						break;

					case S_AGGRAVATE:
						aggravate_monsters(m_idx);
						break;

					case S_KIN:
						summon_kin_type = r_ptr->d_char;
					case S_MONSTER:	case S_MONSTERS:
					case S_SPIDER: case S_HOUND: case S_HYDRA: case S_ANGEL:
					case S_ANIMAL:
					case S_DEMON: case S_HI_DEMON:
					case S_UNDEAD: case S_HI_UNDEAD: case S_WRAITH:
					case S_DRAGON: case S_HI_DRAGON:
					case S_UNIQUE:
						for (i = 0; i < re_ptr->base.base; i++)
							count += summon_specific(m_ptr->fy, m_ptr->fx,
								rlev, re_ptr->flag, 0);

						for (i = 0; i < re_ptr->dam.base; i++)
							count += summon_specific(m_ptr->fy, m_ptr->fx,
								rlev, S_HI_UNDEAD, 0);

						if (count && p_ptr->timed[TMD_BLIND])
							msgt(rs_ptr->msgt, "You hear %s appear nearby.",
								(count > 1 ? "many things" : "something"));

					default:
						break;
				}		
			}
		}
	}
	return;
}

/**
 * Calculate the damage of a monster spell.
 *
 * \param spell is the spell in question.
 * \param hp is the hp of the casting monster.
 * \param rlev is the level of the casting monster.
 * \param dam_aspect is the damage calc we want (min, max, avg, random).
 */
static int mon_spell_dam(int spell, int hp, int rlev, aspect dam_aspect)
{
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];

	if (rs_ptr->div)
		return hp_dam(spell, hp);
	else
		return nonhp_dam(spell, rlev, dam_aspect);
}


/**
 * Process a monster spell 
 *
 * \param spell is the monster spell flag (RSF_FOO)
 * \param m_idx is the attacking monster
 * \param seen is whether the player can see the monster at this moment
 */
void do_mon_spell(int spell, int m_idx, bool seen)
{
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	char m_name[80], ddesc[80];

	bool hits = FALSE;

	int dam = 0, flag = 0, rad = 0;

	/* Extract the monster level */
	int rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0x00);

	/* See if it hits */
	if (rs_ptr->hit == 100) 
		hits = TRUE;
	else if (rs_ptr->hit == 0)
		hits = FALSE;
	else
		hits = check_hit(p_ptr, rs_ptr->hit, rlev);

	/* Tell the player what's going on */
	disturb(p_ptr, 1,0);

	if (!seen)
		msg("Something %s.", rs_ptr->blind_verb);
	else if (!hits) {
		msg("%^s %s %s, but misses.", m_name, rs_ptr->verb,	rs_ptr->desc);
		return;
	} else if (rs_ptr->msgt)
		msgt(rs_ptr->msgt, "%^s %s %s.", m_name, rs_ptr->verb, rs_ptr->desc);
	else 
		msg("%^s %s %s.", m_name, rs_ptr->verb, rs_ptr->desc);


	/* Try a saving throw if available */
	if (rs_ptr->save && randint0(100) < p_ptr->state.skills[SKILL_SAVE]) {
		msg("You avoid the effects!");
		return;
	}

	/* Calculate the damage */
	dam = mon_spell_dam(spell, m_ptr->hp, rlev, RANDOMISE);

	/* Get the "died from" name in case this attack kills @ */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND2);

	/* Display the attack, adjust for resists and apply effects */
	if (rs_ptr->type & RST_BOLT)
		flag = PROJECT_STOP | PROJECT_KILL;
	else if (rs_ptr->type & (RST_BALL | RST_BREATH)) {
		flag = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		rad = rf_has(r_ptr->flags, RF_POWERFUL) ? 3 : 2;
	}

	if (rs_ptr->gf) {
		(void)project(m_idx, rad, p_ptr->py, p_ptr->px, dam, rs_ptr->gf, flag);
		monster_learn_resists(m_ptr, p_ptr, rs_ptr->gf);
	}
	else /* Note that non-projectable attacks are unresistable */
		take_hit(p_ptr, dam, ddesc);

	do_side_effects(spell, dam, m_idx, seen);

	return;
}

/**
 * Test a spell bitflag for a type of spell.
 * Returns TRUE if any desired type is among the flagset
 *
 * \param f is the set of spell flags we're testing
 * \param type is the spell type(s) we're looking for
 */
bool test_spells(bitflag *f, enum mon_spell_type type)
{
	const struct mon_spell *rs_ptr;

	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rsf_has(f, rs_ptr->index) && (rs_ptr->type & type))
			return TRUE;

	return FALSE;
}

/**
 * Set a spell bitflag to allow only a specific set of spell types.
 *
 * \param f is the set of spell flags we're pruning
 * \param type is the spell type(s) we're allowing
 */
void set_spells(bitflag *f, enum mon_spell_type type)
{
	const struct mon_spell *rs_ptr;

	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rsf_has(f, rs_ptr->index) && !(rs_ptr->type & type))
			rsf_off(f, rs_ptr->index);

	return;
}

/**
 * Turn off spells with a side effect or a gf_type that is resisted by
 * something in flags, subject to intelligence and chance.
 *
 * \param spells is the set of spells we're pruning
 * \param flags is the set of flags we're testing
 * \param r_ptr is the monster type we're operating on
 */
void unset_spells(bitflag *spells, bitflag *flags, const monster_race *r_ptr)
{
	const struct mon_spell *rs_ptr;
	const struct spell_effect *re_ptr;

	/* First we test the gf (projectable) spells */
	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rs_ptr->gf && randint0(100) < check_for_resist(p_ptr, rs_ptr->gf, flags,
				FALSE) * (rf_has(r_ptr->flags, RF_SMART) ? 2 : 1) * 25)
			rsf_off(spells, rs_ptr->index);

	/* ... then we test the non-gf side effects */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++)
		if (re_ptr->method && re_ptr->res_flag && (rf_has(r_ptr->flags,
				RF_SMART) || !one_in_(3)) && of_has(flags, re_ptr->res_flag))
			rsf_off(spells, re_ptr->method);
}

/**
 * Calculate a monster's maximum spell power.
 *
 * \param r_ptr is the monster we're studying
 * \param resist is the degree of resistance we're assuming to any
 *   attack type (-1 = vulnerable ... 3 = immune)
 */
int best_spell_power(const monster_race *r_ptr, int resist)
{
	const struct mon_spell *rs_ptr;
	const struct spell_effect *re_ptr;
	int dam = 0, best_dam = 0; 

	/* Extract the monster level */
	int rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++) {
		if (rsf_has(r_ptr->spell_flags, rs_ptr->index)) {

			/* Get the maximum basic damage output of the spell (could be 0) */
			dam = mon_spell_dam(rs_ptr->index, mon_hp(r_ptr, MAXIMISE), rlev,
				MAXIMISE);

			/* For all attack forms the player can save against, damage
			 * is halved */
			if (rs_ptr->save)
				dam /= 2;

			/* Adjust the real damage by the assumed resistance (if it is a
			 * resistable type) */
			if (rs_ptr->gf)
				dam = adjust_dam(p_ptr, rs_ptr->gf, dam, MAXIMISE, resist);

			/* Add the power ratings assigned to the various possible spell
			 * effects (which is crucial for non-damaging spells) */
			for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++) {
				if ((re_ptr->method && (re_ptr->method == rs_ptr->index)) ||
						(re_ptr->gf && (re_ptr->gf == rs_ptr->gf))) {

					/* First we adjust the real damage if necessary */
					if (re_ptr->power.dice)
						dam = (dam * re_ptr->power.dice) / 100;

					/* Then we add any flat rating for this effect */
					dam += re_ptr->power.base;

					/* Then we add any rlev-dependent rating */
					if (re_ptr->power.m_bonus < 0)
						dam += re_ptr->power.sides / (rlev + 1);
					else if (re_ptr->power.m_bonus > 0)
						dam += (re_ptr->power.sides * rlev) / 100;
				}
			}

			/* Update the best_dam tracker */
			if (dam > best_dam)
				best_dam = dam;
		}
	}

	return best_dam;
}

