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
#include "history.h"
#include "monster/mon-spell.h"
#include "object/tvalsval.h"
#include "savefile.h"
#include "squelch.h"

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
 * Read an object, version 3
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 */
static int rd_item_3(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;
	byte tmp8u;
	u16b tmp16u;

	byte ego_idx;
	byte art_idx;

	size_t i, j;

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
	for (i = 0; i < MAX_PVALS; i++) {
		rd_s16b(&o_ptr->pval[i]);
	}
	rd_byte(&o_ptr->num_pvals);

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

	rd_byte(&old_dd);
	rd_byte(&old_ds);

	rd_u16b(&o_ptr->ident);

	rd_byte(&o_ptr->marked);

	rd_byte(&o_ptr->origin);
	rd_byte(&o_ptr->origin_depth);
	rd_u16b(&o_ptr->origin_xtra);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_byte(&o_ptr->flags[i]);
	if (i < OF_BYTES) strip_bytes(OF_BYTES - i);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_byte(&o_ptr->known_flags[i]);
	if (i < OF_BYTES) strip_bytes(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			rd_byte(&o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) strip_bytes(OF_BYTES - i);
	}

	/* Monster holding object */
	rd_s16b(&o_ptr->held_m_idx);

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

	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Get the correct fields */
		if (!randcalc_valid(o_ptr->kind->to_h, o_ptr->to_h))
			o_ptr->to_h = randcalc(o_ptr->kind->to_h, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(o_ptr->kind->to_d, o_ptr->to_d))
			o_ptr->to_d = randcalc(o_ptr->kind->to_d, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(o_ptr->kind->to_a, o_ptr->to_a))
			o_ptr->to_a = randcalc(o_ptr->kind->to_a, o_ptr->origin_depth, RANDOMISE);

		/* Get the correct fields */
		o_ptr->ac = o_ptr->kind->ac;
		o_ptr->dd = o_ptr->kind->dd;
		o_ptr->ds = o_ptr->kind->ds;

		/* Get the correct weight */
		o_ptr->weight = o_ptr->kind->weight;

		/* All done */
		return (0);
	}


	/* Get the standard fields */
	o_ptr->ac = o_ptr->kind->ac;
	o_ptr->dd = o_ptr->kind->dd;
	o_ptr->ds = o_ptr->kind->ds;

	/* Get the standard weight */
	o_ptr->weight = o_ptr->kind->weight;

	/* Artifacts */
	if (o_ptr->artifact)
	{
	        /* Get the new artifact "pvals" */
	        for (i = 0; i < MAX_PVALS; i++)
	                o_ptr->pval[i] = o_ptr->artifact->pval[i];
	        o_ptr->num_pvals = o_ptr->artifact->num_pvals;

	        /* Get the new artifact fields */
	        o_ptr->ac = o_ptr->artifact->ac;
	        o_ptr->dd = o_ptr->artifact->dd;
	        o_ptr->ds = o_ptr->artifact->ds;

	        /* Get the new artifact weight */
	        o_ptr->weight = o_ptr->artifact->weight;
	}

	/* Ego items */
	if (o_ptr->ego)	{
        /* Hack -- keep some old fields */
        if ((o_ptr->dd < old_dd) && (o_ptr->ds == old_ds))
			/* Keep old boosted damage dice */
			o_ptr->dd = old_dd;

		ego_min_pvals(o_ptr);
	}

	/* Success */
	return (0);
}

/*
 * Read an object, version 2 - remove after 3.3
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields. It also copies flags
 * and pval_flags properly, now that they are canonical.
 */
