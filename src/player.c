/**
 * \file player.c
 * \brief Player implementation
 *
 * Copyright (c) 2011 elly+angband@leptoquark.net. See COPYING.
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

#include "effects.h"
#include "init.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-spell.h"
#include "player-timed.h"
#include "z-color.h"
#include "z-util.h"

/**
 * Pointer to the player struct
 */
struct player *player = NULL;

struct player_body *bodies;
struct player_race *races;
struct player_shape *shapes;
struct player_class *classes;
struct player_ability *player_abilities;
struct magic_realm *realms;

/**
 * Base experience levels, may be adjusted up for race and/or class
 */
const s32b player_exp[PY_MAX_LEVEL] =
{
	10,
	25,
	45,
	70,
	100,
	140,
	200,
	280,
	380,
	500,
	650,
	850,
	1100,
	1400,
	1800,
	2300,
	2900,
	3600,
	4400,
	5400,
	6800,
	8400,
	10200,
	12500,
	17500,
	25000,
	35000L,
	50000L,
	75000L,
	100000L,
	150000L,
	200000L,
	275000L,
	350000L,
	450000L,
	550000L,
	700000L,
	850000L,
	1000000L,
	1250000L,
	1500000L,
	1800000L,
	2100000L,
	2400000L,
	2700000L,
	3000000L,
	3500000L,
	4000000L,
	4500000L,
	5000000L
};


static const char *stat_name_list[] = {
	#define STAT(a) #a,
	#include "list-stats.h"
	#undef STAT
	"MAX",
    NULL
};

int stat_name_to_idx(const char *name)
{
    int i;
    for (i = 0; stat_name_list[i]; i++) {
        if (!my_stricmp(name, stat_name_list[i]))
            return i;
    }

    return -1;
}

const char *stat_idx_to_name(int type)
{
    assert(type >= 0);
    assert(type < STAT_MAX);

    return stat_name_list[type];
}

const struct magic_realm *lookup_realm(const char *name)
{
	struct magic_realm *realm = realms;
	while (realm) {
		if (!my_stricmp(name, realm->name)) {
			return realm;
		}
		realm = realm->next;
	}

	/* Fail horribly */
	quit_fmt("Failed to find %s magic realm", name);
	return realm;
}

bool player_stat_inc(struct player *p, int stat)
{
	int v = p->stat_cur[stat];

	if (v >= 18 + 100)
		return false;
	if (v < 18) {
		p->stat_cur[stat]++;
	} else if (v < 18 + 90) {
		int gain = (((18 + 100) - v) / 2 + 3) / 2;
		if (gain < 1)
			gain = 1;
		p->stat_cur[stat] += randint1(gain) + gain / 2;
		if (p->stat_cur[stat] > 18 + 99)
			p->stat_cur[stat] = 18 + 99;
	} else {
		p->stat_cur[stat] = 18 + 100;
	}

	if (p->stat_cur[stat] > p->stat_max[stat])
		p->stat_max[stat] = p->stat_cur[stat];
	
	p->upkeep->update |= PU_BONUS;
	return true;
}

bool player_stat_dec(struct player *p, int stat, bool permanent)
{
	int cur, max, res = false;

	cur = p->stat_cur[stat];
	max = p->stat_max[stat];

	if (cur > 18+10)
		cur -= 10;
	else if (cur > 18)
		cur = 18;
	else if (cur > 3)
		cur -= 1;

	res = (cur != p->stat_cur[stat]);

	if (permanent) {
		if (max > 18+10)
			max -= 10;
		else if (max > 18)
			max = 18;
		else if (max > 3)
			max -= 1;

		res = (max != p->stat_max[stat]);
	}

	if (res) {
		p->stat_cur[stat] = cur;
		p->stat_max[stat] = max;
		p->upkeep->update |= (PU_BONUS);
		p->upkeep->redraw |= (PR_STATS);
	}

	return res;
}

