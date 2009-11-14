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
	{ TR0_STR,     "strength" },
	{ TR0_INT,     "intelligence" },
	{ TR0_WIS,     "wisdom" },
	{ TR0_DEX,     "dexterity" },
	{ TR0_CON,     "constitution" },
	{ TR0_CHR,     "charisma" },
	{ TR0_STEALTH, "stealth" },
	{ TR0_INFRA,   "infravision" },
	{ TR0_TUNNEL,  "tunneling" },
	{ TR0_SPEED,   "speed" },
	{ TR0_BLOWS,   "attack speed" },
	{ TR0_SHOTS,   "shooting speed" },
	{ TR0_MIGHT,   "shooting power" },
};

static const flag_type f2_immunity[] =
{
	{ TR1_IM_ACID, "acid" },
	{ TR1_IM_ELEC, "lightning" },
	{ TR1_IM_FIRE, "fire" },
	{ TR1_IM_COLD, "cold" },
};

static const flag_type f2_vuln[] =
{
	{ TR1_VULN_ACID, "acid" },
	{ TR1_VULN_ELEC, "electricity" },
	{ TR1_VULN_FIRE, "fire" },
	{ TR1_VULN_COLD, "cold" },
};

static const flag_type f2_resist[] =
{
	{ TR1_RES_ACID,  "acid" },
	{ TR1_RES_ELEC,  "lightning" },
	{ TR1_RES_FIRE,  "fire" },
	{ TR1_RES_COLD,  "cold" },
	{ TR1_RES_POIS,  "poison" },
	{ TR1_RES_FEAR,  "fear" },
	{ TR1_RES_LITE,  "light" },
	{ TR1_RES_DARK,  "dark" },
	{ TR1_RES_BLIND, "blindness" },
	{ TR1_RES_CONFU, "confusion" },
	{ TR1_RES_SOUND, "sound" },
	{ TR1_RES_SHARD, "shards" },
	{ TR1_RES_NEXUS, "nexus"  },
	{ TR1_RES_NETHR, "nether" },
	{ TR1_RES_CHAOS, "chaos" },
	{ TR1_RES_DISEN, "disenchantment" },
};

static const flag_type f3_ignore[] =
{
	{ TR2_IGNORE_ACID, "acid" },
	{ TR2_IGNORE_ELEC, "electricity" },
	{ TR2_IGNORE_FIRE, "fire" },
	{ TR2_IGNORE_COLD, "cold" },
};

static const flag_type f2_sustains[] =
{
	{ TR1_SUST_STR, "strength" },
	{ TR1_SUST_INT, "intelligence" },
	{ TR1_SUST_WIS, "wisdom" },
	{ TR1_SUST_DEX, "dexterity" },
	{ TR1_SUST_CON, "constitution" },
	{ TR1_SUST_CHR, "charisma" },
};

static const flag_type f3_misc[] =
{
	{ TR2_BLESSED, "Blessed by the gods" },
	{ TR2_SLOW_DIGEST, "Slows your metabolism" },
	{ TR2_IMPAIR_HP, "Impairs hitpoint recovery" },
	{ TR2_IMPAIR_MANA, "Impairs mana recovery" },
	{ TR2_AFRAID, "Makes you afraid of melee, and worse at shooting and casting spells" },
	{ TR2_FEATHER, "Feather Falling" },
	{ TR2_REGEN, "Speeds regeneration" },
	{ TR2_FREE_ACT, "Prevents paralysis" },
	{ TR2_HOLD_LIFE, "Sustains your life force" },
	{ TR2_TELEPATHY, "Grants telepathy" },
	{ TR2_SEE_INVIS, "Grants the ability to see invisible things" },
	{ TR2_AGGRAVATE, "Aggravates creatures nearby" },
	{ TR2_DRAIN_EXP, "Drains experience" },
	{ TR2_TELEPORT, "Induces random teleportation" },
};


/** Slays **/
/*
 * Entries in this table should be in ascending order of multiplier, to 
 * ensure that the highest one takes precedence 
 * object flag, vulnerable flag, resist flag, multiplier, ranged verb, 
 * melee verb, verb describing what the thing branded does when it is active,
 * description of affected creatures, brand
 */
