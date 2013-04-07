/*
 * File: save.c
 * Purpose: Old-style savefile saving
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "dungeon.h"
#include "history.h"
#include "monster/mon-make.h"
#include "monster/monster.h"
#include "option.h"
#include "savefile.h"
#include "squelch.h"


/*
 * Write a description of the character
 */
void wr_description(void)
{
	char buf[1024];

	if (p_ptr->is_dead)
		strnfmt(buf, sizeof buf, "%s, dead (%s)", op_ptr->full_name, p_ptr->died_from);
	else
		strnfmt(buf, sizeof buf, "%s, L%d %s %s, at DL%d",
				op_ptr->full_name,
				p_ptr->lev,
				p_ptr->race->name,
				p_ptr->class->name,
				p_ptr->depth);

	wr_string(buf);
}


/*
 * Write an "item" record
 */
static void wr_item(const object_type *o_ptr)
{
	size_t i, j;

	wr_uint16_t(0xffff);
	wr_uint8_t(ITEM_VERSION);

	wr_int16_t(0);

	/* Location */
	wr_uint8_t(o_ptr->iy);
	wr_uint8_t(o_ptr->ix);

	wr_uint8_t(o_ptr->tval);
	wr_uint8_t(o_ptr->sval);

        for (i = 0; i < MAX_PVALS; i++) {
		wr_int16_t(o_ptr->pval[i]);
        }
        wr_uint8_t(o_ptr->num_pvals);

	wr_uint8_t(0);

	wr_uint8_t(o_ptr->number);
	wr_int16_t(o_ptr->weight);

	if (o_ptr->artifact) wr_uint8_t(o_ptr->artifact->aidx);
	else wr_uint8_t(0);

	if (o_ptr->ego) wr_uint8_t(o_ptr->ego->eidx);
	else wr_uint8_t(0);

	wr_int16_t(o_ptr->timeout);

	wr_int16_t(o_ptr->to_h);
	wr_int16_t(o_ptr->to_d);
	wr_int16_t(o_ptr->to_a);
	wr_int16_t(o_ptr->ac);
	wr_uint8_t(o_ptr->dd);
	wr_uint8_t(o_ptr->ds);

	wr_uint16_t(o_ptr->ident);

	wr_uint8_t(o_ptr->marked);

	wr_uint8_t(o_ptr->origin);
	wr_uint8_t(o_ptr->origin_depth);
	wr_uint16_t(o_ptr->origin_xtra);
	wr_uint8_t(o_ptr->ignore);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		wr_uint8_t(o_ptr->flags[i]);
	if (i < OF_BYTES) pad_uint8_ts(OF_BYTES - i);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		wr_uint8_t(o_ptr->known_flags[i]);
	if (i < OF_BYTES) pad_uint8_ts(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			wr_uint8_t(o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) pad_uint8_ts(OF_BYTES - i);
	}

	/* Held by monster index */
	wr_int16_t(o_ptr->held_m_idx);
	
	wr_int16_t(o_ptr->mimicking_m_idx);

	/* Save the inscription (if any) */
	if (o_ptr->note)
	{
		wr_string(quark_str(o_ptr->note));
	}
	else
	{
		wr_string("");
	}
}


/*
 * Write RNG state
 *
 * There were originally 64 uint8_ts of randomizer saved. Now we only need
 * 32 + 5 uint8_ts saved, so we'll write an extra 27 uint8_ts at the end which won't
 * be used.
 */
void wr_randomizer(void)
{
	int i;

	/* current value for the simple RNG */
	wr_uint32_t(Rand_value);

	/* state index */
	wr_uint32_t(state_i);

	/* RNG variables */
	wr_uint32_t(z0);
	wr_uint32_t(z1);
	wr_uint32_t(z2);

	/* RNG state */
	for (i = 0; i < RAND_DEG; i++)
		wr_uint32_t(STATE[i]);

	/* NULL padding */
	for (i = 0; i < 59 - RAND_DEG; i++)
		wr_uint32_t(0);
}


/*
 * Write the "options"
 */
