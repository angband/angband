/*
 * File: obj-info.c
 * Purpose: Object description code.
 *
 * Copyright (c) 2002,2007,2008 Andi Sidwell <andi@takkaria.org>
 * Copyright (c) 2002,2003,2004 Robert Ruehlmann <rr9@thangorodrim.net>
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
#include "tvalsval.h"

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

static const flag_type f2_vuln[] =
{
	{ TR2_VULN_ACID, "acid" },
	{ TR2_VULN_ELEC, "electricity" },
	{ TR2_VULN_FIRE, "fire" },
	{ TR2_VULN_COLD, "cold" },
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
	{ TR3_BLESSED, "Blessed by the gods" },
	{ TR3_SLOW_DIGEST, "Slows your metabolism" },
	{ TR3_IMPAIR_HP, "Impairs hitpoint recovery" },
	{ TR3_IMPAIR_MANA, "Impairs mana recovery" },
	{ TR3_AFRAID, "Makes you unable to hit foes" },
	{ TR3_FEATHER, "Feather Falling" },
	{ TR3_REGEN, "Speeds regeneration" },
	{ TR3_FREE_ACT, "Prevents paralysis" },
	{ TR3_HOLD_LIFE, "Stops experience drain" },
	{ TR3_TELEPATHY, "Grants telepathy" },
	{ TR3_SEE_INVIS, "Grants the ability to see invisible things" },
	{ TR3_AGGRAVATE, "Aggravates creatures nearby" },
	{ TR3_DRAIN_EXP, "Drains experience" },
	{ TR3_TELEPORT, "Induces random teleportation" },
};

static const flag_type f1_slay[] =
{
	{ TR1_SLAY_ANIMAL, "animals" },
	{ TR1_SLAY_EVIL, "evil creatures" },
	{ TR1_SLAY_ORC, "orcs" },
	{ TR1_SLAY_TROLL, "trolls" },
	{ TR1_SLAY_GIANT, "giants" },
	{ TR1_SLAY_DRAGON, "dragons" },
	{ TR1_SLAY_DEMON, "demons" },
	{ TR1_SLAY_UNDEAD, "undead" },
};

static const flag_type f1_brand[] =
{
	{ TR1_BRAND_ACID, "acid" },
	{ TR1_BRAND_ELEC, "lightning" },
	{ TR1_BRAND_FIRE, "flames" },
	{ TR1_BRAND_COLD, "frost" },
	{ TR1_BRAND_POIS, "venom" },
};

static const flag_type f1_kill[] =
{
	{ TR1_KILL_DRAGON, "dragons" },
	{ TR1_KILL_DEMON, "demons" },
	{ TR1_KILL_UNDEAD, "undead" },
};

/** Slays **/
const slay_t slay_table[] =
/* Entries in this table should be an ascending order of multiplier, to 
 * ensure that the highest one takes precedence 
 * object flag, vulnerable flag, resist_flag, multiplier, ranged verb, 
 * melee verb, description of vulnerable creature 
 */
{ { TR1_SLAY_ANIMAL, RF2_ANIMAL, 0, 2, "pierces",  "smite",   "animals"},
  { TR1_SLAY_EVIL,   RF2_EVIL,   0, 2, "pierces",  "smite",   "evil creatures"},
  { TR1_SLAY_UNDEAD, RF2_UNDEAD, 0, 3, "pierces",  "smite",   "undead"},
  { TR1_SLAY_DEMON,  RF2_DEMON,  0, 3, "pierces",  "smite",   "demons"},
  { TR1_SLAY_ORC,    RF2_ORC,    0, 3, "pierces",  "smite",   "orcs"},
  { TR1_SLAY_TROLL,  RF2_TROLL,  0, 3, "pierces",  "smite",   "trolls"},
  { TR1_SLAY_GIANT,  RF2_GIANT,  0, 3, "pierces",  "smite",   "giants"},
  { TR1_SLAY_DRAGON, RF2_DRAGON, 0, 3, "pierces",  "smite",   "dragons"},
  { TR1_BRAND_ACID, 0, RF2_IM_ACID, 3, "corrodes", "corrode", "acid-vulnerable creatures"},
  { TR1_BRAND_ELEC, 0, RF2_IM_ELEC, 3, "zaps",     "zap",     "electricity-vulnerable creatures"},
  { TR1_BRAND_FIRE, 0, RF2_IM_FIRE, 3, "burns",    "burn",    "fire-vulnerable creatures"},
  { TR1_BRAND_COLD, 0, RF2_IM_COLD, 3, "freezes",  "freeze",  "cold-vulnerable creatures"},
  { TR1_BRAND_POIS, 0, RF2_IM_POIS, 3, "poisons",  "poison",  "poison-vulnerable creatures"},
  { TR1_KILL_DRAGON, RF2_DRAGON, 0, 5, "deeply pierces", "fiercely smite", "dragons"},
  { TR1_KILL_DEMON,  RF2_DEMON,  0, 5, "deeply pierces", "fiercely smite", "demons"},
  { TR1_KILL_UNDEAD, RF2_UNDEAD, 0, 5, "deeply pierces", "fiercely smite", "undead"},
  { 0, }
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
	if (count)
	{
		text_out_c((pval > 0) ? TERM_L_GREEN : TERM_RED, "%+i ", pval);
		info_out_list(descs, count);
	}

	if (f1 & TR1_SEARCH)
	{
		text_out_c((pval > 0) ? TERM_L_GREEN : TERM_RED, "%+i%% ", pval * 5);
		text_out(" to searching.\n");
	}

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

	/* Resistances */
	count = info_collect(f2_vuln, N_ELEMENTS(f2_vuln), f2, descs);
	if (count)
	{
		text_out("Makes you vulnerable to ");
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
 * Describe slays and brands on weapons
 */

static bool describe_slays(u32b f1)
{
	const char *descs[N_ELEMENTS(f1_slay)];
	size_t count;

	bool printed = FALSE;

	/* Slays */
	count = info_collect(f1_slay, N_ELEMENTS(f1_slay), f1, descs);
	if (count)
	{
		text_out("It is especially deadly to ");
		info_out_list(descs, count);
		printed = TRUE;
	}

	/* Kills */
	count = info_collect(f1_kill, N_ELEMENTS(f1_kill), f1, descs);
	if (count)
	{
		text_out("It is a great bane of ");
		info_out_list(descs, count);
		printed = TRUE;
	}

	/* Brands */
	count = info_collect(f1_brand, N_ELEMENTS(f1_brand), f1, descs);
	if (count)
	{
		text_out("It is branded with ");
		info_out_list(descs, count);
		printed = TRUE;
	}
	return printed;
}



/*
 * list[] and mult[] must be > 16 in size
 */
static int collect_slays(const char *desc[], int mult[], u32b f1)
{
	int cnt = 0;
	const slay_t *s_ptr;

	/* Collect slays */
	for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++)
	{
		if (f1 & s_ptr->slay_flag)
		{
			mult[cnt] = s_ptr->mult; desc[cnt++] = s_ptr->desc;
		}
	}

	return cnt;
}



/*
 * Account for criticals in the calculation of melee prowess
 *
 * Note -- This relies on the criticals being an affine function
 * of previous damage, since we are used to transform the mean
 * of a roll.
 *
 * Also note -- rounding error makes this not completely accurate
 * (but for the big crit weapons like Grond an odd point of damage
 * won't be missed)
 *
 * This code written according to the KISS principle.  650 adds
 * are cheaper than a FOV call and get the job done fine.
 */
static void calculate_melee_crits(player_state *state, int weight,
		int plus, int *mult, int *add, int *div)
{
	int k, to_crit = weight + 5*(state->to_h + plus) + 3*p_ptr->lev;
	to_crit = MIN(5000, MAX(0, to_crit));

	*mult = *add = 0;

	for (k = weight; k < weight + 650; k++)
	{
		if (k <  400) { *mult += 4; *add += 10; continue; }
		if (k <  700) { *mult += 4; *add += 20; continue; }
		if (k <  900) { *mult += 6; *add += 30; continue; }
		if (k < 1300) { *mult += 6; *add += 40; continue; }
		                *mult += 7; *add += 50;
	}

	/*
	 * Scale the output down to a more reasonable size, to prevent
	 * integer overflow downstream.
	 */
	*mult = 100 + to_crit*(*mult - 1300)/(50*1300);
	*add  = *add * to_crit / (500*50);
	*div  = 100;
}

/*
 * Missile crits follow the same approach as melee crits.
 */
static void calculate_missile_crits(player_state *state, int weight,
		int plus, int *mult, int *add, int *div)
{
	int k, to_crit = weight + 4*(state->to_h + plus) + 2*p_ptr->lev;
	to_crit = MIN(5000, MAX(0, to_crit));

	*mult = *add = 0;

	for (k = weight; k < weight + 500; k++)
	{
		if (k <  500) { *mult += 2; *add +=  5; continue; }
		if (k < 1000) { *mult += 2; *add += 10; continue; }
		                *mult += 3; *add += 15;
	}

	*mult = 100 + to_crit*(*mult - 500)/(500*50);
	*add  = *add * to_crit / (500*50);
	*div  = 100;
}

/*
 * Describe combat advantages.
 */
static bool describe_combat(const object_type *o_ptr, bool full)
{
	const char *desc[16];
	int i;
	int mult[16];
	int cnt, dam, total_dam, plus = 0;
	int xtra_postcrit = 0, xtra_precrit = 0;
	int crit_mult, crit_div, crit_add;
	object_type *j_ptr = &inventory[INVEN_BOW];

	u32b f1, f2, f3;

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->state.ammo_tval == o_ptr->tval) &&
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
		/*
		 * Get the player's hypothetical state, were they to be
		 * wielding this item.
		 */
		player_state state;
		object_type inven[INVEN_TOTAL];

		memcpy(inven, inventory, INVEN_TOTAL * sizeof(object_type));
		inven[INVEN_WIELD] = *o_ptr;

		calc_bonuses(inven, &state, TRUE);

		dam = ((o_ptr->ds + 1) * o_ptr->dd * 5);

		xtra_postcrit = state.dis_to_d * 10;
		if (object_known_p(o_ptr)) xtra_precrit += o_ptr->to_d * 10;
		if (object_known_p(o_ptr)) plus += o_ptr->to_h;

		calculate_melee_crits(&state, o_ptr->weight, plus,
				&crit_mult, &crit_add, &crit_div);

		/* Warn about heavy weapons */
		if (adj_str_hold[state.stat_ind[A_STR]] < o_ptr->weight / 10)
			text_out_c(TERM_L_RED, "You are too weak to use this weapon effectively!\n");

		text_out("With this weapon, you would currently get ");
		text_out_c(TERM_L_GREEN, format("%d ", state.num_blow));
		if (state.num_blow > 1)
			text_out("blows per round.  Each blow will do an average damage of ");
		else
			text_out("blow per round, averaging a damage of ");
	}
	else
	{
		int tdis = 6 + 2 * p_ptr->state.ammo_mult;
		u32b f[3];

		if (object_known_p(o_ptr)) plus += o_ptr->to_h;

		calculate_missile_crits(&p_ptr->state, o_ptr->weight, plus,
				&crit_mult, &crit_add, &crit_div);

		/* Calculate damage */
		dam = ((o_ptr->ds + 1) * o_ptr->dd * 5);
		if (object_known_p(o_ptr)) dam += (o_ptr->to_d * 10);
		if (object_known_p(j_ptr)) dam += (j_ptr->to_d * 10);
		dam *= p_ptr->state.ammo_mult;

		/* Apply brands from the shooter to the ammo */
		object_flags(j_ptr, &f[0], &f[1], &f[2]);
		f1 |= f[0];

		text_out("Fired from your current missile launcher, this arrow will hit targets up to ");
		text_out_c(TERM_L_GREEN, format("%d", tdis * 10));
		text_out(" feet away, inflicting an average damage of ");
	}

	/* Collect slays */
	/* Melee weapons get slays from rings now */
	if (weapon)
	{
		u32b g1, g2, g3, h1, h2, h3;
		
		object_flags_known(&inventory[INVEN_LEFT], &g1, &g2, &g3);
		object_flags_known(&inventory[INVEN_RIGHT], &h1, &h2, &h3);

		f1 |= (g1 | h1);
	}
	
	
	cnt = collect_slays(desc, mult, f1);
	for (i = 0; i < cnt; i++)
	{
		/* Include bonus damage and slay in stated average */
		total_dam = dam * mult[i] + xtra_precrit;
		total_dam = (total_dam * crit_mult + crit_add) / crit_div;
		total_dam += xtra_postcrit;

		if (total_dam <= 0)
			text_out_c(TERM_L_RED, "%d", 0);
		else if (total_dam % 10)
			text_out_c(TERM_L_GREEN, "%d.%d",
			           total_dam / 10, total_dam % 10);
		else
			text_out_c(TERM_L_GREEN, "%d", total_dam / 10);


		text_out(" against %s, ", desc[i]);
	}

	if (cnt) text_out("and ");


	/* Include bonus damage in stated average */
	total_dam = dam + xtra_precrit;
	total_dam = (total_dam * crit_mult + crit_add) / crit_div +
		xtra_postcrit;

	if (total_dam <= 0)
		text_out_c(TERM_L_RED, "%d", 0);
	else if (total_dam % 10)
		text_out_c(TERM_L_GREEN, "%d.%d",
				total_dam / 10, total_dam % 10);
	else
		text_out_c(TERM_L_GREEN, "%d", total_dam / 10);

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
 * Describe objects that can be used for digging.
 */
