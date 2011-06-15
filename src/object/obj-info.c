/**
 * File: obj-info.c
 * Purpose: Object description code.
 *
 * Copyright (c) 2010 Andi Sidwell
 * Copyright (c) 2004 Robert Ruehlmann
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
#include "attack.h"
#include "effects.h"
#include "cmds.h"
#include "object/tvalsval.h"
#include "z-textblock.h"
#include "object/slays.h"
#include "object/pval.h"

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
static void info_out_list(textblock *tb, const char *list[], size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
	{
		textblock_append(tb, list[i]);
		if (i != (count - 1)) textblock_append(tb, ", ");
	}

	textblock_append(tb, ".\n");
}


/*
 *
 */
static size_t info_collect(textblock *tb, const flag_type list[], size_t max,
		const bitflag flags[OF_SIZE], const char *recepticle[])
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
	{ OF_RES_LIGHT, "light" },
	{ OF_RES_DARK,  "dark" },
	{ OF_RES_SOUND, "sound" },
	{ OF_RES_SHARD, "shards" },
	{ OF_RES_NEXUS, "nexus"  },
	{ OF_RES_NETHR, "nether" },
	{ OF_RES_CHAOS, "chaos" },
	{ OF_RES_DISEN, "disenchantment" },
};

static const flag_type protect_flags[] =
{
	{ OF_RES_FEAR,  "fear" },
	{ OF_RES_BLIND, "blindness" },
	{ OF_RES_CONFU, "confusion" },
	{ OF_RES_STUN,  "stunning" },
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


/*** Code that makes use of the data tables ***/

/*
 * Describe an item's curses.
 */
static bool describe_curses(textblock *tb, const object_type *o_ptr,
		const bitflag flags[OF_SIZE])
{
	if (of_has(flags, OF_PERMA_CURSE))
		textblock_append_c(tb, TERM_L_RED, "Permanently cursed.\n");
	else if (of_has(flags, OF_HEAVY_CURSE))
		textblock_append_c(tb, TERM_L_RED, "Heavily cursed.\n");
	else if (of_has(flags, OF_LIGHT_CURSE))
		textblock_append_c(tb, TERM_L_RED, "Cursed.\n");
	else
		return FALSE;

	return TRUE;
}


/*
 * Describe stat modifications.
 */
static bool describe_stats(textblock *tb, const object_type *o_ptr,
		bitflag flags[MAX_PVALS][OF_SIZE], oinfo_detail_t mode)
{
	const char *descs[N_ELEMENTS(pval_flags)];
	size_t count, i;
	bool full = mode & OINFO_FULL;
	bool dummy = mode & OINFO_DUMMY;
	bool search = FALSE;

	if (!o_ptr->num_pvals && !dummy)
		return FALSE;

	for (i = 0; i < o_ptr->num_pvals; i++) {
		count = info_collect(tb, pval_flags, N_ELEMENTS(pval_flags), flags[i], descs);

		if (count)
		{
			if ((object_this_pval_is_visible(o_ptr, i) || full) && !dummy)
				textblock_append_c(tb, (o_ptr->pval[i] > 0) ? TERM_L_GREEN : TERM_RED,
					"%+i ", o_ptr->pval[i]);
			else
				textblock_append(tb, "Affects your ");

			info_out_list(tb, descs, count);
		}
		if (of_has(flags[i], OF_SEARCH))
			search = TRUE;
	}

	if (search)
	{
		if ((object_this_pval_is_visible(o_ptr, which_pval(o_ptr, OF_SEARCH)) || full) && !dummy)
		{
			textblock_append_c(tb, (o_ptr->pval[which_pval(o_ptr, OF_SEARCH)] > 0) ? TERM_L_GREEN : TERM_RED,
				"%+i%% ", o_ptr->pval[which_pval(o_ptr, OF_SEARCH)] * 5);
			textblock_append(tb, "to searching.\n");
		}
		else if (count)
			textblock_append(tb, "Also affects your searching skill.\n");
		else
			textblock_append(tb, "Affects your searching skill.\n");
	}

	return TRUE;
}


/*
 * Describe immunities granted by an object.
 */
static bool describe_immune(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *i_descs[N_ELEMENTS(immunity_flags)];
	const char *r_descs[N_ELEMENTS(resist_flags)];
	const char *p_descs[N_ELEMENTS(protect_flags)];
	const char *v_descs[N_ELEMENTS(vuln_flags)];
	size_t count;

	bool prev = FALSE;

	/* Immunities */
	count = info_collect(tb, immunity_flags, N_ELEMENTS(immunity_flags),
			flags, i_descs);
	if (count)
	{
		textblock_append(tb, "Provides immunity to ");
		info_out_list(tb, i_descs, count);
		prev = TRUE;
	}

	/* Resistances */
	count = info_collect(tb, resist_flags, N_ELEMENTS(resist_flags),
			flags, r_descs);
	if (count)
	{
		textblock_append(tb, "Provides resistance to ");
		info_out_list(tb, r_descs, count);
		prev = TRUE;
	}

	/* Protections */
	count = info_collect(tb, protect_flags, N_ELEMENTS(protect_flags),
			flags, p_descs);
	if (count)
	{
		textblock_append(tb, "Provides protection from ");
		info_out_list(tb, p_descs, count);
		prev = TRUE;
	}

	/* Vulnerabilities */
	count = info_collect(tb, vuln_flags, N_ELEMENTS(vuln_flags),
			flags, v_descs);
	if (count)
	{
		textblock_append(tb, "Makes you vulnerable to ");
		info_out_list(tb, v_descs, count);
		prev = TRUE;
	}

	return prev;
}


/*
 * Describe 'ignores' of an object.
 */
static bool describe_ignores(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(ignore_flags)];
	size_t count = info_collect(tb, ignore_flags, N_ELEMENTS(ignore_flags),
			flags, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Cannot be harmed by ");
	info_out_list(tb, descs, count);

	return TRUE;
}


/*
 * Describe stat sustains.
 */
static bool describe_sustains(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(sustain_flags)];
	size_t count = info_collect(tb, sustain_flags, N_ELEMENTS(sustain_flags),
			flags, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Sustains ");
	info_out_list(tb, descs, count);

	return TRUE;
}


/*
 * Describe miscellaneous powers.
 */
static bool describe_misc_magic(textblock *tb, const bitflag flags[OF_SIZE])
{
	size_t i;
	bool printed = FALSE;

	for (i = 0; i < N_ELEMENTS(misc_flags); i++)
	{
		if (of_has(flags, misc_flags[i].flag))
		{
			textblock_append(tb, "%s.  ", misc_flags[i].name);
			printed = TRUE;
		}
	}

	if (printed)
		textblock_append(tb, "\n");

	return printed;
}


/*
 * Describe slays and brands on weapons
 */
static bool describe_slays(textblock *tb, const bitflag flags[OF_SIZE],
		int tval)
{
	bool printed = FALSE;
	const char *slay_descs[SL_MAX] = { 0 };
	bitflag slay_mask[OF_SIZE], kill_mask[OF_SIZE], brand_mask[OF_SIZE];
	size_t count;
	bool fulldesc;

	create_mask(slay_mask, FALSE, OFT_SLAY, OFT_MAX);
	create_mask(kill_mask, FALSE, OFT_KILL, OFT_MAX);
	create_mask(brand_mask, FALSE, OFT_BRAND, OFT_MAX);

	if (tval == TV_SWORD || tval == TV_HAFTED || tval == TV_POLEARM ||
			tval == TV_DIGGING || tval == TV_BOW || tval == TV_SHOT ||
			tval == TV_ARROW || tval == TV_BOLT || tval == TV_FLASK)
		fulldesc = FALSE;
	else
		fulldesc = TRUE;

	/* Slays */
	count = list_slays(flags, slay_mask, slay_descs, NULL, NULL, TRUE);
	if (count)
	{
		if (fulldesc)
			textblock_append(tb, "It causes your melee attacks to slay ");
		else
			textblock_append(tb, "Slays ");
		info_out_list(tb, slay_descs, count);
		printed = TRUE;
	}

	/* Kills */
	count = list_slays(flags, kill_mask, slay_descs, NULL, NULL, TRUE);
	if (count)
	{
		if (fulldesc)
			textblock_append(tb, "It causes your melee attacks to *slay* ");
		else
			textblock_append(tb, "*Slays* ");
		info_out_list(tb, slay_descs, count);
		printed = TRUE;
	}

	/* Brands */
	count = list_slays(flags, brand_mask, NULL, slay_descs, NULL, TRUE);
	if (count)
	{
		if (fulldesc)
			textblock_append(tb, "It brands your melee attacks with ");
		else
			textblock_append(tb, "Branded with ");
		info_out_list(tb, slay_descs, count);
		printed = TRUE;
	}

	return printed;
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
static bool describe_combat(textblock *tb, const object_type *o_ptr,
		oinfo_detail_t mode)
{
	bool full = mode & OINFO_FULL;

	const char *desc[SL_MAX] = { 0 };
	int i;
	int mult[SL_MAX];
	int cnt, dam, total_dam, plus = 0;
	int xtra_postcrit = 0, xtra_precrit = 0;
	int crit_mult, crit_div, crit_add;
	int str_plus, dex_plus, old_blows = 0, new_blows, extra_blows;
	int str_faster = -1, str_done = -1;
	object_type *bow = &p_ptr->inventory[INVEN_BOW];

	bitflag f[OF_SIZE], tmp_f[OF_SIZE], mask[OF_SIZE];

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->state.ammo_tval == o_ptr->tval) &&
	              (bow->kind);
	int multiplier = 1;

	/* Create the "all slays" mask */
	create_mask(mask, FALSE, OFT_SLAY, OFT_KILL, OFT_BRAND, OFT_MAX);

	/* Abort if we've nothing to say */
	if (mode & OINFO_DUMMY) return FALSE;

	if (!weapon && !ammo)
	{
		/* Potions can have special text */
		if (o_ptr->tval != TV_POTION ||
				o_ptr->dd == 0 || o_ptr->ds == 0 ||
				!object_flavor_is_aware(o_ptr))
			return FALSE;

		textblock_append(tb, "It can be thrown at creatures with damaging effect.\n");
		return TRUE;
	}

	if (full) {
		object_flags(o_ptr, f);
	} else {
		object_flags_known(o_ptr, f);
	}

	textblock_append_c(tb, TERM_L_WHITE, "Combat info:\n");

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

		memcpy(inven, p_ptr->inventory, INVEN_TOTAL * sizeof(object_type));
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
			textblock_append_c(tb, TERM_L_RED, "You are too weak to use this weapon.\n");

		textblock_append_c(tb, TERM_L_GREEN, "%d.%d ",
				state.num_blows / 100, (state.num_blows / 10) % 10);
		textblock_append(tb, "blow%s/round.\n",
				(state.num_blows > 100) ? "s" : "");

		/* Check to see if extra STR or DEX would yield extra blows */
		old_blows = state.num_blows;
		extra_blows = 0;

		/* First we need to look for extra blows on other items, as
		 * state does not track these */
		for (i = INVEN_BOW; i < INVEN_TOTAL; i++)
		{
			if (!p_ptr->inventory[i].kind)
				continue;
			object_flags_known(&p_ptr->inventory[i], tmp_f);

			if (of_has(tmp_f, OF_BLOWS))
				extra_blows += p_ptr->inventory[i].pval[which_pval(&p_ptr->inventory[i], OF_BLOWS)];
		}

		/* Then we add blows from the weapon being examined */
		if (of_has(f, OF_BLOWS))
			extra_blows += o_ptr->pval[which_pval(o_ptr, OF_BLOWS)];

		/* Then we check for extra "real" blows */
		for (dex_plus = 0; dex_plus < dex_plus_bound; dex_plus++)
		{
			for (str_plus = 0; str_plus < str_plus_bound; str_plus++)
	        {
				state.stat_ind[A_STR] += str_plus;
				state.stat_ind[A_DEX] += dex_plus;
				new_blows = calc_blows(o_ptr, &state, extra_blows);

				/* Test to make sure that this extra blow is a
				 * new str/dex combination, not a repeat
				 */
				if ((new_blows - new_blows % 10) > (old_blows - old_blows % 10) &&
					(str_plus < str_done ||
					str_done == -1))
				{
					textblock_append(tb, "With +%d STR and +%d DEX you would get %d.%d blows\n",
						str_plus, dex_plus, (new_blows / 100),
						(new_blows / 10) % 10);
					state.stat_ind[A_STR] -= str_plus;
					state.stat_ind[A_DEX] -= dex_plus;
					str_done = str_plus;
					break;
				}

				/* If the combination doesn't increment
				 * the displayed blows number, it might still
				 * take a little less energy
				 */
				if (new_blows > old_blows &&
					(str_plus < str_faster ||
					str_faster == -1) &&
					(str_plus < str_done ||
					str_done == -1))
				{
					textblock_append(tb, "With +%d STR and +%d DEX you would attack a bit faster\n",
						str_plus, dex_plus);
					state.stat_ind[A_STR] -= str_plus;
					state.stat_ind[A_DEX] -= dex_plus;
					str_faster = str_plus;
					continue;
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
		if (object_attack_plusses_are_visible(bow))
			dam += (bow->to_d * 10);

		/* Apply brands/slays from the shooter to the ammo, but only if known
		 * Note that this is not dependent on mode, so that viewing shop-held
		 * ammo (fully known) does not leak information about launcher */
		object_flags_known(bow, tmp_f);
		of_union(f, tmp_f);

		textblock_append(tb, "Hits targets up to ");
		textblock_append_c(tb, TERM_L_GREEN, format("%d", tdis * 10));
		textblock_append(tb, " feet away.\n");
	}

	/* Collect slays */
	/* Melee weapons get slays and brands from other items now */
	if (weapon)
	{
		bool nonweap = FALSE;

		for (i = INVEN_LEFT; i < INVEN_TOTAL; i++)
		{
			if (!p_ptr->inventory[i].kind)
				continue;
			object_flags_known(&p_ptr->inventory[i], tmp_f);

			of_inter(tmp_f, mask); /* strip out non-slays */

			if (of_union(f, tmp_f))
				nonweap = TRUE;
		}

		if (nonweap)
			textblock_append(tb, "This weapon may benefit from one or more off-weapon brands or slays.\n");
	}

	textblock_append(tb, "Average damage/round: ");

	if (ammo) multiplier = p_ptr->state.ammo_mult;

	cnt = list_slays(f, mask, desc, NULL, mult, TRUE);
	for (i = 0; i < cnt; i++)
	{
		int melee_adj_mult = ammo ? 0 : 1; /* ammo mult adds fully, melee mult is times 1, so adds 1 less */
		/* Include bonus damage and slay in stated average */
		total_dam = dam * (multiplier + mult[i] - melee_adj_mult) + xtra_precrit;
		total_dam = (total_dam * crit_mult + crit_add) / crit_div;
		total_dam += xtra_postcrit;
		if (weapon) 
			total_dam = (total_dam * old_blows) / 100;
		else
			total_dam *= p_ptr->state.num_shots;
		

		if (total_dam <= 0)
			textblock_append_c(tb, TERM_L_RED, "%d", 0);
		else if (total_dam % 10)
			textblock_append_c(tb, TERM_L_GREEN, "%d.%d",
					total_dam / 10, total_dam % 10);
		else
			textblock_append_c(tb, TERM_L_GREEN, "%d", total_dam / 10);


		textblock_append(tb, " vs. %s, ", desc[i]);
	}

	if (cnt) textblock_append(tb, "and ");

	/* Include bonus damage in stated average */
	total_dam = dam * multiplier + xtra_precrit;
	total_dam = (total_dam * crit_mult + crit_add) / crit_div;
	total_dam += xtra_postcrit;
	if (weapon)
		total_dam = (total_dam * old_blows) / 100;
	else
		total_dam *= p_ptr->state.num_shots;

	if (total_dam <= 0)
		textblock_append_c(tb, TERM_L_RED, "%d", 0);
	else if (total_dam % 10)
		textblock_append_c(tb, TERM_L_GREEN, "%d.%d",
				total_dam / 10, total_dam % 10);
	else
		textblock_append_c(tb, TERM_L_GREEN, "%d", total_dam / 10);

	if (cnt) textblock_append(tb, " vs. others");
	textblock_append(tb, ".\n");

	/* Note the impact flag */
	if (of_has(f, OF_IMPACT))
		textblock_append(tb, "Sometimes creates earthquakes on impact.\n");

	/* Add breakage chance */
	if (ammo)
	{
		int chance = breakage_chance(o_ptr, TRUE);
		textblock_append_c(tb, TERM_L_GREEN, "%d%%", chance);
		textblock_append(tb, " chance of breaking upon contact.\n");
	}

	/* You always have something to say... */
	return TRUE;
}

/*
 * Describe objects that can be used for digging.
 */
static bool describe_digger(textblock *tb, const object_type *o_ptr,
		oinfo_detail_t mode)
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

	memcpy(inven, p_ptr->inventory, INVEN_TOTAL * sizeof(object_type));

	/*
	 * Hack -- if we examine a ring that is worn on the right finger,
	 * we shouldn't put a copy of it on the left finger before calculating
	 * digging skills.
	 */
	if (o_ptr != &p_ptr->inventory[INVEN_RIGHT])
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

		if (i == 0 && chance > 0) {
			if (sl == INVEN_WIELD)
				textblock_append(tb, "Clears ");
			else
				textblock_append(tb, "With this item, your current weapon clears ");
		}

		if (i == 3 || (i != 0 && chance == 0))
			textblock_append(tb, "and ");

		if (chance == 0) {
			textblock_append_c(tb, TERM_L_RED, "doesn't affect ");
			textblock_append(tb, "%s.\n", names[i]);
			break;
		}

		textblock_append(tb, "%s in ", names[i]);

		if (chance == 1600) {
			textblock_append_c(tb, TERM_L_GREEN, "1 ");
		} else if (decis < 100) {
			textblock_append_c(tb, TERM_GREEN, "%d.%d ", decis/10, decis%10);
		} else {
			textblock_append_c(tb, (decis < 1000) ? TERM_YELLOW : TERM_RED,
			           "%d ", (decis+5)/10);
		}

		textblock_append(tb, "turn%s%s", decis == 10 ? "" : "s",
				(i == 3) ? ".\n" : ", ");
	}

	return TRUE;
}


static bool describe_food(textblock *tb, const object_type *o_ptr,
		bool subjective, bool full)
{
	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->pval[DEFAULT_PVAL])
	{
		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		if (object_is_known(o_ptr) || full) {
			textblock_append(tb, "Nourishes for around ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", (o_ptr->pval[DEFAULT_PVAL] / 2) *
				multiplier / 10);
			textblock_append(tb, " turns.\n");
		} else {
			textblock_append(tb, "Provides some nourishment.\n");
		}

		return TRUE;
	}

	return FALSE;
}