static void adjust_level(struct player *p, bool verbose)
{
	if (p->exp < 0)
		p->exp = 0;

	if (p->max_exp < 0)
		p->max_exp = 0;

	if (p->exp > PY_MAX_EXP)
		p->exp = PY_MAX_EXP;

	if (p->max_exp > PY_MAX_EXP)
		p->max_exp = PY_MAX_EXP;

	if (p->exp > p->max_exp)
		p->max_exp = p->exp;

	p->upkeep->redraw |= PR_EXP;

	handle_stuff(p);

	while ((p->lev > 1) &&
	       (p->exp < (player_exp[p->lev-2] * p->expfact / 100L)))
		p->lev--;


	while ((p->lev < PY_MAX_LEVEL) &&
	       (p->exp >= (player_exp[p->lev-1] * p->expfact / 100L))) {
		char buf[80];

		p->lev++;

		/* Save the highest level */
		if (p->lev > p->max_lev)
			p->max_lev = p->lev;

		if (verbose) {
			/* Log level updates */
			strnfmt(buf, sizeof(buf), "Reached level %d", p->lev);
			history_add(p, buf, HIST_GAIN_LEVEL);

			/* Message */
			msgt(MSG_LEVEL, "Welcome to level %d.",	p->lev);
		}

		effect_simple(EF_RESTORE_STAT, source_none(), "0", STAT_STR, 0, 0, 0, 0, NULL);
		effect_simple(EF_RESTORE_STAT, source_none(), "0", STAT_INT, 0, 0, 0, 0, NULL);
		effect_simple(EF_RESTORE_STAT, source_none(), "0", STAT_WIS, 0, 0, 0, 0, NULL);
		effect_simple(EF_RESTORE_STAT, source_none(), "0", STAT_DEX, 0, 0, 0, 0, NULL);
		effect_simple(EF_RESTORE_STAT, source_none(), "0", STAT_CON, 0, 0, 0, 0, NULL);
	}

	while ((p->max_lev < PY_MAX_LEVEL) &&
	       (p->max_exp >= (player_exp[p->max_lev-1] * p->expfact / 100L)))
		p->max_lev++;

	p->upkeep->update |= (PU_BONUS | PU_HP | PU_SPELLS);
	p->upkeep->redraw |= (PR_LEV | PR_TITLE | PR_EXP | PR_STATS);
	handle_stuff(p);
}

void player_exp_gain(struct player *p, s32b amount)
{
	p->exp += amount;
	if (p->exp < p->max_exp)
		p->max_exp += amount / 10;
	adjust_level(p, true);
}

void player_exp_lose(struct player *p, s32b amount, bool permanent)
{
	if (p->exp < amount)
		amount = p->exp;
	p->exp -= amount;
	if (permanent)
		p->max_exp -= amount;
	adjust_level(p, true);
}

/**
 * Obtain object flags for the player
 */
void player_flags(struct player *p, bitflag f[OF_SIZE])
{
	/* Add racial flags */
	memcpy(f, p->race->flags, sizeof(p->race->flags));
	of_union(f, p->class->flags);

	/* Some classes become immune to fear at a certain plevel */
	if (player_has(p, PF_BRAVERY_30) && p->lev >= 30) {
		of_on(f, OF_PROT_FEAR);
	}
}


/**
 * Combine any flags due to timed effects on the player into those in f.
 */
void player_flags_timed(struct player *p, bitflag f[OF_SIZE])
{
	if (p->timed[TMD_BOLD] || p->timed[TMD_HERO] || p->timed[TMD_SHERO]) {
		of_on(f, OF_PROT_FEAR);
	}
	if (p->timed[TMD_TELEPATHY]) {
		of_on(f, OF_TELEPATHY);
	}
	if (p->timed[TMD_SINVIS]) {
		of_on(f, OF_SEE_INVIS);
	}
	if (p->timed[TMD_FREE_ACT]) {
		of_on(f, OF_FREE_ACT);
	}
	if (p->timed[TMD_AFRAID] || p->timed[TMD_TERROR]) {
		of_on(f, OF_AFRAID);
	}
	if (p->timed[TMD_OPP_CONF]) {
		of_on(f, OF_PROT_CONF);
	}
}


