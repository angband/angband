/* File: load2.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * This file loads savefiles from Angband 2.7.X and 2.8.X
 *
 * Ancient savefiles (pre-2.7.0) are loaded by another file.
 *
 * Note that Angband 2.7.0 through 2.7.3 are now officially obsolete,
 * and savefiles from those versions may not be successfully converted.
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
 * This function determines if the version of the savefile
 * currently being read is older than version "x.y.z".
 */
static bool older_than(byte x, byte y, byte z)
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
 * of savefiles.  They also maintain the "checksum" info for 2.7.0+
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
 * Owner Conversion -- pre-2.7.8 to 2.7.8
 * Shop is column, Owner is Row, see "tables.c"
 */
static const byte convert_owner[24] =
{
	1, 3, 1, 0, 2, 3, 2, 0,
	0, 1, 3, 1, 0, 1, 1, 0,
	3, 2, 0, 2, 1, 2, 3, 0
};


/*
 * Old pre-2.7.4 inventory slot values
 */
#define OLD_INVEN_WIELD     22
#define OLD_INVEN_HEAD      23
#define OLD_INVEN_NECK      24
#define OLD_INVEN_BODY      25
#define OLD_INVEN_ARM       26
#define OLD_INVEN_HANDS     27
#define OLD_INVEN_RIGHT     28
#define OLD_INVEN_LEFT      29
#define OLD_INVEN_FEET      30
#define OLD_INVEN_OUTER     31
#define OLD_INVEN_LITE      32
#define OLD_INVEN_AUX       33

/*
 * Analyze pre-2.7.4 inventory slots
 */
static s16b convert_slot(int old)
{
	/* Move slots */
	switch (old)
	{
		case OLD_INVEN_WIELD: return (INVEN_WIELD);
		case OLD_INVEN_HEAD: return (INVEN_HEAD);
		case OLD_INVEN_NECK: return (INVEN_NECK);
		case OLD_INVEN_BODY: return (INVEN_BODY);
		case OLD_INVEN_ARM: return (INVEN_ARM);
		case OLD_INVEN_HANDS: return (INVEN_HANDS);
		case OLD_INVEN_RIGHT: return (INVEN_RIGHT);
		case OLD_INVEN_LEFT: return (INVEN_LEFT);
		case OLD_INVEN_FEET: return (INVEN_FEET);
		case OLD_INVEN_OUTER: return (INVEN_OUTER);
		case OLD_INVEN_LITE: return (INVEN_LITE);

		/* Hack -- "hold" old aux items */
		case OLD_INVEN_AUX: return (INVEN_WIELD - 1);
	}

	/* Default */
	return (old);
}





/*
 * Hack -- convert pre-2.7.8 ego-item indexes
 */
