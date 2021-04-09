/**
 * \file effects-info.c
 * \brief Implement interfaces for displaying information about effects
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

#include "effects-info.h"
#include "effects.h"
#include "init.h"
#include "message.h"
#include "mon-summon.h"
#include "obj-info.h"
#include "player-timed.h"
#include "project.h"
#include "z-color.h"
#include "z-form.h"
#include "z-util.h"


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
 * Get the possible dice strings.
 */
static void format_dice_string(const random_value *v, int multiplier,
	size_t len, char* dice_string)
{
	if (v->dice && v->base) {
		if (multiplier == 1) {
			strnfmt(dice_string, len, "%d+%dd%d", v->base, v->dice,
				v->sides);
		} else {
			strnfmt(dice_string, len, "%d+%d*(%dd%d)",
				multiplier * v->base, multiplier, v->dice,
				v->sides);
		}
	} else if (v->dice) {
		if (multiplier == 1) {
			strnfmt(dice_string, len, "%dd%d", v->dice, v->sides);
		} else {
			strnfmt(dice_string, len, "%d*(%dd%d)", multiplier,
				v->dice, v->sides);
		}
	} else {
		strnfmt(dice_string, len, "%d", multiplier * v->base);
	}
}


/**
 * Appends a message describing the magical device skill bonus and the average
 * damage. Average damage is only displayed if there is variance or a magical
 * device bonus.
 */
static void append_damage(char *buffer, size_t buffer_size, random_value value,
	int dev_skill_boost)
{
	if (dev_skill_boost != 0) {
		my_strcat(buffer, format(", which your device skill increases by %d%%",
			dev_skill_boost), buffer_size);
	}

	if (randcalc_varies(value) || dev_skill_boost > 0) {
		// Ten times the average damage, for 1 digit of precision
		int dam = (100 + dev_skill_boost) * randcalc(value, 0, AVERAGE) / 10;
		my_strcat(buffer, format(" for an average of %d.%d damage", dam / 10,
			dam % 10), buffer_size);
	}
}


static void copy_to_textblock_with_coloring(textblock *tb, const char *s)
{
	while (*s) {
		if (isdigit((unsigned char) *s)) {
			textblock_append_c(tb, COLOUR_L_GREEN, "%c", *s);
		} else {
			textblock_append(tb, "%c", *s);
		}
		++s;
	}
}


/**
 * Creates a description of the random effect which chooses from the next count
 * effects in the linked list starting with e.  The description is prefaced
 * with the contents of *prefix if prefix is not NULL.  dev_skill_boost is the
 * percent increase in damage to report for the device skill.  Sets *nexte to
 * point to the element in the linked list or NULL that is immediately after
 * the count effects.  Returns a non-NULL value if there was at least one
 * effect that could be described.  Otherwise, returns NULL.
 */
