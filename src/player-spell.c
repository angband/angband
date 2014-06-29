/**
   \file player-spell.c
   \brief Spell and prayer casting/praying
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
#include "cmd-core.h"
#include "effects.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "spells.h"

/**
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
const byte adj_mag_fail[STAT_RANGE] =
{
	99	/* 3 */,
	99	/* 4 */,
	99	/* 5 */,
	99	/* 6 */,
	99	/* 7 */,
	50	/* 8 */,
	30	/* 9 */,
	20	/* 10 */,
	15	/* 11 */,
	12	/* 12 */,
	11	/* 13 */,
	10	/* 14 */,
	9	/* 15 */,
	8	/* 16 */,
	7	/* 17 */,
	6	/* 18/00-18/09 */,
	6	/* 18/10-18/19 */,
	5	/* 18/20-18/29 */,
	5	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	4	/* 18/60-18/69 */,
	4	/* 18/70-18/79 */,
	4	/* 18/80-18/89 */,
	3	/* 18/90-18/99 */,
	3	/* 18/100-18/109 */,
	2	/* 18/110-18/119 */,
	2	/* 18/120-18/129 */,
	2	/* 18/130-18/139 */,
	2	/* 18/140-18/149 */,
	1	/* 18/150-18/159 */,
	1	/* 18/160-18/169 */,
	1	/* 18/170-18/179 */,
	1	/* 18/180-18/189 */,
	1	/* 18/190-18/199 */,
	0	/* 18/200-18/209 */,
	0	/* 18/210-18/219 */,
	0	/* 18/220+ */
};

/**
 * Stat Table (INT/WIS) -- failure rate adjustment
 */
const int adj_mag_stat[STAT_RANGE] =
{
	-5	/* 3 */,
	-4	/* 4 */,
	-3	/* 5 */,
	-3	/* 6 */,
	-2	/* 7 */,
	-1	/* 8 */,
	 0	/* 9 */,
	 0	/* 10 */,
	 0	/* 11 */,
	 0	/* 12 */,
	 0	/* 13 */,
	 1	/* 14 */,
	 2	/* 15 */,
	 3	/* 16 */,
	 4	/* 17 */,
	 5	/* 18/00-18/09 */,
	 6	/* 18/10-18/19 */,
	 7	/* 18/20-18/29 */,
	 8	/* 18/30-18/39 */,
	 9	/* 18/40-18/49 */,
	10	/* 18/50-18/59 */,
	11	/* 18/60-18/69 */,
	12	/* 18/70-18/79 */,
	15	/* 18/80-18/89 */,
	18	/* 18/90-18/99 */,
	21	/* 18/100-18/109 */,
	24	/* 18/110-18/119 */,
	27	/* 18/120-18/129 */,
	30	/* 18/130-18/139 */,
	33	/* 18/140-18/149 */,
	36	/* 18/150-18/159 */,
	39	/* 18/160-18/169 */,
	42	/* 18/170-18/179 */,
	45	/* 18/180-18/189 */,
	48	/* 18/190-18/199 */,
	51	/* 18/200-18/209 */,
	54	/* 18/210-18/219 */,
	57	/* 18/220+ */
};

/**
 * Initialise player spells
 */
void player_spells_init(struct player *p)
{
	int i, num_spells = p->class->magic.total_spells;

	/* None */
	if (!num_spells) return;

	/* Allocate */
	p->spell_flags = mem_zalloc(num_spells * sizeof(byte));
	p->spell_order = mem_zalloc(num_spells * sizeof(byte));

	/* None of the spells have been learned yet */
	for (i = 0; i < num_spells; i++)
		p->spell_order[i] = 99;
}

/**
 * Free player spells
 */
void player_spells_free(struct player *p)
{
	mem_free(p->spell_flags);
	mem_free(p->spell_order);
}

/**
 * Get the spellbook structure from an object which is a book the player can
 * cast from
 */
const class_book *object_to_book(const struct object *obj)
{
	size_t i;

	for (i = 0; i < PY_MAX_BOOKS; i++)
		if ((obj->tval == player->class->magic.books[i].tval) &&
			(obj->sval == player->class->magic.books[i].sval))
			return &player->class->magic.books[i];

	return NULL;
}

