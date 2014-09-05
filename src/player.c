/* player/player.c - player implementation
 * Copyright (c) 2011 elly+angband@leptoquark.net. See COPYING.
 */

#include "z-color.h" /* TERM_* */
#include "z-util.h" /* my_strcpy */
#include "init.h"
#include "history.h" /* history_add */
#include "player.h"
#include "player-timed.h"
#include "player-spell.h"
#include "obj-util.h"
#include "birth.h" /* find_roman_suffix_start */
#include "ui-input.h"


/*
 * The player other record (static)
 */
static player_other player_other_body;

/*
 * Pointer to the player other record
 */
player_other *op_ptr = &player_other_body;

/*
 * Pointer to the player info record
 */
player_type *player;

struct player_body *bodies;
struct player_race *races;
struct player_class *classes;

/*
 * Player Sexes
 *
 *	Title,
 *	Winner
 */
const player_sex sex_info[MAX_SEXES] =
{
	{
		"Female",
		"Queen"
	},

	{
		"Male",
		"King"
	},

	{
		"Neuter",
		"Regent"
	}
};

/**
 * Magic realms:
 * index, spell stat, verb, spell noun, book noun
 */
struct magic_realm realms[REALM_MAX] =
{
	{ REALM_NONE, STAT_STR, "", "", "", "" },
	{ REALM_ARCANE, STAT_INT, "cast", "spell", "magic book", "arcane" },
	{ REALM_PIOUS, STAT_WIS, "recite", "prayer", "prayer book", "divine" }
};


/*
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
	#define STAT(a, b, c, d, e, f, g, h) #a,
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

bool player_stat_inc(struct player *p, int stat)
{
	int v = p->stat_cur[stat];

	if (v >= 18 + 100)
		return FALSE;
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
	return TRUE;
}

bool player_stat_dec(struct player *p, int stat, bool permanent)
{
	int cur, max, res = FALSE;

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

	handle_stuff(p->upkeep);

	while ((p->lev > 1) &&
	       (p->exp < (player_exp[p->lev-2] *
	                      p->expfact / 100L)))
		p->lev--;


	while ((p->lev < PY_MAX_LEVEL) &&
	       (p->exp >= (player_exp[p->lev-1] *
	                       p->expfact / 100L)))
	{
		char buf[80];

		p->lev++;

		/* Save the highest level */
		if (p->lev > p->max_lev)
			p->max_lev = p->lev;

		if (verbose)
		{
			/* Log level updates */
			strnfmt(buf, sizeof(buf), "Reached level %d", p->lev);
			history_add(buf, HISTORY_GAIN_LEVEL, 0);

			/* Message */
			msgt(MSG_LEVEL, "Welcome to level %d.",	p->lev);
		}

		effect_simple(EF_RESTORE_STAT, "0", STAT_STR, 1, 0, NULL);
		effect_simple(EF_RESTORE_STAT, "0", STAT_INT, 1, 0, NULL);
		effect_simple(EF_RESTORE_STAT, "0", STAT_WIS, 1, 0, NULL);
		effect_simple(EF_RESTORE_STAT, "0", STAT_DEX, 1, 0, NULL);
		effect_simple(EF_RESTORE_STAT, "0", STAT_CON, 1, 0, NULL);
	}

	while ((p->max_lev < PY_MAX_LEVEL) &&
	       (p->max_exp >= (player_exp[p->max_lev-1] *
	                           p->expfact / 100L)))
		p->max_lev++;

	p->upkeep->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
	p->upkeep->redraw |= (PR_LEV | PR_TITLE | PR_EXP | PR_STATS);
	handle_stuff(p->upkeep);
}

void player_exp_gain(struct player *p, s32b amount)
{
	p->exp += amount;
	if (p->exp < p->max_exp)
		p->max_exp += amount / 10;
	adjust_level(p, TRUE);
}

void player_exp_lose(struct player *p, s32b amount, bool permanent)
{
	if (p->exp < amount)
		amount = p->exp;
	p->exp -= amount;
	if (permanent)
		p->max_exp -= amount;
	adjust_level(p, TRUE);
}

/**
 * Obtain object flags for the player
 */
void player_flags(struct player *p, bitflag f[OF_SIZE])
{
	/* Add racial flags */
	memcpy(f, p->race->flags, sizeof(p->race->flags));

	/* Some classes become immune to fear at a certain plevel */
	if (pf_has(p->class->pflags, PF_BRAVERY_30) && p->lev >= 30)
		of_on(f, OF_PROT_FEAR);
}


byte player_hp_attr(struct player *p)
{
	byte attr;
	
	if (p->chp >= p->mhp)
		attr = TERM_L_GREEN;
	else if (p->chp > (p->mhp * op_ptr->hitpoint_warn) / 10)
		attr = TERM_YELLOW;
	else
		attr = TERM_RED;
	
	return attr;
}

byte player_sp_attr(struct player *p)
{
	byte attr;
	
	if (p->csp >= p->msp)
		attr = TERM_L_GREEN;
	else if (p->csp > (p->msp * op_ptr->hitpoint_warn) / 10)
		attr = TERM_YELLOW;
	else
		attr = TERM_RED;
	
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
 */
const char *player_safe_name(struct player *p, bool strip_suffix)
{
	static char buf[40];
	int i;
	int limit = 0;

	if (op_ptr->full_name[0]) {
		char *suffix = find_roman_suffix_start(op_ptr->full_name);
		if (suffix)
			limit = suffix - op_ptr->full_name - 1; /* -1 for preceding space */
		else
			limit = strlen(op_ptr->full_name);
	}

	for (i = 0; i < limit; i++) {
		char c = op_ptr->full_name[i];

		/* Convert all non-alphanumeric symbols */
		if (!isalpha((unsigned char)c) && !isdigit((unsigned char)c))
			c = '_';

		/* Build "base_name" */
		buf[i] = c;
	}

	/* Terminate */
	buf[i] = '\0';

	/* Require a "base" name */
	if (!buf[0])
		my_strcpy(buf, "PLAYER", sizeof buf);

	return buf;
}


/** Init / cleanup routines **/

static void init_player(void) {
	/* Create the player array, initialised with 0 */
	player = mem_zalloc(sizeof *player);

	/* Allocate player sub-structs */
	player->gear = mem_zalloc(MAX_GEAR * sizeof(object_type));
	player->upkeep = mem_zalloc(sizeof(player_upkeep));
	player->timed = mem_zalloc(TMD_MAX * sizeof(s16b));
}

static void cleanup_player(void) {
	int i;

	player_spells_free(player);

	mem_free(player->timed);
	mem_free(player->upkeep);
	for (i = 0; i < player->max_gear; i++)
		object_wipe(&player->gear[i]);
	mem_free(player->gear);

	mem_free(player);
}

struct init_module player_module = {
	.name = "player",
	.init = init_player,
	.cleanup = cleanup_player
};
