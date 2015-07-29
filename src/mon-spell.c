/**
 * \file mon-spell.c
 * \brief Monster spell casting and selection
 *
 * Copyright (c) 2010-14 Chris Carr and Nick McConnell
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
#include "mon-attack.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"

/**
 * ------------------------------------------------------------------------
 * Spell casting
 * ------------------------------------------------------------------------ */
typedef enum {
	SPELL_TAG_NONE,
	SPELL_TAG_NAME,
	SPELL_TAG_PRONOUN
} spell_tag_t;

static spell_tag_t spell_tag_lookup(const char *tag)
{
	if (strncmp(tag, "name", 4) == 0)
		return SPELL_TAG_NAME;
	else if (strncmp(tag, "pronoun", 7) == 0)
		return SPELL_TAG_PRONOUN;
	else
		return SPELL_TAG_NONE;
}

/**
 * Print a monster spell message.
 *
 * We fill in the monster name and/or pronoun where necessary in
 * the message to replace instances of {name} or {pronoun}.
 */
static void spell_message(struct monster *mon,
						  const struct monster_spell *spell,
						  bool seen, bool hits)
{
	char buf[1024] = "\0";
	char m_name[80], m_poss[80];
	const char *next;
	const char *s;
	const char *tag;
	const char *in_cursor;
	size_t end = 0;

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), mon, MDESC_PRO_VIS | MDESC_POSS);

	/* Get the message */
	if (!seen)
		in_cursor = spell->blind_message;
	else if (!hits)
		in_cursor = spell->miss_message;
	else
		in_cursor = spell->message;

	next = strchr(in_cursor, '{');
	while (next) {
		/* Copy the text leading up to this { */
		strnfcat(buf, 1024, &end, "%.*s", next - in_cursor, in_cursor); 

		s = next + 1;
		while (*s && isalpha((unsigned char) *s)) s++;

		/* Valid tag */
		if (*s == '}') {
			/* Start the tag after the { */
			tag = next + 1;
			in_cursor = s + 1;

			switch(spell_tag_lookup(tag)) {
			case SPELL_TAG_NAME:
				strnfcat(buf, sizeof(buf), &end, m_name);
				break;
			case SPELL_TAG_PRONOUN:
				strnfcat(buf, sizeof(buf), &end, m_poss);
				break;
			default:
				break;
			}
		} else
			/* An invalid tag, skip it */
			in_cursor = next + 1;

		next = strchr(in_cursor, '{');
	}
	strnfcat(buf, 1024, &end, in_cursor);

	if (spell->msgt)
		msgt(spell->msgt, "%s", buf);
	else
		msg("%s", buf);
}

static const struct monster_spell *monster_spell_by_index(int index)
{
	const struct monster_spell *spell = monster_spells;
	while (spell) {
		if (spell->index == index)
			break;
		spell = spell->next;
	}
	return spell;
}

/**
 * Process a monster spell 
 *
 * \param index is the monster spell flag (RSF_FOO)
 * \param mon is the attacking monster
 * \param seen is whether the player can see the monster at this moment
 */
void do_mon_spell(int index, struct monster *mon, bool seen)
{
	char m_name[80];
	bool ident, hits = FALSE;

	/* Extract the monster level */
	int rlev = ((mon->race->level >= 1) ? mon->race->level : 1);

	const struct monster_spell *spell = monster_spell_by_index(index);

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* See if it hits */
	if (spell->hit == 100)
		hits = TRUE;
	else if (spell->hit == 0)
		hits = FALSE;
	else
		hits = check_hit(player, spell->hit, rlev);

	/* Tell the player what's going on */
	disturb(player, 1);
	spell_message(mon, spell, seen, hits);

	/* Try a saving throw if available */
	if (spell->save_message &&
		randint0(100) < player->state.skills[SKILL_SAVE]) {
		msg("%s", spell->save_message);
		return;
	}

	/* Do effects */
	effect_do(spell->effect, &ident, TRUE, 0, 0, 0);

	return;
}