static bool describe_digger(const object_type *o_ptr, bool full)
{
	player_state st;

	object_type inven[INVEN_TOTAL];

	int sl = wield_slot(o_ptr);
	int i;
	u32b f1, f2, f3;

	int chances[4]; /* These are out of 1600 */
	static const char *names[4] = {
		"rubble", "magma veins", "quartz veins", "granite"
	};

	if (full)
		object_flags(o_ptr, &f1, &f2, &f3);
	else
		object_flags_known(o_ptr, &f1, &f2, &f3);

	if (sl < 0 || ((sl != INVEN_WIELD) && !(f1 & TR1_TUNNEL)))
		return FALSE;

	memcpy(inven, inventory, INVEN_TOTAL * sizeof(object_type));

	/*
	 * Hack -- if we examine a ring that is worn on the right finger,
	 * we shouldn't put a copy of it on the left finger before calculating
	 * digging skills.
	 */
	if (o_ptr != &inventory[INVEN_RIGHT])
		inven[sl] = *o_ptr;

	calc_bonuses(inven, &st, TRUE);

	chances[0] = st.skills[SKILL_DIGGING] * 8;
	chances[1] = (st.skills[SKILL_DIGGING] - 10) * 4;
	chances[2] = (st.skills[SKILL_DIGGING] - 20) * 2;
	chances[3] = (st.skills[SKILL_DIGGING] - 40) * 1;

	text_out("\nWith this item, you can expect to ");

	for (i = 0; i < 4; i++) {
		int chance = MAX(0, MIN(1600, chances[i]));
		int decis = chance ? (16000 / chance) : 0;

		if (i == 0 && chance > 0)
			text_out("clear ");
		if (i == 3 || (i != 0 && chance == 0))
			text_out("and ");

		if (chance == 0) {
			text_out_c(TERM_L_RED, "not affect ");
			text_out("%s.\n", names[i]);
			break;
		}

		text_out("%s in ", names[i]);

		if (chance == 1600) {
			text_out_c(TERM_L_GREEN, "1 ");
		} else if (decis < 100) {
			text_out_c(TERM_GREEN, "%d.%d ", decis/10, decis%10);
		} else {
			text_out_c((decis < 1000) ? TERM_YELLOW : TERM_RED,
			           "%d ", (decis+5)/10);
		}

		text_out("turn%s%s", decis == 10 ? "" : "s",
				(i == 3) ? ".\n" : ", ");
	}

	return TRUE;
}


