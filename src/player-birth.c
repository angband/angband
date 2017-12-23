/**
 * \file player-birth.c
 * \brief Character creation
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
#include "cmd-core.h"
#include "cmds.h"
#include "game-event.h"
#include "game-world.h"
#include "init.h"
#include "mon-lore.h"
#include "monster.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-init.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-properties.h"
#include "obj-randart.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "savefile.h"
#include "store.h"

/**
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


/* These functions are defined at the end of the file */
static int roman_to_int(const char *roman);
static int int_to_roman(int n, char *roman, size_t bufsize);


/*
 * Forward declare
 */
typedef struct birther /*lovely*/ birther; /*sometimes we think she's a dream*/

/**
 * A structure to hold "rolled" information, and any
 * other useful state for the birth process.
 *
 * XXX Demand Obama's birth certificate
 */
struct birther
{
	const struct player_race *race;
	const struct player_class *class;

	s16b age;
	s16b wt;
	s16b ht;
	s16b sc;

	s32b au;

	s16b stat[STAT_MAX];

	char *history;
	char name[PLAYER_NAME_LEN];
};


/**
 * ------------------------------------------------------------------------
 * All of these should be in some kind of 'birth state' struct somewhere else
 * ------------------------------------------------------------------------ */


static int stats[STAT_MAX];
static int points_spent[STAT_MAX];
static int points_left;

static bool quickstart_allowed;
static bool rolled_stats = false;

/**
 * The last character displayed, to allow the user to flick between two.
 * We rely on prev.age being zero to determine whether there is a stored
 * character or not, so initialise it here.
 */
static birther prev;

/**
 * If quickstart is allowed, we store the old character in this,
 * to allow for it to be reloaded if we step back that far in the
 * birth process.
 */
static birther quickstart_prev;




/**
 * Save the currently rolled data into the supplied 'player'.
 */
static void save_roller_data(birther *tosave)
{
	int i;

	/* Save the data */
	tosave->race = player->race;
	tosave->class = player->class;
	tosave->age = player->age;
	tosave->wt = player->wt_birth;
	tosave->ht = player->ht_birth;
	tosave->au = player->au_birth;

	/* Save the stats */
	for (i = 0; i < STAT_MAX; i++)
		tosave->stat[i] = player->stat_birth[i];

	tosave->history = player->history;
	my_strcpy(tosave->name, player->full_name, sizeof(tosave->name));
}


/**
 * Load stored player data from 'player' as the currently rolled data,
 * optionally placing the current data in 'prev_player' (if 'prev_player'
 * is non-NULL).
 *
 * It is perfectly legal to specify the same "birther" for both 'player'
 * and 'prev_player'.
 */
static void load_roller_data(birther *saved, birther *prev_player)
{
	int i;

     /* The initialisation is just paranoia - structure assignment is
        (perhaps) not strictly defined to work with uninitialised parts
        of structures. */
	birther temp;
	memset(&temp, 0, sizeof(birther));

	/* Save the current data if we'll need it later */
	if (prev_player)
		save_roller_data(&temp);

	/* Load previous data */
	player->race     = saved->race;
	player->class    = saved->class;
	player->age      = saved->age;
	player->wt       = player->wt_birth = player->wt;
	player->ht       = player->ht_birth = player->ht;
	player->au_birth = saved->au;
	player->au       = z_info->start_gold;

	/* Load previous stats */
	for (i = 0; i < STAT_MAX; i++) {
		player->stat_max[i] = player->stat_cur[i] = player->stat_birth[i]
			= saved->stat[i];
		player->stat_map[i] = i;
	}

	/* Load previous history */
	player->history = saved->history;
	my_strcpy(player->full_name, saved->name, sizeof(player->full_name));

	/* Save the current data if the caller is interested in it. */
	if (prev_player)
		*prev_player = temp;
}


