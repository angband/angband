/*
 * File: load.c
 * Purpose: Savefile loading functions
 *
 * Copyright (c) 1997 Ben Harrison, and others
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
#include "generate.h"
#include "history.h"
#include "init.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "monster.h"
#include "obj-identify.h"
#include "obj-make.h"
#include "obj-util.h"
#include "object.h"
#include "player.h"
#include "player-timed.h"
#include "savefile.h"
#include "squelch.h"
#include "store.h"
#include "quest.h"
#include "trap.h"
#include "ui-game.h"
#include "ui-input.h"


/* Object constants */
byte obj_mod_max = 0;
byte of_size = 0;
byte elem_max = 0;

/* Monster constants */
byte monster_blow_max = 0;
byte rf_size = 0;
byte rsf_size = 0;

/* Trap constants */
byte trf_size = 0;

/* Shorthand function pointer for rd_item version */
typedef int (*rd_item_t)(object_type *o_ptr);

/**
 * Find an ego item from its index
 */
static struct ego_item *lookup_ego(int idx)
{
	if (idx > 0 && idx < z_info->e_max)
		return &e_info[idx];

	return NULL;
}


/*
 * Read an object.
 */
static int rd_item(object_type *o_ptr)
{
	byte tmp8u;
	u16b tmp16u;
	s16b tmp16s;

	byte ego_idx;
	byte art_idx;

	size_t i;

	char buf[128];

	byte ver = 1;


	rd_u16b(&tmp16u);
	rd_byte(&ver);
	assert(tmp16u == 0xffff);

	strip_bytes(2);

	/* Location */
	rd_byte(&o_ptr->iy);
	rd_byte(&o_ptr->ix);

	/* Type/Subtype */
	rd_byte(&o_ptr->tval);
	rd_byte(&o_ptr->sval);
	rd_s16b(&o_ptr->pval);

	/* Pseudo-ID bit */
	rd_byte(&tmp8u);

	rd_byte(&o_ptr->number);
	rd_s16b(&o_ptr->weight);

	rd_byte(&art_idx);
	rd_byte(&ego_idx);

	rd_s16b(&o_ptr->timeout);

	rd_s16b(&o_ptr->to_h);
	rd_s16b(&o_ptr->to_d);
	rd_s16b(&o_ptr->to_a);

	rd_s16b(&o_ptr->ac);

	rd_byte(&o_ptr->dd);
	rd_byte(&o_ptr->ds);

	rd_u16b(&o_ptr->ident);

	rd_byte(&o_ptr->marked);

	rd_byte(&o_ptr->origin);
	rd_byte(&o_ptr->origin_depth);
	rd_u16b(&o_ptr->origin_xtra);
	rd_byte(&o_ptr->ignore);

	for (i = 0; i < of_size; i++)
		rd_byte(&o_ptr->flags[i]);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < of_size; i++)
		rd_byte(&o_ptr->known_flags[i]);

	for (i = 0; i < obj_mod_max; i++) {
		rd_s16b(&o_ptr->modifiers[i]);
	}

	/* Read brands */
	rd_byte(&tmp8u);
	while (tmp8u) {
		char buf[40];
		struct brand *b = mem_zalloc(sizeof *b);
		rd_string(buf, sizeof(buf));
		b->name = string_make(buf);
		rd_s16b(&tmp16s);
		b->element = tmp16s;
		rd_s16b(&tmp16s);
		b->multiplier = tmp16s;
		rd_byte(&tmp8u);
		b->known = tmp8u ? TRUE : FALSE;
		b->next = o_ptr->brands;
		o_ptr->brands = b;
		rd_byte(&tmp8u);
	}

	/* Read slays */
	rd_byte(&tmp8u);
	while (tmp8u) {
		char buf[40];
		struct slay *s = mem_zalloc(sizeof *s);
		rd_string(buf, sizeof(buf));
		s->name = string_make(buf);
		rd_s16b(&tmp16s);
		s->race_flag = tmp16s;
		rd_s16b(&tmp16s);
		s->multiplier = tmp16s;
		rd_byte(&tmp8u);
		s->known = tmp8u ? TRUE : FALSE;
		s->next = o_ptr->slays;
		o_ptr->slays = s;
		rd_byte(&tmp8u);
	}

	for (i = 0; i < elem_max; i++) {
		rd_s16b(&o_ptr->el_info[i].res_level);
		rd_byte(&o_ptr->el_info[i].flags);
	}

	/* Monster holding object */
	rd_s16b(&o_ptr->held_m_idx);

	rd_s16b(&o_ptr->mimicking_m_idx);

	/* Save the inscription */
	rd_string(buf, sizeof(buf));
	if (buf[0]) o_ptr->note = quark_add(buf);


	/* Lookup item kind */
	o_ptr->kind = lookup_kind(o_ptr->tval, o_ptr->sval);
	if (!o_ptr->kind)
		return 0;

	o_ptr->ego = lookup_ego(ego_idx);

	if (art_idx >= z_info->a_max)
		return -1;
	if (art_idx > 0)
		o_ptr->artifact = &a_info[art_idx];

	/* Success */
	return (0);
}


