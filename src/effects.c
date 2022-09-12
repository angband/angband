/**
 * \file effects.c
 * \brief Public effect and auxiliary functions for every effect in the game
 *
 * Copyright (c) 2007 Andi Sidwell
 * Copyright (c) 2016 Ben Semmler, Nick McConnell
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

#include "effects.h"
#include "effect-handler.h"
#include "game-input.h"
#include "init.h"
#include "mon-summon.h"
#include "obj-gear.h"
#include "player-history.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "trap.h"


/**
 * ------------------------------------------------------------------------
 * Properties of effects
 * ------------------------------------------------------------------------ */
/**
 * Useful things about effects.
 */
static const struct effect_kind effects[] =
{
	{ EF_NONE, false, NULL, NULL, NULL, NULL },
	#define F(x) effect_handler_##x
	#define EFFECT(x, a, b, c, d, e, f)	{ EF_##x, a, b, F(x), e, f },
	#include "list-effects.h"
	#undef EFFECT
	#undef F
	{ EF_MAX, false, NULL, NULL, NULL, NULL }
};


static const char *effect_names[] = {
	NULL,
	#define EFFECT(x, a, b, c, d, e, f)	#x,
	#include "list-effects.h"
	#undef EFFECT
};

/*
 * Utility functions
 */

/**
 * Free all the effects in a structure
 *
 * \param source the effects being freed
 */
void free_effect(struct effect *source)
{
	struct effect *e = source, *e_next;
	while (e) {
		e_next = e->next;
		dice_free(e->dice);
		if (e->msg) {
			string_free(e->msg);
		}
		mem_free(e);
		e = e_next;
	}
}

bool effect_valid(const struct effect *effect)
{
	if (!effect) return false;
	return effect->index > EF_NONE && effect->index < EF_MAX;
}

bool effect_aim(const struct effect *effect)
{
	const struct effect *e = effect;

	if (!effect_valid(effect))
		return false;

	while (e) {
		if (effects[e->index].aim) return true;
		e = e->next;
	}

	return false;
}

const char *effect_info(const struct effect *effect)
{
	if (!effect_valid(effect))
		return NULL;

	return effects[effect->index].info;
}

const char *effect_desc(const struct effect *effect)
{
	if (!effect_valid(effect))
		return NULL;

	return effects[effect->index].desc;
}

effect_index effect_lookup(const char *name)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(effect_names); i++) {
		const char *effect_name = effect_names[i];

		/* Test for equality */
		if (effect_name != NULL && streq(name, effect_name))
			return i;
	}

	return EF_MAX;
}

/**
 * Translate a string to an effect parameter subtype index
 */
int effect_subtype(int index, const char *type)
{
	int val = -1;

	/* If not a numerical value, assign according to effect index */
	if (sscanf(type, "%d", &val) != 1) {
		switch (index) {
				/* Projection name */
			case EF_PROJECT_LOS:
			case EF_PROJECT_LOS_AWARE:
			case EF_DESTRUCTION:
			case EF_SPOT:
			case EF_SPHERE:
			case EF_BALL:
			case EF_BREATH:
			case EF_ARC:
			case EF_SHORT_BEAM:
			case EF_LASH:
			case EF_SWARM:
			case EF_STRIKE:
			case EF_STAR:
			case EF_STAR_BALL:
			case EF_BOLT:
			case EF_BEAM:
			case EF_BOLT_OR_BEAM:
			case EF_LINE:
			case EF_ALTER:
			case EF_BOLT_STATUS:
			case EF_BOLT_STATUS_DAM:
			case EF_BOLT_AWARE:
			case EF_MELEE_BLOWS:
			case EF_TOUCH:
			case EF_TOUCH_AWARE: {
				val = proj_name_to_idx(type);
				break;
			}

				/* Timed effect name */
			case EF_CURE:
			case EF_TIMED_SET:
			case EF_TIMED_INC:
			case EF_TIMED_INC_NO_RES:
			case EF_TIMED_DEC: {
				val = timed_name_to_idx(type);
				break;
			}

				/* Nourishment types */
			case EF_NOURISH: {
				if (streq(type, "INC_BY"))
					val = 0;
				else if (streq(type, "DEC_BY"))
					val = 1;
				else if (streq(type, "SET_TO"))
					val = 2;
				else if (streq(type, "INC_TO"))
					val = 3;
				break;
			}

				/* Monster timed effect name */
			case EF_MON_TIMED_INC: {
				val = mon_timed_name_to_idx(type);
				break;
			}

				/* Summon name */
			case EF_SUMMON: {
				val = summon_name_to_idx(type);
				break;
			}

				/* Stat name */
			case EF_RESTORE_STAT:
			case EF_DRAIN_STAT:
			case EF_LOSE_RANDOM_STAT:
			case EF_GAIN_STAT: {
				val = stat_name_to_idx(type);
				break;
			}

				/* Enchant type name - not worth a separate function */
			case EF_ENCHANT: {
				if (streq(type, "TOBOTH"))
					val = ENCH_TOBOTH;
				else if (streq(type, "TOHIT"))
					val = ENCH_TOHIT;
				else if (streq(type, "TODAM"))
					val = ENCH_TODAM;
				else if (streq(type, "TOAC"))
					val = ENCH_TOAC;
				break;
			}

				/* Player shape name */
			case EF_SHAPECHANGE: {
				val = shape_name_to_idx(type);
				break;
			}

				/* Targeted earthquake */
			case EF_EARTHQUAKE: {
				if (streq(type, "TARGETED"))
					val = 1;
				else if (streq(type, "NONE"))
					val = 0;
				break;
			}

				/* Inscribe a glyph */
			case EF_GLYPH: {
				if (streq(type, "WARDING"))
					val = GLYPH_WARDING;
				else if (streq(type, "DECOY"))
					val = GLYPH_DECOY;
				break;
			}

				/* Allow teleport away */
			case EF_TELEPORT: {
				if (streq(type, "AWAY"))
					val = 1;
				break;
			}

				/* Allow monster teleport toward */
			case EF_TELEPORT_TO: {
				if (streq(type, "SELF"))
					val = 1;
				break;
			}

				/* Some effects only want a radius, so this is a dummy */
			default: {
				if (streq(type, "NONE"))
					val = 0;
			}
		}
	}

	return val;
}

