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
	{ TYPE_WEAPON_POINTY,	TV_SWORD,	0,		SV_UNKNOWN },
	{ TYPE_WEAPON_POINTY,	TV_POLEARM,	0,		SV_UNKNOWN },
	{ TYPE_WEAPON_BLUNT,	TV_HAFTED,	0,		SV_UNKNOWN },
	{ TYPE_SHOOTER,		TV_BOW,		0,		SV_UNKNOWN },
	{ TYPE_MISSILE_SLING,	TV_SHOT,	0,		SV_UNKNOWN },
	{ TYPE_MISSILE_BOW,	TV_ARROW,	0,		SV_UNKNOWN },
	{ TYPE_MISSILE_XBOW,	TV_BOLT,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_ROBE,	TV_SOFT_ARMOR,	SV_ROBE,	SV_ROBE },
	{ TYPE_ARMOR_BODY,	TV_DRAG_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_BODY,	TV_HARD_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_BODY,	TV_SOFT_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_CLOAK,	TV_CLOAK,	SV_CLOAK, 	SV_FUR_CLOAK },
	{ TYPE_ARMOR_CLOAK,	TV_CLOAK,	SV_ETHEREAL_CLOAK, 	SV_ETHEREAL_CLOAK },
/* XXX Eddie need to assert SV_CLOAK < SV_FUR_CLOAK < SV_ELVEN_CLOAK */
	{ TYPE_ARMOR_ELVEN_CLOAK,	TV_CLOAK,	SV_ELVEN_CLOAK, 	SV_ELVEN_CLOAK },
	{ TYPE_ARMOR_SHIELD,	TV_SHIELD,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HEAD,	TV_HELM,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HEAD,	TV_CROWN,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HANDS,	TV_GLOVES,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_FEET,	TV_BOOTS,	0,		SV_UNKNOWN },
	{ TYPE_DIGGER,		TV_DIGGING,	0,		SV_UNKNOWN },
	{ TYPE_RING,		TV_RING,	0,		SV_UNKNOWN },
	{ TYPE_AMULET,		TV_AMULET,	0,		SV_UNKNOWN },
	{ TYPE_LIGHT, 		TV_LIGHT, 	0,		SV_UNKNOWN },
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
		squelch_level[i] = 0;
}



/*** Autoinscription stuff ***/

/*
 * This code needs documenting.
 */
int get_autoinscription_index(s16b k_idx)
{
	int i;

	for (i = 0; i < inscriptions_count; i++)
	{
		if (k_idx == inscriptions[i].kind_idx)
			return i;
	}

	return -1;
}

/*
 * DOCUMENT ME!
 */
const char *get_autoinscription(s16b kind_idx)
{
	int i;

	for (i = 0; i < inscriptions_count; i++)
	{
		if (kind_idx == inscriptions[i].kind_idx)
			return quark_str(inscriptions[i].inscription_idx);
	}

	return 0;
}

/* Put the autoinscription on an object */
int apply_autoinscription(object_type *o_ptr)
{
	char o_name[80];
	cptr note = get_autoinscription(o_ptr->k_idx);
	cptr existing_inscription = quark_str(o_ptr->note);

	/* Don't inscribe unaware objects */
	if (!note || !object_flavor_is_aware(o_ptr))
		return 0;

	/* Don't re-inscribe if it's already inscribed */
	if (existing_inscription)
		return 0;

	/* Get an object description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	if (note[0] != 0)
		o_ptr->note = quark_add(note);
	else
		o_ptr->note = 0;

	msg_format("You autoinscribe %s.", o_name);

	return 1;
}


int remove_autoinscription(s16b kind)
{
	int i = get_autoinscription_index(kind);

	/* It's not here. */
	if (i == -1) return 0;

	while (i < inscriptions_count - 1)
	{
		inscriptions[i] = inscriptions[i+1];
		i++;
	}

	inscriptions_count--;

	return 1;
}


int add_autoinscription(s16b kind, cptr inscription)
{
	int index;

	/* Paranoia */
	if (kind == 0) return 0;

	/* If there's no inscription, remove it */
	if (!inscription || (inscription[0] == 0))
		return remove_autoinscription(kind);

	index = get_autoinscription_index(kind);

	if (index == -1)
		index = inscriptions_count;

	if (index >= AUTOINSCRIPTIONS_MAX)
	{
		msg_format("This inscription (%s) cannot be added because the inscription array is full!", inscription);
		return 0;
	}

	inscriptions[index].kind_idx = kind;
	inscriptions[index].inscription_idx = quark_add(inscription);

	/* Only increment count if inscription added to end of array */
	if (index == inscriptions_count)
		inscriptions_count++;

	return 1;
}


void autoinscribe_ground(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	s16b this_o_idx, next_o_idx = 0;

	/* Scan the pile of objects */
	for (this_o_idx = cave_o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the next object */
		next_o_idx = o_list[this_o_idx].next_o_idx;

		/* Apply an autoinscription */
		apply_autoinscription(&o_list[this_o_idx]);
	}
}

