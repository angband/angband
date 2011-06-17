/*
 * File: squelch.c
 * Purpose: Item destruction
 *
 * Copyright (c) 2007 David T. Blackston, Iain McFall, DarkGod, Jeff Greene,
 * David Vestal, Pete Mack, Andrew Sidwell.
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
#include "cmds.h"
#include "ui-menu.h"
#include "object/tvalsval.h"
#include "object/pval.h"
#include "squelch.h"


typedef struct
{
	squelch_type_t squelch_type;
	int tval;
	int min_sval;
	int max_sval;
} quality_squelch_struct;

static quality_squelch_struct quality_mapping[] =
{
	{ TYPE_WEAPON_POINTY,	TV_SWORD,		0,		SV_UNKNOWN },
	{ TYPE_WEAPON_POINTY,	TV_POLEARM,		0,		SV_UNKNOWN },
	{ TYPE_WEAPON_BLUNT,	TV_HAFTED,		0,		SV_UNKNOWN },
	{ TYPE_SHOOTER,			TV_BOW,			0,		SV_UNKNOWN },
	{ TYPE_MISSILE_SLING,	TV_SHOT,		0,		SV_UNKNOWN },
	{ TYPE_MISSILE_BOW,		TV_ARROW,		0,		SV_UNKNOWN },
	{ TYPE_MISSILE_XBOW,	TV_BOLT,		0,		SV_UNKNOWN },
	{ TYPE_ARMOR_ROBE,		TV_SOFT_ARMOR,	SV_ROBE,SV_ROBE },
	{ TYPE_ARMOR_BODY,		TV_DRAG_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_BODY,		TV_HARD_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_BODY,		TV_SOFT_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_CLOAK,		TV_CLOAK,		SV_CLOAK, SV_FUR_CLOAK },
	{ TYPE_ARMOR_CLOAK,		TV_CLOAK,		SV_ETHEREAL_CLOAK, SV_ETHEREAL_CLOAK },
/* XXX Eddie need to assert SV_CLOAK < SV_FUR_CLOAK < SV_ELVEN_CLOAK */
	{ TYPE_ARMOR_ELVEN_CLOAK, TV_CLOAK,		SV_ELVEN_CLOAK,	SV_ELVEN_CLOAK },
	{ TYPE_ARMOR_SHIELD,	TV_SHIELD,		0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HEAD,		TV_HELM,		0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HEAD,		TV_CROWN,		0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HANDS,		TV_GLOVES,		0,		SV_UNKNOWN },
	{ TYPE_ARMOR_FEET,		TV_BOOTS,		0,		SV_UNKNOWN },
	{ TYPE_DIGGER,			TV_DIGGING,		0,		SV_UNKNOWN },
	{ TYPE_RING,			TV_RING,		0,		SV_UNKNOWN },
	{ TYPE_AMULET,			TV_AMULET,		0,		SV_UNKNOWN },
	{ TYPE_LIGHT, 			TV_LIGHT, 		0,		SV_UNKNOWN },
};



quality_name_struct quality_choices[TYPE_MAX] =
{
	{ TYPE_WEAPON_POINTY,		"Pointy Melee Weapons" },
	{ TYPE_WEAPON_BLUNT,		"Blunt Melee Weapons" },
	{ TYPE_SHOOTER,				"Missile weapons" },
	{ TYPE_MISSILE_SLING,		"Shots and Pebbles" },
	{ TYPE_MISSILE_BOW,			"Arrows" },
	{ TYPE_MISSILE_XBOW,		"Bolts" },
	{ TYPE_ARMOR_ROBE,			"Robes" },
	{ TYPE_ARMOR_BODY,			"Body Armor" },
	{ TYPE_ARMOR_CLOAK,			"Cloaks" },
	{ TYPE_ARMOR_ELVEN_CLOAK,	"Elven Cloaks" },
	{ TYPE_ARMOR_SHIELD,		"Shields" },
	{ TYPE_ARMOR_HEAD,			"Headgear" },
	{ TYPE_ARMOR_HANDS,			"Handgear" },
	{ TYPE_ARMOR_FEET,			"Footgear" },
	{ TYPE_DIGGER,				"Diggers" },
	{ TYPE_RING,				"Rings" },
	{ TYPE_AMULET,				"Amulets" },
	{ TYPE_LIGHT, 				"Lights" },
};

