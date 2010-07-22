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
	int flag;
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
static size_t info_collect(const flag_type list[], size_t max, const bitflag flags[OF_SIZE], const char *recepticle[])
{
	size_t i, count = 0;

	for (i = 0; i < max; i++)
	{
		if (of_has(flags, list[i].flag))
			recepticle[count++] = list[i].name;
	}

	return count;
}


/*** Big fat data tables ***/

static const flag_type pval_flags[] =
{
	{ OF_STR,     "strength" },
	{ OF_INT,     "intelligence" },
	{ OF_WIS,     "wisdom" },
	{ OF_DEX,     "dexterity" },
	{ OF_CON,     "constitution" },
	{ OF_CHR,     "charisma" },
	{ OF_STEALTH, "stealth" },
	{ OF_INFRA,   "infravision" },
	{ OF_TUNNEL,  "tunneling" },
	{ OF_SPEED,   "speed" },
	{ OF_BLOWS,   "attack speed" },
	{ OF_SHOTS,   "shooting speed" },
	{ OF_MIGHT,   "shooting power" },
};

static const flag_type immunity_flags[] =
{
	{ OF_IM_ACID, "acid" },
	{ OF_IM_ELEC, "lightning" },
	{ OF_IM_FIRE, "fire" },
	{ OF_IM_COLD, "cold" },
};

static const flag_type vuln_flags[] =
{
	{ OF_VULN_ACID, "acid" },
	{ OF_VULN_ELEC, "electricity" },
	{ OF_VULN_FIRE, "fire" },
	{ OF_VULN_COLD, "cold" },
};

static const flag_type resist_flags[] =
{
	{ OF_RES_ACID,  "acid" },
	{ OF_RES_ELEC,  "lightning" },
	{ OF_RES_FIRE,  "fire" },
	{ OF_RES_COLD,  "cold" },
	{ OF_RES_POIS,  "poison" },
	{ OF_RES_FEAR,  "fear" },
	{ OF_RES_LIGHT, "light" },
	{ OF_RES_DARK,  "dark" },
	{ OF_RES_BLIND, "blindness" },
	{ OF_RES_CONFU, "confusion" },
	{ OF_RES_SOUND, "sound" },
	{ OF_RES_SHARD, "shards" },
	{ OF_RES_NEXUS, "nexus"  },
	{ OF_RES_NETHR, "nether" },
	{ OF_RES_CHAOS, "chaos" },
	{ OF_RES_DISEN, "disenchantment" },
};

static const flag_type ignore_flags[] =
{
	{ OF_IGNORE_ACID, "acid" },
	{ OF_IGNORE_ELEC, "electricity" },
	{ OF_IGNORE_FIRE, "fire" },
	{ OF_IGNORE_COLD, "cold" },
};

static const flag_type sustain_flags[] =
{
	{ OF_SUST_STR, "strength" },
	{ OF_SUST_INT, "intelligence" },
	{ OF_SUST_WIS, "wisdom" },
	{ OF_SUST_DEX, "dexterity" },
	{ OF_SUST_CON, "constitution" },
	{ OF_SUST_CHR, "charisma" },
};

