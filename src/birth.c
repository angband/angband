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
#include "game-event.h"
#include "game-cmd.h"
#include "ui-menu.h"

/*
 * Overview
 * ========
 * This file contains the game-mechanical part of the birth process.
 * To follow the code, start at player_birth towards the bottom of
 * the file - that is the only external entry point to the functions
 * defined here.
 * 
 * Player (in the Angband sense of character) birth is a series of 
 * steps which must be carried out in a specified order, with choices
 * in one step affecting those further along (e.g. race and class 
 * choices determine maximum values for stats in the autoroller).
 *
 * We implement this through a system of "birth stages".  As the 
 * game enters each birth stage, we typically do some minor
 * housekeeping before sending a signal to the UI so that it can update 
 * its display, or do whatever else it deems fit.  Then we send other
 * messages (such as updates of the progress of the autoroller) or
 * request a command (such as buying a stat, or stepping back in the
 * process).  This file simply responds to such commands by stepping
 * back and forth through the birth sequence, updating values, and so on,
 * until a suitable character has been chosen.  It then does more
 * housekeeping to ensure the "player" is ready to start the game 
 * (clearing the history log, making sure options are set, etc) before 
 * returning contrl to the game proper.
 */


/*
 * Forward declare
 */
typedef struct birther /*lovely*/ birther; /*sometimes we think she's a dream*/

/*
 * A structure to hold "rolled" information, and any
 * other useful state for the birth process.
 */
struct birther
{
	byte sex;
	byte race;
	byte class;

	s16b age;
	s16b wt;
	s16b ht;
	s16b sc;

	s32b au;

	s16b stat[A_MAX];

	char history[250];
};



/*
 * Save the currently rolled data into the supplied 'player'.
 */
static void save_roller_data(birther *player)
{
	int i;

	/* Save the data */
	player->sex = p_ptr->psex;
	player->race = p_ptr->prace;
	player->class = p_ptr->pclass;
	player->age = p_ptr->age;
	player->wt = p_ptr->wt;
	player->ht = p_ptr->ht;
	player->sc = p_ptr->sc;
	player->au = p_ptr->au;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
	{
		player->stat[i] = p_ptr->stat_max[i];
	}

	/* Save the history */
	my_strcpy(player->history, p_ptr->history, sizeof(player->history));
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
	birther temp = { 0 };

	/*** Save the current data if we'll need it later ***/
	if (prev_player)
		save_roller_data(&temp);

	/*** Load the previous data ***/

	/* Load the data */
	p_ptr->psex = player->sex;
	p_ptr->prace = player->race;
	p_ptr->pclass = player->class;
	p_ptr->age = player->age;
	p_ptr->wt = p_ptr->wt_birth = player->wt;
	p_ptr->ht = p_ptr->ht_birth = player->ht;
	p_ptr->sc = player->sc;
	p_ptr->au = p_ptr->au_birth = player->au;

	/* Load the stats */
	for (i = 0; i < A_MAX; i++)
	{
		p_ptr->stat_max[i] = p_ptr->stat_cur[i] = p_ptr->stat_birth[i] = player->stat[i];
	}

	/* Load the history */
	my_strcpy(p_ptr->history, player->history, sizeof(p_ptr->history));


	/*** Save the current data if the caller is interested in it. ***/
	if (prev_player)
		*prev_player = temp;
}


/*
 * Adjust a stat by an amount.
 *
 * This just uses "modify_stat_value()" unless "maximize" mode is false,
 * and a positive bonus is being applied, in which case, a special hack
 * is used, with the "auto_roll" flag affecting the result.
 *
 * The "auto_roll" flag selects "maximal" changes for use with the
 * auto-roller initialization code.  Otherwise, if "maximize" mode
 * is being used, the changes are fixed.  Otherwise, semi-random
 * changes will occur, with larger changes at lower values.
 */
static int adjust_stat(int value, int amount, int auto_roll)
{
	/* Negative amounts or maximize mode */
	if ((amount < 0) || adult_maximize)
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
				value += ((auto_roll ? 15 : randint(15)) + 5);
			}
			else if (value < 18+90)
			{
				value += ((auto_roll ? 6 : randint(6)) + 2);
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
			dice[i] = randint(3 + i % 3);

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
		bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

		/* Variable stat maxes */
		if (adult_maximize)
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
			stat_use[i] = adjust_stat(p_ptr->stat_max[i], bonus, FALSE);

			/* Save the resulting stat maximum */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
		}

		p_ptr->stat_birth[i] = p_ptr->stat_max[i];
	}
}


