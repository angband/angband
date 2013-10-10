/*
 * File: mon-lore.c
 * Purpose: Monster recall code.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "monster/mon-lore.h"
#include "monster/mon-make.h"
#include "monster/mon-spell.h"
#include "monster/mon-util.h"
#include "object/tvalsval.h"
#include "attack.h"

/*
 * Monster genders
 */
enum monster_sex {
	MON_SEX_NEUTER = 0,
	MON_SEX_MALE,
	MON_SEX_FEMALE,
	MON_SEX_MAX,
};
typedef enum monster_sex monster_sex_t;

/**
 * Initializes the color-coding of monster attacks / spells.
 *
 * This function assigns a color to each monster melee attack type and each
 * monster spell, depending on how dangerous the attack is to the player
 * given current gear and state. Attacks may be colored green (least
 * dangerous), yellow, orange, or red (most dangerous). The colors are stored
 * in `melee_colors` and `spell_colors`, which the calling function then
 * uses when printing the monster recall.
 *
 * TODO: Is it possible to simplify this using the new monster spell refactor?
 * We should be able to loop over all spell effects and check for resistance
 * in a nicer way.
 */
static void get_attack_colors(int melee_colors[RBE_MAX], int spell_colors[RSF_MAX])
{
	int i;
	bool known;
	bitflag f[OF_SIZE];
	player_state st;
	int tmp_col;

	calc_bonuses(p_ptr->inventory, &st, TRUE);

	/* Initialize the colors to green */
	for (i = 0; i < RBE_MAX; i++)
		melee_colors[i] = TERM_L_GREEN;
	for (i = 0; i < RSF_MAX; i++)
		spell_colors[i] = TERM_L_GREEN;

	/* Scan the inventory for potentially vulnerable items */
	for (i = 0; i < INVEN_TOTAL; i++) {
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Only occupied slots */
		if (!o_ptr->kind) continue;

		object_flags_known(o_ptr, f);

		/* Don't reveal the nature of an object.
		 * Assume the player is conservative with unknown items.
		 */
		known = object_is_known(o_ptr);

		/* Drain charges - requires a charged item */
		if (i < INVEN_PACK && (!known || o_ptr->pval[DEFAULT_PVAL] > 0) &&
				(o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND))
			melee_colors[RBE_UN_POWER] = TERM_L_RED;

		/* Steal item - requires non-artifacts */
		if (i < INVEN_PACK && (!known || !o_ptr->artifact) &&
				p_ptr->lev + adj_dex_safe[st.stat_ind[A_DEX]] < 100)
			melee_colors[RBE_EAT_ITEM] = TERM_L_RED;

		/* Eat food - requries food */
		if (i < INVEN_PACK && o_ptr->tval == TV_FOOD)
			melee_colors[RBE_EAT_FOOD] = TERM_YELLOW;

		/* Eat light - requires a fuelled light */
		if (i == INVEN_LIGHT && !of_has(f, OF_NO_FUEL) &&
				o_ptr->timeout > 0)
			melee_colors[RBE_EAT_LIGHT] = TERM_YELLOW;

		/* Disenchantment - requires an enchanted item */
		if (i >= INVEN_WIELD && (!known || o_ptr->to_a > 0 ||
				o_ptr->to_h > 0 || o_ptr->to_d > 0) &&
				!check_state(p_ptr, OF_RES_DISEN, st.flags))
		{
			melee_colors[RBE_UN_BONUS] = TERM_L_RED;
			spell_colors[RSF_BR_DISE] = TERM_L_RED;
		}
	}

	/* Acid */
	if (check_state(p_ptr, OF_IM_ACID, st.flags))
		tmp_col = TERM_L_GREEN;
	else if (check_state(p_ptr, OF_RES_ACID, st.flags))
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	melee_colors[RBE_ACID] = tmp_col;
	spell_colors[RSF_BR_ACID] = tmp_col;
	spell_colors[RSF_BO_ACID] = tmp_col;
	spell_colors[RSF_BA_ACID] = tmp_col;

	/* Cold and ice */
	if (check_state(p_ptr, OF_IM_COLD, st.flags))
		tmp_col = TERM_L_GREEN;
	else if (check_state(p_ptr, OF_RES_COLD, st.flags))
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	melee_colors[RBE_COLD] = tmp_col;
	spell_colors[RSF_BR_COLD] = tmp_col;
	spell_colors[RSF_BO_COLD] = tmp_col;
	spell_colors[RSF_BA_COLD] = tmp_col;
	spell_colors[RSF_BO_ICEE] = tmp_col;

	/* Elec */
	if (check_state(p_ptr, OF_IM_ELEC, st.flags))
		tmp_col = TERM_L_GREEN;
	else if (check_state(p_ptr, OF_RES_ELEC, st.flags))
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	melee_colors[RBE_ELEC] = tmp_col;
	spell_colors[RSF_BR_ELEC] = tmp_col;
	spell_colors[RSF_BO_ELEC] = tmp_col;
	spell_colors[RSF_BA_ELEC] = tmp_col;

	/* Fire */
	if (check_state(p_ptr, OF_IM_FIRE, st.flags))
		tmp_col = TERM_L_GREEN;
	else if (check_state(p_ptr, OF_RES_FIRE, st.flags))
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	melee_colors[RBE_FIRE] = tmp_col;
	spell_colors[RSF_BR_FIRE] = tmp_col;
	spell_colors[RSF_BO_FIRE] = tmp_col;
	spell_colors[RSF_BA_FIRE] = tmp_col;

	/* Poison */
	if (!check_state(p_ptr, OF_RES_POIS, st.flags))
	{
		melee_colors[RBE_POISON] = TERM_ORANGE;
		spell_colors[RSF_BR_POIS] = TERM_ORANGE;
		spell_colors[RSF_BA_POIS] = TERM_ORANGE;
	}

	/* Nexus  */
	if (!check_state(p_ptr, OF_RES_NEXUS, st.flags))
	{
		if(st.skills[SKILL_SAVE] < 100)
			spell_colors[RSF_BR_NEXU] = TERM_L_RED;
		else
			spell_colors[RSF_BR_NEXU] = TERM_YELLOW;
	}

	/* Nether */
	if (!check_state(p_ptr, OF_RES_NETHR, st.flags))
	{
		spell_colors[RSF_BR_NETH] = TERM_ORANGE;
		spell_colors[RSF_BA_NETH] = TERM_ORANGE;
		spell_colors[RSF_BO_NETH] = TERM_ORANGE;
	}

	/* Inertia, gravity, and time */
	spell_colors[RSF_BR_INER] = TERM_ORANGE;
	spell_colors[RSF_BR_GRAV] = TERM_L_RED;
	spell_colors[RSF_BR_TIME] = TERM_L_RED;

	/* Sound */
	if (check_state(p_ptr, OF_RES_SOUND, st.flags))
		spell_colors[RSF_BR_SOUN] = TERM_L_GREEN;
	else if (!check_state(p_ptr, OF_RES_SOUND, st.flags) && check_state(p_ptr, OF_RES_STUN, st.flags))
		spell_colors[RSF_BR_SOUN] = TERM_YELLOW;
	else
		spell_colors[RSF_BR_SOUN] = TERM_ORANGE;

 	/* Shards */
 	if(!check_state(p_ptr, OF_RES_SHARD, st.flags))
 		spell_colors[RSF_BR_SHAR] = TERM_ORANGE;

	/* Confusion */
	if (!check_state(p_ptr, OF_RES_CONFU, st.flags))
	{
		melee_colors[RBE_CONFUSE] = TERM_ORANGE;
		spell_colors[RSF_BR_CONF] = TERM_ORANGE;
	}

	/* Stunning */
	if (!check_state(p_ptr, OF_RES_STUN, st.flags)) {
		spell_colors[RSF_BR_WALL] = TERM_YELLOW;
		spell_colors[RSF_BR_PLAS] = TERM_ORANGE;
		spell_colors[RSF_BO_PLAS] = TERM_ORANGE;
		spell_colors[RSF_BO_ICEE] = TERM_ORANGE;
	}
	else {
		spell_colors[RSF_BR_PLAS] = TERM_YELLOW;
		spell_colors[RSF_BO_PLAS] = TERM_YELLOW;
		spell_colors[RSF_BO_ICEE] = TERM_YELLOW;
	}

	/* Chaos */
	if (!check_state(p_ptr, OF_RES_CHAOS, st.flags))
		spell_colors[RSF_BR_CHAO] = TERM_ORANGE;

	/* Light */
	if (!check_state(p_ptr, OF_RES_LIGHT, st.flags))
		spell_colors[RSF_BR_LIGHT] = TERM_ORANGE;

	/* Darkness */
	if (!check_state(p_ptr, OF_RES_DARK, st.flags))
	{
		spell_colors[RSF_BR_DARK] = TERM_ORANGE;
		spell_colors[RSF_BA_DARK] = TERM_L_RED;
	}

	/* Water */
	if (!check_state(p_ptr, OF_RES_CONFU, st.flags) ||
			!check_state(p_ptr, OF_RES_STUN, st.flags))
	{
		spell_colors[RSF_BA_WATE] = TERM_L_RED;
		spell_colors[RSF_BO_WATE] = TERM_L_RED;
	}
	else
	{
		spell_colors[RSF_BA_WATE] = TERM_ORANGE;
		spell_colors[RSF_BO_WATE] = TERM_ORANGE;
	}

	/* Mana */
	spell_colors[RSF_BR_MANA] = TERM_L_RED;
	spell_colors[RSF_BA_MANA] = TERM_L_RED;
	spell_colors[RSF_BO_MANA] = TERM_L_RED;

	/* These attacks only apply without a perfect save */
	if (st.skills[SKILL_SAVE] < 100)
	{
		/* Amnesia */
		spell_colors[RSF_FORGET] = TERM_YELLOW;

		/* Fear */
		if (!check_state(p_ptr, OF_RES_FEAR, st.flags))
		{
			melee_colors[RBE_TERRIFY] = TERM_YELLOW;
			spell_colors[RSF_SCARE] = TERM_YELLOW;
		}

		/* Paralysis and slow */
		if (!check_state(p_ptr, OF_FREE_ACT, st.flags))
		{
			melee_colors[RBE_PARALYZE] = TERM_L_RED;
			spell_colors[RSF_HOLD] = TERM_L_RED;
			spell_colors[RSF_SLOW] = TERM_ORANGE;
		}

		/* Blind */
		if (!check_state(p_ptr, OF_RES_BLIND, st.flags))
			spell_colors[RSF_BLIND] = TERM_ORANGE;

		/* Confusion */
		if (!check_state(p_ptr, OF_RES_CONFU, st.flags))
			spell_colors[RSF_CONF] = TERM_ORANGE;

		/* Cause wounds */
		spell_colors[RSF_CAUSE_1] = TERM_YELLOW;
		spell_colors[RSF_CAUSE_2] = TERM_YELLOW;
		spell_colors[RSF_CAUSE_3] = TERM_YELLOW;
		spell_colors[RSF_CAUSE_4] = TERM_YELLOW;

		/* Mind blast */
		spell_colors[RSF_MIND_BLAST] = (check_state(p_ptr, OF_RES_CONFU, st.flags) ?
				TERM_YELLOW : TERM_ORANGE);

		/* Brain smash slows even when conf/blind resisted */
		spell_colors[RSF_BRAIN_SMASH] = (check_state(p_ptr, OF_RES_BLIND, st.flags) &&
				check_state(p_ptr, OF_FREE_ACT, st.flags) &&
				check_state(p_ptr, OF_RES_CONFU, st.flags)	? TERM_ORANGE : TERM_L_RED);
	}

	/* Gold theft */
	if (p_ptr->lev + adj_dex_safe[st.stat_ind[A_DEX]] < 100 && p_ptr->au)
		melee_colors[RBE_EAT_GOLD] = TERM_YELLOW;

	/* Melee blindness and hallucinations */
	if (!check_state(p_ptr, OF_RES_BLIND, st.flags))
		melee_colors[RBE_BLIND] = TERM_YELLOW;
	if (!check_state(p_ptr, OF_RES_CHAOS, st.flags))
		melee_colors[RBE_HALLU] = TERM_YELLOW;

	/* Stat draining is bad */
	if (!check_state(p_ptr, OF_SUST_STR, st.flags))
		melee_colors[RBE_LOSE_STR] = TERM_ORANGE;
	if (!check_state(p_ptr, OF_SUST_INT, st.flags))
		melee_colors[RBE_LOSE_INT] = TERM_ORANGE;
	if (!check_state(p_ptr, OF_SUST_WIS, st.flags))
		melee_colors[RBE_LOSE_WIS] = TERM_ORANGE;
	if (!check_state(p_ptr, OF_SUST_DEX, st.flags))
		melee_colors[RBE_LOSE_DEX] = TERM_ORANGE;
	if (!check_state(p_ptr, OF_SUST_CON, st.flags))
		melee_colors[RBE_LOSE_CON] = TERM_ORANGE;

	/* Drain all gets a red warning */
	if (!check_state(p_ptr, OF_SUST_STR, st.flags) || !check_state(p_ptr, OF_SUST_INT, st.flags) ||
			!check_state(p_ptr, OF_SUST_WIS, st.flags) || !check_state(p_ptr, OF_SUST_DEX, st.flags) ||
			!check_state(p_ptr, OF_SUST_CON, st.flags))
		melee_colors[RBE_LOSE_ALL] = TERM_L_RED;

	/* Hold life isn't 100% effective */
	melee_colors[RBE_EXP_10] = melee_colors[RBE_EXP_20] = 
			melee_colors[RBE_EXP_40] = melee_colors[RBE_EXP_80] =
			check_state(p_ptr, OF_HOLD_LIFE, st.flags) ? TERM_YELLOW : TERM_ORANGE;

	/* Shatter is always noteworthy */
	melee_colors[RBE_SHATTER] = TERM_YELLOW;

	/* Heal (and drain mana) and haste are always noteworthy */
	spell_colors[RSF_HEAL] = TERM_YELLOW;
	spell_colors[RSF_DRAIN_MANA] = TERM_YELLOW;
	spell_colors[RSF_HASTE] = TERM_YELLOW;

	/* Player teleports and traps are annoying */
	spell_colors[RSF_TELE_TO] = TERM_YELLOW;
	spell_colors[RSF_TELE_AWAY] = TERM_YELLOW;
	if (!check_state(p_ptr, OF_RES_NEXUS, st.flags) && st.skills[SKILL_SAVE] < 100)
		spell_colors[RSF_TELE_LEVEL] = TERM_YELLOW;
	spell_colors[RSF_TRAPS] = TERM_YELLOW;

	/* Summons are potentially dangerous */
	spell_colors[RSF_S_MONSTER] = TERM_ORANGE;
	spell_colors[RSF_S_MONSTERS] = TERM_ORANGE;
	spell_colors[RSF_S_KIN] = TERM_ORANGE;
	spell_colors[RSF_S_ANIMAL] = TERM_ORANGE;
	spell_colors[RSF_S_SPIDER] = TERM_ORANGE;
	spell_colors[RSF_S_HOUND] = TERM_ORANGE;
	spell_colors[RSF_S_HYDRA] = TERM_ORANGE;
	spell_colors[RSF_S_AINU] = TERM_ORANGE;
	spell_colors[RSF_S_DEMON] = TERM_ORANGE;
	spell_colors[RSF_S_DRAGON] = TERM_ORANGE;
	spell_colors[RSF_S_UNDEAD] = TERM_ORANGE;

	/* High level summons are very dangerous */
	spell_colors[RSF_S_HI_DEMON] = TERM_L_RED;
	spell_colors[RSF_S_HI_DRAGON] = TERM_L_RED;
	spell_colors[RSF_S_HI_UNDEAD] = TERM_L_RED;
	spell_colors[RSF_S_UNIQUE] = TERM_L_RED;
	spell_colors[RSF_S_WRAITH] = TERM_L_RED;

	/* Shrieking can lead to bad combos */
	spell_colors[RSF_SHRIEK] = TERM_ORANGE;

	/* Ranged attacks can't be resisted (only mitigated by accuracy)
	 * They are colored yellow to indicate the damage is a hard value
	 */
	spell_colors[RSF_ARROW_1] = TERM_YELLOW;
	spell_colors[RSF_ARROW_2] = TERM_YELLOW;
	spell_colors[RSF_ARROW_3] = TERM_YELLOW;
	spell_colors[RSF_ARROW_4] = TERM_YELLOW;
	spell_colors[RSF_BOULDER] = TERM_YELLOW;
}