void wr_options(void)
{
	int i, k;

	uint32_t window_flag[ANGBAND_TERM_MAX];
	uint32_t window_mask[ANGBAND_TERM_MAX];


	/* Special Options */
	wr_uint8_t(op_ptr->delay_factor);
	wr_uint8_t(op_ptr->hitpoint_warn);
	wr_uint16_t(lazymove_delay);

	/* Normal options */
	for (i = 0; i < OPT_MAX; i++) {
		const char *name = option_name(i);
		if (!name)
			continue;

		wr_string(name);
		wr_uint8_t(op_ptr->opt[i]);
   }

	/* Sentinel */
	wr_uint8_t(0);

	/*** Window options ***/

	/* Reset */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		/* Flags */
		window_flag[i] = op_ptr->window_flag[i];

		/* Mask */
		window_mask[i] = 0L;

		/* Build the mask */
		for (k = 0; k < 32; k++)
		{
			/* Set mask */
			if (window_flag_desc[k])
			{
				window_mask[i] |= (1L << k);
			}
		}
	}

	/* Dump the flags */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) wr_uint32_t(window_flag[i]);

	/* Dump the masks */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) wr_uint32_t(window_mask[i]);
}


void wr_messages(void)
{
	int16_t i;
	uint16_t num;

	num = messages_num();
	if (num > 80) num = 80;
	wr_uint16_t(num);

	/* Dump the messages (oldest first!) */
	for (i = num - 1; i >= 0; i--)
	{
		wr_string(message_str(i));
		wr_uint16_t(message_type(i));
	}
}


void wr_monster_memory(void)
{
	size_t i;
	int r_idx;

	wr_uint16_t(z_info->r_max);
	for (r_idx = 0; r_idx < z_info->r_max; r_idx++)
	{
		monster_race *r_ptr = &r_info[r_idx];
		monster_lore *l_ptr = &l_list[r_idx];

		/* Count sights/deaths/kills */
		wr_int16_t(l_ptr->sights);
		wr_int16_t(l_ptr->deaths);
		wr_int16_t(l_ptr->pkills);
		wr_int16_t(l_ptr->tkills);

		/* Count wakes and ignores */
		wr_uint8_t(l_ptr->wake);
		wr_uint8_t(l_ptr->ignore);

		/* Count drops */
		wr_uint8_t(l_ptr->drop_gold);
		wr_uint8_t(l_ptr->drop_item);

		/* Count spells */
		wr_uint8_t(l_ptr->cast_innate);
		wr_uint8_t(l_ptr->cast_spell);

		/* Count blows of each type */
		for (i = 0; i < MONSTER_BLOW_MAX; i++)
			wr_uint8_t(l_ptr->blows[i]);

		/* Memorize flags */
		for (i = 0; i < RF_BYTES && i < RF_SIZE; i++)
			wr_uint8_t(l_ptr->flags[i]);
		if (i < RF_BYTES) pad_uint8_ts(RF_BYTES - i);

		for (i = 0; i < RF_BYTES && i < RSF_SIZE; i++)
			wr_uint8_t(l_ptr->spell_flags[i]);
		if (i < RF_BYTES) pad_uint8_ts(RF_BYTES - i);

		/* Monster limit per level */
		wr_uint8_t(r_ptr->max_num);

		/* XXX */
		wr_uint8_t(0);
		wr_uint8_t(0);
		wr_uint8_t(0);
	}
}


void wr_object_memory(void)
{
	int k_idx;

	wr_uint16_t(z_info->k_max);
	for (k_idx = 0; k_idx < z_info->k_max; k_idx++)
	{
		uint8_t tmp8u = 0;
		object_kind *k_ptr = &k_info[k_idx];

		if (k_ptr->aware) tmp8u |= 0x01;
		if (k_ptr->tried) tmp8u |= 0x02;
		if (kind_is_squelched_aware(k_ptr)) tmp8u |= 0x04;
		if (k_ptr->everseen) tmp8u |= 0x08;
		if (kind_is_squelched_unaware(k_ptr)) tmp8u |= 0x10;

		wr_uint8_t(tmp8u);
	}
}


void wr_quests(void)
{
	int i;
	uint16_t tmp16u;

	/* Hack -- Dump the quests */
	tmp16u = MAX_Q_IDX;
	wr_uint16_t(tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		wr_uint8_t(q_list[i].level);
		wr_uint8_t(0);
		wr_uint8_t(0);
		wr_uint8_t(0);
	}
}


