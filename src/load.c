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
#include "history.h"
#include "monster/mon-make.h"
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
 * Read an object, version 5 (added mimicking_o_idx)
 *
 * This function no longer attempts to "repair" old savefiles - the info
 * held in o_ptr is now authoritative.
 */
static int rd_item_5(object_type *o_ptr)
{
	uint8_t tmp8u;
	uint16_t tmp16u;

	uint8_t ego_idx;
	uint8_t art_idx;

	size_t i, j;

	char buf[128];

	uint8_t ver = 1;

	rd_uint16_t(&tmp16u);
	rd_uint8_t(&ver);
	assert(tmp16u == 0xffff);

	strip_uint8_ts(2);

	/* Location */
	rd_uint8_t(&o_ptr->iy);
	rd_uint8_t(&o_ptr->ix);

	/* Type/Subtype */
	rd_uint8_t(&o_ptr->tval);
	rd_uint8_t(&o_ptr->sval);
	for (i = 0; i < MAX_PVALS; i++) {
		rd_int16_t(&o_ptr->pval[i]);
	}
	rd_uint8_t(&o_ptr->num_pvals);

	/* Pseudo-ID bit */
	rd_uint8_t(&tmp8u);

	rd_uint8_t(&o_ptr->number);
	rd_int16_t(&o_ptr->weight);

	rd_uint8_t(&art_idx);
	rd_uint8_t(&ego_idx);

	rd_int16_t(&o_ptr->timeout);

	rd_int16_t(&o_ptr->to_h);
	rd_int16_t(&o_ptr->to_d);
	rd_int16_t(&o_ptr->to_a);

	rd_int16_t(&o_ptr->ac);

	rd_uint8_t(&o_ptr->dd);
	rd_uint8_t(&o_ptr->ds);

	rd_uint16_t(&o_ptr->ident);

	rd_uint8_t(&o_ptr->marked);

	rd_uint8_t(&o_ptr->origin);
	rd_uint8_t(&o_ptr->origin_depth);
	rd_uint16_t(&o_ptr->origin_xtra);
	rd_uint8_t(&o_ptr->ignore);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->known_flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			rd_uint8_t(&o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);
	}

	/* Monster holding object */
	rd_int16_t(&o_ptr->held_m_idx);

	rd_int16_t(&o_ptr->mimicking_m_idx);

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

/*
 * Read an object, version 4 (added mimicking_o_idx)
 *
 * This function no longer attempts to "repair" old savefiles - the info
 * held in o_ptr is now authoritative.
 */
static int rd_item_4(object_type *o_ptr)
{
	uint8_t tmp8u;
	uint16_t tmp16u;

	uint8_t ego_idx;
	uint8_t art_idx;

	size_t i, j;

	char buf[128];

	uint8_t ver = 1;

	rd_uint16_t(&tmp16u);
	rd_uint8_t(&ver);
	assert(tmp16u == 0xffff);

	strip_uint8_ts(2);

	/* Location */
	rd_uint8_t(&o_ptr->iy);
	rd_uint8_t(&o_ptr->ix);

	/* Type/Subtype */
	rd_uint8_t(&o_ptr->tval);
	rd_uint8_t(&o_ptr->sval);
	for (i = 0; i < MAX_PVALS; i++) {
		rd_int16_t(&o_ptr->pval[i]);
	}
	rd_uint8_t(&o_ptr->num_pvals);

	/* Pseudo-ID bit */
	rd_uint8_t(&tmp8u);

	rd_uint8_t(&o_ptr->number);
	rd_int16_t(&o_ptr->weight);

	rd_uint8_t(&art_idx);
	rd_uint8_t(&ego_idx);

	rd_int16_t(&o_ptr->timeout);

	rd_int16_t(&o_ptr->to_h);
	rd_int16_t(&o_ptr->to_d);
	rd_int16_t(&o_ptr->to_a);

	rd_int16_t(&o_ptr->ac);

	rd_uint8_t(&o_ptr->dd);
	rd_uint8_t(&o_ptr->ds);

	rd_uint16_t(&o_ptr->ident);

	rd_uint8_t(&o_ptr->marked);

	rd_uint8_t(&o_ptr->origin);
	rd_uint8_t(&o_ptr->origin_depth);
	rd_uint16_t(&o_ptr->origin_xtra);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->known_flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			rd_uint8_t(&o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);
	}

	/* Monster holding object */
	rd_int16_t(&o_ptr->held_m_idx);

	rd_int16_t(&o_ptr->mimicking_m_idx);

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

