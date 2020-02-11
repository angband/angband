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
#include "init.h"
#include "mon-attack.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "obj-knowledge.h"
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
	SPELL_TAG_PRONOUN,
	SPELL_TAG_TARGET,
	SPELL_TAG_TYPE,
	SPELL_TAG_OF_TYPE
} spell_tag_t;

static spell_tag_t spell_tag_lookup(const char *tag)
{
	if (strncmp(tag, "name", 4) == 0)
		return SPELL_TAG_NAME;
	else if (strncmp(tag, "pronoun", 7) == 0)
		return SPELL_TAG_PRONOUN;
	else if (strncmp(tag, "target", 6) == 0)
		return SPELL_TAG_TARGET;
	else if (strncmp(tag, "type", 4) == 0)
		return SPELL_TAG_TYPE;
	else if (strncmp(tag, "oftype", 6) == 0)
		return SPELL_TAG_OF_TYPE;
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
	const char *next;
	const char *s;
	const char *tag;
	const char *in_cursor;
	size_t end = 0;
	struct monster_spell_level *level = spell->level;
	struct monster *t_mon = NULL;

	/* Get the right level of message */
	while (level->next && mon->race->spell_power >= level->next->power) {
		level = level->next;
	}

	/* Get the target monster, if any */
	if (mon->target.midx > 0) {
		t_mon = cave_monster(cave, mon->target.midx);
	}

	/* Get the message */
	if (!seen) {
		if (t_mon) {
			return;
		} else {
			in_cursor = level->blind_message;
		}
	} else if (!hits) {
		in_cursor = level->miss_message;
	} else {
		in_cursor = level->message;
	}

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

			switch (spell_tag_lookup(tag)) {
				case SPELL_TAG_NAME: {
					char m_name[80];
					monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

					strnfcat(buf, sizeof(buf), &end, m_name);
					break;
				}

				case SPELL_TAG_PRONOUN: {
					char m_poss[80];

					/* Get the monster possessive ("his"/"her"/"its") */
					monster_desc(m_poss, sizeof(m_poss), mon, MDESC_PRO_VIS | MDESC_POSS);

					strnfcat(buf, sizeof(buf), &end, m_poss);
					break;
				}

				case SPELL_TAG_TARGET: {
					char m_name[80];
					struct monster *t_mon;
					if (mon->target.midx > 0) {
						t_mon = cave_monster(cave, mon->target.midx);
						monster_desc(m_name, sizeof(m_name), t_mon, MDESC_TARG);
						strnfcat(buf, sizeof(buf), &end, m_name);
					} else {
						strnfcat(buf, sizeof(buf), &end, "you");
					}
					break;
				}

				case SPELL_TAG_TYPE: {
					/* Get the attack type (assuming lash) */
					int type = mon->race->blow[0].effect->lash_type;
					char *type_name = projections[type].lash_desc;

					strnfcat(buf, sizeof(buf), &end, type_name);
					break;
				}

				case SPELL_TAG_OF_TYPE: {
					/* Get the attack type (assuming lash) */
					int type = mon->race->blow[0].effect->lash_type;
					char *type_name = projections[type].lash_desc;

					if (type_name) {
						strnfcat(buf, sizeof(buf), &end, " of ");
						strnfcat(buf, sizeof(buf), &end, type_name);
					}
					break;
				}

				default: {
					break;
				}
			}
		} else {
			/* An invalid tag, skip it */
			in_cursor = next + 1;
		}

		next = strchr(in_cursor, '{');
	}
	strnfcat(buf, 1024, &end, in_cursor);

	msgt(spell->msgt, "%s", buf);
}

const struct monster_spell *monster_spell_by_index(int index)
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
 * Check if a spell effect which has been saved against would also have
 * been prevented by an object property, and learn the appropriate rune
 */
