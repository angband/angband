/**
 * \file obj-desc.c
 * \brief Create object name descriptions
 *
 * Copyright (c) 1997 - 2007 Angband contributors
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
#include "obj-chest.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-tval.h"
#include "obj-util.h"

const char *inscrip_text[] =
{
	NULL,
	"strange",
	"average",
	"magical",
	"splendid",
	"excellent",
	"special",
	"unknown"
};

/**
 * Puts the object base kind's name into buf.
 */
void object_base_name(char *buf, size_t max, int tval, bool plural)
{
	struct object_base *kb = &kb_info[tval];
	size_t end = 0;

	if (kb->name && kb->name[0]) 
		end = obj_desc_name_format(buf, max, end, kb->name, NULL, plural);
}


/**
 * Puts a very stripped-down version of an object's name into buf.
 * If easy_know is TRUE, then the IDed names are used, otherwise
 * flavours, scroll names, etc will be used.
 *
 * Just truncates if the buffer isn't big enough.
 */
void object_kind_name(char *buf, size_t max, const struct object_kind *kind,
					  bool easy_know)
{
	/* If not aware, the plain flavour (e.g. Copper) will do. */
	if (!easy_know && !kind->aware && kind->flavor)
		my_strcpy(buf, kind->flavor->text, max);

	/* Use proper name (Healing, or whatever) */
	else
		obj_desc_name_format(buf, max, 0, kind->name, NULL, FALSE);
}


/**
 * A modifier string, put where '#' goes in the basename below.  The weird
 * games played with book names are to allow the non-essential part of the
 * name to be abbreviated when there is not much room to display.
 */
static const char *obj_desc_get_modstr(const struct object_kind *kind)
{
	if (tval_can_have_flavor_k(kind))
		return kind->flavor ? kind->flavor->text : "";

	if (tval_is_book_k(kind))
		return kind->name;

	return "";
}

/**
 * An object's basic name - a generic name for flavored objects (with the
 * actual name added later depending on awareness, the name from object.txt
 * for almost everything else, and a bit extra for books. 
 */
static const char *obj_desc_get_basename(const struct object *obj, bool aware,
										 bool terse, int mode)
{
	bool show_flavor = !terse && obj->kind->flavor;

	if (mode & ODESC_STORE) show_flavor = FALSE;
	if (aware && !OPT(show_flavors)) show_flavor = FALSE;

	/* Artifacts are special */
	if (obj->artifact && (aware || id_has(obj->id_flags, ID_ARTIFACT) ||
							terse || !obj->kind->flavor))
		return obj->kind->name;

	/* Analyze the object */
	switch (obj->tval)
	{
		case TV_FLASK:
		case TV_CHEST:
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_LIGHT:
		case TV_FOOD:
			return obj->kind->name;

		case TV_AMULET:
			return (show_flavor ? "& # Amulet~" : "& Amulet~");

		case TV_RING:
			return (show_flavor ? "& # Ring~" : "& Ring~");

		case TV_STAFF:
			return (show_flavor ? "& # Sta|ff|ves|" : "& Sta|ff|ves|");

		case TV_WAND:
			return (show_flavor ? "& # Wand~" : "& Wand~");

		case TV_ROD:
			return (show_flavor ? "& # Rod~" : "& Rod~");

		case TV_POTION:
			return (show_flavor ? "& # Potion~" : "& Potion~");

		case TV_SCROLL:
			return (show_flavor ? "& Scroll~ titled #" : "& Scroll~");

		case TV_MAGIC_BOOK:
			if (terse)
				return "& Book~ #";
			else
				return "& Book~ of Magic Spells #";

		case TV_PRAYER_BOOK:
			if (terse)
				return "& Book~ #";
			else
				return "& Holy Book~ of Prayers #";

		case TV_MUSHROOM:
			return (show_flavor ? "& # Mushroom~" : "& Mushroom~");
	}

	return "(nothing)";
}


/**
 * Start to description, indicating number/uniqueness (a, the, no more, 7, etc)
 */