/*
 * Read an object, version 3
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 */
static int rd_item_3(object_type *o_ptr)
{
	uint8_t old_dd;
	uint8_t old_ds;
	uint8_t tmp8u;
	uint16_t tmp16u;

	uint8_t ego_idx;
	uint8_t art_idx;

	size_t i, j;

	char buf[128];

	uint8_t ver = 1;

	rd_uint16_t(&tmp16u);
	rd_uint8_t(&ver);
	assert(tmp16u == 0xffff);

	strip_uint8_ts(2);

	/* Location */
	rd_uint8_t(&o_ptr->iy);
	rd_uint8_t(&o_ptr->ix);

	/* Type/Subtype */
	rd_uint8_t(&o_ptr->tval);
	rd_uint8_t(&o_ptr->sval);
	for (i = 0; i < MAX_PVALS; i++) {
		rd_int16_t(&o_ptr->pval[i]);
	}
	rd_uint8_t(&o_ptr->num_pvals);

	/* Pseudo-ID bit */
	rd_uint8_t(&tmp8u);

	rd_uint8_t(&o_ptr->number);
	rd_int16_t(&o_ptr->weight);

	rd_uint8_t(&art_idx);
	rd_uint8_t(&ego_idx);

	rd_int16_t(&o_ptr->timeout);

	rd_int16_t(&o_ptr->to_h);
	rd_int16_t(&o_ptr->to_d);
	rd_int16_t(&o_ptr->to_a);

	rd_int16_t(&o_ptr->ac);

	rd_uint8_t(&old_dd);
	rd_uint8_t(&old_ds);

	rd_uint16_t(&o_ptr->ident);

	rd_uint8_t(&o_ptr->marked);

	rd_uint8_t(&o_ptr->origin);
	rd_uint8_t(&o_ptr->origin_depth);
	rd_uint16_t(&o_ptr->origin_xtra);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->known_flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			rd_uint8_t(&o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);
	}

	/* Monster holding object */
	rd_int16_t(&o_ptr->held_m_idx);
	
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
	uint8_t old_dd;
	uint8_t old_ds;
	uint8_t tmp8u;
	uint16_t tmp16u;

	uint8_t ego_idx;
	uint8_t art_idx;

	size_t i, j;

	char buf[128];

	uint8_t ver = 1;

	rd_uint16_t(&tmp16u);
	rd_uint8_t(&ver);
	assert(tmp16u == 0xffff);

	strip_uint8_ts(2);

	/* Location */
	rd_uint8_t(&o_ptr->iy);
	rd_uint8_t(&o_ptr->ix);

	/* Type/Subtype */
	rd_uint8_t(&o_ptr->tval);
	rd_uint8_t(&o_ptr->sval);
	for (i = 0; i < MAX_PVALS; i++) {
		rd_int16_t(&o_ptr->pval[i]);
	}
	rd_uint8_t(&o_ptr->num_pvals);

	/* Pseudo-ID bit */
	rd_uint8_t(&tmp8u);

	rd_uint8_t(&o_ptr->number);
	rd_int16_t(&o_ptr->weight);

	rd_uint8_t(&art_idx);
	rd_uint8_t(&ego_idx);

	rd_int16_t(&o_ptr->timeout);

	rd_int16_t(&o_ptr->to_h);
	rd_int16_t(&o_ptr->to_d);
	rd_int16_t(&o_ptr->to_a);

	rd_int16_t(&o_ptr->ac);

	rd_uint8_t(&old_dd);
	rd_uint8_t(&old_ds);

	rd_uint16_t(&o_ptr->ident);

	rd_uint8_t(&o_ptr->marked);

	rd_uint8_t(&o_ptr->origin);
	rd_uint8_t(&o_ptr->origin_depth);
	rd_uint16_t(&o_ptr->origin_xtra);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	of_wipe(o_ptr->known_flags);

	for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->known_flags[i]);
	if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);

	for (j = 0; j < MAX_PVALS; j++) {
		for (i = 0; i < OF_BYTES && i < OF_SIZE; i++)
			rd_uint8_t(&o_ptr->pval_flags[j][i]);
		if (i < OF_BYTES) strip_uint8_ts(OF_BYTES - i);
	}

	/* Monster holding object */
	rd_int16_t(&o_ptr->held_m_idx);

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
	uint8_t old_dd;
	uint8_t old_ds;
	uint8_t tmp8u;
	uint16_t tmp16u;

	uint8_t art_idx;
	uint8_t ego_idx;

	size_t i;

	char buf[128];

	uint8_t ver = 1;

	rd_uint16_t(&tmp16u);
	rd_uint8_t(&ver);
	assert(tmp16u == 0xffff);


	strip_uint8_ts(2);

	/* Location */
	rd_uint8_t(&o_ptr->iy);
	rd_uint8_t(&o_ptr->ix);

	/* Type/Subtype */
	rd_uint8_t(&o_ptr->tval);
	rd_uint8_t(&o_ptr->sval);
	rd_int16_t(&o_ptr->pval[DEFAULT_PVAL]);

	if (o_ptr->pval[DEFAULT_PVAL])
		o_ptr->num_pvals = 1;
	else
		o_ptr->num_pvals = 0;

	/* Pseudo-ID bit */
	rd_uint8_t(&tmp8u);

	rd_uint8_t(&o_ptr->number);
	rd_int16_t(&o_ptr->weight);

	rd_uint8_t(&art_idx);
	rd_uint8_t(&ego_idx);

	rd_int16_t(&o_ptr->timeout);

	rd_int16_t(&o_ptr->to_h);
	rd_int16_t(&o_ptr->to_d);
	rd_int16_t(&o_ptr->to_a);

	rd_int16_t(&o_ptr->ac);

	rd_uint8_t(&old_dd);
	rd_uint8_t(&old_ds);

	rd_uint16_t(&o_ptr->ident);

	rd_uint8_t(&o_ptr->marked);

	rd_uint8_t(&o_ptr->origin);
	rd_uint8_t(&o_ptr->origin_depth);
	rd_uint16_t(&o_ptr->origin_xtra);

	/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
	for (i = 0; i < 12 && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->flags[i]);
	if (i < 12) strip_uint8_ts(12 - i);

	of_wipe(o_ptr->known_flags);

	/* Hack - XXX - MarbleDice - Maximum saveable flags = 96 */
	for (i = 0; i < 12 && i < OF_SIZE; i++)
		rd_uint8_t(&o_ptr->known_flags[i]);
	if (i < 12) strip_uint8_ts(12 - i);


	/* Monster holding object */
	rd_int16_t(&o_ptr->held_m_idx);

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
 * There were originally 64 uint8_ts of randomizer saved. Now we only need
 * 32 + 5 uint8_ts saved, so we'll read an extra 27 uint8_ts at the end which won't
 * be used.
 */