byte player_hp_attr(struct player *p)
{
	byte attr;
	
	if (p->chp >= p->mhp)
		attr = COLOUR_L_GREEN;
	else if (p->chp > (p->mhp * p->opts.hitpoint_warn) / 10)
		attr = COLOUR_YELLOW;
	else
		attr = COLOUR_RED;
	
	return attr;
}

byte player_sp_attr(struct player *p)
{
	byte attr;
	
	if (p->csp >= p->msp)
		attr = COLOUR_L_GREEN;
	else if (p->csp > (p->msp * p->opts.hitpoint_warn) / 10)
		attr = COLOUR_YELLOW;
	else
		attr = COLOUR_RED;
	
	return attr;
}

bool player_restore_mana(struct player *p, int amt) {
	int old_csp = p->csp;

	p->csp += amt;
	if (p->csp > p->msp) {
		p->csp = p->msp;
	}
	p->upkeep->redraw |= PR_MANA;

	msg("You feel some of your energies returning.");

	return p->csp != old_csp;
}

/**
 * Return a version of the player's name safe for use in filesystems.
 *
 * XXX This does not belong here.
 */
void player_safe_name(char *safe, size_t safelen, const char *name, bool strip_suffix)
{
	size_t i;
	size_t limit = 0;

	if (name) {
		char *suffix = find_roman_suffix_start(name);

		if (suffix) {
			limit = suffix - name - 1; /* -1 for preceding space */
		} else {
			limit = strlen(name);
		}
	}

	/* Limit to maximum size of safename buffer */
	limit = MIN(limit, safelen);

	for (i = 0; i < limit; i++) {
		char c = name[i];

		/* Convert all non-alphanumeric symbols */
		if (!isalpha((unsigned char)c) && !isdigit((unsigned char)c))
			c = '_';

		/* Build "base_name" */
		safe[i] = c;
	}

	/* Terminate */
	safe[i] = '\0';

	/* Require a "base" name */
	if (!safe[0])
		my_strcpy(safe, "PLAYER", safelen);
}


/**
 * Release resources allocated for fields in the player structure.
 */
void player_cleanup_members(struct player *p)
{
	/* Free the history */
	history_clear(p);

	/* Free the things that are always initialised */
	if (p->obj_k) {
		object_free(p->obj_k);
	}
	mem_free(p->timed);
	if (p->upkeep) {
		mem_free(p->upkeep->quiver);
		mem_free(p->upkeep->inven);
		mem_free(p->upkeep);
		p->upkeep = NULL;
	}

	/* Free the things that are only sometimes initialised */
	if (p->quests) {
		player_quests_free(p);
	}
	if (p->spell_flags) {
		player_spells_free(p);
	}
	if (p->gear) {
		object_pile_free(p->gear);
		object_pile_free(p->gear_k);
	}
	if (p->body.slots) {
		for (int i = 0; i < p->body.count; i++)
			string_free(p->body.slots[i].name);
		mem_free(p->body.slots);
	}
	string_free(p->body.name);
	string_free(p->history);
	if (p->cave) {
		cave_free(p->cave);
		p->cave = NULL;
	}
}


/**
 * Initialise player struct
 */
static void init_player(void) {
	/* Create the player array, initialised with 0 */
	player = mem_zalloc(sizeof *player);

	/* Allocate player sub-structs */
	player->upkeep = mem_zalloc(sizeof(struct player_upkeep));
	player->upkeep->inven = mem_zalloc((z_info->pack_size + 1) * sizeof(struct object *));
	player->upkeep->quiver = mem_zalloc(z_info->quiver_size * sizeof(struct object *));
	player->timed = mem_zalloc(TMD_MAX * sizeof(s16b));
	player->obj_k = object_new();
	player->obj_k->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	player->obj_k->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	player->obj_k->curses = mem_zalloc(z_info->curse_max *
									   sizeof(struct curse_data));

	options_init_defaults(&player->opts);
}

/**
 * Free player struct
 */
static void cleanup_player(void) {
	if (!player) return;

	player_cleanup_members(player);

	/* Free the basic player struct */
	mem_free(player);
	player = NULL;
}

struct init_module player_module = {
	.name = "player",
	.init = init_player,
	.cleanup = cleanup_player
};