const class_spell *spell_by_index(int index)
{
	int book = 0, count = 0;
	const class_magic *m = &player->class->magic;

	/* Check index validity */
	if (index < 0 || index >= m->total_spells)
		return NULL;

	/* Find the book, count the spells in previous books */
	while (count + m->books[book].num_spells < index)
		count += m->books[book++].num_spells;

	/* Find the spell */
	return &m->books[book].spells[index - count];
}

/**
 * Collect spells from a book into the spells[] array, allocating
 * appropriate memory.
 */
int spell_collect_from_book(const object_type *o_ptr, int **spells)
{
	const class_book *book = object_to_book(o_ptr);
	int i, n_spells = 0;

	/* Count the spells */
	for (i = 0; i < book->num_spells; i++)
		n_spells++;

	/* Allocate the array */
	*spells = mem_zalloc(n_spells * sizeof(*spells));

	/* Write the spells */
	for (i = 0; i < book->num_spells; i++)
		(*spells)[i] = book->spells[i].sidx;

	return n_spells;
}


/**
 * Return the number of castable spells in the spellbook 'o_ptr'.
 */
int spell_book_count_spells(const object_type *o_ptr,
		bool (*tester)(int spell))
{
	const class_book *book = object_to_book(o_ptr);
	int i, n_spells = 0;

	for (i = 0; i < book->num_spells; i++)
		if (tester(book->spells[i].sidx))
			n_spells++;

	return n_spells;
}


/**
 * True if at least one spell in spells[] is OK according to spell_test.
 */
bool spell_okay_list(bool (*spell_test)(int spell),
		const int spells[], int n_spells)
{
	int i;
	bool okay = FALSE;

	for (i = 0; i < n_spells; i++)
	{
		if (spell_test(spells[i]))
			okay = TRUE;
	}

	return okay;
}

/**
 * True if the spell is castable.
 */
bool spell_okay_to_cast(int spell)
{
	return (player->spell_flags[spell] & PY_SPELL_LEARNED);
}

/**
 * True if the spell can be studied.
 */
bool spell_okay_to_study(int spell)
{
	const class_spell *s_ptr = spell_by_index(spell);
	return (s_ptr->slevel <= player->lev) &&
			!(player->spell_flags[spell] & PY_SPELL_LEARNED);
}

/**
 * True if the spell is browsable.
 */
bool spell_okay_to_browse(int spell)
{
	const class_spell *s_ptr = spell_by_index(spell);
	return (s_ptr->slevel < 99);
}

/**
 * Spell failure adjustment by casting stat level
 * (or whatever realm whatever - NRM)
 */
int fail_adjust(struct player *p)
{
	int stat = player->class->magic.spell_realm->stat;
	return adj_mag_stat[player->state.stat_ind[stat]];
}

/**
 * Spell minimum failure casting stat level
 * (or whatever realm whatever - NRM)
 */
int min_fail(struct player *p)
{
	int stat = player->class->magic.spell_realm->stat;
	return adj_mag_fail[player->state.stat_ind[stat]];
}

/**
 * Returns chance of failure for a spell
 */
s16b spell_chance(int spell)
{
	int chance, minfail;

	const class_spell *s_ptr;

	/* Paranoia -- must be literate */
	if (player->class->magic.total_spells == 0) return (100);

	/* Get the spell */
	s_ptr = spell_by_index(spell);

	/* Extract the base spell failure rate */
	chance = s_ptr->sfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (player->lev - s_ptr->slevel);

	/* Reduce failure rate by realm adjustment */
	chance -= fail_adjust(player);

	/* Not enough mana to cast */
	if (s_ptr->smana > player->csp)
	{
		chance += 5 * (s_ptr->smana - player->csp);
	}

	/* Extract the minimum failure rate due to realm */
	minfail = min_fail(player);

	/* Non mage/priest characters never get better than 5 percent */
	if (!player_has(PF_ZERO_FAIL) && minfail < 5)
	{
		minfail = 5;
	}

	/* Priest prayer penalty for "edged" weapons (before minfail) */
	if (player->state.icky_wield)
	{
		chance += 25;
	}

	/* Fear makes spells harder (before minfail) */
	/* Note that spells that remove fear have a much lower fail rate than
	 * surrounding spells, to make sure this doesn't cause mega fail */
	if (player_of_has(player, OF_AFRAID)) chance += 20;

	/* Minimal and maximal failure rate */
	if (chance < minfail) chance = minfail;
	if (chance > 50) chance = 50;

	/* Stunning makes spells harder (after minfail) */
	if (player->timed[TMD_STUN] > 50) chance += 25;
	else if (player->timed[TMD_STUN]) chance += 15;

	/* Amnesia doubles failure change */
	if (player->timed[TMD_AMNESIA]) chance = 50 + chance / 2;

	/* Always a 5 percent chance of working */
	if (chance > 95) chance = 95;

	/* Return the chance */
	return (chance);
}


