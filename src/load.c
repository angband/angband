/*
 * File: load.c
 * Purpose: Old-style savefile loading
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
#include "object/tvalsval.h"
#include "savefile.h"


/*
 * Read an object
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 */
static int rd_item(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;
	byte tmp8u;
	u16b tmp16u;

	size_t i;

	object_kind *k_ptr;

	char buf[128];

	byte ver = 1;

	static bool lose_ok = FALSE;

	/* Kind */
	rd_s16b(&o_ptr->k_idx);

	/* horrible hack */
	if (o_ptr->k_idx == (s16b) 0xffff)
	{
		rd_byte(&ver);
		rd_s16b(&o_ptr->k_idx);
	}

	if (lose_ok == FALSE && ver < 5)
	{
		lose_ok = get_check("Loading this savefile will lose all object knowledge data.  Is that OK? ");
		if (lose_ok == FALSE) return -1;
	}

	/* Paranoia */
	if ((o_ptr->k_idx < 0) || (o_ptr->k_idx >= z_info->k_max))
		return (-1);

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

	rd_byte(&o_ptr->name1);
	rd_byte(&o_ptr->name2);

	rd_s16b(&o_ptr->timeout);

	rd_s16b(&o_ptr->to_h);
	rd_s16b(&o_ptr->to_d);
	rd_s16b(&o_ptr->to_a);

	rd_s16b(&o_ptr->ac);

	rd_byte(&old_dd);
	rd_byte(&old_ds);

	if (ver > 4)
		rd_u16b(&o_ptr->ident);
	else
		rd_u16b(&tmp16u);

	rd_byte(&o_ptr->marked);

	rd_byte(&o_ptr->origin);
	rd_byte(&o_ptr->origin_depth);
	rd_u16b(&o_ptr->origin_xtra);

	/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
	for (i = 0; i < 12 && i < OF_SIZE; i++)
		rd_byte(&o_ptr->flags[i]);
	if (i < 12) strip_bytes(12 - i);
	

	memset(&o_ptr->known_flags, 0, sizeof(o_ptr->known_flags));
	if (ver > 4)
	{
		/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
		for (i = 0; i < 12 && i < OF_SIZE; i++)
			rd_byte(&o_ptr->known_flags[i]);
		if (i < 12) strip_bytes(12 - i);
	}
	else if (ver > 2)
	{
		strip_bytes(3 * 4);
	}


	/* Monster holding object */
	rd_s16b(&o_ptr->held_m_idx);

	rd_string(buf, sizeof(buf));

	/* Save the inscription */
	if (buf[0]) o_ptr->note = quark_add(buf);


	/* Lookup item kind */
	o_ptr->k_idx = lookup_kind(o_ptr->tval, o_ptr->sval);

	k_ptr = &k_info[o_ptr->k_idx];

	/* Return now in case of "blank" or "empty" objects */
	if (!k_ptr->name || !o_ptr->k_idx)
	{
		o_ptr->k_idx = 0;
		return 0;
	}



	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Get the correct fields */
		if (!randcalc_valid(k_ptr->to_h, o_ptr->to_h))
			o_ptr->to_h = randcalc(k_ptr->to_h, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(k_ptr->to_d, o_ptr->to_d))
			o_ptr->to_d = randcalc(k_ptr->to_d, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(k_ptr->to_a, o_ptr->to_a))
			o_ptr->to_a = randcalc(k_ptr->to_a, o_ptr->origin_depth, RANDOMISE);

		/* Get the correct fields */
		o_ptr->ac = k_ptr->ac;
		o_ptr->dd = k_ptr->dd;
		o_ptr->ds = k_ptr->ds;

		/* Get the correct weight */
		o_ptr->weight = k_ptr->weight;

		/* Paranoia */
		o_ptr->name1 = o_ptr->name2 = 0;

		/* All done */
		return (0);
	}



	/* Paranoia */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Paranoia */
		if (o_ptr->name1 >= z_info->a_max) return (-1);

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Verify that artifact */
		if (!a_ptr->name) o_ptr->name1 = 0;
	}

	/* Paranoia */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Paranoia */
		if (o_ptr->name2 >= z_info->e_max) return (-1);

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Verify that ego-item */
		if (!e_ptr->name) o_ptr->name2 = 0;
	}


	/* Get the standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Get the standard weight */
	o_ptr->weight = k_ptr->weight;


	/* Artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Get the new artifact "pval" */
		o_ptr->pval = a_ptr->pval;

		/* Get the new artifact fields */
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;

		/* Get the new artifact weight */
		o_ptr->weight = a_ptr->weight;
	}

	/* Ego items */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Hack -- keep some old fields */
		if ((o_ptr->dd < old_dd) && (o_ptr->ds == old_ds))
		{
			/* Keep old boosted damage dice */
			o_ptr->dd = old_dd;
		}

		/* Hack -- enforce legal pval */
		if (flags_test(e_ptr->flags, OF_SIZE, OF_PVAL_MASK, FLAG_END))
		{
			/* Force a meaningful pval */
			if (!o_ptr->pval) o_ptr->pval = 1;
		}
	}


	/* Success */
	return (0);
}