static size_t obj_desc_name_prefix(char *buf, size_t max, size_t end,
		const struct object *obj, bool known, const char *basename,
		const char *modstr, bool terse)
{
	if (obj->number == 0)
		strnfcat(buf, max, &end, "no more ");
	else if (obj->number > 1)
		strnfcat(buf, max, &end, "%d ", obj->number);
	else if ((object_name_is_visible(obj) || known) && obj->artifact)
		strnfcat(buf, max, &end, "the ");

	else if (*basename == '&')
	{
		bool an = FALSE;
		const char *lookahead = basename + 1;

		while (*lookahead == ' ') lookahead++;

		if (*lookahead == '#')
		{
			if (modstr && is_a_vowel(*modstr))
				an = TRUE;
		}
		else if (is_a_vowel(*lookahead))
		{
			an = TRUE;
		}

		if (!terse)
		{
			if (an)
				strnfcat(buf, max, &end, "an ");
			else
				strnfcat(buf, max, &end, "a ");			
		}
	}

	return end;
}



/**
 * Formats 'fmt' into 'buf', with the following formatting characters:
 *
 * '~' at the end of a word (e.g. "fridge~") will pluralise
 *
 * '|x|y|' will be output as 'x' if singular or 'y' if plural
 *    (e.g. "kni|fe|ves|")
 *
 * '#' will be replaced with 'modstr' (which may contain the pluralising
 * formats given above).
 */
size_t obj_desc_name_format(char *buf, size_t max, size_t end,
		const char *fmt, const char *modstr, bool pluralise)
{
	/* Copy the string */
	while (*fmt)
	{
		if (*fmt == '&')
		{
			while (*fmt == ' ' || *fmt == '&')
				fmt++;
			continue;
		}

		/* Pluralizer (regular English plurals) */
		else if (*fmt == '~')
		{
			char prev = *(fmt - 1);

			if (!pluralise)
			{
				fmt++;
				continue;
			}

			/* e.g. cutlass-e-s, torch-e-s, box-e-s */
			if (prev == 's' || prev == 'h' || prev == 'x')
				strnfcat(buf, max, &end, "es");
			else
				strnfcat(buf, max, &end, "s");
		}

		/* Special plurals */
		else if (*fmt == '|')
		{
			/* e.g. kni|fe|ves|
			 *          ^  ^  ^ */
			const char *singular = fmt + 1;
			const char *plural   = strchr(singular, '|');
			const char *endmark  = NULL;

			if (plural)
			{
				plural++;
				endmark = strchr(plural, '|');
			}

			if (!singular || !plural || !endmark) return end;

			if (!pluralise)
				strnfcat(buf, max, &end, "%.*s", plural - singular - 1,
						 singular);
			else
				strnfcat(buf, max, &end, "%.*s", endmark - plural, plural);

			fmt = endmark;
		}

		/* Add modstr, with pluralisation if relevant */
		else if (*fmt == '#')
		{
			end = obj_desc_name_format(buf, max, end, modstr, NULL,
					pluralise);
		}

		else
			buf[end++] = *fmt;

		fmt++;
	}

	buf[end] = 0;

	return end;
}


/**
 * Format object obj's name into 'buf'.
 */
static size_t obj_desc_name(char *buf, size_t max, size_t end,
		const struct object *obj, bool prefix, int mode, bool spoil, bool terse)
{
	bool known = object_is_known(obj) || spoil;
	bool aware = object_flavor_is_aware(obj) || (mode & ODESC_STORE) || spoil;
	const char *basename = obj_desc_get_basename(obj, aware, terse, mode);
	const char *modstr = obj_desc_get_modstr(obj->kind);

	if (aware && !obj->kind->everseen && !spoil)
		obj->kind->everseen = TRUE;

	if (prefix)
		end = obj_desc_name_prefix(buf, max, end, obj, known,
				basename, modstr, terse);

	/* Pluralize if (not forced singular) and
	 * (not a known/visible artifact) and
	 * (not one in stack or forced plural) */
	end = obj_desc_name_format(buf, max, end, basename, modstr,
			!(mode & ODESC_SINGULAR) &&
			!(obj->artifact &&
			  (object_name_is_visible(obj) || known)) &&
			(obj->number != 1 || (mode & ODESC_PLURAL)));

	/** Append extra names of various kinds **/

	if ((object_name_is_visible(obj) || known) && obj->artifact)
		strnfcat(buf, max, &end, " %s", obj->artifact->name);

	else if (((spoil && obj->ego) || object_ego_is_visible(obj)) &&
			 !(mode & ODESC_NOEGO))
		strnfcat(buf, max, &end, " %s", obj->ego->name);

	else if (aware && !obj->artifact &&
			(obj->kind->flavor || obj->kind->tval == TV_SCROLL)) {
		if (terse)
			strnfcat(buf, max, &end, " '%s'", obj->kind->name);
		else
			strnfcat(buf, max, &end, " of %s", obj->kind->name);
	}

	return end;
}