/**
 * Learn the specified spell.
 */
void spell_learn(int spell)
{
	int i;
	const char *p = player->class->magic.spell_realm->spell_noun;

	/* Learn the spell */
	player->spell_flags[spell] |= PY_SPELL_LEARNED;

	/* Find the next open entry in "spell_order[]" */
	for (i = 0; i < player->class->magic.total_spells; i++)
	{
		/* Stop at the first empty space */
		if (player->spell_order[i] == 99) break;
	}

	/* Add the spell to the known list */
	player->spell_order[i] = spell;

	/* Mention the result */
	msgt(MSG_STUDY, "You have learned the %s of %s.", p,
		 spell_by_index(spell)->name);

	/* One less spell available */
	player->upkeep->new_spells--;

	/* Message if needed */
	if (player->upkeep->new_spells)
	{
		/* Message */
		msg("You can learn %d more %s%s.", player->upkeep->new_spells, p, 
			PLURAL(player->upkeep->new_spells));
	}

	/* Redraw Study Status */
	player->upkeep->redraw |= (PR_STUDY | PR_OBJECT);
}

static bool player_spell_effect(int spell, int dir);

/**
 * Cast the specified spell
 */
bool spell_cast(int spell, int dir)
{
	int chance;

	/* Get the spell */
	const class_spell *s_ptr = spell_by_index(spell);

	/* Spell failure chance */
	chance = spell_chance(spell);

	/* Failed spell */
	if (randint0(100) < chance)
	{
		flush();
		msg("You failed to concentrate hard enough!");
	}

	/* Process spell */
	else
	{
		/* Cast the spell */
		if (!player_spell_effect(spell, dir)) return FALSE;

		/* A spell was cast */
		sound(MSG_SPELL);

		if (!(player->spell_flags[spell] & PY_SPELL_WORKED))
		{
			int e = s_ptr->sexp;

			/* The spell worked */
			player->spell_flags[spell] |= PY_SPELL_WORKED;

			/* Gain experience */
			player_exp_gain(player, e * s_ptr->slevel);

			/* Redraw object recall */
			player->upkeep->redraw |= (PR_OBJECT);
		}
	}

	/* Sufficient mana */
	if (s_ptr->smana <= player->csp)
	{
		/* Use some mana */
		player->csp -= s_ptr->smana;
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - player->csp;

		/* No mana left */
		player->csp = 0;
		player->csp_frac = 0;

		/* Message */
		msg("You faint from the effort!");

		/* Bypass free action */
		(void)player_inc_timed(player, TMD_PARALYZED, randint1(5 * oops + 1),
							   TRUE, FALSE);

		/* Damage CON (possibly permanently) */
		if (randint0(100) < 50)
		{
			bool perm = (randint0(100) < 25);

			/* Message */
			msg("You have damaged your health!");

			/* Reduce constitution */
			player_stat_dec(player, A_CON, perm);
		}
	}

	/* Redraw mana */
	player->upkeep->redraw |= (PR_MANA);

	return TRUE;
}

/* Start of old x-spell.c */

static int beam_chance(void)
{
	int plev = player->lev;
	return (player_has(PF_BEAM) ? plev : (plev / 2));
}


static void spell_wonder(int dir)
{
/* This spell should become more useful (more
   controlled) as the player gains experience levels.
   Thus, add 1/5 of the player's level to the die roll.
   This eliminates the worst effects later on, while
   keeping the results quite random.  It also allows
   some potent effects only at high level. */
	effect_wonder(dir, randint1(100) + player->lev / 5, beam_chance());
}


