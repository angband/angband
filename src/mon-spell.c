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
#include "effects.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "spells.h"

/**
 * Info details of the different monster spells in the game.
 */
static const struct mon_spell_info {
	u16b index;				/* Numerical index (RSF_FOO) */
	int type;				/* Type bitflag */
	const char *desc;		/* Verbal description */
	int msgt;				/* Flag for message colouring */
	bool save;				/* Does this attack allow a saving throw? */
	const char *verb;		/* Description of the attack */
	const char *blind_verb;	/* Description of the attack if unseen */
	const char *lore_desc;	/* Description of the attack used in lore text */
} mon_spell_info_table[] = {
    #define RSF(a, b, c, d, e, f, g, h)	{ RSF_##a, b, c, d, e, f, g, h },
    #include "list-mon-spells-new.h"
    #undef RSF
};


/**
 * Details of the different monster spells in the game.
 * See src/monster.h for structure
 */
static const struct mon_spell mon_spell_table[] =
{
    #define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m, n)		\
			{ RSF_##a, b, c, d, e, f, g, h, i, j, k, l, m, n },
	#define RV(b, x, y, m) {b, x, y, m}
    #include "list-mon-spells.h"
    #undef RSF
	#undef RV
};


/**
 * Details of the different side effects of spells.
 * See src/monster.h for structure
 */
static const struct spell_effect spell_effect_table[] =
{
    #define RSE(a, b, c, d, e, f, g, h) \
			{ RSE_##a, b, c, d, e, f, g, h },
	#define RV(b, x, y, m) {b, x, y, m}
    #include "list-spell-effects.h"
    #undef RSE
	#undef RV
};


static const struct breath_dam {
	int divisor;
	int cap;
} breath[] = {
    #define ELEM(a, b, c, d, e, f, g, col, h, fh, oh, mh, ph) { f, g },
    #include "list-elements.h"
    #undef ELEM
};

/**
 * Determine the damage of a spell attack which ignores monster hp
 * (i.e. bolts and balls, including arrows/boulders/storms/etc.)
 *
 * \param spell is the attack type
 * \param rlev is the monster level of the attacker
 * \param dam_aspect is the damage calc required (min, avg, max, random)
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
		dam += damcalc(MAX(1, rs_ptr->rlev_dam.dice * rlev / 100), 
				rs_ptr->rlev_dam.sides, dam_aspect);
	else /* rlev affects sides */
		dam += damcalc(rs_ptr->rlev_dam.dice, rs_ptr->rlev_dam.sides *
				rlev / 100, dam_aspect);

	return dam;
}

/**
 * Determine the damage of a monster breath attack
 *
 * \param element is the attack element
 * \param hp is the monster's hp
 */
static int breath_dam(int element, int hp)
{
	int dam;

	/* Damage is based on monster's current hp */
	dam = hp / breath[element].divisor;

	/* Check for maximum damage */
	if (dam > breath[element].cap)
		dam = breath[element].cap;

	return dam;
}

/**
 * Drain mana from the player, healing the caster.
 *
 * \param m_ptr is the monster casting
 * \param rlev is its level
 * \param seen is whether @ can see it
 */
static void drain_mana(struct monster *m_ptr, int rlev, bool seen)
{
	int r1;
	char m_name[80];

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_STANDARD);

	if (!player->csp) {
		msg("The draining fails.");
		if (OPT(birth_ai_learn) && !(m_ptr->smart & SM_IMM_MANA)) {
			msg("%s notes that you have no mana!", m_name);
			m_ptr->smart |= SM_IMM_MANA;
		}
		return;
	}

	/* Attack power */
	r1 = (randint1(rlev) / 2) + 1;

	/* Full drain */
	if (r1 >= player->csp) {
		r1 = player->csp;
		player->csp = 0;
		player->csp_frac = 0;
	}
	/* Partial drain */
	else
		player->csp -= r1;

	/* Redraw mana */
	player->upkeep->redraw |= PR_MANA;

	/* Heal the monster */
	if (m_ptr->hp < m_ptr->maxhp) {
		m_ptr->hp += (6 * r1);
		if (m_ptr->hp > m_ptr->maxhp)
			m_ptr->hp = m_ptr->maxhp;

		/* Redraw (later) if needed */
		if (player->upkeep->health_who == m_ptr)
			player->upkeep->redraw |= (PR_HEALTH);

		/* Special message */
		if (seen)
			msg("%s appears healthier.", m_name);
	}
}

