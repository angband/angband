/*
 * File: birth.c
 * Purpose: Character creation
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
#include "cmds.h"
#include "files.h"
#include "game-event.h"
#include "game-cmd.h"
#include "history.h"
#include "object/inventory.h"
#include "object/tvalsval.h"
#include "object/object.h"
#include "squelch.h"
#include "ui-menu.h"

/*
 * Overview
 * ========
 * This file contains the game-mechanical part of the birth process.
 * To follow the code, start at player_birth towards the bottom of
 * the file - that is the only external entry point to the functions
 * defined here.
 *
 * Player (in the Angband sense of character) birth is modelled as a
 * a series of commands from the UI to the game to manipulate the
 * character and corresponding events to inform the UI of the outcomes
 * of these changes.
 *
 * The current aim of this section is that after any birth command
 * is carried out, the character should be left in a playable state.
 * In particular, this means that if a savefile is supplied, the
 * character will be set up according to the "quickstart" rules until
 * another race or class is chosen, or until the stats are reset by
 * the UI.
 *
 * Once the UI signals that the player is happy with the character, the
 * game does housekeeping to ensure the character is ready to start the
 * game (clearing the history log, making sure options are set, etc)
 * before returning control to the game proper.
 */


/* 
 * Maximum amount of starting equipment, and starting gold
 */
#define STARTING_GOLD 600


/*
 * Forward declare
 */
typedef struct birther /*lovely*/ birther; /*sometimes we think she's a dream*/

/*
 * A structure to hold "rolled" information, and any
 * other useful state for the birth process.
 *
 * XXX Demand Obama's birth certificate
 */
struct birther
{
	byte sex;
	const struct player_race *race;
	const struct player_class *class;

	s16b age;
	s16b wt;
	s16b ht;
	s16b sc;

	s32b au;

	s16b stat[A_MAX];

	char *history;
};



/*
 * Save the currently rolled data into the supplied 'player'.
 */
static void save_roller_data(birther *player)
{
	int i;

	/* Save the data */
	player->sex = p_ptr->psex;
	player->race = p_ptr->race;
	player->class = p_ptr->class;
	player->age = p_ptr->age;
	player->wt = p_ptr->wt_birth;
	player->ht = p_ptr->ht_birth;
	player->sc = p_ptr->sc_birth;
	player->au = p_ptr->au_birth;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
		player->stat[i] = p_ptr->stat_birth[i];

	player->history = p_ptr->history;
}


/*
 * Load stored player data from 'player' as the currently rolled data,
 * optionally placing the current data in 'prev_player' (if 'prev_player'
 * is non-NULL).
 *
 * It is perfectly legal to specify the same "birther" for both 'player'
 * and 'prev_player'.
 */
static void load_roller_data(birther *player, birther *prev_player)
{
	int i;

     /* The initialisation is just paranoia - structure assignment is
        (perhaps) not strictly defined to work with uninitialised parts
        of structures. */
	birther temp;
	WIPE(&temp, birther);

	/*** Save the current data if we'll need it later ***/
	if (prev_player) save_roller_data(&temp);

	/*** Load the previous data ***/

	/* Load the data */
	p_ptr->psex = player->sex;
	p_ptr->race = player->race;
	p_ptr->class = player->class;
	p_ptr->age = player->age;
	p_ptr->wt = p_ptr->wt_birth = player->wt;
	p_ptr->ht = p_ptr->ht_birth = player->ht;
	p_ptr->sc = p_ptr->sc_birth = player->sc;
	p_ptr->au_birth = player->au;
	p_ptr->au = STARTING_GOLD;

	/* Load the stats */
	for (i = 0; i < A_MAX; i++)
	{
		p_ptr->stat_max[i] = p_ptr->stat_cur[i] = p_ptr->stat_birth[i] = player->stat[i];
	}

	/* Load the history */
	p_ptr->history = player->history;

	/*** Save the current data if the caller is interested in it. ***/
	if (prev_player) *prev_player = temp;
}


/*
 * Adjust a stat by an amount.
 *
 * This just uses "modify_stat_value()" unless "maximize" mode is false,
 * and a positive bonus is being applied, in which case, a special hack
 * is used.
 */
