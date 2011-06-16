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
#include "history.h"
#include "monster/monster.h"
#include "option.h"
#include "savefile.h"
#include "squelch.h"

/*
 * Write an "item" record
 */
static void wr_item(const object_type *o_ptr)
{
	size_t i, j;

	wr_u16b(0xffff);
	wr_byte(ITEM_VERSION);

	wr_s16b(0);

	/* Location */
	wr_byte(o_ptr->iy);
	wr_byte(o_ptr->ix);

	wr_byte(o_ptr->tval);
	wr_byte(o_ptr->sval);

        for (i = 0; i < MAX_PVALS; i++) {
		wr_s16b(o_ptr->pval[i]);
        }
        wr_byte(o_ptr->num_pvals);

	wr_byte(0);

	wr_byte(o_ptr->number);
	wr_s16b(o_ptr->weight);

	if (o_ptr->artifact) wr_byte(o_ptr->artifact->aidx);
	else wr_byte(0);

	if (o_ptr->ego) wr_byte(o_ptr->ego->eidx);
	else wr_byte(0);

	wr_s16b(o_ptr->timeout);

	wr_s16b(o_ptr->to_h);
	wr_s16b(o_ptr->to_d);
	wr_s16b(o_ptr->to_a);
	wr_s16b(o_ptr->ac);
	wr_byte(o_ptr->dd);
	wr_byte(o_ptr->ds);

	wr_u16b(o_ptr->ident);

	wr_byte(o_ptr->marked);

	wr_byte(o_ptr->origin);
	wr_byte(o_ptr->origin_depth);
	wr_u16b(o_ptr->origin_xtra);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		wr_byte(o_ptr->flags[i]);
	if (i < OF_BYTES) pad_bytes(OF_BYTES - i);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		wr_byte(o_ptr->known_flags[i]);
	if (i < OF_BYTES) pad_bytes(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			wr_byte(o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) pad_bytes(OF_BYTES - i);
	}

	/* Held by monster index */
	wr_s16b(o_ptr->held_m_idx);

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
 * There were originally 64 bytes of randomizer saved. Now we only need
 * 32 + 5 bytes saved, so we'll write an extra 27 bytes at the end which won't
 * be used.
 */
void wr_randomizer(void)
{
	int i;

	/* current value for the simple RNG */
	wr_u32b(Rand_value);

	/* state index */
	wr_u32b(state_i);

	/* RNG variables */
	wr_u32b(z0);
	wr_u32b(z1);
	wr_u32b(z2);

	/* RNG state */
	for (i = 0; i < RAND_DEG; i++)
		wr_u32b(STATE[i]);

	/* NULL padding */
	for (i = 0; i < 59 - RAND_DEG; i++)
		wr_u32b(0);
}


/*
 * Write the "options"
 */
void wr_options(void)
{
	int i, k;

	u32b window_flag[ANGBAND_TERM_MAX];
	u32b window_mask[ANGBAND_TERM_MAX];


	/* Special Options */
	wr_byte(op_ptr->delay_factor);
	wr_byte(op_ptr->hitpoint_warn);
	wr_u16b(lazymove_delay);

	/* Normal options */
	for (i = 0; i < OPT_MAX; i++) {
		const char *name = option_name(i);
		if (!name)
			continue;

		wr_string(name);
		wr_byte(op_ptr->opt[i]);
   }

	/* Sentinel */
	wr_byte(0);

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
	for (i = 0; i < ANGBAND_TERM_MAX; i++) wr_u32b(window_flag[i]);

	/* Dump the masks */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) wr_u32b(window_mask[i]);
}


void wr_messages(void)
{
	s16b i;
	u16b num;

	num = messages_num();
	if (num > 80) num = 80;
	wr_u16b(num);

	/* Dump the messages (oldest first!) */
	for (i = num - 1; i >= 0; i--)
	{
		wr_string(message_str(i));
		wr_u16b(message_type(i));
	}
}


