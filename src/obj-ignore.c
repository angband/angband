/**
 * \file obj-ignore.c
 * \brief Item ignoring
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
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"


typedef struct
{
	ignore_type_t ignore_type;
	int tval;
	const char *identifier;
} quality_ignore_struct;

/**
 * Any entry here with an identifier should appear above the entry with the
 * same tval and no identifier
 */
static quality_ignore_struct quality_mapping[] =
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

/**
 * The names for the various kinds of quality
 */
quality_name_struct quality_values[IGNORE_MAX] =
{
	{ IGNORE_NONE,				"no ignore" },
	{ IGNORE_BAD,				"bad" },
	{ IGNORE_AVERAGE,			"average" },
	{ IGNORE_GOOD,				"good" },
	{ IGNORE_EXCELLENT_NO_HI,	"excellent with no high resists" },
	{ IGNORE_EXCELLENT_NO_SPL,	"excellent but not splendid" },
	{ IGNORE_ALL,				"non-artifact" },
};

byte ignore_level[ITYPE_MAX];
const size_t ignore_size = ITYPE_MAX;
bool **ego_ignore_types;
/* Hackish - ego_ignore_types should be initialised with arrays */
int num_ego_types;


/**
 * Initialise the ignore package 
 */
void init_ignore(void)
{
	int i;

	num_ego_types = z_info->e_max;
	ego_ignore_types = mem_zalloc(z_info->e_max * sizeof(bool*));
	for (i = 0; i < z_info->e_max; i++)
		ego_ignore_types[i] = mem_zalloc(ITYPE_MAX * sizeof(bool));
}


/**
 * Clean up the ignore package
 */
void cleanup_ignore(void)
{
	int i;
	for (i = 0; i < num_ego_types; i++)
		mem_free(ego_ignore_types[i]);
	mem_free(ego_ignore_types);
}


/**
 * Reset the player's ignore choices for a new game.
 */
void ignore_birth_init(void)
{
	int i, j;

	/* Reset ignore bits */
	for (i = 0; i < z_info->k_max; i++)
		k_info[i].ignore = FALSE;

	/* Clear the ignore bytes */
	for (i = ITYPE_NONE; i < ITYPE_MAX; i++)
		ignore_level[i] = IGNORE_NONE;

	/* Clear ego ignore */
	for (i = 0; i < z_info->e_max; i++)
		for (j = ITYPE_NONE; j < ITYPE_MAX; j++)
			ego_ignore_types[i][j] = 0;
}



/**
 * ------------------------------------------------------------------------
 * Autoinscription stuff
 * ------------------------------------------------------------------------ */


/**
 * Return an object kind autoinscription
 */
const char *get_autoinscription(struct object_kind *kind)
{
	return kind ? quark_str(kind->note) : NULL;
}

/**
 * Put an autoinscription on an object
 */
int apply_autoinscription(struct object *obj)
{
	char o_name[80];
	const char *note = obj ? get_autoinscription(obj->kind) : NULL;

	/* Don't inscribe unaware objects */
	if (!note || !object_flavor_is_aware(obj))
		return 0;

	/* Don't re-inscribe if it's already inscribed */
	if (obj->note)
		return 0;

	/* Get an object description */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

	if (note[0] != 0)
		obj->note = quark_add(note);
	else
		obj->note = 0;

	msg("You autoinscribe %s.", o_name);

	return 1;
}


/**
 * Deregister an object kind autoinscription
 */
int remove_autoinscription(s16b kind)
{
	struct object_kind *k = objkind_byid(kind);
	if (!k || !k->note)
		return 0;
	k->note = 0;
	return 1;
}


/**
 * Register an object kind autoinscription
 */
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


/**
 * Put an autoinscription on all objects on the floor beneath the player
 */
void autoinscribe_ground(void)
{
	int py = player->py;
	int px = player->px;
	struct object *obj;

	/* Autoinscribe each object in the pile */
	for (obj = square_object(cave, py, px); obj; obj = obj->next)
		apply_autoinscription(obj);
}

/**
 * Put an autoinscription on all the player's carried objects
 */
void autoinscribe_pack(void)
{
	struct object *obj;

	/* Autoinscribe each object in the inventory */
	for (obj = player->gear; obj; obj = obj->next)
		apply_autoinscription(obj);
}


/**
 * ------------------------------------------------------------------------
 * Ignore code
 * ------------------------------------------------------------------------ */


/**
 * Ignore the flavor of an object
 */
void object_ignore_flavor_of(const struct object *obj)
{
	if (object_flavor_is_aware(obj))
		obj->kind->ignore |= IGNORE_IF_AWARE;
	else
		obj->kind->ignore |= IGNORE_IF_UNAWARE;
}


/**
 * Find the ignore type of the object, or ITYPE_MAX if none
 */
