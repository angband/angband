/*
 * File: obj-info.c
 * Purpose: Object description code.
 *
 * Copyright (c) 2002-2007 Andrew Sidwell, Robert Ruehlmann
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
#include "effects.h"
#include "cmds.h"

/*
 * Describes a flag-name pair.
 */
typedef struct
{
	u32b flag;
	const char *name;
} flag_type;



/*** Utility code ***/

/*
 * Given an array of strings, as so:
 *  { "intelligence", "fish", "lens", "prime", "number" },
 *
 * ... output a list like "intelligence, fish, lens, prime, number.\n".
 */
static void info_out_list(const char *list[], size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
	{
		text_out(list[i]);
		if (i != (count - 1)) text_out(", ");
	}

	text_out(".\n");
}


/*
 *
 */
static size_t info_collect(const flag_type list[], size_t max, u32b flag, const char *recepticle[])
{
	size_t i, count = 0;

	for (i = 0; i < max; i++)
	{
		if (flag & list[i].flag)
			recepticle[count++] = list[i].name;
	}

	return count;
}


/*** Big fat data tables ***/

static const flag_type f1_pval[] =
{
	{ TR1_STR,     "strength" },
	{ TR1_INT,     "intelligence" },
	{ TR1_WIS,     "wisdom" },
	{ TR1_DEX,     "dexterity" },
	{ TR1_CON,     "constitution" },
	{ TR1_CHR,     "charisma" },
	{ TR1_STEALTH, "stealth" },
	{ TR1_SEARCH,  "searching" },
	{ TR1_INFRA,   "infravision" },
	{ TR1_TUNNEL,  "tunneling" },
	{ TR1_SPEED,   "speed" },
	{ TR1_BLOWS,   "attack speed" },
	{ TR1_SHOTS,   "shooting speed" },
	{ TR1_MIGHT,   "shooting power" },
};

static const flag_type f2_immunity[] =
{
	{ TR2_IM_ACID, "acid" },
	{ TR2_IM_ELEC, "lightning" },
	{ TR2_IM_FIRE, "fire" },
	{ TR2_IM_COLD, "cold" },
};

static const flag_type f2_resist[] =
{
	{ TR2_RES_ACID,  "acid" },
	{ TR2_RES_ELEC,  "lightning" },
	{ TR2_RES_FIRE,  "fire" },
	{ TR2_RES_COLD,  "cold" },
	{ TR2_RES_POIS,  "poison" },
	{ TR2_RES_FEAR,  "fear" },
	{ TR2_RES_LITE,  "light" },
	{ TR2_RES_DARK,  "dark" },
	{ TR2_RES_BLIND, "blindness" },
	{ TR2_RES_CONFU, "confusion" },
	{ TR2_RES_SOUND, "sound" },
	{ TR2_RES_SHARD, "shards" },
	{ TR2_RES_NEXUS, "nexus"  },
	{ TR2_RES_NETHR, "nether" },
	{ TR2_RES_CHAOS, "chaos" },
	{ TR2_RES_DISEN, "disenchantment" },
};

static const flag_type f3_ignore[] =
{
	{ TR3_IGNORE_ACID, "acid" },
	{ TR3_IGNORE_ELEC, "electricity" },
	{ TR3_IGNORE_FIRE, "fire" },
	{ TR3_IGNORE_COLD, "cold" },
};

static const flag_type f2_sustains[] =
{
	{ TR2_SUST_STR, "strength" },
	{ TR2_SUST_INT, "intelligence" },
	{ TR2_SUST_WIS, "wisdom" },
	{ TR2_SUST_DEX, "dexterity" },
	{ TR2_SUST_CON, "constitution" },
	{ TR2_SUST_CHR, "charisma" },
};

static const flag_type f3_misc[] =
{
	{ TR3_BLESSED,     "Blessed by the gods" },
	{ TR3_SLOW_DIGEST, "Slows your metabolism" },
	{ TR3_FEATHER,     "Feather Falling" },
	{ TR3_REGEN,       "Speeds regeneration" },
	{ TR3_FREE_ACT,    "Prevents paralysis" },
	{ TR3_HOLD_LIFE,   "Stops experience drain" },
	{ TR3_TELEPATHY,   "Grants telepathy" },
	{ TR3_SEE_INVIS,   "Grants the ability to see invisible things" },
	{ TR3_AGGRAVATE,   "Aggravates creatures nearby" },
	{ TR3_DRAIN_EXP,   "Drains experience" },
	{ TR3_TELEPORT,    "Induces random teleportation" },
};