static void spell_check_for_fail_rune(const struct monster_spell *spell)
{
	struct effect *effect = spell->effect;
	while (effect) {
		if (effect->index == EF_TELEPORT_LEVEL) {
			/* Special case - teleport level */
			equip_learn_element(player, ELEM_NEXUS);
		} else if (effect->index == EF_TIMED_INC) {
			/* Timed effects */
			(void) player_inc_check(player, effect->subtype, false);
		}
		effect = effect->next;
	}
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
	const struct monster_spell *spell = monster_spell_by_index(index);

	bool ident = false;
	bool hits;
	int target_mon = mon->target.midx;

	/* See if it hits */
	if (spell->hit == 100) {
		hits = true;
	} else if (spell->hit == 0) {
		hits = false;
	} else {
		int rlev = MAX(mon->race->level, 1);
		int conf_level = monster_effect_level(mon, MON_TMD_CONF);
		int accuracy = 100;
		while (conf_level) {
			accuracy *= (100 - CONF_HIT_REDUCTION);
			accuracy /= 100;
			conf_level--;
		}
		if (target_mon > 0) {
			hits = check_hit_monster(cave_monster(cave, target_mon),
									 spell->hit, rlev, accuracy);
		} else {
			hits = check_hit(player, spell->hit, rlev, accuracy);
		}
	}

	/* Tell the player what's going on */
	disturb(player, 1);
	spell_message(mon, spell, seen, hits);

	if (hits) {
		struct monster_spell_level *level = spell->level;

		/* Get the right level of save message */
		while (level->next && mon->race->spell_power >= level->next->power) {
			level = level->next;
		}

		/* Try a saving throw if available */
		if (level->save_message && (target_mon <= 0) &&
				randint0(100) < player->state.skills[SKILL_SAVE]) {
			msg("%s", level->save_message);
			spell_check_for_fail_rune(spell);
		} else {
			effect_do(spell->effect, source_monster(mon->midx), NULL, &ident, true, 0, 0, 0);
		}
	}
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


static bool mon_spell_is_valid(int index)
{
	return index > RSF_NONE && index < RSF_MAX;
}

static bool monster_spell_is_breath(int index)
{
	return (mon_spell_types[index].type & RST_BREATH) ? true : false;
}

static bool mon_spell_has_damage(int index)
{
	return (mon_spell_types[index].type & RST_DAMAGE) ? true : false;
}

bool mon_spell_is_innate(int index)
{
	return mon_spell_types[index].type & (RST_INNATE);
}

/**
 * Test a spell bitflag for a type of spell.
 * Returns true if any desired type is among the flagset
 *
 * \param f is the set of spell flags we're testing
 * \param types is the spell type(s) we're looking for
 */
bool test_spells(bitflag *f, int types)
{
	const struct mon_spell_info *info;

	for (info = mon_spell_types; info->index < RSF_MAX; info++)
		if (rsf_has(f, info->index) && (info->type & types))
			return true;

	return false;
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
}

/**
 * Turn off spells with a side effect or a proj_type that is resisted by
 * something in flags, subject to intelligence and chance.
 *
 * \param spells is the set of spells we're pruning
 * \param flags is the set of object flags we're testing
 * \param pflags is the set of player flags we're testing
 * \param el is what we know about the player's elemental resists
 * \param race is the monster type we're operating on
 */
void unset_spells(bitflag *spells, bitflag *flags, bitflag *pflags,
				  struct element_info *el, const struct monster *mon)
{
	const struct mon_spell_info *info;
	bool smart = monster_is_smart(mon);

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
			int element = effect->subtype;
			int learn_chance = el[element].res_level * (smart ? 50 : 25);
			if (randint0(100) < learn_chance) {
				rsf_off(spells, info->index);
			}
		} else {
			/* Now others with resisted effects */
			while (effect) {
				/* Timed effects */
				if ((smart || !one_in_(3)) &&
						effect->index == EF_TIMED_INC &&
						of_has(flags, timed_effects[effect->subtype].fail))
					break;

				/* Mana drain */
				if ((smart || one_in_(2)) &&
						effect->index == EF_DRAIN_MANA &&
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
		/* Lash needs special treatment bacause it depends on monster blows */
		if (effect->index == EF_LASH) {
			int i;

			/* Scan through all blows for damage */
			for (i = 0; i < z_info->mon_blows_max; i++) {
				/* Extract the attack infomation */
				random_value dice = race->blow[i].dice;

				/* Full damage of first blow, plus half damage of others */
				dam += randcalc(dice, race->level, dam_aspect) / (i ? 2 : 1);
			}
		} else if (effect->dice && (effect->index != EF_TIMED_INC)) {
			/* Timed effects increases don't count as damage in lore */
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
 * \param type is the attack element type
 * \param hp is the monster's hp
 */
int breath_dam(int type, int hp)
{
	struct projection *element = &projections[type];
	int dam;

	/* Damage is based on monster's current hp */
	dam = hp / element->divisor;

	/* Check for maximum damage */
	if (dam > element->damage_cap)
		dam = element->damage_cap;

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
		return breath_dam(spell->effect->subtype, hp);
	else
		return nonhp_dam(spell, race, dam_aspect);
}


/**
 * Create a mask of monster spell flags of a specific type.
 *
 * \param f is the flag array we're filling
 * \param ... is the list of flags we're looking for
 *
 * N.B. RST_NONE must be the last item in the ... list
 */
void create_mon_spell_mask(bitflag *f, ...)
{
	const struct mon_spell_info *rs;
	int i;
	va_list args;

	rsf_wipe(f);

	va_start(args, f);

	/* Process each type in the va_args */
    for (i = va_arg(args, int); i != RST_NONE; i = va_arg(args, int)) {
		for (rs = mon_spell_types; rs->index < RSF_MAX; rs++) {
			if (rs->type & i) {
				rsf_on(f, rs->index);
			}
		}
	}

	va_end(args);

	return;
}

const char *mon_spell_lore_description(int index,
									   const struct monster_race *race)
{
	if (mon_spell_is_valid(index)) {
		const struct monster_spell *spell = monster_spell_by_index(index);

		/* Get the right level of description */
		struct monster_spell_level *level = spell->level;
		while (level->next && race->spell_power >= level->next->power) {
			level = level->next;
		}
		return level->lore_desc;
	} else {
		return "";
	}
}

int mon_spell_lore_damage(int index, const struct monster_race *race,
		bool know_hp)
{
	if (mon_spell_is_valid(index) && mon_spell_has_damage(index)) {
		int hp = know_hp ? race->avg_hp : 0;
		return mon_spell_dam(index, hp, race, MAXIMISE);
	} else {
		return 0;
	}
}
