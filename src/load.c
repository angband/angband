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
#include "effects.h"
#include "generate.h"
#include "history.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-make.h"
#include "obj-slays.h"
#include "obj-util.h"
#include "object.h"
#include "player.h"
#include "player-spell.h"
#include "player-timed.h"
#include "savefile.h"
#include "store.h"
#include "quest.h"
#include "trap.h"
#include "ui-game.h"
#include "ui-input.h"


/* Object constants */
byte obj_mod_max = 0;
byte of_size = 0;
byte id_size = 0;
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

	for (i = 0; i < id_size; i++)
		rd_byte(&o_ptr->id_flags[i]);

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

	/* Activation */
	rd_u16b(&tmp16u);
	if (tmp16u)
		o_ptr->activation = &activations[tmp16u];
	rd_u16b(&tmp16u);
	o_ptr->time.base = tmp16u;
	rd_u16b(&tmp16u);
	o_ptr->time.dice = tmp16u;
	rd_u16b(&tmp16u);
	o_ptr->time.sides = tmp16u;

	/* Save the inscription */
	rd_string(buf, sizeof(buf));
	if (buf[0]) o_ptr->note = quark_add(buf);


	/* Lookup item kind */
	o_ptr->kind = lookup_kind(o_ptr->tval, o_ptr->sval);
	if (!o_ptr->kind)
		return 0;

	o_ptr->ego = lookup_ego(ego_idx);
	if (o_ptr->ego)
		o_ptr->effect = o_ptr->ego->effect;
	else
		o_ptr->effect = o_ptr->kind ? o_ptr->kind->effect : NULL;

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
		rd_byte(&m_ptr->known_pstate.flags[j]);

	for (j = 0; j < elem_max; j++)
		rd_s16b(&m_ptr->known_pstate.el_info[j].res_level);

	strip_bytes(1);

}


/**
 * Read a trap record
 */
static void rd_trap(struct trap *trap)
{
    int i;

    rd_byte(&trap->t_idx);
    trap->kind = &trap_info[trap->t_idx];
    rd_byte(&trap->fy);
    rd_byte(&trap->fx);
    rd_byte(&trap->xtra);

    for (i = 0; i < trf_size; i++)
	rd_byte(&trap->flags[i]);
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
	u16b tmp16u;
	char buf[128];
	int i;

	/* Reset maximum numbers per level */
	for (i = 1; z_info && i < z_info->r_max; i++)
	{
		monster_race *race = &r_info[i];
		race->max_num = 100;
		if (rf_has(race->flags, RF_UNIQUE))
			race->max_num = 1;
	}

	rd_string(buf, sizeof(buf));
	while (!streq(buf, "No more monsters")) {
		monster_race *race = lookup_monster(buf);

		/* Get the kill count, skip if monster invalid */
		rd_u16b(&tmp16u);
		if (!race) continue;

		/* Store the kill count, ensure dead uniques stay dead */
		l_list[race->ridx].pkills = tmp16u;
		if (rf_has(race->flags, RF_UNIQUE) && tmp16u)
			race->max_num = 0;

		/* Look for the next monster */
		rd_string(buf, sizeof(buf));
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

	/* Identify flags */
	rd_byte(&id_size);

	/* Incompatible save files */
	if (id_size > ID_SIZE)
	{
	        note(format("Too many (%u) identify flags!", id_size));
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

		if (tmp8u & 0x04) kind_ignore_when_aware(k_ptr);
		if (tmp8u & 0x10) kind_ignore_when_unaware(k_ptr);
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
	byte stat_max = 0;
	char buf[80];

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
	rd_byte(&stat_max);
	assert(stat_max <= STAT_MAX);
	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_max[i]);
	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_cur[i]);
	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_birth[i]);

	rd_s16b(&player->ht_birth);
	rd_s16b(&player->wt_birth);
	strip_bytes(2);
	rd_s32b(&player->au_birth);

	/* Player body */
	rd_string(buf, sizeof(buf));
	player->body.name = string_make(buf);
	rd_u16b(&player->body.count);

	/* Incompatible save files */
	if (player->body.count > z_info->equip_slots_max)
	{
		note(format("Too many (%u) body parts!", player->body.count));
		return (-1);
	}

	player->body.slots = mem_zalloc(player->body.count * sizeof(struct equip_slot));
	for (i = 0; i < player->body.count; i++) {
		rd_u16b(&player->body.slots[i].type);
		rd_string(buf, sizeof(buf));
		player->body.slots[i].name = string_make(buf);
	}

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
 * Read ignore and autoinscription submenu for all known objects
 */