/**
 * Determine if the player knows the AC of the given monster.
 */
static bool know_armour(const monster_race *r_ptr, const monster_lore *l_ptr)
{
	assert(l_ptr);
	return l_ptr->tkills > 0;
}

/**
 * Learn everything about a monster (by cheating).
 *
 * Sets the number of total kills of a monster to MAX_SHORT, so that the
 * player knows the armor etc. of the monster. Sets the number of observed
 * blows to MAX_UCHAR for each blow. Sets the number of observed drops
 * to the maximum possible. The player also automatically learns every
 * monster flag.
 * 
 */
void cheat_monster_lore(const monster_race *r_ptr, monster_lore *l_ptr)
{
	int i;

	assert(r_ptr);
	assert(l_ptr);
	
	/* Hack -- Maximal kills */
	l_ptr->sights = MAX_SHORT;
	l_ptr->tkills = MAX_SHORT;

	/* Hack -- Maximal info */
	l_ptr->wake = l_ptr->ignore = MAX_UCHAR;

	/* Observe "maximal" attacks */
	for (i = 0; i < MONSTER_BLOW_MAX; i++) {
		/* Examine "actual" blows */
		if (r_ptr->blow[i].effect || r_ptr->blow[i].method) {
			/* Hack -- maximal observations */
			l_ptr->blows[i] = MAX_UCHAR;
		}
	}

	/* Hack -- maximal drops */
	l_ptr->drop_item = 0;
	
	if (rf_has(r_ptr->flags, RF_DROP_4))
		l_ptr->drop_item += 6;
	if (rf_has(r_ptr->flags, RF_DROP_3))
		l_ptr->drop_item += 4;
	if (rf_has(r_ptr->flags, RF_DROP_2))
		l_ptr->drop_item += 3;
	if (rf_has(r_ptr->flags, RF_DROP_1))
		l_ptr->drop_item++;

	if (rf_has(r_ptr->flags, RF_DROP_40))
		l_ptr->drop_item++;
	if (rf_has(r_ptr->flags, RF_DROP_60))
		l_ptr->drop_item++;
	if (rf_has(r_ptr->flags, RF_DROP_20))
		l_ptr->drop_item++;

	l_ptr->drop_gold = l_ptr->drop_item;

	/* Hack -- but only "valid" drops */
	if (rf_has(r_ptr->flags, RF_ONLY_GOLD)) l_ptr->drop_item = 0;
	if (rf_has(r_ptr->flags, RF_ONLY_ITEM)) l_ptr->drop_gold = 0;

	/* Hack -- observe many spells */
	l_ptr->cast_innate = MAX_UCHAR;
	l_ptr->cast_spell = MAX_UCHAR;

	/* Hack -- know all the flags */
	rf_setall(l_ptr->flags);
	rsf_copy(l_ptr->spell_flags, r_ptr->spell_flags);
}