int rd_randomizer(void)
{
	int i;
	uint32_t noop;

	/* current value for the simple RNG */
	rd_uint32_t(&Rand_value);

	/* state index */
	rd_uint32_t(&state_i);

	/* for safety, make sure state_i < RAND_DEG */
	state_i = state_i % RAND_DEG;
    
	/* RNG variables */
	rd_uint32_t(&z0);
	rd_uint32_t(&z1);
	rd_uint32_t(&z2);
    
	/* RNG state */
	for (i = 0; i < RAND_DEG; i++)
		rd_uint32_t(&STATE[i]);

	/* NULL padding */
	for (i = 0; i < 59 - RAND_DEG; i++)
		rd_uint32_t(&noop);

	Rand_quick = FALSE;

	return 0;
}


/*
 * Read options, version 2.
 */
int rd_options_2(void)
{
	int i, n;

	uint8_t b;

	uint16_t tmp16u;

	uint32_t window_flag[ANGBAND_TERM_MAX];
	uint32_t window_mask[ANGBAND_TERM_MAX];


	/*** Special info */

	/* Read "delay_factor" */
	rd_uint8_t(&b);
	op_ptr->delay_factor = b;

	/* Read "hitpoint_warn" */
	rd_uint8_t(&b);
	op_ptr->hitpoint_warn = b;

	/* Read lazy movement delay */
	rd_uint16_t(&tmp16u);
	lazymove_delay = (tmp16u < 1000) ? tmp16u : 0;


	/*** Normal Options ***/

	while (1) {
		uint8_t value;
		char name[20];
		rd_string(name, sizeof name);

		if (!name[0])
			break;

		rd_uint8_t(&value);
		option_set(name, !!value);
	}

	/*** Window Options ***/

	for (n = 0; n < ANGBAND_TERM_MAX; n++)
		rd_uint32_t(&window_flag[n]);
	for (n = 0; n < ANGBAND_TERM_MAX; n++)
		rd_uint32_t(&window_mask[n]);

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
	uint16_t tmp16u;
	
	int16_t num;
	
	/* Total */
	rd_int16_t(&num);
	
	/* Read the messages */
	for (i = 0; i < num; i++)
	{
		/* Read the message */
		rd_string(buf, sizeof(buf));
		
		/* Read the message type */
		rd_uint16_t(&tmp16u);

		/* Save the message */
		message_add(buf, tmp16u);
	}
	
	return 0;
}