const slay_t slay_table[] =
{
	{ TR0_SLAY_ANIMAL, RF2_ANIMAL, 0, 2, "pierces",  "smite", NULL,
	  "animals",                                NULL },
	{ TR0_SLAY_EVIL,   RF2_EVIL,   0, 2, "pierces",  "smite", NULL,
	  "evil creatures",                         NULL },
	{ TR0_SLAY_UNDEAD, RF2_UNDEAD, 0, 3, "pierces",  "smite", NULL,
	  "undead",                                 NULL },
	{ TR0_SLAY_DEMON,  RF2_DEMON,  0, 3, "pierces",  "smite", NULL,
	  "demons",                                 NULL },
	{ TR0_SLAY_ORC,    RF2_ORC,    0, 3, "pierces",  "smite", NULL,
	  "orcs",                                   NULL },
	{ TR0_SLAY_TROLL,  RF2_TROLL,  0, 3, "pierces",  "smite", NULL,
	  "trolls",                                 NULL },
	{ TR0_SLAY_GIANT,  RF2_GIANT,  0, 3, "pierces",  "smite", NULL,
	 "giants",                                  NULL },
	{ TR0_SLAY_DRAGON, RF2_DRAGON, 0, 3, "pierces",  "smite", NULL,
	  "dragons",                                NULL },
	{ TR0_BRAND_ACID, 0, RF2_IM_ACID, 3, "corrodes", "corrode", "spits",
	  "creatures not resistant to acid",        "acid" },
	{ TR0_BRAND_ELEC, 0, RF2_IM_ELEC, 3, "zaps",     "zap", "crackles",
	  "creatures not resistant to electricity", "lightning" },
	{ TR0_BRAND_FIRE, 0, RF2_IM_FIRE, 3, "burns",    "burn", "flares",
	  "creatures not resistant to fire",        "flames" },
	{ TR0_BRAND_COLD, 0, RF2_IM_COLD, 3, "freezes",  "freeze", "grows cold",
	  "creatures not resistant to cold",        "frost" },
	{ TR0_BRAND_POIS, 0, RF2_IM_POIS, 3, "poisons",  "poison", "seethes",
	  "creatures not resistant to poison",      "venom" },
	{ TR0_KILL_DRAGON, RF2_DRAGON, 0, 5, "deeply pierces",
	 "fiercely smite", NULL, "dragons",         NULL },
	{ TR0_KILL_DEMON,  RF2_DEMON,  0, 5, "deeply pierces",
	 "fiercely smite", NULL, "demons",          NULL },
	{ TR0_KILL_UNDEAD, RF2_UNDEAD, 0, 5, "deeply pierces",
	 "fiercely smite", NULL, "undead",          NULL },
	{ 0, 0, 0, 0, NULL, NULL, NULL, NULL }
};

/*
 * Helper function to externalise N_ELEMENTS(slay_table), which itself is not
 * available outside this compilation unit
 */
size_t num_slays(void)
{
	return N_ELEMENTS(slay_table);
}

/*** Code that makes use of the data tables ***/

/*
 * Describe an item's curses.
 */
static bool describe_curses(const object_type *o_ptr, u32b f3)
{
	if (cursed_p(o_ptr))
	{
		if (f3 & TR2_PERMA_CURSE)
			text_out_c(TERM_L_RED, "Permanently cursed.\n");
		else if (f3 & TR2_HEAVY_CURSE)
			text_out_c(TERM_L_RED, "Heavily cursed.\n");
		else if (object_is_known(o_ptr))
			text_out_c(TERM_L_RED, "Cursed.\n");
		else
			return FALSE;

		return TRUE;
	}

	return FALSE;
}


/*
 * Describe stat modifications.
 */