static int adjust_stat(int value, int amount)
{
	/* Negative amounts or maximize mode */
	if ((amount < 0) || OPT(birth_maximize))
	{
		return (modify_stat_value(value, amount));
	}

	/* Special hack */
	else
	{
		int i;

		/* Apply reward */
		for (i = 0; i < amount; i++)
		{
			if (value < 18)
			{
				value++;
			}
			else if (value < 18+70)
			{
				value += randint1(15) + 5;
			}
			else if (value < 18+90)
			{
				value += randint1(6) + 2;
			}
			else if (value < 18+100)
			{
				value++;
			}
		}
	}

	/* Return the result */
	return (value);
}




/*
 * Roll for a characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(int stat_use[A_MAX])
{
	int i, j;

	int bonus;

	int dice[18];


	/* Roll and verify some stats */
	while (TRUE)
	{
		/* Roll some dice */
		for (j = i = 0; i < 18; i++)
		{
			/* Roll the dice */
			dice[i] = randint1(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/* Verify totals */
		if ((j > 42) && (j < 54)) break;
	}

	/* Roll the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Extract 5 + 1d3 + 1d4 + 1d5 */
		j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

		/* Save that value */
		p_ptr->stat_max[i] = j;

		/* Obtain a "bonus" for "race" and "class" */
		bonus = p_ptr->race->r_adj[i] + p_ptr->class->c_adj[i];

		/* Variable stat maxes */
		if (OPT(birth_maximize))
		{
			/* Start fully healed */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i];

			/* Efficiency -- Apply the racial/class bonuses */
			stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
		}

		/* Fixed stat maxes */
		else
		{
			/* Apply the bonus to the stat (somewhat randomly) */
			stat_use[i] = adjust_stat(p_ptr->stat_max[i], bonus);

			/* Save the resulting stat maximum */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
		}

		p_ptr->stat_birth[i] = p_ptr->stat_max[i];
	}
}


static void roll_hp(void)
{
	int i, j, min_value, max_value;

	/* Minimum hitpoints at highest level */
	min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
	min_value += PY_MAX_LEVEL;

	/* Maximum hitpoints at highest level */
	max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
	max_value += PY_MAX_LEVEL;

	/* Roll out the hitpoints */
	while (TRUE)
	{
		/* Roll the hitpoint values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			j = randint1(p_ptr->hitdie);
			p_ptr->player_hp[i] = p_ptr->player_hp[i-1] + j;
		}

		/* XXX Could also require acceptable "mid-level" hitpoints */

		/* Require "valid" hitpoints at highest level */
		if (p_ptr->player_hp[PY_MAX_LEVEL-1] < min_value) continue;
		if (p_ptr->player_hp[PY_MAX_LEVEL-1] > max_value) continue;

		/* Acceptable */
		break;
	}
}