/**
 * Roll for a characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(int stat_use[STAT_MAX])
{
	int i, j;

	int dice[3 * STAT_MAX];

	/* Roll and verify some stats */
	while (true) {
		/* Roll some dice */
		for (j = i = 0; i < 3 * STAT_MAX; i++) {
			/* Roll the dice */
			dice[i] = randint1(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/* Verify totals */
		if ((j > 7 * STAT_MAX) && (j < 9 * STAT_MAX)) break;
	}

	/* Roll the stats */
	for (i = 0; i < STAT_MAX; i++) {
		int bonus;

		/* Extract 5 + 1d3 + 1d4 + 1d5 */
		j = 5 + dice[3 * i] + dice[3 * i + 1] + dice[3 * i + 2];

		/* Save that value */
		player->stat_max[i] = j;

		/* Obtain a "bonus" for "race" and "class" */
		bonus = player->race->r_adj[i] + player->class->c_adj[i];

		/* Start fully healed */
		player->stat_cur[i] = player->stat_max[i];

		/* Start with unscrambled stats */
		player->stat_map[i] = i;

		/* Efficiency -- Apply the racial/class bonuses */
		stat_use[i] = modify_stat_value(player->stat_max[i], bonus);

		player->stat_birth[i] = player->stat_max[i];
	}
}


static void roll_hp(void)
{
	int i, j, min_value, max_value;

	/* Minimum hitpoints at highest level */
	min_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 3) / 8;
	min_value += PY_MAX_LEVEL;

	/* Maximum hitpoints at highest level */
	max_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 5) / 8;
	max_value += PY_MAX_LEVEL;

	/* Roll out the hitpoints */
	while (true) {
		/* Roll the hitpoint values */
		for (i = 1; i < PY_MAX_LEVEL; i++) {
			j = randint1(player->hitdie);
			player->player_hp[i] = player->player_hp[i-1] + j;
		}

		/* XXX Could also require acceptable "mid-level" hitpoints */

		/* Require "valid" hitpoints at highest level */
		if (player->player_hp[PY_MAX_LEVEL-1] < min_value) continue;
		if (player->player_hp[PY_MAX_LEVEL-1] > max_value) continue;

		/* Acceptable */
		break;
	}
}


static void get_bonuses(void)
{
	/* Calculate the bonuses and hitpoints */
	player->upkeep->update |= (PU_BONUS | PU_HP);

	/* Update stuff */
	update_stuff(player);

	/* Fully healed */
	player->chp = player->mhp;

	/* Fully rested */
	player->csp = player->msp;
}


/**
 * Get the racial history, and social class, using the "history charts".
 */
char *get_history(struct history_chart *chart)
{
	struct history_entry *entry;
	char *res = NULL;

	while (chart) {
		int roll = randint1(100);
		for (entry = chart->entries; entry; entry = entry->next)
			if (roll <= entry->roll)
				break;
		assert(entry);

		res = string_append(res, entry->text);
		chart = entry->succ;
	}

	return res;
}


/**
 * Computes character's age, height, and weight
 */
static void get_ahw(struct player *p)
{
	/* Calculate the age */
	p->age = p->race->b_age + randint1(p->race->m_age);

	/* Calculate the height/weight */
	p->ht = p->ht_birth = Rand_normal(p->race->base_hgt, p->race->mod_hgt);
	p->wt = p->wt_birth = Rand_normal(p->race->base_wgt, p->race->mod_wgt);
}




/**
 * Creates the player's body
 */
static void player_embody(struct player *p)
{
	char buf[80];
	int i;

	assert(p->race);

	memcpy(&p->body, &bodies[p->race->body], sizeof(p->body));
	my_strcpy(buf, bodies[p->race->body].name, sizeof(buf));
	p->body.name = string_make(buf);
	p->body.slots = mem_zalloc(p->body.count * sizeof(struct equip_slot));
	for (i = 0; i < p->body.count; i++) {
		p->body.slots[i].type = bodies[p->race->body].slots[i].type;
		my_strcpy(buf, bodies[p->race->body].slots[i].name, sizeof(buf));
		p->body.slots[i].name = string_make(buf);
	}
}

/**
 * Get the player's starting money
 */
static void get_money(void)
{
	player->au = player->au_birth = z_info->start_gold;
}