static int rd_item_2(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;
	byte tmp8u;
	u16b tmp16u;

	byte ego_idx;
	byte art_idx;

	size_t i, j;

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
	for (i = 0; i < MAX_PVALS; i++) {
		rd_s16b(&o_ptr->pval[i]);
	}
	rd_byte(&o_ptr->num_pvals);

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

	rd_byte(&old_dd);
	rd_byte(&old_ds);

	rd_u16b(&o_ptr->ident);

	rd_byte(&o_ptr->marked);

	rd_byte(&o_ptr->origin);
	rd_byte(&o_ptr->origin_depth);
	rd_u16b(&o_ptr->origin_xtra);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_byte(&o_ptr->flags[i]);
	if (i < OF_BYTES) strip_bytes(OF_BYTES - i);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_byte(&o_ptr->known_flags[i]);
	if (i < OF_BYTES) strip_bytes(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			rd_byte(&o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) strip_bytes(OF_BYTES - i);
	}

	/* Monster holding object */
	rd_s16b(&o_ptr->held_m_idx);

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

	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Get the correct fields */
		if (!randcalc_valid(o_ptr->kind->to_h, o_ptr->to_h))
			o_ptr->to_h = randcalc(o_ptr->kind->to_h, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(o_ptr->kind->to_d, o_ptr->to_d))
			o_ptr->to_d = randcalc(o_ptr->kind->to_d, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(o_ptr->kind->to_a, o_ptr->to_a))
			o_ptr->to_a = randcalc(o_ptr->kind->to_a, o_ptr->origin_depth, RANDOMISE);
	}

	/* Get the standard fields and flags*/
	o_ptr->ac = o_ptr->kind->ac;
	o_ptr->dd = o_ptr->kind->dd;
	o_ptr->ds = o_ptr->kind->ds;
	o_ptr->weight = o_ptr->kind->weight;
	of_union(o_ptr->flags, o_ptr->kind->base->flags);
	of_union(o_ptr->flags, o_ptr->kind->flags);
	for (i = 0; i < o_ptr->kind->num_pvals; i++)
		of_union(o_ptr->pval_flags[i], o_ptr->kind->pval_flags[i]);

	/* Artifacts */
	if (o_ptr->artifact)
		copy_artifact_data(o_ptr, o_ptr->artifact);

	/* Ego items */
	if (o_ptr->ego)	{
		bitflag pval_mask[OF_SIZE];

		of_union(o_ptr->flags, o_ptr->ego->flags);

        /* Hack -- keep some old fields */
        if ((o_ptr->dd < old_dd) && (o_ptr->ds == old_ds))
			/* Keep old boosted damage dice */
			o_ptr->dd = old_dd;

		create_mask(pval_mask, FALSE, OFT_PVAL, OFT_STAT, OFT_MAX);

        /* Hack -- enforce legal pval, and apply pval flags */
        for (i = 0; i < MAX_PVALS; i++) {
			if (of_is_inter(o_ptr->ego->pval_flags[i], pval_mask)) {

				of_union(o_ptr->pval_flags[i], o_ptr->ego->pval_flags[i]);

				if (!o_ptr->pval[i])
					o_ptr->pval[i] = o_ptr->ego->min_pval[i];
			}
		}
	}

	/* Success */
	return (0);
}

/**
 * Read an object - remove for the version after 3.3
 */