static void get_bonuses(void)
{
	/* Calculate the bonuses and hitpoints */
	p_ptr->update |= (PU_BONUS | PU_HP);

	/* Update stuff */
	update_stuff(p_ptr);

	/* Fully healed */
	p_ptr->chp = p_ptr->mhp;

	/* Fully rested */
	p_ptr->csp = p_ptr->msp;
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
char *get_history(struct history_chart *chart, s16b *sc)
{
	int roll, social_class;
	struct history_entry *entry;
	char *res = NULL;

	social_class = randint1(4);

	while (chart) {
		roll = randint1(100);
		for (entry = chart->entries; entry; entry = entry->next)
			if (roll <= entry->roll)
				break;
		assert(entry);

		res = string_append(res, entry->text);
		social_class += entry->bonus - 50;
		chart = entry->succ;
	}

	if (social_class > 75)
		social_class = 75;
	else if (social_class < 1)
		social_class = 1;

	if (sc)
		*sc = social_class;
	return res;
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(struct player *p)
{
	/* Calculate the age */
	p->age = p->race->b_age + randint1(p->race->m_age);

	/* Calculate the height/weight for males */
	if (p->psex == SEX_MALE)
	{
		p->ht = p->ht_birth = Rand_normal(p->race->m_b_ht, p->race->m_m_ht);
		p->wt = p->wt_birth = Rand_normal(p->race->m_b_wt, p->race->m_m_wt);
	}

	/* Calculate the height/weight for females */
	else if (p->psex == SEX_FEMALE)
	{
		p->ht = p->ht_birth = Rand_normal(p->race->f_b_ht, p->race->f_m_ht);
		p->wt = p->wt_birth = Rand_normal(p->race->f_b_wt, p->race->f_m_wt);
	}
}




/*
 * Get the player's starting money
 */
static void get_money(void)
{
/*	if (OPT(birth_money))
	{
		p_ptr->au_birth = 200;
		p_ptr->au = 500;
	}
	else
	{                                              */
		p_ptr->au = p_ptr->au_birth = STARTING_GOLD;
}

void player_init(struct player *p)
{
	int i;
	bool keep_randarts = FALSE;

	if (p->inventory)
		mem_free(p->inventory);

	/* Preserve p_ptr->randarts so that players can use loaded randarts even
	 * if they create a completely different character */
	if (p->randarts)
		keep_randarts = TRUE;

	/* Wipe the player */
	(void)WIPE(p, struct player);

	if (keep_randarts)
		p->randarts = TRUE;

	/* Start with no artifacts made yet */
	for (i = 0; z_info && i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		a_ptr->created = FALSE;
		a_ptr->seen = FALSE;
	}


	/* Start with no quests */
	for (i = 0; q_list && i < MAX_Q_IDX; i++)
	{
		q_list[i].level = 0;
	}

	if (q_list) {
		q_list[0].level = 99;
		q_list[1].level = 100;
	}

	for (i = 1; z_info && i < z_info->k_max; i++) {
		object_kind *k_ptr = &k_info[i];
		k_ptr->tried = FALSE;
		k_ptr->aware = FALSE;
	}

	for (i = 1; z_info && i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];
		r_ptr->cur_num = 0;
		r_ptr->max_num = 100;
		if (rf_has(r_ptr->flags, RF_UNIQUE))
			r_ptr->max_num = 1;
		l_ptr->pkills = 0;
	}


	/* Hack -- no ghosts */
	if (z_info)
		r_info[z_info->r_max-1].max_num = 0;


	/* Always start with a well fed player (this is surely in the wrong fn) */
	p->food = PY_FOOD_FULL - 1;


	/* None of the spells have been learned yet */
	for (i = 0; i < PY_MAX_SPELLS; i++)
		p->spell_order[i] = 99;

	p->inventory = C_ZNEW(ALL_INVEN_TOTAL, struct object);

	/* First turn. */
	turn = 1;
	p_ptr->total_energy = 0;
	p_ptr->resting_turn = 0;
	/* XXX default race/class */
	p_ptr->race = races;
	p_ptr->class = classes;
}

/**
 * Try to wield everything wieldable in the inventory.
 */
static void wield_all(struct player *p)
{
	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	int slot;
	int item;
	int num;
	bool is_ammo;

	/* Scan through the slots backwards */
	for (item = INVEN_PACK - 1; item >= 0; item--)
	{
		o_ptr = &p->inventory[item];
		is_ammo = obj_is_ammo(o_ptr);

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Make sure we can wield it */
		slot = wield_slot(o_ptr);
		if (slot < INVEN_WIELD) continue;

		i_ptr = &p->inventory[slot];
		if (i_ptr->kind && (!is_ammo ||
				(is_ammo && !object_similar(o_ptr, i_ptr, OSTACK_PACK))))
			continue;

		/* Figure out how much of the item we'll be wielding */
		num = is_ammo ? o_ptr->number : 1;

		/* Get local object */
		i_ptr = &object_type_body;
		object_copy(i_ptr, o_ptr);

		/* Modify quantity */
		i_ptr->number = num;

		/* Decrease the item (from the pack) */
		inven_item_increase(item, -num);
		inven_item_optimize(item);

		/* Get the wield slot */
		o_ptr = &p->inventory[slot];

		/* Wear the new stuff */
		object_copy(o_ptr, i_ptr);

		/* Increase the weight */
		p->total_weight += i_ptr->weight * i_ptr->number;

		/* Increment the equip counter by hand */
		p->equip_cnt++;
	}

	save_quiver_size(p);

	return;
}


/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(struct player *p)
{
	const struct start_item *si;
	object_type object_type_body;

	/* Give the player starting equipment */
	for (si = p_ptr->class->start_items; si; si = si->next)
	{
		/* Get local object */
		struct object *i_ptr = &object_type_body;

		/* Prepare the item */
		object_prep(i_ptr, si->kind, 0, MINIMISE);
		i_ptr->number = (byte)rand_range(si->min, si->max);
		i_ptr->origin = ORIGIN_BIRTH;

		object_flavor_aware(i_ptr);
		object_notice_everything(i_ptr);

		inven_carry(p, i_ptr);
		si->kind->everseen = TRUE;

		/* Deduct the cost of the item from starting cash */
		p->au -= object_value(i_ptr, i_ptr->number, FALSE);
	}

	/* Sanity check */
	if (p->au < 0)
		p->au = 0;

	/* Now try wielding everything */
	wield_all(p);
}


/*
 * Cost of each "point" of a stat.
 */
static const int birth_stat_costs[18 + 1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 4 };

/* It was feasible to get base 17 in 3 stats with the autoroller */
#define MAX_BIRTH_POINTS 24 /* 3 * (1+1+1+1+1+1+2) */

static void recalculate_stats(int *stats, int points_left)
{
	int i;

	/* Process stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Variable stat maxes */
		if (OPT(birth_maximize))
		{
			/* Reset stats */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] =
				p_ptr->stat_birth[i] = stats[i];
		}

		/* Fixed stat maxes */
		else
		{
			/* Obtain a "bonus" for "race" and "class" */
			int bonus = p_ptr->race->r_adj[i] + p_ptr->class->c_adj[i];

			/* Apply the racial/class bonuses */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = 
				p_ptr->stat_birth[i] =
				modify_stat_value(stats[i], bonus);
		}
	}

	/* Gold is inversely proportional to cost */
	p_ptr->au_birth = STARTING_GOLD + (50 * points_left);

	/* Update bonuses, hp, etc. */
	get_bonuses();

	/* Tell the UI about all this stuff that's changed. */
	event_signal(EVENT_GOLD);
	event_signal(EVENT_AC);
	event_signal(EVENT_HP);
	event_signal(EVENT_STATS);
}