/**
 * Read a monster
 */
static void rd_monster(monster_type * m_ptr)
{
	byte tmp8u, flags;
	s16b r_idx;
	size_t j;

	/* Read the monster race */
	rd_s16b(&r_idx);
	m_ptr->race = &r_info[r_idx];

	/* Read the other information */
	rd_byte(&m_ptr->fy);
	rd_byte(&m_ptr->fx);
	rd_s16b(&m_ptr->hp);
	rd_s16b(&m_ptr->maxhp);
	rd_byte(&m_ptr->mspeed);
	rd_byte(&m_ptr->energy);
	rd_byte(&tmp8u);

	for (j = 0; j < tmp8u; j++)
		rd_s16b(&m_ptr->m_timed[j]);

	/* Read and extract the flag */
	rd_byte(&flags);
	m_ptr->unaware = (flags & 0x01) ? TRUE : FALSE;

	for (j = 0; j < of_size; j++)
		rd_byte(&m_ptr->known_pflags[j]);

	strip_bytes(1);

}


/**
 * Read a trap record
 */
static void rd_trap(trap_type *t_ptr)
{
    int i;

    rd_byte(&t_ptr->t_idx);
    t_ptr->kind = &trap_info[t_ptr->t_idx];
    rd_byte(&t_ptr->fy);
    rd_byte(&t_ptr->fx);
    rd_byte(&t_ptr->xtra);

    for (i = 0; i < trf_size; i++)
	rd_byte(&t_ptr->flags[i]);
}

/**
 * Read RNG state
 *
 * There were originally 64 bytes of randomizer saved. Now we only need
 * 32 + 5 bytes saved, so we'll read an extra 27 bytes at the end which won't
 * be used.
 */
int rd_randomizer(void)
{
	int i;
	u32b noop;

	/* current value for the simple RNG */
	rd_u32b(&Rand_value);

	/* state index */
	rd_u32b(&state_i);

	/* for safety, make sure state_i < RAND_DEG */
	state_i = state_i % RAND_DEG;
    
	/* RNG variables */
	rd_u32b(&z0);
	rd_u32b(&z1);
	rd_u32b(&z2);
    
	/* RNG state */
	for (i = 0; i < RAND_DEG; i++)
		rd_u32b(&STATE[i]);

	/* NULL padding */
	for (i = 0; i < 59 - RAND_DEG; i++)
		rd_u32b(&noop);

	Rand_quick = FALSE;

	return 0;
}


/*
 * Read options.
 */
int rd_options(void)
{
	int i, n;

	byte b;

	u16b tmp16u;

	u32b window_flag[ANGBAND_TERM_MAX];
	u32b window_mask[ANGBAND_TERM_MAX];


	/*** Special info */

	/* Read "delay_factor" */
	rd_byte(&b);
	op_ptr->delay_factor = b;

	/* Read "hitpoint_warn" */
	rd_byte(&b);
	op_ptr->hitpoint_warn = b;

	/* Read lazy movement delay */
	rd_u16b(&tmp16u);
	lazymove_delay = (tmp16u < 1000) ? tmp16u : 0;


	/*** Normal Options ***/

	while (1) {
		byte value;
		char name[20];
		rd_string(name, sizeof name);

		if (!name[0])
			break;

		rd_byte(&value);
		option_set(name, !!value);
	}

	/*** Window Options ***/

	for (n = 0; n < ANGBAND_TERM_MAX; n++)
		rd_u32b(&window_flag[n]);
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
		rd_u32b(&window_mask[n]);

	/* Analyze the options */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
	{
		/* Analyze the options */
		for (i = 0; i < 32; i++)
		{
			/* Process valid flags */
			if (window_flag_desc[i])
			{
				/* Blank invalid flags */
				if (!(window_mask[n] & (1L << i)))
				{
					window_flag[n] &= ~(1L << i);
				}
			}
		}
	}

	/* Set up the subwindows */
	subwindows_set_flags(window_flag, ANGBAND_TERM_MAX);

	return 0;
}