/*
 * The names for the various kinds of quality
 */
quality_name_struct quality_values[SQUELCH_MAX] =
{
	{ SQUELCH_NONE,				"no squelch" },
	{ SQUELCH_BAD,				"bad" },
	{ SQUELCH_AVERAGE,			"average" },
	{ SQUELCH_GOOD,				"good" },
	{ SQUELCH_EXCELLENT_NO_HI,	"excellent with no high resists" },
	{ SQUELCH_EXCELLENT_NO_SPL,	"excellent but not splendid" },
	{ SQUELCH_ALL,				"non-artifact" },
};

byte squelch_level[TYPE_MAX];
const size_t squelch_size = TYPE_MAX;



/*
 * Initialise the squelch package (currently just asserts).
 */
void squelch_init(void)
{
}


/*
 * Reset the player's squelch choices for a new game.
 */
void squelch_birth_init(void)
{
	int i;

	/* Reset squelch bits */
	for (i = 0; i < z_info->k_max; i++)
		k_info[i].squelch = FALSE;

	/* Clear the squelch bytes */
	for (i = 0; i < TYPE_MAX; i++)
		squelch_level[i] = SQUELCH_BAD;
}



/*** Autoinscription stuff ***/

const char *get_autoinscription(object_kind *kind)
{
	return kind ? quark_str(kind->note) : NULL;
}

/* Put the autoinscription on an object */
int apply_autoinscription(object_type *o_ptr)
{
	char o_name[80];
	const char *note = get_autoinscription(o_ptr->kind);

	/* Don't inscribe unaware objects */
	if (!note || !object_flavor_is_aware(o_ptr))
		return 0;

	/* Don't re-inscribe if it's already inscribed */
	if (o_ptr->note)
		return 0;

	/* Get an object description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	if (note[0] != 0)
		o_ptr->note = quark_add(note);
	else
		o_ptr->note = 0;

	msg("You autoinscribe %s.", o_name);

	return 1;
}


int remove_autoinscription(s16b kind)
{
	struct object_kind *k = objkind_byid(kind);
	if (!k || !k->note)
		return 0;
	k->note = 0;
	return 1;
}


int add_autoinscription(s16b kind, const char *inscription)
{
	struct object_kind *k = objkind_byid(kind);
	if (!k)
		return 0;
	if (!inscription)
		return remove_autoinscription(kind);
	k->note = quark_add(inscription);
	return 1;
}


void autoinscribe_ground(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	s16b this_o_idx, next_o_idx = 0;

	/* Scan the pile of objects */
	for (this_o_idx = cave->o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the next object */
		next_o_idx = object_byid(this_o_idx)->next_o_idx;

		/* Apply an autoinscription */
		apply_autoinscription(object_byid(this_o_idx));
	}
}

void autoinscribe_pack(void)
{
	int i;

	/* Cycle through the inventory */
	for (i = INVEN_PACK; i >= 0; i--)
	{
		/* Skip empty items */
		if (!p_ptr->inventory[i].kind) continue;

		/* Apply the inscription */
		apply_autoinscription(&p_ptr->inventory[i]);
	}

	return;
}




/*** Squelch code ***/

/*
 * Squelch the flavor of an object
 */
void object_squelch_flavor_of(const object_type *o_ptr)
{
	if (object_flavor_is_aware(o_ptr))
		o_ptr->kind->squelch |= SQUELCH_IF_AWARE;
	else
		o_ptr->kind->squelch |= SQUELCH_IF_UNAWARE;
}


/*
 * Find the squelch type of the object, or TYPE_MAX if none
 */