/* Read monster memory, version 2 */
int rd_monster_memory_2(void)
{
	int r_idx;
	uint16_t tmp16u;
	
	/* Monster Memory */
	rd_uint16_t(&tmp16u);
	
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
		rd_int16_t(&l_ptr->sights);
		rd_int16_t(&l_ptr->deaths);
		rd_int16_t(&l_ptr->pkills);
		rd_int16_t(&l_ptr->tkills);

		/* Count wakes and ignores */
		rd_uint8_t(&l_ptr->wake);
		rd_uint8_t(&l_ptr->ignore);

		/* Count drops */
		rd_uint8_t(&l_ptr->drop_gold);
		rd_uint8_t(&l_ptr->drop_item);

		/* Count spells */
		rd_uint8_t(&l_ptr->cast_innate);
		rd_uint8_t(&l_ptr->cast_spell);

		/* Count blows of each type */
		for (i = 0; i < MONSTER_BLOW_MAX; i++)
			rd_uint8_t(&l_ptr->blows[i]);

		/* Memorize flags */
		for (i = 0; i < RF_BYTES && i < RF_SIZE; i++)
			rd_uint8_t(&l_ptr->flags[i]);
		if (i < RF_BYTES) strip_uint8_ts(RF_BYTES - i);

		for (i = 0; i < RF_BYTES && i < RSF_SIZE; i++)
			rd_uint8_t(&l_ptr->spell_flags[i]);
		if (i < RF_BYTES) strip_uint8_ts(RF_BYTES - i);

		/* Read the "Racial" monster limit per level */
		rd_uint8_t(&r_ptr->max_num);

		/* XXX */
		strip_uint8_ts(3);

		/* Repair the spell lore flags */
		rsf_inter(l_ptr->spell_flags, r_ptr->spell_flags);
	}
	
	return 0;
}


