/*
 * File: obj-desc.c
 * Purpose: Create object name descriptions
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
#include "squelch.h"
#include "object/tvalsval.h"
#include "object/pval.h"

static size_t obj_desc_name_format(char *buf, size_t max, size_t end,
		const char *fmt, const char *modstr, bool pluralise);

/**
 * Puts the object base kind's name into buf.
 */
void object_base_name(char *buf, size_t max, int tval, bool plural)
{
	object_base *kb = &kb_info[tval];
	size_t end = 0;

	end = obj_desc_name_format(buf, max, end, kb->name, NULL, plural);
}


/*
 * Puts a very stripped-down version of an object's name into buf.
 * If easy_know is TRUE, then the IDed names are used, otherwise
 * flavours, scroll names, etc will be used.
 *
 * Just truncates if the buffer isn't big enough.
 */
void object_kind_name(char *buf, size_t max, const object_kind *kind, bool easy_know)
{
	/* If not aware, use flavor */
	if (!easy_know && !kind->aware && kind->flavor)
	{
		if (kind->tval == TV_FOOD && kind->sval > SV_FOOD_MIN_SHROOM)
		{
			strnfmt(buf, max, "%s Mushroom", kind->flavor->text);
		}
		else
		{
			/* Plain flavour (e.g. Copper) will do. */
			my_strcpy(buf, kind->flavor->text, max);
		}
	}

	/* Use proper name (Healing, or whatever) */
	else
	{
		char *t;

		if (kind->tval == TV_FOOD && kind->sval > SV_FOOD_MIN_SHROOM)
		{
			my_strcpy(buf, "Mushroom of ", max);
			max -= strlen(buf);
			t = buf + strlen(buf);
		}
		else
		{
			t = buf;
		}

		/* Format remainder of the string */
		obj_desc_name_format(t, max, 0, kind->name, NULL, FALSE);
	}
}



static const char *obj_desc_get_modstr(const object_kind *kind)
{
	switch (kind->tval)
	{
		case TV_AMULET:
		case TV_RING:
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
		case TV_POTION:
		case TV_FOOD:
		case TV_SCROLL:
			return kind->flavor ? kind->flavor->text : "";

		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
			return kind->name;
	}

	return "";
}

static const char *obj_desc_get_basename(const object_type *o_ptr, bool aware)
{
	bool show_flavor = o_ptr->kind->flavor ? TRUE : FALSE;


	if (o_ptr->ident & IDENT_STORE) show_flavor = FALSE;
	if (aware && !OPT(show_flavors)) show_flavor = FALSE;



	/* Known artifacts get special treatment */
	if (o_ptr->artifact && aware)
		return o_ptr->kind->name;

	/* Analyze the object */
	switch (o_ptr->tval)
	{
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		case TV_SPIKE:
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
			return o_ptr->kind->name;

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
			return "& Book~ of Magic Spells #";

		case TV_PRAYER_BOOK:
			return "& Holy Book~ of Prayers #";

		case TV_FOOD:
			if (o_ptr->sval > SV_FOOD_MIN_SHROOM)
				return (show_flavor ? "& # Mushroom~" : "& Mushroom~");
			else
				return o_ptr->kind->name;
	}

	return "(nothing)";
}