/**
 * Forget everything about a monster.
 *
 * Sets the number of total kills, observed blows, and observed drops to 0.
 * Also wipes all knowledge of monster flags.
 */
void wipe_monster_lore(const monster_race *r_ptr, monster_lore *l_ptr)
{
	int i;

	assert(r_ptr);
	assert(l_ptr);
	
	/* Hack -- No kills */
	l_ptr->tkills = 0;

	/* Hack -- No info */
	l_ptr->wake = l_ptr->ignore = 0;

	/* Observe "maximal" attacks */
	for (i = 0; i < MONSTER_BLOW_MAX; i++) {
		/* Examine "actual" blows */
		if (r_ptr->blow[i].effect || r_ptr->blow[i].method) {
			/* Hack -- no observations */
			l_ptr->blows[i] = 0;
		}
	}

	/* Hack -- no drops */
	l_ptr->drop_item = l_ptr->drop_gold = 0;
	
	/* Hack -- forget all spells */
	l_ptr->cast_innate = 0;
	l_ptr->cast_spell = 0;

	/* Hack -- wipe all the flags */
	rf_wipe(l_ptr->flags);
	rsf_wipe(l_ptr->spell_flags);
}

/**
 * Learn about a monster (by "probing" it)
 */
void lore_do_probe(struct monster *m)
{
	monster_lore *l_ptr = get_lore(m->race);
	unsigned i;

	/* Know various things */
	rf_setall(l_ptr->flags);
	rsf_copy(l_ptr->spell_flags, m->race->spell_flags);
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
		l_ptr->blows[i] = MAX_UCHAR;

	/* Update monster recall window */
	if (p_ptr->monster_race == m->race)
		p_ptr->redraw |= (PR_MONSTER);
}

/**
 * Take note that the given monster just dropped some treasure
 *
 * Note that learning the "GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 *
 * This "indirect" method is used to prevent the player from learning
 * exactly how much treasure a monster can drop from observing only
 * a single example of a drop.  This method actually observes how much
 * gold and items are dropped, and remembers that information to be
 * described later by the monster recall code.
 */
void lore_treasure(struct monster *m_ptr, int num_item, int num_gold)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	assert(num_item >= 0);
	assert(num_gold >= 0);

	/* Note the number of things dropped */
	if (num_item > l_ptr->drop_item)
		l_ptr->drop_item = num_item;
	if (num_gold > l_ptr->drop_gold)
		l_ptr->drop_gold = num_gold;

	/* Learn about drop quality */
	rf_on(l_ptr->flags, RF_DROP_GOOD);
	rf_on(l_ptr->flags, RF_DROP_GREAT);

	/* Update monster recall window */
	if (p_ptr->monster_race == m_ptr->race)
		p_ptr->redraw |= (PR_MONSTER);
}

/**
 * Copies into `flags` the flags of the given monster race that are known
 * to the given lore structure (usually the player's knowledge).
 *
 * Known flags will be 1 for present, or 0 for not present. Unknown flags
 * will always be 0.
 */
void monster_flags_known(const monster_race *r_ptr, const monster_lore *l_ptr,
		bitflag flags[RF_SIZE])
{
	rf_copy(flags, r_ptr->flags);
	rf_inter(flags, l_ptr->flags);
}

/**
 * Return a description for the given monster race flag.
 *
 * Returns an empty string for an out-of-range flag. Descriptions are in list-mon-flag.h.
 *
 * \param flag is one of the RF_ flags.
 */
static const char *lore_describe_race_flag(int flag)
{
	static const char *r_flag_description[] = {
		#define RF(a, b) b,
		#include "monster/list-mon-flags.h"
		#undef RF
		NULL
	};

	if (flag <= RF_NONE || flag >= RF_MAX)
		return "";

	return r_flag_description[flag];
}

/**
 * Return a description for the given monster blow method flags.
 *
 * Returns an sensible placeholder string for an out-of-range flag. Descriptions are in list-blow-methods.h.
 *
 * \param method is one of the RBM_ flags.
 */
static const char *lore_describe_blow_method(int method)
{
	static const char *r_blow_method_description[] = {
		#define RBM(a, b) b,
		#include "monster/list-blow-methods.h"
		#undef RBM
		NULL
	};

	/* Return a placeholder for RBM_NONE, since it doesn't make sense to describe a blow that doesn't have a method */
	if (method <= RBM_NONE || method >= RBM_MAX)
		return "do something weird";

	return r_blow_method_description[method];
}

/**
 * Return a description for the given monster blow effect flags.
 *
 * Returns an sensible placeholder string for an out-of-range flag. Descriptions are in list-blow-effects.h.
 *
 * \param effect is one of the RBE_ flags.
 */
static const char *lore_describe_blow_effect(int effect)
{
	static const char *r_blow_effect_description[] = {
		#define RBE(a, b) b,
		#include "monster/list-blow-effects.h"
		#undef RBE
		NULL
	};

	/* Some blows have no effects, so we do want to return whatever is in the table for RBE_NONE */
	if (effect < RBE_NONE || effect >= RBE_MAX)
		return "do weird things";

	return r_blow_effect_description[effect];
}

/**
 * Return a description for the given monster race awareness value.
 *
 * Descriptions are in a table within the function. Returns a sensible string for values not in the table.
 *
 * \param awareness is the inactivity counter of the race (monster_race.sleep).
 */
