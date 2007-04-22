/* File: load.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * This file loads savefiles from Angband 2.9.X.
 *
 * We attempt to prevent corrupt savefiles from inducing memory errors.
 *
 * Note that this file should not use the random number generator, the
 * object flavors, the visual attr/char mappings, or anything else which
 * is initialized *after* or *during* the "load character" function.
 *
 * This file assumes that the monster/object records are initialized
 * to zero, and the race/kind tables have been loaded correctly.  The
 * order of object stacks is currently not saved in the savefiles, but
 * the "next" pointers are saved, so all necessary knowledge is present.
 *
 * We should implement simple "savefile extenders" using some form of
 * "sized" chunks of bytes, with a {size,type,data} format, so everyone
 * can know the size, interested people can know the type, and the actual
 * data is available to the parsing routines that acknowledge the type.
 *
 * Consider changing the "globe of invulnerability" code so that it
 * takes some form of "maximum damage to protect from" in addition to
 * the existing "number of turns to protect for", and where each hit
 * by a monster will reduce the shield by that amount.  XXX XXX XXX
 */





/*
 * Local "savefile" pointer
 */
static FILE	*fff;

/*
 * Hack -- old "encryption" byte
 */
static byte	xor_byte;

/*
 * Hack -- simple "checksum" on the actual values
 */
static u32b	v_check = 0L;

/*
 * Hack -- simple "checksum" on the encoded bytes
 */
static u32b	x_check = 0L;


/*
 * Hack -- Show information on the screen, one line at a time.
 *
 * Avoid the top two lines, to avoid interference with "msg_print()".
 */
static void note(cptr msg)
{
	static int y = 2;

	/* Draw the message */
	prt(msg, y, 0);

	/* Advance one line (wrap if needed) */
	if (++y >= 24) y = 2;

	/* Flush it */
	Term_fresh();
}


/*
 * This function determines if the version of the savefile
 * currently being read is older than version "x.y.z".
 */
static bool older_than(int x, int y, int z)
{
	/* Much older, or much more recent */
	if (sf_major < x) return (TRUE);
	if (sf_major > x) return (FALSE);

	/* Distinctly older, or distinctly more recent */
	if (sf_minor < y) return (TRUE);
	if (sf_minor > y) return (FALSE);

	/* Barely older, or barely more recent */
	if (sf_patch < z) return (TRUE);
	if (sf_patch > z) return (FALSE);

	/* Identical versions */
	return (FALSE);
}


/*
 * Hack -- determine if an item is "wearable" (or a missile)
 */
