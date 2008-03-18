/*
 * File: loadsave.c
 * Purpose: Game loading and saving
 *
 * Copyright (c) 2007 Elly
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
#include "z-blockfile.h"
#include "z-smap.h"

#define KEYLEN	64

u16b sf_saves = 0;

/* flag keyname enumeration */
const char* const flag_names[] =
	{	"flags1",
		"flags2",
		"flags3",
		"flags4",
		"flags5",
		"flags6"
	};

smap_t *serialize_player();
smap_t *serialize_cave();
smap_t *serialize_object(object_type *o_ptr);
smap_t *serialize_monster(monster_type *m_ptr);
smap_t *serialize_lore(monster_race *r_ptr, monster_lore *l_ptr);
smap_t *serialize_store(store_type *st_ptr);
smap_t *serialize_artifact(artifact_type *a_ptr);

void deserialize_player(smap_t *smap);
void deserialize_cave(smap_t *smap);
void deserialize_object(object_type *o_ptr, smap_t *smap);
void deserialize_monster(monster_type *m_ptr, smap_t *smap);
void deserialize_lore(monster_race *r_ptr, monster_lore *l_ptr, smap_t *smap);
void deserialize_store(store_type *st_ptr, smap_t *smap);
void deserialize_artifact(artifact_type *a_ptr, smap_t *smap);

void save_object_memory(block_t *block, object_kind *k_ptr);
void save_monster(block_t *block, monster_type *m_ptr);
void save_lore(block_t *block, monster_race *r_ptr, monster_lore *l_ptr);
void save_store(block_t *block, store_type *st_ptr);
void save_artifact(block_t *block, artifact_type *a_ptr);

void load_monster(monster_type *m_ptr, smap_t *smap);
void load_lore(monster_race *r_ptr, monster_lore *l_ptr, smap_t *smap);
void load_store(store_type *st_ptr, smap_t *smap);
void load_artifact(artifact_type *a_ptr, smap_t *smap);

void save_player(blockfile_t *bf);
void save_player_hp(blockfile_t *bf);
void save_player_spells(blockfile_t *bf);
void save_player_inventory(blockfile_t *bf);
void save_cave(blockfile_t *bf);
void save_objects(blockfile_t *bf);
void save_object_memories(blockfile_t *bf);
void save_artifact_status(blockfile_t *bf);
void save_monsters(blockfile_t *bf);
void save_lores(blockfile_t *bf);
void save_stores(blockfile_t *bf);
void save_randarts(blockfile_t *bf);
void save_options(blockfile_t *bf);
void save_squelch(blockfile_t *bf);
void save_quests(blockfile_t *bf);
void save_messages(blockfile_t *bf);
void save_random(blockfile_t *bf);

void load_system(blockfile_t *bf);
void load_player(blockfile_t *bf);
void load_player_hp(blockfile_t *bf);
void load_player_spells(blockfile_t *bf);
void load_player_inventory(blockfile_t *bf);
void load_object_memories(blockfile_t *bf);
void load_artifact_status(blockfile_t *bf);
void load_cave(blockfile_t *bf);
void load_objects(blockfile_t *bf);
void load_monsters(blockfile_t *bf);
void load_lores(blockfile_t *bf);
void load_stores(blockfile_t *bf);
void load_randarts(blockfile_t *bf);
void load_options(blockfile_t *bf);
void load_squelch(blockfile_t *bf);
void load_quests(blockfile_t *bf);
void load_messages(blockfile_t *bf);
void load_random(blockfile_t *bf);

void save_smap(block_t *block, smap_t *smap);
smap_t *load_smap(block_t *block);

bool save(char *filename)
{
	char old_name[1024];
	char new_name[1024];

	blockfile_t *bf;
	block_t *global_block;
	smap_t *global_smap;

	strnfmt(new_name, sizeof(new_name), "%s.new", filename);
	strnfmt(old_name, sizeof(old_name), "%s.old", filename);

	safe_setuid_grab();
	file_delete(new_name);
	bf = bf_open(new_name, BF_WRITE | BF_SAVE);
	safe_setuid_drop();

	if (!bf) return FALSE;

	global_block = bf_createblock(bf, "system");
	if (!global_block) return FALSE;
	global_smap = smap_new();

	sf_saves++;

	smap_put_str(global_smap, "version_string", VERSION_STRING);
	smap_put_u16b(global_smap, "past_saves", sf_saves);
	smap_put_u32b(global_smap, "seed_flavor", seed_flavor);
	smap_put_u32b(global_smap, "seed_town", seed_town);
	smap_put_u32b(global_smap, "seed_randart", seed_randart);
	smap_put_u16b(global_smap, "panic_save", p_ptr->panic_save);
	smap_put_u32b(global_smap, "randart_version", RANDART_VERSION);

	save_smap(global_block, global_smap);
	smap_free(global_smap);

	save_player(bf);
	save_player_inventory(bf);
	save_player_spells(bf);
	save_player_hp(bf);
	save_lores(bf);
	save_cave(bf);
	save_objects(bf);
	save_object_memories(bf);
	save_artifact_status(bf);
	save_monsters(bf);
	save_stores(bf);
	save_options(bf);
	save_squelch(bf);
	save_quests(bf);
	save_messages(bf);
	save_random(bf);

	if (adult_randarts)
		save_randarts(bf);

	bf_save(bf);
	bf_close(bf);

	safe_setuid_grab();
	file_delete(old_name);
	file_move(filename, old_name);
	file_move(new_name, filename);
	file_delete(old_name);
	safe_setuid_drop();

	return TRUE;
}

/*
 * Returns TRUE on successful load, FALSE otherwise.
 */
bool load(char *filename)
{
	blockfile_t *bf;

	bf = bf_open(filename, BF_READ);
	if (!bf) return FALSE;

	load_system(bf);
	load_player(bf);
	load_player_inventory(bf);
	load_player_spells(bf);
	load_player_hp(bf);
	load_lores(bf);
	if (!p_ptr->is_dead) load_cave(bf);
	load_monsters(bf);
	load_objects(bf);
	load_object_memories(bf);
	load_artifact_status(bf);
	load_stores(bf);
	load_options(bf);
	load_squelch(bf);
	load_quests(bf);
	load_messages(bf);
	load_random(bf);

	if (adult_randarts)
	{
/*		if (smap_get_u32b(global_smap, "randart_version") == RANDART_VERSION) */
		load_randarts(bf);
	}

	bf_close(bf);
	return TRUE;
}

