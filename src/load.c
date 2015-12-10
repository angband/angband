/**
 * \file load.c
 * \brief Individual loading functions
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
#include "effects.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-randart.h"
#include "obj-slays.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "savefile.h"
#include "store.h"
#include "trap.h"

/**
 * Dungeon constants
 */
byte square_size = 0;

/**
 * Player constants
 */
byte hist_size = 0;

/**
 * Object constants
 */
byte obj_mod_max = 0;
byte of_size = 0;
byte elem_max = 0;

/**
 * Monster constants
 */
byte monster_blow_max = 0;
byte rf_size = 0;
byte rsf_size = 0;
byte mflag_size = 0;

/**
 * Trap constants
 */
byte trf_size = 0;

/**
 * Shorthand function pointer for rd_item version
 */
typedef struct object *(*rd_item_t)(void);

/**
 * Find an ego item from its index
 */
static struct ego_item *lookup_ego(int idx)
{
	if (idx > 0 && idx < z_info->e_max)
		return &e_info[idx];

	return NULL;
}


/**
 * Read an object.
 */
static struct object *rd_item(void)
{
	struct object *obj = object_new();

	byte tmp8u;
	u16b tmp16u;
	s16b tmp16s;
	u32b ego_idx;
	u32b art_idx;
	byte effect;
	size_t i;
	char buf[128];
	byte ver = 1;

	rd_u16b(&tmp16u);
	rd_byte(&ver);
	if (tmp16u != 0xffff)
		return NULL;

	rd_u16b(&obj->oidx);

	/* Location */
	rd_byte(&obj->iy);
	rd_byte(&obj->ix);

	/* Type/Subtype */
	rd_byte(&obj->tval);
	rd_byte(&obj->sval);
	rd_s16b(&obj->pval);

	rd_byte(&obj->number);
	rd_s16b(&obj->weight);

	rd_u32b(&art_idx);
	rd_u32b(&ego_idx);
	rd_byte(&effect);

	rd_s16b(&obj->timeout);

	rd_s16b(&obj->to_h);
	rd_s16b(&obj->to_d);
	rd_s16b(&obj->to_a);

	rd_s16b(&obj->ac);

	rd_byte(&obj->dd);
	rd_byte(&obj->ds);

	rd_byte(&obj->origin);
	rd_byte(&obj->origin_depth);
	rd_u16b(&obj->origin_xtra);
	rd_byte(&obj->notice);

	for (i = 0; i < of_size; i++)
		rd_byte(&obj->flags[i]);

	for (i = 0; i < obj_mod_max; i++) {
		rd_s16b(&obj->modifiers[i]);
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
		b->next = obj->brands;
		obj->brands = b;
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
		s->next = obj->slays;
		obj->slays = s;
		rd_byte(&tmp8u);
	}

	for (i = 0; i < elem_max; i++) {
		rd_s16b(&obj->el_info[i].res_level);
		rd_byte(&obj->el_info[i].flags);
	}

	/* Monster holding object */
	rd_s16b(&obj->held_m_idx);

	rd_s16b(&obj->mimicking_m_idx);

	/* Activation */
	rd_u16b(&tmp16u);
	if (tmp16u)
		obj->activation = &activations[tmp16u];
	rd_u16b(&tmp16u);
	obj->time.base = tmp16u;
	rd_u16b(&tmp16u);
	obj->time.dice = tmp16u;
	rd_u16b(&tmp16u);
	obj->time.sides = tmp16u;

	/* Save the inscription */
	rd_string(buf, sizeof(buf));
	if (buf[0]) obj->note = quark_add(buf);


	/* Lookup item kind */
	obj->kind = lookup_kind(obj->tval, obj->sval);

	/* Check we have a kind */
	if ((!obj->tval && !obj->sval) || !obj->kind) {
		object_delete(&obj);
		return NULL;
	}

	/* Lookup ego */
	obj->ego = lookup_ego(ego_idx);

	/* Set artifact, fail if invalid index */
	if (art_idx >= z_info->a_max) {
		object_delete(&obj);
		return NULL;
	} else if (art_idx > 0) {
		obj->artifact = &a_info[art_idx];
	}

	/* Set effect */
	if (effect && obj->ego)
		obj->effect = obj->ego->effect;

	if (effect && !obj->effect)
		obj->effect = obj->kind->effect;

	/* Success */
	return obj;
}


/**
 * Read a monster
 */
