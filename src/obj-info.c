/**
   \file obj-info.c
   \brief Object description code.
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
#include "init.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-info.h"
#include "obj-make.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "project.h"
#include "tables.h"
#include "z-textblock.h"

/**
 * Describes a flag-name pair.
 */
typedef struct
{
	int flag;
	const char *name;
} flag_type;

/**
 * Describes the number of blows possible for given stat bonuses
 */
struct blow_info {
	int str_plus;
	int dex_plus;  
	int centiblows;
};

/* Denotes the property being present, but specifics being unknown */
#define OBJ_KNOWN_PRESENT -1


/*** Big fat data tables ***/

static const flag_type elements[] =
{
	#define ELEM(a, b, c, d, e, col, f, fh, oh, mh, ph)	\
	{ELEM_##a, b},
    #include "list-elements.h"
    #undef ELEM
};

static const flag_type mod_flags[] =
{
	#define OBJ_MOD(a, b, c, d)	{OBJ_MOD_##a, d},
    #include "list-object-modifiers.h"
    #undef OBJ_MOD
};

static const flag_type protect_flags[] =
{
	{ OF_PROT_FEAR, "fear" },
	{ OF_PROT_BLIND, "blindness" },
	{ OF_PROT_CONF, "confusion" },
	{ OF_PROT_STUN,  "stunning" },
};

static const flag_type sustain_flags[] =
{
	{ OF_SUST_STR, "strength" },
	{ OF_SUST_INT, "intelligence" },
	{ OF_SUST_WIS, "wisdom" },
	{ OF_SUST_DEX, "dexterity" },
	{ OF_SUST_CON, "constitution" },
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


/*** Utility code ***/

/**
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


/**
 * Fills recepticle with all the flags in `flags` that are in the given `list`.
 */
static size_t flag_info_collect(const flag_type list[], size_t max,
								const bitflag flags[OF_SIZE],
								const char *recepticle[])
{
	size_t i, count = 0;

	for (i = 0; i < max; i++)
	{
		if (of_has(flags, list[i].flag))
			recepticle[count++] = list[i].name;
	}

	return count;
}

/**
 * Fills recepticle with all the elements that correspond to the given `list`.
 */
static size_t element_info_collect(const bool list[], const char *recepticle[])
{
	size_t i, count = 0;

	for (i = 0; i < N_ELEMENTS(elements); i++)
	{
		if (list[i])
			recepticle[count++] = elements[i].name;
	}
	
	return count;
}


/*** Code that makes use of the data tables ***/

/**
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


/**
 * Describe stat modifications.
 */
static bool describe_stats(textblock *tb, const object_type *o_ptr,
						   oinfo_detail_t mode)
{
	size_t count = 0, i;
	bool detail = FALSE;

	/* Don't give exact pluses for faked ego items as each real one 
	   will be different */
	bool suppress_details = mode & OINFO_EGO ? TRUE : FALSE;

	/* See what we've got */
	for (i = 0; i < N_ELEMENTS(mod_flags); i++)
		if (o_ptr->modifiers[mod_flags[i].flag] != 0 &&	mod_flags[i].name[0]) {
			count++;
			/* Either all mods are visible, or none are */
			if (object_this_mod_is_visible(o_ptr, i))
				detail = TRUE;
		}
	
	if (!count)
		return FALSE;
	
	for (i = 0; i < N_ELEMENTS(mod_flags); i++) {
		const char *desc = mod_flags[i].name;
		int val = o_ptr->modifiers[mod_flags[i].flag];
		if (!val) continue;
		if (!mod_flags[i].name[0]) continue;
		if (detail && !suppress_details) {
			int attr = (val > 0) ? TERM_L_GREEN : TERM_RED;
			textblock_append_c(tb, attr, "%+i %s.\n", val, desc);
		} 
		else
			textblock_append(tb, "Affects your %s\n", desc);
	}

	return TRUE;
}


/**
 * Describe immunities, resistances and vulnerabilities granted by an object.
 */
static bool describe_elements(textblock *tb,
							  const struct element_info el_info[])
{
	const char *i_descs[N_ELEMENTS(elements)];
	const char *r_descs[N_ELEMENTS(elements)];
	const char *v_descs[N_ELEMENTS(elements)];
	size_t i, count;

	bool list[N_ELEMENTS(elements)], prev = FALSE;

	/* Immunities */
	for (i = 0; i < N_ELEMENTS(elements); i++)
		list[i] = (el_info[i].res_level == 3);
	count = element_info_collect(list, i_descs);
	if (count)
	{
		textblock_append(tb, "Provides immunity to ");
		info_out_list(tb, i_descs, count);
		prev = TRUE;
	}

	/* Resistances */
	for (i = 0; i < N_ELEMENTS(elements); i++)
		list[i] = (el_info[i].res_level == 1);
	count = element_info_collect(list, r_descs);
	if (count)
	{
		textblock_append(tb, "Provides resistance to ");
		info_out_list(tb, r_descs, count);
		prev = TRUE;
	}

	/* Vulnerabilities */
	for (i = 0; i < N_ELEMENTS(elements); i++)
		list[i] = (el_info[i].res_level == -1);
	count = element_info_collect(list, v_descs);
	if (count)
	{
		textblock_append(tb, "Makes you vulnerable to ");
		info_out_list(tb, v_descs, count);
		prev = TRUE;
	}

	return prev;
}


/**
 * Describe protections granted by an object.
 */
static bool describe_protects(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *p_descs[N_ELEMENTS(protect_flags)];
	size_t count;

	/* Protections */
	count = flag_info_collect(protect_flags, N_ELEMENTS(protect_flags),
			flags, p_descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Provides protection from ");
	info_out_list(tb, p_descs, count);

	return  TRUE;
}

/**
 * Describe elements an object ignores.
 */
static bool describe_ignores(textblock *tb, const struct element_info el_info[])
{
	const char *descs[N_ELEMENTS(elements)];
	size_t i, count;
	bool list[N_ELEMENTS(elements)];

	for (i = 0; i < N_ELEMENTS(elements); i++)
		list[i] = (el_info[i].flags & EL_INFO_IGNORE);
	count = element_info_collect(list, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Cannot be harmed by ");
	info_out_list(tb, descs, count);

	return TRUE;
}

/**
 * Describe elements that damage or destroy an object.
 */
static bool describe_hates(textblock *tb, const struct element_info el_info[])
{
	const char *descs[N_ELEMENTS(elements)];
	size_t i, count = 0;
	bool list[N_ELEMENTS(elements)];

	for (i = 0; i < N_ELEMENTS(elements); i++)
		list[i] = (el_info[i].flags & EL_INFO_HATES);
	count = element_info_collect(list, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Can be destroyed by ");
	info_out_list(tb, descs, count);

	return TRUE;
}


/**
 * Describe stat sustains.
 */
static bool describe_sustains(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(sustain_flags)];
	size_t count = flag_info_collect(sustain_flags, N_ELEMENTS(sustain_flags),
									 flags, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Sustains ");
	info_out_list(tb, descs, count);

	return TRUE;
}


/**
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


/**
 * Describe slays and brands on weapons
 */
static bool describe_slays(textblock *tb, const struct object *o_ptr)
{
	struct slay *s = o_ptr->slays;

	if (!s) return FALSE;

	if (tval_is_weapon(o_ptr) || tval_is_fuel(o_ptr))
		textblock_append(tb, "Slays ");
	else
		textblock_append(tb, "It causes your melee attacks to slay ");

	while (s) {
		textblock_append(tb, s->name);
		if (s->multiplier > 3)
			textblock_append(tb, " (powerfully)");
		if (s->next)
			textblock_append(tb, ", ");
		else
			textblock_append(tb, ".\n");
		s = s->next;
	}

	return TRUE;
}

/**
 * Describe slays and brands on weapons
 */
static bool describe_brands(textblock *tb, const struct object *o_ptr)
{
	struct brand *b = o_ptr->brands;

	if (!b) return FALSE;

	if (tval_is_weapon(o_ptr) || tval_is_fuel(o_ptr))
		textblock_append(tb, "Branded with ");
	else
		textblock_append(tb, "It brands your melee attacks with ");

	while (b) {
		if (b->multiplier < 3)
			textblock_append(tb, "weak ");
		textblock_append(tb, b->name);
		if (b->next)
			textblock_append(tb, ", ");
		else
			textblock_append(tb, ".\n");
		b = b->next;
	}

	return TRUE;
}

/**
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
	int k, to_crit = weight + 5*(state->to_h + plus) + 3*player->lev;
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

/**
 * Missile crits follow the same approach as melee crits.
 */
static void calculate_missile_crits(player_state *state, int weight,
		int plus, int *mult, int *add, int *div)
{
	int k, to_crit = weight + 4*(state->to_h + plus) + 2*player->lev;
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

/**
 * Get the object flags the player should know about for the given object/
 * viewing mode combination.
 */
static void get_known_flags(const object_type *o_ptr, const oinfo_detail_t mode, bitflag flags[OF_SIZE])
{
	/* Grab the object flags */
	if (mode & OINFO_EGO) {
		/* Looking at fake egos needs less info than object_flags_known() */
		if (flags)
			object_flags(o_ptr, flags);
	} else {
		if (flags)
			object_flags_known(o_ptr, flags);

		/* Don't include base flags when terse */
		if (flags && mode & OINFO_TERSE)
			of_diff(flags, o_ptr->kind->base->flags);
	}
}

/**
 * Get the object element info the player should know about for the given
 * object/viewing mode combination.
 */
static void get_known_elements(const object_type *o_ptr, const oinfo_detail_t mode, struct element_info el_info[])
{
	size_t i;

	/* Grab the object flags */
	for (i = 0; i < N_ELEMENTS(elements); i++) {
		/* Report fake egos or known element info */
		if ((mode & OINFO_EGO) || (o_ptr->el_info[i].flags & EL_INFO_KNOWN)) {
			el_info[i].res_level = o_ptr->el_info[i].res_level;
			el_info[i].flags = o_ptr->el_info[i].flags;
		} else {
			el_info[i].res_level = 0;
			el_info[i].flags = 0;
		}

		/* Ignoring an element: */
		if (o_ptr->el_info[i].flags & EL_INFO_IGNORE) {
			/* If the object is usually destroyed, mention the ignoring; */
			if (o_ptr->el_info[i].flags & EL_INFO_HATES)
				el_info[i].flags &= ~(EL_INFO_HATES);
			/* Otherwise, don't say anything */
			else
				el_info[i].flags &= ~(EL_INFO_IGNORE);
		}

		/* Don't include hates flag when terse */
		if (mode & OINFO_TERSE)
			el_info[i].flags &= ~(EL_INFO_HATES);
	}
}

/**
 * Gets information about the number of blows possible for the player with
 * the given object.
 *
 * Fills in whether the object is too_heavy to wield effectively,
 * and the possible_blows[] information of .str_plus and .dex_plus needed
 * to achieve the approximate number of blows in centiblows. 
 *
 * `max_blows` must be at least 1 to hold the current number of blows
 * `possible_blows` must be at least [`max_blows`] in size, and will be limited
 * to that number of entries.  The theoretical maximum is STAT_RANGE * 2 if
 * an extra blow/speed boost was given for each combination of STR and DEX.
 *
 * Returns the number of entries made in the possible_blows[] table, or 0
 * if the object is not a weapon.
 *
 * Note that the results are meaningless if called on a fake ego object as
 * the actual ego may have different properties.
 */
static int obj_known_blows(const object_type *o_ptr, int max_num, struct blow_info possible_blows[])
{
	int str_plus, dex_plus, old_blows = 0, new_blows, extra_blows;
	int str_faster = -1, str_done = -1;
	int dex_plus_bound;
	int str_plus_bound;
	int i;

	player_state state;

	object_type *gear;
	int weapon_index = slot_index(player, slot_by_name(player, "weapon"));
	int num = 0;

	/* Not a weapon - no blows! */
	if (!tval_is_melee_weapon(o_ptr)) return 0;

	gear = (object_type *) mem_zalloc(player->max_gear * sizeof(object_type));
	memcpy(gear, player->gear, player->max_gear * sizeof(object_type));
	gear[weapon_index] = *o_ptr;

	/* Calculate the player's hypothetical state */
	calc_bonuses(gear, &state, TRUE);

	/* First entry is always the current num of blows. */
	possible_blows[num].str_plus = 0;
	possible_blows[num].dex_plus = 0;
	possible_blows[num].centiblows = state.num_blows;
	num++;

	/* Check to see if extra STR or DEX would yield extra blows */
	old_blows = state.num_blows;
	extra_blows = 0;

	/* Start with blows from the weapon being examined */
	if (object_this_mod_is_visible(o_ptr, OBJ_MOD_BLOWS))
		extra_blows += o_ptr->modifiers[OBJ_MOD_BLOWS];

	/* Then we need to look for extra blows on other items, as
	 * state does not track these */
	for (i = 0; i < player->body.count; i++)
	{
		object_type *helper = equipped_item_by_slot(player, i);

		if ((i == slot_by_name(player, "weapon")) || !helper->kind)
			continue;

		if (object_this_mod_is_visible(helper, OBJ_MOD_BLOWS))
			extra_blows += helper->modifiers[OBJ_MOD_BLOWS];
	}

	dex_plus_bound = STAT_RANGE - state.stat_ind[A_DEX];
	str_plus_bound = STAT_RANGE - state.stat_ind[A_STR];

	/* Then we check for extra "real" blows */
	for (dex_plus = 0; dex_plus < dex_plus_bound; dex_plus++)
	{
		for (str_plus = 0; str_plus < str_plus_bound; str_plus++)
        {
			if (num == max_num) {
				mem_free(gear);
				return num;
			}

			state.stat_ind[A_STR] += str_plus;
			state.stat_ind[A_DEX] += dex_plus;
			new_blows = calc_blows(o_ptr, &state, extra_blows);
			state.stat_ind[A_STR] -= str_plus;
			state.stat_ind[A_DEX] -= dex_plus;

			/* Test to make sure that this extra blow is a
			 * new str/dex combination, not a repeat
			 */
			if ((new_blows - new_blows % 10) > (old_blows - old_blows % 10) &&
				(str_plus < str_done ||
				str_done == -1))
			{
				possible_blows[num].str_plus = str_plus;
				possible_blows[num].dex_plus = dex_plus;
				possible_blows[num].centiblows = new_blows / 10;
				possible_blows[num].centiblows *= 10;
				num++;

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
				possible_blows[num].str_plus = str_plus;
				possible_blows[num].dex_plus = dex_plus;
				possible_blows[num].centiblows = new_blows;
				num++;

				str_faster = str_plus;
			}
		}
	}

	mem_free(gear);
	return num;
}


/**
 * Describe blows.
 */
static bool describe_blows(textblock *tb, const object_type *o_ptr)
{
	int i;
	struct blow_info blow_info[STAT_RANGE * 2]; /* (Very) theoretical max */
	int num_entries = 0;

	num_entries = obj_known_blows(o_ptr, STAT_RANGE * 2, blow_info);
	if (num_entries == 0) return FALSE;

	/* First entry is always current blows (+0, +0) */
	textblock_append_c(tb, TERM_L_GREEN, "%d.%d ",
			blow_info[0].centiblows / 100, 
			(blow_info[0].centiblows / 10) % 10);
	textblock_append(tb, "blow%s/round.\n",
			(blow_info[0].centiblows > 100) ? "s" : "");

	/* Then list combinations that give more blows / speed boost */
	for (i = 1; i < num_entries; i++) {
		struct blow_info entry = blow_info[i];

		if (entry.centiblows % 10 == 0) {
			textblock_append(tb, 
				"With +%d STR and +%d DEX you would get %d.%d blows\n",
				entry.str_plus, entry.dex_plus, 
				(entry.centiblows / 100),
				(entry.centiblows / 10) % 10);
		} else {
			textblock_append(tb, 
				"With +%d STR and +%d DEX you would attack a bit faster\n",
				entry.str_plus, entry.dex_plus);
		}
	}

	return TRUE;
}


/**
 * Gets information about the average damage/turn that can be inflicted if
 * the player wields the given weapon.
 *
 * Fills in the damage against normal adversaries in `normal_damage`, as well
 * as the slays on the weapon in slay_list[] and corresponding damages in 
 * slay_damage[].  These must both be at least SL_MAX long to be safe.
 * `nonweap_slay` is set to whether other items being worn could add to the
 * damage done by branding attacks.
 *
 * Returns the number of slays populated in slay_list[] and slay_damage[].
 *
 * Note that the results are meaningless if called on a fake ego object as
 * the actual ego may have different properties.
 */
static int obj_known_damage(const object_type *o_ptr, int *normal_damage, 
							struct brand **brand_list, int **brand_damage, 
							struct slay **slay_list, int **slay_damage, 
							bool *nonweap_slay)
{
	int i;
	int dice, sides, dam, total_dam, plus = 0;
	int xtra_postcrit = 0, xtra_precrit = 0;
	int crit_mult, crit_div, crit_add;
	int old_blows = 0;

	object_type *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(o_ptr);
	bool ammo   = (player->state.ammo_tval == o_ptr->tval) &&
	              (bow->kind);
	int multiplier = 1;

	player_state state;
	object_type *gear;
	struct slay *s;
	struct brand *b;
	int num_slays;
	int num_brands;
	int weapon_index = slot_index(player, slot_by_name(player, "weapon"));

	/* Calculate the player's hypothetical state */
	gear = (object_type *) mem_zalloc(player->max_gear * sizeof(object_type));
	memcpy(gear, player->gear, player->max_gear * sizeof(object_type));
	gear[weapon_index] = *o_ptr;
	calc_bonuses(gear, &state, TRUE);

	/* Use displayed dice if real dice not known */
	if (object_attack_plusses_are_visible(o_ptr)) {
		dice = o_ptr->dd;
		sides = o_ptr->ds;
	} else {
		dice = o_ptr->kind->dd;
		sides = o_ptr->kind->ds;
	}

	/* Calculate damage */
	dam = ((sides + 1) * dice * 5);

	if (weapon)	{
		xtra_postcrit = state.to_d * 10;
		if (object_attack_plusses_are_visible(o_ptr)) {
			xtra_precrit += o_ptr->to_d * 10;
			plus += o_ptr->to_h;
		}

		calculate_melee_crits(&state, o_ptr->weight, plus,
				&crit_mult, &crit_add, &crit_div);

		old_blows = state.num_blows;
	} else { /* Ammo */
		if (object_attack_plusses_are_visible(o_ptr))
			plus += o_ptr->to_h;

		calculate_missile_crits(&player->state, o_ptr->weight, plus,
				&crit_mult, &crit_add, &crit_div);

		if (object_attack_plusses_are_visible(o_ptr))
			dam += (o_ptr->to_d * 10);
		if (object_attack_plusses_are_visible(bow))
			dam += (bow->to_d * 10);
	}

	/* Melee weapons may get slays and brands from other items */
	*nonweap_slay = FALSE;
	if (weapon)	{
		for (i = 2; i < player->body.count; i++) {
			struct object *obj = equipped_item_by_slot(player, i);
			if (!obj->kind)
				continue;

			for (s = obj->slays; s; s = s->next) {
				if (s->known) {
					*nonweap_slay = TRUE;
					break;
				}
			}
			for (b = obj->brands; b; b = b->next) {
				if (b->known) {
					*nonweap_slay = TRUE;
					break;
				}
			}
			if (*nonweap_slay) break;
		}
	}

	if (ammo) multiplier = player->state.ammo_mult;

	/* Get the brands */
	*brand_list = brand_collect(o_ptr, ammo ? bow : NULL, &num_brands, TRUE);
	if (*brand_list) *brand_damage = mem_zalloc(num_brands * sizeof(int));

	/* Get damage for each brand on the objects */
	for (i = 0; i < num_brands; i++) {
		/* ammo mult adds fully, melee mult is times 1, so adds 1 less */
		int melee_adj_mult = ammo ? 0 : 1;

		/* Include bonus damage and slay in stated average */
		total_dam = dam * (multiplier + (*brand_list)[i].multiplier
						   - melee_adj_mult) + xtra_precrit;
		total_dam = (total_dam * crit_mult + crit_add) / crit_div;
		total_dam += xtra_postcrit;

		if (weapon)
			total_dam = (total_dam * old_blows) / 100;
		else
			total_dam *= player->state.num_shots;

		(*brand_damage)[i] = total_dam;
		i++;
	}

	/* Get the slays */
	*slay_list = slay_collect(o_ptr, ammo ? bow : NULL, &num_slays, TRUE);
	if (*slay_list) *slay_damage = mem_zalloc(num_slays * sizeof(int));

	/* Get damage for each slay on the objects */
	for (i = 0; i < num_slays; i++) {
		/* ammo mult adds fully, melee mult is times 1, so adds 1 less */
		int melee_adj_mult = ammo ? 0 : 1;

		/* Include bonus damage and slay in stated average */
		total_dam = dam * (multiplier + (*slay_list)[i].multiplier
						   - melee_adj_mult) + xtra_precrit;
		total_dam = (total_dam * crit_mult + crit_add) / crit_div;
		total_dam += xtra_postcrit;

		if (weapon)
			total_dam = (total_dam * old_blows) / 100;
		else
			total_dam *= player->state.num_shots;

		(*slay_damage)[i] = total_dam;
	}

	/* Include bonus damage in stated average */
	total_dam = dam * multiplier + xtra_precrit;
	total_dam = (total_dam * crit_mult + crit_add) / crit_div;
	total_dam += xtra_postcrit;

	/* Normal damage, not considering brands or slays */
	if (weapon)
		total_dam = (total_dam * old_blows) / 100;
	else
		total_dam *= player->state.num_shots;

	*normal_damage = total_dam;

	mem_free(gear);
	return num_brands + num_slays;
}


/**
 * Describe damage.
 */
static bool describe_damage(textblock *tb, const object_type *o_ptr)
{
	bool nonweap_slay = FALSE;
	int normal_damage;
	int *brand_damage = NULL;
	struct brand *brands = NULL;
	int *slay_damage = NULL;
	struct slay *slays = NULL;
	int num;
	int i;

	/* Collect brands and slays */
	num = obj_known_damage(o_ptr, &normal_damage, &brands, &brand_damage,
						   &slays, &slay_damage, &nonweap_slay);

	/* Mention slays and brands from other items */
	if (nonweap_slay)
		textblock_append(tb, "This weapon may benefit from one or more off-weapon brands or slays.\n");

	textblock_append(tb, "Average damage/round: ");

	/* Output damage for creatures effected by the brands */
	i = 0;
	if (brands)
		do {
			if (brand_damage[i] <= 0)
				textblock_append_c(tb, TERM_L_RED, "%d", 0);
			else if (brand_damage[i] % 10)
				textblock_append_c(tb, TERM_L_GREEN, "%d.%d",
								   brand_damage[i] / 10, brand_damage[i] % 10);
			else
				textblock_append_c(tb, TERM_L_GREEN, "%d",brand_damage[i] / 10);

			textblock_append(tb, " vs. creatures not resistant to %s, ",
							 brands[i].name);
			i++;
		} while (brands[i - 1].next);

	/* Output damage for creatures effected by the slays */
	i = 0;
	if (slays)
		do {
			if (slay_damage[i] <= 0)
				textblock_append_c(tb, TERM_L_RED, "%d", 0);
			else if (slay_damage[i] % 10)
				textblock_append_c(tb, TERM_L_GREEN, "%d.%d",
								   slay_damage[i] / 10, slay_damage[i] % 10);
			else
				textblock_append_c(tb, TERM_L_GREEN, "%d", slay_damage[i] / 10);

			textblock_append(tb, " vs. %s, ", slays[i].name);
			i++;
		} while (slays[i - 1].next);

	if (num) textblock_append(tb, "and ");

	if (normal_damage <= 0)
		textblock_append_c(tb, TERM_L_RED, "%d", 0);
	else if (normal_damage % 10)
		textblock_append_c(tb, TERM_L_GREEN, "%d.%d",
			   normal_damage / 10, normal_damage % 10);
	else
		textblock_append_c(tb, TERM_L_GREEN, "%d", normal_damage / 10);

	if (num) textblock_append(tb, " vs. others");
	textblock_append(tb, ".\n");

	if (brands) mem_free(brands);
	if (brand_damage) mem_free(brand_damage);
	if (slays) mem_free(slays);
	if (slay_damage) mem_free(slay_damage);
	return TRUE;
}

/**
 * Gets miscellaneous combat information about the given object.
 *
 * Fills in whether there is a special effect when thrown in `thrown effect`,
 * the `range` in ft (or zero if not ammo), whether the weapon has the 
 * impact flag set, the percentage chance of breakage and whether it is
 * too heavy to be weilded effectively at the moment.
 */
static void obj_known_misc_combat(const object_type *o_ptr, bool *thrown_effect, int *range, bool *impactful, int *break_chance, bool *too_heavy)
{
	object_type *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(o_ptr);
	bool ammo   = (player->state.ammo_tval == o_ptr->tval) &&
	              (bow->kind);
	bitflag f[OF_SIZE];

	*thrown_effect = *impactful = *too_heavy = FALSE;
	*range = *break_chance = 0;

	get_known_flags(o_ptr, 0, f);

	if (!weapon && !ammo) {
		/* Potions can have special text */
		if (tval_is_potion(o_ptr) &&
				o_ptr->dd != 0 && o_ptr->ds != 0 &&
				object_flavor_is_aware(o_ptr))
			*thrown_effect = TRUE;
	}

	if (ammo)
		*range = 6 + 2 * player->state.ammo_mult * 10;;

	/* Note the impact flag */
	*impactful = of_has(f, OF_IMPACT);

	/* Add breakage chance */
	*break_chance = breakage_chance(o_ptr, TRUE);

	/* Is the weapon too heavy? */
	if (weapon) {
		player_state state;
		object_type *gear;
		int weapon_index = slot_index(player, slot_by_name(player, "weapon"));

		gear = (object_type *) mem_zalloc(player->max_gear * sizeof(object_type));
		memcpy(gear, player->gear, player->max_gear * sizeof(object_type));
		gear[weapon_index] = *o_ptr;

		/* Calculate the player's hypothetical state */
		calc_bonuses(gear, &state, TRUE);

		/* Warn about heavy weapons */
		*too_heavy = state.heavy_wield;
		mem_free(gear);
	}
}


/**
 * Describe combat advantages.
 */
static bool describe_combat(textblock *tb, const object_type *o_ptr)
{
	object_type *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(o_ptr);
	bool ammo   = (player->state.ammo_tval == o_ptr->tval) &&
	              (bow->kind);

	int range, break_chance;
	bool impactful, thrown_effect, too_heavy;

	obj_known_misc_combat(o_ptr, &thrown_effect, &range, &impactful, &break_chance, &too_heavy);

	if (!weapon && !ammo) {
		if (thrown_effect) {
			textblock_append(tb, "It can be thrown at creatures with damaging effect.\n");
			return TRUE;
		}
		else return FALSE;
	}

	textblock_append_c(tb, TERM_L_WHITE, "Combat info:\n");

	if (too_heavy)
		textblock_append_c(tb, TERM_L_RED, "You are too weak to use this weapon.\n");

	describe_blows(tb, o_ptr);

	if (!weapon) { /* Ammo */
		textblock_append(tb, "Hits targets up to ");
		textblock_append_c(tb, TERM_L_GREEN, format("%d", range));
		textblock_append(tb, " feet away.\n");
	}

	describe_damage(tb, o_ptr);

	if (impactful)
		textblock_append(tb, "Sometimes creates earthquakes on impact.\n");

	if (ammo) {
		textblock_append_c(tb, TERM_L_GREEN, "%d%%", break_chance);
		textblock_append(tb, " chance of breaking upon contact.\n");
	}

	/* Something has been said */
	return TRUE;
}


/**
 * Returns information about objects that can be used for digging.
 *
 * `deciturns` will be filled in with the avg number of deciturns it will
 * take to dig through each type of diggable terrain, and must be at least 
 * [DIGGING_MAX].
 *
 * Returns FALSE if the object has no effect on digging, or if the specifics
 * are meaningless (i.e. the object is an ego template, not a real item).
 */
static bool obj_known_digging(object_type *o_ptr, int deciturns[])
{
	player_state st;

	object_type *gear;

	int index = slot_index(player, wield_slot(o_ptr));
	int i;

	int chances[DIGGING_MAX];

	if (!tval_is_wearable(o_ptr) || 
		(!tval_is_melee_weapon(o_ptr) && 
		 (o_ptr->modifiers[OBJ_MOD_TUNNEL] <= 0)))
		return FALSE;

	gear = (object_type *) mem_zalloc(player->max_gear * sizeof(object_type));
	memcpy(gear, player->gear, player->max_gear * sizeof(object_type));

	/* Simulate wearing, unless it's already worn */
	if (!item_is_equipped(player, object_gear_index(player, o_ptr)))
		gear[index] = *o_ptr;

	calc_bonuses(gear, &st, TRUE);
	calc_digging_chances(&st, chances); /* Out of 1600 */

	for (i = DIGGING_RUBBLE; i < DIGGING_MAX; i++)
	{
		int chance = MIN(1600, chances[i]);
		deciturns[i] = chance ? (16000 / chance) : 0;
	}

	mem_free(gear);
	return TRUE;
}

/**
 * Describe objects that can be used for digging.
 */
static bool describe_digger(textblock *tb, const object_type *o_ptr)
{
	int i;
	int deciturns[DIGGING_MAX];
	static const char *names[4] = { "rubble", "magma veins", "quartz veins", "granite" };

	/* Get useful info or print nothing */
	if (!obj_known_digging((object_type *)o_ptr, deciturns)) return FALSE;

	for (i = DIGGING_RUBBLE; i < DIGGING_DOORS; i++)
	{
		if (i == 0 && deciturns[0] > 0) {
			if (tval_is_melee_weapon(o_ptr))
				textblock_append(tb, "Clears ");
			else
				textblock_append(tb, "With this item, your current weapon clears ");
		}

		if (i == 3 || (i != 0 && deciturns[i] == 0))
			textblock_append(tb, "and ");

		if (deciturns[i] == 0) {
			textblock_append_c(tb, TERM_L_RED, "doesn't affect ");
			textblock_append(tb, "%s.\n", names[i]);
			break;
		}

		textblock_append(tb, "%s in ", names[i]);

		if (deciturns[i] == 10) {
			textblock_append_c(tb, TERM_L_GREEN, "1 ");
		} else if (deciturns[i] < 100) {
			textblock_append_c(tb, TERM_GREEN, "%d.%d ", deciturns[i]/10, deciturns[i]%10);
		} else {
			textblock_append_c(tb, (deciturns[i] < 1000) ? TERM_YELLOW : TERM_RED,
			           "%d ", (deciturns[i]+5)/10);
		}

		textblock_append(tb, "turn%s%s", deciturns[i] == 10 ? "" : "s",
				(i == 3) ? ".\n" : ", ");
	}

	return TRUE;
}

/**
 * Gives the known nutritional value of the given object.
 *
 * Returns the number of player deciturns it will nourish for or -1 if 
 * the exact value not known.
 */
static int obj_known_food(const object_type *o_ptr)
{
	if (tval_can_have_nourishment(o_ptr) && o_ptr->pval) {
		if (object_is_known(o_ptr)) {
			return o_ptr->pval / 2;
		} else {
			return OBJ_KNOWN_PRESENT;
		}
	}

	return 0;
}

/**
 * Describes a food item
 */
static bool describe_food(textblock *tb, const object_type *o_ptr,
		bool subjective)
{
	int nourishment = obj_known_food(o_ptr);

	if (nourishment) {
		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[player->state.speed];
		if (!subjective) multiplier = 10;

		if (nourishment == OBJ_KNOWN_PRESENT) {
			textblock_append(tb, "Provides some nourishment.\n");
		} else {
			textblock_append(tb, "Nourishes for around ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", nourishment *
				multiplier / 10);
			textblock_append(tb, " turns.\n");
		}

		return TRUE;
	}

	return FALSE;
}


/**
 * Gives the known light-sourcey characteristics of the given object.
 *
 * Fills in the radius of the light in `rad`, whether it uses fuel and
 * how many turns light it can refuel in similar items.
 *
 * Return FALSE if the object is not known to be a light source (which 
 * includes it not actually being a light source).
 */
static bool obj_known_light(const object_type *o_ptr, oinfo_detail_t mode, int *rad, bool *uses_fuel, int *refuel_turns)
{
	bitflag flags[OF_SIZE];
	bool no_fuel;
	bool is_light = tval_is_light(o_ptr);

	get_known_flags(o_ptr, mode, flags);

	if (!is_light && (o_ptr->modifiers[OBJ_MOD_LIGHT] <= 0))
		return FALSE;

	/* Prevent unidentified objects (especially artifact lights) from showing
	 * bad radius and refueling info. */
	if (!object_is_known(o_ptr))
		return FALSE;

	/* Work out radius */
	*rad = o_ptr->modifiers[OBJ_MOD_LIGHT];

	no_fuel = of_has(flags, OF_NO_FUEL) ? TRUE : FALSE;

	if (no_fuel || o_ptr->artifact) {
		*uses_fuel = FALSE;
	} else {
		*uses_fuel = TRUE;
	}

	if (is_light && of_has(flags, OF_TAKES_FUEL)) {
		*refuel_turns = FUEL_LAMP;
	} else {
		*refuel_turns = 0;
	}

	return TRUE;
}

/**
 * Describe things that look like lights.
 */
static bool describe_light(textblock *tb, const object_type *o_ptr,
		oinfo_detail_t mode)
{
	int rad = 0;
	bool uses_fuel = FALSE;
	int refuel_turns = 0;

	bool terse = mode & OINFO_TERSE;

	if (!obj_known_light(o_ptr, mode, &rad, &uses_fuel, &refuel_turns))
		return FALSE;

	textblock_append(tb, "Radius ");
	textblock_append_c(tb, TERM_L_GREEN, format("%d", rad));
	textblock_append(tb, " light.");

	if (!o_ptr->artifact && !uses_fuel)
		textblock_append(tb, "  No fuel required.");

	if (!terse) {
		if (refuel_turns)
			textblock_append(tb, "  Refills other lanterns up to %d turns of fuel.", refuel_turns);
		else
			textblock_append(tb, "  Cannot be refueled.");
	}

	textblock_append(tb, "\n");

	return TRUE;
}


/**
 * Gives the known effects of using the given item.
 *
 * Fills in:
 *  - the effect id, or OBJ_KNOWN_PRESENT if there is an effect but details
 *    are unknown
 *  - whether the effect can be aimed
 *  -  the minimum and maximum time in game turns for the item to recharge 
 *     (or zero if it does not recharge)
 *  - the percentage chance of the effect failing when used
 *
 * Return FALSE if the object has no effect.
 */
static bool obj_known_effect(const object_type *o_ptr, int *effect, bool *aimed, int *min_recharge, int *max_recharge, int *failure_chance)
{
	random_value timeout = {0, 0, 0, 0};

	*effect = 0;
	*min_recharge = 0;
	*max_recharge = 0;
	*failure_chance = 0;
	*aimed = FALSE;

	if (object_effect_is_known(o_ptr)) {
		*effect = o_ptr->effect;
		timeout = o_ptr->time;
	} else if (object_effect(o_ptr)) {
		/* Don't know much - be vague */
		*effect = OBJ_KNOWN_PRESENT;

		if (!o_ptr->artifact && effect_aim(o_ptr->effect))
			*aimed = TRUE;
					
		return TRUE;
	} else {
		/* No effect - no info */
		return FALSE;
	}
	
	if (randcalc(timeout, 0, MAXIMISE) > 0)	{
		*min_recharge = randcalc(timeout, 0, MINIMISE);
		*max_recharge = randcalc(timeout, 0, MAXIMISE);
	}

	if (tval_is_food(o_ptr) || tval_is_potion(o_ptr) || tval_is_scroll(o_ptr)) {
		*failure_chance = 0;
	} else {
		*failure_chance = get_use_device_chance(o_ptr);
	}

	return TRUE;
}

/**
 * Describe an object's effect, if any.
 */
static bool describe_effect(textblock *tb, const object_type *o_ptr,
		bool only_artifacts, bool subjective)
{
	const char *desc;

	int effect = 0;
	bool aimed = FALSE;
	int min_time, max_time, failure_chance;

	/* Sometimes we only print artifact activation info */
	if (only_artifacts && !o_ptr->artifact)
		return FALSE;

	if (obj_known_effect(o_ptr, &effect, &aimed, &min_time, &max_time, &failure_chance) == FALSE)
		return FALSE;

	/* We don't know much */
	if (effect == OBJ_KNOWN_PRESENT) {
		if (aimed)
			textblock_append(tb, "It can be aimed.\n");
		else if (tval_is_edible(o_ptr))
			textblock_append(tb, "It can be eaten.\n");
		else if (tval_is_potion(o_ptr))
			textblock_append(tb, "It can be drunk.\n");
		else if (tval_is_scroll(o_ptr))
			textblock_append(tb, "It can be read.\n");
		else textblock_append(tb, "It can be activated.\n");

		return TRUE;
	}

	/* Obtain the description */
	desc = effect_desc(effect);
	if (!desc) return FALSE;

	if (aimed)
		textblock_append(tb, "When aimed, it ");
	else if (tval_is_food(o_ptr))
		textblock_append(tb, "When eaten, it ");
	else if (tval_is_potion(o_ptr))
		textblock_append(tb, "When quaffed, it ");
	else if (tval_is_scroll(o_ptr))
	    textblock_append(tb, "When read, it ");
	else
	    textblock_append(tb, "When activated, it ");

	/* Print a colourised description */
	do {
		if (isdigit((unsigned char) *desc))
			textblock_append_c(tb, TERM_L_GREEN, "%c", *desc);
		else
			textblock_append(tb, "%c", *desc);
	} while (*desc++);

	textblock_append(tb, ".\n");

	if (min_time || max_time)
	{
		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[player->state.speed];
		if (!subjective) multiplier = 10;

		textblock_append(tb, "Takes ");

		/* Correct for player speed */
		min_time *= multiplier / 10;
		max_time *= multiplier / 10;

		textblock_append_c(tb, TERM_L_GREEN, "%d", min_time);

		if (min_time != max_time)
		{
			textblock_append(tb, " to ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", max_time);
		}

		textblock_append(tb, " turns to recharge");
		if (subjective && player->state.speed != 110)
			textblock_append(tb, " at your current speed");

		textblock_append(tb, ".\n");
	}

	if (failure_chance > 0) {
		textblock_append(tb, "Your chance of success is %d.%d%%\n", 
			(1000 - failure_chance) / 10, (1000 - failure_chance) % 10);
	}

	return TRUE;
}

/**
 * Describe an item's origin
 */
static bool describe_origin(textblock *tb, const object_type *o_ptr, bool terse)
{
	char origin_text[80];

	/* Only give this info in chardumps if wieldable */
	if (terse && !obj_can_wear(o_ptr))
		return FALSE;

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

/**
 * Print an item's flavour text.
 *
 * \param tb is the textblock to which we are adding.
 * \param o_ptr is the object we are describing.
 * \param ego is whether we're describing an ego template (as opposed to a
 * real object)
 */
static void describe_flavor_text(textblock *tb, const object_type *o_ptr,
	bool ego)
{
	/* Display the known artifact description */
	if (!OPT(birth_randarts) && o_ptr->artifact &&
			object_is_known(o_ptr) && o_ptr->artifact->text)
		textblock_append(tb, "%s\n\n", o_ptr->artifact->text);

	/* Display the known object description */
	else if (object_flavor_is_aware(o_ptr) || object_is_known(o_ptr) || ego)
	{
		bool did_desc = FALSE;

		if (!ego && o_ptr->kind->text)
		{
			textblock_append(tb, "%s", o_ptr->kind->text);
			did_desc = TRUE;
		}

		/* Display an additional ego-item description */
		if ((ego || object_ego_is_visible(o_ptr)) && o_ptr->ego->text)
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

/**
 * Describe random properties that an ego item may have
 */
static bool describe_ego(textblock *tb, const struct ego_item *ego)
{
	int i, num = 3;

	/* Hackish */
	for (i = 0; i < 3; i++) {
		if (kf_has(ego->kind_flags, KF_RAND_HI_RES + i))
			num = i;
	}

	if (num < 3)
	{
		const char *xtra[] = { "sustain", "higher resistance", "ability" };
		textblock_append(tb, "It provides one random %s.  ", xtra[num]);

		return TRUE;
	}

	return FALSE;
}


/**
 * Output object information
 */
static textblock *object_info_out(const object_type *o_ptr, int mode)
{
	bitflag flags[OF_SIZE];
	struct element_info el_info[N_ELEMENTS(elements)];
	bool something = FALSE;
	bool known = object_all_but_flavor_is_known(o_ptr);

	bool terse = mode & OINFO_TERSE;
	bool subjective = mode & OINFO_SUBJ;
	bool ego = mode & OINFO_EGO;
	textblock *tb = textblock_new();

	/* Unaware objects get simple descriptions */
	if (o_ptr->marked == MARK_AWARE) {
		textblock_append(tb, "\n\nYou do not know what this is.\n");
		return tb;
	}
	
	/* Grab the object flags */
	get_known_flags(o_ptr, mode, flags);

	/* Grab the element info */
	get_known_elements(o_ptr, mode, el_info);

	if (subjective) describe_origin(tb, o_ptr, terse);
	if (!terse) describe_flavor_text(tb, o_ptr, ego);

	if (!known)
	{
		textblock_append(tb, "You do not know the full extent of this item's powers.\n");
		something = TRUE;
	}

	if (describe_curses(tb, o_ptr, flags)) something = TRUE;
	if (describe_stats(tb, o_ptr, mode)) something = TRUE;
	if (describe_slays(tb, o_ptr)) something = TRUE;
	if (describe_brands(tb, o_ptr)) something = TRUE;
	if (describe_elements(tb, el_info)) something = TRUE;
	if (describe_protects(tb, flags)) something = TRUE;
	if (describe_ignores(tb, el_info)) something = TRUE;
	if (describe_hates(tb, el_info)) something = TRUE;
	if (describe_sustains(tb, flags)) something = TRUE;
	if (describe_misc_magic(tb, flags)) something = TRUE;
	if (describe_light(tb, o_ptr, mode)) something = TRUE;
	if (ego && describe_ego(tb, o_ptr->ego)) something = TRUE;
	if (something) textblock_append(tb, "\n");

	/* Skip all the very specific information where we are giving general
	   ego knowledge rather than for a single item - abilities can vary */
	if (!ego) {
		if (describe_effect(tb, o_ptr, terse, subjective)) {
			something = TRUE;
			textblock_append(tb, "\n");
		}
		
		if (subjective && describe_combat(tb, o_ptr)) {
			something = TRUE;
			textblock_append(tb, "\n");
		}
		
		if (!terse && describe_food(tb, o_ptr, subjective)) something = TRUE;
		if (!terse && subjective && describe_digger(tb, o_ptr)) something = TRUE;
	}

	/* Don't append anything in terse (for chararacter dump) */
	if (!something && !terse)
		textblock_append(tb, "\n\nThis item does not seem to possess any special abilities.");

	return tb;
}


/**
 * Provide information on an item, including how it would affect the current
 * player's state.
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
	size_t i;

	for (i = 0; i < z_info->k_max; i++) {
		kind = &k_info[i];
		if (!kind->name)
			continue;
		if (i == ego->poss_items->kidx)
			break;
	}

	obj.kind = kind;
	obj.tval = kind->tval;
	obj.sval = kind->sval;
	obj.ego = ego;
	ego_apply_magic(&obj, 0);

	object_know_all_but_flavor(&obj);

	return object_info_out(&obj, OINFO_NONE | OINFO_EGO);
}



/**
 * Provide information on an item suitable for writing to the character dump
 * - keep it brief.
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
	textblock *tb = object_info_out(o_ptr, OINFO_NONE);
	textblock_to_file(tb, f, 0, wrap);
	textblock_free(tb);
}