void player_init(struct player *p)
{
	int i;
	struct player_options opts_save = p->opts;

	if (p->upkeep) {
		if (p->upkeep->inven)
			mem_free(p->upkeep->inven);
		if (p->upkeep->quiver)
			mem_free(p->upkeep->quiver);
		mem_free(p->upkeep);
	}
	if (p->timed)
		mem_free(p->timed);
	if (p->obj_k) {
		mem_free(p->obj_k->brands);
		mem_free(p->obj_k->slays);
		mem_free(p->obj_k->curses);
		mem_free(p->obj_k);
	}

	/* Wipe the player */
	memset(p, 0, sizeof(struct player));

	/* Start with no artifacts made yet */
	for (i = 0; z_info && i < z_info->a_max; i++) {
		struct artifact *art = &a_info[i];
		art->created = false;
		art->seen = false;
	}

	/* Start with no quests */
	player_quests_reset(p);

	for (i = 1; z_info && i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];
		kind->tried = false;
		kind->aware = false;
	}

	for (i = 1; z_info && i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];
		struct monster_lore *lore = &l_list[i];
		race->cur_num = 0;
		race->max_num = 100;
		if (rf_has(race->flags, RF_UNIQUE))
			race->max_num = 1;
		lore->pkills = 0;
	}

	/* Always start with a well fed player (this is surely in the wrong fn) */
	p->food = PY_FOOD_FULL - 1;

	p->upkeep = mem_zalloc(sizeof(struct player_upkeep));
	p->upkeep->inven = mem_zalloc((z_info->pack_size + 1) *
								  sizeof(struct object *));
	p->upkeep->quiver = mem_zalloc(z_info->quiver_size *
								   sizeof(struct object *));
	p->timed = mem_zalloc(TMD_MAX * sizeof(s16b));
	p->obj_k = mem_zalloc(sizeof(struct object));
	p->obj_k->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	p->obj_k->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	p->obj_k->curses = mem_zalloc(z_info->curse_max *
								  sizeof(struct curse_data));

	/* Options should persist */
	p->opts = opts_save;

	/* First turn. */
	turn = 1;
	p->total_energy = 0;
	p->resting_turn = 0;

	/* Default to the first race/class in the edit file */
	p->race = races;
	p->class = classes;
}

/**
 * Try to wield everything wieldable in the inventory.
 */
void wield_all(struct player *p)
{
	struct object *obj, *new_pile = NULL, *new_known_pile = NULL;
	int slot;

	/* Scan through the slots */
	for (obj = p->gear; obj; obj = obj->next) {
		struct object *obj_temp;

		/* Skip non-objects */
		assert(obj);

		/* Make sure we can wield it */
		slot = wield_slot(obj);
		if (slot < 0 || slot >= p->body.count)
			continue;

		obj_temp = slot_object(p, slot);
		if (obj_temp)
			continue;

		/* Split if necessary */
		if (obj->number > 1) {
			/* All but one go to the new object */
			struct object *new = object_split(obj, obj->number - 1);

			/* Add to the pile of new objects to carry */
			pile_insert(&new_pile, new);
			pile_insert(&new_known_pile, new->known);
		}

		/* Wear the new stuff */
		p->body.slots[slot].obj = obj;
		object_learn_on_wield(p, obj);

		/* Increment the equip counter by hand */
		p->upkeep->equip_cnt++;
	}

	/* Now add the unwielded split objects to the gear */
	if (new_pile) {
		pile_insert_end(&player->gear, new_pile);
		pile_insert_end(&player->gear_k, new_known_pile);
	}
	return;
}


/**
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(struct player *p)
{
	int i;
	const struct start_item *si;
	struct object *obj, *known_obj;

	/* Currently carrying nothing */
	p->upkeep->total_weight = 0;

	/* Give the player obvious object knowledge */
	p->obj_k->dd = 1;
	p->obj_k->ds = 1;
	p->obj_k->ac = 1;
	for (i = 1; i < OF_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
		if (prop->subtype == OFT_LIGHT) of_on(p->obj_k->flags, i);
		if (prop->subtype == OFT_DIG) of_on(p->obj_k->flags, i);
	}

	/* Give the player starting equipment */
	for (si = p->class->start_items; si; si = si->next) {
		int num = rand_range(si->min, si->max);

		/* Without start_kit, only start with 1 food and 1 light */
		if (!OPT(p, birth_start_kit)) {
			if (!tval_is_food_k(si->kind) && !tval_is_light_k(si->kind))
				continue;

			num = 1;
		}

		/* Prepare a new item */
		obj = object_new();
		object_prep(obj, si->kind, 0, MINIMISE);
		obj->number = num;
		obj->origin = ORIGIN_BIRTH;

		known_obj = object_new();
		obj->known = known_obj;
		object_set_base_known(obj);
		object_flavor_aware(obj);
		obj->known->pval = obj->pval;
		obj->known->effect = obj->effect;
		obj->known->notice |= OBJ_NOTICE_ASSESSED;

		/* Deduct the cost of the item from starting cash */
		p->au -= object_value_real(obj, obj->number);

		/* Carry the item */
		inven_carry(p, obj, true, false);
		si->kind->everseen = true;
	}

	/* Sanity check */
	if (p->au < 0)
		p->au = 0;

	/* Now try wielding everything */
	wield_all(p);

	/* Update knowledge */
	update_player_object_knowledge(p);
}