squelch_type_t squelch_type_of(const object_type *o_ptr)
{
	size_t i;

	/* Find the appropriate squelch group */
	for (i = 0; i < N_ELEMENTS(quality_mapping); i++)
	{
		if ((quality_mapping[i].tval == o_ptr->tval) &&
			(quality_mapping[i].min_sval <= o_ptr->sval) &&
			(quality_mapping[i].max_sval >= o_ptr->sval))
			return quality_mapping[i].squelch_type;
	}

	return TYPE_MAX;
}


/*
 * Determine the squelch level of an object, which is similar to its pseudo.
 *
 * The main point is when the value is undetermined given current info,
 * return the maximum possible value.
 */
byte squelch_level_of(const object_type *o_ptr)
{
	byte value;
	bitflag f[OF_SIZE], f2[OF_SIZE];
	int i;

	object_flags_known(o_ptr, f);

	/* Deal with jewelry specially. */
	if (object_is_jewelry(o_ptr))
	{
		/* CC: average jewelry has at least one known positive pval */
		for (i = 0; i < o_ptr->num_pvals; i++)
			if ((object_this_pval_is_visible(o_ptr, i)) && (o_ptr->pval[i] > 0))
				return SQUELCH_AVERAGE;

		if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
			return SQUELCH_AVERAGE;
		if ((object_attack_plusses_are_visible(o_ptr) &&
				((o_ptr->to_h < 0) || (o_ptr->to_d < 0))) ||
		    	(object_defence_plusses_are_visible(o_ptr) && o_ptr->to_a < 0))
			return SQUELCH_BAD;

		return SQUELCH_AVERAGE;
	}

	/* And lights */
	if (o_ptr->tval == TV_LIGHT)
	{
		create_mask(f2, TRUE, OFID_WIELD, OFT_MAX);
		if (of_is_inter(f, f2))
			return SQUELCH_ALL;
		if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
			return SQUELCH_GOOD;
		if ((o_ptr->to_h < 0) || (o_ptr->to_d < 0) || (o_ptr->to_a < 0))
			return SQUELCH_BAD;

		return SQUELCH_AVERAGE;
	}

	/* CC: we need to redefine "bad" with multiple pvals
	 * At the moment we use "all pvals known and negative" */
	for (i = 0; i < o_ptr->num_pvals; i++) {
		if (!object_this_pval_is_visible(o_ptr, i) ||
			(o_ptr->pval[i] > 0))
			break;

		if (i == (o_ptr->num_pvals - 1))
			return SQUELCH_BAD;
	}

	if (object_was_sensed(o_ptr))
	{
		obj_pseudo_t pseudo = object_pseudo(o_ptr);

		switch (pseudo)
		{
			case INSCRIP_AVERAGE:
				value = SQUELCH_AVERAGE;
				break;

			case INSCRIP_EXCELLENT:
				/* have to assume splendid until you have tested it */
				if (object_was_worn(o_ptr))
				{
					if (object_high_resist_is_possible(o_ptr))
						value = SQUELCH_EXCELLENT_NO_SPL;
					else
						value = SQUELCH_EXCELLENT_NO_HI;
				}
				else
				{
					value = SQUELCH_ALL;
				}
				break;

			case INSCRIP_SPLENDID:
				value = SQUELCH_ALL;
				break;
			case INSCRIP_NULL:
			case INSCRIP_SPECIAL:
				value = SQUELCH_MAX;
				break;

			/* This is the interesting case */
			case INSCRIP_STRANGE:
			case INSCRIP_MAGICAL:
				value = SQUELCH_GOOD;
				if ((object_attack_plusses_are_visible(o_ptr) ||
						randcalc_valid(o_ptr->kind->to_h, o_ptr->to_h) ||
						randcalc_valid(o_ptr->kind->to_d, o_ptr->to_d)) &&
				    	(object_defence_plusses_are_visible(o_ptr) ||
						randcalc_valid(o_ptr->kind->to_a, o_ptr->to_a)) &&
				    	(o_ptr->to_h <= randcalc(o_ptr->kind->to_h, 0, MINIMISE)) &&
				    	(o_ptr->to_d <= randcalc(o_ptr->kind->to_d, 0, MINIMISE)) &&
				    	(o_ptr->to_a <= randcalc(o_ptr->kind->to_a, 0, MINIMISE)))
					value = SQUELCH_BAD;
				break;

			default:
				/* do not handle any other possible pseudo values */
				assert(0);
		}
	}
	else
	{
		if (object_was_worn(o_ptr))
			value = SQUELCH_EXCELLENT_NO_SPL; /* object would be sensed if it were splendid */
		else if (object_is_known_not_artifact(o_ptr))
			value = SQUELCH_ALL;
		else
			value = SQUELCH_MAX;
	}

	return value;
}