static void reset_stats(int stats[A_MAX], int points_spent[A_MAX], int *points_left)
{
	int i;

	/* Calculate and signal initial stats and points totals. */
	*points_left = MAX_BIRTH_POINTS;

	for (i = 0; i < A_MAX; i++)
	{
		/* Initial stats are all 10 and costs are zero */
		stats[i] = 10;
		points_spent[i] = 0;
	}

	/* Use the new "birth stat" values to work out the "other"
	   stat values (i.e. after modifiers) and tell the UI things have 
	   changed. */
	recalculate_stats(stats, *points_left);
	event_signal_birthpoints(points_spent, *points_left);	
}

static bool buy_stat(int choice, int stats[A_MAX], int points_spent[A_MAX], int *points_left)
{
	/* Must be a valid stat, and have a "base" of below 18 to be adjusted */
	if (!(choice >= A_MAX || choice < 0) &&	(stats[choice] < 18))
	{
		/* Get the cost of buying the extra point (beyond what
		   it has already cost to get this far). */
		int stat_cost = birth_stat_costs[stats[choice] + 1];

		if (stat_cost <= *points_left)
		{
			stats[choice]++;
			points_spent[choice] += stat_cost;
			*points_left -= stat_cost;

			/* Tell the UI the new points situation. */
			event_signal_birthpoints(points_spent, *points_left);

			/* Recalculate everything that's changed because
			   the stat has changed, and inform the UI. */
			recalculate_stats(stats, *points_left);

			return TRUE;
		}
	}

	/* Didn't adjust stat. */
	return FALSE;
}


static bool sell_stat(int choice, int stats[A_MAX], int points_spent[A_MAX],
					  int *points_left)
{
	/* Must be a valid stat, and we can't "sell" stats below the base of 10. */
	if (!(choice >= A_MAX || choice < 0) && (stats[choice] > 10))
	{
		int stat_cost = birth_stat_costs[stats[choice]];

		stats[choice]--;
		points_spent[choice] -= stat_cost;
		*points_left += stat_cost;

		/* Tell the UI the new points situation. */
		event_signal_birthpoints(points_spent, *points_left);

		/* Recalculate everything that's changed because
		   the stat has changed, and inform the UI. */
		recalculate_stats(stats, *points_left);

		return TRUE;
	}

	/* Didn't adjust stat. */
	return FALSE;
}