/*
 * Read the saved messages
 */
int rd_messages(void)
{
	int i;
	char buf[128];
	u16b tmp16u;
	
	s16b num;
	
	/* Total */
	rd_s16b(&num);
	
	/* Read the messages */
	for (i = 0; i < num; i++)
	{
		/* Read the message */
		rd_string(buf, sizeof(buf));
		
		/* Read the message type */
		rd_u16b(&tmp16u);

		/* Save the message */
		message_add(buf, tmp16u);
	}
	
	return 0;
}

/* Read monster memory. */
int rd_monster_memory(void)
{
	int r_idx;
	u16b tmp16u;

	/* Monster Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > z_info->r_max)
	{
		note(format("Too many (%u) monster races!", tmp16u));
		return (-1);
	}

	/* Monster flags */
	rd_byte(&rf_size);

	/* Incompatible save files */
	if (rf_size > RF_SIZE)
	{
	        note(format("Too many (%u) monster flags!", rf_size));
		return (-1);
	}

	/* Monster spell flags */
	rd_byte(&rsf_size);

	/* Incompatible save files */
	if (rsf_size > RSF_SIZE)
	{
	        note(format("Too many (%u) monster spell flags!", rsf_size));
		return (-1);
	}

	/* Monster blows */
	rd_byte(&monster_blow_max);

	/* Incompatible save files */
	if (monster_blow_max > MONSTER_BLOW_MAX)
	{
	        note(format("Too many (%u) monster blows!", monster_blow_max));
		return (-1);
	}

	/* Read the available records */
	for (r_idx = 0; r_idx < tmp16u; r_idx++)
	{
		size_t i;

		monster_race *r_ptr = &r_info[r_idx];
		monster_lore *l_ptr = &l_list[r_idx];


		/* Count sights/deaths/kills */
		rd_s16b(&l_ptr->sights);
		rd_s16b(&l_ptr->deaths);
		rd_s16b(&l_ptr->pkills);
		rd_s16b(&l_ptr->tkills);

		/* Count wakes and ignores */
		rd_byte(&l_ptr->wake);
		rd_byte(&l_ptr->ignore);

		/* Count drops */
		rd_byte(&l_ptr->drop_gold);
		rd_byte(&l_ptr->drop_item);

		/* Count spells */
		rd_byte(&l_ptr->cast_innate);
		rd_byte(&l_ptr->cast_spell);

		/* Count blows of each type */
		for (i = 0; i < monster_blow_max; i++)
			rd_byte(&l_ptr->blows[i]);

		/* Memorize flags */
		for (i = 0; i < rf_size; i++)
			rd_byte(&l_ptr->flags[i]);

		for (i = 0; i < rsf_size; i++)
			rd_byte(&l_ptr->spell_flags[i]);

		/* Read the "Racial" monster limit per level */
		rd_byte(&r_ptr->max_num);

		/* XXX */
		strip_bytes(3);

		/* Repair the spell lore flags */
		rsf_inter(l_ptr->spell_flags, r_ptr->spell_flags);
	}

	return 0;
}




int rd_object_memory(void)
{
	int i;
	u16b tmp16u;

	/* Object Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > z_info->k_max)
	{
		note(format("Too many (%u) object kinds!", tmp16u));
		return (-1);
	}

	/* Object flags */
	rd_byte(&of_size);

	/* Incompatible save files */
	if (of_size > OF_SIZE)
	{
	        note(format("Too many (%u) object flags!", of_size));
		return (-1);
	}

	/* Object modifiers */
	rd_byte(&obj_mod_max);

	/* Incompatible save files */
	if (obj_mod_max > OBJ_MOD_MAX)
	{
	        note(format("Too many (%u) object modifiers allowed!", 
						obj_mod_max));
		return (-1);
	}

	/* Elements */
	rd_byte(&elem_max);

	/* Incompatible save files */
	if (elem_max > ELEM_MAX)
	{
	        note(format("Too many (%u) elements allowed!", 
						elem_max));
		return (-1);
	}

	/* Read the object memory */
	for (i = 0; i < tmp16u; i++)
	{
		byte tmp8u;
		object_kind *k_ptr = &k_info[i];

		rd_byte(&tmp8u);

		k_ptr->aware = (tmp8u & 0x01) ? TRUE : FALSE;
		k_ptr->tried = (tmp8u & 0x02) ? TRUE : FALSE;
		k_ptr->everseen = (tmp8u & 0x08) ? TRUE : FALSE;

		if (tmp8u & 0x04) kind_squelch_when_aware(k_ptr);
		if (tmp8u & 0x10) kind_squelch_when_unaware(k_ptr);
	}

	return 0;
}