/*
 * Remove any squelching of a particular flavor
 */
void kind_squelch_clear(object_kind *k_ptr)
{
	k_ptr->squelch = 0;
	p_ptr->notice |= PN_SQUELCH;
}

bool kind_is_squelched_aware(const object_kind *k_ptr)
{
	return (k_ptr->squelch & SQUELCH_IF_AWARE) ? TRUE : FALSE;
}

bool kind_is_squelched_unaware(const object_kind *k_ptr)
{
	return (k_ptr->squelch & SQUELCH_IF_UNAWARE) ? TRUE : FALSE;
}

void kind_squelch_when_aware(object_kind *k_ptr)
{
	k_ptr->squelch |= SQUELCH_IF_AWARE;
	p_ptr->notice |= PN_SQUELCH;
}

void kind_squelch_when_unaware(object_kind *k_ptr)
{
	k_ptr->squelch |= SQUELCH_IF_UNAWARE;
	p_ptr->notice |= PN_SQUELCH;
}


/*
 * Determines if an object is eligible for squelching.
 */
bool squelch_item_ok(const object_type *o_ptr)
{
	byte type;

	if (p_ptr->unignoring)
		return FALSE;

	/* Don't squelch artifacts unless marked to be squelched */
	if (o_ptr->artifact ||
			check_for_inscrip(o_ptr, "!k") || check_for_inscrip(o_ptr, "!*"))
		return FALSE;

	/* Do squelch individual objects that marked ignore */
	if (o_ptr->ignore)
		return TRUE;

	/* Auto-squelch dead chests */
	if (o_ptr->tval == TV_CHEST && o_ptr->pval[DEFAULT_PVAL] == 0)
		return TRUE;

	/* Do squelching by kind */
	if (object_flavor_is_aware(o_ptr) ?
		 kind_is_squelched_aware(o_ptr->kind) :
		 kind_is_squelched_unaware(o_ptr->kind))
		return TRUE;

	type = squelch_type_of(o_ptr);
	if (type == TYPE_MAX)
		return FALSE;

	/* Squelch items known not to be special */
	if (object_is_known_not_artifact(o_ptr) && squelch_level[type] == SQUELCH_ALL)
		return TRUE;

	/* Get result based on the feeling and the squelch_level */
	if (squelch_level_of(o_ptr) <= squelch_level[type])
		return TRUE;
	else
		return FALSE;
}

/*
 * Drop all {squelch}able items.
 */
void squelch_drop(void)
{
	int n;

	/* Scan through the slots backwards */
	for (n = INVEN_PACK - 1; n >= 0; n--)
	{
		object_type *o_ptr = &p_ptr->inventory[n];

		/* Skip non-objects and unsquelchable objects */
		if (!o_ptr->kind) continue;
		if (!squelch_item_ok(o_ptr)) continue;

		/* Check for !d (no drop) inscription */
		if (!check_for_inscrip(o_ptr, "!d") && !check_for_inscrip(o_ptr, "!*"))
		{
			/* We're allowed to drop it. */
			inven_drop(n, o_ptr->number);
		}
	}

	/* Combine/reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
}