static int spell_calculate_value(spell_handler_context_t *context)
{
	int final = 0;

	if (context->value.base > 0 || (context->value.dice > 0 && context->value.sides > 0))
		final = context->value.base + damroll(context->value.dice, context->value.sides);

	return final;
}


static bool spell_handler_BOLT(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	fire_bolt(context->p1, context->dir, dam);
	return TRUE;
}

static bool spell_handler_BEAM(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	fire_beam(context->p1, context->dir, dam);
	return TRUE;
}

static bool spell_handler_BOLT_OR_BEAM(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	fire_bolt_or_beam(context->beam + context->p2, context->p1, context->dir,
					  dam);
	return TRUE;
}

static bool spell_handler_BALL(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	fire_ball(context->p1, context->dir, dam, context->p2);
	return TRUE;
}

static bool spell_handler_REND_SOUL(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	fire_bolt_or_beam(context->beam / 4, GF_NETHER, context->dir, dam);
	return TRUE;
}

static bool spell_handler_SPEAR_OF_LIGHT(spell_handler_context_t *context)
{
	/* light_line() has hardcoded 6d8 damage. */
	msg("A line of blue shimmering light appears.");
	light_line(context->dir);
	return TRUE;
}

static bool spell_handler_ANNIHILATION(spell_handler_context_t *context)
{
	drain_life(context->dir, context->value.base);
	return TRUE;
}

static bool spell_handler_LIGHT_AREA(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	int radius = player->lev / 10 + 1;
	light_area(dam, radius);
	return TRUE;
}

static bool spell_handler_ORB_OF_DRAINING(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	int radius = (player->lev < 30) ? 2 : 3;
	fire_ball(GF_HOLY_ORB, context->dir, dam, radius);
	return TRUE;
}

static bool spell_handler_METEOR_SWARM(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	fire_swarm(context->value.m_bonus, GF_METEOR, context->dir, dam, 1);
	return TRUE;
}

static bool spell_handler_WONDER(spell_handler_context_t *context)
{
	(void)spell_wonder(context->dir);
	return TRUE;
}

static bool spell_handler_DISPEL_UNDEAD(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	dispel_undead(dam);
	return TRUE;
}

static bool spell_handler_DISPEL_EVIL(spell_handler_context_t *context)
{
	int dam = spell_calculate_value(context);
	dispel_evil(dam);
	return TRUE;
}

static bool spell_handler_CONFUSE_MONSTER(spell_handler_context_t *context)
{
	(void)confuse_monster(context->dir, player->lev, TRUE);
	return TRUE;
}

static bool spell_handler_SLEEP_MONSTER(spell_handler_context_t *context)
{
	(void)sleep_monster(context->dir, TRUE);
	return TRUE;
}

static bool spell_handler_POLYMORPH_OTHER(spell_handler_context_t *context)
{
	(void)poly_monster(context->dir);
	return TRUE;
}

static bool spell_handler_SLOW_MONSTER(spell_handler_context_t *context)
{
	(void)slow_monster(context->dir);
	return TRUE;
}

static bool spell_handler_MASS_SLEEP(spell_handler_context_t *context)
{
	(void)sleep_monsters(TRUE);
	return TRUE;
}

static bool spell_handler_BANISHMENT(spell_handler_context_t *context)
{
	return banishment();
}

static bool spell_handler_MASS_BANISHMENT(spell_handler_context_t *context)
{
	(void)mass_banishment();
	return TRUE;
}

static bool spell_handler_SCARE_MONSTER(spell_handler_context_t *context)
{
	(void)fear_monster(context->dir, player->lev, TRUE);
	return TRUE;
}

static bool spell_handler_SANCTUARY(spell_handler_context_t *context)
{
	(void)sleep_monsters_touch(TRUE);
	return TRUE;
}

static bool spell_handler_TURN_UNDEAD(spell_handler_context_t *context)
{
	(void)turn_undead(TRUE);
	return TRUE;
}