/*
 * Describe things that look like lights.
 */
static bool describe_light(textblock *tb, const object_type *o_ptr,
		const bitflag flags[OF_SIZE], bool terse)
{
	int rad = 0;

	bool artifact = o_ptr->artifact ? TRUE : FALSE;
	bool no_fuel = of_has(flags, OF_NO_FUEL) ? TRUE : FALSE;
	bool is_light = (o_ptr->tval == TV_LIGHT) ? TRUE : FALSE;

	if (o_ptr->tval != TV_LIGHT && !of_has(flags, OF_LIGHT))
		return FALSE;

	/* Work out radius */
	if (artifact && is_light)
		rad = 3;
	else if (is_light)
		rad = 2;
	if (of_has(flags, OF_LIGHT))
		rad++;

	/* Describe here */
	textblock_append(tb, "Radius ");
	textblock_append_c(tb, TERM_L_GREEN, format("%d", rad));
	if (no_fuel && !artifact)
		textblock_append(tb, " light.  No fuel required.");
	else if (is_light && o_ptr->sval == SV_LIGHT_TORCH)
		textblock_append(tb, " light, reduced when running out of fuel.");
	else
		textblock_append(tb, " light.");

	if (!terse && is_light && !no_fuel)
	{
		const char *name = (o_ptr->sval == SV_LIGHT_TORCH) ? "torches" : "lanterns";
		int turns = (o_ptr->sval == SV_LIGHT_TORCH) ? FUEL_TORCH : FUEL_LAMP;

		textblock_append(tb, "  Refills other %s up to %d turns of fuel.", name, turns);
	}

	textblock_append(tb, "\n");

	return TRUE;
}