static textblock *create_random_effect_description(const struct effect *e,
	int count, const char *prefix, int dev_skill_boost,
	const struct effect **nexte)
{
	/*
	 * Do one pass through the effects to determine if they are of all the
	 * the same basic type.  That is used to condense the description in
	 * the case where all are breaths.  Ignore random nested effects since
	 * they will do nothing when the outer random effect is processed with
	 * effect_do().
	 */
	textblock *res = NULL;
	const struct effect *efirst;
	const dice_t *first_dice;
	int first_ind, first_other;
	random_value first_rv = { 0, 0, 0, 0 };
	bool same_ind, same_other, same_dice;
	int irand, jrand;
	int nvalid;

	/* Find the first effect that's valid and not random. */
	irand = 0;
	while (1) {
		if (!e || irand >= count) {
			/*
			 * There's no valid or non-random effects; do nothing.
			 */
			*nexte = e;
			return false;
		}
		if (effect_desc(e) && e->index != EF_RANDOM) {
			break;
		}
		e = e->next;
		++irand;
	}

	efirst = e;
	first_ind = e->index;
	first_other = e->other;
	first_dice = e->dice;
	if (e->dice) {
		dice_random_value(e->dice, &first_rv);
	}

	nvalid = 1;
	same_ind = true;
	same_other = true;
	same_dice = true;
	for (e = efirst->next, jrand = irand + 1;
		e && jrand < count;
		e = e->next, ++jrand) {
		if (!effect_desc(e) || e->index == EF_RANDOM) {
			continue;
		}
		++nvalid;
		if (e->index != first_ind) {
			same_ind = false;
		}
		if (e->other != first_other) {
			same_other = false;
		}
		if (e->dice) {
			if (first_dice) {
				random_value this_rv;

				dice_random_value(e->dice, &this_rv);
				if (this_rv.base != first_rv.base ||
					this_rv.dice != first_rv.dice ||
					this_rv.sides != first_rv.sides ||
					this_rv.m_bonus != first_rv.m_bonus) {
					same_dice = false;
				}
			} else {
				same_dice = false;
			}
		} else if (first_dice) {
			same_dice = false;
		}
	}
	*nexte = e;

	if (same_ind && base_descs[first_ind].efinfo_flag == EFINFO_BREATH &&
		same_dice && same_other) {
		/* Concatenate the list of possible elements. */
		char breaths[120], dice_string[20], desc[200];
		int ivalid;

		strnfmt(breaths, sizeof(breaths), "%s",
			projections[efirst->subtype].player_desc);
		ivalid = 1;
		for (e = efirst->next, jrand = irand + 1;
			e && jrand < count;
			e = e->next, ++jrand) {
			if (!effect_desc(e) || e->index == EF_RANDOM) {
				continue;
			}
			if (ivalid == nvalid - 1) {
				my_strcat(breaths,
					(nvalid > 2) ? ", or " : " or ",
					sizeof(breaths));
			} else {
				my_strcat(breaths, ", ", sizeof(breaths));
			}
			my_strcat(breaths, projections[e->subtype].player_desc,
				sizeof(breaths));
			++ivalid;
		}

		/* Then use that in the effect description. */
		format_dice_string(&first_rv, 1, sizeof(dice_string),
			dice_string);
		strnfmt(desc, sizeof(desc), effect_desc(efirst), breaths,
			efirst->other, dice_string);
		append_damage(desc, sizeof(desc), first_rv,
			efirst->index == EF_BREATH ? 0 : dev_skill_boost);

		res = textblock_new();
		if (prefix) {
			textblock_append(res, "%s", prefix);
		}
		textblock_append(res, "randomly ");
		copy_to_textblock_with_coloring(res, desc);
	} else {
		/* Concatenate the effect descriptions. */
		textblock *tb;
		int ivalid;
		
		tb = effect_describe(efirst, "randomly ", dev_skill_boost,
			true);
		if (tb) {
			ivalid = 1;
			if (prefix) {
				res = textblock_new();
				textblock_append(res, "%s", prefix);
				textblock_append_textblock(res, tb);
				textblock_free(tb);
			} else {
				res = tb;
			}
		} else {
			ivalid = 0;
			--nvalid;
		}
		for (e = efirst->next, jrand = irand + 1;
			e && jrand < count;
			e = e->next, ++jrand) {
			if (!effect_desc(e) || e->index == EF_RANDOM) {
				continue;
			}
			tb = effect_describe(e,
				(ivalid == 0) ? "randomly " : NULL,
				dev_skill_boost, true);
			if (!tb) {
				--nvalid;
				continue;
			}
			if (prefix && ! res) {
				assert(ivalid == 0);
				res = textblock_new();
				textblock_append(res, "%s", prefix);
			}
			if (res) {
				if (ivalid > 0) {
					textblock_append(res,
						(ivalid == nvalid - 1) ?
						" or " : ", ");
				}
				textblock_append_textblock(res, tb);
				textblock_free(tb);
			} else {
				res = tb;
			}
			++ivalid;
		}
	}

	return res;
}


/**
 * Creates a new textblock which has a description of the effect in *e (and
 * any linked to it because e->index == EF_RANDOM) if only_first is true or
 * has a description of *e and all the subsequent effects if only_first is
 * false.  If none of the effects has a description, will return NULL.  If
 * there is at least one effect with a description and prefix is not NULL,
 * the string pointed to by prefix will be added to the textblock before
 * the descriptions.  dev_skill_boost is the percent increase from the device
 * skill to show in the descriptions.
 */