void autoinscribe_pack(void)
{
	int i;

	/* Cycle through the inventory */
	for (i = INVEN_PACK; i >= 0; i--)
	{
		/* Skip empty items */
		if (!p_ptr->inventory[i].k_idx) continue;

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
		k_info[o_ptr->k_idx].squelch |= SQUELCH_IF_AWARE;
	else
		k_info[o_ptr->k_idx].squelch |= SQUELCH_IF_UNAWARE;
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
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	byte value;
	bitflag f[OF_SIZE];

	object_flags_known(o_ptr, f);

	if ((object_pval_is_visible(o_ptr)) && (o_ptr->pval < 0))
		return SQUELCH_BAD;

	/* Deal with jewelry specially. */
	if (object_is_jewelry(o_ptr))
	{
		if ((object_pval_is_visible(o_ptr)) && (o_ptr->pval > 0))
			return SQUELCH_AVERAGE;
		if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
			return SQUELCH_AVERAGE;
		if ((object_attack_plusses_are_visible(o_ptr) && ((o_ptr->to_h < 0) || (o_ptr->to_d < 0))) ||
		    (object_defence_plusses_are_visible(o_ptr) && (o_ptr->to_a < 0)))
			return SQUELCH_BAD;

		return SQUELCH_AVERAGE;
	}

	/* And lights */
	if (o_ptr->tval == TV_LIGHT)
	{
		if (flags_test(f, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END))
			return SQUELCH_ALL;
		if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
			return SQUELCH_GOOD;
		if ((o_ptr->to_h < 0) || (o_ptr->to_d < 0) || (o_ptr->to_a < 0))
			return SQUELCH_BAD;

		return SQUELCH_AVERAGE;
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

			case INSCRIP_STRANGE: /* XXX Eddie perhaps some strange count as something else */
			case INSCRIP_SPLENDID:
				value = SQUELCH_ALL;
				break;
			case INSCRIP_NULL:
			case INSCRIP_SPECIAL:
				value = SQUELCH_MAX;
				break;

			/* This is the interesting case */
			case INSCRIP_MAGICAL:
				value = SQUELCH_GOOD;
				if ((object_attack_plusses_are_visible(o_ptr) || (randcalc_valid(k_ptr->to_h, o_ptr->to_h) && randcalc_valid(k_ptr->to_d, o_ptr->to_d))) &&
				    (object_defence_plusses_are_visible(o_ptr) || (randcalc_valid(k_ptr->to_a, o_ptr->to_a))) &&
				    (o_ptr->to_h <= randcalc(k_ptr->to_h, 0, MINIMISE)) &&
				    (o_ptr->to_d <= randcalc(k_ptr->to_d, 0, MINIMISE)) &&
				    (o_ptr->to_a <= randcalc(k_ptr->to_a, 0, MINIMISE)))
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
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	byte type;

	/* Don't squelch artifacts unless marked to be squelched */
	if (artifact_p(o_ptr))
		return FALSE;

	/* Don't squelch stuff inscribed not to be destroyed (!k) */
	if (check_for_inscrip(o_ptr, "!k") || check_for_inscrip(o_ptr, "!*"))
		return FALSE;

	/* Auto-squelch dead chests */
	if (o_ptr->tval == TV_CHEST && o_ptr->pval == 0)
		return TRUE;

	/* check option for worthless kinds */
	if (OPT(squelch_worthless) && o_ptr->tval != TV_GOLD)
	{
		if (object_flavor_is_aware(o_ptr) && k_ptr->cost == 0)
			return TRUE;
		if (object_is_known_cursed(o_ptr))
			return TRUE;
	}

	/* Do squelching by kind */
	if (object_flavor_is_aware(o_ptr) ?
		 kind_is_squelched_aware(k_ptr) :
		 kind_is_squelched_unaware(k_ptr))
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
 * Returns TRUE if an item should be hidden due to the player's
 * current settings.
 */
bool squelch_hide_item(object_type *o_ptr)
{
	return (OPT(hide_squelchable) ? squelch_item_ok(o_ptr) : FALSE);
}


/*
 * Destroy all {squelch}able items.
 *
 * Imported, with thanks, from Ey... much cleaner than the original.
 */
void squelch_items(void)
{
	int floor_list[MAX_FLOOR_STACK];
	int floor_num, n;
	int count = 0;

	object_type *o_ptr;

	/* Set the hook and scan the floor */
	item_tester_hook = squelch_item_ok;
	floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), p_ptr->py, p_ptr->px, 0x01);

	if (floor_num)
	{
		for (n = 0; n < floor_num; n++)
		{
			o_ptr = &o_list[floor_list[n]];

			/* Avoid artifacts */
			if (artifact_p(o_ptr)) continue;

			if (item_tester_okay(o_ptr))
			{
				/* Destroy item */
				floor_item_increase(floor_list[n], -o_ptr->number);
				floor_item_optimize(floor_list[n]);
				count++;
			}
		}
	}

	/* Scan through the slots backwards */
	for (n = INVEN_PACK - 1; n >= 0; n--)
	{
		o_ptr = &p_ptr->inventory[n];

		/* Skip non-objects and artifacts */
		if (!o_ptr->k_idx) continue;
		if (artifact_p(o_ptr)) continue;

		if (item_tester_okay(o_ptr))
		{
			/* Destroy item */
			inven_item_increase(n, -o_ptr->number);
			inven_item_optimize(n);
			count++;
		}
	}

	item_tester_hook = NULL;

	/* Mention casualties */
	if (count > 0)
	{
		message_format(MSG_DESTROY, 0, "%d item%s squelched.",
		               count, ((count > 1) ? "s" : ""));

		/* Combine/reorder the pack */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);
	}
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
		if (!o_ptr->k_idx) continue;
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