static int rd_item_1(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;
	byte tmp8u;
	u16b tmp16u;

	byte art_idx;
	byte ego_idx;

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
	rd_s16b(&o_ptr->pval[DEFAULT_PVAL]);

	if (o_ptr->pval[DEFAULT_PVAL])
		o_ptr->num_pvals = 1;
	else
		o_ptr->num_pvals = 0;

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

	rd_byte(&old_dd);
	rd_byte(&old_ds);

	rd_u16b(&o_ptr->ident);

	rd_byte(&o_ptr->marked);

	rd_byte(&o_ptr->origin);
	rd_byte(&o_ptr->origin_depth);
	rd_u16b(&o_ptr->origin_xtra);

	/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
	for (i = 0; i < 12 && i < OF_SIZE; i++)
		rd_byte(&o_ptr->flags[i]);
	if (i < 12) strip_bytes(12 - i);

	of_wipe(o_ptr->known_flags);

	/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
	for (i = 0; i < 12 && i < OF_SIZE; i++)
		rd_byte(&o_ptr->known_flags[i]);
	if (i < 12) strip_bytes(12 - i);


	/* Monster holding object */
	rd_s16b(&o_ptr->held_m_idx);

	rd_string(buf, sizeof(buf));

	/* Save the inscription */
	if (buf[0]) o_ptr->note = quark_add(buf);


	o_ptr->kind = lookup_kind(o_ptr->tval, o_ptr->sval);
	if (!o_ptr->kind)
		return 0;

	o_ptr->ego = lookup_ego(ego_idx);

	if (art_idx >= z_info->a_max)
		return -1;
	if (art_idx > 0)
		o_ptr->artifact = &a_info[art_idx];

	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Get the correct fields */
		if (!randcalc_valid(o_ptr->kind->to_h, o_ptr->to_h))
			o_ptr->to_h = randcalc(o_ptr->kind->to_h, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(o_ptr->kind->to_d, o_ptr->to_d))
			o_ptr->to_d = randcalc(o_ptr->kind->to_d, o_ptr->origin_depth, RANDOMISE);
		if (!randcalc_valid(o_ptr->kind->to_a, o_ptr->to_a))
			o_ptr->to_a = randcalc(o_ptr->kind->to_a, o_ptr->origin_depth, RANDOMISE);
	}

	/* Get the standard fields and flags*/
	o_ptr->ac = o_ptr->kind->ac;
	o_ptr->dd = o_ptr->kind->dd;
	o_ptr->ds = o_ptr->kind->ds;
	o_ptr->weight = o_ptr->kind->weight;
	of_union(o_ptr->flags, o_ptr->kind->base->flags);
	of_union(o_ptr->flags, o_ptr->kind->flags);
	for (i = 0; i < o_ptr->kind->num_pvals; i++)
		of_union(o_ptr->pval_flags[DEFAULT_PVAL], o_ptr->kind->pval_flags[i]);

	/* Artifacts */
	if (o_ptr->artifact)
		copy_artifact_data(o_ptr, o_ptr->artifact);

	/* Ego items */
	if (o_ptr->ego)	{
		bitflag pval_mask[OF_SIZE];

		of_union(o_ptr->flags, o_ptr->ego->flags);

        /* Hack -- keep some old fields */
        if ((o_ptr->dd < old_dd) && (o_ptr->ds == old_ds))
			/* Keep old boosted damage dice */
			o_ptr->dd = old_dd;

		create_mask(pval_mask, FALSE, OFT_PVAL, OFT_STAT, OFT_MAX);

        /* Hack -- enforce legal pval, and apply pval flags */
        for (i = 0; i < MAX_PVALS; i++) {
			if (of_is_inter(o_ptr->ego->pval_flags[i], pval_mask)) {

				of_union(o_ptr->pval_flags[DEFAULT_PVAL], o_ptr->ego->pval_flags[i]);

				if (o_ptr->pval[DEFAULT_PVAL] < o_ptr->ego->min_pval[i])
					o_ptr->pval[DEFAULT_PVAL] = o_ptr->ego->min_pval[i];
			}
		}
	}

	/* Success */
	return (0);
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
 * Read options, version 2.
 */
int rd_options_2(void)
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