static bool wearable_p(const object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_LITE:
		case TV_AMULET:
		case TV_RING:
		{
			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}


/*
 * The following functions are used to load the basic building blocks
 * of savefiles.  They also maintain the "checksum" info.
 */

static byte sf_get(void)
{
	byte c, v;

	/* Get a character, decode the value */
	c = getc(fff) & 0xFF;
	v = c ^ xor_byte;
	xor_byte = c;

	/* Maintain the checksum info */
	v_check += v;
	x_check += xor_byte;

	/* Return the value */
	return (v);
}

static void rd_byte(byte *ip)
{
	*ip = sf_get();
}

static void rd_u16b(u16b *ip)
{
	(*ip) = sf_get();
	(*ip) |= ((u16b)(sf_get()) << 8);
}

static void rd_s16b(s16b *ip)
{
	rd_u16b((u16b*)ip);
}

static void rd_u32b(u32b *ip)
{
	(*ip) = sf_get();
	(*ip) |= ((u32b)(sf_get()) << 8);
	(*ip) |= ((u32b)(sf_get()) << 16);
	(*ip) |= ((u32b)(sf_get()) << 24);
}

static void rd_s32b(s32b *ip)
{
	rd_u32b((u32b*)ip);
}


/*
 * Hack -- read a string
 */
static void rd_string(char *str, int max)
{
	int i;

	/* Read the string */
	for (i = 0; TRUE; i++)
	{
		byte tmp8u;

		/* Read a byte */
		rd_byte(&tmp8u);

		/* Collect string while legal */
		if (i < max) str[i] = tmp8u;

		/* End of string */
		if (!tmp8u) break;
	}

	/* Terminate */
	str[max-1] = '\0';
}


/*
 * Hack -- strip some bytes
 */
static void strip_bytes(int n)
{
	byte tmp8u;

	/* Strip the bytes */
	while (n--) rd_byte(&tmp8u);
}


/*
 * Read an object
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 */
static errr rd_item(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;

	u32b f1, f2, f3;

	object_kind *k_ptr;

	char buf[128];


	/* Kind */
	rd_s16b(&o_ptr->k_idx);

	/* Paranoia */
	if ((o_ptr->k_idx < 0) || (o_ptr->k_idx >= z_info->k_max))
		return (-1);

	/* Location */
	rd_byte(&o_ptr->iy);
	rd_byte(&o_ptr->ix);

	/* Type/Subtype */
	rd_byte(&o_ptr->tval);
	rd_byte(&o_ptr->sval);

	/* Special pval */
	rd_s16b(&o_ptr->pval);


	rd_byte(&o_ptr->discount);

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

	rd_byte(&o_ptr->ident);

	rd_byte(&o_ptr->marked);

	/* Old flags */
	strip_bytes(12);

	/* Monster holding object */
	rd_s16b(&o_ptr->held_m_idx);

	/* Special powers */
	rd_byte(&o_ptr->xtra1);
	rd_byte(&o_ptr->xtra2);

	/* Inscription */
	rd_string(buf, sizeof(buf));

	/* Save the inscription */
	if (buf[0]) o_ptr->note = quark_add(buf);

	/* Obtain the "kind" template */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Obtain tval/sval from k_info */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;


	/* Hack -- notice "broken" items */
	if (k_ptr->cost <= 0) o_ptr->ident |= (IDENT_BROKEN);


	/*
	 * Ensure that rods and wands get the appropriate pvals,
	 * and transfer rod charges to timeout.
	 * this test should only be passed once, the first
	 * time the file is open with ROD/WAND stacking code
	 * It could change the timeout improperly if the PVAL (time a rod
	 * takes to charge after use) is changed in object.txt.
	 * But this is nothing a little resting won't solve.
	 *
	 * -JG-
	 */
	if ((o_ptr->tval == TV_ROD) &&
	    (o_ptr->pval - (k_ptr->pval * o_ptr->number) != 0))
	{
		o_ptr->timeout = o_ptr->pval;
		o_ptr->pval = k_ptr->pval * o_ptr->number;
	}

	if (older_than(3, 0, 4))
	{
		/* Recalculate charges of stacked wands and staves */
		if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
		{
			o_ptr->pval = o_ptr->pval * o_ptr->number;
		}
	}

	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Get the correct fields */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

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


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);


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

	/* Hack -- extract the "broken" flag */
	if (o_ptr->pval < 0) o_ptr->ident |= (IDENT_BROKEN);


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

		/* Hack -- extract the "broken" flag */
		if (!a_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);
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

		/* Hack -- extract the "broken" flag */
		if (!e_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

		/* Hack -- enforce legal pval */
		if (e_ptr->flags1 & (TR1_PVAL_MASK))
		{
			/* Force a meaningful pval */
			if (!o_ptr->pval) o_ptr->pval = 1;
		}

		/* Mega-Hack - Enforce the special broken items */
		if ((o_ptr->name2 == EGO_BLASTED) ||
			(o_ptr->name2 == EGO_SHATTERED))
		{
			/* These were set to k_info values by preceding code */
			o_ptr->ac = 0;
			o_ptr->dd = 0;
			o_ptr->ds = 0;
		}
	}

	if(older_than(3, 0, 9) && o_ptr->tval == TV_LITE && !artifact_p(o_ptr) && !ego_item_p(o_ptr) && o_ptr->pval)
	{
		o_ptr->timeout = o_ptr->pval;
		o_ptr->pval = 0;
	}


	/* Success */
	return (0);
}




/*
 * Read a monster
 */
static void rd_monster(monster_type *m_ptr)
{
	byte tmp8u;

	/* Read the monster race */
	rd_s16b(&m_ptr->r_idx);

	/* Read the other information */
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
	rd_byte(&tmp8u);
}





/*
 * Read the monster lore
 */