/* Utility routines */

void save_smap(block_t *block, smap_t *smap)
{
	u32b len = 0;
	byte *data = smap_tostring(smap, &len);
	bf_createrecord(block, data, len);
	FREE(data);
}

smap_t *load_smap(block_t *block)
{
	u32b len = 0;

	const byte *data = bf_nextrecord(block, &len);
	if (!data) return NULL;

	return smap_fromstring(data, len);
}

void save_player(blockfile_t *bf)
{
	block_t *player_block = bf_createblock(bf, "player");
	smap_t *sm = serialize_player();
	save_smap(player_block, sm);
	smap_free(sm);
}

void save_player_inventory(blockfile_t *bf)
{
	block_t *inv_block = bf_createblock(bf, "player_inventory");
	u16b slot;

	for (slot = 0; slot < INVEN_TOTAL; slot++)
	{
		smap_t *inven_item = smap_new();
		smap_t *sm = NULL;
		void *str;
		u32b len;

		smap_put_u16b(inven_item, "slot", slot);

		sm = serialize_object(&inventory[slot]);
		str = smap_tostring(sm, &len);
		smap_put_blob(inven_item, "object", str, len);
		FREE(str);

		save_smap(inv_block, inven_item);

		smap_free(sm);
		smap_free(inven_item);
	}
}

void save_player_spells(blockfile_t *bf)
{
	block_t *spell_block = bf_createblock(bf, "player_spells");
	u32b i;

	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		smap_t *sm = smap_new();
		smap_put_byte(sm, "flags", p_ptr->spell_flags[i]);
		smap_put_byte(sm, "order", p_ptr->spell_order[i]);
		save_smap(spell_block, sm);
		smap_free(sm);
	}

#if 0

	/* For when the spells change and we want to make everyone relearn everything */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		p_ptr->spell_flags[i] = 0;
		p_ptr->spell_order[i] = 99;
	}

#endif
}

void save_player_hp(blockfile_t *bf)
{
	block_t *hp_block = bf_createblock(bf, "player_hps");
	u32b i;

	for (i = 0; i < PY_MAX_LEVEL; i++)
	{
		smap_t *sm = smap_new();
		smap_put_s16b(sm, "hp", p_ptr->player_hp[i]);
		save_smap(hp_block, sm);
		smap_free(sm);
	}
}

void save_cave(blockfile_t *bf)
{	
	block_t *cave_block = bf_createblock(bf, "cave");
	smap_t *sm = serialize_cave();
	save_smap(cave_block, sm);
	smap_free(sm);
}

void save_objects(blockfile_t *bf)
{
	block_t *object_block = bf_createblock(bf, "objects");
	int i;

	for (i = 1; i < o_max; i++)
	{
		smap_t *sm = serialize_object(&o_list[i]);
		save_smap(object_block, sm);
		smap_free(sm);
	}
}

void save_object_memories(blockfile_t *bf)
{
	block_t *object_block = bf_createblock(bf, "object_memory");
	int i;

	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		smap_t *sm = smap_new();

		if (k_ptr->aware) smap_put_bool(sm, "aware", TRUE);
		if (k_ptr->tried) smap_put_bool(sm, "tried", TRUE);
		if (k_ptr->squelch) smap_put_bool(sm, "squelch", TRUE);
		if (k_ptr->everseen) smap_put_bool(sm, "everseen", TRUE);

		save_smap(object_block, sm);
		smap_free(sm);
	}
}

void save_artifact_status(blockfile_t *bf)
{
	block_t *art_block = bf_createblock(bf, "artifacts");
	int i;

	for (i = 1; i < z_info->a_max; i++)
	{
		smap_t *sm = smap_new();

		if (a_info[i].cur_num) smap_put_bool(sm, "created", TRUE);

		save_smap(art_block, sm);
		smap_free(sm);
	}
}

void save_monsters(blockfile_t *bf)
{
	block_t *monster_block = bf_createblock(bf, "monsters");
	int i;

	for (i = 1; i < mon_max; i++)
		save_monster(monster_block, &mon_list[i]);
}

void save_lores(blockfile_t *bf)
{
	block_t *lore_block = bf_createblock(bf, "lore");
	u32b i;

	for (i = 0; i < z_info->r_max; i++)
		save_lore(lore_block, &r_info[i], &l_list[i]);
}

void save_stores(blockfile_t *bf)
{
	block_t *store_block = bf_createblock(bf, "stores");
	u32b i;

	for (i = 0; i < MAX_STORES; i++)
		save_store(store_block, &store[i]);
}

void save_randarts(blockfile_t *bf)
{
	block_t *art_block = bf_createblock(bf, "randarts");
	u32b i;

	for (i = 0; i < z_info->a_max; i++)
		save_artifact(art_block, &a_info[i]);
}

void save_random(blockfile_t *bf)
{
	block_t *rand_block = bf_createblock(bf, "random");
	smap_t *sm = smap_new();

	smap_put_u16b(sm, "rand_place", Rand_place);
	smap_put_blob(sm, "rand_state", Rand_state, sizeof(u32b) * RAND_DEG);

	save_smap(rand_block, sm);
	smap_free(sm);
}

void save_options(blockfile_t *bf)
{
	block_t *opt_block = bf_createblock(bf, "options");
	u32b i;
	char nb[KEYLEN];

	smap_t *sm = smap_new();

	smap_put_byte(sm, "delay_factor", op_ptr->delay_factor);
	smap_put_byte(sm, "hitpoint_warn", op_ptr->hitpoint_warn);

	for (i = 0; i < OPT_MAX; i++)
	{
		if (option_name(i))
			smap_put_bool(sm, option_name(i), op_ptr->opt[i]);
	}

	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "window_flag[%u]", (unsigned int)i);
		smap_put_u32b(sm, nb, op_ptr->window_flag[i]);
	}

	save_smap(opt_block, sm);
	smap_free(sm);
}