textblock *effect_describe(const struct effect *e, const char *prefix,
	int dev_skill_boost, bool only_first)
{
	textblock *tb = NULL;
	int nadded = 0;
	char desc[250];

	while (e) {
		const char* edesc = effect_desc(e);
		int roll = 0;
		random_value value = { 0, 0, 0, 0 };
		char dice_string[20];

		if (e->dice != NULL) {
			roll = dice_roll(e->dice, &value);
		}

		/* Deal with special random effect. */
		if (e->index == EF_RANDOM) {
			const struct effect *nexte;
			textblock *tbe = create_random_effect_description(
				e->next, roll, (nadded == 0) ? prefix : NULL,
				dev_skill_boost, &nexte);

			e = (only_first) ? NULL : nexte;
			if (tbe) {
				if (tb) {
					textblock_append(tb,
						e ? ", " : " and ");
					textblock_append_textblock(tb, tbe);
					textblock_free(tbe);
				} else {
					tb = tbe;
				}
				++nadded;
			}
			continue;
		}

		if (!edesc) {
			e = (only_first) ? NULL : e->next;
			continue;
		}

		format_dice_string(&value, 1, sizeof(dice_string), dice_string);

		/* Check all the possible types of description format. */
		switch (base_descs[e->index].efinfo_flag) {
		case EFINFO_DICE:
			strnfmt(desc, sizeof(desc), edesc, dice_string);
			break;

		case EFINFO_HEAL:
			/* Healing sometimes has a minimum percentage. */
			{
				char min_string[50];

				if (value.m_bonus) {
					strnfmt(min_string, sizeof(min_string),
						" (or %d%%, whichever is greater)",
						value.m_bonus);
				} else {
					strnfmt(min_string, sizeof(min_string),
						"");
				}
				strnfmt(desc, sizeof(desc), edesc, dice_string,
					min_string);
			}
			break;

		case EFINFO_CONST:
			strnfmt(desc, sizeof(desc), edesc, value.base / 2);
			break;

		case EFINFO_FOOD:
			{
				const char *fed = e->subtype ?
					(e->subtype == 1 ? "uses enough food value" : 
					 "leaves you nourished") : "feeds you";
				char turn_dice_string[20];

				format_dice_string(&value, z_info->food_value,
					sizeof(turn_dice_string),
					turn_dice_string);

				strnfmt(desc, sizeof(desc), edesc, fed,
					turn_dice_string, dice_string);
			}
			break;

		case EFINFO_CURE:
			strnfmt(desc, sizeof(desc), edesc,
				timed_effects[e->subtype].desc);
			break;

		case EFINFO_TIMED:
			strnfmt(desc, sizeof(desc), edesc,
				timed_effects[e->subtype].desc,
				dice_string);
			break;

		case EFINFO_STAT:
			{
				int stat = e->subtype;

				strnfmt(desc, sizeof(desc), edesc,
					lookup_obj_property(OBJ_PROPERTY_STAT, stat)->name);
			}
			break;

		case EFINFO_SEEN:
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].desc);
			break;

		case EFINFO_SUMM:
			strnfmt(desc, sizeof(desc), edesc,
				summon_desc(e->subtype));
			break;

		case EFINFO_TELE:
			/*
			 * Only currently used for the player, but can handle
			 * monsters.
			 */
			{
				char dist[32];

				if (value.m_bonus) {
					strnfmt(dist, sizeof(dist),
						"a level-dependent distance");
				} else {
					strnfmt(dist, sizeof(dist),
						"%d grids", value.base);
				}
				strnfmt(desc, sizeof(desc), edesc,
					(e->subtype) ? "a monster" : "you",
					dist);
			}
			break;

		case EFINFO_QUAKE:
			strnfmt(desc, sizeof(desc), edesc, e->radius);
			break;

		case EFINFO_BALL:
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].player_desc,
				e->radius, dice_string);
			append_damage(desc, sizeof(desc), value, dev_skill_boost);
			break;

		case EFINFO_SPOT:
			{
				int i_radius = e->other ? e->other : e->radius;

				strnfmt(desc, sizeof(desc), edesc,
					projections[e->subtype].player_desc,
					e->radius, i_radius, dice_string);
				append_damage(desc, sizeof(desc), value, dev_skill_boost);
			}
			break;

		case EFINFO_BREATH:
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].player_desc, e->other,
				dice_string);
			append_damage(desc, sizeof(desc), value,
				e->index == EF_BREATH ? 0 : dev_skill_boost);
			break;

		case EFINFO_SHORT:
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].player_desc,
				e->radius +
					(e->other ? e->other / player->lev : 0),
				dice_string);
			break;

		case EFINFO_LASH:
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].lash_desc, e->subtype);
			break;

		case EFINFO_BOLT:
			/* Bolt that inflict status */
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].desc);
			break;

		case EFINFO_BOLTD:
			/* Bolts and beams that damage */
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].desc, dice_string);
			append_damage(desc, sizeof(desc), value, dev_skill_boost);
			break;

		case EFINFO_TOUCH:
			strnfmt(desc, sizeof(desc), edesc,
				projections[e->subtype].desc);
			break;

		case EFINFO_NONE:
			strnfmt(desc, sizeof(desc), edesc);
			break;

		default:
			strnfmt(desc, sizeof(desc), "");
			msg("Bad effect description passed to effect_info().  Please report this bug.");
			break;
		}

		e = (only_first) ? NULL : e->next;

		if (desc[0] != '\0') {
			if (tb) {
				if (e) {
					textblock_append(tb, ", ");
				} else {
					textblock_append(tb, " and ");
				}
			} else {
				tb = textblock_new();
				if (prefix) {
					textblock_append(tb, "%s", prefix);
				}
			}
			copy_to_textblock_with_coloring(tb, desc);

			++nadded;
		}
	}

	return tb;
}