void wr_monster_memory(void)
{
	size_t i;
	int r_idx;

	wr_u16b(z_info->r_max);
	for (r_idx = 0; r_idx < z_info->r_max; r_idx++)
	{
		monster_race *r_ptr = &r_info[r_idx];
		monster_lore *l_ptr = &l_list[r_idx];

		/* Count sights/deaths/kills */
		wr_s16b(l_ptr->sights);
		wr_s16b(l_ptr->deaths);
		wr_s16b(l_ptr->pkills);
		wr_s16b(l_ptr->tkills);

		/* Count wakes and ignores */
		wr_byte(l_ptr->wake);
		wr_byte(l_ptr->ignore);

		/* Count drops */
		wr_byte(l_ptr->drop_gold);
		wr_byte(l_ptr->drop_item);

		/* Count spells */
		wr_byte(l_ptr->cast_innate);
		wr_byte(l_ptr->cast_spell);

		/* Count blows of each type */
		for (i = 0; i < MONSTER_BLOW_MAX; i++)
			wr_byte(l_ptr->blows[i]);

		/* Memorize flags */
		for (i = 0; i < RF_BYTES && i < RF_SIZE; i++)
			wr_byte(l_ptr->flags[i]);
		if (i < RF_BYTES) pad_bytes(RF_BYTES - i);

		for (i = 0; i < RF_BYTES && i < RSF_SIZE; i++)
			wr_byte(l_ptr->spell_flags[i]);
		if (i < RF_BYTES) pad_bytes(RF_BYTES - i);

		/* Monster limit per level */
		wr_byte(r_ptr->max_num);

		/* XXX */
		wr_byte(0);
		wr_byte(0);
		wr_byte(0);
	}
}


void wr_object_memory(void)
{
	int k_idx;

	wr_u16b(z_info->k_max);
	for (k_idx = 0; k_idx < z_info->k_max; k_idx++)
	{
		byte tmp8u = 0;
		object_kind *k_ptr = &k_info[k_idx];

		if (k_ptr->aware) tmp8u |= 0x01;
		if (k_ptr->tried) tmp8u |= 0x02;
		if (kind_is_squelched_aware(k_ptr)) tmp8u |= 0x04;
		if (k_ptr->everseen) tmp8u |= 0x08;
		if (kind_is_squelched_unaware(k_ptr)) tmp8u |= 0x10;

		wr_byte(tmp8u);
	}
}


void wr_quests(void)
{
	int i;
	u16b tmp16u;

	/* Hack -- Dump the quests */
	tmp16u = MAX_Q_IDX;
	wr_u16b(tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		wr_byte(q_list[i].level);
		wr_byte(0);
		wr_byte(0);
		wr_byte(0);
	}
}


void wr_artifacts(void)
{
	int i;
	u16b tmp16u;

	/* Hack -- Dump the artifacts */
	tmp16u = z_info->a_max;
	wr_u16b(tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		wr_byte(a_ptr->created);
		wr_byte(a_ptr->seen);
		wr_byte(a_ptr->everseen);
		wr_byte(0);
	}
}


void wr_player(void)
{
	int i;

	wr_string(op_ptr->full_name);

	wr_string(p_ptr->died_from);

	wr_string(p_ptr->history);

	/* Race/Class/Gender/Spells */
	wr_byte(p_ptr->race->ridx);
	wr_byte(p_ptr->class->cidx);
	wr_byte(p_ptr->psex);
	wr_byte(op_ptr->name_suffix);

	wr_byte(p_ptr->hitdie);
	wr_byte(p_ptr->expfact);

	wr_s16b(p_ptr->age);
	wr_s16b(p_ptr->ht);
	wr_s16b(p_ptr->wt);

	/* Dump the stats (maximum and current and birth) */
	for (i = 0; i < A_MAX; ++i) wr_s16b(p_ptr->stat_max[i]);
	for (i = 0; i < A_MAX; ++i) wr_s16b(p_ptr->stat_cur[i]);
	for (i = 0; i < A_MAX; ++i) wr_s16b(p_ptr->stat_birth[i]);

	wr_s16b(p_ptr->ht_birth);
	wr_s16b(p_ptr->wt_birth);
	wr_s16b(p_ptr->sc_birth);
	wr_u32b(p_ptr->au_birth);

	/* Padding */
	wr_u32b(0);

	wr_u32b(p_ptr->au);


	wr_u32b(p_ptr->max_exp);
	wr_u32b(p_ptr->exp);
	wr_u16b(p_ptr->exp_frac);
	wr_s16b(p_ptr->lev);

	wr_s16b(p_ptr->mhp);
	wr_s16b(p_ptr->chp);
	wr_u16b(p_ptr->chp_frac);

	wr_s16b(p_ptr->msp);
	wr_s16b(p_ptr->csp);
	wr_u16b(p_ptr->csp_frac);

	/* Max Player and Dungeon Levels */
	wr_s16b(p_ptr->max_lev);
	wr_s16b(p_ptr->max_depth);

	/* More info */
	wr_s16b(0);	/* oops */
	wr_s16b(0);	/* oops */
	wr_s16b(0);	/* oops */
	wr_s16b(0);	/* oops */
	wr_s16b(p_ptr->sc);
	wr_s16b(0);	/* oops */

	wr_s16b(p_ptr->food);
	wr_s16b(p_ptr->energy);
	wr_s16b(p_ptr->word_recall);
	wr_s16b(p_ptr->state.see_infra);
	wr_byte(p_ptr->confusing);
	wr_byte(p_ptr->searching);

	/* Find the number of timed effects */
	wr_byte(TMD_MAX);

	/* Read all the effects, in a loop */
	for (i = 0; i < TMD_MAX; i++)
		wr_s16b(p_ptr->timed[i]);

	/* Total energy used so far */
	wr_u32b(p_ptr->total_energy);
	/* # of turns spent resting */
	wr_u32b(p_ptr->resting_turn);

	/* Future use */
	for (i = 0; i < 8; i++) wr_u32b(0L);
}