/*
 * Describe things that look like lights.
 */
static bool describe_light(const object_type *o_ptr, u32b f3, bool terse)
{
	int rad = 0;

	bool artifact = artifact_p(o_ptr);
	bool no_fuel = (f3 & TR3_NO_FUEL) ? TRUE : FALSE;
	bool is_lite = (o_ptr->tval == TV_LITE) ? TRUE : FALSE;

	if ((o_ptr->tval != TV_LITE) && !(f3 & TR3_LITE))
		return FALSE;

	/* Work out radius */
	if (artifact && is_lite) rad = 3;
	else if (is_lite)  rad = 2;
	if (f3 & TR3_LITE) rad++;

	/* Describe here */
	text_out("Radius ");
	text_out_c(TERM_L_GREEN, format("%d", rad));
	if (no_fuel && !artifact)
		text_out(" light.  No fuel required");
	else if (is_lite && o_ptr->sval == SV_LITE_TORCH)
		text_out(" light, reduced when running out of fuel");
	else
		text_out (" light");
	text_out(".");

	if (!terse && is_lite && !artifact)
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
static bool describe_activation(const object_type *o_ptr, u32b f3, bool full, bool all)
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

	if (all == FALSE && !(f3 & TR3_ACTIVATE)) return FALSE;

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
		int min_time, max_time;
		/* Some artifacts can be activated */
		text_out("It takes ");

		/* Correct for player speed */

		min_time = (dice*1     + base) *
			extract_energy[p_ptr->state.speed] / 10;
		max_time = (dice*sides + base) *
			extract_energy[p_ptr->state.speed] / 10;

		text_out_c(TERM_L_GREEN, "%d", min_time);

		if (min_time != max_time)
		{
			text_out(" to ");
			text_out_c(TERM_L_GREEN, "%d", max_time);
		}

		text_out(" turns to recharge after use%s.\n",
			p_ptr->state.speed == 110 ? "" :
			" at your current speed");
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
	if (describe_slays(f1)) something = TRUE;
	if (describe_immune(f2)) something = TRUE;
	if (describe_ignores(f3)) something = TRUE;
	if (describe_sustains(f2)) something = TRUE;
	if (describe_misc_magic(f3)) something = TRUE;

	if (something) text_out("\n");

	if (describe_activation(o_ptr, f3, full, TRUE)) something = TRUE;
	if (describe_light(o_ptr, f3, FALSE)) something = TRUE;

	return something;
}