/**
 * Is obj armor?
 */
static bool obj_desc_show_armor(const struct object *obj)
{
	if (obj->ac || tval_is_armor(obj)) return TRUE;

	return FALSE;
}

/**
 * Special descriptions for types of chest traps
 */
static size_t obj_desc_chest(const struct object *obj, char *buf, size_t max,
							 size_t end)
{
	bool known = object_is_known(obj);

	if (!tval_is_chest(obj)) return end;
	if (!known) return end;

	/* May be "empty" */
	if (!obj->pval)
		strnfcat(buf, max, &end, " (empty)");

	/* May be "disarmed" */
	else if (!is_locked_chest(obj))
	{
		if (chest_trap_type(obj) != 0)
			strnfcat(buf, max, &end, " (disarmed)");
		else
			strnfcat(buf, max, &end, " (unlocked)");
	}

	/* Describe the traps, if any */
	else
	{
		/* Describe the traps */
		switch (chest_trap_type(obj))
		{
			case 0:
				strnfcat(buf, max, &end, " (Locked)");
				break;

			case CHEST_LOSE_STR:
				strnfcat(buf, max, &end, " (Poison Needle)");
				break;

			case CHEST_LOSE_CON:
				strnfcat(buf, max, &end, " (Poison Needle)");
				break;

			case CHEST_POISON:
				strnfcat(buf, max, &end, " (Gas Trap)");
				break;

			case CHEST_PARALYZE:
				strnfcat(buf, max, &end, " (Gas Trap)");
				break;

			case CHEST_EXPLODE:
				strnfcat(buf, max, &end, " (Explosion Device)");
				break;

			case CHEST_SUMMON:
				strnfcat(buf, max, &end, " (Summoning Runes)");
				break;

			default:
				strnfcat(buf, max, &end, " (Multiple Traps)");
				break;
		}
	}

	return end;
}

/**
 * Describe combat properties of an item - damage dice, to-hit, to-dam, armor
 * class, missile multipler
 */
static size_t obj_desc_combat(const struct object *obj, char *buf, size_t max, 
		size_t end, bool spoil)
{
	bitflag flags_known[OF_SIZE];

	object_flags_known(obj, flags_known);

	if (kf_has(obj->kind->kind_flags, KF_SHOW_DICE)) {
		/* Only display the real damage dice if the combat stats are known */
		if (spoil || object_attack_plusses_are_visible(obj))
			strnfcat(buf, max, &end, " (%dd%d)", obj->dd, obj->ds);
		else
			strnfcat(buf, max, &end, " (%dd%d)", obj->kind->dd,
					 obj->kind->ds);
	}

	if (kf_has(obj->kind->kind_flags, KF_SHOW_MULT)) {
		/* Display shooting power as part of the multiplier */
		if ((obj->modifiers[OBJ_MOD_MIGHT] > 0) &&
		    (spoil || object_this_mod_is_visible(obj, OBJ_MOD_MIGHT)))
			strnfcat(buf, max, &end, " (x%d)",
					 obj->pval + obj->modifiers[OBJ_MOD_MIGHT]);
		else
			strnfcat(buf, max, &end, " (x%d)", obj->pval);
	}

	/* Show weapon bonuses */
	if (spoil || object_attack_plusses_are_visible(obj)) {
		if (tval_is_weapon(obj) || obj->to_d || obj->to_h) {
			/* Make an exception for body armor with only a to-hit penalty */
			if (obj->to_h < 0 && obj->to_d == 0 &&
				tval_is_body_armor(obj))
				strnfcat(buf, max, &end, " (%+d)", obj->to_h);

			/* Otherwise, always use the full tuple */
			else
				strnfcat(buf, max, &end, " (%+d,%+d)", obj->to_h,
						 obj->to_d);
		}
	}


	/* Show armor bonuses */
	if (spoil || object_defence_plusses_are_visible(obj)) {
		if (obj_desc_show_armor(obj))
			strnfcat(buf, max, &end, " [%d,%+d]", obj->ac, obj->to_a);
		else if (obj->to_a)
			strnfcat(buf, max, &end, " [%+d]", obj->to_a);
	}
	else if (obj_desc_show_armor(obj))
		strnfcat(buf, max, &end, " [%d]",
				 object_was_sensed(obj) ? obj->ac : obj->kind->ac);

	return end;
}