int rd_quests(void)
{
	int i;
	u16b tmp16u;
	
	/* Load the Quests */
	rd_u16b(&tmp16u);
	
	/* Incompatible save files */
	if (tmp16u > MAX_Q_IDX)
	{
		note(format("Too many (%u) quests!", tmp16u));
		return (-1);
	}
	
	/* Load the Quests */
	for (i = 0; i < tmp16u; i++)
	{
		byte tmp8u;
		
		rd_byte(&tmp8u);
		q_list[i].level = tmp8u;
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
	}
	
	return 0;
}


int rd_artifacts(void)
{
	int i;
	u16b tmp16u;
	
	/* Load the Artifacts */
	rd_u16b(&tmp16u);
	
	/* Incompatible save files */
	if (tmp16u > z_info->a_max)
	{
		note(format("Too many (%u) artifacts!", tmp16u));
		return (-1);
	}
	
	/* Read the artifact flags */
	for (i = 0; i < tmp16u; i++)
	{
		byte tmp8u;
		
		rd_byte(&tmp8u);
		a_info[i].created = tmp8u;
		rd_byte(&tmp8u);
		a_info[i].seen = tmp8u;
		rd_byte(&tmp8u);
		a_info[i].everseen = tmp8u;
		rd_byte(&tmp8u);
	}

	return 0;
}



/*
 * Read the player information
 */
int rd_player(void)
{
	int i;
	byte num;
	byte a_max = 0;

	rd_string(op_ptr->full_name, sizeof(op_ptr->full_name));
	rd_string(player->died_from, 80);
	player->history = mem_zalloc(250);
	rd_string(player->history, 250);

	/* Player race */
	rd_byte(&num);
	player->race = player_id2race(num);

	/* Verify player race */
	if (!player->race) {
		note(format("Invalid player race (%d).", num));
		return -1;
	}

	/* Player class */
	rd_byte(&num);
	player->class = player_id2class(num);

	if (!player->class) {
		note(format("Invalid player class (%d).", num));
		return -1;
	}

	/* Player gender */
	rd_byte(&player->psex);
	player->sex = &sex_info[player->psex];

	/* Numeric name suffix */
	rd_byte(&op_ptr->name_suffix);

	/* Special Race/Class info */
	rd_byte(&player->hitdie);
	rd_byte(&player->expfact);

	/* Age/Height/Weight */
	rd_s16b(&player->age);
	rd_s16b(&player->ht);
	rd_s16b(&player->wt);

	/* Read the stat info */
	rd_byte(&a_max);
	assert(a_max <= A_MAX);
	for (i = 0; i < a_max; i++) rd_s16b(&player->stat_max[i]);
	for (i = 0; i < a_max; i++) rd_s16b(&player->stat_cur[i]);
	for (i = 0; i < a_max; i++) rd_s16b(&player->stat_birth[i]);

	rd_s16b(&player->ht_birth);
	rd_s16b(&player->wt_birth);
	strip_bytes(2);
	rd_s32b(&player->au_birth);

	strip_bytes(4);

	rd_s32b(&player->au);

	rd_s32b(&player->max_exp);
	rd_s32b(&player->exp);
	rd_u16b(&player->exp_frac);

	rd_s16b(&player->lev);

	/* Verify player level */
	if ((player->lev < 1) || (player->lev > PY_MAX_LEVEL))
	{
		note(format("Invalid player level (%d).", player->lev));
		return (-1);
	}

	rd_s16b(&player->mhp);
	rd_s16b(&player->chp);
	rd_u16b(&player->chp_frac);

	rd_s16b(&player->msp);
	rd_s16b(&player->csp);
	rd_u16b(&player->csp_frac);

	rd_s16b(&player->max_lev);
	rd_s16b(&player->max_depth);

	/* Hack -- Repair maximum player level */
	if (player->max_lev < player->lev) player->max_lev = player->lev;

	/* Hack -- Repair maximum dungeon level */
	if (player->max_depth < 0) player->max_depth = 1;

	/* More info */
	strip_bytes(9);
	rd_byte(&player->unignoring);
	rd_s16b(&player->deep_descent);

	/* Read the flags */
	rd_s16b(&player->food);
	rd_s16b(&player->energy);
	rd_s16b(&player->word_recall);
	rd_byte(&player->confusing);
	rd_byte(&player->searching);

	/* Find the number of timed effects */
	rd_byte(&num);

	if (num <= TMD_MAX)
	{
		/* Read all the effects */
		for (i = 0; i < num; i++)
			rd_s16b(&player->timed[i]);

		/* Initialize any entries not read */
		if (num < TMD_MAX)
			C_WIPE(player->timed + num, TMD_MAX - num, s16b);
	}
	else
	{
		/* Probably in trouble anyway */
		for (i = 0; i < TMD_MAX; i++)
			rd_s16b(&player->timed[i]);

		/* Discard unused entries */
		strip_bytes(2 * (num - TMD_MAX));
		note("Discarded unsupported timed effects");
	}

	/* Total energy used so far */
	rd_u32b(&player->total_energy);
	/* # of turns spent resting */
	rd_u32b(&player->resting_turn);

	/* Future use */
	strip_bytes(32);

	return 0;
}