/*
 * Describe an object's effect, if any.
 */
static bool describe_effect(textblock *tb, const object_type *o_ptr, bool full,
		bool only_artifacts, bool subjective)
{
	const char *desc;
	random_value timeout = {0, 0, 0, 0};

	int effect = 0, fail;

	if (o_ptr->artifact)
	{
		if (object_effect_is_known(o_ptr) || full)
		{
			effect = o_ptr->artifact->effect;
			timeout = o_ptr->artifact->time;
		}
		else if (object_effect(o_ptr))
		{
			textblock_append(tb, "It can be activated.\n");
			return TRUE;
		}
	}
	else
	{
		/* Sometimes only print artifact activation info */
		if (only_artifacts == TRUE) return FALSE;

		if (object_effect_is_known(o_ptr) || full)
		{
			effect = o_ptr->kind->effect;
			timeout = o_ptr->kind->time;
		}
		else if (object_effect(o_ptr) != 0)
		{
			if (effect_aim(o_ptr->kind->effect))
				textblock_append(tb, "It can be aimed.\n");
			else if (o_ptr->tval == TV_FOOD)
				textblock_append(tb, "It can be eaten.\n");
			else if (o_ptr->tval == TV_POTION)
				textblock_append(tb, "It can be drunk.\n");
			else if (o_ptr->tval == TV_SCROLL)
				textblock_append(tb, "It can be read.\n");
			else textblock_append(tb, "It can be activated.\n");

			return TRUE;
		}
	}

	/* Forget it without an effect */
	if (!effect) return FALSE;

	/* Obtain the description */
	desc = effect_desc(effect);
	if (!desc) return FALSE;

	if (effect_aim(effect))
		textblock_append(tb, "When aimed, it ");
	else if (o_ptr->tval == TV_FOOD)
		textblock_append(tb, "When eaten, it ");
	else if (o_ptr->tval == TV_POTION)
		textblock_append(tb, "When drunk, it ");
	else if (o_ptr->tval == TV_SCROLL)
	    textblock_append(tb, "When read, it ");
	else
	    textblock_append(tb, "When activated, it ");

	/* Print a colourised description */
	do
	{
		if (isdigit((unsigned char) *desc))
			textblock_append_c(tb, TERM_L_GREEN, "%c", *desc);
		else
			textblock_append(tb, "%c", *desc);
	} while (*desc++);

	textblock_append(tb, ".\n");

	if (randcalc(timeout, 0, MAXIMISE) > 0)
	{
		int min_time, max_time;

		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		textblock_append(tb, "Takes ");

		/* Correct for player speed */
		min_time = randcalc(timeout, 0, MINIMISE) * multiplier / 10;
		max_time = randcalc(timeout, 0, MAXIMISE) * multiplier / 10;

		textblock_append_c(tb, TERM_L_GREEN, "%d", min_time);

		if (min_time != max_time)
		{
			textblock_append(tb, " to ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", max_time);
		}

		textblock_append(tb, " turns to recharge");
		if (subjective && p_ptr->state.speed != 110)
			textblock_append(tb, " at your current speed");

		textblock_append(tb, ".\n");
	}

	if (!subjective || o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION ||
		o_ptr->tval == TV_SCROLL)
	{
		return TRUE;
	}
	else
	{
		fail = get_use_device_chance(o_ptr);
		textblock_append(tb, "Your chance of success is %d.%d%%\n", (1000 - fail) /
			10, (1000 - fail) % 10);
	}

	return TRUE;
}