ignore_type_t ignore_type_of(const struct object *obj)
{
	size_t i;

	/* Find the appropriate ignore group */
	for (i = 0; i < N_ELEMENTS(quality_mapping); i++) {
		if (quality_mapping[i].tval == obj->tval) {
			/* If there's an identifier, it must match */
			if (quality_mapping[i].identifier[0]) {
				if (!strstr(obj->kind->name, quality_mapping[i].identifier))
					continue;
			}
			/* Otherwise we're fine */
			return quality_mapping[i].ignore_type;
		}
	}

	return ITYPE_MAX;
}

/**
 * Find whether an ignore type is valid for a given tval
 */
bool tval_has_ignore_type(int tval, ignore_type_t itype)
{
	size_t i;

	/* Find the appropriate ignore group */
	for (i = 0; i < N_ELEMENTS(quality_mapping); i++)
		if ((quality_mapping[i].tval == tval) &&
			(quality_mapping[i].ignore_type == itype))
			return TRUE;

	return FALSE;
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
static int is_object_good(const struct object *obj)
{
	int good = 0;
	good += 4 * cmp_object_trait(obj->to_d, obj->kind->to_d);
	good += 2 * cmp_object_trait(obj->to_h, obj->kind->to_h);
	good += 1 * cmp_object_trait(obj->to_a, obj->kind->to_a);
	return good;
}


/**
 * Determine the ignore level of an object, which is similar to its pseudo.
 *
 * The main point is when the value is undetermined given current info,
 * return the maximum possible value.
 */
byte ignore_level_of(const struct object *obj)
{
	byte value = 0;
	bitflag f[OF_SIZE], f2[OF_SIZE];
	int i;
	bool negative_mod = FALSE;

	object_flags_known(obj, f);

	/* Deal with jewelry specially. */
	if (tval_is_jewelry(obj))
	{
		/* CC: average jewelry has at least one known positive modifier */
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if ((object_this_mod_is_visible(obj, i)) && 
				(obj->modifiers[i] > 0))
				return IGNORE_AVERAGE;

		if ((obj->to_h > 0) || (obj->to_d > 0) || (obj->to_a > 0))
			return IGNORE_AVERAGE;
		if ((object_attack_plusses_are_visible(obj) &&
				((obj->to_h < 0) || (obj->to_d < 0))) ||
		    	(object_defence_plusses_are_visible(obj) && obj->to_a < 0))
			return IGNORE_BAD;

		return IGNORE_AVERAGE;
	}

	/* And lights */
	if (tval_is_light(obj))
	{
		create_mask(f2, TRUE, OFID_WIELD, OFT_MAX);
		if (of_is_inter(f, f2))
			return IGNORE_ALL;
		if ((obj->to_h > 0) || (obj->to_d > 0) || (obj->to_a > 0))
			return IGNORE_GOOD;
		if ((obj->to_h < 0) || (obj->to_d < 0) || (obj->to_a < 0))
			return IGNORE_BAD;

		return IGNORE_AVERAGE;
	}

	/* We need to redefine "bad" 
	 * At the moment we use "all modifiers known and negative" */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		if (!object_this_mod_is_visible(obj, i) ||
			(obj->modifiers[i] > 0))
			break;

		if (obj->modifiers[i] < 0)
			negative_mod = TRUE;
	}

	if ((i == OBJ_MOD_MAX) && negative_mod)
		return IGNORE_BAD;

	if (object_was_sensed(obj)) {
		obj_pseudo_t pseudo = object_pseudo(obj);

		switch (pseudo) {
			case INSCRIP_AVERAGE: {
				value = IGNORE_AVERAGE;
				break;
			}

			case INSCRIP_EXCELLENT: {
				/* have to assume splendid until you have tested it */
				if (object_was_worn(obj)) {
					if (object_high_resist_is_possible(obj))
						value = IGNORE_EXCELLENT_NO_SPL;
					else
						value = IGNORE_EXCELLENT_NO_HI;
				} else {
					value = IGNORE_ALL;
				}
				break;
			}

			case INSCRIP_SPLENDID:
				value = IGNORE_ALL;
				break;
			case INSCRIP_NULL:
			case INSCRIP_SPECIAL:
				value = IGNORE_MAX;
				break;

			/* This is the interesting case */
			case INSCRIP_STRANGE:
			case INSCRIP_MAGICAL: {
				value = IGNORE_GOOD;

				if ((object_attack_plusses_are_visible(obj) ||
						randcalc_valid(obj->kind->to_h, obj->to_h) ||
						randcalc_valid(obj->kind->to_d, obj->to_d)) &&
				    	(object_defence_plusses_are_visible(obj) ||
						randcalc_valid(obj->kind->to_a, obj->to_a))) {
					int isgood = is_object_good(obj);
					if (isgood > 0) {
						value = IGNORE_GOOD;
					} else if (isgood < 0) {
						value = IGNORE_BAD;
					} else {
						value = IGNORE_AVERAGE;
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
		if (object_was_worn(obj))
			value = IGNORE_EXCELLENT_NO_SPL; /* object would be sensed if it were splendid */
		else if (object_is_known_not_artifact(obj))
			value = IGNORE_ALL;
		else
			value = IGNORE_MAX;
	}

	return value;
}

/**
 * Remove any ignoring of a particular flavor
 */
void kind_ignore_clear(struct object_kind *kind)
{
	kind->ignore = 0;
	player->upkeep->notice |= PN_IGNORE;
}

void ego_ignore(struct object *obj)
{
	assert(obj->ego);
	ego_ignore_types[obj->ego->eidx][ignore_type_of(obj)] = TRUE;
	player->upkeep->notice |= PN_IGNORE;
}

void ego_ignore_clear(struct object *obj)
{
	assert(obj->ego);
	ego_ignore_types[obj->ego->eidx][ignore_type_of(obj)] = FALSE;
	player->upkeep->notice |= PN_IGNORE;
}

void ego_ignore_toggle(int e_idx, int itype)
{
	ego_ignore_types[e_idx][itype] = !ego_ignore_types[e_idx][itype];
	player->upkeep->notice |= PN_IGNORE;
}

bool ego_is_ignored(int e_idx, int itype)
{
	return ego_ignore_types[e_idx][itype];
}

bool kind_is_ignored_aware(const struct object_kind *kind)
{
	return (kind->ignore & IGNORE_IF_AWARE) ? TRUE : FALSE;
}

bool kind_is_ignored_unaware(const struct object_kind *kind)
{
	return (kind->ignore & IGNORE_IF_UNAWARE) ? TRUE : FALSE;
}

void kind_ignore_when_aware(struct object_kind *kind)
{
	kind->ignore |= IGNORE_IF_AWARE;
	player->upkeep->notice |= PN_IGNORE;
}

void kind_ignore_when_unaware(struct object_kind *kind)
{
	kind->ignore |= IGNORE_IF_UNAWARE;
	player->upkeep->notice |= PN_IGNORE;
}


/**
 * Determines if an object is already ignored.
 */
bool object_is_ignored(const struct object *obj)
{
	byte type;

	/* Do ignore individual objects that marked ignore */
	if (obj->ignore)
		return TRUE;

	/* Don't ignore artifacts unless marked to be ignored */
	if (obj->artifact ||
		check_for_inscrip(obj, "!k") || check_for_inscrip(obj, "!*"))
		return FALSE;

	/* Do ignoring by kind */
	if (object_flavor_is_aware(obj) ?
		 kind_is_ignored_aware(obj->kind) :
		 kind_is_ignored_unaware(obj->kind))
		return TRUE;

	/* Ignore ego items if known */
	if (object_ego_is_visible(obj) &&
		ego_is_ignored(obj->ego->eidx, ignore_type_of(obj)))
		return TRUE;

	type = ignore_type_of(obj);
	if (type == ITYPE_MAX)
		return FALSE;

	/* Ignore items known not to be special */
	if (object_is_known_not_artifact(obj) &&
		ignore_level[type] == IGNORE_ALL)
		return TRUE;

	/* Get result based on the feeling and the ignore_level */
	if (ignore_level_of(obj) <= ignore_level[type])
		return TRUE;
	else
		return FALSE;
}

/**
 * Determines if an object is eligible for ignoring.
 */
bool ignore_item_ok(const struct object *obj)
{
	if (player->unignoring)
		return FALSE;

	return object_is_ignored(obj);
}

/**
 * Drop all {ignore}able items.
 */
void ignore_drop(void)
{
	struct object *obj;

	/* Scan through the slots backwards */
	for (obj = gear_last_item(); obj; obj = obj->prev) {
		/* Skip non-objects and unignoreable objects */
		assert(obj->kind);
		if (!ignore_item_ok(obj)) continue;

		/* Check for !d (no drop) inscription */
		if (!check_for_inscrip(obj, "!d") && !check_for_inscrip(obj, "!*")) {
			/* Confirm the drop if the item is equipped. */
			if (object_is_equipped(player->body, obj)) {
				if (!verify_object("Really take off and drop", obj)) {
					/* Hack - inscribe the item with !d to prevent repeated
					 * confirmations. */
					const char *inscription = quark_str(obj->note);

					if (inscription == NULL) {
						obj->note = quark_add("!d");
					} else {
						char buffer[1024];
						my_strcpy(buffer, inscription, sizeof(buffer));
						my_strcat(buffer, "!d", sizeof(buffer));
						obj->note = quark_add(buffer);
					}

					continue;
				}
			}

			/* We're allowed to drop it. */
			if (!square_isshop(cave, player->py, player->px)) {
				cmdq_push(CMD_DROP);
				cmd_set_arg_item(cmdq_peek(), "item", obj);
				cmd_set_arg_number(cmdq_peek(), "quantity", obj->number);
			}
		}
	}

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine/reorder the pack */
	player->upkeep->notice |= (PN_COMBINE);
}

/**
 * Return the name of an ignore type.
 */
const char *ignore_name_for_type(ignore_type_t type)
{
	size_t i;

	for (i = ITYPE_NONE + 1; i < ITYPE_MAX; i++) {
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