void save_squelch(blockfile_t *bf)
{
	block_t *squelch_block = bf_createblock(bf, "squelch");
	u32b i;
	smap_t *sm = smap_new();
	char nb[KEYLEN];
	char *buf = NULL;

	smap_put_blob(sm, "squelch_levels", squelch_level, SQUELCH_BYTES);

	buf = C_ZNEW(z_info->e_max, char);
	for (i = 0; i < z_info->e_max; i++)
		buf[i] = e_info[i].everseen;
	smap_put_blob(sm, "everseen", buf, z_info->e_max);
	FREE(buf);

	smap_put_u16b(sm, "inscriptions", inscriptions_count);
	for (i = 0; i < inscriptions_count; i++)
	{
		strnfmt(nb, KEYLEN, "inscription_kind[%u]", (unsigned int)i);
		smap_put_s16b(sm, nb, inscriptions[i].kind_idx);

		strnfmt(nb, KEYLEN, "inscription_str[%u]", (unsigned int)i);
		smap_put_str(sm, nb, quark_str(inscriptions[i].inscription_idx));
	}

	save_smap(squelch_block, sm);
	smap_free(sm);
}

void save_messages(blockfile_t *bf)
{
	block_t *message_block = bf_createblock(bf, "messages");
	s32b i = 0;

	i = messages_num();

	for (; i >= 0; i--)
	{
		smap_t *sm = smap_new();

		smap_put_str (sm, "text",  message_str((u16b) i));
		smap_put_s16b(sm, "type",  message_type((u16b) i));
		smap_put_u16b(sm, "count", message_count((u16b) i));

		save_smap(message_block, sm);
		smap_free(sm);
	}

}

void save_quests(blockfile_t *bf)
{
	block_t *quest_block = bf_createblock(bf, "quests");
	smap_t *sm = smap_new();
	char nb[KEYLEN];
	u32b i;

	smap_put_u16b(sm, "quests", MAX_Q_IDX);
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		strnfmt(nb, KEYLEN, "quest[%u]", (unsigned int)i);
		smap_put_byte(sm, nb, q_list[i].level);
	}

	save_smap(quest_block, sm);
	smap_free(sm);
}

/* Medium-level load routines */
void load_system(blockfile_t *bf)
{
	block_t *global_block;
	const void *rec;
	u32b len = 0;

	smap_t *global_smap;

	global_block = bf_findblock(bf, "system");
	rec = bf_nextrecord(global_block, &len);
	global_smap = smap_fromstring(rec, len);

	sf_saves = smap_get_u16b(global_smap, "past_saves");
	sf_lives = smap_get_u16b(global_smap, "past_lives");
	seed_flavor = smap_get_u32b(global_smap, "seed_flavor");
	seed_town = smap_get_u32b(global_smap, "seed_town");
	seed_randart = smap_get_u32b(global_smap, "seed_randart");

	smap_free(global_smap);
}


void load_player(blockfile_t *bf)
{
	block_t *player_block = bf_findblock(bf, "player");
	smap_t *p;

	p = load_smap(player_block);
	deserialize_player(p);
	smap_free(p);

	sp_ptr = &sex_info[p_ptr->psex];
	rp_ptr = &p_info[p_ptr->prace];
	cp_ptr = &c_info[p_ptr->pclass];
	mp_ptr = &cp_ptr->spells;
}

void load_player_inventory(blockfile_t *bf)
{
	block_t *inv_block = bf_findblock(bf, "player_inventory");
	smap_t *sm;
	const void *blob;
	u32b len;

	unsigned slot = 0;
	unsigned cur = 0;

	while (1)
	{
		smap_t *obj_smap;
		object_type temp;

		sm = load_smap(inv_block);
		if (!sm) break;

		slot = smap_get_u16b(sm, "slot");

		blob = smap_get_blob(sm, "object", &len);
		obj_smap = smap_fromstring(blob, len);
		deserialize_object(&temp, obj_smap);
		smap_free(obj_smap);

		smap_free(sm);

		if (!temp.k_idx) continue;

		p_ptr->total_weight += (temp.number * temp.weight);

		if (slot < INVEN_WIELD)
		{
			slot = cur++;
			p_ptr->inven_cnt++;
		}

		object_copy(&inventory[slot], &temp);
	}
}

void load_player_spells(blockfile_t *bf)
{
	block_t *spell_block = bf_findblock(bf, "player_spells");
	u32b i;
	smap_t *s;

	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		s = load_smap(spell_block);
		p_ptr->spell_flags[i] = smap_get_byte(s, "flags");
		p_ptr->spell_order[i] = smap_get_byte(s, "order");
		smap_free(s);
	}
}

void load_player_hp(blockfile_t *bf)
{
	block_t *hp_block = bf_findblock(bf, "player_hps");
	u32b i;
	smap_t *s;

	for (i = 0; i < PY_MAX_LEVEL; i++)
	{
		s = load_smap(hp_block);
		p_ptr->player_hp[i] = smap_get_s16b(s, "hp");
		smap_free(s);
	}
}

void load_cave(blockfile_t *bf)
{
	block_t *cave_block = bf_findblock(bf, "cave");
	smap_t *sm = load_smap(cave_block);
	deserialize_cave(sm);
	smap_free(sm);
	character_dungeon = TRUE;

#if 0
	/* Do this to regenerate the dungeon on changes */
	character_dungeon = FALSE;
#endif
}

void load_objects(blockfile_t *bf)
{
	block_t *object_block = bf_findblock(bf, "objects");
	size_t i;
	smap_t *s;
	object_type *o_ptr;

	for (i = 1; i < z_info->o_max && (s = load_smap(object_block)) != NULL; i++)
	{
		int o_idx = o_pop();
		if (!o_idx) return;

		o_ptr = &o_list[o_idx];
		deserialize_object(o_ptr, s);
		smap_free(s);

		if (!o_ptr->held_m_idx)
		{
			int x = o_ptr->ix;
			int y = o_ptr->iy;

			/* Keep the linked list consistent */
			o_ptr->next_o_idx = cave_o_idx[x][y];
			cave_o_idx[y][x] = o_idx;
		}
		else
		{
			if (o_ptr->held_m_idx > z_info->m_max) return;

			o_ptr->next_o_idx = mon_list[o_ptr->held_m_idx].hold_o_idx;
			mon_list[o_ptr->held_m_idx].hold_o_idx = o_idx;
		}
	}
}

void load_object_memories(blockfile_t *bf)
{
	block_t *object_block = bf_findblock(bf, "object_memory");
	size_t i;

	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		smap_t *sm = load_smap(object_block);
		if (!sm) break;

		k_ptr->aware = smap_get_bool(sm, "aware");
		k_ptr->tried = smap_get_bool(sm, "tried");
		k_ptr->squelch = smap_get_bool(sm, "squelch");
		k_ptr->everseen = smap_get_bool(sm, "everseen");

		smap_free(sm);
	}
}