static const char *lore_describe_awareness(s16b awareness)
{
	/* Value table ordered descending, for priority. Terminator is {MAX_SHORT, NULL}. */
	static const struct lore_awareness {
		s16b threshold;
		const char *description;
	} lore_awareness_description[] = {
		{200,	"prefers to ignore"},
		{95,	"pays very little attention to"},
		{75,	"pays little attention to"},
		{45,	"tends to overlook"},
		{25,	"takes quite a while to see"},
		{10,	"takes a while to see"},
		{5,		"is fairly observant of"},
		{3,		"is observant of"},
		{1,		"is very observant of"},
		{0,		"is vigilant for"},
		{MAX_SHORT,	NULL},
	};
	const struct lore_awareness *current = lore_awareness_description;

	while (current->threshold != MAX_SHORT && current->description != NULL) {
		if (awareness > current->threshold)
			return current->description;

		current++;
	}

	/* Values zero and less are the most vigilant */
	return "is ever vigilant for";
}

/**
 * Return a description for the given monster race speed value.
 *
 * Descriptions are in a table within the function. Returns a sensible string for values not in the table.
 *
 * \param speed is the speed rating of the race (monster_race.speed).
 */
static const char *lore_describe_speed(byte speed)
{
	/* Value table ordered descending, for priority. Terminator is {MAX_UCHAR, NULL}. */
	static const struct lore_speed {
		byte threshold;
		const char *description;
	} lore_speed_description[] = {
		{130,	"incredibly quickly"},
		{120,	"very quickly"},
		{110,	"quickly"},
		{109,	"normal speed"}, /* 110 is normal speed */
		{99,	"slowly"},
		{89,	"very slowly"},
		{0,		"incredibly slowly"},
		{MAX_UCHAR,	NULL},
	};
	const struct lore_speed *current = lore_speed_description;

	while (current->threshold != MAX_UCHAR && current->description != NULL) {
		if (speed > current->threshold)
			return current->description;

		current++;
	}

	/* Return a weird description, since the value wasn't found in the table */
	return "erroneously";
}

/**
 * Return a value describing the sex of the provided monster race.
 */
static monster_sex_t lore_monster_sex(const monster_race *race)
{
	if (rf_has(race->flags, RF_FEMALE))
		return MON_SEX_FEMALE;
	else if (rf_has(race->flags, RF_MALE))
		return MON_SEX_MALE;

	return MON_SEX_NEUTER;
}

/**
 * Return a pronoun for a monster; used as the subject of a sentence.
 *
 * Descriptions are in a table within the function. Table must match monster_sex_t values.
 *
 * \param sex is the gender value (as provided by `lore_monster_sex()`.
 * \param title_case indicates whether the initial letter should be capitalized; `TRUE` is capitalized, `FALSE` is not.
 */
static const char *lore_pronoun_nominative(monster_sex_t sex, bool title_case)
{
	static const char *lore_pronouns[MON_SEX_MAX][2] = {
		{"it", "It"},
		{"he", "He"},
		{"she", "She"},
	};

	int pronoun_index = MON_SEX_NEUTER, case_index = 0;

	if (sex < MON_SEX_MAX)
		pronoun_index = sex;

	if (title_case)
		case_index = 1;

	return lore_pronouns[pronoun_index][case_index];
}

/**
 * Return a possessive pronoun for a monster.
 *
 * Descriptions are in a table within the function. Table must match monster_sex_t values.
 *
 * \param sex is the gender value (as provided by `lore_monster_sex()`.
 * \param title_case indicates whether the initial letter should be capitalized; `TRUE` is capitalized, `FALSE` is not.
 */
static const char *lore_pronoun_possessive(monster_sex_t sex, bool title_case)
{
	static const char *lore_pronouns[MON_SEX_MAX][2] = {
		{"its", "Its"},
		{"his", "His"},
		{"her", "Her"},
	};

	int pronoun_index = MON_SEX_NEUTER, case_index = 0;

	if (sex < MON_SEX_MAX)
		pronoun_index = sex;

	if (title_case)
		case_index = 1;

	return lore_pronouns[pronoun_index][case_index];
}

/**
 * Insert into a list the description for a given flag, if it is set. Return the next index available for insertion.
 *
 * The function returns an incremented index if it inserted something; otherwise, it returns the same index (which is used for the next insertion attempt).
 *
 * \param flag is the RF_ flag to check for in `known_flags`.
 * \param known_flags is the preprocessed set of flags for the lore/race.
 * \param list is the list in which the description will be inserted.
 * \param index is where in `list` the description will be inserted.
 */
static int lore_insert_flag_description(int flag, const bitflag known_flags[RF_SIZE], const char *list[], int index)
{
	if (rf_has(known_flags, flag)) {
		list[index] = lore_describe_race_flag(flag);
		return index + 1;
	}

	return index;
}

/**
 * Insert into a list the description for a given flag, if a flag is not known to the player as a vulnerability. Return the next index available for insertion.
 *
 * The function returns an incremented index if it inserted something; otherwise, it returns the same index (which is used for the next insertion attempt).
 *
 * \param flag is the RF_ flag to check for in `known_flags`.
 * \param known_flags is the preprocessed set of flags for the lore/race.
 * \param lore is the base knowledge about the monster.
 * \param list is the list in which the description will be inserted.
 * \param index is where in `list` the description will be inserted.
 */
static int lore_insert_unknown_vulnerability(int flag, const bitflag known_flags[RF_SIZE], const monster_lore *lore, const char *list[], int index)
{
	if (rf_has(lore->flags, flag) && !rf_has(known_flags, flag)) {
		list[index] = lore_describe_race_flag(flag);
		return index + 1;
	}

	return index;
}

/**
 * Insert into a list the description for a spell if it is known to the player. Return the next index available for insertion.
 *
 * The function returns an incremented index if it inserted something; otherwise, it returns the same index (which is used for the next insertion attempt).
 *
 * \param spell is the RSF_ flag to describe.
 * \param race is the monster race of the spell.
 * \param lore is the player's current knowledge about the monster.
 * \param spell_colors is where the color for `spell` will be chosen from.
 * \param know_hp indicates whether or know the player has determined the monster's AC/HP.
 * \param name_list is the list in which the description will be inserted.
 * \param color_list is the list in which the selected color will be inserted.
 * \param damage_list is the list in which the max spell damage will be inserted.
 * \param index is where in `name_list`, `color_list`, and `damage_list` the description will be inserted.
 */
static int lore_insert_spell_description(int spell, const monster_race *race, const monster_lore *lore, const int spell_colors[RSF_MAX], bool know_hp, const char *name_list[], int color_list[], int damage_list[], int index)
{
	if (rsf_has(lore->spell_flags, spell)) {
		name_list[index] = mon_spell_lore_description(spell);
		color_list[index] = spell_colors[spell];
		damage_list[index] = mon_spell_lore_damage(spell, race, know_hp);
		return index + 1;
	}

	return index;
}

/**
 * Append a list of items to a textblock, with each item using the provided attribute.
 *
 * The text that joins the list is drawn using the default attributes. The list uses a serial comma ("a, b, c, and d").
 *
 * \param tb is the textblock we are adding to.
 * \param list is a list of strings that should be joined and appended; drawn with the attribute in `attribute`.
 * \param count is the number of items in `list`.
 * \param attr is the attribute each list item will be drawn with.
 * \param conjunction is a string that is added before the last item.
 */
static void lore_append_list(textblock *tb, const char *list[], int count, byte attr, const char *conjunction)
{
	int i;

	assert(count >= 0);

	for (i = 0; i < count; i++) {
		if (i > 0) {
			if (count > 2)
				textblock_append(tb, ",");

			if (i == count - 1) {
				textblock_append(tb, " ");
				textblock_append(tb, conjunction);
			}

			textblock_append(tb, " ");
		}

		textblock_append_c(tb, attr, list[i]);
	}
}

/**
 * Append a list of spell descriptions.
 *
 * This is a modified version of `lore_append_list()` to format spells, without have to do a lot of allocating and freeing of formatted strings.
 *
 * \param tb is the textblock we are adding to.
 * \param name_list is a list of base spell description.
 * \param color_list is the list of attributes which the description should be drawn with.
 * \param damage_list is a value that should be appended to the base spell description (if it is greater than zero).
 * \param count is the number of items in the lists.
 * \param conjunction is a string that is added before the last item.
 */