static bool describe_stats(const object_type *o_ptr, u32b f1, bool full)
{
	cptr descs[N_ELEMENTS(f1_pval)];
	size_t count;

	if (!o_ptr->pval) return FALSE;

	count = info_collect(f1_pval, N_ELEMENTS(f1_pval), f1, descs);
	if (count)
	{
		if (object_pval_is_visible(o_ptr) || full)
		{
			text_out_c((o_ptr->pval > 0) ? TERM_L_GREEN : TERM_RED,
				 "%+i ", o_ptr->pval);
			info_out_list(descs, count);

		}
		else
		{
			text_out("Affects your ");
			info_out_list(descs, count);
		}
	}

	if (f1 & TR0_SEARCH)
	{
		if (object_pval_is_visible(o_ptr) || full)
		{
			text_out_c((o_ptr->pval > 0) ? TERM_L_GREEN : TERM_RED,
				"%+i%% ", o_ptr->pval * 5);
			text_out("to searching.\n");
		}
		else if (count) text_out("Also affects your searching skill.\n");
		else text_out("Affects your searching skill.\n");
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
			text_out("%s.  ", f3_misc[i].name);
			printed = TRUE;
		}
	}

	if (printed) text_out("\n");

	return printed;
}


/*
 * Describe slays and brands on weapons
 */