/*
 * This picks some reasonable starting values for stats based on the
 * current race/class combo, etc.  For now I'm disregarding concerns
 * about role-playing, etc, and using the simple outline from
 * http://angband.oook.cz/forum/showpost.php?p=17588&postcount=6:
 *
 * 0. buy base STR 17
 * 1. if possible buy adj DEX of 18/10
 * 2. spend up to half remaining points on each of spell-stat and con, 
 *    but only up to max base of 16 unless a pure class 
 *    [mage or priest or warrior]
 * 3. If there are any points left, spend as much as possible in order 
 *    on DEX, non-spell-stat, CHR. 
 */
static void generate_stats(int stats[A_MAX], int points_spent[A_MAX], 
						   int *points_left)
{
	int step = 0;
	int maxed[A_MAX] = { 0 };
	bool pure = FALSE;

	/* Determine whether the class is "pure" */
	if (p_ptr->class->spell_stat == 0 || p_ptr->class-> max_attacks < 5)
	{
		pure = TRUE;
	}

	while (*points_left && step >= 0)
	{
		switch (step)
		{
			/* Buy base STR 17 */
			case 0:				
			{
				if (!maxed[A_STR] && stats[A_STR] < 17)
				{
					if (!buy_stat(A_STR, stats, points_spent, points_left))
						maxed[A_STR] = TRUE;
				}
				else
				{
					step++;
				}

				break;
			}

			/* Try and buy adj DEX of 18/10 */
			case 1:
			{
				if (!maxed[A_DEX] && p_ptr->state.stat_top[A_DEX] < 18+10)
				{
					if (!buy_stat(A_DEX, stats, points_spent, points_left))
						maxed[A_DEX] = TRUE;
				}
				else
				{
					step++;
				}

				break;
			}

			/* If we can't get 18/10 dex, sell it back. */
			case 2:
			{
				if (p_ptr->state.stat_top[A_DEX] < 18+10)
				{
					while (stats[A_DEX] > 10)
						sell_stat(A_DEX, stats, points_spent, points_left);

					maxed[A_DEX] = FALSE;
				}
				
				step++;
			}

			/* 
			 * Spend up to half remaining points on each of spell-stat and 
			 * con, but only up to max base of 16 unless a pure class 
			 * [mage or priest or warrior]
			 */
			case 3:
			{
				int points_trigger = *points_left / 2;

				if (p_ptr->class->spell_stat)
				{
					while (!maxed[p_ptr->class->spell_stat] &&
						   (pure || stats[p_ptr->class->spell_stat] < 16) &&
						   points_spent[p_ptr->class->spell_stat] < points_trigger)
					{						
						if (!buy_stat(p_ptr->class->spell_stat, stats, points_spent,
									  points_left))
						{
							maxed[p_ptr->class->spell_stat] = TRUE;
						}

						if (points_spent[p_ptr->class->spell_stat] > points_trigger)
						{
							sell_stat(p_ptr->class->spell_stat, stats, points_spent, 
									  points_left);
							maxed[p_ptr->class->spell_stat] = TRUE;
						}
					}
				}

				while (!maxed[A_CON] &&
					   (pure || stats[A_CON] < 16) &&
					   points_spent[A_CON] < points_trigger)
				{						
					if (!buy_stat(A_CON, stats, points_spent,points_left))
					{
						maxed[A_CON] = TRUE;
					}
					
					if (points_spent[A_CON] > points_trigger)
					{
						sell_stat(A_CON, stats, points_spent, points_left);
						maxed[A_CON] = TRUE;
					}
				}
				
				step++;
				break;
			}

			/* 
			 * If there are any points left, spend as much as possible in 
			 * order on DEX, non-spell-stat, CHR. 
			 */
			case 4:
			{				
				int next_stat;

				if (!maxed[A_DEX])
				{
					next_stat = A_DEX;
				}
				else if (!maxed[A_INT] && p_ptr->class->spell_stat != A_INT)
				{
					next_stat = A_INT;
				}
				else if (!maxed[A_WIS] && p_ptr->class->spell_stat != A_WIS)
				{
					next_stat = A_WIS;
				}
				else if (!maxed[A_CHR])
				{
					next_stat = A_CHR;
				}
				else
				{
					step++;
					break;
				}

				/* Buy until we can't buy any more. */
				while (buy_stat(next_stat, stats, points_spent, points_left));
				maxed[next_stat] = TRUE;

				break;
			}

			default:
			{
				step = -1;
				break;
			}
		}
	}
}

