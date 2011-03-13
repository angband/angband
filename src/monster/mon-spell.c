/*
 * File: mon-spell.c
 * Purpose: functions to deal with spell attacks and effects
 *
 * Copyright (c) 2010 Chris Carr
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
    #define RSE(a, b, c, d, e, f, g, h, i, j) \
			{ RSE_##a, b, c, d, e, f, g, h, i, j },
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
 * \param m_idx is the monster attacking
 */
static int hp_dam(int spell, int m_idx)
{
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];
	int dam;

	monster_type *m_ptr = &mon_list[m_idx];

	/* Damage is based on monster's current hp */
	dam = m_ptr->hp / rs_ptr->div;

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
void drain_stats(int num, bool sustain, bool perma)
{
	int i, k;
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
 * Apply side effects from a spell attack to the player
 *
 * \param spell is the attack type
 * \param dam is the amount of damage caused by the attack
 * \param m_idx is the attacking monster
 */
void do_side_effects(int spell, int dam, int m_idx)
{
	const struct spell_effect *re_ptr;
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];
	monster_type *m_ptr = &mon_list[m_idx];
	int i, choice[99], dur = 0, j = 0;
	bool sustain = FALSE, perma = FALSE, chosen[RSE_MAX] = { 0 };
	s32b d = 0;

	/* First we note all the effects we'll be doing. */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++) {
		if ((re_ptr->method == rs_ptr->index) ||
				(re_ptr->gf == rs_ptr->gf)) {

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
			 * 1. Immunity to the attack type
			 * 2. Resistance to the attack type if it affords no immunity
			 * 3. Resistance to the specific side-effect
			 */
			if ((rs_ptr->gf && (check_for_resist(rs_ptr->gf) == 3)) ||
					(rs_ptr->gf && !immunity_possible(rs_ptr->gf) &&
					(check_for_resist(rs_ptr->gf) > 0)) ||
					p_ptr->state.flags[re_ptr->res_flag]) {
				msg("You resist the effect!");
				wieldeds_notice_flag(re_ptr->res_flag);
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

				if (dur > re_ptr->dam.m_bonus)
					dur = re_ptr->dam.m_bonus;

				/* Apply the effect */
				(void)inc_timed(re_ptr->flag, dur, TRUE);

			} else {
				switch (re_ptr->flag) {
					case S_INV_DAM:
						if (dam > 0)
							inven_damage(re_ptr->gf, MIN(dam *
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

					case S_DRAIN_LIFE:
						d = re_ptr->base.base + (p_ptr->exp *
							re_ptr->base.sides / 100) * MON_DRAIN_LIFE;						

						msg("You feel your life force draining away!");
						lose_exp(d);
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

					default:
						quit_fmt("Unknown side effect flag %d", re_ptr->flag);
				}		
			}
		}
	}
	return;
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

	monster_type *m_ptr = &mon_list[m_idx];
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
		hits = check_hit(rs_ptr->hit, rlev);

	/* Tell the player what's going on */
	disturb(1,0);

	if (!seen)
		msg("Something %s.", rs_ptr->blind_verb);
	else if (!hits) {
		msg("%^s %s %s, but misses.", m_name, rs_ptr->verb,	rs_ptr->desc);
		return;
	} else if (rs_ptr->type & RST_BREATH)
		msgt(rs_ptr->msgt, "%^s breathes %s.", m_name, rs_ptr->desc);
	else 
		msg("%^s %s %s.", m_name, rs_ptr->verb, rs_ptr->desc);


	/* Try a saving throw if available */
	if (rs_ptr->save && randint0(100) < p_ptr->state.skills[SKILL_SAVE]) {
		msg("You avoid the effects!");
		return;
	}

	/* Calculate the damage */
	if (rs_ptr->div)
		dam = hp_dam(spell, rlev);
	else
		dam = nonhp_dam(spell, rlev, RANDOMISE);

	/* Get the "died from" name in case this attack kills @ */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND2);

	/* Display the attack, adjust for resists and apply effects */
	if (rs_ptr->type & RST_BOLT)
		flag = PROJECT_STOP | PROJECT_KILL;
	else if (rs_ptr->type & (RST_BALL | RST_BREATH)) {
		flag = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		rad = rf_has(r_ptr->flags, RF_POWERFUL) ? 3 : 2;
	}

	if (rs_ptr->gf)
		(void)project(m_idx, rad, p_ptr->py, p_ptr->px, dam, rs_ptr->gf, flag);
	else /* Note that non-projectable attacks are unresistable */
		take_hit(dam, ddesc);

	do_side_effects(spell, dam, m_idx);

	/* Update monster awareness - XXX need to do opp/imm/vuln also */
/*	if (ra_ptr->res_flag)
		update_smart_learn(m_idx, ra_ptr->res_flag); */
}

/**
 * Test a spell bitflag for a type of spell.
 * Returns TRUE if any desired type is among the flagset
 *
 * \param f is the set of spell flags we're testing
 * \param type is the spell type(s) we're looking for
 */
bool test_spells(bitflag *f, mon_spell_type type)
{
	const struct mon_spell *rs_ptr;
	
	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rsf_has(f, rs_ptr->index) && (rs_ptr->type & type))
			return TRUE;

	return FALSE;
}

/**
 * Set a spell bitflag for a type of spell.
 *
 * \param f is the set of spell flags we're pruning
 * \param type is the spell type(s) we're allowing
 */
void set_spells(bitflag *f, mon_spell_type type)
{
	const struct mon_spell *rs_ptr;

	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rsf_has(f, rs_ptr->index) && !(rs_ptr->type & type))
			rsf_off(f, rs_ptr->index);

	return;
}