void load_artifact_status(blockfile_t *bf)
{
	block_t *art_block = bf_findblock(bf, "artifacts");
	u32b i;

	for (i = 1; i < z_info->a_max; i++)
	{
		smap_t *s = load_smap(art_block);
		if (!s) return;

		a_info[i].cur_num = smap_get_bool(s, "created") ? 1 : 0;

		smap_free(s);
	}
}

void load_monsters(blockfile_t *bf)
{
	block_t *monster_block = bf_findblock(bf, "monsters");
	u32b i;
	monster_type mon;

	for (i = 1; i < z_info->m_max; i++)
	{
		smap_t *s = load_smap(monster_block);
		if (!s) return;

		load_monster(&mon, s);
		smap_free(s);

		if (!monster_place(mon.fy, mon.fx, &mon)) return;
	}
}

void load_lores(blockfile_t *bf)
{
	block_t *lore_block = bf_findblock(bf, "lore");
	u32b i;

	for (i = 0; i < z_info->r_max; i++)
	{
		smap_t *sm = load_smap(lore_block);
		deserialize_lore(&r_info[i], &l_list[i], sm);
		smap_free(sm);
	}
}

void load_stores(blockfile_t *bf)
{
	block_t *store_block = bf_findblock(bf, "stores");
	u32b i;

	for (i = 0; i < MAX_STORES; i++)
	{
		smap_t *sm = load_smap(store_block);
		load_store(&(store[i]), sm);
		smap_free(sm);
	}
}

void load_randarts(blockfile_t *bf)
{
	block_t *art_block = bf_findblock(bf, "randarts");
	smap_t *sm;
	u32b randart_version;
	u32b i;

	sm = load_smap(art_block);
	randart_version = smap_get_u32b(sm, "randart_version");
	smap_free(sm);

	if (randart_version != RANDART_VERSION)
	{
		/* note("Generating new set of randarts..."); */
		do_randart(seed_randart, TRUE);
		return;
	}

	for (i = 0; i < z_info->a_max; i++)
	{
		smap_t *sm = load_smap(art_block);
		load_artifact(&(a_info[i]), sm);
		smap_free(sm);
	}

	do_randart(seed_randart, FALSE);
}

void load_random(blockfile_t *bf)
{
	block_t *rand_block = bf_findblock(bf, "random");
	u32b len = 0;
	const void *data = NULL;
	smap_t *s = load_smap(rand_block);

	Rand_place = smap_get_u16b(s, "rand_place");

	/* NOT ENDIANSAFE */
	data = smap_get_blob(s, "rand_state", &len);
	memcpy(Rand_state, data, len);

	smap_free(s);
}

void load_options(blockfile_t *bf)
{
	block_t *opt_block = bf_findblock(bf, "options");
	u32b i;
	char nb[KEYLEN];
	smap_t *s;
	u32b window_flags[ANGBAND_TERM_MAX];

	s = load_smap(opt_block);

	op_ptr->delay_factor = smap_get_byte(s, "delay_factor");
	op_ptr->hitpoint_warn = smap_get_byte(s, "hitpoint_warn");

	for (i = 0; i < OPT_MAX; i++)
	{
		if (option_name(i))
			op_ptr->opt[i] = smap_get_bool(s, option_name(i));
	}

	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "window_flag[%u]", (unsigned int)i);
		window_flags[i] = smap_get_u32b(s, nb);
	}

	subwindows_set_flags(window_flags, ANGBAND_TERM_MAX);

	smap_free(s);
}

void load_squelch(blockfile_t *bf)
{
	block_t *squelch_block = bf_findblock(bf, "squelch");
	u32b i;
	smap_t *s;
	char nb[KEYLEN];
	const char *data;
	u32b len;

	s = load_smap(squelch_block);

	data = smap_get_blob(s, "squelch_levels", &len);
	memcpy(squelch_level, data, SQUELCH_BYTES);

	data = smap_get_blob(s, "everseen", &len);
	if (len == z_info->e_max)
	{
		for (i = 0; i < z_info->e_max; i++)
			e_info[i].everseen = (bool) data[i];
	}

	inscriptions_count = smap_get_u16b(s, "inscriptions");
	for (i = 0; i < inscriptions_count; i++)
	{
		strnfmt(nb, KEYLEN, "inscription_kind[%u]", (unsigned int)i);
		inscriptions[i].kind_idx = smap_get_s16b(s, nb);

		strnfmt(nb, KEYLEN, "inscription_str[%u]", (unsigned int)i);
		inscriptions[i].inscription_idx = quark_add(smap_get_str(s, nb));
	}

	smap_free(s);
}

void load_messages(blockfile_t *bf)
{
	block_t *message_block = bf_findblock(bf, "messages");

	while (1)
	{
		const char *text;
		u16b type, count;

		smap_t *sm = load_smap(message_block);
		if (!sm) break;

		text = smap_get_str(sm, "text");
		type = smap_get_u16b(sm, "type");
		count = smap_get_u16b(sm, "count");		

		while (count--)
			message_add(text, type);

		smap_free(sm);
	}
}

void load_quests(blockfile_t *bf)
{
	block_t *quest_block = bf_findblock(bf, "quests");
	smap_t *s;
	char nb[KEYLEN];
	u32b i;

	s = load_smap(quest_block);

	for (i = 0; i < MAX_Q_IDX; i++)
	{
		strnfmt(nb, KEYLEN, "quest[%u]", (unsigned int)i);
		q_list[i].level = smap_get_byte(s, nb);
	}

	smap_free(s);
}

/* Low-level functions */

void save_monster(block_t *block, monster_type *m_ptr)
{
	smap_t *sm = serialize_monster(m_ptr);
	save_smap(block, sm);
	smap_free(sm);
}

void save_lore(block_t *block, monster_race *r_ptr, monster_lore *l_ptr)
{
	smap_t *sm = serialize_lore(r_ptr, l_ptr);
	save_smap(block, sm);
	smap_free(sm);
}

void save_store(block_t *block, store_type *st_ptr)
{
	smap_t *sm = serialize_store(st_ptr);
	save_smap(block, sm);
	smap_free(sm);
}

void save_artifact(block_t *block, artifact_type *a_ptr)
{
	smap_t *sm = serialize_artifact(a_ptr);
	save_smap(block, sm);
	smap_free(sm);
}

void load_monster(monster_type *m_ptr, smap_t *smap)
{
	deserialize_monster(m_ptr, smap);
}