static void lore_append_spell_descriptions(textblock *tb, const char *name_list[], int color_list[], int damage_list[], int count, const char *conjunction)
{
	int i;

	assert(count >= 0);

	for (i = 0; i < count; i++) {
		if (i > 0) {
			if (count > 2)
				textblock_append(tb, ",");

			if (i == count - 1) {
				textblock_append(tb, " ");
				textblock_append(tb, conjunction);
			}

			textblock_append(tb, " ");
		}

		textblock_append_c(tb, color_list[i], name_list[i]);

		if (damage_list[i] > 0)
			textblock_append_c(tb, color_list[i], " (%d)", damage_list[i]);
	}
}

/**
 * Append the kill history to a texblock for a given monster race.
 *
 * Known race flags are passed in for simplicity/efficiency.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_kills(textblock *tb, const monster_race *race, const monster_lore *lore, const bitflag known_flags[RF_SIZE])
{
	monster_sex_t msex = MON_SEX_NEUTER;
	bool out = TRUE;

	assert(tb && race && lore);

	/* Extract a gender (if applicable) */
	msex = lore_monster_sex(race);

	/* Treat uniques differently */
	if (rf_has(known_flags, RF_UNIQUE)) {
		/* Hack -- Determine if the unique is "dead" */
		bool dead = (race->max_num == 0) ? TRUE : FALSE;

		/* We've been killed... */
		if (lore->deaths) {
			/* Killed ancestors */
			textblock_append(tb, "%s has slain %d of your ancestors", lore_pronoun_nominative(msex, TRUE), lore->deaths);

			/* But we've also killed it */
			if (dead)
				textblock_append(tb, ", but you have taken revenge!  ");

			/* Unavenged (ever) */
			else
				textblock_append(tb, ", who %s unavenged.  ", VERB_AGREEMENT(lore->deaths, "remains", "remain"));
		}
		else if (dead) {
			/* Dead unique who never hurt us */
			textblock_append(tb, "You have slain this foe.  ");
		}
		else {
			/* Alive and never killed us */
			out = FALSE;
		}
	}

	/* Not unique, but killed us */
	else if (lore->deaths) {
		/* Dead ancestors */
		textblock_append(tb, "%d of your ancestors %s been killed by this creature, ", lore->deaths, VERB_AGREEMENT(lore->deaths, "has", "have"));

		/* Some kills this life */
		if (lore->pkills) {
			textblock_append(tb, "and you have exterminated at least %d of the creatures.  ", lore->pkills);
		}

		/* Some kills past lives */
		else if (lore->tkills) {
			textblock_append(tb, "and your ancestors have exterminated at least %d of the creatures.  ", lore->tkills);
		}

		/* No kills */
		else {
			textblock_append_c(tb, TERM_RED, "and %s is not ever known to have been defeated.  ", lore_pronoun_nominative(msex, FALSE));
		}
	}

	/* Normal monsters */
	else {
		/* Killed some this life */
		if (lore->pkills) {
			textblock_append(tb, "You have killed at least %d of these creatures.  ", lore->pkills);
		}

		/* Killed some last life */
		else if (lore->tkills) {
			textblock_append(tb, "Your ancestors have killed at least %d of these creatures.  ", lore->tkills);
		}

		/* Killed none */
		else {
			textblock_append(tb, "No battles to the death are recalled.  ");
		}
	}

	/* Separate */
	if (out)
		textblock_append(tb, "\n");
}

/**
 * Append the monster race description to a textblock.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param append_utf8 indicates if we should append the flavor text as UTF-8 (which is preferred for spoiler files).
 */
static void lore_append_flavor(textblock *tb, const monster_race *race, bool append_utf8)
{
	assert(tb && race);

	if (append_utf8)
		textblock_append_utf8(tb, race->text);
	else
		textblock_append(tb, race->text);

	textblock_append(tb, "\n");
}

/**
 * Append the monster type, location, and movement patterns to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_movement(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	assert(tb && race && lore);

	textblock_append(tb, "This");

	if (rf_has(race->flags, RF_ANIMAL))		textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_ANIMAL));
	if (rf_has(race->flags, RF_EVIL))		textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_EVIL));
	if (rf_has(race->flags, RF_UNDEAD))		textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_UNDEAD));
	if (rf_has(race->flags, RF_NONLIVING))	textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_NONLIVING));
	if (rf_has(race->flags, RF_METAL))		textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_METAL));

	if (rf_has(race->flags, RF_DRAGON))		textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_DRAGON));
	else if (rf_has(race->flags, RF_DEMON))	textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_DEMON));
	else if (rf_has(race->flags, RF_GIANT))	textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_GIANT));
	else if (rf_has(race->flags, RF_TROLL))	textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_TROLL));
	else if (rf_has(race->flags, RF_ORC))	textblock_append_c(tb, TERM_L_BLUE, " %s", lore_describe_race_flag(RF_ORC));
	else									textblock_append_c(tb, TERM_L_BLUE, " creature");

	/* Describe location */
	if (race->level == 0) {
		textblock_append(tb, " lives in the town");
	}
	else {
		byte colour = (race->level > p_ptr->max_depth) ? TERM_RED : TERM_L_BLUE;

		if (rf_has(known_flags, RF_FORCE_DEPTH))
			textblock_append(tb, " is found ");
		else
			textblock_append(tb, " is normally found ");

		textblock_append(tb, "at depths of ");
		textblock_append_c(tb, colour, "%d", race->level * 50);
		textblock_append(tb, " feet (level ");
		textblock_append_c(tb, colour, "%d", race->level);
		textblock_append(tb, ")");
	}

	textblock_append(tb, ", and moves");

	/* Random-ness */
	if (flags_test(known_flags, RF_SIZE, RF_RAND_50, RF_RAND_25, FLAG_END)) {
		/* Adverb */
		if (rf_has(known_flags, RF_RAND_50) && rf_has(known_flags, RF_RAND_25))
			textblock_append(tb, " extremely");
		else if (rf_has(known_flags, RF_RAND_50))
			textblock_append(tb, " somewhat");
		else if (rf_has(known_flags, RF_RAND_25))
			textblock_append(tb, " a bit");

		/* Adjective */
		textblock_append(tb, " erratically");

		/* Hack -- Occasional conjunction */
		if (race->speed != 110) textblock_append(tb, ", and");
	}

	/* Speed */
	textblock_append(tb, " ");

	/* "at" is separate from the normal speed description in order to use the normal text colour */
	if (race->speed == 110)
		textblock_append(tb, "at ");

	textblock_append_c(tb, TERM_GREEN, lore_describe_speed(race->speed));

	/* The speed description also describes "attack speed" */
	if (rf_has(known_flags, RF_NEVER_MOVE)) {
		textblock_append(tb, ", but ");
		textblock_append_c(tb, TERM_L_GREEN, "does not deign to chase intruders");
	}

	/* End this sentence */
	textblock_append(tb, ".  ");
}

/**
 * Append the monster AC, HP, and hit chance to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_toughness(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	monster_sex_t msex = MON_SEX_NEUTER;
	long chance = 0, chance2 = 0;
	object_type *weapon = &p_ptr->inventory[INVEN_WIELD];

	assert(tb && race && lore);

	/* Extract a gender (if applicable) */
	msex = lore_monster_sex(race);

	/* Describe monster "toughness" */
	if (know_armour(race, lore)) {
		/* Armor */
		textblock_append(tb, "%s has an armor rating of ", lore_pronoun_nominative(msex, TRUE));
		textblock_append_c(tb, TERM_L_BLUE, "%d", race->ac);

		/* Hitpoints */
		textblock_append(tb, ", and a");

		if (!rf_has(known_flags, RF_UNIQUE))
			textblock_append(tb, "n average");

		textblock_append(tb, " life rating of ");
		textblock_append_c(tb, TERM_L_BLUE, "%d", race->avg_hp);
		textblock_append(tb, ".  ");

		/* Player's chance to hit it */
		chance = py_attack_hit_chance(weapon);

		/* The following calculations are based on test_hit(); make sure to keep it in sync */
		/* Avoid division by zero errors, and starting higher on the scale */
		if (chance < 9)
			chance = 9;

		chance2 = 90 * (chance - (race->ac * 2 / 3)) / chance + 5;

		/* There is always a 12 percent chance to hit */
		if (chance2 < 12) chance2 = 12;

		textblock_append(tb, "You have a");
		if ((chance2 == 8) || ((chance2 / 10) == 8))
			textblock_append(tb, "n");
		textblock_append_c(tb, TERM_L_BLUE, " %d", chance2);
		textblock_append(tb, " percent chance to hit such a creature in melee (if you can see it).  ");
	}
}