static bool describe_slays(u32b f1, int tval)
{
	bool printed = FALSE;

	const char *slay_descs[N_ELEMENTS(slay_table)];
	const char *kill_descs[N_ELEMENTS(slay_table)];
	const char *brand_descs[N_ELEMENTS(slay_table)];
	const slay_t *s_ptr;

	size_t x = 0;
	size_t y = 0;
	size_t z = 0;

	bool fulldesc;

	if ((tval == TV_SWORD) || (tval == TV_HAFTED) || (tval == TV_POLEARM)
		|| (tval == TV_DIGGING ) || (tval == TV_BOW) || (tval == TV_SHOT)
		|| (tval == TV_ARROW) || (tval == TV_BOLT) || (tval == TV_FLASK))
		fulldesc = FALSE;		
	else fulldesc = TRUE;

	for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++)
	{
		if (f1 & (s_ptr->slay_flag & TR0_SLAY_MASK))
			slay_descs[x++] = s_ptr->desc;
		else if (f1 & (s_ptr->slay_flag & TR0_KILL_MASK))
			kill_descs[y++] = s_ptr->desc;
		else if (f1 & (s_ptr->slay_flag & TR0_BRAND_MASK))
			brand_descs[z++] = s_ptr->brand;
	}

	/* Slays */
	if (x)
	{
		if (fulldesc) text_out("It causes your melee attacks to slay ");
		else text_out("Slays ");
		info_out_list(slay_descs, x);
		printed = TRUE;
	}

	/* Kills */
	if (y)
	{
		if (fulldesc) text_out("It causes your melee attacks to *slay* ");
		else text_out("*Slays* ");
		info_out_list(kill_descs, y);
		printed = TRUE;
	}

	/* Brands */
	if (z)
	{
		if (fulldesc) text_out("It brands your melee attacks with ");
		else text_out("Branded with ");
		info_out_list(brand_descs, z);
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
			mult[cnt] = s_ptr->mult;
			desc[cnt++] = s_ptr->desc;
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
	int str_plus, dex_plus, old_blows, new_blows, extra_blows;
	int str_done = -1;
	object_type *j_ptr = &inventory[INVEN_BOW];

	u32b f[OBJ_FLAG_N];
	u32b flags[OBJ_FLAG_N];

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->state.ammo_tval == o_ptr->tval) &&
	              (j_ptr->k_idx);

	/* Abort if we've nothing to say */
	if (!weapon && !ammo)
	{
		/* Potions can have special text */
		if (o_ptr->tval != TV_POTION) return FALSE;
		if (!o_ptr->dd || !o_ptr->ds) return FALSE;
		if (!object_flavor_is_aware(o_ptr)) return FALSE;

		text_out("It can be thrown at creatures with damaging effect.\n");
		return TRUE;
	}

	if (full)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	text_out_c(TERM_L_WHITE, "Combat info:\n");

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
		if (object_attack_plusses_are_visible(o_ptr))
		{
			xtra_precrit += o_ptr->to_d * 10;
			plus += o_ptr->to_h;
		}

		calculate_melee_crits(&state, o_ptr->weight, plus,
				&crit_mult, &crit_add, &crit_div);

		/* Warn about heavy weapons */
		if (adj_str_hold[state.stat_ind[A_STR]] < o_ptr->weight / 10)
			text_out_c(TERM_L_RED, "You are too weak to use this weapon.\n");

		text_out_c(TERM_L_GREEN, "%d ", state.num_blow);
		text_out("blow%s/round.\n", (state.num_blow > 1) ? "s" : "");

		/* Check to see if extra STR or DEX would yield extra blows */
		old_blows = state.num_blow;
		extra_blows = 0;

		/* First we need to look for extra blows on items, as state
		 * does not track these */
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			object_flags_known(&inventory[i], flags);
			
			if (flags[0] & TR0_BLOWS)
				extra_blows += inventory[i].pval;
		}

		/* Then we check for extra native blows */
		for (dex_plus = 0; dex_plus < 8; dex_plus++)
		{
			for (str_plus = 0; str_plus < 8; str_plus++)
		        {
				state.stat_ind[A_STR] += str_plus;
				state.stat_ind[A_DEX] += dex_plus;
				new_blows = calc_blows(o_ptr, &state)
					+ extra_blows;

				/* Test to make sure that this extra blow is a
				 * new str/dex combination, not a repeat
				 */
				if ((new_blows > old_blows) &&
					((str_plus < str_done) || 
					(str_done == -1)))
				{
					text_out("With an additional %d strength and %d dex you would get %d blows\n",
						str_plus, dex_plus, new_blows);
					state.stat_ind[A_STR] -= str_plus;
					state.stat_ind[A_DEX] -= dex_plus;
					str_done = str_plus;
					break;
				}
				state.stat_ind[A_STR] -= str_plus;
				state.stat_ind[A_DEX] -= dex_plus;
			}
		}
	}
	else
	{
		int tdis = 6 + 2 * p_ptr->state.ammo_mult;
		u32b g[OBJ_FLAG_N];

		if (object_attack_plusses_are_visible(o_ptr))
			plus += o_ptr->to_h;

		calculate_missile_crits(&p_ptr->state, o_ptr->weight, plus,
				&crit_mult, &crit_add, &crit_div);

		/* Calculate damage */
		dam = ((o_ptr->ds + 1) * o_ptr->dd * 5);

		if (object_attack_plusses_are_visible(o_ptr))
			dam += (o_ptr->to_d * 10);
		if (object_attack_plusses_are_visible(j_ptr))
			dam += (j_ptr->to_d * 10);
		dam *= p_ptr->state.ammo_mult;

		/* Apply brands from the shooter to the ammo */
		object_flags(j_ptr, g);
		f[0] |= g[0];

		text_out("Hits targets up to ");
		text_out_c(TERM_L_GREEN, format("%d", tdis * 10));
		text_out(" feet away.\n");
	}

	/* Collect slays */
	/* Melee weapons get slays and brands from other items now */
	if (weapon)
	{
		u32b g[OBJ_FLAG_N];
		bool nonweap = FALSE;

		for (i = INVEN_LEFT; i < INVEN_TOTAL; i++)
		{
			object_flags_known(&inventory[i], g);

			if ((f[0] & TR0_ALL_SLAYS) !=
			((f[0] | g[0]) & TR0_ALL_SLAYS))
			{
				f[0] |= g[0];
				nonweap = TRUE;
			}
		}

		if (nonweap)
			text_out("This weapon benefits from one or more off-weapon brands or slays.\n");
	}

	text_out("Average damage/hit: ");
	
	cnt = collect_slays(desc, mult, f[0]);
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


		text_out(" vs. %s, ", desc[i]);
	}

	if (cnt) text_out("and ");

	/* Include bonus damage in stated average */
	total_dam = dam + xtra_precrit;
	total_dam = (total_dam * crit_mult + crit_add) / crit_div;
	total_dam += xtra_postcrit;

	if (total_dam <= 0)
		text_out_c(TERM_L_RED, "%d", 0);
	else if (total_dam % 10)
		text_out_c(TERM_L_GREEN, "%d.%d",
				total_dam / 10, total_dam % 10);
	else
		text_out_c(TERM_L_GREEN, "%d", total_dam / 10);

	if (cnt) text_out(" vs. others");
	text_out(".\n");
	
	/* Note the impact flag */
	if (f[2] & TR2_IMPACT)
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

	u32b f[OBJ_FLAG_N];

	int chances[4]; /* These are out of 1600 */
	static const char *names[4] = { "rubble", "magma veins", "quartz veins", "granite" };

	if (full)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	if (sl < 0 || ((sl != INVEN_WIELD) && !(f[0] & TR0_TUNNEL)))
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

	for (i = 0; i < 4; i++)
	{
		int chance = MAX(0, MIN(1600, chances[i]));
		int decis = chance ? (16000 / chance) : 0;

		if (i == 0 && chance > 0)
		{
			if (sl == INVEN_WIELD) text_out("Clears ");
			else text_out("With this item, your current weapon clears ");
		}

		if (i == 3 || (i != 0 && chance == 0))
			text_out("and ");

		if (chance == 0)
		{
			text_out_c(TERM_L_RED, "doesn't affect ");
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


static bool describe_food(const object_type *o_ptr, bool subjective, bool full)
{
	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->pval)
	{
		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		if (object_pval_is_visible(o_ptr) || full)
		{
			text_out("Nourishes for around ");
			text_out_c(TERM_L_GREEN, "%d", (o_ptr->pval / 2) *
				multiplier / 10);
			text_out(" turns.\n");
		}
		else text_out("Provides some nourishment.\n");
		
		return TRUE;
	}

	return FALSE;
}


/*
 * Describe things that look like lights.
 */
static bool describe_light(const object_type *o_ptr, u32b f3, bool terse)
{
	int rad = 0;

	bool artifact = artifact_p(o_ptr);
	bool no_fuel = (f3 & TR2_NO_FUEL) ? TRUE : FALSE;
	bool is_lite = (o_ptr->tval == TV_LITE) ? TRUE : FALSE;

	if ((o_ptr->tval != TV_LITE) && !(f3 & TR2_LITE))
		return FALSE;

	/* Work out radius */
	if (artifact && is_lite) rad = 3;
	else if (is_lite) rad = 2;
	if (f3 & TR2_LITE) rad++;

	/* Describe here */
	text_out("Radius ");
	text_out_c(TERM_L_GREEN, format("%d", rad));
	if (no_fuel && !artifact)
		text_out(" light.  No fuel required.");
	else if (is_lite && o_ptr->sval == SV_LITE_TORCH)
		text_out(" light, reduced when running out of fuel.");
	else
		text_out (" light.");

	if (!terse && is_lite && !no_fuel)
	{
		const char *name = (o_ptr->sval == SV_LITE_TORCH) ? "torches" : "lanterns";
		int turns = (o_ptr->sval == SV_LITE_TORCH) ? FUEL_TORCH : FUEL_LAMP;

		text_out("  Refills other %s up to %d turns of fuel.", name, turns);
	}

	text_out("\n");

	return TRUE;
}



/*
 * Describe an object's effect, if any.
 */
static bool describe_effect(const object_type *o_ptr, u32b f3, bool full,
		bool only_artifacts, bool subjective)
{
	const object_kind *k_ptr = &k_info[o_ptr->k_idx];
	const char *desc;

	int effect = 0, base, dice, sides;

	if (o_ptr->name1)
	{
		const artifact_type *a_ptr = &a_info[o_ptr->name1];

		if (object_effect_is_known(o_ptr) || full)
		{
			effect = a_ptr->effect;
			base = a_ptr->time_base;
			dice = a_ptr->time_dice;
			sides = a_ptr->time_sides;
		}
		else if (object_effect(o_ptr))
		{
			text_out("It can be activated.\n");
			return TRUE;
		}
	}
	else
	{
		/* Sometimes only print artifact activation info */
		if (only_artifacts == TRUE) return FALSE;

		if (object_effect_is_known(o_ptr) || full)
		{
			effect = k_ptr->effect;
			base = k_ptr->time_base;
			dice = k_ptr->time_dice;
			sides = k_ptr->time_sides;
		}
		else if (object_effect(o_ptr) != 0)
		{
			text_out("It can be activated.\n");
			return TRUE;
		}
	}

	/* Forget it without an effect */
	if (!effect) return FALSE;

	/* Obtain the description */
	desc = effect_desc(effect);
	if (!desc) return FALSE;

	if (effect_aim(effect))
		text_out("When aimed, it ");
	else if (o_ptr->tval == TV_FOOD)
		text_out("When eaten, it ");
	else if (o_ptr->tval == TV_POTION)
		text_out("When drunk, it ");
	else if (o_ptr->tval == TV_SCROLL)
	    text_out("When read, it ");
	else
	    text_out("When activated, it ");

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

		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		text_out("Takes ");

		/* Correct for player speed */
		min_time = (dice*1     + base) * multiplier / 10;
		max_time = (dice*sides + base) * multiplier / 10;

		text_out_c(TERM_L_GREEN, "%d", min_time);

		if (min_time != max_time)
		{
			text_out(" to ");
			text_out_c(TERM_L_GREEN, "%d", max_time);
		}

		text_out(" turns to recharge");
		if (subjective && p_ptr->state.speed != 110)
			text_out(" at your current speed");

		text_out(".\n");
	}

	return TRUE;
}