void load_store(store_type *st_ptr, smap_t *smap)
{
	deserialize_store(st_ptr, smap);
}

void load_artifact(artifact_type *a_ptr, smap_t *smap)
{
	deserialize_artifact(a_ptr, smap);
}

/* These should be in the respective files, not here */

/* Player Serialization */

smap_t *serialize_player(void)
{
	smap_t *s = smap_new();
	char nb[KEYLEN];
	u32b i;

	if (p_ptr->chp >= 0)
		my_strcpy(p_ptr->died_from, "(alive and well)", sizeof(p_ptr->died_from));

	smap_put_str(s, "name", op_ptr->full_name);
	smap_put_str(s, "died_from", p_ptr->died_from);
	smap_put_str(s, "history", p_ptr->history);

	smap_put_byte(s, "race", p_ptr->prace);
	smap_put_byte(s, "class", p_ptr->pclass);
	smap_put_byte(s, "sex", p_ptr->psex);

	smap_put_byte(s, "hitdie", p_ptr->hitdie);
	smap_put_byte(s, "expfact", p_ptr->expfact);

	smap_put_s16b(s, "age", p_ptr->age);
	smap_put_s16b(s, "ht", p_ptr->ht);
	smap_put_s16b(s, "wt", p_ptr->wt);

	for (i = 0; i < A_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "stat_max[%u]", (unsigned int)i);
		smap_put_s16b(s, nb, p_ptr->stat_max[i]);
		strnfmt(nb, KEYLEN, "stat_cur[%u]", (unsigned int)i);
		smap_put_s16b(s, nb, p_ptr->stat_cur[i]);
		strnfmt(nb, KEYLEN, "stat_birth[%u]", (unsigned int)i);
		smap_put_s16b(s, nb, p_ptr->stat_birth[i]);
	}

	smap_put_s16b(s, "ht_birth", p_ptr->ht_birth);
	smap_put_s16b(s, "wt_birth", p_ptr->wt_birth);
	smap_put_u32b(s, "au_birth", p_ptr->au_birth);

	smap_put_u32b(s, "au", p_ptr->au);
	smap_put_u32b(s, "max_exp", p_ptr->max_exp);
	smap_put_u32b(s, "exp", p_ptr->exp);
	smap_put_u16b(s, "exp_frac", p_ptr->exp_frac);
	smap_put_s16b(s, "lev", p_ptr->lev);

	smap_put_s16b(s, "mhp", p_ptr->mhp);
	smap_put_s16b(s, "chp", p_ptr->chp);
	smap_put_u16b(s, "chp_frac", p_ptr->chp_frac);

	smap_put_s16b(s, "msp", p_ptr->msp);
	smap_put_s16b(s, "csp", p_ptr->csp);
	smap_put_u16b(s, "csp_frac", p_ptr->csp_frac);

	smap_put_s16b(s, "max_lev", p_ptr->max_lev);
	smap_put_s16b(s, "max_depth", p_ptr->max_depth);

	smap_put_s16b(s, "sc", p_ptr->sc);
	
	smap_put_s16b(s, "food", p_ptr->food);
	smap_put_s16b(s, "energy", p_ptr->energy);
	smap_put_s16b(s, "word_recall", p_ptr->word_recall);
	smap_put_s16b(s, "see_infra", p_ptr->see_infra);
	smap_put_byte(s, "confusing", p_ptr->confusing);
	smap_put_byte(s, "searching", p_ptr->searching);

	smap_put_byte(s, "timed_max", TMD_MAX);

	for (i = 0; i < TMD_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "timed[%u]", (unsigned int)i);
		smap_put_s16b(s, nb, p_ptr->timed[i]);
	}

	smap_put_byte(s, "is_dead", p_ptr->is_dead);
	smap_put_byte(s, "feeling", feeling);
	smap_put_s32b(s, "old_turn", old_turn);
	smap_put_s32b(s, "turn", turn);

	smap_put_u16b(s, "total_winner", p_ptr->total_winner);
	smap_put_u16b(s, "noscore", p_ptr->noscore);

	return s;
}

void deserialize_player(smap_t *s)
{
	char nb[KEYLEN];
	u32b i;
	const char *buf;

	buf = smap_get_str(s, "name");
	my_strcpy(op_ptr->full_name, buf, sizeof(op_ptr->full_name));
	
	buf = smap_get_str(s, "died_from");
	my_strcpy(p_ptr->died_from, buf, sizeof(p_ptr->died_from));

	buf = smap_get_str(s, "history");
	my_strcpy(p_ptr->history, buf, sizeof(p_ptr->history));

	p_ptr->prace = smap_get_byte(s, "race");
	p_ptr->pclass = smap_get_byte(s, "class");
	p_ptr->psex = smap_get_byte(s, "sex");

	p_ptr->hitdie = smap_get_byte(s, "hitdie");
	p_ptr->expfact = smap_get_byte(s, "expfact");

	p_ptr->age = smap_get_s16b(s, "age");
	p_ptr->ht = smap_get_s16b(s, "ht");
	p_ptr->wt = smap_get_s16b(s, "wt");

	for (i = 0; i < A_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "stat_max[%u]", (unsigned int)i);
		p_ptr->stat_max[i] = smap_get_s16b(s, nb);

		strnfmt(nb, KEYLEN, "stat_cur[%u]", (unsigned int)i);
		p_ptr->stat_cur[i] = smap_get_s16b(s, nb);

		strnfmt(nb, KEYLEN, "stat_birth[%u]", (unsigned int)i);
		p_ptr->stat_birth[i] = smap_get_s16b(s, nb);
	}

	p_ptr->ht_birth = smap_get_s16b(s, "ht_birth");
	p_ptr->wt_birth = smap_get_s16b(s, "wt_birth");
	p_ptr->au_birth = smap_get_u32b(s, "au_birth");

	p_ptr->au = smap_get_u32b(s, "au");
	p_ptr->max_exp = smap_get_u32b(s, "max_exp");
	p_ptr->exp = smap_get_u32b(s, "exp");
	p_ptr->exp_frac = smap_get_u16b(s, "exp_frac");
	p_ptr->lev = smap_get_s16b(s, "lev");

	p_ptr->mhp = smap_get_s16b(s, "mhp");
	p_ptr->chp = smap_get_s16b(s, "chp");
	p_ptr->chp_frac = smap_get_u16b(s, "chp_frac");

	p_ptr->msp = smap_get_s16b(s, "msp");
	p_ptr->csp = smap_get_s16b(s, "csp");
	p_ptr->csp_frac = smap_get_u16b(s, "csp_frac");

	p_ptr->max_lev = smap_get_s16b(s, "max_lev");
	p_ptr->max_depth = smap_get_s16b(s, "max_depth");

	p_ptr->sc = smap_get_s16b(s, "sc");

	p_ptr->food = smap_get_s16b(s, "food");
	p_ptr->energy = smap_get_s16b(s, "energy");
	p_ptr->word_recall = smap_get_s16b(s, "word_recall");
	p_ptr->see_infra = smap_get_s16b(s, "see_infra");
	p_ptr->confusing = smap_get_byte(s, "confusing");
	p_ptr->searching = smap_get_byte(s, "searching");

	WIPE(p_ptr->timed, p_ptr->timed);
	for (i = 0; i < TMD_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "timed[%u]", (unsigned int)i);
		p_ptr->timed[i] = smap_get_s16b(s, nb);
	}

	p_ptr->is_dead = smap_get_byte(s, "is_dead");
	feeling = smap_get_byte(s, "feeling");
	old_turn = smap_get_s32b(s, "old_turn");
	turn = smap_get_s32b(s, "turn");

	p_ptr->total_winner = smap_get_u16b(s, "total_winner");
	p_ptr->noscore = smap_get_u16b(s, "noscore");
}