int rd_object_memory(void)
{
	int i;
	uint16_t tmp16u;
	
	/* Object Memory */
	rd_uint16_t(&tmp16u);
	
	/* Incompatible save files */
	if (tmp16u > z_info->k_max)
	{
		note(format("Too many (%u) object kinds!", tmp16u));
		return (-1);
	}
	
	/* Read the object memory */
	for (i = 0; i < tmp16u; i++)
	{
		uint8_t tmp8u;
		object_kind *k_ptr = &k_info[i];
		
		rd_uint8_t(&tmp8u);
		
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
	uint16_t tmp16u;
	
	/* Load the Quests */
	rd_uint16_t(&tmp16u);
	
	/* Incompatible save files */
	if (tmp16u > MAX_Q_IDX)
	{
		note(format("Too many (%u) quests!", tmp16u));
		return (-1);
	}
	
	/* Load the Quests */
	for (i = 0; i < tmp16u; i++)
	{
		uint8_t tmp8u;
		
		rd_uint8_t(&tmp8u);
		q_list[i].level = tmp8u;
		rd_uint8_t(&tmp8u);
		rd_uint8_t(&tmp8u);
		rd_uint8_t(&tmp8u);
	}
	
	return 0;
}


int rd_artifacts(void)
{
	int i;
	uint16_t tmp16u;
	
	/* Load the Artifacts */
	rd_uint16_t(&tmp16u);
	
	/* Incompatible save files */
	if (tmp16u > z_info->a_max)
	{
		note(format("Too many (%u) artifacts!", tmp16u));
		return (-1);
	}
	
	/* Read the artifact flags */
	for (i = 0; i < tmp16u; i++)
	{
		uint8_t tmp8u;
		
		rd_uint8_t(&tmp8u);
		a_info[i].created = tmp8u;
		rd_uint8_t(&tmp8u);
		a_info[i].seen = tmp8u;
		rd_uint8_t(&tmp8u);
		a_info[i].everseen = tmp8u;
		rd_uint8_t(&tmp8u);
	}

	return 0;
}


/*
 * Read the "extra" information
 */
int rd_player(void)
{
	int i;
	uint8_t num;

	rd_string(op_ptr->full_name, sizeof(op_ptr->full_name));
	rd_string(p_ptr->died_from, 80);
	p_ptr->history = mem_zalloc(250);
	rd_string(p_ptr->history, 250);

	/* Player race */
	rd_uint8_t(&num);
	p_ptr->race = player_id2race(num);

	/* Verify player race */
	if (!p_ptr->race) {
		note(format("Invalid player race (%d).", num));
		return -1;
	}

	/* Player class */
	rd_uint8_t(&num);
	p_ptr->class = player_id2class(num);

	if (!p_ptr->class) {
		note(format("Invalid player class (%d).", num));
		return -1;
	}

	/* Player gender */
	rd_uint8_t(&p_ptr->psex);
	p_ptr->sex = &sex_info[p_ptr->psex];

	/* Numeric name suffix */
	rd_uint8_t(&op_ptr->name_suffix);

	/* Special Race/Class info */
	rd_uint8_t(&p_ptr->hitdie);
	rd_uint8_t(&p_ptr->expfact);

	/* Age/Height/Weight */
	rd_int16_t(&p_ptr->age);
	rd_int16_t(&p_ptr->ht);
	rd_int16_t(&p_ptr->wt);

	/* Read the stat info */
	for (i = 0; i < A_MAX; i++) rd_int16_t(&p_ptr->stat_max[i]);
	for (i = 0; i < A_MAX; i++) rd_int16_t(&p_ptr->stat_cur[i]);
	for (i = 0; i < A_MAX; i++) rd_int16_t(&p_ptr->stat_birth[i]);

	rd_int16_t(&p_ptr->ht_birth);
	rd_int16_t(&p_ptr->wt_birth);
	strip_uint8_ts(2);
	rd_int32_t(&p_ptr->au_birth);

	strip_uint8_ts(4);

	rd_int32_t(&p_ptr->au);

	rd_int32_t(&p_ptr->max_exp);
	rd_int32_t(&p_ptr->exp);
	rd_uint16_t(&p_ptr->exp_frac);

	rd_int16_t(&p_ptr->lev);

	/* Verify player level */
	if ((p_ptr->lev < 1) || (p_ptr->lev > PY_MAX_LEVEL))
	{
		note(format("Invalid player level (%d).", p_ptr->lev));
		return (-1);
	}

	rd_int16_t(&p_ptr->mhp);
	rd_int16_t(&p_ptr->chp);
	rd_uint16_t(&p_ptr->chp_frac);

	rd_int16_t(&p_ptr->msp);
	rd_int16_t(&p_ptr->csp);
	rd_uint16_t(&p_ptr->csp_frac);

	rd_int16_t(&p_ptr->max_lev);
	rd_int16_t(&p_ptr->max_depth);

	/* Hack -- Repair maximum player level */
	if (p_ptr->max_lev < p_ptr->lev) p_ptr->max_lev = p_ptr->lev;

	/* Hack -- Repair maximum dungeon level */
	if (p_ptr->max_depth < 0) p_ptr->max_depth = 1;

	/* More info */
	strip_uint8_ts(10);
	rd_int16_t(&p_ptr->deep_descent);

	/* Read the flags */
	rd_int16_t(&p_ptr->food);
	rd_int16_t(&p_ptr->energy);
	rd_int16_t(&p_ptr->word_recall);
	rd_int16_t(&p_ptr->state.see_infra);
	rd_uint8_t(&p_ptr->confusing);
	rd_uint8_t(&p_ptr->searching);

	/* Find the number of timed effects */
	rd_uint8_t(&num);

	if (num <= TMD_MAX)
	{
		/* Read all the effects */
		for (i = 0; i < num; i++)
			rd_int16_t(&p_ptr->timed[i]);

		/* Initialize any entries not read */
		if (num < TMD_MAX)
			C_WIPE(p_ptr->timed + num, TMD_MAX - num, int16_t);
	}
	else
	{
		/* Probably in trouble anyway */
		for (i = 0; i < TMD_MAX; i++)
			rd_int16_t(&p_ptr->timed[i]);

		/* Discard unused entries */
		strip_uint8_ts(2 * (num - TMD_MAX));
		note("Discarded unsupported timed effects");
	}

	/* Total energy used so far */
	rd_uint32_t(&p_ptr->total_energy);
	/* # of turns spent resting */
	rd_uint32_t(&p_ptr->resting_turn);

	/* Future use */
	strip_uint8_ts(32);

	return 0;
}


/*
 * Read squelch and autoinscription submenu for all known objects
 */
int rd_squelch(void)
{
	size_t i;
	uint8_t tmp8u = 24;
	uint16_t file_e_max;
	uint16_t inscriptions;
	
	/* Read how many squelch uint8_ts we have */
	rd_uint8_t(&tmp8u);
	
	/* Check against current number */
	if (tmp8u != squelch_size)
	{
		strip_uint8_ts(tmp8u);
	}
	else
	{
		for (i = 0; i < squelch_size; i++)
			rd_uint8_t(&squelch_level[i]);
	}
		
	/* Read the number of saved ego-item */
	rd_uint16_t(&file_e_max);
		
	for (i = 0; i < file_e_max; i++)
	{
		if (i < z_info->e_max)
		{
			uint8_t flags;
			
			/* Read and extract the flag */
			rd_uint8_t(&flags);
			e_info[i].everseen |= (flags & 0x02);
		}
	}
	
	/* Read the current number of auto-inscriptions */
	rd_uint16_t(&inscriptions);
	
	/* Read the autoinscriptions array */
	for (i = 0; i < inscriptions; i++)
	{
		char tmp[80];
		int16_t kidx;
		struct object_kind *k;
		
		rd_int16_t(&kidx);
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
	uint8_t tmp8u;
	
	/* Read the randart version */
	strip_uint8_ts(4);

	/* Read the randart seed */
	rd_uint32_t(&seed_randart);

	/* Skip the flags */
	strip_uint8_ts(12);


	/* Hack -- the two "special seeds" */
	rd_uint32_t(&seed_flavor);
	rd_uint32_t(&seed_town);


	/* Special stuff */
	rd_uint16_t(&p_ptr->panic_save);
	rd_uint16_t(&p_ptr->total_winner);
	rd_uint16_t(&p_ptr->noscore);


	/* Read "death" */
	rd_uint8_t(&tmp8u);
	p_ptr->is_dead = tmp8u;

	/* Read "feeling" */
	rd_uint8_t(&tmp8u);
	cave->feeling = tmp8u;

	rd_int32_t(&cave->created_at);

	/* Current turn */
	rd_int32_t(&turn);

	return 0;
}

int rd_misc_2(void)
{
	uint8_t tmp8u;
	uint16_t tmp16u;
	
	/* Read the randart version */
	strip_uint8_ts(4);

	/* Read the randart seed */
	rd_uint32_t(&seed_randart);

	/* Skip the flags */
	strip_uint8_ts(12);


	/* Hack -- the two "special seeds" */
	rd_uint32_t(&seed_flavor);
	rd_uint32_t(&seed_town);


	/* Special stuff */
	rd_uint16_t(&p_ptr->panic_save);
	rd_uint16_t(&p_ptr->total_winner);
	rd_uint16_t(&p_ptr->noscore);


	/* Read "death" */
	rd_uint8_t(&tmp8u);
	p_ptr->is_dead = tmp8u;

	/* Read "feeling" */
	rd_uint8_t(&tmp8u);
	cave->feeling = tmp8u;
	rd_uint16_t(&tmp16u);
	cave->feeling_squares = tmp16u;

	rd_int32_t(&cave->created_at);

	/* Current turn */
	rd_int32_t(&turn);

	return 0;
}

int rd_player_hp(void)
{
	int i;
	uint16_t tmp16u;

	/* Read the player_hp array */
	rd_uint16_t(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > PY_MAX_LEVEL)
	{
		note(format("Too many (%u) hitpoint entries!", tmp16u));
		return (-1);
	}

	/* Read the player_hp array */
	for (i = 0; i < tmp16u; i++)
		rd_int16_t(&p_ptr->player_hp[i]);

	return 0;
}


int rd_player_spells(void)
{
	int i;
	uint16_t tmp16u;
	
	int cnt;
	
	/* Read the number of spells */
	rd_uint16_t(&tmp16u);
	if (tmp16u > PY_MAX_SPELLS)
	{
		note(format("Too many player spells (%d).", tmp16u));
		return (-1);
	}
	
	/* Read the spell flags */
	for (i = 0; i < tmp16u; i++)
		rd_uint8_t(&p_ptr->spell_flags[i]);
	
	/* Read the spell order */
	for (i = 0, cnt = 0; i < tmp16u; i++, cnt++)
		rd_uint8_t(&p_ptr->spell_order[cnt]);
	
	/* Success */
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
		uint16_t n;

		/* Get the next item index */
		rd_uint16_t(&n);

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
int rd_inventory_5(void) { return rd_inventory(rd_item_5); }
int rd_inventory_4(void) { return rd_inventory(rd_item_4); }
int rd_inventory_3(void) { return rd_inventory(rd_item_3); }
int rd_inventory_2(void) { return rd_inventory(rd_item_2); } /* remove post-3.3 */
int rd_inventory_1(void) { return rd_inventory(rd_item_1); } /* remove post-3.3 */


/* Read store contents */
static int rd_stores(rd_item_t rd_item_version)
{
	int i;
	uint16_t tmp16u;
	
	/* Read the stores */
	rd_uint16_t(&tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		struct store *st_ptr = &stores[i];

		int j;		
		uint8_t own, num;
		
		/* XXX */
		strip_uint8_ts(6);
		
		/* Read the basic info */
		rd_uint8_t(&own);
		rd_uint8_t(&num);
		
		/* XXX */
		strip_uint8_ts(4);
		
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
int rd_stores_5(void) { return rd_stores(rd_item_5); }
int rd_stores_4(void) { return rd_stores(rd_item_4); }
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

	int16_t depth;
	int16_t py, px;
	int16_t ymax, xmax;

	uint8_t count;
	uint8_t tmp8u;
	uint16_t tmp16u;

	/* Only if the player's alive */
	if (p_ptr->is_dead)
		return 0;

	/*** Basic info ***/

	/* Header info */
	rd_int16_t(&depth);
	rd_uint16_t(&daycount);
	rd_int16_t(&py);
	rd_int16_t(&px);
	rd_int16_t(&ymax);
	rd_int16_t(&xmax);
	rd_uint16_t(&tmp16u);
	rd_uint16_t(&tmp16u);


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
		rd_uint8_t(&count);
		rd_uint8_t(&tmp8u);

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
		rd_uint8_t(&count);
		rd_uint8_t(&tmp8u);

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
		rd_uint8_t(&count);
		rd_uint8_t(&tmp8u);

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
	uint16_t limit;

	/* Only if the player's alive */
	if (p_ptr->is_dead)
		return 0;

	/* Read the item count */
	rd_uint16_t(&limit);

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

		int16_t o_idx;
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
int rd_objects_5(void) { return rd_objects(rd_item_5); }
int rd_objects_4(void) { return rd_objects(rd_item_4); }
int rd_objects_3(void) { return rd_objects(rd_item_3); }
int rd_objects_2(void) { return rd_objects(rd_item_2); } /* remove post-3.3 */
int rd_objects_1(void) { return rd_objects(rd_item_1); } /* remove post-3.3 */

/**
 * Read monsters (added m_ptr->mimicked_o_idx)
 */
int rd_monsters_6(void)
{
	int i;
	size_t j;
	uint16_t limit;

	/* Only if the player's alive */
	if (p_ptr->is_dead)
		return 0;
	
	/* Read the monster count */
	rd_uint16_t(&limit);

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
		int16_t r_idx;
		
		uint8_t flags;
		uint8_t tmp8u;

		/* Get local monster */
		m_ptr = &monster_type_body;
		WIPE(m_ptr, monster_type);

		/* Read in record */
		rd_int16_t(&r_idx);
		m_ptr->race = &r_info[r_idx];
		rd_uint8_t(&m_ptr->fy);
		rd_uint8_t(&m_ptr->fx);
		rd_int16_t(&m_ptr->hp);
		rd_int16_t(&m_ptr->maxhp);
		rd_uint8_t(&m_ptr->mspeed);
		rd_uint8_t(&m_ptr->energy);
		rd_uint8_t(&tmp8u);

		for (j = 0; j < tmp8u; j++)
			rd_int16_t(&m_ptr->m_timed[j]);

		/* Read and extract the flag */
		rd_uint8_t(&flags);
		m_ptr->unaware = (flags & 0x01) ? TRUE : FALSE;
	
		for (j = 0; j < OF_BYTES && j < OF_SIZE; j++)
			rd_uint8_t(&m_ptr->known_pflags[j]);
		if (j < OF_BYTES) strip_uint8_ts(OF_BYTES - j);
		
		strip_uint8_ts(1);

		/* Place monster in dungeon */
		if (place_monster(m_ptr->fy, m_ptr->fx, m_ptr, 0) != i)
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
	uint32_t tmp32u;
	size_t i;
	
	history_clear();
	
	rd_uint32_t(&tmp32u);
	for (i = 0; i < tmp32u; i++)
	{
		int32_t turnno;
		int16_t dlev, clev;
		uint16_t type;
		uint8_t art_name;
		char text[80];
		
		rd_uint16_t(&type);
		rd_int32_t(&turnno);
		rd_int16_t(&dlev);
		rd_int16_t(&clev);
		rd_uint8_t(&art_name);
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