/**
 * Monster self-healing.
 *
 * \param m_ptr is the monster casting
 * \param rlev is its level
 * \param seen is whether @ can see it
 */
static void heal_self(struct monster *m_ptr, int rlev, bool seen)
{
	char m_name[80], m_poss[80];

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_STANDARD);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), m_ptr,
			MDESC_PRO_VIS | MDESC_POSS);

	/* Heal some */
	m_ptr->hp += (rlev * 6);

	/* Fully healed */
	if (m_ptr->hp >= m_ptr->maxhp) {
		m_ptr->hp = m_ptr->maxhp;

		if (seen)
			msg("%s looks REALLY healthy!", m_name);
		else
			msg("%s sounds REALLY healthy!", m_name);
	} else if (seen) { /* Partially healed */
		msg("%s looks healthier.", m_name);
	} else {
		msg("%s sounds healthier.", m_name);
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == m_ptr)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Cancel fear */
	if (m_ptr->m_timed[MON_TMD_FEAR]) {
		mon_clear_timed(m_ptr, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE, FALSE);
		msg("%s recovers %s courage.", m_name, m_poss);
	}
}

/**
 *	This function is used when a group of monsters is summoned.
 */
static int summon_monster_aux(int flag, struct monster *m_ptr, int rlev, int summon_max)
{
	int count = 0, val = 0, attempts = 0;
	int temp;

	/* Continue summoning until we reach the current dungeon level */
	while ((val < player->depth * rlev) && (attempts < summon_max)) {
		/* Get a monster */
		temp = summon_specific(m_ptr->fy, m_ptr->fx, rlev, flag, 0);

		val += temp * temp;

		/* Increase the attempt, needed in case no monsters were available. */
		attempts++;

		/* Increase count of summoned monsters */
		if (val > 0)
			count++;
	}

	/* In the special case that uniques or wraiths were summoned but all were
	 * dead S_HI_UNDEAD is used instead (note lack of infinite recursion) */
	if ((!count) && ((flag == S_WRAITH) || (flag == S_UNIQUE)))
		count = summon_monster_aux(S_HI_UNDEAD, m_ptr, rlev, summon_max);

	return(count);
}

/**
 * Apply side effects from a spell attack to the player
 *
 * \param spell is the attack type
 * \param dam is the amount of damage caused by the attack
 * \param m_ptr is the attacking monster
 * \param seen is whether @ can see it
 */