#if 0
/* Do RLE */

u32b serialize_rle(byte *in, byte *out, u32b hgt, u32b wid, u32b stride) {
	byte tmp = 0;
	byte last = 0;
	byte count = 0;
	u32b out_idx = 0;

	u32b y;
	u32b x;

	for (y = 0; y < hgt; y++) {
		for (x = 0; x < wid; x++) {
			tmp = in[(y * stride) + x];
			if ((tmp != last) || (count == MAX_UCHAR)) {
				out[out_idx++] = last;
				out[out_idx++] = count;
				last = tmp;
				count = 1;
			} else	{
				count++;
			}
		}
	}

	out[out_idx++] = last;
	out[out_idx++] = count;

	return out_idx;
}

u32b deserialize_rle(byte *in, byte *out, u32b hgt, u32b wid, u32b stride) {
	byte tmp;
	byte count;
	u32b in_idx = 0;

	u32b y;
	u32b x;

	for (x = 0, y = 0; y < hgt; y++) {
		tmp = in[in_idx++];
		count = in[in_idx++];

		for (; count > 0; count--) {
			out[(y * stride) + x] = tmp;
			if (++x >= wid) {
				x = 0;
				if (++y >= hgt) {
					break;
				}
			}
		}
	}

	return in_idx;
}
#endif
/* Cave serialization */

smap_t *serialize_cave()
{
	smap_t *s = smap_new();
	byte *tmp_buf = C_ZNEW(DUNGEON_HGT * DUNGEON_WID * 2, byte);

	smap_put_u16b(s, "depth", p_ptr->depth);
	smap_put_u16b(s, "py", p_ptr->py);
	smap_put_u16b(s, "px", p_ptr->px);
	smap_put_u16b(s, "hgt", DUNGEON_HGT);
	smap_put_u16b(s, "wid", DUNGEON_WID);

/*
	len = serialize_rle((byte*)cave_info, tmp_buf, DUNGEON_HGT, DUNGEON_WID, 256);
	smap_put_blob(s, "cave_info", tmp_buf, len);

	len = serialize_rle((byte*)cave_info2, tmp_buf, DUNGEON_HGT, DUNGEON_WID, 256);
	smap_put_blob(s, "cave_info2", tmp_buf, len);

	len = serialize_rle((byte*)cave_feat, tmp_buf, DUNGEON_HGT, DUNGEON_WID, DUNGEON_WID);
	smap_put_blob(s, "cave_feat", tmp_buf, len);
*/

	smap_put_blob(s, "cave_info", cave_info, DUNGEON_HGT * 256);
	smap_put_blob(s, "cave_info2", cave_info2, DUNGEON_HGT * 256);
	smap_put_blob(s, "cave_feat", cave_feat, DUNGEON_HGT * DUNGEON_WID);

	FREE(tmp_buf);

	return s;
}

void deserialize_cave(smap_t *s)
{
	u32b len;
	const byte *buf;
	u32b i;
	u32b j;

	p_ptr->depth = smap_get_u16b(s, "depth");
	p_ptr->py = smap_get_u16b(s, "py");
	p_ptr->px = smap_get_u16b(s, "px");

/*
	deserialize_rle(smap_get_blob(s, "cave_info", &len),
			(byte*)cave_info,
			DUNGEON_HGT,
			DUNGEON_WID,
			256);
	deserialize_rle(smap_get_blob(s, "cave_info2", &len),
			(byte*)cave_info2,
			DUNGEON_HGT,
			DUNGEON_WID,
			256);
	deserialize_rle(smap_get_blob(s, "cave_feat", &len),
			(byte*)cave_feat,
			DUNGEON_HGT,
			DUNGEON_WID,
			DUNGEON_WID);
*/

	buf = smap_get_blob(s, "cave_info", &len);
	for (i = 0; i < DUNGEON_HGT; i++)
		memcpy(&(cave_info[i]), buf + (i * 256), 256);

	buf = smap_get_blob(s, "cave_info2", &len);
	for (i = 0; i < DUNGEON_HGT; i++)
		memcpy(&(cave_info2[i]), buf + (i * 256), 256);

	buf = smap_get_blob(s, "cave_feat", &len);
	for (i = 0; i < DUNGEON_HGT; i++)
	{
		for (j = 0; j < DUNGEON_WID; j++)
			cave_set_feat(i, j, *((byte *)(buf + (i * DUNGEON_WID) + j)));
	}

	player_place(p_ptr->py, p_ptr->px);
}