static void rd_lore(int r_idx)
{
	int i;
	byte tmp8u;
	
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

	/* Extra stuff */
	rd_byte(&l_ptr->xtra1);
	rd_byte(&l_ptr->xtra2);

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
	rd_u32b(&l_ptr->flags1);
	rd_u32b(&l_ptr->flags2);
	rd_u32b(&l_ptr->flags3);
	rd_u32b(&l_ptr->flags4);
	rd_u32b(&l_ptr->flags5);
	rd_u32b(&l_ptr->flags6);


	/* Read the "Racial" monster limit per level */
	rd_byte(&r_ptr->max_num);

	/* Later (?) */
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);
	rd_byte(&tmp8u);


	/* Repair the lore flags */
	l_ptr->flags1 &= r_ptr->flags1;
	l_ptr->flags2 &= r_ptr->flags2;
	l_ptr->flags3 &= r_ptr->flags3;
	l_ptr->flags4 &= r_ptr->flags4;
	l_ptr->flags5 &= r_ptr->flags5;
	l_ptr->flags6 &= r_ptr->flags6;
}




/*
 * Read a store
 */
static errr rd_store(int n)
{
	store_type *st_ptr = &store[n];

	int j;

	byte own, num;

	/* XXX Old values */
	strip_bytes(6);

	/* Read the basic info */
	rd_byte(&own);
	rd_byte(&num);

	/* XXX Old values */
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

		/* Pre-3.0.2 objects in store inventories need to be marked */
		if (older_than(3, 0, 2))
		{
			if (n != STORE_HOME) i_ptr->ident |= IDENT_STORE;
		}

		/* Accept any valid items */
		if (st_ptr->stock_num < STORE_INVEN_MAX)
		{
			int k = st_ptr->stock_num++;

			/* Accept the item */
			object_copy(&st_ptr->stock[k], i_ptr);
		}
	}

	/* Success */
	return (0);
}



/*
 * Read RNG state
 */
static void rd_randomizer(void)
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
static void rd_options(void)
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

	/* Old cheating options */
	rd_u16b(&tmp16u);


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

	/* Load savefiles pre-reorganisation */
	if (older_than(3, 0, 8))
	{
		/* 
		 * Slot	Old layout:	New layout:
		 * 0	xxx			maximise
		 * 1	xxx			randarts
		 * 2	maximise	autoscum
		 * 3	preserve	ironman
		 * 4	ironman		no_stores
		 * 5	no_stores	no_artifacts
		 * 6	no_artifats	no_stacking
		 * 7	randarts	no_preserve
		 * 8	no_stacking	no_stairs
		 */

		/* We #define this so it's obvious what we're doing */
		#define OLD_OPT(n)	op_ptr->opt[n]

		bool old_birth[9];
		for (i = 0; i <= 8; i++)
			old_birth[i] = OLD_OPT(OPT_BIRTH + i);

		if (arg_fiddle) note("Loading pre-3.0.7 options...");

		birth_maximize = adult_maximize = old_birth[2];
		birth_no_preserve = adult_no_preserve = !old_birth[3];
		birth_ironman = adult_ironman = old_birth[4];
		birth_no_stores = adult_no_stores = old_birth[5];
		birth_no_artifacts = adult_no_artifacts = old_birth[6];
		birth_randarts = adult_randarts = old_birth[7];
		birth_no_stacking = adult_no_stacking = old_birth[8];

		birth_no_stairs = adult_no_stairs = !OLD_OPT(41);
		birth_autoscum = adult_autoscum = OLD_OPT(33);
		birth_ai_sound = adult_ai_sound = OLD_OPT(42);
		birth_ai_smell = adult_ai_smell = OLD_OPT(43);
		birth_ai_packs = adult_ai_packs = OLD_OPT(73);
		birth_ai_learn = adult_ai_learn = OLD_OPT(46);
		birth_ai_cheat = adult_ai_cheat = OLD_OPT(47);
		birth_ai_smart = adult_ai_smart = OLD_OPT(72);

		#undef OLD_OPT

		if (arg_fiddle)
		{
			FILE *ffff = fopen("options.txt", "wb");
			int i;

			for (i = 0; i < OPT_SCORE; i++)
				fprintf(ffff, "%3d %s: %s\n",
					i,
					(op_ptr->opt[i] ? "on " : "off"),
					(option_text[i] ? option_text[i] : "NULL"));
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
				/* Process valid flags */
				if (window_mask[n] & (1L << i))
				{
					/* Set */
					if (window_flag[n] & (1L << i))
					{
						/* Set */
						op_ptr->window_flag[n] |= (1L << i);
					}
				}
			}
		}
	}
}