static bool describe_origin(textblock *tb, const object_type *o_ptr)
{
	char origin_text[80];

	if (o_ptr->origin_depth)
		strnfmt(origin_text, sizeof(origin_text), "%d feet (level %d)",
		        o_ptr->origin_depth * 50, o_ptr->origin_depth);
	else
		my_strcpy(origin_text, "town", sizeof(origin_text));

	switch (o_ptr->origin)
	{
		case ORIGIN_NONE:
		case ORIGIN_MIXED:
		case ORIGIN_STOLEN:
			return FALSE;

		case ORIGIN_BIRTH:
			textblock_append(tb, "An inheritance from your family.\n");
			break;

		case ORIGIN_STORE:
			textblock_append(tb, "Bought from a store.\n");
			break;

		case ORIGIN_FLOOR:
			textblock_append(tb, "Found lying on the floor %s %s.\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text);
 			break;

		case ORIGIN_PIT:
			textblock_append(tb, "Found lying on the floor in a pit at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_VAULT:
			textblock_append(tb, "Found lying on the floor in a vault at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_SPECIAL:
			textblock_append(tb, "Found lying on the floor of a special room at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_LABYRINTH:
			textblock_append(tb, "Found lying on the floor of a labyrinth at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_CAVERN:
			textblock_append(tb, "Found lying on the floor of a cavern at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_RUBBLE:
			textblock_append(tb, "Found under some rubble at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_DROP:
		case ORIGIN_DROP_SPECIAL:
		case ORIGIN_DROP_PIT:
		case ORIGIN_DROP_VAULT:
		case ORIGIN_DROP_SUMMON:
		case ORIGIN_DROP_BREED:
		case ORIGIN_DROP_POLY:
		case ORIGIN_DROP_WIZARD:
		{
			const char *name;

			if (r_info[o_ptr->origin_xtra].ridx)
				name = r_info[o_ptr->origin_xtra].name;
			else
				name = "monster lost to history";

			textblock_append(tb, "Dropped by ");

			if (rf_has(r_info[o_ptr->origin_xtra].flags, RF_UNIQUE))
				textblock_append(tb, "%s", name);
			else
				textblock_append(tb, "%s%s",
						is_a_vowel(name[0]) ? "an " : "a ", name);

			textblock_append(tb, " %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text);
 			break;
		}

		case ORIGIN_DROP_UNKNOWN:
			textblock_append(tb, "Dropped by an unknown monster %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text);
			break;

		case ORIGIN_ACQUIRE:
			textblock_append(tb, "Conjured forth by magic %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text);
 			break;

		case ORIGIN_CHEAT:
			textblock_append(tb, "Created by debug option.\n");
 			break;

		case ORIGIN_CHEST:
			textblock_append(tb, "Found in a chest from %s.\n",
			         origin_text);
			break;
	}

	textblock_append(tb, "\n");

	return TRUE;
}

static void describe_flavor_text(textblock *tb, const object_type *o_ptr)
{
	/* Display the known artifact description */
	if (!OPT(birth_randarts) && o_ptr->artifact &&
			object_is_known(o_ptr) && o_ptr->artifact->text)
		textblock_append(tb, "%s\n\n", o_ptr->artifact->text);

	/* Display the known object description */
	else if (object_flavor_is_aware(o_ptr) || object_is_known(o_ptr))
	{
		bool did_desc = FALSE;

		if (o_ptr->kind->text)
		{
			textblock_append(tb, "%s", o_ptr->kind->text);
			did_desc = TRUE;
		}

		/* Display an additional ego-item description */
		if (object_ego_is_visible(o_ptr) && o_ptr->ego->text)
		{
			if (did_desc) textblock_append(tb, "  ");
			textblock_append(tb, "%s\n\n", o_ptr->ego->text);
		}
		else if (did_desc)
		{
			textblock_append(tb, "\n\n");
		}
	}
}


static bool describe_ego(textblock *tb, const struct ego_item *ego)
{
	if (ego && ego->xtra)
	{
		const char *xtra[] = { "sustain", "higher resistance", "ability" };
		textblock_append(tb, "It provides one random %s.  ",
				xtra[ego->xtra - 1]);

		return TRUE;
	}

	return FALSE;
}


/*
 * Output object information
 */
static textblock *object_info_out(const object_type *o_ptr, oinfo_detail_t mode)
{
	bitflag flags[OF_SIZE];
	bitflag pval_flags[MAX_PVALS][OF_SIZE];
	bool something = FALSE;
	bool known = object_is_known(o_ptr);

	bool full = mode & OINFO_FULL;
	bool terse = mode & OINFO_TERSE;
	bool subjective = mode & OINFO_SUBJ;
	bool ego = mode & OINFO_EGO;

	textblock *tb = textblock_new();

	/* Grab the object flags */
	if (full) {
		object_flags(o_ptr, flags);
		object_pval_flags(o_ptr, pval_flags);
	} else {
		object_flags_known(o_ptr, flags);
		object_pval_flags_known(o_ptr, pval_flags);
	}

	if (subjective) describe_origin(tb, o_ptr);
	if (!terse) describe_flavor_text(tb, o_ptr);

	if (!full && !known)
	{
		textblock_append(tb, "You do not know the full extent of this item's powers.\n");
		something = TRUE;
	}

	if (describe_curses(tb, o_ptr, flags)) something = TRUE;
	if (describe_stats(tb, o_ptr, pval_flags, mode)) something = TRUE;
	if (describe_slays(tb, flags, o_ptr->tval)) something = TRUE;
	if (describe_immune(tb, flags)) something = TRUE;
	if (describe_ignores(tb, flags)) something = TRUE;
	if (describe_sustains(tb, flags)) something = TRUE;
	if (describe_misc_magic(tb, flags)) something = TRUE;
	if (ego && describe_ego(tb, o_ptr->ego)) something = TRUE;
	if (something) textblock_append(tb, "\n");

	if (describe_effect(tb, o_ptr, full, terse, subjective))
	{
		something = TRUE;
		textblock_append(tb, "\n");
	}

	if (subjective && describe_combat(tb, o_ptr, mode))
	{
		something = TRUE;
		textblock_append(tb, "\n");
	}

	if (!terse && describe_food(tb, o_ptr, subjective, full)) something = TRUE;
	if (describe_light(tb, o_ptr, flags, terse)) something = TRUE;
	if (!terse && subjective && describe_digger(tb, o_ptr, mode)) something = TRUE;

	if (!something)
		textblock_append(tb, "\n\nThis item does not seem to possess any special abilities.");

	return tb;
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
textblock *object_info(const object_type *o_ptr, oinfo_detail_t mode)
{
	mode |= OINFO_SUBJ;
	return object_info_out(o_ptr, mode);
}

/**
 * Provide information on an ego-item type
 */
textblock *object_info_ego(struct ego_item *ego)
{
	object_kind *kind = NULL;
	object_type obj = { 0 };
	int i;

	for (i = 0; i < z_info->k_max; i++) {
		kind = &k_info[i];
		if (!kind->name)
			continue;
		if (kind->tval == ego->tval[0])
			break;
	}

	obj.kind = kind;
	obj.tval = kind->tval;
	obj.sval = kind->sval;
	obj.ego = ego;
	of_union(obj.flags, ego->flags);

	return object_info_out(&obj, OINFO_FULL | OINFO_EGO | OINFO_DUMMY);
}



/**
 * Provide information on an item suitable for writing to the character dump - keep it brief.
 */
void object_info_chardump(ang_file *f, const object_type *o_ptr, int indent, int wrap)
{
	textblock *tb = object_info_out(o_ptr, OINFO_TERSE | OINFO_SUBJ);
	textblock_to_file(tb, f, indent, wrap);
	textblock_free(tb);
}


/**
 * Provide spoiler information on an item.
 *
 * Practically, this means that we should not print anything which relies upon
 * the player's current state, since that is not suitable for spoiler material.
 */
void object_info_spoil(ang_file *f, const object_type *o_ptr, int wrap)
{
	textblock *tb = object_info_out(o_ptr, OINFO_FULL);
	textblock_to_file(tb, f, 0, wrap);
	textblock_free(tb);
}