/*** Code that makes use of the data tables ***/

/*
 * Describe stat modifications.
 */
static bool describe_stats(u32b f1, int pval)
{
	cptr descs[N_ELEMENTS(f1_pval)];
	size_t count;

	if (!pval) return FALSE;

	count = info_collect(f1_pval, N_ELEMENTS(f1_pval), f1, descs);
	if (!count) return FALSE;

	text_out_c((pval > 0) ? TERM_L_GREEN : TERM_RED, "%+i ", pval);
	info_out_list(descs, count);

	return TRUE;
}


/*
 * Describe immunities granted by an object.
 */
static bool describe_immune(u32b f2)
{
	const char *descs[N_ELEMENTS(f2_resist)];
	size_t count;

	bool prev = FALSE;

	/* Immunities */
	count = info_collect(f2_immunity, N_ELEMENTS(f2_immunity), f2, descs);
	if (count)
	{
		text_out("Provides immunity to ");
		info_out_list(descs, count);
		prev = TRUE;
	}

	/* Resistances */
	count = info_collect(f2_resist, N_ELEMENTS(f2_resist), f2, descs);
	if (count)
	{
		text_out("Provides resistance to ");
		info_out_list(descs, count);
		prev = TRUE;
	}

	return prev;
}


/*
 * Describe 'ignores' of an object.
 */
static bool describe_ignores(u32b f3)
{
	const char *descs[N_ELEMENTS(f3_ignore)];
	size_t count = info_collect(f3_ignore, N_ELEMENTS(f3_ignore), f3, descs);

	if (!count) return FALSE;

	text_out("Cannot be harmed by ");
	info_out_list(descs, count);

	return TRUE;
}


/*
 * Describe stat sustains.
 */
static bool describe_sustains(u32b f2)
{
	const char *descs[N_ELEMENTS(f2_sustains)];
	size_t count = info_collect(f2_sustains, N_ELEMENTS(f2_sustains), f2, descs);

	if (!count) return FALSE;

	text_out("Sustains ");
	info_out_list(descs, count);

	return TRUE;
}


/*
 * Describe miscellaneous powers.
 */
static bool describe_misc_magic(u32b f3)
{
	size_t i;
	bool printed = FALSE;

	for (i = 0; i < N_ELEMENTS(f3_misc); i++)
	{
		if (f3 & f3_misc[i].flag)
		{
			text_out("%s.\n", f3_misc[i].name);
			printed = TRUE;
		}
	}

	return printed;
}











/*
 * list[] and mult[] must be > 11 in size
 */
static int collect_slays(cptr desc[], int mult[], u32b f1)
{
	int cnt = 0;

	/* Collect slays */
	if (f1 & TR1_SLAY_ANIMAL) { mult[cnt] = 2; desc[cnt++] = "animals"; }
	if (f1 & TR1_SLAY_EVIL)   { mult[cnt] = 2; desc[cnt++] = "evil creatures"; }

	if (f1 & TR1_SLAY_ORC)    { mult[cnt] = 3; desc[cnt++] = "orcs"; }
	if (f1 & TR1_SLAY_TROLL)  { mult[cnt] = 3; desc[cnt++] = "trolls"; }
	if (f1 & TR1_SLAY_GIANT)  { mult[cnt] = 3; desc[cnt++] = "giants"; }
	if (f1 & TR1_SLAY_DRAGON) { mult[cnt] = 3; desc[cnt++] = "dragons"; }
	if (f1 & TR1_SLAY_DEMON)  { mult[cnt] = 3; desc[cnt++] = "demons"; }
	if (f1 & TR1_SLAY_UNDEAD) { mult[cnt] = 3; desc[cnt++] = "undead"; }

	if (f1 & TR1_BRAND_ACID)  { mult[cnt] = 3; desc[cnt++] = "acid-vulnerable creatures"; }
	if (f1 & TR1_BRAND_ELEC)  { mult[cnt] = 3; desc[cnt++] = "electricity-vulnerable creatures"; }
	if (f1 & TR1_BRAND_FIRE)  { mult[cnt] = 3; desc[cnt++] = "fire-vulnerable creatures"; }
	if (f1 & TR1_BRAND_COLD)  { mult[cnt] = 3; desc[cnt++] = "frost-vulnerable creatures"; }
	if (f1 & TR1_BRAND_POIS)  { mult[cnt] = 3; desc[cnt++] = "poison-vulnerable creatures"; }

	if (f1 & TR1_KILL_DRAGON) { mult[cnt] = 5; desc[cnt++] = "dragons"; }
	if (f1 & TR1_KILL_DEMON)  { mult[cnt] = 5; desc[cnt++] = "demons"; }
	if (f1 & TR1_KILL_UNDEAD) { mult[cnt] = 5; desc[cnt++] = "undead"; }

	return cnt;
}