void wr_artifacts(void)
{
	int i;
	uint16_t tmp16u;

	/* Hack -- Dump the artifacts */
	tmp16u = z_info->a_max;
	wr_uint16_t(tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		wr_uint8_t(a_ptr->created);
		wr_uint8_t(a_ptr->seen);
		wr_uint8_t(a_ptr->everseen);
		wr_uint8_t(0);
	}
}


void wr_player(void)
{
	int i;

	wr_string(op_ptr->full_name);

	wr_string(p_ptr->died_from);

	wr_string(p_ptr->history);

	/* Race/Class/Gender/Spells */
	wr_uint8_t(p_ptr->race->ridx);
	wr_uint8_t(p_ptr->class->cidx);
	wr_uint8_t(p_ptr->psex);
	wr_uint8_t(op_ptr->name_suffix);

	wr_uint8_t(p_ptr->hitdie);
	wr_uint8_t(p_ptr->expfact);

	wr_int16_t(p_ptr->age);
	wr_int16_t(p_ptr->ht);
	wr_int16_t(p_ptr->wt);

	/* Dump the stats (maximum and current and birth) */
	for (i = 0; i < A_MAX; ++i) wr_int16_t(p_ptr->stat_max[i]);
	for (i = 0; i < A_MAX; ++i) wr_int16_t(p_ptr->stat_cur[i]);
	for (i = 0; i < A_MAX; ++i) wr_int16_t(p_ptr->stat_birth[i]);

	wr_int16_t(p_ptr->ht_birth);
	wr_int16_t(p_ptr->wt_birth);
	wr_int16_t(0);
	wr_uint32_t(p_ptr->au_birth);

	/* Padding */
	wr_uint32_t(0);

	wr_uint32_t(p_ptr->au);


	wr_uint32_t(p_ptr->max_exp);
	wr_uint32_t(p_ptr->exp);
	wr_uint16_t(p_ptr->exp_frac);
	wr_int16_t(p_ptr->lev);

	wr_int16_t(p_ptr->mhp);
	wr_int16_t(p_ptr->chp);
	wr_uint16_t(p_ptr->chp_frac);

	wr_int16_t(p_ptr->msp);
	wr_int16_t(p_ptr->csp);
	wr_uint16_t(p_ptr->csp_frac);

	/* Max Player and Dungeon Levels */
	wr_int16_t(p_ptr->max_lev);
	wr_int16_t(p_ptr->max_depth);

	/* More info */
	wr_int16_t(0);	/* oops */
	wr_int16_t(0);	/* oops */
	wr_int16_t(0);	/* oops */
	wr_int16_t(0);	/* oops */
	wr_int16_t(0); /* oops */
	wr_int16_t(p_ptr->deep_descent);

	wr_int16_t(p_ptr->food);
	wr_int16_t(p_ptr->energy);
	wr_int16_t(p_ptr->word_recall);
	wr_int16_t(p_ptr->state.see_infra);
	wr_uint8_t(p_ptr->confusing);
	wr_uint8_t(p_ptr->searching);

	/* Find the number of timed effects */
	wr_uint8_t(TMD_MAX);

	/* Read all the effects, in a loop */
	for (i = 0; i < TMD_MAX; i++)
		wr_int16_t(p_ptr->timed[i]);

	/* Total energy used so far */
	wr_uint32_t(p_ptr->total_energy);
	/* # of turns spent resting */
	wr_uint32_t(p_ptr->resting_turn);

	/* Future use */
	for (i = 0; i < 8; i++) wr_uint32_t(0L);
}


void wr_squelch(void)
{
	size_t i, n;

	/* Write number of squelch uint8_ts */
	wr_uint8_t(squelch_size);
	for (i = 0; i < squelch_size; i++)
		wr_uint8_t(squelch_level[i]);

	/* Write ego-item squelch bits */
	wr_uint16_t(z_info->e_max);
	for (i = 0; i < z_info->e_max; i++)
	{
		uint8_t flags = 0;

		/* Figure out and write the everseen flag */
		if (e_info[i].everseen) flags |= 0x02;
		wr_uint8_t(flags);
	}

	n = 0;
	for (i = 0; i < z_info->k_max; i++)
		if (k_info[i].note)
			n++;

	/* Write the current number of auto-inscriptions */
	wr_uint16_t(n);

	/* Write the autoinscriptions array */
	for (i = 0; i < z_info->k_max; i++) {
		if (!k_info[i].note)
			continue;
		wr_int16_t(i);
		wr_string(quark_str(k_info[i].note));
	}

	return;
}