/*
 * Read RNG state
 */
int rd_randomizer(u32b version)
{
	int i;

	u16b tmp16u;


	/* Tmp */
	rd_u16b(&tmp16u);

	/* Place */
	rd_u16b(&Rand_place);

	/* State */
	for (i = 0; i < RAND_DEG; i++)
	{
		rd_u32b(&Rand_state[i]);
	}

	/* Accept */
	Rand_quick = FALSE;

	return 0;
}



/*
 * Read options
 *
 * Note that the normal options are stored as a set of 256 bit flags,
 * plus a set of 256 bit masks to indicate which bit flags were defined
 * at the time the savefile was created.  This will allow new options
 * to be added, and old options to be removed, at any time, without
 * hurting old savefiles.
 *
 * The window options are stored in the same way, but note that each
 * window gets 32 options, and their order is fixed by certain defines.
 */
int rd_options(u32b version)
{
	int i, n;

	byte b;

	u16b tmp16u;

	u32b flag[8];
	u32b mask[8];
	u32b window_flag[ANGBAND_TERM_MAX];
	u32b window_mask[ANGBAND_TERM_MAX];


	/*** Oops ***/

	/* Ignore old options */
	strip_bytes(16);


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

	/* Read the option flags */
	for (n = 0; n < 8; n++) rd_u32b(&flag[n]);

	/* Read the option masks */
	for (n = 0; n < 8; n++) rd_u32b(&mask[n]);

	/* Analyze the options */
	for (i = 0; i < OPT_MAX; i++)
	{
		int os = i / 32;
		int ob = i % 32;

		/* Process saved entries */
		if (mask[os] & (1L << ob))
		{
			/* Set flag */
			if (flag[os] & (1L << ob))
				op_ptr->opt[i] = TRUE;

			/* Clear flag */
			else
				op_ptr->opt[i] = FALSE;
		}
	}

	/*** Window Options ***/

	/* Read the window flags */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
	{
		rd_u32b(&window_flag[n]);
	}

	/* Read the window masks */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
	{
		rd_u32b(&window_mask[n]);
	}

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
int rd_messages(u32b version)
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



int rd_monster_memory(u32b version)
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
		for (i = 0; i < MONSTER_BLOW_MAX; i++)
			rd_byte(&l_ptr->blows[i]);

		/* Memorize flags */

		/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
		for (i = 0; i < 12 && i < RF_SIZE; i++)
			rd_byte(&l_ptr->flags[i]);
		if (i < 12) strip_bytes(12 - i);

		/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
		for (i = 0; i < 12 && i < RSF_SIZE; i++)
			rd_byte(&l_ptr->spell_flags[i]);
		if (i < 12) strip_bytes(12 - i);


		/* Read the "Racial" monster limit per level */
		rd_byte(&r_ptr->max_num);

		/* XXX */
		strip_bytes(3);

		/* Repair the spell lore flags */
		rsf_inter(l_ptr->spell_flags, r_ptr->spell_flags);
	}
	
	return 0;
}


