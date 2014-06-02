/*
 * File: squelch.c
 * Purpose: Item destruction
 *
 * Copyright (c) 2007 David T. Blackston, Iain McFall, DarkGod, Jeff Greene,
 * David Vestal, Pete Mack, Andi Sidwell.
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
#include "init.h"
#include "ui-menu.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-tval.h"
#include "obj-ui.h"
#include "obj-util.h"
#include "object.h"
#include "squelch.h"


typedef struct
{
	squelch_type_t squelch_type;
	int tval;
	const char *identifier;
} quality_squelch_struct;

/**
 * Any entry here with an identifier should appear above the entry with the
 * same tval and no identifier
 */
static quality_squelch_struct quality_mapping[] =
{
	{ ITYPE_GREAT,			TV_SWORD,		"Chaos" },
	{ ITYPE_GREAT,			TV_POLEARM,		"Slicing" },
	{ ITYPE_GREAT,			TV_HAFTED,		"Disruption" },
	{ ITYPE_SHARP,			TV_SWORD,		"" },
	{ ITYPE_SHARP,			TV_POLEARM,		"" },
	{ ITYPE_BLUNT,			TV_HAFTED,		"" },
	{ ITYPE_SHOOTER,		TV_BOW,			"" },
	{ ITYPE_SHOT,			TV_SHOT,		"" },
	{ ITYPE_ARROW,			TV_ARROW,		"" },
	{ ITYPE_BOLT,			TV_BOLT,		"" },
	{ ITYPE_ROBE,			TV_SOFT_ARMOR,	"Robe" },
	{ ITYPE_DRAGON_ARMOR,	TV_DRAG_ARMOR,	"" },
	{ ITYPE_BODY_ARMOR,		TV_HARD_ARMOR,	"" },
	{ ITYPE_BODY_ARMOR,		TV_SOFT_ARMOR,	"" },
	{ ITYPE_ELVEN_CLOAK,	TV_CLOAK,		"Elven" },
	{ ITYPE_CLOAK,			TV_CLOAK,		"" },
	{ ITYPE_SHIELD,			TV_SHIELD,		"" },
	{ ITYPE_HEADGEAR,		TV_HELM,		"" },
	{ ITYPE_HEADGEAR,		TV_CROWN,		"" },
	{ ITYPE_HANDGEAR,		TV_GLOVES,		"" },
	{ ITYPE_FEET,			TV_BOOTS,		"" },
	{ ITYPE_DIGGER,			TV_DIGGING,		"" },
	{ ITYPE_RING,			TV_RING,		"" },
	{ ITYPE_AMULET,			TV_AMULET,		"" },
	{ ITYPE_LIGHT, 			TV_LIGHT, 		"" },
};