static bool rd_monster(struct chunk *c, struct monster *mon)
{
	byte tmp8u;
	u16b tmp16u;
	s16b r_idx;
	size_t j;

	/* Read the monster race */
	rd_s16b(&r_idx);
	mon->race = &r_info[r_idx];

	/* Read the other information */
	rd_byte(&mon->fy);
	rd_byte(&mon->fx);
	rd_s16b(&mon->hp);
	rd_s16b(&mon->maxhp);
	rd_byte(&mon->mspeed);
	rd_byte(&mon->energy);
	rd_byte(&tmp8u);

	for (j = 0; j < tmp8u; j++)
		rd_s16b(&mon->m_timed[j]);

	/* Read and extract the flag */
	for (j = 0; j < mflag_size; j++)
		rd_byte(&mon->mflag[j]);

	for (j = 0; j < of_size; j++)
		rd_byte(&mon->known_pstate.flags[j]);

	for (j = 0; j < elem_max; j++)
		rd_s16b(&mon->known_pstate.el_info[j].res_level);

	rd_u16b(&tmp16u);

	if (tmp16u) {
		/* Find and set the mimicked object */
		struct object *square_obj = square_object(c, mon->fy, mon->fx);

		while (square_obj) {
			if (square_obj->mimicking_m_idx == tmp16u) break;
			square_obj = square_obj->next;
		}
		if (!square_obj)
			return false;
		mon->mimicked_obj = square_obj;
	}

	/* Read all the held objects (order is unimportant) */
	while (true) {
		struct object *obj = rd_item();
		if (!obj)
			break;

		pile_insert(&mon->held_obj, obj);
		assert(obj->oidx);
		assert(c->objects[obj->oidx] == NULL);
		c->objects[obj->oidx] = obj;
	}

	return true;
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

	Rand_quick = false;

	return 0;
}


/**
 * Read options.
 */
int rd_options(void)
{
	byte b;

	u16b tmp16u;

	/*** Special info */

	/* Read "delay_factor" */
	rd_byte(&b);
	op_ptr->delay_factor = b;

	/* Read "hitpoint_warn" */
	rd_byte(&b);
	op_ptr->hitpoint_warn = b;

	/* Read lazy movement delay */
	rd_u16b(&tmp16u);
	op_ptr->lazymove_delay = (tmp16u < 1000) ? tmp16u : 0;


	/* Read options */
	while (1) {
		byte value;
		char name[20];
		rd_string(name, sizeof name);

		if (!name[0])
			break;

		rd_byte(&value);
		option_set(name, !!value);
	}

	return 0;
}

/**
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
	for (i = 0; i < num; i++) {
		/* Read the message */
		rd_string(buf, sizeof(buf));

		/* Read the message type */
		rd_u16b(&tmp16u);

		/* Save the message */
		message_add(buf, tmp16u);
	}

	return 0;
}

/**
 * Read monster memory.
 */
int rd_monster_memory(void)
{
	u16b tmp16u;
	char buf[128];
	int i;

	/* Monster temporary flags */
	rd_byte(&mflag_size);

	/* Incompatible save files */
	if (mflag_size > MFLAG_SIZE) {
	        note(format("Too many (%u) monster temporary flags!", mflag_size));
		return (-1);
	}

	/* Reset maximum numbers per level */
	for (i = 1; z_info && i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];
		race->max_num = 100;
		if (rf_has(race->flags, RF_UNIQUE))
			race->max_num = 1;
	}

	rd_string(buf, sizeof(buf));
	while (!streq(buf, "No more monsters")) {
		struct monster_race *race = lookup_monster(buf);

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
	size_t i;
	u16b tmp16u;

	/* Object Memory */
	rd_u16b(&tmp16u);
	if (tmp16u > z_info->k_max) {
		note(format("Too many (%u) object kinds!", tmp16u));
		return (-1);
	}

	/* Object flags */
	rd_byte(&of_size);
	if (of_size > OF_SIZE) {
	        note(format("Too many (%u) object flags!", of_size));
		return (-1);
	}

	/* Object modifiers */
	rd_byte(&obj_mod_max);
	if (obj_mod_max > OBJ_MOD_MAX) {
	        note(format("Too many (%u) object modifiers allowed!", 
						obj_mod_max));
		return (-1);
	}

	/* Elements */
	rd_byte(&elem_max);
	if (elem_max > ELEM_MAX) {
	        note(format("Too many (%u) elements allowed!", 
						elem_max));
		return (-1);
	}

	/* Read the kind knowledge */
	for (i = 0; i < tmp16u; i++) {
		byte tmp8u;
		struct object_kind *kind = &k_info[i];

		rd_byte(&tmp8u);

		kind->aware = (tmp8u & 0x01) ? true : false;
		kind->tried = (tmp8u & 0x02) ? true : false;
		kind->everseen = (tmp8u & 0x08) ? true : false;

		if (tmp8u & 0x04) kind_ignore_when_aware(kind);
		if (tmp8u & 0x10) kind_ignore_when_unaware(kind);
	}

	return 0;
}