int rd_object_memory(u32b version)
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


int rd_quests(u32b version)
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


int rd_artifacts(u32b version)
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

	/* For old versions, we need to go through objects and update */
	if (version == 1)
	{
		size_t i;
		object_type *o_ptr = NULL;

		bool *anywhere;
		anywhere = C_ZNEW(z_info->a_max, bool);

		/* All inventory/home artifacts need to be marked as seen */
		for (i = 0; i < ALL_INVEN_TOTAL; i++)
		{
			o_ptr = &o_list[i];
			if (object_is_known_artifact(o_ptr))
				artifact_of(o_ptr)->seen = TRUE;
			anywhere[o_ptr->name1] = TRUE;
		}

		for (i = 0; i < (size_t)o_max; i++)
		{
			o_ptr = &o_list[i];
			if (object_is_known_artifact(o_ptr))
				artifact_of(o_ptr)->seen = TRUE;
			anywhere[o_ptr->name1] = TRUE;
		}

		for (i = 0; i < MAX_STORES; i++)
		{
			int j = 0;
			for (j = 0; j < store[i].stock_num; j++)
			{
				o_ptr = &store[i].stock[j];
				if (object_is_known_artifact(o_ptr))
					artifact_of(o_ptr)->seen = TRUE;
				anywhere[o_ptr->name1] = TRUE;
			}
		}

		/* Now update the seen flags correctly */
		for (i = 0; i < z_info->a_max; i++)
		{
			artifact_type *a_ptr = &a_info[i];

			/* If it isn't present anywhere, but has been created,
			 * then it has been lost, and thus seen */
			if (a_ptr->created && !anywhere[i])
				a_ptr->seen = TRUE;
		}

		FREE(anywhere);
	}
	
	return 0;
}


static u32b randart_version;


/*
 * Read the "extra" information
 */