/*
 * Read squelch and autoinscription submenu for all known objects
 */
int rd_squelch(void)
{
	size_t i;
	byte tmp8u = 24;
	u16b file_e_max;
	u16b inscriptions;
	
	/* Read how many squelch bytes we have */
	rd_byte(&tmp8u);
	
	/* Check against current number */
	if (tmp8u != squelch_size)
	{
		strip_bytes(tmp8u);
	}
	else
	{
		for (i = 0; i < squelch_size; i++)
			rd_byte(&squelch_level[i]);
	}
		
	/* Read the number of saved ego-item */
	rd_u16b(&file_e_max);
		
	for (i = 0; i < file_e_max; i++)
	{
		if (i < z_info->e_max)
		{
			byte flags;
			
			/* Read and extract the flag */
			rd_byte(&flags);
			e_info[i].everseen |= (flags & 0x02);
		}
	}
	
	/* Read the current number of auto-inscriptions */
	rd_u16b(&inscriptions);
	
	/* Read the autoinscriptions array */
	for (i = 0; i < inscriptions; i++)
	{
		char tmp[80];
		s16b kidx;
		struct object_kind *k;
		
		rd_s16b(&kidx);
		k = objkind_byid(kidx);
		if (!k)
			quit_fmt("objkind_byid(%d) failed", kidx);
		rd_string(tmp, sizeof(tmp));
		k->note = quark_add(tmp);
	}
	
	return 0;
}


int rd_misc(void)
{
	byte tmp8u;
	u16b tmp16u;
	
	/* Read the randart version */
	strip_bytes(4);

	/* Read the randart seed */
	rd_u32b(&seed_randart);

	/* Skip the flags */
	strip_bytes(12);


	/* Hack -- the two "special seeds" */
	rd_u32b(&seed_flavor);


	/* Special stuff */
	rd_u16b(&player->total_winner);
	rd_u16b(&player->noscore);


	/* Read "death" */
	rd_byte(&tmp8u);
	player->is_dead = tmp8u;

	/* Read "feeling" */
	rd_byte(&tmp8u);
	cave->feeling = tmp8u;
	rd_u16b(&tmp16u);
	cave->feeling_squares = tmp16u;

	rd_s32b(&cave->created_at);

	/* Current turn */
	rd_s32b(&turn);

	return 0;
}

int rd_player_hp(void)
{
	int i;
	u16b tmp16u;

	/* Read the player_hp array */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > PY_MAX_LEVEL)
	{
		note(format("Too many (%u) hitpoint entries!", tmp16u));
		return (-1);
	}

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
		rd_s16b(&player->player_hp[i]);

	return 0;
}


int rd_player_spells(void)
{
	int i;
	u16b tmp16u;
	
	int cnt;
	
	/* Read the number of spells */
	rd_u16b(&tmp16u);
	if (tmp16u > PY_MAX_SPELLS)
	{
		note(format("Too many player spells (%d).", tmp16u));
		return (-1);
	}
	
	/* Read the spell flags */
	for (i = 0; i < tmp16u; i++)
		rd_byte(&player->spell_flags[i]);
	
	/* Read the spell order */
	for (i = 0, cnt = 0; i < tmp16u; i++, cnt++)
		rd_byte(&player->spell_order[cnt]);
	
	/* Success */
	return (0);
}


/**
 * Read the player inventory
 *
 * Note that the inventory is re-sorted later by dungeon().
 */