static const byte convert_ego_item[128] =
{
	0,					/* 0 */
	EGO_RESISTANCE,		/* 1 = EGO_RESIST (XXX) */
	EGO_RESIST_ACID,	/* 2 = EGO_RESIST_A (XXX) */
	EGO_RESIST_FIRE,	/* 3 = EGO_RESIST_F (XXX) */
	EGO_RESIST_COLD,	/* 4 = EGO_RESIST_C (XXX) */
	EGO_RESIST_ELEC,	/* 5 = EGO_RESIST_E (XXX) */
	EGO_HA,				/* 6 = EGO_HA */
	EGO_DF,				/* 7 = EGO_DF */
	EGO_SLAY_ANIMAL,	/* 8 = EGO_SLAY_ANIMAL */
	EGO_SLAY_DRAGON,	/* 9 = EGO_SLAY_DRAGON */
	EGO_SLAY_EVIL,		/* 10 = EGO_SLAY_EVIL (XXX) */
	EGO_SLAY_UNDEAD,	/* 11 = EGO_SLAY_UNDEAD (XXX) */
	EGO_BRAND_FIRE,		/* 12 = EGO_FT */
	EGO_BRAND_COLD,		/* 13 = EGO_FB */
	EGO_FREE_ACTION,	/* 14 = EGO_FREE_ACTION (XXX) */
	EGO_SLAYING,		/* 15 = EGO_SLAYING */
	0,					/* 16 */
	0,					/* 17 */
	EGO_SLOW_DESCENT,	/* 18 = EGO_SLOW_DESCENT */
	EGO_SPEED,			/* 19 = EGO_SPEED */
	EGO_STEALTH,		/* 20 = EGO_STEALTH (XXX) */
	0,					/* 21 */
	0,					/* 22 */
	0,					/* 23 */
	EGO_INTELLIGENCE,	/* 24 = EGO_INTELLIGENCE */
	EGO_WISDOM,			/* 25 = EGO_WISDOM */
	EGO_INFRAVISION,	/* 26 = EGO_INFRAVISION */
	EGO_MIGHT,			/* 27 = EGO_MIGHT */
	EGO_LORDLINESS,		/* 28 = EGO_LORDLINESS */
	EGO_MAGI,			/* 29 = EGO_MAGI (XXX) */
	EGO_BEAUTY,			/* 30 = EGO_BEAUTY */
	EGO_SEEING,			/* 31 = EGO_SEEING (XXX) */
	EGO_REGENERATION,	/* 32 = EGO_REGENERATION */
	0,					/* 33 */
	0,					/* 34 */
	0,					/* 35 */
	0,					/* 36 */
	0,					/* 37 */
	EGO_PERMANENCE,		/* 38 = EGO_ROBE_MAGI */
	EGO_PROTECTION,		/* 39 = EGO_PROTECTION */
	0,					/* 40 */
	0,					/* 41 */
	0,					/* 42 */
	EGO_BRAND_FIRE,		/* 43 = EGO_FIRE (XXX) */
	EGO_HURT_EVIL,		/* 44 = EGO_AMMO_EVIL */
	EGO_HURT_DRAGON,	/* 45 = EGO_AMMO_DRAGON */
	0,					/* 46 */
	0,					/* 47 */
	0,					/* 48 */
	0,					/* 49 */
	EGO_FLAME,			/* 50 = EGO_AMMO_FIRE */
	0,					/* 51 */	/* oops */
	EGO_FROST,			/* 52 = EGO_AMMO_SLAYING */
	0,					/* 53 */
	0,					/* 54 */
	EGO_HURT_ANIMAL,	/* 55 = EGO_AMMO_ANIMAL */
	0,					/* 56 */
	0,					/* 57 */
	0,					/* 58 */
	0,					/* 59 */
	EGO_EXTRA_MIGHT,	/* 60 = EGO_EXTRA_MIGHT */
	EGO_EXTRA_SHOTS,	/* 61 = EGO_EXTRA_SHOTS */
	0,					/* 62 */
	0,					/* 63 */
	EGO_VELOCITY,		/* 64 = EGO_VELOCITY */
	EGO_ACCURACY,		/* 65 = EGO_ACCURACY */
	0,					/* 66 */
	EGO_SLAY_ORC,		/* 67 = EGO_SLAY_ORC */
	EGO_POWER,			/* 68 = EGO_POWER */
	0,					/* 69 */
	0,					/* 70 */
	EGO_WEST,			/* 71 = EGO_WEST */
	EGO_BLESS_BLADE,	/* 72 = EGO_BLESS_BLADE */
	EGO_SLAY_DEMON,		/* 73 = EGO_SLAY_DEMON */
	EGO_SLAY_TROLL,		/* 74 = EGO_SLAY_TROLL */
	0,					/* 75 */
	0,					/* 76 */
	EGO_WOUNDING,		/* 77 = EGO_AMMO_WOUNDING */
	0,					/* 78 */
	0,					/* 79 */
	0,					/* 80 */
	EGO_LITE,			/* 81 = EGO_LITE */
	EGO_AGILITY,		/* 82 = EGO_AGILITY */
	0,					/* 83 */
	0,					/* 84 */
	EGO_SLAY_GIANT,		/* 85 = EGO_SLAY_GIANT */
	EGO_TELEPATHY,		/* 86 = EGO_TELEPATHY */
	EGO_ELVENKIND,		/* 87 = EGO_ELVENKIND (XXX) */
	0,					/* 88 */
	0,					/* 89 */
	EGO_ATTACKS,		/* 90 = EGO_ATTACKS */
	EGO_AMAN,			/* 91 = EGO_AMAN */
	0,					/* 92 */
	0,					/* 93 */
	0,					/* 94 */
	0,					/* 95 */
	0,					/* 96 */
	0,					/* 97 */
	0,					/* 98 */
	0,					/* 99 */
	0,					/* 100 */
	0,					/* 101 */
	0,					/* 102 */
	0,					/* 103 */
	EGO_WEAKNESS,		/* 104 = EGO_WEAKNESS */
	EGO_STUPIDITY,		/* 105 = EGO_STUPIDITY */
	EGO_NAIVETY,		/* 106 = EGO_DULLNESS */
	EGO_SICKLINESS,		/* 107 = EGO_SICKLINESS */
	EGO_CLUMSINESS,		/* 108 = EGO_CLUMSINESS */
	EGO_UGLINESS,		/* 109 = EGO_UGLINESS */
	EGO_TELEPORTATION,	/* 110 = EGO_TELEPORTATION */
	0,					/* 111 */
	EGO_IRRITATION,		/* 112 = EGO_IRRITATION */
	EGO_VULNERABILITY,	/* 113 = EGO_VULNERABILITY */
	EGO_ENVELOPING,		/* 114 = EGO_ENVELOPING */
	0,					/* 115 */
	EGO_SLOWNESS,		/* 116 = EGO_SLOWNESS */
	EGO_NOISE,			/* 117 = EGO_NOISE */
	EGO_ANNOYANCE,		/* 118 = EGO_GREAT_MASS */
	0,					/* 119 */
	EGO_BACKBITING,		/* 120 = EGO_BACKBITING */
	0,					/* 121 */
	0,					/* 122 */
	0,					/* 123 */
	EGO_MORGUL,			/* 124 = EGO_MORGUL */
	0,					/* 125 */
	EGO_SHATTERED,		/* 126 = EGO_SHATTERED */
	EGO_BLASTED			/* 127 = EGO_BLASTED (XXX) */
};



/*
 * Read an object
 *
 * This function attempts to "repair" old savefiles, and to extract
 * the most up to date values for various object fields.
 *
 * Note that Angband 2.7.9 introduced a new method for object "flags"
 * in which the "flags" on an object are actually extracted when they
 * are needed from the object kind, artifact index, ego-item index,
 * and two special "xtra" fields which are used to encode any "extra"
 * power of certain ego-items.  This had the side effect that items
 * imported from pre-2.7.9 savefiles will lose any "extra" powers they
 * may have had, and also, all "uncursed" items will become "cursed"
 * again, including Calris, even if it is being worn at the time.  As
 * a complete hack, items which are inscribed with "uncursed" will be
 * "uncursed" when imported from pre-2.7.9 savefiles.
 */