/*
 * Print name, origin, and descriptive text for a given object.
 */
void object_info_header(const object_type *o_ptr)
{
	char o_name[120];

	/* Object name */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, ODESC_FULL);
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
			         o_ptr->origin_depth * 50,
			         o_ptr->origin_depth);
 			break;

		case ORIGIN_DROP:
		{
			const char *name = r_name + r_info[o_ptr->origin_xtra].name;
			bool unique = (r_info[o_ptr->origin_xtra].flags[0] & RF0_UNIQUE) ? TRUE : FALSE;

			text_out("dropped by ");

			if (unique)
				text_out("%s", name);
			else
				text_out("%s%s", is_a_vowel(name[0]) ? "an " : "a ", name);

			text_out(" at %d feet (level %d)\n",
			         o_ptr->origin_depth * 50,
			         o_ptr->origin_depth);

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
			         o_ptr->origin_depth * 50,
			         o_ptr->origin_depth);
			break;
	}

	text_out("\n");

	/* Display the known artifact description */
	if (!OPT(adult_randarts) && o_ptr->name1 &&
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


bool object_info_chardump(const object_type *o_ptr)
{
	u32b f1, f2, f3;
	bool something = FALSE;

	/* Grab the object flags */
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

	if (describe_activation(o_ptr, f3, FALSE, FALSE)) something = TRUE;
	if (describe_light(o_ptr, f3, TRUE)) something = TRUE;

	/* Describe combat bits */
	if (describe_combat(o_ptr, FALSE)) something = TRUE;
	if (describe_digger(o_ptr, FALSE)) something = TRUE;

	return something;
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
		text_out("You do not know the full extent of this item's powers.\n");
		has_info = TRUE;
	}

	/* Describe combat bits */
	if (describe_combat(o_ptr, FALSE)) has_info = TRUE;
	if (describe_digger(o_ptr, FALSE)) has_info = TRUE;

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
	if (describe_digger(o_ptr, TRUE)) has_info = TRUE;

	return has_info;
}