static const flag_type misc_flags[] =
{
	{ OF_BLESSED, "Blessed by the gods" },
	{ OF_SLOW_DIGEST, "Slows your metabolism" },
	{ OF_IMPAIR_HP, "Impairs hitpoint recovery" },
	{ OF_IMPAIR_MANA, "Impairs mana recovery" },
	{ OF_AFRAID, "Makes you afraid of melee, and worse at shooting and casting spells" },
	{ OF_FEATHER, "Feather Falling" },
	{ OF_REGEN, "Speeds regeneration" },
	{ OF_FREE_ACT, "Prevents paralysis" },
	{ OF_HOLD_LIFE, "Sustains your life force" },
	{ OF_TELEPATHY, "Grants telepathy" },
	{ OF_SEE_INVIS, "Grants the ability to see invisible things" },
	{ OF_AGGRAVATE, "Aggravates creatures nearby" },
	{ OF_DRAIN_EXP, "Drains experience" },
	{ OF_TELEPORT, "Induces random teleportation" },
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
	{ OF_SLAY_ANIMAL, RF_ANIMAL, FLAG_END,   2,
	  "pierces",  "smite",   "glows",      "animals", NULL },
	{ OF_SLAY_EVIL,   RF_EVIL,   FLAG_END,   2,
	  "pierces",  "smite",   "glows",      "evil creatures", NULL },
	{ OF_SLAY_UNDEAD, RF_UNDEAD, FLAG_END,   3,
	  "pierces",  "smite",   "glows",      "undead", NULL },
	{ OF_SLAY_DEMON,  RF_DEMON,  FLAG_END,   3,
	  "pierces",  "smite",   "glows",      "demons", NULL },
	{ OF_SLAY_ORC,    RF_ORC,    FLAG_END,   3,
	  "pierces",  "smite",   "glows",      "orcs", NULL },
	{ OF_SLAY_TROLL,  RF_TROLL,  FLAG_END,   3,
	  "pierces",  "smite",   "glows",      "trolls", NULL },
	{ OF_SLAY_GIANT,  RF_GIANT,  FLAG_END,   3,
	  "pierces",  "smite",   "glows",      "giants", NULL },
	{ OF_SLAY_DRAGON, RF_DRAGON, FLAG_END,   3,
	  "pierces",  "smite",   "glows",      "dragons", NULL },
	{ OF_BRAND_ACID,  FLAG_END,  RF_IM_ACID, 3,
	  "corrodes", "corrode", "spits",      "creatures not resistant to acid", "acid" },
	{ OF_BRAND_ELEC,  FLAG_END,  RF_IM_ELEC, 3,
	  "zaps",     "zap",     "crackles",   "creatures not resistant to electricity", "lightning" },
	{ OF_BRAND_FIRE,  FLAG_END,  RF_IM_FIRE, 3,
	  "burns",    "burn",    "flares",     "creatures not resistant to fire", "flames" },
	{ OF_BRAND_COLD,  FLAG_END,  RF_IM_COLD, 3,
	  "freezes" , "freeze",  "grows cold", "creatures not resistant to cold", "frost" },
	{ OF_BRAND_POIS,  FLAG_END,  RF_IM_POIS, 3,
	  "poisons",  "poison",  "seethes",    "creatures not resistant to poison", "venom" },
	{ OF_KILL_DRAGON, RF_DRAGON, FLAG_END,   5, 
	 "deeply pierces",  "fiercely smite", "glows brightly", "dragons", NULL },
	{ OF_KILL_DEMON,  RF_DEMON,  FLAG_END,   5,
	  "deeply pierces", "fiercely smite", "glows brightly", "demons", NULL },
	{ OF_KILL_UNDEAD, RF_UNDEAD, FLAG_END,   5,
	  "deeply pierces", "fiercely smite", "glows brightly", "undead", NULL },
	{ FLAG_END,       FLAG_END,  FLAG_END,   0, NULL, NULL, NULL, NULL, NULL }
};


/* Slays which are in some sense duplicates. *Slay* dragons supercedes slay dragons
 * for example.
 */