void wr_misc(void)
{
	wr_uint32_t(0L);

	/* Random artifact seed */
	wr_uint32_t(seed_randart);


	/* XXX Ignore some flags */
	wr_uint32_t(0L);
	wr_uint32_t(0L);
	wr_uint32_t(0L);


	/* Write the "object seeds" */
	wr_uint32_t(seed_flavor);
	wr_uint32_t(seed_town);


	/* Special stuff */
	wr_uint16_t(p_ptr->panic_save);
	wr_uint16_t(p_ptr->total_winner);
	wr_uint16_t(p_ptr->noscore);


	/* Write death */
	wr_uint8_t(p_ptr->is_dead);

	/* Write feeling */
	wr_uint8_t(cave->feeling);
	wr_uint16_t(cave->feeling_squares);
	wr_int32_t(cave->created_at);

	/* Current turn */
	wr_int32_t(turn);
}


void wr_player_hp(void)
{
	int i;

	wr_uint16_t(PY_MAX_LEVEL);
	for (i = 0; i < PY_MAX_LEVEL; i++)
		wr_int16_t(p_ptr->player_hp[i]);
}


void wr_player_spells(void)
{
	int i;

	wr_uint16_t(PY_MAX_SPELLS);

	for (i = 0; i < PY_MAX_SPELLS; i++)
		wr_uint8_t(p_ptr->spell_flags[i]);

	for (i = 0; i < PY_MAX_SPELLS; i++)
		wr_uint8_t(p_ptr->spell_order[i]);
}


void wr_inventory(void)
{
	int i;

	/* Write the inventory */
	for (i = 0; i < ALL_INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Dump index */
		wr_uint16_t((uint16_t)i);

		/* Dump object */
		wr_item(o_ptr);
	}

	/* Add a sentinel */
	wr_uint16_t(0xFFFF);
}


void wr_stores(void)
{
	int i;

	wr_uint16_t(MAX_STORES);
	for (i = 0; i < MAX_STORES; i++)
	{
		const struct store *st_ptr = &stores[i];
		int j;

		/* XXX Old values */
		wr_uint32_t(0L);
		wr_int16_t(0);

		/* Save the current owner */
		wr_uint8_t(st_ptr->owner->oidx);

		/* Save the stock size */
		wr_uint8_t(st_ptr->stock_num);

		/* XXX Old values */
		wr_int16_t(0);
		wr_int16_t(0);

		/* Save the stock */
		for (j = 0; j < st_ptr->stock_num; j++)
			wr_item(&st_ptr->stock[j]);
	}
}



/*
 * The cave grid flags that get saved in the savefile
 */
#define IMPORTANT_FLAGS (CAVE_MARK | CAVE_GLOW | CAVE_VAULT | CAVE_ROOM)


/*
 * Write the current dungeon
 */