static const struct {
	int num;
	const char *text;
} num_to_text[] = {
	{ 0, "rogue_like_commands" },
	{ 2, "use_sound" },
	{ 4, "use_old_target" },
	{ 5, "pickup_always" },
	{ 6, "pickup_inven" },
	{ 15, "show_flavors" },
	{ 20, "disturb_move" },
	{ 21, "disturb_near" },
	{ 22, "disturb_detect" },
	{ 23, "disturb_state" },
	{ 60, "view_yellow_light" },
	{ 64, "easy_open" },
	{ 66, "animate_flicker" },
	{ 68, "center_player" },
	{ 69, "purple_uniques" },
	{ 70, "xchars_to_file" },
	{ 71, "auto_more" },
	{ 74, "hp_changes_color" },
	{ 77, "mouse_movement" },
	{ 78, "mouse_buttons" },
	{ 79, "notify_recharge" },
	{ 161, "cheat_hear" },
	{ 162, "cheat_room" },
	{ 163, "cheat_xtra" },
	{ 164, "cheat_know" },
	{ 165, "cheat_live" },
	{ 192, "birth_maximize" },
	{ 193, "birth_randarts" },
	{ 195, "birth_ironman" },
	{ 196, "birth_no_stores" },
	{ 197, "birth_no_artifacts" },
	{ 198, "birth_no_stacking" },
	{ 199, "birth_no_preserve" },
	{ 200, "birth_no_stairs" },
	{ 201, "birth_no_feelings" },
	{ 202, "birth_no_selling" },
	{ 205, "birth_ai_sound" },
	{ 206, "birth_ai_smell" },
	{ 207, "birth_ai_packs" },
	{ 208, "birth_ai_learn" },
	{ 209, "birth_ai_cheat" },
	{ 210, "birth_ai_smart" },
	{ 225, "score_hear" },
	{ 226, "score_room" },
	{ 227, "score_xtra" },
	{ 228, "score_know" },
	{ 229, "score_live" },
};

static const char *lookup_option(int opt)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(num_to_text); i++) {
		if (num_to_text[i].num == opt)
			return num_to_text[i].text;
	}

	return NULL;
}


/*
 * Read options
 *
 * XXX Remove this for the next release after 3.2.
 */
int rd_options_1(void)
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
	/* 256 is the old OPT_MAX */
	for (i = 0; i < 256; i++)
	{
		int os = i / 32;
		int ob = i % 32;

		/* Process saved entries */
		if (mask[os] & (1L << ob))
		{
			const char *name = lookup_option(i);
			if (name)
				option_set(name, flag[os] & (1L << ob) ? TRUE : FALSE);
		}
	}

	/*** Window Options ***/

	/* Read the window flags */
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
		rd_u32b(&window_flag[n]);

	/* Read the window masks */
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

/* Read monster memory, version 2 */
int rd_monster_memory_2(void)
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
		for (i = 0; i < RF_BYTES && i < RF_SIZE; i++)
			rd_byte(&l_ptr->flags[i]);
		if (i < RF_BYTES) strip_bytes(RF_BYTES - i);

		for (i = 0; i < RF_BYTES && i < RSF_SIZE; i++)
			rd_byte(&l_ptr->spell_flags[i]);
		if (i < RF_BYTES) strip_bytes(RF_BYTES - i);

		/* Read the "Racial" monster limit per level */
		rd_byte(&r_ptr->max_num);

		/* XXX */
		strip_bytes(3);

		/* Repair the spell lore flags */
		rsf_inter(l_ptr->spell_flags, r_ptr->spell_flags);
	}
	
	return 0;
}

/* Read monster memory - remove after 3.3 */
int rd_monster_memory_1(void)
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
 * Read the "extra" information
 */
