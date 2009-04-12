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
#include "tvalsval.h"


/*
 * Puts a very stripped-down version of an object's name into buf.
 * If easy_know is TRUE, then the IDed names are used, otherwise
 * flavours, scroll names, etc will be used.
 *
 * Just truncates if the buffer isn't big enough.
 */
void object_kind_name(char *buf, size_t max, int k_idx, bool easy_know)
{
	char *t;

	object_kind *k_ptr = &k_info[k_idx];

	/* If not aware, use flavor */
	if (!easy_know && !k_ptr->aware && k_ptr->flavor)
	{
		if (k_ptr->tval == TV_SCROLL)
		{
			strnfmt(buf, max, "\"%s\"", scroll_adj[k_ptr->sval]);
		}
		else if (k_ptr->tval == TV_FOOD && k_ptr->sval > SV_FOOD_MIN_SHROOM)
		{
			strnfmt(buf, max, "%s Mushroom", flavor_text + flavor_info[k_ptr->flavor].text);
		}
		else
		{
			/* Plain flavour (e.g. Copper) will do. */
			my_strcpy(buf, flavor_text + flavor_info[k_ptr->flavor].text, max);
		}
	}

	/* Use proper name (Healing, or whatever) */
	else
	{
		cptr str = (k_name + k_ptr->name);

		if (k_ptr->tval == TV_FOOD && k_ptr->sval > SV_FOOD_MIN_SHROOM)
		{
			my_strcpy(buf, "Mushroom of ", max);
			max -= strlen(buf);
			t = buf + strlen(buf);
		}
		else
		{
			t = buf;
		}

		/* Skip past leading characters */
		while ((*str == ' ') || (*str == '&')) str++;

		/* Copy useful chars */
		for (; *str && max > 1; str++)
		{
			/* Pluralizer for irregular plurals */
			/* Useful for languages where adjective changes for plural */
			if (*str == '|')
			{
				/* Process singular part */
				for (str++; *str != '|' && max > 1; str++)
				{
					*t++ = *str;
					max--;
				}

				/* Process plural part */
				for (str++; *str != '|'; str++) ;
			}

			/* English plural indicator can simply be skipped */
			else if (*str != '~')
			{
				*t++ = *str;
				max--;
			}
		}

		/* Terminate the new name */
		*t = '\0';
	}
}



static const char *obj_desc_get_modstr(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	switch (o_ptr->tval)
	{
		case TV_AMULET:
		case TV_RING:
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
		case TV_POTION:
		case TV_FOOD:
			return (flavor_text + flavor_info[k_ptr->flavor].text);

		case TV_SCROLL:
			return scroll_adj[o_ptr->sval];

		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
			return (k_name + k_ptr->name);
	}

	return "";
}

static const char *obj_desc_get_basename(const object_type *o_ptr, bool aware)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	bool show_flavor = k_ptr->flavor ? TRUE : FALSE;


	if (o_ptr->ident & IDENT_STORE) show_flavor = FALSE;
	if (aware && !OPT(show_flavors)) show_flavor = FALSE;



	/* Known artifacts get special treatment */
	if (artifact_p(o_ptr) && aware)
		return (k_name + k_ptr->name);

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
		case TV_LITE:
			return (k_name + k_ptr->name);

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
			return (show_flavor ? "& Scroll~ titled \"#\"" : "& Scroll~");

		case TV_MAGIC_BOOK:
			return "& Book~ of Magic Spells #";

		case TV_PRAYER_BOOK:
			return "& Holy Book~ of Prayers #";

		case TV_FOOD:
			if (o_ptr->sval > SV_FOOD_MIN_SHROOM)
				return (show_flavor ? "& # Mushroom~" : "& Mushroom~");
			else
				return (k_name + k_ptr->name);
	}

	return "(nothing)";
}






/*
 * Copy 'src' into 'buf, replacing '#' with 'modstr' (if found), putting a plural
 * in the place indicated by '~' if required, or using alterate...
 */
static size_t obj_desc_name(char *buf, size_t max, size_t end,
		const object_type *o_ptr, bool prefix, odesc_detail_t mode,
		bool spoil)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	bool known = object_known_p(o_ptr) || (o_ptr->ident & IDENT_STORE) || spoil;
	bool aware = object_aware_p(o_ptr) || (o_ptr->ident & IDENT_STORE) || spoil;

	const char *basename = obj_desc_get_basename(o_ptr, aware);
	const char *modstr = obj_desc_get_modstr(o_ptr);

	bool pluralise = (mode & ODESC_PLURAL) ? TRUE : FALSE;

	if (o_ptr->number > 1)
		pluralise = TRUE;
	if (mode & ODESC_SINGULAR)
		pluralise = FALSE;

	/* Add a pseudo-numerical prefix if desired */
	if (prefix)
	{
		if (o_ptr->number <= 0)
		{
			strnfcat(buf, max, &end, "no more ");
			
			/* Pluralise for grammatical correctness */
			pluralise = TRUE;
		}
		else if (o_ptr->number > 1)
			strnfcat(buf, max, &end, "%d ", o_ptr->number);
		else if (known && artifact_p(o_ptr))
			strnfcat(buf, max, &end, "The ");

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
	}


