/**
 * \file obj-info.c
 * \brief Object description code.
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
#include "cmds.h"
#include "effects.h"
#include "game-world.h"
#include "init.h"
#include "monster.h"
#include "mon-summon.h"
#include "mon-util.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-info.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "project.h"
#include "z-textblock.h"

/**
 * Describes the number of blows possible for given stat bonuses
 */
struct blow_info {
	int str_plus;
	int dex_plus;  
	int centiblows;
};

/**
 * ------------------------------------------------------------------------
 * Data tables
 * ------------------------------------------------------------------------ */

static const struct origin_type {
	int type;
	int args;
	const char *desc;
} origins[] = {
	#define ORIGIN(a, b, c) { ORIGIN_##a, b, c },
	#include "list-origins.h"
	#undef ORIGIN
};

static struct {
	int index;
	int args;
	int efinfo_flag;
	const char *desc;
} base_descs[] = {
	{ EF_NONE, 0, EFINFO_NONE, "" },
	#define EFFECT(x, a, b, c, d, e) { EF_##x, c, d, e },
	#include "list-effects.h"
	#undef EFFECT
};


/**
 * ------------------------------------------------------------------------
 * List-writiing utility code
 * ------------------------------------------------------------------------ */

/**
 * Given an array of strings, as so:
 *  { "intelligence", "fish", "lens", "prime", "number" },
 *
 * ... output a list like "intelligence, fish, lens, prime, number.\n".
 */
static void info_out_list(textblock *tb, const char *list[], size_t count)
{
	size_t i;

	for (i = 0; i < count; i++) {
		textblock_append(tb, list[i]);
		if (i != (count - 1)) textblock_append(tb, ", ");
	}

	textblock_append(tb, ".\n");
}


/**
 * Fills recepticle with all the elements that correspond to the given `list`.
 */
static size_t element_info_collect(const bool list[], const char *recepticle[])
{
	int i, count = 0;

	for (i = 0; i < ELEM_MAX; i++) {
		if (list[i])
			recepticle[count++] = projections[i].name;
	}

	return count;
}


/**
 * ------------------------------------------------------------------------
 * Code that makes use of the data tables to describe aspects of an 
 * object's information
 * ------------------------------------------------------------------------ */

/**
 * Describe an item's curses.
 */
static bool describe_curses(textblock *tb, const struct object *obj,
		const bitflag flags[OF_SIZE])
{
	int i;
	struct curse_data *c = obj->known->curses;

	if (!c)
		return false;
	for (i = 1; i < z_info->curse_max; i++) {
		if (c[i].power) {
			textblock_append(tb, "It ");
			textblock_append_c(tb, COLOUR_L_RED, curses[i].desc);
			if (c[i].power == 100) {
				textblock_append(tb, "; this curse cannot be removed");
			}
			textblock_append(tb, ".\n");
		}
	}
	/* Say if curse removal has been tried */
	if (of_has(obj->flags, OF_FRAGILE)) {
		textblock_append(tb, "Attempting to uncurse it may destroy it.\n");
	}

	return true;
}


/**
 * Describe stat modifications.
 */
static bool describe_stats(textblock *tb, const struct object *obj,
						   oinfo_detail_t mode)
{
	size_t count = 0, i;
	bool detail = false;

	/* Don't give exact plusses for faked ego items as each real one will
	 * be different */
	bool suppress_details = mode & (OINFO_EGO | OINFO_FAKE) ? true : false;

	/* Fact of but not size of mods is known for egos and flavoured items
	 * the player is aware of */
	bool known_effect = false;
	if (obj->known->ego)
		known_effect = true;
	if (tval_can_have_flavor_k(obj->kind) && object_flavor_is_aware(obj))
		known_effect = true;

	/* See what we've got */
	for (i = 0; i < OBJ_MOD_MAX; i++)
		if (obj->known->modifiers[i]) {
			count++;
			detail = true;
		}

	if (!count)
		return false;

	for (i = 0; i < OBJ_MOD_MAX; i++) {
		const char *desc = lookup_obj_property(OBJ_PROPERTY_MOD, i)->name;
		int val = obj->known->modifiers[i];
		if (!val) continue;

		/* Actual object */
		if (detail && !suppress_details) {
			int attr = (val > 0) ? COLOUR_L_GREEN : COLOUR_RED;
			textblock_append_c(tb, attr, "%+i %s.\n", val, desc);
		} else if (known_effect)
			/* Ego type or jewellery description */
			textblock_append(tb, "Affects your %s\n", desc);
	}

	return true;
}


/**
 * Describe immunities, resistances and vulnerabilities granted by an object.
 */
static bool describe_elements(textblock *tb,
							  const struct element_info el_info[])
{
	const char *i_descs[ELEM_MAX];
	const char *r_descs[ELEM_MAX];
	const char *v_descs[ELEM_MAX];
	size_t i, count;

	bool list[ELEM_MAX], prev = false;

	/* Immunities */
	for (i = 0; i < ELEM_MAX; i++)
		list[i] = (el_info[i].res_level == 3);
	count = element_info_collect(list, i_descs);
	if (count) {
		textblock_append(tb, "Provides immunity to ");
		info_out_list(tb, i_descs, count);
		prev = true;
	}

	/* Resistances */
	for (i = 0; i < ELEM_MAX; i++)
		list[i] = (el_info[i].res_level == 1);
	count = element_info_collect(list, r_descs);
	if (count) {
		textblock_append(tb, "Provides resistance to ");
		info_out_list(tb, r_descs, count);
		prev = true;
	}

	/* Vulnerabilities */
	for (i = 0; i < ELEM_MAX; i++)
		list[i] = (el_info[i].res_level == -1);
	count = element_info_collect(list, v_descs);
	if (count) {
		textblock_append(tb, "Makes you vulnerable to ");
		info_out_list(tb, v_descs, count);
		prev = true;
	}

	return prev;
}


/**
 * Describe protections granted by an object.
 */
static bool describe_protects(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *p_descs[OF_MAX];
	int i, count = 0;

	/* Protections */
	for (i = 1; i < OF_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
		if (prop->subtype != OFT_PROT) continue;
		if (of_has(flags, prop->index)) {
			p_descs[count++] = prop->desc;
		}
	}

	if (!count)
		return false;

	textblock_append(tb, "Provides protection from ");
	info_out_list(tb, p_descs, count);

	return  true;
}

/**
 * Describe elements an object ignores.
 */
static bool describe_ignores(textblock *tb, const struct element_info el_info[])
{
	const char *descs[ELEM_MAX];
	size_t i, count;
	bool list[ELEM_MAX];

	for (i = 0; i < ELEM_MAX; i++)
		list[i] = (el_info[i].flags & EL_INFO_IGNORE);
	count = element_info_collect(list, descs);

	if (!count)
		return false;

	textblock_append(tb, "Cannot be harmed by ");
	info_out_list(tb, descs, count);

	return true;
}

/**
 * Describe elements that damage or destroy an object.
 */
static bool describe_hates(textblock *tb, const struct element_info el_info[])
{
	const char *descs[ELEM_MAX];
	size_t i, count = 0;
	bool list[ELEM_MAX];

	for (i = 0; i < ELEM_MAX; i++)
		list[i] = (el_info[i].flags & EL_INFO_HATES);
	count = element_info_collect(list, descs);

	if (!count)
		return false;

	textblock_append(tb, "Can be destroyed by ");
	info_out_list(tb, descs, count);

	return true;
}