/**
 * ------------------------------------------------------------------------
 * Spell selection
 * ------------------------------------------------------------------------ */
/**
 * Types of monster spells used for spell selection.
 */
static const struct mon_spell_info {
	u16b index;				/* Numerical index (RSF_FOO) */
	int type;				/* Type bitflag */
} mon_spell_types[] = {
    #define RSF(a, b)	{ RSF_##a, b },
    #include "list-mon-spells.h"
    #undef RSF
};


/**
 * Elemental damage properties of monster breaths.
 */
static const struct breath_damage {
	int divisor;
	int cap;
} breath[] = {
    #define ELEM(a, b, c, d, e, f, g, h, i, col) { g, h },
    #include "list-elements.h"
    #undef ELEM
};

static bool mon_spell_is_valid(int index)
{
	return index > RSF_NONE && index < RSF_MAX;
}

static bool monster_spell_is_projectable(int index)
{
	return (mon_spell_types[index].type &
			(RST_BOLT | RST_BALL | RST_BREATH)) ? TRUE : FALSE;
}

static bool monster_spell_is_breath(int index)
{
	return (mon_spell_types[index].type & RST_BREATH) ? TRUE : FALSE;
}

static bool mon_spell_has_damage(int index)
{
	return (mon_spell_types[index].type &
			(RST_BOLT | RST_BALL | RST_BREATH | RST_ATTACK)) ? TRUE : FALSE;
}

bool mon_spell_is_innate(int index)
{
	return mon_spell_types[index].type & (RST_INNATE);
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
	const struct mon_spell_info *info;

	for (info = mon_spell_types; info->index < RSF_MAX; info++)
		if (rsf_has(f, info->index) && (info->type & types))
			return TRUE;

	return FALSE;
}

/**
 * Set a spell bitflag to ignore a specific set of spell types.
 *
 * \param f is the set of spell flags we're pruning
 * \param types is the spell type(s) we're ignoring
 */
void ignore_spells(bitflag *f, int types)
{
	const struct mon_spell_info *info;

	for (info = mon_spell_types; info->index < RSF_MAX; info++)
		if (rsf_has(f, info->index) && (info->type & types))
			rsf_off(f, info->index);

	return;
}

/**
 * Turn off spells with a side effect or a gf_type that is resisted by
 * something in flags, subject to intelligence and chance.
 *
 * \param spells is the set of spells we're pruning
 * \param flags is the set of object flags we're testing
 * \param pflags is the set of player flags we're testing
 * \param el is what we know about the monster's elemental resists
 * \param race is the monster type we're operating on
 */
void unset_spells(bitflag *spells, bitflag *flags, bitflag *pflags,
				  struct element_info *el, const struct monster_race *race)
{
	const struct mon_spell_info *info;
	bool smart = rf_has(race->flags, RF_SMART);

	for (info = mon_spell_types; info->index < RSF_MAX; info++) {
		const struct monster_spell *spell = monster_spell_by_index(info->index);
		const struct effect *effect;

		/* Ignore missing spells */
		if (!spell) continue;
		if (!rsf_has(spells, info->index)) continue;

		/* Get the effect */
		effect = spell->effect;

		/* First we test the elemental spells */
		if (info->type & (RST_BOLT | RST_BALL | RST_BREATH)) {
			int element = effect->params[0];
			int learn_chance = el[element].res_level * (smart ? 50 : 25);
			if (randint0(100) < learn_chance)
				rsf_off(spells, info->index);
		} else {
			/* Now others with resisted effects */
			while (effect) {
				/* Timed effects */
				if ((smart || !one_in_(3)) && (effect->index == EF_TIMED_INC) &&
					of_has(flags, timed_protect_flag(effect->params[0])))
					break;

				/* Mana drain */
				if ((smart || one_in_(2)) && (effect->index == EF_DRAIN_MANA) &&
					pf_has(pflags, PF_NO_MANA))
					break;

				effect = effect->next;
			}
			if (effect)
				rsf_off(spells, info->index);
		}
	}
}