int rd_player(u32b version)
{
	int i;

	byte num;


	rd_string(op_ptr->full_name, sizeof(op_ptr->full_name));
	rd_string(p_ptr->died_from, 80);
	rd_string(p_ptr->history, 250);

	/* Player race */
	rd_byte(&p_ptr->prace);

	/* Verify player race */
	if (p_ptr->prace >= z_info->p_max)
	{
		note(format("Invalid player race (%d).", p_ptr->prace));
		return (-1);
	}
	rp_ptr = &p_info[p_ptr->prace];
	p_ptr->race = rp_ptr;

	/* Player class */
	rd_byte(&p_ptr->pclass);

	/* Verify player class */
	if (p_ptr->pclass >= z_info->c_max)
	{
		note(format("Invalid player class (%d).", p_ptr->pclass));
		return (-1);
	}
	cp_ptr = &c_info[p_ptr->pclass];
	p_ptr->class = cp_ptr;
	mp_ptr = &cp_ptr->spells;


	/* Player gender */
	rd_byte(&p_ptr->psex);
	sp_ptr = &sex_info[p_ptr->psex];
	p_ptr->sex = sp_ptr;

	/* Numeric name suffix */
	rd_byte(&op_ptr->name_suffix);

	/* Special Race/Class info */
	rd_byte(&p_ptr->hitdie);
	rd_byte(&p_ptr->expfact);

	/* Age/Height/Weight */
	rd_s16b(&p_ptr->age);
	rd_s16b(&p_ptr->ht);
	rd_s16b(&p_ptr->wt);

	/* Read the stat info */
	for (i = 0; i < A_MAX; i++) rd_s16b(&p_ptr->stat_max[i]);
	for (i = 0; i < A_MAX; i++) rd_s16b(&p_ptr->stat_cur[i]);
	for (i = 0; i < A_MAX; i++) rd_s16b(&p_ptr->stat_birth[i]);

	rd_s16b(&p_ptr->ht_birth);
	rd_s16b(&p_ptr->wt_birth);
	if (version >= 2) rd_s16b(&p_ptr->sc_birth);
	rd_s32b(&p_ptr->au_birth);

	strip_bytes(4);

	rd_s32b(&p_ptr->au);

	rd_s32b(&p_ptr->max_exp);
	rd_s32b(&p_ptr->exp);
	rd_u16b(&p_ptr->exp_frac);

	rd_s16b(&p_ptr->lev);

	/* Verify player level */
	if ((p_ptr->lev < 1) || (p_ptr->lev > PY_MAX_LEVEL))
	{
		note(format("Invalid player level (%d).", p_ptr->lev));
		return (-1);
	}

	rd_s16b(&p_ptr->mhp);
	rd_s16b(&p_ptr->chp);
	rd_u16b(&p_ptr->chp_frac);

	rd_s16b(&p_ptr->msp);
	rd_s16b(&p_ptr->csp);
	rd_u16b(&p_ptr->csp_frac);

	rd_s16b(&p_ptr->max_lev);
	rd_s16b(&p_ptr->max_depth);

	/* Hack -- Repair maximum player level */
	if (p_ptr->max_lev < p_ptr->lev) p_ptr->max_lev = p_ptr->lev;

	/* Hack -- Repair maximum dungeon level */
	if (p_ptr->max_depth < 0) p_ptr->max_depth = 1;

	/* More info */
	strip_bytes(8);
	rd_s16b(&p_ptr->sc);
	if (version < 2) p_ptr->sc_birth = p_ptr->sc;
	strip_bytes(2);

	/* Read the flags */
	rd_s16b(&p_ptr->food);
	rd_s16b(&p_ptr->energy);
	rd_s16b(&p_ptr->word_recall);
	rd_s16b(&p_ptr->state.see_infra);
	rd_byte(&p_ptr->confusing);
	rd_byte(&p_ptr->searching);

	/* Find the number of timed effects */
	rd_byte(&num);

	if (num <= TMD_MAX)
	{
		/* Read all the effects */
		for (i = 0; i < num; i++)
			rd_s16b(&p_ptr->timed[i]);

		/* Initialize any entries not read */
		if (num < TMD_MAX)
			C_WIPE(p_ptr->timed + num, TMD_MAX - num, s16b);
	}
	else
	{
		/* Probably in trouble anyway */
		for (i = 0; i < TMD_MAX; i++)
			rd_s16b(&p_ptr->timed[i]);

		/* Discard unused entries */
		strip_bytes(2 * (num - TMD_MAX));
		note("Discarded unsupported timed effects");
	}

	/* # of player turns */
	rd_u32b(&p_ptr->player_turn);
	/* # of turns spent resting */
	rd_u32b(&p_ptr->resting_turn);

	/* Future use */
	strip_bytes(32);

	return 0;
}


/*
 * Read squelch and autoinscription submenu for all known objects
 */
int rd_squelch(u32b version)
{
	size_t i;
	byte tmp8u = 24;
	u16b file_e_max;
	
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
	rd_u16b(&inscriptions_count);
	
	/* Write the autoinscriptions array*/
	for (i = 0; i < inscriptions_count; i++)
	{
		char tmp[80];
		
		rd_s16b(&inscriptions[i].kind_idx);
		rd_string(tmp, sizeof(tmp));
		
		inscriptions[i].inscription_idx = quark_add(tmp);
	}
	
	return 0;
}