/*
 * This fleshes out a full player based on the choices currently made,
 * and so is called whenever things like race or class are chosen.
 */
void player_generate(struct player *p, const player_sex *s,
		const struct player_race *r, const struct player_class *c)
{
	if (!s) s = &sex_info[p->psex];
	if (!c)
		c = p->class;
	if (!r)
		r = p->race;

	p->sex = s;
	p->class = c;
	p->race = r;

	/* Level 1 */
	p->max_lev = p->lev = 1;

	/* Experience factor */
	p->expfact = p->race->r_exp + p->class->c_exp;

	/* Hitdice */
	p->hitdie = p->race->r_mhp + p->class->c_mhp;

	/* Initial hitpoints */
	p->mhp = p->hitdie;

	/* Pre-calculate level 1 hitdice */
	p->player_hp[0] = p->hitdie;

	/* Roll for age/height/weight */
	get_ahw(p);

	p->history = get_history(p->race->history, &p->sc);
	p->sc_birth = p->sc;
}


/* Reset everything back to how it would be on loading the game. */
static void do_birth_reset(bool use_quickstart, birther *quickstart_prev)
{

	/* If there's quickstart data, we use it to set default
	   character choices. */
	if (use_quickstart && quickstart_prev)
		load_roller_data(quickstart_prev, NULL);

	player_generate(p_ptr, NULL, NULL, NULL);

	/* Update stats with bonuses, etc. */
	get_bonuses();
}


