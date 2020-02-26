/**
 * \file player-spell.c
 * \brief Spell and prayer casting/praying
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
#include "init.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"

/**
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
static const int adj_mag_fail[STAT_RANGE] =
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
static const int adj_mag_stat[STAT_RANGE] =
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
 * Make a list of the spell realms the player's class has books from
 */
struct magic_realm *class_magic_realms(const struct player_class *c, int *count)
{
	int i;
	struct magic_realm *r = mem_zalloc(sizeof(struct magic_realm));

	*count = 0;

	if (!c->magic.total_spells) {
		mem_free(r);
		return NULL;
	}

	for (i = 0; i < c->magic.num_books; i++) {
		struct magic_realm *r_test = r;
		struct class_book *book = &c->magic.books[i];
		bool found = false;

		/* Test for first realm */
		if (r->name == NULL) {
			memcpy(r, book->realm, sizeof(struct magic_realm));
			r->next = NULL;
			(*count)++;
			continue;
		}

		/* Test for already recorded */
		while (r_test) {
			if (streq(r_test->name, book->realm->name)) {
				found = true;
			}
			r_test = r_test->next;
		}
		if (found) continue;

		/* Add it */
		r_test = mem_zalloc(sizeof(struct magic_realm));
		r_test->next = r;
		r = r_test;
		(*count)++;
	}

	return r;
}


/**
 * Get the spellbook structure from any object which is a book
 */
const struct class_book *object_kind_to_book(const struct object_kind *kind)
{
	struct player_class *class = classes;
	while (class) {
		int i;

		for (i = 0; i < class->magic.num_books; i++)
		if ((kind->tval == class->magic.books[i].tval) &&
			(kind->sval == class->magic.books[i].sval)) {
			return &class->magic.books[i];
		}
		class = class->next;
	}

	return NULL;
}

/**
 * Get the spellbook structure from an object which is a book the player can
 * cast from
 */
const struct class_book *player_object_to_book(struct player *p,
											   const struct object *obj)
{
	int i;

	for (i = 0; i < p->class->magic.num_books; i++)
		if ((obj->tval == p->class->magic.books[i].tval) &&
			(obj->sval == p->class->magic.books[i].sval))
			return &p->class->magic.books[i];

	return NULL;
}

const struct class_spell *spell_by_index(int index)
{
	int book = 0, count = 0;
	const struct class_magic *magic = &player->class->magic;

	/* Check index validity */
	if (index < 0 || index >= magic->total_spells)
		return NULL;

	/* Find the book, count the spells in previous books */
	while (count + magic->books[book].num_spells - 1 < index)
		count += magic->books[book++].num_spells;

	/* Find the spell */
	return &magic->books[book].spells[index - count];
}

/**
 * Collect spells from a book into the spells[] array, allocating
 * appropriate memory.
 */