static int rd_inventory_aux(rd_item_t rd_item_version)
{
	int slot = 0;

	object_type *i_ptr;
	object_type object_type_body;

	player->upkeep->total_weight = 0;

	/* Read until done */
	while (1)
	{
		u16b n;

		/* Get the next item index */
		rd_u16b(&n);

		/* Nope, we reached the end */
		if (n == 0xFFFF) break;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Read the item */
		if ((*rd_item_version)(i_ptr))
		{
			note("Error reading item");
			return (-1);
		}

		/* Hack -- verify item */
		if (!i_ptr->kind) continue;

		/* Verify slot */
		if (n >= ALL_INVEN_TOTAL) return (-1);

		/* Wield equipment */
		if (n >= INVEN_WIELD)
		{
			/* Copy object */
			object_copy(&player->inventory[n], i_ptr);

			/* Add the weight */
			player->upkeep->total_weight += (i_ptr->number * i_ptr->weight);

			/* One more item */
			player->upkeep->equip_cnt++;
		}

		/* Warning -- backpack is full */
		else if (player->upkeep->inven_cnt == INVEN_PACK)
		{
			/* Oops */
			note("Too many items in the inventory!");

			/* Fail */
			return (-1);
		}

		/* Carry inventory */
		else
		{
			/* Get a slot */
			n = slot++;

			/* Copy object */
			object_copy(&player->inventory[n], i_ptr);

			/* Add the weight */
			player->upkeep->total_weight += (i_ptr->number * i_ptr->weight);

			/* One more item */
			player->upkeep->inven_cnt++;
		}
	}

	save_quiver_size(player);

	/* Success */
	return (0);
}

/*
 * Read the player inventory - wrapper functions
 */
int rd_inventory(void) { return rd_inventory_aux(rd_item); }


/* Read store contents */
static int rd_stores_aux(rd_item_t rd_item_version)
{
	int i;
	u16b tmp16u;
	
	/* Read the stores */
	rd_u16b(&tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		struct store *st_ptr = &stores[i];

		int j;		
		byte own, num;
		
		/* XXX */
		strip_bytes(6);
		
		/* Read the basic info */
		rd_byte(&own);
		rd_byte(&num);
		
		/* XXX */
		strip_bytes(4);
		
		/* XXX: refactor into store.c */
		st_ptr->owner = store_ownerbyidx(st_ptr, own);
		
		/* Read the items */
		for (j = 0; j < num; j++)
		{
			object_type *i_ptr;
			object_type object_type_body;
			
			/* Get local object */
			i_ptr = &object_type_body;
			
			/* Wipe the object */
			object_wipe(i_ptr);
			
			/* Read the item */
			if ((*rd_item_version)(i_ptr))
			{
				note("Error reading item");
				return (-1);
			}

			if (i != STORE_HOME)
				i_ptr->ident |= IDENT_STORE;
			
			/* Accept any valid items */
			if (st_ptr->stock_num < STORE_INVEN_MAX && i_ptr->kind)
			{
				int k = st_ptr->stock_num++;

				/* Accept the item */
				object_copy(&st_ptr->stock[k], i_ptr);
			}
		}
	}

	return 0;
}

/*
 * Read the stores - wrapper functions
 */
int rd_stores(void) { return rd_stores_aux(rd_item); }


/*
 * Read the dungeon
 *
 * The monsters/objects must be loaded in the same order
 * that they were stored, since the actual indexes matter.
 *
 * Note that the size of the dungeon is now the currrent dimensions of the
 * cave global variable.
 *
 * Note that dungeon objects, including objects held by monsters, are
 * placed directly into the dungeon, using "object_copy()", which will
 * copy "iy", "ix", and "held_m_idx", leaving "next_o_idx" blank for
 * objects held by monsters, since it is not saved in the savefile.
 *
 * After loading the monsters, the objects being held by monsters are
 * linked directly into those monsters.
 */