/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra(void)
{
	int i, j, min_value, max_value;

	/* Level one */
	p_ptr->max_lev = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

	/* Initial hitpoints */
	p_ptr->mhp = p_ptr->hitdie;

	/* Minimum hitpoints at highest level */
	min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
	min_value += PY_MAX_LEVEL;

	/* Maximum hitpoints at highest level */
	max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
	max_value += PY_MAX_LEVEL;

	/* Pre-calculate level 1 hitdice */
	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Roll out the hitpoints */
	while (TRUE)
	{
		/* Roll the hitpoint values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			j = randint(p_ptr->hitdie);
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


static void get_bonuses()
{
	/* Calculate the bonuses and hitpoints */
	p_ptr->update |= (PU_BONUS | PU_HP);

	/* Update stuff */
	update_stuff();

	/* Fully healed */
	p_ptr->chp = p_ptr->mhp;

	/* Fully rested */
	p_ptr->csp = p_ptr->msp;
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history(void)
{
	int i, chart, roll, social_class;


	/* Clear the previous history strings */
	p_ptr->history[0] = '\0';


	/* Initial social class */
	social_class = randint(4);

	/* Starting place */
	chart = rp_ptr->hist;


	/* Process the history */
	while (chart)
	{
		/* Start over */
		i = 0;

		/* Roll for nobility */
		roll = randint(100);

		/* Get the proper entry in the table */
		while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

		/* Get the textual history */
		my_strcat(p_ptr->history, (h_text + h_info[i].text), sizeof(p_ptr->history));

		/* Add in the social class */
		social_class += (int)(h_info[i].bonus) - 50;

		/* Enter the next chart */
		chart = h_info[i].next;
	}



	/* Verify social class */
	if (social_class > 100) social_class = 100;
	else if (social_class < 1) social_class = 1;

	/* Save the social class */
	p_ptr->sc = social_class;
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(void)
{
	/* Calculate the age */
	p_ptr->age = rp_ptr->b_age + randint(rp_ptr->m_age);

	/* Calculate the height/weight for males */
	if (p_ptr->psex == SEX_MALE)
	{
		p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->m_b_ht, rp_ptr->m_m_ht);
		p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->m_b_wt, rp_ptr->m_m_wt);
	}

	/* Calculate the height/weight for females */
	else if (p_ptr->psex == SEX_FEMALE)
	{
		p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->f_b_ht, rp_ptr->f_m_ht);
		p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->f_b_wt, rp_ptr->f_m_wt);
	}
}




/*
 * Get the player's starting money
 */
static void get_money(int stat_use[A_MAX])
{
	int i;

	int gold;

	/* Social Class determines starting gold */
	gold = (p_ptr->sc * 6) + randint(100) + 300;

	/* Process the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Mega-Hack -- reduce gold for high stats */
		if (stat_use[i] >= 18+50) gold -= 300;
		else if (stat_use[i] >= 18+20) gold -= 200;
		else if (stat_use[i] > 18) gold -= 150;
		else gold -= (stat_use[i] - 8) * 10;
	}

	/* Minimum 200 gold */
	if (gold < 200) gold = 200;

	/* Save the gold */
	p_ptr->au = p_ptr->au_birth = gold;
}



/*
 * Clear all the global "character" data
 */
static void player_wipe(void)
{
	int i;

	/* Wipe the player */
	(void)WIPE(p_ptr, player_type);

	/* Clear the inventory */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_wipe(&inventory[i]);
	}


	/* Start with no artifacts made yet */
	for (i = 0; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		a_ptr->cur_num = 0;
	}


	/* Start with no quests */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		q_list[i].level = 0;
	}

	/* Add a special quest */
	q_list[0].level = 99;

	/* Add a second quest */
	q_list[1].level = 100;


	/* Reset the "objects" */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Reset "tried" */
		k_ptr->tried = FALSE;

		/* Reset "aware" */
		k_ptr->aware = FALSE;
	}


	/* Reset the "monsters" */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Hack -- Reset the counter */
		r_ptr->cur_num = 0;

		/* Hack -- Reset the max counter */
		r_ptr->max_num = 100;

		/* Hack -- Reset the max counter */
		if (r_ptr->flags[0] & (RF1_UNIQUE)) r_ptr->max_num = 1;

		/* Clear player kills */
		l_ptr->pkills = 0;
	}


	/* Hack -- no ghosts */
	r_info[z_info->r_max-1].max_num = 0;


	/* Hack -- Well fed player */
	p_ptr->food = PY_FOOD_FULL - 1;


	/* None of the spells have been learned yet */
	for (i = 0; i < PY_MAX_SPELLS; i++) p_ptr->spell_order[i] = 99;


	/* First turn. */
	turn = old_turn = 1;
}

/*
 * Try to wield everything wieldable in the inventory.
 */
static void wield_all(void)
{
	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	int slot;
	int item;

	/* Scan through the slots backwards */
	for (item = INVEN_PACK - 1; item >= 0; item--)
	{
		o_ptr = &inventory[item];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Make sure we can wield it and that there's nothing else in that slot */
		slot = wield_slot(o_ptr);
		if (slot < INVEN_WIELD) continue;
		if (inventory[slot].k_idx) continue;

		/* Get local object */
		i_ptr = &object_type_body;
		object_copy(i_ptr, o_ptr);

		/* Modify quantity */
		i_ptr->number = 1;

		/* Decrease the item (from the pack) */
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_optimize(item);
		}

		/* Decrease the item (from the floor) */
		else
		{
			floor_item_increase(0 - item, -1);
			floor_item_optimize(0 - item);
		}

		/* Get the wield slot */
		o_ptr = &inventory[slot];

		/* Wear the new stuff */
		object_copy(o_ptr, i_ptr);

		/* Increase the weight */
		p_ptr->total_weight += i_ptr->weight;

		/* Increment the equip counter by hand */
		p_ptr->equip_cnt++;
	}

	return;
}