static void do_spell_effects(int spell, int dam, struct monster *m_ptr,
							 bool seen)
{
	const struct spell_effect *re_ptr;
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];

	int dur = 0, count = 0;

	bool chosen[RSE_MAX] = { 0 };

	/* Extract the monster level */
	int rlev = ((m_ptr->race->level >= 1) ? m_ptr->race->level : 1);

	/* First we note all the effects we'll be doing. */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++)
		if (re_ptr->method && (re_ptr->method == rs_ptr->index))
			chosen[re_ptr->index] = TRUE;

	/* Now we cycle through again to activate the chosen effects */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++) {
		if (chosen[re_ptr->index]) {
			/* Check for protection from the effect */
			if (re_ptr->prot_flag)
				update_smart_learn(m_ptr, player, re_ptr->prot_flag, -1);

			if (player_of_has(player, re_ptr->prot_flag)) {
				msg("You resist the effect!");
				continue;
			}

			/* Implement the effect */
			if (re_ptr->timed) {

				/* Calculate base duration (m_bonus is not used) */
				dur = randcalc(re_ptr->base, 0, RANDOMISE);

				/* Apply the effect - we have already checked for resistance */
				(void)player_inc_timed(player, re_ptr->flag, dur, TRUE, FALSE);

			} else {
				switch (re_ptr->flag) {
					case S_TELEPORT:
						teleport_player(randcalc(re_ptr->base, 0, RANDOMISE));
						break;

					case S_TELE_TO:
						teleport_player_to(m_ptr->fy, m_ptr->fx);
						break;

					case S_TELE_LEV:
						if (player_resists(player, ELEM_NEXUS))
							msg("You resist the effect!");
						else
							teleport_player_level();
						break;

					case S_TELE_SELF:
						teleport_away(m_ptr, randcalc(re_ptr->base, 0,
							RANDOMISE));
						break;

					case S_DRAIN_MANA:
						drain_mana(m_ptr, rlev, seen);
						break;

					case S_HEAL:
						heal_self(m_ptr, rlev, seen);
						break;

					case S_DARKEN:
						(void)unlight_area(0, 3);
						break;

					case S_TRAPS:
						(void)trap_creation();
						break;

					case S_AGGRAVATE:
						aggravate_monsters(m_ptr);
						break;

					case S_KIN:
						summon_kin_type = m_ptr->race->d_char;
					case S_MONSTER:	case S_MONSTERS:
					case S_SPIDER: case S_HOUND: case S_HYDRA: case S_AINU:
					case S_ANIMAL:
					case S_DEMON: case S_HI_DEMON:
					case S_UNDEAD: case S_HI_UNDEAD: 
					case S_DRAGON: case S_HI_DRAGON:					
					case S_UNIQUE: case S_WRAITH:
						count = summon_monster_aux(re_ptr->flag, m_ptr,
												   rlev, re_ptr->base.base);

						if (count && player->timed[TMD_BLIND])
							msgt(rs_ptr->msgt, "You hear %s appear nearby.",
								(count > 1 ? "many things" : "something"));
							
						if (!count)
							msg("But nothing comes.");

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
		return breath_dam(rs_ptr->gf, hp);
	else
		return nonhp_dam(spell, rlev, dam_aspect);
}


/**
 * Process a monster spell 
 *
 * \param spell is the monster spell flag (RSF_FOO)
 * \param m_ptr is the attacking monster
 * \param seen is whether the player can see the monster at this moment
 */
void do_mon_spell(int spell, struct monster *m_ptr, bool seen)
{
	const struct mon_spell *rs_ptr = &mon_spell_table[spell];

	char m_name[80], ddesc[80];

	bool hits = FALSE;

	int dam = 0, flag = 0, rad = 0;

	/* Extract the monster level */
	int rlev = ((m_ptr->race->level >= 1) ? m_ptr->race->level : 1);

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_STANDARD);

	/* See if it hits */
	if (rs_ptr->hit == 100) 
		hits = TRUE;
	else if (rs_ptr->hit == 0)
		hits = FALSE;
	else
		hits = check_hit(player, rs_ptr->hit, rlev);

	/* Tell the player what's going on */
	disturb(player, 1);

	if (!seen)
		msg("Something %s.", rs_ptr->blind_verb);
	else if (!hits) {
		msg("%s %s %s, but misses.", m_name, rs_ptr->verb,	rs_ptr->desc);
		return;
	} else if (rs_ptr->msgt)
		msgt(rs_ptr->msgt, "%s %s %s.", m_name, rs_ptr->verb, rs_ptr->desc);
	else 
		msg("%s %s %s.", m_name, rs_ptr->verb, rs_ptr->desc);


	/* Try a saving throw if available */
	if (rs_ptr->save && randint0(100) < player->state.skills[SKILL_SAVE]) {
		msg("You avoid the effects!");
		return;
	}

	/* Calculate the damage */
	dam = mon_spell_dam(spell, m_ptr->hp, rlev, RANDOMISE);

	/* Get the "died from" name in case this attack kills @ */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_DIED_FROM);

	/* Display the attack, adjust for resists and apply effects */
	if (rs_ptr->type & RST_BOLT)
		flag = PROJECT_STOP | PROJECT_KILL;
	else if (rs_ptr->type & (RST_BALL | RST_BREATH)) {
		flag = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		rad = rf_has(m_ptr->race->flags, RF_POWERFUL) ? 3 : 2;
	}

	if (rs_ptr->gf) {
		(void)project(m_ptr->midx, rad, player->py, player->px, dam,
					  rs_ptr->gf, flag, 0, 0);
		update_smart_learn(m_ptr, player, 0, rs_ptr->gf);
	}
	else {
		/* Note that non-projectable attacks are unresistable */
		take_hit(player, dam, ddesc);
		do_spell_effects(spell, dam, m_ptr, seen);
	}

	return;
}

/**
 * Test a spell bitflag for a type of spell.
 * Returns TRUE if any desired type is among the flagset
 *
 * \param f is the set of spell flags we're testing
 * \param types is the spell type(s) we're looking for
 */
bool test_spells(bitflag *f, int types)
{
	const struct mon_spell *rs_ptr;

	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rsf_has(f, rs_ptr->index) && (rs_ptr->type & types))
			return TRUE;

	return FALSE;
}