int rd_dungeon(void)
{
	int i, n, y, x;

	s16b depth;
	s16b py, px;
	s16b ymax, xmax;

	byte count;
	byte tmp8u;
	u16b tmp16u, square_size;

	/* Only if the player's alive */
	if (player->is_dead)
		return 0;

	/*** Basic info ***/

	/* Header info */
	rd_s16b(&depth);
	rd_u16b(&daycount);
	rd_s16b(&py);
	rd_s16b(&px);
	rd_s16b(&ymax);
	rd_s16b(&xmax);
    rd_u16b(&square_size);
	rd_u16b(&tmp16u);


    /* Always at least two bytes of cave_info */
    square_size = MAX(2, square_size);
  
	/* Ignore illegal dungeons */
	if ((depth < 0) || (depth >= MAX_DEPTH))
	{
		note(format("Ignoring illegal dungeon depth (%d)", depth));
		return (0);
	}

	cave->width = xmax;
	cave->height = ymax;

	/* Ignore illegal dungeons */
	if ((px < 0) || (px >= cave->width) ||
	    (py < 0) || (py >= cave->height))
	{
		note(format("Ignoring illegal player location (%d,%d).", py, px));
		return (1);
	}


	/*** Run length decoding ***/

    /* Loop across bytes of cave_info */
    for (n = 0; n < square_size; n++)
    {
	/* Load the dungeon data */
	for (x = y = 0; y < cave->height; )
	{
		/* Grab RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			/* Extract "info" */
			cave->info[y][x][n] = tmp8u;

			/* Advance/Wrap */
			if (++x >= cave->width)
			{
				/* Wrap */
				x = 0;

				/* Advance/Wrap */
				if (++y >= cave->height) break;
			}
		}
	}
	}


	/*** Run length decoding ***/

	/* Load the dungeon data */
	for (x = y = 0; y < cave->height; )
	{
		/* Grab RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			/* Extract "feat" */
			square_set_feat(cave, y, x, tmp8u);

			/* Advance/Wrap */
			if (++x >= cave->width)
			{
				/* Wrap */
				x = 0;

				/* Advance/Wrap */
				if (++y >= cave->height) break;
			}
		}
	}


	/*** Player ***/

	/* Load depth */
	player->depth = depth;


	/* Place player in dungeon */
	player_place(cave, player, py, px);

	/*** Success ***/

	/* The dungeon is ready */
	character_dungeon = TRUE;

#if 0
	/* Regenerate town in old versions */
	if (player->depth == 0)
		character_dungeon = FALSE;
#endif

	return 0;
}

/*
 * Read the chunk list
 */
int rd_chunks(void)
{
	int y, x;
	int i, j;
	size_t k;
	u16b chunk_max;

	byte tmp8u;

	byte count;


	if (player->is_dead)
		return 0;

	rd_u16b(&chunk_max);
	for (j = 0; j < chunk_max; j++) {
		struct chunk *c;
		char name[100];
		u16b height, width;

		/* Read the name and dimensions */
		rd_string(name, sizeof(name));
		rd_u16b(&height);
		rd_u16b(&width);
		c = cave_new(height, width);
		c->name = string_make(name);

		/* Loop across bytes of c->info */
		for (k = 0; k < SQUARE_SIZE; k++)
		{
			/* Load the chunk data */
			for (x = y = 0; y < height; )
			{
                /* Grab RLE info */
                rd_byte(&count);
                rd_byte(&tmp8u);

                /* Apply the RLE info */
                for (i = count; i > 0; i--)
                {
					/* Extract "info" */
					c->info[y][x][k] = tmp8u;

					/* Advance/Wrap */
					if (++x >= width)
					{
						/* Wrap */
						x = 0;

						/* Advance/Wrap */
						if (++y >= height) break;
					}
                }
			}
		}

        /*** Run length decoding ***/

        /* Load the dungeon data */
        for (x = y = 0; y < height; )
        {
			/* Grab RLE info */
			rd_byte(&count);
			rd_byte(&tmp8u);

			/* Apply the RLE info */
			for (i = count; i > 0; i--)
			{
				/* Extract "feat" */
				square_set_feat(c, y, x, tmp8u);

				/* Advance/Wrap */
				if (++x >= width)
				{
					/* Wrap */
					x = 0;

					/* Advance/Wrap */
					if (++y >= height) break;
				}
			}
        }

		/* Total objects */
		rd_u16b(&c->obj_cnt);

		/* Read the objects */
		for (i = 1; i < c->obj_cnt; i++) {
			object_type *o_ptr = &c->objects[i];

			/* Read it */
			rd_item(o_ptr);
		}

		/* Total monsters */
		rd_u16b(&c->mon_cnt);

		/* Read the monsters */
		for (i = 1; i < c->mon_cnt; i++) {
			monster_type *m_ptr = &c->monsters[i];

			rd_monster(m_ptr);
		}

		/* Total traps */
		rd_u16b(&c->trap_max);

		for (i = 0; i < c->trap_max; i++) {
			trap_type *t_ptr = &c->traps[i];

			rd_trap(t_ptr);
		}
		chunk_list_add(c);
	}

	return 0;
}


/**
 * Read the floor object list
 */
