/* player/player.c - player implementation
 * Copyright (c) 2011 elly+angband@leptoquark.net. See COPYING.
 */

#include "externs.h" /* player_exp */
#include "history.h" /* history_add */
#include "player/player.h"



void health_track(struct player *p, struct monster *m_ptr)
{
	p->health_who = m_ptr;
	p->redraw |= PR_HEALTH;
}

/*
 * Hack -- track the given monster race
 */
void monster_race_track(monster_race *race)
{
	/* Save this monster ID */
	p_ptr->monster_race = race;

	/* Window stuff */
	p_ptr->redraw |= (PR_MONSTER);
}



/*
 * Hack -- track the given object kind
 */
void track_object(int item)
{
	p_ptr->object_idx = item;
	p_ptr->object_kind_idx = NO_OBJECT;
	p_ptr->redraw |= (PR_OBJECT);
}

void track_object_kind(int k_idx)
{
	p_ptr->object_idx = NO_OBJECT;
	p_ptr->object_kind_idx = k_idx;
	p_ptr->redraw |= (PR_OBJECT);
}

bool tracked_object_is(int item)
{
	return (p_ptr->object_idx == item);
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
	
	p->update |= PU_BONUS;
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
		p->update |= (PU_BONUS);
		p->redraw |= (PR_STATS);
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

	p->redraw |= PR_EXP;

	handle_stuff(p);

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

		do_res_stat(A_STR);
		do_res_stat(A_INT);
		do_res_stat(A_WIS);
		do_res_stat(A_DEX);
		do_res_stat(A_CON);
		do_res_stat(A_CHR);
	}

	while ((p->max_lev < PY_MAX_LEVEL) &&
	       (p->max_exp >= (player_exp[p->max_lev-1] *
	                           p->expfact / 100L)))
		p->max_lev++;

	p->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
	p->redraw |= (PR_LEV | PR_TITLE | PR_EXP | PR_STATS);
	handle_stuff(p);
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