/*
 * Hack -- strip the "ghost" info
 *
 * XXX XXX XXX This is such a nasty hack it hurts.
 */
static void rd_ghost(void)
{
	char buf[64];

	/* Strip name */
	rd_string(buf, 64);

	/* Strip old data */
	strip_bytes(60);
}


static u32b randart_version;


static errr rd_player_spells(void)
{
	int i;
	u16b tmp16u;

	if (older_than(2, 9, 8))
	{
		/* The magic spells were changed drastically in Angband 2.9.7 */
		if (older_than(2, 9, 7) &&
		    (c_info[p_ptr->pclass].spell_book == TV_MAGIC_BOOK))
		{
			/* Discard old spell info */
			strip_bytes(24);

			/* Discard old spell order */
			strip_bytes(64);

			/* None of the spells have been learned yet */
			for (i = 0; i < 64; i++)
				p_ptr->spell_order[i] = 99;
		}
		else
		{
			u32b spell_learned1, spell_learned2;
			u32b spell_worked1, spell_worked2;
			u32b spell_forgotten1, spell_forgotten2;

			/* Read spell info */
			rd_u32b(&spell_learned1);
			rd_u32b(&spell_learned2);
			rd_u32b(&spell_worked1);
			rd_u32b(&spell_worked2);
			rd_u32b(&spell_forgotten1);
			rd_u32b(&spell_forgotten2);

			for (i = 0; i < 64; i++)
			{
				if (i < 32)
				{
					if (spell_learned1 & (1L << i))
						p_ptr->spell_flags[i] |= PY_SPELL_LEARNED;
					if (spell_worked1 & (1L << i))
						p_ptr->spell_flags[i] |= PY_SPELL_WORKED;
					if (spell_forgotten1 & (1L << i))
						p_ptr->spell_flags[i] |= PY_SPELL_FORGOTTEN;
				}
				else
				{
					if (spell_learned2 & (1L << (i - 32)))
						p_ptr->spell_flags[i] |= PY_SPELL_LEARNED;
					if (spell_worked2 & (1L << (i - 32)))
						p_ptr->spell_flags[i] |= PY_SPELL_WORKED;
					if (spell_forgotten2 & (1L << (i - 32)))
						p_ptr->spell_flags[i] |= PY_SPELL_FORGOTTEN;
				}
			}

			for (i = 0; i < 64; i++)
			{
				rd_byte(&p_ptr->spell_order[i]);
			}
		}
	}
	else
	{
		/* Read the number of spells */
		rd_u16b(&tmp16u);
		if (tmp16u > PY_MAX_SPELLS)
		{
			note(format("Too many player spells (%d).", tmp16u));
			return (-1);
		}

		/* Read the spell flags */
		for (i = 0; i < tmp16u; i++)
		{
			rd_byte(&p_ptr->spell_flags[i]);
		}

		/* Read the spell order */
		for (i = 0; i < tmp16u; i++)
		{
			rd_byte(&p_ptr->spell_order[i]);
		}
	}

	/* Success */
	return (0);
}

/*
 * Read squelch and autoinscription submenu for all known objects
 */
static int rd_squelch(void)
{
	int i;
	byte tmp8u;
	u16b file_e_max;

	/* Handle old versions, and Pete Mack's patch */
	if (older_than(3, 0, 6))
		return 0;

	/* Read item-quality squelch sub-menu */
	for (i = 0; i < SQUELCH_BYTES; i++)
		rd_byte(&squelch_level[i]);

	/* Read the number of saved ego-item types */
	rd_u16b(&file_e_max);

	/* Read ego-item squelch settings */
	for (i = 0; i < z_info->e_max; i++)
	{
		ego_item_type *e_ptr = &e_info[i];
		tmp8u = 0;

		if (i < file_e_max)
			rd_byte(&tmp8u);

		e_ptr->squelch |= (tmp8u & 0x01);
		e_ptr->everseen |= (tmp8u & 0x02);

		/* Hack - Repair the savefile */
		if (!e_ptr->everseen) e_ptr->squelch = FALSE;
	}

	/* Read possible extra elements */
	while (i < file_e_max)
	{
		rd_byte(&tmp8u);
		i++;
	}

	/* Read the current number of auto-inscriptions */
	rd_u16b(&inscriptions_count);

	/* Write the autoinscriptions array*/
	for (i = 0; i < inscriptions_count; i++)
	{
		 char tmp[80];

		 rd_s16b(&inscriptions[i].kind_idx);
		 rd_string(tmp, 80);

		 inscriptions[i].inscription_idx = quark_add(tmp);
	}

	return 0;
}