static int rd_objects_aux(rd_item_t rd_item_version)
{
	int i;
	u16b limit;

	/* Only if the player's alive */
	if (player->is_dead)
		return 0;

	/* Read the item count */
	rd_u16b(&limit);

	/* Verify maximum */
	if (limit > z_info->o_max)
	{
		note(format("Too many (%d) object entries!", limit));
		return (-1);
	}

	/* Read the dungeon items */
	for (i = 1; i < limit; i++)
	{
		object_type *i_ptr;
		object_type object_type_body;

		s16b o_idx;
		object_type *o_ptr;


		/* Get the object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Read the item */
		if ((*rd_item_version)(i_ptr))
		{
			note("Error reading item");
			return (-1);
		}

		/* Make an object */
		o_idx = o_pop(cave);

		/* Paranoia */
		if (o_idx != i)
		{
			note(format("Cannot place object %d!", i));
			return (-1);
		}

		/* Get the object */
		o_ptr = cave_object(cave, o_idx);

		/* Structure Copy */
		object_copy(o_ptr, i_ptr);

		/* Dungeon floor */
		if (!i_ptr->held_m_idx)
		{
			int x = i_ptr->ix;
			int y = i_ptr->iy;

			/* ToDo: Verify coordinates */

			/* Link the object to the pile */
			o_ptr->next_o_idx = cave->o_idx[y][x];

			/* Link the floor to the object */
			cave->o_idx[y][x] = o_idx;
		}
	}

	return 0;
}

/*
 * Read the object list - wrapper functions
 */
int rd_objects(void) { return rd_objects_aux(rd_item); }

/**
 * Read monsters
 */
int rd_monsters(void)
{
	int i;
	u16b limit;

	/* Only if the player's alive */
	if (player->is_dead)
		return 0;

	/* Read the monster count */
	rd_u16b(&limit);

	/* Hack -- verify */
	if (limit > z_info->m_max)
	{
		note(format("Too many (%d) monster entries!", limit));
		return (-1);
	}

	/* Read the monsters */
	for (i = 1; i < limit; i++)
	{
		monster_type *m_ptr;
		monster_type monster_type_body;

		/* Get local monster */
		m_ptr = &monster_type_body;
		WIPE(m_ptr, monster_type);

		/* Read the monster */
		rd_monster(m_ptr);

		/* Place monster in dungeon */
		if (place_monster(cave, m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
		{
			note(format("Cannot place monster %d", i));
			return (-1);
		}
	}

	/* Reacquire objects */
	for (i = 1; i < cave_object_max(cave); ++i)
	{
		object_type *o_ptr;
		monster_type *m_ptr;

		/* Get the object */
		o_ptr = cave_object(cave, i);

		/* Check for mimics */
		if (o_ptr->mimicking_m_idx) {

			/* Verify monster index */
			if (o_ptr->mimicking_m_idx > z_info->m_max)
			{
				note("Invalid monster index");
				return (-1);
			}

			/* Get the monster */
			m_ptr = cave_monster(cave, o_ptr->mimicking_m_idx);

			/* Link the monster to the object */
			m_ptr->mimicked_o_idx = i;

		} else if (o_ptr->held_m_idx) {

			/* Verify monster index */
			if (o_ptr->held_m_idx > z_info->m_max)
			{
				note("Invalid monster index");
				return (-1);
			}

			/* Get the monster */
			m_ptr = cave_monster(cave, o_ptr->held_m_idx);

			/* Link the object to the pile */
			o_ptr->next_o_idx = m_ptr->hold_o_idx;

			/* Link the monster to the object */
			m_ptr->hold_o_idx = i;
		} else continue;
	}

	return 0;
}


int rd_history(void)
{
	u32b tmp32u;
	size_t i;
	
	history_clear();
	
	rd_u32b(&tmp32u);
	for (i = 0; i < tmp32u; i++)
	{
		s32b turnno;
		s16b dlev, clev;
		u16b type;
		byte art_name;
		char text[80];
		
		rd_u16b(&type);
		rd_s32b(&turnno);
		rd_s16b(&dlev);
		rd_s16b(&clev);
		rd_byte(&art_name);
		rd_string(text, sizeof(text));
		
		history_add_full(type, &a_info[art_name], dlev, clev, turnno, text);
	}

	return 0;
}

int rd_traps(void)
{
    int i;
    u32b tmp32u;

    rd_byte(&trf_size);
    rd_u16b(&cave->trap_max);

    for (i = 0; i < cave_trap_max(cave); i++)
    {
		trap_type *t_ptr = cave_trap(cave, i);

		rd_trap(t_ptr);
    }

    /* Expansion */
    rd_u32b(&tmp32u);

    return 0;
}

/**
 * For blocks that don't need loading anymore.
 */
int rd_null(void) {
	return 0;
}