/**
 * Cost of each "point" of a stat.
 */
static const int birth_stat_costs[18 + 1] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 4 };

/* It was feasible to get base 17 in 3 stats with the autoroller */
#define MAX_BIRTH_POINTS 20 /* 3 * (1+1+1+1+1+1+2) */

static void recalculate_stats(int *stats_local_local, int points_left_local)
{
	int i;

	/* Variable stat maxes */
	for (i = 0; i < STAT_MAX; i++) {
		player->stat_cur[i] = player->stat_max[i] =
				player->stat_birth[i] = stats_local_local[i];
		player->stat_map[i] = i;
	}

	/* Gold is inversely proportional to cost */
	player->au_birth = z_info->start_gold + (50 * points_left_local);

	/* Update bonuses, hp, etc. */
	get_bonuses();

	/* Tell the UI about all this stuff that's changed. */
	event_signal(EVENT_GOLD);
	event_signal(EVENT_AC);
	event_signal(EVENT_HP);
	event_signal(EVENT_STATS);
}

static void reset_stats(int stats_local[STAT_MAX], int points_spent_local_local[STAT_MAX],
						int *points_left_local, bool update_display)
{
	int i;

	/* Calculate and signal initial stats and points totals. */
	*points_left_local = MAX_BIRTH_POINTS;

	for (i = 0; i < STAT_MAX; i++) {
		/* Initial stats are all 10 and costs are zero */
		stats_local[i] = 10;
		points_spent_local_local[i] = 0;
	}

	/* Use the new "birth stat" values to work out the "other"
	   stat values (i.e. after modifiers) and tell the UI things have 
	   changed. */
	if (update_display) {
		recalculate_stats(stats_local, *points_left_local);
		event_signal_birthpoints(points_spent_local_local, *points_left_local);	
	}
}

static bool buy_stat(int choice, int stats_local[STAT_MAX],
					 int points_spent_local[STAT_MAX], int *points_left_local,
					 bool update_display)
{
	/* Must be a valid stat, and have a "base" of below 18 to be adjusted */
	if (!(choice >= STAT_MAX || choice < 0) &&	(stats_local[choice] < 18)) {
		/* Get the cost of buying the extra point (beyond what
		   it has already cost to get this far). */
		int stat_cost = birth_stat_costs[stats_local[choice] + 1];

		if (stat_cost <= *points_left_local) {
			stats_local[choice]++;
			points_spent_local[choice] += stat_cost;
			*points_left_local -= stat_cost;

			if (update_display) {
				/* Tell the UI the new points situation. */
				event_signal_birthpoints(points_spent_local, *points_left_local);

				/* Recalculate everything that's changed because
				   the stat has changed, and inform the UI. */
				recalculate_stats(stats_local, *points_left_local);
			}

			return true;
		}
	}

	/* Didn't adjust stat. */
	return false;
}


static bool sell_stat(int choice, int stats_local[STAT_MAX], int points_spent_local[STAT_MAX],
	int *points_left_local, bool update_display)
{
	/* Must be a valid stat, and we can't "sell" stats below the base of 10. */
	if (!(choice >= STAT_MAX || choice < 0) && (stats_local[choice] > 10)) {
		int stat_cost = birth_stat_costs[stats_local[choice]];

		stats_local[choice]--;
		points_spent_local[choice] -= stat_cost;
		*points_left_local += stat_cost;

		if (update_display) {
			/* Tell the UI the new points situation. */
			event_signal_birthpoints(points_spent_local, *points_left_local);

			/* Recalculate everything that's changed because
			   the stat has changed, and inform the UI. */
			recalculate_stats(stats_local, *points_left_local);
	
			return true;
		}
	}

	/* Didn't adjust stat. */
	return false;
}