/*
 * Read the "extra" information
 */
static errr rd_extra(void)
{
	int i;

	byte tmp8u;
	u16b tmp16u;


	rd_string(op_ptr->full_name, sizeof(op_ptr->full_name));

	rd_string(p_ptr->died_from, 80);

	if (older_than(3, 0, 1))
	{
		char *hist = p_ptr->history;

		for (i = 0; i < 4; i++)
		{
			/* Read a part of the history */
			rd_string(hist, 60);

			/* Advance */
			hist += strlen(hist);

			/* Separate by spaces */
			hist[0] = ' ';
			hist++;
		}

		/* Make sure it is terminated */
		hist[0] = '\0';
	}
	else
	{
		rd_string(p_ptr->history, 250);
	}

	/* Player race */
	rd_byte(&p_ptr->prace);

	/* Verify player race */
	if (p_ptr->prace >= z_info->p_max)
	{
		note(format("Invalid player race (%d).", p_ptr->prace));
		return (-1);
	}

	/* Player class */
	rd_byte(&p_ptr->pclass);

	/* Verify player class */
	if (p_ptr->pclass >= z_info->c_max)
	{
		note(format("Invalid player class (%d).", p_ptr->pclass));
		return (-1);
	}

	/* Player gender */
	rd_byte(&p_ptr->psex);

	strip_bytes(1);

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

	strip_bytes(24);	/* oops */

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
	strip_bytes(2);

	/* Read the flags */
	if (older_than(3, 0, 7))
	{
		strip_bytes(2);	/* Old "rest" */
		rd_s16b(&p_ptr->timed[TMD_BLIND]);
		rd_s16b(&p_ptr->timed[TMD_PARALYZED]);
		rd_s16b(&p_ptr->timed[TMD_CONFUSED]);
		rd_s16b(&p_ptr->food);
		strip_bytes(4);	/* Old "food_digested" / "protection" */
		rd_s16b(&p_ptr->energy);
		rd_s16b(&p_ptr->timed[TMD_FAST]);
		rd_s16b(&p_ptr->timed[TMD_SLOW]);
		rd_s16b(&p_ptr->timed[TMD_AFRAID]);
		rd_s16b(&p_ptr->timed[TMD_CUT]);
		rd_s16b(&p_ptr->timed[TMD_STUN]);
		rd_s16b(&p_ptr->timed[TMD_POISONED]);
		rd_s16b(&p_ptr->timed[TMD_IMAGE]);
		rd_s16b(&p_ptr->timed[TMD_PROTEVIL]);
		rd_s16b(&p_ptr->timed[TMD_INVULN]);
		rd_s16b(&p_ptr->timed[TMD_HERO]);
		rd_s16b(&p_ptr->timed[TMD_SHERO]);
		rd_s16b(&p_ptr->timed[TMD_SHIELD]);
		rd_s16b(&p_ptr->timed[TMD_BLESSED]);
		rd_s16b(&p_ptr->timed[TMD_SINVIS]);
		rd_s16b(&p_ptr->word_recall);
		rd_s16b(&p_ptr->see_infra);
		rd_s16b(&p_ptr->timed[TMD_SINFRA]);
		rd_s16b(&p_ptr->timed[TMD_OPP_FIRE]);
		rd_s16b(&p_ptr->timed[TMD_OPP_COLD]);
		rd_s16b(&p_ptr->timed[TMD_OPP_ACID]);
		rd_s16b(&p_ptr->timed[TMD_OPP_ELEC]);
		rd_s16b(&p_ptr->timed[TMD_OPP_POIS]);

		rd_byte(&p_ptr->confusing);
		rd_byte(&tmp8u);	/* oops */
		rd_byte(&tmp8u);	/* oops */
		rd_byte(&tmp8u);	/* oops */
		rd_byte(&p_ptr->searching);
		rd_byte(&tmp8u);	/* oops */
		rd_byte(&tmp8u);	/* oops */
		rd_byte(&tmp8u);	/* oops */
	}
	else
	{
		byte num;
		int i;

		rd_s16b(&p_ptr->food);
		rd_s16b(&p_ptr->energy);
		rd_s16b(&p_ptr->word_recall);
		rd_s16b(&p_ptr->see_infra);
		rd_byte(&p_ptr->confusing);
		rd_byte(&p_ptr->searching);

		/* Find the number of timed effects */
		rd_byte(&num);

		/* Read all the effects, in a loop */
		for (i = 0; i < num; i++)
			rd_s16b(&p_ptr->timed[i]);
	}

	/* Future use */
	strip_bytes(40);

	/* Read item-quality squelch sub-menu */
	if (rd_squelch()) return -1;

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
	{
		rd_s16b(&p_ptr->player_hp[i]);
	}

	/* Read the player spells */
	if (rd_player_spells()) return (-1);

	return (0);
}