/**
 * Append the experience value description to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_exp(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	const char *ordinal, *article;
	char buf[20] = "";
	long exp_integer, exp_fraction;
	s16b level;

	assert(tb && race && lore);

	/* Introduction */
	if (rf_has(known_flags, RF_UNIQUE))
		textblock_append(tb, "Killing");
	else
		textblock_append(tb, "A kill of");

	textblock_append(tb, " this creature");

	/* calculate the integer exp part */
	exp_integer = (long)race->mexp * race->level / p_ptr->lev;

	/* calculate the fractional exp part scaled by 100, must use long arithmetic to avoid overflow */
	exp_fraction = ((((long)race->mexp * race->level % p_ptr->lev) * (long)1000 / p_ptr->lev + 5) / 10);

	/* Calculate textual representation */
	strnfmt(buf, sizeof(buf), "%ld", (long)exp_integer);
	if (exp_fraction) my_strcat(buf, format(".%02ld", (long)exp_fraction), sizeof(buf));

	/* Mention the experience */
	textblock_append(tb, " is worth ");
	textblock_append_c(tb, TERM_BLUE, format("%s point%s", buf, PLURAL((exp_integer == 1) && (exp_fraction == 0))));

	/* Take account of annoying English */
	ordinal = "th";
	level = p_ptr->lev % 10;
	if ((p_ptr->lev / 10) == 1) /* nothing */;
	else if (level == 1) ordinal = "st";
	else if (level == 2) ordinal = "nd";
	else if (level == 3) ordinal = "rd";

	/* Take account of "leading vowels" in numbers */
	article = "a";
	level = p_ptr->lev;
	if ((level == 8) || (level == 11) || (level == 18)) article = "an";

	/* Mention the dependance on the player's level */
	textblock_append(tb, " for %s %lu%s level character.  ", article, (long)level, ordinal);
}

/**
 * Append the monster drop description to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_drop(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	int n = 0;
	monster_sex_t msex = MON_SEX_NEUTER;

	assert(tb && race && lore);

	/* Extract a gender (if applicable) */
	msex = lore_monster_sex(race);

	/* Count maximum drop */
	n = mon_create_drop_count(race, TRUE);

	/* Drops gold and/or items */
	if (n > 0) {
		bool only_item = rf_has(known_flags, RF_ONLY_ITEM);
		bool only_gold = rf_has(known_flags, RF_ONLY_GOLD);

		textblock_append(tb, "%s may carry", lore_pronoun_nominative(msex, TRUE));

		/* Count drops */
		if (n == 1)
			textblock_append_c(tb, TERM_BLUE, " a single ");
		else if (n == 2)
			textblock_append_c(tb, TERM_BLUE, " one or two ");
		else {
			textblock_append(tb, " up to ");
			textblock_append_c(tb, TERM_BLUE, format("%d ", n));
		}

		/* Quality */
		if (rf_has(known_flags, RF_DROP_GREAT))
			textblock_append_c(tb, TERM_BLUE, "exceptional ");
		else if (rf_has(known_flags, RF_DROP_GOOD))
			textblock_append_c(tb, TERM_BLUE, "good ");

		/* Objects or treasures */
		if (only_item && only_gold)
			textblock_append_c(tb, TERM_BLUE, "error%s", PLURAL(n));
		else if (only_item && !only_gold)
			textblock_append_c(tb, TERM_BLUE, "object%s", PLURAL(n));
		else if (!only_item && only_gold)
			textblock_append_c(tb, TERM_BLUE, "treasure%s", PLURAL(n));
		else if (!only_item && !only_gold)
			textblock_append_c(tb, TERM_BLUE, "object%s or treasure%s", PLURAL(n), PLURAL(n));

		textblock_append(tb, ".  ");
	}
}

/**
 * Append the monster abilities (resists, weaknesses, other traits) to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency. Note the macros that are used to simplify the code.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_abilities(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	int list_index;
	const char *descs[64];
	const char *initial_pronoun;
	bool prev = FALSE;
	monster_sex_t msex = MON_SEX_NEUTER;

	/* "Local" macros for easier reading; undef'd at end of function */
	#define LORE_INSERT_FLAG_DESCRIPTION(x) lore_insert_flag_description((x), known_flags, descs, list_index)
	#define LORE_INSERT_UNKNOWN_VULN(x) lore_insert_unknown_vulnerability((x), known_flags, lore, descs, list_index)

	assert(tb && race && lore);

	/* Extract a gender (if applicable) and get a pronoun for the start of sentences */
	msex = lore_monster_sex(race);
	initial_pronoun = lore_pronoun_nominative(msex, TRUE);

	/* Collect special abilities. */
	list_index = 0;
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_OPEN_DOOR);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_BASH_DOOR);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_PASS_WALL);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_KILL_WALL);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_MOVE_BODY);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_KILL_BODY);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_TAKE_ITEM);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_KILL_ITEM);

	if (list_index > 0) {
		textblock_append(tb, "%s can ", initial_pronoun);
		lore_append_list(tb, descs, list_index, TERM_WHITE, "and");
		textblock_append(tb, ".  ");
	}

	/* Describe detection traits */
	list_index = 0;
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_INVISIBLE);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_COLD_BLOOD);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_EMPTY_MIND);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_WEIRD_MIND);

	if (list_index > 0) {
		textblock_append(tb, "%s is ", initial_pronoun);
		lore_append_list(tb, descs, list_index, TERM_WHITE, "and");
		textblock_append(tb, ".  ");
	}

	/* Describe special things */
	if (rf_has(known_flags, RF_UNAWARE))
		textblock_append(tb, "%s disguises itself to look like something else.  ", initial_pronoun);
	if (rf_has(known_flags, RF_MULTIPLY))
		textblock_append_c(tb, TERM_ORANGE, "%s breeds explosively.  ", initial_pronoun);
	if (rf_has(known_flags, RF_REGENERATE))
		textblock_append(tb, "%s regenerates quickly.  ", initial_pronoun);
	if (rf_has(known_flags, RF_HAS_LIGHT))
		textblock_append(tb, "%s illuminates %s surroundings.  ", initial_pronoun, lore_pronoun_possessive(msex, FALSE));

	/* Collect susceptibilities */
	list_index = 0;
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_HURT_ROCK);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_HURT_LIGHT);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_HURT_FIRE);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_HURT_COLD);

	if (list_index > 0) {
		textblock_append(tb, "%s is hurt by ", initial_pronoun);
		lore_append_list(tb, descs, list_index, TERM_VIOLET, "and");
		prev = TRUE;
	}

	/* Collect immunities and resistances */
	list_index = 0;
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_IM_ACID);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_IM_ELEC);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_IM_FIRE);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_IM_COLD);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_IM_POIS);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_IM_WATER);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_RES_NETH);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_RES_PLAS);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_RES_NEXUS);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_RES_DISE);

	/* Note lack of vulnerability as a resistance */
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_HURT_LIGHT);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_HURT_ROCK);

	if (list_index > 0) {
		/* Output connecting text */
		if (prev)
			textblock_append(tb, ", but resists ");
		else
			textblock_append(tb, "%s resists ", initial_pronoun);

		lore_append_list(tb, descs, list_index, TERM_L_UMBER, "and");
		prev = TRUE;
	}

	/* Collect known but average susceptibilities */
	list_index = 0;
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_IM_ACID);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_IM_ELEC);
	if (rf_has(lore->flags, RF_IM_FIRE)   && !rf_has(known_flags, RF_IM_FIRE) && !rf_has(known_flags, RF_HURT_FIRE))
		descs[list_index++] = lore_describe_race_flag(RF_HURT_FIRE);
	if (rf_has(lore->flags, RF_IM_COLD)   && !rf_has(known_flags, RF_IM_COLD) && !rf_has(known_flags, RF_HURT_COLD))
		descs[list_index++] = lore_describe_race_flag(RF_HURT_COLD);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_IM_POIS);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_IM_WATER);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_RES_NETH);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_RES_PLAS);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_RES_NEXUS);
	list_index = LORE_INSERT_UNKNOWN_VULN(RF_RES_DISE);

	if (list_index > 0) {
		/* Output connecting text */
		if (prev)
			textblock_append(tb, ", and does not resist ");
		else
			textblock_append(tb, "%s does not resist ", initial_pronoun);

		lore_append_list(tb, descs, list_index, TERM_L_UMBER, "or");
		prev = TRUE;
	}

	/* Collect non-effects */
	list_index = 0;
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_NO_STUN);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_NO_FEAR);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_NO_CONF);
	list_index = LORE_INSERT_FLAG_DESCRIPTION(RF_NO_SLEEP);

	if (list_index > 0) {
		/* Output connecting text */
		if (prev)
			textblock_append(tb, ", and cannot be ");
		else
			textblock_append(tb, "%s cannot be ", initial_pronoun);

		lore_append_list(tb, descs, list_index, TERM_L_UMBER, "or");
		prev = TRUE;
	}

	if (prev)
		textblock_append(tb, ".  ");

	#undef LORE_INSERT_FLAG_DESCRIPTION
	#undef LORE_INSERT_UNKNOWN_VULN
}