static errr rd_item(object_type *o_ptr)
{
	byte old_dd;
	byte old_ds;

	s32b old_cost;

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

	/* Old method */
	if (older_than(2, 7, 8))
	{
		rd_byte(&o_ptr->name1);
		rd_byte(&o_ptr->name2);
		rd_byte(&o_ptr->ident);
		rd_byte(&o_ptr->number);
		rd_s16b(&o_ptr->weight);
		rd_s16b(&o_ptr->timeout);

		rd_s16b(&o_ptr->to_h);
		rd_s16b(&o_ptr->to_d);
		rd_s16b(&o_ptr->to_a);

		rd_s16b(&o_ptr->ac);

		rd_byte(&old_dd);
		rd_byte(&old_ds);

		strip_bytes(2);

		rd_s32b(&old_cost);

		strip_bytes(4);
	}

	/* New method */
	else
	{
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
	}

	/* Old flags */
	strip_bytes(12);

	/* Old version */
	if (older_than(2,8,0))
	{
		/* Old something */
		strip_bytes(2);
	}

	/* New version */
	else
	{
		/* Monster holding object */
		rd_s16b(&o_ptr->held_m_idx);
	}

	if (older_than(2,8,2))
	{
		/* Old special powers */
		strip_bytes(2);
	}

	else
	{
		/* Special powers */
		rd_byte(&o_ptr->xtra1);
		rd_byte(&o_ptr->xtra2);
	}


	/* Inscription */
	rd_string(buf, 128);

	/* Save the inscription */
	if (buf[0]) o_ptr->note = quark_add(buf);


	/* Mega-Hack -- handle "dungeon objects" later */
	if ((o_ptr->k_idx >= 445) && (o_ptr->k_idx <= 479)) return (0);

	/* Obtain the "kind" template */
	k_ptr = &k_info[o_ptr->k_idx];

	/* Obtain tval/sval from k_info */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;


	/* Hack -- notice "broken" items */
	if (k_ptr->cost <= 0) o_ptr->ident |= (IDENT_BROKEN);


	/* Hack -- the "gold" values changed in 2.7.8 */
	if (older_than(2, 7, 8) && (o_ptr->tval == TV_GOLD))
	{
		/* Extract the value */
		o_ptr->pval = (s16b)old_cost;

		/* Done */
		return (0);
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

	/* The ego item indexes changed in 2.7.9 */
	if (older_than(2, 7, 9) && o_ptr->name2)
	{
		/* Paranoia */
		if (o_ptr->name2 >= 128) return (-1);

		/* Convert the ego-item names */
		o_ptr->name2 = convert_ego_item[o_ptr->name2];

		/* Hack -- fix some "Ammo" */
		if ((o_ptr->tval == TV_BOLT) ||
		    (o_ptr->tval == TV_ARROW) ||
		    (o_ptr->tval == TV_SHOT))
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_BRAND_FIRE)
			{
				o_ptr->name2 = EGO_FLAME;
			}
			else if (o_ptr->name2 == EGO_SLAYING)
			{
				o_ptr->name2 = EGO_FROST;
			}
			else if (o_ptr->name2 == EGO_SLAY_ANIMAL)
			{
				o_ptr->name2 = EGO_HURT_ANIMAL;
			}
			else if (o_ptr->name2 == EGO_SLAY_EVIL)
			{
				o_ptr->name2 = EGO_HURT_EVIL;
			}
			else if (o_ptr->name2 == EGO_SLAY_DRAGON)
			{
				o_ptr->name2 = EGO_HURT_DRAGON;
			}
		}

		/* Hack -- fix some "Bows" */
		if (o_ptr->tval == TV_BOW)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_MIGHT)
			{
				o_ptr->name2 = EGO_VELOCITY;
			}
		}

		/* Hack -- fix some "Robes" */
		if (o_ptr->tval == TV_SOFT_ARMOR)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_MAGI)
			{
				o_ptr->name2 = EGO_PERMANENCE;
			}
		}

		/* Hack -- fix some "Boots" */
		if (o_ptr->tval == TV_BOOTS)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_STEALTH)
			{
				o_ptr->name2 = EGO_QUIET;
			}
			else if (o_ptr->name2 == EGO_FREE_ACTION)
			{
				o_ptr->name2 = EGO_MOTION;
			}
		}

		/* Hack -- fix some "Shields" */
		if (o_ptr->tval == TV_SHIELD)
		{
			/* Special ego-item indexes */
			if (o_ptr->name2 == EGO_RESIST_ACID)
			{
				o_ptr->name2 = EGO_ENDURE_ACID;
			}
			else if (o_ptr->name2 == EGO_RESIST_ELEC)
			{
				o_ptr->name2 = EGO_ENDURE_ELEC;
			}
			else if (o_ptr->name2 == EGO_RESIST_FIRE)
			{
				o_ptr->name2 = EGO_ENDURE_FIRE;
			}
			else if (o_ptr->name2 == EGO_RESIST_COLD)
			{
				o_ptr->name2 = EGO_ENDURE_COLD;
			}
			else if (o_ptr->name2 == EGO_RESISTANCE)
			{
				o_ptr->name2 = EGO_ENDURANCE;
			}
			else if (o_ptr->name2 == EGO_ELVENKIND)
			{
				o_ptr->name2 = EGO_ENDURANCE;
			}
		}
	}

	/* Hack -- the "searching" bonuses changed in 2.7.6 */
	if (older_than(2, 7, 6))
	{
		/* Reduce the "pval" bonus on "search" */
		if (f1 & (TR1_SEARCH))
		{
			/* Paranoia -- do not lose any search bonuses */
			o_ptr->pval = (o_ptr->pval + 4) / 5;
		}
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

	/* Hack -- extract the "broken" flag */
	if (!o_ptr->pval < 0) o_ptr->ident |= (IDENT_BROKEN);


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

		/* Hack -- assume "curse" */
		if (older_than(2, 7, 9))
		{
			/* Hack -- assume cursed */
			if (a_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);
		}
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

		/* Hack -- assume "curse" */
		if (older_than(2, 7, 9))
		{
			/* Hack -- assume cursed */
			if (e_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);
		}

		/* Hack -- enforce legal pval */
		if (e_ptr->flags1 & (TR1_PVAL_MASK))
		{
			/* Force a meaningful pval */
			if (!o_ptr->pval) o_ptr->pval = 1;
		}
	}


	/* Hack -- assume "cursed" items */
	if (older_than(2, 7, 9))
	{
		/* Hack -- assume cursed */
		if (k_ptr->flags3 & (TR3_LIGHT_CURSE)) o_ptr->ident |= (IDENT_CURSED);

		/* Hack -- apply "uncursed" incription */
		if (streq(buf, "uncursed")) o_ptr->ident &= ~(IDENT_CURSED);
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
	byte tmp8u;

	monster_race *r_ptr = &r_info[r_idx];
	monster_lore *l_ptr = &l_list[r_idx];


	/* Pre-2.7.7 */
	if (older_than(2, 7, 7))
	{
		/* Strip old flags */
		strip_bytes(20);

		/* Kills during this life */
		rd_s16b(&l_ptr->r_pkills);

		/* Strip something */
		strip_bytes(2);

		/* Count observations of attacks */
		rd_byte(&l_ptr->r_blows[0]);
		rd_byte(&l_ptr->r_blows[1]);
		rd_byte(&l_ptr->r_blows[2]);
		rd_byte(&l_ptr->r_blows[3]);

		/* Count some other stuff */
		rd_byte(&l_ptr->r_wake);
		rd_byte(&l_ptr->r_ignore);

		/* Strip something */
		strip_bytes(2);

		/* Count kills by player */
		rd_s16b(&l_ptr->r_tkills);

		/* Count deaths of player */
		rd_s16b(&l_ptr->r_deaths);

		/* Read the "Racial" monster limit per level */
		rd_byte(&r_ptr->max_num);

		/* Strip something */
		strip_bytes(1);

		/* Hack -- guess at "sights" */
		l_ptr->r_sights = MAX(l_ptr->r_tkills, l_ptr->r_deaths);
	}

	/* Current */
	else
	{
		/* Count sights/deaths/kills */
		rd_s16b(&l_ptr->r_sights);
		rd_s16b(&l_ptr->r_deaths);
		rd_s16b(&l_ptr->r_pkills);
		rd_s16b(&l_ptr->r_tkills);

		/* Count wakes and ignores */
		rd_byte(&l_ptr->r_wake);
		rd_byte(&l_ptr->r_ignore);

		/* Extra stuff */
		rd_byte(&l_ptr->r_xtra1);
		rd_byte(&l_ptr->r_xtra2);

		/* Count drops */
		rd_byte(&l_ptr->r_drop_gold);
		rd_byte(&l_ptr->r_drop_item);

		/* Count spells */
		rd_byte(&l_ptr->r_cast_inate);
		rd_byte(&l_ptr->r_cast_spell);

		/* Count blows of each type */
		rd_byte(&l_ptr->r_blows[0]);
		rd_byte(&l_ptr->r_blows[1]);
		rd_byte(&l_ptr->r_blows[2]);
		rd_byte(&l_ptr->r_blows[3]);

		/* Memorize flags */
		rd_u32b(&l_ptr->r_flags1);
		rd_u32b(&l_ptr->r_flags2);
		rd_u32b(&l_ptr->r_flags3);
		rd_u32b(&l_ptr->r_flags4);
		rd_u32b(&l_ptr->r_flags5);
		rd_u32b(&l_ptr->r_flags6);


		/* Read the "Racial" monster limit per level */
		rd_byte(&r_ptr->max_num);

		/* Later (?) */
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
		rd_byte(&tmp8u);
	}

	/* Repair the lore flags */
	l_ptr->r_flags1 &= r_ptr->flags1;
	l_ptr->r_flags2 &= r_ptr->flags2;
	l_ptr->r_flags3 &= r_ptr->flags3;
	l_ptr->r_flags4 &= r_ptr->flags4;
	l_ptr->r_flags5 &= r_ptr->flags5;
	l_ptr->r_flags6 &= r_ptr->flags6;
}