/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(void)
{
	int i;
	const start_item *e_ptr;
	object_type *i_ptr;
	object_type object_type_body;


	/* Hack -- Give the player his equipment */
	for (i = 0; i < MAX_START_ITEMS; i++)
	{
		/* Access the item */
		e_ptr = &(cp_ptr->start_items[i]);

		/* Get local object */
		i_ptr = &object_type_body;

		/* Hack	-- Give the player an object */
		if (e_ptr->tval > 0)
		{
			/* Get the object_kind */
			int k_idx = lookup_kind(e_ptr->tval, e_ptr->sval);

			/* Valid item? */
			if (!k_idx) continue;

			/* Prepare the item */
			object_prep(i_ptr, k_idx);
			i_ptr->number = (byte)rand_range(e_ptr->min, e_ptr->max);
			i_ptr->origin = ORIGIN_BIRTH;

			object_aware(i_ptr);
			object_known(i_ptr);
			(void)inven_carry(i_ptr);
			k_info[k_idx].everseen = TRUE;
		}
	}


	/* Hack -- give the player hardcoded equipment XXX */

	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Give the player some food */
	object_prep(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
	i_ptr->number = (byte)rand_range(3, 7);
	i_ptr->origin = ORIGIN_BIRTH;
	object_aware(i_ptr);
	object_known(i_ptr);
	k_info[i_ptr->k_idx].everseen = TRUE;
	(void)inven_carry(i_ptr);


	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Give the player some torches */
	object_prep(i_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
	i_ptr->number = (byte)rand_range(3, 7);
	i_ptr->timeout = FUEL_TORCH / 2;
	i_ptr->origin = ORIGIN_BIRTH;
	object_aware(i_ptr);
	object_known(i_ptr);
        k_info[i_ptr->k_idx].everseen = TRUE;
	(void)inven_carry(i_ptr);


	/* Now try wielding everything */
	wield_all();
}


/*
 * Get a command for the birth process, handling the command cases here 
 * (i.e. quitting and options.
 *
 * NOTE: We would also handle help here if it was eventually decided
 * there should be a game help mode rather than it being entirely at
 * the UI level.
 */
static game_command get_birth_command()
{
	game_command cmd = { CMD_NULL };

	while (cmd.command == CMD_NULL)
	{
		cmd = get_game_command();

		if (cmd.command == CMD_QUIT) 
			quit(NULL);

		if (cmd.command == CMD_OPTIONS) 
		{
			/* TODO: Change this to use whatever sort of message passing
			   system we eventually decide on for options.  That might
			   still be calling do_cmd_option. :) */
			do_cmd_options();

			/* We've already handled it, so don't pass it on. */
			cmd.command = CMD_NULL;
		}
	}

	/* TODO: Check against list of permitted commands for the given stage. Probably. */

	return cmd;
}

/*
 * Cost of each "point" of a stat.
 */
static const int birth_stat_costs[18 + 1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 4 };

/* It is feasible to get base 17 in 3 stats with the autoroller */
#define MAX_BIRTH_POINTS 24 /* 3 * (1+1+1+1+1+1+2) */


static void recalculate_stats(int *stats, int points_left)
{
	int i;

	/* Process stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Variable stat maxes */
		if (adult_maximize)
		{
			/* Reset stats */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = p_ptr->stat_birth[i] = stats[i];
		}
		
		/* Fixed stat maxes */
		else
		{
			/* Obtain a "bonus" for "race" and "class" */
			int bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];
			
			/* Apply the racial/class bonuses */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] =
				modify_stat_value(stats[i], bonus);
		}
	}
	
	/* Gold is inversely proportional to cost */
	p_ptr->au = p_ptr->au_birth = (50 * points_left) + 100;

	/* Update bonuses, hp, etc. */
	get_bonuses();

	/* Tell the UI about all this stuff that's changed. */
	event_signal(EVENT_GOLD);
	event_signal(EVENT_AC);
	event_signal(EVENT_HP);
	event_signal(EVENT_STATS);
}


/*
 * Due to its relative complexity, point based birth has been split off into
 * this function.  'reset' is TRUE if we're entering from earlier in the
 * birth process - i.e. we want to start a character from scratch rather 
 * than having another go at one already chosen.
 */
static enum birth_stage do_point_based(bool reset)
{
	game_command cmd = { CMD_NULL };
	
	int i, j;
	int stats[A_MAX];

	int points_spent[A_MAX];
	int points_left;

	/* If the first command is BIRTH_BACK, we step back a stage, so this
	   is a reasonable default. */
	enum birth_stage next_stage = BIRTH_ROLLER_CHOICE;

	if (reset)
	{
		/* Roll for base hitpoints, age/height/weight and social class */
		get_extra();
		get_ahw();
		get_history();
	}

	/* Signal that we're entering the point based birth arena. */
	event_signal_birthstage(BIRTH_POINTBASED, NULL);
	