void wr_squelch(void)
{
	size_t i, n;

	/* Write number of squelch bytes */
	wr_byte(squelch_size);
	for (i = 0; i < squelch_size; i++)
		wr_byte(squelch_level[i]);

	/* Write ego-item squelch bits */
	wr_u16b(z_info->e_max);
	for (i = 0; i < z_info->e_max; i++)
	{
		byte flags = 0;

		/* Figure out and write the everseen flag */
		if (e_info[i].everseen) flags |= 0x02;
		wr_byte(flags);
	}

	n = 0;
	for (i = 0; i < z_info->k_max; i++)
		if (k_info[i].note)
			n++;

	/* Write the current number of auto-inscriptions */
	wr_u16b(n);

	/* Write the autoinscriptions array */
	for (i = 0; i < z_info->k_max; i++) {
		if (!k_info[i].note)
			continue;
		wr_s16b(i);
		wr_string(quark_str(k_info[i].note));
	}

	return;
}


void wr_misc(void)
{

	/* XXX Old random artifact version, remove after 3.3 */
	wr_u32b(63);

	/* Random artifact seed */
	wr_u32b(seed_randart);


	/* XXX Ignore some flags */
	wr_u32b(0L);
	wr_u32b(0L);
	wr_u32b(0L);


	/* Write the "object seeds" */
	wr_u32b(seed_flavor);
	wr_u32b(seed_town);


	/* Special stuff */
	wr_u16b(p_ptr->panic_save);
	wr_u16b(p_ptr->total_winner);
	wr_u16b(p_ptr->noscore);


	/* Write death */
	wr_byte(p_ptr->is_dead);

	/* Write feeling */
	wr_byte(cave->feeling);
	wr_s32b(cave->created_at);

	/* Current turn */
	wr_s32b(turn);
}


void wr_player_hp(void)
{
	int i;

	wr_u16b(PY_MAX_LEVEL);
	for (i = 0; i < PY_MAX_LEVEL; i++)
		wr_s16b(p_ptr->player_hp[i]);
}


void wr_player_spells(void)
{
	int i;

	wr_u16b(PY_MAX_SPELLS);

	for (i = 0; i < PY_MAX_SPELLS; i++)
		wr_byte(p_ptr->spell_flags[i]);

	for (i = 0; i < PY_MAX_SPELLS; i++)
		wr_byte(p_ptr->spell_order[i]);
}


/*
 * Dump the random artifacts
 */
void wr_randarts(void)
{
	size_t i, j, k;

	if (!OPT(birth_randarts))
		return;

	wr_u16b(z_info->a_max);

	for (i = 0; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];

		wr_byte(a_ptr->tval);
		wr_byte(a_ptr->sval);
		for (j = 0; j < MAX_PVALS; j++)
			wr_s16b(a_ptr->pval[j]);
		wr_byte(a_ptr->num_pvals);

		wr_s16b(a_ptr->to_h);
		wr_s16b(a_ptr->to_d);
		wr_s16b(a_ptr->to_a);
		wr_s16b(a_ptr->ac);

		wr_byte(a_ptr->dd);
		wr_byte(a_ptr->ds);

		wr_s16b(a_ptr->weight);

		wr_s32b(a_ptr->cost);

		for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
			wr_byte(a_ptr->flags[j]);
		if (j < OF_BYTES) pad_bytes(OF_BYTES - j);

		for (k = 0; k < MAX_PVALS; k++) {
			for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
				wr_byte(a_ptr->pval_flags[k][j]);
			if (j < OF_BYTES) pad_bytes(OF_BYTES - j);
		}

		wr_byte(a_ptr->level);
		wr_byte(a_ptr->rarity);
		wr_byte(a_ptr->alloc_prob);
		wr_byte(a_ptr->alloc_min);
		wr_byte(a_ptr->alloc_max);

		wr_u16b(a_ptr->effect);
		wr_u16b(a_ptr->time.base);
		wr_u16b(a_ptr->time.dice);
		wr_u16b(a_ptr->time.sides);
	}
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
		wr_u16b((u16b)i);

		/* Dump object */
		wr_item(o_ptr);
	}

	/* Add a sentinel */
	wr_u16b(0xFFFF);
}