/**
 * Describe remaining light for refuellable lights
 */
static size_t obj_desc_light(const struct object *obj, char *buf, size_t max,
							 size_t end)
{
	/* Fuelled light sources get number of remaining turns appended */
	if (tval_is_light(obj) && !of_has(obj->flags, OF_NO_FUEL))
		strnfcat(buf, max, &end, " (%d turns)", obj->timeout);

	return end;
}

/**
 * Describe numerical modifiers to stats and other player qualities which
 * allow numerical bonuses - speed, stealth, etc
 */
static size_t obj_desc_mods(const struct object *obj, char *buf, size_t max,
	size_t end, bool spoil)
{
	int i, j, num_mods = 0;
	int mods[OBJ_MOD_MAX] = { 0 };

	/* Run through possible modifiers and store distinct ones */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		/* Check for known non-zero mods */
		if ((spoil || object_this_mod_is_visible(obj, i))
			&& (obj->modifiers[i] != 0)) {
			/* If no mods stored yet, store and move on */
			if (!num_mods) {
				mods[num_mods++] = obj->modifiers[i];
				continue;
			}

			/* Run through the existing mods, quit on duplicates */
			for (j = 0; j < num_mods; j++)
				if (mods[j] == obj->modifiers[i]) break;

			/* Add another mod if needed */
			if (j == num_mods)
				mods[num_mods++] = obj->modifiers[i];
		}
	}

	if (!num_mods) return end;

	/* Print the modifiers */
	strnfcat(buf, max, &end, " <");
	for (j = 0; j < num_mods; j++) {
		if (j) strnfcat(buf, max, &end, ", ");
		strnfcat(buf, max, &end, "%+d", mods[j]);
	}
	strnfcat(buf, max, &end, ">");

	return end;
}

/**
 * Describe charges or charging status for re-usable items with magic effects
 */
static size_t obj_desc_charges(const struct object *obj, char *buf, size_t max,
							   size_t end, int mode)
{
	bool aware = object_flavor_is_aware(obj) || (mode & ODESC_STORE);

	/* Wands and Staffs have charges */
	if (aware && tval_can_have_charges(obj))
		strnfcat(buf, max, &end, " (%d charge%s)", obj->pval,
				 PLURAL(obj->pval));

	/* Charging things */
	else if (obj->timeout > 0)
	{
		if (tval_is_rod(obj) && obj->number > 1)
		{
			strnfcat(buf, max, &end, " (%d charging)", number_charging(obj));
		}
		/* Artifacts, single rods */
		else if (!(tval_is_light(obj) && !obj->artifact))
		{
			strnfcat(buf, max, &end, " (charging)");
		}
	}

	return end;
}

/**
 * Add player-defined inscriptions or game-defined descriptions
 */