int rd_misc(u32b version)
{
	byte tmp8u;
	
	/* Read the randart version */
	rd_u32b(&randart_version);

	/* Read the randart seed */
	rd_u32b(&seed_randart);

	/* Skip the flags */
	strip_bytes(12);


	/* Hack -- the two "special seeds" */
	rd_u32b(&seed_flavor);
	rd_u32b(&seed_town);


	/* Special stuff */
	rd_u16b(&p_ptr->panic_save);
	rd_u16b(&p_ptr->total_winner);
	rd_u16b(&p_ptr->noscore);


	/* Read "death" */
	rd_byte(&tmp8u);
	p_ptr->is_dead = tmp8u;

	/* Read "feeling" */
	rd_byte(&tmp8u);
	feeling = tmp8u;

	/* Turn of last "feeling" */
	rd_s32b(&old_turn);

	/* Current turn */
	rd_s32b(&turn);

	return 0;
}

int rd_player_hp(u32b version)
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
		rd_s16b(&p_ptr->player_hp[i]);

	return 0;
}


int rd_player_spells(u32b version)
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
		rd_byte(&p_ptr->spell_flags[i]);
	
	/* Read the spell order */
	for (i = 0, cnt = 0; i < tmp16u; i++, cnt++)
		rd_byte(&p_ptr->spell_order[cnt]);
	
	/* Success */
	return (0);
}


/*
 * Read the random artifacts
 */
int rd_randarts(u32b version)
{
	size_t i, j;
	byte tmp8u;
	s16b tmp16s;
	u16b tmp16u;
	u16b artifact_count;
	s32b tmp32s;
	u32b tmp32u;

	if (!OPT(adult_randarts))
		return 0;

	if (FALSE)
	{
		/*
		 * XXX XXX XXX
		 * Importing old savefiles with random artifacts is dangerous
		 * since the randart-generators differ and produce different
		 * artifacts from the same random seed.
		 *
		 * Switching off the check for incompatible randart versions
		 * allows to import such a savefile - do it at your own risk.
		 */

		/* Check for incompatible randart version */
		if (randart_version != RANDART_VERSION)
		{
			note(format("Incompatible random artifacts version!"));
			return (-1);
		}

		/* Initialize randarts */
		do_randart(seed_randart, TRUE);
	}
	else
	{
		/* Read the number of artifacts */
		rd_u16b(&artifact_count);

		/* Alive or cheating death */
		if (!p_ptr->is_dead || arg_wizard)
		{
			/* Incompatible save files */
			if (artifact_count > z_info->a_max)
			{
				note(format("Too many (%u) random artifacts!", artifact_count));
				return (-1);
			}

			/* Mark the old artifacts as "empty" */
			for (i = 0; i < z_info->a_max; i++)
			{
				artifact_type *a_ptr = &a_info[i];
				a_ptr->name = 0;
				a_ptr->tval = 0;
				a_ptr->sval = 0;
			}

			/* Read the artifacts */
			for (i = 0; i < artifact_count; i++)
			{
				artifact_type *a_ptr = &a_info[i];
				u16b time_base, time_dice, time_sides;

				rd_byte(&a_ptr->tval);
				rd_byte(&a_ptr->sval);
				rd_s16b(&a_ptr->pval);

				rd_s16b(&a_ptr->to_h);
				rd_s16b(&a_ptr->to_d);
				rd_s16b(&a_ptr->to_a);
				rd_s16b(&a_ptr->ac);

				rd_byte(&a_ptr->dd);
				rd_byte(&a_ptr->ds);

				rd_s16b(&a_ptr->weight);

				rd_s32b(&a_ptr->cost);

				/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
				for (j = 0; j < 12 && j < OF_SIZE; j++)
					rd_byte(&a_ptr->flags[j]);
				if (j < 12) strip_bytes(OF_SIZE - j);

				rd_byte(&a_ptr->level);
				rd_byte(&a_ptr->rarity);
				rd_byte(&a_ptr->alloc_prob);
				rd_byte(&a_ptr->alloc_min);
				rd_byte(&a_ptr->alloc_max);

				rd_u16b(&a_ptr->effect);
				rd_u16b(&time_base);
				rd_u16b(&time_dice);
				rd_u16b(&time_sides);
				a_ptr->time.base = time_base;
				a_ptr->time.dice = time_dice;
				a_ptr->time.sides = time_sides;
			}

			/* Initialize only the randart names */
			do_randart(seed_randart, FALSE);
		}
		else
		{
			/* Read the artifacts */
			for (i = 0; i < artifact_count; i++)
			{
				rd_byte(&tmp8u); /* a_ptr->tval */
				rd_byte(&tmp8u); /* a_ptr->sval */
				rd_s16b(&tmp16s); /* a_ptr->pval */

				rd_s16b(&tmp16s); /* a_ptr->to_h */
				rd_s16b(&tmp16s); /* a_ptr->to_d */
				rd_s16b(&tmp16s); /* a_ptr->to_a */
				rd_s16b(&tmp16s); /* a_ptr->ac */

				rd_byte(&tmp8u); /* a_ptr->dd */
				rd_byte(&tmp8u); /* a_ptr->ds */

				rd_s16b(&tmp16s); /* a_ptr->weight */

				rd_s32b(&tmp32s); /* a_ptr->cost */

				rd_u32b(&tmp32u); /* a_ptr->flags1 */
				rd_u32b(&tmp32u); /* a_ptr->flags2 */
				rd_u32b(&tmp32u); /* a_ptr->flags3 */

				rd_byte(&tmp8u); /* a_ptr->level */
				rd_byte(&tmp8u); /* a_ptr->rarity */
				rd_byte(&tmp8u); /* a_ptr->alloc_prob */
				rd_byte(&tmp8u); /* a_ptr->alloc_min */
				rd_byte(&tmp8u); /* a_ptr->alloc_max */

				rd_u16b(&tmp16u); /* a_ptr->effect */
				rd_u16b(&tmp16u); /* a_ptr->time_base */
				rd_u16b(&tmp16u); /* a_ptr->time_dice */
				rd_u16b(&tmp16u); /* a_ptr->time_sides */
			}
		}
	}

	return (0);
}