static int32_t effect_value_base_spell_power(void)
{
	int power = 0;

	/* Check the reference race first */
	if (ref_race)
	   power = ref_race->spell_power;
	/* Otherwise the current monster if there is one */
	else if (cave->mon_current > 0)
		power = cave_monster(cave, cave->mon_current)->race->spell_power;

	return power;
}

static int32_t effect_value_base_player_level(void)
{
	return player->lev;
}

static int32_t effect_value_base_dungeon_level(void)
{
	return cave->depth;
}

static int32_t effect_value_base_max_sight(void)
{
	return z_info->max_sight;
}

static int32_t effect_value_base_weapon_damage(void)
{
	struct object *obj = player->body.slots[slot_by_name(player, "weapon")].obj;
	if (!obj) {
		return 0;
	}
	return (damroll(obj->dd, obj->ds) + obj->to_d);
}

static int32_t effect_value_base_player_hp(void)
{
	return player->chp;
}

static int32_t effect_value_base_monster_percent_hp_gone(void)
{
	/* Get the targeted monster, fail horribly if none */
	struct monster *mon = target_get_monster();

	return mon ? (((mon->maxhp - mon->hp) * 100) / mon->maxhp) : 0;
}

expression_base_value_f effect_value_base_by_name(const char *name)
{
	static const struct value_base_s {
		const char *name;
		expression_base_value_f function;
	} value_bases[] = {
		{ "SPELL_POWER", effect_value_base_spell_power },
		{ "PLAYER_LEVEL", effect_value_base_player_level },
		{ "DUNGEON_LEVEL", effect_value_base_dungeon_level },
		{ "MAX_SIGHT", effect_value_base_max_sight },
		{ "WEAPON_DAMAGE", effect_value_base_weapon_damage },
		{ "PLAYER_HP", effect_value_base_player_hp },
		{ "MONSTER_PERCENT_HP_GONE",
		  effect_value_base_monster_percent_hp_gone },
		{ NULL, NULL },
	};
	const struct value_base_s *current = value_bases;

	while (current->name != NULL && current->function != NULL) {
		if (my_stricmp(name, current->name) == 0)
			return current->function;

		current++;
	}

	return NULL;
}

/**
 * ------------------------------------------------------------------------
 * Execution of effects
 * ------------------------------------------------------------------------ */
/**
 * Execute an effect chain.
 *
 * \param effect is the effect chain
 * \param origin is the origin of the effect (player, monster etc.)
 * \param obj    is the object making the effect happen (or NULL)
 * \param ident  will be updated if the effect is identifiable
 *               (NB: no effect ever sets *ident to false)
 * \param aware  indicates whether the player is aware of the effect already
 * \param dir    is the direction the effect will go in
 * \param beam   is the base chance out of 100 that a BOLT_OR_BEAM effect will beam
 * \param boost  is the extent to which skill surpasses difficulty, used as % boost. It
 *               ranges from 0 to 138.
 * \param cmd    If the effect is invoked as part of a command, this is the
 *               the command structure - used primarily so repeating the
 *               command can use the same information without prompting the
 *               player again.  Use NULL for this if not invoked as part of
 *               a command.
 */