	/* Calculate and signal initial stats and points totals. */
	points_left = MAX_BIRTH_POINTS;

	for (i = 0; i < A_MAX; i++)
	{
		/* Initial stats are all 10 and costs or zero */
		stats[i] = 10;
		points_spent[i] = 0;

		/* If not resetting, we use the stored stat values from p_ptr
		   to simulate buying the stats, just using the same method as
		   when the BUY_STAT command is received. */
		if (!reset)
		{
			for (j = 10; j < p_ptr->stat_birth[i]; j++)
			{
				int stat_cost = birth_stat_costs[j + 1];
				stats[i]++;
				points_spent[i] += stat_cost;
				points_left -= stat_cost;
			}
		}
	}

	/* Use the new "birth stat" values to work out the "other"
	   stat values (i.e. after modifiers) and tell the UI things have 
	   changed. */
	recalculate_stats(stats, points_left);
	event_signal_birthstats(points_spent, points_left);	
	
	/* Then on to the interactive part - two ways to leave */
	while (cmd.command != CMD_ACCEPT_STATS && 
		   cmd.command != CMD_BIRTH_BACK)
	{		
		cmd = get_birth_command();
		
		/* If BIRTH_BACK isn't the first command, or we didn't start
		   by resetting the stats, we redo this stage rather than actually
		   stepping back. */
		if (cmd.command != CMD_BIRTH_BACK || !reset)
			next_stage = BIRTH_POINTBASED;

		switch (cmd.command)
		{
			case CMD_BUY_STAT:
			{
				/* The choice is the index of the stat in the list to "buy" */
				int choice = cmd.params.choice;
				if (choice >= A_MAX || choice < 0) continue;

				/* Can't increase stats past a "base" of 18 */
				if (stats[choice] < 18)
				{
					/* Get the cost of buying the extra point (beyond what
					   it has already cost to get this far). */
					int stat_cost = birth_stat_costs[stats[choice] + 1];

					if (stat_cost <= points_left)
					{
						stats[choice]++;
						points_spent[choice] += stat_cost;
						points_left -= stat_cost;
				
						/* Tell the UI the new points situation. */
						event_signal_birthstats(points_spent, points_left);

						/* Recalculate everything that's changed because
						   the stat has changed, and inform the UI. */
						recalculate_stats(stats, points_left);
					}
				}

				break;
			}
			
			case CMD_SELL_STAT:
			{
				/* The choice is the index of the stat in the list to "sell" */
				int choice = cmd.params.choice;
				if (choice >= A_MAX || choice < 0) continue;

				/* We can't "sell" stats below the base of 10. */
				if (stats[choice] > 10)
				{
					int stat_cost = birth_stat_costs[stats[choice]];
					
					stats[choice]--;
					points_spent[choice] -= stat_cost;
					points_left += stat_cost;
					
					/* Tell the UI the new points situation. */
					event_signal_birthstats(points_spent, points_left);

					/* Recalculate everything that's changed because
					   the stat has changed, and inform the UI. */
					recalculate_stats(stats, points_left);
				}				
				break;
			}

			case CMD_ACCEPT_STATS:
			{
				next_stage = BIRTH_NAME_CHOICE;
				break;
			}
		}
	}

	return next_stage;
}
	

enum birth_questions
{
	BQ_METHOD = 0,
	BQ_SEX,
	BQ_RACE,
	BQ_CLASS,
	BQ_ROLLER,
	MAX_BIRTH_QUESTIONS
};

enum birth_methods
{
	BM_NORMAL_BIRTH = 0,
	BM_QUICKSTART,
	MAX_BIRTH_METHODS
};

enum birth_rollers
{
	BR_POINTBASED = 0,
	BR_AUTOROLLER,
	BR_NORMAL,
	MAX_BIRTH_ROLLERS
};