smap_t *serialize_object(object_type *o_ptr)
{
	smap_t *s = smap_new();

	smap_put_s16b(s, "k_idx", o_ptr->k_idx);
	smap_put_byte(s, "iy", o_ptr->iy);
	smap_put_byte(s, "ix", o_ptr->ix);
	smap_put_byte(s, "tval", o_ptr->tval);
	smap_put_byte(s, "sval", o_ptr->sval);
	smap_put_s16b(s, "pval", o_ptr->pval);

	smap_put_byte(s, "pseudo", o_ptr->pseudo);

	smap_put_byte(s, "number", o_ptr->number);
	smap_put_s16b(s, "weight", o_ptr->weight);

	smap_put_byte(s, "name1", o_ptr->name1);
	smap_put_byte(s, "name2", o_ptr->name2);

	smap_put_s16b(s, "timeout", o_ptr->timeout);

	smap_put_s16b(s, "to_h", o_ptr->to_h);
	smap_put_s16b(s, "to_a", o_ptr->to_a);
	smap_put_s16b(s, "to_d", o_ptr->to_d);
	smap_put_s16b(s, "ac", o_ptr->ac);
	smap_put_byte(s, "dd", o_ptr->dd);
	smap_put_byte(s, "ds", o_ptr->ds);

	smap_put_byte(s, "ident", o_ptr->ident);
	smap_put_byte(s, "marked", o_ptr->marked);

	smap_put_byte(s, "origin", o_ptr->origin);
	smap_put_byte(s, "origin_depth", o_ptr->origin_depth);
	smap_put_u16b(s, "origin_xtra", o_ptr->origin_xtra);

	smap_put_s16b(s, "held_m_idx", o_ptr->held_m_idx);

	smap_put_u32b(s, "flags1", o_ptr->flags1);
	smap_put_u32b(s, "flags2", o_ptr->flags2);
	smap_put_u32b(s, "flags3", o_ptr->flags3);

	if (o_ptr->note)
		smap_put_str(s, "note", (char*)quark_str(o_ptr->note));

	return s;
}

void deserialize_object(object_type *o_ptr, smap_t *s)
{
	const char *inscrip;

	object_wipe(o_ptr);

	o_ptr->k_idx = smap_get_s16b(s, "k_idx");
	o_ptr->iy = smap_get_byte(s, "iy");
	o_ptr->ix = smap_get_byte(s, "ix");
	o_ptr->tval = smap_get_byte(s, "tval");
	o_ptr->sval = smap_get_byte(s, "sval");
	o_ptr->pval = smap_get_s16b(s, "pval");

	o_ptr->pseudo = smap_get_byte(s, "pseudo");
	o_ptr->number = smap_get_byte(s, "number");
	o_ptr->weight = smap_get_s16b(s, "weight");
	o_ptr->name1 = smap_get_byte(s, "name1");
	o_ptr->name2 = smap_get_byte(s, "name2");
	o_ptr->timeout = smap_get_s16b(s, "timeout");

	o_ptr->to_h = smap_get_s16b(s, "to_h");
	o_ptr->to_a = smap_get_s16b(s, "to_a");
	o_ptr->to_d = smap_get_s16b(s, "to_d");
	o_ptr->ac = smap_get_s16b(s, "ac");
	o_ptr->dd = smap_get_byte(s, "dd");
	o_ptr->ds = smap_get_byte(s, "ds");

	o_ptr->ident = smap_get_byte(s, "ident");
	o_ptr->marked = smap_get_byte(s, "marked");

	o_ptr->origin = smap_get_byte(s, "origin");
	o_ptr->origin_depth = smap_get_byte(s, "origin_depth");
	o_ptr->origin_xtra = smap_get_u16b(s, "origin_xtra");

	o_ptr->held_m_idx = smap_get_s16b(s, "held_m_idx");
	o_ptr->flags1 = smap_get_u32b(s, "flags1");
	o_ptr->flags2 = smap_get_u32b(s, "flags2");
	o_ptr->flags3 = smap_get_u32b(s, "flags3");

	inscrip = smap_get_str(s, "note");
	if (inscrip)
		o_ptr->note = quark_add(inscrip);


	/*** Fix old objects ***/

	if (o_ptr->k_idx >= z_info->k_max)
		o_ptr->k_idx = 0;

	if (o_ptr->name1 >= z_info->a_max || !a_info[o_ptr->name1].name)
		o_ptr->name1 = 0;

	if (o_ptr->name2 >= z_info->e_max || !e_info[o_ptr->name2].name)
		o_ptr->name2 = 0;
}

smap_t *serialize_monster(monster_type *m_ptr)
{
	smap_t *s = smap_new();

	smap_put_s16b(s, "r_idx", m_ptr->r_idx);
	smap_put_byte(s, "fy", m_ptr->fy);
	smap_put_byte(s, "fx", m_ptr->fx);
	smap_put_s16b(s, "hp", m_ptr->hp);
	smap_put_s16b(s, "maxhp", m_ptr->maxhp);
	smap_put_s16b(s, "csleep", m_ptr->csleep);
	smap_put_byte(s, "mspeed", m_ptr->mspeed);
	smap_put_byte(s, "energy", m_ptr->energy);
	smap_put_byte(s, "stunned", m_ptr->stunned);
	smap_put_byte(s, "confused", m_ptr->confused);
	smap_put_byte(s, "monfear", m_ptr->monfear);

	return s;
}

void deserialize_monster(monster_type *m_ptr, smap_t *s)
{
	WIPE(m_ptr, monster_type);

	m_ptr->r_idx = smap_get_s16b(s, "r_idx");
	m_ptr->fy = smap_get_byte(s, "fy");
	m_ptr->fx = smap_get_byte(s, "fx");
	m_ptr->hp = smap_get_s16b(s, "hp");
	m_ptr->maxhp = smap_get_s16b(s, "maxhp");
	m_ptr->csleep = smap_get_s16b(s, "csleep");
	m_ptr->mspeed = smap_get_byte(s, "mspeed");
	m_ptr->energy = smap_get_byte(s, "energy");
	m_ptr->stunned = smap_get_byte(s, "stunned");
	m_ptr->confused = smap_get_byte(s, "confused");
	m_ptr->monfear = smap_get_byte(s, "monfear");
}

smap_t *serialize_lore(monster_race *r_ptr, monster_lore *l_ptr)
{
	smap_t *s = smap_new();
	char nb[KEYLEN];
	u32b i;

	smap_put_s16b(s, "sights", l_ptr->sights);
	smap_put_s16b(s, "deaths", l_ptr->deaths);
	smap_put_s16b(s, "pkills", l_ptr->pkills);
	smap_put_s16b(s, "tkills", l_ptr->tkills);

	smap_put_byte(s, "wake", l_ptr->wake);
	smap_put_byte(s, "ignore", l_ptr->ignore);

	smap_put_byte(s, "drop_gold", l_ptr->drop_gold);
	smap_put_byte(s, "drop_item", l_ptr->drop_item);

	smap_put_byte(s, "cast_innate", l_ptr->cast_innate);
	smap_put_byte(s, "cast_spell", l_ptr->cast_spell);

	smap_put_byte(s, "blows_max", MONSTER_BLOW_MAX);
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "blows[%u]", (unsigned int)i);
		smap_put_byte(s, nb, l_ptr->blows[i]);
	}

	for (i = 0; i < RACE_FLAG_STRICT_UB; i++)
		smap_put_u32b(s, flag_names[i], l_ptr->flags[i]);

	smap_put_byte(s, "max_num", r_ptr->max_num);

	return s;
}