/*
 * Read the random artifacts
 */
static errr rd_randarts(void)
{

#ifdef GJW_RANDART

	int i;
	byte tmp8u;
	s16b tmp16s;
	u16b tmp16u;
	u16b artifact_count;
	s32b tmp32s;
	u32b tmp32u;


	if (older_than(2, 9, 3))
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

				rd_u32b(&a_ptr->flags1);
				rd_u32b(&a_ptr->flags2);
				rd_u32b(&a_ptr->flags3);

				rd_byte(&a_ptr->level);
				rd_byte(&a_ptr->rarity);

				rd_byte(&a_ptr->activation);
				rd_u16b(&a_ptr->time);
				rd_u16b(&a_ptr->randtime);
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

				rd_byte(&tmp8u); /* a_ptr->activation */
				rd_u16b(&tmp16u); /* a_ptr->time */
				rd_u16b(&tmp16u); /* a_ptr->randtime */
			}
		}

		/* Initialize only the randart names */
		do_randart(seed_randart, FALSE);
	}

	return (0);

#else /* GJW_RANDART */

	note("Random artifacts are disabled in this binary.");
	return (-1);

#endif /* GJW_RANDART */

}




/*
 * Read the player inventory
 *
 * Note that the inventory is "re-sorted" later by "dungeon()".
 */
static errr rd_inventory(void)
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
		if (!i_ptr->k_idx) return (-1);

		/* Verify slot */
		if (n >= INVEN_TOTAL) return (-1);

		/* Wield equipment */
		if (n >= INVEN_WIELD)
		{
			/* Copy object */
			object_copy(&inventory[n], i_ptr);

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
			object_copy(&inventory[n], i_ptr);

			/* Add the weight */
			p_ptr->total_weight += (i_ptr->number * i_ptr->weight);

			/* One more item */
			p_ptr->inven_cnt++;
		}
	}

	/* Success */
	return (0);
}



/*
 * Read the saved messages
 */
static void rd_messages(void)
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
		if (!older_than(2, 9, 1))
			rd_u16b(&tmp16u);
		else
			tmp16u = MSG_GENERIC;

		/* Save the message */
		message_add(buf, tmp16u);
	}
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
static errr rd_dungeon(void)
{
	int i, y, x;

	s16b depth;
	s16b py, px;
	s16b ymax, xmax;

	byte count;
	byte tmp8u;
	u16b tmp16u;

	u16b limit;


	/*** Basic info ***/

	/* Header info */
	rd_s16b(&depth);
	rd_u16b(&tmp16u);
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


	/*** Objects ***/

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


	/*** Monsters ***/

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
		monster_type *n_ptr;
		monster_type monster_type_body;


		/* Get local monster */
		n_ptr = &monster_type_body;

		/* Clear the monster */
		(void)WIPE(n_ptr, monster_type);

		/* Read the monster */
		rd_monster(n_ptr);


		/* Place monster in dungeon */
		if (monster_place(n_ptr->fy, n_ptr->fx, n_ptr) != i)
		{
			note(format("Cannot place monster %d", i));
			return (-1);
		}
	}


	/*** Holding ***/

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


	/*** Success ***/

	/* The dungeon is ready */
	character_dungeon = TRUE;

	/* Regenerate town in post 2.9.3 versions */
	if (older_than(2, 9, 4) && (p_ptr->depth == 0))
		character_dungeon = FALSE;

	/* Success */
	return (0);
}



/*
 * Actually read the savefile
 */