/**
 * Describe stat sustains.
 */
static bool describe_sustains(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[STAT_MAX];
	int i, count = 0;

	for (i = 0; i < STAT_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_STAT, i);
		if (of_has(flags, sustain_flag(prop->index)))
			descs[count++] = prop->name;
	}

	if (!count)
		return false;

	textblock_append(tb, "Sustains ");
	info_out_list(tb, descs, count);

	return true;
}


/**
 * Describe miscellaneous powers.
 */
static bool describe_misc_magic(textblock *tb, const bitflag flags[OF_SIZE])
{
	int i;
	bool printed = false;

	for (i = 1; i < OF_MAX; i++) {
		struct obj_property *prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
		if ((prop->subtype != OFT_MISC)  && (prop->subtype != OFT_MELEE) &&
			(prop->subtype != OFT_BAD)) continue;
		if (of_has(flags, prop->index)) {
			textblock_append(tb, "%s.  ", prop->desc);
			printed = true;
		}
	}

	if (printed)
		textblock_append(tb, "\n");

	return printed;
}


/**
 * Describe slays and brands on weapons
 */
static bool describe_slays(textblock *tb, const struct object *obj)
{
	int i, count = 0;
	bool *s = obj->known->slays;

	if (!s) return false;

	if (tval_is_weapon(obj) || tval_is_fuel(obj))
		textblock_append(tb, "Slays ");
	else
		textblock_append(tb, "It causes your melee attacks to slay ");

	for (i = 1; i < z_info->slay_max; i++) {
		if (s[i]) {
			count++;
		}
	}

	assert(count >= 1);
	for (i = 1; i < z_info->slay_max; i++) {
		if (!s[i]) continue;

		textblock_append(tb, slays[i].name);
		if (slays[i].multiplier > 3)
			textblock_append(tb, " (powerfully)");
		if (count > 1)
			textblock_append(tb, ", ");
		else
			textblock_append(tb, ".\n");
		count--;
	}

	return true;
}

/**
 * Describe slays and brands on weapons
 */