/**
 * Determine the damage of a spell attack which ignores monster hp
 * (i.e. bolts and balls, including arrows/boulders/storms/etc.)
 *
 * \param spell is the attack type
 * \param race is the monster race of the attacker
 * \param dam_aspect is the damage calc required (min, avg, max, random)
 */
static int nonhp_dam(const struct monster_spell *spell,
					 const struct monster_race *race, aspect dam_aspect)
{
	int dam = 0;
	struct effect *effect = spell->effect;

	/* Set the reference race for calculations */
	ref_race = race;

	/* Now add the damage for each effect */
	while (effect) {
		random_value rand;
		if (effect->dice) {
			dice_roll(effect->dice, &rand);
			dam += randcalc(rand, 0, dam_aspect);
		}
		effect = effect->next;
	}

	ref_race = NULL;

	return dam;
}

/**
 * Determine the damage of a monster breath attack
 *
 * \param element is the attack element
 * \param hp is the monster's hp
 */
int breath_dam(int element, int hp)
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
 * Calculate the damage of a monster spell.
 *
 * \param index is the index of the spell in question.
 * \param hp is the hp of the casting monster.
 * \param race is the race of the casting monster.
 * \param dam_aspect is the damage calc we want (min, max, avg, random).
 */
static int mon_spell_dam(int index, int hp, const struct monster_race *race,
						 aspect dam_aspect)
{
	const struct monster_spell *spell = monster_spell_by_index(index);

	if (monster_spell_is_breath(index))
		return breath_dam(spell->effect->params[0], hp);
	else
		return nonhp_dam(spell, race, dam_aspect);
}


/**
 * Calculate a monster's maximum spell power.
 *
 * \param race is the monster we're studying
 * \param resist is the degree of resistance we're assuming to any
 *   attack type (-1 = vulnerable ... 3 = immune)
 */
int best_spell_power(const struct monster_race *race, int resist)
{
	const struct mon_spell_info *info;
	int dam = 0, best_dam = 0; 

	/* Extract the monster level */
	int rlev = ((race->level >= 1) ? race->level : 1);

	for (info = mon_spell_types; info->index < RSF_MAX; info++) {
		if (rsf_has(race->spell_flags, info->index)) {
			/* Get the spell */
			const struct monster_spell *spell =
				monster_spell_by_index(info->index);
			if (!spell) continue;

			/* Get the maximum basic damage output of the spell (could be 0) */
			dam = mon_spell_dam(info->index, mon_hp(race, MAXIMISE), race,
				MAXIMISE);

			/* For all attack forms the player can save against, damage
			 * is halved */
			if (spell->save_message)
				dam /= 2;

			/* Adjust the real damage by the assumed resistance (if it is a
			 * resistable type) */
			if (monster_spell_is_projectable(info->index))
				dam = adjust_dam(player, spell->effect->params[0], dam,
								 MAXIMISE, 1);

			/* Add the power rating (crucial for non-damaging spells) */

			/* First we adjust the real damage if necessary */
			if (spell->power.dice)
				dam = (dam * spell->power.dice) / 100;

			/* Then we add any flat rating for this effect */
			dam += spell->power.base;

			/* Then we add any rlev-dependent rating */
			if (spell->power.m_bonus == 1)
				dam += (spell->power.sides * rlev) / 100;
			else if (spell->power.m_bonus == 2)
				dam += spell->power.sides / (rlev + 1);
		}

		/* Update the best_dam tracker */
		if (dam > best_dam)
			best_dam = dam;
	}

	return best_dam;
}

const char *mon_spell_lore_description(int index)
{
	const struct monster_spell *spell = monster_spell_by_index(index);
	if (!mon_spell_is_valid(index))
		return "";

	return spell->lore_desc;
}

int mon_spell_lore_damage(int index, const struct monster_race *race,
						  bool know_hp)
{
	int hp;

	if (!mon_spell_is_valid(index) || !mon_spell_has_damage(index))
		return 0;

	hp = (know_hp) ? race->avg_hp : 0;
	return mon_spell_dam(index, hp, race, MAXIMISE);
}