static size_t obj_desc_name_prefix(char *buf, size_t max, size_t end,
		const object_type *o_ptr, bool known, const char *basename,
		const char *modstr)
{
	if (o_ptr->number <= 0)
		strnfcat(buf, max, &end, "no more ");
	else if (o_ptr->number > 1)
		strnfcat(buf, max, &end, "%d ", o_ptr->number);
	else if ((object_name_is_visible(o_ptr) || known) && o_ptr->artifact)
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

		if (an)
			strnfcat(buf, max, &end, "an ");
		else
			strnfcat(buf, max, &end, "a ");
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
static size_t obj_desc_name_format(char *buf, size_t max, size_t end,
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
				strnfcat(buf, max, &end, "%.*s", plural - singular - 1, singular);
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


/*
 * Format object o_ptr's name into 'buf'.
 */
static size_t obj_desc_name(char *buf, size_t max, size_t end,
		const object_type *o_ptr, bool prefix, odesc_detail_t mode,
		bool spoil)
{
	bool known = object_is_known(o_ptr) || (o_ptr->ident & IDENT_STORE) || spoil;
	bool aware = object_flavor_is_aware(o_ptr) || (o_ptr->ident & IDENT_STORE) || spoil;

	const char *basename = obj_desc_get_basename(o_ptr, aware);
	const char *modstr = obj_desc_get_modstr(o_ptr->kind);

	if (aware && !o_ptr->kind->everseen)
		o_ptr->kind->everseen = TRUE;

	if (prefix)
		end = obj_desc_name_prefix(buf, max, end, o_ptr, known,
				basename, modstr);

	/* Pluralize if (not forced singular) and 
	 * (not a known/visible artifact) and 
	 * (not one in stack or forced plural) */
	end = obj_desc_name_format(buf, max, end, basename, modstr,
			!(mode & ODESC_SINGULAR) &&
			!(o_ptr->artifact &&
			  (object_name_is_visible(o_ptr) || known)) &&
			(o_ptr->number != 1 || (mode & ODESC_PLURAL)));


	/** Append extra names of various kinds **/

	if ((object_name_is_visible(o_ptr) || known) && o_ptr->artifact)
		strnfcat(buf, max, &end, " %s", o_ptr->artifact->name);

	else if ((spoil && o_ptr->ego) || object_ego_is_visible(o_ptr))
		strnfcat(buf, max, &end, " %s", o_ptr->ego->name);

	else if (aware && !o_ptr->artifact &&
			(o_ptr->kind->flavor || o_ptr->kind->tval == TV_SCROLL))
		strnfcat(buf, max, &end, " of %s", o_ptr->kind->name);

	return end;
}

/*
 * Is o_ptr armor?
 */
static bool obj_desc_show_armor(const object_type *o_ptr)
{
	if (o_ptr->ac) return TRUE;

	switch (o_ptr->tval)
	{
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			return TRUE;
			break;
		}
	}

	return FALSE;
}

static size_t obj_desc_chest(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	bool known = object_is_known(o_ptr) || (o_ptr->ident & IDENT_STORE);

	if (o_ptr->tval != TV_CHEST) return end;
	if (!known) return end;

	/* May be "empty" */
	if (!o_ptr->pval[DEFAULT_PVAL])
		strnfcat(buf, max, &end, " (empty)");

	/* May be "disarmed" */
	else if (o_ptr->pval[DEFAULT_PVAL] < 0)
	{
		if (chest_traps[0 - o_ptr->pval[DEFAULT_PVAL]])
			strnfcat(buf, max, &end, " (disarmed)");
		else
			strnfcat(buf, max, &end, " (unlocked)");
	}

	/* Describe the traps, if any */
	else
	{
		/* Describe the traps */
		switch (chest_traps[o_ptr->pval[DEFAULT_PVAL]])
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

static size_t obj_desc_combat(const object_type *o_ptr, char *buf, size_t max, 
		size_t end, bool spoil)
{
	bitflag flags[OF_SIZE];
	bitflag flags_known[OF_SIZE];

	object_flags(o_ptr, flags);
	object_flags_known(o_ptr, flags_known);

	if (of_has(flags, OF_SHOW_DICE))
	{
		/* Only display the real damage dice if the combat stats are known */
		if (spoil || object_attack_plusses_are_visible(o_ptr))
			strnfcat(buf, max, &end, " (%dd%d)", o_ptr->dd, o_ptr->ds);
		else
			strnfcat(buf, max, &end, " (%dd%d)", o_ptr->kind->dd, o_ptr->kind->ds);
	}

	if (of_has(flags, OF_SHOW_MULT))
	{
		/* Display shooting power as part of the multiplier */
		if (of_has(flags, OF_MIGHT) &&
		    (spoil || object_flag_is_known(o_ptr, OF_MIGHT)))
			strnfcat(buf, max, &end, " (x%d)", (o_ptr->sval % 10) + o_ptr->pval[which_pval(o_ptr, OF_MIGHT)]);
		else
			strnfcat(buf, max, &end, " (x%d)", o_ptr->sval % 10);
	}

	/* Show weapon bonuses */
	if (spoil || object_attack_plusses_are_visible(o_ptr))
	{
		if (of_has(flags, OF_SHOW_MODS) || o_ptr->to_d || o_ptr->to_h)
		{
			/* Make an exception for body armor with only a to-hit penalty */
			if (o_ptr->to_h < 0 && o_ptr->to_d == 0 &&
			    (o_ptr->tval == TV_SOFT_ARMOR ||
			     o_ptr->tval == TV_HARD_ARMOR ||
			     o_ptr->tval == TV_DRAG_ARMOR))
				strnfcat(buf, max, &end, " (%+d)", o_ptr->to_h);

			/* Otherwise, always use the full tuple */
			else
				strnfcat(buf, max, &end, " (%+d,%+d)", o_ptr->to_h, o_ptr->to_d);
		}
	}


	/* Show armor bonuses */
	if (spoil || object_defence_plusses_are_visible(o_ptr))
	{
		if (obj_desc_show_armor(o_ptr))
			strnfcat(buf, max, &end, " [%d,%+d]", o_ptr->ac, o_ptr->to_a);
		else if (o_ptr->to_a)
			strnfcat(buf, max, &end, " [%+d]", o_ptr->to_a);
	}
	else if (obj_desc_show_armor(o_ptr))
	{
		strnfcat(buf, max, &end, " [%d]", object_was_sensed(o_ptr) ? o_ptr->ac : o_ptr->kind->ac);
	}

	return end;
}

static size_t obj_desc_light(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	bitflag f[OF_SIZE];

	object_flags(o_ptr, f);

	/* Fuelled light sources get number of remaining turns appended */
	if ((o_ptr->tval == TV_LIGHT) && !of_has(f, OF_NO_FUEL))
		strnfcat(buf, max, &end, " (%d turns)", o_ptr->timeout);

	return end;
}

static size_t obj_desc_pval(const object_type *o_ptr, char *buf, size_t max,
	size_t end, bool spoil)
{
	bitflag f[OF_SIZE], f2[OF_SIZE];
	int i;

	object_flags(o_ptr, f);
	create_mask(f2, FALSE, OFT_PVAL, OFT_STAT, OFT_MAX);

	if (!of_is_inter(f, f2)) return end;

	strnfcat(buf, max, &end, " <");
	for (i = 0; i < o_ptr->num_pvals; i++) {
		if (spoil || object_this_pval_is_visible(o_ptr, i)) {
			if (i > 0)
				strnfcat(buf, max, &end, ", ");
			strnfcat(buf, max, &end, "%+d", o_ptr->pval[i]);
		}
	}

	if ((o_ptr->num_pvals == 1) && !of_has(f, OF_HIDE_TYPE))
	{
		if (of_has(f, OF_STEALTH))
			strnfcat(buf, max, &end, " stealth");
		else if (of_has(f, OF_SEARCH))
			strnfcat(buf, max, &end, " searching");
		else if (of_has(f, OF_INFRA))
			strnfcat(buf, max, &end, " infravision");
		else if (of_has(f, OF_SPEED))
			strnfcat(buf, max, &end, " speed");
		else if (of_has(f, OF_BLOWS))
			strnfcat(buf, max, &end, " attack%s", PLURAL(o_ptr->pval[which_pval(o_ptr, OF_BLOWS)]));
	}

	strnfcat(buf, max, &end, ">");

	return end;
}

static size_t obj_desc_charges(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	bool aware = object_flavor_is_aware(o_ptr) || (o_ptr->ident & IDENT_STORE);

	/* Wands and Staffs have charges */
	if (aware && (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND))
		strnfcat(buf, max, &end, " (%d charge%s)", o_ptr->pval[DEFAULT_PVAL], PLURAL(o_ptr->pval[DEFAULT_PVAL]));

	/* Charging things */
	else if (o_ptr->timeout > 0)
	{
		if (o_ptr->tval == TV_ROD && o_ptr->number > 1)
		{
			int power;
			int time_base = randcalc(o_ptr->kind->time, 0, MINIMISE);

			if (!time_base) time_base = 1;

			/*
			 * Find out how many rods are charging, by dividing
			 * current timeout by each rod's maximum timeout.
			 * Ensure that any remainder is rounded up.  Display
			 * very discharged stacks as merely fully discharged.
			 */
			power = (o_ptr->timeout + (time_base - 1)) / time_base;
			if (power > o_ptr->number) power = o_ptr->number;

			/* Display prettily */
			strnfcat(buf, max, &end, " (%d charging)", power);
		}

		/* Artifacts, single rods */
		else if (!(o_ptr->tval == TV_LIGHT && !o_ptr->artifact))
		{
			strnfcat(buf, max, &end, " (charging)");
		}
	}

	return end;
}

static size_t obj_desc_inscrip(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	const char *u[4] = { 0, 0, 0, 0 };
	int n = 0;
	int feel = object_pseudo(o_ptr);
	bitflag flags_known[OF_SIZE], f2[OF_SIZE];

	object_flags_known(o_ptr, flags_known);

	/* Get inscription */
	if (o_ptr->note)
		u[n++] = quark_str(o_ptr->note);

	/* Use special inscription, if any */
	if (!object_is_known(o_ptr) && feel)
	{
		/* cannot tell excellent vs strange vs splendid until wield */
		if (!object_was_worn(o_ptr) && o_ptr->ego)
			u[n++] = "ego";
		else
			u[n++] = inscrip_text[feel];
	}
	else if ((o_ptr->ident & IDENT_EMPTY) && !object_is_known(o_ptr))
		u[n++] = "empty";
	else if (!object_is_known(o_ptr) && object_was_worn(o_ptr))
	{
		if (wield_slot(o_ptr) == INVEN_WIELD || wield_slot(o_ptr) == INVEN_BOW)
			u[n++] = "wielded";
		else u[n++] = "worn";
	}
	else if (!object_is_known(o_ptr) && object_was_fired(o_ptr))
		u[n++] = "fired";
	else if (!object_flavor_is_aware(o_ptr) && object_flavor_was_tried(o_ptr))
		u[n++] = "tried";

	/* Note curses */
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);
	if (of_is_inter(flags_known, f2))
		u[n++] = "cursed";

	/* Note squelch */
	if (squelch_item_ok(o_ptr))
		u[n++] = "squelch";

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


/* Add "unseen" to the end of unaware items in stores */
static size_t obj_desc_aware(const object_type *o_ptr, char *buf, size_t max,
	size_t end)
{
	if (!object_flavor_is_aware(o_ptr))
		strnfcat(buf, max, &end, " {unseen}");

	return end;
}


/**
 * Describes item `o_ptr` into buffer `buf` of size `max`.
 *
 * ODESC_PREFIX prepends a 'the', 'a' or number
 * ODESC_BASE results in a base description.
 * ODESC_COMBAT will add to-hit, to-dam and AC info.
 * ODESC_EXTRA will add pval/charge/inscription/squelch info.
 * ODESC_PLURAL will pluralise regardless of the number in the stack.
 * ODESC_STORE turns off squelch markers, for in-store display.
 * ODESC_SPOIL treats the object as fully identified.
 *
 * Setting 'prefix' to TRUE prepends a 'the', 'a' or the number in the stack,
 * respectively.
 *
 * \returns The number of bytes used of the buffer.
 */
size_t object_desc(char *buf, size_t max, const object_type *o_ptr,
				   odesc_detail_t mode)
{
	bool prefix = mode & ODESC_PREFIX;
	bool spoil = (mode & ODESC_SPOIL);
	bool known; 

	size_t end = 0, i = 0;

	/* Simple description for null item */
	if (!o_ptr->tval)
		return strnfmt(buf, max, "(nothing)");

	known = object_is_known(o_ptr) ||
			(o_ptr->ident & IDENT_STORE) || spoil;

	/* We've seen it at least once now we're aware of it */
	if (known && o_ptr->ego) o_ptr->ego->everseen = TRUE;


	/*** Some things get really simple descriptions ***/

	if (o_ptr->tval == TV_GOLD)
		return strnfmt(buf, max, "%d gold pieces worth of %s%s",
				o_ptr->pval[DEFAULT_PVAL], o_ptr->kind->name,
				squelch_item_ok(o_ptr) ? " {squelch}" : "");

	/** Construct the name **/

	/* Copy the base name to the buffer */
	end = obj_desc_name(buf, max, end, o_ptr, prefix, mode, spoil);

	if (mode & ODESC_COMBAT)
	{
		if (o_ptr->tval == TV_CHEST)
			end = obj_desc_chest(o_ptr, buf, max, end);
		else if (o_ptr->tval == TV_LIGHT)
			end = obj_desc_light(o_ptr, buf, max, end);

		end = obj_desc_combat(o_ptr, buf, max, end, spoil);
	}

	if (mode & ODESC_EXTRA)
	{
		for (i = 0; i < o_ptr->num_pvals; i++)
			if (spoil || object_this_pval_is_visible(o_ptr, i)) {
				end = obj_desc_pval(o_ptr, buf, max, end, spoil);
				break;
			}

		end = obj_desc_charges(o_ptr, buf, max, end);

		if (mode & ODESC_STORE)
		{
			end = obj_desc_aware(o_ptr, buf, max, end);
		}
		else
			end = obj_desc_inscrip(o_ptr, buf, max, end);
	}

	return end;
}