/*
 * Create a new character.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
void player_birth(bool quickstart_allowed)
{
	int i;
	game_command cmd = { CMD_NULL, 0 };


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

	enum birth_stage next_stage = BIRTH_METHOD_CHOICE;
	enum birth_stage stage = BIRTH_METHOD_CHOICE;
	enum birth_stage last_stage = BIRTH_METHOD_CHOICE;
	enum birth_stage roller_choice = BIRTH_METHOD_CHOICE;

	int roller_mins[A_MAX];

	/* Set up our "hints" for each birth question */
	const char *hints[MAX_BIRTH_QUESTIONS] = {
		"Quickstart lets you make a new character based on your old one.",
		"Your 'sex' does not have any significant gameplay effects.",
		"Your 'race' determines various intrinsic factors and bonuses.",
		"Your 'class' determines various intrinsic abilities and bonuses",
		"Your choice of character generation.  Point-based is recommended."
	};

	/* Set up the list of choices for each birth question. */
	const char *quickstart_choices[MAX_BIRTH_METHODS] = { 
		"Normal birth process", 
		"Quickstart" 
	};

	const char *roller_choices[MAX_BIRTH_ROLLERS] = { 
		"Point-based", 
		"Autoroller", 
		"Standard roller" 
	};

	/* These require setting up in a loop later, so just allocate memory for
	   arrays of char * for now */
	const char *sex_choices[MAX_SEXES];

	const char **race_choices = mem_alloc(z_info->p_max * sizeof *race_choices);
	const char **class_choices = mem_alloc(z_info->c_max * sizeof *class_choices);

	/* For a couple of the options we have extra "help text" as well as the
	   hints, */
	const char **race_help = mem_alloc(z_info->p_max * sizeof *race_help);
	const char **class_help = mem_alloc(z_info->c_max * sizeof *class_help);


	/* Set up the choices for sex, race and class questions. */
	for (i = 0; i < MAX_SEXES; i++)
		sex_choices[i] = sex_info[i].title;

	for (i = 0; i < z_info->p_max; i++)
		race_choices[i] = p_name + p_info[i].name;

	for (i = 0; i < z_info->c_max; i++)
		class_choices[i] = c_name + c_info[i].name;

	/* Set up extra help text for race and class questions (basically just
	   lists of various bonuses you get for each race or class). */
	for (i = 0; i < z_info->p_max; i++)
	{
		int j;
		char *s;
		size_t end; 

		/* Race help text consists of: */
		int bufsize = 
			sizeof "STR: xx\n" * A_MAX + 
			sizeof "Hit die: xx\n" +
			sizeof "Experience: xxx%\n" +
			sizeof "Infravision: xx ft\0";

		/* Allocate enough memory to hold that text for this race choice */
		race_help[i] = mem_alloc(bufsize * sizeof *race_help[i]);

		/* Set 's' up as a shorthand for race_choices[i] to make later lines
		   readable, and make sure it is an empty string. Note we cast away
		   the "constness" of rece_help[i] while we build the string,
		   safe because we know where it points (writable memory). */
		s = (char *) race_help[i];
		s[0] = '\0'; end = 0;

		/* For each stat, concatanate a "STR: xx" type sequence to the
		   end of the help string. */
		for (j = 0; j < A_MAX; j++) 
		{  
			strnfcat(s, bufsize, &end, "%s%+d\n", 
					 stat_names_reduced[j], p_info[i].r_adj[j]); 
		}

		/* Concatenate all other bonuses to the end of the help string. */
		strnfcat(s, bufsize, &end, "Hit die: %d\n", p_info[i].r_mhp);
		strnfcat(s, bufsize, &end, "Experience: %d%%\n", p_info[i].r_exp);
		strnfcat(s, bufsize, &end, "Infravision: %d ft", p_info[i].infra * 10);
	}

	for (i = 0; i < z_info->c_max; i++)
	{
		int j;
		char *s;
		size_t end;

		/* Class help text consists of: */
		int bufsize = 
			sizeof "STR: xx\n" * A_MAX + 
			sizeof "Hit die: xx\n" +
			sizeof "Experience: xxx%\n";

		/* Allocate enough memory to hold that text for this race choice */
		class_help[i] = mem_alloc(bufsize * sizeof *class_help[i]);

		/* Set 's' up as a shorthand for race_choices[i] to make later lines
		   readable, and make sure it is an empty string. Note we cast away
		   the "constness" of rece_help[i] while we build the string,
		   safe because we know where it points (writable memory). */
		s = (char *) class_help[i];
		s[0] = '\0'; end = 0;

		/* For each stat, concatanate a "STR: xx" type sequence to the
		   end of the help string. */
		for (j = 0; j < A_MAX; j++) 
		{  
			strnfcat(s, bufsize, &end, "%s%+d\n", 
					 stat_names_reduced[j], c_info[i].c_adj[j]); 
		}

		/* Concatenate all other bonuses to the end of the help string. */
		strnfcat(s, bufsize, &end, "Hit die: %d\n", c_info[i].c_mhp);   
		strnfcat(s, bufsize, &end, "Experience: %d%%", c_info[i].c_exp);
	}

	/* If there's a quickstart character, store it here. */
	if (quickstart_allowed)
		save_roller_data(&quickstart_prev);

	/* Now we're ready to start the interactive birth process. */
	event_signal(EVENT_ENTER_BIRTH);

	/* "stage" just keeps track of where we're up to in the (somewhat tortuous)
	   process - the stages are laid out in approximately the right order in 
	   the switch statement below. */
	stage = BIRTH_METHOD_CHOICE;

	/* There are two ways to leave - with a working character or by quitting */
	while (stage != BIRTH_COMPLETE)
	{
		switch (stage)
		{
			/* 
			 * First stage is to determine the birth method - currently
			 * simply whether to use quickstart or do the full birth 
			 * process.
			 */
			case BIRTH_METHOD_CHOICE:
			{
				/* Set up a default command to use to proceed. */
				cmd.command = CMD_BIRTH_CHOICE;
				cmd.params.choice = BM_NORMAL_BIRTH;

				/* We're at the beginning, so wipe the player clean */
				player_wipe();

				/* If there's a choice to be made, offer it. */
				if (quickstart_allowed)
				{
					/* If there's quickstart data, we use it to set default
					   character choices even if quickstart isn't chosen. */
					load_roller_data(&quickstart_prev, NULL);

					event_signal_birthstage_question(BIRTH_METHOD_CHOICE,
													 hints[BQ_METHOD],
													 MAX_BIRTH_METHODS, 
													 BM_NORMAL_BIRTH,
													 quickstart_choices,
													 NULL);
					cmd = get_birth_command();
				}
				
				/* Either way, move on to the next relevant stage. */
				if (cmd.command == CMD_BIRTH_CHOICE)
				{
					if (cmd.params.choice)
					{
						/* Get the things not loaded with the roller data 
						   (hitdice, exp bonuses) */
						get_extra();

						/* Update bonuses, hp, etc. */
						get_bonuses();

						/* Set the roller choice so we end up back in the
						   right place. */
						roller_choice = BIRTH_METHOD_CHOICE;
						next_stage = BIRTH_NAME_CHOICE;
					}
					else
					{
						next_stage = BIRTH_SEX_CHOICE;
					}
				}		
	
				break;
			}

			/*
			 * Ask the user to choose a sex from the supplied list.
			 */
			case BIRTH_SEX_CHOICE:
			{
				event_signal_birthstage_question(BIRTH_SEX_CHOICE, 
												 hints[BQ_SEX],
												 MAX_SEXES, 
												 p_ptr->psex,
												 sex_choices,
												 NULL);

				cmd = get_birth_command();

				if (cmd.command == CMD_BIRTH_CHOICE)
				{					
					p_ptr->psex = cmd.params.choice; 
					sp_ptr = &sex_info[p_ptr->psex];
					next_stage = BIRTH_RACE_CHOICE;
				}
				/* At this point there's only one backwards step. */
				else if (cmd.command == CMD_BIRTH_RESTART ||
						 cmd.command == CMD_BIRTH_BACK)
				{
					next_stage = BIRTH_METHOD_CHOICE;
				}
				
				break;
			}

			/*
			 * Ask the user to choose a race from the supplied list.
			 */
			case BIRTH_RACE_CHOICE:
			{
				event_signal_birthstage_question(BIRTH_RACE_CHOICE, 
												 hints[BQ_RACE],
												 z_info->p_max, 
												 p_ptr->prace,
												 race_choices,
												 race_help);

				cmd = get_birth_command();

				if (cmd.command == CMD_BIRTH_CHOICE)
				{					
					p_ptr->prace = cmd.params.choice;
					rp_ptr = &p_info[p_ptr->prace];
					next_stage = BIRTH_CLASS_CHOICE;
				}
				else if (cmd.command == CMD_BIRTH_RESTART)
				{
					next_stage = BIRTH_METHOD_CHOICE;
				}
				else if (cmd.command == CMD_BIRTH_BACK)
				{
					next_stage = BIRTH_SEX_CHOICE;
				}

				break;
			}

			/*
			 * Ask the user to choose a class from the supplied list.
			 */
			case BIRTH_CLASS_CHOICE:
			{
				event_signal_birthstage_question(BIRTH_CLASS_CHOICE, 
												 hints[BQ_CLASS],
												 z_info->c_max, 
												 p_ptr->pclass,
												 class_choices,
												 class_help);

				cmd = get_birth_command();

				if (cmd.command == CMD_BIRTH_CHOICE)
				{
					p_ptr->pclass = cmd.params.choice;
					cp_ptr = &c_info[p_ptr->pclass];
					mp_ptr = &cp_ptr->spells;
					next_stage = BIRTH_ROLLER_CHOICE;
				}
				else if (cmd.command == CMD_BIRTH_RESTART)
				{
					next_stage = BIRTH_METHOD_CHOICE;
				}
				else if (cmd.command == CMD_BIRTH_BACK)
				{
					next_stage = BIRTH_RACE_CHOICE;
				}

				break;
			}

			/*
			 * Ask the user to choose a roller from the supplied list.
			 */
			case BIRTH_ROLLER_CHOICE:
			{
				int init_choice;

				/* If we're coming at this stage from further on in 
				   the process then roller_choice will already have
				   been set.  We change the initial choice to match
				   the user's previous choice. */
				if (roller_choice == BIRTH_AUTOROLLER)
					init_choice = BR_AUTOROLLER;
				else if (roller_choice == BIRTH_ROLLER)
					init_choice = BR_NORMAL;
				else
					init_choice = BR_POINTBASED;

				event_signal_birthstage_question(BIRTH_ROLLER_CHOICE, 
												 hints[BQ_ROLLER],
												 MAX_BIRTH_ROLLERS, 
												 init_choice,
												 roller_choices,
												 NULL);

				cmd = get_birth_command();

				if (cmd.command == CMD_BIRTH_CHOICE)
				{
					switch (cmd.params.choice)
					{
						case BR_POINTBASED:
						{
							roller_choice = next_stage = BIRTH_POINTBASED;
							break;
						}

						case BR_AUTOROLLER:
						{
							/* roller_mins are the user-chosen minima for
							   stats rolled up - reset to zero here for
							   safety. */
							for (i = 0; i < A_MAX; i++)
								roller_mins[i] = 0;

							roller_choice = next_stage = BIRTH_AUTOROLLER;
							break;
						}

						case BR_NORMAL:
						{
							/* roller_mins are the user-chosen minima for
							   stats rolled up - reset to zero here because
							   we'll accept any roll in the "normal" roller.*/
							for (i = 0; i < A_MAX; i++)
								roller_mins[i] = 0;

							roller_choice = next_stage = BIRTH_ROLLER;
							break;
						}
					}
				}
				else if (cmd.command == CMD_BIRTH_RESTART)
					next_stage = BIRTH_METHOD_CHOICE;
				else if (cmd.command == CMD_BIRTH_BACK)
					next_stage = BIRTH_CLASS_CHOICE;
				
				break;
			}


			/*
			 * The point-based birth system is sufficiently complicated
			 * to warrant being split off into another function.  This
			 * will return the next stage (either forward or back,
			 * essentially.
			 */
			case BIRTH_POINTBASED:
			{
				/* Go to the point-based roller, but only reset the
				   stats if we're coming from an earlier point in the
				   process. */
				if (last_stage == BIRTH_POINTBASED ||
					last_stage == BIRTH_ROLLER_CHOICE)
				{
					next_stage = do_point_based(TRUE);
				}
				else
				{
					next_stage = do_point_based(FALSE);
				}

				break;
			}

			/* 
			 * The autoroller stage merely allows the user to select
			 * the minimum values for stats - the actual rolling happens
			 * in BIRTH_ROLLER.  As such, all we do here is calculate the
			 * maxima based on the race/class selections, tell the UI about
			 * them, and then wait for the UI to return the minimum values
			 * the player wants (or to go back, or quit, etc). 
			 */
			case BIRTH_AUTOROLLER:
			{
				int maxes[A_MAX];

				/* Calculate maximum possible values for stats */
				for (i = 0; i < A_MAX; i++)
				{
					/* Race/Class bonus */
					int j = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];
		
					/* Obtain the "maximal" stat */
					maxes[i] = adjust_stat(17, j, TRUE);
				}

				event_signal_birthstage(BIRTH_AUTOROLLER, maxes);
				
				cmd = get_birth_command();

				if (cmd.command == CMD_BIRTH_RESTART)
				{
					next_stage = BIRTH_METHOD_CHOICE;
				}
				else if (cmd.command == CMD_BIRTH_BACK)
				{
					next_stage = BIRTH_ROLLER_CHOICE;
				}
				else if (cmd.command == CMD_AUTOROLL)
				{
					for (i = 0; i < A_MAX; i++)
						roller_mins[i] = cmd.params.stat_limits[i];
					
					next_stage = BIRTH_ROLLER;
				}

				break;
			}

			/*
			 * The roller stage is common to both the simple "one roll,
			 * one character" roller and the autoroller.
			 *
			 * We simply provide a rolled character where stats are at
			 * least as large as those in roller_mins (set up before entering
			 * this stage), and allow the user to accept the stats, roll new
			 * ones, or switch back to the previous set.
			 */
			case BIRTH_ROLLER:
			{
				/* Used to give limited stats about the autoroller. */
				int stat_use[A_MAX] = { 0 };
				int stat_match[A_MAX] = { 0 };

				/* Used to enforce a million-roll limit on the autoroller. */
				long unsigned auto_round = 0;

				/* Only do an initial roll if we're coming from earlier
				   in the birth process. */
				if (last_stage == BIRTH_ROLLER_CHOICE || 
					last_stage == BIRTH_AUTOROLLER)
				{
					/* Reset our saved "swap" character marker, too. */
					prev.age = 0;
					cmd.command = CMD_ROLL;
				}
				else
				{
					cmd.command = CMD_NULL;
				}

				event_signal_birthstage(BIRTH_ROLLER, NULL);

				/* Roll until the user wants to move back or accept stats */
				while (next_stage == BIRTH_ROLLER)
				{
					long unsigned last_round = auto_round;

					if (cmd.command == CMD_ROLL)
					{
						/* Roll until we get a character that meets the minimum
						   stats, or we've done a million rolls (in total,
						   across all rolled characters).	*/
						while (TRUE)
						{
							bool accept = TRUE;
							
							/* Get a new character */
							get_stats(stat_use);
							
							/* If we're above the million round limit, just
							   use the rolled character. */
							if (auto_round >= 1000000L) break;
							
							/* Advance the round */
							auto_round++;
							
							/* Check and count acceptable stats */
							for (i = 0; i < A_MAX; i++)
							{
								/* This stat is okay */
								if (stat_use[i] >= roller_mins[i])
								{
									stat_match[i]++;
								}
								/* This stat is not okay */
								else
								{
									accept = FALSE;
								}
							}
							
							/* Break if "happy" */
							if (accept) break;
							
							/* Update display every round for the first
							   100, then every 25 rounds. */
							if ((auto_round < last_round + 100) ||
								(auto_round % 25L) == 0)
							{
								event_signal_birthautoroller(roller_mins,
															 stat_match,
															 stat_use,
															 auto_round);
							}
						}
						
						/* Roll for base hitpoints */
						get_extra();
						
						/* Roll for age/height/weight */
						get_ahw();
						
						/* Roll for social class */
						get_history();
						
						/* Roll for gold */
						get_money(stat_use);

						/* Update bonuses, hp, etc. */
						get_bonuses();
					}
					
					/* Present the character to the UI - prev.age is
					   only != 0 if we've got a previous character */
					event_signal_birthstats(stat_use, prev.age);

					/* Get the user's opinion on this character. */
					cmd = get_birth_command();
					
					switch (cmd.command)
					{
						case CMD_ROLL:
						{
							/* Before rolling, save this character as
							   the "previous" one to allow the user the
							   choice of two. */
							save_roller_data(&prev);
							break;
						}
						
						case CMD_PREV_STATS:
						{
							/* Only switch to the stored "previous"
							   character if we've actually got one to load. */
							if (prev.age)
								load_roller_data(&prev, &prev);
							break;
						}
						
						case CMD_ACCEPT_STATS:
						{
							next_stage = BIRTH_NAME_CHOICE;
							break;
						}

						case CMD_BIRTH_BACK:
						{
							if (roller_choice == BIRTH_AUTOROLLER)
								next_stage = BIRTH_AUTOROLLER;
							else
								next_stage = BIRTH_ROLLER_CHOICE;

							break;
						}
						case CMD_BIRTH_RESTART:
						{
							next_stage = BIRTH_METHOD_CHOICE;
							break;
						}
					}
				}
				
				break;
			}

			/*
			 * Get a character name from the user.
			 */
			case BIRTH_NAME_CHOICE:
			{
				event_signal_birthstage(BIRTH_NAME_CHOICE, NULL);
				cmd = get_birth_command();

				if (cmd.command == CMD_NAME_CHOICE)
				{
					/* Set player name */
					my_strcpy(op_ptr->full_name, cmd.params.string, 
							  sizeof(op_ptr->full_name));

					string_free((void *) cmd.params.string);
					
					/* Don't change savefile name.  If the UI
					   wants it changed, they can do it. XXX (Good idea?) */
					process_player_name(FALSE);
					
					next_stage = BIRTH_FINAL_CONFIRM;
				}
				else if (cmd.command == CMD_BIRTH_BACK)
				{
					if (roller_choice == BIRTH_POINTBASED)
						next_stage = BIRTH_POINTBASED;
					else if (roller_choice == BIRTH_AUTOROLLER ||
							 roller_choice == BIRTH_ROLLER)
						next_stage = BIRTH_ROLLER;
					else
						next_stage = BIRTH_METHOD_CHOICE;
				} 
				else if (cmd.command == CMD_BIRTH_RESTART)
				{
					next_stage = BIRTH_METHOD_CHOICE;
				}

				break;
			}

			/* 
			 * Here we give the user one last chance to refuse the
			 * character - nothing fancy.
			 */
			case BIRTH_FINAL_CONFIRM:
			{
				event_signal_birthstage(BIRTH_FINAL_CONFIRM, NULL);
				cmd = get_birth_command();
				
				if (cmd.command == CMD_BIRTH_BACK)
				{
					next_stage = BIRTH_NAME_CHOICE;
				}
				else if (cmd.command == CMD_BIRTH_RESTART)
				{
					next_stage = BIRTH_METHOD_CHOICE;
				}
				else if (cmd.command == CMD_ACCEPT_CHARACTER)
				{
					next_stage = BIRTH_COMPLETE;
				}
			}
		}

		last_stage = stage;
		stage = next_stage;
	}
	
	/* Set adult options from birth options */
	for (i = OPT_BIRTH; i < OPT_CHEAT; i++)
	{
		op_ptr->opt[OPT_ADULT + (i - OPT_BIRTH)] = op_ptr->opt[i];
	}

	/* Reset score options from cheat options */
	for (i = OPT_CHEAT; i < OPT_ADULT; i++)
	{
		op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = op_ptr->opt[i];
	}

	/* Reset squelch bits */
	for (i = 0; i < z_info->k_max; i++)
		k_info[i].squelch = FALSE;

	/* Clear the squelch bytes */
	for (i = 0; i < SQUELCH_BYTES; i++)
		squelch_level[i] = 0;

	/* Clear old messages, add new starting message */
	history_clear();
	history_add("Began the quest to destroy Morgoth.", HISTORY_PLAYER_BIRTH, 0);

	/* Note player birth in the message recall */
	message_add(" ", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add("====================", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add(" ", MSG_GENERIC);

	/* Hack -- outfit the player */
	player_outfit();

	/* Initialise the stores */
	store_init();

	/* Tell the UI we're done. */
	event_signal_birthstage(BIRTH_COMPLETE, NULL);

	/* Don't need these any more - the UI has been given fair warning. */
	mem_free(race_choices);
	mem_free(class_choices);

	for (i = 0; i < z_info->p_max; i++)
		mem_free((char *) race_help[i]);

	mem_free(race_help);

	for (i = 0; i < z_info->c_max; i++)
		mem_free((char *) class_help[i]);

	mem_free(class_help);

	/* Now we're really done.. */
	event_signal(EVENT_LEAVE_BIRTH);
}