int rd_ignore(void)
{
	size_t i, j;
	byte tmp8u = 24;
	u16b file_e_max;
	u16b itype_size;
	u16b inscriptions;
	
	/* Read how many ignore bytes we have */
	rd_byte(&tmp8u);
	
	/* Check against current number */
	if (tmp8u != ignore_size)
	{
		strip_bytes(tmp8u);
	}
	else
	{
		for (i = 0; i < ignore_size; i++)
			rd_byte(&ignore_level[i]);
	}
		
	/* Read the number of saved ego-item */
	rd_u16b(&file_e_max);
	rd_u16b(&itype_size);

	/* Incompatible save files */
	if (itype_size > ITYPE_SIZE)
	{
		note(format("Too many (%u) ignore bytes!", itype_size));
		return (-1);
	}
		
	for (i = 0; i < file_e_max; i++)
	{
		if (i < z_info->e_max)
		{
			bitflag flags, itypes[itype_size];
			
			/* Read and extract the everseen flag */
			rd_byte(&flags);
			e_info[i].everseen = (flags & 0x02) ? TRUE : FALSE;

			/* Read and extract the ignore flags */
			for (j = 0; j < ITYPE_SIZE; j++)
				rd_byte(&itypes[j]);

			for (j = ITYPE_NONE; j < ITYPE_MAX; j++)
				if (itype_has(itypes, j))
					ego_ignore_toggle(i, j);
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
	
	/* Read the randart seed */
	rd_u32b(&seed_randart);

	/* Hack -- the two "special seeds" */
	rd_u32b(&seed_flavor);

	/* Special stuff */
	rd_u16b(&player->total_winner);
	rd_u16b(&player->noscore);


	/* Read "death" */
	rd_byte(&tmp8u);
	player->is_dead = tmp8u;

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
	if (tmp16u > player->class->magic.total_spells)
	{
		note(format("Too many player spells (%d).", tmp16u));
		return (-1);
	}

	/* Initialise */
	player_spells_init(player);
	
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
 * Read the player gear
 */
static int rd_gear_aux(rd_item_t rd_item_version)
{
	int slot = 1;

	object_type *i_ptr;
	object_type object_type_body;

	/* Initialise the size of the gear array */
	player->max_gear = MAX_GEAR;

	/* Read until done */
	while (1)
	{
		u16b n;
		byte equip;

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

		/* Increase array size if necessary */
		if (n >= player->max_gear) {
			int newsize = player->max_gear + MAX_GEAR_INCR;
			player->gear = (struct object *) mem_realloc(player->gear, newsize);
			player->max_gear += MAX_GEAR_INCR;
		}

		/* Get a slot */
		n = slot++;

		/* Copy object */
		object_copy(&player->gear[n], i_ptr);

		/* Add the weight */
		player->upkeep->total_weight += (i_ptr->number * i_ptr->weight);

		/* Is it equipment? */
		rd_byte(&equip);
		if (equip) {
			player->body.slots[wield_slot(i_ptr)].index = n;
			player->upkeep->equip_cnt++;
		}

		/* Free object */
		object_wipe(i_ptr);
	}
	calc_inventory(player->upkeep, player->gear, player->body,
				   player->max_gear);

	/* Success */
	return (0);
}

/*
 * Read the player gear - wrapper functions
 */
int rd_gear(void) { return rd_gear_aux(rd_item); }


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

	/* Ignore illegal dungeons */
	if ((depth < 0) || (depth >= MAX_DEPTH))
	{
		note(format("Ignoring illegal dungeon depth (%d)", depth));
		return (0);
	}

	/* We need a cave array */
	cave = cave_new(ymax, xmax);

	/* Ignore illegal dungeons */
	if ((px < 0) || (px >= cave->width) ||
	    (py < 0) || (py >= cave->height))
	{
		note(format("Ignoring illegal player location (%d,%d).", py, px));
		return (1);
	}


	/*** Run length decoding ***/

    /* Loop across bytes of cave->info */
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


	/* Read "feeling" */
	rd_byte(&tmp8u);
	cave->feeling = tmp8u;
	rd_u16b(&tmp16u);
	cave->feeling_squares = tmp16u;
	rd_s32b(&cave->created_at);

	/*** Player ***/

	/* Load depth */
	player->depth = depth;


	/* Place player in dungeon */
	player_place(cave, player, py, px);

	/*** Success ***/

	/* The dungeon is ready */
	character_dungeon = TRUE;

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
			struct trap *trap = &c->traps[i];

			rd_trap(trap);
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
	if (limit > z_info->level_object_max)
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
	if (limit > z_info->level_monster_max)
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
			if (o_ptr->mimicking_m_idx > z_info->level_monster_max)
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
			if (o_ptr->held_m_idx > z_info->level_monster_max)
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
		struct trap *trap = cave_trap(cave, i);

		rd_trap(trap);
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