/*
 * Read the player inventory
 *
 * Note that the inventory is "re-sorted" later by "dungeon()".
 */
int rd_inventory(u32b version)
{
	int slot = 0;

	object_type *i_ptr;
	object_type object_type_body;

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
		if (rd_item(i_ptr))
		{
			note("Error reading item");
			return (-1);
		}

		/* Hack -- verify item */
		if (!i_ptr->k_idx) continue;;

		/* Verify slot */
		if (n >= ALL_INVEN_TOTAL) return (-1);

		/* Wield equipment */
		if (n >= INVEN_WIELD)
		{
			/* Copy object */
			object_copy(&p_ptr->inventory[n], i_ptr);

			/* Add the weight */
			p_ptr->total_weight += (i_ptr->number * i_ptr->weight);

			/* One more item */
			p_ptr->equip_cnt++;
		}

		/* Warning -- backpack is full */
		else if (p_ptr->inven_cnt == INVEN_PACK)
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
			object_copy(&p_ptr->inventory[n], i_ptr);

			/* Add the weight */
			p_ptr->total_weight += (i_ptr->number * i_ptr->weight);

			/* One more item */
			p_ptr->inven_cnt++;
		}
	}

	save_quiver_size();

	/* Success */
	return (0);
}


int rd_stores(u32b version)
{
	int i;
	u16b tmp16u;
	
	/* Read the stores */
	rd_u16b(&tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		store_type *st_ptr = &store[i];

		int j;		
		byte own, num;
		
		/* XXX */
		strip_bytes(6);
		
		/* Read the basic info */
		rd_byte(&own);
		rd_byte(&num);
		
		/* XXs */
		strip_bytes(4);
		
		/* Paranoia */
		if (own >= z_info->b_max)
		{
			note("Illegal store owner!");
			return (-1);
		}
		
		st_ptr->owner = own;
		
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
			if (rd_item(i_ptr))
			{
				note("Error reading item");
				return (-1);
			}

			if (i != STORE_HOME)
				i_ptr->ident |= IDENT_STORE;
			
			/* Accept any valid items */
			if ((st_ptr->stock_num < STORE_INVEN_MAX) &&
				(i_ptr->k_idx))
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
 * Read the dungeon
 *
 * The monsters/objects must be loaded in the same order
 * that they were stored, since the actual indexes matter.
 *
 * Note that the size of the dungeon is now hard-coded to
 * DUNGEON_HGT by DUNGEON_WID, and any dungeon with another
 * size will be silently discarded by this routine.
 *
 * Note that dungeon objects, including objects held by monsters, are
 * placed directly into the dungeon, using "object_copy()", which will
 * copy "iy", "ix", and "held_m_idx", leaving "next_o_idx" blank for
 * objects held by monsters, since it is not saved in the savefile.
 *
 * After loading the monsters, the objects being held by monsters are
 * linked directly into those monsters.
 */
int rd_dungeon(u32b version)
{
	int i, y, x;

	s16b depth;
	s16b py, px;
	s16b ymax, xmax;

	byte count;
	byte tmp8u;
	u16b tmp16u;

	/* Only if the player's alive */
	if (p_ptr->is_dead)
		return 0;

	/*** Basic info ***/

	/* Header info */
	rd_s16b(&depth);
	rd_u16b(&daycount);
	rd_s16b(&py);
	rd_s16b(&px);
	rd_s16b(&ymax);
	rd_s16b(&xmax);
	rd_u16b(&tmp16u);
	rd_u16b(&tmp16u);


	/* Ignore illegal dungeons */
	if ((depth < 0) || (depth >= MAX_DEPTH))
	{
		note(format("Ignoring illegal dungeon depth (%d)", depth));
		return (0);
	}

	/* Ignore illegal dungeons */
	if ((ymax != DUNGEON_HGT) || (xmax != DUNGEON_WID))
	{
		/* XXX XXX XXX */
		note(format("Ignoring illegal dungeon size (%d,%d).", ymax, xmax));
		return (0);
	}

	/* Ignore illegal dungeons */
	if ((px < 0) || (px >= DUNGEON_WID) ||
	    (py < 0) || (py >= DUNGEON_HGT))
	{
		note(format("Ignoring illegal player location (%d,%d).", py, px));
		return (1);
	}


	/*** Run length decoding ***/

	/* Load the dungeon data */
	for (x = y = 0; y < DUNGEON_HGT; )
	{
		/* Grab RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			/* Extract "info" */
			cave_info[y][x] = tmp8u;

			/* Advance/Wrap */
			if (++x >= DUNGEON_WID)
			{
				/* Wrap */
				x = 0;

				/* Advance/Wrap */
				if (++y >= DUNGEON_HGT) break;
			}
		}
	}

	/* Load the dungeon data */
	for (x = y = 0; y < DUNGEON_HGT; )
	{
		/* Grab RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			/* Extract "info" */
			cave_info2[y][x] = tmp8u;

			/* Advance/Wrap */
			if (++x >= DUNGEON_WID)
			{
				/* Wrap */
				x = 0;

				/* Advance/Wrap */
				if (++y >= DUNGEON_HGT) break;
			}
		}
	}


	/*** Run length decoding ***/

	/* Load the dungeon data */
	for (x = y = 0; y < DUNGEON_HGT; )
	{
		/* Grab RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			/* Extract "feat" */
			cave_set_feat(y, x, tmp8u);

			/* Advance/Wrap */
			if (++x >= DUNGEON_WID)
			{
				/* Wrap */
				x = 0;

				/* Advance/Wrap */
				if (++y >= DUNGEON_HGT) break;
			}
		}
	}


	/*** Player ***/

	/* Load depth */
	p_ptr->depth = depth;


	/* Place player in dungeon */
	if (!player_place(py, px))
	{
		note(format("Cannot place player (%d,%d)!", py, px));
		return (-1);
	}

	/*** Success ***/

	/* The dungeon is ready */
	character_dungeon = TRUE;