static size_t obj_desc_inscrip(const struct object *obj, char *buf,
							   size_t max, size_t end)
{
	const char *u[4] = { 0, 0, 0, 0 };
	int n = 0;
	int feel = object_pseudo(obj);
	bitflag flags_known[OF_SIZE], f2[OF_SIZE];

	object_flags_known(obj, flags_known);

	/* Get inscription */
	if (obj->note)
		u[n++] = quark_str(obj->note);

	/* Use special inscription, if any */
	if (!object_is_known(obj)) {
		if (feel) {
			/* cannot tell excellent vs strange vs splendid until wield */
			if (!object_was_worn(obj) && obj->ego)
				u[n++] = "ego";
			else
				u[n++] = inscrip_text[feel];
		} 
		else if (tval_can_have_charges(obj) && (obj->pval == 0))
			u[n++] = "empty";
		else if (object_was_worn(obj))
			u[n++] = (tval_is_weapon(obj)) ? "wielded" : "worn";
		else if (!object_flavor_is_aware(obj) &&
				 object_flavor_was_tried(obj))
			u[n++] = "tried";
	}

	/* Note curses */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);
	if (of_is_inter(flags_known, f2))
		u[n++] = "cursed";

	/* Note ignore */
	if (ignore_item_ok(obj))
		u[n++] = "ignore";

	if (n)
	{
		int i;
		for (i = 0; i < n; i++)
		{
			if (i == 0)
				strnfcat(buf, max, &end, " {");
			strnfcat(buf, max, &end, "%s", u[i]);
			if (i < n-1)
				strnfcat(buf, max, &end, ", ");
		}

		strnfcat(buf, max, &end, "}");
	}

	return end;
}


/**
 * Add "unseen" to the end of unaware items in stores
 */
static size_t obj_desc_aware(const struct object *obj, char *buf, size_t max,
							 size_t end)
{
	if (!object_flavor_is_aware(obj))
		strnfcat(buf, max, &end, " {unseen}");

	return end;
}


/**
 * Describes item `obj` into buffer `buf` of size `max`.
 *
 * ODESC_PREFIX prepends a 'the', 'a' or number
 * ODESC_BASE results in a base description.
 * ODESC_COMBAT will add to-hit, to-dam and AC info.
 * ODESC_EXTRA will add pval/charge/inscription/ignore info.
 * ODESC_PLURAL will pluralise regardless of the number in the stack.
 * ODESC_STORE turns off ignore markers, for in-store display.
 * ODESC_SPOIL treats the object as fully identified.
 *
 * Setting 'prefix' to TRUE prepends a 'the', 'a' or the number in the stack,
 * respectively.
 *
 * \returns The number of bytes used of the buffer.
 */
size_t object_desc(char *buf, size_t max, const struct object *obj, int mode)
{
	bool prefix = mode & ODESC_PREFIX ? TRUE : FALSE;
	bool spoil = mode & ODESC_SPOIL ? TRUE : FALSE;
	bool terse = mode & ODESC_TERSE ? TRUE : FALSE;

	size_t end = 0;

	/* Simple description for null item */
	if (!obj)
		return strnfmt(buf, max, "(nothing)");

	/* Egos whose name we know are seen */
	if (object_name_is_visible(obj) && obj->ego && !spoil)
		obj->ego->everseen = TRUE;


	/*** Some things get really simple descriptions ***/

	if (obj->marked == MARK_AWARE) {
		if (prefix)
			return strnfmt(buf, max, "an unknown item");
		return strnfmt(buf, max, "unknown item");
	}

	if (tval_is_money(obj))
		return strnfmt(buf, max, "%d gold pieces worth of %s%s",
				obj->pval, obj->kind->name,
				ignore_item_ok(obj) ? " {ignore}" : "");

	/** Construct the name **/

	/* Copy the base name to the buffer */
	end = obj_desc_name(buf, max, end, obj, prefix, mode, spoil, terse);

	if (mode & ODESC_COMBAT)
	{
		if (tval_is_chest(obj))
			end = obj_desc_chest(obj, buf, max, end);
		else if (tval_is_light(obj))
			end = obj_desc_light(obj, buf, max, end);

		end = obj_desc_combat(obj, buf, max, end, spoil);
	}

	if (mode & ODESC_EXTRA)
	{
		end = obj_desc_mods(obj, buf, max, end, spoil);

		end = obj_desc_charges(obj, buf, max, end, mode);

		if (mode & ODESC_STORE)
			end = obj_desc_aware(obj, buf, max, end);
		else
			end = obj_desc_inscrip(obj, buf, max, end);
	}

	return end;
}