/*
 * Describe combat advantages.
 */
static bool describe_combat(const object_type *o_ptr, bool full)
{
	cptr desc[15];
	int mult[15];
	int cnt, dam;
	int xtra_dam = 0;
	object_type *j_ptr = &inventory[INVEN_BOW];

	u32b f1, f2, f3;

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->ammo_tval == o_ptr->tval) &&
	              (j_ptr->k_idx);

	/* Abort if we've nothing to say */
	if (!weapon && !ammo)
	{
		/* Potions can have special text */
		if (o_ptr->tval != TV_POTION) return FALSE;
		if (!o_ptr->dd || !o_ptr->ds) return FALSE;
		if (!object_known_p(o_ptr)) return FALSE;

		text_out("It can be thrown at creatures with damaging effect.\n");
		return TRUE;
	}

	if (full)
		object_flags(o_ptr, &f1, &f2, &f3);
	else
		object_flags_known(o_ptr, &f1, &f2, &f3);


	if (weapon)
	{
		int blows = calc_blows(o_ptr);

		dam = (o_ptr->ds * o_ptr->dd * 5);

		xtra_dam = (p_ptr->to_d * 10);
		if (object_known_p(o_ptr))
			xtra_dam += (o_ptr->to_d * 10);

		/* Warn about heavy weapons */
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			text_out_c(TERM_L_RED, "You are too weak to use this weapon effectively!\n");
			blows = 1;
		}
	
	    text_out("With this weapon, you would currently get ");
	    text_out_c(TERM_L_GREEN, format("%d ", blows));
	    if (blows > 1)
			text_out("blows per round.  Each blow will do an average damage of ");
	    else
			text_out("blow per round, averaging a damage of ");
	}
	else
	{
		int tdis = 10 + 5 * p_ptr->ammo_mult;

		/* Calculate damage */
		dam = (o_ptr->ds * o_ptr->dd * 5);
		if (object_known_p(o_ptr)) xtra_dam += (o_ptr->to_d * 10);
		if (object_known_p(j_ptr)) xtra_dam += (j_ptr->to_d * 10);
		xtra_dam *= p_ptr->ammo_mult;
		xtra_dam += (dam * 2);

		text_out("Fired from your current missile launcher, this arrow will hit targets up to ");
		text_out_c(TERM_L_GREEN, format("%d", tdis * 10));
		text_out(" feet away, inflicting an average damage of ");
	}

	/* Collect slays */
	cnt = collect_slays(desc, mult, f1);
	if (object_known_p(o_ptr) && cnt)
	{
		int i;

		for (i = 0; i < cnt; i++)
		{
			text_out_c(TERM_L_GREEN, "%d", ((dam * mult[i]) + xtra_dam) / 10);
			text_out(" against %s, ", desc[i]);
		}

		text_out("and ");
	}

	/* Include bonus damage in stated average */
	dam += xtra_dam;
    if (dam % 10)
		text_out_c(TERM_L_GREEN, "%d.%d", dam / 10, dam % 10);
    else
		text_out_c(TERM_L_GREEN, "%d", dam / 10);

	text_out(" against normal creatures.\n");

	/* Note the impact flag */
	if (f3 & TR3_IMPACT)
		text_out("Sometimes creates earthquakes on impact.\n");

	/* Add breakage chance */
	if (ammo)
	{
		text_out_c(TERM_L_GREEN, "%d%%", breakage_chance(o_ptr));
		text_out(" chance of breaking upon contact.\n");
	}

	/* You always have something to say... */
	return TRUE;
}



/*
 * Describe things that look like lights.
 */