quality_name_struct quality_choices[] =
{
	#define ITYPE(a, b) { ITYPE_##a, b },
	#include "list-ignore-types.h"
	#undef ITYPE
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

byte squelch_level[ITYPE_MAX];
const size_t squelch_size = ITYPE_MAX;
bool **ego_ignore_types;


/**
 * Initialise the ignore package 
 */
void init_ignore(void)
{
	ego_ignore_types = mem_zalloc(z_info->e_max * ITYPE_MAX * sizeof(bool));
}


/**
 * Clean up the ignore package
 */
void cleanup_ignore(void)
{
	mem_free(ego_ignore_types);
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
	for (i = 0; i < ITYPE_MAX; i++)
		squelch_level[i] = SQUELCH_NONE;
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
	int py = player->py;
	int px = player->px;
	s16b this_o_idx, next_o_idx = 0;

	/* Scan the pile of objects */
	for (this_o_idx = cave->o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the next object */
		next_o_idx = cave_object(cave, this_o_idx)->next_o_idx;

		/* Apply an autoinscription */
		apply_autoinscription(cave_object(cave, this_o_idx));
	}
}

void autoinscribe_pack(void)
{
	int i;

	/* Cycle through the inventory */
	for (i = player->max_gear - 1; i >= 0; i--)
	{
		/* Skip empty items */
		if (!player->gear[i].kind) continue;

		/* Apply the inscription */
		apply_autoinscription(&player->gear[i]);
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


/**
 * Find the squelch type of the object, or ITYPE_MAX if none
 */
squelch_type_t squelch_type_of(const object_type *o_ptr)
{
	size_t i;

	/* Find the appropriate squelch group */
	for (i = 0; i < N_ELEMENTS(quality_mapping); i++) {
		if (quality_mapping[i].tval == o_ptr->tval) {
			/* If there's an identifier, it must match */
			if (quality_mapping[i].identifier[0]) {
				if (!strstr(quality_mapping[i].identifier, o_ptr->kind->name))
					continue;
			}
			/* Otherwise we're fine */
			return quality_mapping[i].squelch_type;
		}
	}

	return ITYPE_MAX;
}

/**
 * Small helper function to see how an object trait compares to the one
 * in its base type.
 *
 * If the base type provides a positive bonus, we'll use that. Otherwise, we'll
 * use zero (players don't consider an item with a positive bonus to be bad
 * even if the base kind has a higher positive bonus).
 */
static int cmp_object_trait(int bonus, random_value base)
{
	int amt = randcalc(base, 0, MINIMISE);
	if (amt > 0) amt = 0;
	return CMP(bonus, amt);
}

/**
 * Small helper function to see if an item seems good, bad or average based on
 * to_h, to_d and to_a.
 *
 * The sign of the return value announces if the object is bad (negative),
 * good (positive) or average (zero).
 */
static int is_object_good(const object_type *o_ptr)
{
	int good = 0;
	good += 4 * cmp_object_trait(o_ptr->to_d, o_ptr->kind->to_d);
	good += 2 * cmp_object_trait(o_ptr->to_h, o_ptr->kind->to_h);
	good += 1 * cmp_object_trait(o_ptr->to_a, o_ptr->kind->to_a);
	return good;
}


/*
 * Determine the squelch level of an object, which is similar to its pseudo.
 *
 * The main point is when the value is undetermined given current info,
 * return the maximum possible value.
 */
byte squelch_level_of(const object_type *o_ptr)
{
	byte value = 0;
	bitflag f[OF_SIZE], f2[OF_SIZE];
	int i;

	object_flags_known(o_ptr, f);

	/* Deal with jewelry specially. */
	if (tval_is_jewelry(o_ptr))
	{
		/* CC: average jewelry has at least one known positive modifier */
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if ((object_this_mod_is_visible(o_ptr, i)) && 
				(o_ptr->modifiers[i] > 0))
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
	if (tval_is_light(o_ptr))
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

	/* We need to redefine "bad" 
	 * At the moment we use "all modifiers known and negative" */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (!object_this_mod_is_visible(o_ptr, i) ||
			(o_ptr->modifiers[i] > 0))
			break;
	}

	if (i == OBJ_MOD_MAX)
		return SQUELCH_BAD;

	if (object_was_sensed(o_ptr)) {
		obj_pseudo_t pseudo = object_pseudo(o_ptr);

		switch (pseudo) {
			case INSCRIP_AVERAGE: {
				value = SQUELCH_AVERAGE;
				break;
			}

			case INSCRIP_EXCELLENT: {
				/* have to assume splendid until you have tested it */
				if (object_was_worn(o_ptr)) {
					if (object_high_resist_is_possible(o_ptr))
						value = SQUELCH_EXCELLENT_NO_SPL;
					else
						value = SQUELCH_EXCELLENT_NO_HI;
				} else {
					value = SQUELCH_ALL;
				}
				break;
			}

			case INSCRIP_SPLENDID:
				value = SQUELCH_ALL;
				break;
			case INSCRIP_NULL:
			case INSCRIP_SPECIAL:
				value = SQUELCH_MAX;
				break;

			/* This is the interesting case */
			case INSCRIP_STRANGE:
			case INSCRIP_MAGICAL: {
				value = SQUELCH_GOOD;

				if ((object_attack_plusses_are_visible(o_ptr) ||
						randcalc_valid(o_ptr->kind->to_h, o_ptr->to_h) ||
						randcalc_valid(o_ptr->kind->to_d, o_ptr->to_d)) &&
				    	(object_defence_plusses_are_visible(o_ptr) ||
						randcalc_valid(o_ptr->kind->to_a, o_ptr->to_a))) {
					int isgood = is_object_good(o_ptr);
					if (isgood > 0) {
						value = SQUELCH_GOOD;
					} else if (isgood < 0) {
						value = SQUELCH_BAD;
					} else {
						value = SQUELCH_AVERAGE;
					}
				}
				break;
			}

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
	player->upkeep->notice |= PN_SQUELCH;
}

void ego_squelch(struct object *obj)
{
	obj->ego->squelch = TRUE;
	player->upkeep->notice |= PN_SQUELCH;
}

void ego_squelch_clear(struct object *obj)
{
	obj->ego->squelch = FALSE;
	player->upkeep->notice |= PN_SQUELCH;
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
	player->upkeep->notice |= PN_SQUELCH;
}

void kind_squelch_when_unaware(object_kind *k_ptr)
{
	k_ptr->squelch |= SQUELCH_IF_UNAWARE;
	player->upkeep->notice |= PN_SQUELCH;
}


/**
 * Determines if an object is already squelched.
 */
bool object_is_squelched(const object_type *o_ptr)
{
	byte type;

	/* Do squelch individual objects that marked ignore */
	if (o_ptr->ignore)
		return TRUE;

	/* Don't squelch artifacts unless marked to be squelched */
	if (o_ptr->artifact ||
		check_for_inscrip(o_ptr, "!k") || check_for_inscrip(o_ptr, "!*"))
		return FALSE;

	/* Do squelching by kind */
	if (object_flavor_is_aware(o_ptr) ?
		 kind_is_squelched_aware(o_ptr->kind) :
		 kind_is_squelched_unaware(o_ptr->kind))
		return TRUE;

	/* Squelch ego items if known */
	if (object_ego_is_visible(o_ptr) && (o_ptr->ego->squelch))
		return TRUE;

	type = squelch_type_of(o_ptr);
	if (type == ITYPE_MAX)
		return FALSE;

	/* Squelch items known not to be special */
	if (object_is_known_not_artifact(o_ptr) &&
		squelch_level[type] == SQUELCH_ALL)
		return TRUE;

	/* Get result based on the feeling and the squelch_level */
	if (squelch_level_of(o_ptr) <= squelch_level[type])
		return TRUE;
	else
		return FALSE;
}

/**
 * Determines if an object is eligible for squelching.
 */
bool squelch_item_ok(const object_type *o_ptr)
{
	if (player->unignoring)
		return FALSE;

	return object_is_squelched(o_ptr);
}

/*
 * Drop all {squelch}able items.
 */
void squelch_drop(void)
{
	int n;

	/* Scan through the slots backwards */
	for (n = player->max_gear - 1; n >= 0; n--)
	{
		object_type *o_ptr = &player->gear[n];

		/* Skip non-objects and unsquelchable objects */
		if (!o_ptr->kind) continue;
		if (!squelch_item_ok(o_ptr)) continue;

		/* Check for !d (no drop) inscription */
		if (!check_for_inscrip(o_ptr, "!d") && !check_for_inscrip(o_ptr, "!*"))
		{
			/* Confirm the drop if the item is equipped. */
			if (item_is_equipped(player, n)) {
				if (!verify_item("Really take off and drop", n)) {
					/* Hack - inscribe the item with !d to prevent repeated confirmations. */
					const char *inscription = quark_str(o_ptr->note);

					if (inscription == NULL) {
						o_ptr->note = quark_add("!d");
					}
					else {
						char buffer[1024];
						my_strcpy(buffer, inscription, sizeof(buffer));
						my_strcat(buffer, "!d", sizeof(buffer));
						o_ptr->note = quark_add(buffer);
					}

					continue;
				}
			}

			/* We're allowed to drop it. */
			inven_drop(n, o_ptr->number);
		}
	}

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine/reorder the pack */
	player->upkeep->notice |= (PN_COMBINE);
}

/**
 * Return the name of a squelch type.
 */
const char *squelch_name_for_type(squelch_type_t type)
{
	size_t i;

	for (i = 0; i < ITYPE_MAX; i++) {
		if (quality_choices[i].enum_val == type)
			return quality_choices[i].name;
	}

	return "unknown";
}

struct init_module ignore_module = {
	.name = "ignore",
	.init = init_ignore,
	.cleanup = cleanup_ignore
};