/**
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
 *    on DEX and then the non-spell-stat.
 */
static void generate_stats(int stats_local[STAT_MAX], int points_spent_local[STAT_MAX], 
						   int *points_left_local)
{
	int step = 0;
	bool maxed[STAT_MAX] = { 0 };
	/* Hack - for now, just use stat of first book - NRM */
	int spell_stat = player->class->magic.total_spells ?
		player->class->magic.books[0].realm->stat : 0;
	bool caster = false, warrior = false;

	/* Determine whether the class is warrior */
	if (player->class->max_attacks > 5) { 
		warrior = true;
	}

	/* Determine whether the class is priest/mage */
	if (player->class->max_attacks < 5) {
		caster = true;
	}

	while (*points_left_local && step >= 0) {
	
		switch (step) {
		
			/* Buy base STR 17 */
			case 0: {
			
				if (!maxed[STAT_STR] && stats_local[STAT_STR] < 17) {
					if (!buy_stat(STAT_STR, stats_local, points_spent_local, points_left_local,
								  false))
						maxed[STAT_STR] = true;
				} else {
					step++;
					
					/* If pure caster skip to step 3 */
					if (caster){
						step = 3;
					}
				}

				break;
			}

			/* Try and buy adj DEX of 18/10 */
			case 1: {
				if (!maxed[STAT_DEX] && player->state.stat_top[STAT_DEX]
					< 18+10) {
					if (!buy_stat(STAT_DEX, stats_local, points_spent_local, points_left_local,
								  false))
						maxed[STAT_DEX] = true;
				} else {
					step++;
				}

				break;
			}

			/* If we can't get 18/10 dex, sell it back. */
			case 2: {
				if (player->state.stat_top[STAT_DEX] < 18+10) {
					while (stats_local[STAT_DEX] > 10)
						sell_stat(STAT_DEX, stats_local, points_spent_local, points_left_local,
								  false);
					maxed[STAT_DEX] = false;
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
				int points_trigger = *points_left_local / 2;
				
				if (warrior) {
					points_trigger = *points_left_local;
				} else {
					while (!maxed[spell_stat] &&
						   (caster || stats_local[spell_stat] < 16) &&
						   points_spent_local[spell_stat] < points_trigger) {

						if (!buy_stat(spell_stat, stats_local, points_spent_local,
									  points_left_local, false)) {
							maxed[spell_stat] = true;
						}

						if (points_spent_local[spell_stat] > points_trigger) {
						
							sell_stat(spell_stat, stats_local, points_spent_local, 
									  points_left_local, false);
							maxed[spell_stat] = true;
						}
					}
				}

				/* Skip CON for casters because DEX is more important early
				 * and is handled in 4 */
				while (!maxed[STAT_CON] &&
					   !(caster) && stats_local[STAT_CON] < 16 &&
					   points_spent_local[STAT_CON] < points_trigger) {
					   
					if (!buy_stat(STAT_CON, stats_local, points_spent_local,points_left_local,
								  false)) {
						maxed[STAT_CON] = true;
					}

					if (points_spent_local[STAT_CON] > points_trigger) {
						sell_stat(STAT_CON, stats_local, points_spent_local, points_left_local,
								  false);
						maxed[STAT_CON] = true;
					}
				}
				
				step++;
				break;
			}

			/* 
			 * If there are any points left, spend as much as possible in 
			 * order on DEX, and the non-spell-stat. 
			 */
			case 4:{
			
				int next_stat;

				if (!maxed[STAT_DEX]) {
					next_stat = STAT_DEX;
				} else if (!maxed[STAT_INT] && spell_stat != STAT_INT) {
					next_stat = STAT_INT;
				} else if (!maxed[STAT_WIS] && spell_stat != STAT_WIS) {
					next_stat = STAT_WIS;
				} else {
					step++;
					break;
				}

				/* Buy until we can't buy any more. */
				while (buy_stat(next_stat, stats_local, points_spent_local, points_left_local,
								false));
				maxed[next_stat] = true;

				break;
			}

			default: {
			
				step = -1;
				break;
			}
		}
	}
	/* Tell the UI the new points situation. */
	event_signal_birthpoints(points_spent_local, *points_left_local);

	/* Recalculate everything that's changed because
	   the stat has changed, and inform the UI. */
	recalculate_stats(stats_local, *points_left_local);
}

/**
 * This fleshes out a full player based on the choices currently made,
 * and so is called whenever things like race or class are chosen.
 */
void player_generate(struct player *p, const struct player_race *r,
					 const struct player_class *c, bool old_history)
{
	if (!c)
		c = p->class;
	if (!r)
		r = p->race;

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

	if (!old_history)
		p->history = get_history(p->race->history);
}


/**
 * Reset everything back to how it would be on loading the game.
 */
static void do_birth_reset(bool use_quickstart, birther *quickstart_prev_local)
{
	/* If there's quickstart data, we use it to set default
	   character choices. */
	if (use_quickstart && quickstart_prev_local)
		load_roller_data(quickstart_prev_local, NULL);

	player_generate(player, NULL, NULL, use_quickstart && quickstart_prev_local);

	player->depth = 0;

	/* Update stats with bonuses, etc. */
	get_bonuses();
}

void do_cmd_birth_init(struct command *cmd)
{
	char *buf;

	/* The dungeon is not ready */
	character_dungeon = false;

	/*
	 * If there's a quickstart character, store it for later use.
	 * If not, default to whatever the first of the choices is.
	 */
	if (player->ht_birth) {
		save_roller_data(&quickstart_prev);
		quickstart_allowed = true;
	} else {
		player_generate(player, player_id2race(0), player_id2class(0), false);
		quickstart_allowed = false;
	}

	/* Handle incrementing name suffix */
	buf = find_roman_suffix_start(player->full_name);
	if (buf) {
		/* Try to increment the roman suffix */
		int success = int_to_roman(
				roman_to_int(buf) + 1,
				buf,
				sizeof(player->full_name) - (buf - (char *)&player->full_name));

		if (success) {
			save_roller_data(&quickstart_prev);
		} else {
			msg("Sorry, could not deal with suffix");
		}
	}

	/* We're ready to start the birth process */
	event_signal_flag(EVENT_ENTER_BIRTH, quickstart_allowed);
}

void do_cmd_birth_reset(struct command *cmd)
{
	player_init(player);
	reset_stats(stats, points_spent, &points_left, false);
	do_birth_reset(quickstart_allowed, &quickstart_prev);
	rolled_stats = false;
}

void do_cmd_choose_race(struct command *cmd)
{
	int choice;
	cmd_get_arg_choice(cmd, "choice", &choice);
	player_generate(player, player_id2race(choice), NULL, false);

	reset_stats(stats, points_spent, &points_left, false);
	generate_stats(stats, points_spent, &points_left);
	rolled_stats = false;
}

void do_cmd_choose_class(struct command *cmd)
{
	int choice;
	cmd_get_arg_choice(cmd, "choice", &choice);
	player_generate(player, NULL, player_id2class(choice), false);

	reset_stats(stats, points_spent, &points_left, false);
	generate_stats(stats, points_spent, &points_left);
	rolled_stats = false;
}

void do_cmd_buy_stat(struct command *cmd)
{
	/* .choice is the stat to sell */
	if (!rolled_stats) {
		int choice;
		cmd_get_arg_choice(cmd, "choice", &choice);
		buy_stat(choice, stats, points_spent, &points_left, true);
	}
}

void do_cmd_sell_stat(struct command *cmd)
{
	/* .choice is the stat to sell */
	if (!rolled_stats) {
		int choice;
		cmd_get_arg_choice(cmd, "choice", &choice);
		sell_stat(choice, stats, points_spent, &points_left, true);
	}
}

void do_cmd_reset_stats(struct command *cmd)
{
	/* .choice is whether to regen stats */
	int choice;

	reset_stats(stats, points_spent, &points_left, true);

	cmd_get_arg_choice(cmd, "choice", &choice);
	if (choice)
		generate_stats(stats, points_spent, &points_left);

	rolled_stats = false;
}

void do_cmd_roll_stats(struct command *cmd)
{
	int i;

	save_roller_data(&prev);

	/* Get a new character */
	get_stats(stats);

	/* Update stats with bonuses, etc. */
	get_bonuses();

	/* There's no real need to do this here, but it's tradition. */
	get_ahw(player);
	if (player->history)
		string_free(player->history);
	player->history = get_history(player->race->history);

	event_signal(EVENT_GOLD);
	event_signal(EVENT_AC);
	event_signal(EVENT_HP);
	event_signal(EVENT_STATS);

	/* Give the UI some dummy info about the points situation. */
	points_left = 0;
	for (i = 0; i < STAT_MAX; i++)
		points_spent[i] = 0;

	event_signal_birthpoints(points_spent, points_left);

	/* Lock out buying and selling of stats based on rolled stats. */
	rolled_stats = true;
}

void do_cmd_prev_stats(struct command *cmd)
{
	/* Only switch to the stored "previous"
	   character if we've actually got one to load. */
	if (prev.age) {
		load_roller_data(&prev, &prev);
		get_bonuses();
	}

	event_signal(EVENT_GOLD);
	event_signal(EVENT_AC);
	event_signal(EVENT_HP);
	event_signal(EVENT_STATS);	
}

void do_cmd_choose_name(struct command *cmd)
{
	const char *str;
	cmd_get_arg_string(cmd, "name", &str);

	/* Set player name */
	my_strcpy(player->full_name, str, sizeof(player->full_name));

	string_free((char *) str);
}

void do_cmd_choose_history(struct command *cmd)
{
	const char *str;

	/* Forget the old history */
	if (player->history)
		string_free(player->history);

	/* Get the new history */
	cmd_get_arg_string(cmd, "history", &str);
	player->history = string_make(str);

	string_free((char *) str);
}

void do_cmd_accept_character(struct command *cmd)
{
	options_init_cheat();

	roll_hp();

	ignore_birth_init();

	/* Clear old messages, add new starting message */
	history_clear(player);
	history_add(player, "Began the quest to destroy Morgoth.", HIST_PLAYER_BIRTH);

	/* Note player birth in the message recall */
	message_add(" ", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add("====================", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add(" ", MSG_GENERIC);

	/* Embody */
	player_embody(player);

	/* Give the player some money */
	get_money();

	/* Initialise the spells */
	player_spells_init(player);

	/* Know all runes for ID on walkover */
	if (OPT(player, birth_know_runes))
		player_learn_all_runes(player);

	/* Hack - player knows all combat runes.  Maybe make them not runes? NRM */
	player->obj_k->to_a = 1;
	player->obj_k->to_h = 1;
	player->obj_k->to_d = 1;

	/* Initialise the stores */
	store_reset();

	/* Player learns innate runes */
	player_learn_innate(player);

	/* Randomize the artifacts if required */
	if (OPT(player, birth_randarts)) {
		/* First restore the standard artifacts */
		cleanup_parser(&randart_parser);
		deactivate_randart_file();
		run_parser(&artifact_parser);

		/* Now generate the new randarts */
		seed_randart = randint0(0x10000000);
		do_randart(seed_randart, true);
	}

	/* Seed for flavors */
	seed_flavor = randint0(0x10000000);
	flavor_init();

	/* Know all flavors for auto-ID of consumables */
	if (OPT(player, birth_know_flavors))
		flavor_set_all_aware();

	/* Outfit the player, if they can sell the stuff */
	player_outfit(player);

	/* Stop the player being quite so dead */
	player->is_dead = false;

	/* Character is now "complete" */
	character_generated = true;
	player->upkeep->playing = true;

	/* Disable repeat command, so we don't try to be born again */
	cmd_disable_repeat();

	/* Now we're really done.. */
	event_signal(EVENT_LEAVE_BIRTH);
}



/**
 * ------------------------------------------------------------------------
 * Roman numeral functions, for dynastic successions
 * ------------------------------------------------------------------------ */


/**
 * Find the start of a possible Roman numerals suffix by going back from the
 * end of the string to a space, then checking that all the remaining chars
 * are valid Roman numerals.
 * 
 * Return the start position, or NULL if there isn't a valid suffix. 
 */
char *find_roman_suffix_start(const char *buf)
{
	const char *start = strrchr(buf, ' ');
	const char *p;
	
	if (start) {
		start++;
		p = start;
		while (*p) {
			if (*p != 'I' && *p != 'V' && *p != 'X' && *p != 'L' &&
			    *p != 'C' && *p != 'D' && *p != 'M') {
				start = NULL;
				break;
			}
			++p;			    
		}
	}
	return (char *)start;
}

/**
 * Converts an arabic numeral (int) to a roman numeral (char *).
 *
 * An arabic numeral is accepted in parameter `n`, and the corresponding
 * upper-case roman numeral is placed in the parameter `roman`.  The
 * length of the buffer must be passed in the `bufsize` parameter.  When
 * there is insufficient room in the buffer, or a roman numeral does not
 * exist (e.g. non-positive integers) a value of 0 is returned and the
 * `roman` buffer will be the empty string.  On success, a value of 1 is
 * returned and the zero-terminated roman numeral is placed in the
 * parameter `roman`.
 */
static int int_to_roman(int n, char *roman, size_t bufsize)
{
	/* Roman symbols */
	char roman_symbol_labels[13][3] =
		{"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX",
		 "V", "IV", "I"};
	int  roman_symbol_values[13] =
		{1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};

	/* Clear the roman numeral buffer */
	roman[0] = '\0';

	/* Roman numerals have no zero or negative numbers */
	if (n < 1)
		return 0;

	/* Build the roman numeral in the buffer */
	while (n > 0) {
		int i = 0;

		/* Find the largest possible roman symbol */
		while (n < roman_symbol_values[i])
			i++;

		/* No room in buffer, so abort */
		if (strlen(roman) + strlen(roman_symbol_labels[i]) + 1
			> bufsize)
			break;

		/* Add the roman symbol to the buffer */
		my_strcat(roman, roman_symbol_labels[i], bufsize);

		/* Decrease the value of the arabic numeral */
		n -= roman_symbol_values[i];
	}

	/* Ran out of space and aborted */
	if (n > 0) {
		/* Clean up and return */
		roman[0] = '\0';

		return 0;
	}

	return 1;
}


/**
 * Converts a roman numeral (char *) to an arabic numeral (int).
 *
 * The null-terminated roman numeral is accepted in the `roman`
 * parameter and the corresponding integer arabic numeral is returned.
 * Only upper-case values are considered. When the `roman` parameter
 * is empty or does not resemble a roman numeral, a value of -1 is
 * returned.
 *
 * XXX This function will parse certain non-sense strings as roman
 *     numerals, such as IVXCCCVIII
 */
static int roman_to_int(const char *roman)
{
	size_t i;
	int n = 0;
	char *p;

	char roman_token_chr1[] = "MDCLXVI";
	const char *roman_token_chr2[] = {0, 0, "DM", 0, "LC", 0, "VX"};

	int roman_token_vals[7][3] = {{1000},
	                              {500},
	                              {100, 400, 900},
	                              {50},
	                              {10, 40, 90},
	                              {5},
	                              {1, 4, 9}};

	if (strlen(roman) == 0)
		return -1;

	/* Check each character for a roman token, and look ahead to the
	   character after this one to check for subtraction */
	for (i = 0; i < strlen(roman); i++) {
		char c1, c2;
		int c1i, c2i;

		/* Get the first and second chars of the next roman token */
		c1 = roman[i];
		c2 = roman[i + 1];

		/* Find the index for the first character */
		p = strchr(roman_token_chr1, c1);
		if (p)
			c1i = p - roman_token_chr1;
		else
			return -1;

		/* Find the index for the second character */
		c2i = 0;
		if (roman_token_chr2[c1i] && c2) {
			p = strchr(roman_token_chr2[c1i], c2);
			if (p) {
				c2i = (p - roman_token_chr2[c1i]) + 1;
				/* Two-digit token, so skip a char on the next pass */
				i++;
			}
		}

		/* Increase the arabic numeral */
		n += roman_token_vals[c1i][c2i];
	}

	return n;
}