int rd_player(void)
{
	int i;
	byte num;

	rd_string(op_ptr->full_name, sizeof(op_ptr->full_name));
	rd_string(p_ptr->died_from, 80);
	p_ptr->history = mem_zalloc(250);
	rd_string(p_ptr->history, 250);

	/* Player race */
	rd_byte(&num);
	p_ptr->race = player_id2race(num);

	/* Verify player race */
	if (!p_ptr->race) {
		note(format("Invalid player race (%d).", num));
		return -1;
	}

	/* Player class */
	rd_byte(&num);
	p_ptr->class = player_id2class(num);

	if (!p_ptr->class) {
		note(format("Invalid player class (%d).", num));
		return -1;
	}

	/* Player gender */
	rd_byte(&p_ptr->psex);
	p_ptr->sex = &sex_info[p_ptr->psex];

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
	rd_s16b(&p_ptr->sc_birth);
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
	p_ptr->sc_birth = p_ptr->sc;
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

	/* Total energy used so far */
	rd_u32b(&p_ptr->total_energy);
	/* # of turns spent resting */
	rd_u32b(&p_ptr->resting_turn);

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
	
	/* Read the randart version */
	strip_bytes(4);

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
	cave->feeling = tmp8u;

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
		rd_s16b(&p_ptr->player_hp[i]);

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
		rd_byte(&p_ptr->spell_flags[i]);
	
	/* Read the spell order */
	for (i = 0, cnt = 0; i < tmp16u; i++, cnt++)
		rd_byte(&p_ptr->spell_order[cnt]);
	
	/* Success */
	return (0);
}


/*
 * Read the random artifacts, version 2
 */