#if 0
	/* Regenerate town in old versions */
	if (p_ptr->depth == 0)
		character_dungeon = FALSE;
#endif

	return 0;
}

int rd_objects(u32b version)
{
	int i;
	u16b limit;

	/* Only if the player's alive */
	if (p_ptr->is_dead)
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
		if (rd_item(i_ptr))
		{
			note("Error reading item");
			return (-1);
		}

		/* Make an object */
		o_idx = o_pop();

		/* Paranoia */
		if (o_idx != i)
		{
			note(format("Cannot place object %d!", i));
			return (-1);
		}

		/* Get the object */
		o_ptr = &o_list[o_idx];

		/* Structure Copy */
		object_copy(o_ptr, i_ptr);

		/* Dungeon floor */
		if (!i_ptr->held_m_idx)
		{
			int x = i_ptr->ix;
			int y = i_ptr->iy;

			/* ToDo: Verify coordinates */

			/* Link the object to the pile */
			o_ptr->next_o_idx = cave_o_idx[y][x];

			/* Link the floor to the object */
			cave_o_idx[y][x] = o_idx;
		}
	}

	return 0;
}


int rd_monsters(u32b version)
{
	int i;
	u16b limit;

	/* Only if the player's alive */
	if (p_ptr->is_dead)
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

		/* Read in record */
		rd_s16b(&m_ptr->r_idx);
		rd_byte(&m_ptr->fy);
		rd_byte(&m_ptr->fx);
		rd_s16b(&m_ptr->hp);
		rd_s16b(&m_ptr->maxhp);
		rd_s16b(&m_ptr->csleep);
		rd_byte(&m_ptr->mspeed);
		rd_byte(&m_ptr->energy);
		rd_byte(&m_ptr->stunned);
		rd_byte(&m_ptr->confused);
		rd_byte(&m_ptr->monfear);
		strip_bytes(1);

		/* Place monster in dungeon */
		if (monster_place(m_ptr->fy, m_ptr->fx, m_ptr) != i)
		{
			note(format("Cannot place monster %d", i));
			return (-1);
		}
	}

	/* Reacquire objects */
	for (i = 1; i < o_max; ++i)
	{
		object_type *o_ptr;
		monster_type *m_ptr;

		/* Get the object */
		o_ptr = &o_list[i];

		/* Ignore dungeon objects */
		if (!o_ptr->held_m_idx) continue;

		/* Verify monster index */
		if (o_ptr->held_m_idx > z_info->m_max)
		{
			note("Invalid monster index");
			return (-1);
		}

		/* Get the monster */
		m_ptr = &mon_list[o_ptr->held_m_idx];

		/* Link the object to the pile */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Link the monster to the object */
		m_ptr->hold_o_idx = i;
	}

	return 0;
}


int rd_ghost(u32b version)
{
	char buf[64];

	/* Only if the player's alive */
	if (p_ptr->is_dead)
		return 0;	

	/* XXX */
	
	/* Strip name */
	rd_string(buf, 64);
	
	/* Strip old data */
	strip_bytes(60);

	return 0;
}


int rd_history(u32b version)
{
	u32b tmp32u;
	size_t i;
	
	history_clear();
	
	rd_u32b(&tmp32u);
	for (i = 0; i < tmp32u; i++)
	{
		s32b turn;
		s16b dlev, clev;
		u16b type;
		byte art_name;
		char text[80];
		
		rd_u16b(&type);
		rd_s32b(&turn);
		rd_s16b(&dlev);
		rd_s16b(&clev);
		rd_byte(&art_name);
		rd_string(text, sizeof(text));
		
		history_add_full(type, art_name, dlev, clev, turn, text);
	}

	return 0;
}