static bool spell_handler_DETECT_MONSTERS(spell_handler_context_t *context)
{
	detect_monsters_normal(TRUE);
	return TRUE;
}

static bool spell_handler_FIND_TRAPS_DOORS(spell_handler_context_t *context)
{
	detect_traps(TRUE);
	detect_doorstairs(TRUE);
	return TRUE;
}

static bool spell_handler_OBJECT_DETECTION(spell_handler_context_t *context)
{
	(void)detect_treasure(TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_DETECT_INVISIBLE(spell_handler_context_t *context)
{
	(void)detect_monsters_normal(TRUE);
	(void)detect_monsters_invis(TRUE);
	return TRUE;
}

static bool spell_handler_DETECT_EVIL(spell_handler_context_t *context)
{
	(void)detect_monsters_evil(TRUE);
	return TRUE;
}

static bool spell_handler_TREASURE_DETECTION(spell_handler_context_t *context)
{
	(void)detect_treasure(TRUE, FALSE);
	return TRUE;
}

static bool spell_handler_SENSE_SURROUNDINGS(spell_handler_context_t *context)
{
	map_area();
	return TRUE;
}

static bool spell_handler_DETECTION(spell_handler_context_t *context)
{
	(void)detect_all(TRUE);
	return TRUE;
}

static bool spell_handler_CLAIRVOYANCE(spell_handler_context_t *context)
{
	wiz_light(cave, FALSE);
	return TRUE;
}

static bool spell_handler_PROBING(spell_handler_context_t *context)
{
	(void)probing();
	return TRUE;
}

static bool spell_handler_TRAP_DOOR_DESTRUCTION(spell_handler_context_t *context)
{
	destroy_doors_touch();
	return TRUE;
}

static bool spell_handler_TURN_STONE_TO_MUD(spell_handler_context_t *context)
{
	(void)wall_to_mud(context->dir);
	return TRUE;
}

static bool spell_handler_DOOR_CREATION(spell_handler_context_t *context)
{
	(void)door_creation();
	return TRUE;
}

static bool spell_handler_EARTHQUAKE(spell_handler_context_t *context)
{
	earthquake(player->py, player->px, 10);
	return TRUE;
}

static bool spell_handler_STAIR_CREATION(spell_handler_context_t *context)
{
	(void)stair_creation();
	return TRUE;
}

static bool spell_handler_WORD_OF_DESTRUCTION(spell_handler_context_t *context)
{
	destroy_area(player->py, player->px, 15, TRUE);
	return TRUE;
}

static bool spell_handler_ALTER_REALITY(spell_handler_context_t *context)
{
	msg("The world changes!");

	/* Leaving */
	player->upkeep->leaving = TRUE;

	return TRUE;
}

static bool spell_handler_RUNE_OF_PROTECTION(spell_handler_context_t *context)
{
	warding_glyph_spell();
	return TRUE;
}

static bool spell_handler_CURE_LIGHT_WOUNDS(spell_handler_context_t *context)
{
	heal_player(context->value.m_bonus, context->value.base);
	player_dec_timed(player, TMD_CUT, 20, TRUE);
	player_dec_timed(player, TMD_CONFUSED, 20, TRUE);
	player_clear_timed(player, TMD_BLIND, TRUE);
	return TRUE;
}

static bool spell_handler_CURE_SERIOUS_WOUNDS(spell_handler_context_t *context)
{
	heal_player(context->value.m_bonus, context->value.base);
	player_clear_timed(player, TMD_CUT, TRUE);
	player_clear_timed(player, TMD_CONFUSED, TRUE);
	player_clear_timed(player, TMD_BLIND, TRUE);
	return TRUE;
}

static bool spell_handler_CURE_CRITICAL_WOUNDS(spell_handler_context_t *context)
{
	heal_player(context->value.m_bonus, context->value.base);
	player_clear_timed(player, TMD_CUT, TRUE);
	player_clear_timed(player, TMD_AMNESIA, TRUE);
	player_clear_timed(player, TMD_CONFUSED, TRUE);
	player_clear_timed(player, TMD_BLIND, TRUE);
	player_clear_timed(player, TMD_POISONED, TRUE);
	player_clear_timed(player, TMD_STUN, TRUE);
	return TRUE;
}

static bool spell_handler_HEAL(spell_handler_context_t *context)
{
	int amt = (player->mhp * context->value.m_bonus) / 100;
	if (amt < context->value.base) amt = context->value.base;

	(void)hp_player(amt);
	(void)player_clear_timed(player, TMD_CUT, TRUE);
	(void)player_clear_timed(player, TMD_AMNESIA, TRUE);
	(void)player_clear_timed(player, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(player, TMD_BLIND, TRUE);
	(void)player_clear_timed(player, TMD_POISONED, TRUE);
	(void)player_clear_timed(player, TMD_STUN, TRUE);
	return TRUE;
}

static bool spell_handler_HEALING(spell_handler_context_t *context)
{
	(void)hp_player(context->value.base);
	(void)player_clear_timed(player, TMD_STUN, TRUE);
	(void)player_clear_timed(player, TMD_CUT, TRUE);
	return TRUE;
}

static bool spell_handler_HOLY_WORD(spell_handler_context_t *context)
{
	(void)dispel_evil(randint1(player->lev * 4));
	(void)hp_player(context->value.base);
	(void)player_clear_timed(player, TMD_AFRAID, TRUE);
	(void)player_clear_timed(player, TMD_POISONED, TRUE);
	(void)player_clear_timed(player, TMD_STUN, TRUE);
	(void)player_clear_timed(player, TMD_CUT, TRUE);
	return TRUE;
}

static bool spell_handler_SLOW_POISON(spell_handler_context_t *context)
{
	(void)player_set_timed(player, TMD_POISONED, player->timed[TMD_POISONED] / 2, TRUE);
	return TRUE;
}

static bool spell_handler_CURE_POISON(spell_handler_context_t *context)
{
	player_clear_timed(player, TMD_POISONED, TRUE);
	return TRUE;
}

static bool spell_handler_RESIST_COLD(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	(void)player_inc_timed(player, TMD_OPP_COLD, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_RESIST_FIRE(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	(void)player_inc_timed(player, TMD_OPP_FIRE, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_RESIST_HEAT_COLD(spell_handler_context_t *context)
{
	int dur1 = spell_calculate_value(context);
	int dur2 = spell_calculate_value(context);
	(void)player_inc_timed(player, TMD_OPP_FIRE, dur1, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_OPP_COLD, dur2, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_RESIST_POISON(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	(void)player_inc_timed(player, TMD_OPP_POIS, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_RESISTANCE(spell_handler_context_t *context)
{
	int time = spell_calculate_value(context);
	(void)player_inc_timed(player, TMD_OPP_ACID, time, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_OPP_ELEC, time, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_OPP_FIRE, time, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_OPP_COLD, time, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_OPP_POIS, time, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_SHIELD(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	(void)player_inc_timed(player, TMD_SHIELD, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_SATISFY_HUNGER(spell_handler_context_t *context)
{
	player_set_food(player, PY_FOOD_MAX - 1);
	return TRUE;
}

static bool spell_handler_REMOVE_FEAR(spell_handler_context_t *context)
{
	(void)player_clear_timed(player, TMD_AFRAID, TRUE);
	return TRUE;
}

static bool spell_handler_HEROISM(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	(void)hp_player(10);
	(void)player_clear_timed(player, TMD_AFRAID, TRUE);
	(void)player_inc_timed(player, TMD_BOLD, dur, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_HERO, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_BERSERKER(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	(void)hp_player(30);
	(void)player_clear_timed(player, TMD_AFRAID, TRUE);
	(void)player_inc_timed(player, TMD_BOLD, dur, TRUE, TRUE);
	(void)player_inc_timed(player, TMD_SHERO, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_BLESS(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	player_inc_timed(player, TMD_BLESSED, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_HASTE_SELF(spell_handler_context_t *context)
{
	if (!player->timed[TMD_FAST])
	{
		int dur = spell_calculate_value(context);
		(void)player_set_timed(player, TMD_FAST, dur, TRUE);
	}
	else
	{
		(void)player_inc_timed(player, TMD_FAST, randint1(5), TRUE, TRUE);
	}
	return TRUE;
}

static bool spell_handler_SENSE_INVISIBLE(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	player_inc_timed(player, TMD_SINVIS, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_PROTECTION_FROM_EVIL(spell_handler_context_t *context)
{
	int dur = spell_calculate_value(context);
	player_inc_timed(player, TMD_PROTEVIL, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_RESTORATION(spell_handler_context_t *context)
{
	(void)do_res_stat(A_STR);
	(void)do_res_stat(A_INT);
	(void)do_res_stat(A_WIS);
	(void)do_res_stat(A_DEX);
	(void)do_res_stat(A_CON);
	return TRUE;
}

static bool spell_handler_REMEMBRANCE(spell_handler_context_t *context)
{
	(void)restore_level();
	return TRUE;
}

static bool spell_handler_TELEPORT_SELF(spell_handler_context_t *context)
{
	int range = spell_calculate_value(context);
	teleport_player(range);
	return TRUE;
}

static bool spell_handler_TELEPORT_OTHER(spell_handler_context_t *context)
{
	teleport_monster(context->dir);
	return TRUE;
}

static bool spell_handler_TELEPORT_LEVEL(spell_handler_context_t *context)
{
	teleport_player_level();
	return TRUE;
}

static bool spell_handler_WORD_OF_RECALL(spell_handler_context_t *context)
{
	return set_recall();
}

static bool spell_handler_BANISH_EVIL(spell_handler_context_t *context)
{
	if (banish_evil(100))
	{
		msg("The power of your god banishes evil!");
	}
	return TRUE;
}

static bool spell_handler_IDENTIFY(spell_handler_context_t *context)
{
	return ident_spell();
}

static bool spell_handler_RECHARGE(spell_handler_context_t *context)
{
	return recharge(spell_calculate_value(context));
}

static bool spell_handler_ENCHANT_ARMOR(spell_handler_context_t *context)
{
	/* Original values used randint0, whereas these start at 1, so we adjust. */
	int plus_ac = spell_calculate_value(context) - 1;

	plus_ac = MAX(plus_ac, 0);

	return enchant_spell(0, 0, plus_ac);
}

static bool spell_handler_ENCHANT_WEAPON(spell_handler_context_t *context)
{
	/* Original values used randint0, whereas these start at 1, so we adjust. */
	int plus_hit = spell_calculate_value(context) - 1;
	int plus_dam = spell_calculate_value(context) - 1;

	plus_hit = MAX(plus_hit, 0);
	plus_dam = MAX(plus_dam, 0);

	return enchant_spell(plus_hit, plus_dam, 0);
}

static bool spell_handler_BRAND_AMMO(spell_handler_context_t *context)
{
	return brand_ammo();
}

static bool spell_handler_BRAND_WEAPON(spell_handler_context_t *context)
{
	brand_weapon();
	return TRUE;
}

static bool spell_handler_REMOVE_CURSE(spell_handler_context_t *context)
{
	/* Remove curse has been removed in 3.4 until curses are redone */
	/* remove_curse(); */
	return FALSE;
}

static bool spell_handler_DISPEL_CURSE(spell_handler_context_t *context)
{
	/* Dispel Curse has been removed in 3.4 until curses are redone */
	/* (void)remove_all_curse(); */
	return FALSE;
}

static bool spell_handler_MAX(spell_handler_context_t *context)
{
	/* Hax lol */
	return FALSE;
}

static const spell_info_t spell_effects[] = {
	#define S_EF(x, a, s) {SPELL_EFFECT_##x, a, s, spell_handler_##x},
	#include "list-player-spells.h"
	#undef S_EF
};

static const spell_info_t *spell_info_for_index(const spell_info_t *list, size_t list_size, int spell_max, int spell)
{
	size_t i;

	if (spell < 0 || spell >= spell_max)
		return NULL;

	for (i = 0; i < list_size; i++) {
		if (list[i].spell == spell)
			return &list[i];
	}

	return NULL;
}

static bool player_spell_effect(int spell, int dir)
{
	spell_handler_f spell_handler = NULL;
	const class_spell *sp = spell_by_index(spell);
	const spell_info_t *spell_info = spell_info_for_index(spell_effects, N_ELEMENTS(spell_effects), SPELL_EFFECT_MAX, sp->effect);
	random_value value;
	int p1, p2;

	if (spell_info == NULL)
		return FALSE;

	spell_handler = spell_info->handler;

	if (spell_handler == NULL)
		return FALSE;

	if (sp->dice != NULL)
		dice_roll(sp->dice, &value);

	/* Usually GF_ type. */
	p1 = sp->params[0];
	/* Usually radius for ball spells, or some other modifier. */
	p2 = sp->params[1];

	spell_handler_context_t context = {
		dir,
		beam_chance(),
		value,
		p1, p2
	};

	return spell_handler(&context);
}

bool spell_is_identify(int spell)
{
	const class_spell *sp = spell_by_index(spell);
	return (sp->effect == SPELL_EFFECT_IDENTIFY);
}

bool spell_needs_aim(int spell)
{
	const class_spell *sp = spell_by_index(spell);
	const spell_info_t *spell_info = spell_info_for_index(spell_effects, N_ELEMENTS(spell_effects), SPELL_EFFECT_MAX, sp->effect);

	if (spell_info == NULL)
		return FALSE;

	return spell_info->aim;
}

static size_t append_random_value_string(char *buffer, size_t size, random_value *rv)
{
	size_t offset = 0;

	if (rv->base > 0) {
		offset += strnfmt(buffer + offset, size - offset, "%d", rv->base);

		if (rv->dice > 0 || rv->sides > 0)
			offset += strnfmt(buffer + offset, size - offset, "+");
	}

	if (rv->dice == 1) {
		offset += strnfmt(buffer + offset, size - offset, "d%d", rv->sides);
	}
	else if (rv->dice > 1) {
		offset += strnfmt(buffer + offset, size - offset, "%dd%d", rv->dice, rv->sides);
	}

	return offset;
}

static void spell_append_value_info(int spell, char *p, size_t len)
{
	const class_spell *sp = spell_by_index(spell);
	const spell_info_t *info = spell_info_for_index(spell_effects, N_ELEMENTS(spell_effects), SPELL_EFFECT_MAX, sp->effect);
	random_value rv;
	const char *type = NULL;
	const char *special = NULL;
	size_t offset = 0;

	if (info == NULL)
		return;

	type = info->info;

	if (sp->dice != NULL)
		dice_roll(sp->dice, &rv);

	/* Handle some special cases where we want to append some additional info */
	switch (sp->effect) {
		case SPELL_EFFECT_CURE_LIGHT_WOUNDS:
		case SPELL_EFFECT_CURE_SERIOUS_WOUNDS:
		case SPELL_EFFECT_CURE_CRITICAL_WOUNDS:
		case SPELL_EFFECT_HEAL:
			/* Append percentage only, as the fixed value is always displayed */
			special = format("/%d%%", rv.m_bonus);
			break;
		case SPELL_EFFECT_METEOR_SWARM:
			/* Append number of projectiles. */
			special = format("x%d", rv.m_bonus);
			break;
	}

	if (type == NULL)
		return;

	offset += strnfmt(p, len, " %s ", type);
	offset += append_random_value_string(p + offset, len - offset, &rv);

	if (special != NULL)
		strnfmt(p + offset, len - offset, "%s", special);
}

void get_spell_info(int spell, char *p, size_t len)
{
	/* Blank 'p' first */
	p[0] = '\0';

	spell_append_value_info(spell, p, len);
}

static int spell_value_base_player_level(void)
{
	return player->lev;
}

static int spell_value_base_orb_of_draining(void)
{
	int base = player_has(PF_ZERO_FAIL) ? (player->lev / 2) : (player->lev / 4);
	base += player->lev;
	return base;
}

expression_base_value_f spell_value_base_by_name(const char *name)
{
	static const struct value_base_s {
		const char *name;
		expression_base_value_f function;
	} value_bases[] = {
		{ "PLAYER_LEVEL", spell_value_base_player_level },
		{ "ORB_OF_DRAINING", spell_value_base_orb_of_draining },
		{ NULL, NULL },
	};
	const struct value_base_s *current = value_bases;

	while (current->name != NULL && current->function != NULL) {
		if (my_stricmp(name, current->name) == 0)
			return current->function;

		current++;
	}

	return NULL;
}