int rd_randarts_2(void)
{
	size_t i, j, k;
	byte tmp8u;
	s16b tmp16s;
	u16b tmp16u;
	u16b artifact_count;
	s32b tmp32s;

	if (!OPT(birth_randarts)) {
		p_ptr->randarts = FALSE;
		return 0;
	}

	/* Read the number of artifacts */
	rd_u16b(&artifact_count);

	/* Alive or cheating death or re-using randarts */
	if (!p_ptr->is_dead || arg_wizard ||
			(OPT(birth_randarts) && OPT(birth_keep_randarts)))
	{
		/* Incompatible save files */
		if (artifact_count > z_info->a_max)
		{
			note(format("Too many (%u) random artifacts!", artifact_count));
			return (-1);
		}

		/* Read the artifacts */
		for (i = 0; i < artifact_count; i++)
		{
			artifact_type *a_ptr = &a_info[i];
			u16b time_base, time_dice, time_sides;

			rd_byte(&a_ptr->tval);
			rd_byte(&a_ptr->sval);
			for (j = 0; j < MAX_PVALS; j++)
				rd_s16b(&a_ptr->pval[j]);
			rd_byte(&a_ptr->num_pvals);

			rd_s16b(&a_ptr->to_h);
			rd_s16b(&a_ptr->to_d);
			rd_s16b(&a_ptr->to_a);
			rd_s16b(&a_ptr->ac);

			rd_byte(&a_ptr->dd);
			rd_byte(&a_ptr->ds);

			rd_s16b(&a_ptr->weight);
			rd_s32b(&a_ptr->cost);

			for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
				rd_byte(&a_ptr->flags[j]);
			if (j < OF_BYTES) strip_bytes(OF_BYTES - j);

			for (k = 0; k < MAX_PVALS; k++) {
				for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
					rd_byte(&a_ptr->pval_flags[k][j]);
				if (j < OF_BYTES) strip_bytes(OF_BYTES - j);
			}

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
		p_ptr->randarts = TRUE;

		/* Mark any stray old artifacts as "empty" */
		if (artifact_count < z_info->a_max)
		{
			for (i = artifact_count; i < z_info->a_max; i++)
			{
				artifact_type *a_ptr = &a_info[i];
				a_ptr->name = 0;
				a_ptr->tval = 0;
				a_ptr->sval = 0;
			}
		}
	}
	else
	{
		/* Read the artifacts */
		for (i = 0; i < artifact_count; i++)
		{
			rd_byte(&tmp8u); /* a_ptr->tval */
			rd_byte(&tmp8u); /* a_ptr->sval */
			for (j = 0; j < MAX_PVALS; j++)
				rd_s16b(&tmp16s); /* a_ptr->pval */

			rd_s16b(&tmp16s); /* a_ptr->to_h */
			rd_s16b(&tmp16s); /* a_ptr->to_d */
			rd_s16b(&tmp16s); /* a_ptr->to_a */
			rd_s16b(&tmp16s); /* a_ptr->ac */

			rd_byte(&tmp8u); /* a_ptr->dd */
			rd_byte(&tmp8u); /* a_ptr->ds */

			rd_s16b(&tmp16s); /* a_ptr->weight */
			rd_s32b(&tmp32s); /* a_ptr->cost */

			for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
				rd_byte(&tmp8u);
			if (j < OF_BYTES) strip_bytes(OF_BYTES - j);

			for (k = 0; k < MAX_PVALS; k++) {
				for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
					rd_byte(&tmp8u);
				if (j < OF_BYTES) strip_bytes(OF_BYTES - j);
			}

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

		p_ptr->randarts = FALSE;
	}

	return (0);
}


/*
 * Read the random artifacts - remove after 3.3
 */
int rd_randarts_1(void)
{
	size_t i, j;
	byte tmp8u;
	s16b tmp16s;
	u16b tmp16u;
	u16b artifact_count;
	s32b tmp32s;
	u32b tmp32u;

	if (!OPT(birth_randarts))
		return 0;

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

		/* Read the artifacts */
		for (i = 0; i < artifact_count; i++)
		{
			artifact_type *a_ptr = &a_info[i];
			u16b time_base, time_dice, time_sides;

			rd_byte(&a_ptr->tval);
			rd_byte(&a_ptr->sval);
			rd_s16b(&a_ptr->pval[DEFAULT_PVAL]);

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

		/* Mark any stray old artifacts as "empty" */
		if (artifact_count < z_info->a_max)
		{
			for (i = artifact_count; i < z_info->a_max; i++)
			{
				artifact_type *a_ptr = &a_info[i];
				a_ptr->name = 0;
				a_ptr->tval = 0;
				a_ptr->sval = 0;
			}
		}
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

	return (0);
}


/**
 * Read the player inventory
 *
 * Note that the inventory is re-sorted later by dungeon().
 */
static int rd_inventory(rd_item_t rd_item_version)
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

	save_quiver_size(p_ptr);

	/* Success */
	return (0);
}

/*
 * Read the player inventory - wrapper functions
 */
int rd_inventory_3(void) { return rd_inventory(rd_item_3); }
int rd_inventory_2(void) { return rd_inventory(rd_item_2); } /* remove post-3.3 */
int rd_inventory_1(void) { return rd_inventory(rd_item_1); } /* remove post-3.3 */


/* Read store contents */
static int rd_stores(rd_item_t rd_item_version)
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
int rd_stores_3(void) { return rd_stores(rd_item_3); }
int rd_stores_2(void) { return rd_stores(rd_item_2); } /* remove post-3.3 */
int rd_stores_1(void) { return rd_stores(rd_item_1); } /* remove post-3.3 */


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
int rd_dungeon(void)
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

	cave->width = xmax;
	cave->height = ymax;

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
			cave->info[y][x] = tmp8u;

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
			cave->info2[y][x] = tmp8u;

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
			cave_set_feat(cave, y, x, tmp8u);

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
	player_place(cave, p_ptr, py, px);

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

/* Read the floor object list */
static int rd_objects(rd_item_t rd_item_version)
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
		if ((*rd_item_version)(i_ptr))
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
		o_ptr = object_byid(o_idx);

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
int rd_objects_3(void) { return rd_objects(rd_item_3); }
int rd_objects_2(void) { return rd_objects(rd_item_2); } /* remove post-3.3 */
int rd_objects_1(void) { return rd_objects(rd_item_1); } /* remove post-3.3 */


/**
 * Read monsters (old version - before MON_TMD_FOO) 
 * - remove after 3.3
 */
int rd_monsters_1(void)
{
	int i;
	u16b limit;
	byte tmp;

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
		rd_s16b(&m_ptr->m_timed[MON_TMD_SLEEP]);
		rd_byte(&m_ptr->mspeed);
		rd_byte(&m_ptr->energy);
		rd_byte(&tmp);
		m_ptr->m_timed[MON_TMD_STUN] = tmp;
		rd_byte(&tmp);
		m_ptr->m_timed[MON_TMD_CONF] = tmp;
		rd_byte(&tmp);
		m_ptr->m_timed[MON_TMD_FEAR] = tmp;

		strip_bytes(1);

		/* Place monster in dungeon */
		if (monster_place(m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
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
		o_ptr = object_byid(i);

		/* Ignore dungeon objects */
		if (!o_ptr->held_m_idx) continue;

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
	}

	return 0;
}

/**
 * Read monsters (MON_TMD_MAX = 4)
 */
int rd_monsters_2(void)
{
	int i, j;
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
		rd_byte(&m_ptr->mspeed);
		rd_byte(&m_ptr->energy);

		for (j = 0; j < 4; j++)
			rd_s16b(&m_ptr->m_timed[j]);

		strip_bytes(1);

		/* Place monster in dungeon */
		if (monster_place(m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
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
		o_ptr = object_byid(i);

		/* Ignore dungeon objects */
		if (!o_ptr->held_m_idx) continue;

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
	}

	return 0;
}


/**
 * Read monsters (MON_TMD_MAX = 4, new "unaware" setting)
 */
int rd_monsters_3(void)
{
	int i, j;
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
		
		byte flags;

		/* Get local monster */
		m_ptr = &monster_type_body;
		WIPE(m_ptr, monster_type);

		/* Read in record */
		rd_s16b(&m_ptr->r_idx);
		rd_byte(&m_ptr->fy);
		rd_byte(&m_ptr->fx);
		rd_s16b(&m_ptr->hp);
		rd_s16b(&m_ptr->maxhp);
		rd_byte(&m_ptr->mspeed);
		rd_byte(&m_ptr->energy);

		for (j = 0; j < 4; j++)
			rd_s16b(&m_ptr->m_timed[j]);

		/* Read and extract the flag */
		rd_byte(&flags);
		m_ptr->unaware = (flags & 0x01) ? TRUE : FALSE;
			
		strip_bytes(1);

		/* Place monster in dungeon */
		if (monster_place(m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
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
		o_ptr = object_byid(i);

		/* Ignore dungeon objects */
		if (!o_ptr->held_m_idx) continue;

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
	}

	return 0;
}

/**
 * Read monsters (MON_TMD_MAX = variable)
 */
int rd_monsters_4(void)
{
	int i, j;
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
		
		byte flags;
		byte tmp8u;

		/* Get local monster */
		m_ptr = &monster_type_body;
		WIPE(m_ptr, monster_type);

		/* Read in record */
		rd_s16b(&m_ptr->r_idx);
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
			
		strip_bytes(1);

		/* Place monster in dungeon */
		if (monster_place(m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
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
		o_ptr = object_byid(i);

		/* Ignore dungeon objects */
		if (!o_ptr->held_m_idx) continue;

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
	}

	return 0;
}

/**
 * Read monsters (added m_ptr->known_pflags)
 */
int rd_monsters_5(void)
{
	int i;
	size_t j;
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
		
		byte flags;
		byte tmp8u;

		/* Get local monster */
		m_ptr = &monster_type_body;
		WIPE(m_ptr, monster_type);

		/* Read in record */
		rd_s16b(&m_ptr->r_idx);
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
	
		for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
			rd_byte(&m_ptr->known_pflags[j]);
		if (j < OF_BYTES) strip_bytes(OF_BYTES - j);
		
		strip_bytes(1);

		/* Place monster in dungeon */
		if (monster_place(m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
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
		o_ptr = object_byid(i);

		/* Ignore dungeon objects */
		if (!o_ptr->held_m_idx) continue;

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
	}

	return 0;
}


int rd_ghost(void)
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


int rd_history(void)
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
		
		history_add_full(type, &a_info[art_name], dlev, clev, turn, text);
	}

	return 0;
}