void deserialize_lore(monster_race *r_ptr, monster_lore *l_ptr, smap_t *s)
{
	char nb[KEYLEN];
	int i;

	l_ptr->sights = smap_get_s16b(s, "sights");
	l_ptr->deaths = smap_get_s16b(s, "deaths");
	l_ptr->pkills = smap_get_s16b(s, "pkills");
	l_ptr->tkills = smap_get_s16b(s, "tkills");

	l_ptr->wake = smap_get_byte(s, "wake");
	l_ptr->ignore = smap_get_byte(s, "ignore");

	l_ptr->drop_gold = smap_get_byte(s, "drop_gold");
	l_ptr->drop_item = smap_get_byte(s, "drop_item");

	l_ptr->cast_innate = smap_get_byte(s, "cast_innate");
	l_ptr->cast_spell = smap_get_byte(s, "cast_spell");

	for (i = 0; i < MONSTER_BLOW_MAX; i++)
	{
		strnfmt(nb, KEYLEN, "blows[%u]", (unsigned int)i);
		l_ptr->blows[i] = smap_get_byte(s, nb);
	}

	/* Load and "repair" the flags */
	for (i = 0; i < RACE_FLAG_STRICT_UB; i++)
		l_ptr->flags[i] = smap_get_u32b(s, flag_names[i]) & r_ptr->flags[i];

	/* This should be stored elsewhere. */
	r_ptr->max_num = smap_get_byte(s, "max_num");
}

smap_t *serialize_store(store_type *st_ptr)
{
	smap_t *s = smap_new();
	char nb[KEYLEN];
	u32b i;
	u32b len;

	smap_put_byte(s, "owner", st_ptr->owner);

	for (i = 0; i < st_ptr->stock_num; i++)
	{
		smap_t *t = serialize_object(&(st_ptr->stock[i]));
		byte *blob = smap_tostring(t, &len);

		strnfmt(nb, sizeof(nb), "stock[%u]", (unsigned int)i);
		smap_put_blob(s, nb, blob, len);

		FREE(blob);
		smap_free(t);
	}

	return s;
}

void deserialize_store(store_type *st_ptr, smap_t *s)
{
	u32b i = 0;
	size_t store_items = 0;

	char nb[KEYLEN];
	smap_t *sm;
	u32b len;
	const byte *blob;

	st_ptr->owner = smap_get_byte(s, "owner");
	if (st_ptr->owner >= z_info->b_max)
		st_ptr->owner = 0;

	while (store_items < STORE_INVEN_MAX)
	{
		strnfmt(nb, KEYLEN, "stock[%u]", (unsigned int)i);
		blob = smap_get_blob(s, nb, &len);
		if (!blob) break;

		sm = smap_fromstring(blob, len);
		deserialize_object(&(st_ptr->stock[store_items]), sm);

		if (st_ptr->stock[store_items].k_idx)
			store_items++;

		i++;
		smap_free(sm);
	}

	st_ptr->stock_num = store_items;
}

smap_t *serialize_artifact(artifact_type *a_ptr)
{
	smap_t *s = smap_new();

	smap_put_byte(s, "tval", a_ptr->tval);
	smap_put_byte(s, "sval", a_ptr->sval);
	smap_put_s16b(s, "pval", a_ptr->pval);

	smap_put_s16b(s, "to_h", a_ptr->to_h);
	smap_put_s16b(s, "to_d", a_ptr->to_d);
	smap_put_s16b(s, "to_a", a_ptr->to_a);
	smap_put_s16b(s, "ac", a_ptr->ac);

	smap_put_byte(s, "dd", a_ptr->dd);
	smap_put_byte(s, "ds", a_ptr->ds);

	smap_put_s16b(s, "weight", a_ptr->weight);
	smap_put_s32b(s, "cost", a_ptr->cost);

	smap_put_u32b(s, "flags1", a_ptr->flags1);
	smap_put_u32b(s, "flags2", a_ptr->flags2);
	smap_put_u32b(s, "flags3", a_ptr->flags3);

	smap_put_byte(s, "level", a_ptr->level);
	smap_put_byte(s, "rarity", a_ptr->rarity);

	smap_put_byte(s, "effect", a_ptr->effect);
	smap_put_u16b(s, "time_base", a_ptr->time_base);
	smap_put_u16b(s, "time_dice", a_ptr->time_dice);
	smap_put_u16b(s, "time_sides", a_ptr->time_sides);

	return s;
}

void deserialize_artifact(artifact_type *a_ptr, smap_t *s)
{
	WIPE(a_ptr, artifact_type);

	a_ptr->tval = smap_get_byte(s, "tval");
	a_ptr->sval = smap_get_byte(s, "sval");
	a_ptr->pval = smap_get_s16b(s, "pval");

	a_ptr->to_h = smap_get_s16b(s, "to_h");
	a_ptr->to_d = smap_get_s16b(s, "to_d");
	a_ptr->to_a = smap_get_s16b(s, "to_a");
	a_ptr->ac = smap_get_s16b(s, "ac");

	a_ptr->dd = smap_get_byte(s, "dd");
	a_ptr->ds = smap_get_byte(s, "ds");

	a_ptr->weight = smap_get_s16b(s, "weight");
	a_ptr->cost = smap_get_s32b(s, "cost");

	a_ptr->flags1 = smap_get_u32b(s, "flags1");
	a_ptr->flags2 = smap_get_u32b(s, "flags2");
	a_ptr->flags3 = smap_get_u32b(s, "flags3");

	a_ptr->level = smap_get_byte(s, "level");
	a_ptr->rarity = smap_get_byte(s, "rarity");

	a_ptr->effect = smap_get_byte(s, "effect");
	a_ptr->time_base = smap_get_u16b(s, "time_base");
	a_ptr->time_dice = smap_get_u16b(s, "time_dice");
	a_ptr->time_sides = smap_get_u16b(s, "time_sides");
}