static bool describe_brands(textblock *tb, const struct object *obj)
{
	int i, count = 0;
	bool *b = obj->known->brands;

	if (!b) return false;

	if (tval_is_weapon(obj) || tval_is_fuel(obj))
		textblock_append(tb, "Branded with ");
	else
		textblock_append(tb, "It brands your melee attacks with ");

	for (i = 1; i < z_info->brand_max; i++) {
		if (b[i]) {
			count++;
		}
	}

	assert(count >= 1);
	for (i = 1; i < z_info->brand_max; i++) {
		if (!b[i]) continue;

		if (brands[i].multiplier < 3)
			textblock_append(tb, "weak ");
		textblock_append(tb, brands[i].name);
		if (count > 1)
			textblock_append(tb, ", ");
		else
			textblock_append(tb, ".\n");
		count--;
	}

	return true;
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
static void calculate_melee_crits(struct player_state *state, int weight,
		int plus, int *mult, int *add, int *div)
{
	int k, to_crit = weight + 5 * (state->to_h + plus) + 3 * player->lev;
	to_crit = MIN(5000, MAX(0, to_crit));

	*mult = *add = 0;

	for (k = weight; k < weight + 650; k++) {
		if (k <  400) { *mult += 4; *add += 10; continue; }
		if (k <  700) { *mult += 4; *add += 20; continue; }
		if (k <  900) { *mult += 6; *add += 30; continue; }
		if (k < 1300) { *mult += 6; *add += 40; continue; }
		                *mult += 8; *add += 40;
	}

	/* Scale the output to a reasonable size to prevent integer overflow. */
	*mult = 100 + to_crit * (*mult - 1300) / (50 * 1300);
	*add  = *add * to_crit / (500 * 50);
	*div  = 100;
}

/**
 * Account for criticals in the calculation of melee prowess for O-combat;
 * crit chance * average number of dice added
 *
 * Return value is 100x number of dice
 */
static int o_calculate_melee_crits(struct player_state state,
								   const struct object *obj)
{
	int dice = 0;
	int chance = BTH_PLUS_ADJ * (state.to_h + obj->known->to_h) +
		state.skills[SKILL_TO_HIT_MELEE];
	chance = (100 * chance) / (chance + 240);
	dice = (537 * chance) / 240;

	return dice;
}

/**
 * Missile crits follow the same approach as melee crits.
 */
static void calculate_missile_crits(struct player_state *state, int weight,
		int plus, int *mult, int *add, int *div)
{
	int k, to_crit = weight + 4 * (state->to_h + plus) + 2 * player->lev;
	to_crit = MIN(5000, MAX(0, to_crit));

	*mult = *add = 0;

	for (k = weight; k < weight + 500; k++) {
		if (k <  500) { *mult += 2; *add +=  5; continue; }
		if (k < 1000) { *mult += 2; *add += 10; continue; }
		                *mult += 3; *add += 15;
	}

	*mult = 100 + to_crit * (*mult - 500) / (500 * 50);
	*add  = *add * to_crit / (500 * 50);
	*div  = 100;
}

/**
 * Missile crits follow the same approach as melee crits.
 */
static int o_calculate_missile_crits(struct player_state state,
									 const struct object *obj,
									 const struct object *launcher)
{
	int dice = 0;
	int bonus = state.to_h + obj->known->to_h
		+ (launcher ? launcher->known->to_h : 0);
	int chance = BTH_PLUS_ADJ * bonus;
	if (launcher) {
		chance += state.skills[SKILL_TO_HIT_BOW];
	} else {
		chance += state.skills[SKILL_TO_HIT_THROW];
		chance *= 3 / 2;
	}
	chance = (100 * chance) / (chance + 360);
	dice = (569 * chance) / 500;

	return dice;
}

/**
 * Get the object flags the player should know about for the given object/
 * viewing mode combination.
 */
static void get_known_flags(const struct object *obj, const oinfo_detail_t mode,
							bitflag flags[OF_SIZE])
{
	/* Grab the object flags */
	if (mode & OINFO_EGO) {
			object_flags(obj, flags);
	} else {
		object_flags_known(obj, flags);

		/* Don't include base flags when terse */
		if (mode & OINFO_TERSE)
			of_diff(flags, obj->kind->base->flags);
	}
}

/**
 * Get the object element info the player should know about for the given
 * object/viewing mode combination.
 */
static void get_known_elements(const struct object *obj,
							   const oinfo_detail_t mode,
							   struct element_info el_info[])
{
	size_t i;

	/* Grab the element info */
	for (i = 0; i < ELEM_MAX; i++) {
		/* Report fake egos or known element info */
		if (player->obj_k->el_info[i].res_level || (mode & OINFO_SPOIL))
			el_info[i].res_level = obj->known->el_info[i].res_level;
		else
			el_info[i].res_level = 0;
		el_info[i].flags = obj->known->el_info[i].flags;

		/* Ignoring an element: */
		if (obj->el_info[i].flags & EL_INFO_IGNORE) {
			/* If the object is usually destroyed, mention the ignoring; */
			if (obj->el_info[i].flags & EL_INFO_HATES)
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
 * Fills in whether the object is too heavy to wield effectively,
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
static int obj_known_blows(const struct object *obj, int max_num,
						   struct blow_info possible_blows[])
{
	int str_plus, dex_plus, old_blows = 0;
	int str_faster = -1, str_done = -1;
	int dex_plus_bound;
	int str_plus_bound;

	struct player_state state;

	int weapon_slot = slot_by_name(player, "weapon");
	struct object *current_weapon = slot_object(player, weapon_slot);
	int num = 0;

	/* Not a weapon - no blows! */
	if (!tval_is_melee_weapon(obj)) return 0;

	/* Pretend we're wielding the object */
	player->body.slots[weapon_slot].obj = (struct object *) obj;

	/* Calculate the player's hypothetical state */
	memcpy(&state, &player->state, sizeof(state));
	state.stat_ind[STAT_STR] = 0; //Hack - NRM
	state.stat_ind[STAT_DEX] = 0; //Hack - NRM
	calc_bonuses(player, &state, true, false);

	/* First entry is always the current num of blows. */
	possible_blows[num].str_plus = 0;
	possible_blows[num].dex_plus = 0;
	possible_blows[num].centiblows = state.num_blows;
	num++;

	/* Check to see if extra STR or DEX would yield extra blows */
	old_blows = state.num_blows;
	dex_plus_bound = STAT_RANGE - state.stat_ind[STAT_DEX];
	str_plus_bound = STAT_RANGE - state.stat_ind[STAT_STR];

	/* Re-calculate with increased stats */
	for (dex_plus = 0; dex_plus < dex_plus_bound; dex_plus++) {
		for (str_plus = 0; str_plus < str_plus_bound; str_plus++) {
			int new_blows = 0;

			/* Unlikely */
			if (num == max_num) {
				player->body.slots[weapon_slot].obj = current_weapon;
				return num;
			}

			state.stat_ind[STAT_STR] = str_plus; //Hack - NRM
			state.stat_ind[STAT_DEX] = dex_plus; //Hack - NRM
			calc_bonuses(player, &state, true, false);
			new_blows = state.num_blows;

			/* Test to make sure that this extra blow is a
			 * new str/dex combination, not a repeat */
			if (((new_blows - new_blows % 10) > (old_blows - old_blows % 10)) &&
				(str_plus < str_done || str_done == -1)) {
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
			 * take a little less energy */
			if ((new_blows > old_blows) &&
				(str_plus < str_faster || str_faster == -1) &&
				(str_plus < str_done || str_done == -1)) {
				possible_blows[num].str_plus = str_plus;
				possible_blows[num].dex_plus = dex_plus;
				possible_blows[num].centiblows = new_blows;
				num++;

				str_faster = str_plus;
			}
		}
	}

	/* Stop pretending */
	player->body.slots[weapon_slot].obj = current_weapon;

	return num;
}


/**
 * Describe blows.
 */
static bool describe_blows(textblock *tb, const struct object *obj)
{
	int i;
	struct blow_info blow_info[STAT_RANGE * 2]; /* (Very) theoretical max */
	int num_entries = 0;

	num_entries = obj_known_blows(obj, STAT_RANGE * 2, blow_info);
	if (num_entries == 0) return false;

	/* First entry is always current blows (+0, +0) */
	textblock_append_c(tb, COLOUR_L_GREEN, "%d.%d ",
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

	return true;
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
static bool obj_known_damage(const struct object *obj, int *normal_damage,
							 int *brand_damage, int *slay_damage,
							 bool *nonweap_slay, bool throw)
{
	int i;
	int dice, sides, dam, total_dam, plus = 0;
	int xtra_postcrit = 0, xtra_precrit = 0;
	int crit_mult, crit_div, crit_add;
	int old_blows = 0;
	bool *total_brands;
	bool *total_slays;
	bool has_brands_or_slays = false;

	struct object *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(obj) && !throw;
	bool ammo   = (player->state.ammo_tval == obj->tval) && (bow);
	int melee_adj_mult = ammo ? 0 : 1;
	int multiplier = 1;

	struct player_state state;
	int weapon_slot = slot_by_name(player, "weapon");
	struct object *current_weapon = slot_object(player, weapon_slot);

	/* Pretend we're wielding the object if it's a weapon */
	if (weapon)
		player->body.slots[weapon_slot].obj = (struct object *) obj;

	/* Calculate the player's hypothetical state */
	memcpy(&state, &player->state, sizeof(state));
	state.stat_ind[STAT_STR] = 0; //Hack - NRM
	state.stat_ind[STAT_DEX] = 0; //Hack - NRM
	calc_bonuses(player, &state, true, false);

	/* Stop pretending */
	player->body.slots[weapon_slot].obj = current_weapon;

	/* Finish if dice not known */
	dice = obj->known->dd;
	sides = obj->known->ds;
	if (!dice || !sides) return false;

	/* Calculate damage */
	dam = ((sides + 1) * dice * 5);

	if (weapon)	{
		xtra_postcrit = state.to_d * 10;
		xtra_precrit += obj->known->to_d * 10;
		plus += obj->known->to_h;

		calculate_melee_crits(&state, obj->weight, plus, &crit_mult, &crit_add,
							  &crit_div);

		old_blows = state.num_blows;
	} else if (ammo) {
		plus += obj->known->to_h;

		calculate_missile_crits(&player->state, obj->weight, plus, &crit_mult,
								&crit_add, &crit_div);

		dam += (obj->known->to_d * 10);
		dam += (bow->known->to_d * 10);
	} else {
		plus += obj->known->to_h;

		calculate_missile_crits(&player->state, obj->weight, plus, &crit_mult,
								&crit_add, &crit_div);

		dam += (obj->known->to_d * 10);
		dam *= 1 + player->lev / 12;
	}

	if (ammo) multiplier = player->state.ammo_mult;

	/* Get the brands */
	total_brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	copy_brands(&total_brands, obj->known->brands);
	if (ammo && bow->known)
		copy_brands(&total_brands, bow->known->brands);

	/* Get the slays */
	total_slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	copy_slays(&total_slays, obj->known->slays);
	if (ammo && bow->known)
		copy_slays(&total_slays, bow->known->slays);

	/* Melee weapons may get slays and brands from other items */
	*nonweap_slay = false;
	if (weapon)	{
		for (i = 2; i < player->body.count; i++) {
			struct object *slot_obj = slot_object(player, i);
			if (!slot_obj)
				continue;

			if (slot_obj->known->brands || slot_obj->known->slays)
				*nonweap_slay = true;
			else
				continue;

			/* Replace the old lists with new ones */
			copy_brands(&total_brands, slot_obj->known->brands);
			copy_slays(&total_slays, slot_obj->known->slays);
		}
	}

	/* Get damage for each brand on the objects */
	for (i = 1; i < z_info->brand_max; i++) {
		/* Must have the brand, possibly from a spell */
		if (player_has_temporary_brand(i)) {
			*nonweap_slay = true;
		} else if (!total_brands[i]) {
			continue;
		}
		has_brands_or_slays = true;

		/* Include bonus damage and brand in stated average */
		total_dam = dam * (multiplier + brands[i].multiplier - melee_adj_mult)
			+ xtra_precrit;
		total_dam = (total_dam * crit_mult + crit_add) / crit_div;
		total_dam += xtra_postcrit;

		if (weapon) {
			total_dam = (total_dam * old_blows) / 100;
		} else if (ammo) {
			total_dam *= player->state.num_shots;
			total_dam /= 10;
		}

		brand_damage[i] = total_dam;
	}

	/* Get damage for each slay on the objects */
	for (i = 1; i < z_info->slay_max; i++) {
		/* Must have the slay, possibly from a spell */
		if (player_has_temporary_slay(i)) {
			*nonweap_slay = true;
		} else if (!total_slays[i]) {
			continue;
		}
		has_brands_or_slays = true;

		/* Include bonus damage and slay in stated average */
		total_dam = dam * (multiplier + slays[i].multiplier - melee_adj_mult)
			+ xtra_precrit;
		total_dam = (total_dam * crit_mult + crit_add) / crit_div;
		total_dam += xtra_postcrit;

		if (weapon) {
			total_dam = (total_dam * old_blows) / 100;
		} else if (ammo) {
			total_dam *= player->state.num_shots;
			total_dam /= 10;
		}

		slay_damage[i] = total_dam;
	}

	/* Include bonus damage in stated average */
	total_dam = dam * multiplier + xtra_precrit;
	total_dam = (total_dam * crit_mult + crit_add) / crit_div;
	total_dam += xtra_postcrit;

	/* Normal damage, not considering brands or slays */
	if (weapon) {
		total_dam = (total_dam * old_blows) / 100;
	} else if (ammo) {
		total_dam *= player->state.num_shots;
		total_dam /= 10;
	}

	*normal_damage = total_dam;

	mem_free(total_brands);
	mem_free(total_slays);
	return has_brands_or_slays;
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
static bool o_obj_known_damage(const struct object *obj, int *normal_damage,
								 int *brand_damage, int *slay_damage,
							   bool *nonweap_slay, bool throw)
{
	int i;
	int dice, sides, die_average, total_dam;
	int deadliness = obj->known->to_d;
	int old_blows = 0;
	bool *total_brands;
	bool *total_slays;
	bool has_brands_or_slays = false;

	struct object *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(obj) && !throw;
	bool ammo   = (player->state.ammo_tval == obj->tval) && (bow);
	int multiplier = 1;

	struct player_state state;
	int weapon_slot = slot_by_name(player, "weapon");
	struct object *current_weapon = slot_object(player, weapon_slot);

	/* Pretend we're wielding the object if it's a weapon */
	if (weapon)
		player->body.slots[weapon_slot].obj = (struct object *) obj;

	/* Calculate the player's hypothetical state */
	memcpy(&state, &player->state, sizeof(state));
	state.stat_ind[STAT_STR] = 0; //Hack - NRM
	state.stat_ind[STAT_DEX] = 0; //Hack - NRM
	calc_bonuses(player, &state, true, false);

	/* Stop pretending */
	player->body.slots[weapon_slot].obj = current_weapon;

	/* Finish if dice not known */
	dice = obj->known->dd * 100;
	sides = obj->known->ds;
	if (!dice || !sides) return false;

	/* Get the number of additional dice from criticals (x100) */
	if (weapon)	{
		dice += o_calculate_melee_crits(state, obj);
		old_blows = state.num_blows;
	} else if (ammo) {
		dice += o_calculate_missile_crits(player->state, obj, bow);
	} else {
		dice += o_calculate_missile_crits(player->state, obj, NULL);
		dice *= 2 + player->lev / 12;
	}

	if (ammo) multiplier = player->state.ammo_mult;

	/* Get the average value of a single damage die. (x10) */
	die_average = 5 * (sides + 1);

	/* Apply the launcher multiplier. */
	die_average *= multiplier;

	/* Apply deadliness to average. (100x inflation) */
	if (ammo) {
		deadliness = obj->known->to_d + bow->known->to_d + state.to_d;
	} else {
		deadliness = obj->known->to_d + state.to_d;
	}
	apply_deadliness(&die_average, MIN(deadliness, 150));

	/* Get the brands */
	total_brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	copy_brands(&total_brands, obj->known->brands);
	if (ammo && bow->known)
		copy_brands(&total_brands, bow->known->brands);

	/* Get the slays */
	total_slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	copy_slays(&total_slays, obj->known->slays);
	if (ammo && bow->known)
		copy_slays(&total_slays, bow->known->slays);

	/* Melee weapons may get slays and brands from other items */
	*nonweap_slay = false;
	if (weapon)	{
		for (i = 2; i < player->body.count; i++) {
			struct object *slot_obj = slot_object(player, i);
			if (!slot_obj)
				continue;

			if (slot_obj->known->brands || slot_obj->known->slays)
				*nonweap_slay = true;
			else
				continue;

			/* Replace the old lists with new ones */
			copy_brands(&total_brands, slot_obj->known->brands);
			copy_slays(&total_slays, slot_obj->known->slays);
		}
	}

	/* Increase die average for each brand on the objects */
	for (i = 1; i < z_info->brand_max; i++) {
		int brand_average, add = brands[i].o_multiplier - 10;

		/* Must have the brand */
		if (total_brands[i])
			has_brands_or_slays = true;
		else
			continue;

		/* Include brand in stated average (x10), deflate (/1000) */
		brand_average = die_average * brands[i].o_multiplier;
		brand_average /= 1000;

		/* Damage per hit is now dice * die average, (still x1000) */
		total_dam = (dice * brand_average);

		/* Now adjust for blows and shots and deflate again */
		if (weapon) {
			total_dam *= old_blows;
			total_dam /= 10000;
		} else if (ammo) {
			total_dam *= player->state.num_shots;
			total_dam /= 1000;
		} else {
			total_dam /= 100;
		}

		brand_damage[i] = total_dam + add;
	}

	/* Get damage for each slay on the objects */
	for (i = 1; i < z_info->slay_max; i++) {
		int slay_average, add = slays[i].o_multiplier - 10;

		/* Must have the slay */
		if (total_slays[i])
			has_brands_or_slays = true;
		else
			continue;

		/* Include slay in stated average (x10), deflate (/1000) */
		slay_average = die_average * slays[i].o_multiplier;
		slay_average /= 1000;

		/* Damage per hit is now dice * die average, (still x1000) */
		total_dam = (dice * slay_average);

		/* Now adjust for blows and shots and deflate again */
		if (weapon) {
			total_dam *= old_blows;
			total_dam /= 10000;
		} else if (ammo) {
			total_dam *= player->state.num_shots;
			total_dam /= 1000;
		} else {
			total_dam /= 100;
		}

		slay_damage[i] = total_dam + add;
	}

	/* Normal damage, not considering brands or slays */
	total_dam = (dice * die_average) / 1000;
	if (weapon) {
		total_dam *= old_blows;
		total_dam /= 1000;
	} else if (ammo) {
		total_dam *= player->state.num_shots;
		total_dam /= 100;
	} else {
		total_dam /= 10;
	}
	*normal_damage = total_dam;

	mem_free(total_brands);
	mem_free(total_slays);
	return has_brands_or_slays;
}


/**
 * Describe damage.
 */
static bool describe_damage(textblock *tb, const struct object *obj, bool throw)
{
	int i;
	bool nonweap_slay = false;
	int normal_damage = 0;
	int *brand_damage = mem_zalloc(z_info->brand_max * sizeof(int));
	int *slay_damage = mem_zalloc(z_info->slay_max * sizeof(int));

	/* Collect brands and slays */
	bool has_brands_or_slays = OPT(player, birth_percent_damage) ?
		o_obj_known_damage(obj, &normal_damage, brand_damage, slay_damage,
						   &nonweap_slay, throw) :
		obj_known_damage(obj, &normal_damage, brand_damage, slay_damage,
						 &nonweap_slay, throw);

	/* Mention slays and brands from other items */
	if (nonweap_slay)
		textblock_append(tb, "This weapon may benefit from one or more off-weapon brands or slays.\n");

	if (throw) {
		textblock_append(tb, "Average thrown damage: ");
	} else {
		textblock_append(tb, "Average damage/round: ");
	}

	if (has_brands_or_slays) {
		/* Output damage for creatures effected by the brands */
		for (i = 0; i < z_info->brand_max; i++) {
			if (brand_damage[i] <= 0) {
				continue;
			} else if (brand_damage[i] % 10) {
				textblock_append_c(tb, COLOUR_L_GREEN, "%d.%d",
								   brand_damage[i] / 10, brand_damage[i] % 10);
			} else {
				textblock_append_c(tb, COLOUR_L_GREEN, "%d",
								   brand_damage[i] / 10);
			}
			textblock_append(tb, " vs. creatures not resistant to %s, ",
							 brands[i].name);
		}

		/* Output damage for creatures effected by the slays */
		for (i = 0; i < z_info->slay_max; i++) {
			if (slay_damage[i] <= 0) {
				continue;
			} else if (slay_damage[i] % 10) {
				textblock_append_c(tb, COLOUR_L_GREEN, "%d.%d",
								   slay_damage[i] / 10, slay_damage[i] % 10);
			} else {
				textblock_append_c(tb, COLOUR_L_GREEN, "%d",
								   slay_damage[i] / 10);
			}
			textblock_append(tb, " vs. %s, ", slays[i].name);
		}

		textblock_append(tb, "and ");
	}

	if (normal_damage <= 0)
		textblock_append_c(tb, COLOUR_L_RED, "%d", 0);
	else if (normal_damage % 10)
		textblock_append_c(tb, COLOUR_L_GREEN, "%d.%d",
			   normal_damage / 10, normal_damage % 10);
	else
		textblock_append_c(tb, COLOUR_L_GREEN, "%d", normal_damage / 10);

	if (has_brands_or_slays) textblock_append(tb, " vs. others");
	textblock_append(tb, ".\n");

	mem_free(brand_damage);
	mem_free(slay_damage);
	return true;
}

/**
 * Gets miscellaneous combat information about the given object.
 *
 * Fills in whether there is a special effect when thrown in `thrown effect`,
 * the `range` in ft (or zero if not ammo), the percentage chance of breakage
 * and whether it is too heavy to be wielded effectively at the moment.
 */
static void obj_known_misc_combat(const struct object *obj, bool *thrown_effect,
								  int *range, int *break_chance, bool *heavy)
{
	struct object *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(obj);
	bool ammo   = (player->state.ammo_tval == obj->tval) && (bow);

	*thrown_effect = *heavy = false;
	*range = *break_chance = 0;

	if (!weapon && !ammo) {
		/* Potions can have special text */
		if (tval_is_potion(obj) && obj->dd != 0 && obj->ds != 0 &&
			object_flavor_is_aware(obj))
			*thrown_effect = true;
	}

	if (ammo)
		*range = 10 * MIN(6 + 2 * player->state.ammo_mult, z_info->max_range);

	/* Add breakage chance */
	*break_chance = breakage_chance(obj, true);

	/* Is the weapon too heavy? */
	if (weapon) {
		struct player_state state;
		int weapon_slot = slot_by_name(player, "weapon");
		struct object *current = equipped_item_by_slot_name(player, "weapon");

		/* Pretend we're wielding the object */
		player->body.slots[weapon_slot].obj = (struct object *) obj;

		/* Calculate the player's hypothetical state */
		memcpy(&state, &player->state, sizeof(state));
		state.stat_ind[STAT_STR] = 0; //Hack - NRM
		state.stat_ind[STAT_DEX] = 0; //Hack - NRM
		calc_bonuses(player, &state, true, false);

		/* Stop pretending */
		player->body.slots[weapon_slot].obj = current;

		/* Warn about heavy weapons */
		*heavy = state.heavy_wield;
	}
}


/**
 * Describe combat advantages.
 */
static bool describe_combat(textblock *tb, const struct object *obj)
{
	struct object *bow = equipped_item_by_slot_name(player, "shooting");
	bool weapon = tval_is_melee_weapon(obj);
	bool ammo   = (player->state.ammo_tval == obj->tval) && (bow);
	bool throwing_weapon = weapon && of_has(obj->flags, OF_THROWING);

	int range, break_chance;
	bool thrown_effect, heavy;

	obj_known_misc_combat(obj, &thrown_effect, &range, &break_chance, &heavy);

	if (!weapon && !ammo) {
		if (thrown_effect) {
			textblock_append(tb, "It can be thrown at creatures with damaging effect.\n");
			return true;
		} else
			return false;
	}

	textblock_append_c(tb, COLOUR_L_WHITE, "Combat info:\n");

	if (heavy)
		textblock_append_c(tb, COLOUR_L_RED, "You are too weak to use this weapon.\n");

	describe_blows(tb, obj);

	if (!weapon) { /* Ammo */
		textblock_append(tb, "Hits targets up to ");
		textblock_append_c(tb, COLOUR_L_GREEN, format("%d", range));
		textblock_append(tb, " feet away.\n");
	}

	describe_damage(tb, obj, false);
	if (throwing_weapon) {
		describe_damage(tb, obj, true);
	}

	if (ammo) {
		textblock_append_c(tb, COLOUR_L_GREEN, "%d%%", break_chance);
		textblock_append(tb, " chance of breaking upon contact.\n");
	}

	/* Something has been said */
	return true;
}


/**
 * Returns information about objects that can be used for digging.
 *
 * `deciturns` will be filled in with the avg number of deciturns it will
 * take to dig through each type of diggable terrain, and must be at least 
 * [DIGGING_MAX].
 *
 * Returns false if the object has no effect on digging, or if the specifics
 * are meaningless (i.e. the object is an ego template, not a real item).
 */
static bool obj_known_digging(struct object *obj, int deciturns[])
{
	struct player_state state;
	int i;
	int chances[DIGGING_MAX];
	int slot = wield_slot(obj);
	struct object *current = slot_object(player, slot);

	/* Doesn't remotely resemble a digger */
	if (!tval_is_wearable(obj) ||
		(!tval_is_melee_weapon(obj) && (obj->modifiers[OBJ_MOD_TUNNEL] <= 0)))
		return false;

	/* Player has no digging info */
	if (!tval_is_melee_weapon(obj) && !obj->known->modifiers[OBJ_MOD_TUNNEL])
		return false;

	/* Pretend we're wielding the object */
	player->body.slots[slot].obj = obj;

	/* Calculate the player's hypothetical state */
	memcpy(&state, &player->state, sizeof(state));
	state.stat_ind[STAT_STR] = 0; //Hack - NRM
	state.stat_ind[STAT_DEX] = 0; //Hack - NRM
	calc_bonuses(player, &state, true, false);

	/* Stop pretending */
	player->body.slots[slot].obj = current;

	calc_digging_chances(&state, chances);

	/* Digging chance is out of 1600 */
	for (i = DIGGING_RUBBLE; i < DIGGING_MAX; i++) {
		int chance = MIN(1600, chances[i]);
		deciturns[i] = chance ? (16000 / chance) : 0;
	}

	return true;
}

/**
 * Describe objects that can be used for digging.
 */
static bool describe_digger(textblock *tb, const struct object *obj)
{
	int i;
	int deciturns[DIGGING_MAX];
	struct object *obj1 = (struct object *) obj;
	static const char *names[4] = { "rubble", "magma veins", "quartz veins",
									"granite" };

	/* Get useful info or print nothing */
	if (!obj_known_digging(obj1, deciturns)) return false;

	for (i = DIGGING_RUBBLE; i < DIGGING_DOORS; i++) {
		if (i == 0 && deciturns[0] > 0) {
			if (tval_is_melee_weapon(obj))
				textblock_append(tb, "Clears ");
			else
				textblock_append(tb, "With this item, your current weapon clears ");
		}

		if (i == 3 || (i != 0 && deciturns[i] == 0))
			textblock_append(tb, "and ");

		if (deciturns[i] == 0) {
			textblock_append_c(tb, COLOUR_L_RED, "doesn't affect ");
			textblock_append(tb, "%s.\n", names[i]);
			break;
		}

		textblock_append(tb, "%s in ", names[i]);

		if (deciturns[i] == 10) {
			textblock_append_c(tb, COLOUR_L_GREEN, "1 ");
		} else if (deciturns[i] < 100) {
			textblock_append_c(tb, COLOUR_GREEN, "%d.%d ", deciturns[i]/10,
							   deciturns[i]%10);
		} else {
			textblock_append_c(tb, (deciturns[i] < 1000) ? COLOUR_YELLOW :
							   COLOUR_RED, "%d ", (deciturns[i]+5)/10);
		}

		textblock_append(tb, "turn%s%s", deciturns[i] == 10 ? "" : "s",
				(i == 3) ? ".\n" : ", ");
	}

	return true;
}

/**
 * Gives the known light-sourcey characteristics of the given object.
 *
 * Fills in the intensity of the light in `intensity`, whether it uses fuel and
 * how many turns light it can refuel in similar items.
 *
 * Return false if the object is not known to be a light source (which 
 * includes it not actually being a light source).
 */
static bool obj_known_light(const struct object *obj, oinfo_detail_t mode,
							int *intensity, bool *uses_fuel, int *refuel_turns)
{
	bool no_fuel;
	bool is_light = tval_is_light(obj);

	if (!is_light && (obj->modifiers[OBJ_MOD_LIGHT] <= 0))
		return false;

	/* Work out intensity */
	if (of_has(obj->flags, OF_LIGHT_2))
		*intensity = 2;
	else if (of_has(obj->flags, OF_LIGHT_3))
		*intensity = 3;
	*intensity += obj->known->modifiers[OBJ_MOD_LIGHT];

	/* Prevent unidentified objects (especially artifact lights) from showing
	 * bad intensity and refueling info. */
	if (*intensity == 0)
		return false;

	no_fuel = of_has(obj->known->flags, OF_NO_FUEL) ? true : false;

	if (no_fuel || obj->known->artifact) {
		*uses_fuel = false;
	} else {
		*uses_fuel = true;
	}

	if (is_light && of_has(obj->known->flags, OF_TAKES_FUEL)) {
		*refuel_turns = z_info->fuel_lamp;
	} else {
		*refuel_turns = 0;
	}

	return true;
}

/**
 * Describe things that look like lights.
 */
static bool describe_light(textblock *tb, const struct object *obj,
						   oinfo_detail_t mode)
{
	int intensity = 0;
	bool uses_fuel = false;
	int refuel_turns = 0;
	bool terse = mode & OINFO_TERSE ? true : false;

	if (!obj_known_light(obj, mode, &intensity, &uses_fuel, &refuel_turns))
		return false;

	if (tval_is_light(obj)) {
		textblock_append(tb, "Intensity ");
		textblock_append_c(tb, COLOUR_L_GREEN, format("%d", intensity));
		textblock_append(tb, " light.");

		if (!obj->artifact && !uses_fuel)
			textblock_append(tb, "  No fuel required.");

		if (!terse) {
			if (refuel_turns)
				textblock_append(tb, "  Refills other lanterns up to %d turns of fuel.", refuel_turns);
			else
				textblock_append(tb, "  Cannot be refueled.");
		}
		textblock_append(tb, "\n");
	}

	return true;
}


/**
 * Describe readable books.
 */
static bool describe_book(textblock *tb, const struct object *obj,
						   oinfo_detail_t mode)
{
	if (!obj_can_browse(obj)) return false;

	textblock_append(tb, "\nYou can read this book.\n");

	return true;
}


/**
 * Gives the known effects of using the given item.
 *
 * Fills in:
 *  - the effect
 *  - whether the effect can be aimed
 *  -  the minimum and maximum time in game turns for the item to recharge 
 *     (or zero if it does not recharge)
 *  - the percentage chance of the effect failing when used
 *
 * Return false if the object has no effect.
 */
static bool obj_known_effect(const struct object *obj, struct effect **effect,
								 bool *aimed, int *min_recharge,
								 int *max_recharge, int *failure_chance)
{
	random_value timeout = {0, 0, 0, 0};
	bool store_consumable = object_is_in_store(obj) && tval_is_useable(obj);

	*effect = NULL;
	*min_recharge = 0;
	*max_recharge = 0;
	*failure_chance = 0;
	*aimed = false;

	if (object_effect_is_known(obj) || store_consumable) {
		*effect = object_effect(obj);
		timeout = obj->time;
		if (effect_aim(*effect))
			*aimed = true;;
	} else if (object_effect(obj)) {
		/* Don't know much - be vague */
		*effect = NULL;
		if (tval_is_wand(obj) || tval_is_rod(obj)) {
			*aimed = true;
		}
		return true;
	} else {
		/* No effect - no info */
		return false;
	}

	if (randcalc(timeout, 0, MAXIMISE) > 0)	{
		*min_recharge = randcalc(timeout, 0, MINIMISE);
		*max_recharge = randcalc(timeout, 0, MAXIMISE);
	}

	if (tval_is_edible(obj) || tval_is_potion(obj) || tval_is_scroll(obj)) {
		*failure_chance = 0;
	} else {
		*failure_chance = get_use_device_chance(obj);
	}

	return true;
}

/**
 * Describe an object's effect, if any.
 */
static bool describe_effect(textblock *tb, const struct object *obj,
		bool only_artifacts, bool subjective)
{
	char desc[200];
	struct effect *effect = NULL;
	bool aimed = false;
	int min_time, max_time, failure_chance;

	/* Sometimes we only print artifact activation info */
	if (only_artifacts && !obj->artifact) {
		return false;
	}

	if (obj_known_effect(obj, &effect, &aimed, &min_time, &max_time,
						 &failure_chance) == false) {
		return false;
	}

	/* Effect not known, mouth platitudes */
	if (!effect && object_effect(obj)) {
		if (tval_is_edible(obj)) {
			textblock_append(tb, "It can be eaten.\n");
		} else if (tval_is_potion(obj)) {
			textblock_append(tb, "It can be drunk.\n");
		} else if (tval_is_scroll(obj)) {
			textblock_append(tb, "It can be read.\n");
		} else if (aimed) {
			textblock_append(tb, "It can be aimed.\n");
		} else {
			textblock_append(tb, "It can be activated.\n");
		}

		return true;
	}

	/* Activations get a special message */
	if (obj->activation && obj->activation->desc) {
		textblock_append(tb, "When activated, it ");
		textblock_append(tb, obj->activation->desc);
	} else {
		int random_choices = 0;
		bool random_breath = (effect && (effect->index == EF_RANDOM) &&
							  effect->next &&
							  (effect->next->index == EF_BREATH));
		char breaths[120];

		my_strcpy(breaths, "", sizeof(breaths));

		/* Get descriptions for all the effects */
		effect = object_effect(obj);
		if (!effect_desc(effect)) return false;

		if (aimed)
			textblock_append(tb, "When aimed, it ");
		else if (tval_is_edible(obj))
			textblock_append(tb, "When eaten, it ");
		else if (tval_is_potion(obj))
			textblock_append(tb, "When quaffed, it ");
		else if (tval_is_scroll(obj))
			textblock_append(tb, "When read, it ");
		else
			textblock_append(tb, "When activated, it ");

		/* Print a colourised description */
		while (effect) {
			char *next_char = desc;
			int roll = 0;
			random_value value = { 0, 0, 0, 0 };
			char dice_string[20];

			int level = obj->artifact ? obj->artifact->level : obj->kind->level;
			int boost = MAX(player->state.skills[SKILL_DEVICE] - level, 0);

			if (effect->dice != NULL)
				roll = dice_roll(effect->dice, &value);

			/* Deal with special random effect */
			if (effect->index == EF_RANDOM)
				random_choices = roll + 1;

			/* Get the possible dice strings */
			if (value.dice && value.base)
				strnfmt(dice_string, sizeof(dice_string), "%d+%dd%d",
						value.base, value.dice, value.sides);
			else if (value.dice)
				strnfmt(dice_string, sizeof(dice_string), "%dd%d",
						value.dice, value.sides);
			else
				strnfmt(dice_string, sizeof(dice_string), "%d", value.base);

			/* Check all the possible types of description format */
			switch (base_descs[effect->index].efinfo_flag) {
				/* Healing sometimes has a minimum percentage */
			case EFINFO_HEAL: {
				char min_string[50];
				if (value.m_bonus)
					strnfmt(min_string, sizeof(min_string),
							" (or %d%%, whichever is greater)", value.m_bonus);
				else
					strnfmt(min_string, sizeof(min_string), "");
				strnfmt(desc, sizeof(desc), effect_desc(effect), dice_string,
						min_string);
				break;
			}

			case EFINFO_CONST: {
				strnfmt(desc, sizeof(desc), effect_desc(effect), value.base/2);
				break;
			}
			case EFINFO_FOOD: {
				char *fed = effect->subtype ? "leaves you nourished" :
					"feeds you";
				strnfmt(desc, sizeof(desc), effect_desc(effect), fed,
						value.base * z_info->food_value, value.base);
				break;
			}
			case EFINFO_CURE: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						timed_effects[effect->subtype].desc);
				break;
			}
			case EFINFO_TIMED: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						timed_effects[effect->subtype].desc, dice_string);
				break;
			}
			case EFINFO_STAT: {
				int stat = effect->subtype;
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						lookup_obj_property(OBJ_PROPERTY_STAT, stat)->name);
				break;
			}
			case EFINFO_SEEN: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						projections[effect->subtype].desc);
				break;
			}
			case EFINFO_SUMM: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						summon_desc(effect->subtype));
				break;
			}

			/* Only currently used for the player, but can handle monsters */
			case EFINFO_TELE: {
				char *dist = value.m_bonus ?
					" a level dependent distance" :
					format(" %d grids", value.base);

				if (effect->subtype) {
					strnfmt(desc, sizeof(desc), effect_desc(effect),
							"a monster", dist);
				} else {
					strnfmt(desc, sizeof(desc), effect_desc(effect), "you",
							dist);
				}
				break;
			}
			case EFINFO_QUAKE: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						effect->radius);
				break;
			}

			/* Object generated balls are elemental */
			case EFINFO_BALL: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						projections[effect->subtype].player_desc,
						effect->radius, dice_string);
				if (boost)
					my_strcat(desc, format(", which your device skill increases by %d per cent", boost),
							  sizeof(desc));
				break;
			}

			/* Object generated breaths are elemental */
			case EFINFO_BREATH: {
				/* Special treatment for several random breaths */
				if (random_breath) {
					my_strcat(breaths,
							  projections[effect->subtype].player_desc,
							  sizeof(breaths));
					if (random_choices > 3) {
						my_strcat(breaths, ", ", sizeof(breaths));
					} else if (random_choices == 3) {
						my_strcat(breaths, " or ", sizeof(breaths));
					}
					random_choices--;

					if ((!effect->next) || (effect->next->index != EF_BREATH)) {
						random_breath = false;
					}
					strnfmt(desc, sizeof(desc), effect_desc(effect), breaths,
							effect->other, dice_string);
				} else {
					strnfmt(desc, sizeof(desc), effect_desc(effect),
							projections[effect->subtype].player_desc,
							effect->other, dice_string);
				}
				break;
			}

			case EFINFO_SHORT: {
				strnfmt(desc, sizeof(desc), effect_desc(effect), 
						projections[effect->subtype].player_desc,
						effect->radius +
						effect->other ? effect->other / player->lev : 0,
						dice_string);
				break;
			}

			/* Currently no object generated lashes */
			case EFINFO_LASH: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						projections[effect->subtype].lash_desc,
						effect->subtype);
				break;
			}

			/* Bolts that inflict status */
			case EFINFO_BOLT: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						projections[effect->subtype].desc);
				break;
			}
			/* Bolts and beams that damage */
			case EFINFO_BOLTD: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						projections[effect->subtype].desc, dice_string);
				if (boost)
					my_strcat(desc, format(", which your device skill increases by %d per cent", boost),
							  sizeof(desc));
				break;
			}
			case EFINFO_TOUCH: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						projections[effect->subtype].desc);
				break;
			}
			case EFINFO_TAP: {
				strnfmt(desc, sizeof(desc), effect_desc(effect),
						dice_string);
				break;
			}
			case EFINFO_NONE: {
				strnfmt(desc, sizeof(desc), effect_desc(effect));
				break;
			}
			default: {
				msg("Bad effect description passed to describe_effect(). Please report this bug.");
				return false;
			}
			}

			do {
				if (random_breath && effect->index != EF_RANDOM) break;
				if (isdigit((unsigned char) *next_char))
					textblock_append_c(tb, COLOUR_L_GREEN, "%c", *next_char);
				else
					textblock_append(tb, "%c", *next_char);
			} while (*next_char++);

			/* Random choices need special treatment - note that this code
			 * assumes that RANDOM and the random choices will be the last
			 * effect in the object/activation description */
			if (random_breath) {
				/* Handled in effect description */
				;
			} else if (random_choices >= 1) {
				if (effect->index == EF_RANDOM)
					;
				else if (random_choices > 2)
					textblock_append(tb, ", ");
				else if (random_choices == 2)
					textblock_append(tb, " or ");
				random_choices--;
			} else if (effect->next) {
				if (effect->next->next && (effect->next->index != EF_RANDOM))
					textblock_append(tb, ", ");
				else
					textblock_append(tb, " and ");
			}
			effect = effect->next;
		}
	}

	textblock_append(tb, ".\n");

	if (min_time || max_time) {
		/* Sometimes adjust for player speed */
		int multiplier = turn_energy(player->state.speed);
		if (!subjective) multiplier = 10;

		textblock_append(tb, "Takes ");

		/* Correct for player speed */
		min_time = (min_time * multiplier) / 10;
		max_time = (max_time * multiplier) / 10;

		textblock_append_c(tb, COLOUR_L_GREEN, "%d", min_time);

		if (min_time != max_time) {
			textblock_append(tb, " to ");
			textblock_append_c(tb, COLOUR_L_GREEN, "%d", max_time);
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

	return true;
}

/**
 * Describe an item's origin
 */
static bool describe_origin(textblock *tb, const struct object *obj, bool terse)
{
	char loot_spot[80];
	char name[80];
	int origin;
	const char *dropper = NULL;
	const char *article;
	bool unique = false;

	/* Only give this info in chardumps if wieldable */
	if (terse && !obj_can_wear(obj))
		return false;

	/* Set the origin - care needed for mimics */
	if ((obj->origin == ORIGIN_DROP_MIMIC) && (obj->mimicking_m_idx != 0))
		origin = ORIGIN_FLOOR;
	else
		origin = obj->origin;

	/* Name the place of origin */
	if (obj->origin_depth)
		strnfmt(loot_spot, sizeof(loot_spot), "at %d feet (level %d)",
		        obj->origin_depth * 50, obj->origin_depth);
	else
		my_strcpy(loot_spot, "in town", sizeof(loot_spot));

	/* Name the monster of origin */
	if (obj->origin_race) {
		dropper = obj->origin_race->name;
		if (rf_has(obj->origin_race->flags, RF_UNIQUE)) {
			unique = true;
		}
	} else {
		dropper = "monster lost to history";
	}
	article = is_a_vowel(dropper[0]) ? "an " : "a ";
	if (unique)
		my_strcpy(name, dropper, sizeof(name));
	else {
		my_strcpy(name, article, sizeof(name));
		my_strcat(name, dropper, sizeof(name));
	}

	/* Print an appropriate description */
	switch (origins[origin].args)
	{
		case -1: return false;
		case 0: textblock_append(tb, origins[origin].desc); break;
		case 1: textblock_append(tb, origins[origin].desc, loot_spot);
				break;
		case 2:
			textblock_append(tb, origins[origin].desc, name, loot_spot);
			break;
	}

	textblock_append(tb, "\n\n");

	return true;
}

/**
 * Print an item's flavour text.
 *
 * \param tb is the textblock to which we are adding.
 * \param obj is the object we are describing.
 * \param ego is whether we're describing an ego template (as opposed to a
 * real object)
 */
static void describe_flavor_text(textblock *tb, const struct object *obj,
								 bool ego)
{
	/* Display the known artifact or object description */
	if (!OPT(player, birth_randarts) && obj->artifact &&
		obj->known->artifact && obj->artifact->text) {
		textblock_append(tb, "%s\n\n", obj->artifact->text);

	} else if (object_flavor_is_aware(obj) || ego) {
		bool did_desc = false;

		if (!ego && obj->kind->text) {
			textblock_append(tb, "%s", obj->kind->text);
			did_desc = true;
		}

		/* Display an additional ego-item description */
		if ((ego || (obj->known->ego != NULL)) && obj->ego->text) {
			if (did_desc) textblock_append(tb, "  ");
			textblock_append(tb, "%s\n\n", obj->ego->text);
		} else if (did_desc) {
			textblock_append(tb, "\n\n");
		}
	}
}

/**
 * Describe random properties that an ego item may have
 */
static bool describe_ego(textblock *tb, const struct ego_item *ego)
{
	if (kf_has(ego->kind_flags, KF_RAND_HI_RES))
		textblock_append(tb, "It provides one random higher resistance.  ");
	else if (kf_has(ego->kind_flags, KF_RAND_SUSTAIN))
		textblock_append(tb, "It provides one random sustain.  ");
	else if (kf_has(ego->kind_flags, KF_RAND_POWER))
		textblock_append(tb, "It provides one random ability.  ");
	else if (kf_has(ego->kind_flags, KF_RAND_RES_POWER))
		textblock_append(tb, "It provides one random ability or base resistance.  ");
	else
		return false;

	return true;
}


/**
 * ------------------------------------------------------------------------
 * Output code
 * ------------------------------------------------------------------------ */
/**
 * Output object information
 */
static textblock *object_info_out(const struct object *obj, int mode)
{
	bitflag flags[OF_SIZE];
	struct element_info el_info[ELEM_MAX];
	bool something = false;

	bool terse = mode & OINFO_TERSE ? true : false;
	bool subjective = mode & OINFO_SUBJ ? true : false;
	bool ego = mode & OINFO_EGO ? true : false;
	textblock *tb = textblock_new();

	assert(obj->known);

	/* Unaware objects get simple descriptions */
	if (obj->kind != obj->known->kind) {
		textblock_append(tb, "\n\nYou do not know what this is.\n");
		return tb;
	}

	/* Grab the object flags */
	get_known_flags(obj, mode, flags);

	/* Grab the element info */
	get_known_elements(obj, mode, el_info);

	if (subjective) describe_origin(tb, obj, terse);
	if (!terse) describe_flavor_text(tb, obj, ego);

	if (!object_fully_known(obj) &&	(obj->known->notice & OBJ_NOTICE_ASSESSED) && !tval_is_useable(obj)) {
		textblock_append(tb, "You do not know the full extent of this item's powers.\n");
		something = true;
	}

	if (describe_curses(tb, obj, flags)) something = true;
	if (describe_stats(tb, obj, mode)) something = true;
	if (describe_slays(tb, obj)) something = true;
	if (describe_brands(tb, obj)) something = true;
	if (describe_elements(tb, el_info)) something = true;
	if (describe_protects(tb, flags)) something = true;
	if (describe_ignores(tb, el_info)) something = true;
	if (describe_hates(tb, el_info)) something = true;
	if (describe_sustains(tb, flags)) something = true;
	if (describe_misc_magic(tb, flags)) something = true;
	if (describe_light(tb, obj, mode)) something = true;
	if (describe_book(tb, obj, mode)) something = true;
	if (ego && describe_ego(tb, obj->ego)) something = true;
	if (something) textblock_append(tb, "\n");

	/* Skip all the very specific information where we are giving general
	   ego knowledge rather than for a single item - abilities can vary */
	if (!ego) {
		if (describe_effect(tb, obj, terse, subjective)) {
			something = true;
			textblock_append(tb, "\n");
		}

		if (subjective && describe_combat(tb, obj)) {
			something = true;
			textblock_append(tb, "\n");
		}

		if (!terse && subjective && describe_digger(tb, obj)) something = true;
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
 * returns true if anything is printed.
 */
textblock *object_info(const struct object *obj, oinfo_detail_t mode)
{
	mode |= OINFO_SUBJ;
	return object_info_out(obj, mode);
}

/**
 * Provide information on an ego-item type
 */
textblock *object_info_ego(struct ego_item *ego)
{
	struct object_kind *kind = NULL;
	struct object obj = OBJECT_NULL, known_obj = OBJECT_NULL;
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

	object_copy(&known_obj, &obj);
	obj.known = &known_obj;

	return object_info_out(&obj, OINFO_NONE | OINFO_EGO);
}



/**
 * Provide information on an item suitable for writing to the character dump
 * - keep it brief.
 */
void object_info_chardump(ang_file *f, const struct object *obj, int indent,
						  int wrap)
{
	textblock *tb = object_info_out(obj, OINFO_TERSE | OINFO_SUBJ);
	textblock_to_file(tb, f, indent, wrap);
	textblock_free(tb);
}


/**
 * Provide spoiler information on an item.
 *
 * Practically, this means that we should not print anything which relies upon
 * the player's current state, since that is not suitable for spoiler material.
 */
void object_info_spoil(ang_file *f, const struct object *obj, int wrap)
{
	textblock *tb = object_info_out(obj, OINFO_SPOIL);
	textblock_to_file(tb, f, 0, wrap);
	textblock_free(tb);
}