static bool describe_light(const object_type *o_ptr, u32b f3)
{
	int rad = 0;

	bool artifact = artifact_p(o_ptr);
	bool no_fuel = (f3 & TR3_NO_FUEL) ? TRUE : FALSE;
	bool is_lite = (o_ptr->tval == TV_LITE) ? TRUE : FALSE;

	if ((o_ptr->tval != TV_LITE) && !(f3 & TR3_LITE))
		return FALSE;

	/* Work out radius */
	if (artifact)      rad = 3;
	else if (is_lite)  rad = 2;
	if (f3 & TR3_LITE) rad++;

	/* Describe here */
	text_out("Radius ");
	text_out_c(TERM_L_GREEN, format("%d", rad));
	if (no_fuel && !artifact)
		text_out(" light.  No fuel required.");
	else if (is_lite && o_ptr->sval == SV_LITE_TORCH)
		text_out(" light, reduced when running of out fuel");
	text_out(".");

	if (is_lite)
	{
		const char *name = (o_ptr->sval == SV_LITE_TORCH) ? "torch" : "lantern";
		int turns = (o_ptr->sval == SV_LITE_TORCH) ? FUEL_TORCH : FUEL_LAMP;

		text_out("  Can refill another %s, up to %d turns of fuel.", name, turns);
	}

	text_out("\n");

	return TRUE;
}



/*
 * Describe an object's activation, if any.
 */
static bool describe_activation(const object_type *o_ptr, u32b f3, bool full)
{
	const object_kind *k_ptr = &k_info[o_ptr->k_idx];
	const char *desc;

	int effect, base, dice, sides;

	if (o_ptr->name1)
	{
		const artifact_type *a_ptr = &a_info[o_ptr->name1];
		if (!object_known_p(o_ptr) && !full) return FALSE;

		effect = a_ptr->effect;
		base = a_ptr->time_base;
		dice = a_ptr->time_dice;
		sides = a_ptr->time_sides;
	}
	else
	{
		if (!object_aware_p(o_ptr) && !full) return FALSE;
        
		effect = k_ptr->effect;
		base = k_ptr->time_base;
		dice = k_ptr->time_dice;
		sides = k_ptr->time_sides;
	}

	/* Forget it without an effect */
	if (!effect) return FALSE;

	/* Obtain the description */
	desc = effect_desc(effect);
	if (!desc) return FALSE;

	text_out("When ");

	if (f3 & TR3_ACTIVATE)
		text_out("activated");
	else if (effect_aim(effect))
		text_out("aimed");
	else if (o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION)
		text_out("ingested");
	else if (o_ptr->tval == TV_SCROLL)
	    text_out("read");
	else
	    text_out("used");

	text_out(", it ");

	/* Print a colourised description */
	do
	{
		if (isdigit((unsigned char) *desc))
			text_out_c(TERM_L_GREEN, "%c", *desc);
		else
			text_out("%c", *desc);
	} while (*desc++);

	text_out(".\n");

	if (base || dice || sides)
	{
		/* Some artifacts can be activated */
		text_out("It takes ");

		/* Output the number of turns */
		if (dice && dice != 1)
		    text_out_c(TERM_L_GREEN, "%d", dice);

		if (sides)
		    text_out_c(TERM_L_GREEN, "d%d", sides);

		if (base)
		{
			if (sides) text_out_c(TERM_L_GREEN, "+");
		    text_out_c(TERM_L_GREEN, "%d", base);
		}

		text_out(" turns to recharge after use.\n");
	}

	return TRUE;
}



/*
 * Output object information
 */
static bool object_info_out(const object_type *o_ptr, bool full)
{
	u32b f1, f2, f3;
	bool something = FALSE;

	/* Grab the object flags */
	if (full)
		object_flags(o_ptr, &f1, &f2, &f3);
	else
		object_flags_known(o_ptr, &f1, &f2, &f3);


	if (cursed_p(o_ptr))
	{
		if (f3 & TR3_PERMA_CURSE)
			text_out_c(TERM_L_RED, "Permanently cursed.\n");
		else if (f3 & TR3_HEAVY_CURSE)
			text_out_c(TERM_L_RED, "Heavily cursed.\n");
		else if (object_known_p(o_ptr))
			text_out_c(TERM_L_RED, "Cursed.\n");
	}

	if (describe_stats(f1, o_ptr->pval)) something = TRUE;
	if (describe_immune(f2)) something = TRUE;
	if (describe_ignores(f3)) something = TRUE;
	if (describe_sustains(f2)) something = TRUE;
	if (describe_misc_magic(f3)) something = TRUE;

	if (something) text_out("\n");

	if (describe_activation(o_ptr, f3, full)) something = TRUE;
	if (describe_light(o_ptr, f3)) something = TRUE;

	return something;
}