/**
 * Append how the monster reacts to intruders and at what distance it does so.
 *
 * Known race flags are passed in for simplicity/efficiency. Note the macros that are used to simplify the checks; they append to an array.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_awareness(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	monster_sex_t msex = MON_SEX_NEUTER;

	assert(tb && race && lore);

	/* Extract a gender (if applicable) */
	msex = lore_monster_sex(race);

	/* Do we know how aware it is? */
	if ((((int)lore->wake * (int)lore->wake) > race->sleep) ||
	    (lore->ignore == MAX_UCHAR) ||
	    ((race->sleep == 0) && (lore->tkills >= 10)))
	{
		const char *aware = lore_describe_awareness(race->sleep);
		textblock_append(tb, "%s %s intruders, which %s may notice from ", lore_pronoun_nominative(msex, TRUE), aware, lore_pronoun_nominative(msex, FALSE));
		textblock_append_c(tb, TERM_L_BLUE, "%d", (OPT(birth_small_range) ? 5 : 10) * race->aaf);
		textblock_append(tb, " feet.  ");
	}
}

/**
 * Append information about what other races the monster appears with and if they work together.
 *
 * Known race flags are passed in for simplicity/efficiency. Note the macros that are used to simplify the checks; they append to an array.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 */
static void lore_append_friends(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE])
{
	monster_sex_t msex = MON_SEX_NEUTER;

	assert(tb && race && lore);

	/* Extract a gender (if applicable) */
	msex = lore_monster_sex(race);

	/* Describe friends */
	if (race->friends || race->friends_base) {
		textblock_append(tb, "%s may appear with other monsters", lore_pronoun_nominative(msex, TRUE));
		if (rf_has(known_flags, RF_GROUP_AI))
			textblock_append(tb, " and hunts in packs");
		textblock_append(tb, ".  ");
	}
}

/**
 * Append the monster's attack spells to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency. Note the macros that are used to simplify the code.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 * \param spell_colors is a list of colors that is associated with each RSF_ spell.
 */
static void lore_append_spells(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE], const int spell_colors[RSF_MAX])
{
	int i, average_frequency, casting_frequency;
	monster_sex_t msex = MON_SEX_NEUTER;
	bool breath = FALSE;
	bool magic = FALSE;
	int list_index;
	static const int list_size = 64;
	const char *initial_pronoun;
	const char *name_list[list_size];
	int color_list[list_size];
	int damage_list[list_size];
	bool know_hp;

	/* "Local" macros for easier reading; undef'd at end of function */
	#define LORE_INSERT_SPELL_DESCRIPTION(x) lore_insert_spell_description((x), race, lore, spell_colors, know_hp, name_list, color_list, damage_list, list_index)
	#define LORE_RESET_LISTS() { list_index = 0; for(i = 0; i < list_size; i++) { damage_list[i] = 0; color_list[i] = TERM_WHITE; } }

	assert(tb && race && lore);

	know_hp = know_armour(race, lore);

	/* Extract a gender (if applicable) and get a pronoun for the start of sentences */
	msex = lore_monster_sex(race);
	initial_pronoun = lore_pronoun_nominative(msex, TRUE);

	/* Collect innate attacks */
	LORE_RESET_LISTS();
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_SHRIEK);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_ARROW_1);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_ARROW_2);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_ARROW_3);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_ARROW_4);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BOULDER);

	if (list_index > 0) {
		textblock_append(tb, "%s may ", initial_pronoun);
		lore_append_spell_descriptions(tb, name_list, color_list, damage_list, list_index, "or");
		textblock_append(tb, ".  ");
	}

	/* Collect breaths */
	LORE_RESET_LISTS();
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_ACID);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_ELEC);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_FIRE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_COLD);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_POIS);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_NETH);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_LIGHT);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_DARK);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_SOUN);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_CHAO);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_DISE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_NEXU);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_TIME);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_INER);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_GRAV);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_SHAR);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_PLAS);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_WALL);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BR_MANA);
	/* Enum counting note: RSF_BR_CONF isn't used anymore */

	if (list_index > 0) {
		breath = TRUE;
		textblock_append(tb, "%s may ", initial_pronoun);
		textblock_append_c(tb, TERM_L_RED, "breathe ");
		lore_append_spell_descriptions(tb, name_list, color_list, damage_list, list_index, "or");
	}

	/* Collect spell information */
	LORE_RESET_LISTS();

	/* Ball spells */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_MANA);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_DARK);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_WATE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_NETH);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_FIRE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_ACID);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_COLD);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_ELEC);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BA_POIS);

	/* Bolt spells */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_MANA);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_PLAS);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_ICEE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_WATE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_NETH);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_FIRE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_ACID);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_COLD);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_ELEC);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BO_POIS);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_MISSILE);

	/* Curses */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BRAIN_SMASH);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_MIND_BLAST);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_CAUSE_4);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_CAUSE_3);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_CAUSE_2);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_CAUSE_1);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_FORGET);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_SCARE);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BLIND);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_CONF);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_SLOW);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_HOLD);

	/* Healing and haste */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_DRAIN_MANA);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_HEAL);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_HASTE);

	/* Teleports */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_BLINK);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_TPORT);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_TELE_TO);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_TELE_AWAY);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_TELE_LEVEL);

	/* Annoyances */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_DARKNESS);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_TRAPS);

	/* Summoning */
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_KIN);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_MONSTER);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_MONSTERS);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_ANIMAL);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_SPIDER);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_HOUND);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_HYDRA);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_AINU);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_DEMON);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_UNDEAD);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_DRAGON);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_HI_UNDEAD);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_HI_DRAGON);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_HI_DEMON);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_WRAITH);
	list_index = LORE_INSERT_SPELL_DESCRIPTION(RSF_S_UNIQUE);

	if (list_index > 0) {
		magic = TRUE;

		/* Intro */
		if (breath)
			textblock_append(tb, ", and may ");
		else
			textblock_append(tb, "%s may ", initial_pronoun);

		/* Verb Phrase */
		textblock_append_c(tb, TERM_L_RED, "cast spells");

		/* Adverb */
		if (rf_has(known_flags, RF_SMART)) textblock_append(tb, " intelligently");

		/* List */
		textblock_append(tb, " which ");
		lore_append_spell_descriptions(tb, name_list, color_list, damage_list, list_index, "or");
	}

	/* End the sentence about innate/other spells */
	if (breath || magic) {
		/* Calculate total casting and average frequency */
		casting_frequency = lore->cast_innate + lore->cast_spell;
		average_frequency = (race->freq_innate + race->freq_spell) / 2;

		if (casting_frequency > 100) {
			/* Describe the spell frequency */
			textblock_append(tb, "; ");
			textblock_append_c(tb, TERM_L_GREEN, "1");
			textblock_append(tb, " time in ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", 100 / average_frequency);
		}
		else if (casting_frequency) {
			/* Guess at the frequency */
			average_frequency = ((average_frequency + 9) / 10) * 10;
			textblock_append(tb, "; about ");
			textblock_append_c(tb, TERM_L_GREEN, "1");
			textblock_append(tb, " time in ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", 100 / average_frequency);
		}

		textblock_append(tb, ".  ");
	}

	#undef LORE_INSERT_SPELL_DESCRIPTION
	#undef LORE_RESET_LISTS
}

/**
 * Append the monster's melee attacks to a textblock.
 *
 * Known race flags are passed in for simplicity/efficiency.
 *
 * \param tb is the textblock we are adding to.
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 * \param known_flags is the preprocessed bitfield of race flags known to the player.
 * \param melee_colors is a list of colors that is associated with each RBE_ effect.
 */