/*
 * Read a store
 */
static errr rd_store(int n)
{
	store_type *st_ptr = &store[n];

	int j;

	byte own, num;


	/* Read the basic info */
	rd_s32b(&st_ptr->store_open);
	rd_s16b(&st_ptr->insult_cur);
	rd_byte(&own);
	rd_byte(&num);
	rd_s16b(&st_ptr->good_buy);
	rd_s16b(&st_ptr->bad_buy);

	/* Extract the owner (see above) */
	if (older_than(2, 7, 8))
	{
		/* Paranoia */
		if (own >= 24)
		{
			note("Illegal store owner!");
			return (-1);
		}

		st_ptr->owner = convert_owner[own];
	}
	else
	{
		/* Paranoia */
		if (own >= z_info->b_max)
		{
			note("Illegal store owner!");
			return (-1);
		}

		st_ptr->owner = own;
	}

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

		/* Not marked XXX XXX */
		if (older_than(2, 8, 2))
		{
			i_ptr->marked = FALSE;
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
 * Read RNG state (added in 2.8.0)
 */
static void rd_randomizer(void)
{
	int i;

	u16b tmp16u;

	/* Old version */
	if (older_than(2, 8, 0)) return;

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
 * Read options (ignore most pre-2.8.0 options)
 *
 * Note that the normal options are now stored as a set of 256 bit flags,
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

	/* Pre-2.8.0 savefiles are done */
	if (older_than(2, 8, 0)) return;


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

		/* Process real entries */
		if (option_text[i])
		{
			/* Process saved entries */
			if (mask[os] & (1L << ob))
			{
				/* Set flag */
				if (flag[os] & (1L << ob))
				{
					/* Set */
					op_ptr->opt[i] = TRUE;
				}

				/* Clear flag */
				else
				{
					/* Set */
					op_ptr->opt[i] = FALSE;
				}
			}
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

	/* Older ghosts */
	if (older_than(2, 7, 7))
	{
		/* Strip old data */
		strip_bytes(52);
	}

	/* Newer ghosts */
	else
	{
		/* Strip old data */
		strip_bytes(60);
	}
}




/*
 * Read the "extra" information
 */
static errr rd_extra(void)
{
	int i;

	byte tmp8u;
	u16b tmp16u;

	u32b randart_version;

	rd_string(op_ptr->full_name, 32);

	rd_string(p_ptr->died_from, 80);

	for (i = 0; i < 4; i++)
	{
		rd_string(p_ptr->history[i], 60);
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
	if (p_ptr->pclass >= MAX_CLASS)
	{
		note(format("Invalid player class (%d).", p_ptr->pclass));
		return (-1);
	}

	/* Player gender */
	rd_byte(&p_ptr->psex);

	/* Repair psex (2.8.1 beta) */
	if (p_ptr->psex > MAX_SEXES - 1) p_ptr->psex = MAX_SEXES - 1;

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

	/* Ignore old redundant info */
	if (older_than(2, 7, 7)) strip_bytes(24);

	/* Read the flags */
	strip_bytes(2);	/* Old "rest" */
	rd_s16b(&p_ptr->blind);
	rd_s16b(&p_ptr->paralyzed);
	rd_s16b(&p_ptr->confused);
	rd_s16b(&p_ptr->food);
	strip_bytes(4);	/* Old "food_digested" / "protection" */
	rd_s16b(&p_ptr->energy);
	rd_s16b(&p_ptr->fast);
	rd_s16b(&p_ptr->slow);
	rd_s16b(&p_ptr->afraid);
	rd_s16b(&p_ptr->cut);
	rd_s16b(&p_ptr->stun);
	rd_s16b(&p_ptr->poisoned);
	rd_s16b(&p_ptr->image);
	rd_s16b(&p_ptr->protevil);
	rd_s16b(&p_ptr->invuln);
	rd_s16b(&p_ptr->hero);
	rd_s16b(&p_ptr->shero);
	rd_s16b(&p_ptr->shield);
	rd_s16b(&p_ptr->blessed);
	rd_s16b(&p_ptr->tim_invis);
	rd_s16b(&p_ptr->word_recall);
	rd_s16b(&p_ptr->see_infra);
	rd_s16b(&p_ptr->tim_infra);
	rd_s16b(&p_ptr->oppose_fire);
	rd_s16b(&p_ptr->oppose_cold);
	rd_s16b(&p_ptr->oppose_acid);
	rd_s16b(&p_ptr->oppose_elec);
	rd_s16b(&p_ptr->oppose_pois);

	/* Old redundant flags */
	if (older_than(2, 7, 7)) strip_bytes(34);

	rd_byte(&p_ptr->confusing);
	rd_byte(&tmp8u);	/* oops */
	rd_byte(&tmp8u);	/* oops */
	rd_byte(&tmp8u);	/* oops */
	rd_byte(&p_ptr->searching);
	rd_byte(&tmp8u);	/* oops */
	if (older_than(2, 8, 5)) adult_maximize = tmp8u;
	rd_byte(&tmp8u);	/* oops */
	if (older_than(2, 8, 5)) adult_preserve = tmp8u;
	rd_byte(&tmp8u);
	if (older_than(2, 8, 5)) adult_rand_artifacts = tmp8u;

	/* Future use */
	strip_bytes(40);

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

	/* Initialize random artifacts */
	if (adult_rand_artifacts && !(p_ptr->is_dead))
	{
#ifdef GJW_RANDART

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
		do_randart(seed_randart);

#else /* GJW_RANDART */

		note("Random artifacts are disabled in this binary.");
		return (-1);

#endif /* GJW_RANDART */

	}

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


	/* Read spell info */
	rd_u32b(&p_ptr->spell_learned1);
	rd_u32b(&p_ptr->spell_learned2);
	rd_u32b(&p_ptr->spell_worked1);
	rd_u32b(&p_ptr->spell_worked2);
	rd_u32b(&p_ptr->spell_forgotten1);
	rd_u32b(&p_ptr->spell_forgotten2);

	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		rd_byte(&p_ptr->spell_order[i]);
	}

	return (0);
}




/*
 * Read the player inventory
 *
 * Note that the inventory changed in Angband 2.7.4.  Two extra
 * pack slots were added and the equipment was rearranged.  Note
 * that these two features combine when parsing old save-files, in
 * which items from the old "aux" slot are "carried", perhaps into
 * one of the two new "inventory" slots.
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

		/* Not marked XXX XXX */
		if (older_than(2, 8, 2))
		{
			i_ptr->marked = FALSE;
		}

		/* Hack -- verify item */
		if (!i_ptr->k_idx) return (-1);

		/* Hack -- convert old slot numbers */
		if (older_than(2, 7, 4)) n = convert_slot(n);

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
		rd_string(buf, 128);

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
 * Old "cave grid" flags -- saved in savefile
 */
#define OLD_GRID_W_01	0x0001	/* Wall type (bit 1) */
#define OLD_GRID_W_02	0x0002	/* Wall type (bit 2) */
#define OLD_GRID_PERM	0x0004	/* Wall type is permanent */
#define OLD_GRID_QQQQ	0x0008	/* Unused */
#define OLD_GRID_MARK	0x0010	/* Grid is memorized */
#define OLD_GRID_GLOW	0x0020	/* Grid is illuminated */
#define OLD_GRID_ROOM	0x0040	/* Grid is part of a room */
#define OLD_GRID_ICKY	0x0080	/* Grid is anti-teleport */

/*
 * Masks for the new grid types
 */
#define OLD_GRID_WALL_MASK	0x0003	/* Wall type */

/*
 * Legal results of OLD_GRID_WALL_MASK
 */
#define OLD_GRID_WALL_NONE		0x0000	/* No wall */
#define OLD_GRID_WALL_MAGMA		0x0001	/* Magma vein */
#define OLD_GRID_WALL_QUARTZ	0x0002	/* Quartz vein */
#define OLD_GRID_WALL_GRANITE	0x0003	/* Granite wall */


/*
 * Read pre-2.8.0 dungeon info
 *
 * Try to be more flexible about "too many monsters" XXX XXX
 *
 * Convert the old "flags" and "fake objects" into the new terrain features.
 */
static errr rd_dungeon_aux(s16b depth, s16b py, s16b px)
{
	int i, y, x;
	byte count;
	byte tmp8u;
	u16b start;
	u16b limit;


	/* Read the dungeon */
	for (y = x = 0; y < DUNGEON_HGT; )
	{
		/* Extract some RLE info */
		rd_byte(&count);
		rd_byte(&tmp8u);

		/* Apply the RLE info */
		for (i = count; i > 0; i--)
		{
			byte info = 0x00;
			byte feat = FEAT_FLOOR;

			/* Old method */
			if (older_than(2, 7, 5))
			{
				/* Extract the old "info" flags */
				if ((tmp8u >> 4) & 0x1) info |= (CAVE_ROOM);
				if ((tmp8u >> 5) & 0x1) info |= (CAVE_MARK);
				if ((tmp8u >> 6) & 0x1) info |= (CAVE_GLOW);

				/* Hack -- process old style "light" */
				if (info & (CAVE_GLOW))
				{
					info |= (CAVE_MARK);
				}

				/* Mega-Hack -- light all walls */
				else if ((tmp8u & 0x0F) >= 12)
				{
					info |= (CAVE_GLOW);
				}

				/* Process the "floor type" */
				switch (tmp8u & 0x0F)
				{
					/* Lite Room Floor */
					case 2:
					{
						info |= (CAVE_GLOW);
					}

					/* Dark Room Floor */
					case 1:
					{
						info |= (CAVE_ROOM);
						break;
					}

					/* Lite Vault Floor */
					case 4:
					{
						info |= (CAVE_GLOW);
					}

					/* Dark Vault Floor */
					case 3:
					{
						info |= (CAVE_ROOM);
						info |= (CAVE_ICKY);
						break;
					}

					/* Corridor Floor */
					case 5:
					{
						break;
					}

					/* Perma-wall */
					case 15:
					{
						feat = FEAT_PERM_SOLID;
						break;
					}

					/* Granite wall */
					case 12:
					{
						feat = FEAT_WALL_EXTRA;
						break;
					}

					/* Quartz vein */
					case 13:
					{
						feat = FEAT_QUARTZ;
						break;
					}

					/* Magma vein */
					case 14:
					{
						feat = FEAT_MAGMA;
						break;
					}
				}
			}

			/* Newer method */
			else
			{
				/* The old "vault" flag */
				if (tmp8u & (OLD_GRID_ICKY)) info |= (CAVE_ICKY);

				/* The old "room" flag */
				if (tmp8u & (OLD_GRID_ROOM)) info |= (CAVE_ROOM);

				/* The old "glow" flag */
				if (tmp8u & (OLD_GRID_GLOW)) info |= (CAVE_GLOW);

				/* The old "mark" flag */
				if (tmp8u & (OLD_GRID_MARK)) info |= (CAVE_MARK);

				/* The old "wall" flags -- granite wall */
				if ((tmp8u & (OLD_GRID_WALL_MASK)) ==
				    OLD_GRID_WALL_GRANITE)
				{
					/* Permanent wall */
					if (tmp8u & (OLD_GRID_PERM))
					{
						feat = FEAT_PERM_SOLID;
					}

					/* Normal wall */
					else
					{
						feat = FEAT_WALL_EXTRA;
					}
				}

				/* The old "wall" flags -- quartz vein */
				else if ((tmp8u & (OLD_GRID_WALL_MASK)) ==
				         OLD_GRID_WALL_QUARTZ)
				{
					/* Assume no treasure */
					feat = FEAT_QUARTZ;
				}

				/* The old "wall" flags -- magma vein */
				else if ((tmp8u & (OLD_GRID_WALL_MASK)) ==
				         OLD_GRID_WALL_MAGMA)
				{
					/* Assume no treasure */
					feat = FEAT_MAGMA;
				}
			}

			/* Save the info */
			cave_info[y][x] = info;

			/* Save the feat */
			cave_set_feat(y, x, feat);

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

	/* Save depth */
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

	/* Hack -- verify */
	if (limit >= 512)
	{
		note(format("Too many (%d) object entries!", limit));
		return (-1);
	}

	/* Read the dungeon items */
	for (i = 1; i < limit; i++)
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

		/* Location */
		y = i_ptr->iy;
		x = i_ptr->ix;


		/* Skip dead objects */
		if (!i_ptr->k_idx) continue;


		/* Hack -- convert old "dungeon" objects */
		if ((i_ptr->k_idx >= 445) && (i_ptr->k_idx <= 479))
		{
			byte feat = FEAT_FLOOR;

			bool invis = FALSE;

			/* Hack -- catch "invisible traps" */
			if (i_ptr->tval == 101) invis = TRUE;

			/* Analyze the "dungeon objects" */
			switch (i_ptr->k_idx)
			{
				/* Rubble */
				case 445:
				{
					feat = FEAT_RUBBLE;
					break;
				}

				/* Open Door */
				case 446:
				{
					/* Broken door */
					if (i_ptr->pval)
					{
						feat = FEAT_BROKEN;
					}

					/* Open door */
					else
					{
						feat = FEAT_OPEN;
					}

					break;
				}

				/* Closed Door */
				case 447:
				{
					/* Jammed door */
					if (i_ptr->pval < 0)
					{
						feat = (0 - i_ptr->pval) / 2;
						if (feat > 0x07) feat = 0x07;
						feat = FEAT_DOOR_HEAD + 0x08 + feat;
					}

					/* Locked door */
					else
					{
						feat = i_ptr->pval / 2;
						if (feat > 0x07) feat = 0x07;
						feat = FEAT_DOOR_HEAD + feat;
					}

					break;
				}

				/* Secret Door */
				case 448:
				{
					feat = FEAT_SECRET;
					break;
				}

				/* Up Stairs */
				case 449:
				{
					feat = FEAT_LESS;
					break;
				}

				/* Down Stairs */
				case 450:
				{
					feat = FEAT_MORE;
					break;
				}

				/* Store '1' */
				case 451:
				{
					feat = FEAT_SHOP_HEAD + 0x00;
					break;
				}

				/* Store '2' */
				case 452:
				{
					feat = FEAT_SHOP_HEAD + 0x01;
					break;
				}

				/* Store '3' */
				case 453:
				{
					feat = FEAT_SHOP_HEAD + 0x02;
					break;
				}

				/* Store '4' */
				case 454:
				{
					feat = FEAT_SHOP_HEAD + 0x03;
					break;
				}

				/* Store '5' */
				case 455:
				{
					feat = FEAT_SHOP_HEAD + 0x04;
					break;
				}

				/* Store '6' */
				case 456:
				{
					feat = FEAT_SHOP_HEAD + 0x05;
					break;
				}

				/* Store '7' */
				case 457:
				{
					feat = FEAT_SHOP_HEAD + 0x06;
					break;
				}

				/* Store '8' */
				case 458:
				{
					feat = FEAT_SHOP_HEAD + 0x07;
					break;
				}

				/* Glyph of Warding */
				case 459:
				{
					feat = FEAT_GLYPH;
					break;
				}

				/* Trap -- Pit */
				case 460:
				{
					feat = FEAT_TRAP_HEAD + 0x01;
					break;
				}

				/* Trap -- Spiked Pit */
				case 461:
				{
					feat = FEAT_TRAP_HEAD + 0x02;
					break;
				}

				/* Trap -- Trap Door */
				case 462:
				{
					feat = FEAT_TRAP_HEAD + 0x00;
					break;
				}

				/* Trap -- Gas -- Sleep */
				case 463:
				{
					feat = FEAT_TRAP_HEAD + 0x0F;
					break;
				}

				/* Trap -- Loose rock */
				case 464:
				{
					feat = FEAT_TRAP_HEAD + 0x01;
					break;
				}

				/* Trap -- Dart -- lose str */
				case 465:
				{
					feat = FEAT_TRAP_HEAD + 0x09;
					break;
				}

				/* Trap -- Teleport */
				case 466:
				{
					feat = FEAT_TRAP_HEAD + 0x05;
					break;
				}

				/* Trap -- Falling rock */
				case 467:
				{
					feat = FEAT_TRAP_HEAD + 0x03;
					break;
				}

				/* Trap -- Dart -- lose dex */
				case 468:
				{
					feat = FEAT_TRAP_HEAD + 0x0A;
					break;
				}

				/* Trap -- Summoning */
				case 469:
				{
					feat = FEAT_TRAP_HEAD + 0x04;
					break;
				}

				/* Trap -- Fire */
				case 470:
				{
					feat = FEAT_TRAP_HEAD + 0x06;
					break;
				}

				/* Trap -- Acid */
				case 471:
				{
					feat = FEAT_TRAP_HEAD + 0x07;
					break;
				}

				/* Trap -- Gas -- poison */
				case 472:
				{
					feat = FEAT_TRAP_HEAD + 0x0E;
					break;
				}

				/* Trap -- Gas -- blind */
				case 473:
				{
					feat = FEAT_TRAP_HEAD + 0x0C;
					break;
				}

				/* Trap -- Gas -- confuse */
				case 474:
				{
					feat = FEAT_TRAP_HEAD + 0x0D;
					break;
				}

				/* Trap -- Dart -- slow */
				case 475:
				{
					feat = FEAT_TRAP_HEAD + 0x08;
					break;
				}

				/* Trap -- Dart -- lose con */
				case 476:
				{
					feat = FEAT_TRAP_HEAD + 0x0B;
					break;
				}

				/* Trap -- Arrow */
				case 477:
				{
					feat = FEAT_TRAP_HEAD + 0x08;
					break;
				}
			}

			/* Hack -- handle "invisible traps" */
			if (invis) feat = FEAT_INVIS;

			/* Set new bits */
			cave_set_feat(y, x, feat);

			/* Skip it */
			continue;
		}


		/* Hack -- treasure in walls */
		if (i_ptr->tval == TV_GOLD)
		{
			/* Quartz + treasure */
			if ((cave_feat[y][x] == FEAT_QUARTZ) ||
			    (cave_feat[y][x] == FEAT_MAGMA))
			{
				/* Add known treasure */
				cave_set_feat(y, x, cave_feat[y][x] + 0x04);

				/* Done */
				continue;
			}
		}


		/* Give the item to the floor */
		if (!floor_carry(y, x, i_ptr))
		{
			note(format("Cannot place object %d!", o_max));
			return (-1);
		}
	}


	/*** Monsters ***/

	/* Extract index of first monster */
	start = (older_than(2, 7, 7) ? 2 : 1);

	/* Read the monster count */
	rd_u16b(&limit);

	/* Hack -- verify */
	if (limit >= 1024)
	{
		note(format("Too many (%d) monster entries!", limit));
		return (-1);
	}

	/* Read the monsters */
	for (i = start; i < limit; i++)
	{
		monster_type *n_ptr;
		monster_type monster_type_body;


		/* Get local monster */
		n_ptr = &monster_type_body;

		/* Clear the monster */
		(void)WIPE(n_ptr, monster_type);

		/* Read the monster */
		rd_monster(n_ptr);


		/* Hack -- ignore "broken" monsters */
		if (n_ptr->r_idx <= 0) continue;

		/* Hack -- ignore "player ghosts" */
		if (n_ptr->r_idx >= z_info->r_max-1) continue;


		/* Place monster in dungeon */
		if (!monster_place(n_ptr->fy, n_ptr->fx, n_ptr))
		{
			note(format("Cannot place monster %d!", i));
			return (-1);
		}
	}


	/* The dungeon is ready */
	character_dungeon = TRUE;


	/* Success */
	return (0);
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


	/* Old method */
	if (older_than(2,8,0))
	{
		return (rd_dungeon_aux(depth, py, px));
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

	/* Save depth */
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
	if (limit >= z_info->o_max)
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
	if (limit >= z_info->m_max)
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
		if (o_ptr->held_m_idx >= z_info->m_max)
		{
			note("Invalid monster index");
			return (-1);
		}

		/* Get the monster */
		m_ptr = &m_list[o_ptr->held_m_idx];

		/* Link the object to the pile */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Link the monster to the object */
		m_ptr->hold_o_idx = i;
	}


	/*** Success ***/

	/* The dungeon is ready */
	character_dungeon = TRUE;

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


#ifdef VERIFY_CHECKSUMS
	u32b n_x_check, n_v_check;
	u32b o_x_check, o_v_check;
#endif


	/* Mention the savefile version */
	note(format("Loading a %d.%d.%d savefile...",
	            sf_major, sf_minor, sf_patch));


	/* Hack -- Warn about "obsolete" versions */
	if (older_than(2, 7, 4))
	{
		note("Warning -- converting obsolete save file.");
	}


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
		monster_race *r_ptr;
		monster_lore *l_ptr;

		/* Read the lore */
		rd_lore(i);

		/* Get the monster race */
		r_ptr = &r_info[i];
		l_ptr = &l_list[i];

		/* XXX XXX Hack -- repair old savefiles */
		if (older_than(2, 7, 6))
		{
			/* Assume no kills */
			l_ptr->r_pkills = 0;

			/* Hack -- no previous lives */
			if (sf_lives == 0)
			{
				/* All kills by this life */
				l_ptr->r_pkills = l_ptr->r_tkills;
			}

			/* Hack -- handle uniques */
			if (r_ptr->flags1 & (RF1_UNIQUE))
			{
				/* Assume no kills */
				l_ptr->r_pkills = 0;

				/* Handle dead uniques */
				if (r_ptr->max_num == 0) l_ptr->r_pkills = 1;
			}
		}
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

		k_ptr->aware = (tmp8u & 0x01) ? TRUE: FALSE;
		k_ptr->tried = (tmp8u & 0x02) ? TRUE: FALSE;
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


	/* Important -- Initialize the sex */
	sp_ptr = &sex_info[p_ptr->psex];

	/* Important -- Initialize the race/class */
	rp_ptr = &p_info[p_ptr->prace];
	cp_ptr = &class_info[p_ptr->pclass];

	/* Important -- Initialize the magic */
	mp_ptr = &magic_info[p_ptr->pclass];


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


#ifdef VERIFY_CHECKSUMS

	/* Recent version */
	if (!older_than(2,8,2))
	{
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
	}

#endif


	/* Hack -- no ghosts */
	r_info[z_info->r_max-1].max_num = 0;


	/* Success */
	return (0);
}


/*
 * Actually read the savefile
 */
errr rd_savefile_new(void)
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