const struct {
	int minor;
	int major;
} slay_dups[] =
{
	{ OF_SLAY_DRAGON, OF_KILL_DRAGON },
	{ OF_SLAY_DEMON, OF_KILL_DEMON },
	{ OF_SLAY_UNDEAD, OF_KILL_UNDEAD },
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
static bool describe_curses(const object_type *o_ptr, const bitflag flags[OF_SIZE])
{
	if (of_has(flags, OF_PERMA_CURSE))
		text_out_c(TERM_L_RED, "Permanently cursed.\n");
	else if (of_has(flags, OF_HEAVY_CURSE))
		text_out_c(TERM_L_RED, "Heavily cursed.\n");
	else if (of_has(flags, OF_LIGHT_CURSE))
		text_out_c(TERM_L_RED, "Cursed.\n");
	else
		return FALSE;

	return TRUE;
}


/*
 * Describe stat modifications.
 */
static bool describe_stats(const object_type *o_ptr, const bitflag flags[OF_SIZE], oinfo_detail_t
	mode)
{
	cptr descs[N_ELEMENTS(pval_flags)];
	size_t count;
	bool full = mode & OINFO_FULL;
	bool dummy = mode & OINFO_DUMMY;

	if (!o_ptr->pval && !dummy) return FALSE;

	count = info_collect(pval_flags, N_ELEMENTS(pval_flags), flags, descs);

	if (count)
	{
		if ((object_pval_is_visible(o_ptr) || full) && !dummy)
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

	if (of_has(flags, OF_SEARCH))
	{
		if ((object_pval_is_visible(o_ptr) || full) && !dummy)
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
static bool describe_immune(const bitflag flags[OF_SIZE])
{
	const char *i_descs[N_ELEMENTS(immunity_flags)];
	const char *r_descs[N_ELEMENTS(resist_flags)];
	const char *v_descs[N_ELEMENTS(vuln_flags)];
	size_t count;

	bool prev = FALSE;

	/* Immunities */
	count = info_collect(immunity_flags, N_ELEMENTS(immunity_flags), flags, i_descs);
	if (count)
	{
		text_out("Provides immunity to ");
		info_out_list(i_descs, count);
		prev = TRUE;
	}

	/* Resistances */
	count = info_collect(resist_flags, N_ELEMENTS(resist_flags), flags, r_descs);
	if (count)
	{
		text_out("Provides resistance to ");
		info_out_list(r_descs, count);
		prev = TRUE;
	}

	/* Vulnerabilities */
	count = info_collect(vuln_flags, N_ELEMENTS(vuln_flags), flags, v_descs);
	if (count)
	{
		text_out("Makes you vulnerable to ");
		info_out_list(v_descs, count);
		prev = TRUE;
	}

	return prev;
}


/*
 * Describe 'ignores' of an object.
 */
static bool describe_ignores(const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(ignore_flags)];
	size_t count = info_collect(ignore_flags, N_ELEMENTS(ignore_flags), flags, descs);

	if (!count) return FALSE;

	text_out("Cannot be harmed by ");
	info_out_list(descs, count);

	return TRUE;
}


/*
 * Describe stat sustains.
 */
static bool describe_sustains(const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(sustain_flags)];
	size_t count = info_collect(sustain_flags, N_ELEMENTS(sustain_flags), flags, descs);

	if (!count) return FALSE;

	text_out("Sustains ");
	info_out_list(descs, count);

	return TRUE;
}


/*
 * Describe miscellaneous powers.
 */
static bool describe_misc_magic(const bitflag flags[OF_SIZE])
{
	size_t i;
	bool printed = FALSE;

	for (i = 0; i < N_ELEMENTS(misc_flags); i++)
	{
		if (of_has(flags, misc_flags[i].flag))
		{
			text_out("%s.  ", misc_flags[i].name);
			printed = TRUE;
		}
	}

	if (printed) text_out("\n");

	return printed;
}


/*
 * Describe slays and brands on weapons
 */
static bool describe_slays(const bitflag flags[OF_SIZE], int tval)
{
	bool printed = FALSE;

	const char *slay_descs[N_ELEMENTS(slay_table)];
	const char *kill_descs[N_ELEMENTS(slay_table)];
	const char *brand_descs[N_ELEMENTS(slay_table)];
	const slay_t *s_ptr;
	bitflag slay_mask[OF_SIZE], kill_mask[OF_SIZE], brand_mask[OF_SIZE];

	size_t x = 0;
	size_t y = 0;
	size_t z = 0;

	bool fulldesc;

	flags_init(slay_mask, OF_SIZE, OF_SLAY_MASK, FLAG_END);
	flags_init(kill_mask, OF_SIZE, OF_KILL_MASK, FLAG_END);
	flags_init(brand_mask, OF_SIZE, OF_BRAND_MASK, FLAG_END);

	if ((tval == TV_SWORD) || (tval == TV_HAFTED) || (tval == TV_POLEARM)
		|| (tval == TV_DIGGING ) || (tval == TV_BOW) || (tval == TV_SHOT)
		|| (tval == TV_ARROW) || (tval == TV_BOLT) || (tval == TV_FLASK))
		fulldesc = FALSE;
	else fulldesc = TRUE;

	for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++)
	{
		if (!of_has(flags, s_ptr->slay_flag)) continue;

		if (of_has(slay_mask, s_ptr->slay_flag))
			slay_descs[x++] = s_ptr->desc;
		else if (of_has(kill_mask, s_ptr->slay_flag))
			kill_descs[y++] = s_ptr->desc;
		else if (of_has(brand_mask, s_ptr->slay_flag))
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
static int collect_slays(const char *desc[], int mult[], bitflag *flags)
{
	int cnt = 0;
	int i;
	const slay_t *s_ptr;

	/* Remove "duplicate" flags e.g. *slay* and slay the same
	 * monster type
	 */
	for (i = 0; i < N_ELEMENTS(slay_dups); i++) {
		if (of_has(flags, slay_dups[i].minor) &&
			of_has(flags, slay_dups[i].major)) {
			of_off(flags, slay_dups[i].minor);
		}
	}

	/* Collect slays */
	for (s_ptr = slay_table; s_ptr->slay_flag; s_ptr++)
	{
		if (of_has(flags, s_ptr->slay_flag))
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
static bool describe_combat(const object_type *o_ptr, oinfo_detail_t mode)
{
	bool full = mode & OINFO_FULL;

	const char *desc[16];
	int i;
	int mult[16];
	int cnt, dam, total_dam, plus = 0;
	int xtra_postcrit = 0, xtra_precrit = 0;
	int crit_mult, crit_div, crit_add;
	int str_plus, dex_plus, old_blows, new_blows, extra_blows;
	int str_done = -1;
	object_type *j_ptr = &inventory[INVEN_BOW];

	bitflag f[OF_SIZE];
	bitflag tmp_f[OF_SIZE];

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->state.ammo_tval == o_ptr->tval) &&
	              (j_ptr->k_idx);
	int multiplier = 1;

	/* Abort if we've nothing to say */
	if (mode & OINFO_DUMMY) return FALSE;

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
		int dex_plus_bound;
		int str_plus_bound;

		object_type inven[INVEN_TOTAL];

		memcpy(inven, inventory, INVEN_TOTAL * sizeof(object_type));
		inven[INVEN_WIELD] = *o_ptr;

		if (full) object_know_all_flags(&inven[INVEN_WIELD]);

		calc_bonuses(inven, &state, TRUE);
		dex_plus_bound = STAT_RANGE - state.stat_ind[A_DEX];
		str_plus_bound = STAT_RANGE - state.stat_ind[A_STR];

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

		/* First we need to look for extra blows on other items, as
		 * state does not track these */
		for (i = INVEN_BOW; i < INVEN_TOTAL; i++)
		{
			object_flags_known(&inventory[i], tmp_f);

			if (of_has(tmp_f, OF_BLOWS))
				extra_blows += inventory[i].pval;
		}

		/* Then we add blows from the weapon being examined */
		if (of_has(f, OF_BLOWS)) extra_blows += o_ptr->pval;

		/* Then we check for extra "real" blows */
		for (dex_plus = 0; dex_plus < dex_plus_bound; dex_plus++)
		{
			for (str_plus = 0; str_plus < str_plus_bound; str_plus++)
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

		/* Apply brands from the shooter to the ammo */
		object_flags(j_ptr, tmp_f);
		of_union(f, tmp_f);

		text_out("Hits targets up to ");
		text_out_c(TERM_L_GREEN, format("%d", tdis * 10));
		text_out(" feet away.\n");
	}

	/* Collect slays */
	/* Melee weapons get slays and brands from other items now */
	if (weapon)
	{
		bool nonweap = FALSE;

		for (i = INVEN_LEFT; i < INVEN_TOTAL; i++)
		{
			object_flags_known(&inventory[i], tmp_f);

			flags_mask(tmp_f, OF_SIZE, OF_ALL_SLAY_MASK, FLAG_END);

			if (of_union(f, tmp_f))
				nonweap = TRUE;
		}

		if (nonweap)
			text_out("This weapon benefits from one or more off-weapon brands or slays.\n");
	}

	text_out("Average damage/hit: ");

	if (ammo) multiplier = p_ptr->state.ammo_mult;

	cnt = collect_slays(desc, mult, f);
	for (i = 0; i < cnt; i++)
	{
		/* Include bonus damage and slay in stated average */
		total_dam = dam * (multiplier + mult[i]) + xtra_precrit;
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
	total_dam = dam * multiplier + xtra_precrit;
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
	if (of_has(f, OF_IMPACT))
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
static bool describe_digger(const object_type *o_ptr, oinfo_detail_t mode)
{
	bool full = mode & OINFO_FULL;

	player_state st;

	object_type inven[INVEN_TOTAL];

	int sl = wield_slot(o_ptr);
	int i;

	bitflag f[OF_SIZE];

	int chances[4]; /* These are out of 1600 */
	static const char *names[4] = { "rubble", "magma veins", "quartz veins", "granite" };

	/* abort if we are a dummy object */
	if (mode & OINFO_DUMMY) return FALSE;

	if (full)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	if (sl < 0 || (sl != INVEN_WIELD && !of_has(f, OF_TUNNEL)))
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

		if (object_is_known(o_ptr) || full)
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
static bool describe_light(const object_type *o_ptr, const bitflag flags[OF_SIZE], bool terse)
{
	int rad = 0;

	bool artifact = artifact_p(o_ptr);
	bool no_fuel = of_has(flags, OF_NO_FUEL) ? TRUE : FALSE;
	bool is_light = (o_ptr->tval == TV_LIGHT) ? TRUE : FALSE;

	if (o_ptr->tval != TV_LIGHT && !of_has(flags, OF_LIGHT))
		return FALSE;

	/* Work out radius */
	if (artifact && is_light) rad = 3;
	else if (is_light) rad = 2;
	if (of_has(flags, OF_LIGHT)) rad++;

	/* Describe here */
	text_out("Radius ");
	text_out_c(TERM_L_GREEN, format("%d", rad));
	if (no_fuel && !artifact)
		text_out(" light.  No fuel required.");
	else if (is_light && o_ptr->sval == SV_LIGHT_TORCH)
		text_out(" light, reduced when running out of fuel.");
	else
		text_out (" light.");

	if (!terse && is_light && !no_fuel)
	{
		const char *name = (o_ptr->sval == SV_LIGHT_TORCH) ? "torches" : "lanterns";
		int turns = (o_ptr->sval == SV_LIGHT_TORCH) ? FUEL_TORCH : FUEL_LAMP;

		text_out("  Refills other %s up to %d turns of fuel.", name, turns);
	}

	text_out("\n");

	return TRUE;
}



/*
 * Describe an object's effect, if any.
 */
static bool describe_effect(const object_type *o_ptr, bool full,
		bool only_artifacts, bool subjective)
{
	const object_kind *k_ptr = &k_info[o_ptr->k_idx];
	const char *desc;
	random_value timeout = {0, 0, 0, 0};

	int effect = 0, fail;

	if (o_ptr->name1)
	{
		const artifact_type *a_ptr = &a_info[o_ptr->name1];

		if (object_effect_is_known(o_ptr) || full)
		{
			effect = a_ptr->effect;
			timeout = a_ptr->time;
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
			timeout = k_ptr->time;
		}
		else if (object_effect(o_ptr) != 0)
		{
			if (effect_aim(k_ptr->effect))
				text_out("It can be aimed.\n");
			else if (o_ptr->tval == TV_FOOD)
				text_out("It can be eaten.\n");
			else if (o_ptr->tval == TV_POTION)
				text_out("It can be drunk.\n");
			else if (o_ptr->tval == TV_SCROLL)
				text_out("It can be read.\n");
			else text_out("It can be activated.\n");

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

	if (randcalc(timeout, 0, MAXIMISE) > 0)
	{
		int min_time, max_time;

		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		text_out("Takes ");

		/* Correct for player speed */
		min_time = randcalc(timeout, 0, MINIMISE) * multiplier / 10;
		max_time = randcalc(timeout, 0, MAXIMISE) * multiplier / 10;

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

	if (!subjective || o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION ||
		o_ptr->tval == TV_SCROLL)
	{
		return TRUE;
	}
	else
	{
		fail = get_use_device_chance(o_ptr);
		text_out("Your chance of success is %d.%d%%\n", (1000 - fail) /
			10, (1000 - fail) % 10);
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
	char origin_text[80];

	/* Object name */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);
	text_out_c(TERM_L_BLUE, "%^s\n", o_name);

	/* Display the origin */
	
	if (o_ptr->origin_depth)
		strnfmt(origin_text, sizeof(origin_text), "%d feet (level %d)",
		        o_ptr->origin_depth * 50, o_ptr->origin_depth);
	else
		my_strcpy(origin_text, "town", sizeof(origin_text));

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
			text_out("(lying on the floor %s %s)\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text);
 			break;

		case ORIGIN_DROP:
		{
			const char *name = r_name + r_info[o_ptr->origin_xtra].name;
			bool unique = rf_has(r_info[o_ptr->origin_xtra].flags, RF_UNIQUE) ? TRUE : FALSE;

			text_out("(dropped by ");

			if (unique)
				text_out("%s", name);
			else
				text_out("%s%s", is_a_vowel(name[0]) ? "an " : "a ", name);

			text_out(" %s %s)\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text);
 			break;
		}

		case ORIGIN_DROP_UNKNOWN:
		{
			text_out("(dropped by an unknown monster %s %s)\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text);
			break;
		}

		case ORIGIN_ACQUIRE:
		{
			text_out("(conjured forth by magic %s %s)\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text);
 			break;
		}

		case ORIGIN_CHEAT:
			text_out("(created by debug option)\n");
 			break;

		case ORIGIN_CHEST:
		{
			text_out("(found in a chest from %s)\n",
			         origin_text);
			break;
		}
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
static bool object_info_out(const object_type *o_ptr, oinfo_detail_t mode)
{
	bitflag flags[OF_SIZE];
	bool something = FALSE;
	bool known = object_is_known(o_ptr);
	bool full = mode & OINFO_FULL;
	bool terse = mode & OINFO_TERSE;
	bool subjective = mode & OINFO_SUBJ;

	/* Grab the object flags */
	if (full)
		object_flags(o_ptr, flags);
	else
		object_flags_known(o_ptr, flags);

	if (!full && !known)
	{
		text_out("You do not know the full extent of this item's powers.\n");
		something = TRUE;
	}

	if (describe_curses(o_ptr, flags)) something = TRUE;
	if (describe_stats(o_ptr, flags, mode)) something = TRUE;
	if (describe_slays(flags, o_ptr->tval)) something = TRUE;
	if (describe_immune(flags)) something = TRUE;
	if (describe_ignores(flags)) something = TRUE;
	if (describe_sustains(flags)) something = TRUE;
	if (describe_misc_magic(flags)) something = TRUE;
	if (something) text_out("\n");

	if (describe_effect(o_ptr, full, terse, subjective))
	{
		something = TRUE;
		text_out("\n");
	}

	if (subjective && describe_combat(o_ptr, mode))
	{
		something = TRUE;
		text_out("\n");
	}

	if (!terse && describe_food(o_ptr, subjective, full)) something = TRUE;
	if (describe_light(o_ptr, flags, terse)) something = TRUE;
	if (!terse && subjective && describe_digger(o_ptr, mode)) something = TRUE;

	return something;
}


/**
 * Provide information on an item, including how it would affect the current
 * player's state.
 *
 * mode OINFO_FULL should be set if actual player knowledge should be ignored
 * in favour of full knowledge.
 *
 * returns TRUE if anything is printed.
 */
bool object_info(const object_type *o_ptr, oinfo_detail_t mode)
{
	mode |= OINFO_SUBJ;
	return object_info_out(o_ptr, mode);
}


/**
 * Provide information on an item suitable for writing to the character dump - keep it brief.
 */
bool object_info_chardump(const object_type *o_ptr)
{
	return object_info_out(o_ptr, OINFO_TERSE | OINFO_SUBJ);
}


/**
 * Provide spoiler information on an item.
 *
 * Practically, this means that we should not print anything which relies upon the player's
 * current state, since that is not suitable for spoiler material.
 */
bool object_info_spoil(const object_type *o_ptr)
{
	return object_info_out(o_ptr, OINFO_FULL);
}