int rd_quests(void)
{
	int i;
	u16b tmp16u;

	/* Load the Quests */
	rd_u16b(&tmp16u);
	if (tmp16u > z_info->quest_max) {
		note(format("Too many (%u) quests!", tmp16u));
		return (-1);
	}

	/* Load the Quests */
	player_quests_reset(player);
	for (i = 0; i < tmp16u; i++) {
		u16b cur_num;
		rd_byte(&player->quests[i].level);
		rd_u16b(&cur_num);
		player->quests[i].cur_num = cur_num;
	}

	return 0;
}


int rd_artifacts(void)
{
	int i;
	u16b tmp16u;
	
	/* Load the Artifacts */
	rd_u16b(&tmp16u);
	if (tmp16u > z_info->a_max) {
		note(format("Too many (%u) artifacts!", tmp16u));
		return (-1);
	}
	
	/* Read the artifact flags */
	for (i = 0; i < tmp16u; i++) {
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



/**
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
	if (stat_max > STAT_MAX) {
		note(format("Too many stats (%d).", stat_max));
		return -1;
	}

	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_max[i]);
	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_cur[i]);
	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_map[i]);
	for (i = 0; i < stat_max; i++) rd_s16b(&player->stat_birth[i]);

	rd_s16b(&player->ht_birth);
	rd_s16b(&player->wt_birth);
	strip_bytes(2);
	rd_s32b(&player->au_birth);

	/* Player body */
	rd_string(buf, sizeof(buf));
	player->body.name = string_make(buf);
	rd_u16b(&player->body.count);
	if (player->body.count > z_info->equip_slots_max) {
		note(format("Too many (%u) body parts!", player->body.count));
		return (-1);
	}

	player->body.slots = mem_zalloc(player->body.count *
									sizeof(struct equip_slot));
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
	if ((player->lev < 1) || (player->lev > PY_MAX_LEVEL)) {
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

	/* Hack -- Reset cause of death */
	if (player->chp >= 0)
		my_strcpy(player->died_from, "(alive and well)",
				  sizeof(player->died_from));

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

	if (num <= TMD_MAX) {
		/* Read all the effects */
		for (i = 0; i < num; i++)
			rd_s16b(&player->timed[i]);

		/* Initialize any entries not read */
		if (num < TMD_MAX)
			memset(player->timed + num, 0, (TMD_MAX - num) * sizeof(s16b));
	} else {
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


/**
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
	if (tmp8u != ignore_size) {
		strip_bytes(tmp8u);
	} else {
		for (i = 0; i < ignore_size; i++)
			rd_byte(&ignore_level[i]);
	}

	/* Read the number of saved ego-item */
	rd_u16b(&file_e_max);
	rd_u16b(&itype_size);
	if (itype_size > ITYPE_SIZE) {
		note(format("Too many (%u) ignore bytes!", itype_size));
		return (-1);
	}

	for (i = 0; i < file_e_max; i++) {
		if (i < z_info->e_max) {
			bitflag flags, itypes[itype_size];
			
			/* Read and extract the everseen flag */
			rd_byte(&flags);
			e_info[i].everseen = (flags & 0x02) ? true : false;

			/* Read and extract the ignore flags */
			for (j = 0; j < itype_size; j++)
				rd_byte(&itypes[j]);

			/* If number of ignore types has changed, don't set anything */
			if (itype_size == ITYPE_SIZE) {
				for (j = ITYPE_NONE; j < ITYPE_MAX; j++)
					if (itype_has(itypes, j))
						ego_ignore_toggle(i, j);
			}
		}
	}

	/* Read the current number of auto-inscriptions */
	rd_u16b(&inscriptions);

	/* Read the autoinscriptions array */
	for (i = 0; i < inscriptions; i++) {
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
	size_t i;
	byte tmp8u;
	s16b tmp16s;
	
	/* Read the randart seed */
	rd_u32b(&seed_randart);

	/* Read the flavors seed */
	rd_u32b(&seed_flavor);
	flavor_init();

	/* Special stuff */
	rd_u16b(&player->total_winner);
	rd_u16b(&player->noscore);


	/* Read "death" */
	rd_byte(&tmp8u);
	player->is_dead = tmp8u;
	if (!player->is_dead && OPT(birth_randarts))
		do_randart(seed_randart, true);

	/* Current turn */
	rd_s32b(&turn);

	if (player->is_dead)
		return 0;

	/* Property knowledge */
	/* Flags */
	for (i = 0; i < OF_SIZE; i++)
		rd_byte(&player->obj_k->flags[i]);

	/* Modifiers */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		rd_s16b(&player->obj_k->modifiers[i]);
	}

	/* Elements */
	for (i = 0; i < ELEM_MAX; i++) {
		rd_s16b(&player->obj_k->el_info[i].res_level);
		rd_byte(&player->obj_k->el_info[i].flags);
	}

	/* Brands */
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
		b->next = player->obj_k->brands;
		player->obj_k->brands = b;
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
		s->next = player->obj_k->slays;
		player->obj_k->slays = s;
		rd_byte(&tmp8u);
	}

	/* Combat data */
	rd_s16b(&player->obj_k->ac);
	rd_s16b(&player->obj_k->to_a);
	rd_s16b(&player->obj_k->to_h);
	rd_s16b(&player->obj_k->to_d);
	rd_byte(&player->obj_k->dd);
	rd_byte(&player->obj_k->ds);
	return 0;
}

int rd_player_hp(void)
{
	int i;
	u16b tmp16u;

	/* Read the player_hp array */
	rd_u16b(&tmp16u);
	if (tmp16u > PY_MAX_LEVEL) {
		note(format("Too many (%u) hitpoint entries!", tmp16u));
		return (-1);
	}

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
		rd_s16b(&player->player_hp[i]);

	return 0;
}


/**
 * Read the player spells
 */
int rd_player_spells(void)
{
	int i;
	u16b tmp16u;
	
	int cnt;
	
	/* Read the number of spells */
	rd_u16b(&tmp16u);
	if (tmp16u > player->class->magic.total_spells) {
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
static int rd_gear_aux(rd_item_t rd_item_version, struct object **gear)
{
	byte code;
	struct object *last_gear_obj = NULL;

	/* Get the first item code */
	rd_byte(&code);

	/* Read until done */
	while (code != FINISHED_CODE) {
		struct object *obj = (*rd_item_version)();

		/* Read the item */
		if (!obj) {
			note("Error reading item");
			return (-1);
		}

		/* Append the object */
		obj->prev = last_gear_obj;
		if (last_gear_obj)
			last_gear_obj->next = obj;
		else
			*gear = obj;
		last_gear_obj = obj;

		/* Add the weight */
		player->upkeep->total_weight += (obj->number * obj->weight);

		/* If it's equipment, wield it */
		if (code < player->body.count) {
			player->body.slots[code].obj = obj;
			player->upkeep->equip_cnt++;
		}

		/* Get the next item code */
		rd_byte(&code);
	}

	/* Success */
	return (0);
}

/**
 * Read the player gear - wrapper functions
 */
int rd_gear(void)
{
	struct object *obj, *known_obj;

	/* Get real gear */
	if (rd_gear_aux(rd_item, &player->gear))
		return -1;

	/* Get known gear */
	if (rd_gear_aux(rd_item, &player->gear_k))
		return -1;

	/* Align the two */
	for (obj = player->gear, known_obj = player->gear_k; obj;
		 obj = obj->next, known_obj = known_obj->next)
		obj->known = known_obj;

	/* Maybe we have to duplicate also upkeep and body */
	calc_inventory(player->upkeep, player->gear, player->body);

	return 0;
}


/**
 * Read store contents
 */
static int rd_stores_aux(rd_item_t rd_item_version)
{
	int i;
	u16b tmp16u;

	/* Read the stores */
	rd_u16b(&tmp16u);
	for (i = 0; i < tmp16u; i++) {
		struct store *store = &stores[i];

		byte own, num;

		/* Read the basic info */
		rd_byte(&own);
		rd_byte(&num);

		/* XXX: refactor into store.c */
		store->owner = store_ownerbyidx(store, own);

		/* Read the items */
		for (; num; num--) {
			/* Read the known item */
			struct object *obj, *known_obj = (*rd_item_version)();
			if (!known_obj) {
				note("Error reading known item");
				return (-1);
			}

			/* Read the item */
			obj = (*rd_item_version)();
			if (!obj) {
				note("Error reading item");
				return (-1);
			}
			obj->known = known_obj;

			/* Accept any valid items */
			if (store->stock_num < z_info->store_inven_max && obj->kind) {
				if (store->sidx == STORE_HOME)
					home_carry(obj);
				else
					store_carry(store, obj);
			}
		}
	}

	return 0;
}

/**
 * Read the stores - wrapper functions
 */
int rd_stores(void) { return rd_stores_aux(rd_item); }


/**
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
static int rd_dungeon_aux(struct chunk **c)
{
	struct chunk *c1 = *c;
	int i, n, y, x;

	u16b height, width;

	byte count;
	byte tmp8u;
	u16b tmp16u;
	char name[100];

	/* Header info */
	rd_string(name, sizeof(name));
	rd_u16b(&height);
	rd_u16b(&width);

	/* We need a cave struct */
	c1 = cave_new(height, width);
	c1->name = string_make(name);

    /* Run length decoding of cave->squares[y][x].info */
	for (n = 0; n < square_size; n++) {
		/* Load the dungeon data */
		for (x = y = 0; y < c1->height; ) {
			/* Grab RLE info */
			rd_byte(&count);
			rd_byte(&tmp8u);

			/* Apply the RLE info */
			for (i = count; i > 0; i--) {
				/* Extract "info" */
				c1->squares[y][x].info[n] = tmp8u;

				/* Advance/Wrap */
				if (++x >= c1->width) {
					/* Wrap */
					x = 0;

					/* Advance/Wrap */
					if (++y >= c1->height) break;
				}
			}
		}
	}

	/* Run length decoding of dungeon data */
	for (x = y = 0; y < c1->height; ) {
		/* Grab RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--) {
			/* Extract "feat" */
			square_set_feat(c1, y, x, tmp8u);

			/* Advance/Wrap */
			if (++x >= c1->width) {
				/* Wrap */
				x = 0;

				/* Advance/Wrap */
				if (++y >= c1->height) break;
			}
		}
	}


	/* Read "feeling" */
	rd_byte(&tmp8u);
	c1->feeling = tmp8u;
	rd_u16b(&tmp16u);
	c1->feeling_squares = tmp16u;
	rd_s32b(&c1->created_at);

	/* Assign */
	*c = c1;

	return 0;
}

/**
 * Read the floor object list
 */
static int rd_objects_aux(rd_item_t rd_item_version, struct chunk *c)
{
	int i;

	/* Only if the player's alive */
	if (player->is_dead)
		return 0;

	/* Make the object list */
	rd_u16b(&c->obj_max);
	c->objects = mem_realloc(c->objects,
							 (c->obj_max + 1) * sizeof(struct object*));
	for (i = 0; i <= c->obj_max; i++)
		c->objects[i] = NULL;

	/* Read the dungeon items until one isn't returned */
	while (true) {
		struct object *obj = (*rd_item_version)();
		if (!obj)
			break;

		if (square_in_bounds_fully(c, obj->iy, obj->ix))
			pile_insert_end(&c->squares[obj->iy][obj->ix].obj, obj);
		assert(obj->oidx);
		assert(c->objects[obj->oidx] == NULL);
		c->objects[obj->oidx] = obj;
	}

	return 0;
}

/**
 * Read monsters
 */
static int rd_monsters_aux(struct chunk *c)
{
	int i;
	u16b limit;

	/* Read the monster count */
	rd_u16b(&limit);
	if (limit > z_info->level_monster_max) {
		note(format("Too many (%d) monster entries!", limit));
		return (-1);
	}

	/* Read the monsters */
	for (i = 1; i < limit; i++) {
		struct monster *mon;
		struct monster monster_body;

		/* Get local monster */
		mon = &monster_body;
		memset(mon, 0, sizeof(*mon));

		/* Read the monster */
		if (!rd_monster(c, mon)) {
			note(format("Cannot read monster %d", i));
			return (-1);
		}

		/* Place monster in dungeon */
		if (place_monster(c, mon->fy, mon->fx, mon, 0) != i) {
			note(format("Cannot place monster %d", i));
			return (-1);
		}
	}

	return 0;
}

static int rd_traps_aux(struct chunk *c)
{
	int y, x;
	struct trap *trap;

    /* Only if the player's alive */
    if (player->is_dead)
		return 0;

    rd_byte(&trf_size);

	/* Read traps until one has no location */
	while (true) {
		trap = mem_zalloc(sizeof(*trap));
		rd_trap(trap);
		y = trap->fy;
		x = trap->fx;
		if ((y == 0) && (x == 0))
			break;
		else {
			/* Put the trap at the front of the grid trap list */
			trap->next = c->squares[y][x].trap;
			c->squares[y][x].trap = trap;
		}
	}

	mem_free(trap);
    return 0;
}

int rd_dungeon(void)
{
	u16b depth;
	u16b py, px;

	/* Only if the player's alive */
	if (player->is_dead)
		return 0;

	/* Header info */
	rd_u16b(&depth);
	rd_u16b(&daycount);
	rd_u16b(&py);
	rd_u16b(&px);
	rd_byte(&square_size);

	/* Ignore illegal dungeons */
	if (depth >= z_info->max_depth) {
		note(format("Ignoring illegal dungeon depth (%d)", depth));
		return (0);
	}

	if (rd_dungeon_aux(&cave))
		return 1;

	/* Ignore illegal dungeons */
	if ((px >= cave->width) || (py >= cave->height)) {
		note(format("Ignoring illegal player location (%d,%d).", py, px));
		return (1);
	}

	/* Load player depth */
	player->depth = depth;
	cave->depth = depth;

	/* Place player in dungeon */
	player_place(cave, player, py, px);

	/* The dungeon is ready */
	character_dungeon = true;

	/* Read known cave */
	if (rd_dungeon_aux(&cave_k))
		return 1;

	return 0;
}


/**
 * Read the objects - wrapper functions
 */
int rd_objects(void)
{
	if (rd_objects_aux(rd_item, cave))
		return -1;
	if (rd_objects_aux(rd_item, cave_k))
		return -1;

	return 0;
}

/**
 * Read the monster list - wrapper functions
 */
int rd_monsters (void)
{
	int i;

	/* Only if the player's alive */
	if (player->is_dead)
		return 0;

	if (rd_monsters_aux(cave))
		return -1;
	if (rd_monsters_aux(cave_k))
		return -1;

	/* Associate known objects */
	for (i = 0; i < cave_k->obj_max; i++)
		if (cave->objects[i] && cave_k->objects[i])
			cave->objects[i]->known = cave_k->objects[i];

	return 0;
}

/**
 * Read the traps - wrapper functions
 */
int rd_traps(void)
{
	if (rd_traps_aux(cave))
		return -1;
	if (rd_traps_aux(cave_k))
		return -1;
	return 0;
}

/**
 * Read the chunk list
 */
int rd_chunks(void)
{
	int j;
	u16b chunk_max;

	if (player->is_dead)
		return 0;

	rd_u16b(&chunk_max);
	for (j = 0; j < chunk_max; j++) {
		struct chunk *c;

		/* Read the dungeon */
		if (rd_dungeon_aux(&c))
			return -1;

		/* Read the objects */
		if (rd_objects_aux(rd_item, c))
			return -1;

		/* Read the monsters */
		if (rd_monsters_aux(c))
			return -1;

		/* Read traps */
		if (rd_traps_aux(c))
			return -1;

		chunk_list_add(c);
	}

	return 0;
}


int rd_history(void)
{
	u32b tmp32u;
	size_t i, j;
	
	history_clear();

	/* History type flags */
	rd_byte(&hist_size);
	if (hist_size > HIST_SIZE) {
	        note(format("Too many (%u) history types!", hist_size));
		return (-1);
	}

	rd_u32b(&tmp32u);
	for (i = 0; i < tmp32u; i++) {
		s32b turnno;
		s16b dlev, clev;
		bitflag type[HIST_SIZE];
		byte art_name;
		char text[80];

		for (j = 0; j < hist_size; j++)		
			rd_byte(&type[j]);
		rd_s32b(&turnno);
		rd_s16b(&dlev);
		rd_s16b(&clev);
		rd_byte(&art_name);
		rd_string(text, sizeof(text));
		
		history_add_full(type, &a_info[art_name], dlev, clev, turnno, text);
	}

	return 0;
}

/**
 * For blocks that don't need loading anymore.
 */
int rd_null(void) {
	return 0;
}