static void lore_append_attack(textblock *tb, const monster_race *race, const monster_lore *lore, bitflag known_flags[RF_SIZE], const int melee_colors[RBE_MAX])
{
	int i, total_attacks, described_count;
	monster_sex_t msex = MON_SEX_NEUTER;

	assert(tb && race && lore);

	/* Extract a gender (if applicable) */
	msex = lore_monster_sex(race);

	/* Notice lack of attacks */
	if (rf_has(known_flags, RF_NEVER_BLOW)) {
		textblock_append(tb, "%s has no physical attacks.  ", lore_pronoun_nominative(msex, TRUE));
		return;
	}

	/* Count the number of known attacks */
	for (total_attacks = 0, i = 0; i < MONSTER_BLOW_MAX; i++) {
		/* Skip non-attacks */
		if (!race->blow[i].method) continue;

		/* Count known attacks */
		if (lore->blows[i])
			total_attacks++;
	}

	/* Describe the lack of knowledge */
	if (total_attacks == 0) {
		textblock_append(tb, "Nothing is known about %s attack.  ", lore_pronoun_possessive(msex, FALSE));
		return;
	}

	described_count = 0;

	/* Describe each melee attack */
	for (i = 0; i < MONSTER_BLOW_MAX; i++) {
		int dice, sides;
		const char *method_str = NULL;
		const char *effect_str = NULL;

		/* Skip unknown and undefined attacks */
		if (!race->blow[i].method || !lore->blows[i]) continue;

		/* Extract the attack info */
		dice = race->blow[i].d_dice;
		sides = race->blow[i].d_side;
		method_str = lore_describe_blow_method(race->blow[i].method);
		effect_str = lore_describe_blow_effect(race->blow[i].effect);

		/* Introduce the attack description */
		if (described_count == 0)
			textblock_append(tb, "%s can ", lore_pronoun_nominative(msex, TRUE));
		else if (described_count < total_attacks - 1)
			textblock_append(tb, ", ");
		else
			textblock_append(tb, ", and ");

		/* Describe the method */
		textblock_append(tb, method_str);

		/* Describe the effect (if any) */
		if (effect_str && strlen(effect_str) > 0) {
			/* Describe the attack type */
			textblock_append(tb, " to ");
			textblock_append_c(tb, melee_colors[race->blow[i].effect], effect_str);

			/* Describe damage (if known) */
			if (dice && sides) {
				textblock_append(tb, " with damage ");
				textblock_append_c(tb, TERM_L_GREEN, "%dd%d", dice, sides);
			}
		}

		described_count++;
	}

	textblock_append(tb, ".  ");
}

/**
 * Place a monster recall title into a textblock.
 *
 * If graphics are turned on, this appends the title with the appropriate tile. Note: if the title is the only thing in the textblock, make sure to append a newline so that the textui stuff works properly. 
 *
 * \param tb is the textblock we are placing the title into.
 * \param race is the monster race we are describing.
 */
void lore_title(textblock *tb, const monster_race *race)
{
	byte standard_attr, optional_attr;
	wchar_t standard_char, optional_char;

	assert(race);

	/* Get the chars */
	standard_char = race->d_char;
	optional_char = race->x_char;

	/* Get the attrs */
	standard_attr = race->d_attr;
	optional_attr = race->x_attr;

	/* A title (use "The" for non-uniques) */
	if (!rf_has(race->flags, RF_UNIQUE))
		textblock_append(tb, "The ");
	else if (OPT(purple_uniques)) {
		standard_attr = TERM_VIOLET;
		if (!(optional_attr & 0x80))
			optional_attr = TERM_VIOLET;
	}

	/* Dump the name and then append standard attr/char info */
	textblock_append(tb, race->name);

	textblock_append(tb, " ('");
	textblock_append_pict(tb, standard_attr, standard_char);
	textblock_append(tb, "')");

	if (((optional_attr != standard_attr) || (optional_char != standard_char)) && (tile_width == 1) && (tile_height == 1)) {
		/* Append the "optional" attr/char info */
		textblock_append(tb, " ('");
		textblock_append_pict(tb, optional_attr, optional_char);
		textblock_append(tb, "')");
	}
}

/**
 * Place a full monster recall description (with title) into a textblock, with or without spoilers.
 *
 * \param tb is the textblock we are placing the description into.
 * \param race is the monster race we are describing.
 * \param original_lore is the known information about the monster race.
 * \param spoilers indicates what information is used; `TRUE` will display full information without subjective information and monstor flavor, while `FALSE` only shows what the player knows.
 */
void lore_description(textblock *tb, const monster_race *race, const monster_lore *original_lore, bool spoilers)
{
	monster_lore mutable_lore;
	monster_lore *lore = &mutable_lore;
	bitflag known_flags[RF_SIZE];
	int melee_colors[RBE_MAX], spell_colors[RSF_MAX];

	assert(tb && race && original_lore);

	/* Determine the special attack colors */
	get_attack_colors(melee_colors, spell_colors);

	/* Hack -- create a copy of the monster-memory that we can modify */
	COPY(lore, original_lore, monster_lore);

	/* Assume some "obvious" flags */
	flags_set(lore->flags, RF_SIZE, RF_OBVIOUS_MASK, FLAG_END);

	/* Killing a monster reveals some properties */
	if (lore->tkills > 0) {
		/* Know "race", "forced", and "drop" flags */
		flags_set(lore->flags, RF_SIZE, RF_RACE_MASK, FLAG_END);
		flags_set(lore->flags, RF_SIZE, RF_DROP_MASK, FLAG_END);
		rf_on(lore->flags, RF_FORCE_DEPTH);
	}

	/* Now get the known monster flags */
	monster_flags_known(race, lore, known_flags);

	/* Cheat -- know everything */
	if (OPT(cheat_know) || spoilers)
		cheat_monster_lore(race, lore);

	/* Appending the title here simplifies code in the callers. It also causes a crash when generating spoilers (we don't need titles for them anwyay) */
	if (!spoilers) {
		lore_title(tb, race);
		textblock_append(tb, "\n");
	}

	/* Show kills of monster vs. player(s) */
	if (!spoilers)
		lore_append_kills(tb, race, lore, known_flags);

	/* If we are generating spoilers, we want to output as UTF-8. As of 3.5, the values in race->name and race->text remain unconverted from the UTF-8 edit files. */
	lore_append_flavor(tb, race, spoilers);

	/* Describe the monster type, speed, life, and armor */
	lore_append_movement(tb, race, lore, known_flags);

	if (!spoilers)
		lore_append_toughness(tb, race, lore, known_flags);

	/* Describe the experience and item reward when killed */
	if (!spoilers)
		lore_append_exp(tb, race, lore, known_flags);

	lore_append_drop(tb, race, lore, known_flags);

	/* Describe the special properties of the monster */
	lore_append_abilities(tb, race, lore, known_flags);
	lore_append_awareness(tb, race, lore, known_flags);
	lore_append_friends(tb, race, lore, known_flags);

	/* Describe the spells, spell-like abilities and melee attacks */
	lore_append_spells(tb, race, lore, known_flags, spell_colors);
	lore_append_attack(tb, race, lore, known_flags, melee_colors);

	/* Notice "Quest" monsters */
	if (rf_has(race->flags, RF_QUESTOR))
		textblock_append(tb, "You feel an intense desire to kill this monster...  ");

	textblock_append(tb, "\n");
}

/**
 * Display monster recall modally and wait for a keypress.
 *
 * This is intended to be called when the main window is active (hence the message flushing).
 *
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 */
void lore_show_interactive(const monster_race *race, const monster_lore *lore)
{
	textblock *tb;
	assert(race && lore);

	message_flush();

	tb = textblock_new();
	lore_description(tb, race, lore, FALSE);
	textui_textblock_show(tb, SCREEN_REGION, NULL);
	textblock_free(tb);
}

/**
 * Display monster recall statically.
 *
 * This is intended to be called in a subwindow, since it clears the entire window before drawing, and has no interactivity.
 *
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 */
void lore_show_subwindow(const monster_race *race, const monster_lore *lore)
{
	int y;
	textblock *tb;

	assert(race && lore);

	/* Erase the window, since textui_textblock_place() only clears what it needs */
	for (y = 0; y < Term->hgt; y++)
		Term_erase(0, y, 255);

	tb = textblock_new();
	lore_description(tb, race, lore, FALSE);
	textui_textblock_place(tb, SCREEN_REGION, NULL);
	textblock_free(tb);
}