/*
 * Create a new character.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
void player_birth(bool quickstart_allowed)
{
	int i;
	game_command blank = { CMD_NULL, 0, {{0}} };
	game_command *cmd = &blank;

	int stats[A_MAX];
	int points_spent[A_MAX];
	int points_left;
	char *buf;
	int success;

	bool rolled_stats = FALSE;

	/*
	 * The last character displayed, to allow the user to flick between two.
	 * We rely on prev.age being zero to determine whether there is a stored
	 * character or not, so initialise it here.
	 */
	birther prev = { 0 };

	/*
	 * If quickstart is allowed, we store the old character in this,
	 * to allow for it to be reloaded if we step back that far in the
	 * birth process.
	 */
	birther quickstart_prev = { 0 };

	/*
	 * If there's a quickstart character, store it for later use.
	 * If not, default to whatever the first of the choices is.
	 */
	if (quickstart_allowed)
		save_roller_data(&quickstart_prev);
	else
	{
		p_ptr->psex = 0;
		/* XXX default race/class */
		p_ptr->class = classes;
		p_ptr->race = races;
		player_generate(p_ptr, NULL, NULL, NULL);
	}

	/* Handle incrementing name suffix */
	buf = find_roman_suffix_start(op_ptr->full_name);

	if (buf)
	{
		/* Try to increment the roman suffix */
		success = int_to_roman((roman_to_int(buf) + 1), buf,
			(sizeof(op_ptr->full_name) - (buf -
			(char *)&op_ptr->full_name)));
			
		if (!success) msg("Sorry, could not deal with suffix");
	}
	

	/* We're ready to start the interactive birth process. */
	event_signal_flag(EVENT_ENTER_BIRTH, quickstart_allowed);

	/* 
	 * Loop around until the UI tells us we have an acceptable character.
	 * Note that it is possible to quit from inside this loop.
	 */
	while (cmd->command != CMD_ACCEPT_CHARACTER)
	{
		/* Grab a command from the queue - we're happy to wait for it. */
		if (cmd_get(CMD_BIRTH, &cmd, TRUE) != 0) continue;

		if (cmd->command == CMD_BIRTH_RESET)
		{
			player_init(p_ptr);
			reset_stats(stats, points_spent, &points_left);
			do_birth_reset(quickstart_allowed, &quickstart_prev);
			rolled_stats = FALSE;
		}
		else if (cmd->command == CMD_CHOOSE_SEX)
		{
			p_ptr->psex = cmd->arg[0].choice; 
			player_generate(p_ptr, NULL, NULL, NULL);
		}
		else if (cmd->command == CMD_CHOOSE_RACE)
		{
			player_generate(p_ptr, NULL, player_id2race(cmd->arg[0].choice), NULL);

			reset_stats(stats, points_spent, &points_left);
			generate_stats(stats, points_spent, &points_left);
			rolled_stats = FALSE;
		}
		else if (cmd->command == CMD_CHOOSE_CLASS)
		{
			player_generate(p_ptr, NULL, NULL, player_id2class(cmd->arg[0].choice));

			reset_stats(stats, points_spent, &points_left);
			generate_stats(stats, points_spent, &points_left);
			rolled_stats = FALSE;
		}
		else if (cmd->command == CMD_FINALIZE_OPTIONS)
		{
			/* Reset score options from cheat options */
			for (i = OPT_CHEAT; i < OPT_CHEAT + N_OPTS_CHEAT; i++)
			{
				op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] =
					op_ptr->opt[i];
			}
		}
		else if (cmd->command == CMD_BUY_STAT)
		{
			/* .choice is the stat to buy */
			if (!rolled_stats)
				buy_stat(cmd->arg[0].choice, stats, points_spent, &points_left);
		}
		else if (cmd->command == CMD_SELL_STAT)
		{
			/* .choice is the stat to sell */
			if (!rolled_stats)
				sell_stat(cmd->arg[0].choice, stats, points_spent, &points_left);
		}
		else if (cmd->command == CMD_RESET_STATS)
		{
			/* .choice is whether to regen stats */
			reset_stats(stats, points_spent, &points_left);

			if (cmd->arg[0].choice)
				generate_stats(stats, points_spent, &points_left);

			rolled_stats = FALSE;
		}
		else if (cmd->command == CMD_ROLL_STATS)
		{
			int i;

			save_roller_data(&prev);

			/* Get a new character */
			get_stats(stats);

			/* Update stats with bonuses, etc. */
			get_bonuses();

			/* There's no real need to do this here, but it's tradition. */
			get_ahw(p_ptr);
			p_ptr->history = get_history(p_ptr->race->history, &p_ptr->sc);
			p_ptr->sc_birth = p_ptr->sc;

			event_signal(EVENT_GOLD);
			event_signal(EVENT_AC);
			event_signal(EVENT_HP);
			event_signal(EVENT_STATS);

			/* Give the UI some dummy info about the points situation. */
			points_left = 0;
			for (i = 0; i < A_MAX; i++)
			{
				points_spent[i] = 0;
			}

			event_signal_birthpoints(points_spent, points_left);

			/* Lock out buying and selling of stats based on rolled stats. */
			rolled_stats = TRUE;
		}
		else if (cmd->command == CMD_PREV_STATS)
		{
			/* Only switch to the stored "previous"
			   character if we've actually got one to load. */
			if (prev.age)
			{
				load_roller_data(&prev, &prev);
				get_bonuses();
			}

			event_signal(EVENT_GOLD);
			event_signal(EVENT_AC);
			event_signal(EVENT_HP);
			event_signal(EVENT_STATS);
		}
		else if (cmd->command == CMD_NAME_CHOICE)
		{
			/* Set player name */
			my_strcpy(op_ptr->full_name, cmd->arg[0].string,
					  sizeof(op_ptr->full_name));

			string_free((void *) cmd->arg[0].string);

			/* Don't change savefile name.  If the UI
			   wants it changed, they can do it. XXX (Good idea?) */
			process_player_name(FALSE);
		}
		/* Various not-specific-to-birth commands. */
		else if (cmd->command == CMD_HELP)
		{
			char buf[80];

			strnfmt(buf, sizeof(buf), "birth.txt");
			screen_save();
			show_file(buf, NULL, 0, 0);
			screen_load();
		}
		else if (cmd->command == CMD_QUIT)
		{
			quit(NULL);
		}
	}

	roll_hp();

	squelch_birth_init();

	/* Clear old messages, add new starting message */
	history_clear();
	history_add("Began the quest to destroy Morgoth.", HISTORY_PLAYER_BIRTH, 0);

	/* Reset message prompt (i.e. no extraneous -more-s) */
	msg_flag = TRUE;

	/* Note player birth in the message recall */
	message_add(" ", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add("====================", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add(" ", MSG_GENERIC);

	/* Give the player some money */
	get_money();

	/* Outfit the player, if they can sell the stuff */
	if (!OPT(birth_no_selling)) player_outfit(p_ptr);

	/* Initialise the stores */
	store_reset();

	/* Now we're really done.. */
	event_signal(EVENT_LEAVE_BIRTH);
}