static errr rd_savefile_new_aux(void)
{
	int i;

	byte tmp8u;
	u16b tmp16u;
	u32b tmp32u;

	u32b n_x_check, n_v_check;
	u32b o_x_check, o_v_check;


	/* Mention the savefile version */
	note(format("Loading a %d.%d.%d savefile...",
	            sf_major, sf_minor, sf_patch));

	/* Strip the version bytes */
	strip_bytes(4);

	/* Hack -- decrypt */
	xor_byte = sf_extra;


	/* Clear the checksums */
	v_check = 0L;
	x_check = 0L;


	/* Operating system info */
	rd_u32b(&sf_xtra);

	/* Time of savefile creation */
	rd_u32b(&sf_when);

	/* Number of resurrections */
	rd_u16b(&sf_lives);

	/* Number of times played */
	rd_u16b(&sf_saves);


	/* Later use (always zero) */
	rd_u32b(&tmp32u);

	/* Later use (always zero) */
	rd_u32b(&tmp32u);


	/* Read RNG state */
	rd_randomizer();
	if (arg_fiddle) note("Loaded Randomizer Info");


	/* Then the options */
	rd_options();
	if (arg_fiddle) note("Loaded Option Flags");


	/* Then the "messages" */
	rd_messages();
	if (arg_fiddle) note("Loaded Messages");


	/* Monster Memory */
	rd_u16b(&tmp16u);

	/* Incompatible save files */
	if (tmp16u > z_info->r_max)
	{
		note(format("Too many (%u) monster races!", tmp16u));
		return (-1);
	}

	/* Read the available records */
	for (i = 0; i < tmp16u; i++)
	{
		/* Read the lore */
		rd_lore(i);
	}
	if (arg_fiddle) note("Loaded Monster Memory");


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

		if (!older_than(3, 0, 6))
			rd_byte(&k_ptr->squelch);
	}
	if (arg_fiddle) note("Loaded Object Memory");


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
		rd_byte(&tmp8u);
		q_list[i].level = tmp8u;
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
	}
	if (arg_fiddle) note("Loaded Quests");


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
		rd_byte(&tmp8u);
		a_info[i].cur_num = tmp8u;
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
	}
	if (arg_fiddle) note("Loaded Artifacts");


	/* Read the extra stuff */
	if (rd_extra()) return (-1);
	if (arg_fiddle) note("Loaded extra information");


	/* Read random artifacts */
	if (adult_randarts)
	{
		if (rd_randarts()) return (-1);
		if (arg_fiddle) note("Loaded Random Artifacts");
	}


	/* Important -- Initialize the sex */
	sp_ptr = &sex_info[p_ptr->psex];

	/* Important -- Initialize the race/class */
	rp_ptr = &p_info[p_ptr->prace];
	cp_ptr = &c_info[p_ptr->pclass];

	/* Important -- Initialize the magic */
	mp_ptr = &cp_ptr->spells;


	/* Read the inventory */
	if (rd_inventory())
	{
		note("Unable to read inventory");
		return (-1);
	}


	/* Read the stores */
	rd_u16b(&tmp16u);
	for (i = 0; i < tmp16u; i++)
	{
		if (rd_store(i)) return (-1);
	}


	/* I'm not dead yet... */
	if (!p_ptr->is_dead)
	{
		/* Dead players have no dungeon */
		note("Restoring Dungeon...");
		if (rd_dungeon())
		{
			note("Error reading dungeon data");
			return (-1);
		}

		/* Read the ghost info */
		rd_ghost();
	}


	/* Save the checksum */
	n_v_check = v_check;

	/* Read the old checksum */
	rd_u32b(&o_v_check);

	/* Verify */
	if (o_v_check != n_v_check)
	{
		note("Invalid checksum");
		return (-1);
	}

	/* Save the encoded checksum */
	n_x_check = x_check;

	/* Read the checksum */
	rd_u32b(&o_x_check);

	/* Verify */
	if (o_x_check != n_x_check)
	{
		note("Invalid encoded checksum");
		return (-1);
	}


	/* Hack -- no ghosts */
	r_info[z_info->r_max-1].max_num = 0;


	/* Success */
	return (0);
}


/*
 * Actually read the savefile
 */
static errr rd_savefile(void)
{
	errr err;

	/* Grab permissions */
	safe_setuid_grab();

	/* The savefile is a binary file */
	fff = my_fopen(savefile, "rb");

	/* Drop permissions */
	safe_setuid_drop();

	/* Paranoia */
	if (!fff) return (-1);

	/* Call the sub-function */
	err = rd_savefile_new_aux();

	/* Check for errors */
	if (ferror(fff)) err = -1;

	/* Close the file */
	my_fclose(fff);

	/* Result */
	return (err);
}