/*** Different ways to present the data ***/


/*
 * Print name, origin, and descriptive text for a given object.
 */
void object_info_header(const object_type *o_ptr)
{
	char o_name[120];

	/* Object name */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);
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
			text_out("(from a store)\n");
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
	    object_is_known(o_ptr) && a_info[o_ptr->name1].text)
	{
		text_out(a_text + a_info[o_ptr->name1].text);
		text_out("\n\n");
	}

	/* Display the known object description */
	else if (object_flavor_is_aware(o_ptr) || object_is_known(o_ptr))
	{
		bool did_desc = FALSE;

		if (k_info[o_ptr->k_idx].text)
		{
			text_out(k_text + k_info[o_ptr->k_idx].text);
			did_desc = TRUE;
		}

		/* Display an additional ego-item description */
		if (object_ego_is_visible(o_ptr) && e_info[o_ptr->name2].text)
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




/*
 * Output object information
 */
static bool object_info_out(const object_type *o_ptr, bool full, bool terse, bool subjective)
{
	u32b f[OBJ_FLAG_N];
	bool something = FALSE;
	bool known = object_is_known(o_ptr);
	
	/* Grab the object flags */
	if (full)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	if (!full && !known)
	{
		text_out("You do not know the full extent of this item's powers.\n");
		something = TRUE;
	}	
	
	if (describe_curses(o_ptr, f[2])) something = TRUE;
	if (describe_stats(o_ptr, f[0], full)) something = TRUE;
	if (describe_slays(f[0], o_ptr->tval)) something = TRUE;
	if (describe_immune(f[1])) something = TRUE;
	if (describe_ignores(f[2])) something = TRUE;
	if (describe_sustains(f[1])) something = TRUE;
	if (describe_misc_magic(f[2])) something = TRUE;
	if (something) text_out("\n");
	
	if (describe_effect(o_ptr, f[2], full, terse, subjective))
	{
		something = TRUE;
		text_out("\n");
	}

	if (subjective && describe_combat(o_ptr, full))
	{
		something = TRUE;
		text_out("\n");
	}

	if (!terse && describe_food(o_ptr, subjective, full)) something = TRUE;
	if (describe_light(o_ptr, f[2], terse)) something = TRUE;
	if (!terse && subjective && describe_digger(o_ptr, full)) something = TRUE;

	return something;
}


/**
 * Provide information on an item, including how it would affect the current player's state.
 * 
 * \param full should be set if actual player knowledge should be ignored in favour of
 *              full knowledge.
 *
 * \returns TRUE if anything is printed.
 */
bool object_info(const object_type *o_ptr, bool full)
{
	return object_info_out(o_ptr, full, FALSE, TRUE);
}


/**
 * Provide information on an item suitable for writing to the character dump - keep it brief.
 */
bool object_info_chardump(const object_type *o_ptr)
{
	return object_info_out(o_ptr, FALSE, TRUE, TRUE);
}


/**
 * Provide spoiler information on an item.
 *
 * Practically, this means that we should not print anything which relies upon the player's
 * current state, since that is not suitable for spoiler material.
 */
bool object_info_spoil(const object_type *o_ptr)
{
	return object_info_out(o_ptr, TRUE, FALSE, FALSE);
}