/*
 * Names have the following elements:
 *
 * '~' indicates where to place an 's' or an 'es'.  Other plural forms should
 * be handled with the syntax '|singular|plural|', e.g. "kni|fe|ves|".
 *
 * '#' indicates the position of the "modifier", e.g. the flavour or spellbook
 * name.
 */



	/* Copy the string */
	while (*basename)
	{
		if (*basename == '&')
		{
			while (*basename == ' ' || *basename == '&')
				basename++;
			continue;
		}

		/* Pluralizer (regular English plurals) */
		else if (*basename == '~')
		{
			char prev = *(basename - 1);

			if (!pluralise)
			{
				basename++;
				continue;
			}

			/* e.g. cutlass-e-s, torch-e-s, box-e-s */
			if (prev == 's' || prev == 'h' || prev == 'x')
				strnfcat(buf, max, &end, "es");
			else
				strnfcat(buf, max, &end, "s");
		}

		/* Special plurals */
		else if (*basename == '|')
		{
			/* e.g. & Wooden T|o|e|rch~
			 *                 ^ ^^ */
			const char *singular = basename + 1;
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

			basename = endmark;
		}

		/* Handle pluralisation in the modifier XXX */
		else if (*basename == '#')
		{
			const char *basename = modstr;

			while (basename && *basename && (end < max - 1))
			{
				/* Special plurals */
				if (*basename == '|')
				{
					/* e.g. & Wooden T|o|e|rch~
					 *                 ^ ^^ */
					const char *singular = basename + 1;
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

					basename = endmark;
				}

				else
					buf[end++] = *basename;

				basename++;
			}
		}

		else
			buf[end++] = *basename;

		basename++;
	}

	/* 0-terminate, just in case XXX */
	buf[end] = 0;


	/** Append extra names of various kinds **/

	if (known && o_ptr->name1)
		strnfcat(buf, max, &end, " %s", a_name + a_info[o_ptr->name1].name);

	else if (known && o_ptr->name2)
		strnfcat(buf, max, &end, " %s", e_name + e_info[o_ptr->name2].name);

	else if (aware && (k_ptr->flavor || k_ptr->tval == TV_SCROLL))
		strnfcat(buf, max, &end, " of %s", k_name + k_ptr->name);

	return end;
}

/*
 * Is o_ptr a weapon?
 */
static bool obj_desc_show_weapon(const object_type *o_ptr)
{
	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);

	if (f[2] & TR2_SHOW_MODS) return TRUE;
	if (o_ptr->to_h && o_ptr->to_d) return TRUE;

	switch (o_ptr->tval)
	{
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			return TRUE;
		}
	}

	return FALSE;
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
	bool known = object_known_p(o_ptr) || (o_ptr->ident & IDENT_STORE);

	if (o_ptr->tval != TV_CHEST) return end;
	if (!known) return end;

	/* May be "empty" */
	if (!o_ptr->pval)
		strnfcat(buf, max, &end, " (empty)");

	/* May be "disarmed" */
	else if (o_ptr->pval < 0)
	{
		if (chest_traps[0 - o_ptr->pval])
			strnfcat(buf, max, &end, " (disarmed)");
		else
			strnfcat(buf, max, &end, " (unlocked)");
	}

	/* Describe the traps, if any */
	else
	{
		/* Describe the traps */
		switch (chest_traps[o_ptr->pval])
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
	bool known = object_known_p(o_ptr) || (o_ptr->ident & IDENT_STORE) ||
			spoil;

	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		/* Weapons */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			strnfcat(buf, max, &end, " (%dd%d)", o_ptr->dd, o_ptr->ds);
			break;
		}

		/* Missile launchers */
		case TV_BOW:
		{
			strnfcat(buf, max, &end, " (x%d)", o_ptr->sval % 10);
			break;
		}
	}


	/* Show weapon bonuses */
	if (known || o_ptr->ident & IDENT_ATTACK)
	{
		if (obj_desc_show_weapon(o_ptr) || o_ptr->to_d)
			strnfcat(buf, max, &end, " (%+d,%+d)", o_ptr->to_h, o_ptr->to_d);
		else if (o_ptr->to_h)
			strnfcat(buf, max, &end, " (%+d)", o_ptr->to_h);
	}


	/* Show armor bonuses */
	if (known || o_ptr->ident & IDENT_DEFENCE)
	{
		if (obj_desc_show_armor(o_ptr))
			strnfcat(buf, max, &end, " [%d,%+d]", o_ptr->ac, o_ptr->to_a);
		else if (o_ptr->to_a)
			strnfcat(buf, max, &end, " [%+d]", o_ptr->to_a);
	}
	else if (obj_desc_show_armor(o_ptr))
	{
		strnfcat(buf, max, &end, " [%d]", o_ptr->ac);
	}

	return end;
}