void wr_dungeon(void)
{
	int y, x;

	uint8_t tmp8u;

	uint8_t count;
	uint8_t prev_char;


	if (p_ptr->is_dead)
		return;

	/*** Basic info ***/

	/* Dungeon specific info follows */
	wr_uint16_t(p_ptr->depth);
	wr_uint16_t(daycount);
	wr_uint16_t(p_ptr->py);
	wr_uint16_t(p_ptr->px);
	wr_uint16_t(cave->height);
	wr_uint16_t(cave->width);
	wr_uint16_t(0);
	wr_uint16_t(0);


	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Note that this will induce two wasted uint8_ts */
	count = 0;
	prev_char = 0;

	/* Dump the cave */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Extract the important cave->info flags */
			tmp8u = (cave->info[y][x] & (IMPORTANT_FLAGS));

			/* If the run is broken, or too full, flush it */
			if ((tmp8u != prev_char) || (count == MAX_UCHAR))
			{
				wr_uint8_t((uint8_t)count);
				wr_uint8_t((uint8_t)prev_char);
				prev_char = tmp8u;
				count = 1;
			}

			/* Continue the run */
			else
			{
				count++;
			}
		}
	}

	/* Flush the data (if any) */
	if (count)
	{
		wr_uint8_t((uint8_t)count);
		wr_uint8_t((uint8_t)prev_char);
	}

	/** Now dump the cave->info2[][] stuff **/

	/* Note that this will induce two wasted uint8_ts */
	count = 0;
	prev_char = 0;

	/* Dump the cave */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Keep all the information from info2 */
			tmp8u = cave->info2[y][x];

			/* If the run is broken, or too full, flush it */
			if ((tmp8u != prev_char) || (count == MAX_UCHAR))
			{
				wr_uint8_t((uint8_t)count);
				wr_uint8_t((uint8_t)prev_char);
				prev_char = tmp8u;
				count = 1;
			}

			/* Continue the run */
			else
			{
				count++;
			}
		}
	}

	/* Flush the data (if any) */
	if (count)
	{
		wr_uint8_t((uint8_t)count);
		wr_uint8_t((uint8_t)prev_char);
	}


	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Note that this will induce two wasted uint8_ts */
	count = 0;
	prev_char = 0;

	/* Dump the cave */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Extract a uint8_t */
			tmp8u = cave->feat[y][x];

			/* If the run is broken, or too full, flush it */
			if ((tmp8u != prev_char) || (count == MAX_UCHAR))
			{
				wr_uint8_t((uint8_t)count);
				wr_uint8_t((uint8_t)prev_char);
				prev_char = tmp8u;
				count = 1;
			}

			/* Continue the run */
			else
			{
				count++;
			}
		}
	}

	/* Flush the data (if any) */
	if (count)
	{
		wr_uint8_t((uint8_t)count);
		wr_uint8_t((uint8_t)prev_char);
	}


	/*** Compact ***/

	/* Compact the objects */
	compact_objects(0);

	/* Compact the monsters */
	compact_monsters(0);
}


void wr_objects(void)
{
	int i;

	if (p_ptr->is_dead)
		return;
	
	/* Total objects */
	wr_uint16_t(o_max);

	/* Dump the objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = object_byid(i);

		/* Dump it */
		wr_item(o_ptr);
	}
}


void wr_monsters(void)
{
	int i;
	size_t j;

	if (p_ptr->is_dead)
		return;

	/* Total monsters */
	wr_uint16_t(cave_monster_max(cave));

	/* Dump the monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		uint8_t unaware = 0;
	
		const monster_type *m_ptr = cave_monster(cave, i);

		wr_int16_t(m_ptr->race->ridx);
		wr_uint8_t(m_ptr->fy);
		wr_uint8_t(m_ptr->fx);
		wr_int16_t(m_ptr->hp);
		wr_int16_t(m_ptr->maxhp);
		wr_uint8_t(m_ptr->mspeed);
		wr_uint8_t(m_ptr->energy);
		wr_uint8_t(MON_TMD_MAX);

		for (j = 0; j < MON_TMD_MAX; j++)
			wr_int16_t(m_ptr->m_timed[j]);

		if (m_ptr->unaware) unaware |= 0x01;
		wr_uint8_t(unaware);

		for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
			wr_uint8_t(m_ptr->known_pflags[j]);
		if (j < OF_BYTES) pad_uint8_ts(OF_BYTES - j);
		
		wr_uint8_t(0);
	}
}


void wr_ghost(void)
{
	int i;

	if (p_ptr->is_dead)
		return;

	/* XXX */
	
	/* Name */
	wr_string("Broken Ghost");
	
	/* Hack -- stupid data */
	for (i = 0; i < 60; i++) wr_uint8_t(0);
}


void wr_history(void)
{
	size_t i;
	uint32_t tmp32u = history_get_num();

	wr_uint32_t(tmp32u);
	for (i = 0; i < tmp32u; i++)
	{
		wr_uint16_t(history_list[i].type);
		wr_int32_t(history_list[i].turn);
		wr_int16_t(history_list[i].dlev);
		wr_int16_t(history_list[i].clev);
		wr_uint8_t(history_list[i].a_idx);
		wr_string(history_list[i].event);
	}
}