int spell_collect_from_book(const struct object *obj, int **spells)
{
	const struct class_book *book = player_object_to_book(player, obj);
	int i, n_spells = 0;

	if (!book) {
		return n_spells;
	}

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
 * Return the number of castable spells in the spellbook 'obj'.
 */
int spell_book_count_spells(const struct object *obj,
		bool (*tester)(int spell))
{
	const struct class_book *book = player_object_to_book(player, obj);
	int i, n_spells = 0;

	if (!book) {
		return n_spells;
	}

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
	bool okay = false;

	for (i = 0; i < n_spells; i++)
		if (spell_test(spells[i]))
			okay = true;

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
bool spell_okay_to_study(int spell_index)
{
	const struct class_spell *spell = spell_by_index(spell_index);
	if (!spell) return false;
	return (spell->slevel <= player->lev) &&
			!(player->spell_flags[spell_index] & PY_SPELL_LEARNED);
}

/**
 * True if the spell is browsable.
 */
bool spell_okay_to_browse(int spell_index)
{
	const struct class_spell *spell = spell_by_index(spell_index);
	if (!spell) return false;
	return (spell->slevel < 99);
}

/**
 * Spell failure adjustment by casting stat level
 */
static int fail_adjust(struct player *p, const struct class_spell *spell)
{
	int stat = spell->realm->stat;
	return adj_mag_stat[p->state.stat_ind[stat]];
}

/**
 * Spell minimum failure by casting stat level
 */
static int min_fail(struct player *p, const struct class_spell *spell)
{
	int stat = spell->realm->stat;
	return adj_mag_fail[p->state.stat_ind[stat]];
}

/**
 * Returns chance of failure for a spell
 */
s16b spell_chance(int spell_index)
{
	int chance = 100, minfail;

	const struct class_spell *spell;

	/* Paranoia -- must be literate */
	if (!player->class->magic.total_spells) return chance;

	/* Get the spell */
	spell = spell_by_index(spell_index);
	if (!spell) return chance;

	/* Extract the base spell failure rate */
	chance = spell->sfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (player->lev - spell->slevel);

	/* Reduce failure rate by casting stat level adjustment */
	chance -= fail_adjust(player, spell);

	/* Not enough mana to cast */
	if (spell->smana > player->csp)
		chance += 5 * (spell->smana - player->csp);

	/* Get the minimum failure rate for the casting stat level */
	minfail = min_fail(player, spell);

	/* Non zero-fail characters never get better than 5 percent */
	if (!player_has(player, PF_ZERO_FAIL) && minfail < 5) {
		minfail = 5;
	}

	/* Necromancers are punished by being on lit squares */
	if (player_has(player, PF_UNLIGHT) && square_islit(cave, player->grid)) {
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
	if (player->timed[TMD_STUN] > 50) {
		chance += 25;
	} else if (player->timed[TMD_STUN]) {
		chance += 15;
	}

	/* Amnesia doubles failure chance */
	if (player->timed[TMD_AMNESIA]) {
		chance = 50 + chance / 2;
	}

	/* Always a 5 percent chance of working */
	if (chance > 95) {
		chance = 95;
	}

	/* Return the chance */
	return (chance);
}


/**
 * Learn the specified spell.
 */
void spell_learn(int spell_index)
{
	int i;
	const struct class_spell *spell = spell_by_index(spell_index);

	/* Learn the spell */
	player->spell_flags[spell_index] |= PY_SPELL_LEARNED;

	/* Find the next open entry in "spell_order[]" */
	for (i = 0; i < player->class->magic.total_spells; i++)
		if (player->spell_order[i] == 99) break;

	/* Add the spell to the known list */
	player->spell_order[i] = spell_index;

	/* Mention the result */
	msgt(MSG_STUDY, "You have learned the %s of %s.", spell->realm->spell_noun,
		 spell->name);

	/* One less spell available */
	player->upkeep->new_spells--;

	/* Message if needed */
	if (player->upkeep->new_spells)
		msg("You can learn %d more %s%s.", player->upkeep->new_spells,
			spell->realm->spell_noun, PLURAL(player->upkeep->new_spells));

	/* Redraw Study Status */
	player->upkeep->redraw |= (PR_STUDY | PR_OBJECT);
}

static int beam_chance(void)
{
	int plev = player->lev;
	return (player_has(player, PF_BEAM) ? plev : (plev / 2));
}

/**
 * Cast the specified spell
 */
bool spell_cast(int spell_index, int dir)
{
	int chance;
	bool *ident = mem_zalloc(sizeof(*ident));
	int beam  = beam_chance();

	/* Get the spell */
	const struct class_spell *spell = spell_by_index(spell_index);

	/* Spell failure chance */
	chance = spell_chance(spell_index);

	/* Fail or succeed */
	if (randint0(100) < chance) {
		event_signal(EVENT_INPUT_FLUSH);
		msg("You failed to concentrate hard enough!");
	} else {
		/* Cast the spell */
		if (!effect_do(spell->effect, source_player(), NULL, ident, true, dir,
					   beam, 0)) {
			mem_free(ident);
			return false;
		}

		/* A spell was cast */
		sound(MSG_SPELL);

		if (!(player->spell_flags[spell_index] & PY_SPELL_WORKED)) {
			int e = spell->sexp;

			/* The spell worked */
			player->spell_flags[spell_index] |= PY_SPELL_WORKED;

			/* Gain experience */
			player_exp_gain(player, e * spell->slevel);

			/* Redraw object recall */
			player->upkeep->redraw |= (PR_OBJECT);
		}
	}

	/* Sufficient mana? */
	if (spell->smana <= player->csp) {
		/* Use some mana */
		player->csp -= spell->smana;
	} else {
		int oops = spell->smana - player->csp;

		/* No mana left */
		player->csp = 0;
		player->csp_frac = 0;

		/* Over-exert the player */
		player_over_exert(player, PY_EXERT_FAINT, 100, 5 * oops + 1);
		player_over_exert(player, PY_EXERT_CON, 50, 0);
	}

	/* Redraw mana */
	player->upkeep->redraw |= (PR_MANA);

	mem_free(ident);
	return true;
}


bool spell_needs_aim(int spell_index)
{
	const struct class_spell *spell = spell_by_index(spell_index);
	assert(spell);
	return effect_aim(spell->effect);
}

static size_t append_random_value_string(char *buffer, size_t size,
										 random_value *rv)
{
	size_t offset = 0;

	if (rv->base > 0) {
		offset += strnfmt(buffer + offset, size - offset, "%d", rv->base);

		if (rv->dice > 0 && rv->sides > 0) {
			offset += strnfmt(buffer + offset, size - offset, "+");
		}
	}

	if (rv->dice == 1 && rv->sides > 0) {
		offset += strnfmt(buffer + offset, size - offset, "d%d", rv->sides);
	} else if (rv->dice > 1 && rv->sides > 0) {
		offset += strnfmt(buffer + offset, size - offset, "%dd%d", rv->dice,
						  rv->sides);
	}

	return offset;
}

static void spell_effect_append_value_info(const struct effect *effect,
										   char *p, size_t len)
{
	random_value rv;
	const char *type = NULL;
	const char *special = NULL;
	size_t offset = strlen(p);

	type = effect_info(effect);

	if (effect->dice != NULL)
		dice_roll(effect->dice, &rv);

	/* Handle some special cases where we want to append some additional info */
	switch (effect->index) {
		case EF_HEAL_HP:
			/* Append percentage only, as the fixed value is always displayed */
			if (rv.m_bonus) special = format("/%d%%", rv.m_bonus);
			break;
		case EF_TELEPORT:
			/* m_bonus means it's a weird random thing */
			if (rv.m_bonus) special = "random";
			break;
		case EF_SPHERE:
			/* Halve damage */
			rv.base /= 2;
			rv.sides /= 2;

			/* Append radius */
			if (effect->radius) {
				int rad = effect->radius;
				if (effect->other) {
					rad += player->lev / effect->other;
				}
				special = format(", rad %d", rad);
			} else {
				special = ", rad 2";
			}
			break;
		case EF_BALL:
			/* Append radius */
			if (effect->radius) {
				int rad = effect->radius;
				if (effect->other) {
					rad += player->lev / effect->other;
				}
				special = format(", rad %d", rad);
			} else {
				special = "rad 2";
			}
			break;
		case EF_STRIKE:
			/* Append radius */
			if (effect->radius) {
				special = format(", rad %d", effect->radius);
			}
			break;
		case EF_SHORT_BEAM: {
			/* Append length of beam */
			int len = effect->radius;
			if (effect->other) {
				len += player->lev / effect->other;
			}
			special = format(", len %d", len);
			break;
		}
		case EF_SWARM:
			/* Append number of projectiles. */
			special = format("x%d", rv.m_bonus);
			break;
		default:
			break;
	}

	if (type == NULL)
		return;

	if (offset) {
		offset += strnfmt(p + offset, len - offset, ";");
	}

	if ((rv.base > 0) || (rv.dice > 0 && rv.sides > 0)) {
		offset += strnfmt(p + offset, len - offset, " %s ", type);
		offset += append_random_value_string(p + offset, len - offset, &rv);

		if (special != NULL)
			strnfmt(p + offset, len - offset, "%s", special);
	}
}

void get_spell_info(int spell_index, char *p, size_t len)
{
	struct effect *effect = spell_by_index(spell_index)->effect;

	p[0] = '\0';

	while (effect) {
		spell_effect_append_value_info(effect, p, len);
		effect = effect->next;
	}
}

static int spell_value_base_spell_power(void)
{
	int power = 0;

	/* Check the reference race first */
	if (ref_race)
	   power = ref_race->spell_power;
	/* Otherwise the current monster if there is one */
	else if (cave->mon_current > 0)
		power = cave_monster(cave, cave->mon_current)->race->spell_power;

	return power;
}

static int spell_value_base_player_level(void)
{
	return player->lev;
}

static int spell_value_base_dungeon_level(void)
{
	return cave->depth;
}

static int spell_value_base_max_sight(void)
{
	return z_info->max_sight;
}

static int spell_value_base_weapon_damage(void)
{
	struct object *obj = player->body.slots[slot_by_name(player, "weapon")].obj;
	if (!obj) {
		return 0;
	}
	return (damroll(obj->dd, obj->ds) + obj->to_d);
}

static int spell_value_base_player_hp(void)
{
	return player->chp;
}

static int spell_value_base_monster_percent_hp_gone(void)
{
	/* Get the targeted monster, fail horribly if none */
	struct monster *mon = target_get_monster();

	return mon ? (((mon->maxhp - mon->hp) * 100) / mon->maxhp) : 0;
}

expression_base_value_f spell_value_base_by_name(const char *name)
{
	static const struct value_base_s {
		const char *name;
		expression_base_value_f function;
	} value_bases[] = {
		{ "SPELL_POWER", spell_value_base_spell_power },
		{ "PLAYER_LEVEL", spell_value_base_player_level },
		{ "DUNGEON_LEVEL", spell_value_base_dungeon_level },
		{ "MAX_SIGHT", spell_value_base_max_sight },
		{ "WEAPON_DAMAGE", spell_value_base_weapon_damage },
		{ "PLAYER_HP", spell_value_base_player_hp },
		{ "MONSTER_PERCENT_HP_GONE", spell_value_base_monster_percent_hp_gone },
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