/*
 * Attempt to Load a "savefile"
 *
 * On multi-user systems, you may only "read" a savefile if you will be
 * allowed to "write" it later, this prevents painful situations in which
 * the player loads a savefile belonging to someone else, and then is not
 * allowed to save his game when he quits.
 *
 * We return "TRUE" if the savefile was usable, and we set the global
 * flag "character_loaded" if a real, living, character was loaded.
 *
 * Note that we always try to load the "current" savefile, even if
 * there is no such file, so we must check for "empty" savefile names.
 */
bool load_player(void)
{
	int fd = -1;

	errr err = 0;

	byte vvv[4];

	cptr what = "generic";


	/* Paranoia */
	turn = 0;

	/* Paranoia */
	p_ptr->is_dead = FALSE;


	/* Allow empty savefile name */
	if (!savefile[0]) return (TRUE);

	/* Grab permissions */
	safe_setuid_grab();

	/* Open the savefile */
	fd = fd_open(savefile, O_RDONLY);

	/* Drop permissions */
	safe_setuid_drop();

	/* No file */
	if (fd < 0)
	{
		/* Give a message */
		msg_print("Savefile does not exist.");
		message_flush();

		/* Allow this */
		return (TRUE);
	}

	/* Close the file */
	fd_close(fd);


	/* Okay */
	if (!err)
	{
		/* Grab permissions */
		safe_setuid_grab();

		/* Open the savefile */
		fd = fd_open(savefile, O_RDONLY);

		/* Drop permissions */
		safe_setuid_drop();

		/* No file */
		if (fd < 0) err = -1;

		/* Message (below) */
		if (err) what = "Cannot open savefile";
	}

	/* Process file */
	if (!err)
	{
		/* Read the first four bytes */
		if (fd_read(fd, (char*)(vvv), sizeof(vvv))) err = -1;

		/* What */
		if (err) what = "Cannot read savefile";

		/* Close the file */
		fd_close(fd);
	}

	/* Process file */
	if (!err)
	{
		/* Extract version */
		sf_major = vvv[0];
		sf_minor = vvv[1];
		sf_patch = vvv[2];
		sf_extra = vvv[3];

		/* Clear screen */
		Term_clear();

		if (older_than(OLD_VERSION_MAJOR, OLD_VERSION_MINOR, OLD_VERSION_PATCH))
		{
			err = -1;
			what = "Savefile is too old";
		}
		else if (!older_than(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH + 1))
		{
			err = -1;
			what = "Savefile is from the future";
		}
		else
		{
			/* Attempt to load */
			err = rd_savefile();

			/* Message (below) */
			if (err) what = "Cannot parse savefile";
		}
	}

	/* Paranoia */
	if (!err)
	{
		/* Invalid turn */
		if (!turn) err = -1;

		/* Message (below) */
		if (err) what = "Broken savefile";
	}


	/* Okay */
	if (!err)
	{
		/* Give a conversion warning */
		if ((version_major != sf_major) ||
		    (version_minor != sf_minor) ||
		    (version_patch != sf_patch))
		{
			/* Message */
			msg_format("Converted a %d.%d.%d savefile.",
			           sf_major, sf_minor, sf_patch);
			message_flush();
		}

		/* Player is dead */
		if (p_ptr->is_dead)
		{
			/* Cheat death (unless the character retired) */
			if (arg_wizard)
			{
				/* A character was loaded */
				character_loaded = TRUE;

				/* Done */
				return (TRUE);
			}

			/* Forget death */
			p_ptr->is_dead = FALSE;

			/* Count lives */
			sf_lives++;

			/* Forget turns */
			turn = old_turn = 0;

			/* Done */
			return (TRUE);
		}

		/* A character was loaded */
		character_loaded = TRUE;

		/* Still alive */
		if (p_ptr->chp >= 0)
		{
			/* Reset cause of death */
			strcpy(p_ptr->died_from, "(alive and well)");
		}

		/* Success */
		return (TRUE);
	}


	/* Message */
	msg_format("Error (%s) reading %d.%d.%d savefile.",
	           what, sf_major, sf_minor, sf_patch);
	message_flush();

	/* Oops */
	return (FALSE);
}