void wr_stores(void)
{
	int i;

	wr_u16b(MAX_STORES);
	for (i = 0; i < MAX_STORES; i++)
	{
		const struct store *st_ptr = &stores[i];
		int j;

		/* XXX Old values */
		wr_u32b(0L);
		wr_s16b(0);

		/* Save the current owner */
		wr_byte(st_ptr->owner->oidx);

		/* Save the stock size */
		wr_byte(st_ptr->stock_num);

		/* XXX Old values */
		wr_s16b(0);
		wr_s16b(0);

		/* Save the stock */
		for (j = 0; j < st_ptr->stock_num; j++)
			wr_item(&st_ptr->stock[j]);
	}
}



/*
 * The cave grid flags that get saved in the savefile
 */
#define IMPORTANT_FLAGS (CAVE_MARK | CAVE_GLOW | CAVE_ICKY | CAVE_ROOM)


/*
 * Write the current dungeon
 */
void wr_dungeon(void)
{
	int y, x;

	byte tmp8u;

	byte count;
	byte prev_char;


	if (p_ptr->is_dead)
		return;

	/*** Basic info ***/

	/* Dungeon specific info follows */
	wr_u16b(p_ptr->depth);
	wr_u16b(daycount);
	wr_u16b(p_ptr->py);
	wr_u16b(p_ptr->px);
	wr_u16b(cave->height);
	wr_u16b(cave->width);
	wr_u16b(0);
	wr_u16b(0);


	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Note that this will induce two wasted bytes */
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
				wr_byte((byte)count);
				wr_byte((byte)prev_char);
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
		wr_byte((byte)count);
		wr_byte((byte)prev_char);
	}

	/** Now dump the cave->info2[][] stuff **/

	/* Note that this will induce two wasted bytes */
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
				wr_byte((byte)count);
				wr_byte((byte)prev_char);
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
		wr_byte((byte)count);
		wr_byte((byte)prev_char);
	}


	/*** Simple "Run-Length-Encoding" of cave ***/

	/* Note that this will induce two wasted bytes */
	count = 0;
	prev_char = 0;

	/* Dump the cave */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Extract a byte */
			tmp8u = cave->feat[y][x];

			/* If the run is broken, or too full, flush it */
			if ((tmp8u != prev_char) || (count == MAX_UCHAR))
			{
				wr_byte((byte)count);
				wr_byte((byte)prev_char);
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
		wr_byte((byte)count);
		wr_byte((byte)prev_char);
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
	wr_u16b(o_max);

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
	wr_u16b(cave_monster_max(cave));

	/* Dump the monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		byte unaware = 0;
	
		const monster_type *m_ptr = cave_monster(cave, i);

		wr_s16b(m_ptr->r_idx);
		wr_byte(m_ptr->fy);
		wr_byte(m_ptr->fx);
		wr_s16b(m_ptr->hp);
		wr_s16b(m_ptr->maxhp);
		wr_byte(m_ptr->mspeed);
		wr_byte(m_ptr->energy);
		wr_byte(MON_TMD_MAX);

		for (j = 0; j < MON_TMD_MAX; j++)
			wr_s16b(m_ptr->m_timed[j]);

		if (m_ptr->unaware) unaware |= 0x01;
		wr_byte(unaware);

		for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
			wr_byte(m_ptr->known_pflags[j]);
		if (j < OF_BYTES) pad_bytes(OF_BYTES - j);

		wr_byte(0);
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
	for (i = 0; i < 60; i++) wr_byte(0);
}


void wr_history(void)
{
	size_t i;
	u32b tmp32u = history_get_num();

	wr_u32b(tmp32u);
	for (i = 0; i < tmp32u; i++)
	{
		wr_u16b(history_list[i].type);
		wr_s32b(history_list[i].turn);
		wr_s16b(history_list[i].dlev);
		wr_s16b(history_list[i].clev);
		wr_byte(history_list[i].a_idx);
		wr_string(history_list[i].event);
	}
}