bool effect_do(struct effect *effect,
		struct source origin,
		struct object *obj,
		bool *ident,
		bool aware,
		int dir,
		int beam,
		int boost,
		struct command *cmd)
{
	bool completed = false;
	effect_handler_f handler;
	random_value value = { 0, 0, 0, 0 };

	do {
		int choice_count = 0, leftover = 1;

		if (!effect_valid(effect)) {
			msg("Bad effect passed to effect_do(). Please report this bug.");
			return false;
		}

		if (effect->dice != NULL)
			choice_count = dice_roll(effect->dice, &value);

		/* Deal with special random and select effects */
		if (effect->index == EF_RANDOM || effect->index == EF_SELECT) {
			int choice;

			/*
			 * If it has no subeffects, act as if it completed
			 * successfully and go to the next effect.
			 */
			if (choice_count <= 0) {
				completed = true;
				effect = effect->next;
				continue;
			}

			/*
			 * Treat select effects like random ones if they
			 * aren't from a player or if there's really no choice
			 * to be made.
			 */
			if (effect->index == EF_RANDOM ||
					origin.what != SRC_PLAYER ||
					choice_count < 2) {
				choice = randint0(choice_count);
			} else {
				assert(effect->index == EF_SELECT &&
					origin.what == SRC_PLAYER);
				/*
				 * Since a choice is presented, allow
				 * identification, even if no choice is made.
				 */
				*ident = true;
				if (cmd) {
					if (cmd_get_effect_from_list(cmd,
							"list_index",
							&choice, NULL,
							effect->next,
							choice_count,
							true) != CMD_OK) {
						return false;
					}
				} else {
					choice = get_effect_from_list(NULL,
						effect->next, choice_count,
						true);
					if (choice == -1) return false;
				}

				/*
				 * If the player chose to use a random effect,
				 * roll for it.
				 */
				if (choice == -2) {
					choice = randint0(choice_count);
				}
				assert(choice >= 0 && choice < choice_count);
			}

			leftover = choice_count - choice;

			/* Skip to the chosen effect */
			effect = effect->next;
			while (choice-- && effect)
				effect = effect->next;
			if (!effect) {
				/*
				 * There's fewer subeffects than expected.  Act
				 * as if it ran successfully.
				 */
				completed = true;
				break;
			}

			/* Roll the damage, if needed */
			if (effect->dice != NULL)
				(void) dice_roll(effect->dice, &value);
		}

		/* Handle the effect */
		handler = effects[effect->index].handler;
		if (handler != NULL) {
			effect_handler_context_t context = {
				effect->index,
				origin,
				obj,
				aware,
				dir,
				beam,
				boost,
				value,
				effect->subtype,
				effect->radius,
				effect->other,
				effect->y,
				effect->x,
				effect->msg,
				*ident,
				cmd
			};

			completed = handler(&context) || completed;
			*ident = context.ident;
		}

		/* Get the next effect, if there is one */
		while (leftover-- && effect)
			effect = effect->next;
	} while (effect);

	return completed;
}

/**
 * Perform a single effect with a simple dice string and parameters
 * Calling with ident a valid pointer will (depending on effect) give success
 * information; ident = NULL will ignore this
 */
void effect_simple(int index,
				   struct source origin,
				   const char *dice_string,
				   int subtype,
				   int radius,
				   int other,
				   int y,
				   int x,
				   bool *ident)
{
	struct effect effect;
	int dir = DIR_TARGET;
	bool dummy_ident = false;

	/* Set all the values */
	memset(&effect, 0, sizeof(effect));
	effect.index = index;
	effect.dice = dice_new();
	dice_parse_string(effect.dice, dice_string);
	effect.subtype = subtype;
	effect.radius = radius;
	effect.other = other;
	effect.y = y;
	effect.x = x;

	/* Direction if needed */
	if (effect_aim(&effect))
		get_aim_dir(&dir);

	/* Do the effect */
	if (!ident) {
		ident = &dummy_ident;
	}

	effect_do(&effect, origin, NULL, ident, true, dir, 0, 0, NULL);
	dice_free(effect.dice);
}

/**
 * Returns N which is the 1 in N chance for recharging to fail.
 */
int recharge_failure_chance(const struct object *obj, int strength) {
	/* Ease of recharge ranges from 9 down to 4 (wands) or 3 (staffs) */
	int ease_of_recharge = (100 - obj->kind->level) / 10;
	int raw_chance = strength + ease_of_recharge
		- 2 * (obj->pval / obj->number);
	return raw_chance > 1 ? raw_chance : 1;
}