/**
 * Set a spell bitflag to allow only a specific set of spell types.
 *
 * \param f is the set of spell flags we're pruning
 * \param types is the spell type(s) we're allowing
 */
void set_spells(bitflag *f, int types)
{
	const struct mon_spell *rs_ptr;

	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++)
		if (rsf_has(f, rs_ptr->index) && !(rs_ptr->type & types))
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
void unset_spells(bitflag *spells, bitflag *flags, struct element_info *el,
				  const monster_race *r_ptr)
{
	const struct mon_spell *rs_ptr;
	const struct spell_effect *re_ptr;

	/* First we test the gf (projectable) spells */
	for (rs_ptr = mon_spell_table; rs_ptr->index < RSF_MAX; rs_ptr++) {
		/* Hack - DARK_WEAK is not an element - needs fix - NRM */
		if (rs_ptr->gf == GF_DARK_WEAK) return;

		if (rs_ptr->gf && randint0(100) < el[rs_ptr->gf].res_level * 
			(rf_has(r_ptr->flags, RF_SMART) ? 2 : 1) * 25)
			rsf_off(spells, rs_ptr->index);
	}

	/* ... then we test the non-gf side effects */
	for (re_ptr = spell_effect_table; re_ptr->index < RSE_MAX; re_ptr++)
		if (re_ptr->method && re_ptr->prot_flag && (rf_has(r_ptr->flags,
				RF_SMART) || !one_in_(3)) && of_has(flags, re_ptr->prot_flag))
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
				dam = adjust_dam(rs_ptr->gf, dam, MAXIMISE, 1);

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

static bool mon_spell_is_valid(int spell)
{
	return spell > RSF_NONE && spell < RSF_MAX;
}

static bool mon_spell_has_damage(int spell)
{
	return mon_spell_table[spell].type & (RST_BOLT | RST_BALL | RST_BREATH | RST_ATTACK);
}

const char *mon_spell_lore_description(int spell)
{
	if (!mon_spell_is_valid(spell))
		return "";

	return mon_spell_table[spell].lore_desc;
}

int mon_spell_lore_damage(int spell, const monster_race *race, bool know_hp)
{
	int hp;

	if (!mon_spell_is_valid(spell) || !mon_spell_has_damage(spell))
		return 0;

	hp = (know_hp) ? race->avg_hp : 0;
	return mon_spell_dam(spell, hp, race->level, MAXIMISE);
}