/*
 * Print name, origin, and descriptive text for a given object.
 */
void object_info_header(const object_type *o_ptr)
{
	char o_name[120];

	/* Object name */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);
	text_out_c(TERM_L_BLUE, "%^s\n", o_name);

	/* Display the origin */
	switch (o_ptr->origin)
	{
		case ORIGIN_NONE:
		case ORIGIN_MIXED:
			break;

		case ORIGIN_BIRTH:
			text_out("(an inheritance from your family)\n");
			break;

		case ORIGIN_STORE:
			text_out("(bought in a store)\n");
			break;

		case ORIGIN_FLOOR:
			text_out("(lying on the floor at %d feet (level %d))\n",
		         o_ptr->origin_depth * 50, o_ptr->origin_depth);			
 			break;

		case ORIGIN_DROP:
		{
			const char *name = r_name + r_info[o_ptr->origin_xtra].name;
			bool unique = (r_info[o_ptr->origin_xtra].flags1 & RF1_UNIQUE) ? TRUE : FALSE;

			text_out("dropped by ");

			if (unique)
				text_out("%s", name);
			else
				text_out("%s%s", is_a_vowel(name[0]) ? "an " : "a ", name);

			text_out(" at %d feet (level %d))\n",
		         o_ptr->origin_depth * 50, o_ptr->origin_depth);			

 			break;
		}

		case ORIGIN_DROP_UNKNOWN:
			text_out("(dropped by an unknown monster)\n");
			break;

		case ORIGIN_ACQUIRE:
			text_out("(conjured forth by magic)\n");
 			break;

		case ORIGIN_CHEAT:
			text_out("(created by debug option)\n");
 			break;

		case ORIGIN_CHEST:
			text_out("(found in a chest at %d feet (level %d))\n",
		         o_ptr->origin_depth * 50, o_ptr->origin_depth);			
			break;
	}

	text_out("\n");

	/* Display the known artifact description */
	if (!adult_randarts && o_ptr->name1 &&
	    object_known_p(o_ptr) && a_info[o_ptr->name1].text)
	{
		text_out(a_text + a_info[o_ptr->name1].text);
		text_out("\n\n");
	}

	/* Display the known object description */
	else if (object_aware_p(o_ptr) || object_known_p(o_ptr))
	{
		bool did_desc = FALSE;

		if (k_info[o_ptr->k_idx].text)
		{
			text_out(k_text + k_info[o_ptr->k_idx].text);
			did_desc = TRUE;
		}

		/* Display an additional ego-item description */
		if (o_ptr->name2 && object_known_p(o_ptr) && e_info[o_ptr->name2].text)
		{
			if (did_desc) text_out("  ");
			text_out(e_text + e_info[o_ptr->name2].text);
			text_out("\n\n");
		}
		else if (did_desc)
		{
			text_out("\n\n");
		}
	}

	return;
}


bool object_info_known(const object_type *o_ptr)
{
	bool has_info = FALSE;

	has_info = object_info_out(o_ptr, FALSE);

	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->pval)
	{
		text_out("Provides nourishment for about ");
		text_out_c(TERM_L_GREEN, "%d", o_ptr->pval / 2);
		text_out(" turns under normal conditions.\n");
		has_info = TRUE;
	}

	if (!object_known_p(o_ptr))
	{
		text_out("You do not know the full extent of this item's powers.");
		has_info = TRUE;
	}

	/* Describe combat bits */
	if (describe_combat(o_ptr, FALSE)) has_info = TRUE;

	return has_info;
}

bool object_info_full(const object_type *o_ptr)
{
	return object_info_out(o_ptr, TRUE);
}

bool object_info_store(const object_type *o_ptr)
{
	bool has_info = FALSE;

	has_info = object_info_out(o_ptr, TRUE);

	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->pval)
	{
		text_out("Provides nourishment for about ");
		text_out_c(TERM_L_GREEN, "%d", o_ptr->pval / 2);
		text_out(" turns under normal conditions.\n");
		has_info = TRUE;
	}

	if (describe_combat(o_ptr, TRUE)) has_info = TRUE;

	return has_info;
}
