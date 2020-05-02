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
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
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
	{ ITYPE_GREAT,					TV_SWORD,		"Chaos" },
	{ ITYPE_GREAT,					TV_POLEARM,		"Slicing" },
	{ ITYPE_GREAT,					TV_HAFTED,		"Disruption" },
	{ ITYPE_SHARP,					TV_SWORD,		"" },
	{ ITYPE_SHARP,					TV_POLEARM,		"" },
	{ ITYPE_BLUNT,					TV_HAFTED,		"" },
	{ ITYPE_SLING,					TV_BOW,			"Sling" },
	{ ITYPE_BOW,					TV_BOW,			"Bow" },
	{ ITYPE_CROSSBOW,				TV_BOW,			"Crossbow" },
	{ ITYPE_SHOT,					TV_SHOT,		"" },
	{ ITYPE_ARROW,					TV_ARROW,		"" },
	{ ITYPE_BOLT,					TV_BOLT,		"" },
	{ ITYPE_ROBE,					TV_SOFT_ARMOR,	"Robe" },
	{ ITYPE_BASIC_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Black" },
	{ ITYPE_BASIC_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Blue" },
	{ ITYPE_BASIC_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"White" },
	{ ITYPE_BASIC_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Red" },
	{ ITYPE_BASIC_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Green" },
	{ ITYPE_MULTI_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Multi" },
	{ ITYPE_HIGH_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Shining" },
	{ ITYPE_HIGH_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Law" },
	{ ITYPE_HIGH_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Gold" },
	{ ITYPE_HIGH_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Chaos" },
	{ ITYPE_BALANCE_DRAGON_ARMOR,	TV_DRAG_ARMOR,	"Balance" },
	{ ITYPE_POWER_DRAGON_ARMOR,		TV_DRAG_ARMOR,	"Power" },
	{ ITYPE_BODY_ARMOR,				TV_HARD_ARMOR,	"" },
	{ ITYPE_BODY_ARMOR,				TV_SOFT_ARMOR,	"" },
	{ ITYPE_ELVEN_CLOAK,			TV_CLOAK,		"Elven" },
	{ ITYPE_CLOAK,					TV_CLOAK,		"" },
	{ ITYPE_SHIELD,					TV_SHIELD,		"" },
	{ ITYPE_HEADGEAR,				TV_HELM,		"" },
	{ ITYPE_HEADGEAR,				TV_CROWN,		"" },
	{ ITYPE_HANDGEAR,				TV_GLOVES,		"" },
	{ ITYPE_FEET,					TV_BOOTS,		"" },
	{ ITYPE_DIGGER,					TV_DIGGING,		"" },
	{ ITYPE_RING,					TV_RING,		"" },
	{ ITYPE_AMULET,					TV_AMULET,		"" },
	{ ITYPE_LIGHT, 					TV_LIGHT, 		"" },
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
	{ IGNORE_ALL,				"non-artifact" },
};

byte ignore_level[ITYPE_MAX];
const size_t ignore_size = ITYPE_MAX;
bool **ego_ignore_types;
/* Hackish - ego_ignore_types should be initialised with arrays */
static int num_ego_types;


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
		k_info[i].ignore = false;

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
 * Make or extend a rune autoinscription
 */
static void rune_add_autoinscription(struct object *obj, int i)
{
	char current_note[80] = "";

	/* No autoinscription, or already there, don't bother */
	if (!rune_note(i)) return;
	if (obj->note && strstr(quark_str(obj->note), quark_str(rune_note(i))))
		return;

	/* Extend any current note */
	if (obj->note)
		my_strcpy(current_note, quark_str(obj->note), sizeof(current_note));
	my_strcat(current_note, quark_str(rune_note(i)), sizeof(current_note));

	/* Add the inscription */
	obj->note = quark_add(current_note);
}

/**
 * Put a rune autoinscription on all available objects
 */
void rune_autoinscribe(int i)
{
	struct object *obj;

	/* Check the player knows the rune */
	if (!player_knows_rune(player, i)) {
		return;
	}

	/* Autoinscribe each object on the ground */
	if (cave)
		for (obj = square_object(cave, player->grid); obj; obj = obj->next)
			if (object_has_rune(obj, i))
				rune_add_autoinscription(obj, i);

	/* Autoinscribe each object in the inventory */
	for (obj = player->gear; obj; obj = obj->next)
		if (object_has_rune(obj, i))
			rune_add_autoinscription(obj, i);
}

/**
 * Put all appropriate rune autoinscriptions on an object
 */
static void runes_autoinscribe(struct object *obj)
{
	int i, rune_max = max_runes();

	for (i = 0; i < rune_max; i++)
		if (object_has_rune(obj, i) && player_knows_rune(player, i))
			rune_add_autoinscription(obj, i);
}

/**
 * Return an object kind autoinscription
 */
const char *get_autoinscription(struct object_kind *kind, bool aware)
{
	if (!kind)
		return NULL;
	else if (aware)
		return quark_str(kind->note_aware);
	else 
		return quark_str(kind->note_unaware);
}

/**
 * Put an autoinscription on an object
 */
int apply_autoinscription(struct object *obj)
{
	char o_name[80];
	bool aware = obj->kind->aware;
	const char *note = obj ? get_autoinscription(obj->kind, aware) : NULL;

	/* Remove unaware inscription if aware */
	if (aware && quark_str(obj->note) && quark_str(obj->kind->note_unaware) &&
		streq(quark_str(obj->note), quark_str(obj->kind->note_unaware)))
		obj->note = 0;

	/* Make rune autoinscription go first, for now */
	runes_autoinscribe(obj);

	/* No note - don't inscribe */
	if (!note)
		return 0;

	/* Don't re-inscribe if it's already inscribed */
	if (obj->note)
		return 0;

	/* Don't inscribe unless the player is carrying it */
	if (!object_is_carried(player, obj))
		return 0;

	/* Don't inscribe if ignored */
	if (ignore_item_ok(obj))
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
	if (!k)
		return 0;

	/* Unaware */
	if (!k->aware) {
		if (!k->note_unaware) {
			return 0;
		} else {
			k->note_unaware = 0;
			return 1;
		}
	}

	/* Aware */
	if (!k->note_aware)
		return 0;

	k->note_aware = 0;
	return 1;
}


/**
 * Register an object kind autoinscription
 */
int add_autoinscription(s16b kind, const char *inscription, bool aware)
{
	struct object_kind *k = objkind_byid(kind);
	if (!k)
		return 0;
	if (!inscription)
		return remove_autoinscription(kind);
	if (aware)
		k->note_aware = quark_add(inscription);
	else
		k->note_unaware = quark_add(inscription);
	return 1;
}


/**
 * Put an autoinscription on all objects on the floor beneath the player
 */
void autoinscribe_ground(void)
{
	struct object *obj;

	/* Autoinscribe each object in the pile */
	for (obj = square_object(cave, player->grid); obj; obj = obj->next)
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
 * Find whether an ignore type is valid for a given ego item
 */
bool ego_has_ignore_type(struct ego_item *ego, ignore_type_t itype)
{
	struct poss_item *poss;

	/* Go through all the possible items */
	for (poss = ego->poss_items; poss; poss = poss->next) {
		size_t i;
		struct object_kind *kind = &k_info[poss->kidx];

		/* Check the appropriate ignore group */
		for (i = 0; i < N_ELEMENTS(quality_mapping); i++)
			if ((quality_mapping[i].tval == kind->tval) &&
				(quality_mapping[i].ignore_type == itype) &&
				strstr(kind->name, quality_mapping[i].identifier))
				return true;
	}

	return false;
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
 * Determine the ignore level of an object
 *
 * The main point is when the value is undetermined given current info,
 * return the maximum possible value.
 */
byte ignore_level_of(const struct object *obj)
{
	byte value = 0;
	int i;

	if (!obj->known) return IGNORE_MAX;

	/* Deal with jewelry specially - only bad or average */
	if (tval_is_jewelry(obj)) {
		/* One positive modifier means not bad*/
		for (i = 0; i < OBJ_MOD_MAX; i++)
			if (obj->known->modifiers[i] > 0)
				return IGNORE_AVERAGE;

		/* One positive combat value means not bad, one negative means bad */
		if ((obj->known->to_h > 0) || (obj->known->to_d > 0) ||
			(obj->known->to_a > 0))
			return IGNORE_AVERAGE;
		if ((obj->known->to_h < 0) || (obj->known->to_d < 0) ||
			(obj->known->to_a < 0))
			return IGNORE_BAD;

		return IGNORE_AVERAGE;
	}

	/* Now just do bad, average, good, ego */
	if (object_fully_known(obj)) {
		int isgood = is_object_good(obj);

		/* Values for items not egos or artifacts, may be updated */
		if (isgood > 0) {
			value = IGNORE_GOOD;
		} else if (isgood < 0) {
			value = IGNORE_BAD;
		} else {
			value = IGNORE_AVERAGE;
		}

		if (obj->ego)
			value = IGNORE_ALL;
		else if (obj->artifact)
			value = IGNORE_MAX;
	} else {
		if ((obj->known->notice & OBJ_NOTICE_ASSESSED) && !obj->artifact)
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
	ego_ignore_types[obj->ego->eidx][ignore_type_of(obj)] = true;
	player->upkeep->notice |= PN_IGNORE;
}

void ego_ignore_clear(struct object *obj)
{
	assert(obj->ego);
	ego_ignore_types[obj->ego->eidx][ignore_type_of(obj)] = false;
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
	return (kind->ignore & IGNORE_IF_AWARE) ? true : false;
}

bool kind_is_ignored_unaware(const struct object_kind *kind)
{
	return (kind->ignore & IGNORE_IF_UNAWARE) ? true : false;
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

	/* Objects that aren't yet known can't be ignored */
	if (!obj->known)
		return false;

	/* Objects with an unknown rune shouldn't be ignored */
	if (tval_has_variable_power(obj) && !object_runes_known(obj))
		return false;

	/* Do ignore individual objects that marked ignore */
	if (obj->known->notice & OBJ_NOTICE_IGNORE)
		return true;

	/* Don't ignore artifacts unless marked to be ignored */
	if (obj->artifact ||
		check_for_inscrip(obj, "!k") || check_for_inscrip(obj, "!*"))
		return false;

	/* Do ignoring by kind */
	if (object_flavor_is_aware(obj) ?
		 kind_is_ignored_aware(obj->kind) :
		 kind_is_ignored_unaware(obj->kind))
		return true;

	/* Ignore ego items if known */
	if (obj->known->ego && ego_is_ignored(obj->ego->eidx, ignore_type_of(obj)))
		return true;

	type = ignore_type_of(obj);
	if (type == ITYPE_MAX)
		return false;

	/* Ignore items known not to be artifacts */
	if ((obj->known->notice & OBJ_NOTICE_ASSESSED) && !obj->artifact &&
		ignore_level[type] == IGNORE_ALL)
		return true;

	/* Get result based on the feeling and the ignore_level */
	if (ignore_level_of(obj) <= ignore_level[type])
		return true;
	else
		return false;
}

/**
 * Determines if an object is eligible for ignoring.
 */
bool ignore_item_ok(const struct object *obj)
{
	if (player->unignoring)
		return false;

	return object_is_ignored(obj);
}

/**
 * Determines if the known version of an object is eligible for ignoring.
 *
 * This function should only be called on known version of items which have a
 * (real or imaginary) listed base item in the current level
 */
bool ignore_known_item_ok(const struct object *obj)
{
	struct object *base_obj = cave->objects[obj->oidx];

	if (player->unignoring)
		return false;

	/* Get the real object and check its ignore properties */
	assert(base_obj);
	return object_is_ignored(base_obj);
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
			if (!square_isshop(cave, player->grid)) {
				player->upkeep->dropping = true;
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