static size_t obj_desc_light(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);

	/* Fuelled light sources get number of remaining turns appended */
	if ((o_ptr->tval == TV_LITE) && !(f[2] & TR2_NO_FUEL))
		strnfcat(buf, max, &end, " (%d turns)", o_ptr->timeout);

	return end;
}

static size_t obj_desc_pval(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	u32b f[OBJ_FLAG_N];
	object_flags(o_ptr, f);

	if (!(f[0] & TR0_PVAL_MASK)) return end;

	if (f[2] & TR2_HIDE_TYPE)
	{
		strnfcat(buf, max, &end, " (%+d)", o_ptr->pval);
		return end;
	}

	strnfcat(buf, max, &end, " (%+d", o_ptr->pval);

	if (f[0] & TR0_STEALTH)
		strnfcat(buf, max, &end, " stealth");
	else if (f[0] & TR0_SEARCH)
		strnfcat(buf, max, &end, " searching");
	else if (f[0] & TR0_INFRA)
		strnfcat(buf, max, &end, " infravision");
	else if (f[0] & TR0_SPEED)
		strnfcat(buf, max, &end, " speed");
	else if (f[0] & TR0_BLOWS)
		strnfcat(buf, max, &end, " attack%s", PLURAL(o_ptr->pval));

	strnfcat(buf, max, &end, ")");

	return end;
}

static size_t obj_desc_charges(const object_type *o_ptr, char *buf, size_t max, size_t end)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Wands and Staffs have charges */
	if (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND)
		strnfcat(buf, max, &end, " (%d charge%s)", o_ptr->pval, PLURAL(o_ptr->pval));

	/* Charging things */
	else if (o_ptr->timeout > 0)
	{
		if (o_ptr->tval == TV_ROD && o_ptr->number > 1)
		{
			int power;
			int time_base = k_ptr->time_base;

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
		else if (!(o_ptr->tval == TV_LITE && !artifact_p(o_ptr)))
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

	/* Get inscription */
	if (o_ptr->note)
		u[n++] = quark_str(o_ptr->note);

	/* Use special inscription, if any */
	if (!object_known_p(o_ptr) && feel)
		u[n++] = inscrip_text[feel];
	else if ((o_ptr->ident & IDENT_EMPTY) && !object_known_p(o_ptr))
		u[n++] = "empty";
	else if (!object_aware_p(o_ptr) && object_tried_p(o_ptr))
		u[n++] = "tried";

	/* Note curses */
	if (o_ptr->known_flags[2] & TR2_CURSE_MASK)
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


/**
 * Describes item `o_ptr` into buffer `buf` of size `max`.
 *
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
		bool prefix, odesc_detail_t mode)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	bool spoil = (mode & ODESC_SPOIL);
	bool aware = object_aware_p(o_ptr) ||
			(o_ptr->ident & IDENT_STORE) || spoil;
	bool known = object_known_p(o_ptr) ||
			(o_ptr->ident & IDENT_STORE) || spoil;

	size_t end = 0;


	/*** Some things get really simple descriptions ***/

	if (o_ptr->tval == TV_GOLD)
		return strnfmt(buf, max, "%d gold pieces worth of %s",
				o_ptr->pval, k_name + k_ptr->name);
	else if (!o_ptr->tval)
		return strnfmt(buf, max, "(nothing)");


	/** Horrible, horrible squelch **/

	/* Hack -- mark-to-squelch worthless items XXX */
	if (aware && !k_ptr->everseen)
	{
		k_ptr->everseen = TRUE;
		if (OPT(squelch_worthless) && k_ptr->cost == 0)
		{
			k_ptr->squelch = TRUE;
			p_ptr->notice |= PN_SQUELCH;
		}
	}

	/* We've seen it at least once now we're aware of it */
	if (known && o_ptr->name2) e_info[o_ptr->name2].everseen = TRUE;


	/** Construct the name **/

	/* Copy the base name to the buffer */
	end = obj_desc_name(buf, max, end, o_ptr, prefix, mode, spoil);

	if (mode & ODESC_COMBAT)
	{
		if (o_ptr->tval == TV_CHEST)
			end = obj_desc_chest(o_ptr, buf, max, end);
		else if (o_ptr->tval == TV_LITE)
			end = obj_desc_light(o_ptr, buf, max, end);

		end = obj_desc_combat(o_ptr, buf, max, end, spoil);
	}

	if (mode & ODESC_EXTRA)
	{
		if (known)
		{
			end = obj_desc_pval(o_ptr, buf, max, end);
			end = obj_desc_charges(o_ptr, buf, max, end);
		}

		if (!(mode & ODESC_STORE))
			end = obj_desc_inscrip(o_ptr, buf, max, end);
	}

	return end;
}