/**
 * Returns a pointer to the next effect in the effect stack, skipping over
 * all the sub-effects from random effects
 */
struct effect *effect_next(struct effect *effect)
{
	if (effect->index == EF_RANDOM) {
		struct effect *e = effect;
		int num_subeffects = dice_evaluate(effect->dice, 0, AVERAGE, NULL);
		// Skip all the sub-effects, plus one to advance beyond current
		for (int i = 0; e != NULL && i < num_subeffects + 1; i++) {
			e = e->next;
		}
		return e;
	} else {
		return effect->next;
	}
}

/**
 * Checks if the effect deals damage, by checking the effect's info string.
 * Random effects are considered to deal damage if any sub-effect deals
 * damage.
 */
bool effect_damages(const struct effect *effect)
{
	if (effect->index == EF_RANDOM) {
		// Random effect
		struct effect *e = effect->next;
		int num_subeffects = dice_evaluate(effect->dice, 0, AVERAGE, NULL);

		// Check if any of the subeffects do damage
		for (int i = 0; e != NULL && i < num_subeffects; i++) {
			if (effect_damages(e)) {
				return true;
			}
			e = e->next;
		}
		return false;
	} else {
		// Non-random effect, check the info string for damage
		return effect_info(effect) != NULL &&
			streq(effect_info(effect), "dam");
	}
}

/**
 * Calculates the average damage of the effect. Random effects return an
 * average of all sub-effect averages.
 */
int effect_avg_damage(const struct effect *effect)
{
	if (effect->index == EF_RANDOM) {
		// Random effect, check the sub-effects to accumulate damage
		int total = 0;
		struct effect *e = effect->next;
		int num_subeffects = dice_evaluate(effect->dice, 0, AVERAGE, NULL);
		for (int i = 0; e != NULL && i < num_subeffects; i++) {
			total += effect_avg_damage(e);
			e = e->next;
		}
		// Return an average of the sub-effects' average damages
		return total / num_subeffects;
	} else {
		// Non-random effect, calculate the average damage
		return effect_damages(effect) ?
			dice_evaluate(effect->dice, 0, AVERAGE, NULL)
			: 0;
	}
}

/**
 * Returns the projection of the effect, or an empty string if it has none.
 * Random effects only return a projection if all sub-effects have the same
 * projection.
 */
const char *effect_projection(const struct effect *effect)
{
	if (effect->index == EF_RANDOM) {
		// Random effect
		int num_subeffects = dice_evaluate(effect->dice, 0, AVERAGE, NULL);
		struct effect *e = effect->next;
		const char *subeffect_proj = effect_projection(e);

		// Check if all subeffects have the same projection, and if not just
		// give up on it
		for (int i = 0; e != NULL && i < num_subeffects; i++) {
			if (!streq(subeffect_proj, effect_projection(e))) {
				return "";
			}
			e = e->next;
		}

		return subeffect_proj;
	} else if (projections[effect->subtype].player_desc != NULL) {
		// Non-random effect, extract the projection if there is one
		switch (base_descs[effect->index].efinfo_flag) {
			case EFINFO_BALL:
			case EFINFO_BOLTD:
			case EFINFO_BREATH:
			case EFINFO_SHORT:
			case EFINFO_SPOT:
				return projections[effect->subtype].player_desc;
		}
	}

	return "";
}
